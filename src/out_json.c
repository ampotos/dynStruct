#include "dr_api.h"
#include "../includes/allocs.h"
#include "../includes/tree.h"
#include "../includes/args.h"
#include "../includes/block_utils.h"
#include "../includes/out_json.h"

#define NULL_STR(s) ((s) ? (s) : "")

char    *global_buf = NULL;
char    *tmp_buf = NULL;
int     global_idx = 0;
int      len_tmp = 0;

void init_buffering()
{
  DR_ASSERT((global_buf = dr_custom_alloc(NULL, DR_ALLOC_NON_HEAP | DR_ALLOC_NON_DR, GLOBAL_BUF_SIZE,
					  DR_MEMPROT_WRITE | DR_MEMPROT_READ , NULL)));
  DR_ASSERT((tmp_buf = dr_custom_alloc(NULL, DR_ALLOC_NON_HEAP | DR_ALLOC_NON_DR, TMP_BUF_SIZE,
				       DR_MEMPROT_WRITE | DR_MEMPROT_READ , NULL)));
}

void stop_buffering()
{
  dr_write_file(args->file_out, global_buf, global_idx);
  dr_custom_free(NULL, DR_ALLOC_NON_HEAP | DR_ALLOC_NON_DR, global_buf, GLOBAL_BUF_SIZE);
  dr_custom_free(NULL, DR_ALLOC_NON_HEAP | DR_ALLOC_NON_DR, tmp_buf, TMP_BUF_SIZE);
}

void print_orig_json(orig_t *orig)
{
  orig_t        *tmp;

  while (orig)
    {
      DS_PRINTF("{\"size_access\":%lu, \"nb_access\":%lu, ",
		 orig->size, orig->nb_hit);
      DS_PRINTF(
		 "\"pc\":%lu, \"func_pc\":%lu, \"func_sym\":\"%s\", ",
		 orig->addr, orig->start_func_addr,
		 NULL_STR(orig->start_func_sym));
      DS_PRINTF("\"func_module\":\"%s\", \"opcode\":\"",
		 NULL_STR(orig->module_name));
      for (unsigned int size = 0; size < orig->instr_size; size++)
	{
	  DS_PRINTF("%02x", orig->raw_instr[size]);
	}
      DS_PRINTF("\", \"ctx_addr\":%lu, \"ctx_opcode\":\"",
		 orig->ctx_addr);
      for (unsigned int size = 0; size < orig->ctx_instr_size; size++)
	{
	  DS_PRINTF("%02x", orig->raw_ctx_instr[size]);
	}
      DS_PRINTF("\"}, ");

      tmp = orig->next;
      orig = tmp;
    }
}

void print_access_json(access_t *access)
{
  DS_PRINTF("{\"offset\":%lu, \"total_access\" : %lu, ",
	     access->offset, access->total_hits);
  
  DS_PRINTF("\"details\":[");
  clean_tree(&(access->origs), (void (*)(void *))print_orig_json, false);
  DS_PRINTF("{}]}, ");
}

void print_block_json(malloc_t *block)
{
  DS_PRINTF("{\"start\":%lu, \"end\":%lu, \"size\":%lu, ",
	     block->start, block->end, block->size);

  DS_PRINTF("\"free\":%d, \"alloc_by_realloc\":%d, \"free_by_realloc\":%d, ",
	     (block->flag & FREE) != 0, (block->flag & ALLOC_BY_REALLOC) != 0,
	     (block->flag & FREE_BY_REALLOC) != 0);

  DS_PRINTF("\"alloc_pc\":%lu, \"alloc_func\":%lu, \"alloc_sym\":\"%s\", ",
	     block->alloc_pc, block->alloc_func_pc,
	     NULL_STR(block->alloc_func_sym));
  DS_PRINTF("\"alloc_module\":\"%s\", ", NULL_STR(block->alloc_module_name));

  DS_PRINTF("\"free_pc\":%lu, \"free_func\":%lu, \"free_sym\":\"%s\", ",
	     block->free_pc, block->free_func_pc,
	     NULL_STR(block->free_func_sym));
  DS_PRINTF("\"free_module\":\"%s\", ", NULL_STR(block->free_module_name));

  DS_PRINTF("\"read_access\" : [");
  clean_tree(&(block->read), (void (*)(void*))print_access_json, false);
  DS_PRINTF("{}], ");
  
  DS_PRINTF("\"write_access\" : [");
  clean_tree(&(block->write), (void (*)(void*))print_access_json, false);
  DS_PRINTF("{}]");

  DS_PRINTF("}, ");

  custom_free_pages(block);
  dr_custom_free(NULL, 0, block, sizeof(*block));
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

}

void write_json(void)
{
  flush_old_block();
  clean_tree(&active_blocks, (void (*)(void*))print_block_json, false);

  DS_PRINTF("{}]}");
  stop_buffering();
}
