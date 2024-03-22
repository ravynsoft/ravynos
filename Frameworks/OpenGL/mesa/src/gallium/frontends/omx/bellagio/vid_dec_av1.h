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

#ifndef VID_DEC_AV1_H
#define VID_DEC_AV1_H

#include "util/u_thread.h"

enum av1_obu_type {
   AV1_OBU_SEQUENCE_HEADER = 1,
   AV1_OBU_TEMPORAL_DELIMITER,
   AV1_OBU_FRAME_HEADER,
   AV1_OBU_TILE_GROUP,
   AV1_OBU_METADATA,
   AV1_OBU_FRAME,
   AV1_OBU_REDUNDANT_FRAME_HEADER,
   AV1_OBU_TILE_LIST,
   AV1_OBU_PADDING = 15
};

enum av1_obu_frame_type {
   AV1_KEY_FRAME = 0,
   AV1_INTER_FRAME,
   AV1_INTRA_ONLY_FRAME,
   AV1_SWITCH_FRAME
};

enum av1_obu_color_primary {
   AV1_CP_BT_709 = 1,
   AV1_CP_UNSPECIFIED,
   AV1_CP_BT_470_M = 4,
   AV1_CP_BT_470_B_G,
   AV1_CP_BT_601,
   AV1_CP_SMPTE_240,
   AV1_CP_GENERIC_FILM,
   AV1_CP_BT_2020,
   AV1_CP_XYZ,
   AV1_CP_SMPTE_431,
   AV1_CP_SMPTE_432,
   AV1_CP_EBU_3213 = 22
};

enum av1_obu_transfer_characteristic {
   AV1_TC_RESERVED_0 = 0,
   AV1_TC_BT_709,
   AV1_TC_UNSPECIFIED,
   AV1_TC_RESERVED_3,
   AV1_TC_BT_470_M,
   AV1_TC_BT_470_B_G,
   AV1_TC_BT_601,
   AV1_TC_SMPTE_240,
   AV1_TC_LINEAR,
   AV1_TC_LOG_100,
   AV1_TC_LOG_100_SQRT10,
   AV1_TC_IEC_61966,
   AV1_TC_BT_1361,
   AV1_TC_SRGB,
   AV1_TC_BT_2020_10_BIT,
   AV1_TC_BT_2020_12_BIT,
   AV1_TC_SMPTE_2084,
   AV1_TC_SMPTE_428,
   AV1_TC_HLG
};

enum av1_obu_matrix_coefficient {
   AV1_MC_IDENTITY = 0,
   AV1_MC_BT_709,
   AV1_MC_UNSPECIFIED,
   AV1_MC_RESERVED_3,
   AV1_MC_FCC,
   AV1_MC_BT_470_B_G,
   AV1_MC_BT_601,
   AV1_MC_SMPTE_240,
   AV1_MC_SMPTE_YCGCO,
   AV1_MC_BT_2020_NCL,
   AV1_MC_BT_2020_CL,
   AV1_MC_SMPTE_2085,
   AV1_MC_CHROMAT_NCL,
   AV1_MC_CHROMAT_CL,
   AV1_MC_ICTCP
};

enum av1_obu_seg_lvl {
   AV1_SEG_LVL_ALT_Q = 0,
   AV1_SEG_LVL_ALT_LF_Y_V,
   AV1_SEG_LVL_REF_FRAME = 5,
   AV1_SEG_LVL_SKIP,
   AV1_SEG_LVL_GLOBALMV,
   AV1_SEG_LVL_MAX
};

enum av1_obu_lrtype {
   AV1_RESTORE_NONE = 0,
   AV1_RESTORE_WIENER,
   AV1_RESTORE_SGRPROJ,
   AV1_RESTORE_SWITCHABLE
};

enum av1_obu_txmode {
   AV1_ONLY_4X4 = 0,
   AV1_TX_MODE_LARGEST,
   AV1_TX_MODE_SELECT
};

enum av1_obu_gmtype {
   AV1_IDENTITY = 0,
   AV1_TRANSLATION,
   AV1_ROTZOOM,
   AV1_AFFINE
};

enum av1_frame_refs {
   AV1_NONE = -1,
   AV1_INTRA_FRAME = 0,
   AV1_LAST_FRAME,
   AV1_LAST2_FRAME,
   AV1_LAST3_FRAME,
   AV1_GOLDEN_FRAME,
   AV1_BWDREF_FRAME,
   AV1_ALTREF2_FRAME,
   AV1_ALTREF_FRAME
};

#define AV1_SELECT_SCREEN_CONTENT_TOOLS 2
#define AV1_SELECT_INTEGER_MV           2

#define AV1_PRIMARY_REF_NONE            7
#define AV1_REFS_PER_FRAME              7
#define AV1_NUM_REF_FRAMES              8

#define AV1_MAX_TILE_WIDTH              4096
#define AV1_MAX_TILE_AREA               (4096 * 2304)
#define AV1_MAX_TILE_ROWS               64
#define AV1_MAX_TILE_COLS               64
#define AV1_MAX_NUM_TILES               AV1_MAX_TILE_ROWS * AV1_MAX_TILE_COLS

