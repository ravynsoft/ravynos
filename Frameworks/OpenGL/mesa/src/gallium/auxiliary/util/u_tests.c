/**************************************************************************
 *
 * Copyright 2014 Advanced Micro Devices, Inc.
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
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#include "util/u_tests.h"

#include "util/u_draw_quad.h"
#include "util/format/u_format.h"
#include "util/u_inlines.h"
#include "util/u_memory.h"
#include "util/u_simple_shaders.h"
#include "util/u_surface.h"
#include "util/u_string.h"
#include "util/u_tile.h"
#include "tgsi/tgsi_strings.h"
#include "tgsi/tgsi_text.h"
#include "cso_cache/cso_context.h"
#include "frontend/winsys_handle.h"
#include <stdio.h>

#define TOLERANCE 0.01

static struct pipe_resource *
util_create_texture2d(struct pipe_screen *screen, unsigned width,
                      unsigned height, enum pipe_format format,
                      unsigned num_samples)
{
   struct pipe_resource templ = {0};

   templ.target = PIPE_TEXTURE_2D;
   templ.width0 = width;
   templ.height0 = height;
   templ.depth0 = 1;
   templ.array_size = 1;
   templ.nr_samples = num_samples;
   templ.nr_storage_samples = num_samples;
   templ.format = format;
   templ.usage = PIPE_USAGE_DEFAULT;
   templ.bind = PIPE_BIND_SAMPLER_VIEW |
                (util_format_is_depth_or_stencil(format) ?
                    PIPE_BIND_DEPTH_STENCIL : PIPE_BIND_RENDER_TARGET);

   return screen->resource_create(screen, &templ);
}

static void
util_set_framebuffer_cb0(struct cso_context *cso, struct pipe_context *ctx,
			 struct pipe_resource *tex)
{
   struct pipe_surface templ = {{0}}, *surf;
   struct pipe_framebuffer_state fb = {0};

   templ.format = tex->format;
   surf = ctx->create_surface(ctx, tex, &templ);

   fb.width = tex->width0;
   fb.height = tex->height0;
   fb.cbufs[0] = surf;
   fb.nr_cbufs = 1;

   cso_set_framebuffer(cso, &fb);
   pipe_surface_reference(&surf, NULL);
}

static void
util_set_blend_normal(struct cso_context *cso)
{
   struct pipe_blend_state blend = {0};

   blend.rt[0].colormask = PIPE_MASK_RGBA;
   cso_set_blend(cso, &blend);
}

static void
util_set_dsa_disable(struct cso_context *cso)
{
   struct pipe_depth_stencil_alpha_state dsa = {{{0}}};

   cso_set_depth_stencil_alpha(cso, &dsa);
}

static void
util_set_rasterizer_normal(struct cso_context *cso)
{
   struct pipe_rasterizer_state rs = {0};

   rs.half_pixel_center = 1;
   rs.bottom_edge_rule = 1;
   rs.depth_clip_near = 1;
   rs.depth_clip_far = 1;

   cso_set_rasterizer(cso, &rs);
}

static void
util_set_max_viewport(struct cso_context *cso, struct pipe_resource *tex)
{
   struct pipe_viewport_state viewport;

   viewport.scale[0] = 0.5f * tex->width0;
   viewport.scale[1] = 0.5f * tex->height0;
   viewport.scale[2] = 1.0f;
   viewport.translate[0] = 0.5f * tex->width0;
   viewport.translate[1] = 0.5f * tex->height0;
   viewport.translate[2] = 0.0f;
   viewport.swizzle_x = PIPE_VIEWPORT_SWIZZLE_POSITIVE_X;
   viewport.swizzle_y = PIPE_VIEWPORT_SWIZZLE_POSITIVE_Y;
   viewport.swizzle_z = PIPE_VIEWPORT_SWIZZLE_POSITIVE_Z;
   viewport.swizzle_w = PIPE_VIEWPORT_SWIZZLE_POSITIVE_W;

   cso_set_viewport(cso, &viewport);
}

static struct cso_velems_state
util_get_interleaved_vertex_elements(struct cso_context *cso,
                                     unsigned num_elements)
{
   struct cso_velems_state velem;
   unsigned i;

   memset(&velem, 0, sizeof(velem));
   velem.count = num_elements;
   for (i = 0; i < num_elements; i++) {
      velem.velems[i].src_format = PIPE_FORMAT_R32G32B32A32_FLOAT;
      velem.velems[i].src_offset = i * 16;
      velem.velems[i].src_stride = num_elements * 4 * sizeof(float);
   }

   return velem;
}

static void *
util_set_passthrough_vertex_shader(struct cso_context *cso,
                                   struct pipe_context *ctx,
                                   bool window_space)
{
   static const enum tgsi_semantic vs_attribs[] = {
      TGSI_SEMANTIC_POSITION,
      TGSI_SEMANTIC_GENERIC
   };
   static const unsigned vs_indices[] = {0, 0};
   void *vs;

   vs = util_make_vertex_passthrough_shader(ctx, 2, vs_attribs, vs_indices,
                                            window_space);
   cso_set_vertex_shader_handle(cso, vs);
   return vs;
}

static void
util_set_common_states_and_clear(struct cso_context *cso, struct pipe_context *ctx,
                                 struct pipe_resource *cb)
{
   static const float clear_color[] = {0.1, 0.1, 0.1, 0.1};

   util_set_framebuffer_cb0(cso, ctx, cb);
   util_set_blend_normal(cso);
   util_set_dsa_disable(cso);
   util_set_rasterizer_normal(cso);
   util_set_max_viewport(cso, cb);

   ctx->clear(ctx, PIPE_CLEAR_COLOR0, NULL, (void*)clear_color, 0, 0);
}

static void
util_draw_fullscreen_quad(struct cso_context *cso)
{
   static float vertices[] = {
     -1, -1, 0, 1,   0, 0, 0, 0,
     -1,  1, 0, 1,   0, 1, 0, 0,
      1,  1, 0, 1,   1, 1, 0, 0,
      1, -1, 0, 1,   1, 0, 0, 0
   };
   struct cso_velems_state ve = util_get_interleaved_vertex_elements(cso, 2);

   util_draw_user_vertices(cso, &ve, vertices, MESA_PRIM_QUADS, 4);
}

static void
util_draw_fullscreen_quad_fill(struct cso_context *cso,
                               float r, float g, float b, float a)
{
   float vertices[] = {
     -1, -1, 0, 1,   r, g, b, a,
     -1,  1, 0, 1,   r, g, b, a,
      1,  1, 0, 1,   r, g, b, a,
      1, -1, 0, 1,   r, g, b, a,
   };
   struct cso_velems_state ve = util_get_interleaved_vertex_elements(cso, 2);

   util_draw_user_vertices(cso, &ve, vertices, MESA_PRIM_QUADS, 4);
}

/**
 * Probe and test if the rectangle contains the expected color.
 *
 * If "num_expected_colors" > 1, at least one expected color must match
 * the probed color. "expected" should be an array of 4*num_expected_colors
 * floats.
 */
