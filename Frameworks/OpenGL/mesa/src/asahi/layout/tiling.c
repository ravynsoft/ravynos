/*
 * Copyright 2021 Alyssa Rosenzweig
 * Copyright 2019 Collabora, Ltd.
 * SPDX-License-Identifier: MIT
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "util/macros.h"
#include "layout.h"

/* Z-order with rectangular (NxN or 2NxN) tiles, at most 128x128:
 *
 * 	[y6][x6][y5][x5][y4][x4]y3][x3][y2][x2][y1][x1][y0][x0]
 *
 * Efficient tiling algorithm described in
 * https://fgiesen.wordpress.com/2011/01/17/texture-tiling-and-swizzling/ but
 * for posterity, we split into X and Y parts, and are faced with the problem
 * of incrementing:
 *
 * 	0 [x6] 0 [x5] 0 [x4] 0 [x3] 0 [x2] 0 [x1] 0 [x0]
 *
 * To do so, we fill in the "holes" with 1's by adding the bitwise inverse of
 * the mask of bits we care about
 *
 * 	0 [x6] 0 [x5] 0 [x4] 0 [x3] 0 [x2] 0 [x1] 0 [x0]
 *    + 1  0   1  0   1  0   1  0   1  0   1  0   1  0
 *    ------------------------------------------------
 * 	1 [x6] 1 [x5] 1 [x4] 1 [x3] 1 [x2] 1 [x1] 1 [x0]
 *
 * Then when we add one, the holes are passed over by forcing carry bits high.
 * Finally, we need to zero out the holes, by ANDing with the mask of bits we
 * care about. In total, we get the expression (X + ~mask + 1) & mask, and
 * applying the two's complement identity, we are left with (X - mask) & mask
 */

#define MOD_POT(x, y) (x) & ((y)-1)

typedef struct {
   uint64_t lo;
   uint64_t hi;
} __attribute__((packed)) ail_uint128_t;

static uint32_t
ail_space_bits(unsigned x)
{
   assert(x < 128 && "offset must be inside the tile");

   return ((x & 1) << 0) | ((x & 2) << 1) | ((x & 4) << 2) | ((x & 8) << 3) |
          ((x & 16) << 4) | ((x & 32) << 5) | ((x & 64) << 6);
}

/*
 * Given a power-of-two block width/height, construct the mask of "X" bits. This
 * is found by restricting the full mask of alternating 0s and 1s to only cover
 * the bottom 2 * log2(dim) bits. That's the same as modding by dim^2.
 */
static uint32_t
ail_space_mask(unsigned x)
{
   assert(util_is_power_of_two_nonzero(x));

   return MOD_POT(0x55555555, x * x);
}

