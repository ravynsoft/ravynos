/**************************************************************************
 * 
 * Copyright 2003 VMware, Inc.
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

#ifndef ST_CONTEXT_H
#define ST_CONTEXT_H

#include "main/arrayobj.h"
#include "main/mtypes.h"
#include "frontend/api.h"
#include "main/fbobject.h"
#include "state_tracker/st_atom.h"
#include "util/u_helpers.h"
#include "util/u_inlines.h"
#include "util/list.h"
#include "vbo/vbo.h"
#include "util/list.h"
#include "cso_cache/cso_context.h"


#ifdef __cplusplus
extern "C" {
#endif


struct draw_context;
struct draw_stage;
struct gen_mipmap_state;
struct st_context;
struct st_program;
struct u_upload_mgr;

#define ST_L3_PINNING_DISABLED 0xffffffff

struct st_bitmap_cache
{
   /** Window pos to render the cached image */
   GLint xpos, ypos;
   /** Bounds of region used in window coords */
   GLint xmin, ymin, xmax, ymax;

   /** GL states */
   struct gl_program *fp;
   bool scissor_enabled;
   bool clamp_frag_color;
   GLfloat color[4];

   /** Bitmap's Z position */
   GLfloat zpos;

   struct pipe_resource *texture;
   struct pipe_transfer *trans;

   GLboolean empty;

   /** An I8 texture image: */
   uint8_t *buffer;
};

struct st_bound_handles
{
   unsigned num_handles;
   uint64_t *handles;
};


#define NUM_DRAWPIX_CACHE_ENTRIES 4

struct drawpix_cache_entry
{
   GLsizei width, height;
   GLenum format, type;
   const void *user_pointer;  /**< Last user 'pixels' pointer */
   void *image;               /**< Copy of the glDrawPixels image data */
   struct pipe_resource *texture;
   unsigned age;
};


/*
 * Node for a linked list of dead sampler views.
 */
struct st_zombie_sampler_view_node
{
   struct pipe_sampler_view *view;
   struct list_head node;
};


/*
 * Node for a linked list of dead shaders.
 */
struct st_zombie_shader_node
{
   void *shader;
   enum pipe_shader_type type;
   struct list_head node;
};


struct st_context
{
   struct gl_context *ctx;
   struct pipe_screen *screen;
   struct pipe_context *pipe;
   struct cso_context *cso_context;

   struct pipe_frontend_screen *frontend_screen; /* e.g. dri_screen */
   void *frontend_context; /* e.g. dri_context */

   struct draw_context *draw;  /**< For selection/feedback/rastpos only */
   struct draw_stage *feedback_stage;  /**< For GL_FEEDBACK rendermode */
   struct draw_stage *selection_stage;  /**< For GL_SELECT rendermode */
   struct draw_stage *rastpos_stage;  /**< For glRasterPos */

   unsigned pin_thread_counter; /* for L3 thread pinning on AMD Zen */

   GLboolean clamp_frag_color_in_shader;
   GLboolean clamp_vert_color_in_shader;
   bool has_stencil_export; /**< can do shader stencil export? */
   bool has_time_elapsed;
   bool has_etc1;
   bool has_etc2;
   bool transcode_etc;
   bool transcode_astc;
   bool has_astc_2d_ldr;
   bool has_astc_5x5_ldr;
   bool astc_void_extents_need_denorm_flush;
   bool has_s3tc;
   bool has_rgtc;
   bool has_latc;
   bool has_bptc;
   bool prefer_blit_based_texture_transfer;
   bool allow_compute_based_texture_transfer;
   bool force_compute_based_texture_transfer;
   bool force_specialized_compute_transfer;
   bool force_persample_in_shader;
   bool has_shareable_shaders;
   bool has_multi_draw_indirect;
   bool has_indirect_partial_stride;
   bool has_occlusion_query;
   bool has_single_pipe_stat;
   bool has_pipeline_stat;
   bool has_indep_blend_enable;
   bool has_indep_blend_func;
   bool can_dither;
   bool can_bind_const_buffer_as_vertex;
   bool lower_flatshade;
   bool lower_alpha_test;
   bool lower_point_size;
   bool lower_two_sided_color;
   bool lower_ucp;
   bool prefer_real_buffer_in_constbuf0;
   bool has_conditional_render;
   bool lower_rect_tex;

   /* There are consequences for drivers wanting to call st_finalize_nir
    * twice, once before shader caching and once after lowering for shader
    * variants. If shader variants use lowering passes that are not ready
    * for that, things can blow up.
    *
    * If this is true, st_finalize_nir and pipe_screen::finalize_nir will be
    * called before the result is stored in the shader cache. If lowering for
    * shader variants is invoked, the functions will be called again.
    */
   bool allow_st_finalize_nir_twice;

   /**
    * If a shader can be created when we get its source.
    * This means it has only 1 variant, not counting glBitmap and
    * glDrawPixels.
    */
   bool shader_has_one_variant[MESA_SHADER_STAGES];

