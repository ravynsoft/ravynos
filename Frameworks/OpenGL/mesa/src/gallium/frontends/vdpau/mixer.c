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
#include "util/u_debug.h"

#include "vl/vl_csc.h"

#include "vdpau_private.h"

/**
 * Create a VdpVideoMixer.
 */
VdpStatus
vlVdpVideoMixerCreate(VdpDevice device,
                      uint32_t feature_count,
                      VdpVideoMixerFeature const *features,
                      uint32_t parameter_count,
                      VdpVideoMixerParameter const *parameters,
                      void const *const *parameter_values,
                      VdpVideoMixer *mixer)
{
   vlVdpVideoMixer *vmixer = NULL;
   VdpStatus ret;
   struct pipe_screen *screen;
   unsigned max_size, i;

   vlVdpDevice *dev = vlGetDataHTAB(device);
   if (!dev)
      return VDP_STATUS_INVALID_HANDLE;
   screen = dev->vscreen->pscreen;

   vmixer = CALLOC(1, sizeof(vlVdpVideoMixer));
   if (!vmixer)
      return VDP_STATUS_RESOURCES;

   DeviceReference(&vmixer->device, dev);

   mtx_lock(&dev->mutex);

   if (!vl_compositor_init_state(&vmixer->cstate, dev->context)) {
      ret = VDP_STATUS_ERROR;
      goto no_compositor_state;
   }

   vl_csc_get_matrix(VL_CSC_COLOR_STANDARD_BT_601, NULL, true, &vmixer->csc);
   if (!debug_get_bool_option("G3DVL_NO_CSC", false)) {
      if (!vl_compositor_set_csc_matrix(&vmixer->cstate, (const vl_csc_matrix *)&vmixer->csc, 1.0f, 0.0f)) {
         ret = VDP_STATUS_ERROR;
         goto err_csc_matrix;
      }
   }

   *mixer = vlAddDataHTAB(vmixer);
   if (*mixer == 0) {
      ret = VDP_STATUS_ERROR;
      goto no_handle;
   }

   ret = VDP_STATUS_INVALID_VIDEO_MIXER_FEATURE;
   for (i = 0; i < feature_count; ++i) {
      switch (features[i]) {
      /* they are valid, but we doesn't support them */
      case VDP_VIDEO_MIXER_FEATURE_DEINTERLACE_TEMPORAL_SPATIAL:
      case VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L2:
      case VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L3:
      case VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L4:
      case VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L5:
      case VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L6:
      case VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L7:
      case VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L8:
      case VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L9:
      case VDP_VIDEO_MIXER_FEATURE_INVERSE_TELECINE:
         break;

      case VDP_VIDEO_MIXER_FEATURE_DEINTERLACE_TEMPORAL:
         vmixer->deint.supported = true;
         break;

      case VDP_VIDEO_MIXER_FEATURE_SHARPNESS:
         vmixer->sharpness.supported = true;
         break;

      case VDP_VIDEO_MIXER_FEATURE_NOISE_REDUCTION:
         vmixer->noise_reduction.supported = true;
         break;

      case VDP_VIDEO_MIXER_FEATURE_LUMA_KEY:
         vmixer->luma_key.supported = true;
         break;

      case VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L1:
         vmixer->bicubic.supported = true;
         break;
      default: goto no_params;
      }
   }

   vmixer->chroma_format = PIPE_VIDEO_CHROMA_FORMAT_420;
   ret = VDP_STATUS_INVALID_VIDEO_MIXER_PARAMETER;
   for (i = 0; i < parameter_count; ++i) {
      switch (parameters[i]) {
      case VDP_VIDEO_MIXER_PARAMETER_VIDEO_SURFACE_WIDTH:
         vmixer->video_width = *(uint32_t*)parameter_values[i];
         break;
      case VDP_VIDEO_MIXER_PARAMETER_VIDEO_SURFACE_HEIGHT:
         vmixer->video_height = *(uint32_t*)parameter_values[i];
         break;
      case VDP_VIDEO_MIXER_PARAMETER_CHROMA_TYPE:
         vmixer->chroma_format = ChromaToPipe(*(VdpChromaType*)parameter_values[i]);
         break;
      case VDP_VIDEO_MIXER_PARAMETER_LAYERS:
         vmixer->max_layers = *(uint32_t*)parameter_values[i];
         break;
      default: goto no_params;
      }
   }
   ret = VDP_STATUS_INVALID_VALUE;
   if (vmixer->max_layers > 4) {
      VDPAU_MSG(VDPAU_WARN, "[VDPAU] Max layers %u > 4 not supported\n", vmixer->max_layers);
      goto no_params;
   }

   max_size = screen->get_param(screen, PIPE_CAP_MAX_TEXTURE_2D_SIZE);
   if (vmixer->video_width < 48 || vmixer->video_width > max_size) {
      VDPAU_MSG(VDPAU_WARN, "[VDPAU] 48 < %u < %u not valid for width\n",
                vmixer->video_width, max_size);
      goto no_params;
   }
   if (vmixer->video_height < 48 || vmixer->video_height > max_size) {
      VDPAU_MSG(VDPAU_WARN, "[VDPAU] 48 < %u < %u  not valid for height\n",
                vmixer->video_height, max_size);
      goto no_params;
   }
   vmixer->luma_key.luma_min = 1.0f;
   vmixer->luma_key.luma_max = 0.0f;
   mtx_unlock(&dev->mutex);

   return VDP_STATUS_OK;

no_params:
   vlRemoveDataHTAB(*mixer);

no_handle:
err_csc_matrix:
   vl_compositor_cleanup_state(&vmixer->cstate);
no_compositor_state:
   mtx_unlock(&dev->mutex);
   DeviceReference(&vmixer->device, NULL);
   FREE(vmixer);
   return ret;
}

