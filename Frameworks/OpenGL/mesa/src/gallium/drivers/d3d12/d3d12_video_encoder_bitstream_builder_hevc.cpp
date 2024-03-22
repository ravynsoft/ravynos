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

#include "d3d12_video_encoder_bitstream_builder_hevc.h"
#include <cmath>

static uint8_t
convert_profile12_to_stdprofile(D3D12_VIDEO_ENCODER_PROFILE_HEVC profile)
{
    // Main is 1, Main10 is 2, one more than the D3D12 enum definition
    return static_cast<uint8_t>(profile) + 1u;
}

void
d3d12_video_bitstream_builder_hevc::init_profile_tier_level(HEVCProfileTierLevel *ptl,
                        uint8_t HEVCProfileIdc,
                        uint8_t HEVCLevelIdc,
                        bool isHighTier)
{
   memset(ptl, 0, sizeof(HEVCProfileTierLevel));

   ptl->general_profile_space = 0;     // must be 0
   ptl->general_tier_flag = isHighTier ? 1 : 0;
   ptl->general_profile_idc = HEVCProfileIdc;

   memset(ptl->general_profile_compatibility_flag, 0, sizeof(ptl->general_profile_compatibility_flag));
   ptl->general_profile_compatibility_flag[ptl->general_profile_idc] = 1;

   ptl->general_progressive_source_flag = 1; // yes
   ptl->general_interlaced_source_flag = 0;  // no
   ptl->general_non_packed_constraint_flag = 1; // no frame packing arrangement SEI messages
   ptl->general_frame_only_constraint_flag = 1;
   ptl->general_level_idc = HEVCLevelIdc;
}

void
d3d12_video_encoder_convert_from_d3d12_level_hevc(D3D12_VIDEO_ENCODER_LEVELS_HEVC level12,
                                                  uint32_t &specLevel)
{
   specLevel = 3u;
   switch(level12)
   {
      case D3D12_VIDEO_ENCODER_LEVELS_HEVC_1:
      {
         specLevel *= 10;
      } break;
      case D3D12_VIDEO_ENCODER_LEVELS_HEVC_2:
      {
         specLevel *= 20;
      } break;
      case D3D12_VIDEO_ENCODER_LEVELS_HEVC_21:
      {
         specLevel *= 21;
      } break;
      case D3D12_VIDEO_ENCODER_LEVELS_HEVC_3:
      {
         specLevel *= 30;
      } break;
      case D3D12_VIDEO_ENCODER_LEVELS_HEVC_31:
      {
         specLevel *= 31;
      } break;
      case D3D12_VIDEO_ENCODER_LEVELS_HEVC_4:
      {
         specLevel *= 40;
      } break;
      case D3D12_VIDEO_ENCODER_LEVELS_HEVC_41:
      {
         specLevel *= 41;
      } break;
      case D3D12_VIDEO_ENCODER_LEVELS_HEVC_5:
      {
         specLevel *= 50;
      } break;
      case D3D12_VIDEO_ENCODER_LEVELS_HEVC_51:
      {
         specLevel *= 51;
      } break;
      case D3D12_VIDEO_ENCODER_LEVELS_HEVC_52:
      {
         specLevel *= 52;
      } break;
      case D3D12_VIDEO_ENCODER_LEVELS_HEVC_6 :
      {
         specLevel *= 60;
      } break;
      case D3D12_VIDEO_ENCODER_LEVELS_HEVC_61 :
      {
         specLevel *= 61;
      } break;
      case D3D12_VIDEO_ENCODER_LEVELS_HEVC_62 :
      {
         specLevel *= 62;
      } break;
      default:
      {
         unreachable("Unsupported D3D12_VIDEO_ENCODER_LEVELS_HEVC value");
      } break;        
   }
}

D3D12_VIDEO_ENCODER_LEVELS_HEVC
d3d12_video_encoder_convert_level_hevc(uint32_t hevcSpecLevel)
{
   hevcSpecLevel = hevcSpecLevel / 3u;
   switch(hevcSpecLevel)
   {
      case 10:
      {
         return D3D12_VIDEO_ENCODER_LEVELS_HEVC_1;
      } break;
      case 20:
      {
         return D3D12_VIDEO_ENCODER_LEVELS_HEVC_2;
      } break;
      case 21:
      {
         return D3D12_VIDEO_ENCODER_LEVELS_HEVC_21;
      } break;
      case 30:
      {
         return D3D12_VIDEO_ENCODER_LEVELS_HEVC_3;
      } break;
      case 31:
      {
         return D3D12_VIDEO_ENCODER_LEVELS_HEVC_31;
      } break;
      case 40:
      {
         return D3D12_VIDEO_ENCODER_LEVELS_HEVC_4;
      } break;
      case 41:
      {
         return D3D12_VIDEO_ENCODER_LEVELS_HEVC_41;
      } break;
      case 50:
      {
         return D3D12_VIDEO_ENCODER_LEVELS_HEVC_5;
      } break;
      case 51:
      {
         return D3D12_VIDEO_ENCODER_LEVELS_HEVC_51;
      } break;
      case 52:
      {
         return D3D12_VIDEO_ENCODER_LEVELS_HEVC_52;
      } break;
      case 60:
      {
         return D3D12_VIDEO_ENCODER_LEVELS_HEVC_6;
      } break;
      case 61:
      {
         return D3D12_VIDEO_ENCODER_LEVELS_HEVC_61;
      } break;
      case 62:
      {
         return D3D12_VIDEO_ENCODER_LEVELS_HEVC_62;
      } break;
      default:
      {
         unreachable("Unsupported D3D12_VIDEO_ENCODER_LEVELS_HEVC value");
      } break;        
   }
}

uint8_t
d3d12_video_encoder_convert_12cusize_to_pixel_size_hevc(const D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC_CUSIZE& cuSize)
{
    switch(cuSize)
    {
        case D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC_CUSIZE_8x8:
        {
            return 8u;
        } break;
        case D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC_CUSIZE_16x16:
        {
            return 16u;
        } break;
        case D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC_CUSIZE_32x32:
        {
            return 32u;
        } break;
        case D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC_CUSIZE_64x64:
        {
            return 64u;
        } break;
        default:
        {
            unreachable(L"Not a supported cu size");
            return 0u;
        } break;            
    }
}

