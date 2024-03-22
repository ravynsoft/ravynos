/**************************************************************************
 *
 * Copyright 2022 Advanced Micro Devices, Inc.
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <pipe/p_state.h>
#include <si_pipe.h>
#include "si_vpe.h"

#define SI_VPE_PRETAG              ""
#define SI_VPE_LOG_LEVEL_DEFAULT     1
#define SI_VPE_LOG_LEVEL_INFO        1
#define SI_VPE_LOG_LEVEL_WARNING     2
#define SI_VPE_LOG_LEVEL_DEBUG       3

#define SIVPE_INFO(dblv, fmt, args...)                                                              \
   if ((dblv) >= SI_VPE_LOG_LEVEL_INFO) {                                                           \
      printf("SIVPE INFO: %s: " fmt, __func__, ##args);                                             \
   }

#define SIVPE_WARN(dblv, fmt, args...)                                                              \
   if ((dblv) >= SI_VPE_LOG_LEVEL_WARNING) {                                                        \
      printf("SIVPE WARNING: %s: " fmt, __func__, ##args);                                          \
   }

#define SIVPE_DBG(dblv, fmt, args...)                                                               \
   if ((dblv) >= SI_VPE_LOG_LEVEL_DEBUG) {                                                          \
      printf("SIVPE DBG: %s: " fmt, __func__, ##args);                                              \
   }

#define SIVPE_ERR(fmt, args...)                                                                     \
   fprintf(stderr, "SIVPE ERROR %s:%d %s " fmt, __FILE__, __LINE__, __func__, ##args)

/* Use this enum to help us for accessing the anonymous struct src, dst
 * in blit_info.
 */
enum {
   USE_SRC_SURFACE,
   USE_DST_SURFACE
};


static void *
si_vpe_zalloc(void* mem_ctx, size_t size)
{
   /* mem_ctx is optional for now */
   return CALLOC(1, size);
}


static void
si_vpe_free(void* mem_ctx, void *ptr)
{
   /* mem_ctx is optional for now */
   if (ptr != NULL) {
      FREE(ptr);
      ptr = NULL;
   }
}


static void
si_vpe_log(void* log_ctx, const char* fmt, ...)
{
   /* log_ctx is optional for now */
   va_list args;

   va_start(args, fmt);
   vfprintf(stderr, fmt, args);
   va_end(args);
}


static void
si_vpe_populate_debug_options(struct vpe_debug_options* debug)
{
   /* ref: vpe-utils */
   debug->flags.cm_in_bypass           = 0;
   debug->identity_3dlut               = 0;
   debug->sce_3dlut                    = 0;
   debug->disable_reuse_bit            = 0;
}


static void
si_vpe_populate_callback_modules(struct vpe_callback_funcs* funcs)
{
   funcs->log     = si_vpe_log;
   funcs->zalloc  = si_vpe_zalloc;
   funcs->free    = si_vpe_free;
   return;
}

static char*
si_vpe_get_cositing_str(enum vpe_chroma_cositing cositing)
{
   switch (cositing) {
   case VPE_CHROMA_COSITING_NONE:
      return "NONE";
   case VPE_CHROMA_COSITING_LEFT:
      return "LEFT";
   case VPE_CHROMA_COSITING_TOPLEFT:
      return "TOPLEFT";
   case VPE_CHROMA_COSITING_COUNT:
   default:
      return "ERROR";
   }
}

static char*
si_vpe_get_primarie_str(enum vpe_color_primaries primarie)
{
   switch (primarie) {
   case VPE_PRIMARIES_BT601:
      return "BT601";
   case VPE_PRIMARIES_BT709:
      return "BT709";
   case VPE_PRIMARIES_BT2020:
      return "BT2020";
   case VPE_PRIMARIES_JFIF:
      return "JFIF";
   case VPE_PRIMARIES_COUNT:
   default:
      return "ERROR";
   }
}

static char*
si_vpe_get_tf_str(enum vpe_transfer_function tf)
{
   switch (tf) {
   case VPE_TF_G22:
      return "G22";
   case VPE_TF_G24:
      return "G24";
   case VPE_TF_G10:
      return "G10";
   case VPE_TF_PQ:
      return "PQ";
   case VPE_TF_PQ_NORMALIZED:
      return "PQ_NORMALIZED";
   case VPE_TF_HLG:
      return "HLG";
   case VPE_TF_COUNT:
   default:
      return "ERROR";
   }
}

static enum vpe_status
si_vpe_populate_init_data(struct si_context *sctx, struct vpe_init_data* params, uint8_t log_level)
{
   if (!sctx || !params) {
      return VPE_STATUS_ERROR;
   }

   params->ver_major = sctx->screen->info.ip[AMD_IP_VPE].ver_major;
   params->ver_minor = sctx->screen->info.ip[AMD_IP_VPE].ver_minor;
   params->ver_rev = sctx->screen->info.ip[AMD_IP_VPE].ver_rev;

   si_vpe_populate_debug_options(&params->debug);
   si_vpe_populate_callback_modules(&params->funcs);

   SIVPE_DBG(log_level, "Get family: %d\n", sctx->family);
   SIVPE_DBG(log_level, "Get gfx_level: %d\n", sctx->gfx_level);
   SIVPE_DBG(log_level, "Set ver_major: %d\n", params->ver_major);
   SIVPE_DBG(log_level, "Set ver_minor: %d\n", params->ver_minor);
   SIVPE_DBG(log_level, "Set ver_rev: %d\n", params->ver_rev);

   return VPE_STATUS_OK;
}


static enum vpe_status
si_vpe_allocate_buffer(struct vpe_build_bufs **bufs)
{
   if (!bufs) {
      return VPE_STATUS_ERROR;
   }

   *bufs = (struct vpe_build_bufs *)malloc(sizeof(struct vpe_build_bufs));
   if (!*bufs) {
      return VPE_STATUS_NO_MEMORY;
   }

   (*bufs)->cmd_buf.cpu_va = 0;
   (*bufs)->emb_buf.cpu_va = 0;
   (*bufs)->cmd_buf.size = 0;
   (*bufs)->emb_buf.size = 0;

   return VPE_STATUS_OK;
}

