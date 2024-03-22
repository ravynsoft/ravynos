/**************************************************************************
 *
 * Copyright 2022 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 *
 **************************************************************************/

#include <stdio.h>

#include "pipe/p_video_codec.h"

#include "util/u_video.h"

#include "si_pipe.h"
#include "radeon_vcn_enc.h"

#define RENCODE_FW_INTERFACE_MAJOR_VERSION   1
#define RENCODE_FW_INTERFACE_MINOR_VERSION   11

#define RENCODE_IB_PARAM_CDF_DEFAULT_TABLE_BUFFER  0x00000019
#define RENCODE_IB_PARAM_ENCODE_STATISTICS         0x0000001a

#define RENCODE_AV1_IB_PARAM_SPEC_MISC             0x00300001
#define RENCODE_AV1_IB_PARAM_BITSTREAM_INSTRUCTION 0x00300002

#define RENCODE_AV1_BITSTREAM_INSTRUCTION_END   RENCODE_HEADER_INSTRUCTION_END
#define RENCODE_AV1_BITSTREAM_INSTRUCTION_COPY  RENCODE_HEADER_INSTRUCTION_COPY
#define RENCODE_AV1_BITSTREAM_INSTRUCTION_ALLOW_HIGH_PRECISION_MV                   0x00000005
#define RENCODE_AV1_BITSTREAM_INSTRUCTION_DELTA_LF_PARAMS                           0x00000006
#define RENCODE_AV1_BITSTREAM_INSTRUCTION_READ_INTERPOLATION_FILTER                 0x00000007
#define RENCODE_AV1_BITSTREAM_INSTRUCTION_LOOP_FILTER_PARAMS                        0x00000008
#define RENCODE_AV1_BITSTREAM_INSTRUCTION_TILE_INFO                                 0x00000009
#define RENCODE_AV1_BITSTREAM_INSTRUCTION_QUANTIZATION_PARAMS                       0x0000000a
#define RENCODE_AV1_BITSTREAM_INSTRUCTION_DELTA_Q_PARAMS                            0x0000000b
#define RENCODE_AV1_BITSTREAM_INSTRUCTION_CDEF_PARAMS                               0x0000000c
#define RENCODE_AV1_BITSTREAM_INSTRUCTION_READ_TX_MODE                              0x0000000d
#define RENCODE_AV1_BITSTREAM_INSTRUCTION_TILE_GROUP_OBU                            0x0000000e

static void radeon_enc_sq_begin(struct radeon_encoder *enc)
{
   rvcn_sq_header(&enc->cs, &enc->sq, true);
   enc->mq_begin(enc);
   rvcn_sq_tail(&enc->cs, &enc->sq);
}

static void radeon_enc_sq_encode(struct radeon_encoder *enc)
{
   rvcn_sq_header(&enc->cs, &enc->sq, true);
   enc->mq_encode(enc);
   rvcn_sq_tail(&enc->cs, &enc->sq);
}

static void radeon_enc_sq_destroy(struct radeon_encoder *enc)
{
   rvcn_sq_header(&enc->cs, &enc->sq, true);
   enc->mq_destroy(enc);
   rvcn_sq_tail(&enc->cs, &enc->sq);
}

static void radeon_enc_op_preset(struct radeon_encoder *enc)
{
   uint32_t preset_mode;

   if (enc->enc_pic.quality_modes.preset_mode == RENCODE_PRESET_MODE_SPEED &&
         (enc->enc_pic.sample_adaptive_offset_enabled_flag &&
         (u_reduce_video_profile(enc->base.profile) == PIPE_VIDEO_FORMAT_HEVC)))
      preset_mode = RENCODE_IB_OP_SET_BALANCE_ENCODING_MODE;
   else if (enc->enc_pic.quality_modes.preset_mode == RENCODE_PRESET_MODE_QUALITY)
      preset_mode = RENCODE_IB_OP_SET_QUALITY_ENCODING_MODE;
   else if (enc->enc_pic.quality_modes.preset_mode == RENCODE_PRESET_MODE_HIGH_QUALITY)
      preset_mode = RENCODE_IB_OP_SET_HIGH_QUALITY_ENCODING_MODE;
   else if (enc->enc_pic.quality_modes.preset_mode == RENCODE_PRESET_MODE_BALANCE)
      preset_mode = RENCODE_IB_OP_SET_BALANCE_ENCODING_MODE;
   else
      preset_mode = RENCODE_IB_OP_SET_SPEED_ENCODING_MODE;

   RADEON_ENC_BEGIN(preset_mode);
   RADEON_ENC_END();
}

static void radeon_enc_session_init(struct radeon_encoder *enc)
{
   bool av1_encoding = false;
   uint32_t av1_height = enc->enc_pic.pic_height_in_luma_samples;

   switch (u_reduce_video_profile(enc->base.profile)) {
      case PIPE_VIDEO_FORMAT_MPEG4_AVC:
         enc->enc_pic.session_init.encode_standard = RENCODE_ENCODE_STANDARD_H264;
         enc->enc_pic.session_init.aligned_picture_width = align(enc->base.width, 16);
         enc->enc_pic.session_init.aligned_picture_height = align(enc->base.height, 16);
         break;
      case PIPE_VIDEO_FORMAT_HEVC:
         enc->enc_pic.session_init.encode_standard = RENCODE_ENCODE_STANDARD_HEVC;
         enc->enc_pic.session_init.aligned_picture_width = align(enc->base.width, 64);
         enc->enc_pic.session_init.aligned_picture_height = align(enc->base.height, 16);
         break;
      case PIPE_VIDEO_FORMAT_AV1:
         enc->enc_pic.session_init.encode_standard = RENCODE_ENCODE_STANDARD_AV1;
         enc->enc_pic.session_init.aligned_picture_width =
                              align(enc->enc_pic.pic_width_in_luma_samples, 64);
         enc->enc_pic.session_init.aligned_picture_height =
                                 align(enc->enc_pic.pic_height_in_luma_samples, 16);
         if (!(av1_height % 8) && (av1_height % 16) && !(enc->enc_pic.enable_render_size))
            enc->enc_pic.session_init.aligned_picture_height = av1_height + 2;

         av1_encoding = true;
         break;
      default:
         assert(0);
         break;
   }

   enc->enc_pic.session_init.padding_width =
      enc->enc_pic.session_init.aligned_picture_width - enc->base.width;
   enc->enc_pic.session_init.padding_height =
      enc->enc_pic.session_init.aligned_picture_height - enc->base.height;

   if (av1_encoding) {
      enc->enc_pic.session_init.padding_width =
         enc->enc_pic.session_init.aligned_picture_width -
         enc->enc_pic.pic_width_in_luma_samples;
      enc->enc_pic.session_init.padding_height =
         enc->enc_pic.session_init.aligned_picture_height - av1_height;

      if (enc->enc_pic.enable_render_size)
         enc->enc_pic.enable_render_size =
                        (enc->enc_pic.session_init.aligned_picture_width !=
                         enc->enc_pic.render_width) ||
                        (enc->enc_pic.session_init.aligned_picture_height !=
                         enc->enc_pic.render_height);
   }

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
   RADEON_ENC_CS(0);
   RADEON_ENC_END();
}

