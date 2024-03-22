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

enum HEVCNALUnitType {
    HEVC_NAL_VPS        = 32,
    HEVC_NAL_SPS        = 33,
    HEVC_NAL_PPS        = 34,
};

VAStatus
vlVaHandleVAEncPictureParameterBufferTypeHEVC(vlVaDriver *drv, vlVaContext *context, vlVaBuffer *buf)
{
   VAEncPictureParameterBufferHEVC *h265;
   vlVaBuffer *coded_buf;
   int i;

   h265 = buf->data;
   context->desc.h265enc.decoded_curr_pic = h265->decoded_curr_pic.picture_id;
   context->desc.h265enc.not_referenced = !h265->pic_fields.bits.reference_pic_flag;

   for (i = 0; i < 15; i++)
      context->desc.h265enc.reference_frames[i] = h265->reference_frames[i].picture_id;

   context->desc.h265enc.pic_order_cnt = h265->decoded_curr_pic.pic_order_cnt;
   coded_buf = handle_table_get(drv->htab, h265->coded_buf);
   if (!coded_buf)
      return VA_STATUS_ERROR_INVALID_BUFFER;

   if (!coded_buf->derived_surface.resource)
      coded_buf->derived_surface.resource = pipe_buffer_create(drv->pipe->screen, PIPE_BIND_VERTEX_BUFFER,
                                            PIPE_USAGE_STAGING, coded_buf->size);

   context->coded_buf = coded_buf;
   context->desc.h265enc.pic.log2_parallel_merge_level_minus2 = h265->log2_parallel_merge_level_minus2;
   context->desc.h265enc.pic.nal_unit_type = h265->nal_unit_type;
   context->desc.h265enc.rc.quant_i_frames = h265->pic_init_qp;
   context->desc.h265enc.rc.quant_p_frames = h265->pic_init_qp;
   context->desc.h265enc.rc.quant_b_frames = h265->pic_init_qp;

   switch(h265->pic_fields.bits.coding_type) {
   case 1:
      if (h265->pic_fields.bits.idr_pic_flag)
         context->desc.h265enc.picture_type = PIPE_H2645_ENC_PICTURE_TYPE_IDR;
      else
         context->desc.h265enc.picture_type = PIPE_H2645_ENC_PICTURE_TYPE_I;
      break;
   case 2:
      context->desc.h265enc.picture_type = PIPE_H2645_ENC_PICTURE_TYPE_P;
      break;
   case 3:
   case 4:
   case 5:
      context->desc.h265enc.picture_type = PIPE_H2645_ENC_PICTURE_TYPE_B;
      break;
   }

   context->desc.h265enc.pic.constrained_intra_pred_flag = h265->pic_fields.bits.constrained_intra_pred_flag;
   context->desc.h265enc.pic.pps_loop_filter_across_slices_enabled_flag = h265->pic_fields.bits.pps_loop_filter_across_slices_enabled_flag;
   context->desc.h265enc.pic.transform_skip_enabled_flag = h265->pic_fields.bits.transform_skip_enabled_flag;

   _mesa_hash_table_insert(context->desc.h265enc.frame_idx,
                       UINT_TO_PTR(h265->decoded_curr_pic.picture_id + 1),
                       UINT_TO_PTR(context->desc.h265enc.frame_num));

   /* Initialize slice descriptors for this picture */
   context->desc.h265enc.num_slice_descriptors = 0;
   memset(&context->desc.h265enc.slices_descriptors, 0, sizeof(context->desc.h265enc.slices_descriptors));

   context->desc.h265enc.num_ref_idx_l0_active_minus1 = h265->num_ref_idx_l0_default_active_minus1;
   context->desc.h265enc.num_ref_idx_l1_active_minus1 = h265->num_ref_idx_l1_default_active_minus1;

   return VA_STATUS_SUCCESS;
}

