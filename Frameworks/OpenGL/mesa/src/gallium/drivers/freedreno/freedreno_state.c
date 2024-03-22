/*
 * Copyright (C) 2012 Rob Clark <robclark@freedesktop.org>
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
 * Authors:
 *    Rob Clark <robclark@freedesktop.org>
 */

#include "pipe/p_state.h"
#include "util/u_dual_blend.h"
#include "util/u_helpers.h"
#include "util/u_memory.h"
#include "util/u_string.h"
#include "util/u_upload_mgr.h"

#include "common/freedreno_guardband.h"

#include "freedreno_context.h"
#include "freedreno_gmem.h"
#include "freedreno_query_hw.h"
#include "freedreno_resource.h"
#include "freedreno_state.h"
#include "freedreno_texture.h"
#include "freedreno_util.h"

#define get_safe(ptr, field) ((ptr) ? (ptr)->field : 0)

/* All the generic state handling.. In case of CSO's that are specific
 * to the GPU version, when the bind and the delete are common they can
 * go in here.
 */

static void
update_draw_cost(struct fd_context *ctx) assert_dt
{
   struct pipe_framebuffer_state *pfb = &ctx->framebuffer;

   ctx->draw_cost = pfb->nr_cbufs;
   for (unsigned i = 0; i < pfb->nr_cbufs; i++)
      if (fd_blend_enabled(ctx, i))
         ctx->draw_cost++;
   if (fd_depth_enabled(ctx))
      ctx->draw_cost++;
   if (fd_depth_write_enabled(ctx))
      ctx->draw_cost++;
}

static void
fd_set_blend_color(struct pipe_context *pctx,
                   const struct pipe_blend_color *blend_color) in_dt
{
   struct fd_context *ctx = fd_context(pctx);
   ctx->blend_color = *blend_color;
   fd_context_dirty(ctx, FD_DIRTY_BLEND_COLOR);
}

static void
fd_set_stencil_ref(struct pipe_context *pctx,
                   const struct pipe_stencil_ref stencil_ref) in_dt
{
   struct fd_context *ctx = fd_context(pctx);
   ctx->stencil_ref = stencil_ref;
   fd_context_dirty(ctx, FD_DIRTY_STENCIL_REF);
}

static void
fd_set_clip_state(struct pipe_context *pctx,
                  const struct pipe_clip_state *clip) in_dt
{
   struct fd_context *ctx = fd_context(pctx);
   ctx->ucp = *clip;
   fd_context_dirty(ctx, FD_DIRTY_UCP);
}

static void
fd_set_sample_mask(struct pipe_context *pctx, unsigned sample_mask) in_dt
{
   struct fd_context *ctx = fd_context(pctx);
   ctx->sample_mask = (uint16_t)sample_mask;
   fd_context_dirty(ctx, FD_DIRTY_SAMPLE_MASK);
}

static void
fd_set_sample_locations(struct pipe_context *pctx, size_t size,
                        const uint8_t *locations)
  in_dt
{
   struct fd_context *ctx = fd_context(pctx);

   if (!locations) {
      ctx->sample_locations_enabled = false;
      return;
   }

   size = MIN2(size, sizeof(ctx->sample_locations));
   memcpy(ctx->sample_locations, locations, size);
   ctx->sample_locations_enabled = true;

   fd_context_dirty(ctx, FD_DIRTY_SAMPLE_LOCATIONS);
}

static void
fd_set_min_samples(struct pipe_context *pctx, unsigned min_samples) in_dt
{
   struct fd_context *ctx = fd_context(pctx);
   ctx->min_samples = min_samples;
   fd_context_dirty(ctx, FD_DIRTY_MIN_SAMPLES);
}

static void
upload_user_buffer(struct pipe_context *pctx, struct pipe_constant_buffer *cb)
{
   u_upload_data(pctx->stream_uploader, 0, cb->buffer_size, 64,
                 cb->user_buffer, &cb->buffer_offset, &cb->buffer);
   cb->user_buffer = NULL;
}

