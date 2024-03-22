/*
 * Copyright (C) 2019-2020 Collabora, Ltd.
 * © Copyright 2018 Alyssa Rosenzweig
 * Copyright © 2014-2017 Broadcom
 * Copyright (C) 2017 Intel Corporation
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include <errno.h>
#include <poll.h>

#include "pan_bo.h"
#include "pan_context.h"
#include "pan_minmax_cache.h"

#include "util/format/u_format.h"
#include "util/half_float.h"
#include "util/libsync.h"
#include "util/macros.h"
#include "util/u_debug_cb.h"
#include "util/u_helpers.h"
#include "util/u_inlines.h"
#include "util/u_math.h"
#include "util/u_memory.h"
#include "util/u_prim.h"
#include "util/u_prim_restart.h"
#include "util/u_surface.h"
#include "util/u_upload_mgr.h"
#include "util/u_vbuf.h"

#include "compiler/nir/nir_serialize.h"
#include "util/pan_lower_framebuffer.h"
#include "decode.h"
#include "pan_fence.h"
#include "pan_screen.h"
#include "pan_util.h"

static void
panfrost_clear(struct pipe_context *pipe, unsigned buffers,
               const struct pipe_scissor_state *scissor_state,
               const union pipe_color_union *color, double depth,
               unsigned stencil)
{
   if (!panfrost_render_condition_check(pan_context(pipe)))
      return;

   /* Only get batch after checking the render condition, since the check can
    * cause the batch to be flushed.
    */
   struct panfrost_context *ctx = pan_context(pipe);
   struct panfrost_batch *batch = panfrost_get_batch_for_fbo(ctx);

   /* At the start of the batch, we can clear for free */
   if (batch->draw_count == 0) {
      panfrost_batch_clear(batch, buffers, color, depth, stencil);
      return;
   }

   /* Once there is content, clear with a fullscreen quad */
   panfrost_blitter_save(ctx, PAN_RENDER_CLEAR);

   perf_debug_ctx(ctx, "Clearing with quad");
   util_blitter_clear(
      ctx->blitter, ctx->pipe_framebuffer.width, ctx->pipe_framebuffer.height,
      util_framebuffer_get_num_layers(&ctx->pipe_framebuffer), buffers, color,
      depth, stencil,
      util_framebuffer_get_num_samples(&ctx->pipe_framebuffer) > 1);
}

bool
panfrost_writes_point_size(struct panfrost_context *ctx)
{
   struct panfrost_compiled_shader *vs = ctx->prog[PIPE_SHADER_VERTEX];
   assert(vs != NULL);

   return vs->info.vs.writes_point_size && ctx->active_prim == MESA_PRIM_POINTS;
}

/* The entire frame is in memory -- send it off to the kernel! */

void
panfrost_flush(struct pipe_context *pipe, struct pipe_fence_handle **fence,
               unsigned flags)
{
   struct panfrost_context *ctx = pan_context(pipe);
   struct panfrost_device *dev = pan_device(pipe->screen);

   /* Submit all pending jobs */
   panfrost_flush_all_batches(ctx, NULL);

   if (fence) {
      struct pipe_fence_handle *f = panfrost_fence_create(ctx);
      pipe->screen->fence_reference(pipe->screen, fence, NULL);
      *fence = f;
   }

   if (dev->debug & PAN_DBG_TRACE)
      pandecode_next_frame(dev->decode_ctx);
}

static void
panfrost_texture_barrier(struct pipe_context *pipe, unsigned flags)
{
   struct panfrost_context *ctx = pan_context(pipe);
   panfrost_flush_all_batches(ctx, "Texture barrier");
}

static void
panfrost_set_frontend_noop(struct pipe_context *pipe, bool enable)
{
   struct panfrost_context *ctx = pan_context(pipe);
   panfrost_flush_all_batches(ctx, "Frontend no-op change");
   ctx->is_noop = enable;
}

static void
panfrost_generic_cso_delete(struct pipe_context *pctx, void *hwcso)
{
   free(hwcso);
}

static void
panfrost_bind_blend_state(struct pipe_context *pipe, void *cso)
{
   struct panfrost_context *ctx = pan_context(pipe);
   ctx->blend = cso;
   ctx->dirty |= PAN_DIRTY_BLEND;
}