VAStatus
vlVaHandleVAEncSliceParameterBufferTypeHEVC(vlVaDriver *drv, vlVaContext *context, vlVaBuffer *buf)
{
   VAEncSliceParameterBufferHEVC *h265;

   h265 = buf->data;
   memset(&context->desc.h265enc.ref_idx_l0_list, VA_INVALID_ID, sizeof(context->desc.h265enc.ref_idx_l0_list));
   memset(&context->desc.h265enc.ref_idx_l1_list, VA_INVALID_ID, sizeof(context->desc.h265enc.ref_idx_l1_list));

   if (h265->slice_fields.bits.num_ref_idx_active_override_flag) {
      context->desc.h265enc.num_ref_idx_l0_active_minus1 = h265->num_ref_idx_l0_active_minus1;
      context->desc.h265enc.num_ref_idx_l1_active_minus1 = h265->num_ref_idx_l1_active_minus1;
   }

   for (int i = 0; i < 15; i++) {
      if (h265->ref_pic_list0[i].picture_id != VA_INVALID_ID) {
         context->desc.h265enc.ref_idx_l0_list[i] = PTR_TO_UINT(util_hash_table_get(context->desc.h265enc.frame_idx,
                                 UINT_TO_PTR(h265->ref_pic_list0[i].picture_id + 1)));
      }
      if (h265->ref_pic_list1[i].picture_id != VA_INVALID_ID && h265->slice_type == PIPE_H265_SLICE_TYPE_B) {
         context->desc.h265enc.ref_idx_l1_list[i] = PTR_TO_UINT(util_hash_table_get(context->desc.h265enc.frame_idx,
                                 UINT_TO_PTR(h265->ref_pic_list1[i].picture_id + 1)));
      }
   }

   context->desc.h265enc.slice.max_num_merge_cand = h265->max_num_merge_cand;
   context->desc.h265enc.slice.slice_cb_qp_offset = h265->slice_cb_qp_offset;
   context->desc.h265enc.slice.slice_cr_qp_offset = h265->slice_cr_qp_offset;
   context->desc.h265enc.slice.slice_beta_offset_div2 = h265->slice_beta_offset_div2;
   context->desc.h265enc.slice.slice_tc_offset_div2 = h265->slice_tc_offset_div2;
   context->desc.h265enc.slice.cabac_init_flag = h265->slice_fields.bits.cabac_init_flag;
   context->desc.h265enc.slice.slice_deblocking_filter_disabled_flag = h265->slice_fields.bits.slice_deblocking_filter_disabled_flag;
   context->desc.h265enc.slice.slice_loop_filter_across_slices_enabled_flag = h265->slice_fields.bits.slice_loop_filter_across_slices_enabled_flag;

   /* Handle the slice control parameters */
   struct h265_slice_descriptor slice_descriptor;
   memset(&slice_descriptor, 0, sizeof(slice_descriptor));
   slice_descriptor.slice_segment_address = h265->slice_segment_address;
   slice_descriptor.num_ctu_in_slice = h265->num_ctu_in_slice;
   slice_descriptor.slice_type = h265->slice_type;
   assert(slice_descriptor.slice_type <= PIPE_H265_SLICE_TYPE_I);

   if (context->desc.h265enc.num_slice_descriptors < ARRAY_SIZE(context->desc.h265enc.slices_descriptors))
      context->desc.h265enc.slices_descriptors[context->desc.h265enc.num_slice_descriptors++] = slice_descriptor;
   else
      return VA_STATUS_ERROR_NOT_ENOUGH_BUFFER;

   return VA_STATUS_SUCCESS;
}

