/**************************************************************************
 *
 * Copyright © 2022 Intel Corporation
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#include "compiler/glsl/astc_glsl.h"
#include "compiler/glsl/bc1_glsl.h"
#include "compiler/glsl/bc4_glsl.h"
#include "compiler/glsl/cross_platform_settings_piece_all.h"
#include "compiler/glsl/etc2_rgba_stitch_glsl.h"

#include "main/context.h"
#include "main/shaderapi.h"
#include "main/shaderobj.h"
#include "main/texcompress_astc.h"
#include "util/texcompress_astc_luts_wrap.h"
#include "main/uniforms.h"

#include "state_tracker/st_atom_constbuf.h"
#include "state_tracker/st_bc1_tables.h"
#include "state_tracker/st_context.h"
#include "state_tracker/st_program.h"
#include "state_tracker/st_texcompress_compute.h"
#include "state_tracker/st_texture.h"

#include "util/u_hash_table.h"
#include "util/u_string.h"

enum compute_program_id {
   COMPUTE_PROGRAM_BC1,
   COMPUTE_PROGRAM_BC4,
   COMPUTE_PROGRAM_STITCH,
   COMPUTE_PROGRAM_ASTC_4x4,
   COMPUTE_PROGRAM_ASTC_5x4,
   COMPUTE_PROGRAM_ASTC_5x5,
   COMPUTE_PROGRAM_ASTC_6x5,
   COMPUTE_PROGRAM_ASTC_6x6,
   COMPUTE_PROGRAM_ASTC_8x5,
   COMPUTE_PROGRAM_ASTC_8x6,
   COMPUTE_PROGRAM_ASTC_8x8,
   COMPUTE_PROGRAM_ASTC_10x5,
   COMPUTE_PROGRAM_ASTC_10x6,
   COMPUTE_PROGRAM_ASTC_10x8,
   COMPUTE_PROGRAM_ASTC_10x10,
   COMPUTE_PROGRAM_ASTC_12x10,
   COMPUTE_PROGRAM_ASTC_12x12,
   COMPUTE_PROGRAM_COUNT
};

static struct gl_program * PRINTFLIKE(3, 4)
get_compute_program(struct st_context *st,
                    enum compute_program_id prog_id,
                    const char *source_fmt, ...)
{
   /* Try to get the program from the cache. */
   assert(prog_id < COMPUTE_PROGRAM_COUNT);
   if (st->texcompress_compute.progs[prog_id])
      return st->texcompress_compute.progs[prog_id];

   /* Cache miss. Create the final source string. */
   char *source_str;
   va_list ap;
   va_start(ap, source_fmt);
   int num_printed_bytes = vasprintf(&source_str, source_fmt, ap);
   va_end(ap);
   if (num_printed_bytes == -1)
      return NULL;

   /* Compile and link the shader. Then, destroy the shader string. */
   const char *strings[] = { source_str };
   GLuint program =
      _mesa_CreateShaderProgramv_impl(st->ctx, GL_COMPUTE_SHADER, 1, strings);
   free(source_str);

   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program(st->ctx, program);
   if (!shProg)
      return NULL;

   if (shProg->data->LinkStatus == LINKING_FAILURE) {
      fprintf(stderr, "Linking failed:\n%s\n", shProg->data->InfoLog);
      _mesa_reference_shader_program(st->ctx, &shProg, NULL);
      return NULL;
   }

   /* Cache the program and return it. */
   return st->texcompress_compute.progs[prog_id] =
          shProg->_LinkedShaders[MESA_SHADER_COMPUTE]->Program;
}

