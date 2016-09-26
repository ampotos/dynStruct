#ifndef OUT_JSON_H_
#define OUT_JSON_H_

#include "dr_api.h"
#include "utils.h"

// This value are arbitrary, they may need to be adapt in specific cases
#define GLOBAL_BUF_SIZE PAGE_SIZE * 4
#define TMP_BUF_SIZE PAGE_SIZE

extern char    *global_buf;
extern char    *tmp_buf;
extern int     global_idx;
extern int     len_tmp;

//here macro is used instead of a function to avoid exposing va_list
#define DS_PRINTF(...) if (!global_buf || !tmp_buf)	                \
    init_buffering();							\
  len_tmp = dr_snprintf(tmp_buf, PAGE_SIZE, __VA_ARGS__);		\
  if (global_idx + len_tmp + 1 > PAGE_SIZE * 4)				\
    {									\
      dr_write_file(args->file_out, global_buf, global_idx);		\
      global_idx = 0;							\
    }									\
  ds_memcpy(global_buf + global_idx, tmp_buf, len_tmp);			\
  global_idx += len_tmp;
void write_json(void);
void flush_old_block(void);

#endif
