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

#ifdef _GAMING_XBOX
#ifdef _GAMING_XBOX_SCARLETT
#include <d3dx12_xs.h>
#else
#include <d3dx12_x.h>
#endif
#endif

#include "d3d12_pipeline_state.h"
#include "d3d12_compiler.h"
#include "d3d12_context.h"
#include "d3d12_screen.h"
#ifndef _GAMING_XBOX
#include <directx/d3dx12_pipeline_state_stream.h>
#endif

#include "util/hash_table.h"
#include "util/set.h"
#include "util/u_memory.h"
#include "util/u_prim.h"

#include <dxguids/dxguids.h>

struct d3d12_gfx_pso_entry {
   struct d3d12_gfx_pipeline_state key;
   ID3D12PipelineState *pso;
};

struct d3d12_compute_pso_entry {
   struct d3d12_compute_pipeline_state key;
   ID3D12PipelineState *pso;
};

static const char *
get_semantic_name(int location, int driver_location, unsigned *index)
{
   *index = 0; /* Default index */

   switch (location) {

   case VARYING_SLOT_POS:
      return "SV_Position";

    case VARYING_SLOT_FACE:
      return "SV_IsFrontFace";

   case VARYING_SLOT_CLIP_DIST1:
      *index = 1;
      FALLTHROUGH;
   case VARYING_SLOT_CLIP_DIST0:
      return "SV_ClipDistance";

   case VARYING_SLOT_CULL_DIST1:
      *index = 1;
      FALLTHROUGH;
   case VARYING_SLOT_CULL_DIST0:
      return "SV_CullDistance";

   case VARYING_SLOT_PRIMITIVE_ID:
      return "SV_PrimitiveID";

   case VARYING_SLOT_VIEWPORT:
      return "SV_ViewportArrayIndex";

   case VARYING_SLOT_LAYER:
      return "SV_RenderTargetArrayIndex";

   default: {
         *index = driver_location;
         return "TEXCOORD";
      }
   }
}

static nir_variable *
find_so_variable(nir_shader *s, int location, unsigned location_frac, unsigned num_components)
{
   nir_foreach_variable_with_modes(var, s, nir_var_shader_out) {
      if (var->data.location != location || var->data.location_frac > location_frac)
         continue;
      unsigned var_num_components = var->data.compact ?
         glsl_get_length(var->type) : glsl_get_components(var->type);
      if (var->data.location_frac <= location_frac &&
          var->data.location_frac + var_num_components >= location_frac + num_components)
         return var;
   }
   return nullptr;
}

static void
fill_so_declaration(const struct pipe_stream_output_info *info,
                    nir_shader *last_vertex_stage,
                    D3D12_SO_DECLARATION_ENTRY *entries, UINT *num_entries,
                    UINT *strides, UINT *num_strides)
{
   int next_offset[PIPE_MAX_VERTEX_STREAMS] = { 0 };

   *num_entries = 0;

   for (unsigned i = 0; i < info->num_outputs; i++) {
      const struct pipe_stream_output *output = &info->output[i];
      const int buffer = output->output_buffer;
      unsigned index;

      /* Mesa doesn't store entries for gl_SkipComponents in the Outputs[]
       * array.  Instead, it simply increments DstOffset for the following
       * input by the number of components that should be skipped.
       *
       * DirectX12 requires that we create gap entries.
       */
      int skip_components = output->dst_offset - next_offset[buffer];

      if (skip_components > 0) {
         entries[*num_entries].Stream = output->stream;
         entries[*num_entries].SemanticName = NULL;
         entries[*num_entries].SemanticIndex = 0;
         entries[*num_entries].StartComponent = 0;
         entries[*num_entries].ComponentCount = skip_components;
         entries[*num_entries].OutputSlot = buffer;
         (*num_entries)++;
      }

      next_offset[buffer] = output->dst_offset + output->num_components;

      entries[*num_entries].Stream = output->stream;
      nir_variable *var = find_so_variable(last_vertex_stage,
         output->register_index, output->start_component, output->num_components);
      assert((var->data.stream & ~NIR_STREAM_PACKED) == output->stream);
      unsigned location = var->data.location;
      if (location == VARYING_SLOT_CLIP_DIST0 || location == VARYING_SLOT_CLIP_DIST1) {
         unsigned component = (location - VARYING_SLOT_CLIP_DIST0) * 4 + var->data.location_frac;
         if (component >= last_vertex_stage->info.clip_distance_array_size)
            location = VARYING_SLOT_CULL_DIST0 + (component - last_vertex_stage->info.clip_distance_array_size) / 4;
      }
      entries[*num_entries].SemanticName = get_semantic_name(location, var->data.driver_location, &index);
      entries[*num_entries].SemanticIndex = index;
      entries[*num_entries].StartComponent = output->start_component - var->data.location_frac;
      entries[*num_entries].ComponentCount = output->num_components;
      entries[*num_entries].OutputSlot = buffer;
      (*num_entries)++;
   }

   for (unsigned i = 0; i < PIPE_MAX_VERTEX_STREAMS; i++)
      strides[i] = info->stride[i] * 4;
   *num_strides = PIPE_MAX_VERTEX_STREAMS;
}

