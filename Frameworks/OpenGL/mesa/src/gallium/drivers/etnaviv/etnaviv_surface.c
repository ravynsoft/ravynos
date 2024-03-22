/*
 * Copyright (c) 2012-2013 Etnaviv Project
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

#include "etnaviv_surface.h"
#include "etnaviv_screen.h"

#include "etnaviv_clear_blit.h"
#include "etnaviv_context.h"
#include "etnaviv_translate.h"
#include "pipe/p_defines.h"
#include "pipe/p_state.h"
#include "util/u_inlines.h"
#include "util/u_math.h"
#include "util/u_memory.h"

#include "hw/common.xml.h"

#include "drm-uapi/drm_fourcc.h"

static struct etna_resource *
etna_render_handle_incompatible(struct pipe_context *pctx,
                                struct pipe_resource *prsc,
                                unsigned int level)
{
   struct etna_context *ctx = etna_context(pctx);
   struct etna_screen *screen = ctx->screen;
   struct etna_resource *res = etna_resource(prsc);
   bool need_multitiled = screen->specs.pixel_pipes > 1 && !screen->specs.single_buffer;
   bool want_supertiled = screen->specs.can_supertile;
   unsigned int min_tilesize = etna_screen_get_tile_size(screen, TS_MODE_128B,
                                                         prsc->nr_samples > 1);

   /* Resource is compatible if it is tiled or PE is able to render to linear
    * and has multi tiling when required.
    */
   if ((res->layout != ETNA_LAYOUT_LINEAR ||
        (VIV_FEATURE(screen, chipMinorFeatures2, LINEAR_PE) &&
         (!VIV_FEATURE(screen, chipFeatures, FAST_CLEAR) ||
          res->levels[level].stride % min_tilesize == 0))) &&
       (!need_multitiled || (res->layout & ETNA_LAYOUT_BIT_MULTI)))
      return res;

   if (!res->render) {
      struct pipe_resource templat = *prsc;
      unsigned layout = ETNA_LAYOUT_TILED;
      if (need_multitiled)
         layout |= ETNA_LAYOUT_BIT_MULTI;
      if (want_supertiled)
         layout |= ETNA_LAYOUT_BIT_SUPER;

      templat.bind &= (PIPE_BIND_DEPTH_STENCIL | PIPE_BIND_RENDER_TARGET |
                        PIPE_BIND_BLENDABLE);
      res->render =
         etna_resource_alloc(pctx->screen, layout,
                             DRM_FORMAT_MOD_LINEAR, &templat);
      assert(res->render);
   }
   return etna_resource(res->render);
}

static struct pipe_surface *
etna_create_surface(struct pipe_context *pctx, struct pipe_resource *prsc,
                    const struct pipe_surface *templat)
{
   struct etna_context *ctx = etna_context(pctx);
   struct etna_screen *screen = ctx->screen;
   unsigned layer = templat->u.tex.first_layer;
   unsigned level = templat->u.tex.level;
   struct etna_resource *rsc = etna_render_handle_incompatible(pctx, prsc, level);
   struct etna_resource_level *lev = &rsc->levels[level];
   struct etna_surface *surf = CALLOC_STRUCT(etna_surface);

   if (!surf)
      return NULL;

   assert(templat->u.tex.first_layer == templat->u.tex.last_layer);
   assert(layer <= util_max_layer(prsc, level));

   surf->base.context = pctx;

   pipe_reference_init(&surf->base.reference, 1);
   pipe_resource_reference(&surf->base.texture, &rsc->base);
   pipe_resource_reference(&surf->prsc, prsc);

   /* Allocate a TS for the resource if there isn't one yet,
    * and it is allowed by the hw (width is a multiple of 16).
    * Avoid doing this for GPUs with MC1.0, as kernel sources
    * indicate the tile status module bypasses the memory
    * offset and MMU. */

   if (VIV_FEATURE(screen, chipFeatures, FAST_CLEAR) &&
       !rsc->ts_bo &&
       /* needs to be RS/BLT compatible for transfer_map/unmap */
       (rsc->levels[level].padded_width & ETNA_RS_WIDTH_MASK) == 0 &&
       (rsc->levels[level].padded_height & ETNA_RS_HEIGHT_MASK) == 0 &&
       etna_resource_hw_tileable(screen->specs.use_blt, prsc) &&
       /* Multi-layer resources would need to keep much more state (TS valid and
        * clear color per layer) and are unlikely to profit from TS usage. */
       prsc->depth0 == 1 && prsc->array_size == 1) {
      etna_screen_resource_alloc_ts(pctx->screen, rsc, 0);
   }

   surf->base.format = templat->format;
   surf->base.width = rsc->levels[level].width;
   surf->base.height = rsc->levels[level].height;
   surf->base.writable = templat->writable; /* what is this for anyway */
   surf->base.u = templat->u;
   surf->level = lev;

   /* XXX we don't really need a copy but it's convenient */
   surf->offset = lev->offset + layer * lev->layer_stride;

   /* Setup template relocations for this surface */
   for (unsigned pipe = 0; pipe < screen->specs.pixel_pipes; ++pipe) {
      surf->reloc[pipe].bo = rsc->bo;
      surf->reloc[pipe].offset = surf->offset;
      surf->reloc[pipe].flags = 0;
   }

   /* In single buffer mode, both pixel pipes must point to the same address,
    * for multi-tiled surfaces on the other hand the second pipe is expected to
    * point halfway the image vertically.
    */
   if (rsc->layout & ETNA_LAYOUT_BIT_MULTI)
      surf->reloc[1].offset = surf->offset + lev->stride * lev->padded_height / 2;

   if (surf->level->ts_size) {
      unsigned int layer_offset = layer * lev->ts_layer_stride;
      assert(layer_offset < surf->level->ts_size);

      surf->ts_offset = surf->level->ts_offset + layer_offset;

      surf->ts_reloc.bo = rsc->ts_bo;
      surf->ts_reloc.offset = surf->ts_offset;
      surf->ts_reloc.flags = 0;

      if (!screen->specs.use_blt) {
         /* This (ab)uses the RS as a plain buffer memset().
          * Currently uses a fixed row size of 64 bytes. Some benchmarking with
          * different sizes may be in order. */
         struct etna_bo *ts_bo = etna_resource(surf->base.texture)->ts_bo;
         etna_compile_rs_state(ctx, &surf->clear_command, &(struct rs_state) {
            .source_format = RS_FORMAT_A8R8G8B8,
            .dest_format = RS_FORMAT_A8R8G8B8,
            .dest = ts_bo,
            .dest_offset = surf->ts_offset,
            .dest_stride = 0x40,
            .dest_tiling = ETNA_LAYOUT_TILED,
            .dither = {0xffffffff, 0xffffffff},
            .width = 16,
            .height = align(lev->ts_layer_stride / 0x40, 4),
            .clear_value = {screen->specs.ts_clear_value},
            .clear_mode = VIVS_RS_CLEAR_CONTROL_MODE_ENABLED1,
            .clear_bits = 0xffff
         });
      }
   }

   return &surf->base;
}

static void
etna_surface_destroy(struct pipe_context *pctx, struct pipe_surface *psurf)
{
   pipe_resource_reference(&psurf->texture, NULL);
   pipe_resource_reference(&etna_surface(psurf)->prsc, NULL);
   FREE(psurf);
}

void
etna_surface_init(struct pipe_context *pctx)
{
   pctx->create_surface = etna_create_surface;
   pctx->surface_destroy = etna_surface_destroy;
}
