
#ifndef BLOCK_UTILS_H_
#define BLOCK_UTILS_H_

#include "allocs.h"
#include "tree.h"

malloc_t *add_block(size_t size, void *pc, void *drcontext);
void set_addr_malloc(malloc_t *block, void *start, unsigned int flag, int realloc);

#endif
