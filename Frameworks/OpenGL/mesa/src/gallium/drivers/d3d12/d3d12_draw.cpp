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

#include "d3d12_cmd_signature.h"
#include "d3d12_compiler.h"
#include "d3d12_compute_transforms.h"
#include "d3d12_context.h"
#include "d3d12_format.h"
#include "d3d12_query.h"
#include "d3d12_resource.h"
#include "d3d12_root_signature.h"
#include "d3d12_screen.h"
#include "d3d12_surface.h"

#include "indices/u_primconvert.h"
#include "util/u_debug.h"
#include "util/u_draw.h"
#include "util/u_helpers.h"
#include "util/u_inlines.h"
#include "util/u_prim.h"
#include "util/u_prim_restart.h"
#include "util/u_math.h"

static const D3D12_RECT MAX_SCISSOR = { 0, 0, 16384, 16384 };

static const D3D12_RECT MAX_SCISSOR_ARRAY[] = {
   MAX_SCISSOR, MAX_SCISSOR, MAX_SCISSOR, MAX_SCISSOR,
   MAX_SCISSOR, MAX_SCISSOR, MAX_SCISSOR, MAX_SCISSOR,
   MAX_SCISSOR, MAX_SCISSOR, MAX_SCISSOR, MAX_SCISSOR,
   MAX_SCISSOR, MAX_SCISSOR, MAX_SCISSOR, MAX_SCISSOR
};
static_assert(ARRAY_SIZE(MAX_SCISSOR_ARRAY) == PIPE_MAX_VIEWPORTS, "Wrong scissor count");

static D3D12_GPU_DESCRIPTOR_HANDLE
fill_cbv_descriptors(struct d3d12_context *ctx,
                     struct d3d12_shader *shader,
                     int stage)
{
   struct d3d12_batch *batch = d3d12_current_batch(ctx);
   struct d3d12_descriptor_handle table_start;
   d2d12_descriptor_heap_get_next_handle(batch->view_heap, &table_start);

   for (unsigned i = 0; i < shader->num_cb_bindings; i++) {
      unsigned binding = shader->cb_bindings[i].binding;
      struct pipe_constant_buffer *buffer = &ctx->cbufs[stage][binding];

      D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc = {};
      if (buffer && buffer->buffer) {
         struct d3d12_resource *res = d3d12_resource(buffer->buffer);
         d3d12_transition_resource_state(ctx, res, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_TRANSITION_FLAG_ACCUMULATE_STATE);
         cbv_desc.BufferLocation = d3d12_resource_gpu_virtual_address(res) + buffer->buffer_offset;
         cbv_desc.SizeInBytes = MIN2(D3D12_REQ_CONSTANT_BUFFER_ELEMENT_COUNT * 16,
            align(buffer->buffer_size, 256));
         d3d12_batch_reference_resource(batch, res, false);
      }

      struct d3d12_descriptor_handle handle;
      d3d12_descriptor_heap_alloc_handle(batch->view_heap, &handle);
      d3d12_screen(ctx->base.screen)->dev->CreateConstantBufferView(&cbv_desc, handle.cpu_handle);
   }

   return table_start.gpu_handle;
}

static D3D12_GPU_DESCRIPTOR_HANDLE
fill_srv_descriptors(struct d3d12_context *ctx,
                     struct d3d12_shader *shader,
                     unsigned stage)
{
   struct d3d12_batch *batch = d3d12_current_batch(ctx);
   struct d3d12_screen *screen = d3d12_screen(ctx->base.screen);
   D3D12_CPU_DESCRIPTOR_HANDLE descs[PIPE_MAX_SHADER_SAMPLER_VIEWS];
   struct d3d12_descriptor_handle table_start;

   d2d12_descriptor_heap_get_next_handle(batch->view_heap, &table_start);

   for (unsigned i = shader->begin_srv_binding; i < shader->end_srv_binding; i++)
   {
      struct d3d12_sampler_view *view;

      if (i == shader->pstipple_binding) {
         view = (struct d3d12_sampler_view*)ctx->pstipple.sampler_view;
      } else {
         view = (struct d3d12_sampler_view*)ctx->sampler_views[stage][i];
      }

      unsigned desc_idx = i - shader->begin_srv_binding;
      if (view != NULL) {
         descs[desc_idx] = view->handle.cpu_handle;
         d3d12_batch_reference_sampler_view(batch, view);

         struct d3d12_resource *res = d3d12_resource(view->base.texture);
         /* If this is a buffer that's been replaced, re-create the descriptor */
         if (view->texture_generation_id != res->generation_id) {
            d3d12_init_sampler_view_descriptor(view);
            view->texture_generation_id = res->generation_id;
         }

         D3D12_RESOURCE_STATES state = (stage == PIPE_SHADER_FRAGMENT) ?
                                       D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE :
                                       D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
         if (view->base.texture->target == PIPE_BUFFER) {
            d3d12_transition_resource_state(ctx, d3d12_resource(view->base.texture),
                                            state,
                                            D3D12_TRANSITION_FLAG_ACCUMULATE_STATE);
         } else {
            d3d12_transition_subresources_state(ctx, d3d12_resource(view->base.texture),
                                                view->base.u.tex.first_level, view->mip_levels,
                                                view->base.u.tex.first_layer, view->array_size,
                                                d3d12_get_format_start_plane(view->base.format),
                                                d3d12_get_format_num_planes(view->base.format),
                                                state,
                                                D3D12_TRANSITION_FLAG_ACCUMULATE_STATE);
         }
      } else {
         descs[desc_idx] = screen->null_srvs[shader->srv_bindings[i].dimension].cpu_handle;
      }
   }

   d3d12_descriptor_heap_append_handles(batch->view_heap, descs, shader->end_srv_binding - shader->begin_srv_binding);

   return table_start.gpu_handle;
}

static D3D12_GPU_DESCRIPTOR_HANDLE
fill_ssbo_descriptors(struct d3d12_context *ctx,
                     const struct d3d12_shader *shader,
                     int stage)
{
   struct d3d12_batch *batch = d3d12_current_batch(ctx);
   struct d3d12_descriptor_handle table_start;

   d2d12_descriptor_heap_get_next_handle(batch->view_heap, &table_start);

   for (unsigned i = 0; i < shader->nir->info.num_ssbos; i++)
   {
      struct pipe_shader_buffer *view = &ctx->ssbo_views[stage][i];

      D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc;
      uav_desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
      uav_desc.Format = DXGI_FORMAT_R32_TYPELESS;
      uav_desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
      uav_desc.Buffer.StructureByteStride = 0;
      uav_desc.Buffer.CounterOffsetInBytes = 0;
      uav_desc.Buffer.FirstElement = 0;
      uav_desc.Buffer.NumElements = 0;
      ID3D12Resource *d3d12_res = nullptr;
      if (view->buffer) {
         struct d3d12_resource *res = d3d12_resource(view->buffer);
         uint64_t res_offset = 0;
         d3d12_res = d3d12_resource_underlying(res, &res_offset);
         d3d12_transition_resource_state(ctx, res, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_TRANSITION_FLAG_ACCUMULATE_STATE);
         uav_desc.Buffer.FirstElement = (view->buffer_offset + res_offset) / 4;
         uav_desc.Buffer.NumElements = DIV_ROUND_UP(view->buffer_size, 4);
         d3d12_batch_reference_resource(batch, res, true);
      }

      struct d3d12_descriptor_handle handle;
      d3d12_descriptor_heap_alloc_handle(batch->view_heap, &handle);
      d3d12_screen(ctx->base.screen)->dev->CreateUnorderedAccessView(d3d12_res, nullptr, &uav_desc, handle.cpu_handle);
   }

   return table_start.gpu_handle;
}

