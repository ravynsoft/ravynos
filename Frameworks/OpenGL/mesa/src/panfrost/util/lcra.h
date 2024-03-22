/*
 * Copyright (C) 2019 Collabora, Ltd.
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors (Collabora):
 *      Alyssa Rosenzweig <alyssa.rosenzweig@collabora.com>
 */

#ifndef __LCRA_H
#define __LCRA_H

#include <stdbool.h>
#include <stdint.h>

struct lcra_state {
   unsigned node_count;

   /* Alignment for node in log2(bytes)+1. Since alignment must be
    * non-negative power-of-two, the elements are strictly positive
    * integers. Zero is the sentinel for a missing node. In upper word,
    * bound. */
   unsigned *alignment;

   /* Linear constraints imposed. Nested array sized upfront, organized as
    * linear[node_left][node_right]. That is, calculate indices as:
    *
    * Each element is itself a bit field denoting whether (c_j - c_i) bias
    * is present or not, including negative biases.
    *
    * Note for Midgard, there are 16 components so the bias is in range
    * [-15, 15] so encoded by 32-bit field. */

   uint32_t *linear;

   /* Per node max modulus constraints */
   uint8_t *modulus;

   /* Classes allow nodes to be partitioned with a starting register.
    * Classes cannot interfere; that is, they are true partitions in the
    * usual sense of the word. class_count is the number of classes.
    * class[] is indexed by a node to get the mapped class. class_start is
    * biased to all solutions in the class. */

   unsigned class_count;
   unsigned *class;
   unsigned *class_start;
   unsigned *class_size;
   bool *class_disjoint;

   /* Before solving, forced registers; after solving, solutions. */
   unsigned *solutions;

   /* For register spilling, the costs to spill nodes (as set by the user)
    * are in spill_cost[], negative if a node is unspillable. Internally,
    * spill_class specifies which class to spill (whichever class failed
    * to allocate) */

   signed *spill_cost;
   unsigned spill_class;
};

struct lcra_state *lcra_alloc_equations(unsigned node_count,
                                        unsigned class_count);

void lcra_free(struct lcra_state *l);

void lcra_set_disjoint_class(struct lcra_state *l, unsigned c1, unsigned c2);

void lcra_set_alignment(struct lcra_state *l, unsigned node,
                        unsigned align_log2, unsigned bound);

void lcra_restrict_range(struct lcra_state *l, unsigned node, unsigned len);

void lcra_add_node_interference(struct lcra_state *l, unsigned i,
                                unsigned cmask_i, unsigned j, unsigned cmask_j);

bool lcra_solve(struct lcra_state *l);

void lcra_set_node_spill_cost(struct lcra_state *l, unsigned node, signed cost);

signed lcra_get_best_spill_node(struct lcra_state *l);

#endif
