#include "dr_api.h"
#include "drsyms.h"
#include "../includes/sym.h"
#include "../includes/utils.h"

bool sym_to_hashmap(drsym_info_t *info, drsym_error_t status, void *data)
{
  // TODO use add_replace (return old value if exist) and if is an old value stock it onthe list
  hashtable_add(sym_hashtab, ((module_data_t *)data)->start + info->start_offs,
  		my_dr_strdup(info->name));
  return true;
}
