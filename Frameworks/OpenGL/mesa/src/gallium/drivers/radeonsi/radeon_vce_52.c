/**************************************************************************
 *
 * Copyright 2015 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 *
 **************************************************************************/

#include "pipe/p_video_codec.h"
#include "radeon_vce.h"
#include "radeon_video.h"
#include "radeonsi/si_pipe.h"
#include "util/u_memory.h"
#include "util/u_video.h"
#include "vl/vl_video_buffer.h"

#include <stdio.h>

static void get_rate_control_param(struct rvce_encoder *enc, struct pipe_h264_enc_picture_desc *pic)
{
   enc->enc_pic.rc.rc_method = pic->rate_ctrl[0].rate_ctrl_method;
   enc->enc_pic.rc.target_bitrate = pic->rate_ctrl[0].target_bitrate;
   enc->enc_pic.rc.peak_bitrate = pic->rate_ctrl[0].peak_bitrate;
   enc->enc_pic.rc.quant_i_frames = pic->quant_i_frames;
   enc->enc_pic.rc.quant_p_frames = pic->quant_p_frames;
   enc->enc_pic.rc.quant_b_frames = pic->quant_b_frames;
   enc->enc_pic.rc.gop_size = pic->gop_size;
   enc->enc_pic.rc.frame_rate_num = pic->rate_ctrl[0].frame_rate_num;
   enc->enc_pic.rc.frame_rate_den = pic->rate_ctrl[0].frame_rate_den;
   enc->enc_pic.rc.max_qp = 51;
   enc->enc_pic.rc.vbv_buffer_size = pic->rate_ctrl[0].vbv_buffer_size;
   enc->enc_pic.rc.vbv_buf_lv = pic->rate_ctrl[0].vbv_buf_lv;
   enc->enc_pic.rc.fill_data_enable = pic->rate_ctrl[0].fill_data_enable;
   enc->enc_pic.rc.enforce_hrd = pic->rate_ctrl[0].enforce_hrd;
   enc->enc_pic.rc.target_bits_picture = pic->rate_ctrl[0].target_bits_picture;
   enc->enc_pic.rc.peak_bits_picture_integer = pic->rate_ctrl[0].peak_bits_picture_integer;
   enc->enc_pic.rc.peak_bits_picture_fraction = pic->rate_ctrl[0].peak_bits_picture_fraction;
}

static void get_motion_estimation_param(struct rvce_encoder *enc,
                                        struct pipe_h264_enc_picture_desc *pic)
{
   enc->enc_pic.me.motion_est_quarter_pixel = 1;
   enc->enc_pic.me.enc_disable_sub_mode = 254;
   enc->enc_pic.me.lsmvert = 2;
   enc->enc_pic.me.enc_en_ime_overw_dis_subm = 0;
   enc->enc_pic.me.enc_ime_overw_dis_subm_no = 0;
   enc->enc_pic.me.enc_ime2_search_range_x = 4;
   enc->enc_pic.me.enc_ime2_search_range_y = 4;
   enc->enc_pic.me.enc_ime_decimation_search = 0x00000001;
   enc->enc_pic.me.motion_est_half_pixel = 0x00000001;
   enc->enc_pic.me.enc_search_range_x = 0x00000010;
   enc->enc_pic.me.enc_search_range_y = 0x00000010;
   enc->enc_pic.me.enc_search1_range_x = 0x00000010;
   enc->enc_pic.me.enc_search1_range_y = 0x00000010;
}

static void get_pic_control_param(struct rvce_encoder *enc, struct pipe_h264_enc_picture_desc *pic)
{
   unsigned encNumMBsPerSlice;
   encNumMBsPerSlice = align(enc->base.width, 16) / 16;
   encNumMBsPerSlice *= align(enc->base.height, 16) / 16;
   if (pic->seq.enc_frame_cropping_flag) {
      enc->enc_pic.pc.enc_crop_left_offset = pic->seq.enc_frame_crop_left_offset;
      enc->enc_pic.pc.enc_crop_right_offset = pic->seq.enc_frame_crop_right_offset;
      enc->enc_pic.pc.enc_crop_top_offset = pic->seq.enc_frame_crop_top_offset;
      enc->enc_pic.pc.enc_crop_bottom_offset = pic->seq.enc_frame_crop_bottom_offset;
   } else {
      enc->enc_pic.pc.enc_crop_right_offset = (align(enc->base.width, 16) - enc->base.width) >> 1;
      enc->enc_pic.pc.enc_crop_bottom_offset =
         (align(enc->base.height, 16) - enc->base.height) >> 1;
   }
   enc->enc_pic.pc.enc_num_mbs_per_slice = encNumMBsPerSlice;
   enc->enc_pic.pc.enc_b_pic_pattern = MAX2(enc->base.max_references, 1) - 1;
   enc->enc_pic.pc.enc_number_of_reference_frames = MIN2(enc->base.max_references, 1);
   enc->enc_pic.pc.enc_max_num_ref_frames = enc->base.max_references + 1;
   enc->enc_pic.pc.enc_num_default_active_ref_l0 = 0x00000001;
   enc->enc_pic.pc.enc_num_default_active_ref_l1 = 0x00000001;
   enc->enc_pic.pc.enc_cabac_enable = pic->pic_ctrl.enc_cabac_enable;
   enc->enc_pic.pc.enc_constraint_set_flags = 0x00000040;
}

