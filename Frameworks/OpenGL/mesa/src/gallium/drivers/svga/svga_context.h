/**********************************************************
 * Copyright 2008-2023 VMware, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 **********************************************************/

#ifndef SVGA_CONTEXT_H
#define SVGA_CONTEXT_H


#include "pipe/p_context.h"
#include "pipe/p_defines.h"
#include "pipe/p_state.h"

#include "util/os_time.h"

#include "util/u_blitter.h"
#include "util/list.h"

#include "svga_screen.h"
#include "svga_state.h"
#include "svga_winsys.h"
#include "svga_hw_reg.h"
#include "svga3d_shaderdefs.h"
#include "svga_image_view.h"
#include "svga_shader_buffer.h"
#include "svga_debug.h"

/** Non-GPU queries for gallium HUD */
enum svga_hud {
/* per-frame counters */
   SVGA_QUERY_NUM_DRAW_CALLS = PIPE_QUERY_DRIVER_SPECIFIC,
   SVGA_QUERY_NUM_FALLBACKS,
   SVGA_QUERY_NUM_FLUSHES,
   SVGA_QUERY_NUM_VALIDATIONS,
   SVGA_QUERY_MAP_BUFFER_TIME,
   SVGA_QUERY_NUM_BUFFERS_MAPPED,
   SVGA_QUERY_NUM_TEXTURES_MAPPED,
   SVGA_QUERY_NUM_BYTES_UPLOADED,
   SVGA_QUERY_NUM_COMMAND_BUFFERS,
   SVGA_QUERY_COMMAND_BUFFER_SIZE,
   SVGA_QUERY_FLUSH_TIME,
   SVGA_QUERY_SURFACE_WRITE_FLUSHES,
   SVGA_QUERY_NUM_READBACKS,
   SVGA_QUERY_NUM_RESOURCE_UPDATES,
   SVGA_QUERY_NUM_BUFFER_UPLOADS,
   SVGA_QUERY_NUM_CONST_BUF_UPDATES,
   SVGA_QUERY_NUM_CONST_UPDATES,
   SVGA_QUERY_NUM_SHADER_RELOCATIONS,
   SVGA_QUERY_NUM_SURFACE_RELOCATIONS,

/* running total counters */
   SVGA_QUERY_MEMORY_USED,
   SVGA_QUERY_NUM_SHADERS,
   SVGA_QUERY_NUM_RESOURCES,
   SVGA_QUERY_NUM_STATE_OBJECTS,
   SVGA_QUERY_NUM_SURFACE_VIEWS,
   SVGA_QUERY_NUM_GENERATE_MIPMAP,
   SVGA_QUERY_NUM_FAILED_ALLOCATIONS,
   SVGA_QUERY_NUM_COMMANDS_PER_DRAW,
   SVGA_QUERY_SHADER_MEM_USED,

/*SVGA_QUERY_MAX has to be last because it is size of an array*/
   SVGA_QUERY_MAX
};


/**
 * Maximum supported number of constant buffers per shader
 * including the zero slot for the default constant buffer.
 */
#define SVGA_MAX_CONST_BUFS 15
#define SVGA_MAX_RAW_BUFS   64

/**
 * Maximum constant buffer size that can be set in the
 * DXSetSingleConstantBuffer command is
 * DX10 constant buffer element count * 4 4-bytes components
 */
#define SVGA_MAX_CONST_BUF_SIZE (4096 * 4 * sizeof(int))

#define CONST0_UPLOAD_ALIGNMENT 256
#define SVGA_MAX_UAVIEWS        SVGA3D_DX11_1_MAX_UAVIEWS
#define SVGA_MAX_IMAGES         SVGA3D_MAX_UAVIEWS
#define SVGA_MAX_SHADER_BUFFERS	SVGA3D_MAX_UAVIEWS
#define SVGA_MAX_ATOMIC_BUFFERS	SVGA3D_MAX_UAVIEWS

enum svga_surface_state
{
   SVGA_SURFACE_STATE_CREATED,
   SVGA_SURFACE_STATE_INVALIDATED,
   SVGA_SURFACE_STATE_UPDATED,
   SVGA_SURFACE_STATE_RENDERED,
};

struct draw_vertex_shader;
struct draw_fragment_shader;
struct svga_shader_variant;
struct SVGACmdMemory;
struct util_bitmask;


struct svga_cache_context;
struct svga_tracked_state;

struct svga_blend_state {
   unsigned need_white_fragments:1;
   unsigned independent_blend_enable:1;
   unsigned alpha_to_coverage:1;
   unsigned alpha_to_one:1;
   unsigned blend_color_alpha:1;  /**< set blend color to alpha value */
   unsigned logicop_enabled:1;
   unsigned logicop_mode:5;

   /** Per-render target state */
   struct {
      uint8_t writemask;

      bool blend_enable;
      uint8_t srcblend;
      uint8_t dstblend;
      uint8_t blendeq;

      bool separate_alpha_blend_enable;
      uint8_t srcblend_alpha;
      uint8_t dstblend_alpha;
      uint8_t blendeq_alpha;
   } rt[PIPE_MAX_COLOR_BUFS];

   SVGA3dBlendStateId id;  /**< vgpu10 */
};

struct svga_depth_stencil_state {
   unsigned zfunc:8;
   unsigned zenable:1;
   unsigned zwriteenable:1;

   unsigned alphatestenable:1;
   unsigned alphafunc:8;

   struct {
      unsigned enabled:1;
      unsigned func:8;
      unsigned fail:8;
      unsigned zfail:8;
      unsigned pass:8;
   } stencil[2];

