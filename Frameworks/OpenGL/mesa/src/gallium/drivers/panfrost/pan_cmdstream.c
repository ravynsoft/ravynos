/*
 * Copyright (C) 2023 Amazon.com, Inc. or its affiliates.
 * Copyright (C) 2018 Alyssa Rosenzweig
 * Copyright (C) 2020 Collabora Ltd.
 * Copyright © 2017 Intel Corporation
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
 */

#include "gallium/auxiliary/util/u_blend.h"
#include "pipe/p_defines.h"
#include "pipe/p_state.h"
#include "util/macros.h"
#include "util/u_draw.h"
#include "util/u_helpers.h"
#include "util/u_memory.h"
#include "util/u_prim.h"
#include "util/u_sample_positions.h"
#include "util/u_vbuf.h"
#include "util/u_viewport.h"

#include "decode.h"

#include "genxml/gen_macros.h"

#include "pan_afbc_cso.h"
#include "pan_blend.h"
#include "pan_blitter.h"
#include "pan_bo.h"
#include "pan_cmdstream.h"
#include "pan_context.h"
#include "pan_indirect_dispatch.h"
#include "pan_jm.h"
#include "pan_job.h"
#include "pan_pool.h"
#include "pan_resource.h"
#include "pan_shader.h"
#include "pan_texture.h"
#include "pan_util.h"

/* JOBX() is used to select the job backend helpers to call from generic
 * functions. */
#if PAN_ARCH <= 9
#define JOBX(__suffix) GENX(jm_##__suffix)
#else
#error "Unsupported arch"
#endif

struct panfrost_sampler_state {
   struct pipe_sampler_state base;
   struct mali_sampler_packed hw;
};

/* Misnomer: Sampler view corresponds to textures, not samplers */

struct panfrost_sampler_view {
   struct pipe_sampler_view base;
   struct panfrost_pool_ref state;
   struct mali_texture_packed bifrost_descriptor;
   mali_ptr texture_bo;
   uint64_t modifier;

   /* Pool used to allocate the descriptor. If NULL, defaults to the global
    * descriptor pool. Can be set for short lived descriptors, useful for
    * shader images on Valhall.
    */
   struct panfrost_pool *pool;
};

/* Statically assert that PIPE_* enums match the hardware enums.
 * (As long as they match, we don't need to translate them.)
 */
static_assert((int)PIPE_FUNC_NEVER == MALI_FUNC_NEVER, "must match");
static_assert((int)PIPE_FUNC_LESS == MALI_FUNC_LESS, "must match");
static_assert((int)PIPE_FUNC_EQUAL == MALI_FUNC_EQUAL, "must match");
static_assert((int)PIPE_FUNC_LEQUAL == MALI_FUNC_LEQUAL, "must match");
static_assert((int)PIPE_FUNC_GREATER == MALI_FUNC_GREATER, "must match");
static_assert((int)PIPE_FUNC_NOTEQUAL == MALI_FUNC_NOT_EQUAL, "must match");
static_assert((int)PIPE_FUNC_GEQUAL == MALI_FUNC_GEQUAL, "must match");
static_assert((int)PIPE_FUNC_ALWAYS == MALI_FUNC_ALWAYS, "must match");

static inline enum mali_sample_pattern
panfrost_sample_pattern(unsigned samples)
{
   switch (samples) {
   case 1:
      return MALI_SAMPLE_PATTERN_SINGLE_SAMPLED;
   case 4:
      return MALI_SAMPLE_PATTERN_ROTATED_4X_GRID;
   case 8:
      return MALI_SAMPLE_PATTERN_D3D_8X_GRID;
   case 16:
      return MALI_SAMPLE_PATTERN_D3D_16X_GRID;
   default:
      unreachable("Unsupported sample count");
   }
}

static unsigned
translate_tex_wrap(enum pipe_tex_wrap w, bool using_nearest)
{
   /* CLAMP is only supported on Midgard, where it is broken for nearest
    * filtering. Use CLAMP_TO_EDGE in that case.
    */

   switch (w) {
   case PIPE_TEX_WRAP_REPEAT:
      return MALI_WRAP_MODE_REPEAT;
   case PIPE_TEX_WRAP_CLAMP_TO_EDGE:
      return MALI_WRAP_MODE_CLAMP_TO_EDGE;
   case PIPE_TEX_WRAP_CLAMP_TO_BORDER:
      return MALI_WRAP_MODE_CLAMP_TO_BORDER;
   case PIPE_TEX_WRAP_MIRROR_REPEAT:
      return MALI_WRAP_MODE_MIRRORED_REPEAT;
   case PIPE_TEX_WRAP_MIRROR_CLAMP_TO_EDGE:
      return MALI_WRAP_MODE_MIRRORED_CLAMP_TO_EDGE;
   case PIPE_TEX_WRAP_MIRROR_CLAMP_TO_BORDER:
      return MALI_WRAP_MODE_MIRRORED_CLAMP_TO_BORDER;

#if PAN_ARCH <= 5
   case PIPE_TEX_WRAP_CLAMP:
      return using_nearest ? MALI_WRAP_MODE_CLAMP_TO_EDGE
                           : MALI_WRAP_MODE_CLAMP;
   case PIPE_TEX_WRAP_MIRROR_CLAMP:
      return using_nearest ? MALI_WRAP_MODE_MIRRORED_CLAMP_TO_EDGE
                           : MALI_WRAP_MODE_MIRRORED_CLAMP;
#endif

   default:
      unreachable("Invalid wrap");
   }
}

/* The hardware compares in the wrong order order, so we have to flip before
 * encoding. Yes, really. */

static enum mali_func
panfrost_sampler_compare_func(const struct pipe_sampler_state *cso)
{
   return !cso->compare_mode
             ? MALI_FUNC_NEVER
             : panfrost_flip_compare_func((enum mali_func)cso->compare_func);
}

static enum mali_mipmap_mode
pan_pipe_to_mipmode(enum pipe_tex_mipfilter f)
{
   switch (f) {
   case PIPE_TEX_MIPFILTER_NEAREST:
      return MALI_MIPMAP_MODE_NEAREST;
   case PIPE_TEX_MIPFILTER_LINEAR:
      return MALI_MIPMAP_MODE_TRILINEAR;
#if PAN_ARCH >= 6
   case PIPE_TEX_MIPFILTER_NONE:
      return MALI_MIPMAP_MODE_NONE;
#else
   case PIPE_TEX_MIPFILTER_NONE:
      return MALI_MIPMAP_MODE_NEAREST;
#endif
   default:
      unreachable("Invalid");
   }
}

static void *
panfrost_create_sampler_state(struct pipe_context *pctx,
                              const struct pipe_sampler_state *cso)
{
   struct panfrost_sampler_state *so = CALLOC_STRUCT(panfrost_sampler_state);
   so->base = *cso;

#if PAN_ARCH == 7
   /* On v7, pan_texture.c composes the API swizzle with a bijective
    * swizzle derived from the format, to allow more formats than the
    * hardware otherwise supports. When packing border colours, we need to
    * undo this bijection, by swizzling with its inverse.
    */
   unsigned mali_format = panfrost_pipe_format_v7[cso->border_color_format].hw;
   enum mali_rgb_component_order order = mali_format & BITFIELD_MASK(12);

   unsigned char inverted_swizzle[4];
   panfrost_invert_swizzle(GENX(pan_decompose_swizzle)(order).post,
                           inverted_swizzle);

   util_format_apply_color_swizzle(&so->base.border_color, &cso->border_color,
                                   inverted_swizzle,
                                   false /* is_integer (irrelevant) */);
#endif

   bool using_nearest = cso->min_img_filter == PIPE_TEX_MIPFILTER_NEAREST;

   pan_pack(&so->hw, SAMPLER, cfg) {
      cfg.magnify_nearest = cso->mag_img_filter == PIPE_TEX_FILTER_NEAREST;
      cfg.minify_nearest = cso->min_img_filter == PIPE_TEX_FILTER_NEAREST;

      cfg.normalized_coordinates = !cso->unnormalized_coords;
      cfg.lod_bias = cso->lod_bias;
      cfg.minimum_lod = cso->min_lod;
      cfg.maximum_lod = cso->max_lod;

      cfg.wrap_mode_s = translate_tex_wrap(cso->wrap_s, using_nearest);
      cfg.wrap_mode_t = translate_tex_wrap(cso->wrap_t, using_nearest);
      cfg.wrap_mode_r = translate_tex_wrap(cso->wrap_r, using_nearest);

      cfg.mipmap_mode = pan_pipe_to_mipmode(cso->min_mip_filter);
      cfg.compare_function = panfrost_sampler_compare_func(cso);
      cfg.seamless_cube_map = cso->seamless_cube_map;

      cfg.border_color_r = so->base.border_color.ui[0];
      cfg.border_color_g = so->base.border_color.ui[1];
      cfg.border_color_b = so->base.border_color.ui[2];
      cfg.border_color_a = so->base.border_color.ui[3];

#if PAN_ARCH >= 6
      if (cso->max_anisotropy > 1) {
         cfg.maximum_anisotropy = cso->max_anisotropy;
         cfg.lod_algorithm = MALI_LOD_ALGORITHM_ANISOTROPIC;
      }
#else
      /* Emulate disabled mipmapping by clamping the LOD as tight as
       * possible (from 0 to epsilon = 1/256) */
      if (cso->min_mip_filter == PIPE_TEX_MIPFILTER_NONE)
         cfg.maximum_lod = cfg.minimum_lod + (1.0 / 256.0);
#endif
   }

   return so;
}

/* Get pointers to the blend shaders bound to each active render target. Used
 * to emit the blend descriptors, as well as the fragment renderer state
 * descriptor.
 */
static void
panfrost_get_blend_shaders(struct panfrost_batch *batch,
                           mali_ptr *blend_shaders)
{
   unsigned shader_offset = 0;
   struct panfrost_bo *shader_bo = NULL;

   for (unsigned c = 0; c < batch->key.nr_cbufs; ++c) {
      if (batch->key.cbufs[c]) {
         blend_shaders[c] =
            panfrost_get_blend(batch, c, &shader_bo, &shader_offset);
      }
   }

   if (shader_bo)
      perf_debug_ctx(batch->ctx, "Blend shader use");
}

#if PAN_ARCH >= 5
UNUSED static uint16_t
pack_blend_constant(enum pipe_format format, float cons)
{
   const struct util_format_description *format_desc =
      util_format_description(format);

   unsigned chan_size = 0;

   for (unsigned i = 0; i < format_desc->nr_channels; i++)
      chan_size = MAX2(format_desc->channel[0].size, chan_size);

   uint16_t unorm = (cons * ((1 << chan_size) - 1));
   return unorm << (16 - chan_size);
}

static void
panfrost_emit_blend(struct panfrost_batch *batch, void *rts,
                    mali_ptr *blend_shaders)
{
   unsigned rt_count = batch->key.nr_cbufs;
   struct panfrost_context *ctx = batch->ctx;
   const struct panfrost_blend_state *so = ctx->blend;
   bool dithered = so->base.dither;

   /* Always have at least one render target for depth-only passes */
   for (unsigned i = 0; i < MAX2(rt_count, 1); ++i) {
      struct mali_blend_packed *packed = rts + (i * pan_size(BLEND));

      /* Disable blending for unbacked render targets */
      if (rt_count == 0 || !batch->key.cbufs[i] || !so->info[i].enabled) {
         pan_pack(rts + i * pan_size(BLEND), BLEND, cfg) {
            cfg.enable = false;
#if PAN_ARCH >= 6
            cfg.internal.mode = MALI_BLEND_MODE_OFF;
#endif
         }

         continue;
      }

      struct pan_blend_info info = so->info[i];
      enum pipe_format format = batch->key.cbufs[i]->format;
      float cons =
         pan_blend_get_constant(info.constant_mask, ctx->blend_color.color);

      /* Word 0: Flags and constant */
      pan_pack(packed, BLEND, cfg) {
         cfg.srgb = util_format_is_srgb(format);
         cfg.load_destination = info.load_dest;
         cfg.round_to_fb_precision = !dithered;
         cfg.alpha_to_one = ctx->blend->base.alpha_to_one;
#if PAN_ARCH >= 6
         if (!blend_shaders[i])
            cfg.constant = pack_blend_constant(format, cons);
#else
         cfg.blend_shader = (blend_shaders[i] != 0);

         if (blend_shaders[i])
            cfg.shader_pc = blend_shaders[i];
         else
            cfg.constant = cons;
#endif
      }

      if (!blend_shaders[i]) {
         /* Word 1: Blend Equation */
         STATIC_ASSERT(pan_size(BLEND_EQUATION) == 4);
         packed->opaque[PAN_ARCH >= 6 ? 1 : 2] = so->equation[i];
      }

#if PAN_ARCH >= 6
      const struct panfrost_device *dev = pan_device(ctx->base.screen);
      struct panfrost_compiled_shader *fs = ctx->prog[PIPE_SHADER_FRAGMENT];

      /* Words 2 and 3: Internal blend */
      if (blend_shaders[i]) {
         /* The blend shader's address needs to be at
          * the same top 32 bit as the fragment shader.
          * TODO: Ensure that's always the case.
          */
         assert(!fs->bin.bo || (blend_shaders[i] & (0xffffffffull << 32)) ==
                                  (fs->bin.gpu & (0xffffffffull << 32)));

         pan_pack(&packed->opaque[2], INTERNAL_BLEND, cfg) {
            cfg.mode = MALI_BLEND_MODE_SHADER;
            cfg.shader.pc = (u32)blend_shaders[i];

#if PAN_ARCH <= 7
            unsigned ret_offset = fs->info.bifrost.blend[i].return_offset;
            assert(!(ret_offset & 0x7));

            cfg.shader.return_value = ret_offset ? fs->bin.gpu + ret_offset : 0;
#endif
         }
      } else {
         pan_pack(&packed->opaque[2], INTERNAL_BLEND, cfg) {
            cfg.mode = info.opaque ? MALI_BLEND_MODE_OPAQUE
                                   : MALI_BLEND_MODE_FIXED_FUNCTION;

            /* If we want the conversion to work properly,
             * num_comps must be set to 4
             */
            cfg.fixed_function.num_comps = 4;
            cfg.fixed_function.conversion.memory_format =
               panfrost_format_to_bifrost_blend(dev, format, dithered);
            cfg.fixed_function.rt = i;

#if PAN_ARCH <= 7
            if (!info.opaque) {
               cfg.fixed_function.alpha_zero_nop = info.alpha_zero_nop;
               cfg.fixed_function.alpha_one_store = info.alpha_one_store;
            }

            if (fs->info.fs.untyped_color_outputs) {
               cfg.fixed_function.conversion.register_format = GENX(
                  pan_fixup_blend_type)(fs->info.bifrost.blend[i].type, format);
            } else {
               cfg.fixed_function.conversion.register_format =
                  fs->info.bifrost.blend[i].format;
            }
#endif
         }
      }
#endif
   }
}
#endif

static mali_ptr
panfrost_emit_compute_shader_meta(struct panfrost_batch *batch,
                                  enum pipe_shader_type stage)
{
   struct panfrost_compiled_shader *ss = batch->ctx->prog[stage];

   panfrost_batch_add_bo(batch, ss->bin.bo, PIPE_SHADER_VERTEX);
   panfrost_batch_add_bo(batch, ss->state.bo, PIPE_SHADER_VERTEX);

   return ss->state.gpu;
}

#if PAN_ARCH <= 7
/* Construct a partial RSD corresponding to no executed fragment shader, and
 * merge with the existing partial RSD. */

static void
pan_merge_empty_fs(struct mali_renderer_state_packed *rsd)
{
   struct mali_renderer_state_packed empty_rsd;

   pan_pack(&empty_rsd, RENDERER_STATE, cfg) {
#if PAN_ARCH >= 6
      cfg.properties.shader_modifies_coverage = true;
      cfg.properties.allow_forward_pixel_to_kill = true;
      cfg.properties.allow_forward_pixel_to_be_killed = true;
      cfg.properties.zs_update_operation = MALI_PIXEL_KILL_STRONG_EARLY;

      /* Alpha isn't written so these are vacuous */
      cfg.multisample_misc.overdraw_alpha0 = true;
      cfg.multisample_misc.overdraw_alpha1 = true;
#else
      cfg.shader.shader = 0x1;
      cfg.properties.work_register_count = 1;
      cfg.properties.depth_source = MALI_DEPTH_SOURCE_FIXED_FUNCTION;
      cfg.properties.force_early_z = true;
#endif
   }

   pan_merge((*rsd), empty_rsd, RENDERER_STATE);
}

static void
panfrost_prepare_fs_state(struct panfrost_context *ctx, mali_ptr *blend_shaders,
                          struct mali_renderer_state_packed *rsd)
{
   struct pipe_rasterizer_state *rast = &ctx->rasterizer->base;
   const struct panfrost_zsa_state *zsa = ctx->depth_stencil;
   struct panfrost_compiled_shader *fs = ctx->prog[PIPE_SHADER_FRAGMENT];
   struct panfrost_blend_state *so = ctx->blend;
   bool alpha_to_coverage = ctx->blend->base.alpha_to_coverage;
   bool msaa = rast->multisample;

   unsigned rt_count = ctx->pipe_framebuffer.nr_cbufs;

   bool has_blend_shader = false;

   for (unsigned c = 0; c < rt_count; ++c)
      has_blend_shader |= (blend_shaders[c] != 0);

   bool has_oq = ctx->occlusion_query && ctx->active_queries;