static bool
depth_bias(struct d3d12_rasterizer_state *state, enum mesa_prim reduced_prim)
{
   /* glPolygonOffset is supposed to be only enabled when rendering polygons.
    * In d3d12 case, all polygons (and quads) are lowered to triangles */
   if (reduced_prim != MESA_PRIM_TRIANGLES)
      return false;

   unsigned fill_mode = state->base.cull_face == PIPE_FACE_FRONT ? state->base.fill_back
                                                                 : state->base.fill_front;

   switch (fill_mode) {
   case PIPE_POLYGON_MODE_FILL:
      return state->base.offset_tri;

   case PIPE_POLYGON_MODE_LINE:
      return state->base.offset_line;

   case PIPE_POLYGON_MODE_POINT:
      return state->base.offset_point;

   default:
      unreachable("unexpected fill mode");
   }
}

static D3D12_PRIMITIVE_TOPOLOGY_TYPE
topology_type(enum mesa_prim reduced_prim)
{
   switch (reduced_prim) {
   case MESA_PRIM_POINTS:
      return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;

   case MESA_PRIM_LINES:
      return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;

   case MESA_PRIM_TRIANGLES:
      return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

   case MESA_PRIM_PATCHES:
      return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;

   default:
      debug_printf("mesa_prim: %s\n", u_prim_name(reduced_prim));
      unreachable("unexpected enum mesa_prim");
   }
}

DXGI_FORMAT
d3d12_rtv_format(struct d3d12_context *ctx, unsigned index)
{
   DXGI_FORMAT fmt = ctx->gfx_pipeline_state.rtv_formats[index];

   if (ctx->gfx_pipeline_state.blend->desc.RenderTarget[0].LogicOpEnable &&
       !ctx->gfx_pipeline_state.has_float_rtv) {
      switch (fmt) {
      case DXGI_FORMAT_R8G8B8A8_SNORM:
      case DXGI_FORMAT_R8G8B8A8_UNORM:
      case DXGI_FORMAT_B8G8R8A8_UNORM:
      case DXGI_FORMAT_B8G8R8X8_UNORM:
         return DXGI_FORMAT_R8G8B8A8_UINT;
      default:
         unreachable("unsupported logic-op format");
      }
   }

   return fmt;
}

