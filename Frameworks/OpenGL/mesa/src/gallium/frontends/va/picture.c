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

#include "pipe/p_video_codec.h"

#include "util/u_handle_table.h"
#include "util/u_video.h"
#include "util/u_memory.h"
#include "util/set.h"

#include "util/vl_vlc.h"
#include "vl/vl_winsys.h"

#include "va_private.h"

static void
vlVaSetSurfaceContext(vlVaDriver *drv, vlVaSurface *surf, vlVaContext *context)
{
   if (surf->ctx == context)
      return;

   if (surf->ctx) {
      assert(_mesa_set_search(surf->ctx->surfaces, surf));
      _mesa_set_remove_key(surf->ctx->surfaces, surf);

      /* Only drivers supporting PIPE_VIDEO_ENTRYPOINT_PROCESSING will create
       * decoder for postproc context and thus be able to wait on and destroy
       * the surface fence. On other drivers we need to destroy the fence here
       * otherwise vaQuerySurfaceStatus/vaSyncSurface will fail and we'll also
       * potentially leak the fence.
       */
      if (surf->fence && !context->decoder &&
          context->templat.entrypoint == PIPE_VIDEO_ENTRYPOINT_PROCESSING &&
          surf->ctx->decoder && surf->ctx->decoder->destroy_fence &&
          !drv->pipe->screen->get_video_param(drv->pipe->screen,
                                              PIPE_VIDEO_PROFILE_UNKNOWN,
                                              PIPE_VIDEO_ENTRYPOINT_PROCESSING,
                                              PIPE_VIDEO_CAP_SUPPORTED)) {
         surf->ctx->decoder->destroy_fence(surf->ctx->decoder, surf->fence);
         surf->fence = NULL;
      }
   }

   surf->ctx = context;
   _mesa_set_add(surf->ctx->surfaces, surf);
}

VAStatus
vlVaBeginPicture(VADriverContextP ctx, VAContextID context_id, VASurfaceID render_target)
{
   vlVaDriver *drv;
   vlVaContext *context;
   vlVaSurface *surf;

   if (!ctx)
      return VA_STATUS_ERROR_INVALID_CONTEXT;

   drv = VL_VA_DRIVER(ctx);
   if (!drv)
      return VA_STATUS_ERROR_INVALID_CONTEXT;

   mtx_lock(&drv->mutex);
   context = handle_table_get(drv->htab, context_id);
   if (!context) {
      mtx_unlock(&drv->mutex);
      return VA_STATUS_ERROR_INVALID_CONTEXT;
   }

   if (u_reduce_video_profile(context->templat.profile) == PIPE_VIDEO_FORMAT_MPEG12) {
      context->desc.mpeg12.intra_matrix = NULL;
      context->desc.mpeg12.non_intra_matrix = NULL;
   }

   surf = handle_table_get(drv->htab, render_target);
   if (!surf || !surf->buffer) {
      mtx_unlock(&drv->mutex);
      return VA_STATUS_ERROR_INVALID_SURFACE;
   }

   context->target_id = render_target;
   vlVaSetSurfaceContext(drv, surf, context);
   context->target = surf->buffer;
   context->mjpeg.sampling_factor = 0;

   if (!context->decoder) {

      /* VPP */
      if (context->templat.profile == PIPE_VIDEO_PROFILE_UNKNOWN &&
          context->target->buffer_format != PIPE_FORMAT_B8G8R8A8_UNORM &&
          context->target->buffer_format != PIPE_FORMAT_R8G8B8A8_UNORM &&
          context->target->buffer_format != PIPE_FORMAT_B8G8R8X8_UNORM &&
          context->target->buffer_format != PIPE_FORMAT_R8G8B8X8_UNORM &&
          context->target->buffer_format != PIPE_FORMAT_NV12 &&
          context->target->buffer_format != PIPE_FORMAT_P010 &&
          context->target->buffer_format != PIPE_FORMAT_P016) {
         mtx_unlock(&drv->mutex);
         return VA_STATUS_ERROR_UNIMPLEMENTED;
      }

      if (drv->pipe->screen->get_video_param(drv->pipe->screen,
                              PIPE_VIDEO_PROFILE_UNKNOWN,
                              PIPE_VIDEO_ENTRYPOINT_PROCESSING,
                              PIPE_VIDEO_CAP_SUPPORTED)) {
         context->needs_begin_frame = true;
      }

      mtx_unlock(&drv->mutex);
      return VA_STATUS_SUCCESS;
   }

   if (context->decoder->entrypoint != PIPE_VIDEO_ENTRYPOINT_ENCODE)
      context->needs_begin_frame = true;

   mtx_unlock(&drv->mutex);
   return VA_STATUS_SUCCESS;
}

void
vlVaGetReferenceFrame(vlVaDriver *drv, VASurfaceID surface_id,
                      struct pipe_video_buffer **ref_frame)
{
   vlVaSurface *surf = handle_table_get(drv->htab, surface_id);
   if (surf)
      *ref_frame = surf->buffer;
   else
      *ref_frame = NULL;
}
/*
 * in->quality = 0; without any settings, it is using speed preset
 *                  and no preencode and no vbaq. It is the fastest setting.
 * in->quality = 1; suggested setting, with balanced preset, and
 *                  preencode and vbaq
 * in->quality = others; it is the customized setting
 *                  with valid bit (bit #0) set to "1"
 *                  for example:
 *
 *                  0x3  (balance preset, no pre-encoding, no vbaq)
 *                  0x13 (balanced preset, no pre-encoding, vbaq)
 *                  0x13 (balanced preset, no pre-encoding, vbaq)
 *                  0x9  (speed preset, pre-encoding, no vbaq)
 *                  0x19 (speed preset, pre-encoding, vbaq)
 *
 *                  The quality value has to be treated as a combination
 *                  of preset mode, pre-encoding and vbaq settings.
 *                  The quality and speed could be vary according to
 *                  different settings,
 */
