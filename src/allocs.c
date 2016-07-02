#include "dr_api.h"
#include "dr_ir_opnd.h"
#include "drwrap.h"
#include "drmgr.h"
#include "../includes/utils.h"
#include "../includes/block_utils.h"
#include "../includes/call.h"
#include "../includes/sym.h"
#include "../includes/args.h"
#include "../includes/out_json.h"

static int malloc_init = 0;
static int realloc_init = 0;

// used to know when flush old_blocks
// list to limit memory usage
static int	old_blocks_count = 0;

// return the addr of the previous instructions
// a bad addr can be return in some specific case but generally work good
void *get_prev_instr_pc(void *pc, void *drc)
{
  instr_t	*instr = instr_create(drc);
  void		*tmp_pc;

  for (int ct = 1; ; ct++)
    {
      tmp_pc = dr_app_pc_for_decoding(pc - ct);
      if (decode(drc, tmp_pc, instr))
	{
	  if (instr_is_call(instr) && decode_next_pc(drc, tmp_pc) == pc)
	    break;
	}
      instr_reuse(drc, instr);
    }

  instr_destroy(drc, instr);

  return tmp_pc;
}

void pre_calloc(void *wrapctx, OUT void **user_data)
{
  void		*drc;
  
  drc = drwrap_get_drcontext(wrapctx);

  dr_mutex_lock(lock);

  if (!module_is_wrapped(drc))
    {
      dr_mutex_unlock(lock);
      return;
    }

  *user_data = add_block((size_t)drwrap_get_arg(wrapctx, 1) *
			 (size_t)drwrap_get_arg(wrapctx, 0),
			 get_prev_instr_pc(drwrap_get_retaddr(wrapctx), drc),
			 drc);

  dr_mutex_unlock(lock);
}

void post_calloc(void *wrapctx, void *user_data)
{
  malloc_t      *block = (malloc_t *)user_data;

  dr_mutex_lock(lock);

  if (block)
    set_addr_malloc(block, drwrap_get_retval(wrapctx), ALLOC, 0);

  dr_mutex_unlock(lock);
}

void pre_malloc(void *wrapctx, OUT void **user_data)
{
  void		*drc;

  drc = drwrap_get_drcontext(wrapctx);
  dr_mutex_lock(lock);

  // the first call on 64 bit and the second in 32bit
  // are init call, so we have to do nothing
#ifdef BUILD_64
  if (!malloc_init++ && !realloc_init)
    {
      dr_mutex_unlock(lock);
      return;
    }
#else
  if (malloc_init++ == 1 && realloc_init != 1)
    {
      dr_mutex_unlock(lock);
      return;
    }
#endif

  
  if (!module_is_wrapped(drc))
    {
      dr_mutex_unlock(lock);
      return;
    }
  
  *user_data = add_block((size_t)drwrap_get_arg(wrapctx, 0),
			 get_prev_instr_pc(drwrap_get_retaddr(wrapctx), drc),
			 drc);

  dr_mutex_unlock(lock);
}

void post_malloc(void *wrapctx, void *user_data)
{
  malloc_t      *block = (malloc_t *)user_data;

  dr_mutex_lock(lock);

  if (!block)
    {
      dr_mutex_unlock(lock);
      return;
    }

  set_addr_malloc(block, drwrap_get_retval(wrapctx), ALLOC, 0);

  dr_mutex_unlock(lock);
}