static void
copy_input_attribs(const D3D12_INPUT_ELEMENT_DESC *ves_elements, D3D12_INPUT_ELEMENT_DESC *ia_elements,
                   D3D12_INPUT_LAYOUT_DESC *ia_desc, nir_shader *vs)
{
   uint32_t vert_input_count = 0;
   int32_t ves_element_count = -1;
   int var_loc = -1;
   nir_foreach_shader_in_variable(var, vs) {
      assert(vert_input_count < D3D12_VS_INPUT_REGISTER_COUNT);

      if (var->data.location != var_loc)
         ves_element_count++;
      var_loc = var->data.location;

      for (uint32_t i = 0; i < glsl_count_attribute_slots(var->type, false); ++i) {
         ia_elements[vert_input_count] = ves_elements[ves_element_count++];
         ia_elements[vert_input_count].SemanticIndex = vert_input_count;
         var->data.driver_location = vert_input_count++;
      }
      --ves_element_count;
   }

   if (vert_input_count > 0) {
      ia_desc->pInputElementDescs = ia_elements;
      ia_desc->NumElements = vert_input_count;
   }
}

static ID3D12PipelineState *
create_gfx_pipeline_state(struct d3d12_context *ctx)
{
   struct d3d12_screen *screen = d3d12_screen(ctx->base.screen);
   struct d3d12_gfx_pipeline_state *state = &ctx->gfx_pipeline_state;
   enum mesa_prim reduced_prim = state->prim_type == MESA_PRIM_PATCHES ?
      MESA_PRIM_PATCHES : u_reduced_prim(state->prim_type);
   D3D12_SO_DECLARATION_ENTRY entries[PIPE_MAX_SO_OUTPUTS];
   UINT strides[PIPE_MAX_VERTEX_STREAMS] = { 0 };
   D3D12_INPUT_ELEMENT_DESC input_attribs[PIPE_MAX_ATTRIBS * 4];
   UINT num_entries = 0, num_strides = 0;

   CD3DX12_PIPELINE_STATE_STREAM3 pso_desc;
   pso_desc.pRootSignature = state->root_signature;

   nir_shader *last_vertex_stage_nir = NULL;

   if (state->stages[PIPE_SHADER_VERTEX]) {
      auto shader = state->stages[PIPE_SHADER_VERTEX];
      pso_desc.VS = D3D12_SHADER_BYTECODE { shader->bytecode, shader->bytecode_length };
      last_vertex_stage_nir = shader->nir;
   }

   if (state->stages[PIPE_SHADER_TESS_CTRL]) {
      auto shader = state->stages[PIPE_SHADER_TESS_CTRL];
      pso_desc.HS = D3D12_SHADER_BYTECODE{ shader->bytecode, shader->bytecode_length };
      last_vertex_stage_nir = shader->nir;
   }

   if (state->stages[PIPE_SHADER_TESS_EVAL]) {
      auto shader = state->stages[PIPE_SHADER_TESS_EVAL];
      pso_desc.DS = D3D12_SHADER_BYTECODE{ shader->bytecode, shader->bytecode_length };
      last_vertex_stage_nir = shader->nir;
   }

   if (state->stages[PIPE_SHADER_GEOMETRY]) {
      auto shader = state->stages[PIPE_SHADER_GEOMETRY];
      pso_desc.GS = D3D12_SHADER_BYTECODE{ shader->bytecode, shader->bytecode_length };
      last_vertex_stage_nir = shader->nir;
   }

   bool last_vertex_stage_writes_pos = (last_vertex_stage_nir->info.outputs_written & VARYING_BIT_POS) != 0;
   if (last_vertex_stage_writes_pos && state->stages[PIPE_SHADER_FRAGMENT] &&
       !state->rast->base.rasterizer_discard) {
      auto shader = state->stages[PIPE_SHADER_FRAGMENT];
      pso_desc.PS = D3D12_SHADER_BYTECODE{ shader->bytecode, shader->bytecode_length };
   }

   if (state->num_so_targets)
      fill_so_declaration(&state->so_info, last_vertex_stage_nir, entries, &num_entries, strides, &num_strides);

   D3D12_STREAM_OUTPUT_DESC& stream_output_desc = (D3D12_STREAM_OUTPUT_DESC&)pso_desc.StreamOutput;
   stream_output_desc.NumEntries = num_entries;
   stream_output_desc.pSODeclaration = entries;
   stream_output_desc.RasterizedStream = state->rast->base.rasterizer_discard ? D3D12_SO_NO_RASTERIZED_STREAM : 0;
   stream_output_desc.NumStrides = num_strides;
   stream_output_desc.pBufferStrides = strides;
   pso_desc.StreamOutput = stream_output_desc;

   D3D12_BLEND_DESC& blend_state = (D3D12_BLEND_DESC&)pso_desc.BlendState;
   blend_state = state->blend->desc;
   if (state->has_float_rtv)
      blend_state.RenderTarget[0].LogicOpEnable = false;

   (d3d12_depth_stencil_desc_type&)pso_desc.DepthStencilState = state->zsa->desc;
   pso_desc.SampleMask = state->sample_mask;

   D3D12_RASTERIZER_DESC& rast = (D3D12_RASTERIZER_DESC&)pso_desc.RasterizerState;
   rast = state->rast->desc;

   if (reduced_prim != MESA_PRIM_TRIANGLES)
      rast.CullMode = D3D12_CULL_MODE_NONE;

   if (depth_bias(state->rast, reduced_prim)) {
      rast.DepthBias = state->rast->base.offset_units * 2;
      rast.DepthBiasClamp = state->rast->base.offset_clamp;
      rast.SlopeScaledDepthBias = state->rast->base.offset_scale;
   }
   D3D12_INPUT_LAYOUT_DESC& input_layout = (D3D12_INPUT_LAYOUT_DESC&)pso_desc.InputLayout;
   input_layout.pInputElementDescs = state->ves->elements;
   input_layout.NumElements = state->ves->num_elements;
   copy_input_attribs(state->ves->elements, input_attribs, &input_layout, state->stages[PIPE_SHADER_VERTEX]->nir);

   pso_desc.IBStripCutValue = state->ib_strip_cut_value;

   pso_desc.PrimitiveTopologyType = topology_type(reduced_prim);

   D3D12_RT_FORMAT_ARRAY& render_targets = (D3D12_RT_FORMAT_ARRAY&)pso_desc.RTVFormats;
   render_targets.NumRenderTargets = state->num_cbufs;
   for (unsigned i = 0; i < state->num_cbufs; ++i)
      render_targets.RTFormats[i] = d3d12_rtv_format(ctx, i);
   pso_desc.DSVFormat = state->dsv_format;

   DXGI_SAMPLE_DESC& samples = (DXGI_SAMPLE_DESC&)pso_desc.SampleDesc;
   samples.Count = state->samples;
   if (state->num_cbufs || state->dsv_format != DXGI_FORMAT_UNKNOWN) {
      if (!state->zsa->desc.DepthEnable &&
          !state->zsa->desc.StencilEnable &&
          !state->rast->desc.MultisampleEnable &&
          state->samples > 1) {
         rast.ForcedSampleCount = 1;
         pso_desc.DSVFormat = DXGI_FORMAT_UNKNOWN;
      }
   }
#ifndef _GAMING_XBOX
   else if (state->samples > 1 &&
              !(screen->opts19.SupportedSampleCountsWithNoOutputs & (1 << state->samples))) {
      samples.Count = 1;
      rast.ForcedSampleCount = state->samples;
   }
#endif
   samples.Quality = 0;

   pso_desc.NodeMask = 0;

   D3D12_CACHED_PIPELINE_STATE& cached_pso = (D3D12_CACHED_PIPELINE_STATE&)pso_desc.CachedPSO;
   cached_pso.pCachedBlob = NULL;
   cached_pso.CachedBlobSizeInBytes = 0;

   pso_desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

   ID3D12PipelineState *ret;

   if (screen->opts14.IndependentFrontAndBackStencilRefMaskSupported) {
      D3D12_PIPELINE_STATE_STREAM_DESC pso_stream_desc{
          sizeof(pso_desc),
          &pso_desc
      };

      if (FAILED(screen->dev->CreatePipelineState(&pso_stream_desc,
                                                  IID_PPV_ARGS(&ret)))) {
         debug_printf("D3D12: CreateGraphicsPipelineState failed!\n");
         return NULL;
      }
   } 
   else {
      D3D12_GRAPHICS_PIPELINE_STATE_DESC v0desc = pso_desc.GraphicsDescV0();
      if (FAILED(screen->dev->CreateGraphicsPipelineState(&v0desc,
                                                       IID_PPV_ARGS(&ret)))) {
         debug_printf("D3D12: CreateGraphicsPipelineState failed!\n");
         return NULL;
      }
   }

   return ret;
}