static void
panfrost_set_blend_color(struct pipe_context *pipe,
                         const struct pipe_blend_color *blend_color)
{
   struct panfrost_context *ctx = pan_context(pipe);
   ctx->dirty |= PAN_DIRTY_BLEND;

   if (blend_color)
      ctx->blend_color = *blend_color;
}

/* Create a final blend given the context */

mali_ptr
panfrost_get_blend(struct panfrost_batch *batch, unsigned rti,
                   struct panfrost_bo **bo, unsigned *shader_offset)
{
   struct panfrost_context *ctx = batch->ctx;
   struct panfrost_device *dev = pan_device(ctx->base.screen);
   struct panfrost_blend_state *blend = ctx->blend;
   struct pan_blend_info info = blend->info[rti];
   struct pipe_surface *surf = batch->key.cbufs[rti];
   enum pipe_format fmt = surf->format;

   /* Use fixed-function if the equation permits, the format is blendable,
    * and no more than one unique constant is accessed */
   if (info.fixed_function && dev->blendable_formats[fmt].internal &&
       pan_blend_is_homogenous_constant(info.constant_mask,
                                        ctx->blend_color.color)) {
      return 0;
   }

   /* On all architectures, we can disable writes for a blend descriptor,
    * at which point the format doesn't matter.
    */
   if (!info.enabled)
      return 0;

   /* On Bifrost and newer, we can also use fixed-function for opaque
    * output regardless of the format by configuring the appropriate
    * conversion descriptor in the internal blend descriptor. (Midgard
    * requires a blend shader even for this case.)
    */
   if (dev->arch >= 6 && info.opaque)
      return 0;

   /* Otherwise, we need to grab a shader */
   struct pan_blend_state pan_blend = blend->pan;
   unsigned nr_samples = surf->nr_samples ?: surf->texture->nr_samples;

   pan_blend.rts[rti].format = fmt;
   pan_blend.rts[rti].nr_samples = nr_samples;
   memcpy(pan_blend.constants, ctx->blend_color.color,
          sizeof(pan_blend.constants));

   /* Upload the shader, sharing a BO */
   if (!(*bo)) {
      *bo = panfrost_batch_create_bo(batch, 4096, PAN_BO_EXECUTE,
                                     PIPE_SHADER_FRAGMENT, "Blend shader");
   }

   struct panfrost_compiled_shader *ss = ctx->prog[PIPE_SHADER_FRAGMENT];

   /* Default for Midgard */
   nir_alu_type col0_type = nir_type_float32;
   nir_alu_type col1_type = nir_type_float32;

   /* Bifrost has per-output types, respect them */
   if (dev->arch >= 6) {
      col0_type = ss->info.bifrost.blend[rti].type;
      col1_type = ss->info.bifrost.blend_src1_type;
   }

   pthread_mutex_lock(&dev->blend_shaders.lock);
   struct pan_blend_shader_variant *shader =
      pan_screen(ctx->base.screen)
         ->vtbl.get_blend_shader(dev, &pan_blend, col0_type, col1_type, rti);

   /* Size check and upload */
   unsigned offset = *shader_offset;
   assert((offset + shader->binary.size) < 4096);
   memcpy((*bo)->ptr.cpu + offset, shader->binary.data, shader->binary.size);
   *shader_offset += shader->binary.size;
   pthread_mutex_unlock(&dev->blend_shaders.lock);

   return ((*bo)->ptr.gpu + offset) | shader->first_tag;
}

static void
panfrost_bind_rasterizer_state(struct pipe_context *pctx, void *hwcso)
{
   struct panfrost_context *ctx = pan_context(pctx);
   ctx->rasterizer = hwcso;

   /* We can assume rasterizer is always dirty, the dependencies are
    * too intricate to bother tracking in detail. However we could
    * probably diff the renderers for viewport dirty tracking, that
    * just cares about the scissor enable and the depth clips. */
   ctx->dirty |= PAN_DIRTY_SCISSOR | PAN_DIRTY_RASTERIZER;
}

