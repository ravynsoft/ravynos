/**************************************************************************
 *
 * Copyright 2010 Thomas Balling Sørensen.
 * Copyright 2011 Christian König.
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

#include <assert.h>

#include "pipe/p_state.h"

#include "util/u_memory.h"
#include "util/u_debug.h"
#include "util/u_rect.h"
#include "util/u_surface.h"
#include "util/u_video.h"
#include "vl/vl_defines.h"

#include "frontend/drm_driver.h"

#include "vdpau_private.h"

enum getbits_conversion {
   CONVERSION_NONE,
   CONVERSION_NV12_TO_YV12,
   CONVERSION_YV12_TO_NV12,
   CONVERSION_SWAP_YUYV_UYVY,
};

/**
 * Create a VdpVideoSurface.
 */
VdpStatus
vlVdpVideoSurfaceCreate(VdpDevice device, VdpChromaType chroma_type,
                        uint32_t width, uint32_t height,
                        VdpVideoSurface *surface)
{
   struct pipe_context *pipe;
   vlVdpSurface *p_surf;
   VdpStatus ret;

   if (!(width && height)) {
      ret = VDP_STATUS_INVALID_SIZE;
      goto inv_size;
   }

   p_surf = CALLOC(1, sizeof(vlVdpSurface));
   if (!p_surf) {
      ret = VDP_STATUS_RESOURCES;
      goto no_res;
   }

   vlVdpDevice *dev = vlGetDataHTAB(device);
   if (!dev) {
      ret = VDP_STATUS_INVALID_HANDLE;
      goto inv_device;
   }

   DeviceReference(&p_surf->device, dev);
   pipe = dev->context;

   mtx_lock(&dev->mutex);
   memset(&p_surf->templat, 0, sizeof(p_surf->templat));
   /* TODO: buffer_format should be selected to match chroma_type */
   p_surf->templat.buffer_format = pipe->screen->get_video_param
   (
      pipe->screen,
      PIPE_VIDEO_PROFILE_UNKNOWN,
      PIPE_VIDEO_ENTRYPOINT_BITSTREAM,
      PIPE_VIDEO_CAP_PREFERED_FORMAT
   );
   p_surf->templat.width = width;
   p_surf->templat.height = height;
   p_surf->templat.interlaced = pipe->screen->get_video_param
   (
      pipe->screen,
      PIPE_VIDEO_PROFILE_UNKNOWN,
      PIPE_VIDEO_ENTRYPOINT_BITSTREAM,
      PIPE_VIDEO_CAP_PREFERS_INTERLACED
   );
   if (p_surf->templat.buffer_format != PIPE_FORMAT_NONE)
      p_surf->video_buffer = pipe->create_video_buffer(pipe, &p_surf->templat);

   /* do not mandate early allocation of a video buffer */
   vlVdpVideoSurfaceClear(p_surf);
   mtx_unlock(&dev->mutex);

   *surface = vlAddDataHTAB(p_surf);
   if (*surface == 0) {
      ret = VDP_STATUS_ERROR;
      goto no_handle;
   }

   return VDP_STATUS_OK;

no_handle:
   p_surf->video_buffer->destroy(p_surf->video_buffer);

inv_device:
   DeviceReference(&p_surf->device, NULL);
   FREE(p_surf);

no_res:
inv_size:
   return ret;
}

/**
 * Destroy a VdpVideoSurface.
 */
VdpStatus
vlVdpVideoSurfaceDestroy(VdpVideoSurface surface)
{
   vlVdpSurface *p_surf;

   p_surf = (vlVdpSurface *)vlGetDataHTAB((vlHandle)surface);
   if (!p_surf)
      return VDP_STATUS_INVALID_HANDLE;

   mtx_lock(&p_surf->device->mutex);
   if (p_surf->video_buffer)
      p_surf->video_buffer->destroy(p_surf->video_buffer);
   mtx_unlock(&p_surf->device->mutex);

   vlRemoveDataHTAB(surface);
   DeviceReference(&p_surf->device, NULL);
   FREE(p_surf);

   return VDP_STATUS_OK;
}

