/**************************************************************************
 *
 * Copyright 2017 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 *
 **************************************************************************/

#include "radeon_vcn_enc.h"
#include "ac_vcn_enc_av1_default_cdf.h"

#include "pipe/p_video_codec.h"
#include "radeon_video.h"
#include "radeonsi/si_pipe.h"
#include "util/u_memory.h"
#include "util/u_video.h"
#include "vl/vl_video_buffer.h"

#include <stdio.h>

static const unsigned index_to_shifts[4] = {24, 16, 8, 0};

/* set quality modes from the input */
static void radeon_vcn_enc_quality_modes(struct radeon_encoder *enc,
                                         struct pipe_enc_quality_modes *in)
{
   rvcn_enc_quality_modes_t *p = &enc->enc_pic.quality_modes;

   p->preset_mode = in->preset_mode > RENCODE_PRESET_MODE_HIGH_QUALITY
                                    ? RENCODE_PRESET_MODE_HIGH_QUALITY
                                    : in->preset_mode;

   if (u_reduce_video_profile(enc->base.profile) != PIPE_VIDEO_FORMAT_AV1 &&
       p->preset_mode == RENCODE_PRESET_MODE_HIGH_QUALITY)
      p->preset_mode = RENCODE_PRESET_MODE_QUALITY;

   p->pre_encode_mode = in->pre_encode_mode ? RENCODE_PREENCODE_MODE_4X
                                            : RENCODE_PREENCODE_MODE_NONE;
   p->vbaq_mode = in->vbaq_mode ? RENCODE_VBAQ_AUTO : RENCODE_VBAQ_NONE;
}

/* to process invalid frame rate */
static void radeon_vcn_enc_invalid_frame_rate(uint32_t *den, uint32_t *num)
{
   if (*den == 0 || *num == 0) {
      *den = 1;
      *num = 30;
   }
}

static uint32_t radeon_vcn_per_frame_integer(uint32_t bitrate, uint32_t den, uint32_t num)
{
   uint64_t rate_den = (uint64_t)bitrate * (uint64_t)den;

   return (uint32_t)(rate_den/num);
}

static uint32_t radeon_vcn_per_frame_frac(uint32_t bitrate, uint32_t den, uint32_t num)
{
   uint64_t rate_den = (uint64_t)bitrate * (uint64_t)den;
   uint64_t remainder = rate_den % num;

   return (uint32_t)((remainder << 32) / num);
}

/* block length for av1 and hevc is the same, 64, for avc 16 */
static uint32_t radeon_vcn_enc_blocks_in_frame(struct radeon_encoder *enc,
                                           uint32_t *width_in_block,
                                           uint32_t *height_in_block)
{
   bool is_h264 = u_reduce_video_profile(enc->base.profile) == PIPE_VIDEO_FORMAT_MPEG4_AVC;
   uint32_t block_length = is_h264 ? PIPE_H264_MB_SIZE : PIPE_H265_ENC_CTB_SIZE;

   *width_in_block  = PIPE_ALIGN_IN_BLOCK_SIZE(enc->base.width,  block_length);
   *height_in_block = PIPE_ALIGN_IN_BLOCK_SIZE(enc->base.height, block_length);

   return block_length;
}

static void radeon_vcn_enc_get_intra_refresh_param(struct radeon_encoder *enc,
                                                   bool need_filter_overlap,
                                                   struct pipe_enc_intra_refresh *intra_refresh)
{
   uint32_t width_in_block, height_in_block;

   enc->enc_pic.intra_refresh.intra_refresh_mode = RENCODE_INTRA_REFRESH_MODE_NONE;
   /* some exceptions where intra-refresh is disabled:
    * 1. if B frame is enabled
    * 2. if SVC (number of temproal layers is larger than 1) is enabled
    */
   if (enc->enc_pic.spec_misc.b_picture_enabled || enc->enc_pic.num_temporal_layers > 1) {
      enc->enc_pic.intra_refresh.region_size = 0;
      enc->enc_pic.intra_refresh.offset = 0;
      enc->enc_pic.need_sequence_header = 0;
      return;
   }

   radeon_vcn_enc_blocks_in_frame(enc, &width_in_block, &height_in_block);

   switch(intra_refresh->mode) {
      case INTRA_REFRESH_MODE_UNIT_ROWS:
         if (intra_refresh->offset < height_in_block)
            enc->enc_pic.intra_refresh.intra_refresh_mode
                                             = RENCODE_INTRA_REFRESH_MODE_CTB_MB_ROWS;
         break;
      case INTRA_REFRESH_MODE_UNIT_COLUMNS:
         if (intra_refresh->offset < width_in_block)
            enc->enc_pic.intra_refresh.intra_refresh_mode
                                             = RENCODE_INTRA_REFRESH_MODE_CTB_MB_COLUMNS;
         break;
      case INTRA_REFRESH_MODE_NONE:
      default:
         break;
   };

   /* with loop filters (avc/hevc/av1) enabled the region_size has to increase 1 to
    * get overlapped (av1 is enabling it all the time). The region_size and offset
    * require to be in unit of MB or CTB or SB according to different codecs.
    */
   if (enc->enc_pic.intra_refresh.intra_refresh_mode != RENCODE_INTRA_REFRESH_MODE_NONE) {
      enc->enc_pic.intra_refresh.region_size = (need_filter_overlap) ?
                                               intra_refresh->region_size + 1 :
                                               intra_refresh->region_size;
      enc->enc_pic.intra_refresh.offset = intra_refresh->offset;
      enc->enc_pic.need_sequence_header = !!(intra_refresh->need_sequence_header);
   } else {
      enc->enc_pic.intra_refresh.region_size = 0;
      enc->enc_pic.intra_refresh.offset = 0;
      enc->enc_pic.need_sequence_header = 0;
   }
}

static void radeon_vcn_enc_get_roi_param(struct radeon_encoder *enc,
                                         struct pipe_enc_roi *roi)
{
   bool is_av1 = u_reduce_video_profile(enc->base.profile)
                             == PIPE_VIDEO_FORMAT_AV1;
   if (!roi->num)
      enc->enc_pic.enc_qp_map.qp_map_type = RENCODE_QP_MAP_TYPE_NONE;
   else {
      uint32_t width_in_block, height_in_block;
      uint32_t block_length;
      int32_t i, j, pa_format = 0;

      /* rate control is using a different qp map type */
      if (enc->enc_pic.rc_session_init.rate_control_method) {
         enc->enc_pic.enc_qp_map.qp_map_type = RENCODE_QP_MAP_TYPE_MAP_PA;
         pa_format = 1;
      }
      else
         enc->enc_pic.enc_qp_map.qp_map_type = RENCODE_QP_MAP_TYPE_DELTA;

      block_length = radeon_vcn_enc_blocks_in_frame(enc, &width_in_block, &height_in_block);

      for (i = RENCODE_QP_MAP_MAX_REGIONS; i >= roi->num; i--)
         enc->enc_pic.enc_qp_map.map[i].is_valid = false;

      /* reverse the map sequence */
      for (j = 0; i >= 0; i--, j++) {
         struct rvcn_enc_qp_map_region *map = &enc->enc_pic.enc_qp_map.map[j];
         struct pipe_enc_region_in_roi *region = &roi->region[i];

         map->is_valid = region->valid;
         if (region->valid) {
            int32_t av1_qi_value;
            /* mapped av1 qi into the legacy qp range by dividing by 5 and
             * rounding up in any rate control mode.
             */
            if (is_av1 && pa_format) {
               if (region->qp_value > 0)
                  av1_qi_value = (region->qp_value + 2) / 5;
               else if (region->qp_value < 0)
                  av1_qi_value = (region->qp_value - 2) / 5;
               else
                  av1_qi_value = region->qp_value;
               map->qp_delta = av1_qi_value;
            } else
               map->qp_delta = region->qp_value;

            map->x_in_unit = CLAMP((region->x / block_length), 0, width_in_block - 1);
            map->y_in_unit = CLAMP((region->y / block_length), 0, height_in_block - 1);
            map->width_in_unit = CLAMP((region->width / block_length), 0, width_in_block);
            map->height_in_unit = CLAMP((region->height / block_length), 0, width_in_block);
         }
      }
   }
}

static void radeon_vcn_enc_h264_get_cropping_param(struct radeon_encoder *enc,
                                                   struct pipe_h264_enc_picture_desc *pic)
{
   if (pic->seq.enc_frame_cropping_flag) {
      enc->enc_pic.crop_left = pic->seq.enc_frame_crop_left_offset;
      enc->enc_pic.crop_right = pic->seq.enc_frame_crop_right_offset;
      enc->enc_pic.crop_top = pic->seq.enc_frame_crop_top_offset;
      enc->enc_pic.crop_bottom = pic->seq.enc_frame_crop_bottom_offset;
   } else {
      enc->enc_pic.crop_left = 0;
      enc->enc_pic.crop_right = (align(enc->base.width, 16) - enc->base.width) / 2;
      enc->enc_pic.crop_top = 0;
      enc->enc_pic.crop_bottom = (align(enc->base.height, 16) - enc->base.height) / 2;
   }
}

static void radeon_vcn_enc_h264_get_dbk_param(struct radeon_encoder *enc,
                                              struct pipe_h264_enc_picture_desc *pic)
{
   enc->enc_pic.h264_deblock.disable_deblocking_filter_idc =
      CLAMP(pic->dbk.disable_deblocking_filter_idc, 0, 2);
   enc->enc_pic.h264_deblock.alpha_c0_offset_div2 = pic->dbk.alpha_c0_offset_div2;
   enc->enc_pic.h264_deblock.beta_offset_div2 = pic->dbk.beta_offset_div2;
   enc->enc_pic.h264_deblock.cb_qp_offset = pic->pic_ctrl.chroma_qp_index_offset;
   enc->enc_pic.h264_deblock.cr_qp_offset = pic->pic_ctrl.second_chroma_qp_index_offset;
}

