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

#ifndef LP_CONTEXT_H
#define LP_CONTEXT_H

#include "pipe/p_context.h"

#include "draw/draw_vertex.h"
#include "util/u_blitter.h"

#include "lp_tex_sample.h"
#include "lp_jit.h"
#include "lp_texture_handle.h"
#include "lp_setup.h"
#include "lp_state_fs.h"
#include "lp_state_cs.h"
#include "lp_state_setup.h"


struct llvmpipe_vbuf_render;
struct draw_context;
struct draw_stage;
struct draw_vertex_shader;
struct lp_fragment_shader;
struct lp_compute_shader;
struct lp_blend_state;
struct lp_setup_context;
struct lp_setup_variant;
struct lp_velems_state;

struct llvmpipe_context {
   struct pipe_context pipe;  /**< base class */

   struct list_head list;
   /** Constant state objects */
   const struct pipe_blend_state *blend;
   struct pipe_sampler_state *samplers[PIPE_SHADER_MESH_TYPES][PIPE_MAX_SAMPLERS];

   const struct pipe_depth_stencil_alpha_state *depth_stencil;
   const struct pipe_rasterizer_state *rasterizer;
   struct lp_fragment_shader *fs;
   struct draw_vertex_shader *vs;
   const struct lp_geometry_shader *gs;
   const struct lp_tess_ctrl_shader *tcs;
   const struct lp_tess_eval_shader *tes;
   struct lp_compute_shader *cs;
   const struct lp_velems_state *velems;
   const struct lp_so_state *so;

   struct lp_compute_shader *tss;
   struct lp_compute_shader *mhs;

   struct lp_sampler_matrix sampler_matrix;

   /** Other rendering state */
   unsigned sample_mask;
   unsigned min_samples;
   struct pipe_blend_color blend_color;
   struct pipe_stencil_ref stencil_ref;
   struct pipe_clip_state clip;
   struct pipe_constant_buffer constants[PIPE_SHADER_MESH_TYPES][LP_MAX_TGSI_CONST_BUFFERS];
   struct pipe_framebuffer_state framebuffer;
   struct pipe_poly_stipple poly_stipple;
   struct pipe_scissor_state scissors[PIPE_MAX_VIEWPORTS];
   struct pipe_sampler_view *sampler_views[PIPE_SHADER_MESH_TYPES][PIPE_MAX_SHADER_SAMPLER_VIEWS];

   struct pipe_viewport_state viewports[PIPE_MAX_VIEWPORTS];
   struct pipe_vertex_buffer vertex_buffer[PIPE_MAX_ATTRIBS];

   struct pipe_shader_buffer ssbos[PIPE_SHADER_MESH_TYPES][LP_MAX_TGSI_SHADER_BUFFERS];
   struct pipe_image_view images[PIPE_SHADER_MESH_TYPES][LP_MAX_TGSI_SHADER_IMAGES];
   uint32_t fs_ssbo_write_mask;
   unsigned num_samplers[PIPE_SHADER_MESH_TYPES];
   unsigned num_sampler_views[PIPE_SHADER_MESH_TYPES];
   unsigned num_images[PIPE_SHADER_MESH_TYPES];

   unsigned num_vertex_buffers;

   struct draw_so_target *so_targets[PIPE_MAX_SO_BUFFERS];
   int num_so_targets;
   struct pipe_query_data_so_statistics so_stats[PIPE_MAX_VERTEX_STREAMS];

   struct pipe_query_data_pipeline_statistics pipeline_statistics;
   unsigned active_statistics_queries;

   unsigned active_occlusion_queries;

   unsigned active_primgen_queries;

   bool queries_disabled;

   uint64_t dirty; /**< Mask of LP_NEW_x flags */
   unsigned cs_dirty; /**< Mask of LP_CSNEW_x flags */
   /** Mapped vertex buffers */
   uint8_t *mapped_vbuffer[PIPE_MAX_ATTRIBS];

   /** Vertex format */
   struct vertex_info vertex_info;

   uint8_t patch_vertices;

   /** Which vertex shader output slot contains color */
   int8_t color_slot[2];

   /** Which vertex shader output slot contains bcolor */
   int8_t bcolor_slot[2];

   /** Which vertex shader output slot contains point size */
   int8_t psize_slot;

   /** Which vertex shader output slot contains viewport index */
   int8_t viewport_index_slot;

   /** Which geometry shader output slot contains layer */
   int8_t layer_slot;

   /** A fake frontface output for unfilled primitives */
   int8_t face_slot;

   /** Depth format and bias settings. */
   bool floating_point_depth;
   double mrd;   /**< minimum resolvable depth value, for polygon offset */

   /** The tiling engine */
   struct lp_setup_context *setup;
   struct lp_setup_variant setup_variant;

   /** The primitive drawing context */
   struct draw_context *draw;

   struct blitter_context *blitter;

   unsigned tex_timestamp;

   /** List of all fragment shader variants */
   struct lp_fs_variant_list_item fs_variants_list;
   unsigned nr_fs_variants;
   unsigned nr_fs_instrs;

   bool permit_linear_rasterizer;
   bool single_vp;

   struct lp_setup_variant_list_item setup_variants_list;
   unsigned nr_setup_variants;

   /** List of all compute shader variants */
   struct lp_cs_variant_list_item cs_variants_list;
   unsigned nr_cs_variants;
   unsigned nr_cs_instrs;
   struct lp_cs_context *csctx;

   struct lp_cs_context *task_ctx;
   struct lp_cs_context *mesh_ctx;

   /** Conditional query object and mode */
   struct pipe_query *render_cond_query;
   enum pipe_render_cond_flag render_cond_mode;
   bool render_cond_cond;

   /** VK render cond */
   struct llvmpipe_resource *render_cond_buffer;
   unsigned render_cond_offset;

   /** The LLVMContext to use for LLVM related work */
   LLVMContextRef context;

   int max_global_buffers;
   struct pipe_resource **global_buffers;

};


struct pipe_context *
llvmpipe_create_context(struct pipe_screen *screen, void *priv,
                        unsigned flags);


struct pipe_resource *
llvmpipe_user_buffer_create(struct pipe_screen *screen,
                            void *ptr,
                            unsigned bytes,
                            unsigned bind_flags);


static inline struct llvmpipe_context *
llvmpipe_context(struct pipe_context *pipe)
{
   return (struct llvmpipe_context *)pipe;
}

#endif /* LP_CONTEXT_H */