/* notes from calim on #dri-devel:
 * index==0 will be non-UBO (ie. glUniformXYZ()) all packed together padded
 * out to vec4's
 * I should be able to consider that I own the user_ptr until the next
 * set_constant_buffer() call, at which point I don't really care about the
 * previous values.
 * index>0 will be UBO's.. well, I'll worry about that later
 */
static void
fd_set_constant_buffer(struct pipe_context *pctx, enum pipe_shader_type shader,
                       uint index, bool take_ownership,
                       const struct pipe_constant_buffer *cb) in_dt
{
   struct fd_context *ctx = fd_context(pctx);
   struct fd_constbuf_stateobj *so = &ctx->constbuf[shader];

   util_copy_constant_buffer(&so->cb[index], cb, take_ownership);

   /* Note that gallium frontends can unbind constant buffers by
    * passing NULL here.
    */
   if (unlikely(!cb)) {
      so->enabled_mask &= ~(1 << index);
      return;
   }

   if (cb->user_buffer && ctx->screen->gen >= 6) {
      upload_user_buffer(pctx, &so->cb[index]);
      cb = &so->cb[index];
   }

   so->enabled_mask |= 1 << index;

   fd_context_dirty_shader(ctx, shader, FD_DIRTY_SHADER_CONST);
   fd_resource_set_usage(cb->buffer, FD_DIRTY_CONST);
   fd_dirty_shader_resource(ctx, cb->buffer, shader, FD_DIRTY_SHADER_CONST, false);
}

void
fd_set_shader_buffers(struct pipe_context *pctx, enum pipe_shader_type shader,
                      unsigned start, unsigned count,
                      const struct pipe_shader_buffer *buffers,
                      unsigned writable_bitmask) in_dt
{
   struct fd_context *ctx = fd_context(pctx);
   struct fd_shaderbuf_stateobj *so = &ctx->shaderbuf[shader];
   const unsigned modified_bits = u_bit_consecutive(start, count);

   so->writable_mask &= ~modified_bits;
   so->writable_mask |= writable_bitmask << start;

   for (unsigned i = 0; i < count; i++) {
      unsigned n = i + start;
      struct pipe_shader_buffer *buf = &so->sb[n];

      if (buffers && buffers[i].buffer) {
         buf->buffer_offset = buffers[i].buffer_offset;
         buf->buffer_size = buffers[i].buffer_size;
         pipe_resource_reference(&buf->buffer, buffers[i].buffer);

         bool write = writable_bitmask & BIT(i);

         fd_resource_set_usage(buffers[i].buffer, FD_DIRTY_SSBO);
         fd_dirty_shader_resource(ctx, buffers[i].buffer, shader,
                                  FD_DIRTY_SHADER_SSBO, write);

         so->enabled_mask |= BIT(n);

         if (write) {
            struct fd_resource *rsc = fd_resource(buf->buffer);
            util_range_add(&rsc->b.b, &rsc->valid_buffer_range,
                           buf->buffer_offset,
                           buf->buffer_offset + buf->buffer_size);
         }
      } else {
         pipe_resource_reference(&buf->buffer, NULL);

         so->enabled_mask &= ~BIT(n);
      }
   }

   fd_context_dirty_shader(ctx, shader, FD_DIRTY_SHADER_SSBO);
}

