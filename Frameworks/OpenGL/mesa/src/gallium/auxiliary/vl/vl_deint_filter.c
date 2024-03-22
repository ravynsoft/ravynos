/**************************************************************************
 *
 * Copyright 2013 Grigori Goronzy <greg@chown.ath.cx>.
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
 * IN NO EVENT SHALL TUNGSTEN GRAPHICS AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

/*
 *  References:
 *
 *  Lin, S. F., Chang, Y. L., & Chen, L. G. (2003).
 *  Motion adaptive interpolation with horizontal motion detection for deinterlacing.
 *  Consumer Electronics, IEEE Transactions on, 49(4), 1256-1265.
 *
 *  Pei-Yin, C. H. E. N., & Yao-Hsien, L. A. I. (2007).
 *  A low-complexity interpolation method for deinterlacing.
 *  IEICE transactions on information and systems, 90(2), 606-608.
 *
 */

#include <stdio.h>

#include "pipe/p_context.h"

#include "tgsi/tgsi_ureg.h"

#include "util/u_draw.h"
#include "util/u_memory.h"
#include "util/u_math.h"
#include "util/format/u_format.h"

#include "vl_types.h"
#include "vl_video_buffer.h"
#include "vl_vertex_buffers.h"
#include "vl_deint_filter.h"

enum VS_OUTPUT
{
   VS_O_VPOS = 0,
   VS_O_VTEX = 0
};

static void *
create_vert_shader(struct vl_deint_filter *filter)
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

static void *
create_copy_frag_shader(struct vl_deint_filter *filter, unsigned field,
                        struct vertex2f *sizes)
{
   struct ureg_program *shader;
   struct ureg_src i_vtex;
   struct ureg_src sampler;
   struct ureg_dst o_fragment;
   struct ureg_dst t_tex;

   shader = ureg_create(PIPE_SHADER_FRAGMENT);
   if (!shader) {
      return NULL;
   }
   t_tex = ureg_DECL_temporary(shader);

   i_vtex = ureg_DECL_fs_input(shader, TGSI_SEMANTIC_GENERIC, VS_O_VTEX, TGSI_INTERPOLATE_LINEAR);
   sampler = ureg_DECL_sampler(shader, 2);
   o_fragment = ureg_DECL_output(shader, TGSI_SEMANTIC_COLOR, 0);

   ureg_MOV(shader, t_tex, i_vtex);
   if (field) {
      if (filter->interleaved)
         ureg_ADD(shader, t_tex, ureg_src(t_tex),
                  ureg_imm4f(shader, 0, sizes->y * 0.5f, 0, 0));
      ureg_MOV(shader, ureg_writemask(t_tex, TGSI_WRITEMASK_ZW),
               ureg_imm4f(shader, 0, 0, 1.0f, 0));
   } else {
      if (filter->interleaved)
         ureg_ADD(shader, t_tex, ureg_src(t_tex),
                  ureg_imm4f(shader, 0, sizes->y * -0.5f, 0, 0));
      ureg_MOV(shader, ureg_writemask(t_tex, TGSI_WRITEMASK_ZW),
               ureg_imm1f(shader, 0));
   }

   ureg_TEX(shader, o_fragment, TGSI_TEXTURE_2D_ARRAY, ureg_src(t_tex), sampler);

   ureg_release_temporary(shader, t_tex);
   ureg_END(shader);

   return ureg_create_shader_and_destroy(shader, filter->pipe);
}

static void *
create_deint_frag_shader(struct vl_deint_filter *filter, unsigned field,
                         struct vertex2f *sizes, bool spatial_filter)
{
   struct ureg_program *shader;
   struct ureg_src i_vtex;
   struct ureg_src sampler_cur;
   struct ureg_src sampler_prevprev;
   struct ureg_src sampler_prev;
   struct ureg_src sampler_next;
   struct ureg_dst o_fragment;
   struct ureg_dst t_tex;
   struct ureg_dst t_comp_top, t_comp_bot;
   struct ureg_dst t_diff;
   struct ureg_dst t_a, t_b;
   struct ureg_dst t_weave, t_linear;