VAStatus
vlVaHandleVAEncSequenceParameterBufferTypeHEVC(vlVaDriver *drv, vlVaContext *context, vlVaBuffer *buf)
{
   VAEncSequenceParameterBufferHEVC *h265 = (VAEncSequenceParameterBufferHEVC *)buf->data;
   uint32_t num_units_in_tick = 0, time_scale = 0;

   if (!context->decoder) {
      context->templat.max_references = PIPE_H265_MAX_REFERENCES;
      context->templat.level = h265->general_level_idc;
      context->decoder = drv->pipe->create_video_codec(drv->pipe, &context->templat);

      if (!context->decoder)
         return VA_STATUS_ERROR_ALLOCATION_FAILED;

      getEncParamPresetH265(context);
      context->desc.h265enc.rc.vbv_buffer_size = 20000000;
      context->desc.h265enc.rc.vbv_buf_lv = 48;
      context->desc.h265enc.rc.fill_data_enable = 1;
      context->desc.h265enc.rc.enforce_hrd = 1;
      context->desc.h265enc.rc.max_qp = 51;
      context->desc.h265enc.rc.min_qp = 0;
      context->desc.h265enc.intra_refresh.mode = INTRA_REFRESH_MODE_NONE;
      context->desc.h265enc.intra_refresh.offset = 0;
      context->desc.h265enc.intra_refresh.region_size = 0;
      context->desc.h265enc.intra_refresh.need_sequence_header = 0;
   }

   context->desc.h265enc.seq.general_profile_idc = h265->general_profile_idc;
   context->desc.h265enc.seq.general_level_idc = h265->general_level_idc;
   context->desc.h265enc.seq.general_tier_flag = h265->general_tier_flag;
   context->desc.h265enc.seq.intra_period = h265->intra_period;
   context->desc.h265enc.seq.ip_period = h265->ip_period;
   context->desc.h265enc.seq.pic_width_in_luma_samples = h265->pic_width_in_luma_samples;
   context->desc.h265enc.seq.pic_height_in_luma_samples = h265->pic_height_in_luma_samples;
   context->desc.h265enc.seq.chroma_format_idc = h265->seq_fields.bits.chroma_format_idc;
   context->desc.h265enc.seq.bit_depth_luma_minus8 = h265->seq_fields.bits.bit_depth_luma_minus8;
   context->desc.h265enc.seq.bit_depth_chroma_minus8 = h265->seq_fields.bits.bit_depth_chroma_minus8;
   context->desc.h265enc.seq.strong_intra_smoothing_enabled_flag = h265->seq_fields.bits.strong_intra_smoothing_enabled_flag;
   context->desc.h265enc.seq.amp_enabled_flag = h265->seq_fields.bits.amp_enabled_flag;
   context->desc.h265enc.seq.sample_adaptive_offset_enabled_flag = h265->seq_fields.bits.sample_adaptive_offset_enabled_flag;
   context->desc.h265enc.seq.pcm_enabled_flag = h265->seq_fields.bits.pcm_enabled_flag;
   context->desc.h265enc.seq.sps_temporal_mvp_enabled_flag = h265->seq_fields.bits.sps_temporal_mvp_enabled_flag;
   context->desc.h265enc.seq.log2_min_luma_coding_block_size_minus3 = h265->log2_min_luma_coding_block_size_minus3;
   context->desc.h265enc.seq.log2_diff_max_min_luma_coding_block_size = h265->log2_diff_max_min_luma_coding_block_size;
   context->desc.h265enc.seq.log2_min_transform_block_size_minus2 = h265->log2_min_transform_block_size_minus2;
   context->desc.h265enc.seq.log2_diff_max_min_transform_block_size = h265->log2_diff_max_min_transform_block_size;
   context->desc.h265enc.seq.max_transform_hierarchy_depth_inter = h265->max_transform_hierarchy_depth_inter;
   context->desc.h265enc.seq.max_transform_hierarchy_depth_intra = h265->max_transform_hierarchy_depth_intra;

   context->desc.h265enc.seq.vui_parameters_present_flag = h265->vui_parameters_present_flag;
   if (h265->vui_parameters_present_flag) {
      context->desc.h265enc.seq.vui_flags.aspect_ratio_info_present_flag =
         h265->vui_fields.bits.aspect_ratio_info_present_flag;
         context->desc.h265enc.seq.aspect_ratio_idc = h265->aspect_ratio_idc;
      context->desc.h265enc.seq.sar_width = h265->sar_width;
      context->desc.h265enc.seq.sar_height = h265->sar_height;

      context->desc.h265enc.seq.vui_flags.timing_info_present_flag =
         h265->vui_fields.bits.vui_timing_info_present_flag;
      num_units_in_tick = h265->vui_num_units_in_tick;
      time_scale  = h265->vui_time_scale;
      context->desc.h265enc.seq.vui_flags.neutral_chroma_indication_flag =
         h265->vui_fields.bits.neutral_chroma_indication_flag;
      context->desc.h265enc.seq.vui_flags.field_seq_flag =
         h265->vui_fields.bits.field_seq_flag;
      context->desc.h265enc.seq.vui_flags.bitstream_restriction_flag =
         h265->vui_fields.bits.bitstream_restriction_flag;
      context->desc.h265enc.seq.vui_flags.tiles_fixed_structure_flag =
         h265->vui_fields.bits.tiles_fixed_structure_flag;
      context->desc.h265enc.seq.vui_flags.motion_vectors_over_pic_boundaries_flag =
         h265->vui_fields.bits.motion_vectors_over_pic_boundaries_flag;
      context->desc.h265enc.seq.vui_flags.restricted_ref_pic_lists_flag =
         h265->vui_fields.bits.restricted_ref_pic_lists_flag;
      context->desc.h265enc.seq.log2_max_mv_length_vertical =
         h265->vui_fields.bits.log2_max_mv_length_vertical;
      context->desc.h265enc.seq.log2_max_mv_length_horizontal =
         h265->vui_fields.bits.log2_max_mv_length_horizontal;
      context->desc.h265enc.seq.min_spatial_segmentation_idc =
         h265->min_spatial_segmentation_idc;
      context->desc.h265enc.seq.max_bytes_per_pic_denom =
         h265->max_bytes_per_pic_denom;
   } else {
      context->desc.h265enc.seq.vui_flags.timing_info_present_flag = 0;
      context->desc.h265enc.seq.vui_flags.neutral_chroma_indication_flag = 0;
      context->desc.h265enc.seq.vui_flags.field_seq_flag = 0;
      context->desc.h265enc.seq.vui_flags.bitstream_restriction_flag = 0;
      context->desc.h265enc.seq.vui_flags.tiles_fixed_structure_flag = 0;
      context->desc.h265enc.seq.vui_flags.motion_vectors_over_pic_boundaries_flag = 0;
      context->desc.h265enc.seq.vui_flags.restricted_ref_pic_lists_flag = 0;
      context->desc.h265enc.seq.log2_max_mv_length_vertical = 0;
      context->desc.h265enc.seq.log2_max_mv_length_horizontal = 0;
      context->desc.h265enc.seq.min_spatial_segmentation_idc = 0;
      context->desc.h265enc.seq.max_bytes_per_pic_denom = 0;
   }

   if (!context->desc.h265enc.seq.vui_flags.timing_info_present_flag) {
      /* if not present, set default value */
      num_units_in_tick = PIPE_DEFAULT_FRAME_RATE_DEN;
      time_scale  = PIPE_DEFAULT_FRAME_RATE_NUM;
   }

   context->desc.h265enc.seq.num_units_in_tick = num_units_in_tick;
   context->desc.h265enc.seq.time_scale = time_scale;
   context->desc.h265enc.rc.frame_rate_num = time_scale;
   context->desc.h265enc.rc.frame_rate_den = num_units_in_tick;

   return VA_STATUS_SUCCESS;
}