void
vlVaHandleVAEncMiscParameterTypeQualityLevel(struct pipe_enc_quality_modes *p, vlVaQualityBits *in)
{
   if (!in->quality) {
      p->level = 0;
      p->preset_mode = PRESET_MODE_SPEED;
      p->pre_encode_mode = PREENCODING_MODE_DISABLE;
      p->vbaq_mode = VBAQ_DISABLE;

      return;
   }

   if (p->level != in->quality) {
      if (in->quality == 1) {
         p->preset_mode = PRESET_MODE_BALANCE;
         p->pre_encode_mode = PREENCODING_MODE_DEFAULT;
         p->vbaq_mode = VBAQ_AUTO;
      } else {
         p->preset_mode = in->preset_mode > PRESET_MODE_HIGH_QUALITY
            ? PRESET_MODE_HIGH_QUALITY : in->preset_mode;
         p->pre_encode_mode = in->pre_encode_mode;
         p->vbaq_mode = in->vbaq_mode;
      }
   }
   p->level = in->quality;
}

static VAStatus
handlePictureParameterBuffer(vlVaDriver *drv, vlVaContext *context, vlVaBuffer *buf)
{
   VAStatus vaStatus = VA_STATUS_SUCCESS;
   enum pipe_video_format format =
      u_reduce_video_profile(context->templat.profile);

   switch (format) {
   case PIPE_VIDEO_FORMAT_MPEG12:
      vlVaHandlePictureParameterBufferMPEG12(drv, context, buf);
      break;

   case PIPE_VIDEO_FORMAT_MPEG4_AVC:
      vlVaHandlePictureParameterBufferH264(drv, context, buf);
      break;

   case PIPE_VIDEO_FORMAT_VC1:
      vlVaHandlePictureParameterBufferVC1(drv, context, buf);
      break;

   case PIPE_VIDEO_FORMAT_MPEG4:
      vlVaHandlePictureParameterBufferMPEG4(drv, context, buf);
      break;

   case PIPE_VIDEO_FORMAT_HEVC:
      vlVaHandlePictureParameterBufferHEVC(drv, context, buf);
      break;

   case PIPE_VIDEO_FORMAT_JPEG:
      vlVaHandlePictureParameterBufferMJPEG(drv, context, buf);
      break;

   case PIPE_VIDEO_FORMAT_VP9:
      vlVaHandlePictureParameterBufferVP9(drv, context, buf);
      break;

   case PIPE_VIDEO_FORMAT_AV1:
      vlVaHandlePictureParameterBufferAV1(drv, context, buf);
      break;

   default:
      break;
   }

   /* Create the decoder once max_references is known. */
   if (!context->decoder) {
      if (!context->target)
         return VA_STATUS_ERROR_INVALID_CONTEXT;

      if (format == PIPE_VIDEO_FORMAT_MPEG4_AVC)
         context->templat.level = u_get_h264_level(context->templat.width,
            context->templat.height, &context->templat.max_references);

      context->decoder = drv->pipe->create_video_codec(drv->pipe,
         &context->templat);

      if (!context->decoder)
         return VA_STATUS_ERROR_ALLOCATION_FAILED;

      context->needs_begin_frame = true;
   }

   if (format == PIPE_VIDEO_FORMAT_VP9) {
      context->decoder->width =
         context->desc.vp9.picture_parameter.frame_width;
      context->decoder->height =
         context->desc.vp9.picture_parameter.frame_height;
   }

   return vaStatus;
}

static void
handleIQMatrixBuffer(vlVaContext *context, vlVaBuffer *buf)
{
   switch (u_reduce_video_profile(context->templat.profile)) {
   case PIPE_VIDEO_FORMAT_MPEG12:
      vlVaHandleIQMatrixBufferMPEG12(context, buf);
      break;

   case PIPE_VIDEO_FORMAT_MPEG4_AVC:
      vlVaHandleIQMatrixBufferH264(context, buf);
      break;

   case PIPE_VIDEO_FORMAT_MPEG4:
      vlVaHandleIQMatrixBufferMPEG4(context, buf);
      break;

   case PIPE_VIDEO_FORMAT_HEVC:
      vlVaHandleIQMatrixBufferHEVC(context, buf);
      break;

   case PIPE_VIDEO_FORMAT_JPEG:
      vlVaHandleIQMatrixBufferMJPEG(context, buf);
      break;

   default:
      break;
   }
}

static void
handleSliceParameterBuffer(vlVaContext *context, vlVaBuffer *buf, unsigned num_slices)
{
   switch (u_reduce_video_profile(context->templat.profile)) {
   case PIPE_VIDEO_FORMAT_MPEG12:
      vlVaHandleSliceParameterBufferMPEG12(context, buf);
      break;

   case PIPE_VIDEO_FORMAT_VC1:
      vlVaHandleSliceParameterBufferVC1(context, buf);
      break;

   case PIPE_VIDEO_FORMAT_MPEG4_AVC:
      vlVaHandleSliceParameterBufferH264(context, buf);
      break;

   case PIPE_VIDEO_FORMAT_MPEG4:
      vlVaHandleSliceParameterBufferMPEG4(context, buf);
      break;

   case PIPE_VIDEO_FORMAT_HEVC:
      vlVaHandleSliceParameterBufferHEVC(context, buf);
      break;

   case PIPE_VIDEO_FORMAT_JPEG:
      vlVaHandleSliceParameterBufferMJPEG(context, buf);
      break;

   case PIPE_VIDEO_FORMAT_VP9:
      vlVaHandleSliceParameterBufferVP9(context, buf);
      break;

   case PIPE_VIDEO_FORMAT_AV1:
      vlVaHandleSliceParameterBufferAV1(context, buf, num_slices);
      break;

   default:
      break;
   }
}

static unsigned int
bufHasStartcode(vlVaBuffer *buf, unsigned int code, unsigned int bits)
{
   struct vl_vlc vlc = {0};
   int i;

   /* search the first 64 bytes for a startcode */
   vl_vlc_init(&vlc, 1, (const void * const*)&buf->data, &buf->size);
   for (i = 0; i < 64 && vl_vlc_bits_left(&vlc) >= bits; ++i) {
      if (vl_vlc_peekbits(&vlc, bits) == code)
         return 1;
      vl_vlc_eatbits(&vlc, 8);
      vl_vlc_fillbits(&vlc);
   }

   return 0;
}

static void
handleVAProtectedSliceDataBufferType(vlVaContext *context, vlVaBuffer *buf)
{
	uint8_t* encrypted_data = (uint8_t*) buf->data;
        uint8_t* drm_key;

	unsigned int drm_key_size = buf->size;

        drm_key = REALLOC(context->desc.base.decrypt_key,
                          context->desc.base.key_size, drm_key_size);
        if (!drm_key)
            return;
        context->desc.base.decrypt_key = drm_key;
	memcpy(context->desc.base.decrypt_key, encrypted_data, drm_key_size);
	context->desc.base.key_size = drm_key_size;
	context->desc.base.protected_playback = true;
}

