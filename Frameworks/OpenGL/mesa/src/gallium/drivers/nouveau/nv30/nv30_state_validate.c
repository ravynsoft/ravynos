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

#include "util/format/u_format.h"
#include "util/u_math.h"
#include "util/half_float.h"

#include "nv_object.xml.h"
#include "nv30/nv30-40_3d.xml.h"
#include "nv30/nv30_context.h"
#include "nv30/nv30_format.h"
#include "nv30/nv30_winsys.h"

static void
nv30_validate_fb(struct nv30_context *nv30)
{
   struct pipe_screen *pscreen = &nv30->screen->base.base;
   struct pipe_framebuffer_state *fb = &nv30->framebuffer;
   struct nouveau_pushbuf *push = nv30->base.pushbuf;
   struct nouveau_object *eng3d = nv30->screen->eng3d;
   uint32_t rt_format;
   int h = fb->height;
   int w = fb->width;
   int x = 0;
   int y = 0;

   nv30->state.rt_enable = (NV30_3D_RT_ENABLE_COLOR0 << fb->nr_cbufs) - 1;
   if (nv30->state.rt_enable > 1)
      nv30->state.rt_enable |= NV30_3D_RT_ENABLE_MRT;

   rt_format = 0;
   if (fb->nr_cbufs > 0) {
      struct nv30_miptree *mt = nv30_miptree(fb->cbufs[0]->texture);
      rt_format |= nv30_format(pscreen, fb->cbufs[0]->format)->hw;
      rt_format |= mt->ms_mode;
      if (mt->swizzled)
         rt_format |= NV30_3D_RT_FORMAT_TYPE_SWIZZLED;
      else
         rt_format |= NV30_3D_RT_FORMAT_TYPE_LINEAR;
   } else {
      if (fb->zsbuf && util_format_get_blocksize(fb->zsbuf->format) > 2)
         rt_format |= NV30_3D_RT_FORMAT_COLOR_A8R8G8B8;
      else
         rt_format |= NV30_3D_RT_FORMAT_COLOR_R5G6B5;
   }

   if (fb->zsbuf) {
      rt_format |= nv30_format(pscreen, fb->zsbuf->format)->hw;
      if (nv30_miptree(fb->zsbuf->texture)->swizzled)
         rt_format |= NV30_3D_RT_FORMAT_TYPE_SWIZZLED;
      else
         rt_format |= NV30_3D_RT_FORMAT_TYPE_LINEAR;
   } else {
      if (fb->nr_cbufs && util_format_get_blocksize(fb->cbufs[0]->format) > 2)
         rt_format |= NV30_3D_RT_FORMAT_ZETA_Z24S8;
      else
         rt_format |= NV30_3D_RT_FORMAT_ZETA_Z16;
   }

   /* hardware rounds down render target offset to 64 bytes, but surfaces
    * with a size of 2x2 pixel (16bpp) or 1x1 pixel (32bpp) have an
    * unaligned start address.  For these two important square formats
    * we can hack around this limitation by adjusting the viewport origin
    */
   if (nv30->state.rt_enable) {
      int off = nv30_surface(fb->cbufs[0])->offset & 63;
      if (off) {
         x += off / (util_format_get_blocksize(fb->cbufs[0]->format) * 2);
         w  = 16;
         h  = 2;
      }
   }

   if (rt_format & NV30_3D_RT_FORMAT_TYPE_SWIZZLED) {
      rt_format |= util_logbase2(w) << 16;
      rt_format |= util_logbase2(h) << 24;
   }

   if (!PUSH_SPACE(push, 64))
      return;
   PUSH_RESET(push, BUFCTX_FB);

   BEGIN_NV04(push, SUBC_3D(0x1da4), 1);
   PUSH_DATA (push, 0);
   BEGIN_NV04(push, NV30_3D(RT_HORIZ), 3);
   PUSH_DATA (push, w << 16);
   PUSH_DATA (push, h << 16);
   PUSH_DATA (push, rt_format);
   BEGIN_NV04(push, NV30_3D(VIEWPORT_TX_ORIGIN), 4);
   PUSH_DATA (push, (y << 16) | x);
   PUSH_DATA (push, 0);
   PUSH_DATA (push, ((w - 1) << 16) | 0);
   PUSH_DATA (push, ((h - 1) << 16) | 0);

   if ((nv30->state.rt_enable & NV30_3D_RT_ENABLE_COLOR0) || fb->zsbuf) {
      struct nv30_surface *rsf = nv30_surface(fb->cbufs[0]);
      struct nv30_surface *zsf = nv30_surface(fb->zsbuf);
      struct nouveau_bo *rbo, *zbo;

      if (!rsf)      rsf = zsf;
      else if (!zsf) zsf = rsf;
      rbo = nv30_miptree(rsf->base.texture)->base.bo;
      zbo = nv30_miptree(zsf->base.texture)->base.bo;

      if (eng3d->oclass >= NV40_3D_CLASS) {
         BEGIN_NV04(push, NV40_3D(ZETA_PITCH), 1);
         PUSH_DATA (push, zsf->pitch);
         BEGIN_NV04(push, NV40_3D(COLOR0_PITCH), 3);
         PUSH_DATA (push, rsf->pitch);
      } else {
         BEGIN_NV04(push, NV30_3D(COLOR0_PITCH), 3);
         PUSH_DATA (push, (zsf->pitch << 16) | rsf->pitch);
      }
      PUSH_MTHDl(push, NV30_3D(COLOR0_OFFSET), BUFCTX_FB, rbo, rsf->offset & ~63,
                       NOUVEAU_BO_VRAM | NOUVEAU_BO_RDWR);
      PUSH_MTHDl(push, NV30_3D(ZETA_OFFSET), BUFCTX_FB, zbo, zsf->offset & ~63,
                       NOUVEAU_BO_VRAM | NOUVEAU_BO_RDWR);
   }

   if (nv30->state.rt_enable & NV30_3D_RT_ENABLE_COLOR1) {
      struct nv30_surface *sf = nv30_surface(fb->cbufs[1]);
      struct nouveau_bo *bo = nv30_miptree(sf->base.texture)->base.bo;

      BEGIN_NV04(push, NV30_3D(COLOR1_OFFSET), 2);
      PUSH_MTHDl(push, NV30_3D(COLOR1_OFFSET), BUFCTX_FB, bo, sf->offset,
                       NOUVEAU_BO_VRAM | NOUVEAU_BO_RDWR);
      PUSH_DATA (push, sf->pitch);
   }

   if (nv30->state.rt_enable & NV40_3D_RT_ENABLE_COLOR2) {
      struct nv30_surface *sf = nv30_surface(fb->cbufs[2]);
      struct nouveau_bo *bo = nv30_miptree(sf->base.texture)->base.bo;

      BEGIN_NV04(push, NV40_3D(COLOR2_OFFSET), 1);
      PUSH_MTHDl(push, NV40_3D(COLOR2_OFFSET), BUFCTX_FB, bo, sf->offset,
                       NOUVEAU_BO_VRAM | NOUVEAU_BO_RDWR);
      BEGIN_NV04(push, NV40_3D(COLOR2_PITCH), 1);
      PUSH_DATA (push, sf->pitch);
   }

   if (nv30->state.rt_enable & NV40_3D_RT_ENABLE_COLOR3) {
      struct nv30_surface *sf = nv30_surface(fb->cbufs[3]);
      struct nouveau_bo *bo = nv30_miptree(sf->base.texture)->base.bo;

      BEGIN_NV04(push, NV40_3D(COLOR3_OFFSET), 1);
      PUSH_MTHDl(push, NV40_3D(COLOR3_OFFSET), BUFCTX_FB, bo, sf->offset,
                       NOUVEAU_BO_VRAM | NOUVEAU_BO_RDWR);
      BEGIN_NV04(push, NV40_3D(COLOR3_PITCH), 1);
      PUSH_DATA (push, sf->pitch);
   }
}

