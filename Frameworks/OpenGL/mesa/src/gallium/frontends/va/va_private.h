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

#ifndef VA_PRIVATE_H
#define VA_PRIVATE_H

#include <assert.h>

#include <va/va.h>
#include <va/va_backend.h>
#include <va/va_backend_vpp.h>
#include <va/va_drmcommon.h>

#include "pipe/p_video_enums.h"
#include "pipe/p_video_codec.h"
#include "pipe/p_video_state.h"

#include "vl/vl_compositor.h"
#include "vl/vl_csc.h"

#include "util/u_dynarray.h"
#include "util/u_thread.h"
#include "util/detect_os.h"

#if DETECT_OS_WINDOWS
#define VA_PUBLIC_API
#else
#define VA_PUBLIC_API PUBLIC
#endif

#ifndef VA_RT_FORMAT_YUV420_10
#define VA_RT_FORMAT_YUV420_10  VA_RT_FORMAT_YUV420_10BPP
#endif

#define VL_VA_DRIVER(ctx) ((vlVaDriver *)ctx->pDriverData)
#define VL_VA_PSCREEN(ctx) (VL_VA_DRIVER(ctx)->vscreen->pscreen)

#define VL_VA_MAX_IMAGE_FORMATS 16
#define VL_VA_ENC_GOP_COEFF 16

#define UINT_TO_PTR(x) ((void*)(uintptr_t)(x))
#define PTR_TO_UINT(x) ((unsigned)((intptr_t)(x)))

#define SOI 2
#define DQT (4 + 4 * 65)
#define DHT (4 + 2 * 29 + 2 * 179)
#define DRI 6
#define SOF (10 + 255 * 3)
#define SOS (8 + 4 * 2)
#define MAX_MJPEG_SLICE_HEADER_SIZE (SOI + DQT + DHT + DRI + SOF + SOS)

#define PRESET_MODE_SPEED   (0)
#define PRESET_MODE_BALANCE (1)
#define PRESET_MODE_QUALITY (2)
#define PRESET_MODE_HIGH_QUALITY (3)

#define PREENCODING_MODE_DISABLE (0)
#define PREENCODING_MODE_DEFAULT (1)

#define VBAQ_DISABLE (0)
#define VBAQ_AUTO    (1)

static inline enum pipe_video_chroma_format
ChromaToPipe(int format)
{
   switch (format) {
   case VA_RT_FORMAT_YUV420:
   case VA_RT_FORMAT_YUV420_10BPP:
      return PIPE_VIDEO_CHROMA_FORMAT_420;
   case VA_RT_FORMAT_YUV422:
      return PIPE_VIDEO_CHROMA_FORMAT_422;
   case VA_RT_FORMAT_YUV444:
      return PIPE_VIDEO_CHROMA_FORMAT_444;
   case VA_RT_FORMAT_YUV400:
      return PIPE_VIDEO_CHROMA_FORMAT_400;
   default:
      return PIPE_VIDEO_CHROMA_FORMAT_NONE;
   }
}

static inline enum pipe_format
VaFourccToPipeFormat(unsigned format)
{
   switch(format) {
   case VA_FOURCC('N','V','1','2'):
      return PIPE_FORMAT_NV12;
   case VA_FOURCC('P','0','1','0'):
      return PIPE_FORMAT_P010;
   case VA_FOURCC('P','0','1','6'):
      return PIPE_FORMAT_P016;
   case VA_FOURCC('I','4','2','0'):
      return PIPE_FORMAT_IYUV;
   case VA_FOURCC('Y','V','1','2'):
      return PIPE_FORMAT_YV12;
   case VA_FOURCC('Y','U','Y','V'):
   case VA_FOURCC('Y','U','Y','2'):
      return PIPE_FORMAT_YUYV;
   case VA_FOURCC('U','Y','V','Y'):
      return PIPE_FORMAT_UYVY;
   case VA_FOURCC('B','G','R','A'):
      return PIPE_FORMAT_B8G8R8A8_UNORM;
   case VA_FOURCC('A','R','G','B'):
      return PIPE_FORMAT_A8R8G8B8_UNORM;
   case VA_FOURCC('R','G','B','A'):
      return PIPE_FORMAT_R8G8B8A8_UNORM;
   case VA_FOURCC('B','G','R','X'):
      return PIPE_FORMAT_B8G8R8X8_UNORM;
   case VA_FOURCC('R','G','B','X'):
      return PIPE_FORMAT_R8G8B8X8_UNORM;
   case VA_FOURCC('R','G','B','P'):
      return PIPE_FORMAT_R8_G8_B8_UNORM;
   case VA_FOURCC('Y','8','0','0'):
      return PIPE_FORMAT_Y8_400_UNORM;
   case VA_FOURCC('4','4','4','P'):
      return PIPE_FORMAT_Y8_U8_V8_444_UNORM;
   default:
      assert(0);
      return PIPE_FORMAT_NONE;
   }
}

