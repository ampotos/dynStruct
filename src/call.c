#include "dr_api.h"
#include "drmgr.h"
#include "dr_ir_opnd.h"
#include "../includes/call.h"
#include "../includes/sym.h"
#include "../includes/elf.h"
#include "../includes/utils.h"

module_t	*module_list = NULL;

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
  // the offset on the got where we can find the addr of the target func
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

#ifdef BUILD_64
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

  stack = drmgr_get_tls_field(drcontext, tls_stack_idx);
  if (!(new_func = dr_thread_alloc(drcontext, sizeof(*new_func))))
    dr_printf("dr_malloc fail\n");
  else
    {
      ds_memset(new_func, 0, sizeof(*new_func));
      new_func->next = stack;
      // we only check if the addr is on plt here for performance
      // the plt addr is going to be replace by the real addr
      // of the target function the first time we need this data
      if (search_on_tree(plt_tree, pc))
	{
	  new_func->on_plt = 1;
	  new_func->was_on_plt = 1;
	}
      new_func->addr = pc;
      drmgr_set_tls_field(drcontext, tls_stack_idx, new_func);
    }
}

/* apparently call relative to gs segment don't have proper return on 32 bits */
/* this made the stack to be false and make the wrapped and monitoring. */
/* detection do false positive */
/* That kind af call don't have symbol so it's not a problem to just */
/* ignore them. */
/* for now no problem was detected on 64 bits build */
#ifdef BUILD_32
bool indirect_call_ignore(instr_t *instr)
{
  opnd_t	src;

  src = instr_get_src(instr, 0);

  if (opnd_is_far_base_disp(src) && opnd_get_segment(src) == DR_SEG_GS)
    return true;
  return false;
}
#endif


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
  
  return ret;
}

#ifdef BUILD_64
void ret_monitor(__attribute__((unused))void *pc)
#else
void ret_monitor(void *pc)
#endif
{
  stack_t       *stack;
  void          *drcontext = dr_get_current_drcontext();

  stack = drmgr_get_tls_field(drcontext, tls_stack_idx);

#ifdef BUILD_64
  if (stack)
#else
  if (stack && (is_in_same_module(stack, pc) || (stack->was_on_plt)))
#endif
    {
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

int add_to_module_list(const module_data_t *mod)
{
  module_t	*node;

  if (!(node = dr_global_alloc(sizeof(*node))))
    return false;

  node->next = module_list;
  module_list = node;

  if (!(node->module = dr_copy_module_data(mod)))
    return false;

  return true;
}

void get_caller_data(void **addr, char **sym, const char **module,
		     void *drcontext, int alloc)
{
  stack_t	*func = drmgr_get_tls_field(drcontext, tls_stack_idx);
  void		*got;
  module_t	*mod;
  
  if(!func)
    return;
  // alloc is set for *alloc and free, because in this case we have to
  // skip the first entry on the stack to get the function
  // that called *alloc or free
  if (alloc && func->next)
    func = func->next;
  // we read the got here, because at the time when the plt is called
  // the got may not contain the addr of the target function.
  // This also help to have better performance by resolving got addr
  // only when it's necessary
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
  
  if (module)
    {
      for (mod = module_list; mod; mod = mod->next)
	{
	  if (dr_module_contains_addr(mod->module, func->addr))
	    {
	      func->module_name = dr_module_preferred_name(mod->module);
	      *module = func->module_name;
	    }
	}
    }
}

void clean_module_list()
{
  module_t	*mod = module_list;
  module_t	*tmp;
  
  while (mod)
    {
      tmp = mod;
      mod = mod->next;
      dr_free_module_data(tmp->module);
      dr_global_free(tmp, sizeof(*tmp));
    }
}
