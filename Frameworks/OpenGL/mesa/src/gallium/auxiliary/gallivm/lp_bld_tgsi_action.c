/**************************************************************************
 * 
 * Copyright 2011-2012 Advanced Micro Devices, Inc.
 * Copyright 2009 VMware, Inc.
 * Copyright 2007-2008 VMware, Inc.
 * All Rights Reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 **************************************************************************/

/**
 * @file
 * TGSI to LLVM IR translation.
 *
 * @author Jose Fonseca <jfonseca@vmware.com>
 * @author Tom Stellard <thomas.stellard@amd.com>
 *
 * Based on tgsi_sse2.c code written by Michal Krol, Keith Whitwell,
 * Brian Paul, and others.
 */


#include "lp_bld_tgsi_action.h"

#include "lp_bld_tgsi.h"
#include "lp_bld_arit.h"
#include "lp_bld_bitarit.h"
#include "lp_bld_const.h"
#include "lp_bld_conv.h"
#include "lp_bld_gather.h"
#include "lp_bld_logic.h"
#include "lp_bld_pack.h"

/* XXX: The CPU only defaults should be repaced by generic ones.  In most
 * cases, the CPU defaults are just wrappers around a function in
 * lp_build_arit.c and these functions should be inlined here and the CPU
 * generic code should be removed and placed elsewhere.
 */

/* Default actions */

/* Generic fetch_arg functions */

static void scalar_unary_fetch_args(
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   /* src0.x */
   emit_data->args[0] = lp_build_emit_fetch(bld_base, emit_data->inst, 0, 0);
   emit_data->arg_count = 1;
   emit_data->dst_type = LLVMTypeOf(emit_data->args[0]);
}

static void scalar_binary_fetch_args(
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   /* src0.x */
   emit_data->args[0] = lp_build_emit_fetch(bld_base, emit_data->inst,
                                            0, TGSI_CHAN_X);
   /* src1.x */
   emit_data->args[1] = lp_build_emit_fetch(bld_base, emit_data->inst,
                                            1, TGSI_CHAN_X);
   emit_data->arg_count = 2;
   emit_data->dst_type = LLVMTypeOf(emit_data->args[0]);
}

/* TGSI_OPCODE_ADD */
static void
add_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] = LLVMBuildFAdd(
                                bld_base->base.gallivm->builder,
                                emit_data->args[0], emit_data->args[1], "");
}

/* TGSI_OPCODE_ARR */
static void
arr_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   LLVMValueRef tmp = lp_build_emit_llvm_unary(bld_base, TGSI_OPCODE_ROUND, emit_data->args[0]);
   emit_data->output[emit_data->chan] = LLVMBuildFPToSI(bld_base->base.gallivm->builder, tmp,
							bld_base->uint_bld.vec_type, "");
}

/* DP* Helper */

static void
dp_fetch_args(
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data,
   unsigned dp_components)
{
   unsigned chan, src;
   for (src = 0; src < 2; src++) {
      for (chan = 0; chan < dp_components; chan++) {
         emit_data->args[(src * dp_components) + chan] =
                     lp_build_emit_fetch(bld_base, emit_data->inst, src, chan);
      }
   }
   emit_data->dst_type = bld_base->base.elem_type;
}

/* TGSI_OPCODE_DP2 */
static void
dp2_fetch_args(
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   dp_fetch_args(bld_base, emit_data, 2);
}

static void
dp2_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   LLVMValueRef tmp0, tmp1;
   tmp0 = lp_build_emit_llvm_binary(bld_base, TGSI_OPCODE_MUL,
                                    emit_data->args[0] /* src0.x */,
                                    emit_data->args[2] /* src1.x */);
   tmp1 = lp_build_emit_llvm_binary(bld_base, TGSI_OPCODE_MUL,
                                    emit_data->args[1] /* src0.y */,
                                    emit_data->args[3] /* src1.y */);
   emit_data->output[emit_data->chan] = lp_build_emit_llvm_binary(bld_base,
                                                    TGSI_OPCODE_ADD, tmp0, tmp1);
}

static struct lp_build_tgsi_action dp2_action = {
   dp2_fetch_args,	 /* fetch_args */
   dp2_emit	 /* emit */
};

/* TGSI_OPCODE_DP3 */
static void
dp3_fetch_args(
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   dp_fetch_args(bld_base, emit_data, 3);
}

static void
dp3_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   LLVMValueRef tmp0, tmp1;
   tmp0 = lp_build_emit_llvm_binary(bld_base, TGSI_OPCODE_MUL,
                                    emit_data->args[0] /* src0.x */,
                                    emit_data->args[3] /* src1.x */);
   tmp1 = lp_build_emit_llvm_binary(bld_base, TGSI_OPCODE_MUL,
                                    emit_data->args[1] /* src0.y */,
                                    emit_data->args[4] /* src1.y */);
   tmp0 = lp_build_emit_llvm_binary(bld_base, TGSI_OPCODE_ADD, tmp1, tmp0);
   tmp1 = lp_build_emit_llvm_binary(bld_base, TGSI_OPCODE_MUL,
                                    emit_data->args[2] /* src0.z */,
                                    emit_data->args[5] /* src1.z */);
   emit_data->output[emit_data->chan] = lp_build_emit_llvm_binary(bld_base,
                                                    TGSI_OPCODE_ADD, tmp0, tmp1);
}

static struct lp_build_tgsi_action dp3_action = {
   dp3_fetch_args,	 /* fetch_args */
   dp3_emit	 /* emit */
};

/* TGSI_OPCODDE_DP4 */

static void
dp4_fetch_args(
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   dp_fetch_args(bld_base, emit_data, 4);
}

static void
dp4_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   LLVMValueRef tmp0, tmp1;
   tmp0 = lp_build_emit_llvm_binary(bld_base, TGSI_OPCODE_MUL,
                                    emit_data->args[0] /* src0.x */,
                                    emit_data->args[4] /* src1.x */);
   tmp1 = lp_build_emit_llvm_binary(bld_base, TGSI_OPCODE_MUL,
                                    emit_data->args[1] /* src0.y */,
                                    emit_data->args[5] /* src1.y */);
   tmp0 = lp_build_emit_llvm_binary(bld_base, TGSI_OPCODE_ADD, tmp0, tmp1);
   tmp1 = lp_build_emit_llvm_binary(bld_base, TGSI_OPCODE_MUL,
                                    emit_data->args[2] /* src0.z */,
                                    emit_data->args[6] /* src1.z */);
   tmp0 = lp_build_emit_llvm_binary(bld_base, TGSI_OPCODE_ADD, tmp0, tmp1);
   tmp1 = lp_build_emit_llvm_binary(bld_base, TGSI_OPCODE_MUL,
                                    emit_data->args[3] /* src0.w */,
                                    emit_data->args[7] /* src1.w */);
   emit_data->output[emit_data->chan] = lp_build_emit_llvm_binary(bld_base,
                                                    TGSI_OPCODE_ADD, tmp0, tmp1);
}

static struct lp_build_tgsi_action dp4_action = {
   dp4_fetch_args,	 /* fetch_args */
   dp4_emit	 /* emit */
};

/* TGSI_OPCODE_DST */
static void
dst_fetch_args(
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   /* src0.y */
   emit_data->args[0] = lp_build_emit_fetch(bld_base, emit_data->inst,
                                            0, TGSI_CHAN_Y);
   /* src0.z */
   emit_data->args[1] = lp_build_emit_fetch(bld_base, emit_data->inst,
                                            0, TGSI_CHAN_Z);
   /* src1.y */
   emit_data->args[2] = lp_build_emit_fetch(bld_base, emit_data->inst,
                                            1, TGSI_CHAN_Y);
   /* src1.w */
   emit_data->args[3] = lp_build_emit_fetch(bld_base, emit_data->inst,
                                            1, TGSI_CHAN_W);
}

static void
dst_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   /* dst.x */
   emit_data->output[TGSI_CHAN_X] = bld_base->base.one;

   /* dst.y */
   emit_data->output[TGSI_CHAN_Y] = lp_build_emit_llvm_binary(bld_base,
                                          TGSI_OPCODE_MUL,
                                          emit_data->args[0] /* src0.y */,
                                          emit_data->args[2] /* src1.y */);
   /* dst.z */
   emit_data->output[TGSI_CHAN_Z] = emit_data->args[1]; /* src0.z */

   /* dst.w */
   emit_data->output[TGSI_CHAN_W] = emit_data->args[3]; /* src1.w */
}

static struct lp_build_tgsi_action dst_action = {
   dst_fetch_args,	 /* fetch_args */
   dst_emit	 /* emit */
};

/* TGSI_OPCODE_END */
static void
end_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   bld_base->pc = -1;
}

/* TGSI_OPCODE_EXP */

static void
exp_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   LLVMValueRef floor_x;

   /* floor( src0.x ) */
   floor_x = lp_build_emit_llvm_unary(bld_base, TGSI_OPCODE_FLR,
                                      emit_data->args[0]);

   /* 2 ^ floor( src0.x ) */
   emit_data->output[TGSI_CHAN_X] = lp_build_emit_llvm_unary(bld_base,
                                       TGSI_OPCODE_EX2, floor_x);

   /* src0.x - floor( src0.x ) */
   emit_data->output[TGSI_CHAN_Y] =
      lp_build_sub(&bld_base->base, emit_data->args[0] /* src0.x */, floor_x);

   /* 2 ^ src0.x */
   emit_data->output[TGSI_CHAN_Z] = lp_build_emit_llvm_unary(bld_base,
                             TGSI_OPCODE_EX2, emit_data->args[0] /* src0.x */);

   emit_data->output[TGSI_CHAN_W] = bld_base->base.one;
}

