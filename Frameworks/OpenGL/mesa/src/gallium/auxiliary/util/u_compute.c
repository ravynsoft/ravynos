/**************************************************************************
 *
 * Copyright 2019 Sonny Jiang <sonnyj608@gmail.com>
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
 **************************************************************************/

#include "pipe/p_context.h"
#include "pipe/p_state.h"

#include "u_bitcast.h"
#include "util/format/u_format.h"
#include "u_sampler.h"
#include "nir/nir_builder.h"
#include "u_inlines.h"
#include "u_compute.h"

static void *blit_compute_shader(struct pipe_context *ctx)
{
   /*
      #version 450

      layout (local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
      layout (binding = 0) uniform sampler2DArray samp;
      layout (binding = 0, rgba32f) uniform writeonly image2D image;

      layout (std140, binding = 0) uniform ubo
      {
         vec4 src;
         vec4 scale;
         ivec4 dst;
         vec4 coord_max;
      };

      void main()
      {
         ivec3 pos = ivec3(gl_GlobalInvocationID.xyz);
         vec3 tex_pos = vec3(pos.x + 0.5, pos.y + 0.5, pos.z);
         tex_pos = tex_pos * scale.xyz + src.xyz;
         tex_pos.xy = min(tex_pos.xy, coord_max.xy);
         vec4 color = texture(samp, tex_pos);
         ivec2 image_pos = pos.xy + dst.xy;
         imageStore(image, image_pos, color);
      }
   */
   const struct glsl_type *sampler_type =
      glsl_sampler_type(GLSL_SAMPLER_DIM_2D, /*is_shadow*/ false, /*is_array*/ true, GLSL_TYPE_FLOAT);
   const struct glsl_type *image_type =
      glsl_image_type(GLSL_SAMPLER_DIM_2D, /*is_array*/ true, GLSL_TYPE_FLOAT);
   const nir_shader_compiler_options *options =
      ctx->screen->get_compiler_options(ctx->screen, PIPE_SHADER_IR_NIR, PIPE_SHADER_COMPUTE);

   nir_builder b = nir_builder_init_simple_shader(MESA_SHADER_COMPUTE, options, "blit_cs");
   b.shader->info.workgroup_size[0] = 64;
   b.shader->info.workgroup_size[1] = 1;
   b.shader->info.workgroup_size[2] = 1;
   b.shader->info.num_ubos = 1;

   nir_def *zero = nir_imm_int(&b, 0);
   nir_def *undef32 = nir_undef(&b, 1, 32);

   nir_def *params[4];
   b.shader->num_uniforms = ARRAY_SIZE(params);
   for (unsigned i = 0; i < b.shader->num_uniforms; ++i)
      params[i] = nir_load_ubo(&b, 4, 32, zero, nir_imm_int(&b, i * 16), .align_mul = 4, .range = ~0);

   nir_variable *sampler = nir_variable_create(b.shader, nir_var_uniform, sampler_type, "sampler");
   sampler->data.binding = 0;
   BITSET_SET(b.shader->info.textures_used, 0);
   BITSET_SET(b.shader->info.samplers_used, 0);

   nir_variable *image = nir_variable_create(b.shader, nir_var_image, image_type, "image");
   image->data.binding = 0;
   image->data.image.format = PIPE_FORMAT_R32G32B32A32_FLOAT;
   BITSET_SET(b.shader->info.images_used, 0);

   nir_def *block_ids = nir_load_workgroup_id(&b);
   nir_def *local_ids = nir_load_local_invocation_id(&b);
   nir_def *ids = nir_iadd(&b, nir_imul(&b, block_ids, nir_imm_ivec3(&b, 64, 1, 1)), local_ids);

   nir_def *tex_pos = nir_u2f32(&b, ids);
   tex_pos = nir_fadd(&b, tex_pos, nir_imm_vec3(&b, 0.5f, 0.5f, 0.0f));
   tex_pos = nir_ffma(&b, tex_pos, params[1], params[0]);
   nir_def *z = nir_channel(&b, tex_pos, 2);
   tex_pos = nir_fmin(&b, tex_pos, params[3]);
   tex_pos = nir_vector_insert_imm(&b, tex_pos, z, 2);
   tex_pos = nir_channels(&b, tex_pos, 0x7);

   nir_deref_instr *tex_deref = nir_build_deref_var(&b, sampler);
   nir_def *color = nir_tex_deref(&b, tex_deref, tex_deref, tex_pos);

   nir_def *image_pos = nir_pad_vector_imm_int(&b, ids, 0, 4);
   image_pos = nir_iadd(&b, image_pos, params[2]);
   nir_image_deref_store(&b, &nir_build_deref_var(&b, image)->def, image_pos, undef32, color, zero);

   ctx->screen->finalize_nir(ctx->screen, b.shader);

   struct pipe_compute_state state = {0};
   state.ir_type = PIPE_SHADER_IR_NIR;
   state.prog = b.shader;

   return ctx->create_compute_state(ctx, &state);
}

