/*
 * Copyright (C) 2021 Valve Corporation
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
 */

#ifndef _IR3_RA_H
#define _IR3_RA_H

#include "util/rb_tree.h"
#include "ir3.h"
#include "ir3_compiler.h"

#ifdef DEBUG
#define RA_DEBUG (ir3_shader_debug & IR3_DBG_RAMSGS)
#else
#define RA_DEBUG 0
#endif
#define d(fmt, ...)                                                            \
   do {                                                                        \
      if (RA_DEBUG) {                                                          \
         mesa_logi("RA: " fmt, ##__VA_ARGS__);                                 \
      }                                                                        \
   } while (0)

#define di(instr, fmt, ...)                                                    \
   do {                                                                        \
      if (RA_DEBUG) {                                                          \
         struct log_stream *stream = mesa_log_streami();                       \
         mesa_log_stream_printf(stream, "RA: " fmt ": ", ##__VA_ARGS__);       \
         ir3_print_instr_stream(stream, instr);                                \
         mesa_log_stream_destroy(stream);                                      \
      }                                                                        \
   } while (0)

typedef uint16_t physreg_t;

static inline unsigned
ra_physreg_to_num(physreg_t physreg, unsigned flags)
{
   if (!(flags & IR3_REG_HALF))
      physreg /= 2;
   if (flags & IR3_REG_SHARED)
      physreg += 48 * 4;
   return physreg;
}

static inline physreg_t
ra_num_to_physreg(unsigned num, unsigned flags)
{
   if (flags & IR3_REG_SHARED)
      num -= 48 * 4;
   if (!(flags & IR3_REG_HALF))
      num *= 2;
   return num;
}

static inline unsigned
ra_reg_get_num(const struct ir3_register *reg)
{
   return (reg->flags & IR3_REG_ARRAY) ? reg->array.base : reg->num;
}

static inline physreg_t
ra_reg_get_physreg(const struct ir3_register *reg)
{
   return ra_num_to_physreg(ra_reg_get_num(reg), reg->flags);
}

static inline bool
def_is_gpr(const struct ir3_register *reg)
{
   return reg_num(reg) != REG_A0 && reg_num(reg) != REG_P0;
}

/* Note: don't count undef as a source.
 */
static inline bool
ra_reg_is_src(const struct ir3_register *reg)
{
   return (reg->flags & IR3_REG_SSA) && reg->def && def_is_gpr(reg->def);
}

static inline bool
ra_reg_is_dst(const struct ir3_register *reg)
{
   return (reg->flags & IR3_REG_SSA) && def_is_gpr(reg) &&
          ((reg->flags & IR3_REG_ARRAY) || reg->wrmask);
}

/* Iterators for sources and destinations which:
 * - Don't include fake sources (irrelevant for RA)
 * - Don't include non-SSA sources (immediates and constants, also irrelevant)
 */

#define ra_foreach_src_n(__srcreg, __n, __instr)                               \
   foreach_src_n(__srcreg, __n, __instr)                                       \
      if (ra_reg_is_src(__srcreg))

#define ra_foreach_src(__srcreg, __instr)                                      \
   ra_foreach_src_n(__srcreg, __i, __instr)

#define ra_foreach_src_rev(__srcreg, __instr)                                  \
   for (struct ir3_register *__srcreg = (void *)~0; __srcreg; __srcreg = NULL) \
      for (int __cnt = (__instr)->srcs_count, __i = __cnt - 1; __i >= 0;       \
           __i--)                                                              \
         if (ra_reg_is_src((__srcreg = (__instr)->srcs[__i])))

#define ra_foreach_dst_n(__dstreg, __n, __instr)                               \
   foreach_dst_n(__dstreg, __n, __instr)                                         \
      if (ra_reg_is_dst(__dstreg))

#define ra_foreach_dst(__dstreg, __instr)                                      \
   ra_foreach_dst_n(__dstreg, __i, __instr)

#define RA_HALF_SIZE     (4 * 48)
#define RA_FULL_SIZE     (4 * 48 * 2)
#define RA_SHARED_SIZE   (2 * 4 * 8)
#define RA_MAX_FILE_SIZE RA_FULL_SIZE

struct ir3_liveness {
   unsigned block_count;
   unsigned interval_offset;
   DECLARE_ARRAY(struct ir3_register *, definitions);
   DECLARE_ARRAY(BITSET_WORD *, live_out);
   DECLARE_ARRAY(BITSET_WORD *, live_in);
};

struct ir3_liveness *ir3_calc_liveness(void *mem_ctx, struct ir3 *ir);

bool ir3_def_live_after(struct ir3_liveness *live, struct ir3_register *def,
                        struct ir3_instruction *instr);

void ir3_create_parallel_copies(struct ir3 *ir);

void ir3_merge_regs(struct ir3_liveness *live, struct ir3 *ir);

void ir3_force_merge(struct ir3_register *a, struct ir3_register *b,
                     int b_offset);

struct ir3_pressure {
   unsigned full, half, shared;
};

void ir3_calc_pressure(struct ir3_shader_variant *v, struct ir3_liveness *live,
                       struct ir3_pressure *max_pressure);

bool ir3_spill(struct ir3 *ir, struct ir3_shader_variant *v,
               struct ir3_liveness **live,
               const struct ir3_pressure *limit_pressure);

bool ir3_lower_spill(struct ir3 *ir);

void ir3_ra_validate(struct ir3_shader_variant *v, unsigned full_size,
                     unsigned half_size, unsigned block_count);

void ir3_lower_copies(struct ir3_shader_variant *v);

/* Register interval datastructure
 *
 * ir3_reg_ctx is used to track which registers are live. The tricky part is
 * that some registers may overlap each other, when registers with overlapping
 * live ranges get coalesced. For example, splits will overlap with their
 * parent vector and sometimes collect sources will also overlap with the
 * collect'ed vector. ir3_merge_regs guarantees for us that none of the
 * registers in a merge set that are live at any given point partially
 * overlap, which means that we can organize them into a forest. While each
 * register has a per-merge-set offset, ir3_merge_regs also computes a
 * "global" offset which allows us to throw away the original merge sets and
 * think of registers as just intervals in a forest of live intervals. When a
 * register becomes live, we insert it into the forest, and when it dies we
 * remove it from the forest (and then its children get moved up a level). We
 * use red-black trees to keep track of each level of the forest, so insertion
 * and deletion should be fast operations. ir3_reg_ctx handles all the
 * internal bookkeeping for this, so that it can be shared between RA,
 * spilling, and register pressure tracking.
 */

struct ir3_reg_interval {
   struct rb_node node;

   struct rb_tree children;

   struct ir3_reg_interval *parent;

   struct ir3_register *reg;

   bool inserted;
};

struct ir3_reg_ctx {
   /* The tree of top-level intervals in the forest. */
   struct rb_tree intervals;

   /* Users of ir3_reg_ctx need to keep around additional state that is
    * modified when top-level intervals are added or removed. For register
    * pressure tracking, this is just the register pressure, but for RA we
    * need to keep track of the physreg of each top-level interval. These
    * callbacks provide a place to let users deriving from ir3_reg_ctx update
    * their state when top-level intervals are inserted/removed.
    */

   /* Called when an interval is added and it turns out to be at the top
    * level.
    */
   void (*interval_add)(struct ir3_reg_ctx *ctx,
                        struct ir3_reg_interval *interval);

   /* Called when an interval is deleted from the top level. */
   void (*interval_delete)(struct ir3_reg_ctx *ctx,
                           struct ir3_reg_interval *interval);

   /* Called when an interval is deleted and its child becomes top-level.
    */
   void (*interval_readd)(struct ir3_reg_ctx *ctx,
                          struct ir3_reg_interval *parent,
                          struct ir3_reg_interval *child);
};

static inline struct ir3_reg_interval *
ir3_rb_node_to_interval(struct rb_node *node)
{
   return rb_node_data(struct ir3_reg_interval, node, node);
}

static inline const struct ir3_reg_interval *
ir3_rb_node_to_interval_const(const struct rb_node *node)
{
   return rb_node_data(struct ir3_reg_interval, node, node);
}

static inline struct ir3_reg_interval *
ir3_reg_interval_next(struct ir3_reg_interval *interval)
{
   struct rb_node *next = rb_node_next(&interval->node);
   return next ? ir3_rb_node_to_interval(next) : NULL;
}

static inline struct ir3_reg_interval *
ir3_reg_interval_next_or_null(struct ir3_reg_interval *interval)
{
   return interval ? ir3_reg_interval_next(interval) : NULL;
}

static inline void
ir3_reg_interval_init(struct ir3_reg_interval *interval,
                      struct ir3_register *reg)
{
   rb_tree_init(&interval->children);
   interval->reg = reg;
   interval->parent = NULL;
   interval->inserted = false;
}

void ir3_reg_interval_dump(struct log_stream *stream,
                           struct ir3_reg_interval *interval);

void ir3_reg_interval_insert(struct ir3_reg_ctx *ctx,
                             struct ir3_reg_interval *interval);

void ir3_reg_interval_remove(struct ir3_reg_ctx *ctx,
                             struct ir3_reg_interval *interval);

void ir3_reg_interval_remove_all(struct ir3_reg_ctx *ctx,
                                 struct ir3_reg_interval *interval);

#endif
