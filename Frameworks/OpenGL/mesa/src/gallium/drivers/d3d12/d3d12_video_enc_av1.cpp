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
#include "d3d12_video_enc_av1.h"
#include "util/u_video.h"
#include "d3d12_resource.h"
#include "d3d12_screen.h"
#include "d3d12_format.h"
#include <cmath>
#include <numeric>

void
d3d12_video_encoder_update_current_rate_control_av1(struct d3d12_video_encoder *pD3D12Enc,
                                                    pipe_av1_enc_picture_desc *picture)
{
   pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc = {};
   pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_FrameRate.Numerator = picture->rc[0].frame_rate_num;
   pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_FrameRate.Denominator = picture->rc[0].frame_rate_den;
   pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Flags = D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_NONE;

   if (picture->roi.num > 0)
      pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Flags |=
         D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_DELTA_QP;

   switch (picture->rc[0].rate_ctrl_method) {
      case PIPE_H2645_ENC_RATE_CONTROL_METHOD_VARIABLE_SKIP:
      case PIPE_H2645_ENC_RATE_CONTROL_METHOD_VARIABLE:
      {
         pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Mode = D3D12_VIDEO_ENCODER_RATE_CONTROL_MODE_VBR;
         pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_VBR.TargetAvgBitRate =
            picture->rc[0].target_bitrate;
         pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_VBR.PeakBitRate =
            picture->rc[0].peak_bitrate;

         if (D3D12_VIDEO_ENC_CBR_FORCE_VBV_EQUAL_BITRATE) {
            debug_printf(
               "[d3d12_video_encoder_av1] d3d12_video_encoder_update_current_rate_control_av1 "
               "D3D12_VIDEO_ENC_CBR_FORCE_VBV_EQUAL_BITRATE environment variable is set, "
               ", forcing VBV Size = VBV Initial Capacity = Target Bitrate = %" PRIu64 " (bits)\n",
               pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_CBR.TargetBitRate);
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Flags |=
               D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_VBV_SIZES;
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_CBR.VBVCapacity =
               pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_CBR.TargetBitRate;
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_CBR.InitialVBVFullness =
               pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_CBR.TargetBitRate;
         } else if (picture->rc[0].app_requested_hrd_buffer) {
            debug_printf(
               "[d3d12_video_encoder_av1] d3d12_video_encoder_update_current_rate_control_av1 HRD required by app,"
               " setting VBV Size = %d (bits) - VBV Initial Capacity %d (bits)\n",
               picture->rc[0].vbv_buffer_size,
               picture->rc[0].vbv_buf_initial_size);
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Flags |=
               D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_VBV_SIZES;
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_VBR.VBVCapacity =
               picture->rc[0].vbv_buffer_size;
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_VBR.InitialVBVFullness =
               picture->rc[0].vbv_buf_initial_size;
         }

         pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.max_frame_size = picture->rc[0].max_au_size;
         if (picture->rc[0].max_au_size > 0) {
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Flags |=
               D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_MAX_FRAME_SIZE;
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_VBR.MaxFrameBitSize =
               picture->rc[0].max_au_size;

            debug_printf(
               "[d3d12_video_encoder_av1] d3d12_video_encoder_update_current_rate_control_av1 "
               "Upper layer requested explicit MaxFrameBitSize: %" PRIu64 "\n",
               pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_VBR.MaxFrameBitSize);
         }

         if (picture->rc[0].app_requested_qp_range) {
            debug_printf("[d3d12_video_encoder_av1] d3d12_video_encoder_update_current_rate_control_av1 "
                         "Upper layer requested explicit MinQP: %d MaxQP: %d\n",
                         picture->rc[0].min_qp,
                         picture->rc[0].max_qp);
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Flags |=
               D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_QP_RANGE;
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_VBR.MinQP =
               picture->rc[0].min_qp;
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_VBR.MaxQP =
               picture->rc[0].max_qp;
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
            picture->rc[0].target_bitrate;
         pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_QVBR.PeakBitRate =
            picture->rc[0].peak_bitrate;
         pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_QVBR.ConstantQualityTarget =
            picture->rc[0].vbr_quality_factor;

         if (D3D12_VIDEO_ENC_CBR_FORCE_VBV_EQUAL_BITRATE) {
            debug_printf("[d3d12_video_encoder_av1] d3d12_video_encoder_update_current_rate_control_av1 "
                         "D3D12_VIDEO_ENC_CBR_FORCE_VBV_EQUAL_BITRATE environment variable is set, "
                         ", forcing VBV Size = VBV Initial Capacity = Target Bitrate = %" PRIu64 " (bits)\n",
                         pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_QVBR1
                            .TargetAvgBitRate);
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Flags |=
               D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_VBV_SIZES;
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Flags |=
               D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_EXTENSION1_SUPPORT;
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_QVBR1.VBVCapacity =
               pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_QVBR1.TargetAvgBitRate;
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_QVBR1.InitialVBVFullness =
               pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_QVBR1.TargetAvgBitRate;
         } else if (picture->rc[0].app_requested_hrd_buffer) {
            debug_printf(
               "[d3d12_video_encoder_av1] d3d12_video_encoder_update_current_rate_control_av1 HRD required by app,"
               " setting VBV Size = %d (bits) - VBV Initial Capacity %d (bits)\n",
               picture->rc[0].vbv_buffer_size,
               picture->rc[0].vbv_buf_initial_size);
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Flags |=
               D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_VBV_SIZES;
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Flags |=
               D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_EXTENSION1_SUPPORT;
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_QVBR1.VBVCapacity =
               picture->rc[0].vbv_buffer_size;
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_QVBR1.InitialVBVFullness =
               picture->rc[0].vbv_buf_initial_size;
         }
         pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.max_frame_size = picture->rc[0].max_au_size;
         if (picture->rc[0].max_au_size > 0) {
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Flags |=
               D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_MAX_FRAME_SIZE;
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_QVBR.MaxFrameBitSize =
               picture->rc[0].max_au_size;

            debug_printf(
               "[d3d12_video_encoder_av1] d3d12_video_encoder_update_current_rate_control_av1 "
               "Upper layer requested explicit MaxFrameBitSize: %" PRIu64 "\n",
               pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_QVBR.MaxFrameBitSize);
         }

         if (picture->rc[0].app_requested_qp_range) {
            debug_printf("[d3d12_video_encoder_av1] d3d12_video_encoder_update_current_rate_control_av1 "
                         "Upper layer requested explicit MinQP: %d MaxQP: %d\n",
                         picture->rc[0].min_qp,
                         picture->rc[0].max_qp);
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Flags |=
               D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_QP_RANGE;
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_QVBR.MinQP =
               picture->rc[0].min_qp;
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_QVBR.MaxQP =
               picture->rc[0].max_qp;
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
            picture->rc[0].target_bitrate;

         /* For CBR mode, to guarantee bitrate of generated stream complies with
          * target bitrate (e.g. no over +/-10%), vbv_buffer_size and initial capacity should be same
          * as target bitrate. Controlled by OS env var D3D12_VIDEO_ENC_CBR_FORCE_VBV_EQUAL_BITRATE
          */
         if (D3D12_VIDEO_ENC_CBR_FORCE_VBV_EQUAL_BITRATE) {
            debug_printf(
               "[d3d12_video_encoder_av1] d3d12_video_encoder_update_current_rate_control_av1 "
               "D3D12_VIDEO_ENC_CBR_FORCE_VBV_EQUAL_BITRATE environment variable is set, "
               ", forcing VBV Size = VBV Initial Capacity = Target Bitrate = %" PRIu64 " (bits)\n",
               pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_CBR.TargetBitRate);
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Flags |=
               D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_VBV_SIZES;
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_CBR.VBVCapacity =
               pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_CBR.TargetBitRate;
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_CBR.InitialVBVFullness =
               pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_CBR.TargetBitRate;
         } else if (picture->rc[0].app_requested_hrd_buffer) {
            debug_printf(
               "[d3d12_video_encoder_av1] d3d12_video_encoder_update_current_rate_control_av1 HRD required by app,"
               " setting VBV Size = %d (bits) - VBV Initial Capacity %d (bits)\n",
               picture->rc[0].vbv_buffer_size,
               picture->rc[0].vbv_buf_initial_size);
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Flags |=
               D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_VBV_SIZES;
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_CBR.VBVCapacity =
               picture->rc[0].vbv_buffer_size;
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_CBR.InitialVBVFullness =
               picture->rc[0].vbv_buf_initial_size;
         }

         pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.max_frame_size = picture->rc[0].max_au_size;
         if (picture->rc[0].max_au_size > 0) {
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Flags |=
               D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_MAX_FRAME_SIZE;
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_CBR.MaxFrameBitSize =
               picture->rc[0].max_au_size;

            debug_printf(
               "[d3d12_video_encoder_av1] d3d12_video_encoder_update_current_rate_control_av1 "
               "Upper layer requested explicit MaxFrameBitSize: %" PRIu64 "\n",
               pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_CBR.MaxFrameBitSize);
         }

         if (picture->rc[0].app_requested_qp_range) {
            debug_printf("[d3d12_video_encoder_av1] d3d12_video_encoder_update_current_rate_control_av1 "
                         "Upper layer requested explicit MinQP: %d MaxQP: %d\n",
                         picture->rc[0].min_qp,
                         picture->rc[0].max_qp);

            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Flags |=
               D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_QP_RANGE;
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_CBR.MinQP =
               picture->rc[0].min_qp;
            pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_CBR.MaxQP =
               picture->rc[0].max_qp;
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
            .ConstantQP_FullIntracodedFrame = picture->rc[0].app_requested_initial_qp ? picture->rc[0].qp : 0;
         pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_CQP
            .ConstantQP_InterPredictedFrame_PrevRefOnly =
            picture->rc[0].app_requested_initial_qp ? picture->rc[0].qp : 0;
         pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Config.m_Configuration_CQP
            .ConstantQP_InterPredictedFrame_BiDirectionalRef =
            picture->rc[0].app_requested_initial_qp ? picture->rc[0].qp : 0;

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
         debug_printf("[d3d12_video_encoder_av1] d3d12_video_encoder_update_current_rate_control_av1 invalid RC "
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

//
// Returns AV1 extra size on top of the usual base metadata layout size
//
size_t
d3d12_video_encoder_calculate_metadata_resolved_buffer_size_av1(uint32_t maxSliceNumber)
{
   return sizeof(D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_SUBREGIONS_LAYOUT_DATA_TILES) +
          sizeof(D3D12_VIDEO_ENCODER_AV1_POST_ENCODE_VALUES);
}

void
d3d12_video_encoder_convert_d3d12_to_spec_level_av1(D3D12_VIDEO_ENCODER_AV1_LEVELS level12, uint32_t &specLevel)
{
   // Enum matches values as in seq_level_idx
   specLevel = (uint32_t) level12;
}

void
d3d12_video_encoder_convert_d3d12_to_spec_tier_av1(D3D12_VIDEO_ENCODER_AV1_TIER tier12, uint32_t &specTier)
{
   // Enum matches values as in seq_level_idx
   specTier = (uint32_t) tier12;
}

void
d3d12_video_encoder_convert_spec_to_d3d12_level_av1(uint32_t specLevel, D3D12_VIDEO_ENCODER_AV1_LEVELS &level12)
{
   // Enum matches values as in seq_level_idx
   level12 = (D3D12_VIDEO_ENCODER_AV1_LEVELS) specLevel;
}

void
d3d12_video_encoder_convert_spec_to_d3d12_tier_av1(uint32_t specTier, D3D12_VIDEO_ENCODER_AV1_TIER &tier12)
{
   // Enum matches values as in seq_tier
   tier12 = (D3D12_VIDEO_ENCODER_AV1_TIER) specTier;
}

uint32_t
d3d12_video_encoder_convert_d3d12_profile_to_spec_profile_av1(D3D12_VIDEO_ENCODER_AV1_PROFILE profile)
{
   switch (profile) {
      case D3D12_VIDEO_ENCODER_AV1_PROFILE_MAIN:
      {
         return 0;
      } break;
      default:
      {
         unreachable("Unsupported D3D12_VIDEO_ENCODER_AV1_PROFILE");
      } break;
   }
}

D3D12_VIDEO_ENCODER_AV1_PROFILE
d3d12_video_encoder_convert_profile_to_d3d12_enc_profile_av1(enum pipe_video_profile profile)
{
   switch (profile) {
      case PIPE_VIDEO_PROFILE_AV1_MAIN:
      {
         return D3D12_VIDEO_ENCODER_AV1_PROFILE_MAIN;
      } break;
      default:
      {
         unreachable("Unsupported pipe_video_profile");
      } break;
   }
}

bool
d3d12_video_encoder_update_av1_gop_configuration(struct d3d12_video_encoder *pD3D12Enc,
                                                 pipe_av1_enc_picture_desc *picture)
{
   static_assert((unsigned) D3D12_VIDEO_ENCODER_AV1_FRAME_TYPE_KEY_FRAME == (unsigned) PIPE_AV1_ENC_FRAME_TYPE_KEY);
   static_assert((unsigned) D3D12_VIDEO_ENCODER_AV1_FRAME_TYPE_INTER_FRAME == (unsigned) PIPE_AV1_ENC_FRAME_TYPE_INTER);
   static_assert((unsigned) D3D12_VIDEO_ENCODER_AV1_FRAME_TYPE_INTRA_ONLY_FRAME ==
                 (unsigned) PIPE_AV1_ENC_FRAME_TYPE_INTRA_ONLY);
   static_assert((unsigned) D3D12_VIDEO_ENCODER_AV1_FRAME_TYPE_SWITCH_FRAME ==
                 (unsigned) PIPE_AV1_ENC_FRAME_TYPE_SWITCH);

   // Only update GOP when it begins
   // This triggers DPB/encoder/heap re-creation, so only check on IDR when a GOP might change
   if ((picture->frame_type == PIPE_AV1_ENC_FRAME_TYPE_INTRA_ONLY) ||
       (picture->frame_type == PIPE_AV1_ENC_FRAME_TYPE_KEY)) {
      uint32_t GOPLength = picture->seq.intra_period;
      uint32_t PPicturePeriod = picture->seq.ip_period;

      // Set dirty flag if m_AV1SequenceStructure changed
      auto previousGOPConfig = pD3D12Enc->m_currentEncodeConfig.m_encoderGOPConfigDesc.m_AV1SequenceStructure;
      pD3D12Enc->m_currentEncodeConfig.m_encoderGOPConfigDesc.m_AV1SequenceStructure = {
         GOPLength,
         PPicturePeriod,
      };

      if (memcmp(&previousGOPConfig,
                 &pD3D12Enc->m_currentEncodeConfig.m_encoderGOPConfigDesc.m_AV1SequenceStructure,
                 sizeof(D3D12_VIDEO_ENCODER_AV1_SEQUENCE_STRUCTURE)) != 0) {
         pD3D12Enc->m_currentEncodeConfig.m_ConfigDirtyFlags |= d3d12_video_encoder_config_dirty_flag_gop;
      }
   }
   return true;
}

D3D12_VIDEO_ENCODER_MOTION_ESTIMATION_PRECISION_MODE
d3d12_video_encoder_convert_av1_motion_configuration(struct d3d12_video_encoder *pD3D12Enc,
                                                     pipe_av1_enc_picture_desc *picture)
{
   return D3D12_VIDEO_ENCODER_MOTION_ESTIMATION_PRECISION_MODE_MAXIMUM;
}

bool
d3d12_video_encoder_compare_tile_config_av1(
   D3D12_VIDEO_ENCODER_FRAME_SUBREGION_LAYOUT_MODE currentTilesMode,
   D3D12_VIDEO_ENCODER_FRAME_SUBREGION_LAYOUT_MODE requestedTilesMode,
   D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_SUBREGIONS_LAYOUT_DATA_TILES currentTilePartition,
   D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_SUBREGIONS_LAYOUT_DATA_TILES requestedTilePartition)
{
   if (currentTilesMode != requestedTilesMode)
      return false;

   if (memcmp(&currentTilePartition,
              &requestedTilePartition,
              sizeof(D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_SUBREGIONS_LAYOUT_DATA_TILES)))
      return false;

   return true;
}

///
/// Tries to configurate the encoder using the requested tile configuration
///
bool
d3d12_video_encoder_negotiate_current_av1_tiles_configuration(struct d3d12_video_encoder *pD3D12Enc,
                                                              pipe_av1_enc_picture_desc *pAV1Pic)
{
   D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_SUBREGIONS_LAYOUT_DATA_TILES tilePartition = {};
   tilePartition.RowCount = pAV1Pic->tile_rows;
   tilePartition.ColCount = pAV1Pic->tile_cols;
   tilePartition.ContextUpdateTileId = pAV1Pic->context_update_tile_id;

   // VA-API interface has 63 entries for tile cols/rows. When 64 requested,
   // last one has to be calculated from the frame width/height in superblocks.

   // Copy the tile col sizes (up to 63 defined in VA-API interface array sizes)
   size_t accum_cols_sb = 0;
   uint8_t src_cols_count = MIN2(63, pAV1Pic->tile_cols);
   for (uint8_t i = 0; i < src_cols_count; i++) {
      tilePartition.ColWidths[i] = pAV1Pic->width_in_sbs_minus_1[i] + 1;
      accum_cols_sb += tilePartition.ColWidths[i];
   }

   // If there are 64 cols, calculate the last one manually as the difference
   // between frame width in sb minus the accumulated tiles sb sizes
   if (pAV1Pic->tile_cols == 64)
      tilePartition.ColWidths[63] = pAV1Pic->frame_width_sb - accum_cols_sb;

   // Copy the tile row sizes (up to 63 defined in VA-API interface array sizes)
   size_t accum_rows_sb = 0;
   uint8_t src_rows_count = MIN2(63, pAV1Pic->tile_rows);
   for (uint8_t i = 0; i < src_rows_count; i++) {
      tilePartition.RowHeights[i] = pAV1Pic->height_in_sbs_minus_1[i] + 1;
      accum_rows_sb += tilePartition.RowHeights[i];
   }

   // If there are 64 rows, calculate the last one manually as the difference
   // between frame height in sb minus the accumulated tiles sb sizes
   if (pAV1Pic->tile_rows == 64)
      tilePartition.RowHeights[63] = pAV1Pic->frame_height_sb - accum_rows_sb;

   // Iterate the tiles and see if they're uniformly partitioned to decide
   // which D3D12 tile mode to use
   // Ignore the last row and last col width
   bool tilesUniform = !D3D12_VIDEO_FORCE_TILE_MODE && util_is_power_of_two_or_zero(tilePartition.RowCount) &&
                       util_is_power_of_two_or_zero(tilePartition.ColCount);
   // Iterate again now that the 63/64 edge case has been handled above.
   for (uint8_t i = 1; tilesUniform && (i < tilePartition.RowCount - 1) /* Ignore last row */; i++)
      tilesUniform = tilesUniform && (tilePartition.RowHeights[i - 1] == tilePartition.RowHeights[i]);

   for (uint8_t i = 1; tilesUniform && (i < tilePartition.ColCount - 1) /* Ignore last col */; i++)
      tilesUniform = tilesUniform && (tilePartition.ColWidths[i - 1] == tilePartition.ColWidths[i]);

   D3D12_VIDEO_ENCODER_FRAME_SUBREGION_LAYOUT_MODE requestedTilesMode =
      tilesUniform ? D3D12_VIDEO_ENCODER_FRAME_SUBREGION_LAYOUT_MODE_UNIFORM_GRID_PARTITION :
                     D3D12_VIDEO_ENCODER_FRAME_SUBREGION_LAYOUT_MODE_CONFIGURABLE_GRID_PARTITION;

   assert(pAV1Pic->num_tile_groups <= 128);   // ARRAY_SIZE(TilesGroups)
   pD3D12Enc->m_currentEncodeConfig.m_encoderSliceConfigDesc.m_TilesConfig_AV1.TilesGroupsCount =
      pAV1Pic->num_tile_groups;
   for (uint8_t i = 0; i < pAV1Pic->num_tile_groups; i++) {
      pD3D12Enc->m_currentEncodeConfig.m_encoderSliceConfigDesc.m_TilesConfig_AV1.TilesGroups[i].tg_start =
         pAV1Pic->tile_groups[i].tile_group_start;
      pD3D12Enc->m_currentEncodeConfig.m_encoderSliceConfigDesc.m_TilesConfig_AV1.TilesGroups[i].tg_end =
         pAV1Pic->tile_groups[i].tile_group_end;
   }

   if (!d3d12_video_encoder_compare_tile_config_av1(
          pD3D12Enc->m_currentEncodeConfig.m_encoderSliceConfigMode,
          requestedTilesMode,
          pD3D12Enc->m_currentEncodeConfig.m_encoderSliceConfigDesc.m_TilesConfig_AV1.TilesPartition,
          tilePartition)) {
      pD3D12Enc->m_currentEncodeConfig.m_ConfigDirtyFlags |= d3d12_video_encoder_config_dirty_flag_slices;
   }

   // Update the encoder state with the tile config
   pD3D12Enc->m_currentEncodeConfig.m_encoderSliceConfigMode = requestedTilesMode;
   pD3D12Enc->m_currentEncodeConfig.m_encoderSliceConfigDesc.m_TilesConfig_AV1.TilesPartition = tilePartition;

   D3D12_FEATURE_DATA_VIDEO_ENCODER_FRAME_SUBREGION_LAYOUT_CONFIG capDataTilesSupport = {};
   capDataTilesSupport.NodeIndex = pD3D12Enc->m_NodeIndex;
   capDataTilesSupport.Codec = D3D12_VIDEO_ENCODER_CODEC_AV1;
   capDataTilesSupport.Profile.DataSize = sizeof(pD3D12Enc->m_currentEncodeConfig.m_encoderProfileDesc.m_AV1Profile);
   capDataTilesSupport.Profile.pAV1Profile = &pD3D12Enc->m_currentEncodeConfig.m_encoderProfileDesc.m_AV1Profile;
   capDataTilesSupport.Level.DataSize = sizeof(pD3D12Enc->m_currentEncodeConfig.m_encoderLevelDesc.m_AV1LevelSetting);
   capDataTilesSupport.Level.pAV1LevelSetting = &pD3D12Enc->m_currentEncodeConfig.m_encoderLevelDesc.m_AV1LevelSetting;
   capDataTilesSupport.FrameResolution.Width = pAV1Pic->frame_width;
   capDataTilesSupport.FrameResolution.Height = pAV1Pic->frame_height;
   capDataTilesSupport.SubregionMode = requestedTilesMode;
   capDataTilesSupport.CodecSupport.DataSize =
      sizeof(pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1TileCaps);
   capDataTilesSupport.CodecSupport.pAV1Support =
      &pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1TileCaps;
   pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1TileCaps.Use128SuperBlocks = false;
   // return units in 64x64 default size
   pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1TileCaps.TilesConfiguration =
      pD3D12Enc->m_currentEncodeConfig.m_encoderSliceConfigDesc.m_TilesConfig_AV1.TilesPartition;
   HRESULT hr =
      pD3D12Enc->m_spD3D12VideoDevice->CheckFeatureSupport(D3D12_FEATURE_VIDEO_ENCODER_FRAME_SUBREGION_LAYOUT_CONFIG,
                                                           &capDataTilesSupport,
                                                           sizeof(capDataTilesSupport));
   if (FAILED(hr) || !capDataTilesSupport.IsSupported) {
      debug_printf("D3D12_FEATURE_VIDEO_ENCODER_SUBREGION_TILES_SUPPORT HR (0x%x) error or IsSupported: (%d).\n",
                   hr,
                   capDataTilesSupport.IsSupported);
      return false;
   }

   return true;
}

D3D12_VIDEO_ENCODER_AV1_CODEC_CONFIGURATION
d3d12_video_encoder_convert_av1_codec_configuration(struct d3d12_video_encoder *pD3D12Enc,
                                                    pipe_av1_enc_picture_desc *pAV1Pic,
                                                    bool &is_supported)
{
   is_supported = true;
   D3D12_VIDEO_ENCODER_AV1_CODEC_CONFIGURATION config = {
      // D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAGS FeatureFlags;
      D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_NONE,
      // UINT OrderHintBitsMinus1;
      pAV1Pic->seq.order_hint_bits - 1,
   };

   //
   // Query AV1 caps and store in m_currentEncodeCapabilities
   //

   D3D12_FEATURE_DATA_VIDEO_ENCODER_CODEC_CONFIGURATION_SUPPORT capCodecConfigData = {};
   capCodecConfigData.NodeIndex = pD3D12Enc->m_NodeIndex;
   capCodecConfigData.Codec = D3D12_VIDEO_ENCODER_CODEC_AV1;
   D3D12_VIDEO_ENCODER_AV1_PROFILE prof =
      d3d12_video_encoder_convert_profile_to_d3d12_enc_profile_av1(pD3D12Enc->base.profile);
   capCodecConfigData.Profile.pAV1Profile = &prof;
   capCodecConfigData.Profile.DataSize = sizeof(prof);
   capCodecConfigData.CodecSupportLimits.pAV1Support =
      &pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps;
   capCodecConfigData.CodecSupportLimits.DataSize =
      sizeof(pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps);

   if (FAILED(
          pD3D12Enc->m_spD3D12VideoDevice->CheckFeatureSupport(D3D12_FEATURE_VIDEO_ENCODER_CODEC_CONFIGURATION_SUPPORT,
                                                               &capCodecConfigData,
                                                               sizeof(capCodecConfigData))) ||
       !capCodecConfigData.IsSupported) {
      debug_printf("D3D12_FEATURE_VIDEO_ENCODER_CODEC_CONFIGURATION_SUPPORT arguments not supported.\n");
      is_supported = false;
      return config;
   }

   // Enable features requested by gallium

   if (pAV1Pic->seq.seq_bits.use_128x128_superblock)
      config.FeatureFlags |= D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_128x128_SUPERBLOCK;
   if (pAV1Pic->seq.seq_bits.enable_filter_intra)
      config.FeatureFlags |= D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_FILTER_INTRA;
   if (pAV1Pic->seq.seq_bits.enable_intra_edge_filter)
      config.FeatureFlags |= D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_INTRA_EDGE_FILTER;
   if (pAV1Pic->seq.seq_bits.enable_interintra_compound)
      config.FeatureFlags |= D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_INTERINTRA_COMPOUND;
   if (pAV1Pic->seq.seq_bits.enable_masked_compound)
      config.FeatureFlags |= D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_MASKED_COMPOUND;
   if (pAV1Pic->seq.seq_bits.enable_warped_motion)
      config.FeatureFlags |= D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_WARPED_MOTION;
   if (pAV1Pic->seq.seq_bits.enable_dual_filter)
      config.FeatureFlags |= D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_DUAL_FILTER;
   if (pAV1Pic->seq.seq_bits.enable_order_hint)
      config.FeatureFlags |= D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_ORDER_HINT_TOOLS;
   if (pAV1Pic->seq.seq_bits.enable_jnt_comp)
      config.FeatureFlags |= D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_JNT_COMP;
   if (pAV1Pic->seq.seq_bits.enable_ref_frame_mvs)
      config.FeatureFlags |= D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_FRAME_REFERENCE_MOTION_VECTORS;
   if (pAV1Pic->seq.seq_bits.enable_superres)
      config.FeatureFlags |= D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_SUPER_RESOLUTION;
   if (pAV1Pic->seq.seq_bits.enable_cdef)
      config.FeatureFlags |= D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_CDEF_FILTERING;
   if (pAV1Pic->seq.seq_bits.enable_restoration)
      config.FeatureFlags |= D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_LOOP_RESTORATION_FILTER;

   // No pipe flag for the following features, associated pic flags can be selected per frame. Enable if supported.

   if ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.SupportedFeatureFlags &
        D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_FORCED_INTEGER_MOTION_VECTORS) != 0)   // seq_force_integer_mv
      config.FeatureFlags |= D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_FORCED_INTEGER_MOTION_VECTORS;

   if ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.SupportedFeatureFlags &
        D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_PALETTE_ENCODING) != 0)
      config.FeatureFlags |= D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_PALETTE_ENCODING;

   if ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.SupportedFeatureFlags &
        D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_INTRA_BLOCK_COPY) != 0)
      config.FeatureFlags |= D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_INTRA_BLOCK_COPY;

   if ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.SupportedFeatureFlags &
        D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_LOOP_FILTER_DELTAS) != 0)
      config.FeatureFlags |= D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_LOOP_FILTER_DELTAS;

   if ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.SupportedFeatureFlags &
        D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_QUANTIZATION_DELTAS) != 0)
      config.FeatureFlags |= D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_QUANTIZATION_DELTAS;

   if ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.SupportedFeatureFlags &
        D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_QUANTIZATION_MATRIX) != 0)
      config.FeatureFlags |= D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_QUANTIZATION_MATRIX;

   if ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.SupportedFeatureFlags &
        D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_REDUCED_TX_SET) != 0)
      config.FeatureFlags |= D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_REDUCED_TX_SET;

   if ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.SupportedFeatureFlags &
        D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_MOTION_MODE_SWITCHABLE) != 0)
      config.FeatureFlags |= D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_MOTION_MODE_SWITCHABLE;

   if ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.SupportedFeatureFlags &
        D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_ALLOW_HIGH_PRECISION_MV) != 0)
      config.FeatureFlags |= D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_ALLOW_HIGH_PRECISION_MV;

   //
   // Add any missing mandatory/required features we didn't enable before
   //
   if ((config.FeatureFlags &
        pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.RequiredFeatureFlags) !=
       pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.RequiredFeatureFlags) {
      debug_printf(
         "[d3d12_video_encoder_convert_av1_codec_configuration] Adding required by caps but not selected already in "
         "config.FeatureFlags...\n");

      if (((config.FeatureFlags & D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_128x128_SUPERBLOCK) == 0) &&
          ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.RequiredFeatureFlags &
            D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_128x128_SUPERBLOCK) != 0)) {
         pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.RequiredNotRequestedFeatureFlags |=
            D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_128x128_SUPERBLOCK;
         debug_printf(
            "[d3d12_video_encoder_convert_av1_codec_configuration] Adding "
            "D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_128x128_SUPERBLOCK required by caps but not selected already in "
            "config.FeatureFlags...\n");
      }

      if (((config.FeatureFlags & D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_FILTER_INTRA) == 0) &&
          ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.RequiredFeatureFlags &
            D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_FILTER_INTRA) != 0)) {
         pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.RequiredNotRequestedFeatureFlags |=
            D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_FILTER_INTRA;
         debug_printf(
            "[d3d12_video_encoder_convert_av1_codec_configuration] "
            "D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_FILTER_INTRA Adding required by caps but not selected already in "
            "config.FeatureFlags...\n");
      }

      if (((config.FeatureFlags & D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_INTRA_EDGE_FILTER) == 0) &&
          ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.RequiredFeatureFlags &
            D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_INTRA_EDGE_FILTER) != 0)) {
         pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.RequiredNotRequestedFeatureFlags |=
            D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_INTRA_EDGE_FILTER;
         debug_printf(
            "[d3d12_video_encoder_convert_av1_codec_configuration] Adding "
            "D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_INTRA_EDGE_FILTER required by caps but not selected already in "
            "config.FeatureFlags...\n");
      }

      if (((config.FeatureFlags & D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_INTERINTRA_COMPOUND) == 0) &&
          ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.RequiredFeatureFlags &
            D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_INTERINTRA_COMPOUND) != 0)) {
         pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.RequiredNotRequestedFeatureFlags |=
            D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_INTERINTRA_COMPOUND;
         debug_printf(
            "[d3d12_video_encoder_convert_av1_codec_configuration] Adding "
            "D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_INTERINTRA_COMPOUND required by caps but not selected already in "
            "config.FeatureFlags...\n");
      }

      if (((config.FeatureFlags & D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_MASKED_COMPOUND) == 0) &&
          ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.RequiredFeatureFlags &
            D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_MASKED_COMPOUND) != 0)) {
         pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.RequiredNotRequestedFeatureFlags |=
            D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_MASKED_COMPOUND;
         debug_printf(
            "[d3d12_video_encoder_convert_av1_codec_configuration] Adding "
            "D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_MASKED_COMPOUND required by caps but not selected already in "
            "config.FeatureFlags...\n");
      }

      if (((config.FeatureFlags & D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_WARPED_MOTION) == 0) &&
          ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.RequiredFeatureFlags &
            D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_WARPED_MOTION) != 0)) {
         pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.RequiredNotRequestedFeatureFlags |=
            D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_WARPED_MOTION;
         debug_printf(
            "[d3d12_video_encoder_convert_av1_codec_configuration] "
            "D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_WARPED_MOTION Adding required by caps but not selected already in "
            "config.FeatureFlags...\n");
      }

      if (((config.FeatureFlags & D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_DUAL_FILTER) == 0) &&
          ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.RequiredFeatureFlags &
            D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_DUAL_FILTER) != 0)) {
         pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.RequiredNotRequestedFeatureFlags |=
            D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_DUAL_FILTER;
         debug_printf("[d3d12_video_encoder_convert_av1_codec_configuration] == Adding required by caps but not "
                      "selected already in "
                      "config.FeatureFlags...\n");
      }

      if (((config.FeatureFlags & D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_JNT_COMP) == 0) &&
          ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.RequiredFeatureFlags &
            D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_JNT_COMP) != 0)) {
         pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.RequiredNotRequestedFeatureFlags |=
            D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_JNT_COMP;
         debug_printf("[d3d12_video_encoder_convert_av1_codec_configuration] 0 Adding required by caps but not "
                      "selected already in "
                      "config.FeatureFlags...\n");
      }

      if (((config.FeatureFlags & D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_FORCED_INTEGER_MOTION_VECTORS) == 0) &&
          ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.RequiredFeatureFlags &
            D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_FORCED_INTEGER_MOTION_VECTORS) != 0)) {
         pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.RequiredNotRequestedFeatureFlags |=
            D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_FORCED_INTEGER_MOTION_VECTORS;
         debug_printf(
            "[d3d12_video_encoder_convert_av1_codec_configuration] Adding required by "
            "D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_FORCED_INTEGER_MOTION_VECTORS caps but not selected already in "
            "config.FeatureFlags...\n");
      }

      if (((config.FeatureFlags & D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_SUPER_RESOLUTION) == 0) &&
          ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.RequiredFeatureFlags &
            D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_SUPER_RESOLUTION) != 0)) {
         pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.RequiredNotRequestedFeatureFlags |=
            D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_SUPER_RESOLUTION;
         debug_printf(
            "[d3d12_video_encoder_convert_av1_codec_configuration] Adding "
            "D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_SUPER_RESOLUTION required by caps but not selected already in "
            "config.FeatureFlags...\n");
      }

      if (((config.FeatureFlags & D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_LOOP_RESTORATION_FILTER) == 0) &&
          ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.RequiredFeatureFlags &
            D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_LOOP_RESTORATION_FILTER) != 0)) {
         pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.RequiredNotRequestedFeatureFlags |=
            D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_LOOP_RESTORATION_FILTER;
         debug_printf(
            "[d3d12_video_encoder_convert_av1_codec_configuration] Adding "
            "D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_LOOP_RESTORATION_FILTER required by caps but not selected already in "
            "config.FeatureFlags...\n");
      }

      if (((config.FeatureFlags & D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_PALETTE_ENCODING) == 0) &&
          ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.RequiredFeatureFlags &
            D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_PALETTE_ENCODING) != 0)) {
         pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.RequiredNotRequestedFeatureFlags |=
            D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_PALETTE_ENCODING;
         debug_printf(
            "[d3d12_video_encoder_convert_av1_codec_configuration] Adding "
            "D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_PALETTE_ENCODING required by caps but not selected already in "
            "config.FeatureFlags...\n");
      }

      if (((config.FeatureFlags & D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_CDEF_FILTERING) == 0) &&
          ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.RequiredFeatureFlags &
            D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_CDEF_FILTERING) != 0)) {
         pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.RequiredNotRequestedFeatureFlags |=
            D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_CDEF_FILTERING;
         debug_printf(
            "[d3d12_video_encoder_convert_av1_codec_configuration] "
            "D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_CDEF_FILTERING Adding required by caps but not selected already in "
            "config.FeatureFlags...\n");
      }

      if (((config.FeatureFlags & D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_INTRA_BLOCK_COPY) == 0) &&
          ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.RequiredFeatureFlags &
            D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_INTRA_BLOCK_COPY) != 0)) {
         pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.RequiredNotRequestedFeatureFlags |=
            D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_INTRA_BLOCK_COPY;
         debug_printf(
            "[d3d12_video_encoder_convert_av1_codec_configuration] Adding "
            "D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_INTRA_BLOCK_COPY required by caps but not selected already in "
            "config.FeatureFlags...\n");
      }

      if (((config.FeatureFlags & D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_FRAME_REFERENCE_MOTION_VECTORS) == 0) &&
          ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.RequiredFeatureFlags &
            D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_FRAME_REFERENCE_MOTION_VECTORS) != 0)) {
         pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.RequiredNotRequestedFeatureFlags |=
            D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_FRAME_REFERENCE_MOTION_VECTORS;
         debug_printf(
            "[d3d12_video_encoder_convert_av1_codec_configuration] Adding required by "
            "D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_FRAME_REFERENCE_MOTION_VECTORS caps but not selected already in "
            "config.FeatureFlags...\n");
      }

      if (((config.FeatureFlags & D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_ORDER_HINT_TOOLS) == 0) &&
          ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.RequiredFeatureFlags &
            D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_ORDER_HINT_TOOLS) != 0)) {
         pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.RequiredNotRequestedFeatureFlags |=
            D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_ORDER_HINT_TOOLS;
         debug_printf(
            "[d3d12_video_encoder_convert_av1_codec_configuration] Adding "
            "D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_ORDER_HINT_TOOLS required by caps but not selected already in "
            "config.FeatureFlags...\n");
      }

      if (((config.FeatureFlags & D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_AUTO_SEGMENTATION) == 0) &&
          ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.RequiredFeatureFlags &
            D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_AUTO_SEGMENTATION) != 0)) {
         pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.RequiredNotRequestedFeatureFlags |=
            D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_AUTO_SEGMENTATION;
         debug_printf(
            "[d3d12_video_encoder_convert_av1_codec_configuration] Adding "
            "D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_AUTO_SEGMENTATION required by caps but not selected already in "
            "config.FeatureFlags...\n");
      }

      if (((config.FeatureFlags & D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_CUSTOM_SEGMENTATION) == 0) &&
          ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.RequiredFeatureFlags &
            D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_CUSTOM_SEGMENTATION) != 0)) {
         pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.RequiredNotRequestedFeatureFlags |=
            D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_CUSTOM_SEGMENTATION;
         debug_printf(
            "[d3d12_video_encoder_convert_av1_codec_configuration] Adding "
            "D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_CUSTOM_SEGMENTATION required by caps but not selected already in "
            "config.FeatureFlags...\n");
      }

      if (((config.FeatureFlags & D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_LOOP_FILTER_DELTAS) == 0) &&
          ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.RequiredFeatureFlags &
            D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_LOOP_FILTER_DELTAS) != 0)) {
         pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.RequiredNotRequestedFeatureFlags |=
            D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_LOOP_FILTER_DELTAS;
         debug_printf(
            "[d3d12_video_encoder_convert_av1_codec_configuration] Adding "
            "D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_LOOP_FILTER_DELTAS required by caps but not selected already in "
            "config.FeatureFlags...\n");
      }

      if (((config.FeatureFlags & D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_QUANTIZATION_DELTAS) == 0) &&
          ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.RequiredFeatureFlags &
            D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_QUANTIZATION_DELTAS) != 0)) {
         pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.RequiredNotRequestedFeatureFlags |=
            D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_QUANTIZATION_DELTAS;
         debug_printf(
            "[d3d12_video_encoder_convert_av1_codec_configuration] Adding "
            "D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_QUANTIZATION_DELTAS required by caps but not selected already in "
            "config.FeatureFlags...\n");
      }

      if (((config.FeatureFlags & D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_QUANTIZATION_MATRIX) == 0) &&
          ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.RequiredFeatureFlags &
            D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_QUANTIZATION_MATRIX) != 0)) {
         pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.RequiredNotRequestedFeatureFlags |=
            D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_QUANTIZATION_MATRIX;
         debug_printf(
            "[d3d12_video_encoder_convert_av1_codec_configuration] Adding "
            "D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_QUANTIZATION_MATRIX required by caps but not selected already in "
            "config.FeatureFlags...\n");
      }

      if (((config.FeatureFlags & D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_REDUCED_TX_SET) == 0) &&
          ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.RequiredFeatureFlags &
            D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_REDUCED_TX_SET) != 0)) {
         pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.RequiredNotRequestedFeatureFlags |=
            D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_REDUCED_TX_SET;
         debug_printf(
            "[d3d12_video_encoder_convert_av1_codec_configuration] "
            "D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_REDUCED_TX_SET Adding required by caps but not selected already in "
            "config.FeatureFlags...\n");
      }

      if (((config.FeatureFlags & D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_MOTION_MODE_SWITCHABLE) == 0) &&
          ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.RequiredFeatureFlags &
            D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_MOTION_MODE_SWITCHABLE) != 0)) {
         pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.RequiredNotRequestedFeatureFlags |=
            D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_MOTION_MODE_SWITCHABLE;
         debug_printf(
            "[d3d12_video_encoder_convert_av1_codec_configuration] Adding "
            "D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_MOTION_MODE_SWITCHABLE required by caps but not selected already in "
            "config.FeatureFlags...\n");
      }

      if (((config.FeatureFlags & D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_ALLOW_HIGH_PRECISION_MV) == 0) &&
          ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.RequiredFeatureFlags &
            D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_ALLOW_HIGH_PRECISION_MV) != 0)) {
         pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.RequiredNotRequestedFeatureFlags |=
            D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_ALLOW_HIGH_PRECISION_MV;
         debug_printf(
            "[d3d12_video_encoder_convert_av1_codec_configuration] Adding "
            "D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_ALLOW_HIGH_PRECISION_MV required by caps but not selected already in "
            "config.FeatureFlags...\n");
      }

      // Enable all required flags previously not selected
      config.FeatureFlags |=
         pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.RequiredFeatureFlags;
   }

   // Check config.FeatureFlags against SupportedFeatureFlags and assign is_supported
   if ((config.FeatureFlags &
        pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.SupportedFeatureFlags) !=
       config.FeatureFlags) {
      debug_printf(
         "[d3d12_video_encoder_convert_av1_codec_configuration] AV1 Configuration flags requested 0x%x not supported "
         "by "
         "m_AV1CodecCaps.SupportedFeatureFlags 0x%x\n",
         config.FeatureFlags,
         pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.SupportedFeatureFlags);

      is_supported = false;
   }

   return config;
}