static void radeon_vcn_enc_h264_get_spec_misc_param(struct radeon_encoder *enc,
                                                    struct pipe_h264_enc_picture_desc *pic)
{
   enc->enc_pic.spec_misc.profile_idc = u_get_h264_profile_idc(enc->base.profile);
   if (enc->enc_pic.spec_misc.profile_idc >= PIPE_VIDEO_PROFILE_MPEG4_AVC_MAIN &&
         enc->enc_pic.spec_misc.profile_idc != PIPE_VIDEO_PROFILE_MPEG4_AVC_EXTENDED)
      enc->enc_pic.spec_misc.cabac_enable = pic->pic_ctrl.enc_cabac_enable;
   else
      enc->enc_pic.spec_misc.cabac_enable = false;

   enc->enc_pic.spec_misc.cabac_init_idc = enc->enc_pic.spec_misc.cabac_enable ?
                                           pic->pic_ctrl.enc_cabac_init_idc : 0;
   enc->enc_pic.spec_misc.deblocking_filter_control_present_flag =
      pic->pic_ctrl.deblocking_filter_control_present_flag;
   enc->enc_pic.spec_misc.redundant_pic_cnt_present_flag =
      pic->pic_ctrl.redundant_pic_cnt_present_flag;
   enc->enc_pic.spec_misc.b_picture_enabled = !!pic->seq.max_num_reorder_frames;
}

static void radeon_vcn_enc_h264_get_rc_param(struct radeon_encoder *enc,
                                             struct pipe_h264_enc_picture_desc *pic)
{
   uint32_t frame_rate_den, frame_rate_num;

   enc->enc_pic.num_temporal_layers = pic->seq.num_temporal_layers ? pic->seq.num_temporal_layers : 1;
   for (int i = 0; i < enc->enc_pic.num_temporal_layers; i++) {
      enc->enc_pic.rc_layer_init[i].target_bit_rate = pic->rate_ctrl[i].target_bitrate;
      enc->enc_pic.rc_layer_init[i].peak_bit_rate = pic->rate_ctrl[i].peak_bitrate;
      frame_rate_den = pic->rate_ctrl[i].frame_rate_den;
      frame_rate_num = pic->rate_ctrl[i].frame_rate_num;
      radeon_vcn_enc_invalid_frame_rate(&frame_rate_den, &frame_rate_num);
      enc->enc_pic.rc_layer_init[i].frame_rate_den = frame_rate_den;
      enc->enc_pic.rc_layer_init[i].frame_rate_num = frame_rate_num;
      enc->enc_pic.rc_layer_init[i].vbv_buffer_size = pic->rate_ctrl[i].vbv_buffer_size;
      enc->enc_pic.rc_layer_init[i].avg_target_bits_per_picture =
         radeon_vcn_per_frame_integer(pic->rate_ctrl[i].target_bitrate,
               frame_rate_den,
               frame_rate_num);
      enc->enc_pic.rc_layer_init[i].peak_bits_per_picture_integer =
         radeon_vcn_per_frame_integer(pic->rate_ctrl[i].peak_bitrate,
               frame_rate_den,
               frame_rate_num);
      enc->enc_pic.rc_layer_init[i].peak_bits_per_picture_fractional =
         radeon_vcn_per_frame_frac(pic->rate_ctrl[i].peak_bitrate,
               frame_rate_den,
               frame_rate_num);
   }
   enc->enc_pic.rc_session_init.vbv_buffer_level = pic->rate_ctrl[0].vbv_buf_lv;
   enc->enc_pic.rc_per_pic.qp = pic->quant_i_frames;
   enc->enc_pic.rc_per_pic.min_qp_app = pic->rate_ctrl[0].min_qp;
   enc->enc_pic.rc_per_pic.max_qp_app = pic->rate_ctrl[0].max_qp ?
      pic->rate_ctrl[0].max_qp : 51;
   enc->enc_pic.rc_per_pic.enabled_filler_data = pic->rate_ctrl[0].fill_data_enable;
   enc->enc_pic.rc_per_pic.skip_frame_enable = pic->rate_ctrl[0].skip_frame_enable;
   enc->enc_pic.rc_per_pic.enforce_hrd = pic->rate_ctrl[0].enforce_hrd;

   switch (pic->rate_ctrl[0].rate_ctrl_method) {
      case PIPE_H2645_ENC_RATE_CONTROL_METHOD_DISABLE:
         enc->enc_pic.rc_session_init.rate_control_method = RENCODE_RATE_CONTROL_METHOD_NONE;
         break;
      case PIPE_H2645_ENC_RATE_CONTROL_METHOD_CONSTANT_SKIP:
      case PIPE_H2645_ENC_RATE_CONTROL_METHOD_CONSTANT:
         enc->enc_pic.rc_session_init.rate_control_method = RENCODE_RATE_CONTROL_METHOD_CBR;
         break;
      case PIPE_H2645_ENC_RATE_CONTROL_METHOD_VARIABLE_SKIP:
      case PIPE_H2645_ENC_RATE_CONTROL_METHOD_VARIABLE:
         enc->enc_pic.rc_session_init.rate_control_method =
            RENCODE_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR;
         break;
      default:
         enc->enc_pic.rc_session_init.rate_control_method = RENCODE_RATE_CONTROL_METHOD_NONE;
   }
   enc->enc_pic.rc_per_pic.max_au_size = pic->rate_ctrl[0].max_au_size;
}

static void radeon_vcn_enc_h264_get_vui_param(struct radeon_encoder *enc,
                                              struct pipe_h264_enc_picture_desc *pic)
{
   enc->enc_pic.vui_info.vui_parameters_present_flag =
      pic->seq.vui_parameters_present_flag;
   enc->enc_pic.vui_info.flags.aspect_ratio_info_present_flag =
      pic->seq.vui_flags.aspect_ratio_info_present_flag;
   enc->enc_pic.vui_info.flags.timing_info_present_flag =
      pic->seq.vui_flags.timing_info_present_flag;
   enc->enc_pic.vui_info.flags.video_signal_type_present_flag =
      pic->seq.vui_flags.video_signal_type_present_flag;
   enc->enc_pic.vui_info.flags.colour_description_present_flag =
      pic->seq.vui_flags.colour_description_present_flag;
   enc->enc_pic.vui_info.flags.chroma_loc_info_present_flag =
      pic->seq.vui_flags.chroma_loc_info_present_flag;
   enc->enc_pic.vui_info.aspect_ratio_idc = pic->seq.aspect_ratio_idc;
   enc->enc_pic.vui_info.sar_width = pic->seq.sar_width;
   enc->enc_pic.vui_info.sar_height = pic->seq.sar_height;
   enc->enc_pic.vui_info.num_units_in_tick = pic->seq.num_units_in_tick;
   enc->enc_pic.vui_info.time_scale = pic->seq.time_scale;
   enc->enc_pic.vui_info.video_format = pic->seq.video_format;
   enc->enc_pic.vui_info.video_full_range_flag = pic->seq.video_full_range_flag;
   enc->enc_pic.vui_info.colour_primaries = pic->seq.colour_primaries;
   enc->enc_pic.vui_info.transfer_characteristics = pic->seq.transfer_characteristics;
   enc->enc_pic.vui_info.matrix_coefficients = pic->seq.matrix_coefficients;
   enc->enc_pic.vui_info.chroma_sample_loc_type_top_field =
      pic->seq.chroma_sample_loc_type_top_field;
   enc->enc_pic.vui_info.chroma_sample_loc_type_bottom_field =
      pic->seq.chroma_sample_loc_type_bottom_field;
   enc->enc_pic.vui_info.max_num_reorder_frames = pic->seq.max_num_reorder_frames;
}

/* only checking the first slice to get num of mbs in slice to
 * determine the number of slices in this frame, only fixed MB mode
 * is supported now, the last slice in frame could have less number of
 * MBs.
 */
static void radeon_vcn_enc_h264_get_slice_ctrl_param(struct radeon_encoder *enc,
                                                     struct pipe_h264_enc_picture_desc *pic)
{
   uint32_t width_in_mb, height_in_mb, num_mbs_in_slice;

   width_in_mb = PIPE_ALIGN_IN_BLOCK_SIZE(enc->base.width, PIPE_H264_MB_SIZE);
   height_in_mb = PIPE_ALIGN_IN_BLOCK_SIZE(enc->base.height, PIPE_H264_MB_SIZE);

   if (pic->slices_descriptors[0].num_macroblocks >= width_in_mb * height_in_mb ||
       pic->slices_descriptors[0].num_macroblocks == 0)
      num_mbs_in_slice = width_in_mb * height_in_mb;
   else
      num_mbs_in_slice = pic->slices_descriptors[0].num_macroblocks;

   enc->enc_pic.slice_ctrl.num_mbs_per_slice = num_mbs_in_slice;
}

