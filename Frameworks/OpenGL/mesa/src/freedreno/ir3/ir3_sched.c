/*
 * Copyright (C) 2014 Rob Clark <robclark@freedesktop.org>
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
 * Authors:
 *    Rob Clark <robclark@freedesktop.org>
 */

#include "util/dag.h"
#include "util/u_math.h"

#include "ir3.h"
#include "ir3_compiler.h"

#ifdef DEBUG
#define SCHED_DEBUG (ir3_shader_debug & IR3_DBG_SCHEDMSGS)
#else
#define SCHED_DEBUG 0
#endif
#define d(fmt, ...)                                                            \
   do {                                                                        \
      if (SCHED_DEBUG) {                                                       \
         mesa_logi("SCHED: " fmt, ##__VA_ARGS__);                              \
      }                                                                        \
   } while (0)

#define di(instr, fmt, ...)                                                    \
   do {                                                                        \
      if (SCHED_DEBUG) {                                                       \
         struct log_stream *stream = mesa_log_streami();                       \
         mesa_log_stream_printf(stream, "SCHED: " fmt ": ", ##__VA_ARGS__);    \
         ir3_print_instr_stream(stream, instr);                                \
         mesa_log_stream_destroy(stream);                                      \
      }                                                                        \
   } while (0)

/*
 * Instruction Scheduling:
 *
 * A block-level pre-RA scheduler, which works by creating a DAG of
 * instruction dependencies, and heuristically picking a DAG head
 * (instruction with no unscheduled dependencies).
 *
 * Where possible, it tries to pick instructions that avoid nop delay
 * slots, but it will prefer to pick instructions that reduce (or do
 * not increase) the number of live values.
 *
 * If the only possible choices are instructions that increase the
 * number of live values, it will try to pick the one with the earliest
 * consumer (based on pre-sched program order).
 *
 * There are a few special cases that need to be handled, since sched
 * is currently independent of register allocation.  Usages of address
 * register (a0.x) or predicate register (p0.x) must be serialized.  Ie.
 * if you have two pairs of instructions that write the same special
 * register and then read it, then those pairs cannot be interleaved.
 * To solve this, when we are in such a scheduling "critical section",
 * and we encounter a conflicting write to a special register, we try
 * to schedule any remaining instructions that use that value first.
 *
 * TODO we can detect too-large live_values here.. would be a good place
 * to "spill" cheap things, like move from uniform/immed.  (Constructing
 * list of ssa def consumers before sched pass would make this easier.
 * Also, in general it is general it might be best not to re-use load_immed
 * across blocks.
 *
 * TODO we can use (abs)/(neg) src modifiers in a lot of cases to reduce
 * the # of immediates in play (or at least that would help with
 * dEQP-GLES31.functional.ubo.random.all_per_block_buffers.*).. probably
 * do this in a nir pass that inserts fneg/etc?  The cp pass should fold
 * these into src modifiers..
 */

struct ir3_sched_ctx {
   struct ir3_block *block; /* the current block */
   struct dag *dag;

   struct list_head unscheduled_list; /* unscheduled instructions */
   struct ir3_instruction *scheduled; /* last scheduled instr */
   struct ir3_instruction *addr0;     /* current a0.x user, if any */
   struct ir3_instruction *addr1;     /* current a1.x user, if any */
   struct ir3_instruction *pred;      /* current p0.x user, if any */

   struct ir3_instruction *split; /* most-recently-split a0/a1/p0 producer */

   int remaining_kills;
   int remaining_tex;

   bool error;

   unsigned ip;

   int sy_delay;
   int ss_delay;

   /* We order the scheduled (sy)/(ss) producers, and keep track of the
    * index of the last waited on instruction, so we can know which
    * instructions are still outstanding (and therefore would require us to
    * wait for all outstanding instructions before scheduling a use).
    */
   int sy_index, first_outstanding_sy_index;
   int ss_index, first_outstanding_ss_index;
};

struct ir3_sched_node {
   struct dag_node dag; /* must be first for util_dynarray_foreach */
   struct ir3_instruction *instr;

   unsigned delay;
   unsigned max_delay;

   unsigned sy_index;
   unsigned ss_index;

   /* For ready instructions, the earliest possible ip that it could be
    * scheduled.
    */
   unsigned earliest_ip;

   /* For instructions that are a meta:collect src, once we schedule
    * the first src of the collect, the entire vecN is live (at least
    * from the PoV of the first RA pass.. the 2nd scalar pass can fill
    * in some of the gaps, but often not all).  So we want to help out
    * RA, and realize that as soon as we schedule the first collect
    * src, there is no penalty to schedule the remainder (ie. they
    * don't make additional values live).  In fact we'd prefer to
    * schedule the rest ASAP to minimize the live range of the vecN.
    *
    * For instructions that are the src of a collect, we track the
    * corresponding collect, and mark them as partially live as soon
    * as any one of the src's is scheduled.
    */
   struct ir3_instruction *collect;
   bool partially_live;

   /* Is this instruction a direct or indirect dependency for a kill?
    * If so, we should prioritize it when possible
    */
   bool kill_path;

   /* This node represents a shader output.  A semi-common pattern in
    * shaders is something along the lines of:
    *
    *    fragcolor.w = 1.0
    *
    * Which we'd prefer to schedule as late as possible, since it
    * produces a live value that is never killed/consumed.  So detect
    * outputs up-front, and avoid scheduling them unless the reduce
    * register pressure (or at least are neutral)
    */
   bool output;
};

#define foreach_sched_node(__n, __list)                                        \
   list_for_each_entry (struct ir3_sched_node, __n, __list, dag.link)

static void sched_node_init(struct ir3_sched_ctx *ctx,
                            struct ir3_instruction *instr);
static void sched_node_add_dep(struct ir3_instruction *instr,
                               struct ir3_instruction *src, int i);

static bool
is_scheduled(struct ir3_instruction *instr)
{
   return !!(instr->flags & IR3_INSTR_MARK);
}

/* check_src_cond() passing a ir3_sched_ctx. */
static bool
sched_check_src_cond(struct ir3_instruction *instr,
                     bool (*cond)(struct ir3_instruction *,
                                  struct ir3_sched_ctx *),
                     struct ir3_sched_ctx *ctx)
{
   foreach_ssa_src (src, instr) {
      /* meta:split/collect aren't real instructions, the thing that
       * we actually care about is *their* srcs
       */
      if ((src->opc == OPC_META_SPLIT) || (src->opc == OPC_META_COLLECT)) {
         if (sched_check_src_cond(src, cond, ctx))
            return true;
      } else {
         if (cond(src, ctx))
            return true;
      }
   }

   return false;
}

/* Is this a sy producer that hasn't been waited on yet? */

static bool
is_outstanding_sy(struct ir3_instruction *instr, struct ir3_sched_ctx *ctx)
{
   if (!is_sy_producer(instr))
      return false;

   /* The sched node is only valid within the same block, we cannot
    * really say anything about src's from other blocks
    */
   if (instr->block != ctx->block)
      return true;

   struct ir3_sched_node *n = instr->data;
   return n->sy_index >= ctx->first_outstanding_sy_index;
}

static bool
is_outstanding_ss(struct ir3_instruction *instr, struct ir3_sched_ctx *ctx)
{
   if (!is_ss_producer(instr))
      return false;

   /* The sched node is only valid within the same block, we cannot
    * really say anything about src's from other blocks
    */
   if (instr->block != ctx->block)
      return true;

   struct ir3_sched_node *n = instr->data;
   return n->ss_index >= ctx->first_outstanding_ss_index;
}

static unsigned
cycle_count(struct ir3_instruction *instr)
{
   if (instr->opc == OPC_META_COLLECT) {
      /* Assume that only immed/const sources produce moves */
      unsigned n = 0;
      foreach_src (src, instr) {
         if (src->flags & (IR3_REG_IMMED | IR3_REG_CONST))
            n++;
      }
      return n;
   } else if (is_meta(instr)) {
      return 0;
   } else {
      return 1;
   }
}

static void
schedule(struct ir3_sched_ctx *ctx, struct ir3_instruction *instr)
{
   assert(ctx->block == instr->block);

   /* remove from depth list:
    */
   list_delinit(&instr->node);

   if (writes_addr0(instr)) {
      assert(ctx->addr0 == NULL);
      ctx->addr0 = instr;
   }

   if (writes_addr1(instr)) {
      assert(ctx->addr1 == NULL);
      ctx->addr1 = instr;
   }

   if (writes_pred(instr)) {
      assert(ctx->pred == NULL);
      ctx->pred = instr;
   }

   instr->flags |= IR3_INSTR_MARK;

   di(instr, "schedule");

   list_addtail(&instr->node, &instr->block->instr_list);
   ctx->scheduled = instr;

   if (is_kill_or_demote(instr)) {
      assert(ctx->remaining_kills > 0);
      ctx->remaining_kills--;
   }

   struct ir3_sched_node *n = instr->data;

   /* If this instruction is a meta:collect src, mark the remaining
    * collect srcs as partially live.
    */
   if (n->collect) {
      foreach_ssa_src (src, n->collect) {
         if (src->block != instr->block)
            continue;
         struct ir3_sched_node *sn = src->data;
         sn->partially_live = true;
      }
   }

   bool counts_for_delay = is_alu(instr) || is_flow(instr);

   /* TODO: switch to "cycles". For now try to match ir3_delay. */
   unsigned delay_cycles = counts_for_delay ? 1 + instr->repeat : 0;

   /* We insert any nop's needed to get to earliest_ip, then advance
    * delay_cycles by scheduling the instruction.
    */
   ctx->ip = MAX2(ctx->ip, n->earliest_ip) + delay_cycles;

   util_dynarray_foreach (&n->dag.edges, struct dag_edge, edge) {
      unsigned delay = (unsigned)(uintptr_t)edge->data;
      struct ir3_sched_node *child =
         container_of(edge->child, struct ir3_sched_node, dag);
      child->earliest_ip = MAX2(child->earliest_ip, ctx->ip + delay);
   }

   dag_prune_head(ctx->dag, &n->dag);

   unsigned cycles = cycle_count(instr);

   if (is_ss_producer(instr)) {
      ctx->ss_delay = soft_ss_delay(instr);
      n->ss_index = ctx->ss_index++;
   } else if (!is_meta(instr) &&
              sched_check_src_cond(instr, is_outstanding_ss, ctx)) {
      ctx->ss_delay = 0;
      ctx->first_outstanding_ss_index = ctx->ss_index;
   } else if (ctx->ss_delay > 0) {
      ctx->ss_delay -= MIN2(cycles, ctx->ss_delay);
   }

   if (is_sy_producer(instr)) {
      /* NOTE that this isn't an attempt to hide texture fetch latency,
       * but an attempt to hide the cost of switching to another warp.
       * If we can, we'd like to try to schedule another texture fetch
       * before scheduling something that would sync.
       */
      ctx->sy_delay = soft_sy_delay(instr, ctx->block->shader);
      assert(ctx->remaining_tex > 0);
      ctx->remaining_tex--;
      n->sy_index = ctx->sy_index++;
   } else if (!is_meta(instr) &&
              sched_check_src_cond(instr, is_outstanding_sy, ctx)) {
      ctx->sy_delay = 0;
      ctx->first_outstanding_sy_index = ctx->sy_index;
   } else if (ctx->sy_delay > 0) {
      ctx->sy_delay -= MIN2(cycles, ctx->sy_delay);
   }

}

struct ir3_sched_notes {
   /* there is at least one kill which could be scheduled, except
    * for unscheduled bary.f's:
    */
   bool blocked_kill;
   /* there is at least one instruction that could be scheduled,
    * except for conflicting address/predicate register usage:
    */
   bool addr0_conflict, addr1_conflict, pred_conflict;
};

static bool
should_skip(struct ir3_sched_ctx *ctx, struct ir3_instruction *instr)
{
   if (ctx->remaining_kills && (is_tex(instr) || is_mem(instr))) {
      /* avoid texture/memory access if we have unscheduled kills
       * that could make the expensive operation unnecessary.  By
       * definition, if there are remaining kills, and this instr
       * is not a dependency of a kill, there are other instructions
       * that we can choose from.
       */
      struct ir3_sched_node *n = instr->data;
      if (!n->kill_path)
         return true;
   }

   return false;
}

/* could an instruction be scheduled if specified ssa src was scheduled? */
static bool
could_sched(struct ir3_sched_ctx *ctx,
            struct ir3_instruction *instr, struct ir3_instruction *src)
{
   foreach_ssa_src (other_src, instr) {
      /* if dependency not scheduled, we aren't ready yet: */
      if ((src != other_src) && !is_scheduled(other_src)) {
         return false;
      }
   }

   /* Instructions not in the current block can never be scheduled.
    */
   if (instr->block != src->block)
      return false;

   return !should_skip(ctx, instr);
}

/* Check if instruction is ok to schedule.  Make sure it is not blocked
 * by use of addr/predicate register, etc.
 */
static bool
check_instr(struct ir3_sched_ctx *ctx, struct ir3_sched_notes *notes,
            struct ir3_instruction *instr)
{
   assert(!is_scheduled(instr));

   if (instr == ctx->split) {
      /* Don't schedule instructions created by splitting a a0.x/a1.x/p0.x
       * write until another "normal" instruction has been scheduled.
       */
      return false;
   }

   if (should_skip(ctx, instr))
       return false;

   /* For instructions that write address register we need to
    * make sure there is at least one instruction that uses the
    * addr value which is otherwise ready.
    *
    * NOTE if any instructions use pred register and have other
    * src args, we would need to do the same for writes_pred()..
    */
   if (writes_addr0(instr)) {
      struct ir3 *ir = instr->block->shader;
      bool ready = false;
      for (unsigned i = 0; (i < ir->a0_users_count) && !ready; i++) {
         struct ir3_instruction *indirect = ir->a0_users[i];
         if (!indirect)
            continue;
         if (indirect->address->def != instr->dsts[0])
            continue;
         ready = could_sched(ctx, indirect, instr);
      }

      /* nothing could be scheduled, so keep looking: */
      if (!ready)
         return false;
   }

   if (writes_addr1(instr)) {
      struct ir3 *ir = instr->block->shader;
      bool ready = false;
      for (unsigned i = 0; (i < ir->a1_users_count) && !ready; i++) {
         struct ir3_instruction *indirect = ir->a1_users[i];
         if (!indirect)
            continue;
         if (indirect->address->def != instr->dsts[0])
            continue;
         ready = could_sched(ctx, indirect, instr);
      }

      /* nothing could be scheduled, so keep looking: */
      if (!ready)
         return false;
   }

   /* if this is a write to address/predicate register, and that
    * register is currently in use, we need to defer until it is
    * free:
    */
   if (writes_addr0(instr) && ctx->addr0) {
      assert(ctx->addr0 != instr);
      notes->addr0_conflict = true;
      return false;
   }

   if (writes_addr1(instr) && ctx->addr1) {
      assert(ctx->addr1 != instr);
      notes->addr1_conflict = true;
      return false;
   }

   if (writes_pred(instr) && ctx->pred) {
      assert(ctx->pred != instr);
      notes->pred_conflict = true;
      return false;
   }

   /* if the instruction is a kill, we need to ensure *every*
    * bary.f is scheduled.  The hw seems unhappy if the thread
    * gets killed before the end-input (ei) flag is hit.
    *
    * We could do this by adding each bary.f instruction as
    * virtual ssa src for the kill instruction.  But we have
    * fixed length instr->srcs[].
    *
    * TODO we could handle this by false-deps now, probably.
    */
   if (is_kill_or_demote(instr)) {
      struct ir3 *ir = instr->block->shader;

      for (unsigned i = 0; i < ir->baryfs_count; i++) {
         struct ir3_instruction *baryf = ir->baryfs[i];
         if (baryf->flags & IR3_INSTR_UNUSED)
            continue;
         if (!is_scheduled(baryf)) {
            notes->blocked_kill = true;
            return false;
         }
      }
   }

   return true;
}

/* Find the instr->ip of the closest use of an instruction, in
 * pre-sched order.  This isn't going to be the same as post-sched
 * order, but it is a reasonable approximation to limit scheduling
 * instructions *too* early.  This is mostly to prevent bad behavior
 * in cases where we have a large number of possible instructions
 * to choose, to avoid creating too much parallelism (ie. blowing
 * up register pressure)
 *
 * See
 * dEQP-GLES31.functional.atomic_counter.layout.reverse_offset.inc_dec.8_counters_5_calls_1_thread
 */
static int
nearest_use(struct ir3_instruction *instr)
{
   unsigned nearest = ~0;
   foreach_ssa_use (use, instr)
      if (!is_scheduled(use))
         nearest = MIN2(nearest, use->ip);

   /* slight hack.. this heuristic tends to push bary.f's to later
    * in the shader, closer to their uses.  But we actually would
    * prefer to get these scheduled earlier, to unlock varying
    * storage for more VS jobs:
    */
   if (is_input(instr))
      nearest /= 2;

   return nearest;
}

static bool
is_only_nonscheduled_use(struct ir3_instruction *instr,
                         struct ir3_instruction *use)
{
   foreach_ssa_use (other_use, instr) {
      if (other_use != use && !is_scheduled(other_use))
         return false;
   }

   return true;
}

static unsigned
new_regs(struct ir3_instruction *instr)
{
   unsigned regs = 0;

   foreach_dst (dst, instr) {
      if (!is_dest_gpr(dst))
         continue;
      regs += reg_elems(dst);
   }

   return regs;
}

/* find net change to live values if instruction were scheduled: */
static int
live_effect(struct ir3_instruction *instr)
{
   struct ir3_sched_node *n = instr->data;
   int new_live =
      (n->partially_live || !instr->uses || instr->uses->entries == 0)
         ? 0
         : new_regs(instr);
   int freed_live = 0;

   /* if we schedule something that causes a vecN to be live,
    * then count all it's other components too:
    */
   if (n->collect)
      new_live *= n->collect->srcs_count;

   foreach_ssa_src_n (src, n, instr) {
      if (__is_false_dep(instr, n))
         continue;

      if (instr->block != src->block)
         continue;

      if (is_only_nonscheduled_use(src, instr))
         freed_live += new_regs(src);
   }

   return new_live - freed_live;
}

/* Determine if this is an instruction that we'd prefer not to schedule
 * yet, in order to avoid an (ss)/(sy) sync.  This is limited by the
 * ss_delay/sy_delay counters, ie. the more cycles it has been since
 * the last SFU/tex, the less costly a sync would be, and the number of
 * outstanding SFU/tex instructions to prevent a blowup in register pressure.
 */
static bool
should_defer(struct ir3_sched_ctx *ctx, struct ir3_instruction *instr)
{
   if (ctx->ss_delay) {
      if (sched_check_src_cond(instr, is_outstanding_ss, ctx))
         return true;
   }

   /* We mostly just want to try to schedule another texture fetch
    * before scheduling something that would (sy) sync, so we can
    * limit this rule to cases where there are remaining texture
    * fetches
    */
   if (ctx->sy_delay && ctx->remaining_tex) {
      if (sched_check_src_cond(instr, is_outstanding_sy, ctx))
         return true;
   }

   /* Avoid scheduling too many outstanding texture or sfu instructions at
    * once by deferring further tex/SFU instructions. This both prevents
    * stalls when the queue of texture/sfu instructions becomes too large,
    * and prevents unacceptably large increases in register pressure from too
    * many outstanding texture instructions.
    */
   if (ctx->sy_index - ctx->first_outstanding_sy_index >= 8 && is_sy_producer(instr))
      return true;

   if (ctx->ss_index - ctx->first_outstanding_ss_index >= 8 && is_ss_producer(instr))
      return true;

   return false;
}

static struct ir3_sched_node *choose_instr_inc(struct ir3_sched_ctx *ctx,
                                               struct ir3_sched_notes *notes,
                                               bool defer, bool avoid_output);

enum choose_instr_dec_rank {
   DEC_NEUTRAL,
   DEC_NEUTRAL_READY,
   DEC_FREED,
   DEC_FREED_READY,
};

static const char *
dec_rank_name(enum choose_instr_dec_rank rank)
{
   switch (rank) {
   case DEC_NEUTRAL:
      return "neutral";
   case DEC_NEUTRAL_READY:
      return "neutral+ready";
   case DEC_FREED:
      return "freed";
   case DEC_FREED_READY:
      return "freed+ready";
   default:
      return NULL;
   }
}

static unsigned
node_delay(struct ir3_sched_ctx *ctx, struct ir3_sched_node *n)
{
   return MAX2(n->earliest_ip, ctx->ip) - ctx->ip;
}

/**
 * Chooses an instruction to schedule using the Goodman/Hsu (1988) CSR (Code
 * Scheduling for Register pressure) heuristic.
 *
 * Only handles the case of choosing instructions that reduce register pressure
 * or are even.
 */
static struct ir3_sched_node *
choose_instr_dec(struct ir3_sched_ctx *ctx, struct ir3_sched_notes *notes,
                 bool defer)
{
   const char *mode = defer ? "-d" : "";
   struct ir3_sched_node *chosen = NULL;
   enum choose_instr_dec_rank chosen_rank = DEC_NEUTRAL;

   foreach_sched_node (n, &ctx->dag->heads) {
      if (defer && should_defer(ctx, n->instr))
         continue;

      unsigned d = node_delay(ctx, n);

      int live = live_effect(n->instr);
      if (live > 0)
         continue;

      if (!check_instr(ctx, notes, n->instr))
         continue;

      enum choose_instr_dec_rank rank;
      if (live < 0) {
         /* Prioritize instrs which free up regs and can be scheduled with no
          * delay.
          */
         if (d == 0)
            rank = DEC_FREED_READY;
         else
            rank = DEC_FREED;
      } else {
         /* Contra the paper, pick a leader with no effect on used regs.  This
          * may open up new opportunities, as otherwise a single-operand instr
          * consuming a value will tend to block finding freeing that value.
          * This had a massive effect on reducing spilling on V3D.
          *
          * XXX: Should this prioritize ready?
          */
         if (d == 0)
            rank = DEC_NEUTRAL_READY;
         else
            rank = DEC_NEUTRAL;
      }

      /* Prefer higher-ranked instructions, or in the case of a rank tie, the
       * highest latency-to-end-of-program instruction.
       */
      if (!chosen || rank > chosen_rank ||
          (rank == chosen_rank && chosen->max_delay < n->max_delay)) {
         chosen = n;
         chosen_rank = rank;
      }
   }

   if (chosen) {
      di(chosen->instr, "dec%s: chose (%s)", mode, dec_rank_name(chosen_rank));
      return chosen;
   }

   return choose_instr_inc(ctx, notes, defer, true);
}

enum choose_instr_inc_rank {
   INC_DISTANCE,
   INC_DISTANCE_READY,
};

static const char *
inc_rank_name(enum choose_instr_inc_rank rank)
{
   switch (rank) {
   case INC_DISTANCE:
      return "distance";
   case INC_DISTANCE_READY:
      return "distance+ready";
   default:
      return NULL;
   }
}

/**
 * When we can't choose an instruction that reduces register pressure or
 * is neutral, we end up here to try and pick the least bad option.
 */
static struct ir3_sched_node *
choose_instr_inc(struct ir3_sched_ctx *ctx, struct ir3_sched_notes *notes,
                 bool defer, bool avoid_output)
{
   const char *mode = defer ? "-d" : "";
   struct ir3_sched_node *chosen = NULL;
   enum choose_instr_inc_rank chosen_rank = INC_DISTANCE;

   /*
    * From hear on out, we are picking something that increases
    * register pressure.  So try to pick something which will
    * be consumed soon:
    */
   unsigned chosen_distance = 0;

   /* Pick the max delay of the remaining ready set. */
   foreach_sched_node (n, &ctx->dag->heads) {
      if (avoid_output && n->output)
         continue;

      if (defer && should_defer(ctx, n->instr))
         continue;

      if (!check_instr(ctx, notes, n->instr))
         continue;

      unsigned d = node_delay(ctx, n);

      enum choose_instr_inc_rank rank;
      if (d == 0)
         rank = INC_DISTANCE_READY;
      else
         rank = INC_DISTANCE;

      unsigned distance = nearest_use(n->instr);

      if (!chosen || rank > chosen_rank ||
          (rank == chosen_rank && distance < chosen_distance)) {
         chosen = n;
         chosen_distance = distance;
         chosen_rank = rank;
      }
   }

   if (chosen) {
      di(chosen->instr, "inc%s: chose (%s)", mode, inc_rank_name(chosen_rank));
      return chosen;
   }

   return NULL;
}

/* Handles instruction selections for instructions we want to prioritize
 * even if csp/csr would not pick them.
 */
static struct ir3_sched_node *
choose_instr_prio(struct ir3_sched_ctx *ctx, struct ir3_sched_notes *notes)
{
   struct ir3_sched_node *chosen = NULL;

   foreach_sched_node (n, &ctx->dag->heads) {
      /*
       * - phi nodes and inputs must be scheduled first
       * - split should be scheduled first, so that the vector value is
       *   killed as soon as possible. RA cannot split up the vector and
       *   reuse components that have been killed until it's been killed.
       * - collect, on the other hand, should be treated as a "normal"
       *   instruction, and may add to register pressure if its sources are
       *   part of another vector or immediates.
       */
      if (!is_meta(n->instr) || n->instr->opc == OPC_META_COLLECT)
         continue;

      if (!chosen || (chosen->max_delay < n->max_delay))
         chosen = n;
   }

   if (chosen) {
      di(chosen->instr, "prio: chose (meta)");
      return chosen;
   }

   return NULL;
}

static void
dump_state(struct ir3_sched_ctx *ctx)
{
   if (!SCHED_DEBUG)
      return;

   foreach_sched_node (n, &ctx->dag->heads) {
      di(n->instr, "maxdel=%3d le=%d del=%u ", n->max_delay,
         live_effect(n->instr), node_delay(ctx, n));

      util_dynarray_foreach (&n->dag.edges, struct dag_edge, edge) {
         struct ir3_sched_node *child = (struct ir3_sched_node *)edge->child;

         di(child->instr, " -> (%d parents) ", child->dag.parent_count);
      }
   }
}

/* find instruction to schedule: */
static struct ir3_instruction *
choose_instr(struct ir3_sched_ctx *ctx, struct ir3_sched_notes *notes)
{
   struct ir3_sched_node *chosen;

   dump_state(ctx);

   chosen = choose_instr_prio(ctx, notes);
   if (chosen)
      return chosen->instr;

   chosen = choose_instr_dec(ctx, notes, true);
   if (chosen)
      return chosen->instr;

   chosen = choose_instr_dec(ctx, notes, false);
   if (chosen)
      return chosen->instr;

   chosen = choose_instr_inc(ctx, notes, false, false);
   if (chosen)
      return chosen->instr;

   return NULL;
}

static struct ir3_instruction *
split_instr(struct ir3_sched_ctx *ctx, struct ir3_instruction *orig_instr)
{
   struct ir3_instruction *new_instr = ir3_instr_clone(orig_instr);
   di(new_instr, "split instruction");
   sched_node_init(ctx, new_instr);
   return new_instr;
}

/* "spill" the address registers by remapping any unscheduled
 * instructions which depend on the current address register
 * to a clone of the instruction which wrote the address reg.
 */
static struct ir3_instruction *
split_addr(struct ir3_sched_ctx *ctx, struct ir3_instruction **addr,
           struct ir3_instruction **users, unsigned users_count)
{
   struct ir3_instruction *new_addr = NULL;
   unsigned i;

   assert(*addr);

   for (i = 0; i < users_count; i++) {
      struct ir3_instruction *indirect = users[i];

      if (!indirect)
         continue;

      /* skip instructions already scheduled: */
      if (is_scheduled(indirect))
         continue;

      /* remap remaining instructions using current addr
       * to new addr:
       */
      if (indirect->address->def == (*addr)->dsts[0]) {
         if (!new_addr) {
            new_addr = split_instr(ctx, *addr);
            /* original addr is scheduled, but new one isn't: */
            new_addr->flags &= ~IR3_INSTR_MARK;
         }
         indirect->address->def = new_addr->dsts[0];
         /* don't need to remove old dag edge since old addr is
          * already scheduled:
          */
         sched_node_add_dep(indirect, new_addr, 0);
         di(indirect, "new address");
      }
   }

   /* all remaining indirects remapped to new addr: */
   *addr = NULL;

   return new_addr;
}

/* "spill" the predicate register by remapping any unscheduled
 * instructions which depend on the current predicate register
 * to a clone of the instruction which wrote the address reg.
 */
static struct ir3_instruction *
split_pred(struct ir3_sched_ctx *ctx)
{
   struct ir3 *ir;
   struct ir3_instruction *new_pred = NULL;
   unsigned i;

   assert(ctx->pred);

   ir = ctx->pred->block->shader;

   for (i = 0; i < ir->predicates_count; i++) {
      struct ir3_instruction *predicated = ir->predicates[i];

      if (!predicated)
         continue;

      /* skip instructions already scheduled: */
      if (is_scheduled(predicated))
         continue;

      /* remap remaining instructions using current pred
       * to new pred:
       *
       * TODO is there ever a case when pred isn't first
       * (and only) src?
       */
      if (ssa(predicated->srcs[0]) == ctx->pred) {
         if (!new_pred) {
            new_pred = split_instr(ctx, ctx->pred);
            /* original pred is scheduled, but new one isn't: */
            new_pred->flags &= ~IR3_INSTR_MARK;
         }
         predicated->srcs[0]->def->instr = new_pred;
         /* don't need to remove old dag edge since old pred is
          * already scheduled:
          */
         sched_node_add_dep(predicated, new_pred, 0);
         di(predicated, "new predicate");
      }
   }

   if (ctx->block->condition == ctx->pred) {
      if (!new_pred) {
         new_pred = split_instr(ctx, ctx->pred);
         /* original pred is scheduled, but new one isn't: */
         new_pred->flags &= ~IR3_INSTR_MARK;
      }
      ctx->block->condition = new_pred;
      d("new branch condition");
   }

   /* all remaining predicated remapped to new pred: */
   ctx->pred = NULL;

   return new_pred;
}

static void
sched_node_init(struct ir3_sched_ctx *ctx, struct ir3_instruction *instr)
{
   struct ir3_sched_node *n = rzalloc(ctx->dag, struct ir3_sched_node);

   dag_init_node(ctx->dag, &n->dag);

   n->instr = instr;
   instr->data = n;
}

static void
sched_node_add_dep(struct ir3_instruction *instr, struct ir3_instruction *src,
                   int i)
{
   /* don't consider dependencies in other blocks: */
   if (src->block != instr->block)
      return;

   /* we could have false-dep's that end up unused: */
   if (src->flags & IR3_INSTR_UNUSED) {
      assert(__is_false_dep(instr, i));
      return;
   }

   struct ir3_sched_node *n = instr->data;
   struct ir3_sched_node *sn = src->data;

   /* If src is consumed by a collect, track that to realize that once
    * any of the collect srcs are live, we should hurry up and schedule
    * the rest.
    */
   if (instr->opc == OPC_META_COLLECT)
      sn->collect = instr;

   unsigned d_soft = ir3_delayslots(src, instr, i, true);
   unsigned d = ir3_delayslots(src, instr, i, false);

   /* delays from (ss) and (sy) are considered separately and more accurately in
    * the scheduling heuristic, so ignore it when calculating the ip of
    * instructions, but do consider it when prioritizing which instructions to
    * schedule.
    */
   dag_add_edge_max_data(&sn->dag, &n->dag, (uintptr_t)d);

   n->delay = MAX2(n->delay, d_soft);
}

static void
mark_kill_path(struct ir3_instruction *instr)
{
   struct ir3_sched_node *n = instr->data;

   if (n->kill_path) {
      return;
   }

   n->kill_path = true;

   foreach_ssa_src (src, instr) {
      if (src->block != instr->block)
         continue;
      mark_kill_path(src);
   }
}

/* Is it an output? */
static bool
is_output_collect(struct ir3_instruction *instr)
{
   if (instr->opc != OPC_META_COLLECT)
      return false;

   foreach_ssa_use (use, instr) {
      if (use->opc != OPC_END && use->opc != OPC_CHMASK)
         return false;
   }

   return true;
}

/* Is it's only use as output? */
static bool
is_output_only(struct ir3_instruction *instr)
{
   foreach_ssa_use (use, instr)
      if (!is_output_collect(use))
         return false;

   return true;
}

static void
sched_node_add_deps(struct ir3_instruction *instr)
{
   /* There's nothing to do for phi nodes, since they always go first. And
    * phi nodes can reference sources later in the same block, so handling
    * sources is not only unnecessary but could cause problems.
    */
   if (instr->opc == OPC_META_PHI)
      return;

   /* Since foreach_ssa_src() already handles false-dep's we can construct
    * the DAG easily in a single pass.
    */
   foreach_ssa_src_n (src, i, instr) {
      sched_node_add_dep(instr, src, i);
   }

   /* NOTE that all inputs must be scheduled before a kill, so
    * mark these to be prioritized as well:
    */
   if (is_kill_or_demote(instr) || is_input(instr)) {
      mark_kill_path(instr);
   }

   if (is_output_only(instr)) {
      struct ir3_sched_node *n = instr->data;
      n->output = true;
   }
}

static void
sched_dag_max_delay_cb(struct dag_node *node, void *state)
{
   struct ir3_sched_node *n = (struct ir3_sched_node *)node;
   uint32_t max_delay = 0;

   util_dynarray_foreach (&n->dag.edges, struct dag_edge, edge) {
      struct ir3_sched_node *child = (struct ir3_sched_node *)edge->child;
      max_delay = MAX2(child->max_delay, max_delay);
   }

   n->max_delay = MAX2(n->max_delay, max_delay + n->delay);
}

static void
sched_dag_validate_cb(const struct dag_node *node, void *data)
{
   struct ir3_sched_node *n = (struct ir3_sched_node *)node;

   ir3_print_instr(n->instr);
}

static void
sched_dag_init(struct ir3_sched_ctx *ctx)
{
   ctx->dag = dag_create(ctx);

   foreach_instr (instr, &ctx->unscheduled_list)
      sched_node_init(ctx, instr);

   dag_validate(ctx->dag, sched_dag_validate_cb, NULL);

   foreach_instr (instr, &ctx->unscheduled_list)
      sched_node_add_deps(instr);

   dag_traverse_bottom_up(ctx->dag, sched_dag_max_delay_cb, NULL);
}

static void
sched_dag_destroy(struct ir3_sched_ctx *ctx)
{
   ralloc_free(ctx->dag);
   ctx->dag = NULL;
}

static void
sched_block(struct ir3_sched_ctx *ctx, struct ir3_block *block)
{
   ctx->block = block;

   /* addr/pred writes are per-block: */
   ctx->addr0 = NULL;
   ctx->addr1 = NULL;
   ctx->pred = NULL;
   ctx->sy_delay = 0;
   ctx->ss_delay = 0;
   ctx->sy_index = ctx->first_outstanding_sy_index = 0;
   ctx->ss_index = ctx->first_outstanding_ss_index = 0;

   /* move all instructions to the unscheduled list, and
    * empty the block's instruction list (to which we will
    * be inserting).
    */
   list_replace(&block->instr_list, &ctx->unscheduled_list);
   list_inithead(&block->instr_list);

   sched_dag_init(ctx);

   ctx->remaining_kills = 0;
   ctx->remaining_tex = 0;
   foreach_instr_safe (instr, &ctx->unscheduled_list) {
      if (is_kill_or_demote(instr))
         ctx->remaining_kills++;
      if (is_sy_producer(instr))
         ctx->remaining_tex++;
   }

   /* First schedule all meta:input and meta:phi instructions, followed by
    * tex-prefetch.  We want all of the instructions that load values into
    * registers before the shader starts to go before any other instructions.
    * But in particular we want inputs to come before prefetches.  This is
    * because a FS's bary_ij input may not actually be live in the shader,
    * but it should not be scheduled on top of any other input (but can be
    * overwritten by a tex prefetch)
    *
    * Note: Because the first block cannot have predecessors, meta:input and
    * meta:phi cannot exist in the same block.
    */
   foreach_instr_safe (instr, &ctx->unscheduled_list)
      if (instr->opc == OPC_META_INPUT || instr->opc == OPC_META_PHI)
         schedule(ctx, instr);

   foreach_instr_safe (instr, &ctx->unscheduled_list)
      if (instr->opc == OPC_META_TEX_PREFETCH)
         schedule(ctx, instr);

   foreach_instr_safe (instr, &ctx->unscheduled_list)
      if (instr->opc == OPC_PUSH_CONSTS_LOAD_MACRO)
         schedule(ctx, instr);

   while (!list_is_empty(&ctx->unscheduled_list)) {
      struct ir3_sched_notes notes = {0};
      struct ir3_instruction *instr;

      instr = choose_instr(ctx, &notes);
      if (instr) {
         unsigned delay = node_delay(ctx, instr->data);
         d("delay=%u", delay);

         assert(delay <= 6);

         schedule(ctx, instr);

         /* Since we've scheduled a "real" instruction, we can now
          * schedule any split instruction created by the scheduler again.
          */
         ctx->split = NULL;
      } else {
         struct ir3_instruction *new_instr = NULL;
         struct ir3 *ir = block->shader;

         /* nothing available to schedule.. if we are blocked on
          * address/predicate register conflict, then break the
          * deadlock by cloning the instruction that wrote that
          * reg:
          */
         if (notes.addr0_conflict) {
            new_instr =
               split_addr(ctx, &ctx->addr0, ir->a0_users, ir->a0_users_count);
         } else if (notes.addr1_conflict) {
            new_instr =
               split_addr(ctx, &ctx->addr1, ir->a1_users, ir->a1_users_count);
         } else if (notes.pred_conflict) {
            new_instr = split_pred(ctx);
         } else {
            d("unscheduled_list:");
            foreach_instr (instr, &ctx->unscheduled_list)
               di(instr, "unscheduled: ");
            assert(0);
            ctx->error = true;
            return;
         }

         if (new_instr) {
            list_delinit(&new_instr->node);
            list_addtail(&new_instr->node, &ctx->unscheduled_list);
         }

         /* If we produced a new instruction, do not schedule it next to
          * guarantee progress.
          */
         ctx->split = new_instr;
      }
   }

   sched_dag_destroy(ctx);
}

int
ir3_sched(struct ir3 *ir)
{
   struct ir3_sched_ctx *ctx = rzalloc(NULL, struct ir3_sched_ctx);

   foreach_block (block, &ir->block_list) {
      foreach_instr (instr, &block->instr_list) {
         instr->data = NULL;
      }
   }

   ir3_count_instructions(ir);
   ir3_clear_mark(ir);
   ir3_find_ssa_uses(ir, ctx, false);

   foreach_block (block, &ir->block_list) {
      sched_block(ctx, block);
   }

   int ret = ctx->error ? -1 : 0;

   ralloc_free(ctx);

   return ret;
}

static unsigned
get_array_id(struct ir3_instruction *instr)
{
   /* The expectation is that there is only a single array
    * src or dst, ir3_cp should enforce this.
    */

   foreach_dst (dst, instr)
      if (dst->flags & IR3_REG_ARRAY)
         return dst->array.id;
   foreach_src (src, instr)
      if (src->flags & IR3_REG_ARRAY)
         return src->array.id;

   unreachable("this was unexpected");
}

/* does instruction 'prior' need to be scheduled before 'instr'? */
static bool
depends_on(struct ir3_instruction *instr, struct ir3_instruction *prior)
{
   /* TODO for dependencies that are related to a specific object, ie
    * a specific SSBO/image/array, we could relax this constraint to
    * make accesses to unrelated objects not depend on each other (at
    * least as long as not declared coherent)
    */
   if (((instr->barrier_class & IR3_BARRIER_EVERYTHING) &&
        prior->barrier_class) ||
       ((prior->barrier_class & IR3_BARRIER_EVERYTHING) &&
        instr->barrier_class))
      return true;

   if (instr->barrier_class & prior->barrier_conflict) {
      if (!(instr->barrier_class &
            ~(IR3_BARRIER_ARRAY_R | IR3_BARRIER_ARRAY_W))) {
         /* if only array barrier, then we can further limit false-deps
          * by considering the array-id, ie reads/writes to different
          * arrays do not depend on each other (no aliasing)
          */
         if (get_array_id(instr) != get_array_id(prior)) {
            return false;
         }
      }

      return true;
   }

   return false;
}

static void
add_barrier_deps(struct ir3_block *block, struct ir3_instruction *instr)
{
   struct list_head *prev = instr->node.prev;
   struct list_head *next = instr->node.next;

   /* add dependencies on previous instructions that must be scheduled
    * prior to the current instruction
    */
   while (prev != &block->instr_list) {
      struct ir3_instruction *pi =
         list_entry(prev, struct ir3_instruction, node);

      prev = prev->prev;

      if (is_meta(pi))
         continue;

      if (instr->barrier_class == pi->barrier_class) {
         ir3_instr_add_dep(instr, pi);
         break;
      }

      if (depends_on(instr, pi))
         ir3_instr_add_dep(instr, pi);
   }

   /* add dependencies on this instruction to following instructions
    * that must be scheduled after the current instruction:
    */
   while (next != &block->instr_list) {
      struct ir3_instruction *ni =
         list_entry(next, struct ir3_instruction, node);

      next = next->next;

      if (is_meta(ni))
         continue;

      if (instr->barrier_class == ni->barrier_class) {
         ir3_instr_add_dep(ni, instr);
         break;
      }

      if (depends_on(ni, instr))
         ir3_instr_add_dep(ni, instr);
   }
}

/* before scheduling a block, we need to add any necessary false-dependencies
 * to ensure that:
 *
 *  (1) barriers are scheduled in the right order wrt instructions related
 *      to the barrier
 *
 *  (2) reads that come before a write actually get scheduled before the
 *      write
 */
bool
ir3_sched_add_deps(struct ir3 *ir)
{
   bool progress = false;

   foreach_block (block, &ir->block_list) {
      foreach_instr (instr, &block->instr_list) {
         if (instr->barrier_class) {
            add_barrier_deps(block, instr);
            progress = true;
         }
      }
   }

   return progress;
}
