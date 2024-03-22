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
#include "si_pipe.h"
#include "util/u_video.h"

#include <stdio.h>

#define RENCODE_FW_INTERFACE_MAJOR_VERSION         1
#define RENCODE_FW_INTERFACE_MINOR_VERSION         1

#define RENCODE_IB_PARAM_SESSION_INFO              0x00000001
#define RENCODE_IB_PARAM_TASK_INFO                 0x00000002
#define RENCODE_IB_PARAM_SESSION_INIT              0x00000003
#define RENCODE_IB_PARAM_LAYER_CONTROL             0x00000004
#define RENCODE_IB_PARAM_LAYER_SELECT              0x00000005
#define RENCODE_IB_PARAM_RATE_CONTROL_SESSION_INIT 0x00000006
#define RENCODE_IB_PARAM_RATE_CONTROL_LAYER_INIT   0x00000007
#define RENCODE_IB_PARAM_RATE_CONTROL_PER_PICTURE  0x00000008
#define RENCODE_IB_PARAM_QUALITY_PARAMS            0x00000009
#define RENCODE_IB_PARAM_DIRECT_OUTPUT_NALU        0x0000000a
#define RENCODE_IB_PARAM_SLICE_HEADER              0x0000000b
#define RENCODE_IB_PARAM_INPUT_FORMAT              0x0000000c
#define RENCODE_IB_PARAM_OUTPUT_FORMAT             0x0000000d
#define RENCODE_IB_PARAM_ENCODE_PARAMS             0x0000000f
#define RENCODE_IB_PARAM_INTRA_REFRESH             0x00000010
#define RENCODE_IB_PARAM_ENCODE_CONTEXT_BUFFER     0x00000011
#define RENCODE_IB_PARAM_VIDEO_BITSTREAM_BUFFER    0x00000012
#define RENCODE_IB_PARAM_QP_MAP                    0x00000014
#define RENCODE_IB_PARAM_FEEDBACK_BUFFER           0x00000015
#define RENCODE_IB_PARAM_ENCODE_STATISTICS         0x00000019

#define RENCODE_HEVC_IB_PARAM_SLICE_CONTROL        0x00100001
#define RENCODE_HEVC_IB_PARAM_SPEC_MISC            0x00100002
#define RENCODE_HEVC_IB_PARAM_LOOP_FILTER          0x00100003

#define RENCODE_H264_IB_PARAM_SLICE_CONTROL        0x00200001
#define RENCODE_H264_IB_PARAM_SPEC_MISC            0x00200002
#define RENCODE_H264_IB_PARAM_ENCODE_PARAMS        0x00200003
#define RENCODE_H264_IB_PARAM_DEBLOCKING_FILTER    0x00200004

static void radeon_enc_op_preset(struct radeon_encoder *enc)
{
   uint32_t preset_mode;

   if (enc->enc_pic.quality_modes.preset_mode == RENCODE_PRESET_MODE_SPEED &&
         (enc->enc_pic.sample_adaptive_offset_enabled_flag &&
         (u_reduce_video_profile(enc->base.profile) == PIPE_VIDEO_FORMAT_HEVC)))
      preset_mode = RENCODE_IB_OP_SET_BALANCE_ENCODING_MODE;
   else if (enc->enc_pic.quality_modes.preset_mode == RENCODE_PRESET_MODE_QUALITY)
      preset_mode = RENCODE_IB_OP_SET_QUALITY_ENCODING_MODE;
   else if (enc->enc_pic.quality_modes.preset_mode == RENCODE_PRESET_MODE_BALANCE)
      preset_mode = RENCODE_IB_OP_SET_BALANCE_ENCODING_MODE;
   else
      preset_mode = RENCODE_IB_OP_SET_SPEED_ENCODING_MODE;

   RADEON_ENC_BEGIN(preset_mode);
   RADEON_ENC_END();
}