#define AV1_MAX_SEGMENTS                8
#define AV1_MAX_CDEF_BITS_ARRAY         8
#define AV1_RESTORATION_TILESIZE        256
#define AV1_WARPEDMODEL_PREC_BITS       16
#define AV1_FG_MAX_NUM_Y_POINTS         14
#define AV1_FG_MAX_NUM_CBR_POINTS       10
#define AV1_FG_MAX_NUM_POS_LUMA         24
#define AV1_FG_MAX_NUM_POS_CHROMA       25

struct av1_obu_extension_header_obu
{
   unsigned temporal_id;
   unsigned spatial_id;
};

struct av1_sequence_header_obu {
   uint8_t seq_profile;
   bool reduced_still_picture_header;

   struct {
      bool equal_picture_interval;
   } timing_info;

   bool decoder_model_info_present_flag;
   struct {
      unsigned num_units_in_decoding_tick;
      uint8_t buffer_removal_time_length_minus_1;
      uint8_t frame_presentation_time_length_minus_1;
   } decoder_model_info;

   uint8_t operating_points_cnt_minus_1;
   uint16_t operating_point_idc[32];
   bool decoder_model_present_for_this_op[32];

   uint8_t frame_width_bits_minus_1;
   uint8_t frame_height_bits_minus_1;
   unsigned max_frame_width_minus_1;
   unsigned max_frame_height_minus_1;
   bool frame_id_numbers_present_flag;
   uint8_t delta_frame_id_length_minus_2;
   uint8_t additional_frame_id_length_minus_1;

   bool use_128x128_superblock;
   bool enable_filter_intra;
   bool enable_intra_edge_filter;
   bool enable_interintra_compound;
   bool enable_masked_compound;
   bool enable_warped_motion;
   bool enable_dual_filter;
   bool enable_order_hint;
   bool enable_jnt_comp;
   bool enable_ref_frame_mvs;
   uint8_t seq_force_screen_content_tools;
   uint8_t seq_force_integer_mv;

   uint8_t order_hint_bits_minus_1;
   uint8_t OrderHintBits;
   bool enable_superres;
   bool enable_cdef;
   bool enable_restoration;
   struct {
      uint8_t BitDepth;
      bool mono_chrome;
      uint8_t NumPlanes;
      bool subsampling_x;
      bool subsampling_y;
      bool separate_uv_delta_q;
   } color_config;
   bool film_grain_params_present;
};

struct ref_frame {
   uint8_t RefFrameType;
   unsigned RefFrameId;
   unsigned RefUpscaledWidth;
   unsigned RefFrameWidth;
   unsigned RefFrameHeight;
   unsigned RefRenderWidth;
   unsigned RefRenderHeight;
};

struct tile_info {
   unsigned TileColsLog2;
   unsigned TileRowsLog2;
   int tile_col_start_sb[AV1_MAX_TILE_COLS + 1];
   int tile_row_start_sb[AV1_MAX_TILE_ROWS + 1];
   unsigned TileCols;
   unsigned TileRows;
   unsigned context_update_tile_id;
   uint8_t TileSizeBytes;
};

struct quantization_params {
   uint8_t base_q_idx;
   int DeltaQYDc;
   int DeltaQUDc;
   int DeltaQUAc;
   int DeltaQVDc;
   int DeltaQVAc;
   uint8_t qm_y;
   uint8_t qm_u;
   uint8_t qm_v;
};

struct segmentation_params {
   bool segmentation_enabled;
   bool segmentation_update_map;
   bool segmentation_temporal_update;
   bool FeatureEnabled[AV1_MAX_SEGMENTS][AV1_SEG_LVL_MAX];
   int FeatureData[AV1_MAX_SEGMENTS][AV1_SEG_LVL_MAX];
   int FeatureMask[AV1_MAX_SEGMENTS];
};

struct delta_q_params {
   bool delta_q_present;
   uint8_t delta_q_res;
};

struct delta_lf_params {
   bool delta_lf_present;
   uint8_t delta_lf_res;
   bool delta_lf_multi;
};

struct loop_filter_params {
   uint8_t loop_filter_level[4];
   uint8_t loop_filter_sharpness;
   bool loop_filter_delta_enabled;
   bool loop_filter_delta_update;
   int8_t loop_filter_ref_deltas[AV1_NUM_REF_FRAMES];
   int8_t loop_filter_mode_deltas[2];
};

struct cdef_params {
   uint8_t cdef_damping_minus_3;
   uint8_t cdef_bits;
   uint16_t cdef_y_strengths[AV1_MAX_CDEF_BITS_ARRAY];
   uint16_t cdef_uv_strengths[AV1_MAX_CDEF_BITS_ARRAY];
};

struct loop_restoration_params {
   uint8_t FrameRestorationType[3];
   uint16_t LoopRestorationSize[3];
};

struct tx_mode_params {
   uint8_t TxMode;
};

enum reference_mode {
   SINGLE_REFERENCE = 0,
   COMPOUND_REFERENCE = 1,
   REFERENCE_MODE_SELECT = 2,
   REFERENCE_MODES = 3,
};