static bool
d3d12_video_encoder_update_intra_refresh_av1(struct d3d12_video_encoder *pD3D12Enc,
                                                        D3D12_VIDEO_SAMPLE srcTextureDesc,
                                                        struct pipe_av1_enc_picture_desc *  picture)
{
   if (picture->intra_refresh.mode != INTRA_REFRESH_MODE_NONE)
   {
      // D3D12 only supports row intra-refresh
      if (picture->intra_refresh.mode != INTRA_REFRESH_MODE_UNIT_ROWS)
      {
         debug_printf("[d3d12_video_encoder_update_intra_refresh_av1] Unsupported INTRA_REFRESH_MODE %d\n", picture->intra_refresh.mode);
         return false;
      }

      uint32_t sbSize = ((pD3D12Enc->m_currentEncodeConfig.m_encoderCodecSpecificConfigDesc.m_AV1Config.FeatureFlags &
        D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_128x128_SUPERBLOCK) != 0) ? 128u : 64u;
      uint32_t total_frame_blocks = static_cast<uint32_t>(std::ceil(srcTextureDesc.Height / sbSize)) *
                              static_cast<uint32_t>(std::ceil(srcTextureDesc.Width / sbSize));
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
d3d12_video_encoder_update_current_encoder_config_state_av1(struct d3d12_video_encoder *pD3D12Enc,
                                                            D3D12_VIDEO_SAMPLE srcTextureDesc,
                                                            struct pipe_picture_desc *picture)
{
   struct pipe_av1_enc_picture_desc *av1Pic = (struct pipe_av1_enc_picture_desc *) picture;

   // Reset reconfig dirty flags
   pD3D12Enc->m_currentEncodeConfig.m_ConfigDirtyFlags = d3d12_video_encoder_config_dirty_flag_none;
   // Reset sequence changes flags
   pD3D12Enc->m_currentEncodeConfig.m_seqFlags = D3D12_VIDEO_ENCODER_SEQUENCE_CONTROL_FLAG_NONE;

   // Set codec
   if (pD3D12Enc->m_currentEncodeConfig.m_encoderCodecDesc != D3D12_VIDEO_ENCODER_CODEC_AV1) {
      pD3D12Enc->m_currentEncodeConfig.m_ConfigDirtyFlags |= d3d12_video_encoder_config_dirty_flag_codec;
   }
   pD3D12Enc->m_currentEncodeConfig.m_encoderCodecDesc = D3D12_VIDEO_ENCODER_CODEC_AV1;

   // Set input format
   DXGI_FORMAT targetFmt = srcTextureDesc.Format.Format;
   if (pD3D12Enc->m_currentEncodeConfig.m_encodeFormatInfo.Format != targetFmt) {
      pD3D12Enc->m_currentEncodeConfig.m_ConfigDirtyFlags |= d3d12_video_encoder_config_dirty_flag_input_format;
   }

   pD3D12Enc->m_currentEncodeConfig.m_encodeFormatInfo = {};
   pD3D12Enc->m_currentEncodeConfig.m_encodeFormatInfo.Format = targetFmt;
   HRESULT hr =
      pD3D12Enc->m_pD3D12Screen->dev->CheckFeatureSupport(D3D12_FEATURE_FORMAT_INFO,
                                                          &pD3D12Enc->m_currentEncodeConfig.m_encodeFormatInfo,
                                                          sizeof(pD3D12Enc->m_currentEncodeConfig.m_encodeFormatInfo));
   if (FAILED(hr)) {
      debug_printf("CheckFeatureSupport failed with HR 0x%x\n", hr);
      return false;
   }

   // Set resolution (ie. frame_size)
   if ((pD3D12Enc->m_currentEncodeConfig.m_currentResolution.Width != srcTextureDesc.Width) ||
       (pD3D12Enc->m_currentEncodeConfig.m_currentResolution.Height != srcTextureDesc.Height)) {
      pD3D12Enc->m_currentEncodeConfig.m_ConfigDirtyFlags |= d3d12_video_encoder_config_dirty_flag_resolution;
   }
   pD3D12Enc->m_currentEncodeConfig.m_currentResolution.Width = srcTextureDesc.Width;
   pD3D12Enc->m_currentEncodeConfig.m_currentResolution.Height = srcTextureDesc.Height;

   // render_size
   pD3D12Enc->m_currentEncodeConfig.m_FrameCroppingCodecConfig.right = av1Pic->frame_width;
   pD3D12Enc->m_currentEncodeConfig.m_FrameCroppingCodecConfig.bottom = av1Pic->frame_height;

   // Set profile
   auto targetProfile = d3d12_video_encoder_convert_profile_to_d3d12_enc_profile_av1(pD3D12Enc->base.profile);
   if (pD3D12Enc->m_currentEncodeConfig.m_encoderProfileDesc.m_AV1Profile != targetProfile) {
      pD3D12Enc->m_currentEncodeConfig.m_ConfigDirtyFlags |= d3d12_video_encoder_config_dirty_flag_profile;
   }
   pD3D12Enc->m_currentEncodeConfig.m_encoderProfileDesc.m_AV1Profile = targetProfile;

   // Set level and tier
   D3D12_VIDEO_ENCODER_AV1_LEVELS targetLevel = {};
   D3D12_VIDEO_ENCODER_AV1_TIER targetTier = {};
   d3d12_video_encoder_convert_spec_to_d3d12_level_av1(av1Pic->seq.level, targetLevel);
   d3d12_video_encoder_convert_spec_to_d3d12_tier_av1(av1Pic->seq.tier, targetTier);

   if ((pD3D12Enc->m_currentEncodeConfig.m_encoderLevelDesc.m_AV1LevelSetting.Level != targetLevel) ||
       (pD3D12Enc->m_currentEncodeConfig.m_encoderLevelDesc.m_AV1LevelSetting.Tier != targetTier)) {
      pD3D12Enc->m_currentEncodeConfig.m_ConfigDirtyFlags |= d3d12_video_encoder_config_dirty_flag_level;
   }
   pD3D12Enc->m_currentEncodeConfig.m_encoderLevelDesc.m_AV1LevelSetting.Tier = targetTier;
   pD3D12Enc->m_currentEncodeConfig.m_encoderLevelDesc.m_AV1LevelSetting.Level = targetLevel;

   //
   // Validate caps support returned values against current settings
   //
   if (pD3D12Enc->m_currentEncodeConfig.m_encoderProfileDesc.m_AV1Profile !=
       pD3D12Enc->m_currentEncodeCapabilities.m_encoderSuggestedProfileDesc.m_AV1Profile) {
      debug_printf("[d3d12_video_encoder_av1] Warning: Requested D3D12_VIDEO_ENCODER_PROFILE_AV1 by upper layer: %d "
                   "mismatches UMD suggested D3D12_VIDEO_ENCODER_PROFILE_AV1: %d\n",
                   pD3D12Enc->m_currentEncodeConfig.m_encoderProfileDesc.m_AV1Profile,
                   pD3D12Enc->m_currentEncodeCapabilities.m_encoderSuggestedProfileDesc.m_AV1Profile);
   }

   if ((pD3D12Enc->m_currentEncodeConfig.m_encoderLevelDesc.m_AV1LevelSetting.Level !=
        pD3D12Enc->m_currentEncodeCapabilities.m_encoderLevelSuggestedDesc.m_AV1LevelSetting.Level) ||
       (pD3D12Enc->m_currentEncodeConfig.m_encoderLevelDesc.m_AV1LevelSetting.Tier !=
        pD3D12Enc->m_currentEncodeCapabilities.m_encoderLevelSuggestedDesc.m_AV1LevelSetting.Tier)) {
      debug_printf(
         "[d3d12_video_encoder_av1] Warning: Requested D3D12_VIDEO_ENCODER_LEVELS_AV1 by upper layer: level %d tier %d "
         "mismatches UMD suggested D3D12_VIDEO_ENCODER_LEVELS_AV1: level %d tier %d\n",
         pD3D12Enc->m_currentEncodeConfig.m_encoderLevelDesc.m_AV1LevelSetting.Level,
         pD3D12Enc->m_currentEncodeConfig.m_encoderLevelDesc.m_AV1LevelSetting.Tier,
         pD3D12Enc->m_currentEncodeCapabilities.m_encoderLevelSuggestedDesc.m_AV1LevelSetting.Level,
         pD3D12Enc->m_currentEncodeCapabilities.m_encoderLevelSuggestedDesc.m_AV1LevelSetting.Tier);
   }

   // Set codec config
   bool is_supported = false;
   auto targetCodecConfig = d3d12_video_encoder_convert_av1_codec_configuration(pD3D12Enc, av1Pic, is_supported);
   if (!is_supported) {
      return false;
   }

   if (memcmp(&pD3D12Enc->m_currentEncodeConfig.m_encoderCodecSpecificConfigDesc.m_AV1Config,
              &targetCodecConfig,
              sizeof(D3D12_VIDEO_ENCODER_AV1_CODEC_CONFIGURATION)) != 0) {
      pD3D12Enc->m_currentEncodeConfig.m_ConfigDirtyFlags |= d3d12_video_encoder_config_dirty_flag_codec_config;
   }
   pD3D12Enc->m_currentEncodeConfig.m_encoderCodecSpecificConfigDesc.m_AV1Config = targetCodecConfig;

   // Set rate control
   d3d12_video_encoder_update_current_rate_control_av1(pD3D12Enc, av1Pic);

   // Set tiles config
   if (!d3d12_video_encoder_negotiate_current_av1_tiles_configuration(pD3D12Enc, av1Pic)) {
      debug_printf("d3d12_video_encoder_negotiate_current_av1_tiles_configuration failed!\n");
      return false;
   }

   // Set GOP config
   if (!d3d12_video_encoder_update_av1_gop_configuration(pD3D12Enc, av1Pic)) {
      debug_printf("d3d12_video_encoder_update_av1_gop_configuration failed!\n");
      return false;
   }

   // Set intra-refresh config
   if(!d3d12_video_encoder_update_intra_refresh_av1(pD3D12Enc, srcTextureDesc, av1Pic)) {
      debug_printf("d3d12_video_encoder_update_intra_refresh_av1 failed!\n");
      return false;
   }

   // m_currentEncodeConfig.m_encoderPicParamsDesc pic params are set in d3d12_video_encoder_reconfigure_encoder_objects
   // after re-allocating objects if needed

   // Set motion estimation config
   auto targetMotionLimit = d3d12_video_encoder_convert_av1_motion_configuration(pD3D12Enc, av1Pic);
   if (pD3D12Enc->m_currentEncodeConfig.m_encoderMotionPrecisionLimit != targetMotionLimit) {
      pD3D12Enc->m_currentEncodeConfig.m_ConfigDirtyFlags |=
         d3d12_video_encoder_config_dirty_flag_motion_precision_limit;
   }
   pD3D12Enc->m_currentEncodeConfig.m_encoderMotionPrecisionLimit = targetMotionLimit;

   // Will call for d3d12 driver support based on the initial requested (non codec specific) features, then
   // try to fallback if any of them is not supported and return the negotiated d3d12 settings
   D3D12_FEATURE_DATA_VIDEO_ENCODER_SUPPORT1 capEncoderSupportData1 = {};
   if (!d3d12_video_encoder_negotiate_requested_features_and_d3d12_driver_caps(pD3D12Enc, capEncoderSupportData1)) {
      debug_printf("[d3d12_video_encoder_av1] After negotiating caps, D3D12_FEATURE_VIDEO_ENCODER_SUPPORT1 "
                   "arguments are not supported - "
                   "ValidationFlags: 0x%x - SupportFlags: 0x%x\n",
                   capEncoderSupportData1.ValidationFlags,
                   capEncoderSupportData1.SupportFlags);
      return false;
   }

   pD3D12Enc->m_currentEncodeCapabilities.m_MaxSlicesInOutput = (av1Pic->tile_cols * av1Pic->tile_rows);
   if (pD3D12Enc->m_currentEncodeCapabilities.m_MaxSlicesInOutput >
       pD3D12Enc->m_currentEncodeCapabilities.m_currentResolutionSupportCaps.MaxSubregionsNumber) {
      debug_printf("[d3d12_video_encoder_av1] Desired number of subregions is not supported (higher than max "
                   "reported slice number in query caps)\n.");
      return false;
   }

   return true;
}

/*
 * Called at begin_frame record time
 */
void
d3d12_video_encoder_update_current_frame_pic_params_info_av1(struct d3d12_video_encoder *pD3D12Enc,
                                                             struct pipe_video_buffer *srcTexture,
                                                             struct pipe_picture_desc *picture,
                                                             D3D12_VIDEO_ENCODER_PICTURE_CONTROL_CODEC_DATA &picParams,
                                                             bool &bUsedAsReference)
{
   struct pipe_av1_enc_picture_desc *pAV1Pic = (struct pipe_av1_enc_picture_desc *) picture;

   // Output param bUsedAsReference
   bUsedAsReference = (pAV1Pic->refresh_frame_flags != 0);

   // D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAGS Flags;
   picParams.pAV1PicData->Flags = D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAG_NONE;

   if (pAV1Pic->error_resilient_mode)
      picParams.pAV1PicData->Flags |= D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAG_ENABLE_ERROR_RESILIENT_MODE;

   if (pAV1Pic->disable_cdf_update)
      picParams.pAV1PicData->Flags |= D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAG_DISABLE_CDF_UPDATE;

   if (pAV1Pic->palette_mode_enable)
      picParams.pAV1PicData->Flags |= D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAG_ENABLE_PALETTE_ENCODING;

   // Override if required feature
   if ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.RequiredFeatureFlags &
        D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_PALETTE_ENCODING) != 0) {
      debug_printf("[d3d12_video_encoder_update_current_frame_pic_params_info_av1] Overriding required feature "
                   "D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAG_ENABLE_PALETTE_ENCODING\n");
      picParams.pAV1PicData->Flags |= D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAG_ENABLE_PALETTE_ENCODING;
   }