const struct lp_build_tgsi_action exp_action = {
   scalar_unary_fetch_args,	 /* fetch_args */
   exp_emit	 /* emit */
};

/* TGSI_OPCODE_FRC */

static void
frc_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   LLVMValueRef tmp;
   tmp = lp_build_emit_llvm_unary(bld_base, TGSI_OPCODE_FLR,
                                  emit_data->args[0]);
   emit_data->output[emit_data->chan] =
      lp_build_sub(&bld_base->base, emit_data->args[0], tmp);
}

/* TGSI_OPCODE_KILL_IF */

static void
kil_fetch_args(
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   /* src0.x */
   emit_data->args[0] = lp_build_emit_fetch(bld_base, emit_data->inst,
                                            0, TGSI_CHAN_X);
   /* src0.y */
   emit_data->args[1] = lp_build_emit_fetch(bld_base, emit_data->inst,
                                            0, TGSI_CHAN_Y);
   /* src0.z */
   emit_data->args[2] = lp_build_emit_fetch(bld_base, emit_data->inst,
                                            0, TGSI_CHAN_Z);
   /* src0.w */
   emit_data->args[3] = lp_build_emit_fetch(bld_base, emit_data->inst,
                                            0, TGSI_CHAN_W);
   emit_data->arg_count = 4;
   emit_data->dst_type = LLVMVoidTypeInContext(bld_base->base.gallivm->context);
}

/* TGSI_OPCODE_KILL */

static void
kilp_fetch_args(
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->dst_type = LLVMVoidTypeInContext(bld_base->base.gallivm->context);
}

/* TGSI_OPCODE_LIT */

static void
lit_fetch_args(
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   /* src0.x */
   emit_data->args[0] = lp_build_emit_fetch(bld_base, emit_data->inst, 0, TGSI_CHAN_X);
   /* src0.y */
   emit_data->args[1] = lp_build_emit_fetch(bld_base, emit_data->inst, 0, TGSI_CHAN_Y);
   /* src0.w */
   emit_data->args[2] = lp_build_emit_fetch(bld_base, emit_data->inst, 0, TGSI_CHAN_W);
   emit_data->arg_count = 3;
}

static void
lit_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   LLVMValueRef tmp0, tmp1, tmp2;

   /* dst.x */
   emit_data->output[TGSI_CHAN_X] = bld_base->base.one;

   /* dst. y */
   emit_data->output[TGSI_CHAN_Y] = lp_build_emit_llvm_binary(bld_base,
                                               TGSI_OPCODE_MAX,
                                               emit_data->args[0] /* src0.x */,
                                               bld_base->base.zero);

   /* dst.z */
   /* XMM[1] = SrcReg[0].yyyy */
   tmp1 = emit_data->args[1];
   /* XMM[1] = max(XMM[1], 0) */
   tmp1 = lp_build_emit_llvm_binary(bld_base, TGSI_OPCODE_MAX,
                                    tmp1, bld_base->base.zero);
   /* XMM[2] = SrcReg[0].wwww */
   tmp2 = emit_data->args[2];
   tmp1 = lp_build_emit_llvm_binary(bld_base, TGSI_OPCODE_POW,
                                    tmp1, tmp2);
   tmp0 = emit_data->args[0];
   emit_data->output[TGSI_CHAN_Z] = lp_build_emit_llvm_ternary(bld_base,
                                             TGSI_OPCODE_CMP,
                                             tmp0, bld_base->base.zero, tmp1);
   /* dst.w */
   emit_data->output[TGSI_CHAN_W] = bld_base->base.one;
}

static struct lp_build_tgsi_action lit_action = {
   lit_fetch_args,	 /* fetch_args */
   lit_emit	 /* emit */
};

/* TGSI_OPCODE_LOG */

static void
log_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{

   LLVMValueRef abs_x, log_abs_x, flr_log_abs_x, ex2_flr_log_abs_x;

   /* abs( src0.x) */
   abs_x = lp_build_abs(&bld_base->base, emit_data->args[0] /* src0.x */);

   /* log( abs( src0.x ) ) */
   log_abs_x = lp_build_emit_llvm_unary(bld_base, TGSI_OPCODE_LG2,
                                        abs_x);

   /* floor( log( abs( src0.x ) ) ) */
   flr_log_abs_x = lp_build_emit_llvm_unary(bld_base, TGSI_OPCODE_FLR,
                                            log_abs_x);
   /* dst.x */
   emit_data->output[TGSI_CHAN_X] = flr_log_abs_x;

   /* dst.y */
   ex2_flr_log_abs_x = lp_build_emit_llvm_unary(bld_base, TGSI_OPCODE_EX2,
                                                flr_log_abs_x);

   /* abs( src0.x ) / 2^( floor( lg2( abs( src0.x ) ) ) ) */
   emit_data->output[TGSI_CHAN_Y] = lp_build_emit_llvm_binary(bld_base,
                                    TGSI_OPCODE_DIV, abs_x, ex2_flr_log_abs_x);

   /* dst.x */
   emit_data->output[TGSI_CHAN_Z] = log_abs_x;

   /* dst.w */
   emit_data->output[TGSI_CHAN_W] = bld_base->base.one;
}

static struct lp_build_tgsi_action log_action = {
   scalar_unary_fetch_args,	 /* fetch_args */
   log_emit	 /* emit */
};

/* TGSI_OPCODE_PK2H */

static void
pk2h_fetch_args(
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   /* src0.x */
   emit_data->args[0] = lp_build_emit_fetch(bld_base, emit_data->inst,
                                            0, TGSI_CHAN_X);
   /* src0.y */
   emit_data->args[1] = lp_build_emit_fetch(bld_base, emit_data->inst,
                                            0, TGSI_CHAN_Y);
}

static void
pk2h_emit(
   const struct lp_build_tgsi_action *action,
   struct lp_build_tgsi_context *bld_base,
   struct lp_build_emit_data *emit_data)
{
   struct gallivm_state *gallivm = bld_base->base.gallivm;
   struct lp_type f16i_t;
   LLVMValueRef lo, hi, res;

   f16i_t = lp_type_uint_vec(16, bld_base->base.type.length * 32);
   lo = lp_build_float_to_half(gallivm, emit_data->args[0]);
   hi = lp_build_float_to_half(gallivm, emit_data->args[1]);
   /* maybe some interleave doubling vector width would be useful... */
   lo = lp_build_pad_vector(gallivm, lo, bld_base->base.type.length * 2);
   hi = lp_build_pad_vector(gallivm, hi, bld_base->base.type.length * 2);
   res = lp_build_interleave2(gallivm, f16i_t, lo, hi, 0);

   emit_data->output[emit_data->chan] = res;
}

static struct lp_build_tgsi_action pk2h_action = {
   pk2h_fetch_args, /* fetch_args */
   pk2h_emit        /* emit */
};

/* TGSI_OPCODE_UP2H */

static void
up2h_emit(
   const struct lp_build_tgsi_action *action,
   struct lp_build_tgsi_context *bld_base,
   struct lp_build_emit_data *emit_data)
{
   struct gallivm_state *gallivm = bld_base->base.gallivm;
   LLVMBuilderRef builder = gallivm->builder;
   LLVMContextRef context = gallivm->context;
   LLVMValueRef lo, hi, res[2], arg;
   unsigned nr = bld_base->base.type.length;
   LLVMTypeRef i16t = LLVMVectorType(LLVMInt16TypeInContext(context), nr * 2);

   arg = LLVMBuildBitCast(builder, emit_data->args[0], i16t, "");
   lo = lp_build_uninterleave1(gallivm, nr * 2, arg, 0);
   hi = lp_build_uninterleave1(gallivm, nr * 2, arg, 1);
   res[0] = lp_build_half_to_float(gallivm, lo);
   res[1] = lp_build_half_to_float(gallivm, hi);

   emit_data->output[0] = emit_data->output[2] = res[0];
   emit_data->output[1] = emit_data->output[3] = res[1];
}

static struct lp_build_tgsi_action up2h_action = {
   scalar_unary_fetch_args, /* fetch_args */
   up2h_emit                /* emit */
};

/* TGSI_OPCODE_LRP */

static void
lrp_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_context *bld = &bld_base->base;
   LLVMValueRef inv, a, b;

   /* This uses the correct version: (1 - t)*a + t*b
    *
    * An alternative version is "a + t*(b-a)". The problem is this version
    * doesn't return "b" for t = 1, because "a + (b-a)" isn't equal to "b"
    * because of the floating-point rounding.
    */
   inv = lp_build_sub(bld, bld_base->base.one, emit_data->args[0]);
   a = lp_build_mul(bld, emit_data->args[1], emit_data->args[0]);
   b = lp_build_mul(bld, emit_data->args[2], inv);
   emit_data->output[emit_data->chan] = lp_build_add(bld, a, b);
}

/* TGSI_OPCODE_MAD */

static void
mad_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   LLVMValueRef tmp;
   tmp = lp_build_emit_llvm_binary(bld_base, TGSI_OPCODE_MUL,
                                   emit_data->args[0],
                                   emit_data->args[1]);
   emit_data->output[emit_data->chan] = lp_build_emit_llvm_binary(bld_base,
                                       TGSI_OPCODE_ADD, tmp, emit_data->args[2]);
}