static bool
util_probe_rect_rgba_multi(struct pipe_context *ctx, struct pipe_resource *tex,
                           unsigned offx, unsigned offy, unsigned w,
                           unsigned h,
                           const float *expected,
                           unsigned num_expected_colors)
{
   struct pipe_transfer *transfer;
   void *map;
   float *pixels = malloc(w * h * 4 * sizeof(float));
   unsigned x,y,e,c;
   bool pass = true;

   map = pipe_texture_map(ctx, tex, 0, 0, PIPE_MAP_READ,
                           offx, offy, w, h, &transfer);
   pipe_get_tile_rgba(transfer, map, 0, 0, w, h, tex->format, pixels);
   pipe_texture_unmap(ctx, transfer);

   for (e = 0; e < num_expected_colors; e++) {
      for (y = 0; y < h; y++) {
         for (x = 0; x < w; x++) {
            float *probe = &pixels[(y*w + x)*4];

            for (c = 0; c < 4; c++) {
               if (fabs(probe[c] - expected[e*4+c]) >= TOLERANCE) {
                  if (e < num_expected_colors-1)
                     goto next_color; /* test the next expected color */

                  printf("Probe color at (%i,%i),  ", offx+x, offy+y);
                  printf("Expected: %.3f, %.3f, %.3f, %.3f,  ",
                         expected[e*4], expected[e*4+1],
                         expected[e*4+2], expected[e*4+3]);
                  printf("Got: %.3f, %.3f, %.3f, %.3f\n",
                         probe[0], probe[1], probe[2], probe[3]);
                  pass = false;
                  goto done;
               }
            }
         }
      }
      break; /* this color was successful */

   next_color:;
   }
done:

   free(pixels);
   return pass;
}

static bool
util_probe_rect_rgba(struct pipe_context *ctx, struct pipe_resource *tex,
                     unsigned offx, unsigned offy, unsigned w, unsigned h,
                     const float *expected)
{
   return util_probe_rect_rgba_multi(ctx, tex, offx, offy, w, h, expected, 1);
}

enum {
   SKIP = -1,
   FAIL = 0, /* also "false" */
   PASS = 1 /* also "true" */
};

static void
util_report_result_helper(int status, const char *name, ...)
{
   char buf[256];
   va_list ap;

   va_start(ap, name);
   vsnprintf(buf, sizeof(buf), name, ap);
   va_end(ap);

   printf("Test(%s) = %s\n", buf,
          status == SKIP ? "skip" :
          status == PASS ? "pass" : "fail");
}

#define util_report_result(status) util_report_result_helper(status, __func__)

/**
 * Test TGSI_PROPERTY_VS_WINDOW_SPACE_POSITION.
 *
 * The viewport state is set as usual, but it should have no effect.
 * Clipping should also be disabled.
 *
 * POSITION.xyz should already be multiplied by 1/w and POSITION.w should
 * contain 1/w. By setting w=0, we can test that POSITION.xyz isn't
 * multiplied by 1/w (otherwise nothing would be rendered).
 *
 * TODO: Whether the value of POSITION.w is correctly interpreted as 1/w
 *       during perspective interpolation is not tested.
 */
