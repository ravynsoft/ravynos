/*
 * Copyright 2021 Alyssa Rosenzweig
 * Copyright 2020-2021 Collabora, Ltd.
 * Copyright 2019 Sonny Jiang <sonnyj608@gmail.com>
 * Copyright 2019 Advanced Micro Devices, Inc.
 * Copyright 2014 Broadcom
 * SPDX-License-Identifier: MIT
 */

#include <stdint.h>
#include "asahi/compiler/agx_compile.h"
#include "asahi/layout/layout.h"
#include "compiler/nir/nir_builder.h"
#include "compiler/nir/nir_format_convert.h"
#include "gallium/auxiliary/util/u_blitter.h"
#include "gallium/auxiliary/util/u_dump.h"
#include "nir/pipe_nir.h"
#include "pipe/p_context.h"
#include "pipe/p_defines.h"
#include "pipe/p_state.h"
#include "util/format/u_format.h"
#include "util/format/u_formats.h"
#include "util/macros.h"
#include "util/u_sampler.h"
#include "util/u_surface.h"
#include "agx_formats.h"
#include "agx_state.h"
#include "shader_enums.h"

#define BLIT_WG_SIZE 32

static void *
asahi_blit_compute_shader(struct pipe_context *ctx, enum asahi_blit_clamp clamp,
                          bool array)
{
   const nir_shader_compiler_options *options =
      ctx->screen->get_compiler_options(ctx->screen, PIPE_SHADER_IR_NIR,
                                        PIPE_SHADER_COMPUTE);

   nir_builder b_ =
      nir_builder_init_simple_shader(MESA_SHADER_COMPUTE, options, "blit_cs");
   nir_builder *b = &b_;
   b->shader->info.workgroup_size[0] = BLIT_WG_SIZE;
   b->shader->info.workgroup_size[1] = BLIT_WG_SIZE;
   b->shader->info.num_ubos = 1;

   BITSET_SET(b->shader->info.textures_used, 0);
   BITSET_SET(b->shader->info.samplers_used, 0);
   BITSET_SET(b->shader->info.images_used, 0);

   nir_def *zero = nir_imm_int(b, 0);

   nir_def *params[3];
   b->shader->num_uniforms = ARRAY_SIZE(params);
   for (unsigned i = 0; i < b->shader->num_uniforms; ++i) {
      params[i] = nir_load_ubo(b, 2, 32, zero, nir_imm_int(b, i * 8),
                               .align_mul = 4, .range = ~0);
   }

   nir_def *ids =
      nir_trim_vector(b, nir_load_global_invocation_id(b, 32), array ? 3 : 2);

   nir_def *tex_pos = nir_u2f32(b, ids);
   nir_def *pos2 =
      nir_ffma(b, nir_trim_vector(b, tex_pos, 2), params[1], params[0]);
   if (array) {
      tex_pos = nir_vector_insert_imm(b, nir_pad_vector(b, pos2, 3),
                                      nir_channel(b, tex_pos, 2), 2);
   } else {
      tex_pos = pos2;
   }

   nir_tex_instr *tex = nir_tex_instr_create(b->shader, 1);
   tex->dest_type = nir_type_uint32; /* irrelevant */
   tex->sampler_dim = GLSL_SAMPLER_DIM_2D;
   tex->is_array = array;
   tex->op = nir_texop_tex;
   tex->src[0] = nir_tex_src_for_ssa(nir_tex_src_coord, tex_pos);
   tex->backend_flags = AGX_TEXTURE_FLAG_NO_CLAMP;
   tex->coord_components = array ? 3 : 2;
   tex->texture_index = 0;
   tex->sampler_index = 0;
   nir_def_init(&tex->instr, &tex->def, 4, 32);
   nir_builder_instr_insert(b, &tex->instr);
   nir_def *color = &tex->def;

   if (clamp == ASAHI_BLIT_CLAMP_SINT_TO_UINT)
      color = nir_imax(b, color, nir_imm_int(b, 0));
   else if (clamp == ASAHI_BLIT_CLAMP_UINT_TO_SINT)
      color = nir_umin(b, color, nir_imm_int(b, INT32_MAX));

   nir_def *image_pos =
      nir_iadd(b, ids, nir_pad_vector_imm_int(b, params[2], 0, array ? 3 : 2));

   nir_image_store(b, nir_imm_int(b, 0), nir_pad_vec4(b, image_pos), zero,
                   color, zero, .image_dim = GLSL_SAMPLER_DIM_2D,
                   .access = ACCESS_NON_READABLE, .image_array = array);

   return pipe_shader_from_nir(ctx, b->shader);
}

