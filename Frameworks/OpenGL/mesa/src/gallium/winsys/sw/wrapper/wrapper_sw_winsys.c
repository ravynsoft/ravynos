/**********************************************************
 * Copyright 2010 VMware, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 **********************************************************/


#include "wrapper_sw_winsys.h"

#include "util/format/u_formats.h"
#include "pipe/p_state.h"

#include "frontend/sw_winsys.h"

#include "util/u_memory.h"
#include "util/u_inlines.h"

/*
 * This code wraps a pipe_screen and exposes a sw_winsys interface for use
 * with software resterizers. This code is used by the DRM based winsys to
 * allow access to the drm driver.
 *
 * We must borrow the whole stack because only the pipe screen knows how
 * to decode the content of a buffer. Or how to create a buffer that
 * can still be used by drivers using real hardware (as the case is
 * with software st/xorg but hw st/dri).
 *
 * We also need a pipe context for the transfers.
 */

struct wrapper_sw_winsys
{
   struct sw_winsys base;
   struct pipe_screen *screen;
   struct pipe_context *pipe;
   enum pipe_texture_target target;
};

struct wrapper_sw_displaytarget
{
   struct wrapper_sw_winsys *winsys;
   struct pipe_resource *tex;
   struct pipe_transfer *transfer;

   unsigned map_count;
   unsigned stride; /**< because we get stride at create */
   void *ptr;
};

static inline struct wrapper_sw_winsys *
wrapper_sw_winsys(struct sw_winsys *ws)
{
   return (struct wrapper_sw_winsys *)ws;
}

static inline struct wrapper_sw_displaytarget *
wrapper_sw_displaytarget(struct sw_displaytarget *dt)
{
   return (struct wrapper_sw_displaytarget *)dt;
}


/*
 * Functions
 */


static bool
wsw_is_dt_format_supported(struct sw_winsys *ws,
                           unsigned tex_usage,
                           enum pipe_format format)
{
   struct wrapper_sw_winsys *wsw = wrapper_sw_winsys(ws);

   return wsw->screen->is_format_supported(wsw->screen, format,
                                           PIPE_TEXTURE_2D, 0, 0,
                                           PIPE_BIND_RENDER_TARGET |
                                           PIPE_BIND_DISPLAY_TARGET);
}

static bool
wsw_dt_get_stride(struct wrapper_sw_displaytarget *wdt, unsigned *stride)
{
   struct pipe_context *pipe = wdt->winsys->pipe;
   struct pipe_resource *tex = wdt->tex;
   struct pipe_transfer *tr;
   void *map;

   map = pipe_texture_map(pipe, tex, 0, 0,
                           PIPE_MAP_READ_WRITE,
                           0, 0, wdt->tex->width0, wdt->tex->height0, &tr);
   if (!map)
      return false;

   *stride = tr->stride;
   wdt->stride = tr->stride;

   pipe->texture_unmap(pipe, tr);

   return true;
}

static struct sw_displaytarget *
wsw_dt_wrap_texture(struct wrapper_sw_winsys *wsw,
                    struct pipe_resource *tex, unsigned *stride)
{
   struct wrapper_sw_displaytarget *wdt = CALLOC_STRUCT(wrapper_sw_displaytarget);
   if (!wdt)
      goto err_unref;

   wdt->tex = tex;
   wdt->winsys = wsw;

   if (!wsw_dt_get_stride(wdt, stride))
      goto err_free;

   return (struct sw_displaytarget *)wdt;

err_free:
   FREE(wdt);
err_unref:
   pipe_resource_reference(&tex, NULL);
   return NULL;
}

static struct sw_displaytarget *
wsw_dt_create(struct sw_winsys *ws,
              unsigned bind,
              enum pipe_format format,
              unsigned width, unsigned height,
              unsigned alignment,
              const void *front_private,
              unsigned *stride)
{
   struct wrapper_sw_winsys *wsw = wrapper_sw_winsys(ws);
   struct pipe_resource templ;
   struct pipe_resource *tex;

   /*
    * XXX Why don't we just get the template.
    */
   memset(&templ, 0, sizeof(templ));
   templ.target = wsw->target;
   templ.width0 = width;
   templ.height0 = height;
   templ.depth0 = 1;
   templ.array_size = 1;
   templ.format = format;
   templ.bind = bind;

   /* XXX alignment: we can't do anything about this */

   tex = wsw->screen->resource_create(wsw->screen, &templ);
   if (!tex)
      return NULL;

   return wsw_dt_wrap_texture(wsw, tex, stride);
}

