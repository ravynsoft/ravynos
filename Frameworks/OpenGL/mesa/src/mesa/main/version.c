/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 2010  VMware, Inc.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */


#include <stdio.h>
#include "context.h"
#include "draw_validate.h"

#include "util/os_misc.h"
#include "util/simple_mtx.h"

#include "mtypes.h"
#include "version.h"
#include "git_sha1.h"

#include "state_tracker/st_context.h"

static simple_mtx_t override_lock = SIMPLE_MTX_INITIALIZER;

/**
 * Scans 'string' to see if it ends with 'ending'.
 */
static bool
check_for_ending(const char *string, const char *ending)
{
   const size_t len1 = strlen(string);
   const size_t len2 = strlen(ending);

   if (len2 > len1)
      return false;

   return strcmp(string + (len1 - len2), ending) == 0;
}

/**
 * Returns the gl override data
 *
 * version > 0 indicates there is an override requested
 * fwd_context is only valid if version > 0
 */
static void
get_gl_override(gl_api api, int *version, bool *fwd_context,
                bool *compat_context)
{
   const char *env_var = (api == API_OPENGL_CORE || api == API_OPENGL_COMPAT)
      ? "MESA_GL_VERSION_OVERRIDE" : "MESA_GLES_VERSION_OVERRIDE";
   const char *version_str;
   int major, minor, n;
   static struct override_info {
      int version;
      bool fc_suffix;
      bool compat_suffix;
   } override[] = {
      [API_OPENGL_COMPAT] = { -1, false, false},
      [API_OPENGLES]      = { -1, false, false},
      [API_OPENGLES2]     = { -1, false, false},
      [API_OPENGL_CORE]   = { -1, false, false},
   };

   STATIC_ASSERT(ARRAY_SIZE(override) == API_OPENGL_LAST + 1);

   simple_mtx_lock(&override_lock);

   if (api == API_OPENGLES)
      goto exit;

   if (override[api].version < 0) {
      override[api].version = 0;

      version_str = os_get_option(env_var);
      if (version_str) {
         override[api].fc_suffix = check_for_ending(version_str, "FC");
         override[api].compat_suffix = check_for_ending(version_str, "COMPAT");

         n = sscanf(version_str, "%u.%u", &major, &minor);
         if (n != 2) {
            fprintf(stderr, "error: invalid value for %s: %s\n",
                    env_var, version_str);
            override[api].version = 0;
         } else {
            override[api].version = major * 10 + minor;

            /* There is no such thing as compatibility or forward-compatible for
             * OpenGL ES 2.0 or 3.x APIs.
             */
            if ((override[api].version < 30 && override[api].fc_suffix) ||
                (api == API_OPENGLES2 && (override[api].fc_suffix ||
                                          override[api].compat_suffix))) {
               fprintf(stderr, "error: invalid value for %s: %s\n",
                       env_var, version_str);
            }
         }
      }
   }

exit:
   *version = override[api].version;
   *fwd_context = override[api].fc_suffix;
   *compat_context = override[api].compat_suffix;

   simple_mtx_unlock(&override_lock);
}

/**
 * Builds the Mesa version string.
 */
static void
create_version_string(struct gl_context *ctx, const char *prefix)
{
   static const int max = 100;

   ctx->VersionString = malloc(max);
   if (ctx->VersionString) {
      snprintf(ctx->VersionString, max,
		     "%s%u.%u%s Mesa " PACKAGE_VERSION MESA_GIT_SHA1,
		     prefix,
		     ctx->Version / 10, ctx->Version % 10,
		     _mesa_is_desktop_gl_core(ctx) ? " (Core Profile)" :
                     (_mesa_is_desktop_gl_compat(ctx) && ctx->Version >= 32) ?
                        " (Compatibility Profile)" : ""
		     );
   }
}

