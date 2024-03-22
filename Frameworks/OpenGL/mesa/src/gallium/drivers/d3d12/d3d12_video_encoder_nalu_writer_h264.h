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

#ifndef D3D12_VIDEO_ENC_NALU_WRITER_H264_H
#define D3D12_VIDEO_ENC_NALU_WRITER_H264_H

#include "d3d12_video_encoder_bitstream.h"

enum H264_NALREF_IDC
{
   NAL_REFIDC_REF    = 3,
   NAL_REFIDC_NONREF = 0
};

enum H264_NALU_TYPE
{
   NAL_TYPE_UNSPECIFIED           = 0,
   NAL_TYPE_SLICE                 = 1,
   NAL_TYPE_SLICEDATA_A           = 2,
   NAL_TYPE_SLICEDATA_B           = 3,
   NAL_TYPE_SLICEDATA_C           = 4,
   NAL_TYPE_IDR                   = 5,
   NAL_TYPE_SEI                   = 6,
   NAL_TYPE_SPS                   = 7,
   NAL_TYPE_PPS                   = 8,
   NAL_TYPE_ACCESS_UNIT_DELIMITER = 9,
   NAL_TYPE_END_OF_SEQUENCE       = 10,
   NAL_TYPE_END_OF_STREAM         = 11,
   NAL_TYPE_FILLER_DATA           = 12,
   NAL_TYPE_SPS_EXTENSION         = 13,
   NAL_TYPE_PREFIX                = 14,
   /* 15...18 RESERVED */
   NAL_TYPE_AUXILIARY_SLICE = 19,
   /* 20...23 RESERVED */
   /* 24...31 UNSPECIFIED */
};

typedef struct H264_HRD_PARAMS
{
   uint32_t cpb_cnt_minus1;
   uint32_t bit_rate_scale;
   uint32_t cpb_size_scale;
   uint32_t bit_rate_value_minus1[32];
   uint32_t cpb_size_value_minus1[32];
   uint32_t cbr_flag[32];
   uint32_t initial_cpb_removal_delay_length_minus1;
   uint32_t cpb_removal_delay_length_minus1;
   uint32_t dpb_output_delay_length_minus1;
   uint32_t time_offset_length;
} H264_HRD_PARAMS;

struct H264_VUI_PARAMS
{
   uint32_t aspect_ratio_info_present_flag;
   uint32_t aspect_ratio_idc;
   uint32_t sar_width;
   uint32_t sar_height;
   uint32_t overscan_info_present_flag;
   uint32_t overscan_appropriate_flag;
   uint32_t video_signal_type_present_flag;
   uint32_t video_format;
   uint32_t video_full_range_flag;
   uint32_t colour_description_present_flag;
   uint32_t colour_primaries;
   uint32_t transfer_characteristics;
   uint32_t matrix_coefficients;
   uint32_t chroma_loc_info_present_flag;
   uint32_t chroma_sample_loc_type_top_field;
   uint32_t chroma_sample_loc_type_bottom_field;
   uint32_t timing_info_present_flag;
   uint32_t time_scale;
   uint32_t num_units_in_tick;
   uint32_t fixed_frame_rate_flag;
   uint32_t nal_hrd_parameters_present_flag;
   H264_HRD_PARAMS nal_hrd_parameters;
   uint32_t vcl_hrd_parameters_present_flag;
   H264_HRD_PARAMS vcl_hrd_parameters;
   uint32_t low_delay_hrd_flag;
   uint32_t pic_struct_present_flag;
   uint32_t bitstream_restriction_flag;
   uint32_t motion_vectors_over_pic_boundaries_flag;
   uint32_t max_bytes_per_pic_denom;
   uint32_t max_bits_per_mb_denom;
   uint32_t log2_max_mv_length_vertical;
   uint32_t log2_max_mv_length_horizontal;
   uint32_t num_reorder_frames;
   uint32_t max_dec_frame_buffering;
};

struct H264_SPS
{
   uint32_t profile_idc;
   uint32_t constraint_set_flags;
   uint32_t level_idc;
   uint32_t seq_parameter_set_id;
   uint32_t bit_depth_luma_minus8;
   uint32_t bit_depth_chroma_minus8;
   uint32_t log2_max_frame_num_minus4;
   uint32_t pic_order_cnt_type;
   uint32_t log2_max_pic_order_cnt_lsb_minus4;
   uint32_t max_num_ref_frames;
   uint32_t gaps_in_frame_num_value_allowed_flag;
   uint32_t pic_width_in_mbs_minus1;
   uint32_t pic_height_in_map_units_minus1;
   uint32_t direct_8x8_inference_flag;
   uint32_t frame_cropping_flag;
   uint32_t frame_cropping_rect_left_offset;
   uint32_t frame_cropping_rect_right_offset;
   uint32_t frame_cropping_rect_top_offset;
   uint32_t frame_cropping_rect_bottom_offset;
   uint32_t vui_parameters_present_flag;
   H264_VUI_PARAMS vui;
};

