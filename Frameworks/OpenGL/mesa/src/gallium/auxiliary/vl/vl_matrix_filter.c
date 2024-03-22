/**************************************************************************
 *
 * Copyright 2012 Christian König.
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

#include <stdio.h>

#include "pipe/p_context.h"

#include "tgsi/tgsi_ureg.h"

#include "util/u_draw.h"
#include "util/u_memory.h"
#include "util/u_math.h"

#include "vl_types.h"
#include "vl_vertex_buffers.h"
#include "vl_matrix_filter.h"

enum VS_OUTPUT
{
   VS_O_VPOS = 0,
   VS_O_VTEX = 0
};

static void *
create_vert_shader(struct vl_matrix_filter *filter)
{
   struct ureg_program *shader;
   struct ureg_src i_vpos;
   struct ureg_dst o_vpos, o_vtex;

   shader = ureg_create(PIPE_SHADER_VERTEX);
   if (!shader)
      return NULL;

   i_vpos = ureg_DECL_vs_input(shader, 0);
   o_vpos = ureg_DECL_output(shader, TGSI_SEMANTIC_POSITION, VS_O_VPOS);
   o_vtex = ureg_DECL_output(shader, TGSI_SEMANTIC_GENERIC, VS_O_VTEX);

   ureg_MOV(shader, o_vpos, i_vpos);
   ureg_MOV(shader, o_vtex, i_vpos);

   ureg_END(shader);

   return ureg_create_shader_and_destroy(shader, filter->pipe);
}

static inline bool
is_vec_zero(struct vertex2f v)
{
   return v.x == 0.0f && v.y == 0.0f;
}

static void *
create_frag_shader(struct vl_matrix_filter *filter, unsigned num_offsets,
                   struct vertex2f *offsets, const float *matrix_values)
{
   struct ureg_program *shader;
   struct ureg_src i_vtex;
   struct ureg_src sampler;
   struct ureg_dst tmp;
   struct ureg_dst t_sum;
   struct ureg_dst o_fragment;
   unsigned i;

   shader = ureg_create(PIPE_SHADER_FRAGMENT);
   if (!shader) {
      return NULL;
   }

   i_vtex = ureg_DECL_fs_input(shader, TGSI_SEMANTIC_GENERIC, VS_O_VTEX, TGSI_INTERPOLATE_LINEAR);
   sampler = ureg_DECL_sampler(shader, 0);
   ureg_DECL_sampler_view(shader, 0, TGSI_TEXTURE_2D,
                          TGSI_RETURN_TYPE_FLOAT,
                          TGSI_RETURN_TYPE_FLOAT,
                          TGSI_RETURN_TYPE_FLOAT,
                          TGSI_RETURN_TYPE_FLOAT);

   tmp = ureg_DECL_temporary(shader);
   t_sum = ureg_DECL_temporary(shader);
   o_fragment = ureg_DECL_output(shader, TGSI_SEMANTIC_COLOR, 0);

   ureg_MOV(shader, t_sum, ureg_imm1f(shader, 0.0f));
   for (i = 0; i < num_offsets; ++i) {
      if (matrix_values[i] == 0.0f)
         continue;

      if (!is_vec_zero(offsets[i])) {
         ureg_ADD(shader, ureg_writemask(tmp, TGSI_WRITEMASK_XY),
                  i_vtex, ureg_imm2f(shader, offsets[i].x, offsets[i].y));
         ureg_MOV(shader, ureg_writemask(tmp, TGSI_WRITEMASK_ZW),
                  ureg_imm1f(shader, 0.0f));
         ureg_TEX(shader, tmp, TGSI_TEXTURE_2D, ureg_src(tmp), sampler);
      } else {
         ureg_TEX(shader, tmp, TGSI_TEXTURE_2D, i_vtex, sampler);
      }
      ureg_MAD(shader, t_sum, ureg_src(tmp), ureg_imm1f(shader, matrix_values[i]),
               ureg_src(t_sum));
   }

   ureg_MOV(shader, o_fragment, ureg_src(t_sum));

   ureg_END(shader);

   return ureg_create_shader_and_destroy(shader, filter->pipe);
}

bool
vl_matrix_filter_init(struct vl_matrix_filter *filter, struct pipe_context *pipe,
                      unsigned video_width, unsigned video_height,
                      unsigned matrix_width, unsigned matrix_height,
                      const float *matrix_values)
{
   struct pipe_rasterizer_state rs_state;
   struct pipe_blend_state blend;
   struct pipe_sampler_state sampler;
   struct pipe_vertex_element ve;
   struct vertex2f *offsets;
   unsigned i, num_offsets = matrix_width * matrix_height;
   int x;
   int y;
   int size_x;
   int size_y;

   assert(filter && pipe);
   assert(video_width && video_height);
   assert(matrix_width && matrix_height);

   memset(filter, 0, sizeof(*filter));
   filter->pipe = pipe;

   memset(&rs_state, 0, sizeof(rs_state));
   rs_state.half_pixel_center = true;
   rs_state.bottom_edge_rule = true;
   rs_state.depth_clip_near = 1;
   rs_state.depth_clip_far = 1;

   filter->rs_state = pipe->create_rasterizer_state(pipe, &rs_state);
   if (!filter->rs_state)
      goto error_rs_state;

   memset(&blend, 0, sizeof blend);
   blend.rt[0].rgb_func = PIPE_BLEND_ADD;
   blend.rt[0].rgb_src_factor = PIPE_BLENDFACTOR_ONE;
   blend.rt[0].rgb_dst_factor = PIPE_BLENDFACTOR_ONE;
   blend.rt[0].alpha_func = PIPE_BLEND_ADD;
   blend.rt[0].alpha_src_factor = PIPE_BLENDFACTOR_ONE;
   blend.rt[0].alpha_dst_factor = PIPE_BLENDFACTOR_ONE;
   blend.logicop_func = PIPE_LOGICOP_CLEAR;
   blend.rt[0].colormask = PIPE_MASK_RGBA;
   filter->blend = pipe->create_blend_state(pipe, &blend);
   if (!filter->blend)
      goto error_blend;

   memset(&sampler, 0, sizeof(sampler));
   sampler.wrap_s = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
   sampler.wrap_t = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
   sampler.wrap_r = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
   sampler.min_img_filter = PIPE_TEX_FILTER_NEAREST;
   sampler.min_mip_filter = PIPE_TEX_MIPFILTER_NONE;
   sampler.mag_img_filter = PIPE_TEX_FILTER_NEAREST;
   sampler.compare_mode = PIPE_TEX_COMPARE_NONE;
   sampler.compare_func = PIPE_FUNC_ALWAYS;
   filter->sampler = pipe->create_sampler_state(pipe, &sampler);
   if (!filter->sampler)
      goto error_sampler;

   filter->quad = vl_vb_upload_quads(pipe);
   if(!filter->quad.buffer.resource)
      goto error_quad;

   memset(&ve, 0, sizeof(ve));
   ve.src_stride = sizeof(struct vertex2f);
   ve.src_offset = 0;
   ve.instance_divisor = 0;
   ve.vertex_buffer_index = 0;
   ve.src_format = PIPE_FORMAT_R32G32_FLOAT;
   filter->ves = pipe->create_vertex_elements_state(pipe, 1, &ve);
   if (!filter->ves)
      goto error_ves;

   offsets = MALLOC(sizeof(struct vertex2f) * num_offsets);
   if (!offsets)
      goto error_offsets;

   size_x = (matrix_width - 1) / 2;
   size_y = (matrix_height - 1) / 2;

   for (x = -size_x, i = 0; x <= size_x; x++)
      for (y = -size_y; y <= size_y; y++) {
         offsets[i].x = x;
         offsets[i].y = y;
         i++;
      }

   for (i = 0; i < num_offsets; ++i) {
      offsets[i].x /= video_width;
      offsets[i].y /= video_height;
   }

   filter->vs = create_vert_shader(filter);
   if (!filter->vs)
      goto error_vs;

   filter->fs = create_frag_shader(filter, num_offsets, offsets, matrix_values);
   if (!filter->fs)
      goto error_fs;

   FREE(offsets);
   return true;

error_fs:
   pipe->delete_vs_state(pipe, filter->vs);

error_vs:
   FREE(offsets);

error_offsets:
   pipe->delete_vertex_elements_state(pipe, filter->ves);

error_ves:
   pipe_resource_reference(&filter->quad.buffer.resource, NULL);

error_quad:
   pipe->delete_sampler_state(pipe, filter->sampler);

error_sampler:
   pipe->delete_blend_state(pipe, filter->blend);

error_blend:
   pipe->delete_rasterizer_state(pipe, filter->rs_state);

error_rs_state:
   return false;
}

void
vl_matrix_filter_cleanup(struct vl_matrix_filter *filter)
{
   assert(filter);

   filter->pipe->delete_sampler_state(filter->pipe, filter->sampler);
   filter->pipe->delete_blend_state(filter->pipe, filter->blend);
   filter->pipe->delete_rasterizer_state(filter->pipe, filter->rs_state);
   filter->pipe->delete_vertex_elements_state(filter->pipe, filter->ves);
   pipe_resource_reference(&filter->quad.buffer.resource, NULL);

   filter->pipe->delete_vs_state(filter->pipe, filter->vs);
   filter->pipe->delete_fs_state(filter->pipe, filter->fs);
}

void
vl_matrix_filter_render(struct vl_matrix_filter *filter,
                        struct pipe_sampler_view *src,
                        struct pipe_surface *dst)
{
   struct pipe_viewport_state viewport;
   struct pipe_framebuffer_state fb_state;

   assert(filter && src && dst);

   memset(&viewport, 0, sizeof(viewport));
   viewport.scale[0] = dst->width;
   viewport.scale[1] = dst->height;
   viewport.scale[2] = 1;
   viewport.swizzle_x = PIPE_VIEWPORT_SWIZZLE_POSITIVE_X;
   viewport.swizzle_y = PIPE_VIEWPORT_SWIZZLE_POSITIVE_Y;
   viewport.swizzle_z = PIPE_VIEWPORT_SWIZZLE_POSITIVE_Z;
   viewport.swizzle_w = PIPE_VIEWPORT_SWIZZLE_POSITIVE_W;

   memset(&fb_state, 0, sizeof(fb_state));
   fb_state.width = dst->width;
   fb_state.height = dst->height;
   fb_state.nr_cbufs = 1;
   fb_state.cbufs[0] = dst;

   filter->pipe->bind_rasterizer_state(filter->pipe, filter->rs_state);
   filter->pipe->bind_blend_state(filter->pipe, filter->blend);
   filter->pipe->bind_sampler_states(filter->pipe, PIPE_SHADER_FRAGMENT,
                                     0, 1, &filter->sampler);
   filter->pipe->set_sampler_views(filter->pipe, PIPE_SHADER_FRAGMENT,
                                   0, 1, 0, false, &src);
   filter->pipe->bind_vs_state(filter->pipe, filter->vs);
   filter->pipe->bind_fs_state(filter->pipe, filter->fs);
   filter->pipe->set_framebuffer_state(filter->pipe, &fb_state);
   filter->pipe->set_viewport_states(filter->pipe, 0, 1, &viewport);
   filter->pipe->set_vertex_buffers(filter->pipe, 1, 0, false, &filter->quad);
   filter->pipe->bind_vertex_elements_state(filter->pipe, filter->ves);

   util_draw_arrays(filter->pipe, MESA_PRIM_QUADS, 0, 4);
}