static void
panfrost_set_shader_images(struct pipe_context *pctx,
                           enum pipe_shader_type shader, unsigned start_slot,
                           unsigned count, unsigned unbind_num_trailing_slots,
                           const struct pipe_image_view *iviews)
{
   struct panfrost_context *ctx = pan_context(pctx);
   ctx->dirty_shader[shader] |= PAN_DIRTY_STAGE_IMAGE;

   /* Unbind start_slot...start_slot+count */
   if (!iviews) {
      for (int i = start_slot;
           i < start_slot + count + unbind_num_trailing_slots; i++) {
         pipe_resource_reference(&ctx->images[shader][i].resource, NULL);
      }

      ctx->image_mask[shader] &= ~(((1ull << count) - 1) << start_slot);
      return;
   }

   /* Bind start_slot...start_slot+count */
   for (int i = 0; i < count; i++) {
      const struct pipe_image_view *image = &iviews[i];
      SET_BIT(ctx->image_mask[shader], 1 << (start_slot + i), image->resource);

      if (!image->resource) {
         util_copy_image_view(&ctx->images[shader][start_slot + i], NULL);
         continue;
      }

      struct panfrost_resource *rsrc = pan_resource(image->resource);

      /* Images don't work with AFBC, since they require pixel-level granularity
       */
      if (drm_is_afbc(rsrc->image.layout.modifier)) {
         pan_resource_modifier_convert(
            ctx, rsrc, DRM_FORMAT_MOD_ARM_16X16_BLOCK_U_INTERLEAVED, true,
            "Shader image");
      }

      util_copy_image_view(&ctx->images[shader][start_slot + i], image);
   }

   /* Unbind start_slot+count...start_slot+count+unbind_num_trailing_slots */
   for (int i = 0; i < unbind_num_trailing_slots; i++) {
      SET_BIT(ctx->image_mask[shader], 1 << (start_slot + count + i), NULL);
      util_copy_image_view(&ctx->images[shader][start_slot + count + i], NULL);
   }
}

static void
panfrost_bind_vertex_elements_state(struct pipe_context *pctx, void *hwcso)
{
   struct panfrost_context *ctx = pan_context(pctx);
   ctx->vertex = hwcso;
   ctx->dirty |= PAN_DIRTY_VERTEX;
}

static void
panfrost_bind_sampler_states(struct pipe_context *pctx,
                             enum pipe_shader_type shader, unsigned start_slot,
                             unsigned num_sampler, void **sampler)
{
   struct panfrost_context *ctx = pan_context(pctx);
   ctx->dirty_shader[shader] |= PAN_DIRTY_STAGE_SAMPLER;

   for (unsigned i = 0; i < num_sampler; i++) {
      unsigned p = start_slot + i;
      ctx->samplers[shader][p] = sampler ? sampler[i] : NULL;
      if (ctx->samplers[shader][p])
         ctx->valid_samplers[shader] |= BITFIELD_BIT(p);
      else
         ctx->valid_samplers[shader] &= ~BITFIELD_BIT(p);
   }

   ctx->sampler_count[shader] = util_last_bit(ctx->valid_samplers[shader]);
}

static void
panfrost_set_vertex_buffers(struct pipe_context *pctx, unsigned num_buffers,
                            unsigned unbind_num_trailing_slots,
                            bool take_ownership,
                            const struct pipe_vertex_buffer *buffers)
{
   struct panfrost_context *ctx = pan_context(pctx);

   util_set_vertex_buffers_mask(ctx->vertex_buffers, &ctx->vb_mask, buffers,
                                num_buffers, unbind_num_trailing_slots,
                                take_ownership);

   ctx->dirty |= PAN_DIRTY_VERTEX;
}

static void
panfrost_set_constant_buffer(struct pipe_context *pctx,
                             enum pipe_shader_type shader, uint index,
                             bool take_ownership,
                             const struct pipe_constant_buffer *buf)
{
   struct panfrost_context *ctx = pan_context(pctx);
   struct panfrost_constant_buffer *pbuf = &ctx->constant_buffer[shader];

   util_copy_constant_buffer(&pbuf->cb[index], buf, take_ownership);

   unsigned mask = (1 << index);

   if (unlikely(!buf)) {
      pbuf->enabled_mask &= ~mask;
      return;
   }

   pbuf->enabled_mask |= mask;
   ctx->dirty_shader[shader] |= PAN_DIRTY_STAGE_CONST;
}

static void
panfrost_set_stencil_ref(struct pipe_context *pctx,
                         const struct pipe_stencil_ref ref)
{
   struct panfrost_context *ctx = pan_context(pctx);
   ctx->stencil_ref = ref;
   ctx->dirty |= PAN_DIRTY_ZS;
}