static VAStatus
handleVASliceDataBufferType(vlVaContext *context, vlVaBuffer *buf)
{
   enum pipe_video_format format = u_reduce_video_profile(context->templat.profile);
   unsigned num_buffers = 0;
   void * const *buffers[3];
   unsigned sizes[3];
   static const uint8_t start_code_h264[] = { 0x00, 0x00, 0x01 };
   static const uint8_t start_code_h265[] = { 0x00, 0x00, 0x01 };
   static const uint8_t start_code_vc1[] = { 0x00, 0x00, 0x01, 0x0d };
   static const uint8_t eoi_jpeg[] = { 0xff, 0xd9 };

   if (!context->decoder)
      return VA_STATUS_ERROR_INVALID_CONTEXT;

   format = u_reduce_video_profile(context->templat.profile);
   if (!context->desc.base.protected_playback) {
      switch (format) {
      case PIPE_VIDEO_FORMAT_MPEG4_AVC:
         if (bufHasStartcode(buf, 0x000001, 24))
            break;

         buffers[num_buffers] = (void *const)&start_code_h264;
         sizes[num_buffers++] = sizeof(start_code_h264);
         break;
      case PIPE_VIDEO_FORMAT_HEVC:
         if (bufHasStartcode(buf, 0x000001, 24))
            break;

         buffers[num_buffers] = (void *const)&start_code_h265;
         sizes[num_buffers++] = sizeof(start_code_h265);
         break;
      case PIPE_VIDEO_FORMAT_VC1:
         if (bufHasStartcode(buf, 0x0000010d, 32) ||
             bufHasStartcode(buf, 0x0000010c, 32) ||
             bufHasStartcode(buf, 0x0000010b, 32))
            break;

         if (context->decoder->profile == PIPE_VIDEO_PROFILE_VC1_ADVANCED) {
            buffers[num_buffers] = (void *const)&start_code_vc1;
            sizes[num_buffers++] = sizeof(start_code_vc1);
         }
         break;
      case PIPE_VIDEO_FORMAT_MPEG4:
         if (bufHasStartcode(buf, 0x000001, 24))
            break;

         vlVaDecoderFixMPEG4Startcode(context);
         buffers[num_buffers] = (void *)context->mpeg4.start_code;
         sizes[num_buffers++] = context->mpeg4.start_code_size;
         break;
      case PIPE_VIDEO_FORMAT_JPEG:
         if (bufHasStartcode(buf, 0xffd8ffdb, 32))
            break;

         vlVaGetJpegSliceHeader(context);
         buffers[num_buffers] = (void *)context->mjpeg.slice_header;
         sizes[num_buffers++] = context->mjpeg.slice_header_size;
         break;
      case PIPE_VIDEO_FORMAT_VP9:
         if (false == context->desc.base.protected_playback)
            vlVaDecoderVP9BitstreamHeader(context, buf);
         break;
      case PIPE_VIDEO_FORMAT_AV1:
         break;
      default:
         break;
      }
   }

   buffers[num_buffers] = buf->data;
   sizes[num_buffers] = buf->size;
   ++num_buffers;

   if (format == PIPE_VIDEO_FORMAT_JPEG) {
      buffers[num_buffers] = (void *const)&eoi_jpeg;
      sizes[num_buffers++] = sizeof(eoi_jpeg);
   }

   if (context->needs_begin_frame) {
      context->decoder->begin_frame(context->decoder, context->target,
         &context->desc.base);
      context->needs_begin_frame = false;
   }
   context->decoder->decode_bitstream(context->decoder, context->target, &context->desc.base,
      num_buffers, (const void * const*)buffers, sizes);
   return VA_STATUS_SUCCESS;
}

static VAStatus
handleVAEncMiscParameterTypeRateControl(vlVaContext *context, VAEncMiscParameterBuffer *misc)
{
   VAStatus status = VA_STATUS_SUCCESS;

   switch (u_reduce_video_profile(context->templat.profile)) {
   case PIPE_VIDEO_FORMAT_MPEG4_AVC:
      status = vlVaHandleVAEncMiscParameterTypeRateControlH264(context, misc);
      break;

   case PIPE_VIDEO_FORMAT_HEVC:
      status = vlVaHandleVAEncMiscParameterTypeRateControlHEVC(context, misc);
      break;

#if VA_CHECK_VERSION(1, 16, 0)
   case PIPE_VIDEO_FORMAT_AV1:
      status = vlVaHandleVAEncMiscParameterTypeRateControlAV1(context, misc);
      break;
#endif
   default:
      break;
   }

   return status;
}

static VAStatus
handleVAEncMiscParameterTypeFrameRate(vlVaContext *context, VAEncMiscParameterBuffer *misc)
{
   VAStatus status = VA_STATUS_SUCCESS;

   switch (u_reduce_video_profile(context->templat.profile)) {
   case PIPE_VIDEO_FORMAT_MPEG4_AVC:
      status = vlVaHandleVAEncMiscParameterTypeFrameRateH264(context, misc);
      break;

   case PIPE_VIDEO_FORMAT_HEVC:
      status = vlVaHandleVAEncMiscParameterTypeFrameRateHEVC(context, misc);
      break;

#if VA_CHECK_VERSION(1, 16, 0)
   case PIPE_VIDEO_FORMAT_AV1:
      status = vlVaHandleVAEncMiscParameterTypeFrameRateAV1(context, misc);
      break;
#endif
   default:
      break;
   }

   return status;
}

static VAStatus
handleVAEncMiscParameterTypeTemporalLayer(vlVaContext *context, VAEncMiscParameterBuffer *misc)
{
   VAStatus status = VA_STATUS_SUCCESS;

   switch (u_reduce_video_profile(context->templat.profile)) {
   case PIPE_VIDEO_FORMAT_MPEG4_AVC:
      status = vlVaHandleVAEncMiscParameterTypeTemporalLayerH264(context, misc);
      break;

   case PIPE_VIDEO_FORMAT_HEVC:
      break;

   default:
      break;
   }

   return status;
}