/**
 * Override the context's version and/or API type if the environment variables
 * MESA_GL_VERSION_OVERRIDE or MESA_GLES_VERSION_OVERRIDE are set.
 *
 * Example uses of MESA_GL_VERSION_OVERRIDE:
 *
 * 2.1: select a compatibility (non-Core) profile with GL version 2.1.
 * 3.0: select a compatibility (non-Core) profile with GL version 3.0.
 * 3.0FC: select a Core+Forward Compatible profile with GL version 3.0.
 * 3.1: select GL version 3.1 with GL_ARB_compatibility enabled per the driver default.
 * 3.1FC: select GL version 3.1 with forward compatibility and GL_ARB_compatibility disabled.
 * 3.1COMPAT: select GL version 3.1 with GL_ARB_compatibility enabled.
 * X.Y: override GL version to X.Y without changing the profile.
 * X.YFC: select a Core+Forward Compatible profile with GL version X.Y.
 * X.YCOMPAT: select a Compatibility profile with GL version X.Y.
 *
 * Example uses of MESA_GLES_VERSION_OVERRIDE:
 *
 * 2.0: select GLES version 2.0.
 * 3.0: select GLES version 3.0.
 * 3.1: select GLES version 3.1.
 */
bool
_mesa_override_gl_version_contextless(struct gl_constants *consts,
                                      gl_api *apiOut, GLuint *versionOut)
{
   int version;
   bool fwd_context, compat_context;

   get_gl_override(*apiOut, &version, &fwd_context, &compat_context);

   if (version > 0) {
      *versionOut = version;

      /* Modify the API and context flags as needed. */
      if (*apiOut == API_OPENGL_CORE || *apiOut == API_OPENGL_COMPAT) {
         if (version >= 30 && fwd_context) {
            *apiOut = API_OPENGL_CORE;
            consts->ContextFlags |= GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT;
         } else if (compat_context) {
            *apiOut = API_OPENGL_COMPAT;
         }
      }

      return true;
   }
   return false;
}

void
_mesa_override_gl_version(struct gl_context *ctx)
{
   if (_mesa_override_gl_version_contextless(&ctx->Const, &ctx->API,
                                             &ctx->Version)) {
      /* We need to include API in version string for OpenGL ES, otherwise
       * application can not detect GLES via glGetString(GL_VERSION) query.
       *
       * From OpenGL ES 3.2 spec, Page 436:
       *
       *     "The VERSION string is laid out as follows:
       *
       *     OpenGL ES N.M vendor-specific information"
       *
       * From OpenGL 4.5 spec, Page 538:
       *
       *     "The VERSION and SHADING_LANGUAGE_VERSION strings are laid out as
       *     follows:
       *
       *     <version number><space><vendor-specific information>"
       */
      create_version_string(ctx, _mesa_is_gles(ctx) ? "OpenGL ES " : "");
      ctx->Extensions.Version = ctx->Version;
   }
}

/**
 * Override the context's GLSL version if the environment variable
 * MESA_GLSL_VERSION_OVERRIDE is set. Valid values for
 * MESA_GLSL_VERSION_OVERRIDE are integers, such as "130".
 */
void
_mesa_override_glsl_version(struct gl_constants *consts)
{
   const char *env_var = "MESA_GLSL_VERSION_OVERRIDE";
   const char *version;
   int n;

   version = getenv(env_var);
   if (!version) {
      return;
   }

   n = sscanf(version, "%u", &consts->GLSLVersion);
   if (n != 1) {
      fprintf(stderr, "error: invalid value for %s: %s\n", env_var, version);
      return;
   }
}

/**
 * Examine enabled GL extensions to determine GL version.
 */
