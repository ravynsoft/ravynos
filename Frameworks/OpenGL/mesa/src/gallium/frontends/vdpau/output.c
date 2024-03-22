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

#include <vdpau/vdpau.h>

#include "util/u_debug.h"
#include "util/u_memory.h"
#include "util/u_sampler.h"
#include "util/format/u_format.h"
#include "util/u_surface.h"

#include "vl/vl_csc.h"

#include "frontend/drm_driver.h"

#include "vdpau_private.h"

/**
 * Create a VdpOutputSurface.
 */
VdpStatus
vlVdpOutputSurfaceCreate(VdpDevice device,
                         VdpRGBAFormat rgba_format,
                         uint32_t width, uint32_t height,
                         VdpOutputSurface  *surface)
{
   struct pipe_context *pipe;
   struct pipe_resource res_tmpl, *res;
   struct pipe_sampler_view sv_templ;
   struct pipe_surface surf_templ;

   vlVdpOutputSurface *vlsurface = NULL;

   if (!(width && height))
      return VDP_STATUS_INVALID_SIZE;

   vlVdpDevice *dev = vlGetDataHTAB(device);
   if (!dev)
      return VDP_STATUS_INVALID_HANDLE;

   pipe = dev->context;
   if (!pipe)
      return VDP_STATUS_INVALID_HANDLE;

   vlsurface = CALLOC(1, sizeof(vlVdpOutputSurface));
   if (!vlsurface)
      return VDP_STATUS_RESOURCES;

   DeviceReference(&vlsurface->device, dev);

   memset(&res_tmpl, 0, sizeof(res_tmpl));

   /*
    * The output won't look correctly when this buffer is send to X,
    * if the VDPAU RGB component order doesn't match the X11 one so
    * we only allow the X11 format
    */
   vlsurface->send_to_X = dev->vscreen->color_depth == 24 &&
      rgba_format == VDP_RGBA_FORMAT_B8G8R8A8;

   res_tmpl.target = PIPE_TEXTURE_2D;
   res_tmpl.format = VdpFormatRGBAToPipe(rgba_format);
   res_tmpl.width0 = width;
   res_tmpl.height0 = height;
   res_tmpl.depth0 = 1;
   res_tmpl.array_size = 1;
   res_tmpl.bind = PIPE_BIND_SAMPLER_VIEW | PIPE_BIND_RENDER_TARGET |
                   PIPE_BIND_SHARED | PIPE_BIND_SCANOUT;
   res_tmpl.usage = PIPE_USAGE_DEFAULT;

   mtx_lock(&dev->mutex);

   if (!CheckSurfaceParams(pipe->screen, &res_tmpl))
      goto err_unlock;

   res = pipe->screen->resource_create(pipe->screen, &res_tmpl);
   if (!res)
      goto err_unlock;

   vlVdpDefaultSamplerViewTemplate(&sv_templ, res);
   vlsurface->sampler_view = pipe->create_sampler_view(pipe, res, &sv_templ);
   if (!vlsurface->sampler_view)
      goto err_resource;

   memset(&surf_templ, 0, sizeof(surf_templ));
   surf_templ.format = res->format;
   vlsurface->surface = pipe->create_surface(pipe, res, &surf_templ);
   if (!vlsurface->surface)
      goto err_resource;

   *surface = vlAddDataHTAB(vlsurface);
   if (*surface == 0)
      goto err_resource;

   pipe_resource_reference(&res, NULL);

   if (!vl_compositor_init_state(&vlsurface->cstate, pipe))
      goto err_resource;

   vl_compositor_reset_dirty_area(&vlsurface->dirty_area);
   mtx_unlock(&dev->mutex);

   return VDP_STATUS_OK;

err_resource:
   pipe_sampler_view_reference(&vlsurface->sampler_view, NULL);
   pipe_surface_reference(&vlsurface->surface, NULL);
   pipe_resource_reference(&res, NULL);
err_unlock:
   mtx_unlock(&dev->mutex);
   DeviceReference(&vlsurface->device, NULL);
   FREE(vlsurface);
   return VDP_STATUS_ERROR;
}

