/*
 * Copyright 2018 Collabora Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "zink_batch.h"
#include "zink_clear.h"
#include "zink_context.h"
#include "zink_format.h"
#include "zink_inlines.h"
#include "zink_query.h"

#include "util/u_blitter.h"
#include "util/format/u_format.h"
#include "util/format_srgb.h"
#include "util/u_framebuffer.h"
#include "util/u_inlines.h"
#include "util/u_rect.h"
#include "util/u_surface.h"
#include "util/u_helpers.h"

static inline bool
scissor_states_equal(const struct pipe_scissor_state *a, const struct pipe_scissor_state *b)
{
   return a->minx == b->minx && a->miny == b->miny && a->maxx == b->maxx && a->maxy == b->maxy;
}

static void
clear_in_rp(struct pipe_context *pctx,
           unsigned buffers,
           const struct pipe_scissor_state *scissor_state,
           const union pipe_color_union *pcolor,
           double depth, unsigned stencil)
{
   struct zink_context *ctx = zink_context(pctx);
   struct pipe_framebuffer_state *fb = &ctx->fb_state;

   zink_flush_dgc_if_enabled(ctx);

   VkClearAttachment attachments[1 + PIPE_MAX_COLOR_BUFS];
   int num_attachments = 0;

   if (buffers & PIPE_CLEAR_COLOR) {
      VkClearColorValue color;
      color.uint32[0] = pcolor->ui[0];
      color.uint32[1] = pcolor->ui[1];
      color.uint32[2] = pcolor->ui[2];
      color.uint32[3] = pcolor->ui[3];

      for (unsigned i = 0; i < fb->nr_cbufs; i++) {
         if (!(buffers & (PIPE_CLEAR_COLOR0 << i)) || !fb->cbufs[i])
            continue;

         attachments[num_attachments].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
         attachments[num_attachments].colorAttachment = i;
         attachments[num_attachments].clearValue.color = color;
         ++num_attachments;
      }
   }

   if (buffers & PIPE_CLEAR_DEPTHSTENCIL && fb->zsbuf) {
      VkImageAspectFlags aspect = 0;
      if (buffers & PIPE_CLEAR_DEPTH)
         aspect |= VK_IMAGE_ASPECT_DEPTH_BIT;
      if (buffers & PIPE_CLEAR_STENCIL)
         aspect |= VK_IMAGE_ASPECT_STENCIL_BIT;

      assert(zink_is_zsbuf_used(ctx));

      attachments[num_attachments].aspectMask = aspect;
      attachments[num_attachments].clearValue.depthStencil.depth = depth;
      attachments[num_attachments].clearValue.depthStencil.stencil = stencil;
      ++num_attachments;
   }

   VkClearRect cr = {0};
   if (scissor_state) {
      /* invalid clear */
      if (scissor_state->minx > ctx->fb_state.width || scissor_state->miny > ctx->fb_state.height)
         return;
      cr.rect.offset.x = scissor_state->minx;
      cr.rect.offset.y = scissor_state->miny;
      cr.rect.extent.width = MIN2(fb->width - cr.rect.offset.x, scissor_state->maxx - scissor_state->minx);
      cr.rect.extent.height = MIN2(fb->height - cr.rect.offset.y, scissor_state->maxy - scissor_state->miny);
   } else {
      cr.rect.extent.width = fb->width;
      cr.rect.extent.height = fb->height;
   }
   cr.baseArrayLayer = 0;
   cr.layerCount = util_framebuffer_get_num_layers(fb);
   struct zink_batch *batch = &ctx->batch;
   assert(batch->in_rp);
   VKCTX(CmdClearAttachments)(batch->state->cmdbuf, num_attachments, attachments, 1, &cr);
   /*
       Rendering within a subpass containing a feedback loop creates a data race, except in the following
       cases:
       • If a memory dependency is inserted between when the attachment is written and when it is
       subsequently read by later fragments. Pipeline barriers expressing a subpass self-dependency
       are the only way to achieve this, and one must be inserted every time a fragment will read
       values at a particular sample (x, y, layer, sample) coordinate, if those values have been written
       since the most recent pipeline barrier

       VK 1.3.211, Chapter 8: Render Pass
    */
   if (ctx->fbfetch_outputs)
      ctx->base.texture_barrier(&ctx->base, PIPE_TEXTURE_BARRIER_FRAMEBUFFER);
}

static struct zink_framebuffer_clear_data *
add_new_clear(struct zink_framebuffer_clear *fb_clear)
{
   struct zink_framebuffer_clear_data cd = {0};
   util_dynarray_append(&fb_clear->clears, struct zink_framebuffer_clear_data, cd);
   return zink_fb_clear_element(fb_clear, zink_fb_clear_count(fb_clear) - 1);
}

static struct zink_framebuffer_clear_data *
get_clear_data(struct zink_context *ctx, struct zink_framebuffer_clear *fb_clear, const struct pipe_scissor_state *scissor_state)
{
   unsigned num_clears = zink_fb_clear_count(fb_clear);
   if (num_clears) {
      struct zink_framebuffer_clear_data *last_clear = zink_fb_clear_element(fb_clear, num_clears - 1);
      /* if we're completely overwriting the previous clear, merge this into the previous clear */
      if (!scissor_state || (last_clear->has_scissor && scissor_states_equal(&last_clear->scissor, scissor_state)))
         return last_clear;
   }
   return add_new_clear(fb_clear);
}

