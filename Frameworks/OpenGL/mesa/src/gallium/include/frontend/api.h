/**********************************************************
 * Copyright 2010 VMware, Inc.  All rights reserved.
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


#ifndef _API_H_
#define _API_H_

#include "util/format/u_formats.h"

struct st_context;

/**
 * \file API for communication between gallium frontends and supporting
 * frontends such as DRI.
 *
 * This file defines the API that the GL frontend uses to talk to
 * the DRI/GLX/WGL frontends.
 */


/**
 * Context flags.
 */
#define ST_CONTEXT_FLAG_DEBUG               (1 << 0)
#define ST_CONTEXT_FLAG_FORWARD_COMPATIBLE  (1 << 1)
#define ST_CONTEXT_FLAG_NO_ERROR            (1 << 2)
#define ST_CONTEXT_FLAG_RELEASE_NONE        (1 << 3)


/**
 * Reasons that context creation might fail.
 */
enum st_context_error {
   ST_CONTEXT_SUCCESS = 0,
   ST_CONTEXT_ERROR_NO_MEMORY,
   ST_CONTEXT_ERROR_BAD_VERSION,
};

/**
 * Available attachments of framebuffer.
 */
enum st_attachment_type {
   ST_ATTACHMENT_FRONT_LEFT,
   ST_ATTACHMENT_BACK_LEFT,
   ST_ATTACHMENT_FRONT_RIGHT,
   ST_ATTACHMENT_BACK_RIGHT,
   ST_ATTACHMENT_DEPTH_STENCIL,
   ST_ATTACHMENT_ACCUM,

   ST_ATTACHMENT_COUNT,
   ST_ATTACHMENT_INVALID = -1
};

/* for buffer_mask in st_visual */
#define ST_ATTACHMENT_FRONT_LEFT_MASK     (1 << ST_ATTACHMENT_FRONT_LEFT)
#define ST_ATTACHMENT_BACK_LEFT_MASK      (1 << ST_ATTACHMENT_BACK_LEFT)
#define ST_ATTACHMENT_FRONT_RIGHT_MASK    (1 << ST_ATTACHMENT_FRONT_RIGHT)
#define ST_ATTACHMENT_BACK_RIGHT_MASK     (1 << ST_ATTACHMENT_BACK_RIGHT)
#define ST_ATTACHMENT_DEPTH_STENCIL_MASK  (1 << ST_ATTACHMENT_DEPTH_STENCIL)
#define ST_ATTACHMENT_ACCUM_MASK          (1 << ST_ATTACHMENT_ACCUM)

/**
 * Flush flags.
 */
#define ST_FLUSH_FRONT                    (1 << 0)
#define ST_FLUSH_END_OF_FRAME             (1 << 1)
#define ST_FLUSH_WAIT                     (1 << 2)
#define ST_FLUSH_FENCE_FD                 (1 << 3)

/**
 * State invalidation flags to notify st_context that states have been changed
 * behind their back.
 */
#define ST_INVALIDATE_FS_SAMPLER_VIEWS    (1 << 0)
#define ST_INVALIDATE_FS_CONSTBUF0        (1 << 1)
#define ST_INVALIDATE_VS_CONSTBUF0        (1 << 2)
#define ST_INVALIDATE_VERTEX_BUFFERS      (1 << 3)
#define ST_INVALIDATE_FB_STATE            (1 << 4)

/**
 * Value to pipe_frontend_streen::get_param function.
 */
enum st_manager_param {
   /**
    * The DRI frontend on old libGL's doesn't do the right thing
    * with regards to invalidating the framebuffers.
    *
    * For the GL gallium frontend that means that it needs to invalidate
    * the framebuffer in glViewport itself.
    */
   ST_MANAGER_BROKEN_INVALIDATE
};

struct pipe_resource;
struct util_queue_monitoring;

/**
 * Used in pipe_frontend_screen::get_egl_image.
 */
struct st_egl_image
{
   /* this is owned by the caller */
   struct pipe_resource *texture;

   /* format only differs from texture->format for multi-planar (YUV): */
   enum pipe_format format;

   unsigned level;
   unsigned layer;
   /* GL internal format. */
   unsigned internalformat;

   /* one of __DRI_YUV_COLOR_SPACE_* */
   unsigned yuv_color_space;

   /* one of __DRI_YUV_RANGE_* */
   unsigned yuv_range;

   bool imported_dmabuf;
};

/**
 * Represent the visual of a framebuffer.
 */
struct st_visual
{
   /**
    * Available buffers.  Bitfield of ST_ATTACHMENT_*_MASK bits.
    */
   unsigned buffer_mask;

   /**
    * Buffer formats.  The formats are always set even when the buffer is
    * not available.
    */
   enum pipe_format color_format;
   enum pipe_format depth_stencil_format;
   enum pipe_format accum_format;
   unsigned samples;
};


/**
 * Configuration options from driconf
 */
