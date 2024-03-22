/**************************************************************************
 *
 * Copyright 2010 Younes Manton & Thomas Balling Sørensen.
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

#ifndef VDPAU_PRIVATE_H
#define VDPAU_PRIVATE_H

#include <assert.h>

#include <vdpau/vdpau.h>
#include <vdpau/vdpau_x11.h>

#include "util/compiler.h"
#include "pipe/p_video_codec.h"

#include "frontend/vdpau_interop.h"
#include "frontend/vdpau_dmabuf.h"
#include "frontend/vdpau_funcs.h"

#include "util/u_debug.h"
#include "util/u_rect.h"
#include "util/u_thread.h"

#include "vl/vl_video_buffer.h"
#include "vl/vl_bicubic_filter.h"
#include "vl/vl_compositor.h"
#include "vl/vl_csc.h"
#include "vl/vl_deint_filter.h"
#include "vl/vl_matrix_filter.h"
#include "vl/vl_median_filter.h"
#include "vl/vl_winsys.h"

/* Full VDPAU API documentation available at :
 * ftp://download.nvidia.com/XFree86/vdpau/doxygen/html/index.html */

#define INFORMATION G3DVL VDPAU Driver Shared Library version VER_MAJOR.VER_MINOR
#define QUOTEME(x) #x
#define TOSTRING(x) QUOTEME(x)
#define INFORMATION_STRING TOSTRING(INFORMATION)

static inline enum pipe_video_chroma_format
ChromaToPipe(VdpChromaType vdpau_type)
{
   switch (vdpau_type) {
      case VDP_CHROMA_TYPE_420:
         return PIPE_VIDEO_CHROMA_FORMAT_420;
      case VDP_CHROMA_TYPE_422:
         return PIPE_VIDEO_CHROMA_FORMAT_422;
      case VDP_CHROMA_TYPE_444:
         return PIPE_VIDEO_CHROMA_FORMAT_444;
      default:
         assert(0);
   }

   return -1;
}

static inline VdpChromaType
PipeToChroma(enum pipe_video_chroma_format pipe_type)
{
   switch (pipe_type) {
      case PIPE_VIDEO_CHROMA_FORMAT_420:
         return VDP_CHROMA_TYPE_420;
      case PIPE_VIDEO_CHROMA_FORMAT_422:
         return VDP_CHROMA_TYPE_422;
      case PIPE_VIDEO_CHROMA_FORMAT_444:
         return VDP_CHROMA_TYPE_444;
      default:
         assert(0);
   }

   return -1;
}

static inline enum pipe_video_chroma_format
FormatYCBCRToPipeChroma(VdpYCbCrFormat vdpau_format)
{
   switch (vdpau_format) {
      case VDP_YCBCR_FORMAT_NV12:
         return PIPE_VIDEO_CHROMA_FORMAT_420;
      case VDP_YCBCR_FORMAT_YV12:
         return PIPE_VIDEO_CHROMA_FORMAT_420;
      case VDP_YCBCR_FORMAT_UYVY:
         return PIPE_VIDEO_CHROMA_FORMAT_422;
      case VDP_YCBCR_FORMAT_YUYV:
         return PIPE_VIDEO_CHROMA_FORMAT_422;
      case VDP_YCBCR_FORMAT_Y8U8V8A8:
         return PIPE_VIDEO_CHROMA_FORMAT_444;
      case VDP_YCBCR_FORMAT_V8U8Y8A8:
         return PIPE_VIDEO_CHROMA_FORMAT_444;
      default:
         assert(0);
   }

   return PIPE_VIDEO_CHROMA_FORMAT_NONE;
}

