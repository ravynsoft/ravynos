/**************************************************************************
 *
 * Copyright 2016 Advanced Micro Devices, Inc.
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

#include "pipe/p_video_codec.h"
#include "util/u_memory.h"
#include "util/u_video.h"
#include "util/vl_rbsp.h"

#include "entrypoint.h"
#include "vid_dec.h"

#define DPB_MAX_SIZE 32
#define MAX_NUM_REF_PICS 16

enum {
   NAL_UNIT_TYPE_TRAIL_N = 0,
   NAL_UNIT_TYPE_TRAIL_R = 1,
   NAL_UNIT_TYPE_TSA_N = 2,
   NAL_UNIT_TYPE_TSA_R = 3,
   NAL_UNIT_TYPE_STSA_N = 4,
   NAL_UNIT_TYPE_STSA_R = 5,
   NAL_UNIT_TYPE_RADL_N = 6,
   NAL_UNIT_TYPE_RADL_R = 7,
   NAL_UNIT_TYPE_RASL_N = 8,
   NAL_UNIT_TYPE_RASL_R = 9,
   NAL_UNIT_TYPE_BLA_W_LP = 16,
   NAL_UNIT_TYPE_BLA_W_RADL = 17,
   NAL_UNIT_TYPE_BLA_N_LP =  18,
   NAL_UNIT_TYPE_IDR_W_RADL = 19,
   NAL_UNIT_TYPE_IDR_N_LP = 20,
   NAL_UNIT_TYPE_CRA = 21,
   NAL_UNIT_TYPE_SPS = 33,
   NAL_UNIT_TYPE_PPS = 34,
};

static const uint8_t Default_8x8_Intra[64] = {
   16, 16, 16, 16, 17, 18, 21, 24,
   16, 16, 16, 16, 17, 19, 22, 25,
   16, 16, 17, 18, 20, 22, 25, 29,
   16, 16, 18, 21, 24, 27, 31, 36,
   17, 17, 20, 24, 30, 35, 41, 47,
   18, 19, 22, 27, 35, 44, 54, 65,
   21, 22, 25, 31, 41, 54, 70, 88,
   24, 25, 29, 36, 47, 65, 88, 115
};

static const uint8_t Default_8x8_Inter[64] = {
   16, 16, 16, 16, 17, 18, 20, 24,
   16, 16, 16, 17, 18, 20, 24, 25,
   16, 16, 17, 18, 20, 24, 25, 28,
   16, 17, 18, 20, 24, 25, 28, 33,
   17, 18, 20, 24, 25, 28, 33, 41,
   18, 20, 24, 25, 28, 33, 41, 54,
   20, 24, 25, 28, 33, 41, 54, 71,
   24, 25, 28, 33, 41, 54, 71, 91
};

struct dpb_list {
   struct list_head list;
   struct pipe_video_buffer *buffer;
   OMX_TICKS timestamp;
   unsigned poc;
};

struct ref_pic_set {
  unsigned  num_pics;
  unsigned  num_neg_pics;
  unsigned  num_pos_pics;
  unsigned  num_delta_poc;
  int  delta_poc[MAX_NUM_REF_PICS];
  bool used[MAX_NUM_REF_PICS];
};

static bool is_idr_picture(unsigned nal_unit_type)
{
   return (nal_unit_type == NAL_UNIT_TYPE_IDR_W_RADL ||
           nal_unit_type == NAL_UNIT_TYPE_IDR_N_LP);
}

/* broken link access picture */
static bool is_bla_picture(unsigned nal_unit_type)
{
   return (nal_unit_type == NAL_UNIT_TYPE_BLA_W_LP ||
           nal_unit_type == NAL_UNIT_TYPE_BLA_W_RADL ||
           nal_unit_type == NAL_UNIT_TYPE_BLA_N_LP);
}

/* random access point picture */
static bool is_rap_picture(unsigned nal_unit_type)
{
   return (nal_unit_type >= NAL_UNIT_TYPE_BLA_W_LP &&
           nal_unit_type <= NAL_UNIT_TYPE_CRA);
}

static bool is_slice_picture(unsigned nal_unit_type)
{
   return (nal_unit_type <= NAL_UNIT_TYPE_RASL_R ||
           is_rap_picture(nal_unit_type));
}

static void set_poc(vid_dec_PrivateType *priv,
                    unsigned nal_unit_type, int i)
{
   priv->picture.h265.CurrPicOrderCntVal = i;

   if (priv->codec_data.h265.temporal_id == 0 &&
       (nal_unit_type == NAL_UNIT_TYPE_TRAIL_R ||
        nal_unit_type == NAL_UNIT_TYPE_TSA_R ||
        nal_unit_type == NAL_UNIT_TYPE_STSA_R ||
        is_rap_picture(nal_unit_type)))
      priv->codec_data.h265.slice_prev_poc = i;
}

static unsigned get_poc(vid_dec_PrivateType *priv)
{
   return priv->picture.h265.CurrPicOrderCntVal;
}