static void
panfrost_set_sampler_views(struct pipe_context *pctx,
                           enum pipe_shader_type shader, unsigned start_slot,
                           unsigned num_views,
                           unsigned unbind_num_trailing_slots,
                           bool take_ownership,
                           struct pipe_sampler_view **views)
{
   struct panfrost_context *ctx = pan_context(pctx);
   ctx->dirty_shader[shader] |= PAN_DIRTY_STAGE_TEXTURE;

   unsigned new_nr = 0;
   unsigned i;

   for (i = 0; i < num_views; ++i) {
      struct pipe_sampler_view *view = views ? views[i] : NULL;
      unsigned p = i + start_slot;

      if (view)
         new_nr = p + 1;

      if (take_ownership) {
         pipe_sampler_view_reference(
            (struct pipe_sampler_view **)&ctx->sampler_views[shader][p], NULL);
         ctx->sampler_views[shader][i] = (struct panfrost_sampler_view *)view;
      } else {
         pipe_sampler_view_reference(
            (struct pipe_sampler_view **)&ctx->sampler_views[shader][p], view);
      }
   }

   for (; i < num_views + unbind_num_trailing_slots; i++) {
      unsigned p = i + start_slot;
      pipe_sampler_view_reference(
         (struct pipe_sampler_view **)&ctx->sampler_views[shader][p], NULL);
   }

   /* If the sampler view count is higher than the greatest sampler view
    * we touch, it can't change */
   if (ctx->sampler_view_count[shader] >
       start_slot + num_views + unbind_num_trailing_slots)
      return;

   /* If we haven't set any sampler views here, search lower numbers for
    * set sampler views */
   if (new_nr == 0) {
      for (i = 0; i < start_slot; ++i) {
         if (ctx->sampler_views[shader][i])
            new_nr = i + 1;
      }
   }

   ctx->sampler_view_count[shader] = new_nr;
}

static void
panfrost_set_shader_buffers(struct pipe_context *pctx,
                            enum pipe_shader_type shader, unsigned start,
                            unsigned count,
                            const struct pipe_shader_buffer *buffers,
                            unsigned writable_bitmask)
{
   struct panfrost_context *ctx = pan_context(pctx);

   util_set_shader_buffers_mask(ctx->ssbo[shader], &ctx->ssbo_mask[shader],
                                buffers, start, count);

   ctx->dirty_shader[shader] |= PAN_DIRTY_STAGE_SSBO;
}

static void
panfrost_set_framebuffer_state(struct pipe_context *pctx,
                               const struct pipe_framebuffer_state *fb)
{
   struct panfrost_context *ctx = pan_context(pctx);

   util_copy_framebuffer_state(&ctx->pipe_framebuffer, fb);
   ctx->batch = NULL;

   /* Hot draw call path needs the mask of active render targets */
   ctx->fb_rt_mask = 0;

   for (unsigned i = 0; i < ctx->pipe_framebuffer.nr_cbufs; ++i) {
      if (ctx->pipe_framebuffer.cbufs[i])
         ctx->fb_rt_mask |= BITFIELD_BIT(i);
   }
}

static void
panfrost_bind_depth_stencil_state(struct pipe_context *pipe, void *cso)
{
   struct panfrost_context *ctx = pan_context(pipe);
   ctx->depth_stencil = cso;
   ctx->dirty |= PAN_DIRTY_ZS;
}

static void
panfrost_set_sample_mask(struct pipe_context *pipe, unsigned sample_mask)
{
   struct panfrost_context *ctx = pan_context(pipe);
   ctx->sample_mask = sample_mask;
   ctx->dirty |= PAN_DIRTY_MSAA;
}

static void
panfrost_set_min_samples(struct pipe_context *pipe, unsigned min_samples)
{
   struct panfrost_context *ctx = pan_context(pipe);
   ctx->min_samples = min_samples;
   ctx->dirty |= PAN_DIRTY_MSAA;
}

static void
panfrost_set_clip_state(struct pipe_context *pipe,
                        const struct pipe_clip_state *clip)
{
   // struct panfrost_context *panfrost = pan_context(pipe);
}

