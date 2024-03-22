/**************************************************************************
 *
 * Copyright 2018 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 *
 **************************************************************************/

#include "pipe/p_video_codec.h"
#include "radeon_uvd_enc.h"
#include "radeon_video.h"
#include "radeonsi/si_pipe.h"
#include "util/u_memory.h"
#include "util/u_video.h"
#include "vl/vl_video_buffer.h"

#include <stdio.h>

#define RADEON_ENC_CS(value) (enc->cs.current.buf[enc->cs.current.cdw++] = (value))
#define RADEON_ENC_BEGIN(cmd)                                                                      \
   {                                                                                               \
      uint32_t *begin = &enc->cs.current.buf[enc->cs.current.cdw++];                             \
      RADEON_ENC_CS(cmd)
#define RADEON_ENC_READ(buf, domain, off)                                                          \
   radeon_uvd_enc_add_buffer(enc, (buf), RADEON_USAGE_READ, (domain), (off))
#define RADEON_ENC_WRITE(buf, domain, off)                                                         \
   radeon_uvd_enc_add_buffer(enc, (buf), RADEON_USAGE_WRITE, (domain), (off))
#define RADEON_ENC_READWRITE(buf, domain, off)                                                     \
   radeon_uvd_enc_add_buffer(enc, (buf), RADEON_USAGE_READWRITE, (domain), (off))
#define RADEON_ENC_END()                                                                           \
   *begin = (&enc->cs.current.buf[enc->cs.current.cdw] - begin) * 4;                             \
   enc->total_task_size += *begin;                                                                 \
   }

static const unsigned index_to_shifts[4] = {24, 16, 8, 0};

static void radeon_uvd_enc_add_buffer(struct radeon_uvd_encoder *enc, struct pb_buffer_lean *buf,
                                      unsigned usage, enum radeon_bo_domain domain,
                                      signed offset)
{
   enc->ws->cs_add_buffer(&enc->cs, buf, usage | RADEON_USAGE_SYNCHRONIZED, domain);
   uint64_t addr;
   addr = enc->ws->buffer_get_virtual_address(buf);
   addr = addr + offset;
   RADEON_ENC_CS(addr >> 32);
   RADEON_ENC_CS(addr);
}

static void radeon_uvd_enc_set_emulation_prevention(struct radeon_uvd_encoder *enc, bool set)
{
   if (set != enc->emulation_prevention) {
      enc->emulation_prevention = set;
      enc->num_zeros = 0;
   }
}

static void radeon_uvd_enc_output_one_byte(struct radeon_uvd_encoder *enc, unsigned char byte)
{
   if (enc->byte_index == 0)
      enc->cs.current.buf[enc->cs.current.cdw] = 0;
   enc->cs.current.buf[enc->cs.current.cdw] |=
      ((unsigned int)(byte) << index_to_shifts[enc->byte_index]);
   enc->byte_index++;

   if (enc->byte_index >= 4) {
      enc->byte_index = 0;
      enc->cs.current.cdw++;
   }
}

static void radeon_uvd_enc_emulation_prevention(struct radeon_uvd_encoder *enc, unsigned char byte)
{
   if (enc->emulation_prevention) {
      if ((enc->num_zeros >= 2) &&
          ((byte == 0x00) || (byte == 0x01) || (byte == 0x02) || (byte == 0x03))) {
         radeon_uvd_enc_output_one_byte(enc, 0x03);
         enc->bits_output += 8;
         enc->num_zeros = 0;
      }
      enc->num_zeros = (byte == 0 ? (enc->num_zeros + 1) : 0);
   }
}

static void radeon_uvd_enc_code_fixed_bits(struct radeon_uvd_encoder *enc, unsigned int value,
                                           unsigned int num_bits)
{
   unsigned int bits_to_pack = 0;

   while (num_bits > 0) {
      unsigned int value_to_pack = value & (0xffffffff >> (32 - num_bits));
      bits_to_pack =
         num_bits > (32 - enc->bits_in_shifter) ? (32 - enc->bits_in_shifter) : num_bits;

      if (bits_to_pack < num_bits)
         value_to_pack = value_to_pack >> (num_bits - bits_to_pack);

      enc->shifter |= value_to_pack << (32 - enc->bits_in_shifter - bits_to_pack);
      num_bits -= bits_to_pack;
      enc->bits_in_shifter += bits_to_pack;

      while (enc->bits_in_shifter >= 8) {
         unsigned char output_byte = (unsigned char)(enc->shifter >> 24);
         enc->shifter <<= 8;
         radeon_uvd_enc_emulation_prevention(enc, output_byte);
         radeon_uvd_enc_output_one_byte(enc, output_byte);
         enc->bits_in_shifter -= 8;
         enc->bits_output += 8;
      }
   }
}

static void radeon_uvd_enc_reset(struct radeon_uvd_encoder *enc)
{
   enc->emulation_prevention = false;
   enc->shifter = 0;
   enc->bits_in_shifter = 0;
   enc->bits_output = 0;
   enc->num_zeros = 0;
   enc->byte_index = 0;
}

static void radeon_uvd_enc_byte_align(struct radeon_uvd_encoder *enc)
{
   unsigned int num_padding_zeros = (32 - enc->bits_in_shifter) % 8;

   if (num_padding_zeros > 0)
      radeon_uvd_enc_code_fixed_bits(enc, 0, num_padding_zeros);
}

static void radeon_uvd_enc_flush_headers(struct radeon_uvd_encoder *enc)
{
   if (enc->bits_in_shifter != 0) {
      unsigned char output_byte = (unsigned char)(enc->shifter >> 24);
      radeon_uvd_enc_emulation_prevention(enc, output_byte);
      radeon_uvd_enc_output_one_byte(enc, output_byte);
      enc->bits_output += enc->bits_in_shifter;
      enc->shifter = 0;
      enc->bits_in_shifter = 0;
      enc->num_zeros = 0;
   }

   if (enc->byte_index > 0) {
      enc->cs.current.cdw++;
      enc->byte_index = 0;
   }
}

static void radeon_uvd_enc_code_ue(struct radeon_uvd_encoder *enc, unsigned int value)
{
   int x = -1;
   unsigned int ue_code = value + 1;
   value += 1;

   while (value) {
      value = (value >> 1);
      x += 1;
   }

   unsigned int ue_length = (x << 1) + 1;
   radeon_uvd_enc_code_fixed_bits(enc, ue_code, ue_length);
}

static void radeon_uvd_enc_code_se(struct radeon_uvd_encoder *enc, int value)
{
   unsigned int v = 0;

   if (value != 0)
      v = (value < 0 ? ((unsigned int)(0 - value) << 1) : (((unsigned int)(value) << 1) - 1));

   radeon_uvd_enc_code_ue(enc, v);
}

static void radeon_uvd_enc_session_info(struct radeon_uvd_encoder *enc)
{
   unsigned int interface_version =
      ((RENC_UVD_FW_INTERFACE_MAJOR_VERSION << RENC_UVD_IF_MAJOR_VERSION_SHIFT) |
       (RENC_UVD_FW_INTERFACE_MINOR_VERSION << RENC_UVD_IF_MINOR_VERSION_SHIFT));
   RADEON_ENC_BEGIN(RENC_UVD_IB_PARAM_SESSION_INFO);
   RADEON_ENC_CS(0x00000000); // reserved
   RADEON_ENC_CS(interface_version);
   RADEON_ENC_READWRITE(enc->si->res->buf, enc->si->res->domains, 0x0);
   RADEON_ENC_END();
}

