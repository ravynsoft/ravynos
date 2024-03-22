/*
 * Copyright 2012 Red Hat Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: Ben Skeggs
 *
 */

#include "util/u_inlines.h"
#include "nv30/nv30_context.h"
#include "nv30/nv30_winsys.h"

void
nv40_verttex_validate(struct nv30_context *nv30)
{
   struct nouveau_pushbuf *push = nv30->base.pushbuf;
   unsigned dirty = nv30->vertprog.dirty_samplers;

   while (dirty) {
      unsigned unit = ffs(dirty) - 1;
      struct nv30_sampler_view *sv = (void *)nv30->fragprog.textures[unit];
      struct nv30_sampler_state *ss = nv30->fragprog.samplers[unit];

      if (ss && sv) {
      } else {
         BEGIN_NV04(push, NV40_3D(VTXTEX_ENABLE(unit)), 1);
         PUSH_DATA (push, 0);
      }
      dirty &= ~(1 << unit);
   }

   nv30->vertprog.dirty_samplers = 0;
}

void
nv40_verttex_sampler_states_bind(struct pipe_context *pipe,
                                 unsigned nr, void **hwcso)
{
   struct nv30_context *nv30 = nv30_context(pipe);
   unsigned i;

   for (i = 0; i < nr; i++) {
      nv30->vertprog.samplers[i] = hwcso[i];
      nv30->vertprog.dirty_samplers |= (1 << i);
   }

   for (; i < nv30->vertprog.num_samplers; i++) {
      nv30->vertprog.samplers[i] = NULL;
      nv30->vertprog.dirty_samplers |= (1 << i);
   }

   nv30->vertprog.num_samplers = nr;
   nv30->dirty |= NV30_NEW_VERTTEX;
}


void
nv40_verttex_set_sampler_views(struct pipe_context *pipe, unsigned nr,
                               bool take_ownership,
                               struct pipe_sampler_view **views)
{
   struct nv30_context *nv30 = nv30_context(pipe);
   unsigned i;

   for (i = 0; i < nr; i++) {
      nouveau_bufctx_reset(nv30->bufctx, BUFCTX_VERTTEX(i));
      if (take_ownership) {
         pipe_sampler_view_reference(&nv30->vertprog.textures[i], NULL);
         nv30->vertprog.textures[i] = views[i];
      } else {
         pipe_sampler_view_reference(&nv30->vertprog.textures[i], views[i]);
      }
      nv30->vertprog.dirty_samplers |= (1 << i);
   }

   for (; i < nv30->vertprog.num_textures; i++) {
      nouveau_bufctx_reset(nv30->bufctx, BUFCTX_VERTTEX(i));
      pipe_sampler_view_reference(&nv30->vertprog.textures[i], NULL);
      nv30->vertprog.dirty_samplers |= (1 << i);
   }

   nv30->vertprog.num_textures = nr;
   nv30->dirty |= NV30_NEW_VERTTEX;
}

void
nv40_verttex_init(struct pipe_context *pipe)
{
   /* nothing */
}
