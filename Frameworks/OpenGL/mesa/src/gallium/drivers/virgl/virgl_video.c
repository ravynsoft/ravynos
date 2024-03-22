/*
 * Copyright 2022 Kylin Software Co., Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * @file
 * Virgl video driver implementation.
 *
 * The virgl video driver acts as the frontend, and the virglrenderer acts as
 * the backend. Currently, the backend is implemented via VA-API, but it is
 * not limited to this.
 *
 * The relationship between vaSurface and video buffer objects:
 *
 *           GUEST (Mesa)           |       HOST (Virglrenderer)
 *                                  |
 *         +------------+           |          +------------+
 *         | vaSurface  |           |          | vaSurface  | <------+
 *         +------------+           |          +------------+        |
 *               |                  |                                |
 *  +---------------------------+   |   +-------------------------+  |
 *  |    virgl_video_buffer     |   |   |    vrend_video_buffer   |  |
 *  | +-----------------------+ |   |   |  +-------------------+  |  |
 *  | |    vl_video_buffer    | |   |   |  | vrend_resource(s) |  |  |
 *  | | +-------------------+ | |<--+-->|  +-------------------+  |  |
 *  | | | virgl_resource(s) | | |   |   |  +--------------------+ |  |
 *  | | +-------------------+ | |   |   |  | virgl_video_buffer |-+--+
 *  | +-----------------------+ |   |   |  +--------------------+ |
 *  +---------------------------+   |   +-------------------------+
 *
 * The relationship between vaContext and video codec objects:
 *
 *           GUEST (Mesa)         |         HOST (Virglrenderer)
 *                                |
 *         +------------+         |           +------------+
 *         | vaContext  |         |           | vaContext  | <-------+
 *         +------------+         |           +------------+         |
 *               |                |                                  |
 *  +------------------------+    |    +--------------------------+  |
 *  |    virgl_video_codec   | <--+--> |    vrend_video_codec     |  |
 *  +------------------------+    |    |  +--------------------+  |  |
 *                                |    |  | virgl_video_codec  | -+--+
 *                                |    |  +--------------------+  |
 *                                |    +--------------------------+
 *
 * @author Feng Jiang <jiangfeng@kylinos.cn>
 */

#include <string.h>
#include <sys/param.h>

#include "vl/vl_decoder.h"
#include "vl/vl_video_buffer.h"
#include "util/u_video.h"
#include "util/u_memory.h"

#include "virgl_screen.h"
#include "virgl_resource.h"
#include "virgl_encode.h"
#include "virgl_video.h"
#include "virtio-gpu/virgl_video_hw.h"

/*
 * The max size of bs buffer is approximately:
 *   num_of_macroblocks * max_size_of_per_macroblock + size_of_some_headers
 * Now, we only support YUV420 formats, this means that we have a limit of
 * 3200 bits(400 Bytes) per macroblock. To simplify the calculation, we
 * directly use 512 instead of 400.
 */
#define BS_BUF_DEFAULT_SIZE(width, height) \
    ((width) * (height) / (VL_MACROBLOCK_WIDTH * VL_MACROBLOCK_HEIGHT) * 512)

static void switch_buffer(struct virgl_video_codec *vcdc)
{
    vcdc->cur_buffer++;
    vcdc->cur_buffer %= VIRGL_VIDEO_CODEC_BUF_NUM;
}

#define ITEM_SET(dest, src, item)   (dest)->item = (src)->item
#define ITEM_CPY(dest, src, item)   memcpy(&(dest)->item, &(src)->item, sizeof((dest)->item))

static int fill_base_picture_desc(const struct pipe_picture_desc *desc,
                                  struct virgl_base_picture_desc *vbase)
{
    ITEM_SET(vbase, desc, profile);
    ITEM_SET(vbase, desc, entry_point);
    ITEM_SET(vbase, desc, protected_playback);
    ITEM_SET(vbase, desc, key_size);
    memcpy(vbase->decrypt_key, desc->decrypt_key,
           MIN(desc->key_size, sizeof(vbase->decrypt_key)));

    return 0;
}

static int fill_h264_picture_desc(const struct pipe_picture_desc *desc,
                                  union virgl_picture_desc *vdsc)
{
    unsigned i;
    struct virgl_video_buffer *vbuf;

    struct virgl_h264_picture_desc *vh264 = &vdsc->h264;
    struct virgl_h264_pps *vpps = &vh264->pps;
    struct virgl_h264_sps *vsps = &vh264->pps.sps;

    struct pipe_h264_picture_desc *h264 = (struct pipe_h264_picture_desc *)desc;
    struct pipe_h264_pps *pps = h264->pps;
    struct pipe_h264_sps *sps = h264->pps->sps;

    fill_base_picture_desc(desc, &vh264->base);

    ITEM_SET(vsps, sps, level_idc);
    ITEM_SET(vsps, sps, chroma_format_idc);
    ITEM_SET(vsps, sps, separate_colour_plane_flag);
    ITEM_SET(vsps, sps, bit_depth_luma_minus8);
    ITEM_SET(vsps, sps, bit_depth_chroma_minus8);
    ITEM_SET(vsps, sps, seq_scaling_matrix_present_flag);
    ITEM_CPY(vsps, sps, ScalingList4x4);
    ITEM_CPY(vsps, sps, ScalingList8x8);
    ITEM_SET(vsps, sps, log2_max_frame_num_minus4);
    ITEM_SET(vsps, sps, pic_order_cnt_type);
    ITEM_SET(vsps, sps, log2_max_pic_order_cnt_lsb_minus4);
    ITEM_SET(vsps, sps, delta_pic_order_always_zero_flag);
    ITEM_SET(vsps, sps, offset_for_non_ref_pic);
    ITEM_SET(vsps, sps, offset_for_top_to_bottom_field);
    ITEM_CPY(vsps, sps, offset_for_ref_frame);
    ITEM_SET(vsps, sps, num_ref_frames_in_pic_order_cnt_cycle);
    ITEM_SET(vsps, sps, max_num_ref_frames);
    ITEM_SET(vsps, sps, frame_mbs_only_flag);
    ITEM_SET(vsps, sps, mb_adaptive_frame_field_flag);
    ITEM_SET(vsps, sps, direct_8x8_inference_flag);
    ITEM_SET(vsps, sps, MinLumaBiPredSize8x8);

    ITEM_SET(vpps, pps, entropy_coding_mode_flag);
    ITEM_SET(vpps, pps, bottom_field_pic_order_in_frame_present_flag);
    ITEM_SET(vpps, pps, num_slice_groups_minus1);
    ITEM_SET(vpps, pps, slice_group_map_type);
    ITEM_SET(vpps, pps, slice_group_change_rate_minus1);
    ITEM_SET(vpps, pps, num_ref_idx_l0_default_active_minus1);
    ITEM_SET(vpps, pps, num_ref_idx_l1_default_active_minus1);
    ITEM_SET(vpps, pps, weighted_pred_flag);
    ITEM_SET(vpps, pps, weighted_bipred_idc);
    ITEM_SET(vpps, pps, pic_init_qp_minus26);
    ITEM_SET(vpps, pps, pic_init_qs_minus26);
    ITEM_SET(vpps, pps, chroma_qp_index_offset);
    ITEM_SET(vpps, pps, deblocking_filter_control_present_flag);
    ITEM_SET(vpps, pps, constrained_intra_pred_flag);
    ITEM_SET(vpps, pps, redundant_pic_cnt_present_flag);
    ITEM_CPY(vpps, pps, ScalingList4x4);
    ITEM_CPY(vpps, pps, ScalingList8x8);
    ITEM_SET(vpps, pps, transform_8x8_mode_flag);
    ITEM_SET(vpps, pps, second_chroma_qp_index_offset);

    ITEM_SET(vh264, h264, frame_num);
    ITEM_SET(vh264, h264, field_pic_flag);
    ITEM_SET(vh264, h264, bottom_field_flag);
    ITEM_SET(vh264, h264, num_ref_idx_l0_active_minus1);
    ITEM_SET(vh264, h264, num_ref_idx_l1_active_minus1);
    ITEM_SET(vh264, h264, slice_count);
    ITEM_CPY(vh264, h264, field_order_cnt);
    ITEM_SET(vh264, h264, is_reference);
    ITEM_SET(vh264, h264, num_ref_frames);
    ITEM_CPY(vh264, h264, field_order_cnt_list);
    ITEM_CPY(vh264, h264, frame_num_list);

    for (i = 0; i < 16; i++) {
        ITEM_SET(vh264, h264, is_long_term[i]);
        ITEM_SET(vh264, h264, top_is_reference[i]);
        ITEM_SET(vh264, h264, bottom_is_reference[i]);

        vbuf = virgl_video_buffer(h264->ref[i]);
        vh264->buffer_id[i] = vbuf ? vbuf->handle : 0;
    }

    return 0;
}

static int fill_h264_enc_picture_desc(const struct pipe_picture_desc *desc,
                                      union virgl_picture_desc *vdsc)
{
    unsigned i;
    struct virgl_h264_enc_picture_desc *vh264 = &vdsc->h264_enc;
    struct pipe_h264_enc_picture_desc *h264 = (struct pipe_h264_enc_picture_desc *)desc;

    fill_base_picture_desc(desc, &vh264->base);

