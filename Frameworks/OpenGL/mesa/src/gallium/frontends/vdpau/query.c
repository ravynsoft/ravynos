/**************************************************************************
 *
 * Copyright 2010 Younes Manton.
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
#include <math.h>

#include "vdpau_private.h"
#include "pipe/p_screen.h"
#include "pipe/p_defines.h"
#include "util/u_debug.h"

#include "vl/vl_codec.h"

/**
 * Retrieve the VDPAU version implemented by the backend.
 */
VdpStatus
vlVdpGetApiVersion(uint32_t *api_version)
{
   if (!api_version)
      return VDP_STATUS_INVALID_POINTER;

   *api_version = 1;
   return VDP_STATUS_OK;
}

/**
 * Retrieve an implementation-specific string description of the implementation.
 * This typically includes detailed version information.
 */
VdpStatus
vlVdpGetInformationString(char const **information_string)
{
   if (!information_string)
      return VDP_STATUS_INVALID_POINTER;

   *information_string = INFORMATION_STRING;
   return VDP_STATUS_OK;
}

/**
 * Query the implementation's VdpVideoSurface capabilities.
 */
VdpStatus
vlVdpVideoSurfaceQueryCapabilities(VdpDevice device, VdpChromaType surface_chroma_type,
                                   VdpBool *is_supported, uint32_t *max_width, uint32_t *max_height)
{
   vlVdpDevice *dev;
   struct pipe_screen *pscreen;
   uint32_t max_2d_texture_size;

   if (!(is_supported && max_width && max_height))
      return VDP_STATUS_INVALID_POINTER;

   dev = vlGetDataHTAB(device);
   if (!dev)
      return VDP_STATUS_INVALID_HANDLE;

   pscreen = dev->vscreen->pscreen;
   if (!pscreen)
      return VDP_STATUS_RESOURCES;

   mtx_lock(&dev->mutex);

   /* XXX: Current limits */
   *is_supported = true;
   max_2d_texture_size = pscreen->get_param(pscreen, PIPE_CAP_MAX_TEXTURE_2D_SIZE);
   mtx_unlock(&dev->mutex);
   if (!max_2d_texture_size)
      return VDP_STATUS_RESOURCES;

   *max_width = *max_height = max_2d_texture_size;

   return VDP_STATUS_OK;
}

/**
 * Query the implementation's VdpVideoSurface GetBits/PutBits capabilities.
 */
VdpStatus
vlVdpVideoSurfaceQueryGetPutBitsYCbCrCapabilities(VdpDevice device, VdpChromaType surface_chroma_type,
                                                  VdpYCbCrFormat bits_ycbcr_format,
                                                  VdpBool *is_supported)
{
   vlVdpDevice *dev;
   struct pipe_screen *pscreen;
   VdpYCbCrFormat ycbcrFormat;
   bool supported;

   if (!is_supported)
      return VDP_STATUS_INVALID_POINTER;

   dev = vlGetDataHTAB(device);
   if (!dev)
      return VDP_STATUS_INVALID_HANDLE;

   pscreen = dev->vscreen->pscreen;
   if (!pscreen)
      return VDP_STATUS_RESOURCES;

   mtx_lock(&dev->mutex);

   ycbcrFormat = bits_ycbcr_format;
   switch(bits_ycbcr_format) {
   case VDP_YCBCR_FORMAT_NV12:
      supported = surface_chroma_type == VDP_CHROMA_TYPE_420;
      break;

   case VDP_YCBCR_FORMAT_YV12:
      supported = surface_chroma_type == VDP_CHROMA_TYPE_420;

      /* We can convert YV12 to NV12 on the fly! */
      ycbcrFormat = VDP_YCBCR_FORMAT_NV12;
      break;

   case VDP_YCBCR_FORMAT_UYVY:
   case VDP_YCBCR_FORMAT_YUYV:
      supported = surface_chroma_type == VDP_CHROMA_TYPE_422;
      break;

   case VDP_YCBCR_FORMAT_Y8U8V8A8:
   case VDP_YCBCR_FORMAT_V8U8Y8A8:
      supported = surface_chroma_type == VDP_CHROMA_TYPE_444;
      break;

   case VDP_YCBCR_FORMAT_P010:
   case VDP_YCBCR_FORMAT_P016:
      /* Do any other profiles imply support for this chroma type? */
      supported = (surface_chroma_type == VDP_CHROMA_TYPE_420_16)
                  && vl_codec_supported(pscreen, PIPE_VIDEO_PROFILE_HEVC_MAIN_10, false);
      break;

   default:
      supported = false;
      break;
   }

   if (supported &&
       !pscreen->is_video_format_supported(pscreen,
                                           FormatYCBCRToPipe(ycbcrFormat),
                                           PIPE_VIDEO_PROFILE_UNKNOWN,
                                           PIPE_VIDEO_ENTRYPOINT_BITSTREAM)) {
      supported = false;
   }
   *is_supported = supported;

   mtx_unlock(&dev->mutex);

   return VDP_STATUS_OK;
}