VAStatus
vlVaHandleVAEncMiscParameterTypeRateControlHEVC(vlVaContext *context, VAEncMiscParameterBuffer *misc)
{
   VAEncMiscParameterRateControl *rc = (VAEncMiscParameterRateControl *)misc->data;

   if (context->desc.h265enc.rc.rate_ctrl_method ==
         PIPE_H2645_ENC_RATE_CONTROL_METHOD_CONSTANT)
      context->desc.h265enc.rc.target_bitrate = rc->bits_per_second;
   else
      context->desc.h265enc.rc.target_bitrate = rc->bits_per_second * (rc->target_percentage / 100.0);
   context->desc.h265enc.rc.peak_bitrate = rc->bits_per_second;
   if (context->desc.h265enc.rc.target_bitrate < 2000000)
      context->desc.h265enc.rc.vbv_buffer_size = MIN2((context->desc.h265enc.rc.target_bitrate * 2.75), 2000000);
   else
      context->desc.h265enc.rc.vbv_buffer_size = context->desc.h265enc.rc.target_bitrate;

   context->desc.h265enc.rc.fill_data_enable = !(rc->rc_flags.bits.disable_bit_stuffing);
   /* context->desc.h265enc.rc.skip_frame_enable = !(rc->rc_flags.bits.disable_frame_skip); */
   context->desc.h265enc.rc.skip_frame_enable = 0;
   context->desc.h265enc.rc.max_qp = rc->max_qp;
   context->desc.h265enc.rc.min_qp = rc->min_qp;
   /* Distinguishes from the default params set for these values in other
      functions and app specific params passed down */
   context->desc.h265enc.rc.app_requested_qp_range = ((rc->max_qp > 0) || (rc->min_qp > 0));

   if (context->desc.h265enc.rc.rate_ctrl_method ==
       PIPE_H2645_ENC_RATE_CONTROL_METHOD_QUALITY_VARIABLE)
      context->desc.h265enc.rc.vbr_quality_factor =
         rc->quality_factor;

   return VA_STATUS_SUCCESS;
}

VAStatus
vlVaHandleVAEncMiscParameterTypeFrameRateHEVC(vlVaContext *context, VAEncMiscParameterBuffer *misc)
{
   VAEncMiscParameterFrameRate *fr = (VAEncMiscParameterFrameRate *)misc->data;

   if (fr->framerate & 0xffff0000) {
      context->desc.h265enc.rc.frame_rate_num = fr->framerate       & 0xffff;
      context->desc.h265enc.rc.frame_rate_den = fr->framerate >> 16 & 0xffff;
   } else {
      context->desc.h265enc.rc.frame_rate_num = fr->framerate;
      context->desc.h265enc.rc.frame_rate_den = 1;
   }

   return VA_STATUS_SUCCESS;
}

VAStatus
vlVaHandleVAEncMiscParameterTypeQualityLevelHEVC(vlVaContext *context, VAEncMiscParameterBuffer *misc)
{
   VAEncMiscParameterBufferQualityLevel *ql = (VAEncMiscParameterBufferQualityLevel *)misc->data;
   vlVaHandleVAEncMiscParameterTypeQualityLevel(&context->desc.h265enc.quality_modes,
                               (vlVaQualityBits *)&ql->quality_level);

   return VA_STATUS_SUCCESS;
}

static void profile_tier(struct vl_rbsp *rbsp)
{
   vl_rbsp_u(rbsp, 2); /* general_profile_space */
   vl_rbsp_u(rbsp, 1); /* general_tier_flag */
   vl_rbsp_u(rbsp, 5); /* general_profile_idc */

   /* general_profile_compatibility_flag */
   for(int i = 0; i < 32; ++i)
      vl_rbsp_u(rbsp, 1);

   vl_rbsp_u(rbsp, 1); /* general_progressive_source_flag */
   vl_rbsp_u(rbsp, 1); /* general_interlaced_source_flag */
   vl_rbsp_u(rbsp, 1); /* general_non_packed_constraint_flag */
   vl_rbsp_u(rbsp, 1); /* general_frame_only_constraint_flag */

   /* general_reserved_zero_44bits */
   vl_rbsp_u(rbsp, 16);
   vl_rbsp_u(rbsp, 16);
   vl_rbsp_u(rbsp, 12);
}

static unsigned profile_tier_level(struct vl_rbsp *rbsp,
                                   int max_sublayers_minus1)
{
   bool sub_layer_profile_present_flag[6];
   bool sub_layer_level_present_flag[6];
   unsigned level_idc;
   int i;

   profile_tier(rbsp);
   level_idc = vl_rbsp_u(rbsp, 8);  /* general_level_idc */

   for (i = 0; i < max_sublayers_minus1; ++i) {
      sub_layer_profile_present_flag[i] = vl_rbsp_u(rbsp, 1);
      sub_layer_level_present_flag[i] = vl_rbsp_u(rbsp, 1);
   }

   if (max_sublayers_minus1 > 0)
      for (i = max_sublayers_minus1; i < 8; ++i)
         vl_rbsp_u(rbsp, 2);        /* reserved_zero_2bits */

   for (i = 0; i < max_sublayers_minus1; ++i) {
      if (sub_layer_profile_present_flag[i])
         profile_tier(rbsp);

      if (sub_layer_level_present_flag[i])
         vl_rbsp_u(rbsp, 8);        /* sub_layer_level_idc */
   }

   return level_idc;
}

static void parse_enc_hrd_sublayer_params_hevc(uint32_t cpb_cnt,
                                               uint32_t sub_pic_hrd_params_present_flag,
                                               struct vl_rbsp *rbsp,
                                               struct pipe_h265_enc_sublayer_hrd_params* sublayer_params)
{
   for (unsigned i = 0; i < cpb_cnt; i++ ) {
      sublayer_params->bit_rate_value_minus1[i] = vl_rbsp_ue(rbsp);
      sublayer_params->cpb_size_value_minus1[i] = vl_rbsp_ue(rbsp);
      if( sub_pic_hrd_params_present_flag ) {
         sublayer_params->cpb_size_du_value_minus1[i] = vl_rbsp_ue(rbsp);
         sublayer_params->bit_rate_du_value_minus1[i] = vl_rbsp_ue(rbsp);
      }
      sublayer_params->cbr_flag[i] = vl_rbsp_u(rbsp, 1);
   }
}

