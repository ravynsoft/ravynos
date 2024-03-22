/*
 * Copyright (c) 2017 Lima Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

#include <limits.h>

#include "gpir.h"

/*
 * GP scheduling algorithm (by Connor Abbott <cwabbott0@gmail.com>)
 * 
 * The GP pipeline has three main stages:
 *
 * --------------------------------------------------------
 * |                                                      |
 * |               Register/Attr/Temp Fetch               |
 * |                                                      |
 * --------------------------------------------------------
 * |        |        |        |        |        |         |
 * |  Mul0  |  Mul1  |  Add0  |  Add1  |  Cplx  |  Pass   |
 * |        |        |        |        |        |         |
 * --------------------------------------------------------
 * |                 |                          |         |
 * |    Complex1     |   Temp/Register/Varying  |  Pass   |
 * |    Stage 2      |         Store            | Stage 2 |
 * |                 |                          |         |
 * --------------------------------------------------------
 *
 * Because of this setup, storing a register has a latency of three cycles.
 * Also, the register file is organized into 4-component vectors, and the
 * load stage can only load two vectors at a time. Aside from these highly
 * constrained register load/store units, there is an explicit bypass
 * network, where each unit (mul0/mul1/etc.) can access the results of the
 * any unit from the previous two cycles directly, except for the complex
 * unit whose result can only be accessed for one cycle (since it's expected
 * to be used directly by the complex2 instruction in the following cycle).
 *
 * Because of the very restricted register file, and because only rarely are
 * all the units in use at the same time, it can be very beneficial to use
 * the unused units to "thread" a value from source to destination by using
 * moves in the otherwise-unused units, without involving the register file
 * at all. It's very difficult to fully exploit this with a traditional
 * scheduler, so we need to do something a little un-traditional. The 512
 * instruction limit means that for more complex shaders, we need to do as
 * well as possible or else the app won't even work.
 *
 * The scheduler works by considering the bypass network as a kind of
 * register file. It's a quite unusual register file, since registers have to
 * be assigned "on the fly" as we schedule operations, but with some care, we
 * can use something conceptually similar to a linear-scan allocator to
 * successfully schedule nodes to instructions without running into
 * conflicts.
 *
 * Values in the IR are separated into normal values, or "value registers",
 * which is what normal nodes like add, mul, etc. produce, and which only
 * live inside one basic block, and registers, which can span multiple basic
 * blocks but have to be accessed via special load_reg/store_reg nodes. RA
 * assigns physical registers to both value registers and normal registers,
 * treating load_reg/store_reg as a move instruction, but these are only used
 * directly for normal registers -- the physreg assigned to a value register
 * is "fake," and is only used inside the scheduler. Before scheduling we
 * insert read-after-write dependencies, even for value registers, as if
 * we're going to use those, but then we throw them away. For example, if we
 * had something like:
 *
 * (*)r2 = add (*)r1, (*)r2
 * (*)r1 = load_reg r0
 *
 * we'd insert a write-after-read dependency between the add and load_reg,
 * even though the starred registers aren't actually used by the scheduler
 * after this step. This step is crucial since it guarantees that during any
 * point in the schedule, the number of live registers + live value registers
 * will never exceed the capacity of the register file and the bypass network
 * combined. This is because each live register/value register will have a
 * different fake number, thanks to the fake dependencies inserted before
 * scheduling. This allows us to not have to worry about spilling to
 * temporaries, which is only done ahead of time.
 *
 * The scheduler is a bottom-up scheduler. It keeps track of each live value
 * register, and decides on-the-fly which value registers to keep in the
 * bypass network and which to "spill" to registers. Of particular importance
 * is the "ready list," which consists of "input nodes" (nodes that produce a
 * value that can be consumed via the bypass network), both "partially ready"
 * (only some of the uses have been scheduled) and "fully ready" (all uses
 * have been scheduled), as well as other non-input nodes like register
 * stores. Each input node on the ready list represents a live value register
 * before the current instruction. There must be at most 11 such input nodes
 * at all times, since there are only 11 slots in the next two instructions
 * which can reach the current instruction.
 *
 * An input node is a "max node" if it has a use two cycles ago, which must be
 * connected to a definition this cycle. Otherwise it may be a "next max node"
 * if it will be a max node on the next instruction (i.e. it has a use at most
 * one cycle ago), or it may be neither if all of its uses are this cycle. As
 * we keep adding instructions to the front, input nodes graduate from
 * neither, to next max, to max, unless we decide to insert a move to keep it
 * alive longer, at which point any uses after the current instruction are
 * rewritten to be uses of the move so that the original node returns to
 * neither. The scheduler decides which nodes to try freely, but we have to
 * reserve slots for two different reasons: (1) out of the 5 non-complex
 * slots, we reserve a slot for each max node, so that we can connect a
 * definition to the use 2 cycles ago. (2) Out of all 6 slots, we reserve a
 * slot for every next-max node above 5, so that for the next instruction
 * there are no more than 5 max nodes. When a max or next-max node gets
 * scheduled, the corresponding reservation is reduced by one. At the end, we
 * insert moves for every slot that was reserved. The reservation is actually
 * managed by nir_instr, and all we have to do is tell it how many to reserve
 * at the beginning and then tell it which nodes are max/next-max nodes. When
 * we start scheduling an instruction, there will be at most 5 max nodes
 * thanks to the previous instruction's next-max reservation/move insertion.
 * Since there are at most 11 total input nodes, if there are N max nodes,
 * there are at most 11 - N next-max nodes, and therefore at most 11 - N - 5 =
 * 6 - N slots need to be reserved for next-max nodes, and so at most
 * 6 - N + N = 6 slots need to be reserved in total, exactly the total number
 * of slots. So, thanks to the total input node restriction, we will never
 * need to reserve too many slots.
 *
 * It sometimes happens that scheduling a given node will violate this total
 * input node restriction, or that a reservation will mean that we can't
 * schedule it. We first schedule a node "speculatively" to see if this is a
 * problem. If some of the node's sources are loads, then we can schedule
 * the node and its dependent loads in one swoop to avoid going over the
 * pressure limit. If that fails, we can try to spill a ready or
 * partially-ready input node to a register by rewriting all of its uses to
 * refer to a register load. This removes it from the list of ready and
 * partially ready input nodes as all of its uses are now unscheduled. If
 * successful, we can then proceed with scheduling the original node. All of
 * this happens "speculatively," meaning that afterwards the node is removed
 * and the entire state of the scheduler is reverted to before it was tried, to
 * ensure that we never get into an invalid state and run out of spots for
 * moves. In try_nodes(), we try to schedule each node speculatively on the
 * ready list, keeping only the nodes that could be successfully scheduled, so
 * that when we finally decide which node to actually schedule, we know it
 * will succeed.  This is how we decide on the fly which values go in
 * registers and which go in the bypass network. Note that "unspilling" a
 * value is simply a matter of scheduling the store_reg instruction created
 * when we spill.
 *
 * The careful accounting of live value registers, reservations for moves, and
 * speculative scheduling guarantee that we never run into a failure case
 * while scheduling. However, we need to make sure that this scheduler will
 * not get stuck in an infinite loop, i.e. that we'll always make forward
 * progress by eventually scheduling a non-move node. If we run out of value
 * registers, then we may have to spill a node to a register. If we
 * were to schedule one of the fully-ready nodes, then we'd have 11 + N live
 * value registers before the current instruction. But since there are at most
 * 64+11 live registers and register values total thanks to the fake
 * dependencies we inserted before scheduling, there are at most 64 - N live
 * physical registers, and therefore there are at least N registers available
 * for spilling. Not all these registers will be available immediately, since
 * in order to spill a node to a given register we have to ensure that there
 * are slots available to rewrite every use to a load instruction, and that
 * may not be the case. There may also be intervening writes which prevent
 * some registers from being used. However, these are all temporary problems,
 * since as we create each instruction, we create additional register load
 * slots that can be freely used for spilling, and we create more move nodes
 * which means that the uses of the nodes we're trying to spill keep moving
 * forward. This means that eventually, these problems will go away, at which
 * point we'll be able to spill a node successfully, so eventually we'll be
 * able to schedule the first node on the ready list.
 */

