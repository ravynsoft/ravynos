/*
 * Mesa 3-D graphics library
 *
 * Copyright 2012 Intel Corporation
 * Copyright 2013 Google
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Chad Versace <chad.versace@linux.intel.com>
 *    Frank Henigman <fjhenigman@google.com>
 */

#include <string.h>

#include "util/macros.h"
#include "util/u_math.h"
#include "util/rounding.h"
#include "isl_priv.h"

#if defined(__SSSE3__)
#include <tmmintrin.h>
#elif defined(__SSE2__)
#include <emmintrin.h>
#endif

#define FILE_DEBUG_FLAG DEBUG_TEXTURE

#define ALIGN_DOWN(a, b) ROUND_DOWN_TO(a, b)
#define ALIGN_UP(a, b) ALIGN(a, b)

/* Tile dimensions.  Width and span are in bytes, height is in pixels (i.e.
 * unitless).  A "span" is the most number of bytes we can copy from linear
 * to tiled without needing to calculate a new destination address.
 */
static const uint32_t xtile_width = 512;
static const uint32_t xtile_height = 8;
static const uint32_t xtile_span = 64;
static const uint32_t ytile_width = 128;
static const uint32_t ytile_height = 32;
static const uint32_t ytile_span = 16;

static inline uint32_t
ror(uint32_t n, uint32_t d)
{
   return (n >> d) | (n << (32 - d));
}

// bswap32 already exists as a macro on some platforms (FreeBSD)
#ifndef bswap32
static inline uint32_t
bswap32(uint32_t n)
{
#if defined(HAVE___BUILTIN_BSWAP32)
   return __builtin_bswap32(n);
#else
   return (n >> 24) |
          ((n >> 8) & 0x0000ff00) |
          ((n << 8) & 0x00ff0000) |
          (n << 24);
#endif
}
#endif

/**
 * Copy RGBA to BGRA - swap R and B.
 */
static inline void *
rgba8_copy(void *dst, const void *src, size_t bytes)
{
   uint32_t *d = dst;
   uint32_t const *s = src;

   assert(bytes % 4 == 0);

   while (bytes >= 4) {
      *d = ror(bswap32(*s), 8);
      d += 1;
      s += 1;
      bytes -= 4;
   }
   return dst;
}

#ifdef __SSSE3__
static const uint8_t rgba8_permutation[16] =
   { 2,1,0,3, 6,5,4,7, 10,9,8,11, 14,13,12,15 };

static inline void
rgba8_copy_16_aligned_dst(void *dst, const void *src)
{
   _mm_store_si128(dst,
                   _mm_shuffle_epi8(_mm_loadu_si128(src),
                                    *(__m128i *)rgba8_permutation));
}

static inline void
rgba8_copy_16_aligned_src(void *dst, const void *src)
{
   _mm_storeu_si128(dst,
                    _mm_shuffle_epi8(_mm_load_si128(src),
                                     *(__m128i *)rgba8_permutation));
}

#elif defined(__SSE2__)
static inline void
rgba8_copy_16_aligned_dst(void *dst, const void *src)
{
   __m128i srcreg, dstreg, agmask, ag, rb, br;

   agmask = _mm_set1_epi32(0xFF00FF00);
   srcreg = _mm_loadu_si128((__m128i *)src);

   rb = _mm_andnot_si128(agmask, srcreg);
   ag = _mm_and_si128(agmask, srcreg);
   br = _mm_shufflehi_epi16(_mm_shufflelo_epi16(rb, _MM_SHUFFLE(2, 3, 0, 1)),
                            _MM_SHUFFLE(2, 3, 0, 1));
   dstreg = _mm_or_si128(ag, br);

   _mm_store_si128((__m128i *)dst, dstreg);
}

static inline void
rgba8_copy_16_aligned_src(void *dst, const void *src)
{
   __m128i srcreg, dstreg, agmask, ag, rb, br;

   agmask = _mm_set1_epi32(0xFF00FF00);
   srcreg = _mm_load_si128((__m128i *)src);

   rb = _mm_andnot_si128(agmask, srcreg);
   ag = _mm_and_si128(agmask, srcreg);
   br = _mm_shufflehi_epi16(_mm_shufflelo_epi16(rb, _MM_SHUFFLE(2, 3, 0, 1)),
                            _MM_SHUFFLE(2, 3, 0, 1));
   dstreg = _mm_or_si128(ag, br);

   _mm_storeu_si128((__m128i *)dst, dstreg);
}
#endif

/**
 * Copy RGBA to BGRA - swap R and B, with the destination 16-byte aligned.
 */
static inline void *
rgba8_copy_aligned_dst(void *dst, const void *src, size_t bytes)
{
   assert(bytes == 0 || !(((uintptr_t)dst) & 0xf));

#if defined(__SSSE3__) || defined(__SSE2__)
   if (bytes == 64) {
      rgba8_copy_16_aligned_dst(dst +  0, src +  0);
      rgba8_copy_16_aligned_dst(dst + 16, src + 16);
      rgba8_copy_16_aligned_dst(dst + 32, src + 32);
      rgba8_copy_16_aligned_dst(dst + 48, src + 48);
      return dst;
   }

   while (bytes >= 16) {
      rgba8_copy_16_aligned_dst(dst, src);
      src += 16;
      dst += 16;
      bytes -= 16;
   }
#endif

   rgba8_copy(dst, src, bytes);

   return dst;
}

/**
 * Copy RGBA to BGRA - swap R and B, with the source 16-byte aligned.
 */
static inline void *
rgba8_copy_aligned_src(void *dst, const void *src, size_t bytes)
{
   assert(bytes == 0 || !(((uintptr_t)src) & 0xf));

#if defined(__SSSE3__) || defined(__SSE2__)
   if (bytes == 64) {
      rgba8_copy_16_aligned_src(dst +  0, src +  0);
      rgba8_copy_16_aligned_src(dst + 16, src + 16);
      rgba8_copy_16_aligned_src(dst + 32, src + 32);
      rgba8_copy_16_aligned_src(dst + 48, src + 48);
      return dst;
   }

   while (bytes >= 16) {
      rgba8_copy_16_aligned_src(dst, src);
      src += 16;
      dst += 16;
      bytes -= 16;
   }
#endif

   rgba8_copy(dst, src, bytes);

   return dst;
}

/**
 * Each row from y0 to y1 is copied in three parts: [x0,x1), [x1,x2), [x2,x3).
 * These ranges are in bytes, i.e. pixels * bytes-per-pixel.
 * The first and last ranges must be shorter than a "span" (the longest linear
 * stretch within a tile) and the middle must equal a whole number of spans.
 * Ranges may be empty.  The region copied must land entirely within one tile.
 * 'dst' is the start of the tile and 'src' is the corresponding
 * address to copy from, though copying begins at (x0, y0).
 * To enable swizzling 'swizzle_bit' must be 1<<6, otherwise zero.
 * Swizzling flips bit 6 in the copy destination offset, when certain other
 * bits are set in it.
 */
typedef void (*tile_copy_fn)(uint32_t x0, uint32_t x1, uint32_t x2, uint32_t x3,
                             uint32_t y0, uint32_t y1,
                             char *dst, const char *src,
                             int32_t linear_pitch,
                             uint32_t swizzle_bit,
                             isl_memcpy_type copy_type);

/**
 * Copy texture data from linear to X tile layout.
 *
 * \copydoc tile_copy_fn
 *
 * The mem_copy parameters allow the user to specify an alternative mem_copy
 * function that, for instance, may do RGBA -> BGRA swizzling.  The first
 * function must handle any memory alignment while the second function must
 * only handle 16-byte alignment in whichever side (source or destination) is
 * tiled.
 */
