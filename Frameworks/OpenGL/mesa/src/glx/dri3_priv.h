/*
 * Copyright © 2013 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

/* This file was derived from dri2_priv.h which carries the following
 * copyright:
 *
 * Copyright © 2008 Red Hat, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Soft-
 * ware"), to deal in the Software without restriction, including without
 * limitation the rights to use, copy, modify, merge, publish, distribute,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, provided that the above copyright
 * notice(s) and this permission notice appear in all copies of the Soft-
 * ware and that both the above copyright notice(s) and this permission
 * notice appear in supporting documentation.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABIL-
 * ITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF THIRD PARTY
 * RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS INCLUDED IN
 * THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT OR CONSE-
 * QUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFOR-
 * MANCE OF THIS SOFTWARE.
 *
 * Except as contained in this notice, the name of a copyright holder shall
 * not be used in advertising or otherwise to promote the sale, use or
 * other dealings in this Software without prior written authorization of
 * the copyright holder.
 *
 * Authors:
 *   Kristian Høgsberg (krh@redhat.com)
 */

#include <xcb/xcb.h>
#include <xcb/dri3.h>
#include <xcb/present.h>
#include <xcb/sync.h>

#include "loader_dri3_helper.h"
#include "GL/internal/mesa_interface.h"

struct dri3_display
{
   __GLXDRIdisplay base;

   const __DRIextension **loader_extensions;
   int has_multibuffer;
};

struct dri3_screen {
   struct glx_screen base;

   __GLXDRIscreen vtable;

   /* DRI screen is created for display GPU in case of prime gpu offloading.
    * This screen is used to allocate linear_buffer from
    * display GPU space in dri3_alloc_render_buffer() function.
    * In case of not gpu offloading driScreenDisplayGPU will be assigned with
    * driScreenRenderGPU.
    * In case of prime gpu offloading if display and render driver names are different
    * (potentially not compatible), driScreenDisplayGPU will be NULL but
    * fd_display_gpu will still hold fd for display driver.
    */
   __DRIscreen *driScreenDisplayGPU;
   __DRIscreen *driScreenRenderGPU;

   const __DRIimageExtension *image;
   const __DRIimageDriverExtension *image_driver;
   const __DRIcoreExtension *core;
   const __DRImesaCoreExtension *mesa;
   const __DRI2flushExtension *f;
   const __DRI2configQueryExtension *config;
   const __DRItexBufferExtension *texBuffer;
   const __DRI2rendererQueryExtension *rendererQuery;
   const __DRI2interopExtension *interop;
   const __DRIconfig **driver_configs;

   void *driver;
   /* fd of the GPU used for rendering. */
   int fd_render_gpu;
   /* fd of the GPU used for display. If the same GPU is used for display
    * and rendering, then fd_render_gpu == fd_display_gpu (no need to use
    * os_same_file_description).
    */
   int fd_display_gpu;
   bool prefer_back_buffer_reuse;

   struct loader_dri3_extensions loader_dri3_ext;
};

struct dri3_drawable {
   __GLXDRIdrawable base;
   struct loader_dri3_drawable loader_drawable;

   /* LIBGL_SHOW_FPS support */
   uint64_t previous_ust;
   unsigned frames;
};

bool
dri3_check_multibuffer(Display * dpy, bool *err);

_X_HIDDEN int
dri3_query_renderer_integer(struct glx_screen *base, int attribute,
                            unsigned int *value);

_X_HIDDEN int
dri3_query_renderer_string(struct glx_screen *base, int attribute,
                           const char **value);

_X_HIDDEN int
dri3_interop_query_device_info(struct glx_context *ctx,
                               struct mesa_glinterop_device_info *out);

_X_HIDDEN int
dri3_interop_export_object(struct glx_context *ctx,
                           struct mesa_glinterop_export_in *in,
                           struct mesa_glinterop_export_out *out);

_X_HIDDEN int
dri3_interop_flush_objects(struct glx_context *ctx,
                           unsigned count, struct mesa_glinterop_export_in *objects,
                           struct mesa_glinterop_flush_out *out);