/**
 * Query the implementation's VdpDecoder capabilities.
 */
VdpStatus
vlVdpDecoderQueryCapabilities(VdpDevice device, VdpDecoderProfile profile,
                              VdpBool *is_supported, uint32_t *max_level, uint32_t *max_macroblocks,
                              uint32_t *max_width, uint32_t *max_height)
{
   vlVdpDevice *dev;
   struct pipe_screen *pscreen;
   enum pipe_video_profile p_profile;

   if (!(is_supported && max_level && max_macroblocks && max_width && max_height))
      return VDP_STATUS_INVALID_POINTER;

   dev = vlGetDataHTAB(device);
   if (!dev)
      return VDP_STATUS_INVALID_HANDLE;

   pscreen = dev->vscreen->pscreen;
   if (!pscreen)
      return VDP_STATUS_RESOURCES;

   p_profile = ProfileToPipe(profile);
   if (p_profile == PIPE_VIDEO_PROFILE_UNKNOWN)	{
      *is_supported = false;
      return VDP_STATUS_OK;
   }

   mtx_lock(&dev->mutex);
   *is_supported = vl_codec_supported(pscreen, p_profile, false);
   if (*is_supported) {
      *max_width = pscreen->get_video_param(pscreen, p_profile, PIPE_VIDEO_ENTRYPOINT_BITSTREAM,
                                            PIPE_VIDEO_CAP_MAX_WIDTH);
      *max_height = pscreen->get_video_param(pscreen, p_profile, PIPE_VIDEO_ENTRYPOINT_BITSTREAM,
                                             PIPE_VIDEO_CAP_MAX_HEIGHT);
      *max_level = pscreen->get_video_param(pscreen, p_profile, PIPE_VIDEO_ENTRYPOINT_BITSTREAM,
                                            PIPE_VIDEO_CAP_MAX_LEVEL);
      *max_macroblocks = pscreen->get_video_param(pscreen, p_profile, PIPE_VIDEO_ENTRYPOINT_BITSTREAM,
                                            PIPE_VIDEO_CAP_MAX_MACROBLOCKS);
      if (*max_macroblocks == 0) {
         *max_macroblocks = (*max_width/16)*(*max_height/16);
      }
   } else {
      *max_width = 0;
      *max_height = 0;
      *max_level = 0;
      *max_macroblocks = 0;
   }
   mtx_unlock(&dev->mutex);

   return VDP_STATUS_OK;
}

/**
 * Query the implementation's VdpOutputSurface capabilities.
 */
