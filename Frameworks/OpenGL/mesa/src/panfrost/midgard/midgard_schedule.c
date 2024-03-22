/*
 * Copyright (C) 2018-2019 Alyssa Rosenzweig <alyssa@rosenzweig.io>
 * Copyright (C) 2019-2020 Collabora, Ltd.
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

#include "util/half_float.h"
#include "util/u_math.h"
#include "util/u_memory.h"
#include "compiler.h"
#include "midgard_ops.h"
#include "midgard_quirks.h"

/* Scheduling for Midgard is complicated, to say the least. ALU instructions
 * must be grouped into VLIW bundles according to following model:
 *
 * [VMUL] [SADD]
 * [VADD] [SMUL] [VLUT]
 *
 * A given instruction can execute on some subset of the units (or a few can
 * execute on all). Instructions can be either vector or scalar; only scalar
 * instructions can execute on SADD/SMUL units. Units on a given line execute
 * in parallel. Subsequent lines execute separately and can pass results
 * directly via pipeline registers r24/r25, bypassing the register file.
 *
 * A bundle can optionally have 128-bits of embedded constants, shared across
 * all of the instructions within a bundle.
 *
 * Instructions consuming conditionals (branches and conditional selects)
 * require their condition to be written into the conditional register (r31)
 * within the same bundle they are consumed.
 *
 * Fragment writeout requires its argument to be written in full within the
 * same bundle as the branch, with no hanging dependencies.
 *
 * Load/store instructions are also in bundles of simply two instructions, and
 * texture instructions have no bundling.
 *
 * -------------------------------------------------------------------------
 *
 */

/* We create the dependency graph with per-byte granularity */

#define BYTE_COUNT 16

static void
add_dependency(struct util_dynarray *table, unsigned index, uint16_t mask,
               midgard_instruction **instructions, unsigned child)
{
   for (unsigned i = 0; i < BYTE_COUNT; ++i) {
      if (!(mask & (1 << i)))
         continue;

      struct util_dynarray *parents = &table[(BYTE_COUNT * index) + i];

      util_dynarray_foreach(parents, unsigned, parent) {
         BITSET_WORD *dependents = instructions[*parent]->dependents;

         /* Already have the dependency */
         if (BITSET_TEST(dependents, child))
            continue;

         BITSET_SET(dependents, child);
         instructions[child]->nr_dependencies++;
      }
   }
}

static void
mark_access(struct util_dynarray *table, unsigned index, uint16_t mask,
            unsigned parent)
{
   for (unsigned i = 0; i < BYTE_COUNT; ++i) {
      if (!(mask & (1 << i)))
         continue;

      util_dynarray_append(&table[(BYTE_COUNT * index) + i], unsigned, parent);
   }
}

static void
mir_create_dependency_graph(midgard_instruction **instructions, unsigned count,
                            unsigned node_count)
{
   size_t sz = node_count * BYTE_COUNT;

   struct util_dynarray *last_read = calloc(sizeof(struct util_dynarray), sz);
   struct util_dynarray *last_write = calloc(sizeof(struct util_dynarray), sz);

   for (unsigned i = 0; i < sz; ++i) {
      util_dynarray_init(&last_read[i], NULL);
      util_dynarray_init(&last_write[i], NULL);
   }

   /* Initialize dependency graph */
   for (unsigned i = 0; i < count; ++i) {
      instructions[i]->dependents =
         calloc(BITSET_WORDS(count), sizeof(BITSET_WORD));

      instructions[i]->nr_dependencies = 0;
   }

   unsigned prev_ldst[3] = {~0, ~0, ~0};

   /* Populate dependency graph */
   for (signed i = count - 1; i >= 0; --i) {
      if (instructions[i]->compact_branch)
         continue;

      unsigned dest = instructions[i]->dest;
      unsigned mask = mir_bytemask(instructions[i]);

      mir_foreach_src((*instructions), s) {
         unsigned src = instructions[i]->src[s];

         if (src < node_count) {
            unsigned readmask =
               mir_bytemask_of_read_components(instructions[i], src);
            add_dependency(last_write, src, readmask, instructions, i);
         }
      }

      /* Create a list of dependencies for each type of load/store
       * instruction to prevent reordering. */
      if (instructions[i]->type == TAG_LOAD_STORE_4 &&
          load_store_opcode_props[instructions[i]->op].props & LDST_ADDRESS) {

         unsigned type = instructions[i]->load_store.arg_reg |
                         instructions[i]->load_store.arg_comp;

         unsigned idx;
         switch (type) {
         case LDST_SHARED:
            idx = 0;
            break;
         case LDST_SCRATCH:
            idx = 1;
            break;
         default:
            idx = 2;
            break;
         }

         unsigned prev = prev_ldst[idx];

         if (prev != ~0) {
            BITSET_WORD *dependents = instructions[prev]->dependents;

            /* Already have the dependency */
            if (BITSET_TEST(dependents, i))
               continue;

            BITSET_SET(dependents, i);
            instructions[i]->nr_dependencies++;
         }

         prev_ldst[idx] = i;
      }

      if (dest < node_count) {
         add_dependency(last_read, dest, mask, instructions, i);
         add_dependency(last_write, dest, mask, instructions, i);
         mark_access(last_write, dest, mask, i);
      }

      mir_foreach_src((*instructions), s) {
         unsigned src = instructions[i]->src[s];

         if (src < node_count) {
            unsigned readmask =
               mir_bytemask_of_read_components(instructions[i], src);
            mark_access(last_read, src, readmask, i);
         }
      }
   }

   /* If there is a branch, all instructions depend on it, as interblock
    * execution must be purely in-order */

   if (instructions[count - 1]->compact_branch) {
      BITSET_WORD *dependents = instructions[count - 1]->dependents;

      for (signed i = count - 2; i >= 0; --i) {
         if (BITSET_TEST(dependents, i))
            continue;

         BITSET_SET(dependents, i);
         instructions[i]->nr_dependencies++;
      }
   }

   /* Free the intermediate structures */
   for (unsigned i = 0; i < sz; ++i) {
      util_dynarray_fini(&last_read[i]);
      util_dynarray_fini(&last_write[i]);
   }

   free(last_read);
   free(last_write);
}

/* Does the mask cover more than a scalar? */