typedef struct {
   /* This is the list of ready and partially-ready nodes. A partially-ready
    * node must have at least one input dependency already scheduled.
    */
   struct list_head ready_list;

   /* The number of ready or partially-ready nodes with at least one input
    * dependency already scheduled. In other words, the number of live value
    * registers. This must be at most 11.
    */
   int ready_list_slots;

   /* The physical registers live into the current instruction. */
   uint64_t live_physregs;

   /* The current instruction. */
   gpir_instr *instr;

   /* The current basic block. */
   gpir_block *block;

   /* True if at least one node failed to schedule due to lack of available
    * value registers.
    */
   bool try_spill_all;

   /* The number of max nodes needed to spill to successfully schedule the
    * instruction.
    */
   int max_node_spill_needed;

   /* The number of max and next-max nodes needed to spill to successfully
    * schedule the instruction.
    */
   int total_spill_needed;

   /* For each physical register, a linked list of loads associated with it in
    * this block. When we spill a value to a given register, and there are
    * existing loads associated with it that haven't been scheduled yet, we
    * have to make sure that the corresponding unspill happens after the last
    * original use has happened, i.e. is scheduled before.
    */
   struct list_head physreg_reads[GPIR_PHYSICAL_REG_NUM];
} sched_ctx;

static int gpir_min_dist_alu(gpir_dep *dep)
{
   switch (dep->pred->op) {
   case gpir_op_load_uniform:
   case gpir_op_load_temp:
   case gpir_op_load_reg:
   case gpir_op_load_attribute:
      return 0;

   case gpir_op_complex1:
      return 2;

   default:
      return 1;
   }
}

static int gpir_get_min_dist(gpir_dep *dep)
{
   switch (dep->type) {
   case GPIR_DEP_INPUT:
      switch (dep->succ->op) {
      case gpir_op_store_temp:
      case gpir_op_store_reg:
      case gpir_op_store_varying:
         /* Stores must use an alu node as input. Also, complex1 takes two
          * cycles, which means that its result cannot be stored to a register
          * as part of the normal path, and therefore it must also have a move
          * inserted.
          */
         if (dep->pred->type == gpir_node_type_load ||
             dep->pred->op == gpir_op_complex1)
            return INT_MAX >> 2;
         else
            return 0;

      default:
         return gpir_min_dist_alu(dep);
      }

   case GPIR_DEP_OFFSET:
      assert(dep->succ->op == gpir_op_store_temp);
      return gpir_min_dist_alu(dep);

   case GPIR_DEP_READ_AFTER_WRITE:
      if (dep->succ->op == gpir_op_load_temp &&
          dep->pred->op == gpir_op_store_temp) {
         return 4;
      } else if (dep->succ->op == gpir_op_load_reg &&
                 dep->pred->op == gpir_op_store_reg) {
         return 3;
      } else if ((dep->pred->op == gpir_op_store_temp_load_off0 ||
                  dep->pred->op == gpir_op_store_temp_load_off1 ||
                  dep->pred->op == gpir_op_store_temp_load_off2) &&
                 dep->succ->op == gpir_op_load_uniform) {
         return 4;
      } else {
         /* Fake dependency */
         return 0;
      }

   case GPIR_DEP_WRITE_AFTER_READ:
      return 0;
   }

   return 0;
}

static int gpir_max_dist_alu(gpir_dep *dep)
{
   switch (dep->pred->op) {
   case gpir_op_load_uniform:
   case gpir_op_load_temp:
      return 0;
   case gpir_op_load_attribute:
      return 1;
   case gpir_op_load_reg:
      if (dep->pred->sched.pos < GPIR_INSTR_SLOT_REG0_LOAD0 ||
          dep->pred->sched.pos > GPIR_INSTR_SLOT_REG0_LOAD3)
         return 0;
      else
         return 1;
   case gpir_op_exp2_impl:
   case gpir_op_log2_impl:
   case gpir_op_rcp_impl:
   case gpir_op_rsqrt_impl:
   case gpir_op_store_temp_load_off0:
   case gpir_op_store_temp_load_off1:
   case gpir_op_store_temp_load_off2:
      return 1;
   case gpir_op_mov:
      if (dep->pred->sched.pos == GPIR_INSTR_SLOT_COMPLEX)
         return 1;
      else
         return 2;
   default:
      return 2;
   }
}

static int gpir_get_max_dist(gpir_dep *dep)
{
   switch (dep->type) {
   case GPIR_DEP_INPUT:
      switch (dep->succ->op) {
      case gpir_op_store_temp:
      case gpir_op_store_reg:
      case gpir_op_store_varying:
         return 0;

      default:
         return gpir_max_dist_alu(dep);
      }

   case GPIR_DEP_OFFSET:
      assert(dep->succ->op == gpir_op_store_temp);
      return gpir_max_dist_alu(dep);

   default:
      return INT_MAX >> 2; /* Don't want to overflow... */
   }
}

static void schedule_update_distance(gpir_node *node)
{
   if (gpir_node_is_leaf(node)) {
      node->sched.dist = 0;
      return;
   }

   gpir_node_foreach_pred(node, dep) {
      gpir_node *pred = dep->pred;

      if (pred->sched.dist < 0)
         schedule_update_distance(pred);

      int dist = pred->sched.dist + gpir_min_dist_alu(dep);
      if (node->sched.dist < dist)
         node->sched.dist = dist;
   }
}

static bool gpir_is_input_node(gpir_node *node)
{
   gpir_node_foreach_succ(node, dep) {
      if (dep->type == GPIR_DEP_INPUT)
         return true;
   }
   return false;
}


/* Get the number of slots required for a node on the ready list.
 */
static int gpir_get_slots_required(gpir_node *node)
{
   if (!gpir_is_input_node(node))
      return 0;

   /* Note that we assume every node only consumes one slot, even dual-slot
    * instructions. While dual-slot instructions may consume more than one
    * slot, we can always safely insert a move if it turns out that there
    * isn't enough space for them. There's the risk that we get stuck in an
    * infinite loop if all the fully ready nodes are dual-slot nodes, but we
    * rely on spilling to registers to save us here.
    */
   return 1;
}

