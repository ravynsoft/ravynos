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

#include <vdpau/vdpau.h>

#include "util/u_memory.h"
#include "util/u_sampler.h"

#include "vdpau_private.h"

/**
 * Create a VdpBitmapSurface.
 */
VdpStatus
vlVdpBitmapSurfaceCreate(VdpDevice device,
                         VdpRGBAFormat rgba_format,
                         uint32_t width, uint32_t height,
                         VdpBool frequently_accessed,
                         VdpBitmapSurface *surface)
{
   struct pipe_context *pipe;
   struct pipe_resource res_tmpl, *res;
   struct pipe_sampler_view sv_templ;
   VdpStatus ret;

   vlVdpBitmapSurface *vlsurface = NULL;

   if (!(width && height))
      return VDP_STATUS_INVALID_SIZE;

   vlVdpDevice *dev = vlGetDataHTAB(device);
   if (!dev)
      return VDP_STATUS_INVALID_HANDLE;

   pipe = dev->context;
   if (!pipe)
      return VDP_STATUS_INVALID_HANDLE;

   if (!surface)
      return VDP_STATUS_INVALID_POINTER;

   vlsurface = CALLOC(1, sizeof(vlVdpBitmapSurface));
   if (!vlsurface)
      return VDP_STATUS_RESOURCES;

   DeviceReference(&vlsurface->device, dev);

   memset(&res_tmpl, 0, sizeof(res_tmpl));
   res_tmpl.target = PIPE_TEXTURE_2D;
   res_tmpl.format = VdpFormatRGBAToPipe(rgba_format);
   res_tmpl.width0 = width;
   res_tmpl.height0 = height;
   res_tmpl.depth0 = 1;
   res_tmpl.array_size = 1;
   res_tmpl.bind = PIPE_BIND_SAMPLER_VIEW | PIPE_BIND_RENDER_TARGET;
   res_tmpl.usage = frequently_accessed ? PIPE_USAGE_DYNAMIC : PIPE_USAGE_DEFAULT;

   mtx_lock(&dev->mutex);

   if (!CheckSurfaceParams(pipe->screen, &res_tmpl)) {
      ret = VDP_STATUS_RESOURCES;
      goto err_unlock;
   }

   res = pipe->screen->resource_create(pipe->screen, &res_tmpl);
   if (!res) {
      ret = VDP_STATUS_RESOURCES;
      goto err_unlock;
   }

   vlVdpDefaultSamplerViewTemplate(&sv_templ, res);
   vlsurface->sampler_view = pipe->create_sampler_view(pipe, res, &sv_templ);

   pipe_resource_reference(&res, NULL);

   if (!vlsurface->sampler_view) {
      ret = VDP_STATUS_RESOURCES;
      goto err_unlock;
   }

   mtx_unlock(&dev->mutex);

   *surface = vlAddDataHTAB(vlsurface);
   if (*surface == 0) {
      mtx_lock(&dev->mutex);
      ret = VDP_STATUS_ERROR;
      goto err_sampler;
   }

   return VDP_STATUS_OK;

err_sampler:
   pipe_sampler_view_reference(&vlsurface->sampler_view, NULL);
err_unlock:
   mtx_unlock(&dev->mutex);
   DeviceReference(&vlsurface->device, NULL);
   FREE(vlsurface);
   return ret;
}

/**
 * Destroy a VdpBitmapSurface.
 */
VdpStatus
vlVdpBitmapSurfaceDestroy(VdpBitmapSurface surface)
{
   vlVdpBitmapSurface *vlsurface;

   vlsurface = vlGetDataHTAB(surface);
   if (!vlsurface)
      return VDP_STATUS_INVALID_HANDLE;

   mtx_lock(&vlsurface->device->mutex);
   pipe_sampler_view_reference(&vlsurface->sampler_view, NULL);
   mtx_unlock(&vlsurface->device->mutex);

   vlRemoveDataHTAB(surface);
   DeviceReference(&vlsurface->device, NULL);
   FREE(vlsurface);

   return VDP_STATUS_OK;
}

/**
 * Retrieve the parameters used to create a VdpBitmapSurface.
 */
VdpStatus
vlVdpBitmapSurfaceGetParameters(VdpBitmapSurface surface,
                                VdpRGBAFormat *rgba_format,
                                uint32_t *width, uint32_t *height,
                                VdpBool *frequently_accessed)
{
   vlVdpBitmapSurface *vlsurface;
   struct pipe_resource *res;

   vlsurface = vlGetDataHTAB(surface);
   if (!vlsurface)
      return VDP_STATUS_INVALID_HANDLE;

   if (!(rgba_format && width && height && frequently_accessed))
      return VDP_STATUS_INVALID_POINTER;

   res = vlsurface->sampler_view->texture;
   *rgba_format = PipeToFormatRGBA(res->format);
   *width = res->width0;
   *height = res->height0;
   *frequently_accessed = res->usage == PIPE_USAGE_DYNAMIC;

   return VDP_STATUS_OK;
}

/**
 * Copy image data from application memory in the surface's native format to
 * a VdpBitmapSurface.
 */
VdpStatus
vlVdpBitmapSurfacePutBitsNative(VdpBitmapSurface surface,
                                void const *const *source_data,
                                uint32_t const *source_pitches,
                                VdpRect const *destination_rect)
{
   vlVdpBitmapSurface *vlsurface;
   struct pipe_box dst_box;
   struct pipe_context *pipe;

   vlsurface = vlGetDataHTAB(surface);
   if (!vlsurface)
      return VDP_STATUS_INVALID_HANDLE;

   if (!(source_data && source_pitches))
      return VDP_STATUS_INVALID_POINTER;

   pipe = vlsurface->device->context;

   mtx_lock(&vlsurface->device->mutex);

   dst_box = RectToPipeBox(destination_rect, vlsurface->sampler_view->texture);
   pipe->texture_subdata(pipe, vlsurface->sampler_view->texture, 0,
                         PIPE_MAP_WRITE, &dst_box, *source_data,
                         *source_pitches, 0);

   mtx_unlock(&vlsurface->device->mutex);

   return VDP_STATUS_OK;
}