    /* seq param */
    ITEM_SET(vh264, h264, seq.enc_constraint_set_flags);
    ITEM_SET(vh264, h264, seq.enc_frame_cropping_flag);
    ITEM_SET(vh264, h264, seq.enc_frame_crop_left_offset);
    ITEM_SET(vh264, h264, seq.enc_frame_crop_right_offset);
    ITEM_SET(vh264, h264, seq.enc_frame_crop_top_offset);
    ITEM_SET(vh264, h264, seq.enc_frame_crop_bottom_offset);
    ITEM_SET(vh264, h264, seq.pic_order_cnt_type);
    ITEM_SET(vh264, h264, seq.num_temporal_layers);
    ITEM_SET(vh264, h264, seq.vui_parameters_present_flag);
    ITEM_SET(vh264, h264, seq.vui_flags.aspect_ratio_info_present_flag);
    ITEM_SET(vh264, h264, seq.vui_flags.timing_info_present_flag);
    ITEM_SET(vh264, h264, seq.aspect_ratio_idc);
    ITEM_SET(vh264, h264, seq.sar_width);
    ITEM_SET(vh264, h264, seq.sar_height);
    ITEM_SET(vh264, h264, seq.num_units_in_tick);
    ITEM_SET(vh264, h264, seq.time_scale);

    /* rate_ctrl */
    for (i = 0; i < 4; i++) {
        ITEM_SET(vh264, h264, rate_ctrl[i].rate_ctrl_method);
        ITEM_SET(vh264, h264, rate_ctrl[i].target_bitrate);
        ITEM_SET(vh264, h264, rate_ctrl[i].peak_bitrate);
        ITEM_SET(vh264, h264, rate_ctrl[i].frame_rate_num);
        ITEM_SET(vh264, h264, rate_ctrl[i].frame_rate_den);
        ITEM_SET(vh264, h264, rate_ctrl[i].vbv_buffer_size);
        ITEM_SET(vh264, h264, rate_ctrl[i].vbv_buf_lv);
        ITEM_SET(vh264, h264, rate_ctrl[i].target_bits_picture);
        ITEM_SET(vh264, h264, rate_ctrl[i].peak_bits_picture_integer);
        ITEM_SET(vh264, h264, rate_ctrl[i].peak_bits_picture_fraction);
        ITEM_SET(vh264, h264, rate_ctrl[i].fill_data_enable);
        ITEM_SET(vh264, h264, rate_ctrl[i].skip_frame_enable);
        ITEM_SET(vh264, h264, rate_ctrl[i].enforce_hrd);
        ITEM_SET(vh264, h264, rate_ctrl[i].max_au_size);
        ITEM_SET(vh264, h264, rate_ctrl[i].max_qp);
        ITEM_SET(vh264, h264, rate_ctrl[i].min_qp);
    }

    /* motion_est */
    ITEM_SET(vh264, h264, motion_est.motion_est_quarter_pixel);
    ITEM_SET(vh264, h264, motion_est.enc_disable_sub_mode);
    ITEM_SET(vh264, h264, motion_est.lsmvert);
    ITEM_SET(vh264, h264, motion_est.enc_en_ime_overw_dis_subm);
    ITEM_SET(vh264, h264, motion_est.enc_ime_overw_dis_subm_no);
    ITEM_SET(vh264, h264, motion_est.enc_ime2_search_range_x);
    ITEM_SET(vh264, h264, motion_est.enc_ime2_search_range_y);

    /* pic_ctrl */
    ITEM_SET(vh264, h264, pic_ctrl.enc_cabac_enable);
    ITEM_SET(vh264, h264, pic_ctrl.enc_cabac_init_idc);

    ITEM_SET(vh264, h264, intra_idr_period);

    ITEM_SET(vh264, h264, quant_i_frames);
    ITEM_SET(vh264, h264, quant_p_frames);
    ITEM_SET(vh264, h264, quant_b_frames);

    ITEM_SET(vh264, h264, picture_type);
    ITEM_SET(vh264, h264, frame_num);
    ITEM_SET(vh264, h264, frame_num_cnt);
    ITEM_SET(vh264, h264, p_remain);
    ITEM_SET(vh264, h264, i_remain);
    ITEM_SET(vh264, h264, idr_pic_id);
    ITEM_SET(vh264, h264, gop_cnt);
    ITEM_SET(vh264, h264, pic_order_cnt);
    ITEM_SET(vh264, h264, num_ref_idx_l0_active_minus1);
    ITEM_SET(vh264, h264, num_ref_idx_l1_active_minus1);

    for (i = 0; i < 32; i++) {
        ITEM_SET(vh264, h264, ref_idx_l0_list[i]);
        ITEM_SET(vh264, h264, ref_idx_l1_list[i]);
        ITEM_SET(vh264, h264, l0_is_long_term[i]);
        ITEM_SET(vh264, h264, l1_is_long_term[i]);
    }

    ITEM_SET(vh264, h264, gop_size);

    ITEM_SET(vh264, h264, quality_modes.level);
    ITEM_SET(vh264, h264, quality_modes.preset_mode);
    ITEM_SET(vh264, h264, quality_modes.pre_encode_mode);
    ITEM_SET(vh264, h264, quality_modes.vbaq_mode);

    ITEM_SET(vh264, h264, not_referenced);
    ITEM_SET(vh264, h264, is_ltr);
    ITEM_SET(vh264, h264, ltr_index);
    ITEM_SET(vh264, h264, enable_vui);

    ITEM_SET(vh264, h264, num_slice_descriptors);
    for (i = 0; i < vh264->num_slice_descriptors; i++) {
        ITEM_SET(vh264, h264, slices_descriptors[i].macroblock_address);
        ITEM_SET(vh264, h264, slices_descriptors[i].num_macroblocks);
        ITEM_SET(vh264, h264, slices_descriptors[i].slice_type);
    }

    return 0;
}

static int fill_h265_picture_desc(const struct pipe_picture_desc *desc,
                                  union virgl_picture_desc *vdsc)
{
    unsigned i;
    struct virgl_video_buffer *vbuf;

    struct virgl_h265_picture_desc *vh265 = &vdsc->h265;
    struct pipe_h265_picture_desc *h265 = (struct pipe_h265_picture_desc *)desc;

    fill_base_picture_desc(desc, &vh265->base);

    ITEM_SET(&vh265->pps.sps, h265->pps->sps, chroma_format_idc);
    ITEM_SET(&vh265->pps.sps, h265->pps->sps, separate_colour_plane_flag);
    ITEM_SET(&vh265->pps.sps, h265->pps->sps, pic_width_in_luma_samples);
    ITEM_SET(&vh265->pps.sps, h265->pps->sps, pic_height_in_luma_samples);
    ITEM_SET(&vh265->pps.sps, h265->pps->sps, bit_depth_luma_minus8);
    ITEM_SET(&vh265->pps.sps, h265->pps->sps, bit_depth_chroma_minus8);
    ITEM_SET(&vh265->pps.sps, h265->pps->sps, log2_max_pic_order_cnt_lsb_minus4);
    ITEM_SET(&vh265->pps.sps, h265->pps->sps, sps_max_dec_pic_buffering_minus1);
    ITEM_SET(&vh265->pps.sps, h265->pps->sps, log2_min_luma_coding_block_size_minus3);
    ITEM_SET(&vh265->pps.sps, h265->pps->sps, log2_diff_max_min_luma_coding_block_size);
    ITEM_SET(&vh265->pps.sps, h265->pps->sps, log2_min_transform_block_size_minus2);
    ITEM_SET(&vh265->pps.sps, h265->pps->sps, log2_diff_max_min_transform_block_size);
    ITEM_SET(&vh265->pps.sps, h265->pps->sps, max_transform_hierarchy_depth_inter);
    ITEM_SET(&vh265->pps.sps, h265->pps->sps, max_transform_hierarchy_depth_intra);
    ITEM_SET(&vh265->pps.sps, h265->pps->sps, scaling_list_enabled_flag);
    ITEM_CPY(&vh265->pps.sps, h265->pps->sps, ScalingList4x4);
    ITEM_CPY(&vh265->pps.sps, h265->pps->sps, ScalingList8x8);
    ITEM_CPY(&vh265->pps.sps, h265->pps->sps, ScalingList16x16);
    ITEM_CPY(&vh265->pps.sps, h265->pps->sps, ScalingList32x32);
    ITEM_CPY(&vh265->pps.sps, h265->pps->sps, ScalingListDCCoeff16x16);
    ITEM_CPY(&vh265->pps.sps, h265->pps->sps, ScalingListDCCoeff32x32);
    ITEM_SET(&vh265->pps.sps, h265->pps->sps, amp_enabled_flag);
    ITEM_SET(&vh265->pps.sps, h265->pps->sps, sample_adaptive_offset_enabled_flag);
    ITEM_SET(&vh265->pps.sps, h265->pps->sps, pcm_enabled_flag);
    ITEM_SET(&vh265->pps.sps, h265->pps->sps, pcm_sample_bit_depth_luma_minus1);
    ITEM_SET(&vh265->pps.sps, h265->pps->sps, pcm_sample_bit_depth_chroma_minus1);
    ITEM_SET(&vh265->pps.sps, h265->pps->sps, log2_min_pcm_luma_coding_block_size_minus3);
    ITEM_SET(&vh265->pps.sps, h265->pps->sps, log2_diff_max_min_pcm_luma_coding_block_size);
    ITEM_SET(&vh265->pps.sps, h265->pps->sps, pcm_loop_filter_disabled_flag);
    ITEM_SET(&vh265->pps.sps, h265->pps->sps, num_short_term_ref_pic_sets);
    ITEM_SET(&vh265->pps.sps, h265->pps->sps, long_term_ref_pics_present_flag);
    ITEM_SET(&vh265->pps.sps, h265->pps->sps, num_long_term_ref_pics_sps);
    ITEM_SET(&vh265->pps.sps, h265->pps->sps, sps_temporal_mvp_enabled_flag);
    ITEM_SET(&vh265->pps.sps, h265->pps->sps, strong_intra_smoothing_enabled_flag);