static D3D12_GPU_DESCRIPTOR_HANDLE
fill_sampler_descriptors(struct d3d12_context *ctx,
                         const struct d3d12_shader_selector *shader_sel,
                         unsigned stage)
{
   const struct d3d12_shader *shader = shader_sel->current;
   struct d3d12_batch *batch = d3d12_current_batch(ctx);
   struct d3d12_sampler_desc_table_key view;

   view.count = 0;
   for (unsigned i = shader->begin_srv_binding; i < shader->end_srv_binding; i++, view.count++) {
      struct d3d12_sampler_state *sampler;

      if (i == shader->pstipple_binding) {
         sampler = ctx->pstipple.sampler_cso;
      } else {
         sampler = ctx->samplers[stage][i];
      }

      unsigned desc_idx = i - shader->begin_srv_binding;
      if (sampler != NULL) {
         if (sampler->is_shadow_sampler && shader_sel->compare_with_lod_bias_grad)
            view.descs[desc_idx] = sampler->handle_without_shadow.cpu_handle;
         else
            view.descs[desc_idx] = sampler->handle.cpu_handle;
      } else
         view.descs[desc_idx] = ctx->null_sampler.cpu_handle;
   }

   hash_entry* sampler_entry =
      (hash_entry*)_mesa_hash_table_search(batch->sampler_tables, &view);

   if (!sampler_entry) {
      d3d12_sampler_desc_table_key* sampler_table_key = MALLOC_STRUCT(d3d12_sampler_desc_table_key);
      sampler_table_key->count = view.count;
      memcpy(sampler_table_key->descs, &view.descs, view.count * sizeof(view.descs[0]));

      d3d12_descriptor_handle* sampler_table_data = MALLOC_STRUCT(d3d12_descriptor_handle);
      d2d12_descriptor_heap_get_next_handle(batch->sampler_heap, sampler_table_data);

      d3d12_descriptor_heap_append_handles(batch->sampler_heap, view.descs, shader->end_srv_binding - shader->begin_srv_binding);

      _mesa_hash_table_insert(batch->sampler_tables, sampler_table_key, sampler_table_data);

      return sampler_table_data->gpu_handle;
   } else
      return ((d3d12_descriptor_handle*)sampler_entry->data)->gpu_handle;

}

static D3D12_UAV_DIMENSION
image_view_dimension(enum pipe_texture_target target)
{
   switch (target) {
   case PIPE_BUFFER: return D3D12_UAV_DIMENSION_BUFFER;
   case PIPE_TEXTURE_1D: return D3D12_UAV_DIMENSION_TEXTURE1D;
   case PIPE_TEXTURE_1D_ARRAY: return D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
   case PIPE_TEXTURE_RECT:
   case PIPE_TEXTURE_2D:
      return D3D12_UAV_DIMENSION_TEXTURE2D;
   case PIPE_TEXTURE_2D_ARRAY:
   case PIPE_TEXTURE_CUBE:
   case PIPE_TEXTURE_CUBE_ARRAY:
      return D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
   case PIPE_TEXTURE_3D: return D3D12_UAV_DIMENSION_TEXTURE3D;
   default:
      unreachable("unexpected target");
   }
}

static D3D12_GPU_DESCRIPTOR_HANDLE
fill_image_descriptors(struct d3d12_context *ctx,
                       const struct d3d12_shader *shader,
                       int stage)
{
   struct d3d12_screen *screen = d3d12_screen(ctx->base.screen);
   struct d3d12_batch *batch = d3d12_current_batch(ctx);
   struct d3d12_descriptor_handle table_start;

   d2d12_descriptor_heap_get_next_handle(batch->view_heap, &table_start);

   for (unsigned i = 0; i < shader->nir->info.num_images; i++)
   {
      struct pipe_image_view *view = &ctx->image_views[stage][i];

      if (view->resource) {
         D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc;
         struct d3d12_resource *res = d3d12_resource(view->resource);
         uint64_t offset = 0;
         ID3D12Resource *d3d12_res = d3d12_resource_underlying(res, &offset);

         enum pipe_format view_format = ctx->image_view_emulation_formats[stage][i];
         if (view_format == PIPE_FORMAT_NONE)
            view_format = view->format;
         uav_desc.Format = d3d12_get_format(view_format);
         uav_desc.ViewDimension = image_view_dimension(res->base.b.target);

         unsigned array_size = view->u.tex.last_layer - view->u.tex.first_layer + 1;
         switch (uav_desc.ViewDimension) {
         case D3D12_UAV_DIMENSION_TEXTURE1D:
            if (view->u.tex.first_layer > 0)
               debug_printf("D3D12: can't create 1D UAV from layer %d\n",
                            view->u.tex.first_layer);
            uav_desc.Texture1D.MipSlice = view->u.tex.level;
            break;
         case D3D12_UAV_DIMENSION_TEXTURE1DARRAY:
            uav_desc.Texture1DArray.FirstArraySlice = view->u.tex.first_layer;
            uav_desc.Texture1DArray.ArraySize = array_size;
            uav_desc.Texture1DArray.MipSlice = view->u.tex.level;
            break;
         case D3D12_UAV_DIMENSION_TEXTURE2D:
            if (view->u.tex.first_layer > 0)
               debug_printf("D3D12: can't create 2D UAV from layer %d\n",
                            view->u.tex.first_layer);
            uav_desc.Texture2D.MipSlice = view->u.tex.level;
            uav_desc.Texture2D.PlaneSlice = 0;
            break;
         case D3D12_UAV_DIMENSION_TEXTURE2DARRAY:
            uav_desc.Texture2DArray.FirstArraySlice = view->u.tex.first_layer;
            uav_desc.Texture2DArray.ArraySize = array_size;
            uav_desc.Texture2DArray.MipSlice = view->u.tex.level;
            uav_desc.Texture2DArray.PlaneSlice = 0;
            break;
         case D3D12_UAV_DIMENSION_TEXTURE3D:
            uav_desc.Texture3D.MipSlice = view->u.tex.level;
            uav_desc.Texture3D.FirstWSlice = view->u.tex.first_layer;
            uav_desc.Texture3D.WSize = array_size;
            break;
         case D3D12_UAV_DIMENSION_BUFFER: {
            uint format_size = util_format_get_blocksize(view_format);
            offset += view->u.buf.offset;
            uav_desc.Buffer.CounterOffsetInBytes = 0;
            uav_desc.Buffer.FirstElement = offset / format_size;
            uav_desc.Buffer.NumElements = MIN2(view->u.buf.size / format_size,
                                               1 << D3D12_REQ_BUFFER_RESOURCE_TEXEL_COUNT_2_TO_EXP);
            uav_desc.Buffer.StructureByteStride = 0;
            uav_desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
            break;
         }
         default:
            unreachable("Unexpected image view dimension");
         }
         
         d3d12_transition_flags transition_flags = (d3d12_transition_flags)(D3D12_TRANSITION_FLAG_ACCUMULATE_STATE |
            (batch->pending_memory_barrier ? D3D12_TRANSITION_FLAG_PENDING_MEMORY_BARRIER : 0));
         if (res->base.b.target == PIPE_BUFFER) {
            d3d12_transition_resource_state(ctx, res, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, transition_flags);
         } else {
            unsigned transition_first_layer = view->u.tex.first_layer;
            unsigned transition_array_size = array_size;
            if (res->base.b.target == PIPE_TEXTURE_3D) {
               transition_first_layer = 0;
               transition_array_size = 0;
            }
            d3d12_transition_subresources_state(ctx, res,
                                                view->u.tex.level, 1,
                                                transition_first_layer, transition_array_size,
                                                0, 1,
                                                D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                                                transition_flags);
         }
         d3d12_batch_reference_resource(batch, res, true);

         struct d3d12_descriptor_handle handle;
         d3d12_descriptor_heap_alloc_handle(batch->view_heap, &handle);
         d3d12_screen(ctx->base.screen)->dev->CreateUnorderedAccessView(d3d12_res, nullptr, &uav_desc, handle.cpu_handle);
      } else {
         d3d12_descriptor_heap_append_handles(batch->view_heap, &screen->null_uavs[shader->uav_bindings[i].dimension].cpu_handle, 1);
      }
   }

   return table_start.gpu_handle;
}

static unsigned
fill_graphics_state_vars(struct d3d12_context *ctx,
                         const struct pipe_draw_info *dinfo,
                         unsigned drawid,
                         const struct pipe_draw_start_count_bias *draw,
                         struct d3d12_shader *shader,
                         uint32_t *values,
                         unsigned cur_root_param_idx,
                         struct d3d12_cmd_signature_key *cmd_sig_key)
{
   unsigned size = 0;