   /* SVGA3D has one ref/mask/writemask triple shared between front &
    * back face stencil.  We really need two:
    */
   unsigned stencil_mask:8;
   unsigned stencil_writemask:8;

   float    alpharef;

   SVGA3dDepthStencilStateId id;  /**< vgpu10 */
};

#define SVGA_UNFILLED_DISABLE 0
#define SVGA_UNFILLED_LINE    1
#define SVGA_UNFILLED_POINT   2

#define SVGA_PIPELINE_FLAG_POINTS   (1<<MESA_PRIM_POINTS)
#define SVGA_PIPELINE_FLAG_LINES    (1<<MESA_PRIM_LINES)
#define SVGA_PIPELINE_FLAG_TRIS     (1<<MESA_PRIM_TRIANGLES)

#define SVGA_MAX_FRAMEBUFFER_DEFAULT_SAMPLES 4

struct svga_rasterizer_state {
   struct pipe_rasterizer_state templ; /* needed for draw module */

   unsigned shademode:8;
   unsigned cullmode:8;
   unsigned scissortestenable:1;
   unsigned multisampleantialias:1;
   unsigned antialiasedlineenable:1;
   unsigned lastpixel:1;
   unsigned pointsprite:1;

   unsigned linepattern;

   float slopescaledepthbias;
   float depthbias;
   float pointsize;
   float linewidth;

   unsigned hw_fillmode:2;         /* PIPE_POLYGON_MODE_x */

   /** Which prims do we need help for?  Bitmask of (1 << MESA_PRIM_x) flags */
   unsigned need_pipeline:16;

   SVGA3dRasterizerStateId id;    /**< vgpu10 */

   /* Alternate SVGA rasterizer state object with forcedSampleCount */
   int altRastIds[SVGA_MAX_FRAMEBUFFER_DEFAULT_SAMPLES+1];

   struct svga_rasterizer_state *no_cull_rasterizer;

   /** For debugging: */
   const char* need_pipeline_tris_str;
   const char* need_pipeline_lines_str;
   const char* need_pipeline_points_str;
};

struct svga_sampler_state {
   unsigned mipfilter;
   unsigned magfilter;
   unsigned minfilter;
   unsigned aniso_level;
   float lod_bias;
   unsigned addressu;
   unsigned addressv;
   unsigned addressw;
   unsigned bordercolor;
   unsigned normalized_coords:1;
   unsigned compare_mode:1;
   unsigned compare_func:3;

   unsigned min_lod;
   unsigned view_min_lod;
   unsigned view_max_lod;

   SVGA3dSamplerId id[2];
};


struct svga_pipe_sampler_view
{
   struct pipe_sampler_view base;

   SVGA3dShaderResourceViewId id;
};


static inline struct svga_pipe_sampler_view *
svga_pipe_sampler_view(struct pipe_sampler_view *v)
{
   return (struct svga_pipe_sampler_view *) v;
}


struct svga_velems_state {
   unsigned count;
   struct pipe_vertex_element velem[PIPE_MAX_ATTRIBS];
   SVGA3dDeclType decl_type[PIPE_MAX_ATTRIBS]; /**< vertex attrib formats */
   uint16_t strides[PIPE_MAX_ATTRIBS];

   /** Bitmasks indicating which attributes need format conversion */
   unsigned adjust_attrib_range;     /**< range adjustment */
   unsigned attrib_is_pure_int;      /**< pure int */
   unsigned adjust_attrib_w_1;       /**< set w = 1 */
   unsigned adjust_attrib_itof;      /**< int->float */
   unsigned adjust_attrib_utof;      /**< uint->float */
   unsigned attrib_is_bgra;          /**< R / B swizzling */
   unsigned attrib_puint_to_snorm;   /**< 10_10_10_2 packed uint -> snorm */
   unsigned attrib_puint_to_uscaled; /**< 10_10_10_2 packed uint -> uscaled */
   unsigned attrib_puint_to_sscaled; /**< 10_10_10_2 packed uint -> sscaled */

   bool need_swvfetch;

   SVGA3dElementLayoutId id; /**< VGPU10 */
};

struct svga_constant_buffer {
   struct svga_winsys_surface *handle;
   unsigned size;
};

struct svga_raw_buffer {
   struct svga_winsys_surface *handle;
   unsigned buffer_offset;
   unsigned buffer_size;
   struct pipe_resource *buffer;
   int32 srvid;
};

/* Use to calculate differences between state emitted to hardware and
 * current driver-calculated state.
 */
struct svga_state
{
   const struct svga_blend_state *blend;
   const struct svga_depth_stencil_state *depth;
   const struct svga_sampler_state *sampler[PIPE_SHADER_TYPES][PIPE_MAX_SAMPLERS];
   const struct svga_velems_state *velems;

   struct svga_rasterizer_state *rast;
   struct pipe_sampler_view *sampler_views[PIPE_SHADER_TYPES][PIPE_MAX_SAMPLERS]; /* or texture ID's? */
   struct svga_fragment_shader *fs;
   struct svga_vertex_shader *vs;
   struct svga_geometry_shader *user_gs; /* user-specified GS */
   struct svga_geometry_shader *gs;      /* derived GS */
   /* derived tessellation control shader */
   struct svga_tcs_shader *tcs;
   /* derived tessellation evaluation shader */
   struct svga_tes_shader *tes;
   struct svga_compute_shader *cs;