/**
 * Destroy a VdpOutputSurface.
 */
VdpStatus
vlVdpOutputSurfaceDestroy(VdpOutputSurface surface)
{
   vlVdpOutputSurface *vlsurface;
   struct pipe_context *pipe;

   vlsurface = vlGetDataHTAB(surface);
   if (!vlsurface)
      return VDP_STATUS_INVALID_HANDLE;

   pipe = vlsurface->device->context;

   mtx_lock(&vlsurface->device->mutex);

   pipe_surface_reference(&vlsurface->surface, NULL);
   pipe_sampler_view_reference(&vlsurface->sampler_view, NULL);
   pipe->screen->fence_reference(pipe->screen, &vlsurface->fence, NULL);
   vl_compositor_cleanup_state(&vlsurface->cstate);
   mtx_unlock(&vlsurface->device->mutex);

   vlRemoveDataHTAB(surface);
   DeviceReference(&vlsurface->device, NULL);
   FREE(vlsurface);

   return VDP_STATUS_OK;
}

/**
 * Retrieve the parameters used to create a VdpOutputSurface.
 */
VdpStatus
vlVdpOutputSurfaceGetParameters(VdpOutputSurface surface,
                                VdpRGBAFormat *rgba_format,
                                uint32_t *width, uint32_t *height)
{
   vlVdpOutputSurface *vlsurface;

   vlsurface = vlGetDataHTAB(surface);
   if (!vlsurface)
      return VDP_STATUS_INVALID_HANDLE;

   *rgba_format = PipeToFormatRGBA(vlsurface->sampler_view->texture->format);
   *width = vlsurface->sampler_view->texture->width0;
   *height = vlsurface->sampler_view->texture->height0;

   return VDP_STATUS_OK;
}

/**
 * Copy image data from a VdpOutputSurface to application memory in the
 * surface's native format.
 */
VdpStatus
vlVdpOutputSurfaceGetBitsNative(VdpOutputSurface surface,
                                VdpRect const *source_rect,
                                void *const *destination_data,
                                uint32_t const *destination_pitches)
{
   vlVdpOutputSurface *vlsurface;
   struct pipe_context *pipe;
   struct pipe_resource *res;
   struct pipe_box box;
   struct pipe_transfer *transfer;
   uint8_t *map;

   vlsurface = vlGetDataHTAB(surface);
   if (!vlsurface)
      return VDP_STATUS_INVALID_HANDLE;

   pipe = vlsurface->device->context;
   if (!pipe)
      return VDP_STATUS_INVALID_HANDLE;

   if (!destination_data || !destination_pitches)
       return VDP_STATUS_INVALID_POINTER;

   mtx_lock(&vlsurface->device->mutex);

   res = vlsurface->sampler_view->texture;
   box = RectToPipeBox(source_rect, res);
   map = pipe->texture_map(pipe, res, 0, PIPE_MAP_READ, &box, &transfer);
   if (!map) {
      mtx_unlock(&vlsurface->device->mutex);
      return VDP_STATUS_RESOURCES;
   }

   util_copy_rect(*destination_data, res->format, *destination_pitches, 0, 0,
                  box.width, box.height, map, transfer->stride, 0, 0);

   pipe_texture_unmap(pipe, transfer);
   mtx_unlock(&vlsurface->device->mutex);

   return VDP_STATUS_OK;
}

/**
 * Copy image data from application memory in the surface's native format to
 * a VdpOutputSurface.
 */
