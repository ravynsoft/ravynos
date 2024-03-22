/**************************************************************************
 *
 * Copyright 2020 Advanced Micro Devices, Inc.
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
#include "vl/vl_video_buffer.h"

#include "vl/vl_codec.h"

#include "entrypoint.h"
#include "vid_dec.h"
#include "vid_dec_av1.h"

static unsigned av1_f(struct vl_vlc *vlc, unsigned n)
{
   unsigned valid = vl_vlc_valid_bits(vlc);

   if (n == 0)
      return 0;

   if (valid < 32)
      vl_vlc_fillbits(vlc);

   return vl_vlc_get_uimsbf(vlc, n);
}

static unsigned av1_uvlc(struct vl_vlc *vlc)
{
   unsigned value;
   unsigned leadingZeros = 0;

   while (1) {
      bool done = av1_f(vlc, 1);
      if (done)
         break;
      leadingZeros++;
   }

   if (leadingZeros >= 32)
      return 0xffffffff;

   value = av1_f(vlc, leadingZeros);

   return value + (1 << leadingZeros) - 1;
}

static int av1_le(struct vl_vlc *vlc, const unsigned n)
{
   unsigned byte, t = 0;
   unsigned i;

   for (i = 0; i < n; ++i) {
      byte = av1_f(vlc, 8);
      t += (byte << (i * 8));
   }

   return t;
}

static unsigned av1_uleb128(struct vl_vlc *vlc)
{
   unsigned value = 0;
   unsigned leb128Bytes = 0;
   unsigned i;

   for (i = 0; i < 8; ++i) {
      leb128Bytes = av1_f(vlc, 8);
      value |= ((leb128Bytes & 0x7f) << (i * 7));
      if (!(leb128Bytes & 0x80))
         break;
   }

   return value;
}

static int av1_su(struct vl_vlc *vlc, const unsigned n)
{
   unsigned value = av1_f(vlc, n);
   unsigned signMask = 1 << (n - 1);

   if (value && signMask)
      value = value - 2 * signMask;

   return value;
}

static unsigned FloorLog2(unsigned x)
{
   unsigned s = 0;
   unsigned x1 = x;

   while (x1 != 0) {
      x1 = x1 >> 1;
      s++;
   }

   return s - 1;
}

static unsigned av1_ns(struct vl_vlc *vlc, unsigned n)
{
   unsigned w = FloorLog2(n) + 1;
   unsigned m = (1 << w) - n;
   unsigned v = av1_f(vlc, w - 1);

   if (v < m)
      return v;

   bool extra_bit = av1_f(vlc, 1);

   return (v << 1) - m + extra_bit;
}

static void av1_byte_alignment(struct vl_vlc *vlc)
{
   vl_vlc_eatbits(vlc, vl_vlc_valid_bits(vlc) % 8);
}

static void sequence_header_obu(vid_dec_PrivateType *priv, struct vl_vlc *vlc)
{
   struct av1_sequence_header_obu *seq = &(priv->codec_data.av1.seq);
   bool timing_info_present_flag;
   bool initial_display_delay_present_flag;
   uint8_t seq_level_idx;
   bool initial_display_delay_present_for_this_op;
   bool high_bitdepth;
   bool twelve_bit;
   bool color_description_present_flag;
   uint8_t color_primaries;
   uint8_t transfer_characteristics;
   uint8_t matrix_coefficients;
   int i;

   seq->seq_profile = av1_f(vlc, 3);
   assert(seq->seq_profile < 3);

   av1_f(vlc, 1); /* still_picture */
   seq->reduced_still_picture_header = av1_f(vlc, 1);
   if (seq->reduced_still_picture_header) {
      timing_info_present_flag = 0;
      seq->decoder_model_info_present_flag = 0;
      initial_display_delay_present_flag = 0;
      seq->operating_points_cnt_minus_1 = 0;
      seq->operating_point_idc[0] = 0;
      seq_level_idx = av1_f(vlc, 5);
      seq->decoder_model_present_for_this_op[0] = 0;
      initial_display_delay_present_for_this_op = 0;
   } else {
      uint8_t buffer_delay_length_minus_1 = 0;

      timing_info_present_flag = av1_f(vlc, 1);
      if (timing_info_present_flag) {
         av1_f(vlc, 32); /* num_units_in_display_tick */
         av1_f(vlc, 32); /* time_scale */
         seq->timing_info.equal_picture_interval = av1_f(vlc, 1);
         if (seq->timing_info.equal_picture_interval)
            av1_uvlc(vlc); /* num_ticks_per_picture_minus_1 */

         seq->decoder_model_info_present_flag = av1_f(vlc, 1);
         if (seq->decoder_model_info_present_flag) {
             /* decoder_model_info */
            buffer_delay_length_minus_1 = av1_f(vlc, 5);
            seq->decoder_model_info.num_units_in_decoding_tick = av1_f(vlc, 32);
            seq->decoder_model_info.buffer_removal_time_length_minus_1 = av1_f(vlc, 5);
            seq->decoder_model_info.frame_presentation_time_length_minus_1 = av1_f(vlc, 5);
         }
      } else {
         seq->decoder_model_info_present_flag = 0;
      }

      initial_display_delay_present_flag = av1_f(vlc, 1);
      seq->operating_points_cnt_minus_1 = av1_f(vlc, 5);
      for (i = 0; i < seq->operating_points_cnt_minus_1 + 1; ++i) {
         seq->operating_point_idc[i] = av1_f(vlc, 12);
         seq_level_idx = av1_f(vlc, 5);
         if (seq_level_idx > 7)
            av1_f(vlc, 1); /* seq_tier */

         if (seq->decoder_model_info_present_flag) {
            seq->decoder_model_present_for_this_op[i] = av1_f(vlc, 1);
            if (seq->decoder_model_present_for_this_op[i]) {
               uint8_t n = buffer_delay_length_minus_1 + 1;
               av1_f(vlc, n); /* decoder_buffer_delay */
               av1_f(vlc, n); /* encoder_buffer_delay */
               av1_f(vlc, 1); /* low_delay_mode_flag */
            }
         } else {
            seq->decoder_model_present_for_this_op[i] = 0;
         }

         if (initial_display_delay_present_flag) {
            initial_display_delay_present_for_this_op = av1_f(vlc, 1);
            if (initial_display_delay_present_for_this_op)
               av1_f(vlc, 4); /* initial_display_delay_minus_1 */
         }
      }
   }

   seq->frame_width_bits_minus_1 = av1_f(vlc, 4);
   seq->frame_height_bits_minus_1 = av1_f(vlc, 4);
   seq->max_frame_width_minus_1 = av1_f(vlc, seq->frame_width_bits_minus_1 + 1);
   seq->max_frame_height_minus_1 = av1_f(vlc, seq->frame_height_bits_minus_1 + 1);

   if (seq->reduced_still_picture_header)
      seq->frame_id_numbers_present_flag = 0;
   else
      seq->frame_id_numbers_present_flag = av1_f(vlc, 1);
   if (seq->frame_id_numbers_present_flag) {
      seq->delta_frame_id_length_minus_2 = av1_f(vlc, 4);
      seq->additional_frame_id_length_minus_1 = av1_f(vlc, 3);
   }

   seq->use_128x128_superblock = av1_f(vlc, 1);
   seq->enable_filter_intra = av1_f(vlc, 1);
   seq->enable_intra_edge_filter = av1_f(vlc, 1);
   if (seq->reduced_still_picture_header) {
      seq->enable_interintra_compound = 0;
      seq->enable_masked_compound = 0;
      seq->enable_warped_motion = 0;
      seq->enable_dual_filter = 0;
      seq->enable_order_hint = 0;
      seq->enable_jnt_comp = 0;
      seq->enable_ref_frame_mvs = 0;
      seq->seq_force_screen_content_tools = AV1_SELECT_SCREEN_CONTENT_TOOLS;
      seq->seq_force_integer_mv = AV1_SELECT_INTEGER_MV;
      seq->OrderHintBits = 0;
  } else {
      bool seq_choose_screen_content_tools;
      seq->enable_interintra_compound = av1_f(vlc, 1);
      seq->enable_masked_compound = av1_f(vlc, 1);
      seq->enable_warped_motion = av1_f(vlc, 1);
      seq->enable_dual_filter = av1_f(vlc, 1);
      seq->enable_order_hint = av1_f(vlc, 1);
      if (seq->enable_order_hint) {
         seq->enable_jnt_comp = av1_f(vlc, 1);
         seq->enable_ref_frame_mvs = av1_f(vlc, 1);
      } else {
         seq->enable_jnt_comp = 0;
         seq->enable_ref_frame_mvs = 0;
      }

      seq_choose_screen_content_tools = av1_f(vlc, 1);
      seq->seq_force_screen_content_tools =
         seq_choose_screen_content_tools ? AV1_SELECT_SCREEN_CONTENT_TOOLS : av1_f(vlc, 1);

      if (seq->seq_force_screen_content_tools > 0) {
         bool seq_choose_integer_mv = av1_f(vlc, 1);
         seq->seq_force_integer_mv =
            seq_choose_integer_mv ? AV1_SELECT_INTEGER_MV : av1_f(vlc, 1);
      } else {
         seq->seq_force_integer_mv = AV1_SELECT_INTEGER_MV;
      }

      if (seq->enable_order_hint) {
         seq->order_hint_bits_minus_1 = av1_f(vlc, 3);
         seq->OrderHintBits = seq->order_hint_bits_minus_1 + 1;
      } else {
         seq->OrderHintBits = 0;
      }
   }

   seq->enable_superres = av1_f(vlc, 1);
   seq->enable_cdef = av1_f(vlc, 1);
   seq->enable_restoration = av1_f(vlc, 1);

   high_bitdepth = av1_f(vlc, 1);
   if (seq->seq_profile == 2 && high_bitdepth) {
      twelve_bit = av1_f(vlc, 1);
      seq->color_config.BitDepth = twelve_bit ? 12 : 10;
   } else if (seq->seq_profile <= 2) {
      seq->color_config.BitDepth = high_bitdepth ? 10 : 8;
   }

   seq->color_config.mono_chrome = (seq->seq_profile == 1) ? 0 : av1_f(vlc, 1);
   seq->color_config.NumPlanes = seq->color_config.mono_chrome ? 1 : 3;

   color_description_present_flag = av1_f(vlc, 1);
   if (color_description_present_flag) {
      color_primaries = av1_f(vlc, 8);
      transfer_characteristics = av1_f(vlc, 8);
      matrix_coefficients = av1_f(vlc, 8);
   } else {
      color_primaries = AV1_CP_UNSPECIFIED;
      transfer_characteristics = AV1_TC_UNSPECIFIED;
      matrix_coefficients = AV1_MC_UNSPECIFIED;
   }

   if (seq->color_config.mono_chrome) {
      av1_f(vlc, 1); /* color_range */
      seq->color_config.subsampling_x = 1;
      seq->color_config.subsampling_y = 1;
      seq->color_config.separate_uv_delta_q = 0;
   } else if (color_primaries == AV1_CP_BT_709 &&
              transfer_characteristics == AV1_TC_SRGB &&
              matrix_coefficients == AV1_MC_IDENTITY) {
      seq->color_config.subsampling_x = 0;
      seq->color_config.subsampling_y = 0;
   } else {
      av1_f(vlc, 1); /* color_range */
      if (seq->seq_profile == 0) {
         seq->color_config.subsampling_x = 1;
         seq->color_config.subsampling_y = 1;
      } else if (seq->seq_profile == 1 ) {
         seq->color_config.subsampling_x = 0;
         seq->color_config.subsampling_y = 0;
      } else {
         if (seq->color_config.BitDepth == 12) {
            seq->color_config.subsampling_x = av1_f(vlc, 1);
            if (seq->color_config.subsampling_x)
               seq->color_config.subsampling_y = av1_f(vlc, 1);
            else
               seq->color_config.subsampling_y = 0;
         } else {
            seq->color_config.subsampling_x = 1;
            seq->color_config.subsampling_y = 0;
         }
      }
      if (seq->color_config.subsampling_x && seq->color_config.subsampling_y)
         av1_f(vlc, 2); /* chroma_sample_position */
   }
   if (!seq->color_config.mono_chrome)
      seq->color_config.separate_uv_delta_q = av1_f(vlc, 1);

   seq->film_grain_params_present = av1_f(vlc, 1);

   priv->picture.av1.picture_parameter.profile = seq->seq_profile;
   priv->picture.av1.picture_parameter.seq_info_fields.use_128x128_superblock =
      seq->use_128x128_superblock;
   priv->picture.av1.picture_parameter.seq_info_fields.enable_filter_intra =
      seq->enable_filter_intra;
   priv->picture.av1.picture_parameter.seq_info_fields.enable_intra_edge_filter =
      seq->enable_intra_edge_filter;
   priv->picture.av1.picture_parameter.order_hint_bits_minus_1 =
      seq->order_hint_bits_minus_1;
   priv->picture.av1.picture_parameter.max_width = seq->max_frame_width_minus_1 + 1;
   priv->picture.av1.picture_parameter.max_height = seq->max_frame_height_minus_1 + 1;
   priv->picture.av1.picture_parameter.seq_info_fields.enable_interintra_compound =
      seq->enable_interintra_compound;
   priv->picture.av1.picture_parameter.seq_info_fields.enable_masked_compound =
      seq->enable_masked_compound;
   priv->picture.av1.picture_parameter.seq_info_fields.enable_dual_filter =
      seq->enable_dual_filter;
   priv->picture.av1.picture_parameter.seq_info_fields.enable_order_hint =
      seq->enable_order_hint;
   priv->picture.av1.picture_parameter.seq_info_fields.enable_jnt_comp =
      seq->enable_jnt_comp;
   priv->picture.av1.picture_parameter.seq_info_fields.ref_frame_mvs =
      seq->enable_ref_frame_mvs;
   priv->picture.av1.picture_parameter.bit_depth_idx =
      (seq->color_config.BitDepth - 8) >> 1;
   priv->picture.av1.picture_parameter.seq_info_fields.mono_chrome =
      seq->color_config.mono_chrome;
}

