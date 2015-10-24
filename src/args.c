#include "dr_api.h"
#include "../includes/args.h"
#include "../includes/utils.h"

args_t *args;

void	print_usage()
{
  dr_printf("\nUsage : drrun -c <dynStruct_path> <dynStruct_args> -- <prog_path> <prog_args>\n\n");

  dr_printf("  -h \t\t\tprint this help\n");

  dr_printf("  -o <file_name>\tset output file name (default: <prog_name>.ds_out.\n");

  dr_printf("  - \t\t\tprint output on console\n");

  dr_printf("  -w <module_name>\twrap <module_name>\n");
  dr_printf("\t\t\t dynStruct record memory blocks only\n");
  dr_printf("\t\t\t if they are by a symbol *alloc or free from this module\n");

  dr_printf("  -m <module_name>\tmonitor <module_name>\n");
  dr_printf("\t\t\t dynStruct record memory access only if\n");
  dr_printf("\t\t\t they are done by a monitore module\n\n");

  dr_printf("for -w and -m options modules names are matched like <module_name>*\n");
  dr_printf("this allow to don't care about the version of a library\n");
  dr_printf("-m libc.so match with all libc verison\n\n");
  
  dr_printf("The main module is always monitored and the libc is always wrapped\n");
  
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

int	alloc_array()
{
  module_name_t	*tmp;
  
  // size start at one because mais module is always wrapped
  for (args->size_wrap = 1, tmp = args->wrap_modules_s;
       tmp; tmp = tmp->next)
    args->size_wrap++;

  if (!(args->wrap_modules =
	dr_global_alloc(sizeof(*(args->wrap_modules)) * args->size_wrap)))
    return false;

  ds_memset(args->wrap_modules, 0,
	    sizeof(*(args->wrap_modules)) * args->size_wrap);
  
  // size start at one because mais module is always monitored
  for (args->size_monitor = 1, tmp = args->monitor_modules_s; tmp;
       tmp = tmp->next)
    args->size_monitor++;

  if (!(args->monitor_modules =
	dr_global_alloc(sizeof(*(args->monitor_modules)) * args->size_monitor)))
    return false;

  ds_memset(args->monitor_modules, 0,
	    sizeof(*(args->monitor_modules)) * args->size_monitor);

  if (!(args->monitor_modules[0] = dr_get_main_module()))
    {
      dr_printf("Can't get main module\n");
      return false;
    }

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
	default:
	  dr_printf("Bad arg %s\n", argv[ct]);
	case 'h':
	  print_usage();
	  return false;
	}
    }

  add_arg(&(args->wrap_modules_s), "libc.so");
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
		   args->monitor_modules, args->size_wrap))
    return false;
  
  return true;
}

void clean_args()
{
  // todo clean linked list if something is still present

  for (int ct = 0; ct < args->size_wrap; ct++)
    if (args->wrap_modules[ct])
      dr_free_module_data(args->wrap_modules[ct]);

  dr_global_free(args->wrap_modules,
  		 sizeof(*(args->wrap_modules)) * args->size_wrap);

  for (int ct = 0; ct < args->size_monitor; ct++)
    if (args->monitor_modules[ct])
      dr_free_module_data(args->monitor_modules[ct]);

  dr_global_free(args->monitor_modules,
  		 sizeof(*(args->monitor_modules)) * args->size_monitor);
}


int module_is_monitored(const module_data_t *mod)
{
  const char *name;
  
  for (int ct = 0; ct < args->size_wrap; ct++)
    if (args->wrap_modules[ct])
      {
	name = dr_module_preferred_name(args->wrap_modules[ct]);
	if (!ds_strncmp(name, dr_module_preferred_name(mod), ds_strlen(name)))
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
