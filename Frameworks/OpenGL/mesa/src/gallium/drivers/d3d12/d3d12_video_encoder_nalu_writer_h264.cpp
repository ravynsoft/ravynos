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

#include "d3d12_video_encoder_nalu_writer_h264.h"
#include <algorithm>

void
d3d12_video_nalu_writer_h264::rbsp_trailing(d3d12_video_encoder_bitstream *pBitstream)
{
   pBitstream->put_bits(1, 1);
   int32_t iLeft = pBitstream->get_num_bits_for_byte_align();

   if (iLeft) {
      pBitstream->put_bits(iLeft, 0);
   }

   ASSERTED bool isAligned = pBitstream->is_byte_aligned();   // causes side-effects in object state, don't put inside assert()
   assert(isAligned);
}

uint32_t
d3d12_video_nalu_writer_h264::write_sps_bytes(d3d12_video_encoder_bitstream *pBitstream, H264_SPS *pSPS)
{
   int32_t iBytesWritten = pBitstream->get_byte_count();

   // Standard constraint to be between 0 and 31 inclusive
   assert(pSPS->seq_parameter_set_id >= 0);
   assert(pSPS->seq_parameter_set_id < 32);

   pBitstream->put_bits(8, pSPS->profile_idc);
   pBitstream->put_bits(6, pSPS->constraint_set_flags);
   pBitstream->put_bits(2, 0); // reserved_zero_2bits
   pBitstream->put_bits(8, pSPS->level_idc);
   pBitstream->exp_Golomb_ue(pSPS->seq_parameter_set_id);

   // If adding new profile support, check that the chroma_format_idc and bit depth are set correctly below
   // for the new additions
   assert((pSPS->profile_idc == H264_PROFILE_MAIN) ||
         (pSPS->profile_idc == H264_PROFILE_HIGH) ||
         (pSPS->profile_idc == H264_PROFILE_HIGH10) ||
         (pSPS->profile_idc == H264_PROFILE_BASELINE) ||
         (pSPS->profile_idc == H264_PROFILE_CONSTRAINED_BASELINE));

   if ((pSPS->profile_idc == H264_PROFILE_HIGH) || (pSPS->profile_idc == H264_PROFILE_HIGH10)) {
      // chroma_format_idc always 4.2.0
      pBitstream->exp_Golomb_ue(1);
      // Assume no separate_colour_plane_flag given chroma_format_idc = 1
      pBitstream->exp_Golomb_ue(pSPS->bit_depth_luma_minus8);
      pBitstream->exp_Golomb_ue(pSPS->bit_depth_chroma_minus8);
      // qpprime_y_zero_transform_bypass_flag
      pBitstream->put_bits(1, 0);
      // seq_scaling_matrix_present_flag)
      pBitstream->put_bits(1, 0);
   }

   pBitstream->exp_Golomb_ue(pSPS->log2_max_frame_num_minus4);

   pBitstream->exp_Golomb_ue(pSPS->pic_order_cnt_type);
   if (pSPS->pic_order_cnt_type == 0) {
      pBitstream->exp_Golomb_ue(pSPS->log2_max_pic_order_cnt_lsb_minus4);
   }
   pBitstream->exp_Golomb_ue(pSPS->max_num_ref_frames);
   pBitstream->put_bits(1, pSPS->gaps_in_frame_num_value_allowed_flag);
   pBitstream->exp_Golomb_ue(pSPS->pic_width_in_mbs_minus1);
   pBitstream->exp_Golomb_ue(pSPS->pic_height_in_map_units_minus1);

   // No support for interlace in D3D12 Video Encode
   // frame_mbs_only_flag coded as 1
   pBitstream->put_bits(1, 1);   // frame_mbs_only_flag
   pBitstream->put_bits(1, pSPS->direct_8x8_inference_flag);

   // no cropping
   pBitstream->put_bits(1, pSPS->frame_cropping_flag);   // frame_cropping_flag
   if (pSPS->frame_cropping_flag) {
      pBitstream->exp_Golomb_ue(pSPS->frame_cropping_rect_left_offset);
      pBitstream->exp_Golomb_ue(pSPS->frame_cropping_rect_right_offset);
      pBitstream->exp_Golomb_ue(pSPS->frame_cropping_rect_top_offset);
      pBitstream->exp_Golomb_ue(pSPS->frame_cropping_rect_bottom_offset);
   }

   pBitstream->put_bits(1, pSPS->vui_parameters_present_flag);
   if (pSPS->vui_parameters_present_flag)
   {
      pBitstream->put_bits(1, pSPS->vui.aspect_ratio_info_present_flag);
      if (pSPS->vui.aspect_ratio_info_present_flag) {
         pBitstream->put_bits(8, pSPS->vui.aspect_ratio_idc);
         if (pSPS->vui.aspect_ratio_idc == 255 /*EXTENDED_SAR*/) {
               pBitstream->put_bits(16, pSPS->vui.sar_width);
               pBitstream->put_bits(16, pSPS->vui.sar_height);
         }
      }

      pBitstream->put_bits(1, pSPS->vui.overscan_info_present_flag);
      if (pSPS->vui.overscan_info_present_flag) {
         pBitstream->put_bits(1, pSPS->vui.overscan_appropriate_flag);
      }

      pBitstream->put_bits(1, pSPS->vui.video_signal_type_present_flag);
      if (pSPS->vui.video_signal_type_present_flag) {
         pBitstream->put_bits(3, pSPS->vui.video_format);
         pBitstream->put_bits(1, pSPS->vui.video_full_range_flag);
         pBitstream->put_bits(1, pSPS->vui.colour_description_present_flag);
         if (pSPS->vui.colour_description_present_flag) {
               pBitstream->put_bits(8, pSPS->vui.colour_primaries);
               pBitstream->put_bits(8, pSPS->vui.transfer_characteristics);
               pBitstream->put_bits(8, pSPS->vui.matrix_coefficients);
         }
      }

      pBitstream->put_bits(1, pSPS->vui.chroma_loc_info_present_flag);
      if (pSPS->vui.chroma_loc_info_present_flag) {
         pBitstream->exp_Golomb_ue(pSPS->vui.chroma_sample_loc_type_top_field);
         pBitstream->exp_Golomb_ue(pSPS->vui.chroma_sample_loc_type_bottom_field);
      }

      pBitstream->put_bits(1, pSPS->vui.timing_info_present_flag);
      if (pSPS->vui.timing_info_present_flag) {
         pBitstream->put_bits(16, pSPS->vui.num_units_in_tick >> 16);
         pBitstream->put_bits(16, pSPS->vui.num_units_in_tick & 0xffff);
         pBitstream->put_bits(16, pSPS->vui.time_scale >> 16);
         pBitstream->put_bits(16, pSPS->vui.time_scale & 0xffff);
         pBitstream->put_bits(1, pSPS->vui.fixed_frame_rate_flag);
      }

      pBitstream->put_bits(1, pSPS->vui.nal_hrd_parameters_present_flag);
      if (pSPS->vui.nal_hrd_parameters_present_flag) {
         write_hrd(pBitstream, &pSPS->vui.nal_hrd_parameters);
      }

      pBitstream->put_bits(1, pSPS->vui.vcl_hrd_parameters_present_flag);
      if (pSPS->vui.vcl_hrd_parameters_present_flag) {
         write_hrd(pBitstream, &pSPS->vui.vcl_hrd_parameters);
      }

      if (pSPS->vui.nal_hrd_parameters_present_flag || pSPS->vui.vcl_hrd_parameters_present_flag) {
         pBitstream->put_bits(1, pSPS->vui.low_delay_hrd_flag);
      }

      pBitstream->put_bits(1, pSPS->vui.pic_struct_present_flag);
      pBitstream->put_bits(1, pSPS->vui.bitstream_restriction_flag);
      if (pSPS->vui.bitstream_restriction_flag) {
         pBitstream->put_bits(1, pSPS->vui.motion_vectors_over_pic_boundaries_flag);
         pBitstream->exp_Golomb_ue(pSPS->vui.max_bytes_per_pic_denom);
         pBitstream->exp_Golomb_ue(pSPS->vui.max_bits_per_mb_denom);
         pBitstream->exp_Golomb_ue(pSPS->vui.log2_max_mv_length_horizontal);
         pBitstream->exp_Golomb_ue(pSPS->vui.log2_max_mv_length_vertical);
         pBitstream->exp_Golomb_ue(pSPS->vui.num_reorder_frames);
         pBitstream->exp_Golomb_ue(pSPS->vui.max_dec_frame_buffering);
      }
   }

   rbsp_trailing(pBitstream);
   pBitstream->flush();

   iBytesWritten = pBitstream->get_byte_count() - iBytesWritten;
   return (uint32_t) iBytesWritten;
}

