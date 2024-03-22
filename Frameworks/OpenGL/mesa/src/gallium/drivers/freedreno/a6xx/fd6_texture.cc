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
#include "util/format/u_format.h"
#include "util/hash_table.h"
#include "util/u_inlines.h"
#include "util/u_memory.h"
#include "util/u_string.h"

#include "freedreno_dev_info.h"
#include "fd6_emit.h"
#include "fd6_resource.h"
#include "fd6_screen.h"
#include "fd6_texture.h"

static void fd6_texture_state_destroy(struct fd6_texture_state *state);

static void
remove_tex_entry(struct fd6_context *fd6_ctx, struct hash_entry *entry)
{
   struct fd6_texture_state *tex = (struct fd6_texture_state *)entry->data;
   _mesa_hash_table_remove(fd6_ctx->tex_cache, entry);
   fd6_texture_state_destroy(tex);
}

static enum a6xx_tex_clamp
tex_clamp(unsigned wrap, bool *needs_border)
{
   switch (wrap) {
   case PIPE_TEX_WRAP_REPEAT:
      return A6XX_TEX_REPEAT;
   case PIPE_TEX_WRAP_CLAMP_TO_EDGE:
      return A6XX_TEX_CLAMP_TO_EDGE;
   case PIPE_TEX_WRAP_CLAMP_TO_BORDER:
      *needs_border = true;
      return A6XX_TEX_CLAMP_TO_BORDER;
   case PIPE_TEX_WRAP_MIRROR_CLAMP_TO_EDGE:
      /* only works for PoT.. need to emulate otherwise! */
      return A6XX_TEX_MIRROR_CLAMP;
   case PIPE_TEX_WRAP_MIRROR_REPEAT:
      return A6XX_TEX_MIRROR_REPEAT;
   case PIPE_TEX_WRAP_MIRROR_CLAMP:
   case PIPE_TEX_WRAP_MIRROR_CLAMP_TO_BORDER:
      /* these two we could perhaps emulate, but we currently
       * just don't advertise PIPE_CAP_TEXTURE_MIRROR_CLAMP
       */
   default:
      DBG("invalid wrap: %u", wrap);
      return (enum a6xx_tex_clamp)0;
   }
}

static enum a6xx_tex_filter
tex_filter(unsigned filter, bool aniso)
{
   switch (filter) {
   case PIPE_TEX_FILTER_NEAREST:
      return A6XX_TEX_NEAREST;
   case PIPE_TEX_FILTER_LINEAR:
      return aniso ? A6XX_TEX_ANISO : A6XX_TEX_LINEAR;
   default:
      DBG("invalid filter: %u", filter);
      return (enum a6xx_tex_filter)0;
   }
}

static enum a6xx_reduction_mode
reduction_mode(unsigned reduction_mode)
{
   switch (reduction_mode) {
   default:
   case PIPE_TEX_REDUCTION_WEIGHTED_AVERAGE:
      return A6XX_REDUCTION_MODE_AVERAGE;
   case PIPE_TEX_REDUCTION_MIN:
      return A6XX_REDUCTION_MODE_MIN;
   case PIPE_TEX_REDUCTION_MAX:
      return A6XX_REDUCTION_MODE_MAX;
   }
}

static void
setup_border_color(struct fd_screen *screen,
                   const struct pipe_sampler_state *sampler,
                   struct fd6_bcolor_entry *e)
{
   STATIC_ASSERT(sizeof(struct fd6_bcolor_entry) == FD6_BORDER_COLOR_SIZE);
   const bool has_z24uint_s8uint = screen->info->a6xx.has_z24uint_s8uint;
   const union pipe_color_union *bc = &sampler->border_color;

   enum pipe_format format = sampler->border_color_format;
   const struct util_format_description *desc =
      util_format_description(format);

   e->rgb565 = 0;
   e->rgb5a1 = 0;
   e->rgba4 = 0;
   e->rgb10a2 = 0;
   e->z24 = 0;

   unsigned char swiz[4];

   fdl6_format_swiz(format, false, swiz);