   pan_pack(rsd, RENDERER_STATE, cfg) {
      if (panfrost_fs_required(fs, so, &ctx->pipe_framebuffer, zsa)) {
#if PAN_ARCH >= 6
         struct pan_earlyzs_state earlyzs = pan_earlyzs_get(
            fs->earlyzs, ctx->depth_stencil->writes_zs || has_oq,
            ctx->blend->base.alpha_to_coverage,
            ctx->depth_stencil->zs_always_passes);

         cfg.properties.pixel_kill_operation = earlyzs.kill;
         cfg.properties.zs_update_operation = earlyzs.update;

         cfg.properties.allow_forward_pixel_to_kill =
            pan_allow_forward_pixel_to_kill(ctx, fs);
#else
         cfg.properties.force_early_z =
            fs->info.fs.can_early_z && !alpha_to_coverage &&
            ((enum mali_func)zsa->base.alpha_func == MALI_FUNC_ALWAYS);

         /* TODO: Reduce this limit? */
         if (has_blend_shader)
            cfg.properties.work_register_count =
               MAX2(fs->info.work_reg_count, 8);
         else
            cfg.properties.work_register_count = fs->info.work_reg_count;

         /* Hardware quirks around early-zs forcing without a
          * depth buffer. Note this breaks occlusion queries. */
         bool force_ez_with_discard = !zsa->enabled && !has_oq;

         cfg.properties.shader_reads_tilebuffer =
            force_ez_with_discard && fs->info.fs.can_discard;
         cfg.properties.shader_contains_discard =
            !force_ez_with_discard && fs->info.fs.can_discard;
#endif
      }

#if PAN_ARCH == 4
      if (rt_count > 0) {
         cfg.multisample_misc.load_destination = so->info[0].load_dest;
         cfg.multisample_misc.blend_shader = (blend_shaders[0] != 0);
         cfg.stencil_mask_misc.write_enable = so->info[0].enabled;
         cfg.stencil_mask_misc.srgb =
            util_format_is_srgb(ctx->pipe_framebuffer.cbufs[0]->format);
         cfg.stencil_mask_misc.dither_disable = !so->base.dither;
         cfg.stencil_mask_misc.alpha_to_one = so->base.alpha_to_one;

         if (blend_shaders[0]) {
            cfg.blend_shader = blend_shaders[0];
         } else {
            cfg.blend_constant = pan_blend_get_constant(
               so->info[0].constant_mask, ctx->blend_color.color);
         }
      } else {
         /* If there is no colour buffer, leaving fields default is
          * fine, except for blending which is nonnullable */
         cfg.blend_equation.color_mask = 0xf;
         cfg.blend_equation.rgb.a = MALI_BLEND_OPERAND_A_SRC;
         cfg.blend_equation.rgb.b = MALI_BLEND_OPERAND_B_SRC;
         cfg.blend_equation.rgb.c = MALI_BLEND_OPERAND_C_ZERO;
         cfg.blend_equation.alpha.a = MALI_BLEND_OPERAND_A_SRC;
         cfg.blend_equation.alpha.b = MALI_BLEND_OPERAND_B_SRC;
         cfg.blend_equation.alpha.c = MALI_BLEND_OPERAND_C_ZERO;
      }
#elif PAN_ARCH == 5
      /* Workaround */
      cfg.legacy_blend_shader = panfrost_last_nonnull(blend_shaders, rt_count);
#endif

      cfg.multisample_misc.sample_mask = msaa ? ctx->sample_mask : 0xFFFF;

      cfg.multisample_misc.evaluate_per_sample = msaa && (ctx->min_samples > 1);

#if PAN_ARCH >= 6
      /* MSAA blend shaders need to pass their sample ID to
       * LD_TILE/ST_TILE, so we must preload it. Additionally, we
       * need per-sample shading for the blend shader, accomplished
       * by forcing per-sample shading for the whole program. */

      if (msaa && has_blend_shader) {
         cfg.multisample_misc.evaluate_per_sample = true;
         cfg.preload.fragment.sample_mask_id = true;
      }

      /* Bifrost does not have native point sprites. Point sprites are
       * lowered in the driver to gl_PointCoord reads. This field
       * actually controls the orientation of gl_PointCoord. Both
       * orientations are controlled with sprite_coord_mode in
       * Gallium.
       */
      cfg.properties.point_sprite_coord_origin_max_y =
         (rast->sprite_coord_mode == PIPE_SPRITE_COORD_LOWER_LEFT);

      cfg.multisample_misc.overdraw_alpha0 = panfrost_overdraw_alpha(ctx, 0);
      cfg.multisample_misc.overdraw_alpha1 = panfrost_overdraw_alpha(ctx, 1);
#endif

      cfg.stencil_mask_misc.alpha_to_coverage = alpha_to_coverage;
      cfg.depth_units = rast->offset_units * 2.0f;
      cfg.depth_factor = rast->offset_scale;
      cfg.depth_bias_clamp = rast->offset_clamp;

      bool back_enab = zsa->base.stencil[1].enabled;
      cfg.stencil_front.reference_value = ctx->stencil_ref.ref_value[0];
      cfg.stencil_back.reference_value =
         ctx->stencil_ref.ref_value[back_enab ? 1 : 0];

#if PAN_ARCH <= 5
      /* v6+ fits register preload here, no alpha testing */
      cfg.alpha_reference = zsa->base.alpha_ref_value;
#endif
   }
}

static void
panfrost_emit_frag_shader(struct panfrost_context *ctx,
                          struct mali_renderer_state_packed *fragmeta,
                          mali_ptr *blend_shaders)
{
   const struct panfrost_zsa_state *zsa = ctx->depth_stencil;
   const struct panfrost_rasterizer *rast = ctx->rasterizer;
   struct panfrost_compiled_shader *fs = ctx->prog[PIPE_SHADER_FRAGMENT];

   /* We need to merge several several partial renderer state descriptors,
    * so stage to temporary storage rather than reading back write-combine
    * memory, which will trash performance. */
   struct mali_renderer_state_packed rsd;
   panfrost_prepare_fs_state(ctx, blend_shaders, &rsd);

#if PAN_ARCH == 4
   if (ctx->pipe_framebuffer.nr_cbufs > 0 && !blend_shaders[0]) {
      /* Word 14: SFBD Blend Equation */
      STATIC_ASSERT(pan_size(BLEND_EQUATION) == 4);
      rsd.opaque[14] = ctx->blend->equation[0];
   }
#endif

   /* Merge with CSO state and upload */
   if (panfrost_fs_required(fs, ctx->blend, &ctx->pipe_framebuffer, zsa)) {
      struct mali_renderer_state_packed *partial_rsd =
         (struct mali_renderer_state_packed *)&fs->partial_rsd;
      STATIC_ASSERT(sizeof(fs->partial_rsd) == sizeof(*partial_rsd));
      pan_merge(rsd, *partial_rsd, RENDERER_STATE);
   } else {
      pan_merge_empty_fs(&rsd);
   }

   /* Word 8, 9 Misc state */
   rsd.opaque[8] |= zsa->rsd_depth.opaque[0] | rast->multisample.opaque[0];

   rsd.opaque[9] |= zsa->rsd_stencil.opaque[0] | rast->stencil_misc.opaque[0];

   /* Word 10, 11 Stencil Front and Back */
   rsd.opaque[10] |= zsa->stencil_front.opaque[0];
   rsd.opaque[11] |= zsa->stencil_back.opaque[0];

   memcpy(fragmeta, &rsd, sizeof(rsd));
}

static mali_ptr
panfrost_emit_frag_shader_meta(struct panfrost_batch *batch)
{
   struct panfrost_context *ctx = batch->ctx;
   struct panfrost_compiled_shader *ss = ctx->prog[PIPE_SHADER_FRAGMENT];

   panfrost_batch_add_bo(batch, ss->bin.bo, PIPE_SHADER_FRAGMENT);

   struct panfrost_ptr xfer;

#if PAN_ARCH == 4
   xfer = pan_pool_alloc_desc(&batch->pool.base, RENDERER_STATE);
#else
   unsigned rt_count = MAX2(ctx->pipe_framebuffer.nr_cbufs, 1);

   xfer =
      pan_pool_alloc_desc_aggregate(&batch->pool.base, PAN_DESC(RENDERER_STATE),
                                    PAN_DESC_ARRAY(rt_count, BLEND));
#endif

   mali_ptr blend_shaders[PIPE_MAX_COLOR_BUFS] = {0};
   panfrost_get_blend_shaders(batch, blend_shaders);

   panfrost_emit_frag_shader(ctx, (struct mali_renderer_state_packed *)xfer.cpu,
                             blend_shaders);

#if PAN_ARCH >= 5
   panfrost_emit_blend(batch, xfer.cpu + pan_size(RENDERER_STATE),
                       blend_shaders);
#endif

   return xfer.gpu;
}
#endif

static mali_ptr
panfrost_emit_viewport(struct panfrost_batch *batch)
{
   struct panfrost_context *ctx = batch->ctx;
   const struct pipe_viewport_state *vp = &ctx->pipe_viewport;
   const struct pipe_scissor_state *ss = &ctx->scissor;
   const struct pipe_rasterizer_state *rast = &ctx->rasterizer->base;

   /* Derive min/max from translate/scale. Note since |x| >= 0 by
    * definition, we have that -|x| <= |x| hence translate - |scale| <=
    * translate + |scale|, so the ordering is correct here. */
   float vp_minx = vp->translate[0] - fabsf(vp->scale[0]);
   float vp_maxx = vp->translate[0] + fabsf(vp->scale[0]);
   float vp_miny = vp->translate[1] - fabsf(vp->scale[1]);
   float vp_maxy = vp->translate[1] + fabsf(vp->scale[1]);

   float minz, maxz;
   util_viewport_zmin_zmax(vp, rast->clip_halfz, &minz, &maxz);

   /* Scissor to the intersection of viewport and to the scissor, clamped
    * to the framebuffer */

   unsigned minx = MIN2(batch->key.width, MAX2((int)vp_minx, 0));
   unsigned maxx = MIN2(batch->key.width, MAX2((int)vp_maxx, 0));
   unsigned miny = MIN2(batch->key.height, MAX2((int)vp_miny, 0));
   unsigned maxy = MIN2(batch->key.height, MAX2((int)vp_maxy, 0));

   if (ss && rast->scissor) {
      minx = MAX2(ss->minx, minx);
      miny = MAX2(ss->miny, miny);
      maxx = MIN2(ss->maxx, maxx);
      maxy = MIN2(ss->maxy, maxy);
   }

   /* Set the range to [1, 1) so max values don't wrap round */
   if (maxx == 0 || maxy == 0)
      maxx = maxy = minx = miny = 1;

   panfrost_batch_union_scissor(batch, minx, miny, maxx, maxy);
   batch->scissor_culls_everything = (minx >= maxx || miny >= maxy);

   /* [minx, maxx) and [miny, maxy) are exclusive ranges in the hardware */
   maxx--;
   maxy--;

   batch->minimum_z = rast->depth_clip_near ? minz : -INFINITY;
   batch->maximum_z = rast->depth_clip_far ? maxz : +INFINITY;

#if PAN_ARCH <= 7
   struct panfrost_ptr T = pan_pool_alloc_desc(&batch->pool.base, VIEWPORT);

   pan_pack(T.cpu, VIEWPORT, cfg) {
      cfg.scissor_minimum_x = minx;
      cfg.scissor_minimum_y = miny;
      cfg.scissor_maximum_x = maxx;
      cfg.scissor_maximum_y = maxy;

      cfg.minimum_z = batch->minimum_z;
      cfg.maximum_z = batch->maximum_z;
   }

   return T.gpu;
#else
   pan_pack(&batch->scissor, SCISSOR, cfg) {
      cfg.scissor_minimum_x = minx;
      cfg.scissor_minimum_y = miny;
      cfg.scissor_maximum_x = maxx;
      cfg.scissor_maximum_y = maxy;
   }

   return 0;
#endif
}

#if PAN_ARCH >= 9
/**
 * Emit a Valhall depth/stencil descriptor at draw-time. The bulk of the
 * descriptor corresponds to a pipe_depth_stencil_alpha CSO and is packed at
 * CSO create time. However, the stencil reference values and shader
 * interactions are dynamic state. Pack only the dynamic state here and OR
 * together.
 */
static mali_ptr
panfrost_emit_depth_stencil(struct panfrost_batch *batch)
{
   struct panfrost_context *ctx = batch->ctx;
   const struct panfrost_zsa_state *zsa = ctx->depth_stencil;
   struct panfrost_rasterizer *rast = ctx->rasterizer;
   struct panfrost_compiled_shader *fs = ctx->prog[PIPE_SHADER_FRAGMENT];
   bool back_enab = zsa->base.stencil[1].enabled;

   struct panfrost_ptr T =
      pan_pool_alloc_desc(&batch->pool.base, DEPTH_STENCIL);
   struct mali_depth_stencil_packed dynamic;

   pan_pack(&dynamic, DEPTH_STENCIL, cfg) {
      cfg.front_reference_value = ctx->stencil_ref.ref_value[0];
      cfg.back_reference_value = ctx->stencil_ref.ref_value[back_enab ? 1 : 0];

      cfg.stencil_from_shader = fs->info.fs.writes_stencil;
      cfg.depth_source = pan_depth_source(&fs->info);

      cfg.depth_bias_enable = rast->base.offset_tri;
      cfg.depth_units = rast->base.offset_units * 2.0f;
      cfg.depth_factor = rast->base.offset_scale;
      cfg.depth_bias_clamp = rast->base.offset_clamp;
   }

   pan_merge(dynamic, zsa->desc, DEPTH_STENCIL);
   memcpy(T.cpu, &dynamic, pan_size(DEPTH_STENCIL));

   return T.gpu;
}

/**
 * Emit Valhall blend descriptor at draw-time. The descriptor itself is shared
 * with Bifrost, but the container data structure is simplified.
 */
static mali_ptr
panfrost_emit_blend_valhall(struct panfrost_batch *batch)
{
   unsigned rt_count = MAX2(batch->key.nr_cbufs, 1);

   struct panfrost_ptr T =
      pan_pool_alloc_desc_array(&batch->pool.base, rt_count, BLEND);

   mali_ptr blend_shaders[PIPE_MAX_COLOR_BUFS] = {0};
   panfrost_get_blend_shaders(batch, blend_shaders);

   panfrost_emit_blend(batch, T.cpu, blend_shaders);

   /* Precalculate for the per-draw path */
   bool has_blend_shader = false;

   for (unsigned i = 0; i < rt_count; ++i)
      has_blend_shader |= !!blend_shaders[i];

   batch->ctx->valhall_has_blend_shader = has_blend_shader;

   return T.gpu;
}

/**
 * Emit Valhall buffer descriptors for bound vertex buffers at draw-time.
 */
static mali_ptr
panfrost_emit_vertex_buffers(struct panfrost_batch *batch)
{
   struct panfrost_context *ctx = batch->ctx;
   unsigned buffer_count = util_last_bit(ctx->vb_mask);
   struct panfrost_ptr T =
      pan_pool_alloc_desc_array(&batch->pool.base, buffer_count, BUFFER);
   struct mali_buffer_packed *buffers = T.cpu;

   u_foreach_bit(i, ctx->vb_mask) {
      struct pipe_vertex_buffer vb = ctx->vertex_buffers[i];
      struct pipe_resource *prsrc = vb.buffer.resource;
      struct panfrost_resource *rsrc = pan_resource(prsrc);
      assert(!vb.is_user_buffer);

      panfrost_batch_read_rsrc(batch, rsrc, PIPE_SHADER_VERTEX);

      pan_pack(buffers + i, BUFFER, cfg) {
         cfg.address = rsrc->image.data.bo->ptr.gpu + vb.buffer_offset;

         cfg.size = prsrc->width0 - vb.buffer_offset;
      }
   }

   return T.gpu;
}

static mali_ptr
panfrost_emit_vertex_data(struct panfrost_batch *batch)
{
   struct panfrost_context *ctx = batch->ctx;
   struct panfrost_vertex_state *vtx = ctx->vertex;

   return pan_pool_upload_aligned(&batch->pool.base, vtx->attributes,
                                  vtx->num_elements * pan_size(ATTRIBUTE),
                                  pan_alignment(ATTRIBUTE));
}

static void panfrost_update_sampler_view(struct panfrost_sampler_view *view,
                                         struct pipe_context *pctx);

static mali_ptr
panfrost_emit_images(struct panfrost_batch *batch, enum pipe_shader_type stage)
{
   struct panfrost_context *ctx = batch->ctx;
   unsigned last_bit = util_last_bit(ctx->image_mask[stage]);

   struct panfrost_ptr T =
      pan_pool_alloc_desc_array(&batch->pool.base, last_bit, TEXTURE);

   struct mali_texture_packed *out = (struct mali_texture_packed *)T.cpu;

   for (int i = 0; i < last_bit; ++i) {
      struct pipe_image_view *image = &ctx->images[stage][i];

      if (!(ctx->image_mask[stage] & BITFIELD_BIT(i))) {
         memset(&out[i], 0, sizeof(out[i]));
         continue;
      }

      /* Construct a synthetic sampler view so we can use our usual
       * sampler view code for the actual descriptor packing.
       *
       * Use the batch pool for a transient allocation, rather than
       * allocating a long-lived descriptor.
       */
      struct panfrost_sampler_view view = {
         .base = util_image_to_sampler_view(image),
         .pool = &batch->pool,
      };

      /* If we specify a cube map, the hardware internally treat it as
       * a 2D array. Since cube maps as images can confuse our common
       * texturing code, explicitly use a 2D array.
       *
       * Similar concerns apply to 3D textures.
       */
      if (view.base.target == PIPE_BUFFER)
         view.base.target = PIPE_BUFFER;
      else
         view.base.target = PIPE_TEXTURE_2D_ARRAY;

      panfrost_update_sampler_view(&view, &ctx->base);
      out[i] = view.bifrost_descriptor;

      panfrost_track_image_access(batch, stage, image);
   }

   return T.gpu;
}
#endif

static mali_ptr
panfrost_map_constant_buffer_gpu(struct panfrost_batch *batch,
                                 enum pipe_shader_type st,
                                 struct panfrost_constant_buffer *buf,
                                 unsigned index)
{
   struct pipe_constant_buffer *cb = &buf->cb[index];
   struct panfrost_resource *rsrc = pan_resource(cb->buffer);

   if (rsrc) {
      panfrost_batch_read_rsrc(batch, rsrc, st);

      /* Alignment gauranteed by
       * PIPE_CAP_CONSTANT_BUFFER_OFFSET_ALIGNMENT */
      return rsrc->image.data.bo->ptr.gpu + cb->buffer_offset;
   } else if (cb->user_buffer) {
      return pan_pool_upload_aligned(&batch->pool.base,
                                     cb->user_buffer + cb->buffer_offset,
                                     cb->buffer_size, 16);
   } else {
      unreachable("No constant buffer");
   }
}

