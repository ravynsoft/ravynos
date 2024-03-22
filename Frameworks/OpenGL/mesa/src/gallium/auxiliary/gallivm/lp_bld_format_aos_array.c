/**************************************************************************
 *
 * Copyright 2012 VMware, Inc.
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

#include "lp_bld_const.h"
#include "lp_bld_struct.h"
#include "lp_bld_format.h"
#include "lp_bld_debug.h"
#include "lp_bld_type.h"
#include "lp_bld_conv.h"
#include "lp_bld_pack.h"
#include "lp_bld_intr.h"
#include "lp_bld_gather.h"

#include "util/u_memory.h"
#include "util/format/u_format.h"
#include "pipe/p_state.h"



/**
 * @brief lp_build_fetch_rgba_aos_array
 *
 * \param format_desc   describes format of the image we're fetching from
 * \param dst_type      output type
 * \param base_ptr      address of the pixel block (or the texel if uncompressed)
 * \param offset        ptr offset
 */
LLVMValueRef
lp_build_fetch_rgba_aos_array(struct gallivm_state *gallivm,
                              const struct util_format_description *format_desc,
                              struct lp_type dst_type,
                              LLVMValueRef base_ptr,
                              LLVMValueRef offset)
{
   struct lp_build_context bld;
   LLVMBuilderRef builder = gallivm->builder;
   LLVMTypeRef src_vec_type;
   LLVMValueRef ptr, res = NULL;
   struct lp_type src_type;
   bool pure_integer = format_desc->channel[0].pure_integer;
   struct lp_type tmp_type;

   lp_type_from_format_desc(&src_type, format_desc);

   assert(src_type.length <= dst_type.length);

   src_vec_type  = lp_build_vec_type(gallivm,  src_type);

   /*
    * Read whole vector from memory, unaligned.
    * XXX: Note it's actually aligned to element type. Not sure if all
    * callers are able to guarantee that (whereas for others, we should
    * be able to use full alignment when there's 2 or 4 channels).
    * (If all callers can guarantee element type alignment, we should
    * relax alignment restrictions elsewhere.)
    */
   LLVMTypeRef byte_type = LLVMInt8TypeInContext(gallivm->context);
   ptr = LLVMBuildGEP2(builder, byte_type, base_ptr, &offset, 1, "");
   ptr = LLVMBuildPointerCast(builder, ptr, LLVMPointerType(src_vec_type, 0), "");
   res = LLVMBuildLoad2(builder, src_vec_type, ptr, "");
   LLVMSetAlignment(res, src_type.width / 8);

   /* Truncate doubles to float */
   if (src_type.floating && src_type.width == 64) {
      src_type.width = 32;
      src_vec_type  = lp_build_vec_type(gallivm,  src_type);

      res = LLVMBuildFPTrunc(builder, res, src_vec_type, "");
   }

   /* Expand to correct length */
   if (src_type.length < dst_type.length) {
      res = lp_build_pad_vector(gallivm, res, dst_type.length);
      src_type.length = dst_type.length;
   }

   tmp_type = dst_type;
   if (pure_integer) {
       /* some callers expect (fake) floats other real ints. */
      tmp_type.floating = 0;
      tmp_type.sign = src_type.sign;
   }

   /* Convert to correct format */
   lp_build_conv(gallivm, src_type, tmp_type, &res, 1, &res, 1);

   /* Swizzle it */
   lp_build_context_init(&bld, gallivm, tmp_type);
   res = lp_build_format_swizzle_aos(format_desc, &bld, res);

   /* Bitcast to floats (for pure integers) when requested */
   if (pure_integer && dst_type.floating) {
      res = LLVMBuildBitCast(builder, res, lp_build_vec_type(gallivm, dst_type), "");
   }

   return res;
}