/**
 * Destroy a VdpVideoMixer.
 */
VdpStatus
vlVdpVideoMixerDestroy(VdpVideoMixer mixer)
{
   vlVdpVideoMixer *vmixer;

   vmixer = vlGetDataHTAB(mixer);
   if (!vmixer)
      return VDP_STATUS_INVALID_HANDLE;

   mtx_lock(&vmixer->device->mutex);

   vlRemoveDataHTAB(mixer);

   vl_compositor_cleanup_state(&vmixer->cstate);

   if (vmixer->deint.filter) {
      vl_deint_filter_cleanup(vmixer->deint.filter);
      FREE(vmixer->deint.filter);
   }

   if (vmixer->noise_reduction.filter) {
      vl_median_filter_cleanup(vmixer->noise_reduction.filter);
      FREE(vmixer->noise_reduction.filter);
   }

   if (vmixer->sharpness.filter) {
      vl_matrix_filter_cleanup(vmixer->sharpness.filter);
      FREE(vmixer->sharpness.filter);
   }

   if (vmixer->bicubic.filter) {
      vl_bicubic_filter_cleanup(vmixer->bicubic.filter);
      FREE(vmixer->bicubic.filter);
   }
   mtx_unlock(&vmixer->device->mutex);
   DeviceReference(&vmixer->device, NULL);

   FREE(vmixer);

   return VDP_STATUS_OK;
}

/**
 * Perform a video post-processing and compositing operation.
 */