static void profile_tier(struct vl_rbsp *rbsp)
{
   int i;

   /* general_profile_space */
   vl_rbsp_u(rbsp, 2);

   /* general_tier_flag */
   vl_rbsp_u(rbsp, 1);

   /* general_profile_idc */
   vl_rbsp_u(rbsp, 5);

   /* general_profile_compatibility_flag */
   for(i = 0; i < 32; ++i)
      vl_rbsp_u(rbsp, 1);

   /* general_progressive_source_flag */
   vl_rbsp_u(rbsp, 1);

   /* general_interlaced_source_flag */
   vl_rbsp_u(rbsp, 1);

   /* general_non_packed_constraint_flag */
   vl_rbsp_u(rbsp, 1);

   /* general_frame_only_constraint_flag */
   vl_rbsp_u(rbsp, 1);

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

   /* general_level_idc */
   level_idc = vl_rbsp_u(rbsp, 8);

   for (i = 0; i < max_sublayers_minus1; ++i) {
      sub_layer_profile_present_flag[i] = vl_rbsp_u(rbsp, 1);
      sub_layer_level_present_flag[i] = vl_rbsp_u(rbsp, 1);
   }

   if (max_sublayers_minus1 > 0)
      for (i = max_sublayers_minus1; i < 8; ++i)
         /* reserved_zero_2bits */
         vl_rbsp_u(rbsp, 2);

   for (i = 0; i < max_sublayers_minus1; ++i) {
      if (sub_layer_profile_present_flag[i])
         profile_tier(rbsp);

      if (sub_layer_level_present_flag[i])
         /* sub_layer_level_idc */
         vl_rbsp_u(rbsp, 8);
   }

   return level_idc;
}

static void scaling_list_data(vid_dec_PrivateType *priv,
                              struct vl_rbsp *rbsp, struct pipe_h265_sps *sps)
{
   unsigned size_id, matrix_id;
   unsigned scaling_list_len[4] = { 16, 64, 64, 64 };
   uint8_t scaling_list4x4[6][64] = {  };
   int i;

   uint8_t (*scaling_list_data[4])[6][64] = {
        (uint8_t (*)[6][64])scaling_list4x4,
        (uint8_t (*)[6][64])sps->ScalingList8x8,
        (uint8_t (*)[6][64])sps->ScalingList16x16,
        (uint8_t (*)[6][64])sps->ScalingList32x32
   };
   uint8_t (*scaling_list_dc_coeff[2])[6] = {
      (uint8_t (*)[6])sps->ScalingListDCCoeff16x16,
      (uint8_t (*)[6])sps->ScalingListDCCoeff32x32
   };

   for (size_id = 0; size_id < 4; ++size_id) {

      for (matrix_id = 0; matrix_id < ((size_id == 3) ? 2 : 6); ++matrix_id) {
         bool scaling_list_pred_mode_flag = vl_rbsp_u(rbsp, 1);

         if (!scaling_list_pred_mode_flag) {
            /* scaling_list_pred_matrix_id_delta */;
            unsigned matrix_id_with_delta = matrix_id - vl_rbsp_ue(rbsp);

            if (matrix_id != matrix_id_with_delta) {
               memcpy((*scaling_list_data[size_id])[matrix_id],
                      (*scaling_list_data[size_id])[matrix_id_with_delta],
                      scaling_list_len[size_id]);
               if (size_id > 1)
                  (*scaling_list_dc_coeff[size_id - 2])[matrix_id] =
                     (*scaling_list_dc_coeff[size_id - 2])[matrix_id_with_delta];
            } else {
               const uint8_t *d;

               if (size_id == 0)
                  memset((*scaling_list_data[0])[matrix_id], 16, 16);
               else {
                  if (size_id < 3)
                     d = (matrix_id < 3) ? Default_8x8_Intra : Default_8x8_Inter;
                  else
                     d = (matrix_id < 1) ? Default_8x8_Intra : Default_8x8_Inter;
                  memcpy((*scaling_list_data[size_id])[matrix_id], d,
                         scaling_list_len[size_id]);
               }
               if (size_id > 1)
                  (*scaling_list_dc_coeff[size_id - 2])[matrix_id] = 16;
            }
         } else {
            int next_coef = 8;
            int coef_num = MIN2(64, (1 << (4 + (size_id << 1))));

            if (size_id > 1) {
               /* scaling_list_dc_coef_minus8 */
               next_coef = vl_rbsp_se(rbsp) + 8;
               (*scaling_list_dc_coeff[size_id - 2])[matrix_id] = next_coef;
            }

            for (i = 0; i < coef_num; ++i) {
               /* scaling_list_delta_coef */
               next_coef = (next_coef + vl_rbsp_se(rbsp) + 256) % 256;
               (*scaling_list_data[size_id])[matrix_id][i] = next_coef;
            }
         }
      }
   }

   for (i = 0; i < 6; ++i)
      memcpy(sps->ScalingList4x4[i], scaling_list4x4[i], 16);

   return;
}

