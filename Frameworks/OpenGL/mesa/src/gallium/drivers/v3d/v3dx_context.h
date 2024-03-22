/*
 * Copyright © 2014-2017 Broadcom
 * Copyright (C) 2012 Rob Clark <robclark@freedesktop.org>
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/* This file generates the per-v3d-version function prototypes.  It must only
 * be included from v3d_context.h.
 */

struct v3d_hw;
struct v3d_format;

void v3dX(start_binning)(struct v3d_context *v3d,
                        struct v3d_job *job);
void v3dX(emit_state)(struct pipe_context *pctx);
void v3dX(emit_rcl)(struct v3d_job *job);
void v3dX(draw_init)(struct pipe_context *pctx);
void v3dX(state_init)(struct pipe_context *pctx);
void v3dX(create_texture_shader_state_bo)(struct v3d_context *v3d,
                                          struct v3d_sampler_view *so);

void v3dX(bcl_epilogue)(struct v3d_context *v3d, struct v3d_job *job);

const struct v3d_format *v3dX(get_format_desc)(enum pipe_format f);
void v3dX(get_internal_type_bpp_for_output_format)(uint32_t format,
                                                   uint32_t *type,
                                                   uint32_t *bpp);

/* FIXME: tex_format should be `enum V3DX(Texture_Data_Formats)`, but using
 * that enum type in the header requires including v3dx_pack.h, which triggers
 * circular include dependencies issues, so we're using a `uint32_t` for now.
 */
bool v3dX(tfu_supports_tex_format)(uint32_t tex_format,
                                   bool for_mipmap);

bool v3dX(tfu)(struct pipe_context *pctx,
               struct pipe_resource *pdst,
               struct pipe_resource *psrc,
               unsigned int src_level,
               unsigned int base_level,
               unsigned int last_level,
               unsigned int src_layer,
               unsigned int dst_layer,
               bool for_mipmap);

int v3dX(get_driver_query_group_info_perfcnt)(struct v3d_screen *screen,
                                              unsigned index,
                                              struct pipe_driver_query_group_info *info);
int v3dX(get_driver_query_info_perfcnt)(struct v3d_screen *screen,
                                        unsigned index,
                                        struct pipe_driver_query_info *info);
struct pipe_query *v3dX(create_batch_query_perfcnt)(struct v3d_context *v3d,
                                                    unsigned num_queries,
                                                    unsigned *query_types);
