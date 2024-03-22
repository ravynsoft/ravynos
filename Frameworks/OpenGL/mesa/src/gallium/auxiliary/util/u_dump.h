/**************************************************************************
 * 
 * Copyright 2009 VMware, Inc.
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

/**
 * @file
 * Dump data in human/machine readable format.
 * 
 * @author Jose Fonseca <jfonseca@vmware.com>
 */

#ifndef U_DEBUG_DUMP_H_
#define U_DEBUG_DUMP_H_


#include "util/compiler.h"
#include "pipe/p_state.h"

#include <stdio.h>


#ifdef __cplusplus
extern "C" {
#endif


#define UTIL_DUMP_INVALID_NAME "<invalid>"


/*
 * p_defines.h
 */

const char *
util_str_blend_factor(unsigned value, bool shortened);

const char *
util_str_blend_func(unsigned value, bool shortened);

const char *
util_str_logicop(unsigned value, bool shortened);

const char *
util_str_func(unsigned value, bool shortened);

const char *
util_str_stencil_op(unsigned value, bool shortened);

const char *
util_str_tex_target(unsigned value, bool shortened);

const char *
util_str_tex_wrap(unsigned value, bool shortened);

const char *
util_str_tex_mipfilter(unsigned value, bool shortened);

const char *
util_str_tex_filter(unsigned value, bool shortened);

const char *
util_str_query_type(unsigned value, bool shortened);

const char *
util_str_query_value_type(unsigned value, bool shortened);

const char *
util_str_prim_mode(unsigned value, bool shortened);

void
util_dump_ns(FILE *f, uint64_t time);

void
util_dump_ptr(FILE *stream, const void *value);

void
util_dump_query_type(FILE *stream, unsigned value);

void
util_dump_query_value_type(FILE *stream, unsigned value);

void
util_dump_query_flags(FILE *stream, unsigned value);

void
util_dump_transfer_usage(FILE *stream, unsigned value);

/*
 * p_state.h, through a FILE
 */

void
util_dump_resource(FILE *stream, const struct pipe_resource *state);

void
util_dump_rasterizer_state(FILE *stream,
                           const struct pipe_rasterizer_state *state);

void
util_dump_poly_stipple(FILE *stream,
                       const struct pipe_poly_stipple *state);

void
util_dump_viewport_state(FILE *stream,
                         const struct pipe_viewport_state *state);

void
util_dump_scissor_state(FILE *stream,
                        const struct pipe_scissor_state *state);

void
util_dump_clip_state(FILE *stream,
                     const struct pipe_clip_state *state);

void
util_dump_stream_output_info(FILE *stream,
                             const struct pipe_stream_output_info *state);

void
util_dump_shader_state(FILE *stream,
                       const struct pipe_shader_state *state);

void
util_dump_depth_stencil_alpha_state(FILE *stream,
                                    const struct pipe_depth_stencil_alpha_state *state);

void
util_dump_rt_blend_state(FILE *stream,
                         const struct pipe_rt_blend_state *state);

void
util_dump_blend_state(FILE *stream,
                      const struct pipe_blend_state *state);

void
util_dump_blend_color(FILE *stream,
                      const struct pipe_blend_color *state);

void
util_dump_stencil_ref(FILE *stream,
                      const struct pipe_stencil_ref *state);

void
util_dump_framebuffer_state(FILE *stream,
                            const struct pipe_framebuffer_state *state);

void
util_dump_sampler_state(FILE *stream,
                        const struct pipe_sampler_state *state);

void
util_dump_surface(FILE *stream,
                  const struct pipe_surface *state);

void
util_dump_image_view(FILE *stream, const struct pipe_image_view *state);

void
util_dump_shader_buffer(FILE *stream, const struct pipe_shader_buffer *state);

void
util_dump_sampler_view(FILE *stream, const struct pipe_sampler_view *state);

void
util_dump_transfer(FILE *stream,
                   const struct pipe_transfer *state);

void
util_dump_constant_buffer(FILE *stream,
                          const struct pipe_constant_buffer *state);

void
util_dump_vertex_buffer(FILE *stream,
                        const struct pipe_vertex_buffer *state);

void
util_dump_vertex_element(FILE *stream,
                         const struct pipe_vertex_element *state);

void
util_dump_stream_output_target(FILE *stream,
                               const struct pipe_stream_output_target *state);

void
util_dump_draw_info(FILE *stream, const struct pipe_draw_info *state);

void
util_dump_draw_start_count_bias(FILE *stream, const struct pipe_draw_start_count_bias *state);

void
util_dump_draw_indirect_info(FILE *stream,
                             const struct pipe_draw_indirect_info *indirect);

void
util_dump_grid_info(FILE *stream, const struct pipe_grid_info *state);

void
util_dump_box(FILE *stream, const struct pipe_box *box);

void
util_dump_blit_info(FILE *stream, const struct pipe_blit_info *info);

/* FIXME: Move the other debug_dump_xxx functions out of u_debug.h into here. */


#ifdef __cplusplus
}
#endif

#endif /* U_DEBUG_H_ */
