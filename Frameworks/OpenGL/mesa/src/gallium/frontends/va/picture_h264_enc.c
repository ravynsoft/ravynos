/**************************************************************************
 *
 * Copyright 2018 Advanced Micro Devices, Inc.
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

#include "util/u_handle_table.h"
#include "util/u_video.h"
#include "va_private.h"

#include "util/vl_rbsp.h"

enum H264NALUnitType {
    H264_NAL_SPS        = 7,
    H264_NAL_PPS        = 8,
};

VAStatus
vlVaHandleVAEncPictureParameterBufferTypeH264(vlVaDriver *drv, vlVaContext *context, vlVaBuffer *buf)
{
   VAEncPictureParameterBufferH264 *h264;
   vlVaBuffer *coded_buf;

   h264 = buf->data;
   if (h264->pic_fields.bits.idr_pic_flag == 1)
      context->desc.h264enc.frame_num = 0;
   context->desc.h264enc.not_referenced = !h264->pic_fields.bits.reference_pic_flag;
   context->desc.h264enc.pic_order_cnt = h264->CurrPic.TopFieldOrderCnt;
   context->desc.h264enc.is_ltr = h264->CurrPic.flags & VA_PICTURE_H264_LONG_TERM_REFERENCE;
   if (context->desc.h264enc.is_ltr)
      context->desc.h264enc.ltr_index = h264->CurrPic.frame_idx;
   if (context->desc.h264enc.gop_cnt == 0)
      context->desc.h264enc.i_remain = context->gop_coeff;
   else if (context->desc.h264enc.frame_num == 1)
      context->desc.h264enc.i_remain--;

   context->desc.h264enc.p_remain = context->desc.h264enc.gop_size - context->desc.h264enc.gop_cnt - context->desc.h264enc.i_remain;

   coded_buf = handle_table_get(drv->htab, h264->coded_buf);
   if (!coded_buf)
      return VA_STATUS_ERROR_INVALID_BUFFER;

   if (!coded_buf->derived_surface.resource)
      coded_buf->derived_surface.resource = pipe_buffer_create(drv->pipe->screen, PIPE_BIND_VERTEX_BUFFER,
                                            PIPE_USAGE_STAGING, coded_buf->size);
   context->coded_buf = coded_buf;

   if (context->desc.h264enc.is_ltr)
      _mesa_hash_table_insert(context->desc.h264enc.frame_idx,
		       UINT_TO_PTR(h264->CurrPic.picture_id + 1),
		       UINT_TO_PTR(context->desc.h264enc.ltr_index));
   else
      _mesa_hash_table_insert(context->desc.h264enc.frame_idx,
		       UINT_TO_PTR(h264->CurrPic.picture_id + 1),
		       UINT_TO_PTR(context->desc.h264enc.frame_num));

   if (h264->pic_fields.bits.idr_pic_flag == 1)
      context->desc.h264enc.picture_type = PIPE_H2645_ENC_PICTURE_TYPE_IDR;
   else
      context->desc.h264enc.picture_type = PIPE_H2645_ENC_PICTURE_TYPE_P;

   /* Initialize slice descriptors for this picture */
   context->desc.h264enc.num_slice_descriptors = 0;
   memset(&context->desc.h264enc.slices_descriptors, 0, sizeof(context->desc.h264enc.slices_descriptors));

   context->desc.h264enc.quant_i_frames = h264->pic_init_qp;
   context->desc.h264enc.quant_b_frames = h264->pic_init_qp;
   context->desc.h264enc.quant_p_frames = h264->pic_init_qp;
   context->desc.h264enc.gop_cnt++;
   if (context->desc.h264enc.gop_cnt == context->desc.h264enc.gop_size)
      context->desc.h264enc.gop_cnt = 0;

   context->desc.h264enc.pic_ctrl.enc_cabac_enable = h264->pic_fields.bits.entropy_coding_mode_flag;
   context->desc.h264enc.num_ref_idx_l0_active_minus1 = h264->num_ref_idx_l0_active_minus1;
   context->desc.h264enc.num_ref_idx_l1_active_minus1 = h264->num_ref_idx_l1_active_minus1;
   context->desc.h264enc.pic_ctrl.deblocking_filter_control_present_flag
      = h264->pic_fields.bits.deblocking_filter_control_present_flag;
   context->desc.h264enc.pic_ctrl.redundant_pic_cnt_present_flag
      = h264->pic_fields.bits.redundant_pic_cnt_present_flag;
   context->desc.h264enc.pic_ctrl.chroma_qp_index_offset = h264->chroma_qp_index_offset;
   context->desc.h264enc.pic_ctrl.second_chroma_qp_index_offset
      = h264->second_chroma_qp_index_offset;

   return VA_STATUS_SUCCESS;
}

