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

/**
 * Private data structures, etc for the draw module.
 */


/**
 * Authors:
 * Keith Whitwell <keithw@vmware.com>
 * Brian Paul
 */


#ifndef DRAW_PRIVATE_H
#define DRAW_PRIVATE_H


#include "pipe/p_state.h"
#include "pipe/p_defines.h"
#include "pipe/p_shader_tokens.h"

#include "draw_vertex_header.h"

#if DRAW_LLVM_AVAILABLE
struct gallivm_state;
#endif

/**
 * The max stage the draw stores resources for.
 * i.e. vs, tcs, tes, gs. no fs/cs/ms/ts.
 */
#define DRAW_MAX_SHADER_STAGE (PIPE_SHADER_GEOMETRY + 1)

/**
 * The largest possible index of a vertex that can be fetched.
 */
#define DRAW_MAX_FETCH_IDX 0xffffffff

/**
 * Maximum number of extra shader outputs.  These are allocated by:
 * - draw_pipe_aaline.c (1)
 * - draw_pipe_aapoint.c (1)
 * - draw_pipe_unfilled.c (1)
 * - draw_pipe_wide_point.c (up to 32)
 * - draw_prim_assembler.c (1)
 */
#define DRAW_MAX_EXTRA_SHADER_OUTPUTS 32

/**
 * Despite some efforts to determine the number of extra shader outputs ahead
 * of time, the matter of fact is that this number will vary as primitives
 * flow through the draw pipeline.  In particular, aaline/aapoint stages
 * only allocate their extra shader outputs on the first line/point.
 *
 * Consequently dup_vert() ends up copying vertices larger than those
 * allocated.
 *
 * Ideally we'd keep track of incoming/outgoing vertex sizes (and strides)
 * throughout the draw pipeline, but unfortunately we recompute these all over
 * the place, so preemptively expanding the vertex stride/size does not work
 * as mismatches ensue.
 *
 * As stopgap to prevent buffer read overflows, we allocate an extra bit of
 * padding at the end of temporary vertex buffers, allowing dup_vert() to copy
 * more vertex attributes than allocated.
 */
#define DRAW_EXTRA_VERTICES_PADDING \
   (DRAW_MAX_EXTRA_SHADER_OUTPUTS * sizeof(float[4]))

struct pipe_context;
struct draw_vertex_shader;
struct draw_stage;
struct draw_pt_front_end;
struct draw_assembler;
struct draw_llvm;
struct vbuf_render;
struct tgsi_exec_machine;
struct tgsi_sampler;
struct tgsi_image;
struct tgsi_buffer;
struct lp_cached_code;
struct draw_vertex_info;
struct draw_prim_info;

/**
 * Represents the mapped vertex buffer.
 */
struct draw_vertex_buffer {
   const void *map;
   uint32_t size;
};

/* NOTE: It should match vertex_id size above */
#define UNDEFINED_VERTEX_ID 0xffff


/* maximum number of shader variants we can cache */
#define DRAW_MAX_SHADER_VARIANTS 512

struct draw_buffer_info {
   const void *ptr;
   unsigned size;
};

/**
 * Private context for the drawing module.
 */
struct draw_context
{
   struct pipe_context *pipe;

   /** Drawing/primitive pipeline stages */
   struct {
      struct draw_stage *first;  /**< one of the following */

      struct draw_stage *validate;

      /* stages (in logical order) */
      struct draw_stage *flatshade;
      struct draw_stage *clip;
      struct draw_stage *cull;
      struct draw_stage *user_cull;
      struct draw_stage *twoside;
      struct draw_stage *offset;
      struct draw_stage *unfilled;
      struct draw_stage *stipple;
      struct draw_stage *aapoint;
      struct draw_stage *aaline;
      struct draw_stage *pstipple;
      struct draw_stage *wide_line;
      struct draw_stage *wide_point;
      struct draw_stage *rasterize;

      float wide_point_threshold; /**< convert pnts to tris if larger than this */
      float wide_line_threshold;  /**< convert lines to tris if wider than this */
      bool wide_point_sprites; /**< convert points to tris for sprite mode */
      bool line_stipple;       /**< do line stipple? */
      bool point_sprite;       /**< convert points to quads for sprites? */

