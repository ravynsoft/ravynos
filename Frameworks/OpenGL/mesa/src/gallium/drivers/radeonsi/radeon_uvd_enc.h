/**************************************************************************
 *
 * Copyright 2018 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 *
 **************************************************************************/

#ifndef _RADEON_UVD_ENC_H
#define _RADEON_UVD_ENC_H

#include "radeon_video.h"

#define RENC_UVD_FW_INTERFACE_MAJOR_VERSION 1
#define RENC_UVD_FW_INTERFACE_MINOR_VERSION 1

#define RENC_UVD_IB_PARAM_SESSION_INFO               0x00000001
#define RENC_UVD_IB_PARAM_TASK_INFO                  0x00000002
#define RENC_UVD_IB_PARAM_SESSION_INIT               0x00000003
#define RENC_UVD_IB_PARAM_LAYER_CONTROL              0x00000004
#define RENC_UVD_IB_PARAM_LAYER_SELECT               0x00000005
#define RENC_UVD_IB_PARAM_SLICE_CONTROL              0x00000006
#define RENC_UVD_IB_PARAM_SPEC_MISC                  0x00000007
#define RENC_UVD_IB_PARAM_RATE_CONTROL_SESSION_INIT  0x00000008
#define RENC_UVD_IB_PARAM_RATE_CONTROL_LAYER_INIT    0x00000009
#define RENC_UVD_IB_PARAM_RATE_CONTROL_PER_PICTURE   0x0000000a
#define RENC_UVD_IB_PARAM_SLICE_HEADER               0x0000000b
#define RENC_UVD_IB_PARAM_ENCODE_PARAMS              0x0000000c
#define RENC_UVD_IB_PARAM_QUALITY_PARAMS             0x0000000d
#define RENC_UVD_IB_PARAM_DEBLOCKING_FILTER          0x0000000e
#define RENC_UVD_IB_PARAM_INTRA_REFRESH              0x0000000f
#define RENC_UVD_IB_PARAM_ENCODE_CONTEXT_BUFFER      0x00000010
#define RENC_UVD_IB_PARAM_VIDEO_BITSTREAM_BUFFER     0x00000011
#define RENC_UVD_IB_PARAM_FEEDBACK_BUFFER            0x00000012
#define RENC_UVD_IB_PARAM_INSERT_NALU_BUFFER         0x00000013
#define RENC_UVD_IB_PARAM_FEEDBACK_BUFFER_ADDITIONAL 0x00000014

#define RENC_UVD_IB_OP_INITIALIZE                0x08000001
#define RENC_UVD_IB_OP_CLOSE_SESSION             0x08000002
#define RENC_UVD_IB_OP_ENCODE                    0x08000003
#define RENC_UVD_IB_OP_INIT_RC                   0x08000004
#define RENC_UVD_IB_OP_INIT_RC_VBV_BUFFER_LEVEL  0x08000005
#define RENC_UVD_IB_OP_SET_SPEED_ENCODING_MODE   0x08000006
#define RENC_UVD_IB_OP_SET_BALANCE_ENCODING_MODE 0x08000007
#define RENC_UVD_IB_OP_SET_QUALITY_ENCODING_MODE 0x08000008

#define RENC_UVD_IF_MAJOR_VERSION_MASK  0xFFFF0000
#define RENC_UVD_IF_MAJOR_VERSION_SHIFT 16
#define RENC_UVD_IF_MINOR_VERSION_MASK  0x0000FFFF
#define RENC_UVD_IF_MINOR_VERSION_SHIFT 0

#define RENC_UVD_PREENCODE_MODE_NONE 0x00000000
#define RENC_UVD_PREENCODE_MODE_1X   0x00000001
#define RENC_UVD_PREENCODE_MODE_2X   0x00000002
#define RENC_UVD_PREENCODE_MODE_4X   0x00000004

#define RENC_UVD_SLICE_CONTROL_MODE_FIXED_CTBS 0x00000000
#define RENC_UVD_SLICE_CONTROL_MODE_FIXED_BITS 0x00000001

#define RENC_UVD_RATE_CONTROL_METHOD_NONE                    0x00000000
#define RENC_UVD_RATE_CONTROL_METHOD_LATENCY_CONSTRAINED_VBR 0x00000001
#define RENC_UVD_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR    0x00000002
#define RENC_UVD_RATE_CONTROL_METHOD_CBR                     0x00000003

#define RENC_UVD_NALU_TYPE_AUD             0x00000001
#define RENC_UVD_NALU_TYPE_VPS             0x00000002
#define RENC_UVD_NALU_TYPE_SPS             0x00000003
#define RENC_UVD_NALU_TYPE_PPS             0x00000004
#define RENC_UVD_NALU_TYPE_END_OF_SEQUENCE 0x00000005

