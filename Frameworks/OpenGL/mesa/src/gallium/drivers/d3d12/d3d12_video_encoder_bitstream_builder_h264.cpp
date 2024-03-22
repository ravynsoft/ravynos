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

#include "d3d12_video_encoder_bitstream_builder_h264.h"

#include <cmath>
#include "util/u_video.h"

d3d12_video_bitstream_builder_h264::d3d12_video_bitstream_builder_h264(bool insert_aud_nalu)
   : m_insert_aud_nalu(insert_aud_nalu)
{ }

inline H264_SPEC_PROFILES
Convert12ToSpecH264Profiles(D3D12_VIDEO_ENCODER_PROFILE_H264 profile12)
{
   switch (profile12) {
      case D3D12_VIDEO_ENCODER_PROFILE_H264_MAIN:
      {
         return H264_PROFILE_MAIN;
      } break;
      case D3D12_VIDEO_ENCODER_PROFILE_H264_HIGH:
      {
         return H264_PROFILE_HIGH;
      } break;
      case D3D12_VIDEO_ENCODER_PROFILE_H264_HIGH_10:
      {
         return H264_PROFILE_HIGH10;
      } break;
      default:
      {
         unreachable("Unsupported D3D12_VIDEO_ENCODER_PROFILE_H264");
      } break;
   }
}

