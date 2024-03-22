/**************************************************************************
 *
 * Copyright 2009 Marek Olšák <maraeo@gmail.com>
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

#ifndef U_BLITTER_H
#define U_BLITTER_H

#include "util/u_framebuffer.h"
#include "util/u_inlines.h"

#include "pipe/p_state.h"

#ifdef __cplusplus
extern "C" {
#endif

struct pipe_context;

enum blitter_attrib_type {
   UTIL_BLITTER_ATTRIB_NONE,
   UTIL_BLITTER_ATTRIB_COLOR,
   UTIL_BLITTER_ATTRIB_TEXCOORD_XY,
   UTIL_BLITTER_ATTRIB_TEXCOORD_XYZW,
};

union blitter_attrib {
   float color[4];

   struct {
      float x1, y1, x2, y2, z, w;
   } texcoord;
};

struct blitter_context;

typedef void *(*blitter_get_vs_func)(struct blitter_context *blitter);

struct blitter_context
{
   /**
    * Draw a rectangle.
    *
    * \param get_vs  Callback for obtaining the vertex shader for the draw call.
    *                It might invoke the shader compiler. The driver is
    *                responsible for setting the vertex shader, and the callback
    *                allows the driver to query the vertex shader CSO if it
    *                wants to use the default one.
    * \param x1      An X coordinate of the top-left corner.
    * \param y1      A Y coordinate of the top-left corner.
    * \param x2      An X coordinate of the bottom-right corner.
    * \param y2      A Y coordinate of the bottom-right corner.
    * \param depth   A depth which the rectangle is rendered at.
    *
    * \param type   Semantics of the attributes "attrib".
    *               If type is UTIL_BLITTER_ATTRIB_NONE, ignore them.
    *               If type is UTIL_BLITTER_ATTRIB_COLOR, the attributes
    *               make up a constant RGBA color, and should go
    *               to the GENERIC0 varying slot of a fragment shader.
    *               If type is UTIL_BLITTER_ATTRIB_TEXCOORD, {a1, a2} and
    *               {a3, a4} specify top-left and bottom-right texture
    *               coordinates of the rectangle, respectively, and should go
    *               to the GENERIC0 varying slot of a fragment shader.
    *
    * \param attrib See type.
    *
    * \note A driver may optionally override this callback to implement
    *       a specialized hardware path for drawing a rectangle, e.g. using
    *       a rectangular point sprite.
    */
   void (*draw_rectangle)(struct blitter_context *blitter,
                          void *vertex_elements_cso,
                          blitter_get_vs_func get_vs,
                          int x1, int y1, int x2, int y2,
                          float depth, unsigned num_instances,
                          enum blitter_attrib_type type,
                          const union blitter_attrib *attrib);

   /* Whether the blitter is running. */
   bool running;

   bool use_index_buffer;

   /* Private members, really. */
   struct pipe_context *pipe; /**< pipe context */

   void *saved_blend_state;   /**< blend state */
   void *saved_dsa_state;     /**< depth stencil alpha state */
   void *saved_velem_state;   /**< vertex elements state */
   void *saved_rs_state;      /**< rasterizer state */
   void *saved_fs, *saved_vs, *saved_gs, *saved_tcs, *saved_tes; /**< shaders */

   struct pipe_framebuffer_state saved_fb_state;  /**< framebuffer state */
   struct pipe_stencil_ref saved_stencil_ref;     /**< stencil ref */
   struct pipe_viewport_state saved_viewport;
   struct pipe_scissor_state saved_scissor;
   bool skip_viewport_restore;
   bool is_sample_mask_saved;
   unsigned saved_sample_mask;
   unsigned saved_min_samples;

   unsigned saved_num_sampler_states;
   void *saved_sampler_states[PIPE_MAX_SAMPLERS];

   unsigned saved_num_sampler_views;
   struct pipe_sampler_view *saved_sampler_views[PIPE_MAX_SAMPLERS];

   unsigned cb_slot;
   struct pipe_constant_buffer saved_fs_constant_buffer;

   struct pipe_vertex_buffer saved_vertex_buffer;

   unsigned saved_num_so_targets;
   struct pipe_stream_output_target *saved_so_targets[PIPE_MAX_SO_BUFFERS];

   struct pipe_query *saved_render_cond_query;
   enum pipe_render_cond_flag saved_render_cond_mode;
   bool saved_render_cond_cond;

   bool saved_window_rectangles_include;
   unsigned saved_num_window_rectangles;
   struct pipe_scissor_state saved_window_rectangles[PIPE_MAX_WINDOW_RECTANGLES];
};

