/*
 * Copyright © Microsoft Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "d3d12_context.h"
#include "d3d12_format.h"
#include "d3d12_resource.h"
#include "d3d12_screen.h"
#include "d3d12_surface.h"
#include "d3d12_video_dec.h"
#if VIDEO_CODEC_H264DEC
#include "d3d12_video_dec_h264.h"
#endif
#if VIDEO_CODEC_H265DEC
#include "d3d12_video_dec_hevc.h"
#endif
#if VIDEO_CODEC_AV1DEC
#include "d3d12_video_dec_av1.h"
#endif
#if VIDEO_CODEC_VP9DEC
#include "d3d12_video_dec_vp9.h"
#endif
#include "d3d12_video_buffer.h"
#include "d3d12_residency.h"

#include "vl/vl_video_buffer.h"
#include "util/format/u_format.h"
#include "util/u_inlines.h"
#include "util/u_memory.h"
#include "util/u_video.h"

uint64_t
d3d12_video_decoder_pool_current_index(struct d3d12_video_decoder *pD3D12Dec)
{
   return pD3D12Dec->m_fenceValue % D3D12_VIDEO_DEC_ASYNC_DEPTH;
}

struct pipe_video_codec *
d3d12_video_create_decoder(struct pipe_context *context, const struct pipe_video_codec *codec)
{
   ///
   /// Initialize d3d12_video_decoder
   ///


   // Not using new doesn't call ctor and the initializations in the class declaration are lost
   struct d3d12_video_decoder *pD3D12Dec = new d3d12_video_decoder;

   pD3D12Dec->m_inflightResourcesPool.resize(D3D12_VIDEO_DEC_ASYNC_DEPTH, { 0 });

   pD3D12Dec->base = *codec;
   pD3D12Dec->m_screen = context->screen;

   pD3D12Dec->base.context = context;
   pD3D12Dec->base.width = codec->width;
   pD3D12Dec->base.height = codec->height;
   // Only fill methods that are supported by the d3d12 decoder, leaving null the rest (ie. encode_* / decode_macroblock
   // / get_feedback for encode)
   pD3D12Dec->base.destroy = d3d12_video_decoder_destroy;
   pD3D12Dec->base.begin_frame = d3d12_video_decoder_begin_frame;
   pD3D12Dec->base.decode_bitstream = d3d12_video_decoder_decode_bitstream;
   pD3D12Dec->base.end_frame = d3d12_video_decoder_end_frame;
   pD3D12Dec->base.flush = d3d12_video_decoder_flush;
   pD3D12Dec->base.get_decoder_fence = d3d12_video_decoder_get_decoder_fence;

   pD3D12Dec->m_decodeFormat = d3d12_convert_pipe_video_profile_to_dxgi_format(codec->profile);
   pD3D12Dec->m_d3d12DecProfileType = d3d12_video_decoder_convert_pipe_video_profile_to_profile_type(codec->profile);
   pD3D12Dec->m_d3d12DecProfile = d3d12_video_decoder_convert_pipe_video_profile_to_d3d12_profile(codec->profile);

   ///
   /// Try initializing D3D12 Video device and check for device caps
   ///

   struct d3d12_context *pD3D12Ctx = (struct d3d12_context *) context;
   pD3D12Dec->m_pD3D12Screen = d3d12_screen(pD3D12Ctx->base.screen);

   ///
   /// Create decode objects
   ///
   HRESULT hr = S_OK;
   if (FAILED(pD3D12Dec->m_pD3D12Screen->dev->QueryInterface(
          IID_PPV_ARGS(pD3D12Dec->m_spD3D12VideoDevice.GetAddressOf())))) {
      debug_printf("[d3d12_video_decoder] d3d12_video_create_decoder - D3D12 Device has no Video support\n");
      goto failed;
   }

   if (!d3d12_video_decoder_check_caps_and_create_decoder(pD3D12Dec->m_pD3D12Screen, pD3D12Dec)) {
      debug_printf("[d3d12_video_decoder] d3d12_video_create_decoder - Failure on "
                   "d3d12_video_decoder_check_caps_and_create_decoder\n");
      goto failed;
   }

   if (!d3d12_video_decoder_create_command_objects(pD3D12Dec->m_pD3D12Screen, pD3D12Dec)) {
      debug_printf(
         "[d3d12_video_decoder] d3d12_video_create_decoder - Failure on d3d12_video_decoder_create_command_objects\n");
      goto failed;
   }

   if (!d3d12_video_decoder_create_video_state_buffers(pD3D12Dec->m_pD3D12Screen, pD3D12Dec)) {
      debug_printf("[d3d12_video_decoder] d3d12_video_create_decoder - Failure on "
                   "d3d12_video_decoder_create_video_state_buffers\n");
      goto failed;
   }

   pD3D12Dec->m_decodeFormatInfo = { pD3D12Dec->m_decodeFormat };
   hr = pD3D12Dec->m_pD3D12Screen->dev->CheckFeatureSupport(D3D12_FEATURE_FORMAT_INFO,
                                                            &pD3D12Dec->m_decodeFormatInfo,
                                                            sizeof(pD3D12Dec->m_decodeFormatInfo));
   if (FAILED(hr)) {
      debug_printf("CheckFeatureSupport failed with HR %x\n", hr);
      goto failed;
   }

   return &pD3D12Dec->base;

failed:
   if (pD3D12Dec != nullptr) {
      d3d12_video_decoder_destroy((struct pipe_video_codec *) pD3D12Dec);
   }

   return nullptr;
}

/**
 * Destroys a d3d12_video_decoder
 * Call destroy_XX for applicable XX nested member types before deallocating
 * Destroy methods should check != nullptr on their input target argument as this method can be called as part of
 * cleanup from failure on the creation method
 */
void
d3d12_video_decoder_destroy(struct pipe_video_codec *codec)
{
   if (codec == nullptr) {
      return;
   }

   struct d3d12_video_decoder *pD3D12Dec = (struct d3d12_video_decoder *) codec;
   // Flush and wait for completion of any in-flight GPU work before destroying objects
   d3d12_video_decoder_flush(codec);
   if (pD3D12Dec->m_fenceValue > 1 /* Check we submitted at least one frame */) {
      auto decode_queue_completion_fence = pD3D12Dec->m_inflightResourcesPool[(pD3D12Dec->m_fenceValue - 1u) % D3D12_VIDEO_DEC_ASYNC_DEPTH].m_FenceData;
      d3d12_video_decoder_sync_completion(codec, decode_queue_completion_fence.cmdqueue_fence, decode_queue_completion_fence.value, OS_TIMEOUT_INFINITE);
      struct pipe_fence_handle *context_queue_completion_fence = NULL;
      pD3D12Dec->base.context->flush(pD3D12Dec->base.context, &context_queue_completion_fence, PIPE_FLUSH_ASYNC | PIPE_FLUSH_HINT_FINISH);
      pD3D12Dec->m_pD3D12Screen->base.fence_finish(&pD3D12Dec->m_pD3D12Screen->base, NULL, context_queue_completion_fence, OS_TIMEOUT_INFINITE);
      pD3D12Dec->m_pD3D12Screen->base.fence_reference(&pD3D12Dec->m_pD3D12Screen->base, &context_queue_completion_fence, NULL);
   }

   //
   // Destroys a decoder
   // Call destroy_XX for applicable XX nested member types before deallocating
   // Destroy methods should check != nullptr on their input target argument as this method can be called as part of
   // cleanup from failure on the creation method
   //

   // No need for d3d12_destroy_video_objects
   //    All the objects created here are smart pointer members of d3d12_video_decoder
   // No need for d3d12_destroy_video_decoder_and_heap
   //    All the objects created here are smart pointer members of d3d12_video_decoder
   // No need for d3d12_destroy_video_dpbmanagers
   //    All the objects created here are smart pointer members of d3d12_video_decoder

   // No need for m_pD3D12Screen as it is not managed by d3d12_video_decoder

   // Call dtor to make ComPtr work
   delete pD3D12Dec;
}

/**
 * start decoding of a new frame
 */
void
d3d12_video_decoder_begin_frame(struct pipe_video_codec *codec,
                                struct pipe_video_buffer *target,
                                struct pipe_picture_desc *picture)
{
   // Do nothing here. Initialize happens on decoder creation, re-config (if any) happens in
   // d3d12_video_decoder_decode_bitstream
   struct d3d12_video_decoder *pD3D12Dec = (struct d3d12_video_decoder *) codec;
   assert(pD3D12Dec);

   ///
   /// Wait here to make sure the next in flight resource set is empty before using it
   ///
   uint64_t fenceValueToWaitOn = static_cast<uint64_t>(
      std::max(static_cast<int64_t>(0l),
               static_cast<int64_t>(pD3D12Dec->m_fenceValue) - static_cast<int64_t>(D3D12_VIDEO_DEC_ASYNC_DEPTH)));

   debug_printf("[d3d12_video_decoder] d3d12_video_decoder_begin_frame Waiting for completion of in flight resource "
                "sets with previous work with fenceValue: %" PRIu64 "\n",
                fenceValueToWaitOn);

   ASSERTED bool wait_res =
      d3d12_video_decoder_sync_completion(codec, pD3D12Dec->m_spFence.Get(), fenceValueToWaitOn, OS_TIMEOUT_INFINITE);
   assert(wait_res);

   HRESULT hr = pD3D12Dec->m_spDecodeCommandList->Reset(
      pD3D12Dec->m_inflightResourcesPool[d3d12_video_decoder_pool_current_index(pD3D12Dec)].m_spCommandAllocator.Get());
   if (FAILED(hr)) {
      debug_printf("[d3d12_video_decoder] resetting ID3D12GraphicsCommandList failed with HR %x\n", hr);
      assert(false);
   }

   debug_printf("[d3d12_video_decoder] d3d12_video_decoder_begin_frame finalized for fenceValue: %d\n",
                pD3D12Dec->m_fenceValue);
}

/**
 * decode a bitstream
 */
