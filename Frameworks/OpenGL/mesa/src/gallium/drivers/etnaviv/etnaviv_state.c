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
 *    Christian Gmeiner <christian.gmeiner@gmail.com>
 */

#include "etnaviv_state.h"

#include "hw/common.xml.h"

#include "etnaviv_blend.h"
#include "etnaviv_clear_blit.h"
#include "etnaviv_context.h"
#include "etnaviv_format.h"
#include "etnaviv_rasterizer.h"
#include "etnaviv_screen.h"
#include "etnaviv_shader.h"
#include "etnaviv_surface.h"
#include "etnaviv_translate.h"
#include "etnaviv_util.h"
#include "etnaviv_zsa.h"
#include "util/u_framebuffer.h"
#include "util/u_helpers.h"
#include "util/u_inlines.h"
#include "util/u_math.h"
#include "util/u_memory.h"
#include "util/u_upload_mgr.h"

static void
etna_set_stencil_ref(struct pipe_context *pctx, const struct pipe_stencil_ref sr)
{
   struct etna_context *ctx = etna_context(pctx);
   struct compiled_stencil_ref *cs = &ctx->stencil_ref;

   ctx->stencil_ref_s = sr;

   for (unsigned i = 0; i < 2; i++) {
      cs->PE_STENCIL_CONFIG[i] =
         VIVS_PE_STENCIL_CONFIG_REF_FRONT(sr.ref_value[i]);
      cs->PE_STENCIL_CONFIG_EXT[i] =
         VIVS_PE_STENCIL_CONFIG_EXT_REF_BACK(sr.ref_value[!i]);
   }
   ctx->dirty |= ETNA_DIRTY_STENCIL_REF;
}

static void
etna_set_clip_state(struct pipe_context *pctx, const struct pipe_clip_state *pcs)
{
   /* NOOP */
}

static void
etna_set_sample_mask(struct pipe_context *pctx, unsigned sample_mask)
{
   struct etna_context *ctx = etna_context(pctx);

   ctx->sample_mask = sample_mask;
   ctx->dirty |= ETNA_DIRTY_SAMPLE_MASK;
}

static void
etna_set_constant_buffer(struct pipe_context *pctx,
      enum pipe_shader_type shader, uint index, bool take_ownership,
      const struct pipe_constant_buffer *cb)
{
   struct etna_context *ctx = etna_context(pctx);
   struct etna_constbuf_state *so = &ctx->constant_buffer[shader];

   assert(index < ETNA_MAX_CONST_BUF);

   util_copy_constant_buffer(&so->cb[index], cb, take_ownership);

   /* Note that the gallium frontends can unbind constant buffers by
    * passing NULL here. */
   if (unlikely(!cb || (!cb->buffer && !cb->user_buffer))) {
      so->enabled_mask &= ~(1 << index);
      return;
   }

   assert(index != 0 || cb->user_buffer != NULL);

   if (!cb->buffer) {
      struct pipe_constant_buffer *cb = &so->cb[index];
      u_upload_data(pctx->const_uploader, 0, cb->buffer_size, 16, cb->user_buffer, &cb->buffer_offset, &cb->buffer);
   }

   so->enabled_mask |= 1 << index;
   ctx->dirty |= ETNA_DIRTY_CONSTBUF;
}

static void
etna_update_render_surface(struct pipe_context *pctx, struct etna_surface *surf)
{
   struct etna_resource *base = etna_resource(surf->prsc);
   struct etna_resource *to = base, *from = base;
   unsigned level = surf->base.u.tex.level;

   if (base->texture &&
       etna_resource_level_newer(&etna_resource(base->texture)->levels[level],
                                 surf->level))
      from = etna_resource(base->texture);

   if (base->render)
      to = etna_resource(base->render);

   if ((to != from) &&
       etna_resource_level_older(&to->levels[level], &from->levels[level]))
      etna_copy_resource(pctx, &to->base, &from->base, level, level);
}

