#ifndef STACKS_H_
#define STACKS_H_

typedef struct stack_s stack_t;
struct stack_s
{
  void			*addr;
  struct stack_s	*next;
};

// store the index of the tls slot who contain stack data
int	tls_stack_idx;

void    dir_call_monitor(void *pc);
void    ind_call_monitor(void *pc);
void    ret_monitor(void *pc);
#endif