static void get_task_info_param(struct rvce_encoder *enc)
{
   enc->enc_pic.ti.offset_of_next_task_info = 0xffffffff;
}

static void get_feedback_buffer_param(struct rvce_encoder *enc, struct pipe_enc_feedback_metadata* metadata)
{
   enc->enc_pic.fb.feedback_ring_size = 0x00000001;
}

static void get_config_ext_param(struct rvce_encoder *enc)
{
   enc->enc_pic.ce.enc_enable_perf_logging = 0x00000003;
}

static void get_vui_param(struct rvce_encoder *enc, struct pipe_h264_enc_picture_desc *pic)
{
   enc->enc_pic.enable_vui = pic->seq.vui_parameters_present_flag;
   enc->enc_pic.vui.aspect_ratio_info_present_flag =
      pic->seq.vui_flags.aspect_ratio_info_present_flag;
   enc->enc_pic.vui.aspect_ratio_idc = pic->seq.aspect_ratio_idc;
   enc->enc_pic.vui.sar_width = pic->seq.sar_width;
   enc->enc_pic.vui.sar_height = pic->seq.sar_height;
   enc->enc_pic.vui.video_signal_type_present_flag =
      pic->seq.vui_flags.video_signal_type_present_flag;
   enc->enc_pic.vui.video_format = pic->seq.video_format;
   enc->enc_pic.vui.video_full_range_flag = pic->seq.video_full_range_flag;
   enc->enc_pic.vui.color_description_present_flag =
      pic->seq.vui_flags.colour_description_present_flag;
   enc->enc_pic.vui.color_prim = pic->seq.colour_primaries;
   enc->enc_pic.vui.transfer_char = pic->seq.transfer_characteristics;
   enc->enc_pic.vui.matrix_coef = pic->seq.matrix_coefficients;
   enc->enc_pic.vui.chroma_loc_info_present_flag =
      pic->seq.vui_flags.chroma_loc_info_present_flag;
   enc->enc_pic.vui.chroma_loc_top = pic->seq.chroma_sample_loc_type_top_field;
   enc->enc_pic.vui.chroma_loc_bottom = pic->seq.chroma_sample_loc_type_bottom_field;
   enc->enc_pic.vui.timing_info_present_flag = pic->seq.vui_flags.timing_info_present_flag;
   enc->enc_pic.vui.num_units_in_tick = pic->rate_ctrl[0].frame_rate_den;
   enc->enc_pic.vui.time_scale = pic->rate_ctrl[0].frame_rate_num * 2;
   enc->enc_pic.vui.fixed_frame_rate_flag = 0x00000001;
   enc->enc_pic.vui.bit_rate_scale = 0x00000004;
   enc->enc_pic.vui.cpb_size_scale = 0x00000006;
   enc->enc_pic.vui.initial_cpb_removal_delay_length_minus1 = 0x00000017;
   enc->enc_pic.vui.cpb_removal_delay_length_minus1 = 0x00000017;
   enc->enc_pic.vui.dpb_output_delay_length_minus1 = 0x00000017;
   enc->enc_pic.vui.time_offset_length = 0x00000018;
   enc->enc_pic.vui.motion_vectors_over_pic_boundaries_flag = 0x00000001;
   enc->enc_pic.vui.max_bytes_per_pic_denom = 0x00000002;
   enc->enc_pic.vui.max_bits_per_mb_denom = 0x00000001;
   enc->enc_pic.vui.log2_max_mv_length_hori = 0x00000010;
   enc->enc_pic.vui.log2_max_mv_length_vert = 0x00000010;
   enc->enc_pic.vui.num_reorder_frames = 0x00000003;
   enc->enc_pic.vui.max_dec_frame_buffering = 0x00000003;
}

void si_vce_52_get_param(struct rvce_encoder *enc, struct pipe_h264_enc_picture_desc *pic)
{
   get_rate_control_param(enc, pic);
   get_motion_estimation_param(enc, pic);
   get_pic_control_param(enc, pic);
   get_task_info_param(enc);
   get_feedback_buffer_param(enc, NULL);
   get_vui_param(enc, pic);
   get_config_ext_param(enc);

   enc->enc_pic.picture_type = pic->picture_type;
   enc->enc_pic.frame_num = pic->frame_num;
   enc->enc_pic.frame_num_cnt = pic->frame_num_cnt;
   enc->enc_pic.p_remain = pic->p_remain;
   enc->enc_pic.i_remain = pic->i_remain;
   enc->enc_pic.gop_cnt = pic->gop_cnt;
   enc->enc_pic.pic_order_cnt = pic->pic_order_cnt;
   enc->enc_pic.ref_idx_l0 = pic->ref_idx_l0_list[0];
   enc->enc_pic.ref_idx_l1 = pic->ref_idx_l1_list[0];
   enc->enc_pic.not_referenced = pic->not_referenced;
   if (enc->dual_inst)
      enc->enc_pic.addrmode_arraymode_disrdo_distwoinstants = 0x00000201;
   else
      enc->enc_pic.addrmode_arraymode_disrdo_distwoinstants = 0x01000201;
   enc->enc_pic.is_idr = (pic->picture_type == PIPE_H2645_ENC_PICTURE_TYPE_IDR);
}

