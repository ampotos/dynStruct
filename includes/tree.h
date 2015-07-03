#ifndef TREE_C_
#define TREE_C_

typedef struct tree_s tree_t;
struct tree_s
{
  struct tree_s *left;
  struct tree_s *right;
  struct tree_s *parent;
  void		*high_addr;
  void		*min_addr;
  void		*data;
  int		height;
};

#endif