static uint32_t
hash_gfx_pipeline_state(const void *key)
{
   return _mesa_hash_data(key, sizeof(struct d3d12_gfx_pipeline_state));
}

static bool
equals_gfx_pipeline_state(const void *a, const void *b)
{
   return memcmp(a, b, sizeof(struct d3d12_gfx_pipeline_state)) == 0;
}

ID3D12PipelineState *
d3d12_get_gfx_pipeline_state(struct d3d12_context *ctx)
{
   uint32_t hash = hash_gfx_pipeline_state(&ctx->gfx_pipeline_state);
   struct hash_entry *entry = _mesa_hash_table_search_pre_hashed(ctx->pso_cache, hash,
                                                                 &ctx->gfx_pipeline_state);
   if (!entry) {
      struct d3d12_gfx_pso_entry *data = (struct d3d12_gfx_pso_entry *)MALLOC(sizeof(struct d3d12_gfx_pso_entry));
      if (!data)
         return NULL;

      data->key = ctx->gfx_pipeline_state;
      data->pso = create_gfx_pipeline_state(ctx);
      if (!data->pso) {
         FREE(data);
         return NULL;
      }

      entry = _mesa_hash_table_insert_pre_hashed(ctx->pso_cache, hash, &data->key, data);
      assert(entry);
   }

   return ((struct d3d12_gfx_pso_entry *)(entry->data))->pso;
}