static void parse_enc_hrd_params_hevc(struct vl_rbsp *rbsp,
                                      uint32_t commonInfPresentFlag,
                                      uint32_t sps_max_sub_layers_minus1,
                                      struct pipe_h265_enc_hrd_params* hrdParams)
{
   if (commonInfPresentFlag) {
      hrdParams->nal_hrd_parameters_present_flag = vl_rbsp_u(rbsp, 1);
      hrdParams->vcl_hrd_parameters_present_flag = vl_rbsp_u(rbsp, 1);
      if (hrdParams->nal_hrd_parameters_present_flag || hrdParams->vcl_hrd_parameters_present_flag) {
         hrdParams->sub_pic_hrd_params_present_flag = vl_rbsp_u(rbsp, 1);
         if (hrdParams->sub_pic_hrd_params_present_flag) {
            hrdParams->tick_divisor_minus2 = vl_rbsp_u(rbsp, 8);
            hrdParams->du_cpb_removal_delay_increment_length_minus1 = vl_rbsp_u(rbsp, 5);
            hrdParams->sub_pic_cpb_params_in_pic_timing_sei_flag  = vl_rbsp_u(rbsp, 1);
            hrdParams->dpb_output_delay_du_length_minus1 = vl_rbsp_u(rbsp, 5);
         }
         hrdParams->bit_rate_scale = vl_rbsp_u(rbsp, 4);
         hrdParams->cpb_rate_scale = vl_rbsp_u(rbsp, 4);
         if (hrdParams->sub_pic_hrd_params_present_flag)
            hrdParams->cpb_size_du_scale = vl_rbsp_u(rbsp, 4);
         hrdParams->initial_cpb_removal_delay_length_minus1 = vl_rbsp_u(rbsp, 5);
         hrdParams->au_cpb_removal_delay_length_minus1 = vl_rbsp_u(rbsp, 5);
         hrdParams->dpb_output_delay_length_minus1 = vl_rbsp_u(rbsp, 5);
      }
   }

   for (unsigned i = 0; i <= sps_max_sub_layers_minus1; i++) {
      hrdParams->fixed_pic_rate_general_flag[i] = vl_rbsp_u(rbsp, 1);
      if (!hrdParams->fixed_pic_rate_general_flag[i])
         hrdParams->fixed_pic_rate_within_cvs_flag[i] = vl_rbsp_u(rbsp, 1);
      if (hrdParams->fixed_pic_rate_within_cvs_flag[i])
         hrdParams->elemental_duration_in_tc_minus1[i] = vl_rbsp_ue(rbsp);
      else
         hrdParams->low_delay_hrd_flag[i] = vl_rbsp_u(rbsp, 1);
      if (!hrdParams->low_delay_hrd_flag[i])
         hrdParams->cpb_cnt_minus1[i] = vl_rbsp_ue(rbsp);

      if (hrdParams->nal_hrd_parameters_present_flag)
         parse_enc_hrd_sublayer_params_hevc(hrdParams->cpb_cnt_minus1[i] + 1,
                                            hrdParams->sub_pic_hrd_params_present_flag,
                                            rbsp,
                                            &hrdParams->nal_hrd_parameters[i]);

      if (hrdParams->vcl_hrd_parameters_present_flag)
         parse_enc_hrd_sublayer_params_hevc(hrdParams->cpb_cnt_minus1[i] + 1,
                                            hrdParams->sub_pic_hrd_params_present_flag,
                                            rbsp,
                                            &hrdParams->vlc_hrd_parameters[i]);
   }
}
/* dummy function for consuming the scaling list data if it is available */
static void scaling_list_data(struct vl_rbsp *rbsp)
{
   unsigned size_id, matrix_id, coef_num;
   unsigned pre_mode_flag;
   for (size_id = 0; size_id < 4; size_id++) {
      for (matrix_id = 0; matrix_id < 6; matrix_id += (size_id == 3) ? 3 : 1) {
         pre_mode_flag = vl_rbsp_u(rbsp, 1);
         if (pre_mode_flag == 0)
            vl_rbsp_ue(rbsp);
         else {
            coef_num = MIN2(64, (1 << (4 + (size_id << 1))));
            if (size_id > 1)
               vl_rbsp_se(rbsp);
            for (unsigned i = 0; i < coef_num; i++)
               vl_rbsp_se(rbsp);
         }
      }
   }
}

/* i is the working rps, st_rps is the start */
static void st_ref_pic_set(unsigned index,
                           unsigned num_short_term_ref_pic_sets,
                           struct pipe_h265_st_ref_pic_set *st_rps,
                           struct vl_rbsp *rbsp)
{
   unsigned inter_ref_pic_set_pred_flag = (index) ? vl_rbsp_u(rbsp, 1) : 0;
   struct pipe_h265_st_ref_pic_set *ref_rps = NULL;
   unsigned delta_idx_minus1 = 0;
   unsigned i;