void
zink_clear(struct pipe_context *pctx,
           unsigned buffers,
           const struct pipe_scissor_state *scissor_state,
           const union pipe_color_union *pcolor,
           double depth, unsigned stencil)
{
   struct zink_context *ctx = zink_context(pctx);
   struct zink_screen *screen = zink_screen(pctx->screen);
   struct pipe_framebuffer_state *fb = &ctx->fb_state;
   struct zink_batch *batch = &ctx->batch;
   bool needs_rp = false;

   if (scissor_state) {
      struct u_rect scissor = {scissor_state->minx, scissor_state->maxx, scissor_state->miny, scissor_state->maxy};
      needs_rp = !zink_blit_region_fills(scissor, fb->width, fb->height);
   }

   if (unlikely(ctx->fb_layer_mismatch)) {
      /* this is a terrible scenario:
       * at least one attachment has a layerCount greater than the others,
       * so iterate over all the mismatched attachments and pre-clear them separately,
       * then continue to flag them as need (additional) clearing
       * to avoid loadOp=LOAD
       */
      unsigned x = 0;
      unsigned y = 0;
      unsigned w = ctx->fb_state.width;
      unsigned h = ctx->fb_state.height;
      if (scissor_state) {
         x = scissor_state->minx;
         y = scissor_state->miny;
         w = scissor_state->minx + scissor_state->maxx;
         h = scissor_state->miny + scissor_state->maxy;
      }
      unsigned clear_buffers = buffers >> 2;
      for (unsigned i = 0; i < ctx->fb_state.nr_cbufs; i++) {
         if (ctx->fb_state.cbufs[i] &&
             (ctx->fb_layer_mismatch & clear_buffers & BITFIELD_BIT(i))) {
            if (ctx->void_clears & (PIPE_CLEAR_COLOR0 << i)) {
               union pipe_color_union color;
               color.f[0] = color.f[1] = color.f[2] = 0;
               color.f[3] = 1.0;
               pctx->clear_render_target(pctx, ctx->fb_state.cbufs[i], &color,
                                         0, 0,
                                         ctx->fb_state.cbufs[i]->width, ctx->fb_state.cbufs[i]->height,
                                         ctx->render_condition_active);
            }
            pctx->clear_render_target(pctx, ctx->fb_state.cbufs[i], pcolor,
                                      x, y, w, h, ctx->render_condition_active);
         }
      }
      if (ctx->fb_state.zsbuf && (buffers & PIPE_CLEAR_DEPTHSTENCIL))
         pctx->clear_depth_stencil(pctx, ctx->fb_state.zsbuf, buffers & PIPE_CLEAR_DEPTHSTENCIL, depth, stencil,
                                   x, y, w, h, ctx->render_condition_active);
   }

   if (batch->in_rp) {
      if (buffers & PIPE_CLEAR_DEPTHSTENCIL && (ctx->zsbuf_unused || ctx->zsbuf_readonly)) {
         /* this will need a layout change */
         assert(!ctx->track_renderpasses);
         zink_batch_no_rp(ctx);
      } else {
         clear_in_rp(pctx, buffers, scissor_state, pcolor, depth, stencil);
         return;
      }
   }

   unsigned rp_clears_enabled = ctx->rp_clears_enabled;

   if (ctx->void_clears & buffers) {
      unsigned void_clears = ctx->void_clears & buffers;
      ctx->void_clears &= ~buffers;
      union pipe_color_union color;
      color.f[0] = color.f[1] = color.f[2] = 0;
      color.f[3] = 1.0;
      for (unsigned i = 0; i < fb->nr_cbufs; i++) {
         if ((void_clears & (PIPE_CLEAR_COLOR0 << i)) && fb->cbufs[i]) {
            struct zink_framebuffer_clear *fb_clear = &ctx->fb_clears[i];
            unsigned num_clears = zink_fb_clear_count(fb_clear);
            if (num_clears) {
               if (zink_fb_clear_first_needs_explicit(fb_clear)) {
                  /* a scissored clear exists:
                   * - extend the clear array
                   * - shift existing clears back by one position
                   * - inject void clear base of array
                   */
                  add_new_clear(fb_clear);
                  struct zink_framebuffer_clear_data *clear = fb_clear->clears.data;
                  memcpy(clear + 1, clear, num_clears);
                  memcpy(&clear->color, &color, sizeof(color));
               } else {
                  /* no void clear needed */
               }
               void_clears &= ~(PIPE_CLEAR_COLOR0 << i);
            }
         }
      }
      if (void_clears)
         pctx->clear(pctx, void_clears, NULL, &color, 0, 0);
   }

   if (buffers & PIPE_CLEAR_COLOR) {
      for (unsigned i = 0; i < fb->nr_cbufs; i++) {
         if ((buffers & (PIPE_CLEAR_COLOR0 << i)) && fb->cbufs[i]) {
            struct pipe_surface *psurf = fb->cbufs[i];
            struct zink_framebuffer_clear *fb_clear = &ctx->fb_clears[i];
            struct zink_framebuffer_clear_data *clear = get_clear_data(ctx, fb_clear, needs_rp ? scissor_state : NULL);

            ctx->clears_enabled |= PIPE_CLEAR_COLOR0 << i;
            clear->conditional = ctx->render_condition_active;
            clear->has_scissor = needs_rp;
            memcpy(&clear->color, pcolor, sizeof(union pipe_color_union));
            zink_convert_color(screen, psurf->format, &clear->color, pcolor);
            if (scissor_state && needs_rp)
               clear->scissor = *scissor_state;
            if (zink_fb_clear_first_needs_explicit(fb_clear))
               ctx->rp_clears_enabled &= ~(PIPE_CLEAR_COLOR0 << i);
            else
               ctx->rp_clears_enabled |= PIPE_CLEAR_COLOR0 << i;
         }
      }
   }

   if (buffers & PIPE_CLEAR_DEPTHSTENCIL && fb->zsbuf) {
      struct zink_framebuffer_clear *fb_clear = &ctx->fb_clears[PIPE_MAX_COLOR_BUFS];
      struct zink_framebuffer_clear_data *clear = get_clear_data(ctx, fb_clear, needs_rp ? scissor_state : NULL);
      ctx->clears_enabled |= PIPE_CLEAR_DEPTHSTENCIL;
      clear->conditional = ctx->render_condition_active;
      clear->has_scissor = needs_rp;
      if (scissor_state && needs_rp)
         clear->scissor = *scissor_state;
      if (buffers & PIPE_CLEAR_DEPTH)
         clear->zs.depth = depth;
      if (buffers & PIPE_CLEAR_STENCIL)
         clear->zs.stencil = stencil;
      clear->zs.bits |= (buffers & PIPE_CLEAR_DEPTHSTENCIL);
      if (zink_fb_clear_first_needs_explicit(fb_clear)) {
         ctx->rp_clears_enabled &= ~PIPE_CLEAR_DEPTHSTENCIL;
         if (!ctx->track_renderpasses)
            ctx->dynamic_fb.tc_info.zsbuf_clear_partial = true;
      } else {
         ctx->rp_clears_enabled |= (buffers & PIPE_CLEAR_DEPTHSTENCIL);
         if (!ctx->track_renderpasses)
            ctx->dynamic_fb.tc_info.zsbuf_clear = true;
      }
   }
   assert(!ctx->batch.in_rp);
   ctx->rp_changed |= ctx->rp_clears_enabled != rp_clears_enabled;
}

