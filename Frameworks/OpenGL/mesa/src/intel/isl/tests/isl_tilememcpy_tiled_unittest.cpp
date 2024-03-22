/*
 * Copyright 2021 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <gtest/gtest.h>
#include <inttypes.h>

#include "util/u_math.h"
#include "isl/isl.h"
#include "isl/isl_priv.h"

#define LIN_OFF(y, tw, x) ((y * tw) + x)
#define IMAGE_FORMAT ISL_FORMAT_R32G32B32_UINT

enum TILE_CONV {LIN_TO_TILE, TILE_TO_LIN};

typedef uint8_t *(*swizzle_func_t)(const uint8_t *base_addr, uint32_t x_B, uint32_t y_px);

#define TILE_COORDINATES  std::make_tuple(0, 128, 0, 32), \
                          std::make_tuple(19, 20, 25, 32), \
                          std::make_tuple(59, 83, 13, 32), \
                          std::make_tuple(10, 12, 5, 8), \
                          std::make_tuple(245, 521, 5, 8)

struct tile_swizzle_ops {
   enum isl_tiling tiling;
   swizzle_func_t linear_to_tile_swizzle;
};

uint32_t swizzle_bitops(uint32_t num, uint8_t field, uint8_t curr_ind, uint8_t swizzle_ind)
{
   uint32_t bitmask = (1 << field) - 1;
   uint32_t maskednum = num & (bitmask << curr_ind);
   uint32_t bits = maskednum >> curr_ind;
   return bits << swizzle_ind;
}

uint8_t *linear_to_Ytile_swizzle(const uint8_t *base_addr, uint32_t x_B, uint32_t y_px)
{
   /* The table below represents the mapping from coordinate (x_B, y_px) to
    * byte offset in a 128x32px 1Bpp image:
    *
    *    Bit ind : 11 10  9  8  7  6  5  4  3  2  1  0
    *     Tile-Y : u6 u5 u4 v4 v3 v2 v1 v0 u3 u2 u1 u0
    */
   uint32_t tiled_off;

   tiled_off = swizzle_bitops(x_B, 4, 0, 0) |
               swizzle_bitops(y_px, 5, 0, 4) |
               swizzle_bitops(x_B, 3, 4, 9);

   return (uint8_t *)(base_addr + tiled_off);
}

uint8_t *linear_to_tile4_swizzle(const uint8_t * base_addr, uint32_t x_B, uint32_t y_px)
{
   /* The table below represents the mapping from coordinate (x_B, y_px) to
    * byte offset in a 128x32px 1Bpp image:
    *
    *    Bit ind : 11 10  9  8  7  6  5  4  3  2  1  0
    *     Tile-Y : v4 v3 u6 v2 u5 u4 v1 v0 u3 u2 u1 u0
    */
   uint32_t tiled_off;

   tiled_off = swizzle_bitops(x_B, 4, 0, 0) |
               swizzle_bitops(y_px, 2, 0, 4) |
               swizzle_bitops(x_B, 2, 4, 6) |
               swizzle_bitops(y_px, 1, 2, 8) |
               swizzle_bitops(x_B, 1, 6, 9) |
	       swizzle_bitops(y_px, 2, 3, 10);

   return (uint8_t *) (base_addr + tiled_off);
}

struct tile_swizzle_ops swizzle_opers[] = {
   {ISL_TILING_Y0, linear_to_Ytile_swizzle},
   {ISL_TILING_4, linear_to_tile4_swizzle},
};

class tileTFixture: public ::testing::Test {

protected:
   uint8_t *buf_dst;
   uint8_t *buf_src;
   uint32_t tile_width, tile_height;
   uint32_t tile_sz;
   TILE_CONV conv;
   struct tile_swizzle_ops ops;
   bool print_results;
   struct isl_tile_info tile_info;

public:
   void test_setup(TILE_CONV convert, enum isl_tiling tiling_fmt,
              enum isl_format format,
              uint32_t max_width, uint32_t max_height);
   void TearDown();
   uint32_t swizzle_bitops(uint32_t num, uint8_t field,
                           uint8_t curr_ind, uint8_t swizzle_ind);
   void bounded_byte_fill(uint8_t x1, uint8_t x2, uint8_t y1, uint8_t y2);
   void hex_oword_print(const uint8_t *buf, uint32_t size);
   void convert_texture(uint8_t x1, uint8_t x2, uint8_t y1, uint8_t y2);
   void compare_conv_result(uint8_t x1, uint8_t x2, uint8_t y1, uint8_t y2);
   void run_test(uint8_t x1, uint8_t x2, uint8_t y1, uint8_t y2);
};

