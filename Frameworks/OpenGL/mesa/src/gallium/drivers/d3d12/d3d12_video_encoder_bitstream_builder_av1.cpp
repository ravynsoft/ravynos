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

#include "d3d12_video_encoder_bitstream_builder_av1.h"

void
d3d12_video_bitstream_builder_av1::write_obu_header(d3d12_video_encoder_bitstream *pBit,
                                                    av1_obutype_t obu_type,
                                                    uint32_t obu_extension_flag,
                                                    uint32_t temporal_id,
                                                    uint32_t spatial_id)
{
   pBit->put_bits(1, 0);          // obu_forbidden_bit
   pBit->put_bits(4, obu_type);   // type
   pBit->put_bits(1, obu_extension_flag);
   pBit->put_bits(1, 1);   // obu_has_size_field
   pBit->put_bits(1, 0);   // reserved
   if (obu_extension_flag) {
      // obu_extension_header()
      pBit->put_bits(3, temporal_id);
      pBit->put_bits(2, spatial_id);
      pBit->put_bits(3, 0);   // extension_header_reserved_3bits
   }
}

void
d3d12_video_bitstream_builder_av1::pack_obu_header_size(d3d12_video_encoder_bitstream *pBit, uint64_t val)
{
   pBit->put_leb128_bytes(val);
}

void
d3d12_video_bitstream_builder_av1::write_seq_data(d3d12_video_encoder_bitstream *pBit, const av1_seq_header_t *pSeqHdr)
{
   pBit->put_bits(3, pSeqHdr->seq_profile);
   pBit->put_bits(1, 0);   // still_picture default 0
   pBit->put_bits(1, 0);   // reduced_still_picture_header
   pBit->put_bits(1, 0);   // timing_info_present_flag
   pBit->put_bits(1, 0);   // initial_display_delay_present_flag

   pBit->put_bits(5, pSeqHdr->operating_points_cnt_minus_1);
   for (uint8_t i = 0; i <= pSeqHdr->operating_points_cnt_minus_1; i++) {
      pBit->put_bits(8, pSeqHdr->operating_point_idc[i] >> 4);
      pBit->put_bits(4, pSeqHdr->operating_point_idc[i] & 0x9f);
      pBit->put_bits(5, pSeqHdr->seq_level_idx[i]);
      if (pSeqHdr->seq_level_idx[i] > 7)
         pBit->put_bits(1, pSeqHdr->seq_tier[i]);
   }

   pBit->put_bits(4, d3d12_video_bitstream_builder_av1::frame_width_bits_minus_1);    // frame_width_bits_minus_1
   pBit->put_bits(4, d3d12_video_bitstream_builder_av1::frame_height_bits_minus_1);   // frame_height_bits_minus_1
   pBit->put_bits(d3d12_video_bitstream_builder_av1::frame_width_bits_minus_1 + 1,
                  pSeqHdr->max_frame_width - 1);   // max_frame_width_minus_1
   pBit->put_bits(d3d12_video_bitstream_builder_av1::frame_height_bits_minus_1 + 1,
                  pSeqHdr->max_frame_height - 1);            // max_frame_height_minus_1
   pBit->put_bits(1, 0);                                     // frame_id_numbers_present_flag
   pBit->put_bits(1, pSeqHdr->use_128x128_superblock);       // use_128x128_superblock
   pBit->put_bits(1, pSeqHdr->enable_filter_intra);          // enable_filter_intra
   pBit->put_bits(1, pSeqHdr->enable_intra_edge_filter);     // enable_intra_edge_filter
   pBit->put_bits(1, pSeqHdr->enable_interintra_compound);   // enable_interintra_compound
   pBit->put_bits(1, pSeqHdr->enable_masked_compound);       // enable_masked_compound
   pBit->put_bits(1, pSeqHdr->enable_warped_motion);         // enable_warped_motion
   pBit->put_bits(1, pSeqHdr->enable_dual_filter);           // enable_dual_filter
   pBit->put_bits(1, pSeqHdr->enable_order_hint);            // enable_order_hint

   if (pSeqHdr->enable_order_hint) {
      pBit->put_bits(1, pSeqHdr->enable_jnt_comp);        // enable_jnt_comp
      pBit->put_bits(1, pSeqHdr->enable_ref_frame_mvs);   // enable_ref_frame_mvs
   }

   pBit->put_bits(1, pSeqHdr->seq_choose_screen_content_tools);   // seq_choose_screen_content_tools
   if (!pSeqHdr->seq_choose_screen_content_tools)
      pBit->put_bits(1, pSeqHdr->seq_force_screen_content_tools);   // seq_force_screen_content_tools

   if (pSeqHdr->seq_force_screen_content_tools) {
      pBit->put_bits(1, pSeqHdr->seq_choose_integer_mv);   // seq_choose_integer_mv
      if (!pSeqHdr->seq_choose_integer_mv)
         pBit->put_bits(1, pSeqHdr->seq_force_integer_mv);   // seq_force_integer_mv
   }

   if (pSeqHdr->enable_order_hint)
      pBit->put_bits(3, pSeqHdr->order_hint_bits_minus1);

   pBit->put_bits(1, pSeqHdr->enable_superres);      // enable_superres
   pBit->put_bits(1, pSeqHdr->enable_cdef);          // enable_cdef
   pBit->put_bits(1, pSeqHdr->enable_restoration);   // enable_restoration

   // color_config ()
   pBit->put_bits(1,
                  pSeqHdr->color_config.bit_depth == DXGI_FORMAT_P010 ? 1 : 0);   // Assume DXGI_FORMAT_NV12 otherwise
   if (pSeqHdr->seq_profile != 1)
      pBit->put_bits(1, 0);   // mono_chrome not supported

   pBit->put_bits(1, pSeqHdr->color_config.color_description_present_flag);

   if (pSeqHdr->color_config.color_description_present_flag) {
      pBit->put_bits(8, pSeqHdr->color_config.color_primaries);
      pBit->put_bits(8, pSeqHdr->color_config.transfer_characteristics);
      pBit->put_bits(8, pSeqHdr->color_config.matrix_coefficients);
   }

   pBit->put_bits(1, pSeqHdr->color_config.color_range);   // color_range

   if (pSeqHdr->seq_profile == 0)
      pBit->put_bits(2, pSeqHdr->color_config.chroma_sample_position);   // chroma_sample_position

   pBit->put_bits(1, pSeqHdr->color_config.separate_uv_delta_q);   // separate_uv_delta_q

   pBit->put_bits(1, 0);   // film_grain_params_present

   pBit->put_trailing_bits();
}