void
d3d12_video_decoder_decode_bitstream(struct pipe_video_codec *codec,
                                     struct pipe_video_buffer *target,
                                     struct pipe_picture_desc *picture,
                                     unsigned num_buffers,
                                     const void *const *buffers,
                                     const unsigned *sizes)
{
   struct d3d12_video_decoder *pD3D12Dec = (struct d3d12_video_decoder *) codec;
   assert(pD3D12Dec);
   debug_printf("[d3d12_video_decoder] d3d12_video_decoder_decode_bitstream started for fenceValue: %d\n",
                pD3D12Dec->m_fenceValue);
   assert(pD3D12Dec->m_spD3D12VideoDevice);
   assert(pD3D12Dec->m_spDecodeCommandQueue);
   assert(pD3D12Dec->m_pD3D12Screen);
   ASSERTED struct d3d12_video_buffer *pD3D12VideoBuffer = (struct d3d12_video_buffer *) target;
   assert(pD3D12VideoBuffer);

   ///
   /// Compressed bitstream buffers
   ///

   /// Mesa VA frontend Video buffer passing semantics for H264, HEVC, MPEG4, VC1 and PIPE_VIDEO_PROFILE_VC1_ADVANCED
   /// are: If num_buffers == 1 -> buf[0] has the compressed bitstream WITH the starting code If num_buffers == 2 ->
   /// buf[0] has the NALU starting code and buf[1] has the compressed bitstream WITHOUT any starting code. If
   /// num_buffers = 3 -> It's JPEG, not supported in D3D12. num_buffers is at most 3.
   /// Mesa VDPAU frontend passes the buffers as they get passed in VdpDecoderRender without fixing any start codes
   /// except for PIPE_VIDEO_PROFILE_VC1_ADVANCED
   // In https://http.download.nvidia.com/XFree86/vdpau/doxygen/html/index.html#video_mixer_usage it's mentioned that:
   // It is recommended that applications pass solely the slice data to VDPAU; specifically that any header data
   // structures be excluded from the portion of the bitstream passed to VDPAU. VDPAU implementations must operate
   // correctly if non-slice data is included, at least for formats employing start codes to delimit slice data. For all
   // codecs/profiles it's highly recommended (when the codec/profile has such codes...) that the start codes are passed
   // to VDPAU, even when not included in the bitstream the VDPAU client is parsing. Let's assume we get all the start
   // codes for VDPAU. The doc also says "VDPAU implementations must operate correctly if non-slice data is included, at
   // least for formats employing start codes to delimit slice data" if we ever get an issue with VDPAU start codes we
   // should consider adding the code that handles this in the VDPAU layer above the gallium driver like mesa VA does.

   // To handle the multi-slice case end_frame already takes care of this by parsing the start codes from the
   // combined bitstream of all decode_bitstream calls.

   // VAAPI seems to send one decode_bitstream command per slice, but we should also support the VDPAU case where the
   // buffers have multiple buffer array entry per slice {startCode (optional), slice1, slice2, ..., startCode
   // (optional) , sliceN}

   if (num_buffers > 2)   // Assume this means multiple slices at once in a decode_bitstream call
   {
      // Based on VA frontend codebase, this never happens for video (no JPEG)
      // Based on VDPAU frontends codebase, this only happens when sending more than one slice at once in decode bitstream

      // To handle the case where VDPAU send all the slices at once in a single decode_bitstream call, let's pretend it
      // was a series of different calls

      // group by start codes and buffers and perform calls for the number of slices
      debug_printf("[d3d12_video_decoder] d3d12_video_decoder_decode_bitstream multiple slices on same call detected "
                   "for fenceValue: %d, breaking down the calls into one per slice\n",
                   pD3D12Dec->m_fenceValue);

      size_t curBufferIdx = 0;

      // Vars to be used for the delegation calls to decode_bitstream
      unsigned call_num_buffers = 0;
      const void *const *call_buffers = nullptr;
      const unsigned *call_sizes = nullptr;

      while (curBufferIdx < num_buffers) {
         // Store the current buffer as the base array pointer for the delegated call, later decide if it'll be a
         // startcode+slicedata or just slicedata call
         call_buffers = &buffers[curBufferIdx];
         call_sizes = &sizes[curBufferIdx];

         // Usually start codes are less or equal than 4 bytes
         // If the current buffer is a start code buffer, send it along with the next buffer. Otherwise, just send the
         // current buffer.
         call_num_buffers = (sizes[curBufferIdx] <= 4) ? 2 : 1;

         // Delegate call with one or two buffers only
         d3d12_video_decoder_decode_bitstream(codec, target, picture, call_num_buffers, call_buffers, call_sizes);

         curBufferIdx += call_num_buffers;   // Consume from the loop the buffers sent in the last call
      }
   } else {
      ///
      /// Handle single slice buffer path, maybe with an extra start code buffer at buffers[0].
      ///

      // Both the start codes being present at buffers[0] and the rest in buffers [1] or full buffer at [0] cases can be
      // handled by flattening all the buffers into a single one and passing that to HW.

      size_t totalReceivedBuffersSize = 0u;   // Combined size of all sizes[]
      for (size_t bufferIdx = 0; bufferIdx < num_buffers; bufferIdx++) {
         totalReceivedBuffersSize += sizes[bufferIdx];
      }

      // Bytes of data pre-staged before this decode_frame call
      auto &inFlightResources = pD3D12Dec->m_inflightResourcesPool[d3d12_video_decoder_pool_current_index(pD3D12Dec)];
      size_t preStagedDataSize = inFlightResources.m_stagingDecodeBitstream.size();

      // Extend the staging buffer size, as decode_frame can be called several times before end_frame
      inFlightResources.m_stagingDecodeBitstream.resize(preStagedDataSize + totalReceivedBuffersSize);

      // Point newSliceDataPositionDstBase to the end of the pre-staged data in m_stagingDecodeBitstream, where the new
      // buffers will be appended
      uint8_t *newSliceDataPositionDstBase = inFlightResources.m_stagingDecodeBitstream.data() + preStagedDataSize;

      // Append new data at the end.
      size_t dstOffset = 0u;
      for (size_t bufferIdx = 0; bufferIdx < num_buffers; bufferIdx++) {
         memcpy(newSliceDataPositionDstBase + dstOffset, buffers[bufferIdx], sizes[bufferIdx]);
         dstOffset += sizes[bufferIdx];
      }

      debug_printf("[d3d12_video_decoder] d3d12_video_decoder_decode_bitstream finalized for fenceValue: %d\n",
                   pD3D12Dec->m_fenceValue);
   }

   if (pD3D12Dec->m_d3d12DecProfileType == d3d12_video_decode_profile_type_h264) {
      struct pipe_h264_picture_desc *h264 = (pipe_h264_picture_desc*) picture;
      target->interlaced = !h264->pps->sps->frame_mbs_only_flag && (h264->field_pic_flag || /* PAFF */ h264->pps->sps->mb_adaptive_frame_field_flag); /* MBAFF */
   }
}

void
d3d12_video_decoder_store_upper_layer_references(struct d3d12_video_decoder *pD3D12Dec,
                                                 struct pipe_video_buffer *target,
                                                 struct pipe_picture_desc *picture)
{
#if D3D12_VIDEO_ANY_DECODER_ENABLED
   pD3D12Dec->m_pCurrentDecodeTarget = target;
   switch (pD3D12Dec->m_d3d12DecProfileType) {
#if VIDEO_CODEC_H264DEC
      case d3d12_video_decode_profile_type_h264:
      {
         pipe_h264_picture_desc *pPicControlH264 = (pipe_h264_picture_desc *) picture;
         pD3D12Dec->m_pCurrentReferenceTargets = pPicControlH264->ref;
      } break;
#endif
#if VIDEO_CODEC_H265DEC
      case d3d12_video_decode_profile_type_hevc:
      {
         pipe_h265_picture_desc *pPicControlHevc = (pipe_h265_picture_desc *) picture;
         pD3D12Dec->m_pCurrentReferenceTargets = pPicControlHevc->ref;
      } break;
#endif
#if VIDEO_CODEC_AV1DEC
      case d3d12_video_decode_profile_type_av1:
      {
         pipe_av1_picture_desc *pPicControlAV1 = (pipe_av1_picture_desc *) picture;
         pD3D12Dec->m_pCurrentReferenceTargets = pPicControlAV1->ref;
      } break;
#endif
#if VIDEO_CODEC_VP9DEC
      case d3d12_video_decode_profile_type_vp9:
      {
         pipe_vp9_picture_desc *pPicControlVP9 = (pipe_vp9_picture_desc *) picture;
         pD3D12Dec->m_pCurrentReferenceTargets = pPicControlVP9->ref;
      } break;
#endif
      default:
      {
         unreachable("Unsupported d3d12_video_decode_profile_type");
      } break;
   }
#endif // D3D12_VIDEO_ANY_DECODER_ENABLED
}

/**
 * end decoding of the current frame
 */
void
d3d12_video_decoder_end_frame(struct pipe_video_codec *codec,
                              struct pipe_video_buffer *target,
                              struct pipe_picture_desc *picture)
{
   struct d3d12_video_decoder *pD3D12Dec = (struct d3d12_video_decoder *) codec;
   assert(pD3D12Dec);
   struct d3d12_screen *pD3D12Screen = (struct d3d12_screen *) pD3D12Dec->m_pD3D12Screen;
   assert(pD3D12Screen);
   debug_printf("[d3d12_video_decoder] d3d12_video_decoder_end_frame started for fenceValue: %d\n",
                pD3D12Dec->m_fenceValue);
   assert(pD3D12Dec->m_spD3D12VideoDevice);
   assert(pD3D12Dec->m_spDecodeCommandQueue);
   struct d3d12_video_buffer *pD3D12VideoBuffer = (struct d3d12_video_buffer *) target;
   assert(pD3D12VideoBuffer);

   ///
   /// Store current decode output target texture and reference textures from upper layer
   ///
   d3d12_video_decoder_store_upper_layer_references(pD3D12Dec, target, picture);

   ///
   /// Codec header picture parameters buffers
   ///

   auto &inFlightResources = pD3D12Dec->m_inflightResourcesPool[d3d12_video_decoder_pool_current_index(pD3D12Dec)];

   d3d12_video_decoder_store_converted_dxva_picparams_from_pipe_input(pD3D12Dec, picture, pD3D12VideoBuffer);
   assert(inFlightResources.m_picParamsBuffer.size() > 0);

   ///
   /// Prepare Slice control buffers before clearing staging buffer
   ///
   assert(inFlightResources.m_stagingDecodeBitstream.size() >
          0);   // Make sure the staging wasn't cleared yet in end_frame
   d3d12_video_decoder_prepare_dxva_slices_control(pD3D12Dec, picture);
   assert(inFlightResources.m_SliceControlBuffer.size() > 0);

   ///
   /// Upload m_stagingDecodeBitstream to GPU memory now that end_frame is called and clear staging buffer
   ///

   uint64_t sliceDataStagingBufferSize = inFlightResources.m_stagingDecodeBitstream.size();
   uint8_t *sliceDataStagingBufferPtr = inFlightResources.m_stagingDecodeBitstream.data();

   // Reallocate if necessary to accomodate the current frame bitstream buffer in GPU memory
   if (inFlightResources.m_curFrameCompressedBitstreamBufferAllocatedSize < sliceDataStagingBufferSize) {
      if (!d3d12_video_decoder_create_staging_bitstream_buffer(pD3D12Screen, pD3D12Dec, sliceDataStagingBufferSize)) {
         debug_printf("[d3d12_video_decoder] d3d12_video_decoder_end_frame - Failure on "
                      "d3d12_video_decoder_create_staging_bitstream_buffer\n");
         debug_printf("[d3d12_video_decoder] d3d12_video_decoder_end_frame failed for fenceValue: %d\n",
                      pD3D12Dec->m_fenceValue);
         assert(false);
         return;
      }
   }

   // Upload frame bitstream CPU data to ID3D12Resource buffer
   inFlightResources.m_curFrameCompressedBitstreamBufferPayloadSize =
      sliceDataStagingBufferSize;   // This can be less than m_curFrameCompressedBitstreamBufferAllocatedSize.
   assert(inFlightResources.m_curFrameCompressedBitstreamBufferPayloadSize <=
          inFlightResources.m_curFrameCompressedBitstreamBufferAllocatedSize);