void
d3d12_video_bitstream_builder_h264::build_sps(const struct pipe_h264_enc_seq_param &                 seqData,
                                              const enum pipe_video_profile &                        profile,
                                              const D3D12_VIDEO_ENCODER_LEVELS_H264 &                level,
                                              const DXGI_FORMAT &                                    inputFmt,
                                              const D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_H264 &   codecConfig,
                                              const D3D12_VIDEO_ENCODER_SEQUENCE_GOP_STRUCTURE_H264 &gopConfig,
                                              uint32_t                                    seq_parameter_set_id,
                                              uint32_t                                    max_num_ref_frames,
                                              D3D12_VIDEO_ENCODER_PICTURE_RESOLUTION_DESC sequenceTargetResolution,
                                              D3D12_BOX                                   frame_cropping_codec_config,
                                              std::vector<uint8_t> &                      headerBitstream,
                                              std::vector<uint8_t>::iterator              placingPositionStart,
                                              size_t &                                    writtenBytes)
{
   H264_SPEC_PROFILES profile_idc          = (H264_SPEC_PROFILES) u_get_h264_profile_idc(profile);
   uint32_t           level_idc            = 0;
   d3d12_video_encoder_convert_from_d3d12_level_h264(
      level,
      level_idc);

   assert((inputFmt == DXGI_FORMAT_NV12) || (inputFmt == DXGI_FORMAT_P010));

   // Assume NV12 YUV 420 8 bits
   uint32_t bit_depth_luma_minus8   = 0;
   uint32_t bit_depth_chroma_minus8 = 0;

   // In case is 420 10 bits fix it
   if (inputFmt == DXGI_FORMAT_P010) {
      bit_depth_luma_minus8   = 2;
      bit_depth_chroma_minus8 = 2;
   }

   // Calculate sequence resolution sizes in MBs
   // Always in MBs since we don't support interlace in D3D12 Encode
   uint32_t pic_width_in_mbs_minus1 = static_cast<uint32_t>(std::ceil(sequenceTargetResolution.Width / 16.0)) - 1;
   uint32_t pic_height_in_map_units_minus1 =
      static_cast<uint32_t>(std::ceil(sequenceTargetResolution.Height / 16.0)) - 1;

   uint32_t frame_cropping_flag               = 0;
   if (frame_cropping_codec_config.left 
      || frame_cropping_codec_config.right 
      || frame_cropping_codec_config.top
      || frame_cropping_codec_config.bottom
   ) {
      frame_cropping_flag               = 1;
   }

   H264_SPS spsStructure = { static_cast<uint32_t>(profile_idc),
                             seqData.enc_constraint_set_flags,
                             level_idc,
                             seq_parameter_set_id,
                             bit_depth_luma_minus8,
                             bit_depth_chroma_minus8,
                             gopConfig.log2_max_frame_num_minus4,
                             gopConfig.pic_order_cnt_type,
                             gopConfig.log2_max_pic_order_cnt_lsb_minus4,
                             max_num_ref_frames,
                             0,   // gaps_in_frame_num_value_allowed_flag
                             pic_width_in_mbs_minus1,
                             pic_height_in_map_units_minus1,
                             ((codecConfig.ConfigurationFlags &
                               D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_H264_FLAG_USE_ADAPTIVE_8x8_TRANSFORM) != 0) ?
                                1u :
                                0u,   // direct_8x8_inference_flag
                             frame_cropping_flag,
                             frame_cropping_codec_config.left,
                             frame_cropping_codec_config.right,
                             frame_cropping_codec_config.top,
                             frame_cropping_codec_config.bottom };

   // Copy VUI params from seqData
   spsStructure.vui_parameters_present_flag = seqData.vui_parameters_present_flag;
   spsStructure.vui.aspect_ratio_info_present_flag = seqData.vui_flags.aspect_ratio_info_present_flag;
   spsStructure.vui.timing_info_present_flag = seqData.vui_flags.timing_info_present_flag;
   spsStructure.vui.video_signal_type_present_flag = seqData.vui_flags.video_signal_type_present_flag;
   spsStructure.vui.colour_description_present_flag = seqData.vui_flags.colour_description_present_flag;
   spsStructure.vui.chroma_loc_info_present_flag = seqData.vui_flags.chroma_loc_info_present_flag;
   spsStructure.vui.overscan_info_present_flag = seqData.vui_flags.overscan_info_present_flag;
   spsStructure.vui.overscan_appropriate_flag = seqData.vui_flags.overscan_appropriate_flag;
   spsStructure.vui.fixed_frame_rate_flag = seqData.vui_flags.fixed_frame_rate_flag;
   spsStructure.vui.nal_hrd_parameters_present_flag = seqData.vui_flags.nal_hrd_parameters_present_flag;
   spsStructure.vui.vcl_hrd_parameters_present_flag = seqData.vui_flags.vcl_hrd_parameters_present_flag;
   spsStructure.vui.low_delay_hrd_flag = seqData.vui_flags.low_delay_hrd_flag;
   spsStructure.vui.pic_struct_present_flag = seqData.vui_flags.pic_struct_present_flag;
   spsStructure.vui.bitstream_restriction_flag = seqData.vui_flags.bitstream_restriction_flag;
   spsStructure.vui.motion_vectors_over_pic_boundaries_flag = seqData.vui_flags.motion_vectors_over_pic_boundaries_flag;
   spsStructure.vui.aspect_ratio_idc = seqData.aspect_ratio_idc;
   spsStructure.vui.sar_width = seqData.sar_width;
   spsStructure.vui.sar_height = seqData.sar_height;
   spsStructure.vui.video_format = seqData.video_format;
   spsStructure.vui.video_full_range_flag = seqData.video_full_range_flag;
   spsStructure.vui.colour_primaries = seqData.colour_primaries;
   spsStructure.vui.transfer_characteristics = seqData.transfer_characteristics;
   spsStructure.vui.matrix_coefficients = seqData.matrix_coefficients;
   spsStructure.vui.chroma_sample_loc_type_top_field = seqData.chroma_sample_loc_type_top_field;
   spsStructure.vui.chroma_sample_loc_type_bottom_field = seqData.chroma_sample_loc_type_bottom_field;
   spsStructure.vui.time_scale = seqData.time_scale;
   spsStructure.vui.num_units_in_tick = seqData.num_units_in_tick;
   memset(&spsStructure.vui.nal_hrd_parameters, 0, sizeof(H264_HRD_PARAMS));
   memset(&spsStructure.vui.vcl_hrd_parameters, 0, sizeof(H264_HRD_PARAMS));
   spsStructure.vui.max_bytes_per_pic_denom = seqData.max_bytes_per_pic_denom;
   spsStructure.vui.max_bits_per_mb_denom = seqData.max_bits_per_mb_denom;
   spsStructure.vui.log2_max_mv_length_vertical = seqData.log2_max_mv_length_vertical;
   spsStructure.vui.log2_max_mv_length_horizontal = seqData.log2_max_mv_length_horizontal;
   spsStructure.vui.num_reorder_frames = seqData.max_num_reorder_frames;
   spsStructure.vui.max_dec_frame_buffering = seqData.max_dec_frame_buffering;

   // Print built SPS structure
   debug_printf(
      "[D3D12 d3d12_video_bitstream_builder_h264] H264_SPS Structure generated before writing to bitstream:\n");
   print_sps(spsStructure);

   // Convert the H264 SPS structure into bytes
   m_h264Encoder.sps_to_nalu_bytes(&spsStructure, headerBitstream, placingPositionStart, writtenBytes);
}

void
d3d12_video_bitstream_builder_h264::write_end_of_stream_nalu(std::vector<uint8_t> &         headerBitstream,
                                                             std::vector<uint8_t>::iterator placingPositionStart,
                                                             size_t &                       writtenBytes)
{
   m_h264Encoder.write_end_of_stream_nalu(headerBitstream, placingPositionStart, writtenBytes);
}