static void
tgsi_vs_window_space_position(struct pipe_context *ctx)
{
   struct cso_context *cso;
   struct pipe_resource *cb;
   void *fs, *vs;
   bool pass = true;
   static const float red[] = {1, 0, 0, 1};

   if (!ctx->screen->get_param(ctx->screen,
                               PIPE_CAP_VS_WINDOW_SPACE_POSITION)) {
      util_report_result(SKIP);
      return;
   }

   cso = cso_create_context(ctx, 0);
   cb = util_create_texture2d(ctx->screen, 256, 256,
                              PIPE_FORMAT_R8G8B8A8_UNORM, 0);
   util_set_common_states_and_clear(cso, ctx, cb);

   /* Fragment shader. */
   fs = util_make_fragment_passthrough_shader(ctx, TGSI_SEMANTIC_GENERIC,
                                       TGSI_INTERPOLATE_LINEAR, true);
   cso_set_fragment_shader_handle(cso, fs);

   /* Vertex shader. */
   vs = util_set_passthrough_vertex_shader(cso, ctx, true);

   /* Draw. */
   {
      static float vertices[] = {
          0,   0, 0, 0,   1,  0, 0, 1,
          0, 256, 0, 0,   1,  0, 0, 1,
        256, 256, 0, 0,   1,  0, 0, 1,
        256,   0, 0, 0,   1,  0, 0, 1,
      };
      struct cso_velems_state ve = util_get_interleaved_vertex_elements(cso, 2);

      util_draw_user_vertices(cso, &ve, vertices, MESA_PRIM_QUADS, 4);
   }

   /* Probe pixels. */
   pass = pass && util_probe_rect_rgba(ctx, cb, 0, 0,
                                       cb->width0, cb->height0, red);

   /* Cleanup. */
   cso_destroy_context(cso);
   ctx->delete_vs_state(ctx, vs);
   ctx->delete_fs_state(ctx, fs);
   pipe_resource_reference(&cb, NULL);

   util_report_result(pass);
}

static void
null_sampler_view(struct pipe_context *ctx, unsigned tgsi_tex_target)
{
   struct cso_context *cso;
   struct pipe_resource *cb;
   void *fs, *vs;
   bool pass = true;
   /* 2 expected colors: */
   static const float expected_tex[] = {0, 0, 0, 1,
                                        0, 0, 0, 0};
   static const float expected_buf[] = {0, 0, 0, 0};
   const float *expected = tgsi_tex_target == TGSI_TEXTURE_BUFFER ?
                              expected_buf : expected_tex;
   unsigned num_expected = tgsi_tex_target == TGSI_TEXTURE_BUFFER ? 1 : 2;

   if (tgsi_tex_target == TGSI_TEXTURE_BUFFER &&
       !ctx->screen->get_param(ctx->screen, PIPE_CAP_TEXTURE_BUFFER_OBJECTS)) {
      util_report_result_helper(SKIP, "%s: %s", __func__,
                                tgsi_texture_names[tgsi_tex_target]);
      return;
   }

   cso = cso_create_context(ctx, 0);
   cb = util_create_texture2d(ctx->screen, 256, 256,
                              PIPE_FORMAT_R8G8B8A8_UNORM, 0);
   util_set_common_states_and_clear(cso, ctx, cb);

   ctx->set_sampler_views(ctx, PIPE_SHADER_FRAGMENT, 0, 0, 1, false, NULL);

   /* Fragment shader. */
   fs = util_make_fragment_tex_shader(ctx, tgsi_tex_target,
                                      TGSI_RETURN_TYPE_FLOAT,
                                      TGSI_RETURN_TYPE_FLOAT, false, false);
   cso_set_fragment_shader_handle(cso, fs);

   /* Vertex shader. */
   vs = util_set_passthrough_vertex_shader(cso, ctx, false);
   util_draw_fullscreen_quad(cso);

   /* Probe pixels. */
   pass = pass && util_probe_rect_rgba_multi(ctx, cb, 0, 0,
                                  cb->width0, cb->height0, expected,
                                  num_expected);

   /* Cleanup. */
   cso_destroy_context(cso);
   ctx->delete_vs_state(ctx, vs);
   ctx->delete_fs_state(ctx, fs);
   pipe_resource_reference(&cb, NULL);

   util_report_result_helper(pass, "%s: %s", __func__,
                             tgsi_texture_names[tgsi_tex_target]);
}

void
util_test_constant_buffer(struct pipe_context *ctx,
                          struct pipe_resource *constbuf)
{
   struct cso_context *cso;
   struct pipe_resource *cb;
   void *fs, *vs;
   bool pass = true;
   static const float zero[] = {0, 0, 0, 0};

   cso = cso_create_context(ctx, 0);
   cb = util_create_texture2d(ctx->screen, 256, 256,
                              PIPE_FORMAT_R8G8B8A8_UNORM, 0);
   util_set_common_states_and_clear(cso, ctx, cb);

   pipe_set_constant_buffer(ctx, PIPE_SHADER_FRAGMENT, 0, constbuf);

   /* Fragment shader. */
   {
      static const char *text = /* I don't like ureg... */
            "FRAG\n"
            "DCL CONST[0][0]\n"
            "DCL OUT[0], COLOR\n"

            "MOV OUT[0], CONST[0][0]\n"
            "END\n";
      struct tgsi_token tokens[1000];
      struct pipe_shader_state state = {0};

      if (!tgsi_text_translate(text, tokens, ARRAY_SIZE(tokens))) {
         puts("Can't compile a fragment shader.");
         util_report_result(FAIL);
         return;
      }
      pipe_shader_state_from_tgsi(&state, tokens);
      fs = ctx->create_fs_state(ctx, &state);
      cso_set_fragment_shader_handle(cso, fs);
   }