static struct pipe_resource *
create_bc1_endpoint_ssbo(struct pipe_context *pipe)
{
   struct pipe_resource *buffer =
      pipe_buffer_create(pipe->screen, PIPE_BIND_SHADER_BUFFER,
                         PIPE_USAGE_IMMUTABLE, sizeof(float) *
                         (sizeof(stb__OMatch5) + sizeof(stb__OMatch6)));

   if (!buffer)
      return NULL;

   struct pipe_transfer *transfer;
   float (*buffer_map)[2] = pipe_buffer_map(pipe, buffer,
                                            PIPE_MAP_WRITE |
                                            PIPE_MAP_DISCARD_WHOLE_RESOURCE,
                                            &transfer);
   if (!buffer_map) {
      pipe_resource_reference(&buffer, NULL);
      return NULL;
   }

   for (int i = 0; i < 256; i++) {
      for (int j = 0; j < 2; j++) {
         buffer_map[i][j] = (float) stb__OMatch5[i][j];
         buffer_map[i + 256][j] = (float) stb__OMatch6[i][j];
      }
   }

   pipe_buffer_unmap(pipe, transfer);

   return buffer;
}

static void
bind_compute_state(struct st_context *st,
                   struct gl_program *prog,
                   struct pipe_sampler_view **sampler_views,
                   const struct pipe_shader_buffer *shader_buffers,
                   const struct pipe_image_view *image_views,
                   bool cs_handle_from_prog,
                   bool constbuf0_from_prog)
{
   assert(prog->info.stage == PIPE_SHADER_COMPUTE);

   /* Set compute states in the same order as defined in st_atom_list.h */

   assert(prog->affected_states & ST_NEW_CS_STATE);
   assert(st->shader_has_one_variant[PIPE_SHADER_COMPUTE]);
   cso_set_compute_shader_handle(st->cso_context,
                                 cs_handle_from_prog ?
                                 prog->variants->driver_shader : NULL);

   if (prog->affected_states & ST_NEW_CS_SAMPLER_VIEWS) {
      st->pipe->set_sampler_views(st->pipe, prog->info.stage, 0,
                                  prog->info.num_textures, 0, false,
                                  sampler_views);
   }

   if (prog->affected_states & ST_NEW_CS_SAMPLERS) {
      /* Programs seem to set this bit more often than needed. For example, if
       * a program only uses texelFetch, this shouldn't be needed. Section
       * "11.1.3.2 Texel Fetches", of the GL 4.6 spec says:
       *
       *    Texel fetch proceeds similarly to the steps described for texture
       *    access in section 11.1.3.5, with the exception that none of the
       *    operations controlled by sampler object state are performed,
       *
       * We assume that the program is using texelFetch or doesn't care about
       * this state for a similar reason.
       *
       * See https://gitlab.freedesktop.org/mesa/mesa/-/issues/8014.
       */
   }

   if (prog->affected_states & ST_NEW_CS_CONSTANTS) {
      st_upload_constants(st, constbuf0_from_prog ? prog : NULL,
                          prog->info.stage);
   }

   if (prog->affected_states & ST_NEW_CS_UBOS) {
      unreachable("Uniform buffer objects not handled");
   }

   if (prog->affected_states & ST_NEW_CS_ATOMICS) {
      unreachable("Atomic buffer objects not handled");
   }

   if (prog->affected_states & ST_NEW_CS_SSBOS) {
      st->pipe->set_shader_buffers(st->pipe, prog->info.stage, 0,
                                   prog->info.num_ssbos, shader_buffers,
                                   prog->sh.ShaderStorageBlocksWriteAccess);
   }

   if (prog->affected_states & ST_NEW_CS_IMAGES) {
      st->pipe->set_shader_images(st->pipe, prog->info.stage, 0,
                                  prog->info.num_images, 0, image_views);
   }
}