VAStatus
vlVaHandleVAEncSliceParameterBufferTypeH264(vlVaDriver *drv, vlVaContext *context, vlVaBuffer *buf)
{
   VAEncSliceParameterBufferH264 *h264;

   h264 = buf->data;
   memset(&context->desc.h264enc.ref_idx_l0_list, VA_INVALID_ID, sizeof(context->desc.h264enc.ref_idx_l0_list));
   memset(&context->desc.h264enc.ref_idx_l1_list, VA_INVALID_ID, sizeof(context->desc.h264enc.ref_idx_l1_list));

   if(h264->num_ref_idx_active_override_flag) {
      context->desc.h264enc.num_ref_idx_l0_active_minus1 = h264->num_ref_idx_l0_active_minus1;
      context->desc.h264enc.num_ref_idx_l1_active_minus1 = h264->num_ref_idx_l1_active_minus1;
   }

   for (int i = 0; i < 32; i++) {
      if (h264->RefPicList0[i].picture_id != VA_INVALID_ID) {
               context->desc.h264enc.ref_idx_l0_list[i] = PTR_TO_UINT(util_hash_table_get(context->desc.h264enc.frame_idx,
                                 UINT_TO_PTR(h264->RefPicList0[i].picture_id + 1)));
               context->desc.h264enc.l0_is_long_term[i] = h264->RefPicList0[i].flags &
		       					  VA_PICTURE_H264_LONG_TERM_REFERENCE;
      }
      if (h264->RefPicList1[i].picture_id != VA_INVALID_ID && h264->slice_type == 1) {
            context->desc.h264enc.ref_idx_l1_list[i] = PTR_TO_UINT(util_hash_table_get(context->desc.h264enc.frame_idx,
               			 UINT_TO_PTR(h264->RefPicList1[i].picture_id + 1)));
            context->desc.h264enc.l1_is_long_term[i] = h264->RefPicList1[i].flags &
		    				       VA_PICTURE_H264_LONG_TERM_REFERENCE;
      }
   }

   /**
    *  VAEncSliceParameterBufferH264.slice_type
    *  Slice type.
    *  Range: 0..2, 5..7, i.e. no switching slices.
   */
   struct h264_slice_descriptor slice_descriptor;
   memset(&slice_descriptor, 0, sizeof(slice_descriptor));
   slice_descriptor.macroblock_address = h264->macroblock_address;
   slice_descriptor.num_macroblocks = h264->num_macroblocks;

   if ((h264->slice_type == 1) || (h264->slice_type == 6)) {
      context->desc.h264enc.picture_type = PIPE_H2645_ENC_PICTURE_TYPE_B;
      slice_descriptor.slice_type = PIPE_H264_SLICE_TYPE_B;
   } else if ((h264->slice_type == 0) || (h264->slice_type == 5)) {
      context->desc.h264enc.picture_type = PIPE_H2645_ENC_PICTURE_TYPE_P;
      slice_descriptor.slice_type = PIPE_H264_SLICE_TYPE_P;
   } else if ((h264->slice_type == 2) || (h264->slice_type == 7)) {
      if (context->desc.h264enc.picture_type == PIPE_H2645_ENC_PICTURE_TYPE_IDR) {
         if (slice_descriptor.macroblock_address == 0) {
            /* Increment it only for the first slice of the IDR frame */
            context->desc.h264enc.idr_pic_id++;
         }
         slice_descriptor.slice_type = PIPE_H264_SLICE_TYPE_I;
      } else {
         context->desc.h264enc.picture_type = PIPE_H2645_ENC_PICTURE_TYPE_I;
         slice_descriptor.slice_type = PIPE_H264_SLICE_TYPE_I;
      }
   } else {
      context->desc.h264enc.picture_type = PIPE_H2645_ENC_PICTURE_TYPE_SKIP;
   }

   context->desc.h264enc.pic_ctrl.enc_cabac_init_idc = h264->cabac_init_idc;
   context->desc.h264enc.dbk.disable_deblocking_filter_idc = h264->disable_deblocking_filter_idc;
   context->desc.h264enc.dbk.alpha_c0_offset_div2 = h264->slice_alpha_c0_offset_div2;
   context->desc.h264enc.dbk.beta_offset_div2 = h264->slice_beta_offset_div2;

   /* Handle the slice control parameters */
   if (context->desc.h264enc.num_slice_descriptors < ARRAY_SIZE(context->desc.h264enc.slices_descriptors)) {
      context->desc.h264enc.slices_descriptors[context->desc.h264enc.num_slice_descriptors++] = slice_descriptor;
   } else {
      return VA_STATUS_ERROR_NOT_ENOUGH_BUFFER;
   }

   return VA_STATUS_SUCCESS;
}

