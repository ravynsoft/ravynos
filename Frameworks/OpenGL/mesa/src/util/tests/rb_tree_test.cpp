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

#undef NDEBUG

#include "rb_tree.h"

#include <assert.h>
#include <gtest/gtest.h>
#include <limits.h>

#include "macros.h"

/* A list of 100 random numbers from 1 to 100.  The number 30 is explicitly
 * missing from this list.
 */
int test_numbers[] = {
    26, 12, 35, 15, 48, 11, 39, 23, 40, 18,
    39, 15, 40, 11, 42, 2, 5, 2, 28, 8,
    10, 22, 23, 38, 47, 12, 31, 22, 26, 39,
    9, 42, 32, 18, 36, 8, 32, 29, 9, 3,
    32, 49, 23, 11, 43, 41, 22, 42, 6, 35,
    38, 48, 5, 35, 39, 44, 22, 16, 16, 32,
    31, 50, 48, 5, 50, 8, 2, 32, 27, 34,
    42, 48, 22, 47, 10, 48, 39, 36, 28, 40,
    32, 33, 21, 17, 14, 38, 27, 6, 25, 18,
    32, 38, 19, 22, 20, 47, 50, 41, 29, 50,
};

#define NON_EXISTANT_NUMBER 30

struct rb_test_node {
    int key;
    struct rb_node node;
};

static int
rb_test_node_cmp_void(const struct rb_node *n, const void *v)
{
    struct rb_test_node *tn = rb_node_data(struct rb_test_node, n, node);
    return *(int *)v - tn->key;
}

static int
rb_test_node_cmp(const struct rb_node *a, const struct rb_node *b)
{
    struct rb_test_node *ta = rb_node_data(struct rb_test_node, a, node);
    struct rb_test_node *tb = rb_node_data(struct rb_test_node, b, node);

    return tb->key - ta->key;
}

static void
validate_tree_order(struct rb_tree *tree, unsigned expected_count)
{
    struct rb_test_node *prev = NULL;
    int max_val = -1;
    unsigned count = 0;
    rb_tree_foreach(struct rb_test_node, n, tree, node) {
        /* Everything should be in increasing order */
        assert(n->key >= max_val);
        if (n->key > max_val) {
            max_val = n->key;
        } else {
            /* Things should be stable, i.e., given equal keys, they should
             * show up in the list in order of insertion.  We insert them
             * in the order they are in in the array.
             */
            assert(prev == NULL || prev < n);
        }

        prev = n;
        count++;
    }
    assert(count == expected_count);

    prev = NULL;
    max_val = -1;
    count = 0;
    rb_tree_foreach_safe(struct rb_test_node, n, tree, node) {
        /* Everything should be in increasing order */
        assert(n->key >= max_val);
        if (n->key > max_val) {
            max_val = n->key;
        } else {
            /* Things should be stable, i.e., given equal keys, they should
             * show up in the list in order of insertion.  We insert them
             * in the order they are in in the array.
             */
            assert(prev == NULL || prev < n);
        }

        prev = n;
        count++;
    }
    assert(count == expected_count);

    prev = NULL;
    int min_val = INT_MAX;
    count = 0;
    rb_tree_foreach_rev(struct rb_test_node, n, tree, node) {
        /* Everything should be in increasing order */
        assert(n->key <= min_val);
        if (n->key < min_val) {
            min_val = n->key;
        } else {
            /* Things should be stable, i.e., given equal keys, they should
             * show up in the list in order of insertion.  We insert them
             * in the order they are in in the array.
             */
            assert(prev == NULL || prev > n);
        }

        prev = n;
        count++;
    }
    assert(count == expected_count);

    prev = NULL;
    min_val = INT_MAX;
    count = 0;
    rb_tree_foreach_rev_safe(struct rb_test_node, n, tree, node) {
        /* Everything should be in increasing order */
        assert(n->key <= min_val);
        if (n->key < min_val) {
            min_val = n->key;
        } else {
            /* Things should be stable, i.e., given equal keys, they should
             * show up in the list in order of insertion.  We insert them
             * in the order they are in in the array.
             */
            assert(prev == NULL || prev > n);
        }

        prev = n;
        count++;
    }
    assert(count == expected_count);
}