static VAStatus
handleVAEncSequenceParameterBufferType(vlVaDriver *drv, vlVaContext *context, vlVaBuffer *buf)
{
   VAStatus status = VA_STATUS_SUCCESS;

   switch (u_reduce_video_profile(context->templat.profile)) {
   case PIPE_VIDEO_FORMAT_MPEG4_AVC:
      status = vlVaHandleVAEncSequenceParameterBufferTypeH264(drv, context, buf);
      break;

   case PIPE_VIDEO_FORMAT_HEVC:
      status = vlVaHandleVAEncSequenceParameterBufferTypeHEVC(drv, context, buf);
      break;

#if VA_CHECK_VERSION(1, 16, 0)
   case PIPE_VIDEO_FORMAT_AV1:
      status = vlVaHandleVAEncSequenceParameterBufferTypeAV1(drv, context, buf);
      break;
#endif

   default:
      break;
   }

   return status;
}

static VAStatus
handleVAEncMiscParameterTypeQualityLevel(vlVaContext *context, VAEncMiscParameterBuffer *misc)
{
   VAStatus status = VA_STATUS_SUCCESS;

   switch (u_reduce_video_profile(context->templat.profile)) {
   case PIPE_VIDEO_FORMAT_MPEG4_AVC:
      status = vlVaHandleVAEncMiscParameterTypeQualityLevelH264(context, misc);
      break;

   case PIPE_VIDEO_FORMAT_HEVC:
      status = vlVaHandleVAEncMiscParameterTypeQualityLevelHEVC(context, misc);
      break;

#if VA_CHECK_VERSION(1, 16, 0)
   case PIPE_VIDEO_FORMAT_AV1:
      status = vlVaHandleVAEncMiscParameterTypeQualityLevelAV1(context, misc);
      break;
#endif

   default:
      break;
   }

   return status;
}

static VAStatus
handleVAEncMiscParameterTypeMaxFrameSize(vlVaContext *context, VAEncMiscParameterBuffer *misc)
{
   VAStatus status = VA_STATUS_SUCCESS;

   switch (u_reduce_video_profile(context->templat.profile)) {
   case PIPE_VIDEO_FORMAT_MPEG4_AVC:
      status = vlVaHandleVAEncMiscParameterTypeMaxFrameSizeH264(context, misc);
      break;

   case PIPE_VIDEO_FORMAT_HEVC:
      status = vlVaHandleVAEncMiscParameterTypeMaxFrameSizeHEVC(context, misc);
      break;

#if VA_CHECK_VERSION(1, 16, 0)
   case PIPE_VIDEO_FORMAT_AV1:
      status = vlVaHandleVAEncMiscParameterTypeMaxFrameSizeAV1(context, misc);
      break;
#endif

   default:
      break;
   }

   return status;
}
static VAStatus
handleVAEncMiscParameterTypeHRD(vlVaContext *context, VAEncMiscParameterBuffer *misc)
{
   VAStatus status = VA_STATUS_SUCCESS;

   switch (u_reduce_video_profile(context->templat.profile)) {
   case PIPE_VIDEO_FORMAT_MPEG4_AVC:
      status = vlVaHandleVAEncMiscParameterTypeHRDH264(context, misc);
      break;

   case PIPE_VIDEO_FORMAT_HEVC:
      status = vlVaHandleVAEncMiscParameterTypeHRDHEVC(context, misc);
      break;

#if VA_CHECK_VERSION(1, 16, 0)
   case PIPE_VIDEO_FORMAT_AV1:
      status = vlVaHandleVAEncMiscParameterTypeHRDAV1(context, misc);
      break;
#endif

   default:
      break;
   }

   return status;
}

static VAStatus
handleVAEncMiscParameterTypeMaxSliceSize(vlVaContext *context, VAEncMiscParameterBuffer *misc)
{
   VAStatus status = VA_STATUS_SUCCESS;
   VAEncMiscParameterMaxSliceSize *max_slice_size_buffer = (VAEncMiscParameterMaxSliceSize *)misc->data;
   switch (u_reduce_video_profile(context->templat.profile)) {
      case PIPE_VIDEO_FORMAT_MPEG4_AVC:
      {
         context->desc.h264enc.slice_mode = PIPE_VIDEO_SLICE_MODE_MAX_SLICE_SICE;
         context->desc.h264enc.max_slice_bytes = max_slice_size_buffer->max_slice_size;
      } break;
      case PIPE_VIDEO_FORMAT_HEVC:
      {
         context->desc.h265enc.slice_mode = PIPE_VIDEO_SLICE_MODE_MAX_SLICE_SICE;
         context->desc.h265enc.max_slice_bytes = max_slice_size_buffer->max_slice_size;
      } break;
      default:
         break;
   }
   return status;
}

static VAStatus
handleVAEncMiscParameterTypeRIR(vlVaContext *context, VAEncMiscParameterBuffer *misc)
{
   VAStatus status = VA_STATUS_SUCCESS;
   struct pipe_enc_intra_refresh *p_intra_refresh = NULL;

   switch (u_reduce_video_profile(context->templat.profile)) {
      case PIPE_VIDEO_FORMAT_MPEG4_AVC:
         p_intra_refresh = &context->desc.h264enc.intra_refresh;
         break;
      case PIPE_VIDEO_FORMAT_HEVC:
         p_intra_refresh = &context->desc.h265enc.intra_refresh;
         break;
#if VA_CHECK_VERSION(1, 16, 0)
      case PIPE_VIDEO_FORMAT_AV1:
         p_intra_refresh = &context->desc.av1enc.intra_refresh;
         break;
#endif
      default:
         p_intra_refresh = NULL;
         break;
   };

   if (p_intra_refresh) {
      VAEncMiscParameterRIR *ir = (VAEncMiscParameterRIR *)misc->data;

      if (ir->rir_flags.value == VA_ENC_INTRA_REFRESH_ROLLING_ROW)
         p_intra_refresh->mode = INTRA_REFRESH_MODE_UNIT_ROWS;
      else if (ir->rir_flags.value == VA_ENC_INTRA_REFRESH_ROLLING_COLUMN)
         p_intra_refresh->mode = INTRA_REFRESH_MODE_UNIT_COLUMNS;
      else if (ir->rir_flags.value) /* if any other values to use the default one*/
         p_intra_refresh->mode = INTRA_REFRESH_MODE_UNIT_COLUMNS;
      else /* if no mode specified then no intra-refresh */
         p_intra_refresh->mode = INTRA_REFRESH_MODE_NONE;

      /* intra refresh should be started with sequence level headers */
      p_intra_refresh->need_sequence_header = 0;
      if (p_intra_refresh->mode) {
         p_intra_refresh->region_size = ir->intra_insert_size;
         p_intra_refresh->offset = ir->intra_insertion_location;
         if (p_intra_refresh->offset == 0)
            p_intra_refresh->need_sequence_header = 1;
      }
   } else {
      p_intra_refresh->mode = INTRA_REFRESH_MODE_NONE;
      p_intra_refresh->region_size = 0;
      p_intra_refresh->offset = 0;
      p_intra_refresh->need_sequence_header = 0;
   }

   return status;
}