static inline unsigned
PipeFormatToVaFourcc(enum pipe_format p_format)
{
   switch (p_format) {
   case PIPE_FORMAT_NV12:
      return VA_FOURCC('N','V','1','2');
   case PIPE_FORMAT_P010:
      return VA_FOURCC('P','0','1','0');
   case PIPE_FORMAT_P016:
      return VA_FOURCC('P','0','1','6');
   case PIPE_FORMAT_IYUV:
      return VA_FOURCC('I','4','2','0');
   case PIPE_FORMAT_YV12:
      return VA_FOURCC('Y','V','1','2');
   case PIPE_FORMAT_UYVY:
      return VA_FOURCC('U','Y','V','Y');
   case PIPE_FORMAT_YUYV:
      return VA_FOURCC('Y','U','Y','V');
   case PIPE_FORMAT_B8G8R8A8_UNORM:
      return VA_FOURCC('B','G','R','A');
   case PIPE_FORMAT_R8G8B8A8_UNORM:
      return VA_FOURCC('R','G','B','A');
   case PIPE_FORMAT_A8R8G8B8_UNORM:
      return VA_FOURCC('A','R','G','B');
   case PIPE_FORMAT_B8G8R8X8_UNORM:
      return VA_FOURCC('B','G','R','X');
   case PIPE_FORMAT_R8G8B8X8_UNORM:
      return VA_FOURCC('R','G','B','X');
    case PIPE_FORMAT_R8_G8_B8_UNORM:
      return VA_FOURCC('R','G','B','P');
   case PIPE_FORMAT_Y8_400_UNORM:
      return VA_FOURCC('Y','8','0','0');
   case PIPE_FORMAT_Y8_U8_V8_444_UNORM:
      return VA_FOURCC('4','4','4','P');
   default:
      assert(0);
      return -1;
   }
}

static inline VAProfile
PipeToProfile(enum pipe_video_profile profile)
{
   switch (profile) {
   case PIPE_VIDEO_PROFILE_MPEG2_SIMPLE:
      return VAProfileMPEG2Simple;
   case PIPE_VIDEO_PROFILE_MPEG2_MAIN:
      return VAProfileMPEG2Main;
   case PIPE_VIDEO_PROFILE_MPEG4_SIMPLE:
      return VAProfileMPEG4Simple;
   case PIPE_VIDEO_PROFILE_MPEG4_ADVANCED_SIMPLE:
      return VAProfileMPEG4AdvancedSimple;
   case PIPE_VIDEO_PROFILE_VC1_SIMPLE:
      return VAProfileVC1Simple;
   case PIPE_VIDEO_PROFILE_VC1_MAIN:
      return VAProfileVC1Main;
   case PIPE_VIDEO_PROFILE_VC1_ADVANCED:
      return VAProfileVC1Advanced;
   case PIPE_VIDEO_PROFILE_MPEG4_AVC_BASELINE:
      return VAProfileH264ConstrainedBaseline;
   case PIPE_VIDEO_PROFILE_MPEG4_AVC_MAIN:
      return VAProfileH264Main;
   case PIPE_VIDEO_PROFILE_MPEG4_AVC_HIGH:
      return VAProfileH264High;
   case PIPE_VIDEO_PROFILE_MPEG4_AVC_HIGH10:
#if VA_CHECK_VERSION(1, 18, 0)
      return VAProfileH264High10;
#else
      return VAProfileNone;
#endif
   case PIPE_VIDEO_PROFILE_HEVC_MAIN:
      return VAProfileHEVCMain;
   case PIPE_VIDEO_PROFILE_HEVC_MAIN_10:
      return VAProfileHEVCMain10;
   case PIPE_VIDEO_PROFILE_JPEG_BASELINE:
      return VAProfileJPEGBaseline;
   case PIPE_VIDEO_PROFILE_VP9_PROFILE0:
      return VAProfileVP9Profile0;
   case PIPE_VIDEO_PROFILE_VP9_PROFILE2:
      return VAProfileVP9Profile2;
   case PIPE_VIDEO_PROFILE_AV1_MAIN:
      return VAProfileAV1Profile0;
   case PIPE_VIDEO_PROFILE_MPEG4_AVC_EXTENDED:
   case PIPE_VIDEO_PROFILE_MPEG4_AVC_HIGH422:
   case PIPE_VIDEO_PROFILE_MPEG4_AVC_HIGH444:
   case PIPE_VIDEO_PROFILE_MPEG4_AVC_CONSTRAINED_BASELINE:
   case PIPE_VIDEO_PROFILE_HEVC_MAIN_12:
   case PIPE_VIDEO_PROFILE_HEVC_MAIN_STILL:
   case PIPE_VIDEO_PROFILE_HEVC_MAIN_444:
   case PIPE_VIDEO_PROFILE_UNKNOWN:
      return VAProfileNone;
   default:
      assert(0);
      return -1;
   }
}

