#include "dr_api.h"
#include "drsyms.h"
#include "../includes/sym.h"
#include "../includes/utils.h"

hashtable_t      *sym_hashtab;
old_sym_t        *old_symlist = NULL;

bool sym_to_hashmap(drsym_info_t *info,
		    drsym_error_t __attribute__((unused))status,
		    void *data)
{
  char		*old_val;
  old_sym_t	*old_sym;

  old_val = hashtable_add_replace(sym_hashtab, ((module_data_t *)data)->start +
				  info->start_offs,
				  ds_strdup(info->name));
  if (old_val)
    {
      if (!(old_sym = dr_global_alloc(sizeof(*old_sym))))
	{
	  dr_global_free(old_val, ds_strlen(old_val));
	  return false;
	}
      old_sym->sym = old_val;
      old_sym->next = old_symlist;
      old_symlist = old_sym;
    }

  return true;
}
