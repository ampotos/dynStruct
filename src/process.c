#include "dr_api.h"
#include "../includes/allocs.h"
#include "../includes/block_utils.h"

// maybe call drsym_demangle_symbol to have clean cpp sym demangle

void	print_orig(orig_t *orig, const char *type)
{
  while (orig)
    {
      dr_printf("\t\t\t %d bytes was %s by %p (%s : %p) %d times\n", orig->size, type,
		orig->addr, orig->start_func_sym, orig->start_func_addr, orig->nb_hit);
      orig  = orig->next;
    }
}

void	print_access(malloc_t *block)
{
  access_t	*access_read = block->read;
  access_t	*access_write = block->write;

  while (access_read)
    {
      dr_printf("\t was read at offset %d (%lu times)\n", access_read->offset,
		access_read->total_hits);
      dr_printf("\tdetails :\n");
      print_orig(access_read->origs, "read");
      access_read = access_read->next;
    }

  while (access_write)
    {
      dr_printf("\t was write at offset %d (%lu times)\n", access_write->offset,
		access_write->total_hits);
      dr_printf("\tdetails :\n");
      print_orig(access_write->origs, "write");
      access_write = access_write->next;
    }
}

void	process_recover(void)
{
  malloc_t      *block = blocks;
  malloc_t      *tmp;

  while (block)
    {
      tmp = block->next;
      if (block->read || block->write)
      	{
	  dr_printf("block : %p-%p(0x%x) ", block->start, block->end, block->size);
	  if (block->flag & FREE)
	    dr_printf("was free\nalloc by %p(%s : %p) and free by %p(%s : %p)\n",
		      block->alloc_pc, block->alloc_func_sym,
		      block->alloc_func_pc, block->free_pc,
		      block->free_func_sym, block->free_func_pc);
	  else
	    dr_printf("was not free\nalloc by %p\n", block->alloc_pc);
	  print_access(block);
	}
      free_malloc_block(block);
      block = tmp;
    }
  blocks = NULL;
}