static void radeon_uvd_enc_task_info(struct radeon_uvd_encoder *enc, bool need_feedback)
{
   enc->enc_pic.task_info.task_id++;

   if (need_feedback)
      enc->enc_pic.task_info.allowed_max_num_feedbacks = 1;
   else
      enc->enc_pic.task_info.allowed_max_num_feedbacks = 0;

   RADEON_ENC_BEGIN(RENC_UVD_IB_PARAM_TASK_INFO);
   enc->p_task_size = &enc->cs.current.buf[enc->cs.current.cdw++];
   RADEON_ENC_CS(enc->enc_pic.task_info.task_id);
   RADEON_ENC_CS(enc->enc_pic.task_info.allowed_max_num_feedbacks);
   RADEON_ENC_END();
}

static void radeon_uvd_enc_session_init_hevc(struct radeon_uvd_encoder *enc)
{
   enc->enc_pic.session_init.aligned_picture_width = align(enc->base.width, 64);
   enc->enc_pic.session_init.aligned_picture_height = align(enc->base.height, 16);
   enc->enc_pic.session_init.padding_width =
      enc->enc_pic.session_init.aligned_picture_width - enc->base.width;
   enc->enc_pic.session_init.padding_height =
      enc->enc_pic.session_init.aligned_picture_height - enc->base.height;
   enc->enc_pic.session_init.pre_encode_mode = RENC_UVD_PREENCODE_MODE_NONE;
   enc->enc_pic.session_init.pre_encode_chroma_enabled = false;

   RADEON_ENC_BEGIN(RENC_UVD_IB_PARAM_SESSION_INIT);
   RADEON_ENC_CS(enc->enc_pic.session_init.aligned_picture_width);
   RADEON_ENC_CS(enc->enc_pic.session_init.aligned_picture_height);
   RADEON_ENC_CS(enc->enc_pic.session_init.padding_width);
   RADEON_ENC_CS(enc->enc_pic.session_init.padding_height);
   RADEON_ENC_CS(enc->enc_pic.session_init.pre_encode_mode);
   RADEON_ENC_CS(enc->enc_pic.session_init.pre_encode_chroma_enabled);
   RADEON_ENC_END();
}

static void radeon_uvd_enc_layer_control(struct radeon_uvd_encoder *enc)
{
   enc->enc_pic.layer_ctrl.max_num_temporal_layers = 1;
   enc->enc_pic.layer_ctrl.num_temporal_layers = 1;

   RADEON_ENC_BEGIN(RENC_UVD_IB_PARAM_LAYER_CONTROL);
   RADEON_ENC_CS(enc->enc_pic.layer_ctrl.max_num_temporal_layers);
   RADEON_ENC_CS(enc->enc_pic.layer_ctrl.num_temporal_layers);
   RADEON_ENC_END();
}

static void radeon_uvd_enc_layer_select(struct radeon_uvd_encoder *enc)
{
   enc->enc_pic.layer_sel.temporal_layer_index = 0;

   RADEON_ENC_BEGIN(RENC_UVD_IB_PARAM_LAYER_SELECT);
   RADEON_ENC_CS(enc->enc_pic.layer_sel.temporal_layer_index);
   RADEON_ENC_END();
}

static void radeon_uvd_enc_slice_control_hevc(struct radeon_uvd_encoder *enc)
{
   enc->enc_pic.hevc_slice_ctrl.slice_control_mode = RENC_UVD_SLICE_CONTROL_MODE_FIXED_CTBS;
   enc->enc_pic.hevc_slice_ctrl.fixed_ctbs_per_slice.num_ctbs_per_slice =
      align(enc->base.width, 64) / 64 * align(enc->base.height, 64) / 64;
   enc->enc_pic.hevc_slice_ctrl.fixed_ctbs_per_slice.num_ctbs_per_slice_segment =
      enc->enc_pic.hevc_slice_ctrl.fixed_ctbs_per_slice.num_ctbs_per_slice;

   RADEON_ENC_BEGIN(RENC_UVD_IB_PARAM_SLICE_CONTROL);
   RADEON_ENC_CS(enc->enc_pic.hevc_slice_ctrl.slice_control_mode);
   RADEON_ENC_CS(enc->enc_pic.hevc_slice_ctrl.fixed_ctbs_per_slice.num_ctbs_per_slice);
   RADEON_ENC_CS(enc->enc_pic.hevc_slice_ctrl.fixed_ctbs_per_slice.num_ctbs_per_slice_segment);
   RADEON_ENC_END();
}

static void radeon_uvd_enc_spec_misc_hevc(struct radeon_uvd_encoder *enc,
                                          struct pipe_picture_desc *picture)
{
   struct pipe_h265_enc_picture_desc *pic = (struct pipe_h265_enc_picture_desc *)picture;
   enc->enc_pic.hevc_spec_misc.log2_min_luma_coding_block_size_minus3 =
      pic->seq.log2_min_luma_coding_block_size_minus3;
   enc->enc_pic.hevc_spec_misc.amp_disabled = !pic->seq.amp_enabled_flag;
   enc->enc_pic.hevc_spec_misc.strong_intra_smoothing_enabled =
      pic->seq.strong_intra_smoothing_enabled_flag;
   enc->enc_pic.hevc_spec_misc.constrained_intra_pred_flag = pic->pic.constrained_intra_pred_flag;
   enc->enc_pic.hevc_spec_misc.cabac_init_flag = pic->slice.cabac_init_flag;
   enc->enc_pic.hevc_spec_misc.half_pel_enabled = 1;
   enc->enc_pic.hevc_spec_misc.quarter_pel_enabled = 1;

   RADEON_ENC_BEGIN(RENC_UVD_IB_PARAM_SPEC_MISC);
   RADEON_ENC_CS(enc->enc_pic.hevc_spec_misc.log2_min_luma_coding_block_size_minus3);
   RADEON_ENC_CS(enc->enc_pic.hevc_spec_misc.amp_disabled);
   RADEON_ENC_CS(enc->enc_pic.hevc_spec_misc.strong_intra_smoothing_enabled);
   RADEON_ENC_CS(enc->enc_pic.hevc_spec_misc.constrained_intra_pred_flag);
   RADEON_ENC_CS(enc->enc_pic.hevc_spec_misc.cabac_init_flag);
   RADEON_ENC_CS(enc->enc_pic.hevc_spec_misc.half_pel_enabled);
   RADEON_ENC_CS(enc->enc_pic.hevc_spec_misc.quarter_pel_enabled);
   RADEON_ENC_END();
}

static void radeon_uvd_enc_rc_session_init(struct radeon_uvd_encoder *enc,
                                           struct pipe_picture_desc *picture)
{
   struct pipe_h265_enc_picture_desc *pic = (struct pipe_h265_enc_picture_desc *)picture;
   enc->enc_pic.rc_session_init.vbv_buffer_level = pic->rc.vbv_buf_lv;
   switch (pic->rc.rate_ctrl_method) {
   case PIPE_H2645_ENC_RATE_CONTROL_METHOD_DISABLE:
      enc->enc_pic.rc_session_init.rate_control_method = RENC_UVD_RATE_CONTROL_METHOD_NONE;
      break;
   case PIPE_H2645_ENC_RATE_CONTROL_METHOD_CONSTANT_SKIP:
   case PIPE_H2645_ENC_RATE_CONTROL_METHOD_CONSTANT:
      enc->enc_pic.rc_session_init.rate_control_method = RENC_UVD_RATE_CONTROL_METHOD_CBR;
      break;
   case PIPE_H2645_ENC_RATE_CONTROL_METHOD_VARIABLE_SKIP:
   case PIPE_H2645_ENC_RATE_CONTROL_METHOD_VARIABLE:
      enc->enc_pic.rc_session_init.rate_control_method =
         RENC_UVD_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR;
      break;
   default:
      enc->enc_pic.rc_session_init.rate_control_method = RENC_UVD_RATE_CONTROL_METHOD_NONE;
   }

