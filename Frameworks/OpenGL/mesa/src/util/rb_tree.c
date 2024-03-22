/*
 * Copyright © 2017 Faith Ekstrand
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "rb_tree.h"

/** \file rb_tree.c
 *
 * An implementation of a red-black tree
 *
 * This file implements the guts of a red-black tree.  The implementation
 * is mostly based on the one in "Introduction to Algorithms", third
 * edition, by Cormen, Leiserson, Rivest, and Stein.  The primary
 * divergence in our algorithms from those presented in CLRS is that we use
 * NULL for the leaves instead of a sentinel.  This means we have to do a
 * tiny bit more tracking in our implementation of delete but it makes the
 * algorithms far more explicit than stashing stuff in the sentinel.
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "macros.h"

static bool
rb_node_is_black(struct rb_node *n)
{
    /* NULL nodes are leaves and therefore black */
    return (n == NULL) || (n->parent & 1);
}

static bool
rb_node_is_red(struct rb_node *n)
{
    return !rb_node_is_black(n);
}

static void
rb_node_set_black(struct rb_node *n)
{
    n->parent |= 1;
}

static void
rb_node_set_red(struct rb_node *n)
{
    n->parent &= ~1ull;
}

static void
rb_node_copy_color(struct rb_node *dst, struct rb_node *src)
{
    dst->parent = (dst->parent & ~1ull) | (src->parent & 1);
}

static void
rb_node_set_parent(struct rb_node *n, struct rb_node *p)
{
    n->parent = (n->parent & 1) | (uintptr_t)p;
}

static struct rb_node *
rb_node_minimum(struct rb_node *node)
{
    while (node->left)
        node = node->left;
    return node;
}

static struct rb_node *
rb_node_maximum(struct rb_node *node)
{
    while (node->right)
        node = node->right;
    return node;
}

/**
 * Replace the subtree of T rooted at u with the subtree rooted at v
 *
 * This is called RB-transplant in CLRS.
 *
 * The node to be replaced is assumed to be a non-leaf.
 */
static void
rb_tree_splice(struct rb_tree *T, struct rb_node *u, struct rb_node *v)
{
    assert(u);
    struct rb_node *p = rb_node_parent(u);
    if (p == NULL) {
        assert(T->root == u);
        T->root = v;
    } else if (u == p->left) {
        p->left = v;
    } else {
        assert(u == p->right);
        p->right = v;
    }
    if (v)
        rb_node_set_parent(v, p);
}

static void
rb_tree_rotate_left(struct rb_tree *T, struct rb_node *x,
                    void (*update)(struct rb_node *))
{
    assert(x && x->right);

    struct rb_node *y = x->right;
    x->right = y->left;
    if (y->left)
        rb_node_set_parent(y->left, x);
    rb_tree_splice(T, x, y);
    y->left = x;
    rb_node_set_parent(x, y);
    if (update) {
        update(x);
        update(y);
    }
}

static void
rb_tree_rotate_right(struct rb_tree *T, struct rb_node *y,
                     void (*update)(struct rb_node *))
{
    assert(y && y->left);

    struct rb_node *x = y->left;
    y->left = x->right;
    if (x->right)
        rb_node_set_parent(x->right, y);
    rb_tree_splice(T, y, x);
    x->right = y;
    rb_node_set_parent(y, x);
    if (update) {
        update(y);
        update(x);
    }
}

void
rb_augmented_tree_insert_at(struct rb_tree *T, struct rb_node *parent,
                            struct rb_node *node, bool insert_left,
                            void (*update)(struct rb_node *node))
{
    /* This sets null children, parent, and a color of red */
    memset(node, 0, sizeof(*node));

    if (update)
       update(node);

    if (parent == NULL) {
        assert(T->root == NULL);
        T->root = node;
        rb_node_set_black(node);
        return;
    }

    if (insert_left) {
        assert(parent->left == NULL);
        parent->left = node;
    } else {
        assert(parent->right == NULL);
        parent->right = node;
    }
    rb_node_set_parent(node, parent);

    if (update) {
        struct rb_node *p = parent;
        while (p) {
            update(p);
            p = rb_node_parent(p);
        }
    }

    /* Now we do the insertion fixup */
    struct rb_node *z = node;
    while (rb_node_is_red(rb_node_parent(z))) {
        struct rb_node *z_p = rb_node_parent(z);
        assert(z == z_p->left || z == z_p->right);
        struct rb_node *z_p_p = rb_node_parent(z_p);
        assert(z_p_p != NULL);
        if (z_p == z_p_p->left) {
            struct rb_node *y = z_p_p->right;
            if (rb_node_is_red(y)) {
                rb_node_set_black(z_p);
                rb_node_set_black(y);
                rb_node_set_red(z_p_p);
                z = z_p_p;
            } else {
                if (z == z_p->right) {
                    z = z_p;
                    rb_tree_rotate_left(T, z, update);
                    /* We changed z */
                    z_p = rb_node_parent(z);
                    assert(z == z_p->left || z == z_p->right);
                    z_p_p = rb_node_parent(z_p);
                }
                rb_node_set_black(z_p);
                rb_node_set_red(z_p_p);
                rb_tree_rotate_right(T, z_p_p, update);
            }
        } else {
            struct rb_node *y = z_p_p->left;
            if (rb_node_is_red(y)) {
                rb_node_set_black(z_p);
                rb_node_set_black(y);
                rb_node_set_red(z_p_p);
                z = z_p_p;
            } else {
                if (z == z_p->left) {
                    z = z_p;
                    rb_tree_rotate_right(T, z, update);
                    /* We changed z */
                    z_p = rb_node_parent(z);
                    assert(z == z_p->left || z == z_p->right);
                    z_p_p = rb_node_parent(z_p);
                }
                rb_node_set_black(z_p);
                rb_node_set_red(z_p_p);
                rb_tree_rotate_left(T, z_p_p, update);
            }
        }
    }
    rb_node_set_black(T->root);
}

