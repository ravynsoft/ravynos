/*
 * Copyright (C) 2020 Collabora, Ltd.
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

#include "bi_test.h"
#include "compiler.h"

#include <gtest/gtest.h>
#include "mesa-gtest-extras.h"

class PackFormats : public testing::Test {
 protected:
   PackFormats()
   {
      util_dynarray_init(&result, NULL);
   }
   ~PackFormats()
   {
      util_dynarray_fini(&result);
   }

   const uint64_t *result_as_u64_array()
   {
      return reinterpret_cast<uint64_t *>(result.data);
   }

   struct util_dynarray result;
};

TEST_F(PackFormats, 1)
{
   /* Test case from the blob */
   struct bi_packed_tuple tuples[] = {
      {0x2380cb1c02200000, 0x10e0},
   };

   uint64_t header = 0x021000011800;

   bi_pack_format(&result, 1, tuples, 1, header, 0, 0, true);

   const uint64_t expected[] = {
      0x80cb1c022000004a,
      0x10800008c000e023,
   };

   ASSERT_EQ(result.size, 16);
   ASSERT_U64_ARRAY_EQUAL(expected, result_as_u64_array(), 2);
}

TEST_F(PackFormats, 2)
{
   struct bi_packed_tuple tuples[] = {
      {0x9380cb6044000044, 0xf65},
      {0xaf8721a05c000081, 0x1831},
   };

   bi_pack_format(&result, 0, tuples, 2, 0x52800011800, 0, 0, false);
   bi_pack_format(&result, 2, tuples, 2, 0x52800011800, 0, 0, false);

   const uint64_t expected[] = {
      0x80cb604400004429,
      0x29400008c0076593,
      0x8721a05c00008103,
      0x60000000000031af,
   };

   ASSERT_EQ(result.size, 32);
   ASSERT_U64_ARRAY_EQUAL(expected, result_as_u64_array(), 4);
}

TEST_F(PackFormats, 3)
{
   struct bi_packed_tuple tuples[] = {
      {0x93805b8040000000, 0xf65},
      {0x93886db05c000000, 0xf65},
      {0xb380cb180c000080, 0x18b1},
   };

   bi_pack_format(&result, 0, tuples, 3, 0x3100000000, 0, 0, true);
   bi_pack_format(&result, 3, tuples, 3, 0x3100000000, 0, 0, true);
   bi_pack_format(&result, 4, tuples, 3, 0x3100000000, 0, 0, true);

   const uint64_t expected[] = {
      0x805b804000000029, 0x0188000000076593, 0x886db05c00000021,
      0x58c0600004076593, 0x0000000000000044, 0x60002c6ce0300000,
   };

   ASSERT_EQ(result.size, 48);
   ASSERT_U64_ARRAY_EQUAL(expected, result_as_u64_array(), 6);
}

TEST_F(PackFormats, 4)
{
   struct bi_packed_tuple tuples[] = {
      {0xad8c87004000005f, 0x2f18},
      {0xad8c87385c00004f, 0x2f18},
      {0xad8c87385c00006e, 0x2f18},
      {0xb380cb182c000080, 0x18b1},
   };

   uint64_t EC0 = (0x10000001ff000000) >> 4;

   bi_pack_format(&result, 0, tuples, 4, 0x3100000000, EC0, 0, false);
   bi_pack_format(&result, 3, tuples, 4, 0x3100000000, EC0, 0, false);
   bi_pack_format(&result, 6, tuples, 4, 0x3100000000, EC0, 0, false);

   const uint64_t expected[] = {
      0x8c87004000005f2d, 0x01880000000718ad, 0x8c87385c00004f25,
      0x39c2e000037718ad, 0x80cb182c00008005, 0xac01c62b6320b1b3,
   };

   ASSERT_EQ(result.size, 48);
   ASSERT_U64_ARRAY_EQUAL(expected, result_as_u64_array(), 6);
}

TEST_F(PackFormats, 5)
{
   struct bi_packed_tuple tuples[] = {
      {0x9380688040000000, 0xf65},  {0xd4057300c000040, 0xf26},
      {0x1f80cb1858000000, 0x19ab}, {0x937401f85c000000, 0xf65},
      {0xb380cb180c000080, 0x18a1},
   };

   uint64_t EC0 = (0x183f800000) >> 4;

   bi_pack_format(&result, 0, tuples, 5, 0x3100000000, EC0, 0, true);
   bi_pack_format(&result, 3, tuples, 5, 0x3100000000, EC0, 0, true);
   bi_pack_format(&result, 7, tuples, 5, 0x3100000000, EC0, 0, true);
   bi_pack_format(&result, 8, tuples, 5, 0x3100000000, EC0, 0, true);

   const uint64_t expected[] = {
      0x8068804000000029, 0x0188000000076593, 0x4057300c00004021,
      0x58c2c0000007260d, 0x7401f85c0000008b, 0x00006ac7e0376593,
      0x80cb180c00008053, 0x000000183f80a1b3,
   };

   ASSERT_EQ(result.size, 64);
   ASSERT_U64_ARRAY_EQUAL(expected, result_as_u64_array(), 8);
}

