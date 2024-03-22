/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 2010 LunarG Inc.
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Chia-I Wu <olv@lunarg.com>
 */

#include <GL/internal/dri_interface.h>
#include "main/errors.h"
#include "main/texobj.h"
#include "main/teximage.h"
#include "util/u_inlines.h"
#include "util/format/u_format.h"
#include "st_cb_eglimage.h"
#include "st_cb_texture.h"
#include "st_context.h"
#include "st_texture.h"
#include "st_format.h"
#include "st_manager.h"
#include "st_sampler_view.h"
#include "util/u_surface.h"

static bool
is_format_supported(struct pipe_screen *screen, enum pipe_format format,
                    unsigned nr_samples, unsigned nr_storage_samples,
                    unsigned usage, bool *native_supported)
{
   bool supported = screen->is_format_supported(screen, format, PIPE_TEXTURE_2D,
                                                nr_samples, nr_storage_samples,
                                                usage);
   *native_supported = supported;

   /* for sampling, some formats can be emulated.. it doesn't matter that
    * the surface will have a format that the driver can't cope with because
    * we'll give it sampler view formats that it can deal with and generate
    * a shader variant that converts.
    */
   if ((usage == PIPE_BIND_SAMPLER_VIEW) && !supported) {
      switch (format) {
      case PIPE_FORMAT_IYUV:
         supported = screen->is_format_supported(screen, PIPE_FORMAT_R8_UNORM,
                                                 PIPE_TEXTURE_2D, nr_samples,
                                                 nr_storage_samples, usage);
         break;
      case PIPE_FORMAT_NV12:
      case PIPE_FORMAT_NV21:
         supported = screen->is_format_supported(screen, PIPE_FORMAT_R8_UNORM,
                                                 PIPE_TEXTURE_2D, nr_samples,
                                                 nr_storage_samples, usage) &&
                     screen->is_format_supported(screen, PIPE_FORMAT_R8G8_UNORM,
                                                 PIPE_TEXTURE_2D, nr_samples,
                                                 nr_storage_samples, usage);
         break;
      case PIPE_FORMAT_P010:
      case PIPE_FORMAT_P012:
      case PIPE_FORMAT_P016:
      case PIPE_FORMAT_P030:
         supported = screen->is_format_supported(screen, PIPE_FORMAT_R16_UNORM,
                                                 PIPE_TEXTURE_2D, nr_samples,
                                                 nr_storage_samples, usage) &&
                     screen->is_format_supported(screen, PIPE_FORMAT_R16G16_UNORM,
                                                 PIPE_TEXTURE_2D, nr_samples,
                                                 nr_storage_samples, usage);
         break;
      case PIPE_FORMAT_Y210:
      case PIPE_FORMAT_Y212:
      case PIPE_FORMAT_Y216:
         supported = screen->is_format_supported(screen, PIPE_FORMAT_R16G16_UNORM,
                                                 PIPE_TEXTURE_2D, nr_samples,
                                                 nr_storage_samples, usage) &&
                     screen->is_format_supported(screen, PIPE_FORMAT_R16G16B16A16_UNORM,
                                                 PIPE_TEXTURE_2D, nr_samples,
                                                 nr_storage_samples, usage);
         break;
      case PIPE_FORMAT_Y410:
         supported = screen->is_format_supported(screen, PIPE_FORMAT_R10G10B10A2_UNORM,
                                                 PIPE_TEXTURE_2D, nr_samples,
                                                 nr_storage_samples, usage);
         break;
      case PIPE_FORMAT_Y412:
      case PIPE_FORMAT_Y416:
         supported = screen->is_format_supported(screen, PIPE_FORMAT_R16G16B16A16_UNORM,
                                                 PIPE_TEXTURE_2D, nr_samples,
                                                 nr_storage_samples, usage);
         break;
      case PIPE_FORMAT_YUYV:
         supported = screen->is_format_supported(screen, PIPE_FORMAT_R8G8_R8B8_UNORM,
                                                 PIPE_TEXTURE_2D, nr_samples,
                                                 nr_storage_samples, usage) ||
                     (screen->is_format_supported(screen, PIPE_FORMAT_RG88_UNORM,
                                                  PIPE_TEXTURE_2D, nr_samples,
                                                  nr_storage_samples, usage) &&
                      screen->is_format_supported(screen, PIPE_FORMAT_BGRA8888_UNORM,
                                                  PIPE_TEXTURE_2D, nr_samples,
                                                  nr_storage_samples, usage));
         break;
      case PIPE_FORMAT_YVYU:
         supported = screen->is_format_supported(screen, PIPE_FORMAT_R8B8_R8G8_UNORM,
                                                 PIPE_TEXTURE_2D, nr_samples,
                                                 nr_storage_samples, usage) ||
                     (screen->is_format_supported(screen, PIPE_FORMAT_RG88_UNORM,
                                                  PIPE_TEXTURE_2D, nr_samples,
                                                  nr_storage_samples, usage) &&
                      screen->is_format_supported(screen, PIPE_FORMAT_BGRA8888_UNORM,
                                                  PIPE_TEXTURE_2D, nr_samples,
                                                  nr_storage_samples, usage));
         break;
      case PIPE_FORMAT_UYVY:
         supported = screen->is_format_supported(screen, PIPE_FORMAT_G8R8_B8R8_UNORM,
                                                 PIPE_TEXTURE_2D, nr_samples,
                                                 nr_storage_samples, usage) ||
                     (screen->is_format_supported(screen, PIPE_FORMAT_RG88_UNORM,
                                                  PIPE_TEXTURE_2D, nr_samples,
                                                  nr_storage_samples, usage) &&
                      screen->is_format_supported(screen, PIPE_FORMAT_RGBA8888_UNORM,
                                                  PIPE_TEXTURE_2D, nr_samples,
                                                  nr_storage_samples, usage));
         break;
      case PIPE_FORMAT_VYUY:
         supported = screen->is_format_supported(screen, PIPE_FORMAT_B8R8_G8R8_UNORM,
                                                 PIPE_TEXTURE_2D, nr_samples,
                                                 nr_storage_samples, usage) ||
                     (screen->is_format_supported(screen, PIPE_FORMAT_RG88_UNORM,
                                                  PIPE_TEXTURE_2D, nr_samples,
                                                  nr_storage_samples, usage) &&
                      screen->is_format_supported(screen, PIPE_FORMAT_RGBA8888_UNORM,
                                                  PIPE_TEXTURE_2D, nr_samples,
                                                  nr_storage_samples, usage));
         break;
      case PIPE_FORMAT_AYUV:
         supported = screen->is_format_supported(screen, PIPE_FORMAT_RGBA8888_UNORM,
                                                 PIPE_TEXTURE_2D, nr_samples,
                                                 nr_storage_samples, usage);
         break;
      case PIPE_FORMAT_XYUV:
         supported = screen->is_format_supported(screen, PIPE_FORMAT_RGBX8888_UNORM,
                                                 PIPE_TEXTURE_2D, nr_samples,
                                                 nr_storage_samples, usage);
         break;
       default:
         break;
      }
   }

   return supported;
}

