/*
 * Copyright (c) 2015-2016, Apple Inc. All rights reserved.
 * Copyright (C) 2025-2026 Zoe Knox <zoe@pixin.net>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1.  Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * 3.  Neither the name of the copyright holder(s) nor the names of any
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _LZVN_DECODE_H
#define _LZVN_DECODE_H

#include <assert.h>
#include <stddef.h> /* size_t */
#include <stdint.h>
#include <string.h>

#define LZVN_INLINE static inline __attribute__((__always_inline__))
#define PLATFORM_NAME_LEN  (64)
#define ROOT_PATH_LEN     (256)

typedef int64_t lzvn_offset;

/* from kext_tools/kernelcache.h */
typedef struct prelinked_kernel_header {
    uint32_t  signature;
    uint32_t  compressType;
    uint32_t  adler32;
    uint32_t  uncompressedSize;
    uint32_t  compressedSize;
    uint32_t  prelinkVersion;
    uint32_t  reserved[10];
    char      platformName[PLATFORM_NAME_LEN]; // unused
    char      rootPath[ROOT_PATH_LEN];         // unused
    char      data[0];
} PrelinkedKernelHeader;

/*! @abstract Load bytes from memory location SRC. */
LZVN_INLINE uint16_t load2(const void *ptr) {
  uint16_t data;
  memcpy(&data, ptr, sizeof data);
  return data;
}

LZVN_INLINE uint32_t load4(const void *ptr) {
  uint32_t data;
  memcpy(&data, ptr, sizeof data);
  return data;
}

LZVN_INLINE uint64_t load8(const void *ptr) {
  uint64_t data;
  memcpy(&data, ptr, sizeof data);
  return data;
}

/*! @abstract Store bytes to memory location DST. */
LZVN_INLINE void store2(void *ptr, uint16_t data) {
  memcpy(ptr, &data, sizeof data);
}

LZVN_INLINE void store4(void *ptr, uint32_t data) {
  memcpy(ptr, &data, sizeof data);
}

LZVN_INLINE void store8(void *ptr, uint64_t data) {
  memcpy(ptr, &data, sizeof data);
}

/*! @abstract Load+store bytes from locations SRC to DST. Not intended for use
 * with overlapping buffers. Note that for LZ-style compression, you need
 * copies to behave like naive memcpy( ) implementations do, splatting the
 * leading sequence if the buffers overlap. This copy does not do that, so
 * should not be used with overlapping buffers. */
LZVN_INLINE void copy8(void *dst, const void *src) { store8(dst, load8(src)); }
LZVN_INLINE void copy16(void *dst, const void *src) {
  uint64_t m0 = load8(src);
  uint64_t m1 = load8((const unsigned char *)src + 8);
  store8(dst, m0);
  store8((unsigned char *)dst + 8, m1);
}

/*! @abstract Extracts \p width bits from \p container, starting with \p lsb; if
 * we view \p container as a bit array, we extract \c container[lsb:lsb+width]. */
LZVN_INLINE uintmax_t extract(uintmax_t container, unsigned lsb,
                              unsigned width) {
  static const size_t container_width = sizeof container * 8;
  assert(lsb < container_width);
  assert(width > 0 && width <= container_width);
  assert(lsb + width <= container_width);
  if (width == container_width)
    return container;
  return (container >> lsb) & (((uintmax_t)1 << width) - 1);
}

/*! @abstract Base decoder state. */
typedef struct {
  // Decoder I/O
  const unsigned char *src; // Next byte to read in source buffer
  const unsigned char *src_end; // Next byte after source buffer

  // Next byte to write in destination buffer (by decoder)
  unsigned char *dst;
  // Valid range for destination buffer is [dst_begin, dst_end - 1]
  unsigned char *dst_begin;
  unsigned char *dst_end;
  // Next byte to read in destination buffer (modified by caller)
  unsigned char *dst_current;

  // Decoder state
  // Partially expanded match, or 0,0,0.
  // In that case, src points to the next literal to copy, or the next op-code
  // if L==0.
  size_t L, M, D;
  lzvn_offset d_prev; // Distance for last emitted match, or 0
  int end_of_stream; // Did we decode end-of-stream?
} lzvn_decoder_state;

/*! @abstract Decode source to destination.
 *  Updates \p state (src,dst,d_prev). */
void lzvn_decode(lzvn_decoder_state *state);

#endif /* _LZVN_DECODE_H */