static void
si_vpe_free_buffer(struct vpe_build_bufs *bufs)
{
   if (!bufs) {
      return;
   }
   free(bufs);
}

static enum vpe_surface_pixel_format
si_vpe_format(enum pipe_format format)
{
   enum vpe_surface_pixel_format ret;

   switch (format) {
   /* VPE input format: */
   case PIPE_FORMAT_NV12:
      ret = VPE_SURFACE_PIXEL_FORMAT_VIDEO_420_YCrCb;
      break;
   case PIPE_FORMAT_NV21:
      ret = VPE_SURFACE_PIXEL_FORMAT_VIDEO_420_YCbCr;
      break;
   case PIPE_FORMAT_P010:
      ret = VPE_SURFACE_PIXEL_FORMAT_VIDEO_420_10bpc_YCbCr;
      break;
   /* VPE output format: */
   case PIPE_FORMAT_A8R8G8B8_UNORM:
      ret = VPE_SURFACE_PIXEL_FORMAT_GRPH_BGRA8888;
      break;
   case PIPE_FORMAT_A8B8G8R8_UNORM:
      ret = VPE_SURFACE_PIXEL_FORMAT_GRPH_RGBA8888;
      break;
   case PIPE_FORMAT_R8G8B8A8_UNORM:
      ret = VPE_SURFACE_PIXEL_FORMAT_GRPH_ABGR8888;
      break;
   case PIPE_FORMAT_B8G8R8A8_UNORM:
      ret = VPE_SURFACE_PIXEL_FORMAT_GRPH_ARGB8888;
      break;
   case PIPE_FORMAT_X8R8G8B8_UNORM:
      ret = VPE_SURFACE_PIXEL_FORMAT_GRPH_BGRX8888;
      break;
   case PIPE_FORMAT_X8B8G8R8_UNORM:
      ret = VPE_SURFACE_PIXEL_FORMAT_GRPH_RGBX8888;
      break;
   case PIPE_FORMAT_R8G8B8X8_UNORM:
      ret = VPE_SURFACE_PIXEL_FORMAT_GRPH_XBGR8888;
      break;
   case PIPE_FORMAT_B8G8R8X8_UNORM:
      ret = VPE_SURFACE_PIXEL_FORMAT_GRPH_XRGB8888;
      break;
   case PIPE_FORMAT_A2R10G10B10_UNORM:
      ret = VPE_SURFACE_PIXEL_FORMAT_GRPH_BGRA1010102;
      break;
   case PIPE_FORMAT_A2B10G10R10_UNORM:
      ret = VPE_SURFACE_PIXEL_FORMAT_GRPH_RGBA1010102;
      break;
   case PIPE_FORMAT_B10G10R10A2_UNORM:
      ret = VPE_SURFACE_PIXEL_FORMAT_GRPH_ARGB2101010;
      break;
   case PIPE_FORMAT_R10G10B10A2_UNORM:
      ret = VPE_SURFACE_PIXEL_FORMAT_GRPH_ABGR2101010;
      break;
   default:
      ret = VPE_SURFACE_PIXEL_FORMAT_INVALID;
      break;
   }
   return ret;
}