void
d3d12_video_nalu_writer_h264::write_hrd(d3d12_video_encoder_bitstream *pBitstream, H264_HRD_PARAMS *pHrd)
{
    pBitstream->exp_Golomb_ue(pHrd->cpb_cnt_minus1);
    pBitstream->put_bits(4, pHrd->bit_rate_scale);
    pBitstream->put_bits(4, pHrd->cpb_size_scale);
    for (uint32_t i = 0; i <= pHrd->cpb_cnt_minus1; i++) {
        pBitstream->exp_Golomb_ue(pHrd->bit_rate_value_minus1[i]);
        pBitstream->exp_Golomb_ue(pHrd->cpb_size_value_minus1[i]);
        pBitstream->put_bits(1, pHrd->cbr_flag[i]);
    }
    pBitstream->put_bits(5, pHrd->initial_cpb_removal_delay_length_minus1);
    pBitstream->put_bits(5, pHrd->cpb_removal_delay_length_minus1);
    pBitstream->put_bits(5, pHrd->dpb_output_delay_length_minus1);
    pBitstream->put_bits(5, pHrd->time_offset_length);
}

uint32_t
d3d12_video_nalu_writer_h264::write_pps_bytes(d3d12_video_encoder_bitstream *pBitstream,
                                              H264_PPS *                     pPPS,
                                              BOOL                           bIsHighProfile)
{
   int32_t iBytesWritten = pBitstream->get_byte_count();

   // Standard constraint to be between 0 and 31 inclusive
   assert(pPPS->seq_parameter_set_id >= 0);
   assert(pPPS->seq_parameter_set_id < 32);

   // Standard constraint to be between 0 and 255 inclusive
   assert(pPPS->pic_parameter_set_id >= 0);
   assert(pPPS->pic_parameter_set_id < 256);

   pBitstream->exp_Golomb_ue(pPPS->pic_parameter_set_id);
   pBitstream->exp_Golomb_ue(pPPS->seq_parameter_set_id);
   pBitstream->put_bits(1, pPPS->entropy_coding_mode_flag);
   pBitstream->put_bits(1, pPPS->pic_order_present_flag);   // bottom_field_pic_order_in_frame_present_flag
   pBitstream->exp_Golomb_ue(0);                            // num_slice_groups_minus1


   pBitstream->exp_Golomb_ue(pPPS->num_ref_idx_l0_active_minus1);
   pBitstream->exp_Golomb_ue(pPPS->num_ref_idx_l1_active_minus1);
   pBitstream->put_bits(1, 0);     // weighted_pred_flag
   pBitstream->put_bits(2, 0);     // weighted_bipred_idc
   pBitstream->exp_Golomb_se(0);   // pic_init_qp_minus26
   pBitstream->exp_Golomb_se(0);   // pic_init_qs_minus26
   pBitstream->exp_Golomb_se(0);   // chroma_qp_index_offset
   pBitstream->put_bits(1, 1);     // deblocking_filter_control_present_flag
   pBitstream->put_bits(1, pPPS->constrained_intra_pred_flag);
   pBitstream->put_bits(1, 0);   // redundant_pic_cnt_present_flag

   if (bIsHighProfile) {
      pBitstream->put_bits(1, pPPS->transform_8x8_mode_flag);
      pBitstream->put_bits(1, 0);     // pic_scaling_matrix_present_flag
      pBitstream->exp_Golomb_se(0);   // second_chroma_qp_index_offset
   }

   rbsp_trailing(pBitstream);
   pBitstream->flush();

   iBytesWritten = pBitstream->get_byte_count() - iBytesWritten;
   return (uint32_t) iBytesWritten;
}