static void
nv30_validate_blend_colour(struct nv30_context *nv30)
{
   struct nouveau_pushbuf *push = nv30->base.pushbuf;
   float *rgba = nv30->blend_colour.color;

   if (nv30->framebuffer.nr_cbufs) {
      switch (nv30->framebuffer.cbufs[0]->format) {
      case PIPE_FORMAT_R16G16B16A16_FLOAT:
      case PIPE_FORMAT_R32G32B32A32_FLOAT:
         BEGIN_NV04(push, NV30_3D(BLEND_COLOR), 1);
         PUSH_DATA (push, (_mesa_float_to_half(rgba[0]) <<  0) |
                          (_mesa_float_to_half(rgba[1]) << 16));
         BEGIN_NV04(push, SUBC_3D(0x037c), 1);
         PUSH_DATA (push, (_mesa_float_to_half(rgba[2]) <<  0) |
                          (_mesa_float_to_half(rgba[3]) << 16));
         break;
      default:
         break;
      }
   }

   BEGIN_NV04(push, NV30_3D(BLEND_COLOR), 1);
   PUSH_DATA (push, (float_to_ubyte(rgba[3]) << 24) |
                    (float_to_ubyte(rgba[0]) << 16) |
                    (float_to_ubyte(rgba[1]) <<  8) |
                    (float_to_ubyte(rgba[2]) <<  0));
}

