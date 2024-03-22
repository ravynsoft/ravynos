/*
 * Copyright (c) 2020 Collabora, Ltd.
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
 *
 * Authors (Collabora):
 *   Alyssa Rosenzweig <alyssa.rosenzweig@collabora.com>
 */

#ifndef H_PAN_MINMAX_CACHE
#define H_PAN_MINMAX_CACHE

#include "util/u_transfer.h"

#define PANFROST_MINMAX_SIZE 64

struct panfrost_minmax_cache {
   uint64_t keys[PANFROST_MINMAX_SIZE];
   uint64_t values[PANFROST_MINMAX_SIZE];
   unsigned size;
   unsigned index;
};

bool panfrost_minmax_cache_get(struct panfrost_minmax_cache *cache,
                               unsigned start, unsigned count,
                               unsigned *min_index, unsigned *max_index);

void panfrost_minmax_cache_add(struct panfrost_minmax_cache *cache,
                               unsigned start, unsigned count,
                               unsigned min_index, unsigned max_index);

void panfrost_minmax_cache_invalidate(struct panfrost_minmax_cache *cache,
                                      struct pipe_transfer *transfer);

#endif
