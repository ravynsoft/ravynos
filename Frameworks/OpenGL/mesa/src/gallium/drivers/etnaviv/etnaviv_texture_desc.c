/*
 * Copyright (c) 2017 Etnaviv Project
 * Copyright (C) 2017 Zodiac Inflight Innovations
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

#include "etnaviv_texture_desc.h"

#include "hw/common.xml.h"
#include "hw/texdesc_3d.xml.h"

#include "etnaviv_clear_blit.h"
#include "etnaviv_context.h"
#include "etnaviv_emit.h"
#include "etnaviv_format.h"
#include "etnaviv_translate.h"
#include "etnaviv_texture.h"
#include "util/u_inlines.h"
#include "util/u_memory.h"

#include <drm_fourcc.h>

struct etna_sampler_state_desc {
   struct pipe_sampler_state base;
   uint32_t SAMP_CTRL0;
   uint32_t SAMP_CTRL1;
   uint32_t SAMP_LOD_MINMAX;
   uint32_t SAMP_LOD_BIAS;
   uint32_t SAMP_ANISOTROPY;
};

static inline struct etna_sampler_state_desc *
etna_sampler_state_desc(struct pipe_sampler_state *samp)
{
   return (struct etna_sampler_state_desc *)samp;
}

struct etna_sampler_view_desc {
   struct pipe_sampler_view base;
   /* format-dependent merged with sampler state */
   uint32_t SAMP_CTRL0;
   uint32_t SAMP_CTRL0_MASK;
   uint32_t SAMP_CTRL1;

   struct pipe_resource *res;
   struct etna_reloc DESC_ADDR;
   struct etna_sampler_ts ts;
};

static inline struct etna_sampler_view_desc *
etna_sampler_view_desc(struct pipe_sampler_view *view)
{
   return (struct etna_sampler_view_desc *)view;
}

static void *
etna_create_sampler_state_desc(struct pipe_context *pipe,
                          const struct pipe_sampler_state *ss)
{
   struct etna_sampler_state_desc *cs = CALLOC_STRUCT(etna_sampler_state_desc);
   const bool ansio = ss->max_anisotropy > 1;

   if (!cs)
      return NULL;

   cs->base = *ss;

   cs->SAMP_CTRL0 =
      VIVS_NTE_DESCRIPTOR_SAMP_CTRL0_UWRAP(translate_texture_wrapmode(ss->wrap_s)) |
      VIVS_NTE_DESCRIPTOR_SAMP_CTRL0_VWRAP(translate_texture_wrapmode(ss->wrap_t)) |
      VIVS_NTE_DESCRIPTOR_SAMP_CTRL0_WWRAP(translate_texture_wrapmode(ss->wrap_r)) |
      VIVS_NTE_DESCRIPTOR_SAMP_CTRL0_MIN(translate_texture_filter(ss->min_img_filter)) |
      VIVS_NTE_DESCRIPTOR_SAMP_CTRL0_MIP(translate_texture_mipfilter(ss->min_mip_filter)) |
      VIVS_NTE_DESCRIPTOR_SAMP_CTRL0_MAG(translate_texture_filter(ss->mag_img_filter)) |
      VIVS_NTE_DESCRIPTOR_SAMP_CTRL0_UNK21;
      /* no ROUND_UV bit? */
   cs->SAMP_CTRL1 = VIVS_NTE_DESCRIPTOR_SAMP_CTRL1_UNK1;
   uint32_t min_lod_fp8 = MIN2(etna_float_to_fixp88(ss->min_lod), 0xfff);
   uint32_t max_lod_fp8 = MIN2(etna_float_to_fixp88(ss->max_lod), 0xfff);
   uint32_t max_lod_min = ss->min_img_filter != ss->mag_img_filter ? 4 : 0;

   cs->SAMP_LOD_MINMAX =
      VIVS_NTE_DESCRIPTOR_SAMP_LOD_MINMAX_MAX(MAX2(max_lod_fp8, max_lod_min)) |
      VIVS_NTE_DESCRIPTOR_SAMP_LOD_MINMAX_MIN(min_lod_fp8);

   cs->SAMP_LOD_BIAS =
      VIVS_NTE_DESCRIPTOR_SAMP_LOD_BIAS_BIAS(etna_float_to_fixp88(ss->lod_bias)) |
      COND(ss->lod_bias != 0.0, VIVS_NTE_DESCRIPTOR_SAMP_LOD_BIAS_ENABLE);
   cs->SAMP_ANISOTROPY = COND(ansio, etna_log2_fixp88(ss->max_anisotropy));

   return cs;
}

static void
etna_delete_sampler_state_desc(struct pipe_context *pctx, void *ss)
{
   FREE(ss);
}

