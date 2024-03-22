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

#include "d3d12_video_enc.h"
#include "d3d12_video_enc_hevc.h"
#include "util/u_video.h"
#include "d3d12_screen.h"
#include "d3d12_format.h"

#include <cmath>
#include <algorithm>
#include <numeric>

void
d3d12_video_encoder_update_current_rate_control_hevc(struct d3d12_video_encoder *pD3D12Enc,
                                                     pipe_h265_enc_picture_desc *picture)
{
   pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc = {};
   pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_FrameRate.Numerator =
      picture->rc.frame_rate_num;
   pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_FrameRate.Denominator =
      picture->rc.frame_rate_den;
   pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Flags = D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_NONE;

   if (picture->roi.num > 0)
      pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Flags |=
         D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_DELTA_QP;

   switch (picture->rc.rate_ctrl_method) {
      case PIPE_H2645_ENC_RATE_CONTROL_METHOD_VARIABLE_SKIP:
      case PIPE_H2645_ENC_RATE_CONTROL_METHOD_VARIABLE:
      {
         pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Mode = D3D12_VIDEO_ENCODER_RATE_CONTROL_MODE_VBR;
         pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_VBR.TargetAvgBitRate =
            picture->rc.target_bitrate;
         pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_VBR.PeakBitRate =
            picture->rc.peak_bitrate;

         if (D3D12_VIDEO_ENC_CBR_FORCE_VBV_EQUAL_BITRATE) {
            debug_printf("[d3d12_video_encoder_hevc] d3d12_video_encoder_update_current_rate_control_hevc D3D12_VIDEO_ENC_CBR_FORCE_VBV_EQUAL_BITRATE environment variable is set, "
                       ", forcing VBV Size = VBV Initial Capacity = Target Bitrate = %" PRIu64 " (bits)\n", pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_CBR.TargetBitRate);
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Flags |=
               D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_VBV_SIZES;
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_CBR.VBVCapacity =
               pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_CBR.TargetBitRate;
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_CBR.InitialVBVFullness =
               pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_CBR.TargetBitRate;
         } else if (picture->rc.app_requested_hrd_buffer) {
            debug_printf("[d3d12_video_encoder_hevc] d3d12_video_encoder_update_current_rate_control_hevc HRD required by app,"
                       " setting VBV Size = %d (bits) - VBV Initial Capacity %d (bits)\n", picture->rc.vbv_buffer_size, picture->rc.vbv_buf_initial_size);
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Flags |=
               D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_VBV_SIZES;
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_VBR.VBVCapacity =
               picture->rc.vbv_buffer_size;
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_VBR.InitialVBVFullness =
               picture->rc.vbv_buf_initial_size;
         }

         pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.max_frame_size = picture->rc.max_au_size;
         if (picture->rc.max_au_size > 0) {
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Flags |=
               D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_MAX_FRAME_SIZE;
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_VBR.MaxFrameBitSize =
               picture->rc.max_au_size;

            debug_printf(
               "[d3d12_video_encoder_hevc] d3d12_video_encoder_update_current_rate_control_hevc "
               "Upper layer requested explicit MaxFrameBitSize: %" PRIu64 "\n",
               pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_VBR.MaxFrameBitSize);
         }

         if (picture->rc.app_requested_qp_range) {
            debug_printf(
               "[d3d12_video_encoder_hevc] d3d12_video_encoder_update_current_rate_control_hevc "
               "Upper layer requested explicit MinQP: %d MaxQP: %d\n",
               picture->rc.min_qp, picture->rc.max_qp);
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Flags |=
               D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_QP_RANGE;
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_VBR.MinQP =
               picture->rc.min_qp;
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_VBR.MaxQP =
               picture->rc.max_qp;
         }

         if (picture->quality_modes.level > 0) {
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Flags |=
               D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_QUALITY_VS_SPEED;
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Flags |=
               D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_EXTENSION1_SUPPORT;

            // Convert between D3D12 definition and PIPE definition
            // D3D12: QualityVsSpeed must be in the range [0, D3D12_FEATURE_DATA_VIDEO_ENCODER_SUPPORT1.MaxQualityVsSpeed]
            // The lower the value, the fastest the encode operation
            // PIPE: The quality level range can be queried through the VAConfigAttribEncQualityRange attribute. 
            // A lower value means higher quality, and a value of 1 represents the highest quality. 
            // The quality level setting is used as a trade-off between quality and speed/power 
            // consumption, with higher quality corresponds to lower speed and higher power consumption.

            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_VBR1.QualityVsSpeed =
               pD3D12Enc->max_quality_levels - picture->quality_modes.level;
         }

      } break;
      case PIPE_H2645_ENC_RATE_CONTROL_METHOD_QUALITY_VARIABLE:
      {
         pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Mode = D3D12_VIDEO_ENCODER_RATE_CONTROL_MODE_QVBR;
         pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_QVBR.TargetAvgBitRate =
            picture->rc.target_bitrate;
         pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_QVBR.PeakBitRate =
            picture->rc.peak_bitrate;
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_QVBR.ConstantQualityTarget =
            picture->rc.vbr_quality_factor;
         if (D3D12_VIDEO_ENC_CBR_FORCE_VBV_EQUAL_BITRATE) {
            debug_printf("[d3d12_video_encoder_hevc] d3d12_video_encoder_update_current_rate_control_hevc D3D12_VIDEO_ENC_CBR_FORCE_VBV_EQUAL_BITRATE environment variable is set, "
                       ", forcing VBV Size = VBV Initial Capacity = Target Bitrate = %" PRIu64 " (bits)\n", pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_QVBR1.TargetAvgBitRate);
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Flags |=
               D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_VBV_SIZES;
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Flags |=
               D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_EXTENSION1_SUPPORT;
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_QVBR1.VBVCapacity =
               pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_QVBR1.TargetAvgBitRate;
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_QVBR1.InitialVBVFullness =
               pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_QVBR1.TargetAvgBitRate;
         } else if (picture->rc.app_requested_hrd_buffer) {
            debug_printf("[d3d12_video_encoder_hevc] d3d12_video_encoder_update_current_rate_control_hevc HRD required by app,"
                       " setting VBV Size = %d (bits) - VBV Initial Capacity %d (bits)\n", picture->rc.vbv_buffer_size, picture->rc.vbv_buf_initial_size);
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Flags |=
               D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_VBV_SIZES;
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Flags |=
               D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_EXTENSION1_SUPPORT;
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_QVBR1.VBVCapacity =
               picture->rc.vbv_buffer_size;
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_QVBR1.InitialVBVFullness =
               picture->rc.vbv_buf_initial_size;
         }
         pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.max_frame_size = picture->rc.max_au_size;
         if (picture->rc.max_au_size > 0) {
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Flags |=
               D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_MAX_FRAME_SIZE;
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_QVBR.MaxFrameBitSize =
               picture->rc.max_au_size;

            debug_printf(
               "[d3d12_video_encoder_hevc] d3d12_video_encoder_update_current_rate_control_hevc "
               "Upper layer requested explicit MaxFrameBitSize: %" PRIu64 "\n",
               pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_QVBR.MaxFrameBitSize);
         }

         if (picture->rc.app_requested_qp_range) {
            debug_printf(
               "[d3d12_video_encoder_hevc] d3d12_video_encoder_update_current_rate_control_hevc "
               "Upper layer requested explicit MinQP: %d MaxQP: %d\n",
               picture->rc.min_qp, picture->rc.max_qp);
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Flags |=
               D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_QP_RANGE;
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_QVBR.MinQP =
               picture->rc.min_qp;
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_QVBR.MaxQP =
               picture->rc.max_qp;
         }

         if (picture->quality_modes.level > 0) {
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Flags |=
               D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_QUALITY_VS_SPEED;
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Flags |=
               D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_EXTENSION1_SUPPORT;

            // Convert between D3D12 definition and PIPE definition
            // D3D12: QualityVsSpeed must be in the range [0, D3D12_FEATURE_DATA_VIDEO_ENCODER_SUPPORT1.MaxQualityVsSpeed]
            // The lower the value, the fastest the encode operation
            // PIPE: The quality level range can be queried through the VAConfigAttribEncQualityRange attribute. 
            // A lower value means higher quality, and a value of 1 represents the highest quality. 
            // The quality level setting is used as a trade-off between quality and speed/power 
            // consumption, with higher quality corresponds to lower speed and higher power consumption.

            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_QVBR1.QualityVsSpeed =
               pD3D12Enc->max_quality_levels - picture->quality_modes.level;
         }

      } break;
      case PIPE_H2645_ENC_RATE_CONTROL_METHOD_CONSTANT_SKIP:
      case PIPE_H2645_ENC_RATE_CONTROL_METHOD_CONSTANT:
      {
         pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Mode = D3D12_VIDEO_ENCODER_RATE_CONTROL_MODE_CBR;
         pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_CBR.TargetBitRate =
            picture->rc.target_bitrate;

         /* For CBR mode, to guarantee bitrate of generated stream complies with
          * target bitrate (e.g. no over +/-10%), vbv_buffer_size and initial capacity should be same
          * as target bitrate. Controlled by OS env var D3D12_VIDEO_ENC_CBR_FORCE_VBV_EQUAL_BITRATE
          */
         if (D3D12_VIDEO_ENC_CBR_FORCE_VBV_EQUAL_BITRATE) {
            debug_printf("[d3d12_video_encoder_hevc] d3d12_video_encoder_update_current_rate_control_hevc D3D12_VIDEO_ENC_CBR_FORCE_VBV_EQUAL_BITRATE environment variable is set, "
                       ", forcing VBV Size = VBV Initial Capacity = Target Bitrate = %" PRIu64 " (bits)\n", pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_CBR.TargetBitRate);
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Flags |=
               D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_VBV_SIZES;
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_CBR.VBVCapacity =
               pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_CBR.TargetBitRate;
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_CBR.InitialVBVFullness =
               pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_CBR.TargetBitRate;
         } else if (picture->rc.app_requested_hrd_buffer) {
            debug_printf("[d3d12_video_encoder_hevc] d3d12_video_encoder_update_current_rate_control_hevc HRD required by app,"
                       " setting VBV Size = %d (bits) - VBV Initial Capacity %d (bits)\n", picture->rc.vbv_buffer_size, picture->rc.vbv_buf_initial_size);
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Flags |=
               D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_VBV_SIZES;
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_CBR.VBVCapacity =
               picture->rc.vbv_buffer_size;
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_CBR.InitialVBVFullness =
               picture->rc.vbv_buf_initial_size;
         }

         pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.max_frame_size = picture->rc.max_au_size;
         if (picture->rc.max_au_size > 0) {
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Flags |=
               D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_MAX_FRAME_SIZE;
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_CBR.MaxFrameBitSize =
               picture->rc.max_au_size;

            debug_printf(
               "[d3d12_video_encoder_hevc] d3d12_video_encoder_update_current_rate_control_hevc "
               "Upper layer requested explicit MaxFrameBitSize: %" PRIu64 "\n",
               pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_CBR.MaxFrameBitSize);
         }

         if (picture->rc.app_requested_qp_range) {
            debug_printf(
               "[d3d12_video_encoder_hevc] d3d12_video_encoder_update_current_rate_control_hevc "
               "Upper layer requested explicit MinQP: %d MaxQP: %d\n",
               picture->rc.min_qp, picture->rc.max_qp);
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Flags |=
               D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_QP_RANGE;
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_CBR.MinQP =
               picture->rc.min_qp;
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_CBR.MaxQP =
               picture->rc.max_qp;
         }

         if (picture->quality_modes.level > 0) {
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Flags |=
               D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_QUALITY_VS_SPEED;
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Flags |=
               D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_EXTENSION1_SUPPORT;

            // Convert between D3D12 definition and PIPE definition
            // D3D12: QualityVsSpeed must be in the range [0, D3D12_FEATURE_DATA_VIDEO_ENCODER_SUPPORT1.MaxQualityVsSpeed]
            // The lower the value, the fastest the encode operation
            // PIPE: The quality level range can be queried through the VAConfigAttribEncQualityRange attribute. 
            // A lower value means higher quality, and a value of 1 represents the highest quality. 
            // The quality level setting is used as a trade-off between quality and speed/power 
            // consumption, with higher quality corresponds to lower speed and higher power consumption.

            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_CBR1.QualityVsSpeed =
               pD3D12Enc->max_quality_levels - picture->quality_modes.level;
         }

      } break;
      case PIPE_H2645_ENC_RATE_CONTROL_METHOD_DISABLE:
      {
         pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Mode = D3D12_VIDEO_ENCODER_RATE_CONTROL_MODE_CQP;
         pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_CQP
            .ConstantQP_FullIntracodedFrame = picture->rc.quant_i_frames;
         pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_CQP
            .ConstantQP_InterPredictedFrame_PrevRefOnly = picture->rc.quant_p_frames;
         pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_CQP
            .ConstantQP_InterPredictedFrame_BiDirectionalRef = picture->rc.quant_b_frames;

         if (picture->quality_modes.level > 0) {
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Flags |=
               D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_QUALITY_VS_SPEED;
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Flags |=
               D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_EXTENSION1_SUPPORT;

            // Convert between D3D12 definition and PIPE definition
            // D3D12: QualityVsSpeed must be in the range [0, D3D12_FEATURE_DATA_VIDEO_ENCODER_SUPPORT1.MaxQualityVsSpeed]
            // The lower the value, the fastest the encode operation
            // PIPE: The quality level range can be queried through the VAConfigAttribEncQualityRange attribute. 
            // A lower value means higher quality, and a value of 1 represents the highest quality. 
            // The quality level setting is used as a trade-off between quality and speed/power 
            // consumption, with higher quality corresponds to lower speed and higher power consumption.

            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_CQP1.QualityVsSpeed =
               pD3D12Enc->max_quality_levels - picture->quality_modes.level;
         }
      } break;
      default:
      {
         debug_printf("[d3d12_video_encoder_hevc] d3d12_video_encoder_update_current_rate_control_hevc invalid RC "
                       "config, using default RC CQP mode\n");
         pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Mode = D3D12_VIDEO_ENCODER_RATE_CONTROL_MODE_CQP;
         pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_CQP
            .ConstantQP_FullIntracodedFrame = 30;
         pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_CQP
            .ConstantQP_InterPredictedFrame_PrevRefOnly = 30;
         pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_CQP
            .ConstantQP_InterPredictedFrame_BiDirectionalRef = 30;
      } break;
   }
}

