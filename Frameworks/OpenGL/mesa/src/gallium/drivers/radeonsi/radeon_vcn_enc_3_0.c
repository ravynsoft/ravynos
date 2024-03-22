/**************************************************************************
 *
 * Copyright 2020 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 *
 **************************************************************************/

#include <stdio.h>

#include "pipe/p_video_codec.h"

#include "util/u_video.h"

#include "si_pipe.h"
#include "radeon_video.h"
#include "radeon_vcn_enc.h"

#define RENCODE_FW_INTERFACE_MAJOR_VERSION   1
#define RENCODE_FW_INTERFACE_MINOR_VERSION   20

static void radeon_enc_session_info(struct radeon_encoder *enc)
{
   RADEON_ENC_BEGIN(enc->cmd.session_info);
   RADEON_ENC_CS(enc->enc_pic.session_info.interface_version);
   RADEON_ENC_READWRITE(enc->si->res->buf, enc->si->res->domains, 0x0);
   RADEON_ENC_CS(0); /* padding 0, not used for vcn3 */
   RADEON_ENC_END();
}

static void radeon_enc_spec_misc(struct radeon_encoder *enc)
{
   enc->enc_pic.spec_misc.constrained_intra_pred_flag = 0;
   enc->enc_pic.spec_misc.half_pel_enabled = 1;
   enc->enc_pic.spec_misc.quarter_pel_enabled = 1;
   enc->enc_pic.spec_misc.level_idc = enc->base.level;
   enc->enc_pic.spec_misc.weighted_bipred_idc = 0;

   RADEON_ENC_BEGIN(enc->cmd.spec_misc_h264);
   RADEON_ENC_CS(enc->enc_pic.spec_misc.constrained_intra_pred_flag);
   RADEON_ENC_CS(enc->enc_pic.spec_misc.cabac_enable);
   RADEON_ENC_CS(enc->enc_pic.spec_misc.cabac_init_idc);
   RADEON_ENC_CS(enc->enc_pic.spec_misc.half_pel_enabled);
   RADEON_ENC_CS(enc->enc_pic.spec_misc.quarter_pel_enabled);
   RADEON_ENC_CS(enc->enc_pic.spec_misc.profile_idc);
   RADEON_ENC_CS(enc->enc_pic.spec_misc.level_idc);
   RADEON_ENC_CS(enc->enc_pic.spec_misc.b_picture_enabled);
   RADEON_ENC_CS(enc->enc_pic.spec_misc.weighted_bipred_idc);
   RADEON_ENC_END();
}

static void radeon_enc_spec_misc_hevc(struct radeon_encoder *enc)
{
   enc->enc_pic.hevc_spec_misc.transform_skip_discarded = 0;
   enc->enc_pic.hevc_spec_misc.cu_qp_delta_enabled_flag = 0;

   RADEON_ENC_BEGIN(enc->cmd.spec_misc_hevc);
   RADEON_ENC_CS(enc->enc_pic.hevc_spec_misc.log2_min_luma_coding_block_size_minus3);
   RADEON_ENC_CS(enc->enc_pic.hevc_spec_misc.amp_disabled);
   RADEON_ENC_CS(enc->enc_pic.hevc_spec_misc.strong_intra_smoothing_enabled);
   RADEON_ENC_CS(enc->enc_pic.hevc_spec_misc.constrained_intra_pred_flag);
   RADEON_ENC_CS(enc->enc_pic.hevc_spec_misc.cabac_init_flag);
   RADEON_ENC_CS(enc->enc_pic.hevc_spec_misc.half_pel_enabled);
   RADEON_ENC_CS(enc->enc_pic.hevc_spec_misc.quarter_pel_enabled);
   RADEON_ENC_CS(enc->enc_pic.hevc_spec_misc.transform_skip_discarded);
   RADEON_ENC_CS(enc->enc_pic.hevc_spec_misc.cu_qp_delta_enabled_flag);
   RADEON_ENC_END();
}