static void
etna_set_framebuffer_state(struct pipe_context *pctx,
      const struct pipe_framebuffer_state *fb)
{
   struct etna_context *ctx = etna_context(pctx);
   struct etna_screen *screen = ctx->screen;
   struct compiled_framebuffer_state *cs = &ctx->framebuffer;
   int nr_samples_color = -1;
   int nr_samples_depth = -1;
   bool target_16bpp = false;
   bool target_linear = false;

   /* Set up TS as well. Warning: this state is used by both the RS and PE */
   uint32_t ts_mem_config = 0;
   uint32_t pe_mem_config = 0;
   uint32_t pe_logic_op = 0;

   if (fb->nr_cbufs > 0) { /* at least one color buffer? */
      struct etna_surface *cbuf = etna_surface(fb->cbufs[0]);
      struct etna_resource *res = etna_resource(cbuf->base.texture);
      bool color_supertiled = (res->layout & ETNA_LAYOUT_BIT_SUPER) != 0;
      uint32_t fmt = translate_pe_format(cbuf->base.format);

      assert((res->layout & ETNA_LAYOUT_BIT_TILE) ||
             VIV_FEATURE(screen, chipMinorFeatures2, LINEAR_PE));
      etna_update_render_surface(pctx, cbuf);

      if (res->layout == ETNA_LAYOUT_LINEAR)
         target_linear = true;

      if (fmt >= PE_FORMAT_R16F)
          cs->PE_COLOR_FORMAT = VIVS_PE_COLOR_FORMAT_FORMAT_EXT(fmt) |
                                VIVS_PE_COLOR_FORMAT_FORMAT_MASK;
      else
          cs->PE_COLOR_FORMAT = VIVS_PE_COLOR_FORMAT_FORMAT(fmt);

      if (util_format_get_blocksize(cbuf->base.format) <= 2)
         target_16bpp = true;

      cs->PE_COLOR_FORMAT |=
         VIVS_PE_COLOR_FORMAT_COMPONENTS__MASK |
         COND(color_supertiled, VIVS_PE_COLOR_FORMAT_SUPER_TILED);

      nr_samples_color = cbuf->base.texture->nr_samples;
      if (nr_samples_color <= 1)
         cs->PE_COLOR_FORMAT |= VIVS_PE_COLOR_FORMAT_OVERWRITE;

      if (VIV_FEATURE(screen, chipMinorFeatures6, CACHE128B256BPERLINE))
         cs->PE_COLOR_FORMAT |= COND(color_supertiled, VIVS_PE_COLOR_FORMAT_SUPER_TILED_NEW);
      /* VIVS_PE_COLOR_FORMAT_COMPONENTS() and
       * VIVS_PE_COLOR_FORMAT_OVERWRITE comes from blend_state
       * but only if we set the bits above. */
      /* merged with depth_stencil_alpha */
      if ((cbuf->offset & 63) ||
          (((cbuf->level->stride * 4) & 63) && cbuf->level->height > 4)) {
         /* XXX Must make temporary surface here.
          * Need the same mechanism on gc2000 when we want to do mipmap
          * generation by
          * rendering to levels > 1 due to multitiled / tiled conversion. */
         BUG("Alignment error, trying to render to offset %08x with tile "
             "stride %i",
             cbuf->offset, cbuf->level->stride * 4);
      }

      if (screen->specs.halti >= 0 && screen->model != 0x880) {
         /* Rendertargets on GPUs with more than a single pixel pipe must always
          * be multi-tiled, or single-buffer mode must be supported */
         assert(screen->specs.pixel_pipes == 1 ||
                (res->layout & ETNA_LAYOUT_BIT_MULTI) || screen->specs.single_buffer);
         for (int i = 0; i < screen->specs.pixel_pipes; i++) {
            cs->PE_PIPE_COLOR_ADDR[i] = cbuf->reloc[i];
            cs->PE_PIPE_COLOR_ADDR[i].flags = ETNA_RELOC_READ | ETNA_RELOC_WRITE;
         }
      } else {
         cs->PE_COLOR_ADDR = cbuf->reloc[0];
         cs->PE_COLOR_ADDR.flags = ETNA_RELOC_READ | ETNA_RELOC_WRITE;
      }

      cs->PE_COLOR_STRIDE = cbuf->level->stride;

      if (cbuf->level->ts_size) {
         cs->TS_COLOR_CLEAR_VALUE = cbuf->level->clear_value;
         cs->TS_COLOR_CLEAR_VALUE_EXT = cbuf->level->clear_value >> 32;

         cs->TS_COLOR_STATUS_BASE = cbuf->ts_reloc;
         cs->TS_COLOR_STATUS_BASE.flags = ETNA_RELOC_READ | ETNA_RELOC_WRITE;

         cs->TS_COLOR_SURFACE_BASE = cbuf->reloc[0];
         cs->TS_COLOR_SURFACE_BASE.flags = ETNA_RELOC_READ | ETNA_RELOC_WRITE;

         pe_mem_config |= VIVS_PE_MEM_CONFIG_COLOR_TS_MODE(cbuf->level->ts_mode);

         if (cbuf->level->ts_compress_fmt >= 0) {
            /* overwrite bit breaks v1/v2 compression */
            if (!screen->specs.v4_compression)
               cs->PE_COLOR_FORMAT &= ~VIVS_PE_COLOR_FORMAT_OVERWRITE;

            ts_mem_config |=
               VIVS_TS_MEM_CONFIG_COLOR_COMPRESSION |
               VIVS_TS_MEM_CONFIG_COLOR_COMPRESSION_FORMAT(cbuf->level->ts_compress_fmt);
         }
      }

      if (util_format_is_srgb(cbuf->base.format))
         pe_logic_op |= VIVS_PE_LOGIC_OP_SRGB;

      cs->PS_CONTROL = COND(util_format_is_unorm(cbuf->base.format), VIVS_PS_CONTROL_SATURATE_RT0);
      cs->PS_CONTROL_EXT =
         VIVS_PS_CONTROL_EXT_OUTPUT_MODE0(translate_output_mode(cbuf->base.format, screen->specs.halti >= 5));
   } else {
      /* Clearing VIVS_PE_COLOR_FORMAT_COMPONENTS__MASK and
       * VIVS_PE_COLOR_FORMAT_OVERWRITE prevents us from overwriting the
       * color target */
      cs->PE_COLOR_FORMAT = VIVS_PE_COLOR_FORMAT_OVERWRITE;
      cs->PE_COLOR_STRIDE = 0;
      cs->TS_COLOR_STATUS_BASE.bo = NULL;
      cs->TS_COLOR_SURFACE_BASE.bo = NULL;

      cs->PE_COLOR_ADDR = screen->dummy_rt_reloc;
      for (int i = 0; i < screen->specs.pixel_pipes; i++)
         cs->PE_PIPE_COLOR_ADDR[i] = screen->dummy_rt_reloc;
   }

   if (fb->zsbuf != NULL) {
      struct etna_surface *zsbuf = etna_surface(fb->zsbuf);
      struct etna_resource *res = etna_resource(zsbuf->base.texture);

      etna_update_render_surface(pctx, zsbuf);

      assert(res->layout &ETNA_LAYOUT_BIT_TILE); /* Cannot render to linear surfaces */

      uint32_t depth_format = translate_depth_format(zsbuf->base.format);
      unsigned depth_bits =
         depth_format == VIVS_PE_DEPTH_CONFIG_DEPTH_FORMAT_D16 ? 16 : 24;
      bool depth_supertiled = (res->layout & ETNA_LAYOUT_BIT_SUPER) != 0;

      if (depth_bits == 16)
         target_16bpp = true;

      cs->PE_DEPTH_CONFIG =
         depth_format |
         COND(depth_supertiled, VIVS_PE_DEPTH_CONFIG_SUPER_TILED) |
         VIVS_PE_DEPTH_CONFIG_DEPTH_MODE_Z |
         VIVS_PE_DEPTH_CONFIG_UNK18; /* something to do with clipping? */
      /* VIVS_PE_DEPTH_CONFIG_ONLY_DEPTH */
      /* merged with depth_stencil_alpha */

      if (screen->specs.halti >= 0 && screen->model != 0x880) {
         for (int i = 0; i < screen->specs.pixel_pipes; i++) {
            cs->PE_PIPE_DEPTH_ADDR[i] = zsbuf->reloc[i];
            cs->PE_PIPE_DEPTH_ADDR[i].flags = ETNA_RELOC_READ | ETNA_RELOC_WRITE;
         }
      } else {
         cs->PE_DEPTH_ADDR = zsbuf->reloc[0];
         cs->PE_DEPTH_ADDR.flags = ETNA_RELOC_READ | ETNA_RELOC_WRITE;
      }

      cs->PE_DEPTH_STRIDE = zsbuf->level->stride;
      cs->PE_HDEPTH_CONTROL = VIVS_PE_HDEPTH_CONTROL_FORMAT_DISABLED;
      cs->PE_DEPTH_NORMALIZE = fui(exp2f(depth_bits) - 1.0f);

      if (zsbuf->level->ts_size) {
         cs->TS_DEPTH_CLEAR_VALUE = zsbuf->level->clear_value;

         cs->TS_DEPTH_STATUS_BASE = zsbuf->ts_reloc;
         cs->TS_DEPTH_STATUS_BASE.flags = ETNA_RELOC_READ | ETNA_RELOC_WRITE;

         cs->TS_DEPTH_SURFACE_BASE = zsbuf->reloc[0];
         cs->TS_DEPTH_SURFACE_BASE.flags = ETNA_RELOC_READ | ETNA_RELOC_WRITE;

         pe_mem_config |= VIVS_PE_MEM_CONFIG_DEPTH_TS_MODE(zsbuf->level->ts_mode);

         if (zsbuf->level->ts_compress_fmt >= 0) {
            ts_mem_config |=
               VIVS_TS_MEM_CONFIG_DEPTH_COMPRESSION |
               COND(zsbuf->level->ts_compress_fmt == COMPRESSION_FORMAT_D24S8,
                    VIVS_TS_MEM_CONFIG_STENCIL_ENABLE);
         }
      }

      ts_mem_config |= COND(depth_bits == 16, VIVS_TS_MEM_CONFIG_DEPTH_16BPP);

      nr_samples_depth = zsbuf->base.texture->nr_samples;
   } else {
      cs->PE_DEPTH_CONFIG = VIVS_PE_DEPTH_CONFIG_DEPTH_MODE_NONE;
      cs->PE_DEPTH_ADDR.bo = NULL;
      cs->PE_DEPTH_STRIDE = 0;
      cs->TS_DEPTH_STATUS_BASE.bo = NULL;
      cs->TS_DEPTH_SURFACE_BASE.bo = NULL;

      for (int i = 0; i < ETNA_MAX_PIXELPIPES; i++)
         cs->PE_PIPE_DEPTH_ADDR[i].bo = NULL;
   }

   /* MSAA setup */
   if (nr_samples_depth != -1 && nr_samples_color != -1 &&
       nr_samples_depth != nr_samples_color) {
      BUG("Number of samples in color and depth texture must match (%i and %i respectively)",
          nr_samples_color, nr_samples_depth);
   }

   switch (MAX2(nr_samples_depth, nr_samples_color)) {
   case 0:
   case 1: /* Are 0 and 1 samples allowed? */
      cs->GL_MULTI_SAMPLE_CONFIG =
         VIVS_GL_MULTI_SAMPLE_CONFIG_MSAA_SAMPLES_NONE;
      cs->msaa_mode = false;
      break;
   case 2:
      cs->GL_MULTI_SAMPLE_CONFIG = VIVS_GL_MULTI_SAMPLE_CONFIG_MSAA_SAMPLES_2X;
      cs->msaa_mode = true; /* Add input to PS */
      cs->RA_MULTISAMPLE_UNK00E04 = 0x0;
      cs->RA_MULTISAMPLE_UNK00E10[0] = 0x0000aa22;
      cs->RA_CENTROID_TABLE[0] = 0x66aa2288;
      cs->RA_CENTROID_TABLE[1] = 0x88558800;
      cs->RA_CENTROID_TABLE[2] = 0x88881100;
      cs->RA_CENTROID_TABLE[3] = 0x33888800;
      break;
   case 4:
      cs->GL_MULTI_SAMPLE_CONFIG = VIVS_GL_MULTI_SAMPLE_CONFIG_MSAA_SAMPLES_4X;
      cs->msaa_mode = true; /* Add input to PS */
      cs->RA_MULTISAMPLE_UNK00E04 = 0x0;
      cs->RA_MULTISAMPLE_UNK00E10[0] = 0xeaa26e26;
      cs->RA_MULTISAMPLE_UNK00E10[1] = 0xe6ae622a;
      cs->RA_MULTISAMPLE_UNK00E10[2] = 0xaaa22a22;
      cs->RA_CENTROID_TABLE[0] = 0x4a6e2688;
      cs->RA_CENTROID_TABLE[1] = 0x888888a2;
      cs->RA_CENTROID_TABLE[2] = 0x888888ea;
      cs->RA_CENTROID_TABLE[3] = 0x888888c6;
      cs->RA_CENTROID_TABLE[4] = 0x46622a88;
      cs->RA_CENTROID_TABLE[5] = 0x888888ae;
      cs->RA_CENTROID_TABLE[6] = 0x888888e6;
      cs->RA_CENTROID_TABLE[7] = 0x888888ca;
      cs->RA_CENTROID_TABLE[8] = 0x262a2288;
      cs->RA_CENTROID_TABLE[9] = 0x886688a2;
      cs->RA_CENTROID_TABLE[10] = 0x888866aa;
      cs->RA_CENTROID_TABLE[11] = 0x668888a6;
      if (VIV_FEATURE(screen, chipMinorFeatures4, SMALL_MSAA))
         pe_logic_op |= VIVS_PE_LOGIC_OP_UNK24(0x5);
      break;
   }

   cs->TS_MEM_CONFIG = ts_mem_config;
   cs->PE_MEM_CONFIG = pe_mem_config;

   /* Single buffer setup. There is only one switch for this, not a separate
    * one per color buffer / depth buffer. To keep the logic simple always use
    * single buffer when this feature is available.
    */
   if (unlikely(target_linear))
      pe_logic_op |= VIVS_PE_LOGIC_OP_SINGLE_BUFFER(1);
   else if (screen->specs.single_buffer)
      pe_logic_op |= VIVS_PE_LOGIC_OP_SINGLE_BUFFER(target_16bpp ? 3 : 2);
   cs->PE_LOGIC_OP = pe_logic_op;

   /* keep copy of original structure */
   util_copy_framebuffer_state(&ctx->framebuffer_s, fb);
   ctx->dirty |= ETNA_DIRTY_FRAMEBUFFER | ETNA_DIRTY_DERIVE_TS;
}