   if (pAV1Pic->skip_mode_present)
      picParams.pAV1PicData->Flags |= D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAG_ENABLE_SKIP_MODE;

   if ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.RequiredFeatureFlags &
        D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_SKIP_MODE_PRESENT) != 0) {
      debug_printf("[d3d12_video_encoder_update_current_frame_pic_params_info_av1] Overriding required feature "
                   "D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAG_ENABLE_SKIP_MODE\n");
      picParams.pAV1PicData->Flags |= D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAG_ENABLE_SKIP_MODE;
   }

   if (pAV1Pic->use_ref_frame_mvs)
      picParams.pAV1PicData->Flags |= D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAG_FRAME_REFERENCE_MOTION_VECTORS;

   // Override if required feature
   if ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.RequiredFeatureFlags &
        D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_FRAME_REFERENCE_MOTION_VECTORS) != 0) {
      debug_printf("[d3d12_video_encoder_update_current_frame_pic_params_info_av1] Overriding required feature "
                   "D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAG_FRAME_REFERENCE_MOTION_VECTORS\n");
      picParams.pAV1PicData->Flags |= D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAG_FRAME_REFERENCE_MOTION_VECTORS;
   }

   // No pipe flag for force_integer_mv (D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAG_FORCE_INTEGER_MOTION_VECTORS)
   // choose default based on required/supported underlying codec flags
   if ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.RequiredFeatureFlags &
        D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_FORCED_INTEGER_MOTION_VECTORS) != 0) {
      debug_printf("[d3d12_video_encoder_update_current_frame_pic_params_info_av1] Overriding required feature "
                   "D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAG_FORCE_INTEGER_MOTION_VECTORS\n");
      picParams.pAV1PicData->Flags |= D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAG_FORCE_INTEGER_MOTION_VECTORS;
   }

   if (pAV1Pic->allow_intrabc)
      picParams.pAV1PicData->Flags |= D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAG_ALLOW_INTRA_BLOCK_COPY;

   // Override if required feature
   if ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.RequiredFeatureFlags &
        D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_INTRA_BLOCK_COPY) != 0) {
      debug_printf("[d3d12_video_encoder_update_current_frame_pic_params_info_av1] Overriding required feature "
                   "D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAG_ALLOW_INTRA_BLOCK_COPY\n");
      picParams.pAV1PicData->Flags |= D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAG_ALLOW_INTRA_BLOCK_COPY;
   }

   if (pAV1Pic->use_superres)
      picParams.pAV1PicData->Flags |= D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAG_USE_SUPER_RESOLUTION;

   if (pAV1Pic->disable_frame_end_update_cdf)
      picParams.pAV1PicData->Flags |= D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAG_DISABLE_FRAME_END_UPDATE_CDF;

   // No pipe flag for D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAG_ENABLE_FRAME_SEGMENTATION_AUTO
   // choose default based on required/supported underlying codec flags
   if ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.RequiredFeatureFlags &
        D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_AUTO_SEGMENTATION) != 0) {
      debug_printf("[d3d12_video_encoder_update_current_frame_pic_params_info_av1] Overriding required feature "
                   "D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAG_ENABLE_FRAME_SEGMENTATION_AUTO\n");
      picParams.pAV1PicData->Flags |= D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAG_ENABLE_FRAME_SEGMENTATION_AUTO;
   }

   // No pipe flag for D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAG_ENABLE_FRAME_SEGMENTATION_CUSTOM
   // choose default based on required/supported underlying codec flags
   if ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.RequiredFeatureFlags &
        D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_CUSTOM_SEGMENTATION) != 0) {
      debug_printf("[d3d12_video_encoder_update_current_frame_pic_params_info_av1] Overriding required feature "
                   "D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAG_ENABLE_FRAME_SEGMENTATION_CUSTOM\n");
      picParams.pAV1PicData->Flags |= D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAG_ENABLE_FRAME_SEGMENTATION_CUSTOM;
      assert(false);   // Not implemented
   }

   // No pipe flag for allow_warped_motion (D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAG_ENABLE_WARPED_MOTION)
   // choose default based on required/supported underlying codec flags
   if ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.RequiredFeatureFlags &
        D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_WARPED_MOTION) != 0) {
      debug_printf("[d3d12_video_encoder_update_current_frame_pic_params_info_av1] Overriding required feature "
                   "D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAG_ENABLE_WARPED_MOTION\n");
      picParams.pAV1PicData->Flags |= D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAG_ENABLE_WARPED_MOTION;
   }

   // Only enable if supported (there is no PIPE/VA cap flag for reduced_tx_set)
   if ((pAV1Pic->reduced_tx_set) &&
       (pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.SupportedFeatureFlags &
        D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_REDUCED_TX_SET) != 0) {
      picParams.pAV1PicData->Flags |= D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAG_REDUCED_TX_SET;
   }

   // Override if required feature
   if ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.RequiredFeatureFlags &
        D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_REDUCED_TX_SET) != 0) {
      debug_printf("[d3d12_video_encoder_update_current_frame_pic_params_info_av1] Overriding required feature "
                   "D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAG_REDUCED_TX_SET\n");
      picParams.pAV1PicData->Flags |= D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAG_REDUCED_TX_SET;
   }

   // Only enable if supported
   if ((pAV1Pic->allow_high_precision_mv) &&
       (pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.SupportedFeatureFlags &
        D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_ALLOW_HIGH_PRECISION_MV) != 0) {
      picParams.pAV1PicData->Flags |= D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAG_ALLOW_HIGH_PRECISION_MV;
   }

   // Override if required feature
   if ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.RequiredFeatureFlags &
        D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_ALLOW_HIGH_PRECISION_MV) != 0) {
      debug_printf("[d3d12_video_encoder_update_current_frame_pic_params_info_av1] Overriding required feature "
                   "D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAG_ALLOW_HIGH_PRECISION_MV\n");
      picParams.pAV1PicData->Flags |= D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAG_ALLOW_HIGH_PRECISION_MV;
   }

   // No pipe flag for is_motion_mode_switchable (D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAG_MOTION_MODE_SWITCHABLE)
   // choose default based on required/supported underlying codec flags
   if ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.RequiredFeatureFlags &
        D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_MOTION_MODE_SWITCHABLE) != 0) {
      debug_printf("[d3d12_video_encoder_update_current_frame_pic_params_info_av1] Overriding required feature "
                   "D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAG_MOTION_MODE_SWITCHABLE\n");
      picParams.pAV1PicData->Flags |= D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAG_MOTION_MODE_SWITCHABLE;
   }

   // D3D12_VIDEO_ENCODER_AV1_FRAME_TYPE FrameType;
   // AV1 spec matches w/D3D12 enum definition
   picParams.pAV1PicData->FrameType = static_cast<D3D12_VIDEO_ENCODER_AV1_FRAME_TYPE>(pAV1Pic->frame_type);

   if (picParams.pAV1PicData->FrameType == D3D12_VIDEO_ENCODER_AV1_FRAME_TYPE_KEY_FRAME)
      debug_printf("Encoding FrameType: D3D12_VIDEO_ENCODER_AV1_FRAME_TYPE_KEY_FRAME\n");
   if (picParams.pAV1PicData->FrameType == D3D12_VIDEO_ENCODER_AV1_FRAME_TYPE_INTER_FRAME)
      debug_printf("Encoding FrameType: D3D12_VIDEO_ENCODER_AV1_FRAME_TYPE_INTER_FRAME\n");
   if (picParams.pAV1PicData->FrameType == D3D12_VIDEO_ENCODER_AV1_FRAME_TYPE_INTRA_ONLY_FRAME)
      debug_printf("Encoding FrameType: D3D12_VIDEO_ENCODER_AV1_FRAME_TYPE_INTRA_ONLY_FRAME\n");
   if (picParams.pAV1PicData->FrameType == D3D12_VIDEO_ENCODER_AV1_FRAME_TYPE_SWITCH_FRAME)
      debug_printf("Encoding FrameType: D3D12_VIDEO_ENCODER_AV1_FRAME_TYPE_SWITCH_FRAME\n");

   // D3D12_VIDEO_ENCODER_AV1_COMP_PREDICTION_TYPE CompoundPredictionType;
   picParams.pAV1PicData->CompoundPredictionType = (pAV1Pic->compound_reference_mode == 0) ?
                                                      D3D12_VIDEO_ENCODER_AV1_COMP_PREDICTION_TYPE_SINGLE_REFERENCE :
                                                      D3D12_VIDEO_ENCODER_AV1_COMP_PREDICTION_TYPE_COMPOUND_REFERENCE;

   // D3D12_VIDEO_ENCODER_AV1_INTERPOLATION_FILTERS InterpolationFilter;
   // AV1 spec matches w/D3D12 enum definition
   picParams.pAV1PicData->InterpolationFilter =
      static_cast<D3D12_VIDEO_ENCODER_AV1_INTERPOLATION_FILTERS>(pAV1Pic->interpolation_filter);

   // Workaround for apps sending interpolation_filter values not supported even when reporting
   // them in pipe_av1_enc_cap_features_ext1.interpolation_filter. If D3D12 driver doesn't support
   // requested InterpolationFilter, fallback to the first supported by D3D12 driver
   if ( (pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.SupportedInterpolationFilters &
      (1 << picParams.pAV1PicData->InterpolationFilter)) == 0 ) { /* See definition of D3D12_VIDEO_ENCODER_AV1_INTERPOLATION_FILTERS_FLAGS */
      debug_printf("[d3d12_video_encoder_update_current_frame_pic_params_info_av1] Requested interpolation_filter"
                   " not supported in pipe_av1_enc_cap_features_ext1.interpolation_filter"
                   ", auto selecting from D3D12 SupportedInterpolationFilters...");
      for(uint8_t i = D3D12_VIDEO_ENCODER_AV1_INTERPOLATION_FILTERS_EIGHTTAP; i <= D3D12_VIDEO_ENCODER_AV1_INTERPOLATION_FILTERS_SWITCHABLE; i++) {
         if ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.SupportedInterpolationFilters &
             (1 << i)) /* See definition of D3D12_VIDEO_ENCODER_AV1_INTERPOLATION_FILTERS_FLAGS */ != 0) {
            picParams.pAV1PicData->InterpolationFilter = static_cast<D3D12_VIDEO_ENCODER_AV1_INTERPOLATION_FILTERS>(i);
            break;
         }
      }
   }

   // D3D12_VIDEO_ENCODER_AV1_RESTORATION_CONFIG FrameRestorationConfig;
   // AV1 spec matches w/D3D12 FrameRestorationType enum definition

   picParams.pAV1PicData->FrameRestorationConfig.FrameRestorationType[0] =
      static_cast<D3D12_VIDEO_ENCODER_AV1_RESTORATION_TYPE>(pAV1Pic->restoration.yframe_restoration_type);
   picParams.pAV1PicData->FrameRestorationConfig.FrameRestorationType[1] =
      static_cast<D3D12_VIDEO_ENCODER_AV1_RESTORATION_TYPE>(pAV1Pic->restoration.cbframe_restoration_type);
   picParams.pAV1PicData->FrameRestorationConfig.FrameRestorationType[2] =
      static_cast<D3D12_VIDEO_ENCODER_AV1_RESTORATION_TYPE>(pAV1Pic->restoration.crframe_restoration_type);

   if (picParams.pAV1PicData->FrameRestorationConfig.FrameRestorationType[0] !=
       D3D12_VIDEO_ENCODER_AV1_RESTORATION_TYPE_DISABLED) {
      picParams.pAV1PicData->FrameRestorationConfig.LoopRestorationPixelSize[0] =
         d3d12_video_encoder_looprestorationsize_uint_to_d3d12_av1(1 << (6 + pAV1Pic->restoration.lr_unit_shift));
   }

   if (picParams.pAV1PicData->FrameRestorationConfig.FrameRestorationType[1] !=
       D3D12_VIDEO_ENCODER_AV1_RESTORATION_TYPE_DISABLED) {
      picParams.pAV1PicData->FrameRestorationConfig.LoopRestorationPixelSize[1] =
         d3d12_video_encoder_looprestorationsize_uint_to_d3d12_av1(
            1 << (6 + pAV1Pic->restoration.lr_unit_shift - pAV1Pic->restoration.lr_uv_shift));
   }

   if (picParams.pAV1PicData->FrameRestorationConfig.FrameRestorationType[2] !=
       D3D12_VIDEO_ENCODER_AV1_RESTORATION_TYPE_DISABLED) {
      picParams.pAV1PicData->FrameRestorationConfig.LoopRestorationPixelSize[2] =
         d3d12_video_encoder_looprestorationsize_uint_to_d3d12_av1(
            1 << (6 + pAV1Pic->restoration.lr_unit_shift - pAV1Pic->restoration.lr_uv_shift));
   }

   // D3D12_VIDEO_ENCODER_AV1_TX_MODE TxMode;
   // AV1 spec matches w/D3D12 enum definition
   picParams.pAV1PicData->TxMode = static_cast<D3D12_VIDEO_ENCODER_AV1_TX_MODE>(pAV1Pic->tx_mode);

   // Workaround for mismatch between VAAPI/D3D12 and TxMode support for all/some frame types
   // If D3D12 driver doesn't support requested TxMode, fallback to the first supported by D3D12
   // driver for the requested frame type
   if ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.SupportedTxModes[picParams.pAV1PicData->FrameType] &
      (1 << picParams.pAV1PicData->TxMode)) == 0) /* See definition of D3D12_VIDEO_ENCODER_AV1_TX_MODE_FLAGS */ {
      debug_printf("[d3d12_video_encoder_update_current_frame_pic_params_info_av1] Requested tx_mode not supported"
                     ", auto selecting from D3D12 SupportedTxModes for current frame type...");
      for(uint8_t i = D3D12_VIDEO_ENCODER_AV1_TX_MODE_ONLY4x4; i <= D3D12_VIDEO_ENCODER_AV1_TX_MODE_SELECT; i++) {
         if ((pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1CodecCaps.SupportedTxModes[picParams.pAV1PicData->FrameType] &
             (1 << i)) /* See definition of D3D12_VIDEO_ENCODER_AV1_TX_MODE_FLAGS */ != 0) {
            picParams.pAV1PicData->TxMode = static_cast<D3D12_VIDEO_ENCODER_AV1_TX_MODE>(i);
            break;
         }
      }
   }

   // UINT SuperResDenominator;
   picParams.pAV1PicData->SuperResDenominator = pAV1Pic->superres_scale_denominator;

   // UINT OrderHint;
   picParams.pAV1PicData->OrderHint = pAV1Pic->order_hint;

   // UINT PictureIndex - Substract the last_key_frame_num to make it modulo KEY frame
   picParams.pAV1PicData->PictureIndex = pAV1Pic->frame_num - pAV1Pic->last_key_frame_num;

   // UINT TemporalLayerIndexPlus1;
   assert(pAV1Pic->temporal_id == pAV1Pic->tg_obu_header.temporal_id);
   picParams.pAV1PicData->TemporalLayerIndexPlus1 = pAV1Pic->temporal_id + 1;

   // UINT SpatialLayerIndexPlus1;
   picParams.pAV1PicData->SpatialLayerIndexPlus1 = pAV1Pic->tg_obu_header.spatial_id + 1;

   //
   // Reference Pictures
   //
   {
      for (uint8_t i = 0; i < ARRAY_SIZE(picParams.pAV1PicData->ReferenceIndices); i++) {
         picParams.pAV1PicData->ReferenceIndices[i] = pAV1Pic->ref_frame_idx[i];
      }

      bool FrameIsIntra = (picParams.pAV1PicData->FrameType == D3D12_VIDEO_ENCODER_AV1_FRAME_TYPE_INTRA_ONLY_FRAME ||
                           picParams.pAV1PicData->FrameType == D3D12_VIDEO_ENCODER_AV1_FRAME_TYPE_KEY_FRAME);
      if (FrameIsIntra)
         picParams.pAV1PicData->PrimaryRefFrame = 7; /* PRIMARY_REF_NONE */
      else
         picParams.pAV1PicData->PrimaryRefFrame = pAV1Pic->primary_ref_frame;
      debug_printf("App requested primary_ref_frame: %" PRIu32 "\n", pAV1Pic->primary_ref_frame);
      picParams.pAV1PicData->RefreshFrameFlags = pAV1Pic->refresh_frame_flags;
   }

   // D3D12_VIDEO_ENCODER_CODEC_AV1_LOOP_FILTER_CONFIG LoopFilter;
   picParams.pAV1PicData->LoopFilter.LoopFilterLevel[0] = pAV1Pic->loop_filter.filter_level[0];
   picParams.pAV1PicData->LoopFilter.LoopFilterLevel[1] = pAV1Pic->loop_filter.filter_level[1];
   picParams.pAV1PicData->LoopFilter.LoopFilterLevelU = pAV1Pic->loop_filter.filter_level_u;
   picParams.pAV1PicData->LoopFilter.LoopFilterLevelV = pAV1Pic->loop_filter.filter_level_v;
   picParams.pAV1PicData->LoopFilter.LoopFilterSharpnessLevel = pAV1Pic->loop_filter.sharpness_level;
   picParams.pAV1PicData->LoopFilter.LoopFilterDeltaEnabled = pAV1Pic->loop_filter.mode_ref_delta_enabled;
   picParams.pAV1PicData->LoopFilter.UpdateRefDelta = pAV1Pic->loop_filter.mode_ref_delta_update;
   if (picParams.pAV1PicData->LoopFilter.UpdateRefDelta) {
      for (uint8_t i = 0; i < ARRAY_SIZE(picParams.pAV1PicData->LoopFilter.RefDeltas); i++) {
         picParams.pAV1PicData->LoopFilter.RefDeltas[i] = pAV1Pic->loop_filter.ref_deltas[i];
      }
   }
   picParams.pAV1PicData->LoopFilter.UpdateModeDelta = pAV1Pic->loop_filter.mode_ref_delta_update;
   if (picParams.pAV1PicData->LoopFilter.UpdateModeDelta) {
      for (uint8_t i = 0; i < ARRAY_SIZE(picParams.pAV1PicData->LoopFilter.ModeDeltas); i++) {
         picParams.pAV1PicData->LoopFilter.ModeDeltas[i] = pAV1Pic->loop_filter.mode_deltas[i];
      }
   }

   // D3D12_VIDEO_ENCODER_CODEC_AV1_LOOP_FILTER_DELTA_CONFIG LoopFilterDelta;
   picParams.pAV1PicData->LoopFilterDelta.DeltaLFMulti = pAV1Pic->loop_filter.delta_lf_multi;
   picParams.pAV1PicData->LoopFilterDelta.DeltaLFPresent = pAV1Pic->loop_filter.delta_lf_present;
   picParams.pAV1PicData->LoopFilterDelta.DeltaLFRes = pAV1Pic->loop_filter.delta_lf_res;

   // D3D12_VIDEO_ENCODER_CODEC_AV1_QUANTIZATION_CONFIG Quantization;
   picParams.pAV1PicData->Quantization.BaseQIndex = pAV1Pic->quantization.base_qindex;
   picParams.pAV1PicData->Quantization.YDCDeltaQ = pAV1Pic->quantization.y_dc_delta_q;
   picParams.pAV1PicData->Quantization.UDCDeltaQ = pAV1Pic->quantization.u_dc_delta_q;
   picParams.pAV1PicData->Quantization.UACDeltaQ = pAV1Pic->quantization.u_ac_delta_q;
   picParams.pAV1PicData->Quantization.VDCDeltaQ = pAV1Pic->quantization.v_dc_delta_q;
   picParams.pAV1PicData->Quantization.VACDeltaQ = pAV1Pic->quantization.v_ac_delta_q;
   picParams.pAV1PicData->Quantization.UsingQMatrix = pAV1Pic->quantization.using_qmatrix;
   picParams.pAV1PicData->Quantization.QMY = pAV1Pic->quantization.qm_y;
   picParams.pAV1PicData->Quantization.QMU = pAV1Pic->quantization.qm_u;
   picParams.pAV1PicData->Quantization.QMV = pAV1Pic->quantization.qm_v;

   // D3D12_VIDEO_ENCODER_CODEC_AV1_QUANTIZATION_DELTA_CONFIG QuantizationDelta;
   picParams.pAV1PicData->QuantizationDelta.DeltaQPresent = pAV1Pic->quantization.delta_q_present;
   picParams.pAV1PicData->QuantizationDelta.DeltaQRes = pAV1Pic->quantization.delta_q_res;

   // D3D12_VIDEO_ENCODER_AV1_CDEF_CONFIG CDEF;
   picParams.pAV1PicData->CDEF.CdefBits = pAV1Pic->cdef.cdef_bits;
   picParams.pAV1PicData->CDEF.CdefDampingMinus3 = pAV1Pic->cdef.cdef_damping_minus_3;
   for (uint32_t i = 0; i < 8; i++) {
      picParams.pAV1PicData->CDEF.CdefYPriStrength[i] = (pAV1Pic->cdef.cdef_y_strengths[i] >> 2);
      picParams.pAV1PicData->CDEF.CdefYSecStrength[i] = (pAV1Pic->cdef.cdef_y_strengths[i] & 0x03);
      picParams.pAV1PicData->CDEF.CdefUVPriStrength[i] = (pAV1Pic->cdef.cdef_uv_strengths[i] >> 2);
      picParams.pAV1PicData->CDEF.CdefUVSecStrength[i] = (pAV1Pic->cdef.cdef_uv_strengths[i] & 0x03);
   }

   //
   // Set values for mandatory but not requested features (ie. RequiredFeature flags reported by driver not requested
   // by pipe)
   //

   // For the ones that are more trivial (ie. 128x128 SB) than enabling their flag, and have more parameters to be
   // set (ie Restoration Filter) Set defaults based on such feature driver d3d12 caps

   // These are a trivial flag enabling
   //  D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_128x128_SUPERBLOCK
   //  D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_FILTER_INTRA
   //  D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_INTRA_EDGE_FILTER
   //  D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_INTERINTRA_COMPOUND
   //  D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_MASKED_COMPOUND
   //  D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_WARPED_MOTION
   //  D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_DUAL_FILTER
   //  D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_JNT_COMP
   //  D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_FORCED_INTEGER_MOTION_VECTORS
   //  D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_SUPER_RESOLUTION
   //  D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_PALETTE_ENCODING
   //  D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_INTRA_BLOCK_COPY
   //  D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_FRAME_REFERENCE_MOTION_VECTORS
   //  D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_ORDER_HINT_TOOLS
   //  D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_REDUCED_TX_SET
   //  D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_MOTION_MODE_SWITCHABLE
   //  D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_ALLOW_HIGH_PRECISION_MV

   // If driver requires these, can use post encode values to enforce specific values
   // D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_CDEF_FILTERING
   // D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_AUTO_SEGMENTATION // Only useful with post encode values as driver
   // controls D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_CUSTOM_SEGMENTATION // There are more Segmentation caps in
   // D3D12_VIDEO_ENCODER_AV1_CODEC_CONFIGURATION_SUPPORT for these one too
   // D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_LOOP_FILTER_DELTAS
   // D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_QUANTIZATION_DELTAS
   // D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_QUANTIZATION_MATRIX

   // If driver requires this one, use driver restoration caps to set default values
   // D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_LOOP_RESTORATION_FILTER
   if (pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps.RequiredNotRequestedFeatureFlags &
       D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_LOOP_RESTORATION_FILTER) {
      debug_printf(
         "[d3d12_video_encoder_update_current_frame_pic_params_info_av1] Adding default params for "
         "D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_LOOP_RESTORATION_FILTER required by caps but not selected already in "
         "config.FeatureFlags...\n");

      // Set Y, U, V luma plane restoration params
      for (uint32_t planeIdx = 0; planeIdx < 3; planeIdx++) {
         // Let's see which filters and with which sizes are supported for planeIdx
         bool foundSupportedFilter = false;
         for (uint32_t filterIdx = 0; ((filterIdx < 3) && !foundSupportedFilter); filterIdx++) {
            auto curFilterSupport = pD3D12Enc->m_currentEncodeCapabilities.m_encoderCodecSpecificConfigCaps
                                       .m_AV1CodecCaps.SupportedRestorationParams[filterIdx][planeIdx];
            for (uint32_t curFilterSize = D3D12_VIDEO_ENCODER_AV1_RESTORATION_TILESIZE_32x32;
                 ((curFilterSize <= D3D12_VIDEO_ENCODER_AV1_RESTORATION_TILESIZE_256x256) && !foundSupportedFilter);
                 curFilterSize++) {

               // Check if there's support for curFilter on CurPlane and choose the first supported restoration size
               //  If yes, set restoration params for planeIdx
               if (curFilterSupport &
                   (1 << (curFilterSize - 1))) { /* Converts D3D12_VIDEO_ENCODER_AV1_RESTORATION_TILESIZE definition
                                        into D3D12_VIDEO_ENCODER_AV1_RESTORATION_SUPPORT_FLAGS definition */
                  {
                     foundSupportedFilter = true;
                     // As per d3d12 spec filterIdx corresponds to D3D12_VIDEO_ENCODER_AV1_RESTORATION_TYPE - 1
                     // (SupportedRestorationParams definition skips D3D12_VIDEO_ENCODER_AV1_RESTORATION_TYPE_DISABLED)
                     D3D12_VIDEO_ENCODER_AV1_RESTORATION_TYPE curFilter =
                        static_cast<D3D12_VIDEO_ENCODER_AV1_RESTORATION_TYPE>(filterIdx + 1);

                     picParams.pAV1PicData->FrameRestorationConfig.FrameRestorationType[planeIdx] = curFilter;
                     picParams.pAV1PicData->FrameRestorationConfig.LoopRestorationPixelSize[planeIdx] =
                        static_cast<D3D12_VIDEO_ENCODER_AV1_RESTORATION_TILESIZE>(
                           curFilterSize); /* loop uses enum type */
                  }
               }
            }
         }
      }
   }

   // Save state snapshot from record time to resolve headers at get_feedback time
   uint64_t current_metadata_slot = (pD3D12Enc->m_fenceValue % D3D12_VIDEO_ENC_METADATA_BUFFERS_COUNT);
   pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].m_associatedEncodeCapabilities =
      pD3D12Enc->m_currentEncodeCapabilities;
   pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].m_associatedEncodeConfig =
      pD3D12Enc->m_currentEncodeConfig;
   pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].m_CodecSpecificData.AV1HeadersInfo.enable_frame_obu =
      pAV1Pic->enable_frame_obu;
   pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].m_CodecSpecificData.AV1HeadersInfo.obu_has_size_field =
      (pAV1Pic->tg_obu_header.obu_has_size_field == 1);
   // Disabling for now as the libva spec does not allow these but some apps send down anyway. It's possible in the future 
   // the libva spec may be retro-fitted to allow this given existing apps in the wild doing it.
   // pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot]
   //   .m_CodecSpecificData.AV1HeadersInfo.temporal_delim_rendered = pAV1Pic->temporal_delim_rendered;

   if ((pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_Flags & D3D12_VIDEO_ENCODER_RATE_CONTROL_FLAG_ENABLE_DELTA_QP) != 0)
   {
      // Use 16 bit qpmap array for AV1 picparams (-255, 255 range and int16_t pRateControlQPMap type)
      const int32_t av1_min_delta_qp = -255;
      const int32_t av1_max_delta_qp = 255;
      d3d12_video_encoder_update_picparams_region_of_interest_qpmap(
         pD3D12Enc,
         &pAV1Pic->roi,
         av1_min_delta_qp,
         av1_max_delta_qp,
         pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_pRateControlQPMap16Bit);
      picParams.pAV1PicData->pRateControlQPMap = pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_pRateControlQPMap16Bit.data();
      picParams.pAV1PicData->QPMapValuesCount = pD3D12Enc->m_currentEncodeConfig.m_encoderRateControlDesc.m_pRateControlQPMap16Bit.size();
   }
}

