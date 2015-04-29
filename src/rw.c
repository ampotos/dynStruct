#include "dr_api.h"
#include "dr_ir_opnd.h"
#include "../includes/utils.h"
#include "../includes/block_utils.h"
#include "../includes/allocs.h"

// todo  : find a way to get instruction with pc (and if none do nothing)

void	memory_read(app_pc pc)
{
  opnd_t	src;
  void		*drcontext = dr_get_current_drcontext();

  dr_printf("pc r: %p\n", pc);

  /* for (int i = 0; i < instr_num_srcs(instr); i++) */
  /*   { */
  /*     /\* src = instr_get_src(instr, i); *\/ */
  /*     /\* if (opnd_is_memory_reference(src)) *\/ */
  /* 	dr_printf("memory ref reading\n");// add to block if ref is in one of the block */
  /*     // check if is reg and if it is check is his value is not memory ref */
  /*     // for each memory ref check is in block */
  /*     // if it is register */
  /*   } */
}

void	memory_write(app_pc pc)
{
  opnd_t	dst;
  void		*drcontext = dr_get_current_drcontext();
  
  dr_printf("pc w: %p\n", pc);

  /* for (int i = 0; i < instr_num_dsts(instr); i++) */
  /*   { */
  /*     /\* dst = instr_get_dst(instr, i); *\/ */
  /*     /\* if (opnd_is_memory_reference(dst)) *\/ */
  /* 	dr_printf("memory ref writting\n");// add to block if ref is in one of the block */
  /*     // check if is reg and if it is check is his value is not memory ref */
  /*     // for each memory ref check is in block */
  /*     // if it is register */
  /*   } */
}
