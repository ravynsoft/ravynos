/**************************************************************************
 *
 * Copyright 2009 Marek Olšák <maraeo@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

/**
 * @file
 * Blitter utility to facilitate acceleration of the clear, clear_render_target,
 * clear_depth_stencil, resource_copy_region, and blit functions.
 *
 * @author Marek Olšák
 */

#include "pipe/p_context.h"
#include "pipe/p_defines.h"
#include "util/u_inlines.h"
#include "pipe/p_shader_tokens.h"
#include "pipe/p_state.h"

#include "util/format/u_format.h"
#include "util/u_memory.h"
#include "util/u_math.h"
#include "util/u_blitter.h"
#include "util/u_draw_quad.h"
#include "util/u_sampler.h"
#include "util/u_simple_shaders.h"
#include "util/u_surface.h"
#include "util/u_texture.h"
#include "util/u_upload_mgr.h"

#define INVALID_PTR ((void*)~0)

#define GET_CLEAR_BLEND_STATE_IDX(clear_buffers) \
   ((clear_buffers) / PIPE_CLEAR_COLOR0)

#define NUM_RESOLVE_FRAG_SHADERS 5 /* MSAA 2x, 4x, 8x, 16x, 32x */
#define GET_MSAA_RESOLVE_FS_IDX(nr_samples) (util_logbase2(nr_samples)-1)

struct blitter_context_priv
{
   struct blitter_context base;

   float vertices[4][2][4];   /**< {pos, color} or {pos, texcoord} */

   /* Templates for various state objects. */

   /* Constant state objects. */
   /* Vertex shaders. */
   void *vs; /**< Vertex shader which passes {pos, generic} to the output.*/
   void *vs_nogeneric;
   void *vs_pos_only[4]; /**< Vertex shader which passes pos to the output
                              for clear_buffer.*/
   void *vs_layered; /**< Vertex shader which sets LAYER = INSTANCEID. */

   /* Fragment shaders. */
   void *fs_empty;
   void *fs_write_one_cbuf;
   void *fs_clear_all_cbufs;

   /* FS which outputs a color from a texture where
    * the 1st index indicates the texture type / destination type,
    * the 2nd index is the PIPE_TEXTURE_* to be sampled,
    * the 3rd index is 0 = use TEX, 1 = use TXF.
    */
   void *fs_texfetch_col[5][PIPE_MAX_TEXTURE_TYPES][2];

   /* FS which outputs a depth from a texture, where
    * the 1st index is the PIPE_TEXTURE_* to be sampled,
    * the 2nd index is 0 = use TEX, 1 = use TXF.
    */
   void *fs_texfetch_depth[PIPE_MAX_TEXTURE_TYPES][2];
   void *fs_texfetch_depthstencil[PIPE_MAX_TEXTURE_TYPES][2];
   void *fs_texfetch_stencil[PIPE_MAX_TEXTURE_TYPES][2];

   /* FS which outputs one sample from a multisample texture. */
   void *fs_texfetch_col_msaa[5][PIPE_MAX_TEXTURE_TYPES];
   void *fs_texfetch_depth_msaa[PIPE_MAX_TEXTURE_TYPES][2];
   void *fs_texfetch_depthstencil_msaa[PIPE_MAX_TEXTURE_TYPES][2];
   void *fs_texfetch_stencil_msaa[PIPE_MAX_TEXTURE_TYPES][2];

   /* FS which outputs an average of all samples. */
   void *fs_resolve[PIPE_MAX_TEXTURE_TYPES][NUM_RESOLVE_FRAG_SHADERS][2];

   /* FS which unpacks color to ZS or packs ZS to color, matching
    * the ZS format. See util_blitter_get_color_format_for_zs().
    */
   void *fs_pack_color_zs[TGSI_TEXTURE_COUNT][10];

   /* FS which is meant for replicating indevidual stencil-buffer bits */
   void *fs_stencil_blit_fallback[2];

   /* Blend state. */
   void *blend[PIPE_MASK_RGBA+1][2]; /**< blend state with writemask */
   void *blend_clear[GET_CLEAR_BLEND_STATE_IDX(PIPE_CLEAR_COLOR)+1];

   /* Depth stencil alpha state. */
   void *dsa_write_depth_stencil;
   void *dsa_write_depth_keep_stencil;
   void *dsa_keep_depth_stencil;
   void *dsa_keep_depth_write_stencil;
   void *dsa_replicate_stencil_bit[8];

   /* Vertex elements states. */
   void *velem_state;
   void *velem_state_readbuf[4]; /**< X, XY, XYZ, XYZW */

   /* Sampler state. */
   void *sampler_state;
   void *sampler_state_linear;
   void *sampler_state_rect;
   void *sampler_state_rect_linear;

   /* Rasterizer state. */
   void *rs_state[2][2];  /**< [scissor][msaa] */
   void *rs_discard_state;

   /* Destination surface dimensions. */
   unsigned dst_width;
   unsigned dst_height;

   void *custom_vs;

   bool has_geometry_shader;
   bool has_tessellation;
   bool has_layered;
   bool has_stream_out;
   bool has_stencil_export;
   bool has_texture_multisample;
   bool has_tex_lz;
   bool has_txf_txq;
   bool has_sample_shading;
   bool cube_as_2darray;
   bool has_texrect;
   bool cached_all_shaders;

   /* The Draw module overrides these functions.
    * Always create the blitter before Draw. */
   void   (*bind_fs_state)(struct pipe_context *, void *);
   void   (*delete_fs_state)(struct pipe_context *, void *);
};

struct blitter_context *util_blitter_create(struct pipe_context *pipe)
{
   struct blitter_context_priv *ctx;
   struct pipe_blend_state blend;
   struct pipe_depth_stencil_alpha_state dsa;
   struct pipe_rasterizer_state rs_state;
   struct pipe_sampler_state sampler_state;
   struct pipe_vertex_element velem[2];
   unsigned i, j;

   ctx = CALLOC_STRUCT(blitter_context_priv);
   if (!ctx)
      return NULL;

   ctx->base.pipe = pipe;
   ctx->base.draw_rectangle = util_blitter_draw_rectangle;

   ctx->bind_fs_state = pipe->bind_fs_state;
   ctx->delete_fs_state = pipe->delete_fs_state;

   /* init state objects for them to be considered invalid */
   ctx->base.saved_blend_state = INVALID_PTR;
   ctx->base.saved_dsa_state = INVALID_PTR;
   ctx->base.saved_rs_state = INVALID_PTR;
   ctx->base.saved_fs = INVALID_PTR;
   ctx->base.saved_vs = INVALID_PTR;
   ctx->base.saved_gs = INVALID_PTR;
   ctx->base.saved_velem_state = INVALID_PTR;
   ctx->base.saved_fb_state.nr_cbufs = ~0;
   ctx->base.saved_num_sampler_views = ~0;
   ctx->base.saved_num_sampler_states = ~0;
   ctx->base.saved_num_so_targets = ~0;

   ctx->has_geometry_shader =
      pipe->screen->get_shader_param(pipe->screen, PIPE_SHADER_GEOMETRY,
                                     PIPE_SHADER_CAP_MAX_INSTRUCTIONS) > 0;

   ctx->has_tessellation =
      pipe->screen->get_shader_param(pipe->screen, PIPE_SHADER_TESS_CTRL,
                                     PIPE_SHADER_CAP_MAX_INSTRUCTIONS) > 0;

   ctx->has_stream_out =
      pipe->screen->get_param(pipe->screen,
                              PIPE_CAP_MAX_STREAM_OUTPUT_BUFFERS) != 0;

   ctx->has_stencil_export =
         pipe->screen->get_param(pipe->screen,
                                 PIPE_CAP_SHADER_STENCIL_EXPORT);

   ctx->has_texture_multisample =
      pipe->screen->get_param(pipe->screen, PIPE_CAP_TEXTURE_MULTISAMPLE);

   ctx->has_tex_lz = pipe->screen->get_param(pipe->screen,
                                             PIPE_CAP_TGSI_TEX_TXF_LZ);
   ctx->has_txf_txq = pipe->screen->get_param(pipe->screen,
                                              PIPE_CAP_GLSL_FEATURE_LEVEL) >= 130;
   ctx->has_sample_shading = pipe->screen->get_param(pipe->screen,
                                                     PIPE_CAP_SAMPLE_SHADING);
   ctx->cube_as_2darray = pipe->screen->get_param(pipe->screen,
                                                  PIPE_CAP_SAMPLER_VIEW_TARGET);
   ctx->has_texrect = pipe->screen->get_param(pipe->screen, PIPE_CAP_TEXRECT);

   /* blend state objects */
   memset(&blend, 0, sizeof(blend));

   for (i = 0; i <= PIPE_MASK_RGBA; i++) {
      for (j = 0; j < 2; j++) {
         memset(&blend.rt[0], 0, sizeof(blend.rt[0]));
         blend.rt[0].colormask = i;
         if (j) {
            blend.rt[0].blend_enable = 1;
            blend.rt[0].rgb_func = PIPE_BLEND_ADD;
            blend.rt[0].rgb_src_factor = PIPE_BLENDFACTOR_SRC_ALPHA;
            blend.rt[0].rgb_dst_factor = PIPE_BLENDFACTOR_INV_SRC_ALPHA;
            blend.rt[0].alpha_func = PIPE_BLEND_ADD;
            blend.rt[0].alpha_src_factor = PIPE_BLENDFACTOR_SRC_ALPHA;
            blend.rt[0].alpha_dst_factor = PIPE_BLENDFACTOR_INV_SRC_ALPHA;
         }
         ctx->blend[i][j] = pipe->create_blend_state(pipe, &blend);
      }
   }

   /* depth stencil alpha state objects */
   memset(&dsa, 0, sizeof(dsa));
   ctx->dsa_keep_depth_stencil =
      pipe->create_depth_stencil_alpha_state(pipe, &dsa);

   dsa.depth_enabled = 1;
   dsa.depth_writemask = 1;
   dsa.depth_func = PIPE_FUNC_ALWAYS;
   ctx->dsa_write_depth_keep_stencil =
      pipe->create_depth_stencil_alpha_state(pipe, &dsa);

   dsa.stencil[0].enabled = 1;
   dsa.stencil[0].func = PIPE_FUNC_ALWAYS;
   dsa.stencil[0].fail_op = PIPE_STENCIL_OP_REPLACE;
   dsa.stencil[0].zpass_op = PIPE_STENCIL_OP_REPLACE;
   dsa.stencil[0].zfail_op = PIPE_STENCIL_OP_REPLACE;
   dsa.stencil[0].valuemask = 0xff;
   dsa.stencil[0].writemask = 0xff;
   ctx->dsa_write_depth_stencil =
      pipe->create_depth_stencil_alpha_state(pipe, &dsa);

   dsa.depth_enabled = 0;
   dsa.depth_writemask = 0;
   ctx->dsa_keep_depth_write_stencil =
      pipe->create_depth_stencil_alpha_state(pipe, &dsa);

   /* sampler state */
   memset(&sampler_state, 0, sizeof(sampler_state));
   sampler_state.wrap_s = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
   sampler_state.wrap_t = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
   sampler_state.wrap_r = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
   sampler_state.unnormalized_coords = 0;
   ctx->sampler_state = pipe->create_sampler_state(pipe, &sampler_state);
   if (ctx->has_texrect) {
      sampler_state.unnormalized_coords = 1;
      ctx->sampler_state_rect = pipe->create_sampler_state(pipe, &sampler_state);
   }

   sampler_state.min_img_filter = PIPE_TEX_FILTER_LINEAR;
   sampler_state.mag_img_filter = PIPE_TEX_FILTER_LINEAR;
   sampler_state.unnormalized_coords = 0;
   ctx->sampler_state_linear = pipe->create_sampler_state(pipe, &sampler_state);
   if (ctx->has_texrect) {
      sampler_state.unnormalized_coords = 1;
      ctx->sampler_state_rect_linear = pipe->create_sampler_state(pipe, &sampler_state);
   }

   /* rasterizer state */
   memset(&rs_state, 0, sizeof(rs_state));
   rs_state.cull_face = PIPE_FACE_NONE;
   rs_state.half_pixel_center = 1;
   rs_state.bottom_edge_rule = 1;
   rs_state.flatshade = 1;
   rs_state.depth_clip_near = 1;
   rs_state.depth_clip_far = 1;

   unsigned scissor, msaa;
   for (scissor = 0; scissor < 2; scissor++) {
      for (msaa = 0; msaa < 2; msaa++) {
         rs_state.scissor = scissor;
         rs_state.multisample = msaa;
         ctx->rs_state[scissor][msaa] =
            pipe->create_rasterizer_state(pipe, &rs_state);
      }
   }

   if (ctx->has_stream_out) {
      rs_state.scissor = rs_state.multisample = 0;
      rs_state.rasterizer_discard = 1;
      ctx->rs_discard_state = pipe->create_rasterizer_state(pipe, &rs_state);
   }

   ctx->base.cb_slot = 0; /* 0 for now */

   /* vertex elements states */
   memset(&velem[0], 0, sizeof(velem[0]) * 2);
   for (i = 0; i < 2; i++) {
      velem[i].src_offset = i * 4 * sizeof(float);
      velem[i].src_stride = 8 * sizeof(float);
      velem[i].src_format = PIPE_FORMAT_R32G32B32A32_FLOAT;
      velem[i].vertex_buffer_index = 0;
   }
   ctx->velem_state = pipe->create_vertex_elements_state(pipe, 2, &velem[0]);

   if (ctx->has_stream_out) {
      static enum pipe_format formats[4] = {
         PIPE_FORMAT_R32_UINT,
         PIPE_FORMAT_R32G32_UINT,
         PIPE_FORMAT_R32G32B32_UINT,
         PIPE_FORMAT_R32G32B32A32_UINT
      };

      for (i = 0; i < 4; i++) {
         velem[0].src_format = formats[i];
         velem[0].vertex_buffer_index = 0;
         velem[0].src_stride = 0;
         ctx->velem_state_readbuf[i] =
               pipe->create_vertex_elements_state(pipe, 1, &velem[0]);
      }
   }

   ctx->has_layered =
      pipe->screen->get_param(pipe->screen, PIPE_CAP_VS_INSTANCEID) &&
      pipe->screen->get_param(pipe->screen, PIPE_CAP_VS_LAYER_VIEWPORT);

   /* set invariant vertex coordinates */
   for (i = 0; i < 4; i++) {
      ctx->vertices[i][0][2] = 0; /*v.z*/
      ctx->vertices[i][0][3] = 1; /*v.w*/
   }

   return &ctx->base;
}

void *util_blitter_get_noop_blend_state(struct blitter_context *blitter)
{
   struct blitter_context_priv *ctx = (struct blitter_context_priv*)blitter;

   return ctx->blend[0][0];
}

void *util_blitter_get_noop_dsa_state(struct blitter_context *blitter)
{
   struct blitter_context_priv *ctx = (struct blitter_context_priv*)blitter;

   return ctx->dsa_keep_depth_stencil;
}

void *util_blitter_get_discard_rasterizer_state(struct blitter_context *blitter)
{
   struct blitter_context_priv *ctx = (struct blitter_context_priv*)blitter;

   return ctx->rs_discard_state;
}

