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

#include <climits>
#include "d3d12_video_encoder_bitstream.h"

d3d12_video_encoder_bitstream::d3d12_video_encoder_bitstream()
{
   m_pBitsBuffer = nullptr;
   m_uiBitsBufferSize = 0;
   m_uiOffset = 0;
   m_iBitsToGo = 32;
   m_uintEncBuffer = 0;
   m_bExternalBuffer = false;
   m_bBufferOverflow = false;
   m_bPreventStartCode = false;
   m_bAllowReallocate = false;
}

d3d12_video_encoder_bitstream::~d3d12_video_encoder_bitstream()
{
   if (!m_bExternalBuffer) {
      if (m_pBitsBuffer) {
         delete[] (m_pBitsBuffer);
         (m_pBitsBuffer) = NULL;
      }
   }
}

int32_t
d3d12_video_encoder_bitstream::get_exp_golomb0_code_len(uint32_t uiVal)
{
   int32_t iLen = 0;
   uiVal++;

   if (uiVal >= 0x10000) {
      uiVal >>= 16;
      iLen += 16;
   }
   if (uiVal >= 0x100) {
      uiVal >>= 8;
      iLen += 8;
   }

   assert(uiVal < 256);

   return iLen + m_iLog_2_N[uiVal];
}

void
d3d12_video_encoder_bitstream::exp_Golomb_ue(uint32_t uiVal)
{
   if (uiVal != UINT32_MAX) {
      int32_t iLen = get_exp_golomb0_code_len(uiVal);
      put_bits((iLen << 1) + 1, uiVal + 1);
   } else {
      put_bits(32, 0);
      put_bits(1, 1);
      put_bits(32, 1);
   }
}

void
d3d12_video_encoder_bitstream::exp_Golomb_se(int32_t iVal)
{
   if (iVal > 0) {
      exp_Golomb_ue((iVal << 1) - 1);
   } else {
      exp_Golomb_ue(((-iVal) << 1) - (iVal == INT_MIN));
   }
}

void
d3d12_video_encoder_bitstream::setup_bitstream(uint32_t uiInitBufferSize, uint8_t *pBuffer, size_t initial_byte_offset)
{
   m_pBitsBuffer = pBuffer;
   m_uiBitsBufferSize = uiInitBufferSize;
   m_uiOffset = initial_byte_offset;
   memset(m_pBitsBuffer + initial_byte_offset, 0, m_uiBitsBufferSize - initial_byte_offset);
   m_bExternalBuffer = true;
   m_bAllowReallocate = false;
}

bool
d3d12_video_encoder_bitstream::create_bitstream(uint32_t uiInitBufferSize)
{
   assert((uiInitBufferSize) >= 4 && !(uiInitBufferSize & 3));

   m_pBitsBuffer = (uint8_t *) new uint8_t[uiInitBufferSize];

   if (nullptr == m_pBitsBuffer) {
      return false;
   }

   m_uiBitsBufferSize = uiInitBufferSize;
   m_uiOffset = 0;
   memset(m_pBitsBuffer, 0, m_uiBitsBufferSize);
   m_bExternalBuffer = false;

   return true;
}

bool
d3d12_video_encoder_bitstream::reallocate_buffer()
{
   uint32_t uiBufferSize = m_uiBitsBufferSize * 3 / 2;
   uint8_t *pNewBuffer = (uint8_t *) new uint8_t[uiBufferSize];

   if (nullptr == pNewBuffer) {
      return false;
   }

   memcpy(pNewBuffer, m_pBitsBuffer, m_uiOffset * sizeof(uint8_t));
   if (m_pBitsBuffer) {
      delete[] (m_pBitsBuffer);
      (m_pBitsBuffer) = NULL;
   }
   m_pBitsBuffer = pNewBuffer;
   m_uiBitsBufferSize = uiBufferSize;
   return true;
}

bool
d3d12_video_encoder_bitstream::verify_buffer(uint32_t uiBytesToWrite)
{
   if (!m_bBufferOverflow) {
      if (m_uiOffset + uiBytesToWrite > m_uiBitsBufferSize) {
         if (!m_bAllowReallocate || !reallocate_buffer()) {
            m_bBufferOverflow = true;
            return false;
         }
      }

      return true;
   }

   return false;
}

void
d3d12_video_encoder_bitstream::inc_current_offset(int32_t dwOffset)
{
   assert(32 == m_iBitsToGo && m_uiOffset < m_uiBitsBufferSize);
   m_uiOffset += dwOffset;
}

void
d3d12_video_encoder_bitstream::get_current_buffer_position_and_size(uint8_t **ppCurrBufPos, int32_t *pdwLeftBufSize)
{
   assert(32 == m_iBitsToGo && m_uiOffset < m_uiBitsBufferSize);
   *ppCurrBufPos = m_pBitsBuffer + m_uiOffset;
   *pdwLeftBufSize = m_uiBitsBufferSize - m_uiOffset;
}

void
d3d12_video_encoder_bitstream::attach(uint8_t *pBitsBuffer, uint32_t uiBufferSize)
{
   m_pBitsBuffer = pBitsBuffer;
   m_uiBitsBufferSize = uiBufferSize;
   m_bExternalBuffer = true;
   m_bBufferOverflow = false;
   m_bAllowReallocate = false;

   clear();
}

void
d3d12_video_encoder_bitstream::write_byte_start_code_prevention(uint8_t u8Val)
{
   int32_t iOffset = m_uiOffset;
   uint8_t *pBuffer = m_pBitsBuffer + iOffset;

   if (m_bPreventStartCode && iOffset > 1) {
      if (((u8Val & 0xfc) | pBuffer[-2] | pBuffer[-1]) == 0) {
         *pBuffer++ = 3;
         iOffset++;
      }
   }

   *pBuffer = u8Val;
   iOffset++;

   m_uiOffset = iOffset;
}