static inline enum pipe_video_profile
ProfileToPipe(VAProfile profile)
{
   switch (profile) {
   case VAProfileMPEG2Simple:
      return PIPE_VIDEO_PROFILE_MPEG2_SIMPLE;
   case VAProfileMPEG2Main:
      return PIPE_VIDEO_PROFILE_MPEG2_MAIN;
   case VAProfileMPEG4Simple:
      return PIPE_VIDEO_PROFILE_MPEG4_SIMPLE;
   case VAProfileMPEG4AdvancedSimple:
      return PIPE_VIDEO_PROFILE_MPEG4_ADVANCED_SIMPLE;
   case VAProfileVC1Simple:
      return PIPE_VIDEO_PROFILE_VC1_SIMPLE;
   case VAProfileVC1Main:
      return PIPE_VIDEO_PROFILE_VC1_MAIN;
   case VAProfileVC1Advanced:
      return PIPE_VIDEO_PROFILE_VC1_ADVANCED;
   case VAProfileH264ConstrainedBaseline:
      return PIPE_VIDEO_PROFILE_MPEG4_AVC_BASELINE;
   case VAProfileH264Main:
      return PIPE_VIDEO_PROFILE_MPEG4_AVC_MAIN;
   case VAProfileH264High:
      return PIPE_VIDEO_PROFILE_MPEG4_AVC_HIGH;
#if VA_CHECK_VERSION(1, 18, 0)
   case VAProfileH264High10:
      return PIPE_VIDEO_PROFILE_MPEG4_AVC_HIGH10;
#endif
   case VAProfileHEVCMain:
      return PIPE_VIDEO_PROFILE_HEVC_MAIN;
   case VAProfileHEVCMain10:
      return PIPE_VIDEO_PROFILE_HEVC_MAIN_10;
   case VAProfileJPEGBaseline:
      return PIPE_VIDEO_PROFILE_JPEG_BASELINE;
   case VAProfileVP9Profile0:
      return PIPE_VIDEO_PROFILE_VP9_PROFILE0;
   case VAProfileVP9Profile2:
      return PIPE_VIDEO_PROFILE_VP9_PROFILE2;
   case VAProfileAV1Profile0:
      return PIPE_VIDEO_PROFILE_AV1_MAIN;
   case VAProfileNone:
       return PIPE_VIDEO_PROFILE_UNKNOWN;
   default:
      return PIPE_VIDEO_PROFILE_UNKNOWN;
   }
}

typedef struct {
   struct vl_screen *vscreen;
   struct pipe_context *pipe;
   struct handle_table *htab;
   struct vl_compositor compositor;
   struct vl_compositor_state cstate;
   vl_csc_matrix csc;
   mtx_t mutex;
   char vendor_string[256];
} vlVaDriver;