/* for new temporal_id, sequence_num has to be incremented ahead. */
static uint32_t radeon_enc_av1_calculate_temporal_id(uint32_t sequence_num,
                                                     uint32_t max_layer)
{
   for (uint32_t i = 0; i <= max_layer; i++)
      if (!(sequence_num % (1 << (max_layer - i))))
         return i;

   /* never come here */
   assert(0);
   return 0;
}

static uint32_t radeon_enc_av1_alloc_recon_slot(struct radeon_encoder *enc)
{
   uint32_t i;
   for (i = 0; i < ARRAY_SIZE(enc->enc_pic.recon_slots); i++) {
      if(!enc->enc_pic.recon_slots[i].in_use) {
         enc->enc_pic.recon_slots[i].in_use = true;
         break;
      }
   }
   return i;
}

static void redeon_enc_av1_release_recon_slot(struct radeon_encoder *enc,
                                              uint32_t index,
                                              bool is_orphaned)
{
   assert(index < (ARRAY_SIZE(enc->enc_pic.recon_slots) - 1));
   assert(enc->enc_pic.recon_slots[index].in_use);

   if (is_orphaned)
      enc->enc_pic.recon_slots[index].is_orphaned = true;
   else
      enc->enc_pic.recon_slots[index].in_use = false;
}

static uint32_t radeon_enc_av1_alloc_curr_frame(struct radeon_encoder *enc,
                                 uint32_t frame_id,
                                 uint32_t temporal_id,
                                 enum pipe_av1_enc_frame_type frame_type)
{
   uint32_t i = 0;

   for (i = 0; i < ARRAY_SIZE(enc->enc_pic.frames); i++) {
      rvcn_enc_av1_ref_frame_t *frame = &enc->enc_pic.frames[i];
      if (!frame->in_use) {
         frame->in_use = true;
         frame->frame_id = frame_id;
         frame->temporal_id = temporal_id;
         frame->slot_id = radeon_enc_av1_alloc_recon_slot(enc);
         frame->frame_type = frame_type;
         break;
      }
   }

   return i;
}

static void radeon_enc_av1_release_ref_frame(struct radeon_encoder *enc,
                                             uint32_t index,
                                             bool is_recon_orphan)
{
   assert(index < (ARRAY_SIZE(enc->enc_pic.frames) - 1));

   redeon_enc_av1_release_recon_slot(enc,
                                     enc->enc_pic.frames[index].slot_id,
                                     is_recon_orphan);
   enc->enc_pic.frames[index].in_use = false;
}

/* save 1 recon slot in max temporal layer = 4 case */
static void radeon_enc_av1_temporal_4_extra_release(struct radeon_encoder *enc,
                                                    uint32_t temporal_id)
{
   uint32_t i = 0;

   if (temporal_id == 0)
      enc->enc_pic.count_last_layer = 0;
   else if (temporal_id == 3)
      enc->enc_pic.count_last_layer++;

   /* since temporal ID = 1 picture will not be used in this
    * temporal period, that can be released */
   if (enc->enc_pic.count_last_layer == 4) {
      for (i = 0; i < ARRAY_SIZE(enc->enc_pic.frames); i++) {
         rvcn_enc_av1_ref_frame_t *frame = &enc->enc_pic.frames[i];
         if (frame->in_use && (frame->temporal_id == 1)) {
            radeon_enc_av1_release_ref_frame(enc, i, false);
            break;
         }
      }
   }
}

static void radeon_enc_av1_pre_scan_frames(struct radeon_encoder *enc,
                                           uint32_t temporal_id)
{
   uint32_t i = 0;

   for (i = 0; i < ARRAY_SIZE(enc->enc_pic.recon_slots); i++) {
      rvcn_enc_av1_recon_slot_t *slot = &enc->enc_pic.recon_slots[i];
      if (slot->in_use && slot->is_orphaned) {
         slot->in_use = false;
         slot->is_orphaned = false;
      }
   }

   for (i = 0; i < ARRAY_SIZE(enc->enc_pic.frames); i++) {
      rvcn_enc_av1_ref_frame_t *frame = &enc->enc_pic.frames[i];
      if (frame->in_use) {
         if (temporal_id < frame->temporal_id)
            radeon_enc_av1_release_ref_frame(enc, i, false);
         else if (temporal_id == frame->temporal_id)
            radeon_enc_av1_release_ref_frame(enc, i, true);
      }
   }
}

static uint32_t radeon_enc_av1_obtain_ref0_frame(struct radeon_encoder *enc,
                                          uint32_t temporal_id)
{
   uint32_t i = 0;
   for (i = ARRAY_SIZE(enc->enc_pic.frames); i > 0; i--) {
      rvcn_enc_av1_ref_frame_t *frame = &enc->enc_pic.frames[i - 1];
      if (frame->in_use && frame->temporal_id <= temporal_id)
         break;
   }
   /* not find, ref = 0, or ref = i - 1 */
   return i == 0 ? i : i - 1;
}

static void radeon_enc_reset_av1_dpb_frames(struct radeon_encoder *enc)
{
   for (int i = 0; i < ARRAY_SIZE(enc->enc_pic.frames); i++) {
      enc->enc_pic.frames[i].in_use = false;
      enc->enc_pic.frames[i].frame_id = 0;
      enc->enc_pic.frames[i].temporal_id = 0;
      enc->enc_pic.frames[i].slot_id = 0;
      enc->enc_pic.frames[i].frame_type = 0;
   }

   for (int i = 0; i < ARRAY_SIZE(enc->enc_pic.recon_slots); i++) {
      enc->enc_pic.recon_slots[i].in_use = false;
      enc->enc_pic.recon_slots[i].is_orphaned = false;
   }
}

