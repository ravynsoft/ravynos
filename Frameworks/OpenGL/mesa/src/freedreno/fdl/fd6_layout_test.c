/*
 * Copyright © 2020 Google LLC
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

#include "freedreno_layout.h"
#include "fd_layout_test.h"
#include "adreno_common.xml.h"
#include "adreno_pm4.xml.h"
#include "a6xx.xml.h"

#include <stdio.h>

static const struct testcase
   testcases[] =
      {
         /* A straightforward first testcase, linear, with an obvious format. */
         {
            .format = PIPE_FORMAT_R8G8B8A8_UNORM,
            .layout =
               {
                  .width0 = 32,
                  .height0 = 32,
                  .slices =
                     {
                        {.offset = 0, .pitch = 256},
                        {.offset = 8192, .pitch = 256},
                        {.offset = 12288, .pitch = 256},
                        {.offset = 14336, .pitch = 256},
                        {.offset = 15360, .pitch = 256},
                        {.offset = 15872, .pitch = 256},
                     },
               },
         },

         /* A tiled/ubwc layout from the blob driver, at a size where the a630
          * blob driver does something interesting for linear.
          */
         {
            .format = PIPE_FORMAT_R8G8B8A8_UNORM,
            .layout =
               {
                  .tile_mode = TILE6_3,
                  .ubwc = true,
                  .width0 = 1024,
                  .height0 = 1,
                  .slices =
                     {
                        {.offset = 0, .pitch = 4096},
                        {.offset = 65536, .pitch = 2048},
                        {.offset = 98304, .pitch = 1024},
                        {.offset = 114688, .pitch = 512},
                        {.offset = 122880, .pitch = 256},
                        {.offset = 126976, .pitch = 256},
                        {.offset = 131072, .pitch = 256},
                        {.offset = 135168, .pitch = 256},
                        {.offset = 139264, .pitch = 256},
                        {.offset = 143360, .pitch = 256},
                        {.offset = 147456, .pitch = 256},
                     },
                  .ubwc_slices =
                     {
                        {.offset = 0, .pitch = 64},
                        {.offset = 4096, .pitch = 64},
                        {.offset = 8192, .pitch = 64},
                        {.offset = 12288, .pitch = 64},
                        {.offset = 16384, .pitch = 64},
                        {.offset = 20480, .pitch = 64},
                        {.offset = 24576, .pitch = 64},
                        {.offset = 28672, .pitch = 64},
                        {.offset = 32768, .pitch = 64},
                        {.offset = 36864, .pitch = 64},
                        {.offset = 40960, .pitch = 64},
                     },
               },
         },

/* An interesting layout from the blob driver on a630, showing that
 * per-level pitch must be derived from level 0's pitch, not width0.  We
 * don't do this level 0 pitch disalignment (we pick 4096), so disabled
 * this test for now.
 */
#if 0
	{
		.format = PIPE_FORMAT_R8G8B8A8_UNORM,
		.layout = {
			.width0 = 1024, .height0 = 1,
			.slices = {
				{ .offset = 0, .pitch = 5120 },
				{ .offset = 5120, .pitch = 2560 },
				{ .offset = 7680, .pitch = 1280 },
				{ .offset = 8960, .pitch = 768 },
				{ .offset = 9728, .pitch = 512 },
				{ .offset = 10240, .pitch = 256 },
				{ .offset = 10496, .pitch = 256 },
				{ .offset = 10752, .pitch = 256 },
				{ .offset = 11008, .pitch = 256 },
				{ .offset = 11264, .pitch = 256 },
				{ .offset = 11520, .pitch = 256 },
			},
		},
	},