static void st_ref_pic_set(vid_dec_PrivateType *priv, struct vl_rbsp *rbsp,
                           struct ref_pic_set *rps, struct pipe_h265_sps *sps,
                           unsigned idx)
{
   bool inter_rps_pred_flag;
   unsigned delta_idx_minus1;
   int delta_poc;
   int i;

   inter_rps_pred_flag = (idx != 0) ? (vl_rbsp_u(rbsp, 1)) : false;

   if (inter_rps_pred_flag) {
      struct ref_pic_set *ref_rps;
      unsigned sign, abs;
      int delta_rps;
      bool used;
      int j;

      if (idx == sps->num_short_term_ref_pic_sets)
         delta_idx_minus1 = vl_rbsp_ue(rbsp);
      else
         delta_idx_minus1 = 0;

      ref_rps = (struct ref_pic_set *)
         priv->codec_data.h265.ref_pic_set_list + idx - (delta_idx_minus1 + 1);

      /* delta_rps_sign */
      sign = vl_rbsp_u(rbsp, 1);
      /* abs_delta_rps_minus1 */
      abs = vl_rbsp_ue(rbsp);
      delta_rps = (1 - 2 * sign) * (abs + 1);

      rps->num_neg_pics = 0;
      rps->num_pos_pics = 0;
      rps->num_pics = 0;

      for(i = 0 ; i <= ref_rps->num_pics; ++i) {
         /* used_by_curr_pic_flag */
         if (!vl_rbsp_u(rbsp, 1))
            /* use_delta_flag */
            vl_rbsp_u(rbsp, 1);
         else {
            delta_poc = delta_rps +
               ((i < ref_rps->num_pics)? ref_rps->delta_poc[i] : 0);
            rps->delta_poc[rps->num_pics] = delta_poc;
            rps->used[rps->num_pics] = true;
            if (delta_poc < 0)
               rps->num_neg_pics++;
            else
               rps->num_pos_pics++;
            rps->num_pics++;
         }
      }

      rps->num_delta_poc = ref_rps->num_pics;

      /* sort delta poc */
      for (i = 1; i < rps->num_pics; ++i) {
         delta_poc = rps->delta_poc[i];
         used = rps->used[i];
         for (j = i - 1; j >= 0; j--) {
            if (delta_poc < rps->delta_poc[j]) {
               rps->delta_poc[j + 1] = rps->delta_poc[j];
               rps->used[j + 1] = rps->used[j];
               rps->delta_poc[j] = delta_poc;
               rps->used[j] = used;
            }
         }
      }

      for (i = 0 , j = rps->num_neg_pics - 1;
           i < rps->num_neg_pics >> 1; i++, j--) {
         delta_poc = rps->delta_poc[i];
         used = rps->used[i];
         rps->delta_poc[i] = rps->delta_poc[j];
         rps->used[i] = rps->used[j];
         rps->delta_poc[j] = delta_poc;
         rps->used[j] = used;
      }
   } else {
      /* num_negative_pics */
      rps->num_neg_pics = vl_rbsp_ue(rbsp);
      /* num_positive_pics */
      rps->num_pos_pics = vl_rbsp_ue(rbsp);
      rps->num_pics = rps->num_neg_pics + rps->num_pos_pics;

      delta_poc = 0;
      for(i = 0 ; i < rps->num_neg_pics; ++i) {
         /* delta_poc_s0_minus1 */
         delta_poc -= (vl_rbsp_ue(rbsp) + 1);
         rps->delta_poc[i] = delta_poc;
         /* used_by_curr_pic_s0_flag */
         rps->used[i] = vl_rbsp_u(rbsp, 1);
      }

      delta_poc = 0;
      for(i = rps->num_neg_pics; i < rps->num_pics; ++i) {
         /* delta_poc_s1_minus1 */
         delta_poc += (vl_rbsp_ue(rbsp) + 1);
         rps->delta_poc[i] = delta_poc;
         /* used_by_curr_pic_s1_flag */
         rps->used[i] = vl_rbsp_u(rbsp, 1);
      }
   }
}

static struct pipe_h265_sps *seq_parameter_set_id(vid_dec_PrivateType *priv,
                                                  struct vl_rbsp *rbsp)
{
   unsigned id = vl_rbsp_ue(rbsp);

   if (id >= ARRAY_SIZE(priv->codec_data.h265.sps))
      return NULL;

   return &priv->codec_data.h265.sps[id];
}