#define RENC_UVD_SLICE_HEADER_TEMPLATE_MAX_TEMPLATE_SIZE_IN_DWORDS 16
#define RENC_UVD_SLICE_HEADER_TEMPLATE_MAX_NUM_INSTRUCTIONS        16

#define RENC_UVD_HEADER_INSTRUCTION_END                 0
#define RENC_UVD_HEADER_INSTRUCTION_DEPENDENT_SLICE_END 1
#define RENC_UVD_HEADER_INSTRUCTION_COPY                2
#define RENC_UVD_HEADER_INSTRUCTION_FIRST_SLICE         3
#define RENC_UVD_HEADER_INSTRUCTION_SLICE_SEGMENT       4
#define RENC_UVD_HEADER_INSTRUCTION_SLICE_QP_DELTA      5

#define RENC_UVD_PICTURE_TYPE_B      0
#define RENC_UVD_PICTURE_TYPE_P      1
#define RENC_UVD_PICTURE_TYPE_I      2
#define RENC_UVD_PICTURE_TYPE_P_SKIP 3

#define RENC_UVD_SWIZZLE_MODE_LINEAR               0
#define RENC_UVD_SWIZZLE_MODE_256B_D               2
#define RENC_UVD_SWIZZLE_MODE_4kB_D                6
#define RENC_UVD_SWIZZLE_MODE_64kB_D               10
#define RENC_UVD_INTRA_REFRESH_MODE_NONE           0
#define RENC_UVD_INTRA_REFRESH_MODE_CTB_MB_ROWS    1
#define RENC_UVD_INTRA_REFRESH_MODE_CTB_MB_COLUMNS 2

#define RENC_UVD_MAX_NUM_RECONSTRUCTED_PICTURES 34
#define RENC_UVD_ADDR_MODE_LINEAR               0
#define RENC_UVD_ADDR_MODE_PELE_8X8_1D          1
#define RENC_UVD_ADDR_MODE_32AS8_88             2

#define RENC_UVD_ARRAY_MODE_LINEAR         0
#define RENC_UVD_ARRAY_MODE_PELE_8X8_1D    2
#define RENC_UVD_ARRAY_MODE_2D_TILED_THIN1 4

#define RENC_UVD_VIDEO_BITSTREAM_BUFFER_MODE_LINEAR   0
#define RENC_UVD_VIDEO_BITSTREAM_BUFFER_MODE_CIRCULAR 1

#define RENC_UVD_FEEDBACK_BUFFER_MODE_LINEAR   0
#define RENC_UVD_FEEDBACK_BUFFER_MODE_CIRCULAR 1

#define RENC_UVD_FEEDBACK_STATUS_OK          0x00000000
#define RENC_UVD_FEEDBACK_STATUS_NOT_ENCODED 0x10000001

typedef struct radeon_uvd_enc_feedback_s {
   uint32_t task_id;
   uint32_t first_in_task;
   uint32_t last_in_task;
   uint32_t status;
   uint32_t has_bitstream;
   uint32_t bitstream_offset;
   uint32_t bitstream_size;
   uint32_t enabled_filler_data;
   uint32_t filler_data_size;
   uint32_t extra_bytes;
} radeon_uvd_enc_feedback_t;

typedef struct ruvd_enc_session_info_s {
   uint32_t reserved;
   uint32_t interface_version;
   uint32_t sw_context_address_hi;
   uint32_t sw_context_address_lo;
} ruvd_enc_session_info_t;

typedef struct ruvd_enc_task_info_s {
   uint32_t total_size_of_all_packages;
   uint32_t task_id;
   uint32_t allowed_max_num_feedbacks;
} ruvd_enc_task_info_t;

typedef struct ruvd_enc_session_init_s {
   uint32_t aligned_picture_width;
   uint32_t aligned_picture_height;
   uint32_t padding_width;
   uint32_t padding_height;
   uint32_t pre_encode_mode;
   uint32_t pre_encode_chroma_enabled;
} ruvd_enc_session_init_t;

typedef struct ruvd_enc_layer_control_s {
   uint32_t max_num_temporal_layers;
   uint32_t num_temporal_layers;
} ruvd_enc_layer_control_t;

typedef struct ruvd_enc_layer_select_s {
   uint32_t temporal_layer_index;
} ruvd_enc_layer_select_t;

typedef struct ruvd_enc_hevc_slice_control_s {
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
} ruvd_enc_hevc_slice_control_t;

typedef struct ruvd_enc_hevc_spec_misc_s {
   uint32_t log2_min_luma_coding_block_size_minus3;
   uint32_t amp_disabled;
   uint32_t strong_intra_smoothing_enabled;
   uint32_t constrained_intra_pred_flag;
   uint32_t cabac_init_flag;
   uint32_t half_pel_enabled;
   uint32_t quarter_pel_enabled;
} ruvd_enc_hevc_spec_misc_t;