   /* One-shot transfer operation with data supplied in a user
    * pointer.
    */
   inFlightResources.pPipeCompressedBufferObj =
      d3d12_resource_from_resource(&pD3D12Screen->base, inFlightResources.m_curFrameCompressedBitstreamBuffer.Get());
   assert(inFlightResources.pPipeCompressedBufferObj);
   pD3D12Dec->base.context->buffer_subdata(pD3D12Dec->base.context,                      // context
                                           inFlightResources.pPipeCompressedBufferObj,   // dst buffer
                                           PIPE_MAP_WRITE,                               // usage PIPE_MAP_x
                                           0,                                            // offset
                                           sizeof(*sliceDataStagingBufferPtr) * sliceDataStagingBufferSize,   // size
                                           sliceDataStagingBufferPtr                                          // data
   );

   // Flush buffer_subdata batch
   // before deleting the source CPU buffer below

   pD3D12Dec->base.context->flush(pD3D12Dec->base.context,
                                  &inFlightResources.m_pBitstreamUploadGPUCompletionFence,
                                  PIPE_FLUSH_ASYNC | PIPE_FLUSH_HINT_FINISH);
   assert(inFlightResources.m_pBitstreamUploadGPUCompletionFence);
   // To be waited on GPU fence before flushing current frame DecodeFrame to GPU

   ///
   /// Proceed to record the GPU Decode commands
   ///

   // Requested conversions by caller upper layer (none for now)
   d3d12_video_decode_output_conversion_arguments requestedConversionArguments = {};

   ///
   /// Record DecodeFrame operation and resource state transitions.
   ///

   // Translate input D3D12 structure
   D3D12_VIDEO_DECODE_INPUT_STREAM_ARGUMENTS d3d12InputArguments = {};

   d3d12InputArguments.CompressedBitstream.pBuffer = inFlightResources.m_curFrameCompressedBitstreamBuffer.Get();
   d3d12InputArguments.CompressedBitstream.Offset = 0u;
   ASSERTED constexpr uint64_t d3d12BitstreamOffsetAlignment =
      128u;   // specified in
              // https://docs.microsoft.com/en-us/windows/win32/api/d3d12video/ne-d3d12video-d3d12_video_decode_tier
   assert((d3d12InputArguments.CompressedBitstream.Offset == 0) ||
          ((d3d12InputArguments.CompressedBitstream.Offset % d3d12BitstreamOffsetAlignment) == 0));
   d3d12InputArguments.CompressedBitstream.Size = inFlightResources.m_curFrameCompressedBitstreamBufferPayloadSize;

   D3D12_RESOURCE_BARRIER resourceBarrierCommonToDecode[1] = {
      CD3DX12_RESOURCE_BARRIER::Transition(d3d12InputArguments.CompressedBitstream.pBuffer,
                                           D3D12_RESOURCE_STATE_COMMON,
                                           D3D12_RESOURCE_STATE_VIDEO_DECODE_READ),
   };
   pD3D12Dec->m_spDecodeCommandList->ResourceBarrier(1u, resourceBarrierCommonToDecode);

   // Schedule reverse (back to common) transitions before command list closes for current frame
   pD3D12Dec->m_transitionsBeforeCloseCmdList.push_back(
      CD3DX12_RESOURCE_BARRIER::Transition(d3d12InputArguments.CompressedBitstream.pBuffer,
                                           D3D12_RESOURCE_STATE_VIDEO_DECODE_READ,
                                           D3D12_RESOURCE_STATE_COMMON));

   ///
   /// Clear texture (no reference only flags in resource allocation) to use as decode output to send downstream for
   /// display/consumption
   ///
   ID3D12Resource *pOutputD3D12Texture;
   uint outputD3D12Subresource = 0;

   ///
   /// Ref Only texture (with reference only flags in resource allocation) to use as reconstructed picture decode output
   /// and to store as future reference in DPB
   ///
   ID3D12Resource *pRefOnlyOutputD3D12Texture;
   uint refOnlyOutputD3D12Subresource = 0;

   if (!d3d12_video_decoder_prepare_for_decode_frame(pD3D12Dec,
                                                     target,
                                                     pD3D12VideoBuffer,
                                                     &pOutputD3D12Texture,             // output
                                                     &outputD3D12Subresource,          // output
                                                     &pRefOnlyOutputD3D12Texture,      // output
                                                     &refOnlyOutputD3D12Subresource,   // output
                                                     requestedConversionArguments)) {
      debug_printf("[d3d12_video_decoder] d3d12_video_decoder_end_frame - Failure on "
                   "d3d12_video_decoder_prepare_for_decode_frame\n");
      debug_printf("[d3d12_video_decoder] d3d12_video_decoder_end_frame failed for fenceValue: %d\n",
                   pD3D12Dec->m_fenceValue);
      assert(false);
      return;
   }

   ///
   /// Set codec picture parameters CPU buffer
   ///

   d3d12InputArguments.NumFrameArguments =
      1u;   // Only the codec data received from the above layer with picture params
   d3d12InputArguments.FrameArguments[d3d12InputArguments.NumFrameArguments - 1] = {
      D3D12_VIDEO_DECODE_ARGUMENT_TYPE_PICTURE_PARAMETERS,
      static_cast<uint32_t>(inFlightResources.m_picParamsBuffer.size()),
      inFlightResources.m_picParamsBuffer.data(),
   };

   if (inFlightResources.m_SliceControlBuffer.size() > 0) {
      d3d12InputArguments.NumFrameArguments++;
      d3d12InputArguments.FrameArguments[d3d12InputArguments.NumFrameArguments - 1] = {
         D3D12_VIDEO_DECODE_ARGUMENT_TYPE_SLICE_CONTROL,
         static_cast<uint32_t>(inFlightResources.m_SliceControlBuffer.size()),
         inFlightResources.m_SliceControlBuffer.data(),
      };
   }

   if (inFlightResources.qp_matrix_frame_argument_enabled &&
       (inFlightResources.m_InverseQuantMatrixBuffer.size() > 0)) {
      d3d12InputArguments.NumFrameArguments++;
      d3d12InputArguments.FrameArguments[d3d12InputArguments.NumFrameArguments - 1] = {
         D3D12_VIDEO_DECODE_ARGUMENT_TYPE_INVERSE_QUANTIZATION_MATRIX,
         static_cast<uint32_t>(inFlightResources.m_InverseQuantMatrixBuffer.size()),
         inFlightResources.m_InverseQuantMatrixBuffer.data(),
      };
   }

   d3d12InputArguments.ReferenceFrames = pD3D12Dec->m_spDPBManager->get_current_reference_frames();
   if (D3D12_DEBUG_VERBOSE & d3d12_debug) {
      pD3D12Dec->m_spDPBManager->print_dpb();
   }

   d3d12InputArguments.pHeap = pD3D12Dec->m_spVideoDecoderHeap.Get();

   // translate output D3D12 structure
   D3D12_VIDEO_DECODE_OUTPUT_STREAM_ARGUMENTS1 d3d12OutputArguments = {};
   d3d12OutputArguments.pOutputTexture2D = pOutputD3D12Texture;
   d3d12OutputArguments.OutputSubresource = outputD3D12Subresource;

   bool fReferenceOnly = (pD3D12Dec->m_ConfigDecoderSpecificFlags &
                          d3d12_video_decode_config_specific_flag_reference_only_textures_required) != 0;
   if (fReferenceOnly) {
      d3d12OutputArguments.ConversionArguments.Enable = true;

      assert(pRefOnlyOutputD3D12Texture);
      d3d12OutputArguments.ConversionArguments.pReferenceTexture2D = pRefOnlyOutputD3D12Texture;
      d3d12OutputArguments.ConversionArguments.ReferenceSubresource = refOnlyOutputD3D12Subresource;

      const D3D12_RESOURCE_DESC &descReference = GetDesc(d3d12OutputArguments.ConversionArguments.pReferenceTexture2D);
      d3d12OutputArguments.ConversionArguments.DecodeColorSpace = d3d12_convert_from_legacy_color_space(
         !util_format_is_yuv(d3d12_get_pipe_format(descReference.Format)),
         util_format_get_blocksize(d3d12_get_pipe_format(descReference.Format)) * 8 /*bytes to bits conversion*/,
         /* StudioRGB= */ false,
         /* P709= */ true,
         /* StudioYUV= */ true);

      const D3D12_RESOURCE_DESC &descOutput = GetDesc(d3d12OutputArguments.pOutputTexture2D);
      d3d12OutputArguments.ConversionArguments.OutputColorSpace = d3d12_convert_from_legacy_color_space(
         !util_format_is_yuv(d3d12_get_pipe_format(descOutput.Format)),
         util_format_get_blocksize(d3d12_get_pipe_format(descOutput.Format)) * 8 /*bytes to bits conversion*/,
         /* StudioRGB= */ false,
         /* P709= */ true,
         /* StudioYUV= */ true);

      const D3D12_VIDEO_DECODER_HEAP_DESC &HeapDesc = GetDesc(pD3D12Dec->m_spVideoDecoderHeap.Get());
      d3d12OutputArguments.ConversionArguments.OutputWidth = HeapDesc.DecodeWidth;
      d3d12OutputArguments.ConversionArguments.OutputHeight = HeapDesc.DecodeHeight;
   } else {
      d3d12OutputArguments.ConversionArguments.Enable = false;
   }

   CD3DX12_RESOURCE_DESC outputDesc(GetDesc(d3d12OutputArguments.pOutputTexture2D));
   uint32_t MipLevel, PlaneSlice, ArraySlice;
   D3D12DecomposeSubresource(d3d12OutputArguments.OutputSubresource,
                             outputDesc.MipLevels,
                             outputDesc.ArraySize(),
                             MipLevel,
                             ArraySlice,
                             PlaneSlice);

   for (PlaneSlice = 0; PlaneSlice < pD3D12Dec->m_decodeFormatInfo.PlaneCount; PlaneSlice++) {
      uint planeOutputSubresource = outputDesc.CalcSubresource(MipLevel, ArraySlice, PlaneSlice);

      D3D12_RESOURCE_BARRIER resourceBarrierCommonToDecode[1] = {
         CD3DX12_RESOURCE_BARRIER::Transition(d3d12OutputArguments.pOutputTexture2D,
                                              D3D12_RESOURCE_STATE_COMMON,
                                              D3D12_RESOURCE_STATE_VIDEO_DECODE_WRITE,
                                              planeOutputSubresource),
      };
      pD3D12Dec->m_spDecodeCommandList->ResourceBarrier(1u, resourceBarrierCommonToDecode);
   }

   // Schedule reverse (back to common) transitions before command list closes for current frame
   for (PlaneSlice = 0; PlaneSlice < pD3D12Dec->m_decodeFormatInfo.PlaneCount; PlaneSlice++) {
      uint planeOutputSubresource = outputDesc.CalcSubresource(MipLevel, ArraySlice, PlaneSlice);
      pD3D12Dec->m_transitionsBeforeCloseCmdList.push_back(
         CD3DX12_RESOURCE_BARRIER::Transition(d3d12OutputArguments.pOutputTexture2D,
                                              D3D12_RESOURCE_STATE_VIDEO_DECODE_WRITE,
                                              D3D12_RESOURCE_STATE_COMMON,
                                              planeOutputSubresource));
   }

