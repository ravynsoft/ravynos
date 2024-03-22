/*
 * Copyright © 2014 Intel Corporation
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
 */

/** @file brw_fs_combine_constants.cpp
 *
 * This file contains the opt_combine_constants() pass that runs after the
 * regular optimization loop. It passes over the instruction list and
 * selectively promotes immediate values to registers by emitting a mov(1)
 * instruction.
 *
 * This is useful on Gen 7 particularly, because a few instructions can be
 * coissued (i.e., issued in the same cycle as another thread on the same EU
 * issues an instruction) under some circumstances, one of which is that they
 * cannot use immediate values.
 */

#include "brw_fs.h"
#include "brw_fs_builder.h"
#include "brw_cfg.h"
#include "util/half_float.h"

using namespace brw;

static const bool debug = false;

enum PACKED interpreted_type {
   float_only = 0,
   integer_only,
   either_type
};

struct value {
   /** Raw bit pattern of the value. */
   nir_const_value value;

   /** Instruction that uses this instance of the value. */
   unsigned instr_index;

   /** Size, in bits, of the value. */
   uint8_t bit_size;

   /**
    * Which source of instr is this value?
    *
    * \note This field is not actually used by \c brw_combine_constants, but
    * it is generally very useful to callers.
    */
   uint8_t src;

   /**
    * In what ways can instr interpret this value?
    *
    * Choices are floating-point only, integer only, or either type.
    */
   enum interpreted_type type;

   /**
    * Only try to make a single source non-constant.
    *
    * On some architectures, some instructions require that all sources be
    * non-constant.  For example, the multiply-accumulate instruction on Intel
    * GPUs upto Gen11 require that all sources be non-constant.  Other
    * instructions, like the selection instruction, allow one constant source.
    *
    * If a single constant source is allowed, set this flag to true.
    *
    * If an instruction allows a single constant and it has only a signle
    * constant to begin, it should be included.  Various places in
    * \c combine_constants will assume that there are multiple constants if
    * \c ::allow_one_constant is set.  This may even be enforced by in-code
    * assertions.
    */
   bool allow_one_constant;

   /**
    * Restrict values that can reach this value to not include negations.
    *
    * This is useful for instructions that cannot have source modifiers.  For
    * example, on Intel GPUs the integer source of a shift instruction (e.g.,
    * SHL) can have a source modifier, but the integer source of the bitfield
    * insertion instruction (i.e., BFI2) cannot.  A pair of these instructions
    * might have sources that are negations of each other.  Using this flag
    * will ensure that the BFI2 does not have a negated source, but the SHL
    * might.
    */
   bool no_negations;

   /**
    * \name UtilCombineConstantsPrivate
    * Private data used only by brw_combine_constants
    *
    * Any data stored in these fields will be overwritten by the call to
    * \c brw_combine_constants.  No assumptions should be made about the
    * state of these fields after that function returns.
    */
   /**@{*/
   /** Mask of negations that can be generated from this value. */
   uint8_t reachable_mask;

   /** Mask of negations that can generate this value. */
   uint8_t reaching_mask;

   /**
    * Value with the next source from the same instruction.
    *
    * This pointer may be \c NULL.  If it is not \c NULL, it will form a
    * singly-linked circular list of values.  The list is unorderd.  That is,
    * as the list is iterated, the \c ::src values will be in arbitrary order.
    *
    * \todo Is it even possible for there to be more than two elements in this
    * list?  This pass does not operate on vecN instructions or intrinsics, so
    * the theoretical limit should be three.  However, instructions with all
    * constant sources should have been folded away.
    */
   struct value *next_src;
   /**@}*/
};

struct combine_constants_value {
   /** Raw bit pattern of the constant loaded. */
   nir_const_value value;

   /**
    * Index of the first user.
    *
    * This is the offset into \c combine_constants_result::user_map of the
    * first user of this value.
    */
   unsigned first_user;

   /** Number of users of this value. */
   unsigned num_users;

   /** Size, in bits, of the value. */
   uint8_t bit_size;
};

struct combine_constants_user {
   /** Index into the array of values passed to brw_combine_constants. */
   unsigned index;

   /**
    * Manner in which the value should be interpreted in the instruction.
    *
    * This is only useful when ::negate is set.  Unless the corresponding
    * value::type is \c either_type, this field must have the same value as
    * value::type.
    */
   enum interpreted_type type;

   /** Should this value be negated to generate the original value? */
   bool negate;
};

class combine_constants_result {
public:
   combine_constants_result(unsigned num_candidates, bool &success)
      : num_values_to_emit(0), user_map(NULL)
   {
      user_map = (struct combine_constants_user *) calloc(num_candidates,
                                                          sizeof(user_map[0]));

      /* In the worst case, the number of output values will be equal to the
       * number of input values.  Allocate a buffer that is known to be large
       * enough now, and it can be reduced later.
       */
      values_to_emit =
         (struct combine_constants_value *) calloc(num_candidates,
                                                   sizeof(values_to_emit[0]));

      success = (user_map != NULL && values_to_emit != NULL);
   }

   ~combine_constants_result()
   {
      free(values_to_emit);
      free(user_map);
   }

   void append_value(const nir_const_value &value, unsigned bit_size)
   {
      values_to_emit[num_values_to_emit].value = value;
      values_to_emit[num_values_to_emit].first_user = 0;
      values_to_emit[num_values_to_emit].num_users = 0;
      values_to_emit[num_values_to_emit].bit_size = bit_size;
      num_values_to_emit++;
   }

   unsigned num_values_to_emit;
   struct combine_constants_value *values_to_emit;

   struct combine_constants_user *user_map;
};

#define VALUE_INDEX                  0
#define FLOAT_NEG_INDEX              1
#define INT_NEG_INDEX                2
#define MAX_NUM_REACHABLE            3

#define VALUE_EXISTS                 (1 << VALUE_INDEX)
#define FLOAT_NEG_EXISTS             (1 << FLOAT_NEG_INDEX)
#define INT_NEG_EXISTS               (1 << INT_NEG_INDEX)

static bool
negation_exists(nir_const_value v, unsigned bit_size,
                enum interpreted_type base_type)
{
   /* either_type does not make sense in this context. */
   assert(base_type == float_only || base_type == integer_only);

   switch (bit_size) {
   case 8:
      if (base_type == float_only)
         return false;
      else
         return v.i8 != 0 && v.i8 != INT8_MIN;

   case 16:
      if (base_type == float_only)
         return !util_is_half_nan(v.i16);
      else
         return v.i16 != 0 && v.i16 != INT16_MIN;

   case 32:
      if (base_type == float_only)
         return !isnan(v.f32);
      else
         return v.i32 != 0 && v.i32 != INT32_MIN;

   case 64:
      if (base_type == float_only)
         return !isnan(v.f64);
      else
         return v.i64 != 0 && v.i64 != INT64_MIN;

   default:
      unreachable("unsupported bit-size should have already been filtered.");
   }
}