   for (unsigned j = 0; j < 4; j++) {
      int c = swiz[j];
      int cd = c;

      /*
       * HACK: for PIPE_FORMAT_X24S8_UINT we end up w/ the
       * stencil border color value in bc->ui[0] but according
       * to desc->swizzle and desc->channel, the .x/.w component
       * is NONE and the stencil value is in the y component.
       * Meanwhile the hardware wants this in the .x component
       * for x24s8 and x32_s8x24, or the .y component for x24s8 with the
       * special Z24UINT_S8UINT format.
       */
      if ((format == PIPE_FORMAT_X24S8_UINT) ||
          (format == PIPE_FORMAT_X32_S8X24_UINT)) {
         if (j == 0) {
            c = 1;
            cd = (format == PIPE_FORMAT_X24S8_UINT && has_z24uint_s8uint) ? 1 : 0;
         } else {
            continue;
         }
      }

      if (c >= 4)
         continue;

      if (desc->channel[c].pure_integer) {
         uint16_t clamped;
         switch (desc->channel[c].size) {
         case 2:
            assert(desc->channel[c].type == UTIL_FORMAT_TYPE_UNSIGNED);
            clamped = CLAMP(bc->ui[j], 0, 0x3);
            break;
         case 8:
            if (desc->channel[c].type == UTIL_FORMAT_TYPE_SIGNED)
               clamped = CLAMP(bc->i[j], -128, 127);
            else
               clamped = CLAMP(bc->ui[j], 0, 255);
            break;
         case 10:
            assert(desc->channel[c].type == UTIL_FORMAT_TYPE_UNSIGNED);
            clamped = CLAMP(bc->ui[j], 0, 0x3ff);
            break;
         case 16:
            if (desc->channel[c].type == UTIL_FORMAT_TYPE_SIGNED)
               clamped = CLAMP(bc->i[j], -32768, 32767);
            else
               clamped = CLAMP(bc->ui[j], 0, 65535);
            break;
         default:
            unreachable("Unexpected bit size");
         case 32:
            clamped = 0;
            break;
         }
         e->fp32[cd] = bc->ui[j];
         e->fp16[cd] = clamped;
      } else {
         float f = bc->f[j];
         float f_u = CLAMP(f, 0, 1);
         float f_s = CLAMP(f, -1, 1);

         e->fp32[c] = fui(f);
         e->fp16[c] = _mesa_float_to_half(f);
         e->srgb[c] = _mesa_float_to_half(f_u);
         e->ui16[c] = f_u * 0xffff;
         e->si16[c] = f_s * 0x7fff;
         e->ui8[c] = f_u * 0xff;
         e->si8[c] = f_s * 0x7f;

         if (c == 1)
            e->rgb565 |= (int)(f_u * 0x3f) << 5;
         else if (c < 3)
            e->rgb565 |= (int)(f_u * 0x1f) << (c ? 11 : 0);
         if (c == 3)
            e->rgb5a1 |= (f_u > 0.5f) ? 0x8000 : 0;
         else
            e->rgb5a1 |= (int)(f_u * 0x1f) << (c * 5);
         if (c == 3)
            e->rgb10a2 |= (int)(f_u * 0x3) << 30;
         else
            e->rgb10a2 |= (int)(f_u * 0x3ff) << (c * 10);
         e->rgba4 |= (int)(f_u * 0xf) << (c * 4);
         if (c == 0)
            e->z24 = f_u * 0xffffff;
      }
   }
}

static uint32_t
bcolor_key_hash(const void *_key)
{
   const struct fd6_bcolor_entry *key = (const struct fd6_bcolor_entry *)_key;
   return XXH32(key, sizeof(*key), 0);
}

static bool
bcolor_key_equals(const void *_a, const void *_b)
{
   const struct fd6_bcolor_entry *a = (const struct fd6_bcolor_entry *)_a;
   const struct fd6_bcolor_entry *b = (const struct fd6_bcolor_entry *)_b;
   return memcmp(a, b, sizeof(struct fd6_bcolor_entry)) == 0;
}

static unsigned
get_bcolor_offset(struct fd_context *ctx, const struct pipe_sampler_state *sampler)
{
   struct fd6_context *fd6_ctx = fd6_context(ctx);
   struct fd6_bcolor_entry *entries =
         (struct fd6_bcolor_entry *)fd_bo_map(fd6_ctx->bcolor_mem);
   struct fd6_bcolor_entry key = {};

   setup_border_color(ctx->screen, sampler, &key);

   uint32_t hash = bcolor_key_hash(&key);

   struct hash_entry *entry =
      _mesa_hash_table_search_pre_hashed(fd6_ctx->bcolor_cache, hash, &key);

   if (entry) {
      return (unsigned)(uintptr_t)entry->data;
   }

   unsigned idx = fd6_ctx->bcolor_cache->entries;
   if (idx >= FD6_MAX_BORDER_COLORS) {
      mesa_loge("too many border colors");
      return 0;
   }

   entries[idx] = key;

   _mesa_hash_table_insert_pre_hashed(fd6_ctx->bcolor_cache, hash,
                                      &entries[idx], (void *)(uintptr_t)idx);

   return idx;
}