   RADEON_ENC_BEGIN(RENC_UVD_IB_PARAM_RATE_CONTROL_SESSION_INIT);
   RADEON_ENC_CS(enc->enc_pic.rc_session_init.rate_control_method);
   RADEON_ENC_CS(enc->enc_pic.rc_session_init.vbv_buffer_level);
   RADEON_ENC_END();
}

static void radeon_uvd_enc_rc_layer_init(struct radeon_uvd_encoder *enc,
                                         struct pipe_picture_desc *picture)
{
   struct pipe_h265_enc_picture_desc *pic = (struct pipe_h265_enc_picture_desc *)picture;
   enc->enc_pic.rc_layer_init.target_bit_rate = pic->rc.target_bitrate;
   enc->enc_pic.rc_layer_init.peak_bit_rate = pic->rc.peak_bitrate;
   enc->enc_pic.rc_layer_init.frame_rate_num = pic->rc.frame_rate_num;
   enc->enc_pic.rc_layer_init.frame_rate_den = pic->rc.frame_rate_den;
   enc->enc_pic.rc_layer_init.vbv_buffer_size = pic->rc.vbv_buffer_size;
   enc->enc_pic.rc_layer_init.avg_target_bits_per_picture = pic->rc.target_bits_picture;
   enc->enc_pic.rc_layer_init.peak_bits_per_picture_integer = pic->rc.peak_bits_picture_integer;
   enc->enc_pic.rc_layer_init.peak_bits_per_picture_fractional = pic->rc.peak_bits_picture_fraction;

   RADEON_ENC_BEGIN(RENC_UVD_IB_PARAM_RATE_CONTROL_LAYER_INIT);
   RADEON_ENC_CS(enc->enc_pic.rc_layer_init.target_bit_rate);
   RADEON_ENC_CS(enc->enc_pic.rc_layer_init.peak_bit_rate);
   RADEON_ENC_CS(enc->enc_pic.rc_layer_init.frame_rate_num);
   RADEON_ENC_CS(enc->enc_pic.rc_layer_init.frame_rate_den);
   RADEON_ENC_CS(enc->enc_pic.rc_layer_init.vbv_buffer_size);
   RADEON_ENC_CS(enc->enc_pic.rc_layer_init.avg_target_bits_per_picture);
   RADEON_ENC_CS(enc->enc_pic.rc_layer_init.peak_bits_per_picture_integer);
   RADEON_ENC_CS(enc->enc_pic.rc_layer_init.peak_bits_per_picture_fractional);
   RADEON_ENC_END();
}

static void radeon_uvd_enc_deblocking_filter_hevc(struct radeon_uvd_encoder *enc,
                                                  struct pipe_picture_desc *picture)
{
   struct pipe_h265_enc_picture_desc *pic = (struct pipe_h265_enc_picture_desc *)picture;
   enc->enc_pic.hevc_deblock.loop_filter_across_slices_enabled =
      pic->slice.slice_loop_filter_across_slices_enabled_flag;
   enc->enc_pic.hevc_deblock.deblocking_filter_disabled =
      pic->slice.slice_deblocking_filter_disabled_flag;
   enc->enc_pic.hevc_deblock.beta_offset_div2 = pic->slice.slice_beta_offset_div2;
   enc->enc_pic.hevc_deblock.tc_offset_div2 = pic->slice.slice_tc_offset_div2;
   enc->enc_pic.hevc_deblock.cb_qp_offset = pic->slice.slice_cb_qp_offset;
   enc->enc_pic.hevc_deblock.cr_qp_offset = pic->slice.slice_cr_qp_offset;

   RADEON_ENC_BEGIN(RENC_UVD_IB_PARAM_DEBLOCKING_FILTER);
   RADEON_ENC_CS(enc->enc_pic.hevc_deblock.loop_filter_across_slices_enabled);
   RADEON_ENC_CS(enc->enc_pic.hevc_deblock.deblocking_filter_disabled);
   RADEON_ENC_CS(enc->enc_pic.hevc_deblock.beta_offset_div2);
   RADEON_ENC_CS(enc->enc_pic.hevc_deblock.tc_offset_div2);
   RADEON_ENC_CS(enc->enc_pic.hevc_deblock.cb_qp_offset);
   RADEON_ENC_CS(enc->enc_pic.hevc_deblock.cr_qp_offset);
   RADEON_ENC_END();
}

static void radeon_uvd_enc_quality_params(struct radeon_uvd_encoder *enc)
{
   enc->enc_pic.quality_params.vbaq_mode = 0;
   enc->enc_pic.quality_params.scene_change_sensitivity = 0;
   enc->enc_pic.quality_params.scene_change_min_idr_interval = 0;

   RADEON_ENC_BEGIN(RENC_UVD_IB_PARAM_QUALITY_PARAMS);
   RADEON_ENC_CS(enc->enc_pic.quality_params.vbaq_mode);
   RADEON_ENC_CS(enc->enc_pic.quality_params.scene_change_sensitivity);
   RADEON_ENC_CS(enc->enc_pic.quality_params.scene_change_min_idr_interval);
   RADEON_ENC_END();
}