static nir_const_value
negate(nir_const_value v, unsigned bit_size, enum interpreted_type base_type)
{
   /* either_type does not make sense in this context. */
   assert(base_type == float_only || base_type == integer_only);

   nir_const_value ret = { 0, };

   switch (bit_size) {
   case 8:
      assert(base_type == integer_only);
      ret.i8 = -v.i8;
      break;

   case 16:
      if (base_type == float_only)
         ret.u16 = v.u16 ^ INT16_MIN;
      else
         ret.i16 = -v.i16;
      break;

   case 32:
      if (base_type == float_only)
         ret.u32 = v.u32 ^ INT32_MIN;
      else
         ret.i32 = -v.i32;
      break;

   case 64:
      if (base_type == float_only)
         ret.u64 = v.u64 ^ INT64_MIN;
      else
         ret.i64 = -v.i64;
      break;

   default:
      unreachable("unsupported bit-size should have already been filtered.");
   }

   return ret;
}

static nir_const_value
absolute(nir_const_value v, unsigned bit_size, enum interpreted_type base_type)
{
   /* either_type does not make sense in this context. */
   assert(base_type == float_only || base_type == integer_only);

   nir_const_value ret = { 0, };

   switch (bit_size) {
   case 8:
      assert(base_type == integer_only);
      ret.i8 = abs(v.i8);
      break;

   case 16:
      if (base_type == float_only)
         ret.u16 = v.u16 & 0x7fff;
      else
         ret.i16 = abs(v.i16);
      break;

   case 32:
      if (base_type == float_only)
         ret.f32 = fabs(v.f32);
      else
         ret.i32 = abs(v.i32);
      break;

   case 64:
      if (base_type == float_only)
         ret.f64 = fabs(v.f64);
      else {
         if (sizeof(v.i64) == sizeof(long int)) {
            ret.i64 = labs((long int) v.i64);
         } else {
            assert(sizeof(v.i64) == sizeof(long long int));
            ret.i64 = llabs((long long int) v.i64);
         }
      }
      break;

   default:
      unreachable("unsupported bit-size should have already been filtered.");
   }

   return ret;
}

static void
calculate_masks(nir_const_value v, enum interpreted_type type,
                unsigned bit_size, uint8_t *reachable_mask,
                uint8_t *reaching_mask)
{
   *reachable_mask = 0;
   *reaching_mask = 0;

   /* Calculate the extended reachable mask. */
   if (type == float_only || type == either_type) {
      if (negation_exists(v, bit_size, float_only))
         *reachable_mask |= FLOAT_NEG_EXISTS;
   }

   if (type == integer_only || type == either_type) {
      if (negation_exists(v, bit_size, integer_only))
         *reachable_mask |= INT_NEG_EXISTS;
   }

   /* Calculate the extended reaching mask.  All of the "is this negation
    * possible" was already determined for the reachable_mask, so reuse that
    * data.
    */
   if (type == float_only || type == either_type) {
      if (*reachable_mask & FLOAT_NEG_EXISTS)
         *reaching_mask |= FLOAT_NEG_EXISTS;
   }

   if (type == integer_only || type == either_type) {
      if (*reachable_mask & INT_NEG_EXISTS)
         *reaching_mask |= INT_NEG_EXISTS;
   }
}

static void
calculate_reachable_values(nir_const_value v,
                           unsigned bit_size,
                           unsigned reachable_mask,
                           nir_const_value *reachable_values)
{
   memset(reachable_values, 0, MAX_NUM_REACHABLE * sizeof(reachable_values[0]));

   reachable_values[VALUE_INDEX] = v;

   if (reachable_mask & INT_NEG_EXISTS) {
      const nir_const_value neg = negate(v, bit_size, integer_only);

      reachable_values[INT_NEG_INDEX] = neg;
   }

   if (reachable_mask & FLOAT_NEG_EXISTS) {
      const nir_const_value neg = negate(v, bit_size, float_only);

      reachable_values[FLOAT_NEG_INDEX] = neg;
   }
}

static bool
value_equal(nir_const_value a, nir_const_value b, unsigned bit_size)
{
   switch (bit_size) {
   case 8:
      return a.u8 == b.u8;
   case 16:
      return a.u16 == b.u16;
   case 32:
      return a.u32 == b.u32;
   case 64:
      return a.u64 == b.u64;
   default:
      unreachable("unsupported bit-size should have already been filtered.");
   }
}

/** Can these values be the same with one level of negation? */
static bool
value_can_equal(const nir_const_value *from, uint8_t reachable_mask,
                nir_const_value to, uint8_t reaching_mask,
                unsigned bit_size)
{
   const uint8_t combined_mask = reachable_mask & reaching_mask;

   return value_equal(from[VALUE_INDEX], to, bit_size) ||
          ((combined_mask & INT_NEG_EXISTS) &&
           value_equal(from[INT_NEG_INDEX], to, bit_size)) ||
          ((combined_mask & FLOAT_NEG_EXISTS) &&
           value_equal(from[FLOAT_NEG_INDEX], to, bit_size));
}

static void
preprocess_candidates(struct value *candidates, unsigned num_candidates)
{
   /* Calculate the reaching_mask and reachable_mask for each candidate. */
   for (unsigned i = 0; i < num_candidates; i++) {
      calculate_masks(candidates[i].value,
                      candidates[i].type,
                      candidates[i].bit_size,
                      &candidates[i].reachable_mask,
                      &candidates[i].reaching_mask);

      /* If negations are not allowed, then only the original value is
       * reaching.
       */
      if (candidates[i].no_negations)
         candidates[i].reaching_mask = 0;
   }

   for (unsigned i = 0; i < num_candidates; i++)
      candidates[i].next_src = NULL;

   for (unsigned i = 0; i < num_candidates - 1; i++) {
      if (candidates[i].next_src != NULL)
         continue;

      struct value *prev = &candidates[i];

      for (unsigned j = i + 1; j < num_candidates; j++) {
         if (candidates[i].instr_index == candidates[j].instr_index) {
            prev->next_src = &candidates[j];
            prev = prev->next_src;
         }
      }

      /* Close the cycle. */
      if (prev != &candidates[i])
         prev->next_src = &candidates[i];
   }
}

static bool
reaching_value_exists(const struct value *c,
                      const struct combine_constants_value *values,
                      unsigned num_values)
{
   nir_const_value reachable_values[MAX_NUM_REACHABLE];

   calculate_reachable_values(c->value, c->bit_size, c->reaching_mask,
                              reachable_values);

   /* Check to see if the value is already in the result set. */
   for (unsigned j = 0; j < num_values; j++) {
      if (c->bit_size == values[j].bit_size &&
          value_can_equal(reachable_values, c->reaching_mask,
                          values[j].value, c->reaching_mask,
                          c->bit_size)) {
         return true;
      }
   }

   return false;
}