void
fd_set_shader_images(struct pipe_context *pctx, enum pipe_shader_type shader,
                     unsigned start, unsigned count,
                     unsigned unbind_num_trailing_slots,
                     const struct pipe_image_view *images) in_dt
{
   struct fd_context *ctx = fd_context(pctx);
   struct fd_shaderimg_stateobj *so = &ctx->shaderimg[shader];

   unsigned mask = 0;

   if (images) {
      for (unsigned i = 0; i < count; i++) {
         unsigned n = i + start;
         struct pipe_image_view *buf = &so->si[n];

         if ((buf->resource == images[i].resource) &&
             (buf->format == images[i].format) &&
             (buf->access == images[i].access) &&
             !memcmp(&buf->u, &images[i].u, sizeof(buf->u)))
            continue;

         mask |= BIT(n);
         util_copy_image_view(buf, &images[i]);

         if (buf->resource) {
            bool write = buf->access & PIPE_IMAGE_ACCESS_WRITE;

            fd_resource_set_usage(buf->resource, FD_DIRTY_IMAGE);
            fd_dirty_shader_resource(ctx, buf->resource, shader,
                                     FD_DIRTY_SHADER_IMAGE, write);
            so->enabled_mask |= BIT(n);

            if (write && (buf->resource->target == PIPE_BUFFER)) {
               struct fd_resource *rsc = fd_resource(buf->resource);
               util_range_add(&rsc->b.b, &rsc->valid_buffer_range,
                              buf->u.buf.offset,
                              buf->u.buf.offset + buf->u.buf.size);
            }
         } else {
            so->enabled_mask &= ~BIT(n);
         }
      }
   } else {
      mask = (BIT(count) - 1) << start;

      for (unsigned i = 0; i < count; i++) {
         unsigned n = i + start;
         struct pipe_image_view *img = &so->si[n];

         pipe_resource_reference(&img->resource, NULL);
      }

      so->enabled_mask &= ~mask;
   }

   for (unsigned i = 0; i < unbind_num_trailing_slots; i++)
      pipe_resource_reference(&so->si[i + start + count].resource, NULL);

   so->enabled_mask &=
      ~(BITFIELD_MASK(unbind_num_trailing_slots) << (start + count));

   fd_context_dirty_shader(ctx, shader, FD_DIRTY_SHADER_IMAGE);
}

void
fd_set_framebuffer_state(struct pipe_context *pctx,
                         const struct pipe_framebuffer_state *framebuffer)
{
   struct fd_context *ctx = fd_context(pctx);
   struct pipe_framebuffer_state *cso;

   DBG("%ux%u, %u layers, %u samples", framebuffer->width, framebuffer->height,
       framebuffer->layers, framebuffer->samples);

   cso = &ctx->framebuffer;

   if (util_framebuffer_state_equal(cso, framebuffer))
      return;

   /* Do this *after* checking that the framebuffer state is actually
    * changing.  In the fd_blitter_clear() path, we get a pfb update
    * to restore the current pfb state, which should not trigger us
    * to flush (as that can cause the batch to be freed at a point
    * before fd_clear() returns, but after the point where it expects
    * flushes to potentially happen.
    */
   fd_context_switch_from(ctx);

   util_copy_framebuffer_state(cso, framebuffer);

   STATIC_ASSERT((4 * PIPE_MAX_COLOR_BUFS) == (8 * sizeof(ctx->all_mrt_channel_mask)));
   ctx->all_mrt_channel_mask = 0;

   /* Generate a bitmask of all valid channels for all MRTs.  Blend
    * state with unwritten channels essentially acts as blend enabled,
    * which disables LRZ write.  But only if the cbuf *has* the masked
    * channels, which is not known at the time the blend state is
    * created.
    */
   for (unsigned i = 0; i < framebuffer->nr_cbufs; i++) {
      if (!framebuffer->cbufs[i])
         continue;

      enum pipe_format format = framebuffer->cbufs[i]->format;
      unsigned nr = util_format_get_nr_components(format);

      ctx->all_mrt_channel_mask |= BITFIELD_MASK(nr) << (4 * i);
   }

   cso->samples = util_framebuffer_get_num_samples(cso);

   if (ctx->screen->reorder) {
      struct fd_batch *old_batch = NULL;

      fd_batch_reference(&old_batch, ctx->batch);

      if (likely(old_batch))
         fd_batch_finish_queries(old_batch);

      fd_batch_reference(&ctx->batch, NULL);
      fd_context_all_dirty(ctx);

      fd_batch_reference(&old_batch, NULL);
   } else if (ctx->batch) {
      DBG("%d: cbufs[0]=%p, zsbuf=%p", ctx->batch->needs_flush,
          framebuffer->cbufs[0], framebuffer->zsbuf);
      fd_batch_flush(ctx->batch);
   }

   fd_context_dirty(ctx, FD_DIRTY_FRAMEBUFFER);

   for (unsigned i = 0; i < PIPE_MAX_VIEWPORTS; i++) {
      ctx->disabled_scissor[i].minx = 0;
      ctx->disabled_scissor[i].miny = 0;
      ctx->disabled_scissor[i].maxx = cso->width - 1;
      ctx->disabled_scissor[i].maxy = cso->height - 1;
   }

   fd_context_dirty(ctx, FD_DIRTY_SCISSOR);
   update_draw_cost(ctx);
}

