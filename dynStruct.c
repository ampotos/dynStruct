
/*  drwrap : to wrap pre and post call for malloc
    drsym : to get adress of malloc by symbole name
*/   

#include "dr_api.h"

int	nb_block;
void	*as_built_lock;

static void exit_event(void);

static dr_emit_flags_t bb_event(void *drcontext, void *tag, instrlist_t *bb,
				bool for_trace, bool translating);

DR_EXPORT void dr_init(client_id_t id)
{
  dr_register_exit_event(exit_event);
  dr_register_bb_event(bb_event);
 
  as_built_lock = dr_mutex_create();
}


static void exit_event(void)
{
  char out[512];
  int  size;
  
  size = snprintf(out, 512, "There is %d basic block", nb_block);
  dr_mutex_destroy(as_built_lock);

  DR_ASSERT(size > 0);

  out[size] = 0;
  dr_printf("%s\n", out);
}

static dr_emit_flags_t bb_event(void *drcontext, void *tag, instrlist_t *bb,
				  bool for_trace,bool translating)
{
  dr_mutex_lock(as_built_lock);

  nb_block++;

  dr_mutex_unlock(as_built_lock);

  return DR_EMIT_DEFAULT;
}