   /* Vertex shader. */
   vs = util_set_passthrough_vertex_shader(cso, ctx, false);
   util_draw_fullscreen_quad(cso);

   /* Probe pixels. */
   pass = pass && util_probe_rect_rgba(ctx, cb, 0, 0, cb->width0,
                                       cb->height0, zero);

   /* Cleanup. */
   cso_destroy_context(cso);
   ctx->delete_vs_state(ctx, vs);
   ctx->delete_fs_state(ctx, fs);
   pipe_resource_reference(&cb, NULL);

   util_report_result(pass);
}

static void
disabled_fragment_shader(struct pipe_context *ctx)
{
   struct cso_context *cso;
   struct pipe_resource *cb;
   void *vs;
   struct pipe_rasterizer_state rs = {0};
   struct pipe_query *query;
   union pipe_query_result qresult;

   cso = cso_create_context(ctx, 0);
   cb = util_create_texture2d(ctx->screen, 256, 256,
                              PIPE_FORMAT_R8G8B8A8_UNORM, 0);
   util_set_common_states_and_clear(cso, ctx, cb);

   /* No rasterization. */
   rs.rasterizer_discard = 1;
   cso_set_rasterizer(cso, &rs);

   vs = util_set_passthrough_vertex_shader(cso, ctx, false);

   void *fs = util_make_empty_fragment_shader(ctx);
   cso_set_fragment_shader_handle(cso, fs);

   query = ctx->create_query(ctx, PIPE_QUERY_PRIMITIVES_GENERATED, 0);
   ctx->begin_query(ctx, query);
   util_draw_fullscreen_quad(cso);
   ctx->end_query(ctx, query);
   ctx->get_query_result(ctx, query, true, &qresult);

   /* Cleanup. */
   cso_destroy_context(cso);
   ctx->delete_vs_state(ctx, vs);
   ctx->delete_fs_state(ctx, fs);
   ctx->destroy_query(ctx, query);
   pipe_resource_reference(&cb, NULL);

   /* Check PRIMITIVES_GENERATED. */
   util_report_result(qresult.u64 == 2);
}

#if DETECT_OS_LINUX && defined(HAVE_LIBDRM)
#include <libsync.h>
#else
#define sync_merge(str, fd1, fd2) (-1)
#define sync_wait(fd, timeout) (-1)
#endif

static void
test_sync_file_fences(struct pipe_context *ctx)
{
   struct pipe_screen *screen = ctx->screen;
   bool pass = true;
   enum pipe_fd_type fd_type = PIPE_FD_TYPE_NATIVE_SYNC;

   if (!screen->get_param(screen, PIPE_CAP_NATIVE_FENCE_FD))
      return;

   struct cso_context *cso = cso_create_context(ctx, 0);
   struct pipe_resource *buf =
      pipe_buffer_create(screen, 0, PIPE_USAGE_DEFAULT, 1024 * 1024);
   struct pipe_resource *tex =
      util_create_texture2d(screen, 4096, 1024, PIPE_FORMAT_R8_UNORM, 0);
   struct pipe_fence_handle *buf_fence = NULL, *tex_fence = NULL;

   /* Run 2 clears, get fencess. */
   uint32_t value = 0;
   ctx->clear_buffer(ctx, buf, 0, buf->width0, &value, sizeof(value));
   ctx->flush(ctx, &buf_fence, PIPE_FLUSH_FENCE_FD);

   struct pipe_box box;
   u_box_2d(0, 0, tex->width0, tex->height0, &box);
   ctx->clear_texture(ctx, tex, 0, &box, &value);
   ctx->flush(ctx, &tex_fence, PIPE_FLUSH_FENCE_FD);
   pass = pass && buf_fence && tex_fence;

   /* Export fences. */
   int buf_fd = screen->fence_get_fd(screen, buf_fence);
   int tex_fd = screen->fence_get_fd(screen, tex_fence);
   pass = pass && buf_fd >= 0 && tex_fd >= 0;

   /* Merge fences. */
   int merged_fd = sync_merge("test", buf_fd, tex_fd);
   pass = pass && merged_fd >= 0;

   /* (Re)import all fences. */
   struct pipe_fence_handle *re_buf_fence = NULL, *re_tex_fence = NULL;
   struct pipe_fence_handle *merged_fence = NULL;
   ctx->create_fence_fd(ctx, &re_buf_fence, buf_fd, fd_type);
   ctx->create_fence_fd(ctx, &re_tex_fence, tex_fd, fd_type);
   ctx->create_fence_fd(ctx, &merged_fence, merged_fd, fd_type);
   pass = pass && re_buf_fence && re_tex_fence && merged_fence;

   /* Run another clear after waiting for everything. */
   struct pipe_fence_handle *final_fence = NULL;
   ctx->fence_server_sync(ctx, merged_fence);
   value = 0xff;
   ctx->clear_buffer(ctx, buf, 0, buf->width0, &value, sizeof(value));
   ctx->flush(ctx, &final_fence, PIPE_FLUSH_FENCE_FD);
   pass = pass && final_fence;

   /* Wait for the last fence. */
   int final_fd = screen->fence_get_fd(screen, final_fence);
   pass = pass && final_fd >= 0;
   pass = pass && sync_wait(final_fd, -1) == 0;

   /* Check that all fences are signalled. */
   pass = pass && sync_wait(buf_fd, 0) == 0;
   pass = pass && sync_wait(tex_fd, 0) == 0;
   pass = pass && sync_wait(merged_fd, 0) == 0;

   pass = pass && screen->fence_finish(screen, NULL, buf_fence, 0);
   pass = pass && screen->fence_finish(screen, NULL, tex_fence, 0);
   pass = pass && screen->fence_finish(screen, NULL, re_buf_fence, 0);
   pass = pass && screen->fence_finish(screen, NULL, re_tex_fence, 0);
   pass = pass && screen->fence_finish(screen, NULL, merged_fence, 0);
   pass = pass && screen->fence_finish(screen, NULL, final_fence, 0);

   /* Cleanup. */
#if !DETECT_OS_WINDOWS
   if (buf_fd >= 0)
      close(buf_fd);
   if (tex_fd >= 0)
      close(tex_fd);
   if (merged_fd >= 0)
      close(merged_fd);
   if (final_fd >= 0)
      close(final_fd);
#endif

   screen->fence_reference(screen, &buf_fence, NULL);
   screen->fence_reference(screen, &tex_fence, NULL);
   screen->fence_reference(screen, &re_buf_fence, NULL);
   screen->fence_reference(screen, &re_tex_fence, NULL);
   screen->fence_reference(screen, &merged_fence, NULL);
   screen->fence_reference(screen, &final_fence, NULL);

   cso_destroy_context(cso);
   pipe_resource_reference(&buf, NULL);
   pipe_resource_reference(&tex, NULL);

   util_report_result(pass);
}

