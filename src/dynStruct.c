#include "dr_api.h"
#include "dr_ir_opnd.h"
#include "drutil.h"
#include "drwrap.h"
#include "drmgr.h"
#include "drsyms.h"
#include "../includes/utils.h"
#include "../includes/block_utils.h"
#include "../includes/allocs.h"
#include "../includes/rw.h"
#include "../includes/out.h"
#include "../includes/call.h"
#include "../includes/sym.h"
#include "../includes/tree.h"

malloc_t  *old_blocks = NULL;
tree_t	  *active_blocks = NULL;
void      *lock;

static void thread_exit_event(void *drcontext)
{
  clean_stack(drcontext);
}

// app2app is the first step of instrumentatiob, only use replace string
// instructions by a loop to have a better monitoring
static dr_emit_flags_t bb_app2app_event(void *drcontext,
					__attribute__((unused))void *tag,
					instrlist_t *bb,
					__attribute__((unused))bool for_trace,
					__attribute__((unused))bool translating)
{
  DR_ASSERT(drutil_expand_rep_string(drcontext, bb));

  return DR_EMIT_DEFAULT;
}

// instrument each read or write instruction in order to monitor them, also instrument each call/return to update the stack of functions
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

  // if it's a direct call we send the callee addr as parameter
  if (instr_is_call_direct(instr))
    {
      dr_insert_clean_call(drcontext, bb, instr, &dir_call_monitor,
			   false, 1, OPND_CREATE_INTPTR(instr_get_branch_target_pc(instr)));
    }
  // for indirect call we have to get callee addr on instrumentation function
  else if (instr_is_call_indirect(instr))
    dr_insert_mbr_instrumentation(drcontext, bb, instr, &ind_call_monitor,
				  SPILL_SLOT_1);
  else if (instr_is_return(instr))
    dr_insert_clean_call(drcontext, bb, instr, &ret_monitor,
			 false, 0);
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

  // store symbols on the hashtable (key : sym addr, value : name);
  dr_mutex_lock(lock);
  drsym_enumerate_symbols_ex(mod->full_path, sym_to_hashmap,
  			     sizeof(drsym_info_t), (void *)mod, 0);
  // todo parse lib to get start/end addr of plt and store it on a tree
  dr_mutex_unlock(lock);

  // free all data relative to sym (like debug info) after loading symbol
  drsym_free_resources(mod->full_path);

  // only wrap libc because we suppose our appli use standard malloc
  if (ds_strncmp("libc.so", mod_name, 7))
    return;

  if (malloc)
    DR_ASSERT(drwrap_wrap(malloc, pre_malloc, post_malloc));

  if (calloc)
    DR_ASSERT(drwrap_wrap(calloc, pre_calloc, post_calloc));

  if (realloc)
    DR_ASSERT(drwrap_wrap(realloc, pre_realloc, post_realloc));

  if (free)
    DR_ASSERT(drwrap_wrap(free, pre_free, NULL));
}

static void exit_event(void)
{
  dr_mutex_lock(lock);

  process_recover();

  clean_old_sym();
  hashtable_delete(sym_hashtab);

  dr_mutex_unlock(lock);
  dr_mutex_destroy(lock);
  
  drsym_exit();
  drwrap_exit();
  drmgr_exit();
  drutil_exit();
}

DR_EXPORT void dr_init(__attribute__((unused))client_id_t id)
{
  drmgr_priority_t p = {
    sizeof(p),
    "reccord heap access and recover datas structures",
    NULL,
    NULL,
    0};

  dr_set_client_name("dynStruct", "");

  drsym_init(0);
  drwrap_init();
  drmgr_init();
  drutil_init();

  dr_register_exit_event(&exit_event);
  if (!drmgr_register_module_load_event(&load_event) ||
      !drmgr_register_bb_app2app_event(&bb_app2app_event, &p) ||
      !drmgr_register_thread_exit_event(&thread_exit_event) ||
      //only use insert event because we only need to monitor single instruction
      !drmgr_register_bb_instrumentation_event(NULL, &bb_insert_event, &p))
    DR_ASSERT_MSG(false, "Can't register event handler\n");
  
  // register slot for each thread
  tls_stack_idx = drmgr_register_tls_field();
  DR_ASSERT_MSG(tls_stack_idx != -1, "Can't register tls field\n");

  // init sym hashtab
  sym_hashtab = dr_global_alloc(sizeof(*sym_hashtab));
  DR_ASSERT_MSG(sym_hashtab, "Global alloc fail\n");
  hashtable_init_ex(sym_hashtab, 8, HASH_INTPTR, false, false,
		    delete_sym, NULL, NULL);

  lock = dr_mutex_create();
  DR_ASSERT_MSG(lock, "Can't create mutex\n");
}
