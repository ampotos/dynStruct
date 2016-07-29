#include "dr_api.h"
#include "../includes/tree.h"

int get_balance(tree_t *node)
{
  int	left = 0;
  int	right = 0;

  if (node->left)
    left = node->left->height;
  if (node->right)
    right = node->right->height;
  
  return left - right;
}

tree_t *get_parent(tree_t *tree, tree_t *node)
{
  // to avoid double dut to some malloc
  // algorithm
  if (tree->min_addr == node->min_addr)
    return NULL;

  if (tree->high_addr < node->min_addr && tree->right)
    return get_parent(tree->right, node);
  else if (tree->min_addr > node->high_addr && tree->left)
    return get_parent(tree->left, node);

  return tree;
}

void recompute_height(tree_t *node)
{
  int	old_height;

  while (node)
    {
      old_height = node->height;
      if (node->right && node->left)
	{
	  if (node->right->height > node->left->height)
	    node->height = node->right->height + 1;
	  else
	    node->height = node->left->height + 1;
	}
      else if (node->left)
	node->height = node->left->height + 1;
      else if (node->right)
	node->height = node->right->height + 1;
      else
	node->height = 0;
      if (node->height == old_height)
	break;
      node = node->parent;
    }
}

void fix_tree_rotate(tree_t *old_node, tree_t *new_node, tree_t *parent, tree_t **tree)
{
  if (!parent)
    {
      new_node->parent = NULL;
      *tree = new_node;
    }
  else
    {
      if (parent->right == old_node)
	parent->right = new_node;
      else
	parent->left = new_node;
      new_node->parent = parent;
    }

  recompute_height(old_node);
  recompute_height(new_node);
}

void rotate_left(tree_t *node, tree_t **tree)
{
  tree_t *parent = node->parent;
  tree_t *tmp;

  tmp = node->right;
  node->right = tmp->left;
  if (node->right)
    node->right->parent = node;
  tmp->left = node;
  node->parent = tmp;

  fix_tree_rotate(node, tmp, parent, tree);
}

void rotate_right(tree_t *node, tree_t **tree)
{
  tree_t *parent = node->parent;
  tree_t *tmp;

  tmp = node->left;
  node->left = tmp->right;
  if (node->left)
    node->left->parent = node;
  tmp->right = node;
  node->parent = tmp;

  fix_tree_rotate(node, tmp, parent, tree);
}

void balance_tree(tree_t *node, tree_t **tree)
{
  tree_t	*parent = node->parent;
  int		left;

  if (parent)
    left = parent->left == node ? 1 : 0;

  if (get_balance(node) < -1)
    {
      if (get_balance(node->right) <= 0)
	rotate_left(node, tree);
      else
	{
	  rotate_left(node, tree);

	  if (parent)
	    {
	      if (left)
		node = parent->left;
	      else
		node = parent->right;
	    }
	  else
	    node = *tree;

	  rotate_right(node, tree);
	}
    }
  else
    {
      if (get_balance(node->left) > 0)
	rotate_right(node, tree);
      else
	{
	  rotate_right(node, tree);

	  if (parent)
	    {
	      if (left)
		node = parent->left;
	      else
		node = parent->right;
	    }
	  else
	    node = *tree;

	  rotate_left(node, tree);
	}
    }
}

int add_to_tree(tree_t **tree, tree_t *node)
{
  tree_t	*parent;
  int		balance;

  if (!node)
    return false;

  node->height = 0;
  node->left = NULL;
  node->right = NULL;

  if (!(*tree))
    {
      node->parent = NULL;
      *tree = node;
    }
  else
    {
      parent = get_parent(*tree, node);
      if (!parent)
	return false;
      node->parent = parent;
      if (parent->min_addr > node->high_addr)
	{
	  // this can happen with some malloc algorithm
	  if (parent->left)
	    return false;
	  parent->left = node;
	}
      else
	{
	  // this can happen with some malloc algorithm
	  if (parent->right)
	    return false;
	  parent->right = node;
	}
      recompute_height(node->parent);
      
      while (parent)
	{
	  balance = get_balance(parent);
	  if (balance < -1 || balance > 1)
	    {
	      balance_tree(parent, tree);
	      break;
	    }	  
	  parent = parent->parent;
	}
    }
  return true;
}