void
fill_av1_seq_header(EncodedBitstreamResolvedMetadata &associatedMetadata, av1_seq_header_t *seq_header)
{
   // Set all zero by default
   memset(seq_header, 0, sizeof(av1_seq_header_t));

   seq_header->seq_profile = d3d12_video_encoder_convert_d3d12_profile_to_spec_profile_av1(
      associatedMetadata.m_associatedEncodeConfig.m_encoderProfileDesc.m_AV1Profile);
   // seq_header->still_picture; // coded in bitstream by default as 0
   // seq_header->reduced_still_picture_header; // coded in bitstream by default as 0
   // seq_header->timing_info_present_flag; // coded in bitstream by default as 0
   // seq_header->decoder_model_info_present_flag; // coded in bitstream by default as 0
   // seq_header->operating_points_cnt_minus_1; // memset default as 0
   // seq_header->operating_point_idc[32]; // memset default as 0
   d3d12_video_encoder_convert_d3d12_to_spec_level_av1(
      associatedMetadata.m_associatedEncodeConfig.m_encoderLevelDesc.m_AV1LevelSetting.Level,
      seq_header->seq_level_idx[0]);
   d3d12_video_encoder_convert_d3d12_to_spec_tier_av1(
      associatedMetadata.m_associatedEncodeConfig.m_encoderLevelDesc.m_AV1LevelSetting.Tier,
      seq_header->seq_tier[0]);
   // seq_header->decoder_model_present_for_this_op[32]; // memset default as 0
   // seq_header->initial_display_delay_present_flag; // coded in bitstream by default as 0
   // seq_header->initial_display_delay_minus_1[32];
   // seq_header->initial_display_delay_present_for_this_op[32]
   // seq_header->frame_width_bits; // coded in bitstream by default as 16
   // seq_header->frame_height_bits; // coded in bitstream by default as 16

   // frame_size (comes from input texture size)
   seq_header->max_frame_width = associatedMetadata.m_associatedEncodeConfig.m_currentResolution.Width;
   seq_header->max_frame_height = associatedMetadata.m_associatedEncodeConfig.m_currentResolution.Height;

   // seq_header->frame_id_numbers_present_flag; // coded in bitstream by default as 0
   seq_header->use_128x128_superblock =
      ((associatedMetadata.m_associatedEncodeConfig.m_encoderCodecSpecificConfigDesc.m_AV1Config.FeatureFlags &
        D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_128x128_SUPERBLOCK) != 0) ?
         1 :
         0;
   seq_header->enable_filter_intra =
      ((associatedMetadata.m_associatedEncodeConfig.m_encoderCodecSpecificConfigDesc.m_AV1Config.FeatureFlags &
        D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_FILTER_INTRA) != 0) ?
         1 :
         0;
   seq_header->enable_intra_edge_filter =
      ((associatedMetadata.m_associatedEncodeConfig.m_encoderCodecSpecificConfigDesc.m_AV1Config.FeatureFlags &
        D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_INTRA_EDGE_FILTER) != 0) ?
         1 :
         0;
   seq_header->enable_interintra_compound =
      ((associatedMetadata.m_associatedEncodeConfig.m_encoderCodecSpecificConfigDesc.m_AV1Config.FeatureFlags &
        D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_INTERINTRA_COMPOUND) != 0) ?
         1 :
         0;
   seq_header->enable_masked_compound =
      ((associatedMetadata.m_associatedEncodeConfig.m_encoderCodecSpecificConfigDesc.m_AV1Config.FeatureFlags &
        D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_MASKED_COMPOUND) != 0) ?
         1 :
         0;
   seq_header->enable_warped_motion =
      ((associatedMetadata.m_associatedEncodeConfig.m_encoderCodecSpecificConfigDesc.m_AV1Config.FeatureFlags &
        D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_WARPED_MOTION) != 0) ?
         1 :
         0;
   seq_header->enable_dual_filter =
      ((associatedMetadata.m_associatedEncodeConfig.m_encoderCodecSpecificConfigDesc.m_AV1Config.FeatureFlags &
        D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_DUAL_FILTER) != 0) ?
         1 :
         0;
   seq_header->enable_order_hint =
      ((associatedMetadata.m_associatedEncodeConfig.m_encoderCodecSpecificConfigDesc.m_AV1Config.FeatureFlags &
        D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_ORDER_HINT_TOOLS) != 0) ?
         1 :
         0;
   seq_header->enable_jnt_comp =
      ((associatedMetadata.m_associatedEncodeConfig.m_encoderCodecSpecificConfigDesc.m_AV1Config.FeatureFlags &
        D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_JNT_COMP) != 0) ?
         1 :
         0;
   seq_header->enable_ref_frame_mvs =
      ((associatedMetadata.m_associatedEncodeConfig.m_encoderCodecSpecificConfigDesc.m_AV1Config.FeatureFlags &
        D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_FRAME_REFERENCE_MOTION_VECTORS) != 0) ?
         1 :
         0;
   seq_header->seq_choose_screen_content_tools = 0;   // coded in bitstream by default as 0
   seq_header->seq_force_screen_content_tools = 0;    // coded in bitstream by default as 0
   seq_header->seq_choose_integer_mv = 0;             // coded in bitstream by default as 0
   seq_header->seq_force_integer_mv = 0;              // coded in bitstream by default as 0
   seq_header->order_hint_bits_minus1 =
      associatedMetadata.m_associatedEncodeConfig.m_encoderCodecSpecificConfigDesc.m_AV1Config.OrderHintBitsMinus1;
   seq_header->enable_superres =
      ((associatedMetadata.m_associatedEncodeConfig.m_encoderCodecSpecificConfigDesc.m_AV1Config.FeatureFlags &
        D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_SUPER_RESOLUTION) != 0) ?
         1 :
         0;
   seq_header->enable_cdef =
      ((associatedMetadata.m_associatedEncodeConfig.m_encoderCodecSpecificConfigDesc.m_AV1Config.FeatureFlags &
        D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_CDEF_FILTERING) != 0) ?
         1 :
         0;
   seq_header->enable_restoration =
      ((associatedMetadata.m_associatedEncodeConfig.m_encoderCodecSpecificConfigDesc.m_AV1Config.FeatureFlags &
        D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_LOOP_RESTORATION_FILTER) != 0) ?
         1 :
         0;

   seq_header->color_config.bit_depth = associatedMetadata.m_associatedEncodeConfig.m_encodeFormatInfo.Format;
   // seq_header->color_config.mono_chrome; // coded in bitstream by default as 0
   // seq_header->color_config.color_description_present_flag; // memset default as 0
   // seq_header->color_config.color_primaries; // memset default as 0
   // seq_header->color_config.transfer_characteristics; // memset default as 0
   // seq_header->color_config.matrix_coefficients; // memset default as 0
   // seq_header->color_config.color_range; // memset default as 0
   seq_header->color_config.chroma_sample_position = 0;   // CSP_UNKNOWN
   seq_header->color_config.subsampling_x = 1;            // DX12 4:2:0 only
   seq_header->color_config.subsampling_y = 1;            // DX12 4:2:0 only
   seq_header->color_config.separate_uv_delta_q = 1;

   // seq_header->film_grain_params_present; // coded in bitstream by default as 1
}