   struct pipe_vertex_buffer vb[PIPE_MAX_ATTRIBS];
   /** Constant buffers for each shader.
    * The size should probably always match with that of
    * svga_shader_emitter_v10.num_shader_consts.
    */
   struct pipe_constant_buffer constbufs[PIPE_SHADER_TYPES][SVGA_MAX_CONST_BUFS];
   struct svga_raw_buffer rawbufs[PIPE_SHADER_TYPES][SVGA_MAX_RAW_BUFS];

   struct pipe_framebuffer_state framebuffer;
   float depthscale;

   /* Hack to limit the number of different render targets between
    * flushes.  Helps avoid blowing out our surface cache in EXA.
    */
   int nr_fbs;

   struct pipe_poly_stipple poly_stipple;
   struct pipe_scissor_state scissor[SVGA3D_DX_MAX_VIEWPORTS];
   struct pipe_blend_color blend_color;
   struct pipe_stencil_ref stencil_ref;
   struct pipe_clip_state clip;
   struct pipe_viewport_state viewport[SVGA3D_DX_MAX_VIEWPORTS];

   bool use_samplers[PIPE_SHADER_TYPES];
   unsigned num_samplers[PIPE_SHADER_TYPES];
   unsigned num_sampler_views[PIPE_SHADER_TYPES];
   unsigned num_vertex_buffers;
   enum mesa_prim reduced_prim;

   unsigned vertex_id_bias;

   struct {
      unsigned flag_1d;
      unsigned flag_srgb;
   } tex_flags;

   unsigned sample_mask;
   unsigned vertices_per_patch;
   float default_tesslevels[6]; /* tessellation (outer[4] + inner[2]) levels */

   /* Image views */
   unsigned num_image_views[PIPE_SHADER_TYPES];
   struct svga_image_view image_views[PIPE_SHADER_TYPES][SVGA_MAX_IMAGES];

   /* Shader buffers */
   unsigned num_shader_buffers[PIPE_SHADER_TYPES];
   struct svga_shader_buffer shader_buffers[PIPE_SHADER_TYPES][SVGA_MAX_SHADER_BUFFERS];

   /* HW atomic buffers */
   unsigned num_atomic_buffers;
   struct svga_shader_buffer atomic_buffers[SVGA_MAX_SHADER_BUFFERS];

   struct {
      /* Determine the layout of the grid (in block units) to be used. */
      unsigned size[3];
      /* If DispatchIndirect is used, this will has grid size info*/
      struct pipe_resource *indirect;
   } grid_info;

};

struct svga_prescale {
   float translate[4];
   float scale[4];
   bool enabled;
};

struct svga_depthrange {
   float zmin;
   float zmax;
};

/* Updated by calling svga_update_state( SVGA_STATE_HW_CLEAR )
 */
struct svga_hw_clear_state
{
   struct pipe_framebuffer_state framebuffer;

   /* VGPU9 only */
   SVGA3dRect viewport;
   struct svga_depthrange depthrange;

   /* VGPU10 state */
   SVGA3dViewport viewports[SVGA3D_DX_MAX_VIEWPORTS];
   struct svga_prescale prescale[SVGA3D_DX_MAX_VIEWPORTS];
   struct pipe_scissor_state scissors[SVGA3D_DX_MAX_VIEWPORTS];
   unsigned num_prescale;

   unsigned num_rendertargets;
   struct pipe_surface *rtv[SVGA3D_MAX_RENDER_TARGETS];
   struct pipe_surface *dsv;
};

struct svga_hw_view_state
{
   struct pipe_resource *texture;
   struct svga_sampler_view *v;
   unsigned min_lod;
   unsigned max_lod;
   bool dirty;
};

/* Updated by calling svga_update_state( SVGA_STATE_HW_DRAW )
 */
struct svga_hw_draw_state
{
   /** VGPU9 rasterization state */
   unsigned rs[SVGA3D_RS_MAX];
   /** VGPU9 texture sampler and bindings state */
   unsigned ts[SVGA3D_PIXEL_SAMPLERREG_MAX][SVGA3D_TS_MAX];

   /** VGPU9 texture views */
   unsigned num_views;
   unsigned num_backed_views; /* views with backing copy of texture */
   struct svga_hw_view_state views[PIPE_MAX_SAMPLERS];

   /** VGPU9 constant buffer values */
   float cb[PIPE_SHADER_TYPES][SVGA3D_CONSTREG_MAX][4];

   /** Currently bound shaders */
   struct svga_shader_variant *fs;
   struct svga_shader_variant *vs;
   struct svga_shader_variant *gs;
   struct svga_shader_variant *tcs;
   struct svga_shader_variant *tes;
   struct svga_shader_variant *cs;

   /** Currently bound constant buffer, per shader stage */
   struct pipe_resource *constbuf[PIPE_SHADER_TYPES][SVGA_MAX_CONST_BUFS];
   struct svga_constant_buffer constbufoffsets[PIPE_SHADER_TYPES][SVGA_MAX_CONST_BUFS];
   struct svga_raw_buffer rawbufs[PIPE_SHADER_TYPES][SVGA_MAX_RAW_BUFS];
   uint64_t enabled_rawbufs[PIPE_SHADER_TYPES];

   /** Bitmask of enabled constant buffers */
   unsigned enabled_constbufs[PIPE_SHADER_TYPES];

   /**
    * These are used to reduce the number of times we call u_upload_unmap()
    * while updating the zero-th/default VGPU10 constant buffer.
    */
   struct pipe_resource *const0_buffer;
   struct svga_winsys_surface *const0_handle;

   /** VGPU10 HW state (used to prevent emitting redundant state) */
   SVGA3dDepthStencilStateId depth_stencil_id;
   unsigned stencil_ref;
   SVGA3dBlendStateId blend_id;
   float blend_factor[4];
   unsigned blend_sample_mask;
   SVGA3dRasterizerStateId rasterizer_id;
   SVGA3dElementLayoutId layout_id;
   SVGA3dPrimitiveType topology;