struct sysval_uniform {
   union {
      float f[4];
      int32_t i[4];
      uint32_t u[4];
      uint64_t du[2];
   };
};

static void
panfrost_upload_viewport_scale_sysval(struct panfrost_batch *batch,
                                      struct sysval_uniform *uniform)
{
   struct panfrost_context *ctx = batch->ctx;
   const struct pipe_viewport_state *vp = &ctx->pipe_viewport;

   uniform->f[0] = vp->scale[0];
   uniform->f[1] = vp->scale[1];
   uniform->f[2] = vp->scale[2];
}

static void
panfrost_upload_viewport_offset_sysval(struct panfrost_batch *batch,
                                       struct sysval_uniform *uniform)
{
   struct panfrost_context *ctx = batch->ctx;
   const struct pipe_viewport_state *vp = &ctx->pipe_viewport;

   uniform->f[0] = vp->translate[0];
   uniform->f[1] = vp->translate[1];
   uniform->f[2] = vp->translate[2];
}

static void
panfrost_upload_txs_sysval(struct panfrost_batch *batch,
                           enum pipe_shader_type st, unsigned int sysvalid,
                           struct sysval_uniform *uniform)
{
   struct panfrost_context *ctx = batch->ctx;
   unsigned texidx = PAN_SYSVAL_ID_TO_TXS_TEX_IDX(sysvalid);
   unsigned dim = PAN_SYSVAL_ID_TO_TXS_DIM(sysvalid);
   bool is_array = PAN_SYSVAL_ID_TO_TXS_IS_ARRAY(sysvalid);
   struct pipe_sampler_view *tex = &ctx->sampler_views[st][texidx]->base;

   assert(dim);

   if (tex->target == PIPE_BUFFER) {
      assert(dim == 1);
      uniform->i[0] = tex->u.buf.size / util_format_get_blocksize(tex->format);
      return;
   }

   uniform->i[0] = u_minify(tex->texture->width0, tex->u.tex.first_level);

   if (dim > 1)
      uniform->i[1] = u_minify(tex->texture->height0, tex->u.tex.first_level);

   if (dim > 2)
      uniform->i[2] = u_minify(tex->texture->depth0, tex->u.tex.first_level);

   if (is_array) {
      unsigned size = tex->texture->array_size;

      /* Internally, we store the number of 2D images (faces * array
       * size). Externally, we report the array size in terms of
       * complete cubes. So divide by the # of faces per cube.
       */
      if (tex->target == PIPE_TEXTURE_CUBE_ARRAY)
         size /= 6;

      uniform->i[dim] = size;
   }
}

static void
panfrost_upload_image_size_sysval(struct panfrost_batch *batch,
                                  enum pipe_shader_type st,
                                  unsigned int sysvalid,
                                  struct sysval_uniform *uniform)
{
   struct panfrost_context *ctx = batch->ctx;
   unsigned idx = PAN_SYSVAL_ID_TO_TXS_TEX_IDX(sysvalid);
   unsigned dim = PAN_SYSVAL_ID_TO_TXS_DIM(sysvalid);
   unsigned is_array = PAN_SYSVAL_ID_TO_TXS_IS_ARRAY(sysvalid);

   assert(dim && dim < 4);

   struct pipe_image_view *image = &ctx->images[st][idx];

   if (image->resource->target == PIPE_BUFFER) {
      unsigned blocksize = util_format_get_blocksize(image->format);
      uniform->i[0] = image->resource->width0 / blocksize;
      return;
   }

   uniform->i[0] = u_minify(image->resource->width0, image->u.tex.level);

   if (dim > 1)
      uniform->i[1] = u_minify(image->resource->height0, image->u.tex.level);

   if (dim > 2)
      uniform->i[2] = u_minify(image->resource->depth0, image->u.tex.level);

   if (is_array)
      uniform->i[dim] = image->resource->array_size;
}

static void
panfrost_upload_ssbo_sysval(struct panfrost_batch *batch,
                            enum pipe_shader_type st, unsigned ssbo_id,
                            struct sysval_uniform *uniform)
{
   struct panfrost_context *ctx = batch->ctx;

   assert(ctx->ssbo_mask[st] & (1 << ssbo_id));
   struct pipe_shader_buffer sb = ctx->ssbo[st][ssbo_id];

   /* Compute address */
   struct panfrost_resource *rsrc = pan_resource(sb.buffer);
   struct panfrost_bo *bo = rsrc->image.data.bo;

   panfrost_batch_write_rsrc(batch, rsrc, st);

   util_range_add(&rsrc->base, &rsrc->valid_buffer_range, sb.buffer_offset,
                  sb.buffer_size);

   /* Upload address and size as sysval */
   uniform->du[0] = bo->ptr.gpu + sb.buffer_offset;
   uniform->u[2] = sb.buffer_size;
}

static void
panfrost_upload_sampler_sysval(struct panfrost_batch *batch,
                               enum pipe_shader_type st, unsigned samp_idx,
                               struct sysval_uniform *uniform)
{
   struct panfrost_context *ctx = batch->ctx;
   struct pipe_sampler_state *sampl = &ctx->samplers[st][samp_idx]->base;

   uniform->f[0] = sampl->min_lod;
   uniform->f[1] = sampl->max_lod;
   uniform->f[2] = sampl->lod_bias;

   /* Even without any errata, Midgard represents "no mipmapping" as
    * fixing the LOD with the clamps; keep behaviour consistent. c.f.
    * panfrost_create_sampler_state which also explains our choice of
    * epsilon value (again to keep behaviour consistent) */

   if (sampl->min_mip_filter == PIPE_TEX_MIPFILTER_NONE)
      uniform->f[1] = uniform->f[0] + (1.0 / 256.0);
}

static void
panfrost_upload_num_work_groups_sysval(struct panfrost_batch *batch,
                                       struct sysval_uniform *uniform)
{
   struct panfrost_context *ctx = batch->ctx;

   uniform->u[0] = ctx->compute_grid->grid[0];
   uniform->u[1] = ctx->compute_grid->grid[1];
   uniform->u[2] = ctx->compute_grid->grid[2];
}

static void
panfrost_upload_local_group_size_sysval(struct panfrost_batch *batch,
                                        struct sysval_uniform *uniform)
{
   struct panfrost_context *ctx = batch->ctx;

   uniform->u[0] = ctx->compute_grid->block[0];
   uniform->u[1] = ctx->compute_grid->block[1];
   uniform->u[2] = ctx->compute_grid->block[2];
}

static void
panfrost_upload_work_dim_sysval(struct panfrost_batch *batch,
                                struct sysval_uniform *uniform)
{
   struct panfrost_context *ctx = batch->ctx;

   uniform->u[0] = ctx->compute_grid->work_dim;
}

/* Sample positions are pushed in a Bifrost specific format on Bifrost. On
 * Midgard, we emulate the Bifrost path with some extra arithmetic in the
 * shader, to keep the code as unified as possible. */

static void
panfrost_upload_sample_positions_sysval(struct panfrost_batch *batch,
                                        struct sysval_uniform *uniform)
{
   struct panfrost_context *ctx = batch->ctx;
   struct panfrost_device *dev = pan_device(ctx->base.screen);

   unsigned samples = util_framebuffer_get_num_samples(&batch->key);
   uniform->du[0] =
      panfrost_sample_positions(dev, panfrost_sample_pattern(samples));
}

static void
panfrost_upload_multisampled_sysval(struct panfrost_batch *batch,
                                    struct sysval_uniform *uniform)
{
   unsigned samples = util_framebuffer_get_num_samples(&batch->key);
   uniform->u[0] = (samples > 1) ? ~0 : 0;
}

#if PAN_ARCH >= 6
static void
panfrost_upload_rt_conversion_sysval(struct panfrost_batch *batch,
                                     unsigned size_and_rt,
                                     struct sysval_uniform *uniform)
{
   struct panfrost_context *ctx = batch->ctx;
   struct panfrost_device *dev = pan_device(ctx->base.screen);
   unsigned rt = size_and_rt & 0xF;
   unsigned size = size_and_rt >> 4;

   if (rt < batch->key.nr_cbufs && batch->key.cbufs[rt]) {
      enum pipe_format format = batch->key.cbufs[rt]->format;
      uniform->u[0] =
         GENX(pan_blend_get_internal_desc)(dev, format, rt, size, false) >> 32;
   } else {
      pan_pack(&uniform->u[0], INTERNAL_CONVERSION, cfg)
         cfg.memory_format = dev->formats[PIPE_FORMAT_NONE].hw;
   }
}
#endif

static unsigned
panfrost_xfb_offset(unsigned stride, struct pipe_stream_output_target *target)
{
   return target->buffer_offset + (pan_so_target(target)->offset * stride);
}

static void
panfrost_upload_sysvals(struct panfrost_batch *batch, void *ptr_cpu,
                        mali_ptr ptr_gpu, struct panfrost_compiled_shader *ss,
                        enum pipe_shader_type st)
{
   struct sysval_uniform *uniforms = ptr_cpu;

   for (unsigned i = 0; i < ss->sysvals.sysval_count; ++i) {
      int sysval = ss->sysvals.sysvals[i];

      switch (PAN_SYSVAL_TYPE(sysval)) {
      case PAN_SYSVAL_VIEWPORT_SCALE:
         panfrost_upload_viewport_scale_sysval(batch, &uniforms[i]);
         break;
      case PAN_SYSVAL_VIEWPORT_OFFSET:
         panfrost_upload_viewport_offset_sysval(batch, &uniforms[i]);
         break;
      case PAN_SYSVAL_TEXTURE_SIZE:
         panfrost_upload_txs_sysval(batch, st, PAN_SYSVAL_ID(sysval),
                                    &uniforms[i]);
         break;
      case PAN_SYSVAL_SSBO:
         panfrost_upload_ssbo_sysval(batch, st, PAN_SYSVAL_ID(sysval),
                                     &uniforms[i]);
         break;

      case PAN_SYSVAL_XFB: {
         unsigned buf = PAN_SYSVAL_ID(sysval);
         struct panfrost_compiled_shader *vs =
            batch->ctx->prog[PIPE_SHADER_VERTEX];
         struct pipe_stream_output_info *so = &vs->stream_output;
         unsigned stride = so->stride[buf] * 4;

         struct pipe_stream_output_target *target = NULL;
         if (buf < batch->ctx->streamout.num_targets)
            target = batch->ctx->streamout.targets[buf];

         if (!target) {
            /* Memory sink */
            uniforms[i].du[0] = 0x8ull << 60;
            break;
         }

         struct panfrost_resource *rsrc = pan_resource(target->buffer);
         unsigned offset = panfrost_xfb_offset(stride, target);

         util_range_add(&rsrc->base, &rsrc->valid_buffer_range, offset,
                        target->buffer_size - offset);

         panfrost_batch_write_rsrc(batch, rsrc, PIPE_SHADER_VERTEX);

         uniforms[i].du[0] = rsrc->image.data.bo->ptr.gpu + offset;
         break;
      }

      case PAN_SYSVAL_NUM_VERTICES:
         uniforms[i].u[0] = batch->ctx->vertex_count;
         break;

      case PAN_SYSVAL_NUM_WORK_GROUPS:
         for (unsigned j = 0; j < 3; j++) {
            batch->num_wg_sysval[j] =
               ptr_gpu + (i * sizeof(*uniforms)) + (j * 4);
         }
         panfrost_upload_num_work_groups_sysval(batch, &uniforms[i]);
         break;
      case PAN_SYSVAL_LOCAL_GROUP_SIZE:
         panfrost_upload_local_group_size_sysval(batch, &uniforms[i]);
         break;
      case PAN_SYSVAL_WORK_DIM:
         panfrost_upload_work_dim_sysval(batch, &uniforms[i]);
         break;
      case PAN_SYSVAL_SAMPLER:
         panfrost_upload_sampler_sysval(batch, st, PAN_SYSVAL_ID(sysval),
                                        &uniforms[i]);
         break;
      case PAN_SYSVAL_IMAGE_SIZE:
         panfrost_upload_image_size_sysval(batch, st, PAN_SYSVAL_ID(sysval),
                                           &uniforms[i]);
         break;
      case PAN_SYSVAL_SAMPLE_POSITIONS:
         panfrost_upload_sample_positions_sysval(batch, &uniforms[i]);
         break;
      case PAN_SYSVAL_MULTISAMPLED:
         panfrost_upload_multisampled_sysval(batch, &uniforms[i]);
         break;
#if PAN_ARCH >= 6
      case PAN_SYSVAL_RT_CONVERSION:
         panfrost_upload_rt_conversion_sysval(batch, PAN_SYSVAL_ID(sysval),
                                              &uniforms[i]);
         break;
#endif
      case PAN_SYSVAL_VERTEX_INSTANCE_OFFSETS:
         uniforms[i].u[0] = batch->ctx->offset_start;
         uniforms[i].u[1] = batch->ctx->base_vertex;
         uniforms[i].u[2] = batch->ctx->base_instance;
         break;
      case PAN_SYSVAL_DRAWID:
         uniforms[i].u[0] = batch->ctx->drawid;
         break;
      default:
         assert(0);
      }
   }
}

static const void *
panfrost_map_constant_buffer_cpu(struct panfrost_context *ctx,
                                 struct panfrost_constant_buffer *buf,
                                 unsigned index)
{
   struct pipe_constant_buffer *cb = &buf->cb[index];
   struct panfrost_resource *rsrc = pan_resource(cb->buffer);

   if (rsrc) {
      panfrost_bo_mmap(rsrc->image.data.bo);
      panfrost_flush_writer(ctx, rsrc, "CPU constant buffer mapping");
      panfrost_bo_wait(rsrc->image.data.bo, INT64_MAX, false);

      return rsrc->image.data.bo->ptr.cpu + cb->buffer_offset;
   } else if (cb->user_buffer) {
      return cb->user_buffer + cb->buffer_offset;
   } else
      unreachable("No constant buffer");
}

/* Emit a single UBO record. On Valhall, UBOs are dumb buffers and are
 * implemented with buffer descriptors in the resource table, sized in terms of
 * bytes. On Bifrost and older, UBOs have special uniform buffer data
 * structure, sized in terms of entries.
 */
static void
panfrost_emit_ubo(void *base, unsigned index, mali_ptr address, size_t size)
{
#if PAN_ARCH >= 9
   struct mali_buffer_packed *out = base;

   pan_pack(out + index, BUFFER, cfg) {
      cfg.size = size;
      cfg.address = address;
   }
#else
   struct mali_uniform_buffer_packed *out = base;

   /* Issue (57) for the ARB_uniform_buffer_object spec says that
    * the buffer can be larger than the uniform data inside it,
    * so clamp ubo size to what hardware supports. */

   pan_pack(out + index, UNIFORM_BUFFER, cfg) {
      cfg.entries = MIN2(DIV_ROUND_UP(size, 16), 1 << 12);
      cfg.pointer = address;
   }
#endif
}

static mali_ptr
panfrost_emit_const_buf(struct panfrost_batch *batch,
                        enum pipe_shader_type stage, unsigned *buffer_count,
                        mali_ptr *push_constants, unsigned *pushed_words)
{
   struct panfrost_context *ctx = batch->ctx;
   struct panfrost_constant_buffer *buf = &ctx->constant_buffer[stage];
   struct panfrost_compiled_shader *ss = ctx->prog[stage];

   if (!ss)
      return 0;

   /* Allocate room for the sysval and the uniforms */
   size_t sys_size = sizeof(float) * 4 * ss->sysvals.sysval_count;
   struct panfrost_ptr transfer =
      pan_pool_alloc_aligned(&batch->pool.base, sys_size, 16);

   /* Upload sysvals requested by the shader */
   uint8_t *sysvals = alloca(sys_size);
   panfrost_upload_sysvals(batch, sysvals, transfer.gpu, ss, stage);
   memcpy(transfer.cpu, sysvals, sys_size);

   /* Next up, attach UBOs. UBO count includes gaps but no sysval UBO */
   struct panfrost_compiled_shader *shader = ctx->prog[stage];
   unsigned ubo_count = shader->info.ubo_count - (sys_size ? 1 : 0);
   unsigned sysval_ubo = sys_size ? ubo_count : ~0;
   struct panfrost_ptr ubos = {0};

#if PAN_ARCH >= 9
   ubos = pan_pool_alloc_desc_array(&batch->pool.base, ubo_count + 1, BUFFER);
#else
   ubos = pan_pool_alloc_desc_array(&batch->pool.base, ubo_count + 1,
                                    UNIFORM_BUFFER);
#endif

   if (buffer_count)
      *buffer_count = ubo_count + (sys_size ? 1 : 0);

   /* Upload sysval as a final UBO */

   if (sys_size)
      panfrost_emit_ubo(ubos.cpu, ubo_count, transfer.gpu, sys_size);

   /* The rest are honest-to-goodness UBOs */

   u_foreach_bit(ubo, ss->info.ubo_mask & buf->enabled_mask) {
      size_t usz = buf->cb[ubo].buffer_size;
      mali_ptr address = 0;

      if (usz > 0) {
         address = panfrost_map_constant_buffer_gpu(batch, stage, buf, ubo);
      }

      panfrost_emit_ubo(ubos.cpu, ubo, address, usz);
   }

   if (pushed_words)
      *pushed_words = ss->info.push.count;

   if (ss->info.push.count == 0)
      return ubos.gpu;

   /* Copy push constants required by the shader */
   struct panfrost_ptr push_transfer =
      pan_pool_alloc_aligned(&batch->pool.base, ss->info.push.count * 4, 16);

   uint32_t *push_cpu = (uint32_t *)push_transfer.cpu;
   *push_constants = push_transfer.gpu;

   for (unsigned i = 0; i < ss->info.push.count; ++i) {
      struct panfrost_ubo_word src = ss->info.push.words[i];

      if (src.ubo == sysval_ubo) {
         unsigned sysval_idx = src.offset / 16;
         unsigned sysval_comp = (src.offset % 16) / 4;
         unsigned sysval_type =
            PAN_SYSVAL_TYPE(ss->sysvals.sysvals[sysval_idx]);
         mali_ptr ptr = push_transfer.gpu + (4 * i);

         if (sysval_type == PAN_SYSVAL_NUM_WORK_GROUPS)
            batch->num_wg_sysval[sysval_comp] = ptr;
      }
      /* Map the UBO, this should be cheap. For some buffers this may
       * read from write-combine memory which is slow, though :-(
       */
      const void *mapped_ubo =
         (src.ubo == sysval_ubo)
            ? sysvals
            : panfrost_map_constant_buffer_cpu(ctx, buf, src.ubo);

      /* TODO: Is there any benefit to combining ranges */
      memcpy(push_cpu + i, (uint8_t *)mapped_ubo + src.offset, 4);
   }

   return ubos.gpu;
}