struct skip_mode_params {
   bool skip_mode_present;
};

struct global_motion_params {
   uint8_t GmType[AV1_NUM_REF_FRAMES];
   int gm_params[AV1_NUM_REF_FRAMES][6];
};

struct film_grain_params {
   bool apply_grain;
   uint16_t grain_seed;
   uint8_t num_y_points;
   uint8_t point_y_value[AV1_FG_MAX_NUM_Y_POINTS];
   uint8_t point_y_scaling[AV1_FG_MAX_NUM_Y_POINTS];
   bool chroma_scaling_from_luma;
   uint8_t num_cb_points;
   uint8_t num_cr_points;
   uint8_t point_cb_value[AV1_FG_MAX_NUM_CBR_POINTS];
   uint8_t point_cb_scaling[AV1_FG_MAX_NUM_CBR_POINTS];
   uint8_t point_cr_value[AV1_FG_MAX_NUM_CBR_POINTS];
   uint8_t point_cr_scaling[AV1_FG_MAX_NUM_CBR_POINTS];
   uint8_t grain_scaling_minus_8;
   uint8_t ar_coeff_lag;
   int ar_coeffs_y[AV1_FG_MAX_NUM_POS_LUMA];
   int ar_coeffs_cb[AV1_FG_MAX_NUM_POS_CHROMA];
   int ar_coeffs_cr[AV1_FG_MAX_NUM_POS_CHROMA];
   uint8_t ar_coeff_shift_minus_6;
   int ar_coeff_shift;
   uint8_t grain_scale_shift;
   uint8_t cb_mult;
   uint8_t cb_luma_mult;
   uint16_t cb_offset;
   uint8_t cr_mult;
   uint8_t cr_luma_mult;
   uint16_t cr_offset;
   bool overlap_flag;
   bool clip_to_restricted_range;
};

struct av1_uncompressed_header_obu
{
   uint8_t frame_type;
   bool FrameIsIntra;
   bool show_frame;
   bool showable_frame;
   bool show_existing_frame;
   uint8_t frame_to_show_map_idx;

   unsigned RefOrderHint[AV1_NUM_REF_FRAMES];
   bool error_resilient_mode;
   bool disable_cdf_update;
   bool allow_screen_content_tools;
   bool force_integer_mv;

   unsigned current_frame_id;
   bool frame_size_override_flag;
   unsigned OrderHint;
   uint8_t primary_ref_frame;

   uint8_t refresh_frame_flags;
   unsigned FrameWidth;
   unsigned FrameHeight;
   bool use_superres;
   unsigned SuperresDenom;
   unsigned UpscaledWidth;
   unsigned MiCols;
   unsigned MiRows;

   unsigned RenderWidth;
   unsigned RenderHeight;

   uint8_t last_frame_idx;
   uint8_t gold_frame_idx;
   uint8_t ref_frame_idx[AV1_REFS_PER_FRAME];
   uint8_t usedFrame[AV1_NUM_REF_FRAMES];
   unsigned curFrameHint;
   unsigned shiftedOrderHints[AV1_NUM_REF_FRAMES];

   bool allow_high_precision_mv;
   bool use_ref_frame_mvs;
   bool allow_intrabc;
   uint8_t interpolation_filter;
   bool is_motion_mode_switchable;

   bool disable_frame_end_update_cdf;
   struct tile_info ti;
   struct quantization_params qp;
   struct segmentation_params sp;
   struct delta_q_params dqp;
   struct delta_lf_params dlfp;

   bool CodedLossless;
   bool AllLossless;
   struct loop_filter_params lfp;
   struct cdef_params cdefp;
   struct loop_restoration_params lrp;
   struct tx_mode_params tm;
   enum reference_mode reference_select;
   struct skip_mode_params smp;
   bool allow_warped_motion;
   bool reduced_tx_set;
   struct global_motion_params gmp;
   struct film_grain_params fgp;
};

struct dec_av1_task {
   struct list_head list;

   struct pipe_video_buffer *buf;
   bool no_show_frame;
   unsigned buf_ref_count;
   struct pipe_video_buffer **buf_ref;
   bool is_sef_task;
};

struct input_buf_private {
   struct list_head tasks;
   struct list_head inps;
};

struct dec_av1 {
   struct av1_obu_extension_header_obu ext;
   struct av1_sequence_header_obu seq;
   struct av1_uncompressed_header_obu uncompressed_header;
   struct av1_uncompressed_header_obu refs[AV1_NUM_REF_FRAMES];
   struct ref_frame RefFrames[AV1_NUM_REF_FRAMES];

   uint8_t bs_obu_td_buf[8];
   unsigned bs_obu_td_sz;
   uint8_t bs_obu_seq_buf[128];
   unsigned bs_obu_seq_sz;
   struct pipe_video_buffer *frame_refs[AV1_NUM_REF_FRAMES];
   struct list_head free_tasks;
   struct list_head started_tasks;
   struct list_head finished_tasks;
   struct list_head decode_tasks;
   unsigned que_num;
   bool stacked_frame;
   mtx_t mutex;
};

#endif