static void *
fd6_sampler_state_create(struct pipe_context *pctx,
                         const struct pipe_sampler_state *cso)
{
   struct fd6_sampler_stateobj *so = CALLOC_STRUCT(fd6_sampler_stateobj);
   struct fd_context *ctx = fd_context(pctx);
   unsigned aniso = util_last_bit(MIN2(cso->max_anisotropy >> 1, 8));
   bool miplinear = false;

   if (!so)
      return NULL;

   so->base = *cso;
   so->seqno = util_idalloc_alloc(&fd6_context(ctx)->tex_ids);

   if (cso->min_mip_filter == PIPE_TEX_MIPFILTER_LINEAR)
      miplinear = true;

   bool needs_border = false;
   so->texsamp0 =
      COND(miplinear, A6XX_TEX_SAMP_0_MIPFILTER_LINEAR_NEAR) |
      A6XX_TEX_SAMP_0_XY_MAG(tex_filter(cso->mag_img_filter, aniso)) |
      A6XX_TEX_SAMP_0_XY_MIN(tex_filter(cso->min_img_filter, aniso)) |
      A6XX_TEX_SAMP_0_ANISO((enum a6xx_tex_aniso)aniso) |
      A6XX_TEX_SAMP_0_WRAP_S(tex_clamp(cso->wrap_s, &needs_border)) |
      A6XX_TEX_SAMP_0_WRAP_T(tex_clamp(cso->wrap_t, &needs_border)) |
      A6XX_TEX_SAMP_0_WRAP_R(tex_clamp(cso->wrap_r, &needs_border));

   so->texsamp1 =
      COND(cso->min_mip_filter == PIPE_TEX_MIPFILTER_NONE,
           A6XX_TEX_SAMP_1_MIPFILTER_LINEAR_FAR) |
      COND(!cso->seamless_cube_map, A6XX_TEX_SAMP_1_CUBEMAPSEAMLESSFILTOFF) |
      COND(cso->unnormalized_coords, A6XX_TEX_SAMP_1_UNNORM_COORDS);

   so->texsamp0 |= A6XX_TEX_SAMP_0_LOD_BIAS(cso->lod_bias);
   so->texsamp1 |= A6XX_TEX_SAMP_1_MIN_LOD(cso->min_lod) |
                   A6XX_TEX_SAMP_1_MAX_LOD(cso->max_lod);

   if (cso->compare_mode)
      so->texsamp1 |=
         A6XX_TEX_SAMP_1_COMPARE_FUNC((enum adreno_compare_func)cso->compare_func); /* maps 1:1 */

   if (needs_border)
      so->texsamp2 = A6XX_TEX_SAMP_2_BCOLOR(get_bcolor_offset(ctx, cso));

   /* We don't know if the format is going to be YUV.  Setting CHROMA_LINEAR
    * unconditionally seems fine.
    */
   if (cso->mag_img_filter == PIPE_TEX_FILTER_LINEAR &&
       cso->min_img_filter == PIPE_TEX_FILTER_LINEAR)
      so->texsamp2 |= A6XX_TEX_SAMP_2_CHROMA_LINEAR;

   so->texsamp2 |=
      A6XX_TEX_SAMP_2_REDUCTION_MODE(reduction_mode(cso->reduction_mode));

   return so;
}

static void
fd6_sampler_state_delete(struct pipe_context *pctx, void *hwcso)
{
   struct fd_context *ctx = fd_context(pctx);
   struct fd6_context *fd6_ctx = fd6_context(ctx);
   struct fd6_sampler_stateobj *samp = (struct fd6_sampler_stateobj *)hwcso;

   fd_screen_lock(ctx->screen);

   hash_table_foreach (fd6_ctx->tex_cache, entry) {
      struct fd6_texture_state *state = (struct fd6_texture_state *)entry->data;

      for (unsigned i = 0; i < ARRAY_SIZE(state->key.samp_seqno); i++) {
         if (samp->seqno == state->key.samp_seqno[i]) {
            remove_tex_entry(fd6_ctx, entry);
            break;
         }
      }
   }

   fd_screen_unlock(ctx->screen);

   util_idalloc_free(&fd6_ctx->tex_ids, samp->seqno);

   free(hwcso);
}