   // Record DecodeFrame

   pD3D12Dec->m_spDecodeCommandList->DecodeFrame1(pD3D12Dec->m_spVideoDecoder.Get(),
                                                  &d3d12OutputArguments,
                                                  &d3d12InputArguments);

   debug_printf("[d3d12_video_decoder] d3d12_video_decoder_end_frame finalized for fenceValue: %d\n",
                pD3D12Dec->m_fenceValue);

   // Save extra references of Decoder, DecoderHeap and DPB allocations in case
   // there's a reconfiguration that trigers the construction of new objects
   inFlightResources.m_spDecoder = pD3D12Dec->m_spVideoDecoder;
   inFlightResources.m_spDecoderHeap = pD3D12Dec->m_spVideoDecoderHeap;
   inFlightResources.m_References = pD3D12Dec->m_spDPBManager;

   ///
   /// Flush work to the GPU
   ///
   pD3D12Dec->m_needsGPUFlush = true;
   d3d12_video_decoder_flush(codec);
   // Call to d3d12_video_decoder_flush increases m_FenceValue
   uint64_t inflightIndexBeforeFlush = (pD3D12Dec->m_fenceValue - 1u) % D3D12_VIDEO_DEC_ASYNC_DEPTH;

   if (pD3D12Dec->m_spDPBManager->is_pipe_buffer_underlying_output_decode_allocation()) {
      // No need to copy, the output surface fence is merely the decode queue fence
      *picture->fence = (pipe_fence_handle *) &pD3D12Dec->m_inflightResourcesPool[inflightIndexBeforeFlush].m_FenceData;
   } else {
      ///
      /// If !pD3D12Dec->m_spDPBManager->is_pipe_buffer_underlying_output_decode_allocation()
      /// We cannot use the standalone video buffer allocation directly and we must use instead
      /// either a ID3D12Resource with DECODE_REFERENCE only flag or a texture array within the same
      /// allocation
      /// Do GPU->GPU texture copy from decode output to pipe target decode texture sampler view planes
      ///

      // Get destination resource
      struct pipe_sampler_view **pPipeDstViews = target->get_sampler_view_planes(target);

      // Get source pipe_resource
      pipe_resource *pPipeSrc =
         d3d12_resource_from_resource(&pD3D12Screen->base, d3d12OutputArguments.pOutputTexture2D);
      assert(pPipeSrc);

      // GPU wait on the graphics context which will do the copy until the decode finishes
      pD3D12Screen->cmdqueue->Wait(
         pD3D12Dec->m_inflightResourcesPool[inflightIndexBeforeFlush].m_FenceData.cmdqueue_fence,
         pD3D12Dec->m_inflightResourcesPool[inflightIndexBeforeFlush].m_FenceData.value);

      // Copy all format subresources/texture planes
      for (PlaneSlice = 0; PlaneSlice < pD3D12Dec->m_decodeFormatInfo.PlaneCount; PlaneSlice++) {
         assert(d3d12OutputArguments.OutputSubresource < INT16_MAX);
         struct pipe_box box = { 0,
                                 0,
                                 // src array slice, taken as Z for TEXTURE_2D_ARRAY
                                 static_cast<int16_t>(d3d12OutputArguments.OutputSubresource),
                                 static_cast<int>(pPipeDstViews[PlaneSlice]->texture->width0),
                                 static_cast<int16_t>(pPipeDstViews[PlaneSlice]->texture->height0),
                                 1 };

         pD3D12Dec->base.context->resource_copy_region(pD3D12Dec->base.context,
                                                       pPipeDstViews[PlaneSlice]->texture,              // dst
                                                       0,                                               // dst level
                                                       0,                                               // dstX
                                                       0,                                               // dstY
                                                       0,                                               // dstZ
                                                       (PlaneSlice == 0) ? pPipeSrc : pPipeSrc->next,   // src
                                                       0,                                               // src level
                                                       &box);
      }
      // Flush resource_copy_region batch
      // The output surface fence is the graphics queue that will signal after the copy ends
      pD3D12Dec->base.context->flush(pD3D12Dec->base.context, picture->fence, PIPE_FLUSH_ASYNC | PIPE_FLUSH_HINT_FINISH);
   }
}

/**
 * Get decoder fence.
 */
int
d3d12_video_decoder_get_decoder_fence(struct pipe_video_codec *codec, struct pipe_fence_handle *fence, uint64_t timeout)
{
   struct d3d12_fence *fenceValueToWaitOn = (struct d3d12_fence *) fence;
   assert(fenceValueToWaitOn);

   ASSERTED bool wait_res =
      d3d12_video_decoder_sync_completion(codec, fenceValueToWaitOn->cmdqueue_fence, fenceValueToWaitOn->value, timeout);

   // Return semantics based on p_video_codec interface
   // ret == 0 -> Decode in progress
   // ret != 0 -> Decode completed
   return wait_res ? 1 : 0;
}

/**
 * flush any outstanding command buffers to the hardware
 * should be called before a video_buffer is acessed by the gallium frontend again
 */
void
d3d12_video_decoder_flush(struct pipe_video_codec *codec)
{
   struct d3d12_video_decoder *pD3D12Dec = (struct d3d12_video_decoder *) codec;
   assert(pD3D12Dec);
   assert(pD3D12Dec->m_spD3D12VideoDevice);
   assert(pD3D12Dec->m_spDecodeCommandQueue);
   debug_printf("[d3d12_video_decoder] d3d12_video_decoder_flush started. Will flush video queue work and CPU wait on "
                "fenceValue: %d\n",
                pD3D12Dec->m_fenceValue);

   if (!pD3D12Dec->m_needsGPUFlush) {
      debug_printf("[d3d12_video_decoder] d3d12_video_decoder_flush started. Nothing to flush, all up to date.\n");
   } else {
      HRESULT hr = pD3D12Dec->m_pD3D12Screen->dev->GetDeviceRemovedReason();
      if (hr != S_OK) {
         debug_printf("[d3d12_video_decoder] d3d12_video_decoder_flush"
                      " - D3D12Device was removed BEFORE commandlist "
                      "execution with HR %x.\n",
                      hr);
         goto flush_fail;
      }

      if (pD3D12Dec->m_transitionsBeforeCloseCmdList.size() > 0) {
         pD3D12Dec->m_spDecodeCommandList->ResourceBarrier(pD3D12Dec->m_transitionsBeforeCloseCmdList.size(),
                                                           pD3D12Dec->m_transitionsBeforeCloseCmdList.data());
         pD3D12Dec->m_transitionsBeforeCloseCmdList.clear();
      }

      hr = pD3D12Dec->m_spDecodeCommandList->Close();
      if (FAILED(hr)) {
         debug_printf("[d3d12_video_decoder] d3d12_video_decoder_flush - Can't close command list with HR %x\n", hr);
         goto flush_fail;
      }

      auto &inFlightResources = pD3D12Dec->m_inflightResourcesPool[d3d12_video_decoder_pool_current_index(pD3D12Dec)];
      ID3D12CommandList *ppCommandLists[1] = { pD3D12Dec->m_spDecodeCommandList.Get() };
      struct d3d12_fence *pUploadBitstreamFence = d3d12_fence(inFlightResources.m_pBitstreamUploadGPUCompletionFence);
      pD3D12Dec->m_spDecodeCommandQueue->Wait(pUploadBitstreamFence->cmdqueue_fence, pUploadBitstreamFence->value);
      pD3D12Dec->m_spDecodeCommandQueue->ExecuteCommandLists(1, ppCommandLists);
      pD3D12Dec->m_spDecodeCommandQueue->Signal(pD3D12Dec->m_spFence.Get(), pD3D12Dec->m_fenceValue);

      // Validate device was not removed
      hr = pD3D12Dec->m_pD3D12Screen->dev->GetDeviceRemovedReason();
      if (hr != S_OK) {
         debug_printf("[d3d12_video_decoder] d3d12_video_decoder_flush"
                      " - D3D12Device was removed AFTER commandlist "
                      "execution with HR %x, but wasn't before.\n",
                      hr);
         goto flush_fail;
      }

      // Set async fence info
      memset(&inFlightResources.m_FenceData, 0, sizeof(inFlightResources.m_FenceData));

      inFlightResources.m_FenceData.value = pD3D12Dec->m_fenceValue;
      inFlightResources.m_FenceData.cmdqueue_fence = pD3D12Dec->m_spFence.Get();

      pD3D12Dec->m_fenceValue++;
      pD3D12Dec->m_needsGPUFlush = false;
   }
   return;

flush_fail:
   debug_printf("[d3d12_video_decoder] d3d12_video_decoder_flush failed for fenceValue: %d\n", pD3D12Dec->m_fenceValue);
   assert(false);
}

bool
d3d12_video_decoder_create_command_objects(const struct d3d12_screen *pD3D12Screen,
                                           struct d3d12_video_decoder *pD3D12Dec)
{
   assert(pD3D12Dec->m_spD3D12VideoDevice);

   D3D12_COMMAND_QUEUE_DESC commandQueueDesc = { D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE };
   HRESULT hr = pD3D12Screen->dev->CreateCommandQueue(&commandQueueDesc,
                                                      IID_PPV_ARGS(pD3D12Dec->m_spDecodeCommandQueue.GetAddressOf()));
   if (FAILED(hr)) {
      debug_printf("[d3d12_video_decoder] d3d12_video_decoder_create_command_objects - Call to CreateCommandQueue "
                   "failed with HR %x\n",
                   hr);
      return false;
   }

   hr = pD3D12Screen->dev->CreateFence(0, D3D12_FENCE_FLAG_SHARED, IID_PPV_ARGS(&pD3D12Dec->m_spFence));
   if (FAILED(hr)) {
      debug_printf(
         "[d3d12_video_decoder] d3d12_video_decoder_create_command_objects - Call to CreateFence failed with HR %x\n",
         hr);
      return false;
   }

   for (auto &inputResource : pD3D12Dec->m_inflightResourcesPool) {
      hr = pD3D12Dec->m_pD3D12Screen->dev->CreateCommandAllocator(
         D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE,
         IID_PPV_ARGS(inputResource.m_spCommandAllocator.GetAddressOf()));
      if (FAILED(hr)) {
         debug_printf("[d3d12_video_decoder] d3d12_video_decoder_create_command_objects - Call to "
                      "CreateCommandAllocator failed with HR %x\n",
                      hr);
         return false;
      }
   }

   ComPtr<ID3D12Device4> spD3D12Device4;
   if (FAILED(pD3D12Dec->m_pD3D12Screen->dev->QueryInterface(IID_PPV_ARGS(spD3D12Device4.GetAddressOf())))) {
      debug_printf(
         "[d3d12_video_decoder] d3d12_video_decoder_create_decoder - D3D12 Device has no ID3D12Device4 support\n");
      return false;
   }

