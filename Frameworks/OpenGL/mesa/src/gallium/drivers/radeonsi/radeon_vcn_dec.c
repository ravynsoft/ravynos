/**************************************************************************
 *
 * Copyright 2017 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 *
 **************************************************************************/

#include "radeon_vcn_dec.h"

#include "pipe/p_video_codec.h"
#include "radeonsi/si_pipe.h"
#include "util/u_memory.h"
#include "util/u_video.h"
#include "vl/vl_mpeg12_decoder.h"
#include "vl/vl_probs_table.h"
#include "pspdecryptionparam.h"

#include <assert.h>
#include <stdio.h>

#include "ac_vcn_av1_default.h"

#define FB_BUFFER_OFFSET             0x2000
#define FB_BUFFER_SIZE               2048
#define IT_SCALING_TABLE_SIZE        992
#define VP9_PROBS_TABLE_SIZE         (RDECODE_VP9_PROBS_DATA_SIZE + 256)

#define NUM_MPEG2_REFS 6
#define NUM_H264_REFS  17
#define NUM_VC1_REFS   5
#define NUM_VP9_REFS   8
#define NUM_AV1_REFS   8
#define NUM_AV1_REFS_PER_FRAME 7

static unsigned calc_dpb_size(struct radeon_decoder *dec);
static unsigned calc_ctx_size_h264_perf(struct radeon_decoder *dec);
static unsigned calc_ctx_size_h265_main(struct radeon_decoder *dec);
static unsigned calc_ctx_size_h265_main10(struct radeon_decoder *dec,
                                          struct pipe_h265_picture_desc *pic);

static void radeon_dec_destroy_associated_data(void *data)
{
   /* NOOP, since we only use an intptr */
}

static void get_current_pic_index(struct radeon_decoder *dec,
                                    struct pipe_video_buffer *target,
                                    unsigned char *curr_pic_idx)
{
   for (int i = 0; i < ARRAY_SIZE(dec->render_pic_list); ++i) {
      if (dec->render_pic_list[i] && dec->render_pic_list[i] == target) {
         if (target->codec != NULL)
            *curr_pic_idx = (uintptr_t)vl_video_buffer_get_associated_data(target, &dec->base);
         else {
            *curr_pic_idx = i;
            vl_video_buffer_set_associated_data(target, &dec->base, (void *)(uintptr_t)i,
                                                &radeon_dec_destroy_associated_data);
         }
         break;
      } else if (!dec->render_pic_list[i]) {
         dec->render_pic_list[i] = target;
         *curr_pic_idx = i;
         vl_video_buffer_set_associated_data(target, &dec->base, (void *)(uintptr_t)i,
                                             &radeon_dec_destroy_associated_data);
         break;
      }
   }
}

static rvcn_dec_message_avc_t get_h264_msg(struct radeon_decoder *dec,
                                           struct pipe_video_buffer *target,
                                           struct pipe_h264_picture_desc *pic)
{
   rvcn_dec_message_avc_t result;
   unsigned i, j, k;

   memset(&result, 0, sizeof(result));
   switch (pic->base.profile) {
   case PIPE_VIDEO_PROFILE_MPEG4_AVC_BASELINE:
   case PIPE_VIDEO_PROFILE_MPEG4_AVC_CONSTRAINED_BASELINE:
      result.profile = RDECODE_H264_PROFILE_BASELINE;
      break;

   case PIPE_VIDEO_PROFILE_MPEG4_AVC_MAIN:
      result.profile = RDECODE_H264_PROFILE_MAIN;
      break;

   case PIPE_VIDEO_PROFILE_MPEG4_AVC_HIGH:
      result.profile = RDECODE_H264_PROFILE_HIGH;
      break;

   default:
      assert(0);
      break;
   }

   result.level = dec->base.level;

   result.sps_info_flags = 0;
   result.sps_info_flags |= pic->pps->sps->direct_8x8_inference_flag << 0;
   result.sps_info_flags |= pic->pps->sps->mb_adaptive_frame_field_flag << 1;
   result.sps_info_flags |= pic->pps->sps->frame_mbs_only_flag << 2;
   result.sps_info_flags |= pic->pps->sps->delta_pic_order_always_zero_flag << 3;
   result.sps_info_flags |= ((dec->dpb_type == DPB_DYNAMIC_TIER_2) ? 0 : 1)
                              << RDECODE_SPS_INFO_H264_EXTENSION_SUPPORT_FLAG_SHIFT;

   result.bit_depth_luma_minus8 = pic->pps->sps->bit_depth_luma_minus8;
   result.bit_depth_chroma_minus8 = pic->pps->sps->bit_depth_chroma_minus8;
   result.log2_max_frame_num_minus4 = pic->pps->sps->log2_max_frame_num_minus4;
   result.pic_order_cnt_type = pic->pps->sps->pic_order_cnt_type;
   result.log2_max_pic_order_cnt_lsb_minus4 = pic->pps->sps->log2_max_pic_order_cnt_lsb_minus4;

   switch (dec->base.chroma_format) {
   case PIPE_VIDEO_CHROMA_FORMAT_NONE:
      break;
   case PIPE_VIDEO_CHROMA_FORMAT_400:
      result.chroma_format = 0;
      break;
   case PIPE_VIDEO_CHROMA_FORMAT_420:
      result.chroma_format = 1;
      break;
   case PIPE_VIDEO_CHROMA_FORMAT_422:
      result.chroma_format = 2;
      break;
   case PIPE_VIDEO_CHROMA_FORMAT_444:
      result.chroma_format = 3;
      break;
   }

   result.pps_info_flags = 0;
   result.pps_info_flags |= pic->pps->transform_8x8_mode_flag << 0;
   result.pps_info_flags |= pic->pps->redundant_pic_cnt_present_flag << 1;
   result.pps_info_flags |= pic->pps->constrained_intra_pred_flag << 2;
   result.pps_info_flags |= pic->pps->deblocking_filter_control_present_flag << 3;
   result.pps_info_flags |= pic->pps->weighted_bipred_idc << 4;
   result.pps_info_flags |= pic->pps->weighted_pred_flag << 6;
   result.pps_info_flags |= pic->pps->bottom_field_pic_order_in_frame_present_flag << 7;
   result.pps_info_flags |= pic->pps->entropy_coding_mode_flag << 8;

   result.num_slice_groups_minus1 = pic->pps->num_slice_groups_minus1;
   result.slice_group_map_type = pic->pps->slice_group_map_type;
   result.slice_group_change_rate_minus1 = pic->pps->slice_group_change_rate_minus1;
   result.pic_init_qp_minus26 = pic->pps->pic_init_qp_minus26;
   result.chroma_qp_index_offset = pic->pps->chroma_qp_index_offset;
   result.second_chroma_qp_index_offset = pic->pps->second_chroma_qp_index_offset;

   memcpy(result.scaling_list_4x4, pic->pps->ScalingList4x4, 6 * 16);
   memcpy(result.scaling_list_8x8, pic->pps->ScalingList8x8, 2 * 64);

   memcpy(dec->it, result.scaling_list_4x4, 6 * 16);
   memcpy((dec->it + 96), result.scaling_list_8x8, 2 * 64);

   result.num_ref_frames = pic->num_ref_frames;

   result.num_ref_idx_l0_active_minus1 = pic->num_ref_idx_l0_active_minus1;
   result.num_ref_idx_l1_active_minus1 = pic->num_ref_idx_l1_active_minus1;

   result.frame_num = pic->frame_num;
   memcpy(result.frame_num_list, pic->frame_num_list, 4 * 16);
   result.curr_field_order_cnt_list[0] = pic->field_order_cnt[0];
   result.curr_field_order_cnt_list[1] = pic->field_order_cnt[1];
   memcpy(result.field_order_cnt_list, pic->field_order_cnt_list, 4 * 16 * 2);
   result.non_existing_frame_flags = 0;
   result.used_for_reference_flags = 0;

   if (dec->dpb_type != DPB_DYNAMIC_TIER_2) {
      result.decoded_pic_idx = pic->frame_num;
      goto end;
   }

   for (i = 0; i < ARRAY_SIZE(dec->render_pic_list); i++) {
      for (j = 0; (pic->ref[j] != NULL) && (j < ARRAY_SIZE(dec->render_pic_list)); j++) {
         if (dec->render_pic_list[i] == pic->ref[j])
            break;
         if (j == ARRAY_SIZE(dec->render_pic_list) - 1)
            dec->render_pic_list[i] = NULL;
         else if (pic->ref[j + 1] == NULL)
            dec->render_pic_list[i] = NULL;
      }
   }
   for (i = 0; i < ARRAY_SIZE(dec->render_pic_list); ++i) {
      if (dec->render_pic_list[i] && dec->render_pic_list[i] == target) {
         if (target->codec != NULL){
            result.decoded_pic_idx =
               (uintptr_t)vl_video_buffer_get_associated_data(target, &dec->base);
         } else {
            result.decoded_pic_idx = i;
            vl_video_buffer_set_associated_data(target, &dec->base, (void *)(uintptr_t)i,
                  &radeon_dec_destroy_associated_data);
         }
         break;
      }
   }
   if (i == ARRAY_SIZE(dec->render_pic_list)) {
      for (i = 0; i < ARRAY_SIZE(dec->render_pic_list); ++i) {
         if (!dec->render_pic_list[i]) {
            dec->render_pic_list[i] = target;
            result.decoded_pic_idx = i;
            vl_video_buffer_set_associated_data(target, &dec->base, (void *)(uintptr_t)i,
                  &radeon_dec_destroy_associated_data);
            break;
         }
      }
   }
   for (i = 0; i < ARRAY_SIZE(result.ref_frame_list); i++) {
      result.ref_frame_list[i] = pic->ref[i] ?
              (uintptr_t)vl_video_buffer_get_associated_data(pic->ref[i], &dec->base) : 0xff;

      if (result.ref_frame_list[i] != 0xff) {
         if (pic->top_is_reference[i])
            result.used_for_reference_flags |= (1 << (2 * i));
         if (pic->bottom_is_reference[i])
            result.used_for_reference_flags |= (1 << (2 * i + 1));

         if (pic->is_long_term[i])
            result.ref_frame_list[i] |= 0x80;

         result.curr_pic_ref_frame_num++;

         for (j = 0; j < ARRAY_SIZE(dec->h264_valid_ref_num); j++) {
            if ((dec->h264_valid_ref_num[j] != (unsigned)-1)
                && (dec->h264_valid_ref_num[j] == result.frame_num_list[i]))
               break;
         }

         for (k = 0; k < ARRAY_SIZE(dec->h264_valid_poc_num); k++) {
            if ((dec->h264_valid_poc_num[k] != (unsigned)-1)
                  && ((dec->h264_valid_poc_num[k] == result.field_order_cnt_list[i][0])
                    || dec->h264_valid_poc_num[k] == result.field_order_cnt_list[i][1]))
               break;
         }
      }
      if (result.ref_frame_list[i] != 0xff && (j == ARRAY_SIZE(dec->h264_valid_ref_num))
                                           && (k == ARRAY_SIZE(dec->h264_valid_poc_num))) {
         result.non_existing_frame_flags |= 1 << i;
         result.curr_pic_ref_frame_num--;
         result.ref_frame_list[i] = 0xff;
      }
   }

   /* if reference picture exists, however no reference picture found at the end
      curr_pic_ref_frame_num == 0, which is not reasonable, should be corrected. */
   if (result.used_for_reference_flags && (result.curr_pic_ref_frame_num == 0)) {
      for (i = 0; i < ARRAY_SIZE(result.ref_frame_list); i++) {
         result.ref_frame_list[i] = pic->ref[i] ?
                (uintptr_t)vl_video_buffer_get_associated_data(pic->ref[i], &dec->base) : 0xff;
         if (result.ref_frame_list[i] != 0xff) {
            result.curr_pic_ref_frame_num++;
            result.non_existing_frame_flags &= ~(1 << i);
            break;
         }
      }
   }

   for (i = 0; i < ARRAY_SIZE(result.ref_frame_list); i++) {
      if (result.ref_frame_list[i] != 0xff) {
         dec->h264_valid_ref_num[i]         = result.frame_num_list[i];
         dec->h264_valid_poc_num[2 * i]     = pic->top_is_reference[i] ?
                      result.field_order_cnt_list[i][0] : (unsigned) -1;
         dec->h264_valid_poc_num[2 * i + 1] = pic->bottom_is_reference[i] ?
                      result.field_order_cnt_list[i][1] : (unsigned) -1;
      } else {
         dec->h264_valid_ref_num[i]         =
         dec->h264_valid_poc_num[2 * i]     =
         dec->h264_valid_poc_num[2 * i + 1] = (unsigned) -1;
      }
   }

   dec->h264_valid_ref_num[ARRAY_SIZE(dec->h264_valid_ref_num) - 1] = result.frame_num;
   dec->h264_valid_poc_num[ARRAY_SIZE(dec->h264_valid_poc_num) - 2] =
                     pic->field_pic_flag && pic->bottom_field_flag ?
                     (unsigned) -1 : result.curr_field_order_cnt_list[0];
   dec->h264_valid_poc_num[ARRAY_SIZE(dec->h264_valid_poc_num) - 1] =
                     pic->field_pic_flag && !pic->bottom_field_flag ?
                     (unsigned) -1 : result.curr_field_order_cnt_list[1];

   if (dec->dpb_type == DPB_DYNAMIC_TIER_2) {
      dec->ref_codec.bts = CODEC_8_BITS;
      dec->ref_codec.index = result.decoded_pic_idx;
      dec->ref_codec.ref_size = 16;
      memset(dec->ref_codec.ref_list, 0xff, sizeof(dec->ref_codec.ref_list));
      memcpy(dec->ref_codec.ref_list, result.ref_frame_list, sizeof(result.ref_frame_list));
   }

end:
   return result;
}

static rvcn_dec_message_hevc_t get_h265_msg(struct radeon_decoder *dec,
                                            struct pipe_video_buffer *target,
                                            struct pipe_h265_picture_desc *pic)
{
   rvcn_dec_message_hevc_t result;
   unsigned i, j;

   memset(&result, 0, sizeof(result));
   result.sps_info_flags = 0;
   result.sps_info_flags |= pic->pps->sps->scaling_list_enabled_flag << 0;
   result.sps_info_flags |= pic->pps->sps->amp_enabled_flag << 1;
   result.sps_info_flags |= pic->pps->sps->sample_adaptive_offset_enabled_flag << 2;
   result.sps_info_flags |= pic->pps->sps->pcm_enabled_flag << 3;
   result.sps_info_flags |= pic->pps->sps->pcm_loop_filter_disabled_flag << 4;
   result.sps_info_flags |= pic->pps->sps->long_term_ref_pics_present_flag << 5;
   result.sps_info_flags |= pic->pps->sps->sps_temporal_mvp_enabled_flag << 6;
   result.sps_info_flags |= pic->pps->sps->strong_intra_smoothing_enabled_flag << 7;
   result.sps_info_flags |= pic->pps->sps->separate_colour_plane_flag << 8;
   if (((struct si_screen *)dec->screen)->info.family == CHIP_CARRIZO)
      result.sps_info_flags |= 1 << 9;
   if (pic->UseRefPicList == true) {
      result.sps_info_flags |= 1 << 10;
      result.sps_info_flags |= 1 << 12;
   }
   if (pic->UseStRpsBits == true && pic->pps->st_rps_bits != 0) {
      result.sps_info_flags |= 1 << 11;
      result.st_rps_bits = pic->pps->st_rps_bits;
   }

   result.chroma_format = pic->pps->sps->chroma_format_idc;
   result.bit_depth_luma_minus8 = pic->pps->sps->bit_depth_luma_minus8;
   result.bit_depth_chroma_minus8 = pic->pps->sps->bit_depth_chroma_minus8;
   result.log2_max_pic_order_cnt_lsb_minus4 = pic->pps->sps->log2_max_pic_order_cnt_lsb_minus4;
   result.sps_max_dec_pic_buffering_minus1 = pic->pps->sps->sps_max_dec_pic_buffering_minus1;
   result.log2_min_luma_coding_block_size_minus3 =
      pic->pps->sps->log2_min_luma_coding_block_size_minus3;
   result.log2_diff_max_min_luma_coding_block_size =
      pic->pps->sps->log2_diff_max_min_luma_coding_block_size;
   result.log2_min_transform_block_size_minus2 =
      pic->pps->sps->log2_min_transform_block_size_minus2;
   result.log2_diff_max_min_transform_block_size =
      pic->pps->sps->log2_diff_max_min_transform_block_size;
   result.max_transform_hierarchy_depth_inter = pic->pps->sps->max_transform_hierarchy_depth_inter;
   result.max_transform_hierarchy_depth_intra = pic->pps->sps->max_transform_hierarchy_depth_intra;
   result.pcm_sample_bit_depth_luma_minus1 = pic->pps->sps->pcm_sample_bit_depth_luma_minus1;
   result.pcm_sample_bit_depth_chroma_minus1 = pic->pps->sps->pcm_sample_bit_depth_chroma_minus1;
   result.log2_min_pcm_luma_coding_block_size_minus3 =
      pic->pps->sps->log2_min_pcm_luma_coding_block_size_minus3;
   result.log2_diff_max_min_pcm_luma_coding_block_size =
      pic->pps->sps->log2_diff_max_min_pcm_luma_coding_block_size;
   result.num_short_term_ref_pic_sets = pic->pps->sps->num_short_term_ref_pic_sets;

   result.pps_info_flags = 0;
   result.pps_info_flags |= pic->pps->dependent_slice_segments_enabled_flag << 0;
   result.pps_info_flags |= pic->pps->output_flag_present_flag << 1;
   result.pps_info_flags |= pic->pps->sign_data_hiding_enabled_flag << 2;
   result.pps_info_flags |= pic->pps->cabac_init_present_flag << 3;
   result.pps_info_flags |= pic->pps->constrained_intra_pred_flag << 4;
   result.pps_info_flags |= pic->pps->transform_skip_enabled_flag << 5;
   result.pps_info_flags |= pic->pps->cu_qp_delta_enabled_flag << 6;
   result.pps_info_flags |= pic->pps->pps_slice_chroma_qp_offsets_present_flag << 7;
   result.pps_info_flags |= pic->pps->weighted_pred_flag << 8;
   result.pps_info_flags |= pic->pps->weighted_bipred_flag << 9;
   result.pps_info_flags |= pic->pps->transquant_bypass_enabled_flag << 10;
   result.pps_info_flags |= pic->pps->tiles_enabled_flag << 11;
   result.pps_info_flags |= pic->pps->entropy_coding_sync_enabled_flag << 12;
   result.pps_info_flags |= pic->pps->uniform_spacing_flag << 13;
   result.pps_info_flags |= pic->pps->loop_filter_across_tiles_enabled_flag << 14;
   result.pps_info_flags |= pic->pps->pps_loop_filter_across_slices_enabled_flag << 15;
   result.pps_info_flags |= pic->pps->deblocking_filter_override_enabled_flag << 16;
   result.pps_info_flags |= pic->pps->pps_deblocking_filter_disabled_flag << 17;
   result.pps_info_flags |= pic->pps->lists_modification_present_flag << 18;
   result.pps_info_flags |= pic->pps->slice_segment_header_extension_present_flag << 19;

   result.num_extra_slice_header_bits = pic->pps->num_extra_slice_header_bits;
   result.num_long_term_ref_pic_sps = pic->pps->sps->num_long_term_ref_pics_sps;
   result.num_ref_idx_l0_default_active_minus1 = pic->pps->num_ref_idx_l0_default_active_minus1;
   result.num_ref_idx_l1_default_active_minus1 = pic->pps->num_ref_idx_l1_default_active_minus1;
   result.pps_cb_qp_offset = pic->pps->pps_cb_qp_offset;
   result.pps_cr_qp_offset = pic->pps->pps_cr_qp_offset;
   result.pps_beta_offset_div2 = pic->pps->pps_beta_offset_div2;
   result.pps_tc_offset_div2 = pic->pps->pps_tc_offset_div2;
   result.diff_cu_qp_delta_depth = pic->pps->diff_cu_qp_delta_depth;
   result.num_tile_columns_minus1 = pic->pps->num_tile_columns_minus1;
   result.num_tile_rows_minus1 = pic->pps->num_tile_rows_minus1;
   result.log2_parallel_merge_level_minus2 = pic->pps->log2_parallel_merge_level_minus2;
   result.init_qp_minus26 = pic->pps->init_qp_minus26;

   for (i = 0; i < 19; ++i)
      result.column_width_minus1[i] = pic->pps->column_width_minus1[i];

   for (i = 0; i < 21; ++i)
      result.row_height_minus1[i] = pic->pps->row_height_minus1[i];

   result.num_delta_pocs_ref_rps_idx = pic->NumDeltaPocsOfRefRpsIdx;
   result.curr_poc = pic->CurrPicOrderCntVal;

   for (i = 0; i < ARRAY_SIZE(dec->render_pic_list); i++) {
      for (j = 0;
           (pic->ref[j] != NULL) && (j < ARRAY_SIZE(dec->render_pic_list));
           j++) {
         if (dec->render_pic_list[i] == pic->ref[j])
            break;
         if (j == ARRAY_SIZE(dec->render_pic_list) - 1)
            dec->render_pic_list[i] = NULL;
         else if (pic->ref[j + 1] == NULL)
            dec->render_pic_list[i] = NULL;
      }
   }
   for (i = 0; i < ARRAY_SIZE(dec->render_pic_list); i++) {
      if (dec->render_pic_list[i] == NULL) {
         dec->render_pic_list[i] = target;
         result.curr_idx = i;
         break;
      }
   }

   vl_video_buffer_set_associated_data(target, &dec->base, (void *)(uintptr_t)result.curr_idx,
                                       &radeon_dec_destroy_associated_data);

   for (i = 0; i < 16; ++i) {
      struct pipe_video_buffer *ref = pic->ref[i];
      uintptr_t ref_pic = 0;

      result.poc_list[i] = pic->PicOrderCntVal[i];

      if (ref)
         ref_pic = (uintptr_t)vl_video_buffer_get_associated_data(ref, &dec->base);
      else
         ref_pic = 0x7F;
      result.ref_pic_list[i] = ref_pic;
   }

   for (i = 0; i < 8; ++i) {
      result.ref_pic_set_st_curr_before[i] = 0xFF;
      result.ref_pic_set_st_curr_after[i] = 0xFF;
      result.ref_pic_set_lt_curr[i] = 0xFF;
   }

   for (i = 0; i < pic->NumPocStCurrBefore; ++i)
      result.ref_pic_set_st_curr_before[i] = pic->RefPicSetStCurrBefore[i];

   for (i = 0; i < pic->NumPocStCurrAfter; ++i)
      result.ref_pic_set_st_curr_after[i] = pic->RefPicSetStCurrAfter[i];

   for (i = 0; i < pic->NumPocLtCurr; ++i)
      result.ref_pic_set_lt_curr[i] = pic->RefPicSetLtCurr[i];

   for (i = 0; i < 6; ++i)
      result.ucScalingListDCCoefSizeID2[i] = pic->pps->sps->ScalingListDCCoeff16x16[i];

   for (i = 0; i < 2; ++i)
      result.ucScalingListDCCoefSizeID3[i] = pic->pps->sps->ScalingListDCCoeff32x32[i];

   memcpy(dec->it, pic->pps->sps->ScalingList4x4, 6 * 16);
   memcpy(dec->it + 96, pic->pps->sps->ScalingList8x8, 6 * 64);
   memcpy(dec->it + 480, pic->pps->sps->ScalingList16x16, 6 * 64);
   memcpy(dec->it + 864, pic->pps->sps->ScalingList32x32, 2 * 64);

   for (i = 0; i < 2; i++) {
      for (j = 0; j < 15; j++)
         result.direct_reflist[i][j] = pic->RefPicList[0][i][j];
   }