static void bind_vs_pos_only(struct blitter_context_priv *ctx,
                             unsigned num_so_channels)
{
   struct pipe_context *pipe = ctx->base.pipe;
   int index = num_so_channels ? num_so_channels - 1 : 0;

   if (!ctx->vs_pos_only[index]) {
      struct pipe_stream_output_info so;
      static const enum tgsi_semantic semantic_names[] =
         { TGSI_SEMANTIC_POSITION };
      const unsigned semantic_indices[] = { 0 };

      memset(&so, 0, sizeof(so));
      so.num_outputs = 1;
      so.output[0].num_components = num_so_channels;
      so.stride[0] = num_so_channels;

      ctx->vs_pos_only[index] =
         util_make_vertex_passthrough_shader_with_so(pipe, 1, semantic_names,
                                                     semantic_indices, false,
                                                     false, &so);
   }

   pipe->bind_vs_state(pipe, ctx->vs_pos_only[index]);
}

static void *get_vs_passthrough_pos_generic(struct blitter_context *blitter)
{
   struct blitter_context_priv *ctx = (struct blitter_context_priv*)blitter;
   struct pipe_context *pipe = ctx->base.pipe;

   if (!ctx->vs) {
      static const enum tgsi_semantic semantic_names[] =
         { TGSI_SEMANTIC_POSITION, TGSI_SEMANTIC_GENERIC };
      const unsigned semantic_indices[] = { 0, 0 };
      ctx->vs =
         util_make_vertex_passthrough_shader(pipe, 2, semantic_names,
                                             semantic_indices, false);
   }
   return ctx->vs;
}

static void *get_vs_passthrough_pos(struct blitter_context *blitter)
{
   struct blitter_context_priv *ctx = (struct blitter_context_priv*)blitter;
   struct pipe_context *pipe = ctx->base.pipe;

   if (!ctx->vs_nogeneric) {
      static const enum tgsi_semantic semantic_names[] =
         { TGSI_SEMANTIC_POSITION };
      const unsigned semantic_indices[] = { 0 };

      ctx->vs_nogeneric =
         util_make_vertex_passthrough_shader(pipe, 1,
                                             semantic_names,
                                             semantic_indices, false);
   }
   return ctx->vs_nogeneric;
}

static void *get_vs_layered(struct blitter_context *blitter)
{
   struct blitter_context_priv *ctx = (struct blitter_context_priv*)blitter;
   struct pipe_context *pipe = ctx->base.pipe;

   if (!ctx->vs_layered) {
      ctx->vs_layered = util_make_layered_clear_vertex_shader(pipe);
   }
   return ctx->vs_layered;
}

static void bind_fs_empty(struct blitter_context_priv *ctx)
{
   struct pipe_context *pipe = ctx->base.pipe;

   if (!ctx->fs_empty) {
      assert(!ctx->cached_all_shaders);
      ctx->fs_empty = util_make_empty_fragment_shader(pipe);
   }

   ctx->bind_fs_state(pipe, ctx->fs_empty);
}

static void bind_fs_write_one_cbuf(struct blitter_context_priv *ctx)
{
   struct pipe_context *pipe = ctx->base.pipe;

   if (!ctx->fs_write_one_cbuf) {
      assert(!ctx->cached_all_shaders);
      ctx->fs_write_one_cbuf =
         util_make_fragment_passthrough_shader(pipe, TGSI_SEMANTIC_GENERIC,
                                               TGSI_INTERPOLATE_CONSTANT, false);
   }

   ctx->bind_fs_state(pipe, ctx->fs_write_one_cbuf);
}

static void bind_fs_clear_all_cbufs(struct blitter_context_priv *ctx)
{
   struct pipe_context *pipe = ctx->base.pipe;

   if (!ctx->fs_clear_all_cbufs) {
      assert(!ctx->cached_all_shaders);
      ctx->fs_clear_all_cbufs = util_make_fs_clear_all_cbufs(pipe);
   }

   ctx->bind_fs_state(pipe, ctx->fs_clear_all_cbufs);
}

void util_blitter_destroy(struct blitter_context *blitter)
{
   struct blitter_context_priv *ctx = (struct blitter_context_priv*)blitter;
   struct pipe_context *pipe = blitter->pipe;
   unsigned i, j, f;

   for (i = 0; i <= PIPE_MASK_RGBA; i++)
      for (j = 0; j < 2; j++)
         pipe->delete_blend_state(pipe, ctx->blend[i][j]);

   for (i = 0; i < ARRAY_SIZE(ctx->blend_clear); i++) {
      if (ctx->blend_clear[i])
         pipe->delete_blend_state(pipe, ctx->blend_clear[i]);
   }
   pipe->delete_depth_stencil_alpha_state(pipe, ctx->dsa_keep_depth_stencil);
   pipe->delete_depth_stencil_alpha_state(pipe,
                                          ctx->dsa_write_depth_keep_stencil);
   pipe->delete_depth_stencil_alpha_state(pipe, ctx->dsa_write_depth_stencil);
   pipe->delete_depth_stencil_alpha_state(pipe, ctx->dsa_keep_depth_write_stencil);

   for (i = 0; i < ARRAY_SIZE(ctx->dsa_replicate_stencil_bit); i++) {
      if (ctx->dsa_replicate_stencil_bit[i])
         pipe->delete_depth_stencil_alpha_state(pipe, ctx->dsa_replicate_stencil_bit[i]);
   }

   unsigned scissor, msaa;
   for (scissor = 0; scissor < 2; scissor++) {
      for (msaa = 0; msaa < 2; msaa++) {
         pipe->delete_rasterizer_state(pipe, ctx->rs_state[scissor][msaa]);
      }
   }

   if (ctx->rs_discard_state)
      pipe->delete_rasterizer_state(pipe, ctx->rs_discard_state);
   if (ctx->vs)
      pipe->delete_vs_state(pipe, ctx->vs);
   if (ctx->vs_nogeneric)
      pipe->delete_vs_state(pipe, ctx->vs_nogeneric);
   for (i = 0; i < 4; i++)
      if (ctx->vs_pos_only[i])
         pipe->delete_vs_state(pipe, ctx->vs_pos_only[i]);
   if (ctx->vs_layered)
      pipe->delete_vs_state(pipe, ctx->vs_layered);
   pipe->delete_vertex_elements_state(pipe, ctx->velem_state);
   for (i = 0; i < 4; i++) {
      if (ctx->velem_state_readbuf[i]) {
         pipe->delete_vertex_elements_state(pipe, ctx->velem_state_readbuf[i]);
      }
   }

   for (i = 0; i < PIPE_MAX_TEXTURE_TYPES; i++) {
      for (unsigned type = 0; type < ARRAY_SIZE(ctx->fs_texfetch_col); ++type) {
         for (unsigned inst = 0; inst < 2; inst++) {
            if (ctx->fs_texfetch_col[type][i][inst])
               ctx->delete_fs_state(pipe, ctx->fs_texfetch_col[type][i][inst]);
         }
         if (ctx->fs_texfetch_col_msaa[type][i])
            ctx->delete_fs_state(pipe, ctx->fs_texfetch_col_msaa[type][i]);
      }

      for (unsigned inst = 0; inst < 2; inst++) {
         if (ctx->fs_texfetch_depth[i][inst])
            ctx->delete_fs_state(pipe, ctx->fs_texfetch_depth[i][inst]);
         if (ctx->fs_texfetch_depthstencil[i][inst])
            ctx->delete_fs_state(pipe, ctx->fs_texfetch_depthstencil[i][inst]);
         if (ctx->fs_texfetch_stencil[i][inst])
            ctx->delete_fs_state(pipe, ctx->fs_texfetch_stencil[i][inst]);
      }

      for (unsigned ss = 0; ss < 2; ss++) {
         if (ctx->fs_texfetch_depth_msaa[i][ss])
            ctx->delete_fs_state(pipe, ctx->fs_texfetch_depth_msaa[i][ss]);
         if (ctx->fs_texfetch_depthstencil_msaa[i][ss])
            ctx->delete_fs_state(pipe, ctx->fs_texfetch_depthstencil_msaa[i][ss]);
         if (ctx->fs_texfetch_stencil_msaa[i][ss])
            ctx->delete_fs_state(pipe, ctx->fs_texfetch_stencil_msaa[i][ss]);
      }

      for (j = 0; j< ARRAY_SIZE(ctx->fs_resolve[i]); j++)
         for (f = 0; f < 2; f++)
            if (ctx->fs_resolve[i][j][f])
               ctx->delete_fs_state(pipe, ctx->fs_resolve[i][j][f]);
   }

   for (i = 0; i < ARRAY_SIZE(ctx->fs_pack_color_zs); i++) {
      for (j = 0; j < ARRAY_SIZE(ctx->fs_pack_color_zs[0]); j++) {
         if (ctx->fs_pack_color_zs[i][j])
            ctx->delete_fs_state(pipe, ctx->fs_pack_color_zs[i][j]);
      }
   }

   if (ctx->fs_empty)
      ctx->delete_fs_state(pipe, ctx->fs_empty);
   if (ctx->fs_write_one_cbuf)
      ctx->delete_fs_state(pipe, ctx->fs_write_one_cbuf);
   if (ctx->fs_clear_all_cbufs)
      ctx->delete_fs_state(pipe, ctx->fs_clear_all_cbufs);

   for (i = 0; i < ARRAY_SIZE(ctx->fs_stencil_blit_fallback); ++i)
      if (ctx->fs_stencil_blit_fallback[i])
         ctx->delete_fs_state(pipe, ctx->fs_stencil_blit_fallback[i]);

   if (ctx->sampler_state_rect_linear)
      pipe->delete_sampler_state(pipe, ctx->sampler_state_rect_linear);
   if (ctx->sampler_state_rect)
      pipe->delete_sampler_state(pipe, ctx->sampler_state_rect);
   pipe->delete_sampler_state(pipe, ctx->sampler_state_linear);
   pipe->delete_sampler_state(pipe, ctx->sampler_state);
   FREE(ctx);
}

void util_blitter_set_texture_multisample(struct blitter_context *blitter,
                                          bool supported)
{
   struct blitter_context_priv *ctx = (struct blitter_context_priv*)blitter;

   ctx->has_texture_multisample = supported;
}

void util_blitter_set_running_flag(struct blitter_context *blitter)
{
   if (blitter->running) {
      _debug_printf("u_blitter:%i: Caught recursion. This is a driver bug.\n", __LINE__);
   }
   blitter->running = true;

   blitter->pipe->set_active_query_state(blitter->pipe, false);
}

void util_blitter_unset_running_flag(struct blitter_context *blitter)
{
   if (!blitter->running) {
      _debug_printf("u_blitter:%i: Caught recursion. This is a driver bug.\n",
                    __LINE__);
   }
   blitter->running = false;

   blitter->pipe->set_active_query_state(blitter->pipe, true);
}

static void blitter_check_saved_vertex_states(ASSERTED struct blitter_context_priv *ctx)
{
   assert(ctx->base.saved_vs != INVALID_PTR);
   assert(!ctx->has_geometry_shader || ctx->base.saved_gs != INVALID_PTR);
   assert(!ctx->has_tessellation || ctx->base.saved_tcs != INVALID_PTR);
   assert(!ctx->has_tessellation || ctx->base.saved_tes != INVALID_PTR);
   assert(!ctx->has_stream_out || ctx->base.saved_num_so_targets != ~0u);
   assert(ctx->base.saved_rs_state != INVALID_PTR);
}

void util_blitter_restore_vertex_states(struct blitter_context *blitter)
{
   struct blitter_context_priv *ctx = (struct blitter_context_priv*)blitter;
   struct pipe_context *pipe = ctx->base.pipe;
   unsigned i;

   /* Vertex buffer. */
   if (ctx->base.saved_vertex_buffer.buffer.resource) {
      pipe->set_vertex_buffers(pipe, 1, 0, true,
                               &ctx->base.saved_vertex_buffer);
      ctx->base.saved_vertex_buffer.buffer.resource = NULL;
   }

   /* Vertex elements. */
   if (ctx->base.saved_velem_state != INVALID_PTR) {
      pipe->bind_vertex_elements_state(pipe, ctx->base.saved_velem_state);
      ctx->base.saved_velem_state = INVALID_PTR;
   }

   /* Vertex shader. */
   pipe->bind_vs_state(pipe, ctx->base.saved_vs);
   ctx->base.saved_vs = INVALID_PTR;

   /* Geometry shader. */
   if (ctx->has_geometry_shader) {
      pipe->bind_gs_state(pipe, ctx->base.saved_gs);
      ctx->base.saved_gs = INVALID_PTR;
   }

   if (ctx->has_tessellation) {
      pipe->bind_tcs_state(pipe, ctx->base.saved_tcs);
      pipe->bind_tes_state(pipe, ctx->base.saved_tes);
      ctx->base.saved_tcs = INVALID_PTR;
      ctx->base.saved_tes = INVALID_PTR;
   }

   /* Stream outputs. */
   if (ctx->has_stream_out) {
      unsigned offsets[PIPE_MAX_SO_BUFFERS];
      for (i = 0; i < ctx->base.saved_num_so_targets; i++)
         offsets[i] = (unsigned)-1;
      pipe->set_stream_output_targets(pipe,
                                      ctx->base.saved_num_so_targets,
                                      ctx->base.saved_so_targets, offsets);

      for (i = 0; i < ctx->base.saved_num_so_targets; i++)
         pipe_so_target_reference(&ctx->base.saved_so_targets[i], NULL);

      ctx->base.saved_num_so_targets = ~0;
   }

   /* Rasterizer. */
   pipe->bind_rasterizer_state(pipe, ctx->base.saved_rs_state);
   ctx->base.saved_rs_state = INVALID_PTR;
}

static void blitter_check_saved_fragment_states(ASSERTED struct blitter_context_priv *ctx)
{
   assert(ctx->base.saved_fs != INVALID_PTR);
   assert(ctx->base.saved_dsa_state != INVALID_PTR);
   assert(ctx->base.saved_blend_state != INVALID_PTR);
}

void util_blitter_restore_fragment_states(struct blitter_context *blitter)
{
   struct blitter_context_priv *ctx = (struct blitter_context_priv*)blitter;
   struct pipe_context *pipe = ctx->base.pipe;

   /* Fragment shader. */
   ctx->bind_fs_state(pipe, ctx->base.saved_fs);
   ctx->base.saved_fs = INVALID_PTR;

   /* Depth, stencil, alpha. */
   pipe->bind_depth_stencil_alpha_state(pipe, ctx->base.saved_dsa_state);
   ctx->base.saved_dsa_state = INVALID_PTR;

   /* Blend state. */
   pipe->bind_blend_state(pipe, ctx->base.saved_blend_state);
   ctx->base.saved_blend_state = INVALID_PTR;

   /* Sample mask. */
   if (ctx->base.is_sample_mask_saved) {
      pipe->set_sample_mask(pipe, ctx->base.saved_sample_mask);
      ctx->base.is_sample_mask_saved = false;
   }

   if (ctx->base.saved_min_samples != ~0 && pipe->set_min_samples)
      pipe->set_min_samples(pipe, ctx->base.saved_min_samples);
   ctx->base.saved_min_samples = ~0;

   /* Miscellaneous states. */
   /* XXX check whether these are saved and whether they need to be restored
    * (depending on the operation) */
   pipe->set_stencil_ref(pipe, ctx->base.saved_stencil_ref);

   if (!blitter->skip_viewport_restore)
      pipe->set_viewport_states(pipe, 0, 1, &ctx->base.saved_viewport);

   if (blitter->saved_num_window_rectangles) {
      pipe->set_window_rectangles(pipe,
                                  blitter->saved_window_rectangles_include,
                                  blitter->saved_num_window_rectangles,
                                  blitter->saved_window_rectangles);
   }
}