void
d3d12_video_bitstream_builder_av1::write_temporal_delimiter_obu(std::vector<uint8_t> &headerBitstream,
                                                                std::vector<uint8_t>::iterator placingPositionStart,
                                                                size_t &writtenBytes)
{
   auto startByteOffset = std::distance(headerBitstream.begin(), placingPositionStart);
   if (headerBitstream.size() < (startByteOffset + c_DefaultBitstreamBufSize))
      headerBitstream.resize(startByteOffset + c_DefaultBitstreamBufSize);

   d3d12_video_encoder_bitstream bitstream_full_obu;
   bitstream_full_obu.setup_bitstream(headerBitstream.size(), headerBitstream.data(), startByteOffset);

   {
      // temporal_delimiter_obu() has empty payload as per AV1 codec spec

      // Write the header
      constexpr uint32_t obu_extension_flag = 0;
      constexpr uint32_t temporal_id = 0;
      constexpr uint32_t spatial_id = 0;
      write_obu_header(&bitstream_full_obu, OBU_TEMPORAL_DELIMITER, obu_extension_flag, temporal_id, spatial_id);

      // Write the data size
      const uint64_t obu_size_in_bytes = 0;
      debug_printf("obu_size: %" PRIu64 " (temporal_delimiter_obu() has empty payload as per AV1 codec spec)\n",
                   obu_size_in_bytes);
      pack_obu_header_size(&bitstream_full_obu, obu_size_in_bytes);
   }

   bitstream_full_obu.flush();

   // Shrink headerBitstream to fit
   writtenBytes = bitstream_full_obu.get_byte_count() - startByteOffset;
   headerBitstream.resize(writtenBytes + startByteOffset);
}

void
d3d12_video_bitstream_builder_av1::write_sequence_header(const av1_seq_header_t *pSeqHdr,
                                                         std::vector<uint8_t> &headerBitstream,
                                                         std::vector<uint8_t>::iterator placingPositionStart,
                                                         size_t &writtenBytes)
{
   auto startByteOffset = std::distance(headerBitstream.begin(), placingPositionStart);
   if (headerBitstream.size() < (startByteOffset + c_DefaultBitstreamBufSize))
      headerBitstream.resize(startByteOffset + c_DefaultBitstreamBufSize);

   d3d12_video_encoder_bitstream bitstream_full_obu;
   bitstream_full_obu.setup_bitstream(headerBitstream.size(), headerBitstream.data(), startByteOffset);

   // to handle variable length we first write the content
   // and later the obu header and concatenate both bitstreams
   d3d12_video_encoder_bitstream bitstream_seq;
   bitstream_seq.create_bitstream(c_DefaultBitstreamBufSize);

   {
      // Write the data
      write_seq_data(&bitstream_seq, pSeqHdr);
      bitstream_seq.flush();
      debug_printf("sequence_header_obu() bytes: %" PRId32 "\n", bitstream_seq.get_byte_count());

      // Write the header
      constexpr uint32_t obu_extension_flag = 0;
      constexpr uint32_t temporal_id = 0;
      constexpr uint32_t spatial_id = 0;
      write_obu_header(&bitstream_full_obu, OBU_SEQUENCE_HEADER, obu_extension_flag, temporal_id, spatial_id);

      // Write the data size
      const uint64_t obu_size_in_bytes = bitstream_seq.get_byte_count();
      debug_printf("obu_size: %" PRIu64 "\n", obu_size_in_bytes);
      pack_obu_header_size(&bitstream_full_obu, obu_size_in_bytes);

      bitstream_full_obu.flush();

      // bitstream_full_obu has external buffer allocation and
      // append_bitstream deep copies bitstream_seq, so it's okay
      // for RAII of bitstream_seq to be deallocated out of scope
      bitstream_full_obu.append_byte_stream(&bitstream_seq);
   }

   bitstream_full_obu.flush();

   // Shrink headerBitstream to fit
   writtenBytes = bitstream_full_obu.get_byte_count() - startByteOffset;
   headerBitstream.resize(writtenBytes + startByteOffset);
}

void
d3d12_video_bitstream_builder_av1::write_frame_size_with_refs(d3d12_video_encoder_bitstream *pBit,
                                                              const av1_seq_header_t *pSeqHdr,
                                                              const av1_pic_header_t *pPicHdr)
{
   bool found_ref = false;   // Send explicitly as default
   for (int i = 0; i < 7 /*REFS_PER_FRAME*/; i++) {
      pBit->put_bits(1, found_ref);   // found_ref
   }

   if (found_ref) {
      // frame_size()
      write_frame_size(pBit, pSeqHdr, pPicHdr);
      // render_size()
      write_render_size(pBit, pPicHdr);
   } else {
      // superres_params()
      write_superres_params(pBit, pSeqHdr, pPicHdr);
   }
}

void
d3d12_video_bitstream_builder_av1::write_frame_size(d3d12_video_encoder_bitstream *pBit,
                                                    const av1_seq_header_t *pSeqHdr,
                                                    const av1_pic_header_t *pPicHdr)
{
   if (pPicHdr->frame_size_override_flag) {
      pBit->put_bits(d3d12_video_bitstream_builder_av1::frame_width_bits_minus_1 + 1,
                     pPicHdr->FrameWidth - 1);   // frame_width_minus_1
      pBit->put_bits(d3d12_video_bitstream_builder_av1::frame_height_bits_minus_1 + 1,
                     pPicHdr->FrameHeight - 1);   // frame_height_minus_1
   }
   // superres_params()
   write_superres_params(pBit, pSeqHdr, pPicHdr);
}