   for (unsigned j = 0; j < shader->num_state_vars; ++j) {
      uint32_t *ptr = values + size;

      switch (shader->state_vars[j].var) {
      case D3D12_STATE_VAR_Y_FLIP:
         ptr[0] = fui(ctx->flip_y);
         size += 4;
         break;
      case D3D12_STATE_VAR_PT_SPRITE:
         ptr[0] = fui(1.0 / ctx->viewports[0].Width);
         ptr[1] = fui(1.0 / ctx->viewports[0].Height);
         ptr[2] = fui(ctx->gfx_pipeline_state.rast->base.point_size);
         ptr[3] = fui(D3D12_MAX_POINT_SIZE);
         size += 4;
         break;
      case D3D12_STATE_VAR_DRAW_PARAMS:
         ptr[0] = dinfo->index_size ? draw->index_bias : draw->start;
         ptr[1] = dinfo->start_instance;
         ptr[2] = drawid;
         ptr[3] = dinfo->index_size ? -1 : 0;
         assert(!cmd_sig_key->draw_or_dispatch_params); // Should only be set once
         cmd_sig_key->draw_or_dispatch_params = 1;
         cmd_sig_key->root_sig = ctx->gfx_pipeline_state.root_signature;
         cmd_sig_key->params_root_const_offset = size;
         cmd_sig_key->params_root_const_param = cur_root_param_idx;
         size += 4;
         break;
      case D3D12_STATE_VAR_DEPTH_TRANSFORM:
         ptr[0] = fui(2.0f * ctx->viewport_states[0].scale[2]);
         ptr[1] = fui(ctx->viewport_states[0].translate[2] - ctx->viewport_states[0].scale[2]);
         size += 4;
         break;
      case D3D12_STATE_VAR_DEFAULT_INNER_TESS_LEVEL:
         memcpy(ptr, ctx->default_inner_tess_factor, sizeof(ctx->default_inner_tess_factor));
         size += 4;
         break;
      case D3D12_STATE_VAR_DEFAULT_OUTER_TESS_LEVEL:
         memcpy(ptr, ctx->default_outer_tess_factor, sizeof(ctx->default_outer_tess_factor));
         size += 4;
         break;
      case D3D12_STATE_VAR_PATCH_VERTICES_IN:
         ptr[0] = ctx->patch_vertices;
         size += 4;
         break;
      default:
         unreachable("unknown state variable");
      }
   }

   return size;
}

static unsigned
fill_compute_state_vars(struct d3d12_context *ctx,
                        const struct pipe_grid_info *info,
                        struct d3d12_shader *shader,
                        uint32_t *values,
                        struct d3d12_cmd_signature_key *cmd_sig_key)
{
   unsigned size = 0;

   for (unsigned j = 0; j < shader->num_state_vars; ++j) {
      uint32_t *ptr = values + size;

      switch (shader->state_vars[j].var) {
      case D3D12_STATE_VAR_NUM_WORKGROUPS:
         ptr[0] = info->grid[0];
         ptr[1] = info->grid[1];
         ptr[2] = info->grid[2];
         cmd_sig_key->draw_or_dispatch_params = 1;
         cmd_sig_key->root_sig = ctx->compute_pipeline_state.root_signature;
         cmd_sig_key->params_root_const_offset = size;
         size += 4;
         break;
      case D3D12_STATE_VAR_TRANSFORM_GENERIC0:
      case D3D12_STATE_VAR_TRANSFORM_GENERIC1: {
         unsigned idx = shader->state_vars[j].var - D3D12_STATE_VAR_TRANSFORM_GENERIC0;
         ptr[0] = ctx->transform_state_vars[idx * 4];
         ptr[1] = ctx->transform_state_vars[idx * 4 + 1];
         ptr[2] = ctx->transform_state_vars[idx * 4 + 2];
         ptr[3] = ctx->transform_state_vars[idx * 4 + 3];
         size += 4;
         break;
      }
      default:
         unreachable("unknown state variable");
      }
   }

   return size;
}

static bool
check_descriptors_left(struct d3d12_context *ctx, bool compute)
{
   struct d3d12_batch *batch = d3d12_current_batch(ctx);
   unsigned needed_descs = 0;

   unsigned count = compute ? 1 : D3D12_GFX_SHADER_STAGES;
   for (unsigned i = 0; i < count; ++i) {
      struct d3d12_shader_selector *shader = compute ? ctx->compute_state : ctx->gfx_stages[i];

      if (!shader)
         continue;

      needed_descs += shader->current->num_cb_bindings;
      needed_descs += shader->current->end_srv_binding - shader->current->begin_srv_binding;
      needed_descs += shader->current->nir->info.num_ssbos;
      needed_descs += shader->current->nir->info.num_images;
   }

   if (d3d12_descriptor_heap_get_remaining_handles(batch->view_heap) < needed_descs)
      return false;

   needed_descs = 0;
   for (unsigned i = 0; i < count; ++i) {
      struct d3d12_shader_selector *shader = compute ? ctx->compute_state : ctx->gfx_stages[i];

      if (!shader)
         continue;

      needed_descs += shader->current->end_srv_binding - shader->current->begin_srv_binding;
   }

   if (d3d12_descriptor_heap_get_remaining_handles(batch->sampler_heap) < needed_descs)
      return false;

   return true;
}

#define MAX_DESCRIPTOR_TABLES (D3D12_GFX_SHADER_STAGES * 4)

static void
update_shader_stage_root_parameters(struct d3d12_context *ctx,
                                    const struct d3d12_shader_selector *shader_sel,
                                    unsigned &num_params,
                                    unsigned &num_root_descriptors,
                                    D3D12_GPU_DESCRIPTOR_HANDLE root_desc_tables[MAX_DESCRIPTOR_TABLES],
                                    int root_desc_indices[MAX_DESCRIPTOR_TABLES])
{
   auto stage = shader_sel->stage;
   struct d3d12_shader *shader = shader_sel->current;
   uint64_t dirty = ctx->shader_dirty[stage];
   assert(shader);

   if (shader->num_cb_bindings > 0) {
      if (dirty & D3D12_SHADER_DIRTY_CONSTBUF) {
         assert(num_root_descriptors < MAX_DESCRIPTOR_TABLES);
         root_desc_tables[num_root_descriptors] = fill_cbv_descriptors(ctx, shader, stage);
         root_desc_indices[num_root_descriptors++] = num_params;
      }
      num_params++;
   }
   if (shader->end_srv_binding > 0) {
      if (dirty & D3D12_SHADER_DIRTY_SAMPLER_VIEWS) {
         assert(num_root_descriptors < MAX_DESCRIPTOR_TABLES);
         root_desc_tables[num_root_descriptors] = fill_srv_descriptors(ctx, shader, stage);
         root_desc_indices[num_root_descriptors++] = num_params;
      }
      num_params++;
      if (dirty & D3D12_SHADER_DIRTY_SAMPLERS) {
         assert(num_root_descriptors < MAX_DESCRIPTOR_TABLES);
         root_desc_tables[num_root_descriptors] = fill_sampler_descriptors(ctx, shader_sel, stage);
         root_desc_indices[num_root_descriptors++] = num_params;
      }
      num_params++;
   }
   if (shader->nir->info.num_ssbos > 0) {
      if (dirty & D3D12_SHADER_DIRTY_SSBO) {
         assert(num_root_descriptors < MAX_DESCRIPTOR_TABLES);
         root_desc_tables[num_root_descriptors] = fill_ssbo_descriptors(ctx, shader, stage);
         root_desc_indices[num_root_descriptors++] = num_params;
      }
      num_params++;
   }
   if (shader->nir->info.num_images > 0) {
      if (dirty & D3D12_SHADER_DIRTY_IMAGE) {
         assert(num_root_descriptors < MAX_DESCRIPTOR_TABLES);
         root_desc_tables[num_root_descriptors] = fill_image_descriptors(ctx, shader, stage);
         root_desc_indices[num_root_descriptors++] = num_params;
      }
      num_params++;
   }
}

static unsigned
update_graphics_root_parameters(struct d3d12_context *ctx,
                                const struct pipe_draw_info *dinfo,
                                unsigned drawid,
                                const struct pipe_draw_start_count_bias *draw,
                                D3D12_GPU_DESCRIPTOR_HANDLE root_desc_tables[MAX_DESCRIPTOR_TABLES],
                                int root_desc_indices[MAX_DESCRIPTOR_TABLES],
                                struct d3d12_cmd_signature_key *cmd_sig_key)
{
   unsigned num_params = 0;
   unsigned num_root_descriptors = 0;

   for (unsigned i = 0; i < D3D12_GFX_SHADER_STAGES; ++i) {
      struct d3d12_shader_selector *shader_sel = ctx->gfx_stages[i];
      if (!shader_sel)
         continue;

      update_shader_stage_root_parameters(ctx, shader_sel, num_params, num_root_descriptors, root_desc_tables, root_desc_indices);
      /* TODO Don't always update state vars */
      if (shader_sel->current->num_state_vars > 0) {
         uint32_t constants[D3D12_MAX_GRAPHICS_STATE_VARS * 4];
         unsigned size = fill_graphics_state_vars(ctx, dinfo, drawid, draw, shader_sel->current, constants, num_params, cmd_sig_key);
         ctx->cmdlist->SetGraphicsRoot32BitConstants(num_params, size, constants, 0);
         num_params++;
      }
   }
   return num_root_descriptors;
}

