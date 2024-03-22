/**************************************************************************
 *
 * Copyright 2017 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 *
 **************************************************************************/

#include "pipe/p_video_codec.h"
#include "radeon_vcn_enc.h"
#include "radeon_video.h"
#include "radeon_temporal.h"
#include "si_pipe.h"
#include "util/u_video.h"

#include <stdio.h>

#define RENCODE_FW_INTERFACE_MAJOR_VERSION 1
#define RENCODE_FW_INTERFACE_MINOR_VERSION 9

#define RENCODE_IB_PARAM_SESSION_INFO              0x00000001
#define RENCODE_IB_PARAM_TASK_INFO                 0x00000002
#define RENCODE_IB_PARAM_SESSION_INIT              0x00000003
#define RENCODE_IB_PARAM_LAYER_CONTROL             0x00000004
#define RENCODE_IB_PARAM_LAYER_SELECT              0x00000005
#define RENCODE_IB_PARAM_RATE_CONTROL_SESSION_INIT 0x00000006
#define RENCODE_IB_PARAM_RATE_CONTROL_LAYER_INIT   0x00000007
#define RENCODE_IB_PARAM_RATE_CONTROL_PER_PICTURE  0x00000008
#define RENCODE_IB_PARAM_QUALITY_PARAMS            0x00000009
#define RENCODE_IB_PARAM_SLICE_HEADER              0x0000000a
#define RENCODE_IB_PARAM_ENCODE_PARAMS             0x0000000b
#define RENCODE_IB_PARAM_INTRA_REFRESH             0x0000000c
#define RENCODE_IB_PARAM_ENCODE_CONTEXT_BUFFER     0x0000000d
#define RENCODE_IB_PARAM_VIDEO_BITSTREAM_BUFFER    0x0000000e
#define RENCODE_IB_PARAM_FEEDBACK_BUFFER           0x00000010
#define RENCODE_IB_PARAM_DIRECT_OUTPUT_NALU        0x00000020
#define RENCODE_IB_PARAM_QP_MAP                    0x00000021
#define RENCODE_IB_PARAM_ENCODE_STATISTICS         0x00000024

#define RENCODE_HEVC_IB_PARAM_SLICE_CONTROL        0x00100001
#define RENCODE_HEVC_IB_PARAM_SPEC_MISC            0x00100002
#define RENCODE_HEVC_IB_PARAM_DEBLOCKING_FILTER    0x00100003

#define RENCODE_H264_IB_PARAM_SLICE_CONTROL        0x00200001
#define RENCODE_H264_IB_PARAM_SPEC_MISC            0x00200002
#define RENCODE_H264_IB_PARAM_ENCODE_PARAMS        0x00200003
#define RENCODE_H264_IB_PARAM_DEBLOCKING_FILTER    0x00200004

static void radeon_enc_session_info(struct radeon_encoder *enc)
{
   RADEON_ENC_BEGIN(enc->cmd.session_info);
   RADEON_ENC_CS(enc->enc_pic.session_info.interface_version);
   RADEON_ENC_READWRITE(enc->si->res->buf, enc->si->res->domains, 0x0);
   RADEON_ENC_CS(RENCODE_ENGINE_TYPE_ENCODE);
   RADEON_ENC_END();
}

static void radeon_enc_task_info(struct radeon_encoder *enc, bool need_feedback)
{
   enc->enc_pic.task_info.task_id++;

   if (need_feedback)
      enc->enc_pic.task_info.allowed_max_num_feedbacks = 1;
   else
      enc->enc_pic.task_info.allowed_max_num_feedbacks = 0;

   RADEON_ENC_BEGIN(enc->cmd.task_info);
   enc->p_task_size = &enc->cs.current.buf[enc->cs.current.cdw++];
   RADEON_ENC_CS(enc->enc_pic.task_info.task_id);
   RADEON_ENC_CS(enc->enc_pic.task_info.allowed_max_num_feedbacks);
   RADEON_ENC_END();
}

static void radeon_enc_session_init(struct radeon_encoder *enc)
{
   if (u_reduce_video_profile(enc->base.profile) == PIPE_VIDEO_FORMAT_MPEG4_AVC) {
      enc->enc_pic.session_init.encode_standard = RENCODE_ENCODE_STANDARD_H264;
      enc->enc_pic.session_init.aligned_picture_width = align(enc->base.width, 16);
   } else if (u_reduce_video_profile(enc->base.profile) == PIPE_VIDEO_FORMAT_HEVC) {
      enc->enc_pic.session_init.encode_standard = RENCODE_ENCODE_STANDARD_HEVC;
      enc->enc_pic.session_init.aligned_picture_width = align(enc->base.width, 64);
   }
   enc->enc_pic.session_init.aligned_picture_height = align(enc->base.height, 16);
   enc->enc_pic.session_init.padding_width =
      enc->enc_pic.session_init.aligned_picture_width - enc->base.width;
   enc->enc_pic.session_init.padding_height =
      enc->enc_pic.session_init.aligned_picture_height - enc->base.height;
   enc->enc_pic.session_init.display_remote = 0;
   enc->enc_pic.session_init.pre_encode_mode = enc->enc_pic.quality_modes.pre_encode_mode;
   enc->enc_pic.session_init.pre_encode_chroma_enabled = !!(enc->enc_pic.quality_modes.pre_encode_mode);

   RADEON_ENC_BEGIN(enc->cmd.session_init);
   RADEON_ENC_CS(enc->enc_pic.session_init.encode_standard);
   RADEON_ENC_CS(enc->enc_pic.session_init.aligned_picture_width);
   RADEON_ENC_CS(enc->enc_pic.session_init.aligned_picture_height);
   RADEON_ENC_CS(enc->enc_pic.session_init.padding_width);
   RADEON_ENC_CS(enc->enc_pic.session_init.padding_height);
   RADEON_ENC_CS(enc->enc_pic.session_init.pre_encode_mode);
   RADEON_ENC_CS(enc->enc_pic.session_init.pre_encode_chroma_enabled);
   RADEON_ENC_CS(enc->enc_pic.session_init.display_remote);
   RADEON_ENC_END();
}

static void radeon_enc_layer_control(struct radeon_encoder *enc)
{
   enc->enc_pic.layer_ctrl.max_num_temporal_layers = enc->enc_pic.num_temporal_layers;
   enc->enc_pic.layer_ctrl.num_temporal_layers = enc->enc_pic.num_temporal_layers;

   RADEON_ENC_BEGIN(enc->cmd.layer_control);
   RADEON_ENC_CS(enc->enc_pic.layer_ctrl.max_num_temporal_layers);
   RADEON_ENC_CS(enc->enc_pic.layer_ctrl.num_temporal_layers);
   RADEON_ENC_END();
}

static void radeon_enc_layer_select(struct radeon_encoder *enc)
{
   RADEON_ENC_BEGIN(enc->cmd.layer_select);
   RADEON_ENC_CS(enc->enc_pic.layer_sel.temporal_layer_index);
   RADEON_ENC_END();
}

static void radeon_enc_slice_control(struct radeon_encoder *enc)
{
   enc->enc_pic.slice_ctrl.slice_control_mode = RENCODE_H264_SLICE_CONTROL_MODE_FIXED_MBS;

   RADEON_ENC_BEGIN(enc->cmd.slice_control_h264);
   RADEON_ENC_CS(enc->enc_pic.slice_ctrl.slice_control_mode);
   RADEON_ENC_CS(enc->enc_pic.slice_ctrl.num_mbs_per_slice);
   RADEON_ENC_END();
}

static void radeon_enc_slice_control_hevc(struct radeon_encoder *enc)
{
   enc->enc_pic.hevc_slice_ctrl.slice_control_mode = RENCODE_HEVC_SLICE_CONTROL_MODE_FIXED_CTBS;

   RADEON_ENC_BEGIN(enc->cmd.slice_control_hevc);
   RADEON_ENC_CS(enc->enc_pic.hevc_slice_ctrl.slice_control_mode);
   RADEON_ENC_CS(enc->enc_pic.hevc_slice_ctrl.fixed_ctbs_per_slice.num_ctbs_per_slice);
   RADEON_ENC_CS(enc->enc_pic.hevc_slice_ctrl.fixed_ctbs_per_slice.num_ctbs_per_slice_segment);
   RADEON_ENC_END();
}

static void radeon_enc_spec_misc(struct radeon_encoder *enc)
{
   enc->enc_pic.spec_misc.constrained_intra_pred_flag = 0;
   enc->enc_pic.spec_misc.half_pel_enabled = 1;
   enc->enc_pic.spec_misc.quarter_pel_enabled = 1;
   enc->enc_pic.spec_misc.level_idc = enc->base.level;

   RADEON_ENC_BEGIN(enc->cmd.spec_misc_h264);
   RADEON_ENC_CS(enc->enc_pic.spec_misc.constrained_intra_pred_flag);
   RADEON_ENC_CS(enc->enc_pic.spec_misc.cabac_enable);
   RADEON_ENC_CS(enc->enc_pic.spec_misc.cabac_init_idc);
   RADEON_ENC_CS(enc->enc_pic.spec_misc.half_pel_enabled);
   RADEON_ENC_CS(enc->enc_pic.spec_misc.quarter_pel_enabled);
   RADEON_ENC_CS(enc->enc_pic.spec_misc.profile_idc);
   RADEON_ENC_CS(enc->enc_pic.spec_misc.level_idc);
   RADEON_ENC_END();
}

static void radeon_enc_spec_misc_hevc(struct radeon_encoder *enc)
{
   RADEON_ENC_BEGIN(enc->cmd.spec_misc_hevc);
   RADEON_ENC_CS(enc->enc_pic.hevc_spec_misc.log2_min_luma_coding_block_size_minus3);
   RADEON_ENC_CS(enc->enc_pic.hevc_spec_misc.amp_disabled);
   RADEON_ENC_CS(enc->enc_pic.hevc_spec_misc.strong_intra_smoothing_enabled);
   RADEON_ENC_CS(enc->enc_pic.hevc_spec_misc.constrained_intra_pred_flag);
   RADEON_ENC_CS(enc->enc_pic.hevc_spec_misc.cabac_init_flag);
   RADEON_ENC_CS(enc->enc_pic.hevc_spec_misc.half_pel_enabled);
   RADEON_ENC_CS(enc->enc_pic.hevc_spec_misc.quarter_pel_enabled);
   RADEON_ENC_END();
}

static void radeon_enc_rc_session_init(struct radeon_encoder *enc)
{
   RADEON_ENC_BEGIN(enc->cmd.rc_session_init);
   RADEON_ENC_CS(enc->enc_pic.rc_session_init.rate_control_method);
   RADEON_ENC_CS(enc->enc_pic.rc_session_init.vbv_buffer_level);
   RADEON_ENC_END();
}

static void radeon_enc_rc_layer_init(struct radeon_encoder *enc)
{
   unsigned int i = enc->enc_pic.layer_sel.temporal_layer_index;
   RADEON_ENC_BEGIN(enc->cmd.rc_layer_init);
   RADEON_ENC_CS(enc->enc_pic.rc_layer_init[i].target_bit_rate);
   RADEON_ENC_CS(enc->enc_pic.rc_layer_init[i].peak_bit_rate);
   RADEON_ENC_CS(enc->enc_pic.rc_layer_init[i].frame_rate_num);
   RADEON_ENC_CS(enc->enc_pic.rc_layer_init[i].frame_rate_den);
   RADEON_ENC_CS(enc->enc_pic.rc_layer_init[i].vbv_buffer_size);
   RADEON_ENC_CS(enc->enc_pic.rc_layer_init[i].avg_target_bits_per_picture);
   RADEON_ENC_CS(enc->enc_pic.rc_layer_init[i].peak_bits_per_picture_integer);
   RADEON_ENC_CS(enc->enc_pic.rc_layer_init[i].peak_bits_per_picture_fractional);
   RADEON_ENC_END();
}