static void blitter_check_saved_fb_state(ASSERTED struct blitter_context_priv *ctx)
{
   assert(ctx->base.saved_fb_state.nr_cbufs != (uint8_t) ~0);
}

static void blitter_disable_render_cond(struct blitter_context_priv *ctx)
{
   struct pipe_context *pipe = ctx->base.pipe;

   if (ctx->base.saved_render_cond_query) {
      pipe->render_condition(pipe, NULL, false, 0);
   }
}

void util_blitter_restore_render_cond(struct blitter_context *blitter)
{
   struct blitter_context_priv *ctx = (struct blitter_context_priv*)blitter;
   struct pipe_context *pipe = ctx->base.pipe;

   if (ctx->base.saved_render_cond_query) {
      pipe->render_condition(pipe, ctx->base.saved_render_cond_query,
                             ctx->base.saved_render_cond_cond,
                             ctx->base.saved_render_cond_mode);
      ctx->base.saved_render_cond_query = NULL;
   }
}

void util_blitter_restore_fb_state(struct blitter_context *blitter)
{
   struct blitter_context_priv *ctx = (struct blitter_context_priv*)blitter;
   struct pipe_context *pipe = ctx->base.pipe;

   pipe->set_framebuffer_state(pipe, &ctx->base.saved_fb_state);
   util_unreference_framebuffer_state(&ctx->base.saved_fb_state);
}

static void blitter_check_saved_textures(ASSERTED struct blitter_context_priv *ctx)
{
   assert(ctx->base.saved_num_sampler_states != ~0u);
   assert(ctx->base.saved_num_sampler_views != ~0u);
}

static void util_blitter_restore_textures_internal(struct blitter_context *blitter, unsigned count)
{
   struct blitter_context_priv *ctx = (struct blitter_context_priv*)blitter;
   struct pipe_context *pipe = ctx->base.pipe;
   unsigned i;

   /* Fragment sampler states. */
   void *states[2] = {NULL};
   assert(count <= ARRAY_SIZE(states));
   if (ctx->base.saved_num_sampler_states)
      pipe->bind_sampler_states(pipe, PIPE_SHADER_FRAGMENT, 0,
                                ctx->base.saved_num_sampler_states,
                                ctx->base.saved_sampler_states);
   else if (count)
      pipe->bind_sampler_states(pipe, PIPE_SHADER_FRAGMENT, 0,
                                count,
                                states);

   ctx->base.saved_num_sampler_states = ~0;

   /* Fragment sampler views. */
   if (ctx->base.saved_num_sampler_views)
      pipe->set_sampler_views(pipe, PIPE_SHADER_FRAGMENT, 0,
                              ctx->base.saved_num_sampler_views, 0, true,
                              ctx->base.saved_sampler_views);
   else if (count)
      pipe->set_sampler_views(pipe, PIPE_SHADER_FRAGMENT, 0,
                              0, count, true,
                              NULL);

   /* Just clear them to NULL because set_sampler_views(take_ownership = true). */
   for (i = 0; i < ctx->base.saved_num_sampler_views; i++)
      ctx->base.saved_sampler_views[i] = NULL;

   ctx->base.saved_num_sampler_views = ~0;
}

void util_blitter_restore_textures(struct blitter_context *blitter)
{
   util_blitter_restore_textures_internal(blitter, 0);
}

void util_blitter_restore_constant_buffer_state(struct blitter_context *blitter)
{
   struct pipe_context *pipe = blitter->pipe;

   pipe->set_constant_buffer(pipe, PIPE_SHADER_FRAGMENT, blitter->cb_slot,
                             true, &blitter->saved_fs_constant_buffer);
   blitter->saved_fs_constant_buffer.buffer = NULL;
}

static void blitter_set_rectangle(struct blitter_context_priv *ctx,
                                  int x1, int y1, int x2, int y2,
                                  float depth)
{
   /* set vertex positions */
   ctx->vertices[0][0][0] = (float)x1 / ctx->dst_width * 2.0f - 1.0f; /*v0.x*/
   ctx->vertices[0][0][1] = (float)y1 / ctx->dst_height * 2.0f - 1.0f; /*v0.y*/

   ctx->vertices[1][0][0] = (float)x2 / ctx->dst_width * 2.0f - 1.0f; /*v1.x*/
   ctx->vertices[1][0][1] = (float)y1 / ctx->dst_height * 2.0f - 1.0f; /*v1.y*/

   ctx->vertices[2][0][0] = (float)x2 / ctx->dst_width * 2.0f - 1.0f; /*v2.x*/
   ctx->vertices[2][0][1] = (float)y2 / ctx->dst_height * 2.0f - 1.0f; /*v2.y*/

   ctx->vertices[3][0][0] = (float)x1 / ctx->dst_width * 2.0f - 1.0f; /*v3.x*/
   ctx->vertices[3][0][1] = (float)y2 / ctx->dst_height * 2.0f - 1.0f; /*v3.y*/

   for (unsigned i = 0; i < 4; ++i)
      ctx->vertices[i][0][2] = depth;

   /* viewport */
   struct pipe_viewport_state viewport;
   viewport.scale[0] = 0.5f * ctx->dst_width;
   viewport.scale[1] = 0.5f * ctx->dst_height;
   viewport.scale[2] = 1.0f;
   viewport.translate[0] = 0.5f * ctx->dst_width;
   viewport.translate[1] = 0.5f * ctx->dst_height;
   viewport.translate[2] = 0.0f;
   viewport.swizzle_x = PIPE_VIEWPORT_SWIZZLE_POSITIVE_X;
   viewport.swizzle_y = PIPE_VIEWPORT_SWIZZLE_POSITIVE_Y;
   viewport.swizzle_z = PIPE_VIEWPORT_SWIZZLE_POSITIVE_Z;
   viewport.swizzle_w = PIPE_VIEWPORT_SWIZZLE_POSITIVE_W;
   ctx->base.pipe->set_viewport_states(ctx->base.pipe, 0, 1, &viewport);
}

static void blitter_set_clear_color(struct blitter_context_priv *ctx,
                                    const float color[4])
{
   int i;

   if (color) {
      for (i = 0; i < 4; i++)
         memcpy(&ctx->vertices[i][1][0], color, sizeof(uint32_t) * 4);
   } else {
      for (i = 0; i < 4; i++)
         memset(&ctx->vertices[i][1][0], 0, sizeof(uint32_t) * 4);
   }
}

static void get_texcoords(struct pipe_sampler_view *src,
                          unsigned src_width0, unsigned src_height0,
                          int x1, int y1, int x2, int y2,
                          float layer, unsigned sample,
                          bool uses_txf, union blitter_attrib *out)
{
   unsigned level = src->u.tex.first_level;
   bool normalized = !uses_txf &&
                        src->target != PIPE_TEXTURE_RECT &&
                        src->texture->nr_samples <= 1;

   if (normalized) {
      out->texcoord.x1 = x1 / (float)u_minify(src_width0,  level);
      out->texcoord.y1 = y1 / (float)u_minify(src_height0, level);
      out->texcoord.x2 = x2 / (float)u_minify(src_width0,  level);
      out->texcoord.y2 = y2 / (float)u_minify(src_height0, level);
   } else {
      out->texcoord.x1 = x1;
      out->texcoord.y1 = y1;
      out->texcoord.x2 = x2;
      out->texcoord.y2 = y2;
   }

   out->texcoord.z = 0;
   out->texcoord.w = 0;

   /* Set the layer. */
   switch (src->target) {
   case PIPE_TEXTURE_3D:
      {
         float r = layer;

         if (!uses_txf)
            r /= u_minify(src->texture->depth0, src->u.tex.first_level);

         out->texcoord.z = r;
      }
      break;

   case PIPE_TEXTURE_1D_ARRAY:
      out->texcoord.y1 = out->texcoord.y2 = layer;
      break;

   case PIPE_TEXTURE_2D_ARRAY:
      out->texcoord.z = layer;
      out->texcoord.w = sample;
      break;

   case PIPE_TEXTURE_CUBE_ARRAY:
      out->texcoord.w = (unsigned)layer / 6;
      break;

   case PIPE_TEXTURE_2D:
      out->texcoord.w = sample;
      break;

   default:;
   }
}

static void blitter_set_dst_dimensions(struct blitter_context_priv *ctx,
                                       unsigned width, unsigned height)
{
   ctx->dst_width = width;
   ctx->dst_height = height;
}

static void set_texcoords_in_vertices(const union blitter_attrib *attrib,
                                      float *out, unsigned stride)
{
   out[0] = attrib->texcoord.x1;
   out[1] = attrib->texcoord.y1;
   out += stride;
   out[0] = attrib->texcoord.x2;
   out[1] = attrib->texcoord.y1;
   out += stride;
   out[0] = attrib->texcoord.x2;
   out[1] = attrib->texcoord.y2;
   out += stride;
   out[0] = attrib->texcoord.x1;
   out[1] = attrib->texcoord.y2;
}

static void *blitter_get_fs_texfetch_col(struct blitter_context_priv *ctx,
                                         enum pipe_format src_format,
                                         enum pipe_format dst_format,
                                         enum pipe_texture_target target,
                                         unsigned src_nr_samples,
                                         unsigned dst_nr_samples,
                                         unsigned filter,
                                         bool use_txf)
{
   struct pipe_context *pipe = ctx->base.pipe;
   enum tgsi_texture_type tgsi_tex =
      util_pipe_tex_to_tgsi_tex(target, src_nr_samples);
   enum tgsi_return_type stype;
   enum tgsi_return_type dtype;
   unsigned type;

   assert(target < PIPE_MAX_TEXTURE_TYPES);

   if (util_format_is_pure_uint(src_format)) {
      stype = TGSI_RETURN_TYPE_UINT;
      if (util_format_is_pure_uint(dst_format)) {
         dtype = TGSI_RETURN_TYPE_UINT;
         type = 0;
      } else {
         assert(util_format_is_pure_sint(dst_format));
         dtype = TGSI_RETURN_TYPE_SINT;
         type = 1;
      }
   } else if (util_format_is_pure_sint(src_format)) {
      stype = TGSI_RETURN_TYPE_SINT;
      if (util_format_is_pure_sint(dst_format)) {
         dtype = TGSI_RETURN_TYPE_SINT;
         type = 2;
      } else {
         assert(util_format_is_pure_uint(dst_format));
         dtype = TGSI_RETURN_TYPE_UINT;
         type = 3;
      }
   } else {
      assert(!util_format_is_pure_uint(dst_format) &&
             !util_format_is_pure_sint(dst_format));
      dtype = stype = TGSI_RETURN_TYPE_FLOAT;
      type = 4;
   }

   if (src_nr_samples > 1) {
      void **shader;

      /* OpenGL requires that integer textures just copy 1 sample instead
       * of averaging.
       */
      if (dst_nr_samples <= 1 &&
          stype != TGSI_RETURN_TYPE_UINT &&
          stype != TGSI_RETURN_TYPE_SINT) {
         /* The destination has one sample, so we'll do color resolve. */
         unsigned index = GET_MSAA_RESOLVE_FS_IDX(src_nr_samples);

         assert(filter < 2);

         shader = &ctx->fs_resolve[target][index][filter];

         if (!*shader) {
            assert(!ctx->cached_all_shaders);
            if (filter == PIPE_TEX_FILTER_LINEAR) {
               *shader = util_make_fs_msaa_resolve_bilinear(pipe, tgsi_tex,
                                                   src_nr_samples, ctx->has_txf_txq);
            }
            else {
               *shader = util_make_fs_msaa_resolve(pipe, tgsi_tex,
                                                   src_nr_samples, ctx->has_txf_txq);
            }
         }
      }
      else {
         /* The destination has multiple samples, we'll do
          * an MSAA->MSAA copy.
          */
         shader = &ctx->fs_texfetch_col_msaa[type][target];

         /* Create the fragment shader on-demand. */
         if (!*shader) {
            assert(!ctx->cached_all_shaders);
            *shader = util_make_fs_blit_msaa_color(pipe, tgsi_tex, stype, dtype,
                                                   ctx->has_sample_shading,
                                                   ctx->has_txf_txq);
         }
      }

      return *shader;
   } else {
      void **shader;

      if (use_txf)
         shader = &ctx->fs_texfetch_col[type][target][1];
      else
         shader = &ctx->fs_texfetch_col[type][target][0];

      /* Create the fragment shader on-demand. */
      if (!*shader) {
         assert(!ctx->cached_all_shaders);
         *shader = util_make_fragment_tex_shader(pipe, tgsi_tex,
                                                 stype, dtype,
                                                 ctx->has_tex_lz, use_txf);
      }

      return *shader;
   }
}

static inline
void *blitter_get_fs_pack_color_zs(struct blitter_context_priv *ctx,
                                   enum pipe_texture_target target,
                                   unsigned nr_samples,
                                   enum pipe_format zs_format,
                                   bool dst_is_color)
{
   struct pipe_context *pipe = ctx->base.pipe;
   enum tgsi_texture_type tgsi_tex =
      util_pipe_tex_to_tgsi_tex(target, nr_samples);
   int format_index = zs_format == PIPE_FORMAT_Z24_UNORM_S8_UINT ? 0 :
                      zs_format == PIPE_FORMAT_S8_UINT_Z24_UNORM ? 1 :
                      zs_format == PIPE_FORMAT_Z32_FLOAT_S8X24_UINT ? 2 :
                      zs_format == PIPE_FORMAT_Z24X8_UNORM ? 3 :
                      zs_format == PIPE_FORMAT_X8Z24_UNORM ? 4 : -1;

   if (format_index == -1) {
      assert(0);
      return NULL;
   }

   /* The first 5 shaders pack ZS to color, the last 5 shaders unpack color
    * to ZS.
    */
   if (dst_is_color)
      format_index += 5;

   void **shader = &ctx->fs_pack_color_zs[tgsi_tex][format_index];

   /* Create the fragment shader on-demand. */
   if (!*shader) {
      assert(!ctx->cached_all_shaders);
      *shader = util_make_fs_pack_color_zs(pipe, tgsi_tex, zs_format,
                                           dst_is_color);
   }
   return *shader;
}

static inline
void *blitter_get_fs_texfetch_depth(struct blitter_context_priv *ctx,
                                    enum pipe_texture_target target,
                                    unsigned src_samples, unsigned dst_samples,
                                    bool use_txf)
{
   struct pipe_context *pipe = ctx->base.pipe;

   assert(target < PIPE_MAX_TEXTURE_TYPES);

   if (src_samples > 1) {
      bool sample_shading = ctx->has_sample_shading && src_samples > 1 &&
                            src_samples == dst_samples;
      void **shader = &ctx->fs_texfetch_depth_msaa[target][sample_shading];

      /* Create the fragment shader on-demand. */
      if (!*shader) {
         enum tgsi_texture_type tgsi_tex;
         assert(!ctx->cached_all_shaders);
         tgsi_tex = util_pipe_tex_to_tgsi_tex(target, src_samples);
         *shader = util_make_fs_blit_msaa_depth(pipe, tgsi_tex, sample_shading,
                                                ctx->has_txf_txq);
      }

      return *shader;
   } else {
      void **shader;

      if (use_txf)
         shader = &ctx->fs_texfetch_depth[target][1];
      else
         shader = &ctx->fs_texfetch_depth[target][0];

      /* Create the fragment shader on-demand. */
      if (!*shader) {
         enum tgsi_texture_type tgsi_tex;
         assert(!ctx->cached_all_shaders);
         tgsi_tex = util_pipe_tex_to_tgsi_tex(target, 0);
         *shader = util_make_fs_blit_zs(pipe, PIPE_MASK_Z, tgsi_tex,
                                        ctx->has_tex_lz, use_txf);
      }

      return *shader;
   }
}

