/**************************************************************************
 *
 * Copyright 2015 Advanced Micro Devices, Inc.
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
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#include "util/u_handle_table.h"
#include "util/u_memory.h"
#include "util/u_compute.h"

#include "vl/vl_defines.h"
#include "vl/vl_video_buffer.h"
#include "vl/vl_deint_filter.h"
#include "vl/vl_winsys.h"

#include "va_private.h"

static const VARectangle *
vlVaRegionDefault(const VARectangle *region, vlVaSurface *surf,
		  VARectangle *def)
{
   if (region)
      return region;

   def->x = 0;
   def->y = 0;
   def->width = surf->templat.width;
   def->height = surf->templat.height;

   return def;
}

static VAStatus
vlVaPostProcCompositor(vlVaDriver *drv, vlVaContext *context,
                       const VARectangle *src_region,
                       const VARectangle *dst_region,
                       struct pipe_video_buffer *src,
                       struct pipe_video_buffer *dst,
                       enum vl_compositor_deinterlace deinterlace)
{
   struct pipe_surface **surfaces;
   struct u_rect src_rect;
   struct u_rect dst_rect;

   surfaces = dst->get_surfaces(dst);
   if (!surfaces || !surfaces[0])
      return VA_STATUS_ERROR_INVALID_SURFACE;

   src_rect.x0 = src_region->x;
   src_rect.y0 = src_region->y;
   src_rect.x1 = src_region->x + src_region->width;
   src_rect.y1 = src_region->y + src_region->height;

   dst_rect.x0 = dst_region->x;
   dst_rect.y0 = dst_region->y;
   dst_rect.x1 = dst_region->x + dst_region->width;
   dst_rect.y1 = dst_region->y + dst_region->height;

   vl_compositor_clear_layers(&drv->cstate);
   vl_compositor_set_buffer_layer(&drv->cstate, &drv->compositor, 0, src,
				  &src_rect, NULL, deinterlace);
   vl_compositor_set_layer_dst_area(&drv->cstate, 0, &dst_rect);
   vl_compositor_render(&drv->cstate, &drv->compositor, surfaces[0], NULL, false);

   drv->pipe->flush(drv->pipe, NULL, 0);
   return VA_STATUS_SUCCESS;
}

static void vlVaGetBox(struct pipe_video_buffer *buf, unsigned idx,
                       struct pipe_box *box, const VARectangle *region)
{
   unsigned plane = buf->interlaced ? idx / 2: idx;
   unsigned x, y, width, height;

   x = abs(region->x);
   y = abs(region->y);
   width = region->width;
   height = region->height;

   vl_video_buffer_adjust_size(&x, &y, plane,
                               pipe_format_to_chroma_format(buf->buffer_format),
                               buf->interlaced);
   vl_video_buffer_adjust_size(&width, &height, plane,
                               pipe_format_to_chroma_format(buf->buffer_format),
                               buf->interlaced);

   box->x = region->x < 0 ? -x : x;
   box->y = region->y < 0 ? -y : y;
   box->width = width;
   box->height = height;
}

static bool vlVaGetFullRange(vlVaSurface *surface, uint8_t va_range)
{
   if (va_range != VA_SOURCE_RANGE_UNKNOWN)
      return va_range == VA_SOURCE_RANGE_FULL;

   /* Assume limited for YUV, full for RGB */
   return !util_format_is_yuv(surface->buffer->buffer_format);
}

static unsigned vlVaGetChromaLocation(unsigned va_chroma_location,
                                      enum pipe_format format)
{
   unsigned ret = VL_COMPOSITOR_LOCATION_NONE;