static void radeon_enc_deblocking_filter_h264(struct radeon_encoder *enc)
{
   RADEON_ENC_BEGIN(enc->cmd.deblocking_filter_h264);
   RADEON_ENC_CS(enc->enc_pic.h264_deblock.disable_deblocking_filter_idc);
   RADEON_ENC_CS(enc->enc_pic.h264_deblock.alpha_c0_offset_div2);
   RADEON_ENC_CS(enc->enc_pic.h264_deblock.beta_offset_div2);
   RADEON_ENC_CS(enc->enc_pic.h264_deblock.cb_qp_offset);
   RADEON_ENC_CS(enc->enc_pic.h264_deblock.cr_qp_offset);
   RADEON_ENC_END();
}

static void radeon_enc_deblocking_filter_hevc(struct radeon_encoder *enc)
{
   RADEON_ENC_BEGIN(enc->cmd.deblocking_filter_hevc);
   RADEON_ENC_CS(enc->enc_pic.hevc_deblock.loop_filter_across_slices_enabled);
   RADEON_ENC_CS(enc->enc_pic.hevc_deblock.deblocking_filter_disabled);
   RADEON_ENC_CS(enc->enc_pic.hevc_deblock.beta_offset_div2);
   RADEON_ENC_CS(enc->enc_pic.hevc_deblock.tc_offset_div2);
   RADEON_ENC_CS(enc->enc_pic.hevc_deblock.cb_qp_offset);
   RADEON_ENC_CS(enc->enc_pic.hevc_deblock.cr_qp_offset);
   RADEON_ENC_END();
}

static void radeon_enc_quality_params(struct radeon_encoder *enc)
{
   enc->enc_pic.quality_params.vbaq_mode = enc->enc_pic.quality_modes.vbaq_mode;
   enc->enc_pic.quality_params.scene_change_sensitivity = 0;
   enc->enc_pic.quality_params.scene_change_min_idr_interval = 0;
   enc->enc_pic.quality_params.two_pass_search_center_map_mode =
                    (enc->enc_pic.quality_modes.pre_encode_mode) ? 1 : 0;

   RADEON_ENC_BEGIN(enc->cmd.quality_params);
   RADEON_ENC_CS(enc->enc_pic.quality_params.vbaq_mode);
   RADEON_ENC_CS(enc->enc_pic.quality_params.scene_change_sensitivity);
   RADEON_ENC_CS(enc->enc_pic.quality_params.scene_change_min_idr_interval);
   RADEON_ENC_CS(enc->enc_pic.quality_params.two_pass_search_center_map_mode);
   RADEON_ENC_END();
}

static void radeon_enc_nalu_sps(struct radeon_encoder *enc)
{
   struct radeon_enc_pic *pic = &enc->enc_pic;
   RADEON_ENC_BEGIN(enc->cmd.nalu);
   RADEON_ENC_CS(RENCODE_DIRECT_OUTPUT_NALU_TYPE_SPS);
   uint32_t *size_in_bytes = &enc->cs.current.buf[enc->cs.current.cdw++];
   radeon_enc_reset(enc);
   radeon_enc_set_emulation_prevention(enc, false);
   radeon_enc_code_fixed_bits(enc, 0x00000001, 32);
   radeon_enc_code_fixed_bits(enc, 0x67, 8);
   radeon_enc_byte_align(enc);
   radeon_enc_set_emulation_prevention(enc, true);
   radeon_enc_code_fixed_bits(enc, pic->spec_misc.profile_idc, 8);
   radeon_enc_code_fixed_bits(enc, 0x44, 8); // hardcode to constrained baseline
   radeon_enc_code_fixed_bits(enc, pic->spec_misc.level_idc, 8);
   radeon_enc_code_ue(enc, 0x0);

   if (pic->spec_misc.profile_idc == 100 || pic->spec_misc.profile_idc == 110 ||
       pic->spec_misc.profile_idc == 122 || pic->spec_misc.profile_idc == 244 ||
       pic->spec_misc.profile_idc == 44  || pic->spec_misc.profile_idc == 83 ||
       pic->spec_misc.profile_idc == 86  || pic->spec_misc.profile_idc == 118 ||
       pic->spec_misc.profile_idc == 128 || pic->spec_misc.profile_idc == 138) {
      radeon_enc_code_ue(enc, 0x1);
      radeon_enc_code_ue(enc, 0x0);
      radeon_enc_code_ue(enc, 0x0);
      radeon_enc_code_fixed_bits(enc, 0x0, 2);
   }

   radeon_enc_code_ue(enc, 1);
   radeon_enc_code_ue(enc, pic->pic_order_cnt_type);

   if (pic->pic_order_cnt_type == 0)
      radeon_enc_code_ue(enc, 1);

   radeon_enc_code_ue(enc, enc->base.max_references);
   radeon_enc_code_fixed_bits(enc, pic->layer_ctrl.max_num_temporal_layers > 1 ? 0x1 : 0x0,
                              1);
   radeon_enc_code_ue(enc, (pic->session_init.aligned_picture_width / 16 - 1));
   radeon_enc_code_ue(enc, (pic->session_init.aligned_picture_height / 16 - 1));
   bool progressive_only = true;
   radeon_enc_code_fixed_bits(enc, progressive_only ? 0x1 : 0x0, 1);

   if (!progressive_only)
      radeon_enc_code_fixed_bits(enc, 0x0, 1);

   radeon_enc_code_fixed_bits(enc, 0x1, 1);

   if ((pic->crop_left != 0) || (pic->crop_right  != 0) ||
       (pic->crop_top  != 0) || (pic->crop_bottom != 0)) {
      radeon_enc_code_fixed_bits(enc, 0x1, 1);
      radeon_enc_code_ue(enc, pic->crop_left);
      radeon_enc_code_ue(enc, pic->crop_right);
      radeon_enc_code_ue(enc, pic->crop_top);
      radeon_enc_code_ue(enc, pic->crop_bottom);
   } else
      radeon_enc_code_fixed_bits(enc, 0x0, 1);

   /* VUI present flag */
   radeon_enc_code_fixed_bits(enc, pic->vui_info.vui_parameters_present_flag, 1);
   if (pic->vui_info.vui_parameters_present_flag) {
      /* aspect ratio present flag */
      radeon_enc_code_fixed_bits(enc, (pic->vui_info.flags.aspect_ratio_info_present_flag), 1);
      if (pic->vui_info.flags.aspect_ratio_info_present_flag) {
         radeon_enc_code_fixed_bits(enc, (pic->vui_info.aspect_ratio_idc), 8);
         if (pic->vui_info.aspect_ratio_idc == PIPE_H2645_EXTENDED_SAR) {
            radeon_enc_code_fixed_bits(enc, (pic->vui_info.sar_width), 16);
            radeon_enc_code_fixed_bits(enc, (pic->vui_info.sar_height), 16);
         }
      }
      radeon_enc_code_fixed_bits(enc, 0x0, 1);  /* overscan info present flag */
      /* video signal type present flag  */
      radeon_enc_code_fixed_bits(enc, pic->vui_info.flags.video_signal_type_present_flag, 1);
      if (pic->vui_info.flags.video_signal_type_present_flag) {
         radeon_enc_code_fixed_bits(enc, pic->vui_info.video_format, 3);
         radeon_enc_code_fixed_bits(enc, pic->vui_info.video_full_range_flag, 1);
         radeon_enc_code_fixed_bits(enc, pic->vui_info.flags.colour_description_present_flag, 1);
         if (pic->vui_info.flags.colour_description_present_flag) {
            radeon_enc_code_fixed_bits(enc, pic->vui_info.colour_primaries, 8);
            radeon_enc_code_fixed_bits(enc, pic->vui_info.transfer_characteristics, 8);
            radeon_enc_code_fixed_bits(enc, pic->vui_info.matrix_coefficients, 8);
         }
      }
      /* chroma loc info present flag */
      radeon_enc_code_fixed_bits(enc, pic->vui_info.flags.chroma_loc_info_present_flag, 1);
      if (pic->vui_info.flags.chroma_loc_info_present_flag) {
         radeon_enc_code_ue(enc, pic->vui_info.chroma_sample_loc_type_top_field);
         radeon_enc_code_ue(enc, pic->vui_info.chroma_sample_loc_type_bottom_field);
      }
      /* timing info present flag */
      radeon_enc_code_fixed_bits(enc, (pic->vui_info.flags.timing_info_present_flag), 1);
      if (pic->vui_info.flags.timing_info_present_flag) {
         radeon_enc_code_fixed_bits(enc, (pic->vui_info.num_units_in_tick), 32);
         radeon_enc_code_fixed_bits(enc, (pic->vui_info.time_scale), 32);
         radeon_enc_code_fixed_bits(enc, 0x0, 1);
      }
      radeon_enc_code_fixed_bits(enc, 0x0, 1);  /* nal hrd parameters present flag */
      radeon_enc_code_fixed_bits(enc, 0x0, 1);  /* vcl hrd parameters present flag */
      radeon_enc_code_fixed_bits(enc, 0x0, 1);  /* pic_struct_present flag */
      radeon_enc_code_fixed_bits(enc, 0x1, 1);  /* bitstream_restriction_flag */
      radeon_enc_code_fixed_bits(enc, 0x1, 1);  /* motion_vectors_over_pic_boundaries_flag */
      radeon_enc_code_ue(enc, 0x0);
      radeon_enc_code_ue(enc, 0x0);
      radeon_enc_code_ue(enc, 16);
      radeon_enc_code_ue(enc, 16);
      radeon_enc_code_ue(enc, 0x0);
      radeon_enc_code_ue(enc, enc->base.max_references); /* max_dec_frame_buffering */
   }
   radeon_enc_code_fixed_bits(enc, 0x1, 1);

   radeon_enc_byte_align(enc);
   radeon_enc_flush_headers(enc);
   *size_in_bytes = (enc->bits_output + 7) / 8;
   RADEON_ENC_END();
}