static inline
void *blitter_get_fs_texfetch_depthstencil(struct blitter_context_priv *ctx,
                                           enum pipe_texture_target target,
                                           unsigned src_samples,
                                           unsigned dst_samples, bool use_txf)
{
   struct pipe_context *pipe = ctx->base.pipe;

   assert(target < PIPE_MAX_TEXTURE_TYPES);

   if (src_samples > 1) {
      bool sample_shading = ctx->has_sample_shading && src_samples > 1 &&
                            src_samples == dst_samples;
      void **shader = &ctx->fs_texfetch_depthstencil_msaa[target][sample_shading];

      /* Create the fragment shader on-demand. */
      if (!*shader) {
         enum tgsi_texture_type tgsi_tex;
         assert(!ctx->cached_all_shaders);
         tgsi_tex = util_pipe_tex_to_tgsi_tex(target, src_samples);
         *shader = util_make_fs_blit_msaa_depthstencil(pipe, tgsi_tex,
                                                       sample_shading,
                                                       ctx->has_txf_txq);
      }

      return *shader;
   } else {
      void **shader;

      if (use_txf)
         shader = &ctx->fs_texfetch_depthstencil[target][1];
      else
         shader = &ctx->fs_texfetch_depthstencil[target][0];

      /* Create the fragment shader on-demand. */
      if (!*shader) {
         enum tgsi_texture_type tgsi_tex;
         assert(!ctx->cached_all_shaders);
         tgsi_tex = util_pipe_tex_to_tgsi_tex(target, 0);
         *shader = util_make_fs_blit_zs(pipe, PIPE_MASK_ZS, tgsi_tex,
                                        ctx->has_tex_lz, use_txf);
      }

      return *shader;
   }
}

static inline
void *blitter_get_fs_texfetch_stencil(struct blitter_context_priv *ctx,
                                      enum pipe_texture_target target,
                                      unsigned src_samples, unsigned dst_samples,
                                      bool use_txf)
{
   struct pipe_context *pipe = ctx->base.pipe;

   assert(target < PIPE_MAX_TEXTURE_TYPES);

   if (src_samples > 1) {
      bool sample_shading = ctx->has_sample_shading && src_samples > 1 &&
                            src_samples == dst_samples;
      void **shader = &ctx->fs_texfetch_stencil_msaa[target][sample_shading];

      /* Create the fragment shader on-demand. */
      if (!*shader) {
         enum tgsi_texture_type tgsi_tex;
         assert(!ctx->cached_all_shaders);
         tgsi_tex = util_pipe_tex_to_tgsi_tex(target, src_samples);
         *shader = util_make_fs_blit_msaa_stencil(pipe, tgsi_tex,
                                                  sample_shading,
                                                  ctx->has_txf_txq);
      }

      return *shader;
   } else {
      void **shader;

      if (use_txf)
         shader = &ctx->fs_texfetch_stencil[target][1];
      else
         shader = &ctx->fs_texfetch_stencil[target][0];

      /* Create the fragment shader on-demand. */
      if (!*shader) {
         enum tgsi_texture_type tgsi_tex;
         assert(!ctx->cached_all_shaders);
         tgsi_tex = util_pipe_tex_to_tgsi_tex(target, 0);
         *shader = util_make_fs_blit_zs(pipe, PIPE_MASK_S, tgsi_tex,
                                        ctx->has_tex_lz, use_txf);
      }

      return *shader;
   }
}


/**
 * Generate and save all fragment shaders that we will ever need for
 * blitting.  Drivers which use the 'draw' fallbacks will typically use
 * this to make sure we generate/use shaders that don't go through the
 * draw module's wrapper functions.
 */
void util_blitter_cache_all_shaders(struct blitter_context *blitter)
{
   struct blitter_context_priv *ctx = (struct blitter_context_priv*)blitter;
   struct pipe_context *pipe = blitter->pipe;
   struct pipe_screen *screen = pipe->screen;
   unsigned samples, j, f, target, max_samples, use_txf;
   bool has_arraytex, has_cubearraytex;

   max_samples = ctx->has_texture_multisample ? 2 : 1;
   has_arraytex = screen->get_param(screen,
                                    PIPE_CAP_MAX_TEXTURE_ARRAY_LAYERS) != 0;
   has_cubearraytex = screen->get_param(screen,
                                    PIPE_CAP_CUBE_MAP_ARRAY) != 0;

   /* It only matters if i <= 1 or > 1. */
   for (samples = 1; samples <= max_samples; samples++) {
      for (target = PIPE_TEXTURE_1D; target < PIPE_MAX_TEXTURE_TYPES; target++) {
         for (use_txf = 0; use_txf <= ctx->has_txf_txq; use_txf++) {
            if (!has_arraytex &&
                (target == PIPE_TEXTURE_1D_ARRAY ||
                 target == PIPE_TEXTURE_2D_ARRAY)) {
               continue;
            }
            if (!has_cubearraytex &&
                (target == PIPE_TEXTURE_CUBE_ARRAY))
               continue;
            if (!ctx->has_texrect &&
                (target == PIPE_TEXTURE_RECT))
               continue;

            if (samples > 1 &&
                (target != PIPE_TEXTURE_2D &&
                 target != PIPE_TEXTURE_2D_ARRAY))
               continue;

            if (samples > 1 && use_txf)
               continue; /* TXF is the only option, use_txf has no effect */

            /* If samples == 1, the shaders read one texel. If samples >= 1,
             * they read one sample.
             */
            blitter_get_fs_texfetch_col(ctx, PIPE_FORMAT_R32_FLOAT,
                                        PIPE_FORMAT_R32_FLOAT, target,
                                        samples, samples, 0, use_txf);
            blitter_get_fs_texfetch_col(ctx, PIPE_FORMAT_R32_UINT,
                                        PIPE_FORMAT_R32_UINT, target,
                                        samples, samples, 0, use_txf);
            blitter_get_fs_texfetch_col(ctx, PIPE_FORMAT_R32_UINT,
                                        PIPE_FORMAT_R32_SINT, target,
                                        samples, samples, 0, use_txf);
            blitter_get_fs_texfetch_col(ctx, PIPE_FORMAT_R32_SINT,
                                        PIPE_FORMAT_R32_SINT, target,
                                        samples, samples, 0, use_txf);
            blitter_get_fs_texfetch_col(ctx, PIPE_FORMAT_R32_SINT,
                                        PIPE_FORMAT_R32_UINT, target,
                                        samples, samples, 0, use_txf);
            blitter_get_fs_texfetch_depth(ctx, target, samples, samples, use_txf);
            if (ctx->has_stencil_export) {
               blitter_get_fs_texfetch_depthstencil(ctx, target, samples, samples, use_txf);
               blitter_get_fs_texfetch_stencil(ctx, target, samples, samples, use_txf);
            }

            if (samples == 2) {
               blitter_get_fs_texfetch_depth(ctx, target, samples, 1, use_txf);
               if (ctx->has_stencil_export) {
                  blitter_get_fs_texfetch_depthstencil(ctx, target, samples, 1, use_txf);
                  blitter_get_fs_texfetch_stencil(ctx, target, samples, 1, use_txf);
               }
            }

            if (samples == 1)
               continue;

            /* MSAA resolve shaders. */
            for (j = 2; j < 32; j++) {
               if (!screen->is_format_supported(screen, PIPE_FORMAT_R32_FLOAT,
                                                target, j, j,
                                                PIPE_BIND_SAMPLER_VIEW)) {
                  continue;
               }

               for (f = 0; f < 2; f++) {
                  if (f != PIPE_TEX_FILTER_NEAREST && use_txf)
                     continue;

                  blitter_get_fs_texfetch_col(ctx, PIPE_FORMAT_R32_FLOAT,
                                              PIPE_FORMAT_R32_FLOAT, target,
                                              j, 1, f, use_txf);
                  blitter_get_fs_texfetch_col(ctx, PIPE_FORMAT_R32_UINT,
                                              PIPE_FORMAT_R32_UINT, target,
                                              j, 1, f, use_txf);
                  blitter_get_fs_texfetch_col(ctx, PIPE_FORMAT_R32_SINT,
                                              PIPE_FORMAT_R32_SINT, target,
                                              j, 1, f, use_txf);
               }
            }
         }
      }
   }

   ctx->fs_empty = util_make_empty_fragment_shader(pipe);

   ctx->fs_write_one_cbuf =
      util_make_fragment_passthrough_shader(pipe, TGSI_SEMANTIC_GENERIC,
                                            TGSI_INTERPOLATE_CONSTANT, false);

   ctx->fs_clear_all_cbufs = util_make_fs_clear_all_cbufs(pipe);

   ctx->cached_all_shaders = true;
}

static void blitter_set_common_draw_rect_state(struct blitter_context_priv *ctx,
                                               bool scissor, bool msaa)
{
   struct pipe_context *pipe = ctx->base.pipe;

   if (ctx->base.saved_num_window_rectangles)
      pipe->set_window_rectangles(pipe, false, 0, NULL);

   pipe->bind_rasterizer_state(pipe, ctx->rs_state[scissor][msaa]);

   if (ctx->has_geometry_shader)
      pipe->bind_gs_state(pipe, NULL);
   if (ctx->has_tessellation) {
      pipe->bind_tcs_state(pipe, NULL);
      pipe->bind_tes_state(pipe, NULL);
   }
   if (ctx->has_stream_out)
      pipe->set_stream_output_targets(pipe, 0, NULL, NULL);
}

static void blitter_draw(struct blitter_context_priv *ctx,
                         void *vertex_elements_cso,
                         blitter_get_vs_func get_vs,
                         int x1, int y1, int x2, int y2, float depth,
                         unsigned num_instances)
{
   struct pipe_context *pipe = ctx->base.pipe;
   struct pipe_vertex_buffer vb = {0};

   blitter_set_rectangle(ctx, x1, y1, x2, y2, depth);

   u_upload_data(pipe->stream_uploader, 0, sizeof(ctx->vertices), 4, ctx->vertices,
                 &vb.buffer_offset, &vb.buffer.resource);
   if (!vb.buffer.resource)
      return;
   u_upload_unmap(pipe->stream_uploader);

   pipe->set_vertex_buffers(pipe, 1, 0, false, &vb);
   pipe->bind_vertex_elements_state(pipe, vertex_elements_cso);
   pipe->bind_vs_state(pipe, get_vs(&ctx->base));

   if (ctx->base.use_index_buffer) {
      /* Note that for V3D,
       * dEQP-GLES3.functional.fbo.blit.rect.nearest_consistency_* require
       * that the last vert of the two tris be the same.
       */
      static uint8_t indices[6] = { 0, 1, 2, 0, 3, 2 };
      util_draw_elements_instanced(pipe, indices, 1, 0,
                                   MESA_PRIM_TRIANGLES, 0, 6,
                                   0, num_instances);
   } else {
      util_draw_arrays_instanced(pipe, MESA_PRIM_TRIANGLE_FAN, 0, 4,
                                 0, num_instances);
   }
   pipe_resource_reference(&vb.buffer.resource, NULL);
}

void util_blitter_draw_rectangle(struct blitter_context *blitter,
                                 void *vertex_elements_cso,
                                 blitter_get_vs_func get_vs,
                                 int x1, int y1, int x2, int y2,
                                 float depth, unsigned num_instances,
                                 enum blitter_attrib_type type,
                                 const union blitter_attrib *attrib)
{
   struct blitter_context_priv *ctx = (struct blitter_context_priv*)blitter;
   unsigned i;

   switch (type) {
      case UTIL_BLITTER_ATTRIB_COLOR:
         blitter_set_clear_color(ctx, attrib->color);
         break;

      case UTIL_BLITTER_ATTRIB_TEXCOORD_XYZW:
         for (i = 0; i < 4; i++) {
            ctx->vertices[i][1][2] = attrib->texcoord.z;
            ctx->vertices[i][1][3] = attrib->texcoord.w;
         }
         set_texcoords_in_vertices(attrib, &ctx->vertices[0][1][0], 8);
         break;
      case UTIL_BLITTER_ATTRIB_TEXCOORD_XY:
         /* We clean-up the ZW components, just in case we used before XYZW,
          * to avoid feeding in the shader with wrong values (like on the lod)
          */
         for (i = 0; i < 4; i++) {
            ctx->vertices[i][1][2] = 0;
            ctx->vertices[i][1][3] = 0;
         }
         set_texcoords_in_vertices(attrib, &ctx->vertices[0][1][0], 8);
         break;

      default:;
   }

   blitter_draw(ctx, vertex_elements_cso, get_vs, x1, y1, x2, y2, depth,
                num_instances);
}

static void *get_clear_blend_state(struct blitter_context_priv *ctx,
                                   unsigned clear_buffers)
{
   struct pipe_context *pipe = ctx->base.pipe;
   int index;

   clear_buffers &= PIPE_CLEAR_COLOR;

   /* Return an existing blend state. */
   if (!clear_buffers)
      return ctx->blend[0][0];

   index = GET_CLEAR_BLEND_STATE_IDX(clear_buffers);

   if (ctx->blend_clear[index])
      return ctx->blend_clear[index];

   /* Create a new one. */
   {
      struct pipe_blend_state blend = {0};
      unsigned i;

      blend.independent_blend_enable = 1;

      for (i = 0; i < PIPE_MAX_COLOR_BUFS; i++) {
         if (clear_buffers & (PIPE_CLEAR_COLOR0 << i)) {
            blend.rt[i].colormask = PIPE_MASK_RGBA;
            blend.max_rt = i;
         }
      }

      ctx->blend_clear[index] = pipe->create_blend_state(pipe, &blend);
   }
   return ctx->blend_clear[index];
}

