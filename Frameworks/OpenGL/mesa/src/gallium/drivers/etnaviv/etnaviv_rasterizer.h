/*
 * Copyright (c) 2012-2015 Etnaviv Project
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
 *    Wladimir J. van der Laan <laanwj@gmail.com>
 */

#ifndef H_ETNAVIV_RASTERIZER
#define H_ETNAVIV_RASTERIZER

#include "pipe/p_context.h"
#include "pipe/p_state.h"

struct etna_rasterizer_state {
   struct pipe_rasterizer_state base;

   uint32_t PA_CONFIG;
   uint32_t PA_LINE_WIDTH;
   uint32_t PA_POINT_SIZE;
   uint32_t PA_SYSTEM_MODE;
   uint32_t SE_DEPTH_SCALE;
   uint32_t SE_DEPTH_BIAS;
   uint32_t SE_CONFIG;
   bool point_size_per_vertex;
   bool scissor;
};

static inline struct etna_rasterizer_state *
etna_rasterizer_state(struct pipe_rasterizer_state *rast)
{
   return (struct etna_rasterizer_state *)rast;
}

void *
etna_rasterizer_state_create(struct pipe_context *pctx,
                             const struct pipe_rasterizer_state *so);

#endif
