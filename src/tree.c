#include "dr_api.h"
#include "../includes/tree.h"

// Tree are used to store active block (meaning non free) and
// plt of each module (to have the right calling addr for *alloc and free)

int	get_balance(tree_t *node)
{
  int	left = 0;
  int	right = 0;

  if (node->left)
    left = node->left->height;
  if (node->right)
    left = node->right->height;
  
  return left - right;
}

tree_t	*get_parent(tree_t *tree, tree_t *node)
{
  if (tree->high_addr < node->min_addr && tree->right)
    return search_on_tree(tree->right, node);
  else if (tree->min_addr > node->high_addr && tree->left)
    return search_on_tree(tree->left, node);
  return tree;
}

void	propage_height(tree_t *node, int val)
{
  while (node->parent)
    {
      node->height += val;
      node = node->parent;
    }
  node->height += val;
}

void	balance_tree(tree_t *tree)
{
  
  return;
}

tree_t	*add_to_tree(tree_t *tree, tree_t *node)
{
  tree_t	*parent;

  if (!tree)
    return node;

  parent = get_parent(tree, node);
  node->parent = parent;
  if (parent->min_addr > node->high_addr)
    parent->left = node;
  else
    parent->right = node;

  propage_height(parent, 1);
  balance_tree(tree);

  return tree;
}

tree_t	*del_from_tree(tree_t *tree, tree_t *node, void (* free_func)(void *))
{
  tree_t	*parent;
  
  parent = node->parent;

  if (parent->left == node)
    parent->left = NULL;
  else
    parent->right = NULL;
  
  propage_height(parent, -1);
  balance_tree(tree);

  if (free_func)
    free_func(node->data);
  dr_global_free(node, sizeof(*node));
  return tree;
}

void	*search_on_tree(tree_t *tree, void *addr)
{
  if (!tree)
    return NULL;

  if (tree->high_addr < addr)
    return search_on_tree(tree->right, addr);
  else if (tree->min_addr > addr)
    return search_on_tree(tree->left, addr);
  return tree->data;
}
