/*
 * Copyright (c) 2019 Etnaviv Project
 * Copyright (C) 2019 Zodiac Inflight Innovations
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Christian Gmeiner <christian.gmeiner@gmail.com>
 */

#include "etnaviv_etc2.h"
#include "etnaviv_resource.h"
#include "etnaviv_screen.h"
#include "hw/common.xml.h"
#include "util/format/u_format.h"

bool
etna_etc2_needs_patching(const struct pipe_resource *prsc)
{
   const struct etna_screen *screen = etna_screen(prsc->screen);

   if (!util_format_is_etc(prsc->format))
      return false;

   if (VIV_FEATURE(screen, chipMinorFeatures2, HALTI1))
      return false;

   switch (prsc->format) {
   case PIPE_FORMAT_ETC2_RGB8:
   case PIPE_FORMAT_ETC2_SRGB8:
   case PIPE_FORMAT_ETC2_RGB8A1:
   case PIPE_FORMAT_ETC2_SRGB8A1:
   case PIPE_FORMAT_ETC2_RGBA8:
   case PIPE_FORMAT_ETC2_SRGBA8:
      return true;
      break;

   default:
      return false;
   }
}

static inline bool
needs_patching(uint8_t *buffer, bool punchthrough_alpha)
{
   /* punchthrough_alpha or etc2 individual mode? */
   if (!punchthrough_alpha && !(buffer[3] & 0x2))
     return false;

   /* etc2 t-mode? */
   static const int lookup[8] = { 0, 1, 2, 3, -4, -3, -2, -1 };
   const int R_plus_dR = (buffer[0] >> 3) + lookup[buffer[0] & 0x7];

   if (R_plus_dR < 0 || R_plus_dR > 31)
      return true;

   return false;
}

void
etna_etc2_calculate_blocks(uint8_t *buffer, unsigned stride,
                           unsigned width, unsigned height,
                           enum pipe_format format,
                           struct util_dynarray *offsets)
{
   const unsigned bw = util_format_get_blockwidth(format);
   const unsigned bh = util_format_get_blockheight(format);
   const unsigned bs = util_format_get_blocksize(format);
   bool punchthrough_alpha = false;
   unsigned offset = 0;
   const uint8_t *base = buffer;

   if (format == PIPE_FORMAT_ETC2_RGB8A1 ||
       format == PIPE_FORMAT_ETC2_SRGB8A1)
      punchthrough_alpha = true;

   if (format == PIPE_FORMAT_ETC2_RGBA8 ||
       format == PIPE_FORMAT_ETC2_SRGBA8 ||
       format == PIPE_FORMAT_ETC2_SRGB8A1)
      offset = 8;

   for (unsigned y = 0; y < height; y += bh) {
      uint8_t *src = buffer;

      for (unsigned x = 0; x < width; x += bw) {
         if (needs_patching(src + offset, punchthrough_alpha))
            util_dynarray_append(offsets, unsigned, src + offset - base);

         src += bs;
      }

      buffer += stride;
   }
}

static inline void
swap_colors(uint8_t *buffer)
{
   const uint8_t r1a = (buffer[0] & 0x18) >> 3;
   const uint8_t r1b = (buffer[0] & 0x3);
   const uint8_t r1 = (r1a << 2) | r1b;
   const uint8_t g1 = (buffer[1] & 0xf0) >> 4;
   const uint8_t b1 = (buffer[1] & 0x0f);
   const uint8_t r2 = (buffer[2] & 0xf0) >> 4;
   const uint8_t g2 = buffer[2] & 0x0f;
   const uint8_t b2 = (buffer[3] & 0xf0) >> 4;
   const uint8_t rest = (buffer[3] & 0x0f);

   /* fixup R and dR used for t-mode detection */
   static const uint8_t fixup[16] = {
      0x04, 0x04, 0x04, 0x04,
      0x04, 0x04, 0x04, 0xe0,
      0x04, 0x04, 0xe0, 0xe0,
      0x04, 0xe0, 0xe0, 0xe0
   };

   /* swap colors */
   buffer[0] = fixup[r2] | ((r2 & 0x0c) << 1) | (r2 & 0x03);
   buffer[1] = (g2 << 4) | b2;
   buffer[2] = (r1 << 4) | g1;
   buffer[3] = (b1 << 4) | rest;
}

void
etna_etc2_patch(uint8_t *buffer, const struct util_dynarray *offsets)
{
   util_dynarray_foreach(offsets, unsigned, offset)
      swap_colors(buffer + *offset);
}