D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC_CUSIZE
d3d12_video_encoder_convert_pixel_size_hevc_to_12cusize(const uint32_t& cuSize)
{
    switch(cuSize)
    {
        case 8u:
        {
            return D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC_CUSIZE_8x8;
        } break;
        case 16u:
        {
            return D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC_CUSIZE_16x16;
        } break;
        case 32u:
        {
            return D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC_CUSIZE_32x32;
        } break;
        case 64u:
        {
            return D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC_CUSIZE_64x64;
        } break;
        default:
        {
            unreachable(L"Not a supported cu size");
        } break;            
    }
}

uint8_t 
d3d12_video_encoder_convert_12tusize_to_pixel_size_hevc(const D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC_TUSIZE& TUSize)
{
    switch(TUSize)
    {
        case D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC_TUSIZE_4x4:
        {
            return 4u;
        } break;
        case D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC_TUSIZE_8x8:
        {
            return 8u;
        } break;
        case D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC_TUSIZE_16x16:
        {
            return 16u;
        } break;
        case D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC_TUSIZE_32x32:
        {
            return 32u;
        } break;        
        default:
        {
            unreachable(L"Not a supported TU size");
        } break;            
    }
}

D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC_TUSIZE 
d3d12_video_encoder_convert_pixel_size_hevc_to_12tusize(const uint32_t& TUSize)
{
    switch(TUSize)
    {
        case 4u:
        {
            return D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC_TUSIZE_4x4;
        } break;
        case 8u:
        {
            return D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC_TUSIZE_8x8;
        } break;
        case 16u:
        {
            return D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC_TUSIZE_16x16;
        } break;
        case 32u:
        {
            return D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC_TUSIZE_32x32;
        } break;        
        default:
        {
            unreachable(L"Not a supported TU size");
        } break;            
    }
}

void
d3d12_video_bitstream_builder_hevc::build_vps(const D3D12_VIDEO_ENCODER_PROFILE_HEVC& profile,
         const D3D12_VIDEO_ENCODER_LEVEL_TIER_CONSTRAINTS_HEVC& level,
         const DXGI_FORMAT inputFmt,
         uint8_t maxRefFrames,
         bool gopHasBFrames,
         uint8_t vps_video_parameter_set_id,
         std::vector<BYTE> &headerBitstream,
         std::vector<BYTE>::iterator placingPositionStart,
         size_t &writtenBytes,
         HevcVideoParameterSet* pVPSStruct)
{
   uint8_t HEVCProfileIdc = convert_profile12_to_stdprofile(profile);
   uint32_t HEVCLevelIdc = 0u;
   d3d12_video_encoder_convert_from_d3d12_level_hevc(level.Level, HEVCLevelIdc);
   bool isHighTier = (level.Tier == D3D12_VIDEO_ENCODER_TIER_HEVC_HIGH);

   memset(&m_latest_vps, 0, sizeof(HevcVideoParameterSet));
   m_latest_vps.nalu = {
         // forbidden_zero_bit 
         0u,
         // nal_unit_type 
         HEVC_NALU_VPS_NUT,
         // nuh_layer_id 
         0u,
         // nuh_temporal_id_plus1 
         1u
      };

   m_latest_vps.vps_video_parameter_set_id = vps_video_parameter_set_id,
   m_latest_vps.vps_reserved_three_2bits = 3u;
   m_latest_vps.vps_max_layers_minus1 = 0u;
   m_latest_vps.vps_max_sub_layers_minus1 = 0u;
   m_latest_vps.vps_temporal_id_nesting_flag = 1u;
   m_latest_vps.vps_reserved_0xffff_16bits = 0xFFFF;
   init_profile_tier_level(&m_latest_vps.ptl, HEVCProfileIdc, HEVCLevelIdc, isHighTier);
   m_latest_vps.vps_sub_layer_ordering_info_present_flag = 0u;
   for (int i = (m_latest_vps.vps_sub_layer_ordering_info_present_flag ? 0 : m_latest_vps.vps_max_sub_layers_minus1); i <= m_latest_vps.vps_max_sub_layers_minus1; i++) {
      m_latest_vps.vps_max_dec_pic_buffering_minus1[i] = (maxRefFrames/*previous reference frames*/ + 1 /*additional current frame recon pic*/) - 1/**minus1 for header*/;        
      m_latest_vps.vps_max_num_reorder_pics[i] = gopHasBFrames ? m_latest_vps.vps_max_dec_pic_buffering_minus1[i] : 0;
      m_latest_vps.vps_max_latency_increase_plus1[i] = 0; // When vps_max_latency_increase_plus1[ i ] is equal to 0, no corresponding limit is expressed.
   }

   // Print built VPS structure
   debug_printf("[HEVCBitstreamBuilder] HevcVideoParameterSet Structure generated before writing to bitstream:\n");
   print_vps(m_latest_vps);

   m_hevcEncoder.vps_to_nalu_bytes(&m_latest_vps, headerBitstream, placingPositionStart, writtenBytes);

   if(pVPSStruct != nullptr)
   {
      *pVPSStruct = m_latest_vps;
   }
}

