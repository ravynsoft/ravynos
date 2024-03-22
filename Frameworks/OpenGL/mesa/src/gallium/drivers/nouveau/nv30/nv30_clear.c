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

#include "pipe/p_defines.h"
#include "util/u_pack_color.h"

#include "nouveau_gldefs.h"
#include "nv_object.xml.h"
#include "nv30/nv30-40_3d.xml.h"
#include "nv30/nv30_context.h"
#include "nv30/nv30_winsys.h"
#include "nv30/nv30_format.h"

static inline uint32_t
pack_rgba(enum pipe_format format, const float *rgba)
{
   union util_color uc;
   util_pack_color(rgba, format, &uc);
   return uc.ui[0];
}

static inline uint32_t
pack_zeta(enum pipe_format format, double depth, unsigned stencil)
{
   uint32_t zuint = (uint32_t)(depth * 4294967295.0);
   if (format != PIPE_FORMAT_Z16_UNORM)
      return (zuint & 0xffffff00) | (stencil & 0xff);
   return zuint >> 16;
}

static void
nv30_clear(struct pipe_context *pipe, unsigned buffers, const struct pipe_scissor_state *scissor_state,
           const union pipe_color_union *color, double depth, unsigned stencil)
{
   struct nv30_context *nv30 = nv30_context(pipe);
   struct nouveau_pushbuf *push = nv30->base.pushbuf;
   struct pipe_framebuffer_state *fb = &nv30->framebuffer;
   uint32_t colr = 0, zeta = 0, mode = 0;

   if (!nv30_state_validate(nv30, NV30_NEW_FRAMEBUFFER, true))
      return;

   if (scissor_state) {
      uint32_t minx = scissor_state->minx;
      uint32_t maxx = MIN2(fb->width, scissor_state->maxx);
      uint32_t miny = scissor_state->miny;
      uint32_t maxy = MIN2(fb->height, scissor_state->maxy);

      BEGIN_NV04(push, NV30_3D(SCISSOR_HORIZ), 2);
      PUSH_DATA (push, minx | (maxx - minx) << 16);
      PUSH_DATA (push, miny | (maxy - miny) << 16);
   }
   else {
      BEGIN_NV04(push, NV30_3D(SCISSOR_HORIZ), 2);
      PUSH_DATA (push, 0x10000000);
      PUSH_DATA (push, 0x10000000);
   }

   if (buffers & PIPE_CLEAR_COLOR && fb->nr_cbufs) {
      colr  = pack_rgba(fb->cbufs[0]->format, color->f);
      mode |= NV30_3D_CLEAR_BUFFERS_COLOR_R |
              NV30_3D_CLEAR_BUFFERS_COLOR_G |
              NV30_3D_CLEAR_BUFFERS_COLOR_B |
              NV30_3D_CLEAR_BUFFERS_COLOR_A;
   }

   if (fb->zsbuf) {
      zeta = pack_zeta(fb->zsbuf->format, depth, stencil);
      if (buffers & PIPE_CLEAR_DEPTH)
         mode |= NV30_3D_CLEAR_BUFFERS_DEPTH;
      if (buffers & PIPE_CLEAR_STENCIL) {
         mode |= NV30_3D_CLEAR_BUFFERS_STENCIL;
         BEGIN_NV04(push, NV30_3D(STENCIL_ENABLE(0)), 2);
         PUSH_DATA (push, 0);
         PUSH_DATA (push, 0x000000ff);
         nv30->dirty |= NV30_NEW_ZSA;
      }
   }

   /*XXX: wtf? fixes clears sometimes not clearing on nv3x... */
   if (nv30->screen->eng3d->oclass < NV40_3D_CLASS) {
      BEGIN_NV04(push, NV30_3D(CLEAR_DEPTH_VALUE), 3);
      PUSH_DATA (push, zeta);
      PUSH_DATA (push, colr);
      PUSH_DATA (push, mode);
   }

   BEGIN_NV04(push, NV30_3D(CLEAR_DEPTH_VALUE), 3);
   PUSH_DATA (push, zeta);
   PUSH_DATA (push, colr);
   PUSH_DATA (push, mode);

   nv30_state_release(nv30);

   /* Make sure regular draw commands will get their scissor state set */
   nv30->dirty |= NV30_NEW_SCISSOR;
   nv30->state.scissor_off = 0;
}