static inline bool
colors_equal(union pipe_color_union *a, union pipe_color_union *b)
{
   return a->ui[0] == b->ui[0] && a->ui[1] == b->ui[1] && a->ui[2] == b->ui[2] && a->ui[3] == b->ui[3];
}

void
zink_clear_framebuffer(struct zink_context *ctx, unsigned clear_buffers)
{
   unsigned to_clear = 0;
   struct pipe_framebuffer_state *fb_state = &ctx->fb_state;
#ifndef NDEBUG
   assert(!(clear_buffers & PIPE_CLEAR_DEPTHSTENCIL) || zink_fb_clear_enabled(ctx, PIPE_MAX_COLOR_BUFS));
   for (int i = 0; i < fb_state->nr_cbufs && clear_buffers >= PIPE_CLEAR_COLOR0; i++) {
      assert(!(clear_buffers & (PIPE_CLEAR_COLOR0 << i)) || zink_fb_clear_enabled(ctx, i));
   }
#endif
   while (clear_buffers) {
      struct zink_framebuffer_clear *color_clear = NULL;
      struct zink_framebuffer_clear *zs_clear = NULL;
      unsigned num_clears = 0;
      for (int i = 0; i < fb_state->nr_cbufs && clear_buffers >= PIPE_CLEAR_COLOR0; i++) {
         struct zink_framebuffer_clear *fb_clear = &ctx->fb_clears[i];
         /* these need actual clear calls inside the rp */
         if (!(clear_buffers & (PIPE_CLEAR_COLOR0 << i)))
            continue;
         if (color_clear) {
            /* different number of clears -> do another clear */
            //XXX: could potentially merge "some" of the clears into this one for a very, very small optimization
            if (num_clears != zink_fb_clear_count(fb_clear))
               goto out;
            /* compare all the clears to determine if we can batch these buffers together */
            for (int j = !zink_fb_clear_first_needs_explicit(fb_clear); j < num_clears; j++) {
               struct zink_framebuffer_clear_data *a = zink_fb_clear_element(color_clear, j);
               struct zink_framebuffer_clear_data *b = zink_fb_clear_element(fb_clear, j);
               /* scissors don't match, fire this one off */
               if (a->has_scissor != b->has_scissor || (a->has_scissor && !scissor_states_equal(&a->scissor, &b->scissor)))
                  goto out;

               /* colors don't match, fire this one off */
               if (!colors_equal(&a->color, &b->color))
                  goto out;
            }
         } else {
            color_clear = fb_clear;
            num_clears = zink_fb_clear_count(fb_clear);
         }

         clear_buffers &= ~(PIPE_CLEAR_COLOR0 << i);
         to_clear |= (PIPE_CLEAR_COLOR0 << i);
      }
      clear_buffers &= ~PIPE_CLEAR_COLOR;
      if (clear_buffers & PIPE_CLEAR_DEPTHSTENCIL) {
         struct zink_framebuffer_clear *fb_clear = &ctx->fb_clears[PIPE_MAX_COLOR_BUFS];
         if (color_clear) {
            if (num_clears != zink_fb_clear_count(fb_clear))
               goto out;
            /* compare all the clears to determine if we can batch these buffers together */
            for (int j = !zink_fb_clear_first_needs_explicit(fb_clear); j < zink_fb_clear_count(color_clear); j++) {
               struct zink_framebuffer_clear_data *a = zink_fb_clear_element(color_clear, j);
               struct zink_framebuffer_clear_data *b = zink_fb_clear_element(fb_clear, j);
               /* scissors don't match, fire this one off */
               if (a->has_scissor != b->has_scissor || (a->has_scissor && !scissor_states_equal(&a->scissor, &b->scissor)))
                  goto out;
            }
         }
         zs_clear = fb_clear;
         to_clear |= (clear_buffers & PIPE_CLEAR_DEPTHSTENCIL);
         clear_buffers &= ~PIPE_CLEAR_DEPTHSTENCIL;
      }
out:
      if (to_clear) {
         if (num_clears) {
            for (int j = !zink_fb_clear_first_needs_explicit(color_clear); j < num_clears; j++) {
               struct zink_framebuffer_clear_data *clear = zink_fb_clear_element(color_clear, j);
               struct zink_framebuffer_clear_data *zsclear = NULL;
               /* zs bits are both set here if those aspects should be cleared at some point */
               unsigned clear_bits = to_clear & ~PIPE_CLEAR_DEPTHSTENCIL;
               if (zs_clear) {
                  zsclear = zink_fb_clear_element(zs_clear, j);
                  clear_bits |= zsclear->zs.bits;
               }
               zink_clear(&ctx->base, clear_bits,
                          clear->has_scissor ? &clear->scissor : NULL,
                          &clear->color,
                          zsclear ? zsclear->zs.depth : 0,
                          zsclear ? zsclear->zs.stencil : 0);
            }
         } else {
            for (int j = !zink_fb_clear_first_needs_explicit(zs_clear); j < zink_fb_clear_count(zs_clear); j++) {
               struct zink_framebuffer_clear_data *clear = zink_fb_clear_element(zs_clear, j);
               zink_clear(&ctx->base, clear->zs.bits,
                          clear->has_scissor ? &clear->scissor : NULL,
                          NULL,
                          clear->zs.depth,
                          clear->zs.stencil);
            }
         }
      }
      to_clear = 0;
   }
   if (ctx->clears_enabled & PIPE_CLEAR_DEPTHSTENCIL)
      zink_fb_clear_reset(ctx, PIPE_MAX_COLOR_BUFS);
   u_foreach_bit(i, ctx->clears_enabled >> 2)
      zink_fb_clear_reset(ctx, i);
}