   shader = ureg_create(PIPE_SHADER_FRAGMENT);
   if (!shader) {
      return NULL;
   }

   t_tex = ureg_DECL_temporary(shader);
   t_comp_top = ureg_DECL_temporary(shader);
   t_comp_bot = ureg_DECL_temporary(shader);
   t_diff = ureg_DECL_temporary(shader);
   t_a = ureg_DECL_temporary(shader);
   t_b = ureg_DECL_temporary(shader);
   t_weave = ureg_DECL_temporary(shader);
   t_linear = ureg_DECL_temporary(shader);

   i_vtex = ureg_DECL_fs_input(shader, TGSI_SEMANTIC_GENERIC, VS_O_VTEX, TGSI_INTERPOLATE_LINEAR);
   sampler_prevprev = ureg_DECL_sampler(shader, 0);
   sampler_prev = ureg_DECL_sampler(shader, 1);
   sampler_cur = ureg_DECL_sampler(shader, 2);
   sampler_next = ureg_DECL_sampler(shader, 3);
   o_fragment = ureg_DECL_output(shader, TGSI_SEMANTIC_COLOR, 0);

   // we don't care about ZW interpolation (allows better optimization)
   ureg_MOV(shader, t_tex, i_vtex);
   ureg_MOV(shader, ureg_writemask(t_tex, TGSI_WRITEMASK_ZW),
            ureg_imm1f(shader, 0));

   // sample between texels for cheap lowpass
   ureg_ADD(shader, t_comp_top, ureg_src(t_tex),
            ureg_imm4f(shader, sizes->x * 0.5f, sizes->y * -0.5f, 0, 0));
   ureg_ADD(shader, t_comp_bot, ureg_src(t_tex),
            ureg_imm4f(shader, sizes->x * -0.5f, sizes->y * 0.5f, 1.0f, 0));

   if (field == 0) {
      /* interpolating top field -> current field is a bottom field */
      // cur vs prev2
      ureg_TEX(shader, t_a, TGSI_TEXTURE_2D_ARRAY, ureg_src(t_comp_bot), sampler_cur);
      ureg_TEX(shader, t_b, TGSI_TEXTURE_2D_ARRAY, ureg_src(t_comp_bot), sampler_prevprev);
      ureg_ADD(shader, ureg_writemask(t_diff, TGSI_WRITEMASK_X), ureg_src(t_a), ureg_negate(ureg_src(t_b)));
      // prev vs next
      ureg_TEX(shader, t_a, TGSI_TEXTURE_2D_ARRAY, ureg_src(t_comp_top), sampler_prev);
      ureg_TEX(shader, t_b, TGSI_TEXTURE_2D_ARRAY, ureg_src(t_comp_top), sampler_next);
      ureg_ADD(shader, ureg_writemask(t_diff, TGSI_WRITEMASK_Y), ureg_src(t_a), ureg_negate(ureg_src(t_b)));
   } else {
      /* interpolating bottom field -> current field is a top field */
      // cur vs prev2
      ureg_TEX(shader, t_a, TGSI_TEXTURE_2D_ARRAY, ureg_src(t_comp_top), sampler_cur);
      ureg_TEX(shader, t_b, TGSI_TEXTURE_2D_ARRAY, ureg_src(t_comp_top), sampler_prevprev);
      ureg_ADD(shader, ureg_writemask(t_diff, TGSI_WRITEMASK_X), ureg_src(t_a), ureg_negate(ureg_src(t_b)));
      // prev vs next
      ureg_TEX(shader, t_a, TGSI_TEXTURE_2D_ARRAY, ureg_src(t_comp_bot), sampler_prev);
      ureg_TEX(shader, t_b, TGSI_TEXTURE_2D_ARRAY, ureg_src(t_comp_bot), sampler_next);
      ureg_ADD(shader, ureg_writemask(t_diff, TGSI_WRITEMASK_Y), ureg_src(t_a), ureg_negate(ureg_src(t_b)));
   }

