/*
 * Copyright © 2010 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Authors:
 *    Eric Anholt <eric@anholt.net>
 *
 */

/** @file register_allocate.c
 *
 * Graph-coloring register allocator.
 *
 * The basic idea of graph coloring is to make a node in a graph for
 * every thing that needs a register (color) number assigned, and make
 * edges in the graph between nodes that interfere (can't be allocated
 * to the same register at the same time).
 *
 * During the "simplify" process, any any node with fewer edges than
 * there are registers means that that edge can get assigned a
 * register regardless of what its neighbors choose, so that node is
 * pushed on a stack and removed (with its edges) from the graph.
 * That likely causes other nodes to become trivially colorable as well.
 *
 * Then during the "select" process, nodes are popped off of that
 * stack, their edges restored, and assigned a color different from
 * their neighbors.  Because they were pushed on the stack only when
 * they were trivially colorable, any color chosen won't interfere
 * with the registers to be popped later.
 *
 * The downside to most graph coloring is that real hardware often has
 * limitations, like registers that need to be allocated to a node in
 * pairs, or aligned on some boundary.  This implementation follows
 * the paper "Retargetable Graph-Coloring Register Allocation for
 * Irregular Architectures" by Johan Runeson and Sven-Olof Nyström.
 *
 * In this system, there are register classes each containing various
 * registers, and registers may interfere with other registers.  For
 * example, one might have a class of base registers, and a class of
 * aligned register pairs that would each interfere with their pair of
 * the base registers.  Each node has a register class it needs to be
 * assigned to.  Define p(B) to be the size of register class B, and
 * q(B,C) to be the number of registers in B that the worst choice
 * register in C could conflict with.  Then, this system replaces the
 * basic graph coloring test of "fewer edges from this node than there
 * are registers" with "For this node of class B, the sum of q(B,C)
 * for each neighbor node of class C is less than pB".
 *
 * A nice feature of the pq test is that q(B,C) can be computed once
 * up front and stored in a 2-dimensional array, so that the cost of
 * coloring a node is constant with the number of registers.  We do
 * this during ra_set_finalize().
 */

#include <stdbool.h>
#include <stdlib.h>

#include "blob.h"
#include "ralloc.h"
#include "util/bitset.h"
#include "util/u_dynarray.h"
#include "u_math.h"
#include "register_allocate.h"
#include "register_allocate_internal.h"

/**
 * Creates a set of registers for the allocator.
 *
 * mem_ctx is a ralloc context for the allocator.  The reg set may be freed
 * using ralloc_free().
 */
struct ra_regs *
ra_alloc_reg_set(void *mem_ctx, unsigned int count, bool need_conflict_lists)
{
   unsigned int i;
   struct ra_regs *regs;

   regs = rzalloc(mem_ctx, struct ra_regs);
   regs->count = count;
   regs->regs = rzalloc_array(regs, struct ra_reg, count);

   for (i = 0; i < count; i++) {
      regs->regs[i].conflicts = rzalloc_array(regs->regs, BITSET_WORD,
                                              BITSET_WORDS(count));
      BITSET_SET(regs->regs[i].conflicts, i);

      util_dynarray_init(&regs->regs[i].conflict_list,
                         need_conflict_lists ? regs->regs : NULL);
      if (need_conflict_lists)
         util_dynarray_append(&regs->regs[i].conflict_list, unsigned int, i);
   }

   return regs;
}

/**
 * The register allocator by default prefers to allocate low register numbers,
 * since it was written for hardware (gen4/5 Intel) that is limited in its
 * multithreadedness by the number of registers used in a given shader.
 *
 * However, for hardware without that restriction, densely packed register
 * allocation can put serious constraints on instruction scheduling.  This
 * function tells the allocator to rotate around the registers if possible as
 * it allocates the nodes.
 */
void
ra_set_allocate_round_robin(struct ra_regs *regs)
{
   regs->round_robin = true;
}

static void
ra_add_conflict_list(struct ra_regs *regs, unsigned int r1, unsigned int r2)
{
   struct ra_reg *reg1 = &regs->regs[r1];

   if (reg1->conflict_list.mem_ctx) {
      util_dynarray_append(&reg1->conflict_list, unsigned int, r2);
   }
   BITSET_SET(reg1->conflicts, r2);
}

void
ra_add_reg_conflict(struct ra_regs *regs, unsigned int r1, unsigned int r2)
{
   if (!BITSET_TEST(regs->regs[r1].conflicts, r2)) {
      ra_add_conflict_list(regs, r1, r2);
      ra_add_conflict_list(regs, r2, r1);
   }
}

