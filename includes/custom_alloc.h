#ifndef CUSTOM_ALLOC_H_
#define CUSTOM_ALLOC_H_

#include "allocs.h"
#include <dr_tools.h>
typedef struct page_header_s
{
  void	*next_page;
  int	next_idx;
} page_header_t;

// dynamoRIO now redefine PAGE_SIZE here it's just not define
// So if it does not exist just use the most common value of
// today page size
#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

#define MAX_IDX_ACCESS (PAGE_SIZE - sizeof(page_header_t)) / sizeof(access_t)
#define MAX_IDX_ORIG (PAGE_SIZE - sizeof(page_header_t)) / sizeof(orig_t)
#define MAX_IDX_INSTR (PAGE_SIZE - sizeof(page_header_t))

typedef struct access_page_s
{
  page_header_t	header;
  access_t	accesses[MAX_IDX_ACCESS];
} access_page_t;

typedef struct orig_page_s
{
  page_header_t	header;
  orig_t	origs[MAX_IDX_ORIG];
} orig_page_t;

typedef struct instr_page_s
{
  page_header_t	header;
  char		instrs[MAX_IDX_INSTR];
} instr_page_t;

struct malloc_s;

void *alloc_instr(struct malloc_s *block, unsigned int size);
access_t *alloc_access(struct malloc_s *block);
void free_last_access(struct malloc_s *block);
orig_t *alloc_orig(struct malloc_s *block);
void free_last_orig(struct malloc_s *block);
void custom_free_pages(struct malloc_s *block);
#endif