/*
 * Choose the number of WLS instances to allocate. This must be a power-of-two.
 * The number of WLS instances limits the number of concurrent tasks on a given
 * shader core, setting to the (rounded) total number of tasks avoids any
 * throttling. Smaller values save memory at the expense of possible throttling.
 *
 * With indirect dispatch, we don't know at launch-time how many tasks will be
 * needed, so we use a conservative value that's unlikely to cause slowdown in
 * practice without wasting too much memory.
 */
static unsigned
panfrost_choose_wls_instance_count(const struct pipe_grid_info *grid)
{
   if (grid->indirect) {
      /* May need tuning in the future, conservative guess */
      return 128;
   } else {
      return util_next_power_of_two(grid->grid[0]) *
             util_next_power_of_two(grid->grid[1]) *
             util_next_power_of_two(grid->grid[2]);
   }
}

static mali_ptr
panfrost_emit_shared_memory(struct panfrost_batch *batch,
                            const struct pipe_grid_info *grid)
{
   struct panfrost_context *ctx = batch->ctx;
   struct panfrost_device *dev = pan_device(ctx->base.screen);
   struct panfrost_compiled_shader *ss = ctx->prog[PIPE_SHADER_COMPUTE];
   struct panfrost_ptr t =
      pan_pool_alloc_desc(&batch->pool.base, LOCAL_STORAGE);

   struct pan_tls_info info = {
      .tls.size = ss->info.tls_size,
      .wls.size = ss->info.wls_size + grid->variable_shared_mem,
      .wls.instances = panfrost_choose_wls_instance_count(grid),
   };

   if (ss->info.tls_size) {
      struct panfrost_bo *bo = panfrost_batch_get_scratchpad(
         batch, ss->info.tls_size, dev->thread_tls_alloc, dev->core_id_range);
      info.tls.ptr = bo->ptr.gpu;
   }

   if (info.wls.size) {
      unsigned size = pan_wls_adjust_size(info.wls.size) * info.wls.instances *
                      dev->core_id_range;

      struct panfrost_bo *bo = panfrost_batch_get_shared_memory(batch, size, 1);

      info.wls.ptr = bo->ptr.gpu;
   }

   GENX(pan_emit_tls)(&info, t.cpu);
   return t.gpu;
}

#if PAN_ARCH <= 5
static mali_ptr
panfrost_get_tex_desc(struct panfrost_batch *batch, enum pipe_shader_type st,
                      struct panfrost_sampler_view *view)
{
   if (!view)
      return (mali_ptr)0;

   struct pipe_sampler_view *pview = &view->base;
   struct panfrost_resource *rsrc = pan_resource(pview->texture);

   panfrost_batch_read_rsrc(batch, rsrc, st);
   panfrost_batch_add_bo(batch, view->state.bo, st);

   return view->state.gpu;
}
#endif

static void
panfrost_create_sampler_view_bo(struct panfrost_sampler_view *so,
                                struct pipe_context *pctx,
                                struct pipe_resource *texture)
{
   struct panfrost_device *device = pan_device(pctx->screen);
   struct panfrost_context *ctx = pan_context(pctx);
   struct panfrost_resource *prsrc = (struct panfrost_resource *)texture;
   enum pipe_format format = so->base.format;
   assert(prsrc->image.data.bo);

   /* Format to access the stencil/depth portion of a Z32_S8 texture */
   if (format == PIPE_FORMAT_X32_S8X24_UINT) {
      assert(prsrc->separate_stencil);
      texture = &prsrc->separate_stencil->base;
      prsrc = (struct panfrost_resource *)texture;
      format = texture->format;
   } else if (format == PIPE_FORMAT_Z32_FLOAT_S8X24_UINT) {
      format = PIPE_FORMAT_Z32_FLOAT;
   }

   so->texture_bo = prsrc->image.data.bo->ptr.gpu;
   so->modifier = prsrc->image.layout.modifier;

   /* MSAA only supported for 2D textures */

   assert(texture->nr_samples <= 1 || so->base.target == PIPE_TEXTURE_2D ||
          so->base.target == PIPE_TEXTURE_2D_ARRAY);

   enum mali_texture_dimension type =
      panfrost_translate_texture_dimension(so->base.target);

   bool is_buffer = (so->base.target == PIPE_BUFFER);

   unsigned first_level = is_buffer ? 0 : so->base.u.tex.first_level;
   unsigned last_level = is_buffer ? 0 : so->base.u.tex.last_level;
   unsigned first_layer = is_buffer ? 0 : so->base.u.tex.first_layer;
   unsigned last_layer = is_buffer ? 0 : so->base.u.tex.last_layer;
   unsigned buf_offset = is_buffer ? so->base.u.buf.offset : 0;
   unsigned buf_size =
      (is_buffer ? so->base.u.buf.size : 0) / util_format_get_blocksize(format);

   if (so->base.target == PIPE_TEXTURE_3D) {
      first_layer /= prsrc->image.layout.depth;
      last_layer /= prsrc->image.layout.depth;
      assert(!first_layer && !last_layer);
   }

   struct pan_image_view iview = {
      .format = format,
      .dim = type,
      .first_level = first_level,
      .last_level = last_level,
      .first_layer = first_layer,
      .last_layer = last_layer,
      .swizzle =
         {
            so->base.swizzle_r,
            so->base.swizzle_g,
            so->base.swizzle_b,
            so->base.swizzle_a,
         },
      .planes = {NULL},
      .buf.offset = buf_offset,
      .buf.size = buf_size,
   };

   panfrost_set_image_view_planes(&iview, texture);

   unsigned size = (PAN_ARCH <= 5 ? pan_size(TEXTURE) : 0) +
                   GENX(panfrost_estimate_texture_payload_size)(&iview);

   struct panfrost_pool *pool = so->pool ?: &ctx->descs;
   struct panfrost_ptr payload = pan_pool_alloc_aligned(&pool->base, size, 64);
   so->state = panfrost_pool_take_ref(&ctx->descs, payload.gpu);

   void *tex = (PAN_ARCH >= 6) ? &so->bifrost_descriptor : payload.cpu;

   if (PAN_ARCH <= 5) {
      payload.cpu += pan_size(TEXTURE);
      payload.gpu += pan_size(TEXTURE);
   }

   GENX(panfrost_new_texture)(device, &iview, tex, &payload);
}

static void
panfrost_update_sampler_view(struct panfrost_sampler_view *view,
                             struct pipe_context *pctx)
{
   struct panfrost_resource *rsrc = pan_resource(view->base.texture);
   if (view->texture_bo != rsrc->image.data.bo->ptr.gpu ||
       view->modifier != rsrc->image.layout.modifier) {
      panfrost_bo_unreference(view->state.bo);
      panfrost_create_sampler_view_bo(view, pctx, &rsrc->base);
   }
}

#if PAN_ARCH >= 6
static void
panfrost_emit_null_texture(struct mali_texture_packed *out)

{
   /* Annoyingly, an all zero texture descriptor is not valid and will raise
    * a DATA_INVALID_FAULT if you try to texture it, instead of returning
    * 0000s! Fill in with sometthing that will behave robustly.
    */
   pan_pack(out, TEXTURE, cfg) {
      cfg.dimension = MALI_TEXTURE_DIMENSION_2D;
      cfg.width = 1;
      cfg.height = 1;
      cfg.depth = 1;
      cfg.array_size = 1;
      cfg.format = MALI_PACK_FMT(CONSTANT, 0000, L);
#if PAN_ARCH <= 7
      cfg.texel_ordering = MALI_TEXTURE_LAYOUT_LINEAR;
#endif
   }
}
#endif

static mali_ptr
panfrost_emit_texture_descriptors(struct panfrost_batch *batch,
                                  enum pipe_shader_type stage)
{
   struct panfrost_context *ctx = batch->ctx;

   unsigned actual_count = ctx->sampler_view_count[stage];
   unsigned needed_count = ctx->prog[stage]->info.texture_count;
   unsigned alloc_count = MAX2(actual_count, needed_count);

   if (!alloc_count)
      return 0;

#if PAN_ARCH >= 6
   struct panfrost_ptr T =
      pan_pool_alloc_desc_array(&batch->pool.base, alloc_count, TEXTURE);
   struct mali_texture_packed *out = (struct mali_texture_packed *)T.cpu;

   for (int i = 0; i < actual_count; ++i) {
      struct panfrost_sampler_view *view = ctx->sampler_views[stage][i];

      if (!view) {
         panfrost_emit_null_texture(&out[i]);
         continue;
      }

      struct pipe_sampler_view *pview = &view->base;
      struct panfrost_resource *rsrc = pan_resource(pview->texture);

      panfrost_update_sampler_view(view, &ctx->base);
      out[i] = view->bifrost_descriptor;

      panfrost_batch_read_rsrc(batch, rsrc, stage);
      panfrost_batch_add_bo(batch, view->state.bo, stage);
   }

   for (int i = actual_count; i < needed_count; ++i)
      panfrost_emit_null_texture(&out[i]);

   return T.gpu;
#else
   uint64_t trampolines[PIPE_MAX_SHADER_SAMPLER_VIEWS];

   for (int i = 0; i < actual_count; ++i) {
      struct panfrost_sampler_view *view = ctx->sampler_views[stage][i];

      if (!view) {
         trampolines[i] = 0;
         continue;
      }

      panfrost_update_sampler_view(view, &ctx->base);

      trampolines[i] = panfrost_get_tex_desc(batch, stage, view);
   }

   for (int i = actual_count; i < needed_count; ++i)
      trampolines[i] = 0;

   return pan_pool_upload_aligned(&batch->pool.base, trampolines,
                                  sizeof(uint64_t) * alloc_count,
                                  sizeof(uint64_t));
#endif
}

static mali_ptr
panfrost_upload_wa_sampler(struct panfrost_batch *batch)
{
   struct panfrost_ptr T = pan_pool_alloc_desc(&batch->pool.base, SAMPLER);
   pan_pack(T.cpu, SAMPLER, cfg)
      ;
   return T.gpu;
}

static mali_ptr
panfrost_emit_sampler_descriptors(struct panfrost_batch *batch,
                                  enum pipe_shader_type stage)
{
   struct panfrost_context *ctx = batch->ctx;

   /* We always need at least 1 sampler for txf to work */
   if (!ctx->sampler_count[stage])
      return panfrost_upload_wa_sampler(batch);

   struct panfrost_ptr T = pan_pool_alloc_desc_array(
      &batch->pool.base, ctx->sampler_count[stage], SAMPLER);
   struct mali_sampler_packed *out = (struct mali_sampler_packed *)T.cpu;

   for (unsigned i = 0; i < ctx->sampler_count[stage]; ++i) {
      struct panfrost_sampler_state *st = ctx->samplers[stage][i];

      out[i] = st ? st->hw : (struct mali_sampler_packed){0};
   }

   return T.gpu;
}

#if PAN_ARCH <= 7
/* Packs all image attribute descs and attribute buffer descs.
 * `first_image_buf_index` must be the index of the first image attribute buffer
 * descriptor.
 */
static void
emit_image_attribs(struct panfrost_context *ctx, enum pipe_shader_type shader,
                   struct mali_attribute_packed *attribs, unsigned first_buf)
{
   struct panfrost_device *dev = pan_device(ctx->base.screen);
   unsigned last_bit = util_last_bit(ctx->image_mask[shader]);

   for (unsigned i = 0; i < last_bit; ++i) {
      enum pipe_format format = ctx->images[shader][i].format;

      pan_pack(attribs + i, ATTRIBUTE, cfg) {
         /* Continuation record means 2 buffers per image */
         cfg.buffer_index = first_buf + (i * 2);
         cfg.offset_enable = (PAN_ARCH <= 5);
         cfg.format = dev->formats[format].hw;
      }
   }
}

static enum mali_attribute_type
pan_modifier_to_attr_type(uint64_t modifier)
{
   switch (modifier) {
   case DRM_FORMAT_MOD_LINEAR:
      return MALI_ATTRIBUTE_TYPE_3D_LINEAR;
   case DRM_FORMAT_MOD_ARM_16X16_BLOCK_U_INTERLEAVED:
      return MALI_ATTRIBUTE_TYPE_3D_INTERLEAVED;
   default:
      unreachable("Invalid modifier for attribute record");
   }
}

static void
emit_image_bufs(struct panfrost_batch *batch, enum pipe_shader_type shader,
                struct mali_attribute_buffer_packed *bufs,
                unsigned first_image_buf_index)
{
   struct panfrost_context *ctx = batch->ctx;
   unsigned last_bit = util_last_bit(ctx->image_mask[shader]);

   for (unsigned i = 0; i < last_bit; ++i) {
      struct pipe_image_view *image = &ctx->images[shader][i];

      if (!(ctx->image_mask[shader] & (1 << i)) ||
          !(image->shader_access & PIPE_IMAGE_ACCESS_READ_WRITE)) {
         /* Unused image bindings */
         pan_pack(bufs + (i * 2), ATTRIBUTE_BUFFER, cfg)
            ;
         pan_pack(bufs + (i * 2) + 1, ATTRIBUTE_BUFFER, cfg)
            ;
         continue;
      }

      struct panfrost_resource *rsrc = pan_resource(image->resource);

      /* TODO: MSAA */
      assert(image->resource->nr_samples <= 1 && "MSAA'd images not supported");

      bool is_3d = rsrc->base.target == PIPE_TEXTURE_3D;
      bool is_buffer = rsrc->base.target == PIPE_BUFFER;

      unsigned offset = is_buffer ? image->u.buf.offset
                                  : panfrost_texture_offset(
                                       &rsrc->image.layout, image->u.tex.level,
                                       is_3d ? 0 : image->u.tex.first_layer,
                                       is_3d ? image->u.tex.first_layer : 0);

      panfrost_track_image_access(batch, shader, image);

      pan_pack(bufs + (i * 2), ATTRIBUTE_BUFFER, cfg) {
         cfg.type = pan_modifier_to_attr_type(rsrc->image.layout.modifier);
         cfg.pointer = rsrc->image.data.bo->ptr.gpu + offset;
         cfg.stride = util_format_get_blocksize(image->format);
         cfg.size = panfrost_bo_size(rsrc->image.data.bo) - offset;
      }

      if (is_buffer) {
         pan_pack(bufs + (i * 2) + 1, ATTRIBUTE_BUFFER_CONTINUATION_3D, cfg) {
            cfg.s_dimension =
               rsrc->base.width0 / util_format_get_blocksize(image->format);
            cfg.t_dimension = cfg.r_dimension = 1;
         }

         continue;
      }

      pan_pack(bufs + (i * 2) + 1, ATTRIBUTE_BUFFER_CONTINUATION_3D, cfg) {
         unsigned level = image->u.tex.level;

         cfg.s_dimension = u_minify(rsrc->base.width0, level);
         cfg.t_dimension = u_minify(rsrc->base.height0, level);
         cfg.r_dimension =
            is_3d ? u_minify(rsrc->base.depth0, level)
                  : image->u.tex.last_layer - image->u.tex.first_layer + 1;

         cfg.row_stride = rsrc->image.layout.slices[level].row_stride;

         if (rsrc->base.target != PIPE_TEXTURE_2D) {
            cfg.slice_stride =
               panfrost_get_layer_stride(&rsrc->image.layout, level);
         }
      }
   }
}

static mali_ptr
panfrost_emit_image_attribs(struct panfrost_batch *batch, mali_ptr *buffers,
                            enum pipe_shader_type type)
{
   struct panfrost_context *ctx = batch->ctx;
   struct panfrost_compiled_shader *shader = ctx->prog[type];

   if (!shader->info.attribute_count) {
      *buffers = 0;
      return 0;
   }

   /* Images always need a MALI_ATTRIBUTE_BUFFER_CONTINUATION_3D */
   unsigned attr_count = shader->info.attribute_count;
   unsigned buf_count = (attr_count * 2) + (PAN_ARCH >= 6 ? 1 : 0);

   struct panfrost_ptr bufs =
      pan_pool_alloc_desc_array(&batch->pool.base, buf_count, ATTRIBUTE_BUFFER);

   struct panfrost_ptr attribs =
      pan_pool_alloc_desc_array(&batch->pool.base, attr_count, ATTRIBUTE);

   emit_image_attribs(ctx, type, attribs.cpu, 0);
   emit_image_bufs(batch, type, bufs.cpu, 0);

   /* We need an empty attrib buf to stop the prefetching on Bifrost */
#if PAN_ARCH >= 6
   pan_pack(bufs.cpu + ((buf_count - 1) * pan_size(ATTRIBUTE_BUFFER)),
            ATTRIBUTE_BUFFER, cfg)
      ;
#endif

   *buffers = bufs.gpu;
   return attribs.gpu;
}