void
d3d12_video_encoder_update_current_frame_pic_params_info_hevc(struct d3d12_video_encoder *pD3D12Enc,
                                                              struct pipe_video_buffer *srcTexture,
                                                              struct pipe_picture_desc *picture,
                                                              D3D12_VIDEO_ENCODER_PICTURE_CONTROL_CODEC_DATA &picParams,
                                                              bool &bUsedAsReference)
{
   struct pipe_h265_enc_picture_desc *hevcPic = (struct pipe_h265_enc_picture_desc *) picture;
   d3d12_video_bitstream_builder_hevc *pHEVCBitstreamBuilder =
      static_cast<d3d12_video_bitstream_builder_hevc *>(pD3D12Enc->m_upBitstreamBuilder.get());
   assert(pHEVCBitstreamBuilder != nullptr);

   bUsedAsReference = !hevcPic->not_referenced;

   picParams.pHEVCPicData->slice_pic_parameter_set_id = pHEVCBitstreamBuilder->get_active_pps_id();
   picParams.pHEVCPicData->FrameType = d3d12_video_encoder_convert_frame_type_hevc(hevcPic->picture_type);
   picParams.pHEVCPicData->PictureOrderCountNumber = hevcPic->pic_order_cnt;

   picParams.pHEVCPicData->List0ReferenceFramesCount = 0;
   picParams.pHEVCPicData->pList0ReferenceFrames = nullptr;
   picParams.pHEVCPicData->List1ReferenceFramesCount = 0;
   picParams.pHEVCPicData->pList1ReferenceFrames = nullptr;

   if (picParams.pHEVCPicData->FrameType == D3D12_VIDEO_ENCODER_FRAME_TYPE_HEVC_P_FRAME) {
      picParams.pHEVCPicData->List0ReferenceFramesCount = hevcPic->num_ref_idx_l0_active_minus1 + 1;
      picParams.pHEVCPicData->pList0ReferenceFrames = hevcPic->ref_idx_l0_list;
   } else if (picParams.pHEVCPicData->FrameType == D3D12_VIDEO_ENCODER_FRAME_TYPE_HEVC_B_FRAME) {
      picParams.pHEVCPicData->List0ReferenceFramesCount = hevcPic->num_ref_idx_l0_active_minus1 + 1;
      picParams.pHEVCPicData->pList0ReferenceFrames = hevcPic->ref_idx_l0_list;
      picParams.pHEVCPicData->List1ReferenceFramesCount = hevcPic->num_ref_idx_l1_active_minus1 + 1;
      picParams.pHEVCPicData->pList1ReferenceFrames = hevcPic->ref_idx_l1_list;
   }

   if ((pD3D12Enc->m_currentEncodeConfig.m_encoderCodecSpecificConfigDesc.m_HEVCConfig.ConfigurationFlags 
      & D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC_FLAG_ALLOW_REQUEST_INTRA_CONSTRAINED_SLICES) != 0)
      picParams.pHEVCPicData->Flags |= D3D12_VIDEO_ENCODER_PICTURE_CONTROL_CODEC_DATA_HEVC_FLAG_REQUEST_INTRA_CONSTRAINED_SLICES;

   if ((pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Flags & D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_DELTA_QP) != 0)
   {
      // Use 8 bit qpmap array for HEVC picparams (-51, 51 range and int8_t pRateControlQPMap type)
      const int32_t hevc_min_delta_qp = -51;
      const int32_t hevc_max_delta_qp = 51;
      d3d12_video_encoder_update_picparams_region_of_interest_qpmap(
         pD3D12Enc,
         &hevcPic->roi,
         hevc_min_delta_qp,
         hevc_max_delta_qp,
         pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_pRateControlQPMap8Bit);
      picParams.pHEVCPicData->pRateControlQPMap = pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_pRateControlQPMap8Bit.data();
      picParams.pHEVCPicData->QPMapValuesCount = pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_pRateControlQPMap8Bit.size();
   }
}