static void
nv30_validate_stencil_ref(struct nv30_context *nv30)
{
   struct nouveau_pushbuf *push = nv30->base.pushbuf;

   BEGIN_NV04(push, NV30_3D(STENCIL_FUNC_REF(0)), 1);
   PUSH_DATA (push, nv30->stencil_ref.ref_value[0]);
   BEGIN_NV04(push, NV30_3D(STENCIL_FUNC_REF(1)), 1);
   PUSH_DATA (push, nv30->stencil_ref.ref_value[1]);
}

static void
nv30_validate_stipple(struct nv30_context *nv30)
{
   struct nouveau_pushbuf *push = nv30->base.pushbuf;

   BEGIN_NV04(push, NV30_3D(POLYGON_STIPPLE_PATTERN(0)), 32);
   PUSH_DATAp(push, nv30->stipple.stipple, 32);
}

static void
nv30_validate_scissor(struct nv30_context *nv30)
{
   struct nouveau_pushbuf *push = nv30->base.pushbuf;
   struct pipe_scissor_state *s = &nv30->scissor;
   bool rast_scissor = nv30->rast ? nv30->rast->pipe.scissor : false;

   if (!(nv30->dirty & NV30_NEW_SCISSOR) &&
       rast_scissor != nv30->state.scissor_off)
      return;
   nv30->state.scissor_off = !rast_scissor;

   BEGIN_NV04(push, NV30_3D(SCISSOR_HORIZ), 2);
   if (rast_scissor) {
      PUSH_DATA (push, ((s->maxx - s->minx) << 16) | s->minx);
      PUSH_DATA (push, ((s->maxy - s->miny) << 16) | s->miny);
   } else {
      PUSH_DATA (push, 0x10000000);
      PUSH_DATA (push, 0x10000000);
   }
}

static void
nv30_validate_viewport(struct nv30_context *nv30)
{
   struct nouveau_pushbuf *push = nv30->base.pushbuf;
   struct pipe_viewport_state *vp = &nv30->viewport;

   unsigned x = CLAMP(vp->translate[0] - fabsf(vp->scale[0]), 0, 4095);
   unsigned y = CLAMP(vp->translate[1] - fabsf(vp->scale[1]), 0, 4095);
   unsigned w = CLAMP(2.0f * fabsf(vp->scale[0]), 0, 4096);
   unsigned h = CLAMP(2.0f * fabsf(vp->scale[1]), 0, 4096);

   BEGIN_NV04(push, NV30_3D(VIEWPORT_TRANSLATE_X), 8);
   PUSH_DATAf(push, vp->translate[0]);
   PUSH_DATAf(push, vp->translate[1]);
   PUSH_DATAf(push, vp->translate[2]);
   PUSH_DATAf(push, 0.0f);
   PUSH_DATAf(push, vp->scale[0]);
   PUSH_DATAf(push, vp->scale[1]);
   PUSH_DATAf(push, vp->scale[2]);
   PUSH_DATAf(push, 0.0f);
   BEGIN_NV04(push, NV30_3D(DEPTH_RANGE_NEAR), 2);
   PUSH_DATAf(push, vp->translate[2] - fabsf(vp->scale[2]));
   PUSH_DATAf(push, vp->translate[2] + fabsf(vp->scale[2]));

   BEGIN_NV04(push, NV30_3D(VIEWPORT_HORIZ), 2);
   PUSH_DATA (push, (w << 16) | x);
   PUSH_DATA (push, (h << 16) | y);
}

