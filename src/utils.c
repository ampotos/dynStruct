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

int my_dr_strncmp(const char *s1, const char *s2, size_t size)
{
  int ct;

  for (ct = 0; ct < size && s1[ct] && s1[ct] == s2[ct]; ct++);
  if (ct == size)
    return 0;
  return s1[ct] - s2[ct];
}