static struct pipe_sampler_view *
fd6_sampler_view_create(struct pipe_context *pctx, struct pipe_resource *prsc,
                        const struct pipe_sampler_view *cso)
{
   struct fd6_context *fd6_ctx = fd6_context(fd_context(pctx));
   struct fd6_pipe_sampler_view *so = CALLOC_STRUCT(fd6_pipe_sampler_view);

   if (!so)
      return NULL;

   so->base = *cso;
   so->seqno = util_idalloc_alloc(&fd6_ctx->tex_ids);
   pipe_reference(NULL, &prsc->reference);
   so->base.texture = prsc;
   so->base.reference.count = 1;
   so->base.context = pctx;

   return &so->base;
}

/**
 * Remove any texture state entries that reference the specified sampler
 * view.
 */
static void
fd6_sampler_view_invalidate(struct fd_context *ctx,
                            struct fd6_pipe_sampler_view *view)
{
   struct fd6_context *fd6_ctx = fd6_context(ctx);

   fd_screen_lock(ctx->screen);

   hash_table_foreach (fd6_ctx->tex_cache, entry) {
      struct fd6_texture_state *state = (struct fd6_texture_state *)entry->data;

      for (unsigned i = 0; i < ARRAY_SIZE(state->key.view_seqno); i++) {
         if (view->seqno == state->key.view_seqno[i]) {
            remove_tex_entry(fd6_ctx, entry);
            break;
         }
      }
   }

   fd_screen_unlock(ctx->screen);
}

static void
fd6_sampler_view_update(struct fd_context *ctx,
                        struct fd6_pipe_sampler_view *so)
   assert_dt
{
   const struct pipe_sampler_view *cso = &so->base;
   struct pipe_resource *prsc = cso->texture;
   struct fd_resource *rsc = fd_resource(prsc);
   enum pipe_format format = cso->format;

   fd6_assert_valid_format(rsc, cso->format);

   /* If texture has not had a layout change, then no update needed: */
   if (so->rsc_seqno == rsc->seqno)
      return;

   fd6_sampler_view_invalidate(ctx, so);

   so->rsc_seqno = rsc->seqno;

   if (format == PIPE_FORMAT_X32_S8X24_UINT) {
      rsc = rsc->stencil;
      format = rsc->b.b.format;
   }

   so->ptr1 = rsc;

   if (cso->target == PIPE_BUFFER) {
      uint8_t swiz[4] = {cso->swizzle_r, cso->swizzle_g, cso->swizzle_b,
                         cso->swizzle_a};

      /* Using relocs for addresses still */
      uint64_t iova = cso->u.buf.offset;

      uint32_t size = fd_clamp_buffer_size(cso->format, cso->u.buf.size,
                                           A4XX_MAX_TEXEL_BUFFER_ELEMENTS_UINT);

      fdl6_buffer_view_init(so->descriptor, cso->format, swiz, iova, size);
   } else {
      struct fdl_view_args args = {
         .chip = A6XX,

         /* Using relocs for addresses still */
         .iova = 0,

         .base_miplevel = fd_sampler_first_level(cso),
         .level_count =
            fd_sampler_last_level(cso) - fd_sampler_first_level(cso) + 1,

         .base_array_layer = cso->u.tex.first_layer,
         .layer_count = cso->u.tex.last_layer - cso->u.tex.first_layer + 1,

         .swiz = {cso->swizzle_r, cso->swizzle_g, cso->swizzle_b,
                  cso->swizzle_a},
         .format = format,

         .type = fdl_type_from_pipe_target(cso->target),
         .chroma_offsets = {FDL_CHROMA_LOCATION_COSITED_EVEN,
                            FDL_CHROMA_LOCATION_COSITED_EVEN},
      };

      if (rsc->b.b.format == PIPE_FORMAT_R8_G8B8_420_UNORM) {
         args.chroma_offsets[0] = FDL_CHROMA_LOCATION_MIDPOINT;
         args.chroma_offsets[1] = FDL_CHROMA_LOCATION_MIDPOINT;
      }

      struct fd_resource *plane1 = fd_resource(rsc->b.b.next);
      struct fd_resource *plane2 =
         plane1 ? fd_resource(plane1->b.b.next) : NULL;
      static const struct fdl_layout dummy_layout = {};
      const struct fdl_layout *layouts[3] = {
         &rsc->layout,
         plane1 ? &plane1->layout : &dummy_layout,
         plane2 ? &plane2->layout : &dummy_layout,
      };
      struct fdl6_view view;
      fdl6_view_init(&view, layouts, &args,
                     ctx->screen->info->a6xx.has_z24uint_s8uint);
      memcpy(so->descriptor, view.descriptor, sizeof(so->descriptor));

      if (rsc->b.b.format == PIPE_FORMAT_R8_G8B8_420_UNORM) {
         /* In case of biplanar R8_G8B8, the UBWC metadata address in
          * dwords 7 and 8, is instead the pointer to the second plane.
          */
         so->ptr2 = plane1;
      } else {
         if (fd_resource_ubwc_enabled(rsc, fd_sampler_first_level(cso))) {
            so->ptr2 = rsc;
         }
      }
   }
}

