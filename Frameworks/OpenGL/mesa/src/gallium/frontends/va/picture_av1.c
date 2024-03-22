/**************************************************************************
 *
 * Copyright 2021 Advanced Micro Devices, Inc.
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

#include "util/vl_vlc.h"
#include "va_private.h"

#define AV1_REFS_PER_FRAME 7
#define AV1_NUM_REF_FRAMES 8
#define AV1_MAX_SEGMENTS 8
#define AV1_SEG_LVL_MAX 8
#define AV1_MAX_CDEF_BITS_ARRAY 8
#define AV1_FG_MAX_NUM_Y_POINTS 14
#define AV1_FG_MAX_NUM_CBR_POINTS 10
#define AV1_FG_MAX_NUM_POS_LUMA 24
#define AV1_FG_MAX_NUM_POS_CHROMA 25

static void tile_info(vlVaContext *context, VADecPictureParameterBufferAV1 *av1)
{
   unsigned sbCols;
   unsigned sbRows;
   int width_sb;
   int height_sb;
   unsigned startSb, i;
   unsigned MiCols = 2 * ((av1->frame_width_minus1 + 8) >> 3);
   unsigned MiRows = 2 * ((av1->frame_height_minus1 + 8) >> 3);

   unsigned TileColsLog2 = util_logbase2_ceil(av1->tile_cols);
   unsigned TileRowsLog2 = util_logbase2_ceil(av1->tile_rows);

   if (av1->pic_info_fields.bits.use_superres) {
      unsigned width = ((av1->frame_width_minus1 + 1) * 8 + av1->superres_scale_denominator / 2)
         / av1->superres_scale_denominator;
      MiCols = 2 * (((width - 1) + 8) >> 3);
   }

   sbCols = (av1->seq_info_fields.fields.use_128x128_superblock) ?
      ((MiCols + 31) >> 5) : ((MiCols + 15) >> 4);
   sbRows = (av1->seq_info_fields.fields.use_128x128_superblock) ?
      ((MiRows + 31) >> 5) : ((MiRows + 15) >> 4);

   width_sb = sbCols;
   height_sb = sbRows;

   if (av1->pic_info_fields.bits.uniform_tile_spacing_flag) {
      unsigned tileWidthSb, tileHeightSb;

      tileWidthSb = (sbCols + (1 << TileColsLog2) - 1) >> TileColsLog2;
      i = 0;
      for (startSb = 0; startSb < sbCols; startSb += tileWidthSb) {
         context->desc.av1.picture_parameter.tile_col_start_sb[i] = startSb;
         context->desc.av1.picture_parameter.width_in_sbs[i] = tileWidthSb;
         i++;
      }
      context->desc.av1.picture_parameter.tile_col_start_sb[i] = sbCols;

      tileHeightSb = (sbRows + (1 << TileRowsLog2) - 1) >> TileRowsLog2;
      i = 0;
      for (startSb = 0; startSb < sbRows; startSb += tileHeightSb) {
         context->desc.av1.picture_parameter.tile_row_start_sb[i] = startSb;
         context->desc.av1.picture_parameter.height_in_sbs[i] = tileHeightSb;
         i++;
      }
      context->desc.av1.picture_parameter.tile_row_start_sb[i] = sbRows;
   } else {
      unsigned widestTileSb = 0;

      startSb = 0;
      for (i = 0; startSb < sbCols; ++i) {
         unsigned sizeSb;

         context->desc.av1.picture_parameter.tile_col_start_sb[i] = startSb;
         sizeSb = (av1->width_in_sbs_minus_1)[i] + 1;
         context->desc.av1.picture_parameter.width_in_sbs[i] = sizeSb;
         widestTileSb = MAX2(sizeSb, widestTileSb);
         startSb += sizeSb;
         width_sb -= sizeSb;
      }
      context->desc.av1.picture_parameter.tile_col_start_sb[i] = startSb + width_sb;

      startSb = 0;
      for (i = 0; startSb < sbRows; ++i) {
         unsigned height_in_sbs_minus_1 = (av1->height_in_sbs_minus_1)[i];
         context->desc.av1.picture_parameter.height_in_sbs[i] = height_in_sbs_minus_1 + 1;

         context->desc.av1.picture_parameter.tile_row_start_sb[i] = startSb;
         startSb += height_in_sbs_minus_1 + 1;
         height_sb -= height_in_sbs_minus_1 + 1;
      }
      context->desc.av1.picture_parameter.tile_row_start_sb[i] = startSb + height_sb;
   }
}

void vlVaHandlePictureParameterBufferAV1(vlVaDriver *drv, vlVaContext *context, vlVaBuffer *buf)
{
   VADecPictureParameterBufferAV1 *av1 = buf->data;
   int i, j;
   bool use_lr;

   assert(buf->size >= sizeof(VADecPictureParameterBufferAV1) && buf->num_elements == 1);

   context->desc.av1.picture_parameter.profile = av1->profile;
   context->desc.av1.picture_parameter.seq_info_fields.use_128x128_superblock =
      av1->seq_info_fields.fields.use_128x128_superblock;
   context->desc.av1.picture_parameter.seq_info_fields.enable_filter_intra =
      av1->seq_info_fields.fields.enable_filter_intra;
   context->desc.av1.picture_parameter.seq_info_fields.enable_cdef =
      av1->seq_info_fields.fields.enable_cdef;
   context->desc.av1.picture_parameter.seq_info_fields.film_grain_params_present =
      av1->seq_info_fields.fields.film_grain_params_present;
   context->desc.av1.picture_parameter.seq_info_fields.enable_intra_edge_filter =
      av1->seq_info_fields.fields.enable_intra_edge_filter;
   context->desc.av1.picture_parameter.order_hint_bits_minus_1 = av1->order_hint_bits_minus_1;
   context->desc.av1.picture_parameter.max_width = av1->frame_width_minus1 + 1;
   context->desc.av1.picture_parameter.max_height = av1->frame_height_minus1 + 1;
   context->desc.av1.picture_parameter.seq_info_fields.enable_interintra_compound =
      av1->seq_info_fields.fields.enable_interintra_compound;
   context->desc.av1.picture_parameter.seq_info_fields.enable_masked_compound =
      av1->seq_info_fields.fields.enable_masked_compound;
   context->desc.av1.picture_parameter.seq_info_fields.enable_dual_filter =
      av1->seq_info_fields.fields.enable_dual_filter;
   context->desc.av1.picture_parameter.seq_info_fields.enable_order_hint =
      av1->seq_info_fields.fields.enable_order_hint;
   context->desc.av1.picture_parameter.seq_info_fields.enable_jnt_comp =
      av1->seq_info_fields.fields.enable_jnt_comp;

   context->desc.av1.picture_parameter.seq_info_fields.ref_frame_mvs =
      av1->seq_info_fields.fields.enable_order_hint;

   context->desc.av1.picture_parameter.bit_depth_idx = av1->bit_depth_idx;
   context->desc.av1.picture_parameter.seq_info_fields.mono_chrome =
      av1->seq_info_fields.fields.mono_chrome;

   context->desc.av1.picture_parameter.pic_info_fields.showable_frame =
      av1->pic_info_fields.bits.showable_frame;
   context->desc.av1.picture_parameter.pic_info_fields.frame_type =
      av1->pic_info_fields.bits.frame_type;
   context->desc.av1.picture_parameter.pic_info_fields.show_frame =
      av1->pic_info_fields.bits.show_frame;
   context->desc.av1.picture_parameter.pic_info_fields.error_resilient_mode =
      av1->pic_info_fields.bits.error_resilient_mode;
   context->desc.av1.picture_parameter.pic_info_fields.disable_cdf_update =
      av1->pic_info_fields.bits.disable_cdf_update;
   context->desc.av1.picture_parameter.pic_info_fields.allow_screen_content_tools =
      av1->pic_info_fields.bits.allow_screen_content_tools;
   context->desc.av1.picture_parameter.pic_info_fields.force_integer_mv =
      av1->pic_info_fields.bits.force_integer_mv;
   context->desc.av1.picture_parameter.pic_info_fields.allow_intrabc =
      av1->pic_info_fields.bits.allow_intrabc;
   context->desc.av1.picture_parameter.pic_info_fields.use_superres =
      av1->pic_info_fields.bits.use_superres;
   context->desc.av1.picture_parameter.pic_info_fields.is_motion_mode_switchable =
      av1->pic_info_fields.bits.is_motion_mode_switchable;
   context->desc.av1.picture_parameter.pic_info_fields.allow_high_precision_mv =
      av1->pic_info_fields.bits.allow_high_precision_mv;
   context->desc.av1.picture_parameter.pic_info_fields.use_ref_frame_mvs =
      av1->pic_info_fields.bits.use_ref_frame_mvs;
   context->desc.av1.picture_parameter.pic_info_fields.disable_frame_end_update_cdf =
      av1->pic_info_fields.bits.disable_frame_end_update_cdf;
   context->desc.av1.picture_parameter.pic_info_fields.allow_warped_motion =
      av1->pic_info_fields.bits.allow_warped_motion;
   context->desc.av1.picture_parameter.pic_info_fields.uniform_tile_spacing_flag =
      av1->pic_info_fields.bits.uniform_tile_spacing_flag;
   context->desc.av1.picture_parameter.pic_info_fields.large_scale_tile =
      av1->pic_info_fields.bits.large_scale_tile;

   context->desc.av1.picture_parameter.matrix_coefficients =
      av1->matrix_coefficients;

   context->desc.av1.film_grain_target = NULL;
   if (av1->film_grain_info.film_grain_info_fields.bits.apply_grain)
      context->desc.av1.picture_parameter.current_frame_id = av1->current_display_picture;
   else
      context->desc.av1.picture_parameter.current_frame_id = av1->current_frame;

   context->desc.av1.picture_parameter.order_hint = av1->order_hint;
   context->desc.av1.picture_parameter.primary_ref_frame = av1->primary_ref_frame;
   context->desc.av1.picture_parameter.frame_width = av1->frame_width_minus1 + 1;
   context->desc.av1.picture_parameter.frame_height = av1->frame_height_minus1 + 1;

   context->desc.av1.picture_parameter.superres_scale_denominator =
      av1->superres_scale_denominator;

   for (i = 0; i < AV1_REFS_PER_FRAME; ++i)
      context->desc.av1.picture_parameter.ref_frame_idx[i] = av1->ref_frame_idx[i];
   context->desc.av1.picture_parameter.refresh_frame_flags = 1;

   /* Tile Info */
   context->desc.av1.picture_parameter.tile_cols = av1->tile_cols;
   context->desc.av1.picture_parameter.tile_rows = av1->tile_rows;
   context->desc.av1.picture_parameter.context_update_tile_id = av1->context_update_tile_id;
   tile_info(context, av1);

   /* Quantization Params */
   context->desc.av1.picture_parameter.base_qindex =  av1->base_qindex;
   context->desc.av1.picture_parameter.y_dc_delta_q = av1->y_dc_delta_q;
   context->desc.av1.picture_parameter.u_dc_delta_q = av1->u_dc_delta_q;
   context->desc.av1.picture_parameter.u_ac_delta_q = av1->u_ac_delta_q;
   context->desc.av1.picture_parameter.v_dc_delta_q = av1->v_dc_delta_q;
   context->desc.av1.picture_parameter.v_ac_delta_q = av1->v_ac_delta_q;
   context->desc.av1.picture_parameter.qmatrix_fields.using_qmatrix =
      av1->qmatrix_fields.bits.using_qmatrix;
   context->desc.av1.picture_parameter.qmatrix_fields.qm_y = av1->qmatrix_fields.bits.using_qmatrix
      ? av1->qmatrix_fields.bits.qm_y : 0xf;
   context->desc.av1.picture_parameter.qmatrix_fields.qm_u = av1->qmatrix_fields.bits.using_qmatrix
      ? av1->qmatrix_fields.bits.qm_u : 0xf;
   context->desc.av1.picture_parameter.qmatrix_fields.qm_v = av1->qmatrix_fields.bits.using_qmatrix
      ? av1->qmatrix_fields.bits.qm_v : 0xf;

   /* Segmentation Params */
   context->desc.av1.picture_parameter.seg_info.segment_info_fields.enabled =
      av1->seg_info.segment_info_fields.bits.enabled;
   context->desc.av1.picture_parameter.seg_info.segment_info_fields.update_map =
      av1->seg_info.segment_info_fields.bits.update_map;
   context->desc.av1.picture_parameter.seg_info.segment_info_fields.update_data =
      av1->seg_info.segment_info_fields.bits.update_data;
   context->desc.av1.picture_parameter.seg_info.segment_info_fields.temporal_update =
      av1->seg_info.segment_info_fields.bits.temporal_update;
   for (i = 0; i < AV1_MAX_SEGMENTS; ++i) {
      for (j = 0; j < AV1_SEG_LVL_MAX; ++j)
         context->desc.av1.picture_parameter.seg_info.feature_data[i][j] =
            av1->seg_info.feature_data[i][j];
      context->desc.av1.picture_parameter.seg_info.feature_mask[i] = av1->seg_info.feature_mask[i];
   }

   /* Delta Q Params */
   context->desc.av1.picture_parameter.mode_control_fields.delta_q_present_flag =
      av1->mode_control_fields.bits.delta_q_present_flag;
   context->desc.av1.picture_parameter.mode_control_fields.log2_delta_q_res =
      av1->mode_control_fields.bits.log2_delta_q_res;

   /* Delta LF Params */
   context->desc.av1.picture_parameter.mode_control_fields.delta_lf_present_flag =
      av1->mode_control_fields.bits.delta_lf_present_flag;
   context->desc.av1.picture_parameter.mode_control_fields.log2_delta_lf_res =
      av1->mode_control_fields.bits.log2_delta_lf_res;
   context->desc.av1.picture_parameter.mode_control_fields.delta_lf_multi =
      av1->mode_control_fields.bits.delta_lf_multi;

   context->desc.av1.picture_parameter.mode_control_fields.tx_mode =
      av1->mode_control_fields.bits.tx_mode;
   context->desc.av1.picture_parameter.mode_control_fields.reference_select =
      av1->mode_control_fields.bits.reference_select;
   context->desc.av1.picture_parameter.mode_control_fields.reduced_tx_set_used =
      av1->mode_control_fields.bits.reduced_tx_set_used;
   context->desc.av1.picture_parameter.mode_control_fields.skip_mode_present =
      av1->mode_control_fields.bits.skip_mode_present;

   /* Loop Filter Params */
   context->desc.av1.picture_parameter.interp_filter = av1->interp_filter;
   for (i = 0; i < 2; ++i)
      context->desc.av1.picture_parameter.filter_level[i] = av1->filter_level[i];
   context->desc.av1.picture_parameter.filter_level_u = av1->filter_level_u;
   context->desc.av1.picture_parameter.filter_level_v = av1->filter_level_v;
   context->desc.av1.picture_parameter.loop_filter_info_fields.sharpness_level =
      av1->loop_filter_info_fields.bits.sharpness_level;
   context->desc.av1.picture_parameter.loop_filter_info_fields.mode_ref_delta_enabled =
      av1->loop_filter_info_fields.bits.mode_ref_delta_enabled;
   context->desc.av1.picture_parameter.loop_filter_info_fields.mode_ref_delta_update =
      av1->loop_filter_info_fields.bits.mode_ref_delta_update;
   for (i = 0; i < AV1_NUM_REF_FRAMES; ++i)
      context->desc.av1.picture_parameter.ref_deltas[i] = av1->ref_deltas[i];
   for (i = 0; i < 2; ++i)
      context->desc.av1.picture_parameter.mode_deltas[i] = av1->mode_deltas[i];

   /* CDEF Params */
   context->desc.av1.picture_parameter.cdef_damping_minus_3 = av1->cdef_damping_minus_3;
   context->desc.av1.picture_parameter.cdef_bits = av1->cdef_bits;
   for (i = 0; i < AV1_MAX_CDEF_BITS_ARRAY; ++i) {
      context->desc.av1.picture_parameter.cdef_y_strengths[i] = av1->cdef_y_strengths[i];
      context->desc.av1.picture_parameter.cdef_uv_strengths[i] = av1->cdef_uv_strengths[i];
   }

   /* Loop Restoration Params */
   context->desc.av1.picture_parameter.loop_restoration_fields.yframe_restoration_type =
      av1->loop_restoration_fields.bits.yframe_restoration_type;
   context->desc.av1.picture_parameter.loop_restoration_fields.cbframe_restoration_type =
      av1->loop_restoration_fields.bits.cbframe_restoration_type;
   context->desc.av1.picture_parameter.loop_restoration_fields.crframe_restoration_type =
      av1->loop_restoration_fields.bits.crframe_restoration_type;
   context->desc.av1.picture_parameter.loop_restoration_fields.lr_unit_shift =
      av1->loop_restoration_fields.bits.lr_unit_shift;
   context->desc.av1.picture_parameter.loop_restoration_fields.lr_uv_shift =
      av1->loop_restoration_fields.bits.lr_uv_shift;

   use_lr = av1->loop_restoration_fields.bits.yframe_restoration_type ||
            av1->loop_restoration_fields.bits.cbframe_restoration_type ||
            av1->loop_restoration_fields.bits.crframe_restoration_type;

   if (use_lr) {
      context->desc.av1.picture_parameter.lr_unit_size[0]
         = 1 << (6 + av1->loop_restoration_fields.bits.lr_unit_shift);
      context->desc.av1.picture_parameter.lr_unit_size[1]
         = 1 << (6 + av1->loop_restoration_fields.bits.lr_unit_shift
                   - av1->loop_restoration_fields.bits.lr_uv_shift);
      context->desc.av1.picture_parameter.lr_unit_size[2]
         = context->desc.av1.picture_parameter.lr_unit_size[1];
   } else {
      for (i = 0; i < 3; ++i)
         context->desc.av1.picture_parameter.lr_unit_size[i] = (1 << 8);
   }

   /* Global Motion Params */
   for (i = 0; i < ARRAY_SIZE(av1->wm); ++i) {
      context->desc.av1.picture_parameter.wm[i].wmtype = av1->wm[i].wmtype;
      context->desc.av1.picture_parameter.wm[i].invalid = av1->wm[i].invalid;
      for (j = 0; j < ARRAY_SIZE(av1->wm[0].wmmat); ++j)
         context->desc.av1.picture_parameter.wm[i].wmmat[j] = av1->wm[i].wmmat[j];
   }

   /* Film Grain Params */
   context->desc.av1.picture_parameter.film_grain_info.film_grain_info_fields.apply_grain =
      av1->film_grain_info.film_grain_info_fields.bits.apply_grain;
   context->desc.av1.picture_parameter.film_grain_info.film_grain_info_fields.chroma_scaling_from_luma =
      av1->film_grain_info.film_grain_info_fields.bits.chroma_scaling_from_luma;
   context->desc.av1.picture_parameter.film_grain_info.film_grain_info_fields.grain_scaling_minus_8 =
      av1->film_grain_info.film_grain_info_fields.bits.grain_scaling_minus_8;
   context->desc.av1.picture_parameter.film_grain_info.film_grain_info_fields.ar_coeff_lag =
      av1->film_grain_info.film_grain_info_fields.bits.ar_coeff_lag;
   context->desc.av1.picture_parameter.film_grain_info.film_grain_info_fields.ar_coeff_shift_minus_6 =
      av1->film_grain_info.film_grain_info_fields.bits.ar_coeff_shift_minus_6;
   context->desc.av1.picture_parameter.film_grain_info.film_grain_info_fields.grain_scale_shift =
      av1->film_grain_info.film_grain_info_fields.bits.grain_scale_shift;
   context->desc.av1.picture_parameter.film_grain_info.film_grain_info_fields.overlap_flag =
      av1->film_grain_info.film_grain_info_fields.bits.overlap_flag;
   context->desc.av1.picture_parameter.film_grain_info.film_grain_info_fields.clip_to_restricted_range =
      av1->film_grain_info.film_grain_info_fields.bits.clip_to_restricted_range;

   context->desc.av1.picture_parameter.film_grain_info.grain_seed = av1->film_grain_info.grain_seed;
   context->desc.av1.picture_parameter.film_grain_info.num_y_points = av1->film_grain_info.num_y_points;
   for (i = 0; i < AV1_FG_MAX_NUM_Y_POINTS; ++i) {
      context->desc.av1.picture_parameter.film_grain_info.point_y_value[i] =
         av1->film_grain_info.point_y_value[i];
      context->desc.av1.picture_parameter.film_grain_info.point_y_scaling[i] =
         av1->film_grain_info.point_y_scaling[i];
   }
   context->desc.av1.picture_parameter.film_grain_info.num_cb_points = av1->film_grain_info.num_cb_points;
   context->desc.av1.picture_parameter.film_grain_info.num_cr_points = av1->film_grain_info.num_cr_points;
   for (i = 0; i < AV1_FG_MAX_NUM_CBR_POINTS; ++i) {
      context->desc.av1.picture_parameter.film_grain_info.point_cb_value[i] =
         av1->film_grain_info.point_cb_value[i];
      context->desc.av1.picture_parameter.film_grain_info.point_cb_scaling[i] =
         av1->film_grain_info.point_cb_scaling[i];
      context->desc.av1.picture_parameter.film_grain_info.point_cr_value[i] =
         av1->film_grain_info.point_cr_value[i];
      context->desc.av1.picture_parameter.film_grain_info.point_cr_scaling[i] =
         av1->film_grain_info.point_cr_scaling[i];
   }

   for (i = 0; i < AV1_FG_MAX_NUM_POS_LUMA; ++i)
      context->desc.av1.picture_parameter.film_grain_info.ar_coeffs_y[i] =
         av1->film_grain_info.ar_coeffs_y[i];
   for (i = 0; i < AV1_FG_MAX_NUM_POS_CHROMA; ++i) {
      context->desc.av1.picture_parameter.film_grain_info.ar_coeffs_cb[i] =
         av1->film_grain_info.ar_coeffs_cb[i];
      context->desc.av1.picture_parameter.film_grain_info.ar_coeffs_cr[i] =
         av1->film_grain_info.ar_coeffs_cr[i];
   }
   context->desc.av1.picture_parameter.film_grain_info.cb_mult = av1->film_grain_info.cb_mult;
   context->desc.av1.picture_parameter.film_grain_info.cb_luma_mult = av1->film_grain_info.cb_luma_mult;
   context->desc.av1.picture_parameter.film_grain_info.cb_offset = av1->film_grain_info.cb_offset;
   context->desc.av1.picture_parameter.film_grain_info.cr_mult = av1->film_grain_info.cr_mult;
   context->desc.av1.picture_parameter.film_grain_info.cr_luma_mult = av1->film_grain_info.cr_luma_mult;
   context->desc.av1.picture_parameter.film_grain_info.cr_offset = av1->film_grain_info.cr_offset;

   for (i = 0 ; i < AV1_NUM_REF_FRAMES; ++i) {
      if (av1->pic_info_fields.bits.frame_type == 0 && av1->pic_info_fields.bits.show_frame)
         context->desc.av1.ref[i] = NULL;
      else
         vlVaGetReferenceFrame(drv, av1->ref_frame_map[i], &context->desc.av1.ref[i]);
   }

  context->desc.av1.slice_parameter.slice_count = 0;
}

void vlVaHandleSliceParameterBufferAV1(vlVaContext *context, vlVaBuffer *buf, unsigned num_slices)
{
   for (uint32_t buffer_idx = 0; buffer_idx < buf->num_elements; buffer_idx++) {
      uint32_t slice_index =
               /* slices obtained so far from vaRenderPicture in previous calls*/
               num_slices +
               /* current slice index processing this VASliceParameterBufferAV1 */
               buffer_idx;

      VASliceParameterBufferAV1 *av1 = &(((VASliceParameterBufferAV1*)buf->data)[buffer_idx]);
      context->desc.av1.slice_parameter.slice_data_size[slice_index] = av1->slice_data_size;
      context->desc.av1.slice_parameter.slice_data_offset[slice_index] = av1->slice_data_offset;
      context->desc.av1.slice_parameter.slice_data_row[slice_index] = av1->tile_row;
      context->desc.av1.slice_parameter.slice_data_col[slice_index] = av1->tile_column;
      context->desc.av1.slice_parameter.slice_data_anchor_frame_idx[slice_index] = av1->anchor_frame_idx;
      context->desc.av1.slice_parameter.slice_count = slice_index + 1;
   }
}
