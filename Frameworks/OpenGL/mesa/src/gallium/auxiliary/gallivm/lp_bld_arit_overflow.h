/**************************************************************************
 *
 * Copyright 2013 VMware, Inc.
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
 * Helper arithmetic functions with support for overflow detection
 * and reporting.
 *
 * @author Zack Rusin <zackr@vmware.com>
 */


#ifndef LP_BLD_ARIT_OVERFLOW_H
#define LP_BLD_ARIT_OVERFLOW_H


#include "gallivm/lp_bld.h"

struct gallivm_state;

LLVMValueRef
lp_build_uadd_overflow(struct gallivm_state *gallivm,
                       LLVMValueRef a,
                       LLVMValueRef b,
                       LLVMValueRef *ofbit);

LLVMValueRef
lp_build_usub_overflow(struct gallivm_state *gallivm,
                       LLVMValueRef a,
                       LLVMValueRef b,
                       LLVMValueRef *ofbit);

LLVMValueRef
lp_build_umul_overflow(struct gallivm_state *gallivm,
                       LLVMValueRef a,
                       LLVMValueRef b,
                       LLVMValueRef *ofbit);

#endif /* !LP_BLD_ARIT_OVERFLOW_H */