static bool
is_single_component_mask(unsigned mask)
{
   int components = 0;

   for (int c = 0; c < 8; ++c) {
      if (mask & (1 << c))
         components++;
   }

   return components == 1;
}

/* Helpers for scheudling */

static bool
mir_is_scalar(midgard_instruction *ains)
{
   /* Do we try to use it as a vector op? */
   if (!is_single_component_mask(ains->mask))
      return false;

   /* Otherwise, check mode hazards */
   bool could_scalar = true;
   unsigned szd = nir_alu_type_get_type_size(ains->dest_type);
   unsigned sz0 = nir_alu_type_get_type_size(ains->src_types[0]);
   unsigned sz1 = nir_alu_type_get_type_size(ains->src_types[1]);

   /* Only 16/32-bit can run on a scalar unit */
   could_scalar &= (szd == 16) || (szd == 32);

   if (ains->src[0] != ~0)
      could_scalar &= (sz0 == 16) || (sz0 == 32);

   if (ains->src[1] != ~0)
      could_scalar &= (sz1 == 16) || (sz1 == 32);

   if (midgard_is_integer_out_op(ains->op) &&
       ains->outmod != midgard_outmod_keeplo)
      return false;

   return could_scalar;
}

/* How many bytes does this ALU instruction add to the bundle? */

static unsigned
bytes_for_instruction(midgard_instruction *ains)
{
   if (ains->unit & UNITS_ANY_VECTOR)
      return sizeof(midgard_reg_info) + sizeof(midgard_vector_alu);
   else if (ains->unit == ALU_ENAB_BRANCH)
      return sizeof(midgard_branch_extended);
   else if (ains->compact_branch)
      return sizeof(uint16_t);
   else
      return sizeof(midgard_reg_info) + sizeof(midgard_scalar_alu);
}

/* We would like to flatten the linked list of midgard_instructions in a bundle
 * to an array of pointers on the heap for easy indexing */

static midgard_instruction **
flatten_mir(midgard_block *block, unsigned *len)
{
   *len = list_length(&block->base.instructions);

   if (!(*len))
      return NULL;

   midgard_instruction **instructions =
      calloc(sizeof(midgard_instruction *), *len);

   unsigned i = 0;

   mir_foreach_instr_in_block(block, ins)
      instructions[i++] = ins;

   return instructions;
}

/* The worklist is the set of instructions that can be scheduled now; that is,
 * the set of instructions with no remaining dependencies */

static void
mir_initialize_worklist(BITSET_WORD *worklist,
                        midgard_instruction **instructions, unsigned count)
{
   for (unsigned i = 0; i < count; ++i) {
      if (instructions[i]->nr_dependencies == 0)
         BITSET_SET(worklist, i);
   }
}

/* Update the worklist after an instruction terminates. Remove its edges from
 * the graph and if that causes any node to have no dependencies, add it to the
 * worklist */

static void
mir_update_worklist(BITSET_WORD *worklist, unsigned count,
                    midgard_instruction **instructions,
                    midgard_instruction *done)
{
   /* Sanity check: if no instruction terminated, there is nothing to do.
    * If the instruction that terminated had dependencies, that makes no
    * sense and means we messed up the worklist. Finally, as the purpose
    * of this routine is to update dependents, we abort early if there are
    * no dependents defined. */

   if (!done)
      return;

   assert(done->nr_dependencies == 0);

   if (!done->dependents)
      return;

   /* We have an instruction with dependents. Iterate each dependent to
    * remove one dependency (`done`), adding dependents to the worklist
    * where possible. */

   unsigned i;
   BITSET_FOREACH_SET(i, done->dependents, count) {
      assert(instructions[i]->nr_dependencies);

      if (!(--instructions[i]->nr_dependencies))
         BITSET_SET(worklist, i);
   }

   free(done->dependents);
}

/* While scheduling, we need to choose instructions satisfying certain
 * criteria. As we schedule backwards, we choose the *last* instruction in the
 * worklist to simulate in-order scheduling. Chosen instructions must satisfy a
 * given predicate. */

struct midgard_predicate {
   /* TAG or ~0 for dont-care */
   unsigned tag;

   /* True if we want to pop off the chosen instruction */
   bool destructive;

   /* For ALU, choose only this unit */
   unsigned unit;

   /* State for bundle constants. constants is the actual constants
    * for the bundle. constant_count is the number of bytes (up to
    * 16) currently in use for constants. When picking in destructive
    * mode, the constants array will be updated, and the instruction
    * will be adjusted to index into the constants array */

   midgard_constants *constants;
   unsigned constant_mask;

   /* Exclude this destination (if not ~0) */
   unsigned exclude;

   /* Don't schedule instructions consuming conditionals (since we already
    * scheduled one). Excludes conditional branches and csel */
   bool no_cond;

   /* Require (or reject) a minimal mask and (if nonzero) given
    * destination. Used for writeout optimizations */

   unsigned mask;
   unsigned no_mask;
   unsigned dest;

   /* Whether to not-care/only/never schedule imov/fmov instructions This
    * allows non-move instructions to get priority on each unit */
   unsigned move_mode;

   /* For load/store: how many pipeline registers are in use? The two
    * scheduled instructions cannot use more than the 256-bits of pipeline
    * space available or RA will fail (as it would run out of pipeline
    * registers and fail to spill without breaking the schedule) */

   unsigned pipeline_count;

   /* For load/store: is a ST_VARY.a32 instruction scheduled into the
    * bundle? is a non-ST_VARY.a32 instruction scheduled? Potential
    * hardware issue, unknown cause.
    */
   bool any_st_vary_a32, any_non_st_vary_a32;
};