static void superres_params(vid_dec_PrivateType *priv, struct vl_vlc *vlc)
{
   struct av1_sequence_header_obu *seq = &(priv->codec_data.av1.seq);
   struct av1_uncompressed_header_obu *hdr = &(priv->codec_data.av1.uncompressed_header);
   unsigned coded_denom;

   if (seq->enable_superres)
      hdr->use_superres = av1_f(vlc, 1);
   else
      hdr->use_superres = 0;

   if (hdr->use_superres) {
      coded_denom = av1_f(vlc, 3 /* SUPERRES_DENOM_BITS */);
      hdr->SuperresDenom = coded_denom + 9 /* SUPERRES_DENOM_MIN */;
   } else {
      hdr->SuperresDenom = 8 /* SUPERRES_NUM */;
   }

   hdr->UpscaledWidth = hdr->FrameWidth;
   hdr->FrameWidth = (hdr->UpscaledWidth * 8 + (hdr->SuperresDenom / 2)) /
      hdr->SuperresDenom;
}

static void compute_image_size(vid_dec_PrivateType *priv)
{
   struct av1_uncompressed_header_obu *hdr = &(priv->codec_data.av1.uncompressed_header);

   hdr->MiCols = 2 * ((hdr->FrameWidth + 7) >> 3);
   hdr->MiRows = 2 * ((hdr->FrameHeight + 7) >> 3);
}

static void frame_size(vid_dec_PrivateType *priv, struct vl_vlc *vlc)
{
   struct av1_sequence_header_obu *seq = &(priv->codec_data.av1.seq);
   struct av1_uncompressed_header_obu *hdr = &(priv->codec_data.av1.uncompressed_header);
   unsigned frame_width_minus_1;
   unsigned frame_height_minus_1;

   if (hdr->frame_size_override_flag) {
      frame_width_minus_1 = av1_f(vlc, seq->frame_width_bits_minus_1 + 1);
      frame_height_minus_1 = av1_f(vlc, seq->frame_height_bits_minus_1 + 1);
      hdr->FrameWidth = frame_width_minus_1 + 1;
      hdr->FrameHeight = frame_height_minus_1 + 1;
   } else {
      hdr->FrameWidth = seq->max_frame_width_minus_1 + 1;
      hdr->FrameHeight = seq->max_frame_height_minus_1 + 1;
   }

   superres_params(priv, vlc);
   compute_image_size(priv);
}

static void render_size(vid_dec_PrivateType *priv, struct vl_vlc *vlc)
{
   struct av1_uncompressed_header_obu *hdr = &(priv->codec_data.av1.uncompressed_header);
   bool render_and_frame_size_different;
   unsigned render_width_minus_1;
   unsigned render_height_minus_1;

   render_and_frame_size_different = av1_f(vlc, 1);
   if (render_and_frame_size_different) {
      render_width_minus_1 = av1_f(vlc, 16);
      render_height_minus_1 = av1_f(vlc, 16);
      hdr->RenderWidth = render_width_minus_1 + 1;
      hdr->RenderHeight = render_height_minus_1 + 1;
   } else {
      hdr->RenderWidth = hdr->UpscaledWidth;
      hdr->RenderHeight = hdr->FrameHeight;
   }
}

static int get_relative_dist(vid_dec_PrivateType *priv, int a, int b)
{
   struct av1_sequence_header_obu *seq = &(priv->codec_data.av1.seq);
   int diff;
   unsigned m;

   if (!seq->enable_order_hint)
      return 0;

   diff = a - b;
   m = 1 << (seq->OrderHintBits - 1);
   diff = (diff & (m - 1)) - (diff & m);

   return diff;
}

static uint8_t find_latest_backward(vid_dec_PrivateType *priv)
{
   struct av1_uncompressed_header_obu *hdr = &(priv->codec_data.av1.uncompressed_header);
   uint8_t ref = 0xff;
   unsigned latestOrderHint = 0;
   int i;

   for (i = 0; i < AV1_NUM_REF_FRAMES; ++i) {
      unsigned hint = hdr->shiftedOrderHints[i];
      if (!hdr->usedFrame[i] &&
          hint >= hdr->curFrameHint &&
          (ref == 0xff || hint >= latestOrderHint)) {
         ref = i;
         latestOrderHint = hint;
      }
   }

   return ref;
}

static uint8_t find_earliest_backward(vid_dec_PrivateType *priv)
{
   struct av1_uncompressed_header_obu *hdr = &(priv->codec_data.av1.uncompressed_header);
   uint8_t ref = 0xff;
   unsigned earliestOrderHint = 0;
   int i;

   for (i = 0; i < AV1_NUM_REF_FRAMES; ++i) {
      unsigned hint = hdr->shiftedOrderHints[i];
      if (!hdr->usedFrame[i] &&
          hint >= hdr->curFrameHint &&
          (ref == 0xff || hint < earliestOrderHint)) {
         ref = i;
         earliestOrderHint = hint;
      }
   }

   return ref;
}

static uint8_t find_latest_forward(vid_dec_PrivateType *priv)
{
   struct av1_uncompressed_header_obu *hdr = &(priv->codec_data.av1.uncompressed_header);
   uint8_t ref = 0xff;
   unsigned latestOrderHint = 0;
   int i;

   for (i = 0; i < AV1_NUM_REF_FRAMES; ++i) {
      unsigned hint = hdr->shiftedOrderHints[i];
      if (!hdr->usedFrame[i] &&
          hint < hdr->curFrameHint &&
          (ref == 0xff || hint >= latestOrderHint)) {
         ref = i;
         latestOrderHint = hint;
      }
   }

   return ref;
}

static void set_frame_refs(vid_dec_PrivateType *priv, struct vl_vlc *vlc)
{
   struct av1_sequence_header_obu *seq = &(priv->codec_data.av1.seq);
   struct av1_uncompressed_header_obu *hdr = &(priv->codec_data.av1.uncompressed_header);
   uint8_t Ref_Frame_List[5] = { AV1_LAST2_FRAME , AV1_LAST3_FRAME, AV1_BWDREF_FRAME,
      AV1_ALTREF2_FRAME, AV1_ALTREF_FRAME };
   unsigned earliestOrderHint = 0;
   uint8_t ref;
   int i;

   for (i = 0; i < AV1_REFS_PER_FRAME; ++i)
      hdr->ref_frame_idx[i] = 0xff;

   hdr->ref_frame_idx[0] = hdr->last_frame_idx;
   hdr->ref_frame_idx[AV1_GOLDEN_FRAME - AV1_LAST_FRAME] = hdr->gold_frame_idx;

   for (i = 0; i < AV1_NUM_REF_FRAMES; ++i)
      hdr->usedFrame[i] = 0;

   hdr->usedFrame[hdr->last_frame_idx] = 1;
   hdr->usedFrame[hdr->gold_frame_idx] = 1;

   hdr->curFrameHint = 1 << (seq->OrderHintBits - 1);

   for (i = 0; i < AV1_NUM_REF_FRAMES; ++i)
      hdr->shiftedOrderHints[i] =
         hdr->curFrameHint +
         get_relative_dist(priv, hdr->RefOrderHint[i], hdr->OrderHint);

   ref = find_latest_backward(priv);
   if (ref != 0xff) {
      hdr->ref_frame_idx[AV1_ALTREF_FRAME - AV1_LAST_FRAME] = ref;
      hdr->usedFrame[ref] = 1;
   }

   ref = find_earliest_backward(priv);
   if (ref != 0xff) {
      hdr->ref_frame_idx[AV1_BWDREF_FRAME - AV1_LAST_FRAME] = ref;
      hdr->usedFrame[ref] = 1;
   }

   ref = find_earliest_backward(priv);
   if (ref != 0xff) {
      hdr->ref_frame_idx[AV1_ALTREF2_FRAME - AV1_LAST_FRAME] = ref;
      hdr->usedFrame[ref] = 1;
   }

   for (i = 0; i < AV1_REFS_PER_FRAME - 2; ++i) {
      uint8_t refFrame = Ref_Frame_List[i];
      if (hdr->ref_frame_idx[refFrame - AV1_LAST_FRAME] == 0xff) {
         ref = find_latest_forward(priv);
         if (ref != 0xff) {
            hdr->ref_frame_idx[refFrame - AV1_LAST_FRAME] = ref;
            hdr->usedFrame[ref] = 1;
         }
      }
   }

   ref = 0xff;
   for (i = 0; i < AV1_NUM_REF_FRAMES; ++i) {
      unsigned hint = hdr->shiftedOrderHints[i];
      if (ref == 0xff || hint < earliestOrderHint) {
         ref = i;
         earliestOrderHint = hint;
      }
   }

   for (i = 0; i < AV1_REFS_PER_FRAME; ++i) {
     if (hdr->ref_frame_idx[i] == 0xff)
        hdr->ref_frame_idx[i] = ref;
   }
}

static void frame_size_with_refs(vid_dec_PrivateType *priv, struct vl_vlc *vlc)
{
   struct av1_uncompressed_header_obu *hdr = &(priv->codec_data.av1.uncompressed_header);
   bool found_ref;
   int i;

   for (i = 0; i < AV1_REFS_PER_FRAME; ++i) {
      found_ref = av1_f(vlc, 1);
      if (found_ref) {
         hdr->UpscaledWidth =
            priv->codec_data.av1.RefFrames[hdr->ref_frame_idx[i]].RefUpscaledWidth;
         hdr->FrameWidth = hdr->UpscaledWidth;
         hdr->FrameHeight =
            priv->codec_data.av1.RefFrames[hdr->ref_frame_idx[i]].RefFrameHeight;
         hdr->RenderWidth =
            priv->codec_data.av1.RefFrames[hdr->ref_frame_idx[i]].RefRenderWidth;
         hdr->RenderHeight =
            priv->codec_data.av1.RefFrames[hdr->ref_frame_idx[i]].RefRenderHeight;
         break;
      }
   }

   if (!found_ref) {
      frame_size(priv, vlc);
      render_size(priv, vlc);
   } else {
      superres_params(priv, vlc);
      compute_image_size(priv);
   }
}

static unsigned tile_log2(unsigned blkSize, unsigned target)
{
   unsigned k = 0;

   for (k = 0; (blkSize << k) < target; k++);

   return k;
}