void
d3d12_video_bitstream_builder_av1::write_superres_params(d3d12_video_encoder_bitstream *pBit,
                                                         const av1_seq_header_t *pSeqHdr,
                                                         const av1_pic_header_t *pPicHdr)
{
   if (pSeqHdr->enable_superres)
      pBit->put_bits(1, pPicHdr->use_superres);   // use_superres

   constexpr unsigned SUPERRES_DENOM_BITS = 3;   // As per AV1 codec spec
   if (pPicHdr->use_superres) {
      constexpr uint32_t SUPERRES_DENOM_MIN = 9;   // As per AV1 codec spec
      assert(pPicHdr->SuperresDenom >= SUPERRES_DENOM_MIN);
      uint32_t coded_denom = pPicHdr->SuperresDenom - SUPERRES_DENOM_MIN;
      pBit->put_bits(SUPERRES_DENOM_BITS, coded_denom);
   }
}

void
d3d12_video_bitstream_builder_av1::write_render_size(d3d12_video_encoder_bitstream *pBit,
                                                     const av1_pic_header_t *pPicHdr)
{
   uint8_t render_and_frame_size_different =
      ((pPicHdr->RenderWidth != pPicHdr->FrameWidth) || (pPicHdr->RenderHeight != pPicHdr->FrameHeight)) ? 1 : 0;

   pBit->put_bits(1, render_and_frame_size_different);   // render_and_frame_size_different

   if (render_and_frame_size_different == 1) {
      pBit->put_bits(16, pPicHdr->RenderWidth - 1);    // render_width_minus_1
      pBit->put_bits(16, pPicHdr->RenderHeight - 1);   // render_height_minus_1
   }
}

void
d3d12_video_bitstream_builder_av1::write_delta_q_value(d3d12_video_encoder_bitstream *pBit, int32_t delta_q_val)
{
   if (delta_q_val) {
      pBit->put_bits(1, 1);
      pBit->put_su_bits(7, delta_q_val);
   } else {
      pBit->put_bits(1, 0);
   }
}

inline int
get_relative_dist(int a, int b, int OrderHintBits, uint8_t enable_order_hint)
{
   if (!enable_order_hint)
      return 0;
   int diff = a - b;
   int m = 1 << (OrderHintBits - 1);
   diff = (diff & (m - 1)) - (diff & m);
   return diff;
}

static uint32_t
tile_log2(uint32_t blkSize, uint32_t target)
{
   uint32_t k = 0;
   for (k = 0; (blkSize << k) < target; k++);
   return k;
}