static enum vpe_status
si_vpe_set_color_space(const struct pipe_vpp_desc *process_properties,
                       struct vpe_color_space *color_space,
                       enum pipe_format format,
                       int which_surface)
{
   enum pipe_video_vpp_color_standard_type colors_standard;
   enum pipe_video_vpp_color_range color_range;
   enum pipe_video_vpp_chroma_siting chroma_siting;

   if (which_surface == USE_SRC_SURFACE) {
      colors_standard = process_properties->in_colors_standard;
      color_range = process_properties->in_color_range;
      chroma_siting = process_properties->in_chroma_siting;
   } else {
      colors_standard = process_properties->out_colors_standard;
      color_range = process_properties->out_color_range;
      chroma_siting = process_properties->out_chroma_siting;
   }

   switch (colors_standard) {
   case PIPE_VIDEO_VPP_COLOR_STANDARD_TYPE_BT601:
      color_space->primaries = VPE_PRIMARIES_BT601;
      color_space->tf = VPE_TF_G24;
      break;
   case PIPE_VIDEO_VPP_COLOR_STANDARD_TYPE_BT709:
      color_space->primaries = VPE_PRIMARIES_BT709;
      color_space->tf = VPE_TF_G22;
      break;
   case PIPE_VIDEO_VPP_COLOR_STANDARD_TYPE_BT2020:
      color_space->primaries = VPE_PRIMARIES_BT2020;
      color_space->tf = VPE_TF_PQ;
      break;
   default:
      color_space->primaries = VPE_PRIMARIES_BT709;
      color_space->tf = VPE_TF_G22;
      break;
   }

   switch (color_range) {
   case PIPE_VIDEO_VPP_CHROMA_COLOR_RANGE_REDUCED:
      color_space->range = VPE_COLOR_RANGE_STUDIO;
      break;
   case PIPE_VIDEO_VPP_CHROMA_COLOR_RANGE_FULL:
      color_space->range = VPE_COLOR_RANGE_FULL;
      break;
   default:
      color_space->range = VPE_COLOR_RANGE_FULL;
      break;
   }

   /* Default use VPE_CHROMA_COSITING_NONE (CENTER | CENTER) */
   color_space->cositing = VPE_CHROMA_COSITING_NONE;
   if (chroma_siting & PIPE_VIDEO_VPP_CHROMA_SITING_VERTICAL_CENTER){
      if (chroma_siting & PIPE_VIDEO_VPP_CHROMA_SITING_HORIZONTAL_LEFT)
         color_space->cositing = VPE_CHROMA_COSITING_LEFT;
      //else if (chroma_siting & PIPE_VIDEO_VPP_CHROMA_SITING_HORIZONTAL_CENTER)
      //   color_space->cositing = VPE_CHROMA_COSITING_NONE;
   } else if (chroma_siting & PIPE_VIDEO_VPP_CHROMA_SITING_VERTICAL_TOP) {
      if (chroma_siting & PIPE_VIDEO_VPP_CHROMA_SITING_HORIZONTAL_LEFT)
         color_space->cositing = VPE_CHROMA_COSITING_TOPLEFT;
      //else if (chroma_siting & PIPE_VIDEO_VPP_CHROMA_SITING_HORIZONTAL_CENTER)
      //   color_space->cositing = VPE_CHROMA_COSITING_NONE;
   } else if (chroma_siting & PIPE_VIDEO_VPP_CHROMA_SITING_VERTICAL_BOTTOM) {
      if (chroma_siting & PIPE_VIDEO_VPP_CHROMA_SITING_HORIZONTAL_LEFT)
         color_space->cositing = VPE_CHROMA_COSITING_LEFT;
      //else if (chroma_siting & PIPE_VIDEO_VPP_CHROMA_SITING_HORIZONTAL_CENTER)
      //   color_space->cositing = VPE_CHROMA_COSITING_NONE;
   }

   /* VPE 1.0 Input format only supports NV12 and NV21 now */
   switch (format) {
   case PIPE_FORMAT_NV12:
   case PIPE_FORMAT_NV21:
   case PIPE_FORMAT_P010:
      color_space->encoding = VPE_PIXEL_ENCODING_YCbCr;
      break;
   case PIPE_FORMAT_A8R8G8B8_UNORM:
   case PIPE_FORMAT_A8B8G8R8_UNORM:
   case PIPE_FORMAT_R8G8B8A8_UNORM:
   case PIPE_FORMAT_B8G8R8A8_UNORM:
   case PIPE_FORMAT_X8R8G8B8_UNORM:
   case PIPE_FORMAT_X8B8G8R8_UNORM:
   case PIPE_FORMAT_R8G8B8X8_UNORM:
   case PIPE_FORMAT_B8G8R8X8_UNORM:
   case PIPE_FORMAT_A2R10G10B10_UNORM:
   case PIPE_FORMAT_R10G10B10A2_UNORM:
   case PIPE_FORMAT_A2B10G10R10_UNORM:
   case PIPE_FORMAT_B10G10R10A2_UNORM:
   default:
      color_space->encoding = VPE_PIXEL_ENCODING_RGB;
      break;
   }
   return VPE_STATUS_OK;
}

/* Combine si_vpe_set_plane_address and si_vpe_set_plane_size*/
static enum vpe_status
si_vpe_set_plane_info(struct vpe_video_processor *vpeproc,
                      const struct pipe_vpp_desc *process_properties,
                      struct pipe_surface **surfaces,
                      int which_surface,
                      struct vpe_surface_info *surface_info)
{
   struct vpe_plane_address *plane_address = &surface_info->address;
   struct vpe_plane_size *plane_size = &surface_info->plane_size;
   struct si_resource *si_res;
   uint32_t width, height, pitch, pos_x, pos_y, offset;
   enum pipe_format format;

   if (which_surface == USE_SRC_SURFACE) {
      pos_x = process_properties->src_region.x0;
      pos_y = process_properties->src_region.y0;
      width  = process_properties->src_region.x1 - pos_x;
      height = process_properties->src_region.y1 - pos_y;
      format = process_properties->base.input_format;
   } else {
      pos_x = process_properties->dst_region.x0;
      pos_y = process_properties->dst_region.y0;
      width  = process_properties->dst_region.x1 - pos_x;
      height = process_properties->dst_region.y1 - pos_y;
      format = process_properties->base.output_format;
   }

   /* Formate Color Space */
   surface_info->format = si_vpe_format(format);
   si_vpe_set_color_space(process_properties, &surface_info->cs, format, which_surface);

   /* Get surface info, such as buffer alignment and offset */
   if (vpeproc->base.context->screen && vpeproc->base.context->screen->resource_get_info)
      vpeproc->base.context->screen->resource_get_info(vpeproc->base.context->screen,
                                                       surfaces[0]->texture,
                                                       &pitch,
                                                       &offset);
   else
      SIVPE_ERR("Get plane pitch and offset info failed\n");

   si_res = si_resource(surfaces[0]->texture);
   plane_address->tmz_surface = false;

   /* here is the SURFACE size, not image rect */
   plane_size->surface_size.x = 0;
   plane_size->surface_size.y = 0;
   plane_size->surface_size.width = surfaces[0]->width;
   plane_size->surface_size.height = surfaces[0]->height;
   plane_size->surface_pitch = pitch;  // Byte alignment