static combine_constants_result *
combine_constants_greedy(struct value *candidates, unsigned num_candidates)
{
   bool success;
   combine_constants_result *result =
      new combine_constants_result(num_candidates, success);
   if (result == NULL || !success) {
      delete result;
      return NULL;
   }

   BITSET_WORD *remain =
      (BITSET_WORD *) calloc(BITSET_WORDS(num_candidates), sizeof(remain[0]));

   if (remain == NULL) {
      delete result;
      return NULL;
   }

   memset(remain, 0xff, BITSET_WORDS(num_candidates) * sizeof(remain[0]));

   /* Operate in three passes.  The first pass handles all values that must be
    * emitted and for which a negation cannot exist.
    */
   unsigned i;
   for (i = 0; i < num_candidates; i++) {
      if (candidates[i].allow_one_constant ||
          (candidates[i].reaching_mask & (FLOAT_NEG_EXISTS | INT_NEG_EXISTS))) {
         continue;
      }

      /* Check to see if the value is already in the result set. */
      bool found = false;
      const unsigned num_values = result->num_values_to_emit;
      for (unsigned j = 0; j < num_values; j++) {
         if (candidates[i].bit_size == result->values_to_emit[j].bit_size &&
             value_equal(candidates[i].value,
                         result->values_to_emit[j].value,
                         candidates[i].bit_size)) {
            found = true;
            break;
         }
      }

      if (!found)
         result->append_value(candidates[i].value, candidates[i].bit_size);

      BITSET_CLEAR(remain, i);
   }

   /* The second pass handles all values that must be emitted and for which a
    * negation can exist.
    */
   BITSET_FOREACH_SET(i, remain, num_candidates) {
      if (candidates[i].allow_one_constant)
         continue;

      assert(candidates[i].reaching_mask & (FLOAT_NEG_EXISTS | INT_NEG_EXISTS));

      if (!reaching_value_exists(&candidates[i], result->values_to_emit,
                                 result->num_values_to_emit)) {
         result->append_value(absolute(candidates[i].value,
                                       candidates[i].bit_size,
                                       candidates[i].type),
                              candidates[i].bit_size);
      }

      BITSET_CLEAR(remain, i);
   }

   /* The third pass handles all of the values that may not have to be
    * emitted.  These are the values where allow_one_constant is set.
    */
   BITSET_FOREACH_SET(i, remain, num_candidates) {
      assert(candidates[i].allow_one_constant);

      /* The BITSET_FOREACH_SET macro does not detect changes to the bitset
       * that occur within the current word.  Since code in this loop may
       * clear bits from the set, re-test here.
       */
      if (!BITSET_TEST(remain, i))
         continue;

      assert(candidates[i].next_src != NULL);

      const struct value *const other_candidate = candidates[i].next_src;
      const unsigned j = other_candidate - candidates;

      if (!reaching_value_exists(&candidates[i], result->values_to_emit,
                                 result->num_values_to_emit)) {
         /* Before emitting a value, see if a match for the other source of
          * the instruction exists.
          */
         if (!reaching_value_exists(&candidates[j], result->values_to_emit,
                                    result->num_values_to_emit)) {
            result->append_value(candidates[i].value, candidates[i].bit_size);
         }
      }

      /* Mark both sources as handled. */
      BITSET_CLEAR(remain, i);
      BITSET_CLEAR(remain, j);
   }

   /* As noted above, there will never be more values in the output than in
    * the input.  If there are fewer values, reduce the size of the
    * allocation.
    */
   if (result->num_values_to_emit < num_candidates) {
      result->values_to_emit = (struct combine_constants_value *)
         realloc(result->values_to_emit, sizeof(result->values_to_emit[0]) *
                 result->num_values_to_emit);

      /* Is it even possible for a reducing realloc to fail? */
      assert(result->values_to_emit != NULL);
   }

   /* Create the mapping from "combined" constants to list of candidates
    * passed in by the caller.
    */
   memset(remain, 0xff, BITSET_WORDS(num_candidates) * sizeof(remain[0]));

   unsigned total_users = 0;

   const unsigned num_values = result->num_values_to_emit;
   for (unsigned value_idx = 0; value_idx < num_values; value_idx++) {
      result->values_to_emit[value_idx].first_user = total_users;

      uint8_t reachable_mask;
      uint8_t unused_mask;

      calculate_masks(result->values_to_emit[value_idx].value, either_type,
                      result->values_to_emit[value_idx].bit_size,
                      &reachable_mask, &unused_mask);

      nir_const_value reachable_values[MAX_NUM_REACHABLE];

      calculate_reachable_values(result->values_to_emit[value_idx].value,
                                 result->values_to_emit[value_idx].bit_size,
                                 reachable_mask, reachable_values);

      for (unsigned i = 0; i < num_candidates; i++) {
         bool matched = false;

         if (!BITSET_TEST(remain, i))
            continue;

         if (candidates[i].bit_size != result->values_to_emit[value_idx].bit_size)
            continue;

         if (value_equal(candidates[i].value, result->values_to_emit[value_idx].value,
                         result->values_to_emit[value_idx].bit_size)) {
            result->user_map[total_users].index = i;
            result->user_map[total_users].type = candidates[i].type;
            result->user_map[total_users].negate = false;
            total_users++;

            matched = true;
            BITSET_CLEAR(remain, i);
         } else {
            const uint8_t combined_mask = reachable_mask &
                                          candidates[i].reaching_mask;

            enum interpreted_type type = either_type;

            if ((combined_mask & INT_NEG_EXISTS) &&
                value_equal(candidates[i].value,
                            reachable_values[INT_NEG_INDEX],
                            candidates[i].bit_size)) {
               type = integer_only;
            }

            if (type == either_type &&
                (combined_mask & FLOAT_NEG_EXISTS) &&
                value_equal(candidates[i].value,
                            reachable_values[FLOAT_NEG_INDEX],
                            candidates[i].bit_size)) {
               type = float_only;
            }

            if (type != either_type) {
               /* Finding a match on this path implies that the user must
                * allow source negations.
                */
               assert(!candidates[i].no_negations);

               result->user_map[total_users].index = i;
               result->user_map[total_users].type = type;
               result->user_map[total_users].negate = true;
               total_users++;

               matched = true;
               BITSET_CLEAR(remain, i);
            }
         }

         /* Mark the other source of instructions that can have a constant
          * source.  Selection is the prime example of this, and we want to
          * avoid generating sequences like bcsel(a, fneg(b), ineg(c)).
          *
          * This also makes sure that the assertion (below) that *all* values
          * were processed holds even when some values may be allowed to
          * remain as constants.
          *
          * FINISHME: There may be value in only doing this when type ==
          * either_type.  If both sources are loaded, a register allocator may
          * be able to make a better choice about which value to "spill"
          * (i.e., replace with an immediate) under heavy register pressure.
          */
         if (matched && candidates[i].allow_one_constant) {
            const struct value *const other_src = candidates[i].next_src;
            const unsigned idx = other_src - candidates;

            assert(idx < num_candidates);
            BITSET_CLEAR(remain, idx);
         }
      }

      assert(total_users > result->values_to_emit[value_idx].first_user);
      result->values_to_emit[value_idx].num_users =
         total_users - result->values_to_emit[value_idx].first_user;
   }

   /* Verify that all of the values were emitted by the loop above.  If any
    * bits are still set in remain, then some value was not emitted.  The use
    * of memset to populate remain prevents the use of a more performant loop.
    */
#ifndef NDEBUG
   bool pass = true;

   BITSET_FOREACH_SET(i, remain, num_candidates) {
      fprintf(stderr, "candidate %d was not processed: { "
              ".b = %s, "
              ".f32 = %f, .f64 = %g, "
              ".i8 = %d, .u8 = 0x%02x, "
              ".i16 = %d, .u16 = 0x%04x, "
              ".i32 = %d, .u32 = 0x%08x, "
              ".i64 = %" PRId64 ", .u64 = 0x%016" PRIx64 " }\n",
              i,
              candidates[i].value.b ? "true" : "false",
              candidates[i].value.f32, candidates[i].value.f64,
              candidates[i].value.i8,  candidates[i].value.u8,
              candidates[i].value.i16, candidates[i].value.u16,
              candidates[i].value.i32, candidates[i].value.u32,
              candidates[i].value.i64, candidates[i].value.u64);
      pass = false;
   }

   assert(pass && "All values should have been processed.");
#endif

   free(remain);

   return result;
}