static void
nv30_validate_clip(struct nv30_context *nv30)
{
   struct nouveau_pushbuf *push = nv30->base.pushbuf;
   unsigned i;
   uint32_t clpd_enable = 0;

   for (i = 0; i < 6; i++) {
      if (nv30->dirty & NV30_NEW_CLIP) {
         BEGIN_NV04(push, NV30_3D(VP_UPLOAD_CONST_ID), 5);
         PUSH_DATA (push, i);
         PUSH_DATAp(push, nv30->clip.ucp[i], 4);
      }
      if (nv30->rast->pipe.clip_plane_enable & (1 << i))
         clpd_enable |= 2 << (4*i);
   }

   BEGIN_NV04(push, NV30_3D(VP_CLIP_PLANES_ENABLE), 1);
   PUSH_DATA (push, clpd_enable);
}

static void
nv30_validate_blend(struct nv30_context *nv30)
{
   struct nouveau_pushbuf *push = nv30->base.pushbuf;

   PUSH_SPACE(push, nv30->blend->size);
   PUSH_DATAp(push, nv30->blend->data, nv30->blend->size);
}

static void
nv30_validate_zsa(struct nv30_context *nv30)
{
   struct nouveau_pushbuf *push = nv30->base.pushbuf;

   PUSH_SPACE(push, nv30->zsa->size);
   PUSH_DATAp(push, nv30->zsa->data, nv30->zsa->size);
}

static void
nv30_validate_rasterizer(struct nv30_context *nv30)
{
   struct nouveau_pushbuf *push = nv30->base.pushbuf;

   PUSH_SPACE(push, nv30->rast->size);
   PUSH_DATAp(push, nv30->rast->data, nv30->rast->size);
}

static void
nv30_validate_multisample(struct nv30_context *nv30)
{
   struct pipe_rasterizer_state *rasterizer = &nv30->rast->pipe;
   struct pipe_blend_state *blend = &nv30->blend->pipe;
   struct nouveau_pushbuf *push = nv30->base.pushbuf;
   uint32_t ctrl = nv30->sample_mask << 16;

   if (blend->alpha_to_one)
      ctrl |= 0x00000100;
   if (blend->alpha_to_coverage)
      ctrl |= 0x00000010;
   if (rasterizer->multisample)
      ctrl |= 0x00000001;

   BEGIN_NV04(push, NV30_3D(MULTISAMPLE_CONTROL), 1);
   PUSH_DATA (push, ctrl);
}

static void
nv30_validate_fragment(struct nv30_context *nv30)
{
   struct nouveau_pushbuf *push = nv30->base.pushbuf;
   struct nv30_fragprog *fp = nv30->fragprog.program;

   BEGIN_NV04(push, NV30_3D(RT_ENABLE), 1);
   PUSH_DATA (push, nv30->state.rt_enable & (fp ? ~fp->rt_enable : 0x1f));
   BEGIN_NV04(push, NV30_3D(COORD_CONVENTIONS), 1);
   PUSH_DATA (push, (fp ? fp->coord_conventions : 0) | nv30->framebuffer.height);
}

