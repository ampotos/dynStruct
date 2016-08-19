#include "dr_api.h"
#include "../includes/allocs.h"

void *new_page(void *old_pages)
{
  page_header_t	*new_page;

  if (!(new_page = dr_custom_alloc(NULL, DR_ALLOC_NON_HEAP | DR_ALLOC_NON_DR, PAGE_SIZE,
				   DR_MEMPROT_WRITE | DR_MEMPROT_READ , NULL)))
    return NULL;
  new_page->next_page = old_pages;
  new_page->next_idx = 0;
  return new_page;
}

void *alloc_instr(malloc_t *block, unsigned int size)
{
  unsigned int	idx;

  if (!(block->instr_pages) ||
      block->instr_pages->header.next_idx + size >= MAX_IDX_INSTR)
    {
      if (!(block->instr_pages = new_page(block->instr_pages)))
	return NULL;
    }
  idx = block->instr_pages->header.next_idx;
  block->instr_pages->header.next_idx += size;
  return &(block->instr_pages->instrs[idx]);
}

access_t *alloc_access(malloc_t *block)
{
  if (!(block->access_pages) ||
      block->access_pages->header.next_idx == MAX_IDX_ACCESS)
    {
      if (!(block->access_pages = new_page(block->access_pages)))
	return NULL;
    }
  return &(block->access_pages->accesses[block->access_pages->header.next_idx++]);
}

void free_last_access(malloc_t *block)
{
  block->access_pages->header.next_idx--;
}

orig_t *alloc_orig(malloc_t *block)
{
  if (!(block->orig_pages) ||
      block->orig_pages->header.next_idx == MAX_IDX_ORIG)
    {
      if (!(block->orig_pages = new_page(block->orig_pages)))
	return NULL;
    }

  return &(block->orig_pages->origs[block->orig_pages->header.next_idx++]);
}

void free_last_orig(malloc_t *block)
{
  block->orig_pages->header.next_idx--;
}

void custom_free_pages(malloc_t *block)
{
  access_page_t	*tmp_a;
  orig_page_t	*tmp_o;
  instr_page_t	*tmp_i;

  while (block->access_pages)
    {
      tmp_a = block->access_pages;
      block->access_pages = block->access_pages->header.next_page;
      dr_custom_free(NULL, DR_ALLOC_NON_HEAP | DR_ALLOC_NON_DR, tmp_a, PAGE_SIZE);
    }
  while (block->orig_pages)
    {
      tmp_o = block->orig_pages;
      block->orig_pages = block->orig_pages->header.next_page;
      dr_custom_free(NULL, DR_ALLOC_NON_HEAP | DR_ALLOC_NON_DR, tmp_o, PAGE_SIZE);
    }
  while (block->instr_pages)
    {
      tmp_i = block->instr_pages;
      block->instr_pages = block->instr_pages->header.next_page;
      dr_custom_free(NULL, DR_ALLOC_NON_HEAP | DR_ALLOC_NON_DR, tmp_i, PAGE_SIZE);
    }
}

