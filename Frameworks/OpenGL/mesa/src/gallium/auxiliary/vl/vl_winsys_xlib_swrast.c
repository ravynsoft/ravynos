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

#include "pipe-loader/pipe_loader.h"
#include "pipe/p_screen.h"

#include "util/u_memory.h"
#include "vl/vl_winsys.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "gallium/include/frontend/xlibsw_api.h"
#include "gallium/winsys/sw/xlib/xlib_sw_winsys.h"
#include "loader.h"
#include "target-helpers/sw_helper_public.h"
#include "vl/vl_compositor.h"

struct vl_xlib_screen
{
   struct vl_screen base;
   struct pipe_context *pipe;
   Display *display;
   int screen;
   struct u_rect dirty_area;
   XVisualInfo xlib_drawable_visual_info;
   struct xlib_drawable xlib_drawable_handle;
   struct pipe_resource *drawable_texture;
};

static void
vl_screen_destroy(struct vl_screen *vscreen)
{
   if (vscreen == NULL)
      return;

   if (vscreen->pscreen)
      vscreen->pscreen->destroy(vscreen->pscreen);

   if (vscreen->dev)
      pipe_loader_release(&vscreen->dev, 1);

   FREE(vscreen);
}

struct pipe_resource *
vl_swrast_texture_from_drawable(struct vl_screen *vscreen, void *drawable);

struct u_rect *
vl_swrast_get_dirty_area(struct vl_screen *vscreen);

void *
vl_swrast_get_private(struct vl_screen *vscreen);

static void
vl_xlib_screen_destroy(struct vl_screen *vscreen)
{
   if (vscreen == NULL)
      return;

   struct vl_xlib_screen *xlib_screen = (struct vl_xlib_screen *) vscreen;

   if (xlib_screen->drawable_texture)
      pipe_resource_reference(&xlib_screen->drawable_texture, NULL);

   if (xlib_screen->pipe)
      xlib_screen->pipe->destroy(xlib_screen->pipe);

   vl_screen_destroy(&xlib_screen->base);
}

struct vl_screen *
vl_xlib_swrast_screen_create(Display *display, int screen)
{
   struct vl_xlib_screen *vscreen = CALLOC_STRUCT(vl_xlib_screen);
   if (!vscreen)
      goto handle_err_xlib_swrast_create;

   struct sw_winsys *xlib_winsys = xlib_create_sw_winsys(display);
   if (xlib_winsys == NULL)
      goto handle_err_xlib_swrast_create;

   vscreen->base.pscreen = sw_screen_create(xlib_winsys);
   if (!vscreen->base.pscreen)
      goto handle_err_xlib_swrast_create;

   vscreen->base.get_private = vl_swrast_get_private;
   vscreen->base.texture_from_drawable = vl_swrast_texture_from_drawable;
   vscreen->base.get_dirty_area = vl_swrast_get_dirty_area;
   vscreen->base.destroy = vl_xlib_screen_destroy;
   vscreen->pipe = vscreen->base.pscreen->context_create(vscreen->base.pscreen, NULL, 0);

   vl_compositor_reset_dirty_area(&vscreen->dirty_area);
   vscreen->display = display;
   vscreen->screen = screen;

   return &vscreen->base;

handle_err_xlib_swrast_create:
   if (vscreen)
      vl_xlib_screen_destroy(&vscreen->base);

   return NULL;
}

void
vl_swrast_fill_xlib_drawable_desc(struct vl_screen *vscreen,
                                  Window x11_window,
                                  struct xlib_drawable *drawable_desc);

void
vl_swrast_fill_xlib_drawable_desc(struct vl_screen *vscreen,
                                  Window x11_window,
                                  struct xlib_drawable *drawable_desc)
{
   struct vl_xlib_screen *scrn = (struct vl_xlib_screen *) vscreen;
   XWindowAttributes x11_window_attrs = {};
   XGetWindowAttributes(scrn->display, x11_window, &x11_window_attrs);
   XMatchVisualInfo(scrn->display, scrn->screen, x11_window_attrs.depth, TrueColor, &scrn->xlib_drawable_visual_info);
   scrn->xlib_drawable_handle.depth = x11_window_attrs.depth;
   scrn->xlib_drawable_handle.drawable = x11_window;
   scrn->xlib_drawable_handle.visual = scrn->xlib_drawable_visual_info.visual;
}

struct pipe_resource *
vl_swrast_texture_from_drawable(struct vl_screen *vscreen, void *drawable)
{
   struct vl_xlib_screen *scrn = (struct vl_xlib_screen *) vscreen;
   Window x11_window = (Window) drawable;
   vl_swrast_fill_xlib_drawable_desc(vscreen, x11_window, &scrn->xlib_drawable_handle);

   XWindowAttributes x11_window_attrs = {};
   XGetWindowAttributes(scrn->display, x11_window, &x11_window_attrs);
   enum pipe_format x11_window_format = vl_dri2_format_for_depth(&scrn->base, x11_window_attrs.depth);

   bool needs_new_back_buffer_allocation = true;
   if (scrn->drawable_texture) {
      needs_new_back_buffer_allocation =
         (scrn->drawable_texture->width0 != x11_window_attrs.width || scrn->drawable_texture->height0 != x11_window_attrs.height ||
          scrn->drawable_texture->format != x11_window_format);
   }

   if (needs_new_back_buffer_allocation) {
      if (scrn->drawable_texture)
         pipe_resource_reference(&scrn->drawable_texture, NULL);

      struct pipe_resource templat;
      memset(&templat, 0, sizeof(templat));
      templat.target = PIPE_TEXTURE_2D;
      templat.format = x11_window_format;
      templat.width0 = x11_window_attrs.width;
      templat.height0 = x11_window_attrs.height;
      templat.depth0 = 1;
      templat.array_size = 1;
      templat.last_level = 0;
      templat.bind = (PIPE_BIND_RENDER_TARGET | PIPE_BIND_DISPLAY_TARGET);

      scrn->drawable_texture = vscreen->pscreen->resource_create(vscreen->pscreen, &templat);
   } else {
      struct pipe_resource *drawable_texture = NULL;
      pipe_resource_reference(&drawable_texture, scrn->drawable_texture);
   }

   return scrn->drawable_texture;
}

void *
vl_swrast_get_private(struct vl_screen *vscreen)
{
   struct vl_xlib_screen *scrn = (struct vl_xlib_screen *) vscreen;
   return &scrn->xlib_drawable_handle;
}

struct u_rect *
vl_swrast_get_dirty_area(struct vl_screen *vscreen)
{
   struct vl_xlib_screen *scrn = (struct vl_xlib_screen *) vscreen;
   vl_compositor_reset_dirty_area(&scrn->dirty_area);
   return &scrn->dirty_area;
}
