#include "dr_api.h"


size_t ds_strlen(const char *s)
{
  for (int ct = 0; ; ct++)
    if (!s[ct])
      return ct;
}

char *ds_strdup(const char* str)
{
  char          *ret;
  size_t        len = ds_strlen(str);

  if ((ret = dr_global_alloc((len + 1) * sizeof(*ret))))
    {
      len = -1;
      while (str[++len])
        ret[len] = str[len];
      ret[len] = 0;
    }
  return ret;
}

int ds_strncmp(const char *s1, const char *s2, size_t size)
{
  size_t ct;

  for (ct = 0; ct < size && s1[ct] && s1[ct] == s2[ct]; ct++);
  if (ct == size)
    return 0;
  return s1[ct] - s2[ct];
}

int ds_strcmp(const char *s1, const char *s2)
{
  size_t ct;

  for (ct = 0; s1[ct] && s1[ct] == s2[ct]; ct++);
  return s1[ct] - s2[ct];
}

void ds_memset(void *ptr, int c, size_t size)
{
  while (--size > 0)
    ((char *)ptr)[size] = (char)c;
  *((char *)ptr) = (char)c;
}

char *ds_strncpy(char *dest, const char *src, size_t size)
{
  for (size_t ct = 0; ct < size; ct++)
    dest[ct] = src[ct];

  return dest;
}