   hr = spD3D12Device4->CreateCommandList1(0,
                                           D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE,
                                           D3D12_COMMAND_LIST_FLAG_NONE,
                                           IID_PPV_ARGS(pD3D12Dec->m_spDecodeCommandList.GetAddressOf()));

   if (FAILED(hr)) {
      debug_printf("[d3d12_video_decoder] d3d12_video_decoder_create_command_objects - Call to CreateCommandList "
                   "failed with HR %x\n",
                   hr);
      return false;
   }

   return true;
}

bool
d3d12_video_decoder_check_caps_and_create_decoder(const struct d3d12_screen *pD3D12Screen,
                                                  struct d3d12_video_decoder *pD3D12Dec)
{
   assert(pD3D12Dec->m_spD3D12VideoDevice);

   pD3D12Dec->m_decoderDesc = {};

   D3D12_VIDEO_DECODE_CONFIGURATION decodeConfiguration = { pD3D12Dec->m_d3d12DecProfile,
                                                            D3D12_BITSTREAM_ENCRYPTION_TYPE_NONE,
                                                            D3D12_VIDEO_FRAME_CODED_INTERLACE_TYPE_NONE };

   D3D12_FEATURE_DATA_VIDEO_DECODE_SUPPORT decodeSupport = {};
   decodeSupport.NodeIndex = pD3D12Dec->m_NodeIndex;
   decodeSupport.Configuration = decodeConfiguration;
   decodeSupport.Width = pD3D12Dec->base.width;
   decodeSupport.Height = pD3D12Dec->base.height;
   decodeSupport.DecodeFormat = pD3D12Dec->m_decodeFormat;
   // no info from above layer on framerate/bitrate
   decodeSupport.FrameRate.Numerator = 0;
   decodeSupport.FrameRate.Denominator = 0;
   decodeSupport.BitRate = 0;

   HRESULT hr = pD3D12Dec->m_spD3D12VideoDevice->CheckFeatureSupport(D3D12_FEATURE_VIDEO_DECODE_SUPPORT,
                                                                     &decodeSupport,
                                                                     sizeof(decodeSupport));
   if (FAILED(hr)) {
      debug_printf("[d3d12_video_decoder] d3d12_video_decoder_check_caps_and_create_decoder - CheckFeatureSupport "
                   "failed with HR %x\n",
                   hr);
      return false;
   }

   if (!(decodeSupport.SupportFlags & D3D12_VIDEO_DECODE_SUPPORT_FLAG_SUPPORTED)) {
      debug_printf("[d3d12_video_decoder] d3d12_video_decoder_check_caps_and_create_decoder - "
                   "D3D12_VIDEO_DECODE_SUPPORT_FLAG_SUPPORTED was false when checking caps \n");
      return false;
   }

   pD3D12Dec->m_configurationFlags = decodeSupport.ConfigurationFlags;
   pD3D12Dec->m_tier = decodeSupport.DecodeTier;

   if (d3d12_video_decoder_supports_aot_dpb(decodeSupport, pD3D12Dec->m_d3d12DecProfileType)) {
      pD3D12Dec->m_ConfigDecoderSpecificFlags |= d3d12_video_decode_config_specific_flag_array_of_textures;
   }

   if (decodeSupport.ConfigurationFlags & D3D12_VIDEO_DECODE_CONFIGURATION_FLAG_HEIGHT_ALIGNMENT_MULTIPLE_32_REQUIRED) {
      pD3D12Dec->m_ConfigDecoderSpecificFlags |= d3d12_video_decode_config_specific_flag_alignment_height;
   }

   if (decodeSupport.ConfigurationFlags & D3D12_VIDEO_DECODE_CONFIGURATION_FLAG_REFERENCE_ONLY_ALLOCATIONS_REQUIRED) {
      pD3D12Dec->m_ConfigDecoderSpecificFlags |=
         d3d12_video_decode_config_specific_flag_reference_only_textures_required;
   }

   pD3D12Dec->m_decoderDesc.NodeMask = pD3D12Dec->m_NodeMask;
   pD3D12Dec->m_decoderDesc.Configuration = decodeConfiguration;

   hr = pD3D12Dec->m_spD3D12VideoDevice->CreateVideoDecoder(&pD3D12Dec->m_decoderDesc,
                                                            IID_PPV_ARGS(pD3D12Dec->m_spVideoDecoder.GetAddressOf()));
   if (FAILED(hr)) {
      debug_printf("[d3d12_video_decoder] d3d12_video_decoder_check_caps_and_create_decoder - CreateVideoDecoder "
                   "failed with HR %x\n",
                   hr);
      return false;
   }

   return true;
}

bool
d3d12_video_decoder_create_video_state_buffers(const struct d3d12_screen *pD3D12Screen,
                                               struct d3d12_video_decoder *pD3D12Dec)
{
   assert(pD3D12Dec->m_spD3D12VideoDevice);
   if (!d3d12_video_decoder_create_staging_bitstream_buffer(pD3D12Screen,
                                                            pD3D12Dec,
                                                            pD3D12Dec->m_InitialCompBitstreamGPUBufferSize)) {
      debug_printf("[d3d12_video_decoder] d3d12_video_decoder_create_video_state_buffers - Failure on "
                   "d3d12_video_decoder_create_staging_bitstream_buffer\n");
      return false;
   }

   return true;
}

bool
d3d12_video_decoder_create_staging_bitstream_buffer(const struct d3d12_screen *pD3D12Screen,
                                                    struct d3d12_video_decoder *pD3D12Dec,
                                                    uint64_t bufSize)
{
   assert(pD3D12Dec->m_spD3D12VideoDevice);
   auto &inFlightResources = pD3D12Dec->m_inflightResourcesPool[d3d12_video_decoder_pool_current_index(pD3D12Dec)];
   if (inFlightResources.m_curFrameCompressedBitstreamBuffer.Get() != nullptr) {
      inFlightResources.m_curFrameCompressedBitstreamBuffer.Reset();
   }

   auto descHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT, pD3D12Dec->m_NodeMask, pD3D12Dec->m_NodeMask);
   auto descResource = CD3DX12_RESOURCE_DESC::Buffer(bufSize);
   HRESULT hr = pD3D12Screen->dev->CreateCommittedResource(
      &descHeap,
      D3D12_HEAP_FLAG_NONE,
      &descResource,
      D3D12_RESOURCE_STATE_COMMON,
      nullptr,
      IID_PPV_ARGS(inFlightResources.m_curFrameCompressedBitstreamBuffer.GetAddressOf()));
   if (FAILED(hr)) {
      debug_printf("[d3d12_video_decoder] d3d12_video_decoder_create_staging_bitstream_buffer - "
                   "CreateCommittedResource failed with HR %x\n",
                   hr);
      return false;
   }

   inFlightResources.m_curFrameCompressedBitstreamBufferAllocatedSize = bufSize;
   return true;
}

bool
d3d12_video_decoder_prepare_for_decode_frame(struct d3d12_video_decoder *pD3D12Dec,
                                             struct pipe_video_buffer *pCurrentDecodeTarget,
                                             struct d3d12_video_buffer *pD3D12VideoBuffer,
                                             ID3D12Resource **ppOutTexture2D,
                                             uint32_t *pOutSubresourceIndex,
                                             ID3D12Resource **ppRefOnlyOutTexture2D,
                                             uint32_t *pRefOnlyOutSubresourceIndex,
                                             const d3d12_video_decode_output_conversion_arguments &conversionArgs)
{
   if (!d3d12_video_decoder_reconfigure_dpb(pD3D12Dec, pD3D12VideoBuffer, conversionArgs)) {
      debug_printf("d3d12_video_decoder_reconfigure_dpb failed!\n");
      return false;
   }

   // Refresh DPB active references for current frame, release memory for unused references.
   d3d12_video_decoder_refresh_dpb_active_references(pD3D12Dec);

   // Get the output texture for the current frame to be decoded
   pD3D12Dec->m_spDPBManager->get_current_frame_decode_output_texture(pCurrentDecodeTarget,
                                                                      ppOutTexture2D,
                                                                      pOutSubresourceIndex);

   auto vidBuffer = (struct d3d12_video_buffer *) (pCurrentDecodeTarget);
   // If is_pipe_buffer_underlying_output_decode_allocation is enabled,
   // we can just use the underlying allocation in pCurrentDecodeTarget
   // and avoid an extra copy after decoding the frame.
   // If this is the case, we need to handle the residency of this resource
   // (if not we're actually creating the resources with CreateCommitedResource with
   // residency by default)
   if (pD3D12Dec->m_spDPBManager->is_pipe_buffer_underlying_output_decode_allocation()) {
      assert(d3d12_resource_resource(vidBuffer->texture) == *ppOutTexture2D);
      // Make it permanently resident for video use
      d3d12_promote_to_permanent_residency(pD3D12Dec->m_pD3D12Screen, vidBuffer->texture);
   }

   // Get the reference only texture for the current frame to be decoded (if applicable)
   bool fReferenceOnly = (pD3D12Dec->m_ConfigDecoderSpecificFlags &
                          d3d12_video_decode_config_specific_flag_reference_only_textures_required) != 0;
   if (fReferenceOnly) {
      bool needsTransitionToDecodeWrite = false;
      pD3D12Dec->m_spDPBManager->get_reference_only_output(pCurrentDecodeTarget,
                                                           ppRefOnlyOutTexture2D,
                                                           pRefOnlyOutSubresourceIndex,
                                                           needsTransitionToDecodeWrite);
      assert(needsTransitionToDecodeWrite);

      CD3DX12_RESOURCE_DESC outputDesc(GetDesc(*ppRefOnlyOutTexture2D));
      uint32_t MipLevel, PlaneSlice, ArraySlice;
      D3D12DecomposeSubresource(*pRefOnlyOutSubresourceIndex,
                                outputDesc.MipLevels,
                                outputDesc.ArraySize(),
                                MipLevel,
                                ArraySlice,
                                PlaneSlice);

      for (PlaneSlice = 0; PlaneSlice < pD3D12Dec->m_decodeFormatInfo.PlaneCount; PlaneSlice++) {
         uint planeOutputSubresource = outputDesc.CalcSubresource(MipLevel, ArraySlice, PlaneSlice);

         D3D12_RESOURCE_BARRIER resourceBarrierCommonToDecode[1] = {
            CD3DX12_RESOURCE_BARRIER::Transition(*ppRefOnlyOutTexture2D,
                                                 D3D12_RESOURCE_STATE_COMMON,
                                                 D3D12_RESOURCE_STATE_VIDEO_DECODE_WRITE,
                                                 planeOutputSubresource),
         };
         pD3D12Dec->m_spDecodeCommandList->ResourceBarrier(1u, resourceBarrierCommonToDecode);
      }

      // Schedule reverse (back to common) transitions before command list closes for current frame
      for (PlaneSlice = 0; PlaneSlice < pD3D12Dec->m_decodeFormatInfo.PlaneCount; PlaneSlice++) {
         uint planeOutputSubresource = outputDesc.CalcSubresource(MipLevel, ArraySlice, PlaneSlice);
         pD3D12Dec->m_transitionsBeforeCloseCmdList.push_back(
            CD3DX12_RESOURCE_BARRIER::Transition(*ppRefOnlyOutTexture2D,
                                                 D3D12_RESOURCE_STATE_VIDEO_DECODE_WRITE,
                                                 D3D12_RESOURCE_STATE_COMMON,
                                                 planeOutputSubresource));
      }
   }

   // If decoded needs reference_only entries in the dpb, use the reference_only allocation for current frame
   // otherwise, use the standard output resource
   [[maybe_unused]] ID3D12Resource *pCurrentFrameDPBEntry = fReferenceOnly ? *ppRefOnlyOutTexture2D : *ppOutTexture2D;
   [[maybe_unused]] uint32_t currentFrameDPBEntrySubresource = fReferenceOnly ? *pRefOnlyOutSubresourceIndex : *pOutSubresourceIndex;
#if D3D12_VIDEO_ANY_DECODER_ENABLED
   switch (pD3D12Dec->m_d3d12DecProfileType) {
#if VIDEO_CODEC_H264DEC
      case d3d12_video_decode_profile_type_h264:
      {
         d3d12_video_decoder_prepare_current_frame_references_h264(pD3D12Dec,
                                                                   pCurrentFrameDPBEntry,
                                                                   currentFrameDPBEntrySubresource);
      } break;
#endif
#if VIDEO_CODEC_H265DEC
      case d3d12_video_decode_profile_type_hevc:
      {
         d3d12_video_decoder_prepare_current_frame_references_hevc(pD3D12Dec,
                                                                   pCurrentFrameDPBEntry,
                                                                   currentFrameDPBEntrySubresource);
      } break;
#endif
#if VIDEO_CODEC_AV1DEC
      case d3d12_video_decode_profile_type_av1:
      {
         d3d12_video_decoder_prepare_current_frame_references_av1(pD3D12Dec,
                                                                  pCurrentFrameDPBEntry,
                                                                  currentFrameDPBEntrySubresource);
      } break;
#endif
#if VIDEO_CODEC_VP9DEC
      case d3d12_video_decode_profile_type_vp9:
      {
         d3d12_video_decoder_prepare_current_frame_references_vp9(pD3D12Dec,
                                                                  pCurrentFrameDPBEntry,
                                                                  currentFrameDPBEntrySubresource);
      } break;
#endif
      default:
      {
         unreachable("Unsupported d3d12_video_decode_profile_type");
      } break;
   }
#endif // D3D12_VIDEO_ANY_DECODER_ENABLED
   return true;
}