VdpStatus
vlVdpOutputSurfaceQueryCapabilities(VdpDevice device, VdpRGBAFormat surface_rgba_format,
                                    VdpBool *is_supported, uint32_t *max_width, uint32_t *max_height)
{
   vlVdpDevice *dev;
   struct pipe_screen *pscreen;
   enum pipe_format format;

   dev = vlGetDataHTAB(device);
   if (!dev)
      return VDP_STATUS_INVALID_HANDLE;

   pscreen = dev->vscreen->pscreen;
   if (!pscreen)
      return VDP_STATUS_RESOURCES;

   format = VdpFormatRGBAToPipe(surface_rgba_format);
   if (format == PIPE_FORMAT_NONE || format == PIPE_FORMAT_A8_UNORM)
      return VDP_STATUS_INVALID_RGBA_FORMAT;

   if (!(is_supported && max_width && max_height))
      return VDP_STATUS_INVALID_POINTER;

   mtx_lock(&dev->mutex);
   *is_supported = pscreen->is_format_supported
   (
      pscreen, format, PIPE_TEXTURE_2D, 1, 1,
      PIPE_BIND_SAMPLER_VIEW | PIPE_BIND_RENDER_TARGET
   );
   if (*is_supported) {
      uint32_t max_2d_texture_size = pscreen->get_param(
         pscreen, PIPE_CAP_MAX_TEXTURE_2D_SIZE);

      if (!max_2d_texture_size) {
         mtx_unlock(&dev->mutex);
         return VDP_STATUS_ERROR;
      }

      *max_width = *max_height = max_2d_texture_size;
   } else {
      *max_width = 0;
      *max_height = 0;
   }
   mtx_unlock(&dev->mutex);

   return VDP_STATUS_OK;
}

/**
 * Query the implementation's capability to perform a PutBits operation using
 * application data matching the surface's format.
 */
VdpStatus
vlVdpOutputSurfaceQueryGetPutBitsNativeCapabilities(VdpDevice device, VdpRGBAFormat surface_rgba_format,
                                                    VdpBool *is_supported)
{
   vlVdpDevice *dev;
   struct pipe_screen *pscreen;
   enum pipe_format format;

   dev = vlGetDataHTAB(device);
   if (!dev)
      return VDP_STATUS_INVALID_HANDLE;

   pscreen = dev->vscreen->pscreen;
   if (!pscreen)
      return VDP_STATUS_ERROR;

   format = VdpFormatRGBAToPipe(surface_rgba_format);
   if (format == PIPE_FORMAT_NONE || format == PIPE_FORMAT_A8_UNORM)
      return VDP_STATUS_INVALID_RGBA_FORMAT;

   if (!is_supported)
      return VDP_STATUS_INVALID_POINTER;

   mtx_lock(&dev->mutex);
   *is_supported = pscreen->is_format_supported
   (
      pscreen, format, PIPE_TEXTURE_2D, 1, 1,
      PIPE_BIND_SAMPLER_VIEW | PIPE_BIND_RENDER_TARGET
   );
   mtx_unlock(&dev->mutex);

   return VDP_STATUS_OK;
}

/**
 * Query the implementation's capability to perform a PutBits operation using
 * application data in a specific indexed format.
 */
