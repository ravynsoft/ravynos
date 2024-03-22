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

#ifndef D3D12_PIPELINE_STATE_H
#define D3D12_PIPELINE_STATE_H

#include "pipe/p_state.h"

#include "d3d12_common.h"

#ifdef _GAMING_XBOX
typedef D3D12_DEPTH_STENCIL_DESC1 d3d12_depth_stencil_desc_type;
#else
typedef D3D12_DEPTH_STENCIL_DESC2 d3d12_depth_stencil_desc_type;
#endif

struct d3d12_context;
struct d3d12_root_signature;

struct d3d12_vertex_elements_state {
   D3D12_INPUT_ELEMENT_DESC elements[PIPE_MAX_ATTRIBS];
   enum pipe_format format_conversion[PIPE_MAX_ATTRIBS];
   uint16_t strides[PIPE_MAX_ATTRIBS];
   unsigned num_elements:6; // <= PIPE_MAX_ATTRIBS
   unsigned num_buffers:6; // <= PIPE_MAX_ATTRIBS
   unsigned needs_format_emulation:1;
   unsigned unused:19;
};

struct d3d12_rasterizer_state {
   struct pipe_rasterizer_state base;
   D3D12_RASTERIZER_DESC desc;
   void *twoface_back;
};

struct d3d12_blend_state {
   D3D12_BLEND_DESC desc;
   unsigned blend_factor_flags;
   bool is_dual_src;
};

struct d3d12_depth_stencil_alpha_state {
   d3d12_depth_stencil_desc_type desc;
   bool backface_enabled;
};

struct d3d12_gfx_pipeline_state {
   ID3D12RootSignature *root_signature;
   struct d3d12_shader *stages[PIPE_SHADER_TYPES - 1];
   struct pipe_stream_output_info so_info;

   struct d3d12_vertex_elements_state *ves;
   struct d3d12_blend_state *blend;
   struct d3d12_depth_stencil_alpha_state *zsa;
   struct d3d12_rasterizer_state *rast;

   unsigned samples;
   unsigned sample_mask;
   unsigned num_cbufs;
   unsigned num_so_targets;
   bool has_float_rtv;
   DXGI_FORMAT rtv_formats[8];
   DXGI_FORMAT dsv_format;
   D3D12_INDEX_BUFFER_STRIP_CUT_VALUE ib_strip_cut_value;
   enum mesa_prim prim_type;
};

struct d3d12_compute_pipeline_state {
   ID3D12RootSignature *root_signature;
   struct d3d12_shader *stage;
};

DXGI_FORMAT
d3d12_rtv_format(struct d3d12_context *ctx, unsigned index);

void
d3d12_gfx_pipeline_state_cache_init(struct d3d12_context *ctx);

void
d3d12_gfx_pipeline_state_cache_destroy(struct d3d12_context *ctx);

ID3D12PipelineState *
d3d12_get_gfx_pipeline_state(struct d3d12_context *ctx);

void
d3d12_gfx_pipeline_state_cache_invalidate(struct d3d12_context *ctx, const void *state);

void
d3d12_gfx_pipeline_state_cache_invalidate_shader(struct d3d12_context *ctx,
                                                 enum pipe_shader_type stage,
                                                 struct d3d12_shader_selector *selector);

void
d3d12_compute_pipeline_state_cache_init(struct d3d12_context *ctx);

void
d3d12_compute_pipeline_state_cache_destroy(struct d3d12_context *ctx);

ID3D12PipelineState *
d3d12_get_compute_pipeline_state(struct d3d12_context *ctx);

void
d3d12_compute_pipeline_state_cache_invalidate_shader(struct d3d12_context *ctx,
                                                     struct d3d12_shader_selector *selector);

#endif
