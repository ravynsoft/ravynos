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

#include "d3d12_common.h"

#include "d3d12_util.h"
#include "d3d12_context.h"
#include "d3d12_format.h"
#include "d3d12_resource.h"
#include "d3d12_screen.h"
#include "d3d12_surface.h"
#include "d3d12_video_enc.h"
#if VIDEO_CODEC_H264ENC
#include "d3d12_video_enc_h264.h"
#endif
#if VIDEO_CODEC_H265ENC
#include "d3d12_video_enc_hevc.h"
#endif
#if VIDEO_CODEC_AV1ENC
#include "d3d12_video_enc_av1.h"
#endif
#include "d3d12_video_buffer.h"
#include "d3d12_video_texture_array_dpb_manager.h"
#include "d3d12_video_array_of_textures_dpb_manager.h"
#include "d3d12_video_encoder_references_manager_h264.h"
#include "d3d12_video_encoder_references_manager_hevc.h"
#include "d3d12_video_encoder_references_manager_av1.h"
#include "d3d12_residency.h"

#include "vl/vl_video_buffer.h"
#include "util/format/u_format.h"
#include "util/u_inlines.h"
#include "util/u_memory.h"
#include "util/u_video.h"

#include <cmath>

D3D12_VIDEO_ENCODER_CODEC
d3d12_video_encoder_convert_codec_to_d3d12_enc_codec(enum pipe_video_profile profile)
{
   switch (u_reduce_video_profile(profile)) {
      case PIPE_VIDEO_FORMAT_MPEG4_AVC:
      {
         return D3D12_VIDEO_ENCODER_CODEC_H264;
      } break;
      case PIPE_VIDEO_FORMAT_HEVC:
      {
         return D3D12_VIDEO_ENCODER_CODEC_HEVC;
      } break;
      case PIPE_VIDEO_FORMAT_AV1:
      {
         return D3D12_VIDEO_ENCODER_CODEC_AV1;
      } break;
      case PIPE_VIDEO_FORMAT_MPEG12:
      case PIPE_VIDEO_FORMAT_MPEG4:
      case PIPE_VIDEO_FORMAT_VC1:
      case PIPE_VIDEO_FORMAT_JPEG:
      case PIPE_VIDEO_FORMAT_VP9:
      case PIPE_VIDEO_FORMAT_UNKNOWN:
      default:
      {
         unreachable("Unsupported pipe_video_profile");
      } break;
   }
}

uint64_t
d3d12_video_encoder_pool_current_index(struct d3d12_video_encoder *pD3D12Enc)
{
   return pD3D12Enc->m_fenceValue % D3D12_VIDEO_ENC_ASYNC_DEPTH;
}

void
d3d12_video_encoder_flush(struct pipe_video_codec *codec)
{
   struct d3d12_video_encoder *pD3D12Enc = (struct d3d12_video_encoder *) codec;
   assert(pD3D12Enc);
   assert(pD3D12Enc->m_spD3D12VideoDevice);
   assert(pD3D12Enc->m_spEncodeCommandQueue);

   if (pD3D12Enc->m_inflightResourcesPool[d3d12_video_encoder_pool_current_index(pD3D12Enc)].encode_result & PIPE_VIDEO_FEEDBACK_METADATA_ENCODE_FLAG_FAILED) {
      debug_printf("WARNING: [d3d12_video_encoder] d3d12_video_encoder_flush - Frame submission %" PRIu64 " failed. Encoder lost, please recreate pipe_video_codec object \n", pD3D12Enc->m_fenceValue);
      assert(false);
      return;
   }

   // Flush any work batched (ie. shaders blit on input texture, etc or bitstream headers buffer_subdata batched upload)
   // and Wait the m_spEncodeCommandQueue for GPU upload completion
   // before recording EncodeFrame below.
   struct pipe_fence_handle *completion_fence = NULL;
   debug_printf("[d3d12_video_encoder] d3d12_video_encoder_flush - Flushing pD3D12Enc->base.context and GPU sync between Video/Context queues before flushing Video Encode Queue.\n");
   pD3D12Enc->base.context->flush(pD3D12Enc->base.context, &completion_fence, PIPE_FLUSH_ASYNC | PIPE_FLUSH_HINT_FINISH);
   assert(completion_fence);
   struct d3d12_fence *casted_completion_fence = d3d12_fence(completion_fence);
   pD3D12Enc->m_spEncodeCommandQueue->Wait(casted_completion_fence->cmdqueue_fence, casted_completion_fence->value);
   pD3D12Enc->m_pD3D12Screen->base.fence_reference(&pD3D12Enc->m_pD3D12Screen->base, &completion_fence, NULL);

   struct d3d12_fence *input_surface_fence = pD3D12Enc->m_inflightResourcesPool[d3d12_video_encoder_pool_current_index(pD3D12Enc)].m_InputSurfaceFence;
   if (input_surface_fence)
      pD3D12Enc->m_spEncodeCommandQueue->Wait(input_surface_fence->cmdqueue_fence, input_surface_fence->value);

   if (!pD3D12Enc->m_bPendingWorkNotFlushed) {
      debug_printf("[d3d12_video_encoder] d3d12_video_encoder_flush started. Nothing to flush, all up to date.\n");
   } else {
      debug_printf("[d3d12_video_encoder] d3d12_video_encoder_flush started. Will flush video queue work async"
                    " on fenceValue: %" PRIu64 "\n",
                    pD3D12Enc->m_fenceValue);

      HRESULT hr = pD3D12Enc->m_pD3D12Screen->dev->GetDeviceRemovedReason();
      if (hr != S_OK) {
         debug_printf("[d3d12_video_encoder] d3d12_video_encoder_flush"
                         " - D3D12Device was removed BEFORE commandlist "
                         "execution with HR %x.\n",
                         hr);
         goto flush_fail;
      }

      if (pD3D12Enc->m_transitionsBeforeCloseCmdList.size() > 0) {
         pD3D12Enc->m_spEncodeCommandList->ResourceBarrier(pD3D12Enc->m_transitionsBeforeCloseCmdList.size(),
                                                           pD3D12Enc->m_transitionsBeforeCloseCmdList.data());
         pD3D12Enc->m_transitionsBeforeCloseCmdList.clear();
      }

      hr = pD3D12Enc->m_spEncodeCommandList->Close();
      if (FAILED(hr)) {
         debug_printf("[d3d12_video_encoder] d3d12_video_encoder_flush - Can't close command list with HR %x\n", hr);
         goto flush_fail;
      }

      ID3D12CommandList *ppCommandLists[1] = { pD3D12Enc->m_spEncodeCommandList.Get() };
      pD3D12Enc->m_spEncodeCommandQueue->ExecuteCommandLists(1, ppCommandLists);
      pD3D12Enc->m_spEncodeCommandQueue->Signal(pD3D12Enc->m_spFence.Get(), pD3D12Enc->m_fenceValue);

      // Validate device was not removed
      hr = pD3D12Enc->m_pD3D12Screen->dev->GetDeviceRemovedReason();
      if (hr != S_OK) {
         debug_printf("[d3d12_video_encoder] d3d12_video_encoder_flush" 
                         " - D3D12Device was removed AFTER commandlist "
                         "execution with HR %x, but wasn't before.\n",
                         hr);
         goto flush_fail;
      }

      pD3D12Enc->m_fenceValue++;
      pD3D12Enc->m_bPendingWorkNotFlushed = false;
   }
   return;

flush_fail:
   debug_printf("[d3d12_video_encoder] d3d12_video_encoder_flush failed for fenceValue: %" PRIu64 "\n", pD3D12Enc->m_fenceValue);
   pD3D12Enc->m_inflightResourcesPool[d3d12_video_encoder_pool_current_index(pD3D12Enc)].encode_result = PIPE_VIDEO_FEEDBACK_METADATA_ENCODE_FLAG_FAILED;
   pD3D12Enc->m_spEncodedFrameMetadata[pD3D12Enc->m_fenceValue % D3D12_VIDEO_ENC_METADATA_BUFFERS_COUNT].encode_result = PIPE_VIDEO_FEEDBACK_METADATA_ENCODE_FLAG_FAILED;
   assert(false);
}

void
d3d12_video_encoder_ensure_fence_finished(struct pipe_video_codec *codec, uint64_t fenceValueToWaitOn, uint64_t timeout_ns)
{
      struct d3d12_video_encoder *pD3D12Enc = (struct d3d12_video_encoder *) codec;
      HRESULT hr = S_OK;
      uint64_t completedValue = pD3D12Enc->m_spFence->GetCompletedValue();

      debug_printf("[d3d12_video_encoder] d3d12_video_encoder_ensure_fence_finished - Waiting for fence (with timeout_ns %" PRIu64 ") to finish with "
                    "fenceValue: %" PRIu64 " - Current Fence Completed Value %" PRIu64 "\n",
                    timeout_ns, fenceValueToWaitOn, completedValue);

      if(completedValue < fenceValueToWaitOn) {

         HANDLE              event = { };
         int                 event_fd = 0;
         event = d3d12_fence_create_event(&event_fd);

         hr = pD3D12Enc->m_spFence->SetEventOnCompletion(fenceValueToWaitOn, event);
         if (FAILED(hr)) {
            debug_printf(
               "[d3d12_video_encoder] d3d12_video_encoder_ensure_fence_finished - SetEventOnCompletion for fenceValue %" PRIu64 " failed with HR %x\n",
               fenceValueToWaitOn, hr);
            goto ensure_fence_finished_fail;
         }

         debug_printf("[d3d12_video_encoder] d3d12_video_encoder_ensure_fence_finished - Waiting on fence to be done with "
               "fenceValue: %" PRIu64 " - current CompletedValue: %" PRIu64 "\n",
               fenceValueToWaitOn,
               completedValue);

         d3d12_fence_wait_event(event, event_fd, timeout_ns);
         d3d12_fence_close_event(event, event_fd);
      } else {
         debug_printf("[d3d12_video_encoder] d3d12_video_encoder_ensure_fence_finished - Fence already done with "
               "fenceValue: %" PRIu64 " - current CompletedValue: %" PRIu64 "\n",
               fenceValueToWaitOn,
               completedValue);
      }
      return;

ensure_fence_finished_fail:
   debug_printf("[d3d12_video_encoder] d3d12_video_encoder_sync_completion failed for fenceValue: %" PRIu64 "\n", fenceValueToWaitOn);
   pD3D12Enc->m_inflightResourcesPool[fenceValueToWaitOn % D3D12_VIDEO_ENC_ASYNC_DEPTH].encode_result = PIPE_VIDEO_FEEDBACK_METADATA_ENCODE_FLAG_FAILED;
   pD3D12Enc->m_spEncodedFrameMetadata[fenceValueToWaitOn % D3D12_VIDEO_ENC_METADATA_BUFFERS_COUNT].encode_result = PIPE_VIDEO_FEEDBACK_METADATA_ENCODE_FLAG_FAILED;
   assert(false);
}

void
d3d12_video_encoder_sync_completion(struct pipe_video_codec *codec, uint64_t fenceValueToWaitOn, uint64_t timeout_ns)
{
      struct d3d12_video_encoder *pD3D12Enc = (struct d3d12_video_encoder *) codec;
      assert(pD3D12Enc);
      assert(pD3D12Enc->m_spD3D12VideoDevice);
      assert(pD3D12Enc->m_spEncodeCommandQueue);
      HRESULT hr = S_OK;

      d3d12_video_encoder_ensure_fence_finished(codec, fenceValueToWaitOn, timeout_ns);

      debug_printf("[d3d12_video_encoder] d3d12_video_encoder_sync_completion - resetting ID3D12CommandAllocator %p...\n",
         pD3D12Enc->m_inflightResourcesPool[fenceValueToWaitOn % D3D12_VIDEO_ENC_ASYNC_DEPTH].m_spCommandAllocator.Get());
      hr = pD3D12Enc->m_inflightResourcesPool[fenceValueToWaitOn % D3D12_VIDEO_ENC_ASYNC_DEPTH].m_spCommandAllocator->Reset();
      if(FAILED(hr)) {
         debug_printf("failed with %x.\n", hr);
         goto sync_with_token_fail;
      }

      // Release references granted on end_frame for this inflight operations
      pD3D12Enc->m_inflightResourcesPool[fenceValueToWaitOn % D3D12_VIDEO_ENC_ASYNC_DEPTH].m_spEncoder.Reset();
      pD3D12Enc->m_inflightResourcesPool[fenceValueToWaitOn % D3D12_VIDEO_ENC_ASYNC_DEPTH].m_spEncoderHeap.Reset();
      pD3D12Enc->m_inflightResourcesPool[fenceValueToWaitOn % D3D12_VIDEO_ENC_ASYNC_DEPTH].m_References.reset();
      pD3D12Enc->m_inflightResourcesPool[fenceValueToWaitOn % D3D12_VIDEO_ENC_ASYNC_DEPTH].m_InputSurfaceFence = NULL;

      // Validate device was not removed
      hr = pD3D12Enc->m_pD3D12Screen->dev->GetDeviceRemovedReason();
      if (hr != S_OK) {
         debug_printf("[d3d12_video_encoder] d3d12_video_encoder_sync_completion"
                         " - D3D12Device was removed AFTER d3d12_video_encoder_ensure_fence_finished "
                         "execution with HR %x, but wasn't before.\n",
                         hr);
         goto sync_with_token_fail;
      }

      debug_printf(
         "[d3d12_video_encoder] d3d12_video_encoder_sync_completion - GPU execution finalized for fenceValue: %" PRIu64 "\n",
         fenceValueToWaitOn);

      return;

sync_with_token_fail:
   debug_printf("[d3d12_video_encoder] d3d12_video_encoder_sync_completion failed for fenceValue: %" PRIu64 "\n", fenceValueToWaitOn);
   pD3D12Enc->m_inflightResourcesPool[fenceValueToWaitOn % D3D12_VIDEO_ENC_ASYNC_DEPTH].encode_result = PIPE_VIDEO_FEEDBACK_METADATA_ENCODE_FLAG_FAILED;
   pD3D12Enc->m_spEncodedFrameMetadata[fenceValueToWaitOn % D3D12_VIDEO_ENC_METADATA_BUFFERS_COUNT].encode_result = PIPE_VIDEO_FEEDBACK_METADATA_ENCODE_FLAG_FAILED;
   assert(false);
}

/**
 * Destroys a d3d12_video_encoder
 * Call destroy_XX for applicable XX nested member types before deallocating
 * Destroy methods should check != nullptr on their input target argument as this method can be called as part of
 * cleanup from failure on the creation method
 */
void
d3d12_video_encoder_destroy(struct pipe_video_codec *codec)
{
   if (codec == nullptr) {
      return;
   }

   struct d3d12_video_encoder *pD3D12Enc = (struct d3d12_video_encoder *) codec;

      // Flush pending work before destroying
   if(pD3D12Enc->m_bPendingWorkNotFlushed){
      uint64_t curBatchFence = pD3D12Enc->m_fenceValue;
      d3d12_video_encoder_flush(codec);
      d3d12_video_encoder_sync_completion(codec, curBatchFence, OS_TIMEOUT_INFINITE);
   }

   // Call d3d12_video_encoder dtor to make ComPtr and other member's destructors work
   delete pD3D12Enc;
}

void
d3d12_video_encoder_update_picparams_tracking(struct d3d12_video_encoder *pD3D12Enc,
                                              struct pipe_video_buffer *  srcTexture,
                                              struct pipe_picture_desc *  picture)
{
   D3D12_VIDEO_ENCODER_PICTURE_CONTROL_CODEC_DATA currentPicParams =
      d3d12_video_encoder_get_current_picture_param_settings(pD3D12Enc);

   enum pipe_video_format codec = u_reduce_video_profile(pD3D12Enc->base.profile);
   bool bUsedAsReference = false;
   switch (codec) {
#if VIDEO_CODEC_H264ENC
      case PIPE_VIDEO_FORMAT_MPEG4_AVC:
      {
         d3d12_video_encoder_update_current_frame_pic_params_info_h264(pD3D12Enc, srcTexture, picture, currentPicParams, bUsedAsReference);
      } break;
#endif
#if VIDEO_CODEC_H265ENC
      case PIPE_VIDEO_FORMAT_HEVC:
      {
         d3d12_video_encoder_update_current_frame_pic_params_info_hevc(pD3D12Enc, srcTexture, picture, currentPicParams, bUsedAsReference);
      } break;
#endif
#if VIDEO_CODEC_AV1ENC
      case PIPE_VIDEO_FORMAT_AV1:
      {
         d3d12_video_encoder_update_current_frame_pic_params_info_av1(pD3D12Enc, srcTexture, picture, currentPicParams, bUsedAsReference);
      } break;
#endif
      default:
      {
         unreachable("Unsupported pipe_video_format");
      } break;
   }

   pD3D12Enc->m_upDPBManager->begin_frame(currentPicParams, bUsedAsReference, picture);
}

bool
d3d12_video_encoder_reconfigure_encoder_objects(struct d3d12_video_encoder *pD3D12Enc,
                                                struct pipe_video_buffer *  srcTexture,
                                                struct pipe_picture_desc *  picture)
{
   bool codecChanged =
      ((pD3D12Enc->m_currentEncodeConfig.m_ConfigDirtyFlags & d3d12_video_encoder_config_dirty_flag_codec) != 0);
   bool profileChanged =
      ((pD3D12Enc->m_currentEncodeConfig.m_ConfigDirtyFlags & d3d12_video_encoder_config_dirty_flag_profile) != 0);
   bool levelChanged =
      ((pD3D12Enc->m_currentEncodeConfig.m_ConfigDirtyFlags & d3d12_video_encoder_config_dirty_flag_level) != 0);
   bool codecConfigChanged =
      ((pD3D12Enc->m_currentEncodeConfig.m_ConfigDirtyFlags & d3d12_video_encoder_config_dirty_flag_codec_config) != 0);
   bool inputFormatChanged =
      ((pD3D12Enc->m_currentEncodeConfig.m_ConfigDirtyFlags & d3d12_video_encoder_config_dirty_flag_input_format) != 0);
   bool resolutionChanged =
      ((pD3D12Enc->m_currentEncodeConfig.m_ConfigDirtyFlags & d3d12_video_encoder_config_dirty_flag_resolution) != 0);
   bool rateControlChanged =
      ((pD3D12Enc->m_currentEncodeConfig.m_ConfigDirtyFlags & d3d12_video_encoder_config_dirty_flag_rate_control) != 0);
   bool slicesChanged =
      ((pD3D12Enc->m_currentEncodeConfig.m_ConfigDirtyFlags & d3d12_video_encoder_config_dirty_flag_slices) != 0);
   bool gopChanged =
      ((pD3D12Enc->m_currentEncodeConfig.m_ConfigDirtyFlags & d3d12_video_encoder_config_dirty_flag_gop) != 0);
   bool motionPrecisionLimitChanged = ((pD3D12Enc->m_currentEncodeConfig.m_ConfigDirtyFlags &
                                        d3d12_video_encoder_config_dirty_flag_motion_precision_limit) != 0);
   bool irChanged = ((pD3D12Enc->m_currentEncodeConfig.m_ConfigDirtyFlags &
                                        d3d12_video_encoder_config_dirty_flag_intra_refresh) != 0);

