/*
 * Copyright (C) 2015 Rob Clark <robclark@freedesktop.org>
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

#include <stdarg.h>

#include "util/u_math.h"
#include "util/u_memory.h"
#include "util/u_string.h"

#include "ir3_compiler.h"
#include "ir3_image.h"
#include "ir3_nir.h"
#include "ir3_shader.h"

#include "instr-a3xx.h"
#include "ir3.h"
#include "ir3_context.h"

void
ir3_handle_nonuniform(struct ir3_instruction *instr,
                      nir_intrinsic_instr *intrin)
{
   if (nir_intrinsic_has_access(intrin) &&
       (nir_intrinsic_access(intrin) & ACCESS_NON_UNIFORM)) {
      instr->flags |= IR3_INSTR_NONUNIF;
   }
}

void
ir3_handle_bindless_cat6(struct ir3_instruction *instr, nir_src rsrc)
{
   nir_intrinsic_instr *intrin = ir3_bindless_resource(rsrc);
   if (!intrin)
      return;

   instr->flags |= IR3_INSTR_B;
   instr->cat6.base = nir_intrinsic_desc_set(intrin);
}

static struct ir3_instruction *
create_input(struct ir3_context *ctx, unsigned compmask)
{
   struct ir3_instruction *in;

   in = ir3_instr_create(ctx->in_block, OPC_META_INPUT, 1, 0);
   in->input.sysval = ~0;
   __ssa_dst(in)->wrmask = compmask;

   array_insert(ctx->ir, ctx->ir->inputs, in);

   return in;
}

static struct ir3_instruction *
create_frag_input(struct ir3_context *ctx, struct ir3_instruction *coord,
                  unsigned n)
{
   struct ir3_block *block = ctx->block;
   struct ir3_instruction *instr;
   /* packed inloc is fixed up later: */
   struct ir3_instruction *inloc = create_immed(block, n);

   if (coord) {
      instr = ir3_BARY_F(block, inloc, 0, coord, 0);
   } else if (ctx->compiler->flat_bypass) {
      if (ctx->compiler->gen >= 6) {
         instr = ir3_FLAT_B(block, inloc, 0, inloc, 0);
      } else {
         instr = ir3_LDLV(block, inloc, 0, create_immed(block, 1), 0);
         instr->cat6.type = TYPE_U32;
         instr->cat6.iim_val = 1;
      }
   } else {
      instr = ir3_BARY_F(block, inloc, 0, ctx->ij[IJ_PERSP_PIXEL], 0);
      instr->srcs[1]->wrmask = 0x3;
   }

   return instr;
}

static struct ir3_instruction *
create_driver_param(struct ir3_context *ctx, enum ir3_driver_param dp)
{
   /* first four vec4 sysval's reserved for UBOs: */
   /* NOTE: dp is in scalar, but there can be >4 dp components: */
   struct ir3_const_state *const_state = ir3_const_state(ctx->so);
   unsigned n = const_state->offsets.driver_param;
   unsigned r = regid(n + dp / 4, dp % 4);
   return create_uniform(ctx->block, r);
}

static struct ir3_instruction *
create_driver_param_indirect(struct ir3_context *ctx, enum ir3_driver_param dp,
                             struct ir3_instruction *address)
{
   /* first four vec4 sysval's reserved for UBOs: */
   /* NOTE: dp is in scalar, but there can be >4 dp components: */
   struct ir3_const_state *const_state = ir3_const_state(ctx->so);
   unsigned n = const_state->offsets.driver_param;
   return create_uniform_indirect(ctx->block, n * 4 + dp, TYPE_U32, address);
}

/*
 * Adreno's comparisons produce a 1 for true and 0 for false, in either 16 or
 * 32-bit registers.  We use NIR's 1-bit integers to represent bools, and
 * trust that we will only see and/or/xor on those 1-bit values, so we can
 * safely store NIR i1s in a 32-bit reg while always containing either a 1 or
 * 0.
 */

/*
 * alu/sfu instructions:
 */

static struct ir3_instruction *
create_cov(struct ir3_context *ctx, struct ir3_instruction *src,
           unsigned src_bitsize, nir_op op)
{
   type_t src_type, dst_type;

   switch (op) {
   case nir_op_f2f32:
   case nir_op_f2f16_rtne:
   case nir_op_f2f16_rtz:
   case nir_op_f2f16:
   case nir_op_f2i32:
   case nir_op_f2i16:
   case nir_op_f2i8:
   case nir_op_f2u32:
   case nir_op_f2u16:
   case nir_op_f2u8:
      switch (src_bitsize) {
      case 32:
         src_type = TYPE_F32;
         break;
      case 16:
         src_type = TYPE_F16;
         break;
      default:
         ir3_context_error(ctx, "invalid src bit size: %u", src_bitsize);
      }
      break;

   case nir_op_i2f32:
   case nir_op_i2f16:
   case nir_op_i2i32:
   case nir_op_i2i16:
   case nir_op_i2i8:
      switch (src_bitsize) {
      case 32:
         src_type = TYPE_S32;
         break;
      case 16:
         src_type = TYPE_S16;
         break;
      case 8:
         src_type = TYPE_S8;
         break;
      default:
         ir3_context_error(ctx, "invalid src bit size: %u", src_bitsize);
      }
      break;

   case nir_op_u2f32:
   case nir_op_u2f16:
   case nir_op_u2u32:
   case nir_op_u2u16:
   case nir_op_u2u8:
      switch (src_bitsize) {
      case 32:
         src_type = TYPE_U32;
         break;
      case 16:
         src_type = TYPE_U16;
         break;
      case 8:
         src_type = TYPE_U8;
         break;
      default:
         ir3_context_error(ctx, "invalid src bit size: %u", src_bitsize);
      }
      break;

   case nir_op_b2f16:
   case nir_op_b2f32:
   case nir_op_b2i8:
   case nir_op_b2i16:
   case nir_op_b2i32:
      src_type = ctx->compiler->bool_type;
      break;

   default:
      ir3_context_error(ctx, "invalid conversion op: %u", op);
   }

   switch (op) {
   case nir_op_f2f32:
   case nir_op_i2f32:
   case nir_op_u2f32:
   case nir_op_b2f32:
      dst_type = TYPE_F32;
      break;

   case nir_op_f2f16_rtne:
   case nir_op_f2f16_rtz:
   case nir_op_f2f16:
   case nir_op_i2f16:
   case nir_op_u2f16:
   case nir_op_b2f16:
      dst_type = TYPE_F16;
      break;

   case nir_op_f2i32:
   case nir_op_i2i32:
   case nir_op_b2i32:
      dst_type = TYPE_S32;
      break;

   case nir_op_f2i16:
   case nir_op_i2i16:
   case nir_op_b2i16:
      dst_type = TYPE_S16;
      break;

   case nir_op_f2i8:
   case nir_op_i2i8:
   case nir_op_b2i8:
      dst_type = TYPE_S8;
      break;

   case nir_op_f2u32:
   case nir_op_u2u32:
      dst_type = TYPE_U32;
      break;

   case nir_op_f2u16:
   case nir_op_u2u16:
      dst_type = TYPE_U16;
      break;

   case nir_op_f2u8:
   case nir_op_u2u8:
      dst_type = TYPE_U8;
      break;

   default:
      ir3_context_error(ctx, "invalid conversion op: %u", op);
   }

   if (src_type == dst_type)
      return src;

   struct ir3_instruction *cov = ir3_COV(ctx->block, src, src_type, dst_type);

   if (op == nir_op_f2f16_rtne) {
      cov->cat1.round = ROUND_EVEN;
   } else if (op == nir_op_f2f16) {
      unsigned execution_mode = ctx->s->info.float_controls_execution_mode;
      nir_rounding_mode rounding_mode =
         nir_get_rounding_mode_from_float_controls(execution_mode,
                                                   nir_type_float16);
      if (rounding_mode == nir_rounding_mode_rtne)
         cov->cat1.round = ROUND_EVEN;
   }

   return cov;
}

/* For shift instructions NIR always has shift amount as 32 bit integer */
static struct ir3_instruction *
resize_shift_amount(struct ir3_context *ctx, struct ir3_instruction *src,
                    unsigned bs)
{
   if (bs == 16)
      return ir3_COV(ctx->block, src, TYPE_U32, TYPE_U16);
   else if (bs == 8)
      return ir3_COV(ctx->block, src, TYPE_U32, TYPE_U8);
   else
      return src;
}

static void
emit_alu_dot_4x8_as_dp4acc(struct ir3_context *ctx, nir_alu_instr *alu,
                           struct ir3_instruction **dst,
                           struct ir3_instruction **src)
{
   struct ir3_instruction *accumulator = NULL;
   if (alu->op == nir_op_udot_4x8_uadd_sat) {
      accumulator = create_immed(ctx->block, 0);
   } else {
      accumulator = src[2];
   }

   dst[0] = ir3_DP4ACC(ctx->block, src[0], 0, src[1], 0, accumulator, 0);

   if (alu->op == nir_op_udot_4x8_uadd ||
       alu->op == nir_op_udot_4x8_uadd_sat) {
      dst[0]->cat3.signedness = IR3_SRC_UNSIGNED;
   } else {
      dst[0]->cat3.signedness = IR3_SRC_MIXED;
   }

   /* For some reason (sat) doesn't work in unsigned case so
    * we have to emulate it.
    */
   if (alu->op == nir_op_udot_4x8_uadd_sat) {
      dst[0] = ir3_ADD_U(ctx->block, dst[0], 0, src[2], 0);
      dst[0]->flags |= IR3_INSTR_SAT;
   } else if (alu->op == nir_op_sudot_4x8_iadd_sat) {
      dst[0]->flags |= IR3_INSTR_SAT;
   }
}

static void
emit_alu_dot_4x8_as_dp2acc(struct ir3_context *ctx, nir_alu_instr *alu,
                           struct ir3_instruction **dst,
                           struct ir3_instruction **src)
{
   int signedness;
   if (alu->op == nir_op_udot_4x8_uadd ||
       alu->op == nir_op_udot_4x8_uadd_sat) {
      signedness = IR3_SRC_UNSIGNED;
   } else {
      signedness = IR3_SRC_MIXED;
   }

   struct ir3_instruction *accumulator = NULL;
   if (alu->op == nir_op_udot_4x8_uadd_sat ||
       alu->op == nir_op_sudot_4x8_iadd_sat) {
      accumulator = create_immed(ctx->block, 0);
   } else {
      accumulator = src[2];
   }

   dst[0] = ir3_DP2ACC(ctx->block, src[0], 0, src[1], 0, accumulator, 0);
   dst[0]->cat3.packed = IR3_SRC_PACKED_LOW;
   dst[0]->cat3.signedness = signedness;

   dst[0] = ir3_DP2ACC(ctx->block, src[0], 0, src[1], 0, dst[0], 0);
   dst[0]->cat3.packed = IR3_SRC_PACKED_HIGH;
   dst[0]->cat3.signedness = signedness;

   if (alu->op == nir_op_udot_4x8_uadd_sat) {
      dst[0] = ir3_ADD_U(ctx->block, dst[0], 0, src[2], 0);
      dst[0]->flags |= IR3_INSTR_SAT;
   } else if (alu->op == nir_op_sudot_4x8_iadd_sat) {
      dst[0] = ir3_ADD_S(ctx->block, dst[0], 0, src[2], 0);
      dst[0]->flags |= IR3_INSTR_SAT;
   }
}

static void
emit_alu(struct ir3_context *ctx, nir_alu_instr *alu)
{
   const nir_op_info *info = &nir_op_infos[alu->op];
   struct ir3_instruction **dst, *src[info->num_inputs];
   unsigned bs[info->num_inputs]; /* bit size */
   struct ir3_block *b = ctx->block;
   unsigned dst_sz, wrmask;
   type_t dst_type = type_uint_size(alu->def.bit_size);

   dst_sz = alu->def.num_components;
   wrmask = (1 << dst_sz) - 1;

   dst = ir3_get_def(ctx, &alu->def, dst_sz);

   /* Vectors are special in that they have non-scalarized writemasks,
    * and just take the first swizzle channel for each argument in
    * order into each writemask channel.
    */
   if ((alu->op == nir_op_vec2) || (alu->op == nir_op_vec3) ||
       (alu->op == nir_op_vec4) || (alu->op == nir_op_vec8) ||
       (alu->op == nir_op_vec16)) {

      for (int i = 0; i < info->num_inputs; i++) {
         nir_alu_src *asrc = &alu->src[i];

         src[i] = ir3_get_src(ctx, &asrc->src)[asrc->swizzle[0]];
         if (!src[i])
            src[i] = create_immed_typed(ctx->block, 0, dst_type);
         dst[i] = ir3_MOV(b, src[i], dst_type);
      }

      ir3_put_def(ctx, &alu->def);
      return;
   }

   /* We also get mov's with more than one component for mov's so
    * handle those specially:
    */
   if (alu->op == nir_op_mov) {
      nir_alu_src *asrc = &alu->src[0];
      struct ir3_instruction *const *src0 = ir3_get_src(ctx, &asrc->src);

      for (unsigned i = 0; i < dst_sz; i++) {
         if (wrmask & (1 << i)) {
            dst[i] = ir3_MOV(b, src0[asrc->swizzle[i]], dst_type);
         } else {
            dst[i] = NULL;
         }
      }

      ir3_put_def(ctx, &alu->def);
      return;
   }

   /* General case: We can just grab the one used channel per src. */
   assert(alu->def.num_components == 1);

   for (int i = 0; i < info->num_inputs; i++) {
      nir_alu_src *asrc = &alu->src[i];

      src[i] = ir3_get_src(ctx, &asrc->src)[asrc->swizzle[0]];
      bs[i] = nir_src_bit_size(asrc->src);

      compile_assert(ctx, src[i]);
   }

   switch (alu->op) {
   case nir_op_f2f32:
   case nir_op_f2f16_rtne:
   case nir_op_f2f16_rtz:
   case nir_op_f2f16:
   case nir_op_f2i32:
   case nir_op_f2i16:
   case nir_op_f2i8:
   case nir_op_f2u32:
   case nir_op_f2u16:
   case nir_op_f2u8:
   case nir_op_i2f32:
   case nir_op_i2f16:
   case nir_op_i2i32:
   case nir_op_i2i16:
   case nir_op_i2i8:
   case nir_op_u2f32:
   case nir_op_u2f16:
   case nir_op_u2u32:
   case nir_op_u2u16:
   case nir_op_u2u8:
   case nir_op_b2f16:
   case nir_op_b2f32:
   case nir_op_b2i8:
   case nir_op_b2i16:
   case nir_op_b2i32:
      dst[0] = create_cov(ctx, src[0], bs[0], alu->op);
      break;

   case nir_op_fquantize2f16:
      dst[0] = create_cov(ctx, create_cov(ctx, src[0], 32, nir_op_f2f16_rtne),
                          16, nir_op_f2f32);
      break;

   case nir_op_b2b1:
      /* b2b1 will appear when translating from
       *
       * - nir_intrinsic_load_shared of a 32-bit 0/~0 value.
       * - nir_intrinsic_load_constant of a 32-bit 0/~0 value
       *
       * A negate can turn those into a 1 or 0 for us.
       */
      dst[0] = ir3_ABSNEG_S(b, src[0], IR3_REG_SNEG);
      break;

   case nir_op_b2b32:
      /* b2b32 will appear when converting our 1-bit bools to a store_shared
       * argument.
       *
       * A negate can turn those into a ~0 for us.
       */
      dst[0] = ir3_ABSNEG_S(b, src[0], IR3_REG_SNEG);
      break;

   case nir_op_fneg:
      dst[0] = ir3_ABSNEG_F(b, src[0], IR3_REG_FNEG);
      break;
   case nir_op_fabs:
      dst[0] = ir3_ABSNEG_F(b, src[0], IR3_REG_FABS);
      break;
   case nir_op_fmax:
      dst[0] = ir3_MAX_F(b, src[0], 0, src[1], 0);
      break;
   case nir_op_fmin:
      dst[0] = ir3_MIN_F(b, src[0], 0, src[1], 0);
      break;
   case nir_op_fsat:
      /* if there is just a single use of the src, and it supports
       * (sat) bit, we can just fold the (sat) flag back to the
       * src instruction and create a mov.  This is easier for cp
       * to eliminate.
       */
      if (is_sat_compatible(src[0]->opc) &&
          (list_length(&alu->src[0].src.ssa->uses) == 1)) {
         src[0]->flags |= IR3_INSTR_SAT;
         dst[0] = ir3_MOV(b, src[0], dst_type);
      } else {
         /* otherwise generate a max.f that saturates.. blob does
          * similar (generating a cat2 mov using max.f)
          */
         dst[0] = ir3_MAX_F(b, src[0], 0, src[0], 0);
         dst[0]->flags |= IR3_INSTR_SAT;
      }
      break;
   case nir_op_fmul:
      dst[0] = ir3_MUL_F(b, src[0], 0, src[1], 0);
      break;
   case nir_op_fadd:
      dst[0] = ir3_ADD_F(b, src[0], 0, src[1], 0);
      break;
   case nir_op_fsub:
      dst[0] = ir3_ADD_F(b, src[0], 0, src[1], IR3_REG_FNEG);
      break;
   case nir_op_ffma:
      dst[0] = ir3_MAD_F32(b, src[0], 0, src[1], 0, src[2], 0);
      break;
   case nir_op_fddx:
   case nir_op_fddx_coarse:
      dst[0] = ir3_DSX(b, src[0], 0);
      dst[0]->cat5.type = TYPE_F32;
      break;
   case nir_op_fddx_fine:
      dst[0] = ir3_DSXPP_MACRO(b, src[0], 0);
      dst[0]->cat5.type = TYPE_F32;
      break;
   case nir_op_fddy:
   case nir_op_fddy_coarse:
      dst[0] = ir3_DSY(b, src[0], 0);
      dst[0]->cat5.type = TYPE_F32;
      break;
      break;
   case nir_op_fddy_fine:
      dst[0] = ir3_DSYPP_MACRO(b, src[0], 0);
      dst[0]->cat5.type = TYPE_F32;
      break;
   case nir_op_flt:
      dst[0] = ir3_CMPS_F(b, src[0], 0, src[1], 0);
      dst[0]->cat2.condition = IR3_COND_LT;
      break;
   case nir_op_fge:
      dst[0] = ir3_CMPS_F(b, src[0], 0, src[1], 0);
      dst[0]->cat2.condition = IR3_COND_GE;
      break;
   case nir_op_feq:
      dst[0] = ir3_CMPS_F(b, src[0], 0, src[1], 0);
      dst[0]->cat2.condition = IR3_COND_EQ;
      break;
   case nir_op_fneu:
      dst[0] = ir3_CMPS_F(b, src[0], 0, src[1], 0);
      dst[0]->cat2.condition = IR3_COND_NE;
      break;
   case nir_op_fceil:
      dst[0] = ir3_CEIL_F(b, src[0], 0);
      break;
   case nir_op_ffloor:
      dst[0] = ir3_FLOOR_F(b, src[0], 0);
      break;
   case nir_op_ftrunc:
      dst[0] = ir3_TRUNC_F(b, src[0], 0);
      break;
   case nir_op_fround_even:
      dst[0] = ir3_RNDNE_F(b, src[0], 0);
      break;
   case nir_op_fsign:
      dst[0] = ir3_SIGN_F(b, src[0], 0);
      break;

   case nir_op_fsin:
      dst[0] = ir3_SIN(b, src[0], 0);
      break;
   case nir_op_fcos:
      dst[0] = ir3_COS(b, src[0], 0);
      break;
   case nir_op_frsq:
      dst[0] = ir3_RSQ(b, src[0], 0);
      break;
   case nir_op_frcp:
      dst[0] = ir3_RCP(b, src[0], 0);
      break;
   case nir_op_flog2:
      dst[0] = ir3_LOG2(b, src[0], 0);
      break;
   case nir_op_fexp2:
      dst[0] = ir3_EXP2(b, src[0], 0);
      break;
   case nir_op_fsqrt:
      dst[0] = ir3_SQRT(b, src[0], 0);
      break;

   case nir_op_iabs:
      dst[0] = ir3_ABSNEG_S(b, src[0], IR3_REG_SABS);
      break;
   case nir_op_iadd:
      dst[0] = ir3_ADD_U(b, src[0], 0, src[1], 0);
      break;
   case nir_op_ihadd:
      dst[0] = ir3_ADD_S(b, src[0], 0, src[1], 0);
      dst[0]->dsts[0]->flags |= IR3_REG_EI;
      break;
   case nir_op_uhadd:
      dst[0] = ir3_ADD_U(b, src[0], 0, src[1], 0);
      dst[0]->dsts[0]->flags |= IR3_REG_EI;
      break;
   case nir_op_iand:
      dst[0] = ir3_AND_B(b, src[0], 0, src[1], 0);
      break;
   case nir_op_imax:
      dst[0] = ir3_MAX_S(b, src[0], 0, src[1], 0);
      break;
   case nir_op_umax:
      dst[0] = ir3_MAX_U(b, src[0], 0, src[1], 0);
      break;
   case nir_op_imin:
      dst[0] = ir3_MIN_S(b, src[0], 0, src[1], 0);
      break;
   case nir_op_umin:
      dst[0] = ir3_MIN_U(b, src[0], 0, src[1], 0);
      break;
   case nir_op_umul_low:
      dst[0] = ir3_MULL_U(b, src[0], 0, src[1], 0);
      break;
   case nir_op_imadsh_mix16:
      dst[0] = ir3_MADSH_M16(b, src[0], 0, src[1], 0, src[2], 0);
      break;
   case nir_op_imad24_ir3:
      dst[0] = ir3_MAD_S24(b, src[0], 0, src[1], 0, src[2], 0);
      break;
   case nir_op_imul:
      compile_assert(ctx, alu->def.bit_size == 16);
      dst[0] = ir3_MUL_S24(b, src[0], 0, src[1], 0);
      break;
   case nir_op_imul24:
      dst[0] = ir3_MUL_S24(b, src[0], 0, src[1], 0);
      break;
   case nir_op_ineg:
      dst[0] = ir3_ABSNEG_S(b, src[0], IR3_REG_SNEG);
      break;
   case nir_op_inot:
      if (bs[0] == 1) {
         struct ir3_instruction *one =
               create_immed_typed(ctx->block, 1, ctx->compiler->bool_type);
         dst[0] = ir3_SUB_U(b, one, 0, src[0], 0);
      } else {
         dst[0] = ir3_NOT_B(b, src[0], 0);
      }
      break;
   case nir_op_ior:
      dst[0] = ir3_OR_B(b, src[0], 0, src[1], 0);
      break;
   case nir_op_ishl:
      dst[0] =
         ir3_SHL_B(b, src[0], 0, resize_shift_amount(ctx, src[1], bs[0]), 0);
      break;
   case nir_op_ishr:
      dst[0] =
         ir3_ASHR_B(b, src[0], 0, resize_shift_amount(ctx, src[1], bs[0]), 0);
      break;
   case nir_op_isub:
      dst[0] = ir3_SUB_U(b, src[0], 0, src[1], 0);
      break;
   case nir_op_ixor:
      dst[0] = ir3_XOR_B(b, src[0], 0, src[1], 0);
      break;
   case nir_op_ushr:
      dst[0] =
         ir3_SHR_B(b, src[0], 0, resize_shift_amount(ctx, src[1], bs[0]), 0);
      break;
   case nir_op_ilt:
      dst[0] = ir3_CMPS_S(b, src[0], 0, src[1], 0);
      dst[0]->cat2.condition = IR3_COND_LT;
      break;
   case nir_op_ige:
      dst[0] = ir3_CMPS_S(b, src[0], 0, src[1], 0);
      dst[0]->cat2.condition = IR3_COND_GE;
      break;
   case nir_op_ieq:
      dst[0] = ir3_CMPS_S(b, src[0], 0, src[1], 0);
      dst[0]->cat2.condition = IR3_COND_EQ;
      break;
   case nir_op_ine:
      dst[0] = ir3_CMPS_S(b, src[0], 0, src[1], 0);
      dst[0]->cat2.condition = IR3_COND_NE;
      break;
   case nir_op_ult:
      dst[0] = ir3_CMPS_U(b, src[0], 0, src[1], 0);
      dst[0]->cat2.condition = IR3_COND_LT;
      break;
   case nir_op_uge:
      dst[0] = ir3_CMPS_U(b, src[0], 0, src[1], 0);
      dst[0]->cat2.condition = IR3_COND_GE;
      break;

   case nir_op_bcsel: {
      struct ir3_instruction *cond = src[0];

      /* If src[0] is a negation (likely as a result of an ir3_b2n(cond)),
       * we can ignore that and use original cond, since the nonzero-ness of
       * cond stays the same.
       */
      if (cond->opc == OPC_ABSNEG_S && cond->flags == 0 &&
          (cond->srcs[0]->flags & (IR3_REG_SNEG | IR3_REG_SABS)) ==
             IR3_REG_SNEG) {
         cond = cond->srcs[0]->def->instr;
      }

      compile_assert(ctx, bs[1] == bs[2]);

      /* The condition's size has to match the other two arguments' size, so
       * convert down if necessary.
       *
       * Single hashtable is fine, because the conversion will either be
       * 16->32 or 32->16, but never both
       */
      if (is_half(src[1]) != is_half(cond)) {
         struct hash_entry *prev_entry =
            _mesa_hash_table_search(ctx->sel_cond_conversions, src[0]);
         if (prev_entry) {
            cond = prev_entry->data;
         } else {
            if (is_half(cond)) {
               cond = ir3_COV(b, cond, TYPE_U16, TYPE_U32);
            } else {
               cond = ir3_COV(b, cond, TYPE_U32, TYPE_U16);
            }
            _mesa_hash_table_insert(ctx->sel_cond_conversions, src[0], cond);
         }
      }

      if (is_half(src[1])) {
         dst[0] = ir3_SEL_B16(b, src[1], 0, cond, 0, src[2], 0);
      } else {
         dst[0] = ir3_SEL_B32(b, src[1], 0, cond, 0, src[2], 0);
      }

      break;
   }
   case nir_op_bit_count: {
      if (ctx->compiler->gen < 5 || (src[0]->dsts[0]->flags & IR3_REG_HALF)) {
         dst[0] = ir3_CBITS_B(b, src[0], 0);
         break;
      }

      // We need to do this 16b at a time on a5xx+a6xx.  Once half-precision
      // support is in place, this should probably move to a NIR lowering pass:
      struct ir3_instruction *hi, *lo;

      hi = ir3_COV(b, ir3_SHR_B(b, src[0], 0, create_immed(b, 16), 0), TYPE_U32,
                   TYPE_U16);
      lo = ir3_COV(b, src[0], TYPE_U32, TYPE_U16);

      hi = ir3_CBITS_B(b, hi, 0);
      lo = ir3_CBITS_B(b, lo, 0);

      // TODO maybe the builders should default to making dst half-precision
      // if the src's were half precision, to make this less awkward.. otoh
      // we should probably just do this lowering in NIR.
      hi->dsts[0]->flags |= IR3_REG_HALF;
      lo->dsts[0]->flags |= IR3_REG_HALF;

      dst[0] = ir3_ADD_S(b, hi, 0, lo, 0);
      dst[0]->dsts[0]->flags |= IR3_REG_HALF;
      dst[0] = ir3_COV(b, dst[0], TYPE_U16, TYPE_U32);
      break;
   }
   case nir_op_ifind_msb: {
      struct ir3_instruction *cmp;
      dst[0] = ir3_CLZ_S(b, src[0], 0);
      cmp = ir3_CMPS_S(b, dst[0], 0, create_immed(b, 0), 0);
      cmp->cat2.condition = IR3_COND_GE;
      dst[0] = ir3_SEL_B32(b, ir3_SUB_U(b, create_immed(b, 31), 0, dst[0], 0),
                           0, cmp, 0, dst[0], 0);
      break;
   }
   case nir_op_ufind_msb:
      dst[0] = ir3_CLZ_B(b, src[0], 0);
      dst[0] = ir3_SEL_B32(b, ir3_SUB_U(b, create_immed(b, 31), 0, dst[0], 0),
                           0, src[0], 0, dst[0], 0);
      break;
   case nir_op_find_lsb:
      dst[0] = ir3_BFREV_B(b, src[0], 0);
      dst[0] = ir3_CLZ_B(b, dst[0], 0);
      break;
   case nir_op_bitfield_reverse:
      dst[0] = ir3_BFREV_B(b, src[0], 0);
      break;

   case nir_op_uadd_sat:
      dst[0] = ir3_ADD_U(b, src[0], 0, src[1], 0);
      dst[0]->flags |= IR3_INSTR_SAT;
      break;
   case nir_op_iadd_sat:
      dst[0] = ir3_ADD_S(b, src[0], 0, src[1], 0);
      dst[0]->flags |= IR3_INSTR_SAT;
      break;
   case nir_op_usub_sat:
      dst[0] = ir3_SUB_U(b, src[0], 0, src[1], 0);
      dst[0]->flags |= IR3_INSTR_SAT;
      break;
   case nir_op_isub_sat:
      dst[0] = ir3_SUB_S(b, src[0], 0, src[1], 0);
      dst[0]->flags |= IR3_INSTR_SAT;
      break;

   case nir_op_udot_4x8_uadd:
   case nir_op_udot_4x8_uadd_sat:
   case nir_op_sudot_4x8_iadd:
   case nir_op_sudot_4x8_iadd_sat: {
      if (ctx->compiler->has_dp4acc) {
         emit_alu_dot_4x8_as_dp4acc(ctx, alu, dst, src);
      } else if (ctx->compiler->has_dp2acc) {
         emit_alu_dot_4x8_as_dp2acc(ctx, alu, dst, src);
      } else {
         ir3_context_error(ctx, "ALU op should have been lowered: %s\n",
                           nir_op_infos[alu->op].name);
      }

      break;
   }

   default:
      ir3_context_error(ctx, "Unhandled ALU op: %s\n",
                        nir_op_infos[alu->op].name);
      break;
   }

   if (nir_alu_type_get_base_type(info->output_type) == nir_type_bool) {
      assert(alu->def.bit_size == 1 || alu->op == nir_op_b2b32);
      assert(dst_sz == 1);
   } else {
      /* 1-bit values stored in 32-bit registers are only valid for certain
       * ALU ops.
       */
      switch (alu->op) {
      case nir_op_iand:
      case nir_op_ior:
      case nir_op_ixor:
      case nir_op_inot:
      case nir_op_bcsel:
         break;
      default:
         compile_assert(ctx, alu->def.bit_size != 1);
      }
   }

   ir3_put_def(ctx, &alu->def);
}

