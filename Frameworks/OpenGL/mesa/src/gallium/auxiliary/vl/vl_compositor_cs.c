/**************************************************************************
 *
 * Copyright 2019 Advanced Micro Devices, Inc.
 * All Rights Reserved.
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
 * Authors: James Zhu <james.zhu<@amd.com>
 *
 **************************************************************************/

#include <assert.h>

#include "nir/nir_builder.h"
#include "vl_compositor_cs.h"

struct cs_viewport {
   float scale_x;
   float scale_y;
   struct u_rect area;
   float crop_x; /* src */
   float crop_y;
   int translate_x; /* dst */
   int translate_y;
   float sampler0_w;
   float sampler0_h;
   float clamp_x;
   float clamp_y;
   float chroma_clamp_x;
   float chroma_clamp_y;
   float chroma_offset_x;
   float chroma_offset_y;
};

struct cs_shader {
   nir_builder b;
   const char *name;
   bool array;
   unsigned num_samplers;
   nir_variable *samplers[3];
   nir_variable *image;
   nir_def *params[8];
   nir_def *fone;
   nir_def *fzero;
};

enum coords_flags {
   COORDS_LUMA          = 0x0,
   COORDS_CHROMA        = 0x1,
   COORDS_CHROMA_OFFSET = 0x2,
};

static nir_def *cs_create_shader(struct vl_compositor *c, struct cs_shader *s)
{
   /*
      #version 450

      layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
      layout (binding = 0) uniform sampler2DRect samplers[3]; // or sampler2DArray
      layout (binding = 0) uniform image2D image;

      layout (std140, binding = 0) uniform ubo
      {
         vec4 csc_mat[3];      // params[0-2]
         float luma_min;       // params[3].x
         float luma_max;       // params[3].y
         vec2 scale;           // params[3].zw
         vec2 crop;            // params[4].xy
         ivec2 translate;      // params[4].zw
         vec2 sampler0_wh;     // params[5].xy
         vec2 subsample_ratio; // params[5].zw
         vec2 coord_clamp;     // params[6].xy
         vec2 chroma_clamp;    // params[6].zw
         vec2 chroma_offset;   // params[7].xy
      };

      void main()
      {
         ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
      }
   */
   enum glsl_sampler_dim sampler_dim = s->array ? GLSL_SAMPLER_DIM_2D : GLSL_SAMPLER_DIM_RECT;
   const struct glsl_type *sampler_type =
      glsl_sampler_type(sampler_dim, /*is_shadow*/ false, s->array, GLSL_TYPE_FLOAT);
   const struct glsl_type *image_type =
      glsl_image_type(GLSL_SAMPLER_DIM_2D, /*is_array*/ false, GLSL_TYPE_FLOAT);
   const nir_shader_compiler_options *options =
      c->pipe->screen->get_compiler_options(c->pipe->screen, PIPE_SHADER_IR_NIR, PIPE_SHADER_COMPUTE);

   s->b = nir_builder_init_simple_shader(MESA_SHADER_COMPUTE, options, "vl:%s", s->name);
   nir_builder *b = &s->b;
   b->shader->info.workgroup_size[0] = 8;
   b->shader->info.workgroup_size[1] = 8;
   b->shader->info.workgroup_size[2] = 1;
   b->shader->info.num_ubos = 1;
   b->shader->num_uniforms = ARRAY_SIZE(s->params);

   nir_def *zero = nir_imm_int(b, 0);
   for (unsigned i = 0; i < b->shader->num_uniforms; ++i)
      s->params[i] = nir_load_ubo(b, 4, 32, zero, nir_imm_int(b, i * 16), .align_mul = 4, .range = ~0);

   for (unsigned i = 0; i < s->num_samplers; ++i) {
      s->samplers[i] = nir_variable_create(b->shader, nir_var_uniform, sampler_type, "sampler");
      s->samplers[i]->data.binding = i;
      BITSET_SET(b->shader->info.textures_used, i);
      BITSET_SET(b->shader->info.samplers_used, i);
   }

   s->image = nir_variable_create(b->shader, nir_var_image, image_type, "image");
   s->image->data.binding = 0;
   BITSET_SET(b->shader->info.images_used, 0);

   s->fone = nir_imm_float(b, 1.0f);
   s->fzero = nir_imm_float(b, 0.0f);

   nir_def *block_ids = nir_load_workgroup_id(b);
   nir_def *local_ids = nir_load_local_invocation_id(b);
   return nir_iadd(b, nir_imul(b, block_ids, nir_imm_ivec3(b, 8, 8, 1)), local_ids);
}

