#ifndef SYM_H_
#define SYM_H_

#include "hashtable.h"

bool sym_to_hashmap(drsym_info_t *info, drsym_error_t status, void *data);

hashtable_t	*sym_hashtab;
#endif