static bool
mir_adjust_constant(midgard_instruction *ins, unsigned src,
                    unsigned *bundle_constant_mask, unsigned *comp_mapping,
                    uint8_t *bundle_constants, bool upper)
{
   unsigned type_size = nir_alu_type_get_type_size(ins->src_types[src]) / 8;
   unsigned type_shift = util_logbase2(type_size);
   unsigned max_comp = mir_components_for_type(ins->src_types[src]);
   unsigned comp_mask = mir_from_bytemask(
      mir_round_bytemask_up(mir_bytemask_of_read_components_index(ins, src),
                            type_size * 8),
      type_size * 8);
   unsigned type_mask = (1 << type_size) - 1;

   /* Upper only makes sense for 16-bit */
   if (type_size != 16 && upper)
      return false;

   /* For 16-bit, we need to stay on either upper or lower halves to avoid
    * disrupting the swizzle */
   unsigned start = upper ? 8 : 0;
   unsigned length = (type_size == 2) ? 8 : 16;

   for (unsigned comp = 0; comp < max_comp; comp++) {
      if (!(comp_mask & (1 << comp)))
         continue;

      uint8_t *constantp = ins->constants.u8 + (type_size * comp);
      unsigned best_reuse_bytes = 0;
      signed best_place = -1;
      unsigned i, j;

      for (i = start; i < (start + length); i += type_size) {
         unsigned reuse_bytes = 0;

         for (j = 0; j < type_size; j++) {
            if (!(*bundle_constant_mask & (1 << (i + j))))
               continue;
            if (constantp[j] != bundle_constants[i + j])
               break;
            if ((i + j) > (start + length))
               break;

            reuse_bytes++;
         }

         /* Select the place where existing bytes can be
          * reused so we leave empty slots to others
          */
         if (j == type_size &&
             (reuse_bytes > best_reuse_bytes || best_place < 0)) {
            best_reuse_bytes = reuse_bytes;
            best_place = i;
            break;
         }
      }

      /* This component couldn't fit in the remaining constant slot,
       * no need check the remaining components, bail out now
       */
      if (best_place < 0)
         return false;

      memcpy(&bundle_constants[i], constantp, type_size);
      *bundle_constant_mask |= type_mask << best_place;
      comp_mapping[comp] = best_place >> type_shift;
   }

   return true;
}

/* For an instruction that can fit, adjust it to fit and update the constants
 * array, in destructive mode. Returns whether the fitting was successful. */

static bool
mir_adjust_constants(midgard_instruction *ins, struct midgard_predicate *pred,
                     bool destructive)
{
   /* No constant, nothing to adjust */
   if (!ins->has_constants)
      return true;

   unsigned r_constant = SSA_FIXED_REGISTER(REGISTER_CONSTANT);
   unsigned bundle_constant_mask = pred->constant_mask;
   unsigned comp_mapping[2][16] = {};
   uint8_t bundle_constants[16];

   memcpy(bundle_constants, pred->constants, 16);

   /* Let's try to find a place for each active component of the constant
    * register.
    */
   for (unsigned src = 0; src < 2; ++src) {
      if (ins->src[src] != SSA_FIXED_REGISTER(REGISTER_CONSTANT))
         continue;

      /* First, try lower half (or whole for !16) */
      if (mir_adjust_constant(ins, src, &bundle_constant_mask,
                              comp_mapping[src], bundle_constants, false))
         continue;

      /* Next, try upper half */
      if (mir_adjust_constant(ins, src, &bundle_constant_mask,
                              comp_mapping[src], bundle_constants, true))
         continue;

      /* Otherwise bail */
      return false;
   }

   /* If non-destructive, we're done */
   if (!destructive)
      return true;

   /* Otherwise update the constant_mask and constant values */
   pred->constant_mask = bundle_constant_mask;
   memcpy(pred->constants, bundle_constants, 16);

   /* Use comp_mapping as a swizzle */
   mir_foreach_src(ins, s) {
      if (ins->src[s] == r_constant)
         mir_compose_swizzle(ins->swizzle[s], comp_mapping[s], ins->swizzle[s]);
   }

   return true;
}

/* Conservative estimate of the pipeline registers required for load/store */

static unsigned
mir_pipeline_count(midgard_instruction *ins)
{
   unsigned bytecount = 0;

   mir_foreach_src(ins, i) {
      /* Skip empty source  */
      if (ins->src[i] == ~0)
         continue;

      if (i == 0) {
         /* First source is a vector, worst-case the mask */
         unsigned bytemask = mir_bytemask_of_read_components_index(ins, i);
         unsigned max = util_logbase2(bytemask) + 1;
         bytecount += max;
      } else {
         /* Sources 1 on are scalars */
         bytecount += 4;
      }
   }

   unsigned dwords = DIV_ROUND_UP(bytecount, 16);
   assert(dwords <= 2);

   return dwords;
}

/* Matches FADD x, x with modifiers compatible. Since x + x = x * 2, for
 * any x including of the form f(y) for some swizzle/abs/neg function f */

static bool
mir_is_add_2(midgard_instruction *ins)
{
   if (ins->op != midgard_alu_op_fadd)
      return false;

   if (ins->src[0] != ins->src[1])
      return false;

   if (ins->src_types[0] != ins->src_types[1])
      return false;

   for (unsigned i = 0; i < MIR_VEC_COMPONENTS; ++i) {
      if (ins->swizzle[0][i] != ins->swizzle[1][i])
         return false;
   }

   if (ins->src_abs[0] != ins->src_abs[1])
      return false;

   if (ins->src_neg[0] != ins->src_neg[1])
      return false;

   return true;
}

static void
mir_adjust_unit(midgard_instruction *ins, unsigned unit)
{
   /* FADD x, x = FMUL x, #2 */
   if (mir_is_add_2(ins) && (unit & (UNITS_MUL | UNIT_VLUT))) {
      ins->op = midgard_alu_op_fmul;

      ins->src[1] = ~0;
      ins->src_abs[1] = false;
      ins->src_neg[1] = false;

      ins->has_inline_constant = true;
      ins->inline_constant = _mesa_float_to_half(2.0);
   }
}

static unsigned
mir_has_unit(midgard_instruction *ins, unsigned unit)
{
   if (alu_opcode_props[ins->op].props & unit)
      return true;

   /* FADD x, x can run on any adder or any multiplier */
   if (mir_is_add_2(ins))
      return true;

   return false;
}

/* Net change in liveness if an instruction were scheduled. Loosely based on
 * ir3's scheduler. */

static int
mir_live_effect(uint16_t *liveness, midgard_instruction *ins, bool destructive)
{
   /* TODO: what if dest is used multiple times? */
   int free_live = 0;

   if (ins->dest < SSA_FIXED_MINIMUM) {
      unsigned bytemask = mir_bytemask(ins);
      bytemask = util_next_power_of_two(bytemask + 1) - 1;
      free_live += util_bitcount(liveness[ins->dest] & bytemask);

      if (destructive)
         liveness[ins->dest] &= ~bytemask;
   }

   int new_live = 0;

   mir_foreach_src(ins, s) {
      unsigned S = ins->src[s];

      bool dupe = false;

      for (unsigned q = 0; q < s; ++q)
         dupe |= (ins->src[q] == S);

      if (dupe)
         continue;

      if (S < SSA_FIXED_MINIMUM) {
         unsigned bytemask = mir_bytemask_of_read_components(ins, S);
         bytemask = util_next_power_of_two(bytemask + 1) - 1;

         /* Count only the new components */
         new_live += util_bitcount(bytemask & ~(liveness[S]));

         if (destructive)
            liveness[S] |= bytemask;
      }
   }

   return new_live - free_live;
}