   if (util_format_get_plane_height(format, 1, 4) != 4) {
      /* Bits 0-1 */
      switch (va_chroma_location & 3) {
      case VA_CHROMA_SITING_VERTICAL_TOP:
         ret |= VL_COMPOSITOR_LOCATION_VERTICAL_TOP;
         break;
      case VA_CHROMA_SITING_VERTICAL_BOTTOM:
         ret |= VL_COMPOSITOR_LOCATION_VERTICAL_BOTTOM;
         break;
      case VA_CHROMA_SITING_VERTICAL_CENTER:
      default:
         ret |= VL_COMPOSITOR_LOCATION_VERTICAL_CENTER;
         break;
      }
   }

   if (util_format_is_subsampled_422(format) ||
       util_format_get_plane_width(format, 1, 4) != 4) {
      /* Bits 2-3 */
      switch (va_chroma_location & 12) {
      case VA_CHROMA_SITING_HORIZONTAL_CENTER:
         ret |= VL_COMPOSITOR_LOCATION_HORIZONTAL_CENTER;
         break;
      case VA_CHROMA_SITING_HORIZONTAL_LEFT:
      default:
         ret |= VL_COMPOSITOR_LOCATION_HORIZONTAL_LEFT;
         break;
      }
   }

   return ret;
}

static void vlVaSetProcParameters(vlVaDriver *drv,
                                  vlVaSurface *src,
                                  vlVaSurface *dst,
                                  VAProcPipelineParameterBuffer *param)
{
   enum VL_CSC_COLOR_STANDARD color_standard;
   bool src_yuv = util_format_is_yuv(src->buffer->buffer_format);
   bool dst_yuv = util_format_is_yuv(dst->buffer->buffer_format);

   if (src_yuv == dst_yuv) {
      color_standard = VL_CSC_COLOR_STANDARD_IDENTITY;
   } else if (src_yuv) {
      switch (param->surface_color_standard) {
      case VAProcColorStandardBT601:
         color_standard = VL_CSC_COLOR_STANDARD_BT_601;
         break;
      case VAProcColorStandardBT709:
      default:
         color_standard = src->full_range ?
            VL_CSC_COLOR_STANDARD_BT_709_FULL :
            VL_CSC_COLOR_STANDARD_BT_709;
         break;
      }
   } else {
      color_standard = VL_CSC_COLOR_STANDARD_BT_709_REV;
   }

   vl_csc_get_matrix(color_standard, NULL, dst->full_range, &drv->csc);
   vl_compositor_set_csc_matrix(&drv->cstate, &drv->csc, 1.0f, 0.0f);

   if (src_yuv)
      drv->cstate.chroma_location =
         vlVaGetChromaLocation(param->input_color_properties.chroma_sample_location,
                               src->buffer->buffer_format);
   else if (dst_yuv)
      drv->cstate.chroma_location =
         vlVaGetChromaLocation(param->output_color_properties.chroma_sample_location,
                               dst->buffer->buffer_format);
}

