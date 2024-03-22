/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 2010 LunarG Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Chia-I Wu <olv@lunarg.com>
 */

#include "xm_api.h"
#include "xm_st.h"

#include "util/u_inlines.h"
#include "util/u_atomic.h"
#include "util/u_memory.h"

#include "state_tracker/st_context.h"

struct xmesa_st_framebuffer {
   struct pipe_frontend_drawable base;

   XMesaDisplay display;
   XMesaBuffer buffer;
   struct pipe_screen *screen;

   struct st_visual stvis;
   enum pipe_texture_target target;

   unsigned texture_width, texture_height, texture_mask;
   struct pipe_resource *textures[ST_ATTACHMENT_COUNT];

   struct pipe_resource *display_resource;
};


static inline struct xmesa_st_framebuffer *
xmesa_st_framebuffer(struct pipe_frontend_drawable *drawable)
{
   return (struct xmesa_st_framebuffer *)drawable;
}


/**
 * Display (present) an attachment to the xlib_drawable of the framebuffer.
 */
static bool
xmesa_st_framebuffer_display(struct pipe_frontend_drawable *drawable,
                             struct st_context *st,
                             enum st_attachment_type statt,
                             struct pipe_box *box)
{
   struct xmesa_st_framebuffer *xstfb = xmesa_st_framebuffer(drawable);
   struct pipe_resource *ptex = xstfb->textures[statt];
   struct pipe_resource *pres;
   struct pipe_context *pctx = st ? st->pipe : NULL;

   if (!ptex)
      return true;

   pres = xstfb->display_resource;
   /* (re)allocate the surface for the texture to be displayed */
   if (!pres || pres != ptex) {
      pipe_resource_reference(&xstfb->display_resource, ptex);
      pres = xstfb->display_resource;
   }

   xstfb->screen->flush_frontbuffer(xstfb->screen, pctx, pres, 0, 0, &xstfb->buffer->ws, box);
   return true;
}


/**
 * Copy the contents between the attachments.
 */
static void
xmesa_st_framebuffer_copy_textures(struct pipe_frontend_drawable *drawable,
                                   enum st_attachment_type src_statt,
                                   enum st_attachment_type dst_statt,
                                   unsigned x, unsigned y,
                                   unsigned width, unsigned height)
{
   struct xmesa_st_framebuffer *xstfb = xmesa_st_framebuffer(drawable);
   struct pipe_resource *src_ptex = xstfb->textures[src_statt];
   struct pipe_resource *dst_ptex = xstfb->textures[dst_statt];
   struct pipe_box src_box;
   struct pipe_context *pipe;

   if (!src_ptex || !dst_ptex)
      return;

   pipe = xmesa_get_context(drawable);

   u_box_2d(x, y, width, height, &src_box);

   if (src_ptex && dst_ptex)
      pipe->resource_copy_region(pipe, dst_ptex, 0, x, y, 0,
                                 src_ptex, 0, &src_box);
}


/**
 * Remove outdated textures and create the requested ones.
 * This is a helper used during framebuffer validation.
 */
bool
xmesa_st_framebuffer_validate_textures(struct pipe_frontend_drawable *drawable,
                                       unsigned width, unsigned height,
                                       unsigned mask)
{
   struct xmesa_st_framebuffer *xstfb = xmesa_st_framebuffer(drawable);
   struct pipe_resource templ;
   enum st_attachment_type i;

   /* remove outdated textures */
   if (xstfb->texture_width != width || xstfb->texture_height != height) {
      for (i = 0; i < ST_ATTACHMENT_COUNT; i++)
         pipe_resource_reference(&xstfb->textures[i], NULL);
   }

   memset(&templ, 0, sizeof(templ));
   templ.target = xstfb->target;
   templ.width0 = width;
   templ.height0 = height;
   templ.depth0 = 1;
   templ.array_size = 1;
   templ.last_level = 0;
   templ.nr_samples = xstfb->stvis.samples;
   templ.nr_storage_samples = xstfb->stvis.samples;

   for (i = 0; i < ST_ATTACHMENT_COUNT; i++) {
      enum pipe_format format;
      unsigned bind;

      /* the texture already exists or not requested */
      if (xstfb->textures[i] || !(mask & (1 << i))) {
         /* remember the texture */
         if (xstfb->textures[i])
            mask |= (1 << i);
         continue;
      }

      switch (i) {
      case ST_ATTACHMENT_FRONT_LEFT:
      case ST_ATTACHMENT_BACK_LEFT:
      case ST_ATTACHMENT_FRONT_RIGHT:
      case ST_ATTACHMENT_BACK_RIGHT:
         format = xstfb->stvis.color_format;
         bind = PIPE_BIND_DISPLAY_TARGET |
                     PIPE_BIND_RENDER_TARGET;
         break;
      case ST_ATTACHMENT_DEPTH_STENCIL:
         format = xstfb->stvis.depth_stencil_format;
         bind = PIPE_BIND_DEPTH_STENCIL;
         break;
      default:
         format = PIPE_FORMAT_NONE;
         break;
      }

      if (format != PIPE_FORMAT_NONE) {
         templ.format = format;
         templ.bind = bind;

         xstfb->textures[i] =
            xstfb->screen->resource_create(xstfb->screen, &templ);
         if (!xstfb->textures[i])
            return false;
      }
   }

   xstfb->texture_width = width;
   xstfb->texture_height = height;
   xstfb->texture_mask = mask;

   return true;
}