static void
emit_intrinsic_load_ubo_ldc(struct ir3_context *ctx, nir_intrinsic_instr *intr,
                            struct ir3_instruction **dst)
{
   struct ir3_block *b = ctx->block;

   /* This is only generated for us by nir_lower_ubo_vec4, which leaves base =
    * 0.
    */
   assert(nir_intrinsic_base(intr) == 0);

   unsigned ncomp = intr->num_components;
   struct ir3_instruction *offset = ir3_get_src(ctx, &intr->src[1])[0];
   struct ir3_instruction *idx = ir3_get_src(ctx, &intr->src[0])[0];
   struct ir3_instruction *ldc = ir3_LDC(b, idx, 0, offset, 0);
   ldc->dsts[0]->wrmask = MASK(ncomp);
   ldc->cat6.iim_val = ncomp;
   ldc->cat6.d = nir_intrinsic_component(intr);
   ldc->cat6.type = utype_def(&intr->def);

   ir3_handle_bindless_cat6(ldc, intr->src[0]);
   if (ldc->flags & IR3_INSTR_B)
      ctx->so->bindless_ubo = true;
   ir3_handle_nonuniform(ldc, intr);

   ir3_split_dest(b, dst, ldc, 0, ncomp);
}

static void
emit_intrinsic_copy_ubo_to_uniform(struct ir3_context *ctx,
                                   nir_intrinsic_instr *intr)
{
   struct ir3_block *b = ctx->block;

   unsigned base = nir_intrinsic_base(intr);
   unsigned size = nir_intrinsic_range(intr);

   struct ir3_instruction *addr1 = ir3_get_addr1(ctx, base);

   struct ir3_instruction *offset = ir3_get_src(ctx, &intr->src[1])[0];
   struct ir3_instruction *idx = ir3_get_src(ctx, &intr->src[0])[0];
   struct ir3_instruction *ldc = ir3_LDC_K(b, idx, 0, offset, 0);
   ldc->cat6.iim_val = size;
   ldc->barrier_class = ldc->barrier_conflict = IR3_BARRIER_CONST_W;

   ir3_handle_bindless_cat6(ldc, intr->src[0]);
   if (ldc->flags & IR3_INSTR_B)
      ctx->so->bindless_ubo = true;

   ir3_instr_set_address(ldc, addr1);

   array_insert(b, b->keeps, ldc);
}

/* handles direct/indirect UBO reads: */
static void
emit_intrinsic_load_ubo(struct ir3_context *ctx, nir_intrinsic_instr *intr,
                        struct ir3_instruction **dst)
{
   struct ir3_block *b = ctx->block;
   struct ir3_instruction *base_lo, *base_hi, *addr, *src0, *src1;
   const struct ir3_const_state *const_state = ir3_const_state(ctx->so);
   unsigned ubo = regid(const_state->offsets.ubo, 0);
   const unsigned ptrsz = ir3_pointer_size(ctx->compiler);

   int off = 0;

   /* First src is ubo index, which could either be an immed or not: */
   src0 = ir3_get_src(ctx, &intr->src[0])[0];
   if (is_same_type_mov(src0) && (src0->srcs[0]->flags & IR3_REG_IMMED)) {
      base_lo = create_uniform(b, ubo + (src0->srcs[0]->iim_val * ptrsz));
      base_hi = create_uniform(b, ubo + (src0->srcs[0]->iim_val * ptrsz) + 1);
   } else {
      base_lo = create_uniform_indirect(b, ubo, TYPE_U32,
                                        ir3_get_addr0(ctx, src0, ptrsz));
      base_hi = create_uniform_indirect(b, ubo + 1, TYPE_U32,
                                        ir3_get_addr0(ctx, src0, ptrsz));

      /* NOTE: since relative addressing is used, make sure constlen is
       * at least big enough to cover all the UBO addresses, since the
       * assembler won't know what the max address reg is.
       */
      ctx->so->constlen =
         MAX2(ctx->so->constlen,
              const_state->offsets.ubo + (ctx->s->info.num_ubos * ptrsz));
   }

   /* note: on 32bit gpu's base_hi is ignored and DCE'd */
   addr = base_lo;

   if (nir_src_is_const(intr->src[1])) {
      off += nir_src_as_uint(intr->src[1]);
   } else {
      /* For load_ubo_indirect, second src is indirect offset: */
      src1 = ir3_get_src(ctx, &intr->src[1])[0];

      /* and add offset to addr: */
      addr = ir3_ADD_S(b, addr, 0, src1, 0);
   }

   /* if offset is to large to encode in the ldg, split it out: */
   if ((off + (intr->num_components * 4)) > 1024) {
      /* split out the minimal amount to improve the odds that
       * cp can fit the immediate in the add.s instruction:
       */
      unsigned off2 = off + (intr->num_components * 4) - 1024;
      addr = ir3_ADD_S(b, addr, 0, create_immed(b, off2), 0);
      off -= off2;
   }

   if (ptrsz == 2) {
      struct ir3_instruction *carry;

      /* handle 32b rollover, ie:
       *   if (addr < base_lo)
       *      base_hi++
       */
      carry = ir3_CMPS_U(b, addr, 0, base_lo, 0);
      carry->cat2.condition = IR3_COND_LT;
      base_hi = ir3_ADD_S(b, base_hi, 0, carry, 0);

      addr = ir3_collect(b, addr, base_hi);
   }

   for (int i = 0; i < intr->num_components; i++) {
      struct ir3_instruction *load =
         ir3_LDG(b, addr, 0, create_immed(b, off + i * 4), 0,
                 create_immed(b, 1), 0); /* num components */
      load->cat6.type = TYPE_U32;
      dst[i] = load;
   }
}

/* Load a kernel param: src[] = { address }. */
static void
emit_intrinsic_load_kernel_input(struct ir3_context *ctx,
                                 nir_intrinsic_instr *intr,
                                 struct ir3_instruction **dst)
{
   const struct ir3_const_state *const_state = ir3_const_state(ctx->so);
   struct ir3_block *b = ctx->block;
   unsigned offset = nir_intrinsic_base(intr);
   unsigned p = regid(const_state->offsets.kernel_params, 0);

   struct ir3_instruction *src0 = ir3_get_src(ctx, &intr->src[0])[0];

   if (is_same_type_mov(src0) && (src0->srcs[0]->flags & IR3_REG_IMMED)) {
      offset += src0->srcs[0]->iim_val;

      /* kernel param position is in bytes, but constant space is 32b registers: */
      compile_assert(ctx, !(offset & 0x3));

      dst[0] = create_uniform(b, p + (offset / 4));
   } else {
      /* kernel param position is in bytes, but constant space is 32b registers: */
      compile_assert(ctx, !(offset & 0x3));

      /* TODO we should probably be lowering this in nir, and also handling
       * non-32b inputs.. Also we probably don't want to be using
       * SP_MODE_CONTROL.CONSTANT_DEMOTION_ENABLE for KERNEL shaders..
       */
      src0 = ir3_SHR_B(b, src0, 0, create_immed(b, 2), 0);

      dst[0] = create_uniform_indirect(b, offset / 4, TYPE_U32,
                                       ir3_get_addr0(ctx, src0, 1));
   }
}

/* src[] = { block_index } */
static void
emit_intrinsic_ssbo_size(struct ir3_context *ctx, nir_intrinsic_instr *intr,
                         struct ir3_instruction **dst)
{
   struct ir3_block *b = ctx->block;
   struct ir3_instruction *ibo = ir3_ssbo_to_ibo(ctx, intr->src[0]);
   struct ir3_instruction *resinfo = ir3_RESINFO(b, ibo, 0);
   resinfo->cat6.iim_val = 1;
   resinfo->cat6.d = ctx->compiler->gen >= 6 ? 1 : 2;
   resinfo->cat6.type = TYPE_U32;
   resinfo->cat6.typed = false;
   /* resinfo has no writemask and always writes out 3 components */
   resinfo->dsts[0]->wrmask = MASK(3);
   ir3_handle_bindless_cat6(resinfo, intr->src[0]);
   ir3_handle_nonuniform(resinfo, intr);

   if (ctx->compiler->gen >= 6) {
      ir3_split_dest(b, dst, resinfo, 0, 1);
   } else {
      /* On a5xx, resinfo returns the low 16 bits of ssbo size in .x and the high 16 bits in .y */
      struct ir3_instruction *resinfo_dst[2];
      ir3_split_dest(b, resinfo_dst, resinfo, 0, 2);
      *dst = ir3_ADD_U(b, ir3_SHL_B(b, resinfo_dst[1], 0, create_immed(b, 16), 0), 0, resinfo_dst[0], 0);
   }
}

/* src[] = { offset }. const_index[] = { base } */
static void
emit_intrinsic_load_shared(struct ir3_context *ctx, nir_intrinsic_instr *intr,
                           struct ir3_instruction **dst)
{
   struct ir3_block *b = ctx->block;
   struct ir3_instruction *ldl, *offset;
   unsigned base;

   offset = ir3_get_src(ctx, &intr->src[0])[0];
   base = nir_intrinsic_base(intr);

   ldl = ir3_LDL(b, offset, 0, create_immed(b, base), 0,
                 create_immed(b, intr->num_components), 0);

   ldl->cat6.type = utype_def(&intr->def);
   ldl->dsts[0]->wrmask = MASK(intr->num_components);

   ldl->barrier_class = IR3_BARRIER_SHARED_R;
   ldl->barrier_conflict = IR3_BARRIER_SHARED_W;

   ir3_split_dest(b, dst, ldl, 0, intr->num_components);
}

/* src[] = { value, offset }. const_index[] = { base, write_mask } */
static void
emit_intrinsic_store_shared(struct ir3_context *ctx, nir_intrinsic_instr *intr)
{
   struct ir3_block *b = ctx->block;
   struct ir3_instruction *stl, *offset;
   struct ir3_instruction *const *value;
   unsigned base, wrmask, ncomp;

   value = ir3_get_src(ctx, &intr->src[0]);
   offset = ir3_get_src(ctx, &intr->src[1])[0];

   base = nir_intrinsic_base(intr);
   wrmask = nir_intrinsic_write_mask(intr);
   ncomp = ffs(~wrmask) - 1;

   assert(wrmask == BITFIELD_MASK(intr->num_components));

   stl = ir3_STL(b, offset, 0, ir3_create_collect(b, value, ncomp), 0,
                 create_immed(b, ncomp), 0);
   stl->cat6.dst_offset = base;
   stl->cat6.type = utype_src(intr->src[0]);
   stl->barrier_class = IR3_BARRIER_SHARED_W;
   stl->barrier_conflict = IR3_BARRIER_SHARED_R | IR3_BARRIER_SHARED_W;

   array_insert(b, b->keeps, stl);
}

/* src[] = { offset }. const_index[] = { base } */
static void
emit_intrinsic_load_shared_ir3(struct ir3_context *ctx,
                               nir_intrinsic_instr *intr,
                               struct ir3_instruction **dst)
{
   struct ir3_block *b = ctx->block;
   struct ir3_instruction *load, *offset;
   unsigned base;

   offset = ir3_get_src(ctx, &intr->src[0])[0];
   base = nir_intrinsic_base(intr);

   load = ir3_LDLW(b, offset, 0, create_immed(b, base), 0,
                   create_immed(b, intr->num_components), 0);

   /* for a650, use LDL for tess ctrl inputs: */
   if (ctx->so->type == MESA_SHADER_TESS_CTRL && ctx->compiler->tess_use_shared)
      load->opc = OPC_LDL;

   load->cat6.type = utype_def(&intr->def);
   load->dsts[0]->wrmask = MASK(intr->num_components);

   load->barrier_class = IR3_BARRIER_SHARED_R;
   load->barrier_conflict = IR3_BARRIER_SHARED_W;

   ir3_split_dest(b, dst, load, 0, intr->num_components);
}

/* src[] = { value, offset }. const_index[] = { base } */
static void
emit_intrinsic_store_shared_ir3(struct ir3_context *ctx,
                                nir_intrinsic_instr *intr)
{
   struct ir3_block *b = ctx->block;
   struct ir3_instruction *store, *offset;
   struct ir3_instruction *const *value;

   value = ir3_get_src(ctx, &intr->src[0]);
   offset = ir3_get_src(ctx, &intr->src[1])[0];

   store = ir3_STLW(b, offset, 0,
                    ir3_create_collect(b, value, intr->num_components), 0,
                    create_immed(b, intr->num_components), 0);

   /* for a650, use STL for vertex outputs used by tess ctrl shader: */
   if (ctx->so->type == MESA_SHADER_VERTEX && ctx->so->key.tessellation &&
       ctx->compiler->tess_use_shared)
      store->opc = OPC_STL;

   store->cat6.dst_offset = nir_intrinsic_base(intr);
   store->cat6.type = utype_src(intr->src[0]);
   store->barrier_class = IR3_BARRIER_SHARED_W;
   store->barrier_conflict = IR3_BARRIER_SHARED_R | IR3_BARRIER_SHARED_W;

   array_insert(b, b->keeps, store);
}

/*
 * CS shared variable atomic intrinsics
 *
 * All of the shared variable atomic memory operations read a value from
 * memory, compute a new value using one of the operations below, write the
 * new value to memory, and return the original value read.
 *
 * All operations take 2 sources except CompSwap that takes 3. These
 * sources represent:
 *
 * 0: The offset into the shared variable storage region that the atomic
 *    operation will operate on.
 * 1: The data parameter to the atomic function (i.e. the value to add
 *    in, etc).
 * 2: For CompSwap only: the second data parameter.
 */
static struct ir3_instruction *
emit_intrinsic_atomic_shared(struct ir3_context *ctx, nir_intrinsic_instr *intr)
{
   struct ir3_block *b = ctx->block;
   struct ir3_instruction *atomic, *src0, *src1;
   type_t type = TYPE_U32;

   src0 = ir3_get_src(ctx, &intr->src[0])[0]; /* offset */
   src1 = ir3_get_src(ctx, &intr->src[1])[0]; /* value */

   switch (nir_intrinsic_atomic_op(intr)) {
   case nir_atomic_op_iadd:
      atomic = ir3_ATOMIC_ADD(b, src0, 0, src1, 0);
      break;
   case nir_atomic_op_imin:
      atomic = ir3_ATOMIC_MIN(b, src0, 0, src1, 0);
      type = TYPE_S32;
      break;
   case nir_atomic_op_umin:
      atomic = ir3_ATOMIC_MIN(b, src0, 0, src1, 0);
      break;
   case nir_atomic_op_imax:
      atomic = ir3_ATOMIC_MAX(b, src0, 0, src1, 0);
      type = TYPE_S32;
      break;
   case nir_atomic_op_umax:
      atomic = ir3_ATOMIC_MAX(b, src0, 0, src1, 0);
      break;
   case nir_atomic_op_iand:
      atomic = ir3_ATOMIC_AND(b, src0, 0, src1, 0);
      break;
   case nir_atomic_op_ior:
      atomic = ir3_ATOMIC_OR(b, src0, 0, src1, 0);
      break;
   case nir_atomic_op_ixor:
      atomic = ir3_ATOMIC_XOR(b, src0, 0, src1, 0);
      break;
   case nir_atomic_op_xchg:
      atomic = ir3_ATOMIC_XCHG(b, src0, 0, src1, 0);
      break;
   case nir_atomic_op_cmpxchg:
      /* for cmpxchg, src1 is [ui]vec2(data, compare): */
      src1 = ir3_collect(b, ir3_get_src(ctx, &intr->src[2])[0], src1);
      atomic = ir3_ATOMIC_CMPXCHG(b, src0, 0, src1, 0);
      break;
   default:
      unreachable("boo");
   }

   atomic->cat6.iim_val = 1;
   atomic->cat6.d = 1;
   atomic->cat6.type = type;
   atomic->barrier_class = IR3_BARRIER_SHARED_W;
   atomic->barrier_conflict = IR3_BARRIER_SHARED_R | IR3_BARRIER_SHARED_W;

   /* even if nothing consume the result, we can't DCE the instruction: */
   array_insert(b, b->keeps, atomic);

   return atomic;
}

static void
stp_ldp_offset(struct ir3_context *ctx, nir_src *src,
               struct ir3_instruction **offset, int32_t *base)
{
   struct ir3_block *b = ctx->block;

   if (nir_src_is_const(*src)) {
      unsigned src_offset = nir_src_as_uint(*src);
      /* The base offset field is only 13 bits, and it's signed. Try to make the
       * offset constant whenever the original offsets are similar, to avoid
       * creating too many constants in the final shader.
       */
      *base = ((int32_t) src_offset << (32 - 13)) >> (32 - 13);
      uint32_t offset_val = src_offset - *base;
      *offset = create_immed(b, offset_val);
   } else {
      /* TODO: match on nir_iadd with a constant that fits */
      *base = 0;
      *offset = ir3_get_src(ctx, src)[0];
   }
}

/* src[] = { offset }. */
static void
emit_intrinsic_load_scratch(struct ir3_context *ctx, nir_intrinsic_instr *intr,
                            struct ir3_instruction **dst)
{
   struct ir3_block *b = ctx->block;
   struct ir3_instruction *ldp, *offset;
   int32_t base;

   stp_ldp_offset(ctx, &intr->src[0], &offset, &base);

   ldp = ir3_LDP(b, offset, 0, create_immed(b, base), 0,
                 create_immed(b, intr->num_components), 0);

   ldp->cat6.type = utype_def(&intr->def);
   ldp->dsts[0]->wrmask = MASK(intr->num_components);

   ldp->barrier_class = IR3_BARRIER_PRIVATE_R;
   ldp->barrier_conflict = IR3_BARRIER_PRIVATE_W;

   ir3_split_dest(b, dst, ldp, 0, intr->num_components);
}