bool
d3d12_video_decoder_reconfigure_dpb(struct d3d12_video_decoder *pD3D12Dec,
                                    struct d3d12_video_buffer *pD3D12VideoBuffer,
                                    const d3d12_video_decode_output_conversion_arguments &conversionArguments)
{
   uint32_t width;
   uint32_t height;
   uint16_t maxDPB;
   bool isInterlaced;
   d3d12_video_decoder_get_frame_info(pD3D12Dec, &width, &height, &maxDPB, isInterlaced);

   ID3D12Resource *pPipeD3D12DstResource = d3d12_resource_resource(pD3D12VideoBuffer->texture);
   D3D12_RESOURCE_DESC outputResourceDesc = GetDesc(pPipeD3D12DstResource);

   assert(pD3D12VideoBuffer->base.interlaced == isInterlaced);
   D3D12_VIDEO_FRAME_CODED_INTERLACE_TYPE interlaceTypeRequested =
      isInterlaced ? D3D12_VIDEO_FRAME_CODED_INTERLACE_TYPE_FIELD_BASED : D3D12_VIDEO_FRAME_CODED_INTERLACE_TYPE_NONE;
   if ((pD3D12Dec->m_decodeFormat != outputResourceDesc.Format) ||
       (pD3D12Dec->m_decoderDesc.Configuration.InterlaceType != interlaceTypeRequested)) {
      // Copy current pD3D12Dec->m_decoderDesc, modify decodeprofile and re-create decoder.
      D3D12_VIDEO_DECODER_DESC decoderDesc = pD3D12Dec->m_decoderDesc;
      decoderDesc.Configuration.InterlaceType = interlaceTypeRequested;
      decoderDesc.Configuration.DecodeProfile =
         d3d12_video_decoder_resolve_profile(pD3D12Dec->m_d3d12DecProfileType, pD3D12Dec->m_decodeFormat);
      pD3D12Dec->m_spVideoDecoder.Reset();
      HRESULT hr =
         pD3D12Dec->m_spD3D12VideoDevice->CreateVideoDecoder(&decoderDesc,
                                                             IID_PPV_ARGS(pD3D12Dec->m_spVideoDecoder.GetAddressOf()));
      if (FAILED(hr)) {
         debug_printf(
            "[d3d12_video_decoder] d3d12_video_decoder_reconfigure_dpb - CreateVideoDecoder failed with HR %x\n",
            hr);
         return false;
      }
      // Update state after CreateVideoDecoder succeeds only.
      pD3D12Dec->m_decoderDesc = decoderDesc;
   }

   if (!pD3D12Dec->m_spDPBManager || !pD3D12Dec->m_spVideoDecoderHeap ||
       pD3D12Dec->m_decodeFormat != outputResourceDesc.Format || pD3D12Dec->m_decoderHeapDesc.DecodeWidth != width ||
       pD3D12Dec->m_decoderHeapDesc.DecodeHeight != height ||
       pD3D12Dec->m_decoderHeapDesc.MaxDecodePictureBufferCount < maxDPB) {
      // Detect the combination of AOT/ReferenceOnly to configure the DPB manager
      uint16_t referenceCount = (conversionArguments.Enable) ? (uint16_t) conversionArguments.ReferenceFrameCount +
                                                                  1 /*extra slot for current picture*/ :
                                                               maxDPB;
      d3d12_video_decode_dpb_descriptor dpbDesc = {};
      dpbDesc.Width = (conversionArguments.Enable) ? conversionArguments.ReferenceInfo.Width : width;
      dpbDesc.Height = (conversionArguments.Enable) ? conversionArguments.ReferenceInfo.Height : height;
      dpbDesc.Format =
         (conversionArguments.Enable) ? conversionArguments.ReferenceInfo.Format.Format : outputResourceDesc.Format;
      dpbDesc.fArrayOfTexture =
         ((pD3D12Dec->m_ConfigDecoderSpecificFlags & d3d12_video_decode_config_specific_flag_array_of_textures) != 0);
      dpbDesc.dpbSize = referenceCount;
      dpbDesc.m_NodeMask = pD3D12Dec->m_NodeMask;
      dpbDesc.fReferenceOnly = ((pD3D12Dec->m_ConfigDecoderSpecificFlags &
                                 d3d12_video_decode_config_specific_flag_reference_only_textures_required) != 0);

      // Create DPB manager
      if (pD3D12Dec->m_spDPBManager == nullptr) {
         pD3D12Dec->m_spDPBManager.reset(new d3d12_video_decoder_references_manager(pD3D12Dec->m_pD3D12Screen,
                                                                                    pD3D12Dec->m_NodeMask,
                                                                                    pD3D12Dec->m_d3d12DecProfileType,
                                                                                    dpbDesc));
      }

      //
      // (Re)-create decoder heap
      //
      D3D12_VIDEO_DECODER_HEAP_DESC decoderHeapDesc = {};
      decoderHeapDesc.NodeMask = pD3D12Dec->m_NodeMask;
      decoderHeapDesc.Configuration = pD3D12Dec->m_decoderDesc.Configuration;
      decoderHeapDesc.DecodeWidth = dpbDesc.Width;
      decoderHeapDesc.DecodeHeight = dpbDesc.Height;
      decoderHeapDesc.Format = dpbDesc.Format;
      decoderHeapDesc.MaxDecodePictureBufferCount = maxDPB;
      pD3D12Dec->m_spVideoDecoderHeap.Reset();
      HRESULT hr = pD3D12Dec->m_spD3D12VideoDevice->CreateVideoDecoderHeap(
         &decoderHeapDesc,
         IID_PPV_ARGS(pD3D12Dec->m_spVideoDecoderHeap.GetAddressOf()));
      if (FAILED(hr)) {
         debug_printf(
            "[d3d12_video_decoder] d3d12_video_decoder_reconfigure_dpb - CreateVideoDecoderHeap failed with HR %x\n",
            hr);
         return false;
      }
      // Update pD3D12Dec after CreateVideoDecoderHeap succeeds only.
      pD3D12Dec->m_decoderHeapDesc = decoderHeapDesc;
   }

   pD3D12Dec->m_decodeFormat = outputResourceDesc.Format;

   return true;
}

void
d3d12_video_decoder_refresh_dpb_active_references(struct d3d12_video_decoder *pD3D12Dec)
{
#if D3D12_VIDEO_ANY_DECODER_ENABLED
   switch (pD3D12Dec->m_d3d12DecProfileType) {
#if VIDEO_CODEC_H264DEC
      case d3d12_video_decode_profile_type_h264:
      {
         d3d12_video_decoder_refresh_dpb_active_references_h264(pD3D12Dec);
      } break;
#endif
#if VIDEO_CODEC_H265DEC
      case d3d12_video_decode_profile_type_hevc:
      {
         d3d12_video_decoder_refresh_dpb_active_references_hevc(pD3D12Dec);
      } break;
#endif
#if VIDEO_CODEC_AV1DEC
      case d3d12_video_decode_profile_type_av1:
      {
         d3d12_video_decoder_refresh_dpb_active_references_av1(pD3D12Dec);
      } break;
#endif
#if VIDEO_CODEC_VP9DEC
      case d3d12_video_decode_profile_type_vp9:
      {
         d3d12_video_decoder_refresh_dpb_active_references_vp9(pD3D12Dec);
      } break;
#endif
      default:
      {
         unreachable("Unsupported d3d12_video_decode_profile_type");
      } break;
   }
#endif // D3D12_VIDEO_ANY_DECODER_ENABLED
}

void
d3d12_video_decoder_get_frame_info(
   struct d3d12_video_decoder *pD3D12Dec, uint32_t *pWidth, uint32_t *pHeight, uint16_t *pMaxDPB, bool &isInterlaced)
{
   *pWidth = 0;
   *pHeight = 0;
   *pMaxDPB = 0;
   isInterlaced = false;

#if D3D12_VIDEO_ANY_DECODER_ENABLED
   switch (pD3D12Dec->m_d3d12DecProfileType) {
#if VIDEO_CODEC_H264DEC
      case d3d12_video_decode_profile_type_h264:
      {
         d3d12_video_decoder_get_frame_info_h264(pD3D12Dec, pWidth, pHeight, pMaxDPB, isInterlaced);
      } break;
#endif
#if VIDEO_CODEC_H265DEC
      case d3d12_video_decode_profile_type_hevc:
      {
         d3d12_video_decoder_get_frame_info_hevc(pD3D12Dec, pWidth, pHeight, pMaxDPB, isInterlaced);
      } break;
#endif
#if VIDEO_CODEC_AV1DEC
      case d3d12_video_decode_profile_type_av1:
      {
         d3d12_video_decoder_get_frame_info_av1(pD3D12Dec, pWidth, pHeight, pMaxDPB, isInterlaced);
      } break;
#endif
#if VIDEO_CODEC_VP9DEC
      case d3d12_video_decode_profile_type_vp9:
      {
         d3d12_video_decoder_get_frame_info_vp9(pD3D12Dec, pWidth, pHeight, pMaxDPB, &isInterlaced);
      } break;
#endif
      default:
      {
         unreachable("Unsupported d3d12_video_decode_profile_type");
      } break;
   }
#endif // D3D12_VIDEO_ANY_DECODER_ENABLED

