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

#ifndef D3D12_VIDEO_ENC_BITSTREAM_H
#define D3D12_VIDEO_ENC_BITSTREAM_H

#include "d3d12_video_types.h"

class d3d12_video_encoder_bitstream
{
 public:
   d3d12_video_encoder_bitstream();
   ~d3d12_video_encoder_bitstream();

 public:
   void get_current_buffer_position_and_size(uint8_t **ppCurrBufPos, int32_t *pdwLeftBufSize);
   void inc_current_offset(int32_t dwOffset);
   bool create_bitstream(uint32_t uiInitBufferSize);
   void setup_bitstream(uint32_t uiInitBufferSize, uint8_t *pBuffer, size_t initial_byte_offset);
   void attach(uint8_t *pBitsBuffer, uint32_t uiBufferSize);
   void put_bits(int32_t uiBitsCount, uint32_t iBitsVal);
   void flush();
   void exp_Golomb_ue(uint32_t uiVal);
   void exp_Golomb_se(int32_t iVal);
   void put_aligning_bits();
   void put_trailing_bits();
   void put_su_bits(uint16_t uiBitsCount, int32_t iBitsVal);
   void put_ns_bits(uint16_t uiBitsCount, uint32_t iBitsVal);
   uint16_t calculate_su_bits(uint16_t uiBitsCount, int32_t iBitsVal);
   void put_le_bytes(size_t uiBytesCount, uint32_t iBitsVal);
   void put_leb128_bytes(uint64_t iBitsVal);

   inline void clear()
   {
      m_iBitsToGo = 32;
      m_uiOffset = 0;
      m_uintEncBuffer = 0;
   };

   void append_byte_stream(d3d12_video_encoder_bitstream *pStream);

   void set_start_code_prevention(bool bSCP)
   {
      m_bPreventStartCode = bSCP;
   }
   int32_t get_bits_count()
   {
      return m_uiOffset * 8 + (32 - m_iBitsToGo);
   }
   int32_t get_byte_count()
   {
      return m_uiOffset + ((32 - m_iBitsToGo) >> 3);
   }
   uint8_t *get_bitstream_buffer()
   {
      return m_pBitsBuffer;
   }
   bool is_byte_aligned()
   {
      if (m_bBufferOverflow) {
         m_iBitsToGo = 32;
      }
      return !(m_iBitsToGo & 7);
   }
   int32_t get_num_bits_for_byte_align()
   {
      return (m_iBitsToGo & 7);
   }
   bool get_start_code_prevention_status()
   {
      return m_bPreventStartCode;
   }
   bool verify_buffer(uint32_t uiBytesToWrite);

 public:
   bool m_bBufferOverflow;
   bool m_bAllowReallocate;

 private:
   void write_byte_start_code_prevention(uint8_t u8Val);
   bool reallocate_buffer();
   int32_t get_exp_golomb0_code_len(uint32_t uiVal);

   const uint8_t m_iLog_2_N[256] = {
      0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5,
      5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
      7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
      7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
      7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
   };

 private:
   uint8_t *m_pBitsBuffer;
   uint32_t m_uiBitsBufferSize;
   uint32_t m_uiOffset;

   bool m_bExternalBuffer;
   uint32_t m_uintEncBuffer;
   int32_t m_iBitsToGo;

   bool m_bPreventStartCode;
};

#endif
