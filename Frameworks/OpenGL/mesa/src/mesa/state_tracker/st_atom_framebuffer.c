/**************************************************************************
 * 
 * Copyright 2007 VMware, Inc.
 * All Rights Reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 **************************************************************************/

 /*
  * Authors:
  *   Keith Whitwell <keithw@vmware.com>
  *   Brian Paul
  */
 
#include <limits.h>

#include "st_context.h"
#include "st_atom.h"
#include "st_cb_bitmap.h"
#include "st_manager.h"
#include "st_texture.h"
#include "st_util.h"
#include "pipe/p_context.h"
#include "cso_cache/cso_context.h"
#include "util/u_math.h"
#include "util/u_inlines.h"
#include "util/format/u_format.h"
#include "util/u_framebuffer.h"
#include "main/framebuffer.h"

#include "main/renderbuffer.h"

/**
 * Update framebuffer size.
 *
 * We need to derive pipe_framebuffer size from the bound pipe_surfaces here
 * instead of copying gl_framebuffer size because for certain target types
 * (like PIPE_TEXTURE_1D_ARRAY) gl_framebuffer::Height has the number of layers
 * instead of 1.
 */
static void
update_framebuffer_size(struct pipe_framebuffer_state *framebuffer,
                        struct pipe_surface *surface)
{
   assert(surface);
   assert(surface->width  < USHRT_MAX);
   assert(surface->height < USHRT_MAX);
   framebuffer->width  = MIN2(framebuffer->width,  surface->width);
   framebuffer->height = MIN2(framebuffer->height, surface->height);
}

/**
 * Round up the requested multisample count to the next supported sample size.
 */
static unsigned
framebuffer_quantize_num_samples(struct st_context *st, unsigned num_samples)
{
   struct pipe_screen *screen = st->screen;
   int quantized_samples = 0;
   unsigned msaa_mode;

   if (!num_samples)
      return 0;

   /* Assumes the highest supported MSAA is a power of 2 */
   msaa_mode = util_next_power_of_two(st->ctx->Const.MaxFramebufferSamples);
   assert(!(num_samples > msaa_mode)); /* be safe from infinite loops */

   /**
    * Check if the MSAA mode that is higher than the requested
    * num_samples is supported, and if so returning it.
    */
   for (; msaa_mode >= num_samples; msaa_mode = msaa_mode / 2) {
      /**
       * For ARB_framebuffer_no_attachment, A format of
       * PIPE_FORMAT_NONE implies what number of samples is
       * supported for a framebuffer with no attachment. Thus the
       * drivers callback must be adjusted for this.
       */
      if (screen->is_format_supported(screen, PIPE_FORMAT_NONE,
                                      PIPE_TEXTURE_2D, msaa_mode, msaa_mode,
                                      PIPE_BIND_RENDER_TARGET))
         quantized_samples = msaa_mode;
   }
   return quantized_samples;
}

/**
 * Update framebuffer state (color, depth, stencil, etc. buffers)
 */
void
st_update_framebuffer_state( struct st_context *st )
{
   struct gl_context *ctx = st->ctx;
   struct pipe_framebuffer_state framebuffer;
   struct gl_framebuffer *fb = st->ctx->DrawBuffer;
   struct gl_renderbuffer *rb;
   GLuint i;

   /* Window framebuffer changes are received here. */
   st_manager_validate_framebuffers(st);

   st_flush_bitmap_cache(st);
   st_invalidate_readpix_cache(st);

   st->state.fb_orientation = _mesa_fb_orientation(fb);

   /**
    * Quantize the derived default number of samples:
    *
    * A query to the driver of supported MSAA values the
    * hardware supports is done as to legalize the number
    * of application requested samples, NumSamples.
    * See commit eb9cf3c for more information.
    */
   fb->DefaultGeometry._NumSamples =
      framebuffer_quantize_num_samples(st, fb->DefaultGeometry.NumSamples);

   framebuffer.width  = _mesa_geometric_width(fb);
   framebuffer.height = _mesa_geometric_height(fb);
   framebuffer.samples = _mesa_geometric_samples(fb);
   framebuffer.layers = _mesa_geometric_layers(fb);
   framebuffer.resolve = fb->resolve;

   /* Examine Mesa's ctx->DrawBuffer->_ColorDrawBuffers state
    * to determine which surfaces to draw to
    */
   framebuffer.nr_cbufs = fb->_NumColorDrawBuffers;

   for (i = 0; i < fb->_NumColorDrawBuffers; i++) {
      framebuffer.cbufs[i] = NULL;
      rb = fb->_ColorDrawBuffers[i];

      if (rb) {
         if (rb->is_rtt || (rb->texture &&
             _mesa_is_format_srgb(rb->Format))) {
            /* rendering to a GL texture, may have to update surface */

            _mesa_update_renderbuffer_surface(ctx, rb);
         }

         if (rb->surface) {
            if (rb->surface->context != st->pipe) {
               _mesa_regen_renderbuffer_surface(ctx, rb);
            }
            framebuffer.cbufs[i] = rb->surface;
            update_framebuffer_size(&framebuffer, rb->surface);
         }
         rb->defined = GL_TRUE; /* we'll be drawing something */
      }
   }

   for (i = framebuffer.nr_cbufs; i < PIPE_MAX_COLOR_BUFS; i++) {
      framebuffer.cbufs[i] = NULL;
   }

   /* Remove trailing GL_NONE draw buffers. */
   while (framebuffer.nr_cbufs &&
          !framebuffer.cbufs[framebuffer.nr_cbufs-1]) {
      framebuffer.nr_cbufs--;
   }

   /*
    * Depth/Stencil renderbuffer/surface.
    */
   rb = fb->Attachment[BUFFER_DEPTH].Renderbuffer;
   if (!rb)
      rb = fb->Attachment[BUFFER_STENCIL].Renderbuffer;

   if (rb) {
      if (rb->is_rtt) {
         /* rendering to a GL texture, may have to update surface */
         _mesa_update_renderbuffer_surface(ctx, rb);
      }
      if (rb->surface && rb->surface->context != ctx->pipe) {
         _mesa_regen_renderbuffer_surface(ctx, rb);
      }
      framebuffer.zsbuf = rb->surface;
      if (rb->surface)
         update_framebuffer_size(&framebuffer, rb->surface);
   }
   else
      framebuffer.zsbuf = NULL;

#ifndef NDEBUG
   /* Make sure the resource binding flags were set properly */
   for (i = 0; i < framebuffer.nr_cbufs; i++) {
      assert(!framebuffer.cbufs[i] ||
             framebuffer.cbufs[i]->texture->bind & PIPE_BIND_RENDER_TARGET);
   }
   if (framebuffer.zsbuf) {
      assert(framebuffer.zsbuf->texture->bind & PIPE_BIND_DEPTH_STENCIL);
   }
#endif

   if (framebuffer.width == USHRT_MAX)
      framebuffer.width = 0;
   if (framebuffer.height == USHRT_MAX)
      framebuffer.height = 0;

   cso_set_framebuffer(st->cso_context, &framebuffer);

   st->state.fb_width = framebuffer.width;
   st->state.fb_height = framebuffer.height;
   st->state.fb_num_samples = util_framebuffer_get_num_samples(&framebuffer);
   st->state.fb_num_layers = util_framebuffer_get_num_layers(&framebuffer);
   st->state.fb_num_cb = framebuffer.nr_cbufs;
}