static void radeon_enc_av1_dpb_management(struct radeon_encoder *enc)
{
   struct radeon_enc_pic *pic = &enc->enc_pic;
   uint32_t current_slot;
   uint32_t ref_slot;

   if (pic->frame_type == PIPE_AV1_ENC_FRAME_TYPE_KEY) {
      pic->frame_id = 0;
      pic->temporal_id = 0;
      pic->reference_delta_frame_id = 0;
      pic->reference_frame_index = 0;
      pic->last_frame_type = PIPE_AV1_ENC_FRAME_TYPE_KEY;
      current_slot = 0;
      ref_slot = 0;
      radeon_enc_reset_av1_dpb_frames(enc);
   } else {
      pic->temporal_id = radeon_enc_av1_calculate_temporal_id(pic->frame_id,
                                                              pic->num_temporal_layers - 1);
      pic->reference_frame_index =
         radeon_enc_av1_obtain_ref0_frame(enc, pic->temporal_id);
      ref_slot = pic->frames[pic->reference_frame_index].slot_id;
      pic->last_frame_type = pic->frames[pic->reference_frame_index].frame_type;
      radeon_enc_av1_pre_scan_frames(enc, pic->temporal_id);
   }

   if (pic->num_temporal_layers == 4)
      radeon_enc_av1_temporal_4_extra_release(enc, pic->temporal_id);

   pic->frame_to_show_map_index = pic->reference_frame_index;

   for (int i = 0; i < ARRAY_SIZE(pic->frames); i++)
      pic->reference_order_hint[i] = pic->frames[i].frame_id;

   pic->reference_delta_frame_id = pic->frame_id -
                                   pic->frames[pic->reference_frame_index].frame_id;
   current_slot = radeon_enc_av1_alloc_curr_frame(enc, pic->frame_id,
                                                       pic->temporal_id,
                                                       pic->frame_type);
   if (pic->frame_type == PIPE_AV1_ENC_FRAME_TYPE_KEY ||
       pic->frame_type == PIPE_AV1_ENC_FRAME_TYPE_SWITCH ||
       ((pic->frame_type == PIPE_AV1_ENC_FRAME_TYPE_SHOW_EXISTING) &&
         pic->last_frame_type == PIPE_AV1_ENC_FRAME_TYPE_KEY))
      pic->refresh_frame_flags = 255;
   else
      pic->refresh_frame_flags = 1 << current_slot;

   pic->enc_params.reference_picture_index = ref_slot;
   pic->enc_params.reconstructed_picture_index = pic->frames[current_slot].slot_id;
   pic->display_frame_id = pic->frame_id;
   pic->order_hint = pic->frame_id;
}

static void radeon_enc_spec_misc_av1(struct radeon_encoder *enc)
{
   uint32_t num_of_tiles = enc->enc_pic.av1_spec_misc.num_tiles_per_picture;
   uint32_t threshold_low, threshold_high;
   uint32_t num_rows;
   uint32_t num_columns;

   num_rows = PIPE_ALIGN_IN_BLOCK_SIZE(enc->enc_pic.session_init.aligned_picture_height,
                                       PIPE_AV1_ENC_SB_SIZE);
   num_columns = PIPE_ALIGN_IN_BLOCK_SIZE(enc->enc_pic.session_init.aligned_picture_width,
                                       PIPE_AV1_ENC_SB_SIZE);

   if (num_rows > 64) {
      /* max tile size 4096 x 2304 */
      threshold_low = ((num_rows + 63) / 64) * ((num_columns + 35) / 36);
      num_of_tiles = (num_of_tiles & 1) ? num_of_tiles - 1 : num_of_tiles;
   } else
      threshold_low = 1;

   threshold_high = num_rows > 16 ? 16 : num_rows;
   threshold_high = num_columns > 64 ? threshold_high * 2 : threshold_high;

   num_of_tiles = CLAMP(num_of_tiles, threshold_low, threshold_high);

   /* in case of multiple tiles, it should be an obu frame */
   if (num_of_tiles > 1)
      enc->enc_pic.stream_obu_frame = 1;
   else
      enc->enc_pic.stream_obu_frame = enc->enc_pic.is_obu_frame;

   enc->enc_pic.av1_spec_misc.num_tiles_per_picture = num_of_tiles;

   RADEON_ENC_BEGIN(enc->cmd.spec_misc_av1);
   RADEON_ENC_CS(enc->enc_pic.av1_spec_misc.palette_mode_enable);
   RADEON_ENC_CS(enc->enc_pic.av1_spec_misc.mv_precision);
   RADEON_ENC_CS(enc->enc_pic.av1_spec_misc.cdef_mode);
   RADEON_ENC_CS(enc->enc_pic.av1_spec_misc.disable_cdf_update);
   RADEON_ENC_CS(enc->enc_pic.av1_spec_misc.disable_frame_end_update_cdf);
   RADEON_ENC_CS(enc->enc_pic.av1_spec_misc.num_tiles_per_picture);
   RADEON_ENC_CS(0);
   RADEON_ENC_CS(0);
   RADEON_ENC_END();
}

static void radeon_enc_cdf_default_table(struct radeon_encoder *enc)
{
   bool use_cdf_default = enc->enc_pic.frame_type == PIPE_AV1_ENC_FRAME_TYPE_KEY ||
                          enc->enc_pic.frame_type == PIPE_AV1_ENC_FRAME_TYPE_INTRA_ONLY ||
                          enc->enc_pic.frame_type == PIPE_AV1_ENC_FRAME_TYPE_SWITCH;

   enc->enc_pic.av1_cdf_default_table.use_cdf_default = use_cdf_default ? 1 : 0;

   RADEON_ENC_BEGIN(enc->cmd.cdf_default_table_av1);
   RADEON_ENC_CS(enc->enc_pic.av1_cdf_default_table.use_cdf_default);
   RADEON_ENC_READWRITE(enc->cdf->res->buf, enc->cdf->res->domains, 0);
   RADEON_ENC_ADDR_SWAP();
   RADEON_ENC_END();
}

uint8_t *radeon_enc_av1_header_size_offset(struct radeon_encoder *enc)
{
   uint32_t *bits_start = enc->enc_pic.copy_start + 3;
   assert(enc->bits_output % 8 == 0); /* should be always byte aligned */
   return (uint8_t *)(bits_start) + (enc->bits_output >> 3);
}

static void radeon_enc_av1_temporal_delimiter(struct radeon_encoder *enc)
{
   bool use_extension_flag;

   /* obu header () */
   /* obu_forbidden_bit */
   radeon_enc_code_fixed_bits(enc, 0, 1);
   /* obu_type */
   radeon_enc_code_fixed_bits(enc, RENCODE_OBU_TYPE_TEMPORAL_DELIMITER, 4);
   /* obu_extension_flag */
   use_extension_flag = (enc->enc_pic.num_temporal_layers) > 1 &&
                        (enc->enc_pic.temporal_id) > 0 ? 1 : 0;
   radeon_enc_code_fixed_bits(enc, use_extension_flag ? 1 : 0, 1);
   /* obu_has_size_field */
   radeon_enc_code_fixed_bits(enc, 1, 1);
   /* obu_reserved_1bit */
   radeon_enc_code_fixed_bits(enc, 0, 1);

   if (use_extension_flag) {
      radeon_enc_code_fixed_bits(enc, enc->enc_pic.temporal_id, 3);
      radeon_enc_code_fixed_bits(enc, 0, 2);  /* spatial_id should always be zero */
      radeon_enc_code_fixed_bits(enc, 0, 3);  /* reserved 3 bits */
   }

   radeon_enc_code_fixed_bits(enc, 0, 8); /* obu has size */
}