static void ASSERTED verify_ready_list(sched_ctx *ctx)
{
   list_for_each_entry(gpir_node, node, &ctx->ready_list, list) {
      if (!gpir_is_input_node(node)) {
         assert(node->sched.ready);
      }

      if (node->sched.ready) {
         /* Every successor must have been scheduled */
         gpir_node_foreach_succ(node, dep) {
            assert(dep->succ->sched.instr);
         }
      } else {
         /* There must be at least one successor that's not scheduled. */
         bool unscheduled = false;
         gpir_node_foreach_succ(node, dep) {
            unscheduled |= !(dep->succ->sched.instr);
         }

         assert(unscheduled);
      }
   }
}

static void schedule_insert_ready_list(sched_ctx *ctx,
                                       gpir_node *insert_node)
{
   /* if this node is fully ready or partially ready
    *   fully ready: all successors have been scheduled
    *   partially ready: part of input successors have been scheduled
    *
    * either fully ready or partially ready node need be inserted to
    * the ready list, but we only schedule a move node for partially
    * ready node.
    */
   bool ready = true, insert = false;
   gpir_node_foreach_succ(insert_node, dep) {
      gpir_node *succ = dep->succ;
      if (succ->sched.instr) {
         if (dep->type == GPIR_DEP_INPUT)
            insert = true;
      }
      else
         ready = false;
   }

   insert_node->sched.ready = ready;
   /* for root node */
   insert |= ready;

   if (!insert || insert_node->sched.inserted)
      return;

   struct list_head *insert_pos = &ctx->ready_list;
   list_for_each_entry(gpir_node, node, &ctx->ready_list, list) {
      if ((insert_node->sched.dist > node->sched.dist ||
          gpir_op_infos[insert_node->op].schedule_first) &&
          !gpir_op_infos[node->op].schedule_first) {
         insert_pos = &node->list;
         break;
      }
   }

   list_addtail(&insert_node->list, insert_pos);
   insert_node->sched.inserted = true;
   ctx->ready_list_slots += gpir_get_slots_required(insert_node);
}

static int gpir_get_max_start(gpir_node *node)
{
   int max_start = 0;

   /* find the max start instr constrainted by all successors */
   gpir_node_foreach_succ(node, dep) {
      gpir_node *succ = dep->succ;
      if (!succ->sched.instr)
         continue;

      int start = succ->sched.instr->index + gpir_get_min_dist(dep);
      if (start > max_start)
         max_start = start;
   }

   return max_start;
}

static int gpir_get_min_end(gpir_node *node)
{
   int min_end = INT_MAX;

   /* find the min end instr constrainted by all successors */
   gpir_node_foreach_succ(node, dep) {
      gpir_node *succ = dep->succ;
      if (!succ->sched.instr)
         continue;

      int end = succ->sched.instr->index + gpir_get_max_dist(dep);
      if (end < min_end)
         min_end = end;
   }

   return min_end;
}

static gpir_node *gpir_sched_instr_has_load(gpir_instr *instr, gpir_node *node)
{
   gpir_load_node *load = gpir_node_to_load(node);

   for (int i = GPIR_INSTR_SLOT_REG0_LOAD0; i <= GPIR_INSTR_SLOT_MEM_LOAD3; i++) {
      if (!instr->slots[i])
         continue;

      gpir_load_node *iload = gpir_node_to_load(instr->slots[i]);
      if (load->node.op == iload->node.op &&
          load->index == iload->index &&
          load->component == iload->component)
         return &iload->node;
   }
   return NULL;
}

/* Simply place the node into the given instruction without trying to deal
 * with liveness or the ready list. This will only fail if the instruction
 * cannot be placed due to a lack of available slots. In addition to normal
 * node placement, this is also used for placing loads when spilling to
 * registers.
 */
static bool _try_place_node(sched_ctx *ctx, gpir_instr *instr, gpir_node *node)
{
   if (node->type == gpir_node_type_load) {
      gpir_node *load = gpir_sched_instr_has_load(instr, node);
      if (load) {
         /* This node may have a store as a successor, in which case we have to
          * fail it exactly like below in order to later create a move node in
          * between.
          */
         if (instr->index < gpir_get_max_start(node))
            return false;

         gpir_debug("same load %d in instr %d for node %d\n",
                    load->index, instr->index, node->index);

         /* not really merge two node, just fake scheduled same place */
         node->sched.instr = load->sched.instr;
         node->sched.pos = load->sched.pos;
         return true;
      }
   }

   if (node->op == gpir_op_store_reg) {
      /* This register may be loaded in the next basic block, in which case
       * there still needs to be a 2 instruction gap. We do what the blob
       * seems to do and simply disable stores in the last two instructions of
       * the basic block.
       *
       * TODO: We may be able to do better than this, but we have to check
       * first if storing a register works across branches.
       */
      if (instr->index < 2)
         return false;
   }

   node->sched.instr = instr;

   int max_node_spill_needed = INT_MAX;
   int total_spill_needed = INT_MAX;
   int *slots = gpir_op_infos[node->op].slots;
   for (int i = 0; slots[i] != GPIR_INSTR_SLOT_END; i++) {
      node->sched.pos = slots[i];
      if (instr->index >= gpir_get_max_start(node) &&
          instr->index <= gpir_get_min_end(node) &&
          gpir_instr_try_insert_node(instr, node))
         return true;
      if (ctx->instr->non_cplx_slot_difference ||
          ctx->instr->slot_difference) {
         /* If one of these fields is non-zero, then we could insert the node
          * here after spilling. To get an accurate count of how many nodes we
          * need to spill, we need to choose one of the positions where there
          * were nonzero slot differences, preferably one with the smallest
          * difference (so we don't have to spill as much).
          */
         if (ctx->instr->non_cplx_slot_difference < max_node_spill_needed ||
             ctx->instr->slot_difference < total_spill_needed) {
            max_node_spill_needed = ctx->instr->non_cplx_slot_difference;
            total_spill_needed = ctx->instr->slot_difference;
         }
      }
   }

   if (max_node_spill_needed != INT_MAX) {
      /* Indicate how many spill nodes are needed. */
      ctx->max_node_spill_needed = MAX2(ctx->max_node_spill_needed,
                                        max_node_spill_needed);
      ctx->total_spill_needed = MAX2(ctx->total_spill_needed,
                                     total_spill_needed);
   }
   node->sched.instr = NULL;
   node->sched.pos = -1;
   return false;
}

/* Try to place just the node given, updating the ready list. If "speculative"
 * is true, then this is part of the pre-commit phase. If false, then we have
 * committed to placing this node, so update liveness and ready list
 * information.
 */