VdpStatus
vlVdpOutputSurfaceQueryPutBitsIndexedCapabilities(VdpDevice device,
                                                  VdpRGBAFormat surface_rgba_format,
                                                  VdpIndexedFormat bits_indexed_format,
                                                  VdpColorTableFormat color_table_format,
                                                  VdpBool *is_supported)
{
   vlVdpDevice *dev;
   struct pipe_screen *pscreen;
   enum pipe_format rgba_format, index_format, colortbl_format;

   dev = vlGetDataHTAB(device);
   if (!dev)
      return VDP_STATUS_INVALID_HANDLE;

   pscreen = dev->vscreen->pscreen;
   if (!pscreen)
      return VDP_STATUS_ERROR;

   rgba_format = VdpFormatRGBAToPipe(surface_rgba_format);
   if (rgba_format == PIPE_FORMAT_NONE || rgba_format == PIPE_FORMAT_A8_UNORM)
      return VDP_STATUS_INVALID_RGBA_FORMAT;

   index_format = FormatIndexedToPipe(bits_indexed_format);
   if (index_format == PIPE_FORMAT_NONE)
       return VDP_STATUS_INVALID_INDEXED_FORMAT;

   colortbl_format = FormatColorTableToPipe(color_table_format);
   if (colortbl_format == PIPE_FORMAT_NONE)
       return VDP_STATUS_INVALID_COLOR_TABLE_FORMAT;

   if (!is_supported)
      return VDP_STATUS_INVALID_POINTER;

   mtx_lock(&dev->mutex);
   *is_supported = pscreen->is_format_supported
   (
      pscreen, rgba_format, PIPE_TEXTURE_2D, 1, 1,
      PIPE_BIND_SAMPLER_VIEW | PIPE_BIND_RENDER_TARGET
   );

   *is_supported &= pscreen->is_format_supported
   (
      pscreen, index_format, PIPE_TEXTURE_2D, 1, 1,
      PIPE_BIND_SAMPLER_VIEW
   );

   *is_supported &= pscreen->is_format_supported
   (
      pscreen, colortbl_format, PIPE_TEXTURE_1D, 1, 1,
      PIPE_BIND_SAMPLER_VIEW
   );
   mtx_unlock(&dev->mutex);

   return VDP_STATUS_OK;
}

/**
 * Query the implementation's capability to perform a PutBits operation using
 * application data in a specific YCbCr/YUB format.
 */
VdpStatus
vlVdpOutputSurfaceQueryPutBitsYCbCrCapabilities(VdpDevice device, VdpRGBAFormat surface_rgba_format,
                                                VdpYCbCrFormat bits_ycbcr_format,
                                                VdpBool *is_supported)
{
   vlVdpDevice *dev;
   struct pipe_screen *pscreen;
   enum pipe_format rgba_format, ycbcr_format;

   dev = vlGetDataHTAB(device);
   if (!dev)
      return VDP_STATUS_INVALID_HANDLE;

   pscreen = dev->vscreen->pscreen;
   if (!pscreen)
      return VDP_STATUS_ERROR;

   rgba_format = VdpFormatRGBAToPipe(surface_rgba_format);
   if (rgba_format == PIPE_FORMAT_NONE || rgba_format == PIPE_FORMAT_A8_UNORM)
      return VDP_STATUS_INVALID_RGBA_FORMAT;

   ycbcr_format = FormatYCBCRToPipe(bits_ycbcr_format);
   if (ycbcr_format == PIPE_FORMAT_NONE)
       return VDP_STATUS_INVALID_INDEXED_FORMAT;

   if (!is_supported)
      return VDP_STATUS_INVALID_POINTER;

   mtx_lock(&dev->mutex);
   *is_supported = pscreen->is_format_supported
   (
      pscreen, rgba_format, PIPE_TEXTURE_2D, 1, 1,
      PIPE_BIND_SAMPLER_VIEW | PIPE_BIND_RENDER_TARGET
   );

   *is_supported &= pscreen->is_video_format_supported
   (
      pscreen, ycbcr_format,
      PIPE_VIDEO_PROFILE_UNKNOWN,
      PIPE_VIDEO_ENTRYPOINT_BITSTREAM
   );
   mtx_unlock(&dev->mutex);

   return VDP_STATUS_OK;
}

/**
 * Query the implementation's VdpBitmapSurface capabilities.
 */
