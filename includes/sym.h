#ifndef SYM_H_
#define SYM_H_

#include "hashtable.h"

typedef struct old_sym_s
{
  struct old_sym_s	*next;
  char			*sym;
}old_sym_t;

bool sym_to_hashmap(drsym_info_t *, drsym_error_t, void *);

extern hashtable_t	*sym_hashtab;
extern old_sym_t	*old_symlist;
#endif