static void
nv30_clear_render_target(struct pipe_context *pipe, struct pipe_surface *ps,
                         const union pipe_color_union *color,
                         unsigned x, unsigned y, unsigned w, unsigned h,
                         bool render_condition_enabled)
{
   struct nv30_context *nv30 = nv30_context(pipe);
   struct nv30_surface *sf = nv30_surface(ps);
   struct nv30_miptree *mt = nv30_miptree(ps->texture);
   struct nouveau_pushbuf *push = nv30->base.pushbuf;
   struct nouveau_object *eng3d = nv30->screen->eng3d;
   uint32_t rt_format;

   rt_format = nv30_format(pipe->screen, ps->format)->hw;
   if (util_format_get_blocksize(ps->format) == 4)
      rt_format |= NV30_3D_RT_FORMAT_ZETA_Z24S8;
   else
      rt_format |= NV30_3D_RT_FORMAT_ZETA_Z16;

   if (nv30_miptree(ps->texture)->swizzled) {
      rt_format |= NV30_3D_RT_FORMAT_TYPE_SWIZZLED;
      rt_format |= util_logbase2(sf->width) << 16;
      rt_format |= util_logbase2(sf->height) << 24;
   } else {
      rt_format |= NV30_3D_RT_FORMAT_TYPE_LINEAR;
   }

   if (!PUSH_SPACE_EX(push, 32, 1, 0) ||
       PUSH_REF1(push, mt->base.bo, NOUVEAU_BO_VRAM | NOUVEAU_BO_WR))
      return;

   BEGIN_NV04(push, NV30_3D(RT_ENABLE), 1);
   PUSH_DATA (push, NV30_3D_RT_ENABLE_COLOR0);
   BEGIN_NV04(push, NV30_3D(RT_HORIZ), 3);
   PUSH_DATA (push, sf->width << 16);
   PUSH_DATA (push, sf->height << 16);
   PUSH_DATA (push, rt_format);
   BEGIN_NV04(push, NV30_3D(COLOR0_PITCH), 2);
   if (eng3d->oclass < NV40_3D_CLASS)
      PUSH_DATA (push, (sf->pitch << 16) | sf->pitch);
   else
      PUSH_DATA (push, sf->pitch);
   PUSH_RELOC(push, mt->base.bo, sf->offset, NOUVEAU_BO_LOW, 0, 0);
   BEGIN_NV04(push, NV30_3D(SCISSOR_HORIZ), 2);
   PUSH_DATA (push, (w << 16) | x);
   PUSH_DATA (push, (h << 16) | y);

   BEGIN_NV04(push, NV30_3D(CLEAR_COLOR_VALUE), 2);
   PUSH_DATA (push, pack_rgba(ps->format, color->f));
   PUSH_DATA (push, NV30_3D_CLEAR_BUFFERS_COLOR_R |
                    NV30_3D_CLEAR_BUFFERS_COLOR_G |
                    NV30_3D_CLEAR_BUFFERS_COLOR_B |
                    NV30_3D_CLEAR_BUFFERS_COLOR_A);

   nv30->dirty |= NV30_NEW_FRAMEBUFFER | NV30_NEW_SCISSOR;
   nv30->state.scissor_off = 0;
}

static void
nv30_clear_depth_stencil(struct pipe_context *pipe, struct pipe_surface *ps,
                         unsigned buffers, double depth, unsigned stencil,
                         unsigned x, unsigned y, unsigned w, unsigned h,
                         bool render_condition_enabled)
{
   struct nv30_context *nv30 = nv30_context(pipe);
   struct nv30_surface *sf = nv30_surface(ps);
   struct nv30_miptree *mt = nv30_miptree(ps->texture);
   struct nouveau_pushbuf *push = nv30->base.pushbuf;
   struct nouveau_object *eng3d = nv30->screen->eng3d;
   uint32_t rt_format, mode = 0;

   rt_format = nv30_format(pipe->screen, ps->format)->hw;
   if (util_format_get_blocksize(ps->format) == 4)
      rt_format |= NV30_3D_RT_FORMAT_COLOR_A8R8G8B8;
   else
      rt_format |= NV30_3D_RT_FORMAT_COLOR_R5G6B5;

   if (nv30_miptree(ps->texture)->swizzled) {
      rt_format |= NV30_3D_RT_FORMAT_TYPE_SWIZZLED;
      rt_format |= util_logbase2(sf->width) << 16;
      rt_format |= util_logbase2(sf->height) << 24;
   } else {
      rt_format |= NV30_3D_RT_FORMAT_TYPE_LINEAR;
   }

   if (buffers & PIPE_CLEAR_DEPTH)
      mode |= NV30_3D_CLEAR_BUFFERS_DEPTH;
   if (buffers & PIPE_CLEAR_STENCIL)
      mode |= NV30_3D_CLEAR_BUFFERS_STENCIL;

   if (!PUSH_SPACE_EX(push, 32, 1, 0) ||
       PUSH_REF1(push, mt->base.bo, NOUVEAU_BO_VRAM | NOUVEAU_BO_WR))
      return;

   BEGIN_NV04(push, NV30_3D(RT_ENABLE), 1);
   PUSH_DATA (push, 0);
   BEGIN_NV04(push, NV30_3D(RT_HORIZ), 3);
   PUSH_DATA (push, sf->width << 16);
   PUSH_DATA (push, sf->height << 16);
   PUSH_DATA (push, rt_format);
   if (eng3d->oclass < NV40_3D_CLASS) {
      BEGIN_NV04(push, NV30_3D(COLOR0_PITCH), 1);
      PUSH_DATA (push, (sf->pitch << 16) | sf->pitch);
   } else {
      BEGIN_NV04(push, NV40_3D(ZETA_PITCH), 1);
      PUSH_DATA (push, sf->pitch);
   }
   BEGIN_NV04(push, NV30_3D(ZETA_OFFSET), 1);
   PUSH_RELOC(push, mt->base.bo, sf->offset, NOUVEAU_BO_LOW, 0, 0);
   BEGIN_NV04(push, NV30_3D(SCISSOR_HORIZ), 2);
   PUSH_DATA (push, (w << 16) | x);
   PUSH_DATA (push, (h << 16) | y);

   BEGIN_NV04(push, NV30_3D(CLEAR_DEPTH_VALUE), 1);
   PUSH_DATA (push, pack_zeta(ps->format, depth, stencil));
   BEGIN_NV04(push, NV30_3D(CLEAR_BUFFERS), 1);
   PUSH_DATA (push, mode);

   nv30->dirty |= NV30_NEW_FRAMEBUFFER | NV30_NEW_SCISSOR;
   nv30->state.scissor_off = 0;
}

void
nv30_clear_init(struct pipe_context *pipe)
{
   pipe->clear = nv30_clear;
   pipe->clear_render_target = nv30_clear_render_target;
   pipe->clear_depth_stencil = nv30_clear_depth_stencil;
}