static midgard_instruction *
mir_choose_instruction(midgard_instruction **instructions, uint16_t *liveness,
                       BITSET_WORD *worklist, unsigned count,
                       struct midgard_predicate *predicate)
{
   /* Parse the predicate */
   unsigned tag = predicate->tag;
   unsigned unit = predicate->unit;
   bool scalar = (unit != ~0) && (unit & UNITS_SCALAR);
   bool no_cond = predicate->no_cond;

   unsigned mask = predicate->mask;
   unsigned dest = predicate->dest;
   bool needs_dest = mask & 0xF;

   /* Iterate to find the best instruction satisfying the predicate */
   unsigned i;

   signed best_index = -1;
   signed best_effect = INT_MAX;
   bool best_conditional = false;

   /* Enforce a simple metric limiting distance to keep down register
    * pressure. TOOD: replace with liveness tracking for much better
    * results */

   unsigned max_active = 0;
   unsigned max_distance = 36;

#ifndef NDEBUG
   /* Force in-order scheduling */
   if (midgard_debug & MIDGARD_DBG_INORDER)
      max_distance = 1;
#endif

   BITSET_FOREACH_SET(i, worklist, count) {
      max_active = MAX2(max_active, i);
   }

   BITSET_FOREACH_SET(i, worklist, count) {
      if ((max_active - i) >= max_distance)
         continue;

      if (tag != ~0 && instructions[i]->type != tag)
         continue;

      bool alu = (instructions[i]->type == TAG_ALU_4);
      bool ldst = (instructions[i]->type == TAG_LOAD_STORE_4);

      bool branch = alu && (unit == ALU_ENAB_BR_COMPACT);
      bool is_move = alu && (instructions[i]->op == midgard_alu_op_imov ||
                             instructions[i]->op == midgard_alu_op_fmov);

      if (predicate->exclude != ~0 &&
          instructions[i]->dest == predicate->exclude)
         continue;

      if (alu && !branch && unit != ~0 &&
          !(mir_has_unit(instructions[i], unit)))
         continue;

      /* 0: don't care, 1: no moves, 2: only moves */
      if (predicate->move_mode && ((predicate->move_mode - 1) != is_move))
         continue;

      if (branch && !instructions[i]->compact_branch)
         continue;

      if (alu && scalar && !mir_is_scalar(instructions[i]))
         continue;

      if (alu && predicate->constants &&
          !mir_adjust_constants(instructions[i], predicate, false))
         continue;

      if (needs_dest && instructions[i]->dest != dest)
         continue;

      if (mask && ((~instructions[i]->mask) & mask))
         continue;

      if (instructions[i]->mask & predicate->no_mask)
         continue;

      if (ldst &&
          mir_pipeline_count(instructions[i]) + predicate->pipeline_count > 2)
         continue;

      bool st_vary_a32 = (instructions[i]->op == midgard_op_st_vary_32);

      if (ldst && predicate->any_non_st_vary_a32 && st_vary_a32)
         continue;

      if (ldst && predicate->any_st_vary_a32 && !st_vary_a32)
         continue;

      bool conditional = alu && !branch && OP_IS_CSEL(instructions[i]->op);
      conditional |= (branch && instructions[i]->branch.conditional);

      if (conditional && no_cond)
         continue;

      int effect = mir_live_effect(liveness, instructions[i], false);

      if (effect > best_effect)
         continue;

      if (effect == best_effect && (signed)i < best_index)
         continue;

      best_effect = effect;
      best_index = i;
      best_conditional = conditional;
   }

   /* Did we find anything?  */

   if (best_index < 0)
      return NULL;

   /* If we found something, remove it from the worklist */
   assert(best_index < count);
   midgard_instruction *I = instructions[best_index];

   if (predicate->destructive) {
      BITSET_CLEAR(worklist, best_index);

      if (I->type == TAG_ALU_4)
         mir_adjust_constants(instructions[best_index], predicate, true);

      if (I->type == TAG_LOAD_STORE_4) {
         predicate->pipeline_count +=
            mir_pipeline_count(instructions[best_index]);

         if (instructions[best_index]->op == midgard_op_st_vary_32)
            predicate->any_st_vary_a32 = true;
         else
            predicate->any_non_st_vary_a32 = true;
      }

      if (I->type == TAG_ALU_4)
         mir_adjust_unit(instructions[best_index], unit);

      /* Once we schedule a conditional, we can't again */
      predicate->no_cond |= best_conditional;
      mir_live_effect(liveness, instructions[best_index], true);
   }

   return I;
}

/* Still, we don't choose instructions in a vacuum. We need a way to choose the
 * best bundle type (ALU, load/store, texture). Nondestructive. */

static unsigned
mir_choose_bundle(midgard_instruction **instructions, uint16_t *liveness,
                  BITSET_WORD *worklist, unsigned count, unsigned num_ldst)
{
   /* At the moment, our algorithm is very simple - use the bundle of the
    * best instruction, regardless of what else could be scheduled
    * alongside it. This is not optimal but it works okay for in-order */

   struct midgard_predicate predicate = {
      .tag = ~0,
      .unit = ~0,
      .destructive = false,
      .exclude = ~0,
   };

   midgard_instruction *chosen = mir_choose_instruction(
      instructions, liveness, worklist, count, &predicate);

   if (chosen && chosen->type == TAG_LOAD_STORE_4 && !(num_ldst % 2)) {
      /* Try to schedule load/store ops in pairs */

      predicate.exclude = chosen->dest;
      predicate.tag = TAG_LOAD_STORE_4;

      chosen = mir_choose_instruction(instructions, liveness, worklist, count,
                                      &predicate);
      if (chosen)
         return TAG_LOAD_STORE_4;

      predicate.tag = ~0;

      chosen = mir_choose_instruction(instructions, liveness, worklist, count,
                                      &predicate);
      assert(chosen == NULL || chosen->type != TAG_LOAD_STORE_4);

      if (chosen)
         return chosen->type;
      else
         return TAG_LOAD_STORE_4;
   }

   if (chosen)
      return chosen->type;
   else
      return ~0;
}