static bool
is_nv12_as_r8_g8b8_supported(struct pipe_screen *screen, struct st_egl_image *out,
                             unsigned usage, bool *native_supported)
{
   if (out->format == PIPE_FORMAT_NV12 &&
       out->texture->format == PIPE_FORMAT_R8_G8B8_420_UNORM &&
       screen->is_format_supported(screen, PIPE_FORMAT_R8_G8B8_420_UNORM,
                                   PIPE_TEXTURE_2D,
                                   out->texture->nr_samples,
                                   out->texture->nr_storage_samples,
                                   usage)) {
      *native_supported = false;
      return true;
   }

   if (out->format == PIPE_FORMAT_NV21 &&
       out->texture->format == PIPE_FORMAT_R8_B8G8_420_UNORM &&
       screen->is_format_supported(screen, PIPE_FORMAT_R8_B8G8_420_UNORM,
                                   PIPE_TEXTURE_2D,
                                   out->texture->nr_samples,
                                   out->texture->nr_storage_samples,
                                   usage)) {
      *native_supported = false;
      return true;
   }

   return false;
}

static bool
is_i420_as_r8_g8_b8_420_supported(struct pipe_screen *screen,
                                  struct st_egl_image *out,
                                  unsigned usage, bool *native_supported)
{
   if (out->format == PIPE_FORMAT_IYUV &&
       out->texture->format == PIPE_FORMAT_R8_G8_B8_420_UNORM &&
       screen->is_format_supported(screen, PIPE_FORMAT_R8_G8_B8_420_UNORM,
                                   PIPE_TEXTURE_2D,
                                   out->texture->nr_samples,
                                   out->texture->nr_storage_samples,
                                   usage)) {
      *native_supported = false;
      return true;
   }

   if (out->format == PIPE_FORMAT_IYUV &&
       out->texture->format == PIPE_FORMAT_R8_B8_G8_420_UNORM &&
       screen->is_format_supported(screen, PIPE_FORMAT_R8_B8_G8_420_UNORM,
                                   PIPE_TEXTURE_2D,
                                   out->texture->nr_samples,
                                   out->texture->nr_storage_samples,
                                   usage)) {
      *native_supported = false;
      return true;
   }

   return false;
}

