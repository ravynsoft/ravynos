/**************************************************************************
 *
 * Copyright (C) 2022 Kylin Software Co., Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

/**
 * @file
 * Data structure definition of video hardware layer.
 *
 * These structures are used for communication between host and guest, and
 * they are 4-byte aligned.
 *
 * 'virgl_picture_desc' and other related structures mainly describe sequence
 * parameters, picture parameters, slice parameters, etc., as well as some
 * context information for encoding and decoding. The video backend needs them
 * to reconstruct VA-API calls.
 *
 * @author Feng Jiang <jiangfeng@kylinos.cn>
 */

#ifndef VIRGL_VIDEO_HW_H
#define VIRGL_VIDEO_HW_H

#include <stdint.h>

struct virgl_base_picture_desc {
    uint16_t profile;       /* enum pipe_video_profile */
    uint8_t entry_point;    /* enum pipe_video_entrypoint */
    uint8_t protected_playback;
    uint8_t decrypt_key[256];
    uint32_t key_size;

};

struct virgl_enc_quality_modes {
    uint32_t level;
    uint32_t preset_mode;
    uint32_t pre_encode_mode;
    uint32_t vbaq_mode;
};

/* H.264 sequence parameter set */
struct virgl_h264_sps {
    uint8_t  level_idc;
    uint8_t  chroma_format_idc;
    uint8_t  separate_colour_plane_flag;
    uint8_t  bit_depth_luma_minus8;

    uint8_t  bit_depth_chroma_minus8;
    uint8_t  seq_scaling_matrix_present_flag;
    uint8_t  ScalingList4x4[6][16];
    uint8_t  ScalingList8x8[6][64];

    uint8_t  log2_max_frame_num_minus4;
    uint8_t  pic_order_cnt_type;
    uint8_t  log2_max_pic_order_cnt_lsb_minus4;
    uint8_t  delta_pic_order_always_zero_flag;

    int32_t  offset_for_non_ref_pic;
    int32_t  offset_for_top_to_bottom_field;
    int32_t  offset_for_ref_frame[256];

    uint8_t  num_ref_frames_in_pic_order_cnt_cycle;
    uint8_t  max_num_ref_frames;
    uint8_t  frame_mbs_only_flag;
    uint8_t  mb_adaptive_frame_field_flag;

    uint8_t  direct_8x8_inference_flag;
    uint8_t  MinLumaBiPredSize8x8;
    uint8_t  reserved[2];
};

/* H.264 picture parameter set */
struct virgl_h264_pps {
    struct virgl_h264_sps sps; /* Seq Param Set */

    uint8_t  entropy_coding_mode_flag;
    uint8_t  bottom_field_pic_order_in_frame_present_flag;
    uint8_t  num_slice_groups_minus1;
    uint8_t  slice_group_map_type;

    uint8_t  slice_group_change_rate_minus1;
    uint8_t  num_ref_idx_l0_default_active_minus1;
    uint8_t  num_ref_idx_l1_default_active_minus1;
    uint8_t  weighted_pred_flag;

    uint8_t  weighted_bipred_idc;
    int8_t   pic_init_qp_minus26;
    int8_t   pic_init_qs_minus26;
    int8_t   chroma_qp_index_offset;

    uint8_t  deblocking_filter_control_present_flag;
    uint8_t  constrained_intra_pred_flag;
    uint8_t  redundant_pic_cnt_present_flag;
    uint8_t  transform_8x8_mode_flag;

    uint8_t  ScalingList4x4[6][16];
    uint8_t  ScalingList8x8[6][64];

    int8_t   second_chroma_qp_index_offset;
    uint8_t  reserved[3];
};

struct virgl_h264_picture_desc {
    struct virgl_base_picture_desc base;

    struct virgl_h264_pps pps;  /* Picture Param Set */

    uint32_t frame_num;

    uint8_t  field_pic_flag;
    uint8_t  bottom_field_flag;
    uint8_t  num_ref_idx_l0_active_minus1;
    uint8_t  num_ref_idx_l1_active_minus1;