void
d3d12_video_bitstream_builder_av1::write_pic_data(d3d12_video_encoder_bitstream *pBit,
                                                  const av1_seq_header_t *pSeqHdr,
                                                  const av1_pic_header_t *pPicHdr)
{
   // uncompressed_header()

   pBit->put_bits(1, pPicHdr->show_existing_frame);

   if (pPicHdr->show_existing_frame) {
      pBit->put_bits(3, pPicHdr->frame_to_show_map_idx);   // frame_to_show_map_idx	f(3)

      // decoder_model_info_present_flag Default 0
      // if ( decoder_model_info_present_flag && !equal_picture_interval ) {
      //       temporal_point_info( )
      // }

      // frame_id_numbers_present_flag default 0
      // if ( frame_id_numbers_present_flag ) {
      //       display_frame_id	f(idLen)
      // }
   } else {

      const uint8_t FrameIsIntra = (pPicHdr->frame_type == D3D12_VIDEO_ENCODER_AV1_FRAME_TYPE_INTRA_ONLY_FRAME ||
                                    pPicHdr->frame_type == D3D12_VIDEO_ENCODER_AV1_FRAME_TYPE_KEY_FRAME);
      pBit->put_bits(2, pPicHdr->frame_type);   // frame_type

      pBit->put_bits(1, pPicHdr->show_frame);   // show_frame
      if (!pPicHdr->show_frame)
         pBit->put_bits(1, pPicHdr->showable_frame);   // showable_frame


      if (pPicHdr->frame_type == D3D12_VIDEO_ENCODER_AV1_FRAME_TYPE_SWITCH_FRAME ||
          (pPicHdr->frame_type == D3D12_VIDEO_ENCODER_AV1_FRAME_TYPE_KEY_FRAME && pPicHdr->show_frame)) {
         assert(pPicHdr->error_resilient_mode == 1);
      } else {
         pBit->put_bits(1, pPicHdr->error_resilient_mode);   // error_resilient_mode
      }

      pBit->put_bits(1, pPicHdr->disable_cdf_update);   // disable_cdf_update
      if (pSeqHdr->seq_force_screen_content_tools == /*SELECT_SCREEN_CONTENT_TOOLS */ 2)
         pBit->put_bits(1, pPicHdr->allow_screen_content_tools);   // allow_screen_content_tools

      if (pPicHdr->allow_screen_content_tools && (pSeqHdr->seq_force_integer_mv == /*SELECT_INTEGER_MV */ 2))
         pBit->put_bits(1, pPicHdr->force_integer_mv);   // force_integer_mv

      // reduced_still_picture_header default 0 and frame_type != SWITCH
      if (pPicHdr->frame_type != D3D12_VIDEO_ENCODER_AV1_FRAME_TYPE_SWITCH_FRAME) {
         // Expicitly coded if NOT SWITCH FRAME
         pBit->put_bits(1, pPicHdr->frame_size_override_flag);   // frame_size_override_flag
      } else {
         assert(pPicHdr->frame_size_override_flag ==
                1);   // As per AV1 spec for SWITCH FRAME it's not coded but defaulted to 1 instead
      }

      pBit->put_bits(pSeqHdr->order_hint_bits_minus1 + 1, pPicHdr->order_hint);   // order_hint

      if (!(FrameIsIntra || pPicHdr->error_resilient_mode))
         pBit->put_bits(3, pPicHdr->primary_ref_frame);   // primary_ref_frame

      // decoder_model_info_present_flag Default 0

      if (!(pPicHdr->frame_type == D3D12_VIDEO_ENCODER_AV1_FRAME_TYPE_SWITCH_FRAME ||
            (pPicHdr->frame_type == D3D12_VIDEO_ENCODER_AV1_FRAME_TYPE_KEY_FRAME && pPicHdr->show_frame)))
         pBit->put_bits(8 /* NUM_REF_FRAMES from AV1 spec */, pPicHdr->refresh_frame_flags);

      constexpr uint32_t allFrames = (1 << 8 /* NUM_REF_FRAMES from AV1 spec */) - 1;
      if (!FrameIsIntra || pPicHdr->refresh_frame_flags != allFrames) {

         if (pPicHdr->error_resilient_mode && pSeqHdr->enable_order_hint) {
            for (uint8_t i = 0; i < 8 /* NUM_REF_FRAMES from AV1 spec */; i++) {
               pBit->put_bits(pSeqHdr->order_hint_bits_minus1 + 1,
                              pPicHdr->ref_order_hint[i]);   // ref_order_hint[i] f(OrderHintBits)
            }
         }
      }

      if (FrameIsIntra) {
         // frame_size()
         write_frame_size(pBit, pSeqHdr, pPicHdr);
         // render_size()
         write_render_size(pBit, pPicHdr);

         if (pPicHdr->allow_screen_content_tools && pPicHdr->UpscaledWidth == pPicHdr->FrameWidth)
            pBit->put_bits(1, pPicHdr->allow_intrabc);
      } else {
         if (pSeqHdr->enable_order_hint)
            pBit->put_bits(1, 0);   // frame_refs_short_signaling default 0

         for (uint8_t ref = 0; ref < ARRAY_SIZE(pPicHdr->ref_frame_idx); ref++)
            pBit->put_bits(3 /* log2 of NUM_REF_FRAMES from AV1 spec */, pPicHdr->ref_frame_idx[ref]);

         // frame_id_numbers_present_flag default 0

         if (pPicHdr->frame_size_override_flag && !pPicHdr->error_resilient_mode) {
            // frame_size_with_refs()
            write_frame_size_with_refs(pBit, pSeqHdr, pPicHdr);
         } else {
            // frame_size()
            write_frame_size(pBit, pSeqHdr, pPicHdr);
            // render_size()
            write_render_size(pBit, pPicHdr);
         }

         if (!pPicHdr->force_integer_mv)
            pBit->put_bits(1, pPicHdr->allow_high_precision_mv);   // allow_high_precision_mv

         // read_interpolation_filter()
         {
            const uint8_t is_filter_switchable =
               (pPicHdr->interpolation_filter == D3D12_VIDEO_ENCODER_AV1_INTERPOLATION_FILTERS_SWITCHABLE ? 1 : 0);
            pBit->put_bits(1, is_filter_switchable);   // is_filter_switchable
            if (!is_filter_switchable) {
               pBit->put_bits(2, pPicHdr->interpolation_filter);   // interpolation_filter
            }
         }

         pBit->put_bits(1, pPicHdr->is_motion_mode_switchable);   // is_motion_mode_switchable

         if (!(pPicHdr->error_resilient_mode || !pPicHdr->use_ref_frame_mvs))
            pBit->put_bits(1, 1);   // use_ref_frame_mvs
      }

      if (!pPicHdr->disable_cdf_update /* || reduced_still_picture_header default 0 */)
         pBit->put_bits(1, pPicHdr->disable_frame_end_update_cdf);   // disable_frame_end_update_cdf

      // tile_info()
      {
         unsigned maxTileWidthSb = pPicHdr->tile_info.tile_support_caps.MaxTileWidth;
         unsigned maxTileAreaSb = pPicHdr->tile_info.tile_support_caps.MaxTileArea;

         unsigned minLog2TileCols = tile_log2(maxTileWidthSb, pPicHdr->tile_info.tile_support_caps.MinTileCols);
         unsigned maxLog2TileCols = tile_log2(1, pPicHdr->tile_info.tile_support_caps.MaxTileCols);
         unsigned log2TileCols = tile_log2(1, pPicHdr->tile_info.tile_partition.ColCount);

         unsigned minLog2TileRows = tile_log2(1, pPicHdr->tile_info.tile_support_caps.MinTileRows);
         unsigned maxLog2TileRows = tile_log2(1, pPicHdr->tile_info.tile_support_caps.MaxTileRows);
         unsigned log2TileRows = tile_log2(1, pPicHdr->tile_info.tile_partition.RowCount);

         pBit->put_bits(1, pPicHdr->tile_info.uniform_tile_spacing_flag);   // uniform_tile_spacing_flag

         if (pPicHdr->tile_info.uniform_tile_spacing_flag) {
            for (unsigned i = minLog2TileCols; i < log2TileCols; i++)
               pBit->put_bits(1, 1);   // one increment_tile_cols_log2
            if (log2TileCols < maxLog2TileCols)
               pBit->put_bits(1, 0);   // zero increment_tile_cols_log2
            for (unsigned i = minLog2TileRows; i < log2TileRows; i++)
               pBit->put_bits(1, 1);   // increment_tile_rows_log2
            if (log2TileRows < maxLog2TileRows)
               pBit->put_bits(1, 0);   // increment_tile_rows_log2
         } else {
            unsigned sizeSb = 0;
            unsigned widestTileSb = 0;
            unsigned widthSb = pPicHdr->frame_width_sb;
            for (unsigned i = 0; i < pPicHdr->tile_info.tile_partition.ColCount; i++) {
               sizeSb = pPicHdr->tile_info.tile_partition.ColWidths[i];
               unsigned maxWidth = std::min(widthSb, maxTileWidthSb);
               pBit->put_ns_bits(maxWidth, sizeSb - 1);   // width_in_sbs_minus_1
               widestTileSb = std::max(sizeSb, widestTileSb);
               widthSb -= sizeSb;
            }

            unsigned maxTileHeightSb = std::max(maxTileAreaSb / widestTileSb, 1u);
            unsigned heightSb = pPicHdr->frame_height_sb;
            for (unsigned i = 0; i < pPicHdr->tile_info.tile_partition.RowCount; i++) {
               sizeSb = pPicHdr->tile_info.tile_partition.RowHeights[i];
               unsigned maxHeight = std::min(heightSb, maxTileHeightSb);
               pBit->put_ns_bits(maxHeight, sizeSb - 1);   // height_in_sbs_minus_1
               heightSb -= sizeSb;
            }
         }

         if (log2TileCols > 0 || log2TileRows > 0) {
            pBit->put_bits(log2TileRows + log2TileCols,
                           pPicHdr->tile_info.tile_partition.ContextUpdateTileId);   // f(TileRowsLog2 + TileColsLog2)
            pBit->put_bits(2, pPicHdr->tile_info.tile_support_caps.TileSizeBytesMinus1);   // tile_size_bytes_minus_1
                                                                                           // f(2)
         }
      }

      // quantization_params()
      {
         pBit->put_bits(8, pPicHdr->quantization_params.BaseQIndex);   // base_q_idx
         write_delta_q_value(pBit, pPicHdr->quantization_params.YDCDeltaQ);

         bool diff_uv_delta = false;
         if (pPicHdr->quantization_params.UDCDeltaQ != pPicHdr->quantization_params.VDCDeltaQ ||
             pPicHdr->quantization_params.UACDeltaQ != pPicHdr->quantization_params.VACDeltaQ)
            diff_uv_delta = true;

         if (diff_uv_delta)
            assert(pSeqHdr->color_config.separate_uv_delta_q == 1);

         if (pSeqHdr->color_config.separate_uv_delta_q)
            pBit->put_bits(1, diff_uv_delta);

         write_delta_q_value(pBit, pPicHdr->quantization_params.UDCDeltaQ);
         write_delta_q_value(pBit, pPicHdr->quantization_params.UACDeltaQ);

         if (diff_uv_delta) {
            write_delta_q_value(pBit, pPicHdr->quantization_params.VDCDeltaQ);
            write_delta_q_value(pBit, pPicHdr->quantization_params.VACDeltaQ);
         }

         pBit->put_bits(1, pPicHdr->quantization_params.UsingQMatrix);   // using_qmatrix
         if (pPicHdr->quantization_params.UsingQMatrix) {
            pBit->put_bits(4, pPicHdr->quantization_params.QMY);   // qm_y
            pBit->put_bits(4, pPicHdr->quantization_params.QMU);   // qm_u
            if (pSeqHdr->color_config.separate_uv_delta_q)
               pBit->put_bits(4, pPicHdr->quantization_params.QMV);   // qm_v
         }
      }

      // segmentation_params()
      {
         pBit->put_bits(1, pPicHdr->segmentation_enabled);   // segmentation_enabled
         if (pPicHdr->segmentation_enabled) {
            if (pPicHdr->primary_ref_frame != 7 /*PRIMARY_REF_NONE*/) {
               pBit->put_bits(1, pPicHdr->segmentation_config.UpdateMap);   // segmentation_update_map f(1)
               if (pPicHdr->segmentation_config.UpdateMap == 1)
                  pBit->put_bits(1, pPicHdr->segmentation_config.TemporalUpdate);   // segmentation_temporal_update f(1)
               pBit->put_bits(1, pPicHdr->segmentation_config.UpdateData);          // segmentation_update_data f(1)
            }

            if (pPicHdr->segmentation_config.UpdateData == 1) {
               const int av1_segmentation_feature_bits[8 /*SEG_LVL_MAX*/] = { 8, 6, 6, 6, 6, 3, 0, 0 };
               const int av1_segmentation_feature_signed[8 /*SEG_LVL_MAX*/] = { 1, 1, 1, 1, 1, 0, 0, 0 };

               for (int i = 0; i < 8 /*MAX_SEGMENTS*/; i++) {
                  for (int j = 0; j < 8 /*SEG_LVL_MAX*/; j++) {
                     bool feature_enabled =
                        ((static_cast<UINT>(1 << j) & static_cast<UINT>(pPicHdr->segmentation_config.SegmentsData[i].EnabledFeatures)) != 0);
                     pBit->put_bits(1, feature_enabled ? 1 : 0);   // feature_enabled	f(1)

                     if (feature_enabled) {
                        int bitsToRead = av1_segmentation_feature_bits[j];
                        if (av1_segmentation_feature_signed[j] == 1) {
                           pBit->put_su_bits(
                              1 + bitsToRead,
                              pPicHdr->segmentation_config.SegmentsData[i].FeatureValue[j]);   // su(1+bitsToRead)
                        } else {
                           pBit->put_bits(
                              bitsToRead,
                              pPicHdr->segmentation_config.SegmentsData[i].FeatureValue[j]);   // f(bitsToRead)
                        }
                     }
                  }
               }
            }
         }
      }

      // delta_q_params()
      // combined with delta_lf_params()
      {
         if (pPicHdr->quantization_params.BaseQIndex)
            pBit->put_bits(1, pPicHdr->delta_q_params.DeltaQPresent);   // delta_q_present
         if (pPicHdr->delta_q_params.DeltaQPresent) {
            pBit->put_bits(2, pPicHdr->delta_q_params.DeltaQRes);   // delta_q_res

            // delta_lf_params()
            if (!pPicHdr->allow_intrabc) {
               pBit->put_bits(1, pPicHdr->delta_lf_params.DeltaLFPresent);   // delta_lf_present
               if (pPicHdr->delta_lf_params.DeltaLFPresent) {
                  pBit->put_bits(2, pPicHdr->delta_lf_params.DeltaLFRes);     // delta_lf_res
                  pBit->put_bits(1, pPicHdr->delta_lf_params.DeltaLFMulti);   // delta_lf_multi
               }
            }
         }
      }

      constexpr bool CodedLossless = false;   // CodedLossless default 0
      constexpr bool AllLossless = false;     // AllLossless default 0
      // loop_filter_params()
      {
         if (!(CodedLossless || pPicHdr->allow_intrabc)) {
            pBit->put_bits(6, pPicHdr->loop_filter_params.LoopFilterLevel[0]);   // loop_filter_level[0]
            pBit->put_bits(6, pPicHdr->loop_filter_params.LoopFilterLevel[1]);   // loop_filter_level[1]

            if (pPicHdr->loop_filter_params.LoopFilterLevel[0] || pPicHdr->loop_filter_params.LoopFilterLevel[1]) {
               pBit->put_bits(6, pPicHdr->loop_filter_params.LoopFilterLevelU);   // loop_filter_level[2]
               pBit->put_bits(6, pPicHdr->loop_filter_params.LoopFilterLevelV);   // loop_filter_level[3]
            }

            pBit->put_bits(3, pPicHdr->loop_filter_params.LoopFilterSharpnessLevel);   // loop_filter_sharpness
            pBit->put_bits(1, pPicHdr->loop_filter_params.LoopFilterDeltaEnabled);     // loop_filter_delta_enabled

            if (pPicHdr->loop_filter_params.LoopFilterDeltaEnabled) {
               bool loop_filter_delta_update =
                  (pPicHdr->loop_filter_params.UpdateRefDelta || pPicHdr->loop_filter_params.UpdateModeDelta);
               pBit->put_bits(1, loop_filter_delta_update);   // loop_filter_delta_update
               if (loop_filter_delta_update) {
                  constexpr uint8_t TOTAL_REFS_PER_FRAME = 8;   // From AV1 spec
                  static_assert(ARRAY_SIZE(pPicHdr->loop_filter_params.RefDeltas) == TOTAL_REFS_PER_FRAME);
                  for (uint8_t i = 0; i < TOTAL_REFS_PER_FRAME; i++) {
                     pBit->put_bits(1, pPicHdr->loop_filter_params.UpdateRefDelta);   // loop_filter_delta_update
                     if (pPicHdr->loop_filter_params.UpdateRefDelta) {
                        pBit->put_su_bits(7, pPicHdr->loop_filter_params.RefDeltas[i]);   // loop_filter_ref_deltas[i]
                     }
                  }

                  static_assert(ARRAY_SIZE(pPicHdr->loop_filter_params.ModeDeltas) == 2);   // From AV1 spec
                  for (uint8_t i = 0; i < 2; i++) {
                     pBit->put_bits(1, pPicHdr->loop_filter_params.UpdateModeDelta);   // update_mode_delta
                     if (pPicHdr->loop_filter_params.UpdateModeDelta) {
                        pBit->put_su_bits(7, pPicHdr->loop_filter_params.ModeDeltas[i]);   // loop_filter_mode_deltas[i]
                     }
                  }
               }
            }
         }
      }

      // cdef_params()
      {
         if (!(!pSeqHdr->enable_cdef || CodedLossless || pPicHdr->allow_intrabc)) {
            uint16_t num_planes = 3;                                     // mono_chrome not supported
            pBit->put_bits(2, pPicHdr->cdef_params.CdefDampingMinus3);   // cdef_damping_minus_3
            pBit->put_bits(2, pPicHdr->cdef_params.CdefBits);            // cdef_bits
            for (uint16_t i = 0; i < (1 << pPicHdr->cdef_params.CdefBits); ++i) {
               pBit->put_bits(4, pPicHdr->cdef_params.CdefYPriStrength[i]);   // cdef_y_pri_strength[i]
               pBit->put_bits(2, pPicHdr->cdef_params.CdefYSecStrength[i]);   // cdef_y_sec_strength[i]
               if (num_planes > 1) {
                  pBit->put_bits(4, pPicHdr->cdef_params.CdefUVPriStrength[i]);   // cdef_uv_pri_strength[i]
                  pBit->put_bits(2, pPicHdr->cdef_params.CdefUVSecStrength[i]);   // cdef_uv_sec_strength[i]
               }
            }
         }
      }

      // lr_params()
      {
         if (!(AllLossless || pPicHdr->allow_intrabc || !pSeqHdr->enable_restoration)) {
            bool uses_lr = false;
            bool uses_chroma_lr = false;
            for (int i = 0; i < 3 /*MaxNumPlanes*/; i++) {
               pBit->put_bits(2, pPicHdr->lr_params.lr_type[i]);
               if (pPicHdr->lr_params.lr_type[i] != D3D12_VIDEO_ENCODER_AV1_RESTORATION_TYPE_DISABLED) {
                  uses_lr = true;
                  if (i > 0)
                     uses_chroma_lr = true;
               }
            }

            if (uses_lr) {
               pBit->put_bits(1, pPicHdr->lr_params.lr_unit_shift);

               if (!pSeqHdr->use_128x128_superblock && pPicHdr->lr_params.lr_unit_shift) {
                  pBit->put_bits(1, pPicHdr->lr_params.lr_unit_extra_shift);
               }

               if (pSeqHdr->color_config.subsampling_x && pSeqHdr->color_config.subsampling_y && uses_chroma_lr) {
                  pBit->put_bits(1, pPicHdr->lr_params.lr_uv_shift);
               }
            }
         }
      }

      // read_tx_mode()
      {
         const uint8_t tx_mode_select = (pPicHdr->TxMode == D3D12_VIDEO_ENCODER_AV1_TX_MODE_SELECT) ? 1 : 0;
         if (!CodedLossless)
            pBit->put_bits(1, tx_mode_select);   // tx_mode_select
      }

      // frame_reference_mode()
      {
         if (!FrameIsIntra)
            pBit->put_bits(1, pPicHdr->reference_select);   // reference_select
      }

      // skip_mode_params()
      {
         uint8_t skipModeAllowed = 0;
         if (!(FrameIsIntra || !pPicHdr->reference_select || !pSeqHdr->enable_order_hint)) {
            int forwardIdx = -1;
            int backwardIdx = -1;
            int forwardHint = 0;
            int backwardHint = 0;
            for (int i = 0; i < 7 /*REFS_PER_FRAME*/; i++) {
               uint32_t refHint = pPicHdr->ref_order_hint[pPicHdr->ref_frame_idx[i]];
               if (get_relative_dist(refHint,
                                     pPicHdr->order_hint,
                                     pSeqHdr->order_hint_bits_minus1 + 1,
                                     pSeqHdr->enable_order_hint) < 0) {
                  if (forwardIdx < 0 || get_relative_dist(refHint,
                                                          forwardHint,
                                                          pSeqHdr->order_hint_bits_minus1 + 1,
                                                          pSeqHdr->enable_order_hint) > 0) {
                     forwardIdx = i;
                     forwardHint = refHint;
                  }
               } else if (get_relative_dist(refHint,
                                            pPicHdr->order_hint,
                                            pSeqHdr->order_hint_bits_minus1 + 1,
                                            pSeqHdr->enable_order_hint) > 0) {
                  if (backwardIdx < 0 || get_relative_dist(refHint,
                                                           backwardHint,
                                                           pSeqHdr->order_hint_bits_minus1 + 1,
                                                           pSeqHdr->enable_order_hint) < 0) {
                     backwardIdx = i;
                     backwardHint = refHint;
                  }
               }
            }
            if (forwardIdx < 0) {
               skipModeAllowed = 0;
            } else if (backwardIdx >= 0) {
               skipModeAllowed = 1;
            } else {
               int secondForwardIdx = -1;
               int secondForwardHint = 0;
               for (int i = 0; i < 7 /*REFS_PER_FRAME*/; i++) {
                  uint32_t refHint = pPicHdr->ref_order_hint[pPicHdr->ref_frame_idx[i]];
                  if (get_relative_dist(refHint,
                                        forwardHint,
                                        pSeqHdr->order_hint_bits_minus1 + 1,
                                        pSeqHdr->enable_order_hint) < 0) {
                     if (secondForwardIdx < 0 || get_relative_dist(refHint,
                                                                   secondForwardHint,
                                                                   pSeqHdr->order_hint_bits_minus1 + 1,
                                                                   pSeqHdr->enable_order_hint) > 0) {
                        secondForwardIdx = i;
                        secondForwardHint = refHint;
                     }
                  }
               }
               if (secondForwardIdx < 0) {
                  skipModeAllowed = 0;
               } else {
                  skipModeAllowed = 1;
               }
            }
         }

         if (skipModeAllowed)
            pBit->put_bits(1, pPicHdr->skip_mode_present);   // skip_mode_present

         if (!(FrameIsIntra || pPicHdr->error_resilient_mode || !pSeqHdr->enable_warped_motion)) {
            pBit->put_bits(1, pPicHdr->allow_warped_motion);   // allow_warped_motion
         }
      }

      pBit->put_bits(1, pPicHdr->reduced_tx_set);   // reduced_tx_set

      // global_motion_params()
      {
         if (!FrameIsIntra) {
            for (uint8_t i = 0; i < 7; i++) {
               pBit->put_bits(1, 0);   // is_global[7]
               // Unimplemented: Enable global_motion_params with ref_global_motion_info
               assert(pPicHdr->ref_global_motion_info[i].TransformationType ==
                      D3D12_VIDEO_ENCODER_AV1_REFERENCE_WARPED_MOTION_TRANSFORMATION_IDENTITY);
            }
         }
      }

      // film_grain_params()
      // constexpr uint8_t film_grain_params_present = 0; // film_grain_params_present default 0
      // {
      // if (!(!film_grain_params_present || (!pPicHdr->show_frame && !pPicHdr->showable_frame))
      // ... this will be unreachable as film_grain_params_present is zero.
      // }
   }
}