static bool
asahi_compute_blit_supported(const struct pipe_blit_info *info)
{
   /* XXX: Hot fix. compute blits broken on G13X? needs investigation */
   return false;
#if 0
   return (info->src.box.depth == info->dst.box.depth) && !info->alpha_blend &&
          !info->num_window_rectangles && !info->sample0_only &&
          !info->scissor_enable && !info->window_rectangle_include &&
          info->src.resource->nr_samples <= 1 &&
          info->dst.resource->nr_samples <= 1 &&
          !util_format_is_depth_and_stencil(info->src.format) &&
          !util_format_is_depth_and_stencil(info->dst.format) &&
          info->src.box.depth >= 0 &&
          info->mask == util_format_get_mask(info->src.format) &&
          /* XXX: texsubimage pbo failing otherwise, needs investigation */
          info->dst.format != PIPE_FORMAT_B5G6R5_UNORM &&
          info->dst.format != PIPE_FORMAT_B5G5R5A1_UNORM &&
          info->dst.format != PIPE_FORMAT_B5G5R5X1_UNORM &&
          info->dst.format != PIPE_FORMAT_R5G6B5_UNORM &&
          info->dst.format != PIPE_FORMAT_R5G5B5A1_UNORM &&
          info->dst.format != PIPE_FORMAT_R5G5B5X1_UNORM;
#endif
}

static void
asahi_compute_save(struct agx_context *ctx)
{
   struct asahi_blitter *blitter = &ctx->compute_blitter;
   struct agx_stage *stage = &ctx->stage[PIPE_SHADER_COMPUTE];

   assert(!blitter->active && "recursion detected, driver bug");

   pipe_resource_reference(&blitter->saved_cb.buffer, stage->cb[0].buffer);
   memcpy(&blitter->saved_cb, &stage->cb[0],
          sizeof(struct pipe_constant_buffer));

   blitter->has_saved_image = stage->image_mask & BITFIELD_BIT(0);
   if (blitter->has_saved_image) {
      pipe_resource_reference(&blitter->saved_image.resource,
                              stage->images[0].resource);
      memcpy(&blitter->saved_image, &stage->images[0],
             sizeof(struct pipe_image_view));
   }

   pipe_sampler_view_reference(&blitter->saved_sampler_view,
                               &stage->textures[0]->base);

   blitter->saved_num_sampler_states = stage->sampler_count;
   memcpy(blitter->saved_sampler_states, stage->samplers,
          stage->sampler_count * sizeof(void *));

   blitter->saved_cs = stage->shader;
   blitter->active = true;
}

static void
asahi_compute_restore(struct agx_context *ctx)
{
   struct pipe_context *pctx = &ctx->base;
   struct asahi_blitter *blitter = &ctx->compute_blitter;

   if (blitter->has_saved_image) {
      pctx->set_shader_images(pctx, PIPE_SHADER_COMPUTE, 0, 1, 0,
                              &blitter->saved_image);
      pipe_resource_reference(&blitter->saved_image.resource, NULL);
   }

   /* take_ownership=true so do not unreference */
   pctx->set_constant_buffer(pctx, PIPE_SHADER_COMPUTE, 0, true,
                             &blitter->saved_cb);
   blitter->saved_cb.buffer = NULL;

   if (blitter->saved_sampler_view) {
      pctx->set_sampler_views(pctx, PIPE_SHADER_COMPUTE, 0, 1, 0, true,
                              &blitter->saved_sampler_view);

      blitter->saved_sampler_view = NULL;
   }

   if (blitter->saved_num_sampler_states) {
      pctx->bind_sampler_states(pctx, PIPE_SHADER_COMPUTE, 0,
                                blitter->saved_num_sampler_states,
                                blitter->saved_sampler_states);
   }

   pctx->bind_compute_state(pctx, blitter->saved_cs);
   blitter->saved_cs = NULL;
   blitter->active = false;
}