static combine_constants_result *
brw_combine_constants(struct value *candidates, unsigned num_candidates)
{
   preprocess_candidates(candidates, num_candidates);

   return combine_constants_greedy(candidates, num_candidates);
}

/* Returns whether an instruction could co-issue if its immediate source were
 * replaced with a GRF source.
 */
static bool
could_coissue(const struct intel_device_info *devinfo, const fs_inst *inst)
{
   assert(inst->opcode == BRW_OPCODE_MOV ||
          inst->opcode == BRW_OPCODE_CMP ||
          inst->opcode == BRW_OPCODE_ADD ||
          inst->opcode == BRW_OPCODE_MUL);

   if (devinfo->ver != 7)
      return false;

   /* Only float instructions can coissue.  We don't have a great
    * understanding of whether or not something like float(int(a) + int(b))
    * would be considered float (based on the destination type) or integer
    * (based on the source types), so we take the conservative choice of
    * only promoting when both destination and source are float.
    */
   return inst->dst.type == BRW_REGISTER_TYPE_F &&
          inst->src[0].type == BRW_REGISTER_TYPE_F;
}

/**
 * Box for storing fs_inst and some other necessary data
 *
 * \sa box_instruction
 */
struct fs_inst_box {
   fs_inst *inst;
   unsigned ip;
   bblock_t *block;
   bool must_promote;
};

/** A box for putting fs_regs in a linked list. */
struct reg_link {
   DECLARE_RALLOC_CXX_OPERATORS(reg_link)

   reg_link(fs_inst *inst, unsigned src, bool negate, enum interpreted_type type)
   : inst(inst), src(src), negate(negate), type(type) {}

   struct exec_node link;
   fs_inst *inst;
   uint8_t src;
   bool negate;
   enum interpreted_type type;
};

static struct exec_node *
link(void *mem_ctx, fs_inst *inst, unsigned src, bool negate,
     enum interpreted_type type)
{
   reg_link *l = new(mem_ctx) reg_link(inst, src, negate, type);
   return &l->link;
}

/**
 * Information about an immediate value.
 */
struct imm {
   /** The common ancestor of all blocks using this immediate value. */
   bblock_t *block;

   /**
    * The instruction generating the immediate value, if all uses are contained
    * within a single basic block. Otherwise, NULL.
    */
   fs_inst *inst;

   /**
    * A list of fs_regs that refer to this immediate.  If we promote it, we'll
    * have to patch these up to refer to the new GRF.
    */
   exec_list *uses;

   /** The immediate value */
   union {
      char bytes[8];
      double df;
      int64_t d64;
      float f;
      int32_t d;
      int16_t w;
   };
   uint8_t size;

   /** When promoting half-float we need to account for certain restrictions */
   bool is_half_float;

   /**
    * The GRF register and subregister number where we've decided to store the
    * constant value.
    */
   uint8_t subreg_offset;
   uint16_t nr;

   /** The number of coissuable instructions using this immediate. */
   uint16_t uses_by_coissue;

   /**
    * Whether this constant is used by an instruction that can't handle an
    * immediate source (and already has to be promoted to a GRF).
    */
   bool must_promote;

   /** Is the value used only in a single basic block? */
   bool used_in_single_block;

   uint16_t first_use_ip;
   uint16_t last_use_ip;
};

/** The working set of information about immediates. */
struct table {
   struct value *values;
   int size;
   int num_values;

   struct imm *imm;
   int len;

   struct fs_inst_box *boxes;
   unsigned num_boxes;
   unsigned size_boxes;
};

static struct value *
new_value(struct table *table, void *mem_ctx)
{
   if (table->num_values == table->size) {
      table->size *= 2;
      table->values = reralloc(mem_ctx, table->values, struct value, table->size);
   }
   return &table->values[table->num_values++];
}

/**
 * Store an instruction with some other data in a table.
 *
 * \returns the index into the dynamic array of boxes for the instruction.
 */
static unsigned
box_instruction(struct table *table, void *mem_ctx, fs_inst *inst,
                unsigned ip, bblock_t *block, bool must_promote)
{
   /* It is common for box_instruction to be called consecutively for each
    * source of an instruction.  As a result, the most common case for finding
    * an instruction in the table is when that instruction was the last one
    * added.  Search the list back to front.
    */
   for (unsigned i = table->num_boxes; i > 0; /* empty */) {
      i--;

      if (table->boxes[i].inst == inst)
         return i;
   }

   if (table->num_boxes == table->size_boxes) {
      table->size_boxes *= 2;
      table->boxes = reralloc(mem_ctx, table->boxes, fs_inst_box,
                              table->size_boxes);
   }

   assert(table->num_boxes < table->size_boxes);

   const unsigned idx = table->num_boxes++;
   fs_inst_box *ib =  &table->boxes[idx];

   ib->inst = inst;
   ib->block = block;
   ib->ip = ip;
   ib->must_promote = must_promote;

   return idx;
}

/**
 * Comparator used for sorting an array of imm structures.
 *
 * We sort by basic block number, then last use IP, then first use IP (least
 * to greatest). This sorting causes immediates live in the same area to be
 * allocated to the same register in the hopes that all values will be dead
 * about the same time and the register can be reused.
 */
static int
compare(const void *_a, const void *_b)
{
   const struct imm *a = (const struct imm *)_a,
                    *b = (const struct imm *)_b;

   int block_diff = a->block->num - b->block->num;
   if (block_diff)
      return block_diff;

   int end_diff = a->last_use_ip - b->last_use_ip;
   if (end_diff)
      return end_diff;

   return a->first_use_ip - b->first_use_ip;
}

static struct brw_reg
build_imm_reg_for_copy(struct imm *imm)
{
   switch (imm->size) {
   case 8:
      return brw_imm_d(imm->d64);
   case 4:
      return brw_imm_d(imm->d);
   case 2:
      return brw_imm_w(imm->w);
   default:
      unreachable("not implemented");
   }
}

static inline uint32_t
get_alignment_for_imm(const struct imm *imm)
{
   if (imm->is_half_float)
      return 4; /* At least MAD seems to require this */
   else
      return imm->size;
}

