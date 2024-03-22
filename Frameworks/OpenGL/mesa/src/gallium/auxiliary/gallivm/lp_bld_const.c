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
 * Helper functions for constant building.
 *
 * @author Jose Fonseca <jfonseca@vmware.com>
 */

#include <float.h>

#include "util/u_debug.h"
#include "util/u_math.h"
#include "util/half_float.h"

#include "lp_bld_type.h"
#include "lp_bld_const.h"
#include "lp_bld_init.h"
#include "lp_bld_limits.h"


unsigned
lp_mantissa(struct lp_type type)
{
   assert(type.floating);

   if (type.floating) {
      switch (type.width) {
      case 16:
         return 10;
      case 32:
         return 23;
      case 64:
         return 52;
      default:
         assert(0);
         return 0;
      }
   } else {
      if (type.sign)
         return type.width - 1;
      else
         return type.width;
   }
}


/**
 * Shift of the unity.
 *
 * Same as lp_const_scale(), but in terms of shifts.
 */
unsigned
lp_const_shift(struct lp_type type)
{
   if (type.floating)
      return 0;
   else if (type.fixed)
      return type.width/2;
   else if (type.norm)
      return type.sign ? type.width - 1 : type.width;
   else
      return 0;
}


unsigned
lp_const_offset(struct lp_type type)
{
   if (type.floating || type.fixed)
      return 0;
   else if (type.norm)
      return 1;
   else
      return 0;
}


/**
 * Scaling factor between the LLVM native value and its interpretation.
 *
 * This is 1.0 for all floating types and unnormalized integers, and something
 * else for the fixed points types and normalized integers.
 */
double
lp_const_scale(struct lp_type type)
{
   unsigned long long llscale;
   double dscale;

   llscale = (unsigned long long)1 << lp_const_shift(type);
   llscale -= lp_const_offset(type);
   dscale = (double)llscale;
   assert((unsigned long long)dscale == llscale);

   return dscale;
}


/**
 * Minimum value representable by the type.
 */
double
lp_const_min(struct lp_type type)
{
   if (!type.sign)
      return 0.0;

   if (type.norm)
      return -1.0;

   if (type.floating) {
      switch (type.width) {
      case 16:
         return -65504;
      case 32:
         return -FLT_MAX;
      case 64:
         return -DBL_MAX;
      default:
         assert(0);
         return 0.0;
      }
   }

   unsigned bits;
   if (type.fixed)
      /* FIXME: consider the fractional bits? */
      bits = type.width / 2 - 1;
   else
      bits = type.width - 1;

   return (double)-((long long)1 << bits);
}


/**
 * Maximum value representable by the type.
 */
double
lp_const_max(struct lp_type type)
{
   if (type.norm)
      return 1.0;

   if (type.floating) {
      switch (type.width) {
      case 16:
         return 65504;
      case 32:
         return FLT_MAX;
      case 64:
         return DBL_MAX;
      default:
         assert(0);
         return 0.0;
      }
   }

   unsigned bits;
   if (type.fixed)
      bits = type.width / 2;
   else
      bits = type.width;

   if (type.sign)
      bits -= 1;

   return (double)(((unsigned long long)1 << bits) - 1);
}


double
lp_const_eps(struct lp_type type)
{
   if (type.floating) {
      switch (type.width) {
      case 16:
         return 2E-10;
      case 32:
         return FLT_EPSILON;
      case 64:
         return DBL_EPSILON;
      default:
         assert(0);
         return 0.0;
      }
   } else {
      double scale = lp_const_scale(type);
      return 1.0/scale;
   }
}


LLVMValueRef
lp_build_undef(struct gallivm_state *gallivm, struct lp_type type)
{
   LLVMTypeRef vec_type = lp_build_vec_type(gallivm, type);
   return LLVMGetUndef(vec_type);
}


LLVMValueRef
lp_build_zero(struct gallivm_state *gallivm, struct lp_type type)
{
   if (type.length == 1) {
      if (type.floating)
         return lp_build_const_float(gallivm, 0.0);
      else
         return LLVMConstInt(LLVMIntTypeInContext(gallivm->context, type.width), 0, 0);
   } else {
      LLVMTypeRef vec_type = lp_build_vec_type(gallivm, type);
      return LLVMConstNull(vec_type);
   }
}