static void *cs_create_shader_state(struct vl_compositor *c, struct cs_shader *s)
{
   c->pipe->screen->finalize_nir(c->pipe->screen, s->b.shader);

   struct pipe_compute_state state = {0};
   state.ir_type = PIPE_SHADER_IR_NIR;
   state.prog = s->b.shader;

   /* create compute shader */
   return c->pipe->create_compute_state(c->pipe, &state);
}

static inline nir_def *cs_translate(struct cs_shader *s, nir_def *src)
{
   /*
      return src.xy + params[4].zw;
   */
   nir_builder *b = &s->b;
   return nir_iadd(b, src, nir_channels(b, s->params[4], 0x3 << 2));
}

static inline nir_def *cs_texture_offset(struct cs_shader *s, nir_def *src)
{
   /*
      return src.xy + 0.5;
   */
   nir_builder *b = &s->b;
   return nir_fadd_imm(b, src, 0.5f);
}

static inline nir_def *cs_chroma_subsampling(struct cs_shader *s, nir_def *src)
{
   /*
      return src.xy * params[5].zw;
   */
   nir_builder *b = &s->b;
   return nir_fmul(b, src, nir_channels(b, s->params[5], 0x3 << 2));
}

static inline nir_def *cs_scale(struct cs_shader *s, nir_def *src)
{
   /*
      return src.xy / params[3].zw;
   */
   nir_builder *b = &s->b;
   return nir_fdiv(b, src, nir_channels(b, s->params[3], 0x3 << 2));
}

static inline nir_def *cs_luma_key(struct cs_shader *s, nir_def *src)
{
   /*
      bool luma_min = params[3].x >= src;
      bool luma_max = params[3].y < src;
      return float(luma_min || luma_max);
   */
   nir_builder *b = &s->b;
   nir_def *luma_min = nir_fge(b, nir_channel(b, s->params[3], 0), src);
   nir_def *luma_max = nir_flt(b, nir_channel(b, s->params[3], 1), src);
   return nir_b2f32(b, nir_ior(b, luma_min, luma_max));
}

static inline nir_def *cs_chroma_offset(struct cs_shader *s, nir_def *src, unsigned flags)
{
   /*
      vec2 offset = params[7].xy;
      if (flags & COORDS_CHROMA)
         return src.xy + offset;
      return offset * -0.5 + src.xy;
   */
   nir_builder *b = &s->b;
   nir_def *offset = nir_channels(b, s->params[7], 0x3);
   if (flags & COORDS_CHROMA)
      return nir_fadd(b, src, offset);
   return nir_ffma_imm1(b, offset, -0.5f, src);
}

static inline nir_def *cs_clamp(struct cs_shader *s, nir_def *src, unsigned flags)
{
   /*
      vec2 coord_max;
      if (flags & COORDS_CHROMA)
         coord_max = params[6].zw;
      else
         coord_max = params[6].xy;
      return min(src.xy, coord_max);
   */
   nir_builder *b = &s->b;
   nir_component_mask_t mask = flags & COORDS_CHROMA ? 0x3 << 2 : 0x3;
   return nir_fmin(b, src, nir_channels(b, s->params[6], mask));
}

static inline nir_def *cs_normalize(struct cs_shader *s, nir_def *src, unsigned flags)
{
   /*
      vec2 div = params[5].xy;
      if (flags & COORDS_CHROMA)
         div = cs_chroma_subsampling(div);
      return src.xy / div;
   */
   nir_builder *b = &s->b;
   nir_def *div = nir_channels(b, s->params[5], 0x3);
   if (flags & COORDS_CHROMA)
      div = cs_chroma_subsampling(s, div);
   return nir_fdiv(b, src, div);
}

static inline nir_def *cs_crop(struct cs_shader *s, nir_def *src, unsigned flags)
{
   /*
      vec2 crop = params[4].xy;
      if (flags & COORDS_CHROMA)
         crop = cs_chroma_subsampling(crop);
      return src.xy + crop;
   */
   nir_builder *b = &s->b;
   nir_def *crop = nir_channels(b, s->params[4], 0x3);
   if (flags & COORDS_CHROMA)
      crop = cs_chroma_subsampling(s, crop);
   return nir_fadd(b, src, crop);
}