static void radeon_enc_av1_sequence_header(struct radeon_encoder *enc)
{
   uint8_t *size_offset;
   uint8_t obu_size_bin[2];
   uint32_t obu_size;
   uint32_t width_bits;
   uint32_t height_bits;
   uint32_t max_temporal_layers = enc->enc_pic.num_temporal_layers;

   /*  obu_header() */
   /* obu_forbidden_bit */
   radeon_enc_code_fixed_bits(enc, 0, 1);
   /*  obu_type */
   radeon_enc_code_fixed_bits(enc, RENCODE_OBU_TYPE_SEQUENCE_HEADER, 4);
   /*  obu_extension_flag */
   radeon_enc_code_fixed_bits(enc, 0, 1);
   /*  obu_has_size_field */
   radeon_enc_code_fixed_bits(enc, 1, 1);
   /*  obu_reserved_1bit */
   radeon_enc_code_fixed_bits(enc, 0, 1);

   /* obu_size, use two bytes for header, the size will be written in afterwards */
   size_offset = radeon_enc_av1_header_size_offset(enc);
   radeon_enc_code_fixed_bits(enc, 0, 2 * 8);

   /* sequence_header_obu() */
   /*  seq_profile, only seq_profile = 0 is supported  */
   radeon_enc_code_fixed_bits(enc, 0, 3);
   /*  still_picture */
   radeon_enc_code_fixed_bits(enc, 0, 1);
   /*  reduced_still_picture_header */
   radeon_enc_code_fixed_bits(enc, 0, 1);
   /*  timing_info_present_flag  */
   radeon_enc_code_fixed_bits(enc, enc->enc_pic.timing_info_present ? 1 : 0, 1);

   if (enc->enc_pic.timing_info_present) {
      /*  num_units_in_display_tick  */
      radeon_enc_code_fixed_bits(enc, enc->enc_pic.av1_timing_info.num_units_in_display_tick, 32);
      /*  time_scale  */
      radeon_enc_code_fixed_bits(enc, enc->enc_pic.av1_timing_info.time_scale, 32);
      /*  equal_picture_interval  */
      radeon_enc_code_fixed_bits(enc, enc->enc_pic.timing_info_equal_picture_interval, 1);
      /*  num_ticks_per_picture_minus_1  */
       if (enc->enc_pic.timing_info_equal_picture_interval)
           radeon_enc_code_uvlc(enc, enc->enc_pic.av1_timing_info.num_tick_per_picture_minus1);
       /*  decoder_model_info_present_flag  */
       radeon_enc_code_fixed_bits(enc, 0, 1);
   }

   /*  initial_display_delay_present_flag  */
   radeon_enc_code_fixed_bits(enc, 0, 1);
   /*  operating_points_cnt_minus_1  */
   radeon_enc_code_fixed_bits(enc, max_temporal_layers - 1, 5);

   for (uint32_t i = 0; i < max_temporal_layers; i++) {
      uint32_t operating_point_idc = 0;
      if (max_temporal_layers > 1) {
         operating_point_idc = (1 << (max_temporal_layers - i)) - 1;
         operating_point_idc |= 0x100;  /* spatial layer not supported */
      }
      radeon_enc_code_fixed_bits(enc, operating_point_idc, 12);
      radeon_enc_code_fixed_bits(enc, enc->enc_pic.general_level_idc, 5);
      if (enc->enc_pic.general_level_idc > 7)
         radeon_enc_code_fixed_bits(enc, 0, 1);  /* tier */
   }

   /*  frame_width_bits_minus_1  */
   width_bits = radeon_enc_value_bits(enc->enc_pic.session_init.aligned_picture_width - 1);
   radeon_enc_code_fixed_bits(enc, width_bits - 1, 4);
   /*  frame_height_bits_minus_1  */
   height_bits = radeon_enc_value_bits(enc->enc_pic.session_init.aligned_picture_height - 1);
   radeon_enc_code_fixed_bits(enc, height_bits - 1, 4);
   /*  max_frame_width_minus_1  */
   radeon_enc_code_fixed_bits(enc, enc->enc_pic.session_init.aligned_picture_width - 1,
                                   width_bits);
   /*  max_frame_height_minus_1  */
   radeon_enc_code_fixed_bits(enc, enc->enc_pic.session_init.aligned_picture_height - 1,
                                   height_bits);

   /*  frame_id_numbers_present_flag  */
   radeon_enc_code_fixed_bits(enc, enc->enc_pic.frame_id_numbers_present, 1);
   if (enc->enc_pic.frame_id_numbers_present) {
      /*  delta_frame_id_length_minus_2  */
      radeon_enc_code_fixed_bits(enc, RENCODE_AV1_DELTA_FRAME_ID_LENGTH - 2, 4);
      /*  additional_frame_id_length_minus_1  */
      radeon_enc_code_fixed_bits(enc, RENCODE_AV1_ADDITIONAL_FRAME_ID_LENGTH - 1, 3);
   }

   /*  use_128x128_superblock  */
   radeon_enc_code_fixed_bits(enc, 0, 1);
   /*  enable_filter_intra  */
   radeon_enc_code_fixed_bits(enc, 0, 1);
   /*  enable_intra_edge_filter  */
   radeon_enc_code_fixed_bits(enc, 0, 1);
   /*  enable_interintra_compound  */
   radeon_enc_code_fixed_bits(enc, 0, 1);
   /*  enable_masked_compound  */
   radeon_enc_code_fixed_bits(enc, 0, 1);
   /*  enable_warped_motion  */
   radeon_enc_code_fixed_bits(enc, 0, 1);
   /*  enable_dual_filter  */
   radeon_enc_code_fixed_bits(enc, 0, 1);
   /*  enable_order_hint  */
   radeon_enc_code_fixed_bits(enc, enc->enc_pic.enable_order_hint ? 1 : 0, 1);

   if (enc->enc_pic.enable_order_hint) {
      /*  enable_jnt_comp  */
      radeon_enc_code_fixed_bits(enc, 0, 1);
      /*  enable_ref_frame_mvs  */
      radeon_enc_code_fixed_bits(enc, 0, 1);
   }

   /*  seq_choose_screen_content_tools  */
   radeon_enc_code_fixed_bits(enc, enc->enc_pic.disable_screen_content_tools ? 0 : 1, 1);
   if (enc->enc_pic.disable_screen_content_tools)
      /*  seq_force_screen_content_tools  */
      radeon_enc_code_fixed_bits(enc, 0, 1);
   else
      /*  seq_choose_integer_mv  */
      radeon_enc_code_fixed_bits(enc, 1, 1);

   if(enc->enc_pic.enable_order_hint)
      /*  order_hint_bits_minus_1  */
      radeon_enc_code_fixed_bits(enc, enc->enc_pic.order_hint_bits - 1, 3);

   /*  enable_superres  */
   radeon_enc_code_fixed_bits(enc, 0, 1);
   /*  enable_cdef  */
   radeon_enc_code_fixed_bits(enc, enc->enc_pic.av1_spec_misc.cdef_mode ? 1 : 0, 1);
   /*  enable_restoration  */
   radeon_enc_code_fixed_bits(enc, 0, 1);
   /*  high_bitdepth  */
   radeon_enc_code_fixed_bits(enc, enc->enc_pic.enc_output_format.output_color_bit_depth, 1);
   /*  mono_chrome  */
   radeon_enc_code_fixed_bits(enc, 0, 1);
   /*  color_description_present_flag  */
   radeon_enc_code_fixed_bits(enc, enc->enc_pic.enable_color_description ? 1 : 0, 1);

   if(enc->enc_pic.enable_color_description) {
      /*  color_primaries  */
      radeon_enc_code_fixed_bits(enc, enc->enc_pic.av1_color_description.color_primaries, 8);
      /*  transfer_characteristics  */
      radeon_enc_code_fixed_bits(enc, enc->enc_pic.av1_color_description.transfer_characteristics, 8);
      /*  matrix_coefficients  */
      radeon_enc_code_fixed_bits(enc, enc->enc_pic.av1_color_description.maxtrix_coefficients, 8);
   }
   /*  color_range  */
   radeon_enc_code_fixed_bits(enc, enc->enc_pic.av1_color_description.color_range, 1);
   /*  chroma_sample_position  */
   radeon_enc_code_fixed_bits(enc, enc->enc_pic.av1_color_description.chroma_sample_position, 2);
   /*  separate_uv_delta_q  */
   radeon_enc_code_fixed_bits(enc, 0, 1);
   /*  film_grain_params_present  */
   radeon_enc_code_fixed_bits(enc, 0, 1);

   /*  trailing_one_bit  */
   radeon_enc_code_fixed_bits(enc, 1, 1);
   radeon_enc_byte_align(enc);

   /* obu_size doesn't include the bytes within obu_header
    * or obu_size syntax element (6.2.1), here we use 2 bytes for obu_size syntax
    * which needs to be removed from the size.
    */
   obu_size = (uint32_t)(radeon_enc_av1_header_size_offset(enc) - size_offset - 2);
   radeon_enc_code_leb128(obu_size_bin, obu_size, 2);

   /* update obu_size */
   for (int i = 0; i < sizeof(obu_size_bin); i++) {
      uint8_t *p = (uint8_t *)((((uintptr_t)size_offset & 3) ^ 3) | ((uintptr_t)size_offset & ~3));
      *p = obu_size_bin[i];
      size_offset++;
   }
}