static void
fd_set_polygon_stipple(struct pipe_context *pctx,
                       const struct pipe_poly_stipple *stipple) in_dt
{
   struct fd_context *ctx = fd_context(pctx);
   ctx->stipple = *stipple;
   fd_context_dirty(ctx, FD_DIRTY_STIPPLE);
}

static void
fd_set_scissor_states(struct pipe_context *pctx, unsigned start_slot,
                      unsigned num_scissors,
                      const struct pipe_scissor_state *scissor) in_dt
{
   struct fd_context *ctx = fd_context(pctx);

   for (unsigned i = 0; i < num_scissors; i++) {
      unsigned idx = start_slot + i;

      if ((scissor[i].minx == scissor[i].maxx) ||
          (scissor[i].miny == scissor[i].maxy)) {
         ctx->scissor[idx].minx = ctx->scissor[idx].miny = 1;
         ctx->scissor[idx].maxx = ctx->scissor[idx].maxy = 0;
      } else {
         ctx->scissor[idx].minx = scissor[i].minx;
         ctx->scissor[idx].miny = scissor[i].miny;
         ctx->scissor[idx].maxx = MAX2(scissor[i].maxx, 1) - 1;
         ctx->scissor[idx].maxy = MAX2(scissor[i].maxy, 1) - 1;
      }
   }

   fd_context_dirty(ctx, FD_DIRTY_SCISSOR);
}

static void
init_scissor_states(struct pipe_context *pctx)
   in_dt
{
   struct fd_context *ctx = fd_context(pctx);

   for (unsigned idx = 0; idx < ARRAY_SIZE(ctx->scissor); idx++) {
      ctx->scissor[idx].minx = ctx->scissor[idx].miny = 1;
      ctx->scissor[idx].maxx = ctx->scissor[idx].maxy = 0;
   }
}

static void
fd_set_viewport_states(struct pipe_context *pctx, unsigned start_slot,
                       unsigned num_viewports,
                       const struct pipe_viewport_state *viewports) in_dt
{
   struct fd_context *ctx = fd_context(pctx);

   for (unsigned i = 0; i < num_viewports; i++) {
      unsigned idx = start_slot + i;
      struct pipe_scissor_state *scissor = &ctx->viewport_scissor[idx];
      const struct pipe_viewport_state *viewport = &viewports[i];

      ctx->viewport[idx] = *viewport;

      /* see si_get_scissor_from_viewport(): */

      /* Convert (-1, -1) and (1, 1) from clip space into window space. */
      float minx = -viewport->scale[0] + viewport->translate[0];
      float miny = -viewport->scale[1] + viewport->translate[1];
      float maxx = viewport->scale[0] + viewport->translate[0];
      float maxy = viewport->scale[1] + viewport->translate[1];

      /* Handle inverted viewports. */
      if (minx > maxx) {
         SWAP(minx, maxx);
      }
      if (miny > maxy) {
         SWAP(miny, maxy);
      }

      const float max_dims = ctx->screen->gen >= 4 ? 16384.f : 4096.f;

      /* Clamp, convert to integer and round up the max bounds. */
      scissor->minx = CLAMP(minx, 0.f, max_dims);
      scissor->miny = CLAMP(miny, 0.f, max_dims);
      scissor->maxx = MAX2(CLAMP(ceilf(maxx), 0.f, max_dims), 1) - 1;
      scissor->maxy = MAX2(CLAMP(ceilf(maxy), 0.f, max_dims), 1) - 1;
   }

   fd_context_dirty(ctx, FD_DIRTY_VIEWPORT);

   /* Guardband is only used on a6xx so far: */
   if (!is_a6xx(ctx->screen))
      return;

   ctx->guardband.x = ~0;
   ctx->guardband.y = ~0;

   bool is3x = is_a3xx(ctx->screen);

   for (unsigned i = 0; i < PIPE_MAX_VIEWPORTS; i++) {
      const struct pipe_viewport_state *vp = & ctx->viewport[i];

      unsigned gx = fd_calc_guardband(vp->translate[0], vp->scale[0], is3x);
      unsigned gy = fd_calc_guardband(vp->translate[1], vp->scale[1], is3x);

      ctx->guardband.x = MIN2(ctx->guardband.x, gx);
      ctx->guardband.y = MIN2(ctx->guardband.y, gy);
   }
}