static inline nir_def *cs_color_space_conversion(struct cs_shader *s, nir_def *src, unsigned comp)
{
   /*
      return dot(src, params[comp]);
   */
   nir_builder *b = &s->b;
   return nir_fdot4(b, src, s->params[comp]);
}

static inline nir_def *cs_fetch_texel(struct cs_shader *s, nir_def *coords, unsigned sampler)
{
   /*
      return texture(samplers[sampler], s->array ? coords.xyz : coords.xy);
   */
   nir_builder *b = &s->b;
   nir_deref_instr *tex_deref = nir_build_deref_var(b, s->samplers[sampler]);
   nir_component_mask_t mask = s->array ? 0x7 : 0x3;
   return nir_tex_deref(b, tex_deref, tex_deref, nir_channels(b, coords, mask));
}

static inline void cs_image_store(struct cs_shader *s, nir_def *pos, nir_def *color)
{
   /*
      imageStore(image, pos.xy, color);
   */
   nir_builder *b = &s->b;
   nir_def *zero = nir_imm_int(b, 0);
   nir_def *undef32 = nir_undef(b, 1, 32);
   pos = nir_pad_vector_imm_int(b, pos, 0, 4);
   nir_image_deref_store(b, &nir_build_deref_var(b, s->image)->def, pos, undef32, color, zero);
}

static nir_def *cs_tex_coords(struct cs_shader *s, nir_def *coords, unsigned flags)
{
   nir_builder *b = &s->b;

   coords = nir_u2f32(b, coords);
   coords = cs_texture_offset(s, coords);

   if (flags & COORDS_CHROMA_OFFSET)
      coords = cs_chroma_offset(s, coords, flags);

   if (flags & COORDS_CHROMA)
      coords = cs_chroma_subsampling(s, coords);

   coords = cs_scale(s, coords);
   coords = cs_crop(s, coords, flags);
   coords = cs_clamp(s, coords, flags);

   return coords;
}

static void *create_video_buffer_shader(struct vl_compositor *c)
{
   struct cs_shader s = {
      .name = "video_buffer",
      .num_samplers = 3,
   };
   nir_builder *b = &s.b;

   nir_def *ipos = cs_create_shader(c, &s);
   nir_def *pos[2] = {
      cs_tex_coords(&s, ipos, COORDS_LUMA),
      cs_tex_coords(&s, ipos, COORDS_CHROMA | COORDS_CHROMA_OFFSET),
   };

   nir_def *col[3];
   for (unsigned i = 0; i < 3; ++i)
      col[i] = cs_fetch_texel(&s, pos[MIN2(i, 1)], i);

   nir_def *alpha = cs_luma_key(&s, col[2]);

   nir_def *color = nir_vec4(b, col[0], col[1], col[2], s.fone);
   for (unsigned i = 0; i < 3; ++i)
      col[i] = cs_color_space_conversion(&s, color, i);

   color = nir_vec4(b, col[0], col[1], col[2], alpha);
   cs_image_store(&s, cs_translate(&s, ipos), color);

   return cs_create_shader_state(c, &s);
}

static void *create_yuv_progressive_shader(struct vl_compositor *c, bool y)
{
   struct cs_shader s = {
      .name = y ? "yuv_progressive_y" : "yuv_progressive_uv",
      .num_samplers = 3,
   };
   nir_builder *b = &s.b;

   nir_def *ipos = cs_create_shader(c, &s);
   nir_def *pos = cs_tex_coords(&s, ipos, y ? COORDS_LUMA : COORDS_CHROMA);

   nir_def *color;
   if (y) {
      color = nir_channel(b, cs_fetch_texel(&s, pos, 0), 0);
   } else {
      nir_def *col1 = cs_fetch_texel(&s, pos, 1);
      nir_def *col2 = cs_fetch_texel(&s, pos, 2);
      color = nir_vec2(b, col1, col2);
   }

   cs_image_store(&s, cs_translate(&s, ipos), color);

   return cs_create_shader_state(c, &s);
}