   if (pD3D12Dec->m_ConfigDecoderSpecificFlags & d3d12_video_decode_config_specific_flag_alignment_height) {
      const uint32_t AlignmentMask = 31;
      *pHeight = (*pHeight + AlignmentMask) & ~AlignmentMask;
   }
}

void
d3d12_video_decoder_store_converted_dxva_picparams_from_pipe_input(
   struct d3d12_video_decoder *codec,   // input argument, current decoder
   struct pipe_picture_desc
      *picture,   // input argument, base structure of pipe_XXX_picture_desc where XXX is the codec name
   struct d3d12_video_buffer *pD3D12VideoBuffer   // input argument, target video buffer
)
{
#if D3D12_VIDEO_ANY_DECODER_ENABLED
   assert(picture);
   assert(codec);
   struct d3d12_video_decoder *pD3D12Dec = (struct d3d12_video_decoder *) codec;

   d3d12_video_decode_profile_type profileType =
      d3d12_video_decoder_convert_pipe_video_profile_to_profile_type(codec->base.profile);
   ID3D12Resource *pPipeD3D12DstResource = d3d12_resource_resource(pD3D12VideoBuffer->texture);
   D3D12_RESOURCE_DESC outputResourceDesc = GetDesc(pPipeD3D12DstResource);
   auto &inFlightResources = pD3D12Dec->m_inflightResourcesPool[d3d12_video_decoder_pool_current_index(pD3D12Dec)];
   inFlightResources.qp_matrix_frame_argument_enabled = false;
   switch (profileType) {
#if VIDEO_CODEC_H264DEC
      case d3d12_video_decode_profile_type_h264:
      {
         size_t dxvaPicParamsBufferSize = sizeof(DXVA_PicParams_H264);
         pipe_h264_picture_desc *pPicControlH264 = (pipe_h264_picture_desc *) picture;
         DXVA_PicParams_H264 dxvaPicParamsH264 =
            d3d12_video_decoder_dxva_picparams_from_pipe_picparams_h264(pD3D12Dec->m_fenceValue,
                                                                        codec->base.profile,
                                                                        outputResourceDesc.Width,
                                                                        outputResourceDesc.Height,
                                                                        pPicControlH264);

         d3d12_video_decoder_store_dxva_picparams_in_picparams_buffer(codec,
                                                                      &dxvaPicParamsH264,
                                                                      dxvaPicParamsBufferSize);

         size_t dxvaQMatrixBufferSize = sizeof(DXVA_Qmatrix_H264);
         DXVA_Qmatrix_H264 dxvaQmatrixH264 = {};
         d3d12_video_decoder_dxva_qmatrix_from_pipe_picparams_h264((pipe_h264_picture_desc *) picture, dxvaQmatrixH264);
         inFlightResources.qp_matrix_frame_argument_enabled =
            true;   // We don't have a way of knowing from the pipe params so send always
         d3d12_video_decoder_store_dxva_qmatrix_in_qmatrix_buffer(codec, &dxvaQmatrixH264, dxvaQMatrixBufferSize);
      } break;
#endif
#if VIDEO_CODEC_H265DEC
      case d3d12_video_decode_profile_type_hevc:
      {
         size_t dxvaPicParamsBufferSize = sizeof(DXVA_PicParams_HEVC);
         pipe_h265_picture_desc *pPicControlHEVC = (pipe_h265_picture_desc *) picture;
         DXVA_PicParams_HEVC dxvaPicParamsHEVC =
            d3d12_video_decoder_dxva_picparams_from_pipe_picparams_hevc(pD3D12Dec, codec->base.profile, pPicControlHEVC);

         d3d12_video_decoder_store_dxva_picparams_in_picparams_buffer(codec,
                                                                      &dxvaPicParamsHEVC,
                                                                      dxvaPicParamsBufferSize);

         size_t dxvaQMatrixBufferSize = sizeof(DXVA_Qmatrix_HEVC);
         DXVA_Qmatrix_HEVC dxvaQmatrixHEVC = {};
         inFlightResources.qp_matrix_frame_argument_enabled = false;
         d3d12_video_decoder_dxva_qmatrix_from_pipe_picparams_hevc((pipe_h265_picture_desc *) picture,
                                                                   dxvaQmatrixHEVC,
                                                                   inFlightResources.qp_matrix_frame_argument_enabled);
         d3d12_video_decoder_store_dxva_qmatrix_in_qmatrix_buffer(codec, &dxvaQmatrixHEVC, dxvaQMatrixBufferSize);
      } break;
#endif
#if VIDEO_CODEC_AV1DEC
      case d3d12_video_decode_profile_type_av1:
      {
         size_t dxvaPicParamsBufferSize = sizeof(DXVA_PicParams_AV1);
         pipe_av1_picture_desc *pPicControlAV1 = (pipe_av1_picture_desc *) picture;
         DXVA_PicParams_AV1 dxvaPicParamsAV1 =
            d3d12_video_decoder_dxva_picparams_from_pipe_picparams_av1(pD3D12Dec->m_fenceValue,
                                                                       codec->base.profile,
                                                                       pPicControlAV1);

         d3d12_video_decoder_store_dxva_picparams_in_picparams_buffer(codec, &dxvaPicParamsAV1, dxvaPicParamsBufferSize);
         inFlightResources.qp_matrix_frame_argument_enabled = false;
      } break;
#endif
#if VIDEO_CODEC_VP9DEC
      case d3d12_video_decode_profile_type_vp9:
      {
         size_t dxvaPicParamsBufferSize = sizeof(DXVA_PicParams_VP9);
         pipe_vp9_picture_desc *pPicControlVP9 = (pipe_vp9_picture_desc *) picture;
         DXVA_PicParams_VP9 dxvaPicParamsVP9 =
            d3d12_video_decoder_dxva_picparams_from_pipe_picparams_vp9(pD3D12Dec, codec->base.profile, pPicControlVP9);

         d3d12_video_decoder_store_dxva_picparams_in_picparams_buffer(codec, &dxvaPicParamsVP9, dxvaPicParamsBufferSize);
         inFlightResources.qp_matrix_frame_argument_enabled = false;
      } break;
#endif
      default:
      {
         unreachable("Unsupported d3d12_video_decode_profile_type");
      } break;
   }
#endif // D3D12_VIDEO_ANY_DECODER_ENABLED
}

void
d3d12_video_decoder_prepare_dxva_slices_control(
   struct d3d12_video_decoder *pD3D12Dec,   // input argument, current decoder
   struct pipe_picture_desc *picture)
{
#if D3D12_VIDEO_ANY_DECODER_ENABLED
   [[maybe_unused]] auto &inFlightResources = pD3D12Dec->m_inflightResourcesPool[d3d12_video_decoder_pool_current_index(pD3D12Dec)];
   d3d12_video_decode_profile_type profileType =
      d3d12_video_decoder_convert_pipe_video_profile_to_profile_type(pD3D12Dec->base.profile);
   switch (profileType) {
#if VIDEO_CODEC_H264DEC
      case d3d12_video_decode_profile_type_h264:
      {
         d3d12_video_decoder_prepare_dxva_slices_control_h264(pD3D12Dec,
                                                              inFlightResources.m_SliceControlBuffer,
                                                              (struct pipe_h264_picture_desc *) picture);
      } break;
#endif
#if VIDEO_CODEC_H265DEC
      case d3d12_video_decode_profile_type_hevc:
      {
         d3d12_video_decoder_prepare_dxva_slices_control_hevc(pD3D12Dec,
                                                              inFlightResources.m_SliceControlBuffer,
                                                              (struct pipe_h265_picture_desc *) picture);
      } break;
#endif
#if VIDEO_CODEC_AV1DEC
      case d3d12_video_decode_profile_type_av1:
      {
         d3d12_video_decoder_prepare_dxva_slices_control_av1(pD3D12Dec,
                                                             inFlightResources.m_SliceControlBuffer,
                                                             (struct pipe_av1_picture_desc *) picture);
      } break;
#endif
#if VIDEO_CODEC_VP9DEC
      case d3d12_video_decode_profile_type_vp9:
      {
         d3d12_video_decoder_prepare_dxva_slices_control_vp9(pD3D12Dec,
                                                             inFlightResources.m_SliceControlBuffer,
                                                             (struct pipe_vp9_picture_desc *) picture);
      } break;
#endif
      default:
      {
         unreachable("Unsupported d3d12_video_decode_profile_type");
      } break;
   }
#endif // D3D12_VIDEO_ANY_DECODER_ENABLED
}

void
d3d12_video_decoder_store_dxva_qmatrix_in_qmatrix_buffer(struct d3d12_video_decoder *pD3D12Dec,
                                                         void *pDXVAStruct,
                                                         uint64_t DXVAStructSize)
{
   auto &inFlightResources = pD3D12Dec->m_inflightResourcesPool[d3d12_video_decoder_pool_current_index(pD3D12Dec)];
   if (inFlightResources.m_InverseQuantMatrixBuffer.capacity() < DXVAStructSize) {
      inFlightResources.m_InverseQuantMatrixBuffer.reserve(DXVAStructSize);
   }

   inFlightResources.m_InverseQuantMatrixBuffer.resize(DXVAStructSize);
   memcpy(inFlightResources.m_InverseQuantMatrixBuffer.data(), pDXVAStruct, DXVAStructSize);
}

void
d3d12_video_decoder_store_dxva_picparams_in_picparams_buffer(struct d3d12_video_decoder *pD3D12Dec,
                                                             void *pDXVAStruct,
                                                             uint64_t DXVAStructSize)
{
   auto &inFlightResources = pD3D12Dec->m_inflightResourcesPool[d3d12_video_decoder_pool_current_index(pD3D12Dec)];
   if (inFlightResources.m_picParamsBuffer.capacity() < DXVAStructSize) {
      inFlightResources.m_picParamsBuffer.reserve(DXVAStructSize);
   }

   inFlightResources.m_picParamsBuffer.resize(DXVAStructSize);
   memcpy(inFlightResources.m_picParamsBuffer.data(), pDXVAStruct, DXVAStructSize);
}

