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

#ifndef DRI_DRAWABLE_H
#define DRI_DRAWABLE_H

#include "util/compiler.h"
#include "util/format/u_formats.h"
#include "frontend/api.h"
#include "dri_util.h"

struct dri_context;
struct dri_screen;

struct dri_drawable
{
   struct pipe_frontend_drawable base;
   struct st_visual stvis;

   struct dri_screen *screen;

   __DRIbuffer old[__DRI_BUFFER_COUNT];
   unsigned old_num;
   unsigned old_w;
   unsigned old_h;

   struct pipe_box *damage_rects;
   unsigned int num_damage_rects;

   struct pipe_resource *textures[ST_ATTACHMENT_COUNT];
   struct pipe_resource *msaa_textures[ST_ATTACHMENT_COUNT];
   unsigned int texture_mask, texture_stamp;
   int swap_interval;

   struct pipe_fence_handle *throttle_fence;
   bool flushing; /* prevents recursion in dri_flush */

   /**
    * Private data from the loader.  We just hold on to it and pass
    * it back when calling into loader provided functions.
    */
   void *loaderPrivate;

   /**
    * Reference count for number of context's currently bound to this
    * drawable.
    *
    * Once it reaches zero, the drawable can be destroyed.
    *
    * \note This behavior will change with GLX 1.3.
    */
   int refcount;

   /**
    * Increased when the loader calls invalidate.
    *
    * If this changes, the drawable information (below) should be retrieved
    * from the loader.
    */
   unsigned int lastStamp;
   int w, h;

   /* kopper */
   struct kopper_loader_info info;
   __DRIimage   *image; //texture_from_pixmap
   bool is_window;
   bool has_modifiers;

   /* hooks filled in by dri2 & drisw */
   void (*allocate_textures)(struct dri_context *ctx,
                             struct dri_drawable *drawable,
                             const enum st_attachment_type *statts,
                             unsigned count);

   void (*update_drawable_info)(struct dri_drawable *drawable);

   bool (*flush_frontbuffer)(struct dri_context *ctx,
                             struct dri_drawable *drawable,
                             enum st_attachment_type statt);

   void (*update_tex_buffer)(struct dri_drawable *drawable,
                             struct dri_context *ctx,
                             struct pipe_resource *res);
   void (*flush_swapbuffers)(struct dri_context *ctx,
                             struct dri_drawable *drawable);

   void (*swap_buffers)(struct dri_drawable *drawable);
};

/* Typecast the opaque pointer to our own type. */
static inline struct dri_drawable *
dri_drawable(__DRIdrawable *drawable)
{
   return (struct dri_drawable *)drawable;
}

/* Typecast our own type to the opaque pointer. */
static inline __DRIdrawable *
opaque_dri_drawable(struct dri_drawable *drawable)
{
   return (__DRIdrawable *)drawable;
}

static inline void
dri_get_drawable(struct dri_drawable *drawable)
{
    drawable->refcount++;
}

/***********************************************************************
 * dri_drawable.c
 */
struct dri_drawable *
dri_create_drawable(struct dri_screen *screen, const struct gl_config *visual,
                    bool isPixmap, void *loaderPrivate);

void
dri_put_drawable(struct dri_drawable *drawable);

void
dri_drawable_get_format(struct dri_drawable *drawable,
                        enum st_attachment_type statt,
                        enum pipe_format *format,
                        unsigned *bind);

void
dri_pipe_blit(struct pipe_context *pipe,
              struct pipe_resource *dst,
              struct pipe_resource *src);

void
dri_flush(__DRIcontext *cPriv,
          __DRIdrawable *dPriv,
          unsigned flags,
          enum __DRI2throttleReason reason);

void
dri_flush_drawable(__DRIdrawable *dPriv);

extern const __DRItexBufferExtension driTexBufferExtension;
extern const __DRI2throttleExtension dri2ThrottleExtension;
#endif

/* vim: set sw=3 ts=8 sts=3 expandtab: */
