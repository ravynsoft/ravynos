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

#ifndef D3D12_VIDEO_ENC_BITSTREAM_BUILDER_HEVC_H
#define D3D12_VIDEO_ENC_BITSTREAM_BUILDER_HEVC_H

#include "d3d12_video_encoder_nalu_writer_hevc.h"
#include "d3d12_video_encoder_bitstream_builder.h"

class d3d12_video_bitstream_builder_hevc : public d3d12_video_bitstream_builder_interface
{

 public:
   d3d12_video_bitstream_builder_hevc() {};
   ~d3d12_video_bitstream_builder_hevc() {};

   void build_vps(const D3D12_VIDEO_ENCODER_PROFILE_HEVC& profile,
                  const D3D12_VIDEO_ENCODER_LEVEL_TIER_CONSTRAINTS_HEVC& level,
                  const DXGI_FORMAT inputFmt,
                  uint8_t maxRefFrames,
                  bool gopHasBFrames,
                  uint8_t vps_video_parameter_set_id,
                  std::vector<BYTE> &headerBitstream,
                  std::vector<BYTE>::iterator placingPositionStart,
                  size_t &writtenBytes,
                  HevcVideoParameterSet* pVPSStruct = nullptr);

   void build_sps(const HevcVideoParameterSet& parentVPS,
                  const struct pipe_h265_enc_seq_param & seqData,
                  uint8_t seq_parameter_set_id,
                  const D3D12_VIDEO_ENCODER_PICTURE_RESOLUTION_DESC& encodeResolution,
                  const D3D12_BOX& crop_window_upper_layer,
                  const UINT picDimensionMultipleRequirement,
                  const DXGI_FORMAT& inputFmt,
                  const D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC& codecConfig,
                  const D3D12_VIDEO_ENCODER_SEQUENCE_GOP_STRUCTURE_HEVC& hevcGOP,    
                  std::vector<BYTE> &headerBitstream,
                  std::vector<BYTE>::iterator placingPositionStart,
                  size_t &writtenBytes,
                  HevcSeqParameterSet* outputSPS = nullptr);

   void build_pps(const HevcSeqParameterSet& parentSPS,
                  uint8_t pic_parameter_set_id,
                  const D3D12_VIDEO_ENCODER_CODEC_CONFIGURATION_HEVC& codecConfig,
                  const D3D12_VIDEO_ENCODER_PICTURE_CONTROL_CODEC_DATA_HEVC& pictureControl,
                  std::vector<BYTE> &headerBitstream,
                  std::vector<BYTE>::iterator placingPositionStart,
                  size_t &writtenBytes,
                  HevcPicParameterSet* outputPPS = nullptr);

   void write_end_of_stream_nalu(std::vector<uint8_t> &         headerBitstream,
                                 std::vector<uint8_t>::iterator placingPositionStart,
                                 size_t &                       writtenBytes);
   void write_end_of_sequence_nalu(std::vector<uint8_t> &         headerBitstream,
                                   std::vector<uint8_t>::iterator placingPositionStart,
                                   size_t &                       writtenBytes);

   void print_vps(const HevcVideoParameterSet& vps);
   void print_sps(const HevcSeqParameterSet& sps);
   void print_pps(const HevcPicParameterSet& pps);
   void print_rps(const HevcSeqParameterSet* sps, UINT stRpsIdx);

   std::vector<uint8_t>& get_active_pps()
   {
      return m_activePPS;
   };

   HevcVideoParameterSet get_latest_vps()
   {
      return m_latest_vps;
   }

   HevcSeqParameterSet get_latest_sps()
   {
      return m_latest_sps;
   }

   HevcPicParameterSet get_latest_pps()
   {
      return m_latest_pps;
   }

   uint32_t get_active_vps_id()
   {
      return m_activeVPSIndex;
   };
   uint32_t get_active_sps_id()
   {
      return m_activeSPSIndex;
   };
   uint32_t get_active_pps_id()
   {
      return m_activePPSIndex;
   };

   void set_active_vps_id(uint32_t active_vps_id)
   {
      m_activeVPSIndex = active_vps_id;
      debug_printf("[d3d12_video_bitstream_builder_hevc] Setting new active VPS ID: %d ", m_activeVPSIndex);
   };

   void set_active_sps_id(uint32_t active_sps_id)
   {
      m_activeSPSIndex = active_sps_id;
      debug_printf("[d3d12_video_bitstream_builder_hevc] Setting new active SPS ID: %d ", m_activeSPSIndex);
   };
   void set_active_pps_id(uint32_t active_pps_id)
   {
      m_activePPSIndex = active_pps_id;
      debug_printf("[d3d12_video_bitstream_builder_hevc] Setting new active PPS ID: %d ", m_activePPSIndex);
   };

 private:
   d3d12_video_nalu_writer_hevc m_hevcEncoder;
   uint32_t m_activeVPSIndex = 0;
   uint32_t m_activeSPSIndex = 0;
   uint32_t m_activePPSIndex = 0;
   HevcVideoParameterSet m_latest_vps = {};
   HevcSeqParameterSet m_latest_sps = {};
   HevcPicParameterSet m_latest_pps = {};
   std::vector<uint8_t> m_activePPS;

   void init_profile_tier_level(HEVCProfileTierLevel *ptl, uint8_t HEVCProfileIdc, uint8_t HEVCLevelIdc, bool isHighTier);
};

#endif