VdpStatus
vlVdpOutputSurfacePutBitsNative(VdpOutputSurface surface,
                                void const *const *source_data,
                                uint32_t const *source_pitches,
                                VdpRect const *destination_rect)
{
   vlVdpOutputSurface *vlsurface;
   struct pipe_box dst_box;
   struct pipe_context *pipe;

   vlsurface = vlGetDataHTAB(surface);
   if (!vlsurface)
      return VDP_STATUS_INVALID_HANDLE;

   pipe = vlsurface->device->context;
   if (!pipe)
      return VDP_STATUS_INVALID_HANDLE;

   if (!source_data || !source_pitches)
       return VDP_STATUS_INVALID_POINTER;

   mtx_lock(&vlsurface->device->mutex);

   dst_box = RectToPipeBox(destination_rect, vlsurface->sampler_view->texture);

   /* Check for a no-op. (application bug?) */
   if (!dst_box.width || !dst_box.height) {
      mtx_unlock(&vlsurface->device->mutex);
      return VDP_STATUS_OK;
   }

   pipe->texture_subdata(pipe, vlsurface->sampler_view->texture, 0,
                         PIPE_MAP_WRITE, &dst_box, *source_data,
                         *source_pitches, 0);
   mtx_unlock(&vlsurface->device->mutex);

   return VDP_STATUS_OK;
}

/**
 * Copy image data from application memory in a specific indexed format to
 * a VdpOutputSurface.
 */
VdpStatus
vlVdpOutputSurfacePutBitsIndexed(VdpOutputSurface surface,
                                 VdpIndexedFormat source_indexed_format,
                                 void const *const *source_data,
                                 uint32_t const *source_pitch,
                                 VdpRect const *destination_rect,
                                 VdpColorTableFormat color_table_format,
                                 void const *color_table)
{
   vlVdpOutputSurface *vlsurface;
   struct pipe_context *context;
   struct vl_compositor *compositor;
   struct vl_compositor_state *cstate;

   enum pipe_format index_format;
   enum pipe_format colortbl_format;

   struct pipe_resource *res, res_tmpl;
   struct pipe_sampler_view sv_tmpl;
   struct pipe_sampler_view *sv_idx = NULL, *sv_tbl = NULL;

   struct pipe_box box;
   struct u_rect dst_rect;

   vlsurface = vlGetDataHTAB(surface);
   if (!vlsurface)
      return VDP_STATUS_INVALID_HANDLE;

   context = vlsurface->device->context;
   compositor = &vlsurface->device->compositor;
   cstate = &vlsurface->cstate;

   index_format = FormatIndexedToPipe(source_indexed_format);
   if (index_format == PIPE_FORMAT_NONE)
       return VDP_STATUS_INVALID_INDEXED_FORMAT;

   if (!source_data || !source_pitch)
       return VDP_STATUS_INVALID_POINTER;

   colortbl_format = FormatColorTableToPipe(color_table_format);
   if (colortbl_format == PIPE_FORMAT_NONE)
       return VDP_STATUS_INVALID_COLOR_TABLE_FORMAT;

   if (!color_table)
       return VDP_STATUS_INVALID_POINTER;

   memset(&res_tmpl, 0, sizeof(res_tmpl));
   res_tmpl.target = PIPE_TEXTURE_2D;
   res_tmpl.format = index_format;

   if (destination_rect) {
      if (destination_rect->x1 > destination_rect->x0 &&
          destination_rect->y1 > destination_rect->y0) {
         res_tmpl.width0 = destination_rect->x1 - destination_rect->x0;
         res_tmpl.height0 = destination_rect->y1 - destination_rect->y0;
      }
   } else {
      res_tmpl.width0 = vlsurface->surface->texture->width0;
      res_tmpl.height0 = vlsurface->surface->texture->height0;
   }
   res_tmpl.depth0 = 1;
   res_tmpl.array_size = 1;
   res_tmpl.usage = PIPE_USAGE_STAGING;
   res_tmpl.bind = PIPE_BIND_SAMPLER_VIEW;

   mtx_lock(&vlsurface->device->mutex);

   if (!CheckSurfaceParams(context->screen, &res_tmpl))
      goto error_resource;

   res = context->screen->resource_create(context->screen, &res_tmpl);
   if (!res)
      goto error_resource;

   box.x = box.y = box.z = 0;
   box.width = res->width0;
   box.height = res->height0;
   box.depth = res->depth0;

   context->texture_subdata(context, res, 0, PIPE_MAP_WRITE, &box,
                            source_data[0], source_pitch[0],
                            source_pitch[0] * res->height0);

   memset(&sv_tmpl, 0, sizeof(sv_tmpl));
   u_sampler_view_default_template(&sv_tmpl, res, res->format);

   sv_idx = context->create_sampler_view(context, res, &sv_tmpl);
   pipe_resource_reference(&res, NULL);

   if (!sv_idx)
      goto error_resource;

   memset(&res_tmpl, 0, sizeof(res_tmpl));
   res_tmpl.target = PIPE_TEXTURE_1D;
   res_tmpl.format = colortbl_format;
   res_tmpl.width0 = 1 << util_format_get_component_bits(
      index_format, UTIL_FORMAT_COLORSPACE_RGB, 0);
   res_tmpl.height0 = 1;
   res_tmpl.depth0 = 1;
   res_tmpl.array_size = 1;
   res_tmpl.usage = PIPE_USAGE_STAGING;
   res_tmpl.bind = PIPE_BIND_SAMPLER_VIEW;

   res = context->screen->resource_create(context->screen, &res_tmpl);
   if (!res)
      goto error_resource;

   box.x = box.y = box.z = 0;
   box.width = res->width0;
   box.height = res->height0;
   box.depth = res->depth0;

   context->texture_subdata(context, res, 0, PIPE_MAP_WRITE, &box, color_table,
                            util_format_get_stride(colortbl_format, res->width0), 0);

   memset(&sv_tmpl, 0, sizeof(sv_tmpl));
   u_sampler_view_default_template(&sv_tmpl, res, res->format);

   sv_tbl = context->create_sampler_view(context, res, &sv_tmpl);
   pipe_resource_reference(&res, NULL);

   if (!sv_tbl)
      goto error_resource;

   vl_compositor_clear_layers(cstate);
   vl_compositor_set_palette_layer(cstate, compositor, 0, sv_idx, sv_tbl, NULL, NULL, false);
   vl_compositor_set_layer_dst_area(cstate, 0, RectToPipe(destination_rect, &dst_rect));
   vl_compositor_render(cstate, compositor, vlsurface->surface, &vlsurface->dirty_area, false);

   pipe_sampler_view_reference(&sv_idx, NULL);
   pipe_sampler_view_reference(&sv_tbl, NULL);
   mtx_unlock(&vlsurface->device->mutex);

   return VDP_STATUS_OK;

error_resource:
   pipe_sampler_view_reference(&sv_idx, NULL);
   pipe_sampler_view_reference(&sv_tbl, NULL);
   mtx_unlock(&vlsurface->device->mutex);
   return VDP_STATUS_RESOURCES;
}