   if (inter_ref_pic_set_pred_flag) {
      if (index == num_short_term_ref_pic_sets)
         delta_idx_minus1 = vl_rbsp_ue(rbsp);
      vl_rbsp_u(rbsp, 1);  /* delta_rps_sign */
      vl_rbsp_ue(rbsp); /* abs_delta_rps_minus1 */
      ref_rps = st_rps + index - (delta_idx_minus1 + 1);
      for (i = 0; i <= (ref_rps->num_neg_pics + ref_rps->num_pos_pics); i++) {
         if (!vl_rbsp_u(rbsp, 1)) /* used_by_curr_pic_flag */
            vl_rbsp_u(rbsp, 1); /* use_delta_flag */
      }
   } else {
      st_rps->num_neg_pics = vl_rbsp_ue(rbsp);
      st_rps->num_pos_pics = vl_rbsp_ue(rbsp);
      for (i = 0; i < st_rps->num_neg_pics; i++) {
         vl_rbsp_ue(rbsp); /* delta_poc_s0_minus1 */
         vl_rbsp_u(rbsp, 1); /* used_by_curr_pic_s0_flag */
      }
      for (i = 0; i < st_rps->num_pos_pics; i++) {
         vl_rbsp_ue(rbsp); /* delta_poc_s1_minus1 */
         vl_rbsp_u(rbsp, 1); /* used_by_curr_pic_s1_flag */
      }
   }
}