uint32_t
d3d12_video_nalu_writer_h264::wrap_sps_nalu(d3d12_video_encoder_bitstream *pNALU, d3d12_video_encoder_bitstream *pRBSP)
{
   return wrap_rbsp_into_nalu(pNALU, pRBSP, NAL_REFIDC_REF, NAL_TYPE_SPS);
}

uint32_t
d3d12_video_nalu_writer_h264::wrap_pps_nalu(d3d12_video_encoder_bitstream *pNALU, d3d12_video_encoder_bitstream *pRBSP)
{
   return wrap_rbsp_into_nalu(pNALU, pRBSP, NAL_REFIDC_REF, NAL_TYPE_PPS);
}

void
d3d12_video_nalu_writer_h264::write_nalu_end(d3d12_video_encoder_bitstream *pNALU)
{
   pNALU->flush();
   pNALU->set_start_code_prevention(false);
   int32_t iNALUnitLen = pNALU->get_byte_count();

   if (false == pNALU->m_bBufferOverflow && 0x00 == pNALU->get_bitstream_buffer()[iNALUnitLen - 1]) {
      pNALU->put_bits(8, 0x03);
      pNALU->flush();
   }
}

uint32_t
d3d12_video_nalu_writer_h264::wrap_rbsp_into_nalu(d3d12_video_encoder_bitstream *pNALU,
                                                  d3d12_video_encoder_bitstream *pRBSP,
                                                  uint32_t                       iNaluIdc,
                                                  uint32_t                       iNaluType)
{
   bool isAligned = pRBSP->is_byte_aligned();   // causes side-effects in object state, don't put inside assert()
   assert(isAligned);

   int32_t iBytesWritten = pNALU->get_byte_count();

   pNALU->set_start_code_prevention(false);

   // NAL start code
   pNALU->put_bits(24, 0);
   pNALU->put_bits(8, 1);

   // NAL header
   pNALU->put_bits(1, 0);
   pNALU->put_bits(2, iNaluIdc);
   pNALU->put_bits(5, iNaluType);
   pNALU->flush();

   // NAL body
   pRBSP->flush();

   if (pRBSP->get_start_code_prevention_status()) {
      // Direct copying.
      pNALU->append_byte_stream(pRBSP);
   } else {
      // Copy with start code prevention.
      pNALU->set_start_code_prevention(true);
      int32_t  iLength = pRBSP->get_byte_count();
      uint8_t *pBuffer = pRBSP->get_bitstream_buffer();

      for (int32_t i = 0; i < iLength; i++) {
         pNALU->put_bits(8, pBuffer[i]);
      }
   }

   isAligned = pNALU->is_byte_aligned();   // causes side-effects in object state, don't put inside assert()
   assert(isAligned);
   write_nalu_end(pNALU);

   pNALU->flush();

   iBytesWritten = pNALU->get_byte_count() - iBytesWritten;
   return (uint32_t) iBytesWritten;
}