static mali_ptr
panfrost_emit_vertex_data(struct panfrost_batch *batch, mali_ptr *buffers)
{
   struct panfrost_context *ctx = batch->ctx;
   struct panfrost_vertex_state *so = ctx->vertex;
   struct panfrost_compiled_shader *vs = ctx->prog[PIPE_SHADER_VERTEX];
   bool instanced = ctx->instance_count > 1;
   uint32_t image_mask = ctx->image_mask[PIPE_SHADER_VERTEX];
   unsigned nr_images = util_last_bit(image_mask);

   /* Worst case: everything is NPOT, which is only possible if instancing
    * is enabled. Otherwise single record is gauranteed.
    * Also, we allocate more memory than what's needed here if either instancing
    * is enabled or images are present, this can be improved. */
   unsigned bufs_per_attrib = (instanced || nr_images > 0) ? 2 : 1;
   unsigned nr_bufs =
      ((so->nr_bufs + nr_images) * bufs_per_attrib) + (PAN_ARCH >= 6 ? 1 : 0);

   unsigned count = vs->info.attribute_count;

   struct panfrost_compiled_shader *xfb =
      ctx->uncompiled[PIPE_SHADER_VERTEX]->xfb;

   if (xfb)
      count = MAX2(count, xfb->info.attribute_count);

#if PAN_ARCH <= 5
   /* Midgard needs vertexid/instanceid handled specially */
   bool special_vbufs = count >= PAN_VERTEX_ID;

   if (special_vbufs)
      nr_bufs += 2;
#endif

   if (!nr_bufs) {
      *buffers = 0;
      return 0;
   }

   struct panfrost_ptr S =
      pan_pool_alloc_desc_array(&batch->pool.base, nr_bufs, ATTRIBUTE_BUFFER);
   struct panfrost_ptr T =
      pan_pool_alloc_desc_array(&batch->pool.base, count, ATTRIBUTE);

   struct mali_attribute_buffer_packed *bufs =
      (struct mali_attribute_buffer_packed *)S.cpu;

   struct mali_attribute_packed *out = (struct mali_attribute_packed *)T.cpu;

   unsigned attrib_to_buffer[PIPE_MAX_ATTRIBS] = {0};
   unsigned k = 0;

   for (unsigned i = 0; i < so->nr_bufs; ++i) {
      unsigned vbi = so->buffers[i].vbi;
      unsigned divisor = so->buffers[i].divisor;
      attrib_to_buffer[i] = k;

      if (!(ctx->vb_mask & (1 << vbi)))
         continue;

      struct pipe_vertex_buffer *buf = &ctx->vertex_buffers[vbi];
      struct panfrost_resource *rsrc;

      rsrc = pan_resource(buf->buffer.resource);
      if (!rsrc)
         continue;

      panfrost_batch_read_rsrc(batch, rsrc, PIPE_SHADER_VERTEX);

      /* Mask off lower bits, see offset fixup below */
      mali_ptr raw_addr = rsrc->image.data.bo->ptr.gpu + buf->buffer_offset;
      mali_ptr addr = raw_addr & ~63;

      /* Since we advanced the base pointer, we shrink the buffer
       * size, but add the offset we subtracted */
      unsigned size =
         rsrc->base.width0 + (raw_addr - addr) - buf->buffer_offset;

      /* When there is a divisor, the hardware-level divisor is
       * the product of the instance divisor and the padded count */
      unsigned stride = so->strides[vbi];
      unsigned hw_divisor = ctx->padded_count * divisor;

      if (ctx->instance_count <= 1) {
         /* Per-instance would be every attribute equal */
         if (divisor)
            stride = 0;

         pan_pack(bufs + k, ATTRIBUTE_BUFFER, cfg) {
            cfg.pointer = addr;
            cfg.stride = stride;
            cfg.size = size;
         }
      } else if (!divisor) {
         pan_pack(bufs + k, ATTRIBUTE_BUFFER, cfg) {
            cfg.type = MALI_ATTRIBUTE_TYPE_1D_MODULUS;
            cfg.pointer = addr;
            cfg.stride = stride;
            cfg.size = size;
            cfg.divisor = ctx->padded_count;
         }
      } else if (util_is_power_of_two_or_zero(hw_divisor)) {
         pan_pack(bufs + k, ATTRIBUTE_BUFFER, cfg) {
            cfg.type = MALI_ATTRIBUTE_TYPE_1D_POT_DIVISOR;
            cfg.pointer = addr;
            cfg.stride = stride;
            cfg.size = size;
            cfg.divisor_r = __builtin_ctz(hw_divisor);
         }

      } else {
         unsigned shift = 0, extra_flags = 0;

         unsigned magic_divisor =
            panfrost_compute_magic_divisor(hw_divisor, &shift, &extra_flags);

         /* Records with continuations must be aligned */
         k = ALIGN_POT(k, 2);
         attrib_to_buffer[i] = k;

         pan_pack(bufs + k, ATTRIBUTE_BUFFER, cfg) {
            cfg.type = MALI_ATTRIBUTE_TYPE_1D_NPOT_DIVISOR;
            cfg.pointer = addr;
            cfg.stride = stride;
            cfg.size = size;

            cfg.divisor_r = shift;
            cfg.divisor_e = extra_flags;
         }

         pan_pack(bufs + k + 1, ATTRIBUTE_BUFFER_CONTINUATION_NPOT, cfg) {
            cfg.divisor_numerator = magic_divisor;
            cfg.divisor = divisor;
         }

         ++k;
      }

      ++k;
   }

#if PAN_ARCH <= 5
   /* Add special gl_VertexID/gl_InstanceID buffers */
   if (special_vbufs) {
      panfrost_vertex_id(ctx->padded_count, &bufs[k], ctx->instance_count > 1);

      pan_pack(out + PAN_VERTEX_ID, ATTRIBUTE, cfg) {
         cfg.buffer_index = k++;
         cfg.format = so->formats[PAN_VERTEX_ID];
      }

      panfrost_instance_id(ctx->padded_count, &bufs[k],
                           ctx->instance_count > 1);

      pan_pack(out + PAN_INSTANCE_ID, ATTRIBUTE, cfg) {
         cfg.buffer_index = k++;
         cfg.format = so->formats[PAN_INSTANCE_ID];
      }
   }
#endif

   if (nr_images) {
      k = ALIGN_POT(k, 2);
      emit_image_attribs(ctx, PIPE_SHADER_VERTEX, out + so->num_elements, k);
      emit_image_bufs(batch, PIPE_SHADER_VERTEX, bufs + k, k);
      k += (util_last_bit(ctx->image_mask[PIPE_SHADER_VERTEX]) * 2);
   }

#if PAN_ARCH >= 6
   /* We need an empty attrib buf to stop the prefetching on Bifrost */
   pan_pack(&bufs[k], ATTRIBUTE_BUFFER, cfg)
      ;
#endif

   /* Attribute addresses require 64-byte alignment, so let:
    *
    *      base' = base & ~63 = base - (base & 63)
    *      offset' = offset + (base & 63)
    *
    * Since base' + offset' = base + offset, these are equivalent
    * addressing modes and now base is 64 aligned.
    */

   /* While these are usually equal, they are not required to be. In some
    * cases, u_blitter passes too high a value for num_elements.
    */
   assert(vs->info.attributes_read_count <= so->num_elements);

   for (unsigned i = 0; i < vs->info.attributes_read_count; ++i) {
      unsigned vbi = so->pipe[i].vertex_buffer_index;
      struct pipe_vertex_buffer *buf = &ctx->vertex_buffers[vbi];

      /* BOs are aligned; just fixup for buffer_offset */
      signed src_offset = so->pipe[i].src_offset;
      src_offset += (buf->buffer_offset & 63);

      /* Base instance offset */
      if (ctx->base_instance && so->pipe[i].instance_divisor) {
         src_offset += (ctx->base_instance * so->pipe[i].src_stride) /
                       so->pipe[i].instance_divisor;
      }

      /* Also, somewhat obscurely per-instance data needs to be
       * offset in response to a delayed start in an indexed draw */

      if (so->pipe[i].instance_divisor && ctx->instance_count > 1)
         src_offset -= so->pipe[i].src_stride * ctx->offset_start;

      pan_pack(out + i, ATTRIBUTE, cfg) {
         cfg.buffer_index = attrib_to_buffer[so->element_buffer[i]];
         cfg.format = so->formats[i];
         cfg.offset = src_offset;
      }
   }

   *buffers = S.gpu;
   return T.gpu;
}

static mali_ptr
panfrost_emit_varyings(struct panfrost_batch *batch,
                       struct mali_attribute_buffer_packed *slot,
                       unsigned stride, unsigned count)
{
   unsigned size = stride * count;
   mali_ptr ptr =
      pan_pool_alloc_aligned(&batch->invisible_pool.base, size, 64).gpu;

   pan_pack(slot, ATTRIBUTE_BUFFER, cfg) {
      cfg.stride = stride;
      cfg.size = size;
      cfg.pointer = ptr;
   }

   return ptr;
}

/* Given a varying, figure out which index it corresponds to */

static inline unsigned
pan_varying_index(unsigned present, enum pan_special_varying v)
{
   return util_bitcount(present & BITFIELD_MASK(v));
}

/* Determines which varying buffers are required */

static inline unsigned
pan_varying_present(const struct panfrost_device *dev,
                    struct pan_shader_info *producer,
                    struct pan_shader_info *consumer, uint16_t point_coord_mask)
{
   /* At the moment we always emit general and position buffers. Not
    * strictly necessary but usually harmless */

   unsigned present =
      BITFIELD_BIT(PAN_VARY_GENERAL) | BITFIELD_BIT(PAN_VARY_POSITION);

   /* Enable special buffers by the shader info */

   if (producer->vs.writes_point_size)
      present |= BITFIELD_BIT(PAN_VARY_PSIZ);

#if PAN_ARCH <= 5
   /* On Midgard, these exist as real varyings. Later architectures use
    * LD_VAR_SPECIAL reads instead. */

   if (consumer->fs.reads_point_coord)
      present |= BITFIELD_BIT(PAN_VARY_PNTCOORD);

   if (consumer->fs.reads_face)
      present |= BITFIELD_BIT(PAN_VARY_FACE);

   if (consumer->fs.reads_frag_coord)
      present |= BITFIELD_BIT(PAN_VARY_FRAGCOORD);

   /* Also, if we have a point sprite, we need a point coord buffer */

   for (unsigned i = 0; i < consumer->varyings.input_count; i++) {
      gl_varying_slot loc = consumer->varyings.input[i].location;

      if (util_varying_is_point_coord(loc, point_coord_mask))
         present |= BITFIELD_BIT(PAN_VARY_PNTCOORD);
   }
#endif

   return present;
}

/* Emitters for varying records */

static void
pan_emit_vary(const struct panfrost_device *dev,
              struct mali_attribute_packed *out, unsigned buffer_index,
              mali_pixel_format format, unsigned offset)
{
   pan_pack(out, ATTRIBUTE, cfg) {
      cfg.buffer_index = buffer_index;
      cfg.offset_enable = (PAN_ARCH <= 5);
      cfg.format = format;
      cfg.offset = offset;
   }
}

/* Special records */

/* clang-format off */
static const struct {
   unsigned components;
   enum mali_format format;
} pan_varying_formats[PAN_VARY_MAX] = {
   [PAN_VARY_POSITION]  = { 4, MALI_SNAP_4   },
   [PAN_VARY_PSIZ]      = { 1, MALI_R16F     },
   [PAN_VARY_PNTCOORD]  = { 4, MALI_RGBA32F  },
   [PAN_VARY_FACE]      = { 1, MALI_R32I     },
   [PAN_VARY_FRAGCOORD] = { 4, MALI_RGBA32F  },
};
/* clang-format on */

static mali_pixel_format
pan_special_format(const struct panfrost_device *dev,
                   enum pan_special_varying buf)
{
   assert(buf < PAN_VARY_MAX);
   mali_pixel_format format = (pan_varying_formats[buf].format << 12);

#if PAN_ARCH <= 6
   unsigned nr = pan_varying_formats[buf].components;
   format |= panfrost_get_default_swizzle(nr);
#endif

   return format;
}

static void
pan_emit_vary_special(const struct panfrost_device *dev,
                      struct mali_attribute_packed *out, unsigned present,
                      enum pan_special_varying buf)
{
   pan_emit_vary(dev, out, pan_varying_index(present, buf),
                 pan_special_format(dev, buf), 0);
}

/* Negative indicates a varying is not found */

static signed
pan_find_vary(const struct pan_shader_varying *vary, unsigned vary_count,
              unsigned loc)
{
   for (unsigned i = 0; i < vary_count; ++i) {
      if (vary[i].location == loc)
         return i;
   }

   return -1;
}

/* Assign varying locations for the general buffer. Returns the calculated
 * per-vertex stride, and outputs offsets into the passed array. Negative
 * offset indicates a varying is not used. */

static unsigned
pan_assign_varyings(const struct panfrost_device *dev,
                    struct pan_shader_info *producer,
                    struct pan_shader_info *consumer, signed *offsets)
{
   unsigned producer_count = producer->varyings.output_count;
   unsigned consumer_count = consumer->varyings.input_count;

   const struct pan_shader_varying *producer_vars = producer->varyings.output;
   const struct pan_shader_varying *consumer_vars = consumer->varyings.input;

   unsigned stride = 0;

   for (unsigned i = 0; i < producer_count; ++i) {
      signed loc = pan_find_vary(consumer_vars, consumer_count,
                                 producer_vars[i].location);
      enum pipe_format format =
         loc >= 0 ? consumer_vars[loc].format : PIPE_FORMAT_NONE;

      if (format != PIPE_FORMAT_NONE) {
         offsets[i] = stride;
         stride += util_format_get_blocksize(format);
      } else {
         offsets[i] = -1;
      }
   }

   return stride;
}

/* Emitter for a single varying (attribute) descriptor */

static void
panfrost_emit_varying(const struct panfrost_device *dev,
                      struct mali_attribute_packed *out,
                      const struct pan_shader_varying varying,
                      enum pipe_format pipe_format, unsigned present,
                      uint16_t point_sprite_mask, signed offset,
                      enum pan_special_varying pos_varying)
{
   /* Note: varying.format != pipe_format in some obscure cases due to a
    * limitation of the NIR linker. This should be fixed in the future to
    * eliminate the additional lookups. See:
    * dEQP-GLES3.functional.shaders.conditionals.if.sequence_statements_vertex
    */
   gl_varying_slot loc = varying.location;
   mali_pixel_format format = dev->formats[pipe_format].hw;

   if (util_varying_is_point_coord(loc, point_sprite_mask)) {
      pan_emit_vary_special(dev, out, present, PAN_VARY_PNTCOORD);
   } else if (loc == VARYING_SLOT_POS) {
      pan_emit_vary_special(dev, out, present, pos_varying);
   } else if (loc == VARYING_SLOT_PSIZ) {
      pan_emit_vary_special(dev, out, present, PAN_VARY_PSIZ);
   } else if (loc == VARYING_SLOT_FACE) {
      pan_emit_vary_special(dev, out, present, PAN_VARY_FACE);
   } else if (offset < 0) {
      pan_emit_vary(dev, out, 0, (MALI_CONSTANT << 12), 0);
   } else {
      STATIC_ASSERT(PAN_VARY_GENERAL == 0);
      pan_emit_vary(dev, out, 0, format, offset);
   }
}

/* Links varyings and uploads ATTRIBUTE descriptors. Can execute at link time,
 * rather than draw time (under good conditions). */

static void
panfrost_emit_varying_descs(struct panfrost_pool *pool,
                            struct panfrost_compiled_shader *producer,
                            struct panfrost_compiled_shader *consumer,
                            uint16_t point_coord_mask, struct pan_linkage *out)
{
   struct panfrost_device *dev = pool->base.dev;
   unsigned producer_count = producer->info.varyings.output_count;
   unsigned consumer_count = consumer->info.varyings.input_count;

   /* Offsets within the general varying buffer, indexed by location */
   signed offsets[PAN_MAX_VARYINGS];
   assert(producer_count <= ARRAY_SIZE(offsets));
   assert(consumer_count <= ARRAY_SIZE(offsets));

   /* Allocate enough descriptors for both shader stages */
   struct panfrost_ptr T = pan_pool_alloc_desc_array(
      &pool->base, producer_count + consumer_count, ATTRIBUTE);

   /* Take a reference if we're being put on the CSO */
   if (!pool->owned) {
      out->bo = pool->transient_bo;
      panfrost_bo_reference(out->bo);
   }

   struct mali_attribute_packed *descs = T.cpu;
   out->producer = producer_count ? T.gpu : 0;
   out->consumer =
      consumer_count ? T.gpu + (pan_size(ATTRIBUTE) * producer_count) : 0;

   /* Lay out the varyings. Must use producer to lay out, in order to
    * respect transform feedback precisions. */
   out->present = pan_varying_present(dev, &producer->info, &consumer->info,
                                      point_coord_mask);

   out->stride =
      pan_assign_varyings(dev, &producer->info, &consumer->info, offsets);

   for (unsigned i = 0; i < producer_count; ++i) {
      signed j = pan_find_vary(consumer->info.varyings.input,
                               consumer->info.varyings.input_count,
                               producer->info.varyings.output[i].location);

      enum pipe_format format = (j >= 0)
                                   ? consumer->info.varyings.input[j].format
                                   : producer->info.varyings.output[i].format;

      panfrost_emit_varying(dev, descs + i, producer->info.varyings.output[i],
                            format, out->present, 0, offsets[i],
                            PAN_VARY_POSITION);
   }

   for (unsigned i = 0; i < consumer_count; ++i) {
      signed j = pan_find_vary(producer->info.varyings.output,
                               producer->info.varyings.output_count,
                               consumer->info.varyings.input[i].location);

      signed offset = (j >= 0) ? offsets[j] : -1;

      panfrost_emit_varying(
         dev, descs + producer_count + i, consumer->info.varyings.input[i],
         consumer->info.varyings.input[i].format, out->present,
         point_coord_mask, offset, PAN_VARY_FRAGCOORD);
   }
}

#if PAN_ARCH <= 5
static void
pan_emit_special_input(struct mali_attribute_buffer_packed *out,
                       unsigned present, enum pan_special_varying v,
                       unsigned special)
{
   if (present & BITFIELD_BIT(v)) {
      unsigned idx = pan_varying_index(present, v);

      pan_pack(out + idx, ATTRIBUTE_BUFFER, cfg) {
         cfg.special = special;
         cfg.type = 0;
      }
   }
}
#endif