static void
dispatch_compute_state(struct st_context *st,
                       struct gl_program *prog,
                       struct pipe_sampler_view **sampler_views,
                       const struct pipe_shader_buffer *shader_buffers,
                       const struct pipe_image_view *image_views,
                       unsigned num_workgroups_x,
                       unsigned num_workgroups_y,
                       unsigned num_workgroups_z)
{
   assert(prog->info.stage == PIPE_SHADER_COMPUTE);

   /* Bind the state */
   bind_compute_state(st, prog, sampler_views, shader_buffers, image_views,
                      true, true);

   /* Launch the grid */
   const struct pipe_grid_info info = {
      .block[0] = prog->info.workgroup_size[0],
      .block[1] = prog->info.workgroup_size[1],
      .block[2] = prog->info.workgroup_size[2],
      .grid[0] = num_workgroups_x,
      .grid[1] = num_workgroups_y,
      .grid[2] = num_workgroups_z,
   };

   st->pipe->launch_grid(st->pipe, &info);

   /* Unbind the state */
   bind_compute_state(st, prog, NULL, NULL, NULL, false, false);

   /* If the previously used compute program was relying on any state that was
    * trampled on by these state changes, dirty the relevant flags.
    */
   if (st->cp) {
      st->ctx->NewDriverState |=
         st->cp->affected_states & prog->affected_states;
   }
}

static struct pipe_resource *
cs_encode_bc1(struct st_context *st,
              struct pipe_resource *rgba8_tex)
{
   /* Create the required compute state */
   struct gl_program *prog =
      get_compute_program(st, COMPUTE_PROGRAM_BC1, bc1_source,
                          cross_platform_settings_piece_all_header);
   if (!prog)
      return NULL;

   /* ... complete the program setup by defining the number of refinements to
    * do on the created blocks. The program will attempt to create a more
    * accurate encoding on each iteration. Doing at least one refinement
    * provides a significant improvement in quality and is needed to give a
    * result comparable to the CPU encoder (according to piglit tests).
    * Additional refinements don't help as much.
    */
   const unsigned num_refinements = 1;
   _mesa_uniform(0, 1, &num_refinements, st->ctx, prog->shader_program,
                 GLSL_TYPE_UINT, 1);

   const struct pipe_sampler_view templ = {
      .target = PIPE_TEXTURE_2D,
      .format = PIPE_FORMAT_R8G8B8A8_UNORM,
      .swizzle_r = PIPE_SWIZZLE_X,
      .swizzle_g = PIPE_SWIZZLE_Y,
      .swizzle_b = PIPE_SWIZZLE_Z,
      .swizzle_a = PIPE_SWIZZLE_W,
   };
   struct pipe_sampler_view *rgba8_view =
      st->pipe->create_sampler_view(st->pipe, rgba8_tex, &templ);
   if (!rgba8_view)
      return NULL;

   const struct pipe_shader_buffer ssbo = {
      .buffer = st->texcompress_compute.bc1_endpoint_buf,
      .buffer_size = st->texcompress_compute.bc1_endpoint_buf->width0,
   };

   struct pipe_resource *bc1_tex =
      st_texture_create(st, PIPE_TEXTURE_2D, PIPE_FORMAT_R32G32_UINT, 0,
                        DIV_ROUND_UP(rgba8_tex->width0, 4),
                        DIV_ROUND_UP(rgba8_tex->height0, 4), 1, 1, 0,
                        PIPE_BIND_SHADER_IMAGE |
                        PIPE_BIND_SAMPLER_VIEW, false);
   if (!bc1_tex)
      goto release_sampler_views;

   const struct pipe_image_view image = {
      .resource = bc1_tex,
      .format = PIPE_FORMAT_R16G16B16A16_UINT,
      .access = PIPE_IMAGE_ACCESS_WRITE,
      .shader_access = PIPE_IMAGE_ACCESS_WRITE,
   };

   /* Dispatch the compute state */
   dispatch_compute_state(st, prog, &rgba8_view, &ssbo, &image,
                          DIV_ROUND_UP(rgba8_tex->width0, 32),
                          DIV_ROUND_UP(rgba8_tex->height0, 32), 1);

release_sampler_views:
   pipe_sampler_view_reference(&rgba8_view, NULL);

   return bc1_tex;
}