    uint32_t slice_count;
    int32_t  field_order_cnt[2];

    uint8_t  is_long_term[16];
    uint8_t  top_is_reference[16];
    uint8_t  bottom_is_reference[16];
    uint32_t field_order_cnt_list[16][2];
    uint32_t frame_num_list[16];
    uint32_t buffer_id[16];

    uint8_t  is_reference;
    uint8_t  num_ref_frames;
    uint8_t  reserved[2];
};

struct virgl_h264_enc_seq_param
{
   uint32_t enc_constraint_set_flags;
   uint32_t enc_frame_cropping_flag;
   uint32_t enc_frame_crop_left_offset;
   uint32_t enc_frame_crop_right_offset;
   uint32_t enc_frame_crop_top_offset;
   uint32_t enc_frame_crop_bottom_offset;
   uint32_t pic_order_cnt_type;
   uint32_t num_temporal_layers;
   uint32_t vui_parameters_present_flag;
   struct {
      uint32_t aspect_ratio_info_present_flag: 1;
      uint32_t timing_info_present_flag: 1;
      uint32_t reserved:30;
   } vui_flags;
   uint32_t aspect_ratio_idc;
   uint32_t sar_width;
   uint32_t sar_height;
   uint32_t num_units_in_tick;
   uint32_t time_scale;
};

struct virgl_h264_enc_rate_control
{
    uint32_t target_bitrate;
    uint32_t peak_bitrate;
    uint32_t frame_rate_num;
    uint32_t frame_rate_den;
    uint32_t vbv_buffer_size;
    uint32_t vbv_buf_lv;
    uint32_t target_bits_picture;
    uint32_t peak_bits_picture_integer;
    uint32_t peak_bits_picture_fraction;
    uint32_t fill_data_enable;
    uint32_t skip_frame_enable;
    uint32_t enforce_hrd;
    uint32_t max_au_size;
    uint32_t max_qp;
    uint32_t min_qp;

    uint8_t  rate_ctrl_method; /* see enum pipe_h2645_enc_rate_control_method */
    uint8_t  reserved[3];
};

struct virgl_h264_enc_motion_estimation
{
    uint32_t motion_est_quarter_pixel;
    uint32_t enc_disable_sub_mode;
    uint32_t lsmvert;
    uint32_t enc_en_ime_overw_dis_subm;
    uint32_t enc_ime_overw_dis_subm_no;
    uint32_t enc_ime2_search_range_x;
    uint32_t enc_ime2_search_range_y;
};

struct virgl_h264_enc_pic_control
{
    uint32_t enc_cabac_enable;
    uint32_t enc_cabac_init_idc;
};

struct virgl_h264_slice_descriptor
{
   uint32_t macroblock_address;
   uint32_t num_macroblocks;

   uint8_t  slice_type; /* see enum pipe_h264_slice_type  */
   uint8_t  reserved[3];
};

struct virgl_h264_enc_picture_desc
{
   struct virgl_base_picture_desc base;

   struct virgl_h264_enc_seq_param seq;
   struct virgl_h264_enc_rate_control rate_ctrl[4];
   struct virgl_h264_enc_motion_estimation motion_est;
   struct virgl_h264_enc_pic_control pic_ctrl;

   uint32_t intra_idr_period;

   uint32_t quant_i_frames;
   uint32_t quant_p_frames;
   uint32_t quant_b_frames;

   uint32_t frame_num;
   uint32_t frame_num_cnt;
   uint32_t p_remain;
   uint32_t i_remain;
   uint32_t idr_pic_id;
   uint32_t gop_cnt;
   uint32_t pic_order_cnt;
   uint32_t num_ref_idx_l0_active_minus1;
   uint32_t num_ref_idx_l1_active_minus1;
   uint32_t ref_idx_l0_list[32];
   uint8_t  l0_is_long_term[32];
   uint32_t ref_idx_l1_list[32];
   uint8_t  l1_is_long_term[32];
   uint32_t gop_size;
   struct virgl_enc_quality_modes quality_modes;

