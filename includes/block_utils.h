
#ifndef BLOCK_UTILS_H_
#define BLOCK_UTILS_H_

#include "allocs.h"

// define flag for malloc_t
#define ALLOC 0x1
#define FREE (0x1 << 1)

access_t *get_access(size_t offset, access_t **l_access);// get access structure of a block

malloc_t *get_active_block_by_access(void *addr); // get a block by an addr of a memory access
malloc_t *get_block_by_addr(void *addr); //get a block by the start addr

malloc_t *add_block(size_t size, void *alloc_pc); // add new block
void free_malloc_block(malloc_t *block); // free a block (usually this is never call directly use remove_block)
void remove_block(malloc_t *block); // remove and free the give block

void set_addr_malloc(malloc_t *block, void *start, unsigned int flag, int realloc); // set adress of start and end on a block

#endif