static void
etna_set_polygon_stipple(struct pipe_context *pctx,
      const struct pipe_poly_stipple *stipple)
{
   /* NOP */
}

static void
etna_set_scissor_states(struct pipe_context *pctx, unsigned start_slot,
      unsigned num_scissors, const struct pipe_scissor_state *ss)
{
   struct etna_context *ctx = etna_context(pctx);
   assert(ss->minx <= ss->maxx);
   assert(ss->miny <= ss->maxy);

   ctx->scissor = *ss;
   ctx->dirty |= ETNA_DIRTY_SCISSOR;
}

static void
etna_set_viewport_states(struct pipe_context *pctx, unsigned start_slot,
      unsigned num_scissors, const struct pipe_viewport_state *vs)
{
   struct etna_context *ctx = etna_context(pctx);
   struct compiled_viewport_state *cs = &ctx->viewport;

   ctx->viewport_s = *vs;
   /**
    * For Vivante GPU, viewport z transformation is 0..1 to 0..1 instead of
    * -1..1 to 0..1.
    * scaling and translation to 0..1 already happened, so remove that
    *
    * z' = (z * 2 - 1) * scale + translate
    *    = z * (2 * scale) + (translate - scale)
    *
    * scale' = 2 * scale
    * translate' = translate - scale
    */

   /* must be fixp as v4 state deltas assume it is */
   cs->PA_VIEWPORT_SCALE_X = etna_f32_to_fixp16(vs->scale[0]);
   cs->PA_VIEWPORT_SCALE_Y = etna_f32_to_fixp16(vs->scale[1]);
   cs->PA_VIEWPORT_SCALE_Z = fui(vs->scale[2] * 2.0f);
   cs->PA_VIEWPORT_OFFSET_X = etna_f32_to_fixp16(vs->translate[0]);
   cs->PA_VIEWPORT_OFFSET_Y = etna_f32_to_fixp16(vs->translate[1]);
   cs->PA_VIEWPORT_OFFSET_Z = fui(vs->translate[2] - vs->scale[2]);

   /* Compute scissor rectangle (fixp) from viewport.
    * Make sure left is always < right and top always < bottom.
    */
   cs->SE_SCISSOR_LEFT = MAX2(vs->translate[0] - fabsf(vs->scale[0]), 0.0f);
   cs->SE_SCISSOR_TOP = MAX2(vs->translate[1] - fabsf(vs->scale[1]), 0.0f);
   cs->SE_SCISSOR_RIGHT = ceilf(MAX2(vs->translate[0] + fabsf(vs->scale[0]), 0.0f));
   cs->SE_SCISSOR_BOTTOM = ceilf(MAX2(vs->translate[1] + fabsf(vs->scale[1]), 0.0f));

   cs->PE_DEPTH_NEAR = fui(0.0); /* not affected if depth mode is Z (as in GL) */
   cs->PE_DEPTH_FAR = fui(1.0);
   ctx->dirty |= ETNA_DIRTY_VIEWPORT;
}