static inline void
linear_to_xtiled(uint32_t x0, uint32_t x1, uint32_t x2, uint32_t x3,
                 uint32_t y0, uint32_t y1,
                 char *dst, const char *src,
                 int32_t src_pitch,
                 uint32_t swizzle_bit,
                 isl_mem_copy_fn mem_copy,
                 isl_mem_copy_fn mem_copy_align16)
{
   /* The copy destination offset for each range copied is the sum of
    * an X offset 'x0' or 'xo' and a Y offset 'yo.'
    */
   uint32_t xo, yo;

   src += (ptrdiff_t)y0 * src_pitch;

   for (yo = y0 * xtile_width; yo < y1 * xtile_width; yo += xtile_width) {
      /* Bits 9 and 10 of the copy destination offset control swizzling.
       * Only 'yo' contributes to those bits in the total offset,
       * so calculate 'swizzle' just once per row.
       * Move bits 9 and 10 three and four places respectively down
       * to bit 6 and xor them.
       */
      uint32_t swizzle = ((yo >> 3) ^ (yo >> 4)) & swizzle_bit;

      mem_copy(dst + ((x0 + yo) ^ swizzle), src + x0, x1 - x0);

      for (xo = x1; xo < x2; xo += xtile_span) {
         mem_copy_align16(dst + ((xo + yo) ^ swizzle), src + xo, xtile_span);
      }

      mem_copy_align16(dst + ((xo + yo) ^ swizzle), src + x2, x3 - x2);

      src += src_pitch;
   }
}

/**
 * Copy texture data from linear to Y tile layout.
 *
 * \copydoc tile_copy_fn
 */
static inline void
linear_to_ytiled(uint32_t x0, uint32_t x1, uint32_t x2, uint32_t x3,
                 uint32_t y0, uint32_t y3,
                 char *dst, const char *src,
                 int32_t src_pitch,
                 uint32_t swizzle_bit,
                 isl_mem_copy_fn mem_copy,
                 isl_mem_copy_fn mem_copy_align16)
{
   /* Y tiles consist of columns that are 'ytile_span' wide (and the same height
    * as the tile).  Thus the destination offset for (x,y) is the sum of:
    *   (x % column_width)                    // position within column
    *   (x / column_width) * bytes_per_column // column number * bytes per column
    *   y * column_width
    *
    * The copy destination offset for each range copied is the sum of
    * an X offset 'xo0' or 'xo' and a Y offset 'yo.'
    */
   const uint32_t column_width = ytile_span;
   const uint32_t bytes_per_column = column_width * ytile_height;

   uint32_t y1 = MIN2(y3, ALIGN_UP(y0, 4));
   uint32_t y2 = MAX2(y1, ALIGN_DOWN(y3, 4));

   uint32_t xo0 = (x0 % ytile_span) + (x0 / ytile_span) * bytes_per_column;
   uint32_t xo1 = (x1 % ytile_span) + (x1 / ytile_span) * bytes_per_column;

   /* Bit 9 of the destination offset control swizzling.
    * Only the X offset contributes to bit 9 of the total offset,
    * so swizzle can be calculated in advance for these X positions.
    * Move bit 9 three places down to bit 6.
    */
   uint32_t swizzle0 = (xo0 >> 3) & swizzle_bit;
   uint32_t swizzle1 = (xo1 >> 3) & swizzle_bit;

   uint32_t x, yo;

   src += (ptrdiff_t)y0 * src_pitch;

   if (y0 != y1) {
      for (yo = y0 * column_width; yo < y1 * column_width; yo += column_width) {
         uint32_t xo = xo1;
         uint32_t swizzle = swizzle1;

         mem_copy(dst + ((xo0 + yo) ^ swizzle0), src + x0, x1 - x0);

         /* Step by spans/columns.  As it happens, the swizzle bit flips
          * at each step so we don't need to calculate it explicitly.
          */
         for (x = x1; x < x2; x += ytile_span) {
            mem_copy_align16(dst + ((xo + yo) ^ swizzle), src + x, ytile_span);
            xo += bytes_per_column;
            swizzle ^= swizzle_bit;
         }

         mem_copy_align16(dst + ((xo + yo) ^ swizzle), src + x2, x3 - x2);

         src += src_pitch;
      }
   }

   for (yo = y1 * column_width; yo < y2 * column_width; yo += 4 * column_width) {
      uint32_t xo = xo1;
      uint32_t swizzle = swizzle1;

      if (x0 != x1) {
         mem_copy(dst + ((xo0 + yo + 0 * column_width) ^ swizzle0), src + x0 + 0 * src_pitch, x1 - x0);
         mem_copy(dst + ((xo0 + yo + 1 * column_width) ^ swizzle0), src + x0 + 1 * src_pitch, x1 - x0);
         mem_copy(dst + ((xo0 + yo + 2 * column_width) ^ swizzle0), src + x0 + 2 * src_pitch, x1 - x0);
         mem_copy(dst + ((xo0 + yo + 3 * column_width) ^ swizzle0), src + x0 + 3 * src_pitch, x1 - x0);
      }

      /* Step by spans/columns.  As it happens, the swizzle bit flips
       * at each step so we don't need to calculate it explicitly.
       */
      for (x = x1; x < x2; x += ytile_span) {
         mem_copy_align16(dst + ((xo + yo + 0 * column_width) ^ swizzle), src + x + 0 * src_pitch, ytile_span);
         mem_copy_align16(dst + ((xo + yo + 1 * column_width) ^ swizzle), src + x + 1 * src_pitch, ytile_span);
         mem_copy_align16(dst + ((xo + yo + 2 * column_width) ^ swizzle), src + x + 2 * src_pitch, ytile_span);
         mem_copy_align16(dst + ((xo + yo + 3 * column_width) ^ swizzle), src + x + 3 * src_pitch, ytile_span);
         xo += bytes_per_column;
         swizzle ^= swizzle_bit;
      }

      if (x2 != x3) {
         mem_copy_align16(dst + ((xo + yo + 0 * column_width) ^ swizzle), src + x2 + 0 * src_pitch, x3 - x2);
         mem_copy_align16(dst + ((xo + yo + 1 * column_width) ^ swizzle), src + x2 + 1 * src_pitch, x3 - x2);
         mem_copy_align16(dst + ((xo + yo + 2 * column_width) ^ swizzle), src + x2 + 2 * src_pitch, x3 - x2);
         mem_copy_align16(dst + ((xo + yo + 3 * column_width) ^ swizzle), src + x2 + 3 * src_pitch, x3 - x2);
      }

      src += 4 * src_pitch;
   }

   if (y2 != y3) {
      for (yo = y2 * column_width; yo < y3 * column_width; yo += column_width) {
         uint32_t xo = xo1;
         uint32_t swizzle = swizzle1;

         mem_copy(dst + ((xo0 + yo) ^ swizzle0), src + x0, x1 - x0);

         /* Step by spans/columns.  As it happens, the swizzle bit flips
          * at each step so we don't need to calculate it explicitly.
          */
         for (x = x1; x < x2; x += ytile_span) {
            mem_copy_align16(dst + ((xo + yo) ^ swizzle), src + x, ytile_span);
            xo += bytes_per_column;
            swizzle ^= swizzle_bit;
         }

         mem_copy_align16(dst + ((xo + yo) ^ swizzle), src + x2, x3 - x2);

         src += src_pitch;
      }
   }
}

/**
 * Copy texture data from linear to Tile-4 layout.
 *
 * \copydoc tile_copy_fn
 */
