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

#ifndef H_ETNAVIV_TEXTURE
#define H_ETNAVIV_TEXTURE

#include "drm/etnaviv_drmif.h"

#include "pipe/p_context.h"
#include "pipe/p_state.h"

struct etna_context;

struct etna_sampler_ts {
   unsigned enable:1;
   unsigned mode:1;
   unsigned comp:1;
   uint32_t TS_SAMPLER_CONFIG;
   struct etna_reloc TS_SAMPLER_STATUS_BASE;
   uint32_t TS_SAMPLER_CLEAR_VALUE;
   uint32_t TS_SAMPLER_CLEAR_VALUE2;
};

/* Initialize texture methods for context. */
void
etna_texture_init(struct pipe_context *pctx);

void
etna_texture_fini(struct pipe_context *pctx);

/* If the original resource is not compatible with the sampler.  Allocate
 * an appropriately tiled texture. */
struct etna_resource *
etna_texture_handle_incompatible(struct pipe_context *pctx, struct pipe_resource *prsc);

/* Create bit field that specifies which samplers are active and thus need to be
 * programmed
 * 32 bits is enough for 32 samplers. As far as I know this is the upper bound
 * supported on any Vivante hw
 * up to GC4000.
 */
uint32_t
active_samplers_bits(struct etna_context *ctx);

/* update TS / cache for a sampler if required */
void
etna_update_sampler_source(struct pipe_sampler_view *view, int num);

#endif