struct st_config_options
{
   bool disable_blend_func_extended;
   bool disable_glsl_line_continuations;
   bool disable_arb_gpu_shader5;
   bool disable_uniform_array_resize;
   char *alias_shader_extension;
   bool allow_vertex_texture_bias;
   bool force_compat_shaders;
   bool force_glsl_extensions_warn;
   unsigned force_glsl_version;
   bool allow_extra_pp_tokens;
   bool allow_glsl_extension_directive_midshader;
   bool allow_glsl_120_subset_in_110;
   bool allow_glsl_builtin_const_expression;
   bool allow_glsl_relaxed_es;
   bool allow_glsl_builtin_variable_redeclaration;
   bool allow_higher_compat_version;
   bool allow_glsl_compat_shaders;
   bool glsl_ignore_write_to_readonly_var;
   bool glsl_zero_init;
   bool vs_position_always_invariant;
   bool vs_position_always_precise;
   bool force_glsl_abs_sqrt;
   bool allow_glsl_cross_stage_interpolation_mismatch;
   bool do_dce_before_clip_cull_analysis;
   bool allow_draw_out_of_order;
   bool glthread_nop_check_framebuffer_status;
   bool ignore_map_unsynchronized;
   bool ignore_discard_framebuffer;
   bool force_integer_tex_nearest;
   bool force_gl_names_reuse;
   bool force_gl_map_buffer_synchronized;
   bool transcode_etc;
   bool transcode_astc;
   char *force_gl_vendor;
   char *force_gl_renderer;
   char *mesa_extension_override;
   bool allow_multisampled_copyteximage;

   unsigned char config_options_sha1[20];
};

struct pipe_frontend_screen;

/**
 * Represent a windowing system drawable.
 *
 * This is inherited by the drawable implementation of the DRI/GLX/WGL
 * frontends, e.g. this is the first field in dri_drawable.
 *
 * st_context uses the callbacks to invoke one of the DRI/GLX/WGL-specific
 * functions.
 *
 * This drawable can be shared between different threads. The atomic stamp
 * is used to communicate that the drawable has been changed, and
 * the framebuffer state should be updated.
 */
struct pipe_frontend_drawable
{
   /**
    * Atomic stamp which changes when framebuffers need to be updated.
    */
   int32_t stamp;

   /**
    * Identifier that uniquely identifies the framebuffer interface object.
    */
   uint32_t ID;

   /**
    * The frontend screen for DRI/GLX/WGL.  This is e.g. dri_screen.
    */
   struct pipe_frontend_screen *fscreen;

   /**
    * The visual of the framebuffer.
    */
   const struct st_visual *visual;

   /**
    * Flush the front buffer.
    *
    * On some window systems, changes to the front buffers are not immediately
    * visible.  They need to be flushed.
    *
    * @att is one of the front buffer attachments.
    */
   bool (*flush_front)(struct st_context *st,
                       struct pipe_frontend_drawable *drawable,
                       enum st_attachment_type statt);

   /**
    * The GL frontend asks for the framebuffer attachments it needs.
    *
    * It should try to only ask for attachments that it currently renders
    * to, thus allowing the winsys to delay the allocation of textures not
    * needed. For example front buffer attachments are not needed if you
    * only do back buffer rendering.
    *
    * The implementor of this function needs to also ensure
    * thread safty as this call might be done from multiple threads.
    *
    * The returned textures are owned by the caller.  They should be
    * unreferenced when no longer used.  If this function is called multiple
    * times with different sets of attachments, those buffers not included in
    * the last call might be destroyed.
    */
   bool (*validate)(struct st_context *st,
                    struct pipe_frontend_drawable *drawable,
                    const enum st_attachment_type *statts,
                    unsigned count,
                    struct pipe_resource **out,
                    struct pipe_resource **resolve);

   bool (*flush_swapbuffers)(struct st_context *st,
                             struct pipe_frontend_drawable *drawable);
};


/**
 * This is inherited by a screen in the DRI/GLX/WGL frontends, e.g. dri_screen.
 */
struct pipe_frontend_screen
{
   struct pipe_screen *screen;

   /**
    * Look up and return the info of an EGLImage.
    *
    * This is used to implement for example EGLImageTargetTexture2DOES.
    * The GLeglImageOES agrument of that call is passed directly to this
    * function call and the information needed to access this is returned
    * in the given struct out.
    *
    * @fscreen: the screen
    * @egl_image: EGLImage that caller recived
    * @out: return struct filled out with access information.
    *
    * This function is optional.
    */
   bool (*get_egl_image)(struct pipe_frontend_screen *fscreen,
                         void *egl_image,
                         struct st_egl_image *out);

   /**
    * Validate EGLImage passed to get_egl_image.
    */
   bool (*validate_egl_image)(struct pipe_frontend_screen *fscreen,
                              void *egl_image);

   /**
    * Query a feature or property from the DRI/GLX/WGL frontend.
    */
   int (*get_param)(struct pipe_frontend_screen *fscreen,
                    enum st_manager_param param);

   /**
    * Call the loader function setBackgroundContext. Called from the worker
    * thread.
    */
   void (*set_background_context)(struct st_context *st,
                                  struct util_queue_monitoring *queue_info);

   /**
    * GL frontend state associated with the screen.
    *
    * This is where st_context stores the state shared by all contexts.
    */
   void *st_screen;
};

#endif /* _API_H_ */