static void radeon_enc_encode_params_h264(struct radeon_encoder *enc)
{
   enc->enc_pic.h264_enc_params.input_picture_structure = RENCODE_H264_PICTURE_STRUCTURE_FRAME;
   enc->enc_pic.h264_enc_params.input_pic_order_cnt = 0;
   enc->enc_pic.h264_enc_params.interlaced_mode = RENCODE_H264_INTERLACING_MODE_PROGRESSIVE;
   enc->enc_pic.h264_enc_params.l0_reference_picture1_index = 0xFFFFFFFF;

   RADEON_ENC_BEGIN(enc->cmd.enc_params_h264);
   RADEON_ENC_CS(enc->enc_pic.h264_enc_params.input_picture_structure);
   RADEON_ENC_CS(enc->enc_pic.h264_enc_params.input_pic_order_cnt);
   RADEON_ENC_CS(enc->enc_pic.h264_enc_params.interlaced_mode);
   RADEON_ENC_CS(enc->enc_pic.h264_enc_params.picture_info_l0_reference_picture0.pic_type);
   RADEON_ENC_CS(enc->enc_pic.h264_enc_params.picture_info_l0_reference_picture0.is_long_term);
   RADEON_ENC_CS(enc->enc_pic.h264_enc_params.picture_info_l0_reference_picture0.picture_structure);
   RADEON_ENC_CS(enc->enc_pic.h264_enc_params.picture_info_l0_reference_picture0.pic_order_cnt);
   RADEON_ENC_CS(enc->enc_pic.h264_enc_params.l0_reference_picture1_index);
   RADEON_ENC_CS(enc->enc_pic.h264_enc_params.picture_info_l0_reference_picture1.pic_type);
   RADEON_ENC_CS(enc->enc_pic.h264_enc_params.picture_info_l0_reference_picture1.is_long_term);
   RADEON_ENC_CS(enc->enc_pic.h264_enc_params.picture_info_l0_reference_picture1.picture_structure);
   RADEON_ENC_CS(enc->enc_pic.h264_enc_params.picture_info_l0_reference_picture1.pic_order_cnt);
   RADEON_ENC_CS(enc->enc_pic.h264_enc_params.l1_reference_picture0_index);
   RADEON_ENC_CS(enc->enc_pic.h264_enc_params.picture_info_l1_reference_picture0.pic_type);
   RADEON_ENC_CS(enc->enc_pic.h264_enc_params.picture_info_l1_reference_picture0.is_long_term);
   RADEON_ENC_CS(enc->enc_pic.h264_enc_params.picture_info_l1_reference_picture0.picture_structure);
   RADEON_ENC_CS(enc->enc_pic.h264_enc_params.picture_info_l1_reference_picture0.pic_order_cnt);
   RADEON_ENC_CS(enc->enc_pic.h264_enc_params.is_reference);
   RADEON_ENC_END();
}