static void parseEncSpsParamsH265(vlVaContext *context, struct vl_rbsp *rbsp)
{
   int sps_max_sub_layers_minus1;
   unsigned i, sps_sub_layer_ordering_info_present_flag;
   unsigned num_st_ref_pic_sets, num_long_term_ref_pics_sps;
   unsigned log2_max_pic_order_cnt_lsb_minus4;
   struct pipe_h265_st_ref_pic_set *st_rps = NULL;

   vl_rbsp_u(rbsp, 4);     /* sps_video_parameter_set_id */
   sps_max_sub_layers_minus1 = vl_rbsp_u(rbsp, 3);
   vl_rbsp_u(rbsp, 1);     /* sps_temporal_id_nesting_flag */

   /* level_idc */
   profile_tier_level(rbsp, sps_max_sub_layers_minus1);

   vl_rbsp_ue(rbsp);       /* id */
   context->desc.h265enc.seq.chroma_format_idc = vl_rbsp_ue(rbsp);
   if (context->desc.h265enc.seq.chroma_format_idc == 3)
      vl_rbsp_u(rbsp, 1);  /* separate_colour_plane_flag */

   context->desc.h265enc.seq.pic_width_in_luma_samples = vl_rbsp_ue(rbsp);
   context->desc.h265enc.seq.pic_height_in_luma_samples = vl_rbsp_ue(rbsp);

   /* conformance_window_flag - used for cropping */
   context->desc.h265enc.seq.conformance_window_flag = vl_rbsp_u(rbsp, 1);
   if (context->desc.h265enc.seq.conformance_window_flag) {
      context->desc.h265enc.seq.conf_win_left_offset = vl_rbsp_ue(rbsp);
      context->desc.h265enc.seq.conf_win_right_offset = vl_rbsp_ue(rbsp);
      context->desc.h265enc.seq.conf_win_top_offset = vl_rbsp_ue(rbsp);
      context->desc.h265enc.seq.conf_win_bottom_offset = vl_rbsp_ue(rbsp);
   }

   vl_rbsp_ue(rbsp); /* bit_depth_luma_minus8 */
   vl_rbsp_ue(rbsp); /* bit_depth_chroma_minus8 */
   log2_max_pic_order_cnt_lsb_minus4 = vl_rbsp_ue(rbsp);

   sps_sub_layer_ordering_info_present_flag = vl_rbsp_u(rbsp, 1);
   i = sps_sub_layer_ordering_info_present_flag ? 0 : sps_max_sub_layers_minus1;
   for (; i <= sps_max_sub_layers_minus1; ++i) {
      vl_rbsp_ue(rbsp); /* sps_max_dec_pic_buffering_minus1[i] */
      vl_rbsp_ue(rbsp); /* sps_max_num_reorder_pics[i] */
      vl_rbsp_ue(rbsp); /* sps_max_latency_increase_plus1[i] */
   }

   vl_rbsp_ue(rbsp); /* log2_min_luma_coding_block_size_minus3 */
   vl_rbsp_ue(rbsp); /* log2_diff_max_min_luma_coding_block_size */
   vl_rbsp_ue(rbsp); /* log2_min_luma_transform_block_size_minus2 */
   vl_rbsp_ue(rbsp); /* log2_diff_max_min_luma_transform_block_size */
   vl_rbsp_ue(rbsp); /* max_transform_hierarchy_depth_inter */
   vl_rbsp_ue(rbsp); /* max_transform_hierarchy_depth_intra */

   if (vl_rbsp_u(rbsp, 1)) /* scaling_list_enabled_flag */
      if (vl_rbsp_u(rbsp, 1)) /* sps_scaling_list_data_present_flag */
         scaling_list_data(rbsp);

   vl_rbsp_u(rbsp, 1); /* amp_enabled_flag */
   vl_rbsp_u(rbsp, 1); /* sample_adaptive_offset_enabled_flag */
   if (vl_rbsp_u(rbsp, 1)) { /* pcm_enabled_flag */
      vl_rbsp_u(rbsp, 4); /* pcm_sample_bit_depth_luma_minus1 */
      vl_rbsp_u(rbsp, 4); /* pcm_sample_bit_depth_chroma_minus1 */
      vl_rbsp_ue(rbsp); /* log2_min_pcm_luma_coding_block_size_minus3 */
      vl_rbsp_ue(rbsp); /* log2_diff_max_min_pcm_luma_coding_block_size */
      vl_rbsp_u(rbsp, 1); /* pcm_loop_filter_disabled_flag */
   }

   num_st_ref_pic_sets = vl_rbsp_ue(rbsp); /* num_short_term_ref_pic_sets */
   for (i = 0; i < num_st_ref_pic_sets; i++) {
      if (i == 0) /* allocating st_ref_pic_sets */
         st_rps = (struct pipe_h265_st_ref_pic_set *)
            CALLOC(num_st_ref_pic_sets + 1, sizeof(struct pipe_h265_st_ref_pic_set));

      st_ref_pic_set(i, num_st_ref_pic_sets, st_rps, rbsp);
   }

   if (vl_rbsp_u(rbsp, 1)) {/* long_term_ref_pics_present_flag */
      num_long_term_ref_pics_sps = vl_rbsp_ue(rbsp);
      for (i = 0; i < num_long_term_ref_pics_sps; i++) {
         /* lt_ref_pic_poc_lsb_sps */
         vl_rbsp_u(rbsp, log2_max_pic_order_cnt_lsb_minus4 + 4);
         vl_rbsp_u(rbsp, 1); /* used_by_curr_pic_lt_sps_flag */
      }
   }

   vl_rbsp_u(rbsp, 1); /* sps_temporal_mvp_enabled_flag */
   vl_rbsp_u(rbsp, 1); /* strong_intra_smoothing_enabled_flag */

   context->desc.h265enc.seq.vui_parameters_present_flag = vl_rbsp_u(rbsp, 1);
   if (context->desc.h265enc.seq.vui_parameters_present_flag) {
      context->desc.h265enc.seq.vui_flags.aspect_ratio_info_present_flag = vl_rbsp_u(rbsp, 1);
      if (context->desc.h265enc.seq.vui_flags.aspect_ratio_info_present_flag) {
         context->desc.h265enc.seq.aspect_ratio_idc = vl_rbsp_u(rbsp, 8);
         if (context->desc.h265enc.seq.aspect_ratio_idc == 255 /* Extended_SAR */) {
            context->desc.h265enc.seq.sar_width = vl_rbsp_u(rbsp, 16);
            context->desc.h265enc.seq.sar_height = vl_rbsp_u(rbsp, 16);
         }
      }

      context->desc.h265enc.seq.vui_flags.overscan_info_present_flag = vl_rbsp_u(rbsp, 1);
      if (context->desc.h265enc.seq.vui_flags.overscan_info_present_flag)
         context->desc.h265enc.seq.vui_flags.overscan_appropriate_flag = vl_rbsp_u(rbsp, 1);

      context->desc.h265enc.seq.vui_flags.video_signal_type_present_flag = vl_rbsp_u(rbsp, 1);
      if (context->desc.h265enc.seq.vui_flags.video_signal_type_present_flag) {
         context->desc.h265enc.seq.video_format = vl_rbsp_u(rbsp, 3);
         context->desc.h265enc.seq.video_full_range_flag = vl_rbsp_u(rbsp, 1);
         context->desc.h265enc.seq.vui_flags.colour_description_present_flag = vl_rbsp_u(rbsp, 1);
         if (context->desc.h265enc.seq.vui_flags.colour_description_present_flag) {
            context->desc.h265enc.seq.colour_primaries = vl_rbsp_u(rbsp, 8);
            context->desc.h265enc.seq.transfer_characteristics = vl_rbsp_u(rbsp, 8);
            context->desc.h265enc.seq.matrix_coefficients = vl_rbsp_u(rbsp, 8);
         }
      }

      context->desc.h265enc.seq.vui_flags.chroma_loc_info_present_flag = vl_rbsp_u(rbsp, 1);
      if (context->desc.h265enc.seq.vui_flags.chroma_loc_info_present_flag) {
         context->desc.h265enc.seq.chroma_sample_loc_type_top_field = vl_rbsp_ue(rbsp);
         context->desc.h265enc.seq.chroma_sample_loc_type_bottom_field = vl_rbsp_ue(rbsp);
      }

      context->desc.h265enc.seq.vui_flags.neutral_chroma_indication_flag = vl_rbsp_u(rbsp, 1);
      context->desc.h265enc.seq.vui_flags.field_seq_flag = vl_rbsp_u(rbsp, 1);
      context->desc.h265enc.seq.vui_flags.frame_field_info_present_flag = vl_rbsp_u(rbsp, 1);
      context->desc.h265enc.seq.vui_flags.default_display_window_flag = vl_rbsp_u(rbsp, 1);

      if (context->desc.h265enc.seq.vui_flags.default_display_window_flag) {
         context->desc.h265enc.seq.def_disp_win_left_offset = vl_rbsp_ue(rbsp);
         context->desc.h265enc.seq.def_disp_win_right_offset = vl_rbsp_ue(rbsp);
         context->desc.h265enc.seq.def_disp_win_top_offset = vl_rbsp_ue(rbsp);
         context->desc.h265enc.seq.def_disp_win_bottom_offset = vl_rbsp_ue(rbsp);
      }

      context->desc.h265enc.seq.vui_flags.timing_info_present_flag = vl_rbsp_u(rbsp, 1);

      if (context->desc.h265enc.seq.vui_flags.timing_info_present_flag) {
         uint32_t num_units_in_tick_high = vl_rbsp_u(rbsp, 16);
         uint32_t num_units_in_tick_low = vl_rbsp_u(rbsp, 16);
         context->desc.h265enc.seq.num_units_in_tick = (num_units_in_tick_high << 16) | num_units_in_tick_low;

         uint32_t time_scale_high = vl_rbsp_u(rbsp, 16);
         uint32_t time_scale_low = vl_rbsp_u(rbsp, 16);
         context->desc.h265enc.seq.time_scale = (time_scale_high << 16) | time_scale_low;

         context->desc.h265enc.seq.vui_flags.poc_proportional_to_timing_flag = vl_rbsp_u(rbsp, 1);
         if (context->desc.h265enc.seq.vui_flags.poc_proportional_to_timing_flag) {
            context->desc.h265enc.seq.num_ticks_poc_diff_one_minus1 = vl_rbsp_ue(rbsp);
            context->desc.h265enc.seq.vui_flags.hrd_parameters_present_flag = vl_rbsp_u(rbsp, 1);
            if (context->desc.h265enc.seq.vui_flags.hrd_parameters_present_flag)
               parse_enc_hrd_params_hevc(rbsp,
                                         1,
                                         sps_max_sub_layers_minus1,
                                         &context->desc.h265enc.seq.hrd_parameters);
         }
      }

      context->desc.h265enc.seq.vui_flags.bitstream_restriction_flag = vl_rbsp_u(rbsp, 1);
      if (context->desc.h265enc.seq.vui_flags.bitstream_restriction_flag) {
         context->desc.h265enc.seq.vui_flags.tiles_fixed_structure_flag = vl_rbsp_u(rbsp, 1);
         context->desc.h265enc.seq.vui_flags.motion_vectors_over_pic_boundaries_flag = vl_rbsp_u(rbsp, 1);
         context->desc.h265enc.seq.vui_flags.restricted_ref_pic_lists_flag = vl_rbsp_u(rbsp, 1);
         context->desc.h265enc.seq.min_spatial_segmentation_idc = vl_rbsp_ue(rbsp);
         context->desc.h265enc.seq.max_bytes_per_pic_denom = vl_rbsp_ue(rbsp);
         context->desc.h265enc.seq.max_bits_per_min_cu_denom = vl_rbsp_ue(rbsp);
         context->desc.h265enc.seq.log2_max_mv_length_horizontal = vl_rbsp_ue(rbsp);
         context->desc.h265enc.seq.log2_max_mv_length_vertical = vl_rbsp_ue(rbsp);
      }
   }

   if (st_rps) FREE(st_rps);
}

