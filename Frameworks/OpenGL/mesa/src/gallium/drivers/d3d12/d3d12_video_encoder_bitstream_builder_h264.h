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

#ifndef D3D12_VIDEO_ENC_BITSTREAM_BUILDER_H264_H
#define D3D12_VIDEO_ENC_BITSTREAM_BUILDER_H264_H

#include "d3d12_video_encoder_nalu_writer_h264.h"
#include "d3d12_video_encoder_bitstream_builder.h"

class d3d12_video_bitstream_builder_h264 : public d3d12_video_bitstream_builder_interface
{

 public:
   d3d12_video_bitstream_builder_h264(bool insert_aud_nalu = false);
   ~d3d12_video_bitstream_builder_h264() {};

   void build_sps(const struct pipe_h264_enc_seq_param &                 seqData,
                  const enum pipe_video_profile &                        profile,
                  const D3D12_VIDEO_ENCODER_LEVELS_H264 &                level,
                  const DXGI_FORMAT &                                    inputFmt,
                  const D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_H264 &   codecConfig,
                  const D3D12_VIDEO_ENCODER_SEQUENCE_GOP_STRUCTURE_H264 &gopConfig,
                  uint32_t                                               seq_parameter_set_id,
                  uint32_t                                               max_num_ref_frames,
                  D3D12_VIDEO_ENCODER_PICTURE_RESOLUTION_DESC            sequenceTargetResolution,
                  D3D12_BOX                                              frame_cropping_codec_config,
                  std::vector<uint8_t> &                                 headerBitstream,
                  std::vector<uint8_t>::iterator                         placingPositionStart,
                  size_t &                                               writtenBytes);

   void build_pps(const enum pipe_video_profile &                            profile,
                  const D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_H264 &       codecConfig,
                  const D3D12_VIDEO_ENCODER_PICTURE_CONTROL_CODEC_DATA_H264 &pictureControl,
                  uint32_t                                                   pic_parameter_set_id,
                  uint32_t                                                   seq_parameter_set_id,
                  std::vector<uint8_t> &                                     headerBitstream,
                  std::vector<uint8_t>::iterator                             placingPositionStart,
                  size_t &                                                   writtenBytes);

   void write_end_of_stream_nalu(std::vector<uint8_t> &         headerBitstream,
                                 std::vector<uint8_t>::iterator placingPositionStart,
                                 size_t &                       writtenBytes);
   void write_end_of_sequence_nalu(std::vector<uint8_t> &         headerBitstream,
                                   std::vector<uint8_t>::iterator placingPositionStart,
                                   size_t &                       writtenBytes);

   void write_aud(std::vector<uint8_t> &         headerBitstream,
                  std::vector<uint8_t>::iterator placingPositionStart,
                  size_t &                       writtenBytes);

   void print_pps(const H264_PPS &pps);
   void print_sps(const H264_SPS &sps);

   uint32_t get_active_sps_id()
   {
      return m_activeSPSIndex;
   };
   uint32_t get_active_pps_id()
   {
      return m_activePPSIndex;
   };

   std::vector<uint8_t>& get_active_pps()
   {
      return m_activePPS;
   };

   void set_active_sps_id(uint32_t active_sps_id)
   {
      m_activeSPSIndex = active_sps_id;
      debug_printf("[d3d12_video_bitstream_builder_h264] Setting new active SPS ID: %d ", m_activeSPSIndex);
   };
   void set_active_pps_id(uint32_t active_pps_id)
   {
      m_activePPSIndex = active_pps_id;
      debug_printf("[d3d12_video_bitstream_builder_h264] Setting new active PPS ID: %d ", m_activePPSIndex);
   };

   bool insert_aud_nalu_requested() { return m_insert_aud_nalu; }

 private:
   d3d12_video_nalu_writer_h264 m_h264Encoder;
   std::vector<uint8_t> m_activePPS;
   uint32_t m_activeSPSIndex = 0;
   uint32_t m_activePPSIndex = 0;
   bool m_insert_aud_nalu = false;
};

#endif
