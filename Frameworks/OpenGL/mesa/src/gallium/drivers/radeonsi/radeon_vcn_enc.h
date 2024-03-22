/**************************************************************************
 *
 * Copyright 2017 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 *
 **************************************************************************/

#ifndef _RADEON_VCN_ENC_H
#define _RADEON_VCN_ENC_H

#include "radeon_vcn.h"
#include "util/macros.h"

#include "ac_vcn_enc.h"

#define PIPE_ALIGN_IN_BLOCK_SIZE(value, alignment) DIV_ROUND_UP(value, alignment)

#define RADEON_ENC_CS(value) (enc->cs.current.buf[enc->cs.current.cdw++] = (value))
#define RADEON_ENC_BEGIN(cmd)                                                                    \
   {                                                                                             \
      uint32_t *begin = &enc->cs.current.buf[enc->cs.current.cdw++];                             \
      RADEON_ENC_CS(cmd)
#define RADEON_ENC_READ(buf, domain, off)                                                        \
   radeon_enc_add_buffer(enc, (buf), RADEON_USAGE_READ, (domain), (off))
#define RADEON_ENC_WRITE(buf, domain, off)                                                       \
   radeon_enc_add_buffer(enc, (buf), RADEON_USAGE_WRITE, (domain), (off))
#define RADEON_ENC_READWRITE(buf, domain, off)                                                   \
   radeon_enc_add_buffer(enc, (buf), RADEON_USAGE_READWRITE, (domain), (off))
#define RADEON_ENC_END()                                                                         \
   *begin = (&enc->cs.current.buf[enc->cs.current.cdw] - begin) * 4;                             \
   enc->total_task_size += *begin;                                                               \
   }
#define RADEON_ENC_ADDR_SWAP()                                                                   \
   do {                                                                                          \
      unsigned int *low  = &enc->cs.current.buf[enc->cs.current.cdw - 2];                        \
      unsigned int *high = &enc->cs.current.buf[enc->cs.current.cdw - 1];                        \
      unsigned int temp = *low;                                                                  \
      *low = *high;                                                                              \
      *high = temp;                                                                              \
   } while(0)

#define RADEON_ENC_DESTROY_VIDEO_BUFFER(buf)                                                     \
   do {                                                                                          \
      if (buf) {                                                                                 \
         si_vid_destroy_buffer(buf);                                                             \
         FREE(buf);                                                                              \
         (buf) = NULL;                                                                           \
      }                                                                                          \
   } while(0)

typedef void (*radeon_enc_get_buffer)(struct pipe_resource *resource, struct pb_buffer_lean **handle,
                                      struct radeon_surf **surface);

struct pipe_video_codec *radeon_create_encoder(struct pipe_context *context,
                                               const struct pipe_video_codec *templat,
                                               struct radeon_winsys *ws,
                                               radeon_enc_get_buffer get_buffer);

struct radeon_enc_pic {
   union {
      enum pipe_h2645_enc_picture_type picture_type;
      enum pipe_av1_enc_frame_type frame_type;
   };

   unsigned frame_num;
   unsigned pic_order_cnt;
   unsigned pic_order_cnt_type;
   unsigned ref_idx_l0;
   bool ref_idx_l0_is_ltr;
   unsigned ref_idx_l1;
   bool ref_idx_l1_is_ltr;
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
   unsigned temporal_id;
   unsigned num_temporal_layers;
   unsigned temporal_layer_pattern_index;
   rvcn_enc_quality_modes_t quality_modes;
   rvcn_enc_vui_info vui_info;

   bool not_referenced;
   bool is_ltr;
   unsigned ltr_idx;
   bool is_idr;
   bool need_sequence_header;
   bool is_even_frame;
   bool sample_adaptive_offset_enabled_flag;
   bool pcm_enabled_flag;
   bool sps_temporal_mvp_enabled_flag;