static inline void
linear_to_tile4(uint32_t x0, uint32_t x1, uint32_t x2, uint32_t x3,
                uint32_t y0, uint32_t y3,
                char *dst, const char *src,
                int32_t src_pitch,
                uint32_t swizzle_bit,
                isl_mem_copy_fn mem_copy,
                isl_mem_copy_fn mem_copy_align16)
{
   /* Tile 4 consist of columns that are 'ytile_span' wide and each 64B tile
    * block consists of 4 row of Y-tile ordered data.
    * Each 512B block within a 4kB tile contains 8 such block.
    *
    * To calculate the tiled  offset, we need to identify:
    * Block X and Block Y offset at each 512B block boundary in X and Y
    * direction.
    *
    * A Tile4 has the following layout :
    *
    *                |<------------- 128 B-------------------|
    *                _________________________________________
    * 512B blk(Blk0)^|  0 |  1 |  2 |  3 |  8 |  9 | 10 | 11 | ^ 512B blk(Blk1)
    * (cell 0..7))  v|  4 |  5 |  6 |  7 | 12 | 13 | 14 | 15 | v (cell 8..15))
    *                -----------------------------------------
    *                | 16 | 17 | 18 | 19 | 24 | 25 | 26 | 27 |
    *                | 20 | 21 | 22 | 23 | 28 | 29 | 30 | 31 |
    *                -----------------------------------------
    *                | 32 | 33 | 34 | 35 | 40 | 41 | 42 | 43 |
    *                | 36 | 37 | 38 | 39 | 44 | 45 | 46 | 47 |
    *                -----------------------------------------
    *                | 48 | 49 | 50 | 51 | 56 | 57 | 58 | 59 |
    *                | 52 | 53 | 54 | 55 | 60 | 61 | 62 | 63 |
    *                -----------------------------------------
    *
    * The tile is divided in 512B blocks[Blk0..Blk7], themselves made of 2
    * rows of 256B sub-blocks.
    *
    * Each sub-block is composed of 4 64B elements[cell(0)-cell(3)] (a cell
    * in the figure above).
    *
    * Each 64B cell represents 4 rows of data.[cell(0), cell(1), .., cell(63)]
    *
    *
    *   Block X - Adds 256B to offset when we encounter block boundary in
    *             X direction.(Ex: Blk 0 --> Blk 1(BlkX_off = 256))
    *   Block Y - Adds 512B to offset when we encounter block boundary in
    *             Y direction.(Ex: Blk 0 --> Blk 3(BlkY_off = 512))
    *
    *   (x / ytile_span) * cacheline_size_B //Byte offset in the X dir of
    *                                         the containing 64B block
    *   x % ytile_span //Byte offset in X dir within a 64B block/cacheline
    *
    *   (y % 4) * 16 // Byte offset of the Y dir within a 64B block/cacheline
    *   (y / 4) * 256// Byte offset of the Y dir within 512B block after 1 row
    *                   of 64B blocks/cachelines
    *
    * The copy destination offset for each range copied is the sum of
    * Block X offset 'BlkX_off', Block Y offset 'BlkY_off', X offset 'xo'
    * and a Y offset 'yo.'
    */
   const uint32_t column_width = ytile_span;
   const uint32_t tile4_blkh = 4;

   assert(ytile_span * tile4_blkh == 64);
   const uint32_t cacheline_size_B = 64;

   /* Find intermediate Y offsets that are aligned to a 64B element
    * (4 rows), so that we can do fully 64B memcpys on those.
    */
   uint32_t y1 = MIN2(y3, ALIGN_UP(y0, 4));
   uint32_t y2 = MAX2(y1, ALIGN_DOWN(y3, 4));

   /* xsb0 and xsb1 are the byte offset within a 256B sub block for x0 and x1 */
   uint32_t xsb0 = (x0 % ytile_span) + (x0 / ytile_span) * cacheline_size_B;
   uint32_t xsb1 = (x1 % ytile_span) + (x1 / ytile_span) * cacheline_size_B;

   uint32_t Blkxsb0_off = ALIGN_DOWN(xsb0, 256);
   uint32_t Blky0_off = (y0 / 8) * 512;

   uint32_t BlkX_off, BlkY_off;

   uint32_t x, yo, Y0, Y2;

   /* Y0 determines the initial byte offset in the Y direction */
   Y0 = (y0 / 4) * 256 + (y0 % 4) * ytile_span;

   /* Y2 determines the byte offset required for reaching y2 if y2 doesn't map
    * exactly to 512B block boundary
    */
   Y2 = y2 * 4 * column_width;

   src += (ptrdiff_t)y0 * src_pitch;

   /* To maximize memcpy speed, we do the copy in 3 parts :
    *   - copy the first lines that are not aligned to the 64B cell's height (4 rows)
    *   - copy the lines that are aligned to 64B cell's height
    *   - copy the remaining lines not making up for a full 64B cell's height
    */
   if (y0 != y1) {
      for (yo = Y0; yo < Y0 + (y1 - y0) * column_width; yo += column_width) {
         uint32_t xo = xsb1;

         if (x0 != x1)
            mem_copy(dst + (Blky0_off + Blkxsb0_off) + (xsb0 + yo), src + x0, x1 - x0);

         for (x = x1; x < x2; x += ytile_span) {
            BlkX_off = ALIGN_DOWN(xo, 256);

            mem_copy_align16(dst + (Blky0_off + BlkX_off) + (xo + yo), src + x, ytile_span);
            xo += cacheline_size_B;
         }

         if (x3 != x2) {
            BlkX_off = ALIGN_DOWN(xo, 256);
            mem_copy_align16(dst + (Blky0_off + BlkX_off) + (xo + yo), src + x2, x3 - x2);
         }

         src += src_pitch;
      }
   }

   for (yo = y1 * 4 * column_width; yo < y2 * 4 * column_width; yo += 16 * column_width) {
      uint32_t xo = xsb1;
      BlkY_off = ALIGN_DOWN(yo, 512);

      if (x0 != x1) {
         mem_copy(dst + (BlkY_off + Blkxsb0_off) + (xsb0 + yo + 0 * column_width),
                  src + x0 + 0 * src_pitch, x1 - x0);
         mem_copy(dst + (BlkY_off + Blkxsb0_off) + (xsb0 + yo + 1 * column_width),
                  src + x0 + 1 * src_pitch, x1 - x0);
         mem_copy(dst + (BlkY_off + Blkxsb0_off) + (xsb0 + yo + 2 * column_width),
                  src + x0 + 2 * src_pitch, x1 - x0);
         mem_copy(dst + (BlkY_off + Blkxsb0_off) + (xsb0 + yo + 3 * column_width),
                  src + x0 + 3 * src_pitch, x1 - x0);
      }

      for (x = x1; x < x2; x += ytile_span) {
         BlkX_off = ALIGN_DOWN(xo, 256);

         mem_copy_align16(dst + (BlkY_off + BlkX_off) + (xo + yo+ 0 * column_width),
                          src + x + 0 * src_pitch, ytile_span);
         mem_copy_align16(dst + (BlkY_off + BlkX_off) + (xo + yo + 1 * column_width),
                          src + x + 1 * src_pitch, ytile_span);
         mem_copy_align16(dst + (BlkY_off + BlkX_off) + (xo + yo + 2 * column_width),
                          src + x + 2 * src_pitch, ytile_span);
         mem_copy_align16(dst + (BlkY_off + BlkX_off) + (xo + yo + 3 * column_width),
                          src + x + 3 * src_pitch, ytile_span);

         xo += cacheline_size_B;
      }

      if (x2 != x3) {
         BlkX_off = ALIGN_DOWN(xo, 256);

         mem_copy(dst + (BlkY_off + BlkX_off) + (xo + yo + 0 * column_width),
                  src + x2 + 0 * src_pitch, x3 - x2);
         mem_copy(dst + (BlkY_off + BlkX_off) + (xo + yo + 1 * column_width),
                  src + x2 + 1 * src_pitch, x3 - x2);
         mem_copy(dst + (BlkY_off + BlkX_off) + (xo + yo + 2 * column_width),
                  src + x2 + 2 * src_pitch, x3 - x2);
         mem_copy(dst + (BlkY_off + BlkX_off) + (xo + yo + 3 * column_width),
                  src + x2 + 3 * src_pitch, x3 - x2);
      }

      src += 4 * src_pitch;
   }

   if (y2 != y3) {
      for (yo = Y2; yo < Y2 + (y3 - y2) * column_width; yo += column_width) {
         uint32_t xo = xsb1;
         BlkY_off = ALIGN_DOWN(yo, 512);

         if (x0 != x1)
            mem_copy(dst + (BlkY_off + Blkxsb0_off) + (xsb0 + yo), src + x0, x1 - x0);

         for (x = x1; x < x2; x += ytile_span) {
            BlkX_off = ALIGN_DOWN(xo, 256);

            mem_copy_align16(dst + (BlkY_off + BlkX_off) + (xo + yo), src + x, ytile_span);
            xo += cacheline_size_B;
         }

         if (x3 != x2) {
            BlkX_off = ALIGN_DOWN(xo, 256);
            mem_copy_align16(dst + (BlkY_off + BlkX_off) + (xo + yo), src + x2, x3 - x2);
         }

         src += src_pitch;
      }
   }
}