static bool
representable_as_hf(float f, uint16_t *hf)
{
   union fi u;
   uint16_t h = _mesa_float_to_half(f);
   u.f = _mesa_half_to_float(h);

   if (u.f == f) {
      *hf = h;
      return true;
   }

   return false;
}

static bool
representable_as_w(int d, int16_t *w)
{
   int res = ((d & 0xffff8000) + 0x8000) & 0xffff7fff;
   if (!res) {
      *w = d;
      return true;
   }

   return false;
}

static bool
representable_as_uw(unsigned ud, uint16_t *uw)
{
   if (!(ud & 0xffff0000)) {
      *uw = ud;
      return true;
   }

   return false;
}

static bool
supports_src_as_imm(const struct intel_device_info *devinfo, const fs_inst *inst)
{
   if (devinfo->ver < 12)
      return false;

   switch (inst->opcode) {
   case BRW_OPCODE_ADD3:
      /* ADD3 only exists on Gfx12.5+. */
      return true;

   case BRW_OPCODE_MAD:
      /* Integer types can always mix sizes. Floating point types can mix
       * sizes on Gfx12. On Gfx12.5, floating point sources must all be HF or
       * all be F.
       */
      return devinfo->verx10 < 125 || inst->src[0].type != BRW_REGISTER_TYPE_F;

   default:
      return false;
   }
}

static bool
can_promote_src_as_imm(const struct intel_device_info *devinfo, fs_inst *inst,
                       unsigned src_idx)
{
   bool can_promote = false;

   /* Experiment shows that we can only support src0 as immediate for MAD on
    * Gfx12. ADD3 can use src0 or src2 in Gfx12.5, but constant propagation
    * only propagates into src0. It's possible that src2 works for W or UW MAD
    * on Gfx12.5.
    */
   if (src_idx != 0)
      return false;

   if (!supports_src_as_imm(devinfo, inst))
      return false;

   /* TODO - Fix the codepath below to use a bfloat16 immediate on XeHP,
    *        since HF/F mixed mode has been removed from the hardware.
    */
   switch (inst->src[src_idx].type) {
   case BRW_REGISTER_TYPE_F: {
      uint16_t hf;
      if (representable_as_hf(inst->src[src_idx].f, &hf)) {
         inst->src[src_idx] = retype(brw_imm_uw(hf), BRW_REGISTER_TYPE_HF);
         can_promote = true;
      }
      break;
   }
   case BRW_REGISTER_TYPE_D: {
      int16_t w;
      if (representable_as_w(inst->src[src_idx].d, &w)) {
         inst->src[src_idx] = brw_imm_w(w);
         can_promote = true;
      }
      break;
   }
   case BRW_REGISTER_TYPE_UD: {
      uint16_t uw;
      if (representable_as_uw(inst->src[src_idx].ud, &uw)) {
         inst->src[src_idx] = brw_imm_uw(uw);
         can_promote = true;
      }
      break;
   }
   case BRW_REGISTER_TYPE_W:
   case BRW_REGISTER_TYPE_UW:
   case BRW_REGISTER_TYPE_HF:
      can_promote = true;
      break;
   default:
      break;
   }

   return can_promote;
}

static void
add_candidate_immediate(struct table *table, fs_inst *inst, unsigned ip,
                        unsigned i,
                        bool must_promote,
                        bool allow_one_constant,
                        bblock_t *block,
                        const struct intel_device_info *devinfo,
                        void *const_ctx)
{
   struct value *v = new_value(table, const_ctx);

   unsigned box_idx = box_instruction(table, const_ctx, inst, ip, block,
                                      must_promote);

   v->value.u64 = inst->src[i].d64;
   v->bit_size = 8 * type_sz(inst->src[i].type);
   v->instr_index = box_idx;
   v->src = i;
   v->allow_one_constant = allow_one_constant;

   /* Right-shift instructions are special.  They can have source modifiers,
    * but changing the type can change the semantic of the instruction.  Only
    * allow negations on a right shift if the source type is already signed.
    */
   v->no_negations = !inst->can_do_source_mods(devinfo) ||
                     ((inst->opcode == BRW_OPCODE_SHR ||
                       inst->opcode == BRW_OPCODE_ASR) &&
                      brw_reg_type_is_unsigned_integer(inst->src[i].type));

   switch (inst->src[i].type) {
   case BRW_REGISTER_TYPE_DF:
   case BRW_REGISTER_TYPE_NF:
   case BRW_REGISTER_TYPE_F:
   case BRW_REGISTER_TYPE_HF:
      v->type = float_only;
      break;

   case BRW_REGISTER_TYPE_UQ:
   case BRW_REGISTER_TYPE_Q:
   case BRW_REGISTER_TYPE_UD:
   case BRW_REGISTER_TYPE_D:
   case BRW_REGISTER_TYPE_UW:
   case BRW_REGISTER_TYPE_W:
      v->type = integer_only;
      break;

   case BRW_REGISTER_TYPE_VF:
   case BRW_REGISTER_TYPE_UV:
   case BRW_REGISTER_TYPE_V:
   case BRW_REGISTER_TYPE_UB:
   case BRW_REGISTER_TYPE_B:
   default:
      unreachable("not reached");
   }

   /* It is safe to change the type of the operands of a select instruction
    * that has no conditional modifier, no source modifiers, and no saturate
    * modifer.
    */
   if (inst->opcode == BRW_OPCODE_SEL &&
       inst->conditional_mod == BRW_CONDITIONAL_NONE &&
       !inst->src[0].negate && !inst->src[0].abs &&
       !inst->src[1].negate && !inst->src[1].abs &&
       !inst->saturate) {
      v->type = either_type;
   }
}

struct register_allocation {
   /** VGRF for storing values. */
   unsigned nr;

   /**
    * Mask of currently available slots in this register.
    *
    * Each register is 16, 16-bit slots.  Allocations require 1, 2, or 4 slots
    * for word, double-word, or quad-word values, respectively.
    */
   uint16_t avail;
};

static fs_reg
allocate_slots(struct register_allocation *regs, unsigned num_regs,
               unsigned bytes, unsigned align_bytes,
               brw::simple_allocator &alloc)
{
   assert(bytes == 2 || bytes == 4 || bytes == 8);
   assert(align_bytes == 2 || align_bytes == 4 || align_bytes == 8);

   const unsigned words = bytes / 2;
   const unsigned align_words = align_bytes / 2;
   const uint16_t mask = (1U << words) - 1;

   for (unsigned i = 0; i < num_regs; i++) {
      for (unsigned j = 0; j <= (16 - words); j += align_words) {
         const uint16_t x = regs[i].avail >> j;

         if ((x & mask) == mask) {
            if (regs[i].nr == UINT_MAX)
               regs[i].nr = alloc.allocate(1);

            regs[i].avail &= ~(mask << j);

            fs_reg reg(VGRF, regs[i].nr);
            reg.offset = j * 2;

            return reg;
         }
      }
   }

   unreachable("No free slots found.");
}

