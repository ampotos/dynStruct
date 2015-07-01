
#ifndef BLOCK_UTILS_H_
#define BLOCK_UTILS_H_

#include "allocs.h"

// define flag for malloc_t
#define ALLOC 0x1
#define FREE (0x1 << 1)

access_t *get_access(size_t, access_t **);
malloc_t *get_active_block_by_access(void *);
malloc_t *get_block_by_addr(void *);
malloc_t *add_block(size_t, void *, void *);
void free_malloc_block(malloc_t *);
void remove_block(malloc_t *);
void set_addr_malloc(malloc_t *, void *, unsigned int, int);

#endif