static struct pipe_surface *
create_clear_surface(struct pipe_context *pctx, struct pipe_resource *pres, unsigned level, const struct pipe_box *box)
{
   struct pipe_surface tmpl = {{0}};

   tmpl.format = pres->format;
   tmpl.u.tex.first_layer = box->z;
   tmpl.u.tex.last_layer = box->z + box->depth - 1;
   tmpl.u.tex.level = level;
   return pctx->create_surface(pctx, pres, &tmpl);
}

static void
set_clear_fb(struct pipe_context *pctx, struct pipe_surface *psurf, struct pipe_surface *zsurf)
{
   struct pipe_framebuffer_state fb_state = {0};
   fb_state.width = psurf ? psurf->width : zsurf->width;
   fb_state.height = psurf ? psurf->height : zsurf->height;
   fb_state.nr_cbufs = !!psurf;
   fb_state.cbufs[0] = psurf;
   fb_state.zsbuf = zsurf;
   pctx->set_framebuffer_state(pctx, &fb_state);
}

void
zink_clear_texture_dynamic(struct pipe_context *pctx,
                           struct pipe_resource *pres,
                           unsigned level,
                           const struct pipe_box *box,
                           const void *data)
{
   struct zink_context *ctx = zink_context(pctx);
   struct zink_screen *screen = zink_screen(pctx->screen);
   struct zink_resource *res = zink_resource(pres);

   bool full_clear = 0 <= box->x && u_minify(pres->width0, level) >= box->x + box->width &&
                     0 <= box->y && u_minify(pres->height0, level) >= box->y + box->height &&
                     0 <= box->z && u_minify(pres->target == PIPE_TEXTURE_3D ? pres->depth0 : pres->array_size, level) >= box->z + box->depth;

   struct pipe_surface *surf = create_clear_surface(pctx, pres, level, box);

   VkRenderingAttachmentInfo att = {0};
   att.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
   att.imageView = zink_csurface(surf)->image_view;
   att.imageLayout = res->aspect & VK_IMAGE_ASPECT_COLOR_BIT ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
   att.loadOp = full_clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
   att.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

   VkRenderingInfo info = {0};
   info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
   info.renderArea.offset.x = box->x;
   info.renderArea.offset.y = box->y;
   info.renderArea.extent.width = box->width;
   info.renderArea.extent.height = box->height;
   info.layerCount = MAX2(box->depth, 1);

   union pipe_color_union color, tmp;
   float depth = 0.0;
   uint8_t stencil = 0;
   if (res->aspect & VK_IMAGE_ASPECT_COLOR_BIT) {
      util_format_unpack_rgba(pres->format, tmp.ui, data, 1);
      zink_convert_color(screen, surf->format, &color, &tmp);
   } else {
      if (res->aspect & VK_IMAGE_ASPECT_DEPTH_BIT)
         util_format_unpack_z_float(pres->format, &depth, data, 1);

      if (res->aspect & VK_IMAGE_ASPECT_STENCIL_BIT)
         util_format_unpack_s_8uint(pres->format, &stencil, data, 1);
   }

   zink_blit_barriers(ctx, NULL, res, full_clear);
   VkCommandBuffer cmdbuf = zink_get_cmdbuf(ctx, NULL, res);
   if (cmdbuf == ctx->batch.state->cmdbuf && ctx->batch.in_rp)
      zink_batch_no_rp(ctx);

   if (res->aspect & VK_IMAGE_ASPECT_COLOR_BIT) {
      memcpy(&att.clearValue, &color, sizeof(float) * 4);
      info.colorAttachmentCount = 1;
      info.pColorAttachments = &att;
   } else {
      att.clearValue.depthStencil.depth = depth;
      att.clearValue.depthStencil.stencil = stencil;
      if (res->aspect & VK_IMAGE_ASPECT_DEPTH_BIT)
         info.pDepthAttachment = &att;
      if (res->aspect & VK_IMAGE_ASPECT_STENCIL_BIT)
         info.pStencilAttachment = &att;
   }
   VKCTX(CmdBeginRendering)(cmdbuf, &info);
   if (!full_clear) {
      VkClearRect rect;
      rect.rect = info.renderArea;
      rect.baseArrayLayer = box->z;
      rect.layerCount = box->depth;

      VkClearAttachment clear_att;
      clear_att.aspectMask = res->aspect;
      clear_att.colorAttachment = 0;
      clear_att.clearValue = att.clearValue;

      VKCTX(CmdClearAttachments)(cmdbuf, 1, &clear_att, 1, &rect);
   }
   VKCTX(CmdEndRendering)(cmdbuf);
   zink_batch_reference_resource_rw(&ctx->batch, res, true);
   /* this will never destroy the surface */
   pipe_surface_reference(&surf, NULL);
}

