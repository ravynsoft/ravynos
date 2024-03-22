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
 * Helper functions for swizzling/shuffling.
 *
 * @author Jose Fonseca <jfonseca@vmware.com>
 */


#ifndef LP_BLD_SWIZZLE_H
#define LP_BLD_SWIZZLE_H


#include "gallivm/lp_bld.h"
#include "pipe/p_defines.h"
#include "util/format/u_formats.h"


struct lp_type;
struct lp_build_context;


#define LP_BLD_SWIZZLE_DONTCARE 0xFF


LLVMValueRef
lp_build_broadcast(struct gallivm_state *gallivm,
                   LLVMTypeRef vec_type,
                   LLVMValueRef scalar);


LLVMValueRef
lp_build_broadcast_scalar(struct lp_build_context *bld,
                          LLVMValueRef scalar);


LLVMValueRef
lp_build_extract_broadcast(struct gallivm_state *gallivm,
                           struct lp_type src_type,
                           struct lp_type dst_type,
                           LLVMValueRef vector,
                           LLVMValueRef index);


/**
 * Broadcast one channel of a vector composed of arrays of XYZ.. structures into
 * all channels XXX...
 */
LLVMValueRef
lp_build_swizzle_scalar_aos(struct lp_build_context *bld,
                            LLVMValueRef a,
                            unsigned channel,
                            unsigned num_channels);


/**
 * Swizzle a vector consisting of an array of XYZW structs.
 *
 * @param swizzles is the in [0,4[ range.
 */
LLVMValueRef
lp_build_swizzle_aos(struct lp_build_context *bld,
                     LLVMValueRef a,
                     const unsigned char swizzles[4]);


LLVMValueRef
lp_build_swizzle_aos_n(struct gallivm_state* gallivm,
                       LLVMValueRef src,
                       const unsigned char* swizzles,
                       unsigned num_swizzles,
                       unsigned dst_len);


LLVMValueRef
lp_build_swizzle_soa_channel(struct lp_build_context *bld,
                             const LLVMValueRef *unswizzled,
                             enum pipe_swizzle swizzle);


void
lp_build_swizzle_soa(struct lp_build_context *bld,
                     const LLVMValueRef *unswizzled,
                     const unsigned char swizzles[4],
                     LLVMValueRef *swizzled);


void
lp_build_swizzle_soa_inplace(struct lp_build_context *bld,
                             LLVMValueRef *values,
                             const unsigned char swizzles[4]);


void
lp_build_transpose_aos(struct gallivm_state *gallivm,
                       struct lp_type type,
                       const LLVMValueRef src[4],
                       LLVMValueRef dst[4]);


void
lp_build_transpose_aos_n(struct gallivm_state *gallivm,
                         struct lp_type type,
                         const LLVMValueRef* src,
                         unsigned num_srcs,
                         LLVMValueRef* dst);


LLVMValueRef
lp_build_pack_aos_scalars(struct gallivm_state *gallivm,
                          struct lp_type src_type,
                          struct lp_type dst_type,
                          const LLVMValueRef src,
                          unsigned channel);


LLVMValueRef
lp_build_unpack_broadcast_aos_scalars(struct gallivm_state *gallivm,
                                      struct lp_type src_type,
                                      struct lp_type dst_type,
                                      const LLVMValueRef src);


#endif /* !LP_BLD_SWIZZLE_H */
