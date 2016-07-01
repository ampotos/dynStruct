#include "dr_api.h"
#include "../includes/allocs.h"
#include "../includes/block_utils.h"
#include "../includes/args.h"
#include "../includes/out_json.h"

void free_orig(orig_t *orig)
{
  orig_t        *tmp;

  while (orig)
    {
      dr_printf("\t\t\t %d bytes were accessed by %p (%s : %p in %s, opcode: ",
		orig->size, orig->addr, orig->start_func_sym,
		orig->start_func_addr, orig->module_name);
      for (unsigned int size = 0; size < orig->instr_size; size++)
	dr_printf("%02x", orig->raw_instr[size]);
      dr_printf(") %d times\n", orig->nb_hit);
      tmp = orig->next;
      orig = tmp;
    }
}

void free_access(access_t *access)
{
  dr_printf("\t was access at offset %d (%lu times)\n", access->offset,
	    access->total_hits);
  dr_printf("\tdetails :\n");

  clean_tree(&(access->origs), (void (*)(void *))free_orig, false);
}

void print_block(malloc_t *block)
{
  dr_printf("block : %p-%p(0x%x) ", block->start, block->end, block->size);
  if (block->flag & FREE)
    dr_printf("was free\n");
  else if (block->flag & FREE_BY_REALLOC)
    dr_printf("was realloc\n");
  else
    dr_printf("was not free\n");
  
  if (block->flag & ALLOC)
    dr_printf("alloc by %p(%s : %p in %s) ",
	      block->alloc_pc, block->alloc_func_sym,
	      block->alloc_func_pc, block->alloc_module_name);
  else if (block->flag & ALLOC_BY_REALLOC)
    dr_printf("alloc (via realloc) by %p(%s : %p in %s) ",
	      block->alloc_pc, block->alloc_func_sym,
	      block->alloc_func_pc, block->alloc_module_name);
  
  if (block->flag & FREE)
    dr_printf("and free by %p(%s : %p in %s)\n",
	      block->free_pc, block->free_func_sym,
	      block->free_func_pc, block->free_module_name);
  else if (block->flag & FREE_BY_REALLOC)
    dr_printf("and free (via a realloc) by %p(%s : %p in %s)\n",
	      block->free_pc, block->free_func_sym,
	      block->free_func_pc, block->free_module_name);
  else
    dr_printf("\n");
  
  if (block->read)
    {
      dr_printf("\t READ :\n");
      clean_tree(&(block->read), (void (*)(void*))free_access, false);
    }
  if (block->write)
    {
      dr_printf("\t WRITE :\n");
      clean_tree(&(block->write), (void (*)(void*))free_access, false);
    }

  custom_free_pages(block);
  dr_custom_free(NULL, 0, block, sizeof(*block));
}

void print_console(void)
{
  malloc_t      *tmp;

  while (old_blocks)
    {
      tmp = old_blocks->next;
      print_block(old_blocks);
      old_blocks = tmp;
    }
  clean_tree(&active_blocks, (void (*)(void*))print_block, false);
}

void output(void)
{
  if (args->console)
    print_console();
  else
    {
      write_json();
      dr_close_file(args->file_out);
    }
}
	 