typedef struct {
   VAImage *image;

   struct u_rect src_rect;
   struct u_rect dst_rect;

   struct pipe_sampler_view *sampler;
} vlVaSubpicture;

typedef struct {
   VABufferType type;
   unsigned int size;
   unsigned int num_elements;
   void *data;
   struct {
      struct pipe_resource *resource;
      struct pipe_transfer *transfer;
      enum pipe_video_entrypoint entrypoint;
   } derived_surface;
   unsigned int export_refcount;
   VABufferInfo export_state;
   unsigned int coded_size;
   struct pipe_enc_feedback_metadata extended_metadata;
   struct pipe_video_buffer *derived_image_buffer;
   void *feedback;
   VASurfaceID associated_encode_input_surf;
   VAContextID ctx;
} vlVaBuffer;

typedef struct {
   struct pipe_video_codec templat, *decoder;
   struct pipe_video_buffer *target;
   union {
      struct pipe_picture_desc base;
      struct pipe_mpeg12_picture_desc mpeg12;
      struct pipe_mpeg4_picture_desc mpeg4;
      struct pipe_vc1_picture_desc vc1;
      struct pipe_h264_picture_desc h264;
      struct pipe_h265_picture_desc h265;
      struct pipe_mjpeg_picture_desc mjpeg;
      struct pipe_vp9_picture_desc vp9;
      struct pipe_av1_picture_desc av1;
      struct pipe_h264_enc_picture_desc h264enc;
      struct pipe_h265_enc_picture_desc h265enc;
      struct pipe_av1_enc_picture_desc av1enc;
      struct pipe_vpp_desc vidproc;
   } desc;

   struct {
      unsigned long long int frame_num;
      unsigned int start_code_size;
      unsigned int vti_bits;
      unsigned int quant_scale;
      VAPictureParameterBufferMPEG4 pps;
      uint8_t start_code[32];
   } mpeg4;

   struct {
      unsigned sampling_factor;
      #define MJPEG_SAMPLING_FACTOR_NV12   (0x221111)
      #define MJPEG_SAMPLING_FACTOR_YUY2   (0x221212)
      #define MJPEG_SAMPLING_FACTOR_YUV422 (0x211111)
      #define MJPEG_SAMPLING_FACTOR_YUV444 (0x111111)
      #define MJPEG_SAMPLING_FACTOR_YUV400 (0x11)
      uint8_t slice_header[MAX_MJPEG_SLICE_HEADER_SIZE];
      unsigned int slice_header_size;
   } mjpeg;

   struct vl_deint_filter *deint;
   vlVaBuffer *coded_buf;
   int target_id;
   bool first_single_submitted;
   int gop_coeff;
   bool needs_begin_frame;
   void *blit_cs;
   int packed_header_type;
   bool packed_header_emulation_bytes;
   struct set *surfaces;
} vlVaContext;

typedef struct {
   enum pipe_video_profile profile;
   enum pipe_video_entrypoint entrypoint;
   enum pipe_h2645_enc_rate_control_method rc;
   unsigned int rt_format;
} vlVaConfig;

typedef struct {
   struct pipe_video_buffer templat, *buffer;
   struct util_dynarray subpics; /* vlVaSubpicture */
   vlVaContext *ctx;
   vlVaBuffer *coded_buf;
   void *feedback;
   unsigned int frame_num_cnt;
   bool force_flushed;
   struct pipe_video_buffer *obsolete_buf;
   enum pipe_format encoder_format;
   bool full_range;
   struct pipe_fence_handle *fence;
} vlVaSurface;

typedef struct {
   union {
      unsigned int quality;
      struct {
         unsigned int valid_setting: 1;
         unsigned int preset_mode: 2;
         unsigned int pre_encode_mode: 1;
         unsigned int vbaq_mode: 1;
         unsigned int reservered: 27;
      };
   };
} vlVaQualityBits;

// Public functions:
VAStatus VA_DRIVER_INIT_FUNC(VADriverContextP ctx);

// vtable functions:
VAStatus vlVaTerminate(VADriverContextP ctx);
VAStatus vlVaQueryConfigProfiles(VADriverContextP ctx, VAProfile *profile_list,int *num_profiles);
VAStatus vlVaQueryConfigEntrypoints(VADriverContextP ctx, VAProfile profile,
                                    VAEntrypoint  *entrypoint_list, int *num_entrypoints);