void
d3d12_video_bitstream_builder_h264::write_end_of_sequence_nalu(std::vector<uint8_t> &         headerBitstream,
                                                               std::vector<uint8_t>::iterator placingPositionStart,
                                                               size_t &                       writtenBytes)
{
   m_h264Encoder.write_end_of_sequence_nalu(headerBitstream, placingPositionStart, writtenBytes);
}

void
d3d12_video_bitstream_builder_h264::write_aud(std::vector<uint8_t> &         headerBitstream,
                                              std::vector<uint8_t>::iterator placingPositionStart,
                                              size_t &                       writtenBytes)
{
   m_h264Encoder.write_access_unit_delimiter_nalu(headerBitstream, placingPositionStart, writtenBytes);
}

void
d3d12_video_bitstream_builder_h264::build_pps(const enum pipe_video_profile &                            profile,
                                              const D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_H264 &       codecConfig,
                                              const D3D12_VIDEO_ENCODER_PICTURE_CONTROL_CODEC_DATA_H264 &pictureControl,
                                              uint32_t                       pic_parameter_set_id,
                                              uint32_t                       seq_parameter_set_id,
                                              std::vector<uint8_t> &         headerBitstream,
                                              std::vector<uint8_t>::iterator placingPositionStart,
                                              size_t &                       writtenBytes)
{
   BOOL bIsHighProfile =
      ((profile == PIPE_VIDEO_PROFILE_MPEG4_AVC_HIGH) || (profile == PIPE_VIDEO_PROFILE_MPEG4_AVC_HIGH10));

   H264_PPS ppsStructure = {
      pic_parameter_set_id,
      seq_parameter_set_id,
      ((codecConfig.ConfigurationFlags & D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_H264_FLAG_ENABLE_CABAC_ENCODING) != 0) ?
         1u :
         0u,   // entropy_coding_mode_flag
      0,   // pic_order_present_flag (bottom_field_pic_order_in_frame_present_flag) - will use pic_cnt 0 or 2, always
           // off ; used with pic_cnt_type 1 and deltas.
      static_cast<uint32_t>(std::max(static_cast<int32_t>(pictureControl.List0ReferenceFramesCount) - 1,
                                     0)),   // num_ref_idx_l0_active_minus1
      static_cast<uint32_t>(std::max(static_cast<int32_t>(pictureControl.List1ReferenceFramesCount) - 1,
                                     0)),   // num_ref_idx_l1_active_minus1
      ((codecConfig.ConfigurationFlags &
        D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_H264_FLAG_USE_CONSTRAINED_INTRAPREDICTION) != 0) ?
         1u :
         0u,   // constrained_intra_pred_flag
      ((codecConfig.ConfigurationFlags &
        D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_H264_FLAG_USE_ADAPTIVE_8x8_TRANSFORM) != 0) ?
         1u :
         0u   // transform_8x8_mode_flag
   };

   // Print built PPS structure
   debug_printf(
      "[D3D12 d3d12_video_bitstream_builder_h264] H264_PPS Structure generated before writing to bitstream:\n");
   print_pps(ppsStructure);

   // Convert the H264 SPS structure into bytes
   m_h264Encoder.pps_to_nalu_bytes(&ppsStructure, headerBitstream, bIsHighProfile, placingPositionStart, writtenBytes);
}

void
d3d12_video_bitstream_builder_h264::print_pps(const H264_PPS &pps)
{
   // Be careful that build_pps also wraps some other NALU bytes in pps_to_nalu_bytes so bitstream returned by build_pps
   // won't be exactly the bytes from the H264_PPS struct

   static_assert(sizeof(H264_PPS) ==
                 (sizeof(uint32_t) *
                  8), "Update the number of uint32_t in struct in assert and add case below if structure changes");

   // Declared fields from definition in d3d12_video_encoder_bitstream_builder_h264.h

   debug_printf("[D3D12 d3d12_video_bitstream_builder_h264] H264_PPS values below:\n");
   debug_printf("pic_parameter_set_id: %d\n", pps.pic_parameter_set_id);
   debug_printf("seq_parameter_set_id: %d\n", pps.seq_parameter_set_id);
   debug_printf("entropy_coding_mode_flag: %d\n", pps.entropy_coding_mode_flag);
   debug_printf("pic_order_present_flag: %d\n", pps.pic_order_present_flag);
   debug_printf("num_ref_idx_l0_active_minus1: %d\n", pps.num_ref_idx_l0_active_minus1);
   debug_printf("num_ref_idx_l1_active_minus1: %d\n", pps.num_ref_idx_l1_active_minus1);
   debug_printf("constrained_intra_pred_flag: %d\n", pps.constrained_intra_pred_flag);
   debug_printf("transform_8x8_mode_flag: %d\n", pps.transform_8x8_mode_flag);
   debug_printf(
      "[D3D12 d3d12_video_bitstream_builder_h264] H264_PPS values end\n--------------------------------------\n");
}

