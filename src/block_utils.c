#include <string.h>
#include "dr_api.h"
#include "drwrap.h"
#include "../includes/utils.h"
#include "../includes/block_utils.h"

malloc_t *get_block_by_access(void *addr)
{
  malloc_t	*block = blocks;

  while (block)
    {
      if (block->start <= addr && block->end >= addr)
	return block;
      block = block->next;
    }
  return NULL;
}

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

malloc_t *add_block(size_t size)
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
      // if start == NULL on realloc set block to free to keep previous access to data
      else if (!(block->flag & FREE))
        {
          dr_printf("Realloc of size %d on %p failed\n", block->size, block->start);
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

void free_malloc_block(malloc_t *block)
{
  if (block)
    dr_global_free(block, sizeof(*block));
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