/**
 * Copy texture data from X tile layout to linear.
 *
 * \copydoc tile_copy_fn
 */
static inline void
xtiled_to_linear(uint32_t x0, uint32_t x1, uint32_t x2, uint32_t x3,
                 uint32_t y0, uint32_t y1,
                 char *dst, const char *src,
                 int32_t dst_pitch,
                 uint32_t swizzle_bit,
                 isl_mem_copy_fn mem_copy,
                 isl_mem_copy_fn mem_copy_align16)
{
   /* The copy destination offset for each range copied is the sum of
    * an X offset 'x0' or 'xo' and a Y offset 'yo.'
    */
   uint32_t xo, yo;

   dst += (ptrdiff_t)y0 * dst_pitch;

   for (yo = y0 * xtile_width; yo < y1 * xtile_width; yo += xtile_width) {
      /* Bits 9 and 10 of the copy destination offset control swizzling.
       * Only 'yo' contributes to those bits in the total offset,
       * so calculate 'swizzle' just once per row.
       * Move bits 9 and 10 three and four places respectively down
       * to bit 6 and xor them.
       */
      uint32_t swizzle = ((yo >> 3) ^ (yo >> 4)) & swizzle_bit;

      mem_copy(dst + x0, src + ((x0 + yo) ^ swizzle), x1 - x0);

      for (xo = x1; xo < x2; xo += xtile_span) {
         mem_copy_align16(dst + xo, src + ((xo + yo) ^ swizzle), xtile_span);
      }

      mem_copy_align16(dst + x2, src + ((xo + yo) ^ swizzle), x3 - x2);

      dst += dst_pitch;
   }
}

 /**
 * Copy texture data from Y tile layout to linear.
 *
 * \copydoc tile_copy_fn
 */
static inline void
ytiled_to_linear(uint32_t x0, uint32_t x1, uint32_t x2, uint32_t x3,
                 uint32_t y0, uint32_t y3,
                 char *dst, const char *src,
                 int32_t dst_pitch,
                 uint32_t swizzle_bit,
                 isl_mem_copy_fn mem_copy,
                 isl_mem_copy_fn mem_copy_align16)
{
   /* Y tiles consist of columns that are 'ytile_span' wide (and the same height
    * as the tile).  Thus the destination offset for (x,y) is the sum of:
    *   (x % column_width)                    // position within column
    *   (x / column_width) * bytes_per_column // column number * bytes per column
    *   y * column_width
    *
    * The copy destination offset for each range copied is the sum of
    * an X offset 'xo0' or 'xo' and a Y offset 'yo.'
    */
   const uint32_t column_width = ytile_span;
   const uint32_t bytes_per_column = column_width * ytile_height;

   uint32_t y1 = MIN2(y3, ALIGN_UP(y0, 4));
   uint32_t y2 = MAX2(y1, ALIGN_DOWN(y3, 4));

   uint32_t xo0 = (x0 % ytile_span) + (x0 / ytile_span) * bytes_per_column;
   uint32_t xo1 = (x1 % ytile_span) + (x1 / ytile_span) * bytes_per_column;

   /* Bit 9 of the destination offset control swizzling.
    * Only the X offset contributes to bit 9 of the total offset,
    * so swizzle can be calculated in advance for these X positions.
    * Move bit 9 three places down to bit 6.
    */
   uint32_t swizzle0 = (xo0 >> 3) & swizzle_bit;
   uint32_t swizzle1 = (xo1 >> 3) & swizzle_bit;

   uint32_t x, yo;

   dst += (ptrdiff_t)y0 * dst_pitch;

   if (y0 != y1) {
      for (yo = y0 * column_width; yo < y1 * column_width; yo += column_width) {
         uint32_t xo = xo1;
         uint32_t swizzle = swizzle1;

         mem_copy(dst + x0, src + ((xo0 + yo) ^ swizzle0), x1 - x0);

         /* Step by spans/columns.  As it happens, the swizzle bit flips
          * at each step so we don't need to calculate it explicitly.
          */
         for (x = x1; x < x2; x += ytile_span) {
            mem_copy_align16(dst + x, src + ((xo + yo) ^ swizzle), ytile_span);
            xo += bytes_per_column;
            swizzle ^= swizzle_bit;
         }

         mem_copy_align16(dst + x2, src + ((xo + yo) ^ swizzle), x3 - x2);

         dst += dst_pitch;
      }
   }

   for (yo = y1 * column_width; yo < y2 * column_width; yo += 4 * column_width) {
      uint32_t xo = xo1;
      uint32_t swizzle = swizzle1;

      if (x0 != x1) {
         mem_copy(dst + x0 + 0 * dst_pitch, src + ((xo0 + yo + 0 * column_width) ^ swizzle0), x1 - x0);
         mem_copy(dst + x0 + 1 * dst_pitch, src + ((xo0 + yo + 1 * column_width) ^ swizzle0), x1 - x0);
         mem_copy(dst + x0 + 2 * dst_pitch, src + ((xo0 + yo + 2 * column_width) ^ swizzle0), x1 - x0);
         mem_copy(dst + x0 + 3 * dst_pitch, src + ((xo0 + yo + 3 * column_width) ^ swizzle0), x1 - x0);
      }

      /* Step by spans/columns.  As it happens, the swizzle bit flips
       * at each step so we don't need to calculate it explicitly.
       */
      for (x = x1; x < x2; x += ytile_span) {
         mem_copy_align16(dst + x + 0 * dst_pitch, src + ((xo + yo + 0 * column_width) ^ swizzle), ytile_span);
         mem_copy_align16(dst + x + 1 * dst_pitch, src + ((xo + yo + 1 * column_width) ^ swizzle), ytile_span);
         mem_copy_align16(dst + x + 2 * dst_pitch, src + ((xo + yo + 2 * column_width) ^ swizzle), ytile_span);
         mem_copy_align16(dst + x + 3 * dst_pitch, src + ((xo + yo + 3 * column_width) ^ swizzle), ytile_span);
         xo += bytes_per_column;
         swizzle ^= swizzle_bit;
      }

      if (x2 != x3) {
         mem_copy_align16(dst + x2 + 0 * dst_pitch, src + ((xo + yo + 0 * column_width) ^ swizzle), x3 - x2);
         mem_copy_align16(dst + x2 + 1 * dst_pitch, src + ((xo + yo + 1 * column_width) ^ swizzle), x3 - x2);
         mem_copy_align16(dst + x2 + 2 * dst_pitch, src + ((xo + yo + 2 * column_width) ^ swizzle), x3 - x2);
         mem_copy_align16(dst + x2 + 3 * dst_pitch, src + ((xo + yo + 3 * column_width) ^ swizzle), x3 - x2);
      }

      dst += 4 * dst_pitch;
   }

   if (y2 != y3) {
      for (yo = y2 * column_width; yo < y3 * column_width; yo += column_width) {
         uint32_t xo = xo1;
         uint32_t swizzle = swizzle1;

         mem_copy(dst + x0, src + ((xo0 + yo) ^ swizzle0), x1 - x0);

         /* Step by spans/columns.  As it happens, the swizzle bit flips
          * at each step so we don't need to calculate it explicitly.
          */
         for (x = x1; x < x2; x += ytile_span) {
            mem_copy_align16(dst + x, src + ((xo + yo) ^ swizzle), ytile_span);
            xo += bytes_per_column;
            swizzle ^= swizzle_bit;
         }

         mem_copy_align16(dst + x2, src + ((xo + yo) ^ swizzle), x3 - x2);

         dst += dst_pitch;
      }
   }
}


