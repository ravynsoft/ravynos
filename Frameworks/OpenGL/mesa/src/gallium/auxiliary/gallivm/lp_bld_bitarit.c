/**************************************************************************
 *
 * Copyright 2010 VMware, Inc.
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS, AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 **************************************************************************/


#include "util/u_debug.h"

#include "lp_bld_type.h"
#include "lp_bld_debug.h"
#include "lp_bld_const.h"
#include "lp_bld_bitarit.h"
#include "lp_bld_intr.h"

/**
 * Return (a | b)
 */
LLVMValueRef
lp_build_or(struct lp_build_context *bld, LLVMValueRef a, LLVMValueRef b)
{
   LLVMBuilderRef builder = bld->gallivm->builder;
   const struct lp_type type = bld->type;
   LLVMValueRef res;

   assert(lp_check_value(type, a));
   assert(lp_check_value(type, b));

   /* can't do bitwise ops on floating-point values */
   if (type.floating) {
      a = LLVMBuildBitCast(builder, a, bld->int_vec_type, "");
      b = LLVMBuildBitCast(builder, b, bld->int_vec_type, "");
   }

   res = LLVMBuildOr(builder, a, b, "");

   if (type.floating) {
      res = LLVMBuildBitCast(builder, res, bld->vec_type, "");
   }

   return res;
}

/* bitwise XOR (a ^ b) */
LLVMValueRef
lp_build_xor(struct lp_build_context *bld, LLVMValueRef a, LLVMValueRef b)
{
   LLVMBuilderRef builder = bld->gallivm->builder;
   const struct lp_type type = bld->type;
   LLVMValueRef res;

   assert(lp_check_value(type, a));
   assert(lp_check_value(type, b));

   /* can't do bitwise ops on floating-point values */
   if (type.floating) {
      a = LLVMBuildBitCast(builder, a, bld->int_vec_type, "");
      b = LLVMBuildBitCast(builder, b, bld->int_vec_type, "");
   }

   res = LLVMBuildXor(builder, a, b, "");

   if (type.floating) {
      res = LLVMBuildBitCast(builder, res, bld->vec_type, "");
   }

   return res;
}

/**
 * Return (a & b)
 */
LLVMValueRef
lp_build_and(struct lp_build_context *bld, LLVMValueRef a, LLVMValueRef b)
{
   LLVMBuilderRef builder = bld->gallivm->builder;
   const struct lp_type type = bld->type;
   LLVMValueRef res;

   assert(lp_check_value(type, a));
   assert(lp_check_value(type, b));

   /* can't do bitwise ops on floating-point values */
   if (type.floating) {
      a = LLVMBuildBitCast(builder, a, bld->int_vec_type, "");
      b = LLVMBuildBitCast(builder, b, bld->int_vec_type, "");
   }

   res = LLVMBuildAnd(builder, a, b, "");

   if (type.floating) {
      res = LLVMBuildBitCast(builder, res, bld->vec_type, "");
   }

   return res;
}


/**
 * Return (a & ~b)
 */
LLVMValueRef
lp_build_andnot(struct lp_build_context *bld, LLVMValueRef a, LLVMValueRef b)
{
   LLVMBuilderRef builder = bld->gallivm->builder;
   const struct lp_type type = bld->type;
   LLVMValueRef res;

   assert(lp_check_value(type, a));
   assert(lp_check_value(type, b));

   /* can't do bitwise ops on floating-point values */
   if (type.floating) {
      a = LLVMBuildBitCast(builder, a, bld->int_vec_type, "");
      b = LLVMBuildBitCast(builder, b, bld->int_vec_type, "");
   }

   res = LLVMBuildNot(builder, b, "");
   res = LLVMBuildAnd(builder, a, res, "");

   if (type.floating) {
      res = LLVMBuildBitCast(builder, res, bld->vec_type, "");
   }

   return res;
}

/* bitwise NOT */
LLVMValueRef
lp_build_not(struct lp_build_context *bld, LLVMValueRef a)
{
   LLVMBuilderRef builder = bld->gallivm->builder;
   const struct lp_type type = bld->type;
   LLVMValueRef res;

   assert(lp_check_value(type, a));

   if (type.floating) {
      a = LLVMBuildBitCast(builder, a, bld->int_vec_type, "");
   }
   res = LLVMBuildNot(builder, a, "");
   if (type.floating) {
      res = LLVMBuildBitCast(builder, res, bld->vec_type, "");
   }
   return res;
}