    ITEM_SET(&vh265->pps, h265->pps, dependent_slice_segments_enabled_flag);
    ITEM_SET(&vh265->pps, h265->pps, output_flag_present_flag);
    ITEM_SET(&vh265->pps, h265->pps, num_extra_slice_header_bits);
    ITEM_SET(&vh265->pps, h265->pps, sign_data_hiding_enabled_flag);
    ITEM_SET(&vh265->pps, h265->pps, cabac_init_present_flag);
    ITEM_SET(&vh265->pps, h265->pps, num_ref_idx_l0_default_active_minus1);
    ITEM_SET(&vh265->pps, h265->pps, num_ref_idx_l1_default_active_minus1);
    ITEM_SET(&vh265->pps, h265->pps, init_qp_minus26);
    ITEM_SET(&vh265->pps, h265->pps, constrained_intra_pred_flag);
    ITEM_SET(&vh265->pps, h265->pps, transform_skip_enabled_flag);
    ITEM_SET(&vh265->pps, h265->pps, cu_qp_delta_enabled_flag);
    ITEM_SET(&vh265->pps, h265->pps, diff_cu_qp_delta_depth);
    ITEM_SET(&vh265->pps, h265->pps, pps_cb_qp_offset);
    ITEM_SET(&vh265->pps, h265->pps, pps_cr_qp_offset);
    ITEM_SET(&vh265->pps, h265->pps, pps_slice_chroma_qp_offsets_present_flag);
    ITEM_SET(&vh265->pps, h265->pps, weighted_pred_flag);
    ITEM_SET(&vh265->pps, h265->pps, weighted_bipred_flag);
    ITEM_SET(&vh265->pps, h265->pps, transquant_bypass_enabled_flag);
    ITEM_SET(&vh265->pps, h265->pps, tiles_enabled_flag);
    ITEM_SET(&vh265->pps, h265->pps, entropy_coding_sync_enabled_flag);
    ITEM_SET(&vh265->pps, h265->pps, num_tile_columns_minus1);
    ITEM_SET(&vh265->pps, h265->pps, num_tile_rows_minus1);
    ITEM_SET(&vh265->pps, h265->pps, uniform_spacing_flag);
    ITEM_CPY(&vh265->pps, h265->pps, column_width_minus1);
    ITEM_CPY(&vh265->pps, h265->pps, row_height_minus1);
    ITEM_SET(&vh265->pps, h265->pps, loop_filter_across_tiles_enabled_flag);
    ITEM_SET(&vh265->pps, h265->pps, pps_loop_filter_across_slices_enabled_flag);
    ITEM_SET(&vh265->pps, h265->pps, deblocking_filter_control_present_flag);
    ITEM_SET(&vh265->pps, h265->pps, deblocking_filter_override_enabled_flag);
    ITEM_SET(&vh265->pps, h265->pps, pps_deblocking_filter_disabled_flag);
    ITEM_SET(&vh265->pps, h265->pps, pps_beta_offset_div2);
    ITEM_SET(&vh265->pps, h265->pps, pps_tc_offset_div2);
    ITEM_SET(&vh265->pps, h265->pps, lists_modification_present_flag);
    ITEM_SET(&vh265->pps, h265->pps, log2_parallel_merge_level_minus2);
    ITEM_SET(&vh265->pps, h265->pps, slice_segment_header_extension_present_flag);
    ITEM_SET(&vh265->pps, h265->pps, st_rps_bits);

    ITEM_SET(vh265, h265, IDRPicFlag);
    ITEM_SET(vh265, h265, RAPPicFlag);
    ITEM_SET(vh265, h265, CurrRpsIdx);
    ITEM_SET(vh265, h265, NumPocTotalCurr);
    ITEM_SET(vh265, h265, NumDeltaPocsOfRefRpsIdx);
    ITEM_SET(vh265, h265, NumShortTermPictureSliceHeaderBits);
    ITEM_SET(vh265, h265, NumLongTermPictureSliceHeaderBits);

    ITEM_SET(vh265, h265, CurrPicOrderCntVal);
    for (i = 0; i < 16; i++) {
        vbuf = virgl_video_buffer(h265->ref[i]);
        vh265->ref[i] = vbuf ? vbuf->handle : 0;
    }
    ITEM_CPY(vh265, h265, PicOrderCntVal);
    ITEM_CPY(vh265, h265, IsLongTerm);
    ITEM_SET(vh265, h265, NumPocStCurrBefore);
    ITEM_SET(vh265, h265, NumPocStCurrAfter);
    ITEM_SET(vh265, h265, NumPocLtCurr);
    ITEM_CPY(vh265, h265, RefPicSetStCurrBefore);
    ITEM_CPY(vh265, h265, RefPicSetStCurrAfter);
    ITEM_CPY(vh265, h265, RefPicSetLtCurr);
    ITEM_CPY(vh265, h265, RefPicList);
    ITEM_SET(vh265, h265, UseRefPicList);
    ITEM_SET(vh265, h265, UseStRpsBits);

    return 0;
}

static int fill_h265_enc_picture_desc(const struct pipe_picture_desc *desc,
                                      union virgl_picture_desc *vdsc)
{
    unsigned i;
    struct virgl_h265_enc_picture_desc *vh265 = &vdsc->h265_enc;
    struct pipe_h265_enc_picture_desc *h265 = (struct pipe_h265_enc_picture_desc *)desc;

    fill_base_picture_desc(desc, &vh265->base);

    ITEM_SET(vh265, h265, seq.general_profile_idc);
    ITEM_SET(vh265, h265, seq.general_level_idc);
    ITEM_SET(vh265, h265, seq.general_tier_flag);
    ITEM_SET(vh265, h265, seq.intra_period);
    ITEM_SET(vh265, h265, seq.ip_period);
    ITEM_SET(vh265, h265, seq.pic_width_in_luma_samples);
    ITEM_SET(vh265, h265, seq.pic_height_in_luma_samples);
    ITEM_SET(vh265, h265, seq.chroma_format_idc);
    ITEM_SET(vh265, h265, seq.bit_depth_luma_minus8);
    ITEM_SET(vh265, h265, seq.bit_depth_chroma_minus8);
    ITEM_SET(vh265, h265, seq.strong_intra_smoothing_enabled_flag);
    ITEM_SET(vh265, h265, seq.amp_enabled_flag);
    ITEM_SET(vh265, h265, seq.sample_adaptive_offset_enabled_flag);
    ITEM_SET(vh265, h265, seq.pcm_enabled_flag);
    ITEM_SET(vh265, h265, seq.sps_temporal_mvp_enabled_flag);
    ITEM_SET(vh265, h265, seq.log2_min_luma_coding_block_size_minus3);
    ITEM_SET(vh265, h265, seq.log2_diff_max_min_luma_coding_block_size);
    ITEM_SET(vh265, h265, seq.log2_min_transform_block_size_minus2);
    ITEM_SET(vh265, h265, seq.log2_diff_max_min_transform_block_size);
    ITEM_SET(vh265, h265, seq.max_transform_hierarchy_depth_inter);
    ITEM_SET(vh265, h265, seq.max_transform_hierarchy_depth_intra);
    ITEM_SET(vh265, h265, seq.conformance_window_flag);
    ITEM_SET(vh265, h265, seq.conf_win_left_offset);
    ITEM_SET(vh265, h265, seq.conf_win_right_offset);
    ITEM_SET(vh265, h265, seq.conf_win_top_offset);
    ITEM_SET(vh265, h265, seq.conf_win_bottom_offset);
    ITEM_SET(vh265, h265, seq.vui_parameters_present_flag);
    ITEM_SET(vh265, h265, seq.vui_flags.aspect_ratio_info_present_flag);
    ITEM_SET(vh265, h265, seq.vui_flags.timing_info_present_flag);
    ITEM_SET(vh265, h265, seq.aspect_ratio_idc);
    ITEM_SET(vh265, h265, seq.sar_width);
    ITEM_SET(vh265, h265, seq.sar_height);
    ITEM_SET(vh265, h265, seq.num_units_in_tick);
    ITEM_SET(vh265, h265, seq.time_scale);

    ITEM_SET(vh265, h265, pic.log2_parallel_merge_level_minus2);
    ITEM_SET(vh265, h265, pic.nal_unit_type);
    ITEM_SET(vh265, h265, pic.constrained_intra_pred_flag);
    ITEM_SET(vh265, h265, pic.pps_loop_filter_across_slices_enabled_flag);
    ITEM_SET(vh265, h265, pic.transform_skip_enabled_flag);

    ITEM_SET(vh265, h265, slice.max_num_merge_cand);
    ITEM_SET(vh265, h265, slice.slice_cb_qp_offset);
    ITEM_SET(vh265, h265, slice.slice_cr_qp_offset);
    ITEM_SET(vh265, h265, slice.slice_beta_offset_div2);
    ITEM_SET(vh265, h265, slice.slice_tc_offset_div2);
    ITEM_SET(vh265, h265, slice.cabac_init_flag);
    ITEM_SET(vh265, h265, slice.slice_deblocking_filter_disabled_flag);
    ITEM_SET(vh265, h265, slice.slice_loop_filter_across_slices_enabled_flag);