static inline enum pipe_format
FormatYCBCRToPipe(VdpYCbCrFormat vdpau_format)
{
   switch (vdpau_format) {
      case VDP_YCBCR_FORMAT_NV12:
         return PIPE_FORMAT_NV12;
      case VDP_YCBCR_FORMAT_YV12:
         return PIPE_FORMAT_YV12;
      case VDP_YCBCR_FORMAT_UYVY:
         return PIPE_FORMAT_UYVY;
      case VDP_YCBCR_FORMAT_YUYV:
         return PIPE_FORMAT_YUYV;
      case VDP_YCBCR_FORMAT_Y8U8V8A8:
         return PIPE_FORMAT_R8G8B8A8_UNORM;
      case VDP_YCBCR_FORMAT_V8U8Y8A8:
         return PIPE_FORMAT_B8G8R8A8_UNORM;
#ifdef VDP_YCBCR_FORMAT_P010
      case VDP_YCBCR_FORMAT_P010:
         return PIPE_FORMAT_P010;
#endif
#ifdef VDP_YCBCR_FORMAT_P016
      case VDP_YCBCR_FORMAT_P016:
         return PIPE_FORMAT_P016;
#endif
      default:
         /* NOTE: Can't be "unreachable", as it's quite reachable. */
         assert(!"unexpected VdpYCbCrFormat");
         return PIPE_FORMAT_NONE;
#ifdef VDP_YCBCR_FORMAT_Y_UV_444
      case VDP_YCBCR_FORMAT_Y_UV_444:
         return PIPE_FORMAT_NONE;
#endif
#ifdef VDP_YCBCR_FORMAT_Y_U_V_444
      case VDP_YCBCR_FORMAT_Y_U_V_444:
         return PIPE_FORMAT_NONE;
#endif
#ifdef VDP_YCBCR_FORMAT_Y_U_V_444_16
      case VDP_YCBCR_FORMAT_Y_U_V_444_16:
         return PIPE_FORMAT_NONE;
#endif
   }

}

static inline VdpYCbCrFormat
PipeToFormatYCBCR(enum pipe_format p_format)
{
   switch (p_format) {
      case PIPE_FORMAT_NV12:
         return VDP_YCBCR_FORMAT_NV12;
      case PIPE_FORMAT_YV12:
         return VDP_YCBCR_FORMAT_YV12;
      case PIPE_FORMAT_UYVY:
         return VDP_YCBCR_FORMAT_UYVY;
      case PIPE_FORMAT_YUYV:
         return VDP_YCBCR_FORMAT_YUYV;
      case PIPE_FORMAT_R8G8B8A8_UNORM:
        return VDP_YCBCR_FORMAT_Y8U8V8A8;
      case PIPE_FORMAT_B8G8R8A8_UNORM:
         return VDP_YCBCR_FORMAT_V8U8Y8A8;
      default:
         assert(0);
   }

   return -1;
}

static inline VdpRGBAFormat
PipeToFormatRGBA(enum pipe_format p_format)
{
   switch (p_format) {
      case PIPE_FORMAT_A8_UNORM:
         return VDP_RGBA_FORMAT_A8;
      case PIPE_FORMAT_B10G10R10A2_UNORM:
         return VDP_RGBA_FORMAT_B10G10R10A2;
      case PIPE_FORMAT_B8G8R8A8_UNORM:
         return VDP_RGBA_FORMAT_B8G8R8A8;
      case PIPE_FORMAT_R10G10B10A2_UNORM:
         return VDP_RGBA_FORMAT_R10G10B10A2;
      case PIPE_FORMAT_R8G8B8A8_UNORM:
         return VDP_RGBA_FORMAT_R8G8B8A8;
      default:
         assert(0);
   }

   return -1;
}

static inline enum pipe_format
FormatIndexedToPipe(VdpRGBAFormat vdpau_format)
{
   switch (vdpau_format) {
      case VDP_INDEXED_FORMAT_A4I4:
         return PIPE_FORMAT_R4A4_UNORM;
      case VDP_INDEXED_FORMAT_I4A4:
         return PIPE_FORMAT_A4R4_UNORM;
      case VDP_INDEXED_FORMAT_A8I8:
         return PIPE_FORMAT_A8R8_UNORM;
      case VDP_INDEXED_FORMAT_I8A8:
         return PIPE_FORMAT_R8A8_UNORM;
      default:
         assert(0);
   }

   return PIPE_FORMAT_NONE;
}

static inline enum pipe_format
FormatColorTableToPipe(VdpColorTableFormat vdpau_format)
{
   switch(vdpau_format) {
      case VDP_COLOR_TABLE_FORMAT_B8G8R8X8:
         return PIPE_FORMAT_B8G8R8X8_UNORM;
      default:
         assert(0);
   }

   return PIPE_FORMAT_NONE;
}