static void create(struct rvce_encoder *enc)
{
   struct si_screen *sscreen = (struct si_screen *)enc->screen;
   enc->task_info(enc, 0x00000000, 0, 0, 0);

   RVCE_BEGIN(0x01000001); // create cmd
   RVCE_CS(enc->enc_pic.ec.enc_use_circular_buffer);
   RVCE_CS(u_get_h264_profile_idc(enc->base.profile)); // encProfile
   RVCE_CS(enc->base.level);                           // encLevel
   RVCE_CS(enc->enc_pic.ec.enc_pic_struct_restriction);
   RVCE_CS(enc->base.width);  // encImageWidth
   RVCE_CS(enc->base.height); // encImageHeight

   if (sscreen->info.gfx_level < GFX9) {
      RVCE_CS(enc->luma->u.legacy.level[0].nblk_x * enc->luma->bpe);     // encRefPicLumaPitch
      RVCE_CS(enc->chroma->u.legacy.level[0].nblk_x * enc->chroma->bpe); // encRefPicChromaPitch
      RVCE_CS(align(enc->luma->u.legacy.level[0].nblk_y, 16) / 8);       // encRefYHeightInQw
   } else {
      RVCE_CS(enc->luma->u.gfx9.surf_pitch * enc->luma->bpe);     // encRefPicLumaPitch
      RVCE_CS(enc->chroma->u.gfx9.surf_pitch * enc->chroma->bpe); // encRefPicChromaPitch
      RVCE_CS(align(enc->luma->u.gfx9.surf_height, 16) / 8);      // encRefYHeightInQw
   }

   RVCE_CS(enc->enc_pic.addrmode_arraymode_disrdo_distwoinstants);

   RVCE_CS(enc->enc_pic.ec.enc_pre_encode_context_buffer_offset);
   RVCE_CS(enc->enc_pic.ec.enc_pre_encode_input_luma_buffer_offset);
   RVCE_CS(enc->enc_pic.ec.enc_pre_encode_input_chroma_buffer_offset);
   RVCE_CS(enc->enc_pic.ec.enc_pre_encode_mode_chromaflag_vbaqmode_scenechangesensitivity);
   RVCE_END();
}