/* We want to choose an ALU instruction filling a given unit */
static void
mir_choose_alu(midgard_instruction **slot, midgard_instruction **instructions,
               uint16_t *liveness, BITSET_WORD *worklist, unsigned len,
               struct midgard_predicate *predicate, unsigned unit)
{
   /* Did we already schedule to this slot? */
   if ((*slot) != NULL)
      return;

   /* Try to schedule something, if not */
   predicate->unit = unit;
   *slot =
      mir_choose_instruction(instructions, liveness, worklist, len, predicate);

   /* Store unit upon scheduling */
   if (*slot && !((*slot)->compact_branch))
      (*slot)->unit = unit;
}

/* When we are scheduling a branch/csel, we need the consumed condition in the
 * same block as a pipeline register. There are two options to enable this:
 *
 *  - Move the conditional into the bundle. Preferred, but only works if the
 *    conditional is used only once and is from this block.
 *  - Copy the conditional.
 *
 * We search for the conditional. If it's in this block, single-use, and
 * without embedded constants, we schedule it immediately. Otherwise, we
 * schedule a move for it.
 *
 * mir_comparison_mobile is a helper to find the moveable condition.
 */

static unsigned
mir_comparison_mobile(compiler_context *ctx, midgard_instruction **instructions,
                      struct midgard_predicate *predicate, unsigned count,
                      unsigned cond)
{
   if (!mir_single_use(ctx, cond))
      return ~0;

   unsigned ret = ~0;

   for (unsigned i = 0; i < count; ++i) {
      if (instructions[i]->dest != cond)
         continue;

      /* Must fit in an ALU bundle */
      if (instructions[i]->type != TAG_ALU_4)
         return ~0;

      /* If it would itself require a condition, that's recursive */
      if (OP_IS_CSEL(instructions[i]->op))
         return ~0;

      /* We'll need to rewrite to .w but that doesn't work for vector
       * ops that don't replicate (ball/bany), so bail there */

      if (GET_CHANNEL_COUNT(alu_opcode_props[instructions[i]->op].props))
         return ~0;

      /* Ensure it will fit with constants */

      if (!mir_adjust_constants(instructions[i], predicate, false))
         return ~0;

      /* Ensure it is written only once */

      if (ret != ~0)
         return ~0;
      else
         ret = i;
   }

   /* Inject constants now that we are sure we want to */
   if (ret != ~0)
      mir_adjust_constants(instructions[ret], predicate, true);

   return ret;
}

/* Using the information about the moveable conditional itself, we either pop
 * that condition off the worklist for use now, or create a move to
 * artificially schedule instead as a fallback */

static midgard_instruction *
mir_schedule_comparison(compiler_context *ctx,
                        midgard_instruction **instructions,
                        struct midgard_predicate *predicate,
                        BITSET_WORD *worklist, unsigned count, unsigned cond,
                        bool vector, unsigned *swizzle,
                        midgard_instruction *user)
{
   /* TODO: swizzle when scheduling */
   unsigned comp_i =
      (!vector && (swizzle[0] == 0))
         ? mir_comparison_mobile(ctx, instructions, predicate, count, cond)
         : ~0;

   /* If we can, schedule the condition immediately */
   if ((comp_i != ~0) && BITSET_TEST(worklist, comp_i)) {
      assert(comp_i < count);
      BITSET_CLEAR(worklist, comp_i);
      return instructions[comp_i];
   }

   /* Otherwise, we insert a move */

   midgard_instruction mov = v_mov(cond, cond);
   mov.mask = vector ? 0xF : 0x1;
   memcpy(mov.swizzle[1], swizzle, sizeof(mov.swizzle[1]));

   return mir_insert_instruction_before(ctx, user, mov);
}

/* Most generally, we need instructions writing to r31 in the appropriate
 * components */

static midgard_instruction *
mir_schedule_condition(compiler_context *ctx,
                       struct midgard_predicate *predicate,
                       BITSET_WORD *worklist, unsigned count,
                       midgard_instruction **instructions,
                       midgard_instruction *last)
{
   /* For a branch, the condition is the only argument; for csel, third */
   bool branch = last->compact_branch;
   unsigned condition_index = branch ? 0 : 2;

   /* csel_v is vector; otherwise, conditions are scalar */
   bool vector = !branch && OP_IS_CSEL_V(last->op);

   /* Grab the conditional instruction */

   midgard_instruction *cond = mir_schedule_comparison(
      ctx, instructions, predicate, worklist, count, last->src[condition_index],
      vector, last->swizzle[condition_index], last);

   /* We have exclusive reign over this (possibly move) conditional
    * instruction. We can rewrite into a pipeline conditional register */

   predicate->exclude = cond->dest;
   cond->dest = SSA_FIXED_REGISTER(31);
   last->src[condition_index] = cond->dest;

   if (!vector) {
      cond->mask = (1 << COMPONENT_W);

      mir_foreach_src(cond, s) {
         if (cond->src[s] == ~0)
            continue;

         for (unsigned q = 0; q < 4; ++q)
            cond->swizzle[s][q + COMPONENT_W] = cond->swizzle[s][q];
      }

      last->swizzle[condition_index][0] = COMPONENT_W;
   }

   /* Schedule the unit: csel is always in the latter pipeline, so a csel
    * condition must be in the former pipeline stage (vmul/sadd),
    * depending on scalar/vector of the instruction itself. A branch must
    * be written from the latter pipeline stage and a branch condition is
    * always scalar, so it is always in smul (exception: ball/bany, which
    * will be vadd) */

   if (branch)
      cond->unit = UNIT_SMUL;
   else
      cond->unit = vector ? UNIT_VMUL : UNIT_SADD;

   return cond;
}

/* Schedules a single bundle of the given type */