VdpStatus
vlVdpBitmapSurfaceQueryCapabilities(VdpDevice device, VdpRGBAFormat surface_rgba_format,
                                    VdpBool *is_supported, uint32_t *max_width, uint32_t *max_height)
{
   vlVdpDevice *dev;
   struct pipe_screen *pscreen;
   enum pipe_format format;

   dev = vlGetDataHTAB(device);
   if (!dev)
      return VDP_STATUS_INVALID_HANDLE;

   pscreen = dev->vscreen->pscreen;
   if (!pscreen)
      return VDP_STATUS_RESOURCES;

   format = VdpFormatRGBAToPipe(surface_rgba_format);
   if (format == PIPE_FORMAT_NONE)
      return VDP_STATUS_INVALID_RGBA_FORMAT;

   if (!(is_supported && max_width && max_height))
      return VDP_STATUS_INVALID_POINTER;

   mtx_lock(&dev->mutex);
   *is_supported = pscreen->is_format_supported
   (
      pscreen, format, PIPE_TEXTURE_2D, 1, 1,
      PIPE_BIND_SAMPLER_VIEW | PIPE_BIND_RENDER_TARGET
   );
   if (*is_supported) {
      uint32_t max_2d_texture_size = pscreen->get_param(
         pscreen, PIPE_CAP_MAX_TEXTURE_2D_SIZE);

      if (!max_2d_texture_size) {
         mtx_unlock(&dev->mutex);
         return VDP_STATUS_ERROR;
      }

      *max_width = *max_height = max_2d_texture_size;
   } else {
      *max_width = 0;
      *max_height = 0;
   }
   mtx_unlock(&dev->mutex);

   return VDP_STATUS_OK;
}

/**
 * Query the implementation's support for a specific feature.
 */
VdpStatus
vlVdpVideoMixerQueryFeatureSupport(VdpDevice device, VdpVideoMixerFeature feature,
                                   VdpBool *is_supported)
{
   if (!is_supported)
      return VDP_STATUS_INVALID_POINTER;

   switch (feature) {
   case VDP_VIDEO_MIXER_FEATURE_SHARPNESS:
   case VDP_VIDEO_MIXER_FEATURE_NOISE_REDUCTION:
   case VDP_VIDEO_MIXER_FEATURE_DEINTERLACE_TEMPORAL:
   case VDP_VIDEO_MIXER_FEATURE_LUMA_KEY:
   case VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L1:
      *is_supported = VDP_TRUE;
      break;
   default:
      *is_supported = VDP_FALSE;
      break;
   }
   return VDP_STATUS_OK;
}

/**
 * Query the implementation's support for a specific parameter.
 */
VdpStatus
vlVdpVideoMixerQueryParameterSupport(VdpDevice device, VdpVideoMixerParameter parameter,
                                     VdpBool *is_supported)
{
   if (!is_supported)
      return VDP_STATUS_INVALID_POINTER;

   switch (parameter) {
   case VDP_VIDEO_MIXER_PARAMETER_VIDEO_SURFACE_WIDTH:
   case VDP_VIDEO_MIXER_PARAMETER_VIDEO_SURFACE_HEIGHT:
   case VDP_VIDEO_MIXER_PARAMETER_CHROMA_TYPE:
   case VDP_VIDEO_MIXER_PARAMETER_LAYERS:
      *is_supported = VDP_TRUE;
      break;
   default:
      *is_supported = VDP_FALSE;
      break;
   }
   return VDP_STATUS_OK;
}

/**
 * Query the implementation's supported for a specific parameter.
 */
