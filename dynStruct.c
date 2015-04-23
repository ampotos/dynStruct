#include "dr_api.h"
#include "dr_ir_opnd.h"
#include "drwrap.h"

// sort at insert to have an easy final size calculation
typedef struct access_s access_t;
struct access_s
{
  struct access_s  *next;
  void		   *addr;
  size_t	   nb_hit;
};


typedef struct malloc_s malloc_t;
struct malloc_s
{
  void		  *start;
  void		  *end;
  size_t	  size;
  access_t	  *access;
  unsigned int	  flag;
  void		  *ret_malloc;
  char		  *module_name_malloc;
  void		  *ret_free;
  char		  *module_name_free;
  struct malloc_s *next;
};

// define flag for malloc_t
#define ALLOC 0x1
#define CALLOC (0x1 << 1)
#define FREE (0x1 << 2)

malloc_t  *blocks = NULL;
void	  *lock;

char *my_dr_strdup(const char* str)
{
  char		*ret;
  size_t	len;

  len = -1;
  while (str[++len]);
  if ((ret = dr_global_alloc((len + 1) * sizeof(*ret))))
    {
      len = -1;
      while (str[++len])
	ret[len] = str[len];
      ret[len] = 0;
    }
  return (ret);
}

static void pre_malloc(void *wrapctx, OUT void **user_data)
{
  malloc_t	*last = blocks;
  module_data_t	*m_data;

  dr_mutex_lock(lock);

  if (last)
    {
      while (last->next)
	last = last->next;
      if (!(last->next = dr_global_alloc(sizeof(*last))))
	dr_printf("dr_malloc fail\n");
      last = last->next;
    }
  else
    if (!(blocks = dr_global_alloc(sizeof(*last))))
      dr_printf("dr_malloc fail\n");
    else
      last = blocks;
  
  if (last)
    {
      last->start = NULL;
      last->end = NULL;
      last->size = (size_t)drwrap_get_arg(wrapctx, 0);
      last-> flag = 0;
      last->ret_malloc = drwrap_get_retaddr(wrapctx);
      if (m_data = dr_lookup_module(last->ret_malloc))
	{
	  last->module_name_malloc = my_dr_strdup(dr_module_preferred_name(m_data));
	  dr_free_module_data(m_data);
	}
      else
	last->module_name_malloc = NULL;
      last->next = NULL;
    }

  dr_mutex_unlock(lock);
}

malloc_t *get_unalloc_block(void *ret)
{
  malloc_t	*block = blocks;

  while (block)
    {
      if (block->ret_malloc == ret && !(block->flag))
	return block;
      block = block->next;
    }

  return NULL;
}

void free_malloc_block(malloc_t *block)
{
  size_t	len;

  if (block)
    {
      if (block->module_name_malloc)
	{
	  len = -1;
	  while (block->module_name_malloc[++len]);
	  dr_global_free(block->module_name_malloc, len);
	}

      if (block->module_name_free)
	{
	  len = -1;
	  while (block->module_name_free[++len]);
	  dr_global_free(block->module_name_free, len);
	}

      dr_global_free(block, sizeof(*block));
    }
}

void remove_block(malloc_t *block)
{
  malloc_t	*tmp = blocks;

  if (tmp == block)
    {
      blocks = tmp->next;
      free_malloc_block(block);
    }
  else
    {
      while (tmp)
	{
	  if (tmp->next == block)
	    {
	      tmp->next = tmp->next->next;
	      free_malloc_block(block);
	      break;
	    }
	  tmp = tmp->next;
	}
    }
}

void set_addr_malloc(malloc_t *block, void *start, unsigned int flag)
{
  if (!start && block)
    {
      dr_printf("Malloc of size %d by %s failed\n", block->size, block->module_name_malloc);
      remove_block(block);
    }
  else if (block)
    {
      block->start = start;
      block->end = block->start + block->size;
      block->flag = flag;
    }
  else
    dr_printf("Error : post wrapping call without pre wrapping\n");
}

static void post_malloc(void *wrapctx, void *user_data)
{
  malloc_t	*block;

  dr_mutex_lock(lock);

  block = get_unalloc_block(drwrap_get_retaddr(wrapctx));
  if (block && drwrap_get_retaddr(wrapctx))
    set_addr_malloc(block, drwrap_get_retval(wrapctx), ALLOC);

  dr_mutex_unlock(lock);
}