static void radeon_enc_av1_frame_header(struct radeon_encoder *enc, bool frame_header)
{
   uint32_t i;
   uint32_t extension_flag = enc->enc_pic.num_temporal_layers > 1 ? 1 : 0;
   bool show_existing = false;
   bool frame_is_intra = enc->enc_pic.frame_type == PIPE_AV1_ENC_FRAME_TYPE_KEY ||
                         enc->enc_pic.frame_type == PIPE_AV1_ENC_FRAME_TYPE_INTRA_ONLY;

   radeon_enc_av1_bs_instruction_type(enc, RENCODE_AV1_BITSTREAM_INSTRUCTION_COPY, 0);
   /*  obu_header() */
   /*  obu_forbidden_bit  */
   radeon_enc_code_fixed_bits(enc, 0, 1);
   /*  obu_type  */
   radeon_enc_code_fixed_bits(enc, frame_header ? RENCODE_OBU_TYPE_FRAME_HEADER
                                                : RENCODE_OBU_TYPE_FRAME, 4);
   /*  obu_extension_flag  */
   radeon_enc_code_fixed_bits(enc, extension_flag, 1);
   /*  obu_has_size_field  */
   radeon_enc_code_fixed_bits(enc, 1, 1);
   /*  obu_reserved_1bit  */
   radeon_enc_code_fixed_bits(enc, 0, 1);
   if (extension_flag) {
      radeon_enc_code_fixed_bits(enc, enc->enc_pic.temporal_id, 3);
      radeon_enc_code_fixed_bits(enc, 0, 2);
      radeon_enc_code_fixed_bits(enc, 0, 3);
   }

   radeon_enc_av1_bs_instruction_type(enc, RENCODE_AV1_BITSTREAM_INSTRUCTION_OBU_SIZE, 0);

   /*  uncompressed_header() */
   radeon_enc_av1_bs_instruction_type(enc, RENCODE_AV1_BITSTREAM_INSTRUCTION_COPY, 0);
   /*  show_existing_frame  */
   show_existing = enc->enc_pic.frame_type == PIPE_AV1_ENC_FRAME_TYPE_SHOW_EXISTING;
   radeon_enc_code_fixed_bits(enc, show_existing ? 1 : 0, 1);
   /*  if (show_existing_frame == 1) */
   if(show_existing) {
      /*  frame_to_show_map_idx  */
      radeon_enc_code_fixed_bits(enc, enc->enc_pic.frame_to_show_map_index, 3);
      /*  display_frame_id  */
      if (enc->enc_pic.frame_id_numbers_present)
         radeon_enc_code_fixed_bits(enc, enc->enc_pic.display_frame_id,
                                                 RENCODE_AV1_DELTA_FRAME_ID_LENGTH +
                                                 RENCODE_AV1_ADDITIONAL_FRAME_ID_LENGTH);
   } else {
      /*  frame_type  */
      radeon_enc_code_fixed_bits(enc, enc->enc_pic.frame_type, 2);
      /*  show_frame  */
      radeon_enc_code_fixed_bits(enc, 1, 1);
      bool error_resilient_mode = false;
      if ((enc->enc_pic.frame_type == PIPE_AV1_ENC_FRAME_TYPE_SWITCH) ||
            (enc->enc_pic.frame_type == PIPE_AV1_ENC_FRAME_TYPE_KEY))
         error_resilient_mode = true;
      else {
         /*  error_resilient_mode  */
         radeon_enc_code_fixed_bits(enc, enc->enc_pic.enable_error_resilient_mode ? 1 : 0, 1);
         error_resilient_mode = enc->enc_pic.enable_error_resilient_mode;
      }
      /*  disable_cdf_update  */
      radeon_enc_code_fixed_bits(enc, enc->enc_pic.av1_spec_misc.disable_cdf_update ? 1 : 0, 1);

      bool allow_screen_content_tools = false;
      if (!enc->enc_pic.disable_screen_content_tools) {
         /*  allow_screen_content_tools  */
         allow_screen_content_tools = enc->enc_pic.av1_spec_misc.palette_mode_enable ||
                                      enc->enc_pic.force_integer_mv;
         radeon_enc_code_fixed_bits(enc, allow_screen_content_tools ? 1 : 0, 1);
      }

      if (allow_screen_content_tools)
         /*  force_integer_mv  */
         radeon_enc_code_fixed_bits(enc, enc->enc_pic.force_integer_mv ? 1 : 0, 1);

      if (enc->enc_pic.frame_id_numbers_present)
         /*  current_frame_id  */
         radeon_enc_code_fixed_bits(enc, enc->enc_pic.frame_id,
               RENCODE_AV1_DELTA_FRAME_ID_LENGTH +
               RENCODE_AV1_ADDITIONAL_FRAME_ID_LENGTH);

      bool frame_size_override = false;
      if (enc->enc_pic.frame_type == PIPE_AV1_ENC_FRAME_TYPE_SWITCH)
         frame_size_override = true;
      else {
         /*  frame_size_override_flag  */
         frame_size_override = false;
         radeon_enc_code_fixed_bits(enc, 0, 1);
      }

      if (enc->enc_pic.enable_order_hint)
         radeon_enc_code_fixed_bits(enc, enc->enc_pic.order_hint, enc->enc_pic.order_hint_bits);

      if (!frame_is_intra && !error_resilient_mode)
         /*  primary_ref_frame  */
         radeon_enc_code_fixed_bits(enc, 0, 3);         /* always LAST_FRAME(1) */

      if ((enc->enc_pic.frame_type != PIPE_AV1_ENC_FRAME_TYPE_SWITCH) &&
                                 (enc->enc_pic.frame_type != PIPE_AV1_ENC_FRAME_TYPE_KEY))
         /*  refresh_frame_flags  */
         radeon_enc_code_fixed_bits(enc, enc->enc_pic.refresh_frame_flags, 8);

      if ((!frame_is_intra || enc->enc_pic.refresh_frame_flags != 0xff) &&
                     error_resilient_mode && enc->enc_pic.enable_order_hint)
         for (i = 0; i < RENCDOE_AV1_NUM_REF_FRAMES; i++)
            /*  ref_order_hint  */
            radeon_enc_code_fixed_bits(enc, enc->enc_pic.reference_order_hint[i], enc->enc_pic.order_hint_bits);

      if (frame_is_intra) {
         /*  render_and_frame_size_different  */
         radeon_enc_code_fixed_bits(enc, enc->enc_pic.enable_render_size ? 1 : 0, 1);
         if (enc->enc_pic.enable_render_size) {
            /*  render_width_minus_1  */
            radeon_enc_code_fixed_bits(enc, enc->enc_pic.render_width - 1, 16);
            /*  render_height_minus_1  */
            radeon_enc_code_fixed_bits(enc, enc->enc_pic.render_height - 1, 16);
         }
         if (!enc->enc_pic.disable_screen_content_tools &&
               (enc->enc_pic.av1_spec_misc.palette_mode_enable || enc->enc_pic.force_integer_mv))
            /*  allow_intrabc  */
            radeon_enc_code_fixed_bits(enc, 0, 1);
      } else {
         if (enc->enc_pic.enable_order_hint)
            /*  frame_refs_short_signaling  */
            radeon_enc_code_fixed_bits(enc, 0, 1);
         for (i = 0; i < RENCDOE_AV1_REFS_PER_FRAME; i++) {
            /*  ref_frame_idx  */
            radeon_enc_code_fixed_bits(enc, enc->enc_pic.reference_frame_index, 3);
            if (enc->enc_pic.frame_id_numbers_present)
               radeon_enc_code_fixed_bits(enc,
                                          enc->enc_pic.reference_delta_frame_id - 1,
                                          RENCODE_AV1_DELTA_FRAME_ID_LENGTH);
         }

         if (frame_size_override && !error_resilient_mode)
            /*  found_ref  */
            radeon_enc_code_fixed_bits(enc, 1, 1);
         else {
            if(frame_size_override) {
               /*  frame_width_minus_1  */
               uint32_t used_bits =
                        radeon_enc_value_bits(enc->enc_pic.session_init.aligned_picture_width - 1);
               radeon_enc_code_fixed_bits(enc, enc->enc_pic.session_init.aligned_picture_width - 1,
                                               used_bits);
               /*  frame_height_minus_1  */
               used_bits = radeon_enc_value_bits(enc->enc_pic.session_init.aligned_picture_height - 1);
               radeon_enc_code_fixed_bits(enc, enc->enc_pic.session_init.aligned_picture_height - 1,
                                               used_bits);
            }
            /*  render_and_frame_size_different  */
            radeon_enc_code_fixed_bits(enc, enc->enc_pic.enable_render_size ? 1 : 0, 1);
            if (enc->enc_pic.enable_render_size) {
               /*  render_width_minus_1  */
               radeon_enc_code_fixed_bits(enc, enc->enc_pic.render_width - 1, 16);
               /*  render_height_minus_1  */
               radeon_enc_code_fixed_bits(enc, enc->enc_pic.render_height - 1, 16);
            }
         }

         if (enc->enc_pic.disable_screen_content_tools || !enc->enc_pic.force_integer_mv)
            /*  allow_high_precision_mv  */
            radeon_enc_av1_bs_instruction_type(enc, RENCODE_AV1_BITSTREAM_INSTRUCTION_ALLOW_HIGH_PRECISION_MV, 0);

         /*  read_interpolation_filter  */
         radeon_enc_av1_bs_instruction_type(enc, RENCODE_AV1_BITSTREAM_INSTRUCTION_READ_INTERPOLATION_FILTER, 0);

         radeon_enc_av1_bs_instruction_type(enc, RENCODE_AV1_BITSTREAM_INSTRUCTION_COPY, 0);
         /*  is_motion_mode_switchable  */
         radeon_enc_code_fixed_bits(enc, 0, 1);
      }

      if (!enc->enc_pic.av1_spec_misc.disable_cdf_update)
         /*  disable_frame_end_update_cdf  */
         radeon_enc_code_fixed_bits(enc, enc->enc_pic.av1_spec_misc.disable_frame_end_update_cdf ? 1 : 0, 1);

      /*  tile_info  */
      radeon_enc_av1_bs_instruction_type(enc, RENCODE_AV1_BITSTREAM_INSTRUCTION_TILE_INFO, 0);
      /*  quantization_params  */
      radeon_enc_av1_bs_instruction_type(enc, RENCODE_AV1_BITSTREAM_INSTRUCTION_QUANTIZATION_PARAMS, 0);
      /*  segmentation_enable  */
      radeon_enc_av1_bs_instruction_type(enc, RENCODE_AV1_BITSTREAM_INSTRUCTION_COPY, 0);
      radeon_enc_code_fixed_bits(enc, 0, 1);
      /*  delta_q_params  */
      radeon_enc_av1_bs_instruction_type(enc, RENCODE_AV1_BITSTREAM_INSTRUCTION_DELTA_Q_PARAMS, 0);
      /*  delta_lf_params  */
      radeon_enc_av1_bs_instruction_type(enc, RENCODE_AV1_BITSTREAM_INSTRUCTION_DELTA_LF_PARAMS, 0);
      /*  loop_filter_params  */
      radeon_enc_av1_bs_instruction_type(enc, RENCODE_AV1_BITSTREAM_INSTRUCTION_LOOP_FILTER_PARAMS, 0);
      /*  cdef_params  */
      radeon_enc_av1_bs_instruction_type(enc, RENCODE_AV1_BITSTREAM_INSTRUCTION_CDEF_PARAMS, 0);
      /*  lr_params  */
      /*  read_tx_mode  */
      radeon_enc_av1_bs_instruction_type(enc, RENCODE_AV1_BITSTREAM_INSTRUCTION_READ_TX_MODE, 0);

      radeon_enc_av1_bs_instruction_type(enc, RENCODE_AV1_BITSTREAM_INSTRUCTION_COPY, 0);
      if (!frame_is_intra)
         /*  reference_select  */
         radeon_enc_code_fixed_bits(enc, 0, 1);

      radeon_enc_code_fixed_bits(enc, 0, 1);
      if (!frame_is_intra)
         for (uint32_t ref = 1 /*LAST_FRAME*/; ref <= 7 /*ALTREF_FRAME*/; ref++)
            /*  is_global  */
            radeon_enc_code_fixed_bits(enc, 0, 1);
      /*  film_grain_params() */
   }
}

