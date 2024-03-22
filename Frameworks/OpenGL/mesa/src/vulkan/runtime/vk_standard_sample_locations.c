/*
 * Copyright © 2020 Intel Corporation
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "vk_standard_sample_locations.h"

#include "vk_graphics_state.h"

/**
 * 1x MSAA has a single sample at the center: (0.5, 0.5) -> (0x8, 0x8).
 */
static const struct vk_sample_locations_state sample_locations_state_1x = {
   .per_pixel = VK_SAMPLE_COUNT_1_BIT,
   .grid_size = { 1, 1 },
   .locations = {
      { 0.5, 0.5 },
   },
};


/**
 * 2x MSAA sample positions are (0.25, 0.25) and (0.75, 0.75):
 *   4 c
 * 4 0
 * c   1
 */
static const struct vk_sample_locations_state sample_locations_state_2x = {
   .per_pixel = VK_SAMPLE_COUNT_2_BIT,
   .grid_size = { 1, 1 },
   .locations = {
      { 0.75, 0.75 },
      { 0.25, 0.25 },
   },
};

/**
 * Sample positions:
 *   2 6 a e
 * 2   0
 * 6       1
 * a 2
 * e     3
 */
static const struct vk_sample_locations_state sample_locations_state_4x = {
   .per_pixel = VK_SAMPLE_COUNT_4_BIT,
   .grid_size = { 1, 1 },
   .locations = {
      { 0.375, 0.125 },
      { 0.875, 0.375 },
      { 0.125, 0.625 },
      { 0.625, 0.875 },
   },
};

/**
 * Sample positions:
 *   1 3 5 7 9 b d f
 * 1               7
 * 3     3
 * 5         0
 * 7 5
 * 9             2
 * b       1
 * d   4
 * f           6
 */
static const struct vk_sample_locations_state sample_locations_state_8x = {
   .per_pixel = VK_SAMPLE_COUNT_8_BIT,
   .grid_size = { 1, 1 },
   .locations = {
      { 0.5625, 0.3125 },
      { 0.4375, 0.6875 },
      { 0.8125, 0.5625 },
      { 0.3125, 0.1875 },
      { 0.1875, 0.8125 },
      { 0.0625, 0.4375 },
      { 0.6875, 0.9375 },
      { 0.9375, 0.0625 },
   },
};

/**
 * Sample positions:
 *
 *    0 1 2 3 4 5 6 7 8 9 a b c d e f
 * 0   15
 * 1                  9
 * 2         10
 * 3                        7
 * 4                               13
 * 5                1
 * 6        4
 * 7                          3
 * 8 12
 * 9                    0
 * a            2
 * b                            6
 * c     11
 * d                      5
 * e              8
 * f                             14
 */
static const struct vk_sample_locations_state sample_locations_state_16x = {
   .per_pixel = VK_SAMPLE_COUNT_16_BIT,
   .grid_size = { 1, 1 },
   .locations = {
      { 0.5625, 0.5625 },
      { 0.4375, 0.3125 },
      { 0.3125, 0.6250 },
      { 0.7500, 0.4375 },
      { 0.1875, 0.3750 },
      { 0.6250, 0.8125 },
      { 0.8125, 0.6875 },
      { 0.6875, 0.1875 },
      { 0.3750, 0.8750 },
      { 0.5000, 0.0625 },
      { 0.2500, 0.1250 },
      { 0.1250, 0.7500 },
      { 0.0000, 0.5000 },
      { 0.9375, 0.2500 },
      { 0.8750, 0.9375 },
      { 0.0625, 0.0000 },
   },
};

const struct vk_sample_locations_state *
vk_standard_sample_locations_state(VkSampleCountFlagBits sample_count)
{
   switch (sample_count) {
   case 1:  return &sample_locations_state_1x;
   case 2:  return &sample_locations_state_2x;
   case 4:  return &sample_locations_state_4x;
   case 8:  return &sample_locations_state_8x;
   case 16: return &sample_locations_state_16x;
   default: unreachable("Sample count has no standard locations");
   }
}
