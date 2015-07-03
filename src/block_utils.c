#include <string.h>
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
  
  memset(access, 0, sizeof(*access));
  access->offset = offset;
  access->next = *l_access;
  *l_access = access;
  return access;
}

// todo : when block are store in tree, this has to  search in tree
malloc_t *get_active_block_by_access(void *addr)
{
  malloc_t	*block = blocks;

  while (block)
    {
      if (!(block->flag & FREE) && block->start <= addr && block->end >= addr)
	return block;
      block = block->next;
    }
  return NULL;
}

// todo : when block are store in tree, this has to  search in tree
malloc_t *get_block_by_addr(void *addr)
{
  malloc_t      *block = blocks;

  while (block)
    {
      if (block->start == addr)
        return block;
      block = block->next;
    }
  return NULL;
}

// todo : when block are store in tree, this has to add in tree
malloc_t *add_block(size_t size, void *pc, void *start_pc)
{
  malloc_t      *new = blocks;

  if (new)
    {
      while (new->next)
        new = new->next;
      if (!(new->next = dr_global_alloc(sizeof(*new))))
        dr_printf("dr_malloc fail\n");
      new = new->next;
    }
  else
    if (!(blocks = dr_global_alloc(sizeof(*new))))
      dr_printf("dr_malloc fail\n");
    else
      new = blocks;

  if (new)
    {
      memset(new, 0, sizeof(*new));
      new->size = size;
      new->alloc_pc = pc;
      new->alloc_func_pc = start_pc;
      new->alloc_func_sym = hashtable_lookup(sym_hashtab, start_pc);
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
          remove_block(block);
        }
      // if start == NULL on realloc 
      // set block to free to keep previous access to data
      else if (!(block->flag & FREE))
        {
          dr_printf("Realloc of size %d on %p failed\n",
		    block->size, block->start);
          block->flag |= FREE;
        }
    }
  else if (block)
    {
      block->start = start;
      block->end = block->start + block->size;
      block->flag = flag;
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

void remove_block(malloc_t *block)
{
  malloc_t      *tmp = blocks;
 
  if (tmp == block)
    {
      blocks = tmp->next;
      free_malloc_block(block);
    }
  else
    {
      while (tmp)
        {
          if (tmp->next == block)
            {
              tmp->next = tmp->next->next;
              free_malloc_block(block);
              break;
            }
          tmp = tmp->next;
        }
    }
}