/* src[] = { value, offset }. const_index[] = { write_mask } */
static void
emit_intrinsic_store_scratch(struct ir3_context *ctx, nir_intrinsic_instr *intr)
{
   struct ir3_block *b = ctx->block;
   struct ir3_instruction *stp, *offset;
   struct ir3_instruction *const *value;
   unsigned wrmask, ncomp;
   int32_t base;

   value = ir3_get_src(ctx, &intr->src[0]);

   stp_ldp_offset(ctx, &intr->src[1], &offset, &base);

   wrmask = nir_intrinsic_write_mask(intr);
   ncomp = ffs(~wrmask) - 1;

   assert(wrmask == BITFIELD_MASK(intr->num_components));

   stp = ir3_STP(b, offset, 0, ir3_create_collect(b, value, ncomp), 0,
                 create_immed(b, ncomp), 0);
   stp->cat6.dst_offset = base;
   stp->cat6.type = utype_src(intr->src[0]);
   stp->barrier_class = IR3_BARRIER_PRIVATE_W;
   stp->barrier_conflict = IR3_BARRIER_PRIVATE_R | IR3_BARRIER_PRIVATE_W;

   array_insert(b, b->keeps, stp);
}

struct tex_src_info {
   /* For prefetch */
   unsigned tex_base, samp_base, tex_idx, samp_idx;
   /* For normal tex instructions */
   unsigned base, a1_val, flags;
   struct ir3_instruction *samp_tex;
};

/* TODO handle actual indirect/dynamic case.. which is going to be weird
 * to handle with the image_mapping table..
 */
static struct tex_src_info
get_image_ssbo_samp_tex_src(struct ir3_context *ctx, nir_src *src, bool image)
{
   struct ir3_block *b = ctx->block;
   struct tex_src_info info = {0};
   nir_intrinsic_instr *bindless_tex = ir3_bindless_resource(*src);

   if (bindless_tex) {
      /* Bindless case */
      ctx->so->bindless_tex = true;
      info.flags |= IR3_INSTR_B;

      /* Gather information required to determine which encoding to
       * choose as well as for prefetch.
       */
      info.tex_base = nir_intrinsic_desc_set(bindless_tex);
      bool tex_const = nir_src_is_const(bindless_tex->src[0]);
      if (tex_const)
         info.tex_idx = nir_src_as_uint(bindless_tex->src[0]);
      info.samp_idx = 0;

      /* Choose encoding. */
      if (tex_const && info.tex_idx < 256) {
         if (info.tex_idx < 16) {
            /* Everything fits within the instruction */
            info.base = info.tex_base;
         } else {
            info.base = info.tex_base;
            if (ctx->compiler->gen <= 6) {
               info.a1_val = info.tex_idx << 3;
            } else {
               info.a1_val = info.samp_idx << 3;
            }
            info.flags |= IR3_INSTR_A1EN;
         }
         info.samp_tex = NULL;
      } else {
         info.flags |= IR3_INSTR_S2EN;
         info.base = info.tex_base;

         /* Note: the indirect source is now a vec2 instead of hvec2 */
         struct ir3_instruction *texture, *sampler;

         texture = ir3_get_src(ctx, src)[0];
         sampler = create_immed(b, 0);
         info.samp_tex = ir3_collect(b, texture, sampler);
      }
   } else {
      info.flags |= IR3_INSTR_S2EN;
      unsigned slot = nir_src_as_uint(*src);
      unsigned tex_idx = image ?
            ir3_image_to_tex(&ctx->so->image_mapping, slot) :
            ir3_ssbo_to_tex(&ctx->so->image_mapping, slot);
      struct ir3_instruction *texture, *sampler;

      ctx->so->num_samp = MAX2(ctx->so->num_samp, tex_idx + 1);

      texture = create_immed_typed(ctx->block, tex_idx, TYPE_U16);
      sampler = create_immed_typed(ctx->block, tex_idx, TYPE_U16);

      info.samp_tex = ir3_collect(b, sampler, texture);
   }

   return info;
}

static struct ir3_instruction *
emit_sam(struct ir3_context *ctx, opc_t opc, struct tex_src_info info,
         type_t type, unsigned wrmask, struct ir3_instruction *src0,
         struct ir3_instruction *src1)
{
   struct ir3_instruction *sam, *addr;
   if (info.flags & IR3_INSTR_A1EN) {
      addr = ir3_get_addr1(ctx, info.a1_val);
   }
   sam = ir3_SAM(ctx->block, opc, type, wrmask, info.flags, info.samp_tex, src0,
                 src1);
   if (info.flags & IR3_INSTR_A1EN) {
      ir3_instr_set_address(sam, addr);
   }
   if (info.flags & IR3_INSTR_B) {
      sam->cat5.tex_base = info.base;
      sam->cat5.samp = info.samp_idx;
      sam->cat5.tex  = info.tex_idx;
   }
   return sam;
}

/* src[] = { deref, coord, sample_index }. const_index[] = {} */
static void
emit_intrinsic_load_image(struct ir3_context *ctx, nir_intrinsic_instr *intr,
                          struct ir3_instruction **dst)
{
   /* If the image can be written, must use LDIB to retrieve data, rather than
    * through ISAM (which uses the texture cache and won't get previous writes).
    */
   if (!(nir_intrinsic_access(intr) & ACCESS_CAN_REORDER)) {
      ctx->funcs->emit_intrinsic_load_image(ctx, intr, dst);
      return;
   }

   /* The sparse set of texture descriptors for non-coherent load_images means we can't do indirection, so
    * fall back to coherent load.
    */
   if (ctx->compiler->gen >= 5 &&
       !ir3_bindless_resource(intr->src[0]) &&
       !nir_src_is_const(intr->src[0])) {
      ctx->funcs->emit_intrinsic_load_image(ctx, intr, dst);
      return;
   }

   struct ir3_block *b = ctx->block;
   struct tex_src_info info = get_image_ssbo_samp_tex_src(ctx, &intr->src[0], true);
   struct ir3_instruction *sam;
   struct ir3_instruction *const *src0 = ir3_get_src(ctx, &intr->src[1]);
   struct ir3_instruction *coords[4];
   unsigned flags, ncoords = ir3_get_image_coords(intr, &flags);
   type_t type = ir3_get_type_for_image_intrinsic(intr);

   info.flags |= flags;

   /* hw doesn't do 1d, so we treat it as 2d with height of 1, and patch up the
    * y coord. Note that the array index must come after the fake y coord.
    */
   enum glsl_sampler_dim dim = nir_intrinsic_image_dim(intr);
   if (dim == GLSL_SAMPLER_DIM_1D || dim == GLSL_SAMPLER_DIM_BUF) {
      coords[0] = src0[0];
      coords[1] = create_immed(b, 0);
      for (unsigned i = 1; i < ncoords; i++)
         coords[i + 1] = src0[i];
      ncoords++;
   } else {
      for (unsigned i = 0; i < ncoords; i++)
         coords[i] = src0[i];
   }

   sam = emit_sam(ctx, OPC_ISAM, info, type, 0b1111,
                  ir3_create_collect(b, coords, ncoords), NULL);

   ir3_handle_nonuniform(sam, intr);

   sam->barrier_class = IR3_BARRIER_IMAGE_R;
   sam->barrier_conflict = IR3_BARRIER_IMAGE_W;

   ir3_split_dest(b, dst, sam, 0, 4);
}

/* A4xx version of image_size, see ir3_a6xx.c for newer resinfo version. */
void
emit_intrinsic_image_size_tex(struct ir3_context *ctx,
                              nir_intrinsic_instr *intr,
                              struct ir3_instruction **dst)
{
   struct ir3_block *b = ctx->block;
   struct tex_src_info info = get_image_ssbo_samp_tex_src(ctx, &intr->src[0], true);
   struct ir3_instruction *sam, *lod;
   unsigned flags, ncoords = ir3_get_image_coords(intr, &flags);
   type_t dst_type = intr->def.bit_size == 16 ? TYPE_U16 : TYPE_U32;

   info.flags |= flags;
   assert(nir_src_as_uint(intr->src[1]) == 0);
   lod = create_immed(b, 0);
   sam = emit_sam(ctx, OPC_GETSIZE, info, dst_type, 0b1111, lod, NULL);

   /* Array size actually ends up in .w rather than .z. This doesn't
    * matter for miplevel 0, but for higher mips the value in z is
    * minified whereas w stays. Also, the value in TEX_CONST_3_DEPTH is
    * returned, which means that we have to add 1 to it for arrays for
    * a3xx.
    *
    * Note use a temporary dst and then copy, since the size of the dst
    * array that is passed in is based on nir's understanding of the
    * result size, not the hardware's
    */
   struct ir3_instruction *tmp[4];

   ir3_split_dest(b, tmp, sam, 0, 4);

   for (unsigned i = 0; i < ncoords; i++)
      dst[i] = tmp[i];

   if (flags & IR3_INSTR_A) {
      if (ctx->compiler->levels_add_one) {
         dst[ncoords - 1] = ir3_ADD_U(b, tmp[3], 0, create_immed(b, 1), 0);
      } else {
         dst[ncoords - 1] = ir3_MOV(b, tmp[3], TYPE_U32);
      }
   }
}

/* src[] = { buffer_index, offset }. No const_index */
static void
emit_intrinsic_load_ssbo(struct ir3_context *ctx,
                         nir_intrinsic_instr *intr,
                         struct ir3_instruction **dst)
{
   /* Note: isam currently can't handle vectorized loads/stores */
   if (!(nir_intrinsic_access(intr) & ACCESS_CAN_REORDER) ||
       intr->def.num_components > 1 ||
       !ctx->compiler->has_isam_ssbo) {
      ctx->funcs->emit_intrinsic_load_ssbo(ctx, intr, dst);
      return;
   }

   struct ir3_block *b = ctx->block;
   struct ir3_instruction *offset = ir3_get_src(ctx, &intr->src[2])[0];
   struct ir3_instruction *coords = ir3_collect(b, offset, create_immed(b, 0));
   struct tex_src_info info = get_image_ssbo_samp_tex_src(ctx, &intr->src[0], false);

   unsigned num_components = intr->def.num_components;
   struct ir3_instruction *sam =
      emit_sam(ctx, OPC_ISAM, info, utype_for_size(intr->def.bit_size),
               MASK(num_components), coords, NULL);

   ir3_handle_nonuniform(sam, intr);

   sam->barrier_class = IR3_BARRIER_BUFFER_R;
   sam->barrier_conflict = IR3_BARRIER_BUFFER_W;

   ir3_split_dest(b, dst, sam, 0, num_components);
}

static void
emit_control_barrier(struct ir3_context *ctx)
{
   /* Hull shaders dispatch 32 wide so an entire patch will always
    * fit in a single warp and execute in lock-step. Consequently,
    * we don't need to do anything for TCS barriers. Emitting
    * barrier instruction will deadlock.
    */
   if (ctx->so->type == MESA_SHADER_TESS_CTRL)
      return;

   struct ir3_block *b = ctx->block;
   struct ir3_instruction *barrier = ir3_BAR(b);
   barrier->cat7.g = true;
   if (ctx->compiler->gen < 6)
      barrier->cat7.l = true;
   barrier->flags = IR3_INSTR_SS | IR3_INSTR_SY;
   barrier->barrier_class = IR3_BARRIER_EVERYTHING;
   array_insert(b, b->keeps, barrier);

   ctx->so->has_barrier = true;
}

static void
emit_intrinsic_barrier(struct ir3_context *ctx, nir_intrinsic_instr *intr)
{
   struct ir3_block *b = ctx->block;
   struct ir3_instruction *barrier;

   /* TODO: find out why there is a major difference of .l usage
    * between a5xx and a6xx,
    */

   mesa_scope exec_scope = nir_intrinsic_execution_scope(intr);
   mesa_scope mem_scope = nir_intrinsic_memory_scope(intr);
   nir_variable_mode modes = nir_intrinsic_memory_modes(intr);
   /* loads/stores are always cache-coherent so we can filter out
    * available/visible.
    */
   nir_memory_semantics semantics =
      nir_intrinsic_memory_semantics(intr) & (NIR_MEMORY_ACQUIRE |
                                              NIR_MEMORY_RELEASE);

   if (ctx->so->type == MESA_SHADER_TESS_CTRL) {
      /* Remove mode corresponding to TCS patch barriers because hull shaders
       * dispatch 32 wide so an entire patch will always fit in a single warp
       * and execute in lock-step.
       *
       * TODO: memory barrier also tells us not to reorder stores, this
       * information is lost here (backend doesn't reorder stores so we
       * are safe for now).
       */
      modes &= ~nir_var_shader_out;
   }

   assert(!(modes & nir_var_shader_out));

   if ((modes & (nir_var_mem_shared | nir_var_mem_ssbo | nir_var_mem_global |
                 nir_var_image)) && semantics) {
      barrier = ir3_FENCE(b);
      barrier->cat7.r = true;
      barrier->cat7.w = true;

      if (modes & (nir_var_mem_ssbo | nir_var_image | nir_var_mem_global)) {
         barrier->cat7.g = true;
      }

      if (ctx->compiler->gen >= 6) {
         if (modes & (nir_var_mem_ssbo | nir_var_image)) {
            barrier->cat7.l = true;
         }
      } else {
         if (modes & (nir_var_mem_shared | nir_var_mem_ssbo | nir_var_image)) {
            barrier->cat7.l = true;
         }
      }

      barrier->barrier_class = 0;
      barrier->barrier_conflict = 0;

      if (modes & nir_var_mem_shared) {
         barrier->barrier_class |= IR3_BARRIER_SHARED_W;
         barrier->barrier_conflict |=
            IR3_BARRIER_SHARED_R | IR3_BARRIER_SHARED_W;
      }

      if (modes & (nir_var_mem_ssbo | nir_var_mem_global)) {
         barrier->barrier_class |= IR3_BARRIER_BUFFER_W;
         barrier->barrier_conflict |=
            IR3_BARRIER_BUFFER_R | IR3_BARRIER_BUFFER_W;
      }

      if (modes & nir_var_image) {
         barrier->barrier_class |= IR3_BARRIER_IMAGE_W;
         barrier->barrier_conflict |=
            IR3_BARRIER_IMAGE_W | IR3_BARRIER_IMAGE_R;
      }

      /* make sure barrier doesn't get DCE'd */
      array_insert(b, b->keeps, barrier);

      if (ctx->compiler->gen >= 7 && mem_scope > SCOPE_WORKGROUP &&
          modes & (nir_var_mem_ssbo | nir_var_image) &&
          semantics & NIR_MEMORY_ACQUIRE) {
         /* "r + l" is not enough to synchronize reads with writes from other
          * workgroups, we can disable them since they are useless here.
          */
         barrier->cat7.r = false;
         barrier->cat7.l = false;

         struct ir3_instruction *ccinv = ir3_CCINV(b);
         /* A7XX TODO: ccinv should just stick to the barrier,
          * the barrier class/conflict introduces unnecessary waits.
          */
         ccinv->barrier_class = barrier->barrier_class;
         ccinv->barrier_conflict = barrier->barrier_conflict;
         array_insert(b, b->keeps, ccinv);
      }
   }

   if (exec_scope >= SCOPE_WORKGROUP) {
      emit_control_barrier(ctx);
   }
}

static void
add_sysval_input_compmask(struct ir3_context *ctx, gl_system_value slot,
                          unsigned compmask, struct ir3_instruction *instr)
{
   struct ir3_shader_variant *so = ctx->so;
   unsigned n = so->inputs_count++;

   assert(instr->opc == OPC_META_INPUT);
   instr->input.inidx = n;
   instr->input.sysval = slot;

   so->inputs[n].sysval = true;
   so->inputs[n].slot = slot;
   so->inputs[n].compmask = compmask;
   so->total_in++;

   so->sysval_in += util_last_bit(compmask);
}

static struct ir3_instruction *
create_sysval_input(struct ir3_context *ctx, gl_system_value slot,
                    unsigned compmask)
{
   assert(compmask);
   struct ir3_instruction *sysval = create_input(ctx, compmask);
   add_sysval_input_compmask(ctx, slot, compmask, sysval);
   return sysval;
}

static struct ir3_instruction *
get_barycentric(struct ir3_context *ctx, enum ir3_bary bary)
{
   STATIC_ASSERT(SYSTEM_VALUE_BARYCENTRIC_PERSP_PIXEL + IJ_PERSP_PIXEL ==
                 SYSTEM_VALUE_BARYCENTRIC_PERSP_PIXEL);
   STATIC_ASSERT(SYSTEM_VALUE_BARYCENTRIC_PERSP_PIXEL + IJ_PERSP_SAMPLE ==
                 SYSTEM_VALUE_BARYCENTRIC_PERSP_SAMPLE);
   STATIC_ASSERT(SYSTEM_VALUE_BARYCENTRIC_PERSP_PIXEL + IJ_PERSP_CENTROID ==
                 SYSTEM_VALUE_BARYCENTRIC_PERSP_CENTROID);
   STATIC_ASSERT(SYSTEM_VALUE_BARYCENTRIC_PERSP_PIXEL + IJ_PERSP_CENTER_RHW ==
                 SYSTEM_VALUE_BARYCENTRIC_PERSP_CENTER_RHW);
   STATIC_ASSERT(SYSTEM_VALUE_BARYCENTRIC_PERSP_PIXEL + IJ_LINEAR_PIXEL ==
                 SYSTEM_VALUE_BARYCENTRIC_LINEAR_PIXEL);
   STATIC_ASSERT(SYSTEM_VALUE_BARYCENTRIC_PERSP_PIXEL + IJ_LINEAR_CENTROID ==
                 SYSTEM_VALUE_BARYCENTRIC_LINEAR_CENTROID);
   STATIC_ASSERT(SYSTEM_VALUE_BARYCENTRIC_PERSP_PIXEL + IJ_LINEAR_SAMPLE ==
                 SYSTEM_VALUE_BARYCENTRIC_LINEAR_SAMPLE);

   if (!ctx->ij[bary]) {
      struct ir3_instruction *xy[2];
      struct ir3_instruction *ij;

      ij = create_sysval_input(ctx, SYSTEM_VALUE_BARYCENTRIC_PERSP_PIXEL +
                               bary, 0x3);
      ir3_split_dest(ctx->in_block, xy, ij, 0, 2);

      ctx->ij[bary] = ir3_create_collect(ctx->in_block, xy, 2);
   }

   return ctx->ij[bary];
}

/* TODO: make this a common NIR helper?
 * there is a nir_system_value_from_intrinsic but it takes nir_intrinsic_op so
 * it can't be extended to work with this
 */
static gl_system_value
nir_intrinsic_barycentric_sysval(nir_intrinsic_instr *intr)
{
   enum glsl_interp_mode interp_mode = nir_intrinsic_interp_mode(intr);
   gl_system_value sysval;

   switch (intr->intrinsic) {
   case nir_intrinsic_load_barycentric_pixel:
      if (interp_mode == INTERP_MODE_NOPERSPECTIVE)
         sysval = SYSTEM_VALUE_BARYCENTRIC_LINEAR_PIXEL;
      else
         sysval = SYSTEM_VALUE_BARYCENTRIC_PERSP_PIXEL;
      break;
   case nir_intrinsic_load_barycentric_centroid:
      if (interp_mode == INTERP_MODE_NOPERSPECTIVE)
         sysval = SYSTEM_VALUE_BARYCENTRIC_LINEAR_CENTROID;
      else
         sysval = SYSTEM_VALUE_BARYCENTRIC_PERSP_CENTROID;
      break;
   case nir_intrinsic_load_barycentric_sample:
      if (interp_mode == INTERP_MODE_NOPERSPECTIVE)
         sysval = SYSTEM_VALUE_BARYCENTRIC_LINEAR_SAMPLE;
      else
         sysval = SYSTEM_VALUE_BARYCENTRIC_PERSP_SAMPLE;
      break;
   default:
      unreachable("invalid barycentric intrinsic");
   }

   return sysval;
}

static void
emit_intrinsic_barycentric(struct ir3_context *ctx, nir_intrinsic_instr *intr,
                           struct ir3_instruction **dst)
{
   gl_system_value sysval = nir_intrinsic_barycentric_sysval(intr);

   if (!ctx->so->key.msaa && ctx->compiler->gen < 6) {
      switch (sysval) {
      case SYSTEM_VALUE_BARYCENTRIC_PERSP_SAMPLE:
         sysval = SYSTEM_VALUE_BARYCENTRIC_PERSP_PIXEL;
         break;
      case SYSTEM_VALUE_BARYCENTRIC_PERSP_CENTROID:
         sysval = SYSTEM_VALUE_BARYCENTRIC_PERSP_PIXEL;
         break;
      case SYSTEM_VALUE_BARYCENTRIC_LINEAR_SAMPLE:
         sysval = SYSTEM_VALUE_BARYCENTRIC_LINEAR_PIXEL;
         break;
      case SYSTEM_VALUE_BARYCENTRIC_LINEAR_CENTROID:
         sysval = SYSTEM_VALUE_BARYCENTRIC_LINEAR_PIXEL;
         break;
      default:
         break;
      }
   }

   enum ir3_bary bary = sysval - SYSTEM_VALUE_BARYCENTRIC_PERSP_PIXEL;

   struct ir3_instruction *ij = get_barycentric(ctx, bary);
   ir3_split_dest(ctx->block, dst, ij, 0, 2);
}

static struct ir3_instruction *
get_frag_coord(struct ir3_context *ctx, nir_intrinsic_instr *intr)
{
   if (!ctx->frag_coord) {
      struct ir3_block *b = ir3_after_preamble(ctx->ir);
      struct ir3_instruction *xyzw[4];
      struct ir3_instruction *hw_frag_coord;

      hw_frag_coord = create_sysval_input(ctx, SYSTEM_VALUE_FRAG_COORD, 0xf);
      ir3_split_dest(b, xyzw, hw_frag_coord, 0, 4);

      /* for frag_coord.xy, we get unsigned values.. we need
       * to subtract (integer) 8 and divide by 16 (right-
       * shift by 4) then convert to float:
       *
       *    sub.s tmp, src, 8
       *    shr.b tmp, tmp, 4
       *    mov.u32f32 dst, tmp
       *
       */
      for (int i = 0; i < 2; i++) {
         xyzw[i] = ir3_COV(b, xyzw[i], TYPE_U32, TYPE_F32);
         xyzw[i] =
            ir3_MUL_F(b, xyzw[i], 0, create_immed(b, fui(1.0 / 16.0)), 0);
      }

      ctx->frag_coord = ir3_create_collect(b, xyzw, 4);
   }

   ctx->so->fragcoord_compmask |= nir_def_components_read(&intr->def);

   return ctx->frag_coord;
}

/* This is a bit of a hack until ir3_context is converted to store SSA values
 * as ir3_register's instead of ir3_instruction's. Pick out a given destination
 * of an instruction with multiple destinations using a mov that will get folded
 * away by ir3_cp.
 */
static struct ir3_instruction *
create_multidst_mov(struct ir3_block *block, struct ir3_register *dst)
{
   struct ir3_instruction *mov = ir3_instr_create(block, OPC_MOV, 1, 1);
   unsigned dst_flags = dst->flags & IR3_REG_HALF;
   unsigned src_flags = dst->flags & (IR3_REG_HALF | IR3_REG_SHARED);

   __ssa_dst(mov)->flags |= dst_flags;
   struct ir3_register *src =
      ir3_src_create(mov, INVALID_REG, IR3_REG_SSA | src_flags);
   src->wrmask = dst->wrmask;
   src->def = dst;
   assert(!(dst->flags & IR3_REG_RELATIV));
   mov->cat1.src_type = mov->cat1.dst_type =
      (dst->flags & IR3_REG_HALF) ? TYPE_U16 : TYPE_U32;
   return mov;
}

static reduce_op_t
get_reduce_op(nir_op opc)
{
   switch (opc) {
   case nir_op_iadd: return REDUCE_OP_ADD_U;
   case nir_op_fadd: return REDUCE_OP_ADD_F;
   case nir_op_imul: return REDUCE_OP_MUL_U;
   case nir_op_fmul: return REDUCE_OP_MUL_F;
   case nir_op_umin: return REDUCE_OP_MIN_U;
   case nir_op_imin: return REDUCE_OP_MIN_S;
   case nir_op_fmin: return REDUCE_OP_MIN_F;
   case nir_op_umax: return REDUCE_OP_MAX_U;
   case nir_op_imax: return REDUCE_OP_MAX_S;
   case nir_op_fmax: return REDUCE_OP_MAX_F;
   case nir_op_iand: return REDUCE_OP_AND_B;
   case nir_op_ior:  return REDUCE_OP_OR_B;
   case nir_op_ixor: return REDUCE_OP_XOR_B;
   default:
      unreachable("unknown NIR reduce op");
   }
}

