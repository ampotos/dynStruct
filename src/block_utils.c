#include "dr_api.h"
#include "drwrap.h"
#include "../includes/utils.h"
#include "../includes/block_utils.h"
#include "../includes/sym.h"
#include "../includes/call.h"
#include "../includes/out.h"

malloc_t *add_block(size_t size, void *pc, void *drcontext)
{
  malloc_t      *new = dr_global_alloc(sizeof(*new));

  if (!new)
    dr_printf("dr_malloc fail\n");
  else
    {
      ds_memset(new, 0, sizeof(*new));
      new->size = size;
      new->alloc_pc = pc;
      get_caller_data(&(new->alloc_func_pc),
		      &(new->alloc_func_sym),
		      &(new->alloc_module_name), drcontext, 1);
    }

  return new;
}

void set_addr_malloc(malloc_t *block, void *start, unsigned int flag,
		     int realloc)
{
  if (!start && block)
    {
      if (!realloc)
        {
          dr_printf("alloc of size %d failed\n", block->size);
	  dr_global_free(block, sizeof(*block));
        }
      else if (!(block->flag & FREE))
        {
          dr_printf("Realloc of size %d on %p failed\n",
		    block->size, block->start);
          block->flag |= FREE;
	  block->next = old_blocks;
	  old_blocks = block;

	  del_from_tree(&active_blocks, block->start, NULL, false);
        }
    }
  else if (block)
    {
      block->start = start;
      block->end = block->start + block->size;
      block->flag = flag;

      block->node.min_addr = block->start;
      block->node.high_addr = block->end;
      block->node.data = block;
      add_to_tree(&active_blocks, (tree_t *)block);
    }
  else
    dr_printf("Error : *alloc post wrapping call without pre wrapping\n");
}



