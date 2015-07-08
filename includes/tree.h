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

void    *search_on_tree(tree_t *, void *);
void    del_from_tree(tree_t **, void *, void (*)(void *));
void    clean_tree(tree_t **tree, void (* free_func)(void *));
void    add_to_tree(tree_t **, tree_t *);

#endif
