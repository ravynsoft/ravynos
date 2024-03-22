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

#ifndef H_ETNAVIV_ZSA
#define H_ETNAVIV_ZSA

#include "pipe/p_context.h"
#include "pipe/p_state.h"

struct etna_zsa_state {
   struct pipe_depth_stencil_alpha_state base;

   uint32_t PE_DEPTH_CONFIG;
   uint32_t PE_ALPHA_OP;
   uint32_t PE_STENCIL_OP[2];
   uint32_t PE_STENCIL_CONFIG[2];
   uint32_t PE_STENCIL_CONFIG_EXT;
   uint32_t PE_STENCIL_CONFIG_EXT2[2];
   uint32_t RA_DEPTH_CONFIG;
   unsigned z_test_enabled:1;
   unsigned z_write_enabled:1;
   unsigned stencil_enabled:1;
   unsigned stencil_modified:1;

};

static inline struct etna_zsa_state *
etna_zsa_state(struct pipe_depth_stencil_alpha_state *zsa)
{
   return (struct etna_zsa_state *)zsa;
}

void *
etna_zsa_state_create(struct pipe_context *pctx,
                      const struct pipe_depth_stencil_alpha_state *so);

#endif
