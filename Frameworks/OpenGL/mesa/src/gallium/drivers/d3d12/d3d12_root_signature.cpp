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

#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

#include "d3d12_root_signature.h"
#include "d3d12_compiler.h"
#include "d3d12_screen.h"

#include "util/u_memory.h"

#include <dxguids/dxguids.h>

struct d3d12_root_signature {
   struct d3d12_root_signature_key key;
   ID3D12RootSignature *sig;
};

static D3D12_SHADER_VISIBILITY
get_shader_visibility(enum pipe_shader_type stage)
{
   switch (stage) {
   case PIPE_SHADER_VERTEX:
      return D3D12_SHADER_VISIBILITY_VERTEX;
   case PIPE_SHADER_FRAGMENT:
      return D3D12_SHADER_VISIBILITY_PIXEL;
   case PIPE_SHADER_GEOMETRY:
      return D3D12_SHADER_VISIBILITY_GEOMETRY;
   case PIPE_SHADER_TESS_CTRL:
      return D3D12_SHADER_VISIBILITY_HULL;
   case PIPE_SHADER_TESS_EVAL:
      return D3D12_SHADER_VISIBILITY_DOMAIN;
   case PIPE_SHADER_COMPUTE:
      return D3D12_SHADER_VISIBILITY_ALL;
   default:
      unreachable("unknown shader stage");
   }
}

static inline void
init_constant_root_param(D3D12_ROOT_PARAMETER1 *param,
                         unsigned reg,
                         unsigned size,
                         D3D12_SHADER_VISIBILITY visibility)
{
   param->ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
   param->ShaderVisibility = visibility;
   param->Constants.RegisterSpace = 0;
   param->Constants.ShaderRegister = reg;
   param->Constants.Num32BitValues = size;
}

static inline void
init_range(D3D12_DESCRIPTOR_RANGE1 *range,
           D3D12_DESCRIPTOR_RANGE_TYPE type,
           uint32_t num_descs,
           uint32_t base_shader_register,
           uint32_t register_space,
           uint32_t offset_from_start)
{
   range->RangeType = type;
   range->NumDescriptors = num_descs;
   range->BaseShaderRegister = base_shader_register;
   range->RegisterSpace = register_space;
#ifdef _GAMING_XBOX
   range->Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;
#else
   if (type == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER ||
       type == D3D12_DESCRIPTOR_RANGE_TYPE_UAV)
      range->Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;
   else
      range->Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_STATIC_KEEPING_BUFFER_BOUNDS_CHECKS;
#endif
   range->OffsetInDescriptorsFromTableStart = offset_from_start;
}