/* TGSI_OPCODE_MOV */

static void
mov_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] = emit_data->args[0];
}

/* TGSI_OPCODE_MUL */
static void
mul_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] = LLVMBuildFMul(
                                   bld_base->base.gallivm->builder,
                                   emit_data->args[0], emit_data->args[1], "");
}

/*.TGSI_OPCODE_DIV.*/
static void fdiv_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] = LLVMBuildFDiv(
                                   bld_base->base.gallivm->builder,
                                   emit_data->args[0], emit_data->args[1], "");
}

/*.TGSI_OPCODE_RCP.*/
static void rcp_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   LLVMValueRef one;
   one = lp_build_const_float(bld_base->base.gallivm, 1.0f);
   emit_data->output[emit_data->chan] = lp_build_emit_llvm_binary(bld_base,
                                   TGSI_OPCODE_DIV, one, emit_data->args[0]);
}

/* TGSI_OPCODE_POW */

static void
pow_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] = lp_build_pow(&bld_base->base,
                                   emit_data->args[0], emit_data->args[1]);
}

static struct lp_build_tgsi_action pow_action = {
   scalar_binary_fetch_args,	 /* fetch_args */
   pow_emit	 /* emit */
};

/* TGSI_OPCODE_RSQ */

static void
rsq_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   if (bld_base->rsq_action.emit) {
      bld_base->rsq_action.emit(&bld_base->rsq_action, bld_base, emit_data);
   } else {
      emit_data->output[emit_data->chan] = bld_base->base.undef;
   }
}

const struct lp_build_tgsi_action rsq_action = {
   scalar_unary_fetch_args,	 /* fetch_args */
   rsq_emit	 /* emit */

};

/* TGSI_OPCODE_SQRT */

static void
sqrt_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   if (bld_base->sqrt_action.emit) {
      bld_base->sqrt_action.emit(&bld_base->sqrt_action, bld_base, emit_data);
   } else {
      emit_data->output[emit_data->chan] = bld_base->base.undef;
   }
}

const struct lp_build_tgsi_action sqrt_action = {
   scalar_unary_fetch_args,	 /* fetch_args */
   sqrt_emit	 /* emit */
};

/* TGSI_OPCODE_F2U */
static void
f2u_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] =
      LLVMBuildFPToUI(bld_base->base.gallivm->builder,
                      emit_data->args[0],
                      bld_base->base.int_vec_type, "");
}

/* TGSI_OPCODE_U2F */
static void
u2f_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] =
      LLVMBuildUIToFP(bld_base->base.gallivm->builder,
                      emit_data->args[0],
                      bld_base->base.vec_type, "");
}

static void
umad_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   LLVMValueRef tmp;
   tmp = lp_build_emit_llvm_binary(bld_base, TGSI_OPCODE_UMUL,
                                   emit_data->args[0],
                                   emit_data->args[1]);
   emit_data->output[emit_data->chan] = lp_build_emit_llvm_binary(bld_base,
                                       TGSI_OPCODE_UADD, tmp, emit_data->args[2]);
}

/* TGSI_OPCODE_UMUL */
static void
umul_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] = lp_build_mul(&bld_base->uint_bld,
                                   emit_data->args[0], emit_data->args[1]);
}

/* TGSI_OPCODE_IMUL_HI */
static void
imul_hi_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_context *int_bld = &bld_base->int_bld;
   LLVMValueRef hi_bits;

   assert(int_bld->type.width == 32);

   /* low result bits are tossed away */
   lp_build_mul_32_lohi(int_bld, emit_data->args[0],
                        emit_data->args[1], &hi_bits);
   emit_data->output[emit_data->chan] = hi_bits;
}

static void
imul_hi_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_context *int_bld = &bld_base->int_bld;
   LLVMValueRef hi_bits;

   assert(int_bld->type.width == 32);

   /* low result bits are tossed away */
   lp_build_mul_32_lohi_cpu(int_bld, emit_data->args[0],
                            emit_data->args[1], &hi_bits);
   emit_data->output[emit_data->chan] = hi_bits;
}

/* TGSI_OPCODE_UMUL_HI */
static void
umul_hi_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_context *uint_bld = &bld_base->uint_bld;
   LLVMValueRef hi_bits;

   assert(uint_bld->type.width == 32);

   /* low result bits are tossed away */
   lp_build_mul_32_lohi(uint_bld, emit_data->args[0],
                        emit_data->args[1], &hi_bits);
   emit_data->output[emit_data->chan] = hi_bits;
}

static void
umul_hi_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_context *uint_bld = &bld_base->uint_bld;
   LLVMValueRef hi_bits;

   assert(uint_bld->type.width == 32);

   /* low result bits are tossed away */
   lp_build_mul_32_lohi_cpu(uint_bld, emit_data->args[0],
                            emit_data->args[1], &hi_bits);
   emit_data->output[emit_data->chan] = hi_bits;
}

/* TGSI_OPCODE_MAX */
static void fmax_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   LLVMBuilderRef builder = bld_base->base.gallivm->builder;
   emit_data->output[emit_data->chan] = LLVMBuildSelect(builder,
                                   LLVMBuildFCmp(builder, LLVMRealUGE,
                                   emit_data->args[0], emit_data->args[1], ""),
                                   emit_data->args[0], emit_data->args[1], "");
}

/* TGSI_OPCODE_MIN */
static void fmin_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   LLVMBuilderRef builder = bld_base->base.gallivm->builder;
   emit_data->output[emit_data->chan] = LLVMBuildSelect(builder,
                                   LLVMBuildFCmp(builder, LLVMRealUGE,
                                   emit_data->args[0], emit_data->args[1], ""),
                                   emit_data->args[1], emit_data->args[0], "");
}

/* TGSI_OPCODE_D2F */
static void
d2f_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] =
      LLVMBuildFPTrunc(bld_base->base.gallivm->builder,
                      emit_data->args[0],
                       bld_base->base.vec_type, "");
}

/* TGSI_OPCODE_D2I */
static void
d2i_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] =
      LLVMBuildFPToSI(bld_base->base.gallivm->builder,
                      emit_data->args[0],
                      bld_base->base.int_vec_type, "");
}

/* TGSI_OPCODE_D2U */
static void
d2u_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] =
      LLVMBuildFPToUI(bld_base->base.gallivm->builder,
                      emit_data->args[0],
                      bld_base->base.int_vec_type, "");
}

/* TGSI_OPCODE_F2D */
static void
f2d_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] =
      LLVMBuildFPExt(bld_base->base.gallivm->builder,
                      emit_data->args[0],
                      bld_base->dbl_bld.vec_type, "");
}

/* TGSI_OPCODE_U2D */
static void
u2d_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] =
      LLVMBuildUIToFP(bld_base->base.gallivm->builder,
                      emit_data->args[0],
                      bld_base->dbl_bld.vec_type, "");
}

/* TGSI_OPCODE_I2D */
static void
i2d_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] =
      LLVMBuildSIToFP(bld_base->base.gallivm->builder,
                      emit_data->args[0],
                      bld_base->dbl_bld.vec_type, "");
}

/* TGSI_OPCODE_DMAD */
static void
dmad_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   LLVMValueRef tmp;
   tmp = lp_build_emit_llvm_binary(bld_base, TGSI_OPCODE_DMUL,
                                   emit_data->args[0],
                                   emit_data->args[1]);
   emit_data->output[emit_data->chan] = lp_build_emit_llvm_binary(bld_base,
                                       TGSI_OPCODE_DADD, tmp, emit_data->args[2]);
}

/*.TGSI_OPCODE_DRCP.*/
static void drcp_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   LLVMValueRef one;
   one = lp_build_const_vec(bld_base->dbl_bld.gallivm, bld_base->dbl_bld.type, 1.0f);
   emit_data->output[emit_data->chan] = LLVMBuildFDiv(
      bld_base->base.gallivm->builder,
      one, emit_data->args[0], "");
}

/* TGSI_OPCODE_DFRAC */
static void dfrac_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   LLVMValueRef tmp;
   tmp = lp_build_floor(&bld_base->dbl_bld,
			emit_data->args[0]);
   emit_data->output[emit_data->chan] =  LLVMBuildFSub(bld_base->base.gallivm->builder,
                                                       emit_data->args[0], tmp, "");
}

static void
u64mul_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] = lp_build_mul(&bld_base->uint64_bld,
                                   emit_data->args[0], emit_data->args[1]);
}

static void
u64mod_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   LLVMBuilderRef builder = bld_base->base.gallivm->builder;
   LLVMValueRef div_mask = lp_build_cmp(&bld_base->uint64_bld,
                                        PIPE_FUNC_EQUAL, emit_data->args[1],
                                        bld_base->uint64_bld.zero);
   /* We want to make sure that we never divide/mod by zero to not
    * generate sigfpe. We don't want to crash just because the
    * shader is doing something weird. */
   LLVMValueRef divisor = LLVMBuildOr(builder,
                                      div_mask,
                                      emit_data->args[1], "");
   LLVMValueRef result = lp_build_mod(&bld_base->uint64_bld,
                                      emit_data->args[0], divisor);
   /* umod by zero doesn't have a guaranteed return value chose -1 for now. */
   emit_data->output[emit_data->chan] = LLVMBuildOr(builder,
                                                    div_mask,
                                                    result, "");
}

