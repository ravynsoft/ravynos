/*
 * Copyright 2021 Alyssa Rosenzweig
 * SPDX-License-Identifier: MIT
 */
#include <stdio.h>
#include "asahi/lib/agx_pack.h"
#include "agx_state.h"

static uint64_t
agx_const_buffer_ptr(struct agx_batch *batch, struct pipe_constant_buffer *cb)
{
   if (cb->buffer) {
      struct agx_resource *rsrc = agx_resource(cb->buffer);
      agx_batch_reads(batch, rsrc);

      return rsrc->bo->ptr.gpu + cb->buffer_offset;
   } else {
      return 0;
   }
}

static uint64_t
agx_shader_buffer_ptr(struct agx_batch *batch, struct pipe_shader_buffer *sb)
{
   if (sb->buffer) {
      struct agx_resource *rsrc = agx_resource(sb->buffer);

      /* Assume SSBOs are written. TODO: Optimize read-only SSBOs */
      agx_batch_writes(batch, rsrc, 0);

      return rsrc->bo->ptr.gpu + sb->buffer_offset;
   } else {
      return 0;
   }
}

void
agx_upload_vbos(struct agx_batch *batch)
{
   struct agx_context *ctx = batch->ctx;

   u_foreach_bit(vbo, ctx->vb_mask) {
      struct pipe_vertex_buffer vb = ctx->vertex_buffers[vbo];
      assert(!vb.is_user_buffer);

      if (vb.buffer.resource) {
         struct agx_resource *rsrc = agx_resource(vb.buffer.resource);
         agx_batch_reads(batch, rsrc);

         batch->uniforms.vbo_base[vbo] = rsrc->bo->ptr.gpu + vb.buffer_offset;
      } else {
         batch->uniforms.vbo_base[vbo] = 0;
      }
   }
}

void
agx_upload_uniforms(struct agx_batch *batch)
{
   struct agx_context *ctx = batch->ctx;

   struct agx_ptr root_ptr = agx_pool_alloc_aligned(
      &batch->pool, sizeof(struct agx_draw_uniforms), 16);

   batch->uniforms.tables[AGX_SYSVAL_TABLE_ROOT] = root_ptr.gpu;
   batch->uniforms.sample_mask = ctx->sample_mask;

   batch->uniforms.sprite_mask = (batch->reduced_prim == MESA_PRIM_POINTS)
                                    ? ctx->rast->base.sprite_coord_enable
                                    : 0;

   memcpy(root_ptr.cpu, &batch->uniforms, sizeof(batch->uniforms));
}

uint64_t
agx_upload_stage_uniforms(struct agx_batch *batch, uint64_t textures,
                          enum pipe_shader_type stage)
{
   struct agx_context *ctx = batch->ctx;
   struct agx_stage *st = &ctx->stage[stage];
   struct agx_device *dev = agx_device(ctx->base.screen);

   struct agx_ptr root_ptr = agx_pool_alloc_aligned(
      &batch->pool, sizeof(struct agx_stage_uniforms), 16);

   struct agx_stage_uniforms uniforms = {
      .texture_base = textures,
   };

   u_foreach_bit(s, st->valid_samplers) {
      uniforms.lod_bias[s] = st->samplers[s]->lod_bias_as_fp16;
   }

   /* If we use bindless samplers, insert sampler into the heap */
   if (st->shader && st->shader->uses_bindless_samplers) {
      u_foreach_bit(s, st->valid_samplers) {
         uniforms.sampler_handle[s] =
            28 +
            agx_sampler_heap_add(dev, &batch->sampler_heap,
                                 &st->samplers[s]->desc_without_custom_border);
      }
   }

   u_foreach_bit(cb, st->cb_mask) {
      uniforms.ubo_base[cb] = agx_const_buffer_ptr(batch, &st->cb[cb]);
      uniforms.ubo_size[cb] = st->cb[cb].buffer_size;
   }

   u_foreach_bit(cb, st->ssbo_mask) {
      uniforms.ssbo_base[cb] = agx_shader_buffer_ptr(batch, &st->ssbo[cb]);
      uniforms.ssbo_size[cb] = st->ssbo[cb].buffer_size;
   }

   memcpy(root_ptr.cpu, &uniforms, sizeof(uniforms));
   return root_ptr.gpu;
}