static void radeon_uvd_enc_nalu_sps_hevc(struct radeon_uvd_encoder *enc)
{
   RADEON_ENC_BEGIN(RENC_UVD_IB_PARAM_INSERT_NALU_BUFFER);
   RADEON_ENC_CS(RENC_UVD_NALU_TYPE_SPS);
   uint32_t *size_in_bytes = &enc->cs.current.buf[enc->cs.current.cdw++];
   int i;

   radeon_uvd_enc_reset(enc);
   radeon_uvd_enc_set_emulation_prevention(enc, false);
   radeon_uvd_enc_code_fixed_bits(enc, 0x00000001, 32);
   radeon_uvd_enc_code_fixed_bits(enc, 0x4201, 16);
   radeon_uvd_enc_byte_align(enc);
   radeon_uvd_enc_set_emulation_prevention(enc, true);
   radeon_uvd_enc_code_fixed_bits(enc, 0x0, 4);
   radeon_uvd_enc_code_fixed_bits(enc, enc->enc_pic.layer_ctrl.max_num_temporal_layers - 1, 3);
   radeon_uvd_enc_code_fixed_bits(enc, 0x1, 1);
   radeon_uvd_enc_code_fixed_bits(enc, 0x0, 2);
   radeon_uvd_enc_code_fixed_bits(enc, enc->enc_pic.general_tier_flag, 1);
   radeon_uvd_enc_code_fixed_bits(enc, enc->enc_pic.general_profile_idc, 5);
   radeon_uvd_enc_code_fixed_bits(enc, 0x60000000, 32);
   radeon_uvd_enc_code_fixed_bits(enc, 0xb0000000, 32);
   radeon_uvd_enc_code_fixed_bits(enc, 0x0, 16);
   radeon_uvd_enc_code_fixed_bits(enc, enc->enc_pic.general_level_idc, 8);

   for (i = 0; i < (enc->enc_pic.layer_ctrl.max_num_temporal_layers - 1); i++)
      radeon_uvd_enc_code_fixed_bits(enc, 0x0, 2);

   if ((enc->enc_pic.layer_ctrl.max_num_temporal_layers - 1) > 0) {
      for (i = (enc->enc_pic.layer_ctrl.max_num_temporal_layers - 1); i < 8; i++)
         radeon_uvd_enc_code_fixed_bits(enc, 0x0, 2);
   }

   radeon_uvd_enc_code_ue(enc, 0x0);
   radeon_uvd_enc_code_ue(enc, enc->enc_pic.chroma_format_idc);
   radeon_uvd_enc_code_ue(enc, enc->enc_pic.session_init.aligned_picture_width);
   radeon_uvd_enc_code_ue(enc, enc->enc_pic.session_init.aligned_picture_height);

   int conformance_window_flag = (enc->enc_pic.crop_top > 0) || (enc->enc_pic.crop_bottom > 0) ||
                                       (enc->enc_pic.crop_left > 0) || (enc->enc_pic.crop_right > 0)
                                    ? 0x1
                                    : 0x0;
   radeon_uvd_enc_code_fixed_bits(enc, conformance_window_flag, 1);

   if (conformance_window_flag == 1) {
      radeon_uvd_enc_code_ue(enc, enc->enc_pic.crop_left);
      radeon_uvd_enc_code_ue(enc, enc->enc_pic.crop_right);
      radeon_uvd_enc_code_ue(enc, enc->enc_pic.crop_top);
      radeon_uvd_enc_code_ue(enc, enc->enc_pic.crop_bottom);
   }

   radeon_uvd_enc_code_ue(enc, enc->enc_pic.bit_depth_luma_minus8);
   radeon_uvd_enc_code_ue(enc, enc->enc_pic.bit_depth_chroma_minus8);
   radeon_uvd_enc_code_ue(enc, enc->enc_pic.log2_max_poc - 4);
   radeon_uvd_enc_code_fixed_bits(enc, 0x0, 1);
   radeon_uvd_enc_code_ue(enc, 1);
   radeon_uvd_enc_code_ue(enc, 0x0);
   radeon_uvd_enc_code_ue(enc, 0x0);
   radeon_uvd_enc_code_ue(enc, enc->enc_pic.hevc_spec_misc.log2_min_luma_coding_block_size_minus3);
   /* Only support CTBSize 64 */
   radeon_uvd_enc_code_ue(
      enc, 6 - (enc->enc_pic.hevc_spec_misc.log2_min_luma_coding_block_size_minus3 + 3));
   radeon_uvd_enc_code_ue(enc, enc->enc_pic.log2_min_transform_block_size_minus2);
   radeon_uvd_enc_code_ue(enc, enc->enc_pic.log2_diff_max_min_transform_block_size);
   radeon_uvd_enc_code_ue(enc, enc->enc_pic.max_transform_hierarchy_depth_inter);
   radeon_uvd_enc_code_ue(enc, enc->enc_pic.max_transform_hierarchy_depth_intra);

   radeon_uvd_enc_code_fixed_bits(enc, 0x0, 1);
   radeon_uvd_enc_code_fixed_bits(enc, !enc->enc_pic.hevc_spec_misc.amp_disabled, 1);
   radeon_uvd_enc_code_fixed_bits(enc, enc->enc_pic.sample_adaptive_offset_enabled_flag, 1);
   radeon_uvd_enc_code_fixed_bits(enc, enc->enc_pic.pcm_enabled_flag, 1);

   radeon_uvd_enc_code_ue(enc, 1);
   radeon_uvd_enc_code_ue(enc, 1);
   radeon_uvd_enc_code_ue(enc, 0);
   radeon_uvd_enc_code_ue(enc, 0);
   radeon_uvd_enc_code_fixed_bits(enc, 0x1, 1);

   radeon_uvd_enc_code_fixed_bits(enc, 0x0, 1);

   radeon_uvd_enc_code_fixed_bits(enc, 0, 1);
   radeon_uvd_enc_code_fixed_bits(enc, enc->enc_pic.hevc_spec_misc.strong_intra_smoothing_enabled,
                                  1);

   radeon_uvd_enc_code_fixed_bits(enc, (enc->enc_pic.vui_info.vui_parameters_present_flag), 1);
   if (enc->enc_pic.vui_info.vui_parameters_present_flag) {
      /* aspect ratio present flag */
      radeon_uvd_enc_code_fixed_bits(enc, (enc->enc_pic.vui_info.flags.aspect_ratio_info_present_flag), 1);
      if (enc->enc_pic.vui_info.flags.aspect_ratio_info_present_flag) {
         radeon_uvd_enc_code_fixed_bits(enc, (enc->enc_pic.vui_info.aspect_ratio_idc), 8);
         if (enc->enc_pic.vui_info.aspect_ratio_idc == PIPE_H2645_EXTENDED_SAR) {
            radeon_uvd_enc_code_fixed_bits(enc, (enc->enc_pic.vui_info.sar_width), 16);
            radeon_uvd_enc_code_fixed_bits(enc, (enc->enc_pic.vui_info.sar_height), 16);
         }
      }
      radeon_uvd_enc_code_fixed_bits(enc, 0x0, 1);  /* overscan info present flag */
      /* video signal type present flag  */
      radeon_uvd_enc_code_fixed_bits(enc, enc->enc_pic.vui_info.flags.video_signal_type_present_flag, 1);
      if (enc->enc_pic.vui_info.flags.video_signal_type_present_flag) {
         radeon_uvd_enc_code_fixed_bits(enc, enc->enc_pic.vui_info.video_format, 3);
         radeon_uvd_enc_code_fixed_bits(enc, enc->enc_pic.vui_info.video_full_range_flag, 1);
         radeon_uvd_enc_code_fixed_bits(enc, enc->enc_pic.vui_info.flags.colour_description_present_flag, 1);
         if (enc->enc_pic.vui_info.flags.colour_description_present_flag) {
            radeon_uvd_enc_code_fixed_bits(enc, enc->enc_pic.vui_info.colour_primaries, 8);
            radeon_uvd_enc_code_fixed_bits(enc, enc->enc_pic.vui_info.transfer_characteristics, 8);
            radeon_uvd_enc_code_fixed_bits(enc, enc->enc_pic.vui_info.matrix_coefficients, 8);
         }
      }
      /* chroma loc info present flag */
      radeon_uvd_enc_code_fixed_bits(enc, enc->enc_pic.vui_info.flags.chroma_loc_info_present_flag, 1);
      if (enc->enc_pic.vui_info.flags.chroma_loc_info_present_flag) {
         radeon_uvd_enc_code_ue(enc, enc->enc_pic.vui_info.chroma_sample_loc_type_top_field);
         radeon_uvd_enc_code_ue(enc, enc->enc_pic.vui_info.chroma_sample_loc_type_bottom_field);
      }
      radeon_uvd_enc_code_fixed_bits(enc, 0x0, 1);  /* neutral chroma indication flag */
      radeon_uvd_enc_code_fixed_bits(enc, 0x0, 1);  /* field seq flag */
      radeon_uvd_enc_code_fixed_bits(enc, 0x0, 1);  /* frame field info present flag */
      radeon_uvd_enc_code_fixed_bits(enc, 0x0, 1);  /* default display windows flag */
      /* vui timing info present flag */
      radeon_uvd_enc_code_fixed_bits(enc, (enc->enc_pic.vui_info.flags.timing_info_present_flag), 1);
      if (enc->enc_pic.vui_info.flags.timing_info_present_flag) {
         radeon_uvd_enc_code_fixed_bits(enc, (enc->enc_pic.vui_info.num_units_in_tick), 32);
         radeon_uvd_enc_code_fixed_bits(enc, (enc->enc_pic.vui_info.time_scale), 32);
         radeon_uvd_enc_code_fixed_bits(enc, 0x0, 1);
         radeon_uvd_enc_code_fixed_bits(enc, 0x0, 1);
      }
      radeon_uvd_enc_code_fixed_bits(enc, 0x0, 1);  /* bitstream restriction flag */
   }

   radeon_uvd_enc_code_fixed_bits(enc, 0x0, 1);

   radeon_uvd_enc_code_fixed_bits(enc, 0x1, 1);

   radeon_uvd_enc_byte_align(enc);
   radeon_uvd_enc_flush_headers(enc);
   *size_in_bytes = (enc->bits_output + 7) / 8;
   RADEON_ENC_END();
}