#endif

         /* A layout that we failed on (129 wide has a surprise level 1 pitch
          * increase), and the sizes bracketing it.
          */
         {
            .format = PIPE_FORMAT_R8G8B8A8_UNORM,
            .layout =
               {
                  .width0 = 128,
                  .height0 = 1,
                  .slices =
                     {
                        {.offset = 0, .pitch = 512},
                        {.offset = 512, .pitch = 256},
                        {.offset = 768, .pitch = 256},
                        {.offset = 1024, .pitch = 256},
                        {.offset = 1280, .pitch = 256},
                        {.offset = 1536, .pitch = 256},
                        {.offset = 1792, .pitch = 256},
                        {.offset = 2048, .pitch = 256},
                     },
               },
         },
         {
            .format = PIPE_FORMAT_R8G8B8A8_UNORM,
            .layout =
               {
                  .width0 = 129,
                  .height0 = 1,
                  .slices =
                     {
                        {.offset = 0, .pitch = 768},
                        {.offset = 768, .pitch = 512},
                        {.offset = 1280, .pitch = 256},
                        {.offset = 1536, .pitch = 256},
                        {.offset = 1792, .pitch = 256},
                        {.offset = 2048, .pitch = 256},
                        {.offset = 2304, .pitch = 256},
                        {.offset = 2560, .pitch = 256},
                     },
               },
         },
         {
            .format = PIPE_FORMAT_R8G8B8A8_UNORM,
            .layout =
               {
                  .width0 = 130,
                  .height0 = 1,
                  .slices =
                     {
                        {.offset = 0, .pitch = 768},
                        {.offset = 768, .pitch = 512},
                        {.offset = 1280, .pitch = 256},
                        {.offset = 1536, .pitch = 256},
                        {.offset = 1792, .pitch = 256},
                        {.offset = 2048, .pitch = 256},
                        {.offset = 2304, .pitch = 256},
                        {.offset = 2560, .pitch = 256},
                     },
               },
         },

         /* The 129 failure seems to be across formats, let's test some cpps */
         {
            .format = PIPE_FORMAT_R8_UNORM,
            .layout =
               {
                  .width0 = 129,
                  .height0 = 1,
                  .slices =
                     {
                        {.offset = 0, .pitch = 192},
                        {.offset = 192, .pitch = 128},
                        {.offset = 320, .pitch = 64},
                        {.offset = 384, .pitch = 64},
                        {.offset = 448, .pitch = 64},
                        {.offset = 512, .pitch = 64},
                        {.offset = 576, .pitch = 64},
                        {.offset = 640, .pitch = 64},
                     },
               },
         },
         {
            .format = PIPE_FORMAT_R16_UINT,
            .layout =
               {
                  .width0 = 129,
                  .height0 = 1,
                  .slices =
                     {
                        {.offset = 0, .pitch = 384},
                        {.offset = 384, .pitch = 256},
                        {.offset = 640, .pitch = 128},
                        {.offset = 768, .pitch = 128},
                        {.offset = 896, .pitch = 128},
                        {.offset = 1024, .pitch = 128},
                        {.offset = 1152, .pitch = 128},
                        {.offset = 1280, .pitch = 128},
                     },
               },
         },
         {
            .format = PIPE_FORMAT_R32G32B32A32_FLOAT,
            .layout =
               {
                  .width0 = 129,
                  .height0 = 1,
                  .slices =
                     {
                        {.offset = 0, .pitch = 3072},
                        {.offset = 3072, .pitch = 2048},
                        {.offset = 5120, .pitch = 1024},
                        {.offset = 6144, .pitch = 1024},
                        {.offset = 7168, .pitch = 1024},
                        {.offset = 8192, .pitch = 1024},
                        {.offset = 9216, .pitch = 1024},
                        {.offset = 10240, .pitch = 1024},
                     },
               },
         },

         /* The 129 failure replicated at every +256 pixels wide.  Pick one of
          * them, and this time increase the height as a new variable as well.
          */
         {
            .format = PIPE_FORMAT_R8G8B8A8_UNORM,
            .layout =
               {
                  .width0 = 385,
                  .height0 = 128,
                  .slices =
                     {
                        {.offset = 0, .pitch = 1792},
                        {.offset = 229376, .pitch = 1024},
                        {.offset = 294912, .pitch = 512},
                        {.offset = 311296, .pitch = 256},
                        {.offset = 315392, .pitch = 256},
                        {.offset = 317440, .pitch = 256},
                        {.offset = 318464, .pitch = 256},
                        {.offset = 318976, .pitch = 256},
                        {.offset = 319232, .pitch = 256},
                     },
               },
         },

         /* At 257-259 (and replicated every +256 pixels) we had another
            failure. */
         {
            .format = PIPE_FORMAT_R8G8B8A8_UNORM,
            .layout =
               {
                  .width0 = 257,
                  .height0 = 1,
                  .slices =
                     {
                        {.offset = 0, .pitch = 1280},
                        {.offset = 1280, .pitch = 768},
                        {.offset = 2048, .pitch = 512},
                        {.offset = 2560, .pitch = 256},
                        {.offset = 2816, .pitch = 256},
                        {.offset = 3072, .pitch = 256},
                        {.offset = 3328, .pitch = 256},
                        {.offset = 3584, .pitch = 256},
                        {.offset = 3840, .pitch = 256},
                     },
               },
         },
         {
            .format = PIPE_FORMAT_R8G8B8A8_UNORM,
            .layout =
               {
                  .width0 = 258,
                  .height0 = 1,
                  .slices =
                     {
                        {.offset = 0, .pitch = 1280},
                        {.offset = 1280, .pitch = 768},
                        {.offset = 2048, .pitch = 512},
                        {.offset = 2560, .pitch = 256},
                        {.offset = 2816, .pitch = 256},
                        {.offset = 3072, .pitch = 256},
                        {.offset = 3328, .pitch = 256},
                        {.offset = 3584, .pitch = 256},
                        {.offset = 3840, .pitch = 256},
                     },
               },
         },
         {
            .format = PIPE_FORMAT_R8G8B8A8_UNORM,
            .layout =
               {
                  .width0 = 259,
                  .height0 = 1,
                  .slices =
                     {
                        {.offset = 0, .pitch = 1280},
                        {.offset = 1280, .pitch = 768},
                        {.offset = 2048, .pitch = 512},
                        {.offset = 2560, .pitch = 256},
                        {.offset = 2816, .pitch = 256},
                        {.offset = 3072, .pitch = 256},
                        {.offset = 3328, .pitch = 256},
                        {.offset = 3584, .pitch = 256},
                        {.offset = 3840, .pitch = 256},
                     },
               },
         },
         {
            .format = PIPE_FORMAT_R8G8B8A8_UNORM,
            .layout =
               {
                  .width0 = 260,
                  .height0 = 1,
                  .slices =
                     {
                        {.offset = 0, .pitch = 1280},
                        {.offset = 1280, .pitch = 768},
                        {.offset = 2048, .pitch = 512},
                        {.offset = 2560, .pitch = 256},
                        {.offset = 2816, .pitch = 256},
                        {.offset = 3072, .pitch = 256},
                        {.offset = 3328, .pitch = 256},
                        {.offset = 3584, .pitch = 256},
                        {.offset = 3840, .pitch = 256},
                     },
               },
         },

         /* And, again for the 257-9 failure, test a replica with a larger size*/
         {
            .format = PIPE_FORMAT_R8G8B8A8_UNORM,
            .layout =
               {
                  .width0 = 513,
                  .height0 = 32,
                  .slices =
                     {
                        {.offset = 0, .pitch = 2304},
                        {.offset = 73728, .pitch = 1280},
                        {.offset = 94208, .pitch = 768},
                        {.offset = 100352, .pitch = 512},
                        {.offset = 102400, .pitch = 256},
                        {.offset = 102912, .pitch = 256},
                        {.offset = 103168, .pitch = 256},
                        {.offset = 103424, .pitch = 256},
                        {.offset = 103680, .pitch = 256},
                        {.offset = 103936, .pitch = 256},
                     },
               },
         },

         /* Oh, look.  The 513-517 failure extends up to 518 at the next texture
          * level!
          */
         {
            .format = PIPE_FORMAT_R8G8B8A8_UNORM,
            .layout =
               {
                  .width0 = 518,
                  .height0 = 1,
                  .slices =
                     {
                        {.offset = 0, .pitch = 2304},
                        {.offset = 2304, .pitch = 1280},
                        {.offset = 3584, .pitch = 768},
                        {.offset = 4352, .pitch = 512},
                        {.offset = 4864, .pitch = 256},
                        {.offset = 5120, .pitch = 256},
                        {.offset = 5376, .pitch = 256},
                        {.offset = 5632, .pitch = 256},
                        {.offset = 5888, .pitch = 256},
                        {.offset = 6144, .pitch = 256},
                     },
               },
         },

         /* Tiled mode testing of the unusual 1/2-bytes-per-pixel pitch
            alignment */
         {
            .format = PIPE_FORMAT_R8_UNORM,
            .layout =
               {
                  .tile_mode = TILE6_3,
                  .width0 = 129,
                  .height0 = 1,
                  .slices =
                     {
                        {.offset = 0, .pitch = 256},
                        {.offset = 8192, .pitch = 128},
                        {.offset = 12288, .pitch = 128},
                        {.offset = 16384, .pitch = 128},
                        {.offset = 20480, .pitch = 128},
                        {.offset = 20608, .pitch = 128},
                        {.offset = 20736, .pitch = 128},
                        {.offset = 20864, .pitch = 128},
                     },
               },
         },

         /* Single-level RGBA8888 UBWC following UBWC alignment rules laid out
          * in msm_media_info.h to verify that we don't break buffer sharing.
          */
         {
            .format = PIPE_FORMAT_R8G8B8A8_UNORM,
            .layout =
               {
                  .tile_mode = TILE6_3,
                  .ubwc = true,
                  .width0 = 16384,
                  .height0 = 129,
                  .slices =
                     {
                        {.offset = 1024 * 48, .pitch = 16384 * 4},
                     },
                  .ubwc_slices =
                     {
                        {.offset = 0, .pitch = 1024},
                     },
               },
         },

         /* UBWC: Pitch comes from POT-aligned level 0. */
         /* Pitch fixed in this commit, but offsets broken.  Will be fixed in
          * following commits.
          */
         {
            .format = PIPE_FORMAT_R8G8B8A8_UNORM,
            .layout =
               {
                  .tile_mode = TILE6_3,
                  .ubwc = true,
                  .width0 = 2049,
                  .height0 = 128,
                  .slices =
                     {
                        {.offset = 0, .pitch = 8448},
                        {.offset = 1081344, .pitch = 4352},
                        {.offset = 1359872, .pitch = 2304},
                        {.offset = 1433600, .pitch = 1280},
                        {.offset = 1454080, .pitch = 768},
                        {.offset = 1466368, .pitch = 512},
                        {.offset = 1474560, .pitch = 256},
                        {.offset = 1478656, .pitch = 256},
                        {.offset = 1482752, .pitch = 256},
                        {.offset = 1486848, .pitch = 256},
                        {.offset = 1490944, .pitch = 256},
                        {.offset = 1495040, .pitch = 256},
                     },
                  .ubwc_slices =
                     {
                        {.offset = 0, .pitch = 256},
                        {.offset = 16384, .pitch = 128},
                        {.offset = 24576, .pitch = 64},
                        {.offset = 28672, .pitch = 64},
                        {.offset = 32768, .pitch = 64},
                        {.offset = 36864, .pitch = 64},
                        {.offset = 40960, .pitch = 64},
                        {.offset = 45056, .pitch = 64},
                        {.offset = 49152, .pitch = 64},
                        {.offset = 53248, .pitch = 64},
                        {.offset = 57344, .pitch = 64},
                        {.offset = 61440, .pitch = 64},
                     },
               },
         },
         /* UBWC: Height comes from POT-aligned level 0. */
         {
            .format = PIPE_FORMAT_R8G8B8A8_UNORM,
            .layout =
               {
                  .tile_mode = TILE6_3,
                  .ubwc = true,
                  .width0 = 1024,
                  .height0 = 1025,
                  .slices =
                     {
                        {.offset = 0, .pitch = 4096},
                        {.offset = 4259840, .pitch = 2048},
                        {.offset = 5308416, .pitch = 1024},
                        {.offset = 5570560, .pitch = 512},
                        {.offset = 5636096, .pitch = 256},
                        {.offset = 5652480, .pitch = 256},
                        {.offset = 5660672, .pitch = 256},
                        {.offset = 5664768, .pitch = 256},
                        {.offset = 5668864, .pitch = 256},
                        {.offset = 5672960, .pitch = 256},
                        {.offset = 5677056, .pitch = 256},
                     },
                  .ubwc_slices =
                     {
                        {.offset = 0, .pitch = 64},
                        {.offset = 32768, .pitch = 64},
                        {.offset = 49152, .pitch = 64},
                        {.offset = 57344, .pitch = 64},
                        {.offset = 61440, .pitch = 64},
                        {.offset = 65536, .pitch = 64},
                        {.offset = 69632, .pitch = 64},
                        {.offset = 73728, .pitch = 64},
                        {.offset = 77824, .pitch = 64},
                        {.offset = 81920, .pitch = 64},
                        {.offset = 86016, .pitch = 64},
                     },
               },
         },

         /* UBWC: Get at minimum height of a level across cpps */
         {
            .format = PIPE_FORMAT_R16_UINT,
            .layout =
               {
                  .tile_mode = TILE6_3,
                  .ubwc = true,
                  .width0 = 16384,
                  .height0 = 1,
                  .slices =
                     {
                        {.offset = 0, .pitch = 32768},
                        {.offset = 524288, .pitch = 16384},
                        {.offset = 786432, .pitch = 8192},
                        {.offset = 917504, .pitch = 4096},
                        {.offset = 983040, .pitch = 2048},
                        {.offset = 1015808, .pitch = 1024},
                        {.offset = 1032192, .pitch = 512},
                        {.offset = 1040384, .pitch = 256},
                        {.offset = 1044480, .pitch = 256},
                        {.offset = 1048576, .pitch = 256},
                        {.offset = 1052672, .pitch = 256},
                        {.offset = 1056768, .pitch = 256},
                        {.offset = 1060864, .pitch = 256},
                        {.offset = 1064960, .pitch = 256},
                        {.offset = 1069056, .pitch = 256},
                     },
                  .ubwc_slices =
                     {
                        {.offset = 0, .pitch = 1024},
                        {.offset = 65536, .pitch = 512},
                        {.offset = 98304, .pitch = 256},
                        {.offset = 114688, .pitch = 128},
                        {.offset = 122880, .pitch = 64},
                        {.offset = 126976, .pitch = 64},
                        {.offset = 131072, .pitch = 64},
                        {.offset = 135168, .pitch = 64},
                        {.offset = 139264, .pitch = 64},
                        {.offset = 143360, .pitch = 64},
                        {.offset = 147456, .pitch = 64},
                        {.offset = 151552, .pitch = 64},
                        {.offset = 155648, .pitch = 64},
                        {.offset = 159744, .pitch = 64},
                        {.offset = 163840, .pitch = 64},
                     },
               },
         },
         {
            .format = PIPE_FORMAT_R8G8B8A8_UNORM,
            .layout =
               {
                  .tile_mode = TILE6_3,
                  .ubwc = true,
                  .width0 = 16384,
                  .height0 = 1,
                  .slices =
                     {
                        {.offset = 0, .pitch = 65536},
                        {.offset = 1048576, .pitch = 32768},
                        {.offset = 1572864, .pitch = 16384},
                        {.offset = 1835008, .pitch = 8192},
                        {.offset = 1966080, .pitch = 4096},
                        {.offset = 2031616, .pitch = 2048},
                        {.offset = 2064384, .pitch = 1024},
                        {.offset = 2080768, .pitch = 512},
                        {.offset = 2088960, .pitch = 256},
                        {.offset = 2093056, .pitch = 256},
                        {.offset = 2097152, .pitch = 256},
                        {.offset = 2101248, .pitch = 256},
                        {.offset = 2105344, .pitch = 256},
                        {.offset = 2109440, .pitch = 256},
                        {.offset = 2113536, .pitch = 256},
                     },
                  .ubwc_slices =
                     {
                        {.offset = 0, .pitch = 1024},
                        {.offset = 65536, .pitch = 512},
                        {.offset = 98304, .pitch = 256},
                        {.offset = 114688, .pitch = 128},
                        {.offset = 122880, .pitch = 64},
                        {.offset = 126976, .pitch = 64},
                        {.offset = 131072, .pitch = 64},
                        {.offset = 135168, .pitch = 64},
                        {.offset = 139264, .pitch = 64},
                        {.offset = 143360, .pitch = 64},
                        {.offset = 147456, .pitch = 64},
                        {.offset = 151552, .pitch = 64},
                        {.offset = 155648, .pitch = 64},
                        {.offset = 159744, .pitch = 64},
                        {.offset = 163840, .pitch = 64},
                     },
               },
         },
         {
            .format = PIPE_FORMAT_R32G32B32A32_FLOAT,
            .layout =
               {
                  .tile_mode = TILE6_3,
                  .ubwc = true,
                  .width0 = 16384,
                  .height0 = 1,
                  .slices =
                     {
                        {.offset = 0, .pitch = 262144},
                        {.offset = 4194304, .pitch = 131072},
                        {.offset = 6291456, .pitch = 65536},
                        {.offset = 7340032, .pitch = 32768},
                        {.offset = 7864320, .pitch = 16384},
                        {.offset = 8126464, .pitch = 8192},
                        {.offset = 8257536, .pitch = 4096},
                        {.offset = 8323072, .pitch = 2048},
                        {.offset = 8355840, .pitch = 1024},
                        {.offset = 8372224, .pitch = 1024},
                        {.offset = 8388608, .pitch = 1024},
                        {.offset = 8404992, .pitch = 1024},
                        {.offset = 8421376, .pitch = 1024},
                        {.offset = 8437760, .pitch = 1024},
                        {.offset = 8454144, .pitch = 1024},
                     },
                  .ubwc_slices =
                     {
                        {.offset = 0, .pitch = 4096},
                        {.offset = 262144, .pitch = 2048},
                        {.offset = 393216, .pitch = 1024},
                        {.offset = 458752, .pitch = 512},
                        {.offset = 491520, .pitch = 256},
                        {.offset = 507904, .pitch = 128},
                        {.offset = 516096, .pitch = 64},
                        {.offset = 520192, .pitch = 64},
                        {.offset = 524288, .pitch = 64},
                        {.offset = 528384, .pitch = 64},
                        {.offset = 532480, .pitch = 64},
                        {.offset = 536576, .pitch = 64},
                        {.offset = 540672, .pitch = 64},
                        {.offset = 544768, .pitch = 64},
                        {.offset = 548864, .pitch = 64},
                     },
               },
         },

         {
            .format = PIPE_FORMAT_R8G8B8A8_UNORM,
            .layout =
               {
                  .tile_mode = TILE6_3,
                  .ubwc = true,
                  .nr_samples = 4,
                  .width0 = 412,
                  .height0 = 732,
                  .slices =
                     {
                        {.offset = 0, .pitch = 7168},
                     },
                  .ubwc_slices =
                     {
                        {.offset = 0, .pitch = 128},
                     },
               },
         },

         /* Easy 32x32x32 3d case */
         {
            .format = PIPE_FORMAT_R9G9B9E5_FLOAT,
            .is_3d = true,
            .layout =
               {
                  .tile_mode = TILE6_3,
                  .ubwc = false,
                  .width0 = 32,
                  .height0 = 32,
                  .depth0 = 32,
                  .slices =
                     {
                        {.offset = 0, .pitch = 256, .size0 = 8192},
                        {.offset = 262144, .pitch = 256, .size0 = 4096},
                        {.offset = 327680, .pitch = 256, .size0 = 4096},
                        {.offset = 360448, .pitch = 256, .size0 = 4096},
                        {.offset = 376832, .pitch = 256, .size0 = 4096},
                        {.offset = 385024, .pitch = 256},
                     },
               },
         },

         /* Scale up a bit to 128x128x32 3d */
         {
            .format = PIPE_FORMAT_R9G9B9E5_FLOAT,
            .is_3d = true,
            .layout =
               {
                  .tile_mode = TILE6_3,
                  .ubwc = false,
                  .width0 = 128,
                  .height0 = 128,
                  .depth0 = 32,
                  .slices =
                     {
                        {.offset = 0, .pitch = 512, .size0 = 65536},
                        {.offset = 2097152, .pitch = 256, .size0 = 16384},
                        {.offset = 2359296, .pitch = 256, .size0 = 8192},
                        {.offset = 2424832, .pitch = 256, .size0 = 8192},
                        {.offset = 2457600, .pitch = 256, .size0 = 8192},
                        {.offset = 2473984, .pitch = 256},
                        {.offset = 2482176, .pitch = 256},
                        {.offset = 2490368, .pitch = 256},
                     },
               },
         },

         /* Changing width to 1 changes where minimum layer size happens. */
         {
            .format = PIPE_FORMAT_R9G9B9E5_FLOAT,
            .is_3d = true,
            .layout =
               {
                  .tile_mode = TILE6_LINEAR,
                  .ubwc = false,
                  .width0 = 1,
                  .height0 = 128,
                  .depth0 = 32,
                  .slices =
                     {
                        {.offset = 0, .pitch = 256, .size0 = 32768},
                        {.offset = 1048576, .pitch = 256, .size0 = 16384},
                        {.offset = 1310720, .pitch = 256, .size0 = 16384},
                        {.offset = 1441792, .pitch = 256, .size0 = 16384},
                        {.offset = 1507328, .pitch = 256, .size0 = 16384},
                        {.offset = 1540096, .pitch = 256},
                        {.offset = 1556480, .pitch = 256},
                        {.offset = 1572864, .pitch = 256},
                     },
               },
         },

         /* And increasing width makes it happen later. */
         {
            .format = PIPE_FORMAT_R9G9B9E5_FLOAT,
            .is_3d = true,
            .layout =
               {
                  .tile_mode = TILE6_3,
                  .ubwc = false,
                  .width0 = 1024,
                  .height0 = 128,
                  .depth0 = 32,
                  .slices =
                     {
                        {.offset = 0, .pitch = 4096, .size0 = 524288},
                        {.offset = 16777216, .pitch = 2048, .size0 = 131072},
                        {.offset = 18874368, .pitch = 1024, .size0 = 32768},
                        {.offset = 19136512, .pitch = 512, .size0 = 8192},
                        {.offset = 19169280, .pitch = 256, .size0 = 4096},
                        {.offset = 19177472, .pitch = 256},
                        {.offset = 19181568, .pitch = 256},
                        {.offset = 19185664, .pitch = 256},
                        {.offset = 19189760, .pitch = 256},
                        {.offset = 19193856, .pitch = 256},
                        {.offset = 19197952, .pitch = 256},
                     },
               },
         },

         /* NPOT height case that piglit was catching 3d texture failure in, we
          * use a higher depth though to get more slice pitches detected from
          * the blob.
          */
         {
            .format = PIPE_FORMAT_R9G9B9E5_FLOAT,
            .is_3d = true,
            .layout =
               {
                  .tile_mode = TILE6_3,
                  .ubwc = false,
                  .width0 = 128,
                  .height0 = 129,
                  .depth0 = 16,
                  .slices =
                     {
                        {.offset = 0, .pitch = 512, .size0 = 73728},
                        {.offset = 1179648, .pitch = 256, .size0 = 20480},
                        {.offset = 1343488, .pitch = 256, .size0 = 20480},
                        {.offset = 1425408, .pitch = 256, .size0 = 20480},
                        {.offset = 1466368, .pitch = 256},
                        {.offset = 1486848, .pitch = 256},
                        {.offset = 1507328, .pitch = 256},
                        {.offset = 1527808, .pitch = 256},
                     },
               },
         },

         /* NPOT height case that my first 3d layout ideas failed on. */
         {
            .format = PIPE_FORMAT_R9G9B9E5_FLOAT,
            .is_3d = true,
            .layout =
               {
                  .tile_mode = TILE6_3,
                  .ubwc = false,
                  .width0 = 128,
                  .height0 = 132,
                  .depth0 = 16,
                  .slices =
                     {
                        {.offset = 0, .pitch = 512, .size0 = 73728},
                        {.offset = 1179648, .pitch = 256, .size0 = 20480},
                        {.offset = 1343488, .pitch = 256, .size0 = 20480},
                        {.offset = 1425408, .pitch = 256, .size0 = 20480},
                        {.offset = 1466368, .pitch = 256},
                        {.offset = 1486848, .pitch = 256},
                        {.offset = 1507328, .pitch = 256},
                        {.offset = 1527808, .pitch = 256},
                     },
               },
         },

         /* blob used MIN_LAYERSZ = 0x3000 here.
          *
          * This is an interesting case for 3d layout, since pitch stays NPOT for a while.
          */
         {
            .format = PIPE_FORMAT_R9G9B9E5_FLOAT,
            .is_3d = true,
            .layout =
               {
                  .tile_mode = TILE6_3,
                  .ubwc = false,
                  .width0 = 768,
                  .height0 = 32,
                  .depth0 = 128,
                  .slices =
                     {
                        {.offset = 0, .pitch = 3072, .size0 = 98304},
                        {.offset = 12582912, .pitch = 1536, .size0 = 24576},
                        {.offset = 14155776, .pitch = 768, .size0 = 12288},
                        {.offset = 14548992, .pitch = 512, .size0 = 12288},
                        {.offset = 14745600, .pitch = 256, .size0 = 12288},
                        {.offset = 14843904, .pitch = 256, .size0 = 12288},
                        {.offset = 14893056, .pitch = 256, .size0 = 12288},
                        {.offset = 14917632, .pitch = 256},
                        {.offset = 14929920, .pitch = 256},
                        {.offset = 14942208, .pitch = 256},
                     },
               },
         },

         /* dEQP-GLES31.functional.copy_image.mixed.viewclass_128_bits_mixed.rgba32f_rg11_eac.texture3d_to_texture2d */