static void radeon_enc_av1_tile_group(struct radeon_encoder *enc)
{
   uint32_t extension_flag = enc->enc_pic.num_temporal_layers > 1 ? 1 : 0;

   radeon_enc_av1_bs_instruction_type(enc, RENCODE_AV1_BITSTREAM_INSTRUCTION_OBU_START,
                                           RENCODE_OBU_START_TYPE_TILE_GROUP);
   radeon_enc_av1_bs_instruction_type(enc, RENCODE_AV1_BITSTREAM_INSTRUCTION_COPY, 0);

   /*  obu_header() */
   /*  obu_forbidden_bit  */
   radeon_enc_code_fixed_bits(enc, 0, 1);
   /*  obu_type  */
   radeon_enc_code_fixed_bits(enc, RENCODE_OBU_TYPE_TILE_GROUP, 4);
   /*  obu_extension_flag  */
   radeon_enc_code_fixed_bits(enc, extension_flag, 1);
   /*  obu_has_size_field  */
   radeon_enc_code_fixed_bits(enc, 1, 1);
   /*  obu_reserved_1bit  */
   radeon_enc_code_fixed_bits(enc, 0, 1);

   if (extension_flag) {
      radeon_enc_code_fixed_bits(enc, enc->enc_pic.temporal_id, 3);
      radeon_enc_code_fixed_bits(enc, 0, 2);
      radeon_enc_code_fixed_bits(enc, 0, 3);
   }

   radeon_enc_av1_bs_instruction_type(enc, RENCODE_AV1_BITSTREAM_INSTRUCTION_OBU_SIZE, 0);
   radeon_enc_av1_bs_instruction_type(enc, RENCODE_AV1_BITSTREAM_INSTRUCTION_TILE_GROUP_OBU, 0);
   radeon_enc_av1_bs_instruction_type(enc, RENCODE_AV1_BITSTREAM_INSTRUCTION_OBU_END, 0);
}