struct H264_PPS
{
   uint32_t pic_parameter_set_id;
   uint32_t seq_parameter_set_id;
   uint32_t entropy_coding_mode_flag;
   uint32_t pic_order_present_flag;
   uint32_t num_ref_idx_l0_active_minus1;
   uint32_t num_ref_idx_l1_active_minus1;
   uint32_t constrained_intra_pred_flag;
   uint32_t transform_8x8_mode_flag;
};

enum H264_SPEC_PROFILES
{
   // Same as BASELINE (66) with constraint_set1_flag set
   H264_PROFILE_CONSTRAINED_BASELINE = 66,
   H264_PROFILE_BASELINE   = 66,
   H264_PROFILE_MAIN   = 77,
   H264_PROFILE_HIGH   = 100,
   H264_PROFILE_HIGH10 = 110,
};

#define MAX_COMPRESSED_PPS 256
#define MAX_COMPRESSED_SPS 256

class d3d12_video_nalu_writer_h264
{
 public:
   d3d12_video_nalu_writer_h264()
   { }
   ~d3d12_video_nalu_writer_h264()
   { }

   // Writes the H264 SPS structure into a bitstream passed in headerBitstream
   // Function resizes bitstream accordingly and puts result in byte vector
   void sps_to_nalu_bytes(H264_SPS *                     pSPS,
                          std::vector<uint8_t> &         headerBitstream,
                          std::vector<uint8_t>::iterator placingPositionStart,
                          size_t &                       writtenBytes);

   // Writes the H264 PPS structure into a bitstream passed in headerBitstream
   // Function resizes bitstream accordingly and puts result in byte vector
   void pps_to_nalu_bytes(H264_PPS *                     pPPS,
                          std::vector<uint8_t> &         headerBitstream,
                          BOOL                           bIsFREXTProfile,
                          std::vector<uint8_t>::iterator placingPositionStart,
                          size_t &                       writtenBytes);

   void write_end_of_stream_nalu(std::vector<uint8_t> &         headerBitstream,
                                 std::vector<uint8_t>::iterator placingPositionStart,
                                 size_t &                       writtenBytes);
   void write_end_of_sequence_nalu(std::vector<uint8_t> &         headerBitstream,
                                   std::vector<uint8_t>::iterator placingPositionStart,
                                   size_t &                       writtenBytes);

   void write_access_unit_delimiter_nalu(std::vector<uint8_t> &         headerBitstream,
                                         std::vector<uint8_t>::iterator placingPositionStart,
                                         size_t &                       writtenBytes);

 private:
   // Writes from structure into bitstream with RBSP trailing but WITHOUT NAL unit wrap (eg. nal_idc_type, etc)
   uint32_t write_sps_bytes(d3d12_video_encoder_bitstream *pBitstream, H264_SPS *pSPS);
   uint32_t write_pps_bytes(d3d12_video_encoder_bitstream *pBitstream, H264_PPS *pPPS, BOOL bIsFREXTProfile);

   // Adds NALU wrapping into structures and ending NALU control bits
   uint32_t wrap_sps_nalu(d3d12_video_encoder_bitstream *pNALU, d3d12_video_encoder_bitstream *pRBSP);
   uint32_t wrap_pps_nalu(d3d12_video_encoder_bitstream *pNALU, d3d12_video_encoder_bitstream *pRBSP);

   // Helpers
   void     write_hrd(d3d12_video_encoder_bitstream *pBitstream, H264_HRD_PARAMS *pHrd);
   void     write_nalu_end(d3d12_video_encoder_bitstream *pNALU);
   void     rbsp_trailing(d3d12_video_encoder_bitstream *pBitstream);
   uint32_t wrap_rbsp_into_nalu(d3d12_video_encoder_bitstream *pNALU,
                                d3d12_video_encoder_bitstream *pRBSP,
                                uint32_t                       iNaluIdc,
                                uint32_t                       iNaluType);
};

#endif