static void *create_rgb_yuv_shader(struct vl_compositor *c, bool y)
{
   struct cs_shader s = {
      .name = y ? "rgb_yuv_y" : "rgb_yuv_uv",
      .num_samplers = 1,
   };
   nir_builder *b = &s.b;

   nir_def *ipos = cs_create_shader(c, &s);
   nir_def *color = NULL;

   if (y) {
      nir_def *pos = cs_tex_coords(&s, ipos, COORDS_LUMA);
      color = cs_fetch_texel(&s, pos, 0);
   } else {
      /*
         vec2 pos[4];
         pos[0] = vec2(ipos);
         pos[0] = cs_texture_offset(pos[0]);
         pos[0] = cs_chroma_offset(pos[0], COORDS_LUMA);

         // Sample offset
         pos[3] = pos[0] + vec2( 0.25, -0.25);
         pos[2] = pos[0] + vec2(-0.25,  0.25);
         pos[1] = pos[0] + vec2(-0.25, -0.25);
         pos[0] = pos[0] + vec2( 0.25,  0.25);

         vec4 col[4];
         for (uint i = 0; i < 4; ++i) {
            pos[i] = cs_scale(pos[i]);
            pos[i] = cs_crop(pos[i], COORDS_LUMA);
            pos[i] = cs_clamp(pos[i], COORDS_LUMA);
            col[i] = texture(samp[0], pos[i]);
         }
         color = (col[0] + col[1] + col[2] + col[3]) * 0.25;
      */
      nir_def *pos[4];
      pos[0] = nir_u2f32(b, ipos);
      pos[0] = cs_texture_offset(&s, pos[0]);
      pos[0] = cs_chroma_offset(&s, pos[0], COORDS_LUMA);

      /* Sample offset */
      nir_def *o_plus = nir_imm_float(b, 0.25f);
      nir_def *o_minus = nir_imm_float(b, -0.25f);
      pos[3] = nir_fadd(b, pos[0], nir_vec2(b, o_plus, o_minus));
      pos[2] = nir_fadd(b, pos[0], nir_vec2(b, o_minus, o_plus));
      pos[1] = nir_fadd(b, pos[0], nir_vec2(b, o_minus, o_minus));
      pos[0] = nir_fadd(b, pos[0], nir_vec2(b, o_plus, o_plus));

      for (unsigned i = 0; i < 4; ++i) {
         pos[i] = cs_scale(&s, pos[i]);
         pos[i] = cs_crop(&s, pos[i], COORDS_LUMA);
         pos[i] = cs_clamp(&s, pos[i], COORDS_LUMA);

         nir_def *c = cs_fetch_texel(&s, pos[i], 0);
         color = color ? nir_fadd(b, color, c) : c;
      }
      color = nir_fmul_imm(b, color, 0.25f);
   }

   color = nir_vector_insert_imm(b, color, s.fone, 3);

   if (y) {
      color = cs_color_space_conversion(&s, color, 0);
   } else {
      nir_def *col1 = cs_color_space_conversion(&s, color, 1);
      nir_def *col2 = cs_color_space_conversion(&s, color, 2);
      color = nir_vec2(b, col1, col2);
   }

   cs_image_store(&s, cs_translate(&s, ipos), color);

   return cs_create_shader_state(c, &s);
}