static unsigned
update_compute_root_parameters(struct d3d12_context *ctx,
                               const struct pipe_grid_info *info,
                               D3D12_GPU_DESCRIPTOR_HANDLE root_desc_tables[MAX_DESCRIPTOR_TABLES],
                               int root_desc_indices[MAX_DESCRIPTOR_TABLES],
                               struct d3d12_cmd_signature_key *cmd_sig_key)
{
   unsigned num_params = 0;
   unsigned num_root_descriptors = 0;

   struct d3d12_shader_selector *shader_sel = ctx->compute_state;
   if (shader_sel) {
      update_shader_stage_root_parameters(ctx, shader_sel, num_params, num_root_descriptors, root_desc_tables, root_desc_indices);
      /* TODO Don't always update state vars */
      if (shader_sel->current->num_state_vars > 0) {
         uint32_t constants[D3D12_MAX_COMPUTE_STATE_VARS * 4];
         unsigned size = fill_compute_state_vars(ctx, info, shader_sel->current, constants, cmd_sig_key);
         if (cmd_sig_key->draw_or_dispatch_params)
            cmd_sig_key->params_root_const_param = num_params;
         ctx->cmdlist->SetComputeRoot32BitConstants(num_params, size, constants, 0);
         num_params++;
      }
   }
   return num_root_descriptors;
}

static bool
validate_stream_output_targets(struct d3d12_context *ctx)
{
   unsigned factor = 0;

   if (ctx->gfx_pipeline_state.num_so_targets &&
       ctx->gfx_pipeline_state.stages[PIPE_SHADER_GEOMETRY])
      factor = ctx->gfx_pipeline_state.stages[PIPE_SHADER_GEOMETRY]->key.gs.stream_output_factor;

   if (factor > 1)
      return d3d12_enable_fake_so_buffers(ctx, factor);
   else
      return d3d12_disable_fake_so_buffers(ctx);
}

static D3D_PRIMITIVE_TOPOLOGY
topology(enum mesa_prim prim_type, uint8_t patch_vertices)
{
   switch (prim_type) {
   case MESA_PRIM_POINTS:
      return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;

   case MESA_PRIM_LINES:
      return D3D_PRIMITIVE_TOPOLOGY_LINELIST;

   case MESA_PRIM_LINE_STRIP:
      return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;

   case MESA_PRIM_TRIANGLES:
      return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

   case MESA_PRIM_TRIANGLE_STRIP:
      return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

   case MESA_PRIM_LINES_ADJACENCY:
      return D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ;

   case MESA_PRIM_LINE_STRIP_ADJACENCY:
      return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ;

   case MESA_PRIM_TRIANGLES_ADJACENCY:
      return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ;

   case MESA_PRIM_TRIANGLE_STRIP_ADJACENCY:
      return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ;

   case MESA_PRIM_PATCHES:
      return (D3D_PRIMITIVE_TOPOLOGY)(D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST + patch_vertices - 1);

   case MESA_PRIM_QUADS:
   case MESA_PRIM_QUAD_STRIP:
      return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; /* HACK: this is just wrong! */

   default:
      debug_printf("mesa_prim: %s\n", u_prim_name(prim_type));
      unreachable("unexpected enum mesa_prim");
   }
}

static DXGI_FORMAT
ib_format(unsigned index_size)
{
   switch (index_size) {
   case 1: return DXGI_FORMAT_R8_UINT;
   case 2: return DXGI_FORMAT_R16_UINT;
   case 4: return DXGI_FORMAT_R32_UINT;

   default:
      unreachable("unexpected index-buffer size");
   }
}

static void
twoface_emulation(struct d3d12_context *ctx,
                  struct d3d12_rasterizer_state *rast,
                  const struct pipe_draw_info *dinfo,
                  const struct pipe_draw_indirect_info *indirect,
                  const struct pipe_draw_start_count_bias *draw)
{
   /* draw backfaces */
   ctx->base.bind_rasterizer_state(&ctx->base, rast->twoface_back);
   d3d12_draw_vbo(&ctx->base, dinfo, 0, indirect, draw, 1);

   /* restore real state */
   ctx->base.bind_rasterizer_state(&ctx->base, rast);
}

static void
transition_surface_subresources_state(struct d3d12_context *ctx,
                                      struct pipe_surface *psurf,
                                      struct pipe_resource *pres,
                                      D3D12_RESOURCE_STATES state)
{
   struct d3d12_resource *res = d3d12_resource(pres);
   unsigned start_layer, num_layers;
   if (!d3d12_subresource_id_uses_layer(res->base.b.target)) {
      start_layer = 0;
      num_layers = 1;
   } else {
      start_layer = psurf->u.tex.first_layer;
      num_layers = psurf->u.tex.last_layer - psurf->u.tex.first_layer + 1;
   }
   d3d12_transition_subresources_state(ctx, res,
                                       psurf->u.tex.level, 1,
                                       start_layer, num_layers,
                                       d3d12_get_format_start_plane(psurf->format),
                                       d3d12_get_format_num_planes(psurf->format),
                                       state,
                                       D3D12_TRANSITION_FLAG_ACCUMULATE_STATE);
}

static bool
prim_supported(enum mesa_prim prim_type)
{
   switch (prim_type) {
   case MESA_PRIM_POINTS:
   case MESA_PRIM_LINES:
   case MESA_PRIM_LINE_STRIP:
   case MESA_PRIM_TRIANGLES:
   case MESA_PRIM_TRIANGLE_STRIP:
   case MESA_PRIM_LINES_ADJACENCY:
   case MESA_PRIM_LINE_STRIP_ADJACENCY:
   case MESA_PRIM_TRIANGLES_ADJACENCY:
   case MESA_PRIM_TRIANGLE_STRIP_ADJACENCY:
   case MESA_PRIM_PATCHES:
      return true;

   default:
      return false;
   }
}

static inline struct d3d12_shader_selector *
d3d12_last_vertex_stage(struct d3d12_context *ctx)
{
   struct d3d12_shader_selector *sel = ctx->gfx_stages[PIPE_SHADER_GEOMETRY];
   if (!sel || sel->is_variant)
      sel = ctx->gfx_stages[PIPE_SHADER_TESS_EVAL];
   if (!sel)
      sel = ctx->gfx_stages[PIPE_SHADER_VERTEX];
   return sel;
}

static bool
update_draw_indirect_with_sysvals(struct d3d12_context *ctx,
   const struct pipe_draw_info *dinfo,
   unsigned drawid,
   const struct pipe_draw_indirect_info **indirect_inout,
   struct pipe_draw_indirect_info *indirect_out)
{
   if (*indirect_inout == nullptr ||
      ctx->gfx_stages[PIPE_SHADER_VERTEX] == nullptr)
      return false;

   auto sys_values_read = ctx->gfx_stages[PIPE_SHADER_VERTEX]->initial->info.system_values_read;
   bool any =  BITSET_TEST(sys_values_read, SYSTEM_VALUE_VERTEX_ID_ZERO_BASE) ||
               BITSET_TEST(sys_values_read, SYSTEM_VALUE_BASE_VERTEX) ||
               BITSET_TEST(sys_values_read, SYSTEM_VALUE_FIRST_VERTEX) ||
               BITSET_TEST(sys_values_read, SYSTEM_VALUE_BASE_INSTANCE) ||
               BITSET_TEST(sys_values_read, SYSTEM_VALUE_DRAW_ID);

   if (!any)
      return false;

   d3d12_compute_transform_save_restore save;
   d3d12_save_compute_transform_state(ctx, &save);

   auto indirect_in = *indirect_inout;
   *indirect_inout = indirect_out;

   d3d12_compute_transform_key key;
   memset(&key, 0, sizeof(key));
   key.type = d3d12_compute_transform_type::base_vertex;
   key.base_vertex.indexed = dinfo->index_size > 0;
   key.base_vertex.dynamic_count = indirect_in->indirect_draw_count != nullptr;
   ctx->base.bind_compute_state(&ctx->base, d3d12_get_compute_transform(ctx, &key));