void util_blitter_common_clear_setup(struct blitter_context *blitter,
                                     unsigned width, unsigned height,
                                     unsigned clear_buffers,
                                     void *custom_blend, void *custom_dsa)
{
   struct blitter_context_priv *ctx = (struct blitter_context_priv*)blitter;
   struct pipe_context *pipe = ctx->base.pipe;

   util_blitter_set_running_flag(blitter);
   blitter_check_saved_vertex_states(ctx);
   blitter_check_saved_fragment_states(ctx);
   blitter_disable_render_cond(ctx);

   /* bind states */
   if (custom_blend) {
      pipe->bind_blend_state(pipe, custom_blend);
   } else {
      pipe->bind_blend_state(pipe, get_clear_blend_state(ctx, clear_buffers));
   }

   if (custom_dsa) {
      pipe->bind_depth_stencil_alpha_state(pipe, custom_dsa);
   } else if ((clear_buffers & PIPE_CLEAR_DEPTHSTENCIL) == PIPE_CLEAR_DEPTHSTENCIL) {
      pipe->bind_depth_stencil_alpha_state(pipe, ctx->dsa_write_depth_stencil);
   } else if (clear_buffers & PIPE_CLEAR_DEPTH) {
      pipe->bind_depth_stencil_alpha_state(pipe, ctx->dsa_write_depth_keep_stencil);
   } else if (clear_buffers & PIPE_CLEAR_STENCIL) {
      pipe->bind_depth_stencil_alpha_state(pipe, ctx->dsa_keep_depth_write_stencil);
   } else {
      pipe->bind_depth_stencil_alpha_state(pipe, ctx->dsa_keep_depth_stencil);
   }

   pipe->set_sample_mask(pipe, ~0);
   if (pipe->set_min_samples)
      pipe->set_min_samples(pipe, 1);
   blitter_set_dst_dimensions(ctx, width, height);
}

static void util_blitter_clear_custom(struct blitter_context *blitter,
                                      unsigned width, unsigned height,
                                      unsigned num_layers,
                                      unsigned clear_buffers,
                                      const union pipe_color_union *color,
                                      double depth, unsigned stencil,
                                      void *custom_blend, void *custom_dsa,
                                      bool msaa)
{
   struct blitter_context_priv *ctx = (struct blitter_context_priv*)blitter;
   struct pipe_context *pipe = ctx->base.pipe;
   struct pipe_stencil_ref sr = { { 0 } };

   assert(ctx->has_layered || num_layers <= 1);

   util_blitter_common_clear_setup(blitter, width, height, clear_buffers,
                                   custom_blend, custom_dsa);

   sr.ref_value[0] = stencil & 0xff;
   pipe->set_stencil_ref(pipe, sr);

   bool pass_generic = (clear_buffers & PIPE_CLEAR_COLOR) != 0;
   enum blitter_attrib_type type = UTIL_BLITTER_ATTRIB_NONE;

   if (pass_generic) {
      struct pipe_constant_buffer cb = {
         .user_buffer = color->f,
         .buffer_size = 4 * sizeof(float),
      };
      pipe->set_constant_buffer(pipe, PIPE_SHADER_FRAGMENT, blitter->cb_slot,
                                false, &cb);
      bind_fs_clear_all_cbufs(ctx);
   } else {
      bind_fs_empty(ctx);
   }

   if (num_layers > 1 && ctx->has_layered) {
      blitter_get_vs_func get_vs = get_vs_layered;

      blitter_set_common_draw_rect_state(ctx, false, msaa);
      blitter->draw_rectangle(blitter, ctx->velem_state, get_vs,
                              0, 0, width, height,
                              (float) depth, num_layers, type, NULL);
   } else {
      blitter_get_vs_func get_vs;

      if (pass_generic)
         get_vs = get_vs_passthrough_pos_generic;
      else
         get_vs = get_vs_passthrough_pos;

      blitter_set_common_draw_rect_state(ctx, false, msaa);
      blitter->draw_rectangle(blitter, ctx->velem_state, get_vs,
                              0, 0, width, height,
                              (float) depth, 1, type, NULL);
   }

   util_blitter_restore_vertex_states(blitter);
   util_blitter_restore_fragment_states(blitter);
   util_blitter_restore_constant_buffer_state(blitter);
   util_blitter_restore_render_cond(blitter);
   util_blitter_unset_running_flag(blitter);
}

void util_blitter_clear(struct blitter_context *blitter,
                        unsigned width, unsigned height, unsigned num_layers,
                        unsigned clear_buffers,
                        const union pipe_color_union *color,
                        double depth, unsigned stencil,
                        bool msaa)
{
   util_blitter_clear_custom(blitter, width, height, num_layers,
                             clear_buffers, color, depth, stencil,
                             NULL, NULL, msaa);
}

void util_blitter_custom_clear_depth(struct blitter_context *blitter,
                                     unsigned width, unsigned height,
                                     double depth, void *custom_dsa)
{
   static const union pipe_color_union color;
   util_blitter_clear_custom(blitter, width, height, 0, 0, &color, depth, 0,
                             NULL, custom_dsa, false);
}

void util_blitter_default_dst_texture(struct pipe_surface *dst_templ,
                                      struct pipe_resource *dst,
                                      unsigned dstlevel,
                                      unsigned dstz)
{
   memset(dst_templ, 0, sizeof(*dst_templ));
   dst_templ->format = util_format_linear(dst->format);
   dst_templ->u.tex.level = dstlevel;
   dst_templ->u.tex.first_layer = dstz;
   dst_templ->u.tex.last_layer = dstz;
}

static struct pipe_surface *
util_blitter_get_next_surface_layer(struct pipe_context *pipe,
                                    struct pipe_surface *surf)
{
   struct pipe_surface dst_templ;

   memset(&dst_templ, 0, sizeof(dst_templ));
   dst_templ.format = surf->format;
   dst_templ.u.tex.level = surf->u.tex.level;
   dst_templ.u.tex.first_layer = surf->u.tex.first_layer + 1;
   dst_templ.u.tex.last_layer = surf->u.tex.last_layer + 1;

   return pipe->create_surface(pipe, surf->texture, &dst_templ);
}

void util_blitter_default_src_texture(struct blitter_context *blitter,
                                      struct pipe_sampler_view *src_templ,
                                      struct pipe_resource *src,
                                      unsigned srclevel)
{
   struct blitter_context_priv *ctx = (struct blitter_context_priv*)blitter;

   memset(src_templ, 0, sizeof(*src_templ));

   if (ctx->cube_as_2darray &&
       (src->target == PIPE_TEXTURE_CUBE ||
        src->target == PIPE_TEXTURE_CUBE_ARRAY))
      src_templ->target = PIPE_TEXTURE_2D_ARRAY;
   else
      src_templ->target = src->target;

   src_templ->format = util_format_linear(src->format);
   src_templ->u.tex.first_level = srclevel;
   src_templ->u.tex.last_level = srclevel;
   src_templ->u.tex.first_layer = 0;
   src_templ->u.tex.last_layer =
      src->target == PIPE_TEXTURE_3D ? u_minify(src->depth0, srclevel) - 1
                                     : (unsigned)(src->array_size - 1);
   src_templ->swizzle_r = PIPE_SWIZZLE_X;
   src_templ->swizzle_g = PIPE_SWIZZLE_Y;
   src_templ->swizzle_b = PIPE_SWIZZLE_Z;
   src_templ->swizzle_a = PIPE_SWIZZLE_W;
}

static bool is_blit_generic_supported(struct blitter_context *blitter,
                                      const struct pipe_resource *dst,
                                      enum pipe_format dst_format,
                                      const struct pipe_resource *src,
                                      enum pipe_format src_format,
                                      unsigned mask)
{
   struct blitter_context_priv *ctx = (struct blitter_context_priv*)blitter;
   struct pipe_screen *screen = ctx->base.pipe->screen;

   if (dst) {
      unsigned bind;
      const struct util_format_description *desc =
            util_format_description(dst_format);
      bool dst_has_stencil = util_format_has_stencil(desc);

      /* Stencil export must be supported for stencil copy. */
      if ((mask & PIPE_MASK_S) && dst_has_stencil &&
          !ctx->has_stencil_export) {
         return false;
      }

      if (dst_has_stencil || util_format_has_depth(desc))
         bind = PIPE_BIND_DEPTH_STENCIL;
      else
         bind = PIPE_BIND_RENDER_TARGET;

      if (!screen->is_format_supported(screen, dst_format, dst->target,
                                       dst->nr_samples, dst->nr_storage_samples,
                                       bind)) {
         return false;
      }
   }

   if (src) {
      if (src->nr_samples > 1 && !ctx->has_texture_multisample) {
         return false;
      }

      if (!screen->is_format_supported(screen, src_format, src->target,
                                       src->nr_samples, src->nr_storage_samples,
                                       PIPE_BIND_SAMPLER_VIEW)) {
         return false;
      }

      /* Check stencil sampler support for stencil copy. */
      if (mask & PIPE_MASK_S) {
         if (util_format_has_stencil(util_format_description(src_format))) {
            enum pipe_format stencil_format =
               util_format_stencil_only(src_format);
            assert(stencil_format != PIPE_FORMAT_NONE);

            if (stencil_format != src_format &&
                !screen->is_format_supported(screen, stencil_format,
                                             src->target, src->nr_samples,
                                             src->nr_storage_samples,
                                             PIPE_BIND_SAMPLER_VIEW)) {
               return false;
            }
         }
      }
   }

   return true;
}

bool util_blitter_is_copy_supported(struct blitter_context *blitter,
                                    const struct pipe_resource *dst,
                                    const struct pipe_resource *src)
{
   return is_blit_generic_supported(blitter, dst, dst->format,
                                    src, src->format, PIPE_MASK_RGBAZS);
}

bool util_blitter_is_blit_supported(struct blitter_context *blitter,
                                    const struct pipe_blit_info *info)
{
   return is_blit_generic_supported(blitter,
                                    info->dst.resource, info->dst.format,
                                    info->src.resource, info->src.format,
                                    info->mask);
}

void util_blitter_copy_texture(struct blitter_context *blitter,
                               struct pipe_resource *dst,
                               unsigned dst_level,
                               unsigned dstx, unsigned dsty, unsigned dstz,
                               struct pipe_resource *src,
                               unsigned src_level,
                               const struct pipe_box *srcbox)
{
   struct blitter_context_priv *ctx = (struct blitter_context_priv*)blitter;
   struct pipe_context *pipe = ctx->base.pipe;
   struct pipe_surface *dst_view, dst_templ;
   struct pipe_sampler_view src_templ, *src_view;
   struct pipe_box dstbox;

   assert(dst && src);
   assert(src->target < PIPE_MAX_TEXTURE_TYPES);

   u_box_3d(dstx, dsty, dstz, abs(srcbox->width), abs(srcbox->height),
            abs(srcbox->depth), &dstbox);

   /* Initialize the surface. */
   util_blitter_default_dst_texture(&dst_templ, dst, dst_level, dstz);
   dst_view = pipe->create_surface(pipe, dst, &dst_templ);

   /* Initialize the sampler view. */
   util_blitter_default_src_texture(blitter, &src_templ, src, src_level);
   src_view = pipe->create_sampler_view(pipe, src, &src_templ);

   /* Copy. */
   util_blitter_blit_generic(blitter, dst_view, &dstbox,
                             src_view, srcbox, src->width0, src->height0,
                             PIPE_MASK_RGBAZS, PIPE_TEX_FILTER_NEAREST, NULL,
                             false, false, 0);

   pipe_surface_reference(&dst_view, NULL);
   pipe_sampler_view_reference(&src_view, NULL);
}

static void
blitter_draw_tex(struct blitter_context_priv *ctx,
                 int dst_x1, int dst_y1, int dst_x2, int dst_y2,
                 struct pipe_sampler_view *src,
                 unsigned src_width0, unsigned src_height0,
                 int src_x1, int src_y1, int src_x2, int src_y2,
                 float layer, unsigned sample,
                 bool uses_txf, enum blitter_attrib_type type)
{
   union blitter_attrib coord;
   blitter_get_vs_func get_vs = get_vs_passthrough_pos_generic;

   get_texcoords(src, src_width0, src_height0,
                 src_x1, src_y1, src_x2, src_y2, layer, sample,
                 uses_txf, &coord);

   if (src->target == PIPE_TEXTURE_CUBE ||
       src->target == PIPE_TEXTURE_CUBE_ARRAY) {
      float face_coord[4][2];

      set_texcoords_in_vertices(&coord, &face_coord[0][0], 2);
      util_map_texcoords2d_onto_cubemap((unsigned)layer % 6,
                                        /* pointer, stride in floats */
                                        &face_coord[0][0], 2,
                                        &ctx->vertices[0][1][0], 8,
                                        false);
      for (unsigned i = 0; i < 4; i++)
         ctx->vertices[i][1][3] = coord.texcoord.w;

      /* Cubemaps don't use draw_rectangle. */
      blitter_draw(ctx, ctx->velem_state, get_vs,
                   dst_x1, dst_y1, dst_x2, dst_y2, 0, 1);
   } else {
      ctx->base.draw_rectangle(&ctx->base, ctx->velem_state, get_vs,
                               dst_x1, dst_y1, dst_x2, dst_y2,
                               0, 1, type, &coord);
   }
}