   uint32_t num_slice_descriptors;
   struct virgl_h264_slice_descriptor slices_descriptors[128];

   uint8_t  picture_type; /* see enum pipe_h2645_enc_picture_type */
   uint8_t  not_referenced;
   uint8_t  is_ltr;
   uint8_t  enable_vui;

   uint32_t ltr_index;
};


struct virgl_h265_sps
{
   uint32_t pic_width_in_luma_samples;
   uint32_t pic_height_in_luma_samples;

   uint8_t chroma_format_idc;
   uint8_t separate_colour_plane_flag;
   uint8_t bit_depth_luma_minus8;
   uint8_t bit_depth_chroma_minus8;

   uint8_t log2_max_pic_order_cnt_lsb_minus4;
   uint8_t sps_max_dec_pic_buffering_minus1;
   uint8_t log2_min_luma_coding_block_size_minus3;
   uint8_t log2_diff_max_min_luma_coding_block_size;

   uint8_t log2_min_transform_block_size_minus2;
   uint8_t log2_diff_max_min_transform_block_size;
   uint8_t max_transform_hierarchy_depth_inter;
   uint8_t max_transform_hierarchy_depth_intra;

   uint8_t ScalingList4x4[6][16];
   uint8_t ScalingList8x8[6][64];
   uint8_t ScalingList16x16[6][64];
   uint8_t ScalingList32x32[2][64];

   uint8_t ScalingListDCCoeff16x16[6];
   uint8_t ScalingListDCCoeff32x32[2];

   uint8_t scaling_list_enabled_flag;
   uint8_t amp_enabled_flag;
   uint8_t sample_adaptive_offset_enabled_flag;
   uint8_t pcm_enabled_flag;

   uint8_t pcm_sample_bit_depth_luma_minus1;
   uint8_t pcm_sample_bit_depth_chroma_minus1;
   uint8_t log2_min_pcm_luma_coding_block_size_minus3;
   uint8_t log2_diff_max_min_pcm_luma_coding_block_size;

   uint8_t pcm_loop_filter_disabled_flag;
   uint8_t num_short_term_ref_pic_sets;
   uint8_t long_term_ref_pics_present_flag;
   uint8_t num_long_term_ref_pics_sps;

   uint8_t sps_temporal_mvp_enabled_flag;
   uint8_t strong_intra_smoothing_enabled_flag;
   uint8_t reserved[2];
};

struct virgl_h265_pps
{
   struct virgl_h265_sps sps;

   uint8_t dependent_slice_segments_enabled_flag;
   uint8_t output_flag_present_flag;
   uint8_t num_extra_slice_header_bits;
   uint8_t sign_data_hiding_enabled_flag;

   uint8_t cabac_init_present_flag;
   uint8_t num_ref_idx_l0_default_active_minus1;
   uint8_t num_ref_idx_l1_default_active_minus1;
   int8_t init_qp_minus26;

   uint8_t constrained_intra_pred_flag;
   uint8_t transform_skip_enabled_flag;
   uint8_t cu_qp_delta_enabled_flag;
   uint8_t diff_cu_qp_delta_depth;

   int8_t pps_cb_qp_offset;
   int8_t pps_cr_qp_offset;
   uint8_t pps_slice_chroma_qp_offsets_present_flag;
   uint8_t weighted_pred_flag;

   uint8_t weighted_bipred_flag;
   uint8_t transquant_bypass_enabled_flag;
   uint8_t tiles_enabled_flag;
   uint8_t entropy_coding_sync_enabled_flag;

   uint16_t column_width_minus1[20];
   uint16_t row_height_minus1[22];

   uint8_t num_tile_columns_minus1;
   uint8_t num_tile_rows_minus1;
   uint8_t uniform_spacing_flag;
   uint8_t loop_filter_across_tiles_enabled_flag;

   uint8_t pps_loop_filter_across_slices_enabled_flag;
   uint8_t deblocking_filter_control_present_flag;
   uint8_t deblocking_filter_override_enabled_flag;
   uint8_t pps_deblocking_filter_disabled_flag;