static void
asahi_compute_blit(struct pipe_context *ctx, const struct pipe_blit_info *info,
                   struct asahi_blitter *blitter)
{
   if (info->src.box.width == 0 || info->src.box.height == 0 ||
       info->dst.box.width == 0 || info->dst.box.height == 0)
      return;

   assert(asahi_compute_blit_supported(info));
   asahi_compute_save(agx_context(ctx));

   unsigned depth = info->dst.box.depth;
   bool array = depth > 1;

   struct pipe_resource *src = info->src.resource;
   struct pipe_resource *dst = info->dst.resource;
   struct pipe_sampler_view src_templ = {0}, *src_view;
   unsigned width = info->dst.box.width;
   unsigned height = info->dst.box.height;

   float src_width = (float)u_minify(src->width0, info->src.level);
   float src_height = (float)u_minify(src->height0, info->src.level);

   float x_scale = (info->src.box.width / (float)width) / src_width;
   float y_scale = (info->src.box.height / (float)height) / src_height;

   unsigned data[] = {
      fui(0.5f * x_scale + (float)info->src.box.x / src_width),
      fui(0.5f * y_scale + (float)info->src.box.y / src_height),
      fui(x_scale),
      fui(y_scale),
      info->dst.box.x,
      info->dst.box.y,
   };

   struct pipe_constant_buffer cb = {
      .buffer_size = sizeof(data),
      .user_buffer = data,
   };
   ctx->set_constant_buffer(ctx, PIPE_SHADER_COMPUTE, 0, false, &cb);

   struct pipe_image_view image = {
      .resource = dst,
      .access = PIPE_IMAGE_ACCESS_WRITE | PIPE_IMAGE_ACCESS_DRIVER_INTERNAL,
      .shader_access = PIPE_IMAGE_ACCESS_WRITE,
      .format = info->dst.format,
      .u.tex.level = info->dst.level,
      .u.tex.first_layer = info->dst.box.z,
      .u.tex.last_layer = info->dst.box.z + depth - 1,
      .u.tex.single_layer_view = !array,
   };
   ctx->set_shader_images(ctx, PIPE_SHADER_COMPUTE, 0, 1, 0, &image);

   if (!blitter->sampler[info->filter]) {
      struct pipe_sampler_state sampler_state = {
         .wrap_s = PIPE_TEX_WRAP_CLAMP_TO_EDGE,
         .wrap_t = PIPE_TEX_WRAP_CLAMP_TO_EDGE,
         .wrap_r = PIPE_TEX_WRAP_CLAMP_TO_EDGE,
         .min_img_filter = info->filter,
         .mag_img_filter = info->filter,
         .compare_func = PIPE_FUNC_ALWAYS,
         .seamless_cube_map = true,
         .max_lod = 31.0f,
      };

      blitter->sampler[info->filter] =
         ctx->create_sampler_state(ctx, &sampler_state);
   }

   ctx->bind_sampler_states(ctx, PIPE_SHADER_COMPUTE, 0, 1,
                            &blitter->sampler[info->filter]);

   /* Initialize the sampler view. */
   u_sampler_view_default_template(&src_templ, src, src->format);
   src_templ.format = info->src.format;
   src_templ.target = array ? PIPE_TEXTURE_2D_ARRAY : PIPE_TEXTURE_2D;
   src_templ.swizzle_r = PIPE_SWIZZLE_X;
   src_templ.swizzle_g = PIPE_SWIZZLE_Y;
   src_templ.swizzle_b = PIPE_SWIZZLE_Z;
   src_templ.swizzle_a = PIPE_SWIZZLE_W;
   src_templ.u.tex.first_layer = info->src.box.z;
   src_templ.u.tex.last_layer = info->src.box.z + depth - 1;
   src_templ.u.tex.first_level = info->src.level;
   src_templ.u.tex.last_level = info->src.level;
   src_view = ctx->create_sampler_view(ctx, src, &src_templ);
   ctx->set_sampler_views(ctx, PIPE_SHADER_COMPUTE, 0, 1, 0, true, &src_view);

