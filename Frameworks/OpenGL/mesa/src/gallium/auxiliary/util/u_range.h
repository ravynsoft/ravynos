/*
 * Copyright 2013 Marek Olšák <maraeo@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE. */

/**
 * @file
 * 1D integer range, capable of the union and intersection operations.
 *
 * It only maintains a single interval which is extended when the union is
 * done. This implementation is partially thread-safe (readers are not
 * protected by a lock).
 *
 * @author Marek Olšák
 */

#ifndef U_RANGE_H
#define U_RANGE_H

#include "util/u_thread.h"
#include "pipe/p_state.h"
#include "pipe/p_screen.h"
#include "util/u_atomic.h"
#include "util/u_math.h"
#include "util/simple_mtx.h"

struct util_range {
   unsigned start; /* inclusive */
   unsigned end; /* exclusive */

   /* for the range to be consistent with multiple contexts: */
   simple_mtx_t write_mutex;
};


static inline void
util_range_set_empty(struct util_range *range)
{
   range->start = ~0;
   range->end = 0;
}

/* This is like a union of two sets. */
static inline void
util_range_add(struct pipe_resource *resource, struct util_range *range,
               unsigned start, unsigned end)
{
   if (start < range->start || end > range->end) {
      if (resource->flags & PIPE_RESOURCE_FLAG_SINGLE_THREAD_USE ||
          p_atomic_read(&resource->screen->num_contexts) == 1) {
         range->start = MIN2(start, range->start);
         range->end = MAX2(end, range->end);
      } else {
         simple_mtx_lock(&range->write_mutex);
         range->start = MIN2(start, range->start);
         range->end = MAX2(end, range->end);
         simple_mtx_unlock(&range->write_mutex);
      }
   }
}

static inline bool
util_ranges_intersect(const struct util_range *range,
                      unsigned start, unsigned end)
{
   return MAX2(start, range->start) < MIN2(end, range->end);
}


/* Init/deinit */

static inline void
util_range_init(struct util_range *range)
{
   (void) simple_mtx_init(&range->write_mutex, mtx_plain);
   util_range_set_empty(range);
}

static inline void
util_range_destroy(struct util_range *range)
{
   simple_mtx_destroy(&range->write_mutex);
}

#endif