   switch (format) {
   case PIPE_FORMAT_NV12:
   case PIPE_FORMAT_NV21:
      plane_address->type = VPE_PLN_ADDR_TYPE_VIDEO_PROGRESSIVE;
      plane_address->video_progressive.luma_addr.quad_part = si_res->gpu_address + offset;
      plane_address->video_progressive.luma_meta_addr.quad_part = 0;
      plane_address->video_progressive.luma_dcc_const_color.quad_part = 0;
      //plane_size->surface_pitch /= 1;   // Byte alignment to Pixel alignment
      /* Get 2nd plane buffer info */
      if (surfaces[1] && vpeproc->base.context->screen && vpeproc->base.context->screen->resource_get_info)
         vpeproc->base.context->screen->resource_get_info(vpeproc->base.context->screen,
                                                          surfaces[1]->texture,
                                                          &pitch,
                                                          &offset);
      else {
         SIVPE_ERR("Get 2nd plane pitch and offset info failed\n");
         return VPE_STATUS_ERROR;
      }
      si_res = si_resource(surfaces[1]->texture);
      plane_address->video_progressive.chroma_addr.quad_part = si_res->gpu_address + offset;
      plane_address->video_progressive.chroma_meta_addr.quad_part = 0;
      plane_address->video_progressive.chroma_dcc_const_color.quad_part = 0;

      plane_size->chroma_size.x = pos_x;
      plane_size->chroma_size.y = pos_y;
      plane_size->chroma_size.width = (width + 1) / 2;   // 2 pixel-width alignment
      plane_size->chroma_size.height = (height + 1) / 2;  // 2 pixel-height alignment
      plane_size->chroma_pitch = pitch / 2;  // Byte alignment to Pixel alignment (NV12/NV21 2nd plane is 16 bits per pixel)
      break;
   case PIPE_FORMAT_A8R8G8B8_UNORM:
   case PIPE_FORMAT_A8B8G8R8_UNORM:
   case PIPE_FORMAT_R8G8B8A8_UNORM:
   case PIPE_FORMAT_B8G8R8A8_UNORM:
   case PIPE_FORMAT_X8R8G8B8_UNORM:
   case PIPE_FORMAT_X8B8G8R8_UNORM:
   case PIPE_FORMAT_R8G8B8X8_UNORM:
   case PIPE_FORMAT_B8G8R8X8_UNORM:
   case PIPE_FORMAT_A2R10G10B10_UNORM:
   case PIPE_FORMAT_R10G10B10A2_UNORM:
   case PIPE_FORMAT_A2B10G10R10_UNORM:
   case PIPE_FORMAT_B10G10R10A2_UNORM:
   default:
      plane_address->type = VPE_PLN_ADDR_TYPE_GRAPHICS;
      plane_address->grph.addr.quad_part = si_res->gpu_address + offset;
      plane_address->grph.meta_addr.quad_part = 0;
      plane_address->grph.dcc_const_color.quad_part = 0;
      plane_size->surface_pitch /= 4;  // Byte alignment to Pixel alignment (RGBA plane is 32 bits per pixel)

      plane_size->chroma_size.x = 0;
      plane_size->chroma_size.y = 0;
      plane_size->chroma_size.width = 0;
      plane_size->chroma_size.height = 0;
      plane_size->chroma_pitch = 0;
      break;
   }
   return VPE_STATUS_OK;
}

static enum vpe_status
si_vpe_set_surface_info(struct vpe_video_processor *vpeproc,
                        const struct pipe_vpp_desc *process_properties,
                        struct pipe_surface **surfaces,
                        int which_surface,
                        struct vpe_surface_info *surface_info)
{
   assert(surface_info);

   /* Set up surface pitch, plane address, color space */
   si_vpe_set_plane_info(vpeproc, process_properties, surfaces, which_surface, surface_info);

   /* VAAPI does not provide swizzle info right now.
    * Swizzle mode is strongly releated to hardware DMA design,
    * intel-vaapi-driver/i965_drv_video.c also does not handle swizzle information,
    * maybe this is the reason why it is not currently supported.
    *
    * Just default to linear or none temporarily.
    */
   surface_info->swizzle               = VPE_SW_LINEAR;

   struct vpe_plane_dcc_param *dcc_param = &surface_info->dcc;
   dcc_param->enable                   = false;
   dcc_param->meta_pitch               = 0;
   dcc_param->independent_64b_blks     = false;
   dcc_param->dcc_ind_blk              = 0;
   dcc_param->meta_pitch_c             = 0;
   dcc_param->independent_64b_blks_c   = false;
   dcc_param->dcc_ind_blk_c            = 0;

   return VPE_STATUS_OK;
}

static enum vpe_status
si_vpe_set_stream(const struct pipe_vpp_desc *process_properties,
                  struct vpe_stream *stream)
{
   struct vpe_scaling_info *scaling_info = &stream->scaling_info;
   scaling_info->src_rect.x            = process_properties->src_region.x0;
   scaling_info->src_rect.y            = process_properties->src_region.y0;
   scaling_info->src_rect.width        = process_properties->src_region.x1 - process_properties->src_region.x0;
   scaling_info->src_rect.height       = process_properties->src_region.y1 - process_properties->src_region.y0;
   scaling_info->dst_rect.x            = process_properties->dst_region.x0;
   scaling_info->dst_rect.y            = process_properties->dst_region.y0;
   scaling_info->dst_rect.width        = process_properties->dst_region.x1 - process_properties->dst_region.x0;
   scaling_info->dst_rect.height       = process_properties->dst_region.y1 - process_properties->dst_region.y0;
   /* Programmable 1 to 8 taps of vertical polyphase filter, and
    * programmable 1, 2, 4, 6, 8 taps of horizontal polyphase filter.
    */
   scaling_info->taps.v_taps           = 4;
   scaling_info->taps.h_taps           = 4;
   scaling_info->taps.v_taps_c         = 2;
   scaling_info->taps.h_taps_c         = 2;

   /* Blending is not supported for now */
   struct vpe_blend_info *blend_info   = &stream->blend_info;
   blend_info->blending                = false;
   blend_info->pre_multiplied_alpha    = false;
   blend_info->global_alpha            = false;
   blend_info->global_alpha_value      = 0.0;
   /* Global Alpha for Background ? */
   if (process_properties->blend.mode == PIPE_VIDEO_VPP_BLEND_MODE_GLOBAL_ALPHA) {
      blend_info->global_alpha = true;
      blend_info->global_alpha_value = process_properties->blend.global_alpha;
   }

   /* TODO: do ProcAmp in next stage */
   struct vpe_color_adjust *color_adj  = &stream->color_adj;
   color_adj->brightness               = 0.0;
   color_adj->contrast                 = 1.0;
   color_adj->hue                      = 0.0;
   color_adj->saturation               = 1.0;

   /* TODO: Tone Mapping */
   //struct vpe_tonemap_params *tm_params = &stream->tm_params;


