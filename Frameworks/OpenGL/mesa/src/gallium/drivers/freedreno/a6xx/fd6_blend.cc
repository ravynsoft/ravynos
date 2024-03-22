/*
 * Copyright (C) 2016 Rob Clark <robclark@freedesktop.org>
 * Copyright © 2018 Google, Inc.
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
 * Authors:
 *    Rob Clark <robclark@freedesktop.org>
 */

#define FD_BO_NO_HARDPIN 1

#include "pipe/p_state.h"
#include "util/u_blend.h"
#include "util/u_dual_blend.h"
#include "util/u_memory.h"
#include "util/u_string.h"

#include "fd6_blend.h"
#include "fd6_context.h"
#include "fd6_pack.h"

// XXX move somewhere common.. same across a3xx/a4xx/a5xx..
static enum a3xx_rb_blend_opcode
blend_func(unsigned func)
{
   switch (func) {
   case PIPE_BLEND_ADD:
      return BLEND_DST_PLUS_SRC;
   case PIPE_BLEND_MIN:
      return BLEND_MIN_DST_SRC;
   case PIPE_BLEND_MAX:
      return BLEND_MAX_DST_SRC;
   case PIPE_BLEND_SUBTRACT:
      return BLEND_SRC_MINUS_DST;
   case PIPE_BLEND_REVERSE_SUBTRACT:
      return BLEND_DST_MINUS_SRC;
   default:
      DBG("invalid blend func: %x", func);
      return (enum a3xx_rb_blend_opcode)0;
   }
}

struct fd6_blend_variant *
__fd6_setup_blend_variant(struct fd6_blend_stateobj *blend,
                          unsigned sample_mask)
{
   const struct pipe_blend_state *cso = &blend->base;
   struct fd6_blend_variant *so;
   enum a3xx_rop_code rop = ROP_COPY;
   bool reads_dest = false;
   unsigned mrt_blend = 0;

   if (cso->logicop_enable) {
      rop = (enum a3xx_rop_code)cso->logicop_func; /* maps 1:1 */
      reads_dest = util_logicop_reads_dest((enum pipe_logicop)cso->logicop_func);
   }

   so = (struct fd6_blend_variant *)rzalloc_size(blend, sizeof(*so));
   if (!so)
      return NULL;

   struct fd_ringbuffer *ring = fd_ringbuffer_new_object(
      blend->ctx->pipe, ((A6XX_MAX_RENDER_TARGETS * 4) + 6) * 4);
   so->stateobj = ring;

   for (unsigned i = 0; i <= cso->max_rt; i++) {
      const struct pipe_rt_blend_state *rt;

      if (cso->independent_blend_enable)
         rt = &cso->rt[i];
      else
         rt = &cso->rt[0];

      OUT_REG(ring,
              A6XX_RB_MRT_BLEND_CONTROL(
                 i, .rgb_src_factor = fd_blend_factor(rt->rgb_src_factor),
                 .rgb_blend_opcode = blend_func(rt->rgb_func),
                 .rgb_dest_factor = fd_blend_factor(rt->rgb_dst_factor),
                 .alpha_src_factor = fd_blend_factor(rt->alpha_src_factor),
                 .alpha_blend_opcode = blend_func(rt->alpha_func),
                 .alpha_dest_factor = fd_blend_factor(rt->alpha_dst_factor), ));

      OUT_REG(ring,
              A6XX_RB_MRT_CONTROL(
                    i,
                    .blend = rt->blend_enable,
                    .blend2 = rt->blend_enable,
                    .rop_enable = cso->logicop_enable,
                    .rop_code = rop,
                    .component_enable = rt->colormask,
              )
      );

      if (rt->blend_enable) {
         mrt_blend |= (1 << i);
      }

      if (reads_dest) {
         mrt_blend |= (1 << i);
      }
   }

   OUT_REG(
      ring,
      A6XX_RB_DITHER_CNTL(
            .dither_mode_mrt0 = cso->dither ? DITHER_ALWAYS : DITHER_DISABLE,
            .dither_mode_mrt1 = cso->dither ? DITHER_ALWAYS : DITHER_DISABLE,
            .dither_mode_mrt2 = cso->dither ? DITHER_ALWAYS : DITHER_DISABLE,
            .dither_mode_mrt3 = cso->dither ? DITHER_ALWAYS : DITHER_DISABLE,
            .dither_mode_mrt4 = cso->dither ? DITHER_ALWAYS : DITHER_DISABLE,
            .dither_mode_mrt5 = cso->dither ? DITHER_ALWAYS : DITHER_DISABLE,
            .dither_mode_mrt6 = cso->dither ? DITHER_ALWAYS : DITHER_DISABLE,
            .dither_mode_mrt7 =
               cso->dither ? DITHER_ALWAYS : DITHER_DISABLE, ));

   OUT_REG(ring,
           A6XX_SP_BLEND_CNTL(
                 .enable_blend = mrt_blend,
                 .unk8 = true,
                 .dual_color_in_enable = blend->use_dual_src_blend,
                 .alpha_to_coverage = cso->alpha_to_coverage,
           ),
   );

   OUT_REG(ring,
           A6XX_RB_BLEND_CNTL(
                 .enable_blend = mrt_blend,
                 .independent_blend = cso->independent_blend_enable,
                 .dual_color_in_enable = blend->use_dual_src_blend,
                 .alpha_to_coverage = cso->alpha_to_coverage,
                 .alpha_to_one = cso->alpha_to_one,
                 .sample_mask = sample_mask,
           ),
   );

   so->sample_mask = sample_mask;

   util_dynarray_append(&blend->variants, struct fd6_blend_variant *, so);

   return so;
}

void *
fd6_blend_state_create(struct pipe_context *pctx,
                       const struct pipe_blend_state *cso)
{
   struct fd6_blend_stateobj *so;

   so = (struct fd6_blend_stateobj *)rzalloc_size(NULL, sizeof(*so));
   if (!so)
      return NULL;

   so->base = *cso;
   so->ctx = fd_context(pctx);

   if (cso->logicop_enable) {
      so->reads_dest |= util_logicop_reads_dest((enum pipe_logicop)cso->logicop_func);
   }

   so->use_dual_src_blend =
      cso->rt[0].blend_enable && util_blend_state_is_dual(cso, 0);

   STATIC_ASSERT((4 * PIPE_MAX_COLOR_BUFS) == (8 * sizeof(so->all_mrt_write_mask)));
   so->all_mrt_write_mask = 0;

   for (unsigned i = 0; i <= cso->max_rt; i++) {
      const struct pipe_rt_blend_state *rt =
         &cso->rt[cso->independent_blend_enable ? i : 0];

      so->reads_dest |= rt->blend_enable;

      so->all_mrt_write_mask |= rt->colormask << (4 * i);
   }

   util_dynarray_init(&so->variants, so);

   return so;
}

void
fd6_blend_state_delete(struct pipe_context *pctx, void *hwcso)
{
   struct fd6_blend_stateobj *so = (struct fd6_blend_stateobj *)hwcso;

   util_dynarray_foreach (&so->variants, struct fd6_blend_variant *, vp) {
      struct fd6_blend_variant *v = *vp;
      fd_ringbuffer_del(v->stateobj);
   }

   ralloc_free(so);
}