static struct pipe_sampler_view *
etna_create_sampler_view_desc(struct pipe_context *pctx, struct pipe_resource *prsc,
                         const struct pipe_sampler_view *so)
{
   const struct util_format_description *desc = util_format_description(so->format);
   struct etna_sampler_view_desc *sv = CALLOC_STRUCT(etna_sampler_view_desc);
   struct etna_context *ctx = etna_context(pctx);
   const uint32_t format = translate_texture_format(so->format);
   const bool ext = !!(format & EXT_FORMAT);
   const bool astc = !!(format & ASTC_FORMAT);
   const uint32_t swiz = get_texture_swiz(so->format, so->swizzle_r,
                                          so->swizzle_g, so->swizzle_b,
                                          so->swizzle_a);
   unsigned suballoc_offset;

   if (!sv)
      return NULL;

   struct etna_resource *res = etna_texture_handle_incompatible(pctx, prsc);
   if (!res)
      goto error;

   sv->base = *so;
   pipe_reference_init(&sv->base.reference, 1);
   sv->base.texture = NULL;
   pipe_resource_reference(&sv->base.texture, prsc);
   sv->base.context = pctx;
   sv->SAMP_CTRL0_MASK = 0xffffffff;

   /* Determine whether target supported */
   uint32_t target_hw = translate_texture_target(sv->base.target);
   if (target_hw == ETNA_NO_MATCH) {
      BUG("Unhandled texture target");
      goto error;
   }

   /* Texture descriptor sampler bits */
   if (util_format_is_srgb(so->format))
      sv->SAMP_CTRL1 |= VIVS_NTE_DESCRIPTOR_SAMP_CTRL1_SRGB;

   /* Create texture descriptor */
   u_suballocator_alloc(&ctx->tex_desc_allocator, 256, 64,
                        &suballoc_offset, &sv->res);
   if (!sv->res)
      goto error;

   uint32_t *buf = etna_bo_map(etna_resource(sv->res)->bo) + suballoc_offset;

   /** GC7000 needs the size of the BASELOD level */
   uint32_t base_width = u_minify(res->base.width0, sv->base.u.tex.first_level);
   uint32_t base_height = u_minify(res->base.height0, sv->base.u.tex.first_level);
   uint32_t base_depth = u_minify(res->base.depth0, sv->base.u.tex.first_level);
   bool is_array = false;
   bool sint = util_format_is_pure_sint(so->format);

   switch(sv->base.target) {
   case PIPE_TEXTURE_1D:
      target_hw = TEXTURE_TYPE_2D;
      sv->SAMP_CTRL0_MASK = ~VIVS_NTE_DESCRIPTOR_SAMP_CTRL0_VWRAP__MASK;
      sv->SAMP_CTRL0 = VIVS_NTE_DESCRIPTOR_SAMP_CTRL0_VWRAP(TEXTURE_WRAPMODE_REPEAT);
      break;
   case PIPE_TEXTURE_1D_ARRAY:
      is_array = true;
      base_height = res->base.array_size;
      break;
   case PIPE_TEXTURE_2D_ARRAY:
      is_array = true;
      base_depth = res->base.array_size;
      break;
   default:
      break;
   }

#define DESC_SET(x, y) buf[(TEXDESC_##x)>>2] = (y)
   DESC_SET(CONFIG0, COND(!ext && !astc, VIVS_TE_SAMPLER_CONFIG0_FORMAT(format))
                   | VIVS_TE_SAMPLER_CONFIG0_TYPE(target_hw) |
                   COND(res->layout == ETNA_LAYOUT_LINEAR && !util_format_is_compressed(so->format),
                        VIVS_TE_SAMPLER_CONFIG0_ADDRESSING_MODE(TEXTURE_ADDRESSING_MODE_LINEAR)));
   DESC_SET(CONFIG1, COND(ext, VIVS_TE_SAMPLER_CONFIG1_FORMAT_EXT(format)) |
                     COND(astc, VIVS_TE_SAMPLER_CONFIG1_FORMAT_EXT(TEXTURE_FORMAT_EXT_ASTC)) |
                     COND(is_array, VIVS_TE_SAMPLER_CONFIG1_TEXTURE_ARRAY) |
                     VIVS_TE_SAMPLER_CONFIG1_HALIGN(res->halign) | swiz);
   DESC_SET(CONFIG2, 0x00030000 |
         COND(sint && desc->channel[0].size == 8, TE_SAMPLER_CONFIG2_SIGNED_INT8) |
         COND(sint && desc->channel[0].size == 16, TE_SAMPLER_CONFIG2_SIGNED_INT16));
   DESC_SET(LINEAR_STRIDE, res->levels[0].stride);
   DESC_SET(VOLUME, etna_log2_fixp88(base_depth));
   DESC_SET(SLICE, res->levels[0].layer_stride);
   DESC_SET(3D_CONFIG, VIVS_TE_SAMPLER_3D_CONFIG_DEPTH(base_depth));
   DESC_SET(ASTC0, COND(astc, VIVS_NTE_SAMPLER_ASTC0_ASTC_FORMAT(format)) |
                   VIVS_NTE_SAMPLER_ASTC0_UNK8(0xc) |
                   VIVS_NTE_SAMPLER_ASTC0_UNK16(0xc) |
                   VIVS_NTE_SAMPLER_ASTC0_UNK24(0xc));
   DESC_SET(BASELOD, TEXDESC_BASELOD_BASELOD(sv->base.u.tex.first_level) |
                     TEXDESC_BASELOD_MAXLOD(MIN2(sv->base.u.tex.last_level, res->base.last_level)));
   DESC_SET(LOG_SIZE_EXT, TEXDESC_LOG_SIZE_EXT_WIDTH(etna_log2_fixp88(base_width)) |
                          TEXDESC_LOG_SIZE_EXT_HEIGHT(etna_log2_fixp88(base_height)));
   DESC_SET(SIZE, VIVS_TE_SAMPLER_SIZE_WIDTH(base_width) |
                  VIVS_TE_SAMPLER_SIZE_HEIGHT(base_height));
   for (int lod = 0; lod <= res->base.last_level; ++lod)
      DESC_SET(LOD_ADDR(lod), etna_bo_gpu_va(res->bo) + res->levels[lod].offset);
#undef DESC_SET

   sv->DESC_ADDR.bo = etna_resource(sv->res)->bo;
   sv->DESC_ADDR.offset = suballoc_offset;
   sv->DESC_ADDR.flags = ETNA_RELOC_READ;

   return &sv->base;

error:
   free(sv);
   return NULL;
}