static void radeon_uvd_enc_nalu_pps_hevc(struct radeon_uvd_encoder *enc)
{
   RADEON_ENC_BEGIN(RENC_UVD_IB_PARAM_INSERT_NALU_BUFFER);
   RADEON_ENC_CS(RENC_UVD_NALU_TYPE_PPS);
   uint32_t *size_in_bytes = &enc->cs.current.buf[enc->cs.current.cdw++];
   radeon_uvd_enc_reset(enc);
   radeon_uvd_enc_set_emulation_prevention(enc, false);
   radeon_uvd_enc_code_fixed_bits(enc, 0x00000001, 32);
   radeon_uvd_enc_code_fixed_bits(enc, 0x4401, 16);
   radeon_uvd_enc_byte_align(enc);
   radeon_uvd_enc_set_emulation_prevention(enc, true);
   radeon_uvd_enc_code_ue(enc, 0x0);
   radeon_uvd_enc_code_ue(enc, 0x0);
   radeon_uvd_enc_code_fixed_bits(enc, 0x1, 1);
   radeon_uvd_enc_code_fixed_bits(enc, 0x0, 1); /* output_flag_resent_flag */
   radeon_uvd_enc_code_fixed_bits(enc, 0x0, 3); /* num_extra_slice_header_bits */
   radeon_uvd_enc_code_fixed_bits(enc, 0x0, 1);
   radeon_uvd_enc_code_fixed_bits(enc, 0x1, 1);
   radeon_uvd_enc_code_ue(enc, 0x0);
   radeon_uvd_enc_code_ue(enc, 0x0);
   radeon_uvd_enc_code_se(enc, 0x0);
   radeon_uvd_enc_code_fixed_bits(enc, enc->enc_pic.hevc_spec_misc.constrained_intra_pred_flag, 1);
   radeon_uvd_enc_code_fixed_bits(enc, 0x0, 1);
   if (enc->enc_pic.rc_session_init.rate_control_method == RENC_UVD_RATE_CONTROL_METHOD_NONE)
      radeon_uvd_enc_code_fixed_bits(enc, 0x0, 1);
   else {
      radeon_uvd_enc_code_fixed_bits(enc, 0x1, 1);
      radeon_uvd_enc_code_ue(enc, 0x0);
   }
   radeon_uvd_enc_code_se(enc, enc->enc_pic.hevc_deblock.cb_qp_offset);
   radeon_uvd_enc_code_se(enc, enc->enc_pic.hevc_deblock.cr_qp_offset);
   radeon_uvd_enc_code_fixed_bits(enc, 0x0, 1);
   radeon_uvd_enc_code_fixed_bits(enc, 0x0, 2);
   radeon_uvd_enc_code_fixed_bits(enc, 0x0, 1);
   radeon_uvd_enc_code_fixed_bits(enc, 0x0, 1);
   radeon_uvd_enc_code_fixed_bits(enc, 0x0, 1);
   radeon_uvd_enc_code_fixed_bits(enc, enc->enc_pic.hevc_deblock.loop_filter_across_slices_enabled,
                                  1);
   radeon_uvd_enc_code_fixed_bits(enc, 0x1, 1);
   radeon_uvd_enc_code_fixed_bits(enc, 0x0, 1);
   radeon_uvd_enc_code_fixed_bits(enc, enc->enc_pic.hevc_deblock.deblocking_filter_disabled, 1);

   if (!enc->enc_pic.hevc_deblock.deblocking_filter_disabled) {
      radeon_uvd_enc_code_se(enc, enc->enc_pic.hevc_deblock.beta_offset_div2);
      radeon_uvd_enc_code_se(enc, enc->enc_pic.hevc_deblock.tc_offset_div2);
   }

   radeon_uvd_enc_code_fixed_bits(enc, 0x0, 1);
   radeon_uvd_enc_code_fixed_bits(enc, 0x0, 1);
   radeon_uvd_enc_code_ue(enc, enc->enc_pic.log2_parallel_merge_level_minus2);
   radeon_uvd_enc_code_fixed_bits(enc, 0x0, 2);

   radeon_uvd_enc_code_fixed_bits(enc, 0x1, 1);

   radeon_uvd_enc_byte_align(enc);
   radeon_uvd_enc_flush_headers(enc);
   *size_in_bytes = (enc->bits_output + 7) / 8;
   RADEON_ENC_END();
}

static void radeon_uvd_enc_nalu_vps_hevc(struct radeon_uvd_encoder *enc)
{
   RADEON_ENC_BEGIN(RENC_UVD_IB_PARAM_INSERT_NALU_BUFFER);
   RADEON_ENC_CS(RENC_UVD_NALU_TYPE_VPS);
   uint32_t *size_in_bytes = &enc->cs.current.buf[enc->cs.current.cdw++];
   int i;

   radeon_uvd_enc_reset(enc);
   radeon_uvd_enc_set_emulation_prevention(enc, false);
   radeon_uvd_enc_code_fixed_bits(enc, 0x00000001, 32);
   radeon_uvd_enc_code_fixed_bits(enc, 0x4001, 16);
   radeon_uvd_enc_byte_align(enc);
   radeon_uvd_enc_set_emulation_prevention(enc, true);

   radeon_uvd_enc_code_fixed_bits(enc, 0x0, 4);
   radeon_uvd_enc_code_fixed_bits(enc, 0x3, 2);
   radeon_uvd_enc_code_fixed_bits(enc, 0x0, 6);
   radeon_uvd_enc_code_fixed_bits(enc, enc->enc_pic.layer_ctrl.max_num_temporal_layers - 1, 3);
   radeon_uvd_enc_code_fixed_bits(enc, 0x1, 1);
   radeon_uvd_enc_code_fixed_bits(enc, 0xffff, 16);
   radeon_uvd_enc_code_fixed_bits(enc, 0x0, 2);
   radeon_uvd_enc_code_fixed_bits(enc, enc->enc_pic.general_tier_flag, 1);
   radeon_uvd_enc_code_fixed_bits(enc, enc->enc_pic.general_profile_idc, 5);
   radeon_uvd_enc_code_fixed_bits(enc, 0x60000000, 32);
   radeon_uvd_enc_code_fixed_bits(enc, 0xb0000000, 32);
   radeon_uvd_enc_code_fixed_bits(enc, 0x0, 16);
   radeon_uvd_enc_code_fixed_bits(enc, enc->enc_pic.general_level_idc, 8);

   for (i = 0; i < (enc->enc_pic.layer_ctrl.max_num_temporal_layers - 1); i++)
      radeon_uvd_enc_code_fixed_bits(enc, 0x0, 2);

   if ((enc->enc_pic.layer_ctrl.max_num_temporal_layers - 1) > 0) {
      for (i = (enc->enc_pic.layer_ctrl.max_num_temporal_layers - 1); i < 8; i++)
         radeon_uvd_enc_code_fixed_bits(enc, 0x0, 2);
   }

   radeon_uvd_enc_code_fixed_bits(enc, 0x0, 1);
   radeon_uvd_enc_code_ue(enc, 0x1);
   radeon_uvd_enc_code_ue(enc, 0x0);
   radeon_uvd_enc_code_ue(enc, 0x0);

   radeon_uvd_enc_code_fixed_bits(enc, 0x0, 6);
   radeon_uvd_enc_code_ue(enc, 0x0);
   radeon_uvd_enc_code_fixed_bits(enc, 0x0, 1);
   radeon_uvd_enc_code_fixed_bits(enc, 0x0, 1);

   radeon_uvd_enc_code_fixed_bits(enc, 0x1, 1);

   radeon_uvd_enc_byte_align(enc);
   radeon_uvd_enc_flush_headers(enc);
   *size_in_bytes = (enc->bits_output + 7) / 8;
   RADEON_ENC_END();
}

