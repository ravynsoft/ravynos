/**************************************************************************
 *
 * Copyright 2009 VMware, Inc.
 * All rights reserved.
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

#ifndef LP_JIT_SAMPLE_H
#define LP_JIT_SAMPLE_H

struct lp_sampler_static_state
{
   /*
    * These attributes are effectively interleaved for more sane key handling.
    * However, there might be lots of null space if the amount of samplers and
    * textures isn't the same.
    */
   struct lp_static_sampler_state sampler_state;
   struct lp_static_texture_state texture_state;
};


struct lp_image_static_state
{
   struct lp_static_texture_state image_state;
};

struct lp_build_sampler_soa *
lp_bld_llvm_sampler_soa_create(const struct lp_sampler_static_state *static_state,
                               unsigned nr_samplers);

static inline void
lp_bld_llvm_sampler_soa_destroy(struct lp_build_sampler_soa *sampler)
{
   FREE(sampler);
}

struct lp_build_image_soa *
lp_bld_llvm_image_soa_create(const struct lp_image_static_state *static_state,
                             unsigned nr_images);

static inline void
lp_bld_llvm_image_soa_destroy(struct lp_build_image_soa *image)
{
   FREE(image);
}

#if LLVM_VERSION_MAJOR >= 15
#define LP_TOTAL_IMAGE_OP_COUNT ((LP_IMG_OP_COUNT + LLVMAtomicRMWBinOpFMin) * 2)
#else
#define LP_TOTAL_IMAGE_OP_COUNT ((LP_IMG_OP_COUNT + 14) * 2)
#endif

#endif