static void
deallocate_slots(struct register_allocation *regs, unsigned num_regs,
                 unsigned reg_nr, unsigned subreg_offset, unsigned bytes)
{
   assert(bytes == 2 || bytes == 4 || bytes == 8);
   assert(subreg_offset % 2 == 0);
   assert(subreg_offset + bytes <= 32);

   const unsigned words = bytes / 2;
   const unsigned offset = subreg_offset / 2;
   const uint16_t mask = ((1U << words) - 1) << offset;

   for (unsigned i = 0; i < num_regs; i++) {
      if (regs[i].nr == reg_nr) {
         regs[i].avail |= mask;
         return;
      }
   }

   unreachable("No such register found.");
}

static void
parcel_out_registers(struct imm *imm, unsigned len, const bblock_t *cur_block,
                     struct register_allocation *regs, unsigned num_regs,
                     brw::simple_allocator &alloc, unsigned ver)
{
   /* Each basic block has two distinct set of constants.  There is the set of
    * constants that only have uses in that block, and there is the set of
    * constants that have uses after that block.
    *
    * Allocation proceeds in three passes.
    *
    * 1. Allocate space for the values that are used outside this block.
    *
    * 2. Allocate space for the values that are used only in this block.
    *
    * 3. Deallocate the space for the values that are used only in this block.
    */

   for (unsigned pass = 0; pass < 2; pass++) {
      const bool used_in_single_block = pass != 0;

      for (unsigned i = 0; i < len; i++) {
         if (imm[i].block == cur_block &&
             imm[i].used_in_single_block == used_in_single_block) {
            /* From the BDW and CHV PRM, 3D Media GPGPU, Special Restrictions:
             *
             *   "In Align16 mode, the channel selects and channel enables apply
             *    to a pair of half-floats, because these parameters are defined
             *    for DWord elements ONLY. This is applicable when both source
             *    and destination are half-floats."
             *
             * This means that Align16 instructions that use promoted HF
             * immediates and use a <0,1,0>:HF region would read 2 HF slots
             * instead of replicating the single one we want. To avoid this, we
             * always populate both HF slots within a DWord with the constant.
             */
            const unsigned width = ver == 8 && imm[i].is_half_float ? 2 : 1;

            const fs_reg reg = allocate_slots(regs, num_regs,
                                              imm[i].size * width,
                                              get_alignment_for_imm(&imm[i]),
                                              alloc);

            imm[i].nr = reg.nr;
            imm[i].subreg_offset = reg.offset;
         }
      }
   }

   for (unsigned i = 0; i < len; i++) {
      if (imm[i].block == cur_block && imm[i].used_in_single_block) {
         const unsigned width = ver == 8 && imm[i].is_half_float ? 2 : 1;

         deallocate_slots(regs, num_regs, imm[i].nr, imm[i].subreg_offset,
                          imm[i].size * width);
      }
   }
}