   // absolute maximum of differences
   ureg_MAX(shader, ureg_writemask(t_diff, TGSI_WRITEMASK_X), ureg_abs(ureg_src(t_diff)),
            ureg_scalar(ureg_abs(ureg_src(t_diff)), TGSI_SWIZZLE_Y));

   if (field == 0) {
      /* weave with prev top field */
      if (filter->interleaved)
         ureg_ADD(shader, t_tex, ureg_src(t_tex),
                  ureg_imm4f(shader, 0, sizes->y * -0.5f, 0, 0));
      ureg_TEX(shader, t_weave, TGSI_TEXTURE_2D_ARRAY, ureg_src(t_tex), sampler_prev);
      /* get linear interpolation from current bottom field */
      ureg_ADD(shader, t_comp_top, ureg_src(t_tex),
               ureg_imm4f(shader, 0, sizes->y * (filter->interleaved ? 1.0f : -1.0f), 1.0f, 0));
      ureg_TEX(shader, t_linear, TGSI_TEXTURE_2D_ARRAY, ureg_src(t_comp_top), sampler_cur);
   } else {
      /* weave with prev bottom field */
      if (filter->interleaved)
         ureg_ADD(shader, t_tex, ureg_src(t_tex),
                  ureg_imm4f(shader, 0, sizes->y * 0.5f, 0, 0));
      ureg_ADD(shader, t_comp_bot, ureg_src(t_tex), ureg_imm4f(shader, 0, 0, 1.0f, 0));
      ureg_TEX(shader, t_weave, TGSI_TEXTURE_2D_ARRAY, ureg_src(t_comp_bot), sampler_prev);
      /* get linear interpolation from current top field */
      ureg_ADD(shader, t_comp_bot, ureg_src(t_tex),
               ureg_imm4f(shader, 0, sizes->y * (filter->interleaved ? -1.0f : 1.0f), 0, 0));
      ureg_TEX(shader, t_linear, TGSI_TEXTURE_2D_ARRAY, ureg_src(t_comp_bot), sampler_cur);
   }

   // mix between weave and linear
   // fully weave if diff < 6 (0.02353), fully interpolate if diff > 14 (0.05490)
   ureg_ADD(shader, ureg_writemask(t_diff, TGSI_WRITEMASK_X), ureg_src(t_diff),
            ureg_imm4f(shader, -0.02353f, 0, 0, 0));
   ureg_MUL(shader, ureg_saturate(ureg_writemask(t_diff, TGSI_WRITEMASK_X)),
            ureg_src(t_diff), ureg_imm4f(shader, 31.8750f, 0, 0, 0));
   ureg_LRP(shader, ureg_writemask(t_tex, TGSI_WRITEMASK_X), ureg_src(t_diff),
            ureg_src(t_linear), ureg_src(t_weave));
   ureg_MOV(shader, o_fragment, ureg_scalar(ureg_src(t_tex), TGSI_SWIZZLE_X));

   ureg_release_temporary(shader, t_tex);
   ureg_release_temporary(shader, t_comp_top);
   ureg_release_temporary(shader, t_comp_bot);
   ureg_release_temporary(shader, t_diff);
   ureg_release_temporary(shader, t_a);
   ureg_release_temporary(shader, t_b);
   ureg_release_temporary(shader, t_weave);
   ureg_release_temporary(shader, t_linear);
   ureg_END(shader);

   return ureg_create_shader_and_destroy(shader, filter->pipe);
}