static void radeon_enc_nalu_sps_hevc(struct radeon_encoder *enc)
{
   struct radeon_enc_pic *pic = &enc->enc_pic;
   RADEON_ENC_BEGIN(enc->cmd.nalu);
   RADEON_ENC_CS(RENCODE_DIRECT_OUTPUT_NALU_TYPE_SPS);
   uint32_t *size_in_bytes = &enc->cs.current.buf[enc->cs.current.cdw++];
   int i;

   radeon_enc_reset(enc);
   radeon_enc_set_emulation_prevention(enc, false);
   radeon_enc_code_fixed_bits(enc, 0x00000001, 32);
   radeon_enc_code_fixed_bits(enc, 0x4201, 16);
   radeon_enc_byte_align(enc);
   radeon_enc_set_emulation_prevention(enc, true);
   radeon_enc_code_fixed_bits(enc, 0x0, 4);
   radeon_enc_code_fixed_bits(enc, pic->layer_ctrl.max_num_temporal_layers - 1, 3);
   radeon_enc_code_fixed_bits(enc, 0x1, 1);
   radeon_enc_code_fixed_bits(enc, 0x0, 2);
   radeon_enc_code_fixed_bits(enc, pic->general_tier_flag, 1);
   radeon_enc_code_fixed_bits(enc, pic->general_profile_idc, 5);
   radeon_enc_code_fixed_bits(enc, 0x60000000, 32);
   radeon_enc_code_fixed_bits(enc, 0xb0000000, 32);
   radeon_enc_code_fixed_bits(enc, 0x0, 16);
   radeon_enc_code_fixed_bits(enc, pic->general_level_idc, 8);

   for (i = 0; i < (pic->layer_ctrl.max_num_temporal_layers - 1); i++)
      radeon_enc_code_fixed_bits(enc, 0x0, 2);

   if ((pic->layer_ctrl.max_num_temporal_layers - 1) > 0) {
      for (i = (pic->layer_ctrl.max_num_temporal_layers - 1); i < 8; i++)
         radeon_enc_code_fixed_bits(enc, 0x0, 2);
   }

   radeon_enc_code_ue(enc, 0x0);
   radeon_enc_code_ue(enc, pic->chroma_format_idc);
   radeon_enc_code_ue(enc, pic->session_init.aligned_picture_width);
   radeon_enc_code_ue(enc, pic->session_init.aligned_picture_height);

   if ((pic->crop_left != 0) || (pic->crop_right  != 0) ||
       (pic->crop_top  != 0) || (pic->crop_bottom != 0)) {
      radeon_enc_code_fixed_bits(enc, 0x1, 1);
      radeon_enc_code_ue(enc, pic->crop_left);
      radeon_enc_code_ue(enc, pic->crop_right);
      radeon_enc_code_ue(enc, pic->crop_top);
      radeon_enc_code_ue(enc, pic->crop_bottom);
   } else if (pic->session_init.padding_width  != 0 ||
              pic->session_init.padding_height != 0) {
      radeon_enc_code_fixed_bits(enc, 0x1, 1);
      radeon_enc_code_ue(enc, 0);
      radeon_enc_code_ue(enc, pic->session_init.padding_width / 2);
      radeon_enc_code_ue(enc, 0);
      radeon_enc_code_ue(enc, pic->session_init.padding_height / 2);
   } else
      radeon_enc_code_fixed_bits(enc, 0x0, 1);

   radeon_enc_code_ue(enc, pic->bit_depth_luma_minus8);
   radeon_enc_code_ue(enc, pic->bit_depth_chroma_minus8);
   radeon_enc_code_ue(enc, pic->log2_max_poc - 4);
   radeon_enc_code_fixed_bits(enc, 0x0, 1);
   radeon_enc_code_ue(enc, 1);
   radeon_enc_code_ue(enc, 0x0);
   radeon_enc_code_ue(enc, 0x0);
   radeon_enc_code_ue(enc, pic->hevc_spec_misc.log2_min_luma_coding_block_size_minus3);
   // Only support CTBSize 64
   radeon_enc_code_ue(enc,
                      6 - (pic->hevc_spec_misc.log2_min_luma_coding_block_size_minus3 + 3));
   radeon_enc_code_ue(enc, pic->log2_min_transform_block_size_minus2);
   radeon_enc_code_ue(enc, pic->log2_diff_max_min_transform_block_size);
   radeon_enc_code_ue(enc, pic->max_transform_hierarchy_depth_inter);
   radeon_enc_code_ue(enc, pic->max_transform_hierarchy_depth_intra);

   radeon_enc_code_fixed_bits(enc, 0x0, 1);
   radeon_enc_code_fixed_bits(enc, !pic->hevc_spec_misc.amp_disabled, 1);
   radeon_enc_code_fixed_bits(enc, pic->sample_adaptive_offset_enabled_flag, 1);
   radeon_enc_code_fixed_bits(enc, pic->pcm_enabled_flag, 1);

   radeon_enc_code_ue(enc, 1);
   radeon_enc_code_ue(enc, 1);
   radeon_enc_code_ue(enc, 0);
   radeon_enc_code_ue(enc, 0);
   radeon_enc_code_fixed_bits(enc, 0x1, 1);

   radeon_enc_code_fixed_bits(enc, 0x0, 1);

   radeon_enc_code_fixed_bits(enc, 0, 1);
   radeon_enc_code_fixed_bits(enc, pic->hevc_spec_misc.strong_intra_smoothing_enabled, 1);

   /* VUI parameter present flag */
   radeon_enc_code_fixed_bits(enc, (pic->vui_info.vui_parameters_present_flag), 1);
   if (pic->vui_info.vui_parameters_present_flag) {
      /* aspect ratio present flag */
      radeon_enc_code_fixed_bits(enc, (pic->vui_info.flags.aspect_ratio_info_present_flag), 1);
      if (pic->vui_info.flags.aspect_ratio_info_present_flag) {
         radeon_enc_code_fixed_bits(enc, (pic->vui_info.aspect_ratio_idc), 8);
         if (pic->vui_info.aspect_ratio_idc == PIPE_H2645_EXTENDED_SAR) {
            radeon_enc_code_fixed_bits(enc, (pic->vui_info.sar_width), 16);
            radeon_enc_code_fixed_bits(enc, (pic->vui_info.sar_height), 16);
         }
      }
      radeon_enc_code_fixed_bits(enc, 0x0, 1);  /* overscan info present flag */
      /* video signal type present flag  */
      radeon_enc_code_fixed_bits(enc, pic->vui_info.flags.video_signal_type_present_flag, 1);
      if (pic->vui_info.flags.video_signal_type_present_flag) {
         radeon_enc_code_fixed_bits(enc, pic->vui_info.video_format, 3);
         radeon_enc_code_fixed_bits(enc, pic->vui_info.video_full_range_flag, 1);
         radeon_enc_code_fixed_bits(enc, pic->vui_info.flags.colour_description_present_flag, 1);
         if (pic->vui_info.flags.colour_description_present_flag) {
            radeon_enc_code_fixed_bits(enc, pic->vui_info.colour_primaries, 8);
            radeon_enc_code_fixed_bits(enc, pic->vui_info.transfer_characteristics, 8);
            radeon_enc_code_fixed_bits(enc, pic->vui_info.matrix_coefficients, 8);
         }
      }
      /* chroma loc info present flag */
      radeon_enc_code_fixed_bits(enc, pic->vui_info.flags.chroma_loc_info_present_flag, 1);
      if (pic->vui_info.flags.chroma_loc_info_present_flag) {
         radeon_enc_code_ue(enc, pic->vui_info.chroma_sample_loc_type_top_field);
         radeon_enc_code_ue(enc, pic->vui_info.chroma_sample_loc_type_bottom_field);
      }
      radeon_enc_code_fixed_bits(enc, 0x0, 1);  /* neutral chroma indication flag */
      radeon_enc_code_fixed_bits(enc, 0x0, 1);  /* field seq flag */
      radeon_enc_code_fixed_bits(enc, 0x0, 1);  /* frame field info present flag */
      radeon_enc_code_fixed_bits(enc, 0x0, 1);  /* default display windows flag */
      /* vui timing info present flag */
      radeon_enc_code_fixed_bits(enc, (pic->vui_info.flags.timing_info_present_flag), 1);
      if (pic->vui_info.flags.timing_info_present_flag) {
         radeon_enc_code_fixed_bits(enc, (pic->vui_info.num_units_in_tick), 32);
         radeon_enc_code_fixed_bits(enc, (pic->vui_info.time_scale), 32);
         radeon_enc_code_fixed_bits(enc, 0x0, 1);
         radeon_enc_code_fixed_bits(enc, 0x0, 1);
      }
      radeon_enc_code_fixed_bits(enc, 0x0, 1);  /* bitstream restriction flag */
   }

   radeon_enc_code_fixed_bits(enc, 0x0, 1); /* SPS extension present */
   radeon_enc_code_fixed_bits(enc, 0x1, 1);

   radeon_enc_byte_align(enc);
   radeon_enc_flush_headers(enc);
   *size_in_bytes = (enc->bits_output + 7) / 8;
   RADEON_ENC_END();
}

static void radeon_enc_nalu_prefix(struct radeon_encoder *enc)
{
   uint nalRefIdc = enc->enc_pic.is_idr ? 3 : 0;

   rvcn_temporal_layer_pattern_table_t table_info;
   table_info = rvcn_temporal_layer_pattern_tables[enc->enc_pic.layer_ctrl.num_temporal_layers];

   if (enc->enc_pic.pic_order_cnt == 0)
      enc->enc_pic.temporal_layer_pattern_index = 0;
   else if(enc->enc_pic.temporal_layer_pattern_index == (table_info.pattern_size - 1))
      enc->enc_pic.temporal_layer_pattern_index = 1;
   else
      enc->enc_pic.temporal_layer_pattern_index++;

   rvcn_temporal_layer_pattern_entry_t pattern =
      table_info.pattern_table[enc->enc_pic.temporal_layer_pattern_index];

   RADEON_ENC_BEGIN(enc->cmd.nalu);
   RADEON_ENC_CS(RENCODE_DIRECT_OUTPUT_NALU_TYPE_PREFIX);
   uint32_t *size_in_bytes = &enc->cs.current.buf[enc->cs.current.cdw++];
   radeon_enc_reset(enc);
   radeon_enc_set_emulation_prevention(enc, false);
   radeon_enc_code_fixed_bits(enc, 0x00000001, 32);
   radeon_enc_code_fixed_bits(enc, 0x0, 1);
   radeon_enc_code_fixed_bits(enc, nalRefIdc, 2);
   radeon_enc_code_fixed_bits(enc, 14, 5);
   radeon_enc_byte_align(enc);
   radeon_enc_set_emulation_prevention(enc, true);
   radeon_enc_code_fixed_bits(enc, 0x1, 1);
   radeon_enc_code_fixed_bits(enc, enc->enc_pic.is_idr ? 0x1 : 0x0, 1);
   radeon_enc_code_fixed_bits(enc, 0x0, 6);
   radeon_enc_code_fixed_bits(enc, 0x1, 1);
   radeon_enc_code_fixed_bits(enc, 0x0, 3);
   radeon_enc_code_fixed_bits(enc, 0x0, 4);
   radeon_enc_code_fixed_bits(enc, pattern.temporal_id, 3);
   radeon_enc_code_fixed_bits(enc, 0x0, 1);
   radeon_enc_code_fixed_bits(enc, 0x0, 1);
   radeon_enc_code_fixed_bits(enc, 0x0, 1);
   radeon_enc_code_fixed_bits(enc, 0x3, 2);

   if(nalRefIdc != 0)
   {
      radeon_enc_code_fixed_bits(enc, 0x0, 1);
      radeon_enc_code_fixed_bits(enc, 0x0, 1);
      radeon_enc_code_fixed_bits(enc, 0x1, 1);
      radeon_enc_byte_align(enc);
   }

   radeon_enc_flush_headers(enc);
   *size_in_bytes = (enc->bits_output + 7) / 8;
   RADEON_ENC_END();
}