/**
 * Copy image data from application memory in a specific YCbCr format to
 * a VdpOutputSurface.
 */
VdpStatus
vlVdpOutputSurfacePutBitsYCbCr(VdpOutputSurface surface,
                               VdpYCbCrFormat source_ycbcr_format,
                               void const *const *source_data,
                               uint32_t const *source_pitches,
                               VdpRect const *destination_rect,
                               VdpCSCMatrix const *csc_matrix)
{
   vlVdpOutputSurface *vlsurface;
   struct vl_compositor *compositor;
   struct vl_compositor_state *cstate;

   struct pipe_context *pipe;
   enum pipe_format format;
   struct pipe_video_buffer vtmpl, *vbuffer;
   struct u_rect dst_rect;
   struct pipe_sampler_view **sampler_views;

   unsigned i;

   vlsurface = vlGetDataHTAB(surface);
   if (!vlsurface)
      return VDP_STATUS_INVALID_HANDLE;


   pipe = vlsurface->device->context;
   compositor = &vlsurface->device->compositor;
   cstate = &vlsurface->cstate;

   format = FormatYCBCRToPipe(source_ycbcr_format);
   if (format == PIPE_FORMAT_NONE)
       return VDP_STATUS_INVALID_Y_CB_CR_FORMAT;

   if (!source_data || !source_pitches)
       return VDP_STATUS_INVALID_POINTER;

   mtx_lock(&vlsurface->device->mutex);
   memset(&vtmpl, 0, sizeof(vtmpl));
   vtmpl.buffer_format = format;

   if (destination_rect) {
      if (destination_rect->x1 > destination_rect->x0 &&
          destination_rect->y1 > destination_rect->y0) {
         vtmpl.width = destination_rect->x1 - destination_rect->x0;
         vtmpl.height = destination_rect->y1 - destination_rect->y0;
      }
   } else {
      vtmpl.width = vlsurface->surface->texture->width0;
      vtmpl.height = vlsurface->surface->texture->height0;
   }

   vbuffer = pipe->create_video_buffer(pipe, &vtmpl);
   if (!vbuffer) {
      mtx_unlock(&vlsurface->device->mutex);
      return VDP_STATUS_RESOURCES;
   }

   sampler_views = vbuffer->get_sampler_view_planes(vbuffer);
   if (!sampler_views) {
      vbuffer->destroy(vbuffer);
      mtx_unlock(&vlsurface->device->mutex);
      return VDP_STATUS_RESOURCES;
   }

   for (i = 0; i < 3; ++i) {
      struct pipe_sampler_view *sv = sampler_views[i];
      if (!sv) continue;

      struct pipe_box dst_box = {
         0, 0, 0,
         sv->texture->width0, sv->texture->height0, 1
      };

      pipe->texture_subdata(pipe, sv->texture, 0, PIPE_MAP_WRITE, &dst_box,
                            source_data[i], source_pitches[i], 0);
   }

   if (!csc_matrix) {
      vl_csc_matrix csc;
      vl_csc_get_matrix(VL_CSC_COLOR_STANDARD_BT_601, NULL, 1, &csc);
      if (!vl_compositor_set_csc_matrix(cstate, (const vl_csc_matrix*)&csc, 1.0f, 0.0f))
         goto err_csc_matrix;
   } else {
      if (!vl_compositor_set_csc_matrix(cstate, csc_matrix, 1.0f, 0.0f))
         goto err_csc_matrix;
   }

   vl_compositor_clear_layers(cstate);
   vl_compositor_set_buffer_layer(cstate, compositor, 0, vbuffer, NULL, NULL, VL_COMPOSITOR_WEAVE);
   vl_compositor_set_layer_dst_area(cstate, 0, RectToPipe(destination_rect, &dst_rect));
   vl_compositor_render(cstate, compositor, vlsurface->surface, &vlsurface->dirty_area, false);

   vbuffer->destroy(vbuffer);
   mtx_unlock(&vlsurface->device->mutex);

   return VDP_STATUS_OK;
err_csc_matrix:
   vbuffer->destroy(vbuffer);
   mtx_unlock(&vlsurface->device->mutex);
   return VDP_STATUS_ERROR;
}