   /** Vertex buffer state */
   SVGA3dVertexBuffer_v2 vbuffer_attrs[PIPE_MAX_ATTRIBS];
   struct pipe_resource *vbuffers[PIPE_MAX_ATTRIBS];
   unsigned num_vbuffers;

   struct pipe_resource *ib;  /**< index buffer for drawing */
   SVGA3dSurfaceFormat ib_format;
   unsigned ib_offset;

   unsigned num_samplers[PIPE_SHADER_TYPES];
   SVGA3dSamplerId samplers[PIPE_SHADER_TYPES][PIPE_MAX_SAMPLERS];

   unsigned num_sampler_views[PIPE_SHADER_TYPES];
   struct pipe_sampler_view
      *sampler_views[PIPE_SHADER_TYPES][PIPE_MAX_SAMPLERS];

   /* used for rebinding */
   unsigned default_constbuf_size[PIPE_SHADER_TYPES];

   bool rasterizer_discard; /* set if rasterization is disabled */
   bool has_backed_views;   /* set if any of the rtv/dsv is a backed surface view */

   /* Image Views */
   int uavSpliceIndex;
   unsigned num_image_views[PIPE_SHADER_TYPES];
   struct svga_image_view image_views[PIPE_SHADER_TYPES][SVGA_MAX_IMAGES];

   /* Shader Buffers */
   unsigned num_shader_buffers[PIPE_SHADER_TYPES];
   struct svga_shader_buffer shader_buffers[PIPE_SHADER_TYPES][SVGA_MAX_SHADER_BUFFERS];
   uint64_t enabled_raw_shaderbufs[PIPE_SHADER_TYPES];

   /* HW Atomic Buffers */
   unsigned num_atomic_buffers;
   struct svga_shader_buffer atomic_buffers[SVGA_MAX_SHADER_BUFFERS];

   /* UAV state */
   unsigned num_uavs;
   SVGA3dUAViewId uaViewIds[SVGA_MAX_UAVIEWS];
   struct svga_winsys_surface *uaViews[SVGA_MAX_UAVIEWS];

   /* Compute UAV state */
   unsigned num_cs_uavs;
   SVGA3dUAViewId csUAViewIds[SVGA_MAX_UAVIEWS];
   struct svga_winsys_surface *csUAViews[SVGA_MAX_UAVIEWS];

   /* starting uav index for each shader */
   unsigned uav_start_index[PIPE_SHADER_TYPES];

   /* starting uav index for HW atomic buffers */
   unsigned uav_atomic_buf_index;
};


/* Updated by calling svga_update_state( SVGA_STATE_NEED_SWTNL )
 */
struct svga_sw_state
{
   /* which parts we need */
   bool need_swvfetch;
   bool need_pipeline;
   bool need_swtnl;

   /* Flag to make sure that need sw is on while
    * updating state within a swtnl call.
    */
   bool in_swtnl_draw;
};


/* Queue some state updates (like rss) and submit them to hardware in
 * a single packet.
 */
struct svga_hw_queue;

struct svga_query;
struct svga_qmem_alloc_entry;

enum svga_uav_type
{
   SVGA_IMAGE_VIEW = 0,
   SVGA_SHADER_BUFFER
};

struct svga_uav
{
   enum svga_uav_type type;
   union {
      struct svga_image_view image_view;
      struct svga_shader_buffer shader_buffer;
   } desc;
   struct pipe_resource *resource;
   unsigned next_uaView;
   SVGA3dUAViewId uaViewId;
   unsigned timestamp[2];
};

struct svga_cache_uav
{
   unsigned num_uaViews;
   unsigned next_uaView;
   struct svga_uav uaViews[SVGA3D_DX11_1_MAX_UAVIEWS];
};

struct svga_context
{
   struct pipe_context pipe;
   struct svga_winsys_context *swc;
   struct blitter_context *blitter;
   struct u_upload_mgr *const0_upload;
   struct u_upload_mgr *tex_upload;

   struct {
      bool no_swtnl;
      bool force_swtnl;
      bool use_min_mipmap;

      /* incremented for each shader */
      unsigned shader_id;

      bool no_line_width;
      bool force_hw_line_stipple;

      /** To report perf/conformance/etc issues to the gallium frontend */
      struct util_debug_callback callback;
   } debug;

   struct {
      struct draw_context *draw;
      struct vbuf_render *backend;
      unsigned hw_prim;
      bool new_vbuf;
      bool new_vdecl;
   } swtnl;

   /* Bitmask of blend state objects IDs */
   struct util_bitmask *blend_object_id_bm;

   /* Bitmask of depth/stencil state objects IDs */
   struct util_bitmask *ds_object_id_bm;

   /* Bitmask of input element object IDs */
   struct util_bitmask *input_element_object_id_bm;

   /* Bitmask of rasterizer object IDs */
   struct util_bitmask *rast_object_id_bm;

   /* Bitmask of sampler state objects IDs */
   struct util_bitmask *sampler_object_id_bm;

   /* Bitmask of sampler view IDs */
   struct util_bitmask *sampler_view_id_bm;

   /* Bitmask of to-free sampler view IDs created for raw buffer srv */
   struct util_bitmask *sampler_view_to_free_id_bm;

   /* Bitmask of used shader IDs */
   struct util_bitmask *shader_id_bm;

   /* Bitmask of used surface view IDs */
   struct util_bitmask *surface_view_id_bm;