static void tile_info(vid_dec_PrivateType *priv, struct vl_vlc *vlc)
{
   struct av1_sequence_header_obu *seq = &(priv->codec_data.av1.seq);
   struct av1_uncompressed_header_obu *hdr = &(priv->codec_data.av1.uncompressed_header);
   struct tile_info *ti = &(priv->codec_data.av1.uncompressed_header.ti);
   unsigned sbCols;
   unsigned sbRows;
   int width_sb;
   int height_sb;
   unsigned sbSize;
   unsigned maxTileWidthSb;
   unsigned minLog2TileCols;
   unsigned maxLog2TileCols;
   unsigned maxLog2TileRows;
   unsigned minLog2Tiles;
   bool uniform_tile_spacing_flag;
   unsigned maxTileAreaSb;
   unsigned startSb, i;

   sbCols = (seq->use_128x128_superblock) ?
      ((hdr->MiCols + 31) >> 5) : ((hdr->MiCols + 15) >> 4);
   sbRows = (seq->use_128x128_superblock) ?
      ((hdr->MiRows + 31) >> 5) : ((hdr->MiRows + 15) >> 4);
   width_sb = sbCols;
   height_sb = sbRows;
   sbSize = (seq->use_128x128_superblock ? 5 : 4) + 2;
   maxTileWidthSb = AV1_MAX_TILE_WIDTH >> sbSize;
   maxTileAreaSb = AV1_MAX_TILE_AREA >> (2 * sbSize);
   minLog2TileCols = tile_log2(maxTileWidthSb, sbCols);
   maxLog2TileCols = tile_log2(1, MIN2(sbCols, AV1_MAX_TILE_COLS));
   maxLog2TileRows = tile_log2(1, MIN2(sbRows, AV1_MAX_TILE_ROWS));
   minLog2Tiles = MAX2(minLog2TileCols, tile_log2(maxTileAreaSb, sbRows * sbCols));

   uniform_tile_spacing_flag = av1_f(vlc, 1);
   if (uniform_tile_spacing_flag) {
      unsigned tileWidthSb, tileHeightSb;
      unsigned minLog2TileRows;

      ti->TileColsLog2 = minLog2TileCols;
      while (ti->TileColsLog2 < maxLog2TileCols) {
         bool increment_tile_cols_log2 = av1_f(vlc, 1);
         if (increment_tile_cols_log2)
            ti->TileColsLog2++;
         else
            break;
      }
      tileWidthSb = (sbCols + (1 << ti->TileColsLog2) - 1) >> ti->TileColsLog2;
      i = 0;
      for (startSb = 0; startSb < sbCols; startSb += tileWidthSb) {
         ti->tile_col_start_sb[i] = startSb;
         i++;
      }
      ti->tile_col_start_sb[i] = sbCols;
      ti->TileCols = i;

      minLog2TileRows = (minLog2Tiles > ti->TileColsLog2)?
         (minLog2Tiles - ti->TileColsLog2) : 0;
      ti->TileRowsLog2 = minLog2TileRows;
      while (ti->TileRowsLog2 < maxLog2TileRows) {
         bool increment_tile_rows_log2 = av1_f(vlc, 1);
         if (increment_tile_rows_log2)
            ti->TileRowsLog2++;
         else
            break;
      }
      tileHeightSb = (sbRows + (1 << ti->TileRowsLog2) - 1) >> ti->TileRowsLog2;
      i = 0;
      for (startSb = 0; startSb < sbRows; startSb += tileHeightSb) {
         ti->tile_row_start_sb[i] = startSb;
         i++;
      }
      ti->tile_row_start_sb[i] = sbRows;
      ti->TileRows = i;
   } else {
      unsigned widestTileSb = 0;
      unsigned maxTileHeightSb;

      startSb = 0;
      for (i = 0; startSb < sbCols; ++i) {
         uint8_t maxWidth;
         unsigned sizeSb;
         unsigned width_in_sbs_minus_1;

         ti->tile_col_start_sb[i] = startSb;
         maxWidth = MIN2(sbCols - startSb, maxTileWidthSb);
         width_in_sbs_minus_1 = av1_ns(vlc, maxWidth);
         sizeSb = width_in_sbs_minus_1 + 1;
         widestTileSb = MAX2(sizeSb, widestTileSb);
         startSb += sizeSb;
         width_sb -= sizeSb;
      }
      ti->TileCols = i;

      ti->tile_col_start_sb[i] = startSb + width_sb;
      ti->TileColsLog2 = tile_log2(1, ti->TileCols);

      if (minLog2Tiles > 0)
         maxTileAreaSb = (sbRows * sbCols) >> (minLog2Tiles + 1);
      else
         maxTileAreaSb = (sbRows * sbCols);
      maxTileHeightSb = MAX2(maxTileAreaSb / widestTileSb, 1);

      startSb = 0;
      for (i = 0; startSb < sbRows; ++i) {
         uint8_t maxHeight;
         unsigned height_in_sbs_minus_1;

         maxHeight = MIN2(sbRows - startSb, maxTileHeightSb);
         height_in_sbs_minus_1 = av1_ns(vlc, maxHeight);
         ti->tile_row_start_sb[i] = startSb;
         startSb += height_in_sbs_minus_1 + 1;
         height_sb -= height_in_sbs_minus_1 + 1;
      }
      ti->TileRows = i;
      ti->tile_row_start_sb[i] = startSb + height_sb;
      ti->TileRowsLog2 = tile_log2(1, ti->TileRows);
   }

   if (ti->TileColsLog2 > 0 || ti->TileRowsLog2 > 0) {
      ti->context_update_tile_id =
         av1_f(vlc, ti->TileRowsLog2 + ti->TileColsLog2);
      uint8_t tile_size_bytes_minus_1 = av1_f(vlc, 2);
      ti->TileSizeBytes = tile_size_bytes_minus_1 + 1;
   } else {
      ti->context_update_tile_id = 0;
   }
}

static int read_delta_q(struct vl_vlc *vlc)
{
   bool delta_coded = av1_f(vlc, 1);
   int delta_q = 0;

   if (delta_coded)
      delta_q = av1_su(vlc, 7);

   return delta_q;
}

static void quantization_params(vid_dec_PrivateType *priv, struct vl_vlc *vlc)
{
   struct av1_sequence_header_obu *seq = &(priv->codec_data.av1.seq);
   struct quantization_params* qp = &(priv->codec_data.av1.uncompressed_header.qp);
   bool using_qmatrix;

   qp->base_q_idx = av1_f(vlc, 8);
   qp->DeltaQYDc = read_delta_q(vlc);
   if (seq->color_config.NumPlanes > 1) {
      bool diff_uv_delta =
         (seq->color_config.separate_uv_delta_q) ? av1_f(vlc, 1) : 0;

      qp->DeltaQUDc = read_delta_q(vlc);
      qp->DeltaQUAc = read_delta_q(vlc);
      if (diff_uv_delta) {
         qp->DeltaQVDc = read_delta_q(vlc);
         qp->DeltaQVAc = read_delta_q(vlc);
      } else {
         qp->DeltaQVDc = qp->DeltaQUDc;
         qp->DeltaQVAc = qp->DeltaQUAc;
      }
   } else {
      qp->DeltaQVDc = 0;
      qp->DeltaQVAc = 0;
      qp->DeltaQUDc = 0;
      qp->DeltaQUAc = 0;
   }

   using_qmatrix = av1_f(vlc, 1);
   if (using_qmatrix) {
      qp->qm_y = av1_f(vlc, 4);
      qp->qm_u = av1_f(vlc, 4);
      if (!seq->color_config.separate_uv_delta_q)
         qp->qm_v = qp->qm_u;
      else
         qp->qm_v = av1_f(vlc, 4);
   } else {
      qp->qm_y = 0xf;
      qp->qm_u = 0xf;
      qp->qm_v = 0xf;
   }
}

static void segmentation_params(vid_dec_PrivateType *priv, struct vl_vlc *vlc)
{
   struct av1_uncompressed_header_obu *hdr = &(priv->codec_data.av1.uncompressed_header);
   struct segmentation_params* sp = &(priv->codec_data.av1.uncompressed_header.sp);
   int i, j;

   sp->segmentation_enabled = av1_f(vlc, 1);
   if (sp->segmentation_enabled) {
      bool segmentation_update_data;

      if (hdr->primary_ref_frame == AV1_PRIMARY_REF_NONE) {
         sp->segmentation_update_map = 1;
         sp->segmentation_temporal_update = 0;
         segmentation_update_data = 1;
      } else {
         sp->segmentation_update_map = av1_f(vlc, 1);
         if (sp->segmentation_update_map)
            sp->segmentation_temporal_update = av1_f(vlc, 1);
         else
            sp->segmentation_temporal_update = 0;
         segmentation_update_data = av1_f(vlc, 1);
      }

      if (segmentation_update_data) {
         uint8_t Segmentation_Feature_Bits[AV1_SEG_LVL_MAX] = { 8, 6, 6, 6, 6, 3, 0, 0 };
         bool Segmentation_Feature_Signed[AV1_SEG_LVL_MAX] = { 1, 1, 1, 1, 1, 0, 0, 0 };
         unsigned Segmentation_Feature_Max[AV1_SEG_LVL_MAX] = { 255, 63, 63, 63, 63, 7, 0, 0 };

         memset(sp->FeatureData, 0, sizeof(sp->FeatureData));
         memset(sp->FeatureMask, 0, sizeof(sp->FeatureMask));
         for (i = 0; i < AV1_MAX_SEGMENTS; ++i) {
            for (j = 0; j < AV1_SEG_LVL_MAX; ++j) {
               int feature_value = 0;
               bool feature_enabled = av1_f(vlc, 1);

               sp->FeatureEnabled[i][j] = feature_enabled;
               int clippedValue = 0;
               if (feature_enabled) {
                  uint8_t bitsToRead = Segmentation_Feature_Bits[j];
                  int limit = Segmentation_Feature_Max[j];
                  if (Segmentation_Feature_Signed[j]) {
                     feature_value = av1_su(vlc, 1 + bitsToRead);
                     clippedValue = CLAMP(feature_value, -limit, limit);
                     sp->FeatureMask[i] |= 1 << j;
                  } else {
                     feature_value = av1_f(vlc, bitsToRead);
                     clippedValue = CLAMP(feature_value, 0, limit);
                     sp->FeatureMask[i] |= 1 << j;
                  }
               }
               sp->FeatureData[i][j] = clippedValue;
            }
         }
      } else {
         int r = hdr->ref_frame_idx[hdr->primary_ref_frame];
         memcpy(sp, &(priv->codec_data.av1.refs[r].sp), sizeof(*sp));
      }
   } else {
      memset(sp, 0, sizeof(*sp));
   }
}

static void delta_q_params(vid_dec_PrivateType *priv, struct vl_vlc *vlc)
{
   struct quantization_params* qp = &(priv->codec_data.av1.uncompressed_header.qp);
   struct delta_q_params * dqp = &(priv->codec_data.av1.uncompressed_header.dqp);

   dqp->delta_q_present = 0;
   dqp->delta_q_res = 0;
   if (qp->base_q_idx > 0)
      dqp->delta_q_present = av1_f(vlc, 1);
   if (dqp->delta_q_present)
      dqp->delta_q_res = av1_f(vlc, 2);
}

static void delta_lf_params(vid_dec_PrivateType *priv, struct vl_vlc *vlc)
{
   struct av1_uncompressed_header_obu *hdr = &(priv->codec_data.av1.uncompressed_header);
   struct delta_q_params * dqp = &(priv->codec_data.av1.uncompressed_header.dqp);
   struct delta_lf_params* dlfp = &(priv->codec_data.av1.uncompressed_header.dlfp);

   dlfp->delta_lf_present = 0;
   dlfp->delta_lf_res = 0;
   dlfp->delta_lf_multi = 0;
   if (dqp->delta_q_present) {
      if (!hdr->allow_intrabc)
         dlfp->delta_lf_present = av1_f(vlc, 1);

      if (dlfp->delta_lf_present) {
         dlfp->delta_lf_res = av1_f(vlc, 2);
         dlfp->delta_lf_multi = av1_f(vlc, 1);
      }
   }
}

static unsigned get_qindex(vid_dec_PrivateType * priv, bool ignoreDeltaQ, unsigned segmentId)
{
   struct av1_uncompressed_header_obu *hdr = &(priv->codec_data.av1.uncompressed_header);
   struct segmentation_params* sp = &(priv->codec_data.av1.uncompressed_header.sp);
   struct quantization_params* qp = &(priv->codec_data.av1.uncompressed_header.qp);
   unsigned qindex = 0;

   if (sp->segmentation_enabled && sp->FeatureEnabled[segmentId][AV1_SEG_LVL_ALT_Q]) {
      unsigned data = sp->FeatureData[segmentId][AV1_SEG_LVL_ALT_Q];
      qindex = qp->base_q_idx + data;
      if (!ignoreDeltaQ && hdr->dqp.delta_q_present)
         qindex = data;

      return CLAMP(qindex, 0, 255);
   }

   if (!ignoreDeltaQ && hdr->dqp.delta_q_present)
      return 0;

   return qp->base_q_idx;
}

static void loop_filter_params(vid_dec_PrivateType *priv, struct vl_vlc *vlc)
{
   struct av1_sequence_header_obu *seq = &(priv->codec_data.av1.seq);
   struct av1_uncompressed_header_obu *hdr = &(priv->codec_data.av1.uncompressed_header);
   struct loop_filter_params *lfp = &(priv->codec_data.av1.uncompressed_header.lfp);
   int i;

   if (hdr->CodedLossless || hdr->allow_intrabc) {
      lfp->loop_filter_level[0] = 0;
      lfp->loop_filter_level[1] = 0;
      lfp->loop_filter_ref_deltas[AV1_INTRA_FRAME] = 1;
      lfp->loop_filter_ref_deltas[AV1_LAST_FRAME] = 0;
      lfp->loop_filter_ref_deltas[AV1_LAST2_FRAME] = 0;
      lfp->loop_filter_ref_deltas[AV1_LAST3_FRAME] = 0;
      lfp->loop_filter_ref_deltas[AV1_BWDREF_FRAME] = 0;
      lfp->loop_filter_ref_deltas[AV1_GOLDEN_FRAME] = -1;
      lfp->loop_filter_ref_deltas[AV1_ALTREF2_FRAME] = -1;
      lfp->loop_filter_ref_deltas[AV1_ALTREF_FRAME] = -1;
      lfp->loop_filter_mode_deltas[0] = 0;
      lfp->loop_filter_mode_deltas[1] = 0;
      return;
   }

   if (hdr->primary_ref_frame == AV1_PRIMARY_REF_NONE) {
      lfp->loop_filter_ref_deltas[AV1_INTRA_FRAME] = 1;
      lfp->loop_filter_ref_deltas[AV1_LAST_FRAME] = 0;
      lfp->loop_filter_ref_deltas[AV1_LAST2_FRAME] = 0;
      lfp->loop_filter_ref_deltas[AV1_LAST3_FRAME] = 0;
      lfp->loop_filter_ref_deltas[AV1_BWDREF_FRAME] = 0;
      lfp->loop_filter_ref_deltas[AV1_GOLDEN_FRAME] = -1;
      lfp->loop_filter_ref_deltas[AV1_ALTREF2_FRAME] = -1;
      lfp->loop_filter_ref_deltas[AV1_ALTREF_FRAME] = -1;
      lfp->loop_filter_mode_deltas[0] = 0;
      lfp->loop_filter_mode_deltas[1] = 0;
   } else {
      int r = hdr->ref_frame_idx[hdr->primary_ref_frame];
      memcpy(lfp->loop_filter_ref_deltas,
            priv->codec_data.av1.refs[r].lfp.loop_filter_ref_deltas, 8);
      memcpy(lfp->loop_filter_mode_deltas,
            priv->codec_data.av1.refs[r].lfp.loop_filter_mode_deltas, 2);
   }

   lfp->loop_filter_level[0] = av1_f(vlc, 6);
   lfp->loop_filter_level[1] = av1_f(vlc, 6);
   if (seq->color_config.NumPlanes > 1) {
      if (lfp->loop_filter_level[0] || lfp->loop_filter_level[1]) {
         lfp->loop_filter_level[2] = av1_f(vlc, 6);
         lfp->loop_filter_level[3] = av1_f(vlc, 6);
      }
   }

   lfp->loop_filter_sharpness = av1_f(vlc, 3);
   lfp->loop_filter_delta_enabled = av1_f(vlc, 1);
   if (lfp->loop_filter_delta_enabled) {
      lfp->loop_filter_delta_update = av1_f(vlc, 1);
      if (lfp->loop_filter_delta_update) {
         for (i = 0; i < AV1_NUM_REF_FRAMES; ++i) {
            int8_t update_ref_delta = av1_f(vlc, 1);
            if (update_ref_delta)
               lfp->loop_filter_ref_deltas[i] = av1_su(vlc, 7);
         }

         for (i = 0; i < 2; ++i) {
            int8_t update_mode_delta = av1_f(vlc, 1);
            if (update_mode_delta)
               lfp->loop_filter_mode_deltas[i] = av1_su(vlc, 7);
         }
      }
   }
}