static void
nv30_validate_point_coord(struct nv30_context *nv30)
{
   struct pipe_rasterizer_state *rasterizer = &nv30->rast->pipe;
   struct nouveau_pushbuf *push = nv30->base.pushbuf;
   struct nv30_fragprog *fp = nv30->fragprog.program;
   uint32_t hw = 0x00000000;

   if (rasterizer) {
      hw |= (nv30->rast->pipe.sprite_coord_enable & 0xff) << 8;
      if (fp)
         hw |= fp->point_sprite_control;

      if (rasterizer->sprite_coord_mode == PIPE_SPRITE_COORD_LOWER_LEFT) {
         if (hw)
            nv30->draw_flags |= NV30_NEW_RASTERIZER;
      } else
      if (rasterizer->point_quad_rasterization) {
         hw |= NV30_3D_POINT_SPRITE_ENABLE;
      }
   }

   BEGIN_NV04(push, NV30_3D(POINT_SPRITE), 1);
   PUSH_DATA (push, hw);
}

struct state_validate {
   void (*func)(struct nv30_context *);
   uint32_t mask;
};

static struct state_validate hwtnl_validate_list[] = {
    { nv30_validate_fb,            NV30_NEW_FRAMEBUFFER },
    { nv30_validate_blend,         NV30_NEW_BLEND },
    { nv30_validate_zsa,           NV30_NEW_ZSA },
    { nv30_validate_rasterizer,    NV30_NEW_RASTERIZER },
    { nv30_validate_multisample,   NV30_NEW_SAMPLE_MASK | NV30_NEW_BLEND |
                                   NV30_NEW_RASTERIZER },
    { nv30_validate_blend_colour,  NV30_NEW_BLEND_COLOUR |
                                   NV30_NEW_FRAMEBUFFER },
    { nv30_validate_stencil_ref,   NV30_NEW_STENCIL_REF },
    { nv30_validate_stipple,       NV30_NEW_STIPPLE },
    { nv30_validate_scissor,       NV30_NEW_SCISSOR | NV30_NEW_RASTERIZER },
    { nv30_validate_viewport,      NV30_NEW_VIEWPORT },
    { nv30_validate_clip,          NV30_NEW_CLIP | NV30_NEW_RASTERIZER },
    { nv30_fragprog_validate,      NV30_NEW_FRAGPROG | NV30_NEW_FRAGCONST },
    { nv30_vertprog_validate,      NV30_NEW_VERTPROG | NV30_NEW_VERTCONST |
                                   NV30_NEW_FRAGPROG | NV30_NEW_RASTERIZER },
    { nv30_validate_fragment,      NV30_NEW_FRAMEBUFFER | NV30_NEW_FRAGPROG },
    { nv30_validate_point_coord,   NV30_NEW_RASTERIZER | NV30_NEW_FRAGPROG },
    { nv30_fragtex_validate,       NV30_NEW_FRAGTEX },
    { nv40_verttex_validate,       NV30_NEW_VERTTEX },
    { nv30_vbo_validate,           NV30_NEW_VERTEX | NV30_NEW_ARRAYS },
    {}
};

#define NV30_SWTNL_MASK (NV30_NEW_VIEWPORT |  \
                         NV30_NEW_CLIP |      \
                         NV30_NEW_VERTPROG |  \
                         NV30_NEW_VERTCONST | \
                         NV30_NEW_VERTTEX |   \
                         NV30_NEW_VERTEX |    \
                         NV30_NEW_ARRAYS)

static struct state_validate swtnl_validate_list[] = {
    { nv30_validate_fb,            NV30_NEW_FRAMEBUFFER },
    { nv30_validate_blend,         NV30_NEW_BLEND },
    { nv30_validate_zsa,           NV30_NEW_ZSA },
    { nv30_validate_rasterizer,    NV30_NEW_RASTERIZER },
    { nv30_validate_multisample,   NV30_NEW_SAMPLE_MASK | NV30_NEW_BLEND |
                                   NV30_NEW_RASTERIZER },
    { nv30_validate_blend_colour,  NV30_NEW_BLEND_COLOUR |
                                   NV30_NEW_FRAMEBUFFER },
    { nv30_validate_stencil_ref,   NV30_NEW_STENCIL_REF },
    { nv30_validate_stipple,       NV30_NEW_STIPPLE },
    { nv30_validate_scissor,       NV30_NEW_SCISSOR | NV30_NEW_RASTERIZER },
    { nv30_fragprog_validate,      NV30_NEW_FRAGPROG | NV30_NEW_FRAGCONST },
    { nv30_validate_fragment,      NV30_NEW_FRAMEBUFFER | NV30_NEW_FRAGPROG },
    { nv30_fragtex_validate,       NV30_NEW_FRAGTEX },
    {}
};