/**
 * Adds a conflict between base_reg and reg, and also between reg and
 * anything that base_reg conflicts with.
 *
 * This can simplify code for setting up multiple register classes
 * which are aggregates of some base hardware registers, compared to
 * explicitly using ra_add_reg_conflict.
 */
void
ra_add_transitive_reg_conflict(struct ra_regs *regs,
                               unsigned int base_reg, unsigned int reg)
{
   ra_add_reg_conflict(regs, reg, base_reg);

   util_dynarray_foreach(&regs->regs[base_reg].conflict_list, unsigned int,
                         r2p) {
      ra_add_reg_conflict(regs, reg, *r2p);
   }
}

/**
 * Set up conflicts between base_reg and it's two half registers reg0 and
 * reg1, but take care to not add conflicts between reg0 and reg1.
 *
 * This is useful for architectures where full size registers are aliased by
 * two half size registers (eg 32 bit float and 16 bit float registers).
 */
void
ra_add_transitive_reg_pair_conflict(struct ra_regs *regs,
                                    unsigned int base_reg, unsigned int reg0, unsigned int reg1)
{
   ra_add_reg_conflict(regs, reg0, base_reg);
   ra_add_reg_conflict(regs, reg1, base_reg);

   util_dynarray_foreach(&regs->regs[base_reg].conflict_list, unsigned int, i) {
      unsigned int conflict = *i;
      if (conflict != reg1)
         ra_add_reg_conflict(regs, reg0, conflict);
      if (conflict != reg0)
         ra_add_reg_conflict(regs, reg1, conflict);
   }
}

/**
 * Makes every conflict on the given register transitive.  In other words,
 * every register that conflicts with r will now conflict with every other
 * register conflicting with r.
 *
 * This can simplify code for setting up multiple register classes
 * which are aggregates of some base hardware registers, compared to
 * explicitly using ra_add_reg_conflict.
 */
void
ra_make_reg_conflicts_transitive(struct ra_regs *regs, unsigned int r)
{
   struct ra_reg *reg = &regs->regs[r];
   int c;

   BITSET_FOREACH_SET(c, reg->conflicts, regs->count) {
      struct ra_reg *other = &regs->regs[c];
      unsigned i;
      for (i = 0; i < BITSET_WORDS(regs->count); i++)
         other->conflicts[i] |= reg->conflicts[i];
   }
}

struct ra_class *
ra_alloc_reg_class(struct ra_regs *regs)
{
   struct ra_class *class;

   regs->classes = reralloc(regs->regs, regs->classes, struct ra_class *,
                            regs->class_count + 1);

   class = rzalloc(regs, struct ra_class);
   class->regset = regs;

   /* Users may rely on the class index being allocated in order starting from 0. */
   class->index = regs->class_count++;
   regs->classes[class->index] = class;

   class->regs = rzalloc_array(class, BITSET_WORD, BITSET_WORDS(regs->count));

   return class;
}

/**
 * Creates a register class for contiguous register groups of a base register
 * set.
 *
 * A reg set using this type of register class must use only this type of
 * register class.
 */
struct ra_class *
ra_alloc_contig_reg_class(struct ra_regs *regs, int contig_len)
{
   struct ra_class *c = ra_alloc_reg_class(regs);

   assert(contig_len != 0);
   c->contig_len = contig_len;

   return c;
}

struct ra_class *
ra_get_class_from_index(struct ra_regs *regs, unsigned int class)
{
   return regs->classes[class];
}

unsigned int
ra_class_index(struct ra_class *c)
{
   return c->index;
}

void
ra_class_add_reg(struct ra_class *class, unsigned int r)
{
   assert(r < class->regset->count);
   assert(r + class->contig_len <= class->regset->count);

   BITSET_SET(class->regs, r);
   class->p++;
}

/**
 * Returns true if the register belongs to the given class.
 */
static bool
reg_belongs_to_class(unsigned int r, struct ra_class *c)
{
   return BITSET_TEST(c->regs, r);
}

/**
 * Must be called after all conflicts and register classes have been
 * set up and before the register set is used for allocation.
 * To avoid costly q value computation, use the q_values paramater
 * to pass precomputed q values to this function.
 */