static VAStatus vlVaVidEngineBlit(vlVaDriver *drv, vlVaContext *context,
                                 const VARectangle *src_region,
                                 const VARectangle *dst_region,
                                 struct pipe_video_buffer *src,
                                 struct pipe_video_buffer *dst,
                                 enum vl_compositor_deinterlace deinterlace,
                                 VAProcPipelineParameterBuffer* param)
{
   if (deinterlace != VL_COMPOSITOR_NONE)
      return VA_STATUS_ERROR_UNIMPLEMENTED;

   if (!drv->pipe->screen->is_video_format_supported(drv->pipe->screen,
                                                     src->buffer_format,
                                                     PIPE_VIDEO_PROFILE_UNKNOWN,
                                                     PIPE_VIDEO_ENTRYPOINT_PROCESSING))
      return VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT;

   if (!drv->pipe->screen->is_video_format_supported(drv->pipe->screen,
                                                     dst->buffer_format,
                                                     PIPE_VIDEO_PROFILE_UNKNOWN,
                                                     PIPE_VIDEO_ENTRYPOINT_PROCESSING))
      return VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT;

   struct u_rect src_rect;
   struct u_rect dst_rect;

   src_rect.x0 = src_region->x;
   src_rect.y0 = src_region->y;
   src_rect.x1 = src_region->x + src_region->width;
   src_rect.y1 = src_region->y + src_region->height;

   dst_rect.x0 = dst_region->x;
   dst_rect.y0 = dst_region->y;
   dst_rect.x1 = dst_region->x + dst_region->width;
   dst_rect.y1 = dst_region->y + dst_region->height;

   context->desc.vidproc.base.input_format = src->buffer_format;
   context->desc.vidproc.base.output_format = dst->buffer_format;

   context->desc.vidproc.src_region = src_rect;
   context->desc.vidproc.dst_region = dst_rect;

   if (param->rotation_state == VA_ROTATION_NONE)
      context->desc.vidproc.orientation = PIPE_VIDEO_VPP_ORIENTATION_DEFAULT;
   else if (param->rotation_state == VA_ROTATION_90)
      context->desc.vidproc.orientation = PIPE_VIDEO_VPP_ROTATION_90;
   else if (param->rotation_state == VA_ROTATION_180)
      context->desc.vidproc.orientation = PIPE_VIDEO_VPP_ROTATION_180;
   else if (param->rotation_state == VA_ROTATION_270)
      context->desc.vidproc.orientation = PIPE_VIDEO_VPP_ROTATION_270;

   if (param->mirror_state == VA_MIRROR_HORIZONTAL)
      context->desc.vidproc.orientation |= PIPE_VIDEO_VPP_FLIP_HORIZONTAL;
   if (param->mirror_state == VA_MIRROR_VERTICAL)
      context->desc.vidproc.orientation |= PIPE_VIDEO_VPP_FLIP_VERTICAL;

   memset(&context->desc.vidproc.blend, 0, sizeof(context->desc.vidproc.blend));
   context->desc.vidproc.blend.mode = PIPE_VIDEO_VPP_BLEND_MODE_NONE;
   if (param->blend_state != NULL) {
      if (param->blend_state->flags & VA_BLEND_GLOBAL_ALPHA) {
         context->desc.vidproc.blend.mode = PIPE_VIDEO_VPP_BLEND_MODE_GLOBAL_ALPHA;
         context->desc.vidproc.blend.global_alpha = param->blend_state->global_alpha;
      }
   }

   // Output background color
   context->desc.vidproc.background_color = param->output_background_color;

   // Input surface color standard
   context->desc.vidproc.in_colors_standard = PIPE_VIDEO_VPP_COLOR_STANDARD_TYPE_NONE;
   if (param->surface_color_standard == VAProcColorStandardBT601)
      context->desc.vidproc.in_colors_standard = PIPE_VIDEO_VPP_COLOR_STANDARD_TYPE_BT601;
   else if (param->surface_color_standard == VAProcColorStandardBT709)
      context->desc.vidproc.in_colors_standard = PIPE_VIDEO_VPP_COLOR_STANDARD_TYPE_BT709;
   else if (param->surface_color_standard == VAProcColorStandardBT2020)
      context->desc.vidproc.in_colors_standard = PIPE_VIDEO_VPP_COLOR_STANDARD_TYPE_BT2020;

   // Input surface color range
   context->desc.vidproc.in_color_range = PIPE_VIDEO_VPP_CHROMA_COLOR_RANGE_NONE;
   if (param->input_color_properties.color_range == VA_SOURCE_RANGE_REDUCED)
      context->desc.vidproc.in_color_range = PIPE_VIDEO_VPP_CHROMA_COLOR_RANGE_REDUCED;
   else if (param->input_color_properties.color_range == VA_SOURCE_RANGE_FULL)
      context->desc.vidproc.in_color_range = PIPE_VIDEO_VPP_CHROMA_COLOR_RANGE_FULL;

   // Input surface chroma sample location
   context->desc.vidproc.in_chroma_siting = PIPE_VIDEO_VPP_CHROMA_SITING_NONE;
   if (param->input_color_properties.chroma_sample_location & VA_CHROMA_SITING_VERTICAL_TOP)
      context->desc.vidproc.in_chroma_siting |= PIPE_VIDEO_VPP_CHROMA_SITING_VERTICAL_TOP;
   else if (param->input_color_properties.chroma_sample_location & VA_CHROMA_SITING_VERTICAL_CENTER)
      context->desc.vidproc.in_chroma_siting |= PIPE_VIDEO_VPP_CHROMA_SITING_VERTICAL_CENTER;
   else if (param->input_color_properties.chroma_sample_location & VA_CHROMA_SITING_VERTICAL_BOTTOM)
      context->desc.vidproc.in_chroma_siting |= PIPE_VIDEO_VPP_CHROMA_SITING_VERTICAL_BOTTOM;
   if (param->input_color_properties.chroma_sample_location & VA_CHROMA_SITING_HORIZONTAL_LEFT)
      context->desc.vidproc.in_chroma_siting |= PIPE_VIDEO_VPP_CHROMA_SITING_HORIZONTAL_LEFT;
   else if (param->input_color_properties.chroma_sample_location & VA_CHROMA_SITING_HORIZONTAL_CENTER)
      context->desc.vidproc.in_chroma_siting |= PIPE_VIDEO_VPP_CHROMA_SITING_HORIZONTAL_CENTER;

   // Output surface color standard
   context->desc.vidproc.out_colors_standard = PIPE_VIDEO_VPP_COLOR_STANDARD_TYPE_NONE;
   if (param->output_color_standard == VAProcColorStandardBT601)
      context->desc.vidproc.out_colors_standard = PIPE_VIDEO_VPP_COLOR_STANDARD_TYPE_BT601;
   else if (param->output_color_standard == VAProcColorStandardBT709)
      context->desc.vidproc.out_colors_standard = PIPE_VIDEO_VPP_COLOR_STANDARD_TYPE_BT709;
   else if (param->output_color_standard == VAProcColorStandardBT2020)
      context->desc.vidproc.out_colors_standard = PIPE_VIDEO_VPP_COLOR_STANDARD_TYPE_BT2020;

   // Output surface color range
   context->desc.vidproc.out_color_range = PIPE_VIDEO_VPP_CHROMA_COLOR_RANGE_NONE;
   if (param->output_color_properties.color_range == VA_SOURCE_RANGE_REDUCED)
      context->desc.vidproc.out_color_range = PIPE_VIDEO_VPP_CHROMA_COLOR_RANGE_REDUCED;
   else if (param->output_color_properties.color_range == VA_SOURCE_RANGE_FULL)
      context->desc.vidproc.out_color_range = PIPE_VIDEO_VPP_CHROMA_COLOR_RANGE_FULL;

   // Output surface chroma sample location
   context->desc.vidproc.out_chroma_siting = PIPE_VIDEO_VPP_CHROMA_SITING_NONE;
   if (param->output_color_properties.chroma_sample_location & VA_CHROMA_SITING_VERTICAL_TOP)
      context->desc.vidproc.out_chroma_siting |= PIPE_VIDEO_VPP_CHROMA_SITING_VERTICAL_TOP;
   else if (param->output_color_properties.chroma_sample_location & VA_CHROMA_SITING_VERTICAL_CENTER)
      context->desc.vidproc.out_chroma_siting |= PIPE_VIDEO_VPP_CHROMA_SITING_VERTICAL_CENTER;
   else if (param->output_color_properties.chroma_sample_location & VA_CHROMA_SITING_VERTICAL_BOTTOM)
      context->desc.vidproc.out_chroma_siting |= PIPE_VIDEO_VPP_CHROMA_SITING_VERTICAL_BOTTOM;
   if (param->output_color_properties.chroma_sample_location & VA_CHROMA_SITING_HORIZONTAL_LEFT)
      context->desc.vidproc.out_chroma_siting |= PIPE_VIDEO_VPP_CHROMA_SITING_HORIZONTAL_LEFT;
   else if (param->output_color_properties.chroma_sample_location & VA_CHROMA_SITING_HORIZONTAL_CENTER)
      context->desc.vidproc.out_chroma_siting |= PIPE_VIDEO_VPP_CHROMA_SITING_HORIZONTAL_CENTER;

   if (context->needs_begin_frame) {
      context->decoder->begin_frame(context->decoder, dst,
                                    &context->desc.base);
      context->needs_begin_frame = false;
   }
   context->decoder->process_frame(context->decoder, src, &context->desc.vidproc);

   return VA_STATUS_SUCCESS;
}