   int8_t pps_beta_offset_div2;
   int8_t pps_tc_offset_div2;
   uint8_t lists_modification_present_flag;
   uint8_t log2_parallel_merge_level_minus2;

   uint16_t st_rps_bits;
   uint8_t slice_segment_header_extension_present_flag;
   uint8_t reserved;
};

struct virgl_h265_picture_desc
{
   struct virgl_base_picture_desc base;

   struct virgl_h265_pps pps;

   int32_t CurrPicOrderCntVal;
   uint32_t ref[16];
   int32_t PicOrderCntVal[16];

   uint32_t NumPocTotalCurr;
   uint32_t NumDeltaPocsOfRefRpsIdx;
   uint32_t NumShortTermPictureSliceHeaderBits;
   uint32_t NumLongTermPictureSliceHeaderBits;

   uint8_t IsLongTerm[16];

   uint8_t IDRPicFlag;
   uint8_t RAPPicFlag;
   uint8_t CurrRpsIdx;
   uint8_t NumPocStCurrBefore;

   uint8_t NumPocStCurrAfter;
   uint8_t NumPocLtCurr;
   uint8_t UseRefPicList;
   uint8_t UseStRpsBits;

   uint8_t RefPicSetStCurrBefore[8];
   uint8_t RefPicSetStCurrAfter[8];
   uint8_t RefPicSetLtCurr[8];

   uint8_t RefPicList[2][15];
   uint8_t reserved[2];
};

struct virgl_h265_enc_seq_param
{
   uint8_t  general_profile_idc;
   uint8_t  general_level_idc;
   uint8_t  general_tier_flag;
   uint8_t  strong_intra_smoothing_enabled_flag;

   uint32_t intra_period;
   uint32_t ip_period;

   uint16_t pic_width_in_luma_samples;
   uint16_t pic_height_in_luma_samples;

   uint32_t chroma_format_idc;
   uint32_t bit_depth_luma_minus8;
   uint32_t bit_depth_chroma_minus8;

   uint8_t  amp_enabled_flag;
   uint8_t  sample_adaptive_offset_enabled_flag;
   uint8_t  pcm_enabled_flag;
   uint8_t  sps_temporal_mvp_enabled_flag;

   uint8_t  log2_min_luma_coding_block_size_minus3;
   uint8_t  log2_diff_max_min_luma_coding_block_size;
   uint8_t  log2_min_transform_block_size_minus2;
   uint8_t  log2_diff_max_min_transform_block_size;

   uint16_t conf_win_left_offset;
   uint16_t conf_win_right_offset;
   uint16_t conf_win_top_offset;
   uint16_t conf_win_bottom_offset;

   uint32_t vui_parameters_present_flag;
   struct {
      uint32_t aspect_ratio_info_present_flag: 1;
      uint32_t timing_info_present_flag: 1;
      uint32_t reserved:30;
   } vui_flags;
   uint32_t aspect_ratio_idc;
   uint32_t sar_width;
   uint32_t sar_height;
   uint32_t num_units_in_tick;
   uint32_t time_scale;

   uint8_t  max_transform_hierarchy_depth_inter;
   uint8_t  max_transform_hierarchy_depth_intra;
   uint8_t  conformance_window_flag;
   uint8_t  reserved;
};

struct virgl_h265_enc_pic_param
{
   uint8_t log2_parallel_merge_level_minus2;
   uint8_t nal_unit_type;
   uint8_t constrained_intra_pred_flag;
   uint8_t pps_loop_filter_across_slices_enabled_flag;

   uint8_t transform_skip_enabled_flag;
   uint8_t reserved[3];
};

struct virgl_h265_enc_slice_param
{
   uint8_t max_num_merge_cand;
   int8_t  slice_cb_qp_offset;
   int8_t  slice_cr_qp_offset;
   int8_t  slice_beta_offset_div2;

   uint32_t slice_deblocking_filter_disabled_flag;

   int8_t  slice_tc_offset_div2;
   uint8_t cabac_init_flag;
   uint8_t slice_loop_filter_across_slices_enabled_flag;
   uint8_t reserved;
};