static void
panfrost_set_viewport_states(struct pipe_context *pipe, unsigned start_slot,
                             unsigned num_viewports,
                             const struct pipe_viewport_state *viewports)
{
   struct panfrost_context *ctx = pan_context(pipe);

   assert(start_slot == 0);
   assert(num_viewports == 1);

   ctx->pipe_viewport = *viewports;
   ctx->dirty |= PAN_DIRTY_VIEWPORT;
}

static void
panfrost_set_scissor_states(struct pipe_context *pipe, unsigned start_slot,
                            unsigned num_scissors,
                            const struct pipe_scissor_state *scissors)
{
   struct panfrost_context *ctx = pan_context(pipe);

   assert(start_slot == 0);
   assert(num_scissors == 1);

   ctx->scissor = *scissors;
   ctx->dirty |= PAN_DIRTY_SCISSOR;
}

static void
panfrost_set_polygon_stipple(struct pipe_context *pipe,
                             const struct pipe_poly_stipple *stipple)
{
   // struct panfrost_context *panfrost = pan_context(pipe);
}

static void
panfrost_set_active_query_state(struct pipe_context *pipe, bool enable)
{
   struct panfrost_context *ctx = pan_context(pipe);
   ctx->active_queries = enable;
   ctx->dirty |= PAN_DIRTY_OQ;
}

static void
panfrost_render_condition(struct pipe_context *pipe, struct pipe_query *query,
                          bool condition, enum pipe_render_cond_flag mode)
{
   struct panfrost_context *ctx = pan_context(pipe);

   ctx->cond_query = (struct panfrost_query *)query;
   ctx->cond_cond = condition;
   ctx->cond_mode = mode;
}

static void
panfrost_destroy(struct pipe_context *pipe)
{
   struct panfrost_context *panfrost = pan_context(pipe);
   struct panfrost_device *dev = pan_device(pipe->screen);

   _mesa_hash_table_destroy(panfrost->writers, NULL);

   if (panfrost->blitter)
      util_blitter_destroy(panfrost->blitter);

   util_unreference_framebuffer_state(&panfrost->pipe_framebuffer);
   u_upload_destroy(pipe->stream_uploader);

   panfrost_pool_cleanup(&panfrost->descs);
   panfrost_pool_cleanup(&panfrost->shaders);
   panfrost_afbc_context_destroy(panfrost);

   drmSyncobjDestroy(panfrost_device_fd(dev), panfrost->in_sync_obj);
   if (panfrost->in_sync_fd != -1)
      close(panfrost->in_sync_fd);

   drmSyncobjDestroy(panfrost_device_fd(dev), panfrost->syncobj);
   ralloc_free(pipe);
}

static struct pipe_query *
panfrost_create_query(struct pipe_context *pipe, unsigned type, unsigned index)
{
   struct panfrost_query *q = rzalloc(pipe, struct panfrost_query);

   q->type = type;
   q->index = index;

   return (struct pipe_query *)q;
}

static void
panfrost_destroy_query(struct pipe_context *pipe, struct pipe_query *q)
{
   struct panfrost_query *query = (struct panfrost_query *)q;

   if (query->rsrc)
      pipe_resource_reference(&query->rsrc, NULL);

   ralloc_free(q);
}

static bool
panfrost_begin_query(struct pipe_context *pipe, struct pipe_query *q)
{
   struct panfrost_context *ctx = pan_context(pipe);
   struct panfrost_device *dev = pan_device(ctx->base.screen);
   struct panfrost_query *query = (struct panfrost_query *)q;

   switch (query->type) {
   case PIPE_QUERY_OCCLUSION_COUNTER:
   case PIPE_QUERY_OCCLUSION_PREDICATE:
   case PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE: {
      unsigned size = sizeof(uint64_t) * dev->core_id_range;

      /* Allocate a resource for the query results to be stored */
      if (!query->rsrc) {
         query->rsrc = pipe_buffer_create(ctx->base.screen,
                                          PIPE_BIND_QUERY_BUFFER, 0, size);
      }

      /* Default to 0 if nothing at all drawn. */
      uint8_t *zeroes = alloca(size);
      memset(zeroes, 0, size);
      pipe_buffer_write(pipe, query->rsrc, 0, size, zeroes);

      query->msaa = (ctx->pipe_framebuffer.samples > 1);
      ctx->occlusion_query = query;
      ctx->dirty |= PAN_DIRTY_OQ;
      break;
   }

      /* Geometry statistics are computed in the driver. XXX: geom/tess
       * shaders.. */

   case PIPE_QUERY_PRIMITIVES_GENERATED:
      query->start = ctx->prims_generated;
      break;
   case PIPE_QUERY_PRIMITIVES_EMITTED:
      query->start = ctx->tf_prims_generated;
      break;

   case PAN_QUERY_DRAW_CALLS:
      query->start = ctx->draw_calls;
      break;

   default:
      /* TODO: timestamp queries, etc? */
      break;
   }

   return true;
}