VAStatus
vlVaHandleVAEncSequenceParameterBufferTypeH264(vlVaDriver *drv, vlVaContext *context, vlVaBuffer *buf)
{
   uint32_t num_units_in_tick = 0, time_scale  = 0;

   VAEncSequenceParameterBufferH264 *h264 = (VAEncSequenceParameterBufferH264 *)buf->data;
   if (!context->decoder) {
      context->templat.max_references = h264->max_num_ref_frames;
      context->templat.level = h264->level_idc;
      context->decoder = drv->pipe->create_video_codec(drv->pipe, &context->templat);
      if (!context->decoder)
         return VA_STATUS_ERROR_ALLOCATION_FAILED;

      getEncParamPresetH264(context);
      context->desc.h264enc.rate_ctrl[0].vbv_buffer_size = 20000000;
      context->desc.h264enc.rate_ctrl[0].vbv_buf_lv = 48;
      context->desc.h264enc.rate_ctrl[0].fill_data_enable = 1;
      context->desc.h264enc.rate_ctrl[0].enforce_hrd = 1;
      context->desc.h264enc.rate_ctrl[0].max_qp = 51;
      context->desc.h264enc.rate_ctrl[0].min_qp = 0;
      context->desc.h264enc.enable_vui = false;
      context->desc.h264enc.intra_refresh.mode = INTRA_REFRESH_MODE_NONE;
      context->desc.h264enc.intra_refresh.offset = 0;
      context->desc.h264enc.intra_refresh.region_size = 0;
      context->desc.h264enc.intra_refresh.need_sequence_header = 0;
   }

   context->desc.h264enc.ip_period = h264->ip_period;
   context->desc.h264enc.intra_idr_period =
      h264->intra_idr_period != 0 ? h264->intra_idr_period : PIPE_DEFAULT_INTRA_IDR_PERIOD;
   context->gop_coeff = ((1024 + context->desc.h264enc.intra_idr_period - 1) /
                        context->desc.h264enc.intra_idr_period + 1) / 2 * 2;
   if (context->gop_coeff > VL_VA_ENC_GOP_COEFF)
      context->gop_coeff = VL_VA_ENC_GOP_COEFF;
   context->desc.h264enc.gop_size = context->desc.h264enc.intra_idr_period * context->gop_coeff;
   context->desc.h264enc.seq.pic_order_cnt_type = h264->seq_fields.bits.pic_order_cnt_type;
   context->desc.h264enc.seq.log2_max_frame_num_minus4 = h264->seq_fields.bits.log2_max_frame_num_minus4;
   context->desc.h264enc.seq.log2_max_pic_order_cnt_lsb_minus4 = h264->seq_fields.bits.log2_max_pic_order_cnt_lsb_minus4;
   context->desc.h264enc.seq.vui_parameters_present_flag = h264->vui_parameters_present_flag;
   if (h264->vui_parameters_present_flag) {
      context->desc.h264enc.seq.vui_flags.aspect_ratio_info_present_flag =
         h264->vui_fields.bits.aspect_ratio_info_present_flag;
      context->desc.h264enc.seq.aspect_ratio_idc = h264->aspect_ratio_idc;
      context->desc.h264enc.seq.sar_width = h264->sar_width;
      context->desc.h264enc.seq.sar_height = h264->sar_height;
      context->desc.h264enc.seq.vui_flags.timing_info_present_flag =
         h264->vui_fields.bits.timing_info_present_flag;
      num_units_in_tick = h264->num_units_in_tick;
      time_scale = h264->time_scale;
      context->desc.h264enc.seq.vui_flags.fixed_frame_rate_flag =
         h264->vui_fields.bits.fixed_frame_rate_flag;
      context->desc.h264enc.seq.vui_flags.low_delay_hrd_flag =
         h264->vui_fields.bits.low_delay_hrd_flag;
      context->desc.h264enc.seq.vui_flags.bitstream_restriction_flag =
         h264->vui_fields.bits.bitstream_restriction_flag;
      context->desc.h264enc.seq.vui_flags.motion_vectors_over_pic_boundaries_flag =
         h264->vui_fields.bits.motion_vectors_over_pic_boundaries_flag;
      context->desc.h264enc.seq.log2_max_mv_length_vertical =
            h264->vui_fields.bits.log2_max_mv_length_vertical;
      context->desc.h264enc.seq.log2_max_mv_length_horizontal =
            h264->vui_fields.bits.log2_max_mv_length_horizontal;
   } else {
      context->desc.h264enc.seq.vui_flags.timing_info_present_flag = 0;
      context->desc.h264enc.seq.vui_flags.fixed_frame_rate_flag = 0;
      context->desc.h264enc.seq.vui_flags.low_delay_hrd_flag = 0;
      context->desc.h264enc.seq.vui_flags.bitstream_restriction_flag = 0;
      context->desc.h264enc.seq.vui_flags.motion_vectors_over_pic_boundaries_flag = 0;
      context->desc.h264enc.seq.log2_max_mv_length_vertical = 0;
      context->desc.h264enc.seq.log2_max_mv_length_horizontal = 0;
   }

   if (!context->desc.h264enc.seq.vui_flags.timing_info_present_flag) {
      /* if not present, set default value */
      num_units_in_tick = PIPE_DEFAULT_FRAME_RATE_DEN;
      time_scale = PIPE_DEFAULT_FRAME_RATE_NUM * 2;
   }

   context->desc.h264enc.seq.num_units_in_tick = num_units_in_tick;
   context->desc.h264enc.seq.time_scale = time_scale;
   context->desc.h264enc.rate_ctrl[0].frame_rate_num = time_scale / 2;
   context->desc.h264enc.rate_ctrl[0].frame_rate_den = num_units_in_tick;

   if (h264->frame_cropping_flag) {
      context->desc.h264enc.seq.enc_frame_cropping_flag = h264->frame_cropping_flag;
      context->desc.h264enc.seq.enc_frame_crop_left_offset = h264->frame_crop_left_offset;
      context->desc.h264enc.seq.enc_frame_crop_right_offset = h264->frame_crop_right_offset;
      context->desc.h264enc.seq.enc_frame_crop_top_offset = h264->frame_crop_top_offset;
      context->desc.h264enc.seq.enc_frame_crop_bottom_offset = h264->frame_crop_bottom_offset;
   }
   return VA_STATUS_SUCCESS;
}

