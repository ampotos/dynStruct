
#ifndef BLOCK_UTILS_H_
#define BLOCK_UTILS_H_

// sort at insert by offset typedef struct access_s access_t;
typedef struct access_s access_t;
struct access_s
{
  struct access_s  *next;
  size_t           offset;
  size_t           nb_hit;
};

//maybe store module_names for realloc
typedef struct malloc_s malloc_t;
struct malloc_s
{
  void            *start;
  void            *end;
  size_t          size;
  access_t        *access;
  unsigned int    flag;
  struct malloc_s *next;
};

typedef struct
{
  malloc_t	*block;
  size_t	size;
} realloc_tmp_t;

// define flag for malloc_t
#define ALLOC 0x1
#define FREE (0x1 << 1)

// Globals
extern malloc_t  *blocks;
extern void      *lock;

malloc_t *get_block_by_addr(void *addr); //get a block by the start addr
malloc_t *add_block(size_t size); // add new block
void free_malloc_block(malloc_t *block); // free a block (usually this is never call directly use remove_block)
void remove_block(malloc_t *block); // remove and free the give block

#endif