/**
 * Return the gallium texture of an EGLImage.
 */
bool
st_get_egl_image(struct gl_context *ctx, GLeglImageOES image_handle,
                 unsigned usage, const char *error, struct st_egl_image *out,
                 bool *native_supported)
{
   struct st_context *st = st_context(ctx);
   struct pipe_screen *screen = st->screen;
   struct pipe_frontend_screen *fscreen = st->frontend_screen;

   if (!fscreen || !fscreen->get_egl_image)
      return false;

   memset(out, 0, sizeof(*out));
   if (!fscreen->get_egl_image(fscreen, (void *) image_handle, out)) {
      /* image_handle does not refer to a valid EGL image object */
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(image handle not found)", error);
      return false;
   }

   if (!is_nv12_as_r8_g8b8_supported(screen, out, usage, native_supported) &&
       !is_i420_as_r8_g8_b8_420_supported(screen, out, usage, native_supported) &&
       !is_format_supported(screen, out->format, out->texture->nr_samples,
                            out->texture->nr_storage_samples, usage,
                            native_supported)) {
      /* unable to specify a texture object using the specified EGL image */
      pipe_resource_reference(&out->texture, NULL);
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s(format not supported)", error);
      return false;
   }

   ctx->Shared->HasExternallySharedImages = true;
   return true;
}

/**
 * Return the base format just like _mesa_base_fbo_format does.
 */
static GLenum
st_pipe_format_to_base_format(enum pipe_format format)
{
   GLenum base_format;

   if (util_format_is_depth_or_stencil(format)) {
      if (util_format_is_depth_and_stencil(format)) {
         base_format = GL_DEPTH_STENCIL;
      }
      else {
         if (format == PIPE_FORMAT_S8_UINT)
            base_format = GL_STENCIL_INDEX;
         else
            base_format = GL_DEPTH_COMPONENT;
      }
   }
   else {
      /* is this enough? */
      if (util_format_has_alpha(format))
         base_format = GL_RGBA;
      else
         base_format = GL_RGB;
   }

   return base_format;
}

void
st_egl_image_target_renderbuffer_storage(struct gl_context *ctx,
                                         struct gl_renderbuffer *rb,
                                         GLeglImageOES image_handle)
{
   struct st_egl_image stimg;
   bool native_supported;

   if (st_get_egl_image(ctx, image_handle, PIPE_BIND_RENDER_TARGET,
                        "glEGLImageTargetRenderbufferStorage",
                        &stimg, &native_supported)) {
      struct pipe_context *pipe = st_context(ctx)->pipe;
      struct pipe_surface *ps, surf_tmpl;

      u_surface_default_template(&surf_tmpl, stimg.texture);
      surf_tmpl.format = stimg.format;
      surf_tmpl.u.tex.level = stimg.level;
      surf_tmpl.u.tex.first_layer = stimg.layer;
      surf_tmpl.u.tex.last_layer = stimg.layer;
      ps = pipe->create_surface(pipe, stimg.texture, &surf_tmpl);
      pipe_resource_reference(&stimg.texture, NULL);

      if (!ps)
         return;

      rb->Format = st_pipe_format_to_mesa_format(ps->format);
      rb->_BaseFormat = st_pipe_format_to_base_format(ps->format);
      rb->InternalFormat = rb->_BaseFormat;

      st_set_ws_renderbuffer_surface(rb, ps);
      pipe_surface_reference(&ps, NULL);
   }
}