#define TILED_UNALIGNED_TYPE(element_t, is_store)                              \
   {                                                                           \
      enum pipe_format format = tiled_layout->format;                          \
      unsigned linear_pitch_el = linear_pitch_B / blocksize_B;                 \
      unsigned stride_el = tiled_layout->stride_el[level];                     \
      unsigned sx_el = util_format_get_nblocksx(format, sx_px);                \
      unsigned sy_el = util_format_get_nblocksy(format, sy_px);                \
      unsigned swidth_el = util_format_get_nblocksx(format, swidth_px);        \
      unsigned sheight_el = util_format_get_nblocksy(format, sheight_px);      \
      unsigned sx_end_el = sx_el + swidth_el;                                  \
      unsigned sy_end_el = sy_el + sheight_el;                                 \
                                                                               \
      struct ail_tile tile_size = tiled_layout->tilesize_el[level];            \
      unsigned tile_area_el = tile_size.width_el * tile_size.height_el;        \
      unsigned tiles_per_row = DIV_ROUND_UP(stride_el, tile_size.width_el);    \
      unsigned y_offs_el = ail_space_bits(MOD_POT(sy_el, tile_size.height_el)) \
                           << 1;                                               \
      unsigned x_offs_start_el =                                               \
         ail_space_bits(MOD_POT(sx_el, tile_size.width_el));                   \
      unsigned space_mask_x = ail_space_mask(tile_size.width_el);              \
      unsigned space_mask_y = ail_space_mask(tile_size.height_el) << 1;        \
      unsigned log2_tile_width_el = util_logbase2(tile_size.width_el);         \
      unsigned log2_tile_height_el = util_logbase2(tile_size.height_el);       \
                                                                               \
      element_t *linear = _linear;                                             \
      element_t *tiled = _tiled;                                               \
                                                                               \
      for (unsigned y_el = sy_el; y_el < sy_end_el; ++y_el) {                  \
         unsigned y_rowtile = y_el >> log2_tile_height_el;                     \
         unsigned y_tile = y_rowtile * tiles_per_row;                          \
         unsigned x_offs_el = x_offs_start_el;                                 \
                                                                               \
         element_t *linear_row = linear;                                       \
                                                                               \
         for (unsigned x_el = sx_el; x_el < sx_end_el; ++x_el) {               \
            unsigned tile_idx = (y_tile + (x_el >> log2_tile_width_el));       \
            unsigned tile_offset_el = tile_idx * tile_area_el;                 \
                                                                               \
            element_t *ptiled =                                                \
               &tiled[tile_offset_el + y_offs_el + x_offs_el];                 \
            element_t *plinear = (linear_row++);                               \
            element_t *outp = (element_t *)(is_store ? ptiled : plinear);      \
            element_t *inp = (element_t *)(is_store ? plinear : ptiled);       \
            *outp = *inp;                                                      \
            x_offs_el = (x_offs_el - space_mask_x) & space_mask_x;             \
         }                                                                     \
                                                                               \
         y_offs_el = (y_offs_el - space_mask_y) & space_mask_y;                \
         linear += linear_pitch_el;                                            \
      }                                                                        \
   }

#define TILED_UNALIGNED_TYPES(blocksize_B, store)                              \
   {                                                                           \
      if (blocksize_B == 1)                                                    \
         TILED_UNALIGNED_TYPE(uint8_t, store)                                  \
      else if (blocksize_B == 2)                                               \
         TILED_UNALIGNED_TYPE(uint16_t, store)                                 \
      else if (blocksize_B == 4)                                               \
         TILED_UNALIGNED_TYPE(uint32_t, store)                                 \
      else if (blocksize_B == 8)                                               \
         TILED_UNALIGNED_TYPE(uint64_t, store)                                 \
      else if (blocksize_B == 16)                                              \
         TILED_UNALIGNED_TYPE(ail_uint128_t, store)                            \
      else                                                                     \
         unreachable("Invalid block size");                                    \
   }

void
ail_detile(void *_tiled, void *_linear, struct ail_layout *tiled_layout,
           unsigned level, unsigned linear_pitch_B, unsigned sx_px,
           unsigned sy_px, unsigned swidth_px, unsigned sheight_px)
{
   unsigned width_px = u_minify(tiled_layout->width_px, level);
   unsigned height_px = u_minify(tiled_layout->height_px, level);
   unsigned blocksize_B = util_format_get_blocksize(tiled_layout->format);

   assert(level < tiled_layout->levels && "Mip level out of bounds");
   assert(ail_is_level_twiddled_uncompressed(tiled_layout, level) &&
          "Invalid usage");
   assert((sx_px + swidth_px) <= width_px && "Invalid usage");
   assert((sy_px + sheight_px) <= height_px && "Invalid usage");

   TILED_UNALIGNED_TYPES(blocksize_B, false);
}

void
ail_tile(void *_tiled, void *_linear, struct ail_layout *tiled_layout,
         unsigned level, unsigned linear_pitch_B, unsigned sx_px,
         unsigned sy_px, unsigned swidth_px, unsigned sheight_px)
{
   unsigned width_px = u_minify(tiled_layout->width_px, level);
   unsigned height_px = u_minify(tiled_layout->height_px, level);
   unsigned blocksize_B = util_format_get_blocksize(tiled_layout->format);

   assert(level < tiled_layout->levels && "Mip level out of bounds");
   assert(ail_is_level_twiddled_uncompressed(tiled_layout, level) &&
          "Invalid usage");
   assert((sx_px + swidth_px) <= width_px && "Invalid usage");
   assert((sy_px + sheight_px) <= height_px && "Invalid usage");

   TILED_UNALIGNED_TYPES(blocksize_B, true);
}