void
ra_set_finalize(struct ra_regs *regs, unsigned int **q_values)
{
   unsigned int b, c;

   for (b = 0; b < regs->class_count; b++) {
      regs->classes[b]->q = ralloc_array(regs, unsigned int, regs->class_count);
   }

   if (q_values) {
      for (b = 0; b < regs->class_count; b++) {
         for (c = 0; c < regs->class_count; c++) {
            regs->classes[b]->q[c] = q_values[b][c];
         }
      }
   } else {
      /* Compute, for each class B and C, how many regs of B an
       * allocation to C could conflict with.
       */
      for (b = 0; b < regs->class_count; b++) {
         for (c = 0; c < regs->class_count; c++) {
            struct ra_class *class_b = regs->classes[b];
            struct ra_class *class_c = regs->classes[c];

            if (class_b->contig_len && class_c->contig_len) {
               if (class_b->contig_len == 1 && class_c->contig_len == 1) {
                  /* If both classes are single registers, then they only
                   * conflict if there are any regs shared between them.  This
                   * is a cheap test for a common case.
                   */
                  class_b->q[c] = 0;
                  for (int i = 0; i < BITSET_WORDS(regs->count); i++) {
                     if (class_b->regs[i] & class_c->regs[i]) {
                        class_b->q[c] = 1;
                        break;
                     }
                  }
               } else {
                  int max_possible_conflicts = class_b->contig_len + class_c->contig_len - 1;

                  unsigned int max_conflicts = 0;
                  unsigned int rc;
                  BITSET_FOREACH_SET(rc, regs->classes[c]->regs, regs->count) {
                     int start = MAX2(0, (int)rc - class_b->contig_len + 1);
                     int end = MIN2(regs->count, rc + class_c->contig_len);
                     unsigned int conflicts = 0;
                     for (int i = start; i < end; i++) {
                        if (BITSET_TEST(class_b->regs, i))
                           conflicts++;
                     }
                     max_conflicts = MAX2(max_conflicts, conflicts);
                     /* Unless a class has some restriction like the register
                      * bases are all aligned, then we should quickly find this
                      * limit and exit the loop.
                      */
                     if (max_conflicts == max_possible_conflicts)
                        break;
                  }
                  class_b->q[c] = max_conflicts;
               }
            } else {
               /* If you're doing contiguous classes, you have to be all in
                * because I don't want to deal with it.
                */
               assert(!class_b->contig_len && !class_c->contig_len);

               unsigned int rc;
               int max_conflicts = 0;

               BITSET_FOREACH_SET(rc, regs->classes[c]->regs, regs->count) {
                  int conflicts = 0;

                  util_dynarray_foreach(&regs->regs[rc].conflict_list,
                                       unsigned int, rbp) {
                     unsigned int rb = *rbp;
                     if (reg_belongs_to_class(rb, regs->classes[b]))
                        conflicts++;
                  }
                  max_conflicts = MAX2(max_conflicts, conflicts);
               }
               regs->classes[b]->q[c] = max_conflicts;
            }
         }
      }
   }

   for (b = 0; b < regs->count; b++) {
      util_dynarray_fini(&regs->regs[b].conflict_list);
   }

   bool all_contig = true;
   for (int c = 0; c < regs->class_count; c++)
      all_contig &= regs->classes[c]->contig_len != 0;
   if (all_contig) {
      /* In this case, we never need the conflicts lists (and it would probably
       * be a mistake to look at conflicts when doing contiguous classes!), so
       * free them.  TODO: Avoid the allocation in the first place.
       */
      for (int i = 0; i < regs->count; i++) {
         ralloc_free(regs->regs[i].conflicts);
         regs->regs[i].conflicts = NULL;
      }
   }
}

void
ra_set_serialize(const struct ra_regs *regs, struct blob *blob)
{
   blob_write_uint32(blob, regs->count);
   blob_write_uint32(blob, regs->class_count);

   bool is_contig = regs->classes[0]->contig_len != 0;
   blob_write_uint8(blob, is_contig);

   if (!is_contig) {
      for (unsigned int r = 0; r < regs->count; r++) {
         struct ra_reg *reg = &regs->regs[r];
         blob_write_bytes(blob, reg->conflicts, BITSET_WORDS(regs->count) *
                                                sizeof(BITSET_WORD));
         assert(util_dynarray_num_elements(&reg->conflict_list, unsigned int) == 0);
      }
   }

   for (unsigned int c = 0; c < regs->class_count; c++) {
      struct ra_class *class = regs->classes[c];
      blob_write_bytes(blob, class->regs, BITSET_WORDS(regs->count) *
                                          sizeof(BITSET_WORD));
      blob_write_uint32(blob, class->contig_len);
      blob_write_uint32(blob, class->p);
      blob_write_bytes(blob, class->q, regs->class_count * sizeof(*class->q));
   }

   blob_write_uint32(blob, regs->round_robin);
}

