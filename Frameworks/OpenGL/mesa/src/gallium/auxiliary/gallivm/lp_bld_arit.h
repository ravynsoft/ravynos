/**************************************************************************
 *
 * Copyright 2009 VMware, Inc.
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
 * Helper arithmetic functions.
 *
 * @author Jose Fonseca <jfonseca@vmware.com>
 */


#ifndef LP_BLD_ARIT_H
#define LP_BLD_ARIT_H


#include "gallivm/lp_bld.h"
#include "util/compiler.h"


struct lp_type;
struct lp_build_context;
struct gallivm_state;


/**
 * Complement, i.e., 1 - a.
 */
LLVMValueRef
lp_build_comp(struct lp_build_context *bld,
              LLVMValueRef a);

LLVMValueRef
lp_build_add(struct lp_build_context *bld,
             LLVMValueRef a,
             LLVMValueRef b);

LLVMValueRef
lp_build_horizontal_add(struct lp_build_context *bld,
                        LLVMValueRef a);

LLVMValueRef
lp_build_hadd_partial4(struct lp_build_context *bld,
                       LLVMValueRef vectors[],
                       unsigned num_vecs);

LLVMValueRef
lp_build_sub(struct lp_build_context *bld,
             LLVMValueRef a,
             LLVMValueRef b);


LLVMValueRef
lp_build_mul_norm(struct gallivm_state *gallivm,
                  struct lp_type wide_type,
                  LLVMValueRef a,
                  LLVMValueRef b);

LLVMValueRef
lp_build_mul(struct lp_build_context *bld,
             LLVMValueRef a,
             LLVMValueRef b);

LLVMValueRef
lp_build_mul_32_lohi_cpu(struct lp_build_context *bld,
                         LLVMValueRef a,
                         LLVMValueRef b,
                         LLVMValueRef *res_hi);

LLVMValueRef
lp_build_mul_32_lohi(struct lp_build_context *bld,
                     LLVMValueRef a,
                     LLVMValueRef b,
                     LLVMValueRef *res_hi);

LLVMValueRef
lp_build_mul_imm(struct lp_build_context *bld,
                 LLVMValueRef a,
                 int b);

LLVMValueRef
lp_build_div(struct lp_build_context *bld,
             LLVMValueRef a,
             LLVMValueRef b);


/* llvm.fmuladd.* intrinsic */
LLVMValueRef
lp_build_fmuladd(LLVMBuilderRef builder,
                 LLVMValueRef a,
                 LLVMValueRef b,
                 LLVMValueRef c);

/* a * b + c */
LLVMValueRef
lp_build_mad(struct lp_build_context *bld,
             LLVMValueRef a,
             LLVMValueRef b,
             LLVMValueRef c);


/**
 * Set when the weights for normalized are prescaled, that is, in range
 * 0..2**n, as opposed to range 0..2**(n-1).
 */
#define LP_BLD_LERP_PRESCALED_WEIGHTS (1 << 0)

/**
 * Used internally when using wide intermediates for normalized lerps.
 *
 * Do not use.
 */
#define LP_BLD_LERP_WIDE_NORMALIZED (1 << 1)

LLVMValueRef
lp_build_lerp(struct lp_build_context *bld,
              LLVMValueRef x,
              LLVMValueRef v0,
              LLVMValueRef v1,
              unsigned flags);

LLVMValueRef
lp_build_lerp_2d(struct lp_build_context *bld,
                 LLVMValueRef x,
                 LLVMValueRef y,
                 LLVMValueRef v00,
                 LLVMValueRef v01,
                 LLVMValueRef v10,
                 LLVMValueRef v11,
                 unsigned flags);

LLVMValueRef
lp_build_lerp_3d(struct lp_build_context *bld,
                 LLVMValueRef x,
                 LLVMValueRef y,
                 LLVMValueRef z,
                 LLVMValueRef v000,
                 LLVMValueRef v001,
                 LLVMValueRef v010,
                 LLVMValueRef v011,
                 LLVMValueRef v100,
                 LLVMValueRef v101,
                 LLVMValueRef v110,
                 LLVMValueRef v111,
                 unsigned flags);

/**
 * Specifies floating point NaN behavior.
 */
enum gallivm_nan_behavior {
   /* Results are undefined with NaN. Results in fastest code */
   GALLIVM_NAN_BEHAVIOR_UNDEFINED,
   /* If one of the inputs is NaN, the other operand is returned */
   GALLIVM_NAN_RETURN_OTHER,
   /* If one of the inputs is NaN, the other operand is returned,
    * but we guarantee the second operand is not a NaN.
    * In min/max it will be as fast as undefined with sse opcodes,
    * and archs having native return_other can benefit too. */
   GALLIVM_NAN_RETURN_OTHER_SECOND_NONNAN,
   /* If one of the inputs is NaN, NaN is returned,
    * but we guarantee the first operand is not a NaN.
    * In min/max it will be as fast as undefined with sse opcodes,
    * and archs having native return_nan can benefit too. */
   GALLIVM_NAN_RETURN_NAN_FIRST_NONNAN,

};

LLVMValueRef
lp_build_min(struct lp_build_context *bld,
             LLVMValueRef a,
             LLVMValueRef b);

LLVMValueRef
lp_build_min_ext(struct lp_build_context *bld,
                 LLVMValueRef a,
                 LLVMValueRef b,
                 enum gallivm_nan_behavior nan_behavior);

LLVMValueRef
lp_build_max(struct lp_build_context *bld,
             LLVMValueRef a,
             LLVMValueRef b);

LLVMValueRef
lp_build_max_ext(struct lp_build_context *bld,
                 LLVMValueRef a,
                 LLVMValueRef b,
                 enum gallivm_nan_behavior nan_behavior);