static unsigned
BlendFactorToPipe(VdpOutputSurfaceRenderBlendFactor factor)
{
   switch (factor) {
   case VDP_OUTPUT_SURFACE_RENDER_BLEND_FACTOR_ZERO:
      return PIPE_BLENDFACTOR_ZERO;
   case VDP_OUTPUT_SURFACE_RENDER_BLEND_FACTOR_ONE:
      return PIPE_BLENDFACTOR_ONE;
   case VDP_OUTPUT_SURFACE_RENDER_BLEND_FACTOR_SRC_COLOR:
      return PIPE_BLENDFACTOR_SRC_COLOR;
   case VDP_OUTPUT_SURFACE_RENDER_BLEND_FACTOR_ONE_MINUS_SRC_COLOR:
      return PIPE_BLENDFACTOR_INV_SRC_COLOR;
   case VDP_OUTPUT_SURFACE_RENDER_BLEND_FACTOR_SRC_ALPHA:
      return PIPE_BLENDFACTOR_SRC_ALPHA;
   case VDP_OUTPUT_SURFACE_RENDER_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA:
      return PIPE_BLENDFACTOR_INV_SRC_ALPHA;
   case VDP_OUTPUT_SURFACE_RENDER_BLEND_FACTOR_DST_ALPHA:
      return PIPE_BLENDFACTOR_DST_ALPHA;
   case VDP_OUTPUT_SURFACE_RENDER_BLEND_FACTOR_ONE_MINUS_DST_ALPHA:
      return PIPE_BLENDFACTOR_INV_DST_ALPHA;
   case VDP_OUTPUT_SURFACE_RENDER_BLEND_FACTOR_DST_COLOR:
      return PIPE_BLENDFACTOR_DST_COLOR;
   case VDP_OUTPUT_SURFACE_RENDER_BLEND_FACTOR_ONE_MINUS_DST_COLOR:
      return PIPE_BLENDFACTOR_INV_DST_COLOR;
   case VDP_OUTPUT_SURFACE_RENDER_BLEND_FACTOR_SRC_ALPHA_SATURATE:
      return PIPE_BLENDFACTOR_SRC_ALPHA_SATURATE;
   case VDP_OUTPUT_SURFACE_RENDER_BLEND_FACTOR_CONSTANT_COLOR:
      return PIPE_BLENDFACTOR_CONST_COLOR;
   case VDP_OUTPUT_SURFACE_RENDER_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR:
      return PIPE_BLENDFACTOR_INV_CONST_COLOR;
   case VDP_OUTPUT_SURFACE_RENDER_BLEND_FACTOR_CONSTANT_ALPHA:
      return PIPE_BLENDFACTOR_CONST_ALPHA;
   case VDP_OUTPUT_SURFACE_RENDER_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA:
      return PIPE_BLENDFACTOR_INV_CONST_ALPHA;
   default:
      assert(0);
      return PIPE_BLENDFACTOR_ONE;
   }
}

