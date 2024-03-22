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

bool
fdl_test_layout(const struct testcase *testcase, int gpu_id)
{
   struct fdl_layout layout = {
      .ubwc = testcase->layout.ubwc,
      .tile_mode = testcase->layout.tile_mode,
      .tile_all = testcase->layout.tile_all,
   };
   bool ok = true;

   int max_size = MAX2(testcase->layout.width0, testcase->layout.height0);
   int mip_levels = 1;
   while (max_size > 1 && testcase->layout.slices[mip_levels].pitch) {
      mip_levels++;
      max_size = u_minify(max_size, 1);
   }

   if (gpu_id >= 600) {
      fdl6_layout(&layout, testcase->format,
                  MAX2(testcase->layout.nr_samples, 1), testcase->layout.width0,
                  MAX2(testcase->layout.height0, 1),
                  MAX2(testcase->layout.depth0, 1), mip_levels,
                  MAX2(testcase->array_size, 1), testcase->is_3d, NULL);
   } else {
      assert(gpu_id >= 500);
      fdl5_layout(&layout, testcase->format,
                  MAX2(testcase->layout.nr_samples, 1), testcase->layout.width0,
                  MAX2(testcase->layout.height0, 1),
                  MAX2(testcase->layout.depth0, 1), mip_levels,
                  MAX2(testcase->array_size, 1), testcase->is_3d);
   }

   /* fdl lays out UBWC data before the color data, while all we have
    * recorded in this testcase are the color offsets (other than the UBWC
    * buffer sharing test).  Shift the fdl layout down so we can compare
    * color offsets.
    */
   if (layout.ubwc && !testcase->layout.slices[0].offset) {
      for (int l = 1; l < mip_levels; l++)
         layout.slices[l].offset -= layout.slices[0].offset;
      layout.slices[0].offset = 0;
   }

   for (int l = 0; l < mip_levels; l++) {
      if (layout.slices[l].offset != testcase->layout.slices[l].offset) {
         fprintf(stderr, "%s %dx%dx%d@%dx lvl%d: offset 0x%x != 0x%x\n",
                 util_format_short_name(testcase->format), layout.width0,
                 layout.height0, layout.depth0, layout.nr_samples, l,
                 layout.slices[l].offset, testcase->layout.slices[l].offset);
         ok = false;
      }
      if (fdl_pitch(&layout, l) != testcase->layout.slices[l].pitch) {
         fprintf(stderr, "%s %dx%dx%d@%dx lvl%d: pitch %d != %d\n",
                 util_format_short_name(testcase->format), layout.width0,
                 layout.height0, layout.depth0, layout.nr_samples, l,
                 fdl_pitch(&layout, l), testcase->layout.slices[l].pitch);
         ok = false;
      }

      /* Test optional requirement of the slice size.  Important for testing 3D
       * layouts.
       */
      if (testcase->layout.slices[l].size0 && layout.slices[l].size0 !=
          testcase->layout.slices[l].size0) {
         fprintf(stderr, "%s %dx%dx%d@%dx lvl%d: slice size %d != %d\n",
                 util_format_short_name(testcase->format), layout.width0,
                 layout.height0, layout.depth0, layout.nr_samples, l,
                 layout.slices[l].size0,
                 testcase->layout.slices[l].size0);
         ok = false;
      }

      if (layout.ubwc_slices[l].offset !=
          testcase->layout.ubwc_slices[l].offset) {
         fprintf(stderr, "%s %dx%dx%d@%dx lvl%d: UBWC offset 0x%x != 0x%x\n",
                 util_format_short_name(testcase->format), layout.width0,
                 layout.height0, layout.depth0, layout.nr_samples, l,
                 layout.ubwc_slices[l].offset,
                 testcase->layout.ubwc_slices[l].offset);
         ok = false;
      }
      if (fdl_ubwc_pitch(&layout, l) != testcase->layout.ubwc_slices[l].pitch) {
         fprintf(stderr, "%s %dx%dx%d@%dx lvl%d: UBWC pitch %d != %d\n",
                 util_format_short_name(testcase->format), layout.width0,
                 layout.height0, layout.depth0, layout.nr_samples, l,
                 fdl_ubwc_pitch(&layout, l),
                 testcase->layout.ubwc_slices[l].pitch);
         ok = false;
      }
   }

   if (!ok) {
      fdl_dump_layout(&layout);
      fprintf(stderr, "\n");
   }

   return ok;
}