/**
 * Create a blitter context.
 */
struct blitter_context *util_blitter_create(struct pipe_context *pipe);

/**
 * Destroy a blitter context.
 */
void util_blitter_destroy(struct blitter_context *blitter);

void util_blitter_cache_all_shaders(struct blitter_context *blitter);
void *util_blitter_get_noop_blend_state(struct blitter_context *blitter);
void *util_blitter_get_noop_dsa_state(struct blitter_context *blitter);
void *util_blitter_get_discard_rasterizer_state(struct blitter_context *blitter);


/**
 * Return the pipe context associated with a blitter context.
 */
static inline
struct pipe_context *util_blitter_get_pipe(struct blitter_context *blitter)
{
   return blitter->pipe;
}

/**
 * Override PIPE_CAP_TEXTURE_MULTISAMPLE as reported by the driver.
 */
void util_blitter_set_texture_multisample(struct blitter_context *blitter,
                                          bool supported);

/* The default function to draw a rectangle. This can only be used
 * inside of the draw_rectangle callback if the driver overrides it. */
void util_blitter_draw_rectangle(struct blitter_context *blitter,
                                 void *vertex_elements_cso,
                                 blitter_get_vs_func get_vs,
                                 int x1, int y1, int x2, int y2,
                                 float depth, unsigned num_instances,
                                 enum blitter_attrib_type type,
                                 const union blitter_attrib *attrib);


/*
 * These states must be saved before any of the following functions are called:
 * - vertex buffers
 * - vertex elements
 * - vertex shader
 * - geometry shader (if supported)
 * - stream output targets (if supported)
 * - rasterizer state
 */

/**
 * Clear a specified set of currently bound buffers to specified values.
 *
 * These states must be saved in the blitter in addition to the state objects
 * already required to be saved:
 * - fragment shader
 * - depth stencil alpha state
 * - blend state
 */
void util_blitter_clear(struct blitter_context *blitter,
                        unsigned width, unsigned height, unsigned num_layers,
                        unsigned clear_buffers,
                        const union pipe_color_union *color,
                        double depth, unsigned stencil,
                        bool msaa);

/**
 * Check if the blitter (with the help of the driver) can blit between
 * the two resources.
 */
bool util_blitter_is_copy_supported(struct blitter_context *blitter,
                                    const struct pipe_resource *dst,
                                    const struct pipe_resource *src);

bool util_blitter_is_blit_supported(struct blitter_context *blitter,
				    const struct pipe_blit_info *info);

/**
 * Copy a block of pixels from one surface to another.
 *
 * These states must be saved in the blitter in addition to the state objects
 * already required to be saved:
 * - fragment shader
 * - depth stencil alpha state
 * - blend state
 * - fragment sampler states
 * - fragment sampler textures
 * - framebuffer state
 * - sample mask
 */
void util_blitter_copy_texture(struct blitter_context *blitter,
                               struct pipe_resource *dst,
                               unsigned dst_level,
                               unsigned dstx, unsigned dsty, unsigned dstz,
                               struct pipe_resource *src,
                               unsigned src_level,
                               const struct pipe_box *srcbox);

/**
 * This is a generic implementation of pipe->blit, which accepts
 * sampler/surface views instead of resources.
 *
 * The layer and mipmap level are specified by the views.
 *
 * Drivers can use this to change resource properties (like format, width,
 * height) by changing how the views interpret them, instead of changing
 * pipe_resource directly. This is used to blit resources of formats which
 * are not renderable.
 *
 * src_width0 and src_height0 are sampler_view-private properties that
 * override pipe_resource. The blitter uses them for computation of texture
 * coordinates. The dst dimensions are supplied through pipe_surface::width
 * and height.
 *
 * The mask is a combination of the PIPE_MASK_* flags.
 * Set to PIPE_MASK_RGBAZS if unsure.
 */