static void
test_texture_barrier(struct pipe_context *ctx, bool use_fbfetch,
                     unsigned num_samples)
{
   struct cso_context *cso;
   struct pipe_resource *cb;
   struct pipe_sampler_view *view = NULL;
   char name[256];
   const char *text;

   assert(num_samples >= 1 && num_samples <= 8);

   snprintf(name, sizeof(name), "%s: %s, %u samples", __func__,
            use_fbfetch ? "FBFETCH" : "sampler", MAX2(num_samples, 1));

   if (!ctx->screen->get_param(ctx->screen, PIPE_CAP_TEXTURE_BARRIER)) {
      util_report_result_helper(SKIP, name);
      return;
   }
   if (use_fbfetch &&
       !ctx->screen->get_param(ctx->screen, PIPE_CAP_FBFETCH)) {
      util_report_result_helper(SKIP, name);
      return;
   }

   cso = cso_create_context(ctx, 0);
   cb = util_create_texture2d(ctx->screen, 256, 256,
                              PIPE_FORMAT_R8G8B8A8_UNORM, num_samples);
   util_set_common_states_and_clear(cso, ctx, cb);

   /* Clear each sample to a different value. */
   if (num_samples > 1) {
      void *fs =
         util_make_fragment_passthrough_shader(ctx, TGSI_SEMANTIC_GENERIC,
                                               TGSI_INTERPOLATE_LINEAR, true);
      cso_set_fragment_shader_handle(cso, fs);

      /* Vertex shader. */
      void *vs = util_set_passthrough_vertex_shader(cso, ctx, false);

      for (unsigned i = 0; i < num_samples / 2; i++) {
         float value;

         /* 2 consecutive samples should have the same color to test MSAA
          * compression properly.
          */
         if (num_samples == 2) {
            value = 0.1;
         } else {
            /* The average value must be 0.1 */
            static const float values[] = {
               0.0, 0.2, 0.05, 0.15
            };
            value = values[i];
         }

         ctx->set_sample_mask(ctx, 0x3 << (i * 2));
         util_draw_fullscreen_quad_fill(cso, value, value, value, value);
      }
      ctx->set_sample_mask(ctx, ~0);

      cso_set_vertex_shader_handle(cso, NULL);
      cso_set_fragment_shader_handle(cso, NULL);
      ctx->delete_vs_state(ctx, vs);
      ctx->delete_fs_state(ctx, fs);
   }

   if (use_fbfetch) {
      /* Fragment shader. */
      text = "FRAG\n"
             "DCL OUT[0], COLOR[0]\n"
             "DCL TEMP[0]\n"
             "IMM[0] FLT32 { 0.1, 0.2, 0.3, 0.4}\n"

             "FBFETCH TEMP[0], OUT[0]\n"
             "ADD OUT[0], TEMP[0], IMM[0]\n"
             "END\n";
   } else {
      struct pipe_sampler_view templ = {0};
      templ.format = cb->format;
      templ.target = cb->target;
      templ.swizzle_r = PIPE_SWIZZLE_X;
      templ.swizzle_g = PIPE_SWIZZLE_Y;
      templ.swizzle_b = PIPE_SWIZZLE_Z;
      templ.swizzle_a = PIPE_SWIZZLE_W;
      view = ctx->create_sampler_view(ctx, cb, &templ);
      ctx->set_sampler_views(ctx, PIPE_SHADER_FRAGMENT, 0, 1, 0, false, &view);

      /* Fragment shader. */
      if (num_samples > 1) {
         text = "FRAG\n"
                "DCL SV[0], POSITION\n"
                "DCL SV[1], SAMPLEID\n"
                "DCL SAMP[0]\n"
                "DCL SVIEW[0], 2D_MSAA, FLOAT\n"
                "DCL OUT[0], COLOR[0]\n"
                "DCL TEMP[0]\n"
                "IMM[0] FLT32 { 0.1, 0.2, 0.3, 0.4}\n"

                "F2I TEMP[0].xy, SV[0].xyyy\n"
                "MOV TEMP[0].w, SV[1].xxxx\n"
                "TXF TEMP[0], TEMP[0], SAMP[0], 2D_MSAA\n"
                "ADD OUT[0], TEMP[0], IMM[0]\n"
                "END\n";
      } else {
         text = "FRAG\n"
                "DCL SV[0], POSITION\n"
                "DCL SAMP[0]\n"
                "DCL SVIEW[0], 2D, FLOAT\n"
                "DCL OUT[0], COLOR[0]\n"
                "DCL TEMP[0]\n"
                "IMM[0] FLT32 { 0.1, 0.2, 0.3, 0.4}\n"
                "IMM[1] INT32 { 0, 0, 0, 0}\n"

                "F2I TEMP[0].xy, SV[0].xyyy\n"
                "MOV TEMP[0].zw, IMM[1]\n"
                "TXF TEMP[0], TEMP[0], SAMP[0], 2D\n"
                "ADD OUT[0], TEMP[0], IMM[0]\n"
                "END\n";
      }
   }

   struct tgsi_token tokens[1000];
   struct pipe_shader_state state = {0};

   if (!tgsi_text_translate(text, tokens, ARRAY_SIZE(tokens))) {
      assert(0);
      util_report_result_helper(FAIL, name);
      return;
   }
   pipe_shader_state_from_tgsi(&state, tokens);

   void *fs = ctx->create_fs_state(ctx, &state);
   cso_set_fragment_shader_handle(cso, fs);

   /* Vertex shader. */
   void *vs = util_set_passthrough_vertex_shader(cso, ctx, false);

   if (num_samples > 1 && !use_fbfetch)
      ctx->set_min_samples(ctx, num_samples);

   for (int i = 0; i < 2; i++) {
      ctx->texture_barrier(ctx,
                           use_fbfetch ? PIPE_TEXTURE_BARRIER_FRAMEBUFFER :
                                         PIPE_TEXTURE_BARRIER_SAMPLER);
      util_draw_fullscreen_quad(cso);
   }
   if (num_samples > 1 && !use_fbfetch)
      ctx->set_min_samples(ctx, 1);

   /* Probe pixels.
    *
    * For single sample:
    *   result = 0.1 (clear) + (0.1, 0.2, 0.3, 0.4) * 2 = (0.3, 0.5, 0.7, 0.9)
    *
    * For MSAA 4x:
    *   sample0 = 0.0 (clear) + (0.1, 0.2, 0.3, 0.4) * 2 = (0.2, 0.4, 0.6, 0.8)
    *   sample1 = sample0
    *   sample2 = 0.2 (clear) + (0.1, 0.2, 0.3, 0.4) * 2 = (0.4, 0.6, 0.8, 1.0)
    *   sample3 = sample2
    *   resolved = sum(sample[0:3]) / 4 = (0.3, 0.5, 0.7, 0.9)
    */
   static const float expected[] = {0.3, 0.5, 0.7, 0.9};
   bool pass = util_probe_rect_rgba(ctx, cb, 0, 0,
                                    cb->width0, cb->height0, expected);

   /* Cleanup. */
   cso_destroy_context(cso);
   ctx->delete_vs_state(ctx, vs);
   ctx->delete_fs_state(ctx, fs);
   pipe_sampler_view_reference(&view, NULL);
   pipe_resource_reference(&cb, NULL);

   util_report_result_helper(pass, name);
}