static void radeon_enc_nalu_sei(struct radeon_encoder *enc)
{
   unsigned number_of_layers;

   rvcn_temporal_layer_pattern_table_t table_info;
   table_info = rvcn_temporal_layer_pattern_tables[enc->enc_pic.layer_ctrl.num_temporal_layers - 1];
   number_of_layers = table_info.pattern_size;

   RADEON_ENC_BEGIN(enc->cmd.nalu);
   RADEON_ENC_CS(RENCODE_DIRECT_OUTPUT_NALU_TYPE_SEI);
   unsigned *size_in_bytes = &enc->cs.current.buf[enc->cs.current.cdw++];
   radeon_enc_reset(enc);
   radeon_enc_set_emulation_prevention(enc, false);

   radeon_enc_code_fixed_bits(enc, 0x00000001, 32);
   radeon_enc_code_fixed_bits(enc, 0x6, 8);
   radeon_enc_byte_align(enc);

   radeon_enc_set_emulation_prevention(enc, true);

   /* save the current position for later */
   unsigned position = enc->cs.current.cdw;
   unsigned shifter = enc->shifter;
   unsigned bits_in_shifter = enc->bits_in_shifter;
   unsigned num_zeros = enc->num_zeros;
   unsigned byte_index = enc->byte_index;
   unsigned bits_output = enc->bits_output;
   bool emulation_prevention = enc->emulation_prevention;

   /* temporarily fill out the payload type and size */
   radeon_enc_code_fixed_bits(enc, 24, 8);
   radeon_enc_code_fixed_bits(enc, 0, 8);

   unsigned svc_start_offset = enc->bits_size;

   radeon_enc_code_fixed_bits(enc, 0x0, 1);
   radeon_enc_code_fixed_bits(enc, 0x0, 1);
   radeon_enc_code_fixed_bits(enc, 0x0, 1);
   radeon_enc_code_ue(enc, number_of_layers - 1);

   for(int i = 0; i < number_of_layers; i++ )
   {
      rvcn_temporal_layer_pattern_entry_t pattern = table_info.pattern_table[i];
      radeon_enc_code_ue(enc, i);
      radeon_enc_code_fixed_bits(enc, 0x0, 6);
      radeon_enc_code_fixed_bits(enc, 0x0, 1);
      radeon_enc_code_fixed_bits(enc, 0x0, 3);
      radeon_enc_code_fixed_bits(enc, 0x0, 4);
      radeon_enc_code_fixed_bits(enc, pattern.temporal_id, 3);
      radeon_enc_code_fixed_bits(enc, 0x0, 1);
      radeon_enc_code_fixed_bits(enc, 0x0, 1);
      radeon_enc_code_fixed_bits(enc, 0x0, 1);
      radeon_enc_code_fixed_bits(enc, 0x0, 1);
      radeon_enc_code_fixed_bits(enc, 0x0, 1);
      radeon_enc_code_fixed_bits(enc, 0x0, 1);
      radeon_enc_code_fixed_bits(enc, 0x0, 1);
      radeon_enc_code_fixed_bits(enc, 0x0, 1);
      radeon_enc_code_fixed_bits(enc, 0x0, 1);
      radeon_enc_code_fixed_bits(enc, 0x0, 1);
      radeon_enc_code_fixed_bits(enc, 0x0, 1);
      radeon_enc_code_fixed_bits(enc, 0x0, 1);
      radeon_enc_code_fixed_bits(enc, 0x0, 1);
      radeon_enc_code_ue(enc, 0);
      radeon_enc_code_ue(enc, 0);
   }
   unsigned svc_size = ((enc->bits_size - svc_start_offset) + 7) / 8;
   unsigned aligned = (32 - enc->bits_in_shifter) % 8;
   if (aligned > 0)
      radeon_enc_code_fixed_bits(enc, 0x1, 1);
   radeon_enc_byte_align(enc);

   radeon_enc_code_fixed_bits(enc, 0x1, 1);
   radeon_enc_byte_align(enc);

   /* store our current state, and go to the beginning to write the size */
   unsigned position2 = enc->cs.current.cdw;
   unsigned shifter2 = enc->shifter;
   unsigned bits_in_shifter2 = enc->bits_in_shifter;
   unsigned num_zeros2 = enc->num_zeros;
   unsigned byte_index2 = enc->byte_index;
   unsigned bits_output2 = enc->bits_output;
   bool emulation_prevention2 = enc->emulation_prevention;

   enc->cs.current.cdw = position;
   enc->shifter = shifter;
   enc->bits_in_shifter = bits_in_shifter;
   enc->num_zeros = num_zeros;
   enc->byte_index = byte_index;
   enc->bits_output = bits_output;
   enc->emulation_prevention = emulation_prevention;

   radeon_enc_output_one_byte(enc, 24);
   radeon_enc_output_one_byte(enc, svc_size);

   /* restore our state */
   enc->cs.current.cdw = position2;
   enc->shifter = shifter2;
   enc->bits_in_shifter = bits_in_shifter2;
   enc->num_zeros = num_zeros2;
   enc->byte_index = byte_index2;
   enc->bits_output = bits_output2;
   enc->emulation_prevention = emulation_prevention2;

   radeon_enc_flush_headers(enc);

   *size_in_bytes = (enc->bits_output + 7) / 8;
   RADEON_ENC_END();
}

static void radeon_enc_nalu_pps(struct radeon_encoder *enc)
{
   RADEON_ENC_BEGIN(enc->cmd.nalu);
   RADEON_ENC_CS(RENCODE_DIRECT_OUTPUT_NALU_TYPE_PPS);
   uint32_t *size_in_bytes = &enc->cs.current.buf[enc->cs.current.cdw++];
   radeon_enc_reset(enc);
   radeon_enc_set_emulation_prevention(enc, false);
   radeon_enc_code_fixed_bits(enc, 0x00000001, 32);
   radeon_enc_code_fixed_bits(enc, 0x68, 8);
   radeon_enc_byte_align(enc);
   radeon_enc_set_emulation_prevention(enc, true);
   radeon_enc_code_ue(enc, 0x0);
   radeon_enc_code_ue(enc, 0x0);
   radeon_enc_code_fixed_bits(enc, (enc->enc_pic.spec_misc.cabac_enable ? 0x1 : 0x0), 1);
   radeon_enc_code_fixed_bits(enc, 0x0, 1); /* bottom_field_pic_order_in_frame_present_flag */
   radeon_enc_code_ue(enc, 0x0); /* num_slice_groups_minus_1 */
   radeon_enc_code_ue(enc, 0x0); /* num_ref_idx_l0_default_active_minus1 */
   radeon_enc_code_ue(enc, 0x0); /* num_ref_idx_l1_default_active_minus1 */
   radeon_enc_code_fixed_bits(enc, 0x0, 1); /* weighted_pred_flag */
   radeon_enc_code_fixed_bits(enc, 0x0, 2); /* weighted_bipred_idc */
   radeon_enc_code_se(enc, 0x0); /* pic_init_qp_minus26 */
   radeon_enc_code_se(enc, 0x0); /* pic_init_qs_minus26 */
   radeon_enc_code_se(enc, enc->enc_pic.h264_deblock.cb_qp_offset); /* chroma_qp_index_offset */
   /* deblocking_filter_control_present_flag */
   radeon_enc_code_fixed_bits(enc, (enc->enc_pic.spec_misc.deblocking_filter_control_present_flag), 1);
   radeon_enc_code_fixed_bits(enc, 0x0, 1); /* constrained_intra_pred_flag */
   /* redundant_pic_cnt_present_flag */
   radeon_enc_code_fixed_bits(enc, (enc->enc_pic.spec_misc.redundant_pic_cnt_present_flag), 1);
   if (enc->enc_pic.spec_misc.redundant_pic_cnt_present_flag) {
      radeon_enc_code_fixed_bits(enc, 0x0, 1); /* transform_8x8_mode_flag */
      radeon_enc_code_fixed_bits(enc, 0x0, 1); /* pic_scaling_matrix_present_flag */
      /* second_chroma_qp_index_offset */
      radeon_enc_code_se(enc, enc->enc_pic.h264_deblock.cr_qp_offset);
   }

   radeon_enc_code_fixed_bits(enc, 0x1, 1);

   radeon_enc_byte_align(enc);
   radeon_enc_flush_headers(enc);
   *size_in_bytes = (enc->bits_output + 7) / 8;
   RADEON_ENC_END();
}

static void radeon_enc_nalu_pps_hevc(struct radeon_encoder *enc)
{
   RADEON_ENC_BEGIN(enc->cmd.nalu);
   RADEON_ENC_CS(RENCODE_DIRECT_OUTPUT_NALU_TYPE_PPS);
   uint32_t *size_in_bytes = &enc->cs.current.buf[enc->cs.current.cdw++];
   radeon_enc_reset(enc);
   radeon_enc_set_emulation_prevention(enc, false);
   radeon_enc_code_fixed_bits(enc, 0x00000001, 32);
   radeon_enc_code_fixed_bits(enc, 0x4401, 16);
   radeon_enc_byte_align(enc);
   radeon_enc_set_emulation_prevention(enc, true);
   radeon_enc_code_ue(enc, 0x0);
   radeon_enc_code_ue(enc, 0x0);
   radeon_enc_code_fixed_bits(enc, 0x1, 1);
   radeon_enc_code_fixed_bits(enc, 0x0, 4);
   radeon_enc_code_fixed_bits(enc, 0x0, 1);
   radeon_enc_code_fixed_bits(enc, 0x1, 1);
   radeon_enc_code_ue(enc, 0x0);
   radeon_enc_code_ue(enc, 0x0);
   radeon_enc_code_se(enc, 0x0);
   radeon_enc_code_fixed_bits(enc, enc->enc_pic.hevc_spec_misc.constrained_intra_pred_flag, 1);
   radeon_enc_code_fixed_bits(enc, 0x0, 1);
   if (enc->enc_pic.rc_session_init.rate_control_method == RENCODE_RATE_CONTROL_METHOD_NONE &&
       enc->enc_pic.enc_qp_map.qp_map_type == RENCODE_QP_MAP_TYPE_NONE)
      radeon_enc_code_fixed_bits(enc, 0x0, 1);
   else {
      radeon_enc_code_fixed_bits(enc, 0x1, 1);
      radeon_enc_code_ue(enc, 0x0);
   }
   radeon_enc_code_se(enc, enc->enc_pic.hevc_deblock.cb_qp_offset);
   radeon_enc_code_se(enc, enc->enc_pic.hevc_deblock.cr_qp_offset);
   radeon_enc_code_fixed_bits(enc, 0x0, 1);
   radeon_enc_code_fixed_bits(enc, 0x0, 2);
   radeon_enc_code_fixed_bits(enc, 0x0, 1);
   radeon_enc_code_fixed_bits(enc, 0x0, 1);
   radeon_enc_code_fixed_bits(enc, 0x0, 1);
   radeon_enc_code_fixed_bits(enc, enc->enc_pic.hevc_deblock.loop_filter_across_slices_enabled, 1);
   radeon_enc_code_fixed_bits(enc, 0x1, 1);
   radeon_enc_code_fixed_bits(enc, 0x0, 1);
   radeon_enc_code_fixed_bits(enc, enc->enc_pic.hevc_deblock.deblocking_filter_disabled, 1);

   if (!enc->enc_pic.hevc_deblock.deblocking_filter_disabled) {
      radeon_enc_code_se(enc, enc->enc_pic.hevc_deblock.beta_offset_div2);
      radeon_enc_code_se(enc, enc->enc_pic.hevc_deblock.tc_offset_div2);
   }

   radeon_enc_code_fixed_bits(enc, 0x0, 1);
   radeon_enc_code_fixed_bits(enc, 0x0, 1);
   radeon_enc_code_ue(enc, enc->enc_pic.log2_parallel_merge_level_minus2);
   radeon_enc_code_fixed_bits(enc, 0x0, 2);

   radeon_enc_code_fixed_bits(enc, 0x1, 1);

   radeon_enc_byte_align(enc);
   radeon_enc_flush_headers(enc);
   *size_in_bytes = (enc->bits_output + 7) / 8;
   RADEON_ENC_END();
}