VdpStatus vlVdpVideoMixerRender(VdpVideoMixer mixer,
                                VdpOutputSurface background_surface,
                                VdpRect const *background_source_rect,
                                VdpVideoMixerPictureStructure current_picture_structure,
                                uint32_t video_surface_past_count,
                                VdpVideoSurface const *video_surface_past,
                                VdpVideoSurface video_surface_current,
                                uint32_t video_surface_future_count,
                                VdpVideoSurface const *video_surface_future,
                                VdpRect const *video_source_rect,
                                VdpOutputSurface destination_surface,
                                VdpRect const *destination_rect,
                                VdpRect const *destination_video_rect,
                                uint32_t layer_count,
                                VdpLayer const *layers)
{
   enum vl_compositor_deinterlace deinterlace;
   struct u_rect rect, clip, *prect, dirty_area;
   unsigned i, layer = 0;
   struct pipe_video_buffer *video_buffer;
   struct pipe_sampler_view *sampler_view, sv_templ;
   struct pipe_surface *surface, surf_templ;
   struct pipe_context *pipe = NULL;
   struct pipe_resource res_tmpl, *res;

   vlVdpVideoMixer *vmixer;
   vlVdpSurface *surf;
   vlVdpOutputSurface *dst, *bg = NULL;

   struct vl_compositor *compositor;

   vmixer = vlGetDataHTAB(mixer);
   if (!vmixer)
      return VDP_STATUS_INVALID_HANDLE;

   compositor = &vmixer->device->compositor;

   surf = vlGetDataHTAB(video_surface_current);
   if (!surf)
      return VDP_STATUS_INVALID_HANDLE;
   video_buffer = surf->video_buffer;

   if (surf->device != vmixer->device)
      return VDP_STATUS_HANDLE_DEVICE_MISMATCH;

   if (vmixer->video_width > video_buffer->width ||
       vmixer->video_height > video_buffer->height ||
       vmixer->chroma_format != pipe_format_to_chroma_format(video_buffer->buffer_format))
      return VDP_STATUS_INVALID_SIZE;

   if (layer_count > vmixer->max_layers)
      return VDP_STATUS_INVALID_VALUE;

   dst = vlGetDataHTAB(destination_surface);
   if (!dst)
      return VDP_STATUS_INVALID_HANDLE;

   if (background_surface != VDP_INVALID_HANDLE) {
      bg = vlGetDataHTAB(background_surface);
      if (!bg)
         return VDP_STATUS_INVALID_HANDLE;
   }

   mtx_lock(&vmixer->device->mutex);

   vl_compositor_clear_layers(&vmixer->cstate);

   if (bg)
      vl_compositor_set_rgba_layer(&vmixer->cstate, compositor, layer++, bg->sampler_view,
                                   RectToPipe(background_source_rect, &rect), NULL, NULL);

   switch (current_picture_structure) {
   case VDP_VIDEO_MIXER_PICTURE_STRUCTURE_TOP_FIELD:
      deinterlace = VL_COMPOSITOR_BOB_TOP;
      break;

   case VDP_VIDEO_MIXER_PICTURE_STRUCTURE_BOTTOM_FIELD:
      deinterlace = VL_COMPOSITOR_BOB_BOTTOM;
      break;

   case VDP_VIDEO_MIXER_PICTURE_STRUCTURE_FRAME:
      deinterlace = VL_COMPOSITOR_WEAVE;
      break;

   default:
      mtx_unlock(&vmixer->device->mutex);
      return VDP_STATUS_INVALID_VIDEO_MIXER_PICTURE_STRUCTURE;
   }

   if (deinterlace != VL_COMPOSITOR_WEAVE && vmixer->deint.enabled &&
       video_surface_past_count > 1 && video_surface_future_count > 0) {
      vlVdpSurface *prevprev = vlGetDataHTAB(video_surface_past[1]);
      vlVdpSurface *prev = vlGetDataHTAB(video_surface_past[0]);
      vlVdpSurface *next = vlGetDataHTAB(video_surface_future[0]);
      if (prevprev && prev && next &&
          vl_deint_filter_check_buffers(vmixer->deint.filter,
          prevprev->video_buffer, prev->video_buffer, surf->video_buffer, next->video_buffer)) {
         vl_deint_filter_render(vmixer->deint.filter, prevprev->video_buffer,
                                prev->video_buffer, surf->video_buffer,
                                next->video_buffer,
                                deinterlace == VL_COMPOSITOR_BOB_BOTTOM);
         deinterlace = VL_COMPOSITOR_WEAVE;
         video_buffer = vmixer->deint.filter->video_buffer;
      }
   }

   if (!destination_video_rect)
      destination_video_rect = video_source_rect;

   prect = RectToPipe(video_source_rect, &rect);
   if (!prect) {
      rect.x0 = 0;
      rect.y0 = 0;
      rect.x1 = surf->templat.width;
      rect.y1 = surf->templat.height;
      prect = &rect;
   }
   vl_compositor_set_buffer_layer(&vmixer->cstate, compositor, layer, video_buffer, prect, NULL, deinterlace);

   if (vmixer->bicubic.filter || vmixer->sharpness.filter || vmixer->noise_reduction.filter) {
      pipe = vmixer->device->context;
      memset(&res_tmpl, 0, sizeof(res_tmpl));

      res_tmpl.target = PIPE_TEXTURE_2D;
      res_tmpl.format = dst->sampler_view->format;
      res_tmpl.depth0 = 1;
      res_tmpl.array_size = 1;
      res_tmpl.bind = PIPE_BIND_SAMPLER_VIEW | PIPE_BIND_RENDER_TARGET;
      res_tmpl.usage = PIPE_USAGE_DEFAULT;

      if (!vmixer->bicubic.filter) {
         res_tmpl.width0 = dst->surface->width;
         res_tmpl.height0 = dst->surface->height;
      } else {
         res_tmpl.width0 = surf->templat.width;
         res_tmpl.height0 = surf->templat.height;
      }

      res = pipe->screen->resource_create(pipe->screen, &res_tmpl);

      vlVdpDefaultSamplerViewTemplate(&sv_templ, res);
      sampler_view = pipe->create_sampler_view(pipe, res, &sv_templ);

      memset(&surf_templ, 0, sizeof(surf_templ));
      surf_templ.format = res->format;
      surface = pipe->create_surface(pipe, res, &surf_templ);

      vl_compositor_reset_dirty_area(&dirty_area);
      pipe_resource_reference(&res, NULL);
   } else {
      surface = dst->surface;
      sampler_view = dst->sampler_view;
      dirty_area = dst->dirty_area;
   }

   if (!vmixer->bicubic.filter) {
      vl_compositor_set_layer_dst_area(&vmixer->cstate, layer++, RectToPipe(destination_video_rect, &rect));
      vl_compositor_set_dst_clip(&vmixer->cstate, RectToPipe(destination_rect, &clip));
   }

   for (i = 0; i < layer_count; ++i) {
      vlVdpOutputSurface *src = vlGetDataHTAB(layers->source_surface);
      if (!src) {
         mtx_unlock(&vmixer->device->mutex);
         return VDP_STATUS_INVALID_HANDLE;
      }

      assert(layers->struct_version == VDP_LAYER_VERSION);

      vl_compositor_set_rgba_layer(&vmixer->cstate, compositor, layer, src->sampler_view,
                                   RectToPipe(layers->source_rect, &rect), NULL, NULL);
      vl_compositor_set_layer_dst_area(&vmixer->cstate, layer++, RectToPipe(layers->destination_rect, &rect));

      ++layers;
   }

   vl_compositor_render(&vmixer->cstate, compositor, surface, &dirty_area, true);

   if (vmixer->noise_reduction.filter) {
      if (!vmixer->sharpness.filter && !vmixer->bicubic.filter) {
         vl_median_filter_render(vmixer->noise_reduction.filter,
                                 sampler_view, dst->surface);
      } else {
         res = pipe->screen->resource_create(pipe->screen, &res_tmpl);
         struct pipe_sampler_view *sampler_view_temp = pipe->create_sampler_view(pipe, res, &sv_templ);
         struct pipe_surface *surface_temp = pipe->create_surface(pipe, res, &surf_templ);
         pipe_resource_reference(&res, NULL);

         vl_median_filter_render(vmixer->noise_reduction.filter,
                                 sampler_view, surface_temp);

         pipe_sampler_view_reference(&sampler_view, NULL);
         pipe_surface_reference(&surface, NULL);

         sampler_view = sampler_view_temp;
         surface = surface_temp;
      }
   }

   if (vmixer->sharpness.filter) {
      if (!vmixer->bicubic.filter) {
         vl_matrix_filter_render(vmixer->sharpness.filter,
                                 sampler_view, dst->surface);
      } else {
         res = pipe->screen->resource_create(pipe->screen, &res_tmpl);
         struct pipe_sampler_view *sampler_view_temp = pipe->create_sampler_view(pipe, res, &sv_templ);
         struct pipe_surface *surface_temp = pipe->create_surface(pipe, res, &surf_templ);
         pipe_resource_reference(&res, NULL);

         vl_matrix_filter_render(vmixer->sharpness.filter,
                                 sampler_view, surface_temp);

         pipe_sampler_view_reference(&sampler_view, NULL);
         pipe_surface_reference(&surface, NULL);

         sampler_view = sampler_view_temp;
         surface = surface_temp;
      }
   }

   if (vmixer->bicubic.filter)
      vl_bicubic_filter_render(vmixer->bicubic.filter,
                               sampler_view, dst->surface,
                               RectToPipe(destination_video_rect, &rect),
                               RectToPipe(destination_rect, &clip));

   if(surface != dst->surface) {
      pipe_sampler_view_reference(&sampler_view, NULL);
      pipe_surface_reference(&surface, NULL);
   }
   mtx_unlock(&vmixer->device->mutex);

   return VDP_STATUS_OK;
}

