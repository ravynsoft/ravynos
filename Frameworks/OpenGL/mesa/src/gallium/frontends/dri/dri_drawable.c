/**************************************************************************
 *
 * Copyright 2009, VMware, Inc.
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
 * Author: Keith Whitwell <keithw@vmware.com>
 * Author: Jakob Bornecrantz <wallbraker@gmail.com>
 */

#include "dri_screen.h"
#include "dri_context.h"
#include "dri_drawable.h"

#include "pipe/p_screen.h"
#include "util/format/u_format.h"
#include "util/u_memory.h"
#include "util/u_inlines.h"

#include "state_tracker/st_context.h"

static uint32_t drifb_ID = 0;

static bool
dri_st_framebuffer_validate(struct st_context *st,
                            struct pipe_frontend_drawable *pdrawable,
                            const enum st_attachment_type *statts,
                            unsigned count,
                            struct pipe_resource **out,
                            struct pipe_resource **resolve)
{
   struct dri_context *ctx = (struct dri_context *)st->frontend_context;
   struct dri_drawable *drawable = (struct dri_drawable *)pdrawable;
   struct dri_screen *screen = drawable->screen;
   unsigned statt_mask, new_mask;
   bool new_stamp;
   int i;
   unsigned int lastStamp;
   struct pipe_resource **textures =
      drawable->stvis.samples > 1 ? drawable->msaa_textures
                                  : drawable->textures;

   statt_mask = 0x0;
   for (i = 0; i < count; i++)
      statt_mask |= (1 << statts[i]);

   /* record newly allocated textures */
   new_mask = (statt_mask & ~drawable->texture_mask);

   do {
      lastStamp = drawable->lastStamp;
      new_stamp = (drawable->texture_stamp != lastStamp);

      if (new_stamp || new_mask) {
         if (new_stamp && drawable->update_drawable_info)
            drawable->update_drawable_info(drawable);

         drawable->allocate_textures(ctx, drawable, statts, count);

         /* add existing textures */
         for (i = 0; i < ST_ATTACHMENT_COUNT; i++) {
            if (textures[i])
               statt_mask |= (1 << i);
         }

         drawable->texture_stamp = lastStamp;
         drawable->texture_mask = statt_mask;
      }
   } while (lastStamp != drawable->lastStamp);

   /* Flush the pending set_damage_region request. */
   struct pipe_screen *pscreen = screen->base.screen;

   if (new_mask & (1 << ST_ATTACHMENT_BACK_LEFT) &&
       pscreen->set_damage_region) {
      struct pipe_resource *resource = textures[ST_ATTACHMENT_BACK_LEFT];

      pscreen->set_damage_region(pscreen, resource,
                                 drawable->num_damage_rects,
                                 drawable->damage_rects);
   }

   if (!out)
      return true;

   /* Set the window-system buffers for the gallium frontend. */
   for (i = 0; i < count; i++)
      pipe_resource_reference(&out[i], textures[statts[i]]);
   if (resolve && drawable->stvis.samples > 1) {
      if (statt_mask & BITFIELD_BIT(ST_ATTACHMENT_FRONT_LEFT))
         pipe_resource_reference(resolve, drawable->textures[ST_ATTACHMENT_FRONT_LEFT]);
      else if (statt_mask & BITFIELD_BIT(ST_ATTACHMENT_BACK_LEFT))
         pipe_resource_reference(resolve, drawable->textures[ST_ATTACHMENT_BACK_LEFT]);
   }

   return true;
}

static bool
dri_st_framebuffer_flush_front(struct st_context *st,
                               struct pipe_frontend_drawable *pdrawable,
                               enum st_attachment_type statt)
{
   struct dri_context *ctx = (struct dri_context *)st->frontend_context;
   struct dri_drawable *drawable = (struct dri_drawable *)pdrawable;

   /* XXX remove this and just set the correct one on the framebuffer */
   return drawable->flush_frontbuffer(ctx, drawable, statt);
}

/**
 * The gallium frontend framebuffer interface flush_swapbuffers callback
 */
static bool
dri_st_framebuffer_flush_swapbuffers(struct st_context *st,
                                     struct pipe_frontend_drawable *pdrawable)
{
   struct dri_context *ctx = (struct dri_context *)st->frontend_context;
   struct dri_drawable *drawable = (struct dri_drawable *)pdrawable;