bool
vl_deint_filter_init(struct vl_deint_filter *filter, struct pipe_context *pipe,
                     unsigned video_width, unsigned video_height,
                     bool skip_chroma, bool spatial_filter, bool interleaved)
{
   struct pipe_rasterizer_state rs_state;
   struct pipe_blend_state blend;
   struct pipe_sampler_state sampler;
   struct pipe_vertex_element ve;
   struct vertex2f sizes;
   struct pipe_video_buffer templ;

   assert(filter && pipe);
   assert(video_width && video_height);

   memset(filter, 0, sizeof(*filter));
   filter->pipe = pipe;
   filter->skip_chroma = skip_chroma;
   filter->interleaved = interleaved;
   filter->video_width = video_width;
   filter->video_height = video_height;

   /* TODO: handle other than 4:2:0 subsampling */
   memset(&templ, 0, sizeof(templ));
   templ.buffer_format = pipe->screen->get_video_param
   (
      pipe->screen,
      PIPE_VIDEO_PROFILE_UNKNOWN,
      PIPE_VIDEO_ENTRYPOINT_PROCESSING,
      PIPE_VIDEO_CAP_PREFERED_FORMAT
   );
   templ.width = video_width;
   templ.height = video_height;
   templ.interlaced = true;
   filter->video_buffer = vl_video_buffer_create(pipe, &templ);
   if (!filter->video_buffer)
      goto error_video_buffer;

   memset(&rs_state, 0, sizeof(rs_state));
   rs_state.half_pixel_center = true;
   rs_state.bottom_edge_rule = true;
   rs_state.depth_clip_near = 1;
   rs_state.depth_clip_far = 1;

   filter->rs_state = pipe->create_rasterizer_state(pipe, &rs_state);
   if (!filter->rs_state)
      goto error_rs_state;

   memset(&blend, 0, sizeof blend);
   blend.rt[0].colormask = PIPE_MASK_R;
   filter->blend[0] = pipe->create_blend_state(pipe, &blend);
   if (!filter->blend[0])
      goto error_blendR;

   blend.rt[0].colormask = PIPE_MASK_G;
   filter->blend[1] = pipe->create_blend_state(pipe, &blend);
   if (!filter->blend[1])
      goto error_blendG;

   blend.rt[0].colormask = PIPE_MASK_B;
   filter->blend[2] = pipe->create_blend_state(pipe, &blend);
   if (!filter->blend[2])
      goto error_blendB;

   memset(&sampler, 0, sizeof(sampler));
   sampler.wrap_s = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
   sampler.wrap_t = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
   sampler.wrap_r = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
   sampler.min_img_filter = PIPE_TEX_FILTER_LINEAR;
   sampler.min_mip_filter = PIPE_TEX_MIPFILTER_NONE;
   sampler.mag_img_filter = PIPE_TEX_FILTER_LINEAR;
   filter->sampler[0] = pipe->create_sampler_state(pipe, &sampler);
   filter->sampler[1] = filter->sampler[2] = filter->sampler[3] = filter->sampler[0];
   if (!filter->sampler[0])
      goto error_sampler;

   filter->quad = vl_vb_upload_quads(pipe);
   if(!filter->quad.buffer.resource)
      goto error_quad;

   memset(&ve, 0, sizeof(ve));
   ve.src_offset = 0;
   ve.src_stride = sizeof(struct vertex2f);
   ve.instance_divisor = 0;
   ve.vertex_buffer_index = 0;
   ve.src_format = PIPE_FORMAT_R32G32_FLOAT;
   filter->ves = pipe->create_vertex_elements_state(pipe, 1, &ve);
   if (!filter->ves)
      goto error_ves;

   sizes.x = 1.0f / video_width;
   sizes.y = 1.0f / video_height;

   filter->vs = create_vert_shader(filter);
   if (!filter->vs)
      goto error_vs;

   filter->fs_copy_top = create_copy_frag_shader(filter, 0, &sizes);
   if (!filter->fs_copy_top)
      goto error_fs_copy_top;

   filter->fs_copy_bottom = create_copy_frag_shader(filter, 1, &sizes);
   if (!filter->fs_copy_bottom)
      goto error_fs_copy_bottom;

   filter->fs_deint_top = create_deint_frag_shader(filter, 0, &sizes, spatial_filter);
   if (!filter->fs_deint_top)
      goto error_fs_deint_top;

   filter->fs_deint_bottom = create_deint_frag_shader(filter, 1, &sizes, spatial_filter);
   if (!filter->fs_deint_bottom)
      goto error_fs_deint_bottom;

   return true;

error_fs_deint_bottom:
   pipe->delete_fs_state(pipe, filter->fs_deint_top);

error_fs_deint_top:
   pipe->delete_fs_state(pipe, filter->fs_copy_bottom);

error_fs_copy_bottom:
   pipe->delete_fs_state(pipe, filter->fs_copy_top);

error_fs_copy_top:
   pipe->delete_vs_state(pipe, filter->vs);

error_vs:
   pipe->delete_vertex_elements_state(pipe, filter->ves);

error_ves:
   pipe_resource_reference(&filter->quad.buffer.resource, NULL);

error_quad:
   pipe->delete_sampler_state(pipe, filter->sampler);

error_sampler:
   pipe->delete_blend_state(pipe, filter->blend[2]);

error_blendB:
   pipe->delete_blend_state(pipe, filter->blend[1]);

error_blendG:
   pipe->delete_blend_state(pipe, filter->blend[0]);

error_blendR:
   pipe->delete_rasterizer_state(pipe, filter->rs_state);

error_rs_state:
   filter->video_buffer->destroy(filter->video_buffer);

error_video_buffer:
   return false;
}

