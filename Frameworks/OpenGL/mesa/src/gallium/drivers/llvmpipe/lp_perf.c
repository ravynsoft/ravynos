/**************************************************************************
 *
 * Copyright 2009 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#include "util/u_debug.h"
#include "lp_debug.h"
#include "lp_perf.h"



struct lp_counters lp_count;


void
lp_reset_counters(void)
{
   memset(&lp_count, 0, sizeof(lp_count));
}


void
lp_print_counters(void)
{
   if (LP_DEBUG & DEBUG_COUNTERS) {
      unsigned total_64, total_16, total_4;
      float p1, p2, p3, p4, p5, p6;

      debug_printf("llvmpipe: nr_triangles:                 %9u\n", lp_count.nr_tris);
      debug_printf("llvmpipe: nr_culled_triangles:          %9u\n", lp_count.nr_culled_tris);
      debug_printf("llvmpipe: nr_rectangles:                %9u\n", lp_count.nr_rects);
      debug_printf("llvmpipe: nr_culled_rectangles:         %9u\n", lp_count.nr_culled_rects);

      total_64 = (lp_count.nr_empty_64 + 
                  lp_count.nr_fully_covered_64 +
                  lp_count.nr_partially_covered_64);

      p1 = 100.0 * (float) lp_count.nr_empty_64 / (float) total_64;
      p2 = 100.0 * (float) lp_count.nr_fully_covered_64 / (float) total_64;
      p3 = 100.0 * (float) lp_count.nr_partially_covered_64 / (float) total_64;
      p4 = 100.0 * (float) lp_count.nr_blit_64 / (float) total_64;
      p5 = 100.0 * (float) lp_count.nr_shade_opaque_64 / (float) total_64;
      p6 = 100.0 * (float) lp_count.nr_shade_64 / (float) total_64;

      debug_printf("llvmpipe: nr_64x64:                     %9u\n", total_64);
      debug_printf("llvmpipe:   nr_fully_covered_64x64:     %9u (%3.0f%% of %u)\n", lp_count.nr_fully_covered_64, p2, total_64);
      debug_printf("llvmpipe:     nr_blit_64x64:            %9u (%3.0f%% of %u)\n", lp_count.nr_blit_64, p4, total_64);
      debug_printf("llvmpipe:        nr_pure_blit_64x64:    %9u (%3.0f%% of %u)\n", lp_count.nr_pure_blit_64, 0.0, lp_count.nr_blit_64);
      debug_printf("llvmpipe:     nr_shade_opaque_64x64:    %9u (%3.0f%% of %u)\n", lp_count.nr_shade_opaque_64, p5, total_64);
      debug_printf("llvmpipe:        nr_pure_shade_opaque:  %9u (%3.0f%% of %u)\n", lp_count.nr_pure_shade_opaque_64, 0.0, lp_count.nr_shade_opaque_64);
      debug_printf("llvmpipe:     nr_shade_64x64:           %9u (%3.0f%% of %u)\n", lp_count.nr_shade_64, p6, total_64);
      debug_printf("llvmpipe:        nr_pure_shade:         %9u (%3.0f%% of %u)\n", lp_count.nr_pure_shade_64, 0.0, lp_count.nr_shade_64);
      debug_printf("llvmpipe:   nr_partially_covered_64x64: %9u (%3.0f%% of %u)\n", lp_count.nr_partially_covered_64, p3, total_64);
      debug_printf("llvmpipe:   nr_empty_64x64:             %9u (%3.0f%% of %u)\n", lp_count.nr_empty_64, p1, total_64);

      total_16 = (lp_count.nr_empty_16 + 
                  lp_count.nr_fully_covered_16 +
                  lp_count.nr_partially_covered_16);

      p1 = 100.0 * (float) lp_count.nr_empty_16 / (float) total_16;
      p2 = 100.0 * (float) lp_count.nr_fully_covered_16 / (float) total_16;
      p3 = 100.0 * (float) lp_count.nr_partially_covered_16 / (float) total_16;

      debug_printf("llvmpipe: nr_16x16:                     %9u\n", total_16);
      debug_printf("llvmpipe:   nr_fully_covered_16x16:     %9u (%3.0f%% of %u)\n", lp_count.nr_fully_covered_16, p2, total_16);
      debug_printf("llvmpipe:   nr_partially_covered_16x16: %9u (%3.0f%% of %u)\n", lp_count.nr_partially_covered_16, p3, total_16);
      debug_printf("llvmpipe:   nr_empty_16x16:             %9u (%3.0f%% of %u)\n", lp_count.nr_empty_16, p1, total_16);

      total_4 = (lp_count.nr_empty_4 +
                 lp_count.nr_fully_covered_4 +
                 lp_count.nr_partially_covered_4);

      p1 = 100.0 * (float) lp_count.nr_empty_4 / (float) total_4;
      p2 = 100.0 * (float) lp_count.nr_fully_covered_4 / (float) total_4;
      p3 = 100.0 * (float) lp_count.nr_partially_covered_4 / (float) total_4;
      p4 = 100.0 * (float) lp_count.nr_non_empty_4 / (float) total_4;

      debug_printf("llvmpipe: nr_tri_4x4:                   %9u\n", total_4);
      debug_printf("llvmpipe:   nr_fully_covered_4x4:       %9u (%3.0f%% of %u)\n", lp_count.nr_fully_covered_4, p2, total_4);
      debug_printf("llvmpipe:   nr_partially_covered_4x4:   %9u (%3.0f%% of %u)\n", lp_count.nr_partially_covered_4, p3, total_4);
      debug_printf("llvmpipe:   nr_empty_4x4:               %9u (%3.0f%% of %u)\n", lp_count.nr_empty_4, p1, total_4);
      debug_printf("llvmpipe:   nr_non_empty_4x4:           %9u (%3.0f%% of %u)\n", lp_count.nr_non_empty_4, p4, total_4);

      total_4 = (lp_count.nr_rect_partially_covered_4 +
                 lp_count.nr_rect_fully_covered_4);

      p1 = 100.0 * (float) lp_count.nr_rect_partially_covered_4 / (float) total_4;
      p2 = 100.0 * (float) lp_count.nr_rect_fully_covered_4 / (float) total_4;

      debug_printf("llvmpipe: nr_rect_4x4:                  %9u\n", total_4);
      debug_printf("llvmpipe:   nr_rect_full_4x4:           %9u (%3.0f%% of %u)\n", lp_count.nr_rect_fully_covered_4, p1, total_4);
      debug_printf("llvmpipe:   nr_rect_part_4x4:           %9u (%3.0f%% of %u)\n", lp_count.nr_rect_partially_covered_4, p2, total_4);


      debug_printf("llvmpipe: nr_color_tile_clear:          %9u\n", lp_count.nr_color_tile_clear);
      debug_printf("llvmpipe: nr_color_tile_load:           %9u\n", lp_count.nr_color_tile_load);
      debug_printf("llvmpipe: nr_color_tile_store:          %9u\n", lp_count.nr_color_tile_store);

      debug_printf("llvmpipe: nr_llvm_compiles:             %u\n", lp_count.nr_llvm_compiles);
      debug_printf("llvmpipe: total LLVM compile time:      %.2f sec\n", lp_count.llvm_compile_time / 1000000.0);
      debug_printf("llvmpipe: average LLVM compile time:    %.2f sec\n", lp_count.llvm_compile_time / 1000000.0 / lp_count.nr_llvm_compiles);

   }
}