class tileYFixture : public tileTFixture,
                     public ::testing::WithParamInterface<std::tuple<int, int,
                                                                     int, int>>
{};

class tile4Fixture : public tileTFixture,
                     public ::testing::WithParamInterface<std::tuple<int, int,
                                                                     int, int>>
{};

void tileTFixture::test_setup(TILE_CONV convert,
                         enum isl_tiling tiling_fmt,
                         enum isl_format format,
                         uint32_t max_width,
                         uint32_t max_height)
{
   print_results = debug_get_bool_option("ISL_TEST_DEBUG", false);

   const struct isl_format_layout *fmtl = isl_format_get_layout(format);
   conv = convert;
   ops.tiling = tiling_fmt;

   isl_tiling_get_info(tiling_fmt, ISL_SURF_DIM_2D, ISL_MSAA_LAYOUT_NONE,
		       fmtl->bpb, 1 , &tile_info);

   tile_width = DIV_ROUND_UP(max_width, tile_info.logical_extent_el.w) *
                tile_info.phys_extent_B.w;
   tile_height = DIV_ROUND_UP(max_height, tile_info.logical_extent_el.h) *
                 tile_info.phys_extent_B.h;
   tile_sz = tile_width * tile_height;

   buf_src = (uint8_t *) calloc(tile_sz, sizeof(uint8_t));
   ASSERT_TRUE(buf_src != nullptr);

   buf_dst = (uint8_t *) calloc(tile_sz, sizeof(uint8_t));
   ASSERT_TRUE(buf_src != nullptr);

   for (uint8_t i = 0; i < ARRAY_SIZE(swizzle_opers); i++)
      if (ops.tiling == swizzle_opers[i].tiling)
         ops.linear_to_tile_swizzle = swizzle_opers[i].linear_to_tile_swizzle;

   memset(buf_src, 0xcc, tile_sz);
   memset(buf_dst, 0xcc, tile_sz);
}

void tileTFixture::TearDown()
{
   free(buf_src);
   buf_src = nullptr;

   free(buf_dst);
   buf_dst = nullptr;
}

void tileTFixture::bounded_byte_fill(uint8_t x1, uint8_t x2, uint8_t y1, uint8_t y2)
{
   uint8_t *itr = (uint8_t *) buf_src;

   for(auto y = y1; y < y2; y++)
      for (auto x = x1; x < x2; x++)
         if (conv == LIN_TO_TILE)
            *(itr + LIN_OFF(y, tile_width, x)) = LIN_OFF(y, tile_width, x)/16;
         else
            *(ops.linear_to_tile_swizzle(buf_src, x, y)) = LIN_OFF(y, tile_width, x)/16;
}

void tileTFixture::hex_oword_print(const uint8_t *buf, uint32_t size)
{
   uint64_t *itr;
   uint32_t i;

   for (itr = (uint64_t *)buf, i=0; itr < (uint64_t *)(buf + size); i++) {
      fprintf(stdout, "%.16" PRIx64 "%.16" PRIx64, util_bswap64(*(itr)), util_bswap64(*(itr+1)));

      itr = itr+2;

      if((i+1) % 8 == 0 && i > 0)
         printf("\n");
      else
         printf("  ");
   }
}

void tileTFixture::convert_texture(uint8_t x1, uint8_t x2, uint8_t y1, uint8_t y2)
{
   if (print_results) {
      printf("/************** Printing src ***************/\n");
      hex_oword_print((const uint8_t *)buf_src, tile_sz);
   }

   if (conv == LIN_TO_TILE)
      isl_memcpy_linear_to_tiled(x1, x2, y1, y2,
                                 (char *)buf_dst,
                                 (const char *)buf_src + LIN_OFF(y1, tile_width, x1),
                                 tile_width, tile_width,
                                 0, ops.tiling, ISL_MEMCPY);
   else
      isl_memcpy_tiled_to_linear(x1, x2, y1, y2,
                                 (char *)buf_dst + LIN_OFF(y1, tile_width, x1),
                                 (const char *)buf_src,
                                 tile_width, tile_width,
                                 0, ops.tiling, ISL_MEMCPY);

   if (print_results) {
      printf("/************** Printing dest **************/\n");
      hex_oword_print((const uint8_t *) buf_dst, tile_sz);
   }
}