static void
panfrost_emit_varying_descriptor(struct panfrost_batch *batch,
                                 unsigned vertex_count,
                                 bool point_coord_replace)
{
   struct panfrost_context *ctx = batch->ctx;
   struct panfrost_compiled_shader *vs = ctx->prog[PIPE_SHADER_VERTEX];
   struct panfrost_compiled_shader *fs = ctx->prog[PIPE_SHADER_FRAGMENT];

   uint16_t point_coord_mask = 0;

   memset(&batch->varyings, 0, sizeof(batch->varyings));

#if PAN_ARCH <= 5
   struct pipe_rasterizer_state *rast = &ctx->rasterizer->base;

   /* Point sprites are lowered on Bifrost and newer */
   if (point_coord_replace)
      point_coord_mask = ctx->rasterizer->base.sprite_coord_enable;
#endif

   /* In good conditions, we only need to link varyings once */
   bool prelink =
      (point_coord_mask == 0) && !vs->info.separable && !fs->info.separable;

   /* Try to reduce copies */
   struct pan_linkage _linkage;
   struct pan_linkage *linkage = prelink ? &vs->linkage : &_linkage;

   /* Emit ATTRIBUTE descriptors if needed */
   if (!prelink || vs->linkage.bo == NULL) {
      struct panfrost_pool *pool = prelink ? &ctx->descs : &batch->pool;

      panfrost_emit_varying_descs(pool, vs, fs, point_coord_mask, linkage);
   }

   unsigned present = linkage->present, stride = linkage->stride;
   unsigned count = util_bitcount(present);
   struct panfrost_ptr T =
      pan_pool_alloc_desc_array(&batch->pool.base, count + 1, ATTRIBUTE_BUFFER);
   struct mali_attribute_buffer_packed *varyings =
      (struct mali_attribute_buffer_packed *)T.cpu;

   batch->varyings.nr_bufs = count;

#if PAN_ARCH >= 6
   /* Suppress prefetch on Bifrost */
   memset(varyings + count, 0, sizeof(*varyings));
#endif

   if (stride) {
      panfrost_emit_varyings(
         batch, &varyings[pan_varying_index(present, PAN_VARY_GENERAL)], stride,
         vertex_count);
   } else {
      /* The indirect draw code reads the stride field, make sure
       * that it is initialised */
      memset(varyings + pan_varying_index(present, PAN_VARY_GENERAL), 0,
             sizeof(*varyings));
   }

   /* fp32 vec4 gl_Position */
   batch->varyings.pos = panfrost_emit_varyings(
      batch, &varyings[pan_varying_index(present, PAN_VARY_POSITION)],
      sizeof(float) * 4, vertex_count);

   if (present & BITFIELD_BIT(PAN_VARY_PSIZ)) {
      batch->varyings.psiz = panfrost_emit_varyings(
         batch, &varyings[pan_varying_index(present, PAN_VARY_PSIZ)], 2,
         vertex_count);
   }

#if PAN_ARCH <= 5
   pan_emit_special_input(
      varyings, present, PAN_VARY_PNTCOORD,
      (rast->sprite_coord_mode == PIPE_SPRITE_COORD_LOWER_LEFT)
         ? MALI_ATTRIBUTE_SPECIAL_POINT_COORD_MAX_Y
         : MALI_ATTRIBUTE_SPECIAL_POINT_COORD_MIN_Y);
   pan_emit_special_input(varyings, present, PAN_VARY_FACE,
                          MALI_ATTRIBUTE_SPECIAL_FRONT_FACING);
   pan_emit_special_input(varyings, present, PAN_VARY_FRAGCOORD,
                          MALI_ATTRIBUTE_SPECIAL_FRAG_COORD);
#endif

   batch->varyings.bufs = T.gpu;
   batch->varyings.vs = linkage->producer;
   batch->varyings.fs = linkage->consumer;
}
#endif

static void
emit_tls(struct panfrost_batch *batch)
{
   struct panfrost_device *dev = pan_device(batch->ctx->base.screen);

   /* Emitted with the FB descriptor on Midgard. */
   if (PAN_ARCH <= 5 && batch->framebuffer.gpu)
      return;

   struct panfrost_bo *tls_bo =
      batch->stack_size ? panfrost_batch_get_scratchpad(
                             batch, batch->stack_size, dev->thread_tls_alloc,
                             dev->core_id_range)
                        : NULL;
   struct pan_tls_info tls = {
      .tls =
         {
            .ptr = tls_bo ? tls_bo->ptr.gpu : 0,
            .size = batch->stack_size,
         },
   };

   assert(batch->tls.cpu);
   GENX(pan_emit_tls)(&tls, batch->tls.cpu);
}

static void
emit_fbd(struct panfrost_batch *batch, const struct pan_fb_info *fb)
{
   struct panfrost_device *dev = pan_device(batch->ctx->base.screen);
   struct panfrost_bo *tls_bo =
      batch->stack_size ? panfrost_batch_get_scratchpad(
                             batch, batch->stack_size, dev->thread_tls_alloc,
                             dev->core_id_range)
                        : NULL;
   struct pan_tls_info tls = {
      .tls =
         {
            .ptr = tls_bo ? tls_bo->ptr.gpu : 0,
            .size = batch->stack_size,
         },
   };

   batch->framebuffer.gpu |= GENX(pan_emit_fbd)(
      dev, fb, &tls, &batch->tiler_ctx, batch->framebuffer.cpu);
}

/* Mark a surface as written */

static void
panfrost_initialize_surface(struct panfrost_batch *batch,
                            struct pipe_surface *surf)
{
   if (surf) {
      struct panfrost_resource *rsrc = pan_resource(surf->texture);
      BITSET_SET(rsrc->valid.data, surf->u.tex.level);
   }
}

/* Generate a fragment job. This should be called once per frame. (Usually,
 * this corresponds to eglSwapBuffers or one of glFlush, glFinish)
 */
static void
emit_fragment_job(struct panfrost_batch *batch, const struct pan_fb_info *pfb)
{
   /* Mark the affected buffers as initialized, since we're writing to it.
    * Also, add the surfaces we're writing to to the batch */

   struct pipe_framebuffer_state *fb = &batch->key;

   for (unsigned i = 0; i < fb->nr_cbufs; ++i)
      panfrost_initialize_surface(batch, fb->cbufs[i]);

   panfrost_initialize_surface(batch, fb->zsbuf);

   /* The passed tile coords can be out of range in some cases, so we need
    * to clamp them to the framebuffer size to avoid a TILE_RANGE_FAULT.
    * Theoretically we also need to clamp the coordinates positive, but we
    * avoid that edge case as all four values are unsigned. Also,
    * theoretically we could clamp the minima, but if that has to happen
    * the asserts would fail anyway (since the maxima would get clamped
    * and then be smaller than the minima). An edge case of sorts occurs
    * when no scissors are added to draw, so by default min=~0 and max=0.
    * But that can't happen if any actual drawing occurs (beyond a
    * wallpaper reload), so this is again irrelevant in practice. */

   batch->maxx = MIN2(batch->maxx, fb->width);
   batch->maxy = MIN2(batch->maxy, fb->height);

   /* Rendering region must be at least 1x1; otherwise, there is nothing
    * to do and the whole job chain should have been discarded. */

   assert(batch->maxx > batch->minx);
   assert(batch->maxy > batch->miny);

   JOBX(emit_fragment_job)(batch, pfb);
}

/* Count generated primitives (when there is no geom/tess shaders) for
 * transform feedback */

static void
panfrost_statistics_record(struct panfrost_context *ctx,
                           const struct pipe_draw_info *info,
                           const struct pipe_draw_start_count_bias *draw)
{
   if (!ctx->active_queries)
      return;

   uint32_t prims = u_prims_for_vertices(info->mode, draw->count);
   ctx->prims_generated += prims;

   if (!ctx->streamout.num_targets)
      return;

   ctx->tf_prims_generated += prims;
   ctx->dirty |= PAN_DIRTY_SO;
}

static void
panfrost_update_streamout_offsets(struct panfrost_context *ctx)
{
   unsigned count =
      u_stream_outputs_for_vertices(ctx->active_prim, ctx->vertex_count);

   for (unsigned i = 0; i < ctx->streamout.num_targets; ++i) {
      if (!ctx->streamout.targets[i])
         continue;

      pan_so_target(ctx->streamout.targets[i])->offset += count;
   }
}

/* On Bifrost and older, the Renderer State Descriptor aggregates many pieces of
 * 3D state. In particular, it groups the fragment shader descriptor with
 * depth/stencil, blend, polygon offset, and multisampling state. These pieces
 * of state are dirty tracked independently for the benefit of newer GPUs that
 * separate the descriptors. FRAGMENT_RSD_DIRTY_MASK contains the list of 3D
 * dirty flags that trigger re-emits of the fragment RSD.
 *
 * Obscurely, occlusion queries are included. Occlusion query state is nominally
 * specified in the draw call descriptor, but must be considered when determing
 * early-Z state which is part of the RSD.
 */
#define FRAGMENT_RSD_DIRTY_MASK                                                \
   (PAN_DIRTY_ZS | PAN_DIRTY_BLEND | PAN_DIRTY_MSAA | PAN_DIRTY_RASTERIZER |   \
    PAN_DIRTY_OQ)

static inline void
panfrost_update_shader_state(struct panfrost_batch *batch,
                             enum pipe_shader_type st)
{
   struct panfrost_context *ctx = batch->ctx;
   struct panfrost_compiled_shader *ss = ctx->prog[st];

   bool frag = (st == PIPE_SHADER_FRAGMENT);
   unsigned dirty_3d = ctx->dirty;
   unsigned dirty = ctx->dirty_shader[st];

   if (dirty & (PAN_DIRTY_STAGE_TEXTURE | PAN_DIRTY_STAGE_SHADER)) {
      batch->textures[st] = panfrost_emit_texture_descriptors(batch, st);
   }

   if (dirty & PAN_DIRTY_STAGE_SAMPLER) {
      batch->samplers[st] = panfrost_emit_sampler_descriptors(batch, st);
   }

   /* On Bifrost and older, the fragment shader descriptor is fused
    * together with the renderer state; the combined renderer state
    * descriptor is emitted below. Otherwise, the shader descriptor is
    * standalone and is emitted here.
    */
   if ((dirty & PAN_DIRTY_STAGE_SHADER) && !((PAN_ARCH <= 7) && frag)) {
      batch->rsd[st] = panfrost_emit_compute_shader_meta(batch, st);
   }

#if PAN_ARCH >= 9
   if (dirty & PAN_DIRTY_STAGE_IMAGE) {
      batch->images[st] =
         ctx->image_mask[st] ? panfrost_emit_images(batch, st) : 0;
   }
#endif

   if ((dirty & ss->dirty_shader) || (dirty_3d & ss->dirty_3d)) {
      batch->uniform_buffers[st] = panfrost_emit_const_buf(
         batch, st, &batch->nr_uniform_buffers[st], &batch->push_uniforms[st],
         &batch->nr_push_uniforms[st]);
   }

#if PAN_ARCH <= 7
   /* On Bifrost and older, if the fragment shader changes OR any renderer
    * state specified with the fragment shader, the whole renderer state
    * descriptor is dirtied and must be reemited.
    */
   if (frag && ((dirty & PAN_DIRTY_STAGE_SHADER) ||
                (dirty_3d & FRAGMENT_RSD_DIRTY_MASK))) {

      batch->rsd[st] = panfrost_emit_frag_shader_meta(batch);
   }

   /* Vertex shaders need to mix vertex data and image descriptors in the
    * attribute array. This is taken care of in panfrost_update_state_3d().
    */
   if (st != PIPE_SHADER_VERTEX && (dirty & PAN_DIRTY_STAGE_IMAGE)) {
      batch->attribs[st] =
         panfrost_emit_image_attribs(batch, &batch->attrib_bufs[st], st);
   }
#endif
}

static inline void
panfrost_update_state_3d(struct panfrost_batch *batch)
{
   struct panfrost_context *ctx = batch->ctx;
   unsigned dirty = ctx->dirty;

   if (dirty & PAN_DIRTY_TLS_SIZE)
      panfrost_batch_adjust_stack_size(batch);

   if (dirty & PAN_DIRTY_BLEND)
      panfrost_set_batch_masks_blend(batch);

   if (dirty & PAN_DIRTY_ZS)
      panfrost_set_batch_masks_zs(batch);

#if PAN_ARCH >= 9
   if ((dirty & (PAN_DIRTY_ZS | PAN_DIRTY_RASTERIZER)) ||
       (ctx->dirty_shader[PIPE_SHADER_FRAGMENT] & PAN_DIRTY_STAGE_SHADER))
      batch->depth_stencil = panfrost_emit_depth_stencil(batch);

   if (dirty & PAN_DIRTY_BLEND)
      batch->blend = panfrost_emit_blend_valhall(batch);

   if (dirty & PAN_DIRTY_VERTEX) {
      batch->attribs[PIPE_SHADER_VERTEX] = panfrost_emit_vertex_data(batch);

      batch->attrib_bufs[PIPE_SHADER_VERTEX] =
         panfrost_emit_vertex_buffers(batch);
   }
#else
   unsigned vt_shader_dirty = ctx->dirty_shader[PIPE_SHADER_VERTEX];

   /* Vertex data, vertex shader and images accessed by the vertex shader have
    * an impact on the attributes array, we need to re-emit anytime one of these
    * parameters changes. */
   if ((dirty & PAN_DIRTY_VERTEX) ||
       (vt_shader_dirty & (PAN_DIRTY_STAGE_IMAGE | PAN_DIRTY_STAGE_SHADER))) {
      batch->attribs[PIPE_SHADER_VERTEX] = panfrost_emit_vertex_data(
         batch, &batch->attrib_bufs[PIPE_SHADER_VERTEX]);
   }
#endif
}

static void
panfrost_launch_xfb(struct panfrost_batch *batch,
                    const struct pipe_draw_info *info, unsigned count)
{
   struct panfrost_context *ctx = batch->ctx;

   /* Nothing to do */
   if (batch->ctx->streamout.num_targets == 0)
      return;

   /* TODO: XFB with index buffers */
   // assert(info->index_size == 0);
   u_trim_pipe_prim(info->mode, &count);

   if (count == 0)
      return;

   perf_debug_ctx(batch->ctx, "Emulating transform feedback");

   struct panfrost_uncompiled_shader *vs_uncompiled =
      ctx->uncompiled[PIPE_SHADER_VERTEX];
   struct panfrost_compiled_shader *vs = ctx->prog[PIPE_SHADER_VERTEX];

   vs_uncompiled->xfb->stream_output = vs->stream_output;

   mali_ptr saved_rsd = batch->rsd[PIPE_SHADER_VERTEX];
   mali_ptr saved_ubo = batch->uniform_buffers[PIPE_SHADER_VERTEX];
   mali_ptr saved_push = batch->push_uniforms[PIPE_SHADER_VERTEX];
   unsigned saved_nr_push_uniforms =
      batch->nr_push_uniforms[PIPE_SHADER_VERTEX];

   ctx->uncompiled[PIPE_SHADER_VERTEX] = NULL; /* should not be read */
   ctx->prog[PIPE_SHADER_VERTEX] = vs_uncompiled->xfb;
   batch->rsd[PIPE_SHADER_VERTEX] =
      panfrost_emit_compute_shader_meta(batch, PIPE_SHADER_VERTEX);

   batch->uniform_buffers[PIPE_SHADER_VERTEX] =
      panfrost_emit_const_buf(batch, PIPE_SHADER_VERTEX, NULL,
                              &batch->push_uniforms[PIPE_SHADER_VERTEX],
                              &batch->nr_push_uniforms[PIPE_SHADER_VERTEX]);

   JOBX(launch_xfb)(batch, info, count);
   batch->compute_count++;

   ctx->uncompiled[PIPE_SHADER_VERTEX] = vs_uncompiled;
   ctx->prog[PIPE_SHADER_VERTEX] = vs;
   batch->rsd[PIPE_SHADER_VERTEX] = saved_rsd;
   batch->uniform_buffers[PIPE_SHADER_VERTEX] = saved_ubo;
   batch->push_uniforms[PIPE_SHADER_VERTEX] = saved_push;
   batch->nr_push_uniforms[PIPE_SHADER_VERTEX] = saved_nr_push_uniforms;
}

/*
 * Increase the vertex count on the batch using a saturating add, and hope the
 * compiler can use the machine instruction here...
 */
static inline void
panfrost_increase_vertex_count(struct panfrost_batch *batch, uint32_t increment)
{
   uint32_t sum = batch->tiler_ctx.vertex_count + increment;

   if (sum >= batch->tiler_ctx.vertex_count)
      batch->tiler_ctx.vertex_count = sum;
   else
      batch->tiler_ctx.vertex_count = UINT32_MAX;
}

/*
 * If we change whether we're drawing points, or whether point sprites are
 * enabled (specified in the rasterizer), we may need to rebind shaders
 * accordingly. This implicitly covers the case of rebinding framebuffers,
 * because all dirty flags are set there.
 */
static void
panfrost_update_point_sprite_shader(struct panfrost_context *ctx,
                                    const struct pipe_draw_info *info)
{
   if ((ctx->dirty & PAN_DIRTY_RASTERIZER) ||
       ((ctx->active_prim == MESA_PRIM_POINTS) ^
        (info->mode == MESA_PRIM_POINTS))) {

      ctx->active_prim = info->mode;
      panfrost_update_shader_variant(ctx, PIPE_SHADER_FRAGMENT);
   }
}

static unsigned
panfrost_draw_get_vertex_count(struct panfrost_batch *batch,
                               const struct pipe_draw_info *info,
                               const struct pipe_draw_start_count_bias *draw,
                               bool idvs)
{
   struct panfrost_context *ctx = batch->ctx;
   unsigned vertex_count = ctx->vertex_count;
   unsigned min_index = 0, max_index = 0;

   batch->indices = 0;
   if (info->index_size && PAN_ARCH >= 9) {
      batch->indices = panfrost_get_index_buffer(batch, info, draw);

      /* Use index count to estimate vertex count */
      panfrost_increase_vertex_count(batch, draw->count);
   } else if (info->index_size) {
      batch->indices = panfrost_get_index_buffer_bounded(
         batch, info, draw, &min_index, &max_index);

      /* Use the corresponding values */
      vertex_count = max_index - min_index + 1;
      ctx->offset_start = min_index + draw->index_bias;
      panfrost_increase_vertex_count(batch, vertex_count);
   } else {
      ctx->offset_start = draw->start;
      panfrost_increase_vertex_count(batch, vertex_count);
   }

   if (info->instance_count > 1) {
      unsigned count = vertex_count;

      /* Index-Driven Vertex Shading requires different instances to
       * have different cache lines for position results. Each vertex
       * position is 16 bytes and the Mali cache line is 64 bytes, so
       * the instance count must be aligned to 4 vertices.
       */
      if (idvs)
         count = ALIGN_POT(count, 4);

      ctx->padded_count = panfrost_padded_vertex_count(count);
   } else {
      ctx->padded_count = vertex_count;
   }

   return vertex_count;
}