static void
etna_sampler_view_update_descriptor(struct etna_context *ctx,
                                    struct etna_cmd_stream *stream,
                                    struct etna_sampler_view_desc *sv)
{
   struct etna_resource *res = etna_resource(sv->base.texture);

   if (res->texture) {
      res = etna_resource(res->texture);
   }

   /* No need to ref LOD levels individually as they'll always come from the same bo */
   etna_cmd_stream_ref_bo(stream, res->bo, ETNA_RELOC_READ);
}

static void
etna_sampler_view_desc_destroy(struct pipe_context *pctx,
                          struct pipe_sampler_view *so)
{
   struct etna_sampler_view_desc *sv = etna_sampler_view_desc(so);

   pipe_resource_reference(&sv->base.texture, NULL);
   pipe_resource_reference(&sv->res, NULL);
   FREE(sv);
}

static void
etna_emit_texture_desc(struct etna_context *ctx)
{
   struct etna_cmd_stream *stream = ctx->stream;
   uint32_t active_samplers = active_samplers_bits(ctx);
   uint32_t dirty = ctx->dirty;

   if (unlikely(dirty & ETNA_DIRTY_SAMPLER_VIEWS)) {
      for (int x = 0; x < VIVS_TS_SAMPLER__LEN; ++x) {
         if ((1 << x) & active_samplers) {
            struct etna_sampler_view_desc *sv = etna_sampler_view_desc(ctx->sampler_view[x]);
            struct etna_resource *res = etna_resource(sv->base.texture);
            struct etna_reloc LOD_ADDR_0;

            if (!sv->ts.enable)
               continue;

            etna_set_state(stream, VIVS_TS_SAMPLER_CONFIG(x), sv->ts.TS_SAMPLER_CONFIG);
            etna_set_state_reloc(stream, VIVS_TS_SAMPLER_STATUS_BASE(x), &sv->ts.TS_SAMPLER_STATUS_BASE);
            etna_set_state(stream, VIVS_TS_SAMPLER_CLEAR_VALUE(x), sv->ts.TS_SAMPLER_CLEAR_VALUE);
            etna_set_state(stream, VIVS_TS_SAMPLER_CLEAR_VALUE2(x), sv->ts.TS_SAMPLER_CLEAR_VALUE2);

            LOD_ADDR_0.bo = res->bo;
            LOD_ADDR_0.offset = res->levels[0].offset;
            LOD_ADDR_0.flags = ETNA_RELOC_READ;

            etna_set_state_reloc(stream, VIVS_TS_SAMPLER_SURFACE_BASE(x), &LOD_ADDR_0);
         }
      }
   }

   if (unlikely(dirty & (ETNA_DIRTY_SAMPLERS | ETNA_DIRTY_SAMPLER_VIEWS))) {
      for (int x = 0; x < PIPE_MAX_SAMPLERS; ++x) {
         if ((1 << x) & active_samplers) {
            struct etna_sampler_state_desc *ss = etna_sampler_state_desc(ctx->sampler[x]);
            struct etna_sampler_view_desc *sv = etna_sampler_view_desc(ctx->sampler_view[x]);
            uint32_t SAMP_CTRL0 = (ss->SAMP_CTRL0 & sv->SAMP_CTRL0_MASK) | sv->SAMP_CTRL0;

            if (texture_use_int_filter(&sv->base, &ss->base, true))
               SAMP_CTRL0 |= VIVS_NTE_DESCRIPTOR_SAMP_CTRL0_INT_FILTER;

            etna_set_state(stream, VIVS_NTE_DESCRIPTOR_TX_CTRL(x),
               COND(sv->ts.enable, VIVS_NTE_DESCRIPTOR_TX_CTRL_TS_ENABLE) |
               VIVS_NTE_DESCRIPTOR_TX_CTRL_TS_MODE(sv->ts.mode) |
               VIVS_NTE_DESCRIPTOR_TX_CTRL_TS_INDEX(x)|
               COND(sv->ts.comp, VIVS_NTE_DESCRIPTOR_TX_CTRL_COMPRESSION) |
               COND(!sv->ts.mode, VIVS_NTE_DESCRIPTOR_TX_CTRL_128B_TILE));
            etna_set_state(stream, VIVS_NTE_DESCRIPTOR_SAMP_CTRL0(x), SAMP_CTRL0);
            etna_set_state(stream, VIVS_NTE_DESCRIPTOR_SAMP_CTRL1(x), ss->SAMP_CTRL1 | sv->SAMP_CTRL1);
            etna_set_state(stream, VIVS_NTE_DESCRIPTOR_SAMP_LOD_MINMAX(x), ss->SAMP_LOD_MINMAX);
            etna_set_state(stream, VIVS_NTE_DESCRIPTOR_SAMP_LOD_BIAS(x), ss->SAMP_LOD_BIAS);
            etna_set_state(stream, VIVS_NTE_DESCRIPTOR_SAMP_ANISOTROPY(x), ss->SAMP_ANISOTROPY);
         }
      }
   }

   if (unlikely(dirty & ETNA_DIRTY_SAMPLER_VIEWS)) {
      /* Set texture descriptors */
      for (int x = 0; x < PIPE_MAX_SAMPLERS; ++x) {
         if ((1 << x) & ctx->dirty_sampler_views) {
            if ((1 << x) & active_samplers) {
               struct etna_sampler_view_desc *sv = etna_sampler_view_desc(ctx->sampler_view[x]);
               etna_sampler_view_update_descriptor(ctx, stream, sv);
               etna_set_state_reloc(stream, VIVS_NTE_DESCRIPTOR_ADDR(x), &sv->DESC_ADDR);
            } else if ((1 << x) & ctx->prev_active_samplers){
               /* dummy texture descriptors for unused samplers */
               etna_set_state_reloc(stream, VIVS_NTE_DESCRIPTOR_ADDR(x),
                                    &ctx->screen->dummy_desc_reloc);
            }
         }
      }
   }

   if (unlikely(dirty & ETNA_DIRTY_SAMPLER_VIEWS)) {
      /* Invalidate all dirty sampler views.
       */
      for (int x = 0; x < PIPE_MAX_SAMPLERS; ++x) {
         if ((1 << x) & ctx->dirty_sampler_views) {
            etna_set_state(stream, VIVS_NTE_DESCRIPTOR_INVALIDATE,
                  VIVS_NTE_DESCRIPTOR_INVALIDATE_UNK29 |
                  VIVS_NTE_DESCRIPTOR_INVALIDATE_IDX(x));
         }
      }
   }

   ctx->prev_active_samplers = active_samplers;
}

static struct etna_sampler_ts*
etna_ts_for_sampler_view_state(struct pipe_sampler_view *pview)
{
   struct etna_sampler_view_desc *sv = etna_sampler_view_desc(pview);
   return &sv->ts;
}

void
etna_texture_desc_init(struct pipe_context *pctx)
{
   struct etna_context *ctx = etna_context(pctx);
   DBG("etnaviv: Using descriptor-based texturing\n");
   ctx->base.create_sampler_state = etna_create_sampler_state_desc;
   ctx->base.delete_sampler_state = etna_delete_sampler_state_desc;
   ctx->base.create_sampler_view = etna_create_sampler_view_desc;
   ctx->base.sampler_view_destroy = etna_sampler_view_desc_destroy;
   ctx->emit_texture_state = etna_emit_texture_desc;
   ctx->ts_for_sampler_view = etna_ts_for_sampler_view_state;
}