void pre_realloc(void *wrapctx, OUT void **user_data)
{
  malloc_t      *block;
  malloc_t	*new_block;
  realloc_tmp_t *tmp = NULL;
  void          *start = drwrap_get_arg(wrapctx, 0);
  size_t        size = (size_t)drwrap_get_arg(wrapctx, 1);
  void		*drc = drwrap_get_drcontext(wrapctx);

  dr_mutex_lock(lock);
  // the first call on 64 bit and the second in 32bit
  // are init call, so we have to do nothing
#ifdef BUILD_64
  if (!realloc_init)
    {
      realloc_init++;
      dr_mutex_unlock(lock);
      return;
    }
#else
  if (realloc_init++ == 1)
    {
      dr_mutex_unlock(lock);
      return;
    }
#endif

  // if size == 0, realloc call free
  if (!size)
    {
      // this can happen if a block is alloc by a non wrap module and realloc
      // on a wrapped one
      if (!(block = search_on_tree(active_blocks, start)))
	{
	  dr_mutex_unlock(lock);
	  return;
	}
      block->free_pc = get_prev_instr_pc(drwrap_get_retaddr(wrapctx), drc);
      get_caller_data(&(block->free_func_pc), &(block->free_func_sym),
		      &(block->free_module_name), drc, 1);
      dr_mutex_unlock(lock);
      return;
    }

  if (!(tmp = dr_global_alloc(sizeof(realloc_tmp_t))))
    {
      dr_printf("dr_malloc fail\n");
      dr_mutex_unlock(lock);
      return;
    }

  // if realloc is use like malloc save the size to set it on the post wrapping
  tmp->size = size;
  *user_data = tmp;

  // if start == 0, realloc call malloc
  if (!start)
    {
      // if a block is alloc by a wrapped function and realloc by
      // an unwrapped one we have to take the realloc
      // so when realloc is called to do a malloc is the only case
      // when we have to check if the module is wrapped
      if(!module_is_wrapped(drc))
	{
	  *user_data = NULL;
	  dr_global_free(tmp, sizeof(*tmp));
	  dr_mutex_unlock(lock);
	  return;
	}
      tmp->block = NULL;
      dr_mutex_unlock(lock);
      return;
    }
  
  // this can happen if the block is alloc by a non wrapped module
  if (!(block = search_on_tree(active_blocks, start)))
    {
      *user_data = NULL;
      dr_global_free(tmp, sizeof(*tmp));
      dr_mutex_unlock(lock);
      return;
    }
  else
    {
      del_from_tree(&active_blocks, start, NULL, false);
  
      if ((new_block = dr_custom_alloc(NULL, 0, sizeof(*new_block),
				       DR_MEMPROT_WRITE | DR_MEMPROT_READ, NULL)))
	{
	  block->flag |= FREE_BY_REALLOC;
	  block->free_pc = get_prev_instr_pc(drwrap_get_retaddr(wrapctx), drc);
	  get_caller_data(&(block->free_func_pc), &(block->free_func_sym),
			  &(block->free_module_name), drc, 1);
	  block->next = old_blocks;
	  old_blocks = block;

	  old_blocks_count++;
	  if (!args->console && old_blocks_count == MAX_OLD_BLOCKS)
	    {
	      flush_old_block();
	      old_blocks_count = 0;
	    }
      
	  ds_memset(new_block, 0, sizeof(*new_block));
	  new_block->flag |= ALLOC_BY_REALLOC;
	  block = new_block;
	}
      else
	dr_printf("fail alloc\n");
      block->size = size;
    }
  
  tmp->block = block;

  dr_mutex_unlock(lock);
}

void post_realloc(void *wrapctx, void *user_data)
{
  malloc_t	*block;
  void          *ret = drwrap_get_retval(wrapctx);
  void		*drc = drwrap_get_drcontext(wrapctx);;
  realloc_tmp_t	*data = user_data;
  
  // if user_data is not set realloc was called to do a free
  // or the call to realloc is an init call or the block
  // was alloc by a non wrap module
  if (data)
    {
      dr_mutex_lock(lock);
      if (data->block)
        {
	  block = data->block;
	  set_addr_malloc(block, ret, block->flag, 1);
	  // we dont set the alloc arg in get_caller_data
	  // because the post wrap happen after the return
	  get_caller_data(&(block->alloc_func_pc), &(block->alloc_func_sym),
			  &(block->alloc_module_name), drc, 0);
	  block->alloc_pc = get_prev_instr_pc(drwrap_get_retaddr(wrapctx),
					      drc);
	}
      // if realloc is use like a malloc set, we the size here
      // because malloc wrapper receive a null size
      else if ((block = search_on_tree(active_blocks, ret)))
	{
	  block->size = data->size;
	  block->end = block->start + block->size;
	}

      dr_global_free(user_data, sizeof(realloc_tmp_t));

      dr_mutex_unlock(lock);
    }
}

void pre_free(void *wrapctx, __attribute__((unused))OUT void **user_data)
{
  malloc_t	*block;
  void		*drc;
  void		*addr;
  
  // free(0) do nothing
  if (!(addr = drwrap_get_arg(wrapctx, 0)))
    return;

  drc = drwrap_get_drcontext(wrapctx);

  dr_mutex_lock(lock);

  if ((block = search_on_tree(active_blocks, addr)))
    {
      block->flag |= FREE;
      // can be set if free by realloc
      if (!(block->free_pc))
	{
	  block->free_pc = get_prev_instr_pc(drwrap_get_retaddr(wrapctx), drc);
	  get_caller_data(&(block->free_func_pc), &(block->free_func_sym),
			  &(block->free_module_name), drc, 1);
	}

      block->next = old_blocks;
      old_blocks = block;
      old_blocks_count++;

      del_from_tree(&active_blocks, block->start, NULL, false);

      if (!args->console && old_blocks_count == MAX_OLD_BLOCKS)
	{
	  flush_old_block();
	  old_blocks_count = 0;
	}
    }

  dr_mutex_unlock(lock);
}
