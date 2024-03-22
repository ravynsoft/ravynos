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

#ifndef H_ETNAVIV_SURFACE
#define H_ETNAVIV_SURFACE

#include "etnaviv_resource.h"
#include "etnaviv_rs.h"
#include "etnaviv_tiling.h"
#include "pipe/p_state.h"

struct etna_surface {
   struct pipe_surface base;

   struct compiled_rs_state clear_command;
   /* Keep pointer to resource level, for fast clear */
   struct etna_resource_level *level;
   struct etna_reloc reloc[ETNA_MAX_PIXELPIPES];
   struct etna_reloc ts_reloc;
   uint32_t offset; /* pre-calculated level + layer offset */
   uint32_t ts_offset; /* pre-calculated level + layer TS offset */
   /* keep pointer to original resource (for when a render compatible resource is used) */
   struct pipe_resource *prsc;
};

static inline struct etna_surface *
etna_surface(struct pipe_surface *p)
{
   return (struct etna_surface *)p;
}

void
etna_surface_init(struct pipe_context *pctx);

#endif