static void
fd6_set_sampler_views(struct pipe_context *pctx, enum pipe_shader_type shader,
                      unsigned start, unsigned nr,
                      unsigned unbind_num_trailing_slots,
                      bool take_ownership,
                      struct pipe_sampler_view **views)
   in_dt
{
   struct fd_context *ctx = fd_context(pctx);

   fd_set_sampler_views(pctx, shader, start, nr, unbind_num_trailing_slots,
                        take_ownership, views);

   if (!views)
      return;

   for (unsigned i = 0; i < nr; i++) {
      struct fd6_pipe_sampler_view *so = fd6_pipe_sampler_view(views[i + start]);

      if (!so)
         continue;

      struct fd_resource *rsc = fd_resource(so->base.texture);

      fd6_validate_format(ctx, rsc, so->base.format);
      fd6_sampler_view_update(ctx, so);
   }
}

/* NOTE this can be called in either driver thread or frontend thread
 * depending on where the last unref comes from
 */
static void
fd6_sampler_view_destroy(struct pipe_context *pctx,
                         struct pipe_sampler_view *_view)
{
   struct fd_context *ctx = fd_context(pctx);
   struct fd6_pipe_sampler_view *view = fd6_pipe_sampler_view(_view);

   fd6_sampler_view_invalidate(ctx, view);

   pipe_resource_reference(&view->base.texture, NULL);

   util_idalloc_free(&fd6_context(ctx)->tex_ids, view->seqno);

   free(view);
}

static uint32_t
tex_key_hash(const void *_key)
{
   const struct fd6_texture_key *key = (const struct fd6_texture_key *)_key;
   return XXH32(key, sizeof(*key), 0);
}

static bool
tex_key_equals(const void *_a, const void *_b)
{
   const struct fd6_texture_key *a = (const struct fd6_texture_key *)_a;
   const struct fd6_texture_key *b = (const struct fd6_texture_key *)_b;
   return memcmp(a, b, sizeof(struct fd6_texture_key)) == 0;
}