static void
print_hrd(const H264_HRD_PARAMS *pHrd)
{
   debug_printf("cpb_cnt_minus1: %d\n", pHrd->cpb_cnt_minus1);
   debug_printf("bit_rate_scale: %d\n", pHrd->bit_rate_scale);
   debug_printf("cpb_size_scale: %d\n", pHrd->cpb_size_scale);
   for (uint32_t i = 0; i <= pHrd->cpb_cnt_minus1; i++)
   {
      debug_printf("bit_rate_value_minus1[%d]: %d\n", i, pHrd->bit_rate_value_minus1[i]);
      debug_printf("cpb_size_value_minus1[%d]: %d\n", i, pHrd->cpb_size_value_minus1[i]);
      debug_printf("cbr_flag[%d]: %d\n", i, pHrd->cbr_flag[i]);
   }
   debug_printf("initial_cpb_removal_delay_length_minus1: %d\n", pHrd->initial_cpb_removal_delay_length_minus1);
   debug_printf("cpb_removal_delay_length_minus1: %d\n", pHrd->cpb_removal_delay_length_minus1);
   debug_printf("dpb_output_delay_length_minus1: %d\n", pHrd->dpb_output_delay_length_minus1);
   debug_printf("time_offset_length: %d\n", pHrd->time_offset_length);
}