   ctx->transform_state_vars[0] = indirect_in->stride;
   ctx->transform_state_vars[1] = indirect_in->offset;
   ctx->transform_state_vars[2] = drawid;

   if (indirect_in->indirect_draw_count) {
      pipe_constant_buffer draw_count_cbuf;
      draw_count_cbuf.buffer = indirect_in->indirect_draw_count;
      draw_count_cbuf.buffer_offset = indirect_in->indirect_draw_count_offset;
      draw_count_cbuf.buffer_size = 4;
      draw_count_cbuf.user_buffer = nullptr;
      ctx->base.set_constant_buffer(&ctx->base, PIPE_SHADER_COMPUTE, 1, false, &draw_count_cbuf);
   }
   
   pipe_shader_buffer new_cs_ssbos[2];
   new_cs_ssbos[0].buffer = indirect_in->buffer;
   new_cs_ssbos[0].buffer_offset = 0;
   new_cs_ssbos[0].buffer_size = indirect_in->buffer->width0;

   /* 4 additional uints for base vertex, base instance, draw ID, and a bool for indexed draw */
   unsigned out_stride = sizeof(uint32_t) * ((key.base_vertex.indexed ? 5 : 4) + 4);
   pipe_resource output_buf_templ = {};
   output_buf_templ.target = PIPE_BUFFER;
   output_buf_templ.width0 = out_stride * indirect_in->draw_count;
   output_buf_templ.height0 = output_buf_templ.depth0 = output_buf_templ.array_size =
      output_buf_templ.last_level = 1;
   output_buf_templ.usage = PIPE_USAGE_DEFAULT;

   new_cs_ssbos[1].buffer = ctx->base.screen->resource_create(ctx->base.screen, &output_buf_templ);
   new_cs_ssbos[1].buffer_offset = 0;
   new_cs_ssbos[1].buffer_size = output_buf_templ.width0;
   ctx->base.set_shader_buffers(&ctx->base, PIPE_SHADER_COMPUTE, 0, 2, new_cs_ssbos, 2);

   pipe_grid_info grid = {};
   grid.block[0] = grid.block[1] = grid.block[2] = 1;
   grid.grid[0] = indirect_in->draw_count;
   grid.grid[1] = grid.grid[2] = 1;
   ctx->base.launch_grid(&ctx->base, &grid);

   d3d12_restore_compute_transform_state(ctx, &save);

   *indirect_out = *indirect_in;
   indirect_out->buffer = new_cs_ssbos[1].buffer;
   indirect_out->offset = 0;
   indirect_out->stride = out_stride;
   return true;
}

static bool
update_draw_auto(struct d3d12_context *ctx,
   const struct pipe_draw_indirect_info **indirect_inout,
   struct pipe_draw_indirect_info *indirect_out)
{
   if (*indirect_inout == nullptr ||
       (*indirect_inout)->count_from_stream_output == nullptr ||
       ctx->gfx_stages[PIPE_SHADER_VERTEX] == nullptr)
      return false;

   d3d12_compute_transform_save_restore save;
   d3d12_save_compute_transform_state(ctx, &save);

   auto indirect_in = *indirect_inout;
   *indirect_inout = indirect_out;

   d3d12_compute_transform_key key;
   memset(&key, 0, sizeof(key));
   key.type = d3d12_compute_transform_type::draw_auto;
   ctx->base.bind_compute_state(&ctx->base, d3d12_get_compute_transform(ctx, &key));

   auto so_arg = indirect_in->count_from_stream_output;
   d3d12_stream_output_target *target = (d3d12_stream_output_target *)so_arg;

   ctx->transform_state_vars[0] = ctx->gfx_pipeline_state.ves->strides[0];
   ctx->transform_state_vars[1] = ctx->vbs[0].buffer_offset - so_arg->buffer_offset;
   
   pipe_shader_buffer new_cs_ssbo;
   new_cs_ssbo.buffer = target->fill_buffer;
   new_cs_ssbo.buffer_offset = target->fill_buffer_offset;
   new_cs_ssbo.buffer_size = target->fill_buffer->width0 - new_cs_ssbo.buffer_offset;
   ctx->base.set_shader_buffers(&ctx->base, PIPE_SHADER_COMPUTE, 0, 1, &new_cs_ssbo, 1);

   pipe_grid_info grid = {};
   grid.block[0] = grid.block[1] = grid.block[2] = 1;
   grid.grid[0] = grid.grid[1] = grid.grid[2] = 1;
   ctx->base.launch_grid(&ctx->base, &grid);

   d3d12_restore_compute_transform_state(ctx, &save);

   *indirect_out = *indirect_in;
   pipe_resource_reference(&indirect_out->buffer, target->fill_buffer);
   indirect_out->offset = target->fill_buffer_offset + 4;
   indirect_out->stride = sizeof(D3D12_DRAW_ARGUMENTS);
   indirect_out->count_from_stream_output = nullptr;
   return true;
}