static struct fd_ringbuffer *
build_texture_state(struct fd_context *ctx, enum pipe_shader_type type,
                    struct fd_texture_stateobj *tex)
   assert_dt
{
   struct fd_ringbuffer *ring = fd_ringbuffer_new_object(ctx->pipe, 32 * 4);
   unsigned opcode, tex_samp_reg, tex_const_reg, tex_count_reg;
   enum a6xx_state_block sb;

   switch (type) {
   case PIPE_SHADER_VERTEX:
      sb = SB6_VS_TEX;
      opcode = CP_LOAD_STATE6_GEOM;
      tex_samp_reg = REG_A6XX_SP_VS_TEX_SAMP;
      tex_const_reg = REG_A6XX_SP_VS_TEX_CONST;
      tex_count_reg = REG_A6XX_SP_VS_TEX_COUNT;
      break;
   case PIPE_SHADER_TESS_CTRL:
      sb = SB6_HS_TEX;
      opcode = CP_LOAD_STATE6_GEOM;
      tex_samp_reg = REG_A6XX_SP_HS_TEX_SAMP;
      tex_const_reg = REG_A6XX_SP_HS_TEX_CONST;
      tex_count_reg = REG_A6XX_SP_HS_TEX_COUNT;
      break;
   case PIPE_SHADER_TESS_EVAL:
      sb = SB6_DS_TEX;
      opcode = CP_LOAD_STATE6_GEOM;
      tex_samp_reg = REG_A6XX_SP_DS_TEX_SAMP;
      tex_const_reg = REG_A6XX_SP_DS_TEX_CONST;
      tex_count_reg = REG_A6XX_SP_DS_TEX_COUNT;
      break;
   case PIPE_SHADER_GEOMETRY:
      sb = SB6_GS_TEX;
      opcode = CP_LOAD_STATE6_GEOM;
      tex_samp_reg = REG_A6XX_SP_GS_TEX_SAMP;
      tex_const_reg = REG_A6XX_SP_GS_TEX_CONST;
      tex_count_reg = REG_A6XX_SP_GS_TEX_COUNT;
      break;
   case PIPE_SHADER_FRAGMENT:
      sb = SB6_FS_TEX;
      opcode = CP_LOAD_STATE6_FRAG;
      tex_samp_reg = REG_A6XX_SP_FS_TEX_SAMP;
      tex_const_reg = REG_A6XX_SP_FS_TEX_CONST;
      tex_count_reg = REG_A6XX_SP_FS_TEX_COUNT;
      break;
   case PIPE_SHADER_COMPUTE:
      sb = SB6_CS_TEX;
      opcode = CP_LOAD_STATE6_FRAG;
      tex_samp_reg = REG_A6XX_SP_CS_TEX_SAMP;
      tex_const_reg = REG_A6XX_SP_CS_TEX_CONST;
      tex_count_reg = REG_A6XX_SP_CS_TEX_COUNT;
      break;
   default:
      unreachable("bad state block");
   }

   if (tex->num_samplers > 0) {
      struct fd_ringbuffer *state =
         fd_ringbuffer_new_object(ctx->pipe, tex->num_samplers * 4 * 4);
      for (unsigned i = 0; i < tex->num_samplers; i++) {
         static const struct fd6_sampler_stateobj dummy_sampler = {};
         const struct fd6_sampler_stateobj *sampler =
            tex->samplers[i] ? fd6_sampler_stateobj(tex->samplers[i])
                             : &dummy_sampler;
         OUT_RING(state, sampler->texsamp0);
         OUT_RING(state, sampler->texsamp1);
         OUT_RING(state, sampler->texsamp2);
         OUT_RING(state, sampler->texsamp3);
      }

      /* output sampler state: */
      OUT_PKT7(ring, opcode, 3);
      OUT_RING(ring, CP_LOAD_STATE6_0_DST_OFF(0) |
                        CP_LOAD_STATE6_0_STATE_TYPE(ST6_SHADER) |
                        CP_LOAD_STATE6_0_STATE_SRC(SS6_INDIRECT) |
                        CP_LOAD_STATE6_0_STATE_BLOCK(sb) |
                        CP_LOAD_STATE6_0_NUM_UNIT(tex->num_samplers));
      OUT_RB(ring, state); /* SRC_ADDR_LO/HI */

      OUT_PKT4(ring, tex_samp_reg, 2);
      OUT_RB(ring, state); /* SRC_ADDR_LO/HI */

      fd_ringbuffer_del(state);
   }

   unsigned num_textures = tex->num_textures;

   if (num_textures > 0) {
      struct fd_ringbuffer *state =
         fd_ringbuffer_new_object(ctx->pipe, num_textures * 16 * 4);
      for (unsigned i = 0; i < num_textures; i++) {
         const struct fd6_pipe_sampler_view *view;

         if (tex->textures[i]) {
            view = fd6_pipe_sampler_view(tex->textures[i]);
            struct fd_resource *rsc = fd_resource(view->base.texture);
            fd6_assert_valid_format(rsc, view->base.format);
            assert(view->rsc_seqno == rsc->seqno);
         } else {
            static const struct fd6_pipe_sampler_view dummy_view = {};
            view = &dummy_view;
         }

         OUT_RING(state, view->descriptor[0]);
         OUT_RING(state, view->descriptor[1]);
         OUT_RING(state, view->descriptor[2]);
         OUT_RING(state, view->descriptor[3]);

         if (view->ptr1) {
            OUT_RELOC(state, view->ptr1->bo, view->descriptor[4],
                      (uint64_t)view->descriptor[5] << 32, 0);
         } else {
            OUT_RING(state, view->descriptor[4]);
            OUT_RING(state, view->descriptor[5]);
         }

         OUT_RING(state, view->descriptor[6]);

         if (view->ptr2) {
            OUT_RELOC(state, view->ptr2->bo, view->descriptor[7], 0, 0);
         } else {
            OUT_RING(state, view->descriptor[7]);
            OUT_RING(state, view->descriptor[8]);
         }

         OUT_RING(state, view->descriptor[9]);
         OUT_RING(state, view->descriptor[10]);
         OUT_RING(state, view->descriptor[11]);
         OUT_RING(state, view->descriptor[12]);
         OUT_RING(state, view->descriptor[13]);
         OUT_RING(state, view->descriptor[14]);
         OUT_RING(state, view->descriptor[15]);
      }

      /* emit texture state: */
      OUT_PKT7(ring, opcode, 3);
      OUT_RING(ring, CP_LOAD_STATE6_0_DST_OFF(0) |
                        CP_LOAD_STATE6_0_STATE_TYPE(ST6_CONSTANTS) |
                        CP_LOAD_STATE6_0_STATE_SRC(SS6_INDIRECT) |
                        CP_LOAD_STATE6_0_STATE_BLOCK(sb) |
                        CP_LOAD_STATE6_0_NUM_UNIT(num_textures));
      OUT_RB(ring, state); /* SRC_ADDR_LO/HI */

      OUT_PKT4(ring, tex_const_reg, 2);
      OUT_RB(ring, state); /* SRC_ADDR_LO/HI */

      fd_ringbuffer_del(state);
   }

   OUT_PKT4(ring, tex_count_reg, 1);
   OUT_RING(ring, num_textures);

   return ring;
}

