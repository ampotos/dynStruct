#include "dr_api.h"
#include "dr_ir_opnd.h"
#include "../includes/call.h"

// actually we only follow call and return, so if a program use a jump instead of a call we don't see it in the stack

void	dir_call_monitor(void *pc)
{
  stack_t	*new_func;
  return;
}


void	ind_call_monitor(app_pc caller, app_pc callee)
{
  stack_t	*new_func;
  return;
}


void	ret_monitor(void *pc)
{
  // TODO pop addr from tls stack 
  // using drmgr extention

  return;
}