   // Events that that trigger a re-creation of the reference picture manager
   // Stores codec agnostic textures so only input format, resolution and gop (num dpb references) affects this
   if (!pD3D12Enc->m_upDPBManager
       // || codecChanged
       // || profileChanged
       // || levelChanged
       // || codecConfigChanged
       || inputFormatChanged ||
       resolutionChanged
       // || rateControlChanged
       // || slicesChanged
       || gopChanged
       // || motionPrecisionLimitChanged
   ) {
      if (!pD3D12Enc->m_upDPBManager) {
         debug_printf("[d3d12_video_encoder] d3d12_video_encoder_reconfigure_encoder_objects - Creating Reference "
                       "Pictures Manager for the first time\n");
      } else {
         debug_printf("[d3d12_video_encoder] Reconfiguration triggered -> Re-creating Reference Pictures Manager\n");
      }

      D3D12_RESOURCE_FLAGS resourceAllocFlags =
         D3D12_RESOURCE_FLAG_VIDEO_ENCODE_REFERENCE_ONLY | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
      bool     fArrayOfTextures = ((pD3D12Enc->m_currentEncodeCapabilities.m_SupportFlags &
                                D3D12_VIDEO_ENCODER_SUPPORT_FLAG_RECONSTRUCTED_FRAMES_REQUIRE_TEXTURE_ARRAYS) == 0);
      uint32_t texturePoolSize  = d3d12_video_encoder_get_current_max_dpb_capacity(pD3D12Enc) +
                                 1u;   // adding an extra slot as we also need to count the current frame output recon
                                       // allocation along max reference frame allocations
      assert(texturePoolSize < UINT16_MAX);
      pD3D12Enc->m_upDPBStorageManager.reset();
      if (fArrayOfTextures) {
         pD3D12Enc->m_upDPBStorageManager = std::make_unique<d3d12_array_of_textures_dpb_manager>(
            static_cast<uint16_t>(texturePoolSize),
            pD3D12Enc->m_pD3D12Screen->dev,
            pD3D12Enc->m_currentEncodeConfig.m_encodeFormatInfo.Format,
            pD3D12Enc->m_currentEncodeConfig.m_currentResolution,
            (D3D12_RESOURCE_FLAG_VIDEO_ENCODE_REFERENCE_ONLY | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE),
            true,   // setNullSubresourcesOnAllZero - D3D12 Video Encode expects nullptr pSubresources if AoT,
            pD3D12Enc->m_NodeMask,
            /*use underlying pool, we can't reuse upper level allocations, need D3D12_RESOURCE_FLAG_VIDEO_ENCODE_REFERENCE_ONLY*/
            true);
      } else {
         pD3D12Enc->m_upDPBStorageManager = std::make_unique<d3d12_texture_array_dpb_manager>(
            static_cast<uint16_t>(texturePoolSize),
            pD3D12Enc->m_pD3D12Screen->dev,
            pD3D12Enc->m_currentEncodeConfig.m_encodeFormatInfo.Format,
            pD3D12Enc->m_currentEncodeConfig.m_currentResolution,
            resourceAllocFlags,
            pD3D12Enc->m_NodeMask);
      }
      d3d12_video_encoder_create_reference_picture_manager(pD3D12Enc, picture);
   }

   bool reCreatedEncoder = false;
   // Events that that trigger a re-creation of the encoder
   if (!pD3D12Enc->m_spVideoEncoder || codecChanged ||
       profileChanged
       // || levelChanged // Only affects encoder heap
       || codecConfigChanged ||
       inputFormatChanged
       // || resolutionChanged // Only affects encoder heap
       // Only re-create if there is NO SUPPORT for reconfiguring rateControl on the fly
       || (rateControlChanged && ((pD3D12Enc->m_currentEncodeCapabilities.m_SupportFlags &
                                   D3D12_VIDEO_ENCODER_SUPPORT_FLAG_RATE_CONTROL_RECONFIGURATION_AVAILABLE) ==
                                  0 /*checking the flag is NOT set*/))
       // Only re-create if there is NO SUPPORT for reconfiguring slices on the fly
       || (slicesChanged && ((pD3D12Enc->m_currentEncodeCapabilities.m_SupportFlags &
                              D3D12_VIDEO_ENCODER_SUPPORT_FLAG_SUBREGION_LAYOUT_RECONFIGURATION_AVAILABLE) ==
                             0 /*checking the flag is NOT set*/))
       // Only re-create if there is NO SUPPORT for reconfiguring gop on the fly
       || (gopChanged && ((pD3D12Enc->m_currentEncodeCapabilities.m_SupportFlags &
                           D3D12_VIDEO_ENCODER_SUPPORT_FLAG_SEQUENCE_GOP_RECONFIGURATION_AVAILABLE) ==
                          0 /*checking the flag is NOT set*/)) ||
       motionPrecisionLimitChanged) {
      if (!pD3D12Enc->m_spVideoEncoder) {
         debug_printf("[d3d12_video_encoder] d3d12_video_encoder_reconfigure_encoder_objects - Creating "
                       "D3D12VideoEncoder for the first time\n");
      } else {
         debug_printf("[d3d12_video_encoder] Reconfiguration triggered -> Re-creating D3D12VideoEncoder\n");
         reCreatedEncoder = true;
      }

      D3D12_VIDEO_ENCODER_DESC encoderDesc = { pD3D12Enc->m_NodeMask,
                                               D3D12_VIDEO_ENCODER_FLAG_NONE,
                                               pD3D12Enc->m_currentEncodeConfig.m_encoderCodecDesc,
                                               d3d12_video_encoder_get_current_profile_desc(pD3D12Enc),
                                               pD3D12Enc->m_currentEncodeConfig.m_encodeFormatInfo.Format,
                                               d3d12_video_encoder_get_current_codec_config_desc(pD3D12Enc),
                                               pD3D12Enc->m_currentEncodeConfig.m_encoderMotionPrecisionLimit };

      // Create encoder
      pD3D12Enc->m_spVideoEncoder.Reset();
      HRESULT hr = pD3D12Enc->m_spD3D12VideoDevice->CreateVideoEncoder(&encoderDesc,
                                                             IID_PPV_ARGS(pD3D12Enc->m_spVideoEncoder.GetAddressOf()));
      if (FAILED(hr)) {
         debug_printf("CreateVideoEncoder failed with HR %x\n", hr);
         return false;
      }
   }

   bool reCreatedEncoderHeap = false;
   // Events that that trigger a re-creation of the encoder heap
   if (!pD3D12Enc->m_spVideoEncoderHeap || codecChanged || profileChanged ||
       levelChanged
       // || codecConfigChanged // Only affects encoder
       || inputFormatChanged   // Might affect internal textures in the heap
       || resolutionChanged
       // Only re-create if there is NO SUPPORT for reconfiguring rateControl on the fly
       || (rateControlChanged && ((pD3D12Enc->m_currentEncodeCapabilities.m_SupportFlags &
                                   D3D12_VIDEO_ENCODER_SUPPORT_FLAG_RATE_CONTROL_RECONFIGURATION_AVAILABLE) ==
                                  0 /*checking the flag is NOT set*/))
       // Only re-create if there is NO SUPPORT for reconfiguring slices on the fly
       || (slicesChanged && ((pD3D12Enc->m_currentEncodeCapabilities.m_SupportFlags &
                              D3D12_VIDEO_ENCODER_SUPPORT_FLAG_SUBREGION_LAYOUT_RECONFIGURATION_AVAILABLE) ==
                             0 /*checking the flag is NOT set*/))
       // Only re-create if there is NO SUPPORT for reconfiguring gop on the fly
       || (gopChanged && ((pD3D12Enc->m_currentEncodeCapabilities.m_SupportFlags &
                           D3D12_VIDEO_ENCODER_SUPPORT_FLAG_SEQUENCE_GOP_RECONFIGURATION_AVAILABLE) ==
                          0 /*checking the flag is NOT set*/))
       // || motionPrecisionLimitChanged // Only affects encoder
   ) {
      if (!pD3D12Enc->m_spVideoEncoderHeap) {
         debug_printf("[d3d12_video_encoder] d3d12_video_encoder_reconfigure_encoder_objects - Creating "
                       "D3D12VideoEncoderHeap for the first time\n");
      } else {
         debug_printf("[d3d12_video_encoder] Reconfiguration triggered -> Re-creating D3D12VideoEncoderHeap\n");
         reCreatedEncoderHeap = true;
      }

      D3D12_VIDEO_ENCODER_HEAP_DESC heapDesc = { pD3D12Enc->m_NodeMask,
                                                 D3D12_VIDEO_ENCODER_HEAP_FLAG_NONE,
                                                 pD3D12Enc->m_currentEncodeConfig.m_encoderCodecDesc,
                                                 d3d12_video_encoder_get_current_profile_desc(pD3D12Enc),
                                                 d3d12_video_encoder_get_current_level_desc(pD3D12Enc),
                                                 // resolution list count
                                                 1,
                                                 // resolution list
                                                 &pD3D12Enc->m_currentEncodeConfig.m_currentResolution };

      // Create encoder heap
      pD3D12Enc->m_spVideoEncoderHeap.Reset();
      HRESULT hr = pD3D12Enc->m_spD3D12VideoDevice->CreateVideoEncoderHeap(&heapDesc,
                                                                           IID_PPV_ARGS(pD3D12Enc->m_spVideoEncoderHeap.GetAddressOf()));
      if (FAILED(hr)) {
         debug_printf("CreateVideoEncoderHeap failed with HR %x\n", hr);
         return false;
      }
   }

   // If on-the-fly reconfiguration happened without object recreation, set
   // D3D12_VIDEO_ENCODER_SEQUENCE_CONTROL_FLAG_*_CHANGED reconfiguration flags in EncodeFrame
   if (rateControlChanged &&
       ((pD3D12Enc->m_currentEncodeCapabilities.m_SupportFlags &
         D3D12_VIDEO_ENCODER_SUPPORT_FLAG_RATE_CONTROL_RECONFIGURATION_AVAILABLE) !=
        0 /*checking if the flag it's actually set*/) &&
       (pD3D12Enc->m_fenceValue > 1) && (!reCreatedEncoder || !reCreatedEncoderHeap)) {
      pD3D12Enc->m_currentEncodeConfig.m_seqFlags |= D3D12_VIDEO_ENCODER_SEQUENCE_CONTROL_FLAG_RATE_CONTROL_CHANGE;
   }

   if (slicesChanged &&
       ((pD3D12Enc->m_currentEncodeCapabilities.m_SupportFlags &
         D3D12_VIDEO_ENCODER_SUPPORT_FLAG_SUBREGION_LAYOUT_RECONFIGURATION_AVAILABLE) !=
        0 /*checking if the flag it's actually set*/) &&
       (pD3D12Enc->m_fenceValue > 1) && (!reCreatedEncoder || !reCreatedEncoderHeap)) {
      pD3D12Enc->m_currentEncodeConfig.m_seqFlags |= D3D12_VIDEO_ENCODER_SEQUENCE_CONTROL_FLAG_SUBREGION_LAYOUT_CHANGE;
   }

   if (gopChanged &&
       ((pD3D12Enc->m_currentEncodeCapabilities.m_SupportFlags &
         D3D12_VIDEO_ENCODER_SUPPORT_FLAG_SEQUENCE_GOP_RECONFIGURATION_AVAILABLE) !=
        0 /*checking if the flag it's actually set*/) &&
       (pD3D12Enc->m_fenceValue > 1) && (!reCreatedEncoder || !reCreatedEncoderHeap)) {
      pD3D12Enc->m_currentEncodeConfig.m_seqFlags |= D3D12_VIDEO_ENCODER_SEQUENCE_CONTROL_FLAG_GOP_SEQUENCE_CHANGE;
   }

   if(irChanged)
      pD3D12Enc->m_currentEncodeConfig.m_seqFlags |= D3D12_VIDEO_ENCODER_SEQUENCE_CONTROL_FLAG_REQUEST_INTRA_REFRESH;

   return true;
}

void
d3d12_video_encoder_create_reference_picture_manager(struct d3d12_video_encoder *pD3D12Enc, struct pipe_picture_desc *  picture)
{
   pD3D12Enc->m_upDPBManager.reset();
   pD3D12Enc->m_upBitstreamBuilder.reset();
   enum pipe_video_format codec = u_reduce_video_profile(pD3D12Enc->base.profile);
   switch (codec) {
#if VIDEO_CODEC_H264ENC
      case PIPE_VIDEO_FORMAT_MPEG4_AVC:
      {
         bool gopHasPFrames =
            (pD3D12Enc->m_currentEncodeConfig.m_encoderGOPConfigDesc.m_H264GroupOfPictures.PPicturePeriod > 0) &&
            ((pD3D12Enc->m_currentEncodeConfig.m_encoderGOPConfigDesc.m_H264GroupOfPictures.GOPLength == 0) ||
             (pD3D12Enc->m_currentEncodeConfig.m_encoderGOPConfigDesc.m_H264GroupOfPictures.PPicturePeriod <
              pD3D12Enc->m_currentEncodeConfig.m_encoderGOPConfigDesc.m_H264GroupOfPictures.GOPLength));

         pD3D12Enc->m_upDPBManager = std::make_unique<d3d12_video_encoder_references_manager_h264>(
            gopHasPFrames,
            *pD3D12Enc->m_upDPBStorageManager,
            // Max number of frames to be used as a reference, without counting the current recon picture
            d3d12_video_encoder_get_current_max_dpb_capacity(pD3D12Enc)
         );

         struct pipe_h264_enc_picture_desc *pH264Pic = (struct pipe_h264_enc_picture_desc *) picture;
         pD3D12Enc->m_upBitstreamBuilder = std::make_unique<d3d12_video_bitstream_builder_h264>(pH264Pic->insert_aud_nalu);
      } break;
#endif
#if VIDEO_CODEC_H265ENC
      case PIPE_VIDEO_FORMAT_HEVC:
      {
         bool gopHasPFrames =
            (pD3D12Enc->m_currentEncodeConfig.m_encoderGOPConfigDesc.m_HEVCGroupOfPictures.PPicturePeriod > 0) &&
            ((pD3D12Enc->m_currentEncodeConfig.m_encoderGOPConfigDesc.m_HEVCGroupOfPictures.GOPLength == 0) ||
             (pD3D12Enc->m_currentEncodeConfig.m_encoderGOPConfigDesc.m_HEVCGroupOfPictures.PPicturePeriod <
              pD3D12Enc->m_currentEncodeConfig.m_encoderGOPConfigDesc.m_HEVCGroupOfPictures.GOPLength));

         pD3D12Enc->m_upDPBManager = std::make_unique<d3d12_video_encoder_references_manager_hevc>(
            gopHasPFrames,
            *pD3D12Enc->m_upDPBStorageManager,
            // Max number of frames to be used as a reference, without counting the current recon picture
            d3d12_video_encoder_get_current_max_dpb_capacity(pD3D12Enc)
         );

         pD3D12Enc->m_upBitstreamBuilder = std::make_unique<d3d12_video_bitstream_builder_hevc>();
      } break;
#endif
#if VIDEO_CODEC_AV1ENC
      case PIPE_VIDEO_FORMAT_AV1:
      {
         bool hasInterFrames =
            (pD3D12Enc->m_currentEncodeConfig.m_encoderGOPConfigDesc.m_AV1SequenceStructure.InterFramePeriod > 0) &&
            ((pD3D12Enc->m_currentEncodeConfig.m_encoderGOPConfigDesc.m_AV1SequenceStructure.IntraDistance == 0) ||
             (pD3D12Enc->m_currentEncodeConfig.m_encoderGOPConfigDesc.m_AV1SequenceStructure.InterFramePeriod <
              pD3D12Enc->m_currentEncodeConfig.m_encoderGOPConfigDesc.m_AV1SequenceStructure.IntraDistance));

         pD3D12Enc->m_upDPBManager = std::make_unique<d3d12_video_encoder_references_manager_av1>(
            hasInterFrames,
            *pD3D12Enc->m_upDPBStorageManager
         );

         // We use packed headers and pist encode execution syntax for AV1
         pD3D12Enc->m_upBitstreamBuilder = std::make_unique<d3d12_video_bitstream_builder_av1>();
      } break;
#endif
      default:
      {
         unreachable("Unsupported pipe_video_format");
      } break;
   }
}

D3D12_VIDEO_ENCODER_PICTURE_CONTROL_SUBREGIONS_LAYOUT_DATA
d3d12_video_encoder_get_current_slice_param_settings(struct d3d12_video_encoder *pD3D12Enc)
{
   enum pipe_video_format codec = u_reduce_video_profile(pD3D12Enc->base.profile);
   switch (codec) {
#if VIDEO_CODEC_H264ENC
      case PIPE_VIDEO_FORMAT_MPEG4_AVC:
      {
         D3D12_VIDEO_ENCODER_PICTURE_CONTROL_SUBREGIONS_LAYOUT_DATA subregionData = {};
         if (pD3D12Enc->m_currentEncodeConfig.m_encoderSliceConfigMode !=
             D3D12_VIDEO_ENCODER_FRAME_SUBREGION_LAYOUT_MODE_FULL_FRAME) {
            subregionData.pSlicesPartition_H264 =
               &pD3D12Enc->m_currentEncodeConfig.m_encoderSliceConfigDesc.m_SlicesPartition_H264;
            subregionData.DataSize = sizeof(D3D12_VIDEO_ENCODER_PICTURE_CONTROL_SUBREGIONS_LAYOUT_DATA_SLICES);
         }
         return subregionData;
      } break;
#endif
#if VIDEO_CODEC_H265ENC
      case PIPE_VIDEO_FORMAT_HEVC:
      {
         D3D12_VIDEO_ENCODER_PICTURE_CONTROL_SUBREGIONS_LAYOUT_DATA subregionData = {};
         if (pD3D12Enc->m_currentEncodeConfig.m_encoderSliceConfigMode !=
             D3D12_VIDEO_ENCODER_FRAME_SUBREGION_LAYOUT_MODE_FULL_FRAME) {
            subregionData.pSlicesPartition_HEVC =
               &pD3D12Enc->m_currentEncodeConfig.m_encoderSliceConfigDesc.m_SlicesPartition_HEVC;
            subregionData.DataSize = sizeof(D3D12_VIDEO_ENCODER_PICTURE_CONTROL_SUBREGIONS_LAYOUT_DATA_SLICES);
         }
         return subregionData;
      } break;
#endif
#if VIDEO_CODEC_AV1ENC
      case PIPE_VIDEO_FORMAT_AV1:
      {
         D3D12_VIDEO_ENCODER_PICTURE_CONTROL_SUBREGIONS_LAYOUT_DATA subregionData = {};
         if (pD3D12Enc->m_currentEncodeConfig.m_encoderSliceConfigMode !=
             D3D12_VIDEO_ENCODER_FRAME_SUBREGION_LAYOUT_MODE_FULL_FRAME) {
            subregionData.pTilesPartition_AV1 =
               &pD3D12Enc->m_currentEncodeConfig.m_encoderSliceConfigDesc.m_TilesConfig_AV1.TilesPartition;
            subregionData.DataSize = sizeof(D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_SUBREGIONS_LAYOUT_DATA_TILES);
         }
         return subregionData;
      } break;
#endif
      default:
      {
         unreachable("Unsupported pipe_video_format");
      } break;
   }
}