static void radeon_enc_obu_instruction(struct radeon_encoder *enc)
{
   bool frame_header = !enc->enc_pic.stream_obu_frame ||
                       (enc->enc_pic.frame_type == PIPE_AV1_ENC_FRAME_TYPE_SHOW_EXISTING);
   radeon_enc_reset(enc);
   RADEON_ENC_BEGIN(enc->cmd.bitstream_instruction_av1);
   radeon_enc_av1_bs_instruction_type(enc, RENCODE_AV1_BITSTREAM_INSTRUCTION_COPY, 0);

   radeon_enc_av1_temporal_delimiter(enc);
   if (enc->enc_pic.need_av1_seq || enc->enc_pic.need_sequence_header)
      radeon_enc_av1_sequence_header(enc);

   /* if others OBU types are needed such as meta data, then they need to be byte aligned and added here
    *
    * if (others)
    *    radeon_enc_av1_others(enc); */

   radeon_enc_av1_bs_instruction_type(enc,
         RENCODE_AV1_BITSTREAM_INSTRUCTION_OBU_START,
            frame_header ? RENCODE_OBU_START_TYPE_FRAME_HEADER
                         : RENCODE_OBU_START_TYPE_FRAME);

   radeon_enc_av1_frame_header(enc, frame_header);

   if (!frame_header && (enc->enc_pic.frame_type != PIPE_AV1_ENC_FRAME_TYPE_SHOW_EXISTING))
      radeon_enc_av1_bs_instruction_type(enc, RENCODE_AV1_BITSTREAM_INSTRUCTION_TILE_GROUP_OBU, 0);

   radeon_enc_av1_bs_instruction_type(enc, RENCODE_AV1_BITSTREAM_INSTRUCTION_OBU_END, 0);

   if (frame_header && (enc->enc_pic.frame_type != PIPE_AV1_ENC_FRAME_TYPE_SHOW_EXISTING))
      radeon_enc_av1_tile_group(enc);

   radeon_enc_av1_bs_instruction_type(enc, RENCODE_AV1_BITSTREAM_INSTRUCTION_END, 0);
   RADEON_ENC_END();
}