   /* Bitmask of used stream output IDs */
   struct util_bitmask *stream_output_id_bm;

   /* Bitmask of used query IDs */
   struct util_bitmask *query_id_bm;

   /* Bitmask of used uav IDs */
   struct util_bitmask *uav_id_bm;

   /* Bitmask of to-free uav IDs */
   struct util_bitmask *uav_to_free_id_bm;

   struct {
      uint64_t dirty[SVGA_STATE_MAX];

      /** bitmasks of which const buffers are changed */
      unsigned dirty_constbufs[PIPE_SHADER_TYPES];

      /** bitmasks of which const buffers to be bound as srv raw buffers */
      unsigned raw_constbufs[PIPE_SHADER_TYPES];

      /** bitmasks of which shader buffers to be bound as srv raw buffers */
      uint64_t raw_shaderbufs[PIPE_SHADER_TYPES];

      unsigned texture_timestamp;
      unsigned uav_timestamp[2];

      struct svga_sw_state          sw;
      struct svga_hw_draw_state     hw_draw;
      struct svga_hw_clear_state    hw_clear;
   } state;

   struct svga_state curr;      /* state from the gallium frontend */
   uint64_t dirty;              /* statechanges since last update_state() */

   union {
      struct {
         unsigned rendertargets:1;
         unsigned texture_samplers:1;
         unsigned constbufs:1;
         unsigned vs:1;
         unsigned fs:1;
         unsigned gs:1;
         unsigned tcs:1;
         unsigned tes:1;
         unsigned cs:1;
         unsigned query:1;
         unsigned images:1;
         unsigned shaderbufs:1;
         unsigned atomicbufs:1;
         unsigned uav:1;
         unsigned indexbuf:1;
         unsigned vertexbufs:1;
      } flags;
      unsigned val;
   } rebind;

   struct svga_hwtnl *hwtnl;

   /** Queries states */
   struct svga_winsys_gb_query *gb_query;     /**< gb query object, one per context */
   unsigned gb_query_len;                     /**< gb query object size */
   struct util_bitmask *gb_query_alloc_mask;  /**< gb query object allocation mask */
   struct svga_qmem_alloc_entry *gb_query_map[SVGA_QUERY_MAX];
                                              /**< query mem block mapping */
   struct svga_query *sq[SVGA_QUERY_MAX+12];  /**< queries currently in progress */
                                              /* The last 12 entries are for streamout
                                               * queries for stream 0..3
                                               */

   /** List of buffers with queued transfers */
   struct list_head dirty_buffers;

   /** performance / info queries for HUD */
   struct {
      uint64_t num_draw_calls;          /**< SVGA_QUERY_DRAW_CALLS */
      uint64_t num_fallbacks;           /**< SVGA_QUERY_NUM_FALLBACKS */
      uint64_t num_flushes;             /**< SVGA_QUERY_NUM_FLUSHES */
      uint64_t num_validations;         /**< SVGA_QUERY_NUM_VALIDATIONS */
      uint64_t map_buffer_time;         /**< SVGA_QUERY_MAP_BUFFER_TIME */
      uint64_t num_buffers_mapped;      /**< SVGA_QUERY_NUM_BUFFERS_MAPPED */
      uint64_t num_textures_mapped;     /**< SVGA_QUERY_NUM_TEXTURES_MAPPED */
      uint64_t num_command_buffers;     /**< SVGA_QUERY_NUM_COMMAND_BUFFERS */
      uint64_t command_buffer_size;     /**< SVGA_QUERY_COMMAND_BUFFER_SIZE */
      uint64_t flush_time;              /**< SVGA_QUERY_FLUSH_TIME */
      uint64_t surface_write_flushes;   /**< SVGA_QUERY_SURFACE_WRITE_FLUSHES */
      uint64_t num_readbacks;           /**< SVGA_QUERY_NUM_READBACKS */
      uint64_t num_resource_updates;    /**< SVGA_QUERY_NUM_RESOURCE_UPDATES */
      uint64_t num_buffer_uploads;      /**< SVGA_QUERY_NUM_BUFFER_UPLOADS */
      uint64_t num_const_buf_updates;   /**< SVGA_QUERY_NUM_CONST_BUF_UPDATES */
      uint64_t num_const_updates;       /**< SVGA_QUERY_NUM_CONST_UPDATES */
      uint64_t num_shaders;             /**< SVGA_QUERY_NUM_SHADERS */

      /** The following are summed for SVGA_QUERY_NUM_STATE_OBJECTS */
      uint64_t num_blend_objects;
      uint64_t num_depthstencil_objects;
      uint64_t num_rasterizer_objects;
      uint64_t num_sampler_objects;
      uint64_t num_samplerview_objects;
      uint64_t num_vertexelement_objects;

      uint64_t num_surface_views;       /**< SVGA_QUERY_NUM_SURFACE_VIEWS */
      uint64_t num_bytes_uploaded;      /**< SVGA_QUERY_NUM_BYTES_UPLOADED */
      uint64_t num_generate_mipmap;     /**< SVGA_QUERY_NUM_GENERATE_MIPMAP */
      uint64_t shader_mem_used;         /**< SVGA_QUERY_SHADER_MEM_USED */

      bool uses_time;                /**< os_time_get() calls needed? */
   } hud;

   /** The currently bound stream output targets */
   bool in_streamout;                /* Set if streamout is active */
   unsigned num_so_targets;
   struct svga_winsys_surface *so_surfaces[SVGA3D_DX_MAX_SOTARGETS];
   struct pipe_stream_output_target *so_targets[SVGA3D_DX_MAX_SOTARGETS];
   struct svga_stream_output *current_so;