static void
nv30_state_context_switch(struct nv30_context *nv30)
{
   struct nv30_context *prev = nv30->screen->cur_ctx;

   if (prev)
      nv30->state = prev->state;
   nv30->dirty = NV30_NEW_ALL;

   if (!nv30->vertex)
      nv30->dirty &= ~(NV30_NEW_VERTEX | NV30_NEW_ARRAYS);

   if (!nv30->vertprog.program)
      nv30->dirty &= ~NV30_NEW_VERTPROG;
   if (!nv30->fragprog.program)
      nv30->dirty &= ~NV30_NEW_FRAGPROG;

   if (!nv30->blend)
      nv30->dirty &= ~NV30_NEW_BLEND;
   if (!nv30->rast)
      nv30->dirty &= ~NV30_NEW_RASTERIZER;
   if (!nv30->zsa)
      nv30->dirty &= ~NV30_NEW_ZSA;

   nv30->screen->cur_ctx = nv30;
}

bool
nv30_state_validate(struct nv30_context *nv30, uint32_t mask, bool hwtnl)
{
   struct nouveau_pushbuf *push = nv30->base.pushbuf;
   struct nouveau_bufctx *bctx = nv30->bufctx;
   struct nouveau_bufref *bref;
   struct state_validate *validate;

   if (nv30->screen->cur_ctx != nv30)
      nv30_state_context_switch(nv30);

   if (hwtnl) {
      nv30->draw_dirty |= nv30->dirty;
      if (nv30->draw_flags) {
         nv30->draw_flags &= ~nv30->dirty;
         if (!nv30->draw_flags)
            nv30->dirty |= NV30_SWTNL_MASK;
      }
   }

   if (!nv30->draw_flags)
      validate = hwtnl_validate_list;
   else
      validate = swtnl_validate_list;

   mask &= nv30->dirty;

   if (mask) {
      while (validate->func) {
         if (mask & validate->mask)
            validate->func(nv30);
         validate++;
      }

      nv30->dirty &= ~mask;
   }

   nouveau_pushbuf_bufctx(push, bctx);
   if (PUSH_VAL(push)) {
      nouveau_pushbuf_bufctx(push, NULL);
      return false;
   }

   /*XXX*/
   BEGIN_NV04(push, NV30_3D(VTX_CACHE_INVALIDATE_1710), 1);
   PUSH_DATA (push, 0);
   if (nv30->screen->eng3d->oclass >= NV40_3D_CLASS) {
      BEGIN_NV04(push, NV40_3D(TEX_CACHE_CTL), 1);
      PUSH_DATA (push, 2);
      BEGIN_NV04(push, NV40_3D(TEX_CACHE_CTL), 1);
      PUSH_DATA (push, 1);
      BEGIN_NV04(push, NV30_3D(R1718), 1);
      PUSH_DATA (push, 0);
      BEGIN_NV04(push, NV30_3D(R1718), 1);
      PUSH_DATA (push, 0);
      BEGIN_NV04(push, NV30_3D(R1718), 1);
      PUSH_DATA (push, 0);
   }

   LIST_FOR_EACH_ENTRY(bref, &bctx->current, thead) {
      struct nv04_resource *res = bref->priv;
      if (res && res->mm) {
         nouveau_fence_ref(nv30->base.fence, &res->fence);

         if (bref->flags & NOUVEAU_BO_RD)
            res->status |= NOUVEAU_BUFFER_STATUS_GPU_READING;

         if (bref->flags & NOUVEAU_BO_WR) {
            nouveau_fence_ref(nv30->base.fence, &res->fence_wr);
            res->status |= NOUVEAU_BUFFER_STATUS_GPU_WRITING;
         }
      }
   }

   return true;
}

void
nv30_state_release(struct nv30_context *nv30)
{
   nouveau_pushbuf_bufctx(nv30->base.pushbuf, NULL);
}