static midgard_bundle
mir_schedule_texture(midgard_instruction **instructions, uint16_t *liveness,
                     BITSET_WORD *worklist, unsigned len, bool is_vertex)
{
   struct midgard_predicate predicate = {
      .tag = TAG_TEXTURE_4,
      .destructive = true,
      .exclude = ~0,
   };

   midgard_instruction *ins =
      mir_choose_instruction(instructions, liveness, worklist, len, &predicate);

   mir_update_worklist(worklist, len, instructions, ins);

   struct midgard_bundle out = {
      .tag = ins->op == midgard_tex_op_barrier ? TAG_TEXTURE_4_BARRIER
             : (ins->op == midgard_tex_op_fetch) || is_vertex
                ? TAG_TEXTURE_4_VTX
                : TAG_TEXTURE_4,
      .instruction_count = 1,
      .instructions = {ins},
   };

   return out;
}

static midgard_bundle
mir_schedule_ldst(midgard_instruction **instructions, uint16_t *liveness,
                  BITSET_WORD *worklist, unsigned len, unsigned *num_ldst)
{
   struct midgard_predicate predicate = {
      .tag = TAG_LOAD_STORE_4,
      .destructive = true,
      .exclude = ~0,
   };

   /* Try to pick two load/store ops. Second not gauranteed to exist */

   midgard_instruction *ins =
      mir_choose_instruction(instructions, liveness, worklist, len, &predicate);

   midgard_instruction *pair =
      mir_choose_instruction(instructions, liveness, worklist, len, &predicate);

   assert(ins != NULL);

   struct midgard_bundle out = {
      .tag = TAG_LOAD_STORE_4,
      .instruction_count = pair ? 2 : 1,
      .instructions = {ins, pair},
   };

   *num_ldst -= out.instruction_count;

   /* We have to update the worklist atomically, since the two
    * instructions run concurrently (TODO: verify it's not pipelined) */

   mir_update_worklist(worklist, len, instructions, ins);
   mir_update_worklist(worklist, len, instructions, pair);

   return out;
}

static void
mir_schedule_zs_write(compiler_context *ctx,
                      struct midgard_predicate *predicate,
                      midgard_instruction **instructions, uint16_t *liveness,
                      BITSET_WORD *worklist, unsigned len,
                      midgard_instruction *branch, midgard_instruction **smul,
                      midgard_instruction **vadd, midgard_instruction **vlut,
                      bool stencil)
{
   bool success = false;
   unsigned idx = stencil ? 3 : 2;
   unsigned src =
      (branch->src[0] == ~0) ? SSA_FIXED_REGISTER(1) : branch->src[idx];

   predicate->dest = src;
   predicate->mask = 0x1;

   midgard_instruction **units[] = {smul, vadd, vlut};
   unsigned unit_names[] = {UNIT_SMUL, UNIT_VADD, UNIT_VLUT};

   for (unsigned i = 0; i < 3; ++i) {
      if (*(units[i]))
         continue;

      predicate->unit = unit_names[i];
      midgard_instruction *ins = mir_choose_instruction(
         instructions, liveness, worklist, len, predicate);

      if (ins) {
         ins->unit = unit_names[i];
         *(units[i]) = ins;
         success |= true;
         break;
      }
   }

   predicate->dest = predicate->mask = 0;

   if (success)
      return;

   midgard_instruction *mov = ralloc(ctx, midgard_instruction);
   *mov = v_mov(src, make_compiler_temp(ctx));
   mov->mask = 0x1;

   branch->src[idx] = mov->dest;

   if (stencil) {
      unsigned swizzle = (branch->src[0] == ~0) ? COMPONENT_Y : COMPONENT_X;

      for (unsigned c = 0; c < 16; ++c)
         mov->swizzle[1][c] = swizzle;
   }

   for (unsigned i = 0; i < 3; ++i) {
      if (!(*(units[i]))) {
         *(units[i]) = mov;
         mov->unit = unit_names[i];
         return;
      }
   }

   unreachable("Could not schedule Z/S move to any unit");
}

