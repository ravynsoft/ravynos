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

#ifndef D3D12_VIDEO_ENC_NALU_WRITER_HEVC_H
#define D3D12_VIDEO_ENC_NALU_WRITER_HEVC_H

#include "d3d12_video_encoder_bitstream.h"

#define HEVC_MAX_REF_PICS       16
#define HEVC_MAX_SUB_LAYERS_NUM 7
#define HEVC_MAX_TILE_NUM       64

#define MAX_COMPRESSED_NALU (10*1024)

enum HEVCNaluType {
   HEVC_NALU_TRAIL_N = 0,
   HEVC_NALU_TRAIL_R = 1,
   HEVC_NALU_TSA_N = 2,
   HEVC_NALU_TSA_R = 3,
   HEVC_NALU_STSA_N = 4,
   HEVC_NALU_STSA_R = 5,
   HEVC_NALU_RADL_N = 6,
   HEVC_NALU_RADL_R = 7,
   HEVC_NALU_RASL_N = 8,
   HEVC_NALU_RASL_R = 9,
   HEVC_NALU_RSV_VCL_N10 = 10,
   HEVC_NALU_RSV_VCL_N12 = 12,
   HEVC_NALU_RSV_VCL_N14 = 14,
   HEVC_NALU_RSV_VCL_R11 = 11,
   HEVC_NALU_RSV_VCL_R13 = 13,
   HEVC_NALU_RSV_VCL_R15 = 15,
   HEVC_NALU_BLA_W_LP = 16,
   HEVC_NALU_BLA_W_RADL = 17,
   HEVC_NALU_BLA_N_LP = 18,
   HEVC_NALU_IDR_W_RADL = 19, 
   HEVC_NALU_IDR_N_LP = 20,
   HEVC_NALU_CRA_NUT = 21,
   HEVC_NALU_RSV_IRAP_VCL22 = 22,
   HEVC_NALU_RSV_IRAP_VCL23 = 23,
   HEVC_NALU_RSV_VCL_24 = 24,
   HEVC_NALU_RSV_VCL_25 = 25,
   HEVC_NALU_RSV_VCL_26 = 26,
   HEVC_NALU_RSV_VCL_27 = 27,
   HEVC_NALU_RSV_VCL_28 = 28,
   HEVC_NALU_RSV_VCL_29 = 29,
   HEVC_NALU_RSV_VCL_30 = 30,
   HEVC_NALU_RSV_VCL_31 = 31,
   HEVC_NALU_VPS_NUT = 32,
   HEVC_NALU_SPS_NUT = 33,
   HEVC_NALU_PPS_NUT = 34,
   HEVC_NALU_AUD_NUT = 35,
   HEVC_NALU_EOS_NUT = 36,
   HEVC_NALU_EOB_NUT = 37,
   HEVC_NALU_FD_NUT = 38,
   HEVC_NALU_PREFIX_SEI_NUT = 39,
   HEVC_NALU_SUFFIX_SEI_NUT = 40,
};

struct HEVCNaluHeader {
   uint8_t forbidden_zero_bit;
   uint8_t nal_unit_type;
   uint8_t nuh_layer_id;
   uint8_t nuh_temporal_id_plus1;
};

