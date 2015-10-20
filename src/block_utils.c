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
  
  if ((access = search_on_tree(*t_access, (void *)offset)))
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
    {
      dr_printf("dr_malloc fail\n");
      return NULL;
    }
  else
    {
      ds_memset(new, 0, sizeof(*new));
      new->size = size;
      new->alloc_pc = pc;
      get_caller_data(&new->alloc_func_pc,
		      &new->alloc_func_sym, drcontext, 1);
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
	  free_malloc_block(block);
        }
      // if start == NULL on realloc this is a free
      // set block to free to keep previous access to data
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

void free_orig(orig_t *orig)
{
  orig_t	*tmp;

  while (orig)
    {
      dr_printf("\t\t\t %d bytes was accessed by %p (%s : %p) %d times\n", orig->size,
		orig->addr, orig->start_func_sym, orig->start_func_addr, orig->nb_hit);
      tmp = orig->next;
      dr_global_free(orig, sizeof(*orig));
      orig = tmp;
    }
}

void free_access(access_t *access)
{
  dr_printf("\t was access at offset %d (%lu times)\n", access->offset,
	    access->total_hits);
  dr_printf("\tdetails :\n");
  /* print_orig(access->origs); */
  /* free_orig(access->origs); */
  clean_tree(&(access->origs), (void (*)(void *))free_orig);
  dr_global_free(access, sizeof(*access));
}

void free_malloc_block(malloc_t *block)
{
  if (block)
    {
      // actually cleaning is done at printing time
      // todo if not print clean here
      /* clean_tree(&(block->read), (void (*)(void *))free_access); */
      /* clean_tree(&(block->write), (void (*)(void *))free_access); */
      dr_global_free(block, sizeof(*block));
    }
}