void
fill_av1_pic_header(EncodedBitstreamResolvedMetadata &associatedMetadata,
                    av1_pic_header_t *pic_header,
                    const av1_seq_header_t *seqHdr,
                    const D3D12_VIDEO_ENCODER_AV1_POST_ENCODE_VALUES *pParsedPostEncodeValues,
                    const D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_SUBREGIONS_LAYOUT_DATA_TILES *pParsedTilePartitions)
{
   // Set all zero by default
   memset(pic_header, 0, sizeof(av1_pic_header_t));

   {
      pic_header->quantization_params = pParsedPostEncodeValues->Quantization;

      debug_printf("[d3d12_video_enc_av1] Parsing driver post encode "
                   "values...D3D12_VIDEO_ENCODER_AV1_POST_ENCODE_VALUES_FLAG_QUANTIZATION\n");

      debug_printf("Post encode quantization_params.BaseQIndex: %" PRIu64 "\n",
                   pic_header->quantization_params.BaseQIndex);
      debug_printf("Post encode quantization_params.YDCDeltaQ: %" PRId64 "\n",
                   pic_header->quantization_params.YDCDeltaQ);
      debug_printf("Post encode quantization_params.UDCDeltaQ: %" PRId64 "\n",
                   pic_header->quantization_params.UDCDeltaQ);
      debug_printf("Post encode quantization_params.UACDeltaQ: %" PRId64 "\n",
                   pic_header->quantization_params.UACDeltaQ);
      debug_printf("Post encode quantization_params.VDCDeltaQ: %" PRId64 "\n",
                   pic_header->quantization_params.VDCDeltaQ);
      debug_printf("Post encode quantization_params.VACDeltaQ: %" PRId64 "\n",
                   pic_header->quantization_params.VACDeltaQ);
      debug_printf("Post encode quantization_params.UsingQMatrix: %" PRIu64 "\n",
                   pic_header->quantization_params.UsingQMatrix);
      debug_printf("Post encode quantization_params.QMY: %" PRIu64 "\n", pic_header->quantization_params.QMY);
      debug_printf("Post encode quantization_params.QMU: %" PRIu64 "\n", pic_header->quantization_params.QMU);
      debug_printf("Post encode quantization_params.QMV: %" PRIu64 "\n", pic_header->quantization_params.QMV);
   }

   pic_header->segmentation_enabled = 0;
   if ((associatedMetadata.m_associatedEncodeConfig.m_encoderPicParamsDesc.m_AV1PicData.Flags &
        D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAG_ENABLE_FRAME_SEGMENTATION_AUTO) != 0) {

      // The segmentation info comes from the driver
      pic_header->segmentation_config = pParsedPostEncodeValues->SegmentationConfig;
      pic_header->segmentation_enabled = (pic_header->segmentation_config.NumSegments != 0);
   }

   {
      debug_printf("[d3d12_video_enc_av1] Parsing driver post encode "
                   "values...D3D12_VIDEO_ENCODER_AV1_POST_ENCODE_VALUES_FLAG_QUANTIZATION_DELTA\n");
      pic_header->delta_q_params = pParsedPostEncodeValues->QuantizationDelta;

      debug_printf("Post encode delta_q_params.DeltaQPresent: %" PRIu64 "\n", pic_header->delta_q_params.DeltaQPresent);
      debug_printf("Post encode delta_q_params.DeltaQRes: %" PRIu64 "\n", pic_header->delta_q_params.DeltaQRes);
   }

   {
      debug_printf("[d3d12_video_enc_av1] Parsing driver post encode "
                   "values...D3D12_VIDEO_ENCODER_AV1_POST_ENCODE_VALUES_FLAG_LOOP_FILTER_DELTA\n");
      pic_header->delta_lf_params = pParsedPostEncodeValues->LoopFilterDelta;

      debug_printf("Post encode delta_lf_params.DeltaLFMulti: %" PRIu64 "\n", pic_header->delta_lf_params.DeltaLFMulti);
      debug_printf("Post encode delta_lf_params.DeltaLFPresent: %" PRIu64 "\n",
                   pic_header->delta_lf_params.DeltaLFPresent);
      debug_printf("Post encode delta_lf_params.DeltaLFRes: %" PRIu64 "\n", pic_header->delta_lf_params.DeltaLFRes);
   }

   {
      debug_printf("[d3d12_video_enc_av1] Parsing driver post encode "
                   "values...D3D12_VIDEO_ENCODER_AV1_POST_ENCODE_VALUES_FLAG_LOOP_FILTER\n");
      pic_header->loop_filter_params = pParsedPostEncodeValues->LoopFilter;

      debug_printf("Post encode loop_filter_params.LoopFilterDeltaEnabled: %" PRIu64 "\n",
                   pic_header->loop_filter_params.LoopFilterDeltaEnabled);
      debug_printf("Post encode loop_filter_params.LoopFilterLevel[0]: %" PRIu64 "\n",
                   pic_header->loop_filter_params.LoopFilterLevel[0]);
      debug_printf("Post encode loop_filter_params.LoopFilterLevel[1]: %" PRIu64 "\n",
                   pic_header->loop_filter_params.LoopFilterLevel[1]);
      debug_printf("Post encode loop_filter_params.LoopFilterLevelU: %" PRIu64 "\n",
                   pic_header->loop_filter_params.LoopFilterLevelU);
      debug_printf("Post encode loop_filter_params.LoopFilterLevelV: %" PRIu64 "\n",
                   pic_header->loop_filter_params.LoopFilterLevelV);
      debug_printf("Post encode loop_filter_params.LoopFilterSharpnessLevel: %" PRIu64 "\n",
                   pic_header->loop_filter_params.LoopFilterSharpnessLevel);
      debug_printf("Post encode loop_filter_params.ModeDeltas[0]: %" PRIu64 "\n",
                   pic_header->loop_filter_params.ModeDeltas[0]);
      debug_printf("Post encode loop_filter_params.ModeDeltas[1]: %" PRIu64 "\n",
                   pic_header->loop_filter_params.ModeDeltas[1]);
      for (uint8_t i = 0; i < 8; i++) {
         debug_printf("Post encode loop_filter_params.RefDeltas[%d]: %" PRIu64 "\n",
                      i,
                      pic_header->loop_filter_params.RefDeltas[i]);
      }
      debug_printf("Post encode loop_filter_params.UpdateModeDelta: %" PRIu64 "\n",
                   pic_header->loop_filter_params.UpdateModeDelta);
      debug_printf("Post encode loop_filter_params.UpdateRefDelta: %" PRIu64 "\n",
                   pic_header->loop_filter_params.UpdateRefDelta);
   }

   {
      debug_printf("[d3d12_video_enc_av1] Parsing driver post encode "
                   "values...D3D12_VIDEO_ENCODER_AV1_POST_ENCODE_VALUES_FLAG_CDEF_DATA\n");
      pic_header->cdef_params = pParsedPostEncodeValues->CDEF;

      debug_printf("Post encode cdef_params.CdefBits: %" PRIu64 "\n", pic_header->cdef_params.CdefBits);

      debug_printf("Post encode cdef_params.CdefDampingMinus3: %" PRIu64 "\n",
                   pic_header->cdef_params.CdefDampingMinus3);

      for (uint8_t i = 0; i < 8; i++) {
         debug_printf("Post encode cdef_params.CdefYPriStrength[%d]: %" PRIu64 "\n",
                      i,
                      pic_header->cdef_params.CdefYPriStrength[i]);

         debug_printf("Post encode cdef_params.CdefUVPriStrength[%d]: %" PRIu64 "\n",
                      i,
                      pic_header->cdef_params.CdefUVPriStrength[i]);

         debug_printf("Post encode cdef_params.CdefYSecStrength[%d]: %" PRIu64 "\n",
                      i,
                      pic_header->cdef_params.CdefYSecStrength[i]);

         debug_printf("Post encode cdef_params.CdefUVSecStrength[%d]: %" PRIu64 "\n",
                      i,
                      pic_header->cdef_params.CdefUVSecStrength[i]);
      }
   }

   {
      debug_printf("[d3d12_video_enc_av1] Parsing driver post encode "
                   "values...D3D12_VIDEO_ENCODER_AV1_POST_ENCODE_VALUES_FLAG_COMPOUND_PREDICTION_MODE\n");
      pic_header->reference_select = (pParsedPostEncodeValues->CompoundPredictionType ==
                                      D3D12_VIDEO_ENCODER_AV1_COMP_PREDICTION_TYPE_SINGLE_REFERENCE) ?
                                        0 :
                                        1;
      debug_printf("Post encode reference_select: %" PRIu32 "\n", pic_header->reference_select);
   }

   // if ((pevFlags & D3D12_VIDEO_ENCODER_AV1_POST_ENCODE_VALUES_FLAG_CONTEXT_UPDATE_TILE_ID) != 0)
   // We just copy the whole structure, the driver must copy it even if the pev flag is not set
   pic_header->tile_info.tile_partition = *pParsedTilePartitions;

   {
      debug_printf("[d3d12_video_enc_av1] Parsing driver post encode "
                   "values...D3D12_VIDEO_ENCODER_AV1_POST_ENCODE_VALUES_FLAG_PRIMARY_REF_FRAME\n");
      pic_header->primary_ref_frame = static_cast<uint32_t>(pParsedPostEncodeValues->PrimaryRefFrame);
      debug_printf("Post encode primary_ref_frame: %" PRIu32 "\n", pic_header->primary_ref_frame);
   }

   debug_printf("Post encode tile_info.tile_partition.ContextUpdateTileId: %" PRIu64 "\n",
                pic_header->tile_info.tile_partition.ContextUpdateTileId);
   debug_printf("Post encode tile_info.tile_partition.ColCount: %" PRIu64 "\n",
                pic_header->tile_info.tile_partition.ColCount);
   debug_printf("Post encode tile_info.tile_partition.RowCount: %" PRIu64 "\n",
                pic_header->tile_info.tile_partition.RowCount);

   assert(pic_header->tile_info.tile_partition.ColCount < 64);
   for (uint8_t i = 0; i < pic_header->tile_info.tile_partition.ColCount; i++) {
      debug_printf("Post encode tile_info.tile_partition.ColWidths[%d]: %" PRIu64 "\n",
                   i,
                   pic_header->tile_info.tile_partition.ColWidths[i]);
   }

   assert(pic_header->tile_info.tile_partition.RowCount < 64);
   for (uint8_t i = 0; i < pic_header->tile_info.tile_partition.RowCount; i++) {
      debug_printf("Post encode tile_info.tile_partition.RowHeights[%d]: %" PRIu64 "\n",
                   i,
                   pic_header->tile_info.tile_partition.RowHeights[i]);
   }

   pic_header->show_existing_frame = 0;
   pic_header->frame_to_show_map_idx = 0;

   // pic_header->display_frame_id; // frame_id_numbers_present_flag coded in bitstream by default as 0

   pic_header->frame_type = associatedMetadata.m_associatedEncodeConfig.m_encoderPicParamsDesc.m_AV1PicData.FrameType;
   {
      UINT EncodeOrderInGop =
         (associatedMetadata.m_associatedEncodeConfig.m_encoderPicParamsDesc.m_AV1PicData.PictureIndex %
          associatedMetadata.m_associatedEncodeConfig.m_encoderGOPConfigDesc.m_AV1SequenceStructure.IntraDistance);

      UINT ShowOrderInGop =
         (associatedMetadata.m_associatedEncodeConfig.m_encoderPicParamsDesc.m_AV1PicData.OrderHint %
          associatedMetadata.m_associatedEncodeConfig.m_encoderGOPConfigDesc.m_AV1SequenceStructure.IntraDistance);

      pic_header->show_frame = (ShowOrderInGop <= EncodeOrderInGop);
   }

   pic_header->showable_frame =   // this will always be showable for P/B frames, even when using show_frame for B
                                  // frame playback reordering
      associatedMetadata.m_associatedEncodeConfig.m_encoderPicParamsDesc.m_AV1PicData.FrameType !=
      D3D12_VIDEO_ENCODER_AV1_FRAME_TYPE_KEY_FRAME;
   pic_header->error_resilient_mode =
      ((associatedMetadata.m_associatedEncodeConfig.m_encoderPicParamsDesc.m_AV1PicData.Flags &
        D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAG_ENABLE_ERROR_RESILIENT_MODE) != 0);
   pic_header->disable_cdf_update =
      ((associatedMetadata.m_associatedEncodeConfig.m_encoderPicParamsDesc.m_AV1PicData.Flags &
        D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAG_DISABLE_CDF_UPDATE) != 0);

   pic_header->allow_screen_content_tools =
      ((associatedMetadata.m_associatedEncodeConfig.m_encoderPicParamsDesc.m_AV1PicData.Flags &
        D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAG_ENABLE_PALETTE_ENCODING) != 0);
   pic_header->force_integer_mv =
      ((associatedMetadata.m_associatedEncodeConfig.m_encoderPicParamsDesc.m_AV1PicData.Flags &
        D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAG_FORCE_INTEGER_MOTION_VECTORS) != 0);

   // frame_size_override_flag is not coded + default as 1 for SWITCH_FRAME and explicitly coded otherwise
   if (pic_header->frame_type == D3D12_VIDEO_ENCODER_AV1_FRAME_TYPE_SWITCH_FRAME)
      pic_header->frame_size_override_flag = 1;   // As per AV1 codec spec
   else
      pic_header->frame_size_override_flag = 0;   // Set as default as 0 when explicitly coded

   pic_header->order_hint = associatedMetadata.m_associatedEncodeConfig.m_encoderPicParamsDesc.m_AV1PicData.OrderHint;

   pic_header->refresh_frame_flags =
      associatedMetadata.m_associatedEncodeConfig.m_encoderPicParamsDesc.m_AV1PicData.RefreshFrameFlags;

   // frame_size (comes from input texture size)
   pic_header->FrameWidth = associatedMetadata.m_associatedEncodeConfig.m_currentResolution.Width;
   pic_header->UpscaledWidth = associatedMetadata.m_associatedEncodeConfig.m_currentResolution.Width;
   pic_header->FrameHeight = associatedMetadata.m_associatedEncodeConfig.m_currentResolution.Height;

   // render_size (comes from AV1 pipe picparams pic)
   pic_header->RenderWidth = associatedMetadata.m_associatedEncodeConfig.m_FrameCroppingCodecConfig.right;
   pic_header->RenderHeight = associatedMetadata.m_associatedEncodeConfig.m_FrameCroppingCodecConfig.bottom;

   bool use_128x128_superblock =
      ((associatedMetadata.m_associatedEncodeConfig.m_encoderCodecSpecificConfigDesc.m_AV1Config.FeatureFlags &
        D3D12_VIDEO_ENCODER_AV1_FEATURE_FLAG_128x128_SUPERBLOCK) != 0) ?
         1 :
         0;
   unsigned MiCols = 2 * ((pic_header->FrameWidth + 7) >> 3);
   unsigned MiRows = 2 * ((pic_header->FrameHeight + 7) >> 3);
   pic_header->frame_width_sb = use_128x128_superblock ? ((MiCols + 31) >> 5) : ((MiCols + 15) >> 4);
   pic_header->frame_height_sb = use_128x128_superblock ? ((MiRows + 31) >> 5) : ((MiRows + 15) >> 4);

   pic_header->use_superres = ((associatedMetadata.m_associatedEncodeConfig.m_encoderPicParamsDesc.m_AV1PicData.Flags &
                                D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAG_USE_SUPER_RESOLUTION) != 0);
   pic_header->SuperresDenom =
      associatedMetadata.m_associatedEncodeConfig.m_encoderPicParamsDesc.m_AV1PicData.SuperResDenominator;

   pic_header->allow_intrabc = ((associatedMetadata.m_associatedEncodeConfig.m_encoderPicParamsDesc.m_AV1PicData.Flags &
                                 D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAG_ALLOW_INTRA_BLOCK_COPY) != 0);

   for (unsigned i = 0; i < ARRAY_SIZE(associatedMetadata.m_associatedEncodeConfig.m_encoderPicParamsDesc.m_AV1PicData
                                          .ReferenceFramesReconPictureDescriptors);
        i++) {
      pic_header->ref_order_hint[i] = associatedMetadata.m_associatedEncodeConfig.m_encoderPicParamsDesc.m_AV1PicData
                                         .ReferenceFramesReconPictureDescriptors[i]
                                         .OrderHint;
   }

   for (uint8_t i = 0; i < ARRAY_SIZE(pParsedPostEncodeValues->ReferenceIndices); i++)
      pic_header->ref_frame_idx[i] = pParsedPostEncodeValues->ReferenceIndices[i];

   pic_header->allow_high_precision_mv =
      ((associatedMetadata.m_associatedEncodeConfig.m_encoderPicParamsDesc.m_AV1PicData.Flags &
        D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAG_ALLOW_HIGH_PRECISION_MV) != 0);
   pic_header->interpolation_filter =
      associatedMetadata.m_associatedEncodeConfig.m_encoderPicParamsDesc.m_AV1PicData.InterpolationFilter;
   pic_header->is_motion_mode_switchable =
      ((associatedMetadata.m_associatedEncodeConfig.m_encoderPicParamsDesc.m_AV1PicData.Flags &
        D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAG_MOTION_MODE_SWITCHABLE) != 0);
   pic_header->use_ref_frame_mvs =
      ((associatedMetadata.m_associatedEncodeConfig.m_encoderPicParamsDesc.m_AV1PicData.Flags &
        D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAG_FRAME_REFERENCE_MOTION_VECTORS) != 0);
   pic_header->disable_frame_end_update_cdf =
      ((associatedMetadata.m_associatedEncodeConfig.m_encoderPicParamsDesc.m_AV1PicData.Flags &
        D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAG_DISABLE_FRAME_END_UPDATE_CDF) != 0);

   pic_header->tile_info.tile_mode = associatedMetadata.m_associatedEncodeConfig.m_encoderSliceConfigMode;
   pic_header->tile_info.tile_support_caps =
      associatedMetadata.m_associatedEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1TileCaps;

   pic_header->tile_info.uniform_tile_spacing_flag = 0;
   if ((pic_header->tile_info.tile_mode == D3D12_VIDEO_ENCODER_FRAME_SUBREGION_LAYOUT_MODE_UNIFORM_GRID_PARTITION) ||
       (pic_header->tile_info.tile_mode == D3D12_VIDEO_ENCODER_FRAME_SUBREGION_LAYOUT_MODE_FULL_FRAME))
      pic_header->tile_info.uniform_tile_spacing_flag = 1;
   else if (pic_header->tile_info.tile_mode ==
            D3D12_VIDEO_ENCODER_FRAME_SUBREGION_LAYOUT_MODE_CONFIGURABLE_GRID_PARTITION)
      pic_header->tile_info.uniform_tile_spacing_flag = 0;
   else
      assert(false);   // Unknown pic_header->tile_info.tile_mode for AV1

   // lr_params
   for (uint32_t i = 0; i < 3; i++)
      pic_header->lr_params.lr_type[i] = associatedMetadata.m_associatedEncodeConfig.m_encoderPicParamsDesc.m_AV1PicData
                                            .FrameRestorationConfig.FrameRestorationType[i];
   bool lr_enabled =
      pic_header->lr_params.lr_type[0] || pic_header->lr_params.lr_type[1] || pic_header->lr_params.lr_type[2];
   if (lr_enabled) {
      uint8_t luma_shift_total = log2(d3d12_video_encoder_looprestorationsize_d3d12_to_uint_av1(
                                    associatedMetadata.m_associatedEncodeConfig.m_encoderPicParamsDesc.m_AV1PicData
                                       .FrameRestorationConfig.LoopRestorationPixelSize[0])) -
                                 6;
      pic_header->lr_params.lr_unit_shift = (luma_shift_total > 0) ? 1 : 0;
      pic_header->lr_params.lr_unit_extra_shift = (luma_shift_total > 1) ? 1 : 0;
      assert(associatedMetadata.m_associatedEncodeConfig.m_encoderPicParamsDesc.m_AV1PicData.FrameRestorationConfig
                .LoopRestorationPixelSize[1] == associatedMetadata.m_associatedEncodeConfig.m_encoderPicParamsDesc
                                                   .m_AV1PicData.FrameRestorationConfig.LoopRestorationPixelSize[2]);

      if (associatedMetadata.m_associatedEncodeConfig.m_encoderPicParamsDesc.m_AV1PicData.FrameRestorationConfig
             .LoopRestorationPixelSize[1]) {
         pic_header->lr_params.lr_uv_shift = log2(d3d12_video_encoder_looprestorationsize_d3d12_to_uint_av1(
                                                associatedMetadata.m_associatedEncodeConfig.m_encoderPicParamsDesc
                                                   .m_AV1PicData.FrameRestorationConfig.LoopRestorationPixelSize[1])) +
                                             6 + luma_shift_total;
      } else {
         pic_header->lr_params.lr_uv_shift = 0;
      }
   }

   pic_header->TxMode = associatedMetadata.m_associatedEncodeConfig.m_encoderPicParamsDesc.m_AV1PicData.TxMode;
   pic_header->skip_mode_present =
      ((associatedMetadata.m_associatedEncodeConfig.m_encoderPicParamsDesc.m_AV1PicData.Flags &
        D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAG_ENABLE_SKIP_MODE) != 0);
   pic_header->allow_warped_motion =
      ((associatedMetadata.m_associatedEncodeConfig.m_encoderPicParamsDesc.m_AV1PicData.Flags &
        D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAG_ENABLE_WARPED_MOTION) != 0);
   pic_header->reduced_tx_set =
      ((associatedMetadata.m_associatedEncodeConfig.m_encoderPicParamsDesc.m_AV1PicData.Flags &
        D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_FLAG_REDUCED_TX_SET) != 0);
   // pic_header->frame_refs_short_signaling; // coded in bitstream by default as 0
}

