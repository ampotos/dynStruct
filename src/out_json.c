#include "dr_api.h"
#include "../includes/allocs.h"
#include "../includes/tree.h"
#include "../includes/args.h"
#include "../includes/block_utils.h"

#define NULL_STR(s) ((s) ? (s) : "")

void print_orig_json(orig_t *orig)
{
  orig_t        *tmp;

  while (orig)
    {
      dr_fprintf(args->file_out, "{\"size_access\":%lu, \"nb_access\":%lu, ",
		 orig->size, orig->nb_hit);

      dr_fprintf(args->file_out,
		 "\"pc\":%lu, \"func_pc\":%lu, \"func_sym\":\"%s\", ",
		 orig->addr, orig->start_func_addr,
		 NULL_STR(orig->start_func_sym));
      dr_fprintf(args->file_out, "\"func_module\":\"%s\"}, ",
		 NULL_STR(orig->module_name));
		 
      tmp = orig->next;
      dr_global_free(orig, sizeof(*orig));
      orig = tmp;
    }
}

void print_access_json(access_t *access)
{
  dr_fprintf(args->file_out, "{\"offset\":%lu, \"total_access\" : %lu, ",
	     access->offset, access->total_hits);
  
  dr_fprintf(args->file_out, "\"details\":[");
  clean_tree(&(access->origs), (void (*)(void *))print_orig_json);
  dr_fprintf(args->file_out, "{}]}, ");

  dr_global_free(access, sizeof(*access));
}

void print_block_json(malloc_t *block)
{
  dr_fprintf(args->file_out,
	     "{\"start\":%lu, \"end\":%lu, \"size\":%lu, ",
	     block->start, block->end, block->size);

  dr_fprintf(args->file_out,
	     "\"free\":%d, \"alloc_by_realloc\":%d, \"free_by_realloc\":%d, ",
	     (block->flag & FREE) != 0, (block->flag & ALLOC_BY_REALLOC) != 0,
	     (block->flag & FREE_BY_REALLOC) != 0);

  dr_fprintf(args->file_out,
	     "\"alloc_pc\":%lu, \"alloc_func\":%lu, \"alloc_sym\":\"%s\", ",
	     block->alloc_pc, block->alloc_func_pc,
	     NULL_STR(block->alloc_func_sym));
  dr_fprintf(args->file_out,
	     "\"alloc_module\":\"%s\", ", NULL_STR(block->alloc_module_name));

  dr_fprintf(args->file_out,
	     "\"free_pc\":%lu, \"free_func\":%lu, \"free_sym\":\"%s\", ",
	     block->free_pc, block->free_func_pc,
	     NULL_STR(block->free_func_sym));
  dr_fprintf(args->file_out,
	     "\"free_module\":\"%s\", ", NULL_STR(block->free_module_name));

  dr_fprintf(args->file_out, "\"read_access\" : [");
  clean_tree(&(block->read), (void (*)(void*))print_access_json);
  dr_fprintf(args->file_out, "{}], ");
  
  dr_fprintf(args->file_out, "\"write_access\" : [");
  clean_tree(&(block->write), (void (*)(void*))print_access_json);
  dr_fprintf(args->file_out, "{}]");

  dr_fprintf(args->file_out, "}, ");
  dr_global_free(block, sizeof(*block));
}

void write_json(void)
{
  malloc_t      *tmp;
  
  while (old_blocks)
    {
      tmp = old_blocks->next;
      print_block_json(old_blocks);
      old_blocks = tmp;
    }
  clean_tree(&active_blocks, (void (*)(void*))print_block_json);

  dr_fprintf(args->file_out, "{}]");
}

void flush_old_block(void)
{
  malloc_t      *tmp;

  while (old_blocks)
    {
      tmp = old_blocks->next;
      print_block_json(old_blocks);
      old_blocks = tmp;
    }

  dr_flush_file(args->file_out);
}