void
rb_augmented_tree_remove(struct rb_tree *T, struct rb_node *z,
                         void (*update)(struct rb_node *))
{
    /* x_p is always the parent node of X.  We have to track this
     * separately because x may be NULL.
     */
    struct rb_node *x, *x_p;
    struct rb_node *y = z;
    bool y_was_black = rb_node_is_black(y);
    if (z->left == NULL) {
        x = z->right;
        x_p = rb_node_parent(z);
        rb_tree_splice(T, z, x);
    } else if (z->right == NULL) {
        x = z->left;
        x_p = rb_node_parent(z);
        rb_tree_splice(T, z, x);
    } else {
        /* Find the minimum sub-node of z->right */
        y = rb_node_minimum(z->right);
        y_was_black = rb_node_is_black(y);

        x = y->right;
        if (rb_node_parent(y) == z) {
            x_p = y;
        } else {
            x_p = rb_node_parent(y);
            rb_tree_splice(T, y, x);
            y->right = z->right;
            rb_node_set_parent(y->right, y);
        }
        assert(y->left == NULL);
        rb_tree_splice(T, z, y);
        y->left = z->left;
        rb_node_set_parent(y->left, y);
        rb_node_copy_color(y, z);
    }

    assert(x_p == NULL || x == x_p->left || x == x_p->right);

    if (update) {
        struct rb_node *p = x_p;
        while (p) {
            update(p);
            p = rb_node_parent(p);
        }
    }

    if (!y_was_black)
        return;

    /* Fixup RB tree after the delete */
    while (x != T->root && rb_node_is_black(x)) {
        if (x == x_p->left) {
            struct rb_node *w = x_p->right;
            if (rb_node_is_red(w)) {
                rb_node_set_black(w);
                rb_node_set_red(x_p);
                rb_tree_rotate_left(T, x_p, update);
                assert(x == x_p->left);
                w = x_p->right;
            }
            if (rb_node_is_black(w->left) && rb_node_is_black(w->right)) {
                rb_node_set_red(w);
                x = x_p;
            } else {
                if (rb_node_is_black(w->right)) {
                    rb_node_set_black(w->left);
                    rb_node_set_red(w);
                    rb_tree_rotate_right(T, w, update);
                    w = x_p->right;
                }
                rb_node_copy_color(w, x_p);
                rb_node_set_black(x_p);
                rb_node_set_black(w->right);
                rb_tree_rotate_left(T, x_p, update);
                x = T->root;
            }
        } else {
            struct rb_node *w = x_p->left;
            if (rb_node_is_red(w)) {
                rb_node_set_black(w);
                rb_node_set_red(x_p);
                rb_tree_rotate_right(T, x_p, update);
                assert(x == x_p->right);
                w = x_p->left;
            }
            if (rb_node_is_black(w->right) && rb_node_is_black(w->left)) {
                rb_node_set_red(w);
                x = x_p;
            } else {
                if (rb_node_is_black(w->left)) {
                    rb_node_set_black(w->right);
                    rb_node_set_red(w);
                    rb_tree_rotate_left(T, w, update);
                    w = x_p->left;
                }
                rb_node_copy_color(w, x_p);
                rb_node_set_black(x_p);
                rb_node_set_black(w->left);
                rb_tree_rotate_right(T, x_p, update);
                x = T->root;
            }
        }
        x_p = rb_node_parent(x);
    }
    if (x)
        rb_node_set_black(x);
}