static VAStatus
handleVAEncMiscParameterTypeROI(vlVaContext *context, VAEncMiscParameterBuffer *misc)
{
   VAStatus status = VA_STATUS_SUCCESS;
   struct pipe_enc_roi *proi= NULL;
   switch (u_reduce_video_profile(context->templat.profile)) {
      case PIPE_VIDEO_FORMAT_MPEG4_AVC:
         proi = &context->desc.h264enc.roi;
         break;
      case PIPE_VIDEO_FORMAT_HEVC:
         proi = &context->desc.h265enc.roi;
         break;
#if VA_CHECK_VERSION(1, 16, 0)
      case PIPE_VIDEO_FORMAT_AV1:
         proi = &context->desc.av1enc.roi;
         break;
#endif
      default:
         break;
   };

   if (proi) {
      VAEncMiscParameterBufferROI *roi = (VAEncMiscParameterBufferROI *)misc->data;
      /* do not support priority type, and the maximum region is 32  */
      if ((roi->num_roi > 0 && roi->roi_flags.bits.roi_value_is_qp_delta == 0)
           || roi->num_roi > PIPE_ENC_ROI_REGION_NUM_MAX)
         status = VA_STATUS_ERROR_FLAG_NOT_SUPPORTED;
      else {
         uint32_t i;
         VAEncROI *src = roi->roi;

         proi->num = roi->num_roi;
         for (i = 0; i < roi->num_roi; i++) {
            proi->region[i].valid = true;
            proi->region[i].x = src->roi_rectangle.x;
            proi->region[i].y = src->roi_rectangle.y;
            proi->region[i].width = src->roi_rectangle.width;
            proi->region[i].height = src->roi_rectangle.height;
            proi->region[i].qp_value = (int32_t)CLAMP(src->roi_value, roi->min_delta_qp, roi->max_delta_qp);
            src++;
         }

         for (; i < PIPE_ENC_ROI_REGION_NUM_MAX; i++)
            proi->region[i].valid = false;
      }
   }

   return status;
}

static VAStatus
handleVAEncMiscParameterBufferType(vlVaContext *context, vlVaBuffer *buf)
{
   VAStatus vaStatus = VA_STATUS_SUCCESS;
   VAEncMiscParameterBuffer *misc;
   misc = buf->data;

   switch (misc->type) {
   case VAEncMiscParameterTypeRateControl:
      vaStatus = handleVAEncMiscParameterTypeRateControl(context, misc);
      break;

   case VAEncMiscParameterTypeFrameRate:
      vaStatus = handleVAEncMiscParameterTypeFrameRate(context, misc);
      break;

   case VAEncMiscParameterTypeTemporalLayerStructure:
      vaStatus = handleVAEncMiscParameterTypeTemporalLayer(context, misc);
      break;

   case VAEncMiscParameterTypeQualityLevel:
      vaStatus = handleVAEncMiscParameterTypeQualityLevel(context, misc);
      break;

   case VAEncMiscParameterTypeMaxFrameSize:
      vaStatus = handleVAEncMiscParameterTypeMaxFrameSize(context, misc);
      break;

   case VAEncMiscParameterTypeHRD:
      vaStatus = handleVAEncMiscParameterTypeHRD(context, misc);
      break;

   case VAEncMiscParameterTypeRIR:
      vaStatus = handleVAEncMiscParameterTypeRIR(context, misc);
      break;

   case VAEncMiscParameterTypeMaxSliceSize:
      vaStatus = handleVAEncMiscParameterTypeMaxSliceSize(context, misc);
      break;

   case VAEncMiscParameterTypeROI:
      vaStatus = handleVAEncMiscParameterTypeROI(context, misc);
      break;

   default:
      break;
   }

   return vaStatus;
}

static VAStatus
handleVAEncPictureParameterBufferType(vlVaDriver *drv, vlVaContext *context, vlVaBuffer *buf)
{
   VAStatus status = VA_STATUS_SUCCESS;

   switch (u_reduce_video_profile(context->templat.profile)) {
   case PIPE_VIDEO_FORMAT_MPEG4_AVC:
      status = vlVaHandleVAEncPictureParameterBufferTypeH264(drv, context, buf);
      break;

   case PIPE_VIDEO_FORMAT_HEVC:
      status = vlVaHandleVAEncPictureParameterBufferTypeHEVC(drv, context, buf);
      break;

#if VA_CHECK_VERSION(1, 16, 0)
   case PIPE_VIDEO_FORMAT_AV1:
      status = vlVaHandleVAEncPictureParameterBufferTypeAV1(drv, context, buf);
      break;
#endif

   default:
      break;
   }

   return status;
}

static VAStatus
handleVAEncSliceParameterBufferType(vlVaDriver *drv, vlVaContext *context, vlVaBuffer *buf)
{
   VAStatus status = VA_STATUS_SUCCESS;

   switch (u_reduce_video_profile(context->templat.profile)) {
   case PIPE_VIDEO_FORMAT_MPEG4_AVC:
      status = vlVaHandleVAEncSliceParameterBufferTypeH264(drv, context, buf);
      break;

   case PIPE_VIDEO_FORMAT_HEVC:
      status = vlVaHandleVAEncSliceParameterBufferTypeHEVC(drv, context, buf);
      break;

#if VA_CHECK_VERSION(1, 16, 0)
   case PIPE_VIDEO_FORMAT_AV1:
      status = vlVaHandleVAEncSliceParameterBufferTypeAV1(drv, context, buf);
      break;
#endif

   default:
      break;
   }

   return status;
}