void
d3d12_video_bitstream_builder_av1::write_frame_header(const av1_seq_header_t *pSeqHdr,
                                                      const av1_pic_header_t *pPicHdr,
                                                      av1_obutype_t frame_pack_type,
                                                      size_t extra_obu_size_bytes,
                                                      std::vector<uint8_t> &headerBitstream,
                                                      std::vector<uint8_t>::iterator placingPositionStart,
                                                      size_t &writtenBytes)
{
   assert((frame_pack_type == OBU_FRAME) || (frame_pack_type == OBU_FRAME_HEADER));
   auto startByteOffset = std::distance(headerBitstream.begin(), placingPositionStart);
   if (headerBitstream.size() < (startByteOffset + c_DefaultBitstreamBufSize))
      headerBitstream.resize(startByteOffset + c_DefaultBitstreamBufSize);

   d3d12_video_encoder_bitstream bitstream_full_obu;
   bitstream_full_obu.setup_bitstream(headerBitstream.size(), headerBitstream.data(), startByteOffset);

   // to handle variable length we first write the content
   // and later the obu header and concatenate both bitstreams
   d3d12_video_encoder_bitstream bitstream_pic;
   bitstream_pic.create_bitstream(c_DefaultBitstreamBufSize);

   {
      // Write frame_header_obu()
      write_pic_data(&bitstream_pic, pSeqHdr, pPicHdr);

      debug_printf("frame_header_obu() bytes (without OBU_FRAME nor OBU_FRAME_HEADER alignment padding): %" PRId32 "\n",
                   bitstream_pic.get_byte_count());   // May be bit unaligned at this point (see padding below)
      debug_printf("extra_obu_size_bytes (ie. tile_group_obu_size if writing OBU_FRAME ): %" PRIu64 "\n",
                   static_cast<uint64_t>(extra_obu_size_bytes));

      // Write the obu_header
      constexpr uint32_t obu_extension_flag = 0;
      constexpr uint32_t temporal_id = 0;
      constexpr uint32_t spatial_id = 0;
      write_obu_header(&bitstream_full_obu, frame_pack_type, obu_extension_flag, temporal_id, spatial_id);

      if (frame_pack_type == OBU_FRAME) {
         // Required byte_alignment() in frame_obu() after frame_header_obu()
         bitstream_pic.put_aligning_bits();
         debug_printf("Adding byte_alignment() after frame_header_obu() for OBU_FRAME\n");
      } else if (frame_pack_type == OBU_FRAME_HEADER) {
         // whole open_bitstream_unit() for OBU_FRAME_HEADER
         // required in open_bitstream_unit () for OBU_FRAME_HEADER
         bitstream_pic.put_trailing_bits();
         debug_printf("Adding trailing_bits() after frame_header_obu() for OBU_FRAME\n");
         assert(extra_obu_size_bytes == 0);
      }

      bitstream_pic.flush();

      // Write the obu_size element
      const uint64_t obu_size_in_bytes = bitstream_pic.get_byte_count() + extra_obu_size_bytes;
      debug_printf("obu_size: %" PRIu64 "\n", obu_size_in_bytes);
      pack_obu_header_size(&bitstream_full_obu, obu_size_in_bytes);

      bitstream_full_obu.flush();

      // bitstream_full_obu has external buffer allocation and
      // append_bitstream deep copies bitstream_pic, so it's okay
      // for RAII of bitstream_pic to be deallocated out of scope
      bitstream_full_obu.append_byte_stream(&bitstream_pic);
   }

   bitstream_full_obu.flush();

   // Shrink headerBitstream to fit
   writtenBytes = bitstream_full_obu.get_byte_count() - startByteOffset;
   headerBitstream.resize(writtenBytes + startByteOffset);
}