struct virgl_h265_enc_rate_control
{
   uint32_t target_bitrate;
   uint32_t peak_bitrate;
   uint32_t frame_rate_num;
   uint32_t frame_rate_den;
   uint32_t quant_i_frames;
   uint32_t quant_p_frames;
   uint32_t quant_b_frames;
   uint32_t vbv_buffer_size;
   uint32_t vbv_buf_lv;
   uint32_t target_bits_picture;
   uint32_t peak_bits_picture_integer;
   uint32_t peak_bits_picture_fraction;
   uint32_t fill_data_enable;
   uint32_t skip_frame_enable;
   uint32_t enforce_hrd;
   uint32_t max_au_size;
   uint32_t max_qp;
   uint32_t min_qp;

   uint8_t  rate_ctrl_method; /* see enum pipe_h2645_enc_rate_control_method */
   uint8_t  reserved[3];
};

struct virgl_h265_slice_descriptor
{
   uint32_t slice_segment_address;
   uint32_t num_ctu_in_slice;

   uint8_t  slice_type; /* see enum pipe_h265_slice_type */
   uint8_t  reserved[3];
};

struct virgl_h265_enc_picture_desc
{
   struct virgl_base_picture_desc base;

   struct virgl_h265_enc_seq_param seq;
   struct virgl_h265_enc_pic_param pic;
   struct virgl_h265_enc_slice_param slice;
   struct virgl_h265_enc_rate_control rc;

   uint32_t decoded_curr_pic;
   uint32_t reference_frames[16];
   uint32_t frame_num;
   uint32_t pic_order_cnt;
   uint32_t pic_order_cnt_type;
   uint32_t num_ref_idx_l0_active_minus1;
   uint32_t num_ref_idx_l1_active_minus1;
   uint32_t ref_idx_l0_list[15];
   uint32_t ref_idx_l1_list[15];
   uint32_t num_slice_descriptors;
   struct virgl_h265_slice_descriptor slices_descriptors[128];
   struct virgl_enc_quality_modes quality_modes;

   uint8_t  picture_type; /* see enum pipe_h2645_enc_picture_type */
   uint8_t  not_referenced;
   uint8_t  reserved[2];
};

struct virgl_mpeg4_picture_desc
{
   struct virgl_base_picture_desc base;

   int32_t trd[2];
   int32_t trb[2];
   uint16_t vop_time_increment_resolution;
   uint8_t vop_coding_type;
   uint8_t vop_fcode_forward;
   uint8_t vop_fcode_backward;
   uint8_t resync_marker_disable;
   uint8_t interlaced;
   uint8_t quant_type;
   uint8_t quarter_sample;
   uint8_t short_video_header;
   uint8_t rounding_control;
   uint8_t alternate_vertical_scan_flag;
   uint8_t top_field_first;

   uint8_t intra_matrix[64];
   uint8_t non_intra_matrix[64];

   uint32_t ref[2];
};

struct virgl_mpeg12_picture_desc
{
    struct virgl_base_picture_desc base;

    unsigned picture_coding_type;
    unsigned picture_structure;
    unsigned frame_pred_frame_dct;
    unsigned q_scale_type;
    unsigned alternate_scan;
    unsigned intra_vlc_format;
    unsigned concealment_motion_vectors;
    unsigned intra_dc_precision;
    unsigned f_code[2][2];
    unsigned top_field_first;
    unsigned full_pel_forward_vector;
    unsigned full_pel_backward_vector;
    unsigned num_slices;

    uint8_t intra_matrix[64];
    uint8_t non_intra_matrix[64];

    uint32_t ref[2];
};

struct virgl_vc1_picture_desc
{
   struct virgl_base_picture_desc base;

