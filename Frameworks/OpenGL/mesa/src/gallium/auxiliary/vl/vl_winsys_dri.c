/**************************************************************************
 *
 * Copyright 2009 Younes Manton.
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

/* directly referenced from target Makefile, because of X dependencies */

#include <sys/types.h>
#include <sys/stat.h>

#include <X11/Xlib-xcb.h>
#include <X11/extensions/dri2tokens.h>
#include <xcb/dri2.h>
#include <xf86drm.h>
#include <errno.h>

#include "loader.h"

#include "pipe/p_screen.h"
#include "pipe/p_context.h"
#include "pipe/p_state.h"
#include "pipe-loader/pipe_loader.h"
#include "frontend/drm_driver.h"

#include "util/u_memory.h"
#include "util/crc32.h"
#include "util/u_hash_table.h"
#include "util/u_inlines.h"

#include "vl/vl_compositor.h"
#include "vl/vl_winsys.h"

#include "drm-uapi/drm_fourcc.h"

struct vl_dri_screen
{
   struct vl_screen base;
   xcb_connection_t *conn;
   xcb_drawable_t drawable;

   unsigned width, height;

   bool current_buffer;
   uint32_t buffer_names[2];
   struct u_rect dirty_areas[2];

   bool flushed;
   xcb_dri2_swap_buffers_cookie_t swap_cookie;
   xcb_dri2_wait_sbc_cookie_t wait_cookie;
   xcb_dri2_get_buffers_cookie_t buffers_cookie;

   int64_t last_ust, ns_frame, last_msc, next_msc;
};

static const unsigned attachments[1] = { XCB_DRI2_ATTACHMENT_BUFFER_BACK_LEFT };

static void vl_dri2_screen_destroy(struct vl_screen *vscreen);

static void
vl_dri2_handle_stamps(struct vl_dri_screen *scrn,
                      uint32_t ust_hi, uint32_t ust_lo,
                      uint32_t msc_hi, uint32_t msc_lo)
{
   int64_t ust = ((((uint64_t)ust_hi) << 32) | ust_lo) * 1000;
   int64_t msc = (((uint64_t)msc_hi) << 32) | msc_lo;

   if (scrn->last_ust && (ust > scrn->last_ust) &&
       scrn->last_msc && (msc > scrn->last_msc))
      scrn->ns_frame = (ust - scrn->last_ust) / (msc - scrn->last_msc);

   scrn->last_ust = ust;
   scrn->last_msc = msc;
}

static xcb_dri2_get_buffers_reply_t *
vl_dri2_get_flush_reply(struct vl_dri_screen *scrn)
{
   xcb_dri2_wait_sbc_reply_t *wait_sbc_reply;

   assert(scrn);

   if (!scrn->flushed)
      return NULL;

   scrn->flushed = false;

   free(xcb_dri2_swap_buffers_reply(scrn->conn, scrn->swap_cookie, NULL));

   wait_sbc_reply = xcb_dri2_wait_sbc_reply(scrn->conn, scrn->wait_cookie, NULL);
   if (!wait_sbc_reply)
      return NULL;
   vl_dri2_handle_stamps(scrn, wait_sbc_reply->ust_hi, wait_sbc_reply->ust_lo,
                         wait_sbc_reply->msc_hi, wait_sbc_reply->msc_lo);
   free(wait_sbc_reply);

   return xcb_dri2_get_buffers_reply(scrn->conn, scrn->buffers_cookie, NULL);
}

static void
vl_dri2_flush_frontbuffer(struct pipe_screen *screen,
                          struct pipe_context *pipe,
                          struct pipe_resource *resource,
                          unsigned level, unsigned layer,
                          void *context_private, struct pipe_box *sub_box)
{
   struct vl_dri_screen *scrn = (struct vl_dri_screen *)context_private;
   uint32_t msc_hi, msc_lo;

   assert(screen);
   assert(resource);
   assert(context_private);

   free(vl_dri2_get_flush_reply(scrn));

   msc_hi = scrn->next_msc >> 32;
   msc_lo = scrn->next_msc & 0xFFFFFFFF;

   scrn->swap_cookie = xcb_dri2_swap_buffers_unchecked(scrn->conn, scrn->drawable,
                                                       msc_hi, msc_lo, 0, 0, 0, 0);
   scrn->wait_cookie = xcb_dri2_wait_sbc_unchecked(scrn->conn, scrn->drawable, 0, 0);
   scrn->buffers_cookie = xcb_dri2_get_buffers_unchecked(scrn->conn, scrn->drawable,
                                                         1, 1, attachments);

   scrn->flushed = true;
   scrn->current_buffer = !scrn->current_buffer;
}