void
d3d12_video_nalu_writer_h264::sps_to_nalu_bytes(H264_SPS *                     pSPS,
                                                std::vector<uint8_t> &         headerBitstream,
                                                std::vector<uint8_t>::iterator placingPositionStart,
                                                size_t &                       writtenBytes)
{
   // Wrap SPS into NALU and copy full NALU into output byte array
   d3d12_video_encoder_bitstream rbsp, nalu;

   if (!rbsp.create_bitstream(MAX_COMPRESSED_SPS)) {
      debug_printf("rbsp.create_bitstream(MAX_COMPRESSED_SPS) failed\n");
      assert(false);
   }

   if (!nalu.create_bitstream(2 * MAX_COMPRESSED_SPS)) {
      debug_printf("nalu.create_bitstream(2 * MAX_COMPRESSED_SPS) failed\n");
      assert(false);
   }

   rbsp.set_start_code_prevention(true);
   if (write_sps_bytes(&rbsp, pSPS) <= 0u) {
      debug_printf("write_sps_bytes(&rbsp, pSPS) didn't write any bytes.\n");
      assert(false);
   }

   if (wrap_sps_nalu(&nalu, &rbsp) <= 0u) {
      debug_printf("wrap_sps_nalu(&nalu, &rbsp) didn't write any bytes.\n");
      assert(false);
   }

   // Deep copy nalu into headerBitstream, nalu gets out of scope here and its destructor frees the nalu object buffer
   // memory.
   uint8_t *naluBytes    = nalu.get_bitstream_buffer();
   size_t   naluByteSize = nalu.get_byte_count();

   auto startDstIndex = std::distance(headerBitstream.begin(), placingPositionStart);
   if (headerBitstream.size() < (startDstIndex + naluByteSize)) {
      headerBitstream.resize(startDstIndex + naluByteSize);
   }

   std::copy_n(&naluBytes[0], naluByteSize, &headerBitstream.data()[startDstIndex]);

   writtenBytes = naluByteSize;
}

