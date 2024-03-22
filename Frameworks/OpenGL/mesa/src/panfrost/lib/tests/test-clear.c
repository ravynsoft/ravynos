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

#include "pan_texture.h"
#include "pan_util.h"

/* A test consists of a render target format, clear colour, dither state, and
 * translated form. Dither state matters when the tilebuffer format is more
 * precise than the final format. */
struct test {
   enum pipe_format format;
   bool dithered;
   union pipe_color_union colour;
   uint32_t packed[4];
};

#define RRRR(r)                                                                \
   {                                                                           \
      r, r, r, r                                                               \
   }
#define RGRG(r, g)                                                             \
   {                                                                           \
      r, g, r, g                                                               \
   }
#define F(r, g, b, a)                                                          \
   {                                                                           \
      .f = { r, g, b, a }                                                      \
   }
#define UI(r, g, b, a)                                                         \
   {                                                                           \
      .ui = { r, g, b, a }                                                     \
   }
#define D (true)
#define _ (false)

/* clang-format off */
static const struct test clear_tests[] = {
   /* Basic tests */
   { PIPE_FORMAT_R8G8B8A8_UNORM,    D, F(0.0,   0.0, 0.0, 0.0),   RRRR(0x00000000) },
   { PIPE_FORMAT_R8G8B8A8_UNORM,    _, F(0.0,   0.0, 0.0, 0.0),   RRRR(0x00000000) },
   { PIPE_FORMAT_R8G8B8A8_UNORM,    D, F(1.0,   0.0, 0.0, 1.0),   RRRR(0xFF0000FF) },
   { PIPE_FORMAT_R8G8B8A8_UNORM,    _, F(1.0,   0.0, 0.0, 1.0),   RRRR(0xFF0000FF) },
   { PIPE_FORMAT_B8G8R8A8_UNORM,    D, F(1.0,   0.0, 0.0, 1.0),   RRRR(0xFF0000FF) },
   { PIPE_FORMAT_B8G8R8A8_UNORM,    _, F(1.0,   0.0, 0.0, 1.0),   RRRR(0xFF0000FF) },
   { PIPE_FORMAT_R8G8B8A8_UNORM,    D, F(0.664, 0.0, 0.0, 0.0),   RRRR(0x000000A9) },
   { PIPE_FORMAT_R8G8B8A8_UNORM,    _, F(0.664, 0.0, 0.0, 0.0),   RRRR(0x000000A9) },
   { PIPE_FORMAT_R4G4B4A4_UNORM,    D, F(0.664, 0.0, 0.0, 0.0),   RRRR(0x0000009F) },
   { PIPE_FORMAT_R4G4B4A4_UNORM,    _, F(0.664, 0.0, 0.0, 0.0),   RRRR(0x000000A0) },

   /* Test rounding to nearest even. The values are cherrypicked to multiply
    * out to a fractional part of 0.5. The first test should round down and
    * second test should round up. */

   { PIPE_FORMAT_R4G4B4A4_UNORM,    D, F(0.41875, 0.0, 0.0, 1.0), RRRR(0xF0000064) },
   { PIPE_FORMAT_R4G4B4A4_UNORM,    D, F(0.40625, 0.0, 0.0, 1.0), RRRR(0xF0000062) },

   /* Testing rounding direction when dithering is disabled. The packed value
    * is what gets rounded. This behaves as round-to-even with the cutoff point
    * at 2^-m + 2^-n for an m:n representation. */

   { PIPE_FORMAT_R4G4B4A4_UNORM,    _, F(0.03125, 0.0, 0.0, 1.0),  RRRR(0xF0000000) },
   { PIPE_FORMAT_R4G4B4A4_UNORM,    _, F(0.03125, 0.0, 0.0, 1.0),  RRRR(0xF0000000) },
   { PIPE_FORMAT_R4G4B4A4_UNORM,    _, F(0.03333, 0.0, 0.0, 1.0),  RRRR(0xF0000000) },
   { PIPE_FORMAT_R4G4B4A4_UNORM,    _, F(0.03334, 0.0, 0.0, 1.0),  RRRR(0xF0000010) },
   { PIPE_FORMAT_R4G4B4A4_UNORM,    _, F(0.09375, 0.0, 0.0, 1.0),  RRRR(0xF0000010) },

   /* Check all the special formats with different edge cases */

   { PIPE_FORMAT_R4G4B4A4_UNORM,    D, F(0.127, 2.4, -1.0, 0.5), RRRR(0x7800F01E) },
   { PIPE_FORMAT_R5G5B5A1_UNORM,    D, F(0.127, 2.4, -1.0, 0.5), RRRR(0x400F807E) },
   { PIPE_FORMAT_R5G6B5_UNORM,      D, F(0.127, 2.4, -1.0, 0.5), RRRR(0x000FC07E) },
   { PIPE_FORMAT_R10G10B10A2_UNORM, D, F(0.127, 2.4, -1.0, 0.5), RRRR(0x800FFC82) },
   { PIPE_FORMAT_R8G8B8A8_SRGB,     D, F(0.127, 2.4, -1.0, 0.5), RRRR(0x8000FF64) },

   { PIPE_FORMAT_R4G4B4A4_UNORM,    D, F(0.718, 0.18, 1.0, 2.0), RRRR(0xF0F02BAC) },
   { PIPE_FORMAT_R5G6B5_UNORM,      D, F(0.718, 0.18, 1.0, 2.0), RRRR(0x3E02D6C8) },
   { PIPE_FORMAT_R5G5B5A1_UNORM,    D, F(0.718, 0.18, 1.0, 2.0), RRRR(0xBE02CEC8) },
   { PIPE_FORMAT_R10G10B10A2_UNORM, D, F(0.718, 0.18, 1.0, 2.0), RRRR(0xFFF2E2DF) },
   { PIPE_FORMAT_R8G8B8A8_SRGB,     D, F(0.718, 0.18, 1.0, 2.0), RRRR(0xFFFF76DC) },

   /* Check that values are padded when dithering is disabled */

   { PIPE_FORMAT_R4G4B4A4_UNORM,    _, F(0.718, 0.18, 1.0, 2.0), RRRR(0xF0F030B0) },
   { PIPE_FORMAT_R5G6B5_UNORM,      _, F(0.718, 0.18, 1.0, 2.0), RRRR(0x3E02C2C0) },
   { PIPE_FORMAT_R5G5B5A1_UNORM,    _, F(0.718, 0.18, 1.0, 2.0), RRRR(0xBE0302C0) },
   { PIPE_FORMAT_R10G10B10A2_UNORM, _, F(0.718, 0.18, 1.0, 2.0), RRRR(0xFFF2E2DF) },
   { PIPE_FORMAT_R8G8B8A8_SRGB,     _, F(0.718, 0.18, 1.0, 2.0), RRRR(0xFFFF76DC) },

   /* Check that blendable tilebuffer values are invariant under swizzling */

   { PIPE_FORMAT_B4G4R4A4_UNORM,    D, F(0.127, 2.4, -1.0, 0.5), RRRR(0x7800F01E) },
   { PIPE_FORMAT_B5G5R5A1_UNORM,    D, F(0.127, 2.4, -1.0, 0.5), RRRR(0x400F807E) },
   { PIPE_FORMAT_B5G6R5_UNORM,      D, F(0.127, 2.4, -1.0, 0.5), RRRR(0x000FC07E) },
   { PIPE_FORMAT_B10G10R10A2_UNORM, D, F(0.127, 2.4, -1.0, 0.5), RRRR(0x800FFC82) },
   { PIPE_FORMAT_B8G8R8A8_SRGB,     D, F(0.127, 2.4, -1.0, 0.5), RRRR(0x8000FF64) },

   { PIPE_FORMAT_B4G4R4A4_UNORM,    D, F(0.718, 0.18, 1.0, 2.0), RRRR(0xF0F02BAC) },
   { PIPE_FORMAT_B5G6R5_UNORM,      D, F(0.718, 0.18, 1.0, 2.0), RRRR(0x3E02D6C8) },
   { PIPE_FORMAT_B5G5R5A1_UNORM,    D, F(0.718, 0.18, 1.0, 2.0), RRRR(0xBE02CEC8) },
   { PIPE_FORMAT_B10G10R10A2_UNORM, D, F(0.718, 0.18, 1.0, 2.0), RRRR(0xFFF2E2DF) },
   { PIPE_FORMAT_B8G8R8A8_SRGB,     D, F(0.718, 0.18, 1.0, 2.0), RRRR(0xFFFF76DC) },

   { PIPE_FORMAT_B4G4R4A4_UNORM,    _, F(0.718, 0.18, 1.0, 2.0), RRRR(0xF0F030B0) },
   { PIPE_FORMAT_B5G6R5_UNORM,      _, F(0.718, 0.18, 1.0, 2.0), RRRR(0x3E02C2C0) },
   { PIPE_FORMAT_B5G5R5A1_UNORM,    _, F(0.718, 0.18, 1.0, 2.0), RRRR(0xBE0302C0) },
   { PIPE_FORMAT_B10G10R10A2_UNORM, _, F(0.718, 0.18, 1.0, 2.0), RRRR(0xFFF2E2DF) },
   { PIPE_FORMAT_B8G8R8A8_SRGB,     _, F(0.718, 0.18, 1.0, 2.0), RRRR(0xFFFF76DC) },

   /* Check raw formats, which are not invariant under swizzling. Raw formats
    * cannot be dithered. */

   { PIPE_FORMAT_R8G8B8A8_UINT,   D, UI(0xCA, 0xFE, 0xBA, 0xBE), RRRR(0xBEBAFECA) },
   { PIPE_FORMAT_R8G8B8A8_UINT,   _, UI(0xCA, 0xFE, 0xBA, 0xBE), RRRR(0xBEBAFECA) },

   { PIPE_FORMAT_B8G8R8A8_UINT,   D, UI(0xCA, 0xFE, 0xBA, 0xBE), RRRR(0xBECAFEBA) },
   { PIPE_FORMAT_B8G8R8A8_UINT,   _, UI(0xCA, 0xFE, 0xBA, 0xBE), RRRR(0xBECAFEBA) },

   /* Check that larger raw formats are replicated correctly */

   { PIPE_FORMAT_R16G16B16A16_UINT, D, UI(0xCAFE, 0xBABE, 0xABAD, 0x1DEA),
                                       RGRG(0xBABECAFE, 0x1DEAABAD) },

   { PIPE_FORMAT_R16G16B16A16_UINT, _, UI(0xCAFE, 0xBABE, 0xABAD, 0x1DEA),
                                       RGRG(0xBABECAFE, 0x1DEAABAD) },

   { PIPE_FORMAT_R32G32B32A32_UINT, D,
      UI(0xCAFEBABE, 0xABAD1DEA, 0xDEADBEEF, 0xABCDEF01),
      { 0xCAFEBABE, 0xABAD1DEA, 0xDEADBEEF, 0xABCDEF01 } },

   { PIPE_FORMAT_R32G32B32A32_UINT, _,
      UI(0xCAFEBABE, 0xABAD1DEA, 0xDEADBEEF, 0xABCDEF01),
      { 0xCAFEBABE, 0xABAD1DEA, 0xDEADBEEF, 0xABCDEF01 } },
};
/* clang-format on */

