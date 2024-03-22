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

#include "etnaviv_texture.h"

#include "hw/common.xml.h"

#include "etnaviv_clear_blit.h"
#include "etnaviv_context.h"
#include "etnaviv_emit.h"
#include "etnaviv_format.h"
#include "etnaviv_texture_desc.h"
#include "etnaviv_texture_state.h"
#include "etnaviv_translate.h"
#include "util/u_inlines.h"
#include "util/u_memory.h"

#include "drm-uapi/drm_fourcc.h"

static void
etna_bind_sampler_states(struct pipe_context *pctx, enum pipe_shader_type shader,
                         unsigned start_slot, unsigned num_samplers,
                         void **samplers)
{
   /* bind fragment sampler */
   struct etna_context *ctx = etna_context(pctx);
   struct etna_screen *screen = ctx->screen;
   int offset;

   switch (shader) {
   case PIPE_SHADER_FRAGMENT:
      offset = 0;
      ctx->num_fragment_samplers = num_samplers;
      break;
   case PIPE_SHADER_VERTEX:
      offset = screen->specs.vertex_sampler_offset;
      break;
   default:
      assert(!"Invalid shader");
      return;
   }

   uint32_t mask = 1 << offset;
   for (int idx = 0; idx < num_samplers; ++idx, mask <<= 1) {
      ctx->sampler[offset + idx] = samplers[idx];
      if (samplers[idx])
         ctx->active_samplers |= mask;
      else
         ctx->active_samplers &= ~mask;
   }

   ctx->dirty |= ETNA_DIRTY_SAMPLERS;
}

static bool
etna_configure_sampler_ts(struct etna_sampler_ts *sts, struct pipe_sampler_view *pview, bool enable)
{
   bool dirty = (sts->enable != enable);

   assert(sts);
   sts->enable = enable;

   if (!enable) {
      sts->TS_SAMPLER_CONFIG = 0;
      sts->TS_SAMPLER_STATUS_BASE.bo = NULL;
      return dirty;
   }

   struct etna_resource *rsc = etna_resource(pview->texture);
   struct etna_resource_level *lev = &rsc->levels[0];

   if ((lev->clear_value & 0xffffffff) != sts->TS_SAMPLER_CLEAR_VALUE ||
       (lev->clear_value >> 32) != sts->TS_SAMPLER_CLEAR_VALUE2)
      dirty = true;

   assert(rsc->ts_bo && etna_resource_level_ts_valid(lev));

   sts->mode = lev->ts_mode;
   sts->comp = lev->ts_compress_fmt >= 0;
   sts->TS_SAMPLER_CONFIG =
      VIVS_TS_SAMPLER_CONFIG_ENABLE |
      COND(lev->ts_compress_fmt >= 0, VIVS_TS_SAMPLER_CONFIG_COMPRESSION) |
      VIVS_TS_SAMPLER_CONFIG_COMPRESSION_FORMAT(lev->ts_compress_fmt);
   sts->TS_SAMPLER_CLEAR_VALUE = lev->clear_value;
   sts->TS_SAMPLER_CLEAR_VALUE2 = lev->clear_value >> 32;
   sts->TS_SAMPLER_STATUS_BASE.bo = rsc->ts_bo;
   sts->TS_SAMPLER_STATUS_BASE.offset = lev->ts_offset;
   sts->TS_SAMPLER_STATUS_BASE.flags = ETNA_RELOC_READ;

   return dirty;
}

/* Return true if the GPU can use sampler TS with this sampler view.
 * Sampler TS is an optimization used when rendering to textures, where
 * a resolve-in-place can be avoided when rendering has left a (valid) TS.
 */
static bool
etna_can_use_sampler_ts(struct pipe_sampler_view *view, int num)
{
   struct etna_resource *rsc = etna_resource(view->texture);
   struct etna_screen *screen = etna_screen(rsc->base.screen);

   /* Sampler TS can be used under the following conditions: */

   /* The resource TS is valid for level 0. */
   if (!etna_resource_level_ts_valid(&rsc->levels[0]))
      return false;

   /* The hardware supports it. */
   if (!VIV_FEATURE(screen, chipMinorFeatures2, TEXTURE_TILED_READ))
      return false;

   /* The sampler view will be bound to sampler < VIVS_TS_SAMPLER__LEN.
    * HALTI5 adds a mapping from sampler to sampler TS unit, but this is AFAIK
    * absent on earlier models. */
   if (num >= VIVS_TS_SAMPLER__LEN)
      return false;

   /* It is a texture, not a buffer. */
   if (rsc->base.target == PIPE_BUFFER)
      return false;

   /* Does not use compression or the hardware supports V4 compression. */
   if (rsc->levels[0].ts_compress_fmt >= 0 && !screen->specs.v4_compression)
      return false;

   /* The sampler will have one LOD, and it happens to be level 0.
    * (It is not sure if the hw supports it for other levels, but available
    *  state strongly suggests only one at a time). */
   if (view->u.tex.first_level != 0 ||
       MIN2(view->u.tex.last_level, rsc->base.last_level) != 0)
      return false;

   return true;
}