static void encode(struct rvce_encoder *enc)
{
   struct si_screen *sscreen = (struct si_screen *)enc->screen;
   signed luma_offset, chroma_offset, bs_offset;
   unsigned dep, bs_idx = enc->bs_idx++;
   int i;

   if (enc->dual_inst) {
      if (bs_idx == 0)
         dep = 1;
      else if (enc->enc_pic.picture_type == PIPE_H2645_ENC_PICTURE_TYPE_IDR)
         dep = 0;
      else
         dep = 2;
   } else
      dep = 0;

   enc->task_info(enc, 0x00000003, dep, 0, bs_idx);

   RVCE_BEGIN(0x05000001);                                      // context buffer
   RVCE_READWRITE(enc->cpb.res->buf, enc->cpb.res->domains, 0); // encodeContextAddressHi/Lo
   RVCE_END();

   bs_offset = -(signed)(bs_idx * enc->bs_size);

   RVCE_BEGIN(0x05000004);                                   // video bitstream buffer
   RVCE_WRITE(enc->bs_handle, RADEON_DOMAIN_GTT, bs_offset); // videoBitstreamRingAddressHi/Lo
   RVCE_CS(enc->bs_size);                                    // videoBitstreamRingSize
   RVCE_END();

   if (enc->dual_pipe) {
      unsigned aux_offset =
         enc->cpb.res->buf->size - RVCE_MAX_AUX_BUFFER_NUM * RVCE_MAX_BITSTREAM_OUTPUT_ROW_SIZE * 2;
      RVCE_BEGIN(0x05000002); // auxiliary buffer
      for (i = 0; i < 8; ++i) {
         RVCE_CS(aux_offset);
         aux_offset += RVCE_MAX_BITSTREAM_OUTPUT_ROW_SIZE;
      }
      for (i = 0; i < 8; ++i)
         RVCE_CS(RVCE_MAX_BITSTREAM_OUTPUT_ROW_SIZE);
      RVCE_END();
   }

   RVCE_BEGIN(0x03000001);                       // encode
   RVCE_CS(enc->enc_pic.frame_num ? 0x0 : 0x11); // insertHeaders
   RVCE_CS(enc->enc_pic.eo.picture_structure);
   RVCE_CS(enc->bs_size); // allowedMaxBitstreamSize
   RVCE_CS(enc->enc_pic.eo.force_refresh_map);
   RVCE_CS(enc->enc_pic.eo.insert_aud);
   RVCE_CS(enc->enc_pic.eo.end_of_sequence);
   RVCE_CS(enc->enc_pic.eo.end_of_stream);

   if (sscreen->info.gfx_level < GFX9) {
      RVCE_READ(enc->handle, RADEON_DOMAIN_VRAM,
                (uint64_t)enc->luma->u.legacy.level[0].offset_256B * 256); // inputPictureLumaAddressHi/Lo
      RVCE_READ(enc->handle, RADEON_DOMAIN_VRAM,
                (uint64_t)enc->chroma->u.legacy.level[0].offset_256B * 256);        // inputPictureChromaAddressHi/Lo
      RVCE_CS(align(enc->luma->u.legacy.level[0].nblk_y, 16)); // encInputFrameYPitch
      RVCE_CS(enc->luma->u.legacy.level[0].nblk_x * enc->luma->bpe);     // encInputPicLumaPitch
      RVCE_CS(enc->chroma->u.legacy.level[0].nblk_x * enc->chroma->bpe); // encInputPicChromaPitch
   } else {
      RVCE_READ(enc->handle, RADEON_DOMAIN_VRAM,
                enc->luma->u.gfx9.surf_offset); // inputPictureLumaAddressHi/Lo
      RVCE_READ(enc->handle, RADEON_DOMAIN_VRAM,
                enc->chroma->u.gfx9.surf_offset);                 // inputPictureChromaAddressHi/Lo
      RVCE_CS(align(enc->luma->u.gfx9.surf_height, 16));          // encInputFrameYPitch
      RVCE_CS(enc->luma->u.gfx9.surf_pitch * enc->luma->bpe);     // encInputPicLumaPitch
      RVCE_CS(enc->chroma->u.gfx9.surf_pitch * enc->chroma->bpe); // encInputPicChromaPitch
   }

   if (enc->dual_pipe)
      enc->enc_pic.eo.enc_input_pic_addr_array_disable2pipe_disablemboffload = 0x00000000;
   else
      enc->enc_pic.eo.enc_input_pic_addr_array_disable2pipe_disablemboffload = 0x00010000;
   RVCE_CS(enc->enc_pic.eo.enc_input_pic_addr_array_disable2pipe_disablemboffload);
   RVCE_CS(enc->enc_pic.eo.enc_input_pic_tile_config);
   RVCE_CS(enc->enc_pic.picture_type);                                    // encPicType
   RVCE_CS(enc->enc_pic.picture_type == PIPE_H2645_ENC_PICTURE_TYPE_IDR); // encIdrFlag
   if ((enc->enc_pic.picture_type == PIPE_H2645_ENC_PICTURE_TYPE_IDR) &&
       (enc->enc_pic.eo.enc_idr_pic_id != 0))
      enc->enc_pic.eo.enc_idr_pic_id = enc->enc_pic.idr_pic_id - 1;
   else
      enc->enc_pic.eo.enc_idr_pic_id = 0x00000000;
   RVCE_CS(enc->enc_pic.eo.enc_idr_pic_id);
   RVCE_CS(enc->enc_pic.eo.enc_mgs_key_pic);
   RVCE_CS(!enc->enc_pic.not_referenced);
   RVCE_CS(enc->enc_pic.eo.enc_temporal_layer_index);
   RVCE_CS(enc->enc_pic.eo.num_ref_idx_active_override_flag);
   RVCE_CS(enc->enc_pic.eo.num_ref_idx_l0_active_minus1);
   RVCE_CS(enc->enc_pic.eo.num_ref_idx_l1_active_minus1);

   i = enc->enc_pic.frame_num - enc->enc_pic.ref_idx_l0;
   if (i > 1 && enc->enc_pic.picture_type == PIPE_H2645_ENC_PICTURE_TYPE_P) {
      enc->enc_pic.eo.enc_ref_list_modification_op = 0x00000001;
      enc->enc_pic.eo.enc_ref_list_modification_num = i - 1;
      RVCE_CS(enc->enc_pic.eo.enc_ref_list_modification_op);
      RVCE_CS(enc->enc_pic.eo.enc_ref_list_modification_num);
   } else {
      enc->enc_pic.eo.enc_ref_list_modification_op = 0x00000000;
      enc->enc_pic.eo.enc_ref_list_modification_num = 0x00000000;
      RVCE_CS(enc->enc_pic.eo.enc_ref_list_modification_op);
      RVCE_CS(enc->enc_pic.eo.enc_ref_list_modification_num);
   }

   for (i = 0; i < 3; ++i) {
      enc->enc_pic.eo.enc_ref_list_modification_op = 0x00000000;
      enc->enc_pic.eo.enc_ref_list_modification_num = 0x00000000;
      RVCE_CS(enc->enc_pic.eo.enc_ref_list_modification_op);
      RVCE_CS(enc->enc_pic.eo.enc_ref_list_modification_num);
   }
   for (i = 0; i < 4; ++i) {
      RVCE_CS(enc->enc_pic.eo.enc_decoded_picture_marking_op);
      RVCE_CS(enc->enc_pic.eo.enc_decoded_picture_marking_num);
      RVCE_CS(enc->enc_pic.eo.enc_decoded_picture_marking_idx);
      RVCE_CS(enc->enc_pic.eo.enc_decoded_ref_base_picture_marking_op);
      RVCE_CS(enc->enc_pic.eo.enc_decoded_ref_base_picture_marking_num);
   }

   // encReferencePictureL0[0]
   RVCE_CS(0x00000000); // pictureStructure
   if (enc->enc_pic.picture_type == PIPE_H2645_ENC_PICTURE_TYPE_P ||
       enc->enc_pic.picture_type == PIPE_H2645_ENC_PICTURE_TYPE_B) {
      struct rvce_cpb_slot *l0 = si_l0_slot(enc);
      si_vce_frame_offset(enc, l0, &luma_offset, &chroma_offset);
      RVCE_CS(l0->picture_type);
      RVCE_CS(l0->frame_num);
      RVCE_CS(l0->pic_order_cnt);
      RVCE_CS(luma_offset);
      RVCE_CS(chroma_offset);
   } else {
      enc->enc_pic.eo.l0_enc_pic_type = 0x00000000;
      enc->enc_pic.eo.l0_frame_number = 0x00000000;
      enc->enc_pic.eo.l0_picture_order_count = 0x00000000;
      enc->enc_pic.eo.l0_luma_offset = 0xffffffff;
      enc->enc_pic.eo.l0_chroma_offset = 0xffffffff;
      RVCE_CS(enc->enc_pic.eo.l0_enc_pic_type);
      RVCE_CS(enc->enc_pic.eo.l0_frame_number);
      RVCE_CS(enc->enc_pic.eo.l0_picture_order_count);
      RVCE_CS(enc->enc_pic.eo.l0_luma_offset);
      RVCE_CS(enc->enc_pic.eo.l0_chroma_offset);
   }

   // encReferencePictureL0[1]
   enc->enc_pic.eo.l0_picture_structure = 0x00000000;
   enc->enc_pic.eo.l0_enc_pic_type = 0x00000000;
   enc->enc_pic.eo.l0_frame_number = 0x00000000;
   enc->enc_pic.eo.l0_picture_order_count = 0x00000000;
   enc->enc_pic.eo.l0_luma_offset = 0xffffffff;
   enc->enc_pic.eo.l0_chroma_offset = 0xffffffff;
   RVCE_CS(enc->enc_pic.eo.l0_picture_structure);
   RVCE_CS(enc->enc_pic.eo.l0_enc_pic_type);
   RVCE_CS(enc->enc_pic.eo.l0_frame_number);
   RVCE_CS(enc->enc_pic.eo.l0_picture_order_count);
   RVCE_CS(enc->enc_pic.eo.l0_luma_offset);
   RVCE_CS(enc->enc_pic.eo.l0_chroma_offset);

   // encReferencePictureL1[0]
   RVCE_CS(0x00000000); // pictureStructure
   if (enc->enc_pic.picture_type == PIPE_H2645_ENC_PICTURE_TYPE_B) {
      struct rvce_cpb_slot *l1 = si_l1_slot(enc);
      si_vce_frame_offset(enc, l1, &luma_offset, &chroma_offset);
      RVCE_CS(l1->picture_type);
      RVCE_CS(l1->frame_num);
      RVCE_CS(l1->pic_order_cnt);
      RVCE_CS(luma_offset);
      RVCE_CS(chroma_offset);
   } else {
      enc->enc_pic.eo.l1_enc_pic_type = 0x00000000;
      enc->enc_pic.eo.l1_frame_number = 0x00000000;
      enc->enc_pic.eo.l1_picture_order_count = 0x00000000;
      enc->enc_pic.eo.l1_luma_offset = 0xffffffff;
      enc->enc_pic.eo.l1_chroma_offset = 0xffffffff;
      RVCE_CS(enc->enc_pic.eo.l1_enc_pic_type);
      RVCE_CS(enc->enc_pic.eo.l1_frame_number);
      RVCE_CS(enc->enc_pic.eo.l1_picture_order_count);
      RVCE_CS(enc->enc_pic.eo.l1_luma_offset);
      RVCE_CS(enc->enc_pic.eo.l1_chroma_offset);
   }

   si_vce_frame_offset(enc, si_current_slot(enc), &luma_offset, &chroma_offset);
   RVCE_CS(luma_offset);
   RVCE_CS(chroma_offset);
   RVCE_CS(enc->enc_pic.eo.enc_coloc_buffer_offset);
   RVCE_CS(enc->enc_pic.eo.enc_reconstructed_ref_base_picture_luma_offset);
   RVCE_CS(enc->enc_pic.eo.enc_reconstructed_ref_base_picture_chroma_offset);
   RVCE_CS(enc->enc_pic.eo.enc_reference_ref_base_picture_luma_offset);
   RVCE_CS(enc->enc_pic.eo.enc_reference_ref_base_picture_chroma_offset);
   RVCE_CS(enc->enc_pic.frame_num_cnt - 1);
   RVCE_CS(enc->enc_pic.frame_num);
   RVCE_CS(enc->enc_pic.pic_order_cnt);
   RVCE_CS(enc->enc_pic.i_remain);
   RVCE_CS(enc->enc_pic.p_remain);
   RVCE_CS(enc->enc_pic.eo.num_b_pic_remain_in_rcgop);
   RVCE_CS(enc->enc_pic.eo.num_ir_pic_remain_in_rcgop);
   RVCE_CS(enc->enc_pic.eo.enable_intra_refresh);

   RVCE_CS(enc->enc_pic.eo.aq_variance_en);
   RVCE_CS(enc->enc_pic.eo.aq_block_size);
   RVCE_CS(enc->enc_pic.eo.aq_mb_variance_sel);
   RVCE_CS(enc->enc_pic.eo.aq_frame_variance_sel);
   RVCE_CS(enc->enc_pic.eo.aq_param_a);
   RVCE_CS(enc->enc_pic.eo.aq_param_b);
   RVCE_CS(enc->enc_pic.eo.aq_param_c);
   RVCE_CS(enc->enc_pic.eo.aq_param_d);
   RVCE_CS(enc->enc_pic.eo.aq_param_e);

   RVCE_CS(enc->enc_pic.eo.context_in_sfb);
   RVCE_END();
}