static nir_def *create_weave_shader(struct vl_compositor *c, bool rgb, bool y)
{
   struct cs_shader s = {
      .name = rgb ? "weave" : y ? "yuv_weave_y" : "yuv_weave_uv",
      .array = true,
      .num_samplers = 3,
   };
   nir_builder *b = &s.b;

   nir_def *ipos = cs_create_shader(c, &s);

   /*
      vec2 top_y = cs_texture_offset(vec2(ipos));
      vec2 top_uv = rgb ? cs_chroma_offset(top_y, COORDS_CHROMA) : top_y;
      top_uv = cs_chroma_subsampling(top_uv);
      vec2 down_y = top_y;
      vec2 down_uv = top_uv;

      top_y = cs_crop(cs_scale(top_y), COORDS_LUMA);
      top_uv = cs_crop(cs_scale(top_uv), COORDS_CHROMA);
      down_y = cs_crop(cs_scale(down_y), COORDS_LUMA);
      down_uv = cs_crop(cs_scale(down_uv), COORDS_CHROMA);

      // Weave offset
      top_y = top_y + vec2(0.0, 0.25);
      top_uv = top_uv + vec2(0.0, 0.25);
      down_y = down_y + vec2(0.0, -0.25);
      down_uv = down_uv + vec2(0.0, -0.25);

      // Texture layer
      vec3 tex_layer = vec3(top_y.y, top_uv.y, top_uv.y);
      tex_layer = tex_layer + round(tex_layer) * -1.0;
      tex_layer = abs(tex_layer) * 2.0;

      top_y = cs_clamp(top_y, COORDS_LUMA);
      top_y = cs_normalize(top_y, COORDS_LUMA);
      top_uv = cs_clamp(top_uv, COORDS_CHROMA);
      top_uv = cs_normalize(top_uv, COORDS_CHROMA);
      down_y = cs_clamp(down_y, COORDS_LUMA);
      down_y = cs_normalize(down_y, COORDS_LUMA);
      down_uv = cs_clamp(down_uv, COORDS_CHROMA);
      down_uv = cs_normalize(down_uv, COORDS_CHROMA);

      vec4 top_col, down_col;
      top_col.x = texture(samp[0], vec3(top_y, 0.0)).x;
      top_col.y = texture(samp[1], vec3(top_uv, 0.0)).x;
      top_col.z = texture(samp[2], vec3(top_uv, 0.0)).x;
      top_col.w = 1.0;
      down_col.x = texture(samp[0], vec3(down_y, 1.0)).x;
      down_col.y = texture(samp[1], vec3(down_uv, 1.0)).x;
      down_col.z = texture(samp[2], vec3(down_uv, 1.0)).x;
      down_col.w = 1.0;

      vec4 color = mix(down_col, top_col, tex_layer);
   */
   nir_def *pos[4];
   /* Top Y */
   pos[0] = nir_u2f32(b, ipos);
   pos[0] = cs_texture_offset(&s, pos[0]);
   /* Top UV */
   pos[1] = rgb ? cs_chroma_offset(&s, pos[0], COORDS_CHROMA) : pos[0];
   pos[1] = cs_chroma_subsampling(&s, pos[1]);
   /* Down Y */
   pos[2] = pos[0];
   /* Down UV */
   pos[3] = pos[1];

   /* Weave offset */
   nir_def *o_plus = nir_imm_vec2(b, 0.0f, 0.25f);
   nir_def *o_minus = nir_imm_vec2(b, 0.0f, -0.25f);
   for (unsigned i = 0; i < 4; ++i) {
      pos[i] = cs_scale(&s, pos[i]);
      pos[i] = cs_crop(&s, pos[i], i % 2 ? COORDS_CHROMA : COORDS_LUMA);
      pos[i] = nir_fadd(b, pos[i], i < 2 ? o_plus : o_minus);
   }

   /* Texture layer */
   nir_def *tex_layer = nir_vec3(b,
                                 nir_channel(b, pos[0], 1),
                                 nir_channel(b, pos[1], 1),
                                 nir_channel(b, pos[1], 1));
   tex_layer = nir_fadd(b, tex_layer,
                        nir_fneg(b, nir_fround_even(b, tex_layer)));
   tex_layer = nir_fabs(b, tex_layer);
   tex_layer = nir_fmul_imm(b, tex_layer, 2.0f);

   nir_def *col[6];
   for (unsigned i = 0; i < 4; ++i) {
      bool top = i < 2;
      unsigned j = top ? 0 : 3;
      unsigned flags = i % 2 ? COORDS_CHROMA : COORDS_LUMA;
      pos[i] = cs_clamp(&s, pos[i], flags);
      pos[i] = cs_normalize(&s, pos[i], flags);
      pos[i] = nir_vector_insert_imm(b, pos[i],
                                     top ? s.fzero : s.fone, 2);
      if (flags == COORDS_LUMA) {
         col[j] = cs_fetch_texel(&s, pos[i], 0);
      } else {
         col[j + 1] = cs_fetch_texel(&s, pos[i], 1);
         col[j + 2] = cs_fetch_texel(&s, pos[i], 2);
      }
   }

   nir_def *color_top = nir_vec4(b, col[0], col[1], col[2], s.fone);
   nir_def *color_down = nir_vec4(b, col[3], col[4], col[5], s.fone);
   nir_def *color = nir_flrp(b, color_down, color_top, tex_layer);

   if (rgb) {
      nir_def *alpha = cs_luma_key(&s, nir_channel(b, color, 2));
      for (unsigned i = 0; i < 3; ++i)
         col[i] = cs_color_space_conversion(&s, color, i);
      color = nir_vec4(b, col[0], col[1], col[2], alpha);
   } else if (y) {
      color = nir_channel(b, color, 0);
   } else {
      nir_def *col1 = nir_channel(b, color, 1);
      nir_def *col2 = nir_channel(b, color, 2);
      color = nir_vec2(b, col1, col2);
   }

   cs_image_store(&s, cs_translate(&s, ipos), color);

   return cs_create_shader_state(c, &s);
}

