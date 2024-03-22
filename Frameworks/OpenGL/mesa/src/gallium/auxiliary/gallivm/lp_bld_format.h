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

#ifndef LP_BLD_FORMAT_H
#define LP_BLD_FORMAT_H


/**
 * @file
 * Pixel format helpers.
 */

#include "gallivm/lp_bld.h"
#include "gallivm/lp_bld_init.h"

#include "util/format/u_formats.h"

struct util_format_description;
struct lp_type;
struct lp_build_context;


#define LP_BUILD_FORMAT_CACHE_DEBUG 0
/*
 * Block cache
 *
 * Optional block cache to be used when unpacking big pixel blocks.
 * Must be a power of 2
 */

#define LP_BUILD_FORMAT_CACHE_SIZE 128

/*
 * Note: cache_data needs 16 byte alignment.
 */
struct lp_build_format_cache
{
   alignas(16) uint32_t cache_data[LP_BUILD_FORMAT_CACHE_SIZE][4][4];
   uint64_t cache_tags[LP_BUILD_FORMAT_CACHE_SIZE];
#if LP_BUILD_FORMAT_CACHE_DEBUG
   uint64_t cache_access_total;
   uint64_t cache_access_miss;
#endif
};


enum cache_member {
   LP_BUILD_FORMAT_CACHE_MEMBER_DATA = 0,
   LP_BUILD_FORMAT_CACHE_MEMBER_TAGS,
#if LP_BUILD_FORMAT_CACHE_DEBUG
   LP_BUILD_FORMAT_CACHE_MEMBER_ACCESS_TOTAL,
   LP_BUILD_FORMAT_CACHE_MEMBER_ACCESS_MISS,
#endif
   LP_BUILD_FORMAT_CACHE_MEMBER_COUNT
};


LLVMTypeRef
lp_build_format_cache_type(struct gallivm_state *gallivm);

LLVMTypeRef
lp_build_format_cache_member_type(struct gallivm_state *gallivm, enum cache_member member);

LLVMTypeRef
lp_build_format_cache_elem_type(struct gallivm_state *gallivm, enum cache_member member);

/*
 * AoS
 */

LLVMValueRef
lp_build_format_swizzle_aos(const struct util_format_description *desc,
                            struct lp_build_context *bld,
                            LLVMValueRef unswizzled);

LLVMValueRef
lp_build_pack_rgba_aos(struct gallivm_state *gallivm,
                       const struct util_format_description *desc,
                       LLVMValueRef rgba);

LLVMValueRef
lp_build_fetch_rgba_aos(struct gallivm_state *gallivm,
                        const struct util_format_description *format_desc,
                        struct lp_type type,
                        bool aligned,
                        LLVMValueRef base_ptr,
                        LLVMValueRef offset,
                        LLVMValueRef i,
                        LLVMValueRef j,
                        LLVMValueRef cache);

LLVMValueRef
lp_build_fetch_rgba_aos_array(struct gallivm_state *gallivm,
                        const struct util_format_description *format_desc,
                        struct lp_type type,
                        LLVMValueRef base_ptr,
                        LLVMValueRef offset);


/*
 * SoA
 */

void
lp_build_format_swizzle_soa(const struct util_format_description *format_desc,
                            struct lp_build_context *bld,
                            const LLVMValueRef unswizzled[4],
                            LLVMValueRef swizzled_out[4]);

void
lp_build_unpack_rgba_soa(struct gallivm_state *gallivm,
                         const struct util_format_description *format_desc,
                         struct lp_type type,
                         LLVMValueRef packed,
                         LLVMValueRef rgba_out[4]);

void
lp_build_rgba8_to_fi32_soa(struct gallivm_state *gallivm,
                          struct lp_type dst_type,
                          LLVMValueRef packed,
                          LLVMValueRef *rgba);

void
lp_build_fetch_rgba_soa(struct gallivm_state *gallivm,
                        const struct util_format_description *format_desc,
                        struct lp_type type,
                        bool aligned,
                        LLVMValueRef base_ptr,
                        LLVMValueRef offsets,
                        LLVMValueRef i,
                        LLVMValueRef j,
                        LLVMValueRef cache,
                        LLVMValueRef rgba_out[4]);

void
lp_build_store_rgba_soa(struct gallivm_state *gallivm,
                        const struct util_format_description *format_desc,
                        struct lp_type type,
                        LLVMValueRef exec_mask,
                        LLVMValueRef base_ptr,
                        LLVMValueRef offset,
                        LLVMValueRef out_of_bounds,
                        const LLVMValueRef rgba_in[4]);

/*
 * YUV
 */

LLVMValueRef
lp_build_fetch_subsampled_rgba_aos(struct gallivm_state *gallivm,
                                   const struct util_format_description *format_desc,
                                   unsigned n,
                                   LLVMValueRef base_ptr,
                                   LLVMValueRef offset,
                                   LLVMValueRef i,
                                   LLVMValueRef j);


/*
 * S3TC
 */

LLVMValueRef
lp_build_fetch_s3tc_rgba_aos(struct gallivm_state *gallivm,
                             const struct util_format_description *format_desc,
                             unsigned n,
                             LLVMValueRef base_ptr,
                             LLVMValueRef offset,
                             LLVMValueRef i,
                             LLVMValueRef j,
                             LLVMValueRef cache);

/*
 * RGTC
 */

LLVMValueRef
lp_build_fetch_rgtc_rgba_aos(struct gallivm_state *gallivm,
                             const struct util_format_description *format_desc,
                             unsigned n,
                             LLVMValueRef base_ptr,
                             LLVMValueRef offset,
                             LLVMValueRef i,
                             LLVMValueRef j,
                             LLVMValueRef cache);

/*
 * special float formats
 */

LLVMValueRef
lp_build_float_to_smallfloat(struct gallivm_state *gallivm,
                             struct lp_type i32_type,
                             LLVMValueRef src,
                             unsigned mantissa_bits,
                             unsigned exponent_bits,
                             unsigned mantissa_start,
                             bool has_sign);

LLVMValueRef
lp_build_smallfloat_to_float(struct gallivm_state *gallivm,
                             struct lp_type f32_type,
                             LLVMValueRef src,
                             unsigned mantissa_bits,
                             unsigned exponent_bits,
                             unsigned mantissa_start,
                             bool has_sign);

LLVMValueRef
lp_build_float_to_r11g11b10(struct gallivm_state *gallivm,
                            const LLVMValueRef *src);

void
lp_build_r11g11b10_to_float(struct gallivm_state *gallivm,
                            LLVMValueRef src,
                            LLVMValueRef *dst);

void
lp_build_rgb9e5_to_float(struct gallivm_state *gallivm,
                         LLVMValueRef src,
                         LLVMValueRef *dst);

LLVMValueRef
lp_build_float_to_srgb_packed(struct gallivm_state *gallivm,
                              const struct util_format_description *dst_fmt,
                              struct lp_type src_type,
                              LLVMValueRef *src);

LLVMValueRef
lp_build_srgb_to_linear(struct gallivm_state *gallivm,
                        struct lp_type src_type,
                        unsigned chan_bits,
                        LLVMValueRef src);


#endif /* !LP_BLD_FORMAT_H */