static void radeon_vcn_enc_get_output_format_param(struct radeon_encoder *enc, bool full_range)
{
   switch (enc->enc_pic.bit_depth_luma_minus8) {
   case 2: /* 10 bits */
      enc->enc_pic.enc_output_format.output_color_volume = RENCODE_COLOR_VOLUME_G22_BT709;
      enc->enc_pic.enc_output_format.output_color_range = full_range ?
         RENCODE_COLOR_RANGE_FULL : RENCODE_COLOR_RANGE_STUDIO;
      enc->enc_pic.enc_output_format.output_chroma_location = RENCODE_CHROMA_LOCATION_INTERSTITIAL;
      enc->enc_pic.enc_output_format.output_color_bit_depth = RENCODE_COLOR_BIT_DEPTH_10_BIT;
      break;
   default: /* 8 bits */
      enc->enc_pic.enc_output_format.output_color_volume = RENCODE_COLOR_VOLUME_G22_BT709;
      enc->enc_pic.enc_output_format.output_color_range = full_range ?
         RENCODE_COLOR_RANGE_FULL : RENCODE_COLOR_RANGE_STUDIO;
      enc->enc_pic.enc_output_format.output_chroma_location = RENCODE_CHROMA_LOCATION_INTERSTITIAL;
      enc->enc_pic.enc_output_format.output_color_bit_depth = RENCODE_COLOR_BIT_DEPTH_8_BIT;
      break;
   }
}

static void radeon_vcn_enc_get_input_format_param(struct radeon_encoder *enc,
                                                  struct pipe_picture_desc *pic_base)
{
   switch (pic_base->input_format) {
   case PIPE_FORMAT_P010:
      enc->enc_pic.enc_input_format.input_color_bit_depth = RENCODE_COLOR_BIT_DEPTH_10_BIT;
      enc->enc_pic.enc_input_format.input_color_packing_format = RENCODE_COLOR_PACKING_FORMAT_P010;
      enc->enc_pic.enc_input_format.input_chroma_subsampling = RENCODE_CHROMA_SUBSAMPLING_4_2_0;
      enc->enc_pic.enc_input_format.input_color_space = RENCODE_COLOR_SPACE_YUV;
      break;
   case PIPE_FORMAT_B8G8R8A8_UNORM:
   case PIPE_FORMAT_B8G8R8X8_UNORM:
      enc->enc_pic.enc_input_format.input_color_bit_depth = RENCODE_COLOR_BIT_DEPTH_8_BIT;
      enc->enc_pic.enc_input_format.input_chroma_subsampling = RENCODE_CHROMA_SUBSAMPLING_4_4_4;
      enc->enc_pic.enc_input_format.input_color_packing_format = RENCODE_COLOR_PACKING_FORMAT_A8R8G8B8;
      enc->enc_pic.enc_input_format.input_color_space = RENCODE_COLOR_SPACE_RGB;
      break;
   case PIPE_FORMAT_R8G8B8A8_UNORM:
   case PIPE_FORMAT_R8G8B8X8_UNORM:
      enc->enc_pic.enc_input_format.input_color_bit_depth = RENCODE_COLOR_BIT_DEPTH_8_BIT;
      enc->enc_pic.enc_input_format.input_chroma_subsampling = RENCODE_CHROMA_SUBSAMPLING_4_4_4;
      enc->enc_pic.enc_input_format.input_color_packing_format = RENCODE_COLOR_PACKING_FORMAT_A8B8G8R8;
      enc->enc_pic.enc_input_format.input_color_space = RENCODE_COLOR_SPACE_RGB;
      break;
   case PIPE_FORMAT_NV12: /* FALL THROUGH */
   default:
      enc->enc_pic.enc_input_format.input_color_bit_depth = RENCODE_COLOR_BIT_DEPTH_8_BIT;
      enc->enc_pic.enc_input_format.input_color_packing_format = RENCODE_COLOR_PACKING_FORMAT_NV12;
      enc->enc_pic.enc_input_format.input_chroma_subsampling = RENCODE_CHROMA_SUBSAMPLING_4_2_0;
      enc->enc_pic.enc_input_format.input_color_space = RENCODE_COLOR_SPACE_YUV;
      break;
   }

  enc->enc_pic.enc_input_format.input_color_volume = RENCODE_COLOR_VOLUME_G22_BT709;
  enc->enc_pic.enc_input_format.input_color_range = pic_base->input_full_range ?
     RENCODE_COLOR_RANGE_FULL : RENCODE_COLOR_RANGE_STUDIO;
   enc->enc_pic.enc_input_format.input_chroma_location = RENCODE_CHROMA_LOCATION_INTERSTITIAL;
}

static void radeon_vcn_enc_h264_get_param(struct radeon_encoder *enc,
                                          struct pipe_h264_enc_picture_desc *pic)
{
   bool use_filter;

   enc->enc_pic.picture_type = pic->picture_type;
   enc->enc_pic.bit_depth_luma_minus8 = 0;
   enc->enc_pic.bit_depth_chroma_minus8 = 0;
   radeon_vcn_enc_quality_modes(enc, &pic->quality_modes);
   enc->enc_pic.frame_num = pic->frame_num;
   enc->enc_pic.pic_order_cnt = pic->pic_order_cnt;
   enc->enc_pic.pic_order_cnt_type = pic->seq.pic_order_cnt_type;
   enc->enc_pic.ref_idx_l0 = pic->ref_idx_l0_list[0];
   enc->enc_pic.ref_idx_l0_is_ltr = pic->l0_is_long_term[0];
   enc->enc_pic.ref_idx_l1 = pic->ref_idx_l1_list[0];
   enc->enc_pic.ref_idx_l1_is_ltr = pic->l1_is_long_term[0];
   enc->enc_pic.not_referenced = pic->not_referenced;
   enc->enc_pic.is_idr = (pic->picture_type == PIPE_H2645_ENC_PICTURE_TYPE_IDR);
   enc->enc_pic.is_ltr = pic->is_ltr;
   enc->enc_pic.ltr_idx = pic->is_ltr ? pic->ltr_index : 0;
   radeon_vcn_enc_h264_get_cropping_param(enc, pic);
   radeon_vcn_enc_h264_get_dbk_param(enc, pic);
   radeon_vcn_enc_h264_get_rc_param(enc, pic);
   radeon_vcn_enc_h264_get_spec_misc_param(enc, pic);
   radeon_vcn_enc_h264_get_vui_param(enc, pic);
   radeon_vcn_enc_h264_get_slice_ctrl_param(enc, pic);
   radeon_vcn_enc_get_input_format_param(enc, &pic->base);
   radeon_vcn_enc_get_output_format_param(enc, pic->seq.video_full_range_flag);

   use_filter = enc->enc_pic.h264_deblock.disable_deblocking_filter_idc != 1;
   radeon_vcn_enc_get_intra_refresh_param(enc, use_filter, &pic->intra_refresh);
   radeon_vcn_enc_get_roi_param(enc, &pic->roi);
}

static void radeon_vcn_enc_hevc_get_cropping_param(struct radeon_encoder *enc,
                                                   struct pipe_h265_enc_picture_desc *pic)
{
   if (pic->seq.conformance_window_flag) {
      enc->enc_pic.crop_left = pic->seq.conf_win_left_offset;
      enc->enc_pic.crop_right = pic->seq.conf_win_right_offset;
      enc->enc_pic.crop_top = pic->seq.conf_win_top_offset;
      enc->enc_pic.crop_bottom = pic->seq.conf_win_bottom_offset;
   } else {
      enc->enc_pic.crop_left = 0;
      enc->enc_pic.crop_right = (align(enc->base.width, 16) - enc->base.width) / 2;
      enc->enc_pic.crop_top = 0;
      enc->enc_pic.crop_bottom = (align(enc->base.height, 16) - enc->base.height) / 2;
   }
}

static void radeon_vcn_enc_hevc_get_dbk_param(struct radeon_encoder *enc,
                                              struct pipe_h265_enc_picture_desc *pic)
{
   enc->enc_pic.hevc_deblock.loop_filter_across_slices_enabled =
      pic->slice.slice_loop_filter_across_slices_enabled_flag;
   enc->enc_pic.hevc_deblock.deblocking_filter_disabled =
      pic->slice.slice_deblocking_filter_disabled_flag;
   enc->enc_pic.hevc_deblock.beta_offset_div2 = pic->slice.slice_beta_offset_div2;
   enc->enc_pic.hevc_deblock.tc_offset_div2 = pic->slice.slice_tc_offset_div2;
   enc->enc_pic.hevc_deblock.cb_qp_offset = pic->slice.slice_cb_qp_offset;
   enc->enc_pic.hevc_deblock.cr_qp_offset = pic->slice.slice_cr_qp_offset;
}

static void radeon_vcn_enc_hevc_get_spec_misc_param(struct radeon_encoder *enc,
                                                    struct pipe_h265_enc_picture_desc *pic)
{
   enc->enc_pic.hevc_spec_misc.log2_min_luma_coding_block_size_minus3 =
      pic->seq.log2_min_luma_coding_block_size_minus3;
   enc->enc_pic.hevc_spec_misc.amp_disabled = !pic->seq.amp_enabled_flag;
   enc->enc_pic.hevc_spec_misc.strong_intra_smoothing_enabled =
      pic->seq.strong_intra_smoothing_enabled_flag;
   enc->enc_pic.hevc_spec_misc.constrained_intra_pred_flag =
      pic->pic.constrained_intra_pred_flag;
   enc->enc_pic.hevc_spec_misc.cabac_init_flag = pic->slice.cabac_init_flag;
   enc->enc_pic.hevc_spec_misc.half_pel_enabled = 1;
   enc->enc_pic.hevc_spec_misc.quarter_pel_enabled = 1;
}

static void radeon_vcn_enc_hevc_get_rc_param(struct radeon_encoder *enc,
                                             struct pipe_h265_enc_picture_desc *pic)
{
   uint32_t frame_rate_den, frame_rate_num;