static bool schedule_try_place_node(sched_ctx *ctx, gpir_node *node,
                                    bool speculative)
{
   if (!_try_place_node(ctx, ctx->instr, node)) {
      if (!speculative)
         gpir_debug("failed to place %d\n", node->index);
      return false;
   }

   ctx->ready_list_slots -= gpir_get_slots_required(node);

   if (!speculative) {
      gpir_debug("placed node %d\n", node->index);

      /* We assume here that writes are placed before reads. If this changes,
       * then this needs to be updated.
       */
      if (node->op == gpir_op_store_reg) {
         gpir_store_node *store = gpir_node_to_store(node);
         ctx->live_physregs &=
            ~(1ull << (4 * store->index + store->component));
         if (store->child->sched.physreg_store == store)
            store->child->sched.physreg_store = NULL;
      }

      if (node->op == gpir_op_load_reg) {
         gpir_load_node *load = gpir_node_to_load(node);
         ctx->live_physregs |= 
            (1ull << (4 * load->index + load->component));
      }

      list_del(&node->list);
      list_add(&node->list, &ctx->block->node_list);
      gpir_node_foreach_pred(node, dep) {
         gpir_node *pred = dep->pred;
         schedule_insert_ready_list(ctx, pred);
      }
   } else {
      gpir_node_foreach_pred(node, dep) {
         gpir_node *pred = dep->pred;
         if (!pred->sched.inserted && dep->type == GPIR_DEP_INPUT)
            ctx->ready_list_slots += gpir_get_slots_required(pred);
      }
   }

   return true;
}

/* Create a new node with "node" as the child, replace all uses of "node" with
 * this new node, and replace "node" with it in the ready list.
 */
static gpir_node *create_replacement(sched_ctx *ctx, gpir_node *node,
                                     gpir_op op)
{

   gpir_alu_node *new_node = gpir_node_create(node->block, op);
   if (unlikely(!new_node))
      return NULL;

   new_node->children[0] = node;
   new_node->num_child = 1;

   new_node->node.sched.instr = NULL;
   new_node->node.sched.pos = -1;
   new_node->node.sched.dist = node->sched.dist;
   new_node->node.sched.max_node = node->sched.max_node;
   new_node->node.sched.next_max_node = node->sched.next_max_node;
   new_node->node.sched.complex_allowed = node->sched.complex_allowed;

   ctx->ready_list_slots--;
   list_del(&node->list);
   node->sched.max_node = false;
   node->sched.next_max_node = false;
   node->sched.ready = false;
   node->sched.inserted = false;
   gpir_node_replace_succ(&new_node->node, node);
   gpir_node_add_dep(&new_node->node, node, GPIR_DEP_INPUT);
   schedule_insert_ready_list(ctx, &new_node->node);
   return &new_node->node;
}

static gpir_node *create_move(sched_ctx *ctx, gpir_node *node)
{
   gpir_node *move = create_replacement(ctx, node, gpir_op_mov);
   gpir_debug("create move %d for %d\n", move->index, node->index);
   return move;
}

static gpir_node *create_postlog2(sched_ctx *ctx, gpir_node *node)
{
   assert(node->op == gpir_op_complex1);
   gpir_node *postlog2 = create_replacement(ctx, node, gpir_op_postlog2);
   gpir_debug("create postlog2 %d for %d\n", postlog2->index, node->index);
   return postlog2;
}

/* Once we schedule the successor, would the predecessor be fully ready? */
static bool pred_almost_ready(gpir_dep *dep)
{
   bool fully_ready = true;
   gpir_node_foreach_succ(dep->pred, other_dep) {
      gpir_node *succ = other_dep->succ;
      if (!succ->sched.instr && dep->succ != other_dep->succ) {
         fully_ready = false;
         break;
      }
   }

   return fully_ready;
}

/* Recursively try to schedule a node and all its dependent nodes that can fit
 * in the same  instruction. There is a simple heuristic scoring system to try
 * to group together nodes that load different components of the same input,
 * to avoid bottlenecking for operations like matrix multiplies that are
 * mostly input-bound.
 */

static int _schedule_try_node(sched_ctx *ctx, gpir_node *node, bool speculative)
{
   if (!schedule_try_place_node(ctx, node, speculative))
      return INT_MIN;

   int score = 0;

   gpir_node_foreach_pred(node, dep) {
      if (dep->type != GPIR_DEP_INPUT)
         continue;

      int pred_score = INT_MIN;
      if (pred_almost_ready(dep)) {
         if (dep->pred->type == gpir_node_type_load || 
             node->type == gpir_node_type_store) {
            pred_score = _schedule_try_node(ctx, dep->pred, speculative);
         }
      }
      if (dep->pred->type == gpir_node_type_load || 
          node->type == gpir_node_type_store) {
         if (pred_score == INT_MIN) {
            if (node->op == gpir_op_mov) {
               /* The only moves on the ready list are for loads that we
                * couldn't schedule immediately, as created below. If we
                * couldn't schedule the load, there's no point scheduling
                * the move. The normal move threading logic will ensure
                * that another move is created if we're about to go too far
                * from the uses of this move.
                */
               assert(speculative);
               return INT_MIN;
            } else if (!speculative && dep->pred->type == gpir_node_type_load) {
               /* We couldn't schedule the load right away, so it will have
                * to happen in some earlier instruction and then be moved
                * into a value register and threaded to the use by "node".
                * We create the move right away, so that later we'll fail
                * to schedule it if there isn't a slot for a move
                * available.
                */
               create_move(ctx, dep->pred);
            }
            /* Penalize nodes whose dependent ops we couldn't schedule.
             */
            score--;
         } else {
            score += pred_score;
            continue;
         }
      }
   }

   return score;
}

/* If we speculatively tried a node, undo everything.
 */

static void schedule_undo_node(sched_ctx *ctx, gpir_node *node)
{
   gpir_instr_remove_node(ctx->instr, node);

   gpir_node_foreach_pred(node, dep) {
      gpir_node *pred = dep->pred;
      if (pred->sched.instr) {
         schedule_undo_node(ctx, pred);
      }
   }
}

/* Try to schedule a node. We also try to schedule any predecessors that can
 * be part of the same instruction. If "speculative" is true, then we don't
 * actually change any state, only returning the score were the node to be
 * scheduled, with INT_MIN meaning "cannot be scheduled at all".
 */
static int schedule_try_node(sched_ctx *ctx, gpir_node *node, bool speculative)
{
   int prev_slots = ctx->ready_list_slots;

   int score = _schedule_try_node(ctx, node, speculative);

   if (ctx->ready_list_slots > GPIR_VALUE_REG_NUM) {
      assert(speculative);
      ctx->total_spill_needed = MAX2(ctx->total_spill_needed,
                                     ctx->ready_list_slots - GPIR_VALUE_REG_NUM);
      score = INT_MIN;
   }

   if (speculative) {
      ctx->ready_list_slots = prev_slots;
      if (node->sched.instr)
         schedule_undo_node(ctx, node);
   }

   return score;
}

/* This is called when we want to spill "node" by inserting loads at its uses.
 * It returns all the possible registers we can use so that all the loads will
 * successfully be inserted. Also return the first instruction we'll need to
 * insert a load for.
 */

static uint64_t get_available_regs(sched_ctx *ctx, gpir_node *node,
                                   int *min_index)
{
   uint64_t available = ~0ull;
   gpir_node_foreach_succ(node, dep) {
      if (dep->type != GPIR_DEP_INPUT)
         continue;

      gpir_node *use = dep->succ;
      gpir_instr *instr = use->sched.instr;

      if (!instr) {
         /* This use isn't scheduled, so no need to spill it. */
         continue;
      }

      if (use->type == gpir_node_type_store) {
         /* We're trying to spill something that was recently stored... just
          * bail out.
          */
         return 0;
      }

      if (use->op == gpir_op_mov && instr == ctx->instr) {
         /* We try to spill the sources of this move, so we can free up space
          * in the current instruction.
          *
          * TODO: should we go back further? It might let us schedule the
          * write earlier in some cases, but then we might fail to spill.
          */
         available &= get_available_regs(ctx, use, min_index);
      } else {
         if (instr->index < *min_index)
            *min_index = instr->index;

         uint64_t use_available = 0;

         if (instr->reg0_use_count == 0)
            use_available = ~0ull;
         else if (!instr->reg0_is_attr)
            use_available = 0xfull << (4 * instr->reg0_index);

         if (instr->reg1_use_count == 0)
            use_available = ~0ull;
         else
            use_available |= 0xfull << (4 * instr->reg1_index);

         available &= use_available;
      }
   }

   return available;
}