static void cdef_params(vid_dec_PrivateType *priv, struct vl_vlc *vlc)
{
   struct av1_sequence_header_obu *seq = &(priv->codec_data.av1.seq);
   struct av1_uncompressed_header_obu *hdr = &(priv->codec_data.av1.uncompressed_header);
   struct cdef_params *cdefp = &(priv->codec_data.av1.uncompressed_header.cdefp);;
   int i;

   if (hdr->CodedLossless || hdr->allow_intrabc || !seq->enable_cdef) {
      cdefp->cdef_bits = 0;
      cdefp->cdef_y_strengths[0] = 0;
      cdefp->cdef_uv_strengths[0] = 0;
      return;
   }

   cdefp->cdef_damping_minus_3 = av1_f(vlc, 2);
   cdefp->cdef_bits = av1_f(vlc, 2);
   for (i = 0; i < (1 << cdefp->cdef_bits); ++i) {
      cdefp->cdef_y_strengths[i] = av1_f(vlc, 6);
      if (seq->color_config.NumPlanes > 1)
         cdefp->cdef_uv_strengths[i] = av1_f(vlc, 6);
   }
}

static void lr_params(vid_dec_PrivateType *priv, struct vl_vlc *vlc)
{
   struct av1_sequence_header_obu *seq = &(priv->codec_data.av1.seq);
   struct av1_uncompressed_header_obu *hdr = &(priv->codec_data.av1.uncompressed_header);
   struct loop_restoration_params *lrp = &(priv->codec_data.av1.uncompressed_header.lrp);
   uint8_t Remap_Lr_Type[4] =
      { AV1_RESTORE_NONE, AV1_RESTORE_SWITCHABLE, AV1_RESTORE_WIENER, AV1_RESTORE_SGRPROJ };
   bool UsesLr = false;
   bool UsesChromaLr = false;
   uint8_t lr_unit_shift, lr_uv_shift;
   int i;

   if (hdr->AllLossless || hdr->allow_intrabc || !seq->enable_restoration) {
      lrp->FrameRestorationType[0] = AV1_RESTORE_NONE;
      lrp->FrameRestorationType[1] = AV1_RESTORE_NONE;
      lrp->FrameRestorationType[2] = AV1_RESTORE_NONE;
      return;
   }

   for (i = 0; i < seq->color_config.NumPlanes; ++i) {
      uint8_t lr_type = av1_f(vlc, 2);
      lrp->FrameRestorationType[i] = Remap_Lr_Type[lr_type];
      if (lrp->FrameRestorationType[i] != AV1_RESTORE_NONE) {
         UsesLr = true;
         if (i > 0)
            UsesChromaLr = true;
      }
   }

   if (UsesLr) {
      if (seq->use_128x128_superblock) {
         lr_unit_shift = av1_f(vlc, 1) + 1;
      } else {
         lr_unit_shift = av1_f(vlc, 1);
         if (lr_unit_shift) {
            uint8_t lr_unit_extra_shift = av1_f(vlc, 1);
            lr_unit_shift += lr_unit_extra_shift;
         }
      }

      lrp->LoopRestorationSize[0] = AV1_RESTORATION_TILESIZE >> (2 - lr_unit_shift);
      lr_uv_shift =
         (seq->color_config.subsampling_x && seq->color_config.subsampling_y && UsesChromaLr) ?
         av1_f(vlc, 1) : 0;

      lrp->LoopRestorationSize[1] = lrp->LoopRestorationSize[0] >> lr_uv_shift;
      lrp->LoopRestorationSize[2] = lrp->LoopRestorationSize[0] >> lr_uv_shift;
   } else {
      lrp->LoopRestorationSize[0] = lrp->LoopRestorationSize[1] =
         lrp->LoopRestorationSize[2] = (1 << 8);
   }
}

static void tx_mode(vid_dec_PrivateType *priv, struct vl_vlc *vlc)
{
   struct av1_uncompressed_header_obu *hdr = &(priv->codec_data.av1.uncompressed_header);
   struct tx_mode_params *tm = &(priv->codec_data.av1.uncompressed_header.tm);

   if (hdr->CodedLossless) {
      tm->TxMode = AV1_ONLY_4X4;
   } else {
      bool tx_mode_select = av1_f(vlc, 1);
      tm->TxMode = (tx_mode_select) ? AV1_TX_MODE_SELECT : AV1_TX_MODE_LARGEST;
   }
}

static void frame_reference_mode(vid_dec_PrivateType *priv, struct vl_vlc *vlc)
{
   struct av1_uncompressed_header_obu *hdr = &(priv->codec_data.av1.uncompressed_header);

   if (hdr->FrameIsIntra)
      hdr->reference_select = SINGLE_REFERENCE;
   else
      hdr->reference_select = av1_f(vlc, 1) ? REFERENCE_MODE_SELECT : SINGLE_REFERENCE;
}

static void skip_mode_params(vid_dec_PrivateType *priv, struct vl_vlc *vlc)
{
   struct av1_sequence_header_obu *seq = &(priv->codec_data.av1.seq);
   struct av1_uncompressed_header_obu *hdr = &(priv->codec_data.av1.uncompressed_header);
   struct skip_mode_params *smp = &(priv->codec_data.av1.uncompressed_header.smp);;
   bool skipModeAllowed;
   int i;

   if (hdr->FrameIsIntra || hdr->reference_select == SINGLE_REFERENCE ||
         !seq->enable_order_hint) {
      skipModeAllowed = 0;
   } else {
      int ref_frame_offset[2] = { -1, INT_MAX };
      int ref_idx[2] = { -1, -1 };

      skipModeAllowed = 0;
      for (i = 0; i < AV1_REFS_PER_FRAME; ++i) {
         unsigned ref_offset = priv->codec_data.av1.refs[hdr->ref_frame_idx[i]].OrderHint;
         if (get_relative_dist(priv, ref_offset, hdr->OrderHint) < 0) {
            if (ref_frame_offset[0] == -1 ||
               get_relative_dist(priv, ref_offset, ref_frame_offset[0]) > 0) {
               ref_frame_offset[0] = ref_offset;
               ref_idx[0] = i;
            }
         } else if (get_relative_dist(priv, ref_offset, hdr->OrderHint) > 0) {
            if (ref_frame_offset[1] == INT_MAX ||
               get_relative_dist(priv, ref_offset, ref_frame_offset[1]) < 0) {
               ref_frame_offset[1] = ref_offset;
               ref_idx[1] = i;
            }
         }
      }

      if (ref_idx[0] != -1 && ref_idx[1] != -1) {
         skipModeAllowed = 1;
      } else if (ref_idx[0] != -1 && ref_idx[1] == -1) {
         ref_frame_offset[1] = -1;
         for (i = 0; i < AV1_ALTREF_FRAME - AV1_LAST_FRAME + 1; ++i) {
            unsigned ref_offset = priv->codec_data.av1.refs[hdr->ref_frame_idx[i]].OrderHint;
            if ((ref_frame_offset[0] != -1 &&
                get_relative_dist(priv, ref_offset, ref_frame_offset[0]) < 0) &&
                (ref_frame_offset[1] == -1 ||
                get_relative_dist(priv, ref_offset, ref_frame_offset[1]) > 0)) {
               ref_frame_offset[1] = ref_offset;
               ref_idx[1] = i;
            }
         }
         if (ref_frame_offset[1] != -1)
            skipModeAllowed = 1;
      }
   }

   smp->skip_mode_present = skipModeAllowed ? av1_f(vlc, 1) : 0;
}

static unsigned inverse_recenter(unsigned r, unsigned v)
{
   if (v > (2 * r))
      return v;
   else if (v & 1)
      return (r - ((v + 1) >> 1));
   else
      return (r + (v >> 1));
}

static unsigned decode_subexp(struct vl_vlc *vlc, unsigned numSyms)
{
   unsigned i = 0;
   unsigned mk = 0;
   unsigned k = 3;

   while (1) {
      unsigned b2 = (i) ? (k + i - 1) : k;
      unsigned a = 1 << b2;
      if (numSyms <= (mk + 3 * a)) {
         unsigned subexp_final_bits = av1_ns(vlc, (numSyms - mk));
         return (subexp_final_bits + mk);
      } else {
         bool subexp_more_bits = av1_f(vlc, 1);
         if (subexp_more_bits) {
            i++;
            mk += a;
         } else {
            unsigned subexp_bits = av1_f(vlc, b2);
            return (subexp_bits + mk);
         }
      }
   }
}

static unsigned decode_unsigned_subexp_with_ref(struct vl_vlc *vlc,
      unsigned mx, unsigned r)
{
   unsigned smart;
   unsigned v = decode_subexp(vlc, mx);

   if ((r << 1) <= mx) {
      smart = inverse_recenter(r, v);
      return smart;
   } else {
      smart = inverse_recenter(mx - 1 - r, v);
      return (mx - 1 - smart);
   }
}

static int decode_signed_subexp_with_ref(struct vl_vlc *vlc, int low, int high, int r)
{
   int x = decode_unsigned_subexp_with_ref(vlc, high - low, r - low);

   return (x + low);
}

static void read_global_param(struct global_motion_params* global_params,
                              struct global_motion_params* ref_params,
                              vid_dec_PrivateType *priv, struct vl_vlc *vlc,
                              uint8_t type, uint8_t ref, uint8_t idx)
{
   struct av1_uncompressed_header_obu *hdr = &(priv->codec_data.av1.uncompressed_header);
   uint8_t absBits = 12; /* GM_ABS_ALPHA_BITS */
   uint8_t precBits = 15; /* GM_ALPHA_PREC_BITS */
   int precDiff, round, sub, mx, r = 0;

   if (idx < 2) {
      if (type == AV1_TRANSLATION) {
         absBits = 9 /* GM_ABS_TRANS_ONLY_BITS */ - !hdr->allow_high_precision_mv;
         precBits = 3 /* GM_TRANS_ONLY_PREC_BITS */ - !hdr->allow_high_precision_mv;
      } else {
         absBits = 12; /* GM_ABS_TRANS_BITS */
         precBits = 6; /* GM_TRANS_PREC_BITS */;
      }
   }

   precDiff = AV1_WARPEDMODEL_PREC_BITS - precBits;
   round = ((idx % 3) == 2) ? (1 << AV1_WARPEDMODEL_PREC_BITS) : 0;
   sub = ((idx % 3) == 2) ? (1 << precBits) : 0;
   mx = (int)(1 << absBits);
   if (ref_params)
      r = (ref_params->gm_params[ref][idx] >> precDiff) - sub;

   global_params->gm_params[ref][idx] =
      (decode_signed_subexp_with_ref(vlc, -mx, mx + 1, r) << precDiff) + round;
}