VAStatus
vlVaHandleVAEncMiscParameterTypeRateControlH264(vlVaContext *context, VAEncMiscParameterBuffer *misc)
{
   unsigned temporal_id;
   VAEncMiscParameterRateControl *rc = (VAEncMiscParameterRateControl *)misc->data;

   temporal_id = context->desc.h264enc.rate_ctrl[0].rate_ctrl_method !=
                 PIPE_H2645_ENC_RATE_CONTROL_METHOD_DISABLE ?
                 rc->rc_flags.bits.temporal_id :
                 0;

   if (context->desc.h264enc.rate_ctrl[0].rate_ctrl_method ==
       PIPE_H2645_ENC_RATE_CONTROL_METHOD_CONSTANT)
      context->desc.h264enc.rate_ctrl[temporal_id].target_bitrate =
         rc->bits_per_second;
   else
      context->desc.h264enc.rate_ctrl[temporal_id].target_bitrate =
         rc->bits_per_second * (rc->target_percentage / 100.0);

   if (context->desc.h264enc.seq.num_temporal_layers > 0 &&
       temporal_id >= context->desc.h264enc.seq.num_temporal_layers)
      return VA_STATUS_ERROR_INVALID_PARAMETER;

   context->desc.h264enc.rate_ctrl[temporal_id].fill_data_enable = !(rc->rc_flags.bits.disable_bit_stuffing);
   /* context->desc.h264enc.rate_ctrl[temporal_id].skip_frame_enable = !(rc->rc_flags.bits.disable_frame_skip); */
   context->desc.h264enc.rate_ctrl[temporal_id].skip_frame_enable = 0;
   context->desc.h264enc.rate_ctrl[temporal_id].peak_bitrate = rc->bits_per_second;

   if ((context->desc.h264enc.rate_ctrl[0].rate_ctrl_method == PIPE_H2645_ENC_RATE_CONTROL_METHOD_CONSTANT) ||
       (context->desc.h264enc.rate_ctrl[0].rate_ctrl_method == PIPE_H2645_ENC_RATE_CONTROL_METHOD_CONSTANT_SKIP))
      context->desc.h264enc.rate_ctrl[temporal_id].vbv_buffer_size =
         context->desc.h264enc.rate_ctrl[temporal_id].target_bitrate;
   else if (context->desc.h264enc.rate_ctrl[temporal_id].target_bitrate < 2000000)
      context->desc.h264enc.rate_ctrl[temporal_id].vbv_buffer_size =
         MIN2((context->desc.h264enc.rate_ctrl[0].target_bitrate * 2.75), 2000000);
   else
      context->desc.h264enc.rate_ctrl[temporal_id].vbv_buffer_size =
         context->desc.h264enc.rate_ctrl[temporal_id].target_bitrate;

   context->desc.h264enc.rate_ctrl[temporal_id].max_qp = rc->max_qp;
   context->desc.h264enc.rate_ctrl[temporal_id].min_qp = rc->min_qp;
   /* Distinguishes from the default params set for these values in other
      functions and app specific params passed down */
   context->desc.h264enc.rate_ctrl[temporal_id].app_requested_qp_range = ((rc->max_qp > 0) || (rc->min_qp > 0));

   if (context->desc.h264enc.rate_ctrl[0].rate_ctrl_method ==
       PIPE_H2645_ENC_RATE_CONTROL_METHOD_QUALITY_VARIABLE)
      context->desc.h264enc.rate_ctrl[temporal_id].vbr_quality_factor =
         rc->quality_factor;

   return VA_STATUS_SUCCESS;
}

