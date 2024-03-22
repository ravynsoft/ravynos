/**************************************************************************
 *
 * Copyright 2017 Advanced Micro Devices, Inc.
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

#ifndef AC_VCN_ENC_H
#define AC_VCN_ENC_H

#define RENCODE_IB_OP_INITIALIZE                                                    0x01000001
#define RENCODE_IB_OP_CLOSE_SESSION                                                 0x01000002
#define RENCODE_IB_OP_ENCODE                                                        0x01000003
#define RENCODE_IB_OP_INIT_RC                                                       0x01000004
#define RENCODE_IB_OP_INIT_RC_VBV_BUFFER_LEVEL                                      0x01000005
#define RENCODE_IB_OP_SET_SPEED_ENCODING_MODE                                       0x01000006
#define RENCODE_IB_OP_SET_BALANCE_ENCODING_MODE                                     0x01000007
#define RENCODE_IB_OP_SET_QUALITY_ENCODING_MODE                                     0x01000008
#define RENCODE_IB_OP_SET_HIGH_QUALITY_ENCODING_MODE                                0x01000009

#define RENCODE_IF_MAJOR_VERSION_MASK                                               0xFFFF0000
#define RENCODE_IF_MAJOR_VERSION_SHIFT                                              16
#define RENCODE_IF_MINOR_VERSION_MASK                                               0x0000FFFF
#define RENCODE_IF_MINOR_VERSION_SHIFT                                              0

#define RENCODE_ENGINE_TYPE_ENCODE                                                  1

#define RENCODE_ENCODE_STANDARD_HEVC                                                0
#define RENCODE_ENCODE_STANDARD_H264                                                1
#define RENCODE_ENCODE_STANDARD_AV1                                                 2

#define RENCODE_PREENCODE_MODE_NONE                                                 0x00000000
#define RENCODE_PREENCODE_MODE_1X                                                   0x00000001
#define RENCODE_PREENCODE_MODE_2X                                                   0x00000002
#define RENCODE_PREENCODE_MODE_4X                                                   0x00000004

#define RENCODE_VBAQ_NONE                                                           0x00000000
#define RENCODE_VBAQ_AUTO                                                           0x00000001

#define RENCODE_PRESET_MODE_SPEED                                                   0x00000000
#define RENCODE_PRESET_MODE_BALANCE                                                 0x00000001
#define RENCODE_PRESET_MODE_QUALITY                                                 0x00000002
#define RENCODE_PRESET_MODE_HIGH_QUALITY                                            0x00000003

#define RENCODE_H264_SLICE_CONTROL_MODE_FIXED_MBS                                   0x00000000
#define RENCODE_H264_SLICE_CONTROL_MODE_FIXED_BITS                                  0x00000001

#define RENCODE_HEVC_SLICE_CONTROL_MODE_FIXED_CTBS                                  0x00000000
#define RENCODE_HEVC_SLICE_CONTROL_MODE_FIXED_BITS                                  0x00000001

#define RENCODE_RATE_CONTROL_METHOD_NONE                                            0x00000000
#define RENCODE_RATE_CONTROL_METHOD_LATENCY_CONSTRAINED_VBR                         0x00000001
#define RENCODE_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR                            0x00000002
#define RENCODE_RATE_CONTROL_METHOD_CBR                                             0x00000003

#define RENCODE_DIRECT_OUTPUT_NALU_TYPE_AUD                                         0x00000000
#define RENCODE_DIRECT_OUTPUT_NALU_TYPE_VPS                                         0x00000001
#define RENCODE_DIRECT_OUTPUT_NALU_TYPE_SPS                                         0x00000002
#define RENCODE_DIRECT_OUTPUT_NALU_TYPE_PPS                                         0x00000003
#define RENCODE_DIRECT_OUTPUT_NALU_TYPE_PREFIX                                      0x00000004
#define RENCODE_DIRECT_OUTPUT_NALU_TYPE_END_OF_SEQUENCE                             0x00000005
#define RENCODE_DIRECT_OUTPUT_NALU_TYPE_SEI                                         0x00000006

#define RENCODE_SLICE_HEADER_TEMPLATE_MAX_TEMPLATE_SIZE_IN_DWORDS                   16
#define RENCODE_SLICE_HEADER_TEMPLATE_MAX_NUM_INSTRUCTIONS                          16

#define RENCODE_HEADER_INSTRUCTION_END                                              0x00000000
#define RENCODE_HEADER_INSTRUCTION_COPY                                             0x00000001

#define RENCODE_HEVC_HEADER_INSTRUCTION_DEPENDENT_SLICE_END                         0x00010000
#define RENCODE_HEVC_HEADER_INSTRUCTION_FIRST_SLICE                                 0x00010001
#define RENCODE_HEVC_HEADER_INSTRUCTION_SLICE_SEGMENT                               0x00010002
#define RENCODE_HEVC_HEADER_INSTRUCTION_SLICE_QP_DELTA                              0x00010003
#define RENCODE_HEVC_HEADER_INSTRUCTION_SAO_ENABLE                                  0x00010004
#define RENCODE_HEVC_HEADER_INSTRUCTION_LOOP_FILTER_ACROSS_SLICES_ENABLE            0x00010005

#define RENCODE_H264_HEADER_INSTRUCTION_FIRST_MB                                    0x00020000
#define RENCODE_H264_HEADER_INSTRUCTION_SLICE_QP_DELTA                              0x00020001

#define RENCODE_AV1_BITSTREAM_INSTRUCTION_OBU_START                                 0x00000002
#define RENCODE_AV1_BITSTREAM_INSTRUCTION_OBU_SIZE                                  0x00000003
#define RENCODE_AV1_BITSTREAM_INSTRUCTION_OBU_END                                   0x00000004

#define RENCODE_OBU_START_TYPE_FRAME                                                1
#define RENCODE_OBU_START_TYPE_FRAME_HEADER                                         2
#define RENCODE_OBU_START_TYPE_TILE_GROUP                                           3

#define RENCODE_OBU_TYPE_SEQUENCE_HEADER                                            1
#define RENCODE_OBU_TYPE_TEMPORAL_DELIMITER                                         2
#define RENCODE_OBU_TYPE_FRAME_HEADER                                               3
#define RENCODE_OBU_TYPE_TILE_GROUP                                                 4
#define RENCODE_OBU_TYPE_METADATA                                                   5
#define RENCODE_OBU_TYPE_FRAME                                                      6
#define RENCODE_OBU_TYPE_REDUNDANT_FRAME_HEADER                                     7
#define RENCODE_OBU_TYPE_TILE_LIST                                                  8
#define RENCODE_OBU_TYPE_PADDING                                                    15

#define RENCODE_AV1_MV_PRECISION_ALLOW_HIGH_PRECISION                               0x00
#define RENCODE_AV1_MV_PRECISION_DISALLOW_HIGH_PRECISION                            0x10
#define RENCODE_AV1_MV_PRECISION_FORCE_INTEGER_MV                                   0x30

#define RENCODE_AV1_CDEF_MODE_DISABLE                                               0
#define RENCODE_AV1_CDEF_MODE_ENABLE                                                1

#define RENCODE_AV1_ORDER_HINT_BITS                                                 8
#define RENCODE_AV1_DELTA_FRAME_ID_LENGTH                                           15
#define RENCODE_AV1_ADDITIONAL_FRAME_ID_LENGTH                                      1

#define RENCDOE_AV1_NUM_REF_FRAMES                                                  8
#define RENCDOE_AV1_REFS_PER_FRAME                                                  7
#define RENCODE_AV1_SDB_FRAME_CONTEXT_SIZE                                          947200
#define RENCODE_AV1_FRAME_CONTEXT_CDF_TABLE_SIZE                                    22528
#define RENCODE_AV1_CDEF_ALGORITHM_FRAME_CONTEXT_SIZE                               (64 * 8 * 2)

#define RENCODE_PICTURE_TYPE_B                                                      0
#define RENCODE_PICTURE_TYPE_P                                                      1
#define RENCODE_PICTURE_TYPE_I                                                      2
#define RENCODE_PICTURE_TYPE_P_SKIP                                                 3

#define RENCODE_INPUT_SWIZZLE_MODE_LINEAR                                           0
#define RENCODE_INPUT_SWIZZLE_MODE_256B_S                                           1
#define RENCODE_INPUT_SWIZZLE_MODE_4kB_S                                            5
#define RENCODE_INPUT_SWIZZLE_MODE_64kB_S                                           9

#define RENCODE_H264_PICTURE_STRUCTURE_FRAME                                        0
#define RENCODE_H264_PICTURE_STRUCTURE_TOP_FIELD                                    1
#define RENCODE_H264_PICTURE_STRUCTURE_BOTTOM_FIELD                                 2

#define RENCODE_H264_INTERLACING_MODE_PROGRESSIVE                                   0
#define RENCODE_H264_INTERLACING_MODE_INTERLACED_STACKED                            1
#define RENCODE_H264_INTERLACING_MODE_INTERLACED_INTERLEAVED                        2

#define RENCODE_H264_DISABLE_DEBLOCKING_FILTER_IDC_ENABLE                           0
#define RENCODE_H264_DISABLE_DEBLOCKING_FILTER_IDC_DISABLE                          1
#define RENCODE_H264_DISABLE_DEBLOCKING_FILTER_IDC_DISABLE_ACROSS_SLICE_BOUNDARY    2

#define RENCODE_INTRA_REFRESH_MODE_NONE                                             0
#define RENCODE_INTRA_REFRESH_MODE_CTB_MB_ROWS                                      1
#define RENCODE_INTRA_REFRESH_MODE_CTB_MB_COLUMNS                                   2

#define RENCODE_MAX_NUM_RECONSTRUCTED_PICTURES                                      34

#define RENCODE_REC_SWIZZLE_MODE_LINEAR                                             0
#define RENCODE_REC_SWIZZLE_MODE_256B_S                                             1
#define RENCODE_REC_SWIZZLE_MODE_256B_D                                             2
#define RENCODE_REC_SWIZZLE_MODE_8x8_1D_THIN_12_24BPP                               0x10000001

#define RENCODE_VIDEO_BITSTREAM_BUFFER_MODE_LINEAR                                  0
#define RENCODE_VIDEO_BITSTREAM_BUFFER_MODE_CIRCULAR                                1

#define RENCODE_FEEDBACK_BUFFER_MODE_LINEAR                                         0
#define RENCODE_FEEDBACK_BUFFER_MODE_CIRCULAR                                       1

#define RENCODE_STATISTICS_TYPE_NONE                                                0
#define RENCODE_STATISTICS_TYPE_0                                                   1

#define RENCODE_MAX_NUM_TEMPORAL_LAYERS                                             4

#define PIPE_AV1_ENC_SB_SIZE                                                        64
#define PIPE_H265_ENC_CTB_SIZE                                                      64
#define PIPE_H264_MB_SIZE                                                           16

#define RENCODE_COLOR_VOLUME_G22_BT709                                              0

#define RENCODE_COLOR_RANGE_FULL                                                    0
#define RENCODE_COLOR_RANGE_STUDIO                                                  1
#define RENCODE_CHROMA_LOCATION_INTERSTITIAL                                        0

#define RENCODE_COLOR_BIT_DEPTH_8_BIT                                               0
#define RENCODE_COLOR_BIT_DEPTH_10_BIT                                              1

#define RENCODE_CHROMA_SUBSAMPLING_4_2_0                                            0
#define RENCODE_CHROMA_SUBSAMPLING_4_4_4                                            1

#define RENCODE_COLOR_PACKING_FORMAT_NV12                                           0
#define RENCODE_COLOR_PACKING_FORMAT_P010                                           1
#define RENCODE_COLOR_PACKING_FORMAT_A8R8G8B8                                       4
#define RENCODE_COLOR_PACKING_FORMAT_A8B8G8R8                                       7

#define RENCODE_COLOR_SPACE_YUV                                                     0
#define RENCODE_COLOR_SPACE_RGB                                                     1

typedef struct rvcn_enc_session_info_s {
   uint32_t interface_version;
   uint32_t sw_context_address_hi;
   uint32_t sw_context_address_lo;
} rvcn_enc_session_info_t;

typedef struct rvcn_enc_task_info_s {
   uint32_t total_size_of_all_packages;
   uint32_t task_id;
   uint32_t allowed_max_num_feedbacks;
} rvcn_enc_task_info_t;

typedef struct rvcn_enc_session_init_s {
   uint32_t encode_standard;
   uint32_t aligned_picture_width;
   uint32_t aligned_picture_height;
   uint32_t padding_width;
   uint32_t padding_height;
   uint32_t pre_encode_mode;
   uint32_t pre_encode_chroma_enabled;
   uint32_t slice_output_enabled;
   uint32_t display_remote;
} rvcn_enc_session_init_t;

typedef struct rvcn_enc_layer_control_s {
   uint32_t max_num_temporal_layers;
   uint32_t num_temporal_layers;
} rvcn_enc_layer_control_t;

typedef struct rvcn_enc_layer_select_s {
   uint32_t temporal_layer_index;
} rvcn_enc_layer_select_t;

typedef struct rvcn_enc_h264_slice_control_s {
   uint32_t slice_control_mode;
   union {
      uint32_t num_mbs_per_slice;
      uint32_t num_bits_per_slice;
   };
} rvcn_enc_h264_slice_control_t;

typedef struct rvcn_enc_hevc_slice_control_s {
   uint32_t slice_control_mode;
   union {
      struct {
         uint32_t num_ctbs_per_slice;
         uint32_t num_ctbs_per_slice_segment;
      } fixed_ctbs_per_slice;

      struct {
         uint32_t num_bits_per_slice;
         uint32_t num_bits_per_slice_segment;
      } fixed_bits_per_slice;
   };
} rvcn_enc_hevc_slice_control_t;

typedef struct rvcn_enc_h264_spec_misc_s {
   uint32_t constrained_intra_pred_flag;
   uint32_t cabac_enable;
   uint32_t cabac_init_idc;
   uint32_t half_pel_enabled;
   uint32_t quarter_pel_enabled;
   uint32_t profile_idc;
   uint32_t level_idc;
   uint32_t b_picture_enabled;
   uint32_t weighted_bipred_idc;
   struct {
      uint32_t deblocking_filter_control_present_flag:1;
      uint32_t redundant_pic_cnt_present_flag:1;
   };
} rvcn_enc_h264_spec_misc_t;

typedef struct rvcn_enc_hevc_spec_misc_s {
   uint32_t log2_min_luma_coding_block_size_minus3;
   uint32_t amp_disabled;
   uint32_t strong_intra_smoothing_enabled;
   uint32_t constrained_intra_pred_flag;
   uint32_t cabac_init_flag;
   uint32_t half_pel_enabled;
   uint32_t quarter_pel_enabled;
   uint32_t transform_skip_discarded;
   uint32_t cu_qp_delta_enabled_flag;
} rvcn_enc_hevc_spec_misc_t;

typedef struct rvcn_enc_av1_spec_misc_s {
   uint32_t palette_mode_enable;
   uint32_t mv_precision;
   uint32_t cdef_mode;
   uint32_t disable_cdf_update;
   uint32_t disable_frame_end_update_cdf;
   uint32_t num_tiles_per_picture;
} rvcn_enc_av1_spec_misc_t;

typedef struct rvcn_enc_rate_ctl_session_init_s {
   uint32_t rate_control_method;
   uint32_t vbv_buffer_level;
} rvcn_enc_rate_ctl_session_init_t;

typedef struct rvcn_enc_rate_ctl_layer_init_s {
   uint32_t target_bit_rate;
   uint32_t peak_bit_rate;
   uint32_t frame_rate_num;
   uint32_t frame_rate_den;
   uint32_t vbv_buffer_size;
   uint32_t avg_target_bits_per_picture;
   uint32_t peak_bits_per_picture_integer;
   uint32_t peak_bits_per_picture_fractional;
} rvcn_enc_rate_ctl_layer_init_t;

typedef struct rvcn_enc_rate_ctl_per_picture_s {
   uint32_t qp;
   uint32_t min_qp_app;
   uint32_t max_qp_app;
   uint32_t max_au_size;
   uint32_t enabled_filler_data;
   uint32_t skip_frame_enable;
   uint32_t enforce_hrd;
} rvcn_enc_rate_ctl_per_picture_t;

typedef struct rvcn_enc_quality_params_s {
   uint32_t vbaq_mode;
   uint32_t scene_change_sensitivity;
   uint32_t scene_change_min_idr_interval;
   uint32_t two_pass_search_center_map_mode;
   uint32_t vbaq_strength;
} rvcn_enc_quality_params_t;

typedef struct rvcn_enc_direct_output_nalu_s {
   uint32_t type;
   uint32_t size;
   uint32_t data[1];
} rvcn_enc_direct_output_nalu_t;

typedef struct rvcn_enc_slice_header_s {
   uint32_t bitstream_template[RENCODE_SLICE_HEADER_TEMPLATE_MAX_TEMPLATE_SIZE_IN_DWORDS];
   struct {
      uint32_t instruction;
      uint32_t num_bits;
   } instructions[RENCODE_SLICE_HEADER_TEMPLATE_MAX_NUM_INSTRUCTIONS];
} rvcn_enc_slice_header_t;

typedef struct rvcn_enc_h264_reference_picture_info_s {
   unsigned int pic_type;
   unsigned int is_long_term;
   unsigned int picture_structure;
   unsigned int pic_order_cnt;
} rvcn_enc_h264_reference_picture_info_t;

typedef struct rvcn_enc_encode_params_s {
   uint32_t pic_type;
   uint32_t allowed_max_bitstream_size;
   uint32_t input_picture_luma_address_hi;
   uint32_t input_picture_luma_address_lo;
   uint32_t input_picture_chroma_address_hi;
   uint32_t input_picture_chroma_address_lo;
   uint32_t input_pic_luma_pitch;
   uint32_t input_pic_chroma_pitch;
   uint8_t input_pic_swizzle_mode;
   uint32_t reference_picture_index;
   uint32_t reconstructed_picture_index;
} rvcn_enc_encode_params_t;

typedef struct rvcn_enc_h264_encode_params_s {
   uint32_t input_picture_structure;
   uint32_t input_pic_order_cnt;
   uint32_t interlaced_mode;
   uint32_t reference_picture_structure;
   uint32_t reference_picture1_index;
   rvcn_enc_h264_reference_picture_info_t picture_info_l0_reference_picture0;
   uint32_t l0_reference_picture1_index;
   rvcn_enc_h264_reference_picture_info_t picture_info_l0_reference_picture1;
   uint32_t l1_reference_picture0_index;
   rvcn_enc_h264_reference_picture_info_t picture_info_l1_reference_picture0;
   uint32_t is_reference;
} rvcn_enc_h264_encode_params_t;

typedef struct rvcn_enc_h264_deblocking_filter_s {
   uint32_t disable_deblocking_filter_idc;
   int32_t alpha_c0_offset_div2;
   int32_t beta_offset_div2;
   int32_t cb_qp_offset;
   int32_t cr_qp_offset;
} rvcn_enc_h264_deblocking_filter_t;

typedef struct rvcn_enc_hevc_deblocking_filter_s {
   uint32_t loop_filter_across_slices_enabled;
   int32_t deblocking_filter_disabled;
   int32_t beta_offset_div2;
   int32_t tc_offset_div2;
   int32_t cb_qp_offset;
   int32_t cr_qp_offset;
} rvcn_enc_hevc_deblocking_filter_t;

typedef struct rvcn_enc_intra_refresh_s {
   uint32_t intra_refresh_mode;
   uint32_t offset;
   uint32_t region_size;
} rvcn_enc_intra_refresh_t;

typedef struct rvcn_enc_reconstructed_picture_s {
   uint32_t luma_offset;
   uint32_t chroma_offset;
   union {
      struct
      {
         uint32_t av1_cdf_frame_context_offset;
         uint32_t av1_cdef_algorithm_context_offset;
      } av1;
   };
} rvcn_enc_reconstructed_picture_t;

typedef struct rvcn_enc_picture_info_s
{
   bool in_use;
   bool is_ltr;
   uint32_t pic_num;
} rvcn_enc_picture_info_t;

typedef struct rvcn_enc_pre_encode_input_picture_s {
   union {
      struct {
         uint32_t luma_offset;
         uint32_t chroma_offset;
      } yuv;
      struct {
         uint32_t red_offset;
         uint32_t green_offset;
         uint32_t blue_offset;
      } rgb;
   };
} rvcn_enc_pre_encode_input_picture_t;

typedef struct rvcn_enc_encode_context_buffer_s {
   uint32_t encode_context_address_hi;
   uint32_t encode_context_address_lo;
   uint32_t swizzle_mode;
   uint32_t rec_luma_pitch;
   uint32_t rec_chroma_pitch;
   uint32_t num_reconstructed_pictures;
   rvcn_enc_reconstructed_picture_t reconstructed_pictures[RENCODE_MAX_NUM_RECONSTRUCTED_PICTURES];
   uint32_t pre_encode_picture_luma_pitch;
   uint32_t pre_encode_picture_chroma_pitch;
   rvcn_enc_reconstructed_picture_t
   pre_encode_reconstructed_pictures[RENCODE_MAX_NUM_RECONSTRUCTED_PICTURES];
   rvcn_enc_pre_encode_input_picture_t pre_encode_input_picture;
   uint32_t two_pass_search_center_map_offset;
   union {
      uint32_t colloc_buffer_offset;
      struct {
         uint32_t av1_sdb_intermedidate_context_offset;
      } av1;
   };
} rvcn_enc_encode_context_buffer_t;

typedef struct rvcn_enc_video_bitstream_buffer_s {
   uint32_t mode;
   uint32_t video_bitstream_buffer_address_hi;
   uint32_t video_bitstream_buffer_address_lo;
   uint32_t video_bitstream_buffer_size;
   uint32_t video_bitstream_data_offset;
} rvcn_enc_video_bitstream_buffer_t;

typedef struct rvcn_enc_feedback_buffer_s {
   uint32_t mode;
   uint32_t feedback_buffer_address_hi;
   uint32_t feedback_buffer_address_lo;
   uint32_t feedback_buffer_size;
   uint32_t feedback_data_size;
} rvcn_enc_feedback_buffer_t;

typedef struct rvcn_enc_av1_cdf_default_table_s {
   uint32_t use_cdf_default;
   uint32_t cdf_default_buffer_address_lo;
   uint32_t cdf_default_buffer_address_hi;
} rvcn_enc_av1_cdf_default_table_t;

typedef struct rvcn_encode_stats_type_0_s
{
    uint32_t qp_frame;
    uint32_t qp_avg_ctb;
    uint32_t qp_max_ctb;
    uint32_t qp_min_ctb;
    uint32_t pix_intra;
    uint32_t pix_inter;
    uint32_t pix_skip;
    uint32_t bitcount_residual;
    uint32_t bitcount_all_minus_header;
    uint32_t bitcount_motion;
    uint32_t bitcount_inter;
    uint32_t bitcount_intra;
    uint32_t mv_x_frame;
    uint32_t mv_y_frame;
} rvcn_encode_stats_type_0_t;

typedef struct rvcn_encode_stats_s
{
    uint32_t encode_stats_type;
    uint32_t encode_stats_buffer_address_hi;
    uint32_t encode_stats_buffer_address_lo;
} rvcn_enc_stats_t;

typedef struct rvcn_enc_cmd_s {
   uint32_t session_info;
   uint32_t task_info;
   uint32_t session_init;
   uint32_t layer_control;
   uint32_t layer_select;
   uint32_t rc_session_init;
   uint32_t rc_layer_init;
   uint32_t rc_per_pic;
   uint32_t quality_params;
   uint32_t slice_header;
   uint32_t enc_params;
   uint32_t intra_refresh;
   uint32_t ctx;
   uint32_t bitstream;
   uint32_t feedback;
   uint32_t nalu;
   uint32_t slice_control_hevc;
   uint32_t spec_misc_hevc;
   uint32_t enc_params_hevc;
   uint32_t deblocking_filter_hevc;
   uint32_t slice_control_h264;
   uint32_t spec_misc_h264;
   uint32_t enc_params_h264;
   uint32_t deblocking_filter_h264;
   uint32_t spec_misc_av1;
   uint32_t bitstream_instruction_av1;
   uint32_t cdf_default_table_av1;
   uint32_t input_format;
   uint32_t output_format;
   uint32_t enc_statistics;
   uint32_t enc_qp_map;
} rvcn_enc_cmd_t;

typedef struct rvcn_enc_quality_modes_s
{
   unsigned pre_encode_mode;
   unsigned vbaq_mode;
   unsigned preset_mode;
} rvcn_enc_quality_modes_t;

typedef struct rvcn_enc_vui_info_s
{
   uint32_t vui_parameters_present_flag;
   struct {
      uint32_t aspect_ratio_info_present_flag : 1;
      uint32_t timing_info_present_flag : 1;
      uint32_t video_signal_type_present_flag : 1;
      uint32_t colour_description_present_flag : 1;
      uint32_t chroma_loc_info_present_flag : 1;
   } flags;
   uint32_t aspect_ratio_idc;
   uint32_t sar_width;
   uint32_t sar_height;
   uint32_t num_units_in_tick;
   uint32_t time_scale;
   uint32_t video_format;
   uint32_t video_full_range_flag;
   uint32_t colour_primaries;
   uint32_t transfer_characteristics;
   uint32_t matrix_coefficients;
   uint32_t chroma_sample_loc_type_top_field;
   uint32_t chroma_sample_loc_type_bottom_field;
   uint32_t max_num_reorder_frames;
}rvcn_enc_vui_info;

typedef struct rvcn_enc_input_format_s
{
   uint32_t input_color_volume;
   uint32_t input_color_space;
   uint32_t input_color_range;
   uint32_t input_chroma_subsampling;
   uint32_t input_chroma_location;
   uint32_t input_color_bit_depth;
   uint32_t input_color_packing_format;
} rvcn_enc_input_format_t;

typedef struct rvcn_enc_output_format_s
{
   uint32_t output_color_volume;
   uint32_t output_color_range;
   uint32_t output_chroma_location;  /* chroma location to luma */
   uint32_t output_color_bit_depth;
} rvcn_enc_output_format_t;