static struct sw_displaytarget *
wsw_dt_from_handle(struct sw_winsys *ws,
                   const struct pipe_resource *templ,
                   struct winsys_handle *whandle,
                   unsigned *stride)
{
   struct wrapper_sw_winsys *wsw = wrapper_sw_winsys(ws);
   struct pipe_resource *tex;

   tex = wsw->screen->resource_from_handle(wsw->screen, templ, whandle,
                                           PIPE_HANDLE_USAGE_FRAMEBUFFER_WRITE);
   if (!tex)
      return NULL;

   return wsw_dt_wrap_texture(wsw, tex, stride);
}

static bool
wsw_dt_get_handle(struct sw_winsys *ws,
                  struct sw_displaytarget *dt,
                  struct winsys_handle *whandle)
{
   struct wrapper_sw_winsys *wsw = wrapper_sw_winsys(ws);
   struct wrapper_sw_displaytarget *wdt = wrapper_sw_displaytarget(dt);
   struct pipe_resource *tex = wdt->tex;

   return wsw->screen->resource_get_handle(wsw->screen, NULL, tex, whandle,
                                           PIPE_HANDLE_USAGE_FRAMEBUFFER_WRITE);
}

static void *
wsw_dt_map(struct sw_winsys *ws,
           struct sw_displaytarget *dt,
           unsigned flags)
{
   struct wrapper_sw_displaytarget *wdt = wrapper_sw_displaytarget(dt);
   struct pipe_context *pipe = wdt->winsys->pipe;
   struct pipe_resource *tex = wdt->tex;
   struct pipe_transfer *tr;
   void *ptr;

   if (!wdt->map_count) {

      assert(!wdt->transfer);

      ptr = pipe_texture_map(pipe, tex, 0, 0,
                              PIPE_MAP_READ_WRITE,
                              0, 0, wdt->tex->width0, wdt->tex->height0, &tr);
      if (!ptr)
        goto err;

      wdt->transfer = tr;
      wdt->ptr = ptr;

      /* XXX Handle this case */
      assert(tr->stride == wdt->stride);
   }

   wdt->map_count++;

   return wdt->ptr;

err:
   pipe->texture_unmap(pipe, tr);
   return NULL;
}

static void
wsw_dt_unmap(struct sw_winsys *ws,
             struct sw_displaytarget *dt)
{
   struct wrapper_sw_displaytarget *wdt = wrapper_sw_displaytarget(dt);
   struct pipe_context *pipe = wdt->winsys->pipe;

   assert(wdt->transfer);

   wdt->map_count--;

   if (wdt->map_count)
      return;

   pipe->texture_unmap(pipe, wdt->transfer);
   pipe->flush(pipe, NULL, 0);
   wdt->transfer = NULL;
}

static void
wsw_dt_destroy(struct sw_winsys *ws,
               struct sw_displaytarget *dt)
{
   struct wrapper_sw_displaytarget *wdt = wrapper_sw_displaytarget(dt);

   pipe_resource_reference(&wdt->tex, NULL);

   FREE(wdt);
}

static void
wsw_destroy(struct sw_winsys *ws)
{
   struct wrapper_sw_winsys *wsw = wrapper_sw_winsys(ws);

   wsw->pipe->destroy(wsw->pipe);
   wsw->screen->destroy(wsw->screen);

   FREE(wsw);
}

struct sw_winsys *
wrapper_sw_winsys_wrap_pipe_screen(struct pipe_screen *screen)
{
   struct wrapper_sw_winsys *wsw = CALLOC_STRUCT(wrapper_sw_winsys);

   if (!wsw)
      goto err;

   wsw->base.is_displaytarget_format_supported = wsw_is_dt_format_supported;
   wsw->base.displaytarget_create = wsw_dt_create;
   wsw->base.displaytarget_from_handle = wsw_dt_from_handle;
   wsw->base.displaytarget_get_handle = wsw_dt_get_handle;
   wsw->base.displaytarget_map = wsw_dt_map;
   wsw->base.displaytarget_unmap = wsw_dt_unmap;
   wsw->base.displaytarget_destroy = wsw_dt_destroy;
   wsw->base.destroy = wsw_destroy;

   wsw->screen = screen;
   wsw->pipe = screen->context_create(screen, NULL, 0);
   if (!wsw->pipe)
      goto err_free;

   if(screen->get_param(screen, PIPE_CAP_NPOT_TEXTURES))
      wsw->target = PIPE_TEXTURE_2D;
   else
      wsw->target = PIPE_TEXTURE_RECT;

   return &wsw->base;

err_free:
   FREE(wsw);
err:
   return NULL;
}

struct pipe_screen *
wrapper_sw_winsys_dewrap_pipe_screen(struct sw_winsys *ws)
{
   struct wrapper_sw_winsys *wsw = wrapper_sw_winsys(ws);
   struct pipe_screen *screen = wsw->screen;

   wsw->pipe->destroy(wsw->pipe);
   /* don't destroy the screen its needed later on */

   FREE(wsw);
   return screen;
}
