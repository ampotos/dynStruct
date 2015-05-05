#include "dr_api.h"
#include "dr_ir_opnd.h"
#include "../includes/utils.h"
#include "../includes/block_utils.h"
#include "../includes/allocs.h"

void	memory_read(void *pc)
{
  void		*drcontext = dr_get_current_drcontext();
  instr_t	*instr = instr_create(drcontext);
  dr_mcontext_t mctx;
  opnd_t	src;
  void		*addr_read;

  pc = dr_app_pc_for_decoding(pc);

  mctx.flags = DR_MC_CONTROL|DR_MC_INTEGER;
  mctx.size = sizeof(mctx);
  dr_get_mcontext(drcontext, &mctx);

  instr_init(drcontext, instr);
  if (!decode(drcontext, pc, instr))
    {
      dr_printf("Decode of instruction at %p failed\n", pc);
      return;
    }

  for (int i = 0; i < instr_num_srcs(instr); i++)
    {
      src = instr_get_src(instr, i);
      if (opnd_is_memory_reference(src))
	// take a look at other type of memory ref to be sure we don't miss any ref to the heap
	if (opnd_is_base_disp(src))
	  { 
	    addr_read = opnd_get_disp(src) + 
	      (void *)reg_get_value(opnd_get_base(src), &mctx);
	    if (get_block_by_access(addr_read))
	      dr_printf("read from % p at %p of %d\n", pc, addr_read,
			opnd_size_in_bytes(opnd_get_size(src)));
	  }
    }
  
  instr_destroy(drcontext, instr);
}

void	memory_write(void *pc)
{
  void		*drcontext = dr_get_current_drcontext();
  instr_t	*instr = instr_create(drcontext);
  dr_mcontext_t mctx;
  opnd_t	dst;
  void		*addr_write;
  
  pc = dr_app_pc_for_decoding(pc);
  
  mctx.flags = DR_MC_CONTROL|DR_MC_INTEGER;
  mctx.size = sizeof(mctx);
  dr_get_mcontext(drcontext, &mctx);

  instr_init(drcontext, instr);
  if(!decode(drcontext, pc, instr))
    {
      dr_printf("Decode of instruction at %p failed\n", pc);
      return;
    }

  for (int i = 0; i < instr_num_dsts(instr); i++)
    {
      dst = instr_get_dst(instr, i);
      if (opnd_is_memory_reference(dst))
	// take a look at other type of memory ref to be sure we don't miss any ref to the heap
	if (opnd_is_base_disp(dst))
	  {
	    addr_write = opnd_get_disp(dst) + 
	      (void *)reg_get_value(opnd_get_base(dst), &mctx);
	    if (get_block_by_access(addr_write))
	      dr_printf("write from % p at %p of %d\n", pc, addr_write,
			opnd_size_in_bytes(opnd_get_size(dst)));
	  }
    }

  instr_destroy(drcontext, instr);
}