typedef struct ruvd_enc_rate_ctl_session_init_s {
   uint32_t rate_control_method;
   uint32_t vbv_buffer_level;
} ruvd_enc_rate_ctl_session_init_t;

typedef struct ruvd_enc_rate_ctl_layer_init_s {
   uint32_t target_bit_rate;
   uint32_t peak_bit_rate;
   uint32_t frame_rate_num;
   uint32_t frame_rate_den;
   uint32_t vbv_buffer_size;
   uint32_t avg_target_bits_per_picture;
   uint32_t peak_bits_per_picture_integer;
   uint32_t peak_bits_per_picture_fractional;
} ruvd_enc_rate_ctl_layer_init_t;

typedef struct ruvd_enc_rate_ctl_per_picture_s {
   uint32_t qp;
   uint32_t min_qp_app;
   uint32_t max_qp_app;
   uint32_t max_au_size;
   uint32_t enabled_filler_data;
   uint32_t skip_frame_enable;
   uint32_t enforce_hrd;
} ruvd_enc_rate_ctl_per_picture_t;

typedef struct ruvd_enc_quality_params_s {
   uint32_t vbaq_mode;
   uint32_t scene_change_sensitivity;
   uint32_t scene_change_min_idr_interval;
} ruvd_enc_quality_params_t;

typedef struct ruvd_enc_direct_output_nalu_s {
   uint32_t type;
   uint32_t size;
   uint32_t data[1];
} ruvd_enc_direct_output_nalu_t;

typedef struct ruvd_enc_slice_header_s {
   uint32_t bitstream_template[RENC_UVD_SLICE_HEADER_TEMPLATE_MAX_TEMPLATE_SIZE_IN_DWORDS];
   struct {
      uint32_t instruction;
      uint32_t num_bits;
   } instructions[RENC_UVD_SLICE_HEADER_TEMPLATE_MAX_NUM_INSTRUCTIONS];
} ruvd_enc_slice_header_t;

typedef struct ruvd_enc_encode_params_s {
   uint32_t pic_type;
   uint32_t allowed_max_bitstream_size;
   uint32_t input_picture_luma_address_hi;
   uint32_t input_picture_luma_address_lo;
   uint32_t input_picture_chroma_address_hi;
   uint32_t input_picture_chroma_address_lo;
   uint32_t input_pic_luma_pitch;
   uint32_t input_pic_chroma_pitch;
   union {
      uint32_t input_pic_addr_mode;
      uint32_t reserved;
   };
   union {
      uint32_t input_pic_array_mode;
      uint32_t input_pic_swizzle_mode;
   };
   uint32_t reference_picture_index;
   uint32_t reconstructed_picture_index;
} ruvd_enc_encode_params_t;

typedef struct ruvd_enc_hevc_deblocking_filter_s {
   uint32_t loop_filter_across_slices_enabled;
   int32_t deblocking_filter_disabled;
   int32_t beta_offset_div2;
   int32_t tc_offset_div2;
   int32_t cb_qp_offset;
   int32_t cr_qp_offset;
} ruvd_enc_hevc_deblocking_filter_t;

typedef struct ruvd_enc_intra_refresh_s {
   uint32_t intra_refresh_mode;
   uint32_t offset;
   uint32_t region_size;
} ruvd_enc_intra_refresh_t;

typedef struct ruvd_enc_reconstructed_picture_s {
   uint32_t luma_offset;
   uint32_t chroma_offset;
} ruvd_enc_reconstructed_picture_t;

typedef struct ruvd_enc_encode_context_buffer_s {
   uint32_t encode_context_address_hi;
   uint32_t encode_context_address_lo;
   union {
      uint32_t addr_mode;
      uint32_t reserved;
   };
   union {
      uint32_t array_mode;
      uint32_t swizzle_mode;
   };
   uint32_t rec_luma_pitch;
   uint32_t rec_chroma_pitch;
   uint32_t num_reconstructed_pictures;
   ruvd_enc_reconstructed_picture_t reconstructed_pictures[RENC_UVD_MAX_NUM_RECONSTRUCTED_PICTURES];
   uint32_t pre_encode_picture_luma_pitch;
   uint32_t pre_encode_picture_chroma_pitch;
   ruvd_enc_reconstructed_picture_t
      pre_encode_reconstructed_pictures[RENC_UVD_MAX_NUM_RECONSTRUCTED_PICTURES];
   ruvd_enc_reconstructed_picture_t pre_encode_input_picture;
} ruvd_enc_encode_context_buffer_t;

typedef struct ruvd_enc_video_bitstream_buffer_s {
   uint32_t mode;
   uint32_t video_bitstream_buffer_address_hi;
   uint32_t video_bitstream_buffer_address_lo;
   uint32_t video_bitstream_buffer_size;
   uint32_t video_bitstream_data_offset;
} ruvd_enc_video_bitstream_buffer_t;