static void rate_control(struct rvce_encoder *enc)
{
   RVCE_BEGIN(0x04000005); // rate control
   RVCE_CS(enc->enc_pic.rc.rc_method);
   RVCE_CS(enc->enc_pic.rc.target_bitrate);
   RVCE_CS(enc->enc_pic.rc.peak_bitrate);
   RVCE_CS(enc->enc_pic.rc.frame_rate_num);
   RVCE_CS(enc->enc_pic.rc.gop_size);
   RVCE_CS(enc->enc_pic.rc.quant_i_frames);
   RVCE_CS(enc->enc_pic.rc.quant_p_frames);
   RVCE_CS(enc->enc_pic.rc.quant_b_frames);
   RVCE_CS(enc->enc_pic.rc.vbv_buffer_size);
   RVCE_CS(enc->enc_pic.rc.frame_rate_den);
   RVCE_CS(enc->enc_pic.rc.vbv_buf_lv);
   RVCE_CS(enc->enc_pic.rc.max_au_size);
   RVCE_CS(enc->enc_pic.rc.qp_initial_mode);
   RVCE_CS(enc->enc_pic.rc.target_bits_picture);
   RVCE_CS(enc->enc_pic.rc.peak_bits_picture_integer);
   RVCE_CS(enc->enc_pic.rc.peak_bits_picture_fraction);
   RVCE_CS(enc->enc_pic.rc.min_qp);
   RVCE_CS(enc->enc_pic.rc.max_qp);
   RVCE_CS(enc->enc_pic.rc.skip_frame_enable);
   RVCE_CS(enc->enc_pic.rc.fill_data_enable);
   RVCE_CS(enc->enc_pic.rc.enforce_hrd);
   RVCE_CS(enc->enc_pic.rc.b_pics_delta_qp);
   RVCE_CS(enc->enc_pic.rc.ref_b_pics_delta_qp);
   RVCE_CS(enc->enc_pic.rc.rc_reinit_disable);
   RVCE_CS(enc->enc_pic.rc.enc_lcvbr_init_qp_flag);
   RVCE_CS(enc->enc_pic.rc.lcvbrsatd_based_nonlinear_bit_budget_flag);
   RVCE_END();
}