static void
i64mod_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   LLVMBuilderRef builder = bld_base->base.gallivm->builder;
   LLVMValueRef div_mask = lp_build_cmp(&bld_base->uint64_bld,
                                        PIPE_FUNC_EQUAL, emit_data->args[1],
                                        bld_base->uint64_bld.zero);
   /* We want to make sure that we never divide/mod by zero to not
    * generate sigfpe. We don't want to crash just because the
    * shader is doing something weird. */
   LLVMValueRef divisor = LLVMBuildOr(builder,
                                      div_mask,
                                      emit_data->args[1], "");
   LLVMValueRef result = lp_build_mod(&bld_base->int64_bld,
                                      emit_data->args[0], divisor);
   /* umod by zero doesn't have a guaranteed return value chose -1 for now. */
   emit_data->output[emit_data->chan] = LLVMBuildOr(builder,
                                                    div_mask,
                                                    result, "");
}

static void
u64div_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{

   LLVMBuilderRef builder = bld_base->base.gallivm->builder;
   LLVMValueRef div_mask = lp_build_cmp(&bld_base->uint64_bld,
                                        PIPE_FUNC_EQUAL, emit_data->args[1],
                                        bld_base->uint64_bld.zero);
   /* We want to make sure that we never divide/mod by zero to not
    * generate sigfpe. We don't want to crash just because the
    * shader is doing something weird. */
   LLVMValueRef divisor = LLVMBuildOr(builder,
                                      div_mask,
                                      emit_data->args[1], "");
   LLVMValueRef result = LLVMBuildUDiv(builder,
				       emit_data->args[0], divisor, "");
   /* udiv by zero is guaranteed to return 0xffffffff at least with d3d10 */
   emit_data->output[emit_data->chan] = LLVMBuildOr(builder,
                                                    div_mask,
                                                    result, "");
}

static void
i64div_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{

   LLVMBuilderRef builder = bld_base->base.gallivm->builder;
   LLVMValueRef div_mask = lp_build_cmp(&bld_base->int64_bld,
                                        PIPE_FUNC_EQUAL, emit_data->args[1],
                                        bld_base->int64_bld.zero);
   /* We want to make sure that we never divide/mod by zero to not
    * generate sigfpe. We don't want to crash just because the
    * shader is doing something weird. */
   LLVMValueRef divisor = LLVMBuildOr(builder,
                                      div_mask,
                                      emit_data->args[1], "");
   LLVMValueRef result = LLVMBuildSDiv(builder,
				       emit_data->args[0], divisor, "");
   /* udiv by zero is guaranteed to return 0xffffffff at least with d3d10 */
   emit_data->output[emit_data->chan] = LLVMBuildOr(builder,
                                                    div_mask,
                                                    result, "");
}

static void
f2u64_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] =
      LLVMBuildFPToUI(bld_base->base.gallivm->builder,
                      emit_data->args[0],
                      bld_base->uint64_bld.vec_type, "");
}

static void
f2i64_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] =
      LLVMBuildFPToSI(bld_base->base.gallivm->builder,
                      emit_data->args[0],
                      bld_base->int64_bld.vec_type, "");
}

static void
u2i64_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] =
      LLVMBuildZExt(bld_base->base.gallivm->builder,
                      emit_data->args[0],
                      bld_base->uint64_bld.vec_type, "");
}

static void
i2i64_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] =
      LLVMBuildSExt(bld_base->base.gallivm->builder,
                      emit_data->args[0],
                      bld_base->int64_bld.vec_type, "");
}

static void
i642f_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] =
      LLVMBuildSIToFP(bld_base->base.gallivm->builder,
                      emit_data->args[0],
                      bld_base->base.vec_type, "");
}

static void
u642f_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] =
      LLVMBuildUIToFP(bld_base->base.gallivm->builder,
                      emit_data->args[0],
                      bld_base->base.vec_type, "");
}

static void
i642d_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] =
      LLVMBuildSIToFP(bld_base->base.gallivm->builder,
                      emit_data->args[0],
                      bld_base->dbl_bld.vec_type, "");
}

static void
u642d_emit(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] =
      LLVMBuildUIToFP(bld_base->base.gallivm->builder,
                      emit_data->args[0],
                      bld_base->dbl_bld.vec_type, "");
}

void
lp_set_default_actions(struct lp_build_tgsi_context * bld_base)
{
   bld_base->op_actions[TGSI_OPCODE_DP2] = dp2_action;
   bld_base->op_actions[TGSI_OPCODE_DP3] = dp3_action;
   bld_base->op_actions[TGSI_OPCODE_DP4] = dp4_action;
   bld_base->op_actions[TGSI_OPCODE_DST] = dst_action;
   bld_base->op_actions[TGSI_OPCODE_EXP] = exp_action;
   bld_base->op_actions[TGSI_OPCODE_LIT] = lit_action;
   bld_base->op_actions[TGSI_OPCODE_LOG] = log_action;
   bld_base->op_actions[TGSI_OPCODE_PK2H] = pk2h_action;
   bld_base->op_actions[TGSI_OPCODE_RSQ] = rsq_action;
   bld_base->op_actions[TGSI_OPCODE_SQRT] = sqrt_action;
   bld_base->op_actions[TGSI_OPCODE_POW] = pow_action;
   bld_base->op_actions[TGSI_OPCODE_UP2H] = up2h_action;

   bld_base->op_actions[TGSI_OPCODE_SWITCH].fetch_args = scalar_unary_fetch_args;
   bld_base->op_actions[TGSI_OPCODE_CASE].fetch_args = scalar_unary_fetch_args;
   bld_base->op_actions[TGSI_OPCODE_COS].fetch_args = scalar_unary_fetch_args;
   bld_base->op_actions[TGSI_OPCODE_EX2].fetch_args = scalar_unary_fetch_args;
   bld_base->op_actions[TGSI_OPCODE_IF].fetch_args = scalar_unary_fetch_args;
   bld_base->op_actions[TGSI_OPCODE_UIF].fetch_args = scalar_unary_fetch_args;
   bld_base->op_actions[TGSI_OPCODE_KILL_IF].fetch_args = kil_fetch_args;
   bld_base->op_actions[TGSI_OPCODE_KILL].fetch_args = kilp_fetch_args;
   bld_base->op_actions[TGSI_OPCODE_RCP].fetch_args = scalar_unary_fetch_args;
   bld_base->op_actions[TGSI_OPCODE_SIN].fetch_args = scalar_unary_fetch_args;
   bld_base->op_actions[TGSI_OPCODE_LG2].fetch_args = scalar_unary_fetch_args;

   bld_base->op_actions[TGSI_OPCODE_ADD].emit = add_emit;
   bld_base->op_actions[TGSI_OPCODE_ARR].emit = arr_emit;
   bld_base->op_actions[TGSI_OPCODE_END].emit = end_emit;
   bld_base->op_actions[TGSI_OPCODE_FRC].emit = frc_emit;
   bld_base->op_actions[TGSI_OPCODE_LRP].emit = lrp_emit;
   bld_base->op_actions[TGSI_OPCODE_MAD].emit = mad_emit;
   bld_base->op_actions[TGSI_OPCODE_MOV].emit = mov_emit;
   bld_base->op_actions[TGSI_OPCODE_MUL].emit = mul_emit;
   bld_base->op_actions[TGSI_OPCODE_DIV].emit = fdiv_emit;
   bld_base->op_actions[TGSI_OPCODE_RCP].emit = rcp_emit;

   bld_base->op_actions[TGSI_OPCODE_UARL].emit = mov_emit;
   bld_base->op_actions[TGSI_OPCODE_F2U].emit = f2u_emit;
   bld_base->op_actions[TGSI_OPCODE_U2F].emit = u2f_emit;
   bld_base->op_actions[TGSI_OPCODE_UMAD].emit = umad_emit;
   bld_base->op_actions[TGSI_OPCODE_UMUL].emit = umul_emit;
   bld_base->op_actions[TGSI_OPCODE_IMUL_HI].emit = imul_hi_emit;
   bld_base->op_actions[TGSI_OPCODE_UMUL_HI].emit = umul_hi_emit;

   bld_base->op_actions[TGSI_OPCODE_MAX].emit = fmax_emit;
   bld_base->op_actions[TGSI_OPCODE_MIN].emit = fmin_emit;

   bld_base->op_actions[TGSI_OPCODE_DADD].emit = add_emit;
   bld_base->op_actions[TGSI_OPCODE_DMAX].emit = fmax_emit;
   bld_base->op_actions[TGSI_OPCODE_DMIN].emit = fmin_emit;
   bld_base->op_actions[TGSI_OPCODE_DMUL].emit = mul_emit;
   bld_base->op_actions[TGSI_OPCODE_DDIV].emit = fdiv_emit;

   bld_base->op_actions[TGSI_OPCODE_D2F].emit = d2f_emit;
   bld_base->op_actions[TGSI_OPCODE_D2I].emit = d2i_emit;
   bld_base->op_actions[TGSI_OPCODE_D2U].emit = d2u_emit;

   bld_base->op_actions[TGSI_OPCODE_F2D].emit = f2d_emit;
   bld_base->op_actions[TGSI_OPCODE_I2D].emit = i2d_emit;
   bld_base->op_actions[TGSI_OPCODE_U2D].emit = u2d_emit;

   bld_base->op_actions[TGSI_OPCODE_DMAD].emit = dmad_emit;

   bld_base->op_actions[TGSI_OPCODE_DRCP].emit = drcp_emit;
   bld_base->op_actions[TGSI_OPCODE_DFRAC].emit = dfrac_emit;

   bld_base->op_actions[TGSI_OPCODE_U64MUL].emit = u64mul_emit;

   bld_base->op_actions[TGSI_OPCODE_F2I64].emit = f2i64_emit;
   bld_base->op_actions[TGSI_OPCODE_F2U64].emit = f2u64_emit;

   bld_base->op_actions[TGSI_OPCODE_D2I64].emit = f2i64_emit;
   bld_base->op_actions[TGSI_OPCODE_D2U64].emit = f2u64_emit;

   bld_base->op_actions[TGSI_OPCODE_I2I64].emit = i2i64_emit;
   bld_base->op_actions[TGSI_OPCODE_U2I64].emit = u2i64_emit;

   bld_base->op_actions[TGSI_OPCODE_I642F].emit = i642f_emit;
   bld_base->op_actions[TGSI_OPCODE_U642F].emit = u642f_emit;

   bld_base->op_actions[TGSI_OPCODE_I642F].emit = i642f_emit;
   bld_base->op_actions[TGSI_OPCODE_U642F].emit = u642f_emit;

   bld_base->op_actions[TGSI_OPCODE_I642D].emit = i642d_emit;
   bld_base->op_actions[TGSI_OPCODE_U642D].emit = u642d_emit;

}

