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

/**
 * Texture sampling code generation
 *
 * This file is nothing more than ugly glue between three largely independent
 * entities:
 * - TGSI -> LLVM translation (i.e., lp_build_tgsi_soa)
 * - texture sampling code generation (i.e., lp_build_sample_soa)
 * - LLVM pipe driver
 *
 * All interesting code is in the functions mentioned above. There is really
 * nothing to see here.
 *
 * @author Jose Fonseca <jfonseca@vmware.com>
 */

#include "pipe/p_defines.h"
#include "pipe/p_shader_tokens.h"
#include "gallivm/lp_bld_debug.h"
#include "gallivm/lp_bld_const.h"
#include "gallivm/lp_bld_type.h"
#include "gallivm/lp_bld_sample.h"
#include "gallivm/lp_bld_tgsi.h"
#include "lp_jit.h"
#include "lp_tex_sample.h"
#include "lp_state_fs.h"
#include "lp_debug.h"


#if LP_USE_TEXTURE_CACHE
static LLVMValueRef
lp_llvm_texture_cache_ptr(struct gallivm_state *gallivm,
                          LLVMTypeRef thread_data_type,
                          LLVMValueRef thread_data_ptr,
                          unsigned unit)
{
   /* We use the same cache for all units */
   (void)unit;

   return lp_jit_thread_data_cache(gallivm, thread_data_type, thread_data_ptr);
}
#endif
struct lp_build_sampler_soa *
lp_llvm_sampler_soa_create(const struct lp_sampler_static_state *static_state,
                           unsigned nr_samplers)
{
   struct lp_build_sampler_soa *sampler;

   sampler = lp_bld_llvm_sampler_soa_create(static_state, nr_samplers);

#if LP_USE_TEXTURE_CACHE
   struct lp_sampler_dynamic_state *dynamic_state = lp_build_sampler_soa_dynamic_state(sampler);
   dynamic_state->cache_ptr = lp_llvm_texture_cache_ptr;
#endif
   return sampler;
}