D3D12_VIDEO_ENCODER_PICTURE_CONTROL_CODEC_DATA
d3d12_video_encoder_get_current_picture_param_settings(struct d3d12_video_encoder *pD3D12Enc)
{
   enum pipe_video_format codec = u_reduce_video_profile(pD3D12Enc->base.profile);
   switch (codec) {
#if VIDEO_CODEC_H264ENC
      case PIPE_VIDEO_FORMAT_MPEG4_AVC:
      {
         D3D12_VIDEO_ENCODER_PICTURE_CONTROL_CODEC_DATA curPicParamsData = {};
         curPicParamsData.pH264PicData = &pD3D12Enc->m_currentEncodeConfig.m_encoderPicParamsDesc.m_H264PicData;
         curPicParamsData.DataSize     = sizeof(pD3D12Enc->m_currentEncodeConfig.m_encoderPicParamsDesc.m_H264PicData);
         return curPicParamsData;
      } break;
#endif
#if VIDEO_CODEC_H265ENC
      case PIPE_VIDEO_FORMAT_HEVC:
      {
         D3D12_VIDEO_ENCODER_PICTURE_CONTROL_CODEC_DATA curPicParamsData = {};
         curPicParamsData.pHEVCPicData = &pD3D12Enc->m_currentEncodeConfig.m_encoderPicParamsDesc.m_HEVCPicData;
         curPicParamsData.DataSize     = sizeof(pD3D12Enc->m_currentEncodeConfig.m_encoderPicParamsDesc.m_HEVCPicData);
         return curPicParamsData;
      } break;
#endif
#if VIDEO_CODEC_AV1ENC
      case PIPE_VIDEO_FORMAT_AV1:
      {
         D3D12_VIDEO_ENCODER_PICTURE_CONTROL_CODEC_DATA curPicParamsData = {};
         curPicParamsData.pAV1PicData = &pD3D12Enc->m_currentEncodeConfig.m_encoderPicParamsDesc.m_AV1PicData;
         curPicParamsData.DataSize     = sizeof(pD3D12Enc->m_currentEncodeConfig.m_encoderPicParamsDesc.m_AV1PicData);
         return curPicParamsData;
      } break;
#endif
      default:
      {
         unreachable("Unsupported pipe_video_format");
      } break;
   }
}

D3D12_VIDEO_ENCODER_RATE_CONTROL
d3d12_video_encoder_get_current_rate_control_settings(struct d3d12_video_encoder *pD3D12Enc)
{
   D3D12_VIDEO_ENCODER_RATE_CONTROL curRateControlDesc = {};
   curRateControlDesc.Mode            = pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Mode;
   curRateControlDesc.Flags           = pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Flags;
   curRateControlDesc.TargetFrameRate = pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_FrameRate;

   if ((curRateControlDesc.Flags & D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_EXTENSION1_SUPPORT) != 0)
   {
      switch (pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Mode) {
         case D3D12_VIDEO_ENCODER_RATE_CONTROL_MODE_ABSOLUTE_QP_MAP:
         {
            curRateControlDesc.ConfigParams.pConfiguration_CQP1 = nullptr;
            curRateControlDesc.ConfigParams.DataSize           = 0;
         } break;
         case D3D12_VIDEO_ENCODER_RATE_CONTROL_MODE_CQP:
         {
            curRateControlDesc.ConfigParams.pConfiguration_CQP1 =
               &pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_CQP1;
            curRateControlDesc.ConfigParams.DataSize =
               sizeof(pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_CQP1);
         } break;
         case D3D12_VIDEO_ENCODER_RATE_CONTROL_MODE_CBR:
         {
            curRateControlDesc.ConfigParams.pConfiguration_CBR1 =
               &pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_CBR1;
            curRateControlDesc.ConfigParams.DataSize =
               sizeof(pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_CBR1);
         } break;
         case D3D12_VIDEO_ENCODER_RATE_CONTROL_MODE_VBR:
         {
            curRateControlDesc.ConfigParams.pConfiguration_VBR1 =
               &pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_VBR1;
            curRateControlDesc.ConfigParams.DataSize =
               sizeof(pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_VBR1);
         } break;
         case D3D12_VIDEO_ENCODER_RATE_CONTROL_MODE_QVBR:
         {
            curRateControlDesc.ConfigParams.pConfiguration_QVBR1 =
               &pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_QVBR1;
            curRateControlDesc.ConfigParams.DataSize =
               sizeof(pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_QVBR1);
         } break;
         default:
         {
            unreachable("Unsupported D3D12_VIDEO_ENCODER_RATE_CONTROL_MODE");
         } break;
      }
   }
   else 
   {
      switch (pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Mode) {
         case D3D12_VIDEO_ENCODER_RATE_CONTROL_MODE_ABSOLUTE_QP_MAP:
         {
            curRateControlDesc.ConfigParams.pConfiguration_CQP = nullptr;
            curRateControlDesc.ConfigParams.DataSize           = 0;
         } break;
         case D3D12_VIDEO_ENCODER_RATE_CONTROL_MODE_CQP:
         {
            curRateControlDesc.ConfigParams.pConfiguration_CQP =
               &pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_CQP;
            curRateControlDesc.ConfigParams.DataSize =
               sizeof(pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_CQP);
         } break;
         case D3D12_VIDEO_ENCODER_RATE_CONTROL_MODE_CBR:
         {
            curRateControlDesc.ConfigParams.pConfiguration_CBR =
               &pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_CBR;
            curRateControlDesc.ConfigParams.DataSize =
               sizeof(pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_CBR);
         } break;
         case D3D12_VIDEO_ENCODER_RATE_CONTROL_MODE_VBR:
         {
            curRateControlDesc.ConfigParams.pConfiguration_VBR =
               &pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_VBR;
            curRateControlDesc.ConfigParams.DataSize =
               sizeof(pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_VBR);
         } break;
         case D3D12_VIDEO_ENCODER_RATE_CONTROL_MODE_QVBR:
         {
            curRateControlDesc.ConfigParams.pConfiguration_QVBR =
               &pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_QVBR;
            curRateControlDesc.ConfigParams.DataSize =
               sizeof(pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_QVBR);
         } break;
         default:
         {
            unreachable("Unsupported D3D12_VIDEO_ENCODER_RATE_CONTROL_MODE");
         } break;
      }
   }

   return curRateControlDesc;
}

D3D12_VIDEO_ENCODER_LEVEL_SETTING
d3d12_video_encoder_get_current_level_desc(struct d3d12_video_encoder *pD3D12Enc)
{
   enum pipe_video_format codec = u_reduce_video_profile(pD3D12Enc->base.profile);
   switch (codec) {
#if VIDEO_CODEC_H264ENC
      case PIPE_VIDEO_FORMAT_MPEG4_AVC:
      {
         D3D12_VIDEO_ENCODER_LEVEL_SETTING curLevelDesc = {};
         curLevelDesc.pH264LevelSetting = &pD3D12Enc->m_currentEncodeConfig.m_encoderLevelDesc.m_H264LevelSetting;
         curLevelDesc.DataSize = sizeof(pD3D12Enc->m_currentEncodeConfig.m_encoderLevelDesc.m_H264LevelSetting);
         return curLevelDesc;
      } break;
#endif
#if VIDEO_CODEC_H265ENC
      case PIPE_VIDEO_FORMAT_HEVC:
      {
         D3D12_VIDEO_ENCODER_LEVEL_SETTING curLevelDesc = {};
         curLevelDesc.pHEVCLevelSetting = &pD3D12Enc->m_currentEncodeConfig.m_encoderLevelDesc.m_HEVCLevelSetting;
         curLevelDesc.DataSize = sizeof(pD3D12Enc->m_currentEncodeConfig.m_encoderLevelDesc.m_HEVCLevelSetting);
         return curLevelDesc;
      } break;
#endif
#if VIDEO_CODEC_AV1ENC
      case PIPE_VIDEO_FORMAT_AV1:
      {
         D3D12_VIDEO_ENCODER_LEVEL_SETTING curLevelDesc = {};
         curLevelDesc.pAV1LevelSetting = &pD3D12Enc->m_currentEncodeConfig.m_encoderLevelDesc.m_AV1LevelSetting;
         curLevelDesc.DataSize = sizeof(pD3D12Enc->m_currentEncodeConfig.m_encoderLevelDesc.m_AV1LevelSetting);
         return curLevelDesc;
      } break;
#endif
      default:
      {
         unreachable("Unsupported pipe_video_format");
      } break;
   }
}

void
d3d12_video_encoder_build_pre_encode_codec_headers(struct d3d12_video_encoder *pD3D12Enc,
                                                   bool &postEncodeHeadersNeeded,
                                                   uint64_t &preEncodeGeneratedHeadersByteSize,
                                                   std::vector<uint64_t> &pWrittenCodecUnitsSizes)
{
   enum pipe_video_format codec = u_reduce_video_profile(pD3D12Enc->base.profile);
   switch (codec) {
#if VIDEO_CODEC_H264ENC
      case PIPE_VIDEO_FORMAT_MPEG4_AVC:
      {
         postEncodeHeadersNeeded = false;
         preEncodeGeneratedHeadersByteSize = d3d12_video_encoder_build_codec_headers_h264(pD3D12Enc, pWrittenCodecUnitsSizes);
      } break;
#endif
#if VIDEO_CODEC_H265ENC
      case PIPE_VIDEO_FORMAT_HEVC:
      {
         postEncodeHeadersNeeded = false;
         preEncodeGeneratedHeadersByteSize = d3d12_video_encoder_build_codec_headers_hevc(pD3D12Enc, pWrittenCodecUnitsSizes);
      } break;
#endif
#if VIDEO_CODEC_AV1ENC
      case PIPE_VIDEO_FORMAT_AV1:
      { 
         pD3D12Enc->m_BitstreamHeadersBuffer.resize(0);
         postEncodeHeadersNeeded = true;
         preEncodeGeneratedHeadersByteSize = 0;
         pWrittenCodecUnitsSizes.clear();
      } break;
#endif
      default:
      {
         unreachable("Unsupported pipe_video_format");
      } break;
   }
}

D3D12_VIDEO_ENCODER_SEQUENCE_GOP_STRUCTURE
d3d12_video_encoder_get_current_gop_desc(struct d3d12_video_encoder *pD3D12Enc)
{
   enum pipe_video_format codec = u_reduce_video_profile(pD3D12Enc->base.profile);
   switch (codec) {
#if VIDEO_CODEC_H264ENC
      case PIPE_VIDEO_FORMAT_MPEG4_AVC:
      {
         D3D12_VIDEO_ENCODER_SEQUENCE_GOP_STRUCTURE curGOPDesc = {};
         curGOPDesc.pH264GroupOfPictures =
            &pD3D12Enc->m_currentEncodeConfig.m_encoderGOPConfigDesc.m_H264GroupOfPictures;
         curGOPDesc.DataSize = sizeof(pD3D12Enc->m_currentEncodeConfig.m_encoderGOPConfigDesc.m_H264GroupOfPictures);
         return curGOPDesc;
      } break;
#endif
#if VIDEO_CODEC_H265ENC
      case PIPE_VIDEO_FORMAT_HEVC:
      {
         D3D12_VIDEO_ENCODER_SEQUENCE_GOP_STRUCTURE curGOPDesc = {};
         curGOPDesc.pHEVCGroupOfPictures =
            &pD3D12Enc->m_currentEncodeConfig.m_encoderGOPConfigDesc.m_HEVCGroupOfPictures;
         curGOPDesc.DataSize = sizeof(pD3D12Enc->m_currentEncodeConfig.m_encoderGOPConfigDesc.m_HEVCGroupOfPictures);
         return curGOPDesc;
      } break;
#endif
#if VIDEO_CODEC_AV1ENC
      case PIPE_VIDEO_FORMAT_AV1:
      {
         D3D12_VIDEO_ENCODER_SEQUENCE_GOP_STRUCTURE curGOPDesc = {};
         curGOPDesc.pAV1SequenceStructure =
            &pD3D12Enc->m_currentEncodeConfig.m_encoderGOPConfigDesc.m_AV1SequenceStructure;
         curGOPDesc.DataSize = sizeof(pD3D12Enc->m_currentEncodeConfig.m_encoderGOPConfigDesc.m_AV1SequenceStructure);
         return curGOPDesc;
      } break;
#endif
      default:
      {
         unreachable("Unsupported pipe_video_format");
      } break;
   }
}

D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION
d3d12_video_encoder_get_current_codec_config_desc(struct d3d12_video_encoder *pD3D12Enc)
{
   enum pipe_video_format codec = u_reduce_video_profile(pD3D12Enc->base.profile);
   switch (codec) {
#if VIDEO_CODEC_H264ENC
      case PIPE_VIDEO_FORMAT_MPEG4_AVC:
      {
         D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION codecConfigDesc = {};
         codecConfigDesc.pH264Config = &pD3D12Enc->m_currentEncodeConfig.m_encoderCodecSpecificConfigDesc.m_H264Config;
         codecConfigDesc.DataSize =
            sizeof(pD3D12Enc->m_currentEncodeConfig.m_encoderCodecSpecificConfigDesc.m_H264Config);
         return codecConfigDesc;
      } break;
#endif
#if VIDEO_CODEC_H265ENC
      case PIPE_VIDEO_FORMAT_HEVC:
      {
         D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION codecConfigDesc = {};
         codecConfigDesc.pHEVCConfig = &pD3D12Enc->m_currentEncodeConfig.m_encoderCodecSpecificConfigDesc.m_HEVCConfig;
         codecConfigDesc.DataSize =
            sizeof(pD3D12Enc->m_currentEncodeConfig.m_encoderCodecSpecificConfigDesc.m_HEVCConfig);
         return codecConfigDesc;
      } break;
#endif
#if VIDEO_CODEC_AV1ENC
      case PIPE_VIDEO_FORMAT_AV1:
      {
         D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION codecConfigDesc = {};
         codecConfigDesc.pAV1Config = &pD3D12Enc->m_currentEncodeConfig.m_encoderCodecSpecificConfigDesc.m_AV1Config;
         codecConfigDesc.DataSize =
            sizeof(pD3D12Enc->m_currentEncodeConfig.m_encoderCodecSpecificConfigDesc.m_AV1Config);
         return codecConfigDesc;
      } break;
#endif
      default:
      {
         unreachable("Unsupported pipe_video_format");
      } break;
   }
}

D3D12_VIDEO_ENCODER_CODEC
d3d12_video_encoder_get_current_codec(struct d3d12_video_encoder *pD3D12Enc)
{
   enum pipe_video_format codec = u_reduce_video_profile(pD3D12Enc->base.profile);
   switch (codec) {
#if VIDEO_CODEC_H264ENC
      case PIPE_VIDEO_FORMAT_MPEG4_AVC:
      {
         return D3D12_VIDEO_ENCODER_CODEC_H264;
      } break;
#endif
#if VIDEO_CODEC_H265ENC
      case PIPE_VIDEO_FORMAT_HEVC:
      {
         return D3D12_VIDEO_ENCODER_CODEC_HEVC;
      } break;
#endif
#if VIDEO_CODEC_AV1ENC
      case PIPE_VIDEO_FORMAT_AV1:
      {
         return D3D12_VIDEO_ENCODER_CODEC_AV1;
      } break;
#endif
      default:
      {
         unreachable("Unsupported pipe_video_format");
      } break;
   }
}

static void
d3d12_video_encoder_disable_rc_vbv_sizes(struct D3D12EncodeRateControlState & rcState)
{
   rcState.m_Flags &= ~D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_VBV_SIZES;
   switch (rcState.m_Mode) {
      case D3D12_VIDEO_ENCODER_RATE_CONTROL_MODE_CBR:
      {
         rcState.m_Config.m_Configuration_CBR.VBVCapacity = 0;
         rcState.m_Config.m_Configuration_CBR.InitialVBVFullness = 0;
      } break;
      case D3D12_VIDEO_ENCODER_RATE_CONTROL_MODE_VBR:
      {
         rcState.m_Config.m_Configuration_VBR.VBVCapacity = 0;
         rcState.m_Config.m_Configuration_VBR.InitialVBVFullness = 0;
      } break;
      case D3D12_VIDEO_ENCODER_RATE_CONTROL_MODE_QVBR:
      {
         rcState.m_Config.m_Configuration_QVBR1.VBVCapacity = 0;
         rcState.m_Config.m_Configuration_QVBR1.InitialVBVFullness = 0;
      } break;
      default:
      {
         unreachable("Unsupported D3D12_VIDEO_ENCODER_RATE_CONTROL_MODE for VBV Sizes");
      } break;
   }
}

static void
d3d12_video_encoder_disable_rc_maxframesize(struct D3D12EncodeRateControlState & rcState)
{
   rcState.m_Flags &= ~D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_MAX_FRAME_SIZE;
   rcState.max_frame_size = 0;
   switch (rcState.m_Mode) {
      case D3D12_VIDEO_ENCODER_RATE_CONTROL_MODE_CBR:
      {
         rcState.m_Config.m_Configuration_CBR.MaxFrameBitSize = 0;
      } break;
      case D3D12_VIDEO_ENCODER_RATE_CONTROL_MODE_VBR:
      {
         rcState.m_Config.m_Configuration_VBR.MaxFrameBitSize = 0;
      } break;
      case D3D12_VIDEO_ENCODER_RATE_CONTROL_MODE_QVBR:
      {
         rcState.m_Config.m_Configuration_QVBR.MaxFrameBitSize = 0;
      } break;
      default:
      {
         unreachable("Unsupported D3D12_VIDEO_ENCODER_RATE_CONTROL_MODE for VBV Sizes");
      } break;
   }
}

static bool
d3d12_video_encoder_is_qualitylevel_in_range(struct D3D12EncodeRateControlState & rcState, UINT MaxQualityVsSpeed)
{
   switch (rcState.m_Mode) {
      case D3D12_VIDEO_ENCODER_RATE_CONTROL_MODE_CQP:
      {
         return rcState.m_Config.m_Configuration_CQP1.QualityVsSpeed <= MaxQualityVsSpeed;
      } break;
      case D3D12_VIDEO_ENCODER_RATE_CONTROL_MODE_CBR:
      {
         return rcState.m_Config.m_Configuration_CBR1.QualityVsSpeed <= MaxQualityVsSpeed;
      } break;
      case D3D12_VIDEO_ENCODER_RATE_CONTROL_MODE_VBR:
      {
         return rcState.m_Config.m_Configuration_VBR1.QualityVsSpeed <= MaxQualityVsSpeed;
      } break;
      case D3D12_VIDEO_ENCODER_RATE_CONTROL_MODE_QVBR:
      {
         return rcState.m_Config.m_Configuration_QVBR1.QualityVsSpeed <= MaxQualityVsSpeed;
      } break;
      default:
      {
         unreachable("Unsupported D3D12_VIDEO_ENCODER_RATE_CONTROL_MODE");
      } break;
   }
}