static void radeon_enc_nalu_vps(struct radeon_encoder *enc)
{
   RADEON_ENC_BEGIN(enc->cmd.nalu);
   RADEON_ENC_CS(RENCODE_DIRECT_OUTPUT_NALU_TYPE_VPS);
   uint32_t *size_in_bytes = &enc->cs.current.buf[enc->cs.current.cdw++];
   int i;

   radeon_enc_reset(enc);
   radeon_enc_set_emulation_prevention(enc, false);
   radeon_enc_code_fixed_bits(enc, 0x00000001, 32);
   radeon_enc_code_fixed_bits(enc, 0x4001, 16);
   radeon_enc_byte_align(enc);
   radeon_enc_set_emulation_prevention(enc, true);

   radeon_enc_code_fixed_bits(enc, 0x0, 4);
   radeon_enc_code_fixed_bits(enc, 0x3, 2);
   radeon_enc_code_fixed_bits(enc, 0x0, 6);
   radeon_enc_code_fixed_bits(enc, enc->enc_pic.layer_ctrl.max_num_temporal_layers - 1, 3);
   radeon_enc_code_fixed_bits(enc, 0x1, 1);
   radeon_enc_code_fixed_bits(enc, 0xffff, 16);
   radeon_enc_code_fixed_bits(enc, 0x0, 2);
   radeon_enc_code_fixed_bits(enc, enc->enc_pic.general_tier_flag, 1);
   radeon_enc_code_fixed_bits(enc, enc->enc_pic.general_profile_idc, 5);
   radeon_enc_code_fixed_bits(enc, 0x60000000, 32);
   radeon_enc_code_fixed_bits(enc, 0xb0000000, 32);
   radeon_enc_code_fixed_bits(enc, 0x0, 16);
   radeon_enc_code_fixed_bits(enc, enc->enc_pic.general_level_idc, 8);

   for (i = 0; i < (enc->enc_pic.layer_ctrl.max_num_temporal_layers - 1); i++)
      radeon_enc_code_fixed_bits(enc, 0x0, 2);

   if ((enc->enc_pic.layer_ctrl.max_num_temporal_layers - 1) > 0) {
      for (i = (enc->enc_pic.layer_ctrl.max_num_temporal_layers - 1); i < 8; i++)
         radeon_enc_code_fixed_bits(enc, 0x0, 2);
   }

   radeon_enc_code_fixed_bits(enc, 0x0, 1);
   radeon_enc_code_ue(enc, 0x1);
   radeon_enc_code_ue(enc, 0x0);
   radeon_enc_code_ue(enc, 0x0);

   radeon_enc_code_fixed_bits(enc, 0x0, 6);
   radeon_enc_code_ue(enc, 0x0);
   radeon_enc_code_fixed_bits(enc, 0x0, 1);
   radeon_enc_code_fixed_bits(enc, 0x0, 1);

   radeon_enc_code_fixed_bits(enc, 0x1, 1);

   radeon_enc_byte_align(enc);
   radeon_enc_flush_headers(enc);
   *size_in_bytes = (enc->bits_output + 7) / 8;
   RADEON_ENC_END();
}

static void radeon_enc_nalu_aud(struct radeon_encoder *enc)
{
   RADEON_ENC_BEGIN(enc->cmd.nalu);
   RADEON_ENC_CS(RENCODE_DIRECT_OUTPUT_NALU_TYPE_AUD);
   uint32_t *size_in_bytes = &enc->cs.current.buf[enc->cs.current.cdw++];
   radeon_enc_reset(enc);
   radeon_enc_set_emulation_prevention(enc, false);
   radeon_enc_code_fixed_bits(enc, 0x00000001, 32);

   if (u_reduce_video_profile(enc->base.profile) == PIPE_VIDEO_FORMAT_MPEG4_AVC)
      radeon_enc_code_fixed_bits(enc, 0x9, 8);
   else if (u_reduce_video_profile(enc->base.profile) == PIPE_VIDEO_FORMAT_HEVC) {
      radeon_enc_code_fixed_bits(enc, 0x0, 1);
      radeon_enc_code_fixed_bits(enc, 35, 6);
      radeon_enc_code_fixed_bits(enc, 0x0, 6);
      radeon_enc_code_fixed_bits(enc, 0x1, 3);
   }
   radeon_enc_byte_align(enc);
   radeon_enc_set_emulation_prevention(enc, true);
   switch (enc->enc_pic.picture_type) {
   case PIPE_H2645_ENC_PICTURE_TYPE_I:
   case PIPE_H2645_ENC_PICTURE_TYPE_IDR:
      radeon_enc_code_fixed_bits(enc, 0x00, 3);
      break;
   case PIPE_H2645_ENC_PICTURE_TYPE_P:
      radeon_enc_code_fixed_bits(enc, 0x01, 3);
      break;
   case PIPE_H2645_ENC_PICTURE_TYPE_B:
      radeon_enc_code_fixed_bits(enc, 0x02, 3);
      break;
   default:
      radeon_enc_code_fixed_bits(enc, 0x02, 3);
   }

   radeon_enc_code_fixed_bits(enc, 0x1, 1);

   radeon_enc_byte_align(enc);
   radeon_enc_flush_headers(enc);
   *size_in_bytes = (enc->bits_output + 7) / 8;
   RADEON_ENC_END();
}

static void radeon_enc_slice_header(struct radeon_encoder *enc)
{
   uint32_t instruction[RENCODE_SLICE_HEADER_TEMPLATE_MAX_NUM_INSTRUCTIONS] = {0};
   uint32_t num_bits[RENCODE_SLICE_HEADER_TEMPLATE_MAX_NUM_INSTRUCTIONS] = {0};
   unsigned int inst_index = 0;
   unsigned int cdw_start = 0;
   unsigned int cdw_filled = 0;
   unsigned int bits_copied = 0;
   RADEON_ENC_BEGIN(enc->cmd.slice_header);
   radeon_enc_reset(enc);
   radeon_enc_set_emulation_prevention(enc, false);

   cdw_start = enc->cs.current.cdw;
   if (enc->enc_pic.is_idr)
      radeon_enc_code_fixed_bits(enc, 0x65, 8);
   else if (enc->enc_pic.not_referenced)
      radeon_enc_code_fixed_bits(enc, 0x01, 8);
   else
      radeon_enc_code_fixed_bits(enc, 0x41, 8);

   radeon_enc_flush_headers(enc);
   instruction[inst_index] = RENCODE_HEADER_INSTRUCTION_COPY;
   num_bits[inst_index] = enc->bits_output - bits_copied;
   bits_copied = enc->bits_output;
   inst_index++;

   instruction[inst_index] = RENCODE_H264_HEADER_INSTRUCTION_FIRST_MB;
   inst_index++;

   switch (enc->enc_pic.picture_type) {
   case PIPE_H2645_ENC_PICTURE_TYPE_I:
   case PIPE_H2645_ENC_PICTURE_TYPE_IDR:
      radeon_enc_code_fixed_bits(enc, 0x08, 7);
      break;
   case PIPE_H2645_ENC_PICTURE_TYPE_P:
   case PIPE_H2645_ENC_PICTURE_TYPE_SKIP:
      radeon_enc_code_fixed_bits(enc, 0x06, 5);
      break;
   case PIPE_H2645_ENC_PICTURE_TYPE_B:
      radeon_enc_code_fixed_bits(enc, 0x07, 5);
      break;
   default:
      radeon_enc_code_fixed_bits(enc, 0x08, 7);
   }

   radeon_enc_code_ue(enc, 0x0);
   radeon_enc_code_fixed_bits(enc, enc->enc_pic.frame_num % 32, 5);

   if (enc->enc_pic.h264_enc_params.input_picture_structure !=
       RENCODE_H264_PICTURE_STRUCTURE_FRAME) {
      radeon_enc_code_fixed_bits(enc, 0x1, 1);
      radeon_enc_code_fixed_bits(enc,
                                 enc->enc_pic.h264_enc_params.input_picture_structure ==
                                       RENCODE_H264_PICTURE_STRUCTURE_BOTTOM_FIELD
                                    ? 1
                                    : 0,
                                 1);
   }

   if (enc->enc_pic.is_idr)
      radeon_enc_code_ue(enc, enc->enc_pic.is_even_frame);

   enc->enc_pic.is_even_frame = !enc->enc_pic.is_even_frame;

   if (enc->enc_pic.pic_order_cnt_type == 0)
      radeon_enc_code_fixed_bits(enc, enc->enc_pic.pic_order_cnt % 32, 5);

   /* ref_pic_list_modification() */
   if (enc->enc_pic.picture_type != PIPE_H2645_ENC_PICTURE_TYPE_IDR &&
       enc->enc_pic.picture_type != PIPE_H2645_ENC_PICTURE_TYPE_I) {
      radeon_enc_code_fixed_bits(enc, 0x0, 1);

      /* long-term reference */
      if (enc->enc_pic.ref_idx_l0_is_ltr) {
         radeon_enc_code_fixed_bits(enc, 0x1, 1);            /* ref_pic_list_modification_flag_l0 */
         radeon_enc_code_ue(enc, 0x2);                       /* modification_of_pic_nums_idc */
         radeon_enc_code_ue(enc, enc->enc_pic.ref_idx_l0);   /* long_term_pic_num */
         radeon_enc_code_ue(enc, 0x3);
      }

      /* short-term reference */
      /* list_mod_diff_pic_minus1 != 0 */
      else if (enc->enc_pic.frame_num - enc->enc_pic.ref_idx_l0 > 1) {
         radeon_enc_code_fixed_bits(enc, 0x1, 1);  /* ref_pic_list_modification_flag_l0 */
         radeon_enc_code_ue(enc, 0x0);             /* modification_of_pic_nums_idc */
         /* abs_diff_pic_num_minus1 */
         radeon_enc_code_ue(enc, (enc->enc_pic.frame_num - enc->enc_pic.ref_idx_l0 - 1));
         radeon_enc_code_ue(enc, 0x3);
      } else
         radeon_enc_code_fixed_bits(enc, 0x0, 1);
   }

   if (enc->enc_pic.is_idr) {
      radeon_enc_code_fixed_bits(enc, 0x0, 1);
      if (enc->enc_pic.is_ltr)
         radeon_enc_code_fixed_bits(enc, 0x1, 1); /* long_term_reference_flag */
      else
         radeon_enc_code_fixed_bits(enc, 0x0, 1);
   } else if (!enc->enc_pic.not_referenced) {
      if (enc->enc_pic.is_ltr) {
         radeon_enc_code_fixed_bits(enc, 0x1, 1);
         radeon_enc_code_ue(enc, 0x4); /* memory_management_control_operation */
         radeon_enc_code_ue(enc, enc->max_ltr_idx + 1); /* max_long_term_frame_idx_plus1 */
         radeon_enc_code_ue(enc, 0x6); /*memory_management_control_operation */
         radeon_enc_code_ue(enc, enc->enc_pic.ltr_idx); /* long_term_frame_idx */
         radeon_enc_code_ue(enc, 0x0); /*memory_management_control_operation end*/
      } else
         radeon_enc_code_fixed_bits(enc, 0x0, 1);
   }

   if ((enc->enc_pic.picture_type != PIPE_H2645_ENC_PICTURE_TYPE_IDR) &&
       (enc->enc_pic.picture_type != PIPE_H2645_ENC_PICTURE_TYPE_I) &&
       (enc->enc_pic.spec_misc.cabac_enable))
      radeon_enc_code_ue(enc, enc->enc_pic.spec_misc.cabac_init_idc);

   radeon_enc_flush_headers(enc);
   instruction[inst_index] = RENCODE_HEADER_INSTRUCTION_COPY;
   num_bits[inst_index] = enc->bits_output - bits_copied;
   bits_copied = enc->bits_output;
   inst_index++;

   instruction[inst_index] = RENCODE_H264_HEADER_INSTRUCTION_SLICE_QP_DELTA;
   inst_index++;

   if (enc->enc_pic.spec_misc.deblocking_filter_control_present_flag) {
      radeon_enc_code_ue(enc, enc->enc_pic.h264_deblock.disable_deblocking_filter_idc);
      if (!enc->enc_pic.h264_deblock.disable_deblocking_filter_idc) {
         radeon_enc_code_se(enc, enc->enc_pic.h264_deblock.alpha_c0_offset_div2);
         radeon_enc_code_se(enc, enc->enc_pic.h264_deblock.beta_offset_div2);
      }
   }

   radeon_enc_flush_headers(enc);
   instruction[inst_index] = RENCODE_HEADER_INSTRUCTION_COPY;
   num_bits[inst_index] = enc->bits_output - bits_copied;
   bits_copied = enc->bits_output;
   inst_index++;

   instruction[inst_index] = RENCODE_HEADER_INSTRUCTION_END;

   cdw_filled = enc->cs.current.cdw - cdw_start;
   for (int i = 0; i < RENCODE_SLICE_HEADER_TEMPLATE_MAX_TEMPLATE_SIZE_IN_DWORDS - cdw_filled; i++)
      RADEON_ENC_CS(0x00000000);

   for (int j = 0; j < RENCODE_SLICE_HEADER_TEMPLATE_MAX_NUM_INSTRUCTIONS; j++) {
      RADEON_ENC_CS(instruction[j]);
      RADEON_ENC_CS(num_bits[j]);
   }

   RADEON_ENC_END();
}