void util_blitter_blit_generic(struct blitter_context *blitter,
                               struct pipe_surface *dst,
                               const struct pipe_box *dstbox,
                               struct pipe_sampler_view *src,
                               const struct pipe_box *srcbox,
                               unsigned src_width0, unsigned src_height0,
                               unsigned mask, unsigned filter,
                               const struct pipe_scissor_state *scissor,
                               bool alpha_blend, bool sample0_only,
                               unsigned dst_sample);

void util_blitter_blit(struct blitter_context *blitter,
		       const struct pipe_blit_info *info);

void util_blitter_generate_mipmap(struct blitter_context *blitter,
                                  struct pipe_resource *tex,
                                  enum pipe_format format,
                                  unsigned base_level, unsigned last_level,
                                  unsigned first_layer, unsigned last_layer);

/**
 * Helper function to initialize a view for copy_texture_view.
 * The parameters must match copy_texture_view.
 */
void util_blitter_default_dst_texture(struct pipe_surface *dst_templ,
                                      struct pipe_resource *dst,
                                      unsigned dstlevel,
                                      unsigned dstz);

/**
 * Helper function to initialize a view for copy_texture_view.
 * The parameters must match copy_texture_view.
 */
void util_blitter_default_src_texture(struct blitter_context *blitter,
                                      struct pipe_sampler_view *src_templ,
                                      struct pipe_resource *src,
                                      unsigned srclevel);

/**
 * Clear the contents of a buffer using the Stream Output functionality.
 * 4-byte alignment is required.
 *
 * "num_channels" can be 1, 2, 3, or 4, and specifies if the clear value is
 * R, RG, RGB, or RGBA.
 *
 * For each element, only "num_channels" components of "clear_value" are
 * copied to the buffer, then the offset is incremented by num_channels*4.
 */
void util_blitter_clear_buffer(struct blitter_context *blitter,
                               struct pipe_resource *dst,
                               unsigned offset, unsigned size,
                               unsigned num_channels,
                               const union pipe_color_union *clear_value);

/**
 * Clear a region of a (color) surface to a constant value.
 *
 * These states must be saved in the blitter in addition to the state objects
 * already required to be saved:
 * - fragment shader
 * - depth stencil alpha state
 * - blend state
 * - framebuffer state
 */
void util_blitter_clear_render_target(struct blitter_context *blitter,
                                      struct pipe_surface *dst,
                                      const union pipe_color_union *color,
                                      unsigned dstx, unsigned dsty,
                                      unsigned width, unsigned height);

/**
 * Clear a region of a depth-stencil surface, both stencil and depth
 * or only one of them if this is a combined depth-stencil surface.
 *
 * These states must be saved in the blitter in addition to the state objects
 * already required to be saved:
 * - fragment shader
 * - depth stencil alpha state
 * - blend state
 * - framebuffer state
 */
void util_blitter_clear_depth_stencil(struct blitter_context *blitter,
                                      struct pipe_surface *dst,
                                      unsigned clear_flags,
                                      double depth,
                                      unsigned stencil,
                                      unsigned dstx, unsigned dsty,
                                      unsigned width, unsigned height);

/* The following functions are customized variants of the clear functions.
 * Some drivers use them internally to do things like MSAA resolve
 * and resource decompression. It usually consists of rendering a full-screen
 * quad with a special blend or DSA state.
 */

/* Used by r300g for depth decompression. */
void util_blitter_custom_clear_depth(struct blitter_context *blitter,
                                     unsigned width, unsigned height,
                                     double depth, void *custom_dsa);

/* Used by r600g for depth decompression. */
void util_blitter_custom_depth_stencil(struct blitter_context *blitter,
				       struct pipe_surface *zsurf,
				       struct pipe_surface *cbsurf,
				       unsigned sample_mask,
				       void *dsa_stage, float depth);

/* Used by r600g for color decompression. */
void util_blitter_custom_color(struct blitter_context *blitter,
                               struct pipe_surface *dstsurf,
                               void *custom_blend);

/* Used by r600g for MSAA color resolve. */
void util_blitter_custom_resolve_color(struct blitter_context *blitter,
                                       struct pipe_resource *dst,
                                       unsigned dst_level,
                                       unsigned dst_layer,
                                       struct pipe_resource *src,
                                       unsigned src_layer,
				       unsigned sampled_mask,
                                       void *custom_blend,
                                       enum pipe_format format);