static void
etna_set_vertex_buffers(struct pipe_context *pctx, unsigned num_buffers,
      unsigned unbind_num_trailing_slots, bool take_ownership,
      const struct pipe_vertex_buffer *vb)
{
   struct etna_context *ctx = etna_context(pctx);
   struct etna_vertexbuf_state *so = &ctx->vertex_buffer;

   util_set_vertex_buffers_mask(so->vb, &so->enabled_mask, vb,
                                num_buffers, unbind_num_trailing_slots,
                                take_ownership);
   so->count = util_last_bit(so->enabled_mask);

   for (unsigned idx = 0; idx < num_buffers; ++idx) {
      struct compiled_set_vertex_buffer *cs = &so->cvb[idx];
      struct pipe_vertex_buffer *vbi = &so->vb[idx];

      assert(!vbi->is_user_buffer); /* XXX support user_buffer using
                                       etna_usermem_map */

      if (vbi->buffer.resource) { /* GPU buffer */
         cs->FE_VERTEX_STREAM_BASE_ADDR.bo = etna_resource(vbi->buffer.resource)->bo;
         cs->FE_VERTEX_STREAM_BASE_ADDR.offset = vbi->buffer_offset;
         cs->FE_VERTEX_STREAM_BASE_ADDR.flags = ETNA_RELOC_READ;
      } else {
         cs->FE_VERTEX_STREAM_BASE_ADDR.bo = NULL;
      }
   }

   ctx->dirty |= ETNA_DIRTY_VERTEX_BUFFERS;
}