void
d3d12_draw_vbo(struct pipe_context *pctx,
               const struct pipe_draw_info *dinfo,
               unsigned drawid_offset,
               const struct pipe_draw_indirect_info *indirect,
               const struct pipe_draw_start_count_bias *draws,
               unsigned num_draws)
{
   if (num_draws > 1) {
      util_draw_multi(pctx, dinfo, drawid_offset, indirect, draws, num_draws);
      return;
   }

   if (!indirect && (!draws[0].count || !dinfo->instance_count))
      return;

   struct d3d12_context *ctx = d3d12_context(pctx);
   struct d3d12_screen *screen = d3d12_screen(pctx->screen);
   struct d3d12_batch *batch;
   struct pipe_resource *index_buffer = NULL;
   unsigned index_offset = 0;
   enum d3d12_surface_conversion_mode conversion_modes[PIPE_MAX_COLOR_BUFS] = {};
   struct pipe_draw_indirect_info patched_indirect = {};

   if (!prim_supported((enum mesa_prim)dinfo->mode) ||
       dinfo->index_size == 1 ||
       (dinfo->primitive_restart && dinfo->restart_index != 0xffff &&
        dinfo->restart_index != 0xffffffff)) {

      if (!dinfo->primitive_restart &&
          !indirect &&
          !u_trim_pipe_prim((enum mesa_prim)dinfo->mode, (unsigned *)&draws[0].count))
         return;

      ctx->initial_api_prim = (enum mesa_prim)dinfo->mode;
      util_primconvert_save_rasterizer_state(ctx->primconvert, &ctx->gfx_pipeline_state.rast->base);
      util_primconvert_draw_vbo(ctx->primconvert, dinfo, drawid_offset, indirect, draws, num_draws);
      return;
   }

   bool draw_auto = update_draw_auto(ctx, &indirect, &patched_indirect);
   bool indirect_with_sysvals = !draw_auto && update_draw_indirect_with_sysvals(ctx, dinfo, drawid_offset, &indirect, &patched_indirect);
   struct d3d12_cmd_signature_key cmd_sig_key;
   memset(&cmd_sig_key, 0, sizeof(cmd_sig_key));

   if (indirect) {
      cmd_sig_key.compute = false;
      cmd_sig_key.indexed = dinfo->index_size > 0;
      if (indirect->draw_count > 1 ||
          indirect->indirect_draw_count ||
          indirect_with_sysvals)
         cmd_sig_key.multi_draw_stride = indirect->stride;
      else if (cmd_sig_key.indexed)
         cmd_sig_key.multi_draw_stride = sizeof(D3D12_DRAW_INDEXED_ARGUMENTS);
      else
         cmd_sig_key.multi_draw_stride = sizeof(D3D12_DRAW_ARGUMENTS);
   }

   for (int i = 0; i < ctx->fb.nr_cbufs; ++i) {
      if (ctx->fb.cbufs[i]) {
         struct d3d12_surface *surface = d3d12_surface(ctx->fb.cbufs[i]);
         conversion_modes[i] = d3d12_surface_update_pre_draw(pctx, surface, d3d12_rtv_format(ctx, i));
         if (conversion_modes[i] != D3D12_SURFACE_CONVERSION_NONE)
            ctx->cmdlist_dirty |= D3D12_DIRTY_FRAMEBUFFER;
      }
   }

   struct d3d12_rasterizer_state *rast = ctx->gfx_pipeline_state.rast;
   if (rast->twoface_back) {
      enum mesa_prim saved_mode = ctx->initial_api_prim;
      twoface_emulation(ctx, rast, dinfo, indirect, &draws[0]);
      ctx->initial_api_prim = saved_mode;
   }

   if (ctx->pstipple.enabled && ctx->gfx_pipeline_state.rast->base.poly_stipple_enable)
      ctx->shader_dirty[PIPE_SHADER_FRAGMENT] |= D3D12_SHADER_DIRTY_SAMPLER_VIEWS |
                                                 D3D12_SHADER_DIRTY_SAMPLERS;

   /* this should *really* be fixed at a higher level than here! */
   enum mesa_prim reduced_prim = u_reduced_prim((enum mesa_prim)dinfo->mode);
   if (reduced_prim == MESA_PRIM_TRIANGLES &&
       ctx->gfx_pipeline_state.rast->base.cull_face == PIPE_FACE_FRONT_AND_BACK)
      return;

   if (ctx->gfx_pipeline_state.prim_type != dinfo->mode) {
      ctx->gfx_pipeline_state.prim_type = (enum mesa_prim)dinfo->mode;
      ctx->state_dirty |= D3D12_DIRTY_PRIM_MODE;
   }

   d3d12_select_shader_variants(ctx, dinfo);
   d3d12_validate_queries(ctx);
   for (unsigned i = 0; i < D3D12_GFX_SHADER_STAGES; ++i) {
      struct d3d12_shader *shader = ctx->gfx_stages[i] ? ctx->gfx_stages[i]->current : NULL;
      if (ctx->gfx_pipeline_state.stages[i] != shader) {
         ctx->gfx_pipeline_state.stages[i] = shader;
         ctx->state_dirty |= D3D12_DIRTY_SHADER;
      }
   }

   /* Reset to an invalid value after it's been used */
   ctx->initial_api_prim = MESA_PRIM_COUNT;

   /* Copy the stream output info from the current vertex/geometry shader */
   if (ctx->state_dirty & D3D12_DIRTY_SHADER) {
      struct d3d12_shader_selector *sel = d3d12_last_vertex_stage(ctx);
      if (sel) {
         ctx->gfx_pipeline_state.so_info = sel->so_info;
      } else {
         memset(&ctx->gfx_pipeline_state.so_info, 0, sizeof(sel->so_info));
      }
   }
   if (!validate_stream_output_targets(ctx)) {
      debug_printf("validate_stream_output_targets() failed\n");
      return;
   }

   D3D12_INDEX_BUFFER_STRIP_CUT_VALUE ib_strip_cut_value =
      D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
   if (dinfo->index_size > 0) {
      assert(dinfo->index_size != 1);

      if (dinfo->has_user_indices) {
         if (!util_upload_index_buffer(pctx, dinfo, &draws[0], &index_buffer,
             &index_offset, 4)) {
            debug_printf("util_upload_index_buffer() failed\n");
            return;
         }
      } else {
         index_buffer = dinfo->index.resource;
      }

      if (dinfo->primitive_restart) {
         assert(dinfo->restart_index == 0xffff ||
                dinfo->restart_index == 0xffffffff);
         ib_strip_cut_value = dinfo->restart_index == 0xffff ?
            D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFF :
            D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFFFFFF;
      }
   }

   if (ctx->gfx_pipeline_state.ib_strip_cut_value != ib_strip_cut_value) {
      ctx->gfx_pipeline_state.ib_strip_cut_value = ib_strip_cut_value;
      ctx->state_dirty |= D3D12_DIRTY_STRIP_CUT_VALUE;
   }

   if (!ctx->gfx_pipeline_state.root_signature || ctx->state_dirty & D3D12_DIRTY_SHADER) {
      ID3D12RootSignature *root_signature = d3d12_get_root_signature(ctx, false);
      if (ctx->gfx_pipeline_state.root_signature != root_signature) {
         ctx->gfx_pipeline_state.root_signature = root_signature;
         ctx->state_dirty |= D3D12_DIRTY_ROOT_SIGNATURE;
         for (int i = 0; i < D3D12_GFX_SHADER_STAGES; ++i)
            ctx->shader_dirty[i] |= D3D12_SHADER_DIRTY_ALL;
      }
   }

   if (!ctx->current_gfx_pso || ctx->state_dirty & D3D12_DIRTY_GFX_PSO) {
      ctx->current_gfx_pso = d3d12_get_gfx_pipeline_state(ctx);
      assert(ctx->current_gfx_pso);
   }

   ctx->cmdlist_dirty |= ctx->state_dirty;

   if (!check_descriptors_left(ctx, false))
      d3d12_flush_cmdlist(ctx);
   batch = d3d12_current_batch(ctx);

   if (ctx->cmdlist_dirty & D3D12_DIRTY_ROOT_SIGNATURE) {
      d3d12_batch_reference_object(batch, ctx->gfx_pipeline_state.root_signature);
      ctx->cmdlist->SetGraphicsRootSignature(ctx->gfx_pipeline_state.root_signature);
   }

   if (ctx->cmdlist_dirty & D3D12_DIRTY_GFX_PSO) {
      assert(ctx->current_gfx_pso);
      d3d12_batch_reference_object(batch, ctx->current_gfx_pso);
      ctx->cmdlist->SetPipelineState(ctx->current_gfx_pso);
   }

   D3D12_GPU_DESCRIPTOR_HANDLE root_desc_tables[MAX_DESCRIPTOR_TABLES];
   int root_desc_indices[MAX_DESCRIPTOR_TABLES];
   unsigned num_root_descriptors = update_graphics_root_parameters(ctx, dinfo, drawid_offset, &draws[0],
      root_desc_tables, root_desc_indices, &cmd_sig_key);

   bool need_zero_one_depth_range = d3d12_need_zero_one_depth_range(ctx);
   if (need_zero_one_depth_range != ctx->need_zero_one_depth_range) {
      ctx->cmdlist_dirty |= D3D12_DIRTY_VIEWPORT;
      ctx->need_zero_one_depth_range = need_zero_one_depth_range;
   }

   if (ctx->cmdlist_dirty & D3D12_DIRTY_VIEWPORT) {
      D3D12_VIEWPORT viewports[PIPE_MAX_VIEWPORTS];
      for (unsigned i = 0; i < ctx->num_viewports; ++i) {
         viewports[i] = ctx->viewports[i];
         if (ctx->need_zero_one_depth_range) {
            viewports[i].MinDepth = 0.0f;
            viewports[i].MaxDepth = 1.0f;
         }
         if (ctx->fb.nr_cbufs == 0 && !ctx->fb.zsbuf) {
            viewports[i].TopLeftX = MAX2(0.0f, viewports[i].TopLeftX);
            viewports[i].TopLeftY = MAX2(0.0f, viewports[i].TopLeftY);
            viewports[i].Width = MIN2(ctx->fb.width, viewports[i].Width);
            viewports[i].Height = MIN2(ctx->fb.height, viewports[i].Height);
         }
      }
      ctx->cmdlist->RSSetViewports(ctx->num_viewports, viewports);
   }

   if (ctx->cmdlist_dirty & D3D12_DIRTY_SCISSOR) {
      if (ctx->gfx_pipeline_state.rast->base.scissor && ctx->num_viewports > 0)
         ctx->cmdlist->RSSetScissorRects(ctx->num_viewports, ctx->scissors);
      else
         ctx->cmdlist->RSSetScissorRects(PIPE_MAX_VIEWPORTS, MAX_SCISSOR_ARRAY);
   }

   if (ctx->cmdlist_dirty & D3D12_DIRTY_BLEND_COLOR) {
      unsigned blend_factor_flags = ctx->gfx_pipeline_state.blend->blend_factor_flags;
      if (blend_factor_flags & (D3D12_BLEND_FACTOR_COLOR | D3D12_BLEND_FACTOR_ANY)) {
         ctx->cmdlist->OMSetBlendFactor(ctx->blend_factor);
      } else if (blend_factor_flags & D3D12_BLEND_FACTOR_ALPHA) {
         float alpha_const[4] = { ctx->blend_factor[3], ctx->blend_factor[3],
                                 ctx->blend_factor[3], ctx->blend_factor[3] };
         ctx->cmdlist->OMSetBlendFactor(alpha_const);
      }
   }

   if (ctx->cmdlist_dirty & D3D12_DIRTY_STENCIL_REF) {
      if (ctx->gfx_pipeline_state.zsa->backface_enabled &&
          screen->opts14.IndependentFrontAndBackStencilRefMaskSupported &&
          ctx->cmdlist8 != nullptr)
         ctx->cmdlist8->OMSetFrontAndBackStencilRef(ctx->stencil_ref.ref_value[0], ctx->stencil_ref.ref_value[1]);
      else
         ctx->cmdlist->OMSetStencilRef(ctx->stencil_ref.ref_value[0]);
   }

   if (ctx->cmdlist_dirty & D3D12_DIRTY_PRIM_MODE)
      ctx->cmdlist->IASetPrimitiveTopology(topology((enum mesa_prim)dinfo->mode, ctx->patch_vertices));

   for (unsigned i = 0; i < ctx->num_vbs; ++i) {
      if (ctx->vbs[i].buffer.resource) {
         struct d3d12_resource *res = d3d12_resource(ctx->vbs[i].buffer.resource);
         d3d12_transition_resource_state(ctx, res, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_TRANSITION_FLAG_ACCUMULATE_STATE);
         if (ctx->cmdlist_dirty & D3D12_DIRTY_VERTEX_BUFFERS)
            d3d12_batch_reference_resource(batch, res, false);
      }
   }
   if (ctx->cmdlist_dirty & (D3D12_DIRTY_VERTEX_BUFFERS | D3D12_DIRTY_VERTEX_ELEMENTS)) {
      uint16_t *strides = ctx->gfx_pipeline_state.ves ? ctx->gfx_pipeline_state.ves->strides : NULL;
      if (strides) {
         for (unsigned i = 0; i < ctx->num_vbs; i++)
            ctx->vbvs[i].StrideInBytes = strides[i];
      } else {
         for (unsigned i = 0; i < ctx->num_vbs; i++)
            ctx->vbvs[i].StrideInBytes = 0;
      }
      ctx->cmdlist->IASetVertexBuffers(0, ctx->num_vbs, ctx->vbvs);
   }

   if (index_buffer) {
      D3D12_INDEX_BUFFER_VIEW ibv;
      struct d3d12_resource *res = d3d12_resource(index_buffer);
      ibv.BufferLocation = d3d12_resource_gpu_virtual_address(res) + index_offset;
      ibv.SizeInBytes = res->base.b.width0 - index_offset;
      ibv.Format = ib_format(dinfo->index_size);
      d3d12_transition_resource_state(ctx, res, D3D12_RESOURCE_STATE_INDEX_BUFFER, D3D12_TRANSITION_FLAG_ACCUMULATE_STATE);
      if (ctx->cmdlist_dirty & D3D12_DIRTY_INDEX_BUFFER ||
          memcmp(&ctx->ibv, &ibv, sizeof(D3D12_INDEX_BUFFER_VIEW)) != 0) {
         ctx->ibv = ibv;
         d3d12_batch_reference_resource(batch, res, false);
         ctx->cmdlist->IASetIndexBuffer(&ibv);
      }

      if (dinfo->has_user_indices)
         pipe_resource_reference(&index_buffer, NULL);
   }

   if (ctx->cmdlist_dirty & D3D12_DIRTY_FRAMEBUFFER) {
      D3D12_CPU_DESCRIPTOR_HANDLE render_targets[PIPE_MAX_COLOR_BUFS] = {};
      D3D12_CPU_DESCRIPTOR_HANDLE *depth_desc = NULL, tmp_desc;
      for (int i = 0; i < ctx->fb.nr_cbufs; ++i) {
         if (ctx->fb.cbufs[i]) {
            struct d3d12_surface *surface = d3d12_surface(ctx->fb.cbufs[i]);
            render_targets[i] = d3d12_surface_get_handle(surface, conversion_modes[i]);
            d3d12_batch_reference_surface_texture(batch, surface);
         } else
            render_targets[i] = screen->null_rtv.cpu_handle;
      }
      if (ctx->fb.zsbuf) {
         struct d3d12_surface *surface = d3d12_surface(ctx->fb.zsbuf);
         tmp_desc = surface->desc_handle.cpu_handle;
         d3d12_batch_reference_surface_texture(batch, surface);
         depth_desc = &tmp_desc;
      }
      ctx->cmdlist->OMSetRenderTargets(ctx->fb.nr_cbufs, render_targets, false, depth_desc);
   }

   struct pipe_stream_output_target **so_targets = ctx->fake_so_buffer_factor ? ctx->fake_so_targets
                                                                              : ctx->so_targets;
   D3D12_STREAM_OUTPUT_BUFFER_VIEW *so_buffer_views = ctx->fake_so_buffer_factor ? ctx->fake_so_buffer_views
                                                                                 : ctx->so_buffer_views;
   for (unsigned i = 0; i < ctx->gfx_pipeline_state.num_so_targets; ++i) {
      struct d3d12_stream_output_target *target = (struct d3d12_stream_output_target *)so_targets[i];

      if (!target)
         continue;

      struct d3d12_resource *so_buffer = d3d12_resource(target->base.buffer);
      struct d3d12_resource *fill_buffer = d3d12_resource(target->fill_buffer);

      if (ctx->cmdlist_dirty & D3D12_DIRTY_STREAM_OUTPUT) {
         d3d12_batch_reference_resource(batch, so_buffer, true);
         d3d12_batch_reference_resource(batch, fill_buffer, true);
      }

      d3d12_transition_resource_state(ctx, so_buffer, D3D12_RESOURCE_STATE_STREAM_OUT, D3D12_TRANSITION_FLAG_ACCUMULATE_STATE);
      d3d12_transition_resource_state(ctx, fill_buffer, D3D12_RESOURCE_STATE_STREAM_OUT, D3D12_TRANSITION_FLAG_ACCUMULATE_STATE);
   }
   if (ctx->cmdlist_dirty & D3D12_DIRTY_STREAM_OUTPUT)
      ctx->cmdlist->SOSetTargets(0, 4, so_buffer_views);

   for (int i = 0; i < ctx->fb.nr_cbufs; ++i) {
      struct pipe_surface *psurf = ctx->fb.cbufs[i];
      if (!psurf)
         continue;

      struct pipe_resource *pres = conversion_modes[i] == D3D12_SURFACE_CONVERSION_BGRA_UINT ?
                                      d3d12_surface(psurf)->rgba_texture : psurf->texture;
      transition_surface_subresources_state(ctx, psurf, pres,
         D3D12_RESOURCE_STATE_RENDER_TARGET);
   }
   if (ctx->fb.zsbuf) {
      struct pipe_surface *psurf = ctx->fb.zsbuf;
      transition_surface_subresources_state(ctx, psurf, psurf->texture,
         D3D12_RESOURCE_STATE_DEPTH_WRITE);
   }

   ID3D12Resource *indirect_arg_buf = nullptr;
   ID3D12Resource *indirect_count_buf = nullptr;
   uint64_t indirect_arg_offset = 0, indirect_count_offset = 0;
   if (indirect) {
      if (indirect->buffer) {
         struct d3d12_resource *indirect_buf = d3d12_resource(indirect->buffer);
         uint64_t buf_offset = 0;
         indirect_arg_buf = d3d12_resource_underlying(indirect_buf, &buf_offset);
         indirect_arg_offset = indirect->offset + buf_offset;
         d3d12_transition_resource_state(ctx, indirect_buf,
            D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, D3D12_TRANSITION_FLAG_ACCUMULATE_STATE);
         d3d12_batch_reference_resource(batch, indirect_buf, false);
      }
      if (indirect->indirect_draw_count) {
         struct d3d12_resource *count_buf = d3d12_resource(indirect->indirect_draw_count);
         uint64_t count_offset = 0;
         indirect_count_buf = d3d12_resource_underlying(count_buf, &count_offset);
         indirect_count_offset = indirect->indirect_draw_count_offset + count_offset;
         d3d12_transition_resource_state(ctx, count_buf,
            D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, D3D12_TRANSITION_FLAG_ACCUMULATE_STATE);
         d3d12_batch_reference_resource(batch, count_buf, false);
      }
      assert(!indirect->count_from_stream_output);
   }

   d3d12_apply_resource_states(ctx, false);

   for (unsigned i = 0; i < num_root_descriptors; ++i)
      ctx->cmdlist->SetGraphicsRootDescriptorTable(root_desc_indices[i], root_desc_tables[i]);

   if (indirect) {
      unsigned draw_count = draw_auto ? 1 : indirect->draw_count;
      ID3D12CommandSignature *cmd_sig = d3d12_get_cmd_signature(ctx, &cmd_sig_key);
      ctx->cmdlist->ExecuteIndirect(cmd_sig, draw_count, indirect_arg_buf,
         indirect_arg_offset, indirect_count_buf, indirect_count_offset);
   } else {
      if (dinfo->index_size > 0)
         ctx->cmdlist->DrawIndexedInstanced(draws[0].count, dinfo->instance_count,
                                            draws[0].start, draws[0].index_bias,
                                            dinfo->start_instance);
      else
         ctx->cmdlist->DrawInstanced(draws[0].count, dinfo->instance_count,
                                     draws[0].start, dinfo->start_instance);
   }

   ctx->state_dirty &= D3D12_DIRTY_COMPUTE_MASK;
   batch->pending_memory_barrier = false;

   ctx->cmdlist_dirty &= D3D12_DIRTY_COMPUTE_MASK |
      (index_buffer ? 0 : D3D12_DIRTY_INDEX_BUFFER);

   /* The next dispatch needs to reassert the compute PSO */
   ctx->cmdlist_dirty |= D3D12_DIRTY_COMPUTE_SHADER;

   for (unsigned i = 0; i < D3D12_GFX_SHADER_STAGES; ++i)
      ctx->shader_dirty[i] = 0;

   for (int i = 0; i < ctx->fb.nr_cbufs; ++i) {
      if (ctx->fb.cbufs[i]) {
         struct d3d12_surface *surface = d3d12_surface(ctx->fb.cbufs[i]);
         d3d12_surface_update_post_draw(pctx, surface, conversion_modes[i]);
      }
   }

   pipe_resource_reference(&patched_indirect.buffer, NULL);
}