static void
fd_set_vertex_buffers(struct pipe_context *pctx,
                      unsigned count, unsigned unbind_num_trailing_slots,
                      bool take_ownership,
                      const struct pipe_vertex_buffer *vb) in_dt
{
   struct fd_context *ctx = fd_context(pctx);
   struct fd_vertexbuf_stateobj *so = &ctx->vtx.vertexbuf;
   int i;

   /* on a2xx, pitch is encoded in the vtx fetch instruction, so
    * we need to mark VTXSTATE as dirty as well to trigger patching
    * and re-emitting the vtx shader:
    */
   if (ctx->screen->gen < 3) {
      for (i = 0; i < count; i++) {
         bool new_enabled = vb && vb[i].buffer.resource;
         bool old_enabled = so->vb[i].buffer.resource != NULL;
         if (new_enabled != old_enabled) {
            fd_context_dirty(ctx, FD_DIRTY_VTXSTATE);
            break;
         }
      }
   }

   util_set_vertex_buffers_mask(so->vb, &so->enabled_mask, vb,
                                count, unbind_num_trailing_slots,
                                take_ownership);
   so->count = util_last_bit(so->enabled_mask);

   if (!vb)
      return;

   fd_context_dirty(ctx, FD_DIRTY_VTXBUF);

   for (unsigned i = 0; i < count; i++) {
      assert(!vb[i].is_user_buffer);
      fd_resource_set_usage(vb[i].buffer.resource, FD_DIRTY_VTXBUF);
      fd_dirty_resource(ctx, vb[i].buffer.resource, FD_DIRTY_VTXBUF, false);

      /* Robust buffer access: Return undefined data (the start of the buffer)
       * instead of process termination or a GPU hang in case of overflow.
       */
      if (vb[i].buffer.resource &&
          unlikely(vb[i].buffer_offset >= vb[i].buffer.resource->width0)) {
         so->vb[i].buffer_offset = 0;
      }
   }
}

static void
fd_blend_state_bind(struct pipe_context *pctx, void *hwcso) in_dt
{
   struct fd_context *ctx = fd_context(pctx);
   struct pipe_blend_state *cso = hwcso;
   bool old_is_dual = ctx->blend ? ctx->blend->rt[0].blend_enable &&
                                      util_blend_state_is_dual(ctx->blend, 0)
                                 : false;
   bool new_is_dual =
      cso ? cso->rt[0].blend_enable && util_blend_state_is_dual(cso, 0) : false;
   fd_context_dirty(ctx, FD_DIRTY_BLEND);
   if (old_is_dual != new_is_dual)
      fd_context_dirty(ctx, FD_DIRTY_BLEND_DUAL);

   bool old_coherent = get_safe(ctx->blend, blend_coherent);
   bool new_coherent = get_safe(cso, blend_coherent);
   if (new_coherent != old_coherent) {
      fd_context_dirty(ctx, FD_DIRTY_BLEND_COHERENT);
   }
   ctx->blend = hwcso;
   update_draw_cost(ctx);
}