static GLuint
compute_version(const struct gl_extensions *extensions,
                const struct gl_constants *consts, gl_api api)
{
   GLuint major, minor, version;

   const bool ver_1_4 = (extensions->ARB_shadow);
   const bool ver_1_5 = ver_1_4;
   const bool ver_2_0 = (ver_1_5 &&
                         extensions->ARB_vertex_shader &&
                         extensions->ARB_fragment_shader &&
                         extensions->ARB_texture_non_power_of_two &&
                         extensions->EXT_blend_equation_separate &&
                         extensions->EXT_stencil_two_side);
   const bool ver_2_1 = (ver_2_0 &&
                         extensions->EXT_texture_sRGB);
   /* We lie about the minimum number of color attachments. Strictly, OpenGL
    * 3.0 requires 8, whereas OpenGL ES requires 4. OpenGL ES 3.0 class
    * hardware may only support 4 render targets. Advertise non-conformant
    * OpenGL 3.0 anyway. Affects freedreno on a3xx
    */
   const bool ver_3_0 = (ver_2_1 &&
                         consts->GLSLVersion >= 130 &&
                         consts->MaxColorAttachments >= 4 &&
                         (consts->MaxSamples >= 4 || consts->FakeSWMSAA) &&
                         (api == API_OPENGL_CORE ||
                          extensions->ARB_color_buffer_float) &&
                         extensions->ARB_depth_buffer_float &&
                         extensions->ARB_half_float_vertex &&
                         extensions->ARB_map_buffer_range &&
                         extensions->ARB_shader_texture_lod &&
                         extensions->ARB_texture_float &&
                         extensions->ARB_texture_rg &&
                         extensions->ARB_texture_compression_rgtc &&
                         extensions->EXT_draw_buffers2 &&
                         extensions->ARB_framebuffer_object &&
                         extensions->EXT_framebuffer_sRGB &&
                         extensions->EXT_packed_float &&
                         extensions->EXT_texture_array &&
                         extensions->EXT_texture_shared_exponent &&
                         extensions->EXT_transform_feedback &&
                         extensions->NV_conditional_render);
   const bool ver_3_1 = (ver_3_0 &&
                         consts->GLSLVersion >= 140 &&
                         extensions->ARB_draw_instanced &&
                         extensions->ARB_texture_buffer_object &&
                         extensions->ARB_uniform_buffer_object &&
                         extensions->EXT_texture_snorm &&
                         extensions->NV_primitive_restart &&
                         extensions->NV_texture_rectangle &&
                         consts->Program[MESA_SHADER_VERTEX].MaxTextureImageUnits >= 16);
   const bool ver_3_2 = (ver_3_1 &&
                         consts->GLSLVersion >= 150 &&
                         extensions->ARB_depth_clamp &&
                         extensions->ARB_draw_elements_base_vertex &&
                         extensions->ARB_fragment_coord_conventions &&
                         extensions->EXT_provoking_vertex &&
                         extensions->ARB_seamless_cube_map &&
                         extensions->ARB_sync &&
                         extensions->ARB_texture_multisample &&
                         extensions->EXT_vertex_array_bgra);
   const bool ver_3_3 = (ver_3_2 &&
                         consts->GLSLVersion >= 330 &&
                         extensions->ARB_blend_func_extended &&
                         extensions->ARB_explicit_attrib_location &&
                         extensions->ARB_instanced_arrays &&
                         extensions->ARB_shader_bit_encoding &&
                         extensions->ARB_texture_rgb10_a2ui &&
                         extensions->ARB_timer_query &&
                         extensions->ARB_vertex_type_2_10_10_10_rev &&
                         extensions->EXT_texture_swizzle);
   /* ARB_sampler_objects is always enabled in mesa */

   const bool ver_4_0 = (ver_3_3 &&
                         consts->GLSLVersion >= 400 &&
                         extensions->ARB_draw_buffers_blend &&
                         extensions->ARB_draw_indirect &&
                         extensions->ARB_gpu_shader5 &&
                         extensions->ARB_gpu_shader_fp64 &&
                         extensions->ARB_sample_shading &&
                         extensions->ARB_tessellation_shader &&
                         extensions->ARB_texture_buffer_object_rgb32 &&
                         extensions->ARB_texture_cube_map_array &&
                         extensions->ARB_texture_query_lod &&
                         extensions->ARB_transform_feedback2 &&
                         extensions->ARB_transform_feedback3);
   const bool ver_4_1 = (ver_4_0 &&
                         consts->GLSLVersion >= 410 &&
                         consts->MaxTextureSize >= 16384 &&
                         consts->MaxRenderbufferSize >= 16384 &&
                         consts->MaxCubeTextureLevels >= 15 &&
                         consts->Max3DTextureLevels >= 12 &&
                         consts->MaxArrayTextureLayers >= 2048 &&
                         extensions->ARB_ES2_compatibility &&
                         extensions->ARB_shader_precision &&
                         extensions->ARB_vertex_attrib_64bit &&
                         extensions->ARB_viewport_array);
   const bool ver_4_2 = (ver_4_1 &&
                         consts->GLSLVersion >= 420 &&
                         extensions->ARB_base_instance &&
                         extensions->ARB_conservative_depth &&
                         extensions->ARB_internalformat_query &&
                         extensions->ARB_shader_atomic_counters &&
                         extensions->ARB_shader_image_load_store &&
                         extensions->ARB_shading_language_420pack &&
                         extensions->ARB_shading_language_packing &&
                         extensions->ARB_texture_compression_bptc &&
                         extensions->ARB_transform_feedback_instanced);
   const bool ver_4_3 = (ver_4_2 &&
                         consts->GLSLVersion >= 430 &&
                         consts->Program[MESA_SHADER_VERTEX].MaxUniformBlocks >= 14 &&
                         extensions->ARB_ES3_compatibility &&
                         extensions->ARB_arrays_of_arrays &&
                         extensions->ARB_compute_shader &&
                         extensions->ARB_copy_image &&
                         extensions->ARB_explicit_uniform_location &&
                         extensions->ARB_fragment_layer_viewport &&
                         extensions->ARB_framebuffer_no_attachments &&
                         extensions->ARB_internalformat_query2 &&
                         extensions->ARB_robust_buffer_access_behavior &&
                         extensions->ARB_shader_image_size &&
                         extensions->ARB_shader_storage_buffer_object &&
                         extensions->ARB_stencil_texturing &&
                         extensions->ARB_texture_buffer_range &&
                         extensions->ARB_texture_query_levels &&
                         extensions->ARB_texture_view);
   const bool ver_4_4 = (ver_4_3 &&
                         consts->GLSLVersion >= 440 &&
                         consts->MaxVertexAttribStride >= 2048 &&
                         extensions->ARB_buffer_storage &&
                         extensions->ARB_enhanced_layouts &&
                         extensions->ARB_query_buffer_object &&
                         extensions->ARB_texture_mirror_clamp_to_edge &&
                         extensions->ARB_texture_stencil8 &&
                         extensions->ARB_vertex_type_10f_11f_11f_rev);
   const bool ver_4_5 = (ver_4_4 &&
                         consts->GLSLVersion >= 450 &&
                         extensions->ARB_ES3_1_compatibility &&
                         extensions->ARB_clip_control &&
                         extensions->ARB_conditional_render_inverted &&
                         extensions->ARB_cull_distance &&
                         extensions->ARB_derivative_control &&
                         extensions->ARB_shader_texture_image_samples &&
                         extensions->NV_texture_barrier);
   const bool ver_4_6 = (ver_4_5 &&
                         consts->GLSLVersion >= 460 &&
                         extensions->ARB_gl_spirv &&
                         extensions->ARB_spirv_extensions &&
                         extensions->ARB_indirect_parameters &&
                         extensions->ARB_polygon_offset_clamp &&
                         extensions->ARB_shader_atomic_counter_ops &&
                         extensions->ARB_shader_draw_parameters &&
                         extensions->ARB_shader_group_vote &&
                         extensions->ARB_texture_filter_anisotropic &&
                         extensions->ARB_transform_feedback_overflow_query);

   if (ver_4_6) {
      major = 4;
      minor = 6;
   }
   else if (ver_4_5) {
      major = 4;
      minor = 5;
   }
   else if (ver_4_4) {
      major = 4;
      minor = 4;
   }
   else if (ver_4_3) {
      major = 4;
      minor = 3;
   }
   else if (ver_4_2) {
      major = 4;
      minor = 2;
   }
   else if (ver_4_1) {
      major = 4;
      minor = 1;
   }
   else if (ver_4_0) {
      major = 4;
      minor = 0;
   }
   else if (ver_3_3) {
      major = 3;
      minor = 3;
   }
   else if (ver_3_2) {
      major = 3;
      minor = 2;
   }
   else if (ver_3_1) {
      major = 3;
      minor = 1;
   }
   else if (ver_3_0) {
      major = 3;
      minor = 0;
   }
   else if (ver_2_1) {
      major = 2;
      minor = 1;
   }
   else if (ver_2_0) {
      major = 2;
      minor = 0;
   }
   else if (ver_1_5) {
      major = 1;
      minor = 5;
   }
   else if (ver_1_4) {
      major = 1;
      minor = 4;
   }
   else {
      major = 1;
      minor = 3;
   }

   version = major * 10 + minor;

   if (api == API_OPENGL_CORE && version < 31)
      return 0;

   return version;
}