bool
fs_visitor::opt_combine_constants()
{
   void *const_ctx = ralloc_context(NULL);

   struct table table;

   /* For each of the dynamic arrays in the table, allocate about a page of
    * memory.  On LP64 systems, this gives 126 value objects 169 fs_inst_box
    * objects.  Even larger shaders that have been obverved rarely need more
    * than 20 or 30 values.  Most smaller shaders, which is most shaders, need
    * at most a couple dozen fs_inst_box.
    */
   table.size = (4096 - (5 * sizeof(void *))) / sizeof(struct value);
   table.num_values = 0;
   table.values = ralloc_array(const_ctx, struct value, table.size);

   table.size_boxes = (4096 - (5 * sizeof(void *))) / sizeof(struct fs_inst_box);
   table.num_boxes = 0;
   table.boxes = ralloc_array(const_ctx, fs_inst_box, table.size_boxes);

   const brw::idom_tree &idom = idom_analysis.require();
   unsigned ip = -1;

   /* Make a pass through all instructions and count the number of times each
    * constant is used by coissueable instructions or instructions that cannot
    * take immediate arguments.
    */
   foreach_block_and_inst(block, fs_inst, inst, cfg) {
      ip++;

      switch (inst->opcode) {
      case SHADER_OPCODE_INT_QUOTIENT:
      case SHADER_OPCODE_INT_REMAINDER:
      case SHADER_OPCODE_POW:
         if (inst->src[0].file == IMM) {
            assert(inst->opcode != SHADER_OPCODE_POW);

            add_candidate_immediate(&table, inst, ip, 0, true, false, block,
                                    devinfo, const_ctx);
         }

         if (inst->src[1].file == IMM && devinfo->ver < 8) {
            add_candidate_immediate(&table, inst, ip, 1, true, false, block,
                                    devinfo, const_ctx);
         }

         break;

      case BRW_OPCODE_ADD3:
      case BRW_OPCODE_MAD: {
         for (int i = 0; i < inst->sources; i++) {
            if (inst->src[i].file != IMM)
               continue;

            if (can_promote_src_as_imm(devinfo, inst, i))
               continue;

            add_candidate_immediate(&table, inst, ip, i, true, false, block,
                                    devinfo, const_ctx);
         }

         break;
      }

      case BRW_OPCODE_BFE:
      case BRW_OPCODE_BFI2:
      case BRW_OPCODE_LRP:
         for (int i = 0; i < inst->sources; i++) {
            if (inst->src[i].file != IMM)
               continue;

            add_candidate_immediate(&table, inst, ip, i, true, false, block,
                                    devinfo, const_ctx);
         }

         break;

      case BRW_OPCODE_SEL:
         if (inst->src[0].file == IMM) {
            /* It is possible to have src0 be immediate but src1 not be
             * immediate for the non-commutative conditional modifiers (e.g.,
             * G).
             */
            if (inst->conditional_mod == BRW_CONDITIONAL_NONE ||
                /* Only GE and L are commutative. */
                inst->conditional_mod == BRW_CONDITIONAL_GE ||
                inst->conditional_mod == BRW_CONDITIONAL_L) {
               assert(inst->src[1].file == IMM);

               add_candidate_immediate(&table, inst, ip, 0, true, true, block,
                                       devinfo, const_ctx);
               add_candidate_immediate(&table, inst, ip, 1, true, true, block,
                                       devinfo, const_ctx);
            } else {
               add_candidate_immediate(&table, inst, ip, 0, true, false, block,
                                       devinfo, const_ctx);
            }
         }
         break;

      case BRW_OPCODE_ASR:
      case BRW_OPCODE_BFI1:
      case BRW_OPCODE_ROL:
      case BRW_OPCODE_ROR:
      case BRW_OPCODE_SHL:
      case BRW_OPCODE_SHR:
         if (inst->src[0].file == IMM) {
            add_candidate_immediate(&table, inst, ip, 0, true, false, block,
                                    devinfo, const_ctx);
         }
         break;

      case BRW_OPCODE_MOV:
         if (could_coissue(devinfo, inst) && inst->src[0].file == IMM) {
            add_candidate_immediate(&table, inst, ip, 0, false, false, block,
                                    devinfo, const_ctx);
         }
         break;

      case BRW_OPCODE_CMP:
      case BRW_OPCODE_ADD:
      case BRW_OPCODE_MUL:
         assert(inst->src[0].file != IMM);

         if (could_coissue(devinfo, inst) && inst->src[1].file == IMM) {
            add_candidate_immediate(&table, inst, ip, 1, false, false, block,
                                    devinfo, const_ctx);
         }
         break;

      default:
         break;
      }
   }

   if (table.num_values == 0) {
      ralloc_free(const_ctx);
      return false;
   }

   combine_constants_result *result =
      brw_combine_constants(table.values, table.num_values);

   table.imm = ralloc_array(const_ctx, struct imm, result->num_values_to_emit);
   table.len = 0;

   for (unsigned i = 0; i < result->num_values_to_emit; i++) {
      struct imm *imm = &table.imm[table.len];

      imm->block = NULL;
      imm->inst = NULL;
      imm->d64 = result->values_to_emit[i].value.u64;
      imm->size = result->values_to_emit[i].bit_size / 8;

      imm->uses_by_coissue = 0;
      imm->must_promote = false;
      imm->is_half_float = false;

      imm->first_use_ip = UINT16_MAX;
      imm->last_use_ip = 0;

      imm->uses = new(const_ctx) exec_list;

      const unsigned first_user = result->values_to_emit[i].first_user;
      const unsigned last_user = first_user +
         result->values_to_emit[i].num_users;

      for (unsigned j = first_user; j < last_user; j++) {
         const unsigned idx = table.values[result->user_map[j].index].instr_index;
         fs_inst_box *const ib = &table.boxes[idx];

         const unsigned src = table.values[result->user_map[j].index].src;

         imm->uses->push_tail(link(const_ctx, ib->inst, src,
                                   result->user_map[j].negate,
                                   result->user_map[j].type));

         if (ib->must_promote)
            imm->must_promote = true;
         else
            imm->uses_by_coissue++;

         if (imm->block == NULL) {
            /* Block should only be NULL on the first pass.  On the first
             * pass, inst should also be NULL.
             */
            assert(imm->inst == NULL);

            imm->inst = ib->inst;
            imm->block = ib->block;
            imm->first_use_ip = ib->ip;
            imm->last_use_ip = ib->ip;
            imm->used_in_single_block = true;
         } else {
            bblock_t *intersection = idom.intersect(ib->block,
                                                    imm->block);

            if (ib->block != imm->block)
               imm->used_in_single_block = false;

            if (imm->first_use_ip > ib->ip) {
               imm->first_use_ip = ib->ip;

               /* If the first-use instruction is to be tracked, block must be
                * the block that contains it.  The old block was read in the
                * idom.intersect call above, so it is safe to overwrite it
                * here.
                */
               imm->inst = ib->inst;
               imm->block = ib->block;
            }

            if (imm->last_use_ip < ib->ip)
               imm->last_use_ip = ib->ip;

            /* The common dominator is not the block that contains the
             * first-use instruction, so don't track that instruction.  The
             * load instruction will be added in the common dominator block
             * instead of before the first-use instruction.
             */
            if (intersection != imm->block)
               imm->inst = NULL;

            imm->block = intersection;
         }

         if (ib->inst->src[src].type == BRW_REGISTER_TYPE_HF)
            imm->is_half_float = true;
      }

      /* Remove constants from the table that don't have enough uses to make
       * them profitable to store in a register.
       */
      if (imm->must_promote || imm->uses_by_coissue >= 4)
         table.len++;
   }

   delete result;

   if (table.len == 0) {
      ralloc_free(const_ctx);
      return false;
   }
   if (cfg->num_blocks != 1)
      qsort(table.imm, table.len, sizeof(struct imm), compare);

   if (devinfo->ver > 7) {
      struct register_allocation *regs =
         (struct register_allocation *) calloc(table.len, sizeof(regs[0]));

      for (int i = 0; i < table.len; i++) {
         regs[i].nr = UINT_MAX;
         regs[i].avail = 0xffff;
      }

      foreach_block(block, cfg) {
         parcel_out_registers(table.imm, table.len, block, regs, table.len,
                              alloc, devinfo->ver);
      }

      free(regs);
   } else {
      fs_reg reg(VGRF, alloc.allocate(1));
      reg.stride = 0;

      for (int i = 0; i < table.len; i++) {
         struct imm *imm = &table.imm[i];

         /* Put the immediate in an offset aligned to its size. Some
          * instructions seem to have additional alignment requirements, so
          * account for that too.
          */
         reg.offset = ALIGN(reg.offset, get_alignment_for_imm(imm));

         /* Ensure we have enough space in the register to copy the immediate */
         if (reg.offset + imm->size > REG_SIZE) {
            reg.nr = alloc.allocate(1);
            reg.offset = 0;
         }

         imm->nr = reg.nr;
         imm->subreg_offset = reg.offset;

         reg.offset += imm->size;
      }
   }

   bool rebuild_cfg = false;

   /* Insert MOVs to load the constant values into GRFs. */
   for (int i = 0; i < table.len; i++) {
      struct imm *imm = &table.imm[i];

      /* Insert it either before the instruction that generated the immediate
       * or after the last non-control flow instruction of the common ancestor.
       */
      exec_node *n;
      bblock_t *insert_block;
      if (imm->inst != nullptr) {
         n = imm->inst;
         insert_block = imm->block;
      } else {
         if (imm->block->start()->opcode == BRW_OPCODE_DO) {
            /* DO blocks are weird. They can contain only the single DO
             * instruction. As a result, MOV instructions cannot be added to
             * the DO block.
             */
            bblock_t *next_block = imm->block->next();
            if (next_block->starts_with_control_flow()) {
               /* This is the difficult case. This occurs for code like
                *
                *    do {
                *       do {
                *          ...
                *       } while (...);
                *    } while (...);
                *
                * when the MOV instructions need to be inserted between the
                * two DO instructions.
                *
                * To properly handle this scenario, a new block would need to
                * be inserted. Doing so would require modifying arbitrary many
                * CONTINUE, BREAK, and WHILE instructions to point to the new
                * block.
                *
                * It is unlikely that this would ever be correct. Instead,
                * insert the MOV instructions in the known wrong place and
                * rebuild the CFG at the end of the pass.
                */
               insert_block = imm->block;
               n = insert_block->last_non_control_flow_inst()->next;

               rebuild_cfg = true;
            } else {
               insert_block = next_block;
               n = insert_block->start();
            }
         } else {
            insert_block = imm->block;
            n = insert_block->last_non_control_flow_inst()->next;
         }
      }

      /* From the BDW and CHV PRM, 3D Media GPGPU, Special Restrictions:
       *
       *   "In Align16 mode, the channel selects and channel enables apply to a
       *    pair of half-floats, because these parameters are defined for DWord
       *    elements ONLY. This is applicable when both source and destination
       *    are half-floats."
       *
       * This means that Align16 instructions that use promoted HF immediates
       * and use a <0,1,0>:HF region would read 2 HF slots instead of
       * replicating the single one we want. To avoid this, we always populate
       * both HF slots within a DWord with the constant.
       */
      const uint32_t width = devinfo->ver == 8 && imm->is_half_float ? 2 : 1;
      const fs_builder ibld = fs_builder(this, width).at(insert_block, n).exec_all();

      fs_reg reg(VGRF, imm->nr);
      reg.offset = imm->subreg_offset;
      reg.stride = 0;

      /* Put the immediate in an offset aligned to its size. Some instructions
       * seem to have additional alignment requirements, so account for that
       * too.
       */
      assert(reg.offset == ALIGN(reg.offset, get_alignment_for_imm(imm)));

      struct brw_reg imm_reg = build_imm_reg_for_copy(imm);

      /* Ensure we have enough space in the register to copy the immediate */
      assert(reg.offset + type_sz(imm_reg.type) * width <= REG_SIZE);

      ibld.MOV(retype(reg, imm_reg.type), imm_reg);
   }
   shader_stats.promoted_constants = table.len;

   /* Rewrite the immediate sources to refer to the new GRFs. */
   for (int i = 0; i < table.len; i++) {
      foreach_list_typed(reg_link, link, link, table.imm[i].uses) {
         fs_reg *reg = &link->inst->src[link->src];

         if (link->inst->opcode == BRW_OPCODE_SEL) {
            if (link->type == either_type) {
               /* Do not change the register type. */
            } else if (link->type == integer_only) {
               reg->type = brw_int_type(type_sz(reg->type), true);
            } else {
               assert(link->type == float_only);

               switch (type_sz(reg->type)) {
               case 2:
                  reg->type = BRW_REGISTER_TYPE_HF;
                  break;
               case 4:
                  reg->type = BRW_REGISTER_TYPE_F;
                  break;
               case 8:
                  reg->type = BRW_REGISTER_TYPE_DF;
                  break;
               default:
                  unreachable("Bad type size");
               }
            }
         } else if ((link->inst->opcode == BRW_OPCODE_SHL ||
                     link->inst->opcode == BRW_OPCODE_ASR) &&
                    link->negate) {
            reg->type = brw_int_type(type_sz(reg->type), true);
         }

#ifdef DEBUG
         switch (reg->type) {
         case BRW_REGISTER_TYPE_DF:
            assert((isnan(reg->df) && isnan(table.imm[i].df)) ||
                   (fabs(reg->df) == fabs(table.imm[i].df)));
            break;
         case BRW_REGISTER_TYPE_F:
            assert((isnan(reg->f) && isnan(table.imm[i].f)) ||
                   (fabsf(reg->f) == fabsf(table.imm[i].f)));
            break;
         case BRW_REGISTER_TYPE_HF:
            assert((isnan(_mesa_half_to_float(reg->d & 0xffffu)) &&
                    isnan(_mesa_half_to_float(table.imm[i].w))) ||
                   (fabsf(_mesa_half_to_float(reg->d & 0xffffu)) ==
                    fabsf(_mesa_half_to_float(table.imm[i].w))));
            break;
         case BRW_REGISTER_TYPE_Q:
            assert(abs(reg->d64) == abs(table.imm[i].d64));
            break;
         case BRW_REGISTER_TYPE_UQ:
            assert(!link->negate);
            assert(reg->d64 == table.imm[i].d64);
            break;
         case BRW_REGISTER_TYPE_D:
            assert(abs(reg->d) == abs(table.imm[i].d));
            break;
         case BRW_REGISTER_TYPE_UD:
            assert(!link->negate);
            assert(reg->d == table.imm[i].d);
            break;
         case BRW_REGISTER_TYPE_W:
            assert(abs((int16_t) (reg->d & 0xffff)) == table.imm[i].w);
            break;
         case BRW_REGISTER_TYPE_UW:
            assert(!link->negate);
            assert((reg->ud & 0xffffu) == (uint16_t) table.imm[i].w);
            break;
         default:
            break;
         }
#endif

         assert(link->inst->can_do_source_mods(devinfo) || !link->negate);

         reg->file = VGRF;
         reg->offset = table.imm[i].subreg_offset;
         reg->stride = 0;
         reg->negate = link->negate;
         reg->nr = table.imm[i].nr;
      }
   }

   /* Fixup any SEL instructions that have src0 still as an immediate.  Fixup
    * the types of any SEL instruction that have a negation on one of the
    * sources.  Adding the negation may have changed the type of that source,
    * so the other source (and destination) must be changed to match.
    */
   for (unsigned i = 0; i < table.num_boxes; i++) {
      fs_inst *inst = table.boxes[i].inst;

      if (inst->opcode != BRW_OPCODE_SEL)
         continue;

      /* If both sources have negation, the types had better be the same! */
      assert(!inst->src[0].negate || !inst->src[1].negate ||
             inst->src[0].type == inst->src[1].type);

      /* If either source has a negation, force the type of the other source
       * and the type of the result to be the same.
       */
      if (inst->src[0].negate) {
         inst->src[1].type = inst->src[0].type;
         inst->dst.type = inst->src[0].type;
      }

      if (inst->src[1].negate) {
         inst->src[0].type = inst->src[1].type;
         inst->dst.type = inst->src[1].type;
      }

      if (inst->src[0].file != IMM)
         continue;

      assert(inst->src[1].file != IMM);
      assert(inst->conditional_mod == BRW_CONDITIONAL_NONE ||
             inst->conditional_mod == BRW_CONDITIONAL_GE ||
             inst->conditional_mod == BRW_CONDITIONAL_L);

      fs_reg temp = inst->src[0];
      inst->src[0] = inst->src[1];
      inst->src[1] = temp;

      /* If this was predicated, flipping operands means we also need to flip
       * the predicate.
       */
      if (inst->conditional_mod == BRW_CONDITIONAL_NONE)
         inst->predicate_inverse = !inst->predicate_inverse;
   }

   if (debug) {
      for (int i = 0; i < table.len; i++) {
         struct imm *imm = &table.imm[i];

         fprintf(stderr,
                 "0x%016" PRIx64 " - block %3d, reg %3d sub %2d, "
                 "Uses: (%2d, %2d), IP: %4d to %4d, length %4d\n",
                 (uint64_t)(imm->d & BITFIELD64_MASK(imm->size * 8)),
                 imm->block->num,
                 imm->nr,
                 imm->subreg_offset,
                 imm->must_promote,
                 imm->uses_by_coissue,
                 imm->first_use_ip,
                 imm->last_use_ip,
                 imm->last_use_ip - imm->first_use_ip);
      }
   }

   if (rebuild_cfg) {
      /* When the CFG is initially built, the instructions are removed from
       * the list of instructions stored in fs_visitor -- the same exec_node
       * is used for membership in that list and in a block list.  So we need
       * to pull them back before rebuilding the CFG.
       */
      assert(exec_list_length(&instructions) == 0);
      foreach_block(block, cfg) {
         exec_list_append(&instructions, &block->instructions);
      }

      delete cfg;
      cfg = NULL;
      calculate_cfg();
   }

   ralloc_free(const_ctx);

   invalidate_analysis(DEPENDENCY_INSTRUCTIONS | DEPENDENCY_VARIABLES |
                       (rebuild_cfg ? DEPENDENCY_BLOCKS : DEPENDENCY_NOTHING));

   return true;
}
