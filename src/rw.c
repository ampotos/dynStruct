#include "dr_api.h"
#include "dr_ir_opnd.h"
#include "../includes/utils.h"
#include "../includes/block_utils.h"
#include "../includes/allocs.h"

// TODO read stack of entry point to get the entry point of the current fuction en store it on orig struct

void	incr_orig(access_t *access, size_t size, void *pc)
{
  orig_t	*tmp_orig = access->origs;
  orig_t	*orig = NULL;

  while (tmp_orig)
    {
      if (tmp_orig->size == size && tmp_orig->addr == pc)
	{
	  orig = tmp_orig;
	  break;
	}
      tmp_orig  = tmp_orig->next;
    }

  // if a similar access with the actual is not found we create it
  if (!orig)
    {
      if (!(orig = dr_global_alloc(sizeof(*orig))))
	dr_printf("dr_malloc fail\n");
      else
	{
	  orig->size = size;
	  orig->nb_hit = 0;
	  orig->addr = pc;
	  orig->next = access->origs;
	  access->origs = orig;
	}
    }
  if (orig)
    orig->nb_hit++;
}

void	add_hit(void *pc, size_t size, void *target, int read)
{
  malloc_t	*block = get_active_block_by_access(target);
  access_t	*access;

  // if the access is not on a malloc block we do nothing
  if (!block)
    return;

  dr_mutex_lock(lock);

  if (read)
    access = get_access(target - block->start, &(block->read));
  else
    access = get_access(target - block->start, &(block->write));

  access->total_hits++;
  incr_orig(access, size, pc);

  dr_mutex_unlock(lock);  
}

void	memory_read(void *pc)
{
  void		*drcontext = dr_get_current_drcontext();
  instr_t	*instr = instr_create(drcontext);
  dr_mcontext_t mctx;
  opnd_t	src;

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
	// take a look at other type of memory ref to be 
	// sure we don't miss any ref to the heap
	if (opnd_is_base_disp(src))
	  add_hit(pc, opnd_size_in_bytes(opnd_get_size(src)), 
		  opnd_get_disp(src) + (void *)reg_get_value(opnd_get_base(src),
							     &mctx),
		  1);
    }
  
  instr_destroy(drcontext, instr);
}

void	memory_write(void *pc)
{
  void		*drcontext = dr_get_current_drcontext();
  instr_t	*instr = instr_create(drcontext);
  dr_mcontext_t mctx;
  opnd_t	dst;

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
	// take a look at other type of memory ref to be
	// sure we don't miss any ref to the heap
	if (opnd_is_base_disp(dst))
	  add_hit(pc, opnd_size_in_bytes(opnd_get_size(dst)), 
		  opnd_get_disp(dst) + (void *)reg_get_value(opnd_get_base(dst),
							     &mctx),
		  0);

    }

  instr_destroy(drcontext, instr);
}