VAStatus
vlVaHandleVAEncMiscParameterTypeFrameRateH264(vlVaContext *context, VAEncMiscParameterBuffer *misc)
{
   unsigned temporal_id;
   VAEncMiscParameterFrameRate *fr = (VAEncMiscParameterFrameRate *)misc->data;

   temporal_id = context->desc.h264enc.rate_ctrl[0].rate_ctrl_method !=
                 PIPE_H2645_ENC_RATE_CONTROL_METHOD_DISABLE ?
                 fr->framerate_flags.bits.temporal_id :
                 0;

   if (context->desc.h264enc.seq.num_temporal_layers > 0 &&
       temporal_id >= context->desc.h264enc.seq.num_temporal_layers)
      return VA_STATUS_ERROR_INVALID_PARAMETER;

   if (fr->framerate & 0xffff0000) {
      context->desc.h264enc.rate_ctrl[temporal_id].frame_rate_num = fr->framerate       & 0xffff;
      context->desc.h264enc.rate_ctrl[temporal_id].frame_rate_den = fr->framerate >> 16 & 0xffff;
   } else {
      context->desc.h264enc.rate_ctrl[temporal_id].frame_rate_num = fr->framerate;
      context->desc.h264enc.rate_ctrl[temporal_id].frame_rate_den = 1;
   }

   return VA_STATUS_SUCCESS;
}

static void parseEncHrdParamsH264(struct vl_rbsp *rbsp, pipe_h264_enc_hrd_params* hrd_params)
{
   unsigned i;

   hrd_params->cpb_cnt_minus1 = vl_rbsp_ue(rbsp);
   hrd_params->bit_rate_scale = vl_rbsp_u(rbsp, 4);
   hrd_params->cpb_size_scale = vl_rbsp_u(rbsp, 4);
   for (i = 0; i <= hrd_params->cpb_cnt_minus1; ++i) {
      hrd_params->bit_rate_value_minus1[i] = vl_rbsp_ue(rbsp);
      hrd_params->cpb_size_value_minus1[i] = vl_rbsp_ue(rbsp);
      hrd_params->cbr_flag[i] = vl_rbsp_u(rbsp, 1);
   }
   hrd_params->initial_cpb_removal_delay_length_minus1 = vl_rbsp_u(rbsp, 5);
   hrd_params->cpb_removal_delay_length_minus1 = vl_rbsp_u(rbsp, 5);
   hrd_params->dpb_output_delay_length_minus1 = vl_rbsp_u(rbsp, 5);
   hrd_params->time_offset_length = vl_rbsp_u(rbsp, 5);
}

