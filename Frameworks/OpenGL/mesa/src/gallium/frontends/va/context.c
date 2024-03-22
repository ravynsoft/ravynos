/**************************************************************************
 *
 * Copyright 2010 Thomas Balling Sørensen & Orasanu Lucian.
 * Copyright 2014 Advanced Micro Devices, Inc.
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

#include "pipe/p_screen.h"
#include "pipe/p_video_codec.h"
#include "util/u_memory.h"
#include "util/u_handle_table.h"
#include "util/u_video.h"
#include "util/set.h"
#include "vl/vl_deint_filter.h"
#include "vl/vl_winsys.h"

#include "va_private.h"
#ifdef HAVE_DRISW_KMS
#include "loader/loader.h"
#endif

#include <va/va_drmcommon.h>

static struct VADriverVTable vtable =
{
   &vlVaTerminate,
   &vlVaQueryConfigProfiles,
   &vlVaQueryConfigEntrypoints,
   &vlVaGetConfigAttributes,
   &vlVaCreateConfig,
   &vlVaDestroyConfig,
   &vlVaQueryConfigAttributes,
   &vlVaCreateSurfaces,
   &vlVaDestroySurfaces,
   &vlVaCreateContext,
   &vlVaDestroyContext,
   &vlVaCreateBuffer,
   &vlVaBufferSetNumElements,
   &vlVaMapBuffer,
   &vlVaUnmapBuffer,
   &vlVaDestroyBuffer,
   &vlVaBeginPicture,
   &vlVaRenderPicture,
   &vlVaEndPicture,
   &vlVaSyncSurface,
   &vlVaQuerySurfaceStatus,
   &vlVaQuerySurfaceError,
   &vlVaPutSurface,
   &vlVaQueryImageFormats,
   &vlVaCreateImage,
   &vlVaDeriveImage,
   &vlVaDestroyImage,
   &vlVaSetImagePalette,
   &vlVaGetImage,
   &vlVaPutImage,
   &vlVaQuerySubpictureFormats,
   &vlVaCreateSubpicture,
   &vlVaDestroySubpicture,
   &vlVaSubpictureImage,
   &vlVaSetSubpictureChromakey,
   &vlVaSetSubpictureGlobalAlpha,
   &vlVaAssociateSubpicture,
   &vlVaDeassociateSubpicture,
   &vlVaQueryDisplayAttributes,
   &vlVaGetDisplayAttributes,
   &vlVaSetDisplayAttributes,
   &vlVaBufferInfo,
   &vlVaLockSurface,
   &vlVaUnlockSurface,
   NULL, /* DEPRECATED VaGetSurfaceAttributes */
   &vlVaCreateSurfaces2,
   &vlVaQuerySurfaceAttributes,
   &vlVaAcquireBufferHandle,
   &vlVaReleaseBufferHandle,
#if VA_CHECK_VERSION(1, 1, 0)
   NULL, /* vaCreateMFContext */
   NULL, /* vaMFAddContext */
   NULL, /* vaMFReleaseContext */
   NULL, /* vaMFSubmit */
   NULL, /* vaCreateBuffer2 */
   NULL, /* vaQueryProcessingRate */
   &vlVaExportSurfaceHandle,
#endif
#if VA_CHECK_VERSION(1, 15, 0)
   NULL, /* vaSyncSurface2 */
   &vlVaSyncBuffer,
#endif
#if VA_CHECK_VERSION(1, 21, 0)
   NULL, /* vaCopy */
   &vlVaMapBuffer2,
#endif
};

static struct VADriverVTableVPP vtable_vpp =
{
   1,
   &vlVaQueryVideoProcFilters,
   &vlVaQueryVideoProcFilterCaps,
   &vlVaQueryVideoProcPipelineCaps
};