static midgard_bundle
mir_schedule_alu(compiler_context *ctx, midgard_instruction **instructions,
                 uint16_t *liveness, BITSET_WORD *worklist, unsigned len)
{
   struct midgard_bundle bundle = {};

   unsigned bytes_emitted = sizeof(bundle.control);

   struct midgard_predicate predicate = {
      .tag = TAG_ALU_4,
      .destructive = true,
      .exclude = ~0,
      .constants = &bundle.constants,
   };

   midgard_instruction *vmul = NULL;
   midgard_instruction *vadd = NULL;
   midgard_instruction *vlut = NULL;
   midgard_instruction *smul = NULL;
   midgard_instruction *sadd = NULL;
   midgard_instruction *branch = NULL;

   mir_choose_alu(&branch, instructions, liveness, worklist, len, &predicate,
                  ALU_ENAB_BR_COMPACT);
   mir_update_worklist(worklist, len, instructions, branch);
   unsigned writeout = branch ? branch->writeout : 0;

   if (branch && branch->branch.conditional) {
      midgard_instruction *cond = mir_schedule_condition(
         ctx, &predicate, worklist, len, instructions, branch);

      if (cond->unit == UNIT_VADD)
         vadd = cond;
      else if (cond->unit == UNIT_SMUL)
         smul = cond;
      else
         unreachable("Bad condition");
   }

   /* If we have a render target reference, schedule a move for it. Since
    * this will be in sadd, we boost this to prevent scheduling csel into
    * smul */

   if (writeout && (branch->constants.u32[0] || ctx->inputs->is_blend)) {
      sadd = ralloc(ctx, midgard_instruction);
      *sadd = v_mov(~0, make_compiler_temp(ctx));
      sadd->unit = UNIT_SADD;
      sadd->mask = 0x1;
      sadd->has_inline_constant = true;
      sadd->inline_constant = branch->constants.u32[0];
      branch->src[1] = sadd->dest;
      branch->src_types[1] = sadd->dest_type;
   }

   if (writeout) {
      /* Propagate up */
      bundle.last_writeout = branch->last_writeout;

      /* Mask off any conditionals.
       * This prevents csel and csel_v being scheduled into smul
       * since we might not have room for a conditional in vmul/sadd.
       * This is important because both writeout and csel have same-bundle
       * requirements on their dependencies. */
      predicate.no_cond = true;
   }

   /* Set r1.w to the return address so we can return from blend shaders */
   if (writeout) {
      vadd = ralloc(ctx, midgard_instruction);
      *vadd = v_mov(~0, make_compiler_temp(ctx));

      if (!ctx->inputs->is_blend) {
         vadd->op = midgard_alu_op_iadd;
         vadd->src[0] = SSA_FIXED_REGISTER(31);
         vadd->src_types[0] = nir_type_uint32;

         for (unsigned c = 0; c < 16; ++c)
            vadd->swizzle[0][c] = COMPONENT_X;

         vadd->has_inline_constant = true;
         vadd->inline_constant = 0;
      } else {
         vadd->src[1] = SSA_FIXED_REGISTER(1);
         vadd->src_types[0] = nir_type_uint32;

         for (unsigned c = 0; c < 16; ++c)
            vadd->swizzle[1][c] = COMPONENT_W;
      }

      vadd->unit = UNIT_VADD;
      vadd->mask = 0x1;
      branch->dest = vadd->dest;
      branch->dest_type = vadd->dest_type;
   }

   if (writeout & PAN_WRITEOUT_Z)
      mir_schedule_zs_write(ctx, &predicate, instructions, liveness, worklist,
                            len, branch, &smul, &vadd, &vlut, false);

   if (writeout & PAN_WRITEOUT_S)
      mir_schedule_zs_write(ctx, &predicate, instructions, liveness, worklist,
                            len, branch, &smul, &vadd, &vlut, true);

   mir_choose_alu(&smul, instructions, liveness, worklist, len, &predicate,
                  UNIT_SMUL);

   for (unsigned mode = 1; mode < 3; ++mode) {
      predicate.move_mode = mode;
      predicate.no_mask = writeout ? (1 << 3) : 0;
      mir_choose_alu(&vlut, instructions, liveness, worklist, len, &predicate,
                     UNIT_VLUT);
      predicate.no_mask = 0;
      mir_choose_alu(&vadd, instructions, liveness, worklist, len, &predicate,
                     UNIT_VADD);
   }

   /* Reset */
   predicate.move_mode = 0;
   predicate.exclude = ~0;

   mir_update_worklist(worklist, len, instructions, vlut);
   mir_update_worklist(worklist, len, instructions, vadd);
   mir_update_worklist(worklist, len, instructions, smul);

   bool vadd_csel = vadd && OP_IS_CSEL(vadd->op);
   bool smul_csel = smul && OP_IS_CSEL(smul->op);

   if (vadd_csel || smul_csel) {
      midgard_instruction *ins = vadd_csel ? vadd : smul;
      midgard_instruction *cond = mir_schedule_condition(
         ctx, &predicate, worklist, len, instructions, ins);

      if (cond->unit == UNIT_VMUL)
         vmul = cond;
      else if (cond->unit == UNIT_SADD)
         sadd = cond;
      else
         unreachable("Bad condition");
   }

   /* Stage 2, let's schedule sadd before vmul for writeout */
   mir_choose_alu(&sadd, instructions, liveness, worklist, len, &predicate,
                  UNIT_SADD);

   /* Check if writeout reads its own register */

   if (writeout) {
      midgard_instruction *stages[] = {sadd, vadd, smul, vlut};
      unsigned src =
         (branch->src[0] == ~0) ? SSA_FIXED_REGISTER(0) : branch->src[0];
      unsigned writeout_mask = 0x0;
      bool bad_writeout = false;

      for (unsigned i = 0; i < ARRAY_SIZE(stages); ++i) {
         if (!stages[i])
            continue;

         if (stages[i]->dest != src)
            continue;

         writeout_mask |= stages[i]->mask;
         bad_writeout |= mir_has_arg(stages[i], branch->src[0]);
      }

      /* It's possible we'll be able to schedule something into vmul
       * to fill r0. Let's peak into the future, trying to schedule
       * vmul specially that way. */

      unsigned full_mask = 0xF;

      if (!bad_writeout && writeout_mask != full_mask) {
         predicate.unit = UNIT_VMUL;
         predicate.dest = src;
         predicate.mask = writeout_mask ^ full_mask;

         struct midgard_instruction *peaked = mir_choose_instruction(
            instructions, liveness, worklist, len, &predicate);

         if (peaked) {
            vmul = peaked;
            vmul->unit = UNIT_VMUL;
            writeout_mask |= predicate.mask;
            assert(writeout_mask == full_mask);
         }

         /* Cleanup */
         predicate.dest = predicate.mask = 0;
      }

      /* Finally, add a move if necessary */
      if (bad_writeout || writeout_mask != full_mask) {
         unsigned temp = (branch->src[0] == ~0) ? SSA_FIXED_REGISTER(0)
                                                : make_compiler_temp(ctx);

         vmul = ralloc(ctx, midgard_instruction);
         *vmul = v_mov(src, temp);
         vmul->unit = UNIT_VMUL;
         vmul->mask = full_mask ^ writeout_mask;

         /* Rewrite to use our temp */

         for (unsigned i = 0; i < ARRAY_SIZE(stages); ++i) {
            if (stages[i]) {
               mir_rewrite_index_dst_single(stages[i], src, temp);
               mir_rewrite_index_src_single(stages[i], src, temp);
            }
         }

         mir_rewrite_index_src_single(branch, src, temp);
      }
   }

   mir_choose_alu(&vmul, instructions, liveness, worklist, len, &predicate,
                  UNIT_VMUL);

   mir_update_worklist(worklist, len, instructions, vmul);
   mir_update_worklist(worklist, len, instructions, sadd);

   bundle.has_embedded_constants = predicate.constant_mask != 0;

   unsigned padding = 0;

   /* Now that we have finished scheduling, build up the bundle */
   midgard_instruction *stages[] = {vmul, sadd, vadd, smul, vlut, branch};

   for (unsigned i = 0; i < ARRAY_SIZE(stages); ++i) {
      if (stages[i]) {
         bundle.control |= stages[i]->unit;
         bytes_emitted += bytes_for_instruction(stages[i]);
         bundle.instructions[bundle.instruction_count++] = stages[i];

         /* If we branch, we can't spill to TLS since the store
          * instruction will never get executed. We could try to
          * break the bundle but this is probably easier for
          * now. */

         if (branch)
            stages[i]->no_spill |= (1 << REG_CLASS_WORK);
      }
   }

   /* Pad ALU op to nearest word */

   if (bytes_emitted & 15) {
      padding = 16 - (bytes_emitted & 15);
      bytes_emitted += padding;
   }

   /* Constants must always be quadwords */
   if (bundle.has_embedded_constants)
      bytes_emitted += 16;

   /* Size ALU instruction for tag */
   bundle.tag = (TAG_ALU_4) + (bytes_emitted / 16) - 1;

   bool tilebuf_wait = branch && branch->compact_branch &&
                       branch->branch.target_type == TARGET_TILEBUF_WAIT;

   /* MRT capable GPUs use a special writeout procedure */
   if ((writeout || tilebuf_wait) && !(ctx->quirks & MIDGARD_NO_UPPER_ALU))
      bundle.tag += 4;

   bundle.padding = padding;
   bundle.control |= bundle.tag;

   return bundle;
}

