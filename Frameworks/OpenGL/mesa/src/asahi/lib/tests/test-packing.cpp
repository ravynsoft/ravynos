/*
 * Copyright 2022 Alyssa Rosenzweig
 * SPDX-License-Identifier: MIT
 */

#include "agx_pack.h"

#include <gtest/gtest.h>

const struct {
   float f;
   uint16_t encoded;
   bool inexact;
} lod_cases[] = {
   /* Lower bound clamp */
   {-INFINITY, 0x00, true},
   {-0.1, 0x00, true},
   {-0.0, 0x00, true},

   /* Exact bounds */
   {0.0, 0x00},
   {14.0, 0x380},

   /* Upper bound clamp */
   {14.1, 0x380, true},
   {18.1, 0x380, true},
   {INFINITY, 0x380, true},
};

const struct {
   uint32_t group_size;
   uint32_t length;
   uint32_t value;
   uint32_t encoded;
} group_cases[] = {
   /* Groups of 16 in a 4-bit word */
   {16, 4, 0, 0x1},   {16, 4, 1, 0x1},   {16, 4, 16, 0x1},  {16, 4, 17, 0x2},
   {16, 4, 31, 0x2},  {16, 4, 32, 0x2},  {16, 4, 33, 0x3},  {16, 4, 239, 0xF},
   {16, 4, 240, 0xF}, {16, 4, 241, 0x0}, {16, 4, 255, 0x0}, {16, 4, 256, 0x0},
};

TEST(LODClamp, Encode)
{
   for (unsigned i = 0; i < ARRAY_SIZE(lod_cases); ++i)
      ASSERT_EQ(__gen_pack_lod(lod_cases[i].f, 0, 9), lod_cases[i].encoded);
}

TEST(LODClamp, Decode)
{
   for (unsigned i = 0; i < ARRAY_SIZE(lod_cases); ++i) {
      if (lod_cases[i].inexact)
         continue;

      uint32_t cl;
      memcpy(&cl, &lod_cases[i].encoded, sizeof(lod_cases[i].encoded));

      ASSERT_EQ(__gen_unpack_lod(&cl, 0, 10), lod_cases[i].f);
   }
}

TEST(Groups, Encode)
{
   for (unsigned i = 0; i < ARRAY_SIZE(group_cases); ++i) {
      ASSERT_EQ(__gen_to_groups(group_cases[i].value, group_cases[i].group_size,
                                group_cases[i].length),
                group_cases[i].encoded);
   }
}

TEST(Groups, Decode)
{
   for (unsigned i = 0; i < ARRAY_SIZE(group_cases); ++i) {
      unsigned expected =
         ALIGN_POT(group_cases[i].value, group_cases[i].group_size);

      /* Clamp to minimum encodable */
      if (group_cases[i].value == 0)
         expected = group_cases[i].group_size;

      ASSERT_EQ(
         __gen_from_groups(group_cases[i].encoded, group_cases[i].group_size,
                           group_cases[i].length),
         expected);
   }
}