   if (pic->base.profile == PIPE_VIDEO_PROFILE_HEVC_MAIN_10) {
      if (target->buffer_format == PIPE_FORMAT_P010 || target->buffer_format == PIPE_FORMAT_P016) {
         result.p010_mode = 1;
         result.msb_mode = 1;
      } else {
         result.p010_mode = 0;
         result.luma_10to8 = 5;
         result.chroma_10to8 = 5;
         result.hevc_reserved[0] = 4; /* sclr_luma10to8 */
         result.hevc_reserved[1] = 4; /* sclr_chroma10to8 */
      }
   }

   if (dec->dpb_type == DPB_DYNAMIC_TIER_2) {
      dec->ref_codec.bts = (pic->base.profile == PIPE_VIDEO_PROFILE_HEVC_MAIN_10) ?
         CODEC_10_BITS : CODEC_8_BITS;
      dec->ref_codec.index = result.curr_idx;
      dec->ref_codec.ref_size = 15;
      memset(dec->ref_codec.ref_list, 0x7f, sizeof(dec->ref_codec.ref_list));
      memcpy(dec->ref_codec.ref_list, result.ref_pic_list, sizeof(result.ref_pic_list));
   }
   return result;
}

static void fill_probs_table(void *ptr)
{
   rvcn_dec_vp9_probs_t *probs = (rvcn_dec_vp9_probs_t *)ptr;

   memcpy(&probs->coef_probs[0], default_coef_probs_4x4, sizeof(default_coef_probs_4x4));
   memcpy(&probs->coef_probs[1], default_coef_probs_8x8, sizeof(default_coef_probs_8x8));
   memcpy(&probs->coef_probs[2], default_coef_probs_16x16, sizeof(default_coef_probs_16x16));
   memcpy(&probs->coef_probs[3], default_coef_probs_32x32, sizeof(default_coef_probs_32x32));
   memcpy(probs->y_mode_prob, default_if_y_probs, sizeof(default_if_y_probs));
   memcpy(probs->uv_mode_prob, default_if_uv_probs, sizeof(default_if_uv_probs));
   memcpy(probs->single_ref_prob, default_single_ref_p, sizeof(default_single_ref_p));
   memcpy(probs->switchable_interp_prob, default_switchable_interp_prob,
          sizeof(default_switchable_interp_prob));
   memcpy(probs->partition_prob, default_partition_probs, sizeof(default_partition_probs));
   memcpy(probs->inter_mode_probs, default_inter_mode_probs, sizeof(default_inter_mode_probs));
   memcpy(probs->mbskip_probs, default_skip_probs, sizeof(default_skip_probs));
   memcpy(probs->intra_inter_prob, default_intra_inter_p, sizeof(default_intra_inter_p));
   memcpy(probs->comp_inter_prob, default_comp_inter_p, sizeof(default_comp_inter_p));
   memcpy(probs->comp_ref_prob, default_comp_ref_p, sizeof(default_comp_ref_p));
   memcpy(probs->tx_probs_32x32, default_tx_probs_32x32, sizeof(default_tx_probs_32x32));
   memcpy(probs->tx_probs_16x16, default_tx_probs_16x16, sizeof(default_tx_probs_16x16));
   memcpy(probs->tx_probs_8x8, default_tx_probs_8x8, sizeof(default_tx_probs_8x8));
   memcpy(probs->mv_joints, default_nmv_joints, sizeof(default_nmv_joints));
   memcpy(&probs->mv_comps[0], default_nmv_components, sizeof(default_nmv_components));
   memset(&probs->nmvc_mask, 0, sizeof(rvcn_dec_vp9_nmv_ctx_mask_t));
}

static rvcn_dec_message_vp9_t get_vp9_msg(struct radeon_decoder *dec,
                                          struct pipe_video_buffer *target,
                                          struct pipe_vp9_picture_desc *pic)
{
   rvcn_dec_message_vp9_t result;
   unsigned i ,j;

   memset(&result, 0, sizeof(result));

   /* segment table */
   rvcn_dec_vp9_probs_segment_t *prbs = (rvcn_dec_vp9_probs_segment_t *)(dec->probs);

   if (pic->picture_parameter.pic_fields.segmentation_enabled) {
      for (i = 0; i < 8; ++i) {
         prbs->seg.feature_data[i] =
            (pic->slice_parameter.seg_param[i].alt_quant & 0xffff) |
            ((pic->slice_parameter.seg_param[i].alt_lf & 0xff) << 16) |
            ((pic->slice_parameter.seg_param[i].segment_flags.segment_reference & 0xf) << 24);
         prbs->seg.feature_mask[i] =
            (pic->slice_parameter.seg_param[i].alt_quant_enabled << 0) |
            (pic->slice_parameter.seg_param[i].alt_lf_enabled << 1) |
            (pic->slice_parameter.seg_param[i].segment_flags.segment_reference_enabled << 2) |
            (pic->slice_parameter.seg_param[i].segment_flags.segment_reference_skipped << 3);
      }

      for (i = 0; i < 7; ++i)
         prbs->seg.tree_probs[i] = pic->picture_parameter.mb_segment_tree_probs[i];

      for (i = 0; i < 3; ++i)
         prbs->seg.pred_probs[i] = pic->picture_parameter.segment_pred_probs[i];

      prbs->seg.abs_delta = pic->picture_parameter.abs_delta;
   } else
      memset(&prbs->seg, 0, 256);

   result.frame_header_flags = (pic->picture_parameter.pic_fields.frame_type
                                << RDECODE_FRAME_HDR_INFO_VP9_FRAME_TYPE_SHIFT) &
                               RDECODE_FRAME_HDR_INFO_VP9_FRAME_TYPE_MASK;

   result.frame_header_flags |= (pic->picture_parameter.pic_fields.error_resilient_mode
                                 << RDECODE_FRAME_HDR_INFO_VP9_ERROR_RESILIENT_MODE_SHIFT) &
                                RDECODE_FRAME_HDR_INFO_VP9_ERROR_RESILIENT_MODE_MASK;

   result.frame_header_flags |= (pic->picture_parameter.pic_fields.intra_only
                                 << RDECODE_FRAME_HDR_INFO_VP9_INTRA_ONLY_SHIFT) &
                                RDECODE_FRAME_HDR_INFO_VP9_INTRA_ONLY_MASK;

   result.frame_header_flags |= (pic->picture_parameter.pic_fields.allow_high_precision_mv
                                 << RDECODE_FRAME_HDR_INFO_VP9_ALLOW_HIGH_PRECISION_MV_SHIFT) &
                                RDECODE_FRAME_HDR_INFO_VP9_ALLOW_HIGH_PRECISION_MV_MASK;

   result.frame_header_flags |= (pic->picture_parameter.pic_fields.frame_parallel_decoding_mode
                                 << RDECODE_FRAME_HDR_INFO_VP9_FRAME_PARALLEL_DECODING_MODE_SHIFT) &
                                RDECODE_FRAME_HDR_INFO_VP9_FRAME_PARALLEL_DECODING_MODE_MASK;

   result.frame_header_flags |= (pic->picture_parameter.pic_fields.refresh_frame_context
                                 << RDECODE_FRAME_HDR_INFO_VP9_REFRESH_FRAME_CONTEXT_SHIFT) &
                                RDECODE_FRAME_HDR_INFO_VP9_REFRESH_FRAME_CONTEXT_MASK;

   result.frame_header_flags |= (pic->picture_parameter.pic_fields.segmentation_enabled
                                 << RDECODE_FRAME_HDR_INFO_VP9_SEGMENTATION_ENABLED_SHIFT) &
                                RDECODE_FRAME_HDR_INFO_VP9_SEGMENTATION_ENABLED_MASK;

   result.frame_header_flags |= (pic->picture_parameter.pic_fields.segmentation_update_map
                                 << RDECODE_FRAME_HDR_INFO_VP9_SEGMENTATION_UPDATE_MAP_SHIFT) &
                                RDECODE_FRAME_HDR_INFO_VP9_SEGMENTATION_UPDATE_MAP_MASK;

   result.frame_header_flags |= (pic->picture_parameter.pic_fields.segmentation_temporal_update
                                 << RDECODE_FRAME_HDR_INFO_VP9_SEGMENTATION_TEMPORAL_UPDATE_SHIFT) &
                                RDECODE_FRAME_HDR_INFO_VP9_SEGMENTATION_TEMPORAL_UPDATE_MASK;

   result.frame_header_flags |= (pic->picture_parameter.mode_ref_delta_enabled
                                 << RDECODE_FRAME_HDR_INFO_VP9_MODE_REF_DELTA_ENABLED_SHIFT) &
                                RDECODE_FRAME_HDR_INFO_VP9_MODE_REF_DELTA_ENABLED_MASK;

   result.frame_header_flags |= (pic->picture_parameter.mode_ref_delta_update
                                 << RDECODE_FRAME_HDR_INFO_VP9_MODE_REF_DELTA_UPDATE_SHIFT) &
                                RDECODE_FRAME_HDR_INFO_VP9_MODE_REF_DELTA_UPDATE_MASK;

   result.frame_header_flags |=
      ((dec->show_frame && !pic->picture_parameter.pic_fields.error_resilient_mode &&
        dec->last_width == dec->base.width && dec->last_height == dec->base.height)
       << RDECODE_FRAME_HDR_INFO_VP9_USE_PREV_IN_FIND_MV_REFS_SHIFT) &
      RDECODE_FRAME_HDR_INFO_VP9_USE_PREV_IN_FIND_MV_REFS_MASK;
   dec->show_frame = pic->picture_parameter.pic_fields.show_frame;

   result.frame_header_flags |=  (1 << RDECODE_FRAME_HDR_INFO_VP9_USE_UNCOMPRESSED_HEADER_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_VP9_USE_UNCOMPRESSED_HEADER_MASK;

   result.interp_filter = pic->picture_parameter.pic_fields.mcomp_filter_type;

   result.frame_context_idx = pic->picture_parameter.pic_fields.frame_context_idx;
   result.reset_frame_context = pic->picture_parameter.pic_fields.reset_frame_context;

   result.filter_level = pic->picture_parameter.filter_level;
   result.sharpness_level = pic->picture_parameter.sharpness_level;

   for (i = 0; i < 8; ++i)
      memcpy(result.lf_adj_level[i], pic->slice_parameter.seg_param[i].filter_level, 4 * 2);

   if (pic->picture_parameter.pic_fields.lossless_flag) {
      result.base_qindex = 0;
      result.y_dc_delta_q = 0;
      result.uv_ac_delta_q = 0;
      result.uv_dc_delta_q = 0;
   } else {
      result.base_qindex = pic->picture_parameter.base_qindex;
      result.y_dc_delta_q = pic->picture_parameter.y_dc_delta_q;
      result.uv_ac_delta_q = pic->picture_parameter.uv_ac_delta_q;
      result.uv_dc_delta_q = pic->picture_parameter.uv_dc_delta_q;
   }

   result.log2_tile_cols = pic->picture_parameter.log2_tile_columns;
   result.log2_tile_rows = pic->picture_parameter.log2_tile_rows;
   result.chroma_format = 1;
   result.bit_depth_luma_minus8 = result.bit_depth_chroma_minus8 =
      (pic->picture_parameter.bit_depth - 8);

   result.vp9_frame_size = align(dec->bs_size, 128);
   result.uncompressed_header_size = pic->picture_parameter.frame_header_length_in_bytes;
   result.compressed_header_size = pic->picture_parameter.first_partition_size;

   assert(dec->base.max_references + 1 <= ARRAY_SIZE(dec->render_pic_list));

   //clear the dec->render list if it is not used as a reference
   for (i = 0; i < ARRAY_SIZE(dec->render_pic_list); i++) {
      if (dec->render_pic_list[i]) {
         for (j=0;j<8;j++) {
            if (dec->render_pic_list[i] == pic->ref[j])
               break;
         }
         if (j == 8)
            dec->render_pic_list[i] = NULL;
      }
   }

   get_current_pic_index(dec, target, &result.curr_pic_idx);

   for (i = 0; i < 8; i++) {
      result.ref_frame_map[i] =
         (pic->ref[i]) ? (uintptr_t)vl_video_buffer_get_associated_data(pic->ref[i], &dec->base)
                       : 0x7f;
   }

   result.frame_refs[0] = result.ref_frame_map[pic->picture_parameter.pic_fields.last_ref_frame];
   result.ref_frame_sign_bias[0] = pic->picture_parameter.pic_fields.last_ref_frame_sign_bias;
   result.frame_refs[1] = result.ref_frame_map[pic->picture_parameter.pic_fields.golden_ref_frame];
   result.ref_frame_sign_bias[1] = pic->picture_parameter.pic_fields.golden_ref_frame_sign_bias;
   result.frame_refs[2] = result.ref_frame_map[pic->picture_parameter.pic_fields.alt_ref_frame];
   result.ref_frame_sign_bias[2] = pic->picture_parameter.pic_fields.alt_ref_frame_sign_bias;

   if (pic->base.profile == PIPE_VIDEO_PROFILE_VP9_PROFILE2) {
      if (target->buffer_format == PIPE_FORMAT_P010 || target->buffer_format == PIPE_FORMAT_P016) {
         result.p010_mode = 1;
         result.msb_mode = 1;
      } else {
         result.p010_mode = 0;
         result.luma_10to8 = 1;
         result.chroma_10to8 = 1;
      }
   }

   if (dec->dpb_type == DPB_DYNAMIC_TIER_2) {
      dec->ref_codec.bts = (pic->base.profile == PIPE_VIDEO_PROFILE_VP9_PROFILE2) ?
         CODEC_10_BITS : CODEC_8_BITS;
      dec->ref_codec.index = result.curr_pic_idx;
      dec->ref_codec.ref_size = 8;
      memset(dec->ref_codec.ref_list, 0x7f, sizeof(dec->ref_codec.ref_list));
      memcpy(dec->ref_codec.ref_list, result.ref_frame_map, sizeof(result.ref_frame_map));
   }

   dec->last_width = dec->base.width;
   dec->last_height = dec->base.height;

   return result;
}

static void get_h265_reflist(rvcn_dec_message_hevc_direct_ref_list_t *hevc_reflist,
                             struct pipe_h265_picture_desc *pic)
{
   hevc_reflist->num_direct_reflist = pic->slice_parameter.slice_count;
   for (int i = 0; i <hevc_reflist->num_direct_reflist; i++) {
      for (int j = 0; j < 2; j++) {
         for (int k = 0; k < 15; k++)
            hevc_reflist->multi_direct_reflist[i][j][k] = pic->RefPicList[i][j][k];
      }
   }
}

static void set_drm_keys(rvcn_dec_message_drm_t *drm, DECRYPT_PARAMETERS *decrypted)
{
   int cbc = decrypted->u.s.cbc;
   int ctr = decrypted->u.s.ctr;
   int id = decrypted->u.s.drm_id;
   int ekc = 1;
   int data1 = 1;
   int data2 = 1;

   drm->drm_cmd = 0;
   drm->drm_cntl = 0;

   drm->drm_cntl = 1 << DRM_CNTL_BYPASS_SHIFT;

   if (cbc || ctr) {
      drm->drm_cntl = 0 << DRM_CNTL_BYPASS_SHIFT;
      drm->drm_cmd |= 0xff << DRM_CMD_BYTE_MASK_SHIFT;

      if (ctr)
         drm->drm_cmd |= 0x00 << DRM_CMD_ALGORITHM_SHIFT;
      else if (cbc)
         drm->drm_cmd |= 0x02 << DRM_CMD_ALGORITHM_SHIFT;

      drm->drm_cmd |= 1 << DRM_CMD_GEN_MASK_SHIFT;
      drm->drm_cmd |= ekc << DRM_CMD_UNWRAP_KEY_SHIFT;
      drm->drm_cmd |= 0 << DRM_CMD_OFFSET_SHIFT;
      drm->drm_cmd |= data2 << DRM_CMD_CNT_DATA_SHIFT;
      drm->drm_cmd |= data1 << DRM_CMD_CNT_KEY_SHIFT;
      drm->drm_cmd |= ekc << DRM_CMD_KEY_SHIFT;
      drm->drm_cmd |= id << DRM_CMD_SESSION_SEL_SHIFT;

      if (ekc)
         memcpy(drm->drm_wrapped_key, decrypted->encrypted_key, 16);
      if (data1)
         memcpy(drm->drm_key, decrypted->session_iv, 16);
      if (data2)
         memcpy(drm->drm_counter, decrypted->encrypted_iv, 16);
      drm->drm_offset = 0;
   }
}

static int32_t rvcn_av1_film_grain_random_number(unsigned short *seed, int32_t bits)
{
   unsigned short bit;
   unsigned short value = *seed;

   bit = ((value >> 0) ^ (value >> 1) ^ (value >> 3) ^ (value >> 12)) & 1;
   value = (value >> 1) | (bit << 15);
   *seed = value;

   return (value >> (16 - bits)) & ((1 << bits) - 1);
}

static void rvcn_av1_film_grain_init_scaling(uint8_t scaling_points[][2],
                                             uint8_t num,
                                             short scaling_lut[])
{
   int32_t i, x, delta_x, delta_y;
   int64_t delta;

   if (num == 0)
      return;

   for ( i = 0; i < scaling_points[0][0]; i++ )
      scaling_lut[i] = scaling_points[0][1];

   for ( i = 0; i < num - 1; i++ ) {
      delta_y = scaling_points[i + 1][1] - scaling_points[i][1];
      delta_x = scaling_points[i + 1][0] - scaling_points[i][0];

      delta = delta_y * ((65536 + (delta_x >> 1)) / delta_x);

      for ( x = 0; x < delta_x; x++ )
         scaling_lut[scaling_points[i][0] + x] =
            (short)(scaling_points[i][1] + (int32_t)((x * delta + 32768) >> 16));
   }

   for ( i = scaling_points[num - 1][0]; i < 256; i++ )
      scaling_lut[i] = scaling_points[num - 1][1];
}

static void rvcn_av1_init_film_grain_buffer(rvcn_dec_film_grain_params_t *fg_params,
                                            rvcn_dec_av1_fg_init_buf_t *fg_buf)
{
   const int32_t luma_block_size_y = 73;
   const int32_t luma_block_size_x = 82;
   const int32_t chroma_block_size_y = 38;
   const int32_t chroma_block_size_x = 44;
   const int32_t gauss_bits = 11;
   int32_t filt_luma_grain_block[luma_block_size_y][luma_block_size_x];
   int32_t filt_cb_grain_block[chroma_block_size_y][chroma_block_size_x];
   int32_t filt_cr_grain_block[chroma_block_size_y][chroma_block_size_x];
   int32_t chroma_subsamp_y = 1;
   int32_t chroma_subsamp_x = 1;
   unsigned short seed = fg_params->random_seed;
   int32_t ar_coeff_lag = fg_params->ar_coeff_lag;
   int32_t bit_depth = fg_params->bit_depth_minus_8 + 8;
   short grain_center = 128 << (bit_depth - 8);
   short grain_min = 0 - grain_center;
   short grain_max = (256 << (bit_depth - 8)) - 1 - grain_center;
   int32_t shift = 12 - bit_depth + fg_params->grain_scale_shift;
   short luma_grain_block_tmp[64][80];
   short cb_grain_block_tmp[32][40];
   short cr_grain_block_tmp[32][40];
   short *align_ptr, *align_ptr0, *align_ptr1;
   int32_t x, y, g, i, j, c, c0, c1, delta_row, delta_col;
   int32_t s, s0, s1, pos, r;

   /* generate luma grain block */
   memset(filt_luma_grain_block, 0, sizeof(filt_luma_grain_block));
   for ( y = 0; y < luma_block_size_y; y++ ) {
      for ( x = 0; x < luma_block_size_x; x++ ) {
         g = 0;
         if (fg_params->num_y_points > 0) {
            r = rvcn_av1_film_grain_random_number(&seed, gauss_bits);
            g = gaussian_sequence[CLAMP(r, 0, 2048 - 1)];
         }
         filt_luma_grain_block[y][x] = ROUND_POWER_OF_TWO(g, shift);
      }
   }

   for ( y = 3; y < luma_block_size_y; y++ ) {
      for ( x = 3; x < luma_block_size_x - 3; x++ ) {
         s = 0;
         pos = 0;
         for (delta_row = -ar_coeff_lag; delta_row <= 0; delta_row++) {
            for (delta_col = -ar_coeff_lag; delta_col <= ar_coeff_lag; delta_col++) {
               if (delta_row == 0 && delta_col == 0)
                  break;
               c = fg_params->ar_coeffs_y[pos];
               s += filt_luma_grain_block[y + delta_row][x + delta_col] * c;
               pos++;
            }
         }
         filt_luma_grain_block[y][x] =
            AV1_CLAMP(filt_luma_grain_block[y][x]
                      + ROUND_POWER_OF_TWO(s, fg_params->ar_coeff_shift),
                      grain_min, grain_max);
      }
   }

   /* generate chroma grain block */
   memset(filt_cb_grain_block, 0, sizeof(filt_cb_grain_block));
   shift = 12 - bit_depth + fg_params->grain_scale_shift;
   seed = fg_params->random_seed ^ 0xb524;
   for (y = 0; y < chroma_block_size_y; y++) {
      for (x = 0; x < chroma_block_size_x; x++) {
         g = 0;
         if (fg_params->num_cb_points || fg_params->chroma_scaling_from_luma) {
            r = rvcn_av1_film_grain_random_number(&seed, gauss_bits);
            g = gaussian_sequence[CLAMP(r, 0, 2048 - 1)];
         }
         filt_cb_grain_block[y][x] = ROUND_POWER_OF_TWO(g, shift);
      }
   }

   memset(filt_cr_grain_block, 0, sizeof(filt_cr_grain_block));
   seed = fg_params->random_seed ^ 0x49d8;
   for (y = 0; y < chroma_block_size_y; y++) {
      for (x = 0; x < chroma_block_size_x; x++) {
         g = 0;
         if (fg_params->num_cr_points || fg_params->chroma_scaling_from_luma) {
            r = rvcn_av1_film_grain_random_number(&seed, gauss_bits);
            g = gaussian_sequence[CLAMP(r, 0, 2048 - 1)];
         }
         filt_cr_grain_block[y][x] = ROUND_POWER_OF_TWO(g, shift);
      }
   }

   for (y = 3; y < chroma_block_size_y; y++) {
      for (x = 3; x < chroma_block_size_x - 3; x++) {
         s0 = 0, s1 = 0, pos = 0;
         for (delta_row = -ar_coeff_lag; delta_row <= 0; delta_row++) {
            for (delta_col = -ar_coeff_lag; delta_col <= ar_coeff_lag; delta_col++) {
               c0 = fg_params->ar_coeffs_cb[pos];
               c1 = fg_params->ar_coeffs_cr[pos];
               if (delta_row == 0 && delta_col == 0) {
                  if (fg_params->num_y_points > 0) {
                     int luma = 0;
                     int luma_x = ((x - 3) << chroma_subsamp_x) + 3;
                     int luma_y = ((y - 3) << chroma_subsamp_y) + 3;
                     for ( i = 0; i <= chroma_subsamp_y; i++)
                        for ( j = 0; j <= chroma_subsamp_x; j++)
                           luma += filt_luma_grain_block[luma_y + i][luma_x + j];

                     luma = ROUND_POWER_OF_TWO(luma, chroma_subsamp_x + chroma_subsamp_y);
                     s0 += luma * c0;
                     s1 += luma * c1;
                  }
                  break;
               }
               s0 += filt_cb_grain_block[y + delta_row][x + delta_col] * c0;
               s1 += filt_cr_grain_block[y + delta_row][x + delta_col] * c1;
               pos++;
            }
         }
         filt_cb_grain_block[y][x] = AV1_CLAMP(filt_cb_grain_block[y][x] +
                                       ROUND_POWER_OF_TWO(s0, fg_params->ar_coeff_shift),
                                     grain_min, grain_max);
         filt_cr_grain_block[y][x] = AV1_CLAMP(filt_cr_grain_block[y][x] +
                                       ROUND_POWER_OF_TWO(s1, fg_params->ar_coeff_shift),
                                     grain_min, grain_max);
      }
   }

   for ( i = 9; i < luma_block_size_y; i++ )
      for ( j = 9; j < luma_block_size_x; j++ )
         luma_grain_block_tmp[i - 9][j - 9] = filt_luma_grain_block[i][j];

   for ( i = 6; i < chroma_block_size_y; i++ )
      for ( j = 6; j < chroma_block_size_x; j++ ) {
         cb_grain_block_tmp[i - 6][j - 6] = filt_cb_grain_block[i][j];
         cr_grain_block_tmp[i - 6][j - 6] = filt_cr_grain_block[i][j];
      }

   align_ptr = &fg_buf->luma_grain_block[0][0];
   for ( i = 0; i < 64; i++ ) {
      for ( j = 0; j < 80; j++)
         *align_ptr++ = luma_grain_block_tmp[i][j];

      if (((i + 1) % 4) == 0)
         align_ptr += 64;
   }

   align_ptr0 = &fg_buf->cb_grain_block[0][0];
   align_ptr1 = &fg_buf->cr_grain_block[0][0];
   for ( i = 0; i < 32; i++) {
      for ( j = 0; j < 40; j++) {
         *align_ptr0++ = cb_grain_block_tmp[i][j];
         *align_ptr1++ = cr_grain_block_tmp[i][j];
      }
      if (((i + 1) % 8) == 0) {
         align_ptr0 += 64;
         align_ptr1 += 64;
      }
   }

   memset(fg_buf->scaling_lut_y, 0, sizeof(fg_buf->scaling_lut_y));
   rvcn_av1_film_grain_init_scaling(fg_params->scaling_points_y, fg_params->num_y_points,
                                    fg_buf->scaling_lut_y);
   if (fg_params->chroma_scaling_from_luma) {
      memcpy(fg_buf->scaling_lut_cb, fg_buf->scaling_lut_y, sizeof(fg_buf->scaling_lut_y));
      memcpy(fg_buf->scaling_lut_cr, fg_buf->scaling_lut_y, sizeof(fg_buf->scaling_lut_y));
   } else {
      memset(fg_buf->scaling_lut_cb, 0, sizeof(fg_buf->scaling_lut_cb));
      memset(fg_buf->scaling_lut_cr, 0, sizeof(fg_buf->scaling_lut_cr));
      rvcn_av1_film_grain_init_scaling(fg_params->scaling_points_cb, fg_params->num_cb_points,
                                       fg_buf->scaling_lut_cb);
      rvcn_av1_film_grain_init_scaling(fg_params->scaling_points_cr, fg_params->num_cr_points,
                                       fg_buf->scaling_lut_cr);
   }
}

static void rvcn_dec_av1_film_grain_surface(struct pipe_video_buffer **target,
                                            struct pipe_av1_picture_desc *pic)
{
   if (!pic->picture_parameter.film_grain_info.film_grain_info_fields.apply_grain ||
       !pic->film_grain_target)
      return;

