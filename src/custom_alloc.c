#include "dr_api.h"
#include "../includes/allocs.h"


access_t *alloc_access(malloc_t *block)
{
  access_page_t	*new_page;
  
  if (!(block->access_pages) ||
      block->access_pages->header.next_idx == MAX_IDX_ACCESS)
    {
      if (!(new_page = dr_custom_alloc(NULL, DR_ALLOC_NON_HEAP, PAGE_SIZE,
				       DR_MEMPROT_WRITE | DR_MEMPROT_READ , NULL)))
	return NULL;
      new_page->header.next_page = block->access_pages;
      new_page->header.next_idx = 0;
      block->access_pages = new_page;
    }
  return &(block->access_pages->accesses[block->access_pages->header.next_idx++]);
}

orig_t *alloc_orig(malloc_t *block)
{
  orig_page_t	*new_page;
  
  if (!(block->orig_pages) ||
      block->orig_pages->header.next_idx == MAX_IDX_ORIG)
    {
      if (!(new_page = dr_custom_alloc(NULL, DR_ALLOC_NON_HEAP, PAGE_SIZE,
				       DR_MEMPROT_WRITE | DR_MEMPROT_READ, NULL)))
	return NULL;
      new_page->header.next_page = block->orig_pages;
      new_page->header.next_idx = 0;
      block->orig_pages = new_page;
    }

  return &(block->orig_pages->origs[block->orig_pages->header.next_idx++]);
}

void custom_free_pages(malloc_t *block)
{
  access_page_t	*tmp_a;
  orig_page_t	*tmp_o;

  while (block->access_pages)
    {
      tmp_a = block->access_pages;
      block->access_pages = block->access_pages->header.next_page;
      dr_custom_free(NULL, DR_ALLOC_NON_HEAP, tmp_a, PAGE_SIZE);
    }
  while (block->orig_pages)
    {
      tmp_o = block->orig_pages;
      block->orig_pages = block->orig_pages->header.next_page;
      dr_custom_free(NULL, DR_ALLOC_NON_HEAP, tmp_o, PAGE_SIZE);
    }
}