   if (drawable->flush_swapbuffers)
      drawable->flush_swapbuffers(ctx, drawable);

   return true;
}

/**
 * This is called when we need to set up GL rendering to a new X window.
 */
struct dri_drawable *
dri_create_drawable(struct dri_screen *screen, const struct gl_config *visual,
                    bool isPixmap, void *loaderPrivate)
{
   struct dri_drawable *drawable = NULL;

   if (isPixmap)
      goto fail;		       /* not implemented */

   drawable = CALLOC_STRUCT(dri_drawable);
   if (drawable == NULL)
      goto fail;

   drawable->loaderPrivate = loaderPrivate;
   drawable->refcount = 1;
   drawable->lastStamp = 0;
   drawable->w = 0;
   drawable->h = 0;

   dri_fill_st_visual(&drawable->stvis, screen, visual);

   /* setup the pipe_frontend_drawable */
   drawable->base.visual = &drawable->stvis;
   drawable->base.flush_front = dri_st_framebuffer_flush_front;
   drawable->base.validate = dri_st_framebuffer_validate;
   drawable->base.flush_swapbuffers = dri_st_framebuffer_flush_swapbuffers;

   drawable->screen = screen;

   p_atomic_set(&drawable->base.stamp, 1);
   drawable->base.ID = p_atomic_inc_return(&drifb_ID);
   drawable->base.fscreen = &screen->base;

   return drawable;
fail:
   FREE(drawable);
   return NULL;
}

static void
dri_destroy_drawable(struct dri_drawable *drawable)
{
   struct dri_screen *screen = drawable->screen;
   int i;

   for (i = 0; i < ST_ATTACHMENT_COUNT; i++)
      pipe_resource_reference(&drawable->textures[i], NULL);
   for (i = 0; i < ST_ATTACHMENT_COUNT; i++)
      pipe_resource_reference(&drawable->msaa_textures[i], NULL);

   screen->base.screen->fence_reference(screen->base.screen,
         &drawable->throttle_fence, NULL);

   /* Notify the st manager that this drawable is no longer valid */
   st_api_destroy_drawable(&drawable->base);

   FREE(drawable->damage_rects);
   FREE(drawable);
}

void
dri_put_drawable(struct dri_drawable *drawable)
{
   if (drawable) {
      int refcount = --drawable->refcount;
      assert(refcount >= 0);

      if (!refcount)
         dri_destroy_drawable(drawable);
   }
}

/**
 * Validate the texture at an attachment.  Allocate the texture if it does not
 * exist.  Used by the TFP extension.
 */
static void
dri_drawable_validate_att(struct dri_context *ctx,
                          struct dri_drawable *drawable,
                          enum st_attachment_type statt)
{
   enum st_attachment_type statts[ST_ATTACHMENT_COUNT];
   unsigned i, count = 0;

   /* check if buffer already exists */
   if (drawable->texture_mask & (1 << statt))
      return;

   /* make sure DRI2 does not destroy existing buffers */
   for (i = 0; i < ST_ATTACHMENT_COUNT; i++) {
      if (drawable->texture_mask & (1 << i)) {
         statts[count++] = i;
      }
   }
   statts[count++] = statt;

   drawable->texture_stamp = drawable->lastStamp - 1;

   drawable->base.validate(ctx->st, &drawable->base, statts, count, NULL, NULL);
}

/**
 * These are used for GLX_EXT_texture_from_pixmap
 */