void
d3d12_video_nalu_writer_h264::pps_to_nalu_bytes(H264_PPS *                     pPPS,
                                                std::vector<uint8_t> &         headerBitstream,
                                                BOOL                           bIsHighProfile,
                                                std::vector<uint8_t>::iterator placingPositionStart,
                                                size_t &                       writtenBytes)
{
   // Wrap PPS into NALU and copy full NALU into output byte array
   d3d12_video_encoder_bitstream rbsp, nalu;
   if (!rbsp.create_bitstream(MAX_COMPRESSED_PPS)) {
      debug_printf("rbsp.create_bitstream(MAX_COMPRESSED_PPS) failed\n");
      assert(false);
   }

   if (!nalu.create_bitstream(2 * MAX_COMPRESSED_PPS)) {
      debug_printf("nalu.create_bitstream(2 * MAX_COMPRESSED_PPS) failed\n");
      assert(false);
   }

   rbsp.set_start_code_prevention(true);

   if (write_pps_bytes(&rbsp, pPPS, bIsHighProfile) <= 0u) {
      debug_printf("write_pps_bytes(&rbsp, pPPS, bIsHighProfile) didn't write any bytes.\n");
      assert(false);
   }

   if (wrap_pps_nalu(&nalu, &rbsp) <= 0u) {
      debug_printf("wrap_pps_nalu(&nalu, &rbsp) didn't write any bytes.\n");
      assert(false);
   }

   // Deep copy nalu into headerBitstream, nalu gets out of scope here and its destructor frees the nalu object buffer
   // memory.
   uint8_t *naluBytes    = nalu.get_bitstream_buffer();
   size_t   naluByteSize = nalu.get_byte_count();

   auto startDstIndex = std::distance(headerBitstream.begin(), placingPositionStart);
   if (headerBitstream.size() < (startDstIndex + naluByteSize)) {
      headerBitstream.resize(startDstIndex + naluByteSize);
   }

   std::copy_n(&naluBytes[0], naluByteSize, &headerBitstream.data()[startDstIndex]);

   writtenBytes = naluByteSize;
}

void
d3d12_video_nalu_writer_h264::write_end_of_stream_nalu(std::vector<uint8_t> &         headerBitstream,
                                                       std::vector<uint8_t>::iterator placingPositionStart,
                                                       size_t &                       writtenBytes)
{
   d3d12_video_encoder_bitstream rbsp, nalu;
   if (!rbsp.create_bitstream(8)) {
      debug_printf("rbsp.create_bitstream(8) failed\n");
      assert(false);
   }
   if (!nalu.create_bitstream(2 * MAX_COMPRESSED_PPS)) {
      debug_printf("nalu.create_bitstream(2 * MAX_COMPRESSED_PPS) failed\n");
      assert(false);
   }

   rbsp.set_start_code_prevention(true);
   if (wrap_rbsp_into_nalu(&nalu, &rbsp, NAL_REFIDC_REF, NAL_TYPE_END_OF_STREAM) <= 0u) {
      debug_printf(
         "wrap_rbsp_into_nalu(&nalu, &rbsp, NAL_REFIDC_REF, NAL_TYPE_END_OF_STREAM) didn't write any bytes.\n");;
      assert(false);
   }

   // Deep copy nalu into headerBitstream, nalu gets out of scope here and its destructor frees the nalu object buffer
   // memory.
   uint8_t *naluBytes    = nalu.get_bitstream_buffer();
   size_t   naluByteSize = nalu.get_byte_count();

   auto startDstIndex = std::distance(headerBitstream.begin(), placingPositionStart);
   if (headerBitstream.size() < (startDstIndex + naluByteSize)) {
      headerBitstream.resize(startDstIndex + naluByteSize);
   }

   std::copy_n(&naluBytes[0], naluByteSize, &headerBitstream.data()[startDstIndex]);

   writtenBytes = naluByteSize;
}