static void
cs_launch(struct vl_compositor *c,
          void                 *cs,
          const struct u_rect  *draw_area)
{
   struct pipe_context *ctx = c->pipe;
   unsigned width, height;

   width = draw_area->x1 - draw_area->x0;
   height = draw_area->y1 - draw_area->y0;

   /* Bind the image */
   struct pipe_image_view image = {0};
   image.resource = c->fb_state.cbufs[0]->texture;
   image.shader_access = image.access = PIPE_IMAGE_ACCESS_READ_WRITE;
   image.format = c->fb_state.cbufs[0]->texture->format;

   ctx->set_shader_images(c->pipe, PIPE_SHADER_COMPUTE, 0, 1, 0, &image);

   /* Bind compute shader */
   ctx->bind_compute_state(ctx, cs);

   /* Dispatch compute */
   struct pipe_grid_info info = {0};
   info.block[0] = 8;
   info.last_block[0] = width % info.block[0];
   info.block[1] = 8;
   info.last_block[1] = height % info.block[1];
   info.block[2] = 1;
   info.grid[0] = DIV_ROUND_UP(width, info.block[0]);
   info.grid[1] = DIV_ROUND_UP(height, info.block[1]);
   info.grid[2] = 1;

   ctx->launch_grid(ctx, &info);

   /* Make the result visible to all clients. */
   ctx->memory_barrier(ctx, PIPE_BARRIER_ALL);

}

static inline struct u_rect
calc_drawn_area(struct vl_compositor_state *s,
                struct vl_compositor_layer *layer)
{
   struct vertex2f tl, br;
   struct u_rect result;

   assert(s && layer);

   tl = layer->dst.tl;
   br = layer->dst.br;

   /* Scale */
   result.x0 = tl.x * layer->viewport.scale[0] + layer->viewport.translate[0];
   result.y0 = tl.y * layer->viewport.scale[1] + layer->viewport.translate[1];
   result.x1 = br.x * layer->viewport.scale[0] + layer->viewport.translate[0];
   result.y1 = br.y * layer->viewport.scale[1] + layer->viewport.translate[1];

   /* Clip */
   result.x0 = MAX2(result.x0, s->scissor.minx);
   result.y0 = MAX2(result.y0, s->scissor.miny);
   result.x1 = MIN2(result.x1, s->scissor.maxx);
   result.y1 = MIN2(result.y1, s->scissor.maxy);
   return result;
}

static inline float
chroma_offset_x(unsigned location)
{
   if (location & VL_COMPOSITOR_LOCATION_HORIZONTAL_LEFT)
      return 0.5f;
   else
      return 0.0f;
}

static inline float
chroma_offset_y(unsigned location)
{
   if (location & VL_COMPOSITOR_LOCATION_VERTICAL_TOP)
      return 0.5f;
   else if (location & VL_COMPOSITOR_LOCATION_VERTICAL_BOTTOM)
      return -0.5f;
   else
      return 0.0f;
}

static bool
set_viewport(struct vl_compositor_state *s,
             struct cs_viewport         *drawn,
             struct pipe_sampler_view **samplers)
{
   struct pipe_transfer *buf_transfer;

   assert(s && drawn);

   void *ptr = pipe_buffer_map(s->pipe, s->shader_params,
                               PIPE_MAP_WRITE | PIPE_MAP_DISCARD_WHOLE_RESOURCE,
                               &buf_transfer);

   if (!ptr)
     return false;

   memcpy(ptr, &s->csc_matrix, sizeof(vl_csc_matrix));

   float *ptr_float = (float *)ptr;
   ptr_float += sizeof(vl_csc_matrix) / sizeof(float);
   *ptr_float++ = s->luma_min;
   *ptr_float++ = s->luma_max;
   *ptr_float++ = drawn->scale_x;
   *ptr_float++ = drawn->scale_y;
   *ptr_float++ = drawn->crop_x;
   *ptr_float++ = drawn->crop_y;

   int *ptr_int = (int *)ptr_float;
   *ptr_int++ = drawn->translate_x;
   *ptr_int++ = drawn->translate_y;

   ptr_float = (float *)ptr_int;
   *ptr_float++ = drawn->sampler0_w;
   *ptr_float++ = drawn->sampler0_h;