static VAStatus vlVaPostProcBlit(vlVaDriver *drv, vlVaContext *context,
                                 const VARectangle *src_region,
                                 const VARectangle *dst_region,
                                 struct pipe_video_buffer *src,
                                 struct pipe_video_buffer *dst,
                                 enum vl_compositor_deinterlace deinterlace)
{
   struct pipe_surface **src_surfaces;
   struct pipe_surface **dst_surfaces;
   struct u_rect src_rect;
   struct u_rect dst_rect;
   bool scale = false;
   bool grab = false;
   unsigned i;

   if ((src->buffer_format == PIPE_FORMAT_B8G8R8X8_UNORM ||
        src->buffer_format == PIPE_FORMAT_B8G8R8A8_UNORM ||
        src->buffer_format == PIPE_FORMAT_R8G8B8X8_UNORM ||
        src->buffer_format == PIPE_FORMAT_R8G8B8A8_UNORM) &&
       !src->interlaced)
      grab = true;

   if ((src->width != dst->width || src->height != dst->height) &&
       (src->interlaced && dst->interlaced))
      scale = true;

   src_surfaces = src->get_surfaces(src);
   if (!src_surfaces || !src_surfaces[0])
      return VA_STATUS_ERROR_INVALID_SURFACE;

   if (scale || (src->interlaced != dst->interlaced && dst->interlaced)) {
      vlVaSurface *surf;

      surf = handle_table_get(drv->htab, context->target_id);
      if (!surf)
         return VA_STATUS_ERROR_INVALID_SURFACE;
      surf->templat.interlaced = false;
      dst->destroy(dst);

      if (vlVaHandleSurfaceAllocate(drv, surf, &surf->templat, NULL, 0) != VA_STATUS_SUCCESS)
         return VA_STATUS_ERROR_ALLOCATION_FAILED;

      dst = context->target = surf->buffer;
   }