static void radeon_uvd_enc_nalu_aud_hevc(struct radeon_uvd_encoder *enc)
{
   RADEON_ENC_BEGIN(RENC_UVD_IB_PARAM_INSERT_NALU_BUFFER);
   RADEON_ENC_CS(RENC_UVD_NALU_TYPE_AUD);
   uint32_t *size_in_bytes = &enc->cs.current.buf[enc->cs.current.cdw++];
   radeon_uvd_enc_reset(enc);
   radeon_uvd_enc_set_emulation_prevention(enc, false);
   radeon_uvd_enc_code_fixed_bits(enc, 0x00000001, 32);
   radeon_uvd_enc_code_fixed_bits(enc, 0x0, 1);
   radeon_uvd_enc_code_fixed_bits(enc, 35, 6);
   radeon_uvd_enc_code_fixed_bits(enc, 0x0, 6);
   radeon_uvd_enc_code_fixed_bits(enc, 0x1, 3);
   radeon_uvd_enc_byte_align(enc);
   radeon_uvd_enc_set_emulation_prevention(enc, true);
   switch (enc->enc_pic.picture_type) {
   case PIPE_H2645_ENC_PICTURE_TYPE_I:
   case PIPE_H2645_ENC_PICTURE_TYPE_IDR:
      radeon_uvd_enc_code_fixed_bits(enc, 0x00, 3);
      break;
   case PIPE_H2645_ENC_PICTURE_TYPE_P:
      radeon_uvd_enc_code_fixed_bits(enc, 0x01, 3);
      break;
   case PIPE_H2645_ENC_PICTURE_TYPE_B:
      radeon_uvd_enc_code_fixed_bits(enc, 0x02, 3);
      break;
   default:
      assert(0 && "Unsupported picture type!");
   }

   radeon_uvd_enc_code_fixed_bits(enc, 0x1, 1);

   radeon_uvd_enc_byte_align(enc);
   radeon_uvd_enc_flush_headers(enc);
   *size_in_bytes = (enc->bits_output + 7) / 8;
   RADEON_ENC_END();
}

static void radeon_uvd_enc_slice_header_hevc(struct radeon_uvd_encoder *enc)
{
   uint32_t instruction[RENC_UVD_SLICE_HEADER_TEMPLATE_MAX_NUM_INSTRUCTIONS] = {0};
   uint32_t num_bits[RENC_UVD_SLICE_HEADER_TEMPLATE_MAX_NUM_INSTRUCTIONS] = {0};
   unsigned int inst_index = 0;
   unsigned int bit_index = 0;
   unsigned int bits_copied = 0;
   RADEON_ENC_BEGIN(RENC_UVD_IB_PARAM_SLICE_HEADER);
   radeon_uvd_enc_reset(enc);
   radeon_uvd_enc_set_emulation_prevention(enc, false);

   radeon_uvd_enc_code_fixed_bits(enc, 0x0, 1);
   radeon_uvd_enc_code_fixed_bits(enc, enc->enc_pic.nal_unit_type, 6);
   radeon_uvd_enc_code_fixed_bits(enc, 0x0, 6);
   radeon_uvd_enc_code_fixed_bits(enc, 0x1, 3);

   radeon_uvd_enc_flush_headers(enc);
   bit_index++;
   instruction[inst_index] = RENC_UVD_HEADER_INSTRUCTION_COPY;
   num_bits[inst_index] = enc->bits_output - bits_copied;
   bits_copied = enc->bits_output;
   inst_index++;

   instruction[inst_index] = RENC_UVD_HEADER_INSTRUCTION_FIRST_SLICE;
   inst_index++;

   if ((enc->enc_pic.nal_unit_type >= 16) && (enc->enc_pic.nal_unit_type <= 23))
      radeon_uvd_enc_code_fixed_bits(enc, 0x0, 1);

   radeon_uvd_enc_code_ue(enc, 0x0);

   radeon_uvd_enc_flush_headers(enc);
   bit_index++;
   instruction[inst_index] = RENC_UVD_HEADER_INSTRUCTION_COPY;
   num_bits[inst_index] = enc->bits_output - bits_copied;
   bits_copied = enc->bits_output;
   inst_index++;

   instruction[inst_index] = RENC_UVD_HEADER_INSTRUCTION_SLICE_SEGMENT;
   inst_index++;

   instruction[inst_index] = RENC_UVD_HEADER_INSTRUCTION_DEPENDENT_SLICE_END;
   inst_index++;

   switch (enc->enc_pic.picture_type) {
   case PIPE_H2645_ENC_PICTURE_TYPE_I:
   case PIPE_H2645_ENC_PICTURE_TYPE_IDR:
      radeon_uvd_enc_code_ue(enc, 0x2);
      break;
   case PIPE_H2645_ENC_PICTURE_TYPE_P:
   case PIPE_H2645_ENC_PICTURE_TYPE_SKIP:
      radeon_uvd_enc_code_ue(enc, 0x1);
      break;
   case PIPE_H2645_ENC_PICTURE_TYPE_B:
      radeon_uvd_enc_code_ue(enc, 0x0);
      break;
   default:
      radeon_uvd_enc_code_ue(enc, 0x1);
   }

   if ((enc->enc_pic.nal_unit_type != 19) && (enc->enc_pic.nal_unit_type != 20)) {
      radeon_uvd_enc_code_fixed_bits(enc, enc->enc_pic.pic_order_cnt, enc->enc_pic.log2_max_poc);
      if (enc->enc_pic.picture_type == PIPE_H2645_ENC_PICTURE_TYPE_P)
         radeon_uvd_enc_code_fixed_bits(enc, 0x1, 1);
      else {
         radeon_uvd_enc_code_fixed_bits(enc, 0x0, 1);
         radeon_uvd_enc_code_fixed_bits(enc, 0x0, 1);
         radeon_uvd_enc_code_ue(enc, 0x0);
         radeon_uvd_enc_code_ue(enc, 0x0);
      }
   }

   if (enc->enc_pic.sample_adaptive_offset_enabled_flag)
      radeon_uvd_enc_code_fixed_bits(enc, 0x0, 1); /* slice_sao_luma_flag */

   if ((enc->enc_pic.picture_type == PIPE_H2645_ENC_PICTURE_TYPE_P) ||
       (enc->enc_pic.picture_type == PIPE_H2645_ENC_PICTURE_TYPE_B)) {
      radeon_uvd_enc_code_fixed_bits(enc, 0x0, 1);
      radeon_uvd_enc_code_fixed_bits(enc, enc->enc_pic.hevc_spec_misc.cabac_init_flag, 1);
      radeon_uvd_enc_code_ue(enc, 5 - enc->enc_pic.max_num_merge_cand);
   }

   radeon_uvd_enc_flush_headers(enc);
   bit_index++;
   instruction[inst_index] = RENC_UVD_HEADER_INSTRUCTION_COPY;
   num_bits[inst_index] = enc->bits_output - bits_copied;
   bits_copied = enc->bits_output;
   inst_index++;

   instruction[inst_index] = RENC_UVD_HEADER_INSTRUCTION_SLICE_QP_DELTA;
   inst_index++;

   if ((enc->enc_pic.hevc_deblock.loop_filter_across_slices_enabled) &&
       (!enc->enc_pic.hevc_deblock.deblocking_filter_disabled)) {
      radeon_uvd_enc_code_fixed_bits(
         enc, enc->enc_pic.hevc_deblock.loop_filter_across_slices_enabled, 1);

      radeon_uvd_enc_flush_headers(enc);
      bit_index++;
      instruction[inst_index] = RENC_UVD_HEADER_INSTRUCTION_COPY;
      num_bits[inst_index] = enc->bits_output - bits_copied;
      bits_copied = enc->bits_output;
      inst_index++;
   }

   instruction[inst_index] = RENC_UVD_HEADER_INSTRUCTION_END;

   for (int i = bit_index; i < RENC_UVD_SLICE_HEADER_TEMPLATE_MAX_TEMPLATE_SIZE_IN_DWORDS; i++)
      RADEON_ENC_CS(0x00000000);

   for (int j = 0; j < RENC_UVD_SLICE_HEADER_TEMPLATE_MAX_NUM_INSTRUCTIONS; j++) {
      RADEON_ENC_CS(instruction[j]);
      RADEON_ENC_CS(num_bits[j]);
   }

   RADEON_ENC_END();
}