static void
vlVdpVideoMixerUpdateDeinterlaceFilter(vlVdpVideoMixer *vmixer)
{
   struct pipe_context *pipe = vmixer->device->context;
   assert(vmixer);

   /* remove existing filter */
   if (vmixer->deint.filter) {
      vl_deint_filter_cleanup(vmixer->deint.filter);
      FREE(vmixer->deint.filter);
      vmixer->deint.filter = NULL;
   }

   /* create a new filter if requested */
   if (vmixer->deint.enabled && vmixer->chroma_format == PIPE_VIDEO_CHROMA_FORMAT_420) {
      vmixer->deint.filter = MALLOC(sizeof(struct vl_deint_filter));
      vmixer->deint.enabled = vl_deint_filter_init(vmixer->deint.filter, pipe,
            vmixer->video_width, vmixer->video_height,
            vmixer->skip_chroma_deint, vmixer->deint.spatial, false);
      if (!vmixer->deint.enabled) {
         FREE(vmixer->deint.filter);
      }
   }
}

/**
 * Update the noise reduction setting
 */
static void
vlVdpVideoMixerUpdateNoiseReductionFilter(vlVdpVideoMixer *vmixer)
{
   assert(vmixer);

   /* if present remove the old filter first */
   if (vmixer->noise_reduction.filter) {
      vl_median_filter_cleanup(vmixer->noise_reduction.filter);
      FREE(vmixer->noise_reduction.filter);
      vmixer->noise_reduction.filter = NULL;
   }

   /* and create a new filter as needed */
   if (vmixer->noise_reduction. enabled && vmixer->noise_reduction.level > 0) {
      vmixer->noise_reduction.filter = MALLOC(sizeof(struct vl_median_filter));
      vl_median_filter_init(vmixer->noise_reduction.filter, vmixer->device->context,
                            vmixer->video_width, vmixer->video_height,
                            vmixer->noise_reduction.level + 1,
                            VL_MEDIAN_FILTER_CROSS);
   }
}