static void seq_parameter_set(vid_dec_PrivateType *priv, struct vl_rbsp *rbsp)
{
   struct pipe_h265_sps *sps;
   int sps_max_sub_layers_minus1;
   unsigned i;

   /* sps_video_parameter_set_id */
   vl_rbsp_u(rbsp, 4);

   /* sps_max_sub_layers_minus1 */
   sps_max_sub_layers_minus1 = vl_rbsp_u(rbsp, 3);

   assert(sps_max_sub_layers_minus1 <= 6);

   /* sps_temporal_id_nesting_flag */
   vl_rbsp_u(rbsp, 1);

   priv->codec_data.h265.level_idc =
      profile_tier_level(rbsp, sps_max_sub_layers_minus1);

   sps = seq_parameter_set_id(priv, rbsp);
   if (!sps)
      return;

   memset(sps, 0, sizeof(*sps));

   sps->chroma_format_idc = vl_rbsp_ue(rbsp);

   if (sps->chroma_format_idc == 3)
      sps->separate_colour_plane_flag = vl_rbsp_u(rbsp, 1);

   priv->codec_data.h265.pic_width_in_luma_samples =
      sps->pic_width_in_luma_samples = vl_rbsp_ue(rbsp);

   priv->codec_data.h265.pic_height_in_luma_samples =
      sps->pic_height_in_luma_samples = vl_rbsp_ue(rbsp);

   /* conformance_window_flag */
   if (vl_rbsp_u(rbsp, 1)) {
      /* conf_win_left_offset */
      vl_rbsp_ue(rbsp);
      /* conf_win_right_offset */
      vl_rbsp_ue(rbsp);
      /* conf_win_top_offset */
      vl_rbsp_ue(rbsp);
      /* conf_win_bottom_offset */
      vl_rbsp_ue(rbsp);
   }

   sps->bit_depth_luma_minus8 = vl_rbsp_ue(rbsp);
   sps->bit_depth_chroma_minus8 = vl_rbsp_ue(rbsp);
   sps->log2_max_pic_order_cnt_lsb_minus4 = vl_rbsp_ue(rbsp);

   /* sps_sub_layer_ordering_info_present_flag */
   i  = vl_rbsp_u(rbsp, 1) ? 0 : sps_max_sub_layers_minus1;
   for (; i <= sps_max_sub_layers_minus1; ++i) {
      sps->sps_max_dec_pic_buffering_minus1 = vl_rbsp_ue(rbsp);
      /* sps_max_num_reorder_pics */
      vl_rbsp_ue(rbsp);
      /* sps_max_latency_increase_plus */
      vl_rbsp_ue(rbsp);
   }

   sps->log2_min_luma_coding_block_size_minus3 = vl_rbsp_ue(rbsp);
   sps->log2_diff_max_min_luma_coding_block_size = vl_rbsp_ue(rbsp);
   sps->log2_min_transform_block_size_minus2 = vl_rbsp_ue(rbsp);
   sps->log2_diff_max_min_transform_block_size = vl_rbsp_ue(rbsp);
   sps->max_transform_hierarchy_depth_inter = vl_rbsp_ue(rbsp);
   sps->max_transform_hierarchy_depth_intra = vl_rbsp_ue(rbsp);

   sps->scaling_list_enabled_flag = vl_rbsp_u(rbsp, 1);
   if (sps->scaling_list_enabled_flag)
      /* sps_scaling_list_data_present_flag */
      if (vl_rbsp_u(rbsp, 1))
         scaling_list_data(priv, rbsp, sps);

   sps->amp_enabled_flag = vl_rbsp_u(rbsp, 1);
   sps->sample_adaptive_offset_enabled_flag = vl_rbsp_u(rbsp, 1);
   sps->pcm_enabled_flag = vl_rbsp_u(rbsp, 1);
   if (sps->pcm_enabled_flag) {
      sps->pcm_sample_bit_depth_luma_minus1 = vl_rbsp_u(rbsp, 4);
      sps->pcm_sample_bit_depth_chroma_minus1 = vl_rbsp_u(rbsp, 4);
      sps->log2_min_pcm_luma_coding_block_size_minus3 = vl_rbsp_ue(rbsp);
      sps->log2_diff_max_min_pcm_luma_coding_block_size = vl_rbsp_ue(rbsp);
      sps->pcm_loop_filter_disabled_flag = vl_rbsp_u(rbsp, 1);
   }

   sps->num_short_term_ref_pic_sets = vl_rbsp_ue(rbsp);

   for (i = 0; i < sps->num_short_term_ref_pic_sets; ++i) {
      struct ref_pic_set *rps;

      rps = (struct ref_pic_set *)
         priv->codec_data.h265.ref_pic_set_list + i;
      st_ref_pic_set(priv, rbsp, rps, sps, i);
   }

   sps->long_term_ref_pics_present_flag = vl_rbsp_u(rbsp, 1);
   if (sps->long_term_ref_pics_present_flag) {
      sps->num_long_term_ref_pics_sps = vl_rbsp_ue(rbsp);
      for (i = 0; i < sps->num_long_term_ref_pics_sps; ++i) {
         /* lt_ref_pic_poc_lsb_sps */
         vl_rbsp_u(rbsp, sps->log2_max_pic_order_cnt_lsb_minus4 + 4);
         /* used_by_curr_pic_lt_sps_flag */
         vl_rbsp_u(rbsp, 1);
      }
   }

   sps->sps_temporal_mvp_enabled_flag = vl_rbsp_u(rbsp, 1);
   sps->strong_intra_smoothing_enabled_flag = vl_rbsp_u(rbsp, 1);
}

static struct pipe_h265_pps *pic_parameter_set_id(vid_dec_PrivateType *priv,
                                                  struct vl_rbsp *rbsp)
{
   unsigned id = vl_rbsp_ue(rbsp);