static void global_motion_params(vid_dec_PrivateType *priv, struct vl_vlc *vlc)
{
   struct av1_uncompressed_header_obu *hdr = &(priv->codec_data.av1.uncompressed_header);
   struct global_motion_params *gmp = &(priv->codec_data.av1.uncompressed_header.gmp);
   struct global_motion_params *ref_gmp = NULL;
   unsigned ref, i;

   if (hdr->primary_ref_frame == AV1_PRIMARY_REF_NONE) {
      for (ref = 0; ref < AV1_NUM_REF_FRAMES; ++ref) {
         gmp->GmType[ref] = AV1_IDENTITY;
         for (i = 0; i < 6; ++i)
            gmp->gm_params[ref][i] = (((i % 3) == 2) ? (1 << AV1_WARPEDMODEL_PREC_BITS) : 0);
      }
   } else {
      const int r = hdr->ref_frame_idx[hdr->primary_ref_frame];
      ref_gmp = &(priv->codec_data.av1.refs[r].gmp);
   }

   for (ref = AV1_LAST_FRAME; ref <= AV1_ALTREF_FRAME; ++ref) {
      gmp->GmType[ref] = AV1_IDENTITY;
      for (i = 0; i < 6; ++i)
         gmp->gm_params[ref][i] = (((i % 3) == 2) ? (1 << AV1_WARPEDMODEL_PREC_BITS) : 0);
   }

   if (hdr->FrameIsIntra)
      return;

   for (ref = AV1_LAST_FRAME; ref <= AV1_ALTREF_FRAME; ++ref) {
      uint8_t type = AV1_IDENTITY;
      bool is_global;

      gmp->GmType[ref] = AV1_IDENTITY;
      for (i = 0; i < 6; ++i)
         gmp->gm_params[ref][i] = (((i % 3) == 2) ? (1 << AV1_WARPEDMODEL_PREC_BITS) : 0);

      is_global = av1_f(vlc, 1);
      if (is_global) {
         bool is_rot_zoom = av1_f(vlc, 1);
         if (is_rot_zoom) {
            type = AV1_ROTZOOM;
         } else {
            bool is_translation = av1_f(vlc, 1);
            type = is_translation ? AV1_TRANSLATION : AV1_AFFINE;
         }
      }

      gmp->GmType[ref] = type;

      if (type >= AV1_ROTZOOM) {
         read_global_param(gmp, ref_gmp, priv, vlc, type, ref, 2);
         read_global_param(gmp, ref_gmp, priv, vlc, type, ref, 3);
         if (type == AV1_AFFINE) {
            read_global_param(gmp, ref_gmp, priv, vlc, type, ref, 4);
            read_global_param(gmp, ref_gmp, priv, vlc, type, ref, 5);
         } else {
            gmp->gm_params[ref][4] = -gmp->gm_params[ref][3];
            gmp->gm_params[ref][5] = gmp->gm_params[ref][2];
         }
      }

      if (type >= AV1_TRANSLATION) {
         read_global_param(gmp, ref_gmp, priv, vlc, type, ref, 0);
         read_global_param(gmp, ref_gmp, priv, vlc, type, ref, 1);
      }
   }
}

static void film_grain_params(vid_dec_PrivateType *priv, struct vl_vlc *vlc)
{
   struct av1_sequence_header_obu *seq = &(priv->codec_data.av1.seq);
   struct av1_uncompressed_header_obu *hdr = &(priv->codec_data.av1.uncompressed_header);
   struct film_grain_params *fgp = &(priv->codec_data.av1.uncompressed_header.fgp);

   bool update_grain;
   uint8_t numPosLuma;
   uint8_t numPosChroma;
   unsigned i;

   if (!seq->film_grain_params_present ||
         (!hdr->show_frame && !hdr->showable_frame)) {
      memset(fgp, 0, sizeof(*fgp));
      return;
   }

   fgp->apply_grain = av1_f(vlc, 1);
   if (!fgp->apply_grain) {
      memset(fgp, 0, sizeof(*fgp));
      return;
   }

   fgp->grain_seed = av1_f(vlc, 16);
   update_grain =
      (hdr->frame_type == AV1_INTER_FRAME) ? av1_f(vlc, 1) : 1;

   if (!update_grain) {
      uint8_t film_grain_params_ref_idx = av1_f(vlc, 3);
      uint16_t tempGrainSeed = fgp->grain_seed;
      memcpy(fgp, &(priv->codec_data.av1.refs[film_grain_params_ref_idx].fgp),
            sizeof(*fgp));
      fgp->grain_seed = tempGrainSeed;
      return;
   }

   fgp->num_y_points = av1_f(vlc, 4);
   for (i = 0; i < fgp->num_y_points; ++i) {
      fgp->point_y_value[i] = av1_f(vlc, 8);
      fgp->point_y_scaling[i] = av1_f(vlc, 8);
   }

   fgp->chroma_scaling_from_luma =
      (seq->color_config.mono_chrome) ? 0 : av1_f(vlc, 1);
   if (seq->color_config.mono_chrome || fgp->chroma_scaling_from_luma ||
       (seq->color_config.subsampling_x && seq->color_config.subsampling_y &&
       (fgp->num_y_points == 0))) {
      fgp->num_cb_points = 0;
      fgp->num_cr_points = 0;
   } else {
      fgp->num_cb_points = av1_f(vlc, 4);
      for (i = 0; i < fgp->num_cb_points; ++i) {
         fgp->point_cb_value[i] = av1_f(vlc, 8);
         fgp->point_cb_scaling[i] = av1_f(vlc, 8);
      }
      fgp->num_cr_points = av1_f(vlc, 4);
      for (i = 0; i < fgp->num_cr_points; ++i) {
         fgp->point_cr_value[i] = av1_f(vlc, 8);
         fgp->point_cr_scaling[i] = av1_f(vlc, 8);
      }
   }

   fgp->grain_scaling_minus_8 = av1_f(vlc, 2);
   fgp->ar_coeff_lag = av1_f(vlc, 2);
   numPosLuma = 2 * fgp->ar_coeff_lag * (fgp->ar_coeff_lag + 1);
   if (fgp->num_y_points) {
      numPosChroma = numPosLuma + 1;
      for (i = 0; i < numPosLuma; ++i) {
         uint8_t ar_coeffs_y_plus_128 = av1_f(vlc, 8);
         fgp->ar_coeffs_y[i] = ar_coeffs_y_plus_128 - 128;
      }
   } else {
      numPosChroma = numPosLuma;
   }

   if (fgp->chroma_scaling_from_luma || fgp->num_cb_points) {
      for (i = 0; i < numPosChroma; ++i) {
         uint8_t ar_coeffs_cb_plus_128 = av1_f(vlc, 8);
         fgp->ar_coeffs_cb[i] = ar_coeffs_cb_plus_128 - 128;
      }
   }

   if (fgp->chroma_scaling_from_luma || fgp->num_cr_points) {
      for (i = 0; i < numPosChroma; ++i) {
         uint8_t ar_coeffs_cr_plus_128 = av1_f(vlc, 8);
         fgp->ar_coeffs_cr[i] = ar_coeffs_cr_plus_128 - 128;
      }
   }

   fgp->ar_coeff_shift_minus_6 = av1_f(vlc, 2);
   fgp->grain_scale_shift = av1_f(vlc, 2);
   if (fgp->num_cb_points) {
      fgp->cb_mult = av1_f(vlc, 8);
      fgp->cb_luma_mult = av1_f(vlc, 8);
      fgp->cb_offset = av1_f(vlc, 9);
   }

   if (fgp->num_cr_points) {
      fgp->cr_mult = av1_f(vlc, 8);
      fgp->cr_luma_mult = av1_f(vlc, 8);
      fgp->cr_offset = av1_f(vlc, 9);
   }

   fgp->overlap_flag = av1_f(vlc, 1);
   fgp->clip_to_restricted_range = av1_f(vlc, 1);
}

