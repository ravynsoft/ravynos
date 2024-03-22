/*
 * Copyright © Microsoft Corporation
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "nir_builder.h"

/* The following float-to-half conversion routines are based on the "half" library:
 * https://sourceforge.net/projects/half/
 *
 * half - IEEE 754-based half-precision floating-point library.
 *
 * Copyright (c) 2012-2019 Christian Rau <rauy@users.sourceforge.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Version 2.1.0
 */

static nir_def *
half_rounded(nir_builder *b, nir_def *value, nir_def *guard, nir_def *sticky,
             nir_def *sign, nir_rounding_mode mode)
{
   switch (mode) {
   case nir_rounding_mode_rtne:
      return nir_iadd(b, value, nir_iand(b, guard, nir_ior(b, sticky, value)));
   case nir_rounding_mode_ru:
      sign = nir_ushr_imm(b, sign, 31);
      return nir_iadd(b, value, nir_iand(b, nir_inot(b, sign), nir_ior(b, guard, sticky)));
   case nir_rounding_mode_rd:
      sign = nir_ushr_imm(b, sign, 31);
      return nir_iadd(b, value, nir_iand(b, sign, nir_ior(b, guard, sticky)));
   default:
      return value;
   }
}

static nir_def *
float_to_half_impl(nir_builder *b, nir_def *src, nir_rounding_mode mode)
{
   nir_def *f32infinity = nir_imm_int(b, 255 << 23);
   nir_def *f16max = nir_imm_int(b, (127 + 16) << 23);

   nir_def *sign = nir_iand_imm(b, src, 0x80000000);
   nir_def *one = nir_imm_int(b, 1);

   nir_def *abs = nir_iand_imm(b, src, 0x7FFFFFFF);
   /* NaN or INF. For rtne, overflow also becomes INF, so combine the comparisons */
   nir_push_if(b, nir_ige(b, abs, mode == nir_rounding_mode_rtne ? f16max : f32infinity));
   nir_def *inf_nanfp16 = nir_bcsel(b,
                                    nir_ilt(b, f32infinity, abs),
                                    nir_imm_int(b, 0x7E00),
                                    nir_imm_int(b, 0x7C00));
   nir_push_else(b, NULL);

   nir_def *overflowed_fp16 = NULL;
   if (mode != nir_rounding_mode_rtne) {
      /* Handle overflow */
      nir_push_if(b, nir_ige(b, abs, f16max));
      switch (mode) {
      case nir_rounding_mode_rtz:
         overflowed_fp16 = nir_imm_int(b, 0x7BFF);
         break;
      case nir_rounding_mode_ru:
         /* Negative becomes max float, positive becomes inf */
         overflowed_fp16 = nir_bcsel(b, nir_i2b(b, sign), nir_imm_int(b, 0x7BFF), nir_imm_int(b, 0x7C00));
         break;
      case nir_rounding_mode_rd:
         /* Negative becomes inf, positive becomes max float */
         overflowed_fp16 = nir_bcsel(b, nir_i2b(b, sign), nir_imm_int(b, 0x7C00), nir_imm_int(b, 0x7BFF));
         break;
      default:
         unreachable("Should've been handled already");
      }
      nir_push_else(b, NULL);
   }

   nir_def *zero = nir_imm_int(b, 0);

   nir_push_if(b, nir_ige_imm(b, abs, 113 << 23));

   /* FP16 will be normal */
   nir_def *value = nir_ior(b,
                            nir_ishl_imm(b,
                                         nir_iadd_imm(b,
                                                      nir_ushr_imm(b, abs, 23),
                                                      -112),
                                         10),
                            nir_iand_imm(b, nir_ushr_imm(b, abs, 13), 0x3FFF));
   nir_def *guard = nir_iand(b, nir_ushr_imm(b, abs, 12), one);
   nir_def *sticky = nir_bcsel(b, nir_ine(b, nir_iand_imm(b, abs, 0xFFF), zero), one, zero);
   nir_def *normal_fp16 = half_rounded(b, value, guard, sticky, sign, mode);

   nir_push_else(b, NULL);
   nir_push_if(b, nir_ige_imm(b, abs, 102 << 23));

   /* FP16 will be denormal */
   nir_def *i = nir_isub_imm(b, 125, nir_ushr_imm(b, abs, 23));
   nir_def *masked = nir_ior_imm(b, nir_iand_imm(b, abs, 0x7FFFFF), 0x800000);
   value = nir_ushr(b, masked, nir_iadd(b, i, one));
   guard = nir_iand(b, nir_ushr(b, masked, i), one);
   sticky = nir_bcsel(b, nir_ine(b, nir_iand(b, masked, nir_isub(b, nir_ishl(b, one, i), one)), zero), one, zero);
   nir_def *denormal_fp16 = half_rounded(b, value, guard, sticky, sign, mode);

   nir_push_else(b, NULL);

   /* Handle underflow. Nonzero values need to shift up or down for round-up or round-down */
   nir_def *underflowed_fp16 = zero;
   if (mode == nir_rounding_mode_ru ||
       mode == nir_rounding_mode_rd) {
      nir_push_if(b, nir_i2b(b, abs));

      if (mode == nir_rounding_mode_ru)
         underflowed_fp16 = nir_bcsel(b, nir_i2b(b, sign), zero, one);
      else
         underflowed_fp16 = nir_bcsel(b, nir_i2b(b, sign), one, zero);

      nir_push_else(b, NULL);
      nir_pop_if(b, NULL);
      underflowed_fp16 = nir_if_phi(b, underflowed_fp16, zero);
   }

   nir_pop_if(b, NULL);
   nir_def *underflowed_or_denorm_fp16 = nir_if_phi(b, denormal_fp16, underflowed_fp16);

   nir_pop_if(b, NULL);
   nir_def *finite_fp16 = nir_if_phi(b, normal_fp16, underflowed_or_denorm_fp16);

   nir_def *finite_or_overflowed_fp16 = finite_fp16;
   if (mode != nir_rounding_mode_rtne) {
      nir_pop_if(b, NULL);
      finite_or_overflowed_fp16 = nir_if_phi(b, overflowed_fp16, finite_fp16);
   }

   nir_pop_if(b, NULL);
   nir_def *fp16 = nir_if_phi(b, inf_nanfp16, finite_or_overflowed_fp16);

   return nir_u2u16(b, nir_ior(b, fp16, nir_ushr_imm(b, sign, 16)));
}

