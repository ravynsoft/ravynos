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

#include "nv_object.xml.h"
#include "nv30/nv30-40_3d.xml.h"
#include "nv30/nv30_context.h"
#include "nv30/nv30_format.h"
#include "nv30/nv30_winsys.h"

void
nv30_fragtex_validate(struct nv30_context *nv30)
{
   struct pipe_screen *pscreen = &nv30->screen->base.base;
   struct nouveau_object *eng3d = nv30->screen->eng3d;
   struct nouveau_pushbuf *push = nv30->base.pushbuf;
   unsigned dirty = nv30->fragprog.dirty_samplers;

   while (dirty) {
      unsigned unit = ffs(dirty) - 1;
      struct nv30_sampler_view *sv = (void *)nv30->fragprog.textures[unit];
      struct nv30_sampler_state *ss = nv30->fragprog.samplers[unit];

      PUSH_RESET(push, BUFCTX_FRAGTEX(unit));

      if (ss && sv) {
         const struct nv30_texfmt *fmt = nv30_texfmt(pscreen, sv->pipe.format);
         struct pipe_resource *pt = sv->pipe.texture;
         struct nv30_miptree *mt = nv30_miptree(pt);
         unsigned min_lod, max_lod;
         u32 filter = sv->filt | (ss->filt & sv->filt_mask);
         u32 format = sv->fmt | ss->fmt;
         u32 enable = ss->en;

         /* handle base_level when not using a mip filter, min/max level
          * is unfortunately ignored by the hardware otherwise
          */
         if (ss->pipe.min_mip_filter == PIPE_TEX_MIPFILTER_NONE) {
            if (sv->base_lod)
               filter += 0x00020000; /* N/L -> NMN/LMN */
            max_lod = sv->base_lod;
            min_lod = sv->base_lod;
         } else {
            max_lod = MIN2(ss->max_lod + sv->base_lod, sv->high_lod);
            min_lod = MIN2(ss->min_lod + sv->base_lod, max_lod);
         }

         if (eng3d->oclass >= NV40_3D_CLASS) {
            /* this is a tad stupid of the hardware, but there's no non-rcomp
             * z16/z24 texture formats to be had, we have to suffer and lose
             * some precision to handle this case.
             */
            if (ss->pipe.compare_mode != PIPE_TEX_COMPARE_R_TO_TEXTURE) {
               if (fmt->nv40 == NV40_3D_TEX_FORMAT_FORMAT_Z16)
                  format |= NV40_3D_TEX_FORMAT_FORMAT_A8L8;
               else
               if (fmt->nv40 == NV40_3D_TEX_FORMAT_FORMAT_Z24)
                  format |= NV40_3D_TEX_FORMAT_FORMAT_A16L16;
               else
                  format |= fmt->nv40;
            } else {
               format |= fmt->nv40;
            }

            enable |= (min_lod << 19) | (max_lod << 7);
            enable |= NV40_3D_TEX_ENABLE_ENABLE;

            BEGIN_NV04(push, NV40_3D(TEX_SIZE1(unit)), 1);
            PUSH_DATA (push, sv->npot_size1);
         } else {
            /* this is a tad stupid of the hardware, but there's no non-rcomp
             * z16/z24 texture formats to be had, we have to suffer and lose
             * some precision to handle this case.
             */
            if (ss->pipe.compare_mode != PIPE_TEX_COMPARE_R_TO_TEXTURE) {
               if (fmt->nv30 == NV30_3D_TEX_FORMAT_FORMAT_Z16) {
                  if (!ss->pipe.unnormalized_coords)
                     format |= NV30_3D_TEX_FORMAT_FORMAT_A8L8;
                  else
                     format |= NV30_3D_TEX_FORMAT_FORMAT_A8L8_RECT;
               } else
               if (fmt->nv30 == NV30_3D_TEX_FORMAT_FORMAT_Z24) {
                  if (!ss->pipe.unnormalized_coords)
                     format |= NV30_3D_TEX_FORMAT_FORMAT_HILO16;
                  else
                     format |= NV30_3D_TEX_FORMAT_FORMAT_HILO16_RECT;
               } else {
                  if (!ss->pipe.unnormalized_coords)
                     format |= fmt->nv30;
                  else
                     format |= fmt->nv30_rect;
               }
            } else {
               if (!ss->pipe.unnormalized_coords)
                  format |= fmt->nv30;
               else
                  format |= fmt->nv30_rect;
            }

            enable |= NV30_3D_TEX_ENABLE_ENABLE;
            enable |= (min_lod << 18) | (max_lod << 6);
         }

         BEGIN_NV04(push, NV30_3D(TEX_OFFSET(unit)), 8);
         PUSH_MTHDl(push, NV30_3D(TEX_OFFSET(unit)), BUFCTX_FRAGTEX(unit),
                          mt->base.bo, 0, NOUVEAU_BO_VRAM | NOUVEAU_BO_RD);
         PUSH_MTHDs(push, NV30_3D(TEX_FORMAT(unit)), BUFCTX_FRAGTEX(unit),
                          mt->base.bo, format, NOUVEAU_BO_VRAM | NOUVEAU_BO_RD,
                          NV30_3D_TEX_FORMAT_DMA0,
                          NV30_3D_TEX_FORMAT_DMA1);
         PUSH_DATA (push, sv->wrap | (ss->wrap & sv->wrap_mask));
         PUSH_DATA (push, enable);
         PUSH_DATA (push, sv->swz);
         PUSH_DATA (push, filter);
         PUSH_DATA (push, sv->npot_size0);
         PUSH_DATA (push, ss->bcol);
         BEGIN_NV04(push, NV30_3D(TEX_FILTER_OPTIMIZATION(unit)), 1);
         PUSH_DATA (push, nv30->config.filter);
      } else {
         BEGIN_NV04(push, NV30_3D(TEX_ENABLE(unit)), 1);
         PUSH_DATA (push, 0);
      }

      dirty &= ~(1 << unit);
   }

   nv30->fragprog.dirty_samplers = 0;
}