static void
d3d12_video_encoder_disable_rc_qualitylevels(struct D3D12EncodeRateControlState & rcState)
{
   rcState.m_Flags &= ~D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_QUALITY_VS_SPEED;
   switch (rcState.m_Mode) {
      case D3D12_VIDEO_ENCODER_RATE_CONTROL_MODE_CBR:
      {
         rcState.m_Config.m_Configuration_CBR1.QualityVsSpeed = 0;
      } break;
      case D3D12_VIDEO_ENCODER_RATE_CONTROL_MODE_VBR:
      {
         rcState.m_Config.m_Configuration_VBR1.QualityVsSpeed = 0;
      } break;
      case D3D12_VIDEO_ENCODER_RATE_CONTROL_MODE_QVBR:
      {
         rcState.m_Config.m_Configuration_QVBR1.QualityVsSpeed = 0;
      } break;
      default:
      {
         unreachable("Unsupported D3D12_VIDEO_ENCODER_RATE_CONTROL_MODE");
      } break;
   }
}

static void
d3d12_video_encoder_disable_rc_deltaqp(struct D3D12EncodeRateControlState & rcState)
{
   rcState.m_Flags &= ~D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_DELTA_QP;
}

static void
d3d12_video_encoder_disable_rc_minmaxqp(struct D3D12EncodeRateControlState & rcState)
{
   rcState.m_Flags &= ~D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_QP_RANGE;
   switch (rcState.m_Mode) {
      case D3D12_VIDEO_ENCODER_RATE_CONTROL_MODE_CBR:
      {
         rcState.m_Config.m_Configuration_CBR.MinQP = 0;
         rcState.m_Config.m_Configuration_CBR.MaxQP = 0;
      } break;
      case D3D12_VIDEO_ENCODER_RATE_CONTROL_MODE_VBR:
      {
         rcState.m_Config.m_Configuration_VBR.MinQP = 0;
         rcState.m_Config.m_Configuration_VBR.MaxQP = 0;
      } break;
      case D3D12_VIDEO_ENCODER_RATE_CONTROL_MODE_QVBR:
      {
         rcState.m_Config.m_Configuration_QVBR.MinQP = 0;
         rcState.m_Config.m_Configuration_QVBR.MaxQP = 0;
      } break;
      default:
      {
         unreachable("Unsupported D3D12_VIDEO_ENCODER_RATE_CONTROL_MODE for VBV Sizes");
      } break;
   }
}

static void
d3d12_video_encoder_disable_rc_extended1_to_legacy(struct D3D12EncodeRateControlState & rcState)
{
   rcState.m_Flags &= ~D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_EXTENSION1_SUPPORT;
   // Also remove features that require extension1 enabled (eg. quality levels)
   rcState.m_Flags &= ~D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_QUALITY_VS_SPEED;
   // rcState.m_Configuration_XXX and m_Configuration_XXX1 are unions, can be aliased
   // as the m_Configuration_XXX1 extensions are binary backcompat with m_Configuration_XXX
}

///
/// Call d3d12_video_encoder_query_d3d12_driver_caps and see if any optional feature requested
/// is not supported, disable it, query again until finding a negotiated cap/feature set
/// Note that with fallbacks, the upper layer will not get exactly the encoding seetings they requested
/// but for very particular settings it's better to continue with warnings than failing the whole encoding process
///
bool d3d12_video_encoder_negotiate_requested_features_and_d3d12_driver_caps(struct d3d12_video_encoder *pD3D12Enc, D3D12_FEATURE_DATA_VIDEO_ENCODER_SUPPORT1 &capEncoderSupportData1) {

   ///
   /// Check for general support
   /// Check for validation errors (some drivers return general support but also validation errors anyways, work around for those unexpected cases)
   ///

   bool configSupported = d3d12_video_encoder_query_d3d12_driver_caps(pD3D12Enc, /*inout*/ capEncoderSupportData1)
    && (((capEncoderSupportData1.SupportFlags & D3D12_VIDEO_ENCODER_SUPPORT_FLAG_GENERAL_SUPPORT_OK) != 0)
                        && (capEncoderSupportData1.ValidationFlags == D3D12_VIDEO_ENCODER_VALIDATION_FLAG_NONE));

   ///
   /// If D3D12_FEATURE_VIDEO_ENCODER_SUPPORT is not supported, try falling back to unsetting optional features and check for caps again
   ///   

   if (!configSupported) {
      debug_printf("[d3d12_video_encoder] WARNING: D3D12_FEATURE_VIDEO_ENCODER_SUPPORT is not supported, trying fallback to unsetting optional features\n");

      bool isRequestingVBVSizesSupported = ((capEncoderSupportData1.SupportFlags & D3D12_VIDEO_ENCODER_SUPPORT_FLAG_RATE_CONTROL_VBV_SIZE_CONFIG_AVAILABLE) != 0);
      bool isClientRequestingVBVSizes = ((pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Flags & D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_VBV_SIZES) != 0);
      
      if(isClientRequestingVBVSizes && !isRequestingVBVSizesSupported) {
         debug_printf("[d3d12_video_encoder] WARNING: Requested D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_VBV_SIZES with VBVCapacity and InitialVBVFullness is not supported, will continue encoding unsetting this feature as fallback.\n");
         d3d12_video_encoder_disable_rc_vbv_sizes(pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc);
      }

      bool isRequestingPeakFrameSizeSupported = ((capEncoderSupportData1.SupportFlags & D3D12_VIDEO_ENCODER_SUPPORT_FLAG_RATE_CONTROL_MAX_FRAME_SIZE_AVAILABLE) != 0);
      bool isClientRequestingPeakFrameSize = ((pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Flags & D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_MAX_FRAME_SIZE) != 0);

      if(isClientRequestingPeakFrameSize && !isRequestingPeakFrameSizeSupported) {
         debug_printf("[d3d12_video_encoder] WARNING: Requested D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_MAX_FRAME_SIZE with MaxFrameBitSize but the feature is not supported, will continue encoding unsetting this feature as fallback.\n");
         d3d12_video_encoder_disable_rc_maxframesize(pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc);
      }

      bool isRequestingQPRangesSupported = ((capEncoderSupportData1.SupportFlags & D3D12_VIDEO_ENCODER_SUPPORT_FLAG_RATE_CONTROL_ADJUSTABLE_QP_RANGE_AVAILABLE) != 0);
      bool isClientRequestingQPRanges = ((pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Flags & D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_QP_RANGE) != 0);

      if(isClientRequestingQPRanges && !isRequestingQPRangesSupported) {
         debug_printf("[d3d12_video_encoder] WARNING: Requested D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_QP_RANGE with QPMin QPMax but the feature is not supported, will continue encoding unsetting this feature as fallback.\n");
         d3d12_video_encoder_disable_rc_minmaxqp(pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc);
      }

      bool isRequestingDeltaQPSupported = ((capEncoderSupportData1.SupportFlags & D3D12_VIDEO_ENCODER_SUPPORT_FLAG_RATE_CONTROL_DELTA_QP_AVAILABLE) != 0);
      bool isClientRequestingDeltaQP = ((pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Flags & D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_DELTA_QP) != 0);

      if(isClientRequestingDeltaQP && !isRequestingDeltaQPSupported) {
         debug_printf("[d3d12_video_encoder] WARNING: Requested D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_DELTA_QP but the feature is not supported, will continue encoding unsetting this feature as fallback.\n");
         d3d12_video_encoder_disable_rc_deltaqp(pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc);
      }

      bool isRequestingExtended1RCSupported = ((capEncoderSupportData1.SupportFlags & D3D12_VIDEO_ENCODER_SUPPORT_FLAG_RATE_CONTROL_EXTENSION1_SUPPORT) != 0);
      bool isClientRequestingExtended1RC = ((pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Flags & D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_EXTENSION1_SUPPORT) != 0);

      if(isClientRequestingExtended1RC && !isRequestingExtended1RCSupported) {
         debug_printf("[d3d12_video_encoder] WARNING: Requested D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_EXTENSION1_SUPPORT but the feature is not supported, will continue encoding unsetting this feature and dependent features as fallback.\n");
         d3d12_video_encoder_disable_rc_extended1_to_legacy(pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc);
      }

      /* d3d12_video_encoder_disable_rc_extended1_to_legacy may change m_Flags */
      if ((pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Flags & D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_EXTENSION1_SUPPORT) != 0)
      { // Quality levels also requires D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_EXTENSION1_SUPPORT
         bool isRequestingQualityLevelsSupported = ((capEncoderSupportData1.SupportFlags & D3D12_VIDEO_ENCODER_SUPPORT_FLAG_RATE_CONTROL_QUALITY_VS_SPEED_AVAILABLE) != 0);
         bool isClientRequestingQualityLevels = ((pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Flags & D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_QUALITY_VS_SPEED) != 0);

         if (isClientRequestingQualityLevels)
         {
            if (!isRequestingQualityLevelsSupported) {
               debug_printf("[d3d12_video_encoder] WARNING: Requested D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_QUALITY_VS_SPEED but the feature is not supported, will continue encoding unsetting this feature as fallback.\n");
               d3d12_video_encoder_disable_rc_qualitylevels(pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc);
            } else if (!d3d12_video_encoder_is_qualitylevel_in_range(pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc, capEncoderSupportData1.MaxQualityVsSpeed)) {
               debug_printf("[d3d12_video_encoder] WARNING: Requested D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_QUALITY_VS_SPEED but the value is out of supported range, will continue encoding unsetting this feature as fallback.\n");
               d3d12_video_encoder_disable_rc_qualitylevels(pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc);
            }
         }
      }

      ///
      /// Try fallback configuration
      ///
      configSupported = d3d12_video_encoder_query_d3d12_driver_caps(pD3D12Enc, /*inout*/ capEncoderSupportData1)
         && (((capEncoderSupportData1.SupportFlags & D3D12_VIDEO_ENCODER_SUPPORT_FLAG_GENERAL_SUPPORT_OK) != 0)
                        && (capEncoderSupportData1.ValidationFlags == D3D12_VIDEO_ENCODER_VALIDATION_FLAG_NONE));
   }

   if (pD3D12Enc->m_currentEncodeConfig.m_IntraRefresh.IntraRefreshDuration >
      pD3D12Enc->m_currentEncodeCapabilities.m_currentResolutionSupportCaps.MaxIntraRefreshFrameDuration)
   {
      debug_printf("[d3d12_video_encoder] Desired duration of intrarefresh %d is not supported (higher than max "
                  "reported IR duration %d in query caps) for current resolution.\n",
                  pD3D12Enc->m_currentEncodeConfig.m_IntraRefresh.IntraRefreshDuration,
                  pD3D12Enc->m_currentEncodeCapabilities.m_currentResolutionSupportCaps.MaxIntraRefreshFrameDuration);
      return false;
   }

   if(!configSupported) {
      debug_printf("[d3d12_video_encoder] Cap negotiation failed, see more details below:\n");
      
      if ((capEncoderSupportData1.ValidationFlags & D3D12_VIDEO_ENCODER_VALIDATION_FLAG_CODEC_NOT_SUPPORTED) != 0) {
         debug_printf("[d3d12_video_encoder] Requested codec is not supported\n");
      }

      if ((capEncoderSupportData1.ValidationFlags &
         D3D12_VIDEO_ENCODER_VALIDATION_FLAG_RESOLUTION_NOT_SUPPORTED_IN_LIST) != 0) {
         debug_printf("[d3d12_video_encoder] Requested resolution is not supported\n");
      }

      if ((capEncoderSupportData1.ValidationFlags &
         D3D12_VIDEO_ENCODER_VALIDATION_FLAG_RATE_CONTROL_CONFIGURATION_NOT_SUPPORTED) != 0) {
         debug_printf("[d3d12_video_encoder] Requested bitrate or rc config is not supported\n");
      }

      if ((capEncoderSupportData1.ValidationFlags &
         D3D12_VIDEO_ENCODER_VALIDATION_FLAG_CODEC_CONFIGURATION_NOT_SUPPORTED) != 0) {
         debug_printf("[d3d12_video_encoder] Requested codec config is not supported\n");
      }

      if ((capEncoderSupportData1.ValidationFlags &
         D3D12_VIDEO_ENCODER_VALIDATION_FLAG_RATE_CONTROL_MODE_NOT_SUPPORTED) != 0) {
         debug_printf("[d3d12_video_encoder] Requested rate control mode is not supported\n");
      }

      if ((capEncoderSupportData1.ValidationFlags &
         D3D12_VIDEO_ENCODER_VALIDATION_FLAG_INTRA_REFRESH_MODE_NOT_SUPPORTED) != 0) {
         debug_printf("[d3d12_video_encoder] Requested intra refresh config is not supported\n");
      }

      if ((capEncoderSupportData1.ValidationFlags &
         D3D12_VIDEO_ENCODER_VALIDATION_FLAG_SUBREGION_LAYOUT_MODE_NOT_SUPPORTED) != 0) {
         debug_printf("[d3d12_video_encoder] Requested subregion layout mode is not supported\n");
      }

      if ((capEncoderSupportData1.ValidationFlags & D3D12_VIDEO_ENCODER_VALIDATION_FLAG_INPUT_FORMAT_NOT_SUPPORTED) !=
         0) {
         debug_printf("[d3d12_video_encoder] Requested input dxgi format is not supported\n");
      }
   }

   if (memcmp(&pD3D12Enc->m_prevFrameEncodeConfig.m_encoderRateControlDesc,
              &pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc,
              sizeof(pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc)) != 0) {
      pD3D12Enc->m_currentEncodeConfig.m_ConfigDirtyFlags |= d3d12_video_encoder_config_dirty_flag_rate_control;
   }

   return configSupported;
}

bool d3d12_video_encoder_query_d3d12_driver_caps(struct d3d12_video_encoder *pD3D12Enc, D3D12_FEATURE_DATA_VIDEO_ENCODER_SUPPORT1 &capEncoderSupportData1) {
   capEncoderSupportData1.NodeIndex                                = pD3D12Enc->m_NodeIndex;
   capEncoderSupportData1.Codec                                    = d3d12_video_encoder_get_current_codec(pD3D12Enc);
   capEncoderSupportData1.InputFormat            = pD3D12Enc->m_currentEncodeConfig.m_encodeFormatInfo.Format;
   capEncoderSupportData1.RateControl            = d3d12_video_encoder_get_current_rate_control_settings(pD3D12Enc);
   capEncoderSupportData1.IntraRefresh           = pD3D12Enc->m_currentEncodeConfig.m_IntraRefresh.Mode;
   capEncoderSupportData1.SubregionFrameEncoding = pD3D12Enc->m_currentEncodeConfig.m_encoderSliceConfigMode;
   capEncoderSupportData1.ResolutionsListCount   = 1;
   capEncoderSupportData1.pResolutionList        = &pD3D12Enc->m_currentEncodeConfig.m_currentResolution;
   capEncoderSupportData1.CodecGopSequence       = d3d12_video_encoder_get_current_gop_desc(pD3D12Enc);
   capEncoderSupportData1.MaxReferenceFramesInDPB = d3d12_video_encoder_get_current_max_dpb_capacity(pD3D12Enc);
   capEncoderSupportData1.CodecConfiguration = d3d12_video_encoder_get_current_codec_config_desc(pD3D12Enc);

   enum pipe_video_format codec = u_reduce_video_profile(pD3D12Enc->base.profile);
   switch (codec) {
#if VIDEO_CODEC_H264ENC
      case PIPE_VIDEO_FORMAT_MPEG4_AVC:
      {
         capEncoderSupportData1.SuggestedProfile.pH264Profile =
            &pD3D12Enc->m_currentEncodeCapabilities.m_encoderSuggestedProfileDesc.m_H264Profile;
         capEncoderSupportData1.SuggestedProfile.DataSize =
            sizeof(pD3D12Enc->m_currentEncodeCapabilities.m_encoderSuggestedProfileDesc.m_H264Profile);
         capEncoderSupportData1.SuggestedLevel.pH264LevelSetting =
            &pD3D12Enc->m_currentEncodeCapabilities.m_encoderLevelSuggestedDesc.m_H264LevelSetting;
         capEncoderSupportData1.SuggestedLevel.DataSize =
            sizeof(pD3D12Enc->m_currentEncodeCapabilities.m_encoderLevelSuggestedDesc.m_H264LevelSetting);
      } break;
#endif
#if VIDEO_CODEC_H265ENC
      case PIPE_VIDEO_FORMAT_HEVC:
      {
         capEncoderSupportData1.SuggestedProfile.pHEVCProfile =
            &pD3D12Enc->m_currentEncodeCapabilities.m_encoderSuggestedProfileDesc.m_HEVCProfile;
         capEncoderSupportData1.SuggestedProfile.DataSize =
            sizeof(pD3D12Enc->m_currentEncodeCapabilities.m_encoderSuggestedProfileDesc.m_HEVCProfile);
         capEncoderSupportData1.SuggestedLevel.pHEVCLevelSetting =
            &pD3D12Enc->m_currentEncodeCapabilities.m_encoderLevelSuggestedDesc.m_HEVCLevelSetting;
         capEncoderSupportData1.SuggestedLevel.DataSize =
            sizeof(pD3D12Enc->m_currentEncodeCapabilities.m_encoderLevelSuggestedDesc.m_HEVCLevelSetting);
      } break;
#endif
#if VIDEO_CODEC_AV1ENC
      case PIPE_VIDEO_FORMAT_AV1:
      {
         capEncoderSupportData1.SuggestedProfile.pAV1Profile =
            &pD3D12Enc->m_currentEncodeCapabilities.m_encoderSuggestedProfileDesc.m_AV1Profile;
         capEncoderSupportData1.SuggestedProfile.DataSize =
            sizeof(pD3D12Enc->m_currentEncodeCapabilities.m_encoderSuggestedProfileDesc.m_AV1Profile);
         capEncoderSupportData1.SuggestedLevel.pAV1LevelSetting =
            &pD3D12Enc->m_currentEncodeCapabilities.m_encoderLevelSuggestedDesc.m_AV1LevelSetting;
         capEncoderSupportData1.SuggestedLevel.DataSize =
            sizeof(pD3D12Enc->m_currentEncodeCapabilities.m_encoderLevelSuggestedDesc.m_AV1LevelSetting);
      } break;
#endif
      default:
      {
         unreachable("Unsupported pipe_video_format");
      } break;
   }

   // prepare inout storage for the resolution dependent result.
   capEncoderSupportData1.pResolutionDependentSupport =
      &pD3D12Enc->m_currentEncodeCapabilities.m_currentResolutionSupportCaps;
   
   capEncoderSupportData1.SubregionFrameEncodingData = d3d12_video_encoder_get_current_slice_param_settings(pD3D12Enc);
   HRESULT hr = pD3D12Enc->m_spD3D12VideoDevice->CheckFeatureSupport(D3D12_FEATURE_VIDEO_ENCODER_SUPPORT1,
                                                                         &capEncoderSupportData1,
                                                                         sizeof(capEncoderSupportData1));
   if (FAILED(hr)) {
      debug_printf("CheckFeatureSupport D3D12_FEATURE_VIDEO_ENCODER_SUPPORT1 failed with HR %x\n", hr);
      debug_printf("Falling back to check previous query version D3D12_FEATURE_VIDEO_ENCODER_SUPPORT...\n");

      // D3D12_FEATURE_DATA_VIDEO_ENCODER_SUPPORT1 extends D3D12_FEATURE_DATA_VIDEO_ENCODER_SUPPORT
      // in a binary compatible way, so just cast it and try with the older query D3D12_FEATURE_VIDEO_ENCODER_SUPPORT
      D3D12_FEATURE_DATA_VIDEO_ENCODER_SUPPORT * casted_down_cap_data = reinterpret_cast<D3D12_FEATURE_DATA_VIDEO_ENCODER_SUPPORT*>(&capEncoderSupportData1);
      hr = pD3D12Enc->m_spD3D12VideoDevice->CheckFeatureSupport(D3D12_FEATURE_VIDEO_ENCODER_SUPPORT,
                                                                         casted_down_cap_data,
                                                                         sizeof(D3D12_FEATURE_DATA_VIDEO_ENCODER_SUPPORT));
      if (FAILED(hr)) {
         debug_printf("CheckFeatureSupport D3D12_FEATURE_VIDEO_ENCODER_SUPPORT failed with HR %x\n", hr);
         return false;
      }
   }
   pD3D12Enc->m_currentEncodeCapabilities.m_SupportFlags    = capEncoderSupportData1.SupportFlags;
   pD3D12Enc->m_currentEncodeCapabilities.m_ValidationFlags = capEncoderSupportData1.ValidationFlags;
   return true;
}