static void
vlVdpVideoMixerUpdateSharpnessFilter(vlVdpVideoMixer *vmixer)
{
   assert(vmixer);

   /* if present remove the old filter first */
   if (vmixer->sharpness.filter) {
      vl_matrix_filter_cleanup(vmixer->sharpness.filter);
      FREE(vmixer->sharpness.filter);
      vmixer->sharpness.filter = NULL;
   }

   /* and create a new filter as needed */
   if (vmixer->sharpness.enabled && vmixer->sharpness.value != 0.0f) {
      float matrix[9];
      unsigned i;

      if (vmixer->sharpness.value > 0.0f) {
         matrix[0] = -1.0f; matrix[1] = -1.0f; matrix[2] = -1.0f;
         matrix[3] = -1.0f; matrix[4] =  8.0f; matrix[5] = -1.0f;
         matrix[6] = -1.0f; matrix[7] = -1.0f; matrix[8] = -1.0f;

         for (i = 0; i < 9; ++i)
            matrix[i] *= vmixer->sharpness.value;

         matrix[4] += 1.0f;

      } else {
         matrix[0] = 1.0f; matrix[1] = 2.0f; matrix[2] = 1.0f;
         matrix[3] = 2.0f; matrix[4] = 4.0f; matrix[5] = 2.0f;
         matrix[6] = 1.0f; matrix[7] = 2.0f; matrix[8] = 1.0f;

         for (i = 0; i < 9; ++i)
               matrix[i] *= fabsf(vmixer->sharpness.value) / 16.0f;

         matrix[4] += 1.0f - fabsf(vmixer->sharpness.value);
      }

      vmixer->sharpness.filter = MALLOC(sizeof(struct vl_matrix_filter));
      vl_matrix_filter_init(vmixer->sharpness.filter, vmixer->device->context,
                            vmixer->video_width, vmixer->video_height,
                            3, 3, matrix);
   }
}

/**
 * Update the bicubic filter
 */
static void
vlVdpVideoMixerUpdateBicubicFilter(vlVdpVideoMixer *vmixer)
{
   assert(vmixer);

   /* if present remove the old filter first */
   if (vmixer->bicubic.filter) {
      vl_bicubic_filter_cleanup(vmixer->bicubic.filter);
      FREE(vmixer->bicubic.filter);
      vmixer->bicubic.filter = NULL;
   }
   /* and create a new filter as needed */
   if (vmixer->bicubic.enabled) {
      vmixer->bicubic.filter = MALLOC(sizeof(struct vl_bicubic_filter));
      vl_bicubic_filter_init(vmixer->bicubic.filter, vmixer->device->context,
                            vmixer->video_width, vmixer->video_height);
   }
}

/**
 * Retrieve whether features were requested at creation time.
 */
VdpStatus
vlVdpVideoMixerGetFeatureSupport(VdpVideoMixer mixer,
                                 uint32_t feature_count,
                                 VdpVideoMixerFeature const *features,
                                 VdpBool *feature_supports)
{
   vlVdpVideoMixer *vmixer;
   unsigned i;

   if (!(features && feature_supports))
      return VDP_STATUS_INVALID_POINTER;

   vmixer = vlGetDataHTAB(mixer);
   if (!vmixer)
      return VDP_STATUS_INVALID_HANDLE;

   for (i = 0; i < feature_count; ++i) {
      switch (features[i]) {
      /* they are valid, but we doesn't support them */
      case VDP_VIDEO_MIXER_FEATURE_DEINTERLACE_TEMPORAL_SPATIAL:
      case VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L2:
      case VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L3:
      case VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L4:
      case VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L5:
      case VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L6:
      case VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L7:
      case VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L8:
      case VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L9:
      case VDP_VIDEO_MIXER_FEATURE_INVERSE_TELECINE:
         feature_supports[i] = false;
         break;

      case VDP_VIDEO_MIXER_FEATURE_DEINTERLACE_TEMPORAL:
         feature_supports[i] = vmixer->deint.supported;
         break;

      case VDP_VIDEO_MIXER_FEATURE_SHARPNESS:
         feature_supports[i] = vmixer->sharpness.supported;
         break;

      case VDP_VIDEO_MIXER_FEATURE_NOISE_REDUCTION:
         feature_supports[i] = vmixer->noise_reduction.supported;
         break;

      case VDP_VIDEO_MIXER_FEATURE_LUMA_KEY:
         feature_supports[i] = vmixer->luma_key.supported;
         break;

      case VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L1:
         feature_supports[i] = vmixer->bicubic.supported;
         break;

      default:
         return VDP_STATUS_INVALID_VIDEO_MIXER_FEATURE;
      }
   }

   return VDP_STATUS_OK;
}

/**
 * Enable or disable features.
 */