static nir_def *
split_f2f16_conversion(nir_builder *b, nir_def *src, nir_rounding_mode rnd)
{
   nir_def *tmp = nir_f2f32(b, src);

   if (rnd == nir_rounding_mode_rtne) {
      /* We round down from double to half float by going through float in
       * between, but this can give us inaccurate results in some cases. One
       * such case is 0x40ee6a0000000001, which should round to 0x7b9b, but
       * going through float first turns into 0x7b9a instead. This is because
       * the first non-fitting bit is set, so we get a tie, but with the least
       * significant bit of the original number set, the tie should break
       * rounding up. The cast to float, however, turns into 0x47735000, which
       * when going to half still ties, but now we lost the tie-up bit, and
       * instead we round to the nearest even, which in this case is down.
       *
       * To fix this, we check if the original would have tied, and if the tie
       * would have rounded up, and if both are true, set the least
       * significant bit of the intermediate float to 1, so that a tie on the
       * next cast rounds up as well. If the rounding already got rid of the
       * tie, that set bit will just be truncated anyway and the end result
       * doesn't change.
       *
       * Another failing case is 0x40effdffffffffff. This one doesn't have the
       * tie from double to half, so it just rounds down to 0x7bff (65504.0),
       * but going through float first, it turns into 0x477ff000, which does
       * have the tie bit for half set, and when that one gets rounded it
       * turns into 0x7c00 (Infinity).
       * The fix for that one is to make sure the intermediate float does not
       * have the tie bit set if the original didn't have it.
       *
       * For the RTZ case, we don't need to do anything, as the intermediate
       * float should be ok already.
       */
      int significand_bits16 = 10;
      int significand_bits32 = 23;
      int significand_bits64 = 52;
      int f64_to_16_tie_bit = significand_bits64 - significand_bits16 - 1;
      int f32_to_16_tie_bit = significand_bits32 - significand_bits16 - 1;
      uint64_t f64_rounds_up_mask = ((1ULL << f64_to_16_tie_bit) - 1);

      nir_def *would_tie = nir_iand_imm(b, src, 1ULL << f64_to_16_tie_bit);
      nir_def *would_rnd_up = nir_iand_imm(b, src, f64_rounds_up_mask);

      nir_def *tie_up = nir_b2i32(b, nir_ine_imm(b, would_rnd_up, 0));

      nir_def *break_tie = nir_bcsel(b,
                                     nir_ine_imm(b, would_tie, 0),
                                     nir_imm_int(b, ~0),
                                     nir_imm_int(b, ~(1U << f32_to_16_tie_bit)));

      tmp = nir_ior(b, tmp, tie_up);
      tmp = nir_iand(b, tmp, break_tie);
   }

   return tmp;
}