static void frame_header_obu(vid_dec_PrivateType *priv, struct vl_vlc *vlc)
{
   struct av1_sequence_header_obu *seq = &(priv->codec_data.av1.seq);
   struct av1_uncompressed_header_obu *hdr = &(priv->codec_data.av1.uncompressed_header);
   unsigned idLen = 0;
   unsigned allFrames;
   int i, j;

   memset(hdr, 0, sizeof(*hdr));

   if (seq->frame_id_numbers_present_flag)
      idLen = seq->additional_frame_id_length_minus_1 +
         seq->delta_frame_id_length_minus_2 + 3;

   allFrames = (1 << AV1_NUM_REF_FRAMES) - 1;
   if (seq->reduced_still_picture_header) {
      hdr->show_existing_frame = 0;
      hdr->frame_type = AV1_KEY_FRAME;
      hdr->FrameIsIntra = 1;
      hdr->show_frame = 1;
      hdr->showable_frame = 0;
   } else {
      hdr->show_existing_frame = av1_f(vlc, 1);
      if (hdr->show_existing_frame) {
         hdr->frame_to_show_map_idx = av1_f(vlc, 3);
         if (seq->decoder_model_info_present_flag &&
               !seq->timing_info.equal_picture_interval)
            av1_f(vlc, seq->decoder_model_info.
                  frame_presentation_time_length_minus_1 + 1);
         hdr->refresh_frame_flags  = 0;
         if (seq->frame_id_numbers_present_flag)
            av1_f(vlc, idLen); /* display_frame_id */

         hdr->frame_type =
            priv->codec_data.av1.RefFrames[priv->codec_data.av1.uncompressed_header.
               frame_to_show_map_idx].RefFrameType;

         return;
      }

      hdr->frame_type = av1_f(vlc, 2);
      hdr->FrameIsIntra = (hdr->frame_type == AV1_INTRA_ONLY_FRAME ||
            hdr->frame_type == AV1_KEY_FRAME);
      hdr->show_frame = av1_f(vlc, 1);

      if (hdr->show_frame && seq->decoder_model_info_present_flag &&
            !seq->timing_info.equal_picture_interval)
         av1_f(vlc, seq->decoder_model_info.frame_presentation_time_length_minus_1 + 1);

      hdr->showable_frame =
         hdr->show_frame ? (hdr->frame_type != AV1_KEY_FRAME) : av1_f(vlc, 1);

      hdr->error_resilient_mode = (hdr->frame_type == AV1_SWITCH_FRAME ||
          (hdr->frame_type == AV1_KEY_FRAME && hdr->show_frame)) ? 1 : av1_f(vlc, 1);
   }

   if (hdr->frame_type == AV1_KEY_FRAME && hdr->show_frame) {
      for (i = 0; i < AV1_NUM_REF_FRAMES; ++i)
         hdr->RefOrderHint[i] = 0;
   }

   hdr->disable_cdf_update = av1_f(vlc, 1);

   hdr->allow_screen_content_tools =
      (seq->seq_force_screen_content_tools == AV1_SELECT_SCREEN_CONTENT_TOOLS) ?
      av1_f(vlc, 1) : seq->seq_force_screen_content_tools;

   if (hdr->allow_screen_content_tools) {
      if (seq->seq_force_integer_mv == AV1_SELECT_INTEGER_MV)
         hdr->force_integer_mv = av1_f(vlc, 1);
      else
         hdr->force_integer_mv = seq->seq_force_integer_mv;
   } else {
      hdr->force_integer_mv = 0;
   }

   if (hdr->FrameIsIntra)
      hdr->force_integer_mv = 1;

   hdr->current_frame_id =
      seq->frame_id_numbers_present_flag ? av1_f(vlc, idLen) : 0;

   if (hdr->frame_type == AV1_SWITCH_FRAME)
      hdr->frame_size_override_flag = 1;
   else if (seq->reduced_still_picture_header)
      hdr->frame_size_override_flag = 0;
   else
      hdr->frame_size_override_flag = av1_f(vlc, 1);

   hdr->OrderHint = av1_f(vlc, seq->OrderHintBits);

   if (hdr->FrameIsIntra || hdr->error_resilient_mode)
      hdr->primary_ref_frame = AV1_PRIMARY_REF_NONE;
   else
      hdr->primary_ref_frame = av1_f(vlc, 3);

   if (seq->decoder_model_info_present_flag) {
      bool buffer_removal_time_present_flag = av1_f(vlc, 1);
      if (buffer_removal_time_present_flag) {
         for (i = 0; i <= seq->operating_points_cnt_minus_1; ++i) {
            if (seq->decoder_model_present_for_this_op[i]) {
               unsigned opPtIdc;
               bool inTemporalLayer;
               bool inSpatialLayer;
               opPtIdc = seq->operating_point_idc[i];
               inTemporalLayer =
                  (opPtIdc >> priv->codec_data.av1.ext.temporal_id) & 1;
               inSpatialLayer =
                  (opPtIdc >> (priv->codec_data.av1.ext.spatial_id + 8)) & 1;
               if ((opPtIdc == 0) || (inTemporalLayer && inSpatialLayer))
                  av1_f(vlc, seq->decoder_model_info.
                        buffer_removal_time_length_minus_1 + 1);
            }
         }
      }
   }

   hdr->allow_high_precision_mv = 0;
   hdr->use_ref_frame_mvs = 0;
   hdr->allow_intrabc = 0;

   hdr->refresh_frame_flags = allFrames = (hdr->frame_type == AV1_SWITCH_FRAME ||
         (hdr->frame_type == AV1_KEY_FRAME && hdr->show_frame)) ?
      allFrames : av1_f(vlc, AV1_NUM_REF_FRAMES);

   if (!hdr->FrameIsIntra || hdr->refresh_frame_flags != allFrames) {
      if (hdr->error_resilient_mode && seq->enable_order_hint) {
         for (i = 0; i < AV1_NUM_REF_FRAMES; ++i)
            av1_f(vlc, seq->OrderHintBits);
      }
   }

   if (hdr->FrameIsIntra) {
      frame_size(priv, vlc);
      render_size(priv, vlc);
      if (hdr->allow_screen_content_tools && (hdr->UpscaledWidth == hdr->FrameWidth))
         hdr->allow_intrabc = av1_f(vlc, 1);
   } else {
      bool is_filter_switchable;
      bool frame_refs_short_signaling;

      if (!seq->enable_order_hint) {
         frame_refs_short_signaling = 0;
      } else {
         frame_refs_short_signaling = av1_f(vlc, 1);
         if (frame_refs_short_signaling) {
            hdr->last_frame_idx = av1_f(vlc, 3);
            hdr->gold_frame_idx = av1_f(vlc, 3);
            set_frame_refs(priv, vlc);
         }
      }

      for (i = 0; i < AV1_REFS_PER_FRAME; ++i) {
         if (!frame_refs_short_signaling)
            hdr->ref_frame_idx[i] = av1_f(vlc, 3);
         if (seq->frame_id_numbers_present_flag)
            av1_f(vlc, seq->delta_frame_id_length_minus_2 + 2);
      }

      if (hdr->frame_size_override_flag && !hdr->error_resilient_mode) {
         frame_size_with_refs(priv, vlc);
      } else {
         frame_size(priv, vlc);
         render_size(priv, vlc);
      }

      hdr->allow_high_precision_mv = hdr->force_integer_mv ? 0 : av1_f(vlc, 1);

      is_filter_switchable = av1_f(vlc, 1);
      hdr->interpolation_filter = is_filter_switchable ? 4 /* SWITCHABLE */ : av1_f(vlc, 2);

      hdr->is_motion_mode_switchable = av1_f(vlc, 1);
      hdr->use_ref_frame_mvs =
         (hdr->error_resilient_mode || !seq->enable_ref_frame_mvs) ? 0 : av1_f(vlc, 1);
   }

   hdr->disable_frame_end_update_cdf =
      (seq->reduced_still_picture_header || hdr->disable_cdf_update) ? 1 : av1_f(vlc, 1);

   tile_info(priv, vlc);
   quantization_params(priv, vlc);
   segmentation_params(priv, vlc);
   delta_q_params(priv, vlc);
   delta_lf_params(priv, vlc);

   hdr->CodedLossless = 1;
   for (i = 0; i < AV1_MAX_SEGMENTS; ++i) {
      unsigned qindex = get_qindex(priv, 1, i);
      bool LosslessArray =
         (qindex == 0) && (hdr->qp.DeltaQYDc == 0) &&
         (hdr->qp.DeltaQUAc == 0) && (hdr->qp.DeltaQUDc == 0) &&
         (hdr->qp.DeltaQVAc == 0) && (hdr->qp.DeltaQVDc == 0);

      if (!LosslessArray)
         hdr->CodedLossless = 0;
   }
   hdr->AllLossless = hdr->CodedLossless && (hdr->FrameWidth == hdr->UpscaledWidth);

   loop_filter_params(priv, vlc);
   cdef_params(priv, vlc);
   lr_params(priv, vlc);
   tx_mode(priv, vlc);
   frame_reference_mode(priv, vlc);
   skip_mode_params(priv, vlc);

   if (hdr->FrameIsIntra || hdr->error_resilient_mode || !seq->enable_warped_motion)
      hdr->allow_warped_motion = 0;
   else
      hdr->allow_warped_motion = av1_f(vlc, 1);
   hdr->reduced_tx_set = av1_f(vlc, 1);

   global_motion_params(priv, vlc);

   film_grain_params(priv, vlc);

   priv->picture.av1.film_grain_target = NULL;
   priv->picture.av1.picture_parameter.pic_info_fields.frame_type = hdr->frame_type;
   priv->picture.av1.picture_parameter.pic_info_fields.show_frame = hdr->show_frame;
   priv->picture.av1.picture_parameter.pic_info_fields.error_resilient_mode =
      hdr->error_resilient_mode;
   priv->picture.av1.picture_parameter.pic_info_fields.disable_cdf_update =
      hdr->disable_cdf_update;
   priv->picture.av1.picture_parameter.pic_info_fields.allow_screen_content_tools =
      hdr->allow_screen_content_tools;
   priv->picture.av1.picture_parameter.pic_info_fields.force_integer_mv =
      hdr->force_integer_mv;
   priv->picture.av1.picture_parameter.current_frame_id = hdr->current_frame_id;
   priv->picture.av1.picture_parameter.order_hint = hdr->OrderHint;
   priv->picture.av1.picture_parameter.primary_ref_frame = hdr->primary_ref_frame;
   priv->picture.av1.picture_parameter.frame_width = hdr->FrameWidth;
   priv->picture.av1.picture_parameter.frame_height = hdr->FrameHeight;
   priv->picture.av1.picture_parameter.pic_info_fields.use_superres =
      hdr->use_superres;
   priv->picture.av1.picture_parameter.superres_scale_denominator =
      hdr->SuperresDenom;

   for (i = 0; i < AV1_REFS_PER_FRAME; ++i)
      priv->picture.av1.picture_parameter.ref_frame_idx[i] = hdr->ref_frame_idx[i];

   priv->picture.av1.picture_parameter.pic_info_fields.allow_high_precision_mv =
      hdr->allow_high_precision_mv;
   priv->picture.av1.picture_parameter.pic_info_fields.allow_intrabc = hdr->allow_intrabc;
   priv->picture.av1.picture_parameter.pic_info_fields.use_ref_frame_mvs =
      hdr->use_ref_frame_mvs;
   priv->picture.av1.picture_parameter.interp_filter = hdr->interpolation_filter;
   priv->picture.av1.picture_parameter.pic_info_fields.is_motion_mode_switchable =
      hdr->is_motion_mode_switchable;
   priv->picture.av1.picture_parameter.refresh_frame_flags =
      hdr->refresh_frame_flags;
   priv->picture.av1.picture_parameter.pic_info_fields.disable_frame_end_update_cdf =
      hdr->disable_frame_end_update_cdf;

   /* Tile Info */
   priv->picture.av1.picture_parameter.tile_rows = hdr->ti.TileRows;
   priv->picture.av1.picture_parameter.tile_cols = hdr->ti.TileCols;
   priv->picture.av1.picture_parameter.context_update_tile_id =
      hdr->ti.context_update_tile_id;
   for (i = 0; i <AV1_MAX_TILE_ROWS; ++i)
      priv->picture.av1.picture_parameter.tile_row_start_sb[i] =
         hdr->ti.tile_row_start_sb[i];
   for (i = 0; i <AV1_MAX_TILE_COLS; ++i)
      priv->picture.av1.picture_parameter.tile_col_start_sb[i] =
         hdr->ti.tile_col_start_sb[i];

   /* Quantization Params */
   priv->picture.av1.picture_parameter.base_qindex =  hdr->qp.base_q_idx;
   priv->picture.av1.picture_parameter.y_dc_delta_q = hdr->qp.DeltaQYDc;
   priv->picture.av1.picture_parameter.u_dc_delta_q = hdr->qp.DeltaQUDc;
   priv->picture.av1.picture_parameter.u_ac_delta_q = hdr->qp.DeltaQUAc;
   priv->picture.av1.picture_parameter.v_dc_delta_q = hdr->qp.DeltaQVDc;
   priv->picture.av1.picture_parameter.v_ac_delta_q = hdr->qp.DeltaQVAc;
   priv->picture.av1.picture_parameter.qmatrix_fields.qm_y = hdr->qp.qm_y;
   priv->picture.av1.picture_parameter.qmatrix_fields.qm_u = hdr->qp.qm_u;
   priv->picture.av1.picture_parameter.qmatrix_fields.qm_v = hdr->qp.qm_v;

   /* Segmentation Params */
   priv->picture.av1.picture_parameter.seg_info.segment_info_fields.enabled =
      hdr->sp.segmentation_enabled;
   priv->picture.av1.picture_parameter.seg_info.segment_info_fields.update_map =
      hdr->sp.segmentation_update_map;
   priv->picture.av1.picture_parameter.seg_info.segment_info_fields.temporal_update =
      hdr->sp.segmentation_temporal_update;
   for (i = 0; i < AV1_MAX_SEGMENTS; ++i) {
      for (j = 0; j < AV1_SEG_LVL_MAX; ++j)
         priv->picture.av1.picture_parameter.seg_info.feature_data[i][j] =
            hdr->sp.FeatureData[i][j];
      priv->picture.av1.picture_parameter.seg_info.feature_mask[i] =
         hdr->sp.FeatureMask[i];
   }

   /* Delta Q Params */
   priv->picture.av1.picture_parameter.mode_control_fields.delta_q_present_flag =
      hdr->dqp.delta_q_present;
   priv->picture.av1.picture_parameter.mode_control_fields.log2_delta_q_res =
      hdr->dqp.delta_q_res;

   /* Delta LF Params */
   priv->picture.av1.picture_parameter.mode_control_fields.delta_lf_present_flag =
      hdr->dlfp.delta_lf_present;
   priv->picture.av1.picture_parameter.mode_control_fields.log2_delta_lf_res =
      hdr->dlfp.delta_lf_res;
   priv->picture.av1.picture_parameter.mode_control_fields.delta_lf_multi =
      hdr->dlfp.delta_lf_multi;

   /* Loop Filter Params */
   for (i = 0; i < 2; ++i)
      priv->picture.av1.picture_parameter.filter_level[i] = hdr->lfp.loop_filter_level[i];
   priv->picture.av1.picture_parameter.filter_level_u = hdr->lfp.loop_filter_level[2];
   priv->picture.av1.picture_parameter.filter_level_v = hdr->lfp.loop_filter_level[3];
   priv->picture.av1.picture_parameter.loop_filter_info_fields.sharpness_level =
      hdr->lfp.loop_filter_sharpness;
   priv->picture.av1.picture_parameter.loop_filter_info_fields.mode_ref_delta_enabled =
      hdr->lfp.loop_filter_delta_enabled;
   priv->picture.av1.picture_parameter.loop_filter_info_fields.mode_ref_delta_update =
      hdr->lfp.loop_filter_delta_update;
   for (i = 0; i < AV1_NUM_REF_FRAMES; ++i)
      priv->picture.av1.picture_parameter.ref_deltas[i] =
         hdr->lfp.loop_filter_ref_deltas[i];
   for (i = 0; i < 2; ++i)
      priv->picture.av1.picture_parameter.mode_deltas[i] =
         hdr->lfp.loop_filter_mode_deltas[i];

   /* CDEF Params */
   priv->picture.av1.picture_parameter.cdef_damping_minus_3 =
      hdr->cdefp.cdef_damping_minus_3;
   priv->picture.av1.picture_parameter.cdef_bits = hdr->cdefp.cdef_bits;
   for (i = 0; i < AV1_MAX_CDEF_BITS_ARRAY; ++i) {
      priv->picture.av1.picture_parameter.cdef_y_strengths[i] =
         hdr->cdefp.cdef_y_strengths[i];
      priv->picture.av1.picture_parameter.cdef_uv_strengths[i] =
         hdr->cdefp.cdef_uv_strengths[i];
   }

   /* Loop Restoration Params */
   priv->picture.av1.picture_parameter.loop_restoration_fields.yframe_restoration_type =
      hdr->lrp.FrameRestorationType[0];
   priv->picture.av1.picture_parameter.loop_restoration_fields.cbframe_restoration_type =
      hdr->lrp.FrameRestorationType[1];
   priv->picture.av1.picture_parameter.loop_restoration_fields.crframe_restoration_type =
      hdr->lrp.FrameRestorationType[2];
   for (i = 0; i < 3; ++i)
      priv->picture.av1.picture_parameter.lr_unit_size[i] = hdr->lrp.LoopRestorationSize[i];

   priv->picture.av1.picture_parameter.mode_control_fields.tx_mode = hdr->tm.TxMode;
   priv->picture.av1.picture_parameter.mode_control_fields.reference_select =
      (hdr->reference_select == REFERENCE_MODE_SELECT) ? COMPOUND_REFERENCE : SINGLE_REFERENCE;
   priv->picture.av1.picture_parameter.mode_control_fields.skip_mode_present =
      hdr->smp.skip_mode_present;
   priv->picture.av1.picture_parameter.pic_info_fields.allow_warped_motion =
      hdr->allow_warped_motion;
   priv->picture.av1.picture_parameter.mode_control_fields.reduced_tx_set_used =
      hdr->reduced_tx_set;

   /* Global Motion Params */
   for (i = 0; i < 7; ++i) {
      priv->picture.av1.picture_parameter.wm[i].wmtype = hdr->gmp.GmType[i + 1];
      for (j = 0; j < 6; ++j)
         priv->picture.av1.picture_parameter.wm[i].wmmat[j] = hdr->gmp.gm_params[i + 1][j];
   }

   /* Film Grain Params */
   priv->picture.av1.picture_parameter.film_grain_info.film_grain_info_fields.apply_grain =
      hdr->fgp.apply_grain;
   priv->picture.av1.picture_parameter.film_grain_info.grain_seed =
      hdr->fgp.grain_seed;
   priv->picture.av1.picture_parameter.film_grain_info.num_y_points =
      hdr->fgp.num_y_points;
   for (i = 0; i < AV1_FG_MAX_NUM_Y_POINTS; ++i) {
      priv->picture.av1.picture_parameter.film_grain_info.point_y_value[i] =
         hdr->fgp.point_y_value[i];
      priv->picture.av1.picture_parameter.film_grain_info.point_y_scaling[i] =
         hdr->fgp.point_y_scaling[i];
   }
   priv->picture.av1.picture_parameter.film_grain_info.film_grain_info_fields.
      chroma_scaling_from_luma = hdr->fgp.chroma_scaling_from_luma;
   priv->picture.av1.picture_parameter.film_grain_info.num_cb_points =
      hdr->fgp.num_cb_points;
   priv->picture.av1.picture_parameter.film_grain_info.num_cr_points =
      hdr->fgp.num_cr_points;
   for (i = 0; i < AV1_FG_MAX_NUM_CBR_POINTS; ++i) {
      priv->picture.av1.picture_parameter.film_grain_info.point_cb_value[i] =
         hdr->fgp.point_cb_value[i];
      priv->picture.av1.picture_parameter.film_grain_info.point_cb_scaling[i] =
         hdr->fgp.point_cb_scaling[i];
      priv->picture.av1.picture_parameter.film_grain_info.point_cr_value[i] =
         hdr->fgp.point_cr_value[i];
      priv->picture.av1.picture_parameter.film_grain_info.point_cr_scaling[i] =
         hdr->fgp.point_cr_scaling[i];
   }
   priv->picture.av1.picture_parameter.film_grain_info.film_grain_info_fields.
      grain_scaling_minus_8 = hdr->fgp.grain_scaling_minus_8;
   priv->picture.av1.picture_parameter.film_grain_info.film_grain_info_fields.
      ar_coeff_lag = hdr->fgp.ar_coeff_lag;
   for (i = 0; i < AV1_FG_MAX_NUM_POS_LUMA; ++i)
      priv->picture.av1.picture_parameter.film_grain_info.ar_coeffs_y[i] =
         hdr->fgp.ar_coeffs_y[i];
   for (i = 0; i < AV1_FG_MAX_NUM_POS_CHROMA; ++i) {
      priv->picture.av1.picture_parameter.film_grain_info.ar_coeffs_cb[i] =
         hdr->fgp.ar_coeffs_cb[i];
      priv->picture.av1.picture_parameter.film_grain_info.ar_coeffs_cr[i] =
         hdr->fgp.ar_coeffs_cr[i];
   }
   priv->picture.av1.picture_parameter.film_grain_info.film_grain_info_fields.
      ar_coeff_shift_minus_6 = hdr->fgp.ar_coeff_shift_minus_6;
   priv->picture.av1.picture_parameter.film_grain_info.film_grain_info_fields.
      grain_scale_shift = hdr->fgp.grain_scale_shift;
   priv->picture.av1.picture_parameter.film_grain_info.cb_mult = hdr->fgp.cb_mult;
   priv->picture.av1.picture_parameter.film_grain_info.cb_luma_mult = hdr->fgp.cb_luma_mult;
   priv->picture.av1.picture_parameter.film_grain_info.cb_offset = hdr->fgp.cb_offset;
   priv->picture.av1.picture_parameter.film_grain_info.cr_mult = hdr->fgp.cr_mult;
   priv->picture.av1.picture_parameter.film_grain_info.cr_luma_mult = hdr->fgp.cr_luma_mult;
   priv->picture.av1.picture_parameter.film_grain_info.cr_offset = hdr->fgp.cr_offset;
   priv->picture.av1.picture_parameter.film_grain_info.film_grain_info_fields.
      overlap_flag = hdr->fgp.overlap_flag;
   priv->picture.av1.picture_parameter.film_grain_info.film_grain_info_fields.
      clip_to_restricted_range = hdr->fgp.clip_to_restricted_range;
}

