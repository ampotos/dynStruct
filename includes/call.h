#ifndef STACKS_H_
#define STACKS_H_

typedef struct stack_s
{
  void			*addr;
  char			*name;
  struct stack_s	*next;
  int			on_plt;
  int			was_on_plt;
} stack_t;

// store the index of the tls slot who contain stack data
int	tls_stack_idx;

void dir_call_monitor(void *pc);
void ind_call_monitor(app_pc caller, app_pc calle);
void ret_monitor(void *pc);
void clean_stack(void *drcontext);
void get_caller_data(void **addr, char **sym, void *drcontext, int alloc);

#if !__LP64__
extern module_data_t	*dynamo_mod;
#endif

#endif