/**
 * Check that a framebuffer's attachments match the window's size.
 *
 * Called via pipe_frontend_drawable::validate()
 *
 * \param statts  array of framebuffer attachments
 * \param count  number of framebuffer attachments in statts[]
 * \param out  returns resources for each of the attachments
 */
static bool
xmesa_st_framebuffer_validate(struct st_context *st,
                              struct pipe_frontend_drawable *drawable,
                              const enum st_attachment_type *statts,
                              unsigned count,
                              struct pipe_resource **out,
                              struct pipe_resource **resolve)
{
   struct xmesa_st_framebuffer *xstfb = xmesa_st_framebuffer(drawable);
   unsigned statt_mask, new_mask, i;
   bool resized;
   bool ret;

   /* build mask of ST_ATTACHMENT bits */
   statt_mask = 0x0;
   for (i = 0; i < count; i++)
      statt_mask |= 1 << statts[i];

   /* record newly allocated textures */
   new_mask = statt_mask & ~xstfb->texture_mask;

   /* If xmesa_strict_invalidate is not set, we will not yet have
    * called XGetGeometry().  Do so here:
    */
   if (!xmesa_strict_invalidate())
      xmesa_check_buffer_size(xstfb->buffer);

   resized = (xstfb->buffer->width != xstfb->texture_width ||
              xstfb->buffer->height != xstfb->texture_height);

   /* revalidate textures */
   if (resized || new_mask) {
      ret = xmesa_st_framebuffer_validate_textures(drawable,
                  xstfb->buffer->width, xstfb->buffer->height, statt_mask);
      if (!ret)
         return ret;

      if (!resized) {
         enum st_attachment_type back, front;

         back = ST_ATTACHMENT_BACK_LEFT;
         front = ST_ATTACHMENT_FRONT_LEFT;
         /* copy the contents if front is newly allocated and back is not */
         if ((statt_mask & (1 << back)) &&
             (new_mask & (1 << front)) &&
             !(new_mask & (1 << back))) {
            xmesa_st_framebuffer_copy_textures(drawable, back, front,
                  0, 0, xstfb->texture_width, xstfb->texture_height);
         }
      }
   }

   for (i = 0; i < count; i++)
      pipe_resource_reference(&out[i], xstfb->textures[statts[i]]);
   if (resolve && drawable->visual->samples > 1) {
      if (statt_mask & BITFIELD_BIT(ST_ATTACHMENT_FRONT_LEFT))
         pipe_resource_reference(resolve, xstfb->display_resource);
      else if (statt_mask & BITFIELD_BIT(ST_ATTACHMENT_BACK_LEFT))
         pipe_resource_reference(resolve, xstfb->display_resource);
   }

   return true;
}


/**
 * Called via pipe_frontend_drawable::flush_front()
 */
static bool
xmesa_st_framebuffer_flush_front(struct st_context *st,
                                 struct pipe_frontend_drawable *drawable,
                                 enum st_attachment_type statt)
{
   struct xmesa_st_framebuffer *xstfb = xmesa_st_framebuffer(drawable);
   bool ret;

   if (statt != ST_ATTACHMENT_FRONT_LEFT)
      return false;

   ret = xmesa_st_framebuffer_display(drawable, st, statt, NULL);

   if (ret && xmesa_strict_invalidate())
      xmesa_check_buffer_size(xstfb->buffer);

   return ret;
}

static uint32_t xmesa_drawable_ID = 0;