void del_leaf(tree_t **tree, tree_t *node)
{
  tree_t	*parent = node->parent;

  if (parent)
    {
      if (parent->left == node)
	parent->left = NULL;
      else
	parent->right = NULL;
      recompute_height(parent);
    }
  else
    *tree = NULL;
}

void del_branch(tree_t **tree, tree_t *node)
{
  tree_t	*parent = node->parent;

 if (parent)
    {
      if (parent->left == node)
	{
	  if (node->right)
	    parent->left = node->right;
	  else
	    parent->left = node->left;
	  parent->left->parent = parent;
	}
      else
	{
	  if (node->right)
	    parent->right = node->right;
	  else
	    parent->right = node->left;
	  parent->right->parent = parent;
	}
      recompute_height(parent);
    }
  else
    {
      if (node->left)
	*tree = node->left;
      else
	*tree = node->right;
      (*tree)->parent = NULL;
    }
}

void del_from_tree(tree_t **tree, void *start_addr,
		   void (* free_func)(void *), int free)
{
  tree_t	*node = search_on_tree(*tree, start_addr);
  tree_t	*to_switch;
  tree_t	*parent_node;
  tree_t	*parent_switch;
  tree_t	*tmp_node;
  int		tmp_height;
  int		balance;

  if (!node || node->min_addr != start_addr)
    {
      // because this tree can be used to store a unique data
      // are an address space, we have to check both
      if (!(node = search_same_addr_on_tree(*tree, start_addr)))
	return;
    }
  parent_node = node->parent;

  // if no child just delete the node
  if (!(node->right) && !(node->left))
    del_leaf(tree, node);
  // if node have 2 child node swap and delete
  else if (node->right && node->left)
    {
      to_switch = node->right;
      while (to_switch->left)
      	to_switch = to_switch->left;

      parent_switch = to_switch->parent;

      // switch node
      tmp_height = node->height;
      node->height = to_switch->height;
      to_switch->height = tmp_height;

      if (parent_node)
      	{
      	  if (parent_node->left == node)
      	    parent_node->left = to_switch;
      	  else
      	    parent_node->right = to_switch;
      	  to_switch->parent = parent_node;
      	}
      else
      	{
      	  to_switch->parent = NULL;
      	  *tree = to_switch;
      	}

      if (parent_switch)
        {
          if (parent_switch->left == to_switch)
            parent_switch->left = node;
          else
            parent_switch->right = node;
          node->parent = parent_switch;
        }

      // switch left child node
      tmp_node = to_switch->left;
      to_switch->left = node->left;
      node->left = tmp_node;
      if (to_switch->left)
      	to_switch->left->parent = to_switch;
      if (node->left)
      	node->left->parent = node;

      // switch right child node
      tmp_node = to_switch->right;
      to_switch->right = node->right;
      node->right = tmp_node;
      if (to_switch->right)
      	to_switch->right->parent = to_switch;
      if (node->right)
      	node->right->parent = node;

      // delete selected node
      if (!(node->height))
      	del_leaf(tree, node);
      else
      	del_branch(tree, node);
    }
  // if node have 1 child delete it without swaping
  else
    del_branch(tree, node);

  if (free_func)
    free_func(node->data);

  if (free)
    dr_global_free(node, sizeof(*node));

  while (parent_node)
    {
      balance = get_balance(parent_node);
      if (balance < -1 || balance > 1)
  	{
  	  balance_tree(parent_node, tree);
  	  break;
  	}
      parent_node = parent_node->parent;
    }
}

void *search_on_tree(tree_t *tree, void *addr)
{
  if (!tree)
    return NULL;

  if (tree->high_addr <= addr)
    return search_on_tree(tree->right, addr);
  else if (tree->min_addr > addr)
    return search_on_tree(tree->left, addr);

  return tree->data;
}

void *search_same_addr_on_tree(tree_t *tree, void *addr)
{
  if (!tree)
    return NULL;

  if (tree->high_addr < addr)
    return search_same_addr_on_tree(tree->right, addr);
  else if (tree->min_addr > addr)
    return search_same_addr_on_tree(tree->left, addr);

  return tree->data;
}

void clean_tree(tree_t **tree, void (* free_func)(void *), int free)
{
  while (*tree)
    del_from_tree(tree, (*tree)->min_addr, free_func, free);
}
 