/* CPU Only default actions */

/* These actions are CPU only, because they could potentially output SSE
 * intrinsics.
 */

/* TGSI_OPCODE_ADD (CPU Only) */
static void
add_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] = lp_build_add(&bld_base->base,
                                   emit_data->args[0], emit_data->args[1]);
}

/* TGSI_OPCODE_AND (CPU Only) */
static void
and_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] = lp_build_and(&bld_base->uint_bld,
                                   emit_data->args[0], emit_data->args[1]);
}

/* TGSI_OPCODE_ARL (CPU Only) */
static void
arl_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   LLVMValueRef tmp;
   tmp = lp_build_floor(&bld_base->base,
			emit_data->args[0]);
   emit_data->output[emit_data->chan] = LLVMBuildFPToSI(bld_base->base.gallivm->builder, tmp,
							bld_base->uint_bld.vec_type, "");
}

/* TGSI_OPCODE_ARR (CPU Only) */
static void
arr_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] = lp_build_iround(&bld_base->base, emit_data->args[0]);
}

/* TGSI_OPCODE_CEIL (CPU Only) */
static void
ceil_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] = lp_build_ceil(&bld_base->base,
                                                      emit_data->args[0]);
}

/* TGSI_OPCODE_CMP (CPU Only) */
static void
cmp_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   LLVMValueRef cond = lp_build_cmp(&bld_base->base, PIPE_FUNC_LESS,
                                   emit_data->args[0], bld_base->base.zero);
   emit_data->output[emit_data->chan] = lp_build_select(&bld_base->base,
                                cond, emit_data->args[1], emit_data->args[2]);
}

/* TGSI_OPCODE_UCMP (CPU Only) */
static void
ucmp_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   LLVMBuilderRef builder = bld_base->base.gallivm->builder;
   struct lp_build_context *uint_bld = &bld_base->uint_bld;
   LLVMValueRef unsigned_cond = 
      LLVMBuildBitCast(builder, emit_data->args[0], uint_bld->vec_type, "");
   LLVMValueRef cond = lp_build_cmp(uint_bld, PIPE_FUNC_NOTEQUAL,
                                    unsigned_cond,
                                    uint_bld->zero);
   emit_data->output[emit_data->chan] =
      lp_build_select(&bld_base->base,
                      cond, emit_data->args[1], emit_data->args[2]);
}

/* TGSI_OPCODE_COS (CPU Only) */
static void
cos_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] = lp_build_cos(&bld_base->base,
                                                       emit_data->args[0]);
}

/* TGSI_OPCODE_DIV (CPU Only) */
static void
div_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] = lp_build_div(&bld_base->base,
                                   emit_data->args[0], emit_data->args[1]);
}

/* TGSI_OPCODE_EX2 (CPU Only) */
static void
ex2_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] = lp_build_exp2(&bld_base->base,
                                                        emit_data->args[0]);
}

/* TGSI_OPCODE_F2I (CPU Only) */
static void
f2i_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] = lp_build_itrunc(&bld_base->base,
                                                        emit_data->args[0]);
}

/* TGSI_OPCODE_FSET Helper (CPU Only) */
static void
fset_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data,
   unsigned pipe_func)
{
   LLVMValueRef cond;

   if (pipe_func != PIPE_FUNC_NOTEQUAL) {
      cond = lp_build_cmp_ordered(&bld_base->base, pipe_func,
                                  emit_data->args[0], emit_data->args[1]);
   }
   else {
      cond = lp_build_cmp(&bld_base->base, pipe_func,
                          emit_data->args[0], emit_data->args[1]);

   }
   emit_data->output[emit_data->chan] = cond;
}


/* TGSI_OPCODE_FSEQ (CPU Only) */
static void
fseq_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   fset_emit_cpu(action, bld_base, emit_data, PIPE_FUNC_EQUAL);
}

/* TGSI_OPCODE_ISGE (CPU Only) */
static void
fsge_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   fset_emit_cpu(action, bld_base, emit_data, PIPE_FUNC_GEQUAL);
}

/* TGSI_OPCODE_ISLT (CPU Only) */
static void
fslt_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   fset_emit_cpu(action, bld_base, emit_data, PIPE_FUNC_LESS);
}

/* TGSI_OPCODE_USNE (CPU Only) */

static void
fsne_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   fset_emit_cpu(action, bld_base, emit_data, PIPE_FUNC_NOTEQUAL);
}

/* TGSI_OPCODE_FLR (CPU Only) */

static void
flr_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] = lp_build_floor(&bld_base->base,
                                                         emit_data->args[0]);
}

/* TGSI_OPCODE_I2F (CPU Only) */
static void
i2f_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] = lp_build_int_to_float(&bld_base->base,
                                                              emit_data->args[0]);
}

/* TGSI_OPCODE_IABS (CPU Only) */
static void
iabs_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] = lp_build_abs(&bld_base->int_bld,
                                                       emit_data->args[0]);
}

/* TGSI_OPCODE_IDIV (CPU Only) */
static void
idiv_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   LLVMBuilderRef builder = bld_base->base.gallivm->builder;
   LLVMValueRef div_mask = lp_build_cmp(&bld_base->uint_bld,
                                        PIPE_FUNC_EQUAL, emit_data->args[1],
                                        bld_base->uint_bld.zero);
   /* We want to make sure that we never divide/mod by zero to not
    * generate sigfpe. We don't want to crash just because the
    * shader is doing something weird. */
   LLVMValueRef divisor = LLVMBuildOr(builder,
                                      div_mask,
                                      emit_data->args[1], "");
   LLVMValueRef result = lp_build_div(&bld_base->int_bld,
                                      emit_data->args[0], divisor);
   LLVMValueRef not_div_mask = LLVMBuildNot(builder,
                                            div_mask,"");
   /* idiv by zero doesn't have a guaranteed return value chose 0 for now. */
   emit_data->output[emit_data->chan] = LLVMBuildAnd(builder,
                                                     not_div_mask,
                                                     result, "");
}

/* TGSI_OPCODE_INEG (CPU Only) */
static void
ineg_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] = lp_build_sub(&bld_base->int_bld,
                                                     bld_base->int_bld.zero,
                                                     emit_data->args[0]);
}

/* TGSI_OPCODE_ISET Helper (CPU Only) */
static void
iset_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data,
   unsigned pipe_func)
{
   LLVMValueRef cond = lp_build_cmp(&bld_base->int_bld, pipe_func,
                                    emit_data->args[0], emit_data->args[1]);
   emit_data->output[emit_data->chan] = cond;
}

/* TGSI_OPCODE_IMAX (CPU Only) */
static void
imax_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] = lp_build_max(&bld_base->int_bld,
                                   emit_data->args[0], emit_data->args[1]);
}

/* TGSI_OPCODE_IMIN (CPU Only) */
static void
imin_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] = lp_build_min(&bld_base->int_bld,
                                   emit_data->args[0], emit_data->args[1]);
}

/* TGSI_OPCODE_ISGE (CPU Only) */
static void
isge_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   iset_emit_cpu(action, bld_base, emit_data, PIPE_FUNC_GEQUAL);
}

/* TGSI_OPCODE_ISHR (CPU Only) */
static void
ishr_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_context *int_bld = &bld_base->int_bld;
   LLVMValueRef mask = lp_build_const_vec(int_bld->gallivm, int_bld->type,
                                          int_bld->type.width - 1);
   LLVMValueRef masked_count = lp_build_and(int_bld, emit_data->args[1], mask);
   emit_data->output[emit_data->chan] = lp_build_shr(int_bld, emit_data->args[0],
                                                     masked_count);
}

/* TGSI_OPCODE_ISLT (CPU Only) */
static void
islt_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   iset_emit_cpu(action, bld_base, emit_data, PIPE_FUNC_LESS);
}


/* TGSI_OPCODE_ISSG (CPU Only) */
static void
issg_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] = lp_build_sgn(&bld_base->int_bld,
                                                       emit_data->args[0]);
}

/* TGSI_OPCODE_LG2 (CPU Only) */
static void
lg2_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] = lp_build_log2_safe(&bld_base->base,
                                                           emit_data->args[0]);
}