static uint32_t
get_reduce_identity(nir_op opc, unsigned size)
{
   switch (opc) {
   case nir_op_iadd:
      return 0;
   case nir_op_fadd: 
      return size == 32 ? fui(0.0f) : _mesa_float_to_half(0.0f);
   case nir_op_imul:
      return 1;
   case nir_op_fmul:
      return size == 32 ? fui(1.0f) : _mesa_float_to_half(1.0f);
   case nir_op_umax:
      return 0;
   case nir_op_imax:
      return size == 32 ? INT32_MIN : (uint32_t)INT16_MIN;
   case nir_op_fmax:
      return size == 32 ? fui(-INFINITY) : _mesa_float_to_half(-INFINITY);
   case nir_op_umin:
      return size == 32 ? UINT32_MAX : UINT16_MAX;
   case nir_op_imin:
      return size == 32 ? INT32_MAX : (uint32_t)INT16_MAX;
   case nir_op_fmin:
      return size == 32 ? fui(INFINITY) : _mesa_float_to_half(INFINITY);
   case nir_op_iand:
      return size == 32 ? ~0 : (size == 16 ? (uint32_t)(uint16_t)~0 : 1);
   case nir_op_ior:
      return 0;
   case nir_op_ixor:
      return 0;
   default:
      unreachable("unknown NIR reduce op");
   }
}

static struct ir3_instruction *
emit_intrinsic_reduce(struct ir3_context *ctx, nir_intrinsic_instr *intr)
{
   struct ir3_instruction *src = ir3_get_src(ctx, &intr->src[0])[0];
   nir_op nir_reduce_op = (nir_op) nir_intrinsic_reduction_op(intr);
   reduce_op_t reduce_op = get_reduce_op(nir_reduce_op);
   unsigned dst_size = intr->def.bit_size;
   unsigned flags = (ir3_bitsize(ctx, dst_size) == 16) ? IR3_REG_HALF : 0;

   /* Note: the shared reg is initialized to the identity, so we need it to
    * always be 32-bit even when the source isn't because half shared regs are
    * not supported.
    */
   struct ir3_instruction *identity =
      create_immed(ctx->block, get_reduce_identity(nir_reduce_op, dst_size));
   identity = ir3_READ_FIRST_MACRO(ctx->block, identity, 0);
   identity->dsts[0]->flags |= IR3_REG_SHARED;

   /* OPC_SCAN_MACRO has the following destinations:
    * - Exclusive scan result (interferes with source)
    * - Inclusive scan result
    * - Shared reg reduction result, must be initialized to the identity
    *
    * The loop computes all three results at the same time, we just have to
    * choose which destination to return.
    */
   struct ir3_instruction *scan =
      ir3_instr_create(ctx->block, OPC_SCAN_MACRO, 3, 2);
   scan->cat1.reduce_op = reduce_op;

   struct ir3_register *exclusive = __ssa_dst(scan);
   exclusive->flags |= flags | IR3_REG_EARLY_CLOBBER;
   struct ir3_register *inclusive = __ssa_dst(scan);
   inclusive->flags |= flags;
   struct ir3_register *reduce = __ssa_dst(scan);
   reduce->flags |= IR3_REG_SHARED;

   /* The 32-bit multiply macro reads its sources after writing a partial result
    * to the destination, therefore inclusive also interferes with the source.
    */
   if (reduce_op == REDUCE_OP_MUL_U && dst_size == 32)
      inclusive->flags |= IR3_REG_EARLY_CLOBBER;

   /* Normal source */
   __ssa_src(scan, src, 0);

   /* shared reg tied source */
   struct ir3_register *reduce_init = __ssa_src(scan, identity, IR3_REG_SHARED);
   ir3_reg_tie(reduce, reduce_init);
   
   struct ir3_register *dst;
   switch (intr->intrinsic) {
   case nir_intrinsic_reduce: dst = reduce; break;
   case nir_intrinsic_inclusive_scan: dst = inclusive; break;
   case nir_intrinsic_exclusive_scan: dst = exclusive; break;
   default:
      unreachable("unknown reduce intrinsic");
   }

   return create_multidst_mov(ctx->block, dst);
}

static void setup_input(struct ir3_context *ctx, nir_intrinsic_instr *intr);
static void setup_output(struct ir3_context *ctx, nir_intrinsic_instr *intr);

static void
emit_intrinsic(struct ir3_context *ctx, nir_intrinsic_instr *intr)
{
   const nir_intrinsic_info *info = &nir_intrinsic_infos[intr->intrinsic];
   struct ir3_instruction **dst;
   struct ir3_instruction *const *src;
   struct ir3_block *b = ctx->block;
   unsigned dest_components = nir_intrinsic_dest_components(intr);
   int idx;

   if (info->has_dest) {
      dst = ir3_get_def(ctx, &intr->def, dest_components);
   } else {
      dst = NULL;
   }

   const struct ir3_const_state *const_state = ir3_const_state(ctx->so);
   const unsigned primitive_param = const_state->offsets.primitive_param * 4;
   const unsigned primitive_map = const_state->offsets.primitive_map * 4;

   switch (intr->intrinsic) {
   case nir_intrinsic_decl_reg:
      /* There's logically nothing to do, but this has a destination in NIR so
       * plug in something... It will get DCE'd.
       */
      dst[0] = create_immed(ctx->block, 0);
      break;

   case nir_intrinsic_load_reg:
   case nir_intrinsic_load_reg_indirect: {
      struct ir3_array *arr = ir3_get_array(ctx, intr->src[0].ssa);
      struct ir3_instruction *addr = NULL;

      if (intr->intrinsic == nir_intrinsic_load_reg_indirect) {
         addr = ir3_get_addr0(ctx, ir3_get_src(ctx, &intr->src[1])[0],
                              dest_components);
      }

      ASSERTED nir_intrinsic_instr *decl = nir_reg_get_decl(intr->src[0].ssa);
      assert(dest_components == nir_intrinsic_num_components(decl));

      for (unsigned i = 0; i < dest_components; i++) {
         unsigned n = nir_intrinsic_base(intr) * dest_components + i;
         compile_assert(ctx, n < arr->length);
         dst[i] = ir3_create_array_load(ctx, arr, n, addr);
      }

      break;
   }

   case nir_intrinsic_store_reg:
   case nir_intrinsic_store_reg_indirect: {
      struct ir3_array *arr = ir3_get_array(ctx, intr->src[1].ssa);
      unsigned num_components = nir_src_num_components(intr->src[0]);
      struct ir3_instruction *addr = NULL;

      ASSERTED nir_intrinsic_instr *decl = nir_reg_get_decl(intr->src[1].ssa);
      assert(num_components == nir_intrinsic_num_components(decl));

      struct ir3_instruction *const *value = ir3_get_src(ctx, &intr->src[0]);

      if (intr->intrinsic == nir_intrinsic_store_reg_indirect) {
         addr = ir3_get_addr0(ctx, ir3_get_src(ctx, &intr->src[2])[0],
                              num_components);
      }

      u_foreach_bit(i, nir_intrinsic_write_mask(intr)) {
         assert(i < num_components);

         unsigned n = nir_intrinsic_base(intr) * num_components + i;
         compile_assert(ctx, n < arr->length);
         if (value[i])
            ir3_create_array_store(ctx, arr, n, value[i], addr);
      }

      break;
   }

   case nir_intrinsic_load_uniform:
      idx = nir_intrinsic_base(intr);
      if (nir_src_is_const(intr->src[0])) {
         idx += nir_src_as_uint(intr->src[0]);
         for (int i = 0; i < dest_components; i++) {
            dst[i] = create_uniform_typed(
               b, idx + i,
               intr->def.bit_size == 16 ? TYPE_F16 : TYPE_F32);
         }
      } else {
         src = ir3_get_src(ctx, &intr->src[0]);
         for (int i = 0; i < dest_components; i++) {
            dst[i] = create_uniform_indirect(
               b, idx + i,
               intr->def.bit_size == 16 ? TYPE_F16 : TYPE_F32,
               ir3_get_addr0(ctx, src[0], 1));
         }
         /* NOTE: if relative addressing is used, we set
          * constlen in the compiler (to worst-case value)
          * since we don't know in the assembler what the max
          * addr reg value can be:
          */
         ctx->so->constlen =
            MAX2(ctx->so->constlen,
                 ctx->so->shader_options.num_reserved_user_consts +
                 const_state->ubo_state.size / 16);
      }
      break;

   case nir_intrinsic_load_vs_primitive_stride_ir3:
      dst[0] = create_uniform(b, primitive_param + 0);
      break;
   case nir_intrinsic_load_vs_vertex_stride_ir3:
      dst[0] = create_uniform(b, primitive_param + 1);
      break;
   case nir_intrinsic_load_hs_patch_stride_ir3:
      dst[0] = create_uniform(b, primitive_param + 2);
      break;
   case nir_intrinsic_load_patch_vertices_in:
      dst[0] = create_uniform(b, primitive_param + 3);
      break;
   case nir_intrinsic_load_tess_param_base_ir3:
      dst[0] = create_uniform(b, primitive_param + 4);
      dst[1] = create_uniform(b, primitive_param + 5);
      break;
   case nir_intrinsic_load_tess_factor_base_ir3:
      dst[0] = create_uniform(b, primitive_param + 6);
      dst[1] = create_uniform(b, primitive_param + 7);
      break;

   case nir_intrinsic_load_primitive_location_ir3:
      idx = nir_intrinsic_driver_location(intr);
      dst[0] = create_uniform(b, primitive_map + idx);
      break;

   case nir_intrinsic_load_gs_header_ir3:
      dst[0] = ctx->gs_header;
      break;
   case nir_intrinsic_load_tcs_header_ir3:
      dst[0] = ctx->tcs_header;
      break;

   case nir_intrinsic_load_rel_patch_id_ir3:
      dst[0] = ctx->rel_patch_id;
      break;

   case nir_intrinsic_load_primitive_id:
      if (!ctx->primitive_id) {
         ctx->primitive_id =
            create_sysval_input(ctx, SYSTEM_VALUE_PRIMITIVE_ID, 0x1);
      }
      dst[0] = ctx->primitive_id;
      break;

   case nir_intrinsic_load_tess_coord_xy:
      if (!ctx->tess_coord) {
         ctx->tess_coord =
            create_sysval_input(ctx, SYSTEM_VALUE_TESS_COORD, 0x3);
      }
      ir3_split_dest(b, dst, ctx->tess_coord, 0, 2);
      break;

   case nir_intrinsic_end_patch_ir3:
      assert(ctx->so->type == MESA_SHADER_TESS_CTRL);
      struct ir3_instruction *end = ir3_PREDE(b);
      array_insert(b, b->keeps, end);

      end->barrier_class = IR3_BARRIER_EVERYTHING;
      end->barrier_conflict = IR3_BARRIER_EVERYTHING;
      break;

   case nir_intrinsic_store_global_ir3:
      ctx->funcs->emit_intrinsic_store_global_ir3(ctx, intr);
      break;
   case nir_intrinsic_load_global_ir3:
      ctx->funcs->emit_intrinsic_load_global_ir3(ctx, intr, dst);
      break;

   case nir_intrinsic_load_ubo:
      emit_intrinsic_load_ubo(ctx, intr, dst);
      break;
   case nir_intrinsic_load_ubo_vec4:
      emit_intrinsic_load_ubo_ldc(ctx, intr, dst);
      break;
   case nir_intrinsic_copy_ubo_to_uniform_ir3:
      emit_intrinsic_copy_ubo_to_uniform(ctx, intr);
      break;
   case nir_intrinsic_load_frag_coord:
   case nir_intrinsic_load_frag_coord_unscaled_ir3:
      ir3_split_dest(b, dst, get_frag_coord(ctx, intr), 0, 4);
      break;
   case nir_intrinsic_load_sample_pos_from_id: {
      /* NOTE: blob seems to always use TYPE_F16 and then cov.f16f32,
       * but that doesn't seem necessary.
       */
      struct ir3_instruction *offset =
         ir3_RGETPOS(b, ir3_get_src(ctx, &intr->src[0])[0], 0);
      offset->dsts[0]->wrmask = 0x3;
      offset->cat5.type = TYPE_F32;

      ir3_split_dest(b, dst, offset, 0, 2);

      break;
   }
   case nir_intrinsic_load_persp_center_rhw_ir3:
      if (!ctx->ij[IJ_PERSP_CENTER_RHW]) {
         ctx->ij[IJ_PERSP_CENTER_RHW] =
            create_sysval_input(ctx, SYSTEM_VALUE_BARYCENTRIC_PERSP_CENTER_RHW, 0x1);
      }
      dst[0] = ctx->ij[IJ_PERSP_CENTER_RHW];
      break;
   case nir_intrinsic_load_barycentric_centroid:
   case nir_intrinsic_load_barycentric_sample:
   case nir_intrinsic_load_barycentric_pixel:
      emit_intrinsic_barycentric(ctx, intr, dst);
      break;
   case nir_intrinsic_load_interpolated_input:
   case nir_intrinsic_load_input:
      setup_input(ctx, intr);
      break;
   case nir_intrinsic_load_kernel_input:
      emit_intrinsic_load_kernel_input(ctx, intr, dst);
      break;
   /* All SSBO intrinsics should have been lowered by 'lower_io_offsets'
    * pass and replaced by an ir3-specifc version that adds the
    * dword-offset in the last source.
    */
   case nir_intrinsic_load_ssbo_ir3:
      emit_intrinsic_load_ssbo(ctx, intr, dst);
      break;
   case nir_intrinsic_store_ssbo_ir3:
      ctx->funcs->emit_intrinsic_store_ssbo(ctx, intr);
      break;
   case nir_intrinsic_get_ssbo_size:
      emit_intrinsic_ssbo_size(ctx, intr, dst);
      break;
   case nir_intrinsic_ssbo_atomic_ir3:
   case nir_intrinsic_ssbo_atomic_swap_ir3:
      dst[0] = ctx->funcs->emit_intrinsic_atomic_ssbo(ctx, intr);
      break;
   case nir_intrinsic_load_shared:
      emit_intrinsic_load_shared(ctx, intr, dst);
      break;
   case nir_intrinsic_store_shared:
      emit_intrinsic_store_shared(ctx, intr);
      break;
   case nir_intrinsic_shared_atomic:
   case nir_intrinsic_shared_atomic_swap:
      dst[0] = emit_intrinsic_atomic_shared(ctx, intr);
      break;
   case nir_intrinsic_load_scratch:
      emit_intrinsic_load_scratch(ctx, intr, dst);
      break;
   case nir_intrinsic_store_scratch:
      emit_intrinsic_store_scratch(ctx, intr);
      break;
   case nir_intrinsic_image_load:
   case nir_intrinsic_bindless_image_load:
      emit_intrinsic_load_image(ctx, intr, dst);
      break;
   case nir_intrinsic_image_store:
   case nir_intrinsic_bindless_image_store:
      ctx->funcs->emit_intrinsic_store_image(ctx, intr);
      break;
   case nir_intrinsic_image_size:
   case nir_intrinsic_bindless_image_size:
      ctx->funcs->emit_intrinsic_image_size(ctx, intr, dst);
      break;
   case nir_intrinsic_image_atomic:
   case nir_intrinsic_bindless_image_atomic:
   case nir_intrinsic_image_atomic_swap:
   case nir_intrinsic_bindless_image_atomic_swap:
      dst[0] = ctx->funcs->emit_intrinsic_atomic_image(ctx, intr);
      break;
   case nir_intrinsic_barrier:
      emit_intrinsic_barrier(ctx, intr);
      /* note that blk ptr no longer valid, make that obvious: */
      b = NULL;
      break;
   case nir_intrinsic_store_output:
      setup_output(ctx, intr);
      break;
   case nir_intrinsic_load_base_vertex:
   case nir_intrinsic_load_first_vertex:
      if (!ctx->basevertex) {
         ctx->basevertex = create_driver_param(ctx, IR3_DP_VTXID_BASE);
      }
      dst[0] = ctx->basevertex;
      break;
   case nir_intrinsic_load_is_indexed_draw:
      if (!ctx->is_indexed_draw) {
         ctx->is_indexed_draw = create_driver_param(ctx, IR3_DP_IS_INDEXED_DRAW);
      }
      dst[0] = ctx->is_indexed_draw;
      break;
   case nir_intrinsic_load_draw_id:
      if (!ctx->draw_id) {
         ctx->draw_id = create_driver_param(ctx, IR3_DP_DRAWID);
      }
      dst[0] = ctx->draw_id;
      break;
   case nir_intrinsic_load_base_instance:
      if (!ctx->base_instance) {
         ctx->base_instance = create_driver_param(ctx, IR3_DP_INSTID_BASE);
      }
      dst[0] = ctx->base_instance;
      break;
   case nir_intrinsic_load_view_index:
      if (!ctx->view_index) {
         ctx->view_index =
            create_sysval_input(ctx, SYSTEM_VALUE_VIEW_INDEX, 0x1);
      }
      dst[0] = ctx->view_index;
      break;
   case nir_intrinsic_load_vertex_id_zero_base:
   case nir_intrinsic_load_vertex_id:
      if (!ctx->vertex_id) {
         gl_system_value sv = (intr->intrinsic == nir_intrinsic_load_vertex_id)
                                 ? SYSTEM_VALUE_VERTEX_ID
                                 : SYSTEM_VALUE_VERTEX_ID_ZERO_BASE;
         ctx->vertex_id = create_sysval_input(ctx, sv, 0x1);
      }
      dst[0] = ctx->vertex_id;
      break;
   case nir_intrinsic_load_instance_id:
      if (!ctx->instance_id) {
         ctx->instance_id =
            create_sysval_input(ctx, SYSTEM_VALUE_INSTANCE_ID, 0x1);
      }
      dst[0] = ctx->instance_id;
      break;
   case nir_intrinsic_load_sample_id:
   case nir_intrinsic_load_sample_id_no_per_sample:
      if (!ctx->samp_id) {
         ctx->samp_id = create_sysval_input(ctx, SYSTEM_VALUE_SAMPLE_ID, 0x1);
         ctx->samp_id->dsts[0]->flags |= IR3_REG_HALF;
      }
      dst[0] = ir3_COV(b, ctx->samp_id, TYPE_U16, TYPE_U32);
      break;
   case nir_intrinsic_load_sample_mask_in:
      if (!ctx->samp_mask_in) {
         ctx->samp_mask_in =
            create_sysval_input(ctx, SYSTEM_VALUE_SAMPLE_MASK_IN, 0x1);
      }
      dst[0] = ctx->samp_mask_in;
      break;
   case nir_intrinsic_load_user_clip_plane:
      idx = nir_intrinsic_ucp_id(intr);
      for (int i = 0; i < dest_components; i++) {
         unsigned n = idx * 4 + i;
         dst[i] = create_driver_param(ctx, IR3_DP_UCP0_X + n);
      }
      break;
   case nir_intrinsic_load_front_face:
      if (!ctx->frag_face) {
         ctx->so->frag_face = true;
         ctx->frag_face =
            create_sysval_input(ctx, SYSTEM_VALUE_FRONT_FACE, 0x1);
         ctx->frag_face->dsts[0]->flags |= IR3_REG_HALF;
      }
      /* for fragface, we get -1 for back and 0 for front. However this is
       * the inverse of what nir expects (where ~0 is true).
       */
      dst[0] = ir3_CMPS_S(b, ctx->frag_face, 0,
                          create_immed_typed(b, 0, TYPE_U16), 0);
      dst[0]->cat2.condition = IR3_COND_EQ;
      break;
   case nir_intrinsic_load_local_invocation_id:
      if (!ctx->local_invocation_id) {
         ctx->local_invocation_id =
            create_sysval_input(ctx, SYSTEM_VALUE_LOCAL_INVOCATION_ID, 0x7);
      }
      ir3_split_dest(b, dst, ctx->local_invocation_id, 0, 3);
      break;
   case nir_intrinsic_load_workgroup_id:
   case nir_intrinsic_load_workgroup_id_zero_base:
      if (ctx->compiler->has_shared_regfile) {
         if (!ctx->work_group_id) {
            ctx->work_group_id =
               create_sysval_input(ctx, SYSTEM_VALUE_WORKGROUP_ID, 0x7);
            ctx->work_group_id->dsts[0]->flags |= IR3_REG_SHARED;
         }
         ir3_split_dest(b, dst, ctx->work_group_id, 0, 3);
      } else {
         /* For a3xx/a4xx, this comes in via const injection by the hw */
         for (int i = 0; i < dest_components; i++) {
            dst[i] = create_driver_param(ctx, IR3_DP_WORKGROUP_ID_X + i);
         }
      }
      break;
   case nir_intrinsic_load_base_workgroup_id:
      for (int i = 0; i < dest_components; i++) {
         dst[i] = create_driver_param(ctx, IR3_DP_BASE_GROUP_X + i);
      }
      break;
   case nir_intrinsic_load_num_workgroups:
      for (int i = 0; i < dest_components; i++) {
         dst[i] = create_driver_param(ctx, IR3_DP_NUM_WORK_GROUPS_X + i);
      }
      break;
   case nir_intrinsic_load_workgroup_size:
      for (int i = 0; i < dest_components; i++) {
         dst[i] = create_driver_param(ctx, IR3_DP_LOCAL_GROUP_SIZE_X + i);
      }
      break;
   case nir_intrinsic_load_subgroup_size: {
      assert(ctx->so->type == MESA_SHADER_COMPUTE ||
             ctx->so->type == MESA_SHADER_FRAGMENT);
      enum ir3_driver_param size = ctx->so->type == MESA_SHADER_COMPUTE ?
         IR3_DP_CS_SUBGROUP_SIZE : IR3_DP_FS_SUBGROUP_SIZE;
      dst[0] = create_driver_param(ctx, size);
      break;
   }
   case nir_intrinsic_load_subgroup_id_shift_ir3:
      dst[0] = create_driver_param(ctx, IR3_DP_SUBGROUP_ID_SHIFT);
      break;
   case nir_intrinsic_load_work_dim:
      dst[0] = create_driver_param(ctx, IR3_DP_WORK_DIM);
      break;
   case nir_intrinsic_load_subgroup_invocation:
      assert(ctx->compiler->has_getfiberid);
      dst[0] = ir3_GETFIBERID(b);
      dst[0]->cat6.type = TYPE_U32;
      __ssa_dst(dst[0]);
      break;
   case nir_intrinsic_load_tess_level_outer_default:
      for (int i = 0; i < dest_components; i++) {
         dst[i] = create_driver_param(ctx, IR3_DP_HS_DEFAULT_OUTER_LEVEL_X + i);
      }
      break;
   case nir_intrinsic_load_tess_level_inner_default:
      for (int i = 0; i < dest_components; i++) {
         dst[i] = create_driver_param(ctx, IR3_DP_HS_DEFAULT_INNER_LEVEL_X + i);
      }
      break;
   case nir_intrinsic_load_frag_invocation_count:
      dst[0] = create_driver_param(ctx, IR3_DP_FS_FRAG_INVOCATION_COUNT);
      break;
   case nir_intrinsic_load_frag_size_ir3:
   case nir_intrinsic_load_frag_offset_ir3: {
      enum ir3_driver_param param =
         intr->intrinsic == nir_intrinsic_load_frag_size_ir3 ?
         IR3_DP_FS_FRAG_SIZE : IR3_DP_FS_FRAG_OFFSET;
      if (nir_src_is_const(intr->src[0])) {
         uint32_t view = nir_src_as_uint(intr->src[0]);
         for (int i = 0; i < dest_components; i++) {
            dst[i] = create_driver_param(ctx, param + 4 * view + i);
         }
      } else {
         struct ir3_instruction *view = ir3_get_src(ctx, &intr->src[0])[0];
         for (int i = 0; i < dest_components; i++) {
            dst[i] = create_driver_param_indirect(ctx, param + i,
                                                  ir3_get_addr0(ctx, view, 4));
         }
         ctx->so->constlen =
            MAX2(ctx->so->constlen,
                 const_state->offsets.driver_param + param / 4 +
                 nir_intrinsic_range(intr));
      }
      break;
   }
   case nir_intrinsic_discard_if:
   case nir_intrinsic_discard:
   case nir_intrinsic_demote:
   case nir_intrinsic_demote_if:
   case nir_intrinsic_terminate:
   case nir_intrinsic_terminate_if: {
      struct ir3_instruction *cond, *kill;

      if (intr->intrinsic == nir_intrinsic_discard_if ||
          intr->intrinsic == nir_intrinsic_demote_if ||
          intr->intrinsic == nir_intrinsic_terminate_if) {
         /* conditional discard: */
         src = ir3_get_src(ctx, &intr->src[0]);
         cond = src[0];
      } else {
         /* unconditional discard: */
         cond = create_immed_typed(b, 1, ctx->compiler->bool_type);
      }

      /* NOTE: only cmps.*.* can write p0.x: */
      struct ir3_instruction *zero =
            create_immed_typed(b, 0, is_half(cond) ? TYPE_U16 : TYPE_U32);
      cond = ir3_CMPS_S(b, cond, 0, zero, 0);
      cond->cat2.condition = IR3_COND_NE;

      /* condition always goes in predicate register: */
      cond->dsts[0]->num = regid(REG_P0, 0);
      cond->dsts[0]->flags &= ~IR3_REG_SSA;

      if (intr->intrinsic == nir_intrinsic_demote ||
          intr->intrinsic == nir_intrinsic_demote_if) {
         kill = ir3_DEMOTE(b, cond, 0);
      } else {
         kill = ir3_KILL(b, cond, 0);
      }

      /* - Side-effects should not be moved on a different side of the kill
       * - Instructions that depend on active fibers should not be reordered
       */
      kill->barrier_class = IR3_BARRIER_IMAGE_W | IR3_BARRIER_BUFFER_W |
                            IR3_BARRIER_ACTIVE_FIBERS_W;
      kill->barrier_conflict = IR3_BARRIER_IMAGE_W | IR3_BARRIER_BUFFER_W |
                               IR3_BARRIER_ACTIVE_FIBERS_R;
      kill->srcs[0]->num = regid(REG_P0, 0);
      array_insert(ctx->ir, ctx->ir->predicates, kill);

      array_insert(b, b->keeps, kill);
      ctx->so->has_kill = true;

      break;
   }

   case nir_intrinsic_cond_end_ir3: {
      struct ir3_instruction *cond, *kill;

      src = ir3_get_src(ctx, &intr->src[0]);
      cond = src[0];

      /* NOTE: only cmps.*.* can write p0.x: */
      struct ir3_instruction *zero =
            create_immed_typed(b, 0, is_half(cond) ? TYPE_U16 : TYPE_U32);
      cond = ir3_CMPS_S(b, cond, 0, zero, 0);
      cond->cat2.condition = IR3_COND_NE;

      /* condition always goes in predicate register: */
      cond->dsts[0]->num = regid(REG_P0, 0);

      kill = ir3_PREDT(b, cond, 0);

      kill->barrier_class = IR3_BARRIER_EVERYTHING;
      kill->barrier_conflict = IR3_BARRIER_EVERYTHING;

      array_insert(ctx->ir, ctx->ir->predicates, kill);
      array_insert(b, b->keeps, kill);
      break;
   }

   case nir_intrinsic_vote_any:
   case nir_intrinsic_vote_all: {
      struct ir3_instruction *src = ir3_get_src(ctx, &intr->src[0])[0];
      struct ir3_instruction *pred = ir3_get_predicate(ctx, src);
      if (intr->intrinsic == nir_intrinsic_vote_any)
         dst[0] = ir3_ANY_MACRO(ctx->block, pred, 0);
      else
         dst[0] = ir3_ALL_MACRO(ctx->block, pred, 0);
      dst[0]->srcs[0]->num = regid(REG_P0, 0);
      array_insert(ctx->ir, ctx->ir->predicates, dst[0]);
      break;
   }
   case nir_intrinsic_elect:
      dst[0] = ir3_ELECT_MACRO(ctx->block);
      /* This may expand to a divergent if/then, so allocate stack space for
       * it.
       */
      ctx->max_stack = MAX2(ctx->max_stack, ctx->stack + 1);
      break;
   case nir_intrinsic_preamble_start_ir3:
      dst[0] = ir3_SHPS_MACRO(ctx->block);
      ctx->max_stack = MAX2(ctx->max_stack, ctx->stack + 1);
      break;

   case nir_intrinsic_read_invocation_cond_ir3: {
      struct ir3_instruction *src = ir3_get_src(ctx, &intr->src[0])[0];
      struct ir3_instruction *cond = ir3_get_src(ctx, &intr->src[1])[0];
      dst[0] = ir3_READ_COND_MACRO(ctx->block, ir3_get_predicate(ctx, cond), 0,
                                   src, 0);
      dst[0]->dsts[0]->flags |= IR3_REG_SHARED;
      dst[0]->srcs[0]->num = regid(REG_P0, 0);
      array_insert(ctx->ir, ctx->ir->predicates, dst[0]);
      ctx->max_stack = MAX2(ctx->max_stack, ctx->stack + 1);
      break;
   }

   case nir_intrinsic_read_first_invocation: {
      struct ir3_instruction *src = ir3_get_src(ctx, &intr->src[0])[0];
      dst[0] = ir3_READ_FIRST_MACRO(ctx->block, src, 0);
      dst[0]->dsts[0]->flags |= IR3_REG_SHARED;
      ctx->max_stack = MAX2(ctx->max_stack, ctx->stack + 1);
      break;
   }

   case nir_intrinsic_ballot: {
      struct ir3_instruction *ballot;
      unsigned components = intr->def.num_components;
      if (nir_src_is_const(intr->src[0]) && nir_src_as_bool(intr->src[0])) {
         /* ballot(true) is just MOVMSK */
         ballot = ir3_MOVMSK(ctx->block, components);
      } else {
         struct ir3_instruction *src = ir3_get_src(ctx, &intr->src[0])[0];
         struct ir3_instruction *pred = ir3_get_predicate(ctx, src);
         ballot = ir3_BALLOT_MACRO(ctx->block, pred, components);
         ballot->srcs[0]->num = regid(REG_P0, 0);
         array_insert(ctx->ir, ctx->ir->predicates, ballot);
         ctx->max_stack = MAX2(ctx->max_stack, ctx->stack + 1);
      }

      ballot->barrier_class = IR3_BARRIER_ACTIVE_FIBERS_R;
      ballot->barrier_conflict = IR3_BARRIER_ACTIVE_FIBERS_W;

      ir3_split_dest(ctx->block, dst, ballot, 0, components);
      break;
   }

   case nir_intrinsic_quad_broadcast: {
      struct ir3_instruction *src = ir3_get_src(ctx, &intr->src[0])[0];
      struct ir3_instruction *idx = ir3_get_src(ctx, &intr->src[1])[0];

      type_t dst_type = type_uint_size(intr->def.bit_size);

      if (dst_type != TYPE_U32)
         idx = ir3_COV(ctx->block, idx, TYPE_U32, dst_type);

      dst[0] = ir3_QUAD_SHUFFLE_BRCST(ctx->block, src, 0, idx, 0);
      dst[0]->cat5.type = dst_type;
      break;
   }

   case nir_intrinsic_quad_swap_horizontal: {
      struct ir3_instruction *src = ir3_get_src(ctx, &intr->src[0])[0];
      dst[0] = ir3_QUAD_SHUFFLE_HORIZ(ctx->block, src, 0);
      dst[0]->cat5.type = type_uint_size(intr->def.bit_size);
      break;
   }

   case nir_intrinsic_quad_swap_vertical: {
      struct ir3_instruction *src = ir3_get_src(ctx, &intr->src[0])[0];
      dst[0] = ir3_QUAD_SHUFFLE_VERT(ctx->block, src, 0);
      dst[0]->cat5.type = type_uint_size(intr->def.bit_size);
      break;
   }

   case nir_intrinsic_quad_swap_diagonal: {
      struct ir3_instruction *src = ir3_get_src(ctx, &intr->src[0])[0];
      dst[0] = ir3_QUAD_SHUFFLE_DIAG(ctx->block, src, 0);
      dst[0]->cat5.type = type_uint_size(intr->def.bit_size);
      break;
   }

   case nir_intrinsic_load_shared_ir3:
      emit_intrinsic_load_shared_ir3(ctx, intr, dst);
      break;
   case nir_intrinsic_store_shared_ir3:
      emit_intrinsic_store_shared_ir3(ctx, intr);
      break;
   case nir_intrinsic_bindless_resource_ir3:
      dst[0] = ir3_get_src(ctx, &intr->src[0])[0];
      break;
   case nir_intrinsic_global_atomic_ir3:
   case nir_intrinsic_global_atomic_swap_ir3: {
      dst[0] = ctx->funcs->emit_intrinsic_atomic_global(ctx, intr);
      break;
   }

   case nir_intrinsic_reduce:
   case nir_intrinsic_inclusive_scan:
   case nir_intrinsic_exclusive_scan:
      dst[0] = emit_intrinsic_reduce(ctx, intr);
      break;

   case nir_intrinsic_preamble_end_ir3: {
      struct ir3_instruction *instr = ir3_SHPE(ctx->block);
      instr->barrier_class = instr->barrier_conflict = IR3_BARRIER_CONST_W;
      array_insert(b, b->keeps, instr);
      break;
   }
   case nir_intrinsic_store_uniform_ir3: {
      unsigned components = nir_src_num_components(intr->src[0]);
      unsigned dst = nir_intrinsic_base(intr);
      unsigned dst_lo = dst & 0xff;
      unsigned dst_hi = dst >> 8;

      struct ir3_instruction *src =
         ir3_create_collect(b, ir3_get_src(ctx, &intr->src[0]), components);
      struct ir3_instruction *a1 = NULL;
      if (dst_hi) {
         /* Encode only the high part of the destination in a1.x to increase the
          * chance that we can reuse the a1.x value in subsequent stc
          * instructions.
          */
         a1 = ir3_get_addr1(ctx, dst_hi << 8);
      }

      struct ir3_instruction *stc =
         ir3_STC(ctx->block, create_immed(b, dst_lo),  0, src, 0);
      stc->cat6.iim_val = components;
      stc->cat6.type = TYPE_U32;
      stc->barrier_conflict = IR3_BARRIER_CONST_W;
      if (a1) {
         ir3_instr_set_address(stc, a1);
         stc->flags |= IR3_INSTR_A1EN;
      }
      array_insert(b, b->keeps, stc);
      break;
   }
   case nir_intrinsic_copy_push_const_to_uniform_ir3: {
      struct ir3_instruction *load =
         ir3_instr_create(ctx->block, OPC_PUSH_CONSTS_LOAD_MACRO, 0, 0);
      array_insert(b, b->keeps, load);

      load->push_consts.dst_base = nir_src_as_uint(intr->src[0]);
      load->push_consts.src_base = nir_intrinsic_base(intr);
      load->push_consts.src_size = nir_intrinsic_range(intr);

      ctx->so->constlen =
         MAX2(ctx->so->constlen,
              DIV_ROUND_UP(
                 load->push_consts.dst_base + load->push_consts.src_size, 4));
      break;
   }
   default:
      ir3_context_error(ctx, "Unhandled intrinsic type: %s\n",
                        nir_intrinsic_infos[intr->intrinsic].name);
      break;
   }

   if (info->has_dest)
      ir3_put_def(ctx, &intr->def);
}