VA_PUBLIC_API VAStatus
VA_DRIVER_INIT_FUNC(VADriverContextP ctx)
{
   vlVaDriver *drv;

   if (!ctx)
      return VA_STATUS_ERROR_INVALID_CONTEXT;

   drv = CALLOC(1, sizeof(vlVaDriver));
   if (!drv)
      return VA_STATUS_ERROR_ALLOCATION_FAILED;

   switch (ctx->display_type) {
#ifdef _WIN32
   case VA_DISPLAY_WIN32: {
      drv->vscreen = vl_win32_screen_create(ctx->native_dpy);
      break;
   }
#else
   case VA_DISPLAY_ANDROID:
      FREE(drv);
      return VA_STATUS_ERROR_UNIMPLEMENTED;
   case VA_DISPLAY_GLX:
   case VA_DISPLAY_X11:
      drv->vscreen = vl_dri3_screen_create(ctx->native_dpy, ctx->x11_screen);
      if (!drv->vscreen)
         drv->vscreen = vl_dri2_screen_create(ctx->native_dpy, ctx->x11_screen);
      if (!drv->vscreen)
         drv->vscreen = vl_xlib_swrast_screen_create(ctx->native_dpy, ctx->x11_screen);
      break;
   case VA_DISPLAY_WAYLAND:
   case VA_DISPLAY_DRM:
   case VA_DISPLAY_DRM_RENDERNODES: {
      const struct drm_state *drm_info = (struct drm_state *) ctx->drm_state;

      if (!drm_info || drm_info->fd < 0) {
         FREE(drv);
         return VA_STATUS_ERROR_INVALID_PARAMETER;
      }
#ifdef HAVE_DRISW_KMS
      char* drm_driver_name = loader_get_driver_for_fd(drm_info->fd);
      if(drm_driver_name) {
         if (strcmp(drm_driver_name, "vgem") == 0)
            drv->vscreen = vl_vgem_drm_screen_create(drm_info->fd);
         FREE(drm_driver_name);
      }
#endif
      if(!drv->vscreen)
         drv->vscreen = vl_drm_screen_create(drm_info->fd);
      break;
   }
#endif
   default:
      FREE(drv);
      return VA_STATUS_ERROR_INVALID_DISPLAY;
   }

   if (!drv->vscreen)
      goto error_screen;

   drv->pipe = pipe_create_multimedia_context(drv->vscreen->pscreen);
   if (!drv->pipe)
      goto error_pipe;

   drv->htab = handle_table_create();
   if (!drv->htab)
      goto error_htab;

   if (!vl_compositor_init(&drv->compositor, drv->pipe))
      goto error_compositor;
   if (!vl_compositor_init_state(&drv->cstate, drv->pipe))
      goto error_compositor_state;

   vl_csc_get_matrix(VL_CSC_COLOR_STANDARD_BT_601, NULL, true, &drv->csc);
   if (!vl_compositor_set_csc_matrix(&drv->cstate, (const vl_csc_matrix *)&drv->csc, 1.0f, 0.0f))
      goto error_csc_matrix;
   (void) mtx_init(&drv->mutex, mtx_plain);

   ctx->pDriverData = (void *)drv;
   ctx->version_major = 0;
   ctx->version_minor = 1;
   *ctx->vtable = vtable;
   *ctx->vtable_vpp = vtable_vpp;
   ctx->max_profiles = PIPE_VIDEO_PROFILE_MAX - PIPE_VIDEO_PROFILE_UNKNOWN - 1;
   ctx->max_entrypoints = 2;
   ctx->max_attributes = 1;
   ctx->max_image_formats = VL_VA_MAX_IMAGE_FORMATS;
   ctx->max_subpic_formats = 1;
   ctx->max_display_attributes = 0;

   snprintf(drv->vendor_string, sizeof(drv->vendor_string),
            "Mesa Gallium driver " PACKAGE_VERSION " for %s",
            drv->vscreen->pscreen->get_name(drv->vscreen->pscreen));
   ctx->str_vendor = drv->vendor_string;

   return VA_STATUS_SUCCESS;

error_csc_matrix:
   vl_compositor_cleanup_state(&drv->cstate);

error_compositor_state:
   vl_compositor_cleanup(&drv->compositor);

error_compositor:
   handle_table_destroy(drv->htab);

error_htab:
   drv->pipe->destroy(drv->pipe);

error_pipe:
   drv->vscreen->destroy(drv->vscreen);

error_screen:
   FREE(drv);
   return VA_STATUS_ERROR_ALLOCATION_FAILED;
}