LLVMValueRef
lp_build_clamp(struct lp_build_context *bld,
               LLVMValueRef a,
               LLVMValueRef min,
               LLVMValueRef max);

LLVMValueRef
lp_build_clamp_zero_one_nanzero(struct lp_build_context *bld,
                                LLVMValueRef a);

LLVMValueRef
lp_build_abs(struct lp_build_context *bld,
             LLVMValueRef a);

LLVMValueRef
lp_build_negate(struct lp_build_context *bld,
                LLVMValueRef a);

LLVMValueRef
lp_build_sgn(struct lp_build_context *bld,
             LLVMValueRef a);

LLVMValueRef
lp_build_set_sign(struct lp_build_context *bld,
                  LLVMValueRef a, LLVMValueRef sign);

LLVMValueRef
lp_build_int_to_float(struct lp_build_context *bld,
                      LLVMValueRef a);

LLVMValueRef
lp_build_round(struct lp_build_context *bld,
               LLVMValueRef a);

LLVMValueRef
lp_build_floor(struct lp_build_context *bld,
               LLVMValueRef a);

LLVMValueRef
lp_build_ceil(struct lp_build_context *bld,
              LLVMValueRef a);

LLVMValueRef
lp_build_trunc(struct lp_build_context *bld,
               LLVMValueRef a);

LLVMValueRef
lp_build_fract(struct lp_build_context *bld,
               LLVMValueRef a);

LLVMValueRef
lp_build_fract_safe(struct lp_build_context *bld,
                    LLVMValueRef a);

LLVMValueRef
lp_build_ifloor(struct lp_build_context *bld,
                LLVMValueRef a);
LLVMValueRef
lp_build_iceil(struct lp_build_context *bld,
               LLVMValueRef a);

LLVMValueRef
lp_build_iround(struct lp_build_context *bld,
                LLVMValueRef a);

LLVMValueRef
lp_build_itrunc(struct lp_build_context *bld,
                LLVMValueRef a);

void
lp_build_ifloor_fract(struct lp_build_context *bld,
                      LLVMValueRef a,
                      LLVMValueRef *out_ipart,
                      LLVMValueRef *out_fpart);

void
lp_build_ifloor_fract_safe(struct lp_build_context *bld,
                           LLVMValueRef a,
                           LLVMValueRef *out_ipart,
                           LLVMValueRef *out_fpart);

LLVMValueRef
lp_build_sqrt(struct lp_build_context *bld,
              LLVMValueRef a);

LLVMValueRef
lp_build_rcp(struct lp_build_context *bld,
             LLVMValueRef a);

LLVMValueRef
lp_build_rsqrt(struct lp_build_context *bld,
               LLVMValueRef a);

bool
lp_build_fast_rsqrt_available(struct lp_type type);

LLVMValueRef
lp_build_fast_rsqrt(struct lp_build_context *bld,
                    LLVMValueRef a);

LLVMValueRef
lp_build_polynomial(struct lp_build_context *bld,
                    LLVMValueRef x,
                    const double *coeffs,
                    unsigned num_coeffs);

LLVMValueRef
lp_build_cos(struct lp_build_context *bld,
             LLVMValueRef a);

LLVMValueRef
lp_build_sin(struct lp_build_context *bld,
             LLVMValueRef a);

LLVMValueRef
lp_build_pow(struct lp_build_context *bld,
             LLVMValueRef a,
             LLVMValueRef b);

LLVMValueRef
lp_build_exp(struct lp_build_context *bld,
             LLVMValueRef a);

LLVMValueRef
lp_build_log(struct lp_build_context *bld,
             LLVMValueRef a);

LLVMValueRef
lp_build_log_safe(struct lp_build_context *bld,
                  LLVMValueRef a);

LLVMValueRef
lp_build_exp2(struct lp_build_context *bld,
              LLVMValueRef a);

LLVMValueRef
lp_build_extract_exponent(struct lp_build_context *bld,
                          LLVMValueRef x,
                          int bias);

LLVMValueRef
lp_build_extract_mantissa(struct lp_build_context *bld,
                          LLVMValueRef x);

LLVMValueRef
lp_build_log2(struct lp_build_context *bld,
              LLVMValueRef a);

LLVMValueRef
lp_build_log2_safe(struct lp_build_context *bld,
                   LLVMValueRef a);

LLVMValueRef
lp_build_fast_log2(struct lp_build_context *bld,
                   LLVMValueRef a);

LLVMValueRef
lp_build_ilog2(struct lp_build_context *bld,
               LLVMValueRef x);

void
lp_build_log2_approx(struct lp_build_context *bld,
                     LLVMValueRef x,
                     LLVMValueRef *p_exp,
                     LLVMValueRef *p_floor_log2,
                     LLVMValueRef *p_log2,
                     bool handle_nans);

LLVMValueRef
lp_build_mod(struct lp_build_context *bld,
             LLVMValueRef x,
             LLVMValueRef y);

LLVMValueRef
lp_build_isnan(struct lp_build_context *bld,
               LLVMValueRef x);

LLVMValueRef
lp_build_isfinite(struct lp_build_context *bld,
                  LLVMValueRef x);


LLVMValueRef
lp_build_is_inf_or_nan(struct gallivm_state *gallivm,
                       const struct lp_type type,
                       LLVMValueRef x);


LLVMValueRef
lp_build_fpstate_get(struct gallivm_state *gallivm);

void
lp_build_fpstate_set_denorms_zero(struct gallivm_state *gallivm,
                                  bool zero);
void
lp_build_fpstate_set(struct gallivm_state *gallivm,
                     LLVMValueRef mxcsr);

#endif /* !LP_BLD_ARIT_H */