static void
dri_set_tex_buffer2(__DRIcontext *pDRICtx, GLint target,
                    GLint format, __DRIdrawable *dPriv)
{
   struct dri_context *ctx = dri_context(pDRICtx);
   struct st_context *st = ctx->st;
   struct dri_drawable *drawable = dri_drawable(dPriv);
   struct pipe_resource *pt;

   _mesa_glthread_finish(st->ctx);

   dri_drawable_validate_att(ctx, drawable, ST_ATTACHMENT_FRONT_LEFT);

   /* Use the pipe resource associated with the X drawable */
   pt = drawable->textures[ST_ATTACHMENT_FRONT_LEFT];

   if (pt) {
      enum pipe_format internal_format = pt->format;

      if (format == __DRI_TEXTURE_FORMAT_RGB)  {
         /* only need to cover the formats recognized by dri_fill_st_visual */
         switch (internal_format) {
         case PIPE_FORMAT_R16G16B16A16_FLOAT:
            internal_format = PIPE_FORMAT_R16G16B16X16_FLOAT;
            break;
         case PIPE_FORMAT_B10G10R10A2_UNORM:
            internal_format = PIPE_FORMAT_B10G10R10X2_UNORM;
            break;
         case PIPE_FORMAT_R10G10B10A2_UNORM:
            internal_format = PIPE_FORMAT_R10G10B10X2_UNORM;
            break;
         case PIPE_FORMAT_BGRA8888_UNORM:
            internal_format = PIPE_FORMAT_BGRX8888_UNORM;
            break;
         case PIPE_FORMAT_ARGB8888_UNORM:
            internal_format = PIPE_FORMAT_XRGB8888_UNORM;
            break;
         default:
            break;
         }
      }

      drawable->update_tex_buffer(drawable, ctx, pt);

      st_context_teximage(ctx->st, target, 0, internal_format, pt, false);
   }
}

static void
dri_set_tex_buffer(__DRIcontext *pDRICtx, GLint target,
                   __DRIdrawable *dPriv)
{
   dri_set_tex_buffer2(pDRICtx, target, __DRI_TEXTURE_FORMAT_RGBA, dPriv);
}

const __DRItexBufferExtension driTexBufferExtension = {
   .base = { __DRI_TEX_BUFFER, 2 },

   .setTexBuffer       = dri_set_tex_buffer,
   .setTexBuffer2      = dri_set_tex_buffer2,
   .releaseTexBuffer   = NULL,
};

/**
 * Get the format and binding of an attachment.
 */
void
dri_drawable_get_format(struct dri_drawable *drawable,
                        enum st_attachment_type statt,
                        enum pipe_format *format,
                        unsigned *bind)
{
   switch (statt) {
   case ST_ATTACHMENT_FRONT_LEFT:
   case ST_ATTACHMENT_BACK_LEFT:
   case ST_ATTACHMENT_FRONT_RIGHT:
   case ST_ATTACHMENT_BACK_RIGHT:
      /* Other pieces of the driver stack get confused and behave incorrectly
       * when they get an sRGB drawable. st/mesa receives "drawable->stvis"
       * though other means and handles it correctly, so we don't really need
       * to use an sRGB format here.
       */
      *format = util_format_linear(drawable->stvis.color_format);
      *bind = PIPE_BIND_RENDER_TARGET | PIPE_BIND_SAMPLER_VIEW;
      break;
   case ST_ATTACHMENT_DEPTH_STENCIL:
      *format = drawable->stvis.depth_stencil_format;
      *bind = PIPE_BIND_DEPTH_STENCIL; /* XXX sampler? */
      break;
   default:
      *format = PIPE_FORMAT_NONE;
      *bind = 0;
      break;
   }
}

void
dri_pipe_blit(struct pipe_context *pipe,
              struct pipe_resource *dst,
              struct pipe_resource *src)
{
   struct pipe_blit_info blit;

   if (!dst || !src)
      return;

   /* From the GL spec, version 4.2, section 4.1.11 (Additional Multisample
    *  Fragment Operations):
    *
    *      If a framebuffer object is not bound, after all operations have
    *      been completed on the multisample buffer, the sample values for
    *      each color in the multisample buffer are combined to produce a
    *      single color value, and that value is written into the
    *      corresponding color buffers selected by DrawBuffer or
    *      DrawBuffers. An implementation may defer the writing of the color
    *      buffers until a later time, but the state of the framebuffer must
    *      behave as if the color buffers were updated as each fragment was
    *      processed. The method of combination is not specified. If the
    *      framebuffer contains sRGB values, then it is recommended that the
    *      an average of sample values is computed in a linearized space, as
    *      for blending (see section 4.1.7).
    *
    * In other words, to do a resolve operation in a linear space, we have
    * to set sRGB formats if the original resources were sRGB, so don't use
    * util_format_linear.
    */