/* Used by vc4 for 8/16-bit linear-to-tiled blits */
void util_blitter_custom_shader(struct blitter_context *blitter,
                                struct pipe_surface *dstsurf,
                                void *custom_vs, void *custom_fs);

/* Used by D3D12 for non-MSAA -> MSAA stencil blits */
void util_blitter_stencil_fallback(struct blitter_context *blitter,
                                   struct pipe_resource *dst,
                                   unsigned dst_level,
                                   const struct pipe_box *dstbox,
                                   struct pipe_resource *src,
                                   unsigned src_level,
                                   const struct pipe_box *srcbox,
                                   const struct pipe_scissor_state *scissor);

/* The functions below should be used to save currently bound constant state
 * objects inside a driver. The objects are automatically restored at the end
 * of the util_blitter_{clear, copy_region, fill_region} functions and then
 * forgotten.
 *
 * States not listed here are not affected by util_blitter. */

static inline void
util_blitter_save_blend(struct blitter_context *blitter, void *state)
{
   blitter->saved_blend_state = state;
}

static inline void
util_blitter_save_depth_stencil_alpha(struct blitter_context *blitter,
                                      void *state)
{
   blitter->saved_dsa_state = state;
}

static inline void
util_blitter_save_vertex_elements(struct blitter_context *blitter, void *state)
{
   blitter->saved_velem_state = state;
}

static inline void
util_blitter_save_stencil_ref(struct blitter_context *blitter,
                              const struct pipe_stencil_ref *state)
{
   blitter->saved_stencil_ref = *state;
}

static inline void
util_blitter_save_rasterizer(struct blitter_context *blitter, void *state)
{
   blitter->saved_rs_state = state;
}

static inline void
util_blitter_save_fragment_shader(struct blitter_context *blitter, void *fs)
{
   blitter->saved_fs = fs;
}

static inline void
util_blitter_save_vertex_shader(struct blitter_context *blitter, void *vs)
{
   blitter->saved_vs = vs;
}

static inline void
util_blitter_save_geometry_shader(struct blitter_context *blitter, void *gs)
{
   blitter->saved_gs = gs;
}

static inline void
util_blitter_save_tessctrl_shader(struct blitter_context *blitter,
                                  void *sh)
{
   blitter->saved_tcs = sh;
}

static inline void
util_blitter_save_tesseval_shader(struct blitter_context *blitter,
                                  void *sh)
{
   blitter->saved_tes = sh;
}

static inline void
util_blitter_save_framebuffer(struct blitter_context *blitter,
                              const struct pipe_framebuffer_state *state)
{
   blitter->saved_fb_state.nr_cbufs = 0; /* It's ~0 now, meaning it's unsaved. */
   util_copy_framebuffer_state(&blitter->saved_fb_state, state);
}

static inline void
util_blitter_save_viewport(struct blitter_context *blitter,
                           struct pipe_viewport_state *state)
{
   blitter->saved_viewport = *state;
}

static inline void
util_blitter_save_scissor(struct blitter_context *blitter,
                          struct pipe_scissor_state *state)
{
   blitter->saved_scissor = *state;
}

static inline void
util_blitter_save_fragment_sampler_states(
                  struct blitter_context *blitter,
                  unsigned num_sampler_states,
                  void **sampler_states)
{
   assert(num_sampler_states <= ARRAY_SIZE(blitter->saved_sampler_states));

   blitter->saved_num_sampler_states = num_sampler_states;
   memcpy(blitter->saved_sampler_states, sampler_states,
          num_sampler_states * sizeof(void *));
}

static inline void
util_blitter_save_fragment_sampler_views(struct blitter_context *blitter,
                                         unsigned num_views,
                                         struct pipe_sampler_view **views)
{
   unsigned i;
   assert(num_views <= ARRAY_SIZE(blitter->saved_sampler_views));

   blitter->saved_num_sampler_views = num_views;
   for (i = 0; i < num_views; i++)
      pipe_sampler_view_reference(&blitter->saved_sampler_views[i],
                                  views[i]);
}

