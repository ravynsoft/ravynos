/*
 * Copyright (C) 2021 Collabora, Ltd.
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

#include "pan_blend.h"

/* A test consists of a given blend mode and its translated form */
struct test {
   const char *label;
   struct pan_blend_equation eq;
   unsigned constant_mask;
   bool reads_dest;
   bool opaque;
   bool fixed_function;
   bool alpha_zero_nop;
   bool alpha_one_store;
   uint32_t hardware;
};

/* clang-format off */
#define RGBA(key, value) \
   .rgb_ ## key = value, \
   .alpha_ ## key = value

static const struct test blend_tests[] = {
   {
      "Replace",
      {
         .blend_enable = false,
         .color_mask = 0xF,
      },
      .constant_mask = 0x0,
      .reads_dest = false,
      .opaque = true,
      .fixed_function = true,
      .alpha_zero_nop = false,
      .alpha_one_store = false,
      .hardware = 0xF0122122
   },
   {
      "Alpha",
      {
         .blend_enable = true,
         .color_mask = 0xF,

         RGBA(func, PIPE_BLEND_ADD),
         RGBA(src_factor, PIPE_BLENDFACTOR_SRC_ALPHA),
         RGBA(dst_factor, PIPE_BLENDFACTOR_INV_SRC_ALPHA),
      },
      .constant_mask = 0x0,
      .reads_dest = true,
      .opaque = false,
      .fixed_function = true,
      .alpha_zero_nop = true,
      .alpha_one_store = true,
      .hardware = 0xF0503503
   },
   {
      "Additive",
      {
         .blend_enable = true,
         .color_mask = 0xF,

         RGBA(func, PIPE_BLEND_ADD),
         RGBA(src_factor, PIPE_BLENDFACTOR_ONE),
         RGBA(dst_factor, PIPE_BLENDFACTOR_ONE),
      },
      .constant_mask = 0x0,
      .reads_dest = true,
      .opaque = false,
      .fixed_function = true,
      .alpha_zero_nop = false,
      .alpha_one_store = false,
      .hardware = 0xF0932932 /* equivalently 0xF0923923 */
   },
   {
      "Additive-Alpha",
      {
         .blend_enable = true,
         .color_mask = 0xF,

         RGBA(func, PIPE_BLEND_ADD),
         RGBA(src_factor, PIPE_BLENDFACTOR_SRC_ALPHA),
         RGBA(dst_factor, PIPE_BLENDFACTOR_ONE),
      },
      .constant_mask = 0x0,
      .reads_dest = true,
      .opaque = false,
      .fixed_function = true,
      .alpha_zero_nop = true,
      .alpha_one_store = false,
      .hardware = 0xF0523523
   },
   {
      "Subtractive",
      {
         .blend_enable = true,
         .color_mask = 0xF,

         RGBA(func, PIPE_BLEND_SUBTRACT),
         RGBA(src_factor, PIPE_BLENDFACTOR_ONE),
         RGBA(dst_factor, PIPE_BLENDFACTOR_ONE),
      },
      .constant_mask = 0x0,
      .reads_dest = true,
      .opaque = false,
      .fixed_function = true,
      .alpha_zero_nop = false,
      .alpha_one_store = false,
      .hardware = 0xF09B29B2 /* equivalently 0xF09A39A3 */
   },
   {
      "Subtractive-Alpha",
      {
         .blend_enable = true,
         .color_mask = 0xF,

         RGBA(func, PIPE_BLEND_SUBTRACT),
         RGBA(src_factor, PIPE_BLENDFACTOR_SRC_ALPHA),
         RGBA(dst_factor, PIPE_BLENDFACTOR_ONE),
      },
      .constant_mask = 0x0,
      .reads_dest = true,
      .opaque = false,
      .fixed_function = true,
      .alpha_zero_nop = false,
      .alpha_one_store = false,
      .hardware = 0xF052B52b /* equivalently 0xF05A35A3 */
   },
   {
      "Modulate",
      {
         .blend_enable = true,
         .color_mask = 0xF,

         RGBA(func, PIPE_BLEND_ADD),
         RGBA(src_factor, PIPE_BLENDFACTOR_ZERO),
         RGBA(dst_factor, PIPE_BLENDFACTOR_SRC_COLOR),
      },
      .constant_mask = 0x0,
      .reads_dest = true,
      .opaque = false,
      .fixed_function = true,
      .alpha_zero_nop = false,
      .alpha_one_store = false,
      .hardware = 0xF0231231 /* equivalently 0xF0321321 */
   },
   {
      "Replace masked",
      {
         .blend_enable = false,
         .color_mask = 0x3,
      },
      .constant_mask = 0x0,
      .reads_dest = true,
      .opaque = false,
      .fixed_function = true,
      .alpha_zero_nop = false,
      .alpha_one_store = false,
      .hardware = 0x30122122
   },
   {
      "Modulate masked",
      {
         .blend_enable = true,
         .color_mask = 0xA,

         RGBA(func, PIPE_BLEND_ADD),
         RGBA(src_factor, PIPE_BLENDFACTOR_ZERO),
         RGBA(dst_factor, PIPE_BLENDFACTOR_SRC_COLOR),
      },
      .constant_mask = 0x0,
      .reads_dest = true,
      .opaque = false,
      .fixed_function = true,
      .alpha_zero_nop = false,
      .alpha_one_store = false,
      .hardware = 0xA0231231 /* equivalently 0xA0321321 */
   },
   {
      "src*dst + dst*src",
      {
         .blend_enable = true,
         .color_mask = 0xF,

         RGBA(func, PIPE_BLEND_ADD),
         RGBA(src_factor, PIPE_BLENDFACTOR_DST_COLOR),
         RGBA(dst_factor, PIPE_BLENDFACTOR_SRC_COLOR),
      },
      .constant_mask = 0x0,
      .reads_dest = true,
      .opaque = false,
      .fixed_function = true,
      .alpha_zero_nop = false,
      .alpha_one_store = false,
      .hardware = 0xF0431431 /* 0 + dest * (2*src) */
   },
   {
      "Mixed src*dst + dst*src masked I",
      {
         .blend_enable = true,
         .color_mask = 0xC,

         .rgb_func = PIPE_BLEND_ADD,
         .rgb_src_factor = PIPE_BLENDFACTOR_ONE,
         .rgb_dst_factor= PIPE_BLENDFACTOR_ZERO,

         .alpha_func = PIPE_BLEND_ADD,
         .alpha_src_factor = PIPE_BLENDFACTOR_DST_COLOR,
         .alpha_dst_factor= PIPE_BLENDFACTOR_SRC_COLOR,
      },
      .constant_mask = 0x0,
      .reads_dest = true,
      .opaque = false,
      .fixed_function = true,
      .alpha_zero_nop = false,
      .alpha_one_store = false,
      .hardware = 0xC0431132 /* 0 + dest * (2*src); equivalent 0xC0431122 */
   },
   {
      "Mixed src*dst + dst*src masked II",
      {
         .blend_enable = true,
         .color_mask = 0xC,

         .rgb_func = PIPE_BLEND_ADD,
         .rgb_src_factor = PIPE_BLENDFACTOR_ONE,
         .rgb_dst_factor = PIPE_BLENDFACTOR_ZERO,

         .alpha_func = PIPE_BLEND_ADD,
         .alpha_src_factor = PIPE_BLENDFACTOR_DST_ALPHA,
         .alpha_dst_factor= PIPE_BLENDFACTOR_SRC_COLOR,
      },
      .constant_mask = 0x0,
      .reads_dest = true,
      .opaque = false,
      .fixed_function = true,
      .alpha_zero_nop = false,
      .alpha_one_store = false,
      .hardware = 0xC0431132 /* 0 + dest * (2*src); equivalent 0xC0431122 */
   },
   {
      "Mixed src*dst + dst*src masked III",
      {
         .blend_enable = true,
         .color_mask = 0xC,

         .rgb_func = PIPE_BLEND_ADD,
         .rgb_src_factor = PIPE_BLENDFACTOR_ONE,
         .rgb_dst_factor = PIPE_BLENDFACTOR_ZERO,

         .alpha_func = PIPE_BLEND_ADD,
         .alpha_src_factor = PIPE_BLENDFACTOR_DST_ALPHA,
         .alpha_dst_factor= PIPE_BLENDFACTOR_SRC_ALPHA,
      },
      .constant_mask = 0x0,
      .reads_dest = true,
      .opaque = false,
      .fixed_function = true,
      .alpha_zero_nop = false,
      .alpha_one_store = false,
      .hardware = 0xC0431132 /* 0 + dest * (2*src); equivalent 0xC0431122 */
   }
};
/* clang-format on */

