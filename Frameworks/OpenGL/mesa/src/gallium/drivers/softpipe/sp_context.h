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

#ifndef SP_CONTEXT_H
#define SP_CONTEXT_H

#include "pipe/p_context.h"
#include "util/u_blitter.h"

#include "draw/draw_vertex.h"

#include "sp_quad_pipe.h"
#include "sp_setup.h"

#include "tgsi/tgsi_exec.h"

struct softpipe_vbuf_render;
struct draw_context;
struct draw_stage;
struct softpipe_tile_cache;
struct softpipe_tex_tile_cache;
struct sp_fragment_shader;
struct sp_vertex_shader;
struct sp_velems_state;
struct sp_so_state;

struct softpipe_context {
   struct pipe_context pipe;  /**< base class */

   /** Constant state objects */
   struct pipe_blend_state *blend;
   struct pipe_sampler_state *samplers[PIPE_SHADER_TYPES][PIPE_MAX_SAMPLERS];
   struct pipe_depth_stencil_alpha_state *depth_stencil;
   struct pipe_rasterizer_state *rasterizer;
   struct sp_fragment_shader *fs;
   struct sp_fragment_shader_variant *fs_variant;
   struct sp_vertex_shader *vs;
   struct sp_geometry_shader *gs;
   struct sp_velems_state *velems;
   struct sp_so_state *so;
   struct sp_compute_shader *cs;

   /** Other rendering state */
   struct pipe_blend_color blend_color;
   struct pipe_blend_color blend_color_clamped;
   struct pipe_stencil_ref stencil_ref;
   struct pipe_clip_state clip;
   struct pipe_resource *constants[PIPE_SHADER_TYPES][PIPE_MAX_CONSTANT_BUFFERS];
   struct pipe_framebuffer_state framebuffer;
   struct pipe_scissor_state scissors[PIPE_MAX_VIEWPORTS];
   struct pipe_sampler_view *sampler_views[PIPE_SHADER_TYPES][PIPE_MAX_SHADER_SAMPLER_VIEWS];

   struct pipe_image_view images[PIPE_SHADER_TYPES][PIPE_MAX_SHADER_IMAGES];
   struct pipe_shader_buffer buffers[PIPE_SHADER_TYPES][PIPE_MAX_SHADER_BUFFERS];
   struct pipe_viewport_state viewports[PIPE_MAX_VIEWPORTS];
   struct pipe_vertex_buffer vertex_buffer[PIPE_MAX_ATTRIBS];
   struct pipe_resource *mapped_vs_tex[PIPE_MAX_SHADER_SAMPLER_VIEWS];
   struct pipe_resource *mapped_gs_tex[PIPE_MAX_SHADER_SAMPLER_VIEWS];

   struct draw_so_target *so_targets[PIPE_MAX_SO_BUFFERS];
   unsigned num_so_targets;
   
   struct pipe_query_data_so_statistics so_stats[PIPE_MAX_VERTEX_STREAMS];

   struct pipe_query_data_pipeline_statistics pipeline_statistics;
   unsigned active_statistics_queries;

   unsigned num_samplers[PIPE_SHADER_TYPES];
   unsigned num_sampler_views[PIPE_SHADER_TYPES];

   unsigned num_vertex_buffers;

   unsigned dirty; /**< Mask of SP_NEW_x flags */

   /* Counter for occlusion queries.  Note this supports overlapping
    * queries.
    */
   uint64_t occlusion_count;
   unsigned active_query_count;

   /** Mapped vertex buffers */
   uint8_t *mapped_vbuffer[PIPE_MAX_ATTRIBS];

   /** Mapped constant buffers */
   struct tgsi_exec_consts_info mapped_constants[PIPE_SHADER_TYPES][PIPE_MAX_CONSTANT_BUFFERS];

   /** Vertex format */
   struct sp_setup_info setup_info;
   struct vertex_info vertex_info;

   /** Which vertex shader output slot contains point size */
   int8_t psize_slot;

   /** Which vertex shader output slot contains viewport index */
   int8_t viewport_index_slot;

   /** Which vertex shader output slot contains layer */
   int8_t layer_slot;

   /** The reduced version of the primitive supplied by the gallium frontend */
   unsigned reduced_api_prim;

   /** Derived information about which winding orders to cull */
   unsigned cull_mode;

   /**
    * The reduced primitive after unfilled triangles, wide-line decomposition,
    * etc, are taken into account.  This is the primitive type that's actually
    * rasterized.
    */
   unsigned reduced_prim;

   /** Derived from scissor and surface bounds: */
   struct pipe_scissor_state cliprect[PIPE_MAX_VIEWPORTS];

   /** Conditional query object and mode */
   struct pipe_query *render_cond_query;
   enum pipe_render_cond_flag render_cond_mode;
   bool render_cond_cond;

   /** Software quad rendering pipeline */
   struct {
      struct quad_stage *shade;
      struct quad_stage *depth_test;
      struct quad_stage *blend;
      struct quad_stage *first; /**< points to one of the above stages */
   } quad;

   /** TGSI exec things */
   struct {
      struct sp_tgsi_sampler *sampler[PIPE_SHADER_TYPES];
      struct sp_tgsi_image *image[PIPE_SHADER_TYPES];
      struct sp_tgsi_buffer *buffer[PIPE_SHADER_TYPES];
   } tgsi;

   struct tgsi_exec_machine *fs_machine;
   /** whether early depth testing is enabled */
   bool early_depth;

   /** The primitive drawing context */
   struct draw_context *draw;

   /** Draw module backend */
   struct vbuf_render *vbuf_backend;
   struct draw_stage *vbuf;

   struct blitter_context *blitter;

   bool dirty_render_cache;

   struct softpipe_tile_cache *cbuf_cache[PIPE_MAX_COLOR_BUFS];
   struct softpipe_tile_cache *zsbuf_cache;

   unsigned tex_timestamp;

   /*
    * Texture caches for vertex, fragment, geometry stages.
    * Don't use PIPE_SHADER_TYPES here to avoid allocating unused memory
    * for compute shaders.
    * XXX wouldn't it make more sense for the tile cache to just be part
    * of sp_sampler_view?
    */
   struct softpipe_tex_tile_cache *tex_cache[PIPE_SHADER_TYPES][PIPE_MAX_SHADER_SAMPLER_VIEWS];
};


static inline struct softpipe_context *
softpipe_context( struct pipe_context *pipe )
{
   return (struct softpipe_context *)pipe;
}


struct pipe_context *
softpipe_create_context(struct pipe_screen *, void *priv, unsigned flags);

struct pipe_resource *
softpipe_user_buffer_create(struct pipe_screen *screen,
                            void *ptr,
                            unsigned bytes,
			    unsigned bind_flags);

#define SP_UNREFERENCED         0
#define SP_REFERENCED_FOR_READ  (1 << 0)
#define SP_REFERENCED_FOR_WRITE (1 << 1)

unsigned int
softpipe_is_resource_referenced( struct pipe_context *pipe,
                                 struct pipe_resource *texture,
                                 unsigned level, int layer);

#endif /* SP_CONTEXT_H */
