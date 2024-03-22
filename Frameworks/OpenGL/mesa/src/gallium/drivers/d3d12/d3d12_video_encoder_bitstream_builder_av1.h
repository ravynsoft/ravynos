/*
 * Copyright © Microsoft Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef D3D12_VIDEO_ENC_BITSTREAM_BUILDER_AV1_H
#define D3D12_VIDEO_ENC_BITSTREAM_BUILDER_AV1_H

#include "d3d12_video_encoder_bitstream_builder.h"
#include "d3d12_video_encoder_bitstream.h"

typedef struct av1_tile_group_t
{
   uint8_t tg_start;
   uint8_t tg_end;
} av1_tile_group_t;

typedef enum av1_obutype_t
{
   OBU_SEQUENCE_HEADER = 1,
   OBU_TEMPORAL_DELIMITER = 2,
   OBU_FRAME_HEADER = 3,
   OBU_TILE_GROUP = 4,
   OBU_METADATA = 5,
   OBU_FRAME = 6,
   OBU_REDUNDANT_FRAME_HEADER = 7,
   OBU_PADDING = 15,
} av1_obutype_t;

typedef struct av1_pic_tile_info_t
{
   uint32_t uniform_tile_spacing_flag;
   D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_SUBREGIONS_LAYOUT_DATA_TILES tile_partition;
   D3D12_VIDEO_ENCODER_FRAME_SUBREGION_LAYOUT_MODE tile_mode;
   D3D12_VIDEO_ENCODER_AV1_FRAME_SUBREGION_LAYOUT_CONFIG_SUPPORT tile_support_caps;
} av1_pic_tile_info_t;

typedef struct av1_pic_lr_params_t
{
   D3D12_VIDEO_ENCODER_AV1_RESTORATION_TYPE lr_type[3];
   uint32_t lr_unit_shift;
   uint32_t lr_uv_shift;
   uint32_t lr_unit_extra_shift;
} av1_pic_lr_params_t;

typedef struct av1_seq_color_config_t
{
   DXGI_FORMAT bit_depth;
   // uint32_t mono_chrome; // coded in bitstream by default as 0
   uint32_t color_primaries;
   uint32_t transfer_characteristics;
   uint32_t matrix_coefficients;
   uint32_t color_description_present_flag;
   uint32_t color_range;
   uint32_t chroma_sample_position;
   uint32_t subsampling_x;
   uint32_t subsampling_y;
   uint32_t separate_uv_delta_q;
} av1_seq_color_config_t;

typedef struct av1_pic_header_t
{
   uint32_t show_existing_frame;
   uint32_t frame_to_show_map_idx;
   // uint32_t display_frame_id; // frame_id_numbers_present_flag coded in bitstream by default as 0
   D3D12_VIDEO_ENCODER_AV1_FRAME_TYPE frame_type;
   uint32_t show_frame;
   uint32_t showable_frame;
   uint32_t error_resilient_mode;
   uint32_t disable_cdf_update;
   uint32_t allow_screen_content_tools;
   uint32_t force_integer_mv;
   uint32_t frame_size_override_flag;
   uint32_t order_hint;
   uint32_t ref_order_hint[8];
   uint32_t primary_ref_frame;
   uint8_t refresh_frame_flags;
   uint32_t FrameWidth;
   uint32_t FrameHeight;
   uint32_t frame_width_sb;
   uint32_t frame_height_sb;
   uint32_t use_superres;
   uint32_t SuperresDenom;
   uint32_t UpscaledWidth;
   uint32_t RenderWidth;
   uint32_t RenderHeight;
   uint32_t allow_intrabc;
   int32_t ref_frame_idx[7];
   D3D12_VIDEO_ENCODER_AV1_REFERENCE_PICTURE_WARPED_MOTION_INFO ref_global_motion_info[7];
   uint32_t allow_high_precision_mv;
   D3D12_VIDEO_ENCODER_AV1_INTERPOLATION_FILTERS interpolation_filter;
   uint32_t is_motion_mode_switchable;
   uint32_t use_ref_frame_mvs;
   uint32_t disable_frame_end_update_cdf;
   av1_pic_tile_info_t tile_info;
   D3D12_VIDEO_ENCODER_CODEC_AV1_QUANTIZATION_CONFIG quantization_params;
   D3D12_VIDEO_ENCODER_CODEC_AV1_LOOP_FILTER_DELTA_CONFIG delta_lf_params;
   D3D12_VIDEO_ENCODER_CODEC_AV1_QUANTIZATION_DELTA_CONFIG delta_q_params;
   // uint32_t CodedLossless; // coded in bitstream by default as 0
   uint32_t AllLossless;   // coded in bitstream by default as 0
   D3D12_VIDEO_ENCODER_CODEC_AV1_LOOP_FILTER_CONFIG loop_filter_params;
   D3D12_VIDEO_ENCODER_AV1_CDEF_CONFIG cdef_params;
   av1_pic_lr_params_t lr_params;
   D3D12_VIDEO_ENCODER_AV1_TX_MODE TxMode;
   uint32_t reference_select;
   uint32_t skip_mode_present;
   uint32_t allow_warped_motion;
   uint32_t reduced_tx_set;
   uint32_t segmentation_enabled;
   D3D12_VIDEO_ENCODER_AV1_SEGMENTATION_CONFIG segmentation_config;
   // uint32_t frame_refs_short_signaling; // coded in bitstream by default as 0
} av1_pic_header_t;

typedef struct av1_seq_header_t
{
   uint32_t seq_profile;
   // uint32_t still_picture; // coded in bitstream by default as 0
   // uint32_t reduced_still_picture_header; // coded in bitstream by default as 0
   // uint32_t timing_info_present_flag; // coded in bitstream by default as 0
   // uint32_t decoder_model_info_present_flag; // coded in bitstream by default as 0
   uint32_t operating_points_cnt_minus_1;
   uint32_t operating_point_idc[32];
   uint32_t seq_level_idx[32];
   uint32_t seq_tier[32];
   uint32_t decoder_model_present_for_this_op[32];
   // uint32_t initial_display_delay_present_flag; // coded in bitstream by default as 0
   // uint32_t initial_display_delay_minus_1[32];
   // uint32_t initial_display_delay_present_for_this_op[32]
   uint32_t max_frame_width;
   uint32_t max_frame_height;
   // uint32_t frame_id_numbers_present_flag; // coded in bitstream by default as 0
   uint32_t use_128x128_superblock;
   uint32_t enable_filter_intra;
   uint32_t enable_intra_edge_filter;
   uint32_t enable_interintra_compound;
   uint32_t enable_masked_compound;
   uint32_t enable_warped_motion;
   uint32_t enable_dual_filter;
   uint32_t enable_order_hint;
   uint32_t enable_jnt_comp;
   uint32_t enable_ref_frame_mvs;
   uint32_t seq_choose_screen_content_tools;
   uint32_t seq_force_screen_content_tools;
   uint32_t seq_choose_integer_mv;
   uint32_t seq_force_integer_mv;
   uint32_t order_hint_bits_minus1;
   uint32_t enable_superres;
   uint32_t enable_cdef;
   uint32_t enable_restoration;
   av1_seq_color_config_t color_config;
   // uint32_t film_grain_params_present; // coded in bitstream by default as 0
} av1_seq_header_t;

class d3d12_video_bitstream_builder_av1 : public d3d12_video_bitstream_builder_interface
{
 public:
   d3d12_video_bitstream_builder_av1()
   { }
   ~d3d12_video_bitstream_builder_av1()
   { }

   void write_temporal_delimiter_obu(std::vector<uint8_t> &headerBitstream,
                                     std::vector<uint8_t>::iterator placingPositionStart,
                                     size_t &writtenBytes);

   void write_sequence_header(const av1_seq_header_t *pSeqHdr,
                              std::vector<uint8_t> &headerBitstream,
                              std::vector<uint8_t>::iterator placingPositionStart,
                              size_t &writtenBytes);


   void write_frame_header(const av1_seq_header_t *pSeqHdr,
                           const av1_pic_header_t *pPicHdr,
                           av1_obutype_t frame_pack_type,
                           size_t extra_obu_size_bytes,
                           std::vector<uint8_t> &headerBitstream,
                           std::vector<uint8_t>::iterator placingPositionStart,
                           size_t &writtenBytes);

   void calculate_tile_group_obu_size(
      const D3D12_VIDEO_ENCODER_OUTPUT_METADATA *pParsedMetadata,
      const D3D12_VIDEO_ENCODER_FRAME_SUBREGION_METADATA *pFrameSubregionMetadata,
      size_t TileSizeBytes,   // Pass already +1'd from TileSizeBytesMinus1
      const D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_SUBREGIONS_LAYOUT_DATA_TILES &TilesPartition,
      const av1_tile_group_t &tileGroup,
      size_t &tile_group_obu_total_size,
      size_t &decode_tile_elements_size);

   void write_obu_tile_group_header(size_t tile_group_obu_size,
                                    std::vector<uint8_t> &headerBitstream,
                                    std::vector<uint8_t>::iterator placingPositionStart,
                                    size_t &writtenBytes);

 private:
   // static void write_delta_q_value(d3d12_video_encoder_bitstream& bitstream,
   //                               int32_t delta_q_val);
   const size_t c_DefaultBitstreamBufSize = 1024;

   void write_obu_header(d3d12_video_encoder_bitstream *pBit,
                         av1_obutype_t obu_type,
                         uint32_t obu_extension_flag,
                         uint32_t temporal_id,
                         uint32_t spatial_id);

   void pack_obu_header_size(d3d12_video_encoder_bitstream *pBit, uint64_t val);

   void write_seq_data(d3d12_video_encoder_bitstream *pBit, const av1_seq_header_t *pSeqHdr);

   void write_pic_data(d3d12_video_encoder_bitstream *pBit,
                       const av1_seq_header_t *pSeqHdr,
                       const av1_pic_header_t *pPicHdr);

   void write_delta_q_value(d3d12_video_encoder_bitstream *pBit, int32_t delta_q_val);

   void write_frame_size(d3d12_video_encoder_bitstream *pBit,
                         const av1_seq_header_t *pSeqHdr,
                         const av1_pic_header_t *pPicHdr);

   void write_frame_size_with_refs(d3d12_video_encoder_bitstream *pBit,
                                   const av1_seq_header_t *pSeqHdr,
                                   const av1_pic_header_t *pPicHdr);

   void write_render_size(d3d12_video_encoder_bitstream *pBit, const av1_pic_header_t *pPicHdr);

   void write_superres_params(d3d12_video_encoder_bitstream *pBit,
                              const av1_seq_header_t *pSeqHdr,
                              const av1_pic_header_t *pPicHdr);

   // Constants
   static const uint32_t frame_width_bits_minus_1 = 15;
   static const uint32_t frame_height_bits_minus_1 = 15;
};

#endif   // D3D12_VIDEO_ENC_BITSTREAM_BUILDER_AV1_H
