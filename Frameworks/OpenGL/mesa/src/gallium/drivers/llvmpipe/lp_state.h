/**************************************************************************
 * 
 * Copyright 2007 VMware, Inc.
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

/* Authors:  Keith Whitwell <keithw@vmware.com>
 */

#ifndef LP_STATE_H
#define LP_STATE_H

#include "pipe/p_state.h"
#include "lp_jit.h"
#include "lp_state_fs.h"
#include "gallivm/lp_bld.h"


#define LP_NEW_VIEWPORT      0x1
#define LP_NEW_RASTERIZER    0x2
#define LP_NEW_FS            0x4
#define LP_NEW_BLEND         0x8
#define LP_NEW_CLIP          0x10
#define LP_NEW_SCISSOR       0x20
#define LP_NEW_STIPPLE       0x40
#define LP_NEW_FRAMEBUFFER   0x80
#define LP_NEW_DEPTH_STENCIL_ALPHA 0x100
#define LP_NEW_FS_CONSTANTS  0x200
#define LP_NEW_SAMPLER       0x400
#define LP_NEW_SAMPLER_VIEW  0x800
#define LP_NEW_VERTEX        0x1000
#define LP_NEW_VS            0x2000
#define LP_NEW_OCCLUSION_QUERY 0x4000
#define LP_NEW_BLEND_COLOR   0x8000
#define LP_NEW_GS            0x10000
#define LP_NEW_SO            0x20000
#define LP_NEW_SO_BUFFERS    0x40000
#define LP_NEW_FS_SSBOS      0x80000
#define LP_NEW_FS_IMAGES    0x100000
#define LP_NEW_TCS          0x200000
#define LP_NEW_TES          0x400000
#define LP_NEW_SAMPLE_MASK  0x800000
#define LP_NEW_TASK                0x1000000
#define LP_NEW_TASK_CONSTANTS      0x2000000
#define LP_NEW_TASK_SAMPLER        0x4000000
#define LP_NEW_TASK_SAMPLER_VIEW   0x8000000
#define LP_NEW_TASK_SSBOS         0x10000000
#define LP_NEW_TASK_IMAGES        0x20000000
#define LP_NEW_MESH               0x40000000
#define LP_NEW_MESH_CONSTANTS     0x80000000
#define LP_NEW_MESH_SAMPLER      0x100000000ULL
#define LP_NEW_MESH_SAMPLER_VIEW 0x200000000ULL
#define LP_NEW_MESH_SSBOS        0x400000000ULL
#define LP_NEW_MESH_IMAGES       0x800000000ULL

#define LP_CSNEW_CS 0x1
#define LP_CSNEW_CONSTANTS 0x2
#define LP_CSNEW_SAMPLER 0x4
#define LP_CSNEW_SAMPLER_VIEW 0x8
#define LP_CSNEW_SSBOS 0x10
#define LP_CSNEW_IMAGES 0x20

struct vertex_info;
struct pipe_context;
struct llvmpipe_context;



struct lp_geometry_shader {
   bool no_tokens;
   struct pipe_stream_output_info stream_output;
   struct draw_geometry_shader *dgs;
};

struct lp_tess_ctrl_shader {
   bool no_tokens;
   struct pipe_stream_output_info stream_output;
   struct draw_tess_ctrl_shader *dtcs;
};

struct lp_tess_eval_shader {
   bool no_tokens;
   struct pipe_stream_output_info stream_output;
   struct draw_tess_eval_shader *dtes;
};


/** Vertex element state */
struct lp_velems_state
{
   unsigned count;
   struct pipe_vertex_element velem[PIPE_MAX_ATTRIBS];
};

struct lp_so_state {
   struct pipe_stream_output_info base;
};


void
llvmpipe_set_framebuffer_state(struct pipe_context *,
                               const struct pipe_framebuffer_state *);

void
llvmpipe_update_fs(struct llvmpipe_context *lp);

void 
llvmpipe_update_setup(struct llvmpipe_context *lp);

void
llvmpipe_update_task_shader(struct llvmpipe_context *lp);

void
llvmpipe_update_mesh_shader(struct llvmpipe_context *lp);

void
llvmpipe_update_derived_clear(struct llvmpipe_context *llvmpipe);

void
llvmpipe_task_update_derived(struct llvmpipe_context *llvmpipe);

void
llvmpipe_mesh_update_derived(struct llvmpipe_context *llvmpipe);

void
llvmpipe_update_derived(struct llvmpipe_context *llvmpipe);

void
llvmpipe_init_sampler_funcs(struct llvmpipe_context *llvmpipe);

void
llvmpipe_init_blend_funcs(struct llvmpipe_context *llvmpipe);

void
llvmpipe_init_vertex_funcs(struct llvmpipe_context *llvmpipe);

void
llvmpipe_init_draw_funcs(struct llvmpipe_context *llvmpipe);

void
llvmpipe_init_compute_funcs(struct llvmpipe_context *llvmpipe);

void
llvmpipe_init_clip_funcs(struct llvmpipe_context *llvmpipe);

void
llvmpipe_init_fs_funcs(struct llvmpipe_context *llvmpipe);

void
llvmpipe_init_vs_funcs(struct llvmpipe_context *llvmpipe);

void
llvmpipe_init_gs_funcs(struct llvmpipe_context *llvmpipe);

void
llvmpipe_init_tess_funcs(struct llvmpipe_context *llvmpipe);

void
llvmpipe_init_task_funcs(struct llvmpipe_context *llvmpipe);

void
llvmpipe_init_mesh_funcs(struct llvmpipe_context *llvmpipe);

void
llvmpipe_init_rasterizer_funcs(struct llvmpipe_context *llvmpipe);

void
llvmpipe_init_so_funcs(struct llvmpipe_context *llvmpipe);

void
llvmpipe_prepare_vertex_sampling(struct llvmpipe_context *ctx,
                                 unsigned num,
                                 struct pipe_sampler_view **views);

void
llvmpipe_prepare_geometry_sampling(struct llvmpipe_context *ctx,
                                   unsigned num,
                                   struct pipe_sampler_view **views);

void
llvmpipe_prepare_tess_ctrl_sampling(struct llvmpipe_context *ctx,
                                    unsigned num,
                                    struct pipe_sampler_view **views);

void
llvmpipe_prepare_tess_eval_sampling(struct llvmpipe_context *ctx,
                                    unsigned num,
                                    struct pipe_sampler_view **views);
void
llvmpipe_cleanup_stage_sampling(struct llvmpipe_context *ctx,
                                enum pipe_shader_type stage);

void
llvmpipe_prepare_vertex_images(struct llvmpipe_context *lp,
                               unsigned num,
                               struct pipe_image_view *views);

void
llvmpipe_prepare_geometry_images(struct llvmpipe_context *lp,
                                 unsigned num,
                                 struct pipe_image_view *views);

void
llvmpipe_prepare_tess_ctrl_images(struct llvmpipe_context *lp,
                                  unsigned num,
                                  struct pipe_image_view *views);

void
llvmpipe_prepare_tess_eval_images(struct llvmpipe_context *lp,
                                  unsigned num,
                                  struct pipe_image_view *views);

void
llvmpipe_cleanup_stage_images(struct llvmpipe_context *ctx,
                              enum pipe_shader_type stage);

#endif