static void radeon_uvd_enc_ctx(struct radeon_uvd_encoder *enc)
{
   struct si_screen *sscreen = (struct si_screen *)enc->screen;

   enc->enc_pic.ctx_buf.swizzle_mode = 0;
   if (sscreen->info.gfx_level < GFX9) {
      enc->enc_pic.ctx_buf.rec_luma_pitch = (enc->luma->u.legacy.level[0].nblk_x * enc->luma->bpe);
      enc->enc_pic.ctx_buf.rec_chroma_pitch =
         (enc->chroma->u.legacy.level[0].nblk_x * enc->chroma->bpe);
   } else {
      enc->enc_pic.ctx_buf.rec_luma_pitch = enc->luma->u.gfx9.surf_pitch * enc->luma->bpe;
      enc->enc_pic.ctx_buf.rec_chroma_pitch = enc->chroma->u.gfx9.surf_pitch * enc->chroma->bpe;
   }
   enc->enc_pic.ctx_buf.num_reconstructed_pictures = 2;

   RADEON_ENC_BEGIN(RENC_UVD_IB_PARAM_ENCODE_CONTEXT_BUFFER);
   RADEON_ENC_READWRITE(enc->cpb.res->buf, enc->cpb.res->domains, 0);
   RADEON_ENC_CS(0x00000000); // reserved
   RADEON_ENC_CS(enc->enc_pic.ctx_buf.swizzle_mode);
   RADEON_ENC_CS(enc->enc_pic.ctx_buf.rec_luma_pitch);
   RADEON_ENC_CS(enc->enc_pic.ctx_buf.rec_chroma_pitch);
   RADEON_ENC_CS(enc->enc_pic.ctx_buf.num_reconstructed_pictures);
   /* reconstructed_picture_1_luma_offset */
   RADEON_ENC_CS(0x00000000);
   /* reconstructed_picture_1_chroma_offset */
   RADEON_ENC_CS(enc->enc_pic.ctx_buf.rec_chroma_pitch * align(enc->base.height, 16));
   /* reconstructed_picture_2_luma_offset */
   RADEON_ENC_CS(enc->enc_pic.ctx_buf.rec_luma_pitch * align(enc->base.height, 16) * 3 / 2);
   /* reconstructed_picture_2_chroma_offset */
   RADEON_ENC_CS(enc->enc_pic.ctx_buf.rec_chroma_pitch * align(enc->base.height, 16) * 5 / 2);

   for (int i = 0; i < 136; i++)
      RADEON_ENC_CS(0x00000000);

   RADEON_ENC_END();
}

static void radeon_uvd_enc_bitstream(struct radeon_uvd_encoder *enc)
{
   enc->enc_pic.bit_buf.mode = RENC_UVD_SWIZZLE_MODE_LINEAR;
   enc->enc_pic.bit_buf.video_bitstream_buffer_size = enc->bs_size;
   enc->enc_pic.bit_buf.video_bitstream_data_offset = 0;

   RADEON_ENC_BEGIN(RENC_UVD_IB_PARAM_VIDEO_BITSTREAM_BUFFER);
   RADEON_ENC_CS(enc->enc_pic.bit_buf.mode);
   RADEON_ENC_WRITE(enc->bs_handle, RADEON_DOMAIN_GTT, 0);
   RADEON_ENC_CS(enc->enc_pic.bit_buf.video_bitstream_buffer_size);
   RADEON_ENC_CS(enc->enc_pic.bit_buf.video_bitstream_data_offset);
   RADEON_ENC_END();
}

static void radeon_uvd_enc_feedback(struct radeon_uvd_encoder *enc)
{
   enc->enc_pic.fb_buf.mode = RENC_UVD_FEEDBACK_BUFFER_MODE_LINEAR;
   enc->enc_pic.fb_buf.feedback_buffer_size = 16;
   enc->enc_pic.fb_buf.feedback_data_size = 40;

   RADEON_ENC_BEGIN(RENC_UVD_IB_PARAM_FEEDBACK_BUFFER);
   RADEON_ENC_CS(enc->enc_pic.fb_buf.mode);
   RADEON_ENC_WRITE(enc->fb->res->buf, enc->fb->res->domains, 0x0);
   RADEON_ENC_CS(enc->enc_pic.fb_buf.feedback_buffer_size);
   RADEON_ENC_CS(enc->enc_pic.fb_buf.feedback_data_size);
   RADEON_ENC_END();
}

static void radeon_uvd_enc_intra_refresh(struct radeon_uvd_encoder *enc)
{
   enc->enc_pic.intra_ref.intra_refresh_mode = RENC_UVD_INTRA_REFRESH_MODE_NONE;
   enc->enc_pic.intra_ref.offset = 0;
   enc->enc_pic.intra_ref.region_size = 0;

   RADEON_ENC_BEGIN(RENC_UVD_IB_PARAM_INTRA_REFRESH);
   RADEON_ENC_CS(enc->enc_pic.intra_ref.intra_refresh_mode);
   RADEON_ENC_CS(enc->enc_pic.intra_ref.offset);
   RADEON_ENC_CS(enc->enc_pic.intra_ref.region_size);
   RADEON_ENC_END();
}

static void radeon_uvd_enc_rc_per_pic(struct radeon_uvd_encoder *enc,
                                      struct pipe_picture_desc *picture)
{
   struct pipe_h265_enc_picture_desc *pic = (struct pipe_h265_enc_picture_desc *)picture;
   enc->enc_pic.rc_per_pic.qp = pic->rc.quant_i_frames;
   enc->enc_pic.rc_per_pic.min_qp_app = 0;
   enc->enc_pic.rc_per_pic.max_qp_app = 51;
   enc->enc_pic.rc_per_pic.max_au_size = 0;
   enc->enc_pic.rc_per_pic.enabled_filler_data = pic->rc.fill_data_enable;
   enc->enc_pic.rc_per_pic.skip_frame_enable = false;
   enc->enc_pic.rc_per_pic.enforce_hrd = pic->rc.enforce_hrd;

   RADEON_ENC_BEGIN(RENC_UVD_IB_PARAM_RATE_CONTROL_PER_PICTURE);
   RADEON_ENC_CS(enc->enc_pic.rc_per_pic.qp);
   RADEON_ENC_CS(enc->enc_pic.rc_per_pic.min_qp_app);
   RADEON_ENC_CS(enc->enc_pic.rc_per_pic.max_qp_app);
   RADEON_ENC_CS(enc->enc_pic.rc_per_pic.max_au_size);
   RADEON_ENC_CS(enc->enc_pic.rc_per_pic.enabled_filler_data);
   RADEON_ENC_CS(enc->enc_pic.rc_per_pic.skip_frame_enable);
   RADEON_ENC_CS(enc->enc_pic.rc_per_pic.enforce_hrd);
   RADEON_ENC_END();
}