static void parse_tile_hdr(vid_dec_PrivateType *priv, struct vl_vlc *vlc,
      unsigned start_bits_pos, unsigned total_obu_len)
{
   struct tile_info *ti = &(priv->codec_data.av1.uncompressed_header.ti);
   unsigned tg_start, tg_end;
   unsigned NumTiles, tileBits;
   bool tile_start_and_end_present_flag;
   unsigned size[AV1_MAX_NUM_TILES] = { 0 };
   unsigned offset[AV1_MAX_NUM_TILES] = { 0 };
   unsigned frame_header_size, left_size;
   unsigned i, j;

   NumTiles = ti->TileCols * ti->TileRows;
   tile_start_and_end_present_flag = 0;
   if (NumTiles > 1)
      tile_start_and_end_present_flag = av1_f(vlc, 1);

   if (NumTiles == 1 || !tile_start_and_end_present_flag) {
      tg_start = 0;
      tg_end = NumTiles - 1;
   } else {
      tileBits = ti->TileColsLog2 + ti->TileRowsLog2;
      tg_start = av1_f(vlc, tileBits);
      tg_end = av1_f(vlc, tileBits);
   }

   av1_byte_alignment(vlc);

   frame_header_size = (start_bits_pos - vl_vlc_bits_left(vlc)) / 8;
   left_size = total_obu_len - frame_header_size;
   for (i = tg_start; i <= tg_end; ++i) {
      if (i == tg_start) {
         offset[i] = priv->codec_data.av1.bs_obu_td_sz +
            priv->codec_data.av1.bs_obu_seq_sz + frame_header_size +
            ti->TileSizeBytes;
         if (tg_start == tg_end) {
            size[i] = left_size;
            for (j = 0; j < size[i]; ++j) {
               vl_vlc_fillbits(vlc);
               vl_vlc_eatbits(vlc, 8);
            }
            break;
         }
      } else {
         offset[i] = offset[i - 1] + ti->TileSizeBytes + size[i - 1];
         left_size -= ti->TileSizeBytes + size[i - 1];
      }

      if (i != tg_end) {
         size[i] = av1_le(vlc, ti->TileSizeBytes) + 1;
      } else {
         offset[i] = offset[i - 1] + size[i - 1];
         size[i] = left_size;
      }

      for (j = 0; j < size[i]; ++j) {
         vl_vlc_fillbits(vlc);
         vl_vlc_eatbits(vlc, 8);
      }
   }

   for (i = tg_start; i <= tg_end; ++i) {
      priv->picture.av1.slice_parameter.slice_data_offset[i] = offset[i];
      priv->picture.av1.slice_parameter.slice_data_size[i] = size[i];
   }
}

static struct dec_av1_task *dec_av1_NeedTask(vid_dec_PrivateType *priv)
{
   struct pipe_video_buffer templat = {};
   struct dec_av1_task *task;
   struct vl_screen *omx_screen;
   struct pipe_screen *pscreen;

   omx_screen = priv->screen;
   assert(omx_screen);

   pscreen = omx_screen->pscreen;
   assert(pscreen);

   if (!list_is_empty(&priv->codec_data.av1.free_tasks)) {
      task = list_entry(priv->codec_data.av1.free_tasks.next,
                        struct dec_av1_task, list);
      task->buf_ref_count = 1;
      list_del(&task->list);
      return task;
   }

   task = CALLOC_STRUCT(dec_av1_task);
   if (!task)
      return NULL;

   memset(&templat, 0, sizeof(templat));
   templat.width = priv->codec->width;
   templat.height = priv->codec->height;
   templat.buffer_format = pscreen->get_video_param(
         pscreen,
         PIPE_VIDEO_PROFILE_UNKNOWN,
         PIPE_VIDEO_ENTRYPOINT_BITSTREAM,
         PIPE_VIDEO_CAP_PREFERED_FORMAT
   );
   templat.interlaced = false;

   task->buf = priv->pipe->create_video_buffer(priv->pipe, &templat);
   if (!task->buf) {
      FREE(task);
      return NULL;
   }
   task->buf_ref_count = 1;
   task->is_sef_task = false;

   return task;
}

static void dec_av1_ReleaseTask(vid_dec_PrivateType *priv,
      struct list_head *head)
{
   if (!head || !head->next)
      return;

   list_for_each_entry_safe(struct dec_av1_task, task, head, list) {
      task->buf->destroy(task->buf);
      FREE(task);
   }
}

static void dec_av1_MoveTask(struct list_head *from,
      struct list_head *to)
{
   to->prev->next = from->next;
   from->next->prev = to->prev;
   from->prev->next = to;
   to->prev = from->prev;
   list_inithead(from);
}

static void dec_av1_SortTask(vid_dec_PrivateType *priv)
{
   int i;

   list_for_each_entry_safe(struct dec_av1_task, t,
         &priv->codec_data.av1.finished_tasks, list) {
      bool found = false;
      for (i = 0; i < 8; ++i) {
         if (t->buf == priv->picture.av1.ref[i]) {
            found = true;
            break;
         }
      }
      if (!found && t->buf_ref_count == 0) {
         list_del(&t->list);
         list_addtail(&t->list, &priv->codec_data.av1.free_tasks);
      }
   }
}

static struct dec_av1_task *dec_av1_SearchTask(vid_dec_PrivateType *priv,
      struct list_head *tasks)
{
   unsigned idx =
      priv->codec_data.av1.uncompressed_header.frame_to_show_map_idx;

   list_for_each_entry_safe(struct dec_av1_task, t, tasks, list) {
      if (t->buf == priv->picture.av1.ref[idx])
         return t;
   }

   return NULL;
}

static bool dec_av1_GetStartedTask(vid_dec_PrivateType *priv,
      struct dec_av1_task *task, struct list_head *tasks)
{
   struct dec_av1_task *started_task;

   ++priv->codec_data.av1.que_num;
   list_addtail(&task->list, &priv->codec_data.av1.started_tasks);
   if (priv->codec_data.av1.que_num <= 16)
      return false;

   started_task = list_entry(priv->codec_data.av1.started_tasks.next,
                             struct dec_av1_task, list);
   list_del(&started_task->list);
   list_addtail(&started_task->list, tasks);
   --priv->codec_data.av1.que_num;

   return true;
}

static void dec_av1_ShowExistingframe(vid_dec_PrivateType *priv)
{
   struct input_buf_private *inp = priv->in_buffers[0]->pInputPortPrivate;
   struct dec_av1_task *task, *existing_task;
   bool fnd;

   task = CALLOC_STRUCT(dec_av1_task);
   if (!task)
      return;

   task->is_sef_task = true;

   mtx_lock(&priv->codec_data.av1.mutex);
   dec_av1_MoveTask(&inp->tasks, &priv->codec_data.av1.finished_tasks);
   dec_av1_SortTask(priv);
   existing_task = dec_av1_SearchTask(priv, &priv->codec_data.av1.started_tasks);
   if (existing_task) {
      ++existing_task->buf_ref_count;
      task->buf = existing_task->buf;
      task->buf_ref = &existing_task->buf;
      task->buf_ref_count = 0;
   } else {
      existing_task = dec_av1_SearchTask(priv, &priv->codec_data.av1.finished_tasks);
      if (existing_task) {
         struct vl_screen *omx_screen;
         struct pipe_screen *pscreen;
         struct pipe_video_buffer templat = {};
         struct pipe_video_buffer *buf;
         struct pipe_box box={};

         omx_screen = priv->screen;
         assert(omx_screen);

         pscreen = omx_screen->pscreen;
         assert(pscreen);

         memset(&templat, 0, sizeof(templat));
         templat.width = priv->codec->width;
         templat.height = priv->codec->height;
         templat.buffer_format = pscreen->get_video_param(
            pscreen,
            PIPE_VIDEO_PROFILE_UNKNOWN,
            PIPE_VIDEO_ENTRYPOINT_BITSTREAM,
            PIPE_VIDEO_CAP_PREFERED_FORMAT
         );
         templat.interlaced = false;
         buf = priv->pipe->create_video_buffer(priv->pipe, &templat);
         if (!buf) {
            FREE(task);
            mtx_unlock(&priv->codec_data.av1.mutex);
            return;
         }

         box.width = priv->codec->width;
         box.height = priv->codec->height;
         box.depth = 1;
         priv->pipe->resource_copy_region(priv->pipe,
               ((struct vl_video_buffer *)buf)->resources[0],
               0, 0, 0, 0,
               ((struct vl_video_buffer *)(existing_task->buf))->resources[0],
               0, &box);
         box.width /= 2;
         box.height/= 2;
         priv->pipe->resource_copy_region(priv->pipe,
               ((struct vl_video_buffer *)buf)->resources[1],
               0, 0, 0, 0,
               ((struct vl_video_buffer *)(existing_task->buf))->resources[1],
               0, &box);
         priv->pipe->flush(priv->pipe, NULL, 0);
         existing_task->buf_ref_count = 0;
         task->buf = buf;
         task->buf_ref_count = 1;
      } else {
         FREE(task);
         mtx_unlock(&priv->codec_data.av1.mutex);
         return;
      }
   }
   dec_av1_SortTask(priv);

   fnd = dec_av1_GetStartedTask(priv, task, &inp->tasks);
   mtx_unlock(&priv->codec_data.av1.mutex);
   if (fnd)
      priv->frame_finished = 1;
}

static struct dec_av1_task *dec_av1_BeginFrame(vid_dec_PrivateType *priv)
{
   struct input_buf_private *inp = priv->in_buffers[0]->pInputPortPrivate;
   struct dec_av1_task *task;

   if (priv->frame_started)
      return NULL;

   if (!priv->codec) {
      struct vl_screen *omx_screen;
      struct pipe_screen *pscreen;
      struct pipe_video_codec templat = {};
      bool supported;

      omx_screen = priv->screen;
      assert(omx_screen);

      pscreen = omx_screen->pscreen;
      assert(pscreen);

      supported = vl_codec_supported(pscreen, priv->profile, false);
      assert(supported && "AV1 is not supported");

      templat.profile = priv->profile;
      templat.entrypoint = PIPE_VIDEO_ENTRYPOINT_BITSTREAM;
      templat.chroma_format = PIPE_VIDEO_CHROMA_FORMAT_420;
      templat.max_references = AV1_NUM_REF_FRAMES;
      templat.expect_chunked_decode = true;
      omx_base_video_PortType *port;
      port = (omx_base_video_PortType *)priv->ports[OMX_BASE_FILTER_INPUTPORT_INDEX];
      templat.width = port->sPortParam.format.video.nFrameWidth;
      templat.height = port->sPortParam.format.video.nFrameHeight;

      priv->codec = priv->pipe->create_video_codec(priv->pipe, &templat);
   }