void
zink_clear_texture(struct pipe_context *pctx,
                   struct pipe_resource *pres,
                   unsigned level,
                   const struct pipe_box *box,
                   const void *data)
{
   struct zink_context *ctx = zink_context(pctx);
   struct zink_resource *res = zink_resource(pres);
   struct pipe_surface *surf = NULL;
   struct pipe_scissor_state scissor = {box->x, box->y, box->x + box->width, box->y + box->height};

   if (res->aspect & VK_IMAGE_ASPECT_COLOR_BIT) {
      union pipe_color_union color;

      util_format_unpack_rgba(pres->format, color.ui, data, 1);

      surf = create_clear_surface(pctx, pres, level, box);
      util_blitter_save_framebuffer(ctx->blitter, &ctx->fb_state);
      set_clear_fb(pctx, surf, NULL);
      zink_blit_barriers(ctx, NULL, res, false);
      ctx->blitting = true;
      ctx->queries_disabled = true;
      pctx->clear(pctx, PIPE_CLEAR_COLOR0, &scissor, &color, 0, 0);
      util_blitter_restore_fb_state(ctx->blitter);
      ctx->queries_disabled = false;
      ctx->blitting = false;
   } else {
      float depth = 0.0;
      uint8_t stencil = 0;

      if (res->aspect & VK_IMAGE_ASPECT_DEPTH_BIT)
         util_format_unpack_z_float(pres->format, &depth, data, 1);

      if (res->aspect & VK_IMAGE_ASPECT_STENCIL_BIT)
         util_format_unpack_s_8uint(pres->format, &stencil, data, 1);

      unsigned flags = 0;
      if (res->aspect & VK_IMAGE_ASPECT_DEPTH_BIT)
         flags |= PIPE_CLEAR_DEPTH;
      if (res->aspect & VK_IMAGE_ASPECT_STENCIL_BIT)
         flags |= PIPE_CLEAR_STENCIL;
      surf = create_clear_surface(pctx, pres, level, box);
      util_blitter_save_framebuffer(ctx->blitter, &ctx->fb_state);
      zink_blit_barriers(ctx, NULL, res, false);
      ctx->blitting = true;
      set_clear_fb(pctx, NULL, surf);
      ctx->queries_disabled = true;
      pctx->clear(pctx, flags, &scissor, NULL, depth, stencil);
      util_blitter_restore_fb_state(ctx->blitter);
      ctx->queries_disabled = false;
      ctx->blitting = false;
   }
   /* this will never destroy the surface */
   pipe_surface_reference(&surf, NULL);
}