/**
 * Handle invalidates potentially coming from other contexts.  By
 * flagging the invalidate rather than handling it immediately, we
 * avoid needing to refcnt (with it's associated atomics) the tex
 * state.  And furthermore, this avoids cross-ctx (thread) sharing
 * of fd_ringbuffer's, avoiding their need for atomic refcnts.
 */
static void
handle_invalidates(struct fd_context *ctx)
   assert_dt
{
   struct fd6_context *fd6_ctx = fd6_context(ctx);

   fd_screen_lock(ctx->screen);

   hash_table_foreach (fd6_ctx->tex_cache, entry) {
      struct fd6_texture_state *state = (struct fd6_texture_state *)entry->data;

      if (state->invalidate)
         remove_tex_entry(fd6_ctx, entry);
   }

   fd_screen_unlock(ctx->screen);

   for (unsigned type = 0; type < ARRAY_SIZE(ctx->tex); type++) {
      struct fd_texture_stateobj *tex = &ctx->tex[type];

      for (unsigned i = 0; i < tex->num_textures; i++) {
         struct fd6_pipe_sampler_view *so =
               fd6_pipe_sampler_view(tex->textures[i]);

         if (!so)
            continue;

         fd6_sampler_view_update(ctx, so);
      }
   }

   fd6_ctx->tex_cache_needs_invalidate = false;
}

struct fd6_texture_state *
fd6_texture_state(struct fd_context *ctx, enum pipe_shader_type type)
{
   struct fd_texture_stateobj *tex = &ctx->tex[type];
   struct fd6_context *fd6_ctx = fd6_context(ctx);
   struct fd6_texture_state *state = NULL;
   struct fd6_texture_key key;

   if (unlikely(fd6_ctx->tex_cache_needs_invalidate))
      handle_invalidates(ctx);

   memset(&key, 0, sizeof(key));

   for (unsigned i = 0; i < tex->num_textures; i++) {
      if (!tex->textures[i])
         continue;

      struct fd6_pipe_sampler_view *view =
         fd6_pipe_sampler_view(tex->textures[i]);

      key.view_seqno[i] = view->seqno;
   }

   for (unsigned i = 0; i < tex->num_samplers; i++) {
      if (!tex->samplers[i])
         continue;

      struct fd6_sampler_stateobj *sampler =
         fd6_sampler_stateobj(tex->samplers[i]);

      key.samp_seqno[i] = sampler->seqno;
   }

   key.type = type;

   uint32_t hash = tex_key_hash(&key);
   fd_screen_lock(ctx->screen);

   struct hash_entry *entry =
      _mesa_hash_table_search_pre_hashed(fd6_ctx->tex_cache, hash, &key);

   if (entry) {
      state = (struct fd6_texture_state *)entry->data;
      for (unsigned i = 0; i < tex->num_textures; i++) {
         uint16_t seqno = tex->textures[i] ?
               fd_resource(tex->textures[i]->texture)->seqno : 0;

         assert(state->view_rsc_seqno[i] == seqno);
      }
      goto out_unlock;
   }

   state = CALLOC_STRUCT(fd6_texture_state);

   for (unsigned i = 0; i < tex->num_textures; i++) {
      if (!tex->textures[i])
         continue;

      struct fd_resource *rsc = fd_resource(tex->textures[i]->texture);

      assert(rsc->dirty & FD_DIRTY_TEX);

      state->view_rsc_seqno[i] = rsc->seqno;
   }

   state->key = key;
   state->stateobj = build_texture_state(ctx, type, tex);

   /* NOTE: uses copy of key in state obj, because pointer passed by caller
    * is probably on the stack
    */
   _mesa_hash_table_insert_pre_hashed(fd6_ctx->tex_cache, hash, &state->key,
                                      state);

out_unlock:
   fd_screen_unlock(ctx->screen);
   return state;
}