   mtx_lock(&priv->codec_data.av1.mutex);
   dec_av1_MoveTask(&inp->tasks, &priv->codec_data.av1.finished_tasks);
   dec_av1_SortTask(priv);
   mtx_unlock(&priv->codec_data.av1.mutex);

   task = dec_av1_NeedTask(priv);
   if (!task)
      return NULL;

   priv->codec->begin_frame(priv->codec, task->buf, &priv->picture.base);
   priv->frame_started = true;

   return task;
}

static void dec_av1_EndFrame(vid_dec_PrivateType *priv, struct dec_av1_task *task)
{
   struct input_buf_private *inp = priv->in_buffers[0]->pInputPortPrivate;
   unsigned refresh_frame_flags;
   bool fnd;
   unsigned i;

   if (!priv->frame_started || ! task)
      return;

   priv->codec->end_frame(priv->codec, task->buf, &priv->picture.base);
   priv->frame_started = false;

   refresh_frame_flags = priv->codec_data.av1.uncompressed_header.refresh_frame_flags;
   for (i = 0; i < AV1_NUM_REF_FRAMES; ++i) {
      if (refresh_frame_flags & (1 << i)) {
         memcpy(&priv->codec_data.av1.refs[i], &priv->codec_data.av1.uncompressed_header,
            sizeof(struct av1_uncompressed_header_obu));
         priv->picture.av1.ref[i] = task->buf;
         priv->codec_data.av1.RefFrames[i].RefFrameType =
            priv->codec_data.av1.uncompressed_header.frame_type;
         priv->codec_data.av1.RefFrames[i].RefFrameId =
            priv->codec_data.av1.uncompressed_header.current_frame_id;
         priv->codec_data.av1.RefFrames[i].RefUpscaledWidth =
            priv->codec_data.av1.uncompressed_header.UpscaledWidth;
         priv->codec_data.av1.RefFrames[i].RefFrameWidth =
            priv->codec_data.av1.uncompressed_header.FrameWidth;
         priv->codec_data.av1.RefFrames[i].RefFrameHeight =
            priv->codec_data.av1.uncompressed_header.FrameHeight;
         priv->codec_data.av1.RefFrames[i].RefRenderWidth =
            priv->codec_data.av1.uncompressed_header.RenderWidth;
         priv->codec_data.av1.RefFrames[i].RefRenderHeight =
            priv->codec_data.av1.uncompressed_header.RenderHeight;
      }
   }
   if (!priv->picture.av1.picture_parameter.pic_info_fields.show_frame)
       task->no_show_frame = true;

   mtx_lock(&priv->codec_data.av1.mutex);
   fnd = dec_av1_GetStartedTask(priv, task, &priv->codec_data.av1.decode_tasks);
   if (!fnd) {
      mtx_unlock(&priv->codec_data.av1.mutex);
      return;
   }
   if (!priv->codec_data.av1.stacked_frame)
      dec_av1_MoveTask(&priv->codec_data.av1.decode_tasks, &inp->tasks);
   mtx_unlock(&priv->codec_data.av1.mutex);
   priv->frame_finished = 1;
}

static void dec_av1_Decode(vid_dec_PrivateType *priv, struct vl_vlc *vlc,
                           unsigned min_bits_left)
{
   unsigned start_bits_pos = vl_vlc_bits_left(vlc);
   unsigned start_bits = vl_vlc_valid_bits(vlc);
   unsigned start_bytes = start_bits / 8;
   const void *obu_data = vlc->data;
   uint8_t start_buf[8];
   unsigned num_buffers = 0;
   void * const * buffers[4];
   unsigned sizes[4];
   unsigned obu_size = 0;
   unsigned total_obu_len;
   enum av1_obu_type type;
   bool obu_extension_flag;
   bool obu_has_size_field;
   unsigned i;

   for (i = 0; i < start_bytes; ++i)
      start_buf[i] =
         vl_vlc_peekbits(vlc, start_bits) >> ((start_bytes - i - 1) * 8);

   /* obu header */
   av1_f(vlc, 1); /* obu_forbidden_bit */
   type = av1_f(vlc, 4);
   obu_extension_flag = av1_f(vlc, 1);
   obu_has_size_field = av1_f(vlc, 1);
   av1_f(vlc, 1); /* obu_reserved_1bit */
   if (obu_extension_flag) {
      priv->codec_data.av1.ext.temporal_id = av1_f(vlc, 3);
      priv->codec_data.av1.ext.spatial_id = av1_f(vlc, 2);
      av1_f(vlc, 3); /* extension_header_reserved_3bits */
   }

   obu_size = (obu_has_size_field) ? av1_uleb128(vlc) :
              (priv->sizes[0] - (unsigned)obu_extension_flag - 1);
   total_obu_len = (start_bits_pos - vl_vlc_bits_left(vlc)) / 8 + obu_size;

   switch (type) {
   case AV1_OBU_SEQUENCE_HEADER: {
      sequence_header_obu(priv, vlc);
      av1_byte_alignment(vlc);
      priv->codec_data.av1.bs_obu_seq_sz = total_obu_len;
      memcpy(priv->codec_data.av1.bs_obu_seq_buf, start_buf, start_bytes);
      memcpy(priv->codec_data.av1.bs_obu_seq_buf + start_bytes, obu_data,
             total_obu_len - start_bytes);
      break;
   }
   case AV1_OBU_TEMPORAL_DELIMITER:
      av1_byte_alignment(vlc);
      priv->codec_data.av1.bs_obu_td_sz = total_obu_len;
      memcpy(priv->codec_data.av1.bs_obu_td_buf, start_buf, total_obu_len);
      break;
   case AV1_OBU_FRAME_HEADER:
      frame_header_obu(priv, vlc);
      if (priv->codec_data.av1.uncompressed_header.show_existing_frame)
         dec_av1_ShowExistingframe(priv);
      av1_byte_alignment(vlc);
      break;
   case AV1_OBU_FRAME: {
      struct dec_av1_task *task;

      frame_header_obu(priv, vlc);
      av1_byte_alignment(vlc);

      parse_tile_hdr(priv, vlc, start_bits_pos, total_obu_len);
      av1_byte_alignment(vlc);

      task = dec_av1_BeginFrame(priv);
      if (!task)
         return;

      if (priv->codec_data.av1.bs_obu_td_sz) {
         buffers[num_buffers] = (void *)priv->codec_data.av1.bs_obu_td_buf;
         sizes[num_buffers++] = priv->codec_data.av1.bs_obu_td_sz;
         priv->codec_data.av1.bs_obu_td_sz = 0;
      }
      if (priv->codec_data.av1.bs_obu_seq_sz) {
         buffers[num_buffers] = (void *)priv->codec_data.av1.bs_obu_seq_buf;
         sizes[num_buffers++] = priv->codec_data.av1.bs_obu_seq_sz;
         priv->codec_data.av1.bs_obu_seq_sz = 0;
      }
      buffers[num_buffers] = (void *)start_buf;
      sizes[num_buffers++] = start_bytes;
      buffers[num_buffers] = (void *)obu_data;
      sizes[num_buffers++] = total_obu_len - start_bytes;

      priv->codec->decode_bitstream(priv->codec, priv->target,
         &priv->picture.base, num_buffers, (const void * const*)buffers, sizes);

      priv->codec_data.av1.stacked_frame =
            (vl_vlc_bits_left(vlc) > min_bits_left) ? true : false;

      dec_av1_EndFrame(priv, task);
      break;
   }
   default:
      av1_byte_alignment(vlc);
      break;
   }

   return;
}

OMX_ERRORTYPE vid_dec_av1_AllocateInBuffer(omx_base_PortType *port,
             OMX_INOUT OMX_BUFFERHEADERTYPE **buf, OMX_IN OMX_U32 idx,
             OMX_IN OMX_PTR private, OMX_IN OMX_U32 size)
{
   struct input_buf_private *inp;
   OMX_ERRORTYPE r;

   r = base_port_AllocateBuffer(port, buf, idx, private, size);
   if (r)
      return r;

   inp = (*buf)->pInputPortPrivate = CALLOC_STRUCT(input_buf_private);
   if (!inp) {
      base_port_FreeBuffer(port, idx, *buf);
      return OMX_ErrorInsufficientResources;
   }

   list_inithead(&inp->tasks);

   return OMX_ErrorNone;
}

OMX_ERRORTYPE vid_dec_av1_UseInBuffer(omx_base_PortType *port,
             OMX_BUFFERHEADERTYPE **buf, OMX_U32 idx,
             OMX_PTR private, OMX_U32 size, OMX_U8 *mem)
{
   struct input_buf_private *inp;
   OMX_ERRORTYPE r;

   r = base_port_UseBuffer(port, buf, idx, private, size, mem);
   if (r)
      return r;

   inp = (*buf)->pInputPortPrivate = CALLOC_STRUCT(input_buf_private);
   if (!inp) {
      base_port_FreeBuffer(port, idx, *buf);
      return OMX_ErrorInsufficientResources;
   }

   list_inithead(&inp->tasks);

   return OMX_ErrorNone;
}

void vid_dec_av1_FreeInputPortPrivate(vid_dec_PrivateType *priv,
                                      OMX_BUFFERHEADERTYPE *buf)
{
   struct input_buf_private *inp = buf->pInputPortPrivate;

   if (!inp || !inp->tasks.next)
      return;

   list_for_each_entry_safe(struct dec_av1_task, task, &inp->tasks, list) {
      task->buf->destroy(task->buf);
      FREE(task);
   }
}

void vid_dec_av1_ReleaseTasks(vid_dec_PrivateType *priv)
{
   dec_av1_ReleaseTask(priv, &priv->codec_data.av1.free_tasks);
   dec_av1_ReleaseTask(priv, &priv->codec_data.av1.started_tasks);
   dec_av1_ReleaseTask(priv, &priv->codec_data.av1.decode_tasks);
   dec_av1_ReleaseTask(priv, &priv->codec_data.av1.finished_tasks);
   mtx_destroy(&priv->codec_data.av1.mutex);
}

void vid_dec_av1_FrameDecoded(OMX_COMPONENTTYPE *comp,
                              OMX_BUFFERHEADERTYPE* input,
                              OMX_BUFFERHEADERTYPE* output)
{
   vid_dec_PrivateType *priv = comp->pComponentPrivate;
   bool eos = !!(input->nFlags & OMX_BUFFERFLAG_EOS);
   struct input_buf_private *inp = input->pInputPortPrivate;
   struct dec_av1_task *task;
   bool stacked = false;

   mtx_lock(&priv->codec_data.av1.mutex);
   if (list_length(&inp->tasks) > 1)
      stacked = true;

   if (list_is_empty(&inp->tasks)) {
      task = list_entry(priv->codec_data.av1.started_tasks.next,
                        struct dec_av1_task, list);
      list_del(&task->list);
      list_addtail(&task->list, &inp->tasks);
      --priv->codec_data.av1.que_num;
   }

   task = list_entry(inp->tasks.next, struct dec_av1_task, list);

   if (!task->no_show_frame) {
      vid_dec_FillOutput(priv, task->buf, output);
      output->nFilledLen = output->nAllocLen;
      output->nTimeStamp = input->nTimeStamp;
   } else {
      task->no_show_frame = false;
      output->nFilledLen = 0;
   }

   if (task->is_sef_task) {
      if (task->buf_ref_count == 0) {
         struct dec_av1_task *t = container_of(task->buf_ref, struct dec_av1_task, buf);
         list_del(&task->list);
         t->buf_ref_count--;
         list_del(&t->list);
         list_addtail(&t->list, &priv->codec_data.av1.finished_tasks);
      } else if (task->buf_ref_count == 1) {
         list_del(&task->list);
         task->buf->destroy(task->buf);
         task->buf_ref_count--;
      }
      FREE(task);
   } else {
      if (task->buf_ref_count == 1) {
         list_del(&task->list);
         list_addtail(&task->list, &priv->codec_data.av1.finished_tasks);
         task->buf_ref_count--;
      } else if (task->buf_ref_count == 2) {
         list_del(&task->list);
         task->buf_ref_count--;
         list_addtail(&task->list, &priv->codec_data.av1.finished_tasks);
      }
   }

   if (eos && input->pInputPortPrivate) {
      if (!priv->codec_data.av1.que_num)
         input->nFilledLen = 0;
      else
         vid_dec_av1_FreeInputPortPrivate(priv, input);
   }
   else {
      if (!stacked)
         input->nFilledLen = 0;
   }
   mtx_unlock(&priv->codec_data.av1.mutex);
}

void vid_dec_av1_Init(vid_dec_PrivateType *priv)
{
   priv->picture.base.profile = PIPE_VIDEO_PROFILE_AV1_MAIN;
   priv->Decode = dec_av1_Decode;
   list_inithead(&priv->codec_data.av1.free_tasks);
   list_inithead(&priv->codec_data.av1.started_tasks);
   list_inithead(&priv->codec_data.av1.decode_tasks);
   list_inithead(&priv->codec_data.av1.finished_tasks);
   (void)mtx_init(&priv->codec_data.av1.mutex, mtx_plain);
}
