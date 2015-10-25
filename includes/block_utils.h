
#ifndef BLOCK_UTILS_H_
#define BLOCK_UTILS_H_

#include "allocs.h"
#include "tree.h"

// define flag for malloc_t
#define ALLOC 0x1
#define FREE (0x1 << 1)
#define ALLOC_BY_REALLOC (0x1 << 2)
#define FREE_BY_REALLOC (0x1 << 3)

access_t *get_access(size_t offset, tree_t **t_access);
malloc_t *add_block(size_t size, void *pc, void *drcontext);
void set_addr_malloc(malloc_t *block, void *start, unsigned int flag, int realloc);

#endif