void
etna_update_sampler_source(struct pipe_sampler_view *view, int num)
{
   struct etna_resource *base = etna_resource(view->texture);
   struct etna_resource *to = base, *from = base;
   struct etna_context *ctx = etna_context(view->context);
   bool enable_sampler_ts = false;

   if (base->render && etna_resource_newer(etna_resource(base->render), base))
      from = etna_resource(base->render);

   if (base->texture)
      to = etna_resource(base->texture);

   if ((to != from) && etna_resource_older(to, from)) {
      etna_copy_resource(view->context, &to->base, &from->base,
                         view->u.tex.first_level,
                         MIN2(view->texture->last_level, view->u.tex.last_level));
      ctx->dirty |= ETNA_DIRTY_TEXTURE_CACHES;
   } else if (to == from) {
      if (etna_can_use_sampler_ts(view, num)) {
         enable_sampler_ts = true;
      } else if (etna_resource_needs_flush(to)) {
         /* Resolve TS if needed */
         etna_copy_resource(view->context, &to->base, &from->base,
                            view->u.tex.first_level,
                            MIN2(view->texture->last_level, view->u.tex.last_level));
         ctx->dirty |= ETNA_DIRTY_TEXTURE_CACHES;
      }
   }

   if (etna_configure_sampler_ts(ctx->ts_for_sampler_view(view), view, enable_sampler_ts)) {
      ctx->dirty |= ETNA_DIRTY_SAMPLER_VIEWS | ETNA_DIRTY_TEXTURE_CACHES;
      ctx->dirty_sampler_views |= (1 << num);
   }
}

static bool
etna_resource_sampler_compatible(struct etna_resource *res)
{
   if (util_format_is_compressed(res->base.format))
      return true;

   struct etna_screen *screen = etna_screen(res->base.screen);
   /* This GPU supports texturing from supertiled textures? */
   if (res->layout == ETNA_LAYOUT_SUPER_TILED && VIV_FEATURE(screen, chipMinorFeatures2, SUPERTILED_TEXTURE))
      return true;

   /* This GPU supports texturing from linear textures? */
   if (res->layout == ETNA_LAYOUT_LINEAR && VIV_FEATURE(screen, chipMinorFeatures1, LINEAR_TEXTURE_SUPPORT))
      return true;

   /* Otherwise, only support tiled layouts */
   if (res->layout != ETNA_LAYOUT_TILED)
      return false;

   /* If we have HALIGN support, we can allow for the RS padding */
   if (VIV_FEATURE(screen, chipMinorFeatures1, TEXTURE_HALIGN))
      return true;

   /* Non-HALIGN GPUs only accept 4x4 tile-aligned textures */
   if (res->halign != TEXTURE_HALIGN_FOUR)
      return false;

   return true;
}

struct etna_resource *
etna_texture_handle_incompatible(struct pipe_context *pctx, struct pipe_resource *prsc)
{
   struct etna_resource *res = etna_resource(prsc);

   if (!etna_resource_sampler_compatible(res)) {
      /* The original resource is not compatible with the sampler.
       * Allocate an appropriately tiled texture. */
      if (!res->texture) {
         struct pipe_resource templat = *prsc;

         templat.bind &= ~(PIPE_BIND_DEPTH_STENCIL | PIPE_BIND_RENDER_TARGET |
                           PIPE_BIND_BLENDABLE);
         res->texture =
            etna_resource_alloc(pctx->screen, ETNA_LAYOUT_TILED,
                                DRM_FORMAT_MOD_LINEAR, &templat);
      }

      if (!res->texture) {
         return NULL;
      }
      res = etna_resource(res->texture);
   }
   return res;
}