void
d3d12_video_bitstream_builder_hevc::build_sps(const HevcVideoParameterSet& parentVPS,
         const struct pipe_h265_enc_seq_param & seqData,
         uint8_t seq_parameter_set_id,
         const D3D12_VIDEO_ENCODER_PICTURE_RESOLUTION_DESC& encodeResolution,
         const D3D12_BOX& crop_window_upper_layer,
         const UINT picDimensionMultipleRequirement,
         const DXGI_FORMAT& inputFmt,
         const D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC& codecConfig,
         const D3D12_VIDEO_ENCODER_SEQUENCE_GOP_STRUCTURE_HEVC& hevcGOP,    
         std::vector<BYTE> &headerBitstream,
         std::vector<BYTE>::iterator placingPositionStart,
         size_t &writtenBytes,
         HevcSeqParameterSet* outputSPS)
{
   memset(&m_latest_sps, 0, sizeof(HevcSeqParameterSet));

   // In case is 420 10 bits
   if(inputFmt == DXGI_FORMAT_P010)
   {
      m_latest_sps.bit_depth_luma_minus8 = 2;
      m_latest_sps.bit_depth_chroma_minus8 = 2;
   }

   uint8_t minCuSize = d3d12_video_encoder_convert_12cusize_to_pixel_size_hevc(codecConfig.MinLumaCodingUnitSize);
   uint8_t maxCuSize = d3d12_video_encoder_convert_12cusize_to_pixel_size_hevc(codecConfig.MaxLumaCodingUnitSize);
   uint8_t minTuSize = d3d12_video_encoder_convert_12tusize_to_pixel_size_hevc(codecConfig.MinLumaTransformUnitSize);
   uint8_t maxTuSize = d3d12_video_encoder_convert_12tusize_to_pixel_size_hevc(codecConfig.MaxLumaTransformUnitSize);

   m_latest_sps.nalu.nal_unit_type = HEVC_NALU_SPS_NUT;
   m_latest_sps.nalu.nuh_temporal_id_plus1 = 1;

   m_latest_sps.sps_seq_parameter_set_id = seq_parameter_set_id;
   m_latest_sps.sps_max_sub_layers_minus1 = parentVPS.vps_max_sub_layers_minus1;
   m_latest_sps.sps_temporal_id_nesting_flag = parentVPS.vps_temporal_id_nesting_flag;

   // inherit PTL from parentVPS fully
   m_latest_sps.ptl = parentVPS.ptl;

   m_latest_sps.chroma_format_idc = 1; // 420

   // Codec spec dictates pic_width/height_in_luma_samples must be divisible by minCuSize but HW might have higher req pow 2 multiples
   assert((picDimensionMultipleRequirement % minCuSize) == 0u);

   // upper layer passes the viewport, can calculate the difference between it and pic_width_in_luma_samples
   D3D12_VIDEO_ENCODER_PICTURE_RESOLUTION_DESC viewport = { };
   viewport.Width = crop_window_upper_layer.front /* passes height */ - ((crop_window_upper_layer.left + crop_window_upper_layer.right) << 1);
   viewport.Height = crop_window_upper_layer.back /* passes width */- ((crop_window_upper_layer.top + crop_window_upper_layer.bottom) << 1);

   m_latest_sps.pic_width_in_luma_samples = ALIGN(encodeResolution.Width, picDimensionMultipleRequirement);
   m_latest_sps.pic_height_in_luma_samples = ALIGN(encodeResolution.Height, picDimensionMultipleRequirement);
   m_latest_sps.conf_win_right_offset = (m_latest_sps.pic_width_in_luma_samples - viewport.Width) >> 1;
   m_latest_sps.conf_win_bottom_offset = (m_latest_sps.pic_height_in_luma_samples - viewport.Height) >> 1;
   m_latest_sps.conformance_window_flag = m_latest_sps.conf_win_left_offset || m_latest_sps.conf_win_right_offset || m_latest_sps.conf_win_top_offset || m_latest_sps.conf_win_bottom_offset;

   m_latest_sps.log2_max_pic_order_cnt_lsb_minus4 = hevcGOP.log2_max_pic_order_cnt_lsb_minus4;
   m_latest_sps.maxPicOrderCntLsb = 1 << (m_latest_sps.log2_max_pic_order_cnt_lsb_minus4 + 4);

   m_latest_sps.sps_sub_layer_ordering_info_present_flag = parentVPS.vps_sub_layer_ordering_info_present_flag;
   for (int i = (m_latest_sps.sps_sub_layer_ordering_info_present_flag ? 0 : m_latest_sps.sps_max_sub_layers_minus1); i <= m_latest_sps.sps_max_sub_layers_minus1; i++) {
      m_latest_sps.sps_max_dec_pic_buffering_minus1[i] = parentVPS.vps_max_dec_pic_buffering_minus1[i];
      m_latest_sps.sps_max_num_reorder_pics[i] = parentVPS.vps_max_num_reorder_pics[i];
      m_latest_sps.sps_max_latency_increase_plus1[i] = parentVPS.vps_max_latency_increase_plus1[i];
   }

   m_latest_sps.log2_min_luma_coding_block_size_minus3 = static_cast<uint8_t>(std::log2(minCuSize) - 3);
   m_latest_sps.log2_diff_max_min_luma_coding_block_size = static_cast<uint8_t>(std::log2(maxCuSize) - std::log2(minCuSize));
   m_latest_sps.log2_min_transform_block_size_minus2 = static_cast<uint8_t>(std::log2(minTuSize) - 2);
   m_latest_sps.log2_diff_max_min_transform_block_size = static_cast<uint8_t>(std::log2(maxTuSize) - std::log2(minTuSize));

   m_latest_sps.max_transform_hierarchy_depth_inter = codecConfig.max_transform_hierarchy_depth_inter;
   m_latest_sps.max_transform_hierarchy_depth_intra = codecConfig.max_transform_hierarchy_depth_intra;

   m_latest_sps.scaling_list_enabled_flag = 0;
   m_latest_sps.amp_enabled_flag = ((codecConfig.ConfigurationFlags & D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC_FLAG_USE_ASYMETRIC_MOTION_PARTITION) != 0) ? 1u : 0u;
   m_latest_sps.sample_adaptive_offset_enabled_flag = ((codecConfig.ConfigurationFlags & D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC_FLAG_ENABLE_SAO_FILTER) != 0) ? 1u : 0u;
   m_latest_sps.pcm_enabled_flag = 0;

   m_latest_sps.num_short_term_ref_pic_sets = 0;

   m_latest_sps.long_term_ref_pics_present_flag = ((codecConfig.ConfigurationFlags & D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC_FLAG_ENABLE_LONG_TERM_REFERENCES) != 0) ? 1u : 0u;
   m_latest_sps.num_long_term_ref_pics_sps = 0; // signal through slice header for now

   m_latest_sps.sps_temporal_mvp_enabled_flag = 0;
   m_latest_sps.strong_intra_smoothing_enabled_flag = 0;

   m_latest_sps.vui_parameters_present_flag = seqData.vui_parameters_present_flag;
   m_latest_sps.vui.aspect_ratio_idc = seqData.aspect_ratio_idc;
   m_latest_sps.vui.sar_width = seqData.sar_width;
   m_latest_sps.vui.sar_height = seqData.sar_height;
   m_latest_sps.vui.video_format = seqData.video_format;
   m_latest_sps.vui.video_full_range_flag = seqData.video_full_range_flag;
   m_latest_sps.vui.colour_primaries = seqData.colour_primaries;
   m_latest_sps.vui.transfer_characteristics = seqData.transfer_characteristics;
   m_latest_sps.vui.matrix_coeffs = seqData.matrix_coefficients;
   m_latest_sps.vui.chroma_sample_loc_type_top_field = seqData.chroma_sample_loc_type_top_field;
   m_latest_sps.vui.chroma_sample_loc_type_bottom_field = seqData.chroma_sample_loc_type_bottom_field;
   m_latest_sps.vui.def_disp_win_left_offset = seqData.def_disp_win_left_offset;
   m_latest_sps.vui.def_disp_win_right_offset = seqData.def_disp_win_right_offset;
   m_latest_sps.vui.def_disp_win_top_offset = seqData.def_disp_win_top_offset;
   m_latest_sps.vui.def_disp_win_bottom_offset = seqData.def_disp_win_bottom_offset;
   m_latest_sps.vui.num_units_in_tick = seqData.num_units_in_tick;
   m_latest_sps.vui.time_scale = seqData.time_scale;
   m_latest_sps.vui.num_ticks_poc_diff_one_minus1 = seqData.num_ticks_poc_diff_one_minus1;
   m_latest_sps.vui.min_spatial_segmentation_idc = seqData.min_spatial_segmentation_idc;
   m_latest_sps.vui.max_bytes_per_pic_denom = seqData.max_bytes_per_pic_denom;
   m_latest_sps.vui.max_bits_per_min_cu_denom = seqData.max_bits_per_min_cu_denom;
   m_latest_sps.vui.log2_max_mv_length_horizontal = seqData.log2_max_mv_length_horizontal;
   m_latest_sps.vui.log2_max_mv_length_vertical = seqData.log2_max_mv_length_vertical;
   m_latest_sps.vui.aspect_ratio_info_present_flag = seqData.vui_flags.aspect_ratio_info_present_flag;
   m_latest_sps.vui.timing_info_present_flag = seqData.vui_flags.timing_info_present_flag;
   m_latest_sps.vui.video_signal_type_present_flag = seqData.vui_flags.video_signal_type_present_flag;
   m_latest_sps.vui.colour_description_present_flag = seqData.vui_flags.colour_description_present_flag;
   m_latest_sps.vui.chroma_loc_info_present_flag = seqData.vui_flags.chroma_loc_info_present_flag;
   m_latest_sps.vui.overscan_info_present_flag = seqData.vui_flags.overscan_info_present_flag;
   m_latest_sps.vui.overscan_appropriate_flag = seqData.vui_flags.overscan_appropriate_flag;
   m_latest_sps.vui.neutral_chroma_indication_flag = seqData.vui_flags.neutral_chroma_indication_flag;
   m_latest_sps.vui.field_seq_flag = seqData.vui_flags.field_seq_flag;
   m_latest_sps.vui.frame_field_info_present_flag = seqData.vui_flags.frame_field_info_present_flag;
   m_latest_sps.vui.default_display_window_flag = seqData.vui_flags.default_display_window_flag;
   m_latest_sps.vui.poc_proportional_to_timing_flag = seqData.vui_flags.poc_proportional_to_timing_flag;
   m_latest_sps.vui.hrd_parameters_present_flag = seqData.vui_flags.hrd_parameters_present_flag;
   m_latest_sps.vui.bitstream_restriction_flag = seqData.vui_flags.bitstream_restriction_flag;
   m_latest_sps.vui.tiles_fixed_structure_flag = seqData.vui_flags.tiles_fixed_structure_flag;
   m_latest_sps.vui.motion_vectors_over_pic_boundaries_flag = seqData.vui_flags.motion_vectors_over_pic_boundaries_flag;
   m_latest_sps.vui.restricted_ref_pic_lists_flag = seqData.vui_flags.restricted_ref_pic_lists_flag;

   m_latest_sps.sps_extension_flag = 0;

   // Print built SPS structure
   debug_printf("[HEVCBitstreamBuilder] HevcSeqParameterSet Structure generated before writing to bitstream:\n");
   print_sps(m_latest_sps);
   
   m_hevcEncoder.sps_to_nalu_bytes(&m_latest_sps, headerBitstream, placingPositionStart, writtenBytes);

   if(outputSPS != nullptr)
   {
      *outputSPS = m_latest_sps;
   }
}

