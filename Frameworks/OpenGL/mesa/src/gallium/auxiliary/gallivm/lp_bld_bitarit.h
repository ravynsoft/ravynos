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
 * Helper bitwise arithmetic functions.
 *
 * @author Jose Fonseca <jfonseca@vmware.com>
 */


#ifndef LP_BLD_BITARIT_H
#define LP_BLD_BITARIT_H


#include "gallivm/lp_bld.h"


struct lp_type;
struct lp_build_context;


LLVMValueRef
lp_build_or(struct lp_build_context *bld, LLVMValueRef a, LLVMValueRef b);

LLVMValueRef
lp_build_xor(struct lp_build_context *bld, LLVMValueRef a, LLVMValueRef b);

LLVMValueRef
lp_build_and(struct lp_build_context *bld, LLVMValueRef a, LLVMValueRef b);

LLVMValueRef
lp_build_andnot(struct lp_build_context *bld, LLVMValueRef a, LLVMValueRef b);

LLVMValueRef
lp_build_shl(struct lp_build_context *bld, LLVMValueRef a, LLVMValueRef b);

LLVMValueRef
lp_build_shr(struct lp_build_context *bld, LLVMValueRef a, LLVMValueRef b);

LLVMValueRef
lp_build_shl_imm(struct lp_build_context *bld, LLVMValueRef a, unsigned imm);

LLVMValueRef
lp_build_shr_imm(struct lp_build_context *bld, LLVMValueRef a, unsigned imm);

LLVMValueRef
lp_build_not(struct lp_build_context *bld, LLVMValueRef a);

LLVMValueRef
lp_build_popcount(struct lp_build_context *bld, LLVMValueRef a);

LLVMValueRef
lp_build_bitfield_reverse(struct lp_build_context *bld, LLVMValueRef a);

LLVMValueRef
lp_build_cttz(struct lp_build_context *bld, LLVMValueRef a);

LLVMValueRef
lp_build_ctlz(struct lp_build_context *bld, LLVMValueRef a);
#endif /* !LP_BLD_ARIT_H */