    ITEM_SET(vh265, h265, rc.rate_ctrl_method);
    ITEM_SET(vh265, h265, rc.target_bitrate);
    ITEM_SET(vh265, h265, rc.peak_bitrate);
    ITEM_SET(vh265, h265, rc.frame_rate_num);
    ITEM_SET(vh265, h265, rc.frame_rate_den);
    ITEM_SET(vh265, h265, rc.quant_i_frames);
    ITEM_SET(vh265, h265, rc.quant_p_frames);
    ITEM_SET(vh265, h265, rc.quant_b_frames);
    ITEM_SET(vh265, h265, rc.vbv_buffer_size);
    ITEM_SET(vh265, h265, rc.vbv_buf_lv);
    ITEM_SET(vh265, h265, rc.target_bits_picture);
    ITEM_SET(vh265, h265, rc.peak_bits_picture_integer);
    ITEM_SET(vh265, h265, rc.peak_bits_picture_fraction);
    ITEM_SET(vh265, h265, rc.fill_data_enable);
    ITEM_SET(vh265, h265, rc.skip_frame_enable);
    ITEM_SET(vh265, h265, rc.enforce_hrd);
    ITEM_SET(vh265, h265, rc.max_au_size);
    ITEM_SET(vh265, h265, rc.max_qp);
    ITEM_SET(vh265, h265, rc.min_qp);

    ITEM_SET(vh265, h265, picture_type);
    ITEM_SET(vh265, h265, decoded_curr_pic);

    for (i = 0; i < 16; i++) {
        ITEM_SET(vh265, h265, reference_frames[i]);
    }

    ITEM_SET(vh265, h265, frame_num);
    ITEM_SET(vh265, h265, pic_order_cnt);
    ITEM_SET(vh265, h265, pic_order_cnt_type);

    ITEM_SET(vh265, h265, quality_modes.level);
    ITEM_SET(vh265, h265, quality_modes.preset_mode);
    ITEM_SET(vh265, h265, quality_modes.pre_encode_mode);
    ITEM_SET(vh265, h265, quality_modes.vbaq_mode);

    ITEM_SET(vh265, h265, num_ref_idx_l0_active_minus1);
    ITEM_SET(vh265, h265, num_ref_idx_l1_active_minus1);

    for (i = 0; i < 15; i++) {
        ITEM_SET(vh265, h265, ref_idx_l0_list[i]);
        ITEM_SET(vh265, h265, ref_idx_l1_list[i]);
    }

    ITEM_SET(vh265, h265, not_referenced);

    ITEM_SET(vh265, h265, num_slice_descriptors);
    for (i = 0; i < vh265->num_slice_descriptors; i++) {
        ITEM_SET(vh265, h265, slices_descriptors[i].slice_segment_address);
        ITEM_SET(vh265, h265, slices_descriptors[i].num_ctu_in_slice);
        ITEM_SET(vh265, h265, slices_descriptors[i].slice_type);
    }

    return 0;
}

static int fill_mpeg4_picture_desc(const struct pipe_picture_desc *desc,
                                   union virgl_picture_desc *vdsc)
{
    unsigned i;
    struct virgl_video_buffer *vbuf;
    struct virgl_mpeg4_picture_desc *vmpeg4 = &vdsc->mpeg4;
    struct pipe_mpeg4_picture_desc *mpeg4 = (struct pipe_mpeg4_picture_desc *)desc;

    fill_base_picture_desc(desc, &vmpeg4->base);

    ITEM_CPY(vmpeg4, mpeg4, trd);
    ITEM_CPY(vmpeg4, mpeg4, trb);
    ITEM_SET(vmpeg4, mpeg4, vop_time_increment_resolution);
    ITEM_SET(vmpeg4, mpeg4, vop_coding_type);
    ITEM_SET(vmpeg4, mpeg4, vop_fcode_forward);
    ITEM_SET(vmpeg4, mpeg4, vop_fcode_backward);
    ITEM_SET(vmpeg4, mpeg4, resync_marker_disable);
    ITEM_SET(vmpeg4, mpeg4, interlaced);
    ITEM_SET(vmpeg4, mpeg4, quant_type);
    ITEM_SET(vmpeg4, mpeg4, quarter_sample);
    ITEM_SET(vmpeg4, mpeg4, short_video_header);
    ITEM_SET(vmpeg4, mpeg4, rounding_control);
    ITEM_SET(vmpeg4, mpeg4, alternate_vertical_scan_flag);
    ITEM_SET(vmpeg4, mpeg4, top_field_first);

    memcpy(vmpeg4->intra_matrix, mpeg4->intra_matrix, 64);
    memcpy(vmpeg4->non_intra_matrix, mpeg4->non_intra_matrix, 64);

    for (i = 0; i < ARRAY_SIZE(mpeg4->ref); i++) {
        vbuf = virgl_video_buffer(mpeg4->ref[i]);
        vmpeg4->ref[i] = vbuf ? vbuf->handle : 0;
    }

    return 0;
}

static int fill_mpeg12_picture_desc(const struct pipe_picture_desc *desc,
                                    union virgl_picture_desc *vdsc)
{
    unsigned i;
    struct virgl_video_buffer *vbuf;
    struct virgl_mpeg12_picture_desc *vmpeg12 = &vdsc->mpeg12;
    struct pipe_mpeg12_picture_desc *mpeg12 = (struct pipe_mpeg12_picture_desc *)desc;

    fill_base_picture_desc(desc, &vmpeg12->base);

    for (i = 0; i < 2; i++) {
        vbuf = virgl_video_buffer(mpeg12->ref[i]);
        vmpeg12->ref[i] = vbuf ? vbuf->handle : 0;
    }

    memcpy(vmpeg12->intra_matrix, mpeg12->intra_matrix, 64);
    memcpy(vmpeg12->non_intra_matrix, mpeg12->non_intra_matrix, 64);

    ITEM_SET(vmpeg12, mpeg12, picture_coding_type);
    vmpeg12->f_code[0][0]  = mpeg12->f_code[0][0] ;
    vmpeg12->f_code[0][1]  = mpeg12->f_code[0][1] ;
    vmpeg12->f_code[1][0]  = mpeg12->f_code[1][0] ;
    vmpeg12->f_code[1][1]  = mpeg12->f_code[1][1] ;
    ITEM_SET(vmpeg12, mpeg12, intra_dc_precision);
    ITEM_SET(vmpeg12, mpeg12, picture_structure);
    ITEM_SET(vmpeg12, mpeg12, top_field_first);
    ITEM_SET(vmpeg12, mpeg12, frame_pred_frame_dct);
    ITEM_SET(vmpeg12, mpeg12, concealment_motion_vectors);
    ITEM_SET(vmpeg12, mpeg12, q_scale_type);
    ITEM_SET(vmpeg12, mpeg12, intra_vlc_format);
    ITEM_SET(vmpeg12, mpeg12, alternate_scan);
    return 0;
}


static int fill_vc1_picture_desc(const struct pipe_picture_desc *desc,
                                 union virgl_picture_desc *vdsc)
{
    unsigned i;
    struct virgl_video_buffer *vbuf;
    struct virgl_vc1_picture_desc *vvc1 = &vdsc->vc1;
    struct pipe_vc1_picture_desc *vc1 = (struct pipe_vc1_picture_desc *)desc;

    fill_base_picture_desc(desc, &vvc1->base);

    for (i = 0; i < 2; i++) {
        vbuf = virgl_video_buffer(vc1->ref[i]);
        vvc1->ref[i] = vbuf ? vbuf->handle : 0;
    }
    ITEM_SET(vvc1, vc1, picture_type);
    ITEM_SET(vvc1, vc1, pulldown);
    ITEM_SET(vvc1, vc1, interlace);
    ITEM_SET(vvc1, vc1, tfcntrflag);
    ITEM_SET(vvc1, vc1, finterpflag);
    ITEM_SET(vvc1, vc1, psf);
    ITEM_SET(vvc1, vc1, dquant);
    ITEM_SET(vvc1, vc1, panscan_flag);
    ITEM_SET(vvc1, vc1, refdist_flag);
    ITEM_SET(vvc1, vc1, quantizer);
    ITEM_SET(vvc1, vc1, extended_mv);
    ITEM_SET(vvc1, vc1, extended_dmv);
    ITEM_SET(vvc1, vc1, overlap);
    ITEM_SET(vvc1, vc1, vstransform);
    ITEM_SET(vvc1, vc1, loopfilter);
    ITEM_SET(vvc1, vc1, fastuvmc);
    ITEM_SET(vvc1, vc1, range_mapy_flag);
    ITEM_SET(vvc1, vc1, range_mapy);
    ITEM_SET(vvc1, vc1, range_mapuv_flag);
    ITEM_SET(vvc1, vc1, range_mapuv);
    ITEM_SET(vvc1, vc1, multires);
    ITEM_SET(vvc1, vc1, syncmarker);
    ITEM_SET(vvc1, vc1, rangered);
    ITEM_SET(vvc1, vc1, maxbframes);
    ITEM_SET(vvc1, vc1, deblockEnable);
    ITEM_SET(vvc1, vc1, pquant);
    ITEM_SET(vvc1, vc1, slice_count);

    return 0;
}

static int fill_mjpeg_picture_desc(const struct pipe_picture_desc *desc,
                                   union virgl_picture_desc *vdsc)
{
    unsigned i;
    struct virgl_mjpeg_picture_desc *vmjpeg = &vdsc->mjpeg;
    struct pipe_mjpeg_picture_desc *mjpeg = (struct pipe_mjpeg_picture_desc *)desc;

    fill_base_picture_desc(desc, &vmjpeg->base);

    ITEM_SET(&vmjpeg->picture_parameter, &mjpeg->picture_parameter, picture_width);
    ITEM_SET(&vmjpeg->picture_parameter, &mjpeg->picture_parameter, picture_height);
    ITEM_SET(&vmjpeg->picture_parameter, &mjpeg->picture_parameter, crop_x);
    ITEM_SET(&vmjpeg->picture_parameter, &mjpeg->picture_parameter, crop_y);
    ITEM_SET(&vmjpeg->picture_parameter, &mjpeg->picture_parameter, crop_width);
    ITEM_SET(&vmjpeg->picture_parameter, &mjpeg->picture_parameter, crop_height);