static void parseEncSpsParamsH264(vlVaContext *context, struct vl_rbsp *rbsp)
{
   unsigned i, profile_idc, num_ref_frames_in_pic_order_cnt_cycle;

   profile_idc = vl_rbsp_u(rbsp, 8);

   context->desc.h264enc.seq.enc_constraint_set_flags =
      vl_rbsp_u(rbsp, 6); /* constraint_set_flags */
   vl_rbsp_u(rbsp, 2); /* reserved_zero_2bits */
   vl_rbsp_u(rbsp, 8); /* level_idc */

   vl_rbsp_ue(rbsp); /* seq_parameter_set_id */

   if (profile_idc == 100 || profile_idc == 110 ||
       profile_idc == 122 || profile_idc == 244 || profile_idc == 44 ||
       profile_idc == 83 || profile_idc == 86 || profile_idc == 118 ||
       profile_idc == 128 || profile_idc == 138 || profile_idc == 139 ||
       profile_idc == 134 || profile_idc == 135) {

      if (vl_rbsp_ue(rbsp) == 3) /* chroma_format_idc */
         vl_rbsp_u(rbsp, 1); /* separate_colour_plane_flag */

      vl_rbsp_ue(rbsp); /* bit_depth_luma_minus8 */
      vl_rbsp_ue(rbsp); /* bit_depth_chroma_minus8 */
      vl_rbsp_u(rbsp, 1); /* qpprime_y_zero_transform_bypass_flag */

      if (vl_rbsp_u(rbsp, 1)) /* seq_scaling_matrix_present_flag */
         return; /* TODO */
   }

   context->desc.h264enc.seq.log2_max_frame_num_minus4 = vl_rbsp_ue(rbsp); /* log2_max_frame_num_minus4 */
   context->desc.h264enc.seq.pic_order_cnt_type = vl_rbsp_ue(rbsp); /* pic_order_cnt_type */

   if (context->desc.h264enc.seq.pic_order_cnt_type == 0)
      context->desc.h264enc.seq.log2_max_pic_order_cnt_lsb_minus4 = vl_rbsp_ue(rbsp); /* log2_max_pic_order_cnt_lsb_minus4 */
   else if (context->desc.h264enc.seq.pic_order_cnt_type == 1) {
      vl_rbsp_u(rbsp, 1); /* delta_pic_order_always_zero_flag */
      vl_rbsp_se(rbsp); /* offset_for_non_ref_pic */
      vl_rbsp_se(rbsp); /* offset_for_top_to_bottom_field */
      num_ref_frames_in_pic_order_cnt_cycle = vl_rbsp_ue(rbsp);
      for (i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; ++i)
         vl_rbsp_se(rbsp); /* offset_for_ref_frame[i] */
   }

   vl_rbsp_ue(rbsp); /* max_num_ref_frames */
   vl_rbsp_u(rbsp, 1); /* gaps_in_frame_num_value_allowed_flag */
   vl_rbsp_ue(rbsp); /* pic_width_in_mbs_minus1 */
   vl_rbsp_ue(rbsp); /* pic_height_in_map_units_minus1 */
   if (!vl_rbsp_u(rbsp, 1)) /* frame_mbs_only_flag */
      vl_rbsp_u(rbsp, 1); /* mb_adaptive_frame_field_flag */

   vl_rbsp_u(rbsp, 1); /* direct_8x8_inference_flag */
   if (vl_rbsp_u(rbsp, 1)) { /* frame_cropping_flag */
      vl_rbsp_ue(rbsp); /* frame_crop_left_offset */
      vl_rbsp_ue(rbsp); /* frame_crop_right_offset */
      vl_rbsp_ue(rbsp); /* frame_crop_top_offset */
      vl_rbsp_ue(rbsp); /* frame_crop_bottom_offset */
   }

   context->desc.h264enc.seq.vui_parameters_present_flag = vl_rbsp_u(rbsp, 1);
   if (context->desc.h264enc.seq.vui_parameters_present_flag) {
      context->desc.h264enc.seq.vui_flags.aspect_ratio_info_present_flag = vl_rbsp_u(rbsp, 1);
      if (context->desc.h264enc.seq.vui_flags.aspect_ratio_info_present_flag) {
         context->desc.h264enc.seq.aspect_ratio_idc = vl_rbsp_u(rbsp, 8);
         if (context->desc.h264enc.seq.aspect_ratio_idc == 255 /* Extended_SAR */) {
            context->desc.h264enc.seq.sar_width = vl_rbsp_u(rbsp, 16);
            context->desc.h264enc.seq.sar_height = vl_rbsp_u(rbsp, 16);
         }
      }

      context->desc.h264enc.seq.vui_flags.overscan_info_present_flag = vl_rbsp_u(rbsp, 1);
      if (context->desc.h264enc.seq.vui_flags.overscan_info_present_flag)
         context->desc.h264enc.seq.vui_flags.overscan_appropriate_flag = vl_rbsp_u(rbsp, 1);

      context->desc.h264enc.seq.vui_flags.video_signal_type_present_flag = vl_rbsp_u(rbsp, 1);
      if (context->desc.h264enc.seq.vui_flags.video_signal_type_present_flag) {
         context->desc.h264enc.seq.video_format = vl_rbsp_u(rbsp, 3);
         context->desc.h264enc.seq.video_full_range_flag = vl_rbsp_u(rbsp, 1);
         context->desc.h264enc.seq.vui_flags.colour_description_present_flag = vl_rbsp_u(rbsp, 1);
         if (context->desc.h264enc.seq.vui_flags.colour_description_present_flag) {
            context->desc.h264enc.seq.colour_primaries = vl_rbsp_u(rbsp, 8);
            context->desc.h264enc.seq.transfer_characteristics = vl_rbsp_u(rbsp, 8);
            context->desc.h264enc.seq.matrix_coefficients = vl_rbsp_u(rbsp, 8);
         }
      }

      context->desc.h264enc.seq.vui_flags.chroma_loc_info_present_flag = vl_rbsp_u(rbsp, 1);
      if (context->desc.h264enc.seq.vui_flags.chroma_loc_info_present_flag) {
         context->desc.h264enc.seq.chroma_sample_loc_type_top_field = vl_rbsp_ue(rbsp);
         context->desc.h264enc.seq.chroma_sample_loc_type_bottom_field = vl_rbsp_ue(rbsp);
      }

      context->desc.h264enc.seq.vui_flags.timing_info_present_flag = vl_rbsp_u(rbsp, 1);
      if (context->desc.h264enc.seq.vui_flags.timing_info_present_flag) {
         context->desc.h264enc.seq.num_units_in_tick = vl_rbsp_u(rbsp, 32);
         context->desc.h264enc.seq.time_scale = vl_rbsp_u(rbsp, 32);
         context->desc.h264enc.seq.vui_flags.fixed_frame_rate_flag = vl_rbsp_u(rbsp, 1);
      }

      context->desc.h264enc.seq.vui_flags.nal_hrd_parameters_present_flag = vl_rbsp_u(rbsp, 1);
      if (context->desc.h264enc.seq.vui_flags.nal_hrd_parameters_present_flag)
         parseEncHrdParamsH264(rbsp, &context->desc.h264enc.seq.nal_hrd_parameters);

      context->desc.h264enc.seq.vui_flags.vcl_hrd_parameters_present_flag = vl_rbsp_u(rbsp, 1);
      if (context->desc.h264enc.seq.vui_flags.vcl_hrd_parameters_present_flag)
         parseEncHrdParamsH264(rbsp, &context->desc.h264enc.seq.vcl_hrd_parameters);

      if (context->desc.h264enc.seq.vui_flags.nal_hrd_parameters_present_flag ||
          context->desc.h264enc.seq.vui_flags.vcl_hrd_parameters_present_flag)
         context->desc.h264enc.seq.vui_flags.low_delay_hrd_flag = vl_rbsp_u(rbsp, 1);

      context->desc.h264enc.seq.vui_flags.pic_struct_present_flag = vl_rbsp_u(rbsp, 1);

      context->desc.h264enc.seq.vui_flags.bitstream_restriction_flag = vl_rbsp_u(rbsp, 1);
      if (context->desc.h264enc.seq.vui_flags.bitstream_restriction_flag) {
         context->desc.h264enc.seq.vui_flags.motion_vectors_over_pic_boundaries_flag = vl_rbsp_u(rbsp, 1);
         context->desc.h264enc.seq.max_bytes_per_pic_denom = vl_rbsp_ue(rbsp);
         context->desc.h264enc.seq.max_bits_per_mb_denom = vl_rbsp_ue(rbsp);
         context->desc.h264enc.seq.log2_max_mv_length_horizontal = vl_rbsp_ue(rbsp);
         context->desc.h264enc.seq.log2_max_mv_length_vertical = vl_rbsp_ue(rbsp);
         context->desc.h264enc.seq.max_num_reorder_frames = vl_rbsp_ue(rbsp);
         context->desc.h264enc.seq.max_dec_frame_buffering = vl_rbsp_ue(rbsp);
      }
   }
}

