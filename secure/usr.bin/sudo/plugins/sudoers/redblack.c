/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2004-2005, 2007, 2009-2015
 *	Todd C. Miller <Todd.Miller@sudo.ws>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 */

/*
 * Adapted from the following code written by Emin Martinian:
 * http://web.mit.edu/~emin/www/source_code/red_black_tree/index.html
 *
 * Copyright (c) 2001 Emin Martinian
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that neither the name of Emin
 * Martinian nor the names of any contributors are be used to endorse or
 * promote products derived from this software without specific prior
 * written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>

#include <sudoers.h>
#include <redblack.h>

static void rbrepair(struct rbtree *, struct rbnode *);
static void rotate_left(struct rbtree *, struct rbnode *);
static void rotate_right(struct rbtree *, struct rbnode *);
static void rbdestroy_int(struct rbtree *, struct rbnode *, void (*)(void *));

/*
 * Red-Black tree, see http://en.wikipedia.org/wiki/Red-black_tree
 *
 * A red-black tree is a binary search tree where each node has a color
 * attribute, the value of which is either red or black.  Essentially, it
 * is just a convenient way to express a 2-3-4 binary search tree where
 * the color indicates whether the node is part of a 3-node or a 4-node.
 * In addition to the ordinary requirements imposed on binary search
 * trees, we make the following additional requirements of any valid
 * red-black tree:
 *  1) Every node is either red or black.
 *  2) The root is black.
 *  3) All leaves are black.
 *  4) Both children of each red node are black.
 *  5) The paths from each leaf up to the root each contain the same
 *     number of black nodes.
 */

/*
 * Create a red black tree struct using the specified compare routine.
 * Allocates and returns the initialized (empty) tree or NULL if
 * memory cannot be allocated.
 */
struct rbtree *
rbcreate(int (*compar)(const void *, const void*))
{
    struct rbtree *tree;
    debug_decl(rbcreate, SUDOERS_DEBUG_RBTREE);

    if ((tree = malloc(sizeof(*tree))) == NULL) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "unable to allocate memory");
	debug_return_ptr(NULL);
    }

    tree->compar = compar;

    /*
     * We use a self-referencing sentinel node called nil to simplify the
     * code by avoiding the need to check for NULL pointers.
     */
    tree->nil.left = tree->nil.right = tree->nil.parent = &tree->nil;
    tree->nil.color = black;
    tree->nil.data = NULL;

    /*
     * Similarly, the fake root node keeps us from having to worry
     * about splitting the root.
     */
    tree->root.left = tree->root.right = tree->root.parent = &tree->nil; // -V778
    tree->root.color = black;
    tree->root.data = NULL;

    debug_return_ptr(tree);
}

/*
 * Perform a left rotation starting at node.
 */
static void
rotate_left(struct rbtree *tree, struct rbnode *node)
{
    struct rbnode *child;
    debug_decl(rotate_left, SUDOERS_DEBUG_RBTREE);

    child = node->right;
    node->right = child->left;

    if (child->left != rbnil(tree))
        child->left->parent = node;
    child->parent = node->parent;

    if (node == node->parent->left)
	node->parent->left = child;
    else
	node->parent->right = child;
    child->left = node;
    node->parent = child;

    debug_return;
}

/*
 * Perform a right rotation starting at node.
 */
static void
rotate_right(struct rbtree *tree, struct rbnode *node)
{
    struct rbnode *child;
    debug_decl(rotate_right, SUDOERS_DEBUG_RBTREE);

    child = node->left;
    node->left = child->right;

    if (child->right != rbnil(tree))
        child->right->parent = node;
    child->parent = node->parent;

    if (node == node->parent->left)
	node->parent->left = child;
    else
	node->parent->right = child;
    child->right = node;
    node->parent = child;

    debug_return;
}

/*
 * Insert data pointer into a redblack tree.
 * Returns a 0 on success, 1 if a node matching "data" already exists
 * (filling in "existing" if not NULL), or -1 on malloc() failure.
 */