static void
emit_load_const(struct ir3_context *ctx, nir_load_const_instr *instr)
{
   struct ir3_instruction **dst =
      ir3_get_dst_ssa(ctx, &instr->def, instr->def.num_components);
   unsigned bit_size = ir3_bitsize(ctx, instr->def.bit_size);

   if (bit_size <= 8) {
      for (int i = 0; i < instr->def.num_components; i++)
         dst[i] = create_immed_typed(ctx->block, instr->value[i].u8, TYPE_U8);
   } else if (bit_size <= 16) {
      for (int i = 0; i < instr->def.num_components; i++)
         dst[i] = create_immed_typed(ctx->block, instr->value[i].u16, TYPE_U16);
   } else {
      for (int i = 0; i < instr->def.num_components; i++)
         dst[i] = create_immed_typed(ctx->block, instr->value[i].u32, TYPE_U32);
   }
}

static void
emit_undef(struct ir3_context *ctx, nir_undef_instr *undef)
{
   struct ir3_instruction **dst =
      ir3_get_dst_ssa(ctx, &undef->def, undef->def.num_components);
   type_t type = utype_for_size(ir3_bitsize(ctx, undef->def.bit_size));

   /* backend doesn't want undefined instructions, so just plug
    * in 0.0..
    */
   for (int i = 0; i < undef->def.num_components; i++)
      dst[i] = create_immed_typed(ctx->block, fui(0.0), type);
}

/*
 * texture fetch/sample instructions:
 */

static type_t
get_tex_dest_type(nir_tex_instr *tex)
{
   type_t type;

   switch (tex->dest_type) {
   case nir_type_float32:
      return TYPE_F32;
   case nir_type_float16:
      return TYPE_F16;
   case nir_type_int32:
      return TYPE_S32;
   case nir_type_int16:
      return TYPE_S16;
   case nir_type_bool32:
   case nir_type_uint32:
      return TYPE_U32;
   case nir_type_bool16:
   case nir_type_uint16:
      return TYPE_U16;
   case nir_type_invalid:
   default:
      unreachable("bad dest_type");
   }

   return type;
}

static void
tex_info(nir_tex_instr *tex, unsigned *flagsp, unsigned *coordsp)
{
   unsigned coords =
      glsl_get_sampler_dim_coordinate_components(tex->sampler_dim);
   unsigned flags = 0;

   /* note: would use tex->coord_components.. except txs.. also,
    * since array index goes after shadow ref, we don't want to
    * count it:
    */
   if (coords == 3)
      flags |= IR3_INSTR_3D;

   if (tex->is_shadow && tex->op != nir_texop_lod)
      flags |= IR3_INSTR_S;

   if (tex->is_array && tex->op != nir_texop_lod)
      flags |= IR3_INSTR_A;

   *flagsp = flags;
   *coordsp = coords;
}

/* Gets the sampler/texture idx as a hvec2.  Which could either be dynamic
 * or immediate (in which case it will get lowered later to a non .s2en
 * version of the tex instruction which encode tex/samp as immediates:
 */
static struct tex_src_info
get_tex_samp_tex_src(struct ir3_context *ctx, nir_tex_instr *tex)
{
   struct ir3_block *b = ctx->block;
   struct tex_src_info info = {0};
   int texture_idx = nir_tex_instr_src_index(tex, nir_tex_src_texture_handle);
   int sampler_idx = nir_tex_instr_src_index(tex, nir_tex_src_sampler_handle);
   struct ir3_instruction *texture, *sampler;

   if (texture_idx >= 0 || sampler_idx >= 0) {
      /* Bindless case */
      info.flags |= IR3_INSTR_B;

      if (tex->texture_non_uniform || tex->sampler_non_uniform)
         info.flags |= IR3_INSTR_NONUNIF;

      /* Gather information required to determine which encoding to
       * choose as well as for prefetch.
       */
      nir_intrinsic_instr *bindless_tex = NULL;
      bool tex_const;
      if (texture_idx >= 0) {
         ctx->so->bindless_tex = true;
         bindless_tex = ir3_bindless_resource(tex->src[texture_idx].src);
         assert(bindless_tex);
         info.tex_base = nir_intrinsic_desc_set(bindless_tex);
         tex_const = nir_src_is_const(bindless_tex->src[0]);
         if (tex_const)
            info.tex_idx = nir_src_as_uint(bindless_tex->src[0]);
      } else {
         /* To simplify some of the logic below, assume the index is
          * constant 0 when it's not enabled.
          */
         tex_const = true;
         info.tex_idx = 0;
      }
      nir_intrinsic_instr *bindless_samp = NULL;
      bool samp_const;
      if (sampler_idx >= 0) {
         ctx->so->bindless_samp = true;
         bindless_samp = ir3_bindless_resource(tex->src[sampler_idx].src);
         assert(bindless_samp);
         info.samp_base = nir_intrinsic_desc_set(bindless_samp);
         samp_const = nir_src_is_const(bindless_samp->src[0]);
         if (samp_const)
            info.samp_idx = nir_src_as_uint(bindless_samp->src[0]);
      } else {
         samp_const = true;
         info.samp_idx = 0;
      }

      /* Choose encoding. */
      if (tex_const && samp_const && info.tex_idx < 256 &&
          info.samp_idx < 256) {
         if (info.tex_idx < 16 && info.samp_idx < 16 &&
             (!bindless_tex || !bindless_samp ||
              info.tex_base == info.samp_base)) {
            /* Everything fits within the instruction */
            info.base = info.tex_base;
         } else {
            info.base = info.tex_base;
            if (ctx->compiler->gen <= 6) {
               info.a1_val = info.tex_idx << 3 | info.samp_base;
            } else {
               info.a1_val = info.samp_idx << 3 | info.samp_base;
            }

            info.flags |= IR3_INSTR_A1EN;
         }
         info.samp_tex = NULL;
      } else {
         info.flags |= IR3_INSTR_S2EN;
         /* In the indirect case, we only use a1.x to store the sampler
          * base if it differs from the texture base.
          */
         if (!bindless_tex || !bindless_samp ||
             info.tex_base == info.samp_base) {
            info.base = info.tex_base;
         } else {
            info.base = info.tex_base;
            info.a1_val = info.samp_base;
            info.flags |= IR3_INSTR_A1EN;
         }

         /* Note: the indirect source is now a vec2 instead of hvec2, and
          * for some reason the texture and sampler are swapped.
          */
         struct ir3_instruction *texture, *sampler;

         if (bindless_tex) {
            texture = ir3_get_src(ctx, &tex->src[texture_idx].src)[0];
         } else {
            texture = create_immed(b, 0);
         }

         if (bindless_samp) {
            sampler = ir3_get_src(ctx, &tex->src[sampler_idx].src)[0];
         } else {
            sampler = create_immed(b, 0);
         }
         info.samp_tex = ir3_collect(b, texture, sampler);
      }
   } else {
      info.flags |= IR3_INSTR_S2EN;
      texture_idx = nir_tex_instr_src_index(tex, nir_tex_src_texture_offset);
      sampler_idx = nir_tex_instr_src_index(tex, nir_tex_src_sampler_offset);
      if (texture_idx >= 0) {
         texture = ir3_get_src(ctx, &tex->src[texture_idx].src)[0];
         texture = ir3_COV(ctx->block, texture, TYPE_U32, TYPE_U16);
      } else {
         /* TODO what to do for dynamic case? I guess we only need the
          * max index for astc srgb workaround so maybe not a problem
          * to worry about if we don't enable indirect samplers for
          * a4xx?
          */
         ctx->max_texture_index =
            MAX2(ctx->max_texture_index, tex->texture_index);
         texture = create_immed_typed(ctx->block, tex->texture_index, TYPE_U16);
         info.tex_idx = tex->texture_index;
      }

      if (sampler_idx >= 0) {
         sampler = ir3_get_src(ctx, &tex->src[sampler_idx].src)[0];
         sampler = ir3_COV(ctx->block, sampler, TYPE_U32, TYPE_U16);
      } else {
         sampler = create_immed_typed(ctx->block, tex->sampler_index, TYPE_U16);
         info.samp_idx = tex->texture_index;
      }

      info.samp_tex = ir3_collect(b, sampler, texture);
   }

   return info;
}