   /**
    * The following states are used in the workaround for auto draw with
    * stream instancing.
    */

   /* Last bound SO targets that can be used to get vertex count */
   struct pipe_stream_output_target *vcount_so_targets[SVGA3D_DX_MAX_SOTARGETS];
   unsigned vcount_buffer_stream;       /* SO buffer to stream index mask */
   struct pipe_query *so_queries[4];    /* SO stat queries for each stream */

   /** A blend state with blending disabled, for falling back to when blending
    * is illegal (e.g. an integer texture is bound)
    */
   struct svga_blend_state *noop_blend;

   struct {
      struct pipe_resource *texture;
      struct svga_pipe_sampler_view *sampler_view;
      void *sampler;
   } polygon_stipple;

   /** Depth stencil state created to disable depth stencil test */
   struct svga_depth_stencil_state *depthstencil_disable;

   /** Current conditional rendering predicate */
   struct {
      SVGA3dQueryId query_id;
      bool cond;
   } pred;

   bool render_condition;
   bool disable_rasterizer; /* Set if to disable rasterization */
   uint8_t patch_vertices;

   struct {
      struct svga_tcs_shader *passthrough_tcs;
      struct svga_vertex_shader *vs;
      struct svga_tes_shader *tes;
      unsigned vertices_per_patch;
      bool passthrough;
   } tcs;

   struct svga_cache_uav cache_uav;
   struct pipe_resource *dummy_resource;
};

/* A flag for each frontend state object:
 */
#define SVGA_NEW_BLEND               ((uint64_t) 0x1)
#define SVGA_NEW_DEPTH_STENCIL_ALPHA ((uint64_t) 0x2)
#define SVGA_NEW_RAST                ((uint64_t) 0x4)
#define SVGA_NEW_SAMPLER             ((uint64_t) 0x8)
#define SVGA_NEW_TEXTURE             ((uint64_t) 0x10)
#define SVGA_NEW_VBUFFER             ((uint64_t) 0x20)
#define SVGA_NEW_VELEMENT            ((uint64_t) 0x40)
#define SVGA_NEW_FS                  ((uint64_t) 0x80)
#define SVGA_NEW_VS                  ((uint64_t) 0x100)
#define SVGA_NEW_FS_CONST_BUFFER     ((uint64_t) 0x200)
#define SVGA_NEW_VS_CONST_BUFFER     ((uint64_t) 0x400)
#define SVGA_NEW_FRAME_BUFFER        ((uint64_t) 0x800)
#define SVGA_NEW_STIPPLE             ((uint64_t) 0x1000)
#define SVGA_NEW_SCISSOR             ((uint64_t) 0x2000)
#define SVGA_NEW_BLEND_COLOR         ((uint64_t) 0x4000)
#define SVGA_NEW_CLIP                ((uint64_t) 0x8000)
#define SVGA_NEW_VIEWPORT            ((uint64_t) 0x10000)
#define SVGA_NEW_PRESCALE            ((uint64_t) 0x20000)
#define SVGA_NEW_REDUCED_PRIMITIVE   ((uint64_t) 0x40000)
#define SVGA_NEW_TEXTURE_BINDING     ((uint64_t) 0x80000)
#define SVGA_NEW_NEED_PIPELINE       ((uint64_t) 0x100000)
#define SVGA_NEW_NEED_SWVFETCH       ((uint64_t) 0x200000)
#define SVGA_NEW_NEED_SWTNL          ((uint64_t) 0x400000)
#define SVGA_NEW_FS_VARIANT          ((uint64_t) 0x800000)
#define SVGA_NEW_VS_VARIANT          ((uint64_t) 0x1000000)
#define SVGA_NEW_TEXTURE_FLAGS       ((uint64_t) 0x4000000)
#define SVGA_NEW_STENCIL_REF         ((uint64_t) 0x8000000)
#define SVGA_NEW_GS                  ((uint64_t) 0x10000000)
#define SVGA_NEW_GS_CONST_BUFFER     ((uint64_t) 0x20000000)
#define SVGA_NEW_GS_VARIANT          ((uint64_t) 0x40000000)
#define SVGA_NEW_TEXTURE_CONSTS      ((uint64_t) 0x80000000)
#define SVGA_NEW_TCS                 ((uint64_t) 0x100000000)
#define SVGA_NEW_TES                 ((uint64_t) 0x200000000)
#define SVGA_NEW_TCS_VARIANT         ((uint64_t) 0x400000000)
#define SVGA_NEW_TES_VARIANT         ((uint64_t) 0x800000000)
#define SVGA_NEW_TCS_CONST_BUFFER    ((uint64_t) 0x1000000000)
#define SVGA_NEW_TES_CONST_BUFFER    ((uint64_t) 0x2000000000)
#define SVGA_NEW_TCS_PARAM           ((uint64_t) 0x4000000000)
#define SVGA_NEW_IMAGE_VIEW          ((uint64_t) 0x8000000000)
#define SVGA_NEW_SHADER_BUFFER       ((uint64_t) 0x10000000000)
#define SVGA_NEW_CS                  ((uint64_t) 0x20000000000)
#define SVGA_NEW_CS_VARIANT          ((uint64_t) 0x40000000000)
#define SVGA_NEW_CS_CONST_BUFFER     ((uint64_t) 0x80000000000)
#define SVGA_NEW_FS_CONSTS           ((uint64_t) 0x100000000000)
#define SVGA_NEW_VS_CONSTS           ((uint64_t) 0x200000000000)
#define SVGA_NEW_GS_CONSTS           ((uint64_t) 0x400000000000)
#define SVGA_NEW_TCS_CONSTS          ((uint64_t) 0x800000000000)
#define SVGA_NEW_TES_CONSTS          ((uint64_t) 0x1000000000000)
#define SVGA_NEW_CS_CONSTS           ((uint64_t) 0x2000000000000)
#define SVGA_NEW_FS_RAW_BUFFER       ((uint64_t) 0x4000000000000)
#define SVGA_NEW_VS_RAW_BUFFER       ((uint64_t) 0x8000000000000)
#define SVGA_NEW_GS_RAW_BUFFER       ((uint64_t) 0x10000000000000)
#define SVGA_NEW_TCS_RAW_BUFFER      ((uint64_t) 0x20000000000000)
#define SVGA_NEW_TES_RAW_BUFFER      ((uint64_t) 0x40000000000000)
#define SVGA_NEW_CS_RAW_BUFFER       ((uint64_t) 0x80000000000000)
#define SVGA_NEW_ALL                 ((uint64_t) 0xFFFFFFFFFFFFFFFF)

