#include "dr_api.h"
#include "dr_ir_opnd.h"
#include "../includes/utils.h"
#include "../includes/block_utils.h"
#include "../includes/allocs.h"

// todo  : find a way to get instruction with pc (and if none do nothing)

void	memory_read(void *pc)
{
  void		*drcontext = dr_get_current_drcontext();
  instr_t	*instr = instr_create(drcontext);
  opnd_t	src;

  pc = dr_app_pc_for_decoding(pc);

  instr_init(drcontext, instr);
  if (!decode(drcontext, pc, instr))
    {
      dr_printf("Decode of instruction at %p failed\n", pc);
      return;
    }

  for (int i = 0; i < instr_num_srcs(instr); i++)
    {
      src = instr_get_src(instr, i);
    }

  instr_destroy(drcontext, instr);
}

void	memory_write(void *pc)
{
  void		*drcontext = dr_get_current_drcontext();
  instr_t	*instr = instr_create(drcontext);
  opnd_t	dst;
  
  pc = dr_app_pc_for_decoding(pc);

  instr_init(drcontext, instr);
  if(!decode(drcontext, pc, instr))
    {
      dr_printf("Decode of instruction at %p failed\n", pc);
      return;
    }

  for (int i = 0; i < instr_num_dsts(instr); i++)
    {
      dst = instr_get_dst(instr, i);
    }

  instr_destroy(drcontext, instr);
}