struct ra_regs *
ra_set_deserialize(void *mem_ctx, struct blob_reader *blob)
{
   unsigned int reg_count = blob_read_uint32(blob);
   unsigned int class_count = blob_read_uint32(blob);
   bool is_contig = blob_read_uint8(blob);

   struct ra_regs *regs = ra_alloc_reg_set(mem_ctx, reg_count, false);
   assert(regs->count == reg_count);

   if (is_contig) {
      for (int i = 0; i < regs->count; i++) {
         ralloc_free(regs->regs[i].conflicts);
         regs->regs[i].conflicts = NULL;
      }
   } else {
      for (unsigned int r = 0; r < reg_count; r++) {
         struct ra_reg *reg = &regs->regs[r];
         blob_copy_bytes(blob, reg->conflicts, BITSET_WORDS(reg_count) *
                                             sizeof(BITSET_WORD));
      }
   }

   assert(regs->classes == NULL);
   regs->classes = ralloc_array(regs->regs, struct ra_class *, class_count);
   regs->class_count = class_count;

   for (unsigned int c = 0; c < class_count; c++) {
      struct ra_class *class = rzalloc(regs, struct ra_class);
      regs->classes[c] = class;
      class->regset = regs;
      class->index = c;

      class->regs = ralloc_array(class, BITSET_WORD, BITSET_WORDS(reg_count));
      blob_copy_bytes(blob, class->regs, BITSET_WORDS(reg_count) *
                                         sizeof(BITSET_WORD));

      class->contig_len = blob_read_uint32(blob);
      class->p = blob_read_uint32(blob);

      class->q = ralloc_array(regs->classes[c], unsigned int, class_count);
      blob_copy_bytes(blob, class->q, class_count * sizeof(*class->q));
   }

   regs->round_robin = blob_read_uint32(blob);

   return regs;
}

static uint64_t
ra_get_num_adjacency_bits(uint64_t n)
{
   return (n * (n - 1)) / 2;
}

static uint64_t
ra_get_adjacency_bit_index(unsigned n1, unsigned n2)
{
   assert(n1 != n2);
   unsigned k1 = MAX2(n1, n2);
   unsigned k2 = MIN2(n1, n2);
   return ra_get_num_adjacency_bits(k1) + k2;
}

static bool
ra_test_adjacency_bit(struct ra_graph *g, unsigned n1, unsigned n2)
{
   uint64_t index = ra_get_adjacency_bit_index(n1, n2);
   return BITSET_TEST(g->adjacency, index);
}

static void
ra_set_adjacency_bit(struct ra_graph *g, unsigned n1, unsigned n2)
{
   unsigned index = ra_get_adjacency_bit_index(n1, n2);
   BITSET_SET(g->adjacency, index);
}

static void
ra_clear_adjacency_bit(struct ra_graph *g, unsigned n1, unsigned n2)
{
   unsigned index = ra_get_adjacency_bit_index(n1, n2);
   BITSET_CLEAR(g->adjacency, index);
}

static void
ra_add_node_adjacency(struct ra_graph *g, unsigned int n1, unsigned int n2)
{
   assert(n1 != n2);

   int n1_class = g->nodes[n1].class;
   int n2_class = g->nodes[n2].class;
   g->nodes[n1].q_total += g->regs->classes[n1_class]->q[n2_class];

   util_dynarray_append(&g->nodes[n1].adjacency_list, unsigned int, n2);
}

static void
ra_node_remove_adjacency(struct ra_graph *g, unsigned int n1, unsigned int n2)
{
   assert(n1 != n2);
   ra_clear_adjacency_bit(g, n1, n2);

   int n1_class = g->nodes[n1].class;
   int n2_class = g->nodes[n2].class;
   g->nodes[n1].q_total -= g->regs->classes[n1_class]->q[n2_class];

   util_dynarray_delete_unordered(&g->nodes[n1].adjacency_list, unsigned int,
                                  n2);
}

