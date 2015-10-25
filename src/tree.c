#include "dr_api.h"
#include "../includes/tree.h"

int get_balance(tree_t *node)
{
  int	left = -1;
  int	right = -1;

  if (node->left)
    left = node->left->height;
  if (node->right)
    right = node->right->height;
  
  return left - right;
}

tree_t *get_parent(tree_t *tree, tree_t *node)
{
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


void balance_tree(tree_t *node, tree_t **tree)
{
  tree_t	*parent = node->parent;
  tree_t	*tmp;
  tree_t	*tmp2;

  if (get_balance(node) < -1)
    if (get_balance(node->right) <= 0)
      {
	tmp = node->right;
	node->right = tmp->left;
	if (node->right)
	  node->right->parent = node;
	tmp->left = node;
	node->parent = tmp;

	if (!parent)
	  {
	    tmp->parent = NULL;
	    *tree = tmp;
	  }
	else
	  {
	    if (parent->right == node)
	      parent->right = tmp;
	    else
	      parent->left = tmp;
	    tmp->parent = parent;
	  }

	recompute_height(node);
	recompute_height(tmp->parent);
      }
    else
      {
	tmp = node->right;
	tmp2 = tmp->left;
	tmp->left = tmp2->right;
	if (tmp->left)
	  tmp->left->parent = tmp;
	node->right = tmp2->left;
	if (node->right)
	  node->right->parent = node;
	tmp2->right = tmp;
	tmp->parent = tmp2;
	tmp2->left = node;
	node->parent = tmp2;

	if (!parent)
	  {
	    tmp2->parent = NULL;
	    *tree = tmp2;
	  }
	else
	  {
	    if (parent->right == node)
	      parent->right = tmp2;
	    else
	      parent->left = tmp2;
	    tmp2->parent = parent;
	  }

	recompute_height(node);
	recompute_height(tmp);
      }
  else
    if (get_balance(node->left) >= 0)
      {
	tmp = node->left;
	node->left = tmp->right;
	if (node->left)
	  node->left->parent = node;
	tmp->right = node;
	node->parent = tmp;

	if (!parent)
	  {
	    tmp->parent = NULL;
	    *tree = tmp;
	  }
	else
	  {
	    if (parent->right == node)
	      parent->right = tmp;
	    else
	      parent->left = tmp;
	    tmp->parent = parent;
	  }

	recompute_height(node);
	recompute_height(tmp->parent);
      }
    else
      {
	tmp = node->left;
	tmp2 = tmp->right;
	node->left = tmp2->right;
	if (node->left)
	  node->left->parent = node;
	tmp->right = tmp2->left;
	if (tmp->right)
	  tmp->right->parent = tmp;
	tmp2->left = tmp;
	tmp->parent = tmp2;
	tmp2->right = node;
	node->parent = tmp2;

	if (!parent)
	  {
	    tmp2->parent = NULL;
	    *tree = tmp2;
	  }
	else
	  {
	    if (parent->right == node)
	      parent->right = tmp2;
	    else
	      parent->left = tmp2;
	    tmp2->parent = parent;
	  }

	recompute_height(node);
	recompute_height(tmp);
      }
}

void add_to_tree(tree_t **tree, tree_t *node)
{
  tree_t	*parent;
  int		balance;

  if (!node)
    return;
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
      node->parent = parent;
      if (parent->min_addr > node->high_addr)
	parent->left = node;
      else
	parent->right = node;
      
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

void *get_node(tree_t *tree, void *addr)
{
  if (!tree)
    return NULL;

  if (tree->high_addr < addr)
    return get_node(tree->right, addr);
  else if (tree->min_addr > addr)
    return get_node(tree->left, addr);

  return tree;
}

void del_from_tree(tree_t **tree, void *start_addr,
		      void (* free_func)(void *))
{
  tree_t	*node = get_node(*tree, start_addr);
  tree_t	*to_switch;
  tree_t	*parent_node;
  tree_t	*parent_switch;
  tree_t	*tmp_node;
  int		tmp_height;
  int		balance;

  if (!node)
    return;
  parent_node = node->parent;

  if (!(node->right) && !(node->left))
    del_leaf(tree, node);
  // swap and delete
  else if (node->right && node->left)
    {
      to_switch = node->right;
      while (to_switch->left)
      	to_switch = to_switch->left;

      parent_switch = to_switch->parent;

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

      tmp_node = to_switch->left;
      to_switch->left = node->left;
      node->left = tmp_node;
      if (to_switch->left)
      	to_switch->left->parent = to_switch;
      if (node->left)
      	node->left->parent = node;
	
      tmp_node = to_switch->right;
      to_switch->right = node->right;
      node->right = tmp_node;
      if (to_switch->right)
      	to_switch->right->parent = to_switch;
      if (node->right)
      	node->right->parent = node;

      if (!(node->height))
      	del_leaf(tree, node);
      else
      	del_branch(tree, node);
    }
  else
    del_branch(tree, node);

  if (free_func)
    free_func(node->data);
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

  if (tree->high_addr < addr)
    return search_on_tree(tree->right, addr);
  else if (tree->min_addr > addr)
    return search_on_tree(tree->left, addr);

  return tree->data;
}

void clean_tree(tree_t **tree, void (* free_func)(void *))
{
  while (*tree)
    del_from_tree(tree, (*tree)->min_addr, free_func);
}
 