static bool
panfrost_end_query(struct pipe_context *pipe, struct pipe_query *q)
{
   struct panfrost_context *ctx = pan_context(pipe);
   struct panfrost_query *query = (struct panfrost_query *)q;

   switch (query->type) {
   case PIPE_QUERY_OCCLUSION_COUNTER:
   case PIPE_QUERY_OCCLUSION_PREDICATE:
   case PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE:
      ctx->occlusion_query = NULL;
      ctx->dirty |= PAN_DIRTY_OQ;
      break;
   case PIPE_QUERY_PRIMITIVES_GENERATED:
      query->end = ctx->prims_generated;
      break;
   case PIPE_QUERY_PRIMITIVES_EMITTED:
      query->end = ctx->tf_prims_generated;
      break;
   case PAN_QUERY_DRAW_CALLS:
      query->end = ctx->draw_calls;
      break;
   }

   return true;
}

static bool
panfrost_get_query_result(struct pipe_context *pipe, struct pipe_query *q,
                          bool wait, union pipe_query_result *vresult)
{
   struct panfrost_query *query = (struct panfrost_query *)q;
   struct panfrost_context *ctx = pan_context(pipe);
   struct panfrost_device *dev = pan_device(ctx->base.screen);
   struct panfrost_resource *rsrc = pan_resource(query->rsrc);

   switch (query->type) {
   case PIPE_QUERY_OCCLUSION_COUNTER:
   case PIPE_QUERY_OCCLUSION_PREDICATE:
   case PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE:
      panfrost_flush_writer(ctx, rsrc, "Occlusion query");
      panfrost_bo_wait(rsrc->image.data.bo, INT64_MAX, false);

      /* Read back the query results */
      uint64_t *result = (uint64_t *)rsrc->image.data.bo->ptr.cpu;

      if (query->type == PIPE_QUERY_OCCLUSION_COUNTER) {
         uint64_t passed = 0;
         for (int i = 0; i < dev->core_id_range; ++i)
            passed += result[i];

         if (dev->arch <= 5 && !query->msaa)
            passed /= 4;

         vresult->u64 = passed;
      } else {
         vresult->b = !!result[0];
      }

      break;

   case PIPE_QUERY_PRIMITIVES_GENERATED:
   case PIPE_QUERY_PRIMITIVES_EMITTED:
      panfrost_flush_all_batches(ctx, "Primitive count query");
      vresult->u64 = query->end - query->start;
      break;

   case PAN_QUERY_DRAW_CALLS:
      vresult->u64 = query->end - query->start;
      break;

   default:
      /* TODO: more queries */
      break;
   }

   return true;
}

/*
 * Check the render condition for software condition rendering.
 *
 * Note: this may invalidate the batch!
 */
bool
panfrost_render_condition_check(struct panfrost_context *ctx)
{
   if (!ctx->cond_query)
      return true;

   perf_debug_ctx(ctx, "Implementing conditional rendering on the CPU");

   union pipe_query_result res = {0};
   bool wait = ctx->cond_mode != PIPE_RENDER_COND_NO_WAIT &&
               ctx->cond_mode != PIPE_RENDER_COND_BY_REGION_NO_WAIT;

   struct pipe_query *pq = (struct pipe_query *)ctx->cond_query;

   if (panfrost_get_query_result(&ctx->base, pq, wait, &res))
      return res.u64 != ctx->cond_cond;

   return true;
}

static struct pipe_stream_output_target *
panfrost_create_stream_output_target(struct pipe_context *pctx,
                                     struct pipe_resource *prsc,
                                     unsigned buffer_offset,
                                     unsigned buffer_size)
{
   struct pipe_stream_output_target *target;

   target = &rzalloc(pctx, struct panfrost_streamout_target)->base;

   if (!target)
      return NULL;

   pipe_reference_init(&target->reference, 1);
   pipe_resource_reference(&target->buffer, prsc);

   target->context = pctx;
   target->buffer_offset = buffer_offset;
   target->buffer_size = buffer_size;

   return target;
}