static void
ra_realloc_interference_graph(struct ra_graph *g, unsigned int alloc)
{
   if (alloc <= g->alloc)
      return;

   /* If we always have a whole number of BITSET_WORDs, it makes it much
    * easier to memset the top of the growing bitsets.
    */
   assert(g->alloc % BITSET_WORDBITS == 0);
   alloc = align(alloc, BITSET_WORDBITS);
   g->nodes = rerzalloc(g, g->nodes, struct ra_node, g->alloc, alloc);
   g->adjacency = rerzalloc(g, g->adjacency, BITSET_WORD,
                            BITSET_WORDS(ra_get_num_adjacency_bits(g->alloc)),
                            BITSET_WORDS(ra_get_num_adjacency_bits(alloc)));

   /* Initialize new nodes. */
   for (unsigned i = g->alloc; i < alloc; i++) {
      struct ra_node* node = g->nodes + i;
      util_dynarray_init(&node->adjacency_list, g);
      node->q_total = 0;
      node->forced_reg = NO_REG;
      node->reg = NO_REG;
   }

   /* These are scratch values and don't need to be zeroed.  We'll clear them
    * as part of ra_select() setup.
    */
   unsigned bitset_count = BITSET_WORDS(alloc);
   g->tmp.stack = reralloc(g, g->tmp.stack, unsigned int, alloc);
   g->tmp.in_stack = reralloc(g, g->tmp.in_stack, BITSET_WORD, bitset_count);

   g->tmp.reg_assigned = reralloc(g, g->tmp.reg_assigned, BITSET_WORD,
                                  bitset_count);
   g->tmp.pq_test = reralloc(g, g->tmp.pq_test, BITSET_WORD, bitset_count);
   g->tmp.min_q_total = reralloc(g, g->tmp.min_q_total, unsigned int,
                                 bitset_count);
   g->tmp.min_q_node = reralloc(g, g->tmp.min_q_node, unsigned int,
                                bitset_count);

   g->alloc = alloc;
}

struct ra_graph *
ra_alloc_interference_graph(struct ra_regs *regs, unsigned int count)
{
   struct ra_graph *g;

   g = rzalloc(NULL, struct ra_graph);
   g->regs = regs;
   g->count = count;
   ra_realloc_interference_graph(g, count);

   return g;
}

void
ra_resize_interference_graph(struct ra_graph *g, unsigned int count)
{
   g->count = count;
   if (count > g->alloc)
      ra_realloc_interference_graph(g, g->alloc * 2);
}

void ra_set_select_reg_callback(struct ra_graph *g,
                                ra_select_reg_callback callback,
                                void *data)
{
   g->select_reg_callback = callback;
   g->select_reg_callback_data = data;
}

void
ra_set_node_class(struct ra_graph *g,
                  unsigned int n, struct ra_class *class)
{
   g->nodes[n].class = class->index;
}

struct ra_class *
ra_get_node_class(struct ra_graph *g,
                  unsigned int n)
{
   return g->regs->classes[g->nodes[n].class];
}

unsigned int
ra_add_node(struct ra_graph *g, struct ra_class *class)
{
   unsigned int n = g->count;
   ra_resize_interference_graph(g, g->count + 1);

   ra_set_node_class(g, n, class);

   return n;
}

void
ra_add_node_interference(struct ra_graph *g,
                         unsigned int n1, unsigned int n2)
{
   assert(n1 < g->count && n2 < g->count);
   if (n1 != n2 && !ra_test_adjacency_bit(g, n1, n2)) {
      ra_set_adjacency_bit(g, n1, n2);
      ra_add_node_adjacency(g, n1, n2);
      ra_add_node_adjacency(g, n2, n1);
   }
}

void
ra_reset_node_interference(struct ra_graph *g, unsigned int n)
{
   util_dynarray_foreach(&g->nodes[n].adjacency_list, unsigned int, n2p) {
      ra_node_remove_adjacency(g, *n2p, n);
   }

   util_dynarray_clear(&g->nodes[n].adjacency_list);
}

static void
update_pq_info(struct ra_graph *g, unsigned int n)
{
   int i = n / BITSET_WORDBITS;
   int n_class = g->nodes[n].class;
   if (g->nodes[n].tmp.q_total < g->regs->classes[n_class]->p) {
      BITSET_SET(g->tmp.pq_test, n);
   } else if (g->tmp.min_q_total[i] != UINT_MAX) {
      /* Only update min_q_total and min_q_node if min_q_total != UINT_MAX so
       * that we don't update while we have stale data and accidentally mark
       * it as non-stale.  Also, in order to remain consistent with the old
       * naive implementation of the algorithm, we do a lexicographical sort
       * to ensure that we always choose the node with the highest node index.
       */
      if (g->nodes[n].tmp.q_total < g->tmp.min_q_total[i] ||
          (g->nodes[n].tmp.q_total == g->tmp.min_q_total[i] &&
           n > g->tmp.min_q_node[i])) {
         g->tmp.min_q_total[i] = g->nodes[n].tmp.q_total;
         g->tmp.min_q_node[i] = n;
      }
   }
}