    ITEM_SET(&vmjpeg->picture_parameter, &mjpeg->picture_parameter, num_components);
    for (i = 0; i < mjpeg->picture_parameter.num_components; ++i) {
        ITEM_SET(&vmjpeg->picture_parameter.components[i], &mjpeg->picture_parameter.components[i], component_id);
        ITEM_SET(&vmjpeg->picture_parameter.components[i], &mjpeg->picture_parameter.components[i], h_sampling_factor);
        ITEM_SET(&vmjpeg->picture_parameter.components[i], &mjpeg->picture_parameter.components[i], v_sampling_factor);
        ITEM_SET(&vmjpeg->picture_parameter.components[i], &mjpeg->picture_parameter.components[i], quantiser_table_selector);
    }

    ITEM_SET(&vmjpeg->slice_parameter, &mjpeg->slice_parameter, slice_data_size);
    ITEM_SET(&vmjpeg->slice_parameter, &mjpeg->slice_parameter, slice_data_offset);
    ITEM_SET(&vmjpeg->slice_parameter, &mjpeg->slice_parameter, slice_data_flag);
    ITEM_SET(&vmjpeg->slice_parameter, &mjpeg->slice_parameter, slice_horizontal_position);
    ITEM_SET(&vmjpeg->slice_parameter, &mjpeg->slice_parameter, slice_vertical_position);

    ITEM_SET(&vmjpeg->slice_parameter, &mjpeg->slice_parameter, num_components);
    for (i = 0; i < mjpeg->slice_parameter.num_components; ++i) {
        ITEM_SET(&vmjpeg->slice_parameter.components[i], &mjpeg->slice_parameter.components[i], component_selector);
        ITEM_SET(&vmjpeg->slice_parameter.components[i], &mjpeg->slice_parameter.components[i], dc_table_selector);
        ITEM_SET(&vmjpeg->slice_parameter.components[i], &mjpeg->slice_parameter.components[i], ac_table_selector);
    }
    
    ITEM_SET(&vmjpeg->slice_parameter, &mjpeg->slice_parameter, restart_interval);
    ITEM_SET(&vmjpeg->slice_parameter, &mjpeg->slice_parameter, num_mcus);

    ITEM_CPY(&vmjpeg->quantization_table, &mjpeg->quantization_table, load_quantiser_table);
    ITEM_CPY(&vmjpeg->quantization_table, &mjpeg->quantization_table, quantiser_table);

    for (i = 0; i < 2; ++i) {
        ITEM_SET(&vmjpeg->huffman_table, &mjpeg->huffman_table, load_huffman_table[i]);
        ITEM_CPY(&vmjpeg->huffman_table.table[i], &mjpeg->huffman_table.table[i], num_dc_codes);
        ITEM_CPY(&vmjpeg->huffman_table.table[i], &mjpeg->huffman_table.table[i], dc_values);
        ITEM_CPY(&vmjpeg->huffman_table.table[i], &mjpeg->huffman_table.table[i], num_ac_codes);
        ITEM_CPY(&vmjpeg->huffman_table.table[i], &mjpeg->huffman_table.table[i], ac_values);
        ITEM_CPY(&vmjpeg->huffman_table.table[i], &mjpeg->huffman_table.table[i], pad);
    }
    return 0;
}

static int fill_vp9_picture_desc(const struct pipe_picture_desc *desc,
                                   union virgl_picture_desc *vdsc)
{
    unsigned i;
    struct virgl_video_buffer *vbuf;
    struct virgl_vp9_picture_desc *vvp9 = &vdsc->vp9;
    struct pipe_vp9_picture_desc *vp9 = (struct pipe_vp9_picture_desc *)desc;

    fill_base_picture_desc(desc, &vvp9->base);

    for (i = 0; i < 16; i++) {
        vbuf = virgl_video_buffer(vp9->ref[i]);
        vvp9->ref[i] = vbuf ? vbuf->handle : 0;
    }

    ITEM_SET(vvp9, vp9, picture_parameter.frame_width);
    ITEM_SET(vvp9, vp9, picture_parameter.frame_height);
    ITEM_SET(vvp9, vp9, picture_parameter.pic_fields.subsampling_x);
    ITEM_SET(vvp9, vp9, picture_parameter.pic_fields.subsampling_y);
    ITEM_SET(vvp9, vp9, picture_parameter.pic_fields.frame_type);
    ITEM_SET(vvp9, vp9, picture_parameter.pic_fields.show_frame);
    ITEM_SET(vvp9, vp9, picture_parameter.pic_fields.error_resilient_mode);
    ITEM_SET(vvp9, vp9, picture_parameter.pic_fields.intra_only);
    ITEM_SET(vvp9, vp9, picture_parameter.pic_fields.allow_high_precision_mv);
    ITEM_SET(vvp9, vp9, picture_parameter.pic_fields.mcomp_filter_type);
    ITEM_SET(vvp9, vp9, picture_parameter.pic_fields.frame_parallel_decoding_mode);
    ITEM_SET(vvp9, vp9, picture_parameter.pic_fields.reset_frame_context);
    ITEM_SET(vvp9, vp9, picture_parameter.pic_fields.refresh_frame_context);
    ITEM_SET(vvp9, vp9, picture_parameter.pic_fields.frame_context_idx);
    ITEM_SET(vvp9, vp9, picture_parameter.pic_fields.segmentation_enabled);
    ITEM_SET(vvp9, vp9, picture_parameter.pic_fields.segmentation_temporal_update);
    ITEM_SET(vvp9, vp9, picture_parameter.pic_fields.segmentation_update_map);
    ITEM_SET(vvp9, vp9, picture_parameter.pic_fields.last_ref_frame);
    ITEM_SET(vvp9, vp9, picture_parameter.pic_fields.last_ref_frame_sign_bias);
    ITEM_SET(vvp9, vp9, picture_parameter.pic_fields.golden_ref_frame);
    ITEM_SET(vvp9, vp9, picture_parameter.pic_fields.golden_ref_frame_sign_bias);
    ITEM_SET(vvp9, vp9, picture_parameter.pic_fields.alt_ref_frame);
    ITEM_SET(vvp9, vp9, picture_parameter.pic_fields.alt_ref_frame_sign_bias);
    ITEM_SET(vvp9, vp9, picture_parameter.pic_fields.lossless_flag);
    ITEM_SET(vvp9, vp9, picture_parameter.filter_level);
    ITEM_SET(vvp9, vp9, picture_parameter.sharpness_level);
    ITEM_SET(vvp9, vp9, picture_parameter.log2_tile_rows);
    ITEM_SET(vvp9, vp9, picture_parameter.log2_tile_columns);
    ITEM_SET(vvp9, vp9, picture_parameter.frame_header_length_in_bytes);
    ITEM_SET(vvp9, vp9, picture_parameter.first_partition_size);
    ITEM_CPY(vvp9, vp9, picture_parameter.mb_segment_tree_probs);
    ITEM_CPY(vvp9, vp9, picture_parameter.segment_pred_probs);
    ITEM_SET(vvp9, vp9, picture_parameter.profile);
    ITEM_SET(vvp9, vp9, picture_parameter.bit_depth);
    ITEM_SET(vvp9, vp9, picture_parameter.mode_ref_delta_enabled);
    ITEM_SET(vvp9, vp9, picture_parameter.mode_ref_delta_update);
    ITEM_SET(vvp9, vp9, picture_parameter.base_qindex);
    ITEM_SET(vvp9, vp9, picture_parameter.y_dc_delta_q);
    ITEM_SET(vvp9, vp9, picture_parameter.uv_ac_delta_q);
    ITEM_SET(vvp9, vp9, picture_parameter.uv_dc_delta_q);
    ITEM_SET(vvp9, vp9, picture_parameter.abs_delta);
    ITEM_CPY(vvp9, vp9, picture_parameter.ref_deltas);
    ITEM_CPY(vvp9, vp9, picture_parameter.mode_deltas);
    
    vvp9->slice_parameter.slice_data_size = vp9->slice_parameter.slice_data_size[0];
    vvp9->slice_parameter.slice_data_offset = vp9->slice_parameter.slice_data_offset[0];
    vvp9->slice_parameter.slice_data_flag = vp9->slice_parameter.slice_data_flag[0];

    for (i = 0; i < 8; i++) {
        ITEM_SET(vvp9, vp9, slice_parameter.seg_param[i].segment_flags.segment_reference_enabled);
        ITEM_SET(vvp9, vp9, slice_parameter.seg_param[i].segment_flags.segment_reference);
        ITEM_SET(vvp9, vp9, slice_parameter.seg_param[i].segment_flags.segment_reference_skipped);
        ITEM_CPY(vvp9, vp9, slice_parameter.seg_param[i].filter_level);
        ITEM_SET(vvp9, vp9, slice_parameter.seg_param[i].luma_ac_quant_scale);
        ITEM_SET(vvp9, vp9, slice_parameter.seg_param[i].luma_dc_quant_scale);
        ITEM_SET(vvp9, vp9, slice_parameter.seg_param[i].chroma_ac_quant_scale);
        ITEM_SET(vvp9, vp9, slice_parameter.seg_param[i].chroma_dc_quant_scale);
    }

    return 0;
}

static int fill_av1_picture_desc(const struct pipe_picture_desc *desc,
                                 union virgl_picture_desc *vdsc)
{
    unsigned i;
    struct virgl_video_buffer *vbuf;
    struct virgl_av1_picture_desc *vav1 = &vdsc->av1;
    struct pipe_av1_picture_desc *av1 = (struct pipe_av1_picture_desc *)desc;

    fill_base_picture_desc(desc, &vav1->base);

