/*
  Red Black Trees
  (C) 1999  Andrea Arcangeli <andrea@suse.de>
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  linux/lib/rbtree.c
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include "rbtree.h"

static void __rb_rotate_left(rb_node_t * node, rb_node_t ** root)
{
	rb_node_t * right = node->rb_right;

	if ((node->rb_right = right->rb_left))
		right->rb_left->rb_parent = node;
	right->rb_left = node;

	if ((right->rb_parent = node->rb_parent))
	{
		if (node == node->rb_parent->rb_left)
			node->rb_parent->rb_left = right;
		else
			node->rb_parent->rb_right = right;
	}
	else
		*root = right;
		//root->rb_node = right;
	node->rb_parent = right;
}

static void __rb_rotate_right(rb_node_t * node, rb_node_t ** root)
{
	rb_node_t * left = node->rb_left;

	if ((node->rb_left = left->rb_right))
		left->rb_right->rb_parent = node;
	left->rb_right = node;

	if ((left->rb_parent = node->rb_parent))
	{
		if (node == node->rb_parent->rb_right)
			node->rb_parent->rb_right = left;
		else
			node->rb_parent->rb_left = left;
	}
	else
		*root = left;
		//root->rb_node = left;
	node->rb_parent = left;
}

void rb_insert_color(rb_node_t * node, rb_node_t ** root)
{
	rb_node_t * parent, * gparent;

	while ((parent = node->rb_parent) && parent->rb_color == RB_RED)
	{
		gparent = parent->rb_parent;

		if (parent == gparent->rb_left)
		{
			{
				register rb_node_t * uncle = gparent->rb_right;
				if (uncle && uncle->rb_color == RB_RED)
				{
					uncle->rb_color = RB_BLACK;
					parent->rb_color = RB_BLACK;
					gparent->rb_color = RB_RED;
					node = gparent;
					continue;
				}
			}

			if (parent->rb_right == node)
			{
				register rb_node_t * tmp;
				__rb_rotate_left(parent, root);
				tmp = parent;
				parent = node;
				node = tmp;
			}

			parent->rb_color = RB_BLACK;
			gparent->rb_color = RB_RED;
			__rb_rotate_right(gparent, root);
		} else {
			{
				register rb_node_t * uncle = gparent->rb_left;
				if (uncle && uncle->rb_color == RB_RED)
				{
					uncle->rb_color = RB_BLACK;
					parent->rb_color = RB_BLACK;
					gparent->rb_color = RB_RED;
					node = gparent;
					continue;
				}
			}

			if (parent->rb_left == node)
			{
				register rb_node_t * tmp;
				__rb_rotate_right(parent, root);
				tmp = parent;
				parent = node;
				node = tmp;
			}

			parent->rb_color = RB_BLACK;
			gparent->rb_color = RB_RED;
			__rb_rotate_left(gparent, root);
		}
	}

	(*root)->rb_color = RB_BLACK;
}


static rb_node_t * rb_search(rb_node_t *root,
		int key)
{

	while (root)
	{
		if (key < root->key)
			root = root->rb_left;
		else if (key > root->key)
			root = root->rb_right;
		else
			return root;
	}
	return NULL;
}

static rb_node_t * __rb_insert(rb_node_t ** root,
		rb_node_t * node)
{
	rb_node_t ** p = root;
	rb_node_t * parent = NULL;

	while (*p)
	{
		parent = *p;

		if (node->key < parent->key)
			p = &(*p)->rb_left;
		else if (node->key > parent->key)
			p = &(*p)->rb_right;
		else
			return parent;
	}

	rb_link_node(node, parent, p);

	return NULL;
}

static rb_node_t * rb_insert(rb_node_t ** root, 
		rb_node_t * node)
{
	rb_node_t * ret;
	if ((ret = __rb_insert(root, node)))
		goto out;
	rb_insert_color(node, root);
out:
	return ret;
}

static rb_node_t * create_node(int key) {
	rb_node_t *p = malloc(sizeof(rb_node_t));
	p->key = key;
	return p;
}

static void print_tree(rb_node_t *root) {
	if (!root)
		return;
	print_tree(root->rb_left);
	printf("%d\n", root->key);
	print_tree(root->rb_right);
}

int main() {
	rb_node_t *mytree = NULL;
	int i;
	struct timeval t_start, t_end;
	double t_diff;
	srand(0);
	
	gettimeofday(&t_start,NULL);
	for (i = 0; i < 10240; i++) {
		rb_insert(&mytree, create_node(rand() % 32768));
	}

	for (i = 0; i < 1000; i++)
		print_tree(mytree);

    gettimeofday(&t_end,NULL);
    t_diff = (t_end.tv_sec - t_start.tv_sec) + (double)(t_end.tv_usec - t_start.tv_usec)/1000000;
	fprintf(stderr, " ---  time spent = %.6f  --- \n", t_diff);
	return 0;
	
}
