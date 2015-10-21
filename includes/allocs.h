#ifndef ALLOCS_H_
#define ALLOCS_H_

#include "tree.h"

// store nb_hit and size of the hit by pc on access instruction
typedef struct orig_s
{
  // next is use if an orig have the same addr but the same access size
  struct orig_s	*next;
  size_t	size;
  size_t	nb_hit;
  void		*addr;
  void		*start_func_addr;
  char		*start_func_sym;
} orig_t;

typedef struct access_s
{
  size_t           offset;
  size_t	   total_hits;
  tree_t	   *origs;
} access_t;

typedef struct malloc_s
{
  struct malloc_s *next;
  void            *start;
  void            *end;
  size_t          size;
  tree_t          *read;
  tree_t          *write;
  unsigned int    flag;
  void		  *alloc_pc;
  void		  *alloc_func_pc;
  char		  *alloc_func_sym;
  void		  *free_pc;
  void		  *free_func_pc;
  char		  *free_func_sym;
} malloc_t;

// this struct is only use as a user_data for realloc wrapping
typedef struct
{
  malloc_t      *block;
  size_t        size;
} realloc_tmp_t;

// globals
extern malloc_t  *old_blocks;
extern tree_t	 *active_blocks;
extern void      *lock;

void pre_malloc(void *, OUT void **);
void post_malloc(void *, void *);

void pre_calloc(void *, OUT void **);
void post_calloc(void *, void *);

void pre_realloc(void *, OUT void **);
void post_realloc(void *, void *);

void pre_free(void *, OUT void **);

#endif
