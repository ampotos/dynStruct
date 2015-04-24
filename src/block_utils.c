#include "dr_api.h"
#include "drwrap.h"
#include "../includes/utils.h"
#include "../includes/block_utils.h"

malloc_t *get_block_by_addr(void *addr)
{
  malloc_t      *block = blocks;

  while(block)
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
  module_data_t *m_data;

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
      new->start = NULL;
      new->end = NULL;
      new->size = size;
      new-> flag = 0;
      new->next = NULL;
    }

  return new;
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