/**
 * Retrieve the parameters used to create a VdpVideoSurface.
 */
VdpStatus
vlVdpVideoSurfaceGetParameters(VdpVideoSurface surface,
                               VdpChromaType *chroma_type,
                               uint32_t *width, uint32_t *height)
{
   if (!(width && height && chroma_type))
      return VDP_STATUS_INVALID_POINTER;

   vlVdpSurface *p_surf = vlGetDataHTAB(surface);
   if (!p_surf)
      return VDP_STATUS_INVALID_HANDLE;

   if (p_surf->video_buffer) {
      *width = p_surf->video_buffer->width;
      *height = p_surf->video_buffer->height;
      *chroma_type = PipeToChroma(pipe_format_to_chroma_format(p_surf->video_buffer->buffer_format));
   } else {
      *width = p_surf->templat.width;
      *height = p_surf->templat.height;
      *chroma_type = PipeToChroma(pipe_format_to_chroma_format(p_surf->templat.buffer_format));
   }

   return VDP_STATUS_OK;
}

static void
vlVdpVideoSurfaceSize(vlVdpSurface *p_surf, int component,
                      unsigned *width, unsigned *height)
{
   *width = p_surf->templat.width;
   *height = p_surf->templat.height;

   vl_video_buffer_adjust_size(width, height, component,
                               pipe_format_to_chroma_format(p_surf->templat.buffer_format),
                               p_surf->templat.interlaced);
}

/**
 * Copy image data from a VdpVideoSurface to application memory in a specified
 * YCbCr format.
 */
VdpStatus
vlVdpVideoSurfaceGetBitsYCbCr(VdpVideoSurface surface,
                              VdpYCbCrFormat destination_ycbcr_format,
                              void *const *destination_data,
                              uint32_t const *destination_pitches)
{
   vlVdpSurface *vlsurface;
   struct pipe_context *pipe;
   enum pipe_format format, buffer_format;
   struct pipe_sampler_view **sampler_views;
   enum getbits_conversion conversion = CONVERSION_NONE;
   unsigned i, j;

   vlsurface = vlGetDataHTAB(surface);
   if (!vlsurface)
      return VDP_STATUS_INVALID_HANDLE;

   pipe = vlsurface->device->context;
   if (!pipe)
      return VDP_STATUS_INVALID_HANDLE;

   if (!destination_data || !destination_pitches)
       return VDP_STATUS_INVALID_POINTER;

   format = FormatYCBCRToPipe(destination_ycbcr_format);
   if (format == PIPE_FORMAT_NONE)
      return VDP_STATUS_INVALID_Y_CB_CR_FORMAT;

   if (vlsurface->video_buffer == NULL)
      return VDP_STATUS_INVALID_VALUE;

   buffer_format = vlsurface->video_buffer->buffer_format;
   if (format != buffer_format) {
      if (format == PIPE_FORMAT_YV12 && buffer_format == PIPE_FORMAT_NV12)
         conversion = CONVERSION_NV12_TO_YV12;
      else if (format == PIPE_FORMAT_NV12 && buffer_format == PIPE_FORMAT_YV12)
         conversion = CONVERSION_YV12_TO_NV12;
      else if ((format == PIPE_FORMAT_YUYV && buffer_format == PIPE_FORMAT_UYVY) ||
               (format == PIPE_FORMAT_UYVY && buffer_format == PIPE_FORMAT_YUYV))
         conversion = CONVERSION_SWAP_YUYV_UYVY;
      else
         return VDP_STATUS_NO_IMPLEMENTATION;
   }

   mtx_lock(&vlsurface->device->mutex);
   sampler_views = vlsurface->video_buffer->get_sampler_view_planes(vlsurface->video_buffer);
   if (!sampler_views) {
      mtx_unlock(&vlsurface->device->mutex);
      return VDP_STATUS_RESOURCES;
   }

   for (i = 0; i < 3; ++i) {
      unsigned width, height;
      struct pipe_sampler_view *sv = sampler_views[i];
      if (!sv) continue;

      vlVdpVideoSurfaceSize(vlsurface, i, &width, &height);

      for (j = 0; j < sv->texture->array_size; ++j) {
         struct pipe_box box = {
            0, 0, j,
            width, height, 1
         };
         struct pipe_transfer *transfer;
         uint8_t *map;

         map = pipe->texture_map(pipe, sv->texture, 0,
                                       PIPE_MAP_READ, &box, &transfer);
         if (!map) {
            mtx_unlock(&vlsurface->device->mutex);
            return VDP_STATUS_RESOURCES;
         }

         if (conversion == CONVERSION_NV12_TO_YV12 && i == 1) {
            u_copy_nv12_to_yv12(destination_data, destination_pitches,
                                i, j, transfer->stride, sv->texture->array_size,
                                map, box.width, box.height);
         } else if (conversion == CONVERSION_YV12_TO_NV12 && i > 0) {
            u_copy_yv12_to_nv12(destination_data, destination_pitches,
                                i, j, transfer->stride, sv->texture->array_size,
                                map, box.width, box.height);
         } else if (conversion == CONVERSION_SWAP_YUYV_UYVY) {
            u_copy_swap422_packed(destination_data, destination_pitches,
                                   i, j, transfer->stride, sv->texture->array_size,
                                   map, box.width, box.height);
         } else {
            util_copy_rect(destination_data[i] + destination_pitches[i] * j, sv->texture->format,
                           destination_pitches[i] * sv->texture->array_size, 0, 0,
                           box.width, box.height, map, transfer->stride, 0, 0);
         }

         pipe_texture_unmap(pipe, transfer);
      }
   }
   mtx_unlock(&vlsurface->device->mutex);

   return VDP_STATUS_OK;
}