   stream->horizontal_mirror           = false;
   stream->vertical_mirror             = false;
   switch (process_properties->orientation & 0xF) {
   case PIPE_VIDEO_VPP_ROTATION_90:
      stream->rotation = VPE_ROTATION_ANGLE_90;
      break;
   case PIPE_VIDEO_VPP_ROTATION_180:
      stream->rotation = VPE_ROTATION_ANGLE_180;
      break;
   case PIPE_VIDEO_VPP_ROTATION_270:
      stream->rotation = VPE_ROTATION_ANGLE_270;
      break;
   default:
      stream->rotation = VPE_ROTATION_ANGLE_0;
      break;
   }
   if (process_properties->orientation & PIPE_VIDEO_VPP_FLIP_HORIZONTAL) {
      stream->horizontal_mirror = true;
   }
   if (process_properties->orientation & PIPE_VIDEO_VPP_FLIP_VERTICAL)
      stream->vertical_mirror = true;

   stream->enable_luma_key             = false;
   stream->lower_luma_bound            = 0.5;
   stream->upper_luma_bound            = 0.5;

   stream->flags.hdr_metadata          = 0;
   stream->flags.reserved              = 0;

   /* TODO: hdr_metadata support in next stage */
   stream->hdr_metadata.redX           = 1;
   stream->hdr_metadata.redY           = 1;
   stream->hdr_metadata.greenX         = 1;
   stream->hdr_metadata.greenY         = 1;
   stream->hdr_metadata.blueX          = 1;
   stream->hdr_metadata.blueY          = 1;
   stream->hdr_metadata.whiteX         = 1;
   stream->hdr_metadata.whiteY         = 1;

   stream->hdr_metadata.min_mastering  = 1;
   stream->hdr_metadata.max_mastering  = 1;
   stream->hdr_metadata.max_content    = 1;
   stream->hdr_metadata.avg_content    = 1;

   return VPE_STATUS_OK;
}

static void
si_vpe_processor_destroy(struct pipe_video_codec *codec)
{
   struct vpe_video_processor *vpeproc = (struct vpe_video_processor *)codec;
   assert(codec);

   if (vpeproc->process_fence) {
      SIVPE_INFO(vpeproc->log_level, "Wait fence\n");
      vpeproc->ws->fence_wait(vpeproc->ws, vpeproc->process_fence, PIPE_DEFAULT_DECODER_FEEDBACK_TIMEOUT_NS);
   }
   vpeproc->ws->cs_destroy(&vpeproc->cs);
   si_vid_destroy_buffer(&vpeproc->emb_buffer);

   if (vpeproc->vpe_build_bufs)
      si_vpe_free_buffer(vpeproc->vpe_build_bufs);
   if (vpeproc->vpe_handle)
      vpe_destroy(&vpeproc->vpe_handle);
   if (vpeproc->vpe_build_param)
      FREE(vpeproc->vpe_build_param);

   SIVPE_DBG(vpeproc->log_level, "Success\n");
   FREE(vpeproc);
}

static void
si_vpe_processor_begin_frame(struct pipe_video_codec *codec,
                             struct pipe_video_buffer *target,
                             struct pipe_picture_desc *picture)
{
   struct vpe_video_processor *vpeproc = (struct vpe_video_processor *)codec;
   struct pipe_surface **dst_surfaces;
   assert(codec);

   dst_surfaces = target->get_surfaces(target);
   if (!dst_surfaces || !dst_surfaces[0]) {
      SIVPE_ERR("Get target surface failed\n");
      return;
   }
   vpeproc->dst_surfaces = dst_surfaces;
}

static void
si_vpe_cs_add_surface_buffer(struct vpe_video_processor *vpeproc,
                             struct pipe_surface **surfaces,
                             unsigned usage)
{
   struct si_resource *si_res;
   int i;

   for (i = 0; i < VL_MAX_SURFACES; ++i) {
      if (!surfaces[i])
         continue;

      si_res = si_resource(surfaces[i]->texture);
      vpeproc->ws->cs_add_buffer(&vpeproc->cs, si_res->buf, usage | RADEON_USAGE_SYNCHRONIZED, 0);
   }
}