/* Using "min_index" returned by get_available_regs(), figure out which
 * registers are killed by a write after or during the current instruction and
 * hence we can't use for spilling. Writes that haven't been scheduled yet
 * should be reflected in live_physregs.
 */

static uint64_t get_killed_regs(sched_ctx *ctx, int min_index)
{
   uint64_t killed = 0;

   list_for_each_entry(gpir_instr, instr, &ctx->block->instr_list, list) {
      if (instr->index <= min_index)
         break;

      for (int slot = GPIR_INSTR_SLOT_STORE0; slot <= GPIR_INSTR_SLOT_STORE3; 
           slot++) {
         if (!instr->slots[slot])
            continue;

         gpir_store_node *store = gpir_node_to_store(instr->slots[slot]);
         if (store->node.op != gpir_op_store_reg)
            continue;

         killed |= 1ull << (4 * store->index + store->component);
      }
   }

   return killed;
}

/* Actually spill a node so that it is no longer in the ready list. Note that
 * this must exactly follow the logic of get_available_regs() or else the
 * loads could fail to schedule.
 */

static void spill_node(sched_ctx *ctx, gpir_node *node, gpir_store_node *store)
{
   gpir_node_foreach_succ_safe(node, dep) {
      if (dep->type != GPIR_DEP_INPUT)
         continue;

      gpir_node *use = dep->succ;
      gpir_instr *instr = use->sched.instr;

      if (!instr)
         continue;

      if (use->op == gpir_op_mov && instr == ctx->instr) {
         spill_node(ctx, use, store);
      } else {
         gpir_load_node *load = gpir_node_create(ctx->block, gpir_op_load_reg);
         load->index = store->index;
         load->component = store->component;
         list_add(&load->node.list, &ctx->block->node_list);
         gpir_node_replace_child(dep->succ, dep->pred, &load->node);
         gpir_node_replace_pred(dep, &load->node);
         gpir_node_add_dep(&load->node, &store->node, GPIR_DEP_READ_AFTER_WRITE);
         gpir_debug("spilling use %d of node %d to load node %d\n",
                    use->index, node->index, load->node.index);
         ASSERTED bool result = _try_place_node(ctx, use->sched.instr, &load->node);
         assert(result);
      }
   }

   if (node->op == gpir_op_mov) {
      /* We replaced all the uses of the move, so it's dead now. */
      gpir_instr_remove_node(node->sched.instr, node);
      gpir_node_delete(node);
   } else {
      /* We deleted all the uses of the node except the store, so it's not
       * live anymore.
       */
      list_del(&node->list);
      node->sched.inserted = false;
      ctx->ready_list_slots--;
      if (node->sched.max_node) {
         node->sched.max_node = false;
         ctx->instr->alu_num_slot_needed_by_max--;
      }
      if (node->sched.next_max_node) {
         node->sched.next_max_node = false;
         ctx->instr->alu_num_unscheduled_next_max--;
      }
   }
}

static bool used_by_store(gpir_node *node, gpir_instr *instr)
{
   gpir_node_foreach_succ(node, dep) {
      if (dep->type != GPIR_DEP_INPUT)
         continue;

      if (dep->succ->type == gpir_node_type_store &&
          dep->succ->sched.instr == instr)
         return true;
   }

   return false;
}

static gpir_node *consuming_postlog2(gpir_node *node)
{
   if (node->op != gpir_op_complex1)
      return NULL;

   gpir_node_foreach_succ(node, dep) {
      if (dep->type != GPIR_DEP_INPUT)
         continue;
      if (dep->succ->op == gpir_op_postlog2)
         return dep->succ;
      else
         return NULL;
   }

   return NULL;
}

static bool try_spill_node(sched_ctx *ctx, gpir_node *node)
{
   assert(node->op != gpir_op_mov);

   if (used_by_store(node, ctx->instr))
      return false;

   gpir_debug("trying to spill %d\n", node->index);

   int min_instr = INT_MAX;
   uint64_t available = get_available_regs(ctx, node, &min_instr);
   available &= ~get_killed_regs(ctx, min_instr);

   if (node->sched.physreg_store) {
      gpir_store_node *store = node->sched.physreg_store;
      if (!(available & (1ull << (4 * store->index + store->component))))
         return false;
   } else {
      available &= ~ctx->live_physregs;

      if (available == 0)
         return false;

      /* Don't spill complex1 if it's used postlog2, turn the postlog2 into a
       * move, replace the complex1 with postlog2 and spill that instead. The
       * store needs a move anyways so the postlog2 is usually free.
       */
      gpir_node *postlog2 = consuming_postlog2(node);
      if (postlog2) {
         postlog2->op = gpir_op_mov;
         node = create_postlog2(ctx, node);
      }

      /* TODO: use a better heuristic for choosing an available register? */
      int physreg = ffsll(available) - 1;

      ctx->live_physregs |= (1ull << physreg);

      gpir_store_node *store = gpir_node_create(ctx->block, gpir_op_store_reg);
      store->index = physreg / 4;
      store->component = physreg % 4;
      store->child = node;
      store->node.sched.max_node = false;
      store->node.sched.next_max_node = false;
      store->node.sched.complex_allowed = false;
      store->node.sched.pos = -1;
      store->node.sched.instr = NULL;
      store->node.sched.inserted = false;
      store->node.sched.dist = node->sched.dist;
      if (node->op == gpir_op_complex1) {
         /* Complex1 cannot be directly stored, and has a latency of 2 */
         store->node.sched.dist += 2;
      }
      node->sched.physreg_store = store;
      gpir_node_add_dep(&store->node, node, GPIR_DEP_INPUT);

      list_for_each_entry(gpir_load_node, load,
                          &ctx->physreg_reads[physreg], reg_link) {
         gpir_node_add_dep(&store->node, &load->node, GPIR_DEP_WRITE_AFTER_READ);
         if (load->node.sched.ready) {
            list_del(&load->node.list);
            load->node.sched.ready = false;
         }
      }

      node->sched.ready = false;
      schedule_insert_ready_list(ctx, &store->node);
   }

   gpir_debug("spilling %d to $%d.%c, store %d\n", node->index,
              node->sched.physreg_store->index,
              "xyzw"[node->sched.physreg_store->component],
              node->sched.physreg_store->node.index);

   spill_node(ctx, node, node->sched.physreg_store);

   return true;
}