VAStatus
vlVaHandleVAEncPackedHeaderDataBufferTypeHEVC(vlVaContext *context, vlVaBuffer *buf)
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

      vl_vlc_eatbits(&vlc, 1);
      unsigned nal_unit_type = vl_vlc_get_uimsbf(&vlc, 6);
      vl_vlc_eatbits(&vlc, 6);
      vl_vlc_eatbits(&vlc, 3);

      struct vl_rbsp rbsp;
      vl_rbsp_init(&rbsp, &vlc, ~0, context->packed_header_emulation_bytes);

      switch(nal_unit_type) {
      case HEVC_NAL_SPS:
         parseEncSpsParamsH265(context, &rbsp);
         break;
      case HEVC_NAL_VPS:
      case HEVC_NAL_PPS:
      default:
         break;
      }

      if (!context->packed_header_emulation_bytes)
         break;
   }

   return VA_STATUS_SUCCESS;
}

VAStatus
vlVaHandleVAEncMiscParameterTypeMaxFrameSizeHEVC(vlVaContext *context, VAEncMiscParameterBuffer *misc)
{
   VAEncMiscParameterBufferMaxFrameSize *ms = (VAEncMiscParameterBufferMaxFrameSize *)misc->data;
   context->desc.h265enc.rc.max_au_size = ms->max_frame_size;
   return VA_STATUS_SUCCESS;
}

VAStatus
vlVaHandleVAEncMiscParameterTypeHRDHEVC(vlVaContext *context, VAEncMiscParameterBuffer *misc)
{
   VAEncMiscParameterHRD *ms = (VAEncMiscParameterHRD *)misc->data;

   if (ms->buffer_size) {
      context->desc.h265enc.rc.vbv_buffer_size = ms->buffer_size;
      context->desc.h265enc.rc.vbv_buf_lv = (ms->initial_buffer_fullness << 6 ) / ms->buffer_size;
      context->desc.h265enc.rc.vbv_buf_initial_size = ms->initial_buffer_fullness;
      /* Distinguishes from the default params set for these values in other
         functions and app specific params passed down via HRD buffer */
      context->desc.h265enc.rc.app_requested_hrd_buffer = true;
   }

   return VA_STATUS_SUCCESS;
}

void getEncParamPresetH265(vlVaContext *context)
{
   //rate control
   if (context->desc.h265enc.rc.frame_rate_num == 0 ||
       context->desc.h265enc.rc.frame_rate_den == 0) {
      context->desc.h265enc.rc.frame_rate_num = 30;
      context->desc.h265enc.rc.frame_rate_den = 1;
   }
   context->desc.h265enc.rc.target_bits_picture =
      context->desc.h265enc.rc.target_bitrate *
      ((float)context->desc.h265enc.rc.frame_rate_den /
      context->desc.h265enc.rc.frame_rate_num);
   context->desc.h265enc.rc.peak_bits_picture_integer =
      context->desc.h265enc.rc.peak_bitrate *
      ((float)context->desc.h265enc.rc.frame_rate_den /
      context->desc.h265enc.rc.frame_rate_num);

   context->desc.h265enc.rc.peak_bits_picture_fraction = 0;
}