static void
panfrost_direct_draw(struct panfrost_batch *batch,
                     const struct pipe_draw_info *info, unsigned drawid_offset,
                     const struct pipe_draw_start_count_bias *draw)
{
   if (!draw->count || !info->instance_count)
      return;

   struct panfrost_context *ctx = batch->ctx;

   panfrost_update_point_sprite_shader(ctx, info);

   /* Take into account a negative bias */
   ctx->vertex_count =
      draw->count + (info->index_size ? abs(draw->index_bias) : 0);
   ctx->instance_count = info->instance_count;
   ctx->base_vertex = info->index_size ? draw->index_bias : 0;
   ctx->base_instance = info->start_instance;
   ctx->active_prim = info->mode;
   ctx->drawid = drawid_offset;

   struct panfrost_compiled_shader *vs = ctx->prog[PIPE_SHADER_VERTEX];
   bool idvs = vs->info.vs.idvs;

   UNUSED unsigned vertex_count =
      panfrost_draw_get_vertex_count(batch, info, draw, idvs);

   panfrost_statistics_record(ctx, info, draw);

   panfrost_update_state_3d(batch);
   panfrost_update_shader_state(batch, PIPE_SHADER_VERTEX);
   panfrost_update_shader_state(batch, PIPE_SHADER_FRAGMENT);
   panfrost_clean_state_3d(ctx);

   if (ctx->uncompiled[PIPE_SHADER_VERTEX]->xfb) {
      panfrost_launch_xfb(batch, info, draw->count);
   }

   /* Increment transform feedback offsets */
   panfrost_update_streamout_offsets(ctx);

   /* Any side effects must be handled by the XFB shader, so we only need
    * to run vertex shaders if we need rasterization.
    */
   if (panfrost_batch_skip_rasterization(batch))
      return;

#if PAN_ARCH <= 7
   /* Emit all sort of descriptors. */
   panfrost_emit_varying_descriptor(batch,
                                    ctx->padded_count * ctx->instance_count,
                                    info->mode == MESA_PRIM_POINTS);
#endif

   JOBX(launch_draw)(batch, info, drawid_offset, draw, vertex_count);
   batch->draw_count++;
}

static bool
panfrost_compatible_batch_state(struct panfrost_batch *batch, bool points)
{
   /* Only applies on Valhall */
   if (PAN_ARCH < 9)
      return true;

   struct panfrost_context *ctx = batch->ctx;
   struct pipe_rasterizer_state *rast = &ctx->rasterizer->base;

   bool coord = (rast->sprite_coord_mode == PIPE_SPRITE_COORD_LOWER_LEFT);
   bool first = rast->flatshade_first;

   /* gl_PointCoord orientation only matters when drawing points, but
    * provoking vertex doesn't matter for points.
    */
   if (points)
      return pan_tristate_set(&batch->sprite_coord_origin, coord);
   else
      return pan_tristate_set(&batch->first_provoking_vertex, first);
}

static void
panfrost_draw_vbo(struct pipe_context *pipe, const struct pipe_draw_info *info,
                  unsigned drawid_offset,
                  const struct pipe_draw_indirect_info *indirect,
                  const struct pipe_draw_start_count_bias *draws,
                  unsigned num_draws)
{
   struct panfrost_context *ctx = pan_context(pipe);
   struct panfrost_device *dev = pan_device(pipe->screen);

   if (!panfrost_render_condition_check(ctx))
      return;

   ctx->draw_calls++;

   /* Emulate indirect draws on JM */
   if (indirect && indirect->buffer) {
      assert(num_draws == 1);
      util_draw_indirect(pipe, info, indirect);
      perf_debug(dev, "Emulating indirect draw on the CPU");
      return;
   }

   /* Do some common setup */
   struct panfrost_batch *batch = panfrost_get_batch_for_fbo(ctx);

   /* Don't add too many jobs to a single batch. Job manager hardware has a
    * hard limit of 65536 jobs per job chain. Given a draw issues a maximum
    * of 3 jobs (a vertex, a tiler and a compute job is XFB is enabled), we
    * could use 65536 / 3 as a limit, but we choose a smaller soft limit
    * (arbitrary) to avoid the risk of timeouts. This might not be a good
    * idea. */
   if (unlikely(batch->draw_count > 10000))
      batch = panfrost_get_fresh_batch_for_fbo(ctx, "Too many draws");

   bool points = (info->mode == MESA_PRIM_POINTS);

   if (unlikely(!panfrost_compatible_batch_state(batch, points))) {
      batch = panfrost_get_fresh_batch_for_fbo(ctx, "State change");

      ASSERTED bool succ = panfrost_compatible_batch_state(batch, points);
      assert(succ && "must be able to set state for a fresh batch");
   }

   /* panfrost_batch_skip_rasterization reads
    * batch->scissor_culls_everything, which is set by
    * panfrost_emit_viewport, so call that first.
    */
   if (ctx->dirty & (PAN_DIRTY_VIEWPORT | PAN_DIRTY_SCISSOR))
      batch->viewport = panfrost_emit_viewport(batch);

   /* Mark everything dirty when debugging */
   if (unlikely(dev->debug & PAN_DBG_DIRTY))
      panfrost_dirty_state_all(ctx);

   /* Conservatively assume draw parameters always change */
   ctx->dirty |= PAN_DIRTY_PARAMS | PAN_DIRTY_DRAWID;

   struct pipe_draw_info tmp_info = *info;
   unsigned drawid = drawid_offset;

   for (unsigned i = 0; i < num_draws; i++) {
      panfrost_direct_draw(batch, &tmp_info, drawid, &draws[i]);

      if (tmp_info.increment_draw_id) {
         ctx->dirty |= PAN_DIRTY_DRAWID;
         drawid++;
      }
   }
}

/* Launch grid is the compute equivalent of draw_vbo, so in this routine, we
 * construct the COMPUTE job and some of its payload.
 */

static void
panfrost_launch_grid_on_batch(struct pipe_context *pipe,
                              struct panfrost_batch *batch,
                              const struct pipe_grid_info *info)
{
   struct panfrost_context *ctx = pan_context(pipe);

   if (info->indirect && !PAN_GPU_INDIRECTS) {
      struct pipe_transfer *transfer;
      uint32_t *params =
         pipe_buffer_map_range(pipe, info->indirect, info->indirect_offset,
                               3 * sizeof(uint32_t), PIPE_MAP_READ, &transfer);

      struct pipe_grid_info direct = *info;
      direct.indirect = NULL;
      direct.grid[0] = params[0];
      direct.grid[1] = params[1];
      direct.grid[2] = params[2];
      pipe_buffer_unmap(pipe, transfer);

      if (params[0] && params[1] && params[2])
         panfrost_launch_grid_on_batch(pipe, batch, &direct);

      return;
   }

   ctx->compute_grid = info;

   /* Conservatively assume workgroup size changes every launch */
   ctx->dirty |= PAN_DIRTY_PARAMS;

   panfrost_update_shader_state(batch, PIPE_SHADER_COMPUTE);

   /* We want our compute thread descriptor to be per job.
    * Save the global one, and restore it when we're done emitting
    * the job.
    */
   mali_ptr saved_tls = batch->tls.gpu;
   batch->tls.gpu = panfrost_emit_shared_memory(batch, info);

   JOBX(launch_grid)(batch, info);
   batch->compute_count++;
   batch->tls.gpu = saved_tls;
}

static void
panfrost_launch_grid(struct pipe_context *pipe,
                     const struct pipe_grid_info *info)
{
   struct panfrost_context *ctx = pan_context(pipe);

   /* XXX - shouldn't be necessary with working memory barriers. Affected
    * test: KHR-GLES31.core.compute_shader.pipeline-post-xfb */
   panfrost_flush_all_batches(ctx, "Launch grid pre-barrier");

   struct panfrost_batch *batch = panfrost_get_batch_for_fbo(ctx);
   panfrost_launch_grid_on_batch(pipe, batch, info);

   panfrost_flush_all_batches(ctx, "Launch grid post-barrier");
}

#define AFBC_BLOCK_ALIGN 16

static void
panfrost_launch_afbc_shader(struct panfrost_batch *batch, void *cso,
                            struct pipe_constant_buffer *cbuf,
                            unsigned nr_blocks)
{
   struct pipe_context *pctx = &batch->ctx->base;
   void *saved_cso = NULL;
   struct pipe_constant_buffer saved_const = {};
   struct pipe_grid_info grid = {
      .block[0] = 1,
      .block[1] = 1,
      .block[2] = 1,
      .grid[0] = nr_blocks,
      .grid[1] = 1,
      .grid[2] = 1,
   };

   struct panfrost_constant_buffer *pbuf =
      &batch->ctx->constant_buffer[PIPE_SHADER_COMPUTE];
   saved_cso = batch->ctx->uncompiled[PIPE_SHADER_COMPUTE];
   util_copy_constant_buffer(&pbuf->cb[0], &saved_const, true);

   pctx->bind_compute_state(pctx, cso);
   pctx->set_constant_buffer(pctx, PIPE_SHADER_COMPUTE, 0, false, cbuf);

   panfrost_launch_grid_on_batch(pctx, batch, &grid);

   pctx->bind_compute_state(pctx, saved_cso);
   pctx->set_constant_buffer(pctx, PIPE_SHADER_COMPUTE, 0, true, &saved_const);
}

#define LAUNCH_AFBC_SHADER(name, batch, rsrc, consts, nr_blocks)               \
   struct pan_afbc_shader_data *shaders =                                      \
      panfrost_afbc_get_shaders(batch->ctx, rsrc, AFBC_BLOCK_ALIGN);           \
   struct pipe_constant_buffer constant_buffer = {                             \
      .buffer_size = sizeof(consts),                                           \
      .user_buffer = &consts};                                                 \
   panfrost_launch_afbc_shader(batch, shaders->name##_cso, &constant_buffer,   \
                               nr_blocks);

static void
panfrost_afbc_size(struct panfrost_batch *batch, struct panfrost_resource *src,
                   struct panfrost_bo *metadata, unsigned offset,
                   unsigned level)
{
   struct pan_image_slice_layout *slice = &src->image.layout.slices[level];
   struct panfrost_afbc_size_info consts = {
      .src =
         src->image.data.bo->ptr.gpu + src->image.data.offset + slice->offset,
      .metadata = metadata->ptr.gpu + offset,
   };

   panfrost_batch_read_rsrc(batch, src, PIPE_SHADER_COMPUTE);
   panfrost_batch_write_bo(batch, metadata, PIPE_SHADER_COMPUTE);

   LAUNCH_AFBC_SHADER(size, batch, src, consts, slice->afbc.nr_blocks);
}

static void
panfrost_afbc_pack(struct panfrost_batch *batch, struct panfrost_resource *src,
                   struct panfrost_bo *dst,
                   struct pan_image_slice_layout *dst_slice,
                   struct panfrost_bo *metadata, unsigned metadata_offset,
                   unsigned level)
{
   struct pan_image_slice_layout *src_slice = &src->image.layout.slices[level];
   struct panfrost_afbc_pack_info consts = {
      .src = src->image.data.bo->ptr.gpu + src->image.data.offset +
             src_slice->offset,
      .dst = dst->ptr.gpu + dst_slice->offset,
      .metadata = metadata->ptr.gpu + metadata_offset,
      .header_size = dst_slice->afbc.header_size,
      .src_stride = src_slice->afbc.stride,
      .dst_stride = dst_slice->afbc.stride,
   };

   panfrost_batch_write_rsrc(batch, src, PIPE_SHADER_COMPUTE);
   panfrost_batch_write_bo(batch, dst, PIPE_SHADER_COMPUTE);
   panfrost_batch_add_bo(batch, metadata, PIPE_SHADER_COMPUTE);

   LAUNCH_AFBC_SHADER(pack, batch, src, consts, dst_slice->afbc.nr_blocks);
}

static void *
panfrost_create_rasterizer_state(struct pipe_context *pctx,
                                 const struct pipe_rasterizer_state *cso)
{
   struct panfrost_rasterizer *so = CALLOC_STRUCT(panfrost_rasterizer);

   so->base = *cso;

#if PAN_ARCH <= 7
   pan_pack(&so->multisample, MULTISAMPLE_MISC, cfg) {
      cfg.multisample_enable = cso->multisample;
      cfg.fixed_function_near_discard = cso->depth_clip_near;
      cfg.fixed_function_far_discard = cso->depth_clip_far;
      cfg.shader_depth_range_fixed = true;
   }

   pan_pack(&so->stencil_misc, STENCIL_MASK_MISC, cfg) {
      cfg.front_facing_depth_bias = cso->offset_tri;
      cfg.back_facing_depth_bias = cso->offset_tri;
      cfg.single_sampled_lines = !cso->multisample;
   }
#endif

   return so;
}

#if PAN_ARCH >= 9
/*
 * Given a pipe_vertex_element, pack the corresponding Valhall attribute
 * descriptor. This function is called at CSO create time.
 */
static void
panfrost_pack_attribute(struct panfrost_device *dev,
                        const struct pipe_vertex_element el,
                        struct mali_attribute_packed *out)
{
   pan_pack(out, ATTRIBUTE, cfg) {
      cfg.table = PAN_TABLE_ATTRIBUTE_BUFFER;
      cfg.frequency = (el.instance_divisor > 0)
                         ? MALI_ATTRIBUTE_FREQUENCY_INSTANCE
                         : MALI_ATTRIBUTE_FREQUENCY_VERTEX;
      cfg.format = dev->formats[el.src_format].hw;
      cfg.offset = el.src_offset;
      cfg.buffer_index = el.vertex_buffer_index;
      cfg.stride = el.src_stride;

      if (el.instance_divisor == 0) {
         /* Per-vertex */
         cfg.attribute_type = MALI_ATTRIBUTE_TYPE_1D;
         cfg.frequency = MALI_ATTRIBUTE_FREQUENCY_VERTEX;
         cfg.offset_enable = true;
      } else if (util_is_power_of_two_or_zero(el.instance_divisor)) {
         /* Per-instance, POT divisor */
         cfg.attribute_type = MALI_ATTRIBUTE_TYPE_1D_POT_DIVISOR;
         cfg.frequency = MALI_ATTRIBUTE_FREQUENCY_INSTANCE;
         cfg.divisor_r = __builtin_ctz(el.instance_divisor);
      } else {
         /* Per-instance, NPOT divisor */
         cfg.attribute_type = MALI_ATTRIBUTE_TYPE_1D_NPOT_DIVISOR;
         cfg.frequency = MALI_ATTRIBUTE_FREQUENCY_INSTANCE;

         cfg.divisor_d = panfrost_compute_magic_divisor(
            el.instance_divisor, &cfg.divisor_r, &cfg.divisor_e);
      }
   }
}
#endif

static void *
panfrost_create_vertex_elements_state(struct pipe_context *pctx,
                                      unsigned num_elements,
                                      const struct pipe_vertex_element *elements)
{
   struct panfrost_vertex_state *so = CALLOC_STRUCT(panfrost_vertex_state);
   struct panfrost_device *dev = pan_device(pctx->screen);

   so->num_elements = num_elements;
   memcpy(so->pipe, elements, sizeof(*elements) * num_elements);

   for (unsigned i = 0; i < num_elements; ++i)
      so->strides[elements[i].vertex_buffer_index] = elements[i].src_stride;
#if PAN_ARCH >= 9
   for (unsigned i = 0; i < num_elements; ++i)
      panfrost_pack_attribute(dev, elements[i], &so->attributes[i]);
#else
   /* Assign attribute buffers corresponding to the vertex buffers, keyed
    * for a particular divisor since that's how instancing works on Mali */
   for (unsigned i = 0; i < num_elements; ++i) {
      so->element_buffer[i] = pan_assign_vertex_buffer(
         so->buffers, &so->nr_bufs, elements[i].vertex_buffer_index,
         elements[i].instance_divisor);
   }

   for (int i = 0; i < num_elements; ++i) {
      enum pipe_format fmt = elements[i].src_format;
      so->formats[i] = dev->formats[fmt].hw;

      assert(MALI_EXTRACT_INDEX(so->formats[i]) && "format must be supported");
   }

   /* Let's also prepare vertex builtins */
   so->formats[PAN_VERTEX_ID] = dev->formats[PIPE_FORMAT_R32_UINT].hw;
   so->formats[PAN_INSTANCE_ID] = dev->formats[PIPE_FORMAT_R32_UINT].hw;
#endif

   return so;
}

static inline unsigned
pan_pipe_to_stencil_op(enum pipe_stencil_op in)
{
   switch (in) {
   case PIPE_STENCIL_OP_KEEP:
      return MALI_STENCIL_OP_KEEP;
   case PIPE_STENCIL_OP_ZERO:
      return MALI_STENCIL_OP_ZERO;
   case PIPE_STENCIL_OP_REPLACE:
      return MALI_STENCIL_OP_REPLACE;
   case PIPE_STENCIL_OP_INCR:
      return MALI_STENCIL_OP_INCR_SAT;
   case PIPE_STENCIL_OP_DECR:
      return MALI_STENCIL_OP_DECR_SAT;
   case PIPE_STENCIL_OP_INCR_WRAP:
      return MALI_STENCIL_OP_INCR_WRAP;
   case PIPE_STENCIL_OP_DECR_WRAP:
      return MALI_STENCIL_OP_DECR_WRAP;
   case PIPE_STENCIL_OP_INVERT:
      return MALI_STENCIL_OP_INVERT;
   default:
      unreachable("Invalid stencil op");
   }
}

#if PAN_ARCH <= 7
static inline void
pan_pipe_to_stencil(const struct pipe_stencil_state *in,
                    struct mali_stencil_packed *out)
{
   pan_pack(out, STENCIL, s) {
      s.mask = in->valuemask;
      s.compare_function = (enum mali_func)in->func;
      s.stencil_fail = pan_pipe_to_stencil_op(in->fail_op);
      s.depth_fail = pan_pipe_to_stencil_op(in->zfail_op);
      s.depth_pass = pan_pipe_to_stencil_op(in->zpass_op);
   }
}
#endif