   enc->enc_pic.rc_layer_init[0].target_bit_rate = pic->rc.target_bitrate;
   enc->enc_pic.rc_layer_init[0].peak_bit_rate = pic->rc.peak_bitrate;
   frame_rate_den = pic->rc.frame_rate_den;
   frame_rate_num = pic->rc.frame_rate_num;
   radeon_vcn_enc_invalid_frame_rate(&frame_rate_den, &frame_rate_num);
   enc->enc_pic.rc_layer_init[0].frame_rate_den = frame_rate_den;
   enc->enc_pic.rc_layer_init[0].frame_rate_num = frame_rate_num;
   enc->enc_pic.rc_layer_init[0].vbv_buffer_size = pic->rc.vbv_buffer_size;
   enc->enc_pic.rc_layer_init[0].avg_target_bits_per_picture =
      radeon_vcn_per_frame_integer(pic->rc.target_bitrate,
            frame_rate_den,
            frame_rate_num);
   enc->enc_pic.rc_layer_init[0].peak_bits_per_picture_integer =
      radeon_vcn_per_frame_integer(pic->rc.peak_bitrate,
            frame_rate_den,
            frame_rate_num);
   enc->enc_pic.rc_layer_init[0].peak_bits_per_picture_fractional =
      radeon_vcn_per_frame_frac(pic->rc.peak_bitrate,
            frame_rate_den,
            frame_rate_num);
   enc->enc_pic.rc_session_init.vbv_buffer_level = pic->rc.vbv_buf_lv;
   enc->enc_pic.rc_per_pic.qp = pic->rc.quant_i_frames;
   enc->enc_pic.rc_per_pic.min_qp_app = pic->rc.min_qp;
   enc->enc_pic.rc_per_pic.max_qp_app = pic->rc.max_qp ? pic->rc.max_qp : 51;
   enc->enc_pic.rc_per_pic.enabled_filler_data = pic->rc.fill_data_enable;
   enc->enc_pic.rc_per_pic.skip_frame_enable = pic->rc.skip_frame_enable;
   enc->enc_pic.rc_per_pic.enforce_hrd = pic->rc.enforce_hrd;
   switch (pic->rc.rate_ctrl_method) {
      case PIPE_H2645_ENC_RATE_CONTROL_METHOD_DISABLE:
         enc->enc_pic.rc_session_init.rate_control_method = RENCODE_RATE_CONTROL_METHOD_NONE;
         break;
      case PIPE_H2645_ENC_RATE_CONTROL_METHOD_CONSTANT_SKIP:
      case PIPE_H2645_ENC_RATE_CONTROL_METHOD_CONSTANT:
         enc->enc_pic.rc_session_init.rate_control_method = RENCODE_RATE_CONTROL_METHOD_CBR;
         break;
      case PIPE_H2645_ENC_RATE_CONTROL_METHOD_VARIABLE_SKIP:
      case PIPE_H2645_ENC_RATE_CONTROL_METHOD_VARIABLE:
         enc->enc_pic.rc_session_init.rate_control_method =
            RENCODE_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR;
         break;
      default:
         enc->enc_pic.rc_session_init.rate_control_method = RENCODE_RATE_CONTROL_METHOD_NONE;
   }
   enc->enc_pic.rc_per_pic.max_au_size = pic->rc.max_au_size;
}

static void radeon_vcn_enc_hevc_get_vui_param(struct radeon_encoder *enc,
                                              struct pipe_h265_enc_picture_desc *pic)
{
   enc->enc_pic.vui_info.vui_parameters_present_flag = pic->seq.vui_parameters_present_flag;
   enc->enc_pic.vui_info.flags.aspect_ratio_info_present_flag =
      pic->seq.vui_flags.aspect_ratio_info_present_flag;
   enc->enc_pic.vui_info.flags.timing_info_present_flag =
      pic->seq.vui_flags.timing_info_present_flag;
   enc->enc_pic.vui_info.flags.video_signal_type_present_flag =
      pic->seq.vui_flags.video_signal_type_present_flag;
   enc->enc_pic.vui_info.flags.colour_description_present_flag =
      pic->seq.vui_flags.colour_description_present_flag;
   enc->enc_pic.vui_info.flags.chroma_loc_info_present_flag =
      pic->seq.vui_flags.chroma_loc_info_present_flag;
   enc->enc_pic.vui_info.aspect_ratio_idc = pic->seq.aspect_ratio_idc;
   enc->enc_pic.vui_info.sar_width = pic->seq.sar_width;
   enc->enc_pic.vui_info.sar_height = pic->seq.sar_height;
   enc->enc_pic.vui_info.num_units_in_tick = pic->seq.num_units_in_tick;
   enc->enc_pic.vui_info.time_scale = pic->seq.time_scale;
   enc->enc_pic.vui_info.video_format = pic->seq.video_format;
   enc->enc_pic.vui_info.video_full_range_flag = pic->seq.video_full_range_flag;
   enc->enc_pic.vui_info.colour_primaries = pic->seq.colour_primaries;
   enc->enc_pic.vui_info.transfer_characteristics = pic->seq.transfer_characteristics;
   enc->enc_pic.vui_info.matrix_coefficients = pic->seq.matrix_coefficients;
   enc->enc_pic.vui_info.chroma_sample_loc_type_top_field =
      pic->seq.chroma_sample_loc_type_top_field;
   enc->enc_pic.vui_info.chroma_sample_loc_type_bottom_field =
      pic->seq.chroma_sample_loc_type_bottom_field;
}

/* only checking the first slice to get num of ctbs in slice to
 * determine the number of slices in this frame, only fixed CTB mode
 * is supported now, the last slice in frame could have less number of
 * ctbs.
 */
static void radeon_vcn_enc_hevc_get_slice_ctrl_param(struct radeon_encoder *enc,
                                                     struct pipe_h265_enc_picture_desc *pic)
{
   uint32_t width_in_ctb, height_in_ctb, num_ctbs_in_slice;

   width_in_ctb = PIPE_ALIGN_IN_BLOCK_SIZE(pic->seq.pic_width_in_luma_samples,
                                           PIPE_H265_ENC_CTB_SIZE);
   height_in_ctb = PIPE_ALIGN_IN_BLOCK_SIZE(pic->seq.pic_height_in_luma_samples,
                                            PIPE_H265_ENC_CTB_SIZE);

   if (pic->slices_descriptors[0].num_ctu_in_slice >= width_in_ctb * height_in_ctb ||
       pic->slices_descriptors[0].num_ctu_in_slice == 0)
      num_ctbs_in_slice = width_in_ctb * height_in_ctb;
   else
      num_ctbs_in_slice = pic->slices_descriptors[0].num_ctu_in_slice;

   enc->enc_pic.hevc_slice_ctrl.fixed_ctbs_per_slice.num_ctbs_per_slice =
      num_ctbs_in_slice;

   enc->enc_pic.hevc_slice_ctrl.fixed_ctbs_per_slice.num_ctbs_per_slice_segment =
      num_ctbs_in_slice;
}

static void radeon_vcn_enc_hevc_get_param(struct radeon_encoder *enc,
                                          struct pipe_h265_enc_picture_desc *pic)
{
   enc->enc_pic.picture_type = pic->picture_type;
   enc->enc_pic.frame_num = pic->frame_num;
   radeon_vcn_enc_quality_modes(enc, &pic->quality_modes);
   enc->enc_pic.pic_order_cnt_type = pic->pic_order_cnt_type;
   enc->enc_pic.ref_idx_l0 = pic->ref_idx_l0_list[0];
   enc->enc_pic.ref_idx_l1 = pic->ref_idx_l1_list[0];
   enc->enc_pic.not_referenced = pic->not_referenced;
   enc->enc_pic.is_idr = (pic->picture_type == PIPE_H2645_ENC_PICTURE_TYPE_IDR);
   radeon_vcn_enc_hevc_get_cropping_param(enc, pic);
   enc->enc_pic.general_tier_flag = pic->seq.general_tier_flag;
   enc->enc_pic.general_profile_idc = pic->seq.general_profile_idc;
   enc->enc_pic.general_level_idc = pic->seq.general_level_idc;
   /* use fixed value for max_poc until new feature added */
   enc->enc_pic.max_poc = 16;
   enc->enc_pic.log2_max_poc = 4;
   enc->enc_pic.num_temporal_layers = 1;
   enc->enc_pic.pic_order_cnt = pic->pic_order_cnt % enc->enc_pic.max_poc;
   enc->enc_pic.chroma_format_idc = pic->seq.chroma_format_idc;
   enc->enc_pic.pic_width_in_luma_samples = pic->seq.pic_width_in_luma_samples;
   enc->enc_pic.pic_height_in_luma_samples = pic->seq.pic_height_in_luma_samples;
   enc->enc_pic.log2_diff_max_min_luma_coding_block_size =
      pic->seq.log2_diff_max_min_luma_coding_block_size;
   enc->enc_pic.log2_min_transform_block_size_minus2 =
      pic->seq.log2_min_transform_block_size_minus2;
   enc->enc_pic.log2_diff_max_min_transform_block_size =
      pic->seq.log2_diff_max_min_transform_block_size;

   /* To fix incorrect hardcoded values set by player
    * log2_diff_max_min_luma_coding_block_size = log2(64) - (log2_min_luma_coding_block_size_minus3 + 3)
    * max_transform_hierarchy_depth_inter = log2_diff_max_min_luma_coding_block_size + 1
    * max_transform_hierarchy_depth_intra = log2_diff_max_min_luma_coding_block_size + 1
    */
   enc->enc_pic.max_transform_hierarchy_depth_inter =
      6 - (pic->seq.log2_min_luma_coding_block_size_minus3 + 3) + 1;
   enc->enc_pic.max_transform_hierarchy_depth_intra =
      enc->enc_pic.max_transform_hierarchy_depth_inter;