void util_compute_blit(struct pipe_context *ctx, struct pipe_blit_info *blit_info,
                       void **compute_state)
{
   if (blit_info->src.box.width == 0 || blit_info->src.box.height == 0 ||
       blit_info->dst.box.width == 0 || blit_info->dst.box.height == 0)
     return;

   struct pipe_resource *src = blit_info->src.resource;
   struct pipe_resource *dst = blit_info->dst.resource;
   struct pipe_sampler_view src_templ = {0}, *src_view;
   void *sampler_state_p;
   unsigned width = blit_info->dst.box.width;
   unsigned height = blit_info->dst.box.height;
   float x_scale = blit_info->src.box.width / (float)blit_info->dst.box.width;
   float y_scale = blit_info->src.box.height / (float)blit_info->dst.box.height;
   float z_scale = blit_info->src.box.depth / (float)blit_info->dst.box.depth;

   unsigned data[] = {u_bitcast_f2u(blit_info->src.box.x / (float)src->width0),
                      u_bitcast_f2u(blit_info->src.box.y / (float)src->height0),
                      u_bitcast_f2u(blit_info->src.box.z),
                      u_bitcast_f2u(0),
                      u_bitcast_f2u(x_scale / src->width0),
                      u_bitcast_f2u(y_scale / src->height0),
                      u_bitcast_f2u(z_scale),
                      u_bitcast_f2u(0),
                      blit_info->dst.box.x,
                      blit_info->dst.box.y,
                      blit_info->dst.box.z,
                      0,
                      u_bitcast_f2u((blit_info->src.box.x + blit_info->src.box.width - 0.5) /
                                    (float)src->width0),
                      u_bitcast_f2u((blit_info->src.box.y + blit_info->src.box.height - 0.5) /
                                    (float)src->height0),
                      0,
                      0};

   struct pipe_constant_buffer cb = {0};
   cb.buffer_size = sizeof(data);
   cb.user_buffer = data;
   ctx->set_constant_buffer(ctx, PIPE_SHADER_COMPUTE, 0, false, &cb);

   struct pipe_image_view image = {0};
   image.resource = dst;
   image.shader_access = image.access = PIPE_IMAGE_ACCESS_WRITE;
   image.format = util_format_linear(blit_info->dst.format);
   image.u.tex.level = blit_info->dst.level;
   image.u.tex.first_layer = 0;
   image.u.tex.last_layer = (unsigned)(dst->array_size - 1);

   ctx->set_shader_images(ctx, PIPE_SHADER_COMPUTE, 0, 1, 0, &image);

   struct pipe_sampler_state sampler_state={0};
   sampler_state.wrap_s = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
   sampler_state.wrap_t = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
   sampler_state.wrap_r = PIPE_TEX_WRAP_CLAMP_TO_EDGE;

   if (blit_info->filter == PIPE_TEX_FILTER_LINEAR) {
      sampler_state.min_img_filter = PIPE_TEX_FILTER_LINEAR;
      sampler_state.mag_img_filter = PIPE_TEX_FILTER_LINEAR;
   }

   sampler_state_p = ctx->create_sampler_state(ctx, &sampler_state);
   ctx->bind_sampler_states(ctx, PIPE_SHADER_COMPUTE, 0, 1, &sampler_state_p);

   /* Initialize the sampler view. */
   u_sampler_view_default_template(&src_templ, src, src->format);
   src_templ.format = util_format_linear(blit_info->src.format);
   src_view = ctx->create_sampler_view(ctx, src, &src_templ);
   ctx->set_sampler_views(ctx, PIPE_SHADER_COMPUTE, 0, 1, 0, false, &src_view);

   if (!*compute_state)
     *compute_state = blit_compute_shader(ctx);
   ctx->bind_compute_state(ctx, *compute_state);

   struct pipe_grid_info grid_info = {0};
   grid_info.block[0] = 64;
   grid_info.last_block[0] = width % 64;
   grid_info.block[1] = 1;
   grid_info.block[2] = 1;
   grid_info.grid[0] = DIV_ROUND_UP(width, 64);
   grid_info.grid[1] = height;
   grid_info.grid[2] = 1;

   ctx->launch_grid(ctx, &grid_info);

   ctx->memory_barrier(ctx, PIPE_BARRIER_ALL);

   ctx->set_shader_images(ctx, PIPE_SHADER_COMPUTE, 0, 0, 1, NULL);
   ctx->set_constant_buffer(ctx, PIPE_SHADER_COMPUTE, 0, false, NULL);
   ctx->set_sampler_views(ctx, PIPE_SHADER_COMPUTE, 0, 0, 1, false, NULL);
   pipe_sampler_view_reference(&src_view, NULL);
   ctx->delete_sampler_state(ctx, sampler_state_p);
   ctx->bind_compute_state(ctx, NULL);
}