static void radeon_uvd_enc_encode_params_hevc(struct radeon_uvd_encoder *enc)
{
   struct si_screen *sscreen = (struct si_screen *)enc->screen;
   switch (enc->enc_pic.picture_type) {
   case PIPE_H2645_ENC_PICTURE_TYPE_I:
   case PIPE_H2645_ENC_PICTURE_TYPE_IDR:
      enc->enc_pic.enc_params.pic_type = RENC_UVD_PICTURE_TYPE_I;
      break;
   case PIPE_H2645_ENC_PICTURE_TYPE_P:
      enc->enc_pic.enc_params.pic_type = RENC_UVD_PICTURE_TYPE_P;
      break;
   case PIPE_H2645_ENC_PICTURE_TYPE_SKIP:
      enc->enc_pic.enc_params.pic_type = RENC_UVD_PICTURE_TYPE_P_SKIP;
      break;
   case PIPE_H2645_ENC_PICTURE_TYPE_B:
      enc->enc_pic.enc_params.pic_type = RENC_UVD_PICTURE_TYPE_B;
      break;
   default:
      enc->enc_pic.enc_params.pic_type = RENC_UVD_PICTURE_TYPE_I;
   }

   enc->enc_pic.enc_params.allowed_max_bitstream_size = enc->bs_size;
   if (sscreen->info.gfx_level < GFX9) {
      enc->enc_pic.enc_params.input_pic_luma_pitch =
         (enc->luma->u.legacy.level[0].nblk_x * enc->luma->bpe);
      enc->enc_pic.enc_params.input_pic_chroma_pitch =
         (enc->chroma->u.legacy.level[0].nblk_x * enc->chroma->bpe);
   } else {
      enc->enc_pic.enc_params.input_pic_luma_pitch = enc->luma->u.gfx9.surf_pitch * enc->luma->bpe;
      enc->enc_pic.enc_params.input_pic_chroma_pitch =
         enc->chroma->u.gfx9.surf_pitch * enc->chroma->bpe;
   }
   enc->enc_pic.enc_params.input_pic_swizzle_mode = RENC_UVD_SWIZZLE_MODE_LINEAR;

   if (enc->enc_pic.enc_params.pic_type == RENC_UVD_PICTURE_TYPE_I)
      enc->enc_pic.enc_params.reference_picture_index = 0xFFFFFFFF;
   else
      enc->enc_pic.enc_params.reference_picture_index = (enc->enc_pic.frame_num - 1) % 2;

   enc->enc_pic.enc_params.reconstructed_picture_index = enc->enc_pic.frame_num % 2;

   RADEON_ENC_BEGIN(RENC_UVD_IB_PARAM_ENCODE_PARAMS);
   RADEON_ENC_CS(enc->enc_pic.enc_params.pic_type);
   RADEON_ENC_CS(enc->enc_pic.enc_params.allowed_max_bitstream_size);

   if (sscreen->info.gfx_level < GFX9) {
      RADEON_ENC_READ(enc->handle, RADEON_DOMAIN_VRAM, (uint64_t)enc->luma->u.legacy.level[0].offset_256B * 256);
      RADEON_ENC_READ(enc->handle, RADEON_DOMAIN_VRAM, (uint64_t)enc->chroma->u.legacy.level[0].offset_256B * 256);
   } else {
      RADEON_ENC_READ(enc->handle, RADEON_DOMAIN_VRAM, enc->luma->u.gfx9.surf_offset);
      RADEON_ENC_READ(enc->handle, RADEON_DOMAIN_VRAM, enc->chroma->u.gfx9.surf_offset);
   }
   RADEON_ENC_CS(enc->enc_pic.enc_params.input_pic_luma_pitch);
   RADEON_ENC_CS(enc->enc_pic.enc_params.input_pic_chroma_pitch);
   RADEON_ENC_CS(0x00000000); // reserved
   RADEON_ENC_CS(enc->enc_pic.enc_params.input_pic_swizzle_mode);
   RADEON_ENC_CS(enc->enc_pic.enc_params.reference_picture_index);
   RADEON_ENC_CS(enc->enc_pic.enc_params.reconstructed_picture_index);
   RADEON_ENC_END();
}

static void radeon_uvd_enc_op_init(struct radeon_uvd_encoder *enc)
{
   RADEON_ENC_BEGIN(RENC_UVD_IB_OP_INITIALIZE);
   RADEON_ENC_END();
}

static void radeon_uvd_enc_op_close(struct radeon_uvd_encoder *enc)
{
   RADEON_ENC_BEGIN(RENC_UVD_IB_OP_CLOSE_SESSION);
   RADEON_ENC_END();
}

static void radeon_uvd_enc_op_enc(struct radeon_uvd_encoder *enc)
{
   RADEON_ENC_BEGIN(RENC_UVD_IB_OP_ENCODE);
   RADEON_ENC_END();
}

static void radeon_uvd_enc_op_init_rc(struct radeon_uvd_encoder *enc)
{
   RADEON_ENC_BEGIN(RENC_UVD_IB_OP_INIT_RC);
   RADEON_ENC_END();
}

static void radeon_uvd_enc_op_init_rc_vbv(struct radeon_uvd_encoder *enc)
{
   RADEON_ENC_BEGIN(RENC_UVD_IB_OP_INIT_RC_VBV_BUFFER_LEVEL);
   RADEON_ENC_END();
}

static void radeon_uvd_enc_op_speed(struct radeon_uvd_encoder *enc)
{
   RADEON_ENC_BEGIN(RENC_UVD_IB_OP_SET_SPEED_ENCODING_MODE);
   RADEON_ENC_END();
}

static void begin(struct radeon_uvd_encoder *enc, struct pipe_picture_desc *pic)
{
   radeon_uvd_enc_session_info(enc);
   enc->total_task_size = 0;
   radeon_uvd_enc_task_info(enc, enc->need_feedback);
   radeon_uvd_enc_op_init(enc);

   radeon_uvd_enc_session_init_hevc(enc);
   radeon_uvd_enc_slice_control_hevc(enc);
   radeon_uvd_enc_spec_misc_hevc(enc, pic);
   radeon_uvd_enc_deblocking_filter_hevc(enc, pic);

   radeon_uvd_enc_layer_control(enc);
   radeon_uvd_enc_rc_session_init(enc, pic);
   radeon_uvd_enc_quality_params(enc);
   radeon_uvd_enc_layer_select(enc);
   radeon_uvd_enc_rc_layer_init(enc, pic);
   radeon_uvd_enc_layer_select(enc);
   radeon_uvd_enc_rc_per_pic(enc, pic);
   radeon_uvd_enc_op_init_rc(enc);
   radeon_uvd_enc_op_init_rc_vbv(enc);
   *enc->p_task_size = (enc->total_task_size);
}

static void encode(struct radeon_uvd_encoder *enc)
{
   radeon_uvd_enc_session_info(enc);
   enc->total_task_size = 0;
   radeon_uvd_enc_task_info(enc, enc->need_feedback);

   radeon_uvd_enc_nalu_aud_hevc(enc);

   if (enc->enc_pic.is_iframe) {
      radeon_uvd_enc_nalu_vps_hevc(enc);
      radeon_uvd_enc_nalu_pps_hevc(enc);
      radeon_uvd_enc_nalu_sps_hevc(enc);
   }
   radeon_uvd_enc_slice_header_hevc(enc);
   radeon_uvd_enc_encode_params_hevc(enc);

   radeon_uvd_enc_ctx(enc);
   radeon_uvd_enc_bitstream(enc);
   radeon_uvd_enc_feedback(enc);
   radeon_uvd_enc_intra_refresh(enc);

   radeon_uvd_enc_op_speed(enc);
   radeon_uvd_enc_op_enc(enc);
   *enc->p_task_size = (enc->total_task_size);
}

static void destroy(struct radeon_uvd_encoder *enc)
{
   radeon_uvd_enc_session_info(enc);
   enc->total_task_size = 0;
   radeon_uvd_enc_task_info(enc, enc->need_feedback);
   radeon_uvd_enc_op_close(enc);
   *enc->p_task_size = (enc->total_task_size);
}

void radeon_uvd_enc_1_1_init(struct radeon_uvd_encoder *enc)
{
   enc->begin = begin;
   enc->encode = encode;
   enc->destroy = destroy;
}