static bool
lower_fp16_cast_impl(nir_builder *b, nir_instr *instr, void *data)
{
   nir_lower_fp16_cast_options options = *(nir_lower_fp16_cast_options *)data;
   nir_src *src;
   nir_def *dst;
   uint8_t *swizzle = NULL;
   nir_rounding_mode mode = nir_rounding_mode_undef;

   if (instr->type == nir_instr_type_alu) {
      nir_alu_instr *alu = nir_instr_as_alu(instr);
      src = &alu->src[0].src;
      swizzle = alu->src[0].swizzle;
      dst = &alu->def;
      switch (alu->op) {
      case nir_op_f2f16:
         if (b->shader->info.float_controls_execution_mode & FLOAT_CONTROLS_ROUNDING_MODE_RTZ_FP16)
            mode = nir_rounding_mode_rtz;
         else if (b->shader->info.float_controls_execution_mode & FLOAT_CONTROLS_ROUNDING_MODE_RTE_FP16)
            mode = nir_rounding_mode_rtne;
         break;
      case nir_op_f2f16_rtne:
         mode = nir_rounding_mode_rtne;
         break;
      case nir_op_f2f16_rtz:
         mode = nir_rounding_mode_rtz;
         break;
      case nir_op_f2f64:
         if (src->ssa->bit_size == 16 && (options & nir_lower_fp16_split_fp64)) {
            b->cursor = nir_before_instr(instr);
            nir_src_rewrite(src, nir_f2f32(b, src->ssa));
            return true;
         }
         return false;
      default:
         return false;
      }
   } else if (instr->type == nir_instr_type_intrinsic) {
      nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
      if (intrin->intrinsic != nir_intrinsic_convert_alu_types)
         return false;

      src = &intrin->src[0];
      dst = &intrin->def;
      mode = nir_intrinsic_rounding_mode(intrin);

      if (nir_intrinsic_src_type(intrin) == nir_type_float16 &&
          nir_intrinsic_dest_type(intrin) == nir_type_float64 &&
          (options & nir_lower_fp16_split_fp64)) {
         b->cursor = nir_before_instr(instr);
         nir_src_rewrite(src, nir_f2f32(b, src->ssa));
         return true;
      }

      if (nir_intrinsic_dest_type(intrin) != nir_type_float16)
         return false;
   } else {
      return false;
   }

   bool progress = false;
   if (src->ssa->bit_size == 64 && (options & nir_lower_fp16_split_fp64)) {
      b->cursor = nir_before_instr(instr);
      nir_src_rewrite(src, split_f2f16_conversion(b, src->ssa, mode));
      if (instr->type == nir_instr_type_intrinsic)
         nir_intrinsic_set_src_type(nir_instr_as_intrinsic(instr), nir_type_float32);
      progress = true;
   }

   nir_lower_fp16_cast_options req_option = 0;
   switch (mode) {
   case nir_rounding_mode_rtz:
      req_option = nir_lower_fp16_rtz;
      break;
   case nir_rounding_mode_rtne:
      req_option = nir_lower_fp16_rtne;
      break;
   case nir_rounding_mode_ru:
      req_option = nir_lower_fp16_ru;
      break;
   case nir_rounding_mode_rd:
      req_option = nir_lower_fp16_rd;
      break;
   case nir_rounding_mode_undef:
      if ((options & nir_lower_fp16_all) == nir_lower_fp16_all) {
         /* Pick one arbitrarily for lowering */
         mode = nir_rounding_mode_rtne;
         req_option = nir_lower_fp16_rtne;
      }
      /* Otherwise assume the backend can handle f2f16 with undef rounding */
      break;
   default:
      unreachable("Invalid rounding mode");
   }
   if (!(options & req_option))
      return progress;

   b->cursor = nir_before_instr(instr);
   nir_def *rets[NIR_MAX_VEC_COMPONENTS] = { NULL };

   for (unsigned i = 0; i < dst->num_components; i++) {
      nir_def *comp = nir_channel(b, src->ssa, swizzle ? swizzle[i] : i);
      if (comp->bit_size == 64)
         comp = split_f2f16_conversion(b, comp, mode);
      rets[i] = float_to_half_impl(b, comp, mode);
   }

   nir_def *new_val = nir_vec(b, rets, dst->num_components);
   nir_def_rewrite_uses(dst, new_val);
   return true;
}

bool
nir_lower_fp16_casts(nir_shader *shader, nir_lower_fp16_cast_options options)
{
   return nir_shader_instructions_pass(shader,
                                       lower_fp16_cast_impl,
                                       nir_metadata_none,
                                       &options);
}
