/*
 * Copyright © 2017 Intel Corporation
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

#include "util/u_math.h"
#include "nir.h"
#include "nir_builder.h"

/**
 * \file nir_opt_intrinsics.c
 */

static nir_intrinsic_instr *
lower_subgroups_64bit_split_intrinsic(nir_builder *b, nir_intrinsic_instr *intrin,
                                      unsigned int component)
{
   nir_def *comp;
   if (component == 0)
      comp = nir_unpack_64_2x32_split_x(b, intrin->src[0].ssa);
   else
      comp = nir_unpack_64_2x32_split_y(b, intrin->src[0].ssa);

   nir_intrinsic_instr *intr = nir_intrinsic_instr_create(b->shader, intrin->intrinsic);
   nir_def_init(&intr->instr, &intr->def, 1, 32);
   intr->const_index[0] = intrin->const_index[0];
   intr->const_index[1] = intrin->const_index[1];
   intr->src[0] = nir_src_for_ssa(comp);
   if (nir_intrinsic_infos[intrin->intrinsic].num_srcs == 2)
      intr->src[1] = nir_src_for_ssa(intrin->src[1].ssa);

   intr->num_components = 1;
   nir_builder_instr_insert(b, &intr->instr);
   return intr;
}

static nir_def *
lower_subgroup_op_to_32bit(nir_builder *b, nir_intrinsic_instr *intrin)
{
   assert(intrin->src[0].ssa->bit_size == 64);
   nir_intrinsic_instr *intr_x = lower_subgroups_64bit_split_intrinsic(b, intrin, 0);
   nir_intrinsic_instr *intr_y = lower_subgroups_64bit_split_intrinsic(b, intrin, 1);
   return nir_pack_64_2x32_split(b, &intr_x->def, &intr_y->def);
}

static nir_def *
ballot_type_to_uint(nir_builder *b, nir_def *value,
                    const nir_lower_subgroups_options *options)
{
   /* Only the new-style SPIR-V subgroup instructions take a ballot result as
    * an argument, so we only use this on uvec4 types.
    */
   assert(value->num_components == 4 && value->bit_size == 32);

   return nir_extract_bits(b, &value, 1, 0, options->ballot_components,
                           options->ballot_bit_size);
}

static nir_def *
uint_to_ballot_type(nir_builder *b, nir_def *value,
                    unsigned num_components, unsigned bit_size)
{
   assert(util_is_power_of_two_nonzero(num_components));
   assert(util_is_power_of_two_nonzero(value->num_components));

   unsigned total_bits = bit_size * num_components;

   /* If the source doesn't have enough bits, zero-pad */
   if (total_bits > value->bit_size * value->num_components)
      value = nir_pad_vector_imm_int(b, value, 0, total_bits / value->bit_size);

   value = nir_bitcast_vector(b, value, bit_size);

   /* If the source has too many components, truncate.  This can happen if,
    * for instance, we're implementing GL_ARB_shader_ballot or
    * VK_EXT_shader_subgroup_ballot which have 64-bit ballot values on an
    * architecture with a native 128-bit uvec4 ballot.  This comes up in Zink
    * for OpenGL on Vulkan.  It's the job of the driver calling this lowering
    * pass to ensure that it's restricted subgroup sizes sufficiently that we
    * have enough ballot bits.
    */
   if (value->num_components > num_components)
      value = nir_trim_vector(b, value, num_components);

   return value;
}

static nir_def *
lower_subgroup_op_to_scalar(nir_builder *b, nir_intrinsic_instr *intrin)
{
   /* This is safe to call on scalar things but it would be silly */
   assert(intrin->def.num_components > 1);

   nir_def *value = intrin->src[0].ssa;
   nir_def *reads[NIR_MAX_VEC_COMPONENTS];

   for (unsigned i = 0; i < intrin->num_components; i++) {
      nir_intrinsic_instr *chan_intrin =
         nir_intrinsic_instr_create(b->shader, intrin->intrinsic);
      nir_def_init(&chan_intrin->instr, &chan_intrin->def, 1,
                   intrin->def.bit_size);
      chan_intrin->num_components = 1;

      /* value */
      chan_intrin->src[0] = nir_src_for_ssa(nir_channel(b, value, i));
      /* invocation */
      if (nir_intrinsic_infos[intrin->intrinsic].num_srcs > 1) {
         assert(nir_intrinsic_infos[intrin->intrinsic].num_srcs == 2);
         chan_intrin->src[1] = nir_src_for_ssa(intrin->src[1].ssa);
      }

      chan_intrin->const_index[0] = intrin->const_index[0];
      chan_intrin->const_index[1] = intrin->const_index[1];

      nir_builder_instr_insert(b, &chan_intrin->instr);
      reads[i] = &chan_intrin->def;
   }

   return nir_vec(b, reads, intrin->num_components);
}