void tileTFixture::compare_conv_result(uint8_t x1, uint8_t x2,
                                       uint8_t y1, uint8_t y2)
{
   uint32_t x_max = (uint32_t) align(x2, tile_info.logical_extent_el.w);
   uint32_t y_max = (uint32_t) align(y2, tile_info.logical_extent_el.h);

   for(uint32_t y = 0; y < y_max; y++) {
      for (uint32_t x = 0; x < x_max; x++) {

         if (x < x1 || x > x2 || y < y1 || y > y2) {
            if (conv == LIN_TO_TILE) {
               EXPECT_EQ(*(buf_src + LIN_OFF(y, tile_width, x)), 0xcc)
                  << "Not matching for x:" << x << "and y:" << y << std::endl;
            } else {
               EXPECT_EQ(*(buf_dst + LIN_OFF(y, tile_width, x)), 0xcc)
                  << "Not matching for x:" << x << "and y:" << y << std::endl;
            }
         } else {
            if (conv == LIN_TO_TILE) {
               EXPECT_EQ(*(buf_src + LIN_OFF(y, tile_width, x)),
                         *(ops.linear_to_tile_swizzle(buf_dst, x, y)))
                  << "Not matching for x:" << x << "and y:" << y << std::endl;
            } else {
               EXPECT_EQ(*(buf_dst + LIN_OFF(y, tile_width, x)),
                         *(ops.linear_to_tile_swizzle(buf_src, x, y)))
                  << "Not matching for x:" << x << "and y:" << y << std::endl;
            }
         }
      }
   }
}

void tileTFixture::run_test(uint8_t x1, uint8_t x2,
                            uint8_t y1, uint8_t y2)
{
    bounded_byte_fill(x1, x2, y1, y2);
    convert_texture(x1, x2, y1, y2);
    compare_conv_result(x1, x2, y1, y2);
}

TEST_P(tileYFixture, lintotile)
{
    auto [x1, x2, y1, y2] = GetParam();
    test_setup(LIN_TO_TILE, ISL_TILING_Y0, IMAGE_FORMAT, x2, y2);
    if (print_results)
       printf("Coordinates: x1=%d x2=%d y1=%d y2=%d \n", x1, x2, y1, y2);
    run_test(x1, x2, y1, y2);
}

TEST_P(tileYFixture, tiletolin)
{
    auto [x1, x2, y1, y2] = GetParam();
    test_setup(TILE_TO_LIN, ISL_TILING_Y0, IMAGE_FORMAT, x2, y2);
    if (print_results)
       printf("Coordinates: x1=%d x2=%d y1=%d y2=%d \n", x1, x2, y1, y2);
    run_test(x1, x2, y1, y2);
}

TEST_P(tile4Fixture, lintotile)
{
    auto [x1, x2, y1, y2] = GetParam();
    test_setup(LIN_TO_TILE, ISL_TILING_4, IMAGE_FORMAT, x2, y2);
    if (print_results)
       printf("Coordinates: x1=%d x2=%d y1=%d y2=%d \n", x1, x2, y1, y2);
    run_test(x1, x2, y1, y2);
}

TEST_P(tile4Fixture, tiletolin)
{
    auto [x1, x2, y1, y2] = GetParam();
    test_setup(TILE_TO_LIN, ISL_TILING_4, IMAGE_FORMAT, x2, y2);
    if (print_results)
       printf("Coordinates: x1=%d x2=%d y1=%d y2=%d \n", x1, x2, y1, y2);
    run_test(x1, x2, y1, y2);
}


INSTANTIATE_TEST_SUITE_P(Ytile, tileYFixture, testing::Values(TILE_COORDINATES));
INSTANTIATE_TEST_SUITE_P(tile4, tile4Fixture, testing::Values(TILE_COORDINATES));