static inline void
init_range_root_param(D3D12_ROOT_PARAMETER1 *param,
                      D3D12_DESCRIPTOR_RANGE1 *range,
                      D3D12_DESCRIPTOR_RANGE_TYPE type,
                      uint32_t num_descs,
                      D3D12_SHADER_VISIBILITY visibility,
                      uint32_t base_shader_register,
                      uint32_t register_space)
{
   init_range(range, type, num_descs, base_shader_register, register_space, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
   param->ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
   param->DescriptorTable.NumDescriptorRanges = 1;
   param->DescriptorTable.pDescriptorRanges = range;
   param->ShaderVisibility = visibility;
}

static ID3D12RootSignature *
create_root_signature(struct d3d12_context *ctx, struct d3d12_root_signature_key *key)
{
   struct d3d12_screen *screen = d3d12_screen(ctx->base.screen);
   D3D12_ROOT_PARAMETER1 root_params[D3D12_GFX_SHADER_STAGES * D3D12_NUM_BINDING_TYPES];
   D3D12_DESCRIPTOR_RANGE1 desc_ranges[D3D12_GFX_SHADER_STAGES * (D3D12_NUM_BINDING_TYPES + 1)];
   unsigned num_params = 0;
   unsigned num_ranges = 0;

   unsigned count = key->compute ? 1 : D3D12_GFX_SHADER_STAGES;
   for (unsigned i = 0; i < count; ++i) {
      unsigned stage = key->compute ? PIPE_SHADER_COMPUTE : i;
      D3D12_SHADER_VISIBILITY visibility = get_shader_visibility((enum pipe_shader_type)stage);

      if (key->stages[i].num_cb_bindings > 0) {
         init_range_root_param(&root_params[num_params++],
                               &desc_ranges[num_ranges++],
                               D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
                               key->stages[i].num_cb_bindings,
                               visibility,
                               key->stages[i].has_default_ubo0 ? 0 : 1,
                               0);
      }

      if (key->stages[i].end_srv_binding > 0) {
         init_range_root_param(&root_params[num_params++],
                               &desc_ranges[num_ranges++],
                               D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                               key->stages[i].end_srv_binding - key->stages[i].begin_srv_binding,
                               visibility,
                               key->stages[i].begin_srv_binding,
                               0);

         init_range_root_param(&root_params[num_params++],
                               &desc_ranges[num_ranges++],
                               D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER,
                               key->stages[i].end_srv_binding - key->stages[i].begin_srv_binding,
                               visibility,
                               key->stages[i].begin_srv_binding,
                               0);
      }

      if (key->stages[i].num_ssbos > 0) {
         init_range_root_param(&root_params[num_params],
                               &desc_ranges[num_ranges++],
                               D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
                               key->stages[i].num_ssbos,
                               visibility,
                               0,
                               0);

         /* To work around a WARP bug, bind these descriptors a second time in descriptor
          * space 2. Space 0 will be used for static indexing, while space 2 will be used
          * for dynamic indexing. Space 0 will be individual SSBOs in the DXIL shader, while
          * space 2 will be a single array.
          */
         root_params[num_params++].DescriptorTable.NumDescriptorRanges++;
         init_range(&desc_ranges[num_ranges++],
                    D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
                    key->stages[i].num_ssbos,
                    0,
                    2,
                    0);
      }

      if (key->stages[i].num_images > 0) {
         init_range_root_param(&root_params[num_params++],
                               &desc_ranges[num_ranges++],
                               D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
                               key->stages[i].num_images,
                               visibility,
                               0,
                               1);
      }

      if (key->stages[i].state_vars_size > 0) {
         init_constant_root_param(&root_params[num_params++],
            key->stages[i].num_cb_bindings + (key->stages[i].has_default_ubo0 ? 0 : 1),
            key->stages[i].state_vars_size,
            visibility);
      }
      assert(num_params < PIPE_SHADER_TYPES * D3D12_NUM_BINDING_TYPES);
      assert(num_ranges < PIPE_SHADER_TYPES * (D3D12_NUM_BINDING_TYPES + 1));
   }

   D3D12_VERSIONED_ROOT_SIGNATURE_DESC root_sig_desc;
   root_sig_desc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
   root_sig_desc.Desc_1_1.NumParameters = num_params;
   root_sig_desc.Desc_1_1.pParameters = (num_params > 0) ? root_params : NULL;
   root_sig_desc.Desc_1_1.NumStaticSamplers = 0;
   root_sig_desc.Desc_1_1.pStaticSamplers = NULL;
   root_sig_desc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

   /* TODO Only enable this flag when needed (optimization) */
   if (!key->compute)
      root_sig_desc.Desc_1_1.Flags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

   if (key->has_stream_output)
      root_sig_desc.Desc_1_1.Flags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT;

   ComPtr<ID3DBlob> sig, error;
#ifndef _GAMING_XBOX
   if (ctx->dev_config) {
      if (FAILED(ctx->dev_config->SerializeVersionedRootSignature(&root_sig_desc,
                                                                  &sig, &error))) {
         debug_printf("D3D12SerializeRootSignature failed\n");
         return NULL;
      }
   } else
#endif
   {
      if (FAILED(ctx->D3D12SerializeVersionedRootSignature(&root_sig_desc,
                                                           &sig, &error))) {
         debug_printf("D3D12SerializeRootSignature failed\n");
         return NULL;
      }
   }

   ID3D12RootSignature *ret;
   if (FAILED(screen->dev->CreateRootSignature(0,
                                               sig->GetBufferPointer(),
                                               sig->GetBufferSize(),
                                               IID_PPV_ARGS(&ret)))) {
      debug_printf("CreateRootSignature failed\n");
      return NULL;
   }
   return ret;
}

static void
fill_key(struct d3d12_context *ctx, struct d3d12_root_signature_key *key, bool compute)
{
   memset(key, 0, sizeof(struct d3d12_root_signature_key));

   key->compute = compute;
   unsigned count = compute ? 1 : D3D12_GFX_SHADER_STAGES;
   for (unsigned i = 0; i < count; ++i) {
      struct d3d12_shader *shader = compute ?
         ctx->compute_pipeline_state.stage :
         ctx->gfx_pipeline_state.stages[i];

      if (shader) {
         key->stages[i].num_cb_bindings = shader->num_cb_bindings;
         key->stages[i].end_srv_binding = shader->end_srv_binding;
         key->stages[i].begin_srv_binding = shader->begin_srv_binding;
         key->stages[i].state_vars_size = shader->state_vars_size;
         key->stages[i].has_default_ubo0 = shader->has_default_ubo0;
         key->stages[i].num_ssbos = shader->nir->info.num_ssbos;
         key->stages[i].num_images = shader->nir->info.num_images;

         if (!compute && ctx->gfx_stages[i]->so_info.num_outputs > 0)
            key->has_stream_output = true;
      }
   }
}

ID3D12RootSignature *
d3d12_get_root_signature(struct d3d12_context *ctx, bool compute)
{
   struct d3d12_root_signature_key key;

   fill_key(ctx, &key, compute);
   struct hash_entry *entry = _mesa_hash_table_search(ctx->root_signature_cache, &key);
   if (!entry) {
      struct d3d12_root_signature *data =
         (struct d3d12_root_signature *)MALLOC(sizeof(struct d3d12_root_signature));
      if (!data)
         return NULL;

      data->key = key;
      data->sig = create_root_signature(ctx, &key);
      if (!data->sig) {
         FREE(data);
         return NULL;
      }

      entry = _mesa_hash_table_insert(ctx->root_signature_cache, &data->key, data);
      assert(entry);
   }

   return ((struct d3d12_root_signature *)entry->data)->sig;
}

static uint32_t
hash_root_signature_key(const void *key)
{
   return _mesa_hash_data(key, sizeof(struct d3d12_root_signature_key));
}

static bool
equals_root_signature_key(const void *a, const void *b)
{
   return memcmp(a, b, sizeof(struct d3d12_root_signature_key)) == 0;
}

void
d3d12_root_signature_cache_init(struct d3d12_context *ctx)
{
   ctx->root_signature_cache = _mesa_hash_table_create(NULL,
                                                       hash_root_signature_key,
                                                       equals_root_signature_key);
}

static void
delete_entry(struct hash_entry *entry)
{
   struct d3d12_root_signature *data = (struct d3d12_root_signature *)entry->data;
   data->sig->Release();
   FREE(data);
}

void
d3d12_root_signature_cache_destroy(struct d3d12_context *ctx)
{
   _mesa_hash_table_destroy(ctx->root_signature_cache, delete_entry);
}