static void
panfrost_stream_output_target_destroy(struct pipe_context *pctx,
                                      struct pipe_stream_output_target *target)
{
   pipe_resource_reference(&target->buffer, NULL);
   ralloc_free(target);
}

static void
panfrost_set_stream_output_targets(struct pipe_context *pctx,
                                   unsigned num_targets,
                                   struct pipe_stream_output_target **targets,
                                   const unsigned *offsets)
{
   struct panfrost_context *ctx = pan_context(pctx);
   struct panfrost_streamout *so = &ctx->streamout;

   assert(num_targets <= ARRAY_SIZE(so->targets));

   for (unsigned i = 0; i < num_targets; i++) {
      if (targets[i] && offsets[i] != -1)
         pan_so_target(targets[i])->offset = offsets[i];

      pipe_so_target_reference(&so->targets[i], targets[i]);
   }

   for (unsigned i = num_targets; i < so->num_targets; i++)
      pipe_so_target_reference(&so->targets[i], NULL);

   so->num_targets = num_targets;
   ctx->dirty |= PAN_DIRTY_SO;
}

static void
panfrost_set_global_binding(struct pipe_context *pctx, unsigned first,
                            unsigned count, struct pipe_resource **resources,
                            uint32_t **handles)
{
   if (!resources)
      return;

   struct panfrost_context *ctx = pan_context(pctx);
   struct panfrost_batch *batch = panfrost_get_batch_for_fbo(ctx);

   for (unsigned i = first; i < first + count; ++i) {
      struct panfrost_resource *rsrc = pan_resource(resources[i]);
      panfrost_batch_write_rsrc(batch, rsrc, PIPE_SHADER_COMPUTE);

      util_range_add(&rsrc->base, &rsrc->valid_buffer_range, 0,
                     rsrc->base.width0);

      /* The handle points to uint32_t, but space is allocated for 64
       * bits. We need to respect the offset passed in. This interface
       * is so bad.
       */
      mali_ptr addr = 0;
      static_assert(sizeof(addr) == 8, "size out of sync");

      memcpy(&addr, handles[i], sizeof(addr));
      addr += rsrc->image.data.bo->ptr.gpu;

      memcpy(handles[i], &addr, sizeof(addr));
   }
}

static void
panfrost_memory_barrier(struct pipe_context *pctx, unsigned flags)
{
   /* TODO: Be smart and only flush the minimum needed, maybe emitting a
    * cache flush job if that would help */
   panfrost_flush_all_batches(pan_context(pctx), "Memory barrier");
}

static void
panfrost_create_fence_fd(struct pipe_context *pctx,
                         struct pipe_fence_handle **pfence, int fd,
                         enum pipe_fd_type type)
{
   *pfence = panfrost_fence_from_fd(pan_context(pctx), fd, type);
}

static void
panfrost_fence_server_sync(struct pipe_context *pctx,
                           struct pipe_fence_handle *f)
{
   struct panfrost_device *dev = pan_device(pctx->screen);
   struct panfrost_context *ctx = pan_context(pctx);
   int fd = -1, ret;

   ret = drmSyncobjExportSyncFile(panfrost_device_fd(dev), f->syncobj, &fd);
   assert(!ret);

   sync_accumulate("panfrost", &ctx->in_sync_fd, fd);
   close(fd);
}

