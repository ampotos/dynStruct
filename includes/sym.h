#ifndef SYM_H_
#define SYM_H_

#include "drsyms.h"
#include "hashtable.h"

typedef struct old_sym_s old_sym_t;
struct old_sym_s
{
  struct old_sym_s	*next;
  char			*sym;
};

bool sym_to_hashmap(drsym_info_t *, drsym_error_t, void *);
void clean_old_sym(void);
void delete_sym(void *);

extern hashtable_t	*sym_hashtab;
extern old_sym_t	*old_symlist;
#endif