   uint32_t slice_count;
   uint8_t picture_type;
   uint8_t frame_coding_mode;
   uint8_t postprocflag;
   uint8_t pulldown;
   uint8_t interlace;
   uint8_t tfcntrflag;
   uint8_t finterpflag;
   uint8_t psf;
   uint8_t dquant;
   uint8_t panscan_flag;
   uint8_t refdist_flag;
   uint8_t quantizer;
   uint8_t extended_mv;
   uint8_t extended_dmv;
   uint8_t overlap;
   uint8_t vstransform;
   uint8_t loopfilter;
   uint8_t fastuvmc;
   uint8_t range_mapy_flag;
   uint8_t range_mapy;
   uint8_t range_mapuv_flag;
   uint8_t range_mapuv;
   uint8_t multires;
   uint8_t syncmarker;
   uint8_t rangered;
   uint8_t maxbframes;
   uint8_t deblockEnable;
   uint8_t pquant;

   uint32_t ref[2];
};

struct virgl_mjpeg_picture_desc
{
   struct virgl_base_picture_desc base;

    struct
    {
        uint16_t picture_width;
        uint16_t picture_height;

        struct {
            uint8_t component_id;
            uint8_t h_sampling_factor;
            uint8_t v_sampling_factor;
            uint8_t quantiser_table_selector;
        } components[255];

        uint8_t num_components;
        uint16_t crop_x;
        uint16_t crop_y;
        uint16_t crop_width;
        uint16_t crop_height;
    } picture_parameter;

    struct
    {
        uint8_t load_quantiser_table[4];
        uint8_t quantiser_table[4][64];
    } quantization_table;

    struct
    {
        uint8_t load_huffman_table[2];

        struct {
            uint8_t   num_dc_codes[16];
            uint8_t   dc_values[12];
            uint8_t   num_ac_codes[16];
            uint8_t   ac_values[162];
            uint8_t   pad[2];
        } table[2];
    } huffman_table;

    struct
    {
        unsigned slice_data_size;
        unsigned slice_data_offset;
        unsigned slice_data_flag;
        unsigned slice_horizontal_position;
        unsigned slice_vertical_position;

        struct {
            uint8_t component_selector;
            uint8_t dc_table_selector;
            uint8_t ac_table_selector;
        } components[4];

        uint8_t num_components;

        uint16_t restart_interval;
        unsigned num_mcus;
    } slice_parameter;
};

struct virgl_vp9_segment_parameter
{
    struct {
        uint16_t segment_reference_enabled:1;
        uint16_t segment_reference:2;
        uint16_t segment_reference_skipped:1;
    } segment_flags;

    uint8_t filter_level[4][2];
    int16_t luma_ac_quant_scale;
    int16_t luma_dc_quant_scale;
    int16_t chroma_ac_quant_scale;
    int16_t chroma_dc_quant_scale;
};

struct virgl_vp9_picture_desc
{
    struct virgl_base_picture_desc base;

    uint32_t ref[16];

    struct {
        uint16_t frame_width;
        uint16_t frame_height;

        struct {
            uint32_t  subsampling_x:1;
            uint32_t  subsampling_y:1;
            uint32_t  frame_type:1;
            uint32_t  show_frame:1;
            uint32_t  error_resilient_mode:1;
            uint32_t  intra_only:1;
            uint32_t  allow_high_precision_mv:1;
            uint32_t  mcomp_filter_type:3;
            uint32_t  frame_parallel_decoding_mode:1;
            uint32_t  reset_frame_context:2;
            uint32_t  refresh_frame_context:1;
            uint32_t  frame_context_idx:2;
            uint32_t  segmentation_enabled:1;
            uint32_t  segmentation_temporal_update:1;
            uint32_t  segmentation_update_map:1;
            uint32_t  last_ref_frame:3;
            uint32_t  last_ref_frame_sign_bias:1;
            uint32_t  golden_ref_frame:3;
            uint32_t  golden_ref_frame_sign_bias:1;
            uint32_t  alt_ref_frame:3;
            uint32_t  alt_ref_frame_sign_bias:1;
            uint32_t  lossless_flag:1;
        } pic_fields;

        uint8_t filter_level;
        uint8_t sharpness_level;
        uint8_t log2_tile_rows;
        uint8_t log2_tile_columns;
        uint8_t frame_header_length_in_bytes;
        uint16_t first_partition_size;
        uint8_t mb_segment_tree_probs[7];
        uint8_t segment_pred_probs[3];
        uint8_t profile;
        uint8_t bit_depth;