static inline enum pipe_video_profile
ProfileToPipe(VdpDecoderProfile vdpau_profile)
{
   switch (vdpau_profile) {
      case VDP_DECODER_PROFILE_MPEG1:
         return PIPE_VIDEO_PROFILE_MPEG1;
      case VDP_DECODER_PROFILE_MPEG2_SIMPLE:
         return PIPE_VIDEO_PROFILE_MPEG2_SIMPLE;
      case VDP_DECODER_PROFILE_MPEG2_MAIN:
         return PIPE_VIDEO_PROFILE_MPEG2_MAIN;
      case VDP_DECODER_PROFILE_H264_BASELINE:
         return PIPE_VIDEO_PROFILE_MPEG4_AVC_BASELINE;
      case VDP_DECODER_PROFILE_H264_CONSTRAINED_BASELINE:
         return PIPE_VIDEO_PROFILE_MPEG4_AVC_CONSTRAINED_BASELINE;
      case VDP_DECODER_PROFILE_H264_MAIN:
         return PIPE_VIDEO_PROFILE_MPEG4_AVC_MAIN;
      case VDP_DECODER_PROFILE_H264_HIGH:
         return PIPE_VIDEO_PROFILE_MPEG4_AVC_HIGH;
      case VDP_DECODER_PROFILE_MPEG4_PART2_SP:
         return PIPE_VIDEO_PROFILE_MPEG4_SIMPLE;
      case VDP_DECODER_PROFILE_MPEG4_PART2_ASP:
         return PIPE_VIDEO_PROFILE_MPEG4_ADVANCED_SIMPLE;
      case VDP_DECODER_PROFILE_VC1_SIMPLE:
         return PIPE_VIDEO_PROFILE_VC1_SIMPLE;
      case VDP_DECODER_PROFILE_VC1_MAIN:
         return PIPE_VIDEO_PROFILE_VC1_MAIN;
      case VDP_DECODER_PROFILE_VC1_ADVANCED:
         return PIPE_VIDEO_PROFILE_VC1_ADVANCED;
      case VDP_DECODER_PROFILE_HEVC_MAIN:
         return PIPE_VIDEO_PROFILE_HEVC_MAIN;
      case VDP_DECODER_PROFILE_HEVC_MAIN_10:
         return PIPE_VIDEO_PROFILE_HEVC_MAIN_10;
      case VDP_DECODER_PROFILE_HEVC_MAIN_STILL:
         return PIPE_VIDEO_PROFILE_HEVC_MAIN_STILL;
      case VDP_DECODER_PROFILE_HEVC_MAIN_12:
         return PIPE_VIDEO_PROFILE_HEVC_MAIN_12;
      case VDP_DECODER_PROFILE_HEVC_MAIN_444:
         return PIPE_VIDEO_PROFILE_HEVC_MAIN_444;
      default:
         return PIPE_VIDEO_PROFILE_UNKNOWN;
   }
}

static inline VdpDecoderProfile
PipeToProfile(enum pipe_video_profile p_profile)
{
   switch (p_profile) {
      case PIPE_VIDEO_PROFILE_MPEG1:
         return VDP_DECODER_PROFILE_MPEG1;
      case PIPE_VIDEO_PROFILE_MPEG2_SIMPLE:
         return VDP_DECODER_PROFILE_MPEG2_SIMPLE;
      case PIPE_VIDEO_PROFILE_MPEG2_MAIN:
         return VDP_DECODER_PROFILE_MPEG2_MAIN;
      case PIPE_VIDEO_PROFILE_MPEG4_AVC_BASELINE:
         return VDP_DECODER_PROFILE_H264_BASELINE;
      case PIPE_VIDEO_PROFILE_MPEG4_AVC_CONSTRAINED_BASELINE:
         return VDP_DECODER_PROFILE_H264_CONSTRAINED_BASELINE;
      case PIPE_VIDEO_PROFILE_MPEG4_AVC_MAIN:
         return VDP_DECODER_PROFILE_H264_MAIN;
      case PIPE_VIDEO_PROFILE_MPEG4_AVC_HIGH:
         return VDP_DECODER_PROFILE_H264_HIGH;
      case PIPE_VIDEO_PROFILE_MPEG4_SIMPLE:
         return VDP_DECODER_PROFILE_MPEG4_PART2_SP;
      case PIPE_VIDEO_PROFILE_MPEG4_ADVANCED_SIMPLE:
         return VDP_DECODER_PROFILE_MPEG4_PART2_ASP;
      case PIPE_VIDEO_PROFILE_VC1_SIMPLE:
         return VDP_DECODER_PROFILE_VC1_SIMPLE;
      case PIPE_VIDEO_PROFILE_VC1_MAIN:
         return VDP_DECODER_PROFILE_VC1_MAIN;
      case PIPE_VIDEO_PROFILE_VC1_ADVANCED:
         return VDP_DECODER_PROFILE_VC1_ADVANCED;
      case PIPE_VIDEO_PROFILE_HEVC_MAIN:
         return VDP_DECODER_PROFILE_HEVC_MAIN;
      case PIPE_VIDEO_PROFILE_HEVC_MAIN_10:
         return VDP_DECODER_PROFILE_HEVC_MAIN_10;
      case PIPE_VIDEO_PROFILE_HEVC_MAIN_STILL:
         return VDP_DECODER_PROFILE_HEVC_MAIN_STILL;
      case PIPE_VIDEO_PROFILE_HEVC_MAIN_12:
         return VDP_DECODER_PROFILE_HEVC_MAIN_12;
      case PIPE_VIDEO_PROFILE_HEVC_MAIN_444:
         return VDP_DECODER_PROFILE_HEVC_MAIN_444;
      default:
         assert(0);
         return -1;
   }
}