   struct {
      struct {
         struct {
            uint32_t enable_tile_obu:1;
            uint32_t enable_render_size:1;
            uint32_t enable_error_resilient_mode:1;
            uint32_t enable_order_hint:1;
            uint32_t enable_color_description:1;
            uint32_t timing_info_present:1;
            uint32_t timing_info_equal_picture_interval:1;
            uint32_t frame_id_numbers_present:1;
            uint32_t force_integer_mv:1;
            uint32_t disable_screen_content_tools:1;
            uint32_t is_obu_frame:1;
            uint32_t stream_obu_frame:1;  /* all frames have the same number of tiles */
            uint32_t need_av1_seq:1;
         };
         uint32_t render_width;
         uint32_t render_height;
         uint32_t frame_to_show_map_index;
         enum pipe_av1_enc_frame_type last_frame_type;
         uint32_t display_frame_id;
         uint32_t frame_id;
         uint32_t order_hint;
         uint32_t order_hint_bits;
         uint32_t refresh_frame_flags;
         uint32_t reference_delta_frame_id;
         uint32_t reference_frame_index;
         uint32_t reference_order_hint[RENCDOE_AV1_NUM_REF_FRAMES];
         uint32_t *copy_start;
      };
      rvcn_enc_av1_spec_misc_t av1_spec_misc;
      rvcn_enc_av1_cdf_default_table_t av1_cdf_default_table;
      rvcn_enc_av1_timing_info_t av1_timing_info;
      rvcn_enc_av1_color_description_t av1_color_description;
      uint32_t count_last_layer;
      rvcn_enc_av1_ref_frame_t frames[RENCDOE_AV1_NUM_REF_FRAMES];
      rvcn_enc_av1_recon_slot_t recon_slots[RENCDOE_AV1_NUM_REF_FRAMES + 1];
   };

   rvcn_enc_session_info_t session_info;
   rvcn_enc_task_info_t task_info;
   rvcn_enc_session_init_t session_init;
   rvcn_enc_layer_control_t layer_ctrl;
   rvcn_enc_layer_select_t layer_sel;
   rvcn_enc_h264_slice_control_t slice_ctrl;
   rvcn_enc_hevc_slice_control_t hevc_slice_ctrl;
   rvcn_enc_h264_spec_misc_t spec_misc;
   rvcn_enc_hevc_spec_misc_t hevc_spec_misc;
   rvcn_enc_rate_ctl_session_init_t rc_session_init;
   rvcn_enc_rate_ctl_layer_init_t rc_layer_init[RENCODE_MAX_NUM_TEMPORAL_LAYERS];
   rvcn_enc_h264_encode_params_t h264_enc_params;
   rvcn_enc_h264_deblocking_filter_t h264_deblock;
   rvcn_enc_hevc_deblocking_filter_t hevc_deblock;
   rvcn_enc_rate_ctl_per_picture_t rc_per_pic;
   rvcn_enc_quality_params_t quality_params;
   rvcn_enc_encode_context_buffer_t ctx_buf;
   rvcn_enc_video_bitstream_buffer_t bit_buf;
   rvcn_enc_feedback_buffer_t fb_buf;
   rvcn_enc_intra_refresh_t intra_refresh;
   rvcn_enc_encode_params_t enc_params;
   rvcn_enc_stats_t enc_statistics;
   rvcn_enc_input_format_t enc_input_format;
   rvcn_enc_output_format_t enc_output_format;
   rvcn_enc_qp_map_t enc_qp_map;
};

struct radeon_encoder {
   struct pipe_video_codec base;

   void (*begin)(struct radeon_encoder *enc);
   void (*before_encode)(struct radeon_encoder *enc);
   void (*encode)(struct radeon_encoder *enc);
   void (*destroy)(struct radeon_encoder *enc);
   void (*session_info)(struct radeon_encoder *enc);
   void (*task_info)(struct radeon_encoder *enc, bool need_feedback);
   void (*session_init)(struct radeon_encoder *enc);
   void (*layer_control)(struct radeon_encoder *enc);
   void (*layer_select)(struct radeon_encoder *enc);
   void (*slice_control)(struct radeon_encoder *enc);
   void (*spec_misc)(struct radeon_encoder *enc);
   void (*rc_session_init)(struct radeon_encoder *enc);
   void (*rc_layer_init)(struct radeon_encoder *enc);
   void (*deblocking_filter)(struct radeon_encoder *enc);
   void (*quality_params)(struct radeon_encoder *enc);
   void (*nalu_sps)(struct radeon_encoder *enc);
   void (*nalu_pps)(struct radeon_encoder *enc);
   void (*nalu_vps)(struct radeon_encoder *enc);
   void (*nalu_aud)(struct radeon_encoder *enc);
   void (*nalu_sei)(struct radeon_encoder *enc);
   void (*nalu_prefix)(struct radeon_encoder *enc);
   void (*slice_header)(struct radeon_encoder *enc);
   void (*ctx)(struct radeon_encoder *enc);
   void (*bitstream)(struct radeon_encoder *enc);
   void (*feedback)(struct radeon_encoder *enc);
   void (*intra_refresh)(struct radeon_encoder *enc);
   void (*rc_per_pic)(struct radeon_encoder *enc);
   void (*encode_params)(struct radeon_encoder *enc);
   void (*encode_params_codec_spec)(struct radeon_encoder *enc);
   void (*qp_map)(struct radeon_encoder *enc);
   void (*op_init)(struct radeon_encoder *enc);
   void (*op_close)(struct radeon_encoder *enc);
   void (*op_enc)(struct radeon_encoder *enc);
   void (*op_init_rc)(struct radeon_encoder *enc);
   void (*op_init_rc_vbv)(struct radeon_encoder *enc);
   void (*op_preset)(struct radeon_encoder *enc);
   void (*encode_headers)(struct radeon_encoder *enc);
   void (*input_format)(struct radeon_encoder *enc);
   void (*output_format)(struct radeon_encoder *enc);
   void (*encode_statistics)(struct radeon_encoder *enc);
   void (*obu_instructions)(struct radeon_encoder *enc);
   void (*cdf_default_table)(struct radeon_encoder *enc);
   /* mq is used for preversing multiple queue ibs */
   void (*mq_begin)(struct radeon_encoder *enc);
   void (*mq_encode)(struct radeon_encoder *enc);
   void (*mq_destroy)(struct radeon_encoder *enc);