   enum asahi_blit_clamp clamp = ASAHI_BLIT_CLAMP_NONE;
   bool src_sint = util_format_is_pure_sint(info->src.format);
   bool dst_sint = util_format_is_pure_sint(info->dst.format);
   if (util_format_is_pure_integer(info->src.format) &&
       util_format_is_pure_integer(info->dst.format)) {

      if (src_sint && !dst_sint)
         clamp = ASAHI_BLIT_CLAMP_SINT_TO_UINT;
      else if (!src_sint && dst_sint)
         clamp = ASAHI_BLIT_CLAMP_UINT_TO_SINT;
   }

   if (!blitter->blit_cs[clamp][array]) {
      blitter->blit_cs[clamp][array] =
         asahi_blit_compute_shader(ctx, clamp, array);
   }

   ctx->bind_compute_state(ctx, blitter->blit_cs[clamp][array]);

   struct pipe_grid_info grid_info = {
      .block = {BLIT_WG_SIZE, BLIT_WG_SIZE, 1},
      .last_block = {width % BLIT_WG_SIZE, height % BLIT_WG_SIZE, 1},
      .grid =
         {
            DIV_ROUND_UP(width, BLIT_WG_SIZE),
            DIV_ROUND_UP(height, BLIT_WG_SIZE),
            depth,
         },
   };
   ctx->launch_grid(ctx, &grid_info);
   ctx->set_shader_images(ctx, PIPE_SHADER_COMPUTE, 0, 0, 1, NULL);
   ctx->set_constant_buffer(ctx, PIPE_SHADER_COMPUTE, 0, false, NULL);
   ctx->set_sampler_views(ctx, PIPE_SHADER_COMPUTE, 0, 0, 1, false, NULL);

   asahi_compute_restore(agx_context(ctx));
}

void
agx_blitter_save(struct agx_context *ctx, struct blitter_context *blitter,
                 bool render_cond)
{
   util_blitter_save_vertex_buffer_slot(blitter, ctx->vertex_buffers);
   util_blitter_save_vertex_elements(blitter, ctx->attributes);
   util_blitter_save_vertex_shader(blitter,
                                   ctx->stage[PIPE_SHADER_VERTEX].shader);
   util_blitter_save_geometry_shader(blitter,
                                     ctx->stage[PIPE_SHADER_GEOMETRY].shader);
   util_blitter_save_rasterizer(blitter, ctx->rast);
   util_blitter_save_viewport(blitter, &ctx->viewport[0]);
   util_blitter_save_scissor(blitter, &ctx->scissor[0]);
   util_blitter_save_fragment_shader(blitter,
                                     ctx->stage[PIPE_SHADER_FRAGMENT].shader);
   util_blitter_save_blend(blitter, ctx->blend);
   util_blitter_save_depth_stencil_alpha(blitter, ctx->zs);
   util_blitter_save_stencil_ref(blitter, &ctx->stencil_ref);
   util_blitter_save_so_targets(blitter, ctx->streamout.num_targets,
                                ctx->streamout.targets);
   util_blitter_save_sample_mask(blitter, ctx->sample_mask, 0);

   util_blitter_save_framebuffer(blitter, &ctx->framebuffer);
   util_blitter_save_fragment_sampler_states(
      blitter, ctx->stage[PIPE_SHADER_FRAGMENT].sampler_count,
      (void **)(ctx->stage[PIPE_SHADER_FRAGMENT].samplers));
   util_blitter_save_fragment_sampler_views(
      blitter, ctx->stage[PIPE_SHADER_FRAGMENT].texture_count,
      (struct pipe_sampler_view **)ctx->stage[PIPE_SHADER_FRAGMENT].textures);
   util_blitter_save_fragment_constant_buffer_slot(
      blitter, ctx->stage[PIPE_SHADER_FRAGMENT].cb);

   if (!render_cond) {
      util_blitter_save_render_condition(blitter,
                                         (struct pipe_query *)ctx->cond_query,
                                         ctx->cond_cond, ctx->cond_mode);
   }
}