static void
vl_dri2_destroy_drawable(struct vl_dri_screen *scrn)
{
   xcb_void_cookie_t destroy_cookie;
   if (scrn->drawable) {
      free(vl_dri2_get_flush_reply(scrn));
      destroy_cookie = xcb_dri2_destroy_drawable_checked(scrn->conn, scrn->drawable);
      /* ignore any error here, since the drawable can be destroyed long ago */
      free(xcb_request_check(scrn->conn, destroy_cookie));
   }
}

static void
vl_dri2_set_drawable(struct vl_dri_screen *scrn, Drawable drawable)
{
   assert(scrn);
   assert(drawable);

   if (scrn->drawable == drawable)
      return;

   vl_dri2_destroy_drawable(scrn);

   xcb_dri2_create_drawable(scrn->conn, drawable);
   scrn->current_buffer = false;
   vl_compositor_reset_dirty_area(&scrn->dirty_areas[0]);
   vl_compositor_reset_dirty_area(&scrn->dirty_areas[1]);
   scrn->drawable = drawable;
}

static struct pipe_resource *
vl_dri2_screen_texture_from_drawable(struct vl_screen *vscreen, void *drawable)
{
   struct vl_dri_screen *scrn = (struct vl_dri_screen *)vscreen;

   struct winsys_handle dri2_handle;
   struct pipe_resource templ, *tex;

   xcb_dri2_get_buffers_reply_t *reply;
   xcb_dri2_dri2_buffer_t *buffers, *back_left = NULL;

   unsigned depth = ((xcb_screen_t *)(vscreen->xcb_screen))->root_depth;
   unsigned i;

   assert(scrn);

   vl_dri2_set_drawable(scrn, (Drawable)drawable);
   reply = vl_dri2_get_flush_reply(scrn);
   if (!reply) {
      xcb_dri2_get_buffers_cookie_t cookie;
      cookie = xcb_dri2_get_buffers_unchecked(scrn->conn, (Drawable)drawable,
                                              1, 1, attachments);
      reply = xcb_dri2_get_buffers_reply(scrn->conn, cookie, NULL);
   }
   if (!reply)
      return NULL;

   buffers = xcb_dri2_get_buffers_buffers(reply);
   if (!buffers)  {
      free(reply);
      return NULL;
   }

   for (i = 0; i < reply->count; ++i) {
      if (buffers[i].attachment == XCB_DRI2_ATTACHMENT_BUFFER_BACK_LEFT) {
         back_left = &buffers[i];
         break;
      }
   }

   if (i == reply->count || !back_left) {
      free(reply);
      return NULL;
   }

   if (reply->width != scrn->width || reply->height != scrn->height) {
      vl_compositor_reset_dirty_area(&scrn->dirty_areas[0]);
      vl_compositor_reset_dirty_area(&scrn->dirty_areas[1]);
      scrn->width = reply->width;
      scrn->height = reply->height;

   } else if (back_left->name != scrn->buffer_names[scrn->current_buffer]) {
      vl_compositor_reset_dirty_area(&scrn->dirty_areas[scrn->current_buffer]);
      scrn->buffer_names[scrn->current_buffer] = back_left->name;
   }

   memset(&dri2_handle, 0, sizeof(dri2_handle));
   dri2_handle.type = WINSYS_HANDLE_TYPE_SHARED;
   dri2_handle.handle = back_left->name;
   dri2_handle.stride = back_left->pitch;
   dri2_handle.modifier = DRM_FORMAT_MOD_INVALID;

   memset(&templ, 0, sizeof(templ));
   templ.target = PIPE_TEXTURE_2D;
   templ.format = vl_dri2_format_for_depth(vscreen, depth);
   templ.last_level = 0;
   templ.width0 = reply->width;
   templ.height0 = reply->height;
   templ.depth0 = 1;
   templ.array_size = 1;
   templ.usage = PIPE_USAGE_DEFAULT;
   templ.bind = PIPE_BIND_RENDER_TARGET;
   templ.flags = 0;

   tex = scrn->base.pscreen->resource_from_handle(scrn->base.pscreen, &templ,
                                                  &dri2_handle,
                                                  PIPE_HANDLE_USAGE_FRAMEBUFFER_WRITE);
   free(reply);

   return tex;
}

static struct u_rect *
vl_dri2_screen_get_dirty_area(struct vl_screen *vscreen)
{
   struct vl_dri_screen *scrn = (struct vl_dri_screen *)vscreen;
   assert(scrn);
   return &scrn->dirty_areas[scrn->current_buffer];
}