void
d3d12_video_bitstream_builder_hevc::build_pps(const HevcSeqParameterSet& parentSPS,
         uint8_t pic_parameter_set_id,
         const D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC& codecConfig,
         const D3D12_VIDEO_ENCODER_PICTURE_CONTROL_CODEC_DATA_HEVC& pictureControl,
         std::vector<BYTE> &headerBitstream,
         std::vector<BYTE>::iterator placingPositionStart,
         size_t &writtenBytes,
         HevcPicParameterSet* outputPPS)
{
   memset(&m_latest_pps, 0, sizeof(HevcPicParameterSet));

   m_latest_pps.nalu.nal_unit_type = HEVC_NALU_PPS_NUT;
   m_latest_pps.nalu.nuh_temporal_id_plus1 = 1;

   m_latest_pps.pps_pic_parameter_set_id = pic_parameter_set_id;
   m_latest_pps.pps_seq_parameter_set_id = parentSPS.sps_seq_parameter_set_id;

   m_latest_pps.weighted_pred_flag = 0u; // no weighted prediction in D3D12

   m_latest_pps.num_ref_idx_lx_default_active_minus1[0] = static_cast<uint8_t>(std::max(static_cast<INT>(pictureControl.List0ReferenceFramesCount) - 1, 0));
   m_latest_pps.num_ref_idx_lx_default_active_minus1[1] = static_cast<uint8_t>(std::max(static_cast<INT>(pictureControl.List1ReferenceFramesCount) - 1, 0));

   m_latest_pps.num_tile_columns_minus1 = 0u; // no tiling in D3D12
   m_latest_pps.num_tile_rows_minus1 = 0u; // no tiling in D3D12
   m_latest_pps.tiles_enabled_flag = 0u; // no tiling in D3D12
   m_latest_pps.loop_filter_across_tiles_enabled_flag = 0;

   m_latest_pps.lists_modification_present_flag = 0;
   m_latest_pps.log2_parallel_merge_level_minus2 = 0;

   m_latest_pps.deblocking_filter_control_present_flag = 1;
   m_latest_pps.deblocking_filter_override_enabled_flag = 0;
   m_latest_pps.pps_deblocking_filter_disabled_flag = 0;
   m_latest_pps.pps_scaling_list_data_present_flag = 0;
   m_latest_pps.pps_beta_offset_div2 = 0;
   m_latest_pps.pps_tc_offset_div2 = 0;
   m_latest_pps.pps_loop_filter_across_slices_enabled_flag = ((codecConfig.ConfigurationFlags & D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC_FLAG_DISABLE_LOOP_FILTER_ACROSS_SLICES) != 0) ? 0 : 1;
   m_latest_pps.transform_skip_enabled_flag = ((codecConfig.ConfigurationFlags & D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC_FLAG_ENABLE_TRANSFORM_SKIPPING) != 0) ? 1 : 0;
   m_latest_pps.constrained_intra_pred_flag = ((codecConfig.ConfigurationFlags & D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC_FLAG_USE_CONSTRAINED_INTRAPREDICTION) != 0) ? 1 : 0;
   m_latest_pps.cabac_init_present_flag = 1;
   m_latest_pps.pps_slice_chroma_qp_offsets_present_flag = 1;
   m_latest_pps.cu_qp_delta_enabled_flag = 1;

   // Print built PPS structure
   debug_printf("[HEVCBitstreamBuilder] HevcPicParameterSet Structure generated before writing to bitstream:\n");
   print_pps(m_latest_pps);

   m_hevcEncoder.pps_to_nalu_bytes(&m_latest_pps, headerBitstream, placingPositionStart, writtenBytes);

   if(outputPPS != nullptr)
   {
      *outputPPS = m_latest_pps;
   }
}

