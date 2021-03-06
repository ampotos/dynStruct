#ifndef ARGS_H_
#define ARGS_H_

#include "dr_api.h"

typedef struct module_name_s
{
  char			*name;
  struct module_name_s	*next;
} module_name_t;

typedef struct
{
  int			console;
  char			*out_dir;
  char			*out_name;
  file_t		file_out;
  module_name_t		*wrap_modules_s;
  module_name_t		*monitor_modules_s; 
  module_data_t		**monitor_modules;
  module_data_t		**wrap_modules;
  int			size_wrap;
  int			size_monitor;
  char			*alloc;
} args_t;

extern args_t *args;

int parse_arg(int argc, char **argv);
int maj_args(const module_data_t *mod);
void clean_args(void);

int module_is_wrapped(void *drcontext);
int pc_is_monitored(app_pc pc);
int module_is_alloc(const module_data_t *mod);

#endif