static nir_def *
lower_vote_eq_to_scalar(nir_builder *b, nir_intrinsic_instr *intrin)
{
   nir_def *value = intrin->src[0].ssa;

   nir_def *result = NULL;
   for (unsigned i = 0; i < intrin->num_components; i++) {
      nir_def* chan = nir_channel(b, value, i);

      if (intrin->intrinsic == nir_intrinsic_vote_feq) {
         chan = nir_vote_feq(b, intrin->def.bit_size, chan);
      } else {
         chan = nir_vote_ieq(b, intrin->def.bit_size, chan);
      }

      if (result) {
         result = nir_iand(b, result, chan);
      } else {
         result = chan;
      }
   }

   return result;
}

static nir_def *
lower_vote_eq(nir_builder *b, nir_intrinsic_instr *intrin)
{
   nir_def *value = intrin->src[0].ssa;

   /* We have to implicitly lower to scalar */
   nir_def *all_eq = NULL;
   for (unsigned i = 0; i < intrin->num_components; i++) {
      nir_def *rfi = nir_read_first_invocation(b, nir_channel(b, value, i));

      nir_def *is_eq;
      if (intrin->intrinsic == nir_intrinsic_vote_feq) {
         is_eq = nir_feq(b, rfi, nir_channel(b, value, i));
      } else {
         is_eq = nir_ieq(b, rfi, nir_channel(b, value, i));
      }

      if (all_eq == NULL) {
         all_eq = is_eq;
      } else {
         all_eq = nir_iand(b, all_eq, is_eq);
      }
   }

   return nir_vote_all(b, 1, all_eq);
}

static nir_def *
lower_shuffle_to_swizzle(nir_builder *b, nir_intrinsic_instr *intrin)
{
   unsigned mask = nir_src_as_uint(intrin->src[1]);

   if (mask >= 32)
      return NULL;

   return nir_masked_swizzle_amd(b, intrin->src[0].ssa,
                                 .swizzle_mask = (mask << 10) | 0x1f,
                                 .fetch_inactive = true);
}

/* Lowers "specialized" shuffles to a generic nir_intrinsic_shuffle. */

static nir_def *
lower_to_shuffle(nir_builder *b, nir_intrinsic_instr *intrin,
                 const nir_lower_subgroups_options *options)
{
   if (intrin->intrinsic == nir_intrinsic_shuffle_xor &&
       options->lower_shuffle_to_swizzle_amd &&
       nir_src_is_const(intrin->src[1])) {

      nir_def *result = lower_shuffle_to_swizzle(b, intrin);
      if (result)
         return result;
   }

   nir_def *index = nir_load_subgroup_invocation(b);
   switch (intrin->intrinsic) {
   case nir_intrinsic_shuffle_xor:
      index = nir_ixor(b, index, intrin->src[1].ssa);
      break;
   case nir_intrinsic_shuffle_up:
      index = nir_isub(b, index, intrin->src[1].ssa);
      break;
   case nir_intrinsic_shuffle_down:
      index = nir_iadd(b, index, intrin->src[1].ssa);
      break;
   case nir_intrinsic_quad_broadcast:
      index = nir_ior(b, nir_iand_imm(b, index, ~0x3),
                      intrin->src[1].ssa);
      break;
   case nir_intrinsic_quad_swap_horizontal:
      /* For Quad operations, subgroups are divided into quads where
       * (invocation % 4) is the index to a square arranged as follows:
       *
       *    +---+---+
       *    | 0 | 1 |
       *    +---+---+
       *    | 2 | 3 |
       *    +---+---+
       */
      index = nir_ixor(b, index, nir_imm_int(b, 0x1));
      break;
   case nir_intrinsic_quad_swap_vertical:
      index = nir_ixor(b, index, nir_imm_int(b, 0x2));
      break;
   case nir_intrinsic_quad_swap_diagonal:
      index = nir_ixor(b, index, nir_imm_int(b, 0x3));
      break;
   case nir_intrinsic_rotate: {
      nir_def *delta = intrin->src[1].ssa;
      nir_def *local_id = nir_load_subgroup_invocation(b);
      const unsigned cluster_size = nir_intrinsic_cluster_size(intrin);

      nir_def *rotation_group_mask =
         cluster_size > 0 ? nir_imm_int(b, (int)(cluster_size - 1)) : nir_iadd_imm(b, nir_load_subgroup_size(b), -1);

      index = nir_iand(b, nir_iadd(b, local_id, delta),
                       rotation_group_mask);
      if (cluster_size > 0) {
         index = nir_iadd(b, index,
                          nir_iand(b, local_id, nir_inot(b, rotation_group_mask)));
      }
      break;
   }
   default:
      unreachable("Invalid intrinsic");
   }

   return nir_shuffle(b, intrin->src[0].ssa, index);
}