bool d3d12_video_encoder_check_subregion_mode_support(struct d3d12_video_encoder *pD3D12Enc,
                                    D3D12_VIDEO_ENCODER_FRAME_SUBREGION_LAYOUT_MODE requestedSlicesMode
   )
{
   D3D12_FEATURE_DATA_VIDEO_ENCODER_FRAME_SUBREGION_LAYOUT_MODE capDataSubregionLayout = { }; 
   capDataSubregionLayout.NodeIndex = pD3D12Enc->m_NodeIndex;
   capDataSubregionLayout.Codec = d3d12_video_encoder_get_current_codec(pD3D12Enc);
   capDataSubregionLayout.Profile = d3d12_video_encoder_get_current_profile_desc(pD3D12Enc);
   capDataSubregionLayout.Level = d3d12_video_encoder_get_current_level_desc(pD3D12Enc);
   capDataSubregionLayout.SubregionMode = requestedSlicesMode;
   HRESULT hr = pD3D12Enc->m_spD3D12VideoDevice->CheckFeatureSupport(D3D12_FEATURE_VIDEO_ENCODER_FRAME_SUBREGION_LAYOUT_MODE, &capDataSubregionLayout, sizeof(capDataSubregionLayout));
   if (FAILED(hr)) {
      debug_printf("CheckFeatureSupport failed with HR %x\n", hr);
      return false;
   }
   return capDataSubregionLayout.IsSupported;
}

D3D12_VIDEO_ENCODER_PROFILE_DESC
d3d12_video_encoder_get_current_profile_desc(struct d3d12_video_encoder *pD3D12Enc)
{
   enum pipe_video_format codec = u_reduce_video_profile(pD3D12Enc->base.profile);
   switch (codec) {
#if VIDEO_CODEC_H264ENC
      case PIPE_VIDEO_FORMAT_MPEG4_AVC:
      {
         D3D12_VIDEO_ENCODER_PROFILE_DESC curProfDesc = {};
         curProfDesc.pH264Profile = &pD3D12Enc->m_currentEncodeConfig.m_encoderProfileDesc.m_H264Profile;
         curProfDesc.DataSize     = sizeof(pD3D12Enc->m_currentEncodeConfig.m_encoderProfileDesc.m_H264Profile);
         return curProfDesc;
      } break;
#endif
#if VIDEO_CODEC_H265ENC
      case PIPE_VIDEO_FORMAT_HEVC:
      {
         D3D12_VIDEO_ENCODER_PROFILE_DESC curProfDesc = {};
         curProfDesc.pHEVCProfile = &pD3D12Enc->m_currentEncodeConfig.m_encoderProfileDesc.m_HEVCProfile;
         curProfDesc.DataSize     = sizeof(pD3D12Enc->m_currentEncodeConfig.m_encoderProfileDesc.m_HEVCProfile);
         return curProfDesc;
      } break;
#endif
#if VIDEO_CODEC_AV1ENC
      case PIPE_VIDEO_FORMAT_AV1:
      {
         D3D12_VIDEO_ENCODER_PROFILE_DESC curProfDesc = {};
         curProfDesc.pAV1Profile = &pD3D12Enc->m_currentEncodeConfig.m_encoderProfileDesc.m_AV1Profile;
         curProfDesc.DataSize     = sizeof(pD3D12Enc->m_currentEncodeConfig.m_encoderProfileDesc.m_AV1Profile);
         return curProfDesc;
      } break;
#endif
      default:
      {
         unreachable("Unsupported pipe_video_format");
      } break;
   }
}

uint32_t
d3d12_video_encoder_get_current_max_dpb_capacity(struct d3d12_video_encoder *pD3D12Enc)
{
   return pD3D12Enc->base.max_references;
}

bool
d3d12_video_encoder_update_current_encoder_config_state(struct d3d12_video_encoder *pD3D12Enc,
                                                        D3D12_VIDEO_SAMPLE srcTextureDesc,
                                                        struct pipe_picture_desc *  picture)
{
   pD3D12Enc->m_prevFrameEncodeConfig = pD3D12Enc->m_currentEncodeConfig;

   enum pipe_video_format codec = u_reduce_video_profile(pD3D12Enc->base.profile);
   switch (codec) {
#if VIDEO_CODEC_H264ENC
      case PIPE_VIDEO_FORMAT_MPEG4_AVC:
      {
         return d3d12_video_encoder_update_current_encoder_config_state_h264(pD3D12Enc, srcTextureDesc, picture);
      } break;
#endif
#if VIDEO_CODEC_H265ENC
      case PIPE_VIDEO_FORMAT_HEVC:
      {
         return d3d12_video_encoder_update_current_encoder_config_state_hevc(pD3D12Enc, srcTextureDesc, picture);
      } break;
#endif
#if VIDEO_CODEC_AV1ENC
      case PIPE_VIDEO_FORMAT_AV1:
      {
         return d3d12_video_encoder_update_current_encoder_config_state_av1(pD3D12Enc, srcTextureDesc, picture);
      } break;
#endif
      default:
      {
         unreachable("Unsupported pipe_video_format");
      } break;
   }
}

bool
d3d12_video_encoder_create_command_objects(struct d3d12_video_encoder *pD3D12Enc)
{
   assert(pD3D12Enc->m_spD3D12VideoDevice);

   D3D12_COMMAND_QUEUE_DESC commandQueueDesc = { D3D12_COMMAND_LIST_TYPE_VIDEO_ENCODE };
   HRESULT                  hr               = pD3D12Enc->m_pD3D12Screen->dev->CreateCommandQueue(
      &commandQueueDesc,
      IID_PPV_ARGS(pD3D12Enc->m_spEncodeCommandQueue.GetAddressOf()));
   if (FAILED(hr)) {
      debug_printf("[d3d12_video_encoder] d3d12_video_encoder_create_command_objects - Call to CreateCommandQueue "
                      "failed with HR %x\n",
                      hr);
      return false;
   }

   hr = pD3D12Enc->m_pD3D12Screen->dev->CreateFence(0, D3D12_FENCE_FLAG_SHARED, IID_PPV_ARGS(&pD3D12Enc->m_spFence));
   if (FAILED(hr)) {
      debug_printf(
         "[d3d12_video_encoder] d3d12_video_encoder_create_command_objects - Call to CreateFence failed with HR %x\n",
         hr);
      return false;
   }

   for (auto& inputResource : pD3D12Enc->m_inflightResourcesPool)
   {
      // Create associated command allocator for Encode, Resolve operations
      hr = pD3D12Enc->m_pD3D12Screen->dev->CreateCommandAllocator(
         D3D12_COMMAND_LIST_TYPE_VIDEO_ENCODE,
         IID_PPV_ARGS(inputResource.m_spCommandAllocator.GetAddressOf()));
      if (FAILED(hr)) {
         debug_printf("[d3d12_video_encoder] d3d12_video_encoder_create_command_objects - Call to "
                        "CreateCommandAllocator failed with HR %x\n",
                        hr);
         return false;
      }
   }

   ComPtr<ID3D12Device4> spD3D12Device4;
   if (FAILED(pD3D12Enc->m_pD3D12Screen->dev->QueryInterface(
          IID_PPV_ARGS(spD3D12Device4.GetAddressOf())))) {
      debug_printf(
         "[d3d12_video_encoder] d3d12_video_encoder_create_encoder - D3D12 Device has no Video encode support\n");
      return false;
   }

   hr = spD3D12Device4->CreateCommandList1(0,
                        D3D12_COMMAND_LIST_TYPE_VIDEO_ENCODE,
                        D3D12_COMMAND_LIST_FLAG_NONE,
                        IID_PPV_ARGS(pD3D12Enc->m_spEncodeCommandList.GetAddressOf()));

   if (FAILED(hr)) {
      debug_printf("[d3d12_video_encoder] d3d12_video_encoder_create_command_objects - Call to CreateCommandList "
                      "failed with HR %x\n",
                      hr);
      return false;
   }

   return true;
}

struct pipe_video_codec *
d3d12_video_encoder_create_encoder(struct pipe_context *context, const struct pipe_video_codec *codec)
{
   ///
   /// Initialize d3d12_video_encoder
   ///

   // Not using new doesn't call ctor and the initializations in the class declaration are lost
   struct d3d12_video_encoder *pD3D12Enc = new d3d12_video_encoder;

   pD3D12Enc->m_spEncodedFrameMetadata.resize(D3D12_VIDEO_ENC_METADATA_BUFFERS_COUNT, {nullptr, 0, 0});
   pD3D12Enc->m_inflightResourcesPool.resize(D3D12_VIDEO_ENC_ASYNC_DEPTH, { 0 });

   pD3D12Enc->base         = *codec;
   pD3D12Enc->m_screen     = context->screen;
   pD3D12Enc->base.context = context;
   pD3D12Enc->base.width   = codec->width;
   pD3D12Enc->base.height  = codec->height;
   pD3D12Enc->base.max_references  = codec->max_references;
   // Only fill methods that are supported by the d3d12 encoder, leaving null the rest (ie. encode_* / encode_macroblock)
   pD3D12Enc->base.destroy          = d3d12_video_encoder_destroy;
   pD3D12Enc->base.begin_frame      = d3d12_video_encoder_begin_frame;
   pD3D12Enc->base.encode_bitstream = d3d12_video_encoder_encode_bitstream;
   pD3D12Enc->base.end_frame        = d3d12_video_encoder_end_frame;
   pD3D12Enc->base.flush            = d3d12_video_encoder_flush;
   pD3D12Enc->base.get_encode_headers = d3d12_video_encoder_get_encode_headers;
   pD3D12Enc->base.get_feedback_fence = d3d12_video_encoder_get_feedback_fence;
   pD3D12Enc->base.get_feedback     = d3d12_video_encoder_get_feedback;

   struct d3d12_context *pD3D12Ctx = (struct d3d12_context *) context;
   pD3D12Enc->m_pD3D12Screen       = d3d12_screen(pD3D12Ctx->base.screen);

   if (FAILED(pD3D12Enc->m_pD3D12Screen->dev->QueryInterface(
          IID_PPV_ARGS(pD3D12Enc->m_spD3D12VideoDevice.GetAddressOf())))) {
      debug_printf(
         "[d3d12_video_encoder] d3d12_video_encoder_create_encoder - D3D12 Device has no Video encode support\n");
      goto failed;
   }

   if (!d3d12_video_encoder_create_command_objects(pD3D12Enc)) {
      debug_printf("[d3d12_video_encoder] d3d12_video_encoder_create_encoder - Failure on "
                      "d3d12_video_encoder_create_command_objects\n");
      goto failed;
   }

   // Cache quality levels cap
   pD3D12Enc->max_quality_levels = context->screen->get_video_param(context->screen, codec->profile,
                                    codec->entrypoint,
                                    PIPE_VIDEO_CAP_ENC_QUALITY_LEVEL);

   return &pD3D12Enc->base;

failed:
   if (pD3D12Enc != nullptr) {
      d3d12_video_encoder_destroy((struct pipe_video_codec *) pD3D12Enc);
   }

   return nullptr;
}

bool
d3d12_video_encoder_prepare_output_buffers(struct d3d12_video_encoder *pD3D12Enc,
                                           struct pipe_video_buffer *  srcTexture,
                                           struct pipe_picture_desc *  picture)
{
   pD3D12Enc->m_currentEncodeCapabilities.m_ResourceRequirementsCaps.NodeIndex = pD3D12Enc->m_NodeIndex;
   pD3D12Enc->m_currentEncodeCapabilities.m_ResourceRequirementsCaps.Codec =
      pD3D12Enc->m_currentEncodeConfig.m_encoderCodecDesc;
   pD3D12Enc->m_currentEncodeCapabilities.m_ResourceRequirementsCaps.Profile =
      d3d12_video_encoder_get_current_profile_desc(pD3D12Enc);
   pD3D12Enc->m_currentEncodeCapabilities.m_ResourceRequirementsCaps.InputFormat =
      pD3D12Enc->m_currentEncodeConfig.m_encodeFormatInfo.Format;
   pD3D12Enc->m_currentEncodeCapabilities.m_ResourceRequirementsCaps.PictureTargetResolution =
      pD3D12Enc->m_currentEncodeConfig.m_currentResolution;

   HRESULT hr = pD3D12Enc->m_spD3D12VideoDevice->CheckFeatureSupport(
      D3D12_FEATURE_VIDEO_ENCODER_RESOURCE_REQUIREMENTS,
      &pD3D12Enc->m_currentEncodeCapabilities.m_ResourceRequirementsCaps,
      sizeof(pD3D12Enc->m_currentEncodeCapabilities.m_ResourceRequirementsCaps));

   if (FAILED(hr)) {
      debug_printf("CheckFeatureSupport failed with HR %x\n", hr);
      return false;
   }

   if (!pD3D12Enc->m_currentEncodeCapabilities.m_ResourceRequirementsCaps.IsSupported) {
      debug_printf("[d3d12_video_encoder] D3D12_FEATURE_VIDEO_ENCODER_RESOURCE_REQUIREMENTS arguments are not supported.\n");
      return false;
   }

   uint64_t current_metadata_slot = (pD3D12Enc->m_fenceValue % D3D12_VIDEO_ENC_METADATA_BUFFERS_COUNT);

   enum pipe_video_format codec = u_reduce_video_profile(pD3D12Enc->base.profile);
   d3d12_video_encoder_calculate_metadata_resolved_buffer_size(
      codec,
      pD3D12Enc->m_currentEncodeCapabilities.m_MaxSlicesInOutput,
      pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].bufferSize);

   D3D12_HEAP_PROPERTIES Properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
   if ((pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].spBuffer == nullptr) ||
       (GetDesc(pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].spBuffer.Get()).Width <
        pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].bufferSize)) {
      CD3DX12_RESOURCE_DESC resolvedMetadataBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(
         pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].bufferSize);

      pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].spBuffer.Reset();
      HRESULT hr = pD3D12Enc->m_pD3D12Screen->dev->CreateCommittedResource(
         &Properties,
         D3D12_HEAP_FLAG_NONE,
         &resolvedMetadataBufferDesc,
         D3D12_RESOURCE_STATE_COMMON,
         nullptr,
         IID_PPV_ARGS(pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].spBuffer.GetAddressOf()));

      if (FAILED(hr)) {
         debug_printf("CreateCommittedResource failed with HR %x\n", hr);
         return false;
      }
   }

   if ((pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].m_spMetadataOutputBuffer == nullptr) ||
       (GetDesc(pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].m_spMetadataOutputBuffer.Get()).Width <
        pD3D12Enc->m_currentEncodeCapabilities.m_ResourceRequirementsCaps.MaxEncoderOutputMetadataBufferSize)) {
      CD3DX12_RESOURCE_DESC metadataBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(
         pD3D12Enc->m_currentEncodeCapabilities.m_ResourceRequirementsCaps.MaxEncoderOutputMetadataBufferSize);

      pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].m_spMetadataOutputBuffer.Reset();
      HRESULT hr = pD3D12Enc->m_pD3D12Screen->dev->CreateCommittedResource(
         &Properties,
         D3D12_HEAP_FLAG_NONE,
         &metadataBufferDesc,
         D3D12_RESOURCE_STATE_COMMON,
         nullptr,
         IID_PPV_ARGS(pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].m_spMetadataOutputBuffer.GetAddressOf()));

      if (FAILED(hr)) {
         debug_printf("CreateCommittedResource failed with HR %x\n", hr);
         return false;
      }
   }
   return true;
}

bool
d3d12_video_encoder_reconfigure_session(struct d3d12_video_encoder *pD3D12Enc,
                                        struct pipe_video_buffer *  srcTexture,
                                        struct pipe_picture_desc *  picture)
{
   assert(pD3D12Enc->m_spD3D12VideoDevice);
   D3D12_VIDEO_SAMPLE srcTextureDesc = {};
   srcTextureDesc.Width = srcTexture->width;
   srcTextureDesc.Height = srcTexture->height;
   srcTextureDesc.Format.Format = d3d12_get_format(srcTexture->buffer_format);
   if(!d3d12_video_encoder_update_current_encoder_config_state(pD3D12Enc, srcTextureDesc, picture)) {
      debug_printf("d3d12_video_encoder_update_current_encoder_config_state failed!\n");
      return false;
   }
   if(!d3d12_video_encoder_reconfigure_encoder_objects(pD3D12Enc, srcTexture, picture)) {
      debug_printf("d3d12_video_encoder_reconfigure_encoder_objects failed!\n");
      return false;
   }
   d3d12_video_encoder_update_picparams_tracking(pD3D12Enc, srcTexture, picture);
   if(!d3d12_video_encoder_prepare_output_buffers(pD3D12Enc, srcTexture, picture)) {
      debug_printf("d3d12_video_encoder_prepare_output_buffers failed!\n");
      return false;
   }

   // Save frame size expectation snapshot from record time to resolve at get_feedback time (after execution)
   uint64_t current_metadata_slot = (pD3D12Enc->m_fenceValue % D3D12_VIDEO_ENC_METADATA_BUFFERS_COUNT);
   pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].expected_max_frame_size =
      pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.max_frame_size;

   pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].expected_max_slice_size =
      (pD3D12Enc->m_currentEncodeConfig.m_encoderSliceConfigMode == D3D12_VIDEO_ENCODER_FRAME_SUBREGION_LAYOUT_MODE_BYTES_PER_SUBREGION) ?
      pD3D12Enc->m_currentEncodeConfig.m_encoderSliceConfigDesc.m_SlicesPartition_H264.MaxBytesPerSlice : 0;

   return true;
}

/**
 * start encoding of a new frame
 */
void
d3d12_video_encoder_begin_frame(struct pipe_video_codec * codec,
                                struct pipe_video_buffer *target,
                                struct pipe_picture_desc *picture)
{
   // Do nothing here. Initialize happens on encoder creation, re-config (if any) happens in
   // d3d12_video_encoder_encode_bitstream
   struct d3d12_video_encoder *pD3D12Enc = (struct d3d12_video_encoder *) codec;
   assert(pD3D12Enc);
   HRESULT hr = S_OK;
   debug_printf("[d3d12_video_encoder] d3d12_video_encoder_begin_frame started for fenceValue: %" PRIu64 "\n",
                 pD3D12Enc->m_fenceValue);

   ///
   /// Wait here to make sure the next in flight resource set is empty before using it
   ///
   uint64_t fenceValueToWaitOn = static_cast<uint64_t>(std::max(static_cast<int64_t>(0l), static_cast<int64_t>(pD3D12Enc->m_fenceValue) - static_cast<int64_t>(D3D12_VIDEO_ENC_ASYNC_DEPTH) ));