void
nv30_fragtex_sampler_states_bind(struct pipe_context *pipe,
                                 unsigned nr, void **hwcso)
{
   struct nv30_context *nv30 = nv30_context(pipe);
   unsigned i;

   for (i = 0; i < nr; i++) {
      nv30->fragprog.samplers[i] = hwcso[i];
      nv30->fragprog.dirty_samplers |= (1 << i);
   }

   for (; i < nv30->fragprog.num_samplers; i++) {
      nv30->fragprog.samplers[i] = NULL;
      nv30->fragprog.dirty_samplers |= (1 << i);
   }

   nv30->fragprog.num_samplers = nr;
   nv30->dirty |= NV30_NEW_FRAGTEX;
}


void
nv30_fragtex_set_sampler_views(struct pipe_context *pipe, unsigned nr,
                               bool take_ownership,
                               struct pipe_sampler_view **views)
{
   struct nv30_context *nv30 = nv30_context(pipe);
   unsigned i;

   for (i = 0; i < nr; i++) {
      nouveau_bufctx_reset(nv30->bufctx, BUFCTX_FRAGTEX(i));
      if (take_ownership) {
         pipe_sampler_view_reference(&nv30->fragprog.textures[i], NULL);
         nv30->fragprog.textures[i] = views[i];
      } else {
         pipe_sampler_view_reference(&nv30->fragprog.textures[i], views[i]);
      }
      nv30->fragprog.dirty_samplers |= (1 << i);
   }

   for (; i < nv30->fragprog.num_textures; i++) {
      nouveau_bufctx_reset(nv30->bufctx, BUFCTX_FRAGTEX(i));
      pipe_sampler_view_reference(&nv30->fragprog.textures[i], NULL);
      nv30->fragprog.dirty_samplers |= (1 << i);
   }

   nv30->fragprog.num_textures = nr;
   nv30->dirty |= NV30_NEW_FRAGTEX;
}


static void
nv30_set_sampler_views(struct pipe_context *pipe, enum pipe_shader_type shader,
                       unsigned start, unsigned nr,
                       unsigned unbind_num_trailing_slots,
                       bool take_ownership,
                       struct pipe_sampler_view **views)
{
   assert(start == 0);
   switch (shader) {
   case PIPE_SHADER_FRAGMENT:
      nv30_fragtex_set_sampler_views(pipe, nr, take_ownership, views);
      break;
   case PIPE_SHADER_VERTEX:
      nv40_verttex_set_sampler_views(pipe, nr, take_ownership, views);
      break;
   default:
      ;
   }
}


void
nv30_fragtex_init(struct pipe_context *pipe)
{
   pipe->set_sampler_views = nv30_set_sampler_views;
}
