/**************************************************************************
 *
 * Copyright 2008 VMware, Inc.
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

#ifndef TR_DUMP_STATE_H_
#define TR_DUMP_STATE_H_

#include "pipe/p_state.h"
#include "pipe/p_shader_tokens.h"
#include "pipe/p_video_codec.h"

struct winsys_handle;

void trace_dump_resource_template(const struct pipe_resource *templat);

void trace_dump_video_codec_template(const struct pipe_video_codec *templat);

void trace_dump_video_buffer_template(const struct pipe_video_buffer *templat);

void trace_dump_pipe_picture_desc(const struct pipe_picture_desc *picture);

void trace_dump_pipe_vpp_blend(const struct pipe_vpp_blend *blend);

void trace_dump_pipe_vpp_desc(const struct pipe_vpp_desc *process_properties);

void trace_dump_box(const struct pipe_box *box);

void trace_dump_u_rect(const struct u_rect *rect);

void trace_dump_rasterizer_state(const struct pipe_rasterizer_state *state);

void trace_dump_poly_stipple(const struct pipe_poly_stipple *state);

void trace_dump_viewport_state(const struct pipe_viewport_state *state);

void trace_dump_scissor_state(const struct pipe_scissor_state *state);

void trace_dump_clip_state(const struct pipe_clip_state *state);

void trace_dump_token(const struct tgsi_token *token);

void trace_dump_shader_state(const struct pipe_shader_state *state);

void trace_dump_compute_state(const struct pipe_compute_state *state);

void trace_dump_depth_stencil_alpha_state(const struct pipe_depth_stencil_alpha_state *state);

void trace_dump_blend_state(const struct pipe_blend_state *state);

void trace_dump_blend_color(const struct pipe_blend_color *state);

void trace_dump_stencil_ref(const struct pipe_stencil_ref *state);

void trace_dump_framebuffer_state(const struct pipe_framebuffer_state *state);

void trace_dump_framebuffer_state_deep(const struct pipe_framebuffer_state *state);

void trace_dump_sampler_state(const struct pipe_sampler_state *state);

void trace_dump_sampler_view_template(const struct pipe_sampler_view *view);

void trace_dump_surface(const struct pipe_surface *surface);

void trace_dump_surface_template(const struct pipe_surface *state,
                                 enum pipe_texture_target target);

void trace_dump_transfer(const struct pipe_transfer *state);

void trace_dump_vertex_buffer(const struct pipe_vertex_buffer *state);

void trace_dump_vertex_element(const struct pipe_vertex_element *state);

void trace_dump_constant_buffer(const struct pipe_constant_buffer *state);

void trace_dump_shader_buffer(const struct pipe_shader_buffer *buffer);

void trace_dump_draw_info(const struct pipe_draw_info *state);

void trace_dump_draw_vertex_state_info(struct pipe_draw_vertex_state_info state);

void trace_dump_draw_start_count(const struct pipe_draw_start_count_bias *state);

void trace_dump_draw_indirect_info(const struct pipe_draw_indirect_info *state);

void trace_dump_blit_info(const struct pipe_blit_info *);

void trace_dump_query_result(unsigned query_type, unsigned index,
                             const union pipe_query_result *result);

void trace_dump_grid_info(const struct pipe_grid_info *state);

void trace_dump_image_view(const struct pipe_image_view *view);

void trace_dump_memory_info(const struct pipe_memory_info *state);

void trace_dump_winsys_handle(const struct winsys_handle *whandle);
#endif /* TR_STATE_H */