/* TGSI_OPCODE_LOG (CPU Only) */
static void
log_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   LLVMValueRef p_floor_log2;
   LLVMValueRef p_exp;
   LLVMValueRef p_log2;
   LLVMValueRef src0 = emit_data->args[0];

   lp_build_log2_approx(&bld_base->base, src0,
                        &p_exp, &p_floor_log2, &p_log2, false);

   emit_data->output[TGSI_CHAN_X] = p_floor_log2;

   emit_data->output[TGSI_CHAN_Y] = lp_build_emit_llvm_binary(bld_base,
                                             TGSI_OPCODE_DIV,
                                             src0, p_exp);
   emit_data->output[TGSI_CHAN_Z] = p_log2;

   emit_data->output[TGSI_CHAN_W] = bld_base->base.one;

}

/* TGSI_OPCODE_MAD (CPU Only) */

static void
mad_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] =
      lp_build_mad(&bld_base->base,
                   emit_data->args[0], emit_data->args[1], emit_data->args[2]);
}

/* TGSI_OPCODE_MAX (CPU Only) */

static void
max_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] =
      lp_build_max_ext(&bld_base->base,
                       emit_data->args[0], emit_data->args[1],
                       GALLIVM_NAN_RETURN_OTHER);
}

/* TGSI_OPCODE_MIN (CPU Only) */
static void
min_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] =
      lp_build_min_ext(&bld_base->base,
                       emit_data->args[0], emit_data->args[1],
                       GALLIVM_NAN_RETURN_OTHER);
}

/* TGSI_OPCODE_MOD (CPU Only) */
static void
mod_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   LLVMBuilderRef builder = bld_base->base.gallivm->builder;
   LLVMValueRef div_mask = lp_build_cmp(&bld_base->uint_bld,
                                        PIPE_FUNC_EQUAL, emit_data->args[1],
                                        bld_base->uint_bld.zero);
   /* We want to make sure that we never divide/mod by zero to not
    * generate sigfpe. We don't want to crash just because the
    * shader is doing something weird. */
   LLVMValueRef divisor = LLVMBuildOr(builder,
                                      div_mask,
                                      emit_data->args[1], "");
   LLVMValueRef result = lp_build_mod(&bld_base->int_bld,
                                      emit_data->args[0], divisor);
   /* umod by zero doesn't have a guaranteed return value chose -1 for now. */
   emit_data->output[emit_data->chan] = LLVMBuildOr(builder,
                                                    div_mask,
                                                    result, "");
}

/* TGSI_OPCODE_NOT */
static void
not_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] = lp_build_not(&bld_base->uint_bld,
                                                     emit_data->args[0]);
}

/* TGSI_OPCODE_OR (CPU Only) */
static void
or_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] = lp_build_or(&bld_base->uint_bld,
                                   emit_data->args[0], emit_data->args[1]);
}

/* TGSI_OPCODE_POW (CPU Only) */
static void
pow_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] = lp_build_pow(&bld_base->base,
                                   emit_data->args[0], emit_data->args[1]);
}


/* TGSI_OPCODE_RCP (CPU Only) */

static void
rcp_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] = lp_build_rcp(&bld_base->base,
                                                       emit_data->args[0]);
}

/* Reciprical squareroot (CPU Only) */
static void
recip_sqrt_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] = lp_build_rsqrt(&bld_base->base,
                                                         emit_data->args[0]);
}

static void
sqrt_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] = lp_build_sqrt(&bld_base->base,
                                                      emit_data->args[0]);
}


/* TGSI_OPCODE_ROUND (CPU Only) */
static void
round_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] = lp_build_round(&bld_base->base,
                                                         emit_data->args[0]);
}

/* TGSI_OPCODE_SET Helper (CPU Only) */

static void
set_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data,
   unsigned pipe_func)
{
   LLVMValueRef cond;

   if (pipe_func != PIPE_FUNC_NOTEQUAL) {
      cond = lp_build_cmp_ordered(&bld_base->base, pipe_func,
                                  emit_data->args[0], emit_data->args[1]);
   }
   else {
      cond = lp_build_cmp(&bld_base->base, pipe_func,
                          emit_data->args[0], emit_data->args[1]);

   }
   emit_data->output[emit_data->chan] = lp_build_select(&bld_base->base,
                                          cond,
                                          bld_base->base.one,
                                          bld_base->base.zero);
}

/* TGSI_OPCODE_SEQ (CPU Only) */

static void
seq_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   set_emit_cpu(action, bld_base, emit_data, PIPE_FUNC_EQUAL);
}

/* TGSI_OPCODE_SGE (CPU Only) */
static void
sge_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   set_emit_cpu(action, bld_base, emit_data, PIPE_FUNC_GEQUAL);
}

/* TGSI_OPCODE_SGT (CPU Only)*/

static void
sgt_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   set_emit_cpu(action, bld_base, emit_data, PIPE_FUNC_GREATER);
}

/* TGSI_OPCODE_SHL (CPU Only) */
static void
shl_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_context *uint_bld = &bld_base->uint_bld;
   LLVMValueRef mask = lp_build_const_vec(uint_bld->gallivm, uint_bld->type,
                                          uint_bld->type.width - 1);
   LLVMValueRef masked_count = lp_build_and(uint_bld, emit_data->args[1], mask);
   emit_data->output[emit_data->chan] = lp_build_shl(uint_bld, emit_data->args[0],
                                                     masked_count);
}

/* TGSI_OPCODE_SIN (CPU Only) */
static void
sin_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] = lp_build_sin(&bld_base->base,
                                                       emit_data->args[0]);
}

/* TGSI_OPCODE_SLE (CPU Only) */
static void
sle_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   set_emit_cpu(action, bld_base, emit_data, PIPE_FUNC_LEQUAL);
}

/* TGSI_OPCODE_SLT (CPU Only) */
static void
slt_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   set_emit_cpu(action, bld_base, emit_data, PIPE_FUNC_LESS);
}

/* TGSI_OPCODE_SNE (CPU Only) */

static void
sne_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   set_emit_cpu(action, bld_base, emit_data, PIPE_FUNC_NOTEQUAL);
}

/* TGSI_OPCODE_SSG (CPU Only) */

static void
ssg_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] = lp_build_sgn(&bld_base->base,
                                                       emit_data->args[0]);
}

/* TGSI_OPCODE_TRUNC (CPU Only) */

static void
trunc_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] = lp_build_trunc(&bld_base->base,
                                                         emit_data->args[0]);
}

/* TGSI_OPCODE_UADD (CPU Only) */
static void
uadd_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] = lp_build_add(&bld_base->uint_bld,
                                   emit_data->args[0], emit_data->args[1]);
}

/* TGSI_OPCODE_UDIV (CPU Only) */
static void
udiv_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   
   LLVMBuilderRef builder = bld_base->base.gallivm->builder;
   LLVMValueRef div_mask = lp_build_cmp(&bld_base->uint_bld,
                                        PIPE_FUNC_EQUAL, emit_data->args[1],
                                        bld_base->uint_bld.zero);
   /* We want to make sure that we never divide/mod by zero to not
    * generate sigfpe. We don't want to crash just because the
    * shader is doing something weird. */
   LLVMValueRef divisor = LLVMBuildOr(builder,
                                      div_mask,
                                      emit_data->args[1], "");
   LLVMValueRef result = lp_build_div(&bld_base->uint_bld,
                                      emit_data->args[0], divisor);
   /* udiv by zero is guaranteed to return 0xffffffff at least with d3d10 */
   emit_data->output[emit_data->chan] = LLVMBuildOr(builder,
                                                    div_mask,
                                                    result, "");
}

/* TGSI_OPCODE_UMAX (CPU Only) */
static void
umax_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] = lp_build_max(&bld_base->uint_bld,
                                   emit_data->args[0], emit_data->args[1]);
}

/* TGSI_OPCODE_UMIN (CPU Only) */
static void
umin_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] = lp_build_min(&bld_base->uint_bld,
                                   emit_data->args[0], emit_data->args[1]);
}

/* TGSI_OPCODE_UMOD (CPU Only) */
static void
umod_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   LLVMBuilderRef builder = bld_base->base.gallivm->builder;
   LLVMValueRef div_mask = lp_build_cmp(&bld_base->uint_bld,
                                        PIPE_FUNC_EQUAL, emit_data->args[1],
                                        bld_base->uint_bld.zero);
   /* We want to make sure that we never divide/mod by zero to not 
    * generate sigfpe. We don't want to crash just because the 
    * shader is doing something weird. */
   LLVMValueRef divisor = LLVMBuildOr(builder,
                                      div_mask,
                                      emit_data->args[1], "");
   LLVMValueRef result = lp_build_mod(&bld_base->uint_bld,
                                      emit_data->args[0], divisor);
   /* umod by zero is guaranteed to return 0xffffffff */
   emit_data->output[emit_data->chan] = LLVMBuildOr(builder,
                                                    div_mask,
                                                    result, "");
}

/* TGSI_OPCODE_USET Helper (CPU Only) */
static void
uset_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data,
   unsigned pipe_func)
{
   LLVMValueRef cond = lp_build_cmp(&bld_base->uint_bld, pipe_func,
                                    emit_data->args[0], emit_data->args[1]);
   emit_data->output[emit_data->chan] = cond;
}


/* TGSI_OPCODE_USEQ (CPU Only) */
static void
useq_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   uset_emit_cpu(action, bld_base, emit_data, PIPE_FUNC_EQUAL);
}

/* TGSI_OPCODE_ISGE (CPU Only) */
static void
usge_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   uset_emit_cpu(action, bld_base, emit_data, PIPE_FUNC_GEQUAL);
}