static void
si_vpe_processor_process_frame(struct pipe_video_codec *codec,
                               struct pipe_video_buffer *input_texture,
                               const struct pipe_vpp_desc *process_properties)
{
   enum vpe_status result = VPE_STATUS_OK;
   struct vpe_video_processor *vpeproc = (struct vpe_video_processor *)codec;
   struct vpe *vpe_handle = vpeproc->vpe_handle;
   struct vpe_build_param *build_param = vpeproc->vpe_build_param;
   struct pipe_surface **src_surfaces;
   struct vpe_bufs_req bufs_required;
   struct pipe_fence_handle *process_fence = NULL;

   assert(codec);
   assert(process_properties);
   assert(vpeproc->dst_surfaces);

   src_surfaces = input_texture->get_surfaces(input_texture);
   if (!src_surfaces || !src_surfaces[0]) {
      SIVPE_ERR("Get source surface failed\n");
      return;
   }
   vpeproc->src_surfaces = src_surfaces;

   /* Following setting is from si_vpe_set_build_param()*/
   /* VPE 1.0 only support one input */
   build_param->num_streams = 1;

   /* Allocate array "streams" */
   build_param->streams = (struct vpe_stream *)CALLOC(build_param->num_streams, sizeof(struct vpe_stream));
   if (!build_param->streams) {
      SIVPE_ERR("Allocate streams failed\n");
      free(build_param);
      return;
   }

   si_vpe_set_surface_info(vpeproc,
                           process_properties,
                           vpeproc->src_surfaces,
                           USE_SRC_SURFACE, 
                           &build_param->streams[0].surface_info);

   si_vpe_set_stream(process_properties, &build_param->streams[0]);

   si_vpe_set_surface_info(vpeproc,
                           process_properties,
                           vpeproc->dst_surfaces,
                           USE_DST_SURFACE, 
                           &build_param->dst_surface);

   if (process_properties->background_color) {
      build_param->target_rect.x = 0;
      build_param->target_rect.y = 0;
      build_param->target_rect.width = vpeproc->dst_surfaces[0]->width;
      build_param->target_rect.height = vpeproc->dst_surfaces[0]->height;
   } else {
      build_param->target_rect.x = process_properties->dst_region.x0;
      build_param->target_rect.y = process_properties->dst_region.y0;
      build_param->target_rect.width = process_properties->dst_region.x1 - process_properties->dst_region.x0;
      build_param->target_rect.height = process_properties->dst_region.y1 - process_properties->dst_region.y0;
   }
 
   /* TODO: background color is not specified in pipe_vpp_desc structure.
    * Need to add this filed in pipe_vpp_desc.
   */
   build_param->bg_color.is_ycbcr            = false;

   if (!(process_properties->background_color & 0xFFFFFF) &&
       (build_param->dst_surface.cs.range == VPE_COLOR_RANGE_STUDIO)) {
      build_param->bg_color.rgba.a =
         (float)((process_properties->background_color & 0xFF000000) >> 24) / 255.0;
      build_param->bg_color.rgba.r = 0.0628;
      build_param->bg_color.rgba.g = 0.0628;
      build_param->bg_color.rgba.b = 0.0628;
   } else if (process_properties->background_color) {
      build_param->bg_color.rgba.a = 
         (float)((process_properties->background_color & 0xFF000000) >> 24) / 255.0;
      build_param->bg_color.rgba.r = 
         (float)((process_properties->background_color & 0x00FF0000) >> 16) / 255.0;
      build_param->bg_color.rgba.g = 
         (float)((process_properties->background_color & 0x0000FF00) >> 8) / 255.0;
      build_param->bg_color.rgba.b = 
         (float)(process_properties->background_color & 0x000000FF) / 255.0;
   } else {
      build_param->bg_color.rgba.r           = 0;
      build_param->bg_color.rgba.g           = 0;
      build_param->bg_color.rgba.b           = 0;
      build_param->bg_color.rgba.a           = 0;
   }

   build_param->alpha_mode                   = VPE_ALPHA_OPAQUE;

   build_param->flags.hdr_metadata           = 0;
   build_param->flags.reserved               = 1;

   /* TODO: hdr_metadata support in next stage */
   build_param->hdr_metadata.redX            = 1;
   build_param->hdr_metadata.redY            = 1;
   build_param->hdr_metadata.greenX          = 1;
   build_param->hdr_metadata.greenY          = 1;
   build_param->hdr_metadata.blueX           = 1;
   build_param->hdr_metadata.blueY           = 1;
   build_param->hdr_metadata.whiteX          = 1;
   build_param->hdr_metadata.whiteY          = 1;

   build_param->hdr_metadata.min_mastering   = 1;
   build_param->hdr_metadata.max_mastering   = 1;
   build_param->hdr_metadata.max_content     = 1;
   build_param->hdr_metadata.avg_content     = 1;

   uint64_t *vpe_ptr;
   vpe_ptr = (uint64_t *)vpeproc->cs.current.buf;
   vpeproc->vpe_build_bufs->cmd_buf.cpu_va = (uintptr_t)vpe_ptr;
   vpeproc->vpe_build_bufs->cmd_buf.gpu_va = 0;
   vpeproc->vpe_build_bufs->cmd_buf.size = vpeproc->cs.current.max_dw;
   vpeproc->vpe_build_bufs->cmd_buf.tmz = false;

   vpe_ptr = (uint64_t *)vpeproc->ws->buffer_map(vpeproc->ws, vpeproc->emb_buffer.res->buf,
                                                 &vpeproc->cs, PIPE_MAP_WRITE | RADEON_MAP_TEMPORARY);
   vpeproc->vpe_build_bufs->emb_buf.cpu_va = (uintptr_t)vpe_ptr;
   vpeproc->vpe_build_bufs->emb_buf.gpu_va = vpeproc->ws->buffer_get_virtual_address(vpeproc->emb_buffer.res->buf);
   vpeproc->vpe_build_bufs->emb_buf.size = VPE_BUILD_BUFS_SIZE;
   vpeproc->vpe_build_bufs->emb_buf.tmz = false;

   if (vpeproc->log_level >= SI_VPE_LOG_LEVEL_DEBUG) {
      SIVPE_DBG(vpeproc->log_level, "src surface format(%d) rect (%d, %d, %d, %d)\n",
               build_param->streams[0].surface_info.format,
               build_param->streams[0].surface_info.plane_size.surface_size.x,
               build_param->streams[0].surface_info.plane_size.surface_size.y,
               build_param->streams[0].surface_info.plane_size.surface_size.width,
               build_param->streams[0].surface_info.plane_size.surface_size.height);

      SIVPE_DBG(vpeproc->log_level, "src surface Cositing(%s), primaries(%s), tf(%s), range(%s)\n",
               si_vpe_get_cositing_str(build_param->streams[0].surface_info.cs.cositing),
               si_vpe_get_primarie_str(build_param->streams[0].surface_info.cs.primaries),
               si_vpe_get_tf_str(build_param->streams[0].surface_info.cs.tf),
               (build_param->streams[0].surface_info.cs.range == VPE_COLOR_RANGE_FULL)?"FULL":"STUDIO");

      SIVPE_DBG(vpeproc->log_level, "dst surface format(%d) rect (%d, %d, %d, %d)\n",
               build_param->dst_surface.format,
               build_param->dst_surface.plane_size.surface_size.x,
               build_param->dst_surface.plane_size.surface_size.y,
               build_param->dst_surface.plane_size.surface_size.width,
               build_param->dst_surface.plane_size.surface_size.height);

      SIVPE_DBG(vpeproc->log_level, "dst surface Cositing(%s), primaries(%s), tf(%s), range(%s)\n",
               si_vpe_get_cositing_str(build_param->dst_surface.cs.cositing),
               si_vpe_get_primarie_str(build_param->dst_surface.cs.primaries),
               si_vpe_get_tf_str(build_param->dst_surface.cs.tf),
               (build_param->dst_surface.cs.range == VPE_COLOR_RANGE_FULL)?"FULL":"STUDIO");

      SIVPE_DBG(vpeproc->log_level, "background color RGBA(%0.3f, %0.3f, %0.3f, %0.3f)\n",
               build_param->bg_color.rgba.r,
               build_param->bg_color.rgba.g,
               build_param->bg_color.rgba.b,
               build_param->bg_color.rgba.a);

      SIVPE_DBG(vpeproc->log_level, "target_rect(%d, %d, %d, %d)\n",
               build_param->target_rect.x,
               build_param->target_rect.y,
               build_param->target_rect.width,
               build_param->target_rect.height);

      SIVPE_DBG(vpeproc->log_level, "rotation(%d) horizontal_mirror(%d) vertical_mirror(%d)\n", 
               build_param->streams[0].rotation,
               build_param->streams[0].horizontal_mirror,
               build_param->streams[0].vertical_mirror);

      SIVPE_DBG(vpeproc->log_level, "scaling_src_rect(%d, %d, %d, %d)\n", 
               build_param->streams[0].scaling_info.src_rect.x,
               build_param->streams[0].scaling_info.src_rect.y,
               build_param->streams[0].scaling_info.src_rect.width,
               build_param->streams[0].scaling_info.src_rect.height);

      SIVPE_DBG(vpeproc->log_level, "scaling_dst_rect(%d, %d, %d, %d)\n", 
               build_param->streams[0].scaling_info.dst_rect.x,
               build_param->streams[0].scaling_info.dst_rect.y,
               build_param->streams[0].scaling_info.dst_rect.width,
               build_param->streams[0].scaling_info.dst_rect.height);

      SIVPE_DBG(vpeproc->log_level, "scaling_taps h_taps(%d) v_taps(%d) h_taps_c(%d) v_taps_c(%d)\n", 
               build_param->streams[0].scaling_info.taps.h_taps,
               build_param->streams[0].scaling_info.taps.v_taps,
               build_param->streams[0].scaling_info.taps.h_taps_c,
               build_param->streams[0].scaling_info.taps.v_taps_c);

      SIVPE_DBG(vpeproc->log_level, "blend global_alpha(%d): %0.3f\n",
               build_param->streams[0].blend_info.global_alpha,
               build_param->streams[0].blend_info.global_alpha_value);

      SIVPE_DBG(vpeproc->log_level, "ToneMapping shaper_tf(%d) lut_out_tf(%d) lut_in_gamut(%d) lut_out_gamut(%d)\n",
               build_param->streams[0].tm_params.shaper_tf,
               build_param->streams[0].tm_params.lut_out_tf,
               build_param->streams[0].tm_params.lut_in_gamut,
               build_param->streams[0].tm_params.lut_out_gamut);

      //SIVPE_DBG(vpeproc->log_level, "ToneMapping update_3dlut(%d) enable_3dlut(%d)\n",
      //         build_param->streams[0].tm_params.update_3dlut,
      //         build_param->streams[0].tm_params.enable_3dlut);
   }

   result = vpe_check_support(vpe_handle, build_param, &bufs_required);
   if (VPE_STATUS_OK != result) {
      SIVPE_ERR("Check support failed with result: %d\n", result);
      goto fail;
   }

   result = vpe_build_commands(vpe_handle, build_param, vpeproc->vpe_build_bufs);
   if (VPE_STATUS_OK != result) {
      SIVPE_ERR("Build commands failed with result: %d\n", result);
      goto fail;
   }

   /* Check buffer size */
   if (vpeproc->vpe_build_bufs->cmd_buf.size == 0 || vpeproc->vpe_build_bufs->cmd_buf.size == vpeproc->cs.current.max_dw) {
      SIVPE_ERR("Cmdbuf size wrong\n");
      goto fail;
   }
   if (vpeproc->vpe_build_bufs->emb_buf.size == 0 || vpeproc->vpe_build_bufs->emb_buf.size == VPE_BUILD_BUFS_SIZE) {
      SIVPE_ERR("Embbuf size wrong\n");
      goto fail;
   }

   /* Wait Source Surface fence */
   if (process_properties->src_surface_fence) {
      struct pipe_fence_handle *input_fence = (struct pipe_fence_handle *)process_properties->src_surface_fence;
      while (!vpeproc->ws->fence_wait(vpeproc->ws, input_fence, VPE_FENCE_TIMEOUT_NS))
         SIVPE_INFO(vpeproc->log_level, "Wait source surface fence fail\n");
   }

   /* Have to tell Command Submission context the command length wrote by libvpe */
   vpeproc->cs.current.cdw += (vpeproc->vpe_build_bufs->cmd_buf.size / 4);

   /* Add embbuf into bo_handle list */
   vpeproc->ws->buffer_unmap(vpeproc->ws, vpeproc->emb_buffer.res->buf);
   vpeproc->ws->cs_add_buffer(&vpeproc->cs, vpeproc->emb_buffer.res->buf, RADEON_USAGE_READ | RADEON_USAGE_SYNCHRONIZED, RADEON_DOMAIN_GTT);

   si_vpe_cs_add_surface_buffer(vpeproc, vpeproc->src_surfaces, RADEON_USAGE_READ);
   si_vpe_cs_add_surface_buffer(vpeproc, vpeproc->dst_surfaces, RADEON_USAGE_WRITE);

   vpeproc->ws->cs_flush(&vpeproc->cs, PIPE_FLUSH_ASYNC, &process_fence);

   if (process_fence)
      vpeproc->process_fence = process_fence;
   SIVPE_INFO(vpeproc->log_level, "Flush success\n");

   FREE(build_param->streams);
   SIVPE_DBG(vpeproc->log_level, "Success\n");
   return;

fail:
   vpeproc->ws->buffer_unmap(vpeproc->ws, vpeproc->emb_buffer.res->buf);
   FREE(build_param->streams);
   SIVPE_ERR("Failed\n");
   return;
}

