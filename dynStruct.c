#include "dr_api.h"
#include "dr_ir_opnd.h"
#include "drwrap.h"

// sort at insert to have an easy size calculation at the end
typedef struct access_s access_t;
struct access_s
{
  struct access_s  *next;
  void		  *addr;
};


typedef struct malloc_s malloc_t;
struct malloc_s
{
  void		  *start;
  void		  *end;
  size_t	  size;
  void		  *ret;
  access_t	  *access;
  int		  flag;
  struct malloc_s *next;
};

// define flag for malloc_t
#define FREE 0x1

#ifdef WINDOWS
# define IF_WINDOWS_ELSE(x,y) x
#else
# define IF_WINDOWS_ELSE(x,y) y
#endif

#define DR_MALLOC(s)  dr_global_alloc(s)
#define DR_FREE(p, s) dr_global_free(p, s)

malloc_t  *blocks = NULL;
void	  *lock;


static void pre_malloc(void *wrapctx, OUT void **user_data)
{
  malloc_t	*last = blocks;

  // TODO find a way to avoid double call of this pre wrapped call
  dr_mutex_lock(lock);
  if (last)
    {
      while (last->next)
	last = last->next;
      if (!(last->next = DR_MALLOC(sizeof(*last))))
	dr_printf("dr_malloc fail\n");
      last = last->next;
    }
  else
    if (!(last = DR_MALLOC(sizeof(*last))))
      dr_printf("dr_malloc fail\n");

  if (last)
    {
      last->start = NULL;
      last->end = NULL;
      last->size = (size_t)drwrap_get_arg(wrapctx, 0);
      last-> flag = 0;
      last->next = NULL;
      //this is use to write return value on the right block
      last->ret = drwrap_get_retaddr(wrapctx);
      dr_printf("m : %p\n", last->ret);
    }
  dr_mutex_unlock(lock);
}

static void post_malloc(void *wrapctx, void *user_data)
{
  dr_mutex_lock(lock);
  // todo print all message here + set start and end in node
  // find node by retaddr
  // if to node have the same ret value and are not free => delete the last one
  dr_printf("%p : ", drwrap_get_retaddr(wrapctx));
  dr_printf("%p\n", drwrap_get_retval(wrapctx));
  dr_mutex_unlock(lock);
}

static void pre_free(void *wrapctx, OUT void **user_data)
{
  // todo set flag in node for the freed block to free (detect use after-free?)
  dr_printf("free(%p)\n", drwrap_get_arg(wrapctx, 0))
}

static void load_event(void *drcontext, const module_data_t *mod, bool loaded)
{
  app_pc malloc = (app_pc)dr_get_proc_address(mod->handle, "malloc");
  app_pc free = (app_pc)dr_get_proc_address(mod->handle, "free");

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

static void exit_event(void);

DR_EXPORT void dr_init(client_id_t id)
{
// enable console printing on windows
#ifdef WINDOWS
  dr_enable_console_printing(); 
#endif

  drwrap_init();
  dr_register_exit_event(exit_event); 
  dr_register_module_load_event(load_event);
  lock = dr_mutex_create();  
}


static void exit_event(void)
{
  // TODO : print résulat, unalloc all blocks
  dr_mutex_destroy(lock);
  drwrap_exit();
}