struct pipe_context *
panfrost_create_context(struct pipe_screen *screen, void *priv, unsigned flags)
{
   struct panfrost_context *ctx = rzalloc(NULL, struct panfrost_context);
   struct pipe_context *gallium = (struct pipe_context *)ctx;
   struct panfrost_device *dev = pan_device(screen);

   gallium->screen = screen;

   gallium->destroy = panfrost_destroy;

   gallium->set_framebuffer_state = panfrost_set_framebuffer_state;
   gallium->set_debug_callback = u_default_set_debug_callback;

   gallium->create_fence_fd = panfrost_create_fence_fd;
   gallium->fence_server_sync = panfrost_fence_server_sync;

   gallium->flush = panfrost_flush;
   gallium->clear = panfrost_clear;
   gallium->clear_texture = u_default_clear_texture;
   gallium->texture_barrier = panfrost_texture_barrier;
   gallium->set_frontend_noop = panfrost_set_frontend_noop;

   gallium->set_vertex_buffers = panfrost_set_vertex_buffers;
   gallium->set_constant_buffer = panfrost_set_constant_buffer;
   gallium->set_shader_buffers = panfrost_set_shader_buffers;
   gallium->set_shader_images = panfrost_set_shader_images;

   gallium->set_stencil_ref = panfrost_set_stencil_ref;

   gallium->set_sampler_views = panfrost_set_sampler_views;

   gallium->bind_rasterizer_state = panfrost_bind_rasterizer_state;
   gallium->delete_rasterizer_state = panfrost_generic_cso_delete;

   gallium->bind_vertex_elements_state = panfrost_bind_vertex_elements_state;
   gallium->delete_vertex_elements_state = panfrost_generic_cso_delete;

   gallium->delete_sampler_state = panfrost_generic_cso_delete;
   gallium->bind_sampler_states = panfrost_bind_sampler_states;

   gallium->bind_depth_stencil_alpha_state = panfrost_bind_depth_stencil_state;
   gallium->delete_depth_stencil_alpha_state = panfrost_generic_cso_delete;

   gallium->set_sample_mask = panfrost_set_sample_mask;
   gallium->set_min_samples = panfrost_set_min_samples;

   gallium->set_clip_state = panfrost_set_clip_state;
   gallium->set_viewport_states = panfrost_set_viewport_states;
   gallium->set_scissor_states = panfrost_set_scissor_states;
   gallium->set_polygon_stipple = panfrost_set_polygon_stipple;
   gallium->set_active_query_state = panfrost_set_active_query_state;
   gallium->render_condition = panfrost_render_condition;

   gallium->create_query = panfrost_create_query;
   gallium->destroy_query = panfrost_destroy_query;
   gallium->begin_query = panfrost_begin_query;
   gallium->end_query = panfrost_end_query;
   gallium->get_query_result = panfrost_get_query_result;

   gallium->create_stream_output_target = panfrost_create_stream_output_target;
   gallium->stream_output_target_destroy =
      panfrost_stream_output_target_destroy;
   gallium->set_stream_output_targets = panfrost_set_stream_output_targets;

   gallium->bind_blend_state = panfrost_bind_blend_state;
   gallium->delete_blend_state = panfrost_generic_cso_delete;

   gallium->set_blend_color = panfrost_set_blend_color;

   gallium->set_global_binding = panfrost_set_global_binding;
   gallium->memory_barrier = panfrost_memory_barrier;

   pan_screen(screen)->vtbl.context_populate_vtbl(gallium);

   panfrost_resource_context_init(gallium);
   panfrost_shader_context_init(gallium);
   panfrost_afbc_context_init(ctx);

   gallium->stream_uploader = u_upload_create_default(gallium);
   gallium->const_uploader = gallium->stream_uploader;

   panfrost_pool_init(&ctx->descs, ctx, dev, 0, 4096, "Descriptors", true,
                      false);

   panfrost_pool_init(&ctx->shaders, ctx, dev, PAN_BO_EXECUTE, 4096, "Shaders",
                      true, false);

   ctx->blitter = util_blitter_create(gallium);

   ctx->writers = _mesa_hash_table_create(gallium, _mesa_hash_pointer,
                                          _mesa_key_pointer_equal);

   assert(ctx->blitter);

   /* Prepare for render! */

   /* By default mask everything on */
   ctx->sample_mask = ~0;
   ctx->active_queries = true;

   int ASSERTED ret;

   /* Create a syncobj in a signaled state. Will be updated to point to the
    * last queued job out_sync every time we submit a new job.
    */
   ret = drmSyncobjCreate(panfrost_device_fd(dev), DRM_SYNCOBJ_CREATE_SIGNALED,
                          &ctx->syncobj);
   assert(!ret && ctx->syncobj);

   /* Sync object/FD used for NATIVE_FENCE_FD. */
   ctx->in_sync_fd = -1;
   ret = drmSyncobjCreate(panfrost_device_fd(dev), 0, &ctx->in_sync_obj);
   assert(!ret);

   return gallium;
}
