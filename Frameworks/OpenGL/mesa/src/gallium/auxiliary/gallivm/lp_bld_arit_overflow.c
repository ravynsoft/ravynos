/**************************************************************************
 *
 * Copyright 2013
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
 * Helper
 *
 * The functions in this file implement arthmetic operations with support
 * for overflow detection and reporting.
 *
 */

#include "lp_bld_arit_overflow.h"

#include "lp_bld_type.h"
#include "lp_bld_const.h"
#include "lp_bld_init.h"
#include "lp_bld_intr.h"
#include "lp_bld_logic.h"
#include "lp_bld_pack.h"
#include "lp_bld_debug.h"
#include "lp_bld_bitarit.h"

#include "util/u_memory.h"
#include "util/u_debug.h"
#include "util/u_math.h"
#include "util/u_string.h"
#include "util/u_cpu_detect.h"

#include <float.h>


static LLVMValueRef
build_binary_int_overflow(struct gallivm_state *gallivm,
                          const char *intr_prefix,
                          LLVMValueRef a,
                          LLVMValueRef b,
                          LLVMValueRef *ofbit)
{
   LLVMBuilderRef builder = gallivm->builder;
   char intr_str[256];
   LLVMTypeRef type_ref;
   ASSERTED LLVMTypeKind type_kind;
   unsigned type_width;
   LLVMTypeRef oelems[2];
   LLVMValueRef oresult;
   LLVMTypeRef otype;

   assert(LLVMTypeOf(a) == LLVMTypeOf(b));
   type_ref = LLVMTypeOf(a);
   type_kind = LLVMGetTypeKind(type_ref);

   assert(type_kind == LLVMIntegerTypeKind);
   type_width = LLVMGetIntTypeWidth(type_ref);

   assert(type_width == 16 || type_width == 32 || type_width == 64);

   snprintf(intr_str, sizeof intr_str, "%s.i%u",
            intr_prefix, type_width);

   oelems[0] = type_ref;
   oelems[1] = LLVMInt1TypeInContext(gallivm->context);

   otype = LLVMStructTypeInContext(gallivm->context, oelems, 2, false);
   oresult = lp_build_intrinsic_binary(builder, intr_str,
                                       otype, a, b);
   if (ofbit) {
      if (*ofbit) {
         *ofbit = LLVMBuildOr(
            builder, *ofbit,
            LLVMBuildExtractValue(builder, oresult, 1, ""), "");
      } else {
         *ofbit = LLVMBuildExtractValue(builder, oresult, 1, "");
      }
   }

   return LLVMBuildExtractValue(builder, oresult, 0, "");
}

/**
 * Performs unsigned addition of two integers and reports 
 * overflow if detected.
 *
 * The values @a and @b must be of the same integer type. If
 * an overflow is detected the IN/OUT @ofbit parameter is used:
 * - if it's pointing to a null value, the overflow bit is simply
 *   stored inside the variable it's pointing to,
 * - if it's pointing to a valid value, then that variable,
 *   which must be of i1 type, is ORed with the newly detected
 *   overflow bit. This is done to allow chaining of a number of
 *   overflow functions together without having to test the 
 *   overflow bit after every single one.
 */
LLVMValueRef
lp_build_uadd_overflow(struct gallivm_state *gallivm,
                       LLVMValueRef a,
                       LLVMValueRef b,
                       LLVMValueRef *ofbit)
{
   return build_binary_int_overflow(gallivm, "llvm.uadd.with.overflow",
                                    a, b, ofbit);
}

/**
 * Performs unsigned subtraction of two integers and reports 
 * overflow if detected.
 *
 * The values @a and @b must be of the same integer type. If
 * an overflow is detected the IN/OUT @ofbit parameter is used:
 * - if it's pointing to a null value, the overflow bit is simply
 *   stored inside the variable it's pointing to,
 * - if it's pointing to a valid value, then that variable,
 *   which must be of i1 type, is ORed with the newly detected
 *   overflow bit. This is done to allow chaining of a number of
 *   overflow functions together without having to test the 
 *   overflow bit after every single one.
 */
LLVMValueRef
lp_build_usub_overflow(struct gallivm_state *gallivm,
                       LLVMValueRef a,
                       LLVMValueRef b,
                       LLVMValueRef *ofbit)
{
   return build_binary_int_overflow(gallivm, "llvm.usub.with.overflow",
                                    a, b, ofbit);
}

/**
 * Performs unsigned multiplication of  two integers and 
 * reports overflow if detected.
 *
 * The values @a and @b must be of the same integer type. If
 * an overflow is detected the IN/OUT @ofbit parameter is used:
 * - if it's pointing to a null value, the overflow bit is simply
 *   stored inside the variable it's pointing to,
 * - if it's pointing to a valid value, then that variable,
 *   which must be of i1 type, is ORed with the newly detected
 *   overflow bit. This is done to allow chaining of a number of
 *   overflow functions together without having to test the 
 *   overflow bit after every single one.
 */
LLVMValueRef
lp_build_umul_overflow(struct gallivm_state *gallivm,
                       LLVMValueRef a,
                       LLVMValueRef b,
                       LLVMValueRef *ofbit)
{
   return build_binary_int_overflow(gallivm, "llvm.umul.with.overflow",
                                    a, b, ofbit);
}