static void
test_compute_clear_image_shader(struct pipe_context *ctx)
{
   struct pipe_resource *cb;
   const char *text;

   cb = util_create_texture2d(ctx->screen, 256, 256,
                              PIPE_FORMAT_R8G8B8A8_UNORM, 1);

   /* Compute shader. */
   text = "COMP\n"
          "PROPERTY CS_FIXED_BLOCK_WIDTH 8\n"
          "PROPERTY CS_FIXED_BLOCK_HEIGHT 8\n"
          "PROPERTY CS_FIXED_BLOCK_DEPTH 1\n"
          "DCL SV[0], THREAD_ID\n"
          "DCL SV[1], BLOCK_ID\n"
          "DCL IMAGE[0], 2D, PIPE_FORMAT_R8G8B8A8_UNORM, WR\n"
          "DCL TEMP[0]\n"
          "IMM[0] UINT32 { 8, 8, 0, 0}\n"
          "IMM[1] FLT32 { 1, 0, 0, 0}\n"

          /* TEMP[0].xy = SV[1] * IMM[0] + SV[0]; */
          "UMAD TEMP[0].xy, SV[1], IMM[0], SV[0]\n"
          "STORE IMAGE[0], TEMP[0], IMM[1], 2D, PIPE_FORMAT_R8G8B8A8_UNORM\n"
          "END\n";

   struct tgsi_token tokens[1000];
   if (!tgsi_text_translate(text, tokens, ARRAY_SIZE(tokens))) {
      assert(0);
      util_report_result(FAIL);
      return;
   }

   struct pipe_compute_state state = {0};
   state.ir_type = PIPE_SHADER_IR_TGSI;
   state.prog = tokens;

   void *compute_shader = ctx->create_compute_state(ctx, &state);
   ctx->bind_compute_state(ctx, compute_shader);

   /* Bind the image. */
   struct pipe_image_view image = {0};
   image.resource = cb;
   image.shader_access = image.access = PIPE_IMAGE_ACCESS_READ_WRITE;
   image.format = cb->format;

   ctx->set_shader_images(ctx, PIPE_SHADER_COMPUTE, 0, 1, 0, &image);

   /* Dispatch compute. */
   struct pipe_grid_info info = {0};
   info.block[0] = 8;
   info.block[1] = 8;
   info.block[2] = 1;
   info.grid[0] = cb->width0 / 8;
   info.grid[1] = cb->height0 / 8;
   info.grid[2] = 1;

   ctx->launch_grid(ctx, &info);

   /* Check pixels. */
   static const float expected[] = {1.0, 0.0, 0.0, 0.0};
   bool pass = util_probe_rect_rgba(ctx, cb, 0, 0,
                                    cb->width0, cb->height0, expected);

   /* Cleanup. */
   ctx->delete_compute_state(ctx, compute_shader);
   pipe_resource_reference(&cb, NULL);

   util_report_result(pass);
}

