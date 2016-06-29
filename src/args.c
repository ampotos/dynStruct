#include "dr_api.h"
#include "../includes/args.h"
#include "../includes/utils.h"
#include "../includes/call.h"

args_t *args;

static char *usage[] = {
  "\nUsage : "
  "drrun -opt_cleancall 3 -c <dynStruct_path> <dynStruct_args> -- <prog_path> <prog_args>\n\n",
  "  -h \t\t\tprint this help\n",

  "  -o <file_name>\tset output name for json file\n"
  "\t\t\t if a file with this name already exist the default name will be used\n",
  "\t\t\t in the case of forks, default name will be used for forks json files\n",
  "\t\t\t (default: <prog_name>.<pid>)\n",

  "  -d <dir_name>\t\tset output directory for json files\n",
  "\t\t\t (default: current directory)\n",

  "  - \t\t\tprint output on console\n",
  "\t\t\t Usable only on very small programs\n",

  "  -w <module_name>\twrap <module_name>\n",  
  "\t\t\t dynStruct record memory blocks only\n",
  "\t\t\t if *alloc is called from this module\n",

  "  -m <module_name>\tmonitor <module_name>\n",
  "\t\t\t dynStruct record memory access only if\n",
  "\t\t\t they are done by a monitore module\n",
  
  "  -a <module_name>\tis used to tell dynStruct ",
  "which module implements\n",
  "\t\t\t allocs functions (malloc, calloc, realloc and free)\n",
  "\t\t\t this has to be used with the -w option ",
  "(ex : \"-a ld -w ld\")\n",
  "\t\t\t this option can only be used one time\n",

  "for -w, -a and -m options modules names are matched like ",
  "<module_name>*\n",
  "this allow to don't care about the version of a library\n",
  "-m libc.so match with all libc verison\n\n",

  "The main module is always monitored and wrapped\n",
  "Tha libc allocs functions are always used ",
  "(regardless the use of the -a option)\n",

  "\nExample : drrun -opt_cleancall 3 -c dynStruct -m libc.so - -- ls -l\n\n",
  "This command run \"ls -l\" and will only look at block ",
  "allocated by the program\n",
  "but will monitor and record memory access from ",
  "the program and the libc\n",
  "and print the result on the console\n\n",
  
  NULL
};

void print_usage(void)
{
  for (int ct = 0; usage[ct]; ct++)
    dr_printf("%s", usage[ct]);
}

int add_arg(module_name_t **list, char *name)
{
  module_name_t	*new;

  if (!name)
    {
      dr_printf("Missing name for -w or -m option\n");
      return false;
    }

  if (name[0] == '-')
    {
      dr_printf("Bad module name : %s\n", name);
      return false;
    }

  if (!(new = dr_global_alloc(sizeof(*new))))
    {
      dr_printf("Can't alloc\n");
      return false;
    }

  new->name = name;
  new->next = *list;
  *list = new;

  return true;
}

int do_alloc_array(module_name_t *list, module_data_t ***array, int *size)
{
  for (; list; list = list->next)
    (*size)++;

  if (!(*array =
	dr_global_alloc(sizeof(**array) * *size)))
    return false;

  ds_memset(*array, 0, sizeof(**array) * *size);

  return true;
}

int alloc_array(void)
{
  args->size_wrap = 1;
  args->size_monitor = 1;

  if (!do_alloc_array(args->wrap_modules_s, &(args->wrap_modules),
		     &(args->size_wrap)) ||
      !do_alloc_array(args->monitor_modules_s, &(args->monitor_modules),
		      &(args->size_monitor)))
    return false;
  
  if (!(args->monitor_modules[0] = dr_get_main_module()) ||
      !(args->wrap_modules[0] = dr_get_main_module()))
    {
      dr_printf("Can't get main module\n");
      return false;
    }

  return true;
}

int set_alloc(char *name)
{
  if (args->alloc)
    dr_printf("-a option have to be use only ont time\n");

  if (name[0] == '-')
    {
      dr_printf("Bad module name : %s\n", name);
      return false;
    }

  args->alloc = name;

  return true;
}