void
d3d12_gfx_pipeline_state_cache_init(struct d3d12_context *ctx)
{
   ctx->pso_cache = _mesa_hash_table_create(NULL, NULL, equals_gfx_pipeline_state);
}

static void
delete_gfx_entry(struct hash_entry *entry)
{
   struct d3d12_gfx_pso_entry *data = (struct d3d12_gfx_pso_entry *)entry->data;
   data->pso->Release();
   FREE(data);
}

static void
remove_gfx_entry(struct d3d12_context *ctx, struct hash_entry *entry)
{
   struct d3d12_gfx_pso_entry *data = (struct d3d12_gfx_pso_entry *)entry->data;

   if (ctx->current_gfx_pso == data->pso)
      ctx->current_gfx_pso = NULL;
   _mesa_hash_table_remove(ctx->pso_cache, entry);
   delete_gfx_entry(entry);
}

void
d3d12_gfx_pipeline_state_cache_destroy(struct d3d12_context *ctx)
{
   _mesa_hash_table_destroy(ctx->pso_cache, delete_gfx_entry);
}

void
d3d12_gfx_pipeline_state_cache_invalidate(struct d3d12_context *ctx, const void *state)
{
   hash_table_foreach(ctx->pso_cache, entry) {
      const struct d3d12_gfx_pipeline_state *key = (struct d3d12_gfx_pipeline_state *)entry->key;
      if (key->blend == state || key->zsa == state || key->rast == state)
         remove_gfx_entry(ctx, entry);
   }
}

void
d3d12_gfx_pipeline_state_cache_invalidate_shader(struct d3d12_context *ctx,
                                                 enum pipe_shader_type stage,
                                                 struct d3d12_shader_selector *selector)
{
   struct d3d12_shader *shader = selector->first;

   while (shader) {
      hash_table_foreach(ctx->pso_cache, entry) {
         const struct d3d12_gfx_pipeline_state *key = (struct d3d12_gfx_pipeline_state *)entry->key;
         if (key->stages[stage] == shader)
            remove_gfx_entry(ctx, entry);
      }
      shader = shader->next_variant;
   }
}