static void
test_compute_clear_texture(struct pipe_context *ctx)
{
   struct pipe_resource *tex;

   tex = util_create_texture2d(ctx->screen, 256, 256,
                              PIPE_FORMAT_R8G8B8A8_UNORM, 1);
   srand(time(NULL));
   uint8_t data[] = {rand() % 256, rand() % 256, rand() % 256, rand() % 256};
   float expected[] = {
      ubyte_to_float(data[0]),
      ubyte_to_float(data[1]),
      ubyte_to_float(data[2]),
      ubyte_to_float(data[3]),
   };

   struct pipe_box box;
   u_box_2d(0, 0, tex->width0, tex->height0, &box);
   ctx->clear_texture(ctx, tex, 0, &box, &data);

   /* Check pixels. */
   bool pass = util_probe_rect_rgba(ctx, tex, 0, 0,
                                    tex->width0, tex->height0, expected);

   /* Cleanup. */
   pipe_resource_reference(&tex, NULL);

   util_report_result(pass);
}

static void
test_compute_resource_copy_region(struct pipe_context *ctx)
{
   struct pipe_resource *src, *dst;

   src = util_create_texture2d(ctx->screen, 256, 256,
                              PIPE_FORMAT_R8G8B8A8_UNORM, 1);
   dst = util_create_texture2d(ctx->screen, 256, 256,
                              PIPE_FORMAT_R8G8B8A8_UNORM, 1);
   srand(time(NULL));
   uint8_t data[] = {rand() % 256, rand() % 256, rand() % 256, rand() % 256};
   float expected[] = {
      ubyte_to_float(data[0]),
      ubyte_to_float(data[1]),
      ubyte_to_float(data[2]),
      ubyte_to_float(data[3]),
   };

   struct pipe_box box;
   u_box_2d(0, 0, src->width0, src->height0, &box);
   ctx->clear_texture(ctx, src, 0, &box, &data);
   ctx->resource_copy_region(ctx, dst, 0, 0, 0, 0, src, 0, &box);

   /* Check pixels. */
   bool pass = util_probe_rect_rgba(ctx, dst, 0, 0,
                                    dst->width0, dst->height0, expected);

   /* Cleanup. */
   pipe_resource_reference(&src, NULL);
   pipe_resource_reference(&dst, NULL);

   util_report_result(pass);
}

#define NV12_WIDTH   2560
#define NV12_HEIGHT  1440

static bool
nv12_validate_resource_fields(struct pipe_resource *tex)
{
   return tex->format == util_format_get_plane_format(PIPE_FORMAT_NV12, 0) &&
         tex->width0 == NV12_WIDTH &&
         tex->height0 == NV12_HEIGHT &&
         tex->last_level == 0 &&
         tex->usage == PIPE_USAGE_DEFAULT &&
         tex->next &&
         tex->next->format == util_format_get_plane_format(PIPE_FORMAT_NV12, 1) &&
         tex->next->width0 == tex->width0 / 2 &&
         tex->next->height0 == tex->height0 / 2 &&
         tex->next->usage == tex->usage;
}