   enc->enc_pic.log2_parallel_merge_level_minus2 = pic->pic.log2_parallel_merge_level_minus2;
   enc->enc_pic.bit_depth_luma_minus8 = pic->seq.bit_depth_luma_minus8;
   enc->enc_pic.bit_depth_chroma_minus8 = pic->seq.bit_depth_chroma_minus8;
   enc->enc_pic.nal_unit_type = pic->pic.nal_unit_type;
   enc->enc_pic.max_num_merge_cand = pic->slice.max_num_merge_cand;
   enc->enc_pic.sample_adaptive_offset_enabled_flag =
      pic->seq.sample_adaptive_offset_enabled_flag;
   enc->enc_pic.pcm_enabled_flag = pic->seq.pcm_enabled_flag;
   enc->enc_pic.sps_temporal_mvp_enabled_flag = pic->seq.sps_temporal_mvp_enabled_flag;
   radeon_vcn_enc_hevc_get_spec_misc_param(enc, pic);
   radeon_vcn_enc_hevc_get_dbk_param(enc, pic);
   radeon_vcn_enc_hevc_get_rc_param(enc, pic);
   radeon_vcn_enc_hevc_get_vui_param(enc, pic);
   radeon_vcn_enc_hevc_get_slice_ctrl_param(enc, pic);
   radeon_vcn_enc_get_input_format_param(enc, &pic->base);
   radeon_vcn_enc_get_output_format_param(enc, pic->seq.video_full_range_flag);
   radeon_vcn_enc_get_intra_refresh_param(enc,
                                        !(enc->enc_pic.hevc_deblock.deblocking_filter_disabled),
                                         &pic->intra_refresh);
   radeon_vcn_enc_get_roi_param(enc, &pic->roi);
}

static void radeon_vcn_enc_av1_get_spec_misc_param(struct radeon_encoder *enc,
                                                   struct pipe_av1_enc_picture_desc *pic)
{
   enc->enc_pic.av1_spec_misc.cdef_mode = pic->seq.seq_bits.enable_cdef;
   enc->enc_pic.av1_spec_misc.disable_cdf_update = pic->disable_cdf_update;
   enc->enc_pic.av1_spec_misc.disable_frame_end_update_cdf = pic->disable_frame_end_update_cdf;
   enc->enc_pic.av1_spec_misc.num_tiles_per_picture = pic->num_tiles_in_pic;
   enc->enc_pic.av1_spec_misc.palette_mode_enable = pic->palette_mode_enable;

   if (enc->enc_pic.disable_screen_content_tools) {
       enc->enc_pic.force_integer_mv  = 0;
       enc->enc_pic.av1_spec_misc.palette_mode_enable = 0;
   }

   if (enc->enc_pic.force_integer_mv)
      enc->enc_pic.av1_spec_misc.mv_precision = RENCODE_AV1_MV_PRECISION_FORCE_INTEGER_MV;
   else
      enc->enc_pic.av1_spec_misc.mv_precision = RENCODE_AV1_MV_PRECISION_ALLOW_HIGH_PRECISION;
}

static void radeon_vcn_enc_av1_timing_info(struct radeon_encoder *enc,
                                           struct pipe_av1_enc_picture_desc *pic)
{
   if (pic->seq.seq_bits.timing_info_present_flag)
   {
      enc->enc_pic.av1_timing_info.num_units_in_display_tick =
         pic->seq.num_units_in_display_tick;
      enc->enc_pic.av1_timing_info.time_scale = pic->seq.time_scale;
      enc->enc_pic.av1_timing_info.num_tick_per_picture_minus1 =
         pic->seq.num_tick_per_picture_minus1;
   }
}

static void radeon_vcn_enc_av1_color_description(struct radeon_encoder *enc,
                                                 struct pipe_av1_enc_picture_desc *pic)
{
   if (pic->seq.seq_bits.color_description_present_flag)
   {
      enc->enc_pic.av1_color_description.color_primaries = pic->seq.color_config.color_primaries;
      enc->enc_pic.av1_color_description.transfer_characteristics = pic->seq.color_config.transfer_characteristics;
      enc->enc_pic.av1_color_description.maxtrix_coefficients = pic->seq.color_config.matrix_coefficients;
   }
   enc->enc_pic.av1_color_description.color_range = pic->seq.color_config.color_range;
   enc->enc_pic.av1_color_description.chroma_sample_position = pic->seq.color_config.chroma_sample_position;
}

static void radeon_vcn_enc_av1_get_rc_param(struct radeon_encoder *enc,
                                            struct pipe_av1_enc_picture_desc *pic)
{
   uint32_t frame_rate_den, frame_rate_num;

   for (int i = 0; i < ARRAY_SIZE(enc->enc_pic.rc_layer_init); i++) {
      enc->enc_pic.rc_layer_init[i].target_bit_rate = pic->rc[i].target_bitrate;
      enc->enc_pic.rc_layer_init[i].peak_bit_rate = pic->rc[i].peak_bitrate;
      frame_rate_den = pic->rc[i].frame_rate_den;
      frame_rate_num = pic->rc[i].frame_rate_num;
      radeon_vcn_enc_invalid_frame_rate(&frame_rate_den, &frame_rate_num);
      enc->enc_pic.rc_layer_init[i].frame_rate_den = frame_rate_den;
      enc->enc_pic.rc_layer_init[i].frame_rate_num = frame_rate_num;
      enc->enc_pic.rc_layer_init[i].vbv_buffer_size = pic->rc[i].vbv_buffer_size;
      enc->enc_pic.rc_layer_init[i].avg_target_bits_per_picture =
          radeon_vcn_per_frame_integer(pic->rc[i].target_bitrate,
                                       frame_rate_den,
                                       frame_rate_num);
      enc->enc_pic.rc_layer_init[i].peak_bits_per_picture_integer =
          radeon_vcn_per_frame_integer(pic->rc[i].peak_bitrate,
                                       frame_rate_den,
                                       frame_rate_num);
      enc->enc_pic.rc_layer_init[i].peak_bits_per_picture_fractional =
          radeon_vcn_per_frame_frac(pic->rc[i].peak_bitrate,
                                    frame_rate_den,
                                    frame_rate_num);
   }
   enc->enc_pic.rc_session_init.vbv_buffer_level = pic->rc[0].vbv_buf_lv;
   enc->enc_pic.rc_per_pic.qp = pic->rc[0].qp;
   enc->enc_pic.rc_per_pic.min_qp_app = pic->rc[0].min_qp ? pic->rc[0].min_qp : 1;
   enc->enc_pic.rc_per_pic.max_qp_app = pic->rc[0].max_qp ? pic->rc[0].max_qp : 255;
   enc->enc_pic.rc_per_pic.enabled_filler_data = pic->rc[0].fill_data_enable;
   enc->enc_pic.rc_per_pic.skip_frame_enable = pic->rc[0].skip_frame_enable;
   enc->enc_pic.rc_per_pic.enforce_hrd = pic->rc[0].enforce_hrd;
   switch (pic->rc[0].rate_ctrl_method) {
      case PIPE_H2645_ENC_RATE_CONTROL_METHOD_DISABLE:
         enc->enc_pic.rc_session_init.rate_control_method = RENCODE_RATE_CONTROL_METHOD_NONE;
         break;
      case PIPE_H2645_ENC_RATE_CONTROL_METHOD_CONSTANT_SKIP:
      case PIPE_H2645_ENC_RATE_CONTROL_METHOD_CONSTANT:
         enc->enc_pic.rc_session_init.rate_control_method = RENCODE_RATE_CONTROL_METHOD_CBR;
         break;
      case PIPE_H2645_ENC_RATE_CONTROL_METHOD_VARIABLE_SKIP:
      case PIPE_H2645_ENC_RATE_CONTROL_METHOD_VARIABLE:
         enc->enc_pic.rc_session_init.rate_control_method =
            RENCODE_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR;
         break;
      default:
         enc->enc_pic.rc_session_init.rate_control_method = RENCODE_RATE_CONTROL_METHOD_NONE;
   }
   enc->enc_pic.rc_per_pic.max_au_size = pic->rc[0].max_au_size;
}

static void radeon_vcn_enc_av1_get_param(struct radeon_encoder *enc,
                                         struct pipe_av1_enc_picture_desc *pic)
{
   struct radeon_enc_pic *enc_pic = &enc->enc_pic;
   enc_pic->frame_type = pic->frame_type;
   enc_pic->frame_num = pic->frame_num;
   enc_pic->bit_depth_luma_minus8 = enc_pic->bit_depth_chroma_minus8 =
      pic->seq.bit_depth_minus8;
   enc_pic->pic_width_in_luma_samples = pic->seq.pic_width_in_luma_samples;
   enc_pic->pic_height_in_luma_samples = pic->seq.pic_height_in_luma_samples;
   enc_pic->general_profile_idc = pic->seq.profile;
   enc_pic->general_level_idc = pic->seq.level;
   enc_pic->general_tier_flag = pic->seq.tier;

   enc_pic->num_temporal_layers =
            pic->seq.num_temporal_layers <= RENCODE_MAX_NUM_TEMPORAL_LAYERS ?
            pic->seq.num_temporal_layers : RENCODE_MAX_NUM_TEMPORAL_LAYERS;

   /* 1, 2 layer needs 1 reference, and 3, 4 layer needs 2 references */
   enc->base.max_references = (enc_pic->num_temporal_layers + 1) / 2;
   radeon_vcn_enc_quality_modes(enc, &pic->quality_modes);
   enc_pic->frame_id_numbers_present = pic->seq.seq_bits.frame_id_number_present_flag;
   enc_pic->enable_error_resilient_mode = pic->error_resilient_mode;
   enc_pic->force_integer_mv = pic->force_integer_mv;
   enc_pic->enable_order_hint = pic->seq.seq_bits.enable_order_hint;
   enc_pic->order_hint_bits = pic->seq.order_hint_bits;
   enc_pic->enable_render_size = pic->enable_render_size;
   enc_pic->render_width = pic->render_width;
   enc_pic->render_height = pic->render_height;
   enc_pic->enable_color_description = pic->seq.seq_bits.color_description_present_flag;
   enc_pic->timing_info_present = pic->seq.seq_bits.timing_info_present_flag;
   enc_pic->timing_info_equal_picture_interval = pic->seq.seq_bits.equal_picture_interval;
   enc_pic->disable_screen_content_tools = !pic->allow_screen_content_tools;
   enc_pic->is_obu_frame = pic->enable_frame_obu;
   enc_pic->need_av1_seq = (pic->frame_type == PIPE_AV1_ENC_FRAME_TYPE_KEY);