   dst_surfaces = dst->get_surfaces(dst);
   if (!dst_surfaces || !dst_surfaces[0])
      return VA_STATUS_ERROR_INVALID_SURFACE;

   src_rect.x0 = src_region->x;
   src_rect.y0 = src_region->y;
   src_rect.x1 = src_region->x + src_region->width;
   src_rect.y1 = src_region->y + src_region->height;

   dst_rect.x0 = dst_region->x;
   dst_rect.y0 = dst_region->y;
   dst_rect.x1 = dst_region->x + dst_region->width;
   dst_rect.y1 = dst_region->y + dst_region->height;

   if (grab) {
      vl_compositor_convert_rgb_to_yuv(&drv->cstate, &drv->compositor, 0,
                                       ((struct vl_video_buffer *)src)->resources[0],
                                       dst, &src_rect, &dst_rect);

      return VA_STATUS_SUCCESS;
   }

   if (src->buffer_format == PIPE_FORMAT_YUYV ||
       src->buffer_format == PIPE_FORMAT_UYVY ||
       src->buffer_format == PIPE_FORMAT_YV12 ||
       src->buffer_format == PIPE_FORMAT_IYUV) {
      vl_compositor_yuv_deint_full(&drv->cstate, &drv->compositor,
                                   src, dst, &src_rect, &dst_rect,
                                   VL_COMPOSITOR_NONE);

      return VA_STATUS_SUCCESS;
   }