typedef struct ruvd_enc_feedback_buffer_s {
   uint32_t mode;
   uint32_t feedback_buffer_address_hi;
   uint32_t feedback_buffer_address_lo;
   uint32_t feedback_buffer_size;
   uint32_t feedback_data_size;
} ruvd_enc_feedback_buffer_t;

typedef struct ruvd_enc_vui_info_s
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
} ruvd_enc_vui_info;

typedef void (*radeon_uvd_enc_get_buffer)(struct pipe_resource *resource, struct pb_buffer_lean **handle,
                                          struct radeon_surf **surface);

struct pipe_video_codec *radeon_uvd_create_encoder(struct pipe_context *context,
                                                   const struct pipe_video_codec *templat,
                                                   struct radeon_winsys *ws,
                                                   radeon_uvd_enc_get_buffer get_buffer);

struct radeon_uvd_enc_pic {
   enum pipe_h2645_enc_picture_type picture_type;

   unsigned frame_num;
   unsigned pic_order_cnt;
   unsigned pic_order_cnt_type;
   unsigned crop_left;
   unsigned crop_right;
   unsigned crop_top;
   unsigned crop_bottom;
   unsigned general_tier_flag;
   unsigned general_profile_idc;
   unsigned general_level_idc;
   unsigned max_poc;
   unsigned log2_max_poc;
   unsigned chroma_format_idc;
   unsigned pic_width_in_luma_samples;
   unsigned pic_height_in_luma_samples;
   unsigned log2_diff_max_min_luma_coding_block_size;
   unsigned log2_min_transform_block_size_minus2;
   unsigned log2_diff_max_min_transform_block_size;
   unsigned max_transform_hierarchy_depth_inter;
   unsigned max_transform_hierarchy_depth_intra;
   unsigned log2_parallel_merge_level_minus2;
   unsigned bit_depth_luma_minus8;
   unsigned bit_depth_chroma_minus8;
   unsigned nal_unit_type;
   unsigned max_num_merge_cand;
   ruvd_enc_vui_info vui_info;

   bool not_referenced;
   bool is_iframe;
   bool is_even_frame;
   bool sample_adaptive_offset_enabled_flag;
   bool pcm_enabled_flag;
   bool sps_temporal_mvp_enabled_flag;

   ruvd_enc_task_info_t task_info;
   ruvd_enc_session_init_t session_init;
   ruvd_enc_layer_control_t layer_ctrl;
   ruvd_enc_layer_select_t layer_sel;
   ruvd_enc_hevc_slice_control_t hevc_slice_ctrl;
   ruvd_enc_hevc_spec_misc_t hevc_spec_misc;
   ruvd_enc_rate_ctl_session_init_t rc_session_init;
   ruvd_enc_rate_ctl_layer_init_t rc_layer_init;
   ruvd_enc_hevc_deblocking_filter_t hevc_deblock;
   ruvd_enc_rate_ctl_per_picture_t rc_per_pic;
   ruvd_enc_quality_params_t quality_params;
   ruvd_enc_encode_context_buffer_t ctx_buf;
   ruvd_enc_video_bitstream_buffer_t bit_buf;
   ruvd_enc_feedback_buffer_t fb_buf;
   ruvd_enc_intra_refresh_t intra_ref;
   ruvd_enc_encode_params_t enc_params;
};

struct radeon_uvd_encoder {
   struct pipe_video_codec base;

   void (*begin)(struct radeon_uvd_encoder *enc, struct pipe_picture_desc *pic);
   void (*encode)(struct radeon_uvd_encoder *enc);
   void (*destroy)(struct radeon_uvd_encoder *enc);

   unsigned stream_handle;

   struct pipe_screen *screen;
   struct radeon_winsys *ws;
   struct radeon_cmdbuf cs;

   radeon_uvd_enc_get_buffer get_buffer;

   struct pb_buffer_lean *handle;
   struct radeon_surf *luma;
   struct radeon_surf *chroma;

   struct pb_buffer_lean *bs_handle;
   unsigned bs_size;

   unsigned cpb_num;

   struct rvid_buffer *si;
   struct rvid_buffer *fb;
   struct rvid_buffer cpb;
   struct radeon_uvd_enc_pic enc_pic;

   unsigned shifter;
   unsigned bits_in_shifter;
   unsigned num_zeros;
   unsigned byte_index;
   unsigned bits_output;
   uint32_t total_task_size;
   uint32_t *p_task_size;

   bool emulation_prevention;
   bool need_feedback;
};

struct si_screen;

void radeon_uvd_enc_1_1_init(struct radeon_uvd_encoder *enc);
bool si_radeon_uvd_enc_supported(struct si_screen *sscreen);

#endif // _RADEON_UVD_ENC_H