VAStatus
vlVaCreateContext(VADriverContextP ctx, VAConfigID config_id, int picture_width,
                  int picture_height, int flag, VASurfaceID *render_targets,
                  int num_render_targets, VAContextID *context_id)
{
   vlVaDriver *drv;
   vlVaContext *context;
   vlVaConfig *config;
   int is_vpp;
   int min_supported_width, min_supported_height;
   int max_supported_width, max_supported_height;

   if (!ctx)
      return VA_STATUS_ERROR_INVALID_CONTEXT;

   drv = VL_VA_DRIVER(ctx);
   mtx_lock(&drv->mutex);
   config = handle_table_get(drv->htab, config_id);
   mtx_unlock(&drv->mutex);

   if (!config)
      return VA_STATUS_ERROR_INVALID_CONFIG;

   is_vpp = config->profile == PIPE_VIDEO_PROFILE_UNKNOWN && !picture_width &&
            !picture_height && !flag && !render_targets && !num_render_targets;

   if (!(picture_width && picture_height) && !is_vpp)
      return VA_STATUS_ERROR_INVALID_IMAGE_FORMAT;

   context = CALLOC(1, sizeof(vlVaContext));
   if (!context)
      return VA_STATUS_ERROR_ALLOCATION_FAILED;

   if (is_vpp && !drv->vscreen->pscreen->get_video_param(drv->vscreen->pscreen,
                                                         PIPE_VIDEO_PROFILE_UNKNOWN,
                                                         PIPE_VIDEO_ENTRYPOINT_PROCESSING,
                                                         PIPE_VIDEO_CAP_SUPPORTED)) {
      context->decoder = NULL;
   } else {
      if (config->entrypoint != PIPE_VIDEO_ENTRYPOINT_PROCESSING) {
         min_supported_width = drv->vscreen->pscreen->get_video_param(drv->vscreen->pscreen,
                        config->profile, config->entrypoint,
                        PIPE_VIDEO_CAP_MIN_WIDTH);
         min_supported_height = drv->vscreen->pscreen->get_video_param(drv->vscreen->pscreen,
                        config->profile, config->entrypoint,
                        PIPE_VIDEO_CAP_MIN_HEIGHT);
         max_supported_width = drv->vscreen->pscreen->get_video_param(drv->vscreen->pscreen,
                        config->profile, config->entrypoint,
                        PIPE_VIDEO_CAP_MAX_WIDTH);
         max_supported_height = drv->vscreen->pscreen->get_video_param(drv->vscreen->pscreen,
                        config->profile, config->entrypoint,
                        PIPE_VIDEO_CAP_MAX_HEIGHT);

         if (picture_width < min_supported_width || picture_height < min_supported_height ||
             picture_width > max_supported_width || picture_height > max_supported_height) {
            FREE(context);
            return VA_STATUS_ERROR_RESOLUTION_NOT_SUPPORTED;
         }
      }
      context->templat.profile = config->profile;
      context->templat.entrypoint = config->entrypoint;
      context->templat.chroma_format = PIPE_VIDEO_CHROMA_FORMAT_420;
      context->templat.width = picture_width;
      context->templat.height = picture_height;
      context->templat.expect_chunked_decode = true;

      switch (u_reduce_video_profile(context->templat.profile)) {
      case PIPE_VIDEO_FORMAT_MPEG12:
      case PIPE_VIDEO_FORMAT_VC1:
      case PIPE_VIDEO_FORMAT_MPEG4:
         context->templat.max_references = 2;
         break;

      case PIPE_VIDEO_FORMAT_MPEG4_AVC:
         context->templat.max_references = 0;
         if (config->entrypoint != PIPE_VIDEO_ENTRYPOINT_ENCODE) {
            context->desc.h264.pps = CALLOC_STRUCT(pipe_h264_pps);
            if (!context->desc.h264.pps) {
               FREE(context);
               return VA_STATUS_ERROR_ALLOCATION_FAILED;
            }
            context->desc.h264.pps->sps = CALLOC_STRUCT(pipe_h264_sps);
            if (!context->desc.h264.pps->sps) {
               FREE(context->desc.h264.pps);
               FREE(context);
               return VA_STATUS_ERROR_ALLOCATION_FAILED;
            }
         }
         break;

     case PIPE_VIDEO_FORMAT_HEVC:
         if (config->entrypoint != PIPE_VIDEO_ENTRYPOINT_ENCODE) {
            context->desc.h265.pps = CALLOC_STRUCT(pipe_h265_pps);
            if (!context->desc.h265.pps) {
               FREE(context);
               return VA_STATUS_ERROR_ALLOCATION_FAILED;
            }
            context->desc.h265.pps->sps = CALLOC_STRUCT(pipe_h265_sps);
            if (!context->desc.h265.pps->sps) {
               FREE(context->desc.h265.pps);
               FREE(context);
               return VA_STATUS_ERROR_ALLOCATION_FAILED;
            }
         }
         break;

      case PIPE_VIDEO_FORMAT_VP9:
         break;

      default:
         break;
      }
   }

   context->desc.base.profile = config->profile;
   context->desc.base.entry_point = config->entrypoint;
   if (config->entrypoint == PIPE_VIDEO_ENTRYPOINT_ENCODE) {
      switch (u_reduce_video_profile(context->templat.profile)) {
      case PIPE_VIDEO_FORMAT_MPEG4_AVC:
         context->desc.h264enc.rate_ctrl[0].rate_ctrl_method = config->rc;
         context->desc.h264enc.frame_idx = util_hash_table_create_ptr_keys();
         break;
      case PIPE_VIDEO_FORMAT_HEVC:
         context->desc.h265enc.rc.rate_ctrl_method = config->rc;
         context->desc.h265enc.frame_idx = util_hash_table_create_ptr_keys();
         break;
      case PIPE_VIDEO_FORMAT_AV1:
         context->desc.av1enc.rc[0].rate_ctrl_method = config->rc;
         break;
      default:
         break;
      }
   }

   context->surfaces = _mesa_set_create(NULL, _mesa_hash_pointer, _mesa_key_pointer_equal);

   mtx_lock(&drv->mutex);
   *context_id = handle_table_add(drv->htab, context);
   mtx_unlock(&drv->mutex);

   return VA_STATUS_SUCCESS;
}