struct rb_node *
rb_tree_first(struct rb_tree *T)
{
    return T->root ? rb_node_minimum(T->root) : NULL;
}

struct rb_node *
rb_tree_last(struct rb_tree *T)
{
    return T->root ? rb_node_maximum(T->root) : NULL;
}

struct rb_node *
rb_node_next(struct rb_node *node)
{
    if (node->right) {
        /* If we have a right child, then the next thing (compared to this
         * node) is the left-most child of our right child.
         */
        return rb_node_minimum(node->right);
    } else {
        /* If node doesn't have a right child, crawl back up the to the
         * left until we hit a parent to the right.
         */
        struct rb_node *p = rb_node_parent(node);
        while (p && node == p->right) {
            node = p;
            p = rb_node_parent(node);
        }
        assert(p == NULL || node == p->left);
        return p;
    }
}

struct rb_node *
rb_node_prev(struct rb_node *node)
{
    if (node->left) {
        /* If we have a left child, then the previous thing (compared to
         * this node) is the right-most child of our left child.
         */
        return rb_node_maximum(node->left);
    } else {
        /* If node doesn't have a left child, crawl back up the to the
         * right until we hit a parent to the left.
         */
        struct rb_node *p = rb_node_parent(node);
        while (p && node == p->left) {
            node = p;
            p = rb_node_parent(node);
        }
        assert(p == NULL || node == p->right);
        return p;
    }
}

/* Return the first node in an interval tree that intersects a given interval
 * or point. The tests against the interval and the max field are abstracted
 * via function pointers, so that this works for any type of interval.
 */
static struct rb_node *
rb_node_min_intersecting(struct rb_node *node, void *interval,
                         int (*cmp_interval)(const struct rb_node *node,
                                             const void *interval),
                         bool (*cmp_max)(const struct rb_node *node, 
                                         const void *interval))
{
    if (!cmp_max(node, interval))
        return NULL;

    while (node) {
        int cmp = cmp_interval(node, interval);

        /* If the node's interval is entirely to the right of the interval
         * we're searching for, all of its right descendants are also to the
         * right and don't intersect so we have to search to the left.
         */
        if (cmp > 0) {
            node = node->left;
            continue;
        }

        /* The interval overlaps or is to the left. This must also be true for
         * its left descendants because their start points are to the left of
         * node's. We can use the max to tell if there is an interval in its
         * left descendants which overlaps our interval, in which case we
         * should descend to the left.
         */
        if (node->left && cmp_max(node->left, interval)) {
            node = node->left;
            continue;
        }

        /* Now the only possibilities are the node's interval intersects the
         * interval or one of its right descendants does.
         */
        if (cmp == 0)
            return node;

        node = node->right;
        if (node && !cmp_max(node, interval))
            return NULL;
    }

    return NULL;
}

/* Return the next node after "node" that intersects a given interval.
 *
 * Because rb_node_min_intersecting() takes O(log n) time and may be called up
 * to O(log n) times, in addition to the O(log n) crawl up the tree, a naive
 * runtime analysis would show that this takes O((log n)^2) time, but actually
 * it's O(log n). Proving this is tricky:
 *
 * Call the rightmost node in the tree whose start is before the end of the
 * interval we're searching for N. All nodes after N in the tree are to the
 * right of the interval. We'll divide the search into two phases: in phase 1,
 * "node" is *not* an ancestor of N, and in phase 2 it is. Because we always
 * crawl up the tree, phase 2 cannot turn back into phase 1, but phase 1 may
 * be followed by phase 2. We'll prove that the calls to
 * rb_node_min_intersecting() take O(log n) time in both phases.
 *
 * Phase 1: Because "node" is to the left of N and N isn't a descendant of
 * "node", the start of every interval in "node"'s subtree must be less than
 * or equal to N's start which is less than or equal to the end of the search
 * interval. Furthermore, either "node"'s max_end is less than the start of
 * the interval, in which case rb_node_min_intersecting() immediately returns
 * NULL, or some descendant has an end equal to "node"'s max_end which is
 * greater than or equal to the search interval's start, and therefore it
 * intersects the search interval and rb_node_min_intersecting() must return
 * non-NULL which causes us to terminate. rb_node_min_intersecting() is called
 * O(log n) times, with all but the last call taking constant time and the
 * last call taking O(log n), so the overall runtime is O(log n).
 *
 * Phase 2: After the first call to rb_node_min_intersecting, we may crawl up
 * the tree until we get to a node p where "node", and therefore N, is in p's
 * left subtree. However this means that p is to the right of N in the tree
 * and is therefore to the right of the search interval, and the search
 * terminates on the first iteration of the loop so that
 * rb_node_min_intersecting() is only called once.
 */
