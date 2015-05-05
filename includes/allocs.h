#ifndef ALLOCS_H_
#define ALLOCS_H_

// sort at insert by offset typedef struct access_s access_t;
typedef struct access_s access_t;
struct access_s
{
  struct access_s  *next;
  size_t           offset;
  size_t           nb_hit;
  int		   size;
};

//maybe store module_names for realloc
typedef struct malloc_s malloc_t;
struct malloc_s
{
  struct malloc_s *next;
  void            *start;
  void            *end;
  size_t          size;
  access_t        *read;
  access_t        *write;
  unsigned int    flag;
};

// this struct is only use as a user_data for realloc wrapping
typedef struct
{
  malloc_t      *block;
  size_t        size;
} realloc_tmp_t;

// globals
extern malloc_t  *blocks;
extern void      *lock;

void pre_malloc(void *wrapctx, OUT void **user_data);
void post_malloc(void *wrapctx, void *user_data);

void pre_calloc(void *wrapctx, OUT void **user_data);
void post_calloc(void *wrapctx, void *user_data);

void pre_realloc(void *wrapctx, OUT void **user_data);
void post_realloc(void *wrapctx, void *user_data);

void pre_free(void *wrapctx, __attribute__((unused))OUT void **user_data);

#endif