/**
 * Copy texture data from linear to Tile-4 layout.
 *
 * \copydoc tile_copy_fn
 */
static inline void
tile4_to_linear(uint32_t x0, uint32_t x1, uint32_t x2, uint32_t x3,
                uint32_t y0, uint32_t y3,
                char *dst, const char *src,
                int32_t dst_pitch,
                uint32_t swizzle_bit,
                isl_mem_copy_fn mem_copy,
                isl_mem_copy_fn mem_copy_align16)
{

   /* Tile 4 consist of columns that are 'ytile_span' wide and each 64B tile block
    * consists of 4 row of Y-tile ordered data.
    * Each 512B block within a 4kB tile contains 8 such block.
    *
    * To calculate the tiled  offset, we need to identify:
    * Block X and Block Y offset at each 512B block boundary in X and Y direction.
    *
    * Refer to the Tile4 layout diagram in linear_to_tile4() function.
    *
    * The tile is divided in 512B blocks[Blk0..Blk7], themselves made of 2
    * rows of 256B sub-blocks
    *
    * Each sub-block is composed of 4 64B elements[cell(0)-cell(3)].
    *
    * Each 64B cell represents 4 rows of data.[cell(0), cell(1), .., cell(63)]
    *
    *
    *   Block X - Adds 256B to offset when we encounter block boundary in
    *             X direction.(Ex: Blk 0 --> Blk 1(BlkX_off = 256))
    *   Block Y - Adds 512B to offset when we encounter block boundary in
    *             Y direction.(Ex: Blk 0 --> Blk 3(BlkY_off = 512))
    *
    *   (x / ytile_span) * cacheline_size_B //Byte offset in the X dir of the
    *                                         containing 64B block
    *   x % ytile_span //Byte offset in X dir within a 64B block/cacheline
    *
    *   (y % 4) * 16 // Byte offset of the Y dir within a 64B block/cacheline
    *   (y / 4) * 256// Byte offset of the Y dir within 512B block after 1 row
    *                   of 64B blocks/cachelines
    *
    * The copy destination offset for each range copied is the sum of
    * Block X offset 'BlkX_off', Block Y offset 'BlkY_off', X offset 'xo'
    * and a Y offset 'yo.'
    */

   const uint32_t column_width = ytile_span;
   const uint32_t tile4_blkh = 4;

   assert(ytile_span * tile4_blkh == 64);
   const uint32_t cacheline_size_B = 64;

   /* Find intermediate Y offsets that are aligned to a 64B element
    * (4 rows), so that we can do fully 64B memcpys on those.
    */
   uint32_t y1 = MIN2(y3, ALIGN_UP(y0, 4));
   uint32_t y2 = MAX2(y1, ALIGN_DOWN(y3, 4));

   /* xsb0 and xsb1 are the byte offset within a 256B sub block for x0 and x1 */
   uint32_t xsb0 = (x0 % ytile_span) + (x0 / ytile_span) * cacheline_size_B;
   uint32_t xsb1 = (x1 % ytile_span) + (x1 / ytile_span) * cacheline_size_B;

   uint32_t Blkxsb0_off = ALIGN_DOWN(xsb0, 256);
   uint32_t Blky0_off = (y0 / 8) * 512;

   uint32_t BlkX_off, BlkY_off;

   uint32_t x, yo, Y0, Y2;

   /* Y0 determines the initial byte offset in the Y direction */
   Y0 = (y0 / 4) * 256 + (y0 % 4) * 16;

   /* Y2 determines the byte offset required for reaching y2 if y2 doesn't map
    * exactly to 512B block boundary
    */
   Y2 = y2 * 4 * column_width;

   dst += (ptrdiff_t)y0 * dst_pitch;

   /* To maximize memcpy speed, we do the copy in 3 parts :
    *   - copy the first lines that are not aligned to the 64B cell's height (4 rows)
    *   - copy the lines that are aligned to 64B cell's height
    *   - copy the remaining lines not making up for a full 64B cell's height
    */
   if (y0 != y1) {
      for (yo = Y0; yo < Y0 + (y1 - y0) * column_width; yo += column_width) {
         uint32_t xo = xsb1;

         if (x0 != x1)
            mem_copy(dst + x0, src + (Blky0_off + Blkxsb0_off) + (xsb0 + yo), x1 - x0);

         for (x = x1; x < x2; x += ytile_span) {
            BlkX_off = ALIGN_DOWN(xo, 256);

            mem_copy_align16(dst + x, src + (Blky0_off + BlkX_off) + (xo + yo), ytile_span);
            xo += cacheline_size_B;
         }

         if (x3 != x2) {
            BlkX_off = ALIGN_DOWN(xo, 256);
            mem_copy_align16(dst + x2, src + (Blky0_off + BlkX_off) + (xo + yo), x3 - x2);
         }

         dst += dst_pitch;
      }
   }

   for (yo = y1 * 4 * column_width; yo < y2 * 4 * column_width; yo += 16 * column_width) {
      uint32_t xo = xsb1;
      BlkY_off = ALIGN_DOWN(yo, 512);

      if (x0 != x1) {
         mem_copy(dst + x0 + 0 * dst_pitch,
                  src + (BlkY_off + Blkxsb0_off) + (xsb0 + yo + 0 * column_width),
                  x1 - x0);
         mem_copy(dst + x0 + 1 * dst_pitch,
                  src + (BlkY_off + Blkxsb0_off) + (xsb0 + yo + 1 * column_width),
                  x1 - x0);
         mem_copy(dst + x0 + 2 * dst_pitch,
                  src + (BlkY_off + Blkxsb0_off) + (xsb0 + yo + 2 * column_width),
                  x1 - x0);
         mem_copy(dst + x0 + 3 * dst_pitch,
                  src + (BlkY_off + Blkxsb0_off) + (xsb0 + yo + 3 * column_width),
                  x1 - x0);
      }

      for (x = x1; x < x2; x += ytile_span) {
         BlkX_off = ALIGN_DOWN(xo, 256);

         mem_copy_align16(dst + x + 0 * dst_pitch,
                          src + (BlkY_off + BlkX_off) + (xo + yo + 0 * column_width),
                          ytile_span);
         mem_copy_align16(dst + x + 1 * dst_pitch,
                          src + (BlkY_off + BlkX_off) + (xo + yo + 1 * column_width),
                          ytile_span);
         mem_copy_align16(dst + x + 2 * dst_pitch,
                          src + (BlkY_off + BlkX_off) + (xo + yo + 2 * column_width),
                          ytile_span);
         mem_copy_align16(dst + x + 3 * dst_pitch,
                          src + (BlkY_off + BlkX_off) + (xo + yo + 3 * column_width),
                          ytile_span);

         xo += cacheline_size_B;
      }

      if (x2 != x3) {
         BlkX_off = ALIGN_DOWN(xo, 256);

         mem_copy(dst + x2 + 0 * dst_pitch,
                  src + (BlkY_off + BlkX_off) + (xo + yo + 0 * column_width),
                  x3 - x2);
         mem_copy(dst + x2 + 1 * dst_pitch,
                  src + (BlkY_off + BlkX_off) + (xo + yo + 1 * column_width),
                  x3 - x2);
         mem_copy(dst + x2 + 2 * dst_pitch,
                  src + (BlkY_off + BlkX_off) + (xo + yo + 2 * column_width),
                  x3 - x2);
         mem_copy(dst + x2 + 3 * dst_pitch,
                  src + (BlkY_off + BlkX_off) + (xo + yo + 3 * column_width),
                  x3 - x2);
      }

      dst += 4 * dst_pitch;
   }

   if (y2 != y3) {
      for (yo = Y2; yo < Y2 + (y3 - y2) * column_width; yo += column_width) {
         uint32_t xo = xsb1;
         BlkY_off = ALIGN_DOWN(yo, 512);

         if (x0 != x1)
            mem_copy(dst + x0, src + (BlkY_off + Blkxsb0_off) + (xsb0 + yo), x1 - x0);

         for (x = x1; x < x2; x += ytile_span) {
            BlkX_off = ALIGN_DOWN(xo, 256);

            mem_copy_align16(dst + x, src + (BlkY_off + BlkX_off) + (xo + yo), ytile_span);
            xo += cacheline_size_B;
         }

         if (x3 != x2) {
            BlkX_off = ALIGN_DOWN(xo, 256);
            mem_copy_align16(dst + x2, src + (BlkY_off + BlkX_off) + (xo + yo), x3 - x2);
         }

         dst += dst_pitch;
      }
   }
}