static void
si_vpe_processor_end_frame(struct pipe_video_codec *codec,
                           struct pipe_video_buffer *target,
                           struct pipe_picture_desc *picture)
{
   struct vpe_video_processor *vpeproc = (struct vpe_video_processor *)codec;
   assert(codec);

   if (picture->fence && vpeproc->process_fence) {
      *picture->fence = vpeproc->process_fence;
      SIVPE_INFO(vpeproc->log_level, "Assign process fence\n");
   }
   SIVPE_INFO(vpeproc->log_level, "Success\n");
}

static void
si_vpe_processor_flush(struct pipe_video_codec *codec)
{
   struct vpe_video_processor *vpeproc = (struct vpe_video_processor *)codec;
   assert(codec);

   SIVPE_DBG(vpeproc->log_level, "Success\n");
   return;
}

static int si_vpe_processor_get_processor_fence(struct pipe_video_codec *codec,
                                                struct pipe_fence_handle *fence,
                                                uint64_t timeout)
{
   struct vpe_video_processor *vpeproc = (struct vpe_video_processor *)codec;
   assert(codec);

   SIVPE_INFO(vpeproc->log_level, "Wait processor fence\n");
   while (!vpeproc->ws->fence_wait(vpeproc->ws, fence, timeout))
      SIVPE_DBG(vpeproc->log_level, "Wait processor fence fail\n");
   SIVPE_INFO(vpeproc->log_level, "Wait processor fence success\n");
   return 1;
}

