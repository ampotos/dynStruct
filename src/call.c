#include "dr_api.h"
#include "drmgr.h"
#include "dr_ir_opnd.h"
#include "../includes/call.h"

// actually we only follow call and return, so if a program use a jump instead
// of a call we don't add it on the stack and there is a tricky return the stack
// is going to be fucked up
void	dir_call_monitor(void *pc)
{
  stack_t	*new_func;
  stack_t	*stack;
  void		*drcontext = dr_get_current_drcontext();

  stack = drmgr_get_tls_field(drcontext, tls_stack_idx);

  if (!(new_func = dr_global_alloc(sizeof(*new_func))))
    dr_printf("dr_malloc fail\n");
  else
    {
      new_func->next = stack;
      new_func->addr = pc;
      drmgr_set_tls_field(drcontext, tls_stack_idx, new_func);
    }
}


void	ind_call_monitor(app_pc caller, app_pc callee)
{
  stack_t	*new_func;
  stack_t	*stack;
  void		*drcontext = dr_get_current_drcontext();

  stack = drmgr_get_tls_field(drcontext, tls_stack_idx);

  if (!(new_func = dr_global_alloc(sizeof(*new_func))))
    dr_printf("dr_malloc fail\n");
  else
    {
      new_func->next = stack;
      new_func->addr = callee;
      drmgr_set_tls_field(drcontext, tls_stack_idx, new_func);
    }
}


void	ret_monitor()
{
  stack_t       *stack;
  void          *drcontext = dr_get_current_drcontext();

  stack = drmgr_get_tls_field(drcontext, tls_stack_idx);
  if (stack)
    {
      drmgr_set_tls_field(drcontext, tls_stack_idx, stack->next);
      dr_global_free(stack, sizeof(*stack));
    }
}

void	clean_stack(void *drcontext)
{
  stack_t       *stack;
  stack_t       *stack_tmp;

  stack = drmgr_get_tls_field(drcontext, tls_stack_idx);
  while (stack)
    {
      stack_tmp = stack;
      stack = stack->next;
      dr_global_free(stack_tmp, sizeof(*stack_tmp));
    }
}