void
zink_clear_buffer(struct pipe_context *pctx,
                  struct pipe_resource *pres,
                  unsigned offset,
                  unsigned size,
                  const void *clear_value,
                  int clear_value_size)
{
   struct zink_context *ctx = zink_context(pctx);
   struct zink_resource *res = zink_resource(pres);

   uint32_t clamped;
   if (util_lower_clearsize_to_dword(clear_value, &clear_value_size, &clamped))
      clear_value = &clamped;
   if (offset % 4 == 0 && size % 4 == 0 && clear_value_size == sizeof(uint32_t)) {
      /*
         - dstOffset is the byte offset into the buffer at which to start filling,
           and must be a multiple of 4.

         - size is the number of bytes to fill, and must be either a multiple of 4,
           or VK_WHOLE_SIZE to fill the range from offset to the end of the buffer
       */
      zink_resource_buffer_transfer_dst_barrier(ctx, res, offset, size);
      VkCommandBuffer cmdbuf = zink_get_cmdbuf(ctx, NULL, res);
      zink_batch_reference_resource_rw(&ctx->batch, res, true);
      VKCTX(CmdFillBuffer)(cmdbuf, res->obj->buffer, offset, size, *(uint32_t*)clear_value);
      return;
   }
   struct pipe_transfer *xfer;
   uint8_t *map = pipe_buffer_map_range(pctx, pres, offset, size,
                                        PIPE_MAP_WRITE | PIPE_MAP_ONCE | PIPE_MAP_DISCARD_RANGE, &xfer);
   if (!map)
      return;
   unsigned rem = size % clear_value_size;
   uint8_t *ptr = map;
   for (unsigned i = 0; i < (size - rem) / clear_value_size; i++) {
      memcpy(ptr, clear_value, clear_value_size);
      ptr += clear_value_size;
   }
   if (rem)
      memcpy(map + size - rem, clear_value, rem);
   pipe_buffer_unmap(pctx, xfer);
}

void
zink_clear_render_target(struct pipe_context *pctx, struct pipe_surface *dst,
                         const union pipe_color_union *color, unsigned dstx,
                         unsigned dsty, unsigned width, unsigned height,
                         bool render_condition_enabled)
{
   struct zink_context *ctx = zink_context(pctx);
   zink_flush_dgc_if_enabled(ctx);
   bool render_condition_active = ctx->render_condition_active;
   if (!render_condition_enabled && render_condition_active) {
      zink_stop_conditional_render(ctx);
      ctx->render_condition_active = false;
   }
   util_blitter_save_framebuffer(ctx->blitter, &ctx->fb_state);
   set_clear_fb(pctx, dst, NULL);
   struct pipe_scissor_state scissor = {dstx, dsty, dstx + width, dsty + height};
   zink_blit_barriers(ctx, NULL, zink_resource(dst->texture), false);
   ctx->blitting = true;
   pctx->clear(pctx, PIPE_CLEAR_COLOR0, &scissor, color, 0, 0);
   util_blitter_restore_fb_state(ctx->blitter);
   ctx->blitting = false;
   if (!render_condition_enabled && render_condition_active)
      zink_start_conditional_render(ctx);
   ctx->render_condition_active = render_condition_active;
}

void
zink_clear_depth_stencil(struct pipe_context *pctx, struct pipe_surface *dst,
                         unsigned clear_flags, double depth, unsigned stencil,
                         unsigned dstx, unsigned dsty, unsigned width, unsigned height,
                         bool render_condition_enabled)
{
   struct zink_context *ctx = zink_context(pctx);
   /* check for stencil fallback */
   bool blitting = ctx->blitting;
   zink_flush_dgc_if_enabled(ctx);
   bool render_condition_active = ctx->render_condition_active;
   if (!render_condition_enabled && render_condition_active) {
      zink_stop_conditional_render(ctx);
      ctx->render_condition_active = false;
   }
   bool cur_attachment = zink_csurface(ctx->fb_state.zsbuf) == zink_csurface(dst);
   if (dstx > ctx->fb_state.width || dsty > ctx->fb_state.height ||
       dstx + width > ctx->fb_state.width ||
       dsty + height > ctx->fb_state.height)
      cur_attachment = false;
   if (!cur_attachment) {
      if (!blitting) {
         util_blitter_save_framebuffer(ctx->blitter, &ctx->fb_state);
         set_clear_fb(pctx, NULL, dst);
         zink_blit_barriers(ctx, NULL, zink_resource(dst->texture), false);
         ctx->blitting = true;
      }
   }
   struct pipe_scissor_state scissor = {dstx, dsty, dstx + width, dsty + height};
   pctx->clear(pctx, clear_flags, &scissor, NULL, depth, stencil);
   if (!cur_attachment && !blitting) {
      util_blitter_restore_fb_state(ctx->blitter);
      ctx->blitting = false;
   }
   if (!render_condition_enabled && render_condition_active)
      zink_start_conditional_render(ctx);
   ctx->render_condition_active = render_condition_active;
}