void
d3d12_video_bitstream_builder_hevc::write_end_of_stream_nalu(std::vector<uint8_t> &         headerBitstream,
                        std::vector<uint8_t>::iterator placingPositionStart,
                        size_t &                       writtenBytes)
{
   m_hevcEncoder.write_end_of_stream_nalu(headerBitstream, placingPositionStart, writtenBytes);
}
void
d3d12_video_bitstream_builder_hevc::write_end_of_sequence_nalu(std::vector<uint8_t> &         headerBitstream,
                           std::vector<uint8_t>::iterator placingPositionStart,
                           size_t &                       writtenBytes)
{
   m_hevcEncoder.write_end_of_sequence_nalu(headerBitstream, placingPositionStart, writtenBytes);
}

void
d3d12_video_bitstream_builder_hevc::print_vps(const HevcVideoParameterSet& VPS)
{
   debug_printf("--------------------------------------\nHevcVideoParameterSet values below:\n");

   debug_printf("vps_video_parameter_set_id: %d\n", VPS.vps_video_parameter_set_id);
   debug_printf("vps_reserved_three_2bits: %d\n", VPS.vps_reserved_three_2bits);
   debug_printf("vps_max_layers_minus1: %d\n", VPS.vps_max_layers_minus1);
   debug_printf("vps_max_sub_layers_minus1: %d\n", VPS.vps_max_sub_layers_minus1);
   debug_printf("vps_temporal_id_nesting_flag: %d\n", VPS.vps_temporal_id_nesting_flag);

   debug_printf("general_profile_space: %d\n", VPS.ptl.general_profile_space);
   debug_printf("general_tier_flag: %d\n", VPS.ptl.general_tier_flag);
   debug_printf("general_profile_idc: %d\n", VPS.ptl.general_profile_idc);
   debug_printf("general_progressive_source_flag: %d\n", VPS.ptl.general_progressive_source_flag);
   debug_printf("general_interlaced_source_flag: %d\n", VPS.ptl.general_interlaced_source_flag);
   debug_printf("general_non_packed_constraint_flag: %d\n", VPS.ptl.general_non_packed_constraint_flag);
   debug_printf("general_frame_only_constraint_flag: %d\n", VPS.ptl.general_frame_only_constraint_flag);
   debug_printf("general_level_idc: %d\n", VPS.ptl.general_level_idc);

   debug_printf("vps_sub_layer_ordering_info_present_flag: %d\n", VPS.vps_sub_layer_ordering_info_present_flag);
   debug_printf("vps_max_dec_pic_buffering_minus1[%d]: %d\n", (0u), VPS.vps_max_dec_pic_buffering_minus1[0u]);
   debug_printf("vps_max_num_reorder_pics[%d]: %d\n", (0u), VPS.vps_max_num_reorder_pics[0u]);
   debug_printf("vps_max_latency_increase_plus1[%d]: %d\n", (0u), VPS.vps_max_latency_increase_plus1[0u]);
   debug_printf("vps_max_layer_id: %d\n", VPS.vps_max_layer_id);
   debug_printf("vps_num_layer_sets_minus1: %d\n", VPS.vps_num_layer_sets_minus1);
   debug_printf("vps_timing_info_present_flag: %d\n", VPS.vps_timing_info_present_flag);
   debug_printf("vps_num_units_in_tick: %d\n", VPS.vps_num_units_in_tick);
   debug_printf("vps_time_scale: %d\n", VPS.vps_time_scale);
   debug_printf("vps_poc_proportional_to_timing_flag: %d\n", VPS.vps_poc_proportional_to_timing_flag);
   debug_printf("vps_num_ticks_poc_diff_one_minus1: %d\n", VPS.vps_num_ticks_poc_diff_one_minus1);
   debug_printf("vps_num_hrd_parameters: %d\n", VPS.vps_num_hrd_parameters);
   debug_printf("vps_extension_flag: %d\n", VPS.vps_extension_flag);
   debug_printf("vps_extension_data_flag: %d\n", VPS.vps_extension_data_flag);

   debug_printf("HevcVideoParameterSet values end\n--------------------------------------\n");
}
void
d3d12_video_bitstream_builder_hevc::print_sps(const HevcSeqParameterSet& SPS)
{
   debug_printf("--------------------------------------\nHevcSeqParameterSet values below:\n");

   debug_printf("sps_video_parameter_set_id: %d\n", SPS.sps_video_parameter_set_id);
   debug_printf("sps_max_sub_layers_minus1: %d\n", SPS.sps_max_sub_layers_minus1);
   debug_printf("sps_temporal_id_nesting_flag: %d\n", SPS.sps_temporal_id_nesting_flag);
   
   debug_printf("general_profile_space: %d\n", SPS.ptl.general_profile_space);
   debug_printf("general_tier_flag: %d\n", SPS.ptl.general_tier_flag);
   debug_printf("general_profile_idc: %d\n", SPS.ptl.general_profile_idc);
   debug_printf("general_progressive_source_flag: %d\n", SPS.ptl.general_progressive_source_flag);
   debug_printf("general_interlaced_source_flag: %d\n", SPS.ptl.general_interlaced_source_flag);
   debug_printf("general_non_packed_constraint_flag: %d\n", SPS.ptl.general_non_packed_constraint_flag);
   debug_printf("general_frame_only_constraint_flag: %d\n", SPS.ptl.general_frame_only_constraint_flag);
   debug_printf("general_level_idc: %d\n", SPS.ptl.general_level_idc);

   debug_printf("sps_seq_parameter_set_id: %d\n", SPS.sps_seq_parameter_set_id);
   debug_printf("chroma_format_idc: %d\n", SPS.chroma_format_idc);
   debug_printf("separate_colour_plane_flag: %d\n", SPS.separate_colour_plane_flag);
   debug_printf("pic_width_in_luma_samples: %d\n", SPS.pic_width_in_luma_samples);
   debug_printf("pic_height_in_luma_samples: %d\n", SPS.pic_height_in_luma_samples);
   debug_printf("conformance_window_flag: %d\n", SPS.conformance_window_flag);
   debug_printf("conf_win_left_offset: %d\n", SPS.conf_win_left_offset);
   debug_printf("conf_win_right_offset: %d\n", SPS.conf_win_right_offset);
   debug_printf("conf_win_top_offset: %d\n", SPS.conf_win_top_offset);
   debug_printf("conf_win_bottom_offset: %d\n", SPS.conf_win_bottom_offset);
   debug_printf("bit_depth_luma_minus8: %d\n", SPS.bit_depth_luma_minus8);
   debug_printf("bit_depth_chroma_minus8: %d\n", SPS.bit_depth_chroma_minus8);
   debug_printf("log2_max_pic_order_cnt_lsb_minus4: %d\n", SPS.log2_max_pic_order_cnt_lsb_minus4);
   debug_printf("maxPicOrderCntLsb: %d\n", SPS.maxPicOrderCntLsb);
   debug_printf("sps_sub_layer_ordering_info_present_flag: %d\n", SPS.sps_sub_layer_ordering_info_present_flag);

   debug_printf("sps_max_dec_pic_buffering_minus1[%d]: %d\n", (0u), SPS.sps_max_dec_pic_buffering_minus1[0u]);
   debug_printf("sps_max_num_reorder_pics[%d]: %d\n", (0u), SPS.sps_max_num_reorder_pics[0u]);
   debug_printf("sps_max_latency_increase_plus1[%d]: %d\n", (0u), SPS.sps_max_latency_increase_plus1[0u]);
   
   debug_printf("log2_min_luma_coding_block_size_minus3: %d\n", SPS.log2_min_luma_coding_block_size_minus3);
   debug_printf("log2_diff_max_min_luma_coding_block_size: %d\n", SPS.log2_diff_max_min_luma_coding_block_size);
   debug_printf("log2_min_transform_block_size_minus2: %d\n", SPS.log2_min_transform_block_size_minus2);
   debug_printf("log2_diff_max_min_transform_block_size: %d\n", SPS.log2_diff_max_min_transform_block_size);
   debug_printf("max_transform_hierarchy_depth_inter: %d\n", SPS.max_transform_hierarchy_depth_inter);
   debug_printf("max_transform_hierarchy_depth_intra: %d\n", SPS.max_transform_hierarchy_depth_intra);
   debug_printf("scaling_list_enabled_flag: %d\n", SPS.scaling_list_enabled_flag);
   debug_printf("sps_scaling_list_data_present_flag: %d\n", SPS.sps_scaling_list_data_present_flag);
   debug_printf("amp_enabled_flag: %d\n", SPS.amp_enabled_flag);
   debug_printf("sample_adaptive_offset_enabled_flag: %d\n", SPS.sample_adaptive_offset_enabled_flag);
   debug_printf("pcm_enabled_flag: %d\n", SPS.pcm_enabled_flag);
   debug_printf("pcm_sample_bit_depth_luma_minus1: %d\n", SPS.pcm_sample_bit_depth_luma_minus1);
   debug_printf("pcm_sample_bit_depth_chroma_minus1: %d\n", SPS.pcm_sample_bit_depth_chroma_minus1);
   debug_printf("log2_min_pcm_luma_coding_block_size_minus3: %d\n", SPS.log2_min_pcm_luma_coding_block_size_minus3);
   debug_printf("log2_diff_max_min_pcm_luma_coding_block_size: %d\n", SPS.log2_diff_max_min_pcm_luma_coding_block_size);
   debug_printf("pcm_loop_filter_disabled_flag: %d\n", SPS.pcm_loop_filter_disabled_flag);
   debug_printf("num_short_term_ref_pic_sets: %d\n", SPS.num_short_term_ref_pic_sets);
   
   for(UINT idx = 0; idx < SPS.num_short_term_ref_pic_sets ; idx++)
   {        
      print_rps(&SPS, idx);
   }
   
   debug_printf("long_term_ref_pics_present_flag: %d\n", SPS.long_term_ref_pics_present_flag);
   debug_printf("num_long_term_ref_pics_sps: %d\n", SPS.num_long_term_ref_pics_sps);
   
   for(UINT idx = 0; idx < SPS.num_long_term_ref_pics_sps ; idx++)
   {
      debug_printf("lt_ref_pic_poc_lsb_sps[%d]: %d\n", idx, SPS.lt_ref_pic_poc_lsb_sps[idx]);
      debug_printf("used_by_curr_pic_lt_sps_flag[%d]: %d\n", idx, SPS.used_by_curr_pic_lt_sps_flag[idx]);
   }
   
   debug_printf("sps_temporal_mvp_enabled_flag: %d\n", SPS.sps_temporal_mvp_enabled_flag);
   debug_printf("strong_intra_smoothing_enabled_flag: %d\n", SPS.strong_intra_smoothing_enabled_flag);
   debug_printf("vui_parameters_present_flag: %d\n", SPS.vui_parameters_present_flag);
   debug_printf("aspect_ratio_info_present_flag: %d\n", SPS.vui.aspect_ratio_info_present_flag);
   debug_printf("aspect_ratio_idc: %d\n", SPS.vui.aspect_ratio_idc);
   debug_printf("sar_width: %d\n", SPS.vui.sar_width);
   debug_printf("sar_height: %d\n", SPS.vui.sar_height);
   debug_printf("overscan_info_present_flag: %d\n", SPS.vui.overscan_info_present_flag);
   debug_printf("overscan_appropriate_flag: %d\n", SPS.vui.overscan_appropriate_flag);
   debug_printf("video_signal_type_present_flag: %d\n", SPS.vui.video_signal_type_present_flag);
   debug_printf("video_format: %d\n", SPS.vui.video_format);
   debug_printf("video_full_range_flag: %d\n", SPS.vui.video_full_range_flag);
   debug_printf("colour_description_present_flag: %d\n", SPS.vui.colour_description_present_flag);
   debug_printf("colour_primaries: %d\n", SPS.vui.colour_primaries);
   debug_printf("transfer_characteristics: %d\n", SPS.vui.transfer_characteristics);
   debug_printf("matrix_coeffs: %d\n", SPS.vui.matrix_coeffs);
   debug_printf("chroma_loc_info_present_flag: %d\n", SPS.vui.chroma_loc_info_present_flag);
   debug_printf("chroma_sample_loc_type_top_field: %d\n", SPS.vui.chroma_sample_loc_type_top_field);
   debug_printf("chroma_sample_loc_type_bottom_field: %d\n", SPS.vui.chroma_sample_loc_type_bottom_field);
   debug_printf("neutral_chroma_indication_flag: %d\n", SPS.vui.neutral_chroma_indication_flag);
   debug_printf("field_seq_flag: %d\n", SPS.vui.field_seq_flag);
   debug_printf("frame_field_info_present_flag: %d\n", SPS.vui.frame_field_info_present_flag);
   debug_printf("default_display_window_flag: %d\n", SPS.vui.default_display_window_flag);
   debug_printf("def_disp_win_left_offset: %d\n", SPS.vui.def_disp_win_left_offset);
   debug_printf("def_disp_win_right_offset: %d\n", SPS.vui.def_disp_win_right_offset);
   debug_printf("def_disp_win_top_offset: %d\n", SPS.vui.def_disp_win_top_offset);
   debug_printf("def_disp_win_bottom_offset: %d\n", SPS.vui.def_disp_win_bottom_offset);
   debug_printf("timing_info_present_flag: %d\n", SPS.vui.timing_info_present_flag);
   debug_printf("num_units_in_tick: %d\n", SPS.vui.num_units_in_tick);
   debug_printf("time_scale: %d\n", SPS.vui.time_scale);
   debug_printf("poc_proportional_to_timing_flag: %d\n", SPS.vui.poc_proportional_to_timing_flag);
   debug_printf("num_ticks_poc_diff_one_minus1: %d\n", SPS.vui.num_ticks_poc_diff_one_minus1);
   debug_printf("hrd_parameters_present_flag: %d\n", SPS.vui.hrd_parameters_present_flag);
   debug_printf("bitstream_restriction_flag: %d\n", SPS.vui.bitstream_restriction_flag);
   debug_printf("tiles_fixed_structure_flag: %d\n", SPS.vui.tiles_fixed_structure_flag);
   debug_printf("motion_vectors_over_pic_boundaries_flag: %d\n", SPS.vui.motion_vectors_over_pic_boundaries_flag);
   debug_printf("restricted_ref_pic_lists_flag: %d\n", SPS.vui.restricted_ref_pic_lists_flag);
   debug_printf("min_spatial_segmentation_idc: %d\n", SPS.vui.min_spatial_segmentation_idc);
   debug_printf("max_bytes_per_pic_denom: %d\n", SPS.vui.max_bytes_per_pic_denom);
   debug_printf("max_bits_per_min_cu_denom: %d\n", SPS.vui.max_bits_per_min_cu_denom);
   debug_printf("log2_max_mv_length_horizontal: %d\n", SPS.vui.log2_max_mv_length_horizontal);
   debug_printf("log2_max_mv_length_vertical: %d\n", SPS.vui.log2_max_mv_length_vertical);
   debug_printf("sps_extension_flag: %d\n", SPS.sps_extension_flag);
   debug_printf("sps_extension_data_flag: %d\n", SPS.sps_extension_data_flag);

   debug_printf("HevcSeqParameterSet values end\n--------------------------------------\n");
}
void
d3d12_video_bitstream_builder_hevc::print_pps(const HevcPicParameterSet& PPS)
{
   debug_printf("--------------------------------------\nHevcPicParameterSet values below:\n");
   debug_printf("pps_pic_parameter_set_id: %d\n", PPS.pps_pic_parameter_set_id);
   debug_printf("pps_seq_parameter_set_id: %d\n", PPS.pps_seq_parameter_set_id);
   debug_printf("dependent_slice_segments_enabled_flag: %d\n", PPS.dependent_slice_segments_enabled_flag);
   debug_printf("output_flag_present_flag: %d\n", PPS.output_flag_present_flag);
   debug_printf("num_extra_slice_header_bits: %d\n", PPS.num_extra_slice_header_bits);
   debug_printf("sign_data_hiding_enabled_flag: %d\n", PPS.sign_data_hiding_enabled_flag);
   debug_printf("cabac_init_present_flag: %d\n", PPS.cabac_init_present_flag);
   debug_printf("num_ref_idx_l0_default_active_minus1: %d\n", PPS.num_ref_idx_lx_default_active_minus1[0]);
   debug_printf("num_ref_idx_l1_default_active_minus1: %d\n", PPS.num_ref_idx_lx_default_active_minus1[1]);
   debug_printf("init_qp_minus26: %d\n", PPS.init_qp_minus26);
   debug_printf("constrained_intra_pred_flag: %d\n", PPS.constrained_intra_pred_flag);
   debug_printf("transform_skip_enabled_flag: %d\n", PPS.transform_skip_enabled_flag);
   debug_printf("cu_qp_delta_enabled_flag: %d\n", PPS.cu_qp_delta_enabled_flag);
   debug_printf("diff_cu_qp_delta_depth: %d\n", PPS.diff_cu_qp_delta_depth);
   debug_printf("pps_cb_qp_offset: %d\n", PPS.pps_cb_qp_offset);
   debug_printf("pps_cr_qp_offset: %d\n", PPS.pps_cr_qp_offset);
   debug_printf("pps_slice_chroma_qp_offsets_present_flag: %d\n", PPS.pps_slice_chroma_qp_offsets_present_flag);
   debug_printf("weighted_pred_flag: %d\n", PPS.weighted_pred_flag);
   debug_printf("weighted_bipred_flag: %d\n", PPS.weighted_bipred_flag);
   debug_printf("transquant_bypass_enabled_flag: %d\n", PPS.transquant_bypass_enabled_flag);
   debug_printf("tiles_enabled_flag: %d\n", PPS.tiles_enabled_flag);
   debug_printf("entropy_coding_sync_enabled_flag: %d\n", PPS.entropy_coding_sync_enabled_flag);
   debug_printf("num_tile_columns_minus1: %d\n", PPS.num_tile_columns_minus1);
   debug_printf("num_tile_rows_minus1: %d\n", PPS.num_tile_rows_minus1);
   debug_printf("uniform_spacing_flag: %d\n", PPS.uniform_spacing_flag);
   debug_printf("column_width_minus1[0]: %d\n", PPS.column_width_minus1[0]); // no tiles in D3D12)
   debug_printf("row_height_minus1[0]: %d\n", PPS.row_height_minus1[0]); // no tiles in D3D12)
   debug_printf("loop_filter_across_tiles_enabled_flag: %d\n", PPS.loop_filter_across_tiles_enabled_flag);
   debug_printf("pps_loop_filter_across_slices_enabled_flag: %d\n", PPS.pps_loop_filter_across_slices_enabled_flag);
   debug_printf("deblocking_filter_control_present_flag: %d\n", PPS.deblocking_filter_control_present_flag);
   debug_printf("deblocking_filter_override_enabled_flag: %d\n", PPS.deblocking_filter_override_enabled_flag);
   debug_printf("pps_deblocking_filter_disabled_flag: %d\n", PPS.pps_deblocking_filter_disabled_flag);
   debug_printf("pps_beta_offset_div2: %d\n", PPS.pps_beta_offset_div2);
   debug_printf("pps_tc_offset_div2: %d\n", PPS.pps_tc_offset_div2);
   debug_printf("pps_scaling_list_data_present_flag: %d\n", PPS.pps_scaling_list_data_present_flag);
   debug_printf("lists_modification_present_flag: %d\n", PPS.lists_modification_present_flag);
   debug_printf("log2_parallel_merge_level_minus2: %d\n", PPS.log2_parallel_merge_level_minus2);
   debug_printf("slice_segment_header_extension_present_flag: %d\n", PPS.slice_segment_header_extension_present_flag);
   debug_printf("pps_extension_flag: %d\n", PPS.pps_extension_flag);
   debug_printf("pps_extension_data_flag: %d\n", PPS.pps_extension_data_flag);
   debug_printf("HevcPicParameterSet values end\n--------------------------------------\n");   
}

