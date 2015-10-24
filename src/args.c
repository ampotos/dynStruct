#include "dr_api.h"
#include "../includes/args.h"
#include "../includes/utils.h"
#include "../includes/call.h"

args_t *args;

void	print_usage()
{
  dr_printf("\nUsage : drrun -c <dynStruct_path> <dynStruct_args> -- <prog_path> <prog_args>\n\n");

  dr_printf("  -h \t\t\tprint this help\n");

  dr_printf("  -o <file_name>\tset output file name for json (default: <prog_name>.ds_out.\n");
  
  dr_printf("  - \t\t\tprint output on console\n");

  dr_printf("  -w <module_name>\twrap <module_name>\n");
  dr_printf("\t\t\t dynStruct record memory blocks only\n");
  dr_printf("\t\t\t if *alloc is called from this module\n");

  dr_printf("  -m <module_name>\tmonitor <module_name>\n");
  dr_printf("\t\t\t dynStruct record memory access only if\n");
  dr_printf("\t\t\t they are done by a monitore module\n");

  dr_printf("  -a <module_name>\tis used to tell dynStruct the module that implements\n");
  dr_printf("\t\t\t allocs functions (malloc, calloc, realloc and free)\n");
  dr_printf("\t\t\t this has to be used with the -w option (ex : \"-a ld -w ld\")\n");
  dr_printf("\t\t\t this option can only be used one time\n");
  
  dr_printf("for -w, -a and -m options modules names are matched like <module_name>*\n");
  dr_printf("this allow to don't care about the version of a library\n");
  dr_printf("-m libc.so match with all libc verison\n\n");
  
  dr_printf("The main module is always monitored and wrapped\n");
  dr_printf("Tha libc allocs functions are always used (regardless the use of the -a option)\n");
  
  dr_printf("\nExample : drrun -c dynStruct -m libc.so - -- ls -l\n\n");
  dr_printf("This command run \"ls -l\" and will only look at block allocated by the program\n");
  dr_printf("but will monitor and record memory access from the program and the libc\n");
  dr_printf("and print the result on the console\n\n");
}

int	add_arg(module_name_t **list, char *name)
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

int	do_alloc_array(module_name_t *list, module_data_t ***array, int *size)
{
  for (; list; list = list->next)
    (*size)++;

  if (!(*array =
	dr_global_alloc(sizeof(**array) * *size)))
    return false;

  ds_memset(*array, 0, sizeof(**array) * *size);

  return true;
}

int	alloc_array()
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

int	set_alloc(char *name)
{
  if (args->alloc)
    dr_printf("-a option have to be use only ont time\n");

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

  args->alloc = name;

  return true;
}

int	parse_arg(int argc, char **argv)
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
	  args->out_file = argv[ct + 1];
	  ct++;
	  break;
	case 'w':
	  if (!add_arg(&(args->wrap_modules_s), argv[ct + 1]))
	    return false;
	  ct++;
	  break;
	case 'm':
	  if (!add_arg(&(args->monitor_modules_s), argv[ct + 1]))
	    return false;
	  ct++;
	  break;
	case 'a':
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

  return alloc_array();
}

int	add_to_array(const module_data_t *mod, module_data_t **array, int array_size)
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

int	search_name(module_name_t **list, const module_data_t *mod,
		    module_data_t **array, int size_array)
{
  module_name_t *tmp_list = *list;
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

int	maj_args(const module_data_t *mod)
{
  if (!search_name(&(args->wrap_modules_s), mod,
		   args->wrap_modules, args->size_wrap) ||
      !search_name(&(args->monitor_modules_s), mod,
		   args->monitor_modules, args->size_monitor))
    return false;
  
  return true;
}

void	clean_array_args(int size, module_data_t **array)
{
  for (int ct = 0; ct < size; ct++)
    if (array[ct])
      dr_free_module_data(array[ct]);

  dr_global_free(array, sizeof(*(array)) * size);
}

void	clean_list_args(module_name_t *list)
{
  module_name_t *tmp;

  for (; list; list = list->next)
    {
      tmp = list;
      list = list->next;
      dr_global_free(tmp, sizeof(*tmp));
    }
}
void clean_args()
{
  clean_list_args(args->wrap_modules_s);
  clean_list_args(args->monitor_modules_s);
  clean_array_args(args->size_wrap, args->wrap_modules);
  clean_array_args(args->size_monitor, args->monitor_modules);
}


int module_is_wrapped(void *drcontext)
{
  void	*addr;

  get_caller_data(&addr, NULL, drcontext, 1);
  
  for (int ct = 0; ct < args->size_wrap; ct++)
    if (args->wrap_modules[ct] &&
	dr_module_contains_addr(args->wrap_modules[ct], addr))
      return true;

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