char *get_output_name(int *size)
{
  char	*filename = NULL;
  int	dir_size;
  int	name_size;

  if (!args->console && args->out_name)
    {
      if (!args->out_dir)
	{
	  filename = ds_strdup(args->out_name);
	  *size = sizeof(*filename) * (ds_strlen(filename) + 1);
	}
      else
	{
	  dir_size = ds_strlen(args->out_dir);
	  name_size = ds_strlen(args->out_name);
	  *size = sizeof(*filename) * (name_size + dir_size + 2);
	  if (!(filename = dr_global_alloc(sizeof(*filename) *
					   (name_size + dir_size + 2))))
	    return NULL;
	  ds_memset(filename, 0, name_size + dir_size + 2);

	  ds_strncpy(filename, args->out_dir, dir_size);
	  filename[dir_size] = '/';
	  ds_strncpy(filename + dir_size + 1, args->out_name, name_size);
	}
    }

  return filename;
}

char *get_generic_name(int *size)
{
  module_data_t	*mod;
  char		*filename;
  const char	*mod_name;
  int		mod_size;
  int		dir_size = 0;
  process_id_t	pid;
  char		pid_str[6] = {0};
  int		pid_size;
  char		*tmp_ptr;

  if (!(mod = dr_get_main_module()))
    return NULL;

  mod_name = dr_module_preferred_name(mod);
  mod_size = ds_strlen(mod_name);

  if (args->out_dir)
    dir_size = ds_strlen(args->out_dir);

  *size = sizeof(*filename) * (mod_size + 8 + dir_size);
  if (!(filename = dr_global_alloc(sizeof(*filename) *
				   (mod_size + 8 + dir_size))))
    {
      dr_free_module_data(mod);
      return NULL;
    }
  ds_memset(filename, 0, sizeof(*filename) * (mod_size + 8 + dir_size));

  if (args->out_dir)
    {
      ds_strncpy(filename, args->out_dir, dir_size);
      filename[dir_size++] = '/';
    }
  ds_strncpy(filename + dir_size, mod_name, mod_size);
  filename[dir_size + mod_size] = '.';

  pid = dr_get_process_id();
  tmp_ptr = pid_str;
  while (pid > 9)
    {
      *tmp_ptr++ = pid % 10 + '0';
      pid = pid / 10;
    }
  *tmp_ptr = pid + '0';

  pid_size = ds_strlen(pid_str) - 1;
  tmp_ptr = filename + dir_size + mod_size + 1;

  while (pid_size >= 0)
    *tmp_ptr++ = pid_str[pid_size--];

  dr_free_module_data(mod);

  return filename;
}

file_t open_out_file()
{
  int		name_size;
  char		*filename = get_output_name(&name_size);
  file_t	file;

  if (!filename || dr_file_exists(filename))
    {
      if (filename)
	dr_global_free(filename, name_size);

      if (!(filename = get_generic_name(&name_size)))
	{
	  dr_printf("Enable to create file name");
	  return INVALID_FILE;
	}
    }

  file = dr_open_file(filename, DR_FILE_WRITE_OVERWRITE | DR_FILE_ALLOW_LARGE);
  dr_global_free(filename, name_size);

  return file;
}

int parse_arg(int argc, char **argv)
{
  if (!(args = dr_global_alloc(sizeof(*args))))
    {
      dr_printf("Can't alloc\n");
      return false;
    }

  ds_memset(args, 0, sizeof(*args));
  
  for (int ct = 1; ct < argc; ct++)
    {
      if (argv[ct][0] != '-')
	{
	  dr_printf("Bad arg %s\n", argv[ct]);
	  print_usage();
	  return false;
	}

      switch (argv[ct][1])
	{
	case '\0':
	  args->console = true;
	  break;
	case 'o':
	  args->out_name = argv[ct + 1];
	  ct++;
	  break;
	case 'd':
	  args->out_dir = argv[ct + 1];
	  ct++;
	  break;
	case 'w':
	  if (ct + 1 == argc)
	    {
	      dr_printf("missing arg for -w\n");
	      return false;
	    }
	  if (!add_arg(&(args->wrap_modules_s), argv[ct + 1]))
	    return false;
	  ct++;
	  break;
	case 'm':
	  if (ct + 1 == argc)
	    {
	      dr_printf("missing arg for -m\n");
	      return false;
	    }
	  if (!add_arg(&(args->monitor_modules_s), argv[ct + 1]))
	    return false;
	  ct++;
	  break;
	case 'a':
	  if (ct + 1 == argc)
	    {
	      dr_printf("missing arg for -a\n");
	      return false;
	    }
	  if (!set_alloc(argv[ct + 1]))
	    return false;
	  ct++;
	  break;
	default:
	  dr_printf("Bad arg %s\n", argv[ct]);
	case 'h':
	  print_usage();
	  return false;
	}
    }

  if (!args->console)
    {
      if ((args->file_out = open_out_file()) == INVALID_FILE)
	{
	  dr_printf("Output file not created\n");
	  return false;
	}
      else
	dr_fprintf(args->file_out, "{\"is_64\":%d, \"blocks\":[",
		   dr_get_isa_mode(dr_get_current_drcontext()) == DR_ISA_AMD64);
    }

  return alloc_array();
}