static bool try_spill_nodes(sched_ctx *ctx, gpir_node *orig_node)
{
   /* First, try to spill max nodes. */
   list_for_each_entry_safe_rev(gpir_node, node, &ctx->ready_list, list) {
      if (ctx->max_node_spill_needed <= 0)
         break;

      /* orig_node is the node we're trying to schedule, so spilling it makes
       * no sense. Also don't try to spill any nodes in front of it, since
       * they might be scheduled instead.
       */
      if (node == orig_node)
         break;

      if (node->op == gpir_op_mov) {
         /* Don't try to spill loads, since that only adds another load and
          * store which is likely pointless.
          */
         continue;
      }

      if (!gpir_is_input_node(node) || !node->sched.max_node)
         continue;

      if (try_spill_node(ctx, node)) {
         ctx->max_node_spill_needed--;
         ctx->total_spill_needed--;
      }
   }

   /* Now, try to spill the remaining nodes. */
   list_for_each_entry_safe_rev(gpir_node, node, &ctx->ready_list, list) {
      if (ctx->total_spill_needed <= 0)
         break;

      if (node == orig_node)
         break;

      if (node->op == gpir_op_mov)
         continue;

      if (!gpir_is_input_node(node) ||
          !(node->sched.max_node || node->sched.next_max_node))
         continue;

      if (try_spill_node(ctx, node))
         ctx->total_spill_needed--;
   }

   return ctx->total_spill_needed <= 0 && ctx->max_node_spill_needed <= 0;
}

static int ASSERTED gpir_get_curr_ready_list_slots(sched_ctx *ctx)
{
   int total = 0;
   list_for_each_entry(gpir_node, node, &ctx->ready_list, list) {
      total += gpir_get_slots_required(node);
   }

   return total;
}

/* What gpir_get_min_end() would return if node were replaced with a move
 * instruction not in the complex slot. Normally this is 2 + min_end, except
 * for some store instructions which must have the move node in the same
 * instruction.
 */
static int gpir_get_min_end_as_move(gpir_node *node)
{
   int min = INT_MAX;
   gpir_node_foreach_succ(node, dep) {
      gpir_node *succ = dep->succ;
      if (succ->sched.instr && dep->type == GPIR_DEP_INPUT) {
         switch (succ->op) {
         case gpir_op_store_temp:
         case gpir_op_store_reg:
         case gpir_op_store_varying:
            continue;
         default:
            break;
         }
         if (min > succ->sched.instr->index + 2)
            min = succ->sched.instr->index + 2;
      }
   }
   return min;
}

/* The second source for add0, add1, mul0, and mul1 units cannot be complex.
 * The hardware overwrites the add second sources with 0 and mul second
 * sources with 1. This can be a problem if we need to insert more next-max
 * moves but we only have values that can't use the complex unit for moves.
 *
 * Fortunately, we only need to insert a next-max move if there are more than
 * 5 next-max nodes, but there are only 4 sources in the previous instruction
 * that make values not complex-capable, which means there can be at most 4
 * non-complex-capable values. Hence there will always be at least two values
 * that can be rewritten to use a move in the complex slot. However, we have
 * to be careful not to waste those values by putting both of them in a
 * non-complex slot. This is handled for us by gpir_instr, which will reject
 * such instructions. We just need to tell it which nodes can use complex, and
 * it will do the accounting to figure out what is safe.
 */

static bool can_use_complex(gpir_node *node)
{
   gpir_node_foreach_succ(node, dep) {
      if (dep->type != GPIR_DEP_INPUT)
         continue;

      gpir_node *succ = dep->succ;
      if (succ->type != gpir_node_type_alu ||
          !succ->sched.instr)
         continue;

      /* Note: this must be consistent with gpir_codegen_{mul,add}_slot{0,1}
       */
      gpir_alu_node *alu = gpir_node_to_alu(succ);
      switch (alu->node.op) {
      case gpir_op_complex1:
         /* complex1 puts its third source in the fourth slot */
         if (alu->children[1] == node || alu->children[2] == node)
            return false;
         break;
      case gpir_op_complex2:
         /* complex2 has its source duplicated, since it actually takes two
          * sources but we only ever use it with both sources the same. Hence
          * its source can never be the complex slot.
          */
         return false;
      case gpir_op_select:
         /* Select has its sources rearranged */
         if (alu->children[0] == node)
            return false;
         break;
      default:
         assert(alu->num_child <= 2);
         if (alu->num_child == 2 && alu->children[1] == node)
            return false;
         break;
      }
   }

   return true;
}

/* Initialize node->sched.max_node and node->sched.next_max_node for every
 * input node on the ready list. We should only need to do this once per
 * instruction, at the beginning, since we never add max nodes to the ready
 * list.
 */

static void sched_find_max_nodes(sched_ctx *ctx)
{
   ctx->instr->alu_num_unscheduled_next_max = 0;
   ctx->instr->alu_num_slot_needed_by_max = 0;

   list_for_each_entry(gpir_node, node, &ctx->ready_list, list) {
      if (!gpir_is_input_node(node))
         continue;

      int min_end_move = gpir_get_min_end_as_move(node);
      node->sched.max_node = (min_end_move == ctx->instr->index);
      node->sched.next_max_node = (min_end_move == ctx->instr->index + 1);
      if (node->sched.next_max_node)
         node->sched.complex_allowed = can_use_complex(node);

      if (node->sched.max_node)
         ctx->instr->alu_num_slot_needed_by_max++;
      if (node->sched.next_max_node)
         ctx->instr->alu_num_unscheduled_next_max++;
   }
}

/* Verify the invariants described in gpir.h, as well as making sure the
 * counts are correct.
 */
static void ASSERTED verify_max_nodes(sched_ctx *ctx)
{
   int alu_num_slot_needed_by_max = 0;
   int alu_num_unscheduled_next_max = 0;
   int alu_num_slot_needed_by_store = 0;
   int alu_num_slot_needed_by_non_cplx_store = 0;
   ASSERTED int alu_max_allowed_next_max = 5;

   list_for_each_entry(gpir_node, node, &ctx->ready_list, list) {
      if (!gpir_is_input_node(node))
         continue;

      if (node->sched.max_node)
         alu_num_slot_needed_by_max++;
      if (node->sched.next_max_node)
         alu_num_unscheduled_next_max++;
      if (used_by_store(node, ctx->instr)) {
         alu_num_slot_needed_by_store++;
         if (node->sched.next_max_node && !node->sched.complex_allowed)
            alu_num_slot_needed_by_non_cplx_store++;
      }
   }

   if (ctx->instr->slots[GPIR_INSTR_SLOT_MUL0] &&
       ctx->instr->slots[GPIR_INSTR_SLOT_MUL0]->op == gpir_op_complex1)
      alu_max_allowed_next_max = 4;

   assert(ctx->instr->alu_num_slot_needed_by_max == alu_num_slot_needed_by_max);
   assert(ctx->instr->alu_num_unscheduled_next_max == alu_num_unscheduled_next_max);
   assert(ctx->instr->alu_max_allowed_next_max == alu_max_allowed_next_max);
   assert(ctx->instr->alu_num_slot_needed_by_store == alu_num_slot_needed_by_store);
   assert(ctx->instr->alu_num_slot_needed_by_non_cplx_store ==
          alu_num_slot_needed_by_non_cplx_store);
   assert(ctx->instr->alu_num_slot_free >= alu_num_slot_needed_by_store + alu_num_slot_needed_by_max + MAX2(alu_num_unscheduled_next_max - alu_max_allowed_next_max, 0));
   assert(ctx->instr->alu_non_cplx_slot_free >= alu_num_slot_needed_by_max + alu_num_slot_needed_by_non_cplx_store);
}