D3D12_VIDEO_ENCODER_AV1_RESTORATION_TILESIZE
d3d12_video_encoder_looprestorationsize_uint_to_d3d12_av1(uint32_t pixel_size)
{
   switch (pixel_size) {
      case 32:
      {
         return D3D12_VIDEO_ENCODER_AV1_RESTORATION_TILESIZE_32x32;
      } break;
      case 64:
      {
         return D3D12_VIDEO_ENCODER_AV1_RESTORATION_TILESIZE_64x64;
      } break;
      case 128:
      {
         return D3D12_VIDEO_ENCODER_AV1_RESTORATION_TILESIZE_128x128;
      } break;
      case 256:
      {
         return D3D12_VIDEO_ENCODER_AV1_RESTORATION_TILESIZE_256x256;
      } break;
      default:
      {
         unreachable("Unsupported D3D12_VIDEO_ENCODER_AV1_PROFILE");
      } break;
   }
}

unsigned
d3d12_video_encoder_looprestorationsize_d3d12_to_uint_av1(D3D12_VIDEO_ENCODER_AV1_RESTORATION_TILESIZE d3d12_type)
{
   switch (d3d12_type) {
      case D3D12_VIDEO_ENCODER_AV1_RESTORATION_TILESIZE_32x32:
      {
         return 32;
      } break;
      case D3D12_VIDEO_ENCODER_AV1_RESTORATION_TILESIZE_64x64:
      {
         return 64;
      } break;
      case D3D12_VIDEO_ENCODER_AV1_RESTORATION_TILESIZE_128x128:
      {
         return 128;
      } break;
      case D3D12_VIDEO_ENCODER_AV1_RESTORATION_TILESIZE_256x256:
      {
         return 256;
      } break;
      default:
      {
         unreachable("Unsupported D3D12_VIDEO_ENCODER_AV1_PROFILE");
      } break;
   }
}

/*
 * Called at get_feedback time FOR A PREVIOUSLY RECORDED AND EXECUTED FRAME
 */