static unsigned
BlendEquationToPipe(VdpOutputSurfaceRenderBlendEquation equation)
{
   switch (equation) {
   case VDP_OUTPUT_SURFACE_RENDER_BLEND_EQUATION_SUBTRACT:
      return PIPE_BLEND_SUBTRACT;
   case VDP_OUTPUT_SURFACE_RENDER_BLEND_EQUATION_REVERSE_SUBTRACT:
      return PIPE_BLEND_REVERSE_SUBTRACT;
   case VDP_OUTPUT_SURFACE_RENDER_BLEND_EQUATION_ADD:
      return PIPE_BLEND_ADD;
   case VDP_OUTPUT_SURFACE_RENDER_BLEND_EQUATION_MIN:
      return PIPE_BLEND_MIN;
   case VDP_OUTPUT_SURFACE_RENDER_BLEND_EQUATION_MAX:
      return PIPE_BLEND_MAX;
   default:
      assert(0);
      return PIPE_BLEND_ADD;
   }
}

static void *
BlenderToPipe(struct pipe_context *context,
              VdpOutputSurfaceRenderBlendState const *blend_state)
{
   struct pipe_blend_state blend;

   memset(&blend, 0, sizeof blend);
   blend.independent_blend_enable = 0;

   if (blend_state) {
      blend.rt[0].blend_enable = 1;
      blend.rt[0].rgb_src_factor = BlendFactorToPipe(blend_state->blend_factor_source_color);
      blend.rt[0].rgb_dst_factor = BlendFactorToPipe(blend_state->blend_factor_destination_color);
      blend.rt[0].alpha_src_factor = BlendFactorToPipe(blend_state->blend_factor_source_alpha);
      blend.rt[0].alpha_dst_factor = BlendFactorToPipe(blend_state->blend_factor_destination_alpha);
      blend.rt[0].rgb_func = BlendEquationToPipe(blend_state->blend_equation_color);
      blend.rt[0].alpha_func = BlendEquationToPipe(blend_state->blend_equation_alpha);
   } else {
      blend.rt[0].blend_enable = 0;
   }

   blend.logicop_enable = 0;
   blend.logicop_func = PIPE_LOGICOP_CLEAR;
   blend.rt[0].colormask = PIPE_MASK_RGBA;
   blend.dither = 0;

   return context->create_blend_state(context, &blend);
}

static struct vertex4f *
ColorsToPipe(VdpColor const *colors, uint32_t flags, struct vertex4f result[4])
{
   unsigned i;
   struct vertex4f *dst = result;

   if (!colors)
      return NULL;

   for (i = 0; i < 4; ++i) {
      dst->x = colors->red;
      dst->y = colors->green;
      dst->z = colors->blue;
      dst->w = colors->alpha;

      ++dst;
      if (flags & VDP_OUTPUT_SURFACE_RENDER_COLOR_PER_VERTEX)
         ++colors;
   }
   return result;
}