/* TGSI_OPCODE_USHR (CPU Only) */
static void
ushr_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_context *uint_bld = &bld_base->uint_bld;
   LLVMValueRef mask = lp_build_const_vec(uint_bld->gallivm, uint_bld->type,
                                          uint_bld->type.width - 1);
   LLVMValueRef masked_count = lp_build_and(uint_bld, emit_data->args[1], mask);
   emit_data->output[emit_data->chan] = lp_build_shr(uint_bld, emit_data->args[0],
                                                     masked_count);
}

/* TGSI_OPCODE_ISLT (CPU Only) */
static void
uslt_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   uset_emit_cpu(action, bld_base, emit_data, PIPE_FUNC_LESS);
}

/* TGSI_OPCODE_USNE (CPU Only) */

static void
usne_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   uset_emit_cpu(action, bld_base, emit_data, PIPE_FUNC_NOTEQUAL);
}

/* TGSI_OPCODE_XOR */
static void
xor_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] = lp_build_xor(&bld_base->uint_bld,
                                                     emit_data->args[0],
                                                     emit_data->args[1]);
}

/* TGSI_OPCODE_DABS (CPU Only) */
static void
dabs_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] = lp_build_abs(&bld_base->dbl_bld,
                                                       emit_data->args[0]);
}

/* TGSI_OPCODE_DNEG (CPU Only) */
static void
dneg_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] = lp_build_sub(&bld_base->dbl_bld,
                                                     bld_base->dbl_bld.zero,
                                                     emit_data->args[0]);
}

/* TGSI_OPCODE_DSET Helper (CPU Only) */
static void
dset_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data,
   unsigned pipe_func)
{
   LLVMBuilderRef builder = bld_base->base.gallivm->builder;
   LLVMValueRef cond = lp_build_cmp(&bld_base->dbl_bld, pipe_func,
                                    emit_data->args[0], emit_data->args[1]);
   /* arguments were 64 bit but store as 32 bit */
   cond = LLVMBuildTrunc(builder, cond, bld_base->int_bld.int_vec_type, "");
   emit_data->output[emit_data->chan] = cond;
}

/* TGSI_OPCODE_DSEQ (CPU Only) */
static void
dseq_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   dset_emit_cpu(action, bld_base, emit_data, PIPE_FUNC_EQUAL);
}

/* TGSI_OPCODE_DSGE (CPU Only) */
static void
dsge_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   dset_emit_cpu(action, bld_base, emit_data, PIPE_FUNC_GEQUAL);
}

/* TGSI_OPCODE_DSLT (CPU Only) */
static void
dslt_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   dset_emit_cpu(action, bld_base, emit_data, PIPE_FUNC_LESS);
}

/* TGSI_OPCODE_DSNE (CPU Only) */
static void
dsne_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   dset_emit_cpu(action, bld_base, emit_data, PIPE_FUNC_NOTEQUAL);
}

/* Double Reciprocal squareroot (CPU Only) */
static void
drecip_sqrt_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] = lp_build_rsqrt(&bld_base->dbl_bld,
                                                         emit_data->args[0]);
}

/* Double Squareroot (CPU Only) */
static void
dsqrt_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] = lp_build_sqrt(&bld_base->dbl_bld,
                                                      emit_data->args[0]);
}

static void
i64abs_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] = lp_build_abs(&bld_base->int64_bld,
                                                       emit_data->args[0]);
}

static void
i64ssg_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] = lp_build_sgn(&bld_base->int64_bld,
                                                       emit_data->args[0]);
}

static void
i64neg_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] = lp_build_sub(&bld_base->int64_bld,
                                                     bld_base->int64_bld.zero,
                                                     emit_data->args[0]);
}

static void
u64set_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data,
   unsigned pipe_func)
{
   LLVMBuilderRef builder = bld_base->base.gallivm->builder;
   LLVMValueRef cond = lp_build_cmp(&bld_base->uint64_bld, pipe_func,
                                    emit_data->args[0], emit_data->args[1]);
   /* arguments were 64 bit but store as 32 bit */
   cond = LLVMBuildTrunc(builder, cond, bld_base->int_bld.int_vec_type, "");
   emit_data->output[emit_data->chan] = cond;
}

static void
u64seq_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   u64set_emit_cpu(action, bld_base, emit_data, PIPE_FUNC_EQUAL);
}

static void
u64sne_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   u64set_emit_cpu(action, bld_base, emit_data, PIPE_FUNC_NOTEQUAL);
}

static void
u64slt_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   u64set_emit_cpu(action, bld_base, emit_data, PIPE_FUNC_LESS);
}

static void
u64sge_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   u64set_emit_cpu(action, bld_base, emit_data, PIPE_FUNC_GEQUAL);
}

static void
i64set_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data,
   unsigned pipe_func)
{
   LLVMBuilderRef builder = bld_base->base.gallivm->builder;
   LLVMValueRef cond = lp_build_cmp(&bld_base->int64_bld, pipe_func,
                                    emit_data->args[0], emit_data->args[1]);
   /* arguments were 64 bit but store as 32 bit */
   cond = LLVMBuildTrunc(builder, cond, bld_base->int_bld.int_vec_type, "");
   emit_data->output[emit_data->chan] = cond;
}

static void
i64slt_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   i64set_emit_cpu(action, bld_base, emit_data, PIPE_FUNC_LESS);
}

static void
i64sge_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   i64set_emit_cpu(action, bld_base, emit_data, PIPE_FUNC_GEQUAL);
}

static void
u64max_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] = lp_build_max(&bld_base->uint64_bld,
                                   emit_data->args[0], emit_data->args[1]);
}

static void
u64min_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] = lp_build_min(&bld_base->uint64_bld,
                                   emit_data->args[0], emit_data->args[1]);
}

static void
i64max_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] = lp_build_max(&bld_base->int64_bld,
                                   emit_data->args[0], emit_data->args[1]);
}

static void
i64min_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] = lp_build_min(&bld_base->int64_bld,
                                   emit_data->args[0], emit_data->args[1]);
}

static void
u64add_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   emit_data->output[emit_data->chan] = lp_build_add(&bld_base->uint64_bld,
                                   emit_data->args[0], emit_data->args[1]);
}

static void
u64shl_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_context *uint_bld = &bld_base->uint64_bld;
   LLVMValueRef mask = lp_build_const_vec(uint_bld->gallivm, uint_bld->type,
                                          uint_bld->type.width - 1);
   LLVMValueRef masked_count = lp_build_and(uint_bld, emit_data->args[1], mask);
   emit_data->output[emit_data->chan] = lp_build_shl(uint_bld, emit_data->args[0],
                                                     masked_count);
}

static void
i64shr_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_context *int_bld = &bld_base->int64_bld;
   LLVMValueRef mask = lp_build_const_vec(int_bld->gallivm, int_bld->type,
                                          int_bld->type.width - 1);
   LLVMValueRef masked_count = lp_build_and(int_bld, emit_data->args[1], mask);
   emit_data->output[emit_data->chan] = lp_build_shr(int_bld, emit_data->args[0],
                                                     masked_count);
}

static void
u64shr_emit_cpu(
   const struct lp_build_tgsi_action * action,
   struct lp_build_tgsi_context * bld_base,
   struct lp_build_emit_data * emit_data)
{
   struct lp_build_context *uint_bld = &bld_base->uint64_bld;
   LLVMValueRef mask = lp_build_const_vec(uint_bld->gallivm, uint_bld->type,
                                          uint_bld->type.width - 1);
   LLVMValueRef masked_count = lp_build_and(uint_bld, emit_data->args[1], mask);
   emit_data->output[emit_data->chan] = lp_build_shr(uint_bld, emit_data->args[0],
                                                     masked_count);
}
static void bfi_emit_cpu(const struct lp_build_tgsi_action *action,
                         struct lp_build_tgsi_context *bld_base,
                         struct lp_build_emit_data *emit_data) {
  /*
   * def bfi(base, insert, offset, bits):
   *   if offset < 0 or bits < 0 or offset + bits > 32:
   *     return undefined
   *   # << defined such that mask == ~0 when bits == 32, offset == 0
   *   mask = ((1 << bits) - 1) << offset
   *   return ((insert << offset) & mask) | (base & ~mask)
   */
  struct lp_build_context *uint_bld = &bld_base->uint_bld;
  LLVMValueRef one_shl_bits_dec_one = lp_build_sub(
      uint_bld, lp_build_shl(uint_bld, uint_bld->one, emit_data->args[3]),
      uint_bld->one);
  LLVMValueRef mask =
      lp_build_shl(uint_bld, one_shl_bits_dec_one, emit_data->args[2]);
  LLVMValueRef insert_shl_offset =
      lp_build_shl(uint_bld, emit_data->args[1], emit_data->args[2]);
  LLVMValueRef insert_shl_offset_and_mask =
      lp_build_and(uint_bld, insert_shl_offset, mask);
  LLVMValueRef base_and_not_mask =
      lp_build_and(uint_bld, emit_data->args[0], lp_build_not(uint_bld, mask));

  emit_data->output[emit_data->chan] =
      lp_build_or(uint_bld, insert_shl_offset_and_mask, base_and_not_mask);
}

