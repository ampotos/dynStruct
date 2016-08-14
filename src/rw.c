#include "dr_api.h"
#include "dr_ir_opnd.h"
#include "dr_ir_instr.h"
#include "drmgr.h"
#include "../includes/utils.h"
#include "../includes/block_utils.h"
#include "../includes/allocs.h"
#include "../includes/call.h"
#include "../includes/sym.h"
#include "../includes/custom_alloc.h"
#include "../includes/rw.h"

void copy_instr(void* addr, int size, unsigned char *instr)
{
  for (; size > 0; size--)
    instr[size - 1] = ((char *)addr)[size - 1];
}

orig_t *new_orig(size_t size, void *pc, void *drcontext, malloc_t *block,
		 ctx_t *ctx)
{
  orig_t	*orig;
  void		*next_pc;
  
  if (!(orig = alloc_orig(block)))
    {
      dr_printf("dr_malloc fail\n");
      return NULL;
    }
  ds_memset(orig, 0, sizeof(*orig));
  
  orig->size = size;
  orig->nb_hit = 1;
  orig->addr = pc;
  next_pc = decode_next_pc(drcontext, pc);
  orig->instr_size = next_pc - pc;
  orig->raw_instr = alloc_instr(block, orig->instr_size);
  copy_instr(orig->addr, orig->instr_size, orig->raw_instr);

  // get ctx isntruction to have information in structure recovery
  if (ctx->dr_addr)
    {
      orig->ctx_addr = ctx->addr;
      orig->ctx_instr_size = (void*)decode_next_pc(drcontext, ctx->dr_addr) - ctx->dr_addr;
      orig->raw_ctx_instr = alloc_instr(block, orig->ctx_instr_size);
      copy_instr(ctx->dr_addr, orig->ctx_instr_size, orig->raw_ctx_instr);
    }
  else
    {
      orig->ctx_addr = NULL;
      orig->ctx_instr_size = 0;
    }
  // get the start addr of the function doing the access
  get_caller_data(&(orig->start_func_addr),
		  &(orig->start_func_sym),
		  &(orig->module_name), drcontext, 0);

  return orig;
}

void incr_orig(access_t *access, size_t size, void *pc, void *drcontext,
	       malloc_t *block, ctx_t *ctx)
{
  orig_t	*orig_tree = search_same_addr_on_tree(access->origs, pc);
  orig_t	*orig_list = orig_tree;

  while (orig_list && orig_list->size != size)
    orig_list = orig_list->next;

  if (orig_list)
    {
      orig_list->nb_hit++;
      return;
    }

  if (!orig_tree)
    {
      if (!(orig_tree = new_orig(size, pc, drcontext, block, ctx)))
	{
	  dr_printf("dr_malloc fail\n");
	  return;
	}
      orig_tree->node.high_addr = pc;
      orig_tree->node.min_addr = pc;
      orig_tree->node.data = orig_tree;
      add_to_tree(&(access->origs), (tree_t*)orig_tree);

      return;
    }

  // if a an orig have the same addr but not the same size we create an other
  // entry and put it in the linked list for this node;
  if (!orig_list)
    {
      if (!(orig_list = new_orig(size, pc, drcontext, block, ctx)))
	{
	  dr_printf("dr_malloc fail\n");
	  return;
	}

      while (orig_tree->next)
	orig_tree = orig_tree->next;
      orig_tree->next = orig_list;
    }
}

access_t *get_access(size_t offset, tree_t **t_access, malloc_t *block)
{
  access_t      *access;

  if ((access = search_same_addr_on_tree(*t_access, (void *)offset)))
    return access;

  // if no access with this offset is found we create a new one
  if (!(access = alloc_access(block)))
    {
      dr_printf("dr_malloc fail\n");
      return NULL;
    }

  ds_memset(access, 0, sizeof(*access));
  access->offset = offset;

  access->node.data = access;
  access->node.high_addr = (void *)offset;
  access->node.min_addr = (void *)offset;

  add_to_tree(t_access, (tree_t*)access);

  return access;
}

void add_hit(void *pc, size_t size, void *target, int read, void *drcontext,
	     ctx_t *ctx)
{
  malloc_t	*block = search_on_tree(active_blocks, target);
  access_t	*access;

  // if the access is not on a malloc block we do nothing
  // we have to check if the target is not the and of the block
  // (because last offset is out of the user
  // data and use only for malloc's internal purposes)
  if (!block || target == block->end)
    return;

  dr_mutex_lock(lock);

  if (read)
    access = get_access(target - block->start, &(block->read), block);
  else
    access = get_access(target - block->start, &(block->write), block);

  access->total_hits++;
  incr_orig(access, size, pc, drcontext, block, ctx);

  dr_mutex_unlock(lock);  
}

void check_opnd(opnd_t opnd, void *pc, int read, void *drcontext,
		dr_mcontext_t *mctx, void *prev_pc)
{
  void *target;

  if (opnd_is_memory_reference(opnd) && opnd_is_base_disp(opnd))
    {
      target = (void *)reg_get_value(opnd_get_base(opnd), mctx);
      target += opnd_get_disp(opnd);
      target += reg_get_value(opnd_get_index(opnd), mctx) * opnd_get_scale(opnd);
      add_hit(pc, opnd_size_in_bytes(opnd_get_size(opnd)), target, read,
	      drcontext, prev_pc);
    }
  else if (opnd_is_memory_reference(opnd) && opnd_get_addr(opnd))
    add_hit(pc, opnd_size_in_bytes(opnd_get_size(opnd)), opnd_get_addr(opnd),
	    read, drcontext, prev_pc);
  // for now no other kind of memory reference was noticed to access heap data
  else if (opnd_is_memory_reference(opnd))
    dr_printf("need to implem other memory ref\n");
}

void memory_read(void *pc, void *next_pc)
{
  void		*drcontext = dr_get_current_drcontext();
  instr_t	*instr = instr_create(drcontext);
  dr_mcontext_t mctx;
  opnd_t	src;
  ctx_t		ctx;
  
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

  ctx.addr = next_pc;
  ctx.dr_addr = next_pc;

  for (int i = 0; i < instr_num_srcs(instr); i++)
    {
      src = instr_get_src(instr, i);
      check_opnd(src, pc, 1, drcontext, &mctx, &ctx);
    }
 
  instr_destroy(drcontext, instr);
}

void memory_write(void *pc, void *prev_pc)
{
  void		*drcontext = dr_get_current_drcontext();
  instr_t	*instr = instr_create(drcontext);
  dr_mcontext_t mctx;
  opnd_t	dst;
  ctx_t		ctx;
  
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

  ctx.addr = prev_pc;
  ctx.dr_addr = prev_pc;

  for (int i = 0; i < instr_num_dsts(instr); i++)
    {
      dst = instr_get_dst(instr, i);
      check_opnd(dst, pc, 0, drcontext, &mctx, &ctx);
    }

  instr_destroy(drcontext, instr);
}