static bool
update_dispatch_indirect_with_sysvals(struct d3d12_context *ctx,
                                      struct pipe_resource **indirect_inout,
                                      unsigned *indirect_offset_inout,
                                      struct pipe_resource **indirect_out)
{
   if (*indirect_inout == nullptr ||
       ctx->compute_state == nullptr)
      return false;

   if (!BITSET_TEST(ctx->compute_state->current->nir->info.system_values_read, SYSTEM_VALUE_NUM_WORKGROUPS))
      return false;

   if (ctx->current_predication)
      ctx->cmdlist->SetPredication(nullptr, 0, D3D12_PREDICATION_OP_EQUAL_ZERO);

   auto indirect_in = *indirect_inout;

   /* 6 uints: 2 copies of the indirect arg buffer */
   pipe_resource output_buf_templ = {};
   output_buf_templ.target = PIPE_BUFFER;
   output_buf_templ.width0 = sizeof(uint32_t) * 6;
   output_buf_templ.height0 = output_buf_templ.depth0 = output_buf_templ.array_size =
      output_buf_templ.last_level = 1;
   output_buf_templ.usage = PIPE_USAGE_DEFAULT;
   *indirect_out = ctx->base.screen->resource_create(ctx->base.screen, &output_buf_templ);

   struct pipe_box src_box = { (int)*indirect_offset_inout, 0, 0, sizeof(uint32_t) * 3, 1, 1 };
   ctx->base.resource_copy_region(&ctx->base, *indirect_out, 0, 0, 0, 0, indirect_in, 0, &src_box);
   ctx->base.resource_copy_region(&ctx->base, *indirect_out, 0, src_box.width, 0, 0, indirect_in, 0, &src_box);