static void config(struct rvce_encoder *enc)
{
   enc->task_info(enc, 0x00000002, 0, 0xffffffff, 0);
   enc->rate_control(enc);
   enc->config_extension(enc);
   enc->motion_estimation(enc);
   enc->rdo(enc);
   if (enc->use_vui)
      enc->vui(enc);
   enc->pic_control(enc);
}

static void config_extension(struct rvce_encoder *enc)
{
   RVCE_BEGIN(0x04000001); // config extension
   RVCE_CS(enc->enc_pic.ce.enc_enable_perf_logging);
   RVCE_END();
}

static void feedback(struct rvce_encoder *enc)
{
   RVCE_BEGIN(0x05000005);                                    // feedback buffer
   RVCE_WRITE(enc->fb->res->buf, enc->fb->res->domains, 0x0); // feedbackRingAddressHi/Lo
   RVCE_CS(enc->enc_pic.fb.feedback_ring_size);
   RVCE_END();
}

static void destroy(struct rvce_encoder *enc)
{
   enc->task_info(enc, 0x00000001, 0, 0, 0);

   feedback(enc);

   RVCE_BEGIN(0x02000001); // destroy
   RVCE_END();
}

static void motion_estimation(struct rvce_encoder *enc)
{
   RVCE_BEGIN(0x04000007); // motion estimation
   RVCE_CS(enc->enc_pic.me.enc_ime_decimation_search);
   RVCE_CS(enc->enc_pic.me.motion_est_half_pixel);
   RVCE_CS(enc->enc_pic.me.motion_est_quarter_pixel);
   RVCE_CS(enc->enc_pic.me.disable_favor_pmv_point);
   RVCE_CS(enc->enc_pic.me.force_zero_point_center);
   RVCE_CS(enc->enc_pic.me.lsmvert);
   RVCE_CS(enc->enc_pic.me.enc_search_range_x);
   RVCE_CS(enc->enc_pic.me.enc_search_range_y);
   RVCE_CS(enc->enc_pic.me.enc_search1_range_x);
   RVCE_CS(enc->enc_pic.me.enc_search1_range_y);
   RVCE_CS(enc->enc_pic.me.disable_16x16_frame1);
   RVCE_CS(enc->enc_pic.me.disable_satd);
   RVCE_CS(enc->enc_pic.me.enable_amd);
   RVCE_CS(enc->enc_pic.me.enc_disable_sub_mode);
   RVCE_CS(enc->enc_pic.me.enc_ime_skip_x);
   RVCE_CS(enc->enc_pic.me.enc_ime_skip_y);
   RVCE_CS(enc->enc_pic.me.enc_en_ime_overw_dis_subm);
   RVCE_CS(enc->enc_pic.me.enc_ime_overw_dis_subm_no);
   RVCE_CS(enc->enc_pic.me.enc_ime2_search_range_x);
   RVCE_CS(enc->enc_pic.me.enc_ime2_search_range_y);
   RVCE_CS(enc->enc_pic.me.parallel_mode_speedup_enable);
   RVCE_CS(enc->enc_pic.me.fme0_enc_disable_sub_mode);
   RVCE_CS(enc->enc_pic.me.fme1_enc_disable_sub_mode);
   RVCE_CS(enc->enc_pic.me.ime_sw_speedup_enable);
   RVCE_END();
}