int
rbinsert(struct rbtree *tree, void *data, struct rbnode **existing)
{
    struct rbnode *node = rbfirst(tree);
    struct rbnode *parent = rbroot(tree);
    int res;
    debug_decl(rbinsert, SUDOERS_DEBUG_RBTREE);

    /* Find correct insertion point. */
    while (node != rbnil(tree)) {
	parent = node;
	if ((res = tree->compar(data, node->data)) == 0) {
	    if (existing != NULL)
		*existing = node;
	    debug_return_int(1);
	}
	node = res < 0 ? node->left : node->right;
    }

    node = malloc(sizeof(*node));
    if (node == NULL) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "unable to allocate memory");
	debug_return_int(-1);
    }
    node->data = data;
    node->left = node->right = rbnil(tree);
    node->parent = parent;
    if (parent == rbroot(tree) || tree->compar(data, parent->data) < 0)
	parent->left = node;
    else
	parent->right = node;
    node->color = red;

    /*
     * If the parent node is black we are all set, if it is red we have
     * the following possible cases to deal with.  We iterate through
     * the rest of the tree to make sure none of the required properties
     * is violated.
     *
     *	1) The uncle is red.  We repaint both the parent and uncle black
     *     and repaint the grandparent node red.
     *
     *  2) The uncle is black and the new node is the right child of its
     *     parent, and the parent in turn is the left child of its parent.
     *     We do a left rotation to switch the roles of the parent and
     *     child, relying on further iterations to fixup the old parent.
     *
     *  3) The uncle is black and the new node is the left child of its
     *     parent, and the parent in turn is the left child of its parent.
     *     We switch the colors of the parent and grandparent and perform
     *     a right rotation around the grandparent.  This makes the former
     *     parent the parent of the new node and the former grandparent.
     *
     * Note that because we use a sentinel for the root node we never
     * need to worry about replacing the root.
     */
    while (node->parent->color == red) {
	struct rbnode *uncle;
	if (node->parent == node->parent->parent->left) {
	    uncle = node->parent->parent->right;
	    if (uncle->color == red) {
		node->parent->color = black;
		uncle->color = black;
		node->parent->parent->color = red;
		node = node->parent->parent;
	    } else /* if (uncle->color == black) */ {
		if (node == node->parent->right) {
		    node = node->parent;
		    rotate_left(tree, node);
		}
		node->parent->color = black;
		node->parent->parent->color = red;
		rotate_right(tree, node->parent->parent);
	    }
	} else { /* if (node->parent == node->parent->parent->right) */
	    uncle = node->parent->parent->left;
	    if (uncle->color == red) {
		node->parent->color = black;
		uncle->color = black;
		node->parent->parent->color = red;
		node = node->parent->parent;
	    } else /* if (uncle->color == black) */ {
		if (node == node->parent->left) {
		    node = node->parent;
		    rotate_right(tree, node);
		}
		node->parent->color = black;
		node->parent->parent->color = red;
		rotate_left(tree, node->parent->parent);
	    }
	}
    }
    rbfirst(tree)->color = black;	/* first node is always black */
    debug_return_int(0);
}

/*
 * Look for a node matching key in tree.
 * Returns a pointer to the node if found, else NULL.
 */
struct rbnode *
rbfind(struct rbtree *tree, void *key)
{
    struct rbnode *node = rbfirst(tree);
    int res;
    debug_decl(rbfind, SUDOERS_DEBUG_RBTREE);

    while (node != rbnil(tree)) {
	if ((res = tree->compar(key, node->data)) == 0)
	    debug_return_ptr(node);
	node = res < 0 ? node->left : node->right;
    }
    debug_return_ptr(NULL);
}

/*
 * Call func() for each node, passing it the node data and a cookie;
 * If func() returns non-zero for a node, the traversal stops and the
 * error value is returned.  Returns 0 on successful traversal.
 */
int
rbapply_node(struct rbtree *tree, struct rbnode *node,
    int (*func)(void *, void *), void *cookie, enum rbtraversal order)
{
    int error;
    debug_decl(rbapply_node, SUDOERS_DEBUG_RBTREE);

    if (node != rbnil(tree)) {
	if (order == preorder)
	    if ((error = func(node->data, cookie)) != 0)
		debug_return_int(error);
	if ((error = rbapply_node(tree, node->left, func, cookie, order)) != 0)
	    debug_return_int(error);
	if (order == inorder)
	    if ((error = func(node->data, cookie)) != 0)
		debug_return_int(error);
	if ((error = rbapply_node(tree, node->right, func, cookie, order)) != 0)
	    debug_return_int(error);
	if (order == postorder)
	    if ((error = func(node->data, cookie)) != 0)
		debug_return_int(error);
    }
    debug_return_int(0);
}

/*
 * Returns the successor of node, or nil if there is none.
 */