static void do_blits(struct blitter_context_priv *ctx,
                     struct pipe_surface *dst,
                     const struct pipe_box *dstbox,
                     struct pipe_sampler_view *src,
                     unsigned src_width0,
                     unsigned src_height0,
                     const struct pipe_box *srcbox,
                     bool is_zsbuf,
                     bool uses_txf, bool sample0_only,
                     unsigned dst_sample)
{
   struct pipe_context *pipe = ctx->base.pipe;
   unsigned src_samples = src->texture->nr_samples;
   unsigned dst_samples = dst->texture->nr_samples;
   bool sample_shading = ctx->has_sample_shading && src_samples > 1 &&
                         src_samples == dst_samples && !sample0_only;
   enum pipe_texture_target src_target = src->target;
   struct pipe_framebuffer_state fb_state = {0};

   /* Initialize framebuffer state. */
   fb_state.width = dst->width;
   fb_state.height = dst->height;
   fb_state.nr_cbufs = is_zsbuf ? 0 : 1;

   blitter_set_dst_dimensions(ctx, fb_state.width, fb_state.height);

   if ((src_target == PIPE_TEXTURE_1D ||
        src_target == PIPE_TEXTURE_2D ||
        src_target == PIPE_TEXTURE_RECT) &&
       (src_samples <= 1 || sample_shading)) {
      /* Set framebuffer state. */
      if (is_zsbuf) {
         fb_state.zsbuf = dst;
      } else {
         fb_state.cbufs[0] = dst;
      }
      pipe->set_framebuffer_state(pipe, &fb_state);

      /* Draw. */
      pipe->set_sample_mask(pipe, dst_sample ? BITFIELD_BIT(dst_sample - 1) : ~0);
      if (pipe->set_min_samples)
         pipe->set_min_samples(pipe, sample_shading ? dst_samples : 1);
      blitter_draw_tex(ctx, dstbox->x, dstbox->y,
                       dstbox->x + dstbox->width,
                       dstbox->y + dstbox->height,
                       src, src_width0, src_height0, srcbox->x, srcbox->y,
                       srcbox->x + srcbox->width, srcbox->y + srcbox->height,
                       0, 0, uses_txf, UTIL_BLITTER_ATTRIB_TEXCOORD_XY);
   } else {
      /* Draw the quad with the generic codepath. */
      int dst_z;
      for (dst_z = 0; dst_z < dstbox->depth; dst_z++) {
         struct pipe_surface *old;
         bool flipped = (srcbox->depth < 0);
         float depth_center_offset = 0.0;
         int src_depth = abs(srcbox->depth);
         float src_z_step = src_depth / (float)dstbox->depth;

         /* Scale Z properly if the blit is scaled.
          *
          * When downscaling, we want the coordinates centered, so that
          * mipmapping works for 3D textures. For example, when generating
          * a 4x4x4 level, this wouldn't average the pixels:
          *
          *   src Z:  0 1 2 3 4 5 6 7
          *   dst Z:  0   1   2   3
          *
          * Because the pixels are not centered below the pixels of the higher
          * level. Therefore, we want this:
          *   src Z:  0 1 2 3 4 5 6 7
          *   dst Z:   0   1   2   3
          *
          * This calculation is taken from the radv driver.
          */
         if (src_target == PIPE_TEXTURE_3D)
            depth_center_offset = 0.5 / dstbox->depth * src_depth;

         if (flipped) {
            src_z_step *= - 1;
            depth_center_offset *= -1;
         }

         float src_z = dst_z * src_z_step + depth_center_offset;

         /* Set framebuffer state. */
         if (is_zsbuf) {
            fb_state.zsbuf = dst;
         } else {
            fb_state.cbufs[0] = dst;
         }
         pipe->set_framebuffer_state(pipe, &fb_state);

         /* See if we need to blit a multisample or singlesample buffer. */
         if (sample0_only || (src_samples == dst_samples && dst_samples > 1)) {
            /* MSAA copy. */
            unsigned i, max_sample = sample0_only ? 0 : dst_samples - 1;

            if (sample_shading) {
               assert(dst_sample == 0);
               pipe->set_sample_mask(pipe, ~0);
               if (pipe->set_min_samples)
                  pipe->set_min_samples(pipe, max_sample);
               blitter_draw_tex(ctx, dstbox->x, dstbox->y,
                                dstbox->x + dstbox->width,
                                dstbox->y + dstbox->height,
                                src, src_width0, src_height0,
                                srcbox->x, srcbox->y,
                                srcbox->x + srcbox->width,
                                srcbox->y + srcbox->height,
                                srcbox->z + src_z, 0, uses_txf,
                                UTIL_BLITTER_ATTRIB_TEXCOORD_XYZW);
            } else {
               if (pipe->set_min_samples)
                  pipe->set_min_samples(pipe, 1);

               for (i = 0; i <= max_sample; i++) {
                  pipe->set_sample_mask(pipe, 1 << i);
                  blitter_draw_tex(ctx, dstbox->x, dstbox->y,
                                   dstbox->x + dstbox->width,
                                   dstbox->y + dstbox->height,
                                   src, src_width0, src_height0,
                                   srcbox->x, srcbox->y,
                                   srcbox->x + srcbox->width,
                                   srcbox->y + srcbox->height,
                                   srcbox->z + src_z, i, uses_txf,
                                   UTIL_BLITTER_ATTRIB_TEXCOORD_XYZW);
               }
            }
         } else {
            /* Normal copy, MSAA upsampling, or MSAA resolve. */
            pipe->set_sample_mask(pipe, dst_sample ? BITFIELD_BIT(dst_sample - 1) : ~0);
            if (pipe->set_min_samples)
               pipe->set_min_samples(pipe, 1);
            blitter_draw_tex(ctx, dstbox->x, dstbox->y,
                             dstbox->x + dstbox->width,
                             dstbox->y + dstbox->height,
                             src, src_width0, src_height0,
                             srcbox->x, srcbox->y,
                             srcbox->x + srcbox->width,
                             srcbox->y + srcbox->height,
                             srcbox->z + src_z, 0, uses_txf,
                             UTIL_BLITTER_ATTRIB_TEXCOORD_XYZW);
         }

         /* Get the next surface or (if this is the last iteration)
          * just unreference the last one. */
         old = dst;
         if (dst_z < dstbox->depth-1) {
            dst = util_blitter_get_next_surface_layer(ctx->base.pipe, dst);
         }
         if (dst_z) {
            pipe_surface_reference(&old, NULL);
         }
      }
   }
}

void util_blitter_blit_generic(struct blitter_context *blitter,
                               struct pipe_surface *dst,
                               const struct pipe_box *dstbox,
                               struct pipe_sampler_view *src,
                               const struct pipe_box *srcbox,
                               unsigned src_width0, unsigned src_height0,
                               unsigned mask, unsigned filter,
                               const struct pipe_scissor_state *scissor,
                               bool alpha_blend, bool sample0_only,
                               unsigned dst_sample)
{
   struct blitter_context_priv *ctx = (struct blitter_context_priv*)blitter;
   struct pipe_context *pipe = ctx->base.pipe;
   enum pipe_texture_target src_target = src->target;
   unsigned src_samples = src->texture->nr_samples;
   unsigned dst_samples = dst->texture->nr_samples;
   void *sampler_state;
   const struct util_format_description *src_desc =
         util_format_description(src->format);
   const struct util_format_description *dst_desc =
         util_format_description(dst->format);

   bool src_has_color = src_desc->colorspace != UTIL_FORMAT_COLORSPACE_ZS;
   bool src_has_depth = util_format_has_depth(src_desc);
   bool src_has_stencil = util_format_has_stencil(src_desc);

   bool dst_has_color = mask & PIPE_MASK_RGBA &&
                        dst_desc->colorspace != UTIL_FORMAT_COLORSPACE_ZS;
   bool dst_has_depth = mask & PIPE_MASK_Z &&
                        util_format_has_depth(dst_desc);
   bool dst_has_stencil = ctx->has_stencil_export &&
                          mask & PIPE_MASK_S &&
                          util_format_has_stencil(dst_desc);

   /* Return if there is nothing to do. */
   if (!dst_has_color && !dst_has_depth && !dst_has_stencil) {
      return;
   }

   bool is_scaled = dstbox->width != abs(srcbox->width) ||
                    dstbox->height != abs(srcbox->height) ||
                    dstbox->depth != abs(srcbox->depth);

   if (src_has_stencil || !is_scaled)
      filter = PIPE_TEX_FILTER_NEAREST;

   bool use_txf = false;

   /* Don't support scaled blits. The TXF shader uses F2I for rounding. */
   if (ctx->has_txf_txq &&
       !is_scaled &&
       filter == PIPE_TEX_FILTER_NEAREST &&
       src->target != PIPE_TEXTURE_CUBE &&
       src->target != PIPE_TEXTURE_CUBE_ARRAY) {
      int src_width = u_minify(src_width0, src->u.tex.first_level);
      int src_height = u_minify(src_height0, src->u.tex.first_level);
      int src_depth = src->u.tex.last_layer + 1;
      struct pipe_box box = *srcbox;

      /* Eliminate negative width/height/depth. */
      if (box.width < 0) {
         box.x += box.width;
         box.width *= -1;
      }
      if (box.height < 0) {
         box.y += box.height;
         box.height *= -1;
      }
      if (box.depth < 0) {
         box.z += box.depth;
         box.depth *= -1;
      }

      /* See if srcbox is in bounds. TXF doesn't clamp the coordinates. */
      use_txf =
         box.x >= 0 && box.x < src_width &&
         box.y >= 0 && box.y < src_height &&
         box.z >= 0 && box.z < src_depth &&
         box.x + box.width > 0 && box.x + box.width <= src_width &&
         box.y + box.height > 0 && box.y + box.height <= src_height &&
         box.z + box.depth > 0 && box.z + box.depth <= src_depth;
   }

   /* Check whether the states are properly saved. */
   util_blitter_set_running_flag(blitter);
   blitter_check_saved_vertex_states(ctx);
   blitter_check_saved_fragment_states(ctx);
   blitter_check_saved_textures(ctx);
   blitter_check_saved_fb_state(ctx);
   blitter_disable_render_cond(ctx);

   /* Blend, DSA, fragment shader. */
   if (dst_has_depth && dst_has_stencil) {
      pipe->bind_blend_state(pipe, ctx->blend[0][0]);
      pipe->bind_depth_stencil_alpha_state(pipe,
                                           ctx->dsa_write_depth_stencil);
      if (src_has_color) {
         assert(use_txf);
         ctx->bind_fs_state(pipe,
            blitter_get_fs_pack_color_zs(ctx, src_target,
                                         src_samples, dst->format, false));
      } else {
         ctx->bind_fs_state(pipe,
            blitter_get_fs_texfetch_depthstencil(ctx, src_target, src_samples,
                                                 dst_samples, use_txf));
      }
   } else if (dst_has_depth) {
      pipe->bind_blend_state(pipe, ctx->blend[0][0]);
      pipe->bind_depth_stencil_alpha_state(pipe,
                                           ctx->dsa_write_depth_keep_stencil);
      if (src_has_color &&
          (src->format == PIPE_FORMAT_R32_UINT ||
           src->format == PIPE_FORMAT_R32G32_UINT)) {
         assert(use_txf);
         ctx->bind_fs_state(pipe,
            blitter_get_fs_pack_color_zs(ctx, src_target,
                                         src_samples, dst->format, false));
      } else {
         ctx->bind_fs_state(pipe,
            blitter_get_fs_texfetch_depth(ctx, src_target, src_samples,
                                          dst_samples, use_txf));
      }
   } else if (dst_has_stencil) {
      pipe->bind_blend_state(pipe, ctx->blend[0][0]);
      pipe->bind_depth_stencil_alpha_state(pipe,
                                           ctx->dsa_keep_depth_write_stencil);

      assert(src_has_stencil); /* unpacking from color is unsupported */
      ctx->bind_fs_state(pipe,
         blitter_get_fs_texfetch_stencil(ctx, src_target, src_samples,
                                         dst_samples, use_txf));
   } else {
      unsigned colormask = mask & PIPE_MASK_RGBA;

      pipe->bind_blend_state(pipe, ctx->blend[colormask][alpha_blend]);
      pipe->bind_depth_stencil_alpha_state(pipe, ctx->dsa_keep_depth_stencil);

      if (src_has_depth &&
          (dst->format == PIPE_FORMAT_R32_UINT ||
           dst->format == PIPE_FORMAT_R32G32_UINT)) {
         assert(use_txf);
         ctx->bind_fs_state(pipe,
            blitter_get_fs_pack_color_zs(ctx, src_target,
                                         src_samples, src->format, true));
      } else {
         ctx->bind_fs_state(pipe,
            blitter_get_fs_texfetch_col(ctx, src->format, dst->format, src_target,
                                        src_samples, dst_samples, filter,
                                        use_txf));
      }
   }

   /* Set the linear filter only for scaled color non-MSAA blits. */
   if (filter == PIPE_TEX_FILTER_LINEAR) {
      if (src_target == PIPE_TEXTURE_RECT && ctx->has_texrect) {
         sampler_state = ctx->sampler_state_rect_linear;
      } else {
         sampler_state = ctx->sampler_state_linear;
      }
   } else {
      if (src_target == PIPE_TEXTURE_RECT && ctx->has_texrect) {
         sampler_state = ctx->sampler_state_rect;
      } else {
         sampler_state = ctx->sampler_state;
      }
   }

   /* Set samplers. */
   unsigned count = 0;
   if (src_has_depth && src_has_stencil &&
       (dst_has_color || (dst_has_depth && dst_has_stencil))) {
      /* Setup two samplers, one for depth and the other one for stencil. */
      struct pipe_sampler_view templ;
      struct pipe_sampler_view *views[2];
      void *samplers[2] = {sampler_state, sampler_state};

      templ = *src;
      templ.format = util_format_stencil_only(templ.format);
      assert(templ.format != PIPE_FORMAT_NONE);

      views[0] = src;
      views[1] = pipe->create_sampler_view(pipe, src->texture, &templ);

      count = 2;
      pipe->set_sampler_views(pipe, PIPE_SHADER_FRAGMENT, 0, 2, 0, false, views);
      pipe->bind_sampler_states(pipe, PIPE_SHADER_FRAGMENT, 0, 2, samplers);

      pipe_sampler_view_reference(&views[1], NULL);
   } else if (src_has_stencil && dst_has_stencil) {
      /* Set a stencil-only sampler view for it not to sample depth instead. */
      struct pipe_sampler_view templ;
      struct pipe_sampler_view *view;

      templ = *src;
      templ.format = util_format_stencil_only(templ.format);
      assert(templ.format != PIPE_FORMAT_NONE);

      view = pipe->create_sampler_view(pipe, src->texture, &templ);

      count = 1;
      pipe->set_sampler_views(pipe, PIPE_SHADER_FRAGMENT, 0, 1, 0, false, &view);
      pipe->bind_sampler_states(pipe, PIPE_SHADER_FRAGMENT,
                                0, 1, &sampler_state);

      pipe_sampler_view_reference(&view, NULL);
   } else {
      count = 1;
      pipe->set_sampler_views(pipe, PIPE_SHADER_FRAGMENT, 0, 1, 0, false, &src);
      pipe->bind_sampler_states(pipe, PIPE_SHADER_FRAGMENT,
                                0, 1, &sampler_state);
   }

   if (scissor) {
      pipe->set_scissor_states(pipe, 0, 1, scissor);
   }

   blitter_set_common_draw_rect_state(ctx, scissor != NULL, dst_samples > 1);

   do_blits(ctx, dst, dstbox, src, src_width0, src_height0,
            srcbox, dst_has_depth || dst_has_stencil, use_txf, sample0_only,
            dst_sample);

   util_blitter_restore_vertex_states(blitter);
   util_blitter_restore_fragment_states(blitter);
   util_blitter_restore_textures_internal(blitter, count);
   util_blitter_restore_fb_state(blitter);
   if (scissor) {
      pipe->set_scissor_states(pipe, 0, 1, &ctx->base.saved_scissor);
   }
   util_blitter_restore_render_cond(blitter);
   util_blitter_unset_running_flag(blitter);
}

void
util_blitter_blit(struct blitter_context *blitter,
                  const struct pipe_blit_info *info)
{
   struct pipe_resource *dst = info->dst.resource;
   struct pipe_resource *src = info->src.resource;
   struct blitter_context_priv *ctx = (struct blitter_context_priv*)blitter;
   struct pipe_context *pipe = ctx->base.pipe;
   struct pipe_surface *dst_view, dst_templ;
   struct pipe_sampler_view src_templ, *src_view;

   /* Initialize the surface. */
   util_blitter_default_dst_texture(&dst_templ, dst, info->dst.level,
                                    info->dst.box.z);
   dst_templ.format = info->dst.format;
   dst_view = pipe->create_surface(pipe, dst, &dst_templ);

   /* Initialize the sampler view. */
   util_blitter_default_src_texture(blitter, &src_templ, src, info->src.level);
   src_templ.format = info->src.format;
   src_view = pipe->create_sampler_view(pipe, src, &src_templ);

   /* Copy. */
   util_blitter_blit_generic(blitter, dst_view, &info->dst.box,
                             src_view, &info->src.box, src->width0, src->height0,
                             info->mask, info->filter,
                             info->scissor_enable ? &info->scissor : NULL,
                             info->alpha_blend, info->sample0_only,
                             info->dst_sample);

   pipe_surface_reference(&dst_view, NULL);
   pipe_sampler_view_reference(&src_view, NULL);
}