static void
fd_blend_state_delete(struct pipe_context *pctx, void *hwcso) in_dt
{
   FREE(hwcso);
}

static void
fd_rasterizer_state_bind(struct pipe_context *pctx, void *hwcso) in_dt
{
   struct fd_context *ctx = fd_context(pctx);
   struct pipe_scissor_state *old_scissor = fd_context_get_scissor(ctx);
   bool discard = get_safe(ctx->rasterizer, rasterizer_discard);
   unsigned clip_plane_enable = get_safe(ctx->rasterizer, clip_plane_enable);

   ctx->rasterizer = hwcso;
   fd_context_dirty(ctx, FD_DIRTY_RASTERIZER);

   if (ctx->rasterizer && ctx->rasterizer->scissor) {
      ctx->current_scissor = ctx->scissor;
   } else {
      ctx->current_scissor = ctx->disabled_scissor;
   }

   /* if scissor enable bit changed we need to mark scissor
    * state as dirty as well:
    * NOTE: we can do a shallow compare, since we only care
    * if it changed to/from &ctx->disable_scissor
    */
   if (old_scissor != fd_context_get_scissor(ctx))
      fd_context_dirty(ctx, FD_DIRTY_SCISSOR);

   if (discard != get_safe(ctx->rasterizer, rasterizer_discard))
      fd_context_dirty(ctx, FD_DIRTY_RASTERIZER_DISCARD);

   if (clip_plane_enable != get_safe(ctx->rasterizer, clip_plane_enable))
      fd_context_dirty(ctx, FD_DIRTY_RASTERIZER_CLIP_PLANE_ENABLE);
}

static void
fd_rasterizer_state_delete(struct pipe_context *pctx, void *hwcso) in_dt
{
   FREE(hwcso);
}

static void
fd_zsa_state_bind(struct pipe_context *pctx, void *hwcso) in_dt
{
   struct fd_context *ctx = fd_context(pctx);
   ctx->zsa = hwcso;
   fd_context_dirty(ctx, FD_DIRTY_ZSA);
   update_draw_cost(ctx);
}

static void
fd_zsa_state_delete(struct pipe_context *pctx, void *hwcso) in_dt
{
   FREE(hwcso);
}

static void *
fd_vertex_state_create(struct pipe_context *pctx, unsigned num_elements,
                       const struct pipe_vertex_element *elements)
{
   struct fd_vertex_stateobj *so = CALLOC_STRUCT(fd_vertex_stateobj);

   if (!so)
      return NULL;

   memcpy(so->pipe, elements, sizeof(*elements) * num_elements);
   so->num_elements = num_elements;
   for (unsigned i = 0; i < num_elements; i++)
      so->strides[elements[i].vertex_buffer_index] = elements[i].src_stride;

   return so;
}

static void
fd_vertex_state_delete(struct pipe_context *pctx, void *hwcso) in_dt
{
   FREE(hwcso);
}

static void
fd_vertex_state_bind(struct pipe_context *pctx, void *hwcso) in_dt
{
   struct fd_context *ctx = fd_context(pctx);
   ctx->vtx.vtx = hwcso;
   fd_context_dirty(ctx, FD_DIRTY_VTXSTATE);
}

static struct pipe_stream_output_target *
fd_create_stream_output_target(struct pipe_context *pctx,
                               struct pipe_resource *prsc,
                               unsigned buffer_offset, unsigned buffer_size)
{
   struct fd_stream_output_target *target;
   struct fd_resource *rsc = fd_resource(prsc);

   target = CALLOC_STRUCT(fd_stream_output_target);
   if (!target)
      return NULL;

   pipe_reference_init(&target->base.reference, 1);
   pipe_resource_reference(&target->base.buffer, prsc);

   target->base.context = pctx;
   target->base.buffer_offset = buffer_offset;
   target->base.buffer_size = buffer_size;

   target->offset_buf = pipe_buffer_create(
      pctx->screen, PIPE_BIND_CUSTOM, PIPE_USAGE_IMMUTABLE, sizeof(uint32_t));

   assert(rsc->b.b.target == PIPE_BUFFER);
   util_range_add(&rsc->b.b, &rsc->valid_buffer_range, buffer_offset,
                  buffer_offset + buffer_size);

   return &target->base;
}