static bool try_node(sched_ctx *ctx)
{
   gpir_node *best_node = NULL;
   int best_score = INT_MIN;

   /* Spilling will delete arbitrary nodes after the current one in the ready
    * list, which means that we always need to look up the next node in the
    * list at the end of each iteration. While list_for_each_entry() works for
    * this purpose, its sanity checking assumes that you don't want to modify
    * the list at all. We know better here, so we have to open-code
    * list_for_each_entry() without the check in order to not assert.
    */
   for (gpir_node *node = list_entry(ctx->ready_list.next, gpir_node, list);
        &node->list != &ctx->ready_list;
        node = list_entry(node->list.next, gpir_node, list)) {
      if (best_score != INT_MIN) {
         if (node->sched.dist < best_node->sched.dist)
            break;
      }

      if (node->sched.ready) {
         ctx->total_spill_needed = 0;
         ctx->max_node_spill_needed = 0;
         int score = schedule_try_node(ctx, node, true);
         if (score == INT_MIN && !best_node &&
             ctx->total_spill_needed > 0 &&
             try_spill_nodes(ctx, node)) {
            score = schedule_try_node(ctx, node, true);
         }

         /* schedule_first nodes must be scheduled if possible */
         if (gpir_op_infos[node->op].schedule_first && score != INT_MIN) {
            best_node = node;
            best_score = score;
            break;
         }

         if (score > best_score) {
            best_score = score;
            best_node = node;
         }
      }
   }

   if (best_node) {
      gpir_debug("scheduling %d (score = %d)%s\n", best_node->index,
                 best_score, best_node->sched.max_node ? " (max)" : "");
      ASSERTED int score = schedule_try_node(ctx, best_node, false);
      assert(score != INT_MIN);
      return true;
   }

   return false;
}

static void place_move(sched_ctx *ctx, gpir_node *node)
{
   /* For complex1 that is consumed by a postlog2, we cannot allow any moves
    * in between. Convert the postlog2 to a move and insert a new postlog2,
    * and try to schedule it again in try_node().
    */
   gpir_node *postlog2 = consuming_postlog2(node);
   if (postlog2) {
      postlog2->op = gpir_op_mov;
      create_postlog2(ctx, node);
      return;
   }

   gpir_node *move = create_move(ctx, node);
   gpir_node_foreach_succ_safe(move, dep) {
      gpir_node *succ = dep->succ;
      if (!succ->sched.instr ||
          ctx->instr->index < succ->sched.instr->index + gpir_get_min_dist(dep)) {
         gpir_node_replace_pred(dep, node);
         if (dep->type == GPIR_DEP_INPUT)
            gpir_node_replace_child(succ, move, node);
      }
   }
   ASSERTED int score = schedule_try_node(ctx, move, false);
   assert(score != INT_MIN);
}

/* For next-max nodes, not every node can be offloaded to a move in the
 * complex slot. If we run out of non-complex slots, then such nodes cannot
 * have moves placed for them. There should always be sufficient
 * complex-capable nodes so that this isn't a problem. We also disallow moves
 * for schedule_first nodes here.
 */
static bool can_place_move(sched_ctx *ctx, gpir_node *node)
{
   if (gpir_op_infos[node->op].schedule_first)
      return false;

   if (!node->sched.next_max_node)
      return true;

   if (node->sched.complex_allowed)
      return true;

   return ctx->instr->alu_non_cplx_slot_free > 0;
}

static bool sched_move(sched_ctx *ctx)
{
   list_for_each_entry(gpir_node, node, &ctx->ready_list, list) {
      if (node->sched.max_node) {
         place_move(ctx, node);
         return true;
      }
   }

   if (ctx->instr->alu_num_slot_needed_by_store > 0) {
      list_for_each_entry(gpir_node, node, &ctx->ready_list, list) {
         if (used_by_store(node, ctx->instr)) {
            place_move(ctx, node);
            /* If we have a store of a load, then we need to make sure that we
             * immediately schedule the dependent load, or create a move
             * instruction for it, like we would with a normal instruction.
             * The rest of the code isn't set up to handle load nodes in the
             * ready list -- see the comments in _schedule_try_node().
             */
            if (node->type == gpir_node_type_load) {
               if (!schedule_try_place_node(ctx, node, false)) {
                  create_move(ctx, node);
               }
            }
            return true;
         }
      }
   }

   /* complex1 is a bit a special case, since it has a latency of 2 cycles.
    * Once it is fully ready, we need to group all its uses in the same
    * instruction, and then we need to avoid creating any moves in the next
    * cycle in order to get it scheduled. Failing to do any of these things
    * could result in a cycle penalty, or even worse, an infinite loop of
    * inserting moves. If it is a next-max node and ready, then it has a use
    * in the previous cycle. If it has a use in the current cycle as well,
    * then we want to insert a move node to make it ready in two cycles -- if
    * we don't, then there will be at least a one cycle penalty. Otherwise, it
    * will be ready next cycle, and we shouldn't insert a move node, or else
    * we'll also have a one cycle penalty. 
    */
   if (ctx->instr->alu_num_slot_free > 0) {
      list_for_each_entry(gpir_node, node, &ctx->ready_list, list) {
         if (!can_place_move(ctx, node))
            continue;

         if (node->sched.next_max_node && node->op == gpir_op_complex1 &&
             node->sched.ready) {
            bool skip = true;
            gpir_node_foreach_succ(node, dep) {
               if (dep->type != GPIR_DEP_INPUT)
                  continue;

               gpir_node *succ = dep->succ;

               if (!succ->sched.instr ||
                   succ->sched.instr->index != ctx->instr->index - 1) {
                  skip = false;
                  break;
               }
            }

            if (skip)
               continue;

            place_move(ctx, node);
            return true;
         }
      }
   }

   /* Once we've made all the required moves, we're free to use any extra
    * slots to schedule more moves for next max nodes. Besides sometimes being
    * necessary, this can free up extra space in the next instruction. We walk
    * from back to front so that we pick nodes less likely to be scheduled
    * next first -- an extra move would be unnecessary there. But make sure
    * not to handle the complex1 case handled above.
    */
   if (ctx->instr->alu_num_slot_free > 0) {
      list_for_each_entry_rev(gpir_node, node, &ctx->ready_list, list) {
         if (!can_place_move(ctx, node))
            continue;

         if (node->sched.next_max_node &&
             !(node->op == gpir_op_complex1 && node->sched.ready)) {
            place_move(ctx, node);
            return true;
         }
      }
   }

   /* We may have skipped complex1 above, but if we run out of space, we still
    * need to insert the move.
    */

   if (ctx->instr->alu_num_unscheduled_next_max >
       ctx->instr->alu_max_allowed_next_max) {
      list_for_each_entry(gpir_node, node, &ctx->ready_list, list) {
         if (!can_place_move(ctx, node))
            continue;

         if (node->sched.next_max_node) {
            place_move(ctx, node);
            return true;
         }
      }
   }


   return false;
}