void
vl_deint_filter_cleanup(struct vl_deint_filter *filter)
{
   assert(filter);

   filter->pipe->delete_sampler_state(filter->pipe, filter->sampler[0]);
   filter->pipe->delete_blend_state(filter->pipe, filter->blend[0]);
   filter->pipe->delete_blend_state(filter->pipe, filter->blend[1]);
   filter->pipe->delete_blend_state(filter->pipe, filter->blend[2]);
   filter->pipe->delete_rasterizer_state(filter->pipe, filter->rs_state);
   filter->pipe->delete_vertex_elements_state(filter->pipe, filter->ves);
   pipe_resource_reference(&filter->quad.buffer.resource, NULL);

   filter->pipe->delete_vs_state(filter->pipe, filter->vs);
   filter->pipe->delete_fs_state(filter->pipe, filter->fs_copy_top);
   filter->pipe->delete_fs_state(filter->pipe, filter->fs_copy_bottom);
   filter->pipe->delete_fs_state(filter->pipe, filter->fs_deint_top);
   filter->pipe->delete_fs_state(filter->pipe, filter->fs_deint_bottom);

   filter->video_buffer->destroy(filter->video_buffer);
}

bool
vl_deint_filter_check_buffers(struct vl_deint_filter *filter,
                              struct pipe_video_buffer *prevprev,
                              struct pipe_video_buffer *prev,
                              struct pipe_video_buffer *cur,
                              struct pipe_video_buffer *next)
{
   int i;
   struct pipe_video_buffer *bufs[] = { prevprev, prev, cur, next };

   for (i = 0; i < 4; i++) {
      if (pipe_format_to_chroma_format(bufs[i]->buffer_format) != PIPE_VIDEO_CHROMA_FORMAT_420)
         return false;
      if (bufs[i]->width < filter->video_width ||
          bufs[i]->height < filter->video_height)
         return false;
      if (bufs[i]->interlaced != !filter->interleaved)
         return false;
   }

   return true;
}