static void
etna_blend_state_bind(struct pipe_context *pctx, void *bs)
{
   struct etna_context *ctx = etna_context(pctx);

   ctx->blend = bs;
   ctx->dirty |= ETNA_DIRTY_BLEND;
}

static void
etna_blend_state_delete(struct pipe_context *pctx, void *bs)
{
   FREE(bs);
}

static void
etna_rasterizer_state_bind(struct pipe_context *pctx, void *rs)
{
   struct etna_context *ctx = etna_context(pctx);

   ctx->rasterizer = rs;
   ctx->dirty |= ETNA_DIRTY_RASTERIZER;
}

static void
etna_rasterizer_state_delete(struct pipe_context *pctx, void *rs)
{
   FREE(rs);
}

static void
etna_zsa_state_bind(struct pipe_context *pctx, void *zs)
{
   struct etna_context *ctx = etna_context(pctx);

   ctx->zsa = zs;
   ctx->dirty |= ETNA_DIRTY_ZSA;
}

static void
etna_zsa_state_delete(struct pipe_context *pctx, void *zs)
{
   FREE(zs);
}

/** Create vertex element states, which define a layout for fetching
 * vertices for rendering.
 */
static void *
etna_vertex_elements_state_create(struct pipe_context *pctx,
      unsigned num_elements, const struct pipe_vertex_element *elements)
{
   struct etna_context *ctx = etna_context(pctx);
   struct etna_screen *screen = ctx->screen;
   struct compiled_vertex_elements_state *cs = CALLOC_STRUCT(compiled_vertex_elements_state);

   if (!cs)
      return NULL;

   if (num_elements > screen->specs.vertex_max_elements) {
      BUG("number of elements (%u) exceeds chip maximum (%u)", num_elements,
          screen->specs.vertex_max_elements);
      FREE(cs);
      return NULL;
   }

   /* XXX could minimize number of consecutive stretches here by sorting, and
    * permuting the inputs in shader or does Mesa do this already? */

   cs->num_elements = num_elements;

   unsigned start_offset = 0; /* start of current consecutive stretch */
   bool nonconsecutive = true; /* previous value of nonconsecutive */
   uint32_t buffer_mask = 0; /* mask of buffer_idx already seen */

   for (unsigned idx = 0; idx < num_elements; ++idx) {
      unsigned buffer_idx = elements[idx].vertex_buffer_index;
      unsigned element_size = util_format_get_blocksize(elements[idx].src_format);
      unsigned end_offset = elements[idx].src_offset + element_size;
      uint32_t format_type, normalize;

      if (nonconsecutive)
         start_offset = elements[idx].src_offset;

      /* guaranteed by PIPE_CAP_MAX_VERTEX_BUFFERS */
      assert(buffer_idx < screen->specs.stream_count);

      /* maximum vertex size is 256 bytes */
      assert(element_size != 0 && (end_offset - start_offset) < 256);

      /* check whether next element is consecutive to this one */
      nonconsecutive = (idx == (num_elements - 1)) ||
                       elements[idx + 1].vertex_buffer_index != buffer_idx ||
                       end_offset != elements[idx + 1].src_offset;

      format_type = translate_vertex_format_type(elements[idx].src_format);
      normalize = translate_vertex_format_normalize(elements[idx].src_format);

      assert(format_type != ETNA_NO_MATCH);
      assert(normalize != ETNA_NO_MATCH);

      if (screen->specs.halti < 5) {
         cs->FE_VERTEX_ELEMENT_CONFIG[idx] =
            COND(nonconsecutive, VIVS_FE_VERTEX_ELEMENT_CONFIG_NONCONSECUTIVE) |
            format_type |
            VIVS_FE_VERTEX_ELEMENT_CONFIG_NUM(util_format_get_nr_components(elements[idx].src_format)) |
            normalize | VIVS_FE_VERTEX_ELEMENT_CONFIG_ENDIAN(ENDIAN_MODE_NO_SWAP) |
            VIVS_FE_VERTEX_ELEMENT_CONFIG_STREAM(buffer_idx) |
            VIVS_FE_VERTEX_ELEMENT_CONFIG_START(elements[idx].src_offset) |
            VIVS_FE_VERTEX_ELEMENT_CONFIG_END(end_offset - start_offset);
      } else { /* HALTI5 spread vertex attrib config over two registers */
         cs->NFE_GENERIC_ATTRIB_CONFIG0[idx] =
            format_type |
            VIVS_NFE_GENERIC_ATTRIB_CONFIG0_NUM(util_format_get_nr_components(elements[idx].src_format)) |
            normalize | VIVS_NFE_GENERIC_ATTRIB_CONFIG0_ENDIAN(ENDIAN_MODE_NO_SWAP) |
            VIVS_NFE_GENERIC_ATTRIB_CONFIG0_STREAM(buffer_idx) |
            VIVS_NFE_GENERIC_ATTRIB_CONFIG0_START(elements[idx].src_offset);
         cs->NFE_GENERIC_ATTRIB_CONFIG1[idx] =
            COND(nonconsecutive, VIVS_NFE_GENERIC_ATTRIB_CONFIG1_NONCONSECUTIVE) |
            VIVS_NFE_GENERIC_ATTRIB_CONFIG1_END(end_offset - start_offset);
      }
      cs->FE_VERTEX_STREAM_CONTROL[buffer_idx] =
            FE_VERTEX_STREAM_CONTROL_VERTEX_STRIDE(elements[idx].src_stride);

      if (util_format_is_pure_integer(elements[idx].src_format))
         cs->NFE_GENERIC_ATTRIB_SCALE[idx] = 1;
      else
         cs->NFE_GENERIC_ATTRIB_SCALE[idx] = fui(1.0f);

      /* instance_divisor is part of elements state but should be the same for all buffers */
      if (buffer_mask & 1 << buffer_idx)
         assert(cs->NFE_VERTEX_STREAMS_VERTEX_DIVISOR[buffer_idx] == elements[idx].instance_divisor);
      else
         cs->NFE_VERTEX_STREAMS_VERTEX_DIVISOR[buffer_idx] = elements[idx].instance_divisor;

      buffer_mask |= 1 << buffer_idx;
      cs->num_buffers = MAX2(cs->num_buffers, buffer_idx + 1);
   }

   return cs;
}