static GLuint
compute_version_es2(const struct gl_extensions *extensions,
                    const struct gl_constants *consts)
{
   /* OpenGL ES 2.0 is derived from OpenGL 2.0 */
   const bool ver_2_0 = (extensions->ARB_vertex_shader &&
                         extensions->ARB_fragment_shader &&
                         extensions->ARB_texture_non_power_of_two &&
                         extensions->EXT_blend_equation_separate);
   /* FINISHME: This list isn't quite right. */
   const bool ver_3_0 = (extensions->ARB_half_float_vertex &&
                         extensions->ARB_internalformat_query &&
                         extensions->ARB_map_buffer_range &&
                         extensions->ARB_shader_texture_lod &&
                         extensions->OES_texture_float &&
                         extensions->OES_texture_half_float &&
                         extensions->OES_texture_half_float_linear &&
                         extensions->ARB_texture_rg &&
                         extensions->ARB_depth_buffer_float &&
                         extensions->ARB_framebuffer_object &&
                         extensions->EXT_sRGB &&
                         extensions->EXT_packed_float &&
                         extensions->EXT_texture_array &&
                         extensions->EXT_texture_shared_exponent &&
                         extensions->EXT_texture_sRGB &&
                         extensions->EXT_transform_feedback &&
                         extensions->ARB_draw_instanced &&
                         extensions->ARB_instanced_arrays &&
                         extensions->ARB_uniform_buffer_object &&
                         extensions->EXT_texture_snorm &&
                         (extensions->NV_primitive_restart ||
                          consts->PrimitiveRestartFixedIndex) &&
                         extensions->OES_depth_texture_cube_map &&
                         extensions->EXT_texture_type_2_10_10_10_REV &&
                         consts->MaxColorAttachments >= 4);
   const bool es31_compute_shader =
      consts->MaxComputeWorkGroupInvocations >= 128 &&
      consts->Program[MESA_SHADER_COMPUTE].MaxShaderStorageBlocks &&
      consts->Program[MESA_SHADER_COMPUTE].MaxAtomicBuffers &&
      consts->Program[MESA_SHADER_COMPUTE].MaxImageUniforms;
   const bool ver_3_1 = (ver_3_0 &&
                         consts->MaxVertexAttribStride >= 2048 &&
                         extensions->ARB_arrays_of_arrays &&
                         es31_compute_shader &&
                         extensions->ARB_draw_indirect &&
                         extensions->ARB_explicit_uniform_location &&
                         extensions->ARB_framebuffer_no_attachments &&
                         extensions->ARB_shading_language_packing &&
                         extensions->ARB_stencil_texturing &&
                         extensions->ARB_texture_multisample &&
                         extensions->ARB_texture_gather &&
                         extensions->MESA_shader_integer_functions &&
                         extensions->EXT_shader_integer_mix);
   const bool ver_3_2 = (ver_3_1 &&
                         /* ES 3.2 requires that images/buffers be accessible
                          * from fragment shaders as well
                          */
                         extensions->ARB_shader_atomic_counters &&
                         extensions->ARB_shader_image_load_store &&
                         extensions->ARB_shader_image_size &&
                         extensions->ARB_shader_storage_buffer_object &&
                         extensions->EXT_color_buffer_float &&
                         extensions->EXT_draw_buffers2 &&
                         extensions->KHR_blend_equation_advanced &&
                         extensions->KHR_robustness &&
                         extensions->KHR_texture_compression_astc_ldr &&
                         extensions->OES_copy_image &&
                         extensions->ARB_draw_buffers_blend &&
                         extensions->ARB_draw_elements_base_vertex &&
                         extensions->OES_geometry_shader &&
                         extensions->OES_primitive_bounding_box &&
                         extensions->OES_sample_variables &&
                         extensions->ARB_tessellation_shader &&
                         extensions->OES_texture_buffer &&
                         extensions->OES_texture_cube_map_array &&
                         extensions->ARB_texture_stencil8);

   if (ver_3_2) {
      return 32;
   } else if (ver_3_1) {
      return 31;
   } else if (ver_3_0) {
      return 30;
   } else if (ver_2_0) {
      return 20;
   } else {
      return 0;
   }
}