struct HEVCProfileTierLevel {
   uint8_t        general_profile_space;
   uint8_t        general_tier_flag;
   uint8_t        general_profile_idc;
   uint8_t        general_profile_compatibility_flag[32];
   uint8_t        general_progressive_source_flag;
   uint8_t        general_interlaced_source_flag;
   uint8_t        general_non_packed_constraint_flag;
   uint8_t        general_frame_only_constraint_flag;
   uint8_t        general_level_idc;
   uint8_t        sub_layer_profile_present_flag[HEVC_MAX_SUB_LAYERS_NUM - 1];
   uint8_t        sub_layer_level_present_flag[HEVC_MAX_SUB_LAYERS_NUM - 1];
   uint8_t        sub_layer_profile_space[HEVC_MAX_SUB_LAYERS_NUM - 1];
   uint8_t        sub_layer_tier_flag[HEVC_MAX_SUB_LAYERS_NUM - 1];
   uint8_t        sub_layer_profile_idc[HEVC_MAX_SUB_LAYERS_NUM - 1];
   uint8_t        sub_layer_profile_compatibility_flag[HEVC_MAX_SUB_LAYERS_NUM - 1][32];
   uint8_t        sub_layer_progressive_source_flag[HEVC_MAX_SUB_LAYERS_NUM - 1];
   uint8_t        sub_layer_interlaced_source_flag[HEVC_MAX_SUB_LAYERS_NUM - 1];
   uint8_t        sub_layer_non_packed_constraint_flag[HEVC_MAX_SUB_LAYERS_NUM - 1];
   uint8_t        sub_layer_frame_only_constraint_flag[HEVC_MAX_SUB_LAYERS_NUM - 1];
   int32_t        sub_layer_level_idc[HEVC_MAX_SUB_LAYERS_NUM - 1];
};

struct ReferencePictureSet {
   int32_t  numberOfPictures;
   int32_t  numberOfNegativePictures;
   int32_t  numberOfPositivePictures;
   int32_t  numberOfLongtermPictures;
   int32_t  deltaPOC[HEVC_MAX_REF_PICS];
   int32_t  POC[HEVC_MAX_REF_PICS];
   uint8_t  used[HEVC_MAX_REF_PICS];
   uint8_t  interRPSPrediction;
   int32_t  deltaRIdxMinus1;
   int32_t  deltaRPS;
   int32_t  numRefIdc;
   int32_t  refIdc[HEVC_MAX_REF_PICS + 1];
   uint8_t  bCheckLTMSB[HEVC_MAX_REF_PICS];
   int32_t  pocLSBLT[HEVC_MAX_REF_PICS];
   int32_t  deltaPOCMSBCycleLT[HEVC_MAX_REF_PICS];
   uint8_t  deltaPocMSBPresentFlag[HEVC_MAX_REF_PICS];
};

struct HEVCReferencePictureSet {
   uint8_t         inter_ref_pic_set_prediction_flag;
   union {
      struct {
         uint32_t        delta_idx_minus1;
         uint8_t         delta_rps_sign;
         uint32_t        abs_delta_rps_minus1;
         uint8_t         used_by_curr_pic_flag[HEVC_MAX_REF_PICS];
         uint8_t         use_delta_flag[HEVC_MAX_REF_PICS];
      };
      struct {
         int32_t         num_negative_pics;
         int32_t         num_positive_pics;
         int32_t         delta_poc_s0_minus1[HEVC_MAX_REF_PICS];
         uint8_t         used_by_curr_pic_s0_flag[HEVC_MAX_REF_PICS];
         int32_t         delta_poc_s1_minus1[HEVC_MAX_REF_PICS];
         uint8_t         used_by_curr_pic_s1_flag[HEVC_MAX_REF_PICS];
      };
   };
};