D3D12_VIDEO_ENCODER_FRAME_TYPE_HEVC
d3d12_video_encoder_convert_frame_type_hevc(enum pipe_h2645_enc_picture_type picType)
{
   switch (picType) {
      case PIPE_H2645_ENC_PICTURE_TYPE_P:
      {
         return D3D12_VIDEO_ENCODER_FRAME_TYPE_HEVC_P_FRAME;
      } break;
      case PIPE_H2645_ENC_PICTURE_TYPE_B:
      {
         return D3D12_VIDEO_ENCODER_FRAME_TYPE_HEVC_B_FRAME;
      } break;
      case PIPE_H2645_ENC_PICTURE_TYPE_I:
      {
         return D3D12_VIDEO_ENCODER_FRAME_TYPE_HEVC_I_FRAME;
      } break;
      case PIPE_H2645_ENC_PICTURE_TYPE_IDR:
      {
         return D3D12_VIDEO_ENCODER_FRAME_TYPE_HEVC_IDR_FRAME;
      } break;
      default:
      {
         unreachable("Unsupported pipe_h2645_enc_picture_type");
      } break;
   }
}

///
/// Tries to configurate the encoder using the requested slice configuration
/// or falls back to single slice encoding.
///
bool
d3d12_video_encoder_negotiate_current_hevc_slices_configuration(struct d3d12_video_encoder *pD3D12Enc,
                                                                pipe_h265_enc_picture_desc *picture)
{
   ///
   /// Initialize single slice by default
   ///
   D3D12_VIDEO_ENCODER_FRAME_SUBREGION_LAYOUT_MODE requestedSlicesMode =
      D3D12_VIDEO_ENCODER_FRAME_SUBREGION_LAYOUT_MODE_FULL_FRAME;
   D3D12_VIDEO_ENCODER_PICTURE_CONTROL_SUBREGIONS_LAYOUT_DATA_SLICES requestedSlicesConfig = {};
   requestedSlicesConfig.NumberOfSlicesPerFrame = 1;

   ///
   /// Try to see if can accomodate for multi-slice request by user
   ///
   if ((picture->slice_mode == PIPE_VIDEO_SLICE_MODE_BLOCKS) && (picture->num_slice_descriptors > 1)) {
      /* Some apps send all same size slices minus 1 slice in any position in the descriptors */
      /* Lets validate that there are at most 2 different slice sizes in all the descriptors */
      std::vector<int> slice_sizes(picture->num_slice_descriptors);
      for (uint32_t i = 0; i < picture->num_slice_descriptors; i++)
         slice_sizes[i] = picture->slices_descriptors[i].num_ctu_in_slice;
      std::sort(slice_sizes.begin(), slice_sizes.end());
      bool bUniformSizeSlices = (std::unique(slice_sizes.begin(), slice_sizes.end()) - slice_sizes.begin()) <= 2;

      uint32_t subregion_block_pixel_size = pD3D12Enc->m_currentEncodeCapabilities.m_currentResolutionSupportCaps.SubregionBlockPixelsSize;
      uint32_t num_subregions_per_scanline =
         DIV_ROUND_UP(pD3D12Enc->m_currentEncodeConfig.m_currentResolution.Width, subregion_block_pixel_size);

      /* m_currentResolutionSupportCaps.SubregionBlockPixelsSize can be a multiple of MinCUSize to accomodate for HW requirements 
         So, if the allowed subregion (slice) pixel size partition is bigger (a multiple) than the CTU size, we have to adjust
         num_subregions_per_slice by this factor respect from slices_descriptors[X].num_ctu_in_slice
      */

      /* This assert should always be true according to the spec
         https://github.com/microsoft/DirectX-Specs/blob/master/d3d/D3D12VideoEncoding.md#3150-struct-d3d12_feature_data_video_encoder_resolution_support_limits
       */
      uint8_t minCUSize = d3d12_video_encoder_convert_12cusize_to_pixel_size_hevc(pD3D12Enc->m_currentEncodeConfig.m_encoderCodecSpecificConfigDesc.m_HEVCConfig.MinLumaCodingUnitSize);
      assert((pD3D12Enc->m_currentEncodeCapabilities.m_currentResolutionSupportCaps.SubregionBlockPixelsSize 
         % minCUSize) == 0);

      uint32_t subregionsize_to_ctu_factor = pD3D12Enc->m_currentEncodeCapabilities.m_currentResolutionSupportCaps.SubregionBlockPixelsSize / 
         minCUSize;
      uint32_t num_subregions_per_slice = picture->slices_descriptors[0].num_ctu_in_slice
                                                   * pD3D12Enc->m_currentEncodeCapabilities.m_currentResolutionSupportCaps.SubregionBlockPixelsSize
                                                   / (subregionsize_to_ctu_factor * subregionsize_to_ctu_factor);

      bool bSliceAligned = ((num_subregions_per_slice % num_subregions_per_scanline) == 0);

      if (bUniformSizeSlices &&
                 d3d12_video_encoder_check_subregion_mode_support(
                    pD3D12Enc,
                    D3D12_VIDEO_ENCODER_FRAME_SUBREGION_LAYOUT_MODE_UNIFORM_PARTITIONING_SUBREGIONS_PER_FRAME)) {
            requestedSlicesMode =
               D3D12_VIDEO_ENCODER_FRAME_SUBREGION_LAYOUT_MODE_UNIFORM_PARTITIONING_SUBREGIONS_PER_FRAME;
            requestedSlicesConfig.NumberOfSlicesPerFrame = picture->num_slice_descriptors;
            debug_printf("[d3d12_video_encoder_hevc] Using multi slice encoding mode: "
                           "D3D12_VIDEO_ENCODER_FRAME_SUBREGION_LAYOUT_MODE_UNIFORM_PARTITIONING_SUBREGIONS_PER_FRAME "
                           "with %d slices per frame.\n",
                           requestedSlicesConfig.NumberOfSlicesPerFrame);
      } else if (bUniformSizeSlices &&
                 d3d12_video_encoder_check_subregion_mode_support(
                    pD3D12Enc,
                    D3D12_VIDEO_ENCODER_FRAME_SUBREGION_LAYOUT_MODE_SQUARE_UNITS_PER_SUBREGION_ROW_UNALIGNED)) {
            requestedSlicesMode =
               D3D12_VIDEO_ENCODER_FRAME_SUBREGION_LAYOUT_MODE_SQUARE_UNITS_PER_SUBREGION_ROW_UNALIGNED;
            requestedSlicesConfig.NumberOfCodingUnitsPerSlice = num_subregions_per_slice;
            debug_printf("[d3d12_video_encoder_hevc] Using multi slice encoding mode: "
                           "D3D12_VIDEO_ENCODER_FRAME_SUBREGION_LAYOUT_MODE_SQUARE_UNITS_PER_SUBREGION_ROW_UNALIGNED "
                           "with %d NumberOfCodingUnitsPerSlice per frame.\n",
                           requestedSlicesConfig.NumberOfCodingUnitsPerSlice);

      } else if (bUniformSizeSlices && bSliceAligned &&
                 d3d12_video_encoder_check_subregion_mode_support(
                    pD3D12Enc,
                    D3D12_VIDEO_ENCODER_FRAME_SUBREGION_LAYOUT_MODE_UNIFORM_PARTITIONING_ROWS_PER_SUBREGION)) {

         // Number of subregion block per slice is aligned to a scanline width, in which case we can
         // use D3D12_VIDEO_ENCODER_FRAME_SUBREGION_LAYOUT_MODE_UNIFORM_PARTITIONING_ROWS_PER_SUBREGION
         requestedSlicesMode = D3D12_VIDEO_ENCODER_FRAME_SUBREGION_LAYOUT_MODE_UNIFORM_PARTITIONING_ROWS_PER_SUBREGION;
         requestedSlicesConfig.NumberOfRowsPerSlice = (num_subregions_per_slice / num_subregions_per_scanline);
         debug_printf("[d3d12_video_encoder_hevc] Using multi slice encoding mode: "
                        "D3D12_VIDEO_ENCODER_FRAME_SUBREGION_LAYOUT_MODE_UNIFORM_PARTITIONING_ROWS_PER_SUBREGION with "
                        "%d subregion block rows (%d pix scanlines) per slice.\n",
                        requestedSlicesConfig.NumberOfRowsPerSlice,
                        pD3D12Enc->m_currentEncodeCapabilities.m_currentResolutionSupportCaps.SubregionBlockPixelsSize);
      } else {
         debug_printf("[d3d12_video_encoder_hevc] Requested slice control mode is not supported: All slices must "
                         "have the same number of macroblocks.\n");
         return false;
      }
   } else if(picture->slice_mode == PIPE_VIDEO_SLICE_MODE_MAX_SLICE_SICE) {
      if ((picture->max_slice_bytes > 0) &&
                 d3d12_video_encoder_check_subregion_mode_support(
                    pD3D12Enc,
                    D3D12_VIDEO_ENCODER_FRAME_SUBREGION_LAYOUT_MODE_BYTES_PER_SUBREGION )) {
            requestedSlicesMode =
               D3D12_VIDEO_ENCODER_FRAME_SUBREGION_LAYOUT_MODE_BYTES_PER_SUBREGION;
            requestedSlicesConfig.MaxBytesPerSlice = picture->max_slice_bytes;
            debug_printf("[d3d12_video_encoder_hevc] Using multi slice encoding mode: "
                           "D3D12_VIDEO_ENCODER_FRAME_SUBREGION_LAYOUT_MODE_BYTES_PER_SUBREGION  "
                           "with %d MaxBytesPerSlice per frame.\n",
                           requestedSlicesConfig.MaxBytesPerSlice);
      } else {
         debug_printf("[d3d12_video_encoder_hevc] Requested slice control mode is not supported: All slices must "
                         "have the same number of macroblocks.\n");
         return false;
      }
   } else {
      requestedSlicesMode = D3D12_VIDEO_ENCODER_FRAME_SUBREGION_LAYOUT_MODE_FULL_FRAME;
      requestedSlicesConfig.NumberOfSlicesPerFrame = 1;
      debug_printf("[d3d12_video_encoder_hevc] Requested slice control mode is full frame. m_SlicesPartition_H264.NumberOfSlicesPerFrame = %d - m_encoderSliceConfigMode = %d \n",
      requestedSlicesConfig.NumberOfSlicesPerFrame, requestedSlicesMode);
   }

   if (!d3d12_video_encoder_isequal_slice_config_hevc(
          pD3D12Enc->m_currentEncodeConfig.m_encoderSliceConfigMode,
          pD3D12Enc->m_currentEncodeConfig.m_encoderSliceConfigDesc.m_SlicesPartition_HEVC,
          requestedSlicesMode,
          requestedSlicesConfig)) {
      pD3D12Enc->m_currentEncodeConfig.m_ConfigDirtyFlags |= d3d12_video_encoder_config_dirty_flag_slices;
   }

   pD3D12Enc->m_currentEncodeConfig.m_encoderSliceConfigDesc.m_SlicesPartition_HEVC = requestedSlicesConfig;
   pD3D12Enc->m_currentEncodeConfig.m_encoderSliceConfigMode = requestedSlicesMode;

   return true;
}