    for (i = 0; i < ARRAY_SIZE(vav1->ref); i++) {
        vbuf = virgl_video_buffer(av1->ref[i]);
        vav1->ref[i] = vbuf ? vbuf->handle : 0;
    }
    vbuf = virgl_video_buffer(av1->film_grain_target);
    vav1->film_grain_target = vbuf ? vbuf->handle : 0;

    ITEM_SET(vav1, av1, picture_parameter.profile);
    ITEM_SET(vav1, av1, picture_parameter.order_hint_bits_minus_1);
    ITEM_SET(vav1, av1, picture_parameter.bit_depth_idx);

    ITEM_SET(vav1, av1, picture_parameter.seq_info_fields.use_128x128_superblock);
    ITEM_SET(vav1, av1, picture_parameter.seq_info_fields.enable_filter_intra);
    ITEM_SET(vav1, av1, picture_parameter.seq_info_fields.enable_intra_edge_filter);
    ITEM_SET(vav1, av1, picture_parameter.seq_info_fields.enable_interintra_compound);
    ITEM_SET(vav1, av1, picture_parameter.seq_info_fields.enable_masked_compound);
    ITEM_SET(vav1, av1, picture_parameter.seq_info_fields.enable_dual_filter);
    ITEM_SET(vav1, av1, picture_parameter.seq_info_fields.enable_order_hint);
    ITEM_SET(vav1, av1, picture_parameter.seq_info_fields.enable_jnt_comp);
    ITEM_SET(vav1, av1, picture_parameter.seq_info_fields.enable_cdef);
    ITEM_SET(vav1, av1, picture_parameter.seq_info_fields.mono_chrome);
    ITEM_SET(vav1, av1, picture_parameter.seq_info_fields.ref_frame_mvs);
    ITEM_SET(vav1, av1, picture_parameter.seq_info_fields.film_grain_params_present);

    ITEM_SET(vav1, av1, picture_parameter.current_frame_id);
    ITEM_SET(vav1, av1, picture_parameter.frame_width);
    ITEM_SET(vav1, av1, picture_parameter.frame_height);
    ITEM_SET(vav1, av1, picture_parameter.max_width);
    ITEM_SET(vav1, av1, picture_parameter.max_height);
    ITEM_CPY(vav1, av1, picture_parameter.ref_frame_idx);
    ITEM_SET(vav1, av1, picture_parameter.primary_ref_frame);
    ITEM_SET(vav1, av1, picture_parameter.order_hint);

    ITEM_SET(vav1, av1, picture_parameter.seg_info.segment_info_fields.enabled);
    ITEM_SET(vav1, av1, picture_parameter.seg_info.segment_info_fields.update_map);
    ITEM_SET(vav1, av1, picture_parameter.seg_info.segment_info_fields.update_data);
    ITEM_SET(vav1, av1, picture_parameter.seg_info.segment_info_fields.temporal_update);
    ITEM_CPY(vav1, av1, picture_parameter.seg_info.feature_data);
    ITEM_CPY(vav1, av1, picture_parameter.seg_info.feature_mask);

    ITEM_SET(vav1, av1, picture_parameter.film_grain_info.film_grain_info_fields.apply_grain);
    ITEM_SET(vav1, av1, picture_parameter.film_grain_info.film_grain_info_fields.chroma_scaling_from_luma);
    ITEM_SET(vav1, av1, picture_parameter.film_grain_info.film_grain_info_fields.grain_scaling_minus_8);
    ITEM_SET(vav1, av1, picture_parameter.film_grain_info.film_grain_info_fields.ar_coeff_lag);
    ITEM_SET(vav1, av1, picture_parameter.film_grain_info.film_grain_info_fields.ar_coeff_shift_minus_6);
    ITEM_SET(vav1, av1, picture_parameter.film_grain_info.film_grain_info_fields.grain_scale_shift);
    ITEM_SET(vav1, av1, picture_parameter.film_grain_info.film_grain_info_fields.overlap_flag);
    ITEM_SET(vav1, av1, picture_parameter.film_grain_info.film_grain_info_fields.clip_to_restricted_range);

    ITEM_SET(vav1, av1, picture_parameter.film_grain_info.grain_seed);
    ITEM_SET(vav1, av1, picture_parameter.film_grain_info.num_y_points);
    ITEM_CPY(vav1, av1, picture_parameter.film_grain_info.point_y_value);
    ITEM_CPY(vav1, av1, picture_parameter.film_grain_info.point_y_scaling);
    ITEM_SET(vav1, av1, picture_parameter.film_grain_info.num_cb_points);
    ITEM_CPY(vav1, av1, picture_parameter.film_grain_info.point_cb_value);
    ITEM_CPY(vav1, av1, picture_parameter.film_grain_info.point_cb_scaling);
    ITEM_SET(vav1, av1, picture_parameter.film_grain_info.num_cr_points);
    ITEM_CPY(vav1, av1, picture_parameter.film_grain_info.point_cr_value);
    ITEM_CPY(vav1, av1, picture_parameter.film_grain_info.point_cr_scaling);
    ITEM_CPY(vav1, av1, picture_parameter.film_grain_info.ar_coeffs_y);
    ITEM_CPY(vav1, av1, picture_parameter.film_grain_info.ar_coeffs_cb);
    ITEM_CPY(vav1, av1, picture_parameter.film_grain_info.ar_coeffs_cr);
    ITEM_SET(vav1, av1, picture_parameter.film_grain_info.cb_mult);
    ITEM_SET(vav1, av1, picture_parameter.film_grain_info.cb_luma_mult);
    ITEM_SET(vav1, av1, picture_parameter.film_grain_info.cb_offset);
    ITEM_SET(vav1, av1, picture_parameter.film_grain_info.cr_mult);
    ITEM_SET(vav1, av1, picture_parameter.film_grain_info.cr_luma_mult);
    ITEM_SET(vav1, av1, picture_parameter.film_grain_info.cr_offset);

    ITEM_SET(vav1, av1, picture_parameter.tile_cols);
    ITEM_SET(vav1, av1, picture_parameter.tile_rows);
    ITEM_CPY(vav1, av1, picture_parameter.tile_col_start_sb);
    ITEM_CPY(vav1, av1, picture_parameter.tile_row_start_sb);
    ITEM_CPY(vav1, av1, picture_parameter.width_in_sbs);
    ITEM_CPY(vav1, av1, picture_parameter.height_in_sbs);
    ITEM_SET(vav1, av1, picture_parameter.context_update_tile_id);

    ITEM_SET(vav1, av1, picture_parameter.pic_info_fields.frame_type);
    ITEM_SET(vav1, av1, picture_parameter.pic_info_fields.show_frame);
    ITEM_SET(vav1, av1, picture_parameter.pic_info_fields.showable_frame);
    ITEM_SET(vav1, av1, picture_parameter.pic_info_fields.error_resilient_mode);
    ITEM_SET(vav1, av1, picture_parameter.pic_info_fields.disable_cdf_update);
    ITEM_SET(vav1, av1, picture_parameter.pic_info_fields.allow_screen_content_tools);
    ITEM_SET(vav1, av1, picture_parameter.pic_info_fields.force_integer_mv);
    ITEM_SET(vav1, av1, picture_parameter.pic_info_fields.allow_intrabc);
    ITEM_SET(vav1, av1, picture_parameter.pic_info_fields.use_superres);
    ITEM_SET(vav1, av1, picture_parameter.pic_info_fields.allow_high_precision_mv);
    ITEM_SET(vav1, av1, picture_parameter.pic_info_fields.is_motion_mode_switchable);
    ITEM_SET(vav1, av1, picture_parameter.pic_info_fields.use_ref_frame_mvs);
    ITEM_SET(vav1, av1, picture_parameter.pic_info_fields.disable_frame_end_update_cdf);
    ITEM_SET(vav1, av1, picture_parameter.pic_info_fields.uniform_tile_spacing_flag);
    ITEM_SET(vav1, av1, picture_parameter.pic_info_fields.allow_warped_motion);
    ITEM_SET(vav1, av1, picture_parameter.pic_info_fields.large_scale_tile);

    ITEM_SET(vav1, av1, picture_parameter.superres_scale_denominator);
    ITEM_SET(vav1, av1, picture_parameter.interp_filter);
    ITEM_CPY(vav1, av1, picture_parameter.filter_level);
    ITEM_SET(vav1, av1, picture_parameter.filter_level_u);
    ITEM_SET(vav1, av1, picture_parameter.filter_level_v);
    ITEM_SET(vav1, av1, picture_parameter.loop_filter_info_fields.sharpness_level);
    ITEM_SET(vav1, av1, picture_parameter.loop_filter_info_fields.mode_ref_delta_enabled);
    ITEM_SET(vav1, av1, picture_parameter.loop_filter_info_fields.mode_ref_delta_update);

    ITEM_CPY(vav1, av1, picture_parameter.ref_deltas);
    ITEM_CPY(vav1, av1, picture_parameter.mode_deltas);
    ITEM_SET(vav1, av1, picture_parameter.base_qindex);
    ITEM_SET(vav1, av1, picture_parameter.y_dc_delta_q);
    ITEM_SET(vav1, av1, picture_parameter.u_dc_delta_q);
    ITEM_SET(vav1, av1, picture_parameter.u_ac_delta_q);
    ITEM_SET(vav1, av1, picture_parameter.v_dc_delta_q);
    ITEM_SET(vav1, av1, picture_parameter.v_ac_delta_q);

    ITEM_SET(vav1, av1, picture_parameter.qmatrix_fields.using_qmatrix);
    ITEM_SET(vav1, av1, picture_parameter.qmatrix_fields.qm_y);
    ITEM_SET(vav1, av1, picture_parameter.qmatrix_fields.qm_u);
    ITEM_SET(vav1, av1, picture_parameter.qmatrix_fields.qm_v);