   if (ctx->current_predication)
      d3d12_enable_predication(ctx);

   *indirect_inout = *indirect_out;
   *indirect_offset_inout = 0;
   return true;
}

void
d3d12_launch_grid(struct pipe_context *pctx, const struct pipe_grid_info *info)
{
   struct d3d12_context *ctx = d3d12_context(pctx);
   struct d3d12_batch *batch;
   struct pipe_resource *patched_indirect = nullptr;

   struct d3d12_cmd_signature_key cmd_sig_key;
   memset(&cmd_sig_key, 0, sizeof(cmd_sig_key));
   cmd_sig_key.compute = 1;
   cmd_sig_key.multi_draw_stride = sizeof(D3D12_DISPATCH_ARGUMENTS);

   struct pipe_resource *indirect = info->indirect;
   unsigned indirect_offset = info->indirect_offset;
   if (indirect && update_dispatch_indirect_with_sysvals(ctx, &indirect, &indirect_offset, &patched_indirect))
      cmd_sig_key.multi_draw_stride = sizeof(D3D12_DISPATCH_ARGUMENTS) * 2;

   d3d12_select_compute_shader_variants(ctx, info);
   d3d12_validate_queries(ctx);
   struct d3d12_shader *shader = ctx->compute_state ? ctx->compute_state->current : NULL;
   if (ctx->compute_pipeline_state.stage != shader) {
      ctx->compute_pipeline_state.stage = shader;
      ctx->state_dirty |= D3D12_DIRTY_COMPUTE_SHADER;
   }

   if (!ctx->compute_pipeline_state.root_signature || ctx->state_dirty & D3D12_DIRTY_COMPUTE_SHADER) {
      ID3D12RootSignature *root_signature = d3d12_get_root_signature(ctx, true);
      if (ctx->compute_pipeline_state.root_signature != root_signature) {
         ctx->compute_pipeline_state.root_signature = root_signature;
         ctx->state_dirty |= D3D12_DIRTY_COMPUTE_ROOT_SIGNATURE;
         ctx->shader_dirty[PIPE_SHADER_COMPUTE] |= D3D12_SHADER_DIRTY_ALL;
      }
   }

   if (!ctx->current_compute_pso || ctx->state_dirty & D3D12_DIRTY_COMPUTE_PSO) {
      ctx->current_compute_pso = d3d12_get_compute_pipeline_state(ctx);
      assert(ctx->current_compute_pso);
   }

   ctx->cmdlist_dirty |= ctx->state_dirty;

   if (!check_descriptors_left(ctx, true))
      d3d12_flush_cmdlist(ctx);
   batch = d3d12_current_batch(ctx);

   if (ctx->cmdlist_dirty & D3D12_DIRTY_COMPUTE_ROOT_SIGNATURE) {
      d3d12_batch_reference_object(batch, ctx->compute_pipeline_state.root_signature);
      ctx->cmdlist->SetComputeRootSignature(ctx->compute_pipeline_state.root_signature);
   }

   if (ctx->cmdlist_dirty & D3D12_DIRTY_COMPUTE_PSO) {
      assert(ctx->current_compute_pso);
      d3d12_batch_reference_object(batch, ctx->current_compute_pso);
      ctx->cmdlist->SetPipelineState(ctx->current_compute_pso);
   }

   D3D12_GPU_DESCRIPTOR_HANDLE root_desc_tables[MAX_DESCRIPTOR_TABLES];
   int root_desc_indices[MAX_DESCRIPTOR_TABLES];
   unsigned num_root_descriptors = update_compute_root_parameters(ctx, info, root_desc_tables, root_desc_indices, &cmd_sig_key);

   ID3D12Resource *indirect_arg_buf = nullptr;
   uint64_t indirect_arg_offset = 0;
   if (indirect) {
      struct d3d12_resource *indirect_buf = d3d12_resource(indirect);
      uint64_t buf_offset = 0;
      indirect_arg_buf = d3d12_resource_underlying(indirect_buf, &buf_offset);
      indirect_arg_offset = indirect_offset + buf_offset;
      d3d12_transition_resource_state(ctx, indirect_buf,
         D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, D3D12_TRANSITION_FLAG_ACCUMULATE_STATE);
      d3d12_batch_reference_resource(batch, indirect_buf, false);
   }

   d3d12_apply_resource_states(ctx, ctx->compute_state->is_variant);

   for (unsigned i = 0; i < num_root_descriptors; ++i)
      ctx->cmdlist->SetComputeRootDescriptorTable(root_desc_indices[i], root_desc_tables[i]);

   if (indirect) {
      ID3D12CommandSignature *cmd_sig = d3d12_get_cmd_signature(ctx, &cmd_sig_key);
      ctx->cmdlist->ExecuteIndirect(cmd_sig, 1, indirect_arg_buf, indirect_arg_offset, nullptr, 0);
   } else {
      ctx->cmdlist->Dispatch(info->grid[0], info->grid[1], info->grid[2]);
   }

   ctx->state_dirty &= D3D12_DIRTY_GFX_MASK;
   ctx->cmdlist_dirty &= D3D12_DIRTY_GFX_MASK;

   /* The next draw needs to reassert the graphics PSO */
   ctx->cmdlist_dirty |= D3D12_DIRTY_SHADER;
   batch->pending_memory_barrier = false;

   ctx->shader_dirty[PIPE_SHADER_COMPUTE] = 0;
   pipe_resource_reference(&patched_indirect, nullptr);
}