static void
emit_tex(struct ir3_context *ctx, nir_tex_instr *tex)
{
   struct ir3_block *b = ctx->block;
   struct ir3_instruction **dst, *sam, *src0[12], *src1[4];
   struct ir3_instruction *const *coord, *const *off, *const *ddx, *const *ddy;
   struct ir3_instruction *lod, *compare, *proj, *sample_index;
   struct tex_src_info info = {0};
   bool has_bias = false, has_lod = false, has_proj = false, has_off = false;
   unsigned i, coords, flags, ncomp;
   unsigned nsrc0 = 0, nsrc1 = 0;
   type_t type;
   opc_t opc = 0;

   ncomp = tex->def.num_components;

   coord = off = ddx = ddy = NULL;
   lod = proj = compare = sample_index = NULL;

   dst = ir3_get_def(ctx, &tex->def, ncomp);

   for (unsigned i = 0; i < tex->num_srcs; i++) {
      switch (tex->src[i].src_type) {
      case nir_tex_src_coord:
         coord = ir3_get_src(ctx, &tex->src[i].src);
         break;
      case nir_tex_src_bias:
         lod = ir3_get_src(ctx, &tex->src[i].src)[0];
         has_bias = true;
         break;
      case nir_tex_src_lod:
         lod = ir3_get_src(ctx, &tex->src[i].src)[0];
         has_lod = true;
         break;
      case nir_tex_src_comparator: /* shadow comparator */
         compare = ir3_get_src(ctx, &tex->src[i].src)[0];
         break;
      case nir_tex_src_projector:
         proj = ir3_get_src(ctx, &tex->src[i].src)[0];
         has_proj = true;
         break;
      case nir_tex_src_offset:
         off = ir3_get_src(ctx, &tex->src[i].src);
         has_off = true;
         break;
      case nir_tex_src_ddx:
         ddx = ir3_get_src(ctx, &tex->src[i].src);
         break;
      case nir_tex_src_ddy:
         ddy = ir3_get_src(ctx, &tex->src[i].src);
         break;
      case nir_tex_src_ms_index:
         sample_index = ir3_get_src(ctx, &tex->src[i].src)[0];
         break;
      case nir_tex_src_texture_offset:
      case nir_tex_src_sampler_offset:
      case nir_tex_src_texture_handle:
      case nir_tex_src_sampler_handle:
         /* handled in get_tex_samp_src() */
         break;
      default:
         ir3_context_error(ctx, "Unhandled NIR tex src type: %d\n",
                           tex->src[i].src_type);
         return;
      }
   }

   switch (tex->op) {
   case nir_texop_tex_prefetch:
      compile_assert(ctx, !has_bias);
      compile_assert(ctx, !has_lod);
      compile_assert(ctx, !compare);
      compile_assert(ctx, !has_proj);
      compile_assert(ctx, !has_off);
      compile_assert(ctx, !ddx);
      compile_assert(ctx, !ddy);
      compile_assert(ctx, !sample_index);
      compile_assert(
         ctx, nir_tex_instr_src_index(tex, nir_tex_src_texture_offset) < 0);
      compile_assert(
         ctx, nir_tex_instr_src_index(tex, nir_tex_src_sampler_offset) < 0);

      if (ctx->so->num_sampler_prefetch < ctx->prefetch_limit) {
         opc = OPC_META_TEX_PREFETCH;
         ctx->so->num_sampler_prefetch++;
         break;
      }
      FALLTHROUGH;
   case nir_texop_tex:
      opc = has_lod ? OPC_SAML : OPC_SAM;
      break;
   case nir_texop_txb:
      opc = OPC_SAMB;
      break;
   case nir_texop_txl:
      opc = OPC_SAML;
      break;
   case nir_texop_txd:
      opc = OPC_SAMGQ;
      break;
   case nir_texop_txf:
      opc = OPC_ISAML;
      break;
   case nir_texop_lod:
      opc = OPC_GETLOD;
      break;
   case nir_texop_tg4:
      switch (tex->component) {
      case 0:
         opc = OPC_GATHER4R;
         break;
      case 1:
         opc = OPC_GATHER4G;
         break;
      case 2:
         opc = OPC_GATHER4B;
         break;
      case 3:
         opc = OPC_GATHER4A;
         break;
      }
      break;
   case nir_texop_txf_ms_fb:
   case nir_texop_txf_ms:
      opc = OPC_ISAMM;
      break;
   default:
      ir3_context_error(ctx, "Unhandled NIR tex type: %d\n", tex->op);
      return;
   }

   tex_info(tex, &flags, &coords);

   /*
    * lay out the first argument in the proper order:
    *  - actual coordinates first
    *  - shadow reference
    *  - array index
    *  - projection w
    *  - starting at offset 4, dpdx.xy, dpdy.xy
    *
    * bias/lod go into the second arg
    */

   /* insert tex coords: */
   for (i = 0; i < coords; i++)
      src0[i] = coord[i];

   nsrc0 = i;

   type_t coord_pad_type = is_half(coord[0]) ? TYPE_U16 : TYPE_U32;
   /* scale up integer coords for TXF based on the LOD */
   if (ctx->compiler->unminify_coords && (opc == OPC_ISAML)) {
      assert(has_lod);
      for (i = 0; i < coords; i++)
         src0[i] = ir3_SHL_B(b, src0[i], 0, lod, 0);
   }

   if (coords == 1) {
      /* hw doesn't do 1d, so we treat it as 2d with
       * height of 1, and patch up the y coord.
       */
      if (is_isam(opc)) {
         src0[nsrc0++] = create_immed_typed(b, 0, coord_pad_type);
      } else if (is_half(coord[0])) {
         src0[nsrc0++] = create_immed_typed(b, _mesa_float_to_half(0.5), coord_pad_type);
      } else {
         src0[nsrc0++] = create_immed_typed(b, fui(0.5), coord_pad_type);
      }
   }

   if (tex->is_shadow && tex->op != nir_texop_lod)
      src0[nsrc0++] = compare;

   if (tex->is_array && tex->op != nir_texop_lod)
      src0[nsrc0++] = coord[coords];

   if (has_proj) {
      src0[nsrc0++] = proj;
      flags |= IR3_INSTR_P;
   }

   /* pad to 4, then ddx/ddy: */
   if (tex->op == nir_texop_txd) {
      while (nsrc0 < 4)
         src0[nsrc0++] = create_immed_typed(b, fui(0.0), coord_pad_type);
      for (i = 0; i < coords; i++)
         src0[nsrc0++] = ddx[i];
      if (coords < 2)
         src0[nsrc0++] = create_immed_typed(b, fui(0.0), coord_pad_type);
      for (i = 0; i < coords; i++)
         src0[nsrc0++] = ddy[i];
      if (coords < 2)
         src0[nsrc0++] = create_immed_typed(b, fui(0.0), coord_pad_type);
   }

   /* NOTE a3xx (and possibly a4xx?) might be different, using isaml
    * with scaled x coord according to requested sample:
    */
   if (opc == OPC_ISAMM) {
      if (ctx->compiler->txf_ms_with_isaml) {
         /* the samples are laid out in x dimension as
          *     0 1 2 3
          * x_ms = (x << ms) + sample_index;
          */
         struct ir3_instruction *ms;
         ms = create_immed(b, (ctx->samples >> (2 * tex->texture_index)) & 3);

         src0[0] = ir3_SHL_B(b, src0[0], 0, ms, 0);
         src0[0] = ir3_ADD_U(b, src0[0], 0, sample_index, 0);

         opc = OPC_ISAML;
      } else {
         src0[nsrc0++] = sample_index;
      }
   }

   /*
    * second argument (if applicable):
    *  - offsets
    *  - lod
    *  - bias
    */
   if (has_off | has_lod | has_bias) {
      if (has_off) {
         unsigned off_coords = coords;
         if (tex->sampler_dim == GLSL_SAMPLER_DIM_CUBE)
            off_coords--;
         for (i = 0; i < off_coords; i++)
            src1[nsrc1++] = off[i];
         if (off_coords < 2)
            src1[nsrc1++] = create_immed_typed(b, fui(0.0), coord_pad_type);
         flags |= IR3_INSTR_O;
      }

      if (has_lod | has_bias)
         src1[nsrc1++] = lod;
   }

   type = get_tex_dest_type(tex);

   if (opc == OPC_GETLOD)
      type = TYPE_S32;

   if (tex->op == nir_texop_txf_ms_fb) {
      compile_assert(ctx, ctx->so->type == MESA_SHADER_FRAGMENT);

      ctx->so->fb_read = true;
      if (ctx->compiler->options.bindless_fb_read_descriptor >= 0) {
         ctx->so->bindless_tex = true;
         info.flags = IR3_INSTR_B;
         info.base = ctx->compiler->options.bindless_fb_read_descriptor;
         struct ir3_instruction *texture, *sampler;

         int base_index =
            nir_tex_instr_src_index(tex, nir_tex_src_texture_handle);
         nir_src tex_src = tex->src[base_index].src;

         if (nir_src_is_const(tex_src)) {
            texture = create_immed_typed(b,
               nir_src_as_uint(tex_src) + ctx->compiler->options.bindless_fb_read_slot,
               TYPE_U32);
         } else {
            texture = create_immed_typed(
               ctx->block, ctx->compiler->options.bindless_fb_read_slot, TYPE_U32);
            struct ir3_instruction *base =
               ir3_get_src(ctx, &tex->src[base_index].src)[0];
            texture = ir3_ADD_U(b, texture, 0, base, 0);
         }
         sampler = create_immed_typed(ctx->block, 0, TYPE_U32);
         info.samp_tex = ir3_collect(b, texture, sampler);
         info.flags |= IR3_INSTR_S2EN;
         if (tex->texture_non_uniform) {
            info.flags |= IR3_INSTR_NONUNIF;
         }
      } else {
         /* Otherwise append a sampler to be patched into the texture
          * state:
          */
         info.samp_tex = ir3_collect(
               b, create_immed_typed(ctx->block, ctx->so->num_samp, TYPE_U16),
               create_immed_typed(ctx->block, ctx->so->num_samp, TYPE_U16));
         info.flags = IR3_INSTR_S2EN;
      }

      ctx->so->num_samp++;
   } else {
      info = get_tex_samp_tex_src(ctx, tex);
   }

   bool tg4_swizzle_fixup = false;
   if (tex->op == nir_texop_tg4 && ctx->compiler->gen == 4 &&
         ctx->sampler_swizzles[tex->texture_index] != 0x688 /* rgba */) {
      uint16_t swizzles = ctx->sampler_swizzles[tex->texture_index];
      uint16_t swizzle = (swizzles >> (tex->component * 3)) & 7;
      if (swizzle > 3) {
         /* this would mean that we can just return 0 / 1, no texturing
          * necessary
          */
         struct ir3_instruction *imm = create_immed(b,
               type_float(type) ? fui(swizzle - 4) : (swizzle - 4));
         for (int i = 0; i < 4; i++)
            dst[i] = imm;
         ir3_put_def(ctx, &tex->def);
         return;
      }
      opc = OPC_GATHER4R + swizzle;
      tg4_swizzle_fixup = true;
   }

   struct ir3_instruction *col0 = ir3_create_collect(b, src0, nsrc0);
   struct ir3_instruction *col1 = ir3_create_collect(b, src1, nsrc1);

   if (opc == OPC_META_TEX_PREFETCH) {
      int idx = nir_tex_instr_src_index(tex, nir_tex_src_coord);


      sam = ir3_SAM(ctx->in_block, opc, type, MASK(ncomp), 0, NULL,
                    get_barycentric(ctx, IJ_PERSP_PIXEL), 0);
      sam->prefetch.input_offset = ir3_nir_coord_offset(tex->src[idx].src.ssa);
      /* make sure not to add irrelevant flags like S2EN */
      sam->flags = flags | (info.flags & IR3_INSTR_B);
      sam->prefetch.tex = info.tex_idx;
      sam->prefetch.samp = info.samp_idx;
      sam->prefetch.tex_base = info.tex_base;
      sam->prefetch.samp_base = info.samp_base;
   } else {
      info.flags |= flags;
      sam = emit_sam(ctx, opc, info, type, MASK(ncomp), col0, col1);
   }

   if (tg4_swizzle_fixup) {
      /* TODO: fix-up for ASTC when alpha is selected? */
      array_insert(ctx->ir, ctx->ir->tg4, sam);

      ir3_split_dest(b, dst, sam, 0, 4);

      uint8_t tex_bits = ctx->sampler_swizzles[tex->texture_index] >> 12;
      if (!type_float(type) && tex_bits != 3 /* 32bpp */ &&
            tex_bits != 0 /* key unset */) {
         uint8_t bits = 0;
         switch (tex_bits) {
         case 1: /* 8bpp */
            bits = 8;
            break;
         case 2: /* 16bpp */
            bits = 16;
            break;
         case 4: /* 10bpp or 2bpp for alpha */
            if (opc == OPC_GATHER4A)
               bits = 2;
            else
               bits = 10;
            break;
         default:
            assert(0);
         }

         sam->cat5.type = TYPE_F32;
         for (int i = 0; i < 4; i++) {
            /* scale and offset the unorm data */
            dst[i] = ir3_MAD_F32(b, dst[i], 0, create_immed(b, fui((1 << bits) - 1)), 0, create_immed(b, fui(0.5f)), 0);
            /* convert the scaled value to integer */
            dst[i] = ir3_COV(b, dst[i], TYPE_F32, TYPE_U32);
            /* sign extend for signed values */
            if (type == TYPE_S32) {
               dst[i] = ir3_SHL_B(b, dst[i], 0, create_immed(b, 32 - bits), 0);
               dst[i] = ir3_ASHR_B(b, dst[i], 0, create_immed(b, 32 - bits), 0);
            }
         }
      }
   } else if ((ctx->astc_srgb & (1 << tex->texture_index)) &&
       tex->op != nir_texop_tg4 && /* leave out tg4, unless it's on alpha? */
       !nir_tex_instr_is_query(tex)) {
      assert(opc != OPC_META_TEX_PREFETCH);

      /* only need first 3 components: */
      sam->dsts[0]->wrmask = 0x7;
      ir3_split_dest(b, dst, sam, 0, 3);

      /* we need to sample the alpha separately with a non-SRGB
       * texture state:
       */
      sam = ir3_SAM(b, opc, type, 0b1000, flags | info.flags, info.samp_tex,
                    col0, col1);

      array_insert(ctx->ir, ctx->ir->astc_srgb, sam);

      /* fixup .w component: */
      ir3_split_dest(b, &dst[3], sam, 3, 1);
   } else {
      /* normal (non-workaround) case: */
      ir3_split_dest(b, dst, sam, 0, ncomp);
   }

   /* GETLOD returns results in 4.8 fixed point */
   if (opc == OPC_GETLOD) {
      bool half = tex->def.bit_size == 16;
      struct ir3_instruction *factor =
         half ? create_immed_typed(b, _mesa_float_to_half(1.0 / 256), TYPE_F16)
              : create_immed(b, fui(1.0 / 256));

      for (i = 0; i < 2; i++) {
         dst[i] = ir3_MUL_F(
            b, ir3_COV(b, dst[i], TYPE_S32, half ? TYPE_F16 : TYPE_F32), 0,
            factor, 0);
      }
   }

   ir3_put_def(ctx, &tex->def);
}

static void
emit_tex_info(struct ir3_context *ctx, nir_tex_instr *tex, unsigned idx)
{
   struct ir3_block *b = ctx->block;
   struct ir3_instruction **dst, *sam;
   type_t dst_type = get_tex_dest_type(tex);
   struct tex_src_info info = get_tex_samp_tex_src(ctx, tex);

   dst = ir3_get_def(ctx, &tex->def, 1);

   sam = emit_sam(ctx, OPC_GETINFO, info, dst_type, 1 << idx, NULL, NULL);

   /* even though there is only one component, since it ends
    * up in .y/.z/.w rather than .x, we need a split_dest()
    */
   ir3_split_dest(b, dst, sam, idx, 1);

   /* The # of levels comes from getinfo.z. We need to add 1 to it, since
    * the value in TEX_CONST_0 is zero-based.
    */
   if (ctx->compiler->levels_add_one)
      dst[0] = ir3_ADD_U(b, dst[0], 0, create_immed(b, 1), 0);

   ir3_put_def(ctx, &tex->def);
}

static void
emit_tex_txs(struct ir3_context *ctx, nir_tex_instr *tex)
{
   struct ir3_block *b = ctx->block;
   struct ir3_instruction **dst, *sam;
   struct ir3_instruction *lod;
   unsigned flags, coords;
   type_t dst_type = get_tex_dest_type(tex);
   struct tex_src_info info = get_tex_samp_tex_src(ctx, tex);

   tex_info(tex, &flags, &coords);
   info.flags |= flags;

   /* Actually we want the number of dimensions, not coordinates. This
    * distinction only matters for cubes.
    */
   if (tex->sampler_dim == GLSL_SAMPLER_DIM_CUBE)
      coords = 2;

   dst = ir3_get_def(ctx, &tex->def, 4);

   int lod_idx = nir_tex_instr_src_index(tex, nir_tex_src_lod);
   compile_assert(ctx, lod_idx >= 0);

   lod = ir3_get_src(ctx, &tex->src[lod_idx].src)[0];

   if (tex->sampler_dim != GLSL_SAMPLER_DIM_BUF) {
      sam = emit_sam(ctx, OPC_GETSIZE, info, dst_type, 0b1111, lod, NULL);
   } else {
      /*
       * The maximum value which OPC_GETSIZE could return for one dimension
       * is 0x007ff0, however sampler buffer could be much bigger.
       * Blob uses OPC_GETBUF for them.
       */
      sam = emit_sam(ctx, OPC_GETBUF, info, dst_type, 0b1111, NULL, NULL);
   }

   ir3_split_dest(b, dst, sam, 0, 4);

   /* Array size actually ends up in .w rather than .z. This doesn't
    * matter for miplevel 0, but for higher mips the value in z is
    * minified whereas w stays. Also, the value in TEX_CONST_3_DEPTH is
    * returned, which means that we have to add 1 to it for arrays.
    */
   if (tex->is_array) {
      if (ctx->compiler->levels_add_one) {
         dst[coords] = ir3_ADD_U(b, dst[3], 0, create_immed(b, 1), 0);
      } else {
         dst[coords] = ir3_MOV(b, dst[3], TYPE_U32);
      }
   }

   ir3_put_def(ctx, &tex->def);
}

/* phi instructions are left partially constructed.  We don't resolve
 * their srcs until the end of the shader, since (eg. loops) one of
 * the phi's srcs might be defined after the phi due to back edges in
 * the CFG.
 */
static void
emit_phi(struct ir3_context *ctx, nir_phi_instr *nphi)
{
   struct ir3_instruction *phi, **dst;

   /* NOTE: phi's should be lowered to scalar at this point */
   compile_assert(ctx, nphi->def.num_components == 1);

   dst = ir3_get_def(ctx, &nphi->def, 1);

   phi = ir3_instr_create(ctx->block, OPC_META_PHI, 1,
                          exec_list_length(&nphi->srcs));
   __ssa_dst(phi);
   phi->phi.nphi = nphi;

   dst[0] = phi;

   ir3_put_def(ctx, &nphi->def);
}

static struct ir3_block *get_block(struct ir3_context *ctx,
                                   const nir_block *nblock);

static struct ir3_instruction *
read_phi_src(struct ir3_context *ctx, struct ir3_block *blk,
             struct ir3_instruction *phi, nir_phi_instr *nphi)
{
   if (!blk->nblock) {
      struct ir3_instruction *continue_phi =
         ir3_instr_create(blk, OPC_META_PHI, 1, blk->predecessors_count);
      __ssa_dst(continue_phi)->flags = phi->dsts[0]->flags;

      for (unsigned i = 0; i < blk->predecessors_count; i++) {
         struct ir3_instruction *src =
            read_phi_src(ctx, blk->predecessors[i], phi, nphi);
         if (src)
            __ssa_src(continue_phi, src, 0);
         else
            ir3_src_create(continue_phi, INVALID_REG, phi->dsts[0]->flags);
      }

      return continue_phi;
   }

   nir_foreach_phi_src (nsrc, nphi) {
      if (blk->nblock == nsrc->pred) {
         if (nsrc->src.ssa->parent_instr->type == nir_instr_type_undef) {
            /* Create an ir3 undef */
            return NULL;
         } else {
            return ir3_get_src(ctx, &nsrc->src)[0];
         }
      }
   }

   unreachable("couldn't find phi node ir3 block");
   return NULL;
}

static void
resolve_phis(struct ir3_context *ctx, struct ir3_block *block)
{
   foreach_instr (phi, &block->instr_list) {
      if (phi->opc != OPC_META_PHI)
         break;

      nir_phi_instr *nphi = phi->phi.nphi;

      if (!nphi) /* skip continue phis created above */
         continue;

      for (unsigned i = 0; i < block->predecessors_count; i++) {
         struct ir3_block *pred = block->predecessors[i];
         struct ir3_instruction *src = read_phi_src(ctx, pred, phi, nphi);
         if (src) {
            __ssa_src(phi, src, 0);
         } else {
            /* Create an ir3 undef */
            ir3_src_create(phi, INVALID_REG, phi->dsts[0]->flags);
         }
      }
   }
}

static void
emit_jump(struct ir3_context *ctx, nir_jump_instr *jump)
{
   switch (jump->type) {
   case nir_jump_break:
   case nir_jump_continue:
   case nir_jump_return:
      /* I *think* we can simply just ignore this, and use the
       * successor block link to figure out where we need to
       * jump to for break/continue
       */
      break;
   default:
      ir3_context_error(ctx, "Unhandled NIR jump type: %d\n", jump->type);
      break;
   }
}

static void
emit_instr(struct ir3_context *ctx, nir_instr *instr)
{
   switch (instr->type) {
   case nir_instr_type_alu:
      emit_alu(ctx, nir_instr_as_alu(instr));
      break;
   case nir_instr_type_deref:
      /* ignored, handled as part of the intrinsic they are src to */
      break;
   case nir_instr_type_intrinsic:
      emit_intrinsic(ctx, nir_instr_as_intrinsic(instr));
      break;
   case nir_instr_type_load_const:
      emit_load_const(ctx, nir_instr_as_load_const(instr));
      break;
   case nir_instr_type_undef:
      emit_undef(ctx, nir_instr_as_undef(instr));
      break;
   case nir_instr_type_tex: {
      nir_tex_instr *tex = nir_instr_as_tex(instr);
      /* couple tex instructions get special-cased:
       */
      switch (tex->op) {
      case nir_texop_txs:
         emit_tex_txs(ctx, tex);
         break;
      case nir_texop_query_levels:
         emit_tex_info(ctx, tex, 2);
         break;
      case nir_texop_texture_samples:
         emit_tex_info(ctx, tex, 3);
         break;
      default:
         emit_tex(ctx, tex);
         break;
      }
      break;
   }
   case nir_instr_type_jump:
      emit_jump(ctx, nir_instr_as_jump(instr));
      break;
   case nir_instr_type_phi:
      emit_phi(ctx, nir_instr_as_phi(instr));
      break;
   case nir_instr_type_call:
   case nir_instr_type_parallel_copy:
      ir3_context_error(ctx, "Unhandled NIR instruction type: %d\n",
                        instr->type);
      break;
   }
}

static struct ir3_block *
get_block(struct ir3_context *ctx, const nir_block *nblock)
{
   struct ir3_block *block;
   struct hash_entry *hentry;

   hentry = _mesa_hash_table_search(ctx->block_ht, nblock);
   if (hentry)
      return hentry->data;

   block = ir3_block_create(ctx->ir);
   block->nblock = nblock;
   _mesa_hash_table_insert(ctx->block_ht, nblock, block);

   return block;
}

static struct ir3_block *
get_block_or_continue(struct ir3_context *ctx, const nir_block *nblock)
{
   struct hash_entry *hentry;

   hentry = _mesa_hash_table_search(ctx->continue_block_ht, nblock);
   if (hentry)
      return hentry->data;

   return get_block(ctx, nblock);
}

static struct ir3_block *
create_continue_block(struct ir3_context *ctx, const nir_block *nblock)
{
   struct ir3_block *block = ir3_block_create(ctx->ir);
   block->nblock = NULL;
   _mesa_hash_table_insert(ctx->continue_block_ht, nblock, block);
   return block;
}

static void
emit_block(struct ir3_context *ctx, nir_block *nblock)
{
   ctx->block = get_block(ctx, nblock);

   list_addtail(&ctx->block->node, &ctx->ir->block_list);

   ctx->block->loop_id = ctx->loop_id;
   ctx->block->loop_depth = ctx->loop_depth;

   /* re-emit addr register in each block if needed: */
   for (int i = 0; i < ARRAY_SIZE(ctx->addr0_ht); i++) {
      _mesa_hash_table_destroy(ctx->addr0_ht[i], NULL);
      ctx->addr0_ht[i] = NULL;
   }

   _mesa_hash_table_u64_destroy(ctx->addr1_ht);
   ctx->addr1_ht = NULL;

   nir_foreach_instr (instr, nblock) {
      ctx->cur_instr = instr;
      emit_instr(ctx, instr);
      ctx->cur_instr = NULL;
      if (ctx->error)
         return;
   }

   for (int i = 0; i < ARRAY_SIZE(ctx->block->successors); i++) {
      if (nblock->successors[i]) {
         ctx->block->successors[i] =
            get_block_or_continue(ctx, nblock->successors[i]);
         ctx->block->physical_successors[i] = ctx->block->successors[i];
      }
   }

   _mesa_hash_table_clear(ctx->sel_cond_conversions, NULL);
}

static void emit_cf_list(struct ir3_context *ctx, struct exec_list *list);

static void
emit_if(struct ir3_context *ctx, nir_if *nif)
{
   struct ir3_instruction *condition = ir3_get_src(ctx, &nif->condition)[0];

   if (condition->opc == OPC_ANY_MACRO && condition->block == ctx->block) {
      ctx->block->condition = ssa(condition->srcs[0]);
      ctx->block->brtype = IR3_BRANCH_ANY;
   } else if (condition->opc == OPC_ALL_MACRO &&
              condition->block == ctx->block) {
      ctx->block->condition = ssa(condition->srcs[0]);
      ctx->block->brtype = IR3_BRANCH_ALL;
   } else if (condition->opc == OPC_ELECT_MACRO &&
              condition->block == ctx->block) {
      ctx->block->condition = NULL;
      ctx->block->brtype = IR3_BRANCH_GETONE;
   } else if (condition->opc == OPC_SHPS_MACRO &&
              condition->block == ctx->block) {
      /* TODO: technically this only works if the block is the only user of the
       * shps, but we only use it in very constrained scenarios so this should
       * be ok.
       */
      ctx->block->condition = NULL;
      ctx->block->brtype = IR3_BRANCH_SHPS;
   } else {
      ctx->block->condition = ir3_get_predicate(ctx, condition);
      ctx->block->brtype = IR3_BRANCH_COND;
   }

   emit_cf_list(ctx, &nif->then_list);
   emit_cf_list(ctx, &nif->else_list);

   struct ir3_block *last_then = get_block(ctx, nir_if_last_then_block(nif));
   struct ir3_block *first_else = get_block(ctx, nir_if_first_else_block(nif));
   assert(last_then->physical_successors[0] &&
          !last_then->physical_successors[1]);
   last_then->physical_successors[1] = first_else;

   struct ir3_block *last_else = get_block(ctx, nir_if_last_else_block(nif));
   struct ir3_block *after_if =
      get_block(ctx, nir_cf_node_as_block(nir_cf_node_next(&nif->cf_node)));
   assert(last_else->physical_successors[0] &&
          !last_else->physical_successors[1]);
   if (after_if != last_else->physical_successors[0])
      last_else->physical_successors[1] = after_if;
}

static void
emit_loop(struct ir3_context *ctx, nir_loop *nloop)
{
   assert(!nir_loop_has_continue_construct(nloop));
   unsigned old_loop_id = ctx->loop_id;
   ctx->loop_id = ctx->so->loops + 1;
   ctx->loop_depth++;

   struct nir_block *nstart = nir_loop_first_block(nloop);
   struct ir3_block *continue_blk = NULL;

   /* There's always one incoming edge from outside the loop, and if there
    * are more than two backedges from inside the loop (so more than 2 total
    * edges) then we need to create a continue block after the loop to ensure
    * that control reconverges at the end of each loop iteration.
    */
   if (nstart->predecessors->entries > 2) {
      continue_blk = create_continue_block(ctx, nstart);
   }

   emit_cf_list(ctx, &nloop->body);

   if (continue_blk) {
      struct ir3_block *start = get_block(ctx, nstart);
      continue_blk->successors[0] = start;
      continue_blk->physical_successors[0] = start;
      continue_blk->loop_id = ctx->loop_id;
      continue_blk->loop_depth = ctx->loop_depth;
      list_addtail(&continue_blk->node, &ctx->ir->block_list);
   }

   ctx->so->loops++;
   ctx->loop_depth--;
   ctx->loop_id = old_loop_id;
}

static void
stack_push(struct ir3_context *ctx)
{
   ctx->stack++;
   ctx->max_stack = MAX2(ctx->max_stack, ctx->stack);
}

static void
stack_pop(struct ir3_context *ctx)
{
   compile_assert(ctx, ctx->stack > 0);
   ctx->stack--;
}

