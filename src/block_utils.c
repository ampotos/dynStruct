#include "dr_api.h"
#include "drwrap.h"
#include "../includes/utils.h"
#include "../includes/block_utils.h"
#include "../includes/sym.h"

access_t *get_access(size_t offset, access_t **l_access)
{
  access_t	*access = *l_access;

  while (access)
    {
      if (access->offset == offset)
	return access;
      access = access->next;
    }
  
  // if no access with this offset is found we create a new one
  if (!(access = dr_global_alloc(sizeof(*access))))
    {
      dr_printf("dr_malloc fail\n");
      return NULL;
    }
  
  ds_memset(access, 0, sizeof(*access));
  access->offset = offset;
  access->next = *l_access;
  *l_access = access;
  return access;
}

malloc_t *add_block(size_t size, void *pc, void *start_pc)
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
      new->alloc_func_pc = start_pc;
      new->alloc_func_sym = hashtable_lookup(sym_hashtab, start_pc);
    }

  return new;

  /* malloc_t new = blocks; */
  /* if (new) */
  /*   { */
  /*     while (new->next) */
  /*       new = new->next; */
  /*     if (!(new->next = dr_global_alloc(sizeof(*new)))) */
  /*       dr_printf("dr_malloc fail\n"); */
  /*     new = new->next; */
  /*   } */
  /* else */
  /*   if (!(blocks = dr_global_alloc(sizeof(*new)))) */
  /*     dr_printf("dr_malloc fail\n"); */
  /*   else */
  /*     new = blocks; */

  /* if (new) */
  /*   { */
  /*     memset(new, 0, sizeof(*new)); */
  /*     new->size = size; */
  /*     new->alloc_pc = pc; */
  /*     new->alloc_func_pc = start_pc; */
  /*     new->alloc_func_sym = hashtable_lookup(sym_hashtab, start_pc); */
  /*   } */
  /* return new; */
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
          /* remove_block(block->data); */
	  dr_global_free(block, sizeof(*block));
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

      if (!realloc)
	{
	  if (!(new_node = dr_global_alloc(sizeof(*new_node))))
	    {
	      dr_printf("Can't malloc\n");
	      dr_global_free(block, sizeof(*block));
	      return ;
	    }
	  new_node->min_addr = block->start;
	  new_node->high_addr = block->end;
	  new_node->data = block;
	  add_to_tree(&active_blocks, new_node);
	}
    }
  else
    dr_printf("Error : *alloc post wrapping call without pre wrapping\n");
}

void free_orig(orig_t *orig)
{
  orig_t	*tmp;

  while (orig)
    {
      tmp = orig->next;
      dr_global_free(orig, sizeof(*orig));
      orig = tmp;
    }
}

void free_access(access_t *access)
{
  access_t	*tmp;

  while (access)
    {
      tmp = access->next;
      free_orig(access->origs);
      dr_global_free(access, sizeof(*access));
      access = tmp;
    }
}

void free_malloc_block(malloc_t *block)
{
  if (block)
    {
      free_access(block->read);
      free_access(block->write);
      dr_global_free(block, sizeof(*block));
    }
}

/* void remove_block(malloc_t *block) */
/* { */
/*   malloc_t      *tmp = blocks; */
 
/*   if (tmp == block) */
/*     { */
/*       blocks = tmp->next; */
/*       free_malloc_block(block); */
/*     } */
/*   else */
/*     { */
/*       while (tmp) */
/*         { */
/*           if (tmp->next == block) */
/*             { */
/*               tmp->next = tmp->next->next; */
/*               free_malloc_block(block); */
/*               break; */
/*             } */
/*           tmp = tmp->next; */
/*         } */
/*     } */
/* } */