static void pic_control(struct rvce_encoder *enc)
{
   RVCE_BEGIN(0x04000002); // pic control
   RVCE_CS(enc->enc_pic.pc.enc_use_constrained_intra_pred);
   RVCE_CS(enc->enc_pic.pc.enc_cabac_enable);
   RVCE_CS(enc->enc_pic.pc.enc_cabac_idc);
   RVCE_CS(enc->enc_pic.pc.enc_loop_filter_disable);
   RVCE_CS(enc->enc_pic.pc.enc_lf_beta_offset);
   RVCE_CS(enc->enc_pic.pc.enc_lf_alpha_c0_offset);
   RVCE_CS(enc->enc_pic.pc.enc_crop_left_offset);
   RVCE_CS(enc->enc_pic.pc.enc_crop_right_offset);
   RVCE_CS(enc->enc_pic.pc.enc_crop_top_offset);
   RVCE_CS(enc->enc_pic.pc.enc_crop_bottom_offset);
   RVCE_CS(enc->enc_pic.pc.enc_num_mbs_per_slice);
   RVCE_CS(enc->enc_pic.pc.enc_intra_refresh_num_mbs_per_slot);
   RVCE_CS(enc->enc_pic.pc.enc_force_intra_refresh);
   RVCE_CS(enc->enc_pic.pc.enc_force_imb_period);
   RVCE_CS(enc->enc_pic.pc.enc_pic_order_cnt_type);
   RVCE_CS(enc->enc_pic.pc.log2_max_pic_order_cnt_lsb_minus4);
   RVCE_CS(enc->enc_pic.pc.enc_sps_id);
   RVCE_CS(enc->enc_pic.pc.enc_pps_id);
   RVCE_CS(enc->enc_pic.pc.enc_constraint_set_flags);
   RVCE_CS(enc->enc_pic.pc.enc_b_pic_pattern);
   RVCE_CS(enc->enc_pic.pc.weight_pred_mode_b_picture);
   RVCE_CS(enc->enc_pic.pc.enc_number_of_reference_frames);
   RVCE_CS(enc->enc_pic.pc.enc_max_num_ref_frames);
   RVCE_CS(enc->enc_pic.pc.enc_num_default_active_ref_l0);
   RVCE_CS(enc->enc_pic.pc.enc_num_default_active_ref_l1);
   RVCE_CS(enc->enc_pic.pc.enc_slice_mode);
   RVCE_CS(enc->enc_pic.pc.enc_max_slice_size);
   RVCE_END();
}

static void rdo(struct rvce_encoder *enc)
{
   RVCE_BEGIN(0x04000008); // rdo
   RVCE_CS(enc->enc_pic.rdo.enc_disable_tbe_pred_i_frame);
   RVCE_CS(enc->enc_pic.rdo.enc_disable_tbe_pred_p_frame);
   RVCE_CS(enc->enc_pic.rdo.use_fme_interpol_y);
   RVCE_CS(enc->enc_pic.rdo.use_fme_interpol_uv);
   RVCE_CS(enc->enc_pic.rdo.use_fme_intrapol_y);
   RVCE_CS(enc->enc_pic.rdo.use_fme_intrapol_uv);
   RVCE_CS(enc->enc_pic.rdo.use_fme_interpol_y_1);
   RVCE_CS(enc->enc_pic.rdo.use_fme_interpol_uv_1);
   RVCE_CS(enc->enc_pic.rdo.use_fme_intrapol_y_1);
   RVCE_CS(enc->enc_pic.rdo.use_fme_intrapol_uv_1);
   RVCE_CS(enc->enc_pic.rdo.enc_16x16_cost_adj);
   RVCE_CS(enc->enc_pic.rdo.enc_skip_cost_adj);
   RVCE_CS(enc->enc_pic.rdo.enc_force_16x16_skip);
   RVCE_CS(enc->enc_pic.rdo.enc_disable_threshold_calc_a);
   RVCE_CS(enc->enc_pic.rdo.enc_luma_coeff_cost);
   RVCE_CS(enc->enc_pic.rdo.enc_luma_mb_coeff_cost);
   RVCE_CS(enc->enc_pic.rdo.enc_chroma_coeff_cost);
   RVCE_END();
}

static void session(struct rvce_encoder *enc)
{
   RVCE_BEGIN(0x00000001); // session cmd
   RVCE_CS(enc->stream_handle);
   RVCE_END();
}