TEST_F(PackFormats, 6)
{
   struct bi_packed_tuple tuples[] = {
      {0xad8c870068000048, 0x2f18}, {0xad8c87385c000050, 0x2f18},
      {0xad8c87385c00006a, 0x2f18}, {0xad8c87385c000074, 0x2f18},
      {0xad8c87385c000020, 0x2f18}, {0xad8c87385c000030, 0x2f18},
   };

   uint64_t EC0 = (0x345678912345670) >> 4;

   bi_pack_format(&result, 0, tuples, 6, 0x60000011800, EC0, 0, false);
   bi_pack_format(&result, 3, tuples, 6, 0x60000011800, EC0, 0, false);
   bi_pack_format(&result, 5, tuples, 6, 0x60000011800, EC0, 0, false);
   bi_pack_format(&result, 9, tuples, 6, 0x60000011800, EC0, 0, false);
   bi_pack_format(&result, 10, tuples, 6, 0x60000011800, EC0, 0, false);

   const uint64_t expected[] = {
      0x8c8700680000482d, 0x30000008c00718ad, 0x8c87385c00005025,
      0x39c2e000035718ad, 0x8c87385c00007401, 0xb401c62b632718ad,
      0x8c87385c00002065, 0x39c2e000018718ad, 0x3456789123456706,
      0xa001c62b63200000,
   };

   ASSERT_EQ(result.size, 80);
   ASSERT_U64_ARRAY_EQUAL(expected, result_as_u64_array(), 10);
}

TEST_F(PackFormats, 7)
{
   struct bi_packed_tuple tuples[] = {
      {0x9020074040000083, 0xf65},  {0x90000d4058100080, 0xf65},
      {0x90000a3058700082, 0xf65},  {0x9020074008114581, 0xf65},
      {0x90000d0058000080, 0xf65},  {0x9000083058700082, 0xf65},
      {0x2380cb199ac38400, 0x327a},
   };

   bi_pack_format(&result, 0, tuples, 7, 0x3000100000, 0, 0, true);
   bi_pack_format(&result, 3, tuples, 7, 0x3000100000, 0, 0, true);
   bi_pack_format(&result, 5, tuples, 7, 0x3000100000, 0, 0, true);
   bi_pack_format(&result, 9, tuples, 7, 0x3000100000, 0, 0, true);
   bi_pack_format(&result, 11, tuples, 7, 0x3000100000, 0, 0, true);

   const uint64_t expected[] = {
      0x2007404000008329, 0x0180008000076590, 0x000d405810008021,
      0x5182c38004176590, 0x2007400811458101, 0x2401d96400076590,
      0x000d005800008061, 0x4182c38004176590, 0x80cb199ac3840047,
      0x3801d96400027a23,
   };

   ASSERT_EQ(result.size, 80);
   ASSERT_U64_ARRAY_EQUAL(expected, result_as_u64_array(), 10);
}

TEST_F(PackFormats, 8)
{
   struct bi_packed_tuple tuples[] = {
      {0x442087037a2f8643, 0x3021}, {0x84008d0586100043, 0x200},
      {0x7c008d0028014543, 0x0},    {0x1c00070058200081, 0x1980},
      {0x1600dd878320400, 0x200},   {0x49709c1b08308900, 0x200},
      {0x6c2007807881ca00, 0x40},   {0x8d70fc0d94900083, 0x800},
   };

   uint64_t EC0 = (0x32e635d0) >> 4;

   bi_pack_format(&result, 0, tuples, 8, 0x61001311800, EC0, 0, true);
   bi_pack_format(&result, 3, tuples, 8, 0x61001311800, EC0, 0, true);
   bi_pack_format(&result, 5, tuples, 8, 0x61001311800, EC0, 0, true);
   bi_pack_format(&result, 9, tuples, 8, 0x61001311800, EC0, 0, true);
   bi_pack_format(&result, 12, tuples, 8, 0x61001311800, EC0, 0, true);
   bi_pack_format(&result, 13, tuples, 8, 0x61001311800, EC0, 0, true);

   const uint64_t expected[] = {
      0x2087037a2f86432e, 0x30800988c0002144, 0x008d058610004320,
      0x6801400a2a1a0084, 0x0007005820008101, 0x0c00001f0021801c,
      0x600dd87832040060, 0xe0d8418448020001, 0x2007807881ca00c0,
      0xc6ba80125c20406c, 0x70fc0d9490008359, 0x0000000032e0008d,
   };

   ASSERT_EQ(result.size, 96);
   ASSERT_U64_ARRAY_EQUAL(expected, result_as_u64_array(), 12);
}