VAStatus vlVaGetConfigAttributes(VADriverContextP ctx, VAProfile profile, VAEntrypoint entrypoint,
                                 VAConfigAttrib *attrib_list, int num_attribs);
VAStatus vlVaCreateConfig(VADriverContextP ctx, VAProfile profile, VAEntrypoint entrypoint,
                          VAConfigAttrib *attrib_list, int num_attribs, VAConfigID *config_id);
VAStatus vlVaDestroyConfig(VADriverContextP ctx, VAConfigID config_id);
VAStatus vlVaQueryConfigAttributes(VADriverContextP ctx, VAConfigID config_id, VAProfile *profile,
                                   VAEntrypoint *entrypoint, VAConfigAttrib *attrib_list, int *num_attribs);
VAStatus vlVaCreateSurfaces(VADriverContextP ctx, int width, int height, int format,
                            int num_surfaces, VASurfaceID *surfaces);
VAStatus vlVaDestroySurfaces(VADriverContextP ctx, VASurfaceID *surface_list, int num_surfaces);
VAStatus vlVaCreateContext(VADriverContextP ctx, VAConfigID config_id, int picture_width, int picture_height,
                           int flag, VASurfaceID *render_targets, int num_render_targets, VAContextID *context);
VAStatus vlVaDestroyContext(VADriverContextP ctx, VAContextID context);
VAStatus vlVaCreateBuffer(VADriverContextP ctx, VAContextID context, VABufferType type, unsigned int size,
                          unsigned int num_elements, void *data, VABufferID *buf_id);
VAStatus vlVaBufferSetNumElements(VADriverContextP ctx, VABufferID buf_id, unsigned int num_elements);
VAStatus vlVaMapBuffer(VADriverContextP ctx, VABufferID buf_id, void **pbuf);
VAStatus vlVaUnmapBuffer(VADriverContextP ctx, VABufferID buf_id);
VAStatus vlVaDestroyBuffer(VADriverContextP ctx, VABufferID buffer_id);
VAStatus vlVaBeginPicture(VADriverContextP ctx, VAContextID context, VASurfaceID render_target);
VAStatus vlVaRenderPicture(VADriverContextP ctx, VAContextID context, VABufferID *buffers, int num_buffers);
VAStatus vlVaEndPicture(VADriverContextP ctx, VAContextID context);
VAStatus vlVaSyncSurface(VADriverContextP ctx, VASurfaceID render_target);
VAStatus vlVaQuerySurfaceStatus(VADriverContextP ctx, VASurfaceID render_target, VASurfaceStatus *status);
VAStatus vlVaQuerySurfaceError(VADriverContextP ctx, VASurfaceID render_target,
                               VAStatus error_status, void **error_info);
VAStatus vlVaPutSurface(VADriverContextP ctx, VASurfaceID surface, void* draw, short srcx, short srcy,
                        unsigned short srcw, unsigned short srch, short destx, short desty, unsigned short destw,
                        unsigned short desth, VARectangle *cliprects, unsigned int number_cliprects,
                        unsigned int flags);
VAStatus vlVaQueryImageFormats(VADriverContextP ctx, VAImageFormat *format_list, int *num_formats);
VAStatus vlVaQuerySubpictureFormats(VADriverContextP ctx, VAImageFormat *format_list,
                                    unsigned int *flags, unsigned int *num_formats);
VAStatus vlVaCreateImage(VADriverContextP ctx, VAImageFormat *format, int width, int height, VAImage *image);
VAStatus vlVaDeriveImage(VADriverContextP ctx, VASurfaceID surface, VAImage *image);
VAStatus vlVaDestroyImage(VADriverContextP ctx, VAImageID image);
VAStatus vlVaSetImagePalette(VADriverContextP ctx, VAImageID image, unsigned char *palette);
VAStatus vlVaGetImage(VADriverContextP ctx, VASurfaceID surface, int x, int y,
                      unsigned int width, unsigned int height, VAImageID image);
VAStatus vlVaPutImage(VADriverContextP ctx, VASurfaceID surface, VAImageID image, int src_x, int src_y,
                      unsigned int src_width, unsigned int src_height, int dest_x, int dest_y,
                      unsigned int dest_width, unsigned int dest_height);