void
d3d12_video_bitstream_builder_hevc::print_rps(const HevcSeqParameterSet* sps, UINT stRpsIdx)
{
   const HEVCReferencePictureSet* rps = &(sps->rpsShortTerm[stRpsIdx]);

   debug_printf("--------------------------------------\nHEVCReferencePictureSet[%d] values below:\n", stRpsIdx);

   debug_printf("inter_ref_pic_set_prediction_flag: %d\n", rps->inter_ref_pic_set_prediction_flag);

   if(rps->inter_ref_pic_set_prediction_flag)
   {
      debug_printf("delta_idx_minus1: %d\n", rps->delta_idx_minus1);                
      debug_printf("delta_rps_sign: %d\n", rps->delta_rps_sign);
      debug_printf("abs_delta_rps_minus1: %d\n", rps->abs_delta_rps_minus1);
      debug_printf("num_negative_pics: %d\n", rps->num_negative_pics);        
      debug_printf("num_positive_pics: %d\n", rps->num_positive_pics);

      int32_t RefRpsIdx = stRpsIdx - 1 - rps->delta_idx_minus1;
      const HEVCReferencePictureSet* rpsRef = &(sps->rpsShortTerm[RefRpsIdx]);
      auto numberOfPictures = rpsRef->num_negative_pics + rpsRef->num_positive_pics;
      for (uint8_t j = 0; j <= numberOfPictures; j++) {
         debug_printf("used_by_curr_pic_flag[%d]: %d\n", j, rps->used_by_curr_pic_flag[j]);
         if (!rps->used_by_curr_pic_flag[j]) {
               debug_printf("use_delta_flag[%d]: %d\n", j, rps->use_delta_flag[j]);
         }
      }
   }
   else
   {
      debug_printf("num_negative_pics: %d\n", rps->num_negative_pics);        
      for (uint8_t i = 0; i < rps->num_negative_pics; i++) {
         debug_printf("delta_poc_s0_minus1[%d]: %d\n", i, rps->delta_poc_s0_minus1[i]);
         debug_printf("used_by_curr_pic_s0_flag[%d]: %d\n", i, rps->used_by_curr_pic_s0_flag[i]);
      }

      debug_printf("num_positive_pics: %d\n", rps->num_positive_pics);
      for (int32_t i = 0; i < rps->num_positive_pics; i++) {
         debug_printf("delta_poc_s1_minus1[%d]: %d\n", i, rps->delta_poc_s1_minus1[i]);
         debug_printf("used_by_curr_pic_s1_flag[%d]: %d\n", i, rps->used_by_curr_pic_s1_flag[i]);
      }
   }

   debug_printf("HEVCReferencePictureSet values end\n--------------------------------------\n");
}