#define WRITE_BYTE(byte) write_byte_start_code_prevention(byte)

void
d3d12_video_encoder_bitstream::put_bits(int32_t uiBitsCount, uint32_t iBitsVal)
{
   assert(uiBitsCount > 0);
   assert(uiBitsCount <= 32);

   if (uiBitsCount < m_iBitsToGo) {
      m_uintEncBuffer |= (iBitsVal << (m_iBitsToGo - uiBitsCount));
      m_iBitsToGo -= uiBitsCount;
   } else if (verify_buffer(4)) {
      int32_t iLeftOverBits = uiBitsCount - m_iBitsToGo;
      m_uintEncBuffer |= (iBitsVal >> iLeftOverBits);

      uint8_t *temp = (uint8_t *) (&m_uintEncBuffer);
      WRITE_BYTE(*(temp + 3));
      WRITE_BYTE(*(temp + 2));
      WRITE_BYTE(*(temp + 1));
      WRITE_BYTE(*temp);

      m_uintEncBuffer = 0;
      m_iBitsToGo = 32 - iLeftOverBits;

      if (iLeftOverBits > 0) {
         m_uintEncBuffer = (iBitsVal << (32 - iLeftOverBits));
      }
   }
}

void
d3d12_video_encoder_bitstream::flush()
{
   ASSERTED bool isAligned = is_byte_aligned();   // causes side-effects in object state, don't put inside assert()
   assert(isAligned);

   uint32_t temp = (uint32_t) (32 - m_iBitsToGo);

   if (!verify_buffer(temp >> 3)) {
      return;
   }

   while (temp > 0) {
      WRITE_BYTE((uint8_t) (m_uintEncBuffer >> 24));
      m_uintEncBuffer <<= 8;
      temp -= 8;
   }

   m_iBitsToGo = 32;
   m_uintEncBuffer = 0;
}

void
d3d12_video_encoder_bitstream::append_byte_stream(d3d12_video_encoder_bitstream *pStream)
{
   ASSERTED bool isStreamAligned =
      pStream->is_byte_aligned();   // causes side-effects in object state, don't put inside assert()
   assert(isStreamAligned);
   ASSERTED bool isThisAligned = is_byte_aligned();   // causes side-effects in object state, don't put inside assert()
   assert(isThisAligned);
   assert(m_iBitsToGo == 32);

   uint8_t *pDst = m_pBitsBuffer + m_uiOffset;
   uint8_t *pSrc = pStream->get_bitstream_buffer();
   uint32_t uiLen = (uint32_t) pStream->get_byte_count();

   if (!verify_buffer(uiLen)) {
      return;
   }

   memcpy(pDst, pSrc, uiLen);
   m_uiOffset += uiLen;
}

void
d3d12_video_encoder_bitstream::put_aligning_bits()
{
   int32_t iLeft = get_num_bits_for_byte_align();
   if (iLeft)
      put_bits(iLeft, 0);   // trailing_zero_bit

   ASSERTED bool isAligned = is_byte_aligned();   // causes side-effects in object state, don't put inside assert()
   assert(isAligned);
}

void
d3d12_video_encoder_bitstream::put_trailing_bits()
{
   // trailing_one_bit shall be equal to 1.
   // When the syntax element trailing_one_bit is read, it is a requirement that nbBits is greater than zero.
   put_bits(1, 1);   // trailing_one_bit
   int32_t nbBits = get_num_bits_for_byte_align();
   while (nbBits > 0) {
      put_bits(1, 0);   // trailing_zero_bit
      nbBits--;
   }
   ASSERTED bool isAligned = is_byte_aligned();   // causes side-effects in object state, don't put inside assert()
   assert(isAligned);
}

void
d3d12_video_encoder_bitstream::put_su_bits(uint16_t uiBitsCount, int32_t iBitsVal)
{
   put_bits(uiBitsCount, calculate_su_bits(uiBitsCount, iBitsVal));
}

void
d3d12_video_encoder_bitstream::put_ns_bits(uint16_t uiBitsCount, uint32_t iBitsVal)
{
   if (uiBitsCount > 1) {
      uint32_t width = 0;
      uint32_t tmp = uiBitsCount;
      while (tmp) {
         tmp = (tmp >> 1);
         width++;
      }
      uint32_t m = (1 << width) - uiBitsCount;
      if (iBitsVal < m)
         put_bits(width - 1, iBitsVal);
      else
         put_bits(width, iBitsVal + m);
   }
}

uint16_t
d3d12_video_encoder_bitstream::calculate_su_bits(uint16_t uiBitsCount, int32_t iBitsVal)
{
   int16_t mask_sign = 1 << (uiBitsCount - 1);
   if (iBitsVal & mask_sign)
      iBitsVal = iBitsVal - 2 * mask_sign;
   return iBitsVal;
}

void
d3d12_video_encoder_bitstream::put_le_bytes(size_t uiBytesCount, uint32_t iBitsVal)
{
   assert(uiBytesCount <= sizeof(iBitsVal));
   for (size_t i = 0; i < uiBytesCount; i++) {
      put_bits(8, static_cast<uint8_t>(iBitsVal & 0xFF));
      iBitsVal >>= 8;
   }
}

void
d3d12_video_encoder_bitstream::put_leb128_bytes(uint64_t iBitsVal)
{
   do {
      uint8_t cur_byte = (iBitsVal & 0x7F);
      iBitsVal >>= 7;
      if (iBitsVal != 0)
         cur_byte |= 0x80;
      put_bits(8, cur_byte);
   } while (iBitsVal != 0);
}