   bool needs_texcoord_semantic;
   bool apply_texture_swizzle_to_border_color;
   bool use_format_with_border_color;
   bool alpha_border_color_is_not_w;
   bool emulate_gl_clamp;

   bool draw_needs_minmax_index;
   bool has_hw_atomics;

   bool validate_all_dirty_states;
   bool can_null_texture;

   /* driver supports scissored clears */
   bool can_scissor_clear;

   /* Some state is contained in constant objects.
    * Other state is just parameter values.
    */
   struct {
      struct pipe_blend_state               blend;
      struct pipe_depth_stencil_alpha_state depth_stencil;
      struct pipe_rasterizer_state          rasterizer;
      struct pipe_sampler_state vert_samplers[PIPE_MAX_SAMPLERS];
      struct pipe_sampler_state frag_samplers[PIPE_MAX_SAMPLERS];
      GLuint num_vert_samplers;
      GLuint num_frag_samplers;
      GLuint num_sampler_views[PIPE_SHADER_TYPES];
      unsigned num_images[PIPE_SHADER_TYPES];
      struct pipe_clip_state clip;
      unsigned constbuf0_enabled_shader_mask;
      unsigned fb_width;
      unsigned fb_height;
      unsigned fb_num_samples;
      unsigned fb_num_layers;
      unsigned fb_num_cb;
      unsigned num_viewports;
      struct pipe_scissor_state scissor[PIPE_MAX_VIEWPORTS];
      struct pipe_viewport_state viewport[PIPE_MAX_VIEWPORTS];
      struct {
         unsigned num;
         bool include;
         struct pipe_scissor_state rects[PIPE_MAX_WINDOW_RECTANGLES];
      } window_rects;

      GLuint poly_stipple[32];  /**< In OpenGL's bottom-to-top order */

      GLuint fb_orientation;

      bool enable_sample_locations;
      unsigned sample_locations_samples;
      uint8_t sample_locations[
         PIPE_MAX_SAMPLE_LOCATION_GRID_SIZE *
         PIPE_MAX_SAMPLE_LOCATION_GRID_SIZE * 32];
   } state;

   /** This masks out unused shader resources. Only valid in draw calls. */
   uint64_t active_states;

   /**
    * The number of currently active queries (excluding timer queries).
    * This is used to know if we need to pause any queries for meta ops.
    */
   unsigned active_queries;

   union {
      struct {
         struct gl_program *vp;    /**< Currently bound vertex program */
         struct gl_program *tcp; /**< Currently bound tess control program */
         struct gl_program *tep; /**< Currently bound tess eval program */
         struct gl_program *gp;  /**< Currently bound geometry program */
         struct gl_program *fp;  /**< Currently bound fragment program */
         struct gl_program *cp;   /**< Currently bound compute program */
      };
      struct gl_program *current_program[MESA_SHADER_STAGES];
   };

   struct st_common_variant *vp_variant;

   struct {
      struct pipe_resource *pixelmap_texture;
      struct pipe_sampler_view *pixelmap_sampler_view;
   } pixel_xfer;

   /** for glBitmap */
   struct {
      struct pipe_rasterizer_state rasterizer;
      struct pipe_sampler_state sampler;
      enum pipe_format tex_format;
      struct st_bitmap_cache cache;
   } bitmap;

   /** for glDraw/CopyPixels */
   struct {
      void *zs_shaders[6];
   } drawpix;

   /** Cache of glDrawPixels images */
   struct {
      struct drawpix_cache_entry entries[NUM_DRAWPIX_CACHE_ENTRIES];
      unsigned age;
   } drawpix_cache;

   /** for glReadPixels */
   struct {
      struct pipe_resource *src;
      struct pipe_resource *cache;
      enum pipe_format dst_format;
      unsigned level;
      unsigned layer;
      unsigned hits;
   } readpix_cache;

   /** for glClear */
   struct {
      struct pipe_rasterizer_state raster;
      struct pipe_viewport_state viewport;
      void *vs;
      void *fs;
      void *vs_layered;
      void *gs_layered;
   } clear;

   /* For gl(Compressed)Tex(Sub)Image */
   struct {
      struct pipe_rasterizer_state raster;
      struct pipe_blend_state upload_blend;
      void *vs;
      void *gs;
      void *upload_fs[5][2];
      /**
       * For drivers supporting formatless storing
       * (PIPE_CAP_IMAGE_STORE_FORMATTED) it is a pointer to the download FS;
       * for those not supporting it, it is a pointer to an array of
       * PIPE_FORMAT_COUNT elements, where each element is a pointer to the
       * download FS using that PIPE_FORMAT as the storing format.
       */
      void *download_fs[5][PIPE_MAX_TEXTURE_TYPES][2];
      struct hash_table *shaders;
      bool upload_enabled;
      bool download_enabled;
      bool rgba_only;
      bool layers;
      bool use_gs;
   } pbo;