static struct pipe_resource *
cs_encode_bc4(struct st_context *st,
              struct pipe_resource *rgba8_tex,
              enum pipe_swizzle component, bool use_snorm)
{
   /* Create the required compute state */
   struct gl_program *prog =
      get_compute_program(st, COMPUTE_PROGRAM_BC4, bc4_source,
                          cross_platform_settings_piece_all_header);
   if (!prog)
      return NULL;

   /* ... complete the program setup by picking the channel to encode and
    * whether to encode it as snorm. The shader doesn't actually support
    * channel index 2. So, pick index 0 and rely on swizzling instead.
    */
   const unsigned params[] = { 0, use_snorm };
   _mesa_uniform(0, 1, params, st->ctx, prog->shader_program,
                 GLSL_TYPE_UINT, 2);

   const struct pipe_sampler_view templ = {
      .target = PIPE_TEXTURE_2D,
      .format = PIPE_FORMAT_R8G8B8A8_UNORM,
      .swizzle_r = component,
      .swizzle_g = PIPE_SWIZZLE_0,
      .swizzle_b = PIPE_SWIZZLE_0,
      .swizzle_a = PIPE_SWIZZLE_1,
   };
   struct pipe_sampler_view *rgba8_view =
      st->pipe->create_sampler_view(st->pipe, rgba8_tex, &templ);
   if (!rgba8_view)
      return NULL;

   struct pipe_resource *bc4_tex =
      st_texture_create(st, PIPE_TEXTURE_2D, PIPE_FORMAT_R32G32_UINT, 0,
                        DIV_ROUND_UP(rgba8_tex->width0, 4),
                        DIV_ROUND_UP(rgba8_tex->height0, 4), 1, 1, 0,
                        PIPE_BIND_SHADER_IMAGE |
                        PIPE_BIND_SAMPLER_VIEW, false);
   if (!bc4_tex)
      goto release_sampler_views;

   const struct pipe_image_view image = {
      .resource = bc4_tex,
      .format = PIPE_FORMAT_R16G16B16A16_UINT,
      .access = PIPE_IMAGE_ACCESS_WRITE,
      .shader_access = PIPE_IMAGE_ACCESS_WRITE,
   };

   /* Dispatch the compute state */
   dispatch_compute_state(st, prog, &rgba8_view, NULL, &image, 1,
                          DIV_ROUND_UP(rgba8_tex->width0, 16),
                          DIV_ROUND_UP(rgba8_tex->height0, 16));

release_sampler_views:
   pipe_sampler_view_reference(&rgba8_view, NULL);

   return bc4_tex;
}

static struct pipe_resource *
cs_stitch_64bpb_textures(struct st_context *st,
                         struct pipe_resource *tex_hi,
                         struct pipe_resource *tex_lo)
{
   assert(util_format_get_blocksizebits(tex_hi->format) == 64);
   assert(util_format_get_blocksizebits(tex_lo->format) == 64);
   assert(tex_hi->width0 == tex_lo->width0);
   assert(tex_hi->height0 == tex_lo->height0);

   struct pipe_resource *stitched_tex = NULL;

   /* Create the required compute state */
   struct gl_program *prog =
      get_compute_program(st, COMPUTE_PROGRAM_STITCH, etc2_rgba_stitch_source,
                          cross_platform_settings_piece_all_header);
   if (!prog)
      return NULL;

   const struct pipe_sampler_view templ = {
      .target = PIPE_TEXTURE_2D,
      .format = PIPE_FORMAT_R32G32_UINT,
      .swizzle_r = PIPE_SWIZZLE_X,
      .swizzle_g = PIPE_SWIZZLE_Y,
      .swizzle_b = PIPE_SWIZZLE_0,
      .swizzle_a = PIPE_SWIZZLE_1,
   };
   struct pipe_sampler_view *rg32_views[2] = {
      [0] = st->pipe->create_sampler_view(st->pipe, tex_hi, &templ),
      [1] = st->pipe->create_sampler_view(st->pipe, tex_lo, &templ),
   };
   if (!rg32_views[0] || !rg32_views[1])
      goto release_sampler_views;