/* This test enforces the behavior of NV12 allocation and exports. */
static void
test_nv12(struct pipe_screen *screen)
{
   struct pipe_resource *tex = util_create_texture2d(screen, NV12_WIDTH, NV12_HEIGHT,
                                                     PIPE_FORMAT_NV12, 1);

   if (!tex) {
      printf("resource_create failed\n");
      util_report_result(false);
      return;
   }

   if (!nv12_validate_resource_fields(tex)) {
      printf("incorrect pipe_resource fields\n");
      util_report_result(false);
      return;
   }

   /* resource_get_param */
   if (screen->resource_get_param) {
      struct {
         uint64_t handle, dmabuf, offset, stride, planes;
      } handle[3];

      /* Export */
      for (unsigned i = 0; i < 3; i++) {
         struct pipe_resource *res = i == 2 ? tex->next : tex;
         unsigned plane = i == 2 ? 0 : i;

         if (!screen->resource_get_param(screen, NULL, res, plane, 0, 0,
                                         PIPE_RESOURCE_PARAM_HANDLE_TYPE_KMS,
                                         0, &handle[i].handle)) {
            printf("resource_get_param failed\n");
            util_report_result(false);
            goto cleanup;
         }

         if (!screen->resource_get_param(screen, NULL, res, plane, 0, 0,
                                         PIPE_RESOURCE_PARAM_HANDLE_TYPE_FD,
                                         0, &handle[i].dmabuf)) {
            printf("resource_get_param failed\n");
            util_report_result(false);
            goto cleanup;
         }

         if (!screen->resource_get_param(screen, NULL, res, plane, 0, 0,
                                         PIPE_RESOURCE_PARAM_OFFSET,
                                         0, &handle[i].offset)) {
            printf("resource_get_param failed\n");
            util_report_result(false);
            goto cleanup;
         }

         if (!screen->resource_get_param(screen, NULL, res, plane, 0, 0,
                                         PIPE_RESOURCE_PARAM_STRIDE,
                                         0, &handle[i].stride)) {
            printf("resource_get_param failed\n");
            util_report_result(false);
            goto cleanup;
         }

         if (!screen->resource_get_param(screen, NULL, res, plane, 0, 0,
                                         PIPE_RESOURCE_PARAM_NPLANES,
                                         0, &handle[i].planes)) {
            printf("resource_get_param failed\n");
            util_report_result(false);
            goto cleanup;
         }
      }

      /* Validate export.  */
      bool get_param_pass = /* Sanity checking */
                            handle[0].handle && handle[1].handle && handle[2].handle &&
                            handle[0].dmabuf && handle[1].dmabuf && handle[2].dmabuf &&
                            handle[0].stride && handle[1].stride && handle[2].stride &&
                            handle[0].planes == 2 &&
                            handle[1].planes == 2 &&
                            handle[2].planes == 2 &&
                            /* Different planes */
                            handle[0].handle == handle[1].handle &&
                            handle[0].offset != handle[1].offset &&
                            /* Same planes. */
                            handle[1].handle == handle[2].handle &&
                            handle[1].stride == handle[2].stride &&
                            handle[1].offset == handle[2].offset;

      if (!get_param_pass) {
         printf("resource_get_param returned incorrect values\n");
         util_report_result(false);
         goto cleanup;
      }
   }

   /* resource_get_handle */
   struct winsys_handle handle[4] = {{0}};

   /* Export */
   for (unsigned i = 0; i < 4; i++) {
      handle[i].type = i < 2 ? WINSYS_HANDLE_TYPE_KMS : WINSYS_HANDLE_TYPE_FD;
      handle[i].plane = i % 2;

      if (!screen->resource_get_handle(screen, NULL, tex, &handle[i], 0)) {
         printf("resource_get_handle failed\n");
         util_report_result(false);
         goto cleanup;
      }
   }

   /* Validate export. */
   bool get_handle_pass = /* Sanity checking */
                          handle[0].handle && handle[1].handle &&
                          handle[0].stride && handle[1].stride &&
                          handle[2].handle && handle[3].handle &&
                          handle[2].stride && handle[3].stride &&
                          /* KMS - different planes */
                          handle[0].handle == handle[1].handle &&
                          handle[0].offset != handle[1].offset &&
                          /* DMABUF - different planes */
                          handle[2].offset != handle[3].offset &&
                          /* KMS and DMABUF equivalence */
                          handle[0].offset == handle[2].offset &&
                          handle[1].offset == handle[3].offset &&
                          handle[0].stride == handle[2].stride &&
                          handle[1].stride == handle[3].stride;

   if (!get_handle_pass) {
      printf("resource_get_handle returned incorrect values\n");
      util_report_result(false);
      goto cleanup;
   }

   util_report_result(true);

cleanup:
   pipe_resource_reference(&tex, NULL);
}

/**
 * Run all tests. This should be run with a clean context after
 * context_create.
 */
void
util_run_tests(struct pipe_screen *screen)
{
   struct pipe_context *ctx = screen->context_create(screen, NULL, 0);

   disabled_fragment_shader(ctx);
   tgsi_vs_window_space_position(ctx);
   null_sampler_view(ctx, TGSI_TEXTURE_2D);
   null_sampler_view(ctx, TGSI_TEXTURE_BUFFER);
   util_test_constant_buffer(ctx, NULL);
   test_sync_file_fences(ctx);

   for (int i = 1; i <= 8; i = i * 2)
      test_texture_barrier(ctx, false, i);
   for (int i = 1; i <= 8; i = i * 2)
      test_texture_barrier(ctx, true, i);
   ctx->destroy(ctx);

   ctx = screen->context_create(screen, NULL, PIPE_CONTEXT_COMPUTE_ONLY);
   test_compute_clear_image_shader(ctx);
   test_compute_clear_texture(ctx);
   test_compute_resource_copy_region(ctx);
   ctx->destroy(ctx);

   test_nv12(screen);

   puts("Done. Exiting..");
   exit(0);
}