   struct {
      struct gl_program **progs;
      struct pipe_resource *bc1_endpoint_buf;
      struct pipe_sampler_view *astc_luts[5];
      struct hash_table *astc_partition_tables;
   } texcompress_compute;

   /** for drawing with st_util_vertex */
   struct cso_velems_state util_velems;

   /** passthrough vertex shader matching the util_velem attributes */
   void *passthrough_vs;

   enum pipe_texture_target internal_target;

   void *winsys_drawable_handle;

   /* The number of vertex buffers from the last call of validate_arrays. */
   unsigned last_num_vbuffers;
   bool uses_user_vertex_buffers;

   unsigned last_used_atomic_bindings[PIPE_SHADER_TYPES];
   unsigned last_num_ssbos[PIPE_SHADER_TYPES];

   int32_t draw_stamp;
   int32_t read_stamp;

   struct st_config_options options;

   enum pipe_reset_status reset_status;

   /* Array of bound texture/image handles which are resident in the context.
    */
   struct st_bound_handles bound_texture_handles[PIPE_SHADER_TYPES];
   struct st_bound_handles bound_image_handles[PIPE_SHADER_TYPES];

   /* Winsys buffers */
   struct list_head winsys_buffers;

   /* Throttling for texture uploads and similar operations to limit memory
    * usage by limiting the number of in-flight operations based on
    * the estimated allocated size needed to execute those operations.
    */
   struct util_throttle throttle;

   struct {
      struct st_zombie_sampler_view_node list;
      simple_mtx_t mutex;
   } zombie_sampler_views;

   struct {
      struct st_zombie_shader_node list;
      simple_mtx_t mutex;
   } zombie_shaders;

   struct hash_table *hw_select_shaders;
};

/**
 * Represent the attributes of a context.
 */
struct st_context_attribs
{
   /**
    * The profile and minimal version to support.
    *
    * The valid profiles and versions are rendering API dependent.  The latest
    * version satisfying the request should be returned.
    */
   gl_api profile;
   int major, minor;

   /** Mask of ST_CONTEXT_FLAG_x bits */
   unsigned flags;

   /** Mask of PIPE_CONTEXT_x bits */
   unsigned context_flags;

   /**
    * The visual of the framebuffers the context will be bound to.
    */
   struct st_visual visual;

   /**
    * Configuration options.
    */
   struct st_config_options options;
};


/*
 * Get the state tracker context for the given Mesa context.
 */
static inline struct st_context *
st_context(struct gl_context *ctx)
{
   return ctx->st;
}


extern struct st_context *
st_create_context(gl_api api, struct pipe_context *pipe,
                  const struct gl_config *visual,
                  struct st_context *share,
                  const struct st_config_options *options,
                  bool no_error, bool has_egl_image_validate);

extern void
st_destroy_context(struct st_context *st);

extern void
st_context_flush(struct st_context *st, unsigned flags,
                 struct pipe_fence_handle **fence,
                 void (*before_flush_cb) (void*), void* args);

extern bool
st_context_teximage(struct st_context *st, GLenum target,
                    int level, enum pipe_format pipe_format,
                    struct pipe_resource *tex, bool mipmap);

extern void
st_context_invalidate_state(struct st_context *st, unsigned flags);

extern void
st_invalidate_buffers(struct st_context *st);


extern void
st_save_zombie_sampler_view(struct st_context *st,
                            struct pipe_sampler_view *view);

extern void
st_save_zombie_shader(struct st_context *st,
                      enum pipe_shader_type type,
                      struct pipe_shader_state *shader);


void
st_context_free_zombie_objects(struct st_context *st);

const struct nir_shader_compiler_options *
st_get_nir_compiler_options(struct st_context *st, gl_shader_stage stage);


void st_invalidate_state(struct gl_context *ctx);
void st_set_background_context(struct gl_context *ctx,
                               struct util_queue_monitoring *queue_info);

void
st_api_query_versions(struct pipe_frontend_screen *fscreen,
                      struct st_config_options *options,
                      int *gl_core_version,
                      int *gl_compat_version,
                      int *gl_es1_version,
                      int *gl_es2_version);

struct st_context *
st_api_create_context(struct pipe_frontend_screen *fscreen,
                      const struct st_context_attribs *attribs,
                      enum st_context_error *error,
                      struct st_context *shared_ctx);

bool
st_api_make_current(struct st_context *st,
                    struct pipe_frontend_drawable *stdrawi,
                    struct pipe_frontend_drawable *streadi);

struct st_context *
st_api_get_current(void);

void
st_api_destroy_drawable(struct pipe_frontend_drawable *drawable);

void
st_screen_destroy(struct pipe_frontend_screen *fscreen);

typedef void (*st_update_func_t)(struct st_context *st);

extern st_update_func_t st_update_functions[ST_NUM_ATOMS];

#ifdef __cplusplus
}
#endif

#endif