   if (src->interlaced != dst->interlaced) {
      deinterlace = deinterlace ? deinterlace : VL_COMPOSITOR_WEAVE;
      vl_compositor_yuv_deint_full(&drv->cstate, &drv->compositor,
                                   src, dst, &src_rect, &dst_rect,
                                   deinterlace);

      return VA_STATUS_SUCCESS;
   }

   for (i = 0; i < VL_MAX_SURFACES; ++i) {
      struct pipe_surface *from = src_surfaces[i];
      struct pipe_blit_info blit;

      if (src->interlaced) {
         /* Not 100% accurate, but close enough */
         switch (deinterlace) {
         case VL_COMPOSITOR_BOB_TOP:
            from = src_surfaces[i & ~1];
            break;
         case VL_COMPOSITOR_BOB_BOTTOM:
            from = src_surfaces[(i & ~1) + 1];
            break;
         default:
            break;
         }
      }

      if (!from || !dst_surfaces[i])
         continue;

      memset(&blit, 0, sizeof(blit));
      blit.src.resource = from->texture;
      blit.src.format = from->format;
      blit.src.level = 0;
      blit.src.box.z = from->u.tex.first_layer;
      blit.src.box.depth = 1;
      vlVaGetBox(src, i, &blit.src.box, src_region);

      blit.dst.resource = dst_surfaces[i]->texture;
      blit.dst.format = dst_surfaces[i]->format;
      blit.dst.level = 0;
      blit.dst.box.z = dst_surfaces[i]->u.tex.first_layer;
      blit.dst.box.depth = 1;
      vlVaGetBox(dst, i, &blit.dst.box, dst_region);

      blit.mask = PIPE_MASK_RGBA;
      blit.filter = PIPE_TEX_MIPFILTER_LINEAR;

      if (drv->pipe->screen->get_param(drv->pipe->screen,
                                       PIPE_CAP_PREFER_COMPUTE_FOR_MULTIMEDIA))
         util_compute_blit(drv->pipe, &blit, &context->blit_cs);
      else
         drv->pipe->blit(drv->pipe, &blit);
   }

   // TODO: figure out why this is necessary for DMA-buf sharing
   drv->pipe->flush(drv->pipe, NULL, 0);

   return VA_STATUS_SUCCESS;
}

static struct pipe_video_buffer *
vlVaApplyDeint(vlVaDriver *drv, vlVaContext *context,
               VAProcPipelineParameterBuffer *param,
               struct pipe_video_buffer *current,
               unsigned field)
{
   vlVaSurface *prevprev, *prev, *next;

   if (param->num_forward_references < 2 ||
       param->num_backward_references < 1)
      return current;

   prevprev = handle_table_get(drv->htab, param->forward_references[1]);
   prev = handle_table_get(drv->htab, param->forward_references[0]);
   next = handle_table_get(drv->htab, param->backward_references[0]);

   if (!prevprev || !prev || !next)
      return current;

   if (context->deint && (context->deint->video_width != current->width ||
       context->deint->video_height != current->height ||
       context->deint->interleaved != !current->interlaced)) {
      vl_deint_filter_cleanup(context->deint);
      FREE(context->deint);
      context->deint = NULL;
   }

   if (!context->deint) {
      context->deint = MALLOC(sizeof(struct vl_deint_filter));
      if (!vl_deint_filter_init(context->deint, drv->pipe, current->width,
                                current->height, false, false, !current->interlaced)) {
         FREE(context->deint);
         context->deint = NULL;
         return current;
      }
   }

   if (!vl_deint_filter_check_buffers(context->deint, prevprev->buffer,
                                      prev->buffer, current, next->buffer))
      return current;

   vl_deint_filter_render(context->deint, prevprev->buffer, prev->buffer,
                          current, next->buffer, field);
   return context->deint->video_buffer;
}