void
d3d12_video_nalu_writer_h264::write_end_of_sequence_nalu(std::vector<uint8_t> &         headerBitstream,
                                                         std::vector<uint8_t>::iterator placingPositionStart,
                                                         size_t &                       writtenBytes)
{
   d3d12_video_encoder_bitstream rbsp, nalu;
   if (!rbsp.create_bitstream(8)) {
      debug_printf("rbsp.create_bitstream(8) failed.\n");
      assert(false);
   }

   if (!nalu.create_bitstream(2 * MAX_COMPRESSED_PPS)) {
      debug_printf("nalu.create_bitstream(2 * MAX_COMPRESSED_PPS) failed.\n");
      assert(false);
   }

   rbsp.set_start_code_prevention(true);
   if (wrap_rbsp_into_nalu(&nalu, &rbsp, NAL_REFIDC_REF, NAL_TYPE_END_OF_SEQUENCE) <= 0u) {

      debug_printf(
         "wrap_rbsp_into_nalu(&nalu, &rbsp, NAL_REFIDC_REF, NAL_TYPE_END_OF_SEQUENCE) didn't write any bytes.\n");
      assert(false);
   }

   // Deep copy nalu into headerBitstream, nalu gets out of scope here and its destructor frees the nalu object buffer
   // memory.
   uint8_t *naluBytes    = nalu.get_bitstream_buffer();
   size_t   naluByteSize = nalu.get_byte_count();

   auto startDstIndex = std::distance(headerBitstream.begin(), placingPositionStart);
   if (headerBitstream.size() < (startDstIndex + naluByteSize)) {
      headerBitstream.resize(startDstIndex + naluByteSize);
   }

   std::copy_n(&naluBytes[0], naluByteSize, &headerBitstream.data()[startDstIndex]);

   writtenBytes = naluByteSize;
}

void
d3d12_video_nalu_writer_h264::write_access_unit_delimiter_nalu(std::vector<uint8_t> &         headerBitstream,
                                                               std::vector<uint8_t>::iterator placingPositionStart,
                                                               size_t &                       writtenBytes)
{
   d3d12_video_encoder_bitstream rbsp, nalu;
   if (!rbsp.create_bitstream(8)) {
      debug_printf("rbsp.create_bitstream(8) failed.\n");
      assert(false);
   }

   if (!nalu.create_bitstream(2 * MAX_COMPRESSED_PPS)) {
      debug_printf("nalu.create_bitstream(2 * MAX_COMPRESSED_PPS) failed.\n");
      assert(false);
   }

   rbsp.set_start_code_prevention(true);
   rbsp.put_bits(3, 2/*primary_pic_type*/);
   rbsp_trailing(&rbsp);
   rbsp.flush();
   if (wrap_rbsp_into_nalu(&nalu, &rbsp, NAL_REFIDC_NONREF, NAL_TYPE_ACCESS_UNIT_DELIMITER) <= 0u) {

      debug_printf(
         "wrap_rbsp_into_nalu(&nalu, &rbsp, NAL_REFIDC_NONREF, NAL_TYPE_ACCESS_UNIT_DELIMITER) didn't write any bytes.\n");
      assert(false);
   }

   // Deep copy nalu into headerBitstream, nalu gets out of scope here and its destructor frees the nalu object buffer
   // memory.
   uint8_t *naluBytes    = nalu.get_bitstream_buffer();
   size_t   naluByteSize = nalu.get_byte_count();

   auto startDstIndex = std::distance(headerBitstream.begin(), placingPositionStart);
   if (headerBitstream.size() < (startDstIndex + naluByteSize)) {
      headerBitstream.resize(startDstIndex + naluByteSize);
   }

   std::copy_n(&naluBytes[0], naluByteSize, &headerBitstream.data()[startDstIndex]);

   writtenBytes = naluByteSize;
}