typedef struct rvcn_enc_av1_timing_info_s
{
   uint32_t num_units_in_display_tick;
   uint32_t time_scale;
   uint32_t num_tick_per_picture_minus1;
}rvcn_enc_av1_timing_info_t;

typedef struct rvcn_enc_av1_color_description_s
{
   uint32_t color_primaries;
   uint32_t transfer_characteristics;
   uint32_t maxtrix_coefficients;
   uint32_t color_range;
   uint32_t chroma_sample_position;
}rvcn_enc_av1_color_description_t;

#define AV1_ENC_FRAME_TYPE_KEY 0x00
#define AV1_ENC_FRAME_TYPE_INTER 0x01
#define AV1_ENC_FRAME_TYPE_INTRA_ONLY 0x02
#define AV1_ENC_FRAME_TYPE_SWITCH 0x03
#define AV1_ENC_FRAME_TYPE_SHOW_EXISTING 0x04

typedef struct rvcn_enc_av1_ref_frame_s
{
   bool in_use;
   uint32_t frame_id;
   uint32_t temporal_id;
   uint32_t slot_id;
   uint32_t frame_type;
} rvcn_enc_av1_ref_frame_t;

typedef struct rvcn_enc_av1_recon_slot_s
{
   bool in_use;
   bool is_orphaned;
} rvcn_enc_av1_recon_slot_t;

#define RENCODE_QP_MAP_TYPE_NONE               0
#define RENCODE_QP_MAP_TYPE_DELTA              1
#define RENCODE_QP_MAP_TYPE_MAP_PA             4
#define RENCODE_QP_MAP_MAX_REGIONS             32

struct rvcn_enc_qp_map_region
{
   bool     is_valid;
   int32_t  qp_delta;
   uint32_t x_in_unit;
   uint32_t y_in_unit;
   uint32_t width_in_unit;
   uint32_t height_in_unit;
};

typedef struct rvcn_enc_qp_map_s
{
   uint32_t qp_map_type;
   uint32_t qp_map_buffer_address_hi;
   uint32_t qp_map_buffer_address_lo;
   uint32_t qp_map_pitch;
   struct rvcn_enc_qp_map_region map[RENCODE_QP_MAP_MAX_REGIONS];
}rvcn_enc_qp_map_t;

#endif