        bool mode_ref_delta_enabled;
        bool mode_ref_delta_update;

        uint8_t base_qindex;
        int8_t y_dc_delta_q;
        int8_t uv_ac_delta_q;
        int8_t uv_dc_delta_q;
        uint8_t abs_delta;
        uint8_t ref_deltas[4];
        uint8_t mode_deltas[2];
    } picture_parameter;

    struct {
        uint32_t slice_data_size;
        uint32_t slice_data_offset;
        uint32_t slice_data_flag;
        struct virgl_vp9_segment_parameter seg_param[8];
    } slice_parameter;
};

struct virgl_av1_picture_desc
{
   struct virgl_base_picture_desc base;

   uint32_t ref[16];
   uint32_t film_grain_target;
   struct {
      uint8_t profile;
      uint8_t order_hint_bits_minus_1;
      uint8_t bit_depth_idx;

      struct {
         uint32_t use_128x128_superblock:1;
         uint32_t enable_filter_intra:1;
         uint32_t enable_intra_edge_filter:1;
         uint32_t enable_interintra_compound:1;
         uint32_t enable_masked_compound:1;
         uint32_t enable_dual_filter:1;
         uint32_t enable_order_hint:1;
         uint32_t enable_jnt_comp:1;
         uint32_t enable_cdef:1;
         uint32_t mono_chrome:1;
         uint32_t ref_frame_mvs:1;
         uint32_t film_grain_params_present:1;
      } seq_info_fields;

      uint32_t current_frame_id;

      uint16_t frame_width;
      uint16_t frame_height;
      uint16_t max_width;
      uint16_t max_height;

      uint8_t ref_frame_idx[7];
      uint8_t primary_ref_frame;
      uint8_t order_hint;

      struct {
         struct {
            uint32_t enabled:1;
            uint32_t update_map:1;
            uint32_t update_data:1;
            uint32_t temporal_update:1;
         } segment_info_fields;

         int16_t feature_data[8][8];
         uint8_t feature_mask[8];
      } seg_info;

      struct {
         struct {
            uint32_t apply_grain:1;
            uint32_t chroma_scaling_from_luma:1;
            uint32_t grain_scaling_minus_8:2;
            uint32_t ar_coeff_lag:2;
            uint32_t ar_coeff_shift_minus_6:2;
            uint32_t grain_scale_shift:2;
            uint32_t overlap_flag:1;
            uint32_t clip_to_restricted_range:1;
         } film_grain_info_fields;

         uint16_t grain_seed;
         uint8_t num_y_points;
         uint8_t point_y_value[14];
         uint8_t point_y_scaling[14];
         uint8_t num_cb_points;
         uint8_t point_cb_value[10];
         uint8_t point_cb_scaling[10];
         uint8_t num_cr_points;
         uint8_t point_cr_value[10];
         uint8_t point_cr_scaling[10];
         int8_t ar_coeffs_y[24];
         int8_t ar_coeffs_cb[25];
         int8_t ar_coeffs_cr[25];
         uint8_t cb_mult;
         uint8_t cb_luma_mult;
         uint16_t cb_offset;
         uint8_t cr_mult;
         uint8_t cr_luma_mult;
         uint16_t cr_offset;
      } film_grain_info;

      uint8_t tile_cols;
      uint8_t tile_rows;
      uint32_t tile_col_start_sb[65];
      uint32_t tile_row_start_sb[65];
      uint16_t width_in_sbs[64];
      uint16_t height_in_sbs[64];
      uint16_t context_update_tile_id;