static void radeon_enc_slice_header_hevc(struct radeon_encoder *enc)
{
   uint32_t instruction[RENCODE_SLICE_HEADER_TEMPLATE_MAX_NUM_INSTRUCTIONS] = {0};
   uint32_t num_bits[RENCODE_SLICE_HEADER_TEMPLATE_MAX_NUM_INSTRUCTIONS] = {0};
   unsigned int inst_index = 0;
   unsigned int cdw_start = 0;
   unsigned int cdw_filled = 0;
   unsigned int bits_copied = 0;
   RADEON_ENC_BEGIN(enc->cmd.slice_header);
   radeon_enc_reset(enc);
   radeon_enc_set_emulation_prevention(enc, false);

   cdw_start = enc->cs.current.cdw;
   radeon_enc_code_fixed_bits(enc, 0x0, 1);
   radeon_enc_code_fixed_bits(enc, enc->enc_pic.nal_unit_type, 6);
   radeon_enc_code_fixed_bits(enc, 0x0, 6);
   radeon_enc_code_fixed_bits(enc, 0x1, 3);

   radeon_enc_flush_headers(enc);
   instruction[inst_index] = RENCODE_HEADER_INSTRUCTION_COPY;
   num_bits[inst_index] = enc->bits_output - bits_copied;
   bits_copied = enc->bits_output;
   inst_index++;

   instruction[inst_index] = RENCODE_HEVC_HEADER_INSTRUCTION_FIRST_SLICE;
   inst_index++;

   if ((enc->enc_pic.nal_unit_type >= 16) && (enc->enc_pic.nal_unit_type <= 23))
      radeon_enc_code_fixed_bits(enc, 0x0, 1);

   radeon_enc_code_ue(enc, 0x0);

   radeon_enc_flush_headers(enc);
   instruction[inst_index] = RENCODE_HEADER_INSTRUCTION_COPY;
   num_bits[inst_index] = enc->bits_output - bits_copied;
   bits_copied = enc->bits_output;
   inst_index++;

   instruction[inst_index] = RENCODE_HEVC_HEADER_INSTRUCTION_SLICE_SEGMENT;
   inst_index++;

   instruction[inst_index] = RENCODE_HEVC_HEADER_INSTRUCTION_DEPENDENT_SLICE_END;
   inst_index++;

   switch (enc->enc_pic.picture_type) {
   case PIPE_H2645_ENC_PICTURE_TYPE_I:
   case PIPE_H2645_ENC_PICTURE_TYPE_IDR:
      radeon_enc_code_ue(enc, 0x2);
      break;
   case PIPE_H2645_ENC_PICTURE_TYPE_P:
   case PIPE_H2645_ENC_PICTURE_TYPE_SKIP:
      radeon_enc_code_ue(enc, 0x1);
      break;
   case PIPE_H2645_ENC_PICTURE_TYPE_B:
      radeon_enc_code_ue(enc, 0x0);
      break;
   default:
      radeon_enc_code_ue(enc, 0x1);
   }

   if ((enc->enc_pic.nal_unit_type != 19) && (enc->enc_pic.nal_unit_type != 20)) {
      radeon_enc_code_fixed_bits(enc, enc->enc_pic.pic_order_cnt, enc->enc_pic.log2_max_poc);
      if (enc->enc_pic.picture_type == PIPE_H2645_ENC_PICTURE_TYPE_P)
         radeon_enc_code_fixed_bits(enc, 0x1, 1);
      else {
         radeon_enc_code_fixed_bits(enc, 0x0, 1);
         radeon_enc_code_fixed_bits(enc, 0x0, 1);
         radeon_enc_code_ue(enc, 0x0);
         radeon_enc_code_ue(enc, 0x0);
      }
   }

   if ((enc->enc_pic.picture_type == PIPE_H2645_ENC_PICTURE_TYPE_P) ||
       (enc->enc_pic.picture_type == PIPE_H2645_ENC_PICTURE_TYPE_B)) {
      radeon_enc_code_fixed_bits(enc, 0x0, 1);
      radeon_enc_code_fixed_bits(enc, enc->enc_pic.hevc_spec_misc.cabac_init_flag, 1);
      radeon_enc_code_ue(enc, 5 - enc->enc_pic.max_num_merge_cand);
   }

   radeon_enc_flush_headers(enc);
   instruction[inst_index] = RENCODE_HEADER_INSTRUCTION_COPY;
   num_bits[inst_index] = enc->bits_output - bits_copied;
   bits_copied = enc->bits_output;
   inst_index++;

   instruction[inst_index] = RENCODE_HEVC_HEADER_INSTRUCTION_SLICE_QP_DELTA;
   inst_index++;

   if ((enc->enc_pic.hevc_deblock.loop_filter_across_slices_enabled) &&
       (!enc->enc_pic.hevc_deblock.deblocking_filter_disabled)) {
      radeon_enc_code_fixed_bits(enc, enc->enc_pic.hevc_deblock.loop_filter_across_slices_enabled,
                                 1);

      radeon_enc_flush_headers(enc);
      instruction[inst_index] = RENCODE_HEADER_INSTRUCTION_COPY;
      num_bits[inst_index] = enc->bits_output - bits_copied;
      bits_copied = enc->bits_output;
      inst_index++;
   }

   instruction[inst_index] = RENCODE_HEADER_INSTRUCTION_END;

   cdw_filled = enc->cs.current.cdw - cdw_start;
   for (int i = 0; i < RENCODE_SLICE_HEADER_TEMPLATE_MAX_TEMPLATE_SIZE_IN_DWORDS - cdw_filled; i++)
      RADEON_ENC_CS(0x00000000);

   for (int j = 0; j < RENCODE_SLICE_HEADER_TEMPLATE_MAX_NUM_INSTRUCTIONS; j++) {
      RADEON_ENC_CS(instruction[j]);
      RADEON_ENC_CS(num_bits[j]);
   }

   RADEON_ENC_END();
}

static void radeon_enc_ctx(struct radeon_encoder *enc)
{
   enc->enc_pic.ctx_buf.swizzle_mode = 0;
   enc->enc_pic.ctx_buf.two_pass_search_center_map_offset = 0;

   RADEON_ENC_BEGIN(enc->cmd.ctx);
   RADEON_ENC_READWRITE(enc->dpb->res->buf, enc->dpb->res->domains, 0);
   RADEON_ENC_CS(enc->enc_pic.ctx_buf.swizzle_mode);
   RADEON_ENC_CS(enc->enc_pic.ctx_buf.rec_luma_pitch);
   RADEON_ENC_CS(enc->enc_pic.ctx_buf.rec_chroma_pitch);
   RADEON_ENC_CS(enc->enc_pic.ctx_buf.num_reconstructed_pictures);

   for (int i = 0; i < RENCODE_MAX_NUM_RECONSTRUCTED_PICTURES; i++) {
      RADEON_ENC_CS(enc->enc_pic.ctx_buf.reconstructed_pictures[i].luma_offset);
      RADEON_ENC_CS(enc->enc_pic.ctx_buf.reconstructed_pictures[i].chroma_offset);
   }

   RADEON_ENC_CS(enc->enc_pic.ctx_buf.pre_encode_picture_luma_pitch);
   RADEON_ENC_CS(enc->enc_pic.ctx_buf.pre_encode_picture_chroma_pitch);

   for (int i = 0; i < RENCODE_MAX_NUM_RECONSTRUCTED_PICTURES; i++) {
      RADEON_ENC_CS(enc->enc_pic.ctx_buf.pre_encode_reconstructed_pictures[i].luma_offset);
      RADEON_ENC_CS(enc->enc_pic.ctx_buf.pre_encode_reconstructed_pictures[i].chroma_offset);
   }

   RADEON_ENC_CS(enc->enc_pic.ctx_buf.pre_encode_input_picture.yuv.luma_offset);
   RADEON_ENC_CS(enc->enc_pic.ctx_buf.pre_encode_input_picture.yuv.chroma_offset);

   RADEON_ENC_CS(enc->enc_pic.ctx_buf.two_pass_search_center_map_offset);
   RADEON_ENC_END();
}

static void radeon_enc_bitstream(struct radeon_encoder *enc)
{
   enc->enc_pic.bit_buf.mode = RENCODE_REC_SWIZZLE_MODE_LINEAR;
   enc->enc_pic.bit_buf.video_bitstream_buffer_size = enc->bs_size;
   enc->enc_pic.bit_buf.video_bitstream_data_offset = 0;

   RADEON_ENC_BEGIN(enc->cmd.bitstream);
   RADEON_ENC_CS(enc->enc_pic.bit_buf.mode);
   RADEON_ENC_WRITE(enc->bs_handle, RADEON_DOMAIN_GTT, 0);
   RADEON_ENC_CS(enc->enc_pic.bit_buf.video_bitstream_buffer_size);
   RADEON_ENC_CS(enc->enc_pic.bit_buf.video_bitstream_data_offset);
   RADEON_ENC_END();
}

static void radeon_enc_feedback(struct radeon_encoder *enc)
{
   enc->enc_pic.fb_buf.mode = RENCODE_FEEDBACK_BUFFER_MODE_LINEAR;
   enc->enc_pic.fb_buf.feedback_buffer_size = 16;
   enc->enc_pic.fb_buf.feedback_data_size = 40;

   RADEON_ENC_BEGIN(enc->cmd.feedback);
   RADEON_ENC_CS(enc->enc_pic.fb_buf.mode);
   RADEON_ENC_WRITE(enc->fb->res->buf, enc->fb->res->domains, 0x0);
   RADEON_ENC_CS(enc->enc_pic.fb_buf.feedback_buffer_size);
   RADEON_ENC_CS(enc->enc_pic.fb_buf.feedback_data_size);
   RADEON_ENC_END();
}