struct HEVCVideoUsabilityInfo {
    uint8_t     aspect_ratio_info_present_flag;
    uint8_t     aspect_ratio_idc;
    int32_t     sar_width;
    int32_t     sar_height;
    uint8_t     overscan_info_present_flag;
    uint8_t     overscan_appropriate_flag;
    uint8_t     video_signal_type_present_flag;
    uint8_t     video_format;
    uint8_t     video_full_range_flag;
    uint8_t     colour_description_present_flag;
    uint8_t     colour_primaries;
    uint8_t     transfer_characteristics;
    uint8_t     matrix_coeffs;
    uint8_t     chroma_loc_info_present_flag;
    int32_t     chroma_sample_loc_type_top_field;
    int32_t     chroma_sample_loc_type_bottom_field;
    uint8_t     neutral_chroma_indication_flag;
    uint8_t     field_seq_flag;
    uint8_t     frame_field_info_present_flag;
    uint8_t     default_display_window_flag;
    int32_t     def_disp_win_left_offset;
    int32_t     def_disp_win_right_offset;
    int32_t     def_disp_win_top_offset;
    int32_t     def_disp_win_bottom_offset;
    uint8_t     timing_info_present_flag;
    uint32_t    num_units_in_tick;
    uint32_t    time_scale;
    uint8_t     poc_proportional_to_timing_flag;
    uint32_t    num_ticks_poc_diff_one_minus1;
    uint8_t     hrd_parameters_present_flag;
    uint8_t     bitstream_restriction_flag;
    uint8_t     tiles_fixed_structure_flag;
    uint8_t     motion_vectors_over_pic_boundaries_flag;
    uint8_t     restricted_ref_pic_lists_flag;
    uint32_t    min_spatial_segmentation_idc;
    uint32_t    max_bytes_per_pic_denom;
    uint32_t    max_bits_per_min_cu_denom;
    uint32_t    log2_max_mv_length_horizontal;
    uint32_t    log2_max_mv_length_vertical;
};

struct HevcSeqParameterSet {
   HEVCNaluHeader  nalu;
   uint8_t         sps_video_parameter_set_id;
   uint8_t         sps_max_sub_layers_minus1;
   uint8_t         sps_temporal_id_nesting_flag;
   HEVCProfileTierLevel ptl;
   uint8_t         sps_seq_parameter_set_id;
   uint8_t         chroma_format_idc;
   uint8_t         separate_colour_plane_flag;
   int32_t         pic_width_in_luma_samples;
   int32_t         pic_height_in_luma_samples;
   uint8_t         conformance_window_flag;
   int32_t         conf_win_left_offset;
   int32_t         conf_win_right_offset;
   int32_t         conf_win_top_offset;
   int32_t         conf_win_bottom_offset;
   uint8_t         bit_depth_luma_minus8;
   uint8_t         bit_depth_chroma_minus8;
   uint8_t         log2_max_pic_order_cnt_lsb_minus4;
   int             maxPicOrderCntLsb;
   uint8_t         sps_sub_layer_ordering_info_present_flag;
   int32_t         sps_max_dec_pic_buffering_minus1[HEVC_MAX_SUB_LAYERS_NUM];
   int32_t         sps_max_num_reorder_pics[HEVC_MAX_SUB_LAYERS_NUM];
   int32_t         sps_max_latency_increase_plus1[HEVC_MAX_SUB_LAYERS_NUM];
   uint8_t         log2_min_luma_coding_block_size_minus3;
   uint8_t         log2_diff_max_min_luma_coding_block_size;
   uint8_t         log2_min_transform_block_size_minus2;
   uint8_t         log2_diff_max_min_transform_block_size;
   uint8_t         max_transform_hierarchy_depth_inter;
   uint8_t         max_transform_hierarchy_depth_intra;
   uint8_t         scaling_list_enabled_flag;
   uint8_t         sps_scaling_list_data_present_flag;
   uint8_t         scaling_list_pred_mode_flag[4][6];
   uint32_t        scaling_list_pred_matrix_id_delta[4][6];
   int32_t         scaling_list_dc_coef_minus8[2][6];
   int32_t         scaling_list_delta_coef;
   int32_t         ScalingList[4][6][64];
   uint8_t         amp_enabled_flag;
   uint8_t         sample_adaptive_offset_enabled_flag;
   uint8_t         pcm_enabled_flag;
   uint8_t         pcm_sample_bit_depth_luma_minus1;
   uint8_t         pcm_sample_bit_depth_chroma_minus1;
   int32_t         log2_min_pcm_luma_coding_block_size_minus3;
   int32_t         log2_diff_max_min_pcm_luma_coding_block_size;
   uint8_t         pcm_loop_filter_disabled_flag;
   uint8_t         num_short_term_ref_pic_sets;
   HEVCReferencePictureSet rpsShortTerm[64];
   uint8_t         long_term_ref_pics_present_flag;
   uint8_t         num_long_term_ref_pics_sps;
   int32_t         lt_ref_pic_poc_lsb_sps[32];
   uint8_t         used_by_curr_pic_lt_sps_flag[32];
   uint8_t         sps_temporal_mvp_enabled_flag;
   uint8_t         strong_intra_smoothing_enabled_flag;
   uint8_t         vui_parameters_present_flag;
   HEVCVideoUsabilityInfo vui;
   uint8_t         sps_extension_flag;
   uint8_t         sps_extension_data_flag;
};