static void
etna_vertex_elements_state_delete(struct pipe_context *pctx, void *ve)
{
   FREE(ve);
}

static void
etna_vertex_elements_state_bind(struct pipe_context *pctx, void *ve)
{
   struct etna_context *ctx = etna_context(pctx);

   ctx->vertex_elements = ve;
   ctx->dirty |= ETNA_DIRTY_VERTEX_ELEMENTS;
}

static void
etna_set_stream_output_targets(struct pipe_context *pctx,
      unsigned num_targets, struct pipe_stream_output_target **targets,
      const unsigned *offsets)
{
   /* stub */
}

static bool
etna_update_ts_config(struct etna_context *ctx)
{
   uint32_t new_ts_config = ctx->framebuffer.TS_MEM_CONFIG;

   if (ctx->framebuffer_s.nr_cbufs > 0) {
      struct etna_surface *c_surf = etna_surface(ctx->framebuffer_s.cbufs[0]);

      if (etna_resource_level_ts_valid(c_surf->level)) {
         new_ts_config |= VIVS_TS_MEM_CONFIG_COLOR_FAST_CLEAR;
      } else {
         new_ts_config &= ~VIVS_TS_MEM_CONFIG_COLOR_FAST_CLEAR;
      }
   }

   if (ctx->framebuffer_s.zsbuf) {
      struct etna_surface *zs_surf = etna_surface(ctx->framebuffer_s.zsbuf);

      if (etna_resource_level_ts_valid(zs_surf->level)) {
         new_ts_config |= VIVS_TS_MEM_CONFIG_DEPTH_FAST_CLEAR;
      } else {
         new_ts_config &= ~VIVS_TS_MEM_CONFIG_DEPTH_FAST_CLEAR;
      }
   }

   if (new_ts_config != ctx->framebuffer.TS_MEM_CONFIG ||
       (ctx->dirty & ETNA_DIRTY_FRAMEBUFFER)) {
      ctx->framebuffer.TS_MEM_CONFIG = new_ts_config;
      ctx->dirty |= ETNA_DIRTY_TS;
   }

   ctx->dirty &= ~ETNA_DIRTY_DERIVE_TS;

   return true;
}