static const struct glsl_type *
glsl_type_for_ssa(nir_def *def)
{
   const struct glsl_type *comp_type = def->bit_size == 1 ? glsl_bool_type() : glsl_uintN_t_type(def->bit_size);
   return glsl_replace_vector_type(comp_type, def->num_components);
}

/* Lower nir_intrinsic_shuffle to a waterfall loop + nir_read_invocation.
 */
static nir_def *
lower_shuffle(nir_builder *b, nir_intrinsic_instr *intrin)
{
   nir_def *val = intrin->src[0].ssa;
   nir_def *id = intrin->src[1].ssa;

   /* The loop is something like:
    *
    * while (true) {
    *    first_id = readFirstInvocation(gl_SubgroupInvocationID);
    *    first_val = readFirstInvocation(val);
    *    first_result = readInvocation(val, readFirstInvocation(id));
    *    if (id == first_id)
    *       result = first_val;
    *    if (elect()) {
    *       if (id > gl_SubgroupInvocationID) {
    *          result = first_result;
    *       }
    *       break;
    *    }
    * }
    *
    * The idea is to guarantee, on each iteration of the loop, that anything
    * reading from first_id gets the correct value, so that we can then kill
    * it off by breaking out of the loop. Before doing that we also have to
    * ensure that first_id invocation gets the correct value. It only won't be
    * assigned the correct value already if the invocation it's reading from
    * isn't already killed off, that is, if it's later than its own ID.
    * Invocations where id <= gl_SubgroupInvocationID will be assigned their
    * result in the first if, and invocations where id >
    * gl_SubgroupInvocationID will be assigned their result in the second if.
    *
    * We do this more complicated loop rather than looping over all id's
    * explicitly because at this point we don't know the "actual" subgroup
    * size and at the moment there's no way to get at it, which means we may
    * loop over always-inactive invocations.
    */

   nir_def *subgroup_id = nir_load_subgroup_invocation(b);

   nir_variable *result =
      nir_local_variable_create(b->impl, glsl_type_for_ssa(val), "result");

   nir_loop *loop = nir_push_loop(b);
   {
      nir_def *first_id = nir_read_first_invocation(b, subgroup_id);
      nir_def *first_val = nir_read_first_invocation(b, val);
      nir_def *first_result =
         nir_read_invocation(b, val, nir_read_first_invocation(b, id));

      nir_if *nif = nir_push_if(b, nir_ieq(b, id, first_id));
      {
         nir_store_var(b, result, first_val, BITFIELD_MASK(val->num_components));
      }
      nir_pop_if(b, nif);

      nir_if *nif2 = nir_push_if(b, nir_elect(b, 1));
      {
         nir_if *nif3 = nir_push_if(b, nir_ult(b, subgroup_id, id));
         {
            nir_store_var(b, result, first_result, BITFIELD_MASK(val->num_components));
         }
         nir_pop_if(b, nif3);

         nir_jump(b, nir_jump_break);
      }
      nir_pop_if(b, nif2);
   }
   nir_pop_loop(b, loop);

   return nir_load_var(b, result);
}

static nir_def *
vec_bit_count(nir_builder *b, nir_def *value)
{
   nir_def *vec_result = nir_bit_count(b, value);
   nir_def *result = nir_channel(b, vec_result, 0);
   for (unsigned i = 1; i < value->num_components; i++)
      result = nir_iadd(b, result, nir_channel(b, vec_result, i));
   return result;
}

/* produce a bitmask of 111...000...111... alternating between "size"
 * 1's and "size" 0's (the LSB is 1).
 */
static uint64_t
reduce_mask(unsigned size, unsigned ballot_bit_size)
{
   uint64_t mask = 0;
   for (unsigned i = 0; i < ballot_bit_size; i += 2 * size) {
      mask |= ((1ull << size) - 1) << i;
   }

   return mask;
}

/* operate on a uniform per-thread bitmask provided by ballot() to perform the
 * desired Boolean reduction. Assumes that the identity of the operation is
 * false (so, no iand).
 */
static nir_def *
lower_boolean_reduce_internal(nir_builder *b, nir_def *src,
                              unsigned cluster_size, nir_op op,
                              const nir_lower_subgroups_options *options)
{
   for (unsigned size = 1; size < cluster_size; size *= 2) {
      nir_def *shifted = nir_ushr_imm(b, src, size);
      src = nir_build_alu2(b, op, shifted, src);
      uint64_t mask = reduce_mask(size, options->ballot_bit_size);
      src = nir_iand_imm(b, src, mask);
      shifted = nir_ishl_imm(b, src, size);
      src = nir_ior(b, src, shifted);
   }

   return src;
}

/* operate on a uniform per-thread bitmask provided by ballot() to perform the
 * desired Boolean inclusive scan. Assumes that the identity of the operation is
 * false (so, no iand).
 */