   radeon_vcn_enc_av1_get_spec_misc_param(enc, pic);
   radeon_vcn_enc_av1_timing_info(enc, pic);
   radeon_vcn_enc_av1_color_description(enc, pic);
   radeon_vcn_enc_av1_get_rc_param(enc, pic);
   radeon_vcn_enc_get_input_format_param(enc, &pic->base);
   radeon_vcn_enc_get_output_format_param(enc, pic->seq.color_config.color_range);
   /* loop filter enabled all the time */
   radeon_vcn_enc_get_intra_refresh_param(enc,
                                         true,
                                         &pic->intra_refresh);
   radeon_vcn_enc_get_roi_param(enc, &pic->roi);
}

static void radeon_vcn_enc_get_param(struct radeon_encoder *enc, struct pipe_picture_desc *picture)
{
   if (u_reduce_video_profile(picture->profile) == PIPE_VIDEO_FORMAT_MPEG4_AVC)
      radeon_vcn_enc_h264_get_param(enc, (struct pipe_h264_enc_picture_desc *)picture);
   else if (u_reduce_video_profile(picture->profile) == PIPE_VIDEO_FORMAT_HEVC)
      radeon_vcn_enc_hevc_get_param(enc, (struct pipe_h265_enc_picture_desc *)picture);
   else if (u_reduce_video_profile(picture->profile) == PIPE_VIDEO_FORMAT_AV1)
      radeon_vcn_enc_av1_get_param(enc, (struct pipe_av1_enc_picture_desc *)picture);
}

static void flush(struct radeon_encoder *enc)
{
   enc->ws->cs_flush(&enc->cs, PIPE_FLUSH_ASYNC, NULL);
}

static void radeon_enc_flush(struct pipe_video_codec *encoder)
{
   struct radeon_encoder *enc = (struct radeon_encoder *)encoder;
   flush(enc);
}

static void radeon_enc_cs_flush(void *ctx, unsigned flags, struct pipe_fence_handle **fence)
{
   // just ignored
}

/* configure reconstructed picture offset */
static void radeon_enc_rec_offset(rvcn_enc_reconstructed_picture_t *recon,
                                  uint32_t *offset,
                                  uint32_t luma_size,
                                  uint32_t chroma_size,
                                  bool is_av1)
{
   if (offset) {
      recon->luma_offset = *offset;
      *offset += luma_size;
      recon->chroma_offset = *offset;
      *offset += chroma_size;
      if (is_av1) {
         recon->av1.av1_cdf_frame_context_offset = *offset;
         *offset += RENCODE_AV1_FRAME_CONTEXT_CDF_TABLE_SIZE;
         recon->av1.av1_cdef_algorithm_context_offset = *offset;
         *offset += RENCODE_AV1_CDEF_ALGORITHM_FRAME_CONTEXT_SIZE;
      }
   } else {
      recon->luma_offset = 0;
      recon->chroma_offset = 0;
      recon->av1.av1_cdf_frame_context_offset = 0;
      recon->av1.av1_cdef_algorithm_context_offset = 0;
   }
}

static int setup_cdf(struct radeon_encoder *enc)
{
   unsigned char *p_cdf = NULL;

   if (!enc->cdf ||
         !si_vid_create_buffer(enc->screen,
                               enc->cdf,
                               VCN_ENC_AV1_DEFAULT_CDF_SIZE,
                               PIPE_USAGE_DYNAMIC)) {
      RVID_ERR("Can't create CDF buffer.\n");
      goto error;
   }

   p_cdf = enc->ws->buffer_map(enc->ws,
                               enc->cdf->res->buf,
                              &enc->cs,
                               PIPE_MAP_READ_WRITE | RADEON_MAP_TEMPORARY);
   if (!p_cdf)
      goto error;

   memcpy(p_cdf, rvcn_av1_cdf_default_table, VCN_ENC_AV1_DEFAULT_CDF_SIZE);
   enc->ws->buffer_unmap(enc->ws, enc->cdf->res->buf);

   return 0;

error:
   return -1;
}

static int setup_dpb(struct radeon_encoder *enc)
{
   bool is_h264 = u_reduce_video_profile(enc->base.profile)
                             == PIPE_VIDEO_FORMAT_MPEG4_AVC;
   bool is_av1 = u_reduce_video_profile(enc->base.profile)
                             == PIPE_VIDEO_FORMAT_AV1;
   uint32_t rec_alignment = is_h264 ? 16 : 64;
   uint32_t aligned_width = align(enc->base.width, rec_alignment);
   uint32_t aligned_height = align(enc->base.height, rec_alignment);
   uint32_t pitch = align(aligned_width, enc->alignment);
   uint32_t num_reconstructed_pictures = enc->base.max_references + 1;
   uint32_t luma_size, chroma_size, offset;
   struct radeon_enc_pic *enc_pic = &enc->enc_pic;
   int i;
   uint32_t aligned_dpb_height = MAX2(256, aligned_height);

   luma_size = align(pitch * aligned_dpb_height , enc->alignment);
   chroma_size = align(luma_size / 2 , enc->alignment);
   if (enc_pic->bit_depth_luma_minus8 || enc_pic->bit_depth_chroma_minus8) {
      luma_size *= 2;
      chroma_size *= 2;
   }

   assert(num_reconstructed_pictures <= RENCODE_MAX_NUM_RECONSTRUCTED_PICTURES);

   enc_pic->ctx_buf.rec_luma_pitch   = pitch;
   enc_pic->ctx_buf.rec_chroma_pitch = pitch;
   enc_pic->ctx_buf.pre_encode_picture_luma_pitch   = pitch;
   enc_pic->ctx_buf.pre_encode_picture_chroma_pitch = pitch;

   offset = 0;
   if (enc_pic->quality_modes.pre_encode_mode) {
      uint32_t pre_size  = DIV_ROUND_UP((aligned_width >> 2), rec_alignment) *
                           DIV_ROUND_UP((aligned_height >> 2), rec_alignment);
      uint32_t full_size = DIV_ROUND_UP(aligned_width, rec_alignment) *
                           DIV_ROUND_UP(aligned_height, rec_alignment);
      pre_size  = align(pre_size, 4);
      full_size = align(full_size, 4);

      enc_pic->ctx_buf.two_pass_search_center_map_offset = offset;
      if (is_h264 && !enc_pic->spec_misc.b_picture_enabled)
         offset += align((pre_size * 4 + full_size) * sizeof(uint32_t), enc->alignment);
      else if (!is_h264)
         offset += align((pre_size * 52 + full_size) * sizeof(uint32_t), enc->alignment);
   } else
      enc_pic->ctx_buf.two_pass_search_center_map_offset = 0;

   if (is_av1) {
      enc_pic->ctx_buf.av1.av1_sdb_intermedidate_context_offset = offset;
      offset += RENCODE_AV1_SDB_FRAME_CONTEXT_SIZE;
   }

   for (i = 0; i < num_reconstructed_pictures; i++) {
      radeon_enc_rec_offset(&enc_pic->ctx_buf.reconstructed_pictures[i],
                            &offset, luma_size, chroma_size, is_av1);

      if (enc_pic->quality_modes.pre_encode_mode)
         radeon_enc_rec_offset(&enc_pic->ctx_buf.pre_encode_reconstructed_pictures[i],
                               &offset, luma_size, chroma_size, is_av1);
   }

   for (; i < RENCODE_MAX_NUM_RECONSTRUCTED_PICTURES; i++) {
      radeon_enc_rec_offset(&enc_pic->ctx_buf.reconstructed_pictures[i],
                            NULL, 0, 0, false);
      if (enc_pic->quality_modes.pre_encode_mode)
         radeon_enc_rec_offset(&enc_pic->ctx_buf.pre_encode_reconstructed_pictures[i],
                               NULL, 0, 0, false);
   }

   if (enc_pic->quality_modes.pre_encode_mode) {
      enc_pic->ctx_buf.pre_encode_input_picture.rgb.red_offset = offset;
      offset += luma_size;
      enc_pic->ctx_buf.pre_encode_input_picture.rgb.green_offset = offset;
      offset += luma_size;
      enc_pic->ctx_buf.pre_encode_input_picture.rgb.blue_offset = offset;
      offset += luma_size;
   }

   enc_pic->ctx_buf.num_reconstructed_pictures = num_reconstructed_pictures;
   enc->max_ltr_idx = 0;

   if (enc_pic->spec_misc.b_picture_enabled) {
      enc_pic->ctx_buf.colloc_buffer_offset = offset;
      offset += (align((aligned_width / 16), 64) / 2) * (aligned_height / 16);
   } else
      enc_pic->ctx_buf.colloc_buffer_offset = 0;

   enc->dpb_size = offset;

   return offset;
}

/* each block (MB/CTB/SB) has one QP/QI value */
static uint32_t roi_buffer_size(struct radeon_encoder *enc)
{
   uint32_t width_in_block, height_in_block;

   radeon_vcn_enc_blocks_in_frame(enc, &width_in_block, &height_in_block);

   return width_in_block * height_in_block * sizeof(uint32_t);
}