void
agx_blit(struct pipe_context *pipe, const struct pipe_blit_info *info)
{
   struct agx_context *ctx = agx_context(pipe);

   if (info->render_condition_enable && !agx_render_condition_check(ctx))
      return;

   if (!util_blitter_is_blit_supported(ctx->blitter, info)) {
      fprintf(stderr, "\n");
      util_dump_blit_info(stderr, info);
      fprintf(stderr, "\n\n");
      unreachable("Unsupported blit");
   }

   /* Legalize compression /before/ calling into u_blitter to avoid recursion.
    * u_blitter bans recursive usage.
    */
   agx_legalize_compression(ctx, agx_resource(info->dst.resource),
                            info->dst.format);

   agx_legalize_compression(ctx, agx_resource(info->src.resource),
                            info->src.format);

   if (asahi_compute_blit_supported(info) &&
       !(ail_is_compressed(&agx_resource(info->dst.resource)->layout) &&
         util_format_get_blocksize(info->dst.format) == 16)) {

      asahi_compute_blit(pipe, info, &ctx->compute_blitter);
      return;
   }

   /* Handle self-blits */
   agx_flush_writer(ctx, agx_resource(info->dst.resource), "Blit");

   agx_blitter_save(ctx, ctx->blitter, info->render_condition_enable);
   util_blitter_blit(ctx->blitter, info);
}

static bool
try_copy_via_blit(struct pipe_context *pctx, struct pipe_resource *dst,
                  unsigned dst_level, unsigned dstx, unsigned dsty,
                  unsigned dstz, struct pipe_resource *src, unsigned src_level,
                  const struct pipe_box *src_box)
{
   struct agx_context *ctx = agx_context(pctx);

   if (dst->target == PIPE_BUFFER)
      return false;

   /* TODO: Handle these for rusticl copies */
   if (dst->target != src->target)
      return false;

   /* TODO: float formats don't roundtrip, cast */
   if (util_format_is_float(dst->format) || util_format_is_float(src->format))
      return false;

   struct pipe_blit_info info = {
      .dst =
         {
            .resource = dst,
            .level = dst_level,
            .box.x = dstx,
            .box.y = dsty,
            .box.z = dstz,
            .box.width = src_box->width,
            .box.height = src_box->height,
            .box.depth = src_box->depth,
            .format = dst->format,
         },
      .src =
         {
            .resource = src,
            .level = src_level,
            .box = *src_box,
            .format = src->format,
         },
      .mask = util_format_get_mask(src->format),
      .filter = PIPE_TEX_FILTER_NEAREST,
      .scissor_enable = 0,
   };

   /* snorm formats don't round trip, so don't use them for copies */
   if (util_format_is_snorm(info.dst.format))
      info.dst.format = util_format_snorm_to_sint(info.dst.format);

   if (util_format_is_snorm(info.src.format))
      info.src.format = util_format_snorm_to_sint(info.src.format);

   if (util_blitter_is_blit_supported(ctx->blitter, &info) &&
       info.dst.format == info.src.format) {

      agx_blit(pctx, &info);
      return true;
   } else {
      return false;
   }
}

void
agx_resource_copy_region(struct pipe_context *pctx, struct pipe_resource *dst,
                         unsigned dst_level, unsigned dstx, unsigned dsty,
                         unsigned dstz, struct pipe_resource *src,
                         unsigned src_level, const struct pipe_box *src_box)
{
   if (try_copy_via_blit(pctx, dst, dst_level, dstx, dsty, dstz, src, src_level,
                         src_box))
      return;

   /* CPU fallback */
   util_resource_copy_region(pctx, dst, dst_level, dstx, dsty, dstz, src,
                             src_level, src_box);
}