static void radeon_enc_intra_refresh(struct radeon_encoder *enc)
{
   RADEON_ENC_BEGIN(enc->cmd.intra_refresh);
   RADEON_ENC_CS(enc->enc_pic.intra_refresh.intra_refresh_mode);
   RADEON_ENC_CS(enc->enc_pic.intra_refresh.offset);
   RADEON_ENC_CS(enc->enc_pic.intra_refresh.region_size);
   RADEON_ENC_END();
}

static void radeon_enc_rc_per_pic(struct radeon_encoder *enc)
{
   RADEON_ENC_BEGIN(enc->cmd.rc_per_pic);
   RADEON_ENC_CS(enc->enc_pic.rc_per_pic.qp);
   RADEON_ENC_CS(enc->enc_pic.rc_per_pic.min_qp_app);
   RADEON_ENC_CS(enc->enc_pic.rc_per_pic.max_qp_app);
   RADEON_ENC_CS(enc->enc_pic.rc_per_pic.max_au_size);
   RADEON_ENC_CS(enc->enc_pic.rc_per_pic.enabled_filler_data);
   RADEON_ENC_CS(enc->enc_pic.rc_per_pic.skip_frame_enable);
   RADEON_ENC_CS(enc->enc_pic.rc_per_pic.enforce_hrd);
   RADEON_ENC_END();
}

static void radeon_enc_encode_params(struct radeon_encoder *enc)
{
   switch (enc->enc_pic.picture_type) {
   case PIPE_H2645_ENC_PICTURE_TYPE_I:
   case PIPE_H2645_ENC_PICTURE_TYPE_IDR:
      enc->enc_pic.enc_params.pic_type = RENCODE_PICTURE_TYPE_I;
      break;
   case PIPE_H2645_ENC_PICTURE_TYPE_P:
      enc->enc_pic.enc_params.pic_type = RENCODE_PICTURE_TYPE_P;
      break;
   case PIPE_H2645_ENC_PICTURE_TYPE_SKIP:
      enc->enc_pic.enc_params.pic_type = RENCODE_PICTURE_TYPE_P_SKIP;
      break;
   case PIPE_H2645_ENC_PICTURE_TYPE_B:
      enc->enc_pic.enc_params.pic_type = RENCODE_PICTURE_TYPE_B;
      break;
   default:
      enc->enc_pic.enc_params.pic_type = RENCODE_PICTURE_TYPE_I;
   }

   if (enc->luma->meta_offset) {
      RVID_ERR("DCC surfaces not supported.\n");
      assert(false);
   }

   enc->enc_pic.enc_params.allowed_max_bitstream_size = enc->bs_size;
   enc->enc_pic.enc_params.input_pic_luma_pitch = enc->luma->u.gfx9.surf_pitch;
   enc->enc_pic.enc_params.input_pic_chroma_pitch = enc->chroma ?
      enc->chroma->u.gfx9.surf_pitch : enc->luma->u.gfx9.surf_pitch;
   enc->enc_pic.enc_params.input_pic_swizzle_mode = enc->luma->u.gfx9.swizzle_mode;

   RADEON_ENC_BEGIN(enc->cmd.enc_params);
   RADEON_ENC_CS(enc->enc_pic.enc_params.pic_type);
   RADEON_ENC_CS(enc->enc_pic.enc_params.allowed_max_bitstream_size);
   RADEON_ENC_READ(enc->handle, RADEON_DOMAIN_VRAM, enc->luma->u.gfx9.surf_offset);
   RADEON_ENC_READ(enc->handle, RADEON_DOMAIN_VRAM, enc->chroma ?
      enc->chroma->u.gfx9.surf_offset : enc->luma->u.gfx9.surf_pitch);
   RADEON_ENC_CS(enc->enc_pic.enc_params.input_pic_luma_pitch);
   RADEON_ENC_CS(enc->enc_pic.enc_params.input_pic_chroma_pitch);
   RADEON_ENC_CS(enc->enc_pic.enc_params.input_pic_swizzle_mode);
   RADEON_ENC_CS(enc->enc_pic.enc_params.reference_picture_index);
   RADEON_ENC_CS(enc->enc_pic.enc_params.reconstructed_picture_index);
   RADEON_ENC_END();
}

static void radeon_enc_encode_params_h264(struct radeon_encoder *enc)
{
   enc->enc_pic.h264_enc_params.input_picture_structure = RENCODE_H264_PICTURE_STRUCTURE_FRAME;
   enc->enc_pic.h264_enc_params.interlaced_mode = RENCODE_H264_INTERLACING_MODE_PROGRESSIVE;
   enc->enc_pic.h264_enc_params.reference_picture_structure = RENCODE_H264_PICTURE_STRUCTURE_FRAME;
   enc->enc_pic.h264_enc_params.reference_picture1_index = 0xFFFFFFFF;

   RADEON_ENC_BEGIN(enc->cmd.enc_params_h264);
   RADEON_ENC_CS(enc->enc_pic.h264_enc_params.input_picture_structure);
   RADEON_ENC_CS(enc->enc_pic.h264_enc_params.interlaced_mode);
   RADEON_ENC_CS(enc->enc_pic.h264_enc_params.reference_picture_structure);
   RADEON_ENC_CS(enc->enc_pic.h264_enc_params.reference_picture1_index);
   RADEON_ENC_END();
}

static void radeon_enc_encode_statistics(struct radeon_encoder *enc)
{
   if (!enc->stats) return;

   enc->enc_pic.enc_statistics.encode_stats_type = RENCODE_STATISTICS_TYPE_0;

   RADEON_ENC_BEGIN(enc->cmd.enc_statistics);
   RADEON_ENC_CS(enc->enc_pic.enc_statistics.encode_stats_type);
   RADEON_ENC_WRITE(enc->stats, RADEON_DOMAIN_GTT, 0);
   RADEON_ENC_END();
}

static void radeon_enc_qp_map(struct radeon_encoder *enc)
{
   if (enc->enc_pic.enc_qp_map.qp_map_type == RENCODE_QP_MAP_TYPE_NONE)
      return;
   enc->enc_pic.enc_qp_map.qp_map_pitch = 0;

   RADEON_ENC_BEGIN(enc->cmd.enc_qp_map);
   RADEON_ENC_CS(enc->enc_pic.enc_qp_map.qp_map_type);
   RADEON_ENC_READWRITE(enc->roi->res->buf, enc->roi->res->domains, 0);
   RADEON_ENC_CS(enc->enc_pic.enc_qp_map.qp_map_pitch);
   RADEON_ENC_END();
}

static void radeon_enc_op_init(struct radeon_encoder *enc)
{
   RADEON_ENC_BEGIN(RENCODE_IB_OP_INITIALIZE);
   RADEON_ENC_END();
}

static void radeon_enc_op_close(struct radeon_encoder *enc)
{
   RADEON_ENC_BEGIN(RENCODE_IB_OP_CLOSE_SESSION);
   RADEON_ENC_END();
}

static void radeon_enc_op_enc(struct radeon_encoder *enc)
{
   RADEON_ENC_BEGIN(RENCODE_IB_OP_ENCODE);
   RADEON_ENC_END();
}

static void radeon_enc_op_init_rc(struct radeon_encoder *enc)
{
   RADEON_ENC_BEGIN(RENCODE_IB_OP_INIT_RC);
   RADEON_ENC_END();
}

static void radeon_enc_op_init_rc_vbv(struct radeon_encoder *enc)
{
   RADEON_ENC_BEGIN(RENCODE_IB_OP_INIT_RC_VBV_BUFFER_LEVEL);
   RADEON_ENC_END();
}

static void radeon_enc_op_preset(struct radeon_encoder *enc)
{
   uint32_t preset_mode;

   if (enc->enc_pic.quality_modes.preset_mode == RENCODE_PRESET_MODE_QUALITY)
      preset_mode = RENCODE_IB_OP_SET_QUALITY_ENCODING_MODE;
   else if (enc->enc_pic.quality_modes.preset_mode == RENCODE_PRESET_MODE_BALANCE)
      preset_mode = RENCODE_IB_OP_SET_BALANCE_ENCODING_MODE;
   else
      preset_mode = RENCODE_IB_OP_SET_SPEED_ENCODING_MODE;

   RADEON_ENC_BEGIN(preset_mode);
   RADEON_ENC_END();
}

static void begin(struct radeon_encoder *enc)
{
   unsigned i;

   enc->session_info(enc);
   enc->total_task_size = 0;
   enc->task_info(enc, enc->need_feedback);
   enc->op_init(enc);

   enc->session_init(enc);
   enc->slice_control(enc);
   enc->spec_misc(enc);
   enc->deblocking_filter(enc);

   enc->layer_control(enc);
   enc->rc_session_init(enc);
   enc->quality_params(enc);

   i = 0;
   do {
      enc->enc_pic.layer_sel.temporal_layer_index = i;
      enc->layer_select(enc);
      enc->rc_layer_init(enc);
      enc->layer_select(enc);
      enc->rc_per_pic(enc);
   } while (++i < enc->enc_pic.num_temporal_layers);

   enc->op_init_rc(enc);
   enc->op_init_rc_vbv(enc);
   *enc->p_task_size = (enc->total_task_size);
}

static void radeon_enc_headers_h264(struct radeon_encoder *enc)
{
   enc->nalu_aud(enc);
   if (enc->enc_pic.layer_ctrl.num_temporal_layers > 1)
      enc->nalu_prefix(enc);
   if (enc->enc_pic.is_idr || enc->enc_pic.need_sequence_header) {
      if (enc->enc_pic.layer_ctrl.num_temporal_layers > 1)
         enc->nalu_sei(enc);
      enc->nalu_sps(enc);
      enc->nalu_pps(enc);
   }
   enc->slice_header(enc);
   enc->encode_params(enc);
   enc->encode_params_codec_spec(enc);
}

static void radeon_enc_headers_hevc(struct radeon_encoder *enc)
{
   enc->nalu_aud(enc);
   if (enc->enc_pic.is_idr || enc->enc_pic.need_sequence_header) {
      enc->nalu_vps(enc);
      enc->nalu_pps(enc);
      enc->nalu_sps(enc);
   }
   enc->slice_header(enc);
   enc->encode_params(enc);
}

static void encode(struct radeon_encoder *enc)
{
   unsigned i;

   enc->before_encode(enc);
   enc->session_info(enc);
   enc->total_task_size = 0;
   enc->task_info(enc, enc->need_feedback);

   if (enc->need_rate_control) {
      i = 0;
      do {
         enc->enc_pic.layer_sel.temporal_layer_index = i;
         enc->layer_select(enc);
         enc->rc_layer_init(enc);
      } while (++i < enc->enc_pic.num_temporal_layers);
   }

   enc->encode_headers(enc);
   enc->ctx(enc);
   enc->bitstream(enc);
   enc->feedback(enc);
   enc->intra_refresh(enc);
   enc->qp_map(enc);

   enc->op_preset(enc);
   enc->op_enc(enc);
   *enc->p_task_size = (enc->total_task_size);
}

static void destroy(struct radeon_encoder *enc)
{
   enc->session_info(enc);
   enc->total_task_size = 0;
   enc->task_info(enc, enc->need_feedback);
   enc->op_close(enc);
   *enc->p_task_size = (enc->total_task_size);
}