#define ASSERT_EQ(x, y)                                                                      \
   do {                                                                                      \
      if ((x[0] == y[0]) && (x[1] == y[1]) && (x[2] == y[2]) &&                              \
          (x[3] == y[3])) {                                                                  \
         nr_pass++;                                                                          \
      } else {                                                                               \
         nr_fail++;                                                                          \
         fprintf(                                                                            \
            stderr,                                                                          \
            "%s%s: Assertion failed %s (%08X %08X %08X %08X) != %s (%08X %08X %08X %08X)\n", \
            util_format_short_name(T.format), T.dithered ? " dithered" : "",                 \
            #x, x[0], x[1], x[2], x[3], #y, y[0], y[1], y[2], y[3]);                         \
      }                                                                                      \
   } while (0)

int
main(int argc, const char **argv)
{
   unsigned nr_pass = 0, nr_fail = 0;

   for (unsigned i = 0; i < ARRAY_SIZE(clear_tests); ++i) {
      struct test T = clear_tests[i];
      uint32_t packed[4];
      pan_pack_color(panfrost_blendable_formats_v7, &packed[0], &T.colour,
                     T.format, T.dithered);

      ASSERT_EQ(T.packed, packed);
   }

   printf("Passed %u/%u\n", nr_pass, nr_pass + nr_fail);
   return nr_fail ? 1 : 0;
}
