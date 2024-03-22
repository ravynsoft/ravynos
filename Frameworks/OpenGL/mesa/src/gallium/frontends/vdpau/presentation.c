/**************************************************************************
 *
 * Copyright 2010 Thomas Balling Sørensen.
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

#include <stdio.h>
#include <vdpau/vdpau.h>

#include "util/u_debug.h"
#include "util/u_memory.h"

#include "vdpau_private.h"

/**
 * Create a VdpPresentationQueue.
 */
VdpStatus
vlVdpPresentationQueueCreate(VdpDevice device,
                             VdpPresentationQueueTarget presentation_queue_target,
                             VdpPresentationQueue *presentation_queue)
{
   vlVdpPresentationQueue *pq = NULL;
   VdpStatus ret;

   if (!presentation_queue)
      return VDP_STATUS_INVALID_POINTER;

   vlVdpDevice *dev = vlGetDataHTAB(device);
   if (!dev)
      return VDP_STATUS_INVALID_HANDLE;

   vlVdpPresentationQueueTarget *pqt = vlGetDataHTAB(presentation_queue_target);
   if (!pqt)
      return VDP_STATUS_INVALID_HANDLE;

   if (dev != pqt->device)
      return VDP_STATUS_HANDLE_DEVICE_MISMATCH;

   pq = CALLOC(1, sizeof(vlVdpPresentationQueue));
   if (!pq)
      return VDP_STATUS_RESOURCES;

   DeviceReference(&pq->device, dev);
   pq->drawable = pqt->drawable;

   mtx_lock(&dev->mutex);
   if (!vl_compositor_init_state(&pq->cstate, dev->context)) {
      mtx_unlock(&dev->mutex);
      ret = VDP_STATUS_ERROR;
      goto no_compositor;
   }
   mtx_unlock(&dev->mutex);

   *presentation_queue = vlAddDataHTAB(pq);
   if (*presentation_queue == 0) {
      ret = VDP_STATUS_ERROR;
      goto no_handle;
   }

   return VDP_STATUS_OK;

no_handle:
no_compositor:
   DeviceReference(&pq->device, NULL);
   FREE(pq);
   return ret;
}

/**
 * Destroy a VdpPresentationQueue.
 */
VdpStatus
vlVdpPresentationQueueDestroy(VdpPresentationQueue presentation_queue)
{
   vlVdpPresentationQueue *pq;

   pq = vlGetDataHTAB(presentation_queue);
   if (!pq)
      return VDP_STATUS_INVALID_HANDLE;

   mtx_lock(&pq->device->mutex);
   vl_compositor_cleanup_state(&pq->cstate);
   mtx_unlock(&pq->device->mutex);

   vlRemoveDataHTAB(presentation_queue);
   DeviceReference(&pq->device, NULL);
   FREE(pq);

   return VDP_STATUS_OK;
}

/**
 * Configure the background color setting.
 */
VdpStatus
vlVdpPresentationQueueSetBackgroundColor(VdpPresentationQueue presentation_queue,
                                         VdpColor *const background_color)
{
   vlVdpPresentationQueue *pq;
   union pipe_color_union color;

   if (!background_color)
      return VDP_STATUS_INVALID_POINTER;

   pq = vlGetDataHTAB(presentation_queue);
   if (!pq)
      return VDP_STATUS_INVALID_HANDLE;

   color.f[0] = background_color->red;
   color.f[1] = background_color->green;
   color.f[2] = background_color->blue;
   color.f[3] = background_color->alpha;

   mtx_lock(&pq->device->mutex);
   vl_compositor_set_clear_color(&pq->cstate, &color);
   mtx_unlock(&pq->device->mutex);

   return VDP_STATUS_OK;
}

/**
 * Retrieve the current background color setting.
 */