static uint64_t
vl_dri2_screen_get_timestamp(struct vl_screen *vscreen, void *drawable)
{
   struct vl_dri_screen *scrn = (struct vl_dri_screen *)vscreen;
   xcb_dri2_get_msc_cookie_t cookie;
   xcb_dri2_get_msc_reply_t *reply;

   assert(scrn);

   vl_dri2_set_drawable(scrn, (Drawable)drawable);
   if (!scrn->last_ust) {
      cookie = xcb_dri2_get_msc_unchecked(scrn->conn, (Drawable)drawable);
      reply = xcb_dri2_get_msc_reply(scrn->conn, cookie, NULL);

      if (reply) {
         vl_dri2_handle_stamps(scrn, reply->ust_hi, reply->ust_lo,
                               reply->msc_hi, reply->msc_lo);
         free(reply);
      }
   }
   return scrn->last_ust;
}

static void
vl_dri2_screen_set_next_timestamp(struct vl_screen *vscreen, uint64_t stamp)
{
   struct vl_dri_screen *scrn = (struct vl_dri_screen *)vscreen;
   assert(scrn);
   if (stamp && scrn->last_ust && scrn->ns_frame && scrn->last_msc)
      scrn->next_msc = ((int64_t)stamp - scrn->last_ust + scrn->ns_frame/2) /
                       scrn->ns_frame + scrn->last_msc;
   else
      scrn->next_msc = 0;
}

static void *
vl_dri2_screen_get_private(struct vl_screen *vscreen)
{
   return vscreen;
}

static xcb_screen_t *
get_xcb_screen(xcb_screen_iterator_t iter, int screen)
{
    for (; iter.rem; --screen, xcb_screen_next(&iter))
        if (screen == 0)
            return iter.data;

    return NULL;
}

static xcb_visualtype_t *
get_xcb_visualtype_for_depth(struct vl_screen *vscreen, int depth)
{
   xcb_visualtype_iterator_t visual_iter;
   xcb_screen_t *screen = vscreen->xcb_screen;
   xcb_depth_iterator_t depth_iter;

   if (!screen)
      return NULL;

   depth_iter = xcb_screen_allowed_depths_iterator(screen);
   for (; depth_iter.rem; xcb_depth_next(&depth_iter)) {
      if (depth_iter.data->depth != depth)
         continue;

      visual_iter = xcb_depth_visuals_iterator(depth_iter.data);
      if (visual_iter.rem)
         return visual_iter.data;
   }

   return NULL;
}

static uint32_t
get_red_mask_for_depth(struct vl_screen *vscreen, int depth)
{
   xcb_visualtype_t *visual = get_xcb_visualtype_for_depth(vscreen, depth);

   if (visual) {
      return visual->red_mask;
   }

   return 0;
}

uint32_t
vl_dri2_format_for_depth(struct vl_screen *vscreen, int depth)
{
   switch (depth) {
   case 24:
      return PIPE_FORMAT_B8G8R8X8_UNORM;
   case 30:
      /* Different preferred formats for different hw */
      if (get_red_mask_for_depth(vscreen, 30) == 0x3ff)
         return PIPE_FORMAT_R10G10B10X2_UNORM;
      else
         return PIPE_FORMAT_B10G10R10X2_UNORM;
   default:
      return PIPE_FORMAT_NONE;
   }
}

