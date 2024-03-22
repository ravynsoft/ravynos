
/*
 * Copyright © Microsoft Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef DXIL_SPIRV_NIR_H
#define DXIL_SPIRV_NIR_H

#include "spirv_to_dxil.h"
#include "nir.h"

const struct spirv_to_nir_options*
dxil_spirv_nir_get_spirv_options(void);

void
dxil_spirv_nir_prep(nir_shader *nir);

/* The pipeline will require runtime data if, and only if, any of the multiple reported
 * runtime data required flags is true.
 */

void
dxil_spirv_nir_link(nir_shader *nir, nir_shader *prev_stage_nir,
                    const struct dxil_spirv_runtime_conf *conf,
                    bool *requires_runtime_data);

void
dxil_spirv_nir_passes(nir_shader *nir,
                      const struct dxil_spirv_runtime_conf *conf,
                      bool *requires_runtime_data);

struct dxil_spirv_binding_remapping {
   /* If ~0, don't lower to bindless */
   uint32_t descriptor_set;
   uint32_t binding;
   bool is_sampler;
};
struct dxil_spirv_nir_lower_bindless_options {
   uint32_t num_descriptor_sets;
   uint32_t dynamic_buffer_binding;
   void(*remap_binding)(struct dxil_spirv_binding_remapping *inout, void *context);
   void *callback_context;
};

/* Each entry in a bindless descriptor set follows this layout. The first 32 bits are
 * either a texture or buffer descriptor index in the descriptor heap. For pure
 * sampler types, these bits are unused. The upper 32 bits are either a buffer
 * offset, or for samplers (including combined image+sampler), a sampler index. */
struct dxil_spirv_bindless_entry {
   union {
      uint32_t texture_idx;
      uint32_t buffer_idx;
   };
   union {
      uint32_t buffer_offset;
      uint32_t sampler_idx;
   };
};

bool
dxil_spirv_nir_lower_bindless(nir_shader *nir, struct dxil_spirv_nir_lower_bindless_options *options);

bool
dxil_spirv_nir_lower_yz_flip(nir_shader *shader,
                             const struct dxil_spirv_runtime_conf *rt_conf,
                             bool *reads_sysval_ubo);

#endif