D3D12_VIDEO_ENCODER_MOTION_ESTIMATION_PRECISION_MODE
d3d12_video_encoder_convert_hevc_motion_configuration(struct d3d12_video_encoder *pD3D12Enc,
                                                      pipe_h265_enc_picture_desc *picture)
{
   return D3D12_VIDEO_ENCODER_MOTION_ESTIMATION_PRECISION_MODE_MAXIMUM;
}

bool
d3d12_video_encoder_update_hevc_gop_configuration(struct d3d12_video_encoder *pD3D12Enc,
                                                  pipe_h265_enc_picture_desc *picture)
{
   // Only update GOP when it begins
   // This triggers DPB/encoder/heap re-creation, so only check on IDR when a GOP might change
   if ((picture->picture_type == PIPE_H2645_ENC_PICTURE_TYPE_IDR)
      || (picture->picture_type == PIPE_H2645_ENC_PICTURE_TYPE_I)) {
      uint32_t GOPLength = picture->seq.intra_period;
      uint32_t PPicturePeriod = picture->seq.ip_period;

      const uint32_t max_pic_order_cnt_lsb = MAX2(16, util_next_power_of_two(GOPLength));
      double log2_max_pic_order_cnt_lsb_minus4 = std::max(0.0, std::ceil(std::log2(max_pic_order_cnt_lsb)) - 4);
      assert(log2_max_pic_order_cnt_lsb_minus4 < UCHAR_MAX);

      // Set dirty flag if m_HEVCGroupOfPictures changed
      auto previousGOPConfig = pD3D12Enc->m_currentEncodeConfig.m_encoderGOPConfigDesc.m_HEVCGroupOfPictures;
      pD3D12Enc->m_currentEncodeConfig.m_encoderGOPConfigDesc.m_HEVCGroupOfPictures = {
         GOPLength,
         PPicturePeriod,
         static_cast<uint8_t>(log2_max_pic_order_cnt_lsb_minus4)
      };

      if (memcmp(&previousGOPConfig,
                 &pD3D12Enc->m_currentEncodeConfig.m_encoderGOPConfigDesc.m_HEVCGroupOfPictures,
                 sizeof(D3D12_VIDEO_ENCODER_SEQUENCE_GOP_STRUCTURE_HEVC)) != 0) {
         pD3D12Enc->m_currentEncodeConfig.m_ConfigDirtyFlags |= d3d12_video_encoder_config_dirty_flag_gop;
      }
   }
   return true;
}