#if defined(INLINE_SSE41)
static ALWAYS_INLINE void *
_memcpy_streaming_load(void *dest, const void *src, size_t count)
{
   if (count == 16) {
      __m128i val = _mm_stream_load_si128((__m128i *)src);
      _mm_storeu_si128((__m128i *)dest, val);
      return dest;
   } else if (count == 64) {
      __m128i val0 = _mm_stream_load_si128(((__m128i *)src) + 0);
      __m128i val1 = _mm_stream_load_si128(((__m128i *)src) + 1);
      __m128i val2 = _mm_stream_load_si128(((__m128i *)src) + 2);
      __m128i val3 = _mm_stream_load_si128(((__m128i *)src) + 3);
      _mm_storeu_si128(((__m128i *)dest) + 0, val0);
      _mm_storeu_si128(((__m128i *)dest) + 1, val1);
      _mm_storeu_si128(((__m128i *)dest) + 2, val2);
      _mm_storeu_si128(((__m128i *)dest) + 3, val3);
      return dest;
   } else {
      assert(count < 64); /* and (count < 16) for ytiled */
      return memcpy(dest, src, count);
   }
}
#endif

static isl_mem_copy_fn
choose_copy_function(isl_memcpy_type copy_type)
{
   switch(copy_type) {
   case ISL_MEMCPY:
      return memcpy;
   case ISL_MEMCPY_BGRA8:
      return rgba8_copy;
   case ISL_MEMCPY_STREAMING_LOAD:
#if defined(INLINE_SSE41)
      return _memcpy_streaming_load;
#else
      unreachable("ISL_MEMCOPY_STREAMING_LOAD requires sse4.1");
#endif
   case ISL_MEMCPY_INVALID:
      unreachable("invalid copy_type");
   }
   unreachable("unhandled copy_type");
   return NULL;
}

/**
 * Copy texture data from linear to X tile layout, faster.
 *
 * Same as \ref linear_to_xtiled but faster, because it passes constant
 * parameters for common cases, allowing the compiler to inline code
 * optimized for those cases.
 *
 * \copydoc tile_copy_fn
 */
static FLATTEN void
linear_to_xtiled_faster(uint32_t x0, uint32_t x1, uint32_t x2, uint32_t x3,
                        uint32_t y0, uint32_t y1,
                        char *dst, const char *src,
                        int32_t src_pitch,
                        uint32_t swizzle_bit,
                        isl_memcpy_type copy_type)
{
   isl_mem_copy_fn mem_copy = choose_copy_function(copy_type);

   if (x0 == 0 && x3 == xtile_width && y0 == 0 && y1 == xtile_height) {
      if (mem_copy == memcpy)
         return linear_to_xtiled(0, 0, xtile_width, xtile_width, 0, xtile_height,
                                 dst, src, src_pitch, swizzle_bit, memcpy, memcpy);
      else if (mem_copy == rgba8_copy)
         return linear_to_xtiled(0, 0, xtile_width, xtile_width, 0, xtile_height,
                                 dst, src, src_pitch, swizzle_bit,
                                 rgba8_copy, rgba8_copy_aligned_dst);
      else
         unreachable("not reached");
   } else {
      if (mem_copy == memcpy)
         return linear_to_xtiled(x0, x1, x2, x3, y0, y1,
                                 dst, src, src_pitch, swizzle_bit,
                                 memcpy, memcpy);
      else if (mem_copy == rgba8_copy)
         return linear_to_xtiled(x0, x1, x2, x3, y0, y1,
                                 dst, src, src_pitch, swizzle_bit,
                                 rgba8_copy, rgba8_copy_aligned_dst);
      else
         unreachable("not reached");
   }
   linear_to_xtiled(x0, x1, x2, x3, y0, y1,
                    dst, src, src_pitch, swizzle_bit, mem_copy, mem_copy);
}

/**
 * Copy texture data from linear to Y tile layout, faster.
 *
 * Same as \ref linear_to_ytiled but faster, because it passes constant
 * parameters for common cases, allowing the compiler to inline code
 * optimized for those cases.
 *
 * \copydoc tile_copy_fn
 */
static FLATTEN void
linear_to_ytiled_faster(uint32_t x0, uint32_t x1, uint32_t x2, uint32_t x3,
                        uint32_t y0, uint32_t y1,
                        char *dst, const char *src,
                        int32_t src_pitch,
                        uint32_t swizzle_bit,
                        isl_memcpy_type copy_type)
{
   isl_mem_copy_fn mem_copy = choose_copy_function(copy_type);

   if (x0 == 0 && x3 == ytile_width && y0 == 0 && y1 == ytile_height) {
      if (mem_copy == memcpy)
         return linear_to_ytiled(0, 0, ytile_width, ytile_width, 0, ytile_height,
                                 dst, src, src_pitch, swizzle_bit, memcpy, memcpy);
      else if (mem_copy == rgba8_copy)
         return linear_to_ytiled(0, 0, ytile_width, ytile_width, 0, ytile_height,
                                 dst, src, src_pitch, swizzle_bit,
                                 rgba8_copy, rgba8_copy_aligned_dst);
      else
         unreachable("not reached");
   } else {
      if (mem_copy == memcpy)
         return linear_to_ytiled(x0, x1, x2, x3, y0, y1,
                                 dst, src, src_pitch, swizzle_bit, memcpy, memcpy);
      else if (mem_copy == rgba8_copy)
         return linear_to_ytiled(x0, x1, x2, x3, y0, y1,
                                 dst, src, src_pitch, swizzle_bit,
                                 rgba8_copy, rgba8_copy_aligned_dst);
      else
         unreachable("not reached");
   }
   linear_to_ytiled(x0, x1, x2, x3, y0, y1,
                    dst, src, src_pitch, swizzle_bit, mem_copy, mem_copy);
}

/**
 * Copy texture data from linear to tile 4 layout, faster.
 *
 * Same as \ref linear_to_tile4 but faster, because it passes constant
 * parameters for common cases, allowing the compiler to inline code
 * optimized for those cases.
 *
 * \copydoc tile_copy_fn
 */
static FLATTEN void
linear_to_tile4_faster(uint32_t x0, uint32_t x1, uint32_t x2, uint32_t x3,
                        uint32_t y0, uint32_t y1,
                        char *dst, const char *src,
                        int32_t src_pitch,
                        uint32_t swizzle_bit,
                        isl_memcpy_type copy_type)
{
   isl_mem_copy_fn mem_copy = choose_copy_function(copy_type);
   assert(swizzle_bit == 0);

   if (x0 == 0 && x3 == ytile_width && y0 == 0 && y1 == ytile_height) {
      if (mem_copy == memcpy)
         return linear_to_tile4(0, 0, ytile_width, ytile_width, 0, ytile_height,
                                 dst, src, src_pitch, swizzle_bit, memcpy, memcpy);
      else if (mem_copy == rgba8_copy)
         return linear_to_tile4(0, 0, ytile_width, ytile_width, 0, ytile_height,
                                 dst, src, src_pitch, swizzle_bit,
                                 rgba8_copy, rgba8_copy_aligned_dst);
      else
         unreachable("not reached");
   } else {
      if (mem_copy == memcpy)
         return linear_to_tile4(x0, x1, x2, x3, y0, y1,
                                 dst, src, src_pitch, swizzle_bit, memcpy, memcpy);
      else if (mem_copy == rgba8_copy)
         return linear_to_tile4(x0, x1, x2, x3, y0, y1,
                                 dst, src, src_pitch, swizzle_bit,
                                 rgba8_copy, rgba8_copy_aligned_dst);
      else
         unreachable("not reached");
   }
}

