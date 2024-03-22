/*
 * Mesa 3-D graphics library
 *
 * Copyright 2007-2008 VMware, Inc.
 * Copyright (C) 2010 LunarG Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifdef ELT_TYPE

/**
 * Fetch all elements in [min_index, max_index] with bias, and use the
 * (rebased) index buffer as the draw elements.
 */
static bool
CONCAT2(vsplit_primitive_, ELT_TYPE)(struct vsplit_frontend *vsplit,
                                     unsigned istart, unsigned icount)
{
   struct draw_context *draw = vsplit->draw;
   const ELT_TYPE *ib = (const ELT_TYPE *) draw->pt.user.elts;
   const unsigned min_index = draw->pt.user.min_index;
   const unsigned max_index = draw->pt.user.max_index;
   const int elt_bias = draw->pt.user.eltBias;
   unsigned fetch_start, fetch_count;
   const uint16_t *draw_elts = NULL;
   const unsigned start = istart;
   const unsigned end = istart + icount;

   /* If the index buffer overflows we'll need to run
    * through the normal paths */
   if (end >= draw->pt.user.eltMax ||
       end < istart)
      return false;

   /* use the ib directly */
   if (min_index == 0 && sizeof(ib[0]) == sizeof(draw_elts[0])) {
      if (icount > vsplit->max_vertices)
         return false;

      for (unsigned i = 0; i < icount; i++) {
         ELT_TYPE idx = DRAW_GET_IDX(ib, start + i);
         if (idx < min_index || idx > max_index) {
            debug_printf("warning: index out of range\n");
         }
      }
      draw_elts = (const uint16_t *) (ib + istart);
   } else {
      /* have to go through vsplit->draw_elts */
      if (icount > vsplit->segment_size)
         return false;
   }

   /* this is faster only when we fetch less elements than the normal path */
   if (max_index - min_index > icount - 1)
      return false;

   if (elt_bias < 0 && (int) min_index < -elt_bias)
      return false;

   /* why this check? */
   for (unsigned i = 0; i < draw->pt.nr_vertex_elements; i++) {
      if (draw->pt.vertex_element[i].instance_divisor)
         return false;
   }

   fetch_start = min_index + elt_bias;
   fetch_count = max_index - min_index + 1;

   /* Check for overflow in the fetch_start */
   if (fetch_start < min_index || fetch_start < elt_bias)
      return false;

   if (!draw_elts) {
      if (min_index == 0) {
         for (unsigned i = 0; i < icount; i++) {
            ELT_TYPE idx = DRAW_GET_IDX(ib, i + start);

            if (idx < min_index || idx > max_index) {
               debug_printf("warning: index out of range\n");
            }
            vsplit->draw_elts[i] = (uint16_t) idx;
         }
      } else {
         for (unsigned i = 0; i < icount; i++) {
            ELT_TYPE idx = DRAW_GET_IDX(ib, i + start);

            if (idx < min_index || idx > max_index) {
               debug_printf("warning: index out of range\n");
            }
            vsplit->draw_elts[i] = (uint16_t) (idx - min_index);
         }
      }

      draw_elts = vsplit->draw_elts;
   }

   return vsplit->middle->run_linear_elts(vsplit->middle,
                                          fetch_start, fetch_count,
                                          draw_elts, icount, 0x0);
}


/**
 * Use the cache to prepare the fetch and draw elements, and flush.
 *
 * When spoken is TRUE, ispoken replaces istart;  When close is TRUE, iclose is
 * appended.
 */
static inline void
CONCAT2(vsplit_segment_cache_, ELT_TYPE)(struct vsplit_frontend *vsplit,
                                         unsigned flags,
                                         unsigned istart, unsigned icount,
                                         bool spoken, unsigned ispoken,
                                         bool close, unsigned iclose)
{
   struct draw_context *draw = vsplit->draw;
   const ELT_TYPE *ib = (const ELT_TYPE *) draw->pt.user.elts;
   const int ibias = draw->pt.user.eltBias;

   assert(icount + !!close <= vsplit->segment_size);

   vsplit_clear_cache(vsplit);

   spoken = !!spoken;
   if (ibias == 0) {
      if (spoken)
         ADD_CACHE(vsplit, ib, 0, ispoken, 0);

      for (unsigned i = spoken; i < icount; i++) {
         ADD_CACHE(vsplit, ib, istart, i, 0);
      }

      if (close)
         ADD_CACHE(vsplit, ib, 0, iclose, 0);
   } else {
      if (spoken)
         ADD_CACHE(vsplit, ib, 0, ispoken, ibias);

      for (unsigned i = spoken; i < icount; i++)
         ADD_CACHE(vsplit, ib, istart, i, ibias);

      if (close)
         ADD_CACHE(vsplit, ib, 0, iclose, ibias);
   }

   vsplit_flush_cache(vsplit, flags);
}


static void
CONCAT2(vsplit_segment_simple_, ELT_TYPE)(struct vsplit_frontend *vsplit,
                                          unsigned flags,
                                          unsigned istart,
                                          unsigned icount)
{
   CONCAT2(vsplit_segment_cache_, ELT_TYPE)(vsplit,
          flags, istart, icount, false, 0, false, 0);
}