VAStatus vlVaQuerySubpictureFormats(VADriverContextP ctx, VAImageFormat *format_list,
                                    unsigned int *flags, unsigned int *num_formats);
VAStatus vlVaCreateSubpicture(VADriverContextP ctx, VAImageID image, VASubpictureID *subpicture);
VAStatus vlVaDestroySubpicture(VADriverContextP ctx, VASubpictureID subpicture);
VAStatus vlVaSubpictureImage(VADriverContextP ctx, VASubpictureID subpicture, VAImageID image);
VAStatus vlVaSetSubpictureChromakey(VADriverContextP ctx, VASubpictureID subpicture,
                                    unsigned int chromakey_min, unsigned int chromakey_max,
                                    unsigned int chromakey_mask);
VAStatus vlVaSetSubpictureGlobalAlpha(VADriverContextP ctx, VASubpictureID subpicture, float global_alpha);
VAStatus vlVaAssociateSubpicture(VADriverContextP ctx, VASubpictureID subpicture, VASurfaceID *target_surfaces,
                                 int num_surfaces, short src_x, short src_y,
                                 unsigned short src_width, unsigned short src_height,
                                 short dest_x, short dest_y, unsigned short dest_width, unsigned short dest_height,
                                 unsigned int flags);
VAStatus vlVaDeassociateSubpicture(VADriverContextP ctx, VASubpictureID subpicture,
                                   VASurfaceID *target_surfaces, int num_surfaces);
VAStatus vlVaQueryDisplayAttributes(VADriverContextP ctx, VADisplayAttribute *attr_list, int *num_attributes);
VAStatus vlVaGetDisplayAttributes(VADriverContextP ctx, VADisplayAttribute *attr_list, int num_attributes);
VAStatus vlVaSetDisplayAttributes(VADriverContextP ctx, VADisplayAttribute *attr_list, int num_attributes);
VAStatus vlVaBufferInfo(VADriverContextP ctx, VABufferID buf_id, VABufferType *type,
                        unsigned int *size, unsigned int *num_elements);
VAStatus vlVaLockSurface(VADriverContextP ctx, VASurfaceID surface, unsigned int *fourcc,
                         unsigned int *luma_stride, unsigned int *chroma_u_stride, unsigned int *chroma_v_stride,
                         unsigned int *luma_offset, unsigned int *chroma_u_offset, unsigned int *chroma_v_offset,
                         unsigned int *buffer_name, void **buffer);
VAStatus vlVaUnlockSurface(VADriverContextP ctx, VASurfaceID surface);
VAStatus vlVaCreateSurfaces2(VADriverContextP ctx, unsigned int format, unsigned int width, unsigned int height,
                             VASurfaceID *surfaces, unsigned int num_surfaces, VASurfaceAttrib *attrib_list,
                             unsigned int num_attribs);
VAStatus vlVaQuerySurfaceAttributes(VADriverContextP ctx, VAConfigID config, VASurfaceAttrib *attrib_list,
                                    unsigned int *num_attribs);

VAStatus vlVaAcquireBufferHandle(VADriverContextP ctx, VABufferID buf_id, VABufferInfo *out_buf_info);
VAStatus vlVaReleaseBufferHandle(VADriverContextP ctx, VABufferID buf_id);
VAStatus vlVaExportSurfaceHandle(VADriverContextP ctx, VASurfaceID surface_id, uint32_t mem_type, uint32_t flags, void *descriptor);

VAStatus vlVaQueryVideoProcFilters(VADriverContextP ctx, VAContextID context, VAProcFilterType *filters,
                                   unsigned int *num_filters);
VAStatus vlVaQueryVideoProcFilterCaps(VADriverContextP ctx, VAContextID context, VAProcFilterType type,
                                      void *filter_caps, unsigned int *num_filter_caps);
VAStatus vlVaQueryVideoProcPipelineCaps(VADriverContextP ctx, VAContextID context, VABufferID *filters,
                                        unsigned int num_filters, VAProcPipelineCaps *pipeline_cap);
