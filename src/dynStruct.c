#include <string.h>
#include "dr_api.h"
#include "dr_ir_opnd.h"
#include "drutil.h"
#include "drwrap.h"
#include "drmgr.h"
#include "../includes/utils.h"
#include "../includes/block_utils.h"
#include "../includes/allocs.h"
#include "../includes/rw.h"
#include "../includes/process.h"
#include "../includes/call.h"

malloc_t  *blocks = NULL;
void      *lock;

// app2app is the first step of instrumentatiob, only use replace string
// instructions by a loop to have a better monitoring
static dr_emit_flags_t bb_app2app_event(void *drcontext,
					__attribute__((unused))void *tag,
					instrlist_t *bb,
					__attribute__((unused))bool for_trace,
					__attribute__((unused))bool translating)
{
  if (!drutil_expand_rep_string(drcontext, bb))
    DR_ASSERT(false);

  return DR_EMIT_DEFAULT;
}

// instrument each read or write instruction in order to be able tu monitor them
static dr_emit_flags_t bb_insert_event( void *drcontext,
					__attribute__((unused))void *tag,
					instrlist_t *bb, instr_t *instr, 
					__attribute__((unused))bool for_trace,
					__attribute__((unused))bool translating,
				        __attribute__((unused))void *user_data)
{
  app_pc	pc = instr_get_app_pc(instr);
  
  // check if the instruction is valid
  if (pc == NULL)
    return DR_EMIT_DEFAULT;

  // TODO : add clean call for each direct/indirect call and return
  // maintain a stack for each thread with the addr of the entry point in the current fonction on top
  // on calls push new entry point val
  // on ret pop

  if (instr_reads_memory(instr))
    for (int i = 0; i < instr_num_srcs(instr); i++)
      if (opnd_is_memory_reference(instr_get_src(instr, i)))
  	{
  	  dr_insert_clean_call(drcontext, bb, instr, &memory_read,
			       false, 1, OPND_CREATE_INTPTR(pc));
  	  // break to not instrument the same instruction 2 time
  	  break;
  	}

  if (instr_writes_memory(instr))
    for (int i = 0; i < instr_num_dsts(instr); i++)
      if (opnd_is_memory_reference(instr_get_dst(instr, i)))
  	{
  	  dr_insert_clean_call(drcontext, bb, instr, &memory_write,
			       false, 1, OPND_CREATE_INTPTR(pc));
  	  // break to not instrument the same instruction 2 time
  	  break;
  	}

  if (instr_is_call_direct(instr))
    dr_insert_clean_call(drcontext, bb, instr, &dir_call_monitor,
			 false, 1, OPND_CREATE_INTPTR(pc));
  if (instr_is_call_indirect(instr))
    dr_insert_clean_call(drcontext, bb, instr, &ind_call_monitor,
			 false, 1, OPND_CREATE_INTPTR(pc));
  if (instr_is_return(instr))
    dr_insert_clean_call(drcontext, bb, instr, &ret_monitor,
			 false, 1, OPND_CREATE_INTPTR(pc));
  return DR_EMIT_DEFAULT;
}

static void load_event(__attribute__((unused))void *drcontext,
		       const module_data_t *mod,
		       __attribute__((unused))bool loaded)
{
  app_pc	malloc = (app_pc)dr_get_proc_address(mod->handle, "malloc");
  app_pc	calloc = (app_pc)dr_get_proc_address(mod->handle, "calloc");
  app_pc	realloc = (app_pc)dr_get_proc_address(mod->handle, "realloc");
  app_pc	free = (app_pc)dr_get_proc_address(mod->handle, "free");
  const char	*mod_name = dr_module_preferred_name(mod);

  // only wrap libc because we suppose our appli use standard malloc
  if (strncmp("libc.so", mod_name, 7))
    return;

  // wrap malloc
  if (malloc)
    if (!drwrap_wrap(malloc, pre_malloc, post_malloc))
      DR_ASSERT(false);

  // wrap calloc
  if (calloc)
    if (!drwrap_wrap(calloc, pre_calloc, post_calloc))
      DR_ASSERT(false);

  // wrap realloc
  if (realloc)
    if (!drwrap_wrap(realloc, pre_realloc, post_realloc))
      DR_ASSERT(false);

  // wrap free
  if (free)
    if (!drwrap_wrap(free, pre_free, NULL))
	DR_ASSERT(false);
}

static void exit_event(void)
{
  dr_mutex_lock(lock);

  process_recover();
  
  dr_mutex_unlock(lock);
  dr_mutex_destroy(lock);
  
  drwrap_exit();
  drmgr_exit();
  drutil_exit();
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
  drutil_init();

  dr_register_exit_event(&exit_event);
  if (!drmgr_register_module_load_event(&load_event) ||
      !drmgr_register_bb_app2app_event(&bb_app2app_event, &p) ||
      //only use insert event because we only need to monitor single instruction
      !drmgr_register_bb_instrumentation_event(NULL, &bb_insert_event, &p))
    DR_ASSERT(false);

  // maybe we need init/exit thread event to manage tls slot (or maybe opt)
  
  // register slot for each thread
  if ((tls_stack_idx = drmgr_register_tls_field()) == -1)
    DR_ASSERT(false);
  
  lock = dr_mutex_create();
}
