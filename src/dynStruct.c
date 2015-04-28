#include "dr_api.h"
#include "dr_ir_opnd.h"
#include "drwrap.h"
#include "drmgr.h"
#include "../includes/utils.h"
#include "../includes/block_utils.h"
#include "../includes/allocs.h"

malloc_t  *blocks = NULL;
void      *lock;

static void load_event(__attribute__((unused))void *drcontext, const module_data_t *mod, __attribute__((unused))bool loaded)
{
  app_pc	malloc = (app_pc)dr_get_proc_address(mod->handle, "malloc");
  app_pc	calloc = (app_pc)dr_get_proc_address(mod->handle, "calloc");
  app_pc	realloc = (app_pc)dr_get_proc_address(mod->handle, "realloc");
  app_pc	free = (app_pc)dr_get_proc_address(mod->handle, "free");

  // blacklist ld-linux to see only his internal alloc
  if (!my_dr_strncmp("ld-linux", dr_module_preferred_name(mod), 8))
    return ;

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

  // wrap calloc (same pre and post wrapping than malloc)
  if (calloc)
    {
      dr_printf("calloc found at %p in %s\n", calloc, dr_module_preferred_name(mod));
      if (drwrap_wrap(malloc, pre_malloc, post_malloc))
	dr_printf("\tWrap sucess\n");
      else
	dr_printf("\tWrap fail\n");
    }
  else
    dr_printf("Malloc not found in %s\n", dr_module_preferred_name(mod));

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
      if (block->flag & FREE)
	dr_printf(" => free\n");
      else
	dr_printf("=> not free\n");
      free_malloc_block(block);
      block = tmp;
    }
  blocks = NULL;
  
  dr_mutex_unlock(lock);
  dr_mutex_destroy(lock);
  
  drwrap_exit();
  drmgr_exit();
}

static dr_emit_flags_t bb_app2app_event(void *drcontext, void *tag, instrlist_t *bb,
					bool for_trace, bool translating)
{
  return DR_EMIT_DEFAULT;
}

static dr_emit_flags_t bb_analysis_event(void *drcontext, void *tag, instrlist_t *bb,
					bool for_trace, bool translating,
					OUT void **user_data)
{
  return DR_EMIT_DEFAULT;
}

static dr_emit_flags_t bb_insert_event(void *drcontext, void *tag, instrlist_t *bb,
				       instr_t *instr, bool for_trace, bool translating,
				       void *user_data)
{
  return DR_EMIT_DEFAULT;
}

DR_EXPORT void dr_init(__attribute__((unused))client_id_t id)
{
  drmgr_priority_t p = {
    sizeof(p),
    "reccord heap access",
    NULL,
    NULL,
    0};

  dr_set_client_name("dynStruct", "");

  drwrap_init();
  drmgr_init();

  dr_register_exit_event(exit_event);
  if (!drmgr_register_module_load_event(load_event) ||
      !drmgr_register_bb_app2app_event(bb_app2app_event, &p) ||
      !drmgr_register_bb_instrumentation_event(bb_analysis_event,
					       bb_insert_event, &p))
    DR_ASSERT(false);

  lock = dr_mutex_create();  
}