      /* Temporary storage while the pipeline is being run:
       */
      char *verts;
      unsigned vertex_stride;
      unsigned vertex_count;
   } pipeline;

   struct vbuf_render *render;

   /* Support prototype passthrough path:
    */
   struct {
      /* Current active frontend */
      struct draw_pt_front_end *frontend;
      enum mesa_prim prim;
      uint8_t vertices_per_patch;
      bool rebind_parameters;

      unsigned opt;     /**< bitmask of PT_x flags */
      unsigned eltSize; /* saved eltSize for flushing */
      unsigned viewid; /* saved viewid for flushing */

      struct {
         struct draw_pt_middle_end *fetch_shade_emit;
         struct draw_pt_middle_end *general;
         struct draw_pt_middle_end *llvm;
         struct draw_pt_middle_end *mesh;
      } middle;

      struct {
         struct draw_pt_front_end *vsplit;
      } front;

      struct pipe_vertex_buffer vertex_buffer[PIPE_MAX_ATTRIBS];
      unsigned nr_vertex_buffers;

      /*
       * This is the largest legal index value for the current set of
       * bound vertex buffers.  Regardless of any other consideration,
       * all vertex lookups need to be clamped to 0..max_index to
       * prevent out-of-bound access.
       */
      unsigned max_index;

      unsigned vertex_strides[PIPE_MAX_ATTRIBS];
      struct pipe_vertex_element vertex_element[PIPE_MAX_ATTRIBS];
      unsigned nr_vertex_elements;

      bool test_fse;         /* enable FSE even though its not correct (eg for softpipe) */
      bool no_fse;           /* disable FSE even when it is correct */

      /* user-space vertex data, buffers */
      struct {
         /** vertex element/index buffer (ex: glDrawElements) */
         const void *elts;
         /** bytes per index (0, 1, 2 or 4) */
         unsigned eltSizeIB;
         unsigned eltSize;
         unsigned eltMax;
         int eltBias;
         unsigned min_index;
         unsigned max_index;
         unsigned drawid;
         bool increment_draw_id;
         unsigned viewid;

         /** vertex arrays */
         struct draw_vertex_buffer vbuffer[PIPE_MAX_ATTRIBS];

         /** constant buffers for each shader stage */
         struct draw_buffer_info constants[DRAW_MAX_SHADER_STAGE][PIPE_MAX_CONSTANT_BUFFERS];
         struct draw_buffer_info ssbos[DRAW_MAX_SHADER_STAGE][PIPE_MAX_SHADER_BUFFERS];

         /* pointer to planes */
         float (*planes)[DRAW_TOTAL_CLIP_PLANES][4];
      } user;
   } pt;

   struct {
      bool bypass_clip_xy;
      bool bypass_clip_z;
      bool guard_band_xy;
      bool bypass_clip_points_lines;
   } driver;

   bool quads_always_flatshade_last;

   bool flushing;         /**< debugging/sanity */
   bool suspend_flushing; /**< internally set */

   /* Flags set if API requires clipping in these planes and the
    * driver doesn't indicate that it can do it for us.
    */
   bool clip_xy;
   bool clip_z;
   bool clip_user;
   bool guard_band_xy;
   bool guard_band_points_lines_xy;

   bool dump_vs;
   bool identity_viewport;
   bool bypass_viewport;

   /** Depth format and bias related settings. */
   bool floating_point_depth;
   double mrd;  /**< minimum resolvable depth value, for polygon offset */

   /** Current rasterizer state given to us by the driver */
   const struct pipe_rasterizer_state *rasterizer;
   /** Driver CSO handle for the current rasterizer state */
   void *rast_handle;

   /** Rasterizer CSOs without culling/stipple/etc */
   void *rasterizer_no_cull[2][2][2];

   struct pipe_viewport_state viewports[PIPE_MAX_VIEWPORTS];

   /** Vertex shader state */
   struct {
      struct draw_vertex_shader *vertex_shader;
      unsigned num_vs_outputs;  /**< convenience, from vertex_shader */
      unsigned position_output;
      unsigned edgeflag_output;
      unsigned clipvertex_output;
      unsigned ccdistance_output[2];

      /** Fields for TGSI interpreter / execution */
      struct {
         struct tgsi_exec_machine *machine;

