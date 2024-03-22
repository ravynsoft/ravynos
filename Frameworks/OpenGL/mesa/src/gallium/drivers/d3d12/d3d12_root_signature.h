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

#ifndef D3D12_ROOT_SIGNATURE_H
#define D3D12_ROOT_SIGNATURE_H

#include "d3d12_context.h"

struct d3d12_root_signature_key {
   bool compute;
   bool has_stream_output;
   struct {
      unsigned num_cb_bindings;
      unsigned end_srv_binding;
      unsigned begin_srv_binding;
      unsigned state_vars_size;
      unsigned num_ssbos;
      unsigned num_images;
      bool has_default_ubo0;
   } stages[D3D12_GFX_SHADER_STAGES];
};

void
d3d12_root_signature_cache_init(struct d3d12_context *ctx);

void
d3d12_root_signature_cache_destroy(struct d3d12_context *ctx);

ID3D12RootSignature *
d3d12_get_root_signature(struct d3d12_context *ctx, bool compute);

#endif