/**
 * Copy texture data from X tile layout to linear, faster.
 *
 * Same as \ref xtile_to_linear but faster, because it passes constant
 * parameters for common cases, allowing the compiler to inline code
 * optimized for those cases.
 *
 * \copydoc tile_copy_fn
 */
static FLATTEN void
xtiled_to_linear_faster(uint32_t x0, uint32_t x1, uint32_t x2, uint32_t x3,
                        uint32_t y0, uint32_t y1,
                        char *dst, const char *src,
                        int32_t dst_pitch,
                        uint32_t swizzle_bit,
                        isl_memcpy_type copy_type)
{
   isl_mem_copy_fn mem_copy = choose_copy_function(copy_type);

   if (x0 == 0 && x3 == xtile_width && y0 == 0 && y1 == xtile_height) {
      if (mem_copy == memcpy)
         return xtiled_to_linear(0, 0, xtile_width, xtile_width, 0, xtile_height,
                                 dst, src, dst_pitch, swizzle_bit, memcpy, memcpy);
      else if (mem_copy == rgba8_copy)
         return xtiled_to_linear(0, 0, xtile_width, xtile_width, 0, xtile_height,
                                 dst, src, dst_pitch, swizzle_bit,
                                 rgba8_copy, rgba8_copy_aligned_src);
#if defined(INLINE_SSE41)
      else if (mem_copy == _memcpy_streaming_load)
         return xtiled_to_linear(0, 0, xtile_width, xtile_width, 0, xtile_height,
                                 dst, src, dst_pitch, swizzle_bit,
                                 memcpy, _memcpy_streaming_load);
#endif
      else
         unreachable("not reached");
   } else {
      if (mem_copy == memcpy)
         return xtiled_to_linear(x0, x1, x2, x3, y0, y1,
                                 dst, src, dst_pitch, swizzle_bit, memcpy, memcpy);
      else if (mem_copy == rgba8_copy)
         return xtiled_to_linear(x0, x1, x2, x3, y0, y1,
                                 dst, src, dst_pitch, swizzle_bit,
                                 rgba8_copy, rgba8_copy_aligned_src);
#if defined(INLINE_SSE41)
      else if (mem_copy == _memcpy_streaming_load)
         return xtiled_to_linear(x0, x1, x2, x3, y0, y1,
                                 dst, src, dst_pitch, swizzle_bit,
                                 memcpy, _memcpy_streaming_load);
#endif
      else
         unreachable("not reached");
   }
   xtiled_to_linear(x0, x1, x2, x3, y0, y1,
                    dst, src, dst_pitch, swizzle_bit, mem_copy, mem_copy);
}

/**
 * Copy texture data from Y tile layout to linear, faster.
 *
 * Same as \ref ytile_to_linear but faster, because it passes constant
 * parameters for common cases, allowing the compiler to inline code
 * optimized for those cases.
 *
 * \copydoc tile_copy_fn
 */
static FLATTEN void
ytiled_to_linear_faster(uint32_t x0, uint32_t x1, uint32_t x2, uint32_t x3,
                        uint32_t y0, uint32_t y1,
                        char *dst, const char *src,
                        int32_t dst_pitch,
                        uint32_t swizzle_bit,
                        isl_memcpy_type copy_type)
{
   isl_mem_copy_fn mem_copy = choose_copy_function(copy_type);

   if (x0 == 0 && x3 == ytile_width && y0 == 0 && y1 == ytile_height) {
      if (mem_copy == memcpy)
         return ytiled_to_linear(0, 0, ytile_width, ytile_width, 0, ytile_height,
                                 dst, src, dst_pitch, swizzle_bit, memcpy, memcpy);
      else if (mem_copy == rgba8_copy)
         return ytiled_to_linear(0, 0, ytile_width, ytile_width, 0, ytile_height,
                                 dst, src, dst_pitch, swizzle_bit,
                                 rgba8_copy, rgba8_copy_aligned_src);
#if defined(INLINE_SSE41)
      else if (copy_type == ISL_MEMCPY_STREAMING_LOAD)
         return ytiled_to_linear(0, 0, ytile_width, ytile_width, 0, ytile_height,
                                 dst, src, dst_pitch, swizzle_bit,
                                 memcpy, _memcpy_streaming_load);
#endif
      else
         unreachable("not reached");
   } else {
      if (mem_copy == memcpy)
         return ytiled_to_linear(x0, x1, x2, x3, y0, y1,
                                 dst, src, dst_pitch, swizzle_bit, memcpy, memcpy);
      else if (mem_copy == rgba8_copy)
         return ytiled_to_linear(x0, x1, x2, x3, y0, y1,
                                 dst, src, dst_pitch, swizzle_bit,
                                 rgba8_copy, rgba8_copy_aligned_src);
#if defined(INLINE_SSE41)
      else if (copy_type == ISL_MEMCPY_STREAMING_LOAD)
         return ytiled_to_linear(x0, x1, x2, x3, y0, y1,
                                 dst, src, dst_pitch, swizzle_bit,
                                 memcpy, _memcpy_streaming_load);
#endif
      else
         unreachable("not reached");
   }
   ytiled_to_linear(x0, x1, x2, x3, y0, y1,
                    dst, src, dst_pitch, swizzle_bit, mem_copy, mem_copy);
}

/**
 * Copy texture data from tile4 layout to linear, faster.
 *
 * Same as \ref tile4_to_linear but faster, because it passes constant
 * parameters for common cases, allowing the compiler to inline code
 * optimized for those cases.
 *
 * \copydoc tile_copy_fn
 */
static FLATTEN void
tile4_to_linear_faster(uint32_t x0, uint32_t x1, uint32_t x2, uint32_t x3,
                        uint32_t y0, uint32_t y1,
                        char *dst, const char *src,
                        int32_t dst_pitch,
                        uint32_t swizzle_bit,
                        isl_memcpy_type copy_type)
{
   isl_mem_copy_fn mem_copy = choose_copy_function(copy_type);
   assert(swizzle_bit == 0);

   if (x0 == 0 && x3 == ytile_width && y0 == 0 && y1 == ytile_height) {
      if (mem_copy == memcpy)
         return tile4_to_linear(0, 0, ytile_width, ytile_width, 0, ytile_height,
                                 dst, src, dst_pitch, swizzle_bit, memcpy, memcpy);
      else if (mem_copy == rgba8_copy)
         return tile4_to_linear(0, 0, ytile_width, ytile_width, 0, ytile_height,
                                 dst, src, dst_pitch, swizzle_bit,
                                 rgba8_copy, rgba8_copy_aligned_src);
#if defined(INLINE_SSE41)
      else if (copy_type == ISL_MEMCPY_STREAMING_LOAD)
         return tile4_to_linear(0, 0, ytile_width, ytile_width, 0, ytile_height,
                                 dst, src, dst_pitch, swizzle_bit,
                                 memcpy, _memcpy_streaming_load);
#endif
      else
         unreachable("not reached");
   } else {
      if (mem_copy == memcpy)
         return tile4_to_linear(x0, x1, x2, x3, y0, y1,
                                 dst, src, dst_pitch, swizzle_bit, memcpy, memcpy);
      else if (mem_copy == rgba8_copy)
         return tile4_to_linear(x0, x1, x2, x3, y0, y1,
                                 dst, src, dst_pitch, swizzle_bit,
                                 rgba8_copy, rgba8_copy_aligned_src);
#if defined(INLINE_SSE41)
      else if (copy_type == ISL_MEMCPY_STREAMING_LOAD)
         return tile4_to_linear(x0, x1, x2, x3, y0, y1,
                                 dst, src, dst_pitch, swizzle_bit,
                                 memcpy, _memcpy_streaming_load);
#endif
      else
         unreachable("not reached");
   }
}

