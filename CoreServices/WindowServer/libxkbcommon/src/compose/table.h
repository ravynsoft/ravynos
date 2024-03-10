/*
 * Copyright © 2013,2021 Ran Benita <ran234@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef COMPOSE_COMPOSE_H
#define COMPOSE_COMPOSE_H

#include "xkbcommon/xkbcommon-compose.h"
#include "utils.h"
#include "context.h"

/*
 * The compose table data structure is a ternary search tree.
 *
 * Reference: https://www.drdobbs.com/database/ternary-search-trees/184410528
 * Visualization: https://www.cs.usfca.edu/~galles/visualization/TST.html
 *
 * Short example. Given these sequences:
 *
 *      <B> <C>        : "first"  dead_a
 *      <B> <D> <E>    : "second" dead_b
 *      <A> <F>        : "third"  dead_c
 *
 * the tree would look like:
 *
 *          -------- [<B>]---------
 *          |          |          #
 *          v          V
 *     -- [<A>] --   [<C>] --------
 *     #    |    #     |          |
 *          v          #     -- [<D>] --
 *     -- [<F>] --           #    |    #
 *     #    |    #                v
 *          #                -- [<E>] --
 *                           #    |    #
 *                                #
 *
 * where:
 * - [<X>] is a node for a sequence keysym <X>.
 * - right arrows are `hikid` pointers.
 * - left arrows are `lokid` pointers.
 * - down arrows are `eqkid` pointers.
 * - # is a nil pointer.
 *
 * The nodes are all kept in a contiguous array.  Pointers are represented
 * as integer offsets into this array.  A nil pointer is represented as 0
 * (which, helpfully, is the offset of an empty dummy node).
 *
 * Nodes without an eqkid are leaf nodes.  Since a sequence cannot be a
 * prefix of another, these are exactly the nodes which terminate the
 * sequences (in a bijective manner).
 *
 * A leaf contains the result data of its sequence.  The result keysym is
 * contained in the node struct itself; the result UTF-8 string is a byte
 * offset into an array of the form "\0first\0second\0third" (the initial
 * \0 is so offset 0 points to an empty string).
 */

/* 7 nodes for every potential Unicode character and then some should be
 * enough for all purposes. */
#define MAX_COMPOSE_NODES (1 << 23)

struct compose_node {
    xkb_keysym_t keysym;

    /* Offset into xkb_compose_table::nodes or 0. */
    uint32_t lokid;
    /* Offset into xkb_compose_table::nodes or 0. */
    uint32_t hikid;

    union {
        struct {
            uint32_t _pad:31;
            bool is_leaf:1;
        };
        struct {
            uint32_t _pad:31;
            bool is_leaf:1;
            /* Offset into xkb_compose_table::nodes or 0. */
            uint32_t eqkid;
        } internal;
        struct {
            /* Offset into xkb_compose_table::utf8. */
            uint32_t utf8:31;
            bool is_leaf:1;
            xkb_keysym_t keysym;
        } leaf;
    };
};

struct xkb_compose_table {
    int refcnt;
    enum xkb_compose_format format;
    enum xkb_compose_compile_flags flags;
    struct xkb_context *ctx;

    char *locale;

    darray_char utf8;
    darray(struct compose_node) nodes;
};

struct xkb_compose_table_entry {
    xkb_keysym_t *sequence;
    size_t sequence_length;
    xkb_keysym_t keysym;
    const char *utf8;
};

#endif