VAStatus vlVaSyncBuffer(VADriverContextP ctx, VABufferID buf_id, uint64_t timeout_ns);
VAStatus vlVaMapBuffer2(VADriverContextP ctx, VABufferID buf_id, void **pbuf, uint32_t flags);

// internal functions
VAStatus vlVaHandleVAProcPipelineParameterBufferType(vlVaDriver *drv, vlVaContext *context, vlVaBuffer *buf);
VAStatus vlVaHandleSurfaceAllocate(vlVaDriver *drv, vlVaSurface *surface, struct pipe_video_buffer *templat,
                                   const uint64_t *modifiers, unsigned int modifiers_count);
void vlVaGetReferenceFrame(vlVaDriver *drv, VASurfaceID surface_id, struct pipe_video_buffer **ref_frame);
void vlVaHandlePictureParameterBufferMPEG12(vlVaDriver *drv, vlVaContext *context, vlVaBuffer *buf);
void vlVaHandleIQMatrixBufferMPEG12(vlVaContext *context, vlVaBuffer *buf);
void vlVaHandleSliceParameterBufferMPEG12(vlVaContext *context, vlVaBuffer *buf);
void vlVaHandlePictureParameterBufferH264(vlVaDriver *drv, vlVaContext *context, vlVaBuffer *buf);
void vlVaHandleIQMatrixBufferH264(vlVaContext *context, vlVaBuffer *buf);
void vlVaHandleSliceParameterBufferH264(vlVaContext *context, vlVaBuffer *buf);
void vlVaHandlePictureParameterBufferVC1(vlVaDriver *drv, vlVaContext *context, vlVaBuffer *buf);
void vlVaHandleSliceParameterBufferVC1(vlVaContext *context, vlVaBuffer *buf);
void vlVaHandlePictureParameterBufferMPEG4(vlVaDriver *drv, vlVaContext *context, vlVaBuffer *buf);
void vlVaHandleIQMatrixBufferMPEG4(vlVaContext *context, vlVaBuffer *buf);
void vlVaHandleSliceParameterBufferMPEG4(vlVaContext *context, vlVaBuffer *buf);
void vlVaDecoderFixMPEG4Startcode(vlVaContext *context);
void vlVaGetJpegSliceHeader(vlVaContext *context);
void vlVaHandlePictureParameterBufferHEVC(vlVaDriver *drv, vlVaContext *context, vlVaBuffer *buf);
void vlVaHandleIQMatrixBufferHEVC(vlVaContext *context, vlVaBuffer *buf);
void vlVaHandleSliceParameterBufferHEVC(vlVaContext *context, vlVaBuffer *buf);
void vlVaHandlePictureParameterBufferMJPEG(vlVaDriver *drv, vlVaContext *context, vlVaBuffer *buf);
void vlVaHandleIQMatrixBufferMJPEG(vlVaContext *context, vlVaBuffer *buf);
void vlVaHandleHuffmanTableBufferType(vlVaContext *context, vlVaBuffer *buf);
void vlVaHandleSliceParameterBufferMJPEG(vlVaContext *context, vlVaBuffer *buf);
void vlVaHandlePictureParameterBufferVP9(vlVaDriver *drv, vlVaContext *context, vlVaBuffer *buf);
void vlVaHandleSliceParameterBufferVP9(vlVaContext *context, vlVaBuffer *buf);
void vlVaDecoderVP9BitstreamHeader(vlVaContext *context, vlVaBuffer *buf);
void vlVaHandlePictureParameterBufferAV1(vlVaDriver *drv, vlVaContext *context, vlVaBuffer *buf);
void vlVaHandleSliceParameterBufferAV1(vlVaContext *context, vlVaBuffer *buf, unsigned num_slices);
void getEncParamPresetH264(vlVaContext *context);
void getEncParamPresetH265(vlVaContext *context);
void getEncParamPresetAV1(vlVaContext *context);
void vlVaHandleVAEncMiscParameterTypeQualityLevel(struct pipe_enc_quality_modes *p, vlVaQualityBits *in);
VAStatus vlVaHandleVAEncPictureParameterBufferTypeH264(vlVaDriver *drv, vlVaContext *context, vlVaBuffer *buf);
VAStatus vlVaHandleVAEncSliceParameterBufferTypeH264(vlVaDriver *drv, vlVaContext *context, vlVaBuffer *buf);
VAStatus vlVaHandleVAEncSequenceParameterBufferTypeH264(vlVaDriver *drv, vlVaContext *context, vlVaBuffer *buf);
VAStatus vlVaHandleVAEncMiscParameterTypeRateControlH264(vlVaContext *context, VAEncMiscParameterBuffer *buf);
VAStatus vlVaHandleVAEncMiscParameterTypeFrameRateH264(vlVaContext *context, VAEncMiscParameterBuffer *buf);
VAStatus vlVaHandleVAEncPackedHeaderDataBufferTypeH264(vlVaContext *context, vlVaBuffer *buf);
VAStatus vlVaHandleVAEncMiscParameterTypeTemporalLayerH264(vlVaContext *context, VAEncMiscParameterBuffer *buf);
VAStatus vlVaHandleVAEncMiscParameterTypeQualityLevelH264(vlVaContext *context, VAEncMiscParameterBuffer *buf);
VAStatus vlVaHandleVAEncMiscParameterTypeMaxFrameSizeH264(vlVaContext *context, VAEncMiscParameterBuffer *buf);
VAStatus vlVaHandleVAEncMiscParameterTypeHRDH264(vlVaContext *context, VAEncMiscParameterBuffer *buf);
VAStatus vlVaHandleVAEncPictureParameterBufferTypeHEVC(vlVaDriver *drv, vlVaContext *context, vlVaBuffer *buf);
VAStatus vlVaHandleVAEncSliceParameterBufferTypeHEVC(vlVaDriver *drv, vlVaContext *context, vlVaBuffer *buf);
VAStatus vlVaHandleVAEncSequenceParameterBufferTypeHEVC(vlVaDriver *drv, vlVaContext *context, vlVaBuffer *buf);
VAStatus vlVaHandleVAEncMiscParameterTypeRateControlHEVC(vlVaContext *context, VAEncMiscParameterBuffer *buf);
VAStatus vlVaHandleVAEncMiscParameterTypeFrameRateHEVC(vlVaContext *context, VAEncMiscParameterBuffer *buf);
VAStatus vlVaHandleVAEncPackedHeaderDataBufferTypeHEVC(vlVaContext *context, vlVaBuffer *buf);
VAStatus vlVaHandleVAEncMiscParameterTypeQualityLevelHEVC(vlVaContext *context, VAEncMiscParameterBuffer *buf);
VAStatus vlVaHandleVAEncMiscParameterTypeMaxFrameSizeHEVC(vlVaContext *context, VAEncMiscParameterBuffer *buf);
VAStatus vlVaHandleVAEncMiscParameterTypeHRDHEVC(vlVaContext *context, VAEncMiscParameterBuffer *buf);