D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC
d3d12_video_encoder_convert_hevc_codec_configuration(struct d3d12_video_encoder *pD3D12Enc,
                                                     pipe_h265_enc_picture_desc *picture,
                                                     bool&is_supported)
{
   is_supported = true;
   uint32_t min_cu_size = (1 << (picture->seq.log2_min_luma_coding_block_size_minus3 + 3));
   uint32_t max_cu_size = (1 << (picture->seq.log2_min_luma_coding_block_size_minus3 + 3 + picture->seq.log2_diff_max_min_luma_coding_block_size));

   uint32_t min_tu_size = (1 << (picture->seq.log2_min_transform_block_size_minus2 + 2));
   uint32_t max_tu_size = (1 << (picture->seq.log2_min_transform_block_size_minus2 + 2 + picture->seq.log2_diff_max_min_transform_block_size));

   D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC config = {
      D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC_FLAG_NONE,
      d3d12_video_encoder_convert_pixel_size_hevc_to_12cusize(min_cu_size),
      d3d12_video_encoder_convert_pixel_size_hevc_to_12cusize(max_cu_size),
      d3d12_video_encoder_convert_pixel_size_hevc_to_12tusize(min_tu_size),
      d3d12_video_encoder_convert_pixel_size_hevc_to_12tusize(max_tu_size),
      picture->seq.max_transform_hierarchy_depth_inter,
      picture->seq.max_transform_hierarchy_depth_intra,
   };

   pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_HEVCCodecCaps = 
   {
         D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_SUPPORT_HEVC_FLAG_NONE,
         config.MinLumaCodingUnitSize,
         config.MaxLumaCodingUnitSize,
         config.MinLumaTransformUnitSize,
         config.MaxLumaTransformUnitSize,
         config.max_transform_hierarchy_depth_inter,
         config.max_transform_hierarchy_depth_intra
   };

   D3D12_FEATURE_DATA_VIDEO_ENCODER_CODEC_CONFIGURATION_SUPPORT capCodecConfigData = { };
   capCodecConfigData.NodeIndex = pD3D12Enc->m_NodeIndex;
   capCodecConfigData.Codec = D3D12_VIDEO_ENCODER_CODEC_HEVC;
   D3D12_VIDEO_ENCODER_PROFILE_HEVC prof = d3d12_video_encoder_convert_profile_to_d3d12_enc_profile_hevc(pD3D12Enc->base.profile);
   capCodecConfigData.Profile.pHEVCProfile = &prof;
   capCodecConfigData.Profile.DataSize = sizeof(prof);
   capCodecConfigData.CodecSupportLimits.pHEVCSupport = &pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_HEVCCodecCaps;
   capCodecConfigData.CodecSupportLimits.DataSize = sizeof(pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_HEVCCodecCaps);

   if(FAILED(pD3D12Enc->m_spD3D12VideoDevice->CheckFeatureSupport(D3D12_FEATURE_VIDEO_ENCODER_CODEC_CONFIGURATION_SUPPORT, &capCodecConfigData, sizeof(capCodecConfigData)))
      || !capCodecConfigData.IsSupported)
   {
         is_supported = false;

         // Workaround for https://github.com/intel/libva/issues/641
         if ( !capCodecConfigData.IsSupported
            && ((picture->seq.max_transform_hierarchy_depth_inter == 0)
               || (picture->seq.max_transform_hierarchy_depth_intra == 0)) )
         {
            // Try and see if the values where 4 and overflowed in the 2 bit fields
            capCodecConfigData.CodecSupportLimits.pHEVCSupport->max_transform_hierarchy_depth_inter =
               (picture->seq.max_transform_hierarchy_depth_inter == 0) ? 4 : picture->seq.max_transform_hierarchy_depth_inter;
            capCodecConfigData.CodecSupportLimits.pHEVCSupport->max_transform_hierarchy_depth_intra =
               (picture->seq.max_transform_hierarchy_depth_intra == 0) ? 4 : picture->seq.max_transform_hierarchy_depth_intra;

            // Call the caps check again
            if(SUCCEEDED(pD3D12Enc->m_spD3D12VideoDevice->CheckFeatureSupport(D3D12_FEATURE_VIDEO_ENCODER_CODEC_CONFIGURATION_SUPPORT, &capCodecConfigData, sizeof(capCodecConfigData)))
               && capCodecConfigData.IsSupported)
            {
               // If this was the case, then update the config return variable with the overriden values too
               is_supported = true;
               config.max_transform_hierarchy_depth_inter =
                  capCodecConfigData.CodecSupportLimits.pHEVCSupport->max_transform_hierarchy_depth_inter;
               config.max_transform_hierarchy_depth_intra =
                  capCodecConfigData.CodecSupportLimits.pHEVCSupport->max_transform_hierarchy_depth_intra;
            }
         }

         if (!is_supported) {
            debug_printf("D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION arguments are not supported - "
               "Call to CheckFeatureCaps (D3D12_FEATURE_VIDEO_ENCODER_CODEC_CONFIGURATION_SUPPORT, ...) returned failure "
               "or not supported for Codec HEVC -  MinLumaSize %d - MaxLumaSize %d -  MinTransformSize %d - "
               "MaxTransformSize %d - Depth_inter %d - Depth intra %d\n",
               config.MinLumaCodingUnitSize,
               config.MaxLumaCodingUnitSize,
               config.MinLumaTransformUnitSize,
               config.MaxLumaTransformUnitSize,
               config.max_transform_hierarchy_depth_inter,
               config.max_transform_hierarchy_depth_intra);

            return config;
         }
   }

   if (picture->seq.amp_enabled_flag)
      config.ConfigurationFlags |= D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC_FLAG_USE_ASYMETRIC_MOTION_PARTITION;

   if (picture->seq.sample_adaptive_offset_enabled_flag)
      config.ConfigurationFlags |= D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC_FLAG_ENABLE_SAO_FILTER;

   if (picture->pic.pps_loop_filter_across_slices_enabled_flag)
      config.ConfigurationFlags |= D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC_FLAG_DISABLE_LOOP_FILTER_ACROSS_SLICES;

   if (picture->pic.transform_skip_enabled_flag)
      config.ConfigurationFlags |= D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC_FLAG_ENABLE_TRANSFORM_SKIPPING;
   
   if (picture->pic.constrained_intra_pred_flag)
      config.ConfigurationFlags |= D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC_FLAG_USE_CONSTRAINED_INTRAPREDICTION;

   if(((config.ConfigurationFlags & D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC_FLAG_DISABLE_LOOP_FILTER_ACROSS_SLICES) != 0)
         && ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_HEVCCodecCaps.SupportFlags & D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_SUPPORT_HEVC_FLAG_DISABLING_LOOP_FILTER_ACROSS_SLICES_SUPPORT) == 0))
   {
         debug_printf("D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION arguments are not supported - Disable deblocking across slice boundary mode not supported."
         " Ignoring the request for this feature flag on this encode session\n");
         // Disable it and keep going with a warning
         config.ConfigurationFlags &= ~D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC_FLAG_DISABLE_LOOP_FILTER_ACROSS_SLICES;
   }  

   if(((config.ConfigurationFlags & D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC_FLAG_ALLOW_REQUEST_INTRA_CONSTRAINED_SLICES) != 0)
         && ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_HEVCCodecCaps.SupportFlags & D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_SUPPORT_HEVC_FLAG_INTRA_SLICE_CONSTRAINED_ENCODING_SUPPORT) == 0))
   {
         debug_printf("D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION arguments are not supported - Intra slice constrained mode not supported."
         " Ignoring the request for this feature flag on this encode session\n");
         // Disable it and keep going with a warning
         config.ConfigurationFlags &= ~D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC_FLAG_ALLOW_REQUEST_INTRA_CONSTRAINED_SLICES;
   }  

   if(((config.ConfigurationFlags & D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC_FLAG_ENABLE_SAO_FILTER) != 0)
         && ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_HEVCCodecCaps.SupportFlags & D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_SUPPORT_HEVC_FLAG_SAO_FILTER_SUPPORT) == 0))
   {
         debug_printf("D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION arguments are not supported - SAO Filter mode not supported."
         " Ignoring the request for this feature flag on this encode session\n");
         // Disable it and keep going with a warning
         config.ConfigurationFlags &= ~D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC_FLAG_ENABLE_SAO_FILTER;
   }  


   if(((config.ConfigurationFlags & D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC_FLAG_USE_ASYMETRIC_MOTION_PARTITION) != 0)
         && ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_HEVCCodecCaps.SupportFlags & D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_SUPPORT_HEVC_FLAG_ASYMETRIC_MOTION_PARTITION_SUPPORT) == 0))
   {
         debug_printf("D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION arguments are not supported - Asymetric motion partition not supported."
         " Ignoring the request for this feature flag on this encode session\n");
         // Disable it and keep going with a warning
         config.ConfigurationFlags &= ~D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC_FLAG_USE_ASYMETRIC_MOTION_PARTITION;
   }  

   if(((config.ConfigurationFlags & D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC_FLAG_USE_ASYMETRIC_MOTION_PARTITION) == 0)
         && ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_HEVCCodecCaps.SupportFlags & D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_SUPPORT_HEVC_FLAG_ASYMETRIC_MOTION_PARTITION_REQUIRED) != 0))
   {
         debug_printf("D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION arguments are not supported - Asymetric motion partition is required to be set."
         " Enabling this HW required feature flag on this encode session\n");
         // HW doesn't support otherwise, so set it
         config.ConfigurationFlags |= D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC_FLAG_USE_ASYMETRIC_MOTION_PARTITION;
   }  

   if(((config.ConfigurationFlags & D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC_FLAG_ENABLE_TRANSFORM_SKIPPING) != 0)
         && ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_HEVCCodecCaps.SupportFlags & D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_SUPPORT_HEVC_FLAG_TRANSFORM_SKIP_SUPPORT) == 0))
   {
         debug_printf("D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION arguments are not supported - Allow transform skipping is not supported."
         " Ignoring the request for this feature flag on this encode session\n");
         // Disable it and keep going with a warning
         config.ConfigurationFlags &= ~D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC_FLAG_ENABLE_TRANSFORM_SKIPPING;
   }  

   if(((config.ConfigurationFlags & D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC_FLAG_USE_CONSTRAINED_INTRAPREDICTION) != 0)
         && ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_HEVCCodecCaps.SupportFlags & D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_SUPPORT_HEVC_FLAG_CONSTRAINED_INTRAPREDICTION_SUPPORT) == 0))
   {
         debug_printf("D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION arguments are not supported - Constrained intra-prediction use is not supported."
         " Ignoring the request for this feature flag on this encode session\n");
         // Disable it and keep going with a warning
         config.ConfigurationFlags &= ~D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC_FLAG_USE_CONSTRAINED_INTRAPREDICTION;
   }

   return config;
}