static int find_ref_idx(struct radeon_encoder *enc, int pic_num, bool is_ltr)
{
   for (int i = 0; i < enc->base.max_references + 1; i++) {
      if (enc->dpb_info[i].pic_num == pic_num &&
	  enc->dpb_info[i].in_use &&
	  enc->dpb_info[i].is_ltr == is_ltr)
         return i;
   }

   return -1;
}

static int get_picture_storage(struct radeon_encoder *enc)
{
   if (enc->enc_pic.is_ltr) {
      if (enc->enc_pic.is_idr) {
         enc->enc_pic.ltr_idx = 0;
         enc->max_ltr_idx = 0;
      }
      /*
         find ltr with the same ltr_idx to replace
         if this is a new ltr_idx, increase max_ltr_idx and use the normal logic to find slot
      */
     if (enc->enc_pic.ltr_idx <= enc->max_ltr_idx) {
        for (int i = 0; i < enc->base.max_references + 1; i++) {
            if (enc->dpb_info[i].in_use &&
		enc->dpb_info[i].is_ltr &&
		enc->enc_pic.ltr_idx == enc->dpb_info[i].pic_num) {
               enc->dpb_info[i].in_use = false;
               return i;
            }
         }
     } else
        enc->max_ltr_idx = enc->enc_pic.ltr_idx;
   }

   for (int i = 0; i < enc->base.max_references + 1; i++) {
      if (!enc->dpb_info[i].in_use) {
         memset(&(enc->dpb_info[i]), 0, sizeof(rvcn_enc_picture_info_t));
         return i;
      }
   }

   /* look for the oldest short term ref pic */
   unsigned int oldest_frame_num = 0xFFFFFFFF;
   int oldest_idx = -1;
   for (int i = 0; i < enc->base.max_references + 1; i++)
      if (!enc->dpb_info[i].is_ltr && enc->dpb_info[i].pic_num < oldest_frame_num) {
         oldest_frame_num = enc->dpb_info[i].pic_num;
         oldest_idx = i;
      }

   if (oldest_idx >= 0)
      enc->dpb_info[oldest_idx].in_use = false;

   return oldest_idx;
}

static void manage_dpb_before_encode(struct radeon_encoder *enc)
{
   int current_pic_idx = 0;

   if (enc->enc_pic.picture_type == PIPE_H2645_ENC_PICTURE_TYPE_IDR) {
      /* clear reference frames */
      for (int i = 0; i < enc->base.max_references + 1; i++)
         memset(&(enc->dpb_info[i]), 0, sizeof(rvcn_enc_picture_info_t));
   }

   current_pic_idx = get_picture_storage(enc);
   assert(current_pic_idx >= 0);

   int ref0_idx = find_ref_idx(enc, enc->enc_pic.ref_idx_l0, enc->enc_pic.ref_idx_l0_is_ltr);
   /* B-frames only supported on VCN >= 3.0 */
   int ref1_idx = find_ref_idx(enc, enc->enc_pic.ref_idx_l1, enc->enc_pic.ref_idx_l1_is_ltr);

   assert(enc->enc_pic.picture_type != PIPE_H2645_ENC_PICTURE_TYPE_P ||
          ref0_idx != -1);
   assert(enc->enc_pic.picture_type != PIPE_H2645_ENC_PICTURE_TYPE_B ||
          (ref0_idx != -1 && ref1_idx != -1));

   /* In case we didn't find the reference in dpb, we have to pick
    * some valid index to prevent GPU hang. */
   if ((enc->enc_pic.picture_type == PIPE_H2645_ENC_PICTURE_TYPE_P ||
        enc->enc_pic.picture_type == PIPE_H2645_ENC_PICTURE_TYPE_B) &&
       ref0_idx == -1) {
      RVID_ERR("Failed to find ref0 (%u).\n", enc->enc_pic.ref_idx_l0);
      ref0_idx = (current_pic_idx + 1) % (enc->base.max_references + 1);
   }

   if (enc->enc_pic.picture_type == PIPE_H2645_ENC_PICTURE_TYPE_B && ref1_idx == -1) {
      RVID_ERR("Failed to find ref1 (%u).\n", enc->enc_pic.ref_idx_l1);
      ref1_idx = (current_pic_idx + 2) % (enc->base.max_references + 1);
   }

   if (!enc->enc_pic.not_referenced)
      enc->dpb_info[current_pic_idx].in_use = true;

   if (enc->enc_pic.is_ltr) {
      enc->dpb_info[current_pic_idx].pic_num = enc->enc_pic.ltr_idx;
      enc->dpb_info[current_pic_idx].is_ltr = true;
   } else {
      enc->dpb_info[current_pic_idx].pic_num = enc->enc_pic.frame_num;
      enc->dpb_info[current_pic_idx].is_ltr = false;
   }

   if (enc->enc_pic.picture_type == PIPE_H2645_ENC_PICTURE_TYPE_IDR) {
      enc->enc_pic.enc_params.reference_picture_index = 0xFFFFFFFF;
      enc->enc_pic.h264_enc_params.l1_reference_picture0_index = 0xFFFFFFFF;
   } else {
      enc->enc_pic.enc_params.reference_picture_index = ref0_idx;
      enc->enc_pic.h264_enc_params.l1_reference_picture0_index = ref1_idx;
   }
   enc->enc_pic.enc_params.reconstructed_picture_index = current_pic_idx;
   enc->enc_pic.h264_enc_params.is_reference = !enc->enc_pic.not_referenced;
}

void radeon_enc_1_2_init(struct radeon_encoder *enc)
{
   enc->before_encode = manage_dpb_before_encode;
   enc->begin = begin;
   enc->encode = encode;
   enc->destroy = destroy;
   enc->session_info = radeon_enc_session_info;
   enc->task_info = radeon_enc_task_info;
   enc->layer_control = radeon_enc_layer_control;
   enc->layer_select = radeon_enc_layer_select;
   enc->rc_session_init = radeon_enc_rc_session_init;
   enc->rc_layer_init = radeon_enc_rc_layer_init;
   enc->quality_params = radeon_enc_quality_params;
   enc->ctx = radeon_enc_ctx;
   enc->bitstream = radeon_enc_bitstream;
   enc->feedback = radeon_enc_feedback;
   enc->intra_refresh = radeon_enc_intra_refresh;
   enc->rc_per_pic = radeon_enc_rc_per_pic;
   enc->encode_params = radeon_enc_encode_params;
   enc->op_init = radeon_enc_op_init;
   enc->op_close = radeon_enc_op_close;
   enc->op_enc = radeon_enc_op_enc;
   enc->op_init_rc = radeon_enc_op_init_rc;
   enc->op_init_rc_vbv = radeon_enc_op_init_rc_vbv;
   enc->op_preset = radeon_enc_op_preset;
   enc->session_init = radeon_enc_session_init;
   enc->encode_statistics = radeon_enc_encode_statistics;
   enc->nalu_aud = radeon_enc_nalu_aud;
   enc->qp_map = radeon_enc_qp_map;

   if (u_reduce_video_profile(enc->base.profile) == PIPE_VIDEO_FORMAT_MPEG4_AVC) {
      enc->slice_control = radeon_enc_slice_control;
      enc->spec_misc = radeon_enc_spec_misc;
      enc->deblocking_filter = radeon_enc_deblocking_filter_h264;
      enc->nalu_sps = radeon_enc_nalu_sps;
      enc->nalu_pps = radeon_enc_nalu_pps;
      enc->slice_header = radeon_enc_slice_header;
      enc->encode_params_codec_spec = radeon_enc_encode_params_h264;
      enc->encode_headers = radeon_enc_headers_h264;
      enc->nalu_prefix = radeon_enc_nalu_prefix;
      enc->nalu_sei = radeon_enc_nalu_sei;
   } else if (u_reduce_video_profile(enc->base.profile) == PIPE_VIDEO_FORMAT_HEVC) {
      enc->slice_control = radeon_enc_slice_control_hevc;
      enc->spec_misc = radeon_enc_spec_misc_hevc;
      enc->deblocking_filter = radeon_enc_deblocking_filter_hevc;
      enc->nalu_sps = radeon_enc_nalu_sps_hevc;
      enc->nalu_pps = radeon_enc_nalu_pps_hevc;
      enc->nalu_vps = radeon_enc_nalu_vps;
      enc->slice_header = radeon_enc_slice_header_hevc;
      enc->encode_headers = radeon_enc_headers_hevc;
   }

   enc->cmd.session_info = RENCODE_IB_PARAM_SESSION_INFO;
   enc->cmd.task_info = RENCODE_IB_PARAM_TASK_INFO;
   enc->cmd.session_init = RENCODE_IB_PARAM_SESSION_INIT;
   enc->cmd.layer_control = RENCODE_IB_PARAM_LAYER_CONTROL;
   enc->cmd.layer_select = RENCODE_IB_PARAM_LAYER_SELECT;
   enc->cmd.rc_session_init = RENCODE_IB_PARAM_RATE_CONTROL_SESSION_INIT;
   enc->cmd.rc_layer_init = RENCODE_IB_PARAM_RATE_CONTROL_LAYER_INIT;
   enc->cmd.rc_per_pic = RENCODE_IB_PARAM_RATE_CONTROL_PER_PICTURE;
   enc->cmd.quality_params = RENCODE_IB_PARAM_QUALITY_PARAMS;
   enc->cmd.nalu = RENCODE_IB_PARAM_DIRECT_OUTPUT_NALU;
   enc->cmd.slice_header = RENCODE_IB_PARAM_SLICE_HEADER;
   enc->cmd.enc_params = RENCODE_IB_PARAM_ENCODE_PARAMS;
   enc->cmd.intra_refresh = RENCODE_IB_PARAM_INTRA_REFRESH;
   enc->cmd.ctx = RENCODE_IB_PARAM_ENCODE_CONTEXT_BUFFER;
   enc->cmd.bitstream = RENCODE_IB_PARAM_VIDEO_BITSTREAM_BUFFER;
   enc->cmd.feedback = RENCODE_IB_PARAM_FEEDBACK_BUFFER;
   enc->cmd.slice_control_hevc = RENCODE_HEVC_IB_PARAM_SLICE_CONTROL;
   enc->cmd.spec_misc_hevc = RENCODE_HEVC_IB_PARAM_SPEC_MISC;
   enc->cmd.deblocking_filter_hevc = RENCODE_HEVC_IB_PARAM_DEBLOCKING_FILTER;
   enc->cmd.slice_control_h264 = RENCODE_H264_IB_PARAM_SLICE_CONTROL;
   enc->cmd.spec_misc_h264 = RENCODE_H264_IB_PARAM_SPEC_MISC;
   enc->cmd.enc_params_h264 = RENCODE_H264_IB_PARAM_ENCODE_PARAMS;
   enc->cmd.deblocking_filter_h264 = RENCODE_H264_IB_PARAM_DEBLOCKING_FILTER;
   enc->cmd.enc_statistics = RENCODE_IB_PARAM_ENCODE_STATISTICS;
   enc->cmd.enc_qp_map = RENCODE_IB_PARAM_QP_MAP;

   enc->enc_pic.session_info.interface_version =
      ((RENCODE_FW_INTERFACE_MAJOR_VERSION << RENCODE_IF_MAJOR_VERSION_SHIFT) |
       (RENCODE_FW_INTERFACE_MINOR_VERSION << RENCODE_IF_MINOR_VERSION_SHIFT));
}
