/*
 * Copyright 2022 Alyssa Rosenzweig
 * Copyright 2022 Asahi Lina
 * SPDX-License-Identifier: MIT
 */

#include <gtest/gtest.h>
#include "layout.h"

/*
 * Test texture layouts with test cases extracted from texture structure dumps
 * produced by Metal.
 *
 * The extracted test cases are stored in separate files which do not get
 * clang-formatted. They do still get parsed as C code, though. They may be
 * sorted with `sort -t"," -k1,1 -k2,5n | uniq`.
 */
struct sizetest {
   enum pipe_format format;
   uint32_t width, height, depth;
   uint8_t levels;
   uint32_t size;
};

struct miptest {
   enum pipe_format format;
   uint32_t width, height;
   uint8_t levels;
   uint32_t offsets[16];
};

struct msaatest {
   enum pipe_format format;
   uint32_t width, height, depth;
   uint8_t levels;
   uint8_t samples;
   bool is_compressed;
   uint32_t meta_offset;
   uint32_t size;
};

static struct sizetest comptests[] = {
#include "comp-twiddled.txt"
};

static struct sizetest sizetests[] = {
#include "uncomp-twiddled.txt"
};

static struct miptest miptests[] = {
#include "miptree.txt"
};

static struct msaatest msaatests[] = {
#include "msaa.txt"
};

TEST(Generated, CompTwiddled)
{
   for (unsigned i = 0; i < ARRAY_SIZE(comptests); ++i) {
      struct sizetest test = comptests[i];

      struct ail_layout layout = {
         .width_px = test.width,
         .height_px = test.height,
         .depth_px = test.depth,
         .sample_count_sa = 1,
         .levels = test.levels,
         .tiling = AIL_TILING_TWIDDLED_COMPRESSED,
         .format = test.format,
      };

      ail_make_miptree(&layout);

      EXPECT_EQ(layout.size_B, test.size)
         << test.width << "x" << test.height << "x" << test.depth << " "
         << (int)test.levels << "L " << util_format_short_name(test.format)
         << " compressed texture has wrong allocation size, off by "
         << ((int)layout.size_B - (int)test.size);
   }
}

TEST(Generated, UncompTwiddled)
{
   for (unsigned i = 0; i < ARRAY_SIZE(sizetests); ++i) {
      struct sizetest test = sizetests[i];

      struct ail_layout layout = {
         .width_px = test.width,
         .height_px = test.height,
         .depth_px = test.depth,
         .sample_count_sa = 1,
         .levels = test.levels,
         .tiling = AIL_TILING_TWIDDLED,
         .format = test.format,
      };

      ail_make_miptree(&layout);

      EXPECT_EQ(layout.size_B, test.size)
         << test.width << "x" << test.height << "x" << test.depth << " "
         << (int)test.levels << "L " << util_format_short_name(test.format)
         << " uncompressed texture has wrong allocation size, off by "
         << ((int)layout.size_B - (int)test.size);
   }
}

TEST(Generated, Miptree2D)
{
   for (unsigned i = 0; i < ARRAY_SIZE(miptests); ++i) {
      struct miptest test = miptests[i];

      struct ail_layout layout = {
         .width_px = test.width,
         .height_px = test.height,
         .depth_px = 1,
         .sample_count_sa = 1,
         .levels = test.levels,
         .tiling = AIL_TILING_TWIDDLED,
         .format = test.format,
      };

      ail_make_miptree(&layout);

      for (unsigned l = 0; l < test.levels; ++l) {
         EXPECT_EQ(ail_get_level_offset_B(&layout, l), test.offsets[l])
            << test.width << "x" << test.height << " "
            << util_format_short_name(test.format)
            << " texture has wrong offset at level " << l << ", off by "
            << test.offsets[l] - ail_get_level_offset_B(&layout, l);
      }
   }
}

TEST(Generated, MSAA)
{
   for (unsigned i = 0; i < ARRAY_SIZE(msaatests); ++i) {
      struct msaatest test = msaatests[i];

      struct ail_layout layout = {
         .width_px = test.width,
         .height_px = test.height,
         .depth_px = test.depth,
         .sample_count_sa = test.samples,
         .levels = test.levels,
         .tiling = test.is_compressed ? AIL_TILING_TWIDDLED_COMPRESSED
                                      : AIL_TILING_TWIDDLED,
         .format = test.format,
      };

      ail_make_miptree(&layout);

      EXPECT_EQ(layout.size_B, test.size)
         << test.width << "x" << test.height << "x" << test.depth << " "
         << (int)test.levels << "L " << (int)test.samples << "S "
         << util_format_short_name(test.format)
         << (test.is_compressed ? " " : " un")
         << "compressed texture has wrong allocation size, off by "
         << ((int)layout.size_B - (int)test.size);

      if (test.is_compressed) {
         EXPECT_EQ(layout.metadata_offset_B, test.meta_offset)
            << test.width << "x" << test.height << "x" << test.depth << " "
            << (int)test.levels << "L " << (int)test.samples << "S "
            << util_format_short_name(test.format)
            << "compressed texture has wrong metadata offset size, off by "
            << ((int)layout.metadata_offset_B - (int)test.meta_offset);
      }
   }
}