   /* compute_shader_video_buffer uses pixel coordinates based on the
    * Y sampler dimensions. If U/V are using separate planes and are
    * subsampled, we need to scale the coordinates */
   if (samplers[1]) {
      float h_ratio = samplers[1]->texture->width0 /
                     (float) samplers[0]->texture->width0;
      *ptr_float++ = h_ratio;
      float v_ratio = samplers[1]->texture->height0 /
                     (float) samplers[0]->texture->height0;
      *ptr_float++ = v_ratio;
   }
   else {
      *ptr_float++ = 1.0f;
      *ptr_float++ = 1.0f;
   }


   *ptr_float++ = drawn->clamp_x;
   *ptr_float++ = drawn->clamp_y;
   *ptr_float++ = drawn->chroma_clamp_x;
   *ptr_float++ = drawn->chroma_clamp_y;
   *ptr_float++ = drawn->chroma_offset_x;
   *ptr_float++ = drawn->chroma_offset_y;

   pipe_buffer_unmap(s->pipe, buf_transfer);

   return true;
}

static void
draw_layers(struct vl_compositor       *c,
            struct vl_compositor_state *s,
            struct u_rect              *dirty)
{
   unsigned i;

   assert(c);

   for (i = 0; i < VL_COMPOSITOR_MAX_LAYERS; ++i) {
      if (s->used_layers & (1 << i)) {
         struct vl_compositor_layer *layer = &s->layers[i];
         struct pipe_sampler_view **samplers = &layer->sampler_views[0];
         unsigned num_sampler_views = !samplers[1] ? 1 : !samplers[2] ? 2 : 3;
         struct pipe_sampler_view *sampler1 = samplers[1] ? samplers[1] : samplers[0];
         struct cs_viewport drawn;

         drawn.area = calc_drawn_area(s, layer);
         drawn.scale_x = layer->viewport.scale[0] /
            ((float)layer->sampler_views[0]->texture->width0 *
             (layer->src.br.x - layer->src.tl.x));
         drawn.scale_y  = layer->viewport.scale[1] /
            ((float)layer->sampler_views[0]->texture->height0 *
             (layer->src.br.y - layer->src.tl.y));
         drawn.crop_x = layer->src.tl.x * layer->sampler_views[0]->texture->width0;
         drawn.translate_x = layer->viewport.translate[0];
         drawn.crop_y = layer->src.tl.y * layer->sampler_views[0]->texture->height0;
         drawn.translate_y = layer->viewport.translate[1];
         drawn.sampler0_w = (float)layer->sampler_views[0]->texture->width0;
         drawn.sampler0_h = (float)layer->sampler_views[0]->texture->height0;
         drawn.clamp_x = (float)samplers[0]->texture->width0 * layer->src.br.x - 0.5;
         drawn.clamp_y = (float)samplers[0]->texture->height0 * layer->src.br.y - 0.5;
         drawn.chroma_clamp_x = (float)sampler1->texture->width0 * layer->src.br.x - 0.5;
         drawn.chroma_clamp_y = (float)sampler1->texture->height0 * layer->src.br.y - 0.5;
         drawn.chroma_offset_x = chroma_offset_x(s->chroma_location);
         drawn.chroma_offset_y = chroma_offset_y(s->chroma_location);
         set_viewport(s, &drawn, samplers);

         c->pipe->bind_sampler_states(c->pipe, PIPE_SHADER_COMPUTE, 0,
                        num_sampler_views, layer->samplers);
         c->pipe->set_sampler_views(c->pipe, PIPE_SHADER_COMPUTE, 0,
                        num_sampler_views, 0, false, samplers);

         cs_launch(c, layer->cs, &(drawn.area));

         /* Unbind. */
         c->pipe->set_shader_images(c->pipe, PIPE_SHADER_COMPUTE, 0, 0, 1, NULL);
         c->pipe->set_constant_buffer(c->pipe, PIPE_SHADER_COMPUTE, 0, false, NULL);
         c->pipe->set_sampler_views(c->pipe, PIPE_SHADER_FRAGMENT, 0, 0,
                        num_sampler_views, false, NULL);
         c->pipe->bind_compute_state(c->pipe, NULL);
         c->pipe->bind_sampler_states(c->pipe, PIPE_SHADER_COMPUTE, 0,
                        num_sampler_views, NULL);

         if (dirty) {
            struct u_rect drawn = calc_drawn_area(s, layer);
            dirty->x0 = MIN2(drawn.x0, dirty->x0);
            dirty->y0 = MIN2(drawn.y0, dirty->y0);
            dirty->x1 = MAX2(drawn.x1, dirty->x1);
            dirty->y1 = MAX2(drawn.y1, dirty->y1);
         }
      }
   }
}

