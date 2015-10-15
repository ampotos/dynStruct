#include "dr_api.h"
#include "../includes/allocs.h"
#include "../includes/block_utils.h"

// maybe call drsym_demangle_symbol to have clean cpp sym demangle

void print_orig(orig_t *orig)
{
  while (orig)
    {
      dr_printf("\t\t\t %d bytes was accessed by %p (%s : %p) %d times\n", orig->size,
		orig->addr, orig->start_func_sym, orig->start_func_addr, orig->nb_hit);
      orig  = orig->next;
    }
}

void print_access(malloc_t *block)
{
  dr_printf("\t READ :\n");
  clean_tree(&(block->read), (void (*)(void*))free_access);
  dr_printf("\t WRITE :\n");
  clean_tree(&(block->write), (void (*)(void*))free_access);
}

void print_block(malloc_t *block)
{
      dr_printf("block : %p-%p(0x%x) ", block->start, block->end, block->size);
      if (block->flag & FREE)
	dr_printf("was free\nalloc by %p(%s : %p) and free by %p(%s : %p)\n",
		  block->alloc_pc, block->alloc_func_sym,
		  block->alloc_func_pc, block->free_pc,
		  block->free_func_sym, block->free_func_pc);
      else
	dr_printf("was not free\nalloc by %p\n", block->alloc_pc); 
  if (block->read || block->write)
    print_access(block);
  free_malloc_block(block);
}

void process_recover(void)
{
  malloc_t      *tmp;

  while (old_blocks)
    {
      tmp = old_blocks->next;
      print_block(old_blocks);
      old_blocks = tmp;
    }
  clean_tree(&active_blocks, (void (*)(void*))print_block);
}