static nir_def *
lower_boolean_scan_internal(nir_builder *b, nir_def *src,
                            nir_op op,
                            const nir_lower_subgroups_options *options)
{
   if (op == nir_op_ior) {
      /* We want to return a bitmask with all 1's starting at the first 1 in
       * src. -src is equivalent to ~src + 1. While src | ~src returns all
       * 1's, src | (~src + 1) returns all 1's except for the bits changed by
       * the increment. Any 1's before the least significant 0 of ~src are
       * turned into 0 (zeroing those bits after or'ing) and the least
       * signficant 0 of ~src is turned into 1 (not doing anything). So the
       * final output is what we want.
       */
      return nir_ior(b, src, nir_ineg(b, src));
   } else {
      assert(op == nir_op_ixor);
      for (unsigned shift = 1; shift < options->ballot_bit_size; shift *= 2) {
         src = nir_ixor(b, src, nir_ishl_imm(b, src, shift));
      }
      return src;
   }
}

static nir_def *
lower_boolean_reduce(nir_builder *b, nir_intrinsic_instr *intrin,
                     const nir_lower_subgroups_options *options)
{
   assert(intrin->num_components == 1);
   assert(options->ballot_components == 1);

   unsigned cluster_size =
      intrin->intrinsic == nir_intrinsic_reduce ? nir_intrinsic_cluster_size(intrin) : 0;
   nir_op op = nir_intrinsic_reduction_op(intrin);

   /* For certain cluster sizes, reductions of iand and ior can be implemented
    * more efficiently.
    */
   if (intrin->intrinsic == nir_intrinsic_reduce) {
      if (cluster_size == 0) {
         if (op == nir_op_iand)
            return nir_vote_all(b, 1, intrin->src[0].ssa);
         else if (op == nir_op_ior)
            return nir_vote_any(b, 1, intrin->src[0].ssa);
         else if (op == nir_op_ixor)
            return nir_i2b(b, nir_iand_imm(b, vec_bit_count(b, nir_ballot(b,
                                                                          options->ballot_components,
                                                                          options->ballot_bit_size,
                                                                          intrin->src[0].ssa)),
                                           1));
         else
            unreachable("bad boolean reduction op");
      }

      if (cluster_size == 4) {
         if (op == nir_op_iand)
            return nir_quad_vote_all(b, 1, intrin->src[0].ssa);
         else if (op == nir_op_ior)
            return nir_quad_vote_any(b, 1, intrin->src[0].ssa);
      }
   }

   nir_def *src = intrin->src[0].ssa;

   /* Apply DeMorgan's law to implement "and" reductions, since all the
    * lower_boolean_*_internal() functions assume an identity of 0 to make the
    * generated code shorter.
    */
   nir_op new_op = (op == nir_op_iand) ? nir_op_ior : op;
   if (op == nir_op_iand) {
      src = nir_inot(b, src);
   }

   nir_def *val = nir_ballot(b, options->ballot_components, options->ballot_bit_size, src);

   switch (intrin->intrinsic) {
   case nir_intrinsic_reduce:
      val = lower_boolean_reduce_internal(b, val, cluster_size, new_op, options);
      break;
   case nir_intrinsic_inclusive_scan:
      val = lower_boolean_scan_internal(b, val, new_op, options);
      break;
   case nir_intrinsic_exclusive_scan:
      val = lower_boolean_scan_internal(b, val, new_op, options);
      val = nir_ishl_imm(b, val, 1);
      break;
   default:
      unreachable("bad intrinsic");
   }

   if (op == nir_op_iand) {
      val = nir_inot(b, val);
   }

   return nir_inverse_ballot(b, 1, val);
}

static bool
lower_subgroups_filter(const nir_instr *instr, const void *_options)
{
   return instr->type == nir_instr_type_intrinsic;
}

/* Return a ballot-mask-sized value which represents "val" sign-extended and
 * then shifted left by "shift". Only particular values for "val" are
 * supported, see below.
 */