static bool
d3d12_video_encoder_update_intra_refresh_hevc(struct d3d12_video_encoder *pD3D12Enc,
                                                        D3D12_VIDEO_SAMPLE srcTextureDesc,
                                                        struct pipe_h265_enc_picture_desc *  picture)
{
   if (picture->intra_refresh.mode != INTRA_REFRESH_MODE_NONE)
   {
      // D3D12 only supports row intra-refresh
      if (picture->intra_refresh.mode != INTRA_REFRESH_MODE_UNIT_ROWS)
      {
         debug_printf("[d3d12_video_encoder_update_intra_refresh_hevc] Unsupported INTRA_REFRESH_MODE %d\n", picture->intra_refresh.mode);
         return false;
      }

      uint8_t ctbSize = d3d12_video_encoder_convert_12cusize_to_pixel_size_hevc(
         pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_HEVCCodecCaps.MaxLumaCodingUnitSize);
      uint32_t total_frame_blocks = static_cast<uint32_t>(std::ceil(srcTextureDesc.Height / ctbSize)) *
                              static_cast<uint32_t>(std::ceil(srcTextureDesc.Width / ctbSize));
      D3D12_VIDEO_ENCODER_INTRA_REFRESH targetIntraRefresh = {
         D3D12_VIDEO_ENCODER_INTRA_REFRESH_MODE_ROW_BASED,
         total_frame_blocks / picture->intra_refresh.region_size,
      };
      double ir_wave_progress = (picture->intra_refresh.offset == 0) ? 0 :
         picture->intra_refresh.offset / (double) total_frame_blocks;
      pD3D12Enc->m_currentEncodeConfig.m_IntraRefreshCurrentFrameIndex =
         static_cast<uint32_t>(std::ceil(ir_wave_progress * targetIntraRefresh.IntraRefreshDuration));

      // Set intra refresh state
      pD3D12Enc->m_currentEncodeConfig.m_IntraRefresh = targetIntraRefresh;
      // Need to send the sequence flag during all the IR duration
      pD3D12Enc->m_currentEncodeConfig.m_ConfigDirtyFlags |= d3d12_video_encoder_config_dirty_flag_intra_refresh;
   } else {
      pD3D12Enc->m_currentEncodeConfig.m_IntraRefreshCurrentFrameIndex = 0;
      pD3D12Enc->m_currentEncodeConfig.m_IntraRefresh = {
         D3D12_VIDEO_ENCODER_INTRA_REFRESH_MODE_NONE,
         0,
      };
   }

   return true;
}

bool
d3d12_video_encoder_update_current_encoder_config_state_hevc(struct d3d12_video_encoder *pD3D12Enc,
                                                             D3D12_VIDEO_SAMPLE srcTextureDesc,
                                                             struct pipe_picture_desc *picture)
{
   struct pipe_h265_enc_picture_desc *hevcPic = (struct pipe_h265_enc_picture_desc *) picture;

   // Reset reconfig dirty flags
   pD3D12Enc->m_currentEncodeConfig.m_ConfigDirtyFlags = d3d12_video_encoder_config_dirty_flag_none;
   // Reset sequence changes flags
   pD3D12Enc->m_currentEncodeConfig.m_seqFlags = D3D12_VIDEO_ENCODER_SEQUENCE_CONTROL_FLAG_NONE;

   // Set codec
   if (pD3D12Enc->m_currentEncodeConfig.m_encoderCodecDesc != D3D12_VIDEO_ENCODER_CODEC_HEVC) {
      pD3D12Enc->m_currentEncodeConfig.m_ConfigDirtyFlags |= d3d12_video_encoder_config_dirty_flag_codec;
   }
   pD3D12Enc->m_currentEncodeConfig.m_encoderCodecDesc = D3D12_VIDEO_ENCODER_CODEC_HEVC;

   // Set Sequence information
   if (memcmp(&pD3D12Enc->m_currentEncodeConfig.m_encoderCodecSpecificSequenceStateDescH265,
              &hevcPic->seq,
              sizeof(hevcPic->seq)) != 0) {
      pD3D12Enc->m_currentEncodeConfig.m_ConfigDirtyFlags |= d3d12_video_encoder_config_dirty_flag_sequence_info;
   }
   pD3D12Enc->m_currentEncodeConfig.m_encoderCodecSpecificSequenceStateDescH265 = hevcPic->seq;

   if ((hevcPic->picture_type == PIPE_H2645_ENC_PICTURE_TYPE_IDR) &&
       (hevcPic->renew_headers_on_idr))
      pD3D12Enc->m_currentEncodeConfig.m_ConfigDirtyFlags |= d3d12_video_encoder_config_dirty_flag_sequence_info;

   // Set input format
   DXGI_FORMAT targetFmt = d3d12_convert_pipe_video_profile_to_dxgi_format(pD3D12Enc->base.profile);
   if (pD3D12Enc->m_currentEncodeConfig.m_encodeFormatInfo.Format != targetFmt) {
      pD3D12Enc->m_currentEncodeConfig.m_ConfigDirtyFlags |= d3d12_video_encoder_config_dirty_flag_input_format;
   }

   pD3D12Enc->m_currentEncodeConfig.m_encodeFormatInfo = {};
   pD3D12Enc->m_currentEncodeConfig.m_encodeFormatInfo.Format = targetFmt;
   HRESULT hr = pD3D12Enc->m_pD3D12Screen->dev->CheckFeatureSupport(D3D12_FEATURE_FORMAT_INFO,
                                                          &pD3D12Enc->m_currentEncodeConfig.m_encodeFormatInfo,
                                                          sizeof(pD3D12Enc->m_currentEncodeConfig.m_encodeFormatInfo));
   if (FAILED(hr)) {
      debug_printf("CheckFeatureSupport failed with HR %x\n", hr);
      return false;
   }

   // Set resolution
   if ((pD3D12Enc->m_currentEncodeConfig.m_currentResolution.Width != srcTextureDesc.Width) ||
       (pD3D12Enc->m_currentEncodeConfig.m_currentResolution.Height != srcTextureDesc.Height)) {
      pD3D12Enc->m_currentEncodeConfig.m_ConfigDirtyFlags |= d3d12_video_encoder_config_dirty_flag_resolution;
   }
   pD3D12Enc->m_currentEncodeConfig.m_currentResolution.Width = srcTextureDesc.Width;
   pD3D12Enc->m_currentEncodeConfig.m_currentResolution.Height = srcTextureDesc.Height;

   // Set resolution codec dimensions (ie. cropping)
   memset(&pD3D12Enc->m_currentEncodeConfig.m_FrameCroppingCodecConfig, 0,
         sizeof(pD3D12Enc->m_currentEncodeConfig.m_FrameCroppingCodecConfig));
   pD3D12Enc->m_currentEncodeConfig.m_FrameCroppingCodecConfig.front = hevcPic->seq.pic_width_in_luma_samples;
   pD3D12Enc->m_currentEncodeConfig.m_FrameCroppingCodecConfig.back = hevcPic->seq.pic_height_in_luma_samples;
   if (hevcPic->seq.conformance_window_flag) {
      pD3D12Enc->m_currentEncodeConfig.m_FrameCroppingCodecConfig.left = hevcPic->seq.conf_win_left_offset;
      pD3D12Enc->m_currentEncodeConfig.m_FrameCroppingCodecConfig.right = hevcPic->seq.conf_win_right_offset;
      pD3D12Enc->m_currentEncodeConfig.m_FrameCroppingCodecConfig.top = hevcPic->seq.conf_win_top_offset;
      pD3D12Enc->m_currentEncodeConfig.m_FrameCroppingCodecConfig.bottom = hevcPic->seq.conf_win_bottom_offset;
   }
   // Set profile
   auto targetProfile = d3d12_video_encoder_convert_profile_to_d3d12_enc_profile_hevc(pD3D12Enc->base.profile);
   if (pD3D12Enc->m_currentEncodeConfig.m_encoderProfileDesc.m_HEVCProfile != targetProfile) {
      pD3D12Enc->m_currentEncodeConfig.m_ConfigDirtyFlags |= d3d12_video_encoder_config_dirty_flag_profile;
   }
   pD3D12Enc->m_currentEncodeConfig.m_encoderProfileDesc.m_HEVCProfile = targetProfile;