GLuint
_mesa_get_version(const struct gl_extensions *extensions,
                  struct gl_constants *consts, gl_api api)
{
   switch (api) {
   case API_OPENGL_COMPAT:
      /* Disable higher GLSL versions for legacy contexts.
       * This disallows creation of higher compatibility contexts. */
      if (!consts->AllowHigherCompatVersion) {
         consts->GLSLVersion = consts->GLSLVersionCompat;
      }
      FALLTHROUGH;
   case API_OPENGL_CORE:
      return compute_version(extensions, consts, api);
   case API_OPENGLES:
      return 11;
   case API_OPENGLES2:
      return compute_version_es2(extensions, consts);
   }
   return 0;
}

/**
 * Set the context's Version and VersionString fields.
 * This should only be called once as part of context initialization
 * or to perform version check for GLX_ARB_create_context_profile.
 */
void
_mesa_compute_version(struct gl_context *ctx)
{
   if (ctx->Version)
      goto done;

   ctx->Version = _mesa_get_version(&ctx->Extensions, &ctx->Const, ctx->API);
   ctx->Extensions.Version = ctx->Version;

   /* Make sure that the GLSL version lines up with the GL version. In some
    * cases it can be too high, e.g. if an extension is missing.
    */
   if (_mesa_is_desktop_gl(ctx)) {
      switch (ctx->Version) {
      case 20:
         FALLTHROUGH; /* GLSL 1.20 is the minimum we support */
      case 21:
         ctx->Const.GLSLVersion = 120;
         break;
      case 30:
         ctx->Const.GLSLVersion = 130;
         break;
      case 31:
         ctx->Const.GLSLVersion = 140;
         break;
      case 32:
         ctx->Const.GLSLVersion = 150;
         break;
      default:
         if (ctx->Version >= 33)
            ctx->Const.GLSLVersion = ctx->Version * 10;
         break;
      }
   }

   switch (ctx->API) {
   case API_OPENGL_COMPAT:
   case API_OPENGL_CORE:
      create_version_string(ctx, "");
      break;

   case API_OPENGLES:
      if (!ctx->Version) {
         _mesa_problem(ctx, "Incomplete OpenGL ES 1.0 support.");
         return;
      }
      create_version_string(ctx, "OpenGL ES-CM ");
      break;

   case API_OPENGLES2:
      if (!ctx->Version) {
         _mesa_problem(ctx, "Incomplete OpenGL ES 2.0 support.");
         return;
      }
      create_version_string(ctx, "OpenGL ES ");
      break;
   }

done:
   if (_mesa_is_desktop_gl_compat(ctx) && ctx->Version >= 31)
      ctx->Extensions.ARB_compatibility = GL_TRUE;

   /* Precompute valid primitive types for faster draw time validation. */
   /* All primitive type enums are less than 32, so we can use the shift. */
   ctx->SupportedPrimMask = (1 << GL_POINTS) |
                           (1 << GL_LINES) |
                           (1 << GL_LINE_LOOP) |
                           (1 << GL_LINE_STRIP) |
                           (1 << GL_TRIANGLES) |
                           (1 << GL_TRIANGLE_STRIP) |
                           (1 << GL_TRIANGLE_FAN);

   if (_mesa_is_desktop_gl_compat(ctx)) {
      ctx->SupportedPrimMask |= (1 << GL_QUADS) |
                               (1 << GL_QUAD_STRIP) |
                               (1 << GL_POLYGON);
   }

   if (_mesa_has_geometry_shaders(ctx)) {
      ctx->SupportedPrimMask |= (1 << GL_LINES_ADJACENCY) |
                               (1 << GL_LINE_STRIP_ADJACENCY) |
                               (1 << GL_TRIANGLES_ADJACENCY) |
                               (1 << GL_TRIANGLE_STRIP_ADJACENCY);
   }

   if (_mesa_has_tessellation(ctx))
      ctx->SupportedPrimMask |= 1 << GL_PATCHES;

   /* Appendix F.2 of the OpenGL ES 3.0 spec says:
    *
    *     "OpenGL ES 3.0 requires that all cube map filtering be
    *     seamless. OpenGL ES 2.0 specified that a single cube map face be
    *     selected and used for filtering."
    *
    * Now that we know our version, enable seamless filtering for GLES3 only.
    */
   ctx->Texture.CubeMapSeamless = _mesa_is_gles3(ctx);

   /* First time initialization. */
   _mesa_update_valid_to_render_state(ctx);
}