static bool
etna_update_clipping(struct etna_context *ctx)
{
   const struct etna_rasterizer_state *rasterizer = etna_rasterizer_state(ctx->rasterizer);
   const struct pipe_framebuffer_state *fb = &ctx->framebuffer_s;

   /* clip framebuffer against viewport */
   uint32_t scissor_left = ctx->viewport.SE_SCISSOR_LEFT;
   uint32_t scissor_top = ctx->viewport.SE_SCISSOR_TOP;
   uint32_t scissor_right = MIN2(fb->width, ctx->viewport.SE_SCISSOR_RIGHT);
   uint32_t scissor_bottom = MIN2(fb->height, ctx->viewport.SE_SCISSOR_BOTTOM);

   /* clip against scissor */
   if (rasterizer->scissor) {
      scissor_left = MAX2(ctx->scissor.minx, scissor_left);
      scissor_top = MAX2(ctx->scissor.miny, scissor_top);
      scissor_right = MIN2(ctx->scissor.maxx, scissor_right);
      scissor_bottom = MIN2(ctx->scissor.maxy, scissor_bottom);
   }

   ctx->clipping.minx = scissor_left;
   ctx->clipping.miny = scissor_top;
   ctx->clipping.maxx = scissor_right;
   ctx->clipping.maxy = scissor_bottom;

   ctx->dirty |= ETNA_DIRTY_SCISSOR_CLIP;

   return true;
}

static bool
etna_update_zsa(struct etna_context *ctx)
{
   struct compiled_shader_state *shader_state = &ctx->shader_state;
   struct pipe_depth_stencil_alpha_state *zsa_state = ctx->zsa;
   struct etna_zsa_state *zsa = etna_zsa_state(zsa_state);
   struct etna_screen *screen = ctx->screen;
   uint32_t new_pe_depth, new_ra_depth;
   bool early_z_allowed = !VIV_FEATURE(screen, chipFeatures, NO_EARLY_Z);
   bool late_zs = false, early_zs = false,
        late_z_test = false, early_z_test = false;

   /* Linear PE breaks the combination of early test with late write, as it
    * seems RA and PE disagree about the buffer layout in this mode. Fall back
    * to late Z always even though early Z write might be possible, as we don't
    * know if any other draws to the same surface require late Z write.
    */
   if (ctx->framebuffer_s.nr_cbufs > 0) {
      struct etna_surface *cbuf = etna_surface(ctx->framebuffer_s.cbufs[0]);
      struct etna_resource *res = etna_resource(cbuf->base.texture);

      if (res->layout == ETNA_LAYOUT_LINEAR)
         early_z_allowed = false;
   }

   if (zsa->z_write_enabled || zsa->stencil_enabled) {
      if (VIV_FEATURE(screen, chipMinorFeatures5, RA_WRITE_DEPTH) &&
          early_z_allowed &&
          !zsa_state->alpha_enabled &&
          !shader_state->writes_z &&
          !shader_state->uses_discard)
         early_zs = true;
      else
         late_zs = true;
   }

   if (zsa->z_test_enabled) {
      if (early_z_allowed &&
          (!zsa->stencil_modified || early_zs) &&
          !shader_state->writes_z)
         early_z_test = true;
      else
         late_z_test = true;
   }

   new_pe_depth = VIVS_PE_DEPTH_CONFIG_DEPTH_FUNC(zsa->z_test_enabled ?
                     /* compare funcs have 1 to 1 mapping */
                     zsa_state->depth_func : PIPE_FUNC_ALWAYS) |
                  COND(zsa->z_write_enabled, VIVS_PE_DEPTH_CONFIG_WRITE_ENABLE) |
                  COND(early_z_test, VIVS_PE_DEPTH_CONFIG_EARLY_Z) |
                  COND(!late_zs && !late_z_test,
                       VIVS_PE_DEPTH_CONFIG_DISABLE_ZS);

   /* blob sets this to 0x40000031 on GC7000, seems to make no difference,
    * but keep it in mind if depth behaves strangely. */
   new_ra_depth = 0x0000030 |
                  COND(early_z_test, VIVS_RA_EARLY_DEPTH_TEST_ENABLE);

   if (VIV_FEATURE(screen, chipMinorFeatures5, RA_WRITE_DEPTH)) {
      if (!early_zs)
         new_ra_depth |= VIVS_RA_EARLY_DEPTH_WRITE_DISABLE;
      /* The new early hierarchical test seems to only work properly if depth
       * is also written from the early stage.
       */
      if (late_z_test || (early_z_test && late_zs))
         new_ra_depth |= VIVS_RA_EARLY_DEPTH_HDEPTH_DISABLE;

      if (ctx->framebuffer_s.nr_cbufs > 0) {
         struct pipe_resource *res = ctx->framebuffer_s.cbufs[0]->texture;

         if ((late_z_test || late_zs) && res->nr_samples > 1)
            new_ra_depth |= VIVS_RA_EARLY_DEPTH_LATE_DEPTH_MSAA;
      }
   }

   if (new_pe_depth != zsa->PE_DEPTH_CONFIG ||
       new_ra_depth != zsa->RA_DEPTH_CONFIG)
      ctx->dirty |= ETNA_DIRTY_ZSA;

   zsa->PE_DEPTH_CONFIG = new_pe_depth;
   zsa->RA_DEPTH_CONFIG = new_ra_depth;

   return true;
}