static void arrange_qp_map(uint32_t *start,
                           struct rvcn_enc_qp_map_region *map,
                           uint32_t width_in_block,
                           uint32_t height_in_block)
{
   uint32_t i, j;
   uint32_t offset;
   uint32_t num_in_x = MIN2(map->x_in_unit + map->width_in_unit, width_in_block)
                      - map->x_in_unit;
   uint32_t num_in_y = MIN2(map->y_in_unit + map->height_in_unit, height_in_block)
                      - map->y_in_unit;;

   for (j = 0; j < num_in_y; j++) {
      for (i = 0; i < num_in_x; i++) {
         offset = map->x_in_unit + i + (map->y_in_unit + j) * width_in_block;
         *(start + offset) = (int32_t)map->qp_delta;
      }
   }
}

/* Arrange roi map values according to the input regions.
 * The arrangment will consider the lower sequence region
 * higher priority and that could overlap the higher sequence
 * map region. */
static int generate_roi_map(struct radeon_encoder *enc)
{
   uint32_t width_in_block, height_in_block;
   uint32_t i;
   uint32_t *p_roi = NULL;

   radeon_vcn_enc_blocks_in_frame(enc, &width_in_block, &height_in_block);

   assert (enc->roi_size >= width_in_block * height_in_block);
   p_roi = (uint32_t *)enc->ws->buffer_map(enc->ws,
                               enc->roi->res->buf,
                              &enc->cs,
                               PIPE_MAP_READ_WRITE | RADEON_MAP_TEMPORARY);
   if (!p_roi)
      goto error;

   memset(p_roi, 0, width_in_block * height_in_block * sizeof(uint32_t));

   for (i = 0; i < ARRAY_SIZE(enc->enc_pic.enc_qp_map.map); i++) {
      struct rvcn_enc_qp_map_region *map = &enc->enc_pic.enc_qp_map.map[i];
      if (map->is_valid)
         arrange_qp_map(p_roi, map, width_in_block, height_in_block);
   }

   enc->ws->buffer_unmap(enc->ws, enc->roi->res->buf);
   return 0;
error:
   return -1;
}

static void radeon_enc_begin_frame(struct pipe_video_codec *encoder,
                                   struct pipe_video_buffer *source,
                                   struct pipe_picture_desc *picture)
{
   struct radeon_encoder *enc = (struct radeon_encoder *)encoder;
   struct vl_video_buffer *vid_buf = (struct vl_video_buffer *)source;
   enc->need_rate_control = false;

   if (u_reduce_video_profile(enc->base.profile) == PIPE_VIDEO_FORMAT_MPEG4_AVC) {
      struct pipe_h264_enc_picture_desc *pic = (struct pipe_h264_enc_picture_desc *)picture;
      enc->need_rate_control =
         (enc->enc_pic.rc_layer_init[0].target_bit_rate != pic->rate_ctrl[0].target_bitrate) ||
         (enc->enc_pic.rc_layer_init[0].frame_rate_num != pic->rate_ctrl[0].frame_rate_num) ||
         (enc->enc_pic.rc_layer_init[0].frame_rate_den != pic->rate_ctrl[0].frame_rate_den);
   } else if (u_reduce_video_profile(picture->profile) == PIPE_VIDEO_FORMAT_HEVC) {
      struct pipe_h265_enc_picture_desc *pic = (struct pipe_h265_enc_picture_desc *)picture;
      enc->need_rate_control =
         (enc->enc_pic.rc_layer_init[0].target_bit_rate != pic->rc.target_bitrate) ||
         (enc->enc_pic.rc_layer_init[0].frame_rate_num != pic->rc.frame_rate_num) ||
         (enc->enc_pic.rc_layer_init[0].frame_rate_den != pic->rc.frame_rate_den);
   } else if (u_reduce_video_profile(picture->profile) == PIPE_VIDEO_FORMAT_AV1) {
      struct pipe_av1_enc_picture_desc *pic = (struct pipe_av1_enc_picture_desc *)picture;
      enc->need_rate_control =
         (enc->enc_pic.rc_layer_init[0].target_bit_rate != pic->rc[0].target_bitrate) ||
         (enc->enc_pic.rc_layer_init[0].frame_rate_num != pic->rc[0].frame_rate_num) ||
         (enc->enc_pic.rc_layer_init[0].frame_rate_den != pic->rc[0].frame_rate_den);

      if (!enc->cdf) {
         enc->cdf = CALLOC_STRUCT(rvid_buffer);
         if (setup_cdf(enc)) {
            RVID_ERR("Can't create cdf buffer.\n");
            goto error;
         }
      }
   }

   radeon_vcn_enc_get_param(enc, picture);
   if (!enc->dpb) {
      enc->dpb = CALLOC_STRUCT(rvid_buffer);
      setup_dpb(enc);
      if (!enc->dpb ||
          !si_vid_create_buffer(enc->screen, enc->dpb, enc->dpb_size, PIPE_USAGE_DEFAULT)) {
         RVID_ERR("Can't create DPB buffer.\n");
         goto error;
      }
   }

   /* qp map buffer could be created here, and release at the end */
   if (enc->enc_pic.enc_qp_map.qp_map_type != RENCODE_QP_MAP_TYPE_NONE) {
      if (!enc->roi) {
         enc->roi = CALLOC_STRUCT(rvid_buffer);
         enc->roi_size = roi_buffer_size(enc);
         if (!enc->roi || !enc->roi_size ||
             !si_vid_create_buffer(enc->screen, enc->roi, enc->roi_size, PIPE_USAGE_DYNAMIC)) {
            RVID_ERR("Can't create ROI buffer.\n");
            goto error;
         }
      }
      if(generate_roi_map(enc)) {
         RVID_ERR("Can't form roi map.\n");
         goto error;
      }
   }

   if (source->buffer_format == PIPE_FORMAT_NV12 ||
       source->buffer_format == PIPE_FORMAT_P010 ||
       source->buffer_format == PIPE_FORMAT_P016) {
      enc->get_buffer(vid_buf->resources[0], &enc->handle, &enc->luma);
      enc->get_buffer(vid_buf->resources[1], NULL, &enc->chroma);
   }
   else {
      enc->get_buffer(vid_buf->resources[0], &enc->handle, &enc->luma);
      enc->chroma = NULL;
   }

   enc->need_feedback = false;

   if (!enc->stream_handle) {
      struct rvid_buffer fb;
      enc->stream_handle = si_vid_alloc_stream_handle();
      enc->si = CALLOC_STRUCT(rvid_buffer);
      if (!enc->si ||
          !enc->stream_handle ||
          !si_vid_create_buffer(enc->screen, enc->si, 128 * 1024, PIPE_USAGE_STAGING)) {
         RVID_ERR("Can't create session buffer.\n");
         goto error;
      }
      si_vid_create_buffer(enc->screen, &fb, 4096, PIPE_USAGE_STAGING);
      enc->fb = &fb;
      enc->begin(enc);
      flush(enc);
      si_vid_destroy_buffer(&fb);
      enc->need_rate_control = false;
   }

   return;

error:
   RADEON_ENC_DESTROY_VIDEO_BUFFER(enc->dpb);
   RADEON_ENC_DESTROY_VIDEO_BUFFER(enc->si);
   RADEON_ENC_DESTROY_VIDEO_BUFFER(enc->cdf);
   RADEON_ENC_DESTROY_VIDEO_BUFFER(enc->roi);
}

static void radeon_enc_encode_bitstream(struct pipe_video_codec *encoder,
                                        struct pipe_video_buffer *source,
                                        struct pipe_resource *destination, void **fb)
{
   struct radeon_encoder *enc = (struct radeon_encoder *)encoder;
   struct vl_video_buffer *vid_buf = (struct vl_video_buffer *)source;

   enc->get_buffer(destination, &enc->bs_handle, NULL);
   enc->bs_size = destination->width0;

   *fb = enc->fb = CALLOC_STRUCT(rvid_buffer);

   if (!si_vid_create_buffer(enc->screen, enc->fb, 4096, PIPE_USAGE_STAGING)) {
      RVID_ERR("Can't create feedback buffer.\n");
      return;
   }

   if (vid_buf->base.statistics_data) {
      enc->get_buffer(vid_buf->base.statistics_data, &enc->stats, NULL);
      if (enc->stats->size < sizeof(rvcn_encode_stats_type_0_t)) {
         RVID_ERR("Encoder statistics output buffer is too small.\n");
         enc->stats = NULL;
      }
      vid_buf->base.statistics_data = NULL;
   }
   else
      enc->stats = NULL;

   enc->need_feedback = true;
   enc->encode(enc);
}

static void radeon_enc_end_frame(struct pipe_video_codec *encoder, struct pipe_video_buffer *source,
                                 struct pipe_picture_desc *picture)
{
   struct radeon_encoder *enc = (struct radeon_encoder *)encoder;
   flush(enc);
}

static void radeon_enc_destroy(struct pipe_video_codec *encoder)
{
   struct radeon_encoder *enc = (struct radeon_encoder *)encoder;

   if (enc->stream_handle) {
      struct rvid_buffer fb;
      enc->need_feedback = false;
      si_vid_create_buffer(enc->screen, &fb, 512, PIPE_USAGE_STAGING);
      enc->fb = &fb;
      enc->destroy(enc);
      flush(enc);
      RADEON_ENC_DESTROY_VIDEO_BUFFER(enc->si);
      si_vid_destroy_buffer(&fb);
   }

   RADEON_ENC_DESTROY_VIDEO_BUFFER(enc->dpb);
   RADEON_ENC_DESTROY_VIDEO_BUFFER(enc->cdf);
   RADEON_ENC_DESTROY_VIDEO_BUFFER(enc->roi);
   enc->ws->cs_destroy(&enc->cs);
   if (enc->ectx)
      enc->ectx->destroy(enc->ectx);

   FREE(enc);
}

