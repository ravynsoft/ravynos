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
 * Alpha testing to LLVM IR translation.
 *
 * @author Jose Fonseca <jfonseca@vmware.com>
 */

#include "pipe/p_state.h"
#include "util/format/u_format.h"

#include "gallivm/lp_bld_type.h"
#include "gallivm/lp_bld_const.h"
#include "gallivm/lp_bld_arit.h"
#include "gallivm/lp_bld_conv.h"
#include "gallivm/lp_bld_logic.h"
#include "gallivm/lp_bld_flow.h"
#include "gallivm/lp_bld_debug.h"

#include "lp_bld_alpha.h"


void
lp_build_alpha_test(struct gallivm_state *gallivm,
                    unsigned func,
                    struct lp_type type,
                    const struct util_format_description *cbuf_format_desc,
                    struct lp_build_mask_context *mask,
                    LLVMValueRef alpha,
                    LLVMValueRef ref,
                    bool do_branch)
{
   struct lp_build_context bld;

   lp_build_context_init(&bld, gallivm, type);

   /*
    * Alpha testing needs to be done in the color buffer precision.
    *
    * TODO: Ideally, instead of duplicating the color conversion code, we
    * would do alpha testing after converting the output colors, but that's
    * not very convenient, because it needs to be done before depth testing.
    * Hopefully LLVM will detect and remove the duplicate expression.
    *
    * FIXME: This should be generalized to formats other than rgba8 variants.
    */
   if (type.floating &&
       util_format_is_rgba8_variant(cbuf_format_desc)) {
      const unsigned dst_width = 8;

      alpha = lp_build_clamp(&bld, alpha, bld.zero, bld.one);
      ref   = lp_build_clamp(&bld, ref,   bld.zero, bld.one);

      alpha = lp_build_clamped_float_to_unsigned_norm(gallivm, type, dst_width, alpha);
      ref   = lp_build_clamped_float_to_unsigned_norm(gallivm, type, dst_width, ref);

      type.floating = 0;
      lp_build_context_init(&bld, gallivm, type);
   }

   LLVMValueRef test = lp_build_cmp(&bld, func, alpha, ref);

   lp_build_name(test, "alpha_mask");

   lp_build_mask_update(mask, test);

   if (do_branch)
      lp_build_mask_check(mask);
}