   if (id >= ARRAY_SIZE(priv->codec_data.h265.pps))
      return NULL;

   return &priv->codec_data.h265.pps[id];
}

static void picture_parameter_set(vid_dec_PrivateType *priv,
                                  struct vl_rbsp *rbsp)
{
   struct pipe_h265_sps *sps;
   struct pipe_h265_pps *pps;
   int i;

   pps = pic_parameter_set_id(priv, rbsp);
   if (!pps)
      return;

   memset(pps, 0, sizeof(*pps));
   sps = pps->sps = seq_parameter_set_id(priv, rbsp);
   if (!sps)
      return;

   pps->dependent_slice_segments_enabled_flag = vl_rbsp_u(rbsp, 1);
   pps->output_flag_present_flag = vl_rbsp_u(rbsp, 1);
   pps->num_extra_slice_header_bits = vl_rbsp_u(rbsp, 3);
   pps->sign_data_hiding_enabled_flag = vl_rbsp_u(rbsp, 1);
   pps->cabac_init_present_flag = vl_rbsp_u(rbsp, 1);

   pps->num_ref_idx_l0_default_active_minus1 = vl_rbsp_ue(rbsp);
   pps->num_ref_idx_l1_default_active_minus1 = vl_rbsp_ue(rbsp);
   pps->init_qp_minus26 = vl_rbsp_se(rbsp);
   pps->constrained_intra_pred_flag = vl_rbsp_u(rbsp, 1);
   pps->transform_skip_enabled_flag = vl_rbsp_u(rbsp, 1);

   pps->cu_qp_delta_enabled_flag = vl_rbsp_u(rbsp, 1);
   if (pps->cu_qp_delta_enabled_flag)
      pps->diff_cu_qp_delta_depth = vl_rbsp_ue(rbsp);

   pps->pps_cb_qp_offset = vl_rbsp_se(rbsp);
   pps->pps_cr_qp_offset = vl_rbsp_se(rbsp);
   pps->pps_slice_chroma_qp_offsets_present_flag = vl_rbsp_u(rbsp, 1);

   pps->weighted_pred_flag = vl_rbsp_u(rbsp, 1);
   pps->weighted_bipred_flag = vl_rbsp_u(rbsp, 1);

   pps->transquant_bypass_enabled_flag = vl_rbsp_u(rbsp, 1);
   pps->tiles_enabled_flag = vl_rbsp_u(rbsp, 1);
   pps->entropy_coding_sync_enabled_flag = vl_rbsp_u(rbsp, 1);

   if (pps->tiles_enabled_flag) {
      pps->num_tile_columns_minus1 = vl_rbsp_ue(rbsp);
      pps->num_tile_rows_minus1 = vl_rbsp_ue(rbsp);

      pps->uniform_spacing_flag = vl_rbsp_u(rbsp, 1);
      if (!pps->uniform_spacing_flag) {
         for (i = 0; i < pps->num_tile_columns_minus1; ++i)
            pps->column_width_minus1[i] = vl_rbsp_ue(rbsp);

         for (i = 0; i < pps->num_tile_rows_minus1; ++i)
            pps->row_height_minus1[i] = vl_rbsp_ue(rbsp);
      }

      if (!pps->num_tile_columns_minus1 || !pps->num_tile_rows_minus1)
         pps->loop_filter_across_tiles_enabled_flag = vl_rbsp_u(rbsp, 1);
   }

   pps->pps_loop_filter_across_slices_enabled_flag = vl_rbsp_u(rbsp, 1);

   pps->deblocking_filter_control_present_flag = vl_rbsp_u(rbsp, 1);
   if (pps->deblocking_filter_control_present_flag) {
      pps->deblocking_filter_override_enabled_flag = vl_rbsp_u(rbsp, 1);
      pps->pps_deblocking_filter_disabled_flag = vl_rbsp_u(rbsp, 1);
      if (!pps->pps_deblocking_filter_disabled_flag) {
         pps->pps_beta_offset_div2 = vl_rbsp_se(rbsp);
         pps->pps_tc_offset_div2 = vl_rbsp_se(rbsp);
      }
   }

   if (vl_vlc_bits_left(&rbsp->nal) == 0)
      return;

   /* pps_scaling_list_data_present_flag */
   if (vl_rbsp_u(rbsp, 1))
      scaling_list_data(priv, rbsp, sps);

   pps->lists_modification_present_flag = vl_rbsp_u(rbsp, 1);
   pps->log2_parallel_merge_level_minus2 = vl_rbsp_ue(rbsp);
   pps->slice_segment_header_extension_present_flag = vl_rbsp_u(rbsp, 1);
}