static inline void
util_blitter_save_fragment_constant_buffer_slot(
                  struct blitter_context *blitter,
                  struct pipe_constant_buffer *constant_buffers)
{
   pipe_resource_reference(&blitter->saved_fs_constant_buffer.buffer,
                           constant_buffers[blitter->cb_slot].buffer);
   memcpy(&blitter->saved_fs_constant_buffer, &constant_buffers[blitter->cb_slot],
          sizeof(struct pipe_constant_buffer));
}

static inline void
util_blitter_save_vertex_buffer_slot(struct blitter_context *blitter,
                                     struct pipe_vertex_buffer *vertex_buffers)
{
   pipe_vertex_buffer_reference(&blitter->saved_vertex_buffer,
                                &vertex_buffers[0]);
}

static inline void
util_blitter_save_so_targets(struct blitter_context *blitter,
                             unsigned num_targets,
                             struct pipe_stream_output_target **targets)
{
   unsigned i;
   assert(num_targets <= ARRAY_SIZE(blitter->saved_so_targets));

   blitter->saved_num_so_targets = num_targets;
   for (i = 0; i < num_targets; i++)
      pipe_so_target_reference(&blitter->saved_so_targets[i],
                               targets[i]);
}

static inline void
util_blitter_save_sample_mask(struct blitter_context *blitter,
                              unsigned sample_mask, unsigned min_samples)
{
   blitter->is_sample_mask_saved = true;
   blitter->saved_sample_mask = sample_mask;
   blitter->saved_min_samples = min_samples;
}

static inline void
util_blitter_save_render_condition(struct blitter_context *blitter,
                                   struct pipe_query *query,
                                   bool condition,
                                   enum pipe_render_cond_flag mode)
{
   blitter->saved_render_cond_query = query;
   blitter->saved_render_cond_mode = mode;
   blitter->saved_render_cond_cond = condition;
}

static inline void
util_blitter_save_window_rectangles(struct blitter_context *blitter,
                                    bool include,
                                    unsigned num_rectangles,
                                    const struct pipe_scissor_state *rects)
{
   blitter->saved_window_rectangles_include = include;
   blitter->saved_num_window_rectangles = num_rectangles;
   if (num_rectangles > 0) {
      assert(num_rectangles < ARRAY_SIZE(blitter->saved_window_rectangles));
      memcpy(blitter->saved_window_rectangles, rects,
             sizeof(*rects) * num_rectangles);
   }
}

void util_blitter_common_clear_setup(struct blitter_context *blitter,
                                     unsigned width, unsigned height,
                                     unsigned clear_buffers,
                                     void *custom_blend, void *custom_dsa);

void util_blitter_set_running_flag(struct blitter_context *blitter);
void util_blitter_unset_running_flag(struct blitter_context *blitter);

void util_blitter_restore_vertex_states(struct blitter_context *blitter);
void util_blitter_restore_fragment_states(struct blitter_context *blitter);
void util_blitter_restore_render_cond(struct blitter_context *blitter);
void util_blitter_restore_fb_state(struct blitter_context *blitter);
void util_blitter_restore_textures(struct blitter_context *blitter);
void util_blitter_restore_constant_buffer_state(struct blitter_context *blitter);

/* These are supported combinations of blits from ZS to color and vice versa.
 * The blitter will do the packing/unpacking of depth and stencil
 * in the fragment shader.
 */
static inline enum pipe_format
util_blitter_get_color_format_for_zs(enum pipe_format format)
{
   switch (format) {
   case PIPE_FORMAT_Z16_UNORM:
      return PIPE_FORMAT_R16_UNORM;

   case PIPE_FORMAT_Z32_FLOAT:
      return PIPE_FORMAT_R32_FLOAT;

   case PIPE_FORMAT_Z24_UNORM_S8_UINT:
   case PIPE_FORMAT_Z24X8_UNORM:
   case PIPE_FORMAT_S8_UINT_Z24_UNORM:
   case PIPE_FORMAT_X8Z24_UNORM:
      return PIPE_FORMAT_R32_UINT;

   case PIPE_FORMAT_Z32_FLOAT_S8X24_UINT:
      return PIPE_FORMAT_R32G32_UINT;

   case PIPE_FORMAT_Z32_UNORM:
   default:
      assert(0);
   }
   return PIPE_FORMAT_NONE;
}

#ifdef __cplusplus
}
#endif

#endif