   memset(&blit, 0, sizeof(blit));
   blit.dst.resource = dst;
   blit.dst.box.width = dst->width0;
   blit.dst.box.height = dst->height0;
   blit.dst.box.depth = 1;
   blit.dst.format = dst->format;
   blit.src.resource = src;
   blit.src.box.width = src->width0;
   blit.src.box.height = src->height0;
   blit.src.box.depth = 1;
   blit.src.format = src->format;
   blit.mask = PIPE_MASK_RGBA;
   blit.filter = PIPE_TEX_FILTER_NEAREST;

   pipe->blit(pipe, &blit);
}

static void
dri_postprocessing(struct dri_context *ctx,
                   struct dri_drawable *drawable,
                   enum st_attachment_type att)
{
   struct pipe_resource *src = drawable->textures[att];
   struct pipe_resource *zsbuf = drawable->textures[ST_ATTACHMENT_DEPTH_STENCIL];

   if (ctx->pp && src)
      pp_run(ctx->pp, src, src, zsbuf);
}

struct notify_before_flush_cb_args {
   struct dri_context *ctx;
   struct dri_drawable *drawable;
   unsigned flags;
   enum __DRI2throttleReason reason;
   bool swap_msaa_buffers;
};

static void
notify_before_flush_cb(void* _args)
{
   struct notify_before_flush_cb_args *args = (struct notify_before_flush_cb_args *) _args;
   struct st_context *st = args->ctx->st;
   struct pipe_context *pipe = st->pipe;

   /* Wait for glthread to finish because we can't use pipe_context from
    * multiple threads.
    */
   _mesa_glthread_finish(st->ctx);

   if (args->drawable->stvis.samples > 1 &&
       (args->reason == __DRI2_THROTTLE_SWAPBUFFER ||
        args->reason == __DRI2_NOTHROTTLE_SWAPBUFFER ||
        args->reason == __DRI2_THROTTLE_COPYSUBBUFFER)) {
      /* Resolve the MSAA back buffer. */
      dri_pipe_blit(st->pipe,
                    args->drawable->textures[ST_ATTACHMENT_BACK_LEFT],
                    args->drawable->msaa_textures[ST_ATTACHMENT_BACK_LEFT]);

      if ((args->reason == __DRI2_THROTTLE_SWAPBUFFER ||
           args->reason == __DRI2_NOTHROTTLE_SWAPBUFFER) &&
          args->drawable->msaa_textures[ST_ATTACHMENT_FRONT_LEFT] &&
          args->drawable->msaa_textures[ST_ATTACHMENT_BACK_LEFT]) {
         args->swap_msaa_buffers = true;
      }

      /* FRONT_LEFT is resolved in drawable->flush_frontbuffer. */
   }

   dri_postprocessing(args->ctx, args->drawable, ST_ATTACHMENT_BACK_LEFT);

   if (pipe->invalidate_resource &&
       (args->flags & __DRI2_FLUSH_INVALIDATE_ANCILLARY)) {
      if (args->drawable->textures[ST_ATTACHMENT_DEPTH_STENCIL])
         pipe->invalidate_resource(pipe, args->drawable->textures[ST_ATTACHMENT_DEPTH_STENCIL]);
      if (args->drawable->msaa_textures[ST_ATTACHMENT_DEPTH_STENCIL])
         pipe->invalidate_resource(pipe, args->drawable->msaa_textures[ST_ATTACHMENT_DEPTH_STENCIL]);
   }

   if (args->ctx->hud) {
      hud_run(args->ctx->hud, args->ctx->st->cso_context,
              args->drawable->textures[ST_ATTACHMENT_BACK_LEFT]);
   }

   pipe->flush_resource(pipe, args->drawable->textures[ST_ATTACHMENT_BACK_LEFT]);
}

/**
 * DRI2 flush extension, the flush_with_flags function.
 *
 * \param context           the context
 * \param drawable          the drawable to flush
 * \param flags             a combination of _DRI2_FLUSH_xxx flags
 * \param throttle_reason   the reason for throttling, 0 = no throttling
 */
void
dri_flush(__DRIcontext *cPriv,
          __DRIdrawable *dPriv,
          unsigned flags,
          enum __DRI2throttleReason reason)
{
   struct dri_context *ctx = dri_context(cPriv);
   struct dri_drawable *drawable = dri_drawable(dPriv);
   struct st_context *st;
   unsigned flush_flags;
   struct notify_before_flush_cb_args args = { 0 };