static void
validate_search(struct rb_tree *tree, int first_number,
                int last_number)
{
    struct rb_node *n;
    struct rb_test_node *tn;

    /* Search for all of the values */
    for (int i = first_number; i <= last_number; i++) {
        n = rb_tree_search(tree, &test_numbers[i], rb_test_node_cmp_void);
        tn = rb_node_data(struct rb_test_node, n, node);
        assert(tn->key == test_numbers[i]);

        n = rb_tree_search_sloppy(tree, &test_numbers[i],
                                  rb_test_node_cmp_void);
        tn = rb_node_data(struct rb_test_node, n, node);
        assert(tn->key == test_numbers[i]);
    }

    int missing_key = NON_EXISTANT_NUMBER;
    n = rb_tree_search(tree, &missing_key, rb_test_node_cmp_void);
    assert(n == NULL);

    n = rb_tree_search_sloppy(tree, &missing_key, rb_test_node_cmp_void);
    if (rb_tree_is_empty(tree)) {
        assert(n == NULL);
    } else {
        assert(n != NULL);
        tn = rb_node_data(struct rb_test_node, n, node);
        assert(tn->key != missing_key);
        if (tn->key < missing_key) {
            struct rb_node *next = rb_node_next(n);
            if (next != NULL) {
                struct rb_test_node *tnext =
                    rb_node_data(struct rb_test_node, next, node);
                assert(missing_key < tnext->key);
            }
        } else {
            struct rb_node *prev = rb_node_prev(n);
            if (prev != NULL) {
                struct rb_test_node *tprev =
                    rb_node_data(struct rb_test_node, prev, node);
                assert(missing_key > tprev->key);
            }
        }
    }
}

TEST(RBTreeTest, InsertAndSearch)
{
    struct rb_test_node nodes[ARRAY_SIZE(test_numbers)];
    struct rb_tree tree;

    rb_tree_init(&tree);

    for (unsigned i = 0; i < ARRAY_SIZE(test_numbers); i++) {
        nodes[i].key = test_numbers[i];
        rb_tree_insert(&tree, &nodes[i].node, rb_test_node_cmp);
        rb_tree_validate(&tree);
        validate_tree_order(&tree, i + 1);
        validate_search(&tree, 0, i);
    }

    for (unsigned i = 0; i < ARRAY_SIZE(test_numbers); i++) {
        rb_tree_remove(&tree, &nodes[i].node);
        rb_tree_validate(&tree);
        validate_tree_order(&tree, ARRAY_SIZE(test_numbers) - i - 1);
        validate_search(&tree, i + 1, ARRAY_SIZE(test_numbers) - 1);
    }
}

TEST(RBTreeTest, FindFirst)
{
    struct rb_test_node nodes[100];
    struct rb_tree tree;

    rb_tree_init(&tree);

    const int x = 13;

    for (unsigned i = 0; i < ARRAY_SIZE(nodes); i++) {
        nodes[i].key = x;
        rb_tree_insert(&tree, &nodes[i].node, rb_test_node_cmp);
    }

    struct rb_node *n = rb_tree_search(&tree, &x, rb_test_node_cmp_void);

    ASSERT_NE(nullptr, n);
    EXPECT_EQ(nullptr, rb_node_prev(n));
    EXPECT_EQ(n, rb_tree_first(&tree));
}

