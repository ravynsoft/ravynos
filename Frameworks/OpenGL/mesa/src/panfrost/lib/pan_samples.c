/*
 * Copyright (C) 2019 Collabora, Ltd.
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "pan_bo.h"
#include "pan_device.h"

/* Sample positions are specified partially in hardware, partially in software
 * on Mali. On Midgard, sample positions are completely fixed but need to be
 * software accessible to implement gl_SamplePosition. On Bifrost, sample
 * positions are part fixed, part programmable, and again need to be accessible
 * for gl_SamplePosition. The mali_sample_position data structure is
 * hardware-defined on Bifrost as a packed 8:8 fixed point format. The
 * mali_sample_positions array is part-hardware, part-software-defined: it must
 * give the sample position when indexed by the sample index, as well as
 * reading back the origin when indexed by 32.
 *
 * The upshot is all of this is known at compile-time, so we can just hardcode
 * the LUT, upload it to GPU memory on device initialization, and forget about
 * it.
 */

struct mali_sample_position {
   uint16_t x, y;
} __attribute__((packed));

struct mali_sample_positions {
   struct mali_sample_position positions[32];
   struct mali_sample_position origin;
   struct mali_sample_position padding[64 - (32 + 1)];
} __attribute__((packed));

/* SAMPLE16 constructs a single sample in terms of 1/16's of the grid, centered
 * at the origin. SAMPLE4/8 swap the units for legibility. */

#define SAMPLE16(x, y)                                                         \
   {                                                                           \
      (((x) + 8) * (256 / 16)), (((y) + 8) * (256 / 16))                       \
   }

#define SAMPLE8(x, y) SAMPLE16((x)*2, (y)*2)
#define SAMPLE4(x, y) SAMPLE16((x)*4, (y)*4)

/* clang-format off */
const struct mali_sample_positions sample_position_lut[] = {
   [MALI_SAMPLE_PATTERN_SINGLE_SAMPLED] = {
      .positions = {
         SAMPLE4(0, 0)
      },
      .origin = SAMPLE4(0, 0)
   },

   [MALI_SAMPLE_PATTERN_ORDERED_4X_GRID] = {
      .positions = {
         SAMPLE4(-1, -1),
         SAMPLE4( 1, -1),
         SAMPLE4(-1,  1),
         SAMPLE4( 1,  1),
      },
      .origin = SAMPLE4(0, 0)
   },

   [MALI_SAMPLE_PATTERN_ROTATED_4X_GRID] = {
      .positions = {
         SAMPLE8(-1, -3),
         SAMPLE8( 3, -1),
         SAMPLE8(-3,  1),
         SAMPLE8( 1,  3),
      },
      .origin = SAMPLE8(0, 0)
   },

   [MALI_SAMPLE_PATTERN_D3D_8X_GRID] = {
      .positions = {
         SAMPLE16( 1, -3),
         SAMPLE16(-1,  3),
         SAMPLE16( 5,  1),
         SAMPLE16(-3, -5),
         SAMPLE16(-5,  5),
         SAMPLE16(-7, -1),
         SAMPLE16( 3,  7),
         SAMPLE16( 7,  -7),
      },
      .origin = SAMPLE16(0, 0)
   },

   [MALI_SAMPLE_PATTERN_D3D_16X_GRID] = {
      .positions = {
         SAMPLE16( 1,  1),
         SAMPLE16(-1, -3),
         SAMPLE16(-3,  2),
         SAMPLE16( 4, -1),
         SAMPLE16(-5, -2),
         SAMPLE16( 2,  5),
         SAMPLE16( 5,  3),
         SAMPLE16( 3, -5),
         SAMPLE16(-2,  6),
         SAMPLE16( 0,  7),
         SAMPLE16(-4, -6),
         SAMPLE16(-6,  4),
         SAMPLE16(-8,  0),
         SAMPLE16( 7, -4),
         SAMPLE16( 6,  7),
         SAMPLE16(-7, -8),
      },
      .origin = SAMPLE16(0, 0)
   }
};
/* clang-format on */

mali_ptr
panfrost_sample_positions(const struct panfrost_device *dev,
                          enum mali_sample_pattern pattern)
{
   assert(pattern < ARRAY_SIZE(sample_position_lut));
   unsigned offset = (pattern * sizeof(sample_position_lut[0]));
   return dev->sample_positions->ptr.gpu + offset;
}

void
panfrost_upload_sample_positions(struct panfrost_device *dev)
{
   STATIC_ASSERT(sizeof(sample_position_lut) < 4096);
   dev->sample_positions = panfrost_bo_create(dev, 4096, 0, "Sample positions");

   memcpy(dev->sample_positions->ptr.cpu, sample_position_lut,
          sizeof(sample_position_lut));
}