static struct rb_node *
rb_node_next_intersecting(struct rb_node *node,
                          void *interval,
                          int (*cmp_interval)(const struct rb_node *node,
                                              const void *interval),
                          bool (*cmp_max)(const struct rb_node *node,
                                          const void *interval))
{
    while (true) {
        /* The first place to search is the node's right subtree. */
        if (node->right) {
            struct rb_node *next =
                rb_node_min_intersecting(node->right, interval, cmp_interval, cmp_max);
            if (next)
                return next;
        }

        /* If we don't find a matching interval there, crawl up the tree until
         * we find an ancestor to the right. This is the next node after the
         * right subtree which we determined didn't match.
         */
        struct rb_node *p = rb_node_parent(node);
        while (p && node == p->right) {
            node = p;
            p = rb_node_parent(node);
        }
        assert(p == NULL || node == p->left);

        /* Check if we've searched everything in the tree. */
        if (!p)
            return NULL;

        int cmp = cmp_interval(p, interval);

        /* If it intersects, return it. */
        if (cmp == 0)
            return p;

        /* If it's to the right of the interval, all following nodes will be
         * to the right and we can bail early.
         */
        if (cmp > 0)
            return NULL;

        node = p;
    }
}

static int
uinterval_cmp(struct uinterval a, struct uinterval b)
{
    if (a.end < b.start)
        return -1;
    else if (b.end < a.start)
        return 1;
    else
        return 0;
}

static int
uinterval_node_cmp(const struct rb_node *_a, const struct rb_node *_b)
{
    const struct uinterval_node *a = rb_node_data(struct uinterval_node, _a, node);
    const struct uinterval_node *b = rb_node_data(struct uinterval_node, _b, node);

    return (int) (b->interval.start - a->interval.start);
}

static int
uinterval_search_cmp(const struct rb_node *_node, const void *_interval)
{
    const struct uinterval_node *node = rb_node_data(struct uinterval_node, _node, node);
    const struct uinterval *interval = _interval;

    return uinterval_cmp(node->interval, *interval);
}

static bool
uinterval_max_cmp(const struct rb_node *_node, const void *data)
{
    const struct uinterval_node *node = rb_node_data(struct uinterval_node, _node, node);
    const struct uinterval *interval = data;

    return node->max_end >= interval->start;
}

static void
uinterval_update_max(struct rb_node *_node)
{
    struct uinterval_node *node = rb_node_data(struct uinterval_node, _node, node);
    node->max_end = node->interval.end;
    if (node->node.left) {
        struct uinterval_node *left = rb_node_data(struct uinterval_node, node->node.left, node);
        node->max_end = MAX2(node->max_end, left->max_end);
    }
    if (node->node.right) {
        struct uinterval_node *right = rb_node_data(struct uinterval_node, node->node.right, node);
        node->max_end = MAX2(node->max_end, right->max_end);
    }
}

void
uinterval_tree_insert(struct rb_tree *tree, struct uinterval_node *node)
{
    rb_augmented_tree_insert(tree, &node->node, uinterval_node_cmp,
                             uinterval_update_max);
}

void
uinterval_tree_remove(struct rb_tree *tree, struct uinterval_node *node)
{
    rb_augmented_tree_remove(tree, &node->node, uinterval_update_max);
}

struct uinterval_node *
uinterval_tree_first(struct rb_tree *tree, struct uinterval interval)
{
    if (!tree->root)
        return NULL;

    struct rb_node *node =
        rb_node_min_intersecting(tree->root, &interval, uinterval_search_cmp,
                                 uinterval_max_cmp);

    return node ? rb_node_data(struct uinterval_node, node, node) : NULL;
}

struct uinterval_node *
uinterval_node_next(struct uinterval_node *node,
                    struct uinterval interval)
{
    struct rb_node *next =
        rb_node_next_intersecting(&node->node, &interval, uinterval_search_cmp,
                                  uinterval_max_cmp);

    return next ? rb_node_data(struct uinterval_node, next, node) : NULL;
}

static void
validate_rb_node(struct rb_node *n, int black_depth)
{
    if (n == NULL) {
        assert(black_depth == 0);
        return;
    }

    if (rb_node_is_black(n)) {
        black_depth--;
    } else {
        assert(rb_node_is_black(n->left));
        assert(rb_node_is_black(n->right));
    }

    validate_rb_node(n->left, black_depth);
    validate_rb_node(n->right, black_depth);
}

void
rb_tree_validate(struct rb_tree *T)
{
    if (T->root == NULL)
        return;

    assert(rb_node_is_black(T->root));

    unsigned black_depth = 0;
    for (struct rb_node *n = T->root; n; n = n->left) {
        if (rb_node_is_black(n))
            black_depth++;
    }

    validate_rb_node(T->root, black_depth);
}