/* av1 encode params */
static void radeon_enc_av1_encode_params(struct radeon_encoder *enc)
{
   switch (enc->enc_pic.frame_type) {
   case PIPE_AV1_ENC_FRAME_TYPE_KEY:
      enc->enc_pic.enc_params.pic_type = RENCODE_PICTURE_TYPE_I;
      break;
   case PIPE_AV1_ENC_FRAME_TYPE_INTRA_ONLY:
      enc->enc_pic.enc_params.pic_type = RENCODE_PICTURE_TYPE_I;
      break;
   case PIPE_AV1_ENC_FRAME_TYPE_INTER:
   case PIPE_AV1_ENC_FRAME_TYPE_SWITCH:
   case PIPE_AV1_ENC_FRAME_TYPE_SHOW_EXISTING:
      enc->enc_pic.enc_params.pic_type = RENCODE_PICTURE_TYPE_P;
      break;
   default:
      assert(0); /* never come to this condition */
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

   /* show existing type doesn't need input picture */
   if (enc->enc_pic.frame_type == PIPE_AV1_ENC_FRAME_TYPE_SHOW_EXISTING) {
      RADEON_ENC_CS(0);
      RADEON_ENC_CS(0);
      RADEON_ENC_CS(0);
      RADEON_ENC_CS(0);
   } else {
      RADEON_ENC_READ(enc->handle, RADEON_DOMAIN_VRAM, enc->luma->u.gfx9.surf_offset);
      RADEON_ENC_READ(enc->handle, RADEON_DOMAIN_VRAM, enc->chroma ?
         enc->chroma->u.gfx9.surf_offset : enc->luma->u.gfx9.surf_pitch);
   }

   RADEON_ENC_CS(enc->enc_pic.enc_params.input_pic_luma_pitch);
   RADEON_ENC_CS(enc->enc_pic.enc_params.input_pic_chroma_pitch);
   RADEON_ENC_CS(enc->enc_pic.enc_params.input_pic_swizzle_mode);
   RADEON_ENC_CS(enc->enc_pic.enc_params.reference_picture_index);
   RADEON_ENC_CS(enc->enc_pic.enc_params.reconstructed_picture_index);
   RADEON_ENC_END();
}

static uint32_t radeon_enc_ref_swizzle_mode(struct radeon_encoder *enc)
{
   /* return RENCODE_REC_SWIZZLE_MODE_LINEAR; for debugging purpose */
   if (enc->enc_pic.bit_depth_luma_minus8 != 0)
      return RENCODE_REC_SWIZZLE_MODE_8x8_1D_THIN_12_24BPP;
   else
      return RENCODE_REC_SWIZZLE_MODE_256B_D;
}

static void radeon_enc_ctx(struct radeon_encoder *enc)
{

   bool is_av1 = u_reduce_video_profile(enc->base.profile)
                                           == PIPE_VIDEO_FORMAT_AV1;
   enc->enc_pic.ctx_buf.swizzle_mode = radeon_enc_ref_swizzle_mode(enc);
   enc->enc_pic.ctx_buf.two_pass_search_center_map_offset = 0;

   RADEON_ENC_BEGIN(enc->cmd.ctx);
   RADEON_ENC_READWRITE(enc->dpb->res->buf, enc->dpb->res->domains, 0);
   RADEON_ENC_CS(enc->enc_pic.ctx_buf.swizzle_mode);
   RADEON_ENC_CS(enc->enc_pic.ctx_buf.rec_luma_pitch);
   RADEON_ENC_CS(enc->enc_pic.ctx_buf.rec_chroma_pitch);
   RADEON_ENC_CS(enc->enc_pic.ctx_buf.num_reconstructed_pictures);

   for (int i = 0; i < RENCODE_MAX_NUM_RECONSTRUCTED_PICTURES; i++) {
      rvcn_enc_reconstructed_picture_t *pic =
                            &enc->enc_pic.ctx_buf.reconstructed_pictures[i];
      RADEON_ENC_CS(pic->luma_offset);
      RADEON_ENC_CS(pic->chroma_offset);
      if (is_av1) {
         RADEON_ENC_CS(pic->av1.av1_cdf_frame_context_offset);
         RADEON_ENC_CS(pic->av1.av1_cdef_algorithm_context_offset);
      } else {
         RADEON_ENC_CS(0x00000000); /* unused offset 1 */
         RADEON_ENC_CS(0x00000000); /* unused offset 2 */
      }
   }

   RADEON_ENC_CS(enc->enc_pic.ctx_buf.pre_encode_picture_luma_pitch);
   RADEON_ENC_CS(enc->enc_pic.ctx_buf.pre_encode_picture_chroma_pitch);

   for (int i = 0; i < RENCODE_MAX_NUM_RECONSTRUCTED_PICTURES; i++) {
      rvcn_enc_reconstructed_picture_t *pic =
                            &enc->enc_pic.ctx_buf.pre_encode_reconstructed_pictures[i];
      RADEON_ENC_CS(pic->luma_offset);
      RADEON_ENC_CS(pic->chroma_offset);
      if (is_av1) {
         RADEON_ENC_CS(pic->av1.av1_cdf_frame_context_offset);
         RADEON_ENC_CS(pic->av1.av1_cdef_algorithm_context_offset);
      } else {
         RADEON_ENC_CS(0x00000000); /* unused offset 1 */
         RADEON_ENC_CS(0x00000000); /* unused offset 2 */
      }
   }

   RADEON_ENC_CS(enc->enc_pic.ctx_buf.pre_encode_input_picture.rgb.red_offset);
   RADEON_ENC_CS(enc->enc_pic.ctx_buf.pre_encode_input_picture.rgb.green_offset);
   RADEON_ENC_CS(enc->enc_pic.ctx_buf.pre_encode_input_picture.rgb.blue_offset);

   RADEON_ENC_CS(enc->enc_pic.ctx_buf.two_pass_search_center_map_offset);
   if (is_av1)
      RADEON_ENC_CS(enc->enc_pic.ctx_buf.av1.av1_sdb_intermedidate_context_offset);
   else
      RADEON_ENC_CS(enc->enc_pic.ctx_buf.colloc_buffer_offset);
   RADEON_ENC_END();
}

static void radeon_enc_header_av1(struct radeon_encoder *enc)
{
   enc->obu_instructions(enc);
   enc->encode_params(enc);
   enc->cdf_default_table(enc);

   enc->enc_pic.frame_id++;
   if (enc->enc_pic.frame_id > (1 << (RENCODE_AV1_DELTA_FRAME_ID_LENGTH - 2)))
      enc->enc_pic.frame_id = 0;
}

void radeon_enc_4_0_init(struct radeon_encoder *enc)
{
   radeon_enc_3_0_init(enc);

   enc->session_init = radeon_enc_session_init;
   enc->ctx = radeon_enc_ctx;
   enc->mq_begin = enc->begin;
   enc->mq_encode = enc->encode;
   enc->mq_destroy = enc->destroy;
   enc->begin = radeon_enc_sq_begin;
   enc->encode = radeon_enc_sq_encode;
   enc->destroy = radeon_enc_sq_destroy;
   enc->op_preset = radeon_enc_op_preset;

   if (u_reduce_video_profile(enc->base.profile) == PIPE_VIDEO_FORMAT_AV1) {
      enc->before_encode = radeon_enc_av1_dpb_management;
      /* begin function need to set these two functions to dummy */
      enc->slice_control = radeon_enc_dummy;
      enc->deblocking_filter = radeon_enc_dummy;
      enc->cmd.cdf_default_table_av1 = RENCODE_IB_PARAM_CDF_DEFAULT_TABLE_BUFFER;
      enc->cmd.bitstream_instruction_av1 = RENCODE_AV1_IB_PARAM_BITSTREAM_INSTRUCTION;
      enc->cmd.spec_misc_av1 = RENCODE_AV1_IB_PARAM_SPEC_MISC;
      enc->spec_misc = radeon_enc_spec_misc_av1;
      enc->encode_headers = radeon_enc_header_av1;
      enc->obu_instructions = radeon_enc_obu_instruction;
      enc->cdf_default_table = radeon_enc_cdf_default_table;
      enc->encode_params = radeon_enc_av1_encode_params;
   }

   enc->cmd.enc_statistics = RENCODE_IB_PARAM_ENCODE_STATISTICS;

   enc->enc_pic.session_info.interface_version =
      ((RENCODE_FW_INTERFACE_MAJOR_VERSION << RENCODE_IF_MAJOR_VERSION_SHIFT) |
      (RENCODE_FW_INTERFACE_MINOR_VERSION << RENCODE_IF_MINOR_VERSION_SHIFT));
}