void
vl_deint_filter_render(struct vl_deint_filter *filter,
                       struct pipe_video_buffer *prevprev,
                       struct pipe_video_buffer *prev,
                       struct pipe_video_buffer *cur,
                       struct pipe_video_buffer *next,
                       unsigned field)
{
   struct pipe_viewport_state viewport;
   struct pipe_framebuffer_state fb_state;
   struct pipe_sampler_view **cur_sv;
   struct pipe_sampler_view **prevprev_sv;
   struct pipe_sampler_view **prev_sv;
   struct pipe_sampler_view **next_sv;
   struct pipe_sampler_view *sampler_views[4];
   struct pipe_surface **dst_surfaces;
   const unsigned *plane_order;
   int i;
   unsigned j;

   assert(filter && prevprev && prev && cur && next && field <= 1);

   /* set up destination and source */
   dst_surfaces = filter->video_buffer->get_surfaces(filter->video_buffer);
   plane_order = vl_video_buffer_plane_order(filter->video_buffer->buffer_format);
   cur_sv = cur->get_sampler_view_components(cur);
   prevprev_sv = prevprev->get_sampler_view_components(prevprev);
   prev_sv = prev->get_sampler_view_components(prev);
   next_sv = next->get_sampler_view_components(next);

   /* set up pipe state */
   filter->pipe->bind_rasterizer_state(filter->pipe, filter->rs_state);
   filter->pipe->set_vertex_buffers(filter->pipe, 1, 0, false, &filter->quad);
   filter->pipe->bind_vertex_elements_state(filter->pipe, filter->ves);
   filter->pipe->bind_vs_state(filter->pipe, filter->vs);
   filter->pipe->bind_sampler_states(filter->pipe, PIPE_SHADER_FRAGMENT,
                                     0, 4, filter->sampler);

   /* prepare viewport */
   memset(&viewport, 0, sizeof(viewport));
   viewport.scale[2] = 1;
   viewport.swizzle_x = PIPE_VIEWPORT_SWIZZLE_POSITIVE_X;
   viewport.swizzle_y = PIPE_VIEWPORT_SWIZZLE_POSITIVE_Y;
   viewport.swizzle_z = PIPE_VIEWPORT_SWIZZLE_POSITIVE_Z;
   viewport.swizzle_w = PIPE_VIEWPORT_SWIZZLE_POSITIVE_W;

   /* prepare framebuffer */
   memset(&fb_state, 0, sizeof(fb_state));
   fb_state.nr_cbufs = 1;

   /* process each plane separately */
   for (i = 0, j = 0; i < VL_NUM_COMPONENTS; ++i) {
      struct pipe_surface *blit_surf = dst_surfaces[field];
      struct pipe_surface *dst_surf = dst_surfaces[1 - field];
      int k = plane_order[i];

      /* bind blend state for this component in the plane */
      filter->pipe->bind_blend_state(filter->pipe, filter->blend[j]);

      /* update render target state */
      viewport.scale[0] = blit_surf->texture->width0;
      viewport.scale[1] = blit_surf->texture->height0;
      fb_state.width = blit_surf->texture->width0;
      fb_state.height = blit_surf->texture->height0;

      /* update sampler view sources  */
      sampler_views[0] = prevprev_sv[k];
      sampler_views[1] = prev_sv[k];
      sampler_views[2] = cur_sv[k];
      sampler_views[3] = next_sv[k];
      filter->pipe->set_sampler_views(filter->pipe, PIPE_SHADER_FRAGMENT,
                                      0, 4, 0, false, sampler_views);

      /* blit current field */
      fb_state.cbufs[0] = blit_surf;
      filter->pipe->bind_fs_state(filter->pipe, field ? filter->fs_copy_bottom : filter->fs_copy_top);
      filter->pipe->set_framebuffer_state(filter->pipe, &fb_state);
      filter->pipe->set_viewport_states(filter->pipe, 0, 1, &viewport);
      util_draw_arrays(filter->pipe, MESA_PRIM_QUADS, 0, 4);

      /* blit or interpolate other field */
      fb_state.cbufs[0] = dst_surf;
      filter->pipe->set_framebuffer_state(filter->pipe, &fb_state);
      if (i > 0 && filter->skip_chroma) {
         util_draw_arrays(filter->pipe, MESA_PRIM_QUADS, 0, 4);
      } else {
         filter->pipe->bind_fs_state(filter->pipe, field ? filter->fs_deint_top : filter->fs_deint_bottom);
         util_draw_arrays(filter->pipe, MESA_PRIM_QUADS, 0, 4);
      }

      if (++j >= util_format_get_nr_components(dst_surf->format)) {
         dst_surfaces += 2;
         j = 0;
      }
   }
}