static inline struct u_rect *
RectToPipe(const VdpRect *src, struct u_rect *dst)
{
   if (src) {
      dst->x0 = src->x0;
      dst->y0 = src->y0;
      dst->x1 = src->x1;
      dst->y1 = src->y1;
      return dst;
   }
   return NULL;
}

static inline struct pipe_box
RectToPipeBox(const VdpRect *rect, struct pipe_resource *res)
{
   struct pipe_box box;

   box.x = 0;
   box.y = 0;
   box.z = 0;
   box.width = res->width0;
   box.height = res->height0;
   box.depth = 1;

   if (rect) {
      if (rect->x1 > rect->x0 &&
          rect->y1 > rect->y0) {
         box.x = rect->x0;
         box.y = rect->y0;
         box.width = rect->x1 - box.x;
         box.height = rect->y1 - box.y;
      } else {
         box.width = 0;
         box.height = 0;
      }
   }

   return box;
}

static inline bool
CheckSurfaceParams(struct pipe_screen *screen,
                   const struct pipe_resource *templ)
{
   return screen->is_format_supported(screen, templ->format, templ->target,
                                      templ->nr_samples,
                                      templ->nr_storage_samples, templ->bind);
}

typedef struct
{
   struct pipe_reference reference;
   struct vl_screen *vscreen;
   struct pipe_context *context;
   struct vl_compositor compositor;
   struct pipe_sampler_view *dummy_sv;
   mtx_t mutex;
} vlVdpDevice;

typedef struct
{
   vlVdpDevice *device;
   struct vl_compositor_state cstate;

   struct {
       bool supported, enabled;
       float luma_min, luma_max;
   } luma_key;

   struct {
	  bool supported, enabled, spatial;
	  struct vl_deint_filter *filter;
   } deint;

   struct {
	  bool supported, enabled;
	  struct vl_bicubic_filter *filter;
   } bicubic;

   struct {
      bool supported, enabled;
      unsigned level;
      struct vl_median_filter *filter;
   } noise_reduction;

   struct {
      bool supported, enabled;
      float value;
      struct vl_matrix_filter *filter;
   } sharpness;

   unsigned video_width, video_height;
   enum pipe_video_chroma_format chroma_format;
   unsigned max_layers, skip_chroma_deint;

   bool custom_csc;
   vl_csc_matrix csc;
} vlVdpVideoMixer;

typedef struct
{
   vlVdpDevice *device;
   struct pipe_video_buffer templat, *video_buffer;
} vlVdpSurface;

typedef struct
{
   vlVdpDevice *device;
   struct pipe_sampler_view *sampler_view;
} vlVdpBitmapSurface;

typedef uint64_t vlVdpTime;

typedef struct
{
   vlVdpDevice *device;
   struct pipe_surface *surface;
   struct pipe_sampler_view *sampler_view;
   struct pipe_fence_handle *fence;
   struct vl_compositor_state cstate;
   struct u_rect dirty_area;
   bool send_to_X;
} vlVdpOutputSurface;