   debug_printf("[d3d12_video_encoder] d3d12_video_encoder_begin_frame Waiting for completion of in flight resource sets with previous work with fenceValue: %" PRIu64 "\n",
                 fenceValueToWaitOn);

   d3d12_video_encoder_ensure_fence_finished(codec, fenceValueToWaitOn, OS_TIMEOUT_INFINITE);

   if (!d3d12_video_encoder_reconfigure_session(pD3D12Enc, target, picture)) {
      debug_printf("[d3d12_video_encoder] d3d12_video_encoder_begin_frame - Failure on "
                      "d3d12_video_encoder_reconfigure_session\n");
      goto fail;
   }

   hr = pD3D12Enc->m_spEncodeCommandList->Reset(pD3D12Enc->m_inflightResourcesPool[d3d12_video_encoder_pool_current_index(pD3D12Enc)].m_spCommandAllocator.Get());
   if (FAILED(hr)) {
      debug_printf(
         "[d3d12_video_encoder] d3d12_video_encoder_flush - resetting ID3D12GraphicsCommandList failed with HR %x\n",
         hr);
      goto fail;
   }

   pD3D12Enc->m_inflightResourcesPool[d3d12_video_encoder_pool_current_index(pD3D12Enc)].m_InputSurfaceFence = (struct d3d12_fence*) *picture->fence;
   pD3D12Enc->m_inflightResourcesPool[d3d12_video_encoder_pool_current_index(pD3D12Enc)].encode_result = PIPE_VIDEO_FEEDBACK_METADATA_ENCODE_FLAG_OK;
   pD3D12Enc->m_spEncodedFrameMetadata[pD3D12Enc->m_fenceValue % D3D12_VIDEO_ENC_METADATA_BUFFERS_COUNT].encode_result = PIPE_VIDEO_FEEDBACK_METADATA_ENCODE_FLAG_OK;

   debug_printf("[d3d12_video_encoder] d3d12_video_encoder_begin_frame finalized for fenceValue: %" PRIu64 "\n",
                 pD3D12Enc->m_fenceValue);
   return;

fail:
   debug_printf("[d3d12_video_encoder] d3d12_video_encoder_begin_frame failed for fenceValue: %" PRIu64 "\n",
                pD3D12Enc->m_fenceValue);
   pD3D12Enc->m_inflightResourcesPool[d3d12_video_encoder_pool_current_index(pD3D12Enc)].encode_result = PIPE_VIDEO_FEEDBACK_METADATA_ENCODE_FLAG_FAILED;
   pD3D12Enc->m_spEncodedFrameMetadata[pD3D12Enc->m_fenceValue % D3D12_VIDEO_ENC_METADATA_BUFFERS_COUNT].encode_result = PIPE_VIDEO_FEEDBACK_METADATA_ENCODE_FLAG_FAILED;
   assert(false);
}

void
d3d12_video_encoder_calculate_metadata_resolved_buffer_size(enum pipe_video_format codec, uint32_t maxSliceNumber, uint64_t &bufferSize)
{
   bufferSize = sizeof(D3D12_VIDEO_ENCODER_OUTPUT_METADATA) +
                (maxSliceNumber * sizeof(D3D12_VIDEO_ENCODER_FRAME_SUBREGION_METADATA));
                
   switch (codec) {
#if VIDEO_CODEC_H264ENC
      case PIPE_VIDEO_FORMAT_MPEG4_AVC:
         break;
#endif
#if VIDEO_CODEC_H265ENC
      case PIPE_VIDEO_FORMAT_HEVC:
         break;
#endif
#if VIDEO_CODEC_AV1ENC
      case PIPE_VIDEO_FORMAT_AV1:
      {
         size_t extra_av1_size = d3d12_video_encoder_calculate_metadata_resolved_buffer_size_av1(maxSliceNumber);
         bufferSize += extra_av1_size;
      } break;
#endif
      default:
      {
         unreachable("Unsupported pipe_video_format");
      } break;
   }
}

// Returns the number of slices that the output will contain for fixed slicing modes
// and the maximum number of slices the output might contain for dynamic slicing modes (eg. max bytes per slice)
uint32_t
d3d12_video_encoder_calculate_max_slices_count_in_output(
   D3D12_VIDEO_ENCODER_FRAME_SUBREGION_LAYOUT_MODE                          slicesMode,
   const D3D12_VIDEO_ENCODER_PICTURE_CONTROL_SUBREGIONS_LAYOUT_DATA_SLICES *slicesConfig,
   uint32_t                                                                 MaxSubregionsNumberFromCaps,
   D3D12_VIDEO_ENCODER_PICTURE_RESOLUTION_DESC                              sequenceTargetResolution,
   uint32_t                                                                 SubregionBlockPixelsSize)
{
   uint32_t pic_width_in_subregion_units =
      static_cast<uint32_t>(std::ceil(sequenceTargetResolution.Width / static_cast<double>(SubregionBlockPixelsSize)));
   uint32_t pic_height_in_subregion_units =
      static_cast<uint32_t>(std::ceil(sequenceTargetResolution.Height / static_cast<double>(SubregionBlockPixelsSize)));
   uint32_t total_picture_subregion_units = pic_width_in_subregion_units * pic_height_in_subregion_units;
   uint32_t maxSlices                     = 0u;
   switch (slicesMode) {
      case D3D12_VIDEO_ENCODER_FRAME_SUBREGION_LAYOUT_MODE_FULL_FRAME:
      {
         maxSlices = 1u;
      } break;
      case D3D12_VIDEO_ENCODER_FRAME_SUBREGION_LAYOUT_MODE_BYTES_PER_SUBREGION:
      {
         maxSlices = MaxSubregionsNumberFromCaps;
      } break;
      case D3D12_VIDEO_ENCODER_FRAME_SUBREGION_LAYOUT_MODE_SQUARE_UNITS_PER_SUBREGION_ROW_UNALIGNED:
      {
         maxSlices = static_cast<uint32_t>(
            std::ceil(total_picture_subregion_units / static_cast<double>(slicesConfig->NumberOfCodingUnitsPerSlice)));
      } break;
      case D3D12_VIDEO_ENCODER_FRAME_SUBREGION_LAYOUT_MODE_UNIFORM_PARTITIONING_ROWS_PER_SUBREGION:
      {
         maxSlices = static_cast<uint32_t>(
            std::ceil(pic_height_in_subregion_units / static_cast<double>(slicesConfig->NumberOfRowsPerSlice)));
      } break;
      case D3D12_VIDEO_ENCODER_FRAME_SUBREGION_LAYOUT_MODE_UNIFORM_PARTITIONING_SUBREGIONS_PER_FRAME:
      {
         maxSlices = slicesConfig->NumberOfSlicesPerFrame;
      } break;
      default:
      {
         unreachable("Unsupported D3D12_VIDEO_ENCODER_FRAME_SUBREGION_LAYOUT_MODE");
      } break;
   }

   return maxSlices;
}

/**
 * encode a bitstream
 */