/**
 * Copy from linear to tiled texture.
 *
 * Divide the region given by X range [xt1, xt2) and Y range [yt1, yt2) into
 * pieces that do not cross tile boundaries and copy each piece with a tile
 * copy function (\ref tile_copy_fn).
 * The X range is in bytes, i.e. pixels * bytes-per-pixel.
 * The Y range is in pixels (i.e. unitless).
 * 'dst' is the address of (0, 0) in the destination tiled texture.
 * 'src' is the address of (xt1, yt1) in the source linear texture.
 */
static void
linear_to_tiled(uint32_t xt1, uint32_t xt2,
                      uint32_t yt1, uint32_t yt2,
                      char *dst, const char *src,
                      uint32_t dst_pitch, int32_t src_pitch,
                      bool has_swizzling,
                      enum isl_tiling tiling,
                      isl_memcpy_type copy_type)
{
   tile_copy_fn tile_copy;
   uint32_t xt0, xt3;
   uint32_t yt0, yt3;
   uint32_t xt, yt;
   uint32_t tw, th, span;
   uint32_t swizzle_bit = has_swizzling ? 1<<6 : 0;

   if (tiling == ISL_TILING_X) {
      tw = xtile_width;
      th = xtile_height;
      span = xtile_span;
      tile_copy = linear_to_xtiled_faster;
   } else if (tiling == ISL_TILING_Y0) {
      tw = ytile_width;
      th = ytile_height;
      span = ytile_span;
      tile_copy = linear_to_ytiled_faster;
   } else if (tiling == ISL_TILING_4) {
      tw = ytile_width;
      th = ytile_height;
      span = ytile_span;
      tile_copy = linear_to_tile4_faster;
   } else {
      unreachable("unsupported tiling");
   }

   /* Round out to tile boundaries. */
   xt0 = ALIGN_DOWN(xt1, tw);
   xt3 = ALIGN_UP  (xt2, tw);
   yt0 = ALIGN_DOWN(yt1, th);
   yt3 = ALIGN_UP  (yt2, th);

   /* Loop over all tiles to which we have something to copy.
    * 'xt' and 'yt' are the origin of the destination tile, whether copying
    * copying a full or partial tile.
    * tile_copy() copies one tile or partial tile.
    * Looping x inside y is the faster memory access pattern.
    */
   for (yt = yt0; yt < yt3; yt += th) {
      for (xt = xt0; xt < xt3; xt += tw) {
         /* The area to update is [x0,x3) x [y0,y1).
          * May not want the whole tile, hence the min and max.
          */
         uint32_t x0 = MAX2(xt1, xt);
         uint32_t y0 = MAX2(yt1, yt);
         uint32_t x3 = MIN2(xt2, xt + tw);
         uint32_t y1 = MIN2(yt2, yt + th);

         /* [x0,x3) is split into [x0,x1), [x1,x2), [x2,x3) such that
          * the middle interval is the longest span-aligned part.
          * The sub-ranges could be empty.
          */
         uint32_t x1, x2;
         x1 = ALIGN_UP(x0, span);
         if (x1 > x3)
            x1 = x2 = x3;
         else
            x2 = ALIGN_DOWN(x3, span);

         assert(x0 <= x1 && x1 <= x2 && x2 <= x3);
         assert(x1 - x0 < span && x3 - x2 < span);
         assert(x3 - x0 <= tw);
         assert((x2 - x1) % span == 0);

         /* Translate by (xt,yt) for single-tile copier. */
         tile_copy(x0-xt, x1-xt, x2-xt, x3-xt,
                   y0-yt, y1-yt,
                   dst + (ptrdiff_t)xt * th  +  (ptrdiff_t)yt        * dst_pitch,
                   src + (ptrdiff_t)xt - xt1 + ((ptrdiff_t)yt - yt1) * src_pitch,
                   src_pitch,
                   swizzle_bit,
                   copy_type);
      }
   }
}

/**
 * Copy from tiled to linear texture.
 *
 * Divide the region given by X range [xt1, xt2) and Y range [yt1, yt2) into
 * pieces that do not cross tile boundaries and copy each piece with a tile
 * copy function (\ref tile_copy_fn).
 * The X range is in bytes, i.e. pixels * bytes-per-pixel.
 * The Y range is in pixels (i.e. unitless).
 * 'dst' is the address of (xt1, yt1) in the destination linear texture.
 * 'src' is the address of (0, 0) in the source tiled texture.
 */
static void
tiled_to_linear(uint32_t xt1, uint32_t xt2,
                      uint32_t yt1, uint32_t yt2,
                      char *dst, const char *src,
                      int32_t dst_pitch, uint32_t src_pitch,
                      bool has_swizzling,
                      enum isl_tiling tiling,
                      isl_memcpy_type copy_type)
{
   tile_copy_fn tile_copy;
   uint32_t xt0, xt3;
   uint32_t yt0, yt3;
   uint32_t xt, yt;
   uint32_t tw, th, span;
   uint32_t swizzle_bit = has_swizzling ? 1<<6 : 0;

   if (tiling == ISL_TILING_X) {
      tw = xtile_width;
      th = xtile_height;
      span = xtile_span;
      tile_copy = xtiled_to_linear_faster;
   } else if (tiling == ISL_TILING_Y0) {
      tw = ytile_width;
      th = ytile_height;
      span = ytile_span;
      tile_copy = ytiled_to_linear_faster;
   } else if (tiling == ISL_TILING_4) {
      tw = ytile_width;
      th = ytile_height;
      span = ytile_span;
      tile_copy = tile4_to_linear_faster;
   } else {
      unreachable("unsupported tiling");
   }

#if defined(INLINE_SSE41)
   if (copy_type == ISL_MEMCPY_STREAMING_LOAD) {
      /* The hidden cacheline sized register used by movntdqa can apparently
       * give you stale data, so do an mfence to invalidate it.
       */
      _mm_mfence();
   }
#endif

   /* Round out to tile boundaries. */
   xt0 = ALIGN_DOWN(xt1, tw);
   xt3 = ALIGN_UP  (xt2, tw);
   yt0 = ALIGN_DOWN(yt1, th);
   yt3 = ALIGN_UP  (yt2, th);

   /* Loop over all tiles to which we have something to copy.
    * 'xt' and 'yt' are the origin of the destination tile, whether copying
    * copying a full or partial tile.
    * tile_copy() copies one tile or partial tile.
    * Looping x inside y is the faster memory access pattern.
    */
   for (yt = yt0; yt < yt3; yt += th) {
      for (xt = xt0; xt < xt3; xt += tw) {
         /* The area to update is [x0,x3) x [y0,y1).
          * May not want the whole tile, hence the min and max.
          */
         uint32_t x0 = MAX2(xt1, xt);
         uint32_t y0 = MAX2(yt1, yt);
         uint32_t x3 = MIN2(xt2, xt + tw);
         uint32_t y1 = MIN2(yt2, yt + th);

         /* [x0,x3) is split into [x0,x1), [x1,x2), [x2,x3) such that
          * the middle interval is the longest span-aligned part.
          * The sub-ranges could be empty.
          */
         uint32_t x1, x2;
         x1 = ALIGN_UP(x0, span);
         if (x1 > x3)
            x1 = x2 = x3;
         else
            x2 = ALIGN_DOWN(x3, span);

         assert(x0 <= x1 && x1 <= x2 && x2 <= x3);
         assert(x1 - x0 < span && x3 - x2 < span);
         assert(x3 - x0 <= tw);
         assert((x2 - x1) % span == 0);

         /* Translate by (xt,yt) for single-tile copier. */
         tile_copy(x0-xt, x1-xt, x2-xt, x3-xt,
                   y0-yt, y1-yt,
                   dst + (ptrdiff_t)xt - xt1 + ((ptrdiff_t)yt - yt1) * dst_pitch,
                   src + (ptrdiff_t)xt * th  +  (ptrdiff_t)yt        * src_pitch,
                   dst_pitch,
                   swizzle_bit,
                   copy_type);
      }
   }
}