static void
set_sampler_views(struct etna_context *ctx, unsigned start, unsigned end,
                  unsigned nr, bool take_ownership, struct pipe_sampler_view **views)
{
   unsigned i, j;
   uint32_t mask = 1 << start;
   uint32_t prev_active_sampler_views = ctx->active_sampler_views;

   for (i = start, j = 0; j < nr; i++, j++, mask <<= 1) {
      struct pipe_sampler_view *view = views ? views[j] : NULL;

      if (take_ownership) {
         pipe_sampler_view_reference(&ctx->sampler_view[i], NULL);
         ctx->sampler_view[i] = view;
      } else {
         pipe_sampler_view_reference(&ctx->sampler_view[i], view);
      }
      if (view) {
         ctx->active_sampler_views |= mask;
         ctx->dirty_sampler_views |= mask;
      } else
         ctx->active_sampler_views &= ~mask;
   }

   for (; i < end; i++, mask <<= 1) {
      pipe_sampler_view_reference(&ctx->sampler_view[i], NULL);
      ctx->active_sampler_views &= ~mask;
   }

   /* sampler views that changed state (even to inactive) are also dirty */
   ctx->dirty_sampler_views |= ctx->active_sampler_views ^ prev_active_sampler_views;
}

static inline void
etna_fragtex_set_sampler_views(struct etna_context *ctx, unsigned nr,
                               bool take_ownership,
                               struct pipe_sampler_view **views)
{
   struct etna_screen *screen = ctx->screen;
   unsigned start = 0;
   unsigned end = start + screen->specs.fragment_sampler_count;

   set_sampler_views(ctx, start, end, nr, take_ownership, views);
   ctx->num_fragment_sampler_views = nr;
}


static inline void
etna_vertex_set_sampler_views(struct etna_context *ctx, unsigned nr,
                              bool take_ownership,
                              struct pipe_sampler_view **views)
{
   struct etna_screen *screen = ctx->screen;
   unsigned start = screen->specs.vertex_sampler_offset;
   unsigned end = start + screen->specs.vertex_sampler_count;

   set_sampler_views(ctx, start, end, nr, take_ownership, views);
}

static void
etna_set_sampler_views(struct pipe_context *pctx, enum pipe_shader_type shader,
                       unsigned start_slot, unsigned num_views,
                       unsigned unbind_num_trailing_slots,
                       bool take_ownership,
                       struct pipe_sampler_view **views)
{
   struct etna_context *ctx = etna_context(pctx);
   assert(start_slot == 0);

   ctx->dirty |= ETNA_DIRTY_SAMPLER_VIEWS | ETNA_DIRTY_TEXTURE_CACHES;

   switch (shader) {
   case PIPE_SHADER_FRAGMENT:
      etna_fragtex_set_sampler_views(ctx, num_views, take_ownership, views);
      break;
   case PIPE_SHADER_VERTEX:
      etna_vertex_set_sampler_views(ctx, num_views, take_ownership, views);
      break;
   default:;
   }
}

static void
etna_texture_barrier(struct pipe_context *pctx, unsigned flags)
{
   struct etna_context *ctx = etna_context(pctx);

   etna_set_state(ctx->stream, VIVS_GL_FLUSH_CACHE,
                  VIVS_GL_FLUSH_CACHE_COLOR | VIVS_GL_FLUSH_CACHE_DEPTH |
                  VIVS_GL_FLUSH_CACHE_TEXTURE);
   etna_set_state(ctx->stream, VIVS_GL_FLUSH_CACHE,
                  VIVS_GL_FLUSH_CACHE_TEXTUREVS);
   etna_stall(ctx->stream, SYNC_RECIPIENT_RA, SYNC_RECIPIENT_PE);
}

uint32_t
active_samplers_bits(struct etna_context *ctx)
{
   return ctx->active_sampler_views & ctx->active_samplers;
}

void
etna_texture_init(struct pipe_context *pctx)
{
   struct etna_context *ctx = etna_context(pctx);
   struct etna_screen *screen = ctx->screen;

   pctx->bind_sampler_states = etna_bind_sampler_states;
   pctx->set_sampler_views = etna_set_sampler_views;
   pctx->texture_barrier = etna_texture_barrier;

   if (screen->specs.halti >= 5) {
      u_suballocator_init(&ctx->tex_desc_allocator, pctx, 4096, 0,
                          PIPE_USAGE_IMMUTABLE, 0, true);
      etna_texture_desc_init(pctx);
   } else {
      etna_texture_state_init(pctx);
   }
}

void
etna_texture_fini(struct pipe_context *pctx)
{
   struct etna_context *ctx = etna_context(pctx);
   struct etna_screen *screen = ctx->screen;

   if (screen->specs.halti >= 5)
      u_suballocator_destroy(&ctx->tex_desc_allocator);
}