void
d3d12_video_encoder_encode_bitstream(struct pipe_video_codec * codec,
                                     struct pipe_video_buffer *source,
                                     struct pipe_resource *    destination,
                                     void **                   feedback)
{
   struct d3d12_video_encoder *pD3D12Enc = (struct d3d12_video_encoder *) codec;
   assert(pD3D12Enc);
   debug_printf("[d3d12_video_encoder] d3d12_video_encoder_encode_bitstream started for fenceValue: %" PRIu64 "\n",
                 pD3D12Enc->m_fenceValue);
   assert(pD3D12Enc->m_spD3D12VideoDevice);
   assert(pD3D12Enc->m_spEncodeCommandQueue);
   assert(pD3D12Enc->m_pD3D12Screen);

   if (pD3D12Enc->m_inflightResourcesPool[d3d12_video_encoder_pool_current_index(pD3D12Enc)].encode_result & PIPE_VIDEO_FEEDBACK_METADATA_ENCODE_FLAG_FAILED) {
      debug_printf("WARNING: [d3d12_video_encoder] d3d12_video_encoder_encode_bitstream - Frame submission %" PRIu64 " failed. Encoder lost, please recreate pipe_video_codec object\n", pD3D12Enc->m_fenceValue);
      assert(false);
      return;
   }

   struct d3d12_video_buffer *pInputVideoBuffer = (struct d3d12_video_buffer *) source;
   assert(pInputVideoBuffer);
   ID3D12Resource *pInputVideoD3D12Res        = d3d12_resource_resource(pInputVideoBuffer->texture);
   uint32_t        inputVideoD3D12Subresource = 0u;

   struct d3d12_resource *pOutputBitstreamBuffer = (struct d3d12_resource *) destination;

   // Make them permanently resident for video use
   d3d12_promote_to_permanent_residency(pD3D12Enc->m_pD3D12Screen, pOutputBitstreamBuffer);
   d3d12_promote_to_permanent_residency(pD3D12Enc->m_pD3D12Screen, pInputVideoBuffer->texture);

   uint64_t current_metadata_slot = (pD3D12Enc->m_fenceValue % D3D12_VIDEO_ENC_METADATA_BUFFERS_COUNT);

   /* Warning if the previous finished async execution stored was read not by get_feedback()
      before overwriting. This should be handled correctly by the app by calling vaSyncBuffer/vaSyncSurface
      without having the async depth going beyond D3D12_VIDEO_ENC_METADATA_BUFFERS_COUNT frames without syncing */
   if(!pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].bRead) {
      debug_printf("WARNING: [d3d12_video_encoder] d3d12_video_encoder_encode_bitstream - overwriting metadata slot %" PRIu64 " before calling get_feedback", current_metadata_slot);
      assert(false);
   }
   pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].bRead = false;

   ///
   /// Record Encode operation
   ///

   ///
   /// pInputVideoBuffer and pOutputBitstreamBuffer are passed externally
   /// and could be tracked by pipe_context and have pending ops. Flush any work on them and transition to
   /// D3D12_RESOURCE_STATE_COMMON before issuing work in Video command queue below. After the video work is done in the
   /// GPU, transition back to D3D12_RESOURCE_STATE_COMMON
   ///
   /// Note that unlike the D3D12TranslationLayer codebase, the state tracker here doesn't (yet) have any kind of
   /// multi-queue support, so it wouldn't implicitly synchronize when trying to transition between a graphics op and a
   /// video op.
   ///

   d3d12_transition_resource_state(
      d3d12_context(pD3D12Enc->base.context),
      pInputVideoBuffer->texture,
      D3D12_RESOURCE_STATE_COMMON,
      D3D12_TRANSITION_FLAG_INVALIDATE_BINDINGS);
   d3d12_transition_resource_state(d3d12_context(pD3D12Enc->base.context),
                                   pOutputBitstreamBuffer,
                                   D3D12_RESOURCE_STATE_COMMON,
                                   D3D12_TRANSITION_FLAG_INVALIDATE_BINDINGS);
   d3d12_apply_resource_states(d3d12_context(pD3D12Enc->base.context), false);

   d3d12_resource_wait_idle(d3d12_context(pD3D12Enc->base.context),
                            pInputVideoBuffer->texture,
                            false /*wantToWrite*/);
   d3d12_resource_wait_idle(d3d12_context(pD3D12Enc->base.context), pOutputBitstreamBuffer, true /*wantToWrite*/);

   ///
   /// Process pre-encode bitstream headers
   ///

   // Decide the D3D12 buffer EncodeFrame will write to based on pre-post encode headers generation policy
   ID3D12Resource *pOutputBufferD3D12Res = nullptr;

   d3d12_video_encoder_build_pre_encode_codec_headers(pD3D12Enc, 
                                                      pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].postEncodeHeadersNeeded,
                                                      pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].preEncodeGeneratedHeadersByteSize,
                                                      pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].pWrittenCodecUnitsSizes);
   assert(pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].preEncodeGeneratedHeadersByteSize == pD3D12Enc->m_BitstreamHeadersBuffer.size());
   pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].preEncodeGeneratedHeadersBytePadding = 0;

   // Only upload headers now and leave prefix offset space gap in compressed bitstream if the codec builds headers before execution.
   if (!pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].postEncodeHeadersNeeded)
   {

      // Headers are written before encode execution, have EncodeFrame write directly into the pipe destination buffer
      pOutputBufferD3D12Res = d3d12_resource_resource(pOutputBitstreamBuffer);

      // It can happen that codecs like H264/HEVC don't write pre-headers for all frames (ie. reuse previous PPS)
      if (pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].preEncodeGeneratedHeadersByteSize > 0)
      {
         // If driver needs offset alignment for bitstream resource, we will pad zeroes on the codec header to this end.
         if (
            (pD3D12Enc->m_currentEncodeCapabilities.m_ResourceRequirementsCaps.CompressedBitstreamBufferAccessAlignment > 1)
            && ((pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].preEncodeGeneratedHeadersByteSize % pD3D12Enc->m_currentEncodeCapabilities.m_ResourceRequirementsCaps.CompressedBitstreamBufferAccessAlignment) != 0)
         ) {
            size_t new_size = ALIGN(pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].preEncodeGeneratedHeadersByteSize, pD3D12Enc->m_currentEncodeCapabilities.m_ResourceRequirementsCaps.CompressedBitstreamBufferAccessAlignment);
            pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].preEncodeGeneratedHeadersBytePadding = new_size - pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].preEncodeGeneratedHeadersByteSize;
            pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].preEncodeGeneratedHeadersByteSize = new_size;
            pD3D12Enc->m_BitstreamHeadersBuffer.resize(pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].preEncodeGeneratedHeadersByteSize, 0);
         }

         // Upload the CPU buffers with the bitstream headers to the compressed bitstream resource in the interval
         // [0..pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].preEncodeGeneratedHeadersByteSize)
         // Note: The buffer_subdata is queued in pD3D12Enc->base.context but doesn't execute immediately
         // Will flush and sync this batch in d3d12_video_encoder_flush with the rest of the Video Encode Queue GPU work

         pD3D12Enc->base.context->buffer_subdata(
            pD3D12Enc->base.context,         // context
            &pOutputBitstreamBuffer->base.b, // dst buffer
            PIPE_MAP_WRITE,                  // usage PIPE_MAP_x
            0,                               // offset
            pD3D12Enc->m_BitstreamHeadersBuffer.size(),
            pD3D12Enc->m_BitstreamHeadersBuffer.data());
      }
   }
   else
   {
      assert(pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].preEncodeGeneratedHeadersByteSize == 0);
      if (pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].spStagingBitstream == nullptr) {
         D3D12_HEAP_PROPERTIES Properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
         CD3DX12_RESOURCE_DESC resolvedMetadataBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(D3D12_DEFAULT_COMPBIT_STAGING_SIZE);
         HRESULT hr = pD3D12Enc->m_pD3D12Screen->dev->CreateCommittedResource(
            &Properties,
            D3D12_HEAP_FLAG_NONE,
            &resolvedMetadataBufferDesc,
            D3D12_RESOURCE_STATE_COMMON,
            nullptr,
            IID_PPV_ARGS(pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].spStagingBitstream.GetAddressOf()));

         if (FAILED(hr)) {
            debug_printf("CreateCommittedResource failed with HR %x\n", hr);
            pD3D12Enc->m_inflightResourcesPool[d3d12_video_encoder_pool_current_index(pD3D12Enc)].encode_result = PIPE_VIDEO_FEEDBACK_METADATA_ENCODE_FLAG_FAILED;
            pD3D12Enc->m_spEncodedFrameMetadata[pD3D12Enc->m_fenceValue % D3D12_VIDEO_ENC_METADATA_BUFFERS_COUNT].encode_result = PIPE_VIDEO_FEEDBACK_METADATA_ENCODE_FLAG_FAILED;
            assert(false);
            return;
         }
      }
      
      // Headers are written after execution, have EncodeFrame write into a staging buffer 
      // and then get_feedback will pack the finalized bitstream and copy into comp_bit_destination
      pOutputBufferD3D12Res = pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].spStagingBitstream.Get();
      
      // Save the pipe destination buffer the headers need to be written to in get_feedback
      pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].comp_bit_destination = &pOutputBitstreamBuffer->base.b;
   }

   memset(&pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].m_FenceData,
            0,
            sizeof(pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].m_FenceData));
   pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].m_FenceData.value = pD3D12Enc->m_fenceValue;
   pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].m_FenceData.cmdqueue_fence = pD3D12Enc->m_spFence.Get();
   *feedback = (void*) &pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].m_FenceData;

   std::vector<D3D12_RESOURCE_BARRIER> rgCurrentFrameStateTransitions = {
      CD3DX12_RESOURCE_BARRIER::Transition(pInputVideoD3D12Res,
                                           D3D12_RESOURCE_STATE_COMMON,
                                           D3D12_RESOURCE_STATE_VIDEO_ENCODE_READ),
      CD3DX12_RESOURCE_BARRIER::Transition(pOutputBufferD3D12Res,
                                           D3D12_RESOURCE_STATE_COMMON,
                                           D3D12_RESOURCE_STATE_VIDEO_ENCODE_WRITE),
      CD3DX12_RESOURCE_BARRIER::Transition(pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].m_spMetadataOutputBuffer.Get(),
                                           D3D12_RESOURCE_STATE_COMMON,
                                           D3D12_RESOURCE_STATE_VIDEO_ENCODE_WRITE)
   };

   pD3D12Enc->m_spEncodeCommandList->ResourceBarrier(rgCurrentFrameStateTransitions.size(),
                                                     rgCurrentFrameStateTransitions.data());

   D3D12_VIDEO_ENCODER_RECONSTRUCTED_PICTURE reconPicOutputTextureDesc =
      pD3D12Enc->m_upDPBManager->get_current_frame_recon_pic_output_allocation();
   D3D12_VIDEO_ENCODE_REFERENCE_FRAMES referenceFramesDescriptor =
      pD3D12Enc->m_upDPBManager->get_current_reference_frames();
   D3D12_VIDEO_ENCODER_PICTURE_CONTROL_FLAGS picCtrlFlags = D3D12_VIDEO_ENCODER_PICTURE_CONTROL_FLAG_NONE;

   // Transition DPB reference pictures to read mode
   uint32_t                            maxReferences = d3d12_video_encoder_get_current_max_dpb_capacity(pD3D12Enc);
   std::vector<D3D12_RESOURCE_BARRIER> rgReferenceTransitions(maxReferences);
   if ((referenceFramesDescriptor.NumTexture2Ds > 0) ||
       (pD3D12Enc->m_upDPBManager->is_current_frame_used_as_reference())) {
      rgReferenceTransitions.clear();
      rgReferenceTransitions.reserve(maxReferences);

      if (reconPicOutputTextureDesc.pReconstructedPicture != nullptr)
         picCtrlFlags |= D3D12_VIDEO_ENCODER_PICTURE_CONTROL_FLAG_USED_AS_REFERENCE_PICTURE;

      // Check if array of textures vs texture array

      if (referenceFramesDescriptor.pSubresources == nullptr) {

         // Array of resources mode for reference pictures

         // Transition all subresources of each reference frame independent resource allocation
         for (uint32_t referenceIdx = 0; referenceIdx < referenceFramesDescriptor.NumTexture2Ds; referenceIdx++) {
            rgReferenceTransitions.push_back(
               CD3DX12_RESOURCE_BARRIER::Transition(referenceFramesDescriptor.ppTexture2Ds[referenceIdx],
                                                    D3D12_RESOURCE_STATE_COMMON,
                                                    D3D12_RESOURCE_STATE_VIDEO_ENCODE_READ));
         }

         // Transition all subresources the output recon pic independent resource allocation
         if (reconPicOutputTextureDesc.pReconstructedPicture != nullptr) {
            rgReferenceTransitions.push_back(
               CD3DX12_RESOURCE_BARRIER::Transition(reconPicOutputTextureDesc.pReconstructedPicture,
                                                    D3D12_RESOURCE_STATE_COMMON,
                                                    D3D12_RESOURCE_STATE_VIDEO_ENCODE_WRITE));
         }
      } else if (referenceFramesDescriptor.NumTexture2Ds > 0) {

         // texture array mode for reference pictures

         // In Texture array mode, the dpb storage allocator uses the same texture array for all the input
         // reference pics in ppTexture2Ds and also for the pReconstructedPicture output allocations, just different
         // subresources.

         CD3DX12_RESOURCE_DESC referencesTexArrayDesc(GetDesc(referenceFramesDescriptor.ppTexture2Ds[0]));

#if DEBUG
   // the reconpic output should be all the same texarray allocation
   if((reconPicOutputTextureDesc.pReconstructedPicture) && (referenceFramesDescriptor.NumTexture2Ds > 0))
      assert(referenceFramesDescriptor.ppTexture2Ds[0] == reconPicOutputTextureDesc.pReconstructedPicture);

   for (uint32_t refIndex = 0; refIndex < referenceFramesDescriptor.NumTexture2Ds; refIndex++) {
            // all reference frames inputs should be all the same texarray allocation
            assert(referenceFramesDescriptor.ppTexture2Ds[0] ==
                   referenceFramesDescriptor.ppTexture2Ds[refIndex]);
   }
#endif

         for (uint32_t referenceSubresource = 0; referenceSubresource < referencesTexArrayDesc.DepthOrArraySize;
              referenceSubresource++) {

            uint32_t MipLevel, PlaneSlice, ArraySlice;
            D3D12DecomposeSubresource(referenceSubresource,
                                      referencesTexArrayDesc.MipLevels,
                                      referencesTexArrayDesc.ArraySize(),
                                      MipLevel,
                                      ArraySlice,
                                      PlaneSlice);

            for (PlaneSlice = 0; PlaneSlice < pD3D12Enc->m_currentEncodeConfig.m_encodeFormatInfo.PlaneCount;
                 PlaneSlice++) {

               uint32_t planeOutputSubresource =
                  referencesTexArrayDesc.CalcSubresource(MipLevel, ArraySlice, PlaneSlice);

               rgReferenceTransitions.push_back(CD3DX12_RESOURCE_BARRIER::Transition(
                  // Always same allocation in texarray mode
                  referenceFramesDescriptor.ppTexture2Ds[0],
                  D3D12_RESOURCE_STATE_COMMON,
                  // If this is the subresource for the reconpic output allocation, transition to ENCODE_WRITE
                  // Otherwise, it's a subresource for an input reference picture, transition to ENCODE_READ
                  (referenceSubresource == reconPicOutputTextureDesc.ReconstructedPictureSubresource) ?
                     D3D12_RESOURCE_STATE_VIDEO_ENCODE_WRITE :
                     D3D12_RESOURCE_STATE_VIDEO_ENCODE_READ,
                  planeOutputSubresource));
            }
         }
      }

      if (rgReferenceTransitions.size() > 0) {
         pD3D12Enc->m_spEncodeCommandList->ResourceBarrier(static_cast<uint32_t>(rgReferenceTransitions.size()),
                                                           rgReferenceTransitions.data());
      }
   }

   // Update current frame pic params state after reconfiguring above.
   D3D12_VIDEO_ENCODER_PICTURE_CONTROL_CODEC_DATA currentPicParams =
      d3d12_video_encoder_get_current_picture_param_settings(pD3D12Enc);

   if (!pD3D12Enc->m_upDPBManager->get_current_frame_picture_control_data(currentPicParams)) {
      debug_printf("[d3d12_video_encoder_encode_bitstream] get_current_frame_picture_control_data failed!\n");
      pD3D12Enc->m_inflightResourcesPool[d3d12_video_encoder_pool_current_index(pD3D12Enc)].encode_result = PIPE_VIDEO_FEEDBACK_METADATA_ENCODE_FLAG_FAILED;
      pD3D12Enc->m_spEncodedFrameMetadata[pD3D12Enc->m_fenceValue % D3D12_VIDEO_ENC_METADATA_BUFFERS_COUNT].encode_result = PIPE_VIDEO_FEEDBACK_METADATA_ENCODE_FLAG_FAILED;
      assert(false);
      return;
   }

   // Stores D3D12_VIDEO_ENCODER_AV1_REFERENCE_PICTURE_DESCRIPTOR in the associated metadata
   // for header generation after execution (if applicable)
   if (pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].postEncodeHeadersNeeded) {
      d3d12_video_encoder_store_current_picture_references(pD3D12Enc, current_metadata_slot);
   }

   const D3D12_VIDEO_ENCODER_ENCODEFRAME_INPUT_ARGUMENTS inputStreamArguments = {
      // D3D12_VIDEO_ENCODER_SEQUENCE_CONTROL_DESC
      { // D3D12_VIDEO_ENCODER_SEQUENCE_CONTROL_FLAGS
        pD3D12Enc->m_currentEncodeConfig.m_seqFlags,
        // D3D12_VIDEO_ENCODER_INTRA_REFRESH
        pD3D12Enc->m_currentEncodeConfig.m_IntraRefresh,
        d3d12_video_encoder_get_current_rate_control_settings(pD3D12Enc),
        // D3D12_VIDEO_ENCODER_PICTURE_RESOLUTION_DESC
        pD3D12Enc->m_currentEncodeConfig.m_currentResolution,
        pD3D12Enc->m_currentEncodeConfig.m_encoderSliceConfigMode,
        d3d12_video_encoder_get_current_slice_param_settings(pD3D12Enc),
        d3d12_video_encoder_get_current_gop_desc(pD3D12Enc) },
      // D3D12_VIDEO_ENCODER_PICTURE_CONTROL_DESC
      { // uint32_t IntraRefreshFrameIndex;
        pD3D12Enc->m_currentEncodeConfig.m_IntraRefreshCurrentFrameIndex,
        // D3D12_VIDEO_ENCODER_PICTURE_CONTROL_FLAGS Flags;
        picCtrlFlags,
        // D3D12_VIDEO_ENCODER_PICTURE_CONTROL_CODEC_DATA PictureControlCodecData;
        currentPicParams,
        // D3D12_VIDEO_ENCODE_REFERENCE_FRAMES ReferenceFrames;
        referenceFramesDescriptor },
      pInputVideoD3D12Res,
      inputVideoD3D12Subresource,
      static_cast<UINT>(pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].preEncodeGeneratedHeadersByteSize)
      // budgeting. - User can also calculate headers fixed size beforehand (eg. no VUI,
      // etc) and build them with final values after EncodeFrame is executed
   };

   const D3D12_VIDEO_ENCODER_ENCODEFRAME_OUTPUT_ARGUMENTS outputStreamArguments = {
      // D3D12_VIDEO_ENCODER_COMPRESSED_BITSTREAM
      {
         pOutputBufferD3D12Res,
         pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].preEncodeGeneratedHeadersByteSize,
      },
      // D3D12_VIDEO_ENCODER_RECONSTRUCTED_PICTURE
      reconPicOutputTextureDesc,
      // D3D12_VIDEO_ENCODER_ENCODE_OPERATION_METADATA_BUFFER
      { pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].m_spMetadataOutputBuffer.Get(), 0 }
   };

   // Record EncodeFrame
   pD3D12Enc->m_spEncodeCommandList->EncodeFrame(pD3D12Enc->m_spVideoEncoder.Get(),
                                                 pD3D12Enc->m_spVideoEncoderHeap.Get(),
                                                 &inputStreamArguments,
                                                 &outputStreamArguments);

   D3D12_RESOURCE_BARRIER rgResolveMetadataStateTransitions[] = {
      CD3DX12_RESOURCE_BARRIER::Transition(pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].spBuffer.Get(),
                                           D3D12_RESOURCE_STATE_COMMON,
                                           D3D12_RESOURCE_STATE_VIDEO_ENCODE_WRITE),
      CD3DX12_RESOURCE_BARRIER::Transition(pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].m_spMetadataOutputBuffer.Get(),
                                           D3D12_RESOURCE_STATE_VIDEO_ENCODE_WRITE,
                                           D3D12_RESOURCE_STATE_VIDEO_ENCODE_READ),
      CD3DX12_RESOURCE_BARRIER::Transition(pInputVideoD3D12Res,
                                           D3D12_RESOURCE_STATE_VIDEO_ENCODE_READ,
                                           D3D12_RESOURCE_STATE_COMMON),
      CD3DX12_RESOURCE_BARRIER::Transition(pOutputBufferD3D12Res,
                                           D3D12_RESOURCE_STATE_VIDEO_ENCODE_WRITE,
                                           D3D12_RESOURCE_STATE_COMMON)
   };

   pD3D12Enc->m_spEncodeCommandList->ResourceBarrier(_countof(rgResolveMetadataStateTransitions),
                                                     rgResolveMetadataStateTransitions);

   const D3D12_VIDEO_ENCODER_RESOLVE_METADATA_INPUT_ARGUMENTS inputMetadataCmd = {
      pD3D12Enc->m_currentEncodeConfig.m_encoderCodecDesc,
      d3d12_video_encoder_get_current_profile_desc(pD3D12Enc),
      pD3D12Enc->m_currentEncodeConfig.m_encodeFormatInfo.Format,
      // D3D12_VIDEO_ENCODER_PICTURE_RESOLUTION_DESC
      pD3D12Enc->m_currentEncodeConfig.m_currentResolution,
      { pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].m_spMetadataOutputBuffer.Get(), 0 }
   };

   const D3D12_VIDEO_ENCODER_RESOLVE_METADATA_OUTPUT_ARGUMENTS outputMetadataCmd = {
      /*If offset were to change, has to be aligned to pD3D12Enc->m_currentEncodeCapabilities.m_ResourceRequirementsCaps.EncoderMetadataBufferAccessAlignment*/
      { pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].spBuffer.Get(), 0 }
   };
   pD3D12Enc->m_spEncodeCommandList->ResolveEncoderOutputMetadata(&inputMetadataCmd, &outputMetadataCmd);

   debug_printf("[d3d12_video_encoder_encode_bitstream] EncodeFrame slot %" PRIu64 " encoder %p encoderheap %p input tex %p output bitstream %p raw metadata buf %p resolved metadata buf %p Command allocator %p\n",
               d3d12_video_encoder_pool_current_index(pD3D12Enc),
               pD3D12Enc->m_spVideoEncoder.Get(),
               pD3D12Enc->m_spVideoEncoderHeap.Get(),
               inputStreamArguments.pInputFrame,
               outputStreamArguments.Bitstream.pBuffer,
               inputMetadataCmd.HWLayoutMetadata.pBuffer,
               outputMetadataCmd.ResolvedLayoutMetadata.pBuffer,
               pD3D12Enc->m_inflightResourcesPool[d3d12_video_encoder_pool_current_index(pD3D12Enc)].m_spCommandAllocator.Get());

   // Transition DPB reference pictures back to COMMON
   if ((referenceFramesDescriptor.NumTexture2Ds > 0) ||
       (pD3D12Enc->m_upDPBManager->is_current_frame_used_as_reference())) {
      for (auto &BarrierDesc : rgReferenceTransitions) {
         std::swap(BarrierDesc.Transition.StateBefore, BarrierDesc.Transition.StateAfter);
      }

      if (rgReferenceTransitions.size() > 0) {
         pD3D12Enc->m_spEncodeCommandList->ResourceBarrier(static_cast<uint32_t>(rgReferenceTransitions.size()),
                                                           rgReferenceTransitions.data());
      }
   }

   D3D12_RESOURCE_BARRIER rgRevertResolveMetadataStateTransitions[] = {
      CD3DX12_RESOURCE_BARRIER::Transition(pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].spBuffer.Get(),
                                           D3D12_RESOURCE_STATE_VIDEO_ENCODE_WRITE,
                                           D3D12_RESOURCE_STATE_COMMON),
      CD3DX12_RESOURCE_BARRIER::Transition(pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].m_spMetadataOutputBuffer.Get(),
                                           D3D12_RESOURCE_STATE_VIDEO_ENCODE_READ,
                                           D3D12_RESOURCE_STATE_COMMON),
   };

   pD3D12Enc->m_spEncodeCommandList->ResourceBarrier(_countof(rgRevertResolveMetadataStateTransitions),
                                                     rgRevertResolveMetadataStateTransitions);

   debug_printf("[d3d12_video_encoder] d3d12_video_encoder_encode_bitstream finalized for fenceValue: %" PRIu64 "\n",
                 pD3D12Enc->m_fenceValue);
}

void
d3d12_video_encoder_get_feedback(struct pipe_video_codec *codec,
                                  void *feedback,
                                  unsigned *output_buffer_size,
                                  struct pipe_enc_feedback_metadata* pMetadata)
{
   struct d3d12_video_encoder *pD3D12Enc = (struct d3d12_video_encoder *) codec;
   assert(pD3D12Enc);

   struct d3d12_fence *feedback_fence = (struct d3d12_fence *) feedback;
   uint64_t requested_metadata_fence = feedback_fence->value;

   struct pipe_enc_feedback_metadata opt_metadata;
   memset(&opt_metadata, 0, sizeof(opt_metadata));

   HRESULT hr = pD3D12Enc->m_pD3D12Screen->dev->GetDeviceRemovedReason();
   if (hr != S_OK) {
      opt_metadata.encode_result = PIPE_VIDEO_FEEDBACK_METADATA_ENCODE_FLAG_FAILED;
      debug_printf("Error: d3d12_video_encoder_get_feedback for Encode GPU command for fence %" PRIu64 " failed with GetDeviceRemovedReason: %x\n",
                     requested_metadata_fence,
                     hr);
      assert(false);
      if(pMetadata)
         *pMetadata = opt_metadata;
      return;
   }

   uint64_t current_metadata_slot = (requested_metadata_fence % D3D12_VIDEO_ENC_METADATA_BUFFERS_COUNT);
   opt_metadata.encode_result = pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].encode_result;
   if (opt_metadata.encode_result & PIPE_VIDEO_FEEDBACK_METADATA_ENCODE_FLAG_FAILED) {
      debug_printf("Error: d3d12_video_encoder_get_feedback for Encode GPU command for fence %" PRIu64 " failed on submission with encode_result: %x\n",
                     requested_metadata_fence,
                     opt_metadata.encode_result);
      assert(false);
      if(pMetadata)
         *pMetadata = opt_metadata;
      return;
   }

   d3d12_video_encoder_sync_completion(codec, requested_metadata_fence, OS_TIMEOUT_INFINITE);

   opt_metadata.encode_result = pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].encode_result;
   if (opt_metadata.encode_result & PIPE_VIDEO_FEEDBACK_METADATA_ENCODE_FLAG_FAILED) {
      debug_printf("Error: d3d12_video_encoder_get_feedback for Encode GPU command for fence %" PRIu64 " failed on GPU fence wait with encode_result: %x\n",
                     requested_metadata_fence,
                     opt_metadata.encode_result);
      assert(false);
      if(pMetadata)
         *pMetadata = opt_metadata;
      return;
   }

   debug_printf("d3d12_video_encoder_get_feedback with feedback: %" PRIu64 ", resources slot %" PRIu64 " metadata resolved ID3D12Resource buffer %p metadata required size %" PRIu64 "\n",
      requested_metadata_fence,
      (requested_metadata_fence % D3D12_VIDEO_ENC_ASYNC_DEPTH),
      pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].spBuffer.Get(),
      pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].bufferSize);

   if((pD3D12Enc->m_fenceValue - requested_metadata_fence) > D3D12_VIDEO_ENC_METADATA_BUFFERS_COUNT)
   {
      debug_printf("[d3d12_video_encoder_get_feedback] Requested metadata for fence %" PRIu64 " at current fence %" PRIu64
         " is too far back in time for the ring buffer of size %" PRIu64 " we keep track off - "
         " Please increase the D3D12_VIDEO_ENC_METADATA_BUFFERS_COUNT environment variable and try again.\n",
         requested_metadata_fence,
         pD3D12Enc->m_fenceValue,
         D3D12_VIDEO_ENC_METADATA_BUFFERS_COUNT);
      opt_metadata.encode_result = PIPE_VIDEO_FEEDBACK_METADATA_ENCODE_FLAG_FAILED;
      assert(false);
      if(pMetadata)
         *pMetadata = opt_metadata;
      return;
   }

   // Extract encode metadata
   D3D12_VIDEO_ENCODER_OUTPUT_METADATA                       encoderMetadata;
   std::vector<D3D12_VIDEO_ENCODER_FRAME_SUBREGION_METADATA> pSubregionsMetadata;
   d3d12_video_encoder_extract_encode_metadata(
      pD3D12Enc,
      pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].spBuffer.Get(),
      pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].bufferSize,
      encoderMetadata,
      pSubregionsMetadata);

   // Validate encoder output metadata
   if ((encoderMetadata.EncodeErrorFlags != D3D12_VIDEO_ENCODER_ENCODE_ERROR_FLAG_NO_ERROR) || (encoderMetadata.EncodedBitstreamWrittenBytesCount == 0)) {
      opt_metadata.encode_result = PIPE_VIDEO_FEEDBACK_METADATA_ENCODE_FLAG_FAILED;
      debug_printf("[d3d12_video_encoder] Encode GPU command for fence %" PRIu64 " failed - EncodeErrorFlags: %" PRIu64 "\n",
                     requested_metadata_fence,
                     encoderMetadata.EncodeErrorFlags);
      assert(false);
      if(pMetadata)
         *pMetadata = opt_metadata;
      return;
   }

   uint64_t unpadded_frame_size = 0;
   if(pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].postEncodeHeadersNeeded)
   {
      *output_buffer_size = d3d12_video_encoder_build_post_encode_codec_bitstream(
         pD3D12Enc,
         requested_metadata_fence,
         pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot]
      );
      for (uint32_t i = 0; i < pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].pWrittenCodecUnitsSizes.size(); i++)
      {
         opt_metadata.codec_unit_metadata[opt_metadata.codec_unit_metadata_count].size = pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].pWrittenCodecUnitsSizes[i];
         opt_metadata.codec_unit_metadata[opt_metadata.codec_unit_metadata_count].offset = unpadded_frame_size;
         unpadded_frame_size += opt_metadata.codec_unit_metadata[opt_metadata.codec_unit_metadata_count].size;
         opt_metadata.codec_unit_metadata_count++;
      }
   }
   else
   {
      *output_buffer_size = 0;
      for (uint32_t i = 0; i < pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].pWrittenCodecUnitsSizes.size() ; i++) {
         unpadded_frame_size += pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].pWrittenCodecUnitsSizes[i];
         opt_metadata.codec_unit_metadata[opt_metadata.codec_unit_metadata_count].size = pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].pWrittenCodecUnitsSizes[i];
         opt_metadata.codec_unit_metadata[opt_metadata.codec_unit_metadata_count].offset = *output_buffer_size;
         *output_buffer_size += pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].pWrittenCodecUnitsSizes[i];
         opt_metadata.codec_unit_metadata_count++;
      }

      // Add padding between pre encode headers (e.g EncodeFrame driver offset alignment) and the first slice
      *output_buffer_size += pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].preEncodeGeneratedHeadersBytePadding;

      for (uint32_t i = 0; i < pSubregionsMetadata.size(); i++)
      {
         uint64_t unpadded_slice_size = pSubregionsMetadata[i].bSize - pSubregionsMetadata[i].bStartOffset;
         unpadded_frame_size += unpadded_slice_size;
         opt_metadata.codec_unit_metadata[opt_metadata.codec_unit_metadata_count].size = unpadded_slice_size;
         opt_metadata.codec_unit_metadata[opt_metadata.codec_unit_metadata_count].offset = *output_buffer_size;
         *output_buffer_size += pSubregionsMetadata[i].bSize;
         if ((pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].expected_max_slice_size > 0) &&
             (unpadded_slice_size > pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].expected_max_slice_size))
            opt_metadata.codec_unit_metadata[opt_metadata.codec_unit_metadata_count].flags |= PIPE_VIDEO_CODEC_UNIT_LOCATION_FLAG_MAX_SLICE_SIZE_OVERFLOW;
         opt_metadata.codec_unit_metadata_count++;
      }
   }

   if ((pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].expected_max_frame_size > 0) &&
      (unpadded_frame_size > pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].expected_max_frame_size))
      opt_metadata.encode_result |= PIPE_VIDEO_FEEDBACK_METADATA_ENCODE_FLAG_MAX_FRAME_SIZE_OVERFLOW;

   opt_metadata.average_frame_qp = static_cast<unsigned int>(encoderMetadata.EncodeStats.AverageQP);

   opt_metadata.present_metadata = (PIPE_VIDEO_FEEDBACK_METADATA_TYPE_BITSTREAM_SIZE |
                                    PIPE_VIDEO_FEEDBACK_METADATA_TYPE_ENCODE_RESULT |
                                    PIPE_VIDEO_FEEDBACK_METADATA_TYPE_CODEC_UNIT_LOCATION |
                                    PIPE_VIDEO_FEEDBACK_METADATA_TYPE_MAX_FRAME_SIZE_OVERFLOW |
                                    PIPE_VIDEO_FEEDBACK_METADATA_TYPE_MAX_SLICE_SIZE_OVERFLOW |
                                    PIPE_VIDEO_FEEDBACK_METADATA_TYPE_AVERAGE_FRAME_QP);

   if (pMetadata)
      *pMetadata = opt_metadata;

   debug_printf("[d3d12_video_encoder_get_feedback] Requested metadata for encoded frame at fence %" PRIu64 " is:\n"
                "\tfeedback was requested at current fence: %" PRIu64 "\n"
                "\toutput_buffer_size (including padding): %d\n"
                "\tunpadded_frame_size: %" PRIu64 "\n"
                "\ttotal padding: %" PRIu64 "\n"
                "\tcodec_unit_metadata_count: %d\n",
                pD3D12Enc->m_fenceValue,
                requested_metadata_fence,
                *output_buffer_size,
                unpadded_frame_size,
                static_cast<uint64_t>(static_cast<uint64_t>(*output_buffer_size) - unpadded_frame_size),
                opt_metadata.codec_unit_metadata_count);

   for (uint32_t i = 0; i < opt_metadata.codec_unit_metadata_count; i++) {
      debug_printf("\tcodec_unit_metadata[%d].offset: %" PRIu64" - codec_unit_metadata[%d].size: %" PRIu64" \n",
         i,
         opt_metadata.codec_unit_metadata[i].offset,
         i,
         opt_metadata.codec_unit_metadata[i].size);
   }

   pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].bRead = true;
}