VdpStatus
vlVdpPresentationQueueGetBackgroundColor(VdpPresentationQueue presentation_queue,
                                         VdpColor *const background_color)
{
   vlVdpPresentationQueue *pq;
   union pipe_color_union color;

   if (!background_color)
      return VDP_STATUS_INVALID_POINTER;

   pq = vlGetDataHTAB(presentation_queue);
   if (!pq)
      return VDP_STATUS_INVALID_HANDLE;

   mtx_lock(&pq->device->mutex);
   vl_compositor_get_clear_color(&pq->cstate, &color);
   mtx_unlock(&pq->device->mutex);

   background_color->red = color.f[0];
   background_color->green = color.f[1];
   background_color->blue = color.f[2];
   background_color->alpha = color.f[3];

   return VDP_STATUS_OK;
}

/**
 * Retrieve the presentation queue's "current" time.
 */
VdpStatus
vlVdpPresentationQueueGetTime(VdpPresentationQueue presentation_queue,
                              VdpTime *current_time)
{
   vlVdpPresentationQueue *pq;

   if (!current_time)
      return VDP_STATUS_INVALID_POINTER;

   pq = vlGetDataHTAB(presentation_queue);
   if (!pq)
      return VDP_STATUS_INVALID_HANDLE;

   mtx_lock(&pq->device->mutex);
   *current_time = pq->device->vscreen->get_timestamp(pq->device->vscreen,
                                                      (void *)pq->drawable);
   mtx_unlock(&pq->device->mutex);

   return VDP_STATUS_OK;
}

/**
 * Enter a surface into the presentation queue.
 */
VdpStatus
vlVdpPresentationQueueDisplay(VdpPresentationQueue presentation_queue,
                              VdpOutputSurface surface,
                              uint32_t clip_width,
                              uint32_t clip_height,
                              VdpTime  earliest_presentation_time)
{
   static int dump_window = -1;

   vlVdpPresentationQueue *pq;
   vlVdpOutputSurface *surf;

   struct pipe_context *pipe;
   struct pipe_resource *tex;
   struct pipe_surface surf_templ, *surf_draw = NULL;
   struct u_rect src_rect, dst_clip, *dirty_area;

   struct vl_compositor *compositor;
   struct vl_compositor_state *cstate;
   struct vl_screen *vscreen;

   pq = vlGetDataHTAB(presentation_queue);
   if (!pq)
      return VDP_STATUS_INVALID_HANDLE;

   surf = vlGetDataHTAB(surface);
   if (!surf)
      return VDP_STATUS_INVALID_HANDLE;

   pipe = pq->device->context;
   compositor = &pq->device->compositor;
   cstate = &pq->cstate;
   vscreen = pq->device->vscreen;

   mtx_lock(&pq->device->mutex);
   if (vscreen->set_back_texture_from_output && surf->send_to_X)
      vscreen->set_back_texture_from_output(vscreen, surf->surface->texture, clip_width, clip_height);
   tex = vscreen->texture_from_drawable(vscreen, (void *)pq->drawable);
   if (!tex) {
      mtx_unlock(&pq->device->mutex);
      return VDP_STATUS_INVALID_HANDLE;
   }

   if (!vscreen->set_back_texture_from_output || !surf->send_to_X) {
      dirty_area = vscreen->get_dirty_area(vscreen);

      memset(&surf_templ, 0, sizeof(surf_templ));
      surf_templ.format = tex->format;
      surf_draw = pipe->create_surface(pipe, tex, &surf_templ);

      dst_clip.x0 = 0;
      dst_clip.y0 = 0;
      dst_clip.x1 = clip_width ? clip_width : surf_draw->width;
      dst_clip.y1 = clip_height ? clip_height : surf_draw->height;

      src_rect.x0 = 0;
      src_rect.y0 = 0;
      src_rect.x1 = surf_draw->width;
      src_rect.y1 = surf_draw->height;

      vl_compositor_clear_layers(cstate);
      vl_compositor_set_rgba_layer(cstate, compositor, 0, surf->sampler_view, &src_rect, NULL, NULL);
      vl_compositor_set_dst_clip(cstate, &dst_clip);
      vl_compositor_render(cstate, compositor, surf_draw, dirty_area, true);
   }

   vscreen->set_next_timestamp(vscreen, earliest_presentation_time);

   // flush before calling flush_frontbuffer so that rendering is flushed
   //  to back buffer so the texture can be copied in flush_frontbuffer
   pipe->screen->fence_reference(pipe->screen, &surf->fence, NULL);
   pipe->flush(pipe, &surf->fence, 0);
   pipe->screen->flush_frontbuffer(pipe->screen, pipe, tex, 0, 0,
                                   vscreen->get_private(vscreen), NULL);

   pq->last_surf = surf;

   if (dump_window == -1) {
      dump_window = debug_get_num_option("VDPAU_DUMP", 0);
   }

   if (dump_window) {
      static unsigned int framenum = 0;
      char cmd[256];

      if (framenum) {
         sprintf(cmd, "xwd -id %d -silent -out vdpau_frame_%08d.xwd", (int)pq->drawable, framenum);
         if (system(cmd) != 0)
            VDPAU_MSG(VDPAU_ERR, "[VDPAU] Dumping surface %d failed.\n", surface);
      }
      framenum++;
   }

   if (!vscreen->set_back_texture_from_output || !surf->send_to_X) {
      pipe_resource_reference(&tex, NULL);
      pipe_surface_reference(&surf_draw, NULL);
   }
   mtx_unlock(&pq->device->mutex);

   return VDP_STATUS_OK;
}