      struct {
         uint32_t frame_type:2;
         uint32_t show_frame:1;
         uint32_t showable_frame:1;
         uint32_t error_resilient_mode:1;
         uint32_t disable_cdf_update:1;
         uint32_t allow_screen_content_tools:1;
         uint32_t force_integer_mv:1;
         uint32_t allow_intrabc:1;
         uint32_t use_superres:1;
         uint32_t allow_high_precision_mv:1;
         uint32_t is_motion_mode_switchable:1;
         uint32_t use_ref_frame_mvs:1;
         uint32_t disable_frame_end_update_cdf:1;
         uint32_t uniform_tile_spacing_flag:1;
         uint32_t allow_warped_motion:1;
         uint32_t large_scale_tile:1;
      } pic_info_fields;

      uint8_t superres_scale_denominator;

      uint8_t interp_filter;
      uint8_t filter_level[2];
      uint8_t filter_level_u;
      uint8_t filter_level_v;
      struct {
         uint8_t sharpness_level:3;
         uint8_t mode_ref_delta_enabled:1;
         uint8_t mode_ref_delta_update:1;
      } loop_filter_info_fields;

      int8_t ref_deltas[8];
      int8_t mode_deltas[2];

      uint8_t base_qindex;
      int8_t y_dc_delta_q;
      int8_t u_dc_delta_q;
      int8_t u_ac_delta_q;
      int8_t v_dc_delta_q;
      int8_t v_ac_delta_q;

      struct {
         uint16_t using_qmatrix:1;
         uint16_t qm_y:4;
         uint16_t qm_u:4;
         uint16_t qm_v:4;
      } qmatrix_fields;

      struct {
         uint32_t delta_q_present_flag:1;
         uint32_t log2_delta_q_res:2;
         uint32_t delta_lf_present_flag:1;
         uint32_t log2_delta_lf_res:2;
         uint32_t delta_lf_multi:1;
         uint32_t tx_mode:2;
         uint32_t reference_select:1;
         uint32_t reduced_tx_set_used:1;
         uint32_t skip_mode_present:1;
      } mode_control_fields;

      uint8_t cdef_damping_minus_3;
      uint8_t cdef_bits;
      uint8_t cdef_y_strengths[8];
      uint8_t cdef_uv_strengths[8];

      struct {
         uint16_t yframe_restoration_type:2;
         uint16_t cbframe_restoration_type:2;
         uint16_t crframe_restoration_type:2;
         uint16_t lr_unit_shift:2;
         uint16_t lr_uv_shift:1;
      } loop_restoration_fields;

      uint16_t lr_unit_size[3];

      struct {
         uint32_t wmtype;
         uint8_t invalid;
         int32_t wmmat[8];
      } wm[7];

      uint32_t refresh_frame_flags;
      uint8_t matrix_coefficients;
   } picture_parameter;

   struct {
      uint32_t slice_data_size[256];
      uint32_t slice_data_offset[256];
      uint16_t slice_data_row[256];
      uint16_t slice_data_col[256];
      uint8_t slice_data_anchor_frame_idx[256];
      uint16_t slice_count;
   } slice_parameter;
};

union virgl_picture_desc {
    struct virgl_base_picture_desc base;
    struct virgl_h264_picture_desc h264;
    struct virgl_h265_picture_desc h265;
    struct virgl_mpeg4_picture_desc mpeg4;
    struct virgl_mpeg12_picture_desc mpeg12;
    struct virgl_vc1_picture_desc vc1;
    struct virgl_mjpeg_picture_desc mjpeg;
    struct virgl_av1_picture_desc av1;
    struct virgl_h264_enc_picture_desc h264_enc;
    struct virgl_h265_enc_picture_desc h265_enc;
    struct virgl_vp9_picture_desc vp9;
};

enum virgl_video_encode_stat {
    VIRGL_VIDEO_ENCODE_STAT_NOT_STARTED = 0,
    VIRGL_VIDEO_ENCODE_STAT_IN_PROGRESS,
    VIRGL_VIDEO_ENCODE_STAT_SUCCESS,
    VIRGL_VIDEO_ENCODE_STAT_FAILURE,
};

struct virgl_video_encode_feedback {
    uint8_t stat;           /* see enum virgl_video_encode_stat */
    uint8_t reserved[3];

    uint32_t coded_size;    /* size of encoded data in bytes */
};

#endif /* VIRGL_VIDEO_HW_H */