#if 0 /* XXX: We disagree with the blob about level 0 size0, but the testcase passes. */
         {
            .format = PIPE_FORMAT_R32G32B32A32_FLOAT,
            .is_3d = true,
            .layout =
               {
                  .tile_mode = TILE6_3,
                  .ubwc = false,
                  .width0 = 129,
                  .height0 = 129,
                  .depth0 = 17,
                  .slices =
                     {
                        {.offset = 0, .pitch = 3072, .size0 = 524288},
                        {.offset = 8912896, .pitch = 2048, .size0 = 131072},
                        {.offset = 9961472, .pitch = 1024, .size0 = 32768},
                        {.offset = 10092544, .pitch = 1024, .size0 = 16384},
                        {.offset = 10125312, .pitch = 1024},
                        {.offset = 10141696, .pitch = 1024},
                        {.offset = 10158080, .pitch = 1024},
                        {.offset = 10174464, .pitch = 1024},
                     },
               },
         },
#endif

         /* Size minification issue found while looking at the above test. */
         {
            .format = PIPE_FORMAT_R32G32B32A32_FLOAT,
            .is_3d = true,
            .layout =
               {
                  .tile_mode = TILE6_3,
                  .ubwc = false,
                  .width0 = 129,
                  .height0 = 9,
                  .depth0 = 8,
                  .slices =
                     {
                        {.offset = 0, .pitch = 3072, .size0 = 49152},
                        {.offset = 393216, .pitch = 2048, .size0 = 32768},
                        {.offset = 524288, .pitch = 1024, .size0 = 32768},
                        {.offset = 589824, .pitch = 1024},
                        {.offset = 622592, .pitch = 1024},
                        {.offset = 655360, .pitch = 1024},
                        {.offset = 688128, .pitch = 1024},
                        {.offset = 720896, .pitch = 1024},
                     },
               },
         },

};

int
main(int argc, char **argv)
{
   int ret = 0;

   for (int i = 0; i < ARRAY_SIZE(testcases); i++) {
      if (!fdl_test_layout(&testcases[i], 630))
         ret = 1;
   }

   return ret;
}