static void vid_dec_h265_BeginFrame(vid_dec_PrivateType *priv)
{
   if (priv->frame_started)
      return;

   if (!priv->codec) {
      struct pipe_video_codec templat = {};
      omx_base_video_PortType *port = (omx_base_video_PortType *)
         priv->ports[OMX_BASE_FILTER_INPUTPORT_INDEX];

      templat.profile = priv->profile;
      templat.entrypoint = PIPE_VIDEO_ENTRYPOINT_BITSTREAM;
      templat.chroma_format = PIPE_VIDEO_CHROMA_FORMAT_420;
      templat.expect_chunked_decode = false;
      templat.width = priv->codec_data.h265.pic_width_in_luma_samples;
      templat.height = priv->codec_data.h265.pic_height_in_luma_samples;
      templat.level =  priv->codec_data.h265.level_idc;
      priv->codec = priv->pipe->create_video_codec(priv->pipe, &templat);

      /* disable transcode tunnel if video size is different from coded size */
      if (priv->codec_data.h265.pic_width_in_luma_samples !=
          port->sPortParam.format.video.nFrameWidth ||
          priv->codec_data.h265.pic_height_in_luma_samples !=
          port->sPortParam.format.video.nFrameHeight)
         priv->disable_tunnel = true;
   }

   vid_dec_NeedTarget(priv);

   if (priv->first_buf_in_frame)
      priv->timestamp = priv->timestamps[0];
   priv->first_buf_in_frame = false;

   priv->codec->begin_frame(priv->codec, priv->target, &priv->picture.base);
   priv->frame_started = true;
}

static struct pipe_video_buffer *vid_dec_h265_Flush(vid_dec_PrivateType *priv,
                                                    OMX_TICKS *timestamp)
{
   struct dpb_list *entry, *result = NULL;
   struct pipe_video_buffer *buf;

   /* search for the lowest poc and break on zeros */
   LIST_FOR_EACH_ENTRY(entry, &priv->codec_data.h265.dpb_list, list) {

      if (result && entry->poc == 0)
         break;

      if (!result || entry->poc < result->poc)
         result = entry;
   }

   if (!result)
      return NULL;

   buf = result->buffer;
   if (timestamp)
      *timestamp = result->timestamp;

   --priv->codec_data.h265.dpb_num;
   list_del(&result->list);
   FREE(result);

   return buf;
}

static void vid_dec_h265_EndFrame(vid_dec_PrivateType *priv)
{
   struct dpb_list *entry = NULL;
   struct pipe_video_buffer *tmp;
   struct ref_pic_set *rps;
   int i;
   OMX_TICKS timestamp;

   if (!priv->frame_started)
      return;

   priv->picture.h265.NumPocStCurrBefore = 0;
   priv->picture.h265.NumPocStCurrAfter = 0;
   memset(priv->picture.h265.RefPicSetStCurrBefore, 0, 8);
   memset(priv->picture.h265.RefPicSetStCurrAfter, 0, 8);
   for (i = 0; i < MAX_NUM_REF_PICS; ++i) {
      priv->picture.h265.ref[i] = NULL;
      priv->picture.h265.PicOrderCntVal[i] = 0;
   }

   rps = priv->codec_data.h265.rps;

   if (rps) {
      unsigned bf = 0, af = 0;

      priv->picture.h265.NumDeltaPocsOfRefRpsIdx = rps->num_delta_poc;
      for (i = 0; i < rps->num_pics; ++i) {
         priv->picture.h265.PicOrderCntVal[i] =
            rps->delta_poc[i] + get_poc(priv);

         LIST_FOR_EACH_ENTRY(entry, &priv->codec_data.h265.dpb_list, list) {
            if (entry->poc == priv->picture.h265.PicOrderCntVal[i]) {
               priv->picture.h265.ref[i] = entry->buffer;
            }
         }

         if (rps->used[i]) {
            if (i < rps->num_neg_pics) {
               priv->picture.h265.NumPocStCurrBefore++;
               priv->picture.h265.RefPicSetStCurrBefore[bf++] = i;
            } else {
               priv->picture.h265.NumPocStCurrAfter++;
               priv->picture.h265.RefPicSetStCurrAfter[af++] = i;
            }
         }
      }
   }

   priv->codec->end_frame(priv->codec, priv->target, &priv->picture.base);
   priv->frame_started = false;

   /* add the decoded picture to the dpb list */
   entry = CALLOC_STRUCT(dpb_list);
   if (!entry)
      return;

   priv->first_buf_in_frame = true;
   entry->buffer = priv->target;
   entry->timestamp = priv->timestamp;
   entry->poc = get_poc(priv);

   list_addtail(&entry->list, &priv->codec_data.h265.dpb_list);
   ++priv->codec_data.h265.dpb_num;
   priv->target = NULL;

   if (priv->codec_data.h265.dpb_num <= DPB_MAX_SIZE)
      return;

   tmp = priv->in_buffers[0]->pInputPortPrivate;
   priv->in_buffers[0]->pInputPortPrivate = vid_dec_h265_Flush(priv, &timestamp);
   priv->in_buffers[0]->nTimeStamp = timestamp;
   priv->target = tmp;
   priv->frame_finished = priv->in_buffers[0]->pInputPortPrivate != NULL;
   if (priv->frame_finished &&
       (priv->in_buffers[0]->nFlags & OMX_BUFFERFLAG_EOS))
      FREE(priv->codec_data.h265.ref_pic_set_list);
}