   // Set level
   auto targetLevel = d3d12_video_encoder_convert_level_hevc(hevcPic->seq.general_level_idc);
   auto targetTier = (hevcPic->seq.general_tier_flag == 0) ? D3D12_VIDEO_ENCODER_TIER_HEVC_MAIN : D3D12_VIDEO_ENCODER_TIER_HEVC_HIGH;
   if ( (pD3D12Enc->m_currentEncodeConfig.m_encoderLevelDesc.m_HEVCLevelSetting.Level != targetLevel)
      || (pD3D12Enc->m_currentEncodeConfig.m_encoderLevelDesc.m_HEVCLevelSetting.Tier != targetTier)) {
      pD3D12Enc->m_currentEncodeConfig.m_ConfigDirtyFlags |= d3d12_video_encoder_config_dirty_flag_level;
   }
   pD3D12Enc->m_currentEncodeConfig.m_encoderLevelDesc.m_HEVCLevelSetting.Tier = targetTier;
   pD3D12Enc->m_currentEncodeConfig.m_encoderLevelDesc.m_HEVCLevelSetting.Level = targetLevel;

   // Set codec config
   bool is_supported = true;
   auto targetCodecConfig = d3d12_video_encoder_convert_hevc_codec_configuration(pD3D12Enc, hevcPic, is_supported);
   if(!is_supported) {
      return false;
   }

   if (memcmp(&pD3D12Enc->m_currentEncodeConfig.m_encoderCodecSpecificConfigDesc.m_HEVCConfig,
              &targetCodecConfig,
              sizeof(D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC)) != 0) {
      pD3D12Enc->m_currentEncodeConfig.m_ConfigDirtyFlags |= d3d12_video_encoder_config_dirty_flag_codec_config;
   }
   pD3D12Enc->m_currentEncodeConfig.m_encoderCodecSpecificConfigDesc.m_HEVCConfig = targetCodecConfig;

   // Set rate control
   d3d12_video_encoder_update_current_rate_control_hevc(pD3D12Enc, hevcPic);

   ///
   /// Check for video encode support detailed capabilities
   ///

   // Will call for d3d12 driver support based on the initial requested features, then
   // try to fallback if any of them is not supported and return the negotiated d3d12 settings
   D3D12_FEATURE_DATA_VIDEO_ENCODER_SUPPORT1 capEncoderSupportData1 = {};
   // Get max number of slices per frame supported
   pD3D12Enc->m_currentEncodeConfig.m_encoderSliceConfigMode =
      D3D12_VIDEO_ENCODER_FRAME_SUBREGION_LAYOUT_MODE_UNIFORM_PARTITIONING_SUBREGIONS_PER_FRAME;
   if (!d3d12_video_encoder_negotiate_requested_features_and_d3d12_driver_caps(pD3D12Enc, capEncoderSupportData1)) {
      debug_printf("[d3d12_video_encoder_hevc] After negotiating caps, D3D12_FEATURE_VIDEO_ENCODER_SUPPORT1 "
                      "arguments are not supported - "
                      "ValidationFlags: 0x%x - SupportFlags: 0x%x\n",
                      capEncoderSupportData1.ValidationFlags,
                      capEncoderSupportData1.SupportFlags);
      return false;
   }

   // Set slices config  (configure before calling d3d12_video_encoder_calculate_max_slices_count_in_output)
   if(!d3d12_video_encoder_negotiate_current_hevc_slices_configuration(pD3D12Enc, hevcPic)) {
      debug_printf("d3d12_video_encoder_negotiate_current_hevc_slices_configuration failed!\n");
      return false;
   }

   ///
   // Calculate current settings based on the returned values from the caps query
   //
   pD3D12Enc->m_currentEncodeCapabilities.m_MaxSlicesInOutput =
      d3d12_video_encoder_calculate_max_slices_count_in_output(
         pD3D12Enc->m_currentEncodeConfig.m_encoderSliceConfigMode,
         &pD3D12Enc->m_currentEncodeConfig.m_encoderSliceConfigDesc.m_SlicesPartition_HEVC,
         pD3D12Enc->m_currentEncodeCapabilities.m_currentResolutionSupportCaps.MaxSubregionsNumber,
         pD3D12Enc->m_currentEncodeConfig.m_currentResolution,
         pD3D12Enc->m_currentEncodeCapabilities.m_currentResolutionSupportCaps.SubregionBlockPixelsSize);

   // Set GOP config
   if(!d3d12_video_encoder_update_hevc_gop_configuration(pD3D12Enc, hevcPic)) {
      debug_printf("d3d12_video_encoder_update_hevc_gop_configuration failed!\n");
      return false;
   }

   // Set intra-refresh config
   if(!d3d12_video_encoder_update_intra_refresh_hevc(pD3D12Enc, srcTextureDesc, hevcPic)) {
      debug_printf("d3d12_video_encoder_update_intra_refresh_hevc failed!\n");
      return false;
   }

   // m_currentEncodeConfig.m_encoderPicParamsDesc pic params are set in d3d12_video_encoder_reconfigure_encoder_objects
   // after re-allocating objects if needed

   // Set motion estimation config
   auto targetMotionLimit = d3d12_video_encoder_convert_hevc_motion_configuration(pD3D12Enc, hevcPic);
   if (pD3D12Enc->m_currentEncodeConfig.m_encoderMotionPrecisionLimit != targetMotionLimit) {
      pD3D12Enc->m_currentEncodeConfig.m_ConfigDirtyFlags |=
         d3d12_video_encoder_config_dirty_flag_motion_precision_limit;
   }
   pD3D12Enc->m_currentEncodeConfig.m_encoderMotionPrecisionLimit = targetMotionLimit;

   //
   // Validate caps support returned values against current settings
   //
   if (pD3D12Enc->m_currentEncodeConfig.m_encoderProfileDesc.m_HEVCProfile !=
       pD3D12Enc->m_currentEncodeCapabilities.m_encoderSuggestedProfileDesc.m_HEVCProfile) {
      debug_printf("[d3d12_video_encoder_hevc] Warning: Requested D3D12_VIDEO_ENCODER_PROFILE_HEVC by upper layer: %d "
                    "mismatches UMD suggested D3D12_VIDEO_ENCODER_PROFILE_HEVC: %d\n",
                    pD3D12Enc->m_currentEncodeConfig.m_encoderProfileDesc.m_HEVCProfile,
                    pD3D12Enc->m_currentEncodeCapabilities.m_encoderSuggestedProfileDesc.m_HEVCProfile);
   }

   if (pD3D12Enc->m_currentEncodeConfig.m_encoderLevelDesc.m_HEVCLevelSetting.Tier !=
       pD3D12Enc->m_currentEncodeCapabilities.m_encoderLevelSuggestedDesc.m_HEVCLevelSetting.Tier) {
      debug_printf("[d3d12_video_encoder_hevc] Warning: Requested D3D12_VIDEO_ENCODER_LEVELS_HEVC.Tier by upper layer: %d "
                    "mismatches UMD suggested D3D12_VIDEO_ENCODER_LEVELS_HEVC.Tier: %d\n",
                    pD3D12Enc->m_currentEncodeConfig.m_encoderLevelDesc.m_HEVCLevelSetting.Tier,
                    pD3D12Enc->m_currentEncodeCapabilities.m_encoderLevelSuggestedDesc.m_HEVCLevelSetting.Tier);
   }

   if (pD3D12Enc->m_currentEncodeConfig.m_encoderLevelDesc.m_HEVCLevelSetting.Level !=
       pD3D12Enc->m_currentEncodeCapabilities.m_encoderLevelSuggestedDesc.m_HEVCLevelSetting.Level) {
      debug_printf("[d3d12_video_encoder_hevc] Warning: Requested D3D12_VIDEO_ENCODER_LEVELS_HEVC.Level by upper layer: %d "
                    "mismatches UMD suggested D3D12_VIDEO_ENCODER_LEVELS_HEVC.Level: %d\n",
                    pD3D12Enc->m_currentEncodeConfig.m_encoderLevelDesc.m_HEVCLevelSetting.Level,
                    pD3D12Enc->m_currentEncodeCapabilities.m_encoderLevelSuggestedDesc.m_HEVCLevelSetting.Level);
   }

   if (pD3D12Enc->m_currentEncodeCapabilities.m_MaxSlicesInOutput >
       pD3D12Enc->m_currentEncodeCapabilities.m_currentResolutionSupportCaps.MaxSubregionsNumber) {
      debug_printf("[d3d12_video_encoder_hevc] Desired number of subregions %d is not supported (higher than max "
                      "reported slice number %d in query caps) for current resolution (%d, %d)\n.",
                      pD3D12Enc->m_currentEncodeCapabilities.m_MaxSlicesInOutput,
                      pD3D12Enc->m_currentEncodeCapabilities.m_currentResolutionSupportCaps.MaxSubregionsNumber,
                      pD3D12Enc->m_currentEncodeConfig.m_currentResolution.Width,
                      pD3D12Enc->m_currentEncodeConfig.m_currentResolution.Height);
      return false;
   }
   return true;
}