static nir_def *
build_ballot_imm_ishl(nir_builder *b, int64_t val, nir_def *shift,
                      const nir_lower_subgroups_options *options)
{
   /* This only works if all the high bits are the same as bit 1. */
   assert((val >> 2) == (val & 0x2 ? -1 : 0));

   /* First compute the result assuming one ballot component. */
   nir_def *result =
      nir_ishl(b, nir_imm_intN_t(b, val, options->ballot_bit_size), shift);

   if (options->ballot_components == 1)
      return result;

   /* Fix up the result when there is > 1 component. The idea is that nir_ishl
    * masks out the high bits of the shift value already, so in case there's
    * more than one component the component which 1 would be shifted into
    * already has the right value and all we have to do is fixup the other
    * components. Components below it should always be 0, and components above
    * it must be either 0 or ~0 because of the assert above. For example, if
    * the target ballot size is 2 x uint32, and we're shifting 1 by 33, then
    * we'll feed 33 into ishl, which will mask it off to get 1, so we'll
    * compute a single-component result of 2, which is correct for the second
    * component, but the first component needs to be 0, which we get by
    * comparing the high bits of the shift with 0 and selecting the original
    * answer or 0 for the first component (and something similar with the
    * second component). This idea is generalized here for any component count
    */
   nir_const_value min_shift[4];
   for (unsigned i = 0; i < options->ballot_components; i++)
      min_shift[i] = nir_const_value_for_int(i * options->ballot_bit_size, 32);
   nir_def *min_shift_val = nir_build_imm(b, options->ballot_components, 32, min_shift);

   nir_const_value max_shift[4];
   for (unsigned i = 0; i < options->ballot_components; i++)
      max_shift[i] = nir_const_value_for_int((i + 1) * options->ballot_bit_size, 32);
   nir_def *max_shift_val = nir_build_imm(b, options->ballot_components, 32, max_shift);

   return nir_bcsel(b, nir_ult(b, shift, max_shift_val),
                    nir_bcsel(b, nir_ult(b, shift, min_shift_val),
                              nir_imm_intN_t(b, val >> 63, result->bit_size),
                              result),
                    nir_imm_intN_t(b, 0, result->bit_size));
}

static nir_def *
build_subgroup_eq_mask(nir_builder *b,
                       const nir_lower_subgroups_options *options)
{
   nir_def *subgroup_idx = nir_load_subgroup_invocation(b);

   return build_ballot_imm_ishl(b, 1, subgroup_idx, options);
}

static nir_def *
build_subgroup_ge_mask(nir_builder *b,
                       const nir_lower_subgroups_options *options)
{
   nir_def *subgroup_idx = nir_load_subgroup_invocation(b);

   return build_ballot_imm_ishl(b, ~0ull, subgroup_idx, options);
}

static nir_def *
build_subgroup_gt_mask(nir_builder *b,
                       const nir_lower_subgroups_options *options)
{
   nir_def *subgroup_idx = nir_load_subgroup_invocation(b);

   return build_ballot_imm_ishl(b, ~1ull, subgroup_idx, options);
}

/* Return a mask which is 1 for threads up to the run-time subgroup size, i.e.
 * 1 for the entire subgroup. SPIR-V requires us to return 0 for indices at or
 * above the subgroup size for the masks, but gt_mask and ge_mask make them 1
 * so we have to "and" with this mask.
 */
static nir_def *
build_subgroup_mask(nir_builder *b,
                    const nir_lower_subgroups_options *options)
{
   nir_def *subgroup_size = nir_load_subgroup_size(b);

   /* First compute the result assuming one ballot component. */
   nir_def *result =
      nir_ushr(b, nir_imm_intN_t(b, ~0ull, options->ballot_bit_size),
               nir_isub_imm(b, options->ballot_bit_size,
                            subgroup_size));

   /* Since the subgroup size and ballot bitsize are both powers of two, there
    * are two possible cases to consider:
    *
    * (1) The subgroup size is less than the ballot bitsize. We need to return
    * "result" in the first component and 0 in every other component.
    * (2) The subgroup size is a multiple of the ballot bitsize. We need to
    * return ~0 if the subgroup size divided by the ballot bitsize is less
    * than or equal to the index in the vector and 0 otherwise. For example,
    * with a target ballot type of 4 x uint32 and subgroup_size = 64 we'd need
    * to return { ~0, ~0, 0, 0 }.
    *
    * In case (2) it turns out that "result" will be ~0, because
    * "ballot_bit_size - subgroup_size" is also a multiple of
    * "ballot_bit_size" and since nir_ushr masks the shift value it will
    * shifted by 0. This means that the first component can just be "result"
    * in all cases.  The other components will also get the correct value in
    * case (1) if we just use the rule in case (2), so we'll get the correct
    * result if we just follow (2) and then replace the first component with
    * "result".
    */
   nir_const_value min_idx[4];
   for (unsigned i = 0; i < options->ballot_components; i++)
      min_idx[i] = nir_const_value_for_int(i * options->ballot_bit_size, 32);
   nir_def *min_idx_val = nir_build_imm(b, options->ballot_components, 32, min_idx);

   nir_def *result_extended =
      nir_pad_vector_imm_int(b, result, ~0ull, options->ballot_components);

   return nir_bcsel(b, nir_ult(b, min_idx_val, subgroup_size),
                    result_extended, nir_imm_intN_t(b, 0, options->ballot_bit_size));
}

static nir_def *
vec_find_lsb(nir_builder *b, nir_def *value)
{
   nir_def *vec_result = nir_find_lsb(b, value);
   nir_def *result = nir_imm_int(b, -1);
   for (int i = value->num_components - 1; i >= 0; i--) {
      nir_def *channel = nir_channel(b, vec_result, i);
      /* result = channel >= 0 ? (i * bitsize + channel) : result */
      result = nir_bcsel(b, nir_ige_imm(b, channel, 0),
                         nir_iadd_imm(b, channel, i * value->bit_size),
                         result);
   }
   return result;
}