static void task_info(struct rvce_encoder *enc, uint32_t op, uint32_t dep, uint32_t fb_idx,
                      uint32_t ring_idx)
{
   RVCE_BEGIN(0x00000002); // task info
   if (op == 0x3) {
      if (enc->task_info_idx) {
         uint32_t offs = enc->cs.current.cdw - enc->task_info_idx + 3;
         // Update offsetOfNextTaskInfo
         enc->cs.current.buf[enc->task_info_idx] = offs;
      }
      enc->task_info_idx = enc->cs.current.cdw;
   }
   enc->enc_pic.ti.task_operation = op;
   enc->enc_pic.ti.reference_picture_dependency = dep;
   enc->enc_pic.ti.feedback_index = fb_idx;
   enc->enc_pic.ti.video_bitstream_ring_index = ring_idx;
   RVCE_CS(enc->enc_pic.ti.offset_of_next_task_info);
   RVCE_CS(enc->enc_pic.ti.task_operation);
   RVCE_CS(enc->enc_pic.ti.reference_picture_dependency);
   RVCE_CS(enc->enc_pic.ti.collocate_flag_dependency);
   RVCE_CS(enc->enc_pic.ti.feedback_index);
   RVCE_CS(enc->enc_pic.ti.video_bitstream_ring_index);
   RVCE_END();
}

static void vui(struct rvce_encoder *enc)
{
   int i;

   if (!enc->enc_pic.enable_vui)
      return;

   RVCE_BEGIN(0x04000009); // vui
   RVCE_CS(enc->enc_pic.vui.aspect_ratio_info_present_flag);
   RVCE_CS(enc->enc_pic.vui.aspect_ratio_idc);
   RVCE_CS(enc->enc_pic.vui.sar_width);
   RVCE_CS(enc->enc_pic.vui.sar_height);
   RVCE_CS(enc->enc_pic.vui.overscan_info_present_flag);
   RVCE_CS(enc->enc_pic.vui.overscan_Approp_flag);
   RVCE_CS(enc->enc_pic.vui.video_signal_type_present_flag);
   RVCE_CS(enc->enc_pic.vui.video_format);
   RVCE_CS(enc->enc_pic.vui.video_full_range_flag);
   RVCE_CS(enc->enc_pic.vui.color_description_present_flag);
   RVCE_CS(enc->enc_pic.vui.color_prim);
   RVCE_CS(enc->enc_pic.vui.transfer_char);
   RVCE_CS(enc->enc_pic.vui.matrix_coef);
   RVCE_CS(enc->enc_pic.vui.chroma_loc_info_present_flag);
   RVCE_CS(enc->enc_pic.vui.chroma_loc_top);
   RVCE_CS(enc->enc_pic.vui.chroma_loc_bottom);
   RVCE_CS(enc->enc_pic.vui.timing_info_present_flag);
   RVCE_CS(enc->enc_pic.vui.num_units_in_tick);
   RVCE_CS(enc->enc_pic.vui.time_scale);
   RVCE_CS(enc->enc_pic.vui.fixed_frame_rate_flag);
   RVCE_CS(enc->enc_pic.vui.nal_hrd_parameters_present_flag);
   RVCE_CS(enc->enc_pic.vui.cpb_cnt_minus1);
   RVCE_CS(enc->enc_pic.vui.bit_rate_scale);
   RVCE_CS(enc->enc_pic.vui.cpb_size_scale);
   for (i = 0; i < 32; i++) {
      RVCE_CS(enc->enc_pic.vui.bit_rate_value_minus);
      RVCE_CS(enc->enc_pic.vui.cpb_size_value_minus);
      RVCE_CS(enc->enc_pic.vui.cbr_flag);
   }
   RVCE_CS(enc->enc_pic.vui.initial_cpb_removal_delay_length_minus1);
   RVCE_CS(enc->enc_pic.vui.cpb_removal_delay_length_minus1);
   RVCE_CS(enc->enc_pic.vui.dpb_output_delay_length_minus1);
   RVCE_CS(enc->enc_pic.vui.time_offset_length);
   RVCE_CS(enc->enc_pic.vui.low_delay_hrd_flag);
   RVCE_CS(enc->enc_pic.vui.pic_struct_present_flag);
   RVCE_CS(enc->enc_pic.vui.bitstream_restriction_present_flag);
   RVCE_CS(enc->enc_pic.vui.motion_vectors_over_pic_boundaries_flag);
   RVCE_CS(enc->enc_pic.vui.max_bytes_per_pic_denom);
   RVCE_CS(enc->enc_pic.vui.max_bits_per_mb_denom);
   RVCE_CS(enc->enc_pic.vui.log2_max_mv_length_hori);
   RVCE_CS(enc->enc_pic.vui.log2_max_mv_length_vert);
   RVCE_CS(enc->enc_pic.vui.num_reorder_frames);
   RVCE_CS(enc->enc_pic.vui.max_dec_frame_buffering);
   RVCE_END();
}

void si_vce_52_init(struct rvce_encoder *enc)
{
   enc->session = session;
   enc->task_info = task_info;
   enc->create = create;
   enc->feedback = feedback;
   enc->rate_control = rate_control;
   enc->config_extension = config_extension;
   enc->pic_control = pic_control;
   enc->motion_estimation = motion_estimation;
   enc->rdo = rdo;
   enc->vui = vui;
   enc->config = config;
   enc->encode = encode;
   enc->destroy = destroy;
   enc->si_get_pic_param = si_vce_52_get_param;
}