struct pipe_video_codec*
si_vpe_create_processor(struct pipe_context *context, const struct pipe_video_codec *templ)
{
   struct si_context *sctx = (struct si_context *)context;
   struct radeon_winsys *ws = sctx->ws;
   struct vpe_video_processor *vpeproc;
   struct vpe_init_data *init_data;
   const char *str = getenv("AMDGPU_SIVPE_LOG_LEVEL");

   vpeproc = CALLOC_STRUCT(vpe_video_processor);
   if (!vpeproc) {
      SIVPE_ERR("Allocate struct failed\n");
      return NULL;
   }

   /* get SI_VPE debug log level */
   if (str == NULL)
      vpeproc->log_level = SI_VPE_LOG_LEVEL_DEFAULT;
   else
      vpeproc->log_level = atoi(str);

   vpeproc->base = *templ;
   vpeproc->base.context = context;
   vpeproc->base.width = templ->width;
   vpeproc->base.height = templ->height;

   vpeproc->base.destroy = si_vpe_processor_destroy;
   vpeproc->base.begin_frame = si_vpe_processor_begin_frame;
   vpeproc->base.process_frame = si_vpe_processor_process_frame;
   vpeproc->base.end_frame = si_vpe_processor_end_frame;
   vpeproc->base.flush = si_vpe_processor_flush;
   vpeproc->base.get_processor_fence = si_vpe_processor_get_processor_fence;

   vpeproc->ver_major = sctx->screen->info.ip[AMD_IP_VPE].ver_major;
   vpeproc->ver_minor = sctx->screen->info.ip[AMD_IP_VPE].ver_minor;

   vpeproc->screen = context->screen;
   vpeproc->ws = ws;
   vpeproc->process_fence = NULL;

   init_data = &vpeproc->vpe_data;
   if (VPE_STATUS_OK != si_vpe_populate_init_data(sctx, init_data, vpeproc->log_level)){
      SIVPE_ERR("Init VPE populate data failed\n");
      goto fail;
   }

   vpeproc->vpe_handle = vpe_create(init_data);
   if (!vpeproc->vpe_handle) {
      SIVPE_ERR("Create VPE handle failed\n");
      goto fail;
   }

   if (VPE_STATUS_OK != si_vpe_allocate_buffer(&vpeproc->vpe_build_bufs)) {
      SIVPE_ERR("Allocate VPE buffers failed\n");
      goto fail;
   }

   /* Create Command Submission context.
    * The cmdbuf (Vpe Descriptor) will be stored in cs.current.buf
    * there is no needs to allocate another buffer handle for cmdbuf.
    */
   if (!ws->cs_create(&vpeproc->cs, sctx->ctx, AMD_IP_VPE, NULL, NULL)) {
      SIVPE_ERR("Get command submission context failed.\n");
      goto fail;
   }
   vpeproc->vpe_build_bufs->cmd_buf.size = vpeproc->cs.current.max_dw;

   /* Creste Vpblit Descriptor buffer.
    * This buffer will store plane config / VPEP config commands
    */
   if (!si_vid_create_buffer(vpeproc->screen, &vpeproc->emb_buffer, VPE_BUILD_BUFS_SIZE,
                                PIPE_USAGE_DEFAULT)) {
      SIVPE_ERR("Allocate VPE emb buffers failed.\n");
      goto fail;
   }
   si_vid_clear_buffer(context, &vpeproc->emb_buffer);
   vpeproc->vpe_build_bufs->emb_buf.size = VPE_BUILD_BUFS_SIZE;

   /* Create VPE parameters structure */
   vpeproc->vpe_build_param = CALLOC_STRUCT(vpe_build_param);
   if (!vpeproc->vpe_build_param) {
      SIVPE_ERR("Allocate build-paramaters sturcture  failed\n");
      goto fail;
   }

   return &vpeproc->base;

fail:
   SIVPE_ERR("Failed\n");
   if (vpeproc) {
      si_vpe_processor_destroy(&vpeproc->base);
   }
   return NULL;
}