/**
 * Composite a sub-rectangle of a VdpOutputSurface into a sub-rectangle of
 * another VdpOutputSurface; Output Surface object VdpOutputSurface.
 */
VdpStatus
vlVdpOutputSurfaceRenderOutputSurface(VdpOutputSurface destination_surface,
                                      VdpRect const *destination_rect,
                                      VdpOutputSurface source_surface,
                                      VdpRect const *source_rect,
                                      VdpColor const *colors,
                                      VdpOutputSurfaceRenderBlendState const *blend_state,
                                      uint32_t flags)
{
   vlVdpOutputSurface *dst_vlsurface;

   struct pipe_context *context;
   struct pipe_sampler_view *src_sv;
   struct vl_compositor *compositor;
   struct vl_compositor_state *cstate;

   struct u_rect src_rect, dst_rect;

   struct vertex4f vlcolors[4];
   void *blend;

   dst_vlsurface = vlGetDataHTAB(destination_surface);
   if (!dst_vlsurface)
      return VDP_STATUS_INVALID_HANDLE;

   if (source_surface == VDP_INVALID_HANDLE) {
      src_sv = dst_vlsurface->device->dummy_sv;

   } else {
      vlVdpOutputSurface *src_vlsurface = vlGetDataHTAB(source_surface);
      if (!src_vlsurface)
         return VDP_STATUS_INVALID_HANDLE;

      if (dst_vlsurface->device != src_vlsurface->device)
         return VDP_STATUS_HANDLE_DEVICE_MISMATCH;

      src_sv = src_vlsurface->sampler_view;
   }

   mtx_lock(&dst_vlsurface->device->mutex);

   context = dst_vlsurface->device->context;
   compositor = &dst_vlsurface->device->compositor;
   cstate = &dst_vlsurface->cstate;

   blend = BlenderToPipe(context, blend_state);

   vl_compositor_clear_layers(cstate);
   vl_compositor_set_layer_blend(cstate, 0, blend, false);
   vl_compositor_set_rgba_layer(cstate, compositor, 0, src_sv,
                                RectToPipe(source_rect, &src_rect), NULL,
                                ColorsToPipe(colors, flags, vlcolors));
   STATIC_ASSERT(VL_COMPOSITOR_ROTATE_0 == VDP_OUTPUT_SURFACE_RENDER_ROTATE_0);
   STATIC_ASSERT(VL_COMPOSITOR_ROTATE_90 == VDP_OUTPUT_SURFACE_RENDER_ROTATE_90);
   STATIC_ASSERT(VL_COMPOSITOR_ROTATE_180 == VDP_OUTPUT_SURFACE_RENDER_ROTATE_180);
   STATIC_ASSERT(VL_COMPOSITOR_ROTATE_270 == VDP_OUTPUT_SURFACE_RENDER_ROTATE_270);
   vl_compositor_set_layer_rotation(cstate, 0, flags & 3);
   vl_compositor_set_layer_dst_area(cstate, 0, RectToPipe(destination_rect, &dst_rect));
   vl_compositor_render(cstate, compositor, dst_vlsurface->surface, &dst_vlsurface->dirty_area, false);

   context->delete_blend_state(context, blend);
   mtx_unlock(&dst_vlsurface->device->mutex);

   return VDP_STATUS_OK;
}

/**
 * Composite a sub-rectangle of a VdpBitmapSurface into a sub-rectangle of
 * a VdpOutputSurface; Output Surface object VdpOutputSurface.
 */
