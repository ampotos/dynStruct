#ifndef UTILS_H_
#define UTILS_H_

char *ds_strdup(const char *);
size_t ds_strlen(const char *);
int ds_strncmp(const char *, const char *, size_t);
int ds_strcmp(const char *, const char *);
void ds_memset(void *, int, size_t);
char *ds_strncpy(char *dest, const char *src, size_t size);

#endif
  