#define ASSERT_EQ(x, y)                                                        \
   do {                                                                        \
      if (x == y) {                                                            \
         nr_pass++;                                                            \
      } else {                                                                 \
         nr_fail++;                                                            \
         fprintf(stderr, "%s: Assertion failed %s (%x) != %s (%x)\n", T.label, \
                 #x, x, #y, y);                                                \
      }                                                                        \
   } while (0)

int
main(int argc, const char **argv)
{
   unsigned nr_pass = 0, nr_fail = 0;

   for (unsigned i = 0; i < ARRAY_SIZE(blend_tests); ++i) {
      struct test T = blend_tests[i];
      ASSERT_EQ(T.constant_mask, pan_blend_constant_mask(T.eq));
      ASSERT_EQ(T.reads_dest, pan_blend_reads_dest(T.eq));
      ASSERT_EQ(T.opaque, pan_blend_is_opaque(T.eq));
      ASSERT_EQ(T.fixed_function, pan_blend_can_fixed_function(T.eq, true));
      ASSERT_EQ(T.alpha_zero_nop, pan_blend_alpha_zero_nop(T.eq));
      ASSERT_EQ(T.alpha_one_store, pan_blend_alpha_one_store(T.eq));

      if (pan_blend_can_fixed_function(T.eq, true)) {
         ASSERT_EQ(T.hardware, pan_pack_blend(T.eq));
      }
   }

   printf("Passed %u/%u\n", nr_pass, nr_pass + nr_fail);
   return nr_fail ? 1 : 0;
}
