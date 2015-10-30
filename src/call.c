#include "dr_api.h"
#include "drmgr.h"
#include "dr_ir_opnd.h"
#include "../includes/call.h"
#include "../includes/sym.h"
#include "../includes/elf.h"
#include "../includes/utils.h"

#if !__LP64__
module_data_t	*dynamo_mod = NULL;
#endif

void *get_real_func_addr(void *pc, void *got)
{
  void          *drcontext = dr_get_current_drcontext();
  instr_t       *instr = instr_create(drcontext);
  int		offset;

  pc = dr_app_pc_for_decoding(pc);

  instr_init(drcontext, instr);
  if (!decode(drcontext, pc, instr))
    {
      dr_printf("Decode of instruction at %p failed\n", pc);
      return NULL;
    }

  // in the plt we want to find the first push in order to know
  // the offset of the got who contain the addr of the target func
  while (instr_get_opcode(instr) != OP_push_imm)
    {
      instr_reset(drcontext, instr);
      pc = decode_next_pc(drcontext, pc);
      if (!decode(drcontext, pc, instr))
	{
	  dr_printf("Decode of instruction at %p failed\n", pc);
	  return NULL;
	}
    }

  offset = opnd_get_immed_int(instr_get_src(instr, 0));
  instr_destroy(drcontext, instr);

#if __LP64__
  return *((ptr_int_t **)(got + offset * sizeof(void*)));
#else
  return *((ptr_int_t **)(got + offset / 2));
#endif
}

void dir_call_monitor(void *pc)
{
  stack_t	*new_func;
  stack_t	*stack;
  void		*drcontext = dr_get_current_drcontext();
  
#if !__LP64__
  if ((dynamo_mod && dr_module_contains_addr(dynamo_mod, pc)))
    return;
#endif

  stack = drmgr_get_tls_field(drcontext, tls_stack_idx);
  if (!(new_func = dr_thread_alloc(drcontext, sizeof(*new_func))))
    dr_printf("dr_malloc fail\n");
  else
    {
      ds_memset(new_func, 0, sizeof(*new_func));
      new_func->next = stack;
      // we check is the addr is on plt here for performance issue
      // the plt addr is going to be replace by the real addr
      // of the target function the first time we need to get
      // this information
      if (search_on_tree(plt_tree, pc))
	{
	  new_func->on_plt = 1;
	  new_func->was_on_plt = 1;
	}
      new_func->addr = pc;
      
      drmgr_set_tls_field(drcontext, tls_stack_idx, new_func);
    }
}


void ind_call_monitor(app_pc __attribute__((unused))caller, app_pc callee)
{
  dir_call_monitor(callee);
}

int is_in_same_module(stack_t *stack, void *func)
{
  module_data_t	*mod;
  int		ret;
  void		*got;
  void		*tmp_addr;

  static int first_malloc= 0;
  
  if (stack->on_plt)
    {
      got = search_on_tree(plt_tree, stack->addr);
      if ((tmp_addr = get_real_func_addr(stack->addr, got)))
  	{
  	  stack->addr = tmp_addr;
  	  stack->on_plt = 0;
  	}
    }

  if (!(mod = dr_lookup_module(stack->addr)))
    return false;
  ret = dr_module_contains_addr(mod, func);

  dr_free_module_data(mod);

  if (!ret && !first_malloc)
    {
      first_malloc++;
      return true;
    }
  
  return ret;
}

#if __LP64__
void ret_monitor(__attribute__((unused))void *pc)
#else
void ret_monitor(void *pc)
#endif
{
  stack_t       *stack;
  void          *drcontext = dr_get_current_drcontext();

  static int first_plt = 0;
  
  stack = drmgr_get_tls_field(drcontext, tls_stack_idx);

#if __LP64__
  if (stack)
#else
  if (stack && (is_in_same_module(stack, pc) ||
		(stack->was_on_plt && !first_plt)))
#endif
    {
#if __LP64__
      if (!first_plt)
	first_plt++;
#endif
      drmgr_set_tls_field(drcontext, tls_stack_idx, stack->next);
      dr_thread_free(drcontext, stack, sizeof(*stack));
    }
}

void clean_stack(void *drcontext)
{
  stack_t       *stack;
  stack_t       *stack_tmp;

  stack = drmgr_get_tls_field(drcontext, tls_stack_idx);
  while (stack)
    {
      stack_tmp = stack;
      stack = stack->next;
      dr_thread_free(drcontext, stack_tmp, sizeof(*stack_tmp));
    }
}

void get_caller_data(void **addr, char **sym, void *drcontext, int alloc)
{
  stack_t *func = drmgr_get_tls_field(drcontext, tls_stack_idx);
  void	*got;
  
  if(!func)
    return;

  // alloc is set for *alloc and free, because in this case we have to
  // skip the first entry on the stack to get the function
  // that called *alloc or free
  if (alloc && func->next)
    func = func->next;
  
  // we read the got here, because at the time when the plt is called
  // the got may not contain the addr of the target function
  // the check to know if the addr is in plt or not is done at the same
  // as the calling for performance issue.
  if (func->on_plt)
    {
      got = search_on_tree(plt_tree, func->addr);
      func->addr = get_real_func_addr(func->addr, got);
      func->on_plt = 0;
    }

  if (addr)
    *addr = func->addr;
  
  if (sym)
    {
      if (!func->name)
	func->name = hashtable_lookup(&sym_hashtab, func->addr);
      *sym = func->name;
    }
}