   unsigned stream_handle;

   struct pipe_screen *screen;
   struct radeon_winsys *ws;
   struct radeon_cmdbuf cs;

   radeon_enc_get_buffer get_buffer;

   struct pb_buffer_lean *handle;
   struct radeon_surf *luma;
   struct radeon_surf *chroma;

   struct pb_buffer_lean *bs_handle;
   unsigned bs_size;

   struct rvid_buffer *si;
   struct rvid_buffer *fb;
   struct rvid_buffer *dpb;
   struct rvid_buffer *cdf;
   struct rvid_buffer *roi;
   struct radeon_enc_pic enc_pic;
   struct pb_buffer_lean *stats;
   rvcn_enc_cmd_t cmd;

   unsigned alignment;
   unsigned shifter;
   unsigned bits_in_shifter;
   unsigned num_zeros;
   unsigned byte_index;
   unsigned bits_output;
   unsigned bits_size;
   uint32_t total_task_size;
   uint32_t *p_task_size;
   struct rvcn_sq_var sq;

   bool emulation_prevention;
   bool need_feedback;
   bool need_rate_control;
   unsigned dpb_size;
   unsigned roi_size;
   rvcn_enc_picture_info_t dpb_info[RENCODE_MAX_NUM_RECONSTRUCTED_PICTURES];
   unsigned max_ltr_idx;

   struct pipe_context *ectx;
};

void radeon_enc_add_buffer(struct radeon_encoder *enc, struct pb_buffer_lean *buf,
                           unsigned usage, enum radeon_bo_domain domain, signed offset);

void radeon_enc_dummy(struct radeon_encoder *enc);

void radeon_enc_set_emulation_prevention(struct radeon_encoder *enc, bool set);

void radeon_enc_output_one_byte(struct radeon_encoder *enc, unsigned char byte);

void radeon_enc_emulation_prevention(struct radeon_encoder *enc, unsigned char byte);

void radeon_enc_code_fixed_bits(struct radeon_encoder *enc, unsigned int value,
                                unsigned int num_bits);

void radeon_enc_reset(struct radeon_encoder *enc);

void radeon_enc_byte_align(struct radeon_encoder *enc);

void radeon_enc_flush_headers(struct radeon_encoder *enc);

void radeon_enc_code_ue(struct radeon_encoder *enc, unsigned int value);

void radeon_enc_code_se(struct radeon_encoder *enc, int value);

void radeon_enc_code_uvlc(struct radeon_encoder *enc, unsigned int value);

void radeon_enc_code_leb128(unsigned char *buf, unsigned int value,
                            unsigned int num_bytes);

void radeon_enc_1_2_init(struct radeon_encoder *enc);

void radeon_enc_2_0_init(struct radeon_encoder *enc);

void radeon_enc_3_0_init(struct radeon_encoder *enc);

void radeon_enc_4_0_init(struct radeon_encoder *enc);

void radeon_enc_av1_bs_instruction_type(struct radeon_encoder *enc,
                                        unsigned int inst, unsigned int obu_type);

unsigned char *radeon_enc_av1_header_size_offset(struct radeon_encoder *enc);

unsigned int radeon_enc_value_bits(unsigned int value);

#endif // _RADEON_VCN_ENC_H