static void
fd_stream_output_target_destroy(struct pipe_context *pctx,
                                struct pipe_stream_output_target *target)
{
   struct fd_stream_output_target *cso = fd_stream_output_target(target);

   pipe_resource_reference(&cso->base.buffer, NULL);
   pipe_resource_reference(&cso->offset_buf, NULL);

   FREE(target);
}

static void
fd_set_stream_output_targets(struct pipe_context *pctx, unsigned num_targets,
                             struct pipe_stream_output_target **targets,
                             const unsigned *offsets) in_dt
{
   struct fd_context *ctx = fd_context(pctx);
   struct fd_streamout_stateobj *so = &ctx->streamout;
   unsigned i;

   assert(num_targets <= ARRAY_SIZE(so->targets));

   /* Older targets need sw stats enabled for streamout emulation in VS: */
   if (ctx->screen->gen < 5) {
      if (num_targets && !so->num_targets) {
         ctx->stats_users++;
      } else if (so->num_targets && !num_targets) {
         ctx->stats_users--;
      }
   }

   for (i = 0; i < num_targets; i++) {
      bool changed = targets[i] != so->targets[i];
      bool reset = (offsets[i] != (unsigned)-1);

      so->reset |= (reset << i);

      if (targets[i]) {
         fd_resource_set_usage(targets[i]->buffer, FD_DIRTY_STREAMOUT);
         fd_dirty_resource(ctx, targets[i]->buffer, FD_DIRTY_STREAMOUT, true);

         struct fd_stream_output_target *target = fd_stream_output_target(targets[i]);
         fd_resource_set_usage(target->offset_buf, FD_DIRTY_STREAMOUT);
         fd_dirty_resource(ctx, target->offset_buf, FD_DIRTY_STREAMOUT, true);
      }

      if (!changed && !reset)
         continue;

      /* Note that all SO targets will be reset at once at a
       * BeginTransformFeedback().
       */
      if (reset) {
         so->offsets[i] = offsets[i];
         ctx->streamout.verts_written = 0;
      }

      pipe_so_target_reference(&so->targets[i], targets[i]);
   }

   for (; i < so->num_targets; i++) {
      pipe_so_target_reference(&so->targets[i], NULL);
   }

   so->num_targets = num_targets;

   fd_context_dirty(ctx, FD_DIRTY_STREAMOUT);
}

static void
fd_bind_compute_state(struct pipe_context *pctx, void *state) in_dt
{
   struct fd_context *ctx = fd_context(pctx);
   ctx->compute = state;
   fd_context_dirty_shader(ctx, PIPE_SHADER_COMPUTE, FD_DIRTY_SHADER_PROG);
}

/* TODO pipe_context::set_compute_resources() should DIAF and clover
 * should be updated to use pipe_context::set_constant_buffer() and
 * pipe_context::set_shader_images().  Until then just directly frob
 * the UBO/image state to avoid the rest of the driver needing to
 * know about this bastard api..
 */
static void
fd_set_compute_resources(struct pipe_context *pctx, unsigned start,
                         unsigned count, struct pipe_surface **prscs) in_dt
{
   struct fd_context *ctx = fd_context(pctx);
   struct fd_constbuf_stateobj *so = &ctx->constbuf[PIPE_SHADER_COMPUTE];

   for (unsigned i = 0; i < count; i++) {
      const uint32_t index = i + start + 1;   /* UBOs start at index 1 */

      if (!prscs) {
         util_copy_constant_buffer(&so->cb[index], NULL, false);
         so->enabled_mask &= ~(1 << index);
      } else if (prscs[i]->format == PIPE_FORMAT_NONE) {
         struct pipe_constant_buffer cb = {
               .buffer = prscs[i]->texture,
         };
         util_copy_constant_buffer(&so->cb[index], &cb, false);
         so->enabled_mask |= (1 << index);
      } else {
         // TODO images
         unreachable("finishme");
      }
   }
}