struct pipe_frontend_drawable *
xmesa_create_st_framebuffer(XMesaDisplay xmdpy, XMesaBuffer b)
{
   struct xmesa_st_framebuffer *xstfb;

   assert(xmdpy->display == b->xm_visual->display);

   xstfb = CALLOC_STRUCT(xmesa_st_framebuffer);
   if (!xstfb) {
      free(xstfb);
      return NULL;
   }

   xstfb->display = xmdpy;
   xstfb->buffer = b;
   xstfb->screen = xmdpy->screen;
   xstfb->stvis = b->xm_visual->stvis;
   if (xstfb->screen->get_param(xstfb->screen, PIPE_CAP_NPOT_TEXTURES))
      xstfb->target = PIPE_TEXTURE_2D;
   else
      xstfb->target = PIPE_TEXTURE_RECT;

   xstfb->base.visual = &xstfb->stvis;
   xstfb->base.flush_front = xmesa_st_framebuffer_flush_front;
   xstfb->base.validate = xmesa_st_framebuffer_validate;
   xstfb->base.ID = p_atomic_inc_return(&xmesa_drawable_ID);
   xstfb->base.fscreen = xmdpy->fscreen;
   p_atomic_set(&xstfb->base.stamp, 1);

   return &xstfb->base;
}


void
xmesa_destroy_st_framebuffer(struct pipe_frontend_drawable *drawable)
{
   struct xmesa_st_framebuffer *xstfb = xmesa_st_framebuffer(drawable);
   int i;

   pipe_resource_reference(&xstfb->display_resource, NULL);

   for (i = 0; i < ST_ATTACHMENT_COUNT; i++)
      pipe_resource_reference(&xstfb->textures[i], NULL);

   free(xstfb);
}


/**
 * Return the pipe_surface which corresponds to the given
 * framebuffer attachment.
 */
struct pipe_resource *
xmesa_get_framebuffer_resource(struct pipe_frontend_drawable *drawable,
                               enum st_attachment_type att)
{
   struct xmesa_st_framebuffer *xstfb = xmesa_st_framebuffer(drawable);
   return xstfb->textures[att];
}


void
xmesa_swap_st_framebuffer(struct pipe_frontend_drawable *drawable)
{
   struct xmesa_st_framebuffer *xstfb = xmesa_st_framebuffer(drawable);
   bool ret;

   ret = xmesa_st_framebuffer_display(drawable, NULL, ST_ATTACHMENT_BACK_LEFT, NULL);
   if (ret) {
      struct pipe_resource **front, **back, *tmp;

      front = &xstfb->textures[ST_ATTACHMENT_FRONT_LEFT];
      back = &xstfb->textures[ST_ATTACHMENT_BACK_LEFT];
      /* swap textures only if the front texture has been allocated */
      if (*front) {
         tmp = *front;
         *front = *back;
         *back = tmp;

         /* the current context should validate the buffer after swapping */
         if (!xmesa_strict_invalidate())
            xmesa_notify_invalid_buffer(xstfb->buffer);
      }

      if (xmesa_strict_invalidate())
         xmesa_check_buffer_size(xstfb->buffer);
   }
}


void
xmesa_copy_st_framebuffer(struct pipe_frontend_drawable *drawable,
                          enum st_attachment_type src,
                          enum st_attachment_type dst,
                          int x, int y, int w, int h)
{
   xmesa_st_framebuffer_copy_textures(drawable, src, dst, x, y, w, h);
   if (dst == ST_ATTACHMENT_FRONT_LEFT) {
      struct pipe_box box = {};

      box.x = x;
      box.y = y;
      box.width = w;
      box.height = h;
      xmesa_st_framebuffer_display(drawable, NULL, src, &box);
   }
}


struct pipe_resource*
xmesa_get_attachment(struct pipe_frontend_drawable *drawable,
                     enum st_attachment_type st_attachment)
{
   struct xmesa_st_framebuffer *xstfb = xmesa_st_framebuffer(drawable);
   struct pipe_resource *res;

   res = xstfb->textures[st_attachment];
   return res;
}


struct pipe_context*
xmesa_get_context(struct pipe_frontend_drawable *drawable)
{
   struct pipe_context *pipe;
   struct xmesa_st_framebuffer *xstfb = xmesa_st_framebuffer(drawable);

   pipe = xstfb->display->pipe;
   if (!pipe) {
      pipe = xstfb->screen->context_create(xstfb->screen, NULL, 0);
      if (!pipe)
         return NULL;
      xstfb->display->pipe = pipe;
   }
   return pipe;
}