int add_to_array(const module_data_t *mod, module_data_t **array,
		 int array_size)
{
  for (int ct = 0; ct < array_size; ct++)
    if (!(array[ct]))
      {	  
	if (!(array[ct] = dr_copy_module_data(mod)))
	  {
	    dr_printf("Can't copy module data\n");
	    return false;
	  }
	break;
      }

  return true;
}

int search_name(module_name_t **list, const module_data_t *mod,
		    module_data_t **array, int size_array)
{
  module_name_t	*tmp_list = *list;
  module_name_t *tmp;
  
  if (tmp_list && !ds_strncmp(tmp_list->name, dr_module_preferred_name(mod),
		 ds_strlen(tmp_list->name)))
    {
      if (!add_to_array(mod, array, size_array))
	return false;
      
      *list = tmp_list->next;
      dr_global_free(tmp_list, sizeof(*tmp_list));
    }
  else if (tmp_list)
    {
      for (; tmp_list->next && ds_strncmp(tmp_list->next->name,
				      dr_module_preferred_name(mod),
				      ds_strlen(tmp_list->next->name));
	   tmp_list = tmp_list->next);

      if (tmp_list->next)
	{
	  if (!add_to_array(mod, array, size_array))
	    return false;
	  
	  tmp = tmp_list->next;
	  tmp_list->next = tmp_list->next->next;
	  dr_global_free(tmp, sizeof(*tmp));
	}
    }

  return true;
}

int maj_args(const module_data_t *mod)
{
  if (!search_name(&(args->wrap_modules_s), mod,
		   args->wrap_modules, args->size_wrap) ||
      !search_name(&(args->monitor_modules_s), mod,
		   args->monitor_modules, args->size_monitor))
    return false;
  
  return true;
}

void clean_array_args(int size, module_data_t **array)
{
  for (int ct = 0; ct < size; ct++)
    if (array[ct])
      dr_free_module_data(array[ct]);

  dr_global_free(array, sizeof(*(array)) * size);
}

void clean_list_args(module_name_t *list)
{
  module_name_t *tmp;

  while (list)
    {
      tmp = list;
      list = list->next;
      dr_global_free(tmp, sizeof(*tmp));
    }
}

void clean_args(void)
{
  clean_list_args(args->wrap_modules_s);
  clean_list_args(args->monitor_modules_s);
  clean_array_args(args->size_wrap, args->wrap_modules);
  clean_array_args(args->size_monitor, args->monitor_modules);
  dr_global_free(args, sizeof(*args));
}


int module_is_wrapped(void *drcontext)
{
  void	*addr;

  get_caller_data(&addr, NULL, NULL, drcontext, 1);

  for (int ct = 0; ct < args->size_wrap; ct++)
    {
      if (args->wrap_modules[ct] &&
	  dr_module_contains_addr(args->wrap_modules[ct], addr))
	return true;
    }
  
  return false;
}

int pc_is_monitored(app_pc pc)
{
  for (int ct = 0; ct < args->size_monitor; ct++)
    if (args->monitor_modules[ct] &&
	dr_module_contains_addr(args->monitor_modules[ct], pc))
      return true;

  return false;
}

int module_is_alloc(const module_data_t *mod)
{
  const char	*name = dr_module_preferred_name(mod);

  if (!ds_strncmp("libc.so", name, ds_strlen("libc.so")))
    return true;
  
  if (args->alloc && !ds_strncmp(args->alloc, name, ds_strlen(args->alloc)))
    return true;

  return false;
}