#if VA_CHECK_VERSION(1, 16, 0)
VAStatus vlVaHandleVAEncSequenceParameterBufferTypeAV1(vlVaDriver *drv, vlVaContext *context, vlVaBuffer *buf);
VAStatus vlVaHandleVAEncPictureParameterBufferTypeAV1(vlVaDriver *drv, vlVaContext *context, vlVaBuffer *buf);
VAStatus vlVaHandleVAEncMiscParameterTypeRateControlAV1(vlVaContext *context, VAEncMiscParameterBuffer *buf);
VAStatus vlVaHandleVAEncPackedHeaderDataBufferTypeAV1(vlVaContext *context, vlVaBuffer *buf);
VAStatus vlVaHandleVAEncMiscParameterTypeFrameRateAV1(vlVaContext *context, VAEncMiscParameterBuffer *buf);
VAStatus vlVaHandleVAEncMiscParameterTypeQualityLevelAV1(vlVaContext *context, VAEncMiscParameterBuffer *buf);
VAStatus vlVaHandleVAEncMiscParameterTypeMaxFrameSizeAV1(vlVaContext *context, VAEncMiscParameterBuffer *buf);
VAStatus vlVaHandleVAEncMiscParameterTypeHRDAV1(vlVaContext *context, VAEncMiscParameterBuffer *buf);
VAStatus vlVaHandleVAEncSliceParameterBufferTypeAV1(vlVaDriver *drv, vlVaContext *context, vlVaBuffer *buf);
#endif
#endif //VA_PRIVATE_H