static struct rbnode *
rbsuccessor(struct rbtree *tree, struct rbnode *node)
{
    struct rbnode *succ;
    debug_decl(rbsuccessor, SUDOERS_DEBUG_RBTREE);

    if ((succ = node->right) != rbnil(tree)) {
	while (succ->left != rbnil(tree))
	    succ = succ->left;
    } else {
	/* No right child, move up until we find it or hit the root */
	for (succ = node->parent; node == succ->right; succ = succ->parent)
	    node = succ;
	if (succ == rbroot(tree))
	    succ = rbnil(tree);
    }
    debug_return_ptr(succ);
}

/*
 * Recursive portion of rbdestroy().
 */
static void
rbdestroy_int(struct rbtree *tree, struct rbnode *node, void (*destroy)(void *))
{
    debug_decl(rbdestroy_int, SUDOERS_DEBUG_RBTREE);
    if (node != rbnil(tree)) {
	rbdestroy_int(tree, node->left, destroy);
	rbdestroy_int(tree, node->right, destroy);
	if (destroy != NULL)
	    destroy(node->data);
	free(node);
    }
    debug_return;
}

/*
 * Destroy the specified tree, calling the destructor "destroy"
 * for each node and then freeing the tree itself.
 */
void
rbdestroy(struct rbtree *tree, void (*destroy)(void *))
{
    debug_decl(rbdestroy, SUDOERS_DEBUG_RBTREE);
    rbdestroy_int(tree, rbfirst(tree), destroy);
    free(tree);
    debug_return;
}

/*
 * Delete node 'z' from the tree and return its data pointer.
 */
void *rbdelete(struct rbtree *tree, struct rbnode *z)
{
    struct rbnode *x, *y;
    void *data = z->data;
    debug_decl(rbdelete, SUDOERS_DEBUG_RBTREE);

    if (z->left == rbnil(tree) || z->right == rbnil(tree))
	y = z;
    else
	y = rbsuccessor(tree, z);
    x = (y->left == rbnil(tree)) ? y->right : y->left;

    if ((x->parent = y->parent) == rbroot(tree)) {
	rbfirst(tree) = x;
    } else {
	if (y == y->parent->left)
	    y->parent->left = x;
	else
	    y->parent->right = x;
    }
    if (y->color == black)
	rbrepair(tree, x);
    if (y != z) {
	y->left = z->left;
	y->right = z->right;
	y->parent = z->parent;
	y->color = z->color;
	z->left->parent = z->right->parent = y;
	if (z == z->parent->left)
	    z->parent->left = y; 
	else
	    z->parent->right = y;
    }
    free(z); 
    
    debug_return_ptr(data);
}

/*
 * Repair the tree after a node has been deleted by rotating and repainting
 * colors to restore the 4 properties inherent in red-black trees.
 */
static void
rbrepair(struct rbtree *tree, struct rbnode *node)
{
    struct rbnode *sibling;
    debug_decl(rbrepair, SUDOERS_DEBUG_RBTREE);

    while (node->color == black && node != rbfirst(tree)) {
	if (node == node->parent->left) {
	    sibling = node->parent->right;
	    if (sibling->color == red) {
		sibling->color = black;
		node->parent->color = red;
		rotate_left(tree, node->parent);
		sibling = node->parent->right;
	    }
	    if (sibling->right->color == black && sibling->left->color == black) {
		sibling->color = red;
		node = node->parent;
	    } else {
		if (sibling->right->color == black) {
		      sibling->left->color = black;
		      sibling->color = red;
		      rotate_right(tree, sibling);
		      sibling = node->parent->right;
		}
		sibling->color = node->parent->color;
		node->parent->color = black;
		sibling->right->color = black;
		rotate_left(tree, node->parent);
		node = rbfirst(tree); /* exit loop */
	    }
	} else { /* if (node == node->parent->right) */
	    sibling = node->parent->left;
	    if (sibling->color == red) {
		sibling->color = black;
		node->parent->color = red;
		rotate_right(tree, node->parent);
		sibling = node->parent->left;
	    }
	    if (sibling->right->color == black && sibling->left->color == black) {
		sibling->color = red;
		node = node->parent;
	    } else {
		if (sibling->left->color == black) {
		    sibling->right->color = black;
		    sibling->color = red;
		    rotate_left(tree, sibling);
		    sibling = node->parent->left;
		}
		sibling->color = node->parent->color;
		node->parent->color = black;
		sibling->left->color = black;
		rotate_right(tree, node->parent);
		node = rbfirst(tree); /* exit loop */
	    }
	}
    }
    node->color = black;

    debug_return;
}