bool
d3d12_video_decoder_supports_aot_dpb(D3D12_FEATURE_DATA_VIDEO_DECODE_SUPPORT decodeSupport,
                                     d3d12_video_decode_profile_type profileType)
{
   bool supportedProfile = false;
#if D3D12_VIDEO_ANY_DECODER_ENABLED
   switch (profileType) {
#if VIDEO_CODEC_H264DEC
      case d3d12_video_decode_profile_type_h264:
      {
         supportedProfile = true;
      } break;
#endif
#if VIDEO_CODEC_H265DEC
      case d3d12_video_decode_profile_type_hevc:
      {
         supportedProfile = true;
      } break;
#endif
#if VIDEO_CODEC_AV1DEC
      case d3d12_video_decode_profile_type_av1:
      {
         supportedProfile = true;
      } break;
#endif
#if VIDEO_CODEC_VP9DEC
      case d3d12_video_decode_profile_type_vp9:
      {
         supportedProfile = true;
      } break;
#endif
      default:
         supportedProfile = false;
         break;
   }
#endif // D3D12_VIDEO_ANY_DECODER_ENABLED

   return (decodeSupport.DecodeTier >= D3D12_VIDEO_DECODE_TIER_2) && supportedProfile;
}

d3d12_video_decode_profile_type
d3d12_video_decoder_convert_pipe_video_profile_to_profile_type(enum pipe_video_profile profile)
{
   switch (profile) {
      case PIPE_VIDEO_PROFILE_MPEG4_AVC_BASELINE:
      case PIPE_VIDEO_PROFILE_MPEG4_AVC_CONSTRAINED_BASELINE:
      case PIPE_VIDEO_PROFILE_MPEG4_AVC_MAIN:
      case PIPE_VIDEO_PROFILE_MPEG4_AVC_EXTENDED:
      case PIPE_VIDEO_PROFILE_MPEG4_AVC_HIGH:
      case PIPE_VIDEO_PROFILE_MPEG4_AVC_HIGH10:
         return d3d12_video_decode_profile_type_h264;
      case PIPE_VIDEO_PROFILE_HEVC_MAIN:
      case PIPE_VIDEO_PROFILE_HEVC_MAIN_10:
         return d3d12_video_decode_profile_type_hevc;
      case PIPE_VIDEO_PROFILE_AV1_MAIN:
         return d3d12_video_decode_profile_type_av1;
      case PIPE_VIDEO_PROFILE_VP9_PROFILE0:
      case PIPE_VIDEO_PROFILE_VP9_PROFILE2:
         return d3d12_video_decode_profile_type_vp9;
      default:
      {
         unreachable("Unsupported pipe video profile");
      } break;
   }
}

GUID
d3d12_video_decoder_convert_pipe_video_profile_to_d3d12_profile(enum pipe_video_profile profile)
{
   switch (profile) {
      case PIPE_VIDEO_PROFILE_MPEG4_AVC_BASELINE:
      case PIPE_VIDEO_PROFILE_MPEG4_AVC_CONSTRAINED_BASELINE:
      case PIPE_VIDEO_PROFILE_MPEG4_AVC_MAIN:
      case PIPE_VIDEO_PROFILE_MPEG4_AVC_EXTENDED:
      case PIPE_VIDEO_PROFILE_MPEG4_AVC_HIGH:
      case PIPE_VIDEO_PROFILE_MPEG4_AVC_HIGH10:
         return D3D12_VIDEO_DECODE_PROFILE_H264;
      case PIPE_VIDEO_PROFILE_HEVC_MAIN:
         return D3D12_VIDEO_DECODE_PROFILE_HEVC_MAIN;
      case PIPE_VIDEO_PROFILE_HEVC_MAIN_10:
         return D3D12_VIDEO_DECODE_PROFILE_HEVC_MAIN10;
      case PIPE_VIDEO_PROFILE_AV1_MAIN:
         return D3D12_VIDEO_DECODE_PROFILE_AV1_PROFILE0;
      case PIPE_VIDEO_PROFILE_VP9_PROFILE0:
         return D3D12_VIDEO_DECODE_PROFILE_VP9;
      case PIPE_VIDEO_PROFILE_VP9_PROFILE2:
         return D3D12_VIDEO_DECODE_PROFILE_VP9_10BIT_PROFILE2;
      default:
         return {};
   }
}

GUID
d3d12_video_decoder_resolve_profile(d3d12_video_decode_profile_type profileType, DXGI_FORMAT decode_format)
{
#if D3D12_VIDEO_ANY_DECODER_ENABLED
   switch (profileType) {
#if VIDEO_CODEC_H264DEC
      case d3d12_video_decode_profile_type_h264:
         return D3D12_VIDEO_DECODE_PROFILE_H264;
#endif
#if VIDEO_CODEC_H265DEC
      case d3d12_video_decode_profile_type_hevc:
      {
         switch (decode_format) {
            case DXGI_FORMAT_NV12:
               return D3D12_VIDEO_DECODE_PROFILE_HEVC_MAIN;
            case DXGI_FORMAT_P010:
               return D3D12_VIDEO_DECODE_PROFILE_HEVC_MAIN10;
            default:
            {
               unreachable("Unsupported decode_format");
            } break;
         }
      } break;
#endif
#if VIDEO_CODEC_AV1DEC
      case d3d12_video_decode_profile_type_av1:
         return D3D12_VIDEO_DECODE_PROFILE_AV1_PROFILE0;
         break;
#endif
#if VIDEO_CODEC_VP9DEC
      case d3d12_video_decode_profile_type_vp9:
      {
         switch (decode_format) {
            case DXGI_FORMAT_NV12:
               return D3D12_VIDEO_DECODE_PROFILE_VP9;
            case DXGI_FORMAT_P010:
               return D3D12_VIDEO_DECODE_PROFILE_VP9_10BIT_PROFILE2;
            default:
            {
               unreachable("Unsupported decode_format");
            } break;
         }
      } break;
#endif
      default:
      {
         unreachable("Unsupported d3d12_video_decode_profile_type");
      } break;
   }
#else
   return {};
#endif // D3D12_VIDEO_ANY_DECODER_ENABLED
}

bool
d3d12_video_decoder_ensure_fence_finished(struct pipe_video_codec *codec,
                                          ID3D12Fence *fence,
                                          uint64_t fenceValueToWaitOn,
                                          uint64_t timeout_ns)
{
   bool wait_result = true;
   HRESULT hr = S_OK;
   uint64_t completedValue = fence->GetCompletedValue();

   debug_printf(
      "[d3d12_video_decoder] d3d12_video_decoder_ensure_fence_finished - Waiting for fence (with timeout_ns %" PRIu64
      ") to finish with "
      "fenceValue: %" PRIu64 " - Current Fence Completed Value %" PRIu64 "\n",
      timeout_ns,
      fenceValueToWaitOn,
      completedValue);

   if (completedValue < fenceValueToWaitOn) {

      HANDLE event = {};
      int event_fd = 0;
      event = d3d12_fence_create_event(&event_fd);

      hr = fence->SetEventOnCompletion(fenceValueToWaitOn, event);
      if (FAILED(hr)) {
         debug_printf("[d3d12_video_decoder] d3d12_video_decoder_ensure_fence_finished - SetEventOnCompletion for "
                      "fenceValue %" PRIu64 " failed with HR %x\n",
                      fenceValueToWaitOn,
                      hr);
         return false;
      }

      wait_result = d3d12_fence_wait_event(event, event_fd, timeout_ns);
      d3d12_fence_close_event(event, event_fd);

      debug_printf("[d3d12_video_decoder] d3d12_video_decoder_ensure_fence_finished - Waiting on fence to be done with "
                   "fenceValue: %" PRIu64 " - current CompletedValue: %" PRIu64 "\n",
                   fenceValueToWaitOn,
                   completedValue);
   } else {
      debug_printf("[d3d12_video_decoder] d3d12_video_decoder_ensure_fence_finished - Fence already done with "
                   "fenceValue: %" PRIu64 " - current CompletedValue: %" PRIu64 "\n",
                   fenceValueToWaitOn,
                   completedValue);
   }
   return wait_result;
}

bool
d3d12_video_decoder_sync_completion(struct pipe_video_codec *codec,
                                    ID3D12Fence *fence,
                                    uint64_t fenceValueToWaitOn,
                                    uint64_t timeout_ns)
{
   struct d3d12_video_decoder *pD3D12Dec = (struct d3d12_video_decoder *) codec;
   assert(pD3D12Dec);
   assert(pD3D12Dec->m_spD3D12VideoDevice);
   assert(pD3D12Dec->m_spDecodeCommandQueue);
   HRESULT hr = S_OK;

   ASSERTED bool wait_result = d3d12_video_decoder_ensure_fence_finished(codec, fence, fenceValueToWaitOn, timeout_ns);
   assert(wait_result);

   // Release references granted on end_frame for this inflight operations
   pD3D12Dec->m_inflightResourcesPool[fenceValueToWaitOn % D3D12_VIDEO_DEC_ASYNC_DEPTH].m_spDecoder.Reset();
   pD3D12Dec->m_inflightResourcesPool[fenceValueToWaitOn % D3D12_VIDEO_DEC_ASYNC_DEPTH].m_spDecoderHeap.Reset();
   pD3D12Dec->m_inflightResourcesPool[fenceValueToWaitOn % D3D12_VIDEO_DEC_ASYNC_DEPTH].m_References.reset();
   pD3D12Dec->m_inflightResourcesPool[fenceValueToWaitOn % D3D12_VIDEO_DEC_ASYNC_DEPTH].m_stagingDecodeBitstream.resize(
      0);
   pipe_resource_reference(
      &pD3D12Dec->m_inflightResourcesPool[fenceValueToWaitOn % D3D12_VIDEO_DEC_ASYNC_DEPTH].pPipeCompressedBufferObj,
      NULL);

   struct d3d12_screen *pD3D12Screen = (struct d3d12_screen *) pD3D12Dec->m_pD3D12Screen;
   assert(pD3D12Screen);

   pD3D12Screen->base.fence_reference(
      &pD3D12Screen->base,
      &pD3D12Dec->m_inflightResourcesPool[d3d12_video_decoder_pool_current_index(pD3D12Dec)]
          .m_pBitstreamUploadGPUCompletionFence,
      NULL);

   hr =
      pD3D12Dec->m_inflightResourcesPool[fenceValueToWaitOn % D3D12_VIDEO_DEC_ASYNC_DEPTH].m_spCommandAllocator->Reset();
   if (FAILED(hr)) {
      debug_printf("failed with %x.\n", hr);
      goto sync_with_token_fail;
   }

   // Validate device was not removed
   hr = pD3D12Dec->m_pD3D12Screen->dev->GetDeviceRemovedReason();
   if (hr != S_OK) {
      debug_printf("[d3d12_video_decoder] d3d12_video_decoder_sync_completion"
                   " - D3D12Device was removed AFTER d3d12_video_decoder_ensure_fence_finished "
                   "execution with HR %x, but wasn't before.\n",
                   hr);
      goto sync_with_token_fail;
   }

   debug_printf(
      "[d3d12_video_decoder] d3d12_video_decoder_sync_completion - GPU execution finalized for fenceValue: %" PRIu64
      "\n",
      fenceValueToWaitOn);

   return wait_result;

sync_with_token_fail:
   debug_printf("[d3d12_video_decoder] d3d12_video_decoder_sync_completion failed for fenceValue: %" PRIu64 "\n",
                fenceValueToWaitOn);
   assert(false);
   return false;
}