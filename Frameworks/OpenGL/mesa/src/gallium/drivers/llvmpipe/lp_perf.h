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

/**
 * Performance / statistic counters, etc.
 */


#ifndef LP_PERF_H
#define LP_PERF_H

#include "util/compiler.h"

/**
 * Various counters
 */
struct lp_counters
{
   unsigned nr_tris;
   unsigned nr_culled_tris;
   unsigned nr_rects;
   unsigned nr_culled_rects;
   unsigned nr_empty_64;
   unsigned nr_fully_covered_64;
   unsigned nr_partially_covered_64;
   unsigned nr_blit_64;
   unsigned nr_pure_blit_64;
   unsigned nr_pure_shade_opaque_64;
   unsigned nr_pure_shade_64;
   unsigned nr_shade_64;
   unsigned nr_shade_opaque_64;
   unsigned nr_empty_16;
   unsigned nr_fully_covered_16;
   unsigned nr_partially_covered_16;
   unsigned nr_empty_4;
   unsigned nr_fully_covered_4;
   unsigned nr_partially_covered_4;
   unsigned nr_rect_fully_covered_4;
   unsigned nr_rect_partially_covered_4;
   unsigned nr_non_empty_4;
   unsigned nr_llvm_compiles;
   int64_t llvm_compile_time;  /**< total, in microseconds */

   unsigned nr_color_tile_clear;
   unsigned nr_color_tile_load;
   unsigned nr_color_tile_store;
};


extern struct lp_counters lp_count;


/** Increment the named counter (only for debug builds) */
#ifdef DEBUG
#define LP_COUNT(counter) lp_count.counter++
#define LP_COUNT_ADD(counter, incr)  lp_count.counter += (incr)
#define LP_COUNT_GET(counter) (lp_count.counter)
#else
#define LP_COUNT(counter) do {} while (0)
#define LP_COUNT_ADD(counter, incr) (void)(incr)
#define LP_COUNT_GET(counter) 0
#endif


extern void
lp_reset_counters(void);


extern void
lp_print_counters(void);


#endif /* LP_PERF_H */