static ID3D12PipelineState *
create_compute_pipeline_state(struct d3d12_context *ctx)
{
   struct d3d12_screen *screen = d3d12_screen(ctx->base.screen);
   struct d3d12_compute_pipeline_state *state = &ctx->compute_pipeline_state;

   D3D12_COMPUTE_PIPELINE_STATE_DESC pso_desc = { 0 };
   pso_desc.pRootSignature = state->root_signature;

   if (state->stage) {
      auto shader = state->stage;
      pso_desc.CS.BytecodeLength = shader->bytecode_length;
      pso_desc.CS.pShaderBytecode = shader->bytecode;
   }

   pso_desc.NodeMask = 0;

   pso_desc.CachedPSO.pCachedBlob = NULL;
   pso_desc.CachedPSO.CachedBlobSizeInBytes = 0;

   pso_desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

   ID3D12PipelineState *ret;
   if (FAILED(screen->dev->CreateComputePipelineState(&pso_desc,
                                                      IID_PPV_ARGS(&ret)))) {
      debug_printf("D3D12: CreateComputePipelineState failed!\n");
      return NULL;
   }

   return ret;
}

static uint32_t
hash_compute_pipeline_state(const void *key)
{
   return _mesa_hash_data(key, sizeof(struct d3d12_compute_pipeline_state));
}

static bool
equals_compute_pipeline_state(const void *a, const void *b)
{
   return memcmp(a, b, sizeof(struct d3d12_compute_pipeline_state)) == 0;
}

ID3D12PipelineState *
d3d12_get_compute_pipeline_state(struct d3d12_context *ctx)
{
   uint32_t hash = hash_compute_pipeline_state(&ctx->compute_pipeline_state);
   struct hash_entry *entry = _mesa_hash_table_search_pre_hashed(ctx->compute_pso_cache, hash,
                                                                 &ctx->compute_pipeline_state);
   if (!entry) {
      struct d3d12_compute_pso_entry *data = (struct d3d12_compute_pso_entry *)MALLOC(sizeof(struct d3d12_compute_pso_entry));
      if (!data)
         return NULL;

      data->key = ctx->compute_pipeline_state;
      data->pso = create_compute_pipeline_state(ctx);
      if (!data->pso) {
         FREE(data);
         return NULL;
      }

      entry = _mesa_hash_table_insert_pre_hashed(ctx->compute_pso_cache, hash, &data->key, data);
      assert(entry);
   }

   return ((struct d3d12_compute_pso_entry *)(entry->data))->pso;
}

void
d3d12_compute_pipeline_state_cache_init(struct d3d12_context *ctx)
{
   ctx->compute_pso_cache = _mesa_hash_table_create(NULL, NULL, equals_compute_pipeline_state);
}

static void
delete_compute_entry(struct hash_entry *entry)
{
   struct d3d12_compute_pso_entry *data = (struct d3d12_compute_pso_entry *)entry->data;
   data->pso->Release();
   FREE(data);
}

static void
remove_compute_entry(struct d3d12_context *ctx, struct hash_entry *entry)
{
   struct d3d12_compute_pso_entry *data = (struct d3d12_compute_pso_entry *)entry->data;

   if (ctx->current_compute_pso == data->pso)
      ctx->current_compute_pso = NULL;
   _mesa_hash_table_remove(ctx->compute_pso_cache, entry);
   delete_compute_entry(entry);
}

void
d3d12_compute_pipeline_state_cache_destroy(struct d3d12_context *ctx)
{
   _mesa_hash_table_destroy(ctx->compute_pso_cache, delete_compute_entry);
}

void
d3d12_compute_pipeline_state_cache_invalidate_shader(struct d3d12_context *ctx,
                                                     struct d3d12_shader_selector *selector)
{
   struct d3d12_shader *shader = selector->first;

   while (shader) {
      hash_table_foreach(ctx->compute_pso_cache, entry) {
         const struct d3d12_compute_pipeline_state *key = (struct d3d12_compute_pipeline_state *)entry->key;
         if (key->stage == shader)
            remove_compute_entry(ctx, entry);
      }
      shader = shader->next_variant;
   }
}