static void radeon_enc_quality_params(struct radeon_encoder *enc)
{
   enc->enc_pic.quality_params.vbaq_mode = enc->enc_pic.quality_modes.vbaq_mode;
   enc->enc_pic.quality_params.scene_change_sensitivity = 0;
   enc->enc_pic.quality_params.scene_change_min_idr_interval = 0;
   enc->enc_pic.quality_params.two_pass_search_center_map_mode =
                    (enc->enc_pic.quality_modes.pre_encode_mode) ? 1 : 0;
   enc->enc_pic.quality_params.vbaq_strength = 0;

   RADEON_ENC_BEGIN(enc->cmd.quality_params);
   RADEON_ENC_CS(enc->enc_pic.quality_params.vbaq_mode);
   RADEON_ENC_CS(enc->enc_pic.quality_params.scene_change_sensitivity);
   RADEON_ENC_CS(enc->enc_pic.quality_params.scene_change_min_idr_interval);
   RADEON_ENC_CS(enc->enc_pic.quality_params.two_pass_search_center_map_mode);
   RADEON_ENC_CS(enc->enc_pic.quality_params.vbaq_strength);
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

   if (enc->enc_pic.sample_adaptive_offset_enabled_flag) {
      radeon_enc_flush_headers(enc);
      instruction[inst_index] = RENCODE_HEADER_INSTRUCTION_COPY;
      num_bits[inst_index] = enc->bits_output - bits_copied;
      bits_copied = enc->bits_output;
      inst_index++;

      instruction[inst_index] = RENCODE_HEVC_HEADER_INSTRUCTION_SAO_ENABLE;
      inst_index++;
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
       (!enc->enc_pic.hevc_deblock.deblocking_filter_disabled ||
        enc->enc_pic.sample_adaptive_offset_enabled_flag)) {
       if (enc->enc_pic.sample_adaptive_offset_enabled_flag) {
           radeon_enc_flush_headers(enc);
           instruction[inst_index] = RENCODE_HEADER_INSTRUCTION_COPY;
           num_bits[inst_index] = enc->bits_output - bits_copied;
           bits_copied = enc->bits_output;
           inst_index++;

           instruction[inst_index] = RENCODE_HEVC_HEADER_INSTRUCTION_LOOP_FILTER_ACROSS_SLICES_ENABLE;
           inst_index++;
       }
       else
           radeon_enc_code_fixed_bits(enc, enc->enc_pic.hevc_deblock.loop_filter_across_slices_enabled, 1);
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

static void radeon_enc_loop_filter_hevc(struct radeon_encoder *enc)
{
   RADEON_ENC_BEGIN(enc->cmd.deblocking_filter_hevc);
   RADEON_ENC_CS(enc->enc_pic.hevc_deblock.loop_filter_across_slices_enabled);
   RADEON_ENC_CS(enc->enc_pic.hevc_deblock.deblocking_filter_disabled);
   RADEON_ENC_CS(enc->enc_pic.hevc_deblock.beta_offset_div2);
   RADEON_ENC_CS(enc->enc_pic.hevc_deblock.tc_offset_div2);
   RADEON_ENC_CS(enc->enc_pic.hevc_deblock.cb_qp_offset);
   RADEON_ENC_CS(enc->enc_pic.hevc_deblock.cr_qp_offset);
   RADEON_ENC_CS(!enc->enc_pic.sample_adaptive_offset_enabled_flag);
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

   if (pic->general_profile_idc == 2)
      radeon_enc_code_fixed_bits(enc, 0x20000000, 32);
   else
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

   /* VUI parameters present flag */
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
   radeon_enc_code_fixed_bits(enc, 0x0, 1);  /* sps extension present flag */
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

static void radeon_enc_input_format(struct radeon_encoder *enc)
{
   RADEON_ENC_BEGIN(enc->cmd.input_format);
   RADEON_ENC_CS(enc->enc_pic.enc_input_format.input_color_volume);
   RADEON_ENC_CS(enc->enc_pic.enc_input_format.input_color_space);
   RADEON_ENC_CS(enc->enc_pic.enc_input_format.input_color_range);
   RADEON_ENC_CS(enc->enc_pic.enc_input_format.input_chroma_subsampling);
   RADEON_ENC_CS(enc->enc_pic.enc_input_format.input_chroma_location);
   RADEON_ENC_CS(enc->enc_pic.enc_input_format.input_color_bit_depth);
   RADEON_ENC_CS(enc->enc_pic.enc_input_format.input_color_packing_format);
   RADEON_ENC_END();
}

static void radeon_enc_output_format(struct radeon_encoder *enc)
{
   RADEON_ENC_BEGIN(enc->cmd.output_format);
   RADEON_ENC_CS(enc->enc_pic.enc_output_format.output_color_volume);
   RADEON_ENC_CS(enc->enc_pic.enc_output_format.output_color_range);
   RADEON_ENC_CS(enc->enc_pic.enc_output_format.output_chroma_location);
   RADEON_ENC_CS(enc->enc_pic.enc_output_format.output_color_bit_depth);
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

   RADEON_ENC_CS(enc->enc_pic.ctx_buf.pre_encode_picture_luma_pitch);
   RADEON_ENC_CS(enc->enc_pic.ctx_buf.pre_encode_picture_chroma_pitch);

   for (int i = 0; i < RENCODE_MAX_NUM_RECONSTRUCTED_PICTURES; i++) {
      RADEON_ENC_CS(enc->enc_pic.ctx_buf.pre_encode_reconstructed_pictures[i].luma_offset);
      RADEON_ENC_CS(enc->enc_pic.ctx_buf.pre_encode_reconstructed_pictures[i].chroma_offset);
   }

   RADEON_ENC_CS(enc->enc_pic.ctx_buf.pre_encode_input_picture.yuv.luma_offset);
   RADEON_ENC_CS(enc->enc_pic.ctx_buf.pre_encode_input_picture.yuv.chroma_offset);
   RADEON_ENC_CS(enc->enc_pic.ctx_buf.two_pass_search_center_map_offset);
   RADEON_ENC_CS(enc->enc_pic.ctx_buf.pre_encode_input_picture.rgb.red_offset);
   RADEON_ENC_CS(enc->enc_pic.ctx_buf.pre_encode_input_picture.rgb.green_offset);
   RADEON_ENC_CS(enc->enc_pic.ctx_buf.pre_encode_input_picture.rgb.blue_offset);

   RADEON_ENC_END();
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
   enc->encode_statistics(enc);
   enc->intra_refresh(enc);
   enc->qp_map(enc);
   enc->input_format(enc);
   enc->output_format(enc);

   enc->op_preset(enc);
   enc->op_enc(enc);
   *enc->p_task_size = (enc->total_task_size);
}

void radeon_enc_2_0_init(struct radeon_encoder *enc)
{
   radeon_enc_1_2_init(enc);
   enc->encode = encode;
   enc->input_format = radeon_enc_input_format;
   enc->output_format = radeon_enc_output_format;
   enc->ctx = radeon_enc_ctx;
   enc->op_preset = radeon_enc_op_preset;
   enc->quality_params = radeon_enc_quality_params;

   if (u_reduce_video_profile(enc->base.profile) == PIPE_VIDEO_FORMAT_HEVC) {
      enc->deblocking_filter = radeon_enc_loop_filter_hevc;
      enc->nalu_sps = radeon_enc_nalu_sps_hevc;
      enc->nalu_pps = radeon_enc_nalu_pps_hevc;
      enc->slice_header = radeon_enc_slice_header_hevc;
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
   enc->cmd.input_format = RENCODE_IB_PARAM_INPUT_FORMAT;
   enc->cmd.output_format = RENCODE_IB_PARAM_OUTPUT_FORMAT;
   enc->cmd.enc_params = RENCODE_IB_PARAM_ENCODE_PARAMS;
   enc->cmd.intra_refresh = RENCODE_IB_PARAM_INTRA_REFRESH;
   enc->cmd.ctx = RENCODE_IB_PARAM_ENCODE_CONTEXT_BUFFER;
   enc->cmd.bitstream = RENCODE_IB_PARAM_VIDEO_BITSTREAM_BUFFER;
   enc->cmd.feedback = RENCODE_IB_PARAM_FEEDBACK_BUFFER;
   enc->cmd.slice_control_hevc = RENCODE_HEVC_IB_PARAM_SLICE_CONTROL;
   enc->cmd.spec_misc_hevc = RENCODE_HEVC_IB_PARAM_SPEC_MISC;
   enc->cmd.deblocking_filter_hevc = RENCODE_HEVC_IB_PARAM_LOOP_FILTER;
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