static VAStatus
handleVAEncPackedHeaderParameterBufferType(vlVaContext *context, vlVaBuffer *buf)
{
   VAStatus status = VA_STATUS_SUCCESS;
   VAEncPackedHeaderParameterBuffer *param = buf->data;

   context->packed_header_emulation_bytes = param->has_emulation_bytes;

   switch (u_reduce_video_profile(context->templat.profile)) {
   case PIPE_VIDEO_FORMAT_MPEG4_AVC:
      if (param->type == VAEncPackedHeaderSequence)
         context->packed_header_type = param->type;
      else
         status = VA_STATUS_ERROR_UNIMPLEMENTED;
      break;
   case PIPE_VIDEO_FORMAT_HEVC:
      if (param->type == VAEncPackedHeaderSequence)
         context->packed_header_type = param->type;
      else
         status = VA_STATUS_ERROR_UNIMPLEMENTED;
      break;
   case PIPE_VIDEO_FORMAT_AV1:
         context->packed_header_type = param->type;
      break;

   default:
      return VA_STATUS_ERROR_UNIMPLEMENTED;
   }

   return status;
}

static VAStatus
handleVAEncPackedHeaderDataBufferType(vlVaContext *context, vlVaBuffer *buf)
{
   VAStatus status = VA_STATUS_SUCCESS;

   switch (u_reduce_video_profile(context->templat.profile)) {
   case PIPE_VIDEO_FORMAT_MPEG4_AVC:
      if (context->packed_header_type != VAEncPackedHeaderSequence)
         return VA_STATUS_ERROR_UNIMPLEMENTED;

      status = vlVaHandleVAEncPackedHeaderDataBufferTypeH264(context, buf);
      break;

   case PIPE_VIDEO_FORMAT_HEVC:
      if (context->packed_header_type != VAEncPackedHeaderSequence)
         return VA_STATUS_ERROR_UNIMPLEMENTED;

      status = vlVaHandleVAEncPackedHeaderDataBufferTypeHEVC(context, buf);
      break;

#if VA_CHECK_VERSION(1, 16, 0)
   case PIPE_VIDEO_FORMAT_AV1:
      status = vlVaHandleVAEncPackedHeaderDataBufferTypeAV1(context, buf);
      break;
#endif

   default:
      break;
   }

   return status;
}

static VAStatus
handleVAStatsStatisticsBufferType(VADriverContextP ctx, vlVaContext *context, vlVaBuffer *buf)
{
   if (context->decoder->entrypoint != PIPE_VIDEO_ENTRYPOINT_ENCODE)
      return VA_STATUS_ERROR_UNIMPLEMENTED;

   vlVaDriver *drv;
   drv = VL_VA_DRIVER(ctx);

   if (!drv)
      return VA_STATUS_ERROR_INVALID_CONTEXT;

   if (!buf->derived_surface.resource)
      buf->derived_surface.resource = pipe_buffer_create(drv->pipe->screen, PIPE_BIND_VERTEX_BUFFER,
                                            PIPE_USAGE_STREAM, buf->size);

   context->target->statistics_data = buf->derived_surface.resource;

   return VA_STATUS_SUCCESS;
}

VAStatus
vlVaRenderPicture(VADriverContextP ctx, VAContextID context_id, VABufferID *buffers, int num_buffers)
{
   vlVaDriver *drv;
   vlVaContext *context;
   VAStatus vaStatus = VA_STATUS_SUCCESS;

   unsigned i;
   unsigned slice_idx = 0;
   vlVaBuffer *seq_param_buf = NULL;

   if (!ctx)
      return VA_STATUS_ERROR_INVALID_CONTEXT;

   drv = VL_VA_DRIVER(ctx);
   if (!drv)
      return VA_STATUS_ERROR_INVALID_CONTEXT;

   mtx_lock(&drv->mutex);
   context = handle_table_get(drv->htab, context_id);
   if (!context) {
      mtx_unlock(&drv->mutex);
      return VA_STATUS_ERROR_INVALID_CONTEXT;
   }

   /* Always process VAProtectedSliceDataBufferType first because it changes the state */
   for (i = 0; i < num_buffers; ++i) {
      vlVaBuffer *buf = handle_table_get(drv->htab, buffers[i]);
      if (!buf) {
         mtx_unlock(&drv->mutex);
         return VA_STATUS_ERROR_INVALID_BUFFER;
      }

      if (buf->type == VAProtectedSliceDataBufferType)
         handleVAProtectedSliceDataBufferType(context, buf);
      else if (buf->type == VAEncSequenceParameterBufferType)
         seq_param_buf = buf;
   }

   /* Now process VAEncSequenceParameterBufferType where the encoder is created
    * and some default parameters are set to make sure it won't overwrite
    * parameters already set by application from earlier buffers. */
   if (seq_param_buf)
      vaStatus = handleVAEncSequenceParameterBufferType(drv, context, seq_param_buf);

   for (i = 0; i < num_buffers && vaStatus == VA_STATUS_SUCCESS; ++i) {
      vlVaBuffer *buf = handle_table_get(drv->htab, buffers[i]);

      switch (buf->type) {
      case VAPictureParameterBufferType:
         vaStatus = handlePictureParameterBuffer(drv, context, buf);
         break;

      case VAIQMatrixBufferType:
         handleIQMatrixBuffer(context, buf);
         break;

      case VASliceParameterBufferType:
      {
         /* Some apps like gstreamer send all the slices at once
            and some others send individual VASliceParameterBufferType buffers

            slice_idx is the zero based number of total slices received
               before this call to handleSliceParameterBuffer
         */
         handleSliceParameterBuffer(context, buf, slice_idx);
         slice_idx += buf->num_elements;
      } break;

      case VASliceDataBufferType:
         vaStatus = handleVASliceDataBufferType(context, buf);
         break;

      case VAProcPipelineParameterBufferType:
         vaStatus = vlVaHandleVAProcPipelineParameterBufferType(drv, context, buf);
         break;

      case VAEncMiscParameterBufferType:
         vaStatus = handleVAEncMiscParameterBufferType(context, buf);
         break;

      case VAEncPictureParameterBufferType:
         vaStatus = handleVAEncPictureParameterBufferType(drv, context, buf);
         break;

      case VAEncSliceParameterBufferType:
         vaStatus = handleVAEncSliceParameterBufferType(drv, context, buf);
         break;

      case VAHuffmanTableBufferType:
         vlVaHandleHuffmanTableBufferType(context, buf);
         break;

      case VAEncPackedHeaderParameterBufferType:
         handleVAEncPackedHeaderParameterBufferType(context, buf);
         break;
      case VAEncPackedHeaderDataBufferType:
         handleVAEncPackedHeaderDataBufferType(context, buf);
         break;

      case VAStatsStatisticsBufferType:
         handleVAStatsStatisticsBufferType(ctx, context, buf);
         break;

      default:
         break;
      }
   }
   mtx_unlock(&drv->mutex);

   return vaStatus;
}