   *target = pic->film_grain_target;
}

static rvcn_dec_message_av1_t get_av1_msg(struct radeon_decoder *dec,
                                          struct pipe_video_buffer *target,
                                          struct pipe_av1_picture_desc *pic)
{
   rvcn_dec_message_av1_t result;
   unsigned i, j;
   uint16_t tile_count = pic->picture_parameter.tile_cols * pic->picture_parameter.tile_rows;

   memset(&result, 0, sizeof(result));

   result.frame_header_flags = (pic->picture_parameter.pic_info_fields.show_frame
                                << RDECODE_FRAME_HDR_INFO_AV1_SHOW_FRAME_SHIFT) &
                                RDECODE_FRAME_HDR_INFO_AV1_SHOW_FRAME_MASK;

   result.frame_header_flags |= (pic->picture_parameter.pic_info_fields.disable_cdf_update
                                 << RDECODE_FRAME_HDR_INFO_AV1_DISABLE_CDF_UPDATE_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_DISABLE_CDF_UPDATE_MASK;

   result.frame_header_flags |= ((!pic->picture_parameter.pic_info_fields.disable_frame_end_update_cdf)
                                 << RDECODE_FRAME_HDR_INFO_AV1_REFRESH_FRAME_CONTEXT_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_REFRESH_FRAME_CONTEXT_MASK;

   result.frame_header_flags |= ((pic->picture_parameter.pic_info_fields.frame_type ==
                                 2 /* INTRA_ONLY_FRAME */) << RDECODE_FRAME_HDR_INFO_AV1_INTRA_ONLY_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_INTRA_ONLY_MASK;

   result.frame_header_flags |= (pic->picture_parameter.pic_info_fields.allow_intrabc
                                 << RDECODE_FRAME_HDR_INFO_AV1_ALLOW_INTRABC_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_ALLOW_INTRABC_MASK;

   result.frame_header_flags |= (pic->picture_parameter.pic_info_fields.allow_high_precision_mv
                                 << RDECODE_FRAME_HDR_INFO_AV1_ALLOW_HIGH_PRECISION_MV_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_ALLOW_HIGH_PRECISION_MV_MASK;

   result.frame_header_flags |= (pic->picture_parameter.seq_info_fields.mono_chrome
                                 << RDECODE_FRAME_HDR_INFO_AV1_MONOCHROME_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_MONOCHROME_MASK;

   result.frame_header_flags |= (pic->picture_parameter.mode_control_fields.skip_mode_present
                                 << RDECODE_FRAME_HDR_INFO_AV1_SKIP_MODE_FLAG_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_SKIP_MODE_FLAG_MASK;

   result.frame_header_flags |= (((pic->picture_parameter.qmatrix_fields.qm_y == 0xf) ? 0 : 1)
                                 << RDECODE_FRAME_HDR_INFO_AV1_USING_QMATRIX_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_USING_QMATRIX_MASK;

   result.frame_header_flags |= (pic->picture_parameter.seq_info_fields.enable_filter_intra
                                 << RDECODE_FRAME_HDR_INFO_AV1_ENABLE_FILTER_INTRA_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_ENABLE_FILTER_INTRA_MASK;

   result.frame_header_flags |= (pic->picture_parameter.seq_info_fields.enable_intra_edge_filter
                                 << RDECODE_FRAME_HDR_INFO_AV1_ENABLE_INTRA_EDGE_FILTER_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_ENABLE_INTRA_EDGE_FILTER_MASK;

   result.frame_header_flags |= (pic->picture_parameter.seq_info_fields.enable_interintra_compound
                                 << RDECODE_FRAME_HDR_INFO_AV1_ENABLE_INTERINTRA_COMPOUND_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_ENABLE_INTERINTRA_COMPOUND_MASK;

   result.frame_header_flags |= (pic->picture_parameter.seq_info_fields.enable_masked_compound
                                 << RDECODE_FRAME_HDR_INFO_AV1_ENABLE_MASKED_COMPOUND_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_ENABLE_MASKED_COMPOUND_MASK;

   result.frame_header_flags |= (pic->picture_parameter.pic_info_fields.allow_warped_motion
                                 << RDECODE_FRAME_HDR_INFO_AV1_ALLOW_WARPED_MOTION_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_ALLOW_WARPED_MOTION_MASK;

   result.frame_header_flags |= (pic->picture_parameter.seq_info_fields.enable_dual_filter
                                 << RDECODE_FRAME_HDR_INFO_AV1_ENABLE_DUAL_FILTER_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_ENABLE_DUAL_FILTER_MASK;

   result.frame_header_flags |= (pic->picture_parameter.seq_info_fields.enable_order_hint
                                 << RDECODE_FRAME_HDR_INFO_AV1_ENABLE_ORDER_HINT_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_ENABLE_ORDER_HINT_MASK;

   result.frame_header_flags |= (pic->picture_parameter.seq_info_fields.enable_jnt_comp
                                 << RDECODE_FRAME_HDR_INFO_AV1_ENABLE_JNT_COMP_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_ENABLE_JNT_COMP_MASK;

   result.frame_header_flags |= (pic->picture_parameter.pic_info_fields.use_ref_frame_mvs
                                 << RDECODE_FRAME_HDR_INFO_AV1_ALLOW_REF_FRAME_MVS_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_ALLOW_REF_FRAME_MVS_MASK;

   result.frame_header_flags |= (pic->picture_parameter.pic_info_fields.allow_screen_content_tools
                                 << RDECODE_FRAME_HDR_INFO_AV1_ALLOW_SCREEN_CONTENT_TOOLS_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_ALLOW_SCREEN_CONTENT_TOOLS_MASK;

   result.frame_header_flags |= (pic->picture_parameter.pic_info_fields.force_integer_mv
                                 << RDECODE_FRAME_HDR_INFO_AV1_CUR_FRAME_FORCE_INTEGER_MV_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_CUR_FRAME_FORCE_INTEGER_MV_MASK;

   result.frame_header_flags |= (pic->picture_parameter.loop_filter_info_fields.mode_ref_delta_enabled
                                 << RDECODE_FRAME_HDR_INFO_AV1_MODE_REF_DELTA_ENABLED_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_MODE_REF_DELTA_ENABLED_MASK;

   result.frame_header_flags |= (pic->picture_parameter.loop_filter_info_fields.mode_ref_delta_update
                                 << RDECODE_FRAME_HDR_INFO_AV1_MODE_REF_DELTA_UPDATE_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_MODE_REF_DELTA_UPDATE_MASK;

   result.frame_header_flags |= (pic->picture_parameter.mode_control_fields.delta_q_present_flag
                                 << RDECODE_FRAME_HDR_INFO_AV1_DELTA_Q_PRESENT_FLAG_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_DELTA_Q_PRESENT_FLAG_MASK;

   result.frame_header_flags |= (pic->picture_parameter.mode_control_fields.delta_lf_present_flag
                                 << RDECODE_FRAME_HDR_INFO_AV1_DELTA_LF_PRESENT_FLAG_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_DELTA_LF_PRESENT_FLAG_MASK;

   result.frame_header_flags |= (pic->picture_parameter.mode_control_fields.reduced_tx_set_used
                                 << RDECODE_FRAME_HDR_INFO_AV1_REDUCED_TX_SET_USED_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_REDUCED_TX_SET_USED_MASK;

   result.frame_header_flags |= (pic->picture_parameter.seg_info.segment_info_fields.enabled
                                 << RDECODE_FRAME_HDR_INFO_AV1_SEGMENTATION_ENABLED_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_SEGMENTATION_ENABLED_MASK;

   result.frame_header_flags |= (pic->picture_parameter.seg_info.segment_info_fields.update_map
                                 << RDECODE_FRAME_HDR_INFO_AV1_SEGMENTATION_UPDATE_MAP_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_SEGMENTATION_UPDATE_MAP_MASK;

   result.frame_header_flags |= (pic->picture_parameter.seg_info.segment_info_fields.temporal_update
                                 << RDECODE_FRAME_HDR_INFO_AV1_SEGMENTATION_TEMPORAL_UPDATE_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_SEGMENTATION_TEMPORAL_UPDATE_MASK;

   result.frame_header_flags |= (pic->picture_parameter.mode_control_fields.delta_lf_multi
                                 << RDECODE_FRAME_HDR_INFO_AV1_DELTA_LF_MULTI_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_DELTA_LF_MULTI_MASK;

   result.frame_header_flags |= (pic->picture_parameter.pic_info_fields.is_motion_mode_switchable
                                 << RDECODE_FRAME_HDR_INFO_AV1_SWITCHABLE_SKIP_MODE_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_SWITCHABLE_SKIP_MODE_MASK;

   result.frame_header_flags |= ((!pic->picture_parameter.refresh_frame_flags)
                                 << RDECODE_FRAME_HDR_INFO_AV1_SKIP_REFERENCE_UPDATE_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_SKIP_REFERENCE_UPDATE_MASK;

   result.frame_header_flags |= ((!pic->picture_parameter.seq_info_fields.ref_frame_mvs)
                                 << RDECODE_FRAME_HDR_INFO_AV1_DISABLE_REF_FRAME_MVS_SHIFT) &
                                 RDECODE_FRAME_HDR_INFO_AV1_DISABLE_REF_FRAME_MVS_MASK;

   result.current_frame_id = pic->picture_parameter.current_frame_id;
   result.frame_offset = pic->picture_parameter.order_hint;

   result.profile = pic->picture_parameter.profile;
   result.is_annexb = 0;
   result.frame_type = pic->picture_parameter.pic_info_fields.frame_type;
   result.primary_ref_frame = pic->picture_parameter.primary_ref_frame;

   get_current_pic_index(dec, target, &result.curr_pic_idx);

   result.sb_size = pic->picture_parameter.seq_info_fields.use_128x128_superblock;
   result.interp_filter = pic->picture_parameter.interp_filter;
   for (i = 0; i < 2; ++i)
      result.filter_level[i] = pic->picture_parameter.filter_level[i];
   result.filter_level_u = pic->picture_parameter.filter_level_u;
   result.filter_level_v = pic->picture_parameter.filter_level_v;
   result.sharpness_level = pic->picture_parameter.loop_filter_info_fields.sharpness_level;
   for (i = 0; i < 8; ++i)
      result.ref_deltas[i] = pic->picture_parameter.ref_deltas[i];
   for (i = 0; i < 2; ++i)
      result.mode_deltas[i] = pic->picture_parameter.mode_deltas[i];
   result.base_qindex = pic->picture_parameter.base_qindex;
   result.y_dc_delta_q = pic->picture_parameter.y_dc_delta_q;
   result.u_dc_delta_q = pic->picture_parameter.u_dc_delta_q;
   result.v_dc_delta_q = pic->picture_parameter.v_dc_delta_q;
   result.u_ac_delta_q = pic->picture_parameter.u_ac_delta_q;
   result.v_ac_delta_q = pic->picture_parameter.v_ac_delta_q;
   result.qm_y = pic->picture_parameter.qmatrix_fields.qm_y | 0xf0;
   result.qm_u = pic->picture_parameter.qmatrix_fields.qm_u | 0xf0;
   result.qm_v = pic->picture_parameter.qmatrix_fields.qm_v | 0xf0;
   result.delta_q_res = 1 << pic->picture_parameter.mode_control_fields.log2_delta_q_res;
   result.delta_lf_res = 1 << pic->picture_parameter.mode_control_fields.log2_delta_lf_res;

   result.tile_cols = pic->picture_parameter.tile_cols;
   result.tile_rows = pic->picture_parameter.tile_rows;
   result.tx_mode = pic->picture_parameter.mode_control_fields.tx_mode;
   result.reference_mode = (pic->picture_parameter.mode_control_fields.reference_select == 1) ? 2 : 0;
   result.chroma_format = pic->picture_parameter.seq_info_fields.mono_chrome ? 0 : 1;
   result.tile_size_bytes = 0xff;
   result.context_update_tile_id = pic->picture_parameter.context_update_tile_id;
   for (i = 0; i < 65; ++i) {
      result.tile_col_start_sb[i] = pic->picture_parameter.tile_col_start_sb[i];
      result.tile_row_start_sb[i] = pic->picture_parameter.tile_row_start_sb[i];
   }
   result.max_width = pic->picture_parameter.max_width;
   result.max_height = pic->picture_parameter.max_height;
   if (pic->picture_parameter.pic_info_fields.use_superres) {
      result.width = (pic->picture_parameter.frame_width * 8 + pic->picture_parameter.superres_scale_denominator / 2) /
         pic->picture_parameter.superres_scale_denominator;
      result.superres_scale_denominator = pic->picture_parameter.superres_scale_denominator;
   } else {
      result.width = pic->picture_parameter.frame_width;
      result.superres_scale_denominator = pic->picture_parameter.superres_scale_denominator;
   }
   result.height = pic->picture_parameter.frame_height;
   result.superres_upscaled_width = pic->picture_parameter.frame_width;
   result.order_hint_bits = pic->picture_parameter.order_hint_bits_minus_1 + 1;

   for (i = 0; i < NUM_AV1_REFS; ++i) {
      result.ref_frame_map[i] =
         (pic->ref[i]) ? (uintptr_t)vl_video_buffer_get_associated_data(pic->ref[i], &dec->base)
                       : 0x7f;
   }
   for (i = 0; i < NUM_AV1_REFS_PER_FRAME; ++i)
       result.frame_refs[i] = result.ref_frame_map[pic->picture_parameter.ref_frame_idx[i]];

   result.bit_depth_luma_minus8 = result.bit_depth_chroma_minus8 = pic->picture_parameter.bit_depth_idx << 1;

   for (i = 0; i < 8; ++i) {
      for (j = 0; j < 8; ++j)
         result.feature_data[i][j] = pic->picture_parameter.seg_info.feature_data[i][j];
      result.feature_mask[i] = pic->picture_parameter.seg_info.feature_mask[i];
   }
   memcpy(dec->probs, &pic->picture_parameter.seg_info.feature_data, 128);
   memcpy((dec->probs + 128), &pic->picture_parameter.seg_info.feature_mask, 8);

   result.cdef_damping = pic->picture_parameter.cdef_damping_minus_3 + 3;
   result.cdef_bits = pic->picture_parameter.cdef_bits;
   for (i = 0; i < 8; ++i) {
      result.cdef_strengths[i] = pic->picture_parameter.cdef_y_strengths[i];
      result.cdef_uv_strengths[i] = pic->picture_parameter.cdef_uv_strengths[i];
   }
   result.frame_restoration_type[0] = pic->picture_parameter.loop_restoration_fields.yframe_restoration_type;
   result.frame_restoration_type[1] = pic->picture_parameter.loop_restoration_fields.cbframe_restoration_type;
   result.frame_restoration_type[2] = pic->picture_parameter.loop_restoration_fields.crframe_restoration_type;
   for (i = 0; i < 3; ++i) {
      int log2_num = 0;
      int unit_size = pic->picture_parameter.lr_unit_size[i];
      if (unit_size) {
         while (unit_size >>= 1)
            log2_num++;
         result.log2_restoration_unit_size_minus5[i] = log2_num - 5;
      } else {
         result.log2_restoration_unit_size_minus5[i] = 0;
      }
   }

   if (pic->picture_parameter.bit_depth_idx) {
      if (target->buffer_format == PIPE_FORMAT_P010 || target->buffer_format == PIPE_FORMAT_P016) {
         result.p010_mode = 1;
         result.msb_mode = 1;
      } else {
         result.luma_10to8 = 1;
         result.chroma_10to8 = 1;
      }
   }

   result.preskip_segid = 0;
   result.last_active_segid = 0;
   for (i = 0; i < 8; i++) {
      for (j = 0; j < 8; j++) {
         if (pic->picture_parameter.seg_info.feature_mask[i] & (1 << j)) {
            result.last_active_segid = i;
            if (j >= 5)
               result.preskip_segid = 1;
         }
      }
   }

   result.seg_lossless_flag = 0;
   for (i = 0; i < 8; ++i) {
      int av1_get_qindex, qindex;
      int segfeature_active = pic->picture_parameter.seg_info.feature_mask[i] & (1 << 0);
      if (segfeature_active) {
         int seg_qindex = pic->picture_parameter.base_qindex +
                          pic->picture_parameter.seg_info.feature_data[i][0];
         av1_get_qindex = seg_qindex < 0 ? 0 : (seg_qindex > 255 ? 255 : seg_qindex);
      } else {
         av1_get_qindex = pic->picture_parameter.base_qindex;
      }
      qindex = pic->picture_parameter.seg_info.segment_info_fields.enabled ?
               av1_get_qindex :
               pic->picture_parameter.base_qindex;
      result.seg_lossless_flag |= (((qindex == 0) && result.y_dc_delta_q == 0 &&
                                    result.u_dc_delta_q == 0 && result.v_dc_delta_q == 0 &&
                                    result.u_ac_delta_q == 0 && result.v_ac_delta_q == 0) << i);
   }

   rvcn_dec_film_grain_params_t* fg_params = &result.film_grain;
   fg_params->apply_grain = pic->picture_parameter.film_grain_info.film_grain_info_fields.apply_grain;
   if (fg_params->apply_grain) {
      rvcn_dec_av1_fg_init_buf_t *fg_buf = (rvcn_dec_av1_fg_init_buf_t *)(dec->probs + 256);

      fg_params->random_seed = pic->picture_parameter.film_grain_info.grain_seed;
      fg_params->grain_scale_shift =
         pic->picture_parameter.film_grain_info.film_grain_info_fields.grain_scale_shift;
      fg_params->scaling_shift =
         pic->picture_parameter.film_grain_info.film_grain_info_fields.grain_scaling_minus_8 + 8;
      fg_params->chroma_scaling_from_luma =
         pic->picture_parameter.film_grain_info.film_grain_info_fields.chroma_scaling_from_luma;
      fg_params->num_y_points = pic->picture_parameter.film_grain_info.num_y_points;
      fg_params->num_cb_points = pic->picture_parameter.film_grain_info.num_cb_points;
      fg_params->num_cr_points = pic->picture_parameter.film_grain_info.num_cr_points;
      fg_params->cb_mult = pic->picture_parameter.film_grain_info.cb_mult;
      fg_params->cb_luma_mult = pic->picture_parameter.film_grain_info.cb_luma_mult;
      fg_params->cb_offset = pic->picture_parameter.film_grain_info.cb_offset;
      fg_params->cr_mult = pic->picture_parameter.film_grain_info.cr_mult;
      fg_params->cr_luma_mult = pic->picture_parameter.film_grain_info.cr_luma_mult;
      fg_params->cr_offset = pic->picture_parameter.film_grain_info.cr_offset;
      fg_params->bit_depth_minus_8 = pic->picture_parameter.bit_depth_idx << 1;

      for (i = 0; i < fg_params->num_y_points; ++i) {
         fg_params->scaling_points_y[i][0] = pic->picture_parameter.film_grain_info.point_y_value[i];
         fg_params->scaling_points_y[i][1] = pic->picture_parameter.film_grain_info.point_y_scaling[i];
      }
      for (i = 0; i < fg_params->num_cb_points; ++i) {
         fg_params->scaling_points_cb[i][0] = pic->picture_parameter.film_grain_info.point_cb_value[i];
         fg_params->scaling_points_cb[i][1] = pic->picture_parameter.film_grain_info.point_cb_scaling[i];
      }
      for (i = 0; i < fg_params->num_cr_points; ++i) {
         fg_params->scaling_points_cr[i][0] = pic->picture_parameter.film_grain_info.point_cr_value[i];
         fg_params->scaling_points_cr[i][1] = pic->picture_parameter.film_grain_info.point_cr_scaling[i];
      }

      fg_params->ar_coeff_lag = pic->picture_parameter.film_grain_info.film_grain_info_fields.ar_coeff_lag;
      fg_params->ar_coeff_shift =
         pic->picture_parameter.film_grain_info.film_grain_info_fields.ar_coeff_shift_minus_6 + 6;

      for (i = 0; i < 24; ++i)
         fg_params->ar_coeffs_y[i] = pic->picture_parameter.film_grain_info.ar_coeffs_y[i];

      for (i = 0; i < 25; ++i) {
         fg_params->ar_coeffs_cb[i] = pic->picture_parameter.film_grain_info.ar_coeffs_cb[i];
         fg_params->ar_coeffs_cr[i] = pic->picture_parameter.film_grain_info.ar_coeffs_cr[i];
      }

      fg_params->overlap_flag = pic->picture_parameter.film_grain_info.film_grain_info_fields.overlap_flag;
      fg_params->clip_to_restricted_range =
         pic->picture_parameter.film_grain_info.film_grain_info_fields.clip_to_restricted_range;

      rvcn_av1_init_film_grain_buffer(fg_params, fg_buf);
   }

   result.uncompressed_header_size = 0;
   for (i = 0; i < 7; ++i) {
      result.global_motion[i + 1].wmtype = (rvcn_dec_transformation_type_e)pic->picture_parameter.wm[i].wmtype;
      for (j = 0; j < 6; ++j)
         result.global_motion[i + 1].wmmat[j] = pic->picture_parameter.wm[i].wmmat[j];
   }
   for (i = 0; i < tile_count && i < 256; ++i) {
      result.tile_info[i].offset = pic->slice_parameter.slice_data_offset[i];
      result.tile_info[i].size = pic->slice_parameter.slice_data_size[i];
   }

   if (dec->dpb_type == DPB_DYNAMIC_TIER_2) {
      dec->ref_codec.bts = pic->picture_parameter.bit_depth_idx ? CODEC_10_BITS : CODEC_8_BITS;
      dec->ref_codec.index = result.curr_pic_idx;
      dec->ref_codec.ref_size = 8;
      memset(dec->ref_codec.ref_list, 0x7f, sizeof(dec->ref_codec.ref_list));
      memcpy(dec->ref_codec.ref_list, result.ref_frame_map, sizeof(result.ref_frame_map));
   }

   return result;
}

static void rvcn_init_mode_probs(void *prob)
{
   rvcn_av1_frame_context_t * fc = (rvcn_av1_frame_context_t*)prob;
   int i;

   memcpy(fc->palette_y_size_cdf, default_palette_y_size_cdf, sizeof(default_palette_y_size_cdf));
   memcpy(fc->palette_uv_size_cdf, default_palette_uv_size_cdf, sizeof(default_palette_uv_size_cdf));
   memcpy(fc->palette_y_color_index_cdf, default_palette_y_color_index_cdf, sizeof(default_palette_y_color_index_cdf));
   memcpy(fc->palette_uv_color_index_cdf, default_palette_uv_color_index_cdf, sizeof(default_palette_uv_color_index_cdf));
   memcpy(fc->kf_y_cdf, default_kf_y_mode_cdf, sizeof(default_kf_y_mode_cdf));
   memcpy(fc->angle_delta_cdf, default_angle_delta_cdf, sizeof(default_angle_delta_cdf));
   memcpy(fc->comp_inter_cdf, default_comp_inter_cdf, sizeof(default_comp_inter_cdf));
   memcpy(fc->comp_ref_type_cdf, default_comp_ref_type_cdf,sizeof(default_comp_ref_type_cdf));
   memcpy(fc->uni_comp_ref_cdf, default_uni_comp_ref_cdf, sizeof(default_uni_comp_ref_cdf));
   memcpy(fc->palette_y_mode_cdf, default_palette_y_mode_cdf, sizeof(default_palette_y_mode_cdf));
   memcpy(fc->palette_uv_mode_cdf, default_palette_uv_mode_cdf, sizeof(default_palette_uv_mode_cdf));
   memcpy(fc->comp_ref_cdf, default_comp_ref_cdf, sizeof(default_comp_ref_cdf));
   memcpy(fc->comp_bwdref_cdf, default_comp_bwdref_cdf, sizeof(default_comp_bwdref_cdf));
   memcpy(fc->single_ref_cdf, default_single_ref_cdf, sizeof(default_single_ref_cdf));
   memcpy(fc->txfm_partition_cdf, default_txfm_partition_cdf, sizeof(default_txfm_partition_cdf));
   memcpy(fc->compound_index_cdf, default_compound_idx_cdfs, sizeof(default_compound_idx_cdfs));
   memcpy(fc->comp_group_idx_cdf, default_comp_group_idx_cdfs, sizeof(default_comp_group_idx_cdfs));
   memcpy(fc->newmv_cdf, default_newmv_cdf, sizeof(default_newmv_cdf));
   memcpy(fc->zeromv_cdf, default_zeromv_cdf, sizeof(default_zeromv_cdf));
   memcpy(fc->refmv_cdf, default_refmv_cdf, sizeof(default_refmv_cdf));
   memcpy(fc->drl_cdf, default_drl_cdf, sizeof(default_drl_cdf));
   memcpy(fc->motion_mode_cdf, default_motion_mode_cdf, sizeof(default_motion_mode_cdf));
   memcpy(fc->obmc_cdf, default_obmc_cdf, sizeof(default_obmc_cdf));
   memcpy(fc->inter_compound_mode_cdf, default_inter_compound_mode_cdf, sizeof(default_inter_compound_mode_cdf));
   memcpy(fc->compound_type_cdf, default_compound_type_cdf, sizeof(default_compound_type_cdf));
   memcpy(fc->wedge_idx_cdf, default_wedge_idx_cdf, sizeof(default_wedge_idx_cdf));
   memcpy(fc->interintra_cdf, default_interintra_cdf, sizeof(default_interintra_cdf));
   memcpy(fc->wedge_interintra_cdf, default_wedge_interintra_cdf, sizeof(default_wedge_interintra_cdf));
   memcpy(fc->interintra_mode_cdf, default_interintra_mode_cdf, sizeof(default_interintra_mode_cdf));
   memcpy(fc->pred_cdf, default_segment_pred_cdf, sizeof(default_segment_pred_cdf));
   memcpy(fc->switchable_restore_cdf, default_switchable_restore_cdf, sizeof(default_switchable_restore_cdf));
   memcpy(fc->wiener_restore_cdf, default_wiener_restore_cdf, sizeof(default_wiener_restore_cdf));
   memcpy(fc->sgrproj_restore_cdf, default_sgrproj_restore_cdf, sizeof(default_sgrproj_restore_cdf));
   memcpy(fc->y_mode_cdf, default_if_y_mode_cdf, sizeof(default_if_y_mode_cdf));
   memcpy(fc->uv_mode_cdf, default_uv_mode_cdf, sizeof(default_uv_mode_cdf));
   memcpy(fc->switchable_interp_cdf, default_switchable_interp_cdf, sizeof(default_switchable_interp_cdf));
   memcpy(fc->partition_cdf, default_partition_cdf, sizeof(default_partition_cdf));
   memcpy(fc->intra_ext_tx_cdf, default_intra_ext_tx_cdf, sizeof(default_intra_ext_tx_cdf));
   memcpy(fc->inter_ext_tx_cdf, default_inter_ext_tx_cdf, sizeof(default_inter_ext_tx_cdf));
   memcpy(fc->skip_cdfs, default_skip_cdfs, sizeof(default_skip_cdfs));
   memcpy(fc->intra_inter_cdf, default_intra_inter_cdf, sizeof(default_intra_inter_cdf));
   memcpy(fc->tree_cdf, default_seg_tree_cdf, sizeof(default_seg_tree_cdf));
   for (i = 0; i < SPATIAL_PREDICTION_PROBS; ++i)
      memcpy(fc->spatial_pred_seg_cdf[i], default_spatial_pred_seg_tree_cdf[i], sizeof(default_spatial_pred_seg_tree_cdf[i]));
   memcpy(fc->tx_size_cdf, default_tx_size_cdf, sizeof(default_tx_size_cdf));
   memcpy(fc->delta_q_cdf, default_delta_q_cdf, sizeof(default_delta_q_cdf));
   memcpy(fc->skip_mode_cdfs, default_skip_mode_cdfs, sizeof(default_skip_mode_cdfs));
   memcpy(fc->delta_lf_cdf, default_delta_lf_cdf, sizeof(default_delta_lf_cdf));
   memcpy(fc->delta_lf_multi_cdf, default_delta_lf_multi_cdf, sizeof(default_delta_lf_multi_cdf));
   memcpy(fc->cfl_sign_cdf, default_cfl_sign_cdf, sizeof(default_cfl_sign_cdf));
   memcpy(fc->cfl_alpha_cdf, default_cfl_alpha_cdf, sizeof(default_cfl_alpha_cdf));
   memcpy(fc->filter_intra_cdfs, default_filter_intra_cdfs, sizeof(default_filter_intra_cdfs));
   memcpy(fc->filter_intra_mode_cdf, default_filter_intra_mode_cdf, sizeof(default_filter_intra_mode_cdf));
   memcpy(fc->intrabc_cdf, default_intrabc_cdf, sizeof(default_intrabc_cdf));
}

static void rvcn_vcn4_init_mode_probs(void *prob)
{
   rvcn_av1_vcn4_frame_context_t * fc = (rvcn_av1_vcn4_frame_context_t*)prob;
   int i;

   memcpy(fc->palette_y_size_cdf, default_palette_y_size_cdf, sizeof(default_palette_y_size_cdf));
   memcpy(fc->palette_uv_size_cdf, default_palette_uv_size_cdf, sizeof(default_palette_uv_size_cdf));
   memcpy(fc->palette_y_color_index_cdf, default_palette_y_color_index_cdf, sizeof(default_palette_y_color_index_cdf));
   memcpy(fc->palette_uv_color_index_cdf, default_palette_uv_color_index_cdf, sizeof(default_palette_uv_color_index_cdf));
   memcpy(fc->kf_y_cdf, default_kf_y_mode_cdf, sizeof(default_kf_y_mode_cdf));
   memcpy(fc->angle_delta_cdf, default_angle_delta_cdf, sizeof(default_angle_delta_cdf));
   memcpy(fc->comp_inter_cdf, default_comp_inter_cdf, sizeof(default_comp_inter_cdf));
   memcpy(fc->comp_ref_type_cdf, default_comp_ref_type_cdf,sizeof(default_comp_ref_type_cdf));
   memcpy(fc->uni_comp_ref_cdf, default_uni_comp_ref_cdf, sizeof(default_uni_comp_ref_cdf));
   memcpy(fc->palette_y_mode_cdf, default_palette_y_mode_cdf, sizeof(default_palette_y_mode_cdf));
   memcpy(fc->palette_uv_mode_cdf, default_palette_uv_mode_cdf, sizeof(default_palette_uv_mode_cdf));
   memcpy(fc->comp_ref_cdf, default_comp_ref_cdf, sizeof(default_comp_ref_cdf));
   memcpy(fc->comp_bwdref_cdf, default_comp_bwdref_cdf, sizeof(default_comp_bwdref_cdf));
   memcpy(fc->single_ref_cdf, default_single_ref_cdf, sizeof(default_single_ref_cdf));
   memcpy(fc->txfm_partition_cdf, default_txfm_partition_cdf, sizeof(default_txfm_partition_cdf));
   memcpy(fc->compound_index_cdf, default_compound_idx_cdfs, sizeof(default_compound_idx_cdfs));
   memcpy(fc->comp_group_idx_cdf, default_comp_group_idx_cdfs, sizeof(default_comp_group_idx_cdfs));
   memcpy(fc->newmv_cdf, default_newmv_cdf, sizeof(default_newmv_cdf));
   memcpy(fc->zeromv_cdf, default_zeromv_cdf, sizeof(default_zeromv_cdf));
   memcpy(fc->refmv_cdf, default_refmv_cdf, sizeof(default_refmv_cdf));
   memcpy(fc->drl_cdf, default_drl_cdf, sizeof(default_drl_cdf));
   memcpy(fc->motion_mode_cdf, default_motion_mode_cdf, sizeof(default_motion_mode_cdf));
   memcpy(fc->obmc_cdf, default_obmc_cdf, sizeof(default_obmc_cdf));
   memcpy(fc->inter_compound_mode_cdf, default_inter_compound_mode_cdf, sizeof(default_inter_compound_mode_cdf));
   memcpy(fc->compound_type_cdf, default_compound_type_cdf, sizeof(default_compound_type_cdf));
   memcpy(fc->wedge_idx_cdf, default_wedge_idx_cdf, sizeof(default_wedge_idx_cdf));
   memcpy(fc->interintra_cdf, default_interintra_cdf, sizeof(default_interintra_cdf));
   memcpy(fc->wedge_interintra_cdf, default_wedge_interintra_cdf, sizeof(default_wedge_interintra_cdf));
   memcpy(fc->interintra_mode_cdf, default_interintra_mode_cdf, sizeof(default_interintra_mode_cdf));
   memcpy(fc->pred_cdf, default_segment_pred_cdf, sizeof(default_segment_pred_cdf));
   memcpy(fc->switchable_restore_cdf, default_switchable_restore_cdf, sizeof(default_switchable_restore_cdf));
   memcpy(fc->wiener_restore_cdf, default_wiener_restore_cdf, sizeof(default_wiener_restore_cdf));
   memcpy(fc->sgrproj_restore_cdf, default_sgrproj_restore_cdf, sizeof(default_sgrproj_restore_cdf));
   memcpy(fc->y_mode_cdf, default_if_y_mode_cdf, sizeof(default_if_y_mode_cdf));
   memcpy(fc->uv_mode_cdf, default_uv_mode_cdf, sizeof(default_uv_mode_cdf));
   memcpy(fc->switchable_interp_cdf, default_switchable_interp_cdf, sizeof(default_switchable_interp_cdf));
   memcpy(fc->partition_cdf, default_partition_cdf, sizeof(default_partition_cdf));
   memcpy(fc->intra_ext_tx_cdf, &default_intra_ext_tx_cdf[1], sizeof(default_intra_ext_tx_cdf[1]) * 2);
   memcpy(fc->inter_ext_tx_cdf, &default_inter_ext_tx_cdf[1], sizeof(default_inter_ext_tx_cdf[1]) * 3);
   memcpy(fc->skip_cdfs, default_skip_cdfs, sizeof(default_skip_cdfs));
   memcpy(fc->intra_inter_cdf, default_intra_inter_cdf, sizeof(default_intra_inter_cdf));
   memcpy(fc->tree_cdf, default_seg_tree_cdf, sizeof(default_seg_tree_cdf));
   for (i = 0; i < SPATIAL_PREDICTION_PROBS; ++i)
      memcpy(fc->spatial_pred_seg_cdf[i], default_spatial_pred_seg_tree_cdf[i], sizeof(default_spatial_pred_seg_tree_cdf[i]));
   memcpy(fc->tx_size_cdf, default_tx_size_cdf, sizeof(default_tx_size_cdf));
   memcpy(fc->delta_q_cdf, default_delta_q_cdf, sizeof(default_delta_q_cdf));
   memcpy(fc->skip_mode_cdfs, default_skip_mode_cdfs, sizeof(default_skip_mode_cdfs));
   memcpy(fc->delta_lf_cdf, default_delta_lf_cdf, sizeof(default_delta_lf_cdf));
   memcpy(fc->delta_lf_multi_cdf, default_delta_lf_multi_cdf, sizeof(default_delta_lf_multi_cdf));
   memcpy(fc->cfl_sign_cdf, default_cfl_sign_cdf, sizeof(default_cfl_sign_cdf));
   memcpy(fc->cfl_alpha_cdf, default_cfl_alpha_cdf, sizeof(default_cfl_alpha_cdf));
   memcpy(fc->filter_intra_cdfs, default_filter_intra_cdfs, sizeof(default_filter_intra_cdfs));
   memcpy(fc->filter_intra_mode_cdf, default_filter_intra_mode_cdf, sizeof(default_filter_intra_mode_cdf));
   memcpy(fc->intrabc_cdf, default_intrabc_cdf, sizeof(default_intrabc_cdf));
}

static void rvcn_av1_init_mv_probs(void *prob)
{
   rvcn_av1_frame_context_t * fc = (rvcn_av1_frame_context_t*)prob;

   memcpy(fc->nmvc_joints_cdf, default_nmv_context.joints_cdf, sizeof(default_nmv_context.joints_cdf));
   memcpy(fc->nmvc_0_bits_cdf, default_nmv_context.comps[0].bits_cdf, sizeof(default_nmv_context.comps[0].bits_cdf));
   memcpy(fc->nmvc_0_class0_cdf, default_nmv_context.comps[0].class0_cdf, sizeof(default_nmv_context.comps[0].class0_cdf));
   memcpy(fc->nmvc_0_class0_fp_cdf, default_nmv_context.comps[0].class0_fp_cdf, sizeof(default_nmv_context.comps[0].class0_fp_cdf));
   memcpy(fc->nmvc_0_class0_hp_cdf, default_nmv_context.comps[0].class0_hp_cdf, sizeof(default_nmv_context.comps[0].class0_hp_cdf));
   memcpy(fc->nmvc_0_classes_cdf, default_nmv_context.comps[0].classes_cdf, sizeof(default_nmv_context.comps[0].classes_cdf));
   memcpy(fc->nmvc_0_fp_cdf, default_nmv_context.comps[0].fp_cdf, sizeof(default_nmv_context.comps[0].fp_cdf));
   memcpy(fc->nmvc_0_hp_cdf, default_nmv_context.comps[0].hp_cdf, sizeof(default_nmv_context.comps[0].hp_cdf));
   memcpy(fc->nmvc_0_sign_cdf, default_nmv_context.comps[0].sign_cdf, sizeof(default_nmv_context.comps[0].sign_cdf));
   memcpy(fc->nmvc_1_bits_cdf, default_nmv_context.comps[1].bits_cdf, sizeof(default_nmv_context.comps[1].bits_cdf));
   memcpy(fc->nmvc_1_class0_cdf, default_nmv_context.comps[1].class0_cdf, sizeof(default_nmv_context.comps[1].class0_cdf));
   memcpy(fc->nmvc_1_class0_fp_cdf, default_nmv_context.comps[1].class0_fp_cdf, sizeof(default_nmv_context.comps[1].class0_fp_cdf));
   memcpy(fc->nmvc_1_class0_hp_cdf, default_nmv_context.comps[1].class0_hp_cdf, sizeof(default_nmv_context.comps[1].class0_hp_cdf));
   memcpy(fc->nmvc_1_classes_cdf, default_nmv_context.comps[1].classes_cdf, sizeof(default_nmv_context.comps[1].classes_cdf));
   memcpy(fc->nmvc_1_fp_cdf, default_nmv_context.comps[1].fp_cdf, sizeof(default_nmv_context.comps[1].fp_cdf));
   memcpy(fc->nmvc_1_hp_cdf, default_nmv_context.comps[1].hp_cdf, sizeof(default_nmv_context.comps[1].hp_cdf));
   memcpy(fc->nmvc_1_sign_cdf, default_nmv_context.comps[1].sign_cdf, sizeof(default_nmv_context.comps[1].sign_cdf));
   memcpy(fc->ndvc_joints_cdf, default_nmv_context.joints_cdf, sizeof(default_nmv_context.joints_cdf));
   memcpy(fc->ndvc_0_bits_cdf, default_nmv_context.comps[0].bits_cdf, sizeof(default_nmv_context.comps[0].bits_cdf));
   memcpy(fc->ndvc_0_class0_cdf, default_nmv_context.comps[0].class0_cdf, sizeof(default_nmv_context.comps[0].class0_cdf));
   memcpy(fc->ndvc_0_class0_fp_cdf, default_nmv_context.comps[0].class0_fp_cdf, sizeof(default_nmv_context.comps[0].class0_fp_cdf));
   memcpy(fc->ndvc_0_class0_hp_cdf, default_nmv_context.comps[0].class0_hp_cdf, sizeof(default_nmv_context.comps[0].class0_hp_cdf));
   memcpy(fc->ndvc_0_classes_cdf, default_nmv_context.comps[0].classes_cdf, sizeof(default_nmv_context.comps[0].classes_cdf));
   memcpy(fc->ndvc_0_fp_cdf, default_nmv_context.comps[0].fp_cdf, sizeof(default_nmv_context.comps[0].fp_cdf));
   memcpy(fc->ndvc_0_hp_cdf, default_nmv_context.comps[0].hp_cdf, sizeof(default_nmv_context.comps[0].hp_cdf));
   memcpy(fc->ndvc_0_sign_cdf, default_nmv_context.comps[0].sign_cdf, sizeof(default_nmv_context.comps[0].sign_cdf));
   memcpy(fc->ndvc_1_bits_cdf, default_nmv_context.comps[1].bits_cdf, sizeof(default_nmv_context.comps[1].bits_cdf));
   memcpy(fc->ndvc_1_class0_cdf, default_nmv_context.comps[1].class0_cdf, sizeof(default_nmv_context.comps[1].class0_cdf));
   memcpy(fc->ndvc_1_class0_fp_cdf, default_nmv_context.comps[1].class0_fp_cdf, sizeof(default_nmv_context.comps[1].class0_fp_cdf));
   memcpy(fc->ndvc_1_class0_hp_cdf, default_nmv_context.comps[1].class0_hp_cdf, sizeof(default_nmv_context.comps[1].class0_hp_cdf));
   memcpy(fc->ndvc_1_classes_cdf, default_nmv_context.comps[1].classes_cdf, sizeof(default_nmv_context.comps[1].classes_cdf));
   memcpy(fc->ndvc_1_fp_cdf, default_nmv_context.comps[1].fp_cdf, sizeof(default_nmv_context.comps[1].fp_cdf));
   memcpy(fc->ndvc_1_hp_cdf, default_nmv_context.comps[1].hp_cdf, sizeof(default_nmv_context.comps[1].hp_cdf));
   memcpy(fc->ndvc_1_sign_cdf, default_nmv_context.comps[1].sign_cdf, sizeof(default_nmv_context.comps[1].sign_cdf));
}

static void rvcn_vcn4_av1_init_mv_probs(void *prob)
{
   rvcn_av1_vcn4_frame_context_t * fc = (rvcn_av1_vcn4_frame_context_t*)prob;

   memcpy(fc->nmvc_joints_cdf, default_nmv_context.joints_cdf, sizeof(default_nmv_context.joints_cdf));
   memcpy(fc->nmvc_0_bits_cdf, default_nmv_context.comps[0].bits_cdf, sizeof(default_nmv_context.comps[0].bits_cdf));
   memcpy(fc->nmvc_0_class0_cdf, default_nmv_context.comps[0].class0_cdf, sizeof(default_nmv_context.comps[0].class0_cdf));
   memcpy(fc->nmvc_0_class0_fp_cdf, default_nmv_context.comps[0].class0_fp_cdf, sizeof(default_nmv_context.comps[0].class0_fp_cdf));
   memcpy(fc->nmvc_0_class0_hp_cdf, default_nmv_context.comps[0].class0_hp_cdf, sizeof(default_nmv_context.comps[0].class0_hp_cdf));
   memcpy(fc->nmvc_0_classes_cdf, default_nmv_context.comps[0].classes_cdf, sizeof(default_nmv_context.comps[0].classes_cdf));
   memcpy(fc->nmvc_0_fp_cdf, default_nmv_context.comps[0].fp_cdf, sizeof(default_nmv_context.comps[0].fp_cdf));
   memcpy(fc->nmvc_0_hp_cdf, default_nmv_context.comps[0].hp_cdf, sizeof(default_nmv_context.comps[0].hp_cdf));
   memcpy(fc->nmvc_0_sign_cdf, default_nmv_context.comps[0].sign_cdf, sizeof(default_nmv_context.comps[0].sign_cdf));
   memcpy(fc->nmvc_1_bits_cdf, default_nmv_context.comps[1].bits_cdf, sizeof(default_nmv_context.comps[1].bits_cdf));
   memcpy(fc->nmvc_1_class0_cdf, default_nmv_context.comps[1].class0_cdf, sizeof(default_nmv_context.comps[1].class0_cdf));
   memcpy(fc->nmvc_1_class0_fp_cdf, default_nmv_context.comps[1].class0_fp_cdf, sizeof(default_nmv_context.comps[1].class0_fp_cdf));
   memcpy(fc->nmvc_1_class0_hp_cdf, default_nmv_context.comps[1].class0_hp_cdf, sizeof(default_nmv_context.comps[1].class0_hp_cdf));
   memcpy(fc->nmvc_1_classes_cdf, default_nmv_context.comps[1].classes_cdf, sizeof(default_nmv_context.comps[1].classes_cdf));
   memcpy(fc->nmvc_1_fp_cdf, default_nmv_context.comps[1].fp_cdf, sizeof(default_nmv_context.comps[1].fp_cdf));
   memcpy(fc->nmvc_1_hp_cdf, default_nmv_context.comps[1].hp_cdf, sizeof(default_nmv_context.comps[1].hp_cdf));
   memcpy(fc->nmvc_1_sign_cdf, default_nmv_context.comps[1].sign_cdf, sizeof(default_nmv_context.comps[1].sign_cdf));
   memcpy(fc->ndvc_joints_cdf, default_nmv_context.joints_cdf, sizeof(default_nmv_context.joints_cdf));
   memcpy(fc->ndvc_0_bits_cdf, default_nmv_context.comps[0].bits_cdf, sizeof(default_nmv_context.comps[0].bits_cdf));
   memcpy(fc->ndvc_0_class0_cdf, default_nmv_context.comps[0].class0_cdf, sizeof(default_nmv_context.comps[0].class0_cdf));
   memcpy(fc->ndvc_0_class0_fp_cdf, default_nmv_context.comps[0].class0_fp_cdf, sizeof(default_nmv_context.comps[0].class0_fp_cdf));
   memcpy(fc->ndvc_0_class0_hp_cdf, default_nmv_context.comps[0].class0_hp_cdf, sizeof(default_nmv_context.comps[0].class0_hp_cdf));
   memcpy(fc->ndvc_0_classes_cdf, default_nmv_context.comps[0].classes_cdf, sizeof(default_nmv_context.comps[0].classes_cdf));
   memcpy(fc->ndvc_0_fp_cdf, default_nmv_context.comps[0].fp_cdf, sizeof(default_nmv_context.comps[0].fp_cdf));
   memcpy(fc->ndvc_0_hp_cdf, default_nmv_context.comps[0].hp_cdf, sizeof(default_nmv_context.comps[0].hp_cdf));
   memcpy(fc->ndvc_0_sign_cdf, default_nmv_context.comps[0].sign_cdf, sizeof(default_nmv_context.comps[0].sign_cdf));
   memcpy(fc->ndvc_1_bits_cdf, default_nmv_context.comps[1].bits_cdf, sizeof(default_nmv_context.comps[1].bits_cdf));
   memcpy(fc->ndvc_1_class0_cdf, default_nmv_context.comps[1].class0_cdf, sizeof(default_nmv_context.comps[1].class0_cdf));
   memcpy(fc->ndvc_1_class0_fp_cdf, default_nmv_context.comps[1].class0_fp_cdf, sizeof(default_nmv_context.comps[1].class0_fp_cdf));
   memcpy(fc->ndvc_1_class0_hp_cdf, default_nmv_context.comps[1].class0_hp_cdf, sizeof(default_nmv_context.comps[1].class0_hp_cdf));
   memcpy(fc->ndvc_1_classes_cdf, default_nmv_context.comps[1].classes_cdf, sizeof(default_nmv_context.comps[1].classes_cdf));
   memcpy(fc->ndvc_1_fp_cdf, default_nmv_context.comps[1].fp_cdf, sizeof(default_nmv_context.comps[1].fp_cdf));
   memcpy(fc->ndvc_1_hp_cdf, default_nmv_context.comps[1].hp_cdf, sizeof(default_nmv_context.comps[1].hp_cdf));
   memcpy(fc->ndvc_1_sign_cdf, default_nmv_context.comps[1].sign_cdf, sizeof(default_nmv_context.comps[1].sign_cdf));
}

static void rvcn_av1_default_coef_probs(void *prob, int index)
{
   rvcn_av1_frame_context_t * fc = (rvcn_av1_frame_context_t*)prob;

   memcpy(fc->txb_skip_cdf, av1_default_txb_skip_cdfs[index], sizeof(av1_default_txb_skip_cdfs[index]));
   memcpy(fc->eob_extra_cdf, av1_default_eob_extra_cdfs[index], sizeof(av1_default_eob_extra_cdfs[index]));
   memcpy(fc->dc_sign_cdf, av1_default_dc_sign_cdfs[index], sizeof(av1_default_dc_sign_cdfs[index]));
   memcpy(fc->coeff_br_cdf, av1_default_coeff_lps_multi_cdfs[index], sizeof(av1_default_coeff_lps_multi_cdfs[index]));
   memcpy(fc->coeff_base_cdf, av1_default_coeff_base_multi_cdfs[index], sizeof(av1_default_coeff_base_multi_cdfs[index]));
   memcpy(fc->coeff_base_eob_cdf, av1_default_coeff_base_eob_multi_cdfs[index], sizeof(av1_default_coeff_base_eob_multi_cdfs[index]));
   memcpy(fc->eob_flag_cdf16, av1_default_eob_multi16_cdfs[index], sizeof(av1_default_eob_multi16_cdfs[index]));
   memcpy(fc->eob_flag_cdf32, av1_default_eob_multi32_cdfs[index], sizeof(av1_default_eob_multi32_cdfs[index]));
   memcpy(fc->eob_flag_cdf64, av1_default_eob_multi64_cdfs[index], sizeof(av1_default_eob_multi64_cdfs[index]));
   memcpy(fc->eob_flag_cdf128, av1_default_eob_multi128_cdfs[index], sizeof(av1_default_eob_multi128_cdfs[index]));
   memcpy(fc->eob_flag_cdf256, av1_default_eob_multi256_cdfs[index], sizeof(av1_default_eob_multi256_cdfs[index]));
   memcpy(fc->eob_flag_cdf512, av1_default_eob_multi512_cdfs[index], sizeof(av1_default_eob_multi512_cdfs[index]));
   memcpy(fc->eob_flag_cdf1024, av1_default_eob_multi1024_cdfs[index], sizeof(av1_default_eob_multi1024_cdfs[index]));
}

static void rvcn_vcn4_av1_default_coef_probs(void *prob, int index)
{
   rvcn_av1_vcn4_frame_context_t *fc = (rvcn_av1_vcn4_frame_context_t*)prob;
   void *p;
   int i, j;
   unsigned size;

   memcpy(fc->txb_skip_cdf, av1_default_txb_skip_cdfs[index], sizeof(av1_default_txb_skip_cdfs[index]));

   p = (void *)fc->eob_extra_cdf;
   size = sizeof(av1_default_eob_extra_cdfs[0][0][0][0]) * EOB_COEF_CONTEXTS_VCN4;
   for (i = 0; i < AV1_TX_SIZES; i++) {
      for ( j = 0; j < AV1_PLANE_TYPES; j++) {
         memcpy(p, &av1_default_eob_extra_cdfs[index][i][j][3], size);
         p += size;
      }
   }

   memcpy(fc->dc_sign_cdf, av1_default_dc_sign_cdfs[index], sizeof(av1_default_dc_sign_cdfs[index]));
   memcpy(fc->coeff_br_cdf, av1_default_coeff_lps_multi_cdfs[index], sizeof(av1_default_coeff_lps_multi_cdfs[index]));
   memcpy(fc->coeff_base_cdf, av1_default_coeff_base_multi_cdfs[index], sizeof(av1_default_coeff_base_multi_cdfs[index]));
   memcpy(fc->coeff_base_eob_cdf, av1_default_coeff_base_eob_multi_cdfs[index], sizeof(av1_default_coeff_base_eob_multi_cdfs[index]));
   memcpy(fc->eob_flag_cdf16, av1_default_eob_multi16_cdfs[index], sizeof(av1_default_eob_multi16_cdfs[index]));
   memcpy(fc->eob_flag_cdf32, av1_default_eob_multi32_cdfs[index], sizeof(av1_default_eob_multi32_cdfs[index]));
   memcpy(fc->eob_flag_cdf64, av1_default_eob_multi64_cdfs[index], sizeof(av1_default_eob_multi64_cdfs[index]));
   memcpy(fc->eob_flag_cdf128, av1_default_eob_multi128_cdfs[index], sizeof(av1_default_eob_multi128_cdfs[index]));
   memcpy(fc->eob_flag_cdf256, av1_default_eob_multi256_cdfs[index], sizeof(av1_default_eob_multi256_cdfs[index]));
   memcpy(fc->eob_flag_cdf512, av1_default_eob_multi512_cdfs[index], sizeof(av1_default_eob_multi512_cdfs[index]));
   memcpy(fc->eob_flag_cdf1024, av1_default_eob_multi1024_cdfs[index], sizeof(av1_default_eob_multi1024_cdfs[index]));
}

static unsigned calc_ctx_size_h265_main(struct radeon_decoder *dec)
{
   unsigned width = align(dec->base.width, VL_MACROBLOCK_WIDTH);
   unsigned height = align(dec->base.height, VL_MACROBLOCK_HEIGHT);

   unsigned max_references = dec->base.max_references + 1;

   if (dec->base.width * dec->base.height >= 4096 * 2000)
      max_references = MAX2(max_references, 8);
   else
      max_references = MAX2(max_references, 17);

   width = align(width, 16);
   height = align(height, 16);
   return ((width + 255) / 16) * ((height + 255) / 16) * 16 * max_references + 52 * 1024;
}

static unsigned calc_ctx_size_h265_main10(struct radeon_decoder *dec,
                                          struct pipe_h265_picture_desc *pic)
{
   unsigned log2_ctb_size, width_in_ctb, height_in_ctb, num_16x16_block_per_ctb;
   unsigned context_buffer_size_per_ctb_row, cm_buffer_size, max_mb_address, db_left_tile_pxl_size;
   unsigned db_left_tile_ctx_size = 4096 / 16 * (32 + 16 * 4);

   unsigned width = align(dec->base.width, VL_MACROBLOCK_WIDTH);
   unsigned height = align(dec->base.height, VL_MACROBLOCK_HEIGHT);
   unsigned coeff_10bit =
      (pic->pps->sps->bit_depth_luma_minus8 || pic->pps->sps->bit_depth_chroma_minus8) ? 2 : 1;

   unsigned max_references = dec->base.max_references + 1;

   if (dec->base.width * dec->base.height >= 4096 * 2000)
      max_references = MAX2(max_references, 8);
   else
      max_references = MAX2(max_references, 17);

   log2_ctb_size = pic->pps->sps->log2_min_luma_coding_block_size_minus3 + 3 +
                   pic->pps->sps->log2_diff_max_min_luma_coding_block_size;

   width_in_ctb = (width + ((1 << log2_ctb_size) - 1)) >> log2_ctb_size;
   height_in_ctb = (height + ((1 << log2_ctb_size) - 1)) >> log2_ctb_size;

   num_16x16_block_per_ctb = ((1 << log2_ctb_size) >> 4) * ((1 << log2_ctb_size) >> 4);
   context_buffer_size_per_ctb_row = align(width_in_ctb * num_16x16_block_per_ctb * 16, 256);
   max_mb_address = (unsigned)ceil(height * 8 / 2048.0);

   cm_buffer_size = max_references * context_buffer_size_per_ctb_row * height_in_ctb;
   db_left_tile_pxl_size = coeff_10bit * (max_mb_address * 2 * 2048 + 1024);

   return cm_buffer_size + db_left_tile_ctx_size + db_left_tile_pxl_size;
}

static rvcn_dec_message_vc1_t get_vc1_msg(struct pipe_vc1_picture_desc *pic)
{
   rvcn_dec_message_vc1_t result;

   memset(&result, 0, sizeof(result));
   switch (pic->base.profile) {
   case PIPE_VIDEO_PROFILE_VC1_SIMPLE:
      result.profile = RDECODE_VC1_PROFILE_SIMPLE;
      result.level = 1;
      break;

   case PIPE_VIDEO_PROFILE_VC1_MAIN:
      result.profile = RDECODE_VC1_PROFILE_MAIN;
      result.level = 2;
      break;

   case PIPE_VIDEO_PROFILE_VC1_ADVANCED:
      result.profile = RDECODE_VC1_PROFILE_ADVANCED;
      result.level = 4;
      break;

   default:
      assert(0);
   }

   result.sps_info_flags |= pic->postprocflag << 7;
   result.sps_info_flags |= pic->pulldown << 6;
   result.sps_info_flags |= pic->interlace << 5;
   result.sps_info_flags |= pic->tfcntrflag << 4;
   result.sps_info_flags |= pic->finterpflag << 3;
   result.sps_info_flags |= pic->psf << 1;

   result.pps_info_flags |= pic->range_mapy_flag << 31;
   result.pps_info_flags |= pic->range_mapy << 28;
   result.pps_info_flags |= pic->range_mapuv_flag << 27;
   result.pps_info_flags |= pic->range_mapuv << 24;
   result.pps_info_flags |= pic->multires << 21;
   result.pps_info_flags |= pic->maxbframes << 16;
   result.pps_info_flags |= pic->overlap << 11;
   result.pps_info_flags |= pic->quantizer << 9;
   result.pps_info_flags |= pic->panscan_flag << 7;
   result.pps_info_flags |= pic->refdist_flag << 6;
   result.pps_info_flags |= pic->vstransform << 0;

   if (pic->base.profile != PIPE_VIDEO_PROFILE_VC1_SIMPLE) {
      result.pps_info_flags |= pic->syncmarker << 20;
      result.pps_info_flags |= pic->rangered << 19;
      result.pps_info_flags |= pic->loopfilter << 5;
      result.pps_info_flags |= pic->fastuvmc << 4;
      result.pps_info_flags |= pic->extended_mv << 3;
      result.pps_info_flags |= pic->extended_dmv << 8;
      result.pps_info_flags |= pic->dquant << 1;
   }

   result.chroma_format = 1;

   return result;
}

static uint32_t get_ref_pic_idx(struct radeon_decoder *dec, struct pipe_video_buffer *ref)
{
   uint32_t min = MAX2(dec->frame_number, NUM_MPEG2_REFS) - NUM_MPEG2_REFS;
   uint32_t max = MAX2(dec->frame_number, 1) - 1;
   uintptr_t frame;

   /* seems to be the most sane fallback */
   if (!ref)
      return max;

   /* get the frame number from the associated data */
   frame = (uintptr_t)vl_video_buffer_get_associated_data(ref, &dec->base);

   /* limit the frame number to a valid range */
   return MAX2(MIN2(frame, max), min);
}

static rvcn_dec_message_mpeg2_vld_t get_mpeg2_msg(struct radeon_decoder *dec,
                                                  struct pipe_mpeg12_picture_desc *pic)
{
   const int *zscan = pic->alternate_scan ? vl_zscan_alternate : vl_zscan_normal;
   rvcn_dec_message_mpeg2_vld_t result;
   unsigned i;

   memset(&result, 0, sizeof(result));
   result.decoded_pic_idx = dec->frame_number;

   result.forward_ref_pic_idx = get_ref_pic_idx(dec, pic->ref[0]);
   result.backward_ref_pic_idx = get_ref_pic_idx(dec, pic->ref[1]);

   if (pic->intra_matrix) {
      result.load_intra_quantiser_matrix = 1;
      for (i = 0; i < 64; ++i) {
         result.intra_quantiser_matrix[i] = pic->intra_matrix[zscan[i]];
      }
   }
   if (pic->non_intra_matrix) {
      result.load_nonintra_quantiser_matrix = 1;
      for (i = 0; i < 64; ++i) {
         result.nonintra_quantiser_matrix[i] = pic->non_intra_matrix[zscan[i]];
      }
   }

   result.profile_and_level_indication = 0;
   result.chroma_format = 0x1;

   result.picture_coding_type = pic->picture_coding_type;
   result.f_code[0][0] = pic->f_code[0][0] + 1;
   result.f_code[0][1] = pic->f_code[0][1] + 1;
   result.f_code[1][0] = pic->f_code[1][0] + 1;
   result.f_code[1][1] = pic->f_code[1][1] + 1;
   result.intra_dc_precision = pic->intra_dc_precision;
   result.pic_structure = pic->picture_structure;
   result.top_field_first = pic->top_field_first;
   result.frame_pred_frame_dct = pic->frame_pred_frame_dct;
   result.concealment_motion_vectors = pic->concealment_motion_vectors;
   result.q_scale_type = pic->q_scale_type;
   result.intra_vlc_format = pic->intra_vlc_format;
   result.alternate_scan = pic->alternate_scan;

   return result;
}

static rvcn_dec_message_mpeg4_asp_vld_t get_mpeg4_msg(struct radeon_decoder *dec,
                                                      struct pipe_mpeg4_picture_desc *pic)
{
   rvcn_dec_message_mpeg4_asp_vld_t result;
   unsigned i;

   memset(&result, 0, sizeof(result));
   result.decoded_pic_idx = dec->frame_number;

   result.forward_ref_pic_idx = get_ref_pic_idx(dec, pic->ref[0]);
   result.backward_ref_pic_idx = get_ref_pic_idx(dec, pic->ref[1]);

   result.variant_type = 0;
   result.profile_and_level_indication = 0xF0;

   result.video_object_layer_verid = 0x5;
   result.video_object_layer_shape = 0x0;

   result.video_object_layer_width = dec->base.width;
   result.video_object_layer_height = dec->base.height;

   result.vop_time_increment_resolution = pic->vop_time_increment_resolution;

   result.short_video_header = pic->short_video_header;
   result.interlaced = pic->interlaced;
   result.load_intra_quant_mat = 1;
   result.load_nonintra_quant_mat = 1;
   result.quarter_sample = pic->quarter_sample;
   result.complexity_estimation_disable = 1;
   result.resync_marker_disable = pic->resync_marker_disable;
   result.newpred_enable = 0;
   result.reduced_resolution_vop_enable = 0;

   result.quant_type = pic->quant_type;

   for (i = 0; i < 64; ++i) {
      result.intra_quant_mat[i] = pic->intra_matrix[vl_zscan_normal[i]];
      result.nonintra_quant_mat[i] = pic->non_intra_matrix[vl_zscan_normal[i]];
   }

   return result;
}

static void rvcn_dec_message_create(struct radeon_decoder *dec)
{
   rvcn_dec_message_header_t *header = dec->msg;
   rvcn_dec_message_create_t *create = dec->msg + sizeof(rvcn_dec_message_header_t);
   unsigned sizes = sizeof(rvcn_dec_message_header_t) + sizeof(rvcn_dec_message_create_t);

   memset(dec->msg, 0, sizes);
   header->header_size = sizeof(rvcn_dec_message_header_t);
   header->total_size = sizes;
   header->num_buffers = 1;
   header->msg_type = RDECODE_MSG_CREATE;
   header->stream_handle = dec->stream_handle;
   header->status_report_feedback_number = 0;

   header->index[0].message_id = RDECODE_MESSAGE_CREATE;
   header->index[0].offset = sizeof(rvcn_dec_message_header_t);
   header->index[0].size = sizeof(rvcn_dec_message_create_t);
   header->index[0].filled = 0;

   create->stream_type = dec->stream_type;
   create->session_flags = 0;
   create->width_in_samples = dec->base.width;
   create->height_in_samples = dec->base.height;
}

static unsigned rvcn_dec_dynamic_dpb_t2_message(struct radeon_decoder *dec, rvcn_dec_message_decode_t *decode,
      rvcn_dec_message_dynamic_dpb_t2_t *dynamic_dpb_t2, bool encrypted)
{
   struct rvcn_dec_dynamic_dpb_t2 *dpb = NULL, *dummy = NULL;
   unsigned width, height, size;
   uint64_t addr;
   int i;

   width = align(decode->width_in_samples, dec->db_alignment);
   height = align(decode->height_in_samples, dec->db_alignment);
   size = align((width * height * 3) / 2, 256);
   if (dec->ref_codec.bts == CODEC_10_BITS)
      size = size * 3 / 2;

   list_for_each_entry_safe(struct rvcn_dec_dynamic_dpb_t2, d, &dec->dpb_ref_list, list) {
      for (i = 0; i < dec->ref_codec.ref_size; ++i) {
         if (((dec->ref_codec.ref_list[i] & 0x7f) != 0x7f) && (d->index == (dec->ref_codec.ref_list[i] & 0x7f))) {
            if (!dummy)
               dummy = d;

            addr = dec->ws->buffer_get_virtual_address(d->dpb.res->buf);
            if (!addr && dummy) {
               RVID_ERR("Ref list from application is incorrect, using dummy buffer instead.\n");
               addr = dec->ws->buffer_get_virtual_address(dummy->dpb.res->buf);
            }
            dynamic_dpb_t2->dpbAddrLo[i] = addr;
            dynamic_dpb_t2->dpbAddrHi[i] = addr >> 32;
            ++dynamic_dpb_t2->dpbArraySize;
            break;
         }
      }
      if (i == dec->ref_codec.ref_size) {
         if (d->dpb.res->b.b.width0 * d->dpb.res->b.b.height0 != size) {
            list_del(&d->list);
            list_addtail(&d->list, &dec->dpb_unref_list);
         } else {
            d->index = 0x7f;
         }
      }
   }

   list_for_each_entry_safe(struct rvcn_dec_dynamic_dpb_t2, d, &dec->dpb_ref_list, list) {
      if (d->dpb.res->b.b.width0 * d->dpb.res->b.b.height0 == size && d->index == dec->ref_codec.index) {
         dpb = d;
         break;
      }
   }

   if (!dpb) {
      list_for_each_entry_safe(struct rvcn_dec_dynamic_dpb_t2, d, &dec->dpb_ref_list, list) {
         if (d->index == 0x7f) {
            d->index = dec->ref_codec.index;
            dpb = d;
            break;
         }
      }
   }

   list_for_each_entry_safe(struct rvcn_dec_dynamic_dpb_t2, d, &dec->dpb_unref_list, list) {
      if (dec->prev_fence)
         dec->ws->fence_wait(dec->ws, dec->prev_fence, PIPE_DEFAULT_DECODER_FEEDBACK_TIMEOUT_NS);
      list_del(&d->list);
      si_vid_destroy_buffer(&d->dpb);
      FREE(d);
   }

   if (!dpb) {
      bool r;
      dpb = CALLOC_STRUCT(rvcn_dec_dynamic_dpb_t2);
      if (!dpb)
         return 1;
      dpb->index = dec->ref_codec.index;
      if (encrypted)
         r = si_vid_create_tmz_buffer(dec->screen, &dpb->dpb, size, PIPE_USAGE_DEFAULT);
      else
         r = si_vid_create_buffer(dec->screen, &dpb->dpb, size, PIPE_USAGE_DEFAULT);
      assert(encrypted == (bool)(dpb->dpb.res->flags & RADEON_FLAG_ENCRYPTED));

      if (!r) {
         RVID_ERR("Can't allocated dpb buffer.\n");
         FREE(dpb);
         return 1;
      }
      list_addtail(&dpb->list, &dec->dpb_ref_list);
   }

   dec->ws->cs_add_buffer(&dec->cs, dpb->dpb.res->buf,
      RADEON_USAGE_READWRITE | RADEON_USAGE_SYNCHRONIZED, RADEON_DOMAIN_VRAM);
   addr = dec->ws->buffer_get_virtual_address(dpb->dpb.res->buf);
   dynamic_dpb_t2->dpbCurrLo = addr;
   dynamic_dpb_t2->dpbCurrHi = addr >> 32;

   decode->decode_flags = 1;
   dynamic_dpb_t2->dpbConfigFlags = 0;
   dynamic_dpb_t2->dpbLumaPitch = align(decode->width_in_samples, dec->db_alignment);
   dynamic_dpb_t2->dpbLumaAlignedHeight = align(decode->height_in_samples, dec->db_alignment);
   dynamic_dpb_t2->dpbLumaAlignedSize = dynamic_dpb_t2->dpbLumaPitch *
      dynamic_dpb_t2->dpbLumaAlignedHeight;
   dynamic_dpb_t2->dpbChromaPitch = dynamic_dpb_t2->dpbLumaPitch >> 1;
   dynamic_dpb_t2->dpbChromaAlignedHeight = dynamic_dpb_t2->dpbLumaAlignedHeight >> 1;
   dynamic_dpb_t2->dpbChromaAlignedSize = dynamic_dpb_t2->dpbChromaPitch *
      dynamic_dpb_t2->dpbChromaAlignedHeight * 2;

   if (dec->ref_codec.bts == CODEC_10_BITS) {
      dynamic_dpb_t2->dpbLumaAlignedSize = dynamic_dpb_t2->dpbLumaAlignedSize * 3 / 2;
      dynamic_dpb_t2->dpbChromaAlignedSize = dynamic_dpb_t2->dpbChromaAlignedSize * 3 / 2;
   }

   return 0;
}

static struct pb_buffer_lean *rvcn_dec_message_decode(struct radeon_decoder *dec,
                                                      struct pipe_video_buffer *target,
                                                      struct pipe_picture_desc *picture)
{
   DECRYPT_PARAMETERS *decrypt = (DECRYPT_PARAMETERS *)picture->decrypt_key;
   bool encrypted = picture->protected_playback;
   struct si_texture *luma;
   struct si_texture *chroma;
   struct pipe_video_buffer *out_surf = target;
   ASSERTED struct si_screen *sscreen = (struct si_screen *)dec->screen;
   rvcn_dec_message_header_t *header;
   rvcn_dec_message_index_t *index_codec;
   rvcn_dec_message_index_t *index_drm = NULL;
   rvcn_dec_message_index_t *index_dynamic_dpb = NULL;
   rvcn_dec_message_index_t *index_hevc_direct_reflist = NULL;
   rvcn_dec_message_decode_t *decode;
   unsigned sizes = 0, offset_decode, offset_codec;
   unsigned offset_drm = 0, offset_dynamic_dpb = 0, offset_hevc_direct_reflist = 0;
   void *codec;
   rvcn_dec_message_drm_t *drm = NULL;
   rvcn_dec_message_dynamic_dpb_t *dynamic_dpb = NULL;
   rvcn_dec_message_dynamic_dpb_t2_t *dynamic_dpb_t2 = NULL;
   rvcn_dec_message_hevc_direct_ref_list_t *hevc_reflist = NULL;
   bool dpb_resize = false;
   header = dec->msg;
   sizes += sizeof(rvcn_dec_message_header_t);

   index_codec = (void*)header + sizes;
   sizes += sizeof(rvcn_dec_message_index_t);

   if (encrypted) {
      index_drm = (void*)header + sizes;
      sizes += sizeof(rvcn_dec_message_index_t);
   }

   if (dec->dpb_type >= DPB_DYNAMIC_TIER_1) {
      index_dynamic_dpb = (void*)header + sizes;
      sizes += sizeof(rvcn_dec_message_index_t);
   }

   if (u_reduce_video_profile(picture->profile) == PIPE_VIDEO_FORMAT_HEVC) {
      index_hevc_direct_reflist = (void*)header + sizes;
      sizes += sizeof(rvcn_dec_message_index_t);
   }

   offset_decode = sizes;
   decode = (void*)header + sizes;
   sizes += sizeof(rvcn_dec_message_decode_t);

   if (encrypted) {
      offset_drm = sizes;
      drm = (void*)header + sizes;
      sizes += sizeof(rvcn_dec_message_drm_t);
   }

   if (dec->dpb_type >= DPB_DYNAMIC_TIER_1) {
      offset_dynamic_dpb = sizes;
      if (dec->dpb_type == DPB_DYNAMIC_TIER_1) {
         dynamic_dpb = (void*)header + sizes;
         sizes += sizeof(rvcn_dec_message_dynamic_dpb_t);
      }
      else if (dec->dpb_type == DPB_DYNAMIC_TIER_2) {
         dynamic_dpb_t2 = (void*)header + sizes;
         sizes += sizeof(rvcn_dec_message_dynamic_dpb_t2_t);
      }
   }

   if (u_reduce_video_profile(picture->profile) == PIPE_VIDEO_FORMAT_HEVC) {
      offset_hevc_direct_reflist = sizes;
      hevc_reflist = (void*)header + sizes;
      sizes += align((4 + 2 * 15 * ((struct pipe_h265_picture_desc *)picture)->slice_parameter.slice_count), 4);
   }

   offset_codec = sizes;
   codec = (void*)header + sizes;

   memset(dec->msg, 0, sizes);
   header->header_size = sizeof(rvcn_dec_message_header_t);
   header->total_size = sizes;
   header->msg_type = RDECODE_MSG_DECODE;
   header->stream_handle = dec->stream_handle;
   header->status_report_feedback_number = dec->frame_number;

   header->index[0].message_id = RDECODE_MESSAGE_DECODE;
   header->index[0].offset = offset_decode;
   header->index[0].size = sizeof(rvcn_dec_message_decode_t);
   header->index[0].filled = 0;
   header->num_buffers = 1;

   index_codec->offset = offset_codec;
   index_codec->size = sizeof(rvcn_dec_message_avc_t);
   index_codec->filled = 0;
   ++header->num_buffers;

   if (encrypted) {
      index_drm->message_id = RDECODE_MESSAGE_DRM;
      index_drm->offset = offset_drm;
      index_drm->size = sizeof(rvcn_dec_message_drm_t);
      index_drm->filled = 0;
      ++header->num_buffers;
   }

   if (dec->dpb_type >= DPB_DYNAMIC_TIER_1) {
      index_dynamic_dpb->message_id = RDECODE_MESSAGE_DYNAMIC_DPB;
      index_dynamic_dpb->offset = offset_dynamic_dpb;
      index_dynamic_dpb->filled = 0;
      ++header->num_buffers;
      if (dec->dpb_type == DPB_DYNAMIC_TIER_1)
         index_dynamic_dpb->size = sizeof(rvcn_dec_message_dynamic_dpb_t);
      else if (dec->dpb_type == DPB_DYNAMIC_TIER_2)
         index_dynamic_dpb->size = sizeof(rvcn_dec_message_dynamic_dpb_t2_t);
   }

   if (u_reduce_video_profile(picture->profile) == PIPE_VIDEO_FORMAT_HEVC) {
      index_hevc_direct_reflist->message_id = RDECODE_MESSAGE_HEVC_DIRECT_REF_LIST;
      index_hevc_direct_reflist->offset = offset_hevc_direct_reflist;
      index_hevc_direct_reflist->size = align((4 + 2 * 15 * ((struct pipe_h265_picture_desc *)picture)->slice_parameter.slice_count), 4);
      index_hevc_direct_reflist->filled = 0;
      ++header->num_buffers;
   }

   decode->stream_type = dec->stream_type;
   decode->decode_flags = 0;
   decode->width_in_samples = dec->base.width;
   decode->height_in_samples = dec->base.height;

   decode->bsd_size = align(dec->bs_size, 128);

   if (dec->dpb_type != DPB_DYNAMIC_TIER_2) {
      bool r;
      if (!dec->dpb.res && dec->dpb_size) {
         if (encrypted) {
            r = si_vid_create_tmz_buffer(dec->screen, &dec->dpb, dec->dpb_size, PIPE_USAGE_DEFAULT);
         } else {
            r = si_vid_create_buffer(dec->screen, &dec->dpb, dec->dpb_size, PIPE_USAGE_DEFAULT);
         }
         assert(encrypted == (bool)(dec->dpb.res->flags & RADEON_FLAG_ENCRYPTED));
         if (!r) {
            RVID_ERR("Can't allocated dpb.\n");
            return NULL;
         }
         si_vid_clear_buffer(dec->base.context, &dec->dpb);
      } else if (dec->dpb_type == DPB_DYNAMIC_TIER_1 && dec->dpb.res &&
                 (dec->max_width < dec->base.width || dec->max_height < dec->base.height)) {
         struct rvid_buf_offset_info buf_offset_info;

         buf_offset_info.num_units = (NUM_VP9_REFS + 1);
         buf_offset_info.old_offset = (align(dec->max_width, dec->db_alignment) *
            align(dec->max_height, dec->db_alignment) * 3 / 2);
         buf_offset_info.new_offset = (align(dec->base.width, dec->db_alignment) *
            align(dec->base.height, dec->db_alignment) * 3 / 2);

         dec->dpb_size = calc_dpb_size(dec);
         r = si_vid_resize_buffer(dec->screen, &dec->cs, &dec->dpb, dec->dpb_size, &buf_offset_info);
         if (!r) {
            RVID_ERR("Can't resize dpb.\n");
            return NULL;
	 }
         dec->max_width = dec->base.width;
         dec->max_height = dec->base.height;
         dpb_resize = true;
      }
   }

   if (!dec->ctx.res) {
      enum pipe_video_format fmt = u_reduce_video_profile(picture->profile);
      if (dec->stream_type == RDECODE_CODEC_H264_PERF) {
         unsigned ctx_size = calc_ctx_size_h264_perf(dec);
         bool r;
         if (encrypted && dec->tmz_ctx) {
            r = si_vid_create_tmz_buffer(dec->screen, &dec->ctx, ctx_size, PIPE_USAGE_DEFAULT);
         } else {
            r = si_vid_create_buffer(dec->screen, &dec->ctx, ctx_size, PIPE_USAGE_DEFAULT);
         }
         assert((encrypted && dec->tmz_ctx) == (bool)(dec->ctx.res->flags & RADEON_FLAG_ENCRYPTED));

         if (!r) {
            RVID_ERR("Can't allocated context buffer.\n");
            return NULL;
         }
         si_vid_clear_buffer(dec->base.context, &dec->ctx);
      } else if (fmt == PIPE_VIDEO_FORMAT_VP9) {
         unsigned ctx_size;
         uint8_t *ptr;
         bool r;

         /* default probability + probability data */
         ctx_size = 2304 * 5;

         if (((struct si_screen *)dec->screen)->info.vcn_ip_version >= VCN_2_0_0) {
            /* SRE collocated context data */
            ctx_size += 32 * 2 * 128 * 68;
            /* SMP collocated context data */
            ctx_size += 9 * 64 * 2 * 128 * 68;
            /* SDB left tile pixel */
            ctx_size += 8 * 2 * 2 * 8192;
         } else {
            ctx_size += 32 * 2 * 64 * 64;
            ctx_size += 9 * 64 * 2 * 64 * 64;
            ctx_size += 8 * 2 * 4096;
         }

         if (dec->base.profile == PIPE_VIDEO_PROFILE_VP9_PROFILE2)
            ctx_size += 8 * 2 * 4096;

         if (encrypted && dec->tmz_ctx) {
            r = si_vid_create_tmz_buffer(dec->screen, &dec->ctx, ctx_size, PIPE_USAGE_DEFAULT);
         } else {
            r = si_vid_create_buffer(dec->screen, &dec->ctx, ctx_size, PIPE_USAGE_DEFAULT);
         }
         if (!r) {
            RVID_ERR("Can't allocated context buffer.\n");
            return NULL;
         }
         si_vid_clear_buffer(dec->base.context, &dec->ctx);

         /* ctx needs probs table */
         ptr = dec->ws->buffer_map(dec->ws, dec->ctx.res->buf, &dec->cs,
                                   PIPE_MAP_WRITE | RADEON_MAP_TEMPORARY);
         fill_probs_table(ptr);
         dec->ws->buffer_unmap(dec->ws, dec->ctx.res->buf);
         dec->bs_ptr = NULL;
      } else if (fmt == PIPE_VIDEO_FORMAT_HEVC) {
         unsigned ctx_size;
         bool r;
         if (dec->base.profile == PIPE_VIDEO_PROFILE_HEVC_MAIN_10)
            ctx_size = calc_ctx_size_h265_main10(dec, (struct pipe_h265_picture_desc *)picture);
         else
            ctx_size = calc_ctx_size_h265_main(dec);

         if (encrypted && dec->tmz_ctx) {
            r = si_vid_create_tmz_buffer(dec->screen, &dec->ctx, ctx_size, PIPE_USAGE_DEFAULT);
         } else {
            r = si_vid_create_buffer(dec->screen, &dec->ctx, ctx_size, PIPE_USAGE_DEFAULT);
         }
         if (!r) {
            RVID_ERR("Can't allocated context buffer.\n");
            return NULL;
         }
         si_vid_clear_buffer(dec->base.context, &dec->ctx);
      }
   }
   if (encrypted != dec->ws->cs_is_secure(&dec->cs)) {
      dec->ws->cs_flush(&dec->cs, RADEON_FLUSH_TOGGLE_SECURE_SUBMISSION, NULL);
   }

   if (dec->stream_type == RDECODE_CODEC_AV1)
      rvcn_dec_av1_film_grain_surface(&out_surf, (struct pipe_av1_picture_desc *)picture);

   luma   = (struct si_texture *)((struct vl_video_buffer *)out_surf)->resources[0];
   chroma = (struct si_texture *)((struct vl_video_buffer *)out_surf)->resources[1];

   decode->dpb_size = (dec->dpb_type != DPB_DYNAMIC_TIER_2) ? dec->dpb.res->buf->size : 0;

   /* When texture being created, the bo will be created with total size of planes,
    * and all planes point to the same buffer */
   assert(si_resource(((struct vl_video_buffer *)out_surf)->resources[0])->buf->size ==
      si_resource(((struct vl_video_buffer *)out_surf)->resources[1])->buf->size);

   decode->dt_size = si_resource(((struct vl_video_buffer *)out_surf)->resources[0])->buf->size;

   decode->sct_size = 0;
   decode->sc_coeff_size = 0;

   decode->sw_ctxt_size = RDECODE_SESSION_CONTEXT_SIZE;
   decode->db_pitch = align(dec->base.width, dec->db_alignment);

   if ((((struct si_screen*)dec->screen)->info.vcn_ip_version >= VCN_3_0_0) &&
       (dec->stream_type == RDECODE_CODEC_VP9 || dec->stream_type == RDECODE_CODEC_AV1 ||
        dec->base.profile == PIPE_VIDEO_PROFILE_HEVC_MAIN_10))
      decode->db_aligned_height = align(dec->base.height, 64);

   decode->db_surf_tile_config = 0;
   decode->db_array_mode = dec->addr_gfx_mode;

   decode->dt_pitch = luma->surface.u.gfx9.surf_pitch * luma->surface.blk_w;
   decode->dt_uv_pitch = chroma->surface.u.gfx9.surf_pitch * chroma->surface.blk_w;

   if (luma->surface.meta_offset) {
      RVID_ERR("DCC surfaces not supported.\n");
      return NULL;
   }

   decode->dt_tiling_mode = 0;
   decode->dt_swizzle_mode = luma->surface.u.gfx9.swizzle_mode;
   decode->dt_array_mode = dec->addr_gfx_mode;
   decode->dt_field_mode = ((struct vl_video_buffer *)out_surf)->base.interlaced;
   decode->dt_surf_tile_config = 0;
   decode->dt_uv_surf_tile_config = 0;

   decode->dt_luma_top_offset = luma->surface.u.gfx9.surf_offset;
   decode->dt_chroma_top_offset = chroma->surface.u.gfx9.surf_offset;
   if (decode->dt_field_mode) {
      decode->dt_luma_bottom_offset =
         luma->surface.u.gfx9.surf_offset + luma->surface.u.gfx9.surf_slice_size;
      decode->dt_chroma_bottom_offset =
         chroma->surface.u.gfx9.surf_offset + chroma->surface.u.gfx9.surf_slice_size;
   } else {
      decode->dt_luma_bottom_offset = decode->dt_luma_top_offset;
      decode->dt_chroma_bottom_offset = decode->dt_chroma_top_offset;
   }
   if (dec->stream_type == RDECODE_CODEC_AV1)
      decode->db_pitch_uv = chroma->surface.u.gfx9.surf_pitch * chroma->surface.blk_w;

   if (encrypted) {
      assert(sscreen->info.has_tmz_support);
      set_drm_keys(drm, decrypt);
   }

   if (dec->dpb_type == DPB_DYNAMIC_TIER_1) {
      decode->decode_flags |= (RDECODE_FLAGS_USE_DYNAMIC_DPB_MASK | RDECODE_FLAGS_USE_PAL_MASK);
      // Add decode flag for RESIZE_DPB ,when we do resize
      if (dpb_resize == true)
        decode->decode_flags |= RDECODE_FLAGS_DPB_RESIZE_MASK;

      dynamic_dpb->dpbArraySize = NUM_VP9_REFS + 1;
      dynamic_dpb->dpbLumaPitch = align(dec->max_width, dec->db_alignment);
      dynamic_dpb->dpbLumaAlignedHeight = align(dec->max_height, dec->db_alignment);
      dynamic_dpb->dpbLumaAlignedSize =
         dynamic_dpb->dpbLumaPitch * dynamic_dpb->dpbLumaAlignedHeight;
      dynamic_dpb->dpbChromaPitch = dynamic_dpb->dpbLumaPitch >> 1;
      dynamic_dpb->dpbChromaAlignedHeight = dynamic_dpb->dpbLumaAlignedHeight >> 1;
      dynamic_dpb->dpbChromaAlignedSize =
         dynamic_dpb->dpbChromaPitch * dynamic_dpb->dpbChromaAlignedHeight * 2;
      dynamic_dpb->dpbReserved0[0] = dec->db_alignment;

      if (dec->base.profile == PIPE_VIDEO_PROFILE_VP9_PROFILE2) {
         dynamic_dpb->dpbLumaAlignedSize = dynamic_dpb->dpbLumaAlignedSize * 3 / 2;
         dynamic_dpb->dpbChromaAlignedSize = dynamic_dpb->dpbChromaAlignedSize * 3 / 2;
      }
   }

   if (u_reduce_video_profile(picture->profile) == PIPE_VIDEO_FORMAT_HEVC)
      get_h265_reflist(hevc_reflist, (struct pipe_h265_picture_desc *)picture);

   switch (u_reduce_video_profile(picture->profile)) {
   case PIPE_VIDEO_FORMAT_MPEG4_AVC: {
      rvcn_dec_message_avc_t avc = get_h264_msg(dec, target, (struct pipe_h264_picture_desc *)picture);
      memcpy(codec, (void *)&avc, sizeof(rvcn_dec_message_avc_t));
      index_codec->message_id = RDECODE_MESSAGE_AVC;
      break;
   }
   case PIPE_VIDEO_FORMAT_HEVC: {
      rvcn_dec_message_hevc_t hevc =
         get_h265_msg(dec, target, (struct pipe_h265_picture_desc *)picture);

      memcpy(codec, (void *)&hevc, sizeof(rvcn_dec_message_hevc_t));
      index_codec->message_id = RDECODE_MESSAGE_HEVC;
      break;
   }
   case PIPE_VIDEO_FORMAT_VC1: {
      rvcn_dec_message_vc1_t vc1 = get_vc1_msg((struct pipe_vc1_picture_desc *)picture);

      memcpy(codec, (void *)&vc1, sizeof(rvcn_dec_message_vc1_t));
      if ((picture->profile == PIPE_VIDEO_PROFILE_VC1_SIMPLE) ||
          (picture->profile == PIPE_VIDEO_PROFILE_VC1_MAIN)) {
         decode->width_in_samples = align(decode->width_in_samples, 16) / 16;
         decode->height_in_samples = align(decode->height_in_samples, 16) / 16;
      }
      index_codec->message_id = RDECODE_MESSAGE_VC1;
      break;
   }
   case PIPE_VIDEO_FORMAT_MPEG12: {
      rvcn_dec_message_mpeg2_vld_t mpeg2 =
         get_mpeg2_msg(dec, (struct pipe_mpeg12_picture_desc *)picture);

      memcpy(codec, (void *)&mpeg2, sizeof(rvcn_dec_message_mpeg2_vld_t));
      index_codec->message_id = RDECODE_MESSAGE_MPEG2_VLD;
      break;
   }
   case PIPE_VIDEO_FORMAT_MPEG4: {
      rvcn_dec_message_mpeg4_asp_vld_t mpeg4 =
         get_mpeg4_msg(dec, (struct pipe_mpeg4_picture_desc *)picture);

      memcpy(codec, (void *)&mpeg4, sizeof(rvcn_dec_message_mpeg4_asp_vld_t));
      index_codec->message_id = RDECODE_MESSAGE_MPEG4_ASP_VLD;
      break;
   }
   case PIPE_VIDEO_FORMAT_VP9: {
      rvcn_dec_message_vp9_t vp9 =
         get_vp9_msg(dec, target, (struct pipe_vp9_picture_desc *)picture);

      memcpy(codec, (void *)&vp9, sizeof(rvcn_dec_message_vp9_t));
      index_codec->message_id = RDECODE_MESSAGE_VP9;
      break;
   }
   case PIPE_VIDEO_FORMAT_AV1: {
      rvcn_dec_message_av1_t av1 =
         get_av1_msg(dec, target, (struct pipe_av1_picture_desc *)picture);

      memcpy(codec, (void *)&av1, sizeof(rvcn_dec_message_av1_t));
      index_codec->message_id = RDECODE_MESSAGE_AV1;

      if (dec->ctx.res == NULL) {
         unsigned frame_ctxt_size = dec->av1_version == RDECODE_AV1_VER_0
                                       ? align(sizeof(rvcn_av1_frame_context_t), 2048)
                                       : align(sizeof(rvcn_av1_vcn4_frame_context_t), 2048);

         unsigned ctx_size = (9 + 4) * frame_ctxt_size + 9 * 64 * 34 * 512 + 9 * 64 * 34 * 256 * 5;
         int num_64x64_CTB_8k = 68;
         int num_128x128_CTB_8k = 34;
         int sdb_pitch_64x64 = align(32 * num_64x64_CTB_8k, 256) * 2;
         int sdb_pitch_128x128 = align(32 * num_128x128_CTB_8k, 256) * 2;
         int sdb_lf_size_ctb_64x64 = sdb_pitch_64x64 * (align(1728, 64) / 64);
         int sdb_lf_size_ctb_128x128 = sdb_pitch_128x128 * (align(3008, 64) / 64);
         int sdb_superres_size_ctb_64x64 = sdb_pitch_64x64 * (align(3232, 64) / 64);
         int sdb_superres_size_ctb_128x128 = sdb_pitch_128x128 * (align(6208, 64) / 64);
         int sdb_output_size_ctb_64x64 = sdb_pitch_64x64 * (align(1312, 64) / 64);
         int sdb_output_size_ctb_128x128 = sdb_pitch_128x128 * (align(2336, 64) / 64);
         int sdb_fg_avg_luma_size_ctb_64x64 = sdb_pitch_64x64 * (align(384, 64) / 64);
         int sdb_fg_avg_luma_size_ctb_128x128 = sdb_pitch_128x128 * (align(640, 64) / 64);
         uint8_t *ptr;
         int i;
         struct rvcn_av1_prob_funcs prob;

         if (dec->av1_version == RDECODE_AV1_VER_0) {
            prob.init_mode_probs = rvcn_init_mode_probs;
            prob.init_mv_probs = rvcn_av1_init_mv_probs;
            prob.default_coef_probs = rvcn_av1_default_coef_probs;
         } else {
            prob.init_mode_probs = rvcn_vcn4_init_mode_probs;
            prob.init_mv_probs = rvcn_vcn4_av1_init_mv_probs;
            prob.default_coef_probs = rvcn_vcn4_av1_default_coef_probs;
         }

         ctx_size += (MAX2(sdb_lf_size_ctb_64x64, sdb_lf_size_ctb_128x128) +
                      MAX2(sdb_superres_size_ctb_64x64, sdb_superres_size_ctb_128x128) +
                      MAX2(sdb_output_size_ctb_64x64, sdb_output_size_ctb_128x128) +
                      MAX2(sdb_fg_avg_luma_size_ctb_64x64, sdb_fg_avg_luma_size_ctb_128x128)) * 2  + 68 * 512;

         if (!si_vid_create_buffer(dec->screen, &dec->ctx, ctx_size, PIPE_USAGE_DEFAULT))
            RVID_ERR("Can't allocated context buffer.\n");
         si_vid_clear_buffer(dec->base.context, &dec->ctx);

         ptr = dec->ws->buffer_map(dec->ws, dec->ctx.res->buf, &dec->cs, PIPE_MAP_WRITE | RADEON_MAP_TEMPORARY);

         for (i = 0; i < 4; ++i) {
            prob.init_mode_probs((void*)(ptr + i * frame_ctxt_size));
            prob.init_mv_probs((void*)(ptr + i * frame_ctxt_size));
            prob.default_coef_probs((void*)(ptr + i * frame_ctxt_size), i);
         }
         dec->ws->buffer_unmap(dec->ws, dec->ctx.res->buf);
      }

      break;
   }
   default:
      assert(0);
      return NULL;
   }

   if (dec->ctx.res)
      decode->hw_ctxt_size = dec->ctx.res->buf->size;

   if (dec->dpb_type == DPB_DYNAMIC_TIER_2)
      if (rvcn_dec_dynamic_dpb_t2_message(dec, decode, dynamic_dpb_t2, encrypted))
         return NULL;

   return luma->buffer.buf;
}

static void rvcn_dec_message_destroy(struct radeon_decoder *dec)
{
   rvcn_dec_message_header_t *header = dec->msg;

   memset(dec->msg, 0, sizeof(rvcn_dec_message_header_t));
   header->header_size = sizeof(rvcn_dec_message_header_t);
   header->total_size = sizeof(rvcn_dec_message_header_t) - sizeof(rvcn_dec_message_index_t);
   header->num_buffers = 0;
   header->msg_type = RDECODE_MSG_DESTROY;
   header->stream_handle = dec->stream_handle;
   header->status_report_feedback_number = 0;
}

static void rvcn_dec_message_feedback(struct radeon_decoder *dec)
{
   rvcn_dec_feedback_header_t *header = (void *)dec->fb;

   header->header_size = sizeof(rvcn_dec_feedback_header_t);
   header->total_size = sizeof(rvcn_dec_feedback_header_t);
   header->num_buffers = 0;
}

static void rvcn_dec_sq_tail(struct radeon_decoder *dec)
{
   if (dec->vcn_dec_sw_ring == false)
      return;

   rvcn_sq_tail(&dec->cs, &dec->sq);
}
/* flush IB to the hardware */
static int flush(struct radeon_decoder *dec, unsigned flags,
                 struct pipe_fence_handle **fence) {
   rvcn_dec_sq_tail(dec);

   return dec->ws->cs_flush(&dec->cs, flags, fence);
}

/* add a new set register command to the IB */
static void set_reg(struct radeon_decoder *dec, unsigned reg, uint32_t val)
{
   radeon_emit(&dec->cs, RDECODE_PKT0(reg >> 2, 0));
   radeon_emit(&dec->cs, val);
}

/* send a command to the VCPU through the GPCOM registers */
static void send_cmd(struct radeon_decoder *dec, unsigned cmd, struct pb_buffer_lean *buf, uint32_t off,
                     unsigned usage, enum radeon_bo_domain domain)
{
   uint64_t addr;

   dec->ws->cs_add_buffer(&dec->cs, buf, usage | RADEON_USAGE_SYNCHRONIZED, domain);
   addr = dec->ws->buffer_get_virtual_address(buf);
   addr = addr + off;

   if (dec->vcn_dec_sw_ring == false) {
      set_reg(dec, dec->reg.data0, addr);
      set_reg(dec, dec->reg.data1, addr >> 32);
      set_reg(dec, dec->reg.cmd, cmd << 1);
      return;
   }

   if (!dec->cs.current.cdw) {
      rvcn_sq_header(&dec->cs, &dec->sq, false);
      rvcn_decode_ib_package_t *ib_header =
         (rvcn_decode_ib_package_t *)&(dec->cs.current.buf[dec->cs.current.cdw]);

      ib_header->package_size = sizeof(struct rvcn_decode_buffer_s) +
         sizeof(struct rvcn_decode_ib_package_s);
      dec->cs.current.cdw++;
      ib_header->package_type = (RDECODE_IB_PARAM_DECODE_BUFFER);
      dec->cs.current.cdw++;

      dec->decode_buffer =
         (rvcn_decode_buffer_t *)&(dec->cs.current.buf[dec->cs.current.cdw]);

      dec->cs.current.cdw += sizeof(struct rvcn_decode_buffer_s) / 4;
      memset(dec->decode_buffer, 0, sizeof(struct rvcn_decode_buffer_s));
   }

   switch(cmd) {
      case RDECODE_CMD_MSG_BUFFER:
            dec->decode_buffer->valid_buf_flag |= RDECODE_CMDBUF_FLAGS_MSG_BUFFER;
            dec->decode_buffer->msg_buffer_address_hi = (addr >> 32);
            dec->decode_buffer->msg_buffer_address_lo = (addr);
         break;
      case RDECODE_CMD_DPB_BUFFER:
            dec->decode_buffer->valid_buf_flag |= (RDECODE_CMDBUF_FLAGS_DPB_BUFFER);
            dec->decode_buffer->dpb_buffer_address_hi = (addr >> 32);
            dec->decode_buffer->dpb_buffer_address_lo = (addr);
         break;
      case RDECODE_CMD_DECODING_TARGET_BUFFER:
            dec->decode_buffer->valid_buf_flag |= (RDECODE_CMDBUF_FLAGS_DECODING_TARGET_BUFFER);
            dec->decode_buffer->target_buffer_address_hi = (addr >> 32);
            dec->decode_buffer->target_buffer_address_lo = (addr);
         break;
      case RDECODE_CMD_FEEDBACK_BUFFER:
            dec->decode_buffer->valid_buf_flag |= (RDECODE_CMDBUF_FLAGS_FEEDBACK_BUFFER);
            dec->decode_buffer->feedback_buffer_address_hi = (addr >> 32);
            dec->decode_buffer->feedback_buffer_address_lo = (addr);
         break;
      case RDECODE_CMD_PROB_TBL_BUFFER:
            dec->decode_buffer->valid_buf_flag |= (RDECODE_CMDBUF_FLAGS_PROB_TBL_BUFFER);
            dec->decode_buffer->prob_tbl_buffer_address_hi = (addr >> 32);
            dec->decode_buffer->prob_tbl_buffer_address_lo = (addr);
         break;
      case RDECODE_CMD_SESSION_CONTEXT_BUFFER:
            dec->decode_buffer->valid_buf_flag |= (RDECODE_CMDBUF_FLAGS_SESSION_CONTEXT_BUFFER);
            dec->decode_buffer->session_contex_buffer_address_hi = (addr >> 32);
            dec->decode_buffer->session_contex_buffer_address_lo = (addr);
         break;
      case RDECODE_CMD_BITSTREAM_BUFFER:
            dec->decode_buffer->valid_buf_flag |= (RDECODE_CMDBUF_FLAGS_BITSTREAM_BUFFER);
            dec->decode_buffer->bitstream_buffer_address_hi = (addr >> 32);
            dec->decode_buffer->bitstream_buffer_address_lo = (addr);
         break;
      case RDECODE_CMD_IT_SCALING_TABLE_BUFFER:
            dec->decode_buffer->valid_buf_flag |= (RDECODE_CMDBUF_FLAGS_IT_SCALING_BUFFER);
            dec->decode_buffer->it_sclr_table_buffer_address_hi = (addr >> 32);
            dec->decode_buffer->it_sclr_table_buffer_address_lo = (addr);
         break;
      case RDECODE_CMD_CONTEXT_BUFFER:
            dec->decode_buffer->valid_buf_flag |= (RDECODE_CMDBUF_FLAGS_CONTEXT_BUFFER);
            dec->decode_buffer->context_buffer_address_hi = (addr >> 32);
            dec->decode_buffer->context_buffer_address_lo = (addr);
         break;
      default:
            printf("Not Support!");
   }
}

/* do the codec needs an IT buffer ?*/
static bool have_it(struct radeon_decoder *dec)
{
   return dec->stream_type == RDECODE_CODEC_H264_PERF || dec->stream_type == RDECODE_CODEC_H265;
}

/* do the codec needs an probs buffer? */
static bool have_probs(struct radeon_decoder *dec)
{
   return (dec->stream_type == RDECODE_CODEC_VP9 || dec->stream_type == RDECODE_CODEC_AV1);
}

/* map the next available message/feedback/itscaling buffer */
static void map_msg_fb_it_probs_buf(struct radeon_decoder *dec)
{
   struct rvid_buffer *buf;
   uint8_t *ptr;

   /* grab the current message/feedback buffer */
   buf = &dec->msg_fb_it_probs_buffers[dec->cur_buffer];

   /* and map it for CPU access */
   ptr =
      dec->ws->buffer_map(dec->ws, buf->res->buf, &dec->cs, PIPE_MAP_WRITE | RADEON_MAP_TEMPORARY);

   /* calc buffer offsets */
   dec->msg = ptr;

   dec->fb = (uint32_t *)(ptr + FB_BUFFER_OFFSET);
   if (have_it(dec))
      dec->it = (uint8_t *)(ptr + FB_BUFFER_OFFSET + FB_BUFFER_SIZE);
   else if (have_probs(dec))
      dec->probs = (uint8_t *)(ptr + FB_BUFFER_OFFSET + FB_BUFFER_SIZE);
}

/* unmap and send a message command to the VCPU */
static void send_msg_buf(struct radeon_decoder *dec)
{
   struct rvid_buffer *buf;

   /* ignore the request if message/feedback buffer isn't mapped */
   if (!dec->msg || !dec->fb)
      return;

   /* grab the current message buffer */
   buf = &dec->msg_fb_it_probs_buffers[dec->cur_buffer];

   /* unmap the buffer */
   dec->ws->buffer_unmap(dec->ws, buf->res->buf);
   dec->bs_ptr = NULL;
   dec->msg = NULL;
   dec->fb = NULL;
   dec->it = NULL;
   dec->probs = NULL;

   if (dec->sessionctx.res)
      send_cmd(dec, RDECODE_CMD_SESSION_CONTEXT_BUFFER, dec->sessionctx.res->buf, 0,
               RADEON_USAGE_READWRITE, RADEON_DOMAIN_VRAM);

   /* and send it to the hardware */
   send_cmd(dec, RDECODE_CMD_MSG_BUFFER, buf->res->buf, 0, RADEON_USAGE_READ, RADEON_DOMAIN_GTT);
}

/* cycle to the next set of buffers */
static void next_buffer(struct radeon_decoder *dec)
{
   ++dec->cur_buffer;
   dec->cur_buffer %= dec->num_dec_bufs;
}

static unsigned calc_ctx_size_h264_perf(struct radeon_decoder *dec)
{
   unsigned width_in_mb, height_in_mb, ctx_size;
   unsigned width = align(dec->base.width, VL_MACROBLOCK_WIDTH);
   unsigned height = align(dec->base.height, VL_MACROBLOCK_HEIGHT);

   unsigned max_references = dec->base.max_references + 1;

   // picture width & height in 16 pixel units
   width_in_mb = width / VL_MACROBLOCK_WIDTH;
   height_in_mb = align(height / VL_MACROBLOCK_HEIGHT, 2);

   unsigned fs_in_mb = width_in_mb * height_in_mb;
   unsigned num_dpb_buffer_lean;
   switch (dec->base.level) {
   case 30:
      num_dpb_buffer_lean = 8100 / fs_in_mb;
      break;
   case 31:
      num_dpb_buffer_lean = 18000 / fs_in_mb;
      break;
   case 32:
      num_dpb_buffer_lean = 20480 / fs_in_mb;
      break;
   case 41:
      num_dpb_buffer_lean = 32768 / fs_in_mb;
      break;
   case 42:
      num_dpb_buffer_lean = 34816 / fs_in_mb;
      break;
   case 50:
      num_dpb_buffer_lean = 110400 / fs_in_mb;
      break;
   case 51:
      num_dpb_buffer_lean = 184320 / fs_in_mb;
      break;
   default:
      num_dpb_buffer_lean = 184320 / fs_in_mb;
      break;
   }
   num_dpb_buffer_lean++;
   max_references = MAX2(MIN2(NUM_H264_REFS, num_dpb_buffer_lean), max_references);
   ctx_size = max_references * align(width_in_mb * height_in_mb * 192, 256);

   return ctx_size;
}

/* calculate size of reference picture buffer */
static unsigned calc_dpb_size(struct radeon_decoder *dec)
{
   unsigned width_in_mb, height_in_mb, image_size, dpb_size;

   // always align them to MB size for dpb calculation
   unsigned width = align(dec->base.width, VL_MACROBLOCK_WIDTH);
   unsigned height = align(dec->base.height, VL_MACROBLOCK_HEIGHT);

   // always one more for currently decoded picture
   unsigned max_references = dec->base.max_references + 1;

   // aligned size of a single frame
   image_size = align(width, 32) * height;
   image_size += image_size / 2;
   image_size = align(image_size, 1024);

   // picture width & height in 16 pixel units
   width_in_mb = width / VL_MACROBLOCK_WIDTH;
   height_in_mb = align(height / VL_MACROBLOCK_HEIGHT, 2);

   switch (u_reduce_video_profile(dec->base.profile)) {
   case PIPE_VIDEO_FORMAT_MPEG4_AVC: {
      unsigned fs_in_mb = width_in_mb * height_in_mb;
      unsigned num_dpb_buffer_lean;

      switch (dec->base.level) {
      case 30:
         num_dpb_buffer_lean = 8100 / fs_in_mb;
         break;
      case 31:
         num_dpb_buffer_lean = 18000 / fs_in_mb;
         break;
      case 32:
         num_dpb_buffer_lean = 20480 / fs_in_mb;
         break;
      case 41:
         num_dpb_buffer_lean = 32768 / fs_in_mb;
         break;
      case 42:
         num_dpb_buffer_lean = 34816 / fs_in_mb;
         break;
      case 50:
         num_dpb_buffer_lean = 110400 / fs_in_mb;
         break;
      case 51:
         num_dpb_buffer_lean = 184320 / fs_in_mb;
         break;
      default:
         num_dpb_buffer_lean = 184320 / fs_in_mb;
         break;
      }
      num_dpb_buffer_lean++;
      max_references = MAX2(MIN2(NUM_H264_REFS, num_dpb_buffer_lean), max_references);
      dpb_size = image_size * max_references;
      break;
   }

   case PIPE_VIDEO_FORMAT_HEVC:
      if (dec->base.width * dec->base.height >= 4096 * 2000)
         max_references = MAX2(max_references, 8);
      else
         max_references = MAX2(max_references, 17);

      width = align(width, 16);
      height = align(height, 16);
      if (dec->base.profile == PIPE_VIDEO_PROFILE_HEVC_MAIN_10)
         dpb_size = align((align(width, 64) * align(height, 64) * 9) / 4, 256) * max_references;
      else
         dpb_size = align((align(width, 32) * height * 3) / 2, 256) * max_references;
      break;

   case PIPE_VIDEO_FORMAT_VC1:
      // the firmware seems to always assume a minimum of ref frames
      max_references = MAX2(NUM_VC1_REFS, max_references);

      // reference picture buffer
      dpb_size = image_size * max_references;

      // CONTEXT_BUFFER
      dpb_size += width_in_mb * height_in_mb * 128;

      // IT surface buffer
      dpb_size += width_in_mb * 64;

      // DB surface buffer
      dpb_size += width_in_mb * 128;

      // BP
      dpb_size += align(MAX2(width_in_mb, height_in_mb) * 7 * 16, 64);
      break;

   case PIPE_VIDEO_FORMAT_MPEG12:
      // reference picture buffer, must be big enough for all frames
      dpb_size = image_size * NUM_MPEG2_REFS;
      break;

   case PIPE_VIDEO_FORMAT_MPEG4:
      // reference picture buffer
      dpb_size = image_size * max_references;

      // CM
      dpb_size += width_in_mb * height_in_mb * 64;

      // IT surface buffer
      dpb_size += align(width_in_mb * height_in_mb * 32, 64);

      dpb_size = MAX2(dpb_size, 30 * 1024 * 1024);
      break;

   case PIPE_VIDEO_FORMAT_VP9:
      max_references = MAX2(max_references, 9);

      if (dec->dpb_type == DPB_MAX_RES)
         dpb_size = (((struct si_screen *)dec->screen)->info.vcn_ip_version >= VCN_2_0_0)
            ? (8192 * 4320 * 3 / 2) * max_references
            : (4096 * 3000 * 3 / 2) * max_references;
      else
         dpb_size = (align(dec->base.width, dec->db_alignment) *
            align(dec->base.height, dec->db_alignment) * 3 / 2) * max_references;

      if (dec->base.profile == PIPE_VIDEO_PROFILE_VP9_PROFILE2)
         dpb_size = dpb_size * 3 / 2;
      break;

   case PIPE_VIDEO_FORMAT_AV1:
      max_references = MAX2(max_references, 9);
      dpb_size = 8192 * 4320 * 3 / 2 * max_references * 3 / 2;
      break;

   case PIPE_VIDEO_FORMAT_JPEG:
      dpb_size = 0;
      break;

   default:
      // something is missing here
      assert(0);

      // at least use a sane default value
      dpb_size = 32 * 1024 * 1024;
      break;
   }
   return dpb_size;
}

/**
 * destroy this video decoder
 */
static void radeon_dec_destroy(struct pipe_video_codec *decoder)
{
   struct radeon_decoder *dec = (struct radeon_decoder *)decoder;
   unsigned i;

   assert(decoder);

   if (dec->stream_type != RDECODE_CODEC_JPEG) {
      map_msg_fb_it_probs_buf(dec);
      rvcn_dec_message_destroy(dec);
      send_msg_buf(dec);
      flush(dec, 0, &dec->destroy_fence);
      dec->ws->fence_wait(dec->ws, dec->destroy_fence, PIPE_DEFAULT_DECODER_FEEDBACK_TIMEOUT_NS);
      dec->ws->fence_reference(dec->ws, &dec->destroy_fence, NULL);
   }

   dec->ws->fence_reference(dec->ws, &dec->prev_fence, NULL);
   dec->ws->cs_destroy(&dec->cs);

   if (dec->stream_type == RDECODE_CODEC_JPEG) {
      for (i = 0; i < dec->njctx; i++) {
         dec->ws->cs_destroy(&dec->jcs[i]);
         dec->ws->ctx_destroy(dec->jctx[i]);
      }
   }

   if (dec->msg_fb_it_probs_buffers && dec->bs_buffers) {
      for (i = 0; i < dec->num_dec_bufs; ++i) {
            si_vid_destroy_buffer(&dec->msg_fb_it_probs_buffers[i]);
            si_vid_destroy_buffer(&dec->bs_buffers[i]);
      }
      FREE(dec->msg_fb_it_probs_buffers);
      FREE(dec->bs_buffers);
   }
   dec->num_dec_bufs = 0;

   if (dec->dpb_type != DPB_DYNAMIC_TIER_2) {
      si_vid_destroy_buffer(&dec->dpb);
   } else {
      list_for_each_entry_safe(struct rvcn_dec_dynamic_dpb_t2, d, &dec->dpb_ref_list, list) {
         list_del(&d->list);
         si_vid_destroy_buffer(&d->dpb);
         FREE(d);
      }
   }
   si_vid_destroy_buffer(&dec->ctx);
   si_vid_destroy_buffer(&dec->sessionctx);

   FREE(dec->jcs);
   FREE(dec->jctx);
   FREE(dec);
}

/**
 * start decoding of a new frame
 */
static void radeon_dec_begin_frame(struct pipe_video_codec *decoder,
                                   struct pipe_video_buffer *target,
                                   struct pipe_picture_desc *picture)
{
   struct radeon_decoder *dec = (struct radeon_decoder *)decoder;
   uintptr_t frame;

   assert(decoder);

   frame = ++dec->frame_number;
   if (dec->stream_type != RDECODE_CODEC_VP9 && dec->stream_type != RDECODE_CODEC_AV1
                                             && dec->stream_type != RDECODE_CODEC_H264_PERF)
      vl_video_buffer_set_associated_data(target, decoder, (void *)frame,
                                          &radeon_dec_destroy_associated_data);

   dec->bs_size = 0;
   dec->bs_ptr = dec->ws->buffer_map(dec->ws, dec->bs_buffers[dec->cur_buffer].res->buf, &dec->cs,
                                     PIPE_MAP_WRITE | RADEON_MAP_TEMPORARY);
}

/**
 * decode a macroblock
 */
static void radeon_dec_decode_macroblock(struct pipe_video_codec *decoder,
                                         struct pipe_video_buffer *target,
                                         struct pipe_picture_desc *picture,
                                         const struct pipe_macroblock *macroblocks,
                                         unsigned num_macroblocks)
{
   /* not supported (yet) */
   assert(0);
}

/**
 * decode a bitstream
 */
static void radeon_dec_decode_bitstream(struct pipe_video_codec *decoder,
                                        struct pipe_video_buffer *target,
                                        struct pipe_picture_desc *picture, unsigned num_buffers,
                                        const void *const *buffers, const unsigned *sizes)
{
   struct radeon_decoder *dec = (struct radeon_decoder *)decoder;
   unsigned i;

   assert(decoder);

   if (!dec->bs_ptr)
      return;

   if (dec->bs_size && dec->stream_type == RDECODE_CODEC_AV1)
      return;

   unsigned long total_bs_size = dec->bs_size;
   for (i = 0; i < num_buffers; ++i)
      total_bs_size += sizes[i];

   struct rvid_buffer *buf = &dec->bs_buffers[dec->cur_buffer];

   if (total_bs_size > buf->res->buf->size) {
      dec->ws->buffer_unmap(dec->ws, buf->res->buf);
      dec->bs_ptr = NULL;
      if (!si_vid_resize_buffer(dec->screen, &dec->cs, buf, total_bs_size, NULL)) {
         RVID_ERR("Can't resize bitstream buffer!");
         return;
      }

      dec->bs_ptr = dec->ws->buffer_map(dec->ws, buf->res->buf, &dec->cs,
                                          PIPE_MAP_WRITE | RADEON_MAP_TEMPORARY);
      if (!dec->bs_ptr)
         return;

      dec->bs_ptr += dec->bs_size;
   }

   for (i = 0; i < num_buffers; ++i) {
      memcpy(dec->bs_ptr, buffers[i], sizes[i]);
      dec->bs_size += sizes[i];
      dec->bs_ptr += sizes[i];
   }
}

/**
 * send cmd for vcn dec
 */
void send_cmd_dec(struct radeon_decoder *dec, struct pipe_video_buffer *target,
                  struct pipe_picture_desc *picture)
{
   struct pb_buffer_lean *dt;
   struct rvid_buffer *msg_fb_it_probs_buf, *bs_buf;

   msg_fb_it_probs_buf = &dec->msg_fb_it_probs_buffers[dec->cur_buffer];
   bs_buf = &dec->bs_buffers[dec->cur_buffer];

   memset(dec->bs_ptr, 0, align(dec->bs_size, 128) - dec->bs_size);
   dec->ws->buffer_unmap(dec->ws, bs_buf->res->buf);
   dec->bs_ptr = NULL;

   map_msg_fb_it_probs_buf(dec);
   dt = rvcn_dec_message_decode(dec, target, picture);
   rvcn_dec_message_feedback(dec);
   send_msg_buf(dec);

   if (dec->dpb_type != DPB_DYNAMIC_TIER_2)
      send_cmd(dec, RDECODE_CMD_DPB_BUFFER, dec->dpb.res->buf, 0, RADEON_USAGE_READWRITE,
            RADEON_DOMAIN_VRAM);
   if (dec->ctx.res)
      send_cmd(dec, RDECODE_CMD_CONTEXT_BUFFER, dec->ctx.res->buf, 0, RADEON_USAGE_READWRITE,
               RADEON_DOMAIN_VRAM);
   send_cmd(dec, RDECODE_CMD_BITSTREAM_BUFFER, bs_buf->res->buf, 0, RADEON_USAGE_READ,
            RADEON_DOMAIN_GTT);
   send_cmd(dec, RDECODE_CMD_DECODING_TARGET_BUFFER, dt, 0, RADEON_USAGE_WRITE, RADEON_DOMAIN_VRAM);
   send_cmd(dec, RDECODE_CMD_FEEDBACK_BUFFER, msg_fb_it_probs_buf->res->buf, FB_BUFFER_OFFSET,
            RADEON_USAGE_WRITE, RADEON_DOMAIN_GTT);
   if (have_it(dec))
      send_cmd(dec, RDECODE_CMD_IT_SCALING_TABLE_BUFFER, msg_fb_it_probs_buf->res->buf,
               FB_BUFFER_OFFSET + FB_BUFFER_SIZE, RADEON_USAGE_READ, RADEON_DOMAIN_GTT);
   else if (have_probs(dec))
      send_cmd(dec, RDECODE_CMD_PROB_TBL_BUFFER, msg_fb_it_probs_buf->res->buf,
               FB_BUFFER_OFFSET + FB_BUFFER_SIZE, RADEON_USAGE_READ, RADEON_DOMAIN_GTT);

   if (dec->vcn_dec_sw_ring == false)
      set_reg(dec, dec->reg.cntl, 1);
}

/**
 * end decoding of the current frame
 */
static void radeon_dec_end_frame(struct pipe_video_codec *decoder, struct pipe_video_buffer *target,
                                 struct pipe_picture_desc *picture)
{
   struct radeon_decoder *dec = (struct radeon_decoder *)decoder;

   assert(decoder);

   if (!dec->bs_ptr)
      return;

   dec->send_cmd(dec, target, picture);
   flush(dec, PIPE_FLUSH_ASYNC, picture->fence);
   if (picture->fence)
      dec->ws->fence_reference(dec->ws, &dec->prev_fence, *picture->fence);
   next_buffer(dec);
}

/**
 * end decoding of the current jpeg frame
 */
static void radeon_dec_jpeg_end_frame(struct pipe_video_codec *decoder, struct pipe_video_buffer *target,
                                 struct pipe_picture_desc *picture)
{
   struct radeon_decoder *dec = (struct radeon_decoder *)decoder;
   struct pipe_mjpeg_picture_desc *pic = (struct pipe_mjpeg_picture_desc *)picture;

   assert(decoder);

   if (!dec->bs_ptr)
      return;

   dec->jpg.crop_x = ROUND_DOWN_TO(pic->picture_parameter.crop_x, VL_MACROBLOCK_WIDTH);
   dec->jpg.crop_y = ROUND_DOWN_TO(pic->picture_parameter.crop_y, VL_MACROBLOCK_HEIGHT);
   dec->jpg.crop_width = align(pic->picture_parameter.crop_width, VL_MACROBLOCK_WIDTH);
   dec->jpg.crop_height = align(pic->picture_parameter.crop_height, VL_MACROBLOCK_HEIGHT);
   if (dec->jpg.crop_x + dec->jpg.crop_width > pic->picture_parameter.picture_width)
      dec->jpg.crop_width = 0;
   if (dec->jpg.crop_y + dec->jpg.crop_height > pic->picture_parameter.picture_height)
      dec->jpg.crop_height = 0;
   dec->send_cmd(dec, target, picture);
   dec->ws->cs_flush(&dec->jcs[dec->cb_idx], PIPE_FLUSH_ASYNC, NULL);
   next_buffer(dec);
   dec->cb_idx = (dec->cb_idx+1) % dec->njctx;
}

/**
 * flush any outstanding command buffers to the hardware
 */
static void radeon_dec_flush(struct pipe_video_codec *decoder)
{
}

static int radeon_dec_get_decoder_fence(struct pipe_video_codec *decoder,
                                        struct pipe_fence_handle *fence,
                                        uint64_t timeout) {

   struct radeon_decoder *dec = (struct radeon_decoder *)decoder;

   return dec->ws->fence_wait(dec->ws, fence, timeout);
}

static void radeon_dec_destroy_fence(struct pipe_video_codec *decoder,
                                     struct pipe_fence_handle *fence)
{
   struct radeon_decoder *dec = (struct radeon_decoder *)decoder;

   dec->ws->fence_reference(dec->ws, &fence, NULL);
}

/**
 * update render list when target buffer got updated, use the existing
 * index and update the new buffer to associate with it.
 */
static void radeon_dec_update_render_list(struct pipe_video_codec *decoder,
                                          struct pipe_video_buffer *old,
                                          struct pipe_video_buffer *updated)
{
   struct radeon_decoder *dec = (struct radeon_decoder *)decoder;
   void *index = vl_video_buffer_get_associated_data(old, decoder);

   vl_video_buffer_set_associated_data(updated, decoder, index,
                                       old->destroy_associated_data);
   for (int i = 0; i < ARRAY_SIZE(dec->render_pic_list); ++i) {
      if (dec->render_pic_list[i] == old) {
         dec->render_pic_list[i] = updated;
         break;
      }
   }
}
/**
 * create and HW decoder
 */
struct pipe_video_codec *radeon_create_decoder(struct pipe_context *context,
                                               const struct pipe_video_codec *templ)
{
   struct si_context *sctx = (struct si_context *)context;
   struct radeon_winsys *ws = sctx->ws;
   unsigned width = templ->width, height = templ->height;
   unsigned bs_buf_size, stream_type = 0, ring = AMD_IP_VCN_DEC;
   struct radeon_decoder *dec;
   int r, i;

   switch (u_reduce_video_profile(templ->profile)) {
   case PIPE_VIDEO_FORMAT_MPEG12:
      if (templ->entrypoint > PIPE_VIDEO_ENTRYPOINT_BITSTREAM)
         return vl_create_mpeg12_decoder(context, templ);
      stream_type = RDECODE_CODEC_MPEG2_VLD;
      break;
   case PIPE_VIDEO_FORMAT_MPEG4:
      width = align(width, VL_MACROBLOCK_WIDTH);
      height = align(height, VL_MACROBLOCK_HEIGHT);
      stream_type = RDECODE_CODEC_MPEG4;
      break;
   case PIPE_VIDEO_FORMAT_VC1:
      stream_type = RDECODE_CODEC_VC1;
      break;
   case PIPE_VIDEO_FORMAT_MPEG4_AVC:
      width = align(width, VL_MACROBLOCK_WIDTH);
      height = align(height, VL_MACROBLOCK_HEIGHT);
      stream_type = RDECODE_CODEC_H264_PERF;
      break;
   case PIPE_VIDEO_FORMAT_HEVC:
      stream_type = RDECODE_CODEC_H265;
      break;
   case PIPE_VIDEO_FORMAT_VP9:
      stream_type = RDECODE_CODEC_VP9;
      break;
   case PIPE_VIDEO_FORMAT_AV1:
      stream_type = RDECODE_CODEC_AV1;
      break;
   case PIPE_VIDEO_FORMAT_JPEG:
      stream_type = RDECODE_CODEC_JPEG;
      ring = AMD_IP_VCN_JPEG;
      break;
   default:
      assert(0);
      break;
   }

   dec = CALLOC_STRUCT(radeon_decoder);

   if (!dec)
      return NULL;

   dec->base = *templ;
   dec->base.context = context;
   dec->base.width = width;
   dec->base.height = height;
   dec->max_width = width;
   dec->max_height = height;
   dec->base.destroy = radeon_dec_destroy;
   dec->base.begin_frame = radeon_dec_begin_frame;
   dec->base.decode_macroblock = radeon_dec_decode_macroblock;
   dec->base.decode_bitstream = radeon_dec_decode_bitstream;
   dec->base.end_frame = radeon_dec_end_frame;
   dec->base.flush = radeon_dec_flush;
   dec->base.get_decoder_fence = radeon_dec_get_decoder_fence;
   dec->base.destroy_fence = radeon_dec_destroy_fence;
   dec->base.update_decoder_target =  radeon_dec_update_render_list;

   dec->stream_type = stream_type;
   dec->stream_handle = si_vid_alloc_stream_handle();
   dec->screen = context->screen;
   dec->ws = ws;

   if (u_reduce_video_profile(templ->profile) != PIPE_VIDEO_FORMAT_JPEG &&
       (sctx->vcn_ip_ver >= VCN_4_0_0)) {
      dec->vcn_dec_sw_ring = true;
      ring = AMD_IP_VCN_UNIFIED;
   }

   dec->sq.ib_total_size_in_dw = NULL;
   dec->sq.ib_checksum = NULL;

   if (!ws->cs_create(&dec->cs, sctx->ctx, ring, NULL, NULL)) {
      RVID_ERR("Can't get command submission context.\n");
      goto error;
   }

   if (dec->stream_type == RDECODE_CODEC_JPEG) {

      if (sctx->vcn_ip_ver == VCN_2_5_0 || sctx->vcn_ip_ver == VCN_2_6_0)
         dec->njctx = 2;
      else if (sctx->vcn_ip_ver == VCN_4_0_3)
         dec->njctx = 24;
      else
         dec->njctx = 1;

      dec->jctx = (struct radeon_winsys_ctx **) CALLOC(dec->njctx,
                                                       sizeof(struct radeon_winsys_ctx *));
      dec->jcs = (struct radeon_cmdbuf *) CALLOC(dec->njctx, sizeof(struct radeon_cmdbuf));
      if(!dec->jctx || !dec->jcs)
         goto err;
      for (i = 0; i < dec->njctx; i++) {
      /* Initialize the context handle and the command stream. */
         dec->jctx[i] = dec->ws->ctx_create(dec->ws, RADEON_CTX_PRIORITY_MEDIUM,
                                            sctx->context_flags & PIPE_CONTEXT_LOSE_CONTEXT_ON_RESET);
         if (!sctx->ctx)
            goto error;
         if (!dec->ws->cs_create(&dec->jcs[i], dec->jctx[i], ring, NULL, NULL)) {
            RVID_ERR("Can't get additional command submission context for mJPEG.\n");
            goto error;
         }
      }
      dec->base.end_frame = radeon_dec_jpeg_end_frame;
      dec->cb_idx = 0;
   }

   for (i = 0; i < ARRAY_SIZE(dec->render_pic_list); i++)
      dec->render_pic_list[i] = NULL;

   if ((sctx->vcn_ip_ver >= VCN_3_0_0) && (stream_type == RDECODE_CODEC_H264_PERF)) {
      for (i = 0; i < ARRAY_SIZE(dec->h264_valid_ref_num); i++)
         dec->h264_valid_ref_num[i] = (unsigned) -1;
      for (i = 0; i < ARRAY_SIZE(dec->h264_valid_poc_num); i++)
         dec->h264_valid_poc_num[i] = (unsigned) -1;
   }

   if (dec->stream_type == RDECODE_CODEC_JPEG) {
      if (sctx->vcn_ip_ver == VCN_4_0_3)
         dec->num_dec_bufs = dec->njctx;
      else
         dec->num_dec_bufs = dec->njctx * NUM_BUFFERS;
   } else
      dec->num_dec_bufs = NUM_BUFFERS;

   bs_buf_size = align(width * height / 32, 128);
   dec->msg_fb_it_probs_buffers = (struct rvid_buffer *) CALLOC(dec->num_dec_bufs, sizeof(struct rvid_buffer));
   dec->bs_buffers = (struct rvid_buffer *) CALLOC(dec->num_dec_bufs, sizeof(struct rvid_buffer));
   if(!dec->msg_fb_it_probs_buffers || !dec->bs_buffers)
      goto error;

   for (i = 0; i < dec->num_dec_bufs; ++i) {
      unsigned msg_fb_it_probs_size = FB_BUFFER_OFFSET + FB_BUFFER_SIZE;
      if (have_it(dec))
         msg_fb_it_probs_size += IT_SCALING_TABLE_SIZE;
      else if (have_probs(dec))
         msg_fb_it_probs_size += (dec->stream_type == RDECODE_CODEC_VP9) ?
                                 VP9_PROBS_TABLE_SIZE :
                                 sizeof(rvcn_dec_av1_segment_fg_t);
      /* use vram to improve performance, workaround an unknown bug */
      if (!si_vid_create_buffer(dec->screen, &dec->msg_fb_it_probs_buffers[i], msg_fb_it_probs_size,
                                PIPE_USAGE_DEFAULT)) {
         RVID_ERR("Can't allocated message buffers.\n");
         goto error;
      }

      if (!si_vid_create_buffer(dec->screen, &dec->bs_buffers[i], bs_buf_size,
                                PIPE_USAGE_STAGING)) {
         RVID_ERR("Can't allocated bitstream buffers.\n");
         goto error;
      }

      si_vid_clear_buffer(context, &dec->msg_fb_it_probs_buffers[i]);
      si_vid_clear_buffer(context, &dec->bs_buffers[i]);

      if (have_probs(dec) && dec->stream_type == RDECODE_CODEC_VP9) {
         struct rvid_buffer *buf;
         void *ptr;

         buf = &dec->msg_fb_it_probs_buffers[i];
         ptr = dec->ws->buffer_map(dec->ws, buf->res->buf, &dec->cs,
                                   PIPE_MAP_WRITE | RADEON_MAP_TEMPORARY);
         ptr += FB_BUFFER_OFFSET + FB_BUFFER_SIZE;
         fill_probs_table(ptr);
         dec->ws->buffer_unmap(dec->ws, buf->res->buf);
         dec->bs_ptr = NULL;
      }
   }

   if ((sctx->vcn_ip_ver >= VCN_3_0_0) &&
         (stream_type == RDECODE_CODEC_VP9 ||
          stream_type == RDECODE_CODEC_AV1 ||
        ((stream_type == RDECODE_CODEC_H265) && templ->expect_chunked_decode) ||
        ((stream_type == RDECODE_CODEC_H264_PERF) && templ->expect_chunked_decode)))
      dec->dpb_type = DPB_DYNAMIC_TIER_2;
   else if (sctx->vcn_ip_ver <= VCN_2_6_0 && stream_type == RDECODE_CODEC_VP9)
      dec->dpb_type = DPB_DYNAMIC_TIER_1;
   else
      dec->dpb_type = DPB_MAX_RES;

   dec->db_alignment = (((struct si_screen *)dec->screen)->info.vcn_ip_version >= VCN_2_0_0 &&
                   dec->base.width > 32 && (dec->stream_type == RDECODE_CODEC_VP9 ||
                   dec->stream_type == RDECODE_CODEC_AV1 ||
                   dec->base.profile == PIPE_VIDEO_PROFILE_HEVC_MAIN_10)) ? 64 : 32;

   dec->dpb_size = calc_dpb_size(dec);

   if (!si_vid_create_buffer(dec->screen, &dec->sessionctx, RDECODE_SESSION_CONTEXT_SIZE,
                             PIPE_USAGE_DEFAULT)) {
      RVID_ERR("Can't allocated session ctx.\n");
      goto error;
   }
   si_vid_clear_buffer(context, &dec->sessionctx);

   dec->addr_gfx_mode = RDECODE_ARRAY_MODE_LINEAR;
   dec->av1_version = RDECODE_AV1_VER_0;

   switch (sctx->vcn_ip_ver) {
   case VCN_1_0_0:
   case VCN_1_0_1:
      dec->reg.data0 = RDECODE_VCN1_GPCOM_VCPU_DATA0;
      dec->reg.data1 = RDECODE_VCN1_GPCOM_VCPU_DATA1;
      dec->reg.cmd = RDECODE_VCN1_GPCOM_VCPU_CMD;
      dec->reg.cntl = RDECODE_VCN1_ENGINE_CNTL;
      dec->jpg_reg.version = RDECODE_JPEG_REG_VER_V1;
      break;
   case VCN_2_0_0:
   case VCN_2_0_2:
   case VCN_2_0_3:
   case VCN_2_2_0:
      dec->reg.data0 = RDECODE_VCN2_GPCOM_VCPU_DATA0;
      dec->reg.data1 = RDECODE_VCN2_GPCOM_VCPU_DATA1;
      dec->reg.cmd = RDECODE_VCN2_GPCOM_VCPU_CMD;
      dec->reg.cntl = RDECODE_VCN2_ENGINE_CNTL;
      dec->jpg_reg.version = RDECODE_JPEG_REG_VER_V2;
      break;
   case VCN_2_5_0:
   case VCN_2_6_0:
   case VCN_3_0_0:
   case VCN_3_0_2:
   case VCN_3_0_16:
   case VCN_3_0_33:
   case VCN_3_1_1:
   case VCN_3_1_2:
      dec->reg.data0 = RDECODE_VCN2_5_GPCOM_VCPU_DATA0;
      dec->reg.data1 = RDECODE_VCN2_5_GPCOM_VCPU_DATA1;
      dec->reg.cmd = RDECODE_VCN2_5_GPCOM_VCPU_CMD;
      dec->reg.cntl = RDECODE_VCN2_5_ENGINE_CNTL;
      dec->jpg_reg.version = RDECODE_JPEG_REG_VER_V2;
      break;
   case VCN_4_0_3:
      dec->jpg_reg.version = RDECODE_JPEG_REG_VER_V3;
      dec->addr_gfx_mode = RDECODE_ARRAY_MODE_ADDRLIB_SEL_GFX9;
      dec->av1_version = RDECODE_AV1_VER_1;
      break;
   case VCN_4_0_0:
   case VCN_4_0_2:
   case VCN_4_0_4:
   case VCN_4_0_5:
      dec->jpg_reg.version = RDECODE_JPEG_REG_VER_V2;
      dec->addr_gfx_mode = RDECODE_ARRAY_MODE_ADDRLIB_SEL_GFX11;
      dec->av1_version = RDECODE_AV1_VER_1;
      break;
   default:
      RVID_ERR("VCN is not supported.\n");
      goto error;
   }

   if (dec->stream_type != RDECODE_CODEC_JPEG) {
      map_msg_fb_it_probs_buf(dec);
      rvcn_dec_message_create(dec);
      send_msg_buf(dec);
      r = flush(dec, 0, NULL);
      if (r)
         goto error;
   } else if (dec->jpg_reg.version != RDECODE_JPEG_REG_VER_V1) {
      dec->jpg_reg.jrbc_ib_cond_rd_timer = vcnipUVD_JRBC_IB_COND_RD_TIMER;
      dec->jpg_reg.jrbc_ib_ref_data = vcnipUVD_JRBC_IB_REF_DATA;
      dec->jpg_reg.jpeg_rb_base = vcnipUVD_JPEG_RB_BASE;
      dec->jpg_reg.jpeg_rb_size = vcnipUVD_JPEG_RB_SIZE;
      dec->jpg_reg.jpeg_rb_wptr = vcnipUVD_JPEG_RB_WPTR;
      dec->jpg_reg.jpeg_int_en = vcnipUVD_JPEG_INT_EN;
      dec->jpg_reg.jpeg_cntl = vcnipUVD_JPEG_CNTL;
      dec->jpg_reg.jpeg_rb_rptr = vcnipUVD_JPEG_RB_RPTR;
      if (dec->jpg_reg.version == RDECODE_JPEG_REG_VER_V2) {
         dec->jpg_reg.jpeg_dec_soft_rst = vcnipUVD_JPEG_DEC_SOFT_RST;
         dec->jpg_reg.lmi_jpeg_read_64bit_bar_high = vcnipUVD_LMI_JPEG_READ_64BIT_BAR_HIGH;
         dec->jpg_reg.lmi_jpeg_read_64bit_bar_low = vcnipUVD_LMI_JPEG_READ_64BIT_BAR_LOW;
         dec->jpg_reg.jpeg_pitch = vcnipUVD_JPEG_PITCH;
         dec->jpg_reg.jpeg_uv_pitch = vcnipUVD_JPEG_UV_PITCH;
         dec->jpg_reg.dec_addr_mode = vcnipJPEG_DEC_ADDR_MODE;
         dec->jpg_reg.dec_y_gfx10_tiling_surface = vcnipJPEG_DEC_Y_GFX10_TILING_SURFACE;
         dec->jpg_reg.dec_uv_gfx10_tiling_surface = vcnipJPEG_DEC_UV_GFX10_TILING_SURFACE;
         dec->jpg_reg.lmi_jpeg_write_64bit_bar_high = vcnipUVD_LMI_JPEG_WRITE_64BIT_BAR_HIGH;
         dec->jpg_reg.lmi_jpeg_write_64bit_bar_low = vcnipUVD_LMI_JPEG_WRITE_64BIT_BAR_LOW;
         dec->jpg_reg.jpeg_tier_cntl2 = vcnipUVD_JPEG_TIER_CNTL2;
         dec->jpg_reg.jpeg_outbuf_cntl = vcnipUVD_JPEG_OUTBUF_CNTL;
         dec->jpg_reg.jpeg_outbuf_rptr = vcnipUVD_JPEG_OUTBUF_RPTR;
         dec->jpg_reg.jpeg_outbuf_wptr = vcnipUVD_JPEG_OUTBUF_WPTR;
         dec->jpg_reg.jpeg_index = vcnipUVD_JPEG_INDEX;
         dec->jpg_reg.jpeg_data = vcnipUVD_JPEG_DATA;
      } else {
         dec->jpg_reg.jpeg_dec_soft_rst = vcnipUVD_JPEG_DEC_SOFT_RST_1;
         dec->jpg_reg.lmi_jpeg_read_64bit_bar_high = vcnipUVD_LMI_JPEG_READ_64BIT_BAR_HIGH_1;
         dec->jpg_reg.lmi_jpeg_read_64bit_bar_low = vcnipUVD_LMI_JPEG_READ_64BIT_BAR_LOW_1;
         dec->jpg_reg.jpeg_pitch = vcnipUVD_JPEG_PITCH_1;
         dec->jpg_reg.jpeg_uv_pitch = vcnipUVD_JPEG_UV_PITCH_1;
         dec->jpg_reg.dec_addr_mode = vcnipJPEG_DEC_ADDR_MODE_1;
         dec->jpg_reg.dec_y_gfx10_tiling_surface = vcnipJPEG_DEC_Y_GFX10_TILING_SURFACE_1;
         dec->jpg_reg.dec_uv_gfx10_tiling_surface = vcnipJPEG_DEC_UV_GFX10_TILING_SURFACE_1;
         dec->jpg_reg.lmi_jpeg_write_64bit_bar_high = vcnipUVD_LMI_JPEG_WRITE_64BIT_BAR_HIGH_1;
         dec->jpg_reg.lmi_jpeg_write_64bit_bar_low = vcnipUVD_LMI_JPEG_WRITE_64BIT_BAR_LOW_1;
         dec->jpg_reg.jpeg_tier_cntl2 = vcnipUVD_JPEG_TIER_CNTL2_1;
         dec->jpg_reg.jpeg_outbuf_cntl = vcnipUVD_JPEG_OUTBUF_CNTL_1;
         dec->jpg_reg.jpeg_outbuf_rptr = vcnipUVD_JPEG_OUTBUF_RPTR_1;
         dec->jpg_reg.jpeg_outbuf_wptr = vcnipUVD_JPEG_OUTBUF_WPTR_1;
         dec->jpg_reg.jpeg_luma_base0_0 = vcnipUVD_JPEG_LUMA_BASE0_0;
         dec->jpg_reg.jpeg_chroma_base0_0 = vcnipUVD_JPEG_CHROMA_BASE0_0;
         dec->jpg_reg.jpeg_chromav_base0_0 = vcnipUVD_JPEG_CHROMAV_BASE0_0;
      }
   }

   next_buffer(dec);

   if (stream_type == RDECODE_CODEC_JPEG)
      dec->send_cmd = send_cmd_jpeg;
   else
      dec->send_cmd = send_cmd_dec;


   if (dec->dpb_type == DPB_DYNAMIC_TIER_2) {
      list_inithead(&dec->dpb_ref_list);
      list_inithead(&dec->dpb_unref_list);
   }

   dec->tmz_ctx = sctx->vcn_ip_ver < VCN_2_2_0 && sctx->vcn_ip_ver != VCN_UNKNOWN;

   return &dec->base;

error:
   dec->ws->cs_destroy(&dec->cs);

   if (dec->stream_type == RDECODE_CODEC_JPEG) {
      for (i = 0; i < dec->njctx; i++) {
         dec->ws->cs_destroy(&dec->jcs[i]);
         dec->ws->ctx_destroy(dec->jctx[i]);
      }
   }

   if (dec->msg_fb_it_probs_buffers && dec->bs_buffers) {
      for (i = 0; i < dec->num_dec_bufs; ++i) {
            si_vid_destroy_buffer(&dec->msg_fb_it_probs_buffers[i]);
            si_vid_destroy_buffer(&dec->bs_buffers[i]);
      }
      FREE(dec->msg_fb_it_probs_buffers);
      FREE(dec->bs_buffers);
   }

   if (dec->dpb_type != DPB_DYNAMIC_TIER_2)
      si_vid_destroy_buffer(&dec->dpb);
   si_vid_destroy_buffer(&dec->ctx);
   si_vid_destroy_buffer(&dec->sessionctx);

err:
   if (dec->jcs)
      FREE(dec->jcs);
   if (dec->jctx)
      FREE(dec->jctx);
   FREE(dec);

   return NULL;
}