void
st_bind_egl_image(struct gl_context *ctx,
                  struct gl_texture_object *texObj,
                  struct gl_texture_image *texImage,
                  struct st_egl_image *stimg,
                  bool tex_storage,
                  bool native_supported)
{
   struct st_context *st = st_context(ctx);
   GLenum internalFormat;
   mesa_format texFormat;

   if (stimg->texture->target != gl_target_to_pipe(texObj->Target)) {
      _mesa_error(ctx, GL_INVALID_OPERATION, __func__);
      return;
   }

   if (stimg->internalformat) {
      internalFormat = stimg->internalformat;
   } else {
      /* map pipe format to base format */
      if (util_format_get_component_bits(stimg->format,
                                         UTIL_FORMAT_COLORSPACE_RGB, 3) > 0)
         internalFormat = GL_RGBA;
      else
         internalFormat = GL_RGB;
   }

   /* switch to surface based */
   if (!texObj->surface_based) {
      _mesa_clear_texture_object(ctx, texObj, NULL);
      texObj->surface_based = GL_TRUE;
   }

   /* TODO RequiredTextureImageUnits should probably be reset back
    * to 1 somewhere if different texture is bound??
    */
   if (!native_supported) {
      switch (stimg->format) {
      case PIPE_FORMAT_NV12:
      case PIPE_FORMAT_NV21:
         if (stimg->texture->format == PIPE_FORMAT_R8_G8B8_420_UNORM ||
             stimg->texture->format == PIPE_FORMAT_R8_B8G8_420_UNORM) {
            texFormat = MESA_FORMAT_R8G8B8X8_UNORM;
            texObj->RequiredTextureImageUnits = 1;
         } else {
            texFormat = MESA_FORMAT_R_UNORM8;
            texObj->RequiredTextureImageUnits = 2;
         }
         break;
      case PIPE_FORMAT_P010:
      case PIPE_FORMAT_P012:
      case PIPE_FORMAT_P016:
      case PIPE_FORMAT_P030:
         texFormat = MESA_FORMAT_R_UNORM16;
         texObj->RequiredTextureImageUnits = 2;
         break;
      case PIPE_FORMAT_Y210:
      case PIPE_FORMAT_Y212:
      case PIPE_FORMAT_Y216:
         texFormat = MESA_FORMAT_RG_UNORM16;
         texObj->RequiredTextureImageUnits = 2;
         break;
      case PIPE_FORMAT_Y410:
         texFormat = MESA_FORMAT_B10G10R10A2_UNORM;
         internalFormat = GL_RGBA;
         texObj->RequiredTextureImageUnits = 1;
         break;
      case PIPE_FORMAT_Y412:
      case PIPE_FORMAT_Y416:
         texFormat = MESA_FORMAT_RGBA_UNORM16;
         internalFormat = GL_RGBA;
         texObj->RequiredTextureImageUnits = 1;
         break;
      case PIPE_FORMAT_IYUV:
         if (stimg->texture->format == PIPE_FORMAT_R8_G8_B8_420_UNORM ||
             stimg->texture->format == PIPE_FORMAT_R8_B8_G8_420_UNORM) {
            texFormat = MESA_FORMAT_R8G8B8X8_UNORM;
            texObj->RequiredTextureImageUnits = 1;
         } else {
            texFormat = MESA_FORMAT_R_UNORM8;
            texObj->RequiredTextureImageUnits = 3;
         }
         break;
      case PIPE_FORMAT_YUYV:
      case PIPE_FORMAT_YVYU:
      case PIPE_FORMAT_UYVY:
      case PIPE_FORMAT_VYUY:
         if (stimg->texture->format == PIPE_FORMAT_R8G8_R8B8_UNORM) {
            texFormat = MESA_FORMAT_RG_RB_UNORM8;
            texObj->RequiredTextureImageUnits = 1;
         } else if (stimg->texture->format == PIPE_FORMAT_R8B8_R8G8_UNORM) {
            texFormat = MESA_FORMAT_RB_RG_UNORM8;
            texObj->RequiredTextureImageUnits = 1;
         } else if (stimg->texture->format == PIPE_FORMAT_G8R8_B8R8_UNORM) {
            texFormat = MESA_FORMAT_GR_BR_UNORM8;
            texObj->RequiredTextureImageUnits = 1;
         } else if (stimg->texture->format == PIPE_FORMAT_B8R8_G8R8_UNORM) {
            texFormat = MESA_FORMAT_BR_GR_UNORM8;
            texObj->RequiredTextureImageUnits = 1;
         } else {
            texFormat = MESA_FORMAT_RG_UNORM8;
            texObj->RequiredTextureImageUnits = 2;
         }
         break;
      case PIPE_FORMAT_AYUV:
         texFormat = MESA_FORMAT_R8G8B8A8_UNORM;
         internalFormat = GL_RGBA;
         texObj->RequiredTextureImageUnits = 1;
         break;
      case PIPE_FORMAT_XYUV:
         texFormat = MESA_FORMAT_R8G8B8X8_UNORM;
         texObj->RequiredTextureImageUnits = 1;
         break;
      default:
         unreachable("unexpected emulated format");
         break;
      }
   } else {
      texFormat = st_pipe_format_to_mesa_format(stimg->format);
      /* Use previously derived internalformat as specified by
       * EXT_EGL_image_storage.
       */
      if (tex_storage && texObj->Target == GL_TEXTURE_2D
          && stimg->internalformat) {
         internalFormat = stimg->internalformat;
         if (internalFormat == GL_NONE) {
            _mesa_error(ctx, GL_INVALID_OPERATION, __func__);
            return;
         }
      }
   }
   assert(texFormat != MESA_FORMAT_NONE);


   /* Minify texture size based on level set on the EGLImage. */
   uint32_t width = u_minify(stimg->texture->width0, stimg->level);
   uint32_t height = u_minify(stimg->texture->height0, stimg->level);

   _mesa_init_teximage_fields(ctx, texImage, width, height,
                              1, 0, internalFormat, texFormat);

   pipe_resource_reference(&texObj->pt, stimg->texture);
   st_texture_release_all_sampler_views(st, texObj);
   pipe_resource_reference(&texImage->pt, texObj->pt);
   if (st->screen->resource_changed)
      st->screen->resource_changed(st->screen, texImage->pt);

   texObj->surface_format = stimg->format;

   switch (stimg->yuv_color_space) {
   case __DRI_YUV_COLOR_SPACE_ITU_REC709:
      texObj->yuv_color_space = GL_TEXTURE_YUV_COLOR_SPACE_REC709;
      break;
   case __DRI_YUV_COLOR_SPACE_ITU_REC2020:
      texObj->yuv_color_space = GL_TEXTURE_YUV_COLOR_SPACE_REC2020;
      break;
   default:
      texObj->yuv_color_space = GL_TEXTURE_YUV_COLOR_SPACE_REC601;
      break;
   }

   if (stimg->yuv_range == __DRI_YUV_FULL_RANGE)
      texObj->yuv_full_range = true;

   texObj->level_override = stimg->level;
   texObj->layer_override = stimg->layer;
   _mesa_update_texture_object_swizzle(ctx, texObj);

   _mesa_dirty_texobj(ctx, texObj);
}

static GLboolean
st_validate_egl_image(struct gl_context *ctx, GLeglImageOES image_handle)
{
   struct st_context *st = st_context(ctx);
   struct pipe_frontend_screen *fscreen = st->frontend_screen;

   return fscreen->validate_egl_image(fscreen, (void *)image_handle);
}

void
st_init_eglimage_functions(struct dd_function_table *functions,
                           bool has_egl_image_validate)
{
   if (has_egl_image_validate)
      functions->ValidateEGLImage = st_validate_egl_image;
}