static bool vlVaQueryApplyFilmGrainAV1(vlVaContext *context,
                                 int *output_id,
                                 struct pipe_video_buffer ***out_target)
{
   struct pipe_av1_picture_desc *av1 = NULL;

   if (u_reduce_video_profile(context->templat.profile) != PIPE_VIDEO_FORMAT_AV1 ||
       context->decoder->entrypoint != PIPE_VIDEO_ENTRYPOINT_BITSTREAM)
      return false;

   av1 = &context->desc.av1;
   if (!av1->picture_parameter.film_grain_info.film_grain_info_fields.apply_grain)
      return false;

   *output_id = av1->picture_parameter.current_frame_id;
   *out_target = &av1->film_grain_target;
   return true;
}

VAStatus
vlVaEndPicture(VADriverContextP ctx, VAContextID context_id)
{
   vlVaDriver *drv;
   vlVaContext *context;
   vlVaBuffer *coded_buf;
   vlVaSurface *surf;
   void *feedback = NULL;
   struct pipe_screen *screen;
   bool supported;
   bool realloc = false;
   bool apply_av1_fg = false;
   enum pipe_format format;
   struct pipe_video_buffer **out_target;
   int output_id;

   if (!ctx)
      return VA_STATUS_ERROR_INVALID_CONTEXT;

   drv = VL_VA_DRIVER(ctx);
   if (!drv)
      return VA_STATUS_ERROR_INVALID_CONTEXT;

   mtx_lock(&drv->mutex);
   context = handle_table_get(drv->htab, context_id);
   mtx_unlock(&drv->mutex);
   if (!context)
      return VA_STATUS_ERROR_INVALID_CONTEXT;

   if (!context->decoder) {
      if (context->templat.profile != PIPE_VIDEO_PROFILE_UNKNOWN)
         return VA_STATUS_ERROR_INVALID_CONTEXT;

      /* VPP */
      return VA_STATUS_SUCCESS;
   }

   output_id = context->target_id;
   out_target = &context->target;
   apply_av1_fg = vlVaQueryApplyFilmGrainAV1(context, &output_id, &out_target);

   mtx_lock(&drv->mutex);
   surf = handle_table_get(drv->htab, output_id);
   if (!surf || !surf->buffer) {
      mtx_unlock(&drv->mutex);
      return VA_STATUS_ERROR_INVALID_SURFACE;
   }

   if (apply_av1_fg) {
      vlVaSetSurfaceContext(drv, surf, context);
      *out_target = surf->buffer;
   }

   context->mpeg4.frame_num++;

   screen = context->decoder->context->screen;
   supported = screen->get_video_param(screen, context->decoder->profile,
                                       context->decoder->entrypoint,
                                       surf->buffer->interlaced ?
                                       PIPE_VIDEO_CAP_SUPPORTS_INTERLACED :
                                       PIPE_VIDEO_CAP_SUPPORTS_PROGRESSIVE);

   if (!supported) {
      surf->templat.interlaced = screen->get_video_param(screen,
                                       context->decoder->profile,
                                       context->decoder->entrypoint,
                                       PIPE_VIDEO_CAP_PREFERS_INTERLACED);
      realloc = true;
   }

   format = screen->get_video_param(screen, context->decoder->profile,
                                    context->decoder->entrypoint,
                                    PIPE_VIDEO_CAP_PREFERED_FORMAT);

   if (surf->buffer->buffer_format != format &&
       surf->buffer->buffer_format == PIPE_FORMAT_NV12) {
      /* check originally as NV12 only */
      surf->templat.buffer_format = format;
      realloc = true;
   }

   if (u_reduce_video_profile(context->templat.profile) == PIPE_VIDEO_FORMAT_JPEG) {
      if (surf->buffer->buffer_format == PIPE_FORMAT_NV12 &&
          context->mjpeg.sampling_factor != MJPEG_SAMPLING_FACTOR_NV12) {
         /* workaround to reallocate surface buffer with right format
          * if it doesnt match with sampling_factor. ffmpeg doesnt
          * use VASurfaceAttribPixelFormat and defaults to NV12.
          */
         switch (context->mjpeg.sampling_factor) {
            case MJPEG_SAMPLING_FACTOR_YUV422:
            case MJPEG_SAMPLING_FACTOR_YUY2:
               surf->templat.buffer_format = PIPE_FORMAT_YUYV;
               break;
            case MJPEG_SAMPLING_FACTOR_YUV444:
               surf->templat.buffer_format = PIPE_FORMAT_Y8_U8_V8_444_UNORM;
               break;
            case MJPEG_SAMPLING_FACTOR_YUV400:
               surf->templat.buffer_format = PIPE_FORMAT_Y8_400_UNORM;
               break;
            default:
               mtx_unlock(&drv->mutex);
               return VA_STATUS_ERROR_INVALID_SURFACE;
         }
         realloc = true;
      }
      /* check if format is supported before proceeding with realloc,
       * also avoid submission if hardware doesnt support the format and
       * applcation failed to check the supported rt_formats.
       */
      if (!screen->is_video_format_supported(screen, surf->templat.buffer_format,
          PIPE_VIDEO_PROFILE_JPEG_BASELINE, PIPE_VIDEO_ENTRYPOINT_BITSTREAM)) {
         mtx_unlock(&drv->mutex);
         return VA_STATUS_ERROR_INVALID_SURFACE;
      }
   }

   if ((bool)(surf->templat.bind & PIPE_BIND_PROTECTED) != context->desc.base.protected_playback) {
      if (context->desc.base.protected_playback) {
         surf->templat.bind |= PIPE_BIND_PROTECTED;
      }
      else
         surf->templat.bind &= ~PIPE_BIND_PROTECTED;
      realloc = true;
   }

   if (u_reduce_video_profile(context->templat.profile) == PIPE_VIDEO_FORMAT_AV1 &&
       surf->buffer->buffer_format == PIPE_FORMAT_NV12 &&
       context->decoder->entrypoint == PIPE_VIDEO_ENTRYPOINT_BITSTREAM) {
      if (context->desc.av1.picture_parameter.bit_depth_idx == 1) {
         surf->templat.buffer_format = PIPE_FORMAT_P010;
         realloc = true;
      }
   }

   if (realloc) {
      struct pipe_video_buffer *old_buf = surf->buffer;

      if (vlVaHandleSurfaceAllocate(drv, surf, &surf->templat, NULL, 0) != VA_STATUS_SUCCESS) {
         mtx_unlock(&drv->mutex);
         return VA_STATUS_ERROR_ALLOCATION_FAILED;
      }

      if (context->decoder->entrypoint == PIPE_VIDEO_ENTRYPOINT_ENCODE) {
         if (old_buf->interlaced) {
            struct u_rect src_rect, dst_rect;

            dst_rect.x0 = src_rect.x0 = 0;
            dst_rect.y0 = src_rect.y0 = 0;
            dst_rect.x1 = src_rect.x1 = surf->templat.width;
            dst_rect.y1 = src_rect.y1 = surf->templat.height;
            vl_compositor_yuv_deint_full(&drv->cstate, &drv->compositor,
                                         old_buf, surf->buffer,
                                         &src_rect, &dst_rect, VL_COMPOSITOR_WEAVE);
         } else {
            /* Can't convert from progressive to interlaced yet */
            mtx_unlock(&drv->mutex);
            return VA_STATUS_ERROR_INVALID_SURFACE;
         }
      }

      old_buf->destroy(old_buf);
      *out_target = surf->buffer;
   }

   if (context->decoder->entrypoint == PIPE_VIDEO_ENTRYPOINT_ENCODE) {
      context->desc.base.fence = &surf->fence;
      struct pipe_screen *screen = context->decoder->context->screen;
      coded_buf = context->coded_buf;
      if (u_reduce_video_profile(context->templat.profile) == PIPE_VIDEO_FORMAT_MPEG4_AVC)
         context->desc.h264enc.frame_num_cnt++;

      /* keep other path the same way */
      if (!screen->get_video_param(screen, context->templat.profile,
                                  context->decoder->entrypoint,
                                  PIPE_VIDEO_CAP_ENC_QUALITY_LEVEL)) {

         if (u_reduce_video_profile(context->templat.profile) == PIPE_VIDEO_FORMAT_MPEG4_AVC)
            getEncParamPresetH264(context);
         else if (u_reduce_video_profile(context->templat.profile) == PIPE_VIDEO_FORMAT_HEVC)
            getEncParamPresetH265(context);
      }

      context->desc.base.input_format = surf->buffer->buffer_format;
      context->desc.base.input_full_range = surf->full_range;
      context->desc.base.output_format = surf->encoder_format;

      int driver_metadata_support = drv->pipe->screen->get_video_param(drv->pipe->screen,
                                                                       context->decoder->profile,
                                                                       context->decoder->entrypoint,
                                                                       PIPE_VIDEO_CAP_ENC_SUPPORTS_FEEDBACK_METADATA);
      if (u_reduce_video_profile(context->templat.profile) == PIPE_VIDEO_FORMAT_MPEG4_AVC)
         context->desc.h264enc.requested_metadata = driver_metadata_support;
      else if (u_reduce_video_profile(context->templat.profile) == PIPE_VIDEO_FORMAT_HEVC)
         context->desc.h265enc.requested_metadata = driver_metadata_support;
      else if (u_reduce_video_profile(context->templat.profile) == PIPE_VIDEO_FORMAT_AV1)
         context->desc.av1enc.requested_metadata = driver_metadata_support;

      context->decoder->begin_frame(context->decoder, context->target, &context->desc.base);
      context->decoder->encode_bitstream(context->decoder, context->target,
                                         coded_buf->derived_surface.resource, &feedback);
      coded_buf->feedback = feedback;
      coded_buf->ctx = context_id;
      surf->feedback = feedback;
      surf->coded_buf = coded_buf;
      coded_buf->associated_encode_input_surf = context->target_id;
   } else if (context->decoder->entrypoint == PIPE_VIDEO_ENTRYPOINT_BITSTREAM) {
      context->desc.base.fence = &surf->fence;
   } else if (context->decoder->entrypoint == PIPE_VIDEO_ENTRYPOINT_PROCESSING) {
      context->desc.base.fence = &surf->fence;
   }

   context->decoder->end_frame(context->decoder, context->target, &context->desc.base);

   if (drv->pipe->screen->get_video_param(drv->pipe->screen,
                           context->decoder->profile,
                           context->decoder->entrypoint,
                           PIPE_VIDEO_CAP_REQUIRES_FLUSH_ON_END_FRAME))
      context->decoder->flush(context->decoder);
   else {
      if (context->decoder->entrypoint == PIPE_VIDEO_ENTRYPOINT_ENCODE &&
         u_reduce_video_profile(context->templat.profile) == PIPE_VIDEO_FORMAT_MPEG4_AVC) {
         int idr_period = context->desc.h264enc.gop_size / context->gop_coeff;
         int p_remain_in_idr = idr_period - context->desc.h264enc.frame_num;
         surf->frame_num_cnt = context->desc.h264enc.frame_num_cnt;
         surf->force_flushed = false;
         if (context->first_single_submitted) {
            context->decoder->flush(context->decoder);
            context->first_single_submitted = false;
            surf->force_flushed = true;
         }
         if (p_remain_in_idr == 1) {
            if ((context->desc.h264enc.frame_num_cnt % 2) != 0) {
               context->decoder->flush(context->decoder);
               context->first_single_submitted = true;
            }
            else
               context->first_single_submitted = false;
            surf->force_flushed = true;
         }
      }
   }

   if (context->decoder->get_feedback_fence &&
       !context->decoder->get_feedback_fence(context->decoder, feedback)) {
         mtx_unlock(&drv->mutex);
         return VA_STATUS_ERROR_OPERATION_FAILED;
   }

   /* Update frame_num disregarding PIPE_VIDEO_CAP_REQUIRES_FLUSH_ON_END_FRAME check above */
   if (context->decoder->entrypoint == PIPE_VIDEO_ENTRYPOINT_ENCODE) {
      if ((u_reduce_video_profile(context->templat.profile) == PIPE_VIDEO_FORMAT_MPEG4_AVC)
         && (!context->desc.h264enc.not_referenced))
         context->desc.h264enc.frame_num++;
      else if (u_reduce_video_profile(context->templat.profile) == PIPE_VIDEO_FORMAT_HEVC)
         context->desc.h265enc.frame_num++;
      else if (u_reduce_video_profile(context->templat.profile) == PIPE_VIDEO_FORMAT_AV1)
         context->desc.av1enc.frame_num++;
   }

   mtx_unlock(&drv->mutex);
   return VA_STATUS_SUCCESS;
}