static void slice_header(vid_dec_PrivateType *priv, struct vl_rbsp *rbsp,
                         unsigned nal_unit_type)
{
   struct pipe_h265_pps *pps;
   struct pipe_h265_sps *sps;
   bool first_slice_segment_in_pic_flag;
   bool dependent_slice_segment_flag = false;
   struct ref_pic_set *rps;
   unsigned poc_lsb, poc_msb, slice_prev_poc;
   unsigned max_poc_lsb, prev_poc_lsb, prev_poc_msb;
   unsigned num_st_rps;
   int i;

   if (priv->picture.h265.IDRPicFlag != is_idr_picture(nal_unit_type))
      vid_dec_h265_EndFrame(priv);

   priv->picture.h265.IDRPicFlag = is_idr_picture(nal_unit_type);

   first_slice_segment_in_pic_flag = vl_rbsp_u(rbsp, 1);

   if (is_rap_picture(nal_unit_type))
      /* no_output_of_prior_pics_flag */
      vl_rbsp_u(rbsp, 1);

   pps = pic_parameter_set_id(priv, rbsp);
   if (!pps)
      return;

   sps = pps->sps;
   if (!sps)
      return;

   if (pps != priv->picture.h265.pps)
      vid_dec_h265_EndFrame(priv);

   priv->picture.h265.pps = pps;

   if (priv->picture.h265.RAPPicFlag != is_rap_picture(nal_unit_type))
      vid_dec_h265_EndFrame(priv);
   priv->picture.h265.RAPPicFlag = is_rap_picture(nal_unit_type);
   priv->picture.h265.IntraPicFlag = is_rap_picture(nal_unit_type);
   num_st_rps = sps->num_short_term_ref_pic_sets;

   if (priv->picture.h265.CurrRpsIdx != num_st_rps)
      vid_dec_h265_EndFrame(priv);
   priv->picture.h265.CurrRpsIdx = num_st_rps;

   if (!first_slice_segment_in_pic_flag) {
      int size, num;
      int bits_slice_segment_address = 0;

      if (pps->dependent_slice_segments_enabled_flag)
         dependent_slice_segment_flag = vl_rbsp_u(rbsp, 1);

      size = 1 << (sps->log2_min_luma_coding_block_size_minus3 + 3 +
                   sps->log2_diff_max_min_luma_coding_block_size);

      num = ((sps->pic_width_in_luma_samples + size - 1) / size) *
            ((sps->pic_height_in_luma_samples + size - 1) / size);

      while (num > (1 << bits_slice_segment_address))
         bits_slice_segment_address++;

      /* slice_segment_address */
      vl_rbsp_u(rbsp, bits_slice_segment_address);
   }

   if (dependent_slice_segment_flag)
      return;

   for (i = 0; i < pps->num_extra_slice_header_bits; ++i)
      /* slice_reserved_flag */
      vl_rbsp_u(rbsp, 1);

   /* slice_type */
   vl_rbsp_ue(rbsp);

   if (pps->output_flag_present_flag)
      /* pic output flag */
      vl_rbsp_u(rbsp, 1);

   if (sps->separate_colour_plane_flag)
      /* colour_plane_id */
      vl_rbsp_u(rbsp, 2);

   if (is_idr_picture(nal_unit_type)) {
      set_poc(priv, nal_unit_type, 0);
      return;
   }

   /* slice_pic_order_cnt_lsb */
   poc_lsb =
      vl_rbsp_u(rbsp, sps->log2_max_pic_order_cnt_lsb_minus4 + 4);

   slice_prev_poc = (int)priv->codec_data.h265.slice_prev_poc;
   max_poc_lsb = 1 << (sps->log2_max_pic_order_cnt_lsb_minus4 + 4);

   prev_poc_lsb = slice_prev_poc & (max_poc_lsb - 1);
   prev_poc_msb = slice_prev_poc - prev_poc_lsb;

   if ((poc_lsb < prev_poc_lsb) &&
       ((prev_poc_lsb - poc_lsb ) >= (max_poc_lsb / 2)))
      poc_msb = prev_poc_msb + max_poc_lsb;

   else if ((poc_lsb > prev_poc_lsb ) &&
            ((poc_lsb - prev_poc_lsb) > (max_poc_lsb / 2)))
      poc_msb = prev_poc_msb - max_poc_lsb;

   else
      poc_msb = prev_poc_msb;

   if (is_bla_picture(nal_unit_type))
      poc_msb = 0;

   if (get_poc(priv) != poc_msb + poc_lsb)
      vid_dec_h265_EndFrame(priv);

   set_poc(priv, nal_unit_type, (poc_msb + poc_lsb));

   /* short_term_ref_pic_set_sps_flag */
   if (!vl_rbsp_u(rbsp, 1)) {
      rps = (struct ref_pic_set *)
         priv->codec_data.h265.ref_pic_set_list + num_st_rps;
      st_ref_pic_set(priv, rbsp, rps, sps, num_st_rps);

   } else if (num_st_rps > 1) {
      int num_bits = 0;
      unsigned idx;

      while ((1 << num_bits) < num_st_rps)
         num_bits++;

      if (num_bits > 0)
         /* short_term_ref_pic_set_idx */
         idx = vl_rbsp_u(rbsp, num_bits);
      else
         idx = 0;

      rps = (struct ref_pic_set *)
         priv->codec_data.h265.ref_pic_set_list + idx;
   } else
      rps = (struct ref_pic_set *)
         priv->codec_data.h265.ref_pic_set_list;

   if (is_bla_picture(nal_unit_type)) {
      rps->num_neg_pics = 0;
      rps->num_pos_pics = 0;
      rps->num_pics = 0;
   }

   priv->codec_data.h265.rps = rps;

   return;
}