static void
add_node_to_stack(struct ra_graph *g, unsigned int n)
{
   int n_class = g->nodes[n].class;

   assert(!BITSET_TEST(g->tmp.in_stack, n));

   util_dynarray_foreach(&g->nodes[n].adjacency_list, unsigned int, n2p) {
      unsigned int n2 = *n2p;
      unsigned int n2_class = g->nodes[n2].class;

      if (!BITSET_TEST(g->tmp.in_stack, n2) &&
          !BITSET_TEST(g->tmp.reg_assigned, n2)) {
         assert(g->nodes[n2].tmp.q_total >= g->regs->classes[n2_class]->q[n_class]);
         g->nodes[n2].tmp.q_total -= g->regs->classes[n2_class]->q[n_class];
         update_pq_info(g, n2);
      }
   }

   g->tmp.stack[g->tmp.stack_count] = n;
   g->tmp.stack_count++;
   BITSET_SET(g->tmp.in_stack, n);

   /* Flag the min_q_total for n's block as dirty so it gets recalculated */
   g->tmp.min_q_total[n / BITSET_WORDBITS] = UINT_MAX;
}

/**
 * Simplifies the interference graph by pushing all
 * trivially-colorable nodes into a stack of nodes to be colored,
 * removing them from the graph, and rinsing and repeating.
 *
 * If we encounter a case where we can't push any nodes on the stack, then
 * we optimistically choose a node and push it on the stack. We heuristically
 * push the node with the lowest total q value, since it has the fewest
 * neighbors and therefore is most likely to be allocated.
 */
static void
ra_simplify(struct ra_graph *g)
{
   bool progress = true;
   unsigned int stack_optimistic_start = UINT_MAX;

   /* Figure out the high bit and bit mask for the first iteration of a loop
    * over BITSET_WORDs.
    */
   const unsigned int top_word_high_bit = (g->count - 1) % BITSET_WORDBITS;

   /* Do a quick pre-pass to set things up */
   g->tmp.stack_count = 0;
   for (int i = BITSET_WORDS(g->count) - 1, high_bit = top_word_high_bit;
        i >= 0; i--, high_bit = BITSET_WORDBITS - 1) {
      g->tmp.in_stack[i] = 0;
      g->tmp.reg_assigned[i] = 0;
      g->tmp.pq_test[i] = 0;
      g->tmp.min_q_total[i] = UINT_MAX;
      g->tmp.min_q_node[i] = UINT_MAX;
      for (int j = high_bit; j >= 0; j--) {
         unsigned int n = i * BITSET_WORDBITS + j;
         g->nodes[n].reg = g->nodes[n].forced_reg;
         g->nodes[n].tmp.q_total = g->nodes[n].q_total;
         if (g->nodes[n].reg != NO_REG)
            g->tmp.reg_assigned[i] |= BITSET_BIT(j);
         update_pq_info(g, n);
      }
   }

   while (progress) {
      unsigned int min_q_total = UINT_MAX;
      unsigned int min_q_node = UINT_MAX;

      progress = false;

      for (int i = BITSET_WORDS(g->count) - 1, high_bit = top_word_high_bit;
           i >= 0; i--, high_bit = BITSET_WORDBITS - 1) {
         BITSET_WORD mask = ~(BITSET_WORD)0 >> (31 - high_bit);

         BITSET_WORD skip = g->tmp.in_stack[i] | g->tmp.reg_assigned[i];
         if (skip == mask)
            continue;

         BITSET_WORD pq = g->tmp.pq_test[i] & ~skip;
         if (pq) {
            /* In this case, we have stuff we can immediately take off the
             * stack.  This also means that we're guaranteed to make progress
             * and we don't need to bother updating lowest_q_total because we
             * know we're going to loop again before attempting to do anything
             * optimistic.
             */
            for (int j = high_bit; j >= 0; j--) {
               if (pq & BITSET_BIT(j)) {
                  unsigned int n = i * BITSET_WORDBITS + j;
                  assert(n < g->count);
                  add_node_to_stack(g, n);
                  /* add_node_to_stack() may update pq_test for this word so
                   * we need to update our local copy.
                   */
                  pq = g->tmp.pq_test[i] & ~skip;
                  progress = true;
               }
            }
         } else if (!progress) {
            if (g->tmp.min_q_total[i] == UINT_MAX) {
               /* The min_q_total and min_q_node are dirty because we added
                * one of these nodes to the stack.  It needs to be
                * recalculated.
                */
               for (int j = high_bit; j >= 0; j--) {
                  if (skip & BITSET_BIT(j))
                     continue;

                  unsigned int n = i * BITSET_WORDBITS + j;
                  assert(n < g->count);
                  if (g->nodes[n].tmp.q_total < g->tmp.min_q_total[i]) {
                     g->tmp.min_q_total[i] = g->nodes[n].tmp.q_total;
                     g->tmp.min_q_node[i] = n;
                  }
               }
            }
            if (g->tmp.min_q_total[i] < min_q_total) {
               min_q_node = g->tmp.min_q_node[i];
               min_q_total = g->tmp.min_q_total[i];
            }
         }
      }

      if (!progress && min_q_total != UINT_MAX) {
         if (stack_optimistic_start == UINT_MAX)
            stack_optimistic_start = g->tmp.stack_count;

         add_node_to_stack(g, min_q_node);
         progress = true;
      }
   }

   g->tmp.stack_optimistic_start = stack_optimistic_start;
}