VdpStatus
vlVdpVideoMixerSetFeatureEnables(VdpVideoMixer mixer,
                                 uint32_t feature_count,
                                 VdpVideoMixerFeature const *features,
                                 VdpBool const *feature_enables)
{
   vlVdpVideoMixer *vmixer;
   unsigned i;

   if (!(features && feature_enables))
      return VDP_STATUS_INVALID_POINTER;

   vmixer = vlGetDataHTAB(mixer);
   if (!vmixer)
      return VDP_STATUS_INVALID_HANDLE;

   mtx_lock(&vmixer->device->mutex);
   for (i = 0; i < feature_count; ++i) {
      switch (features[i]) {
      /* they are valid, but we doesn't support them */
      case VDP_VIDEO_MIXER_FEATURE_DEINTERLACE_TEMPORAL_SPATIAL:
      case VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L2:
      case VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L3:
      case VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L4:
      case VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L5:
      case VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L6:
      case VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L7:
      case VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L8:
      case VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L9:
      case VDP_VIDEO_MIXER_FEATURE_INVERSE_TELECINE:
         break;

      case VDP_VIDEO_MIXER_FEATURE_DEINTERLACE_TEMPORAL:
         vmixer->deint.enabled = feature_enables[i];
         vlVdpVideoMixerUpdateDeinterlaceFilter(vmixer);
         break;

      case VDP_VIDEO_MIXER_FEATURE_SHARPNESS:
         vmixer->sharpness.enabled = feature_enables[i];
         vlVdpVideoMixerUpdateSharpnessFilter(vmixer);
         break;

      case VDP_VIDEO_MIXER_FEATURE_NOISE_REDUCTION:
         vmixer->noise_reduction.enabled = feature_enables[i];
         vlVdpVideoMixerUpdateNoiseReductionFilter(vmixer);
         break;

      case VDP_VIDEO_MIXER_FEATURE_LUMA_KEY:
         vmixer->luma_key.enabled = feature_enables[i];
         if (!debug_get_bool_option("G3DVL_NO_CSC", false))
            if (!vl_compositor_set_csc_matrix(&vmixer->cstate, (const vl_csc_matrix *)&vmixer->csc,
                        vmixer->luma_key.luma_min, vmixer->luma_key.luma_max)) {
               mtx_unlock(&vmixer->device->mutex);
               return VDP_STATUS_ERROR;
            }
         break;

      case VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L1:
         vmixer->bicubic.enabled = feature_enables[i];
         vlVdpVideoMixerUpdateBicubicFilter(vmixer);
         break;

      default:
         mtx_unlock(&vmixer->device->mutex);
         return VDP_STATUS_INVALID_VIDEO_MIXER_FEATURE;
      }
   }
   mtx_unlock(&vmixer->device->mutex);

   return VDP_STATUS_OK;
}

/**
 * Retrieve whether features are enabled.
 */
VdpStatus
vlVdpVideoMixerGetFeatureEnables(VdpVideoMixer mixer,
                                 uint32_t feature_count,
                                 VdpVideoMixerFeature const *features,
                                 VdpBool *feature_enables)
{
   vlVdpVideoMixer *vmixer;
   unsigned i;

   if (!(features && feature_enables))
      return VDP_STATUS_INVALID_POINTER;

   vmixer = vlGetDataHTAB(mixer);
   if (!vmixer)
      return VDP_STATUS_INVALID_HANDLE;

   for (i = 0; i < feature_count; ++i) {
      switch (features[i]) {
      /* they are valid, but we doesn't support them */
      case VDP_VIDEO_MIXER_FEATURE_DEINTERLACE_TEMPORAL:
      case VDP_VIDEO_MIXER_FEATURE_DEINTERLACE_TEMPORAL_SPATIAL:
      case VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L2:
      case VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L3:
      case VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L4:
      case VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L5:
      case VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L6:
      case VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L7:
      case VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L8:
      case VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L9:
      case VDP_VIDEO_MIXER_FEATURE_INVERSE_TELECINE:
         break;

      case VDP_VIDEO_MIXER_FEATURE_SHARPNESS:
         feature_enables[i] = vmixer->sharpness.enabled;
         break;

      case VDP_VIDEO_MIXER_FEATURE_NOISE_REDUCTION:
         feature_enables[i] = vmixer->noise_reduction.enabled;
         break;

      case VDP_VIDEO_MIXER_FEATURE_LUMA_KEY:
         feature_enables[i] = vmixer->luma_key.enabled;
         break;

      case VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L1:
         feature_enables[i] = vmixer->bicubic.enabled;
         break;

      default:
         return VDP_STATUS_INVALID_VIDEO_MIXER_FEATURE;
      }
   }

   return VDP_STATUS_OK;
}

/**
 * Set attribute values.
 */