struct vl_screen *
vl_dri2_screen_create(Display *display, int screen)
{
   struct vl_dri_screen *scrn;
   const xcb_query_extension_reply_t *extension;
   xcb_dri2_query_version_cookie_t dri2_query_cookie;
   xcb_dri2_query_version_reply_t *dri2_query = NULL;
   xcb_dri2_connect_cookie_t connect_cookie;
   xcb_dri2_connect_reply_t *connect = NULL;
   xcb_dri2_authenticate_cookie_t authenticate_cookie;
   xcb_dri2_authenticate_reply_t *authenticate = NULL;
   xcb_screen_iterator_t s;
   xcb_generic_error_t *error = NULL;
   char *device_name;
   int fd, device_name_length;
   unsigned driverType;

   drm_magic_t magic;

   assert(display);

   scrn = CALLOC_STRUCT(vl_dri_screen);
   if (!scrn)
      return NULL;

   scrn->conn = XGetXCBConnection(display);
   if (!scrn->conn)
      goto free_screen;

   xcb_prefetch_extension_data(scrn->conn, &xcb_dri2_id);

   extension = xcb_get_extension_data(scrn->conn, &xcb_dri2_id);
   if (!(extension && extension->present))
      goto free_screen;

   dri2_query_cookie = xcb_dri2_query_version (scrn->conn,
                                               XCB_DRI2_MAJOR_VERSION,
                                               XCB_DRI2_MINOR_VERSION);
   dri2_query = xcb_dri2_query_version_reply (scrn->conn, dri2_query_cookie, &error);
   if (dri2_query == NULL || error != NULL || dri2_query->minor_version < 2)
      goto free_query;

   s = xcb_setup_roots_iterator(xcb_get_setup(scrn->conn));
   scrn->base.xcb_screen = get_xcb_screen(s, screen);
   if (!scrn->base.xcb_screen)
      goto free_query;

   driverType = XCB_DRI2_DRIVER_TYPE_DRI;
   {
      char *prime = getenv("DRI_PRIME");
      if (prime) {
         unsigned primeid;
         errno = 0;
         primeid = strtoul(prime, NULL, 0);
         if (errno == 0)
            driverType |=
               ((primeid & DRI2DriverPrimeMask) << DRI2DriverPrimeShift);
      }
   }

   connect_cookie = xcb_dri2_connect_unchecked(
      scrn->conn, ((xcb_screen_t *)(scrn->base.xcb_screen))->root, driverType);
   connect = xcb_dri2_connect_reply(scrn->conn, connect_cookie, NULL);
   if (connect == NULL ||
       connect->driver_name_length + connect->device_name_length == 0)
      goto free_connect;

   device_name_length = xcb_dri2_connect_device_name_length(connect);
   device_name = CALLOC(1, device_name_length + 1);
   if (!device_name)
      goto free_connect;
   memcpy(device_name, xcb_dri2_connect_device_name(connect), device_name_length);
   fd = loader_open_device(device_name);
   free(device_name);

   if (fd < 0)
      goto free_connect;

   if (drmGetMagic(fd, &magic))
      goto close_fd;

   authenticate_cookie = xcb_dri2_authenticate_unchecked(
      scrn->conn, ((xcb_screen_t *)(scrn->base.xcb_screen))->root, magic);
   authenticate = xcb_dri2_authenticate_reply(scrn->conn, authenticate_cookie, NULL);

   if (authenticate == NULL || !authenticate->authenticated)
      goto free_authenticate;

   if (pipe_loader_drm_probe_fd(&scrn->base.dev, fd, false))
      scrn->base.pscreen = pipe_loader_create_screen(scrn->base.dev);

   if (!scrn->base.pscreen)
      goto release_pipe;

   scrn->base.destroy = vl_dri2_screen_destroy;
   scrn->base.texture_from_drawable = vl_dri2_screen_texture_from_drawable;
   scrn->base.get_dirty_area = vl_dri2_screen_get_dirty_area;
   scrn->base.get_timestamp = vl_dri2_screen_get_timestamp;
   scrn->base.set_next_timestamp = vl_dri2_screen_set_next_timestamp;
   scrn->base.get_private = vl_dri2_screen_get_private;
   scrn->base.pscreen->flush_frontbuffer = vl_dri2_flush_frontbuffer;
   vl_compositor_reset_dirty_area(&scrn->dirty_areas[0]);
   vl_compositor_reset_dirty_area(&scrn->dirty_areas[1]);

   /* The pipe loader duplicates the fd */
   close(fd);
   free(authenticate);
   free(connect);
   free(dri2_query);
   free(error);

   return &scrn->base;

release_pipe:
   if (scrn->base.dev)
      pipe_loader_release(&scrn->base.dev, 1);
free_authenticate:
   free(authenticate);
close_fd:
   close(fd);
free_connect:
   free(connect);
free_query:
   free(dri2_query);
   free(error);

free_screen:
   FREE(scrn);
   return NULL;
}

static void
vl_dri2_screen_destroy(struct vl_screen *vscreen)
{
   struct vl_dri_screen *scrn = (struct vl_dri_screen *)vscreen;

   assert(vscreen);

   if (scrn->flushed) {
      free(xcb_dri2_swap_buffers_reply(scrn->conn, scrn->swap_cookie, NULL));
      free(xcb_dri2_wait_sbc_reply(scrn->conn, scrn->wait_cookie, NULL));
      free(xcb_dri2_get_buffers_reply(scrn->conn, scrn->buffers_cookie, NULL));
   }

   vl_dri2_destroy_drawable(scrn);
   scrn->base.pscreen->destroy(scrn->base.pscreen);
   pipe_loader_release(&scrn->base.dev, 1);
   /* There is no user provided fd */
   FREE(scrn);
}