typedef struct
{
   vlVdpDevice *device;
   Drawable drawable;
} vlVdpPresentationQueueTarget;

typedef struct
{
   vlVdpDevice *device;
   Drawable drawable;
   struct vl_compositor_state cstate;
   vlVdpOutputSurface *last_surf;
} vlVdpPresentationQueue;

typedef struct
{
   vlVdpDevice *device;
   mtx_t mutex;
   struct pipe_video_codec *decoder;
} vlVdpDecoder;

typedef uint32_t vlHandle;

bool vlCreateHTAB(void);
void vlDestroyHTAB(void);
vlHandle vlAddDataHTAB(void *data);
void* vlGetDataHTAB(vlHandle handle);
void vlRemoveDataHTAB(vlHandle handle);

bool vlGetFuncFTAB(VdpFuncId function_id, void **func);

/* Public functions */
VdpDeviceCreateX11 vdp_imp_device_create_x11;

void vlVdpDefaultSamplerViewTemplate(struct pipe_sampler_view *templ, struct pipe_resource *res);

/* Internal function pointers */
VdpGetErrorString vlVdpGetErrorString;
VdpDeviceDestroy vlVdpDeviceDestroy;
void vlVdpDeviceFree(vlVdpDevice *dev);
VdpGetProcAddress vlVdpGetProcAddress;
VdpGetApiVersion vlVdpGetApiVersion;
VdpGetInformationString vlVdpGetInformationString;
VdpVideoSurfaceQueryCapabilities vlVdpVideoSurfaceQueryCapabilities;
VdpVideoSurfaceQueryGetPutBitsYCbCrCapabilities vlVdpVideoSurfaceQueryGetPutBitsYCbCrCapabilities;
VdpDecoderQueryCapabilities vlVdpDecoderQueryCapabilities;
VdpOutputSurfaceQueryCapabilities vlVdpOutputSurfaceQueryCapabilities;
VdpOutputSurfaceQueryGetPutBitsNativeCapabilities vlVdpOutputSurfaceQueryGetPutBitsNativeCapabilities;
VdpOutputSurfaceQueryPutBitsIndexedCapabilities vlVdpOutputSurfaceQueryPutBitsIndexedCapabilities;
VdpOutputSurfaceQueryPutBitsYCbCrCapabilities vlVdpOutputSurfaceQueryPutBitsYCbCrCapabilities;
VdpBitmapSurfaceQueryCapabilities vlVdpBitmapSurfaceQueryCapabilities;
VdpVideoMixerQueryFeatureSupport vlVdpVideoMixerQueryFeatureSupport;
VdpVideoMixerQueryParameterSupport vlVdpVideoMixerQueryParameterSupport;
VdpVideoMixerQueryParameterValueRange vlVdpVideoMixerQueryParameterValueRange;
VdpVideoMixerQueryAttributeSupport vlVdpVideoMixerQueryAttributeSupport;
VdpVideoMixerQueryAttributeValueRange vlVdpVideoMixerQueryAttributeValueRange;
VdpVideoSurfaceCreate vlVdpVideoSurfaceCreate;
VdpVideoSurfaceDestroy vlVdpVideoSurfaceDestroy;
VdpVideoSurfaceGetParameters vlVdpVideoSurfaceGetParameters;
VdpVideoSurfaceGetBitsYCbCr vlVdpVideoSurfaceGetBitsYCbCr;
VdpVideoSurfacePutBitsYCbCr vlVdpVideoSurfacePutBitsYCbCr;
void vlVdpVideoSurfaceClear(vlVdpSurface *vlsurf);
VdpDecoderCreate vlVdpDecoderCreate;
VdpDecoderDestroy vlVdpDecoderDestroy;
VdpDecoderGetParameters vlVdpDecoderGetParameters;
VdpDecoderRender vlVdpDecoderRender;
VdpOutputSurfaceCreate vlVdpOutputSurfaceCreate;
VdpOutputSurfaceDestroy vlVdpOutputSurfaceDestroy;
VdpOutputSurfaceGetParameters vlVdpOutputSurfaceGetParameters;
VdpOutputSurfaceGetBitsNative vlVdpOutputSurfaceGetBitsNative;
VdpOutputSurfacePutBitsNative vlVdpOutputSurfacePutBitsNative;
VdpOutputSurfacePutBitsIndexed vlVdpOutputSurfacePutBitsIndexed;
VdpOutputSurfacePutBitsYCbCr vlVdpOutputSurfacePutBitsYCbCr;
VdpOutputSurfaceRenderOutputSurface vlVdpOutputSurfaceRenderOutputSurface;
VdpOutputSurfaceRenderBitmapSurface vlVdpOutputSurfaceRenderBitmapSurface;
VdpBitmapSurfaceCreate vlVdpBitmapSurfaceCreate;
VdpBitmapSurfaceDestroy vlVdpBitmapSurfaceDestroy;
VdpBitmapSurfaceGetParameters vlVdpBitmapSurfaceGetParameters;
VdpBitmapSurfacePutBitsNative vlVdpBitmapSurfacePutBitsNative;
VdpPresentationQueueTargetDestroy vlVdpPresentationQueueTargetDestroy;
VdpPresentationQueueCreate vlVdpPresentationQueueCreate;
VdpPresentationQueueDestroy vlVdpPresentationQueueDestroy;
VdpPresentationQueueSetBackgroundColor vlVdpPresentationQueueSetBackgroundColor;
VdpPresentationQueueGetBackgroundColor vlVdpPresentationQueueGetBackgroundColor;
VdpPresentationQueueGetTime vlVdpPresentationQueueGetTime;
VdpPresentationQueueDisplay vlVdpPresentationQueueDisplay;
VdpPresentationQueueBlockUntilSurfaceIdle vlVdpPresentationQueueBlockUntilSurfaceIdle;
VdpPresentationQueueQuerySurfaceStatus vlVdpPresentationQueueQuerySurfaceStatus;
VdpPreemptionCallback vlVdpPreemptionCallback;
VdpPreemptionCallbackRegister vlVdpPreemptionCallbackRegister;
VdpVideoMixerSetFeatureEnables vlVdpVideoMixerSetFeatureEnables;
VdpVideoMixerCreate vlVdpVideoMixerCreate;
VdpVideoMixerRender vlVdpVideoMixerRender;
VdpVideoMixerSetAttributeValues vlVdpVideoMixerSetAttributeValues;
VdpVideoMixerGetFeatureSupport vlVdpVideoMixerGetFeatureSupport;
VdpVideoMixerGetFeatureEnables vlVdpVideoMixerGetFeatureEnables;
VdpVideoMixerGetParameterValues vlVdpVideoMixerGetParameterValues;
VdpVideoMixerGetAttributeValues vlVdpVideoMixerGetAttributeValues;
VdpVideoMixerDestroy vlVdpVideoMixerDestroy;
VdpGenerateCSCMatrix vlVdpGenerateCSCMatrix;
/* Winsys specific internal function pointers */
VdpPresentationQueueTargetCreateX11 vlVdpPresentationQueueTargetCreateX11;