bool
zink_fb_clear_needs_explicit(struct zink_framebuffer_clear *fb_clear)
{
   if (zink_fb_clear_count(fb_clear) != 1)
      return true;
   return zink_fb_clear_element_needs_explicit(zink_fb_clear_element(fb_clear, 0));
}

bool
zink_fb_clear_first_needs_explicit(struct zink_framebuffer_clear *fb_clear)
{
   if (!zink_fb_clear_count(fb_clear))
      return false;
   return zink_fb_clear_element_needs_explicit(zink_fb_clear_element(fb_clear, 0));
}

static void
fb_clears_apply_internal(struct zink_context *ctx, struct pipe_resource *pres, int i)
{
   if (!zink_fb_clear_enabled(ctx, i))
      return;
   if (ctx->batch.in_rp)
      zink_clear_framebuffer(ctx, BITFIELD_BIT(i));
   else {
      struct zink_resource *res = zink_resource(pres);
      bool queries_disabled = ctx->queries_disabled;
      VkCommandBuffer cmdbuf = ctx->batch.state->cmdbuf;
      /* slightly different than the u_blitter handling:
       * this can be called recursively while unordered_blitting=true
       */
      bool can_reorder = zink_screen(ctx->base.screen)->info.have_KHR_dynamic_rendering &&
                         !ctx->render_condition_active &&
                         !ctx->unordered_blitting &&
                         zink_get_cmdbuf(ctx, NULL, res) == ctx->batch.state->reordered_cmdbuf;
      if (can_reorder) {
         /* set unordered_blitting but NOT blitting:
          * let begin_rendering handle layouts
          */
         ctx->unordered_blitting = true;
         /* for unordered clears, swap the unordered cmdbuf for the main one for the whole op to avoid conditional hell */
         ctx->batch.state->cmdbuf = ctx->batch.state->reordered_cmdbuf;
         ctx->rp_changed = true;
         ctx->queries_disabled = true;
         ctx->batch.state->has_barriers = true;
      }
      /* this will automatically trigger all the clears */
      zink_batch_rp(ctx);
      if (can_reorder) {
         zink_batch_no_rp(ctx);
         ctx->unordered_blitting = false;
         ctx->rp_changed = true;
         ctx->queries_disabled = queries_disabled;
         ctx->batch.state->cmdbuf = cmdbuf;
      }
   }
   zink_fb_clear_reset(ctx, i);
}

void
zink_fb_clear_reset(struct zink_context *ctx, unsigned i)
{
   unsigned rp_clears_enabled = ctx->clears_enabled;
   util_dynarray_clear(&ctx->fb_clears[i].clears);
   if (i == PIPE_MAX_COLOR_BUFS) {
      ctx->clears_enabled &= ~PIPE_CLEAR_DEPTHSTENCIL;
      ctx->rp_clears_enabled &= ~PIPE_CLEAR_DEPTHSTENCIL;
   } else {
      ctx->clears_enabled &= ~(PIPE_CLEAR_COLOR0 << i);
      ctx->rp_clears_enabled &= ~(PIPE_CLEAR_COLOR0 << i);
   }
   if (ctx->rp_clears_enabled != rp_clears_enabled)
      ctx->rp_loadop_changed = true;
}

void
zink_fb_clears_apply(struct zink_context *ctx, struct pipe_resource *pres)
{
   if (zink_resource(pres)->aspect == VK_IMAGE_ASPECT_COLOR_BIT) {
      for (int i = 0; i < ctx->fb_state.nr_cbufs; i++) {
         if (ctx->fb_state.cbufs[i] && ctx->fb_state.cbufs[i]->texture == pres) {
            fb_clears_apply_internal(ctx, pres, i);
         }
      }
   } else {
      if (ctx->fb_state.zsbuf && ctx->fb_state.zsbuf->texture == pres) {
         fb_clears_apply_internal(ctx, pres, PIPE_MAX_COLOR_BUFS);
      }
   }
}

void
zink_fb_clears_discard(struct zink_context *ctx, struct pipe_resource *pres)
{
   if (zink_resource(pres)->aspect == VK_IMAGE_ASPECT_COLOR_BIT) {
      for (int i = 0; i < ctx->fb_state.nr_cbufs; i++) {
         if (ctx->fb_state.cbufs[i] && ctx->fb_state.cbufs[i]->texture == pres) {
            if (zink_fb_clear_enabled(ctx, i)) {
               zink_fb_clear_reset(ctx, i);
            }
         }
      }
   } else {
      if (zink_fb_clear_enabled(ctx, PIPE_MAX_COLOR_BUFS) && ctx->fb_state.zsbuf && ctx->fb_state.zsbuf->texture == pres) {
         int i = PIPE_MAX_COLOR_BUFS;
         zink_fb_clear_reset(ctx, i);
      }
   }
}