static bool can_convert_with_efc(vlVaSurface *src, vlVaSurface *dst)
{
   enum pipe_format src_format, dst_format;

   if (src->buffer->interlaced)
      return false;

   src_format = src->buffer->buffer_format;

   if (src_format != PIPE_FORMAT_B8G8R8A8_UNORM &&
       src_format != PIPE_FORMAT_R8G8B8A8_UNORM &&
       src_format != PIPE_FORMAT_B8G8R8X8_UNORM &&
       src_format != PIPE_FORMAT_R8G8B8X8_UNORM)
      return false;

   dst_format = dst->encoder_format != PIPE_FORMAT_NONE ?
      dst->encoder_format : dst->buffer->buffer_format;

   return dst_format == PIPE_FORMAT_NV12;
}

VAStatus
vlVaHandleVAProcPipelineParameterBufferType(vlVaDriver *drv, vlVaContext *context, vlVaBuffer *buf)
{
   enum vl_compositor_deinterlace deinterlace = VL_COMPOSITOR_NONE;
   VARectangle def_src_region, def_dst_region;
   const VARectangle *src_region, *dst_region;
   VAProcPipelineParameterBuffer *param;
   struct pipe_video_buffer *src, *dst;
   vlVaSurface *src_surface, *dst_surface;
   unsigned i;
   struct pipe_screen *pscreen;
   VAStatus ret;

   if (!drv || !context)
      return VA_STATUS_ERROR_INVALID_CONTEXT;

   if (!buf || !buf->data)
      return VA_STATUS_ERROR_INVALID_BUFFER;

   if (!context->target)
      return VA_STATUS_ERROR_INVALID_SURFACE;

   param = buf->data;

   src_surface = handle_table_get(drv->htab, param->surface);
   dst_surface = handle_table_get(drv->htab, context->target_id);
   if (!src_surface || !dst_surface)
      return VA_STATUS_ERROR_INVALID_SURFACE;
   if (!src_surface->buffer || !dst_surface->buffer)
      return VA_STATUS_ERROR_INVALID_SURFACE;

   src_surface->full_range = vlVaGetFullRange(src_surface,
      param->input_color_properties.color_range);
   dst_surface->full_range = vlVaGetFullRange(dst_surface,
      param->output_color_properties.color_range);

   pscreen = drv->vscreen->pscreen;

   src_region = vlVaRegionDefault(param->surface_region, src_surface, &def_src_region);
   dst_region = vlVaRegionDefault(param->output_region, dst_surface, &def_dst_region);

   if (!param->num_filters &&
       src_region->width == dst_region->width &&
       src_region->height == dst_region->height &&
       can_convert_with_efc(src_surface, dst_surface) &&
       pscreen->get_video_param(pscreen,
                                PIPE_VIDEO_PROFILE_UNKNOWN,
                                PIPE_VIDEO_ENTRYPOINT_ENCODE,
                                PIPE_VIDEO_CAP_EFC_SUPPORTED)) {

      vlVaSurface *surf = dst_surface;

      // EFC will convert the buffer to a format the encoder accepts
      if (src_surface->buffer->buffer_format != surf->buffer->buffer_format) {
         surf->encoder_format = surf->buffer->buffer_format;

         surf->templat.interlaced = src_surface->templat.interlaced;
         surf->templat.buffer_format = src_surface->templat.buffer_format;
         surf->buffer->destroy(surf->buffer);

         if (vlVaHandleSurfaceAllocate(drv, surf, &surf->templat, NULL, 0) != VA_STATUS_SUCCESS)
            return VA_STATUS_ERROR_ALLOCATION_FAILED;
      }

      pipe_resource_reference(&(((struct vl_video_buffer *)(surf->buffer))->resources[0]), ((struct vl_video_buffer *)(src_surface->buffer))->resources[0]);
      context->target = surf->buffer;

      return VA_STATUS_SUCCESS;
   }

   src = src_surface->buffer;
   dst = dst_surface->buffer;

   /* convert the destination buffer to progressive if we're deinterlacing
      otherwise we might end up deinterlacing twice */
   if (param->num_filters && dst->interlaced) {
      vlVaSurface *surf;
      surf = dst_surface;
      surf->templat.interlaced = false;
      dst->destroy(dst);

      if (vlVaHandleSurfaceAllocate(drv, surf, &surf->templat, NULL, 0) != VA_STATUS_SUCCESS)
         return VA_STATUS_ERROR_ALLOCATION_FAILED;

      dst = context->target = surf->buffer;
   }

   for (i = 0; i < param->num_filters; i++) {
      vlVaBuffer *buf = handle_table_get(drv->htab, param->filters[i]);
      VAProcFilterParameterBufferBase *filter;

      if (!buf || buf->type != VAProcFilterParameterBufferType)
         return VA_STATUS_ERROR_INVALID_BUFFER;

      filter = buf->data;
      switch (filter->type) {
      case VAProcFilterDeinterlacing: {
         VAProcFilterParameterBufferDeinterlacing *deint = buf->data;
         switch (deint->algorithm) {
         case VAProcDeinterlacingBob:
            if (deint->flags & VA_DEINTERLACING_BOTTOM_FIELD)
               deinterlace = VL_COMPOSITOR_BOB_BOTTOM;
            else
               deinterlace = VL_COMPOSITOR_BOB_TOP;
            break;

         case VAProcDeinterlacingWeave:
            deinterlace = VL_COMPOSITOR_WEAVE;
            break;

         case VAProcDeinterlacingMotionAdaptive:
            src = vlVaApplyDeint(drv, context, param, src,
				 !!(deint->flags & VA_DEINTERLACING_BOTTOM_FIELD));
             deinterlace = VL_COMPOSITOR_MOTION_ADAPTIVE;
            break;

         default:
            return VA_STATUS_ERROR_UNIMPLEMENTED;
         }
         drv->compositor.deinterlace = deinterlace;
         break;
      }

      default:
         return VA_STATUS_ERROR_UNIMPLEMENTED;
      }
   }

   /* If the driver supports video engine post proc, attempt to do that
    * if it fails, fallback to the other existing implementations below
    */
   if (pscreen->get_video_param(pscreen,
                                PIPE_VIDEO_PROFILE_UNKNOWN,
                                PIPE_VIDEO_ENTRYPOINT_PROCESSING,
                                PIPE_VIDEO_CAP_SUPPORTED)) {
      if (!context->decoder) {
         context->decoder = drv->pipe->create_video_codec(drv->pipe, &context->templat);
         if (!context->decoder)
            return VA_STATUS_ERROR_ALLOCATION_FAILED;
      }

      context->desc.vidproc.src_surface_fence = src_surface->fence;
      /* Perform VPBlit, if fail, fallback to other implementations below */
      if (VA_STATUS_SUCCESS == vlVaVidEngineBlit(drv, context, src_region, dst_region,
                                                 src, context->target, deinterlace, param))
         return VA_STATUS_SUCCESS;
   }

   vlVaSetProcParameters(drv, src_surface, dst_surface, param);

   /* Try other post proc implementations */
   if (context->target->buffer_format != PIPE_FORMAT_NV12 &&
       context->target->buffer_format != PIPE_FORMAT_P010 &&
       context->target->buffer_format != PIPE_FORMAT_P016)
      ret = vlVaPostProcCompositor(drv, context, src_region, dst_region,
                                   src, context->target, deinterlace);
   else
      ret = vlVaPostProcBlit(drv, context, src_region, dst_region,
                             src, context->target, deinterlace);

   /* Reset chroma location */
   drv->cstate.chroma_location = VL_COMPOSITOR_LOCATION_NONE;

   return ret;
}