void
d3d12_video_bitstream_builder_av1::calculate_tile_group_obu_size(
   const D3D12_VIDEO_ENCODER_OUTPUT_METADATA *pParsedMetadata,
   const D3D12_VIDEO_ENCODER_FRAME_SUBREGION_METADATA *pFrameSubregionMetadata,
   size_t TileSizeBytes,   // Pass already +1'd from TileSizeBytesMinus1
   const D3D12_VIDEO_ENCODER_AV1_PICTURE_CONTROL_SUBREGIONS_LAYOUT_DATA_TILES &TilesPartition,
   const av1_tile_group_t &tileGroup,
   size_t &tile_group_obu_size,
   size_t &decode_tile_elements_size)
{
   size_t tile_group_obu_size_bits = 0;

   uint8_t NumTiles = TilesPartition.ColCount * TilesPartition.RowCount;
   if (NumTiles > 1)
      tile_group_obu_size_bits++;   // tile_start_and_end_present_flag	f(1)

   bool tile_start_and_end_present_flag = !(tileGroup.tg_start == 0 && (tileGroup.tg_end == (NumTiles - 1)));
   if (!(NumTiles == 1 || !tile_start_and_end_present_flag)) {
      uint8_t tileBits = tile_log2(1, TilesPartition.ColCount) + tile_log2(1, TilesPartition.RowCount);
      tile_group_obu_size_bits += tileBits;   // tg_start	f(tileBits)
      tile_group_obu_size_bits += tileBits;   // tg_end	   f(tileBits)
   }

   while (tile_group_obu_size_bits & 7)   // byte_alignment()
      tile_group_obu_size_bits++;

   decode_tile_elements_size = 0;
   for (UINT64 TileIdx = tileGroup.tg_start; TileIdx <= tileGroup.tg_end; TileIdx++) {
      // tile_size_minus_1	not coded for last tile
      if ((TileIdx != tileGroup.tg_end))
         tile_group_obu_size_bits += (TileSizeBytes * 8);   // tile_size_minus_1	le(TileSizeBytes)

      size_t tile_effective_bytes_size =
         static_cast<size_t>(pFrameSubregionMetadata[TileIdx].bSize - pFrameSubregionMetadata[TileIdx].bStartOffset);
      decode_tile_elements_size += tile_effective_bytes_size;
      tile_group_obu_size_bits += (tile_effective_bytes_size * 8);
   }

   assert((tile_group_obu_size_bits % 8) == 0);
   tile_group_obu_size = (tile_group_obu_size_bits / 8);
}