TEST(RBTreeTest, FindFirstOfMiddle)
{
    struct rb_test_node nodes[3 * 33];
    struct rb_tree tree;

    rb_tree_init(&tree);

    unsigned i;
    for (i = 0; i < ARRAY_SIZE(nodes) / 3; i++) {
        nodes[i].key = i * 13;
        rb_tree_insert(&tree, &nodes[i].node, rb_test_node_cmp);
    }

    const int x = i * 13;
    for (/* empty */; i < 2 * ARRAY_SIZE(nodes) / 3; i++) {
        nodes[i].key = x;
        rb_tree_insert(&tree, &nodes[i].node, rb_test_node_cmp);
    }

    for (/* empty */; i < ARRAY_SIZE(nodes); i++) {
        nodes[i].key = i * 13;
        rb_tree_insert(&tree, &nodes[i].node, rb_test_node_cmp);
    }

    struct rb_node *n = rb_tree_search(&tree, &x, rb_test_node_cmp_void);

    ASSERT_NE(nullptr, n);

    struct rb_node *prev = rb_node_prev(n);

    ASSERT_NE(nullptr, prev);

    EXPECT_NE(rb_test_node_cmp(prev, n), 0);
}

struct uinterval_test_node {
    struct uinterval_node node;
};

static void
validate_interval_search(struct rb_tree *tree,
                         struct uinterval_test_node *nodes,
                         int first_node, int last_node,
                         unsigned start,
                         unsigned end)
{
    /* Count the number of intervals intersecting */
    unsigned actual_count = 0;
    for (int i = first_node; i <= last_node; i++) {
        if (nodes[i].node.interval.start <= end &&
            nodes[i].node.interval.end >= start)
            actual_count++;
    }

    /* iterate over matching intervals */
    struct uinterval interval = { start, end };
    unsigned max_val = 0;
    struct uinterval_test_node *prev = NULL;
    unsigned count = 0;
    uinterval_tree_foreach (struct uinterval_test_node, n, interval, tree, node) {
        assert(n->node.interval.start <= end &&
               n->node.interval.end >= start);

        /* Everything should be in increasing order */
        assert(n->node.interval.start >= max_val);
        if (n->node.interval.start > max_val) {
            max_val = n->node.interval.start;
        } else {
            /* Things should be stable, i.e., given equal keys, they should
             * show up in the list in order of insertion.  We insert them
             * in the order they are in in the array.
             */
            assert(prev == NULL || prev < n);
        }

        prev = n;
        count++;
    }

    assert(count == actual_count);
}

TEST(IntervalTreeTest, InsertAndSearch)
{
    struct uinterval_test_node nodes[ARRAY_SIZE(test_numbers) / 2];
    struct rb_tree tree;

    rb_tree_init(&tree);

    for (unsigned i = 0; 2 * i < ARRAY_SIZE(test_numbers); i++) {
        nodes[i].node.interval.start = MIN2(test_numbers[2 * i], test_numbers[2 * i + 1]);
        nodes[i].node.interval.end = MAX2(test_numbers[2 * i], test_numbers[2 * i + 1]);
        uinterval_tree_insert(&tree, &nodes[i].node);
        rb_tree_validate(&tree);
        validate_interval_search(&tree, nodes, 0, i, 0, 100);
        validate_interval_search(&tree, nodes, 0, i, 0, 50);
        validate_interval_search(&tree, nodes, 0, i, 50, 100);
        validate_interval_search(&tree, nodes, 0, i, 0, 2);
    }

    for (unsigned i = 0; 2 * i < ARRAY_SIZE(test_numbers); i++) {
        uinterval_tree_remove(&tree, &nodes[i].node);
        rb_tree_validate(&tree);
        validate_interval_search(&tree, nodes, i + 1,
                                 ARRAY_SIZE(test_numbers) / 2 - 1, 
                                 0, 100);
        validate_interval_search(&tree, nodes, i + 1,
                                 ARRAY_SIZE(test_numbers) / 2 - 1, 
                                 0, 50);
        validate_interval_search(&tree, nodes, i + 1,
                                 ARRAY_SIZE(test_numbers) / 2 - 1, 
                                 50, 100);
        validate_interval_search(&tree, nodes, i + 1,
                                 ARRAY_SIZE(test_numbers) / 2 - 1, 
                                 0, 2);
    }
}