static void lsb_emit_cpu(const struct lp_build_tgsi_action *action,
                         struct lp_build_tgsi_context *bld_base,
                         struct lp_build_emit_data *emit_data) {
  struct lp_build_context *uint_bld = &bld_base->int_bld;

  LLVMValueRef result = lp_build_cttz(uint_bld, emit_data->args[0]);
  LLVMValueRef cond =
      lp_build_cmp(uint_bld, PIPE_FUNC_LESS, result,
                   lp_build_const_vec(uint_bld->gallivm, uint_bld->type, 32));
  emit_data->output[emit_data->chan] = lp_build_select(
      uint_bld, cond, result,
      lp_build_const_vec(uint_bld->gallivm, uint_bld->type, -1));
}

static void umsb_emit_cpu(const struct lp_build_tgsi_action *action,
                          struct lp_build_tgsi_context *bld_base,
                          struct lp_build_emit_data *emit_data) {
  struct lp_build_context *uint_bld = &bld_base->int_bld;
  emit_data->output[emit_data->chan] = lp_build_sub(
      uint_bld, lp_build_const_vec(uint_bld->gallivm, uint_bld->type, 31),
      lp_build_ctlz(uint_bld, emit_data->args[0]));
}

static void imsb_emit_cpu(const struct lp_build_tgsi_action *action,
                          struct lp_build_tgsi_context *bld_base,
                          struct lp_build_emit_data *emit_data) {
  struct lp_build_context *uint_bld = &bld_base->int_bld;

  LLVMValueRef cond =
      lp_build_cmp(uint_bld, PIPE_FUNC_LESS, emit_data->args[0],
                   lp_build_const_vec(uint_bld->gallivm, uint_bld->type, 0));
  emit_data->args[0] = lp_build_select(
      uint_bld, cond, lp_build_not(uint_bld, emit_data->args[0]),
      emit_data->args[0]);
  umsb_emit_cpu(action, bld_base, emit_data);
}

static void popc_emit_cpu(const struct lp_build_tgsi_action *action,
                          struct lp_build_tgsi_context *bld_base,
                          struct lp_build_emit_data *emit_data) {
  struct lp_build_context *uint_bld = &bld_base->int_bld;
  emit_data->output[emit_data->chan] =
      lp_build_popcount(uint_bld, emit_data->args[0]);
}

static void ibfe_emit_cpu(const struct lp_build_tgsi_action *action,
                          struct lp_build_tgsi_context *bld_base,
                          struct lp_build_emit_data *emit_data) {
  /* def ibfe(value, offset, bits):
   *   if offset < 0 or bits < 0 or offset + bits > 32:
   *     return undefined
   *   if bits == 0: return 0
   *   # Note: >> sign-extends
   *   return (value << (32 - offset - bits)) >> (32 - bits)
   */
  struct lp_build_context *uint_bld = &bld_base->int_bld;

  LLVMValueRef r_32_sub_bits = lp_build_sub(
      uint_bld, lp_build_const_vec(uint_bld->gallivm, uint_bld->type, 32),
      emit_data->args[2]);
  LLVMValueRef temp1 =
      lp_build_sub(uint_bld, r_32_sub_bits, emit_data->args[1]);
  LLVMValueRef temp2 = lp_build_shl(uint_bld, emit_data->args[0], temp1);
  LLVMValueRef cond =
      lp_build_cmp(uint_bld, PIPE_FUNC_EQUAL, emit_data->args[2],
                   lp_build_const_vec(uint_bld->gallivm, uint_bld->type, 0));
  emit_data->output[emit_data->chan] = lp_build_select(
      uint_bld, cond, lp_build_const_vec(uint_bld->gallivm, uint_bld->type, 0),
      lp_build_shr(uint_bld, temp2, r_32_sub_bits));
}

static void ubfe_emit_cpu(const struct lp_build_tgsi_action *action,
                          struct lp_build_tgsi_context *bld_base,
                          struct lp_build_emit_data *emit_data) {
  /* def ubfe(value, offset, bits):
   *   if offset < 0 or bits < 0 or offset + bits > 32:
   *     return undefined
   *   if bits == 0: return 0
   *   # Note: >> does not sign-extend
   *   return (value << (32 - offset - bits)) >> (32 - bits)
   */
  struct lp_build_context *uint_bld = &bld_base->uint_bld;

  LLVMValueRef r_32_sub_bits = lp_build_sub(
      uint_bld, lp_build_const_vec(uint_bld->gallivm, uint_bld->type, 32),
      emit_data->args[2]);
  LLVMValueRef temp1 =
      lp_build_sub(uint_bld, r_32_sub_bits, emit_data->args[1]);
  LLVMValueRef temp2 = lp_build_shl(uint_bld, emit_data->args[0], temp1);
  emit_data->output[emit_data->chan] =
      lp_build_shr(uint_bld, temp2, r_32_sub_bits);
}

static void brev_emit_cpu(const struct lp_build_tgsi_action *action,
                          struct lp_build_tgsi_context *bld_base,
                          struct lp_build_emit_data *emit_data) {
  struct lp_build_context *uint_bld = &bld_base->uint_bld;
  emit_data->output[emit_data->chan] =
      lp_build_bitfield_reverse(uint_bld, emit_data->args[0]);
}

void
lp_set_default_actions_cpu(
   struct lp_build_tgsi_context * bld_base)
{
   lp_set_default_actions(bld_base);
   bld_base->op_actions[TGSI_OPCODE_ADD].emit = add_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_AND].emit = and_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_ARL].emit = arl_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_ARR].emit = arr_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_CEIL].emit = ceil_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_COS].emit = cos_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_CMP].emit = cmp_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_DIV].emit = div_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_EX2].emit = ex2_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_F2I].emit = f2i_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_FLR].emit = flr_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_FSEQ].emit = fseq_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_FSGE].emit = fsge_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_FSLT].emit = fslt_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_FSNE].emit = fsne_emit_cpu;

   bld_base->op_actions[TGSI_OPCODE_I2F].emit = i2f_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_IABS].emit = iabs_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_IDIV].emit = idiv_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_INEG].emit = ineg_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_IMAX].emit = imax_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_IMIN].emit = imin_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_ISGE].emit = isge_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_ISHR].emit = ishr_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_ISLT].emit = islt_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_ISSG].emit = issg_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_IMUL_HI].emit = imul_hi_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_UMUL_HI].emit = umul_hi_emit_cpu;

   bld_base->op_actions[TGSI_OPCODE_LG2].emit = lg2_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_LOG].emit = log_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_MAD].emit = mad_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_MAX].emit = max_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_MIN].emit = min_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_MOD].emit = mod_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_NOT].emit = not_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_OR].emit = or_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_POW].emit = pow_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_RCP].emit = rcp_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_ROUND].emit = round_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_SEQ].emit = seq_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_SGE].emit = sge_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_SGT].emit = sgt_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_SIN].emit = sin_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_SHL].emit = shl_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_SLE].emit = sle_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_SLT].emit = slt_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_SNE].emit = sne_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_SSG].emit = ssg_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_TRUNC].emit = trunc_emit_cpu;

   bld_base->rsq_action.emit = recip_sqrt_emit_cpu;
   bld_base->sqrt_action.emit = sqrt_emit_cpu;

   bld_base->op_actions[TGSI_OPCODE_UADD].emit = uadd_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_UCMP].emit = ucmp_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_UDIV].emit = udiv_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_UMAX].emit = umax_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_UMIN].emit = umin_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_UMOD].emit = umod_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_USEQ].emit = useq_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_USGE].emit = usge_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_USHR].emit = ushr_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_USLT].emit = uslt_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_USNE].emit = usne_emit_cpu;

   bld_base->op_actions[TGSI_OPCODE_XOR].emit = xor_emit_cpu;

   bld_base->op_actions[TGSI_OPCODE_DABS].emit = dabs_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_DNEG].emit = dneg_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_DSEQ].emit = dseq_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_DSGE].emit = dsge_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_DSLT].emit = dslt_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_DSNE].emit = dsne_emit_cpu;

   bld_base->op_actions[TGSI_OPCODE_DRSQ].emit = drecip_sqrt_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_DSQRT].emit = dsqrt_emit_cpu;

   bld_base->op_actions[TGSI_OPCODE_I64ABS].emit = i64abs_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_I64SSG].emit = i64ssg_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_I64NEG].emit = i64neg_emit_cpu;

   bld_base->op_actions[TGSI_OPCODE_U64SEQ].emit = u64seq_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_U64SNE].emit = u64sne_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_U64SLT].emit = u64slt_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_U64SGE].emit = u64sge_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_I64SLT].emit = i64slt_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_I64SGE].emit = i64sge_emit_cpu;

   bld_base->op_actions[TGSI_OPCODE_U64MIN].emit = u64min_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_U64MAX].emit = u64max_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_I64MIN].emit = i64min_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_I64MAX].emit = i64max_emit_cpu;

   bld_base->op_actions[TGSI_OPCODE_U64ADD].emit = u64add_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_U64MOD].emit = u64mod_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_I64MOD].emit = i64mod_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_U64DIV].emit = u64div_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_I64DIV].emit = i64div_emit_cpu;

   bld_base->op_actions[TGSI_OPCODE_U64SHL].emit = u64shl_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_I64SHR].emit = i64shr_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_U64SHR].emit = u64shr_emit_cpu;

   bld_base->op_actions[TGSI_OPCODE_BFI].emit = bfi_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_POPC].emit = popc_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_LSB].emit = lsb_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_IMSB].emit = imsb_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_UMSB].emit = umsb_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_IBFE].emit = ibfe_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_UBFE].emit = ubfe_emit_cpu;
   bld_base->op_actions[TGSI_OPCODE_BREV].emit = brev_emit_cpu;

}