   stitched_tex =
      st_texture_create(st, PIPE_TEXTURE_2D, PIPE_FORMAT_R32G32B32A32_UINT, 0,
                        tex_hi->width0,
                        tex_hi->height0, 1, 1, 0,
                        PIPE_BIND_SHADER_IMAGE |
                        PIPE_BIND_SAMPLER_VIEW, false);
   if (!stitched_tex)
      goto release_sampler_views;

   const struct pipe_image_view image = {
      .resource = stitched_tex,
      .format = PIPE_FORMAT_R32G32B32A32_UINT,
      .access = PIPE_IMAGE_ACCESS_WRITE,
      .shader_access = PIPE_IMAGE_ACCESS_WRITE,
   };

   /* Dispatch the compute state */
   dispatch_compute_state(st, prog, rg32_views, NULL, &image,
                          DIV_ROUND_UP(tex_hi->width0, 8),
                          DIV_ROUND_UP(tex_hi->height0, 8), 1);

release_sampler_views:
   pipe_sampler_view_reference(&rg32_views[0], NULL);
   pipe_sampler_view_reference(&rg32_views[1], NULL);

   return stitched_tex;
}

static struct pipe_resource *
cs_encode_bc3(struct st_context *st,
              struct pipe_resource *rgba8_tex)
{
   struct pipe_resource *bc3_tex = NULL;

   /* Encode RGB channels as BC1. */
   struct pipe_resource *bc1_tex = cs_encode_bc1(st, rgba8_tex);
   if (!bc1_tex)
      return NULL;

   /* Encode alpha channels as BC4. */
   struct pipe_resource *bc4_tex =
      cs_encode_bc4(st, rgba8_tex, PIPE_SWIZZLE_W, false);
   if (!bc4_tex)
      goto release_textures;

   st->pipe->memory_barrier(st->pipe, PIPE_BARRIER_TEXTURE);

   /* Combine BC1 and BC4 to create BC3. */
   bc3_tex = cs_stitch_64bpb_textures(st, bc1_tex, bc4_tex);
   if (!bc3_tex)
      goto release_textures;

release_textures:
   pipe_resource_reference(&bc1_tex, NULL);
   pipe_resource_reference(&bc4_tex, NULL);

   return bc3_tex;
}

static struct pipe_resource *
sw_decode_astc(struct st_context *st,
               uint8_t *astc_data,
               unsigned astc_stride,
               mesa_format astc_format,
               unsigned width_px, unsigned height_px)
{
   /* Create the destination */
   struct pipe_resource *rgba8_tex =
      st_texture_create(st, PIPE_TEXTURE_2D, PIPE_FORMAT_R8G8B8A8_UNORM, 0,
                        width_px, height_px, 1, 1, 0,
                        PIPE_BIND_SAMPLER_VIEW, false);
   if (!rgba8_tex)
      return NULL;

   /* Temporarily map the destination and decode into the returned pointer */
   struct pipe_transfer *rgba8_xfer;
   void *rgba8_map = pipe_texture_map(st->pipe, rgba8_tex, 0, 0,
                                      PIPE_MAP_WRITE, 0, 0,
                                      width_px, height_px, &rgba8_xfer);
   if (!rgba8_map) {
      pipe_resource_reference(&rgba8_tex, NULL);
      return NULL;
   }

   _mesa_unpack_astc_2d_ldr(rgba8_map, rgba8_xfer->stride,
                            astc_data, astc_stride,
                            width_px, height_px, astc_format);

   pipe_texture_unmap(st->pipe, rgba8_xfer);

   return rgba8_tex;
}