/**
 * Wait for a surface to finish being displayed.
 */
VdpStatus
vlVdpPresentationQueueBlockUntilSurfaceIdle(VdpPresentationQueue presentation_queue,
                                            VdpOutputSurface surface,
                                            VdpTime *first_presentation_time)
{
   vlVdpPresentationQueue *pq;
   vlVdpOutputSurface *surf;
   struct pipe_screen *screen;

   if (!first_presentation_time)
      return VDP_STATUS_INVALID_POINTER;

   pq = vlGetDataHTAB(presentation_queue);
   if (!pq)
      return VDP_STATUS_INVALID_HANDLE;

   surf = vlGetDataHTAB(surface);
   if (!surf)
      return VDP_STATUS_INVALID_HANDLE;

   mtx_lock(&pq->device->mutex);
   if (surf->fence) {
      screen = pq->device->vscreen->pscreen;
      screen->fence_finish(screen, NULL, surf->fence, OS_TIMEOUT_INFINITE);
      screen->fence_reference(screen, &surf->fence, NULL);
   }
   mtx_unlock(&pq->device->mutex);

   return vlVdpPresentationQueueGetTime(presentation_queue, first_presentation_time);
}

/**
 * Poll the current queue status of a surface.
 */
VdpStatus
vlVdpPresentationQueueQuerySurfaceStatus(VdpPresentationQueue presentation_queue,
                                         VdpOutputSurface surface,
                                         VdpPresentationQueueStatus *status,
                                         VdpTime *first_presentation_time)
{
   vlVdpPresentationQueue *pq;
   vlVdpOutputSurface *surf;
   struct pipe_screen *screen;

   if (!(status && first_presentation_time))
      return VDP_STATUS_INVALID_POINTER;

   pq = vlGetDataHTAB(presentation_queue);
   if (!pq)
      return VDP_STATUS_INVALID_HANDLE;

   surf = vlGetDataHTAB(surface);
   if (!surf)
      return VDP_STATUS_INVALID_HANDLE;

   *first_presentation_time = 0;

   if (!surf->fence) {
      if (pq->last_surf == surf)
         *status = VDP_PRESENTATION_QUEUE_STATUS_VISIBLE;
      else
         *status = VDP_PRESENTATION_QUEUE_STATUS_IDLE;
   } else {
      mtx_lock(&pq->device->mutex);
      screen = pq->device->vscreen->pscreen;
      if (screen->fence_finish(screen, NULL, surf->fence, 0)) {
         screen->fence_reference(screen, &surf->fence, NULL);
         *status = VDP_PRESENTATION_QUEUE_STATUS_VISIBLE;
         mtx_unlock(&pq->device->mutex);

         // We actually need to query the timestamp of the last VSYNC event from the hardware
         vlVdpPresentationQueueGetTime(presentation_queue, first_presentation_time);
         *first_presentation_time += 1;
      } else {
         *status = VDP_PRESENTATION_QUEUE_STATUS_QUEUED;
         mtx_unlock(&pq->device->mutex);
      }
   }

   return VDP_STATUS_OK;
}
