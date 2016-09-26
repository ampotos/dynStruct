#ifndef UTILS_H_
#define UTILS_H_

char *ds_strdup(const char *str);
size_t ds_strlen(const char *str);
int ds_strncmp(const char *str1, const char *str2, size_t size);
int ds_strcmp(const char *str1, const char *str2);
void ds_memset(void *ptr, int c, size_t size);
void ds_memcpy(void *dest, void *src, size_t size);
char *ds_strncpy(char *dest, const char *src, size_t size);

#endif
  