VAStatus
vlVaDestroyContext(VADriverContextP ctx, VAContextID context_id)
{
   vlVaDriver *drv;
   vlVaContext *context;

   if (!ctx)
      return VA_STATUS_ERROR_INVALID_CONTEXT;

   if (context_id == 0)
      return VA_STATUS_ERROR_INVALID_CONTEXT;

   drv = VL_VA_DRIVER(ctx);
   mtx_lock(&drv->mutex);
   context = handle_table_get(drv->htab, context_id);
   if (!context) {
      mtx_unlock(&drv->mutex);
      return VA_STATUS_ERROR_INVALID_CONTEXT;
   }

   set_foreach(context->surfaces, entry) {
      vlVaSurface *surf = (vlVaSurface *)entry->key;
      assert(surf->ctx == context);
      surf->ctx = NULL;
      if (surf->fence && context->decoder && context->decoder->destroy_fence) {
         context->decoder->destroy_fence(context->decoder, surf->fence);
         surf->fence = NULL;
      }
   }
   _mesa_set_destroy(context->surfaces, NULL);

   if (context->decoder) {
      if (context->desc.base.entry_point == PIPE_VIDEO_ENTRYPOINT_ENCODE) {
         if (u_reduce_video_profile(context->decoder->profile) ==
             PIPE_VIDEO_FORMAT_MPEG4_AVC) {
            if (context->desc.h264enc.frame_idx)
               _mesa_hash_table_destroy(context->desc.h264enc.frame_idx, NULL);
         }
         if (u_reduce_video_profile(context->decoder->profile) ==
             PIPE_VIDEO_FORMAT_HEVC) {
            if (context->desc.h265enc.frame_idx)
               _mesa_hash_table_destroy(context->desc.h265enc.frame_idx, NULL);
         }
      } else {
         if (u_reduce_video_profile(context->decoder->profile) ==
               PIPE_VIDEO_FORMAT_MPEG4_AVC) {
            FREE(context->desc.h264.pps->sps);
            FREE(context->desc.h264.pps);
         }
         if (u_reduce_video_profile(context->decoder->profile) ==
               PIPE_VIDEO_FORMAT_HEVC) {
            FREE(context->desc.h265.pps->sps);
            FREE(context->desc.h265.pps);
         }
      }
      context->decoder->destroy(context->decoder);
   }
   if (context->blit_cs)
      drv->pipe->delete_compute_state(drv->pipe, context->blit_cs);
   if (context->deint) {
      vl_deint_filter_cleanup(context->deint);
      FREE(context->deint);
   }
   FREE(context->desc.base.decrypt_key);
   FREE(context);
   handle_table_remove(drv->htab, context_id);
   mtx_unlock(&drv->mutex);

   return VA_STATUS_SUCCESS;
}

VAStatus
vlVaTerminate(VADriverContextP ctx)
{
   vlVaDriver *drv;

   if (!ctx)
      return VA_STATUS_ERROR_INVALID_CONTEXT;

   drv = ctx->pDriverData;
   vl_compositor_cleanup_state(&drv->cstate);
   vl_compositor_cleanup(&drv->compositor);
   drv->pipe->destroy(drv->pipe);
   drv->vscreen->destroy(drv->vscreen);
   handle_table_destroy(drv->htab);
   mtx_destroy(&drv->mutex);
   FREE(drv);

   return VA_STATUS_SUCCESS;
}
