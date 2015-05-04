#include <string.h>
#include "dr_api.h"

char *my_dr_strdup(const char* str)
{
  char          *ret;
  size_t        len = strlen(str);

  if ((ret = dr_global_alloc((len + 1) * sizeof(*ret))))
    {
      len = -1;
      while (str[++len])
        ret[len] = str[len];
      ret[len] = 0;
    }
  return (ret);
}