#define SVGA_NEW_CONST_BUFFER \
   (SVGA_NEW_FS_CONST_BUFFER | SVGA_NEW_VS_CONST_BUFFER | \
    SVGA_NEW_GS_CONST_BUFFER | SVGA_NEW_CS_CONST_BUFFER | \
    SVGA_NEW_TCS_CONST_BUFFER | SVGA_NEW_TES_CONST_BUFFER)


/** Program pipelines */
enum svga_pipe_type
{
   SVGA_PIPE_GRAPHICS = 0,
   SVGA_PIPE_COMPUTE  = 1
};

void svga_init_state_functions( struct svga_context *svga );
void svga_init_flush_functions( struct svga_context *svga );
void svga_init_string_functions( struct svga_context *svga );
void svga_init_blit_functions(struct svga_context *svga);

void svga_init_blend_functions( struct svga_context *svga );
void svga_init_depth_stencil_functions( struct svga_context *svga );
void svga_init_misc_functions( struct svga_context *svga );
void svga_init_rasterizer_functions( struct svga_context *svga );
void svga_init_sampler_functions( struct svga_context *svga );
void svga_init_cs_functions( struct svga_context *svga );
void svga_init_fs_functions( struct svga_context *svga );
void svga_init_vs_functions( struct svga_context *svga );
void svga_init_gs_functions( struct svga_context *svga );
void svga_init_ts_functions( struct svga_context *svga );
void svga_init_vertex_functions( struct svga_context *svga );
void svga_init_constbuffer_functions( struct svga_context *svga );
void svga_init_draw_functions( struct svga_context *svga );
void svga_init_query_functions( struct svga_context *svga );
void svga_init_surface_functions(struct svga_context *svga);
void svga_init_stream_output_functions( struct svga_context *svga );
void svga_init_clear_functions( struct svga_context *svga );
void svga_init_shader_image_functions( struct svga_context *svga );

void svga_cleanup_vertex_state( struct svga_context *svga );
void svga_cleanup_sampler_state( struct svga_context *svga );
void svga_cleanup_tss_binding( struct svga_context *svga );
void svga_cleanup_framebuffer( struct svga_context *svga );
void svga_cleanup_tcs_state( struct svga_context *svga );

void svga_context_flush( struct svga_context *svga,
                         struct pipe_fence_handle **pfence );

void svga_context_finish(struct svga_context *svga);

void svga_hwtnl_flush_retry( struct svga_context *svga );
void svga_hwtnl_flush_buffer( struct svga_context *svga,
                              struct pipe_resource *buffer );
bool svga_hwtnl_has_pending_prim(struct svga_hwtnl *);

void svga_surfaces_flush(struct svga_context *svga);

struct pipe_context *
svga_context_create(struct pipe_screen *screen,
                    void *priv, unsigned flags);

void svga_toggle_render_condition(struct svga_context *svga,
                                  bool render_condition_enabled,
                                  bool on);

int svga_define_rasterizer_object(struct svga_context *svga,
                                  struct svga_rasterizer_state *,
                                  unsigned samples);

enum pipe_error
svga_validate_sampler_resources(struct svga_context *svga,
                                enum svga_pipe_type);

enum pipe_error
svga_validate_constant_buffers(struct svga_context *svga,
                               enum svga_pipe_type);

enum pipe_error
svga_validate_image_views(struct svga_context *svga,
                          enum svga_pipe_type);

enum pipe_error
svga_validate_shader_buffers(struct svga_context *svga,
                             enum svga_pipe_type);

void
svga_destroy_rawbuf_srv(struct svga_context *svga);

void
svga_uav_cache_init(struct svga_context *svga);

void
svga_destroy_rawbuf_srv(struct svga_context *svga);


/***********************************************************************
 * Inline conversion functions.  These are better-typed than the
 * macros used previously:
 */
static inline struct svga_context *
svga_context( struct pipe_context *pipe )
{
   return (struct svga_context *)pipe;
}

static inline struct svga_winsys_screen *
svga_sws(struct svga_context *svga)
{
   return svga_screen(svga->pipe.screen)->sws;
}

static inline bool
svga_have_gb_objects(const struct svga_context *svga)
{
   return svga_screen(svga->pipe.screen)->sws->have_gb_objects;
}

static inline bool
svga_have_gb_dma(const struct svga_context *svga)
{
   return svga_screen(svga->pipe.screen)->sws->have_gb_dma;
}

static inline bool
svga_have_vgpu10(const struct svga_context *svga)
{
   return svga_screen(svga->pipe.screen)->sws->have_vgpu10;
}

