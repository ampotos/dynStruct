#ifndef STACKS_H_
#define STACKS_H_

typedef struct stack_s
{
  void			*addr;
  const char		*module_name;
  char			*name;
  struct stack_s	*next;
  int			on_plt;
  int			was_on_plt;
} stack_t;

typedef struct module_s
{
  module_data_t		*module;
  struct module_s	*next;
} module_t;

// store the index of the tls slot who contain stack data
int	tls_stack_idx;

void dir_call_monitor(void *pc);
void ind_call_monitor(app_pc caller, app_pc calle);
void ret_monitor(void *pc);
void clean_stack(void *drcontext);
void get_caller_data(void **addr, char **sym, const char **module,
		     void *drcontext, int alloc);
int add_to_module_list(const module_data_t *mod);
void clean_module_list();

#if !__X86_64__
bool indirect_call_ignore(instr_t *isntr);
#endif

extern module_t		*module_list;

#endif