struct HevcPicParameterSet {
   HEVCNaluHeader  nalu;
   uint8_t         pps_pic_parameter_set_id;
   uint8_t         pps_seq_parameter_set_id;
   uint8_t         dependent_slice_segments_enabled_flag;
   uint8_t         output_flag_present_flag;
   uint8_t         num_extra_slice_header_bits;
   uint8_t         sign_data_hiding_enabled_flag;
   uint8_t         cabac_init_present_flag;
   uint8_t         num_ref_idx_lx_default_active_minus1[2];
   int8_t          init_qp_minus26;
   uint8_t         constrained_intra_pred_flag;
   uint8_t         transform_skip_enabled_flag;
   uint8_t         cu_qp_delta_enabled_flag;
   uint8_t         diff_cu_qp_delta_depth;
   int8_t          pps_cb_qp_offset;
   int8_t          pps_cr_qp_offset;
   uint8_t         pps_slice_chroma_qp_offsets_present_flag;
   uint8_t         weighted_pred_flag;
   uint8_t         weighted_bipred_flag;
   uint8_t         transquant_bypass_enabled_flag;
   uint8_t         tiles_enabled_flag;
   uint8_t         entropy_coding_sync_enabled_flag;
   int32_t         num_tile_columns_minus1;
   int32_t         num_tile_rows_minus1;
   uint8_t         uniform_spacing_flag;
   int32_t         column_width_minus1[HEVC_MAX_TILE_NUM];
   int32_t         row_height_minus1[HEVC_MAX_TILE_NUM];
   uint8_t         loop_filter_across_tiles_enabled_flag;
   uint8_t         pps_loop_filter_across_slices_enabled_flag;
   uint8_t         deblocking_filter_control_present_flag;
   uint8_t         deblocking_filter_override_enabled_flag;
   uint8_t         pps_deblocking_filter_disabled_flag;
   int8_t          pps_beta_offset_div2;
   int8_t          pps_tc_offset_div2;
   uint8_t         pps_scaling_list_data_present_flag;
   uint8_t         lists_modification_present_flag;
   uint8_t         log2_parallel_merge_level_minus2;
   uint8_t         slice_segment_header_extension_present_flag;
   uint8_t         pps_extension_flag;
   uint8_t         pps_extension_data_flag;
};

struct HevcVideoParameterSet {
   HEVCNaluHeader nalu;
   uint8_t        vps_video_parameter_set_id;
   uint8_t        vps_reserved_three_2bits;
   uint8_t        vps_max_layers_minus1;
   uint8_t        vps_max_sub_layers_minus1;
   uint8_t        vps_temporal_id_nesting_flag;
   int32_t        vps_reserved_0xffff_16bits;
   HEVCProfileTierLevel ptl;
   uint8_t        vps_sub_layer_ordering_info_present_flag;
   uint8_t        vps_max_dec_pic_buffering_minus1[HEVC_MAX_SUB_LAYERS_NUM];
   uint8_t        vps_max_num_reorder_pics[HEVC_MAX_SUB_LAYERS_NUM];
   uint8_t        vps_max_latency_increase_plus1[HEVC_MAX_SUB_LAYERS_NUM];
   uint8_t        vps_max_layer_id;
   uint8_t        vps_num_layer_sets_minus1;
   uint8_t        layer_id_included_flag[1024][1];
   uint8_t        vps_timing_info_present_flag;
   uint32_t       vps_num_units_in_tick;
   uint32_t       vps_time_scale;
   uint8_t        vps_poc_proportional_to_timing_flag;
   uint32_t       vps_num_ticks_poc_diff_one_minus1;
   uint32_t       vps_num_hrd_parameters;
   uint32_t       hrd_layer_set_idx[1024];
   uint8_t        cprms_present_flag[1024];
   uint8_t        vps_extension_flag;
   uint8_t        vps_extension_data_flag;
};