VAStatus
vlVaHandleVAEncPackedHeaderDataBufferTypeH264(vlVaContext *context, vlVaBuffer *buf)
{
   struct vl_vlc vlc = {0};
   vl_vlc_init(&vlc, 1, (const void * const*)&buf->data, &buf->size);

   while (vl_vlc_bits_left(&vlc) > 0) {
      /* search the first 64 bytes for a startcode */
      for (int i = 0; i < 64 && vl_vlc_bits_left(&vlc) >= 24; ++i) {
         if (vl_vlc_peekbits(&vlc, 24) == 0x000001)
            break;
         vl_vlc_eatbits(&vlc, 8);
         vl_vlc_fillbits(&vlc);
      }
      vl_vlc_eatbits(&vlc, 24); /* eat the startcode */

      if (vl_vlc_valid_bits(&vlc) < 15)
         vl_vlc_fillbits(&vlc);

      vl_vlc_eatbits(&vlc, 3);
      unsigned nal_unit_type = vl_vlc_get_uimsbf(&vlc, 5);

      struct vl_rbsp rbsp;
      vl_rbsp_init(&rbsp, &vlc, ~0, context->packed_header_emulation_bytes);

      switch(nal_unit_type) {
      case H264_NAL_SPS:
         parseEncSpsParamsH264(context, &rbsp);
         break;
      case H264_NAL_PPS:
      default:
         break;
      }

      if (!context->packed_header_emulation_bytes)
         break;
   }

   return VA_STATUS_SUCCESS;
}