/**
 * Copy image data from application memory in a specific YCbCr format to
 * a VdpVideoSurface.
 */
VdpStatus
vlVdpVideoSurfacePutBitsYCbCr(VdpVideoSurface surface,
                              VdpYCbCrFormat source_ycbcr_format,
                              void const *const *source_data,
                              uint32_t const *source_pitches)
{
   enum pipe_format pformat = FormatYCBCRToPipe(source_ycbcr_format);
   enum getbits_conversion conversion = CONVERSION_NONE;
   struct pipe_context *pipe;
   struct pipe_sampler_view **sampler_views;
   unsigned i, j;
   unsigned usage = PIPE_MAP_WRITE;

   vlVdpSurface *p_surf = vlGetDataHTAB(surface);
   if (!p_surf)
      return VDP_STATUS_INVALID_HANDLE;

   pipe = p_surf->device->context;
   if (!pipe)
      return VDP_STATUS_INVALID_HANDLE;

   if (!source_data || !source_pitches)
       return VDP_STATUS_INVALID_POINTER;

   mtx_lock(&p_surf->device->mutex);

   if (p_surf->video_buffer == NULL ||
       ((pformat != p_surf->video_buffer->buffer_format))) {
      enum pipe_format nformat = pformat;
      struct pipe_screen *screen = pipe->screen;

      /* Determine the most suitable format for the new surface */
      if (!screen->is_video_format_supported(screen, nformat,
                                             PIPE_VIDEO_PROFILE_UNKNOWN,
                                             PIPE_VIDEO_ENTRYPOINT_BITSTREAM)) {
         nformat = screen->get_video_param(screen,
                                           PIPE_VIDEO_PROFILE_UNKNOWN,
                                           PIPE_VIDEO_ENTRYPOINT_BITSTREAM,
                                           PIPE_VIDEO_CAP_PREFERED_FORMAT);
         if (nformat == PIPE_FORMAT_NONE) {
            mtx_unlock(&p_surf->device->mutex);
            return VDP_STATUS_NO_IMPLEMENTATION;
         }
      }

      if (p_surf->video_buffer == NULL  ||
          nformat != p_surf->video_buffer->buffer_format) {
         /* destroy the old one */
         if (p_surf->video_buffer)
            p_surf->video_buffer->destroy(p_surf->video_buffer);

         /* adjust the template parameters */
         p_surf->templat.buffer_format = nformat;
         if (nformat == PIPE_FORMAT_YUYV || nformat == PIPE_FORMAT_UYVY)
            p_surf->templat.interlaced = false;

         /* and try to create the video buffer with the new format */
         p_surf->video_buffer = pipe->create_video_buffer(pipe, &p_surf->templat);

         /* stil no luck? ok forget it we don't support it */
         if (!p_surf->video_buffer) {
            mtx_unlock(&p_surf->device->mutex);
            return VDP_STATUS_NO_IMPLEMENTATION;
         }
         vlVdpVideoSurfaceClear(p_surf);
      }
   }

   if (pformat != p_surf->video_buffer->buffer_format) {
      if (pformat == PIPE_FORMAT_YV12 &&
          p_surf->video_buffer->buffer_format == PIPE_FORMAT_NV12)
         conversion = CONVERSION_YV12_TO_NV12;
      else {
         mtx_unlock(&p_surf->device->mutex);
         return VDP_STATUS_NO_IMPLEMENTATION;
      }
   }

   sampler_views = p_surf->video_buffer->get_sampler_view_planes(p_surf->video_buffer);
   if (!sampler_views) {
      mtx_unlock(&p_surf->device->mutex);
      return VDP_STATUS_RESOURCES;
   }

   for (i = 0; i < 3; ++i) {
      unsigned width, height;
      struct pipe_sampler_view *sv = sampler_views[i];
      struct pipe_resource *tex;
      if (!sv || !source_pitches[i]) continue;

      tex = sv->texture;
      vlVdpVideoSurfaceSize(p_surf, i, &width, &height);

      for (j = 0; j < tex->array_size; ++j) {
         struct pipe_box dst_box = {
            0, 0, j,
            width, height, 1
         };

         if (conversion == CONVERSION_YV12_TO_NV12 && i == 1) {
            struct pipe_transfer *transfer;
            uint8_t *map;

            map = pipe->texture_map(pipe, tex, 0, usage,
                                     &dst_box, &transfer);
            if (!map) {
               mtx_unlock(&p_surf->device->mutex);
               return VDP_STATUS_RESOURCES;
            }

            u_copy_nv12_from_yv12(source_data, source_pitches,
                                  i, j, transfer->stride, tex->array_size,
                                  map, dst_box.width, dst_box.height);

            pipe_texture_unmap(pipe, transfer);
         } else {
            pipe->texture_subdata(pipe, tex, 0,
                                  PIPE_MAP_WRITE, &dst_box,
                                  source_data[i] + source_pitches[i] * j,
                                  source_pitches[i] * tex->array_size,
                                  0);
         }
         /*
          * This surface has already been synced
          * by the first map.
          */
         usage |= PIPE_MAP_UNSYNCHRONIZED;
      }
   }
   mtx_unlock(&p_surf->device->mutex);

   return VDP_STATUS_OK;
}

