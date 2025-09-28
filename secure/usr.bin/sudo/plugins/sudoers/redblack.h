/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2004, 2007, 2010, 2013
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

#ifndef SUDOERS_REDBLACK_H
#define SUDOERS_REDBLACK_H

enum rbcolor {
    red,
    black
};

enum rbtraversal {
    preorder,
    inorder,
    postorder
};

struct rbnode {
    struct rbnode *left, *right, *parent;
    void *data;
    enum rbcolor color;
};

struct rbtree {
    int (*compar)(const void *, const void *);
    struct rbnode root;
    struct rbnode nil;
};

#define rbapply(t, f, c, o)	rbapply_node((t), (t)->root.left, (f), (c), (o))
#define rbisempty(t)		((t)->root.left == &(t)->nil && (t)->root.right == &(t)->nil)
#define rbfirst(t)		((t)->root.left)
#define rbroot(t)		(&(t)->root)
#define rbnil(t)		(&(t)->nil)

void *rbdelete(struct rbtree *, struct rbnode *);
int rbapply_node(struct rbtree *, struct rbnode *,
	int (*)(void *, void *), void *, enum rbtraversal);
struct rbnode *rbfind(struct rbtree *, void *);
int rbinsert(struct rbtree *, void *, struct rbnode **);
struct rbtree *rbcreate(int (*)(const void *, const void *));
void rbdestroy(struct rbtree *, void (*)(void *));

#endif /* SUDOERS_REDBLACK_H */