static void
CONCAT2(vsplit_segment_loop_, ELT_TYPE)(struct vsplit_frontend *vsplit,
                                        unsigned flags,
                                        unsigned istart,
                                        unsigned icount,
                                        unsigned i0)
{
   const bool close_loop = ((flags) == DRAW_SPLIT_BEFORE);

   CONCAT2(vsplit_segment_cache_, ELT_TYPE)(vsplit,
          flags, istart, icount, false, 0, close_loop, i0);
}


static void
CONCAT2(vsplit_segment_fan_, ELT_TYPE)(struct vsplit_frontend *vsplit,
                                       unsigned flags,
                                       unsigned istart,
                                       unsigned icount,
                                       unsigned i0)
{
   const bool use_spoken = (((flags) & DRAW_SPLIT_BEFORE) != 0);

   CONCAT2(vsplit_segment_cache_, ELT_TYPE)(vsplit,
          flags, istart, icount, use_spoken, i0, false, 0);
}


#define LOCAL_VARS                                                         \
   struct vsplit_frontend *vsplit = (struct vsplit_frontend *) frontend;   \
   const enum mesa_prim prim = vsplit->prim;                          \
   const unsigned max_count_simple = vsplit->segment_size;                 \
   const unsigned max_count_loop = vsplit->segment_size - 1;               \
   const unsigned max_count_fan = vsplit->segment_size;

#define PRIMITIVE(istart, icount)   \
   CONCAT2(vsplit_primitive_, ELT_TYPE)(vsplit, istart, icount)

#else /* ELT_TYPE */

static void
vsplit_segment_simple_linear(struct vsplit_frontend *vsplit, unsigned flags,
                             unsigned istart, unsigned icount)
{
   assert(icount <= vsplit->max_vertices);
   vsplit->middle->run_linear(vsplit->middle, istart, icount, flags);
}


static void
vsplit_segment_loop_linear(struct vsplit_frontend *vsplit, unsigned flags,
                           unsigned istart, unsigned icount, unsigned i0)
{
   bool close_loop = (flags == DRAW_SPLIT_BEFORE);
   unsigned nr;

   assert(icount + !!close_loop <= vsplit->segment_size);

   /* need to draw the sections of the line loop as line strips */
   flags |= DRAW_LINE_LOOP_AS_STRIP;

   if (close_loop) {
      for (nr = 0; nr < icount; nr++)
         vsplit->fetch_elts[nr] = istart + nr;
      vsplit->fetch_elts[nr++] = i0;

      vsplit->middle->run(vsplit->middle, vsplit->fetch_elts, nr,
            vsplit->identity_draw_elts, nr, flags);
   } else {
      vsplit->middle->run_linear(vsplit->middle, istart, icount, flags);
   }
}


static void
vsplit_segment_fan_linear(struct vsplit_frontend *vsplit, unsigned flags,
                          unsigned istart, unsigned icount, unsigned i0)
{
   bool use_spoken = ((flags & DRAW_SPLIT_BEFORE) != 0);
   unsigned nr = 0;

   assert(icount <= vsplit->segment_size);

   if (use_spoken) {
      /* replace istart by i0 */
      vsplit->fetch_elts[nr++] = i0;
      for (unsigned i = 1 ; i < icount; i++)
         vsplit->fetch_elts[nr++] = istart + i;

      vsplit->middle->run(vsplit->middle, vsplit->fetch_elts, nr,
            vsplit->identity_draw_elts, nr, flags);
   } else {
      vsplit->middle->run_linear(vsplit->middle, istart, icount, flags);
   }
}


#define LOCAL_VARS                                                         \
   struct vsplit_frontend *vsplit = (struct vsplit_frontend *) frontend;   \
   const enum mesa_prim prim = vsplit->prim;                          \
   const unsigned max_count_simple = vsplit->max_vertices;                 \
   const unsigned max_count_loop = vsplit->segment_size - 1;               \
   const unsigned max_count_fan = vsplit->segment_size;

#define PRIMITIVE(istart, icount) false

#define ELT_TYPE linear

#endif /* ELT_TYPE */

#define FUNC_VARS                      \
   struct draw_pt_front_end *frontend, \
   unsigned start,                     \
   unsigned count

#define SEGMENT_SIMPLE(flags, istart, icount)   \
   CONCAT2(vsplit_segment_simple_, ELT_TYPE)(vsplit, flags, istart, icount)

#define SEGMENT_LOOP(flags, istart, icount, i0) \
   CONCAT2(vsplit_segment_loop_, ELT_TYPE)(vsplit, flags, istart, icount, i0)

#define SEGMENT_FAN(flags, istart, icount, i0)  \
   CONCAT2(vsplit_segment_fan_, ELT_TYPE)(vsplit, flags, istart, icount, i0)

#include "draw_split_tmp.h"

#undef ELT_TYPE
#undef ADD_CACHE