static bool gpir_sched_instr_pass(sched_ctx *ctx)
{
   if (try_node(ctx))
      return true;

   if (sched_move(ctx))
      return true;

   return false;
}

static void schedule_print_pre_one_instr(sched_ctx *ctx)
{
   if (!(lima_debug & LIMA_DEBUG_GP))
      return;

   printf("instr %d for ready list:", ctx->instr->index);
   list_for_each_entry(gpir_node, node, &ctx->ready_list, list) {
      printf(" %d/%c (%d, %d, %s)", node->index, node->sched.ready ? 'r' : 'p',
             node->sched.dist, gpir_get_slots_required(node),
             node->sched.max_node ? "max" : (node->sched.next_max_node ? "next" : "none"));
   }
   printf("\nlive physregs: ");
   for (unsigned i = 0; i < 16; i++) {
      if (ctx->live_physregs & (0xfull << (4 * i))) {
         printf("$%d.", i);
         for (unsigned j = 0; j < 4; j++) {
            if (ctx->live_physregs & (1ull << (4 * i + j)))
               printf("%c", "xyzw"[j]);
         }
         printf(" ");
      }
   }
   printf("\n");
}

static void schedule_print_post_one_instr(gpir_instr *instr)
{
   if (!(lima_debug & LIMA_DEBUG_GP))
      return;

   printf("post schedule instr");
   for (int i = 0; i < GPIR_INSTR_SLOT_NUM; i++) {
      if (instr->slots[i])
         printf(" %d/%d", i, instr->slots[i]->index);
   }
   printf("\n");
}


static bool schedule_one_instr(sched_ctx *ctx)
{
   gpir_instr *instr = gpir_instr_create(ctx->block);
   if (unlikely(!instr))
      return false;

   ctx->instr = instr;

   sched_find_max_nodes(ctx);
   schedule_print_pre_one_instr(ctx);

   while (gpir_sched_instr_pass(ctx)) {
      assert(ctx->ready_list_slots == gpir_get_curr_ready_list_slots(ctx));
#ifndef NDEBUG
      verify_max_nodes(ctx);
      verify_ready_list(ctx);
#endif
   }

   schedule_print_post_one_instr(instr);
   return true;
}

static bool schedule_block(gpir_block *block)
{
   /* calculate distance */
   list_for_each_entry(gpir_node, node, &block->node_list, list) {
      if (gpir_node_is_root(node))
         schedule_update_distance(node);
   }

   sched_ctx ctx;
   list_inithead(&ctx.ready_list);
   ctx.block = block;
   ctx.ready_list_slots = 0;
   ctx.live_physregs = block->live_out_phys;

   for (unsigned i = 0; i < GPIR_PHYSICAL_REG_NUM; i++) {
      list_inithead(&ctx.physreg_reads[i]);
   }

   /* construct the ready list from root nodes */
   list_for_each_entry_safe(gpir_node, node, &block->node_list, list) {
      /* Add to physreg_reads */
      if (node->op == gpir_op_load_reg) {
         gpir_load_node *load = gpir_node_to_load(node);
         unsigned index = 4 * load->index + load->component;
         list_addtail(&load->reg_link, &ctx.physreg_reads[index]);
      }

      if (gpir_node_is_root(node))
         schedule_insert_ready_list(&ctx, node);
   }

   list_inithead(&block->node_list);
   while (!list_is_empty(&ctx.ready_list)) {
      if (!schedule_one_instr(&ctx))
         return false;
   }

   return true;
}

static void schedule_build_dependency(gpir_block *block)
{
   /* merge dummy_f/m to the node created from */
   list_for_each_entry_safe(gpir_node, node, &block->node_list, list) {
      if (node->op == gpir_op_dummy_m) {
         gpir_alu_node *alu = gpir_node_to_alu(node);
         gpir_node *origin = alu->children[0];
         gpir_node *dummy_f = alu->children[1];

         gpir_node_foreach_succ(node, dep) {
            gpir_node *succ = dep->succ;
            /* origin and node may have same succ (by VREG/INPUT or
             * VREG/VREG dep), so use gpir_node_add_dep() instead of
             * gpir_node_replace_pred() */
            gpir_node_add_dep(succ, origin, dep->type);
            gpir_node_replace_child(succ, node, origin);
         }
         gpir_node_delete(dummy_f);
         gpir_node_delete(node);
      }
   }
}

static void print_statistic(gpir_compiler *comp, int save_index)
{
   int num_nodes[gpir_op_num] = {0};
   int num_created_nodes[gpir_op_num] = {0};

   list_for_each_entry(gpir_block, block, &comp->block_list, list) {
      list_for_each_entry(gpir_node, node, &block->node_list, list) {
         num_nodes[node->op]++;
         if (node->index >= save_index)
            num_created_nodes[node->op]++;
      }
   }

   printf("====== gpir scheduler statistic ======\n");
   printf("---- how many nodes are scheduled ----\n");
   int n = 0, l = 0;
   for (int i = 0; i < gpir_op_num; i++) {
      if (num_nodes[i]) {
         printf("%10s:%-6d", gpir_op_infos[i].name, num_nodes[i]);
         n += num_nodes[i];
         if (!(++l % 4))
            printf("\n");
      }
   }
   if (l % 4)
      printf("\n");
   printf("\ntotal: %d\n", n);

   printf("---- how many nodes are created ----\n");
   n = l = 0;
   for (int i = 0; i < gpir_op_num; i++) {
      if (num_created_nodes[i]) {
         printf("%10s:%-6d", gpir_op_infos[i].name, num_created_nodes[i]);
         n += num_created_nodes[i];
         if (!(++l % 4))
            printf("\n");
      }
   }
   if (l % 4)
      printf("\n");
   printf("\ntotal: %d\n", n);
   printf("------------------------------------\n");
}

bool gpir_schedule_prog(gpir_compiler *comp)
{
   int save_index = comp->cur_index;

   /* init schedule info */
   int index = 0;
   list_for_each_entry(gpir_block, block, &comp->block_list, list) {
      block->sched.instr_index = 0;
      list_for_each_entry(gpir_node, node, &block->node_list, list) {
         node->sched.instr = NULL;
         node->sched.pos = -1;
         node->sched.index = index++;
         node->sched.dist = -1;
         /* TODO when we support multiple basic blocks, we need a way to keep
          * track of this for physregs allocated before the scheduler.
          */
         node->sched.physreg_store = NULL;
         node->sched.ready = false;
         node->sched.inserted = false;
         node->sched.complex_allowed = false;
         node->sched.max_node = false;
         node->sched.next_max_node = false;
      }
   }

   /* build dependency */
   list_for_each_entry(gpir_block, block, &comp->block_list, list) {
      schedule_build_dependency(block);
   }

   //gpir_debug("after scheduler build reg dependency\n");
   //gpir_node_print_prog_dep(comp);

   list_for_each_entry(gpir_block, block, &comp->block_list, list) {
      if (!schedule_block(block)) {
         gpir_error("fail schedule block\n");
         return false;
      }
   }

   if (lima_debug & LIMA_DEBUG_GP) {
      print_statistic(comp, save_index);
      gpir_instr_print_prog(comp);
   }

   return true;
}
