

void	dir_call_monitor(void *pc)
{
  // TODO get the addr off the caled function and push it to stack (with global tls index)
  dr_printf("direct call\n");
  return;
}


void	ind_call_monitor(void *pc)
{
  // TODO get the addr off the caled function and push it to stack (with global tls index)
  dr_printf("indirect call\n");
  return;
}


void	ret_monitor(void *pc)
{
  // TODO pop addr from tls stack (maybe problematic if nb call/ret differ so chack this correctly
  dr_printf("return\n");
  return;
}