/* Schedule a single block by iterating its instruction to create bundles.
 * While we go, tally about the bundle sizes to compute the block size. */

static void
schedule_block(compiler_context *ctx, midgard_block *block)
{
   /* Copy list to dynamic array */
   unsigned len = 0;
   midgard_instruction **instructions = flatten_mir(block, &len);

   if (!len)
      return;

   /* Calculate dependencies and initial worklist */
   unsigned node_count = ctx->temp_count + 1;
   mir_create_dependency_graph(instructions, len, node_count);

   /* Allocate the worklist */
   size_t sz = BITSET_WORDS(len) * sizeof(BITSET_WORD);
   BITSET_WORD *worklist = calloc(sz, 1);
   uint16_t *liveness = calloc(node_count, 2);
   mir_initialize_worklist(worklist, instructions, len);

   /* Count the number of load/store instructions so we know when it's
    * worth trying to schedule them in pairs. */
   unsigned num_ldst = 0;
   for (unsigned i = 0; i < len; ++i) {
      if (instructions[i]->type == TAG_LOAD_STORE_4)
         ++num_ldst;
   }

   struct util_dynarray bundles;
   util_dynarray_init(&bundles, NULL);

   block->quadword_count = 0;

   for (;;) {
      unsigned tag =
         mir_choose_bundle(instructions, liveness, worklist, len, num_ldst);
      midgard_bundle bundle;

      if (tag == TAG_TEXTURE_4)
         bundle = mir_schedule_texture(instructions, liveness, worklist, len,
                                       ctx->stage != MESA_SHADER_FRAGMENT);
      else if (tag == TAG_LOAD_STORE_4)
         bundle =
            mir_schedule_ldst(instructions, liveness, worklist, len, &num_ldst);
      else if (tag == TAG_ALU_4)
         bundle = mir_schedule_alu(ctx, instructions, liveness, worklist, len);
      else
         break;

      for (unsigned i = 0; i < bundle.instruction_count; ++i)
         bundle.instructions[i]->bundle_id =
            ctx->quadword_count + block->quadword_count;

      util_dynarray_append(&bundles, midgard_bundle, bundle);
      block->quadword_count += midgard_tag_props[bundle.tag].size;
   }

   assert(num_ldst == 0);

   /* We emitted bundles backwards; copy into the block in reverse-order */

   util_dynarray_init(&block->bundles, block);
   util_dynarray_foreach_reverse(&bundles, midgard_bundle, bundle) {
      util_dynarray_append(&block->bundles, midgard_bundle, *bundle);
   }
   util_dynarray_fini(&bundles);

   block->scheduled = true;
   ctx->quadword_count += block->quadword_count;

   /* Reorder instructions to match bundled. First remove existing
    * instructions and then recreate the list */

   mir_foreach_instr_in_block_safe(block, ins) {
      list_del(&ins->link);
   }

   mir_foreach_instr_in_block_scheduled_rev(block, ins) {
      list_add(&ins->link, &block->base.instructions);
   }

   free(instructions); /* Allocated by flatten_mir() */
   free(worklist);
   free(liveness);
}

/* Insert moves to ensure we can register allocate load/store registers */
static void
mir_lower_ldst(compiler_context *ctx)
{
   mir_foreach_instr_global_safe(ctx, I) {
      if (I->type != TAG_LOAD_STORE_4)
         continue;

      mir_foreach_src(I, s) {
         if (s == 0)
            continue;
         if (I->src[s] == ~0)
            continue;
         if (I->swizzle[s][0] == 0)
            continue;

         unsigned temp = make_compiler_temp(ctx);
         midgard_instruction mov = v_mov(I->src[s], temp);
         mov.mask = 0x1;
         mov.dest_type = I->src_types[s];
         for (unsigned c = 0; c < NIR_MAX_VEC_COMPONENTS; ++c)
            mov.swizzle[1][c] = I->swizzle[s][0];

         mir_insert_instruction_before(ctx, I, mov);
         I->src[s] = mov.dest;
         I->swizzle[s][0] = 0;
      }
   }
}

/* Insert moves to ensure we can register allocate blend writeout */
static void
mir_lower_blend_input(compiler_context *ctx)
{
   mir_foreach_block(ctx, _blk) {
      midgard_block *blk = (midgard_block *)_blk;

      if (list_is_empty(&blk->base.instructions))
         continue;

      midgard_instruction *I = mir_last_in_block(blk);

      if (!I || I->type != TAG_ALU_4 || !I->writeout)
         continue;

      mir_foreach_src(I, s) {
         unsigned src = I->src[s];

         if (src >= ctx->temp_count)
            continue;

         if (!_blk->live_out[src])
            continue;

         unsigned temp = make_compiler_temp(ctx);
         midgard_instruction mov = v_mov(src, temp);
         mov.mask = 0xF;
         mov.dest_type = nir_type_uint32;
         mir_insert_instruction_before(ctx, I, mov);
         I->src[s] = mov.dest;
      }
   }
}

void
midgard_schedule_program(compiler_context *ctx)
{
   mir_lower_ldst(ctx);
   midgard_promote_uniforms(ctx);

   /* Must be lowered right before scheduling */
   mir_lower_special_reads(ctx);
   mir_squeeze_index(ctx);

   if (ctx->stage == MESA_SHADER_FRAGMENT) {
      mir_invalidate_liveness(ctx);
      mir_compute_liveness(ctx);
      mir_lower_blend_input(ctx);
   }

   mir_squeeze_index(ctx);

   /* Lowering can introduce some dead moves */

   mir_foreach_block(ctx, _block) {
      midgard_block *block = (midgard_block *)_block;
      midgard_opt_dead_move_eliminate(ctx, block);
      schedule_block(ctx, block);
   }
}