/* used by clover to bind global objects, returning the bo address
 * via handles[n]
 */
static void
fd_set_global_binding(struct pipe_context *pctx, unsigned first, unsigned count,
                      struct pipe_resource **prscs, uint32_t **handles) in_dt
{
   struct fd_context *ctx = fd_context(pctx);
   struct fd_global_bindings_stateobj *so = &ctx->global_bindings;
   unsigned mask = 0;

   if (prscs) {
      for (unsigned i = 0; i < count; i++) {
         unsigned n = i + first;

         mask |= BIT(n);

         pipe_resource_reference(&so->buf[n], prscs[i]);

         if (so->buf[n]) {
            struct fd_resource *rsc = fd_resource(so->buf[n]);
            uint32_t offset = *handles[i];
            uint64_t iova = fd_bo_get_iova(rsc->bo) + offset;

            /* Yes, really, despite what the type implies: */
            memcpy(handles[i], &iova, sizeof(iova));
         }

         if (prscs[i])
            so->enabled_mask |= BIT(n);
         else
            so->enabled_mask &= ~BIT(n);
      }
   } else {
      mask = (BIT(count) - 1) << first;

      for (unsigned i = 0; i < count; i++) {
         unsigned n = i + first;
         pipe_resource_reference(&so->buf[n], NULL);
      }

      so->enabled_mask &= ~mask;
   }
}

void
fd_state_init(struct pipe_context *pctx)
{
   pctx->set_blend_color = fd_set_blend_color;
   pctx->set_stencil_ref = fd_set_stencil_ref;
   pctx->set_clip_state = fd_set_clip_state;
   pctx->set_sample_mask = fd_set_sample_mask;
   pctx->set_min_samples = fd_set_min_samples;
   pctx->set_constant_buffer = fd_set_constant_buffer;
   pctx->set_shader_buffers = fd_set_shader_buffers;
   pctx->set_shader_images = fd_set_shader_images;
   pctx->set_framebuffer_state = fd_set_framebuffer_state;
   pctx->set_sample_locations = fd_set_sample_locations;
   pctx->set_polygon_stipple = fd_set_polygon_stipple;
   pctx->set_scissor_states = fd_set_scissor_states;
   pctx->set_viewport_states = fd_set_viewport_states;

   pctx->set_vertex_buffers = fd_set_vertex_buffers;

   pctx->bind_blend_state = fd_blend_state_bind;
   pctx->delete_blend_state = fd_blend_state_delete;

   pctx->bind_rasterizer_state = fd_rasterizer_state_bind;
   pctx->delete_rasterizer_state = fd_rasterizer_state_delete;

   pctx->bind_depth_stencil_alpha_state = fd_zsa_state_bind;
   pctx->delete_depth_stencil_alpha_state = fd_zsa_state_delete;

   if (!pctx->create_vertex_elements_state)
      pctx->create_vertex_elements_state = fd_vertex_state_create;
   pctx->delete_vertex_elements_state = fd_vertex_state_delete;
   pctx->bind_vertex_elements_state = fd_vertex_state_bind;

   pctx->create_stream_output_target = fd_create_stream_output_target;
   pctx->stream_output_target_destroy = fd_stream_output_target_destroy;
   pctx->set_stream_output_targets = fd_set_stream_output_targets;

   if (has_compute(fd_screen(pctx->screen))) {
      pctx->bind_compute_state = fd_bind_compute_state;
      pctx->set_compute_resources = fd_set_compute_resources;
      pctx->set_global_binding = fd_set_global_binding;
   }

   init_scissor_states(pctx);
}