bool
ra_class_allocations_conflict(struct ra_class *c1, unsigned int r1,
                              struct ra_class *c2, unsigned int r2)
{
   if (c1->contig_len) {
      assert(c2->contig_len);

      int r1_end = r1 + c1->contig_len;
      int r2_end = r2 + c2->contig_len;
      return !(r2 >= r1_end || r1 >= r2_end);
   } else {
      return BITSET_TEST(c1->regset->regs[r1].conflicts, r2);
   }
}

static struct ra_node *
ra_find_conflicting_neighbor(struct ra_graph *g, unsigned int n, unsigned int r)
{
   util_dynarray_foreach(&g->nodes[n].adjacency_list, unsigned int, n2p) {
      unsigned int n2 = *n2p;

      /* If our adjacent node is in the stack, it's not allocated yet. */
      if (!BITSET_TEST(g->tmp.in_stack, n2) &&
          ra_class_allocations_conflict(g->regs->classes[g->nodes[n].class], r,
                                        g->regs->classes[g->nodes[n2].class], g->nodes[n2].reg)) {
         return &g->nodes[n2];
      }
   }

   return NULL;
}

/* Computes a bitfield of what regs are available for a given register
 * selection.
 *
 * This lets drivers implement a more complicated policy than our simple first
 * or round robin policies (which don't require knowing the whole bitset)
 */
static bool
ra_compute_available_regs(struct ra_graph *g, unsigned int n, BITSET_WORD *regs)
{
   struct ra_class *c = g->regs->classes[g->nodes[n].class];

   /* Populate with the set of regs that are in the node's class. */
   memcpy(regs, c->regs, BITSET_WORDS(g->regs->count) * sizeof(BITSET_WORD));

   /* Remove any regs that conflict with nodes that we're adjacent to and have
    * already colored.
    */
   util_dynarray_foreach(&g->nodes[n].adjacency_list, unsigned int, n2p) {
      struct ra_node *n2 = &g->nodes[*n2p];
      struct ra_class *n2c = g->regs->classes[n2->class];

      if (!BITSET_TEST(g->tmp.in_stack, *n2p)) {
         if (c->contig_len) {
            int start = MAX2(0, (int)n2->reg - c->contig_len + 1);
            int end = MIN2(g->regs->count, n2->reg + n2c->contig_len);
            for (unsigned i = start; i < end; i++)
               BITSET_CLEAR(regs, i);
         } else {
            for (int j = 0; j < BITSET_WORDS(g->regs->count); j++)
               regs[j] &= ~g->regs->regs[n2->reg].conflicts[j];
         }
      }
   }

   for (int i = 0; i < BITSET_WORDS(g->regs->count); i++) {
      if (regs[i])
         return true;
   }

   return false;
}

/**
 * Pops nodes from the stack back into the graph, coloring them with
 * registers as they go.
 *
 * If all nodes were trivially colorable, then this must succeed.  If
 * not (optimistic coloring), then it may return false;
 */