static void
emit_cf_list(struct ir3_context *ctx, struct exec_list *list)
{
   foreach_list_typed (nir_cf_node, node, node, list) {
      switch (node->type) {
      case nir_cf_node_block:
         emit_block(ctx, nir_cf_node_as_block(node));
         break;
      case nir_cf_node_if:
         stack_push(ctx);
         emit_if(ctx, nir_cf_node_as_if(node));
         stack_pop(ctx);
         break;
      case nir_cf_node_loop:
         stack_push(ctx);
         emit_loop(ctx, nir_cf_node_as_loop(node));
         stack_pop(ctx);
         break;
      case nir_cf_node_function:
         ir3_context_error(ctx, "TODO\n");
         break;
      }
   }
}

/* emit stream-out code.  At this point, the current block is the original
 * (nir) end block, and nir ensures that all flow control paths terminate
 * into the end block.  We re-purpose the original end block to generate
 * the 'if (vtxcnt < maxvtxcnt)' condition, then append the conditional
 * block holding stream-out write instructions, followed by the new end
 * block:
 *
 *   blockOrigEnd {
 *      p0.x = (vtxcnt < maxvtxcnt)
 *      // succs: blockStreamOut, blockNewEnd
 *   }
 *   blockStreamOut {
 *      // preds: blockOrigEnd
 *      ... stream-out instructions ...
 *      // succs: blockNewEnd
 *   }
 *   blockNewEnd {
 *      // preds: blockOrigEnd, blockStreamOut
 *   }
 */
static void
emit_stream_out(struct ir3_context *ctx)
{
   struct ir3 *ir = ctx->ir;
   struct ir3_stream_output_info *strmout = &ctx->so->stream_output;
   struct ir3_block *orig_end_block, *stream_out_block, *new_end_block;
   struct ir3_instruction *vtxcnt, *maxvtxcnt, *cond;
   struct ir3_instruction *bases[IR3_MAX_SO_BUFFERS];

   /* create vtxcnt input in input block at top of shader,
    * so that it is seen as live over the entire duration
    * of the shader:
    */
   vtxcnt = create_sysval_input(ctx, SYSTEM_VALUE_VERTEX_CNT, 0x1);
   maxvtxcnt = create_driver_param(ctx, IR3_DP_VTXCNT_MAX);

   /* at this point, we are at the original 'end' block,
    * re-purpose this block to stream-out condition, then
    * append stream-out block and new-end block
    */
   orig_end_block = ctx->block;

   // maybe w/ store_global intrinsic, we could do this
   // stuff in nir->nir pass

   stream_out_block = ir3_block_create(ir);
   list_addtail(&stream_out_block->node, &ir->block_list);

   new_end_block = ir3_block_create(ir);
   list_addtail(&new_end_block->node, &ir->block_list);

   orig_end_block->successors[0] = stream_out_block;
   orig_end_block->successors[1] = new_end_block;

   orig_end_block->physical_successors[0] = stream_out_block;
   orig_end_block->physical_successors[1] = new_end_block;

   stream_out_block->successors[0] = new_end_block;

   stream_out_block->physical_successors[0] = new_end_block;

   /* setup 'if (vtxcnt < maxvtxcnt)' condition: */
   cond = ir3_CMPS_S(ctx->block, vtxcnt, 0, maxvtxcnt, 0);
   cond->dsts[0]->num = regid(REG_P0, 0);
   cond->dsts[0]->flags &= ~IR3_REG_SSA;
   cond->cat2.condition = IR3_COND_LT;

   /* condition goes on previous block to the conditional,
    * since it is used to pick which of the two successor
    * paths to take:
    */
   orig_end_block->condition = cond;

   /* switch to stream_out_block to generate the stream-out
    * instructions:
    */
   ctx->block = stream_out_block;

   /* Calculate base addresses based on vtxcnt.  Instructions
    * generated for bases not used in following loop will be
    * stripped out in the backend.
    */
   for (unsigned i = 0; i < IR3_MAX_SO_BUFFERS; i++) {
      const struct ir3_const_state *const_state = ir3_const_state(ctx->so);
      unsigned stride = strmout->stride[i];
      struct ir3_instruction *base, *off;

      base = create_uniform(ctx->block, regid(const_state->offsets.tfbo, i));

      /* 24-bit should be enough: */
      off = ir3_MUL_U24(ctx->block, vtxcnt, 0,
                        create_immed(ctx->block, stride * 4), 0);

      bases[i] = ir3_ADD_S(ctx->block, off, 0, base, 0);
   }

   /* Generate the per-output store instructions: */
   for (unsigned i = 0; i < strmout->num_outputs; i++) {
      for (unsigned j = 0; j < strmout->output[i].num_components; j++) {
         unsigned c = j + strmout->output[i].start_component;
         struct ir3_instruction *base, *out, *stg;

         base = bases[strmout->output[i].output_buffer];
         out = ctx->outputs[regid(strmout->output[i].register_index, c)];

         stg = ir3_STG(
            ctx->block, base, 0,
            create_immed(ctx->block, (strmout->output[i].dst_offset + j) * 4),
            0, out, 0, create_immed(ctx->block, 1), 0);
         stg->cat6.type = TYPE_U32;

         array_insert(ctx->block, ctx->block->keeps, stg);
      }
   }

   /* and finally switch to the new_end_block: */
   ctx->block = new_end_block;
}

static void
setup_predecessors(struct ir3 *ir)
{
   foreach_block (block, &ir->block_list) {
      for (int i = 0; i < ARRAY_SIZE(block->successors); i++) {
         if (block->successors[i])
            ir3_block_add_predecessor(block->successors[i], block);
         if (block->physical_successors[i])
            ir3_block_add_physical_predecessor(block->physical_successors[i],
                                               block);
      }
   }
}

static void
emit_function(struct ir3_context *ctx, nir_function_impl *impl)
{
   nir_metadata_require(impl, nir_metadata_block_index);

   compile_assert(ctx, ctx->stack == 0);

   emit_cf_list(ctx, &impl->body);
   emit_block(ctx, impl->end_block);

   compile_assert(ctx, ctx->stack == 0);

   /* at this point, we should have a single empty block,
    * into which we emit the 'end' instruction.
    */
   compile_assert(ctx, list_is_empty(&ctx->block->instr_list));

   /* If stream-out (aka transform-feedback) enabled, emit the
    * stream-out instructions, followed by a new empty block (into
    * which the 'end' instruction lands).
    *
    * NOTE: it is done in this order, rather than inserting before
    * we emit end_block, because NIR guarantees that all blocks
    * flow into end_block, and that end_block has no successors.
    * So by re-purposing end_block as the first block of stream-
    * out, we guarantee that all exit paths flow into the stream-
    * out instructions.
    */
   if ((ctx->compiler->gen < 5) &&
       (ctx->so->stream_output.num_outputs > 0) &&
       !ctx->so->binning_pass) {
      assert(ctx->so->type == MESA_SHADER_VERTEX);
      emit_stream_out(ctx);
   }

   setup_predecessors(ctx->ir);
   foreach_block (block, &ctx->ir->block_list) {
      resolve_phis(ctx, block);
   }
}

static void
setup_input(struct ir3_context *ctx, nir_intrinsic_instr *intr)
{
   struct ir3_shader_variant *so = ctx->so;
   struct ir3_instruction *coord = NULL;

   if (intr->intrinsic == nir_intrinsic_load_interpolated_input)
      coord = ir3_create_collect(ctx->block, ir3_get_src(ctx, &intr->src[0]), 2);

   compile_assert(ctx, nir_src_is_const(intr->src[coord ? 1 : 0]));

   unsigned frac = nir_intrinsic_component(intr);
   unsigned offset = nir_src_as_uint(intr->src[coord ? 1 : 0]);
   unsigned ncomp = nir_intrinsic_dest_components(intr);
   unsigned n = nir_intrinsic_base(intr) + offset;
   unsigned slot = nir_intrinsic_io_semantics(intr).location + offset;
   unsigned compmask;

   /* Inputs are loaded using ldlw or ldg for other stages. */
   compile_assert(ctx, ctx->so->type == MESA_SHADER_FRAGMENT ||
                          ctx->so->type == MESA_SHADER_VERTEX);

   if (ctx->so->type == MESA_SHADER_FRAGMENT)
      compmask = BITFIELD_MASK(ncomp) << frac;
   else
      compmask = BITFIELD_MASK(ncomp + frac);

   /* for a4xx+ rasterflat */
   if (so->inputs[n].rasterflat && ctx->so->key.rasterflat)
      coord = NULL;

   so->total_in += util_bitcount(compmask & ~so->inputs[n].compmask);

   so->inputs[n].slot = slot;
   so->inputs[n].compmask |= compmask;
   so->inputs_count = MAX2(so->inputs_count, n + 1);
   compile_assert(ctx, so->inputs_count < ARRAY_SIZE(so->inputs));
   so->inputs[n].flat = !coord;

   if (ctx->so->type == MESA_SHADER_FRAGMENT) {
      compile_assert(ctx, slot != VARYING_SLOT_POS);

      so->inputs[n].bary = true;

      for (int i = 0; i < ncomp; i++) {
         unsigned idx = (n * 4) + i + frac;
         ctx->last_dst[i] = create_frag_input(ctx, coord, idx);
      }

      if (slot == VARYING_SLOT_PRIMITIVE_ID)
         so->reads_primid = true;
   } else {
      struct ir3_instruction *input = NULL;

      foreach_input (in, ctx->ir) {
         if (in->input.inidx == n) {
            input = in;
            break;
         }
      }

      if (!input) {
         input = create_input(ctx, compmask);
         input->input.inidx = n;
      } else {
         /* For aliased inputs, just append to the wrmask.. ie. if we
          * first see a vec2 index at slot N, and then later a vec4,
          * the wrmask of the resulting overlapped vec2 and vec4 is 0xf
          */
         input->dsts[0]->wrmask |= compmask;
      }

      for (int i = 0; i < ncomp + frac; i++) {
         unsigned idx = (n * 4) + i;
         compile_assert(ctx, idx < ctx->ninputs);

         /* fixup the src wrmask to avoid validation fail */
         if (ctx->inputs[idx] && (ctx->inputs[idx] != input)) {
            ctx->inputs[idx]->srcs[0]->wrmask = input->dsts[0]->wrmask;
            continue;
         }

         ir3_split_dest(ctx->block, &ctx->inputs[idx], input, i, 1);
      }

      for (int i = 0; i < ncomp; i++) {
         unsigned idx = (n * 4) + i + frac;
         ctx->last_dst[i] = ctx->inputs[idx];
      }
   }
}

/* Initially we assign non-packed inloc's for varyings, as we don't really
 * know up-front which components will be unused.  After all the compilation
 * stages we scan the shader to see which components are actually used, and
 * re-pack the inlocs to eliminate unneeded varyings.
 */
static void
pack_inlocs(struct ir3_context *ctx)
{
   struct ir3_shader_variant *so = ctx->so;
   uint8_t used_components[so->inputs_count];

   memset(used_components, 0, sizeof(used_components));

   /*
    * First Step: scan shader to find which bary.f/ldlv remain:
    */

   foreach_block (block, &ctx->ir->block_list) {
      foreach_instr (instr, &block->instr_list) {
         if (is_input(instr)) {
            unsigned inloc = instr->srcs[0]->iim_val;
            unsigned i = inloc / 4;
            unsigned j = inloc % 4;

            compile_assert(ctx, instr->srcs[0]->flags & IR3_REG_IMMED);
            compile_assert(ctx, i < so->inputs_count);

            used_components[i] |= 1 << j;
         } else if (instr->opc == OPC_META_TEX_PREFETCH) {
            for (int n = 0; n < 2; n++) {
               unsigned inloc = instr->prefetch.input_offset + n;
               unsigned i = inloc / 4;
               unsigned j = inloc % 4;

               compile_assert(ctx, i < so->inputs_count);

               used_components[i] |= 1 << j;
            }
         }
      }
   }

   /*
    * Second Step: reassign varying inloc/slots:
    */

   unsigned inloc = 0;

   /* for clip+cull distances, unused components can't be eliminated because
    * they're read by fixed-function, even if there's a hole.  Note that
    * clip/cull distance arrays must be declared in the FS, so we can just
    * use the NIR clip/cull distances to avoid reading ucp_enables in the
    * shader key.
    */
   unsigned clip_cull_mask = so->clip_mask | so->cull_mask;

   for (unsigned i = 0; i < so->inputs_count; i++) {
      unsigned compmask = 0, maxcomp = 0;

      so->inputs[i].inloc = inloc;
      so->inputs[i].bary = false;

      if (so->inputs[i].slot == VARYING_SLOT_CLIP_DIST0 ||
          so->inputs[i].slot == VARYING_SLOT_CLIP_DIST1) {
         if (so->inputs[i].slot == VARYING_SLOT_CLIP_DIST0)
            compmask = clip_cull_mask & 0xf;
         else
            compmask = clip_cull_mask >> 4;
         used_components[i] = compmask;
      }

      for (unsigned j = 0; j < 4; j++) {
         if (!(used_components[i] & (1 << j)))
            continue;

         compmask |= (1 << j);
         maxcomp = j + 1;

         /* at this point, since used_components[i] mask is only
          * considering varyings (ie. not sysvals) we know this
          * is a varying:
          */
         so->inputs[i].bary = true;
      }

      if (so->inputs[i].bary) {
         so->varying_in++;
         so->inputs[i].compmask = (1 << maxcomp) - 1;
         inloc += maxcomp;
      }
   }

   /*
    * Third Step: reassign packed inloc's:
    */

   foreach_block (block, &ctx->ir->block_list) {
      foreach_instr (instr, &block->instr_list) {
         if (is_input(instr)) {
            unsigned inloc = instr->srcs[0]->iim_val;
            unsigned i = inloc / 4;
            unsigned j = inloc % 4;

            instr->srcs[0]->iim_val = so->inputs[i].inloc + j;
            if (instr->opc == OPC_FLAT_B)
               instr->srcs[1]->iim_val = instr->srcs[0]->iim_val;
         } else if (instr->opc == OPC_META_TEX_PREFETCH) {
            unsigned i = instr->prefetch.input_offset / 4;
            unsigned j = instr->prefetch.input_offset % 4;
            instr->prefetch.input_offset = so->inputs[i].inloc + j;
         }
      }
   }
}

static void
setup_output(struct ir3_context *ctx, nir_intrinsic_instr *intr)
{
   struct ir3_shader_variant *so = ctx->so;
   nir_io_semantics io = nir_intrinsic_io_semantics(intr);

   compile_assert(ctx, nir_src_is_const(intr->src[1]));

   unsigned offset = nir_src_as_uint(intr->src[1]);
   unsigned n = nir_intrinsic_base(intr) + offset;
   unsigned frac = nir_intrinsic_component(intr);
   unsigned ncomp = nir_intrinsic_src_components(intr, 0);

   /* For per-view variables, each user-facing slot corresponds to multiple
    * views, each with a corresponding driver_location, and the offset is for
    * the driver_location. To properly figure out of the slot, we'd need to
    * plumb through the number of views. However, for now we only use
    * per-view with gl_Position, so we assume that the variable is not an
    * array or matrix (so there are no indirect accesses to the variable
    * itself) and the indirect offset corresponds to the view.
    */
   unsigned slot = io.location + (io.per_view ? 0 : offset);

   if (io.per_view && offset > 0)
      so->multi_pos_output = true;

   if (ctx->so->type == MESA_SHADER_FRAGMENT) {
      switch (slot) {
      case FRAG_RESULT_DEPTH:
         so->writes_pos = true;
         break;
      case FRAG_RESULT_COLOR:
         if (!ctx->s->info.fs.color_is_dual_source) {
            so->color0_mrt = 1;
         } else {
            slot = FRAG_RESULT_DATA0 + io.dual_source_blend_index;
            if (io.dual_source_blend_index > 0)
               so->dual_src_blend = true;
         }
         break;
      case FRAG_RESULT_SAMPLE_MASK:
         so->writes_smask = true;
         break;
      case FRAG_RESULT_STENCIL:
         so->writes_stencilref = true;
         break;
      default:
         slot += io.dual_source_blend_index; /* For dual-src blend */
         if (io.dual_source_blend_index > 0)
            so->dual_src_blend = true;
         if (slot >= FRAG_RESULT_DATA0)
            break;
         ir3_context_error(ctx, "unknown FS output name: %s\n",
                           gl_frag_result_name(slot));
      }
   } else if (ctx->so->type == MESA_SHADER_VERTEX ||
              ctx->so->type == MESA_SHADER_TESS_EVAL ||
              ctx->so->type == MESA_SHADER_GEOMETRY) {
      switch (slot) {
      case VARYING_SLOT_POS:
         so->writes_pos = true;
         break;
      case VARYING_SLOT_PSIZ:
         so->writes_psize = true;
         break;
      case VARYING_SLOT_VIEWPORT:
         so->writes_viewport = true;
         break;
      case VARYING_SLOT_PRIMITIVE_ID:
      case VARYING_SLOT_GS_VERTEX_FLAGS_IR3:
         assert(ctx->so->type == MESA_SHADER_GEOMETRY);
         FALLTHROUGH;
      case VARYING_SLOT_COL0:
      case VARYING_SLOT_COL1:
      case VARYING_SLOT_BFC0:
      case VARYING_SLOT_BFC1:
      case VARYING_SLOT_FOGC:
      case VARYING_SLOT_CLIP_DIST0:
      case VARYING_SLOT_CLIP_DIST1:
      case VARYING_SLOT_CLIP_VERTEX:
      case VARYING_SLOT_LAYER:
         break;
      default:
         if (slot >= VARYING_SLOT_VAR0)
            break;
         if ((VARYING_SLOT_TEX0 <= slot) && (slot <= VARYING_SLOT_TEX7))
            break;
         ir3_context_error(ctx, "unknown %s shader output name: %s\n",
                           _mesa_shader_stage_to_string(ctx->so->type),
                           gl_varying_slot_name_for_stage(slot, ctx->so->type));
      }
   } else {
      ir3_context_error(ctx, "unknown shader type: %d\n", ctx->so->type);
   }

   so->outputs_count = MAX2(so->outputs_count, n + 1);
   compile_assert(ctx, so->outputs_count <= ARRAY_SIZE(so->outputs));

   so->outputs[n].slot = slot;
   if (io.per_view)
      so->outputs[n].view = offset;

   for (int i = 0; i < ncomp; i++) {
      unsigned idx = (n * 4) + i + frac;
      compile_assert(ctx, idx < ctx->noutputs);
      ctx->outputs[idx] = create_immed(ctx->block, fui(0.0));
   }

   /* if varying packing doesn't happen, we could end up in a situation
    * with "holes" in the output, and since the per-generation code that
    * sets up varying linkage registers doesn't expect to have more than
    * one varying per vec4 slot, pad the holes.
    *
    * Note that this should probably generate a performance warning of
    * some sort.
    */
   for (int i = 0; i < frac; i++) {
      unsigned idx = (n * 4) + i;
      if (!ctx->outputs[idx]) {
         ctx->outputs[idx] = create_immed(ctx->block, fui(0.0));
      }
   }

   struct ir3_instruction *const *src = ir3_get_src(ctx, &intr->src[0]);
   for (int i = 0; i < ncomp; i++) {
      unsigned idx = (n * 4) + i + frac;
      ctx->outputs[idx] = src[i];
   }
}

static bool
uses_load_input(struct ir3_shader_variant *so)
{
   return so->type == MESA_SHADER_VERTEX || so->type == MESA_SHADER_FRAGMENT;
}

static bool
uses_store_output(struct ir3_shader_variant *so)
{
   switch (so->type) {
   case MESA_SHADER_VERTEX:
      return !so->key.has_gs && !so->key.tessellation;
   case MESA_SHADER_TESS_EVAL:
      return !so->key.has_gs;
   case MESA_SHADER_GEOMETRY:
   case MESA_SHADER_FRAGMENT:
      return true;
   case MESA_SHADER_TESS_CTRL:
   case MESA_SHADER_COMPUTE:
   case MESA_SHADER_KERNEL:
      return false;
   default:
      unreachable("unknown stage");
   }
}

static void
emit_instructions(struct ir3_context *ctx)
{
   MESA_TRACE_FUNC();

   nir_function_impl *fxn = nir_shader_get_entrypoint(ctx->s);

   /* some varying setup which can't be done in setup_input(): */
   if (ctx->so->type == MESA_SHADER_FRAGMENT) {
      nir_foreach_shader_in_variable (var, ctx->s) {
         /* set rasterflat flag for front/back color */
         if (var->data.interpolation == INTERP_MODE_NONE) {
            switch (var->data.location) {
            case VARYING_SLOT_COL0:
            case VARYING_SLOT_COL1:
            case VARYING_SLOT_BFC0:
            case VARYING_SLOT_BFC1:
               ctx->so->inputs[var->data.driver_location].rasterflat = true;
               break;
            default:
               break;
            }
         }
      }
   }

   if (uses_load_input(ctx->so)) {
      ctx->so->inputs_count = ctx->s->num_inputs;
      compile_assert(ctx, ctx->so->inputs_count < ARRAY_SIZE(ctx->so->inputs));
      ctx->ninputs = ctx->s->num_inputs * 4;
      ctx->inputs = rzalloc_array(ctx, struct ir3_instruction *, ctx->ninputs);
   } else {
      ctx->ninputs = 0;
      ctx->so->inputs_count = 0;
   }

   if (uses_store_output(ctx->so)) {
      ctx->noutputs = ctx->s->num_outputs * 4;
      ctx->outputs =
         rzalloc_array(ctx, struct ir3_instruction *, ctx->noutputs);
   } else {
      ctx->noutputs = 0;
   }

   ctx->ir = ir3_create(ctx->compiler, ctx->so);

   /* Create inputs in first block: */
   ctx->block = get_block(ctx, nir_start_block(fxn));
   ctx->in_block = ctx->block;

   /* for fragment shader, the vcoord input register is used as the
    * base for bary.f varying fetch instrs:
    *
    * TODO defer creating ctx->ij_pixel and corresponding sysvals
    * until emit_intrinsic when we know they are actually needed.
    * For now, we defer creating ctx->ij_centroid, etc, since we
    * only need ij_pixel for "old style" varying inputs (ie.
    * tgsi_to_nir)
    */
   if (ctx->so->type == MESA_SHADER_FRAGMENT) {
      ctx->ij[IJ_PERSP_PIXEL] = create_input(ctx, 0x3);
   }

   /* Defer add_sysval_input() stuff until after setup_inputs(),
    * because sysvals need to be appended after varyings:
    */
   if (ctx->ij[IJ_PERSP_PIXEL]) {
      add_sysval_input_compmask(ctx, SYSTEM_VALUE_BARYCENTRIC_PERSP_PIXEL, 0x3,
                                ctx->ij[IJ_PERSP_PIXEL]);
   }

   /* Tesselation shaders always need primitive ID for indexing the
    * BO. Geometry shaders don't always need it but when they do it has be
    * delivered and unclobbered in the VS. To make things easy, we always
    * make room for it in VS/DS.
    */
   bool has_tess = ctx->so->key.tessellation != IR3_TESS_NONE;
   bool has_gs = ctx->so->key.has_gs;
   switch (ctx->so->type) {
   case MESA_SHADER_VERTEX:
      if (has_tess) {
         ctx->tcs_header =
            create_sysval_input(ctx, SYSTEM_VALUE_TCS_HEADER_IR3, 0x1);
         ctx->rel_patch_id =
            create_sysval_input(ctx, SYSTEM_VALUE_REL_PATCH_ID_IR3, 0x1);
         ctx->primitive_id =
            create_sysval_input(ctx, SYSTEM_VALUE_PRIMITIVE_ID, 0x1);
      } else if (has_gs) {
         ctx->gs_header =
            create_sysval_input(ctx, SYSTEM_VALUE_GS_HEADER_IR3, 0x1);
         ctx->primitive_id =
            create_sysval_input(ctx, SYSTEM_VALUE_PRIMITIVE_ID, 0x1);
      }
      break;
   case MESA_SHADER_TESS_CTRL:
      ctx->tcs_header =
         create_sysval_input(ctx, SYSTEM_VALUE_TCS_HEADER_IR3, 0x1);
      ctx->rel_patch_id =
         create_sysval_input(ctx, SYSTEM_VALUE_REL_PATCH_ID_IR3, 0x1);
      break;
   case MESA_SHADER_TESS_EVAL:
      if (has_gs) {
         ctx->gs_header =
            create_sysval_input(ctx, SYSTEM_VALUE_GS_HEADER_IR3, 0x1);
         ctx->primitive_id =
            create_sysval_input(ctx, SYSTEM_VALUE_PRIMITIVE_ID, 0x1);
      }
      ctx->rel_patch_id =
         create_sysval_input(ctx, SYSTEM_VALUE_REL_PATCH_ID_IR3, 0x1);
      break;
   case MESA_SHADER_GEOMETRY:
      ctx->gs_header =
         create_sysval_input(ctx, SYSTEM_VALUE_GS_HEADER_IR3, 0x1);
      break;
   default:
      break;
   }

   /* Find # of samplers. Just assume that we'll be reading from images.. if
    * it is write-only we don't have to count it, but after lowering derefs
    * is too late to compact indices for that.
    */
   ctx->so->num_samp =
      BITSET_LAST_BIT(ctx->s->info.textures_used) + ctx->s->info.num_images;

   /* Save off clip+cull information. Note that in OpenGL clip planes may
    * be individually enabled/disabled, and some gens handle lowering in
    * backend, so we also need to consider the shader key:
    */
   ctx->so->clip_mask = ctx->so->key.ucp_enables |
                        MASK(ctx->s->info.clip_distance_array_size);
   ctx->so->cull_mask = MASK(ctx->s->info.cull_distance_array_size)
                        << ctx->s->info.clip_distance_array_size;

   ctx->so->pvtmem_size = ctx->s->scratch_size;
   ctx->so->shared_size = ctx->s->info.shared_size;

   /* NOTE: need to do something more clever when we support >1 fxn */
   nir_foreach_reg_decl (decl, fxn) {
      ir3_declare_array(ctx, decl);
   }

   if (ctx->so->type == MESA_SHADER_TESS_CTRL &&
       ctx->compiler->tess_use_shared) {
      struct ir3_instruction *barrier = ir3_BAR(ctx->block);
      barrier->flags = IR3_INSTR_SS | IR3_INSTR_SY;
      barrier->barrier_class = IR3_BARRIER_EVERYTHING;
      array_insert(ctx->block, ctx->block->keeps, barrier);
      ctx->so->has_barrier = true;
   }

   /* And emit the body: */
   ctx->impl = fxn;
   emit_function(ctx, fxn);
}

