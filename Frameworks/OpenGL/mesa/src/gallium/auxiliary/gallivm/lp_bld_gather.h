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


#ifndef LP_BLD_GATHER_H_
#define LP_BLD_GATHER_H_


#include "gallivm/lp_bld.h"


LLVMValueRef
lp_build_gather_elem_ptr(struct gallivm_state *gallivm,
                         unsigned length,
                         LLVMValueRef base_ptr,
                         LLVMValueRef offsets,
                         unsigned i);

LLVMValueRef
lp_build_gather_elem(struct gallivm_state *gallivm,
                     unsigned length,
                     unsigned src_width,
                     unsigned dst_width,
                     bool aligned,
                     LLVMValueRef base_ptr,
                     LLVMValueRef offsets,
                     unsigned i,
                     bool vector_justify);

LLVMValueRef
lp_build_gather(struct gallivm_state *gallivm,
                unsigned length,
                unsigned src_width,
                struct lp_type dst_type,
                bool aligned,
                LLVMValueRef base_ptr,
                LLVMValueRef offsets,
                bool vector_justify);

LLVMValueRef
lp_build_gather_values(struct gallivm_state * gallivm,
                       LLVMValueRef * values,
                       unsigned value_count);

LLVMValueRef
lp_build_masked_gather(struct gallivm_state *gallivm,
                       unsigned length,
                       unsigned bit_size,
                       LLVMTypeRef vec_type,
                       LLVMValueRef offset_ptr,
                       LLVMValueRef exec_mask);

void
lp_build_masked_scatter(struct gallivm_state *gallivm,
                        unsigned length,
                        unsigned bit_size,
                        LLVMValueRef offset_ptr,
                        LLVMValueRef value_vec,
                        LLVMValueRef exec_mask);

#endif /* LP_BLD_GATHER_H_ */