static struct pipe_sampler_view *
create_astc_cs_payload_view(struct st_context *st,
                            uint8_t *data, unsigned stride,
                            uint32_t width_el, uint32_t height_el)
{
   const struct pipe_resource src_templ = {
      .target = PIPE_TEXTURE_2D,
      .format = PIPE_FORMAT_R32G32B32A32_UINT,
      .bind = PIPE_BIND_SAMPLER_VIEW,
      .usage = PIPE_USAGE_STAGING,
      .width0 = width_el,
      .height0 = height_el,
      .depth0 = 1,
      .array_size = 1,
   };

   struct pipe_resource *payload_res =
      st->screen->resource_create(st->screen, &src_templ);

   if (!payload_res)
      return NULL;

   struct pipe_box box;
   u_box_origin_2d(width_el, height_el, &box);

   st->pipe->texture_subdata(st->pipe, payload_res, 0, 0,
                             &box,
                             data,
                             stride,
                             0 /* unused */);

   const struct pipe_sampler_view view_templ = {
      .target = PIPE_TEXTURE_2D,
      .format = payload_res->format,
      .swizzle_r = PIPE_SWIZZLE_X,
      .swizzle_g = PIPE_SWIZZLE_Y,
      .swizzle_b = PIPE_SWIZZLE_Z,
      .swizzle_a = PIPE_SWIZZLE_W,
   };

   struct pipe_sampler_view *view =
      st->pipe->create_sampler_view(st->pipe, payload_res, &view_templ);

   pipe_resource_reference(&payload_res, NULL);

   return view;
}

static struct pipe_sampler_view *
get_astc_partition_table_view(struct st_context *st,
                              unsigned block_w,
                              unsigned block_h)
{
   unsigned lut_width;
   unsigned lut_height;
   struct pipe_box ptable_box;
   void *ptable_data =
      _mesa_get_astc_decoder_partition_table(block_w, block_h, &lut_width, &lut_height);
   u_box_origin_2d(lut_width, lut_height, &ptable_box);

   struct pipe_sampler_view *view =
      util_hash_table_get(st->texcompress_compute.astc_partition_tables,
                          ptable_data);

   if (view)
      return view;

   struct pipe_resource *res =
      st_texture_create(st, PIPE_TEXTURE_2D, PIPE_FORMAT_R8_UINT, 0,
                        ptable_box.width, ptable_box.height,
                        1, 1, 0,
                        PIPE_BIND_SAMPLER_VIEW, false);
   if (!res)
      return NULL;

   st->pipe->texture_subdata(st->pipe, res, 0, 0,
                             &ptable_box,
                             ptable_data,
                             ptable_box.width,
                             0 /* unused */);

   const struct pipe_sampler_view templ = {
      .target = PIPE_TEXTURE_2D,
      .format = res->format,
      .swizzle_r = PIPE_SWIZZLE_X,
      .swizzle_g = PIPE_SWIZZLE_Y,
      .swizzle_b = PIPE_SWIZZLE_Z,
      .swizzle_a = PIPE_SWIZZLE_W,
   };

   view = st->pipe->create_sampler_view(st->pipe, res, &templ);

   pipe_resource_reference(&res, NULL);

   if (view) {
      _mesa_hash_table_insert(st->texcompress_compute.astc_partition_tables,
                              ptable_data, view);
      ASSERTED const unsigned max_entries =
         COMPUTE_PROGRAM_ASTC_12x12 - COMPUTE_PROGRAM_ASTC_4x4 + 1;
      assert(_mesa_hash_table_num_entries(
         st->texcompress_compute.astc_partition_tables) < max_entries);
   }

   return view;
}