void
d3d12_video_bitstream_builder_h264::print_sps(const H264_SPS &sps)
{
   // Be careful when calling this method that build_sps also wraps some other NALU bytes in sps_to_nalu_bytes so
   // bitstream returned by build_sps won't be exactly the bytes from the H264_SPS struct From definition in
   // d3d12_video_encoder_bitstream_builder_h264.h

   static_assert(sizeof(H264_SPS) ==
                (sizeof(uint32_t) * 20 + sizeof(H264_VUI_PARAMS)), "Update the print function if structure changes");

   static_assert(sizeof(H264_VUI_PARAMS) ==
                 (sizeof(uint32_t) * 32 + 2*sizeof(H264_HRD_PARAMS)), "Update the print function if structure changes");

   static_assert(sizeof(H264_HRD_PARAMS) ==
                 (sizeof(uint32_t) * 7 + 3*32*sizeof(uint32_t)), "Update the print function if structure changes");

   // Declared fields from definition in d3d12_video_encoder_bitstream_builder_h264.h
   debug_printf("[D3D12 d3d12_video_bitstream_builder_h264] H264_SPS values below:\n");
   debug_printf("profile_idc: %d\n", sps.profile_idc);
   debug_printf("constraint_set_flags: %x\n", sps.constraint_set_flags);
   debug_printf("level_idc: %d\n", sps.level_idc);
   debug_printf("seq_parameter_set_id: %d\n", sps.seq_parameter_set_id);
   debug_printf("bit_depth_luma_minus8: %d\n", sps.bit_depth_luma_minus8);
   debug_printf("bit_depth_chroma_minus8: %d\n", sps.bit_depth_chroma_minus8);
   debug_printf("log2_max_frame_num_minus4: %d\n", sps.log2_max_frame_num_minus4);
   debug_printf("pic_order_cnt_type: %d\n", sps.pic_order_cnt_type);
   debug_printf("log2_max_pic_order_cnt_lsb_minus4: %d\n", sps.log2_max_pic_order_cnt_lsb_minus4);
   debug_printf("max_num_ref_frames: %d\n", sps.max_num_ref_frames);
   debug_printf("gaps_in_frame_num_value_allowed_flag: %d\n", sps.gaps_in_frame_num_value_allowed_flag);
   debug_printf("pic_width_in_mbs_minus1: %d\n", sps.pic_width_in_mbs_minus1);
   debug_printf("pic_height_in_map_units_minus1: %d\n", sps.pic_height_in_map_units_minus1);
   debug_printf("direct_8x8_inference_flag: %d\n", sps.direct_8x8_inference_flag);
   debug_printf("frame_cropping_flag: %d\n", sps.frame_cropping_flag);
   debug_printf("frame_cropping_rect_left_offset: %d\n", sps.frame_cropping_rect_left_offset);
   debug_printf("frame_cropping_rect_right_offset: %d\n", sps.frame_cropping_rect_right_offset);
   debug_printf("frame_cropping_rect_top_offset: %d\n", sps.frame_cropping_rect_top_offset);
   debug_printf("frame_cropping_rect_bottom_offset: %d\n", sps.frame_cropping_rect_bottom_offset);
   debug_printf("VUI.vui_parameters_present_flag: %d\n", sps.vui_parameters_present_flag);
   debug_printf("VUI.aspect_ratio_info_present_flag: %d\n", sps.vui.aspect_ratio_info_present_flag);
   debug_printf("VUI.aspect_ratio_idc: %d\n", sps.vui.aspect_ratio_idc);
   debug_printf("VUI.sar_width: %d\n", sps.vui.sar_width);
   debug_printf("VUI.sar_height: %d\n", sps.vui.sar_height);
   debug_printf("VUI.overscan_info_present_flag: %d\n", sps.vui.overscan_info_present_flag);
   debug_printf("VUI.overscan_appropriate_flag: %d\n", sps.vui.overscan_appropriate_flag);
   debug_printf("VUI.video_signal_type_present_flag: %d\n", sps.vui.video_signal_type_present_flag);
   debug_printf("VUI.video_format: %d\n", sps.vui.video_format);
   debug_printf("VUI.video_full_range_flag: %d\n", sps.vui.video_full_range_flag);
   debug_printf("VUI.colour_description_present_flag: %d\n", sps.vui.colour_description_present_flag);
   debug_printf("VUI.colour_primaries: %d\n", sps.vui.colour_primaries);
   debug_printf("VUI.transfer_characteristics: %d\n", sps.vui.transfer_characteristics);
   debug_printf("VUI.matrix_coefficients: %d\n", sps.vui.matrix_coefficients);
   debug_printf("VUI.chroma_loc_info_present_flag: %d\n", sps.vui.chroma_loc_info_present_flag);
   debug_printf("VUI.chroma_sample_loc_type_top_field: %d\n", sps.vui.chroma_sample_loc_type_top_field);
   debug_printf("VUI.chroma_sample_loc_type_bottom_field: %d\n", sps.vui.chroma_sample_loc_type_bottom_field);
   debug_printf("VUI.timing_info_present_flag: %d\n", sps.vui.timing_info_present_flag);
   debug_printf("VUI.time_scale: %d\n", sps.vui.time_scale);
   debug_printf("VUI.num_units_in_tick: %d\n", sps.vui.num_units_in_tick);
   debug_printf("VUI.fixed_frame_rate_flag: %d\n", sps.vui.fixed_frame_rate_flag);
   debug_printf("VUI.nal_hrd_parameters_present_flag: %d\n", sps.vui.nal_hrd_parameters_present_flag);
   debug_printf("VUI.sps.vui.nal_hrd_parameters\n");
   print_hrd(&sps.vui.nal_hrd_parameters);
   debug_printf("VUI.vcl_hrd_parameters_present_flag: %d\n", sps.vui.vcl_hrd_parameters_present_flag);
   debug_printf("VUI.sps.vui.vcl_hrd_parameters\n");
   print_hrd(&sps.vui.vcl_hrd_parameters);
   debug_printf("VUI.low_delay_hrd_flag: %d\n", sps.vui.low_delay_hrd_flag);
   debug_printf("VUI.pic_struct_present_flag: %d\n", sps.vui.pic_struct_present_flag);
   debug_printf("VUI.bitstream_restriction_flag: %d\n", sps.vui.bitstream_restriction_flag);
   debug_printf("VUI.motion_vectors_over_pic_boundaries_flag: %d\n", sps.vui.motion_vectors_over_pic_boundaries_flag);
   debug_printf("VUI.max_bytes_per_pic_denom: %d\n", sps.vui.max_bytes_per_pic_denom);
   debug_printf("VUI.max_bits_per_mb_denom: %d\n", sps.vui.max_bits_per_mb_denom);
   debug_printf("VUI.log2_max_mv_length_vertical: %d\n", sps.vui.log2_max_mv_length_vertical);
   debug_printf("VUI.log2_max_mv_length_horizontal: %d\n", sps.vui.log2_max_mv_length_horizontal);
   debug_printf("VUI.num_reorder_frames: %d\n", sps.vui.num_reorder_frames);
   debug_printf("VUI.max_dec_frame_buffering: %d\n", sps.vui.max_dec_frame_buffering);

   debug_printf(
      "[D3D12 d3d12_video_bitstream_builder_h264] H264_SPS values end\n--------------------------------------\n");
}