static void vid_dec_h265_Decode(vid_dec_PrivateType *priv,
                                struct vl_vlc *vlc,
                                unsigned min_bits_left)
{
   unsigned nal_unit_type;
   unsigned nuh_layer_id;
   unsigned nuh_temporal_id_plus1;

   if (!vl_vlc_search_byte(vlc, vl_vlc_bits_left(vlc) - min_bits_left, 0x00))
      return;

   if (vl_vlc_peekbits(vlc, 24) != 0x000001) {
      vl_vlc_eatbits(vlc, 8);
      return;
   }

   if (priv->slice) {
      unsigned bytes = priv->bytes_left - (vl_vlc_bits_left(vlc) / 8);

      priv->codec->decode_bitstream(priv->codec, priv->target,
                                    &priv->picture.base, 1,
                                    &priv->slice, &bytes);
      priv->slice = NULL;
   }

   vl_vlc_eatbits(vlc, 24);

   /* forbidden_zero_bit */
   vl_vlc_eatbits(vlc, 1);

   if (vl_vlc_valid_bits(vlc) < 15)
      vl_vlc_fillbits(vlc);

   nal_unit_type = vl_vlc_get_uimsbf(vlc, 6);

   /* nuh_layer_id */
   nuh_layer_id = vl_vlc_get_uimsbf(vlc, 6);

   /* nuh_temporal_id_plus1 */
   nuh_temporal_id_plus1 = vl_vlc_get_uimsbf(vlc, 3);
   priv->codec_data.h265.temporal_id = nuh_temporal_id_plus1 - 1;

   if (!is_slice_picture(nal_unit_type))
      vid_dec_h265_EndFrame(priv);

   if (nal_unit_type == NAL_UNIT_TYPE_SPS) {
      struct vl_rbsp rbsp;

      vl_rbsp_init(&rbsp, vlc, ~0, /* emulation_bytes */ true);
      seq_parameter_set(priv, &rbsp);

   } else if (nal_unit_type == NAL_UNIT_TYPE_PPS) {
      struct vl_rbsp rbsp;

      vl_rbsp_init(&rbsp, vlc, ~0, /* emulation_bytes */ true);
      picture_parameter_set(priv, &rbsp);

   } else if (is_slice_picture(nal_unit_type)) {
      unsigned bits = vl_vlc_valid_bits(vlc);
      unsigned bytes = bits / 8 + 5;
      struct vl_rbsp rbsp;
      uint8_t buf[9];
      const void *ptr = buf;
      unsigned i;

      buf[0] = 0x0;
      buf[1] = 0x0;
      buf[2] = 0x1;
      buf[3] = nal_unit_type << 1 | nuh_layer_id >> 5;
      buf[4] = nuh_layer_id << 3 | nuh_temporal_id_plus1;
      for (i = 5; i < bytes; ++i)
         buf[i] = vl_vlc_peekbits(vlc, bits) >> ((bytes - i - 1) * 8);

      priv->bytes_left = (vl_vlc_bits_left(vlc) - bits) / 8;
      priv->slice = vlc->data;

      vl_rbsp_init(&rbsp, vlc, 128, /* emulation_bytes */ true);
      slice_header(priv, &rbsp, nal_unit_type);

      vid_dec_h265_BeginFrame(priv);

      priv->codec->decode_bitstream(priv->codec, priv->target,
                                    &priv->picture.base, 1,
                                    &ptr, &bytes);
   }

   /* resync to byte boundary */
   vl_vlc_eatbits(vlc, vl_vlc_valid_bits(vlc) % 8);
}

void vid_dec_h265_Init(vid_dec_PrivateType *priv)
{
   priv->picture.base.profile = PIPE_VIDEO_PROFILE_HEVC_MAIN;

   list_inithead(&priv->codec_data.h265.dpb_list);
   priv->codec_data.h265.ref_pic_set_list = (struct ref_pic_set *)
      CALLOC(MAX_NUM_REF_PICS, sizeof(struct ref_pic_set));

   priv->Decode = vid_dec_h265_Decode;
   priv->EndFrame = vid_dec_h265_EndFrame;
   priv->Flush = vid_dec_h265_Flush;
   priv->first_buf_in_frame = true;
}