void util_blitter_generate_mipmap(struct blitter_context *blitter,
                                  struct pipe_resource *tex,
                                  enum pipe_format format,
                                  unsigned base_level, unsigned last_level,
                                  unsigned first_layer, unsigned last_layer)
{
   struct blitter_context_priv *ctx = (struct blitter_context_priv*)blitter;
   struct pipe_context *pipe = ctx->base.pipe;
   struct pipe_surface dst_templ, *dst_view;
   struct pipe_sampler_view src_templ, *src_view;
   bool is_depth;
   void *sampler_state;
   const struct util_format_description *desc =
         util_format_description(format);
   unsigned src_level;
   unsigned target = tex->target;

   if (ctx->cube_as_2darray &&
       (target == PIPE_TEXTURE_CUBE || target == PIPE_TEXTURE_CUBE_ARRAY))
      target = PIPE_TEXTURE_2D_ARRAY;

   assert(tex->nr_samples <= 1);
   /* Disallow stencil formats without depth. */
   assert(!util_format_has_stencil(desc) || util_format_has_depth(desc));

   is_depth = desc->colorspace == UTIL_FORMAT_COLORSPACE_ZS;

   /* Check whether the states are properly saved. */
   util_blitter_set_running_flag(blitter);
   blitter_check_saved_vertex_states(ctx);
   blitter_check_saved_fragment_states(ctx);
   blitter_check_saved_textures(ctx);
   blitter_check_saved_fb_state(ctx);
   blitter_disable_render_cond(ctx);

   /* Set states. */
   if (is_depth) {
      pipe->bind_blend_state(pipe, ctx->blend[0][0]);
      pipe->bind_depth_stencil_alpha_state(pipe,
                                           ctx->dsa_write_depth_keep_stencil);
      ctx->bind_fs_state(pipe,
                         blitter_get_fs_texfetch_depth(ctx, target, 1, 1, false));
   } else {
      pipe->bind_blend_state(pipe, ctx->blend[PIPE_MASK_RGBA][0]);
      pipe->bind_depth_stencil_alpha_state(pipe, ctx->dsa_keep_depth_stencil);
      ctx->bind_fs_state(pipe,
            blitter_get_fs_texfetch_col(ctx, tex->format, tex->format, target,
                                        1, 1, PIPE_TEX_FILTER_LINEAR, false));
   }

   if (target == PIPE_TEXTURE_RECT) {
      sampler_state = ctx->sampler_state_rect_linear;
   } else {
      sampler_state = ctx->sampler_state_linear;
   }
   pipe->bind_sampler_states(pipe, PIPE_SHADER_FRAGMENT,
                             0, 1, &sampler_state);

   blitter_set_common_draw_rect_state(ctx, false, false);

   for (src_level = base_level; src_level < last_level; src_level++) {
      struct pipe_box dstbox = {0}, srcbox = {0};
      unsigned dst_level = src_level + 1;

      dstbox.width = u_minify(tex->width0, dst_level);
      dstbox.height = u_minify(tex->height0, dst_level);

      srcbox.width = u_minify(tex->width0, src_level);
      srcbox.height = u_minify(tex->height0, src_level);

      if (target == PIPE_TEXTURE_3D) {
         dstbox.depth = util_num_layers(tex, dst_level);
         srcbox.depth = util_num_layers(tex, src_level);
      } else {
         dstbox.z = srcbox.z = first_layer;
         dstbox.depth = srcbox.depth = last_layer - first_layer + 1;
      }

      /* Initialize the surface. */
      util_blitter_default_dst_texture(&dst_templ, tex, dst_level,
                                       first_layer);
      dst_templ.format = format;
      dst_view = pipe->create_surface(pipe, tex, &dst_templ);

      /* Initialize the sampler view. */
      util_blitter_default_src_texture(blitter, &src_templ, tex, src_level);
      src_templ.format = format;
      src_view = pipe->create_sampler_view(pipe, tex, &src_templ);

      pipe->set_sampler_views(pipe, PIPE_SHADER_FRAGMENT, 0, 1, 0, false, &src_view);

      do_blits(ctx, dst_view, &dstbox, src_view, tex->width0, tex->height0,
               &srcbox, is_depth, false, false, 0);

      pipe_surface_reference(&dst_view, NULL);
      pipe_sampler_view_reference(&src_view, NULL);
   }

   util_blitter_restore_vertex_states(blitter);
   util_blitter_restore_fragment_states(blitter);
   util_blitter_restore_textures_internal(blitter, 1);
   util_blitter_restore_fb_state(blitter);
   util_blitter_restore_render_cond(blitter);
   util_blitter_unset_running_flag(blitter);
}

/* Clear a region of a color surface to a constant value. */
void util_blitter_clear_render_target(struct blitter_context *blitter,
                                      struct pipe_surface *dstsurf,
                                      const union pipe_color_union *color,
                                      unsigned dstx, unsigned dsty,
                                      unsigned width, unsigned height)
{
   struct blitter_context_priv *ctx = (struct blitter_context_priv*)blitter;
   struct pipe_context *pipe = ctx->base.pipe;
   struct pipe_framebuffer_state fb_state;
   bool msaa;
   unsigned num_layers;
   blitter_get_vs_func get_vs;

   assert(dstsurf->texture);
   if (!dstsurf->texture)
      return;

   /* check the saved state */
   util_blitter_set_running_flag(blitter);
   blitter_check_saved_vertex_states(ctx);
   blitter_check_saved_fragment_states(ctx);
   blitter_check_saved_fb_state(ctx);
   blitter_disable_render_cond(ctx);

   /* bind states */
   pipe->bind_blend_state(pipe, ctx->blend[PIPE_MASK_RGBA][0]);
   pipe->bind_depth_stencil_alpha_state(pipe, ctx->dsa_keep_depth_stencil);
   bind_fs_write_one_cbuf(ctx);

   /* set a framebuffer state */
   fb_state.width = dstsurf->width;
   fb_state.height = dstsurf->height;
   fb_state.nr_cbufs = 1;
   fb_state.cbufs[0] = dstsurf;
   fb_state.zsbuf = NULL;
   fb_state.resolve = NULL;
   pipe->set_framebuffer_state(pipe, &fb_state);
   pipe->set_sample_mask(pipe, ~0);
   if (pipe->set_min_samples)
      pipe->set_min_samples(pipe, 1);
   msaa = util_framebuffer_get_num_samples(&fb_state) > 1;

   blitter_set_dst_dimensions(ctx, dstsurf->width, dstsurf->height);
   blitter_set_common_draw_rect_state(ctx, false, msaa);

   union blitter_attrib attrib;
   memcpy(attrib.color, color->ui, sizeof(color->ui));

   num_layers = dstsurf->u.tex.last_layer - dstsurf->u.tex.first_layer + 1;

   if (num_layers > 1 && ctx->has_layered) {
      get_vs = get_vs_layered;
   } else {
      get_vs = get_vs_passthrough_pos_generic;
      num_layers = 1;
   }

   blitter->draw_rectangle(blitter, ctx->velem_state, get_vs,
                           dstx, dsty, dstx+width, dsty+height, 0,
                           num_layers, UTIL_BLITTER_ATTRIB_COLOR, &attrib);

   util_blitter_restore_vertex_states(blitter);
   util_blitter_restore_fragment_states(blitter);
   util_blitter_restore_fb_state(blitter);
   util_blitter_restore_render_cond(blitter);
   util_blitter_unset_running_flag(blitter);
}

/* Clear a region of a depth stencil surface. */
void util_blitter_clear_depth_stencil(struct blitter_context *blitter,
                                      struct pipe_surface *dstsurf,
                                      unsigned clear_flags,
                                      double depth,
                                      unsigned stencil,
                                      unsigned dstx, unsigned dsty,
                                      unsigned width, unsigned height)
{
   struct blitter_context_priv *ctx = (struct blitter_context_priv*)blitter;
   struct pipe_context *pipe = ctx->base.pipe;
   struct pipe_framebuffer_state fb_state;
   struct pipe_stencil_ref sr = { { 0 } };
   unsigned num_layers;

   assert(dstsurf->texture);
   if (!dstsurf->texture)
      return;

   /* check the saved state */
   util_blitter_set_running_flag(blitter);
   blitter_check_saved_vertex_states(ctx);
   blitter_check_saved_fragment_states(ctx);
   blitter_check_saved_fb_state(ctx);
   blitter_disable_render_cond(ctx);

   /* bind states */
   pipe->bind_blend_state(pipe, ctx->blend[0][0]);
   if ((clear_flags & PIPE_CLEAR_DEPTHSTENCIL) == PIPE_CLEAR_DEPTHSTENCIL) {
      sr.ref_value[0] = stencil & 0xff;
      pipe->bind_depth_stencil_alpha_state(pipe, ctx->dsa_write_depth_stencil);
      pipe->set_stencil_ref(pipe, sr);
   }
   else if (clear_flags & PIPE_CLEAR_DEPTH) {
      pipe->bind_depth_stencil_alpha_state(pipe, ctx->dsa_write_depth_keep_stencil);
   }
   else if (clear_flags & PIPE_CLEAR_STENCIL) {
      sr.ref_value[0] = stencil & 0xff;
      pipe->bind_depth_stencil_alpha_state(pipe, ctx->dsa_keep_depth_write_stencil);
      pipe->set_stencil_ref(pipe, sr);
   }
   else
      /* hmm that should be illegal probably, or make it a no-op somewhere */
      pipe->bind_depth_stencil_alpha_state(pipe, ctx->dsa_keep_depth_stencil);

   bind_fs_empty(ctx);

   /* set a framebuffer state */
   fb_state.width = dstsurf->width;
   fb_state.height = dstsurf->height;
   fb_state.nr_cbufs = 0;
   fb_state.cbufs[0] = NULL;
   fb_state.zsbuf = dstsurf;
   fb_state.resolve = NULL;
   pipe->set_framebuffer_state(pipe, &fb_state);
   pipe->set_sample_mask(pipe, ~0);
   if (pipe->set_min_samples)
      pipe->set_min_samples(pipe, 1);

   blitter_set_dst_dimensions(ctx, dstsurf->width, dstsurf->height);

   num_layers = dstsurf->u.tex.last_layer - dstsurf->u.tex.first_layer + 1;
   if (num_layers > 1 && ctx->has_layered) {
      blitter_set_common_draw_rect_state(ctx, false, false);
      blitter->draw_rectangle(blitter, ctx->velem_state, get_vs_layered,
                              dstx, dsty, dstx+width, dsty+height, depth,
                              num_layers, UTIL_BLITTER_ATTRIB_NONE, NULL);
   } else {
      blitter_set_common_draw_rect_state(ctx, false, false);
      blitter->draw_rectangle(blitter, ctx->velem_state,
                              get_vs_passthrough_pos,
                              dstx, dsty, dstx+width, dsty+height, depth, 1,
                              UTIL_BLITTER_ATTRIB_NONE, NULL);
   }

   util_blitter_restore_vertex_states(blitter);
   util_blitter_restore_fragment_states(blitter);
   util_blitter_restore_fb_state(blitter);
   util_blitter_restore_render_cond(blitter);
   util_blitter_unset_running_flag(blitter);
}

/* draw a rectangle across a region using a custom dsa stage - for r600g */
void util_blitter_custom_depth_stencil(struct blitter_context *blitter,
                                       struct pipe_surface *zsurf,
                                       struct pipe_surface *cbsurf,
                                       unsigned sample_mask,
                                       void *dsa_stage, float depth)
{
   struct blitter_context_priv *ctx = (struct blitter_context_priv*)blitter;
   struct pipe_context *pipe = ctx->base.pipe;
   struct pipe_framebuffer_state fb_state;

   assert(zsurf->texture);
   if (!zsurf->texture)
      return;

   /* check the saved state */
   util_blitter_set_running_flag(blitter);
   blitter_check_saved_vertex_states(ctx);
   blitter_check_saved_fragment_states(ctx);
   blitter_check_saved_fb_state(ctx);
   blitter_disable_render_cond(ctx);

   /* bind states */
   pipe->bind_blend_state(pipe, cbsurf ? ctx->blend[PIPE_MASK_RGBA][0] :
                                         ctx->blend[0][0]);
   pipe->bind_depth_stencil_alpha_state(pipe, dsa_stage);
   if (cbsurf)
      bind_fs_write_one_cbuf(ctx);
   else
      bind_fs_empty(ctx);

   /* set a framebuffer state */
   fb_state.width = zsurf->width;
   fb_state.height = zsurf->height;
   fb_state.nr_cbufs = 1;
   if (cbsurf) {
      fb_state.cbufs[0] = cbsurf;
      fb_state.nr_cbufs = 1;
   } else {
      fb_state.cbufs[0] = NULL;
      fb_state.nr_cbufs = 0;
   }
   fb_state.zsbuf = zsurf;
   fb_state.resolve = NULL;
   pipe->set_framebuffer_state(pipe, &fb_state);
   pipe->set_sample_mask(pipe, sample_mask);
   if (pipe->set_min_samples)
      pipe->set_min_samples(pipe, 1);

   blitter_set_common_draw_rect_state(ctx, false,
      util_framebuffer_get_num_samples(&fb_state) > 1);
   blitter_set_dst_dimensions(ctx, zsurf->width, zsurf->height);
   blitter->draw_rectangle(blitter, ctx->velem_state, get_vs_passthrough_pos,
                           0, 0, zsurf->width, zsurf->height, depth,
                           1, UTIL_BLITTER_ATTRIB_NONE, NULL);

   util_blitter_restore_vertex_states(blitter);
   util_blitter_restore_fragment_states(blitter);
   util_blitter_restore_fb_state(blitter);
   util_blitter_restore_render_cond(blitter);
   util_blitter_unset_running_flag(blitter);
}

void util_blitter_clear_buffer(struct blitter_context *blitter,
                               struct pipe_resource *dst,
                               unsigned offset, unsigned size,
                               unsigned num_channels,
                               const union pipe_color_union *clear_value)
{
   struct blitter_context_priv *ctx = (struct blitter_context_priv*)blitter;
   struct pipe_context *pipe = ctx->base.pipe;
   struct pipe_vertex_buffer vb = {0};
   struct pipe_stream_output_target *so_target = NULL;
   unsigned offsets[PIPE_MAX_SO_BUFFERS] = {0};

   assert(num_channels >= 1);
   assert(num_channels <= 4);

   /* IMPORTANT:  DON'T DO ANY BOUNDS CHECKING HERE!
    *
    * R600 uses this to initialize texture resources, so width0 might not be
    * what you think it is.
    */

   /* Streamout is required. */
   if (!ctx->has_stream_out) {
      assert(!"Streamout unsupported in util_blitter_clear_buffer()");
      return;
   }

   /* Some alignment is required. */
   if (offset % 4 != 0 || size % 4 != 0) {
      assert(!"Bad alignment in util_blitter_clear_buffer()");
      return;
   }

   u_upload_data(pipe->stream_uploader, 0, num_channels*4, 4, clear_value,
                 &vb.buffer_offset, &vb.buffer.resource);
   if (!vb.buffer.resource)
      goto out;

   util_blitter_set_running_flag(blitter);
   blitter_check_saved_vertex_states(ctx);
   blitter_disable_render_cond(ctx);

   pipe->set_vertex_buffers(pipe, 1, 0, false, &vb);
   pipe->bind_vertex_elements_state(pipe,
                                    ctx->velem_state_readbuf[num_channels-1]);
   bind_vs_pos_only(ctx, num_channels);
   if (ctx->has_geometry_shader)
      pipe->bind_gs_state(pipe, NULL);
   if (ctx->has_tessellation) {
      pipe->bind_tcs_state(pipe, NULL);
      pipe->bind_tes_state(pipe, NULL);
   }
   pipe->bind_rasterizer_state(pipe, ctx->rs_discard_state);

   so_target = pipe->create_stream_output_target(pipe, dst, offset, size);
   pipe->set_stream_output_targets(pipe, 1, &so_target, offsets);

   util_draw_arrays(pipe, MESA_PRIM_POINTS, 0, size / 4);

out:
   util_blitter_restore_vertex_states(blitter);
   util_blitter_restore_render_cond(blitter);
   util_blitter_unset_running_flag(blitter);
   pipe_so_target_reference(&so_target, NULL);
   pipe_resource_reference(&vb.buffer.resource, NULL);
}

