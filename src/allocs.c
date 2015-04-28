#include "dr_api.h"
#include "dr_ir_opnd.h"
#include "drwrap.h"
#include "../includes/utils.h"
#include "../includes/block_utils.h"

static int       first = 1;

void pre_malloc(void *wrapctx, OUT void **user_data)
{
  if (first) // ugly fix to the double call of first *alloc                                                                           
    {
      first = 0;
      return;
    }

  dr_mutex_lock(lock);

  *user_data = add_block((size_t)drwrap_get_arg(wrapctx, 0));

  dr_mutex_unlock(lock);
}

void post_malloc(void *wrapctx, void *user_data)
{
  malloc_t      *block = (malloc_t *)user_data;

  dr_mutex_lock(lock);

  if (block)
    set_addr_malloc(block, drwrap_get_retval(wrapctx), ALLOC, 0);

  dr_mutex_unlock(lock);
}

void pre_realloc(void *wrapctx, OUT void **user_data)
{
  malloc_t      *block;
  realloc_tmp_t *tmp = NULL;
  void          *start = drwrap_get_arg(wrapctx, 0);
  size_t        size = (size_t)drwrap_get_arg(wrapctx, 1);

  if (first) // ugly fix to the double call of first *alloc                                                                           
    {
      first = 0;
      return;
    }

  // if size == 0 => realloc call free                                                                                                
  if (!size)
    return;

  dr_mutex_lock(lock);
  if (!(tmp = dr_global_alloc(sizeof(realloc_tmp_t))))
    {
      dr_printf("dr_malloc fail\n");
      return;
    }

  // is realloc is use like a malloc save the to set it on the post wrapping                                                          
  tmp->size = size;
  *user_data = tmp;

  // if start == 0 => realloc call malloc                                                                                             
  if (!start)
    {
      tmp->block = NULL;
      dr_mutex_unlock(lock);
      return;
    }

  if (!(block = get_block_by_addr(start)))
    dr_printf("Realloc on %p error : addr was not previously malloc\n", start);
  else
    block->size = size;
  tmp->block = block;

  dr_mutex_unlock(lock);
}

void post_realloc(void *wrapctx, void *user_data)
{
  malloc_t      *block;
  void          *ret = drwrap_get_retval(wrapctx);

  dr_mutex_lock(lock);

  // if user_data is not set realloc was called to do a free                                                                          
  if (user_data)
    {
      if (((realloc_tmp_t *)user_data)->block)
        set_addr_malloc(((realloc_tmp_t *)user_data)->block, ret, ((realloc_tmp_t *)user_data)->block->flag, 1);
      // if realloc is use like a malloc set the size (malloc wrapper receive a null size)                                            
      else if ((block = get_block_by_addr(ret)))
        block->size = ((realloc_tmp_t*)user_data)->size;
      dr_global_free(user_data, sizeof(realloc_tmp_t));
    }

  dr_mutex_unlock(lock);
}

void pre_free(void *wrapctx, __attribute__((unused))OUT void **user_data)
{
  malloc_t      *block;

  // free(0) du nothing                                                                                                               
  if (!drwrap_get_arg(wrapctx,0))
    return;

  dr_mutex_lock(lock);

  block = get_block_by_addr(drwrap_get_arg(wrapctx, 0));

  // if the block was previously malloc we set it to free                                                                             
  if (block)
    block->flag |= FREE;
  else
    dr_printf("free of non alloc adress : %p\n", drwrap_get_arg(wrapctx, 0));

  dr_mutex_unlock(lock);
}