VdpStatus
vlVdpVideoMixerQueryParameterValueRange(VdpDevice device, VdpVideoMixerParameter parameter,
                                        void *min_value, void *max_value)
{
   vlVdpDevice *dev = vlGetDataHTAB(device);
   struct pipe_screen *screen;

   if (!dev)
      return VDP_STATUS_INVALID_HANDLE;
   if (!(min_value && max_value))
      return VDP_STATUS_INVALID_POINTER;

   mtx_lock(&dev->mutex);
   screen = dev->vscreen->pscreen;
   switch (parameter) {
   case VDP_VIDEO_MIXER_PARAMETER_VIDEO_SURFACE_WIDTH:
      *(uint32_t*)min_value = 48;
      *(uint32_t*)max_value = screen->get_video_param(screen, PIPE_VIDEO_PROFILE_UNKNOWN,
                                                      PIPE_VIDEO_ENTRYPOINT_BITSTREAM,
                                                      PIPE_VIDEO_CAP_MAX_WIDTH);
      break;
   case VDP_VIDEO_MIXER_PARAMETER_VIDEO_SURFACE_HEIGHT:
      *(uint32_t*)min_value = 48;
      *(uint32_t*)max_value = screen->get_video_param(screen, PIPE_VIDEO_PROFILE_UNKNOWN,
                                                      PIPE_VIDEO_ENTRYPOINT_BITSTREAM,
                                                      PIPE_VIDEO_CAP_MAX_HEIGHT);
      break;

   case VDP_VIDEO_MIXER_PARAMETER_LAYERS:
      *(uint32_t*)min_value = 0;
      *(uint32_t*)max_value = 4;
      break;

   case VDP_VIDEO_MIXER_PARAMETER_CHROMA_TYPE:
   default:
      mtx_unlock(&dev->mutex);
      return VDP_STATUS_INVALID_VIDEO_MIXER_PARAMETER;
   }
   mtx_unlock(&dev->mutex);
   return VDP_STATUS_OK;
}

/**
 * Query the implementation's support for a specific attribute.
 */
VdpStatus
vlVdpVideoMixerQueryAttributeSupport(VdpDevice device, VdpVideoMixerAttribute attribute,
                                     VdpBool *is_supported)
{
   if (!is_supported)
      return VDP_STATUS_INVALID_POINTER;

   switch (attribute) {
   case VDP_VIDEO_MIXER_ATTRIBUTE_BACKGROUND_COLOR:
   case VDP_VIDEO_MIXER_ATTRIBUTE_CSC_MATRIX:
   case VDP_VIDEO_MIXER_ATTRIBUTE_NOISE_REDUCTION_LEVEL:
   case VDP_VIDEO_MIXER_ATTRIBUTE_SHARPNESS_LEVEL:
   case VDP_VIDEO_MIXER_ATTRIBUTE_LUMA_KEY_MIN_LUMA:
   case VDP_VIDEO_MIXER_ATTRIBUTE_LUMA_KEY_MAX_LUMA:
   case VDP_VIDEO_MIXER_ATTRIBUTE_SKIP_CHROMA_DEINTERLACE:
      *is_supported = VDP_TRUE;
      break;
   default:
      *is_supported = VDP_FALSE;
   }
   return VDP_STATUS_OK;
}

/**
 * Query the implementation's supported for a specific attribute.
 */
VdpStatus
vlVdpVideoMixerQueryAttributeValueRange(VdpDevice device, VdpVideoMixerAttribute attribute,
                                        void *min_value, void *max_value)
{
   if (!(min_value && max_value))
      return VDP_STATUS_INVALID_POINTER;

   switch (attribute) {
   case VDP_VIDEO_MIXER_ATTRIBUTE_NOISE_REDUCTION_LEVEL:
   case VDP_VIDEO_MIXER_ATTRIBUTE_LUMA_KEY_MIN_LUMA:
   case VDP_VIDEO_MIXER_ATTRIBUTE_LUMA_KEY_MAX_LUMA:
      *(float*)min_value = 0.0f;
      *(float*)max_value = 1.0f;
      break;
   case VDP_VIDEO_MIXER_ATTRIBUTE_SHARPNESS_LEVEL:
      *(float*)min_value = -1.0f;
      *(float*)max_value = 1.0f;
      break;
   case VDP_VIDEO_MIXER_ATTRIBUTE_SKIP_CHROMA_DEINTERLACE:
      *(uint8_t*)min_value = 0;
      *(uint8_t*)max_value = 1;
      break;
   case VDP_VIDEO_MIXER_ATTRIBUTE_BACKGROUND_COLOR:
   case VDP_VIDEO_MIXER_ATTRIBUTE_CSC_MATRIX:
   default:
      return VDP_STATUS_INVALID_VIDEO_MIXER_ATTRIBUTE;
   }
   return VDP_STATUS_OK;
}