static void radeon_enc_get_feedback(struct pipe_video_codec *encoder, void *feedback,
                                    unsigned *size, struct pipe_enc_feedback_metadata* metadata)
{
   struct radeon_encoder *enc = (struct radeon_encoder *)encoder;
   struct rvid_buffer *fb = feedback;

   if (size) {
      uint32_t *ptr = enc->ws->buffer_map(enc->ws, fb->res->buf, &enc->cs,
                                          PIPE_MAP_READ_WRITE | RADEON_MAP_TEMPORARY);
      if (ptr[1])
         *size = ptr[6] - ptr[8];
      else
         *size = 0;
      enc->ws->buffer_unmap(enc->ws, fb->res->buf);
   }

   RADEON_ENC_DESTROY_VIDEO_BUFFER(fb);
}

static void radeon_enc_destroy_fence(struct pipe_video_codec *encoder,
                                     struct pipe_fence_handle *fence)
{
   struct radeon_encoder *enc = (struct radeon_encoder *)encoder;

   enc->ws->fence_reference(enc->ws, &fence, NULL);
}

struct pipe_video_codec *radeon_create_encoder(struct pipe_context *context,
                                               const struct pipe_video_codec *templ,
                                               struct radeon_winsys *ws,
                                               radeon_enc_get_buffer get_buffer)
{
   struct si_screen *sscreen = (struct si_screen *)context->screen;
   struct si_context *sctx = (struct si_context *)context;
   struct radeon_encoder *enc;

   enc = CALLOC_STRUCT(radeon_encoder);

   if (!enc)
      return NULL;

   if (sctx->vcn_has_ctx) {
      enc->ectx = pipe_create_multimedia_context(context->screen);
      if (!enc->ectx)
         sctx->vcn_has_ctx = false;
   }

   enc->alignment = 256;
   enc->base = *templ;
   enc->base.context = (sctx->vcn_has_ctx)? enc->ectx : context;
   enc->base.destroy = radeon_enc_destroy;
   enc->base.begin_frame = radeon_enc_begin_frame;
   enc->base.encode_bitstream = radeon_enc_encode_bitstream;
   enc->base.end_frame = radeon_enc_end_frame;
   enc->base.flush = radeon_enc_flush;
   enc->base.get_feedback = radeon_enc_get_feedback;
   enc->base.destroy_fence = radeon_enc_destroy_fence;
   enc->get_buffer = get_buffer;
   enc->bits_in_shifter = 0;
   enc->screen = context->screen;
   enc->ws = ws;

   if (!ws->cs_create(&enc->cs,
       (sctx->vcn_has_ctx) ? ((struct si_context *)enc->ectx)->ctx : sctx->ctx,
       AMD_IP_VCN_ENC, radeon_enc_cs_flush, enc)) {
      RVID_ERR("Can't get command submission context.\n");
      goto error;
   }

   if (sscreen->info.vcn_ip_version >= VCN_4_0_0)
      radeon_enc_4_0_init(enc);
   else if (sscreen->info.vcn_ip_version >= VCN_3_0_0)
      radeon_enc_3_0_init(enc);
   else if (sscreen->info.vcn_ip_version >= VCN_2_0_0)
      radeon_enc_2_0_init(enc);
   else
      radeon_enc_1_2_init(enc);

   return &enc->base;

error:
   enc->ws->cs_destroy(&enc->cs);
   FREE(enc);
   return NULL;
}

void radeon_enc_add_buffer(struct radeon_encoder *enc, struct pb_buffer_lean *buf,
                           unsigned usage, enum radeon_bo_domain domain, signed offset)
{
   enc->ws->cs_add_buffer(&enc->cs, buf, usage | RADEON_USAGE_SYNCHRONIZED, domain);
   uint64_t addr;
   addr = enc->ws->buffer_get_virtual_address(buf);
   addr = addr + offset;
   RADEON_ENC_CS(addr >> 32);
   RADEON_ENC_CS(addr);
}

void radeon_enc_set_emulation_prevention(struct radeon_encoder *enc, bool set)
{
   if (set != enc->emulation_prevention) {
      enc->emulation_prevention = set;
      enc->num_zeros = 0;
   }
}

void radeon_enc_output_one_byte(struct radeon_encoder *enc, unsigned char byte)
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

void radeon_enc_emulation_prevention(struct radeon_encoder *enc, unsigned char byte)
{
   if (enc->emulation_prevention) {
      if ((enc->num_zeros >= 2) && ((byte == 0x00) || (byte == 0x01) ||
         (byte == 0x02) || (byte == 0x03))) {
         radeon_enc_output_one_byte(enc, 0x03);
         enc->bits_output += 8;
         enc->num_zeros = 0;
      }
      enc->num_zeros = (byte == 0 ? (enc->num_zeros + 1) : 0);
   }
}

void radeon_enc_code_fixed_bits(struct radeon_encoder *enc, unsigned int value,
                                unsigned int num_bits)
{
   unsigned int bits_to_pack = 0;
   enc->bits_size += num_bits;

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
         radeon_enc_emulation_prevention(enc, output_byte);
         radeon_enc_output_one_byte(enc, output_byte);
         enc->bits_in_shifter -= 8;
         enc->bits_output += 8;
      }
   }
}

void radeon_enc_code_uvlc(struct radeon_encoder *enc, unsigned int value)
{
   uint32_t num_bits = 0;
   uint64_t value_plus1 = (uint64_t)value + 1;
   uint32_t num_leading_zeros = 0;

   while ((uint64_t)1 << num_bits <= value_plus1)
      num_bits++;

   num_leading_zeros = num_bits - 1;
   radeon_enc_code_fixed_bits(enc, 0, num_leading_zeros);
   radeon_enc_code_fixed_bits(enc, 1, 1);
   radeon_enc_code_fixed_bits(enc, (uint32_t)value_plus1, num_leading_zeros);
}

void radeon_enc_code_leb128(uint8_t *buf, uint32_t value,
                            uint32_t num_bytes)
{
   uint8_t leb128_byte = 0;
   uint32_t i = 0;

   do {
      leb128_byte = (value & 0x7f);
      value >>= 7;
      if (num_bytes > 1)
         leb128_byte |= 0x80;

      *(buf + i) = leb128_byte;
      num_bytes--;
      i++;
   } while((leb128_byte & 0x80));
}

void radeon_enc_reset(struct radeon_encoder *enc)
{
   enc->emulation_prevention = false;
   enc->shifter = 0;
   enc->bits_in_shifter = 0;
   enc->bits_output = 0;
   enc->num_zeros = 0;
   enc->byte_index = 0;
   enc->bits_size = 0;
}

void radeon_enc_byte_align(struct radeon_encoder *enc)
{
   unsigned int num_padding_zeros = (32 - enc->bits_in_shifter) % 8;

   if (num_padding_zeros > 0)
      radeon_enc_code_fixed_bits(enc, 0, num_padding_zeros);
}

void radeon_enc_flush_headers(struct radeon_encoder *enc)
{
   if (enc->bits_in_shifter != 0) {
      unsigned char output_byte = (unsigned char)(enc->shifter >> 24);
      radeon_enc_emulation_prevention(enc, output_byte);
      radeon_enc_output_one_byte(enc, output_byte);
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

void radeon_enc_code_ue(struct radeon_encoder *enc, unsigned int value)
{
   int x = -1;
   unsigned int ue_code = value + 1;
   value += 1;

   while (value) {
      value = (value >> 1);
      x += 1;
   }

   unsigned int ue_length = (x << 1) + 1;
   radeon_enc_code_fixed_bits(enc, ue_code, ue_length);
}

void radeon_enc_code_se(struct radeon_encoder *enc, int value)
{
   unsigned int v = 0;

   if (value != 0)
      v = (value < 0 ? ((unsigned int)(0 - value) << 1) : (((unsigned int)(value) << 1) - 1));

   radeon_enc_code_ue(enc, v);
}

/* dummy function for re-using the same pipeline */
void radeon_enc_dummy(struct radeon_encoder *enc) {}

/* this function has to be in pair with AV1 header copy instruction type at the end */
static void radeon_enc_av1_bs_copy_end(struct radeon_encoder *enc, uint32_t bits)
{
   assert(bits > 0);
   /* it must be dword aligned at the end */
   *enc->enc_pic.copy_start = DIV_ROUND_UP(bits, 32) * 4 + 12;
   *(enc->enc_pic.copy_start + 2) = bits;
}

/* av1 bitstream instruction type */
void radeon_enc_av1_bs_instruction_type(struct radeon_encoder *enc,
                                        uint32_t inst,
                                        uint32_t obu_type)
{
   radeon_enc_flush_headers(enc);

   if (enc->bits_output)
      radeon_enc_av1_bs_copy_end(enc, enc->bits_output);

   enc->enc_pic.copy_start = &enc->cs.current.buf[enc->cs.current.cdw++];
   RADEON_ENC_CS(inst);

   if (inst != RENCODE_HEADER_INSTRUCTION_COPY) {
      *enc->enc_pic.copy_start = 8;
      if (inst == RENCODE_AV1_BITSTREAM_INSTRUCTION_OBU_START) {
         *enc->enc_pic.copy_start += 4;
         RADEON_ENC_CS(obu_type);
      }
   } else
      RADEON_ENC_CS(0); /* allocate a dword for number of bits */

   radeon_enc_reset(enc);
}

uint32_t radeon_enc_value_bits(uint32_t value)
{
   uint32_t i = 1;

   while (value > 1) {
      i++;
      value >>= 1;
   }

   return i;
}