unsigned
d3d12_video_encoder_build_post_encode_codec_bitstream_av1(struct d3d12_video_encoder *pD3D12Enc,
                                                          uint64_t associated_fence_value,
                                                          EncodedBitstreamResolvedMetadata &associatedMetadata)
{
   // Parse the AV1 resolved metadata

   ID3D12Resource *pResolvedMetadataBuffer = associatedMetadata.spBuffer.Get();
   uint64_t resourceMetadataSize = associatedMetadata.bufferSize;

   struct d3d12_screen *pD3D12Screen = (struct d3d12_screen *) pD3D12Enc->m_pD3D12Screen;
   pipe_resource *pPipeResolvedMetadataBuffer =
      d3d12_resource_from_resource(&pD3D12Screen->base, pResolvedMetadataBuffer);
   assert(resourceMetadataSize < INT_MAX);
   struct pipe_box box = {
      0,                                        // x
      0,                                        // y
      0,                                        // z
      static_cast<int>(resourceMetadataSize),   // width
      1,                                        // height
      1                                         // depth
   };
   struct pipe_transfer *mapTransferMetadata;
   uint8_t *pMetadataBufferSrc =
      reinterpret_cast<uint8_t *>(pD3D12Enc->base.context->buffer_map(pD3D12Enc->base.context,
                                                                      pPipeResolvedMetadataBuffer,
                                                                      0,
                                                                      PIPE_MAP_READ,
                                                                      &box,
                                                                      &mapTransferMetadata));

   debug_printf("[d3d12_video_enc_av1] Parsing driver resolved encode metadata...\n");

   D3D12_VIDEO_ENCODER_OUTPUT_METADATA *pParsedMetadata =
      reinterpret_cast<D3D12_VIDEO_ENCODER_OUTPUT_METADATA *>(pMetadataBufferSrc);
   pMetadataBufferSrc += sizeof(D3D12_VIDEO_ENCODER_OUTPUT_METADATA);

   D3D12_VIDEO_ENCODER_FRAME_SUBREGION_METADATA *pFrameSubregionMetadata =
      reinterpret_cast<D3D12_VIDEO_ENCODER_FRAME_SUBREGION_METADATA *>(pMetadataBufferSrc);
   pMetadataBufferSrc +=
      (sizeof(D3D12_VIDEO_ENCODER_FRAME_SUBREGION_METADATA) * pParsedMetadata->WrittenSubregionsCount);

   D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_SUBREGIONS_LAYOUT_DATA_TILES *pParsedTilePartitions =
      reinterpret_cast<D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_SUBREGIONS_LAYOUT_DATA_TILES *>(pMetadataBufferSrc);
   pMetadataBufferSrc += sizeof(D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_SUBREGIONS_LAYOUT_DATA_TILES);

   D3D12_VIDEO_ENCODER_AV1_POST_ENCODE_VALUES *pParsedPostEncodeValues =
      reinterpret_cast<D3D12_VIDEO_ENCODER_AV1_POST_ENCODE_VALUES *>(pMetadataBufferSrc);
   pMetadataBufferSrc += sizeof(D3D12_VIDEO_ENCODER_AV1_POST_ENCODE_VALUES);

   debug_printf("[d3d12_video_enc_av1] D3D12_VIDEO_ENCODER_OUTPUT_METADATA.EncodeErrorFlags: %" PRIx64 " \n",
                pParsedMetadata->EncodeErrorFlags);
   debug_printf("[d3d12_video_enc_av1] D3D12_VIDEO_ENCODER_OUTPUT_METADATA.EncodeStats.AverageQP: %" PRIu64 " \n",
                pParsedMetadata->EncodeStats.AverageQP);
   debug_printf("[d3d12_video_enc_av1] D3D12_VIDEO_ENCODER_OUTPUT_METADATA.EncodeStats.IntraCodingUnitsCount: %" PRIu64
                " \n",
                pParsedMetadata->EncodeStats.IntraCodingUnitsCount);
   debug_printf("[d3d12_video_enc_av1] D3D12_VIDEO_ENCODER_OUTPUT_METADATA.EncodeStats.InterCodingUnitsCount: %" PRIu64
                " \n",
                pParsedMetadata->EncodeStats.InterCodingUnitsCount);
   debug_printf("[d3d12_video_enc_av1] D3D12_VIDEO_ENCODER_OUTPUT_METADATA.EncodeStats.SkipCodingUnitsCount: %" PRIu64
                " \n",
                pParsedMetadata->EncodeStats.SkipCodingUnitsCount);
   debug_printf("[d3d12_video_enc_av1] "
                "D3D12_VIDEO_ENCODER_OUTPUT_METADATA.EncodeStats.AverageMotionEstimationXDirection: %" PRIu64 " \n",
                pParsedMetadata->EncodeStats.AverageMotionEstimationXDirection);
   debug_printf("[d3d12_video_enc_av1] "
                "D3D12_VIDEO_ENCODER_OUTPUT_METADATA.EncodeStats.AverageMotionEstimationYDirection: %" PRIu64 " \n",
                pParsedMetadata->EncodeStats.AverageMotionEstimationYDirection);
   debug_printf("[d3d12_video_enc_av1] D3D12_VIDEO_ENCODER_OUTPUT_METADATA.EncodedBitstreamWrittenBytesCount: %" PRIu64
                " \n",
                pParsedMetadata->EncodedBitstreamWrittenBytesCount);
   debug_printf("[d3d12_video_enc_av1] D3D12_VIDEO_ENCODER_OUTPUT_METADATA.WrittenSubregionsCount: %" PRIu64 " \n",
                pParsedMetadata->WrittenSubregionsCount);

   for (uint8_t i = 0; i < pParsedMetadata->WrittenSubregionsCount; i++) {
      debug_printf("[d3d12_video_enc_av1] D3D12_VIDEO_ENCODER_FRAME_SUBREGION_METADATA[%d].bHeaderSize: %" PRIu64 " \n",
                   i,
                   pFrameSubregionMetadata[i].bHeaderSize);
      debug_printf("[d3d12_video_enc_av1] D3D12_VIDEO_ENCODER_FRAME_SUBREGION_METADATA[%d].bStartOffset: %" PRIu64
                   " \n",
                   i,
                   pFrameSubregionMetadata[i].bStartOffset);
      debug_printf("[d3d12_video_enc_av1] D3D12_VIDEO_ENCODER_FRAME_SUBREGION_METADATA[%d].bSize: %" PRIu64 " \n",
                   i,
                   pFrameSubregionMetadata[i].bSize);
   }

   if (pParsedMetadata->EncodeErrorFlags != D3D12_VIDEO_ENCODER_ENCODE_ERROR_FLAG_NO_ERROR) {
      debug_printf("[d3d12_video_enc_av1] Encode GPU command for fence %" PRIu64 " failed - EncodeErrorFlags: %" PRIu64
                   "\n",
                   associated_fence_value,
                   pParsedMetadata->EncodeErrorFlags);
      assert(false);
      return 0;
   }

   if (pParsedMetadata->EncodedBitstreamWrittenBytesCount == 0u) {
      debug_printf("[d3d12_video_enc_av1] Encode GPU command for fence %" PRIu64
                   " failed - EncodedBitstreamWrittenBytesCount: 0\n",
                   associated_fence_value);
      assert(false);
      return 0;
   }

   // Create headers

   av1_seq_header_t seqHdr = {};
   fill_av1_seq_header(associatedMetadata, &seqHdr);
   av1_pic_header_t picHdr = {};
   fill_av1_pic_header(associatedMetadata, &picHdr, &seqHdr, pParsedPostEncodeValues, pParsedTilePartitions);

   bool bNeedSeqUpdate = false;
   bool diff_uv_delta_from_pev = (picHdr.quantization_params.VDCDeltaQ != picHdr.quantization_params.UDCDeltaQ) ||
                                 (picHdr.quantization_params.VACDeltaQ != picHdr.quantization_params.UACDeltaQ);
   debug_printf("[Calculated] Post encode diff_uv_delta_from_pev: %d\n", diff_uv_delta_from_pev);

   // Make sure the Seq header allows diff_uv_delta from pev
   if (diff_uv_delta_from_pev && !seqHdr.color_config.separate_uv_delta_q) {
      seqHdr.color_config.separate_uv_delta_q = 1;
      bNeedSeqUpdate = true;
   }

   d3d12_video_bitstream_builder_av1 *pAV1BitstreamBuilder =
      static_cast<d3d12_video_bitstream_builder_av1 *>(pD3D12Enc->m_upBitstreamBuilder.get());
   assert(pAV1BitstreamBuilder);

   associatedMetadata.pWrittenCodecUnitsSizes.clear();

   size_t writtenTemporalDelimBytes = 0;
   if (picHdr.show_frame && associatedMetadata.m_CodecSpecificData.AV1HeadersInfo.temporal_delim_rendered) {
      pAV1BitstreamBuilder->write_temporal_delimiter_obu(
         pD3D12Enc->m_BitstreamHeadersBuffer,
         pD3D12Enc->m_BitstreamHeadersBuffer.begin(),   // placingPositionStart
         writtenTemporalDelimBytes                      // Bytes Written AFTER placingPositionStart arg above
      );
      assert(pD3D12Enc->m_BitstreamHeadersBuffer.size() == writtenTemporalDelimBytes);
      debug_printf("Written OBU_TEMPORAL_DELIMITER bytes: %" PRIu64 "\n", static_cast<uint64_t>(writtenTemporalDelimBytes));
      associatedMetadata.pWrittenCodecUnitsSizes.push_back(writtenTemporalDelimBytes);
   }

   size_t writtenSequenceBytes = 0;
   bool isFirstFrame = (associated_fence_value == 1);

   // on first frame or on resolution change or if we changed seq hdr values with post encode values
   bool writeNewSeqHeader = isFirstFrame || bNeedSeqUpdate ||
                            ((associatedMetadata.m_associatedEncodeConfig.m_seqFlags &
                              D3D12_VIDEO_ENCODER_SEQUENCE_CONTROL_FLAG_RESOLUTION_CHANGE) != 0);

   if (writeNewSeqHeader) {
      pAV1BitstreamBuilder->write_sequence_header(
         &seqHdr,
         pD3D12Enc->m_BitstreamHeadersBuffer,
         pD3D12Enc->m_BitstreamHeadersBuffer.begin() + writtenTemporalDelimBytes,   // placingPositionStart
         writtenSequenceBytes   // Bytes Written AFTER placingPositionStart arg above
      );
      associatedMetadata.pWrittenCodecUnitsSizes.push_back(writtenSequenceBytes);
      assert(pD3D12Enc->m_BitstreamHeadersBuffer.size() == (writtenSequenceBytes + writtenTemporalDelimBytes));
      debug_printf("Written OBU_SEQUENCE_HEADER bytes: %" PRIu64 "\n", static_cast<uint64_t>(writtenSequenceBytes));
   }

   // Only supported bitstream format is with obu_size for now.
   assert(associatedMetadata.m_CodecSpecificData.AV1HeadersInfo.obu_has_size_field);

   size_t writtenFrameBytes = 0;
   size_t writtenTileBytes = 0;

   pipe_resource *src_driver_bitstream =
      d3d12_resource_from_resource(&pD3D12Enc->m_pD3D12Screen->base, associatedMetadata.spStagingBitstream.Get());
   assert(src_driver_bitstream);

   size_t comp_bitstream_offset = 0;
   if (associatedMetadata.m_CodecSpecificData.AV1HeadersInfo.enable_frame_obu) {
      // FRAME_OBU combined frame and tile data case

      assert(associatedMetadata.m_associatedEncodeConfig.m_encoderSliceConfigDesc.m_TilesConfig_AV1.TilesGroupsCount ==
             1);

      // write_frame_header writes OBU_FRAME except tile_obu_group, but included obu_size for tile_group_obu as
      // calculated below
      size_t tile_group_obu_size = 0;
      size_t decode_tile_elements_size = 0;
      pAV1BitstreamBuilder->calculate_tile_group_obu_size(
         pParsedMetadata,
         pFrameSubregionMetadata,
         associatedMetadata.m_associatedEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1TileCaps
               .TileSizeBytesMinus1 +
            1,
         associatedMetadata.m_associatedEncodeConfig.m_encoderSliceConfigDesc.m_TilesConfig_AV1.TilesPartition,
         associatedMetadata.m_associatedEncodeConfig.m_encoderSliceConfigDesc.m_TilesConfig_AV1.TilesGroups[0],
         tile_group_obu_size,
         decode_tile_elements_size);

      pAV1BitstreamBuilder->write_frame_header(
         &seqHdr,
         &picHdr,
         OBU_FRAME,
         tile_group_obu_size,   // We need it to code obu_size of OBU_FRAME open_bitstream_unit()
         pD3D12Enc->m_BitstreamHeadersBuffer,
         pD3D12Enc->m_BitstreamHeadersBuffer.begin() + writtenSequenceBytes +
            writtenTemporalDelimBytes,   // placingPositionStart
         writtenFrameBytes               // Bytes Written AFTER placingPositionStart arg above
      );

      debug_printf("Written OBU_FRAME bytes: %" PRIu64 "\n", static_cast<uint64_t>(writtenFrameBytes));
      associatedMetadata.pWrittenCodecUnitsSizes.push_back(writtenFrameBytes);

      assert(pD3D12Enc->m_BitstreamHeadersBuffer.size() ==
             (writtenSequenceBytes + writtenTemporalDelimBytes + writtenFrameBytes));

      debug_printf("Uploading %" PRIu64
                   " bytes from OBU sequence and/or picture headers to comp_bit_destination %p at offset 0\n",
                   static_cast<uint64_t>(pD3D12Enc->m_BitstreamHeadersBuffer.size()),
                   associatedMetadata.comp_bit_destination);

      // Upload headers to the finalized compressed bitstream buffer
      // Note: The buffer_subdata is queued in pD3D12Enc->base.context but doesn't execute immediately
      pD3D12Enc->base.context->buffer_subdata(pD3D12Enc->base.context,                   // context
                                              associatedMetadata.comp_bit_destination,   // comp. bitstream
                                              PIPE_MAP_WRITE,                            // usage PIPE_MAP_x
                                              0,                                         // offset
                                              pD3D12Enc->m_BitstreamHeadersBuffer.size(),
                                              pD3D12Enc->m_BitstreamHeadersBuffer.data());

      comp_bitstream_offset = pD3D12Enc->m_BitstreamHeadersBuffer.size();
      size_t written_bytes_to_staging_bitstream_buffer = 0;

      upload_tile_group_obu(
         pD3D12Enc,
         tile_group_obu_size,
         decode_tile_elements_size,
         associatedMetadata.m_StagingBitstreamConstruction,
         0,   // staging_bitstream_buffer_offset,
         src_driver_bitstream,
         associatedMetadata.comp_bit_destination,
         comp_bitstream_offset,
         pFrameSubregionMetadata,
         associatedMetadata.m_associatedEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1TileCaps
               .TileSizeBytesMinus1 +
            1,
         associatedMetadata.m_associatedEncodeConfig.m_encoderSliceConfigDesc.m_TilesConfig_AV1.TilesPartition,
         associatedMetadata.m_associatedEncodeConfig.m_encoderSliceConfigDesc.m_TilesConfig_AV1.TilesGroups[0],
         written_bytes_to_staging_bitstream_buffer,
         associatedMetadata.pWrittenCodecUnitsSizes);

      writtenTileBytes += tile_group_obu_size;
      comp_bitstream_offset += writtenTileBytes;

      // Flush batch with upload work and wait on this CPU thread for GPU work completion
      {
         struct pipe_fence_handle *pUploadGPUCompletionFence = NULL;
         pD3D12Enc->base.context->flush(pD3D12Enc->base.context,
                                        &pUploadGPUCompletionFence,
                                        PIPE_FLUSH_ASYNC | PIPE_FLUSH_HINT_FINISH);
         assert(pUploadGPUCompletionFence);
         debug_printf(
            "[d3d12_video_encoder] d3d12_video_encoder_build_post_encode_codec_bitstream_av1 - Waiting on GPU "
            "completion fence for "
            " uploading finalized codec headers to compressed bitstream.\n");

         pD3D12Enc->m_pD3D12Screen->base.fence_finish(&pD3D12Enc->m_pD3D12Screen->base,
                                                      NULL,
                                                      pUploadGPUCompletionFence,
                                                      OS_TIMEOUT_INFINITE);
         pD3D12Enc->m_pD3D12Screen->base.fence_reference(&pD3D12Enc->m_pD3D12Screen->base,
                                                         &pUploadGPUCompletionFence,
                                                         NULL);
      }
   } else {
      // FRAME_HEADER_OBU + OBU_TILE_GROUP concatenated case

      // Build open_bitstream_unit for OBU_FRAME_HEADER
      pAV1BitstreamBuilder->write_frame_header(&seqHdr,
                                               &picHdr,
                                               OBU_FRAME_HEADER,
                                               0,   // no tile_obu_group in OBU_FRAME_HEADER
                                               pD3D12Enc->m_BitstreamHeadersBuffer,
                                               pD3D12Enc->m_BitstreamHeadersBuffer.begin() + writtenSequenceBytes +
                                                  writtenTemporalDelimBytes,   // placingPositionStart
                                               writtenFrameBytes   // Bytes Written AFTER placingPositionStart arg above
      );
      associatedMetadata.pWrittenCodecUnitsSizes.push_back(writtenFrameBytes);

      debug_printf("Written OBU_FRAME_HEADER bytes: %" PRIu64 "\n", static_cast<uint64_t>(writtenFrameBytes));

      assert(pD3D12Enc->m_BitstreamHeadersBuffer.size() ==
             (writtenSequenceBytes + writtenTemporalDelimBytes + writtenFrameBytes));

      debug_printf("Uploading %" PRIu64 " bytes from OBU headers to comp_bit_destination %p at offset 0\n",
                   static_cast<uint64_t>(pD3D12Enc->m_BitstreamHeadersBuffer.size()),
                   associatedMetadata.comp_bit_destination);

      // Upload headers to the finalized compressed bitstream buffer
      // Note: The buffer_subdata is queued in pD3D12Enc->base.context but doesn't execute immediately
      pD3D12Enc->base.context->buffer_subdata(pD3D12Enc->base.context,                   // context
                                              associatedMetadata.comp_bit_destination,   // comp. bitstream
                                              PIPE_MAP_WRITE,                            // usage PIPE_MAP_x
                                              0,                                         // offset
                                              pD3D12Enc->m_BitstreamHeadersBuffer.size(),
                                              pD3D12Enc->m_BitstreamHeadersBuffer.data());

      comp_bitstream_offset = pD3D12Enc->m_BitstreamHeadersBuffer.size();
      size_t staging_bitstream_buffer_offset = 0;

      for (int tg_idx = 0;
           tg_idx <
           associatedMetadata.m_associatedEncodeConfig.m_encoderSliceConfigDesc.m_TilesConfig_AV1.TilesGroupsCount;
           tg_idx++) {
         auto &currentTg =
            associatedMetadata.m_associatedEncodeConfig.m_encoderSliceConfigDesc.m_TilesConfig_AV1.TilesGroups[tg_idx];

         debug_printf("Uploading tile group %d to comp_bit_destination %p at offset %" PRIu64 "\n",
                      tg_idx,
                      associatedMetadata.comp_bit_destination,
                      static_cast<uint64_t>(comp_bitstream_offset));

         size_t tile_group_obu_size = 0;
         size_t decode_tile_elements_size = 0;
         pAV1BitstreamBuilder->calculate_tile_group_obu_size(
            pParsedMetadata,
            pFrameSubregionMetadata,
            associatedMetadata.m_associatedEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1TileCaps
                  .TileSizeBytesMinus1 +
               1,
            associatedMetadata.m_associatedEncodeConfig.m_encoderSliceConfigDesc.m_TilesConfig_AV1.TilesPartition,
            currentTg,
            tile_group_obu_size,
            decode_tile_elements_size);

         size_t writtenTileObuPrefixBytes = 0;
         pAV1BitstreamBuilder->write_obu_tile_group_header(
            tile_group_obu_size,   // tile_group_obu() size to pack OBU_TILE_GROUP obu_size element
            associatedMetadata.m_StagingBitstreamConstruction,   // Source CPU buffer cannot be overwritten until
                                                                 // GPU upload flush finishes.
            associatedMetadata.m_StagingBitstreamConstruction.begin() +
               staging_bitstream_buffer_offset,                  // placingPositionStart
            writtenTileObuPrefixBytes);                          // Bytes Written AFTER placingPositionStart arg above

         debug_printf("Written %" PRIu64 " bytes for OBU_TILE_GROUP open_bitstream_unit() prefix with obu_header() and "
                      "obu_size to staging_bitstream_buffer %p at offset %" PRIu64 "\n",
                      static_cast<uint64_t>(writtenTileObuPrefixBytes),
                      associatedMetadata.m_StagingBitstreamConstruction.data(),
                      static_cast<uint64_t>(staging_bitstream_buffer_offset));

         writtenTileBytes += writtenTileObuPrefixBytes;
         associatedMetadata.pWrittenCodecUnitsSizes.push_back(writtenTileObuPrefixBytes);

         // Note: The buffer_subdata is queued in pD3D12Enc->base.context but doesn't execute immediately
         pD3D12Enc->base.context->buffer_subdata(
            pD3D12Enc->base.context,                   // context
            associatedMetadata.comp_bit_destination,   // comp. bitstream
            PIPE_MAP_WRITE,                            // usage PIPE_MAP_x
            comp_bitstream_offset,                     // offset
            writtenTileObuPrefixBytes,
            associatedMetadata.m_StagingBitstreamConstruction.data() + staging_bitstream_buffer_offset);

         debug_printf("Uploading %" PRIu64 " bytes for OBU_TILE_GROUP open_bitstream_unit() prefix with obu_header() "
                      "and obu_size: %" PRIu64 " to comp_bit_destination %p at offset %" PRIu64 "\n",
                      static_cast<uint64_t>(writtenTileObuPrefixBytes),
                      static_cast<uint64_t>(tile_group_obu_size),
                      associatedMetadata.comp_bit_destination,
                      static_cast<uint64_t>(comp_bitstream_offset));

         staging_bitstream_buffer_offset += writtenTileObuPrefixBytes;

         comp_bitstream_offset += writtenTileObuPrefixBytes;

         size_t written_bytes_to_staging_bitstream_buffer = 0;
         upload_tile_group_obu(
            pD3D12Enc,
            tile_group_obu_size,
            decode_tile_elements_size,
            associatedMetadata.m_StagingBitstreamConstruction,
            staging_bitstream_buffer_offset,
            src_driver_bitstream,
            associatedMetadata.comp_bit_destination,
            comp_bitstream_offset,
            pFrameSubregionMetadata,
            associatedMetadata.m_associatedEncodeCapabilities.m_encoderCodecSpecificConfigCaps.m_AV1TileCaps
                  .TileSizeBytesMinus1 +
               1,
            associatedMetadata.m_associatedEncodeConfig.m_encoderSliceConfigDesc.m_TilesConfig_AV1.TilesPartition,
            currentTg,
            written_bytes_to_staging_bitstream_buffer,
            associatedMetadata.pWrittenCodecUnitsSizes);

         staging_bitstream_buffer_offset += written_bytes_to_staging_bitstream_buffer;
         comp_bitstream_offset += tile_group_obu_size;
         writtenTileBytes += tile_group_obu_size;

         // Flush batch with upload work and wait on this CPU thread for GPU work completion
         // for this case we have to do it within the loop, so the CUP arrays that are reused
         // on each loop are completely done being read by the GPU before we modify them.
         {
            struct pipe_fence_handle *pUploadGPUCompletionFence = NULL;
            pD3D12Enc->base.context->flush(pD3D12Enc->base.context,
                                           &pUploadGPUCompletionFence,
                                           PIPE_FLUSH_ASYNC | PIPE_FLUSH_HINT_FINISH);
            assert(pUploadGPUCompletionFence);
            debug_printf(
               "[d3d12_video_encoder] d3d12_video_encoder_build_post_encode_codec_bitstream_av1 - Waiting on GPU "
               "completion fence for "
               " uploading finalized codec headers to compressed bitstream.\n");

            pD3D12Enc->m_pD3D12Screen->base.fence_finish(&pD3D12Enc->m_pD3D12Screen->base,
                                                         NULL,
                                                         pUploadGPUCompletionFence,
                                                         OS_TIMEOUT_INFINITE);
            pD3D12Enc->m_pD3D12Screen->base.fence_reference(&pD3D12Enc->m_pD3D12Screen->base,
                                                            &pUploadGPUCompletionFence,
                                                            NULL);
         }
      }
   }

   size_t extra_show_existing_frame_payload_bytes = 0;
   if (!picHdr.show_frame) {

      // When writing in the bitstream a frame that will be accessed as reference by a frame we did not
      // write to the bitstream yet, let's store them for future show_existing_frame mechanism
      pD3D12Enc->m_currentEncodeConfig.m_encoderCodecSpecificStateDescAV1.pendingShowableFrames.push_back(
         associatedMetadata.m_associatedEncodeConfig.m_encoderPicParamsDesc.m_AV1PicData.PictureIndex);

   } else {   // picHdr.show_frame true

      // Check if any of the pending "to be showed later" frames is present in the current frame references
      // and if so issue the show_existing_frame header and remove it from the pending list

      for (auto pendingFrameIt =
              pD3D12Enc->m_currentEncodeConfig.m_encoderCodecSpecificStateDescAV1.pendingShowableFrames.begin();
           pendingFrameIt !=
           pD3D12Enc->m_currentEncodeConfig.m_encoderCodecSpecificStateDescAV1.pendingShowableFrames.end();) {

         // Check if current frame references uses this pendingFrameIt
         int cur_ref_idx_matching_pending = -1;
         for (unsigned i = 0; i < ARRAY_SIZE(picHdr.ref_frame_idx) /*NUM_REF_FRAMES*/; i++) {
            if (
               // Is a valid reference
               (associatedMetadata.m_associatedEncodeConfig.m_encoderPicParamsDesc.m_AV1PicData
                   .ReferenceFramesReconPictureDescriptors[picHdr.ref_frame_idx[i]]
                   .ReconstructedPictureResourceIndex != UNUSED_VIRTUAL_DPB_SLOT_PHYSICAL_INDEX) &&
               // And matches the pending frame PictureIndex
               (associatedMetadata.m_associatedEncodeConfig.m_encoderPicParamsDesc.m_AV1PicData
                   .ReferenceFramesReconPictureDescriptors[picHdr.ref_frame_idx[i]]
                   .PictureIndex == *pendingFrameIt /*PictureIndex*/)) {

               // Store the reference index
               cur_ref_idx_matching_pending = picHdr.ref_frame_idx[i];
               break;
            }
         }

         // If we found a reference of the current frame using pendingFrameIt,
         // - Issue the show_existing_frame header
         // - Remove it from the pending list
         if (cur_ref_idx_matching_pending >= 0) {
            size_t staging_buf_offset = pD3D12Enc->m_BitstreamHeadersBuffer.size();


            size_t writtenTemporalDelimBytes = 0;
            if (D3D12_VIDEO_AV1_INSERT_SHOW_EXISTING_FRAME_HEADER) {
               pAV1BitstreamBuilder->write_temporal_delimiter_obu(
                  pD3D12Enc->m_BitstreamHeadersBuffer,
                  pD3D12Enc->m_BitstreamHeadersBuffer.begin() + staging_buf_offset,   // placingPositionStart
                  writtenTemporalDelimBytes   // Bytes Written AFTER placingPositionStart arg above
               );
            }
            associatedMetadata.pWrittenCodecUnitsSizes.push_back(writtenTemporalDelimBytes);
            assert(writtenTemporalDelimBytes == (pD3D12Enc->m_BitstreamHeadersBuffer.size() - staging_buf_offset));

            // Add current pending frame being processed in the loop
            extra_show_existing_frame_payload_bytes += writtenTemporalDelimBytes;

            debug_printf("Written OBU_TEMPORAL_DELIMITER bytes: %" PRIu64 "\n", static_cast<uint64_t>(writtenTemporalDelimBytes));

            size_t writtenShowExistingFrameBytes = 0;
            av1_pic_header_t showExistingPicHdr = {};
            showExistingPicHdr.show_existing_frame = 1;
            showExistingPicHdr.frame_to_show_map_idx = cur_ref_idx_matching_pending;

            if (D3D12_VIDEO_AV1_INSERT_SHOW_EXISTING_FRAME_HEADER) {
               pAV1BitstreamBuilder->write_frame_header(
                  NULL,   // No seq header necessary for show_existing_frame
                  &showExistingPicHdr,
                  OBU_FRAME_HEADER,
                  0,   // No tile info for OBU_FRAME_HEADER
                  pD3D12Enc->m_BitstreamHeadersBuffer,
                  pD3D12Enc->m_BitstreamHeadersBuffer.begin() + staging_buf_offset +
                     writtenTemporalDelimBytes,   // placingPositionStart
                  writtenShowExistingFrameBytes   // Bytes Written AFTER placingPositionStart arg above
               );
            }
            associatedMetadata.pWrittenCodecUnitsSizes.push_back(writtenShowExistingFrameBytes);

            assert(writtenShowExistingFrameBytes ==
                   (pD3D12Enc->m_BitstreamHeadersBuffer.size() - staging_buf_offset - writtenTemporalDelimBytes));

            // Add current pending frame being processed in the loop
            extra_show_existing_frame_payload_bytes += writtenShowExistingFrameBytes;

            assert(pD3D12Enc->m_BitstreamHeadersBuffer.size() ==
                   (staging_buf_offset + writtenShowExistingFrameBytes + writtenTemporalDelimBytes));

            // Upload headers to the finalized compressed bitstream buffer
            // Note: The buffer_subdata is queued in pD3D12Enc->base.context but doesn't execute immediately
            pD3D12Enc->base.context->buffer_subdata(pD3D12Enc->base.context,                   // context
                                                    associatedMetadata.comp_bit_destination,   // comp. bitstream
                                                    PIPE_MAP_WRITE,                            // usage PIPE_MAP_x
                                                    comp_bitstream_offset,                     // offset
                                                    writtenShowExistingFrameBytes + writtenTemporalDelimBytes,
                                                    pD3D12Enc->m_BitstreamHeadersBuffer.data() + staging_buf_offset);

            comp_bitstream_offset += writtenShowExistingFrameBytes;
            comp_bitstream_offset += writtenTemporalDelimBytes;

            debug_printf("Written show_existing_frame OBU_FRAME header for previous frame with PictureIndex %d (Used "
                         "in current frame ref_frame_idx[%" PRIu32 "]) bytes: %" PRIu64 "\n",
                         *pendingFrameIt /*PictureIndex*/,
                         showExistingPicHdr.frame_to_show_map_idx,
                         static_cast<uint64_t>(writtenShowExistingFrameBytes));

            // Remove it from the list of pending frames
            pendingFrameIt =
               pD3D12Enc->m_currentEncodeConfig.m_encoderCodecSpecificStateDescAV1.pendingShowableFrames.erase(
                  pendingFrameIt);
         } else {
            pendingFrameIt++;
         }
      }

      // Flush again if uploaded show_existing_frame header(s) to bitstream.
      if (extra_show_existing_frame_payload_bytes) {
         struct pipe_fence_handle *pUploadGPUCompletionFence = NULL;
         pD3D12Enc->base.context->flush(pD3D12Enc->base.context,
                                        &pUploadGPUCompletionFence,
                                        PIPE_FLUSH_ASYNC | PIPE_FLUSH_HINT_FINISH);
         assert(pUploadGPUCompletionFence);
         debug_printf("[d3d12_video_encoder] [show_existing_frame extra headers upload] "
                      "d3d12_video_encoder_build_post_encode_codec_bitstream_av1 - Waiting on GPU "
                      "completion fence for "
                      " uploading finalized codec headers to compressed bitstream.\n");

         pD3D12Enc->m_pD3D12Screen->base.fence_finish(&pD3D12Enc->m_pD3D12Screen->base,
                                                      NULL,
                                                      pUploadGPUCompletionFence,
                                                      OS_TIMEOUT_INFINITE);
         pD3D12Enc->m_pD3D12Screen->base.fence_reference(&pD3D12Enc->m_pD3D12Screen->base,
                                                         &pUploadGPUCompletionFence,
                                                         NULL);
      }
   }

   // d3d12_resource_from_resource calls AddRef to it so this should only be deleting
   // the pipe_resource wrapping object and not the underlying spStagingBitstream
   pipe_resource_reference(&src_driver_bitstream, NULL);
   assert(associatedMetadata.spStagingBitstream.Get());

   // Unmap the metadata buffer tmp storage
   pipe_buffer_unmap(pD3D12Enc->base.context, mapTransferMetadata);
   pipe_resource_reference(&pPipeResolvedMetadataBuffer, NULL);

   assert((writtenSequenceBytes + writtenTemporalDelimBytes + writtenFrameBytes +
           extra_show_existing_frame_payload_bytes) == pD3D12Enc->m_BitstreamHeadersBuffer.size());

   uint32_t total_bytes_written = static_cast<uint32_t>(writtenSequenceBytes + writtenTemporalDelimBytes + writtenFrameBytes +
                                    writtenTileBytes + extra_show_existing_frame_payload_bytes);
   assert(std::accumulate(associatedMetadata.pWrittenCodecUnitsSizes.begin(), associatedMetadata.pWrittenCodecUnitsSizes.end(), 0u) ==
      static_cast<uint64_t>(total_bytes_written));
   return total_bytes_written;
}