static bool
pipe_zs_always_passes(const struct pipe_depth_stencil_alpha_state *zsa)
{
   if (zsa->depth_enabled && zsa->depth_func != PIPE_FUNC_ALWAYS)
      return false;

   if (zsa->stencil[0].enabled && zsa->stencil[0].func != PIPE_FUNC_ALWAYS)
      return false;

   if (zsa->stencil[1].enabled && zsa->stencil[1].func != PIPE_FUNC_ALWAYS)
      return false;

   return true;
}

static void *
panfrost_create_depth_stencil_state(
   struct pipe_context *pipe, const struct pipe_depth_stencil_alpha_state *zsa)
{
   struct panfrost_zsa_state *so = CALLOC_STRUCT(panfrost_zsa_state);
   so->base = *zsa;

   const struct pipe_stencil_state front = zsa->stencil[0];
   const struct pipe_stencil_state back =
      zsa->stencil[1].enabled ? zsa->stencil[1] : front;

   enum mali_func depth_func =
      zsa->depth_enabled ? (enum mali_func)zsa->depth_func : MALI_FUNC_ALWAYS;

   /* Normalize (there's no separate enable) */
   if (PAN_ARCH <= 5 && !zsa->alpha_enabled)
      so->base.alpha_func = MALI_FUNC_ALWAYS;

#if PAN_ARCH <= 7
   /* Prepack relevant parts of the Renderer State Descriptor. They will
    * be ORed in at draw-time */
   pan_pack(&so->rsd_depth, MULTISAMPLE_MISC, cfg) {
      cfg.depth_function = depth_func;
      cfg.depth_write_mask = zsa->depth_writemask;
   }

   pan_pack(&so->rsd_stencil, STENCIL_MASK_MISC, cfg) {
      cfg.stencil_enable = front.enabled;
      cfg.stencil_mask_front = front.writemask;
      cfg.stencil_mask_back = back.writemask;

#if PAN_ARCH <= 5
      cfg.alpha_test_compare_function = (enum mali_func)so->base.alpha_func;
#endif
   }

   /* Stencil tests have their own words in the RSD */
   pan_pipe_to_stencil(&front, &so->stencil_front);
   pan_pipe_to_stencil(&back, &so->stencil_back);
#else
   pan_pack(&so->desc, DEPTH_STENCIL, cfg) {
      cfg.front_compare_function = (enum mali_func)front.func;
      cfg.front_stencil_fail = pan_pipe_to_stencil_op(front.fail_op);
      cfg.front_depth_fail = pan_pipe_to_stencil_op(front.zfail_op);
      cfg.front_depth_pass = pan_pipe_to_stencil_op(front.zpass_op);

      cfg.back_compare_function = (enum mali_func)back.func;
      cfg.back_stencil_fail = pan_pipe_to_stencil_op(back.fail_op);
      cfg.back_depth_fail = pan_pipe_to_stencil_op(back.zfail_op);
      cfg.back_depth_pass = pan_pipe_to_stencil_op(back.zpass_op);

      cfg.stencil_test_enable = front.enabled;
      cfg.front_write_mask = front.writemask;
      cfg.back_write_mask = back.writemask;
      cfg.front_value_mask = front.valuemask;
      cfg.back_value_mask = back.valuemask;

      cfg.depth_write_enable = zsa->depth_writemask;
      cfg.depth_function = depth_func;
   }
#endif

   so->enabled = zsa->stencil[0].enabled ||
                 (zsa->depth_enabled && zsa->depth_func != PIPE_FUNC_ALWAYS);

   so->zs_always_passes = pipe_zs_always_passes(zsa);
   so->writes_zs = util_writes_depth_stencil(zsa);

   /* TODO: Bounds test should be easy */
   assert(!zsa->depth_bounds_test);

   return so;
}

static struct pipe_sampler_view *
panfrost_create_sampler_view(struct pipe_context *pctx,
                             struct pipe_resource *texture,
                             const struct pipe_sampler_view *template)
{
   struct panfrost_context *ctx = pan_context(pctx);
   struct panfrost_sampler_view *so =
      rzalloc(pctx, struct panfrost_sampler_view);

   pan_legalize_afbc_format(ctx, pan_resource(texture), template->format,
                            false, false);

   pipe_reference(NULL, &texture->reference);

   so->base = *template;
   so->base.texture = texture;
   so->base.reference.count = 1;
   so->base.context = pctx;

   panfrost_create_sampler_view_bo(so, pctx, texture);

   return (struct pipe_sampler_view *)so;
}

/* A given Gallium blend state can be encoded to the hardware in numerous,
 * dramatically divergent ways due to the interactions of blending with
 * framebuffer formats. Conceptually, there are two modes:
 *
 * - Fixed-function blending (for suitable framebuffer formats, suitable blend
 *   state, and suitable blend constant)
 *
 * - Blend shaders (for everything else)
 *
 * A given Gallium blend configuration will compile to exactly one
 * fixed-function blend state, if it compiles to any, although the constant
 * will vary across runs as that is tracked outside of the Gallium CSO.
 *
 * However, that same blend configuration will compile to many different blend
 * shaders, depending on the framebuffer formats active. The rationale is that
 * blend shaders override not just fixed-function blending but also
 * fixed-function format conversion, so blend shaders are keyed to a particular
 * framebuffer format. As an example, the tilebuffer format is identical for
 * RG16F and RG16UI -- both are simply 32-bit raw pixels -- so both require
 * blend shaders.
 *
 * All of this state is encapsulated in the panfrost_blend_state struct
 * (our subclass of pipe_blend_state).
 */

/* Create a blend CSO. Essentially, try to compile a fixed-function
 * expression and initialize blend shaders */

static void *
panfrost_create_blend_state(struct pipe_context *pipe,
                            const struct pipe_blend_state *blend)
{
   struct panfrost_blend_state *so = CALLOC_STRUCT(panfrost_blend_state);
   so->base = *blend;

   so->pan.logicop_enable = blend->logicop_enable;
   so->pan.logicop_func = blend->logicop_func;
   so->pan.rt_count = blend->max_rt + 1;

   for (unsigned c = 0; c < so->pan.rt_count; ++c) {
      unsigned g = blend->independent_blend_enable ? c : 0;
      const struct pipe_rt_blend_state pipe = blend->rt[g];
      struct pan_blend_equation equation = {0};

      equation.color_mask = pipe.colormask;
      equation.blend_enable = pipe.blend_enable;

      if (pipe.blend_enable) {
         equation.rgb_func = pipe.rgb_func;
         equation.rgb_src_factor = pipe.rgb_src_factor;
         equation.rgb_dst_factor = pipe.rgb_dst_factor;
         equation.alpha_func = pipe.alpha_func;
         equation.alpha_src_factor = pipe.alpha_src_factor;
         equation.alpha_dst_factor = pipe.alpha_dst_factor;
      }

      /* Determine some common properties */
      unsigned constant_mask = pan_blend_constant_mask(equation);
      const bool supports_2src = pan_blend_supports_2src(PAN_ARCH);
      so->info[c] = (struct pan_blend_info){
         .enabled = (equation.color_mask != 0) &&
                    !(blend->logicop_enable &&
                      blend->logicop_func == PIPE_LOGICOP_NOOP),
         .opaque = !blend->logicop_enable && pan_blend_is_opaque(equation),
         .constant_mask = constant_mask,

         /* TODO: check the dest for the logicop */
         .load_dest = blend->logicop_enable || pan_blend_reads_dest(equation),

         /* Could this possibly be fixed-function? */
         .fixed_function =
            !blend->logicop_enable &&
            pan_blend_can_fixed_function(equation, supports_2src) &&
            (!constant_mask || pan_blend_supports_constant(PAN_ARCH, c)),

         .alpha_zero_nop = pan_blend_alpha_zero_nop(equation),
         .alpha_one_store = pan_blend_alpha_one_store(equation),
      };

      so->pan.rts[c].equation = equation;

      /* Bifrost needs to know if any render target loads its
       * destination in the hot draw path, so precompute this */
      if (so->info[c].load_dest)
         so->load_dest_mask |= BITFIELD_BIT(c);

      /* Bifrost needs to know if any render target loads its
       * destination in the hot draw path, so precompute this */
      if (so->info[c].enabled)
         so->enabled_mask |= BITFIELD_BIT(c);

      /* Converting equations to Mali style is expensive, do it at
       * CSO create time instead of draw-time */
      if (so->info[c].fixed_function) {
         so->equation[c] = pan_pack_blend(equation);
      }
   }

   return so;
}

#if PAN_ARCH >= 9
static enum mali_flush_to_zero_mode
panfrost_ftz_mode(struct pan_shader_info *info)
{
   if (info->ftz_fp32) {
      if (info->ftz_fp16)
         return MALI_FLUSH_TO_ZERO_MODE_ALWAYS;
      else
         return MALI_FLUSH_TO_ZERO_MODE_DX11;
   } else {
      /* We don't have a "flush FP16, preserve FP32" mode, but APIs
       * should not be able to generate that.
       */
      assert(!info->ftz_fp16 && !info->ftz_fp32);
      return MALI_FLUSH_TO_ZERO_MODE_PRESERVE_SUBNORMALS;
   }
}
#endif

static void
prepare_shader(struct panfrost_compiled_shader *state,
               struct panfrost_pool *pool, bool upload)
{
#if PAN_ARCH <= 7
   void *out = &state->partial_rsd;

   if (upload) {
      struct panfrost_ptr ptr =
         pan_pool_alloc_desc(&pool->base, RENDERER_STATE);

      state->state = panfrost_pool_take_ref(pool, ptr.gpu);
      out = ptr.cpu;
   }

   pan_pack(out, RENDERER_STATE, cfg) {
      pan_shader_prepare_rsd(&state->info, state->bin.gpu, &cfg);
   }
#else
   assert(upload);

   /* The address in the shader program descriptor must be non-null, but
    * the entire shader program descriptor may be omitted.
    *
    * See dEQP-GLES31.functional.compute.basic.empty
    */
   if (!state->bin.gpu)
      return;

   bool vs = (state->info.stage == MESA_SHADER_VERTEX);
   bool secondary_enable = (vs && state->info.vs.secondary_enable);

   unsigned nr_variants = secondary_enable ? 3 : vs ? 2 : 1;
   struct panfrost_ptr ptr =
      pan_pool_alloc_desc_array(&pool->base, nr_variants, SHADER_PROGRAM);

   state->state = panfrost_pool_take_ref(pool, ptr.gpu);

   /* Generic, or IDVS/points */
   pan_pack(ptr.cpu, SHADER_PROGRAM, cfg) {
      cfg.stage = pan_shader_stage(&state->info);

      if (cfg.stage == MALI_SHADER_STAGE_FRAGMENT)
         cfg.fragment_coverage_bitmask_type = MALI_COVERAGE_BITMASK_TYPE_GL;
      else if (vs)
         cfg.vertex_warp_limit = MALI_WARP_LIMIT_HALF;

      cfg.register_allocation =
         pan_register_allocation(state->info.work_reg_count);
      cfg.binary = state->bin.gpu;
      cfg.preload.r48_r63 = (state->info.preload >> 48);
      cfg.flush_to_zero_mode = panfrost_ftz_mode(&state->info);

      if (cfg.stage == MALI_SHADER_STAGE_FRAGMENT)
         cfg.requires_helper_threads = state->info.contains_barrier;
   }

   if (!vs)
      return;

   /* IDVS/triangles */
   pan_pack(ptr.cpu + pan_size(SHADER_PROGRAM), SHADER_PROGRAM, cfg) {
      cfg.stage = pan_shader_stage(&state->info);
      cfg.vertex_warp_limit = MALI_WARP_LIMIT_HALF;
      cfg.register_allocation =
         pan_register_allocation(state->info.work_reg_count);
      cfg.binary = state->bin.gpu + state->info.vs.no_psiz_offset;
      cfg.preload.r48_r63 = (state->info.preload >> 48);
      cfg.flush_to_zero_mode = panfrost_ftz_mode(&state->info);
   }

   if (!secondary_enable)
      return;

   pan_pack(ptr.cpu + (pan_size(SHADER_PROGRAM) * 2), SHADER_PROGRAM, cfg) {
      unsigned work_count = state->info.vs.secondary_work_reg_count;

      cfg.stage = pan_shader_stage(&state->info);
      cfg.vertex_warp_limit = MALI_WARP_LIMIT_FULL;
      cfg.register_allocation = pan_register_allocation(work_count);
      cfg.binary = state->bin.gpu + state->info.vs.secondary_offset;
      cfg.preload.r48_r63 = (state->info.vs.secondary_preload >> 48);
      cfg.flush_to_zero_mode = panfrost_ftz_mode(&state->info);
   }
#endif
}

static void
screen_destroy(struct pipe_screen *pscreen)
{
   struct panfrost_device *dev = pan_device(pscreen);
   GENX(pan_blitter_cleanup)(dev);
#if PAN_GPU_INDIRECTS
   GENX(pan_indirect_dispatch_cleanup)(dev);
#endif
}

static void
panfrost_sampler_view_destroy(struct pipe_context *pctx,
                              struct pipe_sampler_view *pview)
{
   struct panfrost_sampler_view *view = (struct panfrost_sampler_view *)pview;

   pipe_resource_reference(&pview->texture, NULL);
   panfrost_bo_unreference(view->state.bo);
   ralloc_free(view);
}

static void
context_populate_vtbl(struct pipe_context *pipe)
{
   pipe->draw_vbo = panfrost_draw_vbo;
   pipe->launch_grid = panfrost_launch_grid;

   pipe->create_vertex_elements_state = panfrost_create_vertex_elements_state;
   pipe->create_rasterizer_state = panfrost_create_rasterizer_state;
   pipe->create_depth_stencil_alpha_state = panfrost_create_depth_stencil_state;
   pipe->create_sampler_view = panfrost_create_sampler_view;
   pipe->sampler_view_destroy = panfrost_sampler_view_destroy;
   pipe->create_sampler_state = panfrost_create_sampler_state;
   pipe->create_blend_state = panfrost_create_blend_state;

   pipe->get_sample_position = u_default_get_sample_position;
}

#if PAN_ARCH <= 5

/* Returns the polygon list's GPU address if available, or otherwise allocates
 * the polygon list.  It's perfectly fast to use allocate/free BO directly,
 * since we'll hit the BO cache and this is one-per-batch anyway. */

static mali_ptr
batch_get_polygon_list(struct panfrost_batch *batch)
{
   struct panfrost_device *dev = pan_device(batch->ctx->base.screen);

   if (!batch->tiler_ctx.midgard.polygon_list) {
      bool has_draws = batch->draw_count > 0;
      unsigned size = panfrost_tiler_get_polygon_list_size(
         dev, batch->key.width, batch->key.height,
         batch->tiler_ctx.vertex_count);

      /* Create the BO as invisible if we can. If there are no draws,
       * we need to write the polygon list manually because there's
       * no WRITE_VALUE job in the chain
       */
      bool init_polygon_list = !has_draws;
      batch->tiler_ctx.midgard.polygon_list = panfrost_batch_create_bo(
         batch, size, init_polygon_list ? 0 : PAN_BO_INVISIBLE,
         PIPE_SHADER_VERTEX, "Polygon list");
      panfrost_batch_add_bo(batch, batch->tiler_ctx.midgard.polygon_list,
                            PIPE_SHADER_FRAGMENT);

      if (init_polygon_list && dev->model->quirks.no_hierarchical_tiling) {
         assert(batch->tiler_ctx.midgard.polygon_list->ptr.cpu);
         uint32_t *polygon_list_body =
            batch->tiler_ctx.midgard.polygon_list->ptr.cpu +
            MALI_MIDGARD_TILER_MINIMUM_HEADER_SIZE;

         /* Magic for Mali T720 */
         polygon_list_body[0] = 0xa0000000;
      } else if (init_polygon_list) {
         assert(batch->tiler_ctx.midgard.polygon_list->ptr.cpu);
         uint32_t *header = batch->tiler_ctx.midgard.polygon_list->ptr.cpu;
         memset(header, 0, size);
      }

      batch->tiler_ctx.midgard.disable = !has_draws;
   }

   return batch->tiler_ctx.midgard.polygon_list->ptr.gpu;
}
#endif

static void
init_polygon_list(struct panfrost_batch *batch)
{
#if PAN_ARCH <= 5
   mali_ptr polygon_list = batch_get_polygon_list(batch);
   pan_jc_initialize_tiler(&batch->pool.base, &batch->jm.jobs.vtc_jc,
                           polygon_list);
#endif
}

static int
submit_batch(struct panfrost_batch *batch, struct pan_fb_info *fb)
{
   JOBX(preload_fb)(batch, fb);
   init_polygon_list(batch);

   /* Now that all draws are in, we can finally prepare the
    * FBD for the batch (if there is one). */

   emit_tls(batch);

   if (panfrost_has_fragment_job(batch)) {
      emit_fbd(batch, fb);
      emit_fragment_job(batch, fb);
   }

   return JOBX(submit_batch)(batch);
}

void
GENX(panfrost_cmdstream_screen_init)(struct panfrost_screen *screen)
{
   struct panfrost_device *dev = &screen->dev;

   screen->vtbl.prepare_shader = prepare_shader;
   screen->vtbl.screen_destroy = screen_destroy;
   screen->vtbl.context_populate_vtbl = context_populate_vtbl;
   screen->vtbl.init_batch = JOBX(init_batch);
   screen->vtbl.submit_batch = submit_batch;
   screen->vtbl.get_blend_shader = GENX(pan_blend_get_shader_locked);
   screen->vtbl.get_compiler_options = GENX(pan_shader_get_compiler_options);
   screen->vtbl.compile_shader = GENX(pan_shader_compile);
   screen->vtbl.afbc_size = panfrost_afbc_size;
   screen->vtbl.afbc_pack = panfrost_afbc_pack;

   GENX(pan_blitter_init)
   (dev, &screen->blitter.bin_pool.base, &screen->blitter.desc_pool.base);
}