class d3d12_video_nalu_writer_hevc
{
public:
   d3d12_video_nalu_writer_hevc() { }
   ~d3d12_video_nalu_writer_hevc() { }
   
   // Writes the HEVC VPS structure into a bitstream passed in headerBitstream
   // Function resizes bitstream accordingly and puts result in byte vector
   void vps_to_nalu_bytes(HevcVideoParameterSet *pVPS,
                        std::vector<BYTE>
                        &headerBitstream,
                        std::vector<BYTE>::iterator
                        placingPositionStart,
                        size_t &writtenBytes);

   // Writes the HEVC SPS structure into a bitstream passed in headerBitstream
   // Function resizes bitstream accordingly and puts result in byte vector
   void sps_to_nalu_bytes(HevcSeqParameterSet *pSPS,
                        std::vector<BYTE>
                        &headerBitstream,
                        std::vector<BYTE>::iterator
                        placingPositionStart,
                        size_t &writtenBytes);

   // Writes the HEVC PPS structure into a bitstream passed in headerBitstream
   // Function resizes bitstream accordingly and puts result in byte vector
   void pps_to_nalu_bytes(HevcPicParameterSet *pPPS,
                        std::vector<BYTE>
                        &headerBitstream,
                        std::vector<BYTE>::iterator
                        placingPositionStart,
                        size_t &writtenBytes);

   void write_end_of_stream_nalu(std::vector<BYTE> &headerBitstream,
                                 std::vector<BYTE>::iterator
                                 placingPositionStart,
                                 size_t
                                 &writtenBytes);
   void write_end_of_sequence_nalu(std::vector<BYTE>
                                 &headerBitstream,
                                 std::vector<BYTE>::iterator
                                 placingPositionStart,
                                 size_t &writtenBytes);

private:

   // Writes from structure into bitstream with RBSP trailing but WITHOUT NAL unit wrap (eg. nal_idc_type, etc)
   uint32_t write_vps_bytes(d3d12_video_encoder_bitstream *pBitstream, HevcVideoParameterSet *pSPS);
   uint32_t write_sps_bytes(d3d12_video_encoder_bitstream *pBitstream, HevcSeqParameterSet *pSPS);
   uint32_t write_pps_bytes(d3d12_video_encoder_bitstream *pBitstream, HevcPicParameterSet *pPPS);

   // Adds NALU wrapping into structures and ending NALU control bits
   uint32_t wrap_rbsp_into_nalu(d3d12_video_encoder_bitstream *pNALU, d3d12_video_encoder_bitstream *pRBSP, HEVCNaluHeader *pHeader);

   // Helpers
   void     write_nalu_end(d3d12_video_encoder_bitstream *pNALU);
   void     rbsp_trailing(d3d12_video_encoder_bitstream *pBitstream);
   void     write_profile_tier_level(d3d12_video_encoder_bitstream* rbsp, HEVCProfileTierLevel* ptl);
   
   void generic_write_bytes(std::vector<BYTE> &headerBitstream,
                           std::vector<BYTE>::iterator placingPositionStart,
                           size_t &writtenBytes, 
                           void *pStructure);
   uint32_t
   write_bytes_from_struct(d3d12_video_encoder_bitstream *pBitstream, void *pData, uint8_t nal_unit_type);

   void write_rps(d3d12_video_encoder_bitstream* rbsp, HevcSeqParameterSet* sps, int stRpsIdx, bool sliceRPS);
};

#endif