/**
 * Helper function to initially clear the VideoSurface after (re-)creation
 */
void
vlVdpVideoSurfaceClear(vlVdpSurface *vlsurf)
{
   struct pipe_context *pipe = vlsurf->device->context;
   struct pipe_surface **surfaces;
   unsigned i;

   if (!vlsurf->video_buffer)
      return;

   surfaces = vlsurf->video_buffer->get_surfaces(vlsurf->video_buffer);
   for (i = 0; i < VL_MAX_SURFACES; ++i) {
      union pipe_color_union c = {};

      if (!surfaces[i])
         continue;

      if (i > !!vlsurf->templat.interlaced)
         c.f[0] = c.f[1] = c.f[2] = c.f[3] = 0.5f;

      pipe->clear_render_target(pipe, surfaces[i], &c, 0, 0,
                                surfaces[i]->width, surfaces[i]->height, false);
   }
   pipe->flush(pipe, NULL, 0);
}

/**
 * Interop for the GL gallium frontend
 */
struct pipe_video_buffer *vlVdpVideoSurfaceGallium(VdpVideoSurface surface)
{
   vlVdpSurface *p_surf = vlGetDataHTAB(surface);
   if (!p_surf)
      return NULL;

   mtx_lock(&p_surf->device->mutex);
   if (p_surf->video_buffer == NULL) {
      struct pipe_context *pipe = p_surf->device->context;

      /* try to create a video buffer if we don't already have one */
      p_surf->video_buffer = pipe->create_video_buffer(pipe, &p_surf->templat);
   }
   mtx_unlock(&p_surf->device->mutex);

