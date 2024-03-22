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
 * Helper functions for packing/unpacking conversions.
 *
 * @author Jose Fonseca <jfonseca@vmware.com>
 */


#ifndef LP_BLD_PACK_H
#define LP_BLD_PACK_H


#include "util/compiler.h"

#include "gallivm/lp_bld.h"


struct lp_type;

LLVMValueRef
lp_build_interleave2_half(struct gallivm_state *gallivm,
                     struct lp_type type,
                     LLVMValueRef a,
                     LLVMValueRef b,
                     unsigned lo_hi);

LLVMValueRef
lp_build_interleave2(struct gallivm_state *gallivm,
                     struct lp_type type,
                     LLVMValueRef a,
                     LLVMValueRef b,
                     unsigned lo_hi);

LLVMValueRef
lp_build_uninterleave1(struct gallivm_state *gallivm,
                       unsigned num_elems,
                       LLVMValueRef a,
                       unsigned lo_hi);

void
lp_build_unpack2(struct gallivm_state *gallivm,
                 struct lp_type src_type,
                 struct lp_type dst_type,
                 LLVMValueRef src,
                 LLVMValueRef *dst_lo,
                 LLVMValueRef *dst_hi);


void
lp_build_unpack2_native(struct gallivm_state *gallivm,
                        struct lp_type src_type,
                        struct lp_type dst_type,
                        LLVMValueRef src,
                        LLVMValueRef *dst_lo,
                        LLVMValueRef *dst_hi);

void
lp_build_unpack(struct gallivm_state *gallivm,
                struct lp_type src_type,
                struct lp_type dst_type,
                LLVMValueRef src,
                LLVMValueRef *dst, unsigned num_dsts);

LLVMValueRef
lp_build_extract_range(struct gallivm_state *gallivm,
                       LLVMValueRef src,
                       unsigned start,
                       unsigned size);

LLVMValueRef
lp_build_concat(struct gallivm_state *gallivm,
                LLVMValueRef src[],
                struct lp_type src_type,
                unsigned num_vectors);

int
lp_build_concat_n(struct gallivm_state *gallivm,
                  struct lp_type src_type,
                  LLVMValueRef *src,
                  unsigned num_srcs,
                  LLVMValueRef *dst,
                  unsigned num_dsts);


LLVMValueRef
lp_build_packs2(struct gallivm_state *gallivm,
                struct lp_type src_type,
                struct lp_type dst_type,
                LLVMValueRef lo,
                LLVMValueRef hi);


LLVMValueRef
lp_build_pack2(struct gallivm_state *gallivm,
               struct lp_type src_type,
               struct lp_type dst_type,
               LLVMValueRef lo,
               LLVMValueRef hi);


LLVMValueRef
lp_build_pack2_native(struct gallivm_state *gallivm,
                      struct lp_type src_type,
                      struct lp_type dst_type,
                      LLVMValueRef lo,
                      LLVMValueRef hi);


LLVMValueRef
lp_build_pack(struct gallivm_state *gallivm,
              struct lp_type src_type,
              struct lp_type dst_type,
              bool clamped,
              const LLVMValueRef *src, unsigned num_srcs);


void
lp_build_resize(struct gallivm_state *gallivm,
                struct lp_type src_type,
                struct lp_type dst_type,
                const LLVMValueRef *src, unsigned num_srcs,
                LLVMValueRef *dst, unsigned num_dsts);


LLVMValueRef
lp_build_pad_vector(struct gallivm_state *gallivm,
                    LLVMValueRef src,
                    unsigned dst_length);

#endif /* !LP_BLD_PACK_H */