/* interop for GL gallium frontend */
VdpVideoSurfaceGallium vlVdpVideoSurfaceGallium;
VdpOutputSurfaceGallium vlVdpOutputSurfaceGallium;
VdpVideoSurfaceDMABuf vlVdpVideoSurfaceDMABuf;
VdpOutputSurfaceDMABuf vlVdpOutputSurfaceDMABuf;

#define VDPAU_OUT   0
#define VDPAU_ERR   1
#define VDPAU_WARN  2
#define VDPAU_TRACE 3

static inline void VDPAU_MSG(unsigned int level, const char *fmt, ...)
{
   static int debug_level = -1;

   if (debug_level == -1) {
      debug_level = MAX2(debug_get_num_option("VDPAU_DEBUG", 0), 0);
   }

   if (level <= debug_level) {
      va_list ap;
      va_start(ap, fmt);
      _debug_vprintf(fmt, ap);
      va_end(ap);
   }
}

static inline void
DeviceReference(vlVdpDevice **ptr, vlVdpDevice *dev)
{
   vlVdpDevice *old_dev = *ptr;

   if (pipe_reference(&(*ptr)->reference, &dev->reference))
      vlVdpDeviceFree(old_dev);
   *ptr = dev;
}

#endif /* VDPAU_PRIVATE_H */