void
_mesa_get_driver_uuid(struct gl_context *ctx, GLint *uuid)
{
   struct pipe_screen *screen = ctx->pipe->screen;
   assert(GL_UUID_SIZE_EXT >= PIPE_UUID_SIZE);
   memset(uuid, 0, GL_UUID_SIZE_EXT);
   screen->get_driver_uuid(screen, (char *)uuid);
}

void
_mesa_get_device_uuid(struct gl_context *ctx, GLint *uuid)
{
   struct pipe_screen *screen = ctx->pipe->screen;
   assert(GL_UUID_SIZE_EXT >= PIPE_UUID_SIZE);
   memset(uuid, 0, GL_UUID_SIZE_EXT);
   screen->get_device_uuid(screen, (char *)uuid);
}

void
_mesa_get_device_luid(struct gl_context *ctx, GLint *luid)
{
   struct pipe_screen *screen = ctx->pipe->screen;
   assert(GL_LUID_SIZE_EXT >= PIPE_LUID_SIZE);
   memset(luid, 0, GL_UUID_SIZE_EXT);
   screen->get_device_luid(screen, (char *)luid);
}

/**
 * Get the i-th GLSL version string.  If index=0, return the most recent
 * supported version.
 * \param ctx context to query
 * \param index  which version string to return, or -1 if none
 * \param versionOut returns the vesrion string
 * \return total number of shading language versions.
 */
