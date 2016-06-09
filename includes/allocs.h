#ifndef ALLOCS_H_
#define ALLOCS_H_

#include "tree.h"

// store nb_hit and size of the hit by pc on access instruction
typedef struct orig_s
{
  tree_t	node;
  // next is use if an orig have the same addr but not the same access size
  struct orig_s	*next;
  size_t	size;
  size_t	nb_hit;
  void		*addr;
  void		*start_func_addr;
  char		*start_func_sym;
  const char	*module_name;
} orig_t;

typedef struct access_s
{
  tree_t		node;
  size_t		offset;
  size_t		total_hits;
  tree_t		*origs;
} access_t;

// include here because it need access_t and orig_t declaration
#include "custom_alloc.h"

typedef struct malloc_s
{
  tree_t		node;
  struct malloc_s	*next;
  void			*start;
  void			*end;
  size_t		size;
  tree_t		*read;
  tree_t		*write;
  unsigned int		flag;
  void			*alloc_pc;
  void			*alloc_func_pc;
  char			*alloc_func_sym;
  const char		*alloc_module_name;
  void			*free_pc;
  void			*free_func_pc;
  char			*free_func_sym;
  const char		*free_module_name;
  access_page_t		*access_pages;
  orig_page_t		*orig_pages;
} malloc_t;

// define flag for malloc_t
#define ALLOC 0x1
#define FREE (0x1 << 1)
#define ALLOC_BY_REALLOC (0x1 << 2)
#define FREE_BY_REALLOC (0x1 << 3)

// maximum number of old blocks in the old_blocks list
// if this number is reached, all old block in the list are
// write in the json output file
#define MAX_OLD_BLOCKS 100

// this struct is only use as a user_data for realloc wrapping
typedef struct
{
  malloc_t      *block;
  size_t        size;
} realloc_tmp_t;

extern malloc_t  *old_blocks;
extern tree_t	 *active_blocks;
extern void      *lock;

void pre_malloc(void *wrapctx, OUT void **user_data);
void post_malloc(void *wrapctx, void *user_data);

void pre_calloc(void *wrapctx, OUT void **user_data);
void post_calloc(void *wrapctx, void *user_data);

void pre_realloc(void *wrapctx, OUT void **user_data);
void post_realloc(void *wrapctx, void *user_data);

void pre_free(void *wrapctx, OUT void **user_data);

#endif
