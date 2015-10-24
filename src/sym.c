#include "dr_api.h"
#include "drsyms.h"
#include "../includes/sym.h"
#include "../includes/utils.h"

hashtable_t      sym_hashtab;
old_sym_t        *old_symlist = NULL;

bool sym_to_hashmap(drsym_info_t *info,
		    drsym_error_t __attribute__((unused))status,
		    void *data)
{
  char		*old_val = NULL;
  old_sym_t	*old_sym;

  // store the got entry to resolve library call
  if (!ds_strcmp(info->name, "_GLOBAL_OFFSET_TABLE_"))
    ((ds_module_data_t*)data)->got = ((ds_module_data_t *)data)->start +
      info->start_offs;
  
  old_val = hashtable_add_replace(&sym_hashtab, ((ds_module_data_t *)data)->start +
  				  info->start_offs,
  				  ds_strdup(info->name));
  /* if there is an old val for this entry (a lib was unload and a new is load) */
  /* store the old sym name in a linked list to keep the string fo the name */
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


void clean_old_sym(void)
{
  old_sym_t	*tmp;

  while (old_symlist)
    {
      tmp = old_symlist;
      old_symlist = old_symlist->next;
      dr_global_free(tmp->sym, ds_strlen(tmp->sym + 1));
      dr_global_free(tmp, sizeof(*tmp));
    }
}