static nir_def *
vec_find_msb(nir_builder *b, nir_def *value)
{
   nir_def *vec_result = nir_ufind_msb(b, value);
   nir_def *result = nir_imm_int(b, -1);
   for (unsigned i = 0; i < value->num_components; i++) {
      nir_def *channel = nir_channel(b, vec_result, i);
      /* result = channel >= 0 ? (i * bitsize + channel) : result */
      result = nir_bcsel(b, nir_ige_imm(b, channel, 0),
                         nir_iadd_imm(b, channel, i * value->bit_size),
                         result);
   }
   return result;
}

static nir_def *
lower_dynamic_quad_broadcast(nir_builder *b, nir_intrinsic_instr *intrin,
                             const nir_lower_subgroups_options *options)
{
   if (!options->lower_quad_broadcast_dynamic_to_const)
      return lower_to_shuffle(b, intrin, options);

   nir_def *dst = NULL;

   for (unsigned i = 0; i < 4; ++i) {
      nir_def *qbcst = nir_quad_broadcast(b, intrin->src[0].ssa,
                                              nir_imm_int(b, i));

      if (i)
         dst = nir_bcsel(b, nir_ieq_imm(b, intrin->src[1].ssa, i),
                         qbcst, dst);
      else
         dst = qbcst;
   }

   return dst;
}

static nir_def *
lower_first_invocation_to_ballot(nir_builder *b, nir_intrinsic_instr *intrin,
                                 const nir_lower_subgroups_options *options)
{
   return nir_ballot_find_lsb(b, 32, nir_ballot(b, 4, 32, nir_imm_true(b)));
}

static nir_def *
lower_read_first_invocation(nir_builder *b, nir_intrinsic_instr *intrin)
{
   return nir_read_invocation(b, intrin->src[0].ssa, nir_first_invocation(b));
}

static nir_def *
lower_read_invocation_to_cond(nir_builder *b, nir_intrinsic_instr *intrin)
{
   return nir_read_invocation_cond_ir3(b, intrin->def.bit_size,
                                       intrin->src[0].ssa,
                                       nir_ieq(b, intrin->src[1].ssa,
                                               nir_load_subgroup_invocation(b)));
}

