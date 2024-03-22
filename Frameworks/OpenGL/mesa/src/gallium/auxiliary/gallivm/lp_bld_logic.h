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
 * Helper functions for logical operations.
 *
 * @author Jose Fonseca <jfonseca@vmware.com>
 */


#ifndef LP_BLD_LOGIC_H
#define LP_BLD_LOGIC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "gallivm/lp_bld.h"

#include "pipe/p_defines.h" /* For PIPE_FUNC_xxx */


struct lp_type;
struct lp_build_context;


LLVMValueRef
lp_build_compare(struct gallivm_state *gallivm,
                 const struct lp_type type,
                 enum pipe_compare_func func,
                 LLVMValueRef a,
                 LLVMValueRef b);


/**
 * @param func is one of PIPE_FUNC_xxx
 */
LLVMValueRef
lp_build_cmp(struct lp_build_context *bld,
             enum pipe_compare_func func,
             LLVMValueRef a,
             LLVMValueRef b);

LLVMValueRef
lp_build_cmp_ordered(struct lp_build_context *bld,
                     enum pipe_compare_func func,
                     LLVMValueRef a,
                     LLVMValueRef b);

LLVMValueRef
lp_build_select_bitwise(struct lp_build_context *bld,
                        LLVMValueRef mask,
                        LLVMValueRef a,
                        LLVMValueRef b);

LLVMValueRef
lp_build_select(struct lp_build_context *bld,
                LLVMValueRef mask,
                LLVMValueRef a,
                LLVMValueRef b);

LLVMValueRef
lp_build_select_aos(struct lp_build_context *bld,
                    unsigned mask,
                    LLVMValueRef a,
                    LLVMValueRef b,
                    unsigned num_channels);


LLVMValueRef
lp_build_any_true_range(struct lp_build_context *bld,
                        unsigned real_length,
                        LLVMValueRef val);

#ifdef __cplusplus
}
#endif

#endif /* !LP_BLD_LOGIC_H */