void
d3d12_video_bitstream_builder_av1::write_obu_tile_group_header(size_t tile_group_obu_size,
                                                               std::vector<uint8_t> &headerBitstream,
                                                               std::vector<uint8_t>::iterator placingPositionStart,
                                                               size_t &writtenBytes)
{
   auto startByteOffset = std::distance(headerBitstream.begin(), placingPositionStart);
   if (headerBitstream.size() < (startByteOffset + c_DefaultBitstreamBufSize))
      headerBitstream.resize(startByteOffset + c_DefaultBitstreamBufSize);

   d3d12_video_encoder_bitstream bitstream_full_obu;
   bitstream_full_obu.setup_bitstream(headerBitstream.size(), headerBitstream.data(), startByteOffset);

   // Write the obu_header
   constexpr uint32_t obu_extension_flag = 0;
   constexpr uint32_t temporal_id = 0;
   constexpr uint32_t spatial_id = 0;
   write_obu_header(&bitstream_full_obu, OBU_TILE_GROUP, obu_extension_flag, temporal_id, spatial_id);

   // tile_group_obu() will be copied by get_feedback from EncodeFrame output
   // we have to calculate its size anyways using the metadata for the obu_header.
   // so we just add below the argument tile_group_obu_size informing about the
   // tile_group_obu() byte size
   // For OBU_TILE_GROUP there is no padding/alignment requirement so they can be concatenated directly by get_feedback

   // Write the obu_size element
   pack_obu_header_size(&bitstream_full_obu, tile_group_obu_size);
   debug_printf("obu_size: %" PRIu64 "\n", static_cast<uint64_t>(tile_group_obu_size));

   bitstream_full_obu.flush();

   // Shrink headerBitstream to fit
   writtenBytes = bitstream_full_obu.get_byte_count() - startByteOffset;
   headerBitstream.resize(writtenBytes + startByteOffset);
}