    ITEM_SET(vav1, av1, picture_parameter.mode_control_fields.delta_q_present_flag);
    ITEM_SET(vav1, av1, picture_parameter.mode_control_fields.log2_delta_q_res);
    ITEM_SET(vav1, av1, picture_parameter.mode_control_fields.delta_lf_present_flag);
    ITEM_SET(vav1, av1, picture_parameter.mode_control_fields.log2_delta_lf_res);
    ITEM_SET(vav1, av1, picture_parameter.mode_control_fields.delta_lf_multi);
    ITEM_SET(vav1, av1, picture_parameter.mode_control_fields.tx_mode);
    ITEM_SET(vav1, av1, picture_parameter.mode_control_fields.reference_select);
    ITEM_SET(vav1, av1, picture_parameter.mode_control_fields.reduced_tx_set_used);
    ITEM_SET(vav1, av1, picture_parameter.mode_control_fields.skip_mode_present);

    ITEM_SET(vav1, av1, picture_parameter.cdef_damping_minus_3);
    ITEM_SET(vav1, av1, picture_parameter.cdef_bits);
    ITEM_CPY(vav1, av1, picture_parameter.cdef_y_strengths);
    ITEM_CPY(vav1, av1, picture_parameter.cdef_uv_strengths);

    ITEM_SET(vav1, av1, picture_parameter.loop_restoration_fields.yframe_restoration_type);
    ITEM_SET(vav1, av1, picture_parameter.loop_restoration_fields.cbframe_restoration_type);
    ITEM_SET(vav1, av1, picture_parameter.loop_restoration_fields.crframe_restoration_type);
    ITEM_SET(vav1, av1, picture_parameter.loop_restoration_fields.lr_unit_shift);
    ITEM_SET(vav1, av1, picture_parameter.loop_restoration_fields.lr_uv_shift);

    for (i = 0; i < ARRAY_SIZE(vav1->picture_parameter.wm); i++) {
        ITEM_SET(vav1, av1, picture_parameter.wm[i].wmtype);
        ITEM_SET(vav1, av1, picture_parameter.wm[i].invalid);
        ITEM_CPY(vav1, av1, picture_parameter.wm[i].wmmat);
    }

    ITEM_SET(vav1, av1, picture_parameter.refresh_frame_flags);
    ITEM_SET(vav1, av1, picture_parameter.matrix_coefficients);

    ITEM_CPY(vav1, av1, slice_parameter.slice_data_size);
    ITEM_CPY(vav1, av1, slice_parameter.slice_data_offset);
    ITEM_CPY(vav1, av1, slice_parameter.slice_data_row);
    ITEM_CPY(vav1, av1, slice_parameter.slice_data_col);
    ITEM_CPY(vav1, av1, slice_parameter.slice_data_anchor_frame_idx);
    ITEM_SET(vav1, av1, slice_parameter.slice_count);

    return 0;
}

#undef ITEM_SET
#undef ITEM_CPY

static int fill_picture_desc(const struct pipe_picture_desc *desc,
                             union virgl_picture_desc *vdsc)
{
    switch (u_reduce_video_profile(desc->profile)) {
    case PIPE_VIDEO_FORMAT_MPEG4:
        return fill_mpeg4_picture_desc(desc, vdsc);
    case PIPE_VIDEO_FORMAT_MPEG4_AVC:
        return fill_h264_picture_desc(desc, vdsc);
    case PIPE_VIDEO_FORMAT_HEVC:
        return fill_h265_picture_desc(desc, vdsc);
    case PIPE_VIDEO_FORMAT_MPEG12:
        return fill_mpeg12_picture_desc(desc, vdsc);
    case PIPE_VIDEO_FORMAT_VC1:
        return fill_vc1_picture_desc(desc, vdsc);
    case PIPE_VIDEO_FORMAT_JPEG:
        return fill_mjpeg_picture_desc(desc, vdsc);
    case PIPE_VIDEO_FORMAT_VP9:
        return fill_vp9_picture_desc(desc, vdsc);
    case PIPE_VIDEO_FORMAT_AV1:
        return fill_av1_picture_desc(desc, vdsc);
    default:
        return -1;
    }
}

static int fill_enc_picture_desc(const struct pipe_picture_desc *desc,
                                 union virgl_picture_desc *vdsc)
{
    switch (u_reduce_video_profile(desc->profile)) {
    case PIPE_VIDEO_FORMAT_MPEG4_AVC:
        return fill_h264_enc_picture_desc(desc, vdsc);
    case PIPE_VIDEO_FORMAT_HEVC:
        return fill_h265_enc_picture_desc(desc, vdsc);
    default:
        return -1;
    }
}

static void virgl_video_begin_frame(struct pipe_video_codec *codec,
                                    struct pipe_video_buffer *target,
                                    struct pipe_picture_desc *picture)
{
    struct virgl_video_codec *vcdc = virgl_video_codec(codec);
    struct virgl_video_buffer *vbuf = virgl_video_buffer(target);

    if (codec->entrypoint == PIPE_VIDEO_ENTRYPOINT_ENCODE)
        fill_enc_picture_desc(picture, &vcdc->desc);

    virgl_encode_begin_frame(vcdc->vctx, vcdc, vbuf);
}

static void virgl_video_decode_macroblock(struct pipe_video_codec *codec,
                                          struct pipe_video_buffer *target,
                                          struct pipe_picture_desc *picture,
                                          const struct pipe_macroblock *macroblocks,
                                          unsigned num_macroblocks)
{
    (void)codec;
    (void)target;
    (void)picture;
    (void)macroblocks;
    (void)num_macroblocks;
}

static void virgl_video_decode_bitstream(struct pipe_video_codec *codec,
                                         struct pipe_video_buffer *target,
                                         struct pipe_picture_desc *picture,
                                         unsigned num_buffers,
                                         const void * const *buffers,
                                         const unsigned *sizes)
{
    struct virgl_video_codec *vcdc = virgl_video_codec(codec);
    struct virgl_video_buffer *vbuf = virgl_video_buffer(target);
    struct virgl_context *vctx = vcdc->vctx;
    struct virgl_screen *vs = virgl_screen(vctx->base.screen);
    struct virgl_resource *vres;
    union  virgl_picture_desc vdsc;
    struct pipe_transfer *xfer = NULL;
    void *ptr;
    unsigned i, total_size;

    /* transfer bitstream data */
    for (i = 0, total_size = 0; i < num_buffers; i++)
        total_size += sizes[i];

    if (total_size > pipe_buffer_size(vcdc->bs_buffers[vcdc->cur_buffer])) {
        pipe_resource_reference(&vcdc->bs_buffers[vcdc->cur_buffer], NULL);
        vcdc->bs_buffers[vcdc->cur_buffer] = pipe_buffer_create(vctx->base.screen,
                        PIPE_BIND_CUSTOM, PIPE_USAGE_STAGING, total_size);
    }

    vctx->base.flush(&vctx->base, NULL, 0);

    vres = virgl_resource(vcdc->bs_buffers[vcdc->cur_buffer]);
    vs->vws->resource_wait(vs->vws, vres->hw_res);
    ptr = pipe_buffer_map(&vctx->base, vcdc->bs_buffers[vcdc->cur_buffer],
                          PIPE_MAP_WRITE, &xfer);
    if (!ptr)
        return;
    for (i = 0, vcdc->bs_size = 0; i < num_buffers; i++) {
        memcpy(ptr + vcdc->bs_size, buffers[i], sizes[i]);
        vcdc->bs_size += sizes[i];
    }
    pipe_buffer_unmap(&vctx->base, xfer);

    /* transfer picture description */
    fill_picture_desc(picture, &vdsc);
    vres = virgl_resource(vcdc->desc_buffers[vcdc->cur_buffer]);
    vs->vws->resource_wait(vs->vws, vres->hw_res);
    ptr = pipe_buffer_map(&vctx->base, vcdc->desc_buffers[vcdc->cur_buffer],
                          PIPE_MAP_WRITE, &xfer);
    if (!ptr)
        return;
    memcpy(ptr, &vdsc, sizeof(vdsc));
    pipe_buffer_unmap(&vctx->base, xfer);

    virgl_encode_decode_bitstream(vctx, vcdc, vbuf, &vdsc, sizeof(vdsc));
}

static void virgl_video_encode_bitstream(struct pipe_video_codec *codec,
                                         struct pipe_video_buffer *source,
                                         struct pipe_resource *target,
                                         void **feedback)
{
    struct virgl_video_codec *vcdc = virgl_video_codec(codec);
    struct virgl_context *vctx = vcdc->vctx;
    struct virgl_screen *vs = virgl_screen(vctx->base.screen);
    struct virgl_resource *vres;
    struct virgl_video_encode_feedback *fb;
    struct pipe_transfer *xfer = NULL;
    void *ptr;

    /* Transfer picture desc */
    vres = virgl_resource(vcdc->desc_buffers[vcdc->cur_buffer]);
    vs->vws->resource_wait(vs->vws, vres->hw_res);
    ptr = pipe_buffer_map(&vctx->base, vcdc->desc_buffers[vcdc->cur_buffer],
                          PIPE_MAP_WRITE, &xfer);
    if (!ptr)
        return;
    memcpy(ptr, &vcdc->desc, sizeof(vcdc->desc));
    pipe_buffer_unmap(&vctx->base, xfer);

    /* Init feedback */
    vres = virgl_resource(vcdc->feed_buffers[vcdc->cur_buffer]);
    vs->vws->resource_wait(vs->vws, vres->hw_res);
    fb = pipe_buffer_map(&vctx->base, vcdc->feed_buffers[vcdc->cur_buffer],
                         PIPE_MAP_WRITE, &xfer);
    if (!fb)
        return;
    fb->stat = VIRGL_VIDEO_ENCODE_STAT_NOT_STARTED;
    fb->coded_size = 0;
    pipe_buffer_unmap(&vctx->base, xfer);
    *feedback = vres;

    /*
     * These objects do not need to be transferred manually:
     *   source - corresponds to VASurface in VA-API
     *   target - corresponds to VACodedBuffer in VA-API
     */

    virgl_encode_encode_bitstream(vctx, vcdc, virgl_video_buffer(source),
                                  virgl_resource(target));
}

