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

/* #define DS_PRINTF(...) dr_fprintf(args->file_out,  __VA_ARGS__) */
// here macro is used instead of a function to avoid exposing va_list
// On my setup there is an issue with the buffering when analyzing xterm,
// a pieace of the json is not written into the file, but dr_fprintf work well
// I don't from where this issue come from yet.
// If you reach this issue just comment the maccro and uncomment the previosu
// define to fix it (but it will be  a bit slower).
#define DS_PRINTF(...) {						\
  if (!global_buf || !tmp_buf)						\
    {									\
      init_buffering();							\
    }									\
  len_tmp = dr_snprintf(tmp_buf, TMP_BUF_SIZE, __VA_ARGS__);		\
  if (global_idx + len_tmp + 1 >= GLOBAL_BUF_SIZE)			\
    {									\
      dr_write_file(args->file_out, global_buf, global_idx);		\
      global_idx = 0;							\
    }									\
  ds_memcpy(global_buf + global_idx, tmp_buf, len_tmp);			\
  global_idx += len_tmp;						\
}

void write_json(void);
void flush_old_block(void);

#endif