void
upload_tile_group_obu(struct d3d12_video_encoder *pD3D12Enc,
                      size_t tile_group_obu_size,
                      size_t decode_tile_elements_size,
                      std::vector<uint8_t> &staging_bitstream_buffer,
                      size_t staging_bitstream_buffer_offset,
                      pipe_resource *src_driver_bitstream,
                      pipe_resource *comp_bit_destination,
                      size_t comp_bit_destination_offset,
                      const D3D12_VIDEO_ENCODER_FRAME_SUBREGION_METADATA *pFrameSubregionMetadata,
                      size_t TileSizeBytes,   // Pass already +1'd from TileSizeBytesMinus1
                      const D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_SUBREGIONS_LAYOUT_DATA_TILES &TilesPartition,
                      const av1_tile_group_t &tileGroup,
                      size_t &written_bytes_to_staging_bitstream_buffer,
                      std::vector<uint64_t> &pWrittenCodecUnitsSizes)
{
   debug_printf("[Tile group start %d to end %d] Writing to comp_bit_destination %p starts at offset %" PRIu64 "\n",
                tileGroup.tg_start,
                tileGroup.tg_end,
                comp_bit_destination,
                static_cast<uint64_t>(comp_bit_destination_offset));

   debug_printf("[Tile group start %d to end %d] Using staging_bitstream_buffer %p at offset %" PRIu64
                " to write the tile_obu_group() prefix syntax: tile_start_and_end_present_flag, tg_start, tg_end and "
                "the tile_size_minus1\n",
                tileGroup.tg_start,
                tileGroup.tg_end,
                staging_bitstream_buffer.data(),
                static_cast<uint64_t>(staging_bitstream_buffer_offset));

   // Reserve space upfront in the scratch storage
   // Do not modify anything before staging_bitstream_buffer_offset

   size_t tile_obu_prefix_size = tile_group_obu_size - decode_tile_elements_size;
   if (staging_bitstream_buffer.size() < (staging_bitstream_buffer_offset + tile_obu_prefix_size))
      staging_bitstream_buffer.resize(staging_bitstream_buffer_offset + tile_obu_prefix_size);

   d3d12_video_encoder_bitstream bitstream_tile_group_obu;
   bitstream_tile_group_obu.setup_bitstream(staging_bitstream_buffer.size(),
                                            staging_bitstream_buffer.data(),
                                            staging_bitstream_buffer_offset);

   uint8_t NumTiles = TilesPartition.ColCount * TilesPartition.RowCount;
   bool tile_start_and_end_present_flag = !(tileGroup.tg_start == 0 && (tileGroup.tg_end == (NumTiles - 1)));
   if (NumTiles > 1)
      bitstream_tile_group_obu.put_bits(1,
                                        tile_start_and_end_present_flag);   // tile_start_and_end_present_flag f(1)

   if (!(NumTiles == 1 || !tile_start_and_end_present_flag)) {
      uint8_t tileBits = log2(TilesPartition.ColCount) + log2(TilesPartition.RowCount);
      bitstream_tile_group_obu.put_bits(tileBits, tileGroup.tg_start);   // tg_start	   f(tileBits)
      bitstream_tile_group_obu.put_bits(tileBits, tileGroup.tg_end);     // tg_end	      f(tileBits)
   }

   bitstream_tile_group_obu.put_aligning_bits();   // byte_alignment()

   bitstream_tile_group_obu.flush();

   size_t bitstream_tile_group_obu_bytes = bitstream_tile_group_obu.get_byte_count() - staging_bitstream_buffer_offset;

   debug_printf("[Tile group start %d to end %d] Written %" PRIu64
                " bitstream_tile_group_obu_bytes at staging_bitstream_buffer %p at offset %" PRIu64
                " for tile_obu_group() prefix syntax: tile_start_and_end_present_flag, tg_start, tg_end\n",
                tileGroup.tg_start,
                tileGroup.tg_end,
                static_cast<uint64_t>(bitstream_tile_group_obu_bytes),
                staging_bitstream_buffer.data(),
                static_cast<uint64_t>(staging_bitstream_buffer_offset));


   // Save this to compare the final written destination byte size against the expected tile_group_obu_size
   // at the end of the function
   ASSERTED size_t comp_bit_destination_offset_before_upload = comp_bit_destination_offset;

   // Upload first part of the header to compressed bitstream destination
   // Note: The buffer_subdata is queued in pD3D12Enc->base.context but doesn't execute immediately
   if (bitstream_tile_group_obu_bytes > 0) {
      pD3D12Enc->base.context->buffer_subdata(
         pD3D12Enc->base.context,                                              // context
         comp_bit_destination,                                                 // comp. bitstream
         PIPE_MAP_WRITE,                                                       // usage PIPE_MAP_x
         comp_bit_destination_offset,                                          // offset
         bitstream_tile_group_obu_bytes,                                       // size
         staging_bitstream_buffer.data() + staging_bitstream_buffer_offset);   // data

      debug_printf("[Tile group start %d to end %d]  Uploading %" PRIu64 " bytes"
                   " for tile_obu_group() prefix syntax: tile_start_and_end_present_flag, tg_start, tg_end"
                   " from staging_bitstream_buffer %p at offset %" PRIu64
                   " to comp_bit_destination %p at offset %" PRIu64 "\n",
                   tileGroup.tg_start,
                   tileGroup.tg_end,
                   static_cast<uint64_t>(bitstream_tile_group_obu_bytes),
                   staging_bitstream_buffer.data(),
                   static_cast<uint64_t>(staging_bitstream_buffer_offset),
                   comp_bit_destination,
                   static_cast<uint64_t>(comp_bit_destination_offset));

      comp_bit_destination_offset += bitstream_tile_group_obu_bytes;
      written_bytes_to_staging_bitstream_buffer += bitstream_tile_group_obu_bytes;
   }

   size_t src_offset = 0;
   for (UINT64 TileIdx = tileGroup.tg_start; TileIdx <= tileGroup.tg_end; TileIdx++) {
      size_t tile_size = pFrameSubregionMetadata[TileIdx].bSize - pFrameSubregionMetadata[TileIdx].bStartOffset;
      // The i-th tile is read from the src_buffer[offset] with offset = [sum j = (0, (i-1)){ tile[j].bSize }] +
      // tile[i].bStartOffset
      size_t src_buf_tile_position = src_offset + pFrameSubregionMetadata[TileIdx].bStartOffset;
      src_offset += pFrameSubregionMetadata[TileIdx].bSize;

      // tile_size_minus_1	not coded for last tile
      if ((TileIdx != tileGroup.tg_end)) {
         bitstream_tile_group_obu.put_le_bytes(TileSizeBytes,   // tile_size_minus_1	le(TileSizeBytes)
                                               tile_size - 1 /* convert to ..._minus_1 */);
         bitstream_tile_group_obu.flush();

         debug_printf("[Tile group start %d to end %d] [TileIdx %" PRIu64 "] Written %" PRIu64
                      " written_bytes_to_staging_bitstream_buffer at staging_bitstream_buffer %p at offset %" PRIu64
                      " for the tile_size_minus1 syntax\n",
                      tileGroup.tg_start,
                      tileGroup.tg_end,
                      TileIdx,
                      static_cast<uint64_t>(TileSizeBytes),
                      staging_bitstream_buffer.data(),
                      static_cast<uint64_t>(written_bytes_to_staging_bitstream_buffer + staging_bitstream_buffer_offset));

         // Upload current tile_size_minus_1
         // Note: The buffer_subdata is queued in pD3D12Enc->base.context but doesn't execute immediately
         pD3D12Enc->base.context->buffer_subdata(pD3D12Enc->base.context,       // context
                                                 comp_bit_destination,          // comp. bitstream
                                                 PIPE_MAP_WRITE,                // usage PIPE_MAP_x
                                                 comp_bit_destination_offset,   // offset
                                                 TileSizeBytes,                 // size
                                                 staging_bitstream_buffer.data() +
                                                    written_bytes_to_staging_bitstream_buffer +
                                                    staging_bitstream_buffer_offset);   // data

         debug_printf("[Tile group start %d to end %d] [TileIdx %" PRIu64 "] Uploading %" PRIu64 " bytes"
                      " for tile_obu_group() prefix syntax: tile_size_minus_1"
                      " from staging_bitstream_buffer %p at offset %" PRIu64
                      " to comp_bit_destination %p at offset %" PRIu64 "\n",
                      tileGroup.tg_start,
                      tileGroup.tg_end,
                      TileIdx,
                      static_cast<uint64_t>(TileSizeBytes),
                      staging_bitstream_buffer.data(),
                      static_cast<uint64_t>(written_bytes_to_staging_bitstream_buffer + staging_bitstream_buffer_offset),
                      comp_bit_destination,
                      static_cast<uint64_t>(comp_bit_destination_offset));

         comp_bit_destination_offset += TileSizeBytes;
         written_bytes_to_staging_bitstream_buffer += TileSizeBytes;
      }

      // Now copy the decode_tile() element from the driver staging GPU buffer onto the finalized GPU buffer

      struct pipe_box src_box = {
         static_cast<int>(src_buf_tile_position),   // x
         0,                                         // y
         0,                                         // z
         static_cast<int>(tile_size),               // width
         1,                                         // height
         1                                          // depth
      };

      pD3D12Enc->base.context->resource_copy_region(pD3D12Enc->base.context,       // ctx
                                                    comp_bit_destination,          // dst
                                                    0,                             // dst_level
                                                    comp_bit_destination_offset,   // dstX
                                                    0,                             // dstY
                                                    0,                             // dstZ
                                                    src_driver_bitstream,          // src
                                                    0,                             // src level
                                                    &src_box);

      debug_printf("[Tile group start %d to end %d] [TileIdx %" PRIu64 "] Written %" PRIu64
                   " tile_size bytes data from src_driver_bitstream %p at offset %" PRIu64
                   " into comp_bit_destination %p at offset %" PRIu64 "\n",
                   tileGroup.tg_start,
                   tileGroup.tg_end,
                   TileIdx,
                   static_cast<uint64_t>(tile_size),
                   src_driver_bitstream,
                   static_cast<uint64_t>(src_buf_tile_position),
                   comp_bit_destination,
                   static_cast<uint64_t>(comp_bit_destination_offset));

      comp_bit_destination_offset += tile_size;

      size_t cur_tile_reportable_size = tile_size;
      if (TileIdx != tileGroup.tg_end)
         cur_tile_reportable_size += TileSizeBytes; /* extra tile_size_bytes_minus1 in all tiles except last*/
      if (TileIdx == 0)
         cur_tile_reportable_size += bitstream_tile_group_obu_bytes; // part of the obu tile group header (make part of first tile)
      pWrittenCodecUnitsSizes.push_back(cur_tile_reportable_size);
   }

   // Make sure we wrote the expected bytes that match the obu_size elements
   // in the OBUs wrapping this uploaded tile_group_obu
   assert((comp_bit_destination_offset - comp_bit_destination_offset_before_upload) == tile_group_obu_size);
}

void
d3d12_video_encoder_store_current_picture_references_av1(d3d12_video_encoder *pD3D12Enc, uint64_t current_metadata_slot)
{
   // Update DX12 picparams for post execution (get_feedback) after d3d12_video_encoder_references_manager_av1
   // changes
   pD3D12Enc->m_spEncodedFrameMetadata[current_metadata_slot].m_associatedEncodeConfig.m_encoderPicParamsDesc =
      pD3D12Enc->m_currentEncodeConfig.m_encoderPicParamsDesc;
}