void
zink_clear_apply_conditionals(struct zink_context *ctx)
{
   for (int i = 0; i < ARRAY_SIZE(ctx->fb_clears); i++) {
      struct zink_framebuffer_clear *fb_clear = &ctx->fb_clears[i];
      if (!zink_fb_clear_enabled(ctx, i))
         continue;
      for (int j = 0; j < zink_fb_clear_count(fb_clear); j++) {
         struct zink_framebuffer_clear_data *clear = zink_fb_clear_element(fb_clear, j);
         if (clear->conditional) {
            struct pipe_surface *surf;
            if (i < PIPE_MAX_COLOR_BUFS)
               surf = ctx->fb_state.cbufs[i];
            else
               surf = ctx->fb_state.zsbuf;
            if (surf)
               fb_clears_apply_internal(ctx, surf->texture, i);
            else
               zink_fb_clear_reset(ctx, i);
            break;
         }
      }
   }
}

static void
fb_clears_apply_or_discard_internal(struct zink_context *ctx, struct pipe_resource *pres, struct u_rect region, bool discard_only, bool invert, int i)
{
   struct zink_framebuffer_clear *fb_clear = &ctx->fb_clears[i];
   if (zink_fb_clear_enabled(ctx, i)) {
      if (zink_blit_region_fills(region, pres->width0, pres->height0)) {
         if (invert)
            fb_clears_apply_internal(ctx, pres, i);
         else
            /* we know we can skip these */
            zink_fb_clears_discard(ctx, pres);
         return;
      }
      for (int j = 0; j < zink_fb_clear_count(fb_clear); j++) {
         struct zink_framebuffer_clear_data *clear = zink_fb_clear_element(fb_clear, j);
         struct u_rect scissor = {clear->scissor.minx, clear->scissor.maxx,
                                  clear->scissor.miny, clear->scissor.maxy};
         if (!clear->has_scissor || zink_blit_region_covers(region, scissor)) {
            /* this is a clear that isn't fully covered by our pending write */
            if (!discard_only)
               fb_clears_apply_internal(ctx, pres, i);
            return;
         }
      }
      if (!invert)
         /* if we haven't already returned, then we know we can discard */
         zink_fb_clears_discard(ctx, pres);
   }
}

void
zink_fb_clears_apply_or_discard(struct zink_context *ctx, struct pipe_resource *pres, struct u_rect region, bool discard_only)
{
   if (zink_resource(pres)->aspect == VK_IMAGE_ASPECT_COLOR_BIT) {
      for (int i = 0; i < ctx->fb_state.nr_cbufs; i++) {
         if (ctx->fb_state.cbufs[i] && ctx->fb_state.cbufs[i]->texture == pres) {
            fb_clears_apply_or_discard_internal(ctx, pres, region, discard_only, false, i);
         }
      }
   }  else {
      if (zink_fb_clear_enabled(ctx, PIPE_MAX_COLOR_BUFS) && ctx->fb_state.zsbuf && ctx->fb_state.zsbuf->texture == pres) {
         fb_clears_apply_or_discard_internal(ctx, pres, region, discard_only, false, PIPE_MAX_COLOR_BUFS);
      }
   }
}

void
zink_fb_clears_apply_region(struct zink_context *ctx, struct pipe_resource *pres, struct u_rect region)
{
   if (zink_resource(pres)->aspect == VK_IMAGE_ASPECT_COLOR_BIT) {
      for (int i = 0; i < ctx->fb_state.nr_cbufs; i++) {
         if (ctx->fb_state.cbufs[i] && ctx->fb_state.cbufs[i]->texture == pres) {
            fb_clears_apply_or_discard_internal(ctx, pres, region, false, true, i);
         }
      }
   }  else {
      if (ctx->fb_state.zsbuf && ctx->fb_state.zsbuf->texture == pres) {
         fb_clears_apply_or_discard_internal(ctx, pres, region, false, true, PIPE_MAX_COLOR_BUFS);
      }
   }
}

void
zink_fb_clear_rewrite(struct zink_context *ctx, unsigned idx, enum pipe_format before, enum pipe_format after)
{
   /* if the values for the clear color are incompatible, they must be rewritten;
    * this occurs if:
    * - the formats' srgb-ness does not match
    * - the formats' signedness does not match
    */
   const struct util_format_description *bdesc = util_format_description(before);
   const struct util_format_description *adesc = util_format_description(after);
   bool bsigned = bdesc->channel[util_format_get_first_non_void_channel(before)].type == UTIL_FORMAT_TYPE_SIGNED;
   bool asigned = adesc->channel[util_format_get_first_non_void_channel(after)].type == UTIL_FORMAT_TYPE_SIGNED;
   if (util_format_is_srgb(before) == util_format_is_srgb(after) &&
       bsigned == asigned)
      return;
   struct zink_framebuffer_clear *fb_clear = &ctx->fb_clears[idx];
   for (int j = 0; j < zink_fb_clear_count(fb_clear); j++) {
      struct zink_framebuffer_clear_data *clear = zink_fb_clear_element(fb_clear, j);
      uint32_t data[4];
      util_format_pack_rgba(before, data, clear->color.ui, 1);
      util_format_unpack_rgba(after, clear->color.ui, data, 1);
   }
}
