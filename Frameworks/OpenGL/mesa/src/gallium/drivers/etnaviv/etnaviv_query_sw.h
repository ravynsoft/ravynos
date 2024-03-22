/*
 * Copyright (c) 2016 Etnaviv Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Rob Clark <robclark@freedesktop.org>
 *    Christian Gmeiner <christian.gmeiner@gmail.com>
 */

#ifndef H_ETNAVIV_QUERY_SW
#define H_ETNAVIV_QUERY_SW

#include "etnaviv_query.h"

#define ETNA_QUERY_DRAW_CALLS    (ETNA_SW_QUERY_BASE + 0)
#define ETNA_QUERY_RS_OPERATIONS (ETNA_SW_QUERY_BASE + 1)

struct etna_sw_query {
   struct etna_query base;
   uint64_t begin_value, end_value;
};

static inline struct etna_sw_query *
etna_sw_query(struct etna_query *q)
{
   return (struct etna_sw_query *)q;
}

struct etna_query *
etna_sw_create_query(struct etna_context *ctx, unsigned query_type);

int
etna_sw_get_driver_query_info(struct pipe_screen *pscreen, unsigned index,
                              struct pipe_driver_query_info *info);

int
etna_sw_get_driver_query_group_info(struct pipe_screen *pscreen,
                                    unsigned index,
                                    struct pipe_driver_query_group_info *info);

#endif