/* probably radeon specific */
void util_blitter_custom_resolve_color(struct blitter_context *blitter,
                                       struct pipe_resource *dst,
                                       unsigned dst_level,
                                       unsigned dst_layer,
                                       struct pipe_resource *src,
                                       unsigned src_layer,
                                       unsigned sample_mask,
                                       void *custom_blend,
                                       enum pipe_format format)
{
   struct blitter_context_priv *ctx = (struct blitter_context_priv*)blitter;
   struct pipe_context *pipe = ctx->base.pipe;
   struct pipe_framebuffer_state fb_state;
   struct pipe_surface *srcsurf, *dstsurf, surf_tmpl;

   util_blitter_set_running_flag(blitter);
   blitter_check_saved_vertex_states(ctx);
   blitter_check_saved_fragment_states(ctx);
   blitter_disable_render_cond(ctx);

   /* bind states */
   pipe->bind_blend_state(pipe, custom_blend);
   pipe->bind_depth_stencil_alpha_state(pipe, ctx->dsa_keep_depth_stencil);
   bind_fs_write_one_cbuf(ctx);
   pipe->set_sample_mask(pipe, sample_mask);
   if (pipe->set_min_samples)
      pipe->set_min_samples(pipe, 1);

   memset(&surf_tmpl, 0, sizeof(surf_tmpl));
   surf_tmpl.format = format;
   surf_tmpl.u.tex.level = dst_level;
   surf_tmpl.u.tex.first_layer = dst_layer;
   surf_tmpl.u.tex.last_layer = dst_layer;

   dstsurf = pipe->create_surface(pipe, dst, &surf_tmpl);

   surf_tmpl.u.tex.level = 0;
   surf_tmpl.u.tex.first_layer = src_layer;
   surf_tmpl.u.tex.last_layer = src_layer;

   srcsurf = pipe->create_surface(pipe, src, &surf_tmpl);

   /* set a framebuffer state */
   fb_state.width = src->width0;
   fb_state.height = src->height0;
   fb_state.nr_cbufs = 2;
   fb_state.cbufs[0] = srcsurf;
   fb_state.cbufs[1] = dstsurf;
   fb_state.zsbuf = NULL;
   fb_state.resolve = NULL;
   pipe->set_framebuffer_state(pipe, &fb_state);

   blitter_set_common_draw_rect_state(ctx, false,
      util_framebuffer_get_num_samples(&fb_state) > 1);
   blitter_set_dst_dimensions(ctx, src->width0, src->height0);
   blitter->draw_rectangle(blitter, ctx->velem_state, get_vs_passthrough_pos,
                           0, 0, src->width0, src->height0,
                           0, 1, UTIL_BLITTER_ATTRIB_NONE, NULL);
   util_blitter_restore_fb_state(blitter);
   util_blitter_restore_vertex_states(blitter);
   util_blitter_restore_fragment_states(blitter);
   util_blitter_restore_render_cond(blitter);
   util_blitter_unset_running_flag(blitter);

   pipe_surface_reference(&srcsurf, NULL);
   pipe_surface_reference(&dstsurf, NULL);
}

void util_blitter_custom_color(struct blitter_context *blitter,
                               struct pipe_surface *dstsurf,
                               void *custom_blend)
{
   struct blitter_context_priv *ctx = (struct blitter_context_priv*)blitter;
   struct pipe_context *pipe = ctx->base.pipe;
   struct pipe_framebuffer_state fb_state;

   assert(dstsurf->texture);
   if (!dstsurf->texture)
      return;

   /* check the saved state */
   util_blitter_set_running_flag(blitter);
   blitter_check_saved_vertex_states(ctx);
   blitter_check_saved_fragment_states(ctx);
   blitter_check_saved_fb_state(ctx);
   blitter_disable_render_cond(ctx);

   /* bind states */
   pipe->bind_blend_state(pipe, custom_blend ? custom_blend
                                             : ctx->blend[PIPE_MASK_RGBA][0]);
   pipe->bind_depth_stencil_alpha_state(pipe, ctx->dsa_keep_depth_stencil);
   bind_fs_write_one_cbuf(ctx);

   /* set a framebuffer state */
   fb_state.width = dstsurf->width;
   fb_state.height = dstsurf->height;
   fb_state.nr_cbufs = 1;
   fb_state.cbufs[0] = dstsurf;
   fb_state.zsbuf = NULL;
   fb_state.resolve = NULL;
   pipe->set_framebuffer_state(pipe, &fb_state);
   pipe->set_sample_mask(pipe, ~0);
   if (pipe->set_min_samples)
      pipe->set_min_samples(pipe, 1);

   blitter_set_common_draw_rect_state(ctx, false,
      util_framebuffer_get_num_samples(&fb_state) > 1);
   blitter_set_dst_dimensions(ctx, dstsurf->width, dstsurf->height);
   blitter->draw_rectangle(blitter, ctx->velem_state, get_vs_passthrough_pos,
                           0, 0, dstsurf->width, dstsurf->height,
                           0, 1, UTIL_BLITTER_ATTRIB_NONE, NULL);

   util_blitter_restore_vertex_states(blitter);
   util_blitter_restore_fragment_states(blitter);
   util_blitter_restore_fb_state(blitter);
   util_blitter_restore_render_cond(blitter);
   util_blitter_unset_running_flag(blitter);
}

static void *get_custom_vs(struct blitter_context *blitter)
{
   struct blitter_context_priv *ctx = (struct blitter_context_priv*)blitter;

   return ctx->custom_vs;
}

/**
 * Performs a custom blit to the destination surface, using the VS and FS
 * provided.
 *
 * Used by vc4 for the 8-bit linear-to-tiled blit.
 */
void util_blitter_custom_shader(struct blitter_context *blitter,
                                struct pipe_surface *dstsurf,
                                void *custom_vs, void *custom_fs)
{
   struct blitter_context_priv *ctx = (struct blitter_context_priv*)blitter;
   struct pipe_context *pipe = ctx->base.pipe;
   struct pipe_framebuffer_state fb_state = { 0 };

   ctx->custom_vs = custom_vs;

   assert(dstsurf->texture);
   if (!dstsurf->texture)
      return;

   /* check the saved state */
   util_blitter_set_running_flag(blitter);
   blitter_check_saved_vertex_states(ctx);
   blitter_check_saved_fragment_states(ctx);
   blitter_check_saved_fb_state(ctx);
   blitter_disable_render_cond(ctx);

   /* bind states */
   pipe->bind_blend_state(pipe, ctx->blend[PIPE_MASK_RGBA][0]);
   pipe->bind_depth_stencil_alpha_state(pipe, ctx->dsa_keep_depth_stencil);
   pipe->bind_fs_state(pipe, custom_fs);

   /* set a framebuffer state */
   fb_state.width = dstsurf->width;
   fb_state.height = dstsurf->height;
   fb_state.nr_cbufs = 1;
   fb_state.cbufs[0] = dstsurf;
   fb_state.resolve = NULL;
   pipe->set_framebuffer_state(pipe, &fb_state);
   pipe->set_sample_mask(pipe, ~0);
   if (pipe->set_min_samples)
      pipe->set_min_samples(pipe, 1);

   blitter_set_common_draw_rect_state(ctx, false,
      util_framebuffer_get_num_samples(&fb_state) > 1);
   blitter_set_dst_dimensions(ctx, dstsurf->width, dstsurf->height);
   blitter->draw_rectangle(blitter, ctx->velem_state, get_custom_vs,
                           0, 0, dstsurf->width, dstsurf->height,
                           0, 1, UTIL_BLITTER_ATTRIB_NONE, NULL);

   util_blitter_restore_vertex_states(blitter);
   util_blitter_restore_fragment_states(blitter);
   util_blitter_restore_fb_state(blitter);
   util_blitter_restore_render_cond(blitter);
   util_blitter_unset_running_flag(blitter);
}

static void *
get_stencil_blit_fallback_fs(struct blitter_context_priv *ctx, bool msaa_src)
{
   if (!ctx->fs_stencil_blit_fallback[msaa_src]) {
      ctx->fs_stencil_blit_fallback[msaa_src] =
         util_make_fs_stencil_blit(ctx->base.pipe, msaa_src, ctx->has_txf_txq);
   }

   return ctx->fs_stencil_blit_fallback[msaa_src];
}

static void *
get_stencil_blit_fallback_dsa(struct blitter_context_priv *ctx, unsigned i)
{
   assert(i < ARRAY_SIZE(ctx->dsa_replicate_stencil_bit));
   if (!ctx->dsa_replicate_stencil_bit[i]) {
      struct pipe_depth_stencil_alpha_state dsa = { 0 };
      dsa.depth_func = PIPE_FUNC_ALWAYS;
      dsa.stencil[0].enabled = 1;
      dsa.stencil[0].func = PIPE_FUNC_ALWAYS;
      dsa.stencil[0].fail_op = PIPE_STENCIL_OP_REPLACE;
      dsa.stencil[0].zpass_op = PIPE_STENCIL_OP_REPLACE;
      dsa.stencil[0].zfail_op = PIPE_STENCIL_OP_REPLACE;
      dsa.stencil[0].valuemask = 0xff;
      dsa.stencil[0].writemask = 1u << i;

      ctx->dsa_replicate_stencil_bit[i] =
         ctx->base.pipe->create_depth_stencil_alpha_state(ctx->base.pipe, &dsa);
   }
   return ctx->dsa_replicate_stencil_bit[i];
}

/**
 * Performs a series of draws to implement stencil blits texture without
 * requiring stencil writes, updating a single bit per pixel at the time.
 */
void
util_blitter_stencil_fallback(struct blitter_context *blitter,
                              struct pipe_resource *dst,
                              unsigned dst_level,
                              const struct pipe_box *dstbox,
                              struct pipe_resource *src,
                              unsigned src_level,
                              const struct pipe_box *srcbox,
                              const struct pipe_scissor_state *scissor)
{
   struct blitter_context_priv *ctx = (struct blitter_context_priv *)blitter;
   struct pipe_context *pipe = ctx->base.pipe;

   /* check the saved state */
   util_blitter_set_running_flag(blitter);
   blitter_check_saved_vertex_states(ctx);
   blitter_check_saved_fragment_states(ctx);
   blitter_check_saved_fb_state(ctx);
   blitter_disable_render_cond(ctx);

   /* Initialize the surface. */
   struct pipe_surface *dst_view, dst_templ;
   util_blitter_default_dst_texture(&dst_templ, dst, dst_level, dstbox->z);
   dst_view = pipe->create_surface(pipe, dst, &dst_templ);

   /* Initialize the sampler view. */
   struct pipe_sampler_view src_templ, *src_view;
   util_blitter_default_src_texture(blitter, &src_templ, src, src_level);
   src_templ.format = util_format_stencil_only(src_templ.format);
   src_view = pipe->create_sampler_view(pipe, src, &src_templ);

   /* bind states */
   pipe->bind_blend_state(pipe, ctx->blend[PIPE_MASK_RGBA][0]);
   pipe->bind_fs_state(pipe,
      get_stencil_blit_fallback_fs(ctx, src->nr_samples > 1));

   /* set a framebuffer state */
   struct pipe_framebuffer_state fb_state = { 0 };
   fb_state.width = dstbox->x + dstbox->width;
   fb_state.height = dstbox->y + dstbox->height;
   fb_state.zsbuf = dst_view;
   fb_state.resolve = NULL;
   pipe->set_framebuffer_state(pipe, &fb_state);
   pipe->set_sample_mask(pipe, ~0);
   if (pipe->set_min_samples)
      pipe->set_min_samples(pipe, 1);

   blitter_set_common_draw_rect_state(ctx, scissor != NULL,
      util_framebuffer_get_num_samples(&fb_state) > 1);
   blitter_set_dst_dimensions(ctx, dst_view->width, dst_view->height);

   if (scissor) {
      pipe->clear_depth_stencil(pipe, dst_view, PIPE_CLEAR_STENCIL, 0.0, 0,
                                MAX2(dstbox->x, scissor->minx),
                                MAX2(dstbox->y, scissor->miny),
                                MIN2(dstbox->x + dstbox->width, scissor->maxx) - dstbox->x,
                                MIN2(dstbox->y + dstbox->height, scissor->maxy) - dstbox->y,
                                true);
      pipe->set_scissor_states(pipe, 0, 1, scissor);
   } else {
      pipe->clear_depth_stencil(pipe, dst_view, PIPE_CLEAR_STENCIL, 0.0, 0,
                                dstbox->x, dstbox->y,
                                dstbox->width, dstbox->height,
                                true);
   }

   pipe->set_sampler_views(pipe, PIPE_SHADER_FRAGMENT, 0, 1, 0, false, &src_view);
   pipe->bind_sampler_states(pipe, PIPE_SHADER_FRAGMENT, 0, 1, &ctx->sampler_state);

   unsigned stencil_bits =
      util_format_get_component_bits(dst->format,
                                     UTIL_FORMAT_COLORSPACE_ZS, 1);

   struct pipe_stencil_ref sr = { { (1u << stencil_bits) - 1 } };
   pipe->set_stencil_ref(pipe, sr);

   for (unsigned i = 0; i <= util_res_sample_count(dst) - 1; i++) {
      pipe->set_sample_mask(pipe, 1 << i);
      union blitter_attrib coord;
      get_texcoords(src_view, src->width0, src->height0,
                  srcbox->x, srcbox->y,
                  srcbox->x + srcbox->width, srcbox->y + srcbox->height,
                  srcbox->z, i, true,
                  &coord);

      for (int i = 0; i < stencil_bits; ++i) {
         uint32_t mask = 1 << i;
         struct pipe_constant_buffer cb = {
            .user_buffer = &mask,
            .buffer_size = sizeof(mask),
         };
         pipe->set_constant_buffer(pipe, PIPE_SHADER_FRAGMENT, blitter->cb_slot,
                                 false, &cb);

         pipe->bind_depth_stencil_alpha_state(pipe,
            get_stencil_blit_fallback_dsa(ctx, i));

         blitter->draw_rectangle(blitter, ctx->velem_state,
                                 get_vs_passthrough_pos_generic,
                                 dstbox->x, dstbox->y,
                                 dstbox->x + dstbox->width,
                                 dstbox->y + dstbox->height,
                                 0, 1,
                                 UTIL_BLITTER_ATTRIB_TEXCOORD_XYZW,
                                 &coord);
      }
   }

   if (scissor)
      pipe->set_scissor_states(pipe, 0, 1, &ctx->base.saved_scissor);

   util_blitter_restore_vertex_states(blitter);
   util_blitter_restore_fragment_states(blitter);
   util_blitter_restore_textures_internal(blitter, 1);
   util_blitter_restore_fb_state(blitter);
   util_blitter_restore_render_cond(blitter);
   util_blitter_restore_constant_buffer_state(blitter);
   util_blitter_unset_running_flag(blitter);

   pipe_surface_reference(&dst_view, NULL);
   pipe_sampler_view_reference(&src_view, NULL);
}