         struct tgsi_sampler *sampler;
         struct tgsi_image *image;
         struct tgsi_buffer *buffer;
      } tgsi;

      struct translate *fetch;
      struct translate_cache *fetch_cache;
      struct translate *emit;
      struct translate_cache *emit_cache;
   } vs;

   /** Geometry shader state */
   struct {
      struct draw_geometry_shader *geometry_shader;
      unsigned num_gs_outputs;  /**< convenience, from geometry_shader */
      unsigned position_output;
      unsigned clipvertex_output;

      /** Fields for TGSI interpreter / execution */
      struct {
         struct tgsi_exec_machine *machine;

         struct tgsi_sampler *sampler;
         struct tgsi_image *image;
         struct tgsi_buffer *buffer;
      } tgsi;
   } gs;

   /* Tessellation state */
   struct {
      struct draw_tess_ctrl_shader *tess_ctrl_shader;
   } tcs;

   struct {
      struct draw_tess_eval_shader *tess_eval_shader;
      unsigned num_tes_outputs;  /**< convenience, from tess_eval_shader */
      unsigned position_output;
      unsigned clipvertex_output;
   } tes;

   /** Fragment shader state */
   struct {
      struct draw_fragment_shader *fragment_shader;
   } fs;

   struct {
      struct draw_mesh_shader *mesh_shader;
      unsigned num_ms_outputs;  /**< convenience, from geometry_shader */
      unsigned position_output;
      unsigned clipvertex_output;
   } ms;

   /** Stream output (vertex feedback) state */
   struct {
      struct draw_so_target *targets[PIPE_MAX_SO_BUFFERS];
      unsigned num_targets;
   } so;

   /* Clip derived state:
    */
   float plane[DRAW_TOTAL_CLIP_PLANES][4];

   /* If a prim stage introduces new vertex attributes, they'll be stored here
    */
   struct {
      unsigned num;
      enum tgsi_semantic semantic_name[DRAW_MAX_EXTRA_SHADER_OUTPUTS];
      unsigned semantic_index[DRAW_MAX_EXTRA_SHADER_OUTPUTS];
      unsigned slot[DRAW_MAX_EXTRA_SHADER_OUTPUTS];
   } extra_shader_outputs;

   unsigned instance_id;
   unsigned start_instance;
   unsigned start_index;
   unsigned constant_buffer_stride;
   struct draw_llvm *llvm;

   /** Texture sampler and sampler view state.
    * Note that we have arrays indexed by shader type.  At this time
    * we only handle vertex and geometry shaders in the draw module, but
    * there may be more in the future (ex: hull and tessellation).
    */
   struct pipe_sampler_view *sampler_views[DRAW_MAX_SHADER_STAGE][PIPE_MAX_SHADER_SAMPLER_VIEWS];
   unsigned num_sampler_views[DRAW_MAX_SHADER_STAGE];
   const struct pipe_sampler_state *samplers[DRAW_MAX_SHADER_STAGE][PIPE_MAX_SAMPLERS];
   unsigned num_samplers[DRAW_MAX_SHADER_STAGE];

   struct pipe_image_view *images[DRAW_MAX_SHADER_STAGE][PIPE_MAX_SHADER_IMAGES];
   unsigned num_images[DRAW_MAX_SHADER_STAGE];

   struct pipe_query_data_pipeline_statistics statistics;
   bool collect_statistics;
   bool collect_primgen;

   float default_outer_tess_level[4];
   float default_inner_tess_level[2];

   struct draw_assembler *ia;

   void *disk_cache_cookie;
   void (*disk_cache_find_shader)(void *cookie,
                                  struct lp_cached_code *cache,
                                  unsigned char ir_sha1_cache_key[20]);
   void (*disk_cache_insert_shader)(void *cookie,
                                    struct lp_cached_code *cache,
                                    unsigned char ir_sha1_cache_key[20]);

   void *driver_private;
};


struct draw_fetch_info {
   bool linear;
   unsigned start;
   const unsigned *elts;
   unsigned count;
};

/* these flags are set if the primitive is a segment of a larger one */
#define DRAW_SPLIT_BEFORE        0x1
#define DRAW_SPLIT_AFTER         0x2
#define DRAW_LINE_LOOP_AS_STRIP  0x4