VdpStatus
vlVdpOutputSurfaceRenderBitmapSurface(VdpOutputSurface destination_surface,
                                      VdpRect const *destination_rect,
                                      VdpBitmapSurface source_surface,
                                      VdpRect const *source_rect,
                                      VdpColor const *colors,
                                      VdpOutputSurfaceRenderBlendState const *blend_state,
                                      uint32_t flags)
{
   vlVdpOutputSurface *dst_vlsurface;

   struct pipe_context *context;
   struct pipe_sampler_view *src_sv;
   struct vl_compositor *compositor;
   struct vl_compositor_state *cstate;

   struct u_rect src_rect, dst_rect;

   struct vertex4f vlcolors[4];
   void *blend;

   dst_vlsurface = vlGetDataHTAB(destination_surface);
   if (!dst_vlsurface)
      return VDP_STATUS_INVALID_HANDLE;

   if (source_surface == VDP_INVALID_HANDLE) {
      src_sv = dst_vlsurface->device->dummy_sv;

   } else {
      vlVdpBitmapSurface *src_vlsurface = vlGetDataHTAB(source_surface);
      if (!src_vlsurface)
         return VDP_STATUS_INVALID_HANDLE;

      if (dst_vlsurface->device != src_vlsurface->device)
         return VDP_STATUS_HANDLE_DEVICE_MISMATCH;

      src_sv = src_vlsurface->sampler_view;
   }

   context = dst_vlsurface->device->context;
   compositor = &dst_vlsurface->device->compositor;
   cstate = &dst_vlsurface->cstate;

   mtx_lock(&dst_vlsurface->device->mutex);

   blend = BlenderToPipe(context, blend_state);

   vl_compositor_clear_layers(cstate);
   vl_compositor_set_layer_blend(cstate, 0, blend, false);
   vl_compositor_set_rgba_layer(cstate, compositor, 0, src_sv,
                                RectToPipe(source_rect, &src_rect), NULL,
                                ColorsToPipe(colors, flags, vlcolors));
   vl_compositor_set_layer_rotation(cstate, 0, flags & 3);
   vl_compositor_set_layer_dst_area(cstate, 0, RectToPipe(destination_rect, &dst_rect));
   vl_compositor_render(cstate, compositor, dst_vlsurface->surface, &dst_vlsurface->dirty_area, false);

   context->delete_blend_state(context, blend);
   mtx_unlock(&dst_vlsurface->device->mutex);

   return VDP_STATUS_OK;
}

struct pipe_resource *vlVdpOutputSurfaceGallium(VdpOutputSurface surface)
{
   vlVdpOutputSurface *vlsurface;

   vlsurface = vlGetDataHTAB(surface);
   if (!vlsurface || !vlsurface->surface)
      return NULL;

   mtx_lock(&vlsurface->device->mutex);
   vlsurface->device->context->flush(vlsurface->device->context, NULL, 0);
   mtx_unlock(&vlsurface->device->mutex);

   return vlsurface->surface->texture;
}

VdpStatus vlVdpOutputSurfaceDMABuf(VdpOutputSurface surface,
                                   struct VdpSurfaceDMABufDesc *result)
{
   vlVdpOutputSurface *vlsurface;
   struct pipe_screen *pscreen;
   struct winsys_handle whandle;

   memset(result, 0, sizeof(*result));
   result->handle = -1;

   vlsurface = vlGetDataHTAB(surface);
   if (!vlsurface || !vlsurface->surface)
      return VDP_STATUS_INVALID_HANDLE;

   mtx_lock(&vlsurface->device->mutex);
   vlsurface->device->context->flush(vlsurface->device->context, NULL, 0);

   memset(&whandle, 0, sizeof(struct winsys_handle));
   whandle.type = WINSYS_HANDLE_TYPE_FD;

   pscreen = vlsurface->surface->texture->screen;
   if (!pscreen->resource_get_handle(pscreen, vlsurface->device->context,
                                     vlsurface->surface->texture, &whandle,
                                     PIPE_HANDLE_USAGE_FRAMEBUFFER_WRITE)) {
      mtx_unlock(&vlsurface->device->mutex);
      return VDP_STATUS_NO_IMPLEMENTATION;
   }

   mtx_unlock(&vlsurface->device->mutex);

   result->handle = whandle.handle;
   result->width = vlsurface->surface->width;
   result->height = vlsurface->surface->height;
   result->offset = whandle.offset;
   result->stride = whandle.stride;
   result->format = PipeToFormatRGBA(vlsurface->surface->format);

   return VDP_STATUS_OK;
}