static void radeon_enc_quality_params(struct radeon_encoder *enc)
{
   enc->enc_pic.quality_params.vbaq_mode = enc->enc_pic.quality_modes.vbaq_mode;
   enc->enc_pic.quality_params.scene_change_sensitivity = 0;
   enc->enc_pic.quality_params.scene_change_min_idr_interval = 0;
   enc->enc_pic.quality_params.two_pass_search_center_map_mode =
      (enc->enc_pic.quality_modes.pre_encode_mode &&
       !enc->enc_pic.spec_misc.b_picture_enabled) ? 1 : 0;
   enc->enc_pic.quality_params.vbaq_strength = 0;

   RADEON_ENC_BEGIN(enc->cmd.quality_params);
   RADEON_ENC_CS(enc->enc_pic.quality_params.vbaq_mode);
   RADEON_ENC_CS(enc->enc_pic.quality_params.scene_change_sensitivity);
   RADEON_ENC_CS(enc->enc_pic.quality_params.scene_change_min_idr_interval);
   RADEON_ENC_CS(enc->enc_pic.quality_params.two_pass_search_center_map_mode);
   RADEON_ENC_CS(enc->enc_pic.quality_params.vbaq_strength);
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
   radeon_enc_code_fixed_bits(enc, 0x0, 8); /* constraint_set_flags */
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
      radeon_enc_code_ue(enc, pic->vui_info.max_num_reorder_frames);
      radeon_enc_code_ue(enc, enc->base.max_references); /* max_dec_frame_buffering */
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

   if (enc->enc_pic.picture_type == PIPE_H2645_ENC_PICTURE_TYPE_B)
      radeon_enc_code_fixed_bits(enc, 0x1, 1); /* direct_spatial_mv_pred_flag */

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
      else if (enc->enc_pic.picture_type == PIPE_H2645_ENC_PICTURE_TYPE_B) {
         radeon_enc_code_fixed_bits(enc, 0x0, 1); /* ref_pic_list_modification_flag_l0 */
         radeon_enc_code_fixed_bits(enc, 0x0, 1); /* ref_pic_list_modification_flag_l1 */
      }
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

static void radeon_enc_nalu_pps_hevc(struct radeon_encoder *enc)
{
   uint32_t *size_in_bytes;

   RADEON_ENC_BEGIN(enc->cmd.nalu);
   RADEON_ENC_CS(RENCODE_DIRECT_OUTPUT_NALU_TYPE_PPS);
   size_in_bytes = &enc->cs.current.buf[enc->cs.current.cdw++];

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
   radeon_enc_code_fixed_bits(enc, 0x1, 1);
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

static uint32_t radeon_enc_ref_swizzle_mode(struct radeon_encoder *enc)
{
   /* return RENCODE_REC_SWIZZLE_MODE_LINEAR; for debugging purpose */
   if (enc->enc_pic.bit_depth_luma_minus8 != 0)
      return RENCODE_REC_SWIZZLE_MODE_8x8_1D_THIN_12_24BPP;
   else
      return RENCODE_REC_SWIZZLE_MODE_256B_S;
}

static void radeon_enc_ctx(struct radeon_encoder *enc)
{
   enc->enc_pic.ctx_buf.swizzle_mode = radeon_enc_ref_swizzle_mode(enc);
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

   RADEON_ENC_CS(enc->enc_pic.ctx_buf.colloc_buffer_offset);
   RADEON_ENC_CS(enc->enc_pic.ctx_buf.pre_encode_picture_luma_pitch);
   RADEON_ENC_CS(enc->enc_pic.ctx_buf.pre_encode_picture_chroma_pitch);

   for (int i = 0; i < RENCODE_MAX_NUM_RECONSTRUCTED_PICTURES; i++) {
      RADEON_ENC_CS(enc->enc_pic.ctx_buf.pre_encode_reconstructed_pictures[i].luma_offset);
      RADEON_ENC_CS(enc->enc_pic.ctx_buf.pre_encode_reconstructed_pictures[i].chroma_offset);
   }

   RADEON_ENC_CS(enc->enc_pic.ctx_buf.pre_encode_input_picture.rgb.red_offset);
   RADEON_ENC_CS(enc->enc_pic.ctx_buf.pre_encode_input_picture.rgb.green_offset);
   RADEON_ENC_CS(enc->enc_pic.ctx_buf.pre_encode_input_picture.rgb.blue_offset);

   RADEON_ENC_CS(enc->enc_pic.ctx_buf.two_pass_search_center_map_offset);
   RADEON_ENC_CS(0x00000000);
   RADEON_ENC_CS(0x00000000);
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
   enc->enc_pic.session_init.slice_output_enabled = 0;
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
   RADEON_ENC_CS(enc->enc_pic.session_init.slice_output_enabled);
   RADEON_ENC_CS(enc->enc_pic.session_init.display_remote);
   RADEON_ENC_END();
}

void radeon_enc_3_0_init(struct radeon_encoder *enc)
{
   radeon_enc_2_0_init(enc);

   enc->session_info = radeon_enc_session_info;
   enc->session_init = radeon_enc_session_init;
   enc->ctx = radeon_enc_ctx;
   enc->quality_params = radeon_enc_quality_params;

   if (u_reduce_video_profile(enc->base.profile) == PIPE_VIDEO_FORMAT_MPEG4_AVC) {
      enc->spec_misc = radeon_enc_spec_misc;
      enc->encode_params_codec_spec = radeon_enc_encode_params_h264;
      enc->nalu_sps = radeon_enc_nalu_sps;
      enc->slice_header = radeon_enc_slice_header;
   }

   if (u_reduce_video_profile(enc->base.profile) == PIPE_VIDEO_FORMAT_HEVC) {
      enc->spec_misc = radeon_enc_spec_misc_hevc;
      enc->nalu_pps = radeon_enc_nalu_pps_hevc;
   }

   enc->enc_pic.session_info.interface_version =
      ((RENCODE_FW_INTERFACE_MAJOR_VERSION << RENCODE_IF_MAJOR_VERSION_SHIFT) |
      (RENCODE_FW_INTERFACE_MINOR_VERSION << RENCODE_IF_MINOR_VERSION_SHIFT));
}
