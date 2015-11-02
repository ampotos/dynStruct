#include "dr_api.h"
#include "dr_ir_opnd.h"
#include "drmgr.h"
#include "../includes/utils.h"
#include "../includes/block_utils.h"
#include "../includes/allocs.h"
#include "../includes/call.h"
#include "../includes/sym.h"

orig_t *new_orig(size_t size, void *pc, void *drcontext)
{
  orig_t	*orig;
  
  if (!(orig = dr_global_alloc(sizeof(*orig))))
    dr_printf("dr_malloc fail\n");
  ds_memset(orig, 0, sizeof(*orig));
  
  orig->size = size;
  orig->nb_hit = 1;
  orig->addr = pc;
  // get the start addr of the function doing the access
  get_caller_data(&(orig->start_func_addr),
		  &(orig->start_func_sym),
		  &(orig->module_name), drcontext, 0);

  return orig;
}

void incr_orig(access_t *access, size_t size, void *pc, void *drcontext)
{
  orig_t	*orig_tree = search_on_tree(access->origs, pc);
  orig_t	*orig_list = orig_tree;
  tree_t	*new_node;

  while (orig_list && orig_list->size != size)
    orig_list = orig_list->next;

  if (orig_list)
    {
      orig_list->nb_hit++;
      return;
    }

  if (!orig_tree)
    {
      if (!(orig_tree = new_orig(size, pc, drcontext)))
	dr_printf("dr_malloc fail\n");
      if (!(new_node = dr_global_alloc(sizeof(*new_node))))
	dr_printf("dr_malloc fail\n");

      new_node->high_addr = pc;
      new_node->min_addr = pc;
      new_node->data = orig_tree;
      add_to_tree(&(access->origs), new_node);

      return;
    }

  // if a an orig have the same addr but not the same size we create an other
  // entry and put it in the linked list for this node;
  if (!orig_list)
    {
      if (!(orig_list = new_orig(size, pc, drcontext)))
	dr_printf("dr_malloc fail\n");
      if (!(new_node = dr_global_alloc(sizeof(*new_node))))
	dr_printf("dr_malloc fail\n");

      while (orig_tree->next)
	orig_tree = orig_tree->next;
      orig_tree->next = orig_list;
    }
}

void add_hit(void *pc, size_t size, void *target, int read, void *drcontext)
{
  malloc_t	*block = search_on_tree(active_blocks, target);
  access_t	*access;

  // if the access is not on a malloc block we do nothing
  // because the tree return a block if we target the last offset
  // we have to check it here (because last offset is out of the user
  // data and use only for malloc's internal purposes)
  if (!block || target == block->end)
    return;

  dr_mutex_lock(lock);

  if (read)
    access = get_access(target - block->start, &(block->read));
  else
    access = get_access(target - block->start, &(block->write));

  access->total_hits++;
  incr_orig(access, size, pc, drcontext);

  dr_mutex_unlock(lock);  
}

void check_opnd(opnd_t opnd, void *pc, int read, void *drcontext,
		  dr_mcontext_t *mctx)
{
  if (opnd_is_memory_reference(opnd) && opnd_is_base_disp(opnd))
    add_hit(pc, opnd_size_in_bytes(opnd_get_size(opnd)),
	    opnd_get_disp(opnd) + (void *)reg_get_value(opnd_get_base(opnd),
						       mctx),
	    read, drcontext);
  else if (opnd_is_memory_reference(opnd) && opnd_get_addr(opnd))
    add_hit(pc, opnd_size_in_bytes(opnd_get_size(opnd)), opnd_get_addr(opnd),
	    read, drcontext);
  // on all programm tested memory reference always match with one of the 2 prev
  else if (opnd_is_memory_reference(opnd))
    dr_printf("need to implem other memory ref\n");
}

void memory_read(void *pc)
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
      check_opnd(src, pc, 1, drcontext, &mctx);
    }
 
  instr_destroy(drcontext, instr);
}

void memory_write(void *pc)
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
  if (!decode(drcontext, pc, instr))
    {
      dr_printf("Decode of instruction at %p failed\n", pc);
      return;
    }

  for (int i = 0; i < instr_num_dsts(instr); i++)
    {
      dst = instr_get_dst(instr, i);
      check_opnd(dst, pc, 0, drcontext, &mctx);
    }

  instr_destroy(drcontext, instr);
}