static bool
etna_record_flush_resources(struct etna_context *ctx)
{
   struct pipe_framebuffer_state *fb = &ctx->framebuffer_s;

   if (fb->nr_cbufs > 0) {
      struct etna_surface *surf = etna_surface(fb->cbufs[0]);

      if (!etna_resource(surf->prsc)->explicit_flush)
         etna_context_add_flush_resource(ctx, surf->prsc);
   }

   return true;
}

struct etna_state_updater {
   bool (*update)(struct etna_context *ctx);
   uint32_t dirty;
};

static const struct etna_state_updater etna_state_updates[] = {
   {
      etna_shader_update_vertex, ETNA_DIRTY_SHADER | ETNA_DIRTY_VERTEX_ELEMENTS,
   },
   {
      etna_shader_link, ETNA_DIRTY_SHADER,
   },
   {
      etna_update_blend, ETNA_DIRTY_BLEND | ETNA_DIRTY_FRAMEBUFFER
   },
   {
      etna_update_blend_color, ETNA_DIRTY_BLEND_COLOR | ETNA_DIRTY_FRAMEBUFFER,
   },
   {
      etna_update_ts_config, ETNA_DIRTY_DERIVE_TS,
   },
   {
      etna_update_clipping, ETNA_DIRTY_SCISSOR | ETNA_DIRTY_FRAMEBUFFER |
                            ETNA_DIRTY_RASTERIZER | ETNA_DIRTY_VIEWPORT,
   },
   {
      etna_update_zsa, ETNA_DIRTY_ZSA | ETNA_DIRTY_SHADER |
                       ETNA_DIRTY_FRAMEBUFFER,
   },
   {
      etna_record_flush_resources, ETNA_DIRTY_FRAMEBUFFER,
   }
};

bool
etna_state_update(struct etna_context *ctx)
{
   for (unsigned int i = 0; i < ARRAY_SIZE(etna_state_updates); i++)
      if (ctx->dirty & etna_state_updates[i].dirty)
         if (!etna_state_updates[i].update(ctx))
            return false;

   return true;
}

void
etna_state_init(struct pipe_context *pctx)
{
   pctx->set_blend_color = etna_set_blend_color;
   pctx->set_stencil_ref = etna_set_stencil_ref;
   pctx->set_clip_state = etna_set_clip_state;
   pctx->set_sample_mask = etna_set_sample_mask;
   pctx->set_constant_buffer = etna_set_constant_buffer;
   pctx->set_framebuffer_state = etna_set_framebuffer_state;
   pctx->set_polygon_stipple = etna_set_polygon_stipple;
   pctx->set_scissor_states = etna_set_scissor_states;
   pctx->set_viewport_states = etna_set_viewport_states;

   pctx->set_vertex_buffers = etna_set_vertex_buffers;

   pctx->bind_blend_state = etna_blend_state_bind;
   pctx->delete_blend_state = etna_blend_state_delete;

   pctx->bind_rasterizer_state = etna_rasterizer_state_bind;
   pctx->delete_rasterizer_state = etna_rasterizer_state_delete;

   pctx->bind_depth_stencil_alpha_state = etna_zsa_state_bind;
   pctx->delete_depth_stencil_alpha_state = etna_zsa_state_delete;

   pctx->create_vertex_elements_state = etna_vertex_elements_state_create;
   pctx->delete_vertex_elements_state = etna_vertex_elements_state_delete;
   pctx->bind_vertex_elements_state = etna_vertex_elements_state_bind;

   pctx->set_stream_output_targets = etna_set_stream_output_targets;
}