static struct pipe_resource *
cs_decode_astc(struct st_context *st,
               uint8_t *astc_data,
               unsigned astc_stride,
               mesa_format astc_format,
               unsigned width_px, unsigned height_px)
{
   const enum compute_program_id astc_id = COMPUTE_PROGRAM_ASTC_4x4 +
      util_format_linear(astc_format) - PIPE_FORMAT_ASTC_4x4;

   unsigned block_w, block_h;
   _mesa_get_format_block_size(astc_format, &block_w, &block_h);

   struct gl_program *prog =
      get_compute_program(st, astc_id, astc_source, block_w, block_h);

   if (!prog)
      return NULL;

   struct pipe_sampler_view *ptable_view =
      get_astc_partition_table_view(st, block_w, block_h);

   if (!ptable_view)
      return NULL;

   struct pipe_sampler_view *payload_view =
      create_astc_cs_payload_view(st, astc_data, astc_stride,
                                  DIV_ROUND_UP(width_px, block_w),
                                  DIV_ROUND_UP(height_px, block_h));

   if (!payload_view)
      return NULL;

   /* Create the destination */
   struct pipe_resource *rgba8_tex =
      st_texture_create(st, PIPE_TEXTURE_2D, PIPE_FORMAT_R8G8B8A8_UNORM, 0,
                        width_px, height_px, 1, 1, 0,
                        PIPE_BIND_SAMPLER_VIEW, false);

   if (!rgba8_tex)
      goto release_payload_view;

   const struct pipe_image_view image = {
      .resource = rgba8_tex,
      .format = PIPE_FORMAT_R8G8B8A8_UINT,
      .access = PIPE_IMAGE_ACCESS_WRITE,
      .shader_access = PIPE_IMAGE_ACCESS_WRITE,
   };

   struct pipe_sampler_view *sampler_views[] = {
      st->texcompress_compute.astc_luts[0],
      st->texcompress_compute.astc_luts[1],
      st->texcompress_compute.astc_luts[2],
      st->texcompress_compute.astc_luts[3],
      st->texcompress_compute.astc_luts[4],
      ptable_view,
      payload_view,
   };

   dispatch_compute_state(st, prog, sampler_views, NULL, &image,
                          DIV_ROUND_UP(payload_view->texture->width0, 2),
                          DIV_ROUND_UP(payload_view->texture->height0, 2),
                          1);

release_payload_view:
   pipe_sampler_view_reference(&payload_view, NULL);

   return rgba8_tex;
}

static struct pipe_sampler_view *
get_sampler_view_for_lut(struct pipe_context *pipe,
                         const astc_decoder_lut *lut)
{
   struct pipe_resource *res =
      pipe_buffer_create_with_data(pipe,
                                   PIPE_BIND_SAMPLER_VIEW,
                                   PIPE_USAGE_DEFAULT,
                                   lut->size_B,
                                   lut->data);
   if (!res)
      return NULL;

   const struct pipe_sampler_view templ = {
      .format = lut->format,
      .target = PIPE_BUFFER,
      .swizzle_r = PIPE_SWIZZLE_X,
      .swizzle_g = PIPE_SWIZZLE_Y,
      .swizzle_b = PIPE_SWIZZLE_Z,
      .swizzle_a = PIPE_SWIZZLE_W,
      .u.buf.offset = 0,
      .u.buf.size = lut->size_B,
   };

   struct pipe_sampler_view *view =
      pipe->create_sampler_view(pipe, res, &templ);

   pipe_resource_reference(&res, NULL);

   return view;
}

/* Initializes required resources for Granite ASTC GPU decode.
 *
 * There are 5 texture buffer objects and one additional texture required.
 * We initialize 5 tbo's here and a single texture later during runtime.
 */
static bool
initialize_astc_decoder(struct st_context *st)
{
   astc_decoder_lut_holder astc_lut_holder;
   _mesa_init_astc_decoder_luts(&astc_lut_holder);

   const astc_decoder_lut *luts[] = {
      &astc_lut_holder.color_endpoint,
      &astc_lut_holder.color_endpoint_unquant,
      &astc_lut_holder.weights,
      &astc_lut_holder.weights_unquant,
      &astc_lut_holder.trits_quints,
   };

   for (unsigned i = 0; i < ARRAY_SIZE(luts); i++) {
      st->texcompress_compute.astc_luts[i] =
         get_sampler_view_for_lut(st->pipe, luts[i]);
      if (!st->texcompress_compute.astc_luts[i])
         return false;
   }

   st->texcompress_compute.astc_partition_tables =
      _mesa_pointer_hash_table_create(NULL);

   if (!st->texcompress_compute.astc_partition_tables)
      return false;

   return true;
}