static void post_calloc(void *wrapctx, void *user_data)
{
  malloc_t	*block;

  dr_mutex_lock(lock);

  block = get_unalloc_block(drwrap_get_retaddr(wrapctx));
  if (block)
    set_addr_malloc(block, drwrap_get_retval(wrapctx), ALLOC | CALLOC);
  
  dr_mutex_unlock(lock);
}

malloc_t *get_block_by_addr(void *addr)
{
  malloc_t	*block = blocks;

  while(block)
    {
      if (block->start == addr)
	return block;
      block = block->next;
    }
  return NULL;
}

static void pre_free(void *wrapctx, OUT void **user_data)
{
  malloc_t	*block;
  module_data_t	*m_data;

  // free(0) du nothing
  if (!drwrap_get_arg(wrapctx,0))
    return ;

  dr_mutex_lock(lock);

  block = get_block_by_addr(drwrap_get_arg(wrapctx, 0));
  // if the block was previously malloc we set it to free
  if (block)
    {
      block->flag |= FREE;
      block->ret_free = drwrap_get_retaddr(wrapctx);

      if (m_data = dr_lookup_module(block->ret_free))
	{
	  block->module_name_free = my_dr_strdup(dr_module_preferred_name(m_data));
	  dr_free_module_data(m_data);
	}
      else
	block->module_name_free = NULL;
    }
  else
    dr_printf("free of non malloc adress : %p\n", drwrap_get_arg(wrapctx, 0));

  dr_mutex_unlock(lock);
}

static void load_event(void *drcontext, const module_data_t *mod, bool loaded)
{
  app_pc	malloc = (app_pc)dr_get_proc_address(mod->handle, "malloc");
  app_pc	calloc = (app_pc)dr_get_proc_address(mod->handle, "calloc");
  app_pc	free = (app_pc)dr_get_proc_address(mod->handle, "free");

  // wrap only libc malloc to have all malloc without duplication
  if (strncmp(dr_module_preferred_name(mod), "libc.so", 7))
    return;

  // wrap malloc
  if (malloc)
    {
      dr_printf("malloc found at %p in %s\n", malloc, dr_module_preferred_name(mod));
      if (drwrap_wrap(malloc, pre_malloc, post_malloc))
	dr_printf("\tWrap sucess\n");
      else
	dr_printf("\tWrap fail\n");
    }
  else
    dr_printf("Malloc not found in %s\n", dr_module_preferred_name(mod));

  // wrap malloc pre malloc and pre calloc are the same
  if (calloc)
    {
      dr_printf("Calloc found at %p in %s\n", malloc, dr_module_preferred_name(mod));
      if (drwrap_wrap(calloc, pre_malloc, post_calloc))
	dr_printf("\tWrap sucess\n");
      else
	dr_printf("\tWrap fail\n");
    }
  else
    dr_printf("Calloc not found in %s\n", dr_module_preferred_name(mod));

  // wrap free
  if (free)
    {
      dr_printf("free found at %p in %s\n", malloc, dr_module_preferred_name(mod));
      if (drwrap_wrap(free, pre_free, NULL))
	dr_printf("\tWrap sucess\n");
      else
	dr_printf("\tWrap fail\n");
    }
  else
    dr_printf("free not found in %s\n", dr_module_preferred_name(mod));
}

static void exit_event(void)
{
  malloc_t	*block = blocks;
  malloc_t	*tmp;

  dr_mutex_lock(lock);

  while (block)
    {
      tmp = block->next;
      dr_printf("%p-%p(0x%x) ", block->start, block->end, block->size);
      if (block->flag & CALLOC)
	dr_printf("calloc by ");
      else
	dr_printf("malloc by ");
      if (block->flag & FREE)
	dr_printf("%s and free by %s\n", block->module_name_malloc, block->module_name_free);
      else
	dr_printf("%s and not free\n", block->module_name_malloc);
      free_malloc_block(block);
      block = tmp;
    }
  blocks = NULL;
  
  dr_mutex_unlock(lock);
  dr_mutex_destroy(lock);
  
  drwrap_exit();
}

DR_EXPORT void dr_init(client_id_t id)
{
  drwrap_init();

  dr_register_exit_event(exit_event); 
  dr_register_module_load_event(load_event);

  lock = dr_mutex_create();  
}
