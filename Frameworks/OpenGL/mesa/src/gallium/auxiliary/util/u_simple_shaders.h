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


#ifndef U_SIMPLE_SHADERS_H
#define U_SIMPLE_SHADERS_H


#include "util/compiler.h"
#include "pipe/p_shader_tokens.h"


struct pipe_context;
struct pipe_shader_state;
struct pipe_stream_output_info;


#ifdef __cplusplus
extern "C" {
#endif


extern void *
util_make_vertex_passthrough_shader(struct pipe_context *pipe,
                                    unsigned num_attribs,
                                    const enum tgsi_semantic *semantic_names,
                                    const unsigned *semantic_indexes,
                                    bool window_space);

extern void *
util_make_vertex_passthrough_shader_with_so(struct pipe_context *pipe,
                                    unsigned num_attribs,
                                    const enum tgsi_semantic *semantic_names,
                                    const unsigned *semantic_indexes,
                                    bool window_space, bool layered,
                                    const struct pipe_stream_output_info *so);

extern void *
util_make_layered_clear_vertex_shader(struct pipe_context *pipe);

extern void *
util_make_layered_clear_helper_vertex_shader(struct pipe_context *pipe);

extern void *
util_make_layered_clear_geometry_shader(struct pipe_context *pipe);

extern void *
util_make_fragment_tex_shader(struct pipe_context *pipe,
                              enum tgsi_texture_type tex_target,
                              enum tgsi_return_type stype,
                              enum tgsi_return_type dtype,
                              bool load_level_zero,
                              bool use_txf);

extern void *
util_make_fs_blit_zs(struct pipe_context *pipe, unsigned zs_mask,
                     enum tgsi_texture_type tex_target,
                     bool load_level_zero, bool use_txf);

extern void *
util_make_fragment_passthrough_shader(struct pipe_context *pipe,
                                      int input_semantic,
                                      int input_interpolate,
                                      bool write_all_cbufs);


extern void *
util_make_empty_fragment_shader(struct pipe_context *pipe);


extern void *
util_make_fragment_cloneinput_shader(struct pipe_context *pipe, int num_cbufs,
                                     int input_semantic,
                                     int input_interpolate);


extern void *
util_make_fs_blit_msaa_color(struct pipe_context *pipe,
                             enum tgsi_texture_type tgsi_tex,
                             enum tgsi_return_type stype,
                             enum tgsi_return_type dtype,
                             bool sample_shading, bool has_txq);


extern void *
util_make_fs_blit_msaa_depth(struct pipe_context *pipe,
                             enum tgsi_texture_type tgsi_tex,
                             bool sample_shading, bool has_txq);


extern void *
util_make_fs_blit_msaa_depthstencil(struct pipe_context *pipe,
                                    enum tgsi_texture_type tgsi_tex,
                                    bool sample_shading, bool has_txq);


void *
util_make_fs_blit_msaa_stencil(struct pipe_context *pipe,
                               enum tgsi_texture_type tgsi_tex,
                               bool sample_shading, bool has_txq);


void *
util_make_fs_msaa_resolve(struct pipe_context *pipe,
                          enum tgsi_texture_type tgsi_tex, unsigned nr_samples,
                          bool has_txq);


void *
util_make_fs_msaa_resolve_bilinear(struct pipe_context *pipe,
                                   enum tgsi_texture_type tgsi_tex,
                                   unsigned nr_samples, bool has_txq);

extern void *
util_make_geometry_passthrough_shader(struct pipe_context *pipe,
                                      unsigned num_attribs,
                                      const uint8_t *semantic_names,
                                      const uint8_t *semantic_indexes);

void *
util_make_fs_pack_color_zs(struct pipe_context *pipe,
                           enum tgsi_texture_type tex_target,
                           enum pipe_format zs_format,
                           bool dst_is_color);

extern void *
util_make_tess_ctrl_passthrough_shader(struct pipe_context *pipe,
                                       unsigned num_vs_outputs,
                                       unsigned num_tes_inputs,
                                       const uint8_t *vs_semantic_names,
                                       const uint8_t *vs_semantic_indexes,
                                       const uint8_t *tes_semantic_names,
                                       const uint8_t *tes_semantic_indexes,
                                       const unsigned vertices_per_patch);

void *
util_make_fs_stencil_blit(struct pipe_context *pipe, bool msaa_src, bool has_txq);

void *
util_make_fs_clear_all_cbufs(struct pipe_context *pipe);

#ifdef __cplusplus
}
#endif


#endif