static void
fd6_texture_state_destroy(struct fd6_texture_state *state)
{
   fd_ringbuffer_del(state->stateobj);
   free(state);
}

static void
fd6_rebind_resource(struct fd_context *ctx, struct fd_resource *rsc) assert_dt
{
   fd_screen_assert_locked(ctx->screen);

   if (!(rsc->dirty & FD_DIRTY_TEX))
      return;

   struct fd6_context *fd6_ctx = fd6_context(ctx);

   hash_table_foreach (fd6_ctx->tex_cache, entry) {
      struct fd6_texture_state *state = (struct fd6_texture_state *)entry->data;

      STATIC_ASSERT(ARRAY_SIZE(state->view_rsc_seqno) == ARRAY_SIZE(state->key.view_seqno));

      for (unsigned i = 0; i < ARRAY_SIZE(state->view_rsc_seqno); i++) {
         if (rsc->seqno == state->view_rsc_seqno[i]) {
            struct fd6_texture_state *tex =
                  (struct fd6_texture_state *)entry->data;
            tex->invalidate = true;
            fd6_ctx->tex_cache_needs_invalidate = true;
         }
      }
   }
}

void
fd6_texture_init(struct pipe_context *pctx) disable_thread_safety_analysis
{
   struct fd_context *ctx = fd_context(pctx);
   struct fd6_context *fd6_ctx = fd6_context(ctx);

   pctx->create_sampler_state = fd6_sampler_state_create;
   pctx->delete_sampler_state = fd6_sampler_state_delete;
   pctx->bind_sampler_states = fd_sampler_states_bind;

   pctx->create_sampler_view = fd6_sampler_view_create;
   pctx->sampler_view_destroy = fd6_sampler_view_destroy;
   pctx->set_sampler_views = fd6_set_sampler_views;

   ctx->rebind_resource = fd6_rebind_resource;

   fd6_ctx->bcolor_cache =
         _mesa_hash_table_create(NULL, bcolor_key_hash, bcolor_key_equals);
   fd6_ctx->bcolor_mem = fd_bo_new(ctx->screen->dev,
                                   FD6_MAX_BORDER_COLORS * FD6_BORDER_COLOR_SIZE,
                                   0, "bcolor");

   fd_context_add_private_bo(ctx, fd6_ctx->bcolor_mem);

   fd6_ctx->tex_cache = _mesa_hash_table_create(NULL, tex_key_hash, tex_key_equals);
   util_idalloc_init(&fd6_ctx->tex_ids, 256);
}

void
fd6_texture_fini(struct pipe_context *pctx)
{
   struct fd_context *ctx = fd_context(pctx);
   struct fd6_context *fd6_ctx = fd6_context(ctx);

   fd_screen_lock(ctx->screen);

   hash_table_foreach (fd6_ctx->tex_cache, entry) {
      remove_tex_entry(fd6_ctx, entry);
   }

   fd_screen_unlock(ctx->screen);

   util_idalloc_fini(&fd6_ctx->tex_ids);
   ralloc_free(fd6_ctx->tex_cache);
   fd_bo_del(fd6_ctx->bcolor_mem);
   ralloc_free(fd6_ctx->bcolor_cache);
}
