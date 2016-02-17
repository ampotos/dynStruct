#include "dr_api.h"
#include "drwrap.h"
#include "../includes/utils.h"
#include "../includes/block_utils.h"
#include "../includes/sym.h"
#include "../includes/call.h"
#include "../includes/out.h"

access_t *get_access(size_t offset, tree_t **t_access)
{
  access_t	*access;
  tree_t	*new_node;
  
  if ((access = search_same_addr_on_tree(*t_access, (void *)offset)))
    return access;

  // if no access with this offset is found we create a new one
  if (!(new_node = dr_global_alloc(sizeof(*new_node))))
    {
      dr_printf("dr_malloc fail\n");
      return NULL;
    }
  if (!(access = dr_global_alloc(sizeof(*access))))
    {
      dr_printf("dr_malloc fail\n");
      dr_global_free(new_node, sizeof(*new_node));
      return NULL;
    }
  
  ds_memset(access, 0, sizeof(*access));
  access->offset = offset;

  new_node->data = access;
  new_node->high_addr = (void *)offset;
  new_node->min_addr = (void *)offset;

  add_to_tree(t_access, new_node);

  return access;
}

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
  tree_t	*new_node;

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

	  del_from_tree(&active_blocks, block->start, NULL);
        }
    }
  else if (block)
    {
      block->start = start;
      block->end = block->start + block->size;
      block->flag = flag;

      if (!(new_node = dr_global_alloc(sizeof(*new_node))))
	{
	  dr_printf("Can't alloc\n");
	  dr_global_free(block, sizeof(*block));
	  return ;
	}
      new_node->min_addr = block->start;
      new_node->high_addr = block->end;
      new_node->data = block;
      add_to_tree(&active_blocks, new_node);
    }
  else
    dr_printf("Error : *alloc post wrapping call without pre wrapping\n");
}