   return p_surf->video_buffer;
}

VdpStatus vlVdpVideoSurfaceDMABuf(VdpVideoSurface surface,
                                  VdpVideoSurfacePlane plane,
                                  struct VdpSurfaceDMABufDesc *result)
{
   vlVdpSurface *p_surf = vlGetDataHTAB(surface);

   struct pipe_screen *pscreen;
   struct winsys_handle whandle;

   struct pipe_surface *surf;

   if (!p_surf)
      return VDP_STATUS_INVALID_HANDLE;

   if (plane > 3)
      return VDP_STATUS_INVALID_VALUE;

   if (!result)
      return VDP_STATUS_INVALID_POINTER;

   memset(result, 0, sizeof(*result));
   result->handle = -1;

   mtx_lock(&p_surf->device->mutex);
   if (p_surf->video_buffer == NULL) {
      struct pipe_context *pipe = p_surf->device->context;

      /* try to create a video buffer if we don't already have one */
      p_surf->video_buffer = pipe->create_video_buffer(pipe, &p_surf->templat);
   }

   /* Check if surface match interop requirements */
   if (p_surf->video_buffer == NULL || !p_surf->video_buffer->interlaced ||
       p_surf->video_buffer->buffer_format != PIPE_FORMAT_NV12) {
      mtx_unlock(&p_surf->device->mutex);
      return VDP_STATUS_NO_IMPLEMENTATION;
   }

   surf = p_surf->video_buffer->get_surfaces(p_surf->video_buffer)[plane];
   if (!surf) {
      mtx_unlock(&p_surf->device->mutex);
      return VDP_STATUS_RESOURCES;
   }

   memset(&whandle, 0, sizeof(struct winsys_handle));
   whandle.type = WINSYS_HANDLE_TYPE_FD;
   whandle.layer = surf->u.tex.first_layer;

   pscreen = surf->texture->screen;
   if (!pscreen->resource_get_handle(pscreen, p_surf->device->context,
                                     surf->texture, &whandle,
                                     PIPE_HANDLE_USAGE_FRAMEBUFFER_WRITE)) {
      mtx_unlock(&p_surf->device->mutex);
      return VDP_STATUS_NO_IMPLEMENTATION;
   }

   mtx_unlock(&p_surf->device->mutex);

   result->handle = whandle.handle;
   result->width = surf->width;
   result->height = surf->height;
   result->offset = whandle.offset;
   result->stride = whandle.stride;

   if (surf->format == PIPE_FORMAT_R8_UNORM)
      result->format = VDP_RGBA_FORMAT_R8;
   else
      result->format = VDP_RGBA_FORMAT_R8G8;

   return VDP_STATUS_OK;
}