/* Fixup tex sampler state for astc/srgb workaround instructions.  We
 * need to assign the tex state indexes for these after we know the
 * max tex index.
 */
static void
fixup_astc_srgb(struct ir3_context *ctx)
{
   struct ir3_shader_variant *so = ctx->so;
   /* indexed by original tex idx, value is newly assigned alpha sampler
    * state tex idx.  Zero is invalid since there is at least one sampler
    * if we get here.
    */
   unsigned alt_tex_state[16] = {0};
   unsigned tex_idx = ctx->max_texture_index + 1;
   unsigned idx = 0;

   so->astc_srgb.base = tex_idx;

   for (unsigned i = 0; i < ctx->ir->astc_srgb_count; i++) {
      struct ir3_instruction *sam = ctx->ir->astc_srgb[i];

      compile_assert(ctx, sam->cat5.tex < ARRAY_SIZE(alt_tex_state));

      if (alt_tex_state[sam->cat5.tex] == 0) {
         /* assign new alternate/alpha tex state slot: */
         alt_tex_state[sam->cat5.tex] = tex_idx++;
         so->astc_srgb.orig_idx[idx++] = sam->cat5.tex;
         so->astc_srgb.count++;
      }

      sam->cat5.tex = alt_tex_state[sam->cat5.tex];
   }
}

/* Fixup tex sampler state for tg4 workaround instructions.  We
 * need to assign the tex state indexes for these after we know the
 * max tex index.
 */
static void
fixup_tg4(struct ir3_context *ctx)
{
   struct ir3_shader_variant *so = ctx->so;
   /* indexed by original tex idx, value is newly assigned alpha sampler
    * state tex idx.  Zero is invalid since there is at least one sampler
    * if we get here.
    */
   unsigned alt_tex_state[16] = {0};
   unsigned tex_idx = ctx->max_texture_index + so->astc_srgb.count + 1;
   unsigned idx = 0;

   so->tg4.base = tex_idx;

   for (unsigned i = 0; i < ctx->ir->tg4_count; i++) {
      struct ir3_instruction *sam = ctx->ir->tg4[i];

      compile_assert(ctx, sam->cat5.tex < ARRAY_SIZE(alt_tex_state));

      if (alt_tex_state[sam->cat5.tex] == 0) {
         /* assign new alternate/alpha tex state slot: */
         alt_tex_state[sam->cat5.tex] = tex_idx++;
         so->tg4.orig_idx[idx++] = sam->cat5.tex;
         so->tg4.count++;
      }

      sam->cat5.tex = alt_tex_state[sam->cat5.tex];
   }
}

static bool
output_slot_used_for_binning(gl_varying_slot slot)
{
   return slot == VARYING_SLOT_POS || slot == VARYING_SLOT_PSIZ ||
          slot == VARYING_SLOT_CLIP_DIST0 || slot == VARYING_SLOT_CLIP_DIST1 ||
          slot == VARYING_SLOT_VIEWPORT;
}

static struct ir3_instruction *
find_end(struct ir3 *ir)
{
   foreach_block_rev (block, &ir->block_list) {
      foreach_instr_rev (instr, &block->instr_list) {
         if (instr->opc == OPC_END || instr->opc == OPC_CHMASK)
            return instr;
      }
   }
   unreachable("couldn't find end instruction");
}

static void
fixup_binning_pass(struct ir3_context *ctx, struct ir3_instruction *end)
{
   struct ir3_shader_variant *so = ctx->so;
   unsigned i, j;

   /* first pass, remove unused outputs from the IR level outputs: */
   for (i = 0, j = 0; i < end->srcs_count; i++) {
      unsigned outidx = end->end.outidxs[i];
      unsigned slot = so->outputs[outidx].slot;

      if (output_slot_used_for_binning(slot)) {
         end->srcs[j] = end->srcs[i];
         end->end.outidxs[j] = end->end.outidxs[i];
         j++;
      }
   }
   end->srcs_count = j;

   /* second pass, cleanup the unused slots in ir3_shader_variant::outputs
    * table:
    */
   for (i = 0, j = 0; i < so->outputs_count; i++) {
      unsigned slot = so->outputs[i].slot;

      if (output_slot_used_for_binning(slot)) {
         so->outputs[j] = so->outputs[i];

         /* fixup outidx to point to new output table entry: */
         for (unsigned k = 0; k < end->srcs_count; k++) {
            if (end->end.outidxs[k] == i) {
               end->end.outidxs[k] = j;
               break;
            }
         }

         j++;
      }
   }
   so->outputs_count = j;
}

static void
collect_tex_prefetches(struct ir3_context *ctx, struct ir3 *ir)
{
   unsigned idx = 0;

   /* Collect sampling instructions eligible for pre-dispatch. */
   foreach_block (block, &ir->block_list) {
      foreach_instr_safe (instr, &block->instr_list) {
         if (instr->opc == OPC_META_TEX_PREFETCH) {
            assert(idx < ARRAY_SIZE(ctx->so->sampler_prefetch));
            struct ir3_sampler_prefetch *fetch =
               &ctx->so->sampler_prefetch[idx];
            idx++;

            fetch->bindless = instr->flags & IR3_INSTR_B;
            if (fetch->bindless) {
               /* In bindless mode, the index is actually the base */
               fetch->tex_id = instr->prefetch.tex_base;
               fetch->samp_id = instr->prefetch.samp_base;
               fetch->tex_bindless_id = instr->prefetch.tex;
               fetch->samp_bindless_id = instr->prefetch.samp;
            } else {
               fetch->tex_id = instr->prefetch.tex;
               fetch->samp_id = instr->prefetch.samp;
            }
            fetch->tex_opc = OPC_SAM;
            fetch->wrmask = instr->dsts[0]->wrmask;
            fetch->dst = instr->dsts[0]->num;
            fetch->src = instr->prefetch.input_offset;

            /* These are the limits on a5xx/a6xx, we might need to
             * revisit if SP_FS_PREFETCH[n] changes on later gens:
             */
            assert(fetch->dst <= 0x3f);
            assert(fetch->tex_id <= 0x1f);
            assert(fetch->samp_id <= 0xf);

            ctx->so->total_in =
               MAX2(ctx->so->total_in, instr->prefetch.input_offset + 2);

            fetch->half_precision = !!(instr->dsts[0]->flags & IR3_REG_HALF);

            /* Remove the prefetch placeholder instruction: */
            list_delinit(&instr->node);
         }
      }
   }
}

int
ir3_compile_shader_nir(struct ir3_compiler *compiler,
                       struct ir3_shader *shader,
                       struct ir3_shader_variant *so)
{
   struct ir3_context *ctx;
   struct ir3 *ir;
   int ret = 0, max_bary;
   bool progress;

   MESA_TRACE_FUNC();

   assert(!so->ir);

   ctx = ir3_context_init(compiler, shader, so);
   if (!ctx) {
      DBG("INIT failed!");
      ret = -1;
      goto out;
   }

   emit_instructions(ctx);

   if (ctx->error) {
      DBG("EMIT failed!");
      ret = -1;
      goto out;
   }

   ir = so->ir = ctx->ir;

   if (gl_shader_stage_is_compute(so->type)) {
      so->local_size[0] = ctx->s->info.workgroup_size[0];
      so->local_size[1] = ctx->s->info.workgroup_size[1];
      so->local_size[2] = ctx->s->info.workgroup_size[2];
      so->local_size_variable = ctx->s->info.workgroup_size_variable;
   }

   /* Vertex shaders in a tessellation or geometry pipeline treat END as a
    * NOP and has an epilogue that writes the VS outputs to local storage, to
    * be read by the HS.  Then it resets execution mask (chmask) and chains
    * to the next shader (chsh). There are also a few output values which we
    * must send to the next stage via registers, and in order for both stages
    * to agree on the register used we must force these to be in specific
    * registers.
    */
   if ((so->type == MESA_SHADER_VERTEX &&
        (so->key.has_gs || so->key.tessellation)) ||
       (so->type == MESA_SHADER_TESS_EVAL && so->key.has_gs)) {
      struct ir3_instruction *outputs[3];
      unsigned outidxs[3];
      unsigned regids[3];
      unsigned outputs_count = 0;

      if (ctx->primitive_id) {
         unsigned n = so->outputs_count++;
         so->outputs[n].slot = VARYING_SLOT_PRIMITIVE_ID;

         struct ir3_instruction *out = ir3_collect(ctx->block, ctx->primitive_id);
         outputs[outputs_count] = out;
         outidxs[outputs_count] = n;
         if (so->type == MESA_SHADER_VERTEX && ctx->rel_patch_id)
            regids[outputs_count] = regid(0, 2);
         else
            regids[outputs_count] = regid(0, 1);
         outputs_count++;
      }

      if (so->type == MESA_SHADER_VERTEX && ctx->rel_patch_id) {
         unsigned n = so->outputs_count++;
         so->outputs[n].slot = VARYING_SLOT_REL_PATCH_ID_IR3;
         struct ir3_instruction *out = ir3_collect(ctx->block, ctx->rel_patch_id);
         outputs[outputs_count] = out;
         outidxs[outputs_count] = n;
         regids[outputs_count] = regid(0, 1);
         outputs_count++;
      }

      if (ctx->gs_header) {
         unsigned n = so->outputs_count++;
         so->outputs[n].slot = VARYING_SLOT_GS_HEADER_IR3;
         struct ir3_instruction *out = ir3_collect(ctx->block, ctx->gs_header);
         outputs[outputs_count] = out;
         outidxs[outputs_count] = n;
         regids[outputs_count] = regid(0, 0);
         outputs_count++;
      }

      if (ctx->tcs_header) {
         unsigned n = so->outputs_count++;
         so->outputs[n].slot = VARYING_SLOT_TCS_HEADER_IR3;
         struct ir3_instruction *out = ir3_collect(ctx->block, ctx->tcs_header);
         outputs[outputs_count] = out;
         outidxs[outputs_count] = n;
         regids[outputs_count] = regid(0, 0);
         outputs_count++;
      }

      struct ir3_instruction *chmask =
         ir3_instr_create(ctx->block, OPC_CHMASK, 0, outputs_count);
      chmask->barrier_class = IR3_BARRIER_EVERYTHING;
      chmask->barrier_conflict = IR3_BARRIER_EVERYTHING;

      for (unsigned i = 0; i < outputs_count; i++)
         __ssa_src(chmask, outputs[i], 0)->num = regids[i];

      chmask->end.outidxs = ralloc_array(chmask, unsigned, outputs_count);
      memcpy(chmask->end.outidxs, outidxs, sizeof(unsigned) * outputs_count);

      array_insert(ctx->block, ctx->block->keeps, chmask);

      struct ir3_instruction *chsh = ir3_CHSH(ctx->block);
      chsh->barrier_class = IR3_BARRIER_EVERYTHING;
      chsh->barrier_conflict = IR3_BARRIER_EVERYTHING;
   } else {
      assert((ctx->noutputs % 4) == 0);
      unsigned outidxs[ctx->noutputs / 4];
      struct ir3_instruction *outputs[ctx->noutputs / 4];
      unsigned outputs_count = 0;

      struct ir3_block *b = ctx->block;
      /* Insert these collect's in the block before the end-block if
       * possible, so that any moves they generate can be shuffled around to
       * reduce nop's:
       */
      if (ctx->block->predecessors_count == 1)
         b = ctx->block->predecessors[0];

      /* Setup IR level outputs, which are "collects" that gather
       * the scalar components of outputs.
       */
      for (unsigned i = 0; i < ctx->noutputs; i += 4) {
         unsigned ncomp = 0;
         /* figure out the # of components written:
          *
          * TODO do we need to handle holes, ie. if .x and .z
          * components written, but .y component not written?
          */
         for (unsigned j = 0; j < 4; j++) {
            if (!ctx->outputs[i + j])
               break;
            ncomp++;
         }

         /* Note that in some stages, like TCS, store_output is
          * lowered to memory writes, so no components of the
          * are "written" from the PoV of traditional store-
          * output instructions:
          */
         if (!ncomp)
            continue;

         struct ir3_instruction *out =
            ir3_create_collect(b, &ctx->outputs[i], ncomp);

         int outidx = i / 4;
         assert(outidx < so->outputs_count);

         outidxs[outputs_count] = outidx;
         outputs[outputs_count] = out;
         outputs_count++;
      }

      /* for a6xx+, binning and draw pass VS use same VBO state, so we
       * need to make sure not to remove any inputs that are used by
       * the nonbinning VS.
       */
      if (ctx->compiler->gen >= 6 && so->binning_pass &&
          so->type == MESA_SHADER_VERTEX) {
         for (int i = 0; i < ctx->ninputs; i++) {
            struct ir3_instruction *in = ctx->inputs[i];

            if (!in)
               continue;

            unsigned n = i / 4;
            unsigned c = i % 4;

            assert(n < so->nonbinning->inputs_count);

            if (so->nonbinning->inputs[n].sysval)
               continue;

            /* be sure to keep inputs, even if only used in VS */
            if (so->nonbinning->inputs[n].compmask & (1 << c))
               array_insert(in->block, in->block->keeps, in);
         }
      }

      struct ir3_instruction *end =
         ir3_instr_create(ctx->block, OPC_END, 0, outputs_count);

      for (unsigned i = 0; i < outputs_count; i++) {
         __ssa_src(end, outputs[i], 0);
      }

      end->end.outidxs = ralloc_array(end, unsigned, outputs_count);
      memcpy(end->end.outidxs, outidxs, sizeof(unsigned) * outputs_count);

      array_insert(ctx->block, ctx->block->keeps, end);

      /* at this point, for binning pass, throw away unneeded outputs: */
      if (so->binning_pass && (ctx->compiler->gen < 6))
         fixup_binning_pass(ctx, end);
   }

   if (so->type == MESA_SHADER_FRAGMENT &&
       ctx->s->info.fs.needs_quad_helper_invocations) {
      so->need_pixlod = true;
      so->need_full_quad = true;
   }

   ir3_debug_print(ir, "AFTER: nir->ir3");
   ir3_validate(ir);

   IR3_PASS(ir, ir3_remove_unreachable);

   IR3_PASS(ir, ir3_array_to_ssa);

   do {
      progress = false;

      /* the folding doesn't seem to work reliably on a4xx */
      if (ctx->compiler->gen != 4)
         progress |= IR3_PASS(ir, ir3_cf);
      progress |= IR3_PASS(ir, ir3_cp, so);
      progress |= IR3_PASS(ir, ir3_cse);
      progress |= IR3_PASS(ir, ir3_dce, so);
   } while (progress);

   /* at this point, for binning pass, throw away unneeded outputs:
    * Note that for a6xx and later, we do this after ir3_cp to ensure
    * that the uniform/constant layout for BS and VS matches, so that
    * we can re-use same VS_CONST state group.
    */
   if (so->binning_pass && (ctx->compiler->gen >= 6)) {
      fixup_binning_pass(ctx, find_end(ctx->so->ir));
      /* cleanup the result of removing unneeded outputs: */
      while (IR3_PASS(ir, ir3_dce, so)) {
      }
   }

   IR3_PASS(ir, ir3_sched_add_deps);

   /* At this point, all the dead code should be long gone: */
   assert(!IR3_PASS(ir, ir3_dce, so));

   ret = ir3_sched(ir);
   if (ret) {
      DBG("SCHED failed!");
      goto out;
   }

   ir3_debug_print(ir, "AFTER: ir3_sched");

   /* Pre-assign VS inputs on a6xx+ binning pass shader, to align
    * with draw pass VS, so binning and draw pass can both use the
    * same VBO state.
    *
    * Note that VS inputs are expected to be full precision.
    */
   bool pre_assign_inputs = (ir->compiler->gen >= 6) &&
                            (ir->type == MESA_SHADER_VERTEX) &&
                            so->binning_pass;

   if (pre_assign_inputs) {
      foreach_input (in, ir) {
         assert(in->opc == OPC_META_INPUT);
         unsigned inidx = in->input.inidx;

         in->dsts[0]->num = so->nonbinning->inputs[inidx].regid;
      }
   } else if (ctx->tcs_header) {
      /* We need to have these values in the same registers between VS and TCS
       * since the VS chains to TCS and doesn't get the sysvals redelivered.
       */

      ctx->tcs_header->dsts[0]->num = regid(0, 0);
      ctx->rel_patch_id->dsts[0]->num = regid(0, 1);
      if (ctx->primitive_id)
         ctx->primitive_id->dsts[0]->num = regid(0, 2);
   } else if (ctx->gs_header) {
      /* We need to have these values in the same registers between producer
       * (VS or DS) and GS since the producer chains to GS and doesn't get
       * the sysvals redelivered.
       */

      ctx->gs_header->dsts[0]->num = regid(0, 0);
      if (ctx->primitive_id)
         ctx->primitive_id->dsts[0]->num = regid(0, 1);
   } else if (so->num_sampler_prefetch) {
      assert(so->type == MESA_SHADER_FRAGMENT);
      int idx = 0;

      foreach_input (instr, ir) {
         if (instr->input.sysval != SYSTEM_VALUE_BARYCENTRIC_PERSP_PIXEL)
            continue;

         assert(idx < 2);
         instr->dsts[0]->num = idx;
         idx++;
      }
   }

   ret = ir3_ra(so);

   if (ret) {
      mesa_loge("ir3_ra() failed!");
      goto out;
   }

   IR3_PASS(ir, ir3_postsched, so);

   IR3_PASS(ir, ir3_legalize_relative);
   IR3_PASS(ir, ir3_lower_subgroups);

   if (so->type == MESA_SHADER_FRAGMENT)
      pack_inlocs(ctx);

   /*
    * Fixup inputs/outputs to point to the actual registers assigned:
    *
    * 1) initialize to r63.x (invalid/unused)
    * 2) iterate IR level inputs/outputs and update the variants
    *    inputs/outputs table based on the assigned registers for
    *    the remaining inputs/outputs.
    */

   for (unsigned i = 0; i < so->inputs_count; i++)
      so->inputs[i].regid = INVALID_REG;
   for (unsigned i = 0; i < so->outputs_count; i++)
      so->outputs[i].regid = INVALID_REG;

   struct ir3_instruction *end = find_end(so->ir);

   for (unsigned i = 0; i < end->srcs_count; i++) {
      unsigned outidx = end->end.outidxs[i];
      struct ir3_register *reg = end->srcs[i];

      so->outputs[outidx].regid = reg->num;
      so->outputs[outidx].half = !!(reg->flags & IR3_REG_HALF);
   }

   foreach_input (in, ir) {
      assert(in->opc == OPC_META_INPUT);
      unsigned inidx = in->input.inidx;

      if (pre_assign_inputs && !so->inputs[inidx].sysval) {
         if (VALIDREG(so->nonbinning->inputs[inidx].regid)) {
            compile_assert(
               ctx, in->dsts[0]->num == so->nonbinning->inputs[inidx].regid);
            compile_assert(ctx, !!(in->dsts[0]->flags & IR3_REG_HALF) ==
                                   so->nonbinning->inputs[inidx].half);
         }
         so->inputs[inidx].regid = so->nonbinning->inputs[inidx].regid;
         so->inputs[inidx].half = so->nonbinning->inputs[inidx].half;
      } else {
         so->inputs[inidx].regid = in->dsts[0]->num;
         so->inputs[inidx].half = !!(in->dsts[0]->flags & IR3_REG_HALF);
      }
   }

   if (ctx->astc_srgb)
      fixup_astc_srgb(ctx);

   if (ctx->compiler->gen == 4 && ctx->s->info.uses_texture_gather)
      fixup_tg4(ctx);

   /* We need to do legalize after (for frag shader's) the "bary.f"
    * offsets (inloc) have been assigned.
    */
   IR3_PASS(ir, ir3_legalize, so, &max_bary);

   /* Set (ss)(sy) on first TCS and GEOMETRY instructions, since we don't
    * know what we might have to wait on when coming in from VS chsh.
    */
   if (so->type == MESA_SHADER_TESS_CTRL || so->type == MESA_SHADER_GEOMETRY) {
      foreach_block (block, &ir->block_list) {
         foreach_instr (instr, &block->instr_list) {
            instr->flags |= IR3_INSTR_SS | IR3_INSTR_SY;
            break;
         }
      }
   }

   if (ctx->compiler->gen >= 7 && so->type == MESA_SHADER_COMPUTE) {
      struct ir3_instruction *end = find_end(so->ir);
      struct ir3_instruction *lock =
         ir3_instr_create(ctx->block, OPC_LOCK, 0, 0);
      /* TODO: This flags should be set by scheduler only when needed */
      lock->flags = IR3_INSTR_SS | IR3_INSTR_SY | IR3_INSTR_JP;
      ir3_instr_move_before(lock, end);
      struct ir3_instruction *unlock =
         ir3_instr_create(ctx->block, OPC_UNLOCK, 0, 0);
      ir3_instr_move_before(unlock, end);
   }

   so->branchstack = ctx->max_stack;

   so->pvtmem_size = ALIGN(so->pvtmem_size, compiler->pvtmem_per_fiber_align);

   /* Note that max_bary counts inputs that are not bary.f'd for FS: */
   if (so->type == MESA_SHADER_FRAGMENT)
      so->total_in = max_bary + 1;

   /* Collect sampling instructions eligible for pre-dispatch. */
   collect_tex_prefetches(ctx, ir);

   if ((ctx->so->type == MESA_SHADER_FRAGMENT) &&
       !ctx->s->info.fs.early_fragment_tests)
      ctx->so->no_earlyz |= ctx->s->info.writes_memory;

   if ((ctx->so->type == MESA_SHADER_FRAGMENT) &&
       ctx->s->info.fs.post_depth_coverage)
      so->post_depth_coverage = true;

   ctx->so->per_samp = ctx->s->info.fs.uses_sample_shading;

out:
   if (ret) {
      if (so->ir)
         ir3_destroy(so->ir);
      so->ir = NULL;
   }
   ir3_context_free(ctx);

   return ret;
}