/*******************************************************************************
 * Draw common initialization code
 */
bool draw_init(struct draw_context *draw);
void draw_new_instance(struct draw_context *draw);

/*******************************************************************************
 * Vertex shader code:
 */
bool draw_vs_init(struct draw_context *draw);
void draw_vs_destroy(struct draw_context *draw);


/*******************************************************************************
 * Geometry shading code:
 */
bool draw_gs_init(struct draw_context *draw);


void draw_gs_destroy(struct draw_context *draw);

/*******************************************************************************
 * Common shading code:
 */
unsigned draw_current_shader_outputs(const struct draw_context *draw);
unsigned draw_current_shader_position_output(const struct draw_context *draw);
unsigned draw_current_shader_viewport_index_output(const struct draw_context *draw);
unsigned draw_current_shader_clipvertex_output(const struct draw_context *draw);
unsigned draw_current_shader_ccdistance_output(const struct draw_context *draw, int index);
unsigned draw_current_shader_num_written_clipdistances(const struct draw_context *draw);
unsigned draw_current_shader_num_written_culldistances(const struct draw_context *draw);
int draw_alloc_extra_vertex_attrib(struct draw_context *draw,
                                   enum tgsi_semantic semantic_name,
                                   unsigned semantic_index);
void draw_remove_extra_vertex_attribs(struct draw_context *draw);
bool draw_current_shader_uses_viewport_index(
   const struct draw_context *draw);


/*******************************************************************************
 * Vertex processing (was passthrough) code:
 */
bool draw_pt_init(struct draw_context *draw);
void draw_pt_destroy(struct draw_context *draw);
void draw_pt_reset_vertex_ids(struct draw_context *draw);
void draw_pt_flush(struct draw_context *draw, unsigned flags);


/*******************************************************************************
 * Primitive processing (pipeline) code:
 */

bool draw_pipeline_init(struct draw_context *draw);
void draw_pipeline_destroy(struct draw_context *draw);

/*
 * These flags are used by the pipeline when unfilled and/or line stipple modes
 * are operational.
 */
#define DRAW_PIPE_EDGE_FLAG_0   0x1
#define DRAW_PIPE_EDGE_FLAG_1   0x2
#define DRAW_PIPE_EDGE_FLAG_2   0x4
#define DRAW_PIPE_EDGE_FLAG_ALL 0x7
#define DRAW_PIPE_RESET_STIPPLE 0x8

void
draw_pipeline_run(struct draw_context *draw,
                  const struct draw_vertex_info *vert,
                  const struct draw_prim_info *prim);

void
draw_pipeline_run_linear(struct draw_context *draw,
                         const struct draw_vertex_info *vert,
                         const struct draw_prim_info *prim);

void
draw_pipeline_flush(struct draw_context *draw,
                    unsigned flags);


/*
 * Flushing
 */

#define DRAW_FLUSH_PARAMETER_CHANGE 0x1  /**< Constants, viewport, etc */
#define DRAW_FLUSH_STATE_CHANGE     0x2  /**< Other/heavy state changes */
#define DRAW_FLUSH_BACKEND          0x4  /**< Flush the output buffer */


void
draw_do_flush(struct draw_context *draw, unsigned flags);

void *
draw_get_rasterizer_no_cull(struct draw_context *draw,
                             const struct pipe_rasterizer_state *rast);

void
draw_stats_clipper_primitives(struct draw_context *draw,
                              const struct draw_prim_info *prim_info);

void
draw_update_clip_flags(struct draw_context *draw);

void
draw_update_viewport_flags(struct draw_context *draw);


/**
 * Return index i from the index buffer.
 * If the index buffer would overflow we return index 0.
 */
#define DRAW_GET_IDX(_elts, _i)                   \
   (((_i) >= draw->pt.user.eltMax) ? 0 : (_elts)[_i])


/**
 * Return index of the given viewport clamping it
 * to be between 0 <= and < PIPE_MAX_VIEWPORTS
 */
static inline unsigned
draw_clamp_viewport_idx(int idx)
{
   return ((PIPE_MAX_VIEWPORTS > idx && idx >= 0) ? idx : 0);
}

#endif /* DRAW_PRIVATE_H */
