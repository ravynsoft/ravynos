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

struct testcase {
   enum pipe_format format;

   int array_size; /* Size for array textures, or 0 otherwise. */
   bool is_3d;

   /* Partially filled layout of input parameters and expected results. */
   struct {
      uint32_t tile_mode : 2;
      bool tile_all : 1;
      bool ubwc : 1;
      uint32_t width0, height0, depth0;
      uint32_t nr_samples;
      struct {
         uint32_t offset;
         uint32_t pitch;
         uint32_t size0;
      } slices[FDL_MAX_MIP_LEVELS];
      struct {
         uint32_t offset;
         uint32_t pitch;
      } ubwc_slices[FDL_MAX_MIP_LEVELS];
   } layout;
};

bool fdl_test_layout(const struct testcase *testcase, int gpu_id);
