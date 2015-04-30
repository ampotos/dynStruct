#include "dr_api.h"
#include "dr_ir_opnd.h"
#include "../includes/utils.h"
#include "../includes/block_utils.h"
#include "../includes/allocs.h"

// todo  : find a way to get instruction with pc (and if none do nothing)

void	memory_read(app_pc pc)
{
  instr_t	instr;
  void		*drcontext = dr_get_current_drcontext();
  opnd_t	src;

  pc = dr_app_pc_for_decoding(pc);
  decode(drcontext, pc, &instr);

  for (int i = 0; i < instr_num_srcs(&instr); i++)
    {
      src = instr_get_src(&instr, i);
    }
}

void	memory_write(app_pc pc)
{
  instr_t	instr;
  void		*drcontext = dr_get_current_drcontext();
  opnd_t	dst;

  
  pc = dr_app_pc_for_decoding(pc);
  decode(drcontext, pc, &instr);

  for (int i = 0; i < instr_num_dsts(&instr); i++)
    {
      dst = instr_get_dst(&instr, i);
    }
}