static nir_def *
lower_subgroups_instr(nir_builder *b, nir_instr *instr, void *_options)
{
   const nir_lower_subgroups_options *options = _options;

   nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
   switch (intrin->intrinsic) {
   case nir_intrinsic_vote_any:
   case nir_intrinsic_vote_all:
      if (options->lower_vote_trivial)
         return intrin->src[0].ssa;
      break;

   case nir_intrinsic_vote_feq:
   case nir_intrinsic_vote_ieq:
      if (options->lower_vote_trivial)
         return nir_imm_true(b);

      if (nir_src_bit_size(intrin->src[0]) == 1) {
         if (options->lower_vote_bool_eq)
            return lower_vote_eq(b, intrin);
      } else {
         if (options->lower_vote_eq)
            return lower_vote_eq(b, intrin);
      }

      if (options->lower_to_scalar && intrin->num_components > 1)
         return lower_vote_eq_to_scalar(b, intrin);
      break;

   case nir_intrinsic_load_subgroup_size:
      if (options->subgroup_size)
         return nir_imm_int(b, options->subgroup_size);
      break;

   case nir_intrinsic_first_invocation:
      if (options->subgroup_size == 1)
         return nir_imm_int(b, 0);

      if (options->lower_first_invocation_to_ballot)
         return lower_first_invocation_to_ballot(b, intrin, options);

      break;

   case nir_intrinsic_read_invocation:
      if (options->lower_to_scalar && intrin->num_components > 1)
         return lower_subgroup_op_to_scalar(b, intrin);

      if (options->lower_read_invocation_to_cond)
         return lower_read_invocation_to_cond(b, intrin);

      break;

   case nir_intrinsic_read_first_invocation:
      if (options->lower_to_scalar && intrin->num_components > 1)
         return lower_subgroup_op_to_scalar(b, intrin);

      if (options->lower_read_first_invocation)
         return lower_read_first_invocation(b, intrin);
      break;

   case nir_intrinsic_load_subgroup_eq_mask:
   case nir_intrinsic_load_subgroup_ge_mask:
   case nir_intrinsic_load_subgroup_gt_mask:
   case nir_intrinsic_load_subgroup_le_mask:
   case nir_intrinsic_load_subgroup_lt_mask: {
      if (!options->lower_subgroup_masks)
         return NULL;

      nir_def *val;
      switch (intrin->intrinsic) {
      case nir_intrinsic_load_subgroup_eq_mask:
         val = build_subgroup_eq_mask(b, options);
         break;
      case nir_intrinsic_load_subgroup_ge_mask:
         val = nir_iand(b, build_subgroup_ge_mask(b, options),
                        build_subgroup_mask(b, options));
         break;
      case nir_intrinsic_load_subgroup_gt_mask:
         val = nir_iand(b, build_subgroup_gt_mask(b, options),
                        build_subgroup_mask(b, options));
         break;
      case nir_intrinsic_load_subgroup_le_mask:
         val = nir_inot(b, build_subgroup_gt_mask(b, options));
         break;
      case nir_intrinsic_load_subgroup_lt_mask:
         val = nir_inot(b, build_subgroup_ge_mask(b, options));
         break;
      default:
         unreachable("you seriously can't tell this is unreachable?");
      }

      return uint_to_ballot_type(b, val,
                                 intrin->def.num_components,
                                 intrin->def.bit_size);
   }

   case nir_intrinsic_ballot: {
      if (intrin->def.num_components == options->ballot_components &&
          intrin->def.bit_size == options->ballot_bit_size)
         return NULL;

      nir_def *ballot =
         nir_ballot(b, options->ballot_components, options->ballot_bit_size,
                    intrin->src[0].ssa);

      return uint_to_ballot_type(b, ballot,
                                 intrin->def.num_components,
                                 intrin->def.bit_size);
   }

   case nir_intrinsic_inverse_ballot:
      if (options->lower_inverse_ballot) {
         return nir_ballot_bitfield_extract(b, 1, intrin->src[0].ssa,
                                            nir_load_subgroup_invocation(b));
      } else if (intrin->src[0].ssa->num_components != options->ballot_components ||
                 intrin->src[0].ssa->bit_size != options->ballot_bit_size) {
         return nir_inverse_ballot(b, 1, ballot_type_to_uint(b, intrin->src[0].ssa, options));
      }
      break;

   case nir_intrinsic_ballot_bitfield_extract:
   case nir_intrinsic_ballot_bit_count_reduce:
   case nir_intrinsic_ballot_find_lsb:
   case nir_intrinsic_ballot_find_msb: {
      nir_def *int_val = ballot_type_to_uint(b, intrin->src[0].ssa,
                                             options);

      if (intrin->intrinsic != nir_intrinsic_ballot_bitfield_extract &&
          intrin->intrinsic != nir_intrinsic_ballot_find_lsb) {
         /* For OpGroupNonUniformBallotFindMSB, the SPIR-V Spec says:
          *
          *    "Find the most significant bit set to 1 in Value, considering
          *    only the bits in Value required to represent all bits of the
          *    group’s invocations.  If none of the considered bits is set to
          *    1, the result is undefined."
          *
          * It has similar text for the other three.  This means that, in case
          * the subgroup size is less than 32, we have to mask off the unused
          * bits.  If the subgroup size is fixed and greater than or equal to
          * 32, the mask will be 0xffffffff and nir_opt_algebraic will delete
          * the iand.
          *
          * We only have to worry about this for BitCount and FindMSB because
          * FindLSB counts from the bottom and BitfieldExtract selects
          * individual bits.  In either case, if run outside the range of
          * valid bits, we hit the undefined results case and we can return
          * anything we want.
          */
         int_val = nir_iand(b, int_val, build_subgroup_mask(b, options));
      }

      switch (intrin->intrinsic) {
      case nir_intrinsic_ballot_bitfield_extract: {
         nir_def *idx = intrin->src[1].ssa;
         if (int_val->num_components > 1) {
            /* idx will be truncated by nir_ushr, so we just need to select
             * the right component using the bits of idx that are truncated in
             * the shift.
             */
            int_val =
               nir_vector_extract(b, int_val,
                                  nir_udiv_imm(b, idx, int_val->bit_size));
         }

         return nir_test_mask(b, nir_ushr(b, int_val, idx), 1);
      }
      case nir_intrinsic_ballot_bit_count_reduce:
         return vec_bit_count(b, int_val);
      case nir_intrinsic_ballot_find_lsb:
         return vec_find_lsb(b, int_val);
      case nir_intrinsic_ballot_find_msb:
         return vec_find_msb(b, int_val);
      default:
         unreachable("you seriously can't tell this is unreachable?");
      }
   }

   case nir_intrinsic_ballot_bit_count_exclusive:
   case nir_intrinsic_ballot_bit_count_inclusive: {
      nir_def *int_val = ballot_type_to_uint(b, intrin->src[0].ssa,
                                             options);
      if (options->lower_ballot_bit_count_to_mbcnt_amd) {
         nir_def *acc;
         if (intrin->intrinsic == nir_intrinsic_ballot_bit_count_exclusive) {
            acc = nir_imm_int(b, 0);
         } else {
            acc = nir_iand_imm(b, nir_u2u32(b, int_val), 0x1);
            int_val = nir_ushr_imm(b, int_val, 1);
         }
         return nir_mbcnt_amd(b, int_val, acc);
      }

      nir_def *mask;
      if (intrin->intrinsic == nir_intrinsic_ballot_bit_count_inclusive) {
         mask = nir_inot(b, build_subgroup_gt_mask(b, options));
      } else {
         mask = nir_inot(b, build_subgroup_ge_mask(b, options));
      }

      return vec_bit_count(b, nir_iand(b, int_val, mask));
   }

   case nir_intrinsic_elect: {
      if (!options->lower_elect)
         return NULL;

      return nir_ieq(b, nir_load_subgroup_invocation(b), nir_first_invocation(b));
   }

   case nir_intrinsic_shuffle:
      if (options->lower_shuffle)
         return lower_shuffle(b, intrin);
      else if (options->lower_to_scalar && intrin->num_components > 1)
         return lower_subgroup_op_to_scalar(b, intrin);
      else if (options->lower_shuffle_to_32bit && intrin->src[0].ssa->bit_size == 64)
         return lower_subgroup_op_to_32bit(b, intrin);
      break;
   case nir_intrinsic_shuffle_xor:
   case nir_intrinsic_shuffle_up:
   case nir_intrinsic_shuffle_down:
      if (options->lower_relative_shuffle)
         return lower_to_shuffle(b, intrin, options);
      else if (options->lower_to_scalar && intrin->num_components > 1)
         return lower_subgroup_op_to_scalar(b, intrin);
      else if (options->lower_shuffle_to_32bit && intrin->src[0].ssa->bit_size == 64)
         return lower_subgroup_op_to_32bit(b, intrin);
      break;

   case nir_intrinsic_quad_broadcast:
   case nir_intrinsic_quad_swap_horizontal:
   case nir_intrinsic_quad_swap_vertical:
   case nir_intrinsic_quad_swap_diagonal:
      if (options->lower_quad ||
          (options->lower_quad_broadcast_dynamic &&
           intrin->intrinsic == nir_intrinsic_quad_broadcast &&
           !nir_src_is_const(intrin->src[1])))
         return lower_dynamic_quad_broadcast(b, intrin, options);
      else if (options->lower_to_scalar && intrin->num_components > 1)
         return lower_subgroup_op_to_scalar(b, intrin);
      break;

   case nir_intrinsic_reduce: {
      nir_def *ret = NULL;
      /* A cluster size greater than the subgroup size is implemention defined */
      if (options->subgroup_size &&
          nir_intrinsic_cluster_size(intrin) >= options->subgroup_size) {
         nir_intrinsic_set_cluster_size(intrin, 0);
         ret = NIR_LOWER_INSTR_PROGRESS;
      }
      if (nir_intrinsic_cluster_size(intrin) == 1)
         return intrin->src[0].ssa;
      if (options->lower_to_scalar && intrin->num_components > 1)
         return lower_subgroup_op_to_scalar(b, intrin);
      if (options->lower_boolean_reduce && intrin->def.bit_size == 1)
         return lower_boolean_reduce(b, intrin, options);
      return ret;
   }
   case nir_intrinsic_inclusive_scan:
   case nir_intrinsic_exclusive_scan:
      if (options->lower_to_scalar && intrin->num_components > 1)
         return lower_subgroup_op_to_scalar(b, intrin);
      if (options->lower_boolean_reduce && intrin->def.bit_size == 1)
         return lower_boolean_reduce(b, intrin, options);
      break;

   case nir_intrinsic_rotate:
      if (nir_intrinsic_execution_scope(intrin) == SCOPE_SUBGROUP) {
         if (options->lower_rotate_to_shuffle)
            return lower_to_shuffle(b, intrin, options);
         else if (options->lower_to_scalar && intrin->num_components > 1)
            return lower_subgroup_op_to_scalar(b, intrin);
         else if (options->lower_shuffle_to_32bit && intrin->src[0].ssa->bit_size == 64)
            return lower_subgroup_op_to_32bit(b, intrin);
      }
      break;
   case nir_intrinsic_masked_swizzle_amd:
      if (options->lower_to_scalar && intrin->num_components > 1) {
         return lower_subgroup_op_to_scalar(b, intrin);
      } else if (options->lower_shuffle_to_32bit && intrin->src[0].ssa->bit_size == 64) {
         return lower_subgroup_op_to_32bit(b, intrin);
      }
      break;

   default:
      break;
   }

   return NULL;
}

bool
nir_lower_subgroups(nir_shader *shader,
                    const nir_lower_subgroups_options *options)
{
   return nir_shader_lower_instructions(shader,
                                        lower_subgroups_filter,
                                        lower_subgroups_instr,
                                        (void *)options);
}
