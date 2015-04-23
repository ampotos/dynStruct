#include "dr_api.h"
#include "dr_ir_opnd.h"
#include "drwrap.h"
#include "utils.h"
#include "block_utils.h"

malloc_t  *blocks = NULL;
void      *lock;

static void pre_malloc(void *wrapctx, OUT void **user_data)
{
  malloc_t	*new;

  dr_mutex_lock(lock);
  
  *user_data = add_block((size_t)drwrap_get_arg(wrapctx, 0), drwrap_get_retval(wrapctx));

  dr_mutex_unlock(lock);
}

void set_addr_malloc(malloc_t *block, void *start, unsigned int flag, int realloc)
{
  if (!start && block)
    {
      if (!realloc)
	{
	  dr_printf("Malloc of size %d by %s failed\n", block->size, block->module_name_malloc);
	  remove_block(block);
	}
      // if start == NULL on realloc set block to free to keep previous access to data
      else if (!(block->flag & FREE))
	{
	  dr_printf("Realloc of size %d by %s on %p failed\n", block->size, block->module_name_malloc, block->start);
	  block->flag |= FREE;
	}
    }
  else if (block)
    {
      block->start = start;
      block->end = block->start + block->size;
      block->flag = flag;
    }
  else
    dr_printf("Error : *alloc post wrapping call without pre wrapping\n");
}

static void post_malloc(void *wrapctx, void *user_data)
{
  malloc_t	*block = (malloc_t *)user_data;

  dr_mutex_lock(lock);

  if (block && drwrap_get_retaddr(wrapctx))
    set_addr_malloc(block, drwrap_get_retval(wrapctx), ALLOC, 0);

  dr_mutex_unlock(lock);
}

static void post_calloc(void *wrapctx, void *user_data)
{
  malloc_t	*block = (malloc_t *)user_data;

  dr_mutex_lock(lock);

  if (block)
    set_addr_malloc(block, drwrap_get_retval(wrapctx), ALLOC | CALLOC, 0);
  
  dr_mutex_unlock(lock);
}

static void pre_realloc(void *wrapctx, OUT void **user_data)
{
  malloc_t	*block;
  void		*start = drwrap_get_arg(wrapctx, 0);
  size_t	size = (size_t)drwrap_get_arg(wrapctx, 1);

  // maybe store module name of each realloc (and size and ptr change);

  // ptr == NULL : like malloc
  dr_mutex_lock(lock);
  if (!start)
    block = add_block(size, drwrap_get_retaddr(wrapctx));
  else
    {
      if (!(block = get_block_by_addr(start)))
	dr_printf("Realloc on %p error : addr was not previously malloc\n", start);
      else if (!size)
	block->flag |= FREE;
      else
	block->size = size;
    }
  *user_data = block;
  dr_mutex_unlock(lock);
}

static void post_realloc(void *wrapctx, void *user_data)
{
  malloc_t	*block = (malloc_t *)user_data;
  void		*ret = drwrap_get_retval(wrapctx);

  // if null => fail or free, on both case flag is set to free
  if (block)
    set_addr_malloc(block, ret, block->flag, 1);
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
  app_pc	realloc = (app_pc)dr_get_proc_address(mod->handle, "realloc");
  app_pc	free = (app_pc)dr_get_proc_address(mod->handle, "free");

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

  // wrap calloc pre malloc and pre calloc are the same
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

  // wrap realloc
  if (realloc)
    {
      dr_printf("realloc found at %p in %s\n", malloc, dr_module_preferred_name(mod));
      if (drwrap_wrap(realloc, pre_realloc, post_realloc))
  	dr_printf("\tWrap sucess\n");
      else
  	dr_printf("\tWrap fail\n");
    }
  else
    dr_printf("realloc not found in %s\n", dr_module_preferred_name(mod));

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