VdpStatus
vlVdpVideoMixerSetAttributeValues(VdpVideoMixer mixer,
                                  uint32_t attribute_count,
                                  VdpVideoMixerAttribute const *attributes,
                                  void const *const *attribute_values)
{
   const VdpColor *background_color;
   union pipe_color_union color;
   const float *vdp_csc;
   float val;
   unsigned i;
   VdpStatus ret;

   if (!(attributes && attribute_values))
      return VDP_STATUS_INVALID_POINTER;

   vlVdpVideoMixer *vmixer = vlGetDataHTAB(mixer);
   if (!vmixer)
      return VDP_STATUS_INVALID_HANDLE;

   mtx_lock(&vmixer->device->mutex);
   for (i = 0; i < attribute_count; ++i) {
      switch (attributes[i]) {
      case VDP_VIDEO_MIXER_ATTRIBUTE_BACKGROUND_COLOR:
         background_color = attribute_values[i];
         color.f[0] = background_color->red;
         color.f[1] = background_color->green;
         color.f[2] = background_color->blue;
         color.f[3] = background_color->alpha;
         vl_compositor_set_clear_color(&vmixer->cstate, &color);
         break;
      case VDP_VIDEO_MIXER_ATTRIBUTE_CSC_MATRIX:
         vdp_csc = attribute_values[i];
         vmixer->custom_csc = !!vdp_csc;
         if (!vdp_csc)
            vl_csc_get_matrix(VL_CSC_COLOR_STANDARD_BT_601, NULL, 1, &vmixer->csc);
         else
            memcpy(vmixer->csc, vdp_csc, sizeof(vl_csc_matrix));
         if (!debug_get_bool_option("G3DVL_NO_CSC", false))
            if (!vl_compositor_set_csc_matrix(&vmixer->cstate, (const vl_csc_matrix *)&vmixer->csc,
                                         vmixer->luma_key.luma_min, vmixer->luma_key.luma_max)) {
               ret = VDP_STATUS_ERROR;
               goto fail;
            }
         break;

      case VDP_VIDEO_MIXER_ATTRIBUTE_NOISE_REDUCTION_LEVEL:

         val = *(float*)attribute_values[i];
         if (val < 0.0f || val > 1.0f) {
            ret = VDP_STATUS_INVALID_VALUE;
            goto fail;
         }

         vmixer->noise_reduction.level = val * 10;
         vlVdpVideoMixerUpdateNoiseReductionFilter(vmixer);
         break;

      case VDP_VIDEO_MIXER_ATTRIBUTE_LUMA_KEY_MIN_LUMA:
         val = *(float*)attribute_values[i];
         if (val < 0.0f || val > 1.0f) {
            ret = VDP_STATUS_INVALID_VALUE;
            goto fail;
         }
         vmixer->luma_key.luma_min = val;
         if (!debug_get_bool_option("G3DVL_NO_CSC", false))
            if (!vl_compositor_set_csc_matrix(&vmixer->cstate, (const vl_csc_matrix *)&vmixer->csc,
                        vmixer->luma_key.luma_min, vmixer->luma_key.luma_max)) {
               ret = VDP_STATUS_ERROR;
               goto fail;
            }
         break;

      case VDP_VIDEO_MIXER_ATTRIBUTE_LUMA_KEY_MAX_LUMA:
         val = *(float*)attribute_values[i];
         if (val < 0.0f || val > 1.0f) {
            ret = VDP_STATUS_INVALID_VALUE;
            goto fail;
         }
         vmixer->luma_key.luma_max = val;
         if (!debug_get_bool_option("G3DVL_NO_CSC", false))
            if (!vl_compositor_set_csc_matrix(&vmixer->cstate, (const vl_csc_matrix *)&vmixer->csc,
                        vmixer->luma_key.luma_min, vmixer->luma_key.luma_max)) {
               ret = VDP_STATUS_ERROR;
               goto fail;
            }
         break;

      case VDP_VIDEO_MIXER_ATTRIBUTE_SHARPNESS_LEVEL:

         val = *(float*)attribute_values[i];
         if (val < -1.0f || val > 1.0f) {
            ret = VDP_STATUS_INVALID_VALUE;
            goto fail;
         }

         vmixer->sharpness.value = val;
         vlVdpVideoMixerUpdateSharpnessFilter(vmixer);
         break;

      case VDP_VIDEO_MIXER_ATTRIBUTE_SKIP_CHROMA_DEINTERLACE:
         if (*(uint8_t*)attribute_values[i] > 1) {
            ret = VDP_STATUS_INVALID_VALUE;
            goto fail;
         }
         vmixer->skip_chroma_deint = *(uint8_t*)attribute_values[i];
         vlVdpVideoMixerUpdateDeinterlaceFilter(vmixer);
         break;
      default:
         ret = VDP_STATUS_INVALID_VIDEO_MIXER_ATTRIBUTE;
         goto fail;
      }
   }
   mtx_unlock(&vmixer->device->mutex);

   return VDP_STATUS_OK;
fail:
   mtx_unlock(&vmixer->device->mutex);
   return ret;
}

/**
 * Retrieve parameter values given at creation time.
 */