bool
st_init_texcompress_compute(struct st_context *st)
{
   st->texcompress_compute.progs =
      calloc(COMPUTE_PROGRAM_COUNT, sizeof(struct gl_program *));
   if (!st->texcompress_compute.progs)
      return false;

   st->texcompress_compute.bc1_endpoint_buf =
      create_bc1_endpoint_ssbo(st->pipe);
   if (!st->texcompress_compute.bc1_endpoint_buf)
      return false;

   if (!initialize_astc_decoder(st))
      return false;

   return true;
}

static void
destroy_astc_decoder(struct st_context *st)
{
   for (unsigned i = 0; i < ARRAY_SIZE(st->texcompress_compute.astc_luts); i++)
      pipe_sampler_view_reference(&st->texcompress_compute.astc_luts[i], NULL);

   if (st->texcompress_compute.astc_partition_tables) {
      hash_table_foreach(st->texcompress_compute.astc_partition_tables,
                         entry) {
         pipe_sampler_view_reference(
            (struct pipe_sampler_view **)&entry->data, NULL);
      }
   }

   _mesa_hash_table_destroy(st->texcompress_compute.astc_partition_tables,
                            NULL);
}

void
st_destroy_texcompress_compute(struct st_context *st)
{
   /* The programs in the array are part of the gl_context (in st->ctx).They
    * are automatically destroyed when the context is destroyed (via
    * _mesa_free_context_data -> ... -> free_shader_program_data_cb).
    */
   free(st->texcompress_compute.progs);

   /* Destroy the SSBO used by the BC1 shader program. */
   pipe_resource_reference(&st->texcompress_compute.bc1_endpoint_buf, NULL);

   destroy_astc_decoder(st);
}

/* See st_texcompress_compute.h for more information. */
bool
st_compute_transcode_astc_to_dxt5(struct st_context *st,
                                  uint8_t *astc_data,
                                  unsigned astc_stride,
                                  mesa_format astc_format,
                                  struct pipe_resource *dxt5_tex,
                                  unsigned dxt5_level,
                                  unsigned dxt5_layer)
{
   assert(_mesa_has_compute_shaders(st->ctx));
   assert(_mesa_is_format_astc_2d(astc_format));
   assert(dxt5_tex->format == PIPE_FORMAT_DXT5_RGBA ||
          dxt5_tex->format == PIPE_FORMAT_DXT5_SRGBA);
   assert(dxt5_level <= dxt5_tex->last_level);
   assert(dxt5_layer <= util_max_layer(dxt5_tex, dxt5_level));

   bool success = false;

   /* Decode ASTC to RGBA8. */
   struct pipe_resource *rgba8_tex =
      cs_decode_astc(st, astc_data, astc_stride, astc_format,
                     u_minify(dxt5_tex->width0, dxt5_level),
                     u_minify(dxt5_tex->height0, dxt5_level));
   if (!rgba8_tex)
      return false;

   st->pipe->memory_barrier(st->pipe, PIPE_BARRIER_TEXTURE);

   /* Encode RGBA8 to BC3. */
   struct pipe_resource *bc3_tex = cs_encode_bc3(st, rgba8_tex);
   if (!bc3_tex)
      goto release_textures;

   /* Upload the result. */
   struct pipe_box src_box;
   u_box_origin_2d(bc3_tex->width0, bc3_tex->height0, &src_box);
   st->pipe->resource_copy_region(st->pipe, dxt5_tex, dxt5_level,
                                  0, 0, dxt5_layer, bc3_tex, 0, &src_box);

   success = true;

release_textures:
   pipe_resource_reference(&rgba8_tex, NULL);
   pipe_resource_reference(&bc3_tex, NULL);

   return success;
}