static void virgl_video_end_frame(struct pipe_video_codec *codec,
                                  struct pipe_video_buffer *target,
                                  struct pipe_picture_desc *picture)
{
    struct virgl_video_codec *vcdc = virgl_video_codec(codec);
    struct virgl_context *vctx = virgl_context(vcdc->base.context);
    struct virgl_video_buffer *vbuf = virgl_video_buffer(target);

    virgl_encode_end_frame(vctx, vcdc, vbuf);
    virgl_flush_eq(vctx, vctx, NULL);

    switch_buffer(vcdc);
}

static int virgl_video_get_decoder_fence(struct pipe_video_codec *decoder,
                                         struct pipe_fence_handle *fence,
                                         uint64_t timeout) {
    struct virgl_video_codec *vcdc = virgl_video_codec(decoder);
    struct virgl_context *vctx = vcdc->vctx;
    struct virgl_screen *vs = virgl_screen(vctx->base.screen);

    return vs->vws->fence_wait(vs->vws, fence, timeout);
}

static void virgl_video_flush(struct pipe_video_codec *codec)
{
    struct pipe_context *ctx = codec->context;
    struct pipe_fence_handle *fence = NULL;

    ctx->flush(ctx, &fence, 0);
    if (fence) {
        ctx->screen->fence_finish(ctx->screen, NULL, fence, OS_TIMEOUT_INFINITE);
        ctx->screen->fence_reference(ctx->screen, &fence, NULL);
    }
}

static void virgl_video_get_feedback(struct pipe_video_codec *codec,
                                     void *feedback,
                                     unsigned *size,
                                     struct pipe_enc_feedback_metadata* metadata)
{
    struct virgl_video_codec *vcdc = virgl_video_codec(codec);
    struct virgl_context *vctx = vcdc->vctx;
    struct virgl_screen *vs = virgl_screen(vctx->base.screen);
    struct virgl_resource *vres = feedback;
    struct virgl_video_encode_feedback *fb;
    struct pipe_transfer *xfer;

    if (!feedback || !size)
        return;

    vs->vws->resource_wait(vs->vws, vres->hw_res);
    fb = pipe_buffer_map(&vctx->base, &vres->b, PIPE_MAP_READ, &xfer);
    if (!fb)
        return;
    if (fb->stat == VIRGL_VIDEO_ENCODE_STAT_SUCCESS) {
        *size = fb->coded_size;
    } else {
        *size = 0;
        if (virgl_debug & VIRGL_DEBUG_VIDEO) {
            debug_printf("unexpected encode feedback: %u\n", fb->stat);
        }
    }
    pipe_buffer_unmap(&vctx->base, xfer);
}

static void virgl_video_destroy_codec(struct pipe_video_codec *codec)
{
    unsigned i;
    struct virgl_video_codec *vcdc = virgl_video_codec(codec);
    struct virgl_context *vctx = virgl_context(vcdc->base.context);

    for (i = 0; i < VIRGL_VIDEO_CODEC_BUF_NUM; i++) {
        if (codec->entrypoint != PIPE_VIDEO_ENTRYPOINT_ENCODE) {
            pipe_resource_reference(&vcdc->bs_buffers[i], NULL);
        } else {
            pipe_resource_reference(&vcdc->feed_buffers[i], NULL);
        }
        pipe_resource_reference(&vcdc->desc_buffers[i], NULL);
    }

    virgl_encode_destroy_video_codec(vctx, vcdc);

    free(vcdc);
}


struct pipe_video_codec *
virgl_video_create_codec(struct pipe_context *ctx,
                         const struct pipe_video_codec *templ)
{
    unsigned i;
    struct virgl_video_codec *vcdc;
    struct virgl_context *vctx = virgl_context(ctx);
    unsigned width = templ->width, height = templ->height;

    if (virgl_debug & VIRGL_DEBUG_VIDEO)
        debug_printf("VIDEO: create codec. profile=%d, level=%u, entryp=%d, "
                     "chroma_fmt=%d, size=%ux%u, max_ref=%u, expect=%d\n",
                     templ->profile, templ->level, templ->entrypoint,
                     templ->chroma_format, templ->width, templ->height,
                     templ->max_references, templ->expect_chunked_decode);

    switch (u_reduce_video_profile(templ->profile)) {
    case PIPE_VIDEO_FORMAT_MPEG4: /* fall through */
    case PIPE_VIDEO_FORMAT_MPEG4_AVC:
        width = align(width, VL_MACROBLOCK_WIDTH);
        height = align(height, VL_MACROBLOCK_HEIGHT);
        break;
    case PIPE_VIDEO_FORMAT_HEVC: 
    case PIPE_VIDEO_FORMAT_MPEG12:
    case PIPE_VIDEO_FORMAT_VC1:
    case PIPE_VIDEO_FORMAT_JPEG:
    case PIPE_VIDEO_FORMAT_VP9:
    case PIPE_VIDEO_FORMAT_AV1: /* fall through */
    default:
        break;
    }

    vcdc = CALLOC_STRUCT(virgl_video_codec);
    if (!vcdc)
        return NULL;

    vcdc->base = *templ;
    vcdc->base.width = width;
    vcdc->base.height = height;
    vcdc->base.context = ctx;

    vcdc->base.destroy = virgl_video_destroy_codec;
    vcdc->base.begin_frame = virgl_video_begin_frame;
    vcdc->base.decode_macroblock = virgl_video_decode_macroblock;
    vcdc->base.decode_bitstream = virgl_video_decode_bitstream;
    vcdc->base.encode_bitstream = virgl_video_encode_bitstream;
    vcdc->base.end_frame = virgl_video_end_frame;
    vcdc->base.flush = virgl_video_flush;
    vcdc->base.get_feedback = virgl_video_get_feedback;

    vcdc->bs_size = 0;
    vcdc->cur_buffer = 0;
    for (i = 0; i < VIRGL_VIDEO_CODEC_BUF_NUM; i++) {
        if (templ->entrypoint != PIPE_VIDEO_ENTRYPOINT_ENCODE) {
            vcdc->bs_buffers[i] = pipe_buffer_create(ctx->screen,
                              PIPE_BIND_CUSTOM, PIPE_USAGE_STAGING,
                              BS_BUF_DEFAULT_SIZE(width, height));
        } else {
            vcdc->feed_buffers[i] = pipe_buffer_create(ctx->screen,
                                PIPE_BIND_CUSTOM, PIPE_USAGE_STAGING,
                                sizeof(struct virgl_video_encode_feedback));
        }

        vcdc->desc_buffers[i] = pipe_buffer_create(ctx->screen,
                            PIPE_BIND_CUSTOM, PIPE_USAGE_STAGING,
                            sizeof(union virgl_picture_desc));
    }

    vcdc->handle = virgl_object_assign_handle();
    vcdc->vctx = vctx;

    virgl_encode_create_video_codec(vctx, vcdc);

    return &vcdc->base;
}


static void virgl_video_destroy_buffer(struct pipe_video_buffer *buffer)
{
    struct virgl_video_buffer *vbuf = virgl_video_buffer(buffer);

    virgl_encode_destroy_video_buffer(vbuf->vctx, vbuf);

    vl_video_buffer_destroy(buffer);

    free(vbuf);
}

static void virgl_video_destroy_buffer_associated_data(void *data)
{
    (void)data;
}

struct pipe_video_buffer *
virgl_video_create_buffer(struct pipe_context *ctx,
                          const struct pipe_video_buffer *tmpl)
{
    struct virgl_context *vctx = virgl_context(ctx);
    struct virgl_video_buffer *vbuf;

    vbuf = CALLOC_STRUCT(virgl_video_buffer);
    if (!vbuf)
        return NULL;

    vbuf->buf = vl_video_buffer_create(ctx, tmpl);
    if (!vbuf->buf) {
        free(vbuf);
        return NULL;
    }
    vbuf->buf->destroy = virgl_video_destroy_buffer;
    vl_video_buffer_set_associated_data(vbuf->buf,
            NULL, vbuf, virgl_video_destroy_buffer_associated_data);

    vbuf->num_planes = util_format_get_num_planes(vbuf->buf->buffer_format);
    vbuf->plane_views = vbuf->buf->get_sampler_view_planes(vbuf->buf);
    vbuf->handle = virgl_object_assign_handle();
    vbuf->buffer_format = tmpl->buffer_format;
    vbuf->width = tmpl->width;
    vbuf->height = tmpl->height;
    vbuf->vctx = vctx;

    virgl_encode_create_video_buffer(vctx, vbuf);

    if (virgl_debug & VIRGL_DEBUG_VIDEO) {
        debug_printf("VIDEO: create buffer. fmt=%s, %ux%u, num_planes=%u\n",
                     util_format_name(tmpl->buffer_format),
                     tmpl->width, tmpl->height, vbuf->num_planes);

        for (unsigned i = 0; i < vbuf->num_planes; i++)
            if (vbuf->plane_views[i])
                debug_printf("VIDEO: plane[%d]: fmt=%s, target=%u\n", i,
                             util_format_name(vbuf->plane_views[i]->format),
                             vbuf->plane_views[i]->target);
    }

    return vbuf->buf;
}