LLVMValueRef
lp_build_one(struct gallivm_state *gallivm, struct lp_type type)
{
   LLVMTypeRef elem_type;
   LLVMValueRef elems[LP_MAX_VECTOR_LENGTH];

   assert(type.length <= LP_MAX_VECTOR_LENGTH);

   elem_type = lp_build_elem_type(gallivm, type);

   if (!lp_has_fp16() && type.floating && type.width == 16)
      elems[0] = LLVMConstInt(elem_type, _mesa_float_to_half(1.0f), 0);
   else if (type.floating)
      elems[0] = LLVMConstReal(elem_type, 1.0);
   else if (type.fixed)
      elems[0] = LLVMConstInt(elem_type, 1LL << (type.width/2), 0);
   else if (!type.norm)
      elems[0] = LLVMConstInt(elem_type, 1, 0);
   else if (type.sign)
      elems[0] = LLVMConstInt(elem_type, (1LL << (type.width - 1)) - 1, 0);
   else {
      /* special case' -- 1.0 for normalized types is more easily attained if
       * we start with a vector consisting of all bits set */
      LLVMTypeRef vec_type = lp_build_vec_type(gallivm, type);
      LLVMValueRef vec = LLVMConstAllOnes(vec_type);

#if 0
      if (type.sign)
         /* TODO: Unfortunately this caused "Tried to create a shift operation
          * on a non-integer type!" */
         vec = LLVMConstLShr(vec, lp_build_const_int_vec(type, 1));
#endif

      return vec;
   }

   for (unsigned i = 1; i < type.length; ++i)
      elems[i] = elems[0];

   if (type.length == 1)
      return elems[0];
   else
      return LLVMConstVector(elems, type.length);
}


/**
 * Build constant-valued element from a scalar value.
 */
LLVMValueRef
lp_build_const_elem(struct gallivm_state *gallivm,
                    struct lp_type type,
                    double val)
{
   LLVMTypeRef elem_type = lp_build_elem_type(gallivm, type);
   LLVMValueRef elem;

   if (!lp_has_fp16() && type.floating && type.width == 16) {
      elem = LLVMConstInt(elem_type, _mesa_float_to_half((float)val), 0);
   } else if (type.floating) {
      elem = LLVMConstReal(elem_type, val);
   } else {
      double dscale = lp_const_scale(type);

      elem = LLVMConstInt(elem_type, (long long) round(val*dscale), 0);
   }

   return elem;
}


/**
 * Build constant-valued vector from a scalar value.
 */
LLVMValueRef
lp_build_const_vec(struct gallivm_state *gallivm, struct lp_type type,
                   double val)
{
   if (type.length == 1) {
      return lp_build_const_elem(gallivm, type, val);
   } else {
      LLVMValueRef elems[LP_MAX_VECTOR_LENGTH];
      elems[0] = lp_build_const_elem(gallivm, type, val);
      for (unsigned i = 1; i < type.length; ++i)
         elems[i] = elems[0];
      return LLVMConstVector(elems, type.length);
   }
}


LLVMValueRef
lp_build_const_int_vec(struct gallivm_state *gallivm, struct lp_type type,
                       long long val)
{
   LLVMTypeRef elem_type = lp_build_int_elem_type(gallivm, type);
   LLVMValueRef elems[LP_MAX_VECTOR_LENGTH];

   assert(type.length <= LP_MAX_VECTOR_LENGTH);

   for (unsigned i = 0; i < type.length; ++i)
      elems[i] = LLVMConstInt(elem_type, val, type.sign ? 1 : 0);

   if (type.length == 1)
      return elems[0];

   return LLVMConstVector(elems, type.length);
}

/* Returns an integer vector of [0, 1, 2, ...] */
LLVMValueRef
lp_build_const_channel_vec(struct gallivm_state *gallivm, struct lp_type type)
{
   LLVMTypeRef elem_type = lp_build_int_elem_type(gallivm, type);
   LLVMValueRef elems[LP_MAX_VECTOR_LENGTH];

   assert(type.length <= LP_MAX_VECTOR_LENGTH);

   for (unsigned i = 0; i < type.length; ++i)
      elems[i] = LLVMConstInt(elem_type, i, 0);

   if (type.length == 1)
      return elems[0];

   return LLVMConstVector(elems, type.length);
}