/**
 * Shift left.
 * Result is undefined if the shift count is not smaller than the type width.
 */
LLVMValueRef
lp_build_shl(struct lp_build_context *bld, LLVMValueRef a, LLVMValueRef b)
{
   LLVMBuilderRef builder = bld->gallivm->builder;
   const struct lp_type type = bld->type;
   LLVMValueRef res;

   assert(!type.floating);

   assert(lp_check_value(type, a));
   assert(lp_check_value(type, b));

   (void)type;

   res = LLVMBuildShl(builder, a, b, "");

   return res;
}


/**
 * Shift right.
 * Result is undefined if the shift count is not smaller than the type width.
 */
LLVMValueRef
lp_build_shr(struct lp_build_context *bld, LLVMValueRef a, LLVMValueRef b)
{
   LLVMBuilderRef builder = bld->gallivm->builder;
   const struct lp_type type = bld->type;
   LLVMValueRef res;

   assert(!type.floating);

   assert(lp_check_value(type, a));
   assert(lp_check_value(type, b));

   if (type.sign) {
      res = LLVMBuildAShr(builder, a, b, "");
   } else {
      res = LLVMBuildLShr(builder, a, b, "");
   }

   return res;
}


/**
 * Shift left with immediate.
 * The immediate shift count must be smaller than the type width.
 */
LLVMValueRef
lp_build_shl_imm(struct lp_build_context *bld, LLVMValueRef a, unsigned imm)
{
   LLVMValueRef b = lp_build_const_int_vec(bld->gallivm, bld->type, imm);
   assert(imm < bld->type.width);
   return lp_build_shl(bld, a, b);
}


/**
 * Shift right with immediate.
 * The immediate shift count must be smaller than the type width.
 */
LLVMValueRef
lp_build_shr_imm(struct lp_build_context *bld, LLVMValueRef a, unsigned imm)
{
   LLVMValueRef b = lp_build_const_int_vec(bld->gallivm, bld->type, imm);
   assert(imm < bld->type.width);
   return lp_build_shr(bld, a, b);
}

LLVMValueRef
lp_build_popcount(struct lp_build_context *bld, LLVMValueRef a)
{
   LLVMBuilderRef builder = bld->gallivm->builder;
   LLVMValueRef result;
   char intr_str[256];

   lp_format_intrinsic(intr_str, sizeof(intr_str), "llvm.ctpop", bld->vec_type);
   result = lp_build_intrinsic_unary(builder, intr_str, bld->vec_type, a);
   return result;
}

LLVMValueRef
lp_build_bitfield_reverse(struct lp_build_context *bld, LLVMValueRef a)
{
   LLVMBuilderRef builder = bld->gallivm->builder;
   LLVMValueRef result;
   char intr_str[256];

   lp_format_intrinsic(intr_str, sizeof(intr_str), "llvm.bitreverse", bld->vec_type);
   result = lp_build_intrinsic_unary(builder, intr_str, bld->vec_type, a);
   return result;
}

LLVMValueRef
lp_build_cttz(struct lp_build_context *bld, LLVMValueRef a)
{
   LLVMBuilderRef builder = bld->gallivm->builder;
   LLVMValueRef result;
   char intr_str[256];

   lp_format_intrinsic(intr_str, sizeof(intr_str), "llvm.cttz", bld->vec_type);

   LLVMValueRef undef_val = LLVMConstNull(LLVMInt1TypeInContext(bld->gallivm->context));
   result = lp_build_intrinsic_binary(builder, intr_str, bld->vec_type, a, undef_val);
   return LLVMBuildSelect(builder, LLVMBuildICmp(builder, LLVMIntEQ, a, bld->zero, ""),
			  lp_build_const_int_vec(bld->gallivm, bld->type, -1), result, "");
}

LLVMValueRef
lp_build_ctlz(struct lp_build_context *bld, LLVMValueRef a)
{
   LLVMBuilderRef builder = bld->gallivm->builder;
   LLVMValueRef result;
   char intr_str[256];

   lp_format_intrinsic(intr_str, sizeof(intr_str), "llvm.ctlz", bld->vec_type);

   LLVMValueRef undef_val = LLVMConstNull(LLVMInt1TypeInContext(bld->gallivm->context));
   result = lp_build_intrinsic_binary(builder, intr_str, bld->vec_type, a, undef_val);
   return result;
}
