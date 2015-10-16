#ifndef STACKS_H_
#define STACKS_H_

typedef struct stack_s
{
  void			*addr;
  char			*name;
  struct stack_s	*next;
  int			on_plt;
} stack_t;

// store the index of the tls slot who contain stack data
int	tls_stack_idx;

void    dir_call_monitor(void *);
void    ind_call_monitor(app_pc, app_pc);
void    ret_monitor();
void    clean_stack(void *);
void	get_caller_data(void **addr, char **sym, void *drcontext, int);
#endif