VAStatus
vlVaHandleVAEncMiscParameterTypeTemporalLayerH264(vlVaContext *context, VAEncMiscParameterBuffer *misc)
{
   VAEncMiscParameterTemporalLayerStructure *tl = (VAEncMiscParameterTemporalLayerStructure *)misc->data;

   context->desc.h264enc.seq.num_temporal_layers = tl->number_of_layers;

   return VA_STATUS_SUCCESS;
}

VAStatus
vlVaHandleVAEncMiscParameterTypeQualityLevelH264(vlVaContext *context, VAEncMiscParameterBuffer *misc)
{
   VAEncMiscParameterBufferQualityLevel *ql = (VAEncMiscParameterBufferQualityLevel *)misc->data;
   vlVaHandleVAEncMiscParameterTypeQualityLevel(&context->desc.h264enc.quality_modes,
                               (vlVaQualityBits *)&ql->quality_level);

   return VA_STATUS_SUCCESS;
}

VAStatus
vlVaHandleVAEncMiscParameterTypeMaxFrameSizeH264(vlVaContext *context, VAEncMiscParameterBuffer *misc)
{
   VAEncMiscParameterBufferMaxFrameSize *ms = (VAEncMiscParameterBufferMaxFrameSize *)misc->data;
   context->desc.h264enc.rate_ctrl[0].max_au_size = ms->max_frame_size;
   return VA_STATUS_SUCCESS;
}

VAStatus
vlVaHandleVAEncMiscParameterTypeHRDH264(vlVaContext *context, VAEncMiscParameterBuffer *misc)
{
   VAEncMiscParameterHRD *ms = (VAEncMiscParameterHRD *)misc->data;

   if (ms->buffer_size) {
      context->desc.h264enc.rate_ctrl[0].vbv_buffer_size = ms->buffer_size;
      context->desc.h264enc.rate_ctrl[0].vbv_buf_lv = (ms->initial_buffer_fullness << 6 ) / ms->buffer_size;
      context->desc.h264enc.rate_ctrl[0].vbv_buf_initial_size = ms->initial_buffer_fullness;
      /* Distinguishes from the default params set for these values in other
      functions and app specific params passed down via HRD buffer */
      context->desc.h264enc.rate_ctrl[0].app_requested_hrd_buffer = true;
   }

   return VA_STATUS_SUCCESS;
}

void getEncParamPresetH264(vlVaContext *context)
{
   //rate control
   if (context->desc.h264enc.rate_ctrl[0].frame_rate_num == 0 ||
       context->desc.h264enc.rate_ctrl[0].frame_rate_den == 0) {
         context->desc.h264enc.rate_ctrl[0].frame_rate_num = 30;
         context->desc.h264enc.rate_ctrl[0].frame_rate_den = 1;
   }
   context->desc.h264enc.rate_ctrl[0].target_bits_picture =
      context->desc.h264enc.rate_ctrl[0].target_bitrate *
      ((float)context->desc.h264enc.rate_ctrl[0].frame_rate_den /
      context->desc.h264enc.rate_ctrl[0].frame_rate_num);
   context->desc.h264enc.rate_ctrl[0].peak_bits_picture_integer =
      context->desc.h264enc.rate_ctrl[0].peak_bitrate *
      ((float)context->desc.h264enc.rate_ctrl[0].frame_rate_den /
      context->desc.h264enc.rate_ctrl[0].frame_rate_num);

   context->desc.h264enc.rate_ctrl[0].peak_bits_picture_fraction = 0;
}