void
vl_compositor_cs_render(struct vl_compositor_state *s,
                        struct vl_compositor       *c,
                        struct pipe_surface        *dst_surface,
                        struct u_rect              *dirty_area,
                        bool                        clear_dirty)
{
   assert(c && s);
   assert(dst_surface);

   c->fb_state.width = dst_surface->width;
   c->fb_state.height = dst_surface->height;
   c->fb_state.cbufs[0] = dst_surface;

   if (!s->scissor_valid) {
      s->scissor.minx = 0;
      s->scissor.miny = 0;
      s->scissor.maxx = dst_surface->width;
      s->scissor.maxy = dst_surface->height;
   }

   if (clear_dirty && dirty_area &&
       (dirty_area->x0 < dirty_area->x1 || dirty_area->y0 < dirty_area->y1)) {

      c->pipe->clear_render_target(c->pipe, dst_surface, &s->clear_color,
                       0, 0, dst_surface->width, dst_surface->height, false);
      dirty_area->x0 = dirty_area->y0 = VL_COMPOSITOR_MAX_DIRTY;
      dirty_area->x1 = dirty_area->y1 = VL_COMPOSITOR_MIN_DIRTY;
   }

   pipe_set_constant_buffer(c->pipe, PIPE_SHADER_COMPUTE, 0, s->shader_params);

   draw_layers(c, s, dirty_area);
}

bool vl_compositor_cs_init_shaders(struct vl_compositor *c)
{
        assert(c);

        c->cs_video_buffer = create_video_buffer_shader(c);
        if (!c->cs_video_buffer) {
                debug_printf("Unable to create video_buffer compute shader.\n");
                return false;
        }

        c->cs_weave_rgb = create_weave_shader(c, true, false);
        if (!c->cs_weave_rgb) {
                debug_printf("Unable to create weave_rgb compute shader.\n");
                return false;
        }

        c->cs_yuv.weave.y = create_weave_shader(c, false, true);
        c->cs_yuv.weave.uv = create_weave_shader(c, false, false);
        c->cs_yuv.progressive.y = create_yuv_progressive_shader(c, true);
        c->cs_yuv.progressive.uv = create_yuv_progressive_shader(c, false);
        if (!c->cs_yuv.weave.y || !c->cs_yuv.weave.uv) {
                debug_printf("Unable to create YCbCr i-to-YCbCr p deint compute shader.\n");
                return false;
        }
        if (!c->cs_yuv.progressive.y || !c->cs_yuv.progressive.uv) {
                debug_printf("Unable to create YCbCr p-to-NV12 compute shader.\n");
                return false;
        }

        c->cs_rgb_yuv.y = create_rgb_yuv_shader(c, true);
        c->cs_rgb_yuv.uv = create_rgb_yuv_shader(c, false);
        if (!c->cs_rgb_yuv.y || !c->cs_rgb_yuv.uv) {
                debug_printf("Unable to create RGB-to-NV12 compute shader.\n");
                return false;
        }

        return true;
}

void vl_compositor_cs_cleanup_shaders(struct vl_compositor *c)
{
        assert(c);

        if (c->cs_video_buffer)
                c->pipe->delete_compute_state(c->pipe, c->cs_video_buffer);
        if (c->cs_weave_rgb)
                c->pipe->delete_compute_state(c->pipe, c->cs_weave_rgb);
        if (c->cs_yuv.weave.y)
                c->pipe->delete_compute_state(c->pipe, c->cs_yuv.weave.y);
        if (c->cs_yuv.weave.uv)
                c->pipe->delete_compute_state(c->pipe, c->cs_yuv.weave.uv);
        if (c->cs_yuv.progressive.y)
                c->pipe->delete_compute_state(c->pipe, c->cs_yuv.progressive.y);
        if (c->cs_yuv.progressive.uv)
                c->pipe->delete_compute_state(c->pipe, c->cs_yuv.progressive.uv);
        if (c->cs_rgb_yuv.y)
                c->pipe->delete_compute_state(c->pipe, c->cs_rgb_yuv.y);
        if (c->cs_rgb_yuv.uv)
                c->pipe->delete_compute_state(c->pipe, c->cs_rgb_yuv.uv);
}