   if (!ctx) {
      assert(0);
      return;
   }

   st = ctx->st;
   _mesa_glthread_finish(st->ctx);

   if (drawable) {
      /* prevent recursion */
      if (drawable->flushing)
         return;

      drawable->flushing = true;
   }
   else {
      flags &= ~__DRI2_FLUSH_DRAWABLE;
   }

   if ((flags & __DRI2_FLUSH_DRAWABLE) &&
       drawable->textures[ST_ATTACHMENT_BACK_LEFT]) {
      /* We can't do operations on the back buffer here, because there
       * may be some pending operations that will get flushed by the
       * call to st->flush (eg: FLUSH_VERTICES).
       * Instead we register a callback to be notified when all operations
       * have been submitted but before the call to st_flush.
       */
      args.ctx = ctx;
      args.drawable = drawable;
      args.flags = flags;
      args.reason = reason;
   }

   flush_flags = 0;
   if (flags & __DRI2_FLUSH_CONTEXT)
      flush_flags |= ST_FLUSH_FRONT;
   if (reason == __DRI2_THROTTLE_SWAPBUFFER ||
       reason == __DRI2_NOTHROTTLE_SWAPBUFFER)
      flush_flags |= ST_FLUSH_END_OF_FRAME;

   /* Flush the context and throttle if needed. */
   if (ctx->screen->throttle &&
       drawable &&
       (reason == __DRI2_THROTTLE_SWAPBUFFER ||
        reason == __DRI2_THROTTLE_FLUSHFRONT)) {

      struct pipe_screen *screen = drawable->screen->base.screen;
      struct pipe_fence_handle *new_fence = NULL;

      st_context_flush(st, flush_flags, &new_fence, args.ctx ? notify_before_flush_cb : NULL, &args);

      /* throttle on the previous fence */
      if (drawable->throttle_fence) {
         screen->fence_finish(screen, NULL, drawable->throttle_fence, OS_TIMEOUT_INFINITE);
         screen->fence_reference(screen, &drawable->throttle_fence, NULL);
      }
      drawable->throttle_fence = new_fence;
   }
   else if (flags & (__DRI2_FLUSH_DRAWABLE | __DRI2_FLUSH_CONTEXT)) {
      st_context_flush(st, flush_flags, NULL, args.ctx ? notify_before_flush_cb : NULL, &args);
   }

   if (drawable) {
      drawable->flushing = false;
   }

   /* Swap the MSAA front and back buffers, so that reading
    * from the front buffer after SwapBuffers returns what was
    * in the back buffer.
    */
   if (args.swap_msaa_buffers) {
      struct pipe_resource *tmp =
         drawable->msaa_textures[ST_ATTACHMENT_FRONT_LEFT];

      drawable->msaa_textures[ST_ATTACHMENT_FRONT_LEFT] =
         drawable->msaa_textures[ST_ATTACHMENT_BACK_LEFT];
      drawable->msaa_textures[ST_ATTACHMENT_BACK_LEFT] = tmp;

      /* Now that we have swapped the buffers, this tells the gallium
       * frontend to revalidate the framebuffer.
       */
      p_atomic_inc(&drawable->base.stamp);
   }

   st_context_invalidate_state(st, ST_INVALIDATE_FB_STATE);
}

/**
 * DRI2 flush extension.
 */
void
dri_flush_drawable(__DRIdrawable *dPriv)
{
   struct dri_context *ctx = dri_get_current();

   if (ctx)
      dri_flush(opaque_dri_context(ctx), dPriv, __DRI2_FLUSH_DRAWABLE, -1);
}

/**
 * dri_throttle - A DRI2ThrottleExtension throttling function.
 */
static void
dri_throttle(__DRIcontext *cPriv, __DRIdrawable *dPriv,
             enum __DRI2throttleReason reason)
{
   dri_flush(cPriv, dPriv, 0, reason);
}


const __DRI2throttleExtension dri2ThrottleExtension = {
    .base = { __DRI2_THROTTLE, 1 },

    .throttle          = dri_throttle,
};


/* vim: set sw=3 ts=8 sts=3 expandtab: */