D3D12_VIDEO_ENCODER_PROFILE_HEVC
d3d12_video_encoder_convert_profile_to_d3d12_enc_profile_hevc(enum pipe_video_profile profile)
{
   switch (profile) {
      case PIPE_VIDEO_PROFILE_HEVC_MAIN:
      {
         return D3D12_VIDEO_ENCODER_PROFILE_HEVC_MAIN;

      } break;
      case PIPE_VIDEO_PROFILE_HEVC_MAIN_10:
      {
         return D3D12_VIDEO_ENCODER_PROFILE_HEVC_MAIN10;
      } break;
      default:
      {
         unreachable("Unsupported pipe_video_profile");
      } break;
   }
}

bool
d3d12_video_encoder_isequal_slice_config_hevc(
   D3D12_VIDEO_ENCODER_FRAME_SUBREGION_LAYOUT_MODE targetMode,
   D3D12_VIDEO_ENCODER_PICTURE_CONTROL_SUBREGIONS_LAYOUT_DATA_SLICES targetConfig,
   D3D12_VIDEO_ENCODER_FRAME_SUBREGION_LAYOUT_MODE otherMode,
   D3D12_VIDEO_ENCODER_PICTURE_CONTROL_SUBREGIONS_LAYOUT_DATA_SLICES otherConfig)
{
   return (targetMode == otherMode) &&
          (memcmp(&targetConfig,
                  &otherConfig,
                  sizeof(D3D12_VIDEO_ENCODER_PICTURE_CONTROL_SUBREGIONS_LAYOUT_DATA_SLICES)) == 0);
}

uint32_t
d3d12_video_encoder_build_codec_headers_hevc(struct d3d12_video_encoder *pD3D12Enc,
                                             std::vector<uint64_t> &pWrittenCodecUnitsSizes)
{
   D3D12_VIDEO_ENCODER_PICTURE_CONTROL_CODEC_DATA currentPicParams =
      d3d12_video_encoder_get_current_picture_param_settings(pD3D12Enc);

   auto profDesc = d3d12_video_encoder_get_current_profile_desc(pD3D12Enc);
   auto levelDesc = d3d12_video_encoder_get_current_level_desc(pD3D12Enc);
   auto codecConfigDesc = d3d12_video_encoder_get_current_codec_config_desc(pD3D12Enc);
   auto MaxDPBCapacity = d3d12_video_encoder_get_current_max_dpb_capacity(pD3D12Enc);

   pWrittenCodecUnitsSizes.clear();
   bool isFirstFrame = (pD3D12Enc->m_fenceValue == 1);

   d3d12_video_bitstream_builder_hevc *pHEVCBitstreamBuilder =
      static_cast<d3d12_video_bitstream_builder_hevc *>(pD3D12Enc->m_upBitstreamBuilder.get());
   assert(pHEVCBitstreamBuilder);

   uint32_t active_seq_parameter_set_id = pHEVCBitstreamBuilder->get_active_sps_id();
   uint32_t active_video_parameter_set_id = pHEVCBitstreamBuilder->get_active_vps_id();
   
   bool writeNewVPS = isFirstFrame;
   
   size_t writtenVPSBytesCount = 0;
   if (writeNewVPS) {
      bool gopHasBFrames = (pD3D12Enc->m_currentEncodeConfig.m_encoderGOPConfigDesc.m_HEVCGroupOfPictures.PPicturePeriod > 1);
      pHEVCBitstreamBuilder->build_vps(*profDesc.pHEVCProfile,
                                       *levelDesc.pHEVCLevelSetting,
                                       pD3D12Enc->m_currentEncodeConfig.m_encodeFormatInfo.Format,
                                       MaxDPBCapacity,   // max_num_ref_frames
                                       gopHasBFrames,
                                       active_video_parameter_set_id,
                                       pD3D12Enc->m_BitstreamHeadersBuffer,
                                       pD3D12Enc->m_BitstreamHeadersBuffer.begin(),
                                       writtenVPSBytesCount);      

      pWrittenCodecUnitsSizes.push_back(writtenVPSBytesCount);
   }

   bool writeNewSPS = writeNewVPS                                          // on new VPS written
                      || ((pD3D12Enc->m_currentEncodeConfig.m_seqFlags &   // also on resolution change
                           D3D12_VIDEO_ENCODER_SEQUENCE_CONTROL_FLAG_RESOLUTION_CHANGE) != 0)
                      // Also on input format dirty flag for new SPS, VUI etc
                      || (pD3D12Enc->m_currentEncodeConfig.m_ConfigDirtyFlags & d3d12_video_encoder_config_dirty_flag_sequence_info);

   size_t writtenSPSBytesCount = 0;
   if (writeNewSPS) {
      pHEVCBitstreamBuilder->build_sps(
                           pHEVCBitstreamBuilder->get_latest_vps(),
                           pD3D12Enc->m_currentEncodeConfig.m_encoderCodecSpecificSequenceStateDescH265,
                           active_seq_parameter_set_id,
                           pD3D12Enc->m_currentEncodeConfig.m_currentResolution,
                           pD3D12Enc->m_currentEncodeConfig.m_FrameCroppingCodecConfig,
                           pD3D12Enc->m_currentEncodeCapabilities.m_currentResolutionSupportCaps.SubregionBlockPixelsSize,
                           pD3D12Enc->m_currentEncodeConfig.m_encodeFormatInfo.Format,
                           *codecConfigDesc.pHEVCConfig,
                           pD3D12Enc->m_currentEncodeConfig.m_encoderGOPConfigDesc.m_HEVCGroupOfPictures,
                           pD3D12Enc->m_BitstreamHeadersBuffer,
                           pD3D12Enc->m_BitstreamHeadersBuffer.begin() + writtenVPSBytesCount,
                           writtenSPSBytesCount);

      pWrittenCodecUnitsSizes.push_back(writtenSPSBytesCount);
   }

   size_t writtenPPSBytesCount = 0;
   pHEVCBitstreamBuilder->build_pps(pHEVCBitstreamBuilder->get_latest_sps(),
                                    currentPicParams.pHEVCPicData->slice_pic_parameter_set_id,
                                    *codecConfigDesc.pHEVCConfig,
                                    *currentPicParams.pHEVCPicData,
                                    pD3D12Enc->m_StagingHeadersBuffer,
                                    pD3D12Enc->m_StagingHeadersBuffer.begin(),
                                    writtenPPSBytesCount);

   std::vector<uint8_t>& active_pps = pHEVCBitstreamBuilder->get_active_pps();
   if (writeNewSPS ||
       (writtenPPSBytesCount != active_pps.size()) ||
       memcmp(pD3D12Enc->m_StagingHeadersBuffer.data(), active_pps.data(), writtenPPSBytesCount)) {
      active_pps = pD3D12Enc->m_StagingHeadersBuffer;
      pD3D12Enc->m_BitstreamHeadersBuffer.resize(writtenSPSBytesCount + writtenVPSBytesCount + writtenPPSBytesCount);
      memcpy(&pD3D12Enc->m_BitstreamHeadersBuffer.data()[(writtenSPSBytesCount + writtenVPSBytesCount)], pD3D12Enc->m_StagingHeadersBuffer.data(), writtenPPSBytesCount);
      pWrittenCodecUnitsSizes.push_back(writtenPPSBytesCount);
   } else {
      writtenPPSBytesCount = 0;
      debug_printf("Skipping PPS (same as active PPS) for fenceValue: %" PRIu64 "\n", pD3D12Enc->m_fenceValue);
   }

   // Shrink buffer to fit the headers
   if (pD3D12Enc->m_BitstreamHeadersBuffer.size() > (writtenPPSBytesCount + writtenSPSBytesCount + writtenVPSBytesCount)) {
      pD3D12Enc->m_BitstreamHeadersBuffer.resize(writtenPPSBytesCount + writtenSPSBytesCount + writtenVPSBytesCount);
   }

   assert(std::accumulate(pWrittenCodecUnitsSizes.begin(), pWrittenCodecUnitsSizes.end(), 0u) ==
      static_cast<uint64_t>(pD3D12Enc->m_BitstreamHeadersBuffer.size()));
   return pD3D12Enc->m_BitstreamHeadersBuffer.size();
}