LLVMValueRef
lp_build_const_aos(struct gallivm_state *gallivm,
                   struct lp_type type,
                   double r, double g, double b, double a,
                   const unsigned char *swizzle)
{
   const unsigned char default_swizzle[4] = {0, 1, 2, 3};
   LLVMValueRef elems[LP_MAX_VECTOR_LENGTH];

   assert(type.length % 4 == 0);
   assert(type.length <= LP_MAX_VECTOR_LENGTH);

   lp_build_elem_type(gallivm, type);

   if (!swizzle)
      swizzle = default_swizzle;

   elems[swizzle[0]] = lp_build_const_elem(gallivm, type, r);
   elems[swizzle[1]] = lp_build_const_elem(gallivm, type, g);
   elems[swizzle[2]] = lp_build_const_elem(gallivm, type, b);
   elems[swizzle[3]] = lp_build_const_elem(gallivm, type, a);

   for (unsigned i = 4; i < type.length; ++i)
      elems[i] = elems[i % 4];

   return LLVMConstVector(elems, type.length);
}


/**
 * @param mask TGSI_WRITEMASK_xxx
 */
LLVMValueRef
lp_build_const_mask_aos(struct gallivm_state *gallivm,
                        struct lp_type type,
                        unsigned mask,
                        unsigned channels)
{
   LLVMTypeRef elem_type = LLVMIntTypeInContext(gallivm->context, type.width);
   LLVMValueRef masks[LP_MAX_VECTOR_LENGTH];

   assert(type.length <= LP_MAX_VECTOR_LENGTH);

   for (unsigned j = 0; j < type.length; j += channels) {
      for (unsigned i = 0; i < channels; ++i) {
         masks[j + i] = LLVMConstInt(elem_type,
                                     mask & (1 << i) ? ~0ULL : 0,
                                     1);
      }
   }

   return LLVMConstVector(masks, type.length);
}


/**
 * Performs lp_build_const_mask_aos, but first swizzles the mask
 */
LLVMValueRef
lp_build_const_mask_aos_swizzled(struct gallivm_state *gallivm,
                                 struct lp_type type,
                                 unsigned mask,
                                 unsigned channels,
                                 const unsigned char *swizzle)
{
   unsigned mask_swizzled = 0;

   for (unsigned i = 0; i < channels; ++i) {
      if (swizzle[i] < 4) {
         mask_swizzled |= ((mask & (1 << swizzle[i])) >> swizzle[i]) << i;
      }
   }

   return lp_build_const_mask_aos(gallivm, type, mask_swizzled, channels);
}


/**
 * Build a zero-terminated constant string.
 */
LLVMValueRef
lp_build_const_string(struct gallivm_state *gallivm,
                      const char *str)
{
   unsigned len = strlen(str) + 1;
   LLVMTypeRef i8 = LLVMInt8TypeInContext(gallivm->context);
   LLVMValueRef string = LLVMAddGlobal(gallivm->module, LLVMArrayType(i8, len), "");
   LLVMSetGlobalConstant(string, true);
   LLVMSetLinkage(string, LLVMInternalLinkage);
   LLVMSetInitializer(string, LLVMConstStringInContext(gallivm->context, str, len, true));
   string = LLVMConstBitCast(string, LLVMPointerType(i8, 0));
   return string;
}


LLVMValueRef
lp_build_const_func_pointer_from_type(struct gallivm_state *gallivm,
                                      const void *ptr,
                                      LLVMTypeRef function_type,
                                      const char *name)
{
   return LLVMBuildBitCast(gallivm->builder,
                           lp_build_const_int_pointer(gallivm, ptr),
                           LLVMPointerType(function_type, 0),
                           name);
}


/**
 * Build a callable function pointer.
 *
 * We use function pointer constants instead of LLVMAddGlobalMapping()
 * to work around a bug in LLVM 2.6, and for efficiency/simplicity.
 */
LLVMValueRef
lp_build_const_func_pointer(struct gallivm_state *gallivm,
                            const void *ptr,
                            LLVMTypeRef ret_type,
                            LLVMTypeRef *arg_types,
                            unsigned num_args,
                            const char *name)
{
   LLVMTypeRef function_type = LLVMFunctionType(ret_type, arg_types, num_args, 0);
   return lp_build_const_func_pointer_from_type(gallivm, ptr, function_type, name);
}