unsigned
d3d12_video_encoder_build_post_encode_codec_bitstream(struct d3d12_video_encoder * pD3D12Enc,
                                             uint64_t associated_fence_value,
                                             EncodedBitstreamResolvedMetadata& associatedMetadata)
{
   enum pipe_video_format codec_format = u_reduce_video_profile(pD3D12Enc->base.profile);
   switch (codec_format) {
#if VIDEO_CODEC_H264ENC
      case PIPE_VIDEO_FORMAT_MPEG4_AVC:
      {
         return 0;
      } break;
#endif
#if VIDEO_CODEC_H265ENC
      case PIPE_VIDEO_FORMAT_HEVC:
      {
         return 0;
      } break; // Do not need post encode values in headers
#endif
#if VIDEO_CODEC_AV1ENC
      case PIPE_VIDEO_FORMAT_AV1:
      {
         return d3d12_video_encoder_build_post_encode_codec_bitstream_av1(
            // Current encoder
            pD3D12Enc,
            // associated fence value
            associated_fence_value,
            // Metadata desc
            associatedMetadata
         );
      } break;
#endif
      default:
         unreachable("Unsupported pipe_video_format");
   }
}

void
d3d12_video_encoder_extract_encode_metadata(
   struct d3d12_video_encoder *                               pD3D12Enc,
   ID3D12Resource *                                           pResolvedMetadataBuffer,   // input
   uint64_t                                                   resourceMetadataSize,      // input
   D3D12_VIDEO_ENCODER_OUTPUT_METADATA &                      parsedMetadata,            // output
   std::vector<D3D12_VIDEO_ENCODER_FRAME_SUBREGION_METADATA> &pSubregionsMetadata        // output
)
{
   struct d3d12_screen *pD3D12Screen = (struct d3d12_screen *) pD3D12Enc->m_pD3D12Screen;
   assert(pD3D12Screen);
   pipe_resource *pPipeResolvedMetadataBuffer =
      d3d12_resource_from_resource(&pD3D12Screen->base, pResolvedMetadataBuffer);
   assert(pPipeResolvedMetadataBuffer);
   assert(resourceMetadataSize < INT_MAX);
   struct pipe_box box = {
      0,                                        // x
      0,                                        // y
      0,                                        // z
      static_cast<int>(resourceMetadataSize),   // width
      1,                                        // height
      1                                         // depth
   };
   struct pipe_transfer *mapTransfer;
   unsigned mapUsage = PIPE_MAP_READ;
   void *                pMetadataBufferSrc = pD3D12Enc->base.context->buffer_map(pD3D12Enc->base.context,
                                                                  pPipeResolvedMetadataBuffer,
                                                                  0,
                                                                  mapUsage,
                                                                  &box,
                                                                  &mapTransfer);

   assert(mapUsage & PIPE_MAP_READ);
   assert(pPipeResolvedMetadataBuffer->usage == PIPE_USAGE_DEFAULT);
   // Note: As we're calling buffer_map with PIPE_MAP_READ on a pPipeResolvedMetadataBuffer which has pipe_usage_default
   // buffer_map itself will do all the synchronization and waits so once the function returns control here
   // the contents of mapTransfer are ready to be accessed.

   // Clear output
   memset(&parsedMetadata, 0, sizeof(D3D12_VIDEO_ENCODER_OUTPUT_METADATA));

   // Calculate sizes
   uint64_t encoderMetadataSize = sizeof(D3D12_VIDEO_ENCODER_OUTPUT_METADATA);

   // Copy buffer to the appropriate D3D12_VIDEO_ENCODER_OUTPUT_METADATA memory layout
   parsedMetadata = *reinterpret_cast<D3D12_VIDEO_ENCODER_OUTPUT_METADATA *>(pMetadataBufferSrc);

   // As specified in D3D12 Encode spec, the array base for metadata for the slices
   // (D3D12_VIDEO_ENCODER_FRAME_SUBREGION_METADATA[]) is placed in memory immediately after the
   // D3D12_VIDEO_ENCODER_OUTPUT_METADATA structure
   D3D12_VIDEO_ENCODER_FRAME_SUBREGION_METADATA *pFrameSubregionMetadata =
      reinterpret_cast<D3D12_VIDEO_ENCODER_FRAME_SUBREGION_METADATA *>(reinterpret_cast<uint8_t *>(pMetadataBufferSrc) +
                                                                       encoderMetadataSize);

   // Copy fields into D3D12_VIDEO_ENCODER_FRAME_SUBREGION_METADATA
   assert(parsedMetadata.WrittenSubregionsCount < SIZE_MAX);
   pSubregionsMetadata.resize(static_cast<size_t>(parsedMetadata.WrittenSubregionsCount));
   for (uint32_t sliceIdx = 0; sliceIdx < parsedMetadata.WrittenSubregionsCount; sliceIdx++) {
      pSubregionsMetadata[sliceIdx].bHeaderSize  = pFrameSubregionMetadata[sliceIdx].bHeaderSize;
      pSubregionsMetadata[sliceIdx].bSize        = pFrameSubregionMetadata[sliceIdx].bSize;
      pSubregionsMetadata[sliceIdx].bStartOffset = pFrameSubregionMetadata[sliceIdx].bStartOffset;
   }

   // Unmap the buffer tmp storage
   pipe_buffer_unmap(pD3D12Enc->base.context, mapTransfer);
   pipe_resource_reference(&pPipeResolvedMetadataBuffer, NULL);
}

/**
 * end encoding of the current frame
 */
void
d3d12_video_encoder_end_frame(struct pipe_video_codec * codec,
                              struct pipe_video_buffer *target,
                              struct pipe_picture_desc *picture)
{
   struct d3d12_video_encoder *pD3D12Enc = (struct d3d12_video_encoder *) codec;
   assert(pD3D12Enc);
   debug_printf("[d3d12_video_encoder] d3d12_video_encoder_end_frame started for fenceValue: %" PRIu64 "\n",
                 pD3D12Enc->m_fenceValue);

   if (pD3D12Enc->m_inflightResourcesPool[d3d12_video_encoder_pool_current_index(pD3D12Enc)].encode_result != PIPE_VIDEO_FEEDBACK_METADATA_ENCODE_FLAG_OK) {
      debug_printf("WARNING: [d3d12_video_encoder] d3d12_video_encoder_end_frame - Frame submission %" PRIu64 " failed. Encoder lost, please recreate pipe_video_codec object\n", pD3D12Enc->m_fenceValue);
      assert(false);
      return;
   }

   // Signal finish of current frame encoding to the picture management tracker
   pD3D12Enc->m_upDPBManager->end_frame();

   // Save extra references of Encoder, EncoderHeap and DPB allocations in case
   // there's a reconfiguration that trigers the construction of new objects
   pD3D12Enc->m_inflightResourcesPool[d3d12_video_encoder_pool_current_index(pD3D12Enc)].m_spEncoder = pD3D12Enc->m_spVideoEncoder;
   pD3D12Enc->m_inflightResourcesPool[d3d12_video_encoder_pool_current_index(pD3D12Enc)].m_spEncoderHeap = pD3D12Enc->m_spVideoEncoderHeap;
   pD3D12Enc->m_inflightResourcesPool[d3d12_video_encoder_pool_current_index(pD3D12Enc)].m_References = pD3D12Enc->m_upDPBStorageManager;

   debug_printf("[d3d12_video_encoder] d3d12_video_encoder_end_frame finalized for fenceValue: %" PRIu64 "\n",
                 pD3D12Enc->m_fenceValue);

   pD3D12Enc->m_bPendingWorkNotFlushed = true;
}

void
d3d12_video_encoder_store_current_picture_references(d3d12_video_encoder *pD3D12Enc,
                                                     uint64_t current_metadata_slot)
{
   enum pipe_video_format codec = u_reduce_video_profile(pD3D12Enc->base.profile);
   switch (codec) {
#if VIDEO_CODEC_H264ENC
      case PIPE_VIDEO_FORMAT_MPEG4_AVC:
      {
         // Not needed (not post encode headers)
      } break;
#endif
#if VIDEO_CODEC_H265ENC
      case PIPE_VIDEO_FORMAT_HEVC:
      {
         // Not needed (not post encode headers)
      } break;
#endif
#if VIDEO_CODEC_AV1ENC
      case PIPE_VIDEO_FORMAT_AV1:
      {
         d3d12_video_encoder_store_current_picture_references_av1(pD3D12Enc, current_metadata_slot);
      } break;
#endif
      default:
      {
         unreachable("Unsupported pipe_video_format");
      } break;
   }
}

struct pipe_fence_handle*
d3d12_video_encoder_get_feedback_fence(struct pipe_video_codec *codec, void *feedback)
{
   struct d3d12_video_encoder *pD3D12Enc = (struct d3d12_video_encoder *) codec;
   struct d3d12_fence *feedback_fence = (struct d3d12_fence *) feedback;
   uint64_t requested_metadata_fence = feedback_fence->value;
   uint64_t current_metadata_slot = (requested_metadata_fence % D3D12_VIDEO_ENC_METADATA_BUFFERS_COUNT);
   if (pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].encode_result & PIPE_VIDEO_FEEDBACK_METADATA_ENCODE_FLAG_FAILED) {
      debug_printf("Error: d3d12_video_encoder_get_feedback_fence failed for Encode GPU command for fence %" PRIu64 " failed on submission with encode_result: %x\n",
                     requested_metadata_fence,
                     pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].encode_result);
      assert(false);
      return NULL;
   }

   return (pipe_fence_handle*) feedback;
}

int d3d12_video_encoder_get_encode_headers([[maybe_unused]] struct pipe_video_codec *codec,
                                           [[maybe_unused]] struct pipe_picture_desc *picture,
                                           [[maybe_unused]] void* bitstream_buf,
                                           [[maybe_unused]] unsigned *bitstream_buf_size)
{
#if (VIDEO_CODEC_H264ENC || VIDEO_CODEC_H265ENC)
   struct d3d12_video_encoder *pD3D12Enc = (struct d3d12_video_encoder *) codec;
   D3D12_VIDEO_SAMPLE srcTextureDesc = {};
   srcTextureDesc.Width = pD3D12Enc->base.width;
   srcTextureDesc.Height = pD3D12Enc->base.height;
   srcTextureDesc.Format.Format = d3d12_get_format(picture->input_format);
   if(!d3d12_video_encoder_update_current_encoder_config_state(pD3D12Enc, srcTextureDesc, picture))
      return EINVAL;

   if (!pD3D12Enc->m_upBitstreamBuilder) {
#if VIDEO_CODEC_H264ENC
      if (u_reduce_video_profile(pD3D12Enc->base.profile) == PIPE_VIDEO_FORMAT_MPEG4_AVC)
         pD3D12Enc->m_upBitstreamBuilder = std::make_unique<d3d12_video_bitstream_builder_h264>();
#endif
#if VIDEO_CODEC_H265ENC
      if (u_reduce_video_profile(pD3D12Enc->base.profile) == PIPE_VIDEO_FORMAT_HEVC)
         pD3D12Enc->m_upBitstreamBuilder = std::make_unique<d3d12_video_bitstream_builder_hevc>();
#endif
   }
   bool postEncodeHeadersNeeded = false;
   uint64_t preEncodeGeneratedHeadersByteSize = 0;
   std::vector<uint64_t> pWrittenCodecUnitsSizes;
   pD3D12Enc->m_currentEncodeConfig.m_ConfigDirtyFlags |= d3d12_video_encoder_config_dirty_flag_sequence_info;
   d3d12_video_encoder_build_pre_encode_codec_headers(pD3D12Enc,
                                                      postEncodeHeadersNeeded,
                                                      preEncodeGeneratedHeadersByteSize,
                                                      pWrittenCodecUnitsSizes);
   if (preEncodeGeneratedHeadersByteSize > *bitstream_buf_size)
      return ENOMEM;

   *bitstream_buf_size = pD3D12Enc->m_BitstreamHeadersBuffer.size();
   memcpy(bitstream_buf,
          pD3D12Enc->m_BitstreamHeadersBuffer.data(),
          *bitstream_buf_size);
   return 0;
#else
   return ENOTSUP;
#endif
}

template void
d3d12_video_encoder_update_picparams_region_of_interest_qpmap(struct d3d12_video_encoder *pD3D12Enc,
                                                              const struct pipe_enc_roi *roi_config,
                                                              int32_t min_delta_qp,
                                                              int32_t max_delta_qp,
                                                              std::vector<int16_t>& pQPMap);

template void
d3d12_video_encoder_update_picparams_region_of_interest_qpmap(struct d3d12_video_encoder *pD3D12Enc,
                                                              const struct pipe_enc_roi *roi_config,
                                                              int32_t min_delta_qp,
                                                              int32_t max_delta_qp,
                                                              std::vector<int8_t>& pQPMap);

template<typename T>
void
d3d12_video_encoder_update_picparams_region_of_interest_qpmap(struct d3d12_video_encoder *pD3D12Enc,
                                                              const struct pipe_enc_roi *roi_config,
                                                              int32_t min_delta_qp,
                                                              int32_t max_delta_qp,
                                                              std::vector<T>& pQPMap)
{
   static_assert(ARRAY_SIZE(roi_config->region) == PIPE_ENC_ROI_REGION_NUM_MAX);
   assert(roi_config->num > 0);
   assert(roi_config->num <= PIPE_ENC_ROI_REGION_NUM_MAX);
   assert(min_delta_qp < 0);
   assert(max_delta_qp > 0);

   // Set all the QP blocks with zero QP Delta, then only fill in the regions that have a non-zero delta value
   uint32_t QPMapRegionPixelsSize = pD3D12Enc->m_currentEncodeCapabilities.m_currentResolutionSupportCaps.QPMapRegionPixelsSize;
   uint64_t pic_width_in_qpmap_block_units = static_cast<uint64_t>(std::ceil(pD3D12Enc->m_currentEncodeConfig.m_currentResolution.Width /
      static_cast<double>(QPMapRegionPixelsSize)));
   uint64_t pic_height_in_qpmap_block_units = static_cast<uint64_t>(std::ceil(pD3D12Enc->m_currentEncodeConfig.m_currentResolution.Height /
      static_cast<double>(QPMapRegionPixelsSize)));
   uint64_t total_picture_qpmap_block_units = pic_width_in_qpmap_block_units * pic_height_in_qpmap_block_units;
   pQPMap.resize(total_picture_qpmap_block_units, 0u);

   // Loop in reverse for priority of overlapping regions as per p_video_state roi parameter docs
   for (int32_t i = (roi_config->num - 1); i >= 0 ; i--)
   {
      auto& cur_region = roi_config->region[i];
      if (cur_region.valid)
      {
         uint32_t bucket_start_block_x = cur_region.x / QPMapRegionPixelsSize;
         uint32_t bucket_start_block_y = cur_region.y / QPMapRegionPixelsSize;
         uint32_t bucket_end_block_x = std::ceil((cur_region.x + cur_region.width) / static_cast<double>(QPMapRegionPixelsSize)) - 1;
         uint32_t bucket_end_block_y = std::ceil((cur_region.y + cur_region.height) / static_cast<double>(QPMapRegionPixelsSize)) - 1;
         for (uint32_t i = bucket_start_block_x; i <= bucket_end_block_x; i++)
            for (uint32_t j = bucket_start_block_y; j <= bucket_end_block_y; j++)
               pQPMap[(j * pic_width_in_qpmap_block_units) + i] = CLAMP(cur_region.qp_value, min_delta_qp, max_delta_qp);
      }
   }
}