VdpStatus
vlVdpVideoMixerGetParameterValues(VdpVideoMixer mixer,
                                  uint32_t parameter_count,
                                  VdpVideoMixerParameter const *parameters,
                                  void *const *parameter_values)
{
   vlVdpVideoMixer *vmixer = vlGetDataHTAB(mixer);
   unsigned i;
   if (!vmixer)
      return VDP_STATUS_INVALID_HANDLE;

   if (!parameter_count)
      return VDP_STATUS_OK;
   if (!(parameters && parameter_values))
      return VDP_STATUS_INVALID_POINTER;
   for (i = 0; i < parameter_count; ++i) {
      switch (parameters[i]) {
      case VDP_VIDEO_MIXER_PARAMETER_VIDEO_SURFACE_WIDTH:
         *(uint32_t*)parameter_values[i] = vmixer->video_width;
         break;
      case VDP_VIDEO_MIXER_PARAMETER_VIDEO_SURFACE_HEIGHT:
         *(uint32_t*)parameter_values[i] = vmixer->video_height;
         break;
      case VDP_VIDEO_MIXER_PARAMETER_CHROMA_TYPE:
         *(VdpChromaType*)parameter_values[i] = PipeToChroma(vmixer->chroma_format);
         break;
      case VDP_VIDEO_MIXER_PARAMETER_LAYERS:
         *(uint32_t*)parameter_values[i] = vmixer->max_layers;
         break;
      default:
         return VDP_STATUS_INVALID_VIDEO_MIXER_PARAMETER;
      }
   }
   return VDP_STATUS_OK;
}

/**
 * Retrieve current attribute values.
 */
VdpStatus
vlVdpVideoMixerGetAttributeValues(VdpVideoMixer mixer,
                                  uint32_t attribute_count,
                                  VdpVideoMixerAttribute const *attributes,
                                  void *const *attribute_values)
{
   unsigned i;
   VdpCSCMatrix **vdp_csc;

   if (!(attributes && attribute_values))
      return VDP_STATUS_INVALID_POINTER;

   vlVdpVideoMixer *vmixer = vlGetDataHTAB(mixer);
   if (!vmixer)
      return VDP_STATUS_INVALID_HANDLE;

   mtx_lock(&vmixer->device->mutex);
   for (i = 0; i < attribute_count; ++i) {
      switch (attributes[i]) {
      case VDP_VIDEO_MIXER_ATTRIBUTE_BACKGROUND_COLOR:
         vl_compositor_get_clear_color(&vmixer->cstate, attribute_values[i]);
         break;
      case VDP_VIDEO_MIXER_ATTRIBUTE_CSC_MATRIX:
         vdp_csc = attribute_values[i];
         if (!vmixer->custom_csc) {
             *vdp_csc = NULL;
            break;
         }
         memcpy(*vdp_csc, vmixer->csc, sizeof(float)*12);
         break;

      case VDP_VIDEO_MIXER_ATTRIBUTE_NOISE_REDUCTION_LEVEL:
         *(float*)attribute_values[i] = (float)vmixer->noise_reduction.level / 10.0f;
         break;

      case VDP_VIDEO_MIXER_ATTRIBUTE_LUMA_KEY_MIN_LUMA:
         *(float*)attribute_values[i] = vmixer->luma_key.luma_min;
         break;
      case VDP_VIDEO_MIXER_ATTRIBUTE_LUMA_KEY_MAX_LUMA:
         *(float*)attribute_values[i] = vmixer->luma_key.luma_max;
         break;
      case VDP_VIDEO_MIXER_ATTRIBUTE_SHARPNESS_LEVEL:
         *(float*)attribute_values[i] = vmixer->sharpness.value;
         break;
      case VDP_VIDEO_MIXER_ATTRIBUTE_SKIP_CHROMA_DEINTERLACE:
         *(uint8_t*)attribute_values[i] = vmixer->skip_chroma_deint;
         break;
      default:
         mtx_unlock(&vmixer->device->mutex);
         return VDP_STATUS_INVALID_VIDEO_MIXER_ATTRIBUTE;
      }
   }
   mtx_unlock(&vmixer->device->mutex);
   return VDP_STATUS_OK;
}

/**
 * Generate a color space conversion matrix.
 */
VdpStatus
vlVdpGenerateCSCMatrix(VdpProcamp *procamp,
                       VdpColorStandard standard,
                       VdpCSCMatrix *csc_matrix)
{
   enum VL_CSC_COLOR_STANDARD vl_std;
   struct vl_procamp camp;

   if (!csc_matrix)
      return VDP_STATUS_INVALID_POINTER;

   switch (standard) {
      case VDP_COLOR_STANDARD_ITUR_BT_601: vl_std = VL_CSC_COLOR_STANDARD_BT_601; break;
      case VDP_COLOR_STANDARD_ITUR_BT_709: vl_std = VL_CSC_COLOR_STANDARD_BT_709; break;
      case VDP_COLOR_STANDARD_SMPTE_240M:  vl_std = VL_CSC_COLOR_STANDARD_SMPTE_240M; break;
      default: return VDP_STATUS_INVALID_COLOR_STANDARD;
   }

   if (procamp) {
      if (procamp->struct_version > VDP_PROCAMP_VERSION)
         return VDP_STATUS_INVALID_STRUCT_VERSION;
      camp.brightness = procamp->brightness;
      camp.contrast = procamp->contrast;
      camp.saturation = procamp->saturation;
      camp.hue = procamp->hue;
   }

   vl_csc_get_matrix(vl_std, procamp ? &camp : NULL, true, csc_matrix);
   return VDP_STATUS_OK;
}