static inline bool
svga_have_sm4_1(const struct svga_context *svga)
{
   return svga_screen(svga->pipe.screen)->sws->have_sm4_1;
}

static inline bool
svga_have_sm5(const struct svga_context *svga)
{
   return svga_screen(svga->pipe.screen)->sws->have_sm5;
}

static inline bool
svga_have_gl43(const struct svga_context *svga)
{
   return svga_screen(svga->pipe.screen)->sws->have_gl43;
}

static inline bool
svga_need_to_rebind_resources(const struct svga_context *svga)
{
   return svga_screen(svga->pipe.screen)->sws->need_to_rebind_resources;
}

static inline bool
svga_rects_equal(const SVGA3dRect *r1, const SVGA3dRect *r2)
{
   return memcmp(r1, r2, sizeof(*r1)) == 0;
}


/* A helper function to return TRUE if sampler state mapping is
 * to be used. Sampler state mapping is used in GL43 context
 * if the number of sampler states exceeds the SVGA device limit or
 * the sampler state mapping environment variable is set.
 */
static inline bool
svga_use_sampler_state_mapping(const struct svga_context *svga,
                               unsigned num_sampler_states)
{
   return svga_have_gl43(svga) &&
          (svga_screen(svga->pipe.screen)->debug.sampler_state_mapping ||
           num_sampler_states > SVGA3D_DX_MAX_SAMPLERS);
}


static inline void
svga_set_curr_shader_use_samplers_flag(struct svga_context *svga,
                                       enum pipe_shader_type shader_type,
                                       bool use_samplers)
{
   svga->curr.use_samplers[shader_type] = use_samplers;
}


static inline bool
svga_curr_shader_use_samplers(const struct svga_context *svga,
	                      enum pipe_shader_type shader_type)
{
   return svga->curr.use_samplers[shader_type];
}


/**
 * If the Gallium HUD is enabled, this will return the current time.
 * Otherwise, just return zero.
 */
static inline int64_t
svga_get_time(struct svga_context *svga)
{
   return svga->hud.uses_time ? os_time_get() : 0;
}

/*
 * The SVGA_TRY_XX family of macros can be used to optionally replace a
 * function call with an error value, the purpose is to trigger and test
 * retry path handling.
 */
#ifdef DEBUG

/*
 * Optionally replace a function call with a PIPE_ERROR_OUT_OF_MEMORY
 * return value
 */
#define SVGA_TRY(_func) \
   ((SVGA_DEBUG & DEBUG_RETRY) ? PIPE_ERROR_OUT_OF_MEMORY : (_func))

/* Optionally replace a function call with a NULL return value */
#define SVGA_TRY_PTR(_func) \
   ((SVGA_DEBUG & DEBUG_RETRY) ? NULL : (_func))

/*
 * Optionally replace a function call with a NULL return value, and set
 * the _retry parameter to TRUE.
 */
#define SVGA_TRY_MAP(_func, _retry) \
   ((SVGA_DEBUG & DEBUG_RETRY) ? (_retry) = true, NULL : (_func))
#else

#define SVGA_TRY(_func) (_func)

#define SVGA_TRY_PTR(_func) (_func)

#define SVGA_TRY_MAP(_func, _retry) (_func)
#endif

/**
 * Enter retry processing after hitting out-of-command space
 */
static inline void
svga_retry_enter(struct svga_context *svga)
{
   /* We shouldn't nest retries, but currently we do. */
   if ((SVGA_DEBUG & DEBUG_RETRY) && svga->swc->in_retry) {
      debug_printf("WARNING: Recursive retry. Level: %u.\n",
                   svga->swc->in_retry);
   }
   svga->swc->in_retry++;
}

/**
 * Exit retry processing after hitting out-of-command space
 */
static inline void
svga_retry_exit(struct svga_context *svga)
{
   assert(svga->swc->in_retry > 0);
   svga->swc->in_retry--;
}

/**
 * Perform a function call, and on failure flush the context and retry,
 * asserting that the retry succeeded. On return, the boolean argument
 * _retried indicates whether the function call was retried or not.
 */
#define SVGA_RETRY_CHECK(_svga, _func, _retried)       \
   do {                                                \
      enum pipe_error ret;                             \
                                                       \
      ret = SVGA_TRY(_func);                           \
      (_retried) = (ret != PIPE_OK);                   \
      if (_retried) {                                  \
         svga_retry_enter(_svga);                      \
         svga_context_flush(_svga, NULL);              \
         ret = (_func);                                \
         assert(ret == PIPE_OK);                       \
         svga_retry_exit(_svga);                       \
      }                                                \
   } while(0)

/**
 * Perform a function call, and on failure flush the context and retry,
 * asserting that the retry succeeded.
 */
#define SVGA_RETRY(_svga, _func)                \
   do {                                         \
      UNUSED bool retried;                      \
                                                \
      SVGA_RETRY_CHECK(_svga, _func, retried);  \
   } while(0)

/**
 * Perform a function call, and on out-of-memory, flush the context and
 * retry. The retry return value is stored in _ret for reuse.
 */
#define SVGA_RETRY_OOM(_svga, _ret, _func)              \
   do {                                                 \
      (_ret) = SVGA_TRY(_func);                         \
      if ((_ret) == PIPE_ERROR_OUT_OF_MEMORY) {         \
         svga_retry_enter(_svga);                       \
         svga_context_flush(_svga, NULL);               \
         (_ret) = (_func);                              \
         svga_retry_exit(_svga);                        \
      }                                                 \
   } while (0);

#endif