static bool
ra_select(struct ra_graph *g)
{
   int start_search_reg = 0;
   BITSET_WORD *select_regs = NULL;

   if (g->select_reg_callback)
      select_regs = malloc(BITSET_WORDS(g->regs->count) * sizeof(BITSET_WORD));

   while (g->tmp.stack_count != 0) {
      unsigned int ri;
      unsigned int r = -1;
      int n = g->tmp.stack[g->tmp.stack_count - 1];
      struct ra_class *c = g->regs->classes[g->nodes[n].class];

      /* set this to false even if we return here so that
       * ra_get_best_spill_node() considers this node later.
       */
      BITSET_CLEAR(g->tmp.in_stack, n);

      if (g->select_reg_callback) {
         if (!ra_compute_available_regs(g, n, select_regs)) {
            free(select_regs);
            return false;
         }

         r = g->select_reg_callback(n, select_regs, g->select_reg_callback_data);
         assert(r < g->regs->count);
      } else {
         /* Find the lowest-numbered reg which is not used by a member
          * of the graph adjacent to us.
          */
         for (ri = 0; ri < g->regs->count; ri++) {
            r = (start_search_reg + ri) % g->regs->count;
            if (!reg_belongs_to_class(r, c))
               continue;

            struct ra_node *conflicting = ra_find_conflicting_neighbor(g, n, r);
            if (!conflicting) {
               /* Found a reg! */
               break;
            }
            if (g->regs->classes[conflicting->class]->contig_len) {
               /* Skip to point at the last base reg of the conflicting reg
                * allocation -- the loop will increment us to check the next reg
                * after the conflicting allocaiton.
                */
               unsigned conflicting_end = (conflicting->reg +
                                           g->regs->classes[conflicting->class]->contig_len - 1);
               assert(conflicting_end >= r);
               ri += conflicting_end - r;
            }
         }

         if (ri >= g->regs->count)
            return false;
      }

      g->nodes[n].reg = r;
      g->tmp.stack_count--;

      /* Rotate the starting point except for any nodes above the lowest
       * optimistically colorable node.  The likelihood that we will succeed
       * at allocating optimistically colorable nodes is highly dependent on
       * the way that the previous nodes popped off the stack are laid out.
       * The round-robin strategy increases the fragmentation of the register
       * file and decreases the number of nearby nodes assigned to the same
       * color, what increases the likelihood of spilling with respect to the
       * dense packing strategy.
       */
      if (g->regs->round_robin &&
          g->tmp.stack_count - 1 <= g->tmp.stack_optimistic_start)
         start_search_reg = r + 1;
   }

   free(select_regs);

   return true;
}

bool
ra_allocate(struct ra_graph *g)
{
   ra_simplify(g);
   return ra_select(g);
}

unsigned int
ra_get_node_reg(struct ra_graph *g, unsigned int n)
{
   if (g->nodes[n].forced_reg != NO_REG)
      return g->nodes[n].forced_reg;
   else
      return g->nodes[n].reg;
}

/**
 * Forces a node to a specific register.  This can be used to avoid
 * creating a register class containing one node when handling data
 * that must live in a fixed location and is known to not conflict
 * with other forced register assignment (as is common with shader
 * input data).  These nodes do not end up in the stack during
 * ra_simplify(), and thus at ra_select() time it is as if they were
 * the first popped off the stack and assigned their fixed locations.
 * Nodes that use this function do not need to be assigned a register
 * class.
 *
 * Must be called before ra_simplify().
 */
void
ra_set_node_reg(struct ra_graph *g, unsigned int n, unsigned int reg)
{
   g->nodes[n].forced_reg = reg;
}

static float
ra_get_spill_benefit(struct ra_graph *g, unsigned int n)
{
   float benefit = 0;
   int n_class = g->nodes[n].class;

   /* Define the benefit of eliminating an interference between n, n2
    * through spilling as q(C, B) / p(C).  This is similar to the
    * "count number of edges" approach of traditional graph coloring,
    * but takes classes into account.
    */
   util_dynarray_foreach(&g->nodes[n].adjacency_list, unsigned int, n2p) {
      unsigned int n2 = *n2p;
      unsigned int n2_class = g->nodes[n2].class;
      benefit += ((float)g->regs->classes[n_class]->q[n2_class] /
                  g->regs->classes[n_class]->p);
   }

   return benefit;
}

/**
 * Returns a node number to be spilled according to the cost/benefit using
 * the pq test, or -1 if there are no spillable nodes.
 */
int
ra_get_best_spill_node(struct ra_graph *g)
{
   unsigned int best_node = -1;
   float best_benefit = 0.0;
   unsigned int n;

   /* Consider any nodes that we colored successfully or the node we failed to
    * color for spilling. When we failed to color a node in ra_select(), we
    * only considered these nodes, so spilling any other ones would not result
    * in us making progress.
    */
   for (n = 0; n < g->count; n++) {
      float cost = g->nodes[n].spill_cost;
      float benefit;

      if (cost <= 0.0f)
         continue;

      if (BITSET_TEST(g->tmp.in_stack, n))
         continue;

      benefit = ra_get_spill_benefit(g, n);

      if (benefit / cost > best_benefit) {
         best_benefit = benefit / cost;
         best_node = n;
      }
   }

   return best_node;
}

/**
 * Only nodes with a spill cost set (cost != 0.0) will be considered
 * for register spilling.
 */
void
ra_set_node_spill_cost(struct ra_graph *g, unsigned int n, float cost)
{
   g->nodes[n].spill_cost = cost;
}