int
_mesa_get_shading_language_version(const struct gl_context *ctx,
                                   int index,
                                   char **versionOut)
{
   int n = 0;

#define GLSL_VERSION(S) \
   if (n++ == index) \
      *versionOut = S

   /* GLSL core */
   if (ctx->Const.GLSLVersion >= 460)
      GLSL_VERSION("460");
   if (ctx->Const.GLSLVersion >= 450)
      GLSL_VERSION("450");
   if (ctx->Const.GLSLVersion >= 440)
      GLSL_VERSION("440");
   if (ctx->Const.GLSLVersion >= 430)
      GLSL_VERSION("430");
   if (ctx->Const.GLSLVersion >= 420)
      GLSL_VERSION("420");
   if (ctx->Const.GLSLVersion >= 410)
      GLSL_VERSION("410");
   if (ctx->Const.GLSLVersion >= 400)
      GLSL_VERSION("400");
   if (ctx->Const.GLSLVersion >= 330)
      GLSL_VERSION("330");
   if (ctx->Const.GLSLVersion >= 150)
      GLSL_VERSION("150");
   if (ctx->Const.GLSLVersion >= 140)
      GLSL_VERSION("140");
   if (ctx->Const.GLSLVersion >= 130)
      GLSL_VERSION("130");
   if (ctx->Const.GLSLVersion >= 120)
      GLSL_VERSION("120");
   /* The GL spec says to return the empty string for GLSL 1.10 */
   if (ctx->Const.GLSLVersion >= 110)
      GLSL_VERSION("");

   /* GLSL es */
   if (_mesa_is_gles32(ctx) || ctx->Extensions.ARB_ES3_2_compatibility)
      GLSL_VERSION("320 es");
   if (_mesa_is_gles31(ctx) || ctx->Extensions.ARB_ES3_1_compatibility)
      GLSL_VERSION("310 es");
   if (_mesa_is_gles3(ctx) || ctx->Extensions.ARB_ES3_compatibility)
      GLSL_VERSION("300 es");
   if (_mesa_is_gles2(ctx) || ctx->Extensions.ARB_ES2_compatibility)
      GLSL_VERSION("100");

#undef GLSL_VERSION

   return n;
}
