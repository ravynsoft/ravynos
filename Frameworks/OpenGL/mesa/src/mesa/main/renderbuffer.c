/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2006  Brian Paul   All Rights Reserved.
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


#include "util/glheader.h"

#include "context.h"
#include "bufferobj.h"
#include "fbobject.h"
#include "formats.h"
#include "glformats.h"
#include "mtypes.h"
#include "renderbuffer.h"
#include "util/u_memory.h"
#include "util/u_inlines.h"

#include "state_tracker/st_context.h"
#include "state_tracker/st_format.h"

/**
 * Called by FBO code to choose a PIPE_FORMAT_ for drawing surfaces.
 */
static enum pipe_format
choose_renderbuffer_format(struct gl_context *ctx,
                           GLenum internalFormat, unsigned sample_count,
                           unsigned storage_sample_count)
{
   unsigned bindings;
   if (_mesa_is_depth_or_stencil_format(internalFormat))
      bindings = PIPE_BIND_DEPTH_STENCIL;
   else
      bindings = PIPE_BIND_RENDER_TARGET;
   return st_choose_format(st_context(ctx), internalFormat, GL_NONE, GL_NONE,
                           PIPE_TEXTURE_2D, sample_count,
                           storage_sample_count, bindings,
                           false, false);
}



/**
 * Delete a gl_framebuffer.
 * This is the default function for renderbuffer->Delete().
 * Drivers which subclass gl_renderbuffer should probably implement their
 * own delete function.  But the driver might also call this function to
 * free the object in the end.
 */
static void
delete_renderbuffer(struct gl_context *ctx, struct gl_renderbuffer *rb)
{
   if (ctx) {
      pipe_surface_release(ctx->pipe, &rb->surface_srgb);
      pipe_surface_release(ctx->pipe, &rb->surface_linear);
   } else {
      pipe_surface_release_no_context(&rb->surface_srgb);
      pipe_surface_release_no_context(&rb->surface_linear);
   }
   rb->surface = NULL;
   pipe_resource_reference(&rb->texture, NULL);
   free(rb->data);
   free(rb->Label);
   free(rb);
}

static GLboolean
renderbuffer_alloc_sw_storage(struct gl_context *ctx,
                              struct gl_renderbuffer *rb,
                              GLenum internalFormat,
                              GLuint width, GLuint height)
{
   enum pipe_format format;
   size_t size;

   free(rb->data);
   rb->data = NULL;

   if (internalFormat == GL_RGBA16_SNORM) {
      /* Special case for software accum buffers.  Otherwise, if the
       * call to choose_renderbuffer_format() fails (because the
       * driver doesn't support signed 16-bit/channel colors) we'd
       * just return without allocating the software accum buffer.
       */
      format = PIPE_FORMAT_R16G16B16A16_SNORM;
   }
   else {
      format = choose_renderbuffer_format(ctx, internalFormat, 0, 0);

      /* Not setting gl_renderbuffer::Format here will cause
       * FRAMEBUFFER_UNSUPPORTED and ValidateFramebuffer will not be called.
       */
      if (format == PIPE_FORMAT_NONE) {
         return GL_TRUE;
      }
   }

   rb->Format = st_pipe_format_to_mesa_format(format);

   size = _mesa_format_image_size(rb->Format, width, height, 1);
   rb->data = malloc(size);
   return rb->data != NULL;
}


/**
 * gl_renderbuffer::AllocStorage()
 * This is called to allocate the original drawing surface, and
 * during window resize.
 */
static GLboolean
renderbuffer_alloc_storage(struct gl_context * ctx,
                           struct gl_renderbuffer *rb,
                           GLenum internalFormat,
                           GLuint width, GLuint height)
{
   struct st_context *st = st_context(ctx);
   struct pipe_screen *screen = ctx->screen;
   enum pipe_format format = PIPE_FORMAT_NONE;
   struct pipe_resource templ;

   /* init renderbuffer fields */
   rb->Width  = width;
   rb->Height = height;
   rb->_BaseFormat = _mesa_base_fbo_format(ctx, internalFormat);
   rb->defined = GL_FALSE;  /* undefined contents now */

   if (rb->software) {
      return renderbuffer_alloc_sw_storage(ctx, rb, internalFormat,
                                           width, height);
   }

   /* Free the old surface and texture
    */
   pipe_surface_reference(&rb->surface_srgb, NULL);
   pipe_surface_reference(&rb->surface_linear, NULL);
   rb->surface = NULL;
   pipe_resource_reference(&rb->texture, NULL);

   /* If an sRGB framebuffer is unsupported, sRGB formats behave like linear
    * formats.
    */
   if (!ctx->Extensions.EXT_sRGB) {
      internalFormat = _mesa_get_linear_internalformat(internalFormat);
   }

   /* Handle multisample renderbuffers first.
    *
    * From ARB_framebuffer_object:
    *   If <samples> is zero, then RENDERBUFFER_SAMPLES is set to zero.
    *   Otherwise <samples> represents a request for a desired minimum
    *   number of samples. Since different implementations may support
    *   different sample counts for multisampled rendering, the actual
    *   number of samples allocated for the renderbuffer image is
    *   implementation dependent.  However, the resulting value for
    *   RENDERBUFFER_SAMPLES is guaranteed to be greater than or equal
    *   to <samples> and no more than the next larger sample count supported
    *   by the implementation.
    *
    * Find the supported number of samples >= rb->NumSamples
    */
   if (rb->NumSamples > 0) {
      unsigned start, start_storage;

      if (ctx->Const.MaxSamples > 1 &&  rb->NumSamples == 1) {
         /* don't try num_samples = 1 with drivers that support real msaa */
         start = 2;
         start_storage = 2;
      } else {
         start = rb->NumSamples;
         start_storage = rb->NumStorageSamples;
      }

      if (ctx->Extensions.AMD_framebuffer_multisample_advanced) {
         if (rb->_BaseFormat == GL_DEPTH_COMPONENT ||
             rb->_BaseFormat == GL_DEPTH_STENCIL ||
             rb->_BaseFormat == GL_STENCIL_INDEX) {
            /* Find a supported depth-stencil format. */
            for (unsigned samples = start;
                 samples <= ctx->Const.MaxDepthStencilFramebufferSamples;
                 samples++) {
               format = choose_renderbuffer_format(ctx, internalFormat,
                                                   samples, samples);

               if (format != PIPE_FORMAT_NONE) {
                  rb->NumSamples = samples;
                  rb->NumStorageSamples = samples;
                  break;
               }
            }
         } else {
            /* Find a supported color format, samples >= storage_samples. */
            for (unsigned storage_samples = start_storage;
                 storage_samples <= ctx->Const.MaxColorFramebufferStorageSamples;
                 storage_samples++) {
               for (unsigned samples = MAX2(start, storage_samples);
                    samples <= ctx->Const.MaxColorFramebufferSamples;
                    samples++) {
                  format = choose_renderbuffer_format(ctx, internalFormat,
                                                      samples,
                                                      storage_samples);

                  if (format != PIPE_FORMAT_NONE) {
                     rb->NumSamples = samples;
                     rb->NumStorageSamples = storage_samples;
                     goto found;
                  }
               }
            }
            found:;
         }
      } else {
         for (unsigned samples = start; samples <= ctx->Const.MaxSamples;
              samples++) {
            format = choose_renderbuffer_format(ctx, internalFormat,
                                                samples, samples);

            if (format != PIPE_FORMAT_NONE) {
               rb->NumSamples = samples;
               rb->NumStorageSamples = samples;
               break;
            }
         }
      }
   } else {
      format = choose_renderbuffer_format(ctx, internalFormat, 0, 0);
   }

   /* Not setting gl_renderbuffer::Format here will cause
    * FRAMEBUFFER_UNSUPPORTED and ValidateFramebuffer will not be called.
    */
   if (format == PIPE_FORMAT_NONE) {
      return GL_TRUE;
   }

   rb->Format = st_pipe_format_to_mesa_format(format);

   if (width == 0 || height == 0) {
      /* if size is zero, nothing to allocate */
      return GL_TRUE;
   }

   /* Setup new texture template.
    */
   memset(&templ, 0, sizeof(templ));
   templ.target = st->internal_target;
   templ.format = format;
   templ.width0 = width;
   templ.height0 = height;
   templ.depth0 = 1;
   templ.array_size = 1;
   templ.nr_samples = rb->NumSamples;
   templ.nr_storage_samples = rb->NumStorageSamples;

   if (util_format_is_depth_or_stencil(format)) {
      templ.bind = PIPE_BIND_DEPTH_STENCIL;
   }
   else if (rb->Name != 0) {
      /* this is a user-created renderbuffer */
      templ.bind = PIPE_BIND_RENDER_TARGET;
   }
   else {
      /* this is a window-system buffer */
      templ.bind = (PIPE_BIND_DISPLAY_TARGET |
                    PIPE_BIND_RENDER_TARGET);
   }

   rb->texture = screen->resource_create(screen, &templ);

   if (!rb->texture)
      return false;

   _mesa_update_renderbuffer_surface(ctx, rb);
   return rb->surface != NULL;
}

/**
 * Initialize the fields of a gl_renderbuffer to default values.
 */
void
_mesa_init_renderbuffer(struct gl_renderbuffer *rb, GLuint name)
{
   GET_CURRENT_CONTEXT(ctx);

   rb->Name = name;
   rb->RefCount = 1;
   rb->Delete = delete_renderbuffer;

   /* The rest of these should be set later by the caller of this function or
    * the AllocStorage method:
    */
   rb->AllocStorage = NULL;

   rb->Width = 0;
   rb->Height = 0;
   rb->Depth = 0;

   /* In GL 3, the initial format is GL_RGBA according to Table 6.26
    * on page 302 of the GL 3.3 spec.
    *
    * In GLES 3, the initial format is GL_RGBA4 according to Table 6.15
    * on page 258 of the GLES 3.0.4 spec.
    *
    * If the context is current, set the initial format based on the
    * specs. If the context is not current, we cannot determine the
    * API, so default to GL_RGBA.
    */
   if (ctx && _mesa_is_gles(ctx)) {
      rb->InternalFormat = GL_RGBA4;
   } else {
      rb->InternalFormat = GL_RGBA;
   }

   rb->Format = MESA_FORMAT_NONE;

   rb->AllocStorage = renderbuffer_alloc_storage;
}

static void
validate_and_init_renderbuffer_attachment(struct gl_framebuffer *fb,
                                          gl_buffer_index bufferName,
                                          struct gl_renderbuffer *rb)
{
   assert(fb);
   assert(rb);
   assert(bufferName < BUFFER_COUNT);

   /* There should be no previous renderbuffer on this attachment point,
    * with the exception of depth/stencil since the same renderbuffer may
    * be used for both.
    */
   assert(bufferName == BUFFER_DEPTH ||
          bufferName == BUFFER_STENCIL ||
          fb->Attachment[bufferName].Renderbuffer == NULL);

   /* winsys vs. user-created buffer cross check */
   if (_mesa_is_user_fbo(fb)) {
      assert(rb->Name);
   }
   else {
      assert(!rb->Name);
   }

   fb->Attachment[bufferName].Type = GL_RENDERBUFFER_EXT;
   fb->Attachment[bufferName].Complete = GL_TRUE;
}


/**
 * Attach a renderbuffer to a framebuffer.
 * \param bufferName  one of the BUFFER_x tokens
 *
 * This function avoids adding a reference and is therefore intended to be
 * used with a freshly created renderbuffer.
 */
void
_mesa_attach_and_own_rb(struct gl_framebuffer *fb,
                        gl_buffer_index bufferName,
                        struct gl_renderbuffer *rb)
{
   assert(rb->RefCount == 1);

   validate_and_init_renderbuffer_attachment(fb, bufferName, rb);

   _mesa_reference_renderbuffer(&fb->Attachment[bufferName].Renderbuffer,
                                NULL);
   fb->Attachment[bufferName].Renderbuffer = rb;
}

/**
 * Attach a renderbuffer to a framebuffer.
 * \param bufferName  one of the BUFFER_x tokens
 */
void
_mesa_attach_and_reference_rb(struct gl_framebuffer *fb,
                              gl_buffer_index bufferName,
                              struct gl_renderbuffer *rb)
{
   validate_and_init_renderbuffer_attachment(fb, bufferName, rb);
   _mesa_reference_renderbuffer(&fb->Attachment[bufferName].Renderbuffer, rb);
}


/**
 * Remove the named renderbuffer from the given framebuffer.
 * \param bufferName  one of the BUFFER_x tokens
 */
void
_mesa_remove_renderbuffer(struct gl_framebuffer *fb,
                          gl_buffer_index bufferName)
{
   assert(bufferName < BUFFER_COUNT);
   _mesa_reference_renderbuffer(&fb->Attachment[bufferName].Renderbuffer,
                                NULL);
}


/**
 * Set *ptr to point to rb.  If *ptr points to another renderbuffer,
 * dereference that buffer first.  The new renderbuffer's refcount will
 * be incremented.  The old renderbuffer's refcount will be decremented.
 * This is normally only called from the _mesa_reference_renderbuffer() macro
 * when there's a real pointer change.
 */
void
_mesa_reference_renderbuffer_(struct gl_renderbuffer **ptr,
                              struct gl_renderbuffer *rb)
{
   if (*ptr) {
      /* Unreference the old renderbuffer */
      struct gl_renderbuffer *oldRb = *ptr;

      assert(oldRb->RefCount > 0);

      if (p_atomic_dec_zero(&oldRb->RefCount)) {
         GET_CURRENT_CONTEXT(ctx);
         oldRb->Delete(ctx, oldRb);
      }
   }

   if (rb) {
      /* reference new renderbuffer */
      p_atomic_inc(&rb->RefCount);
   }

   *ptr = rb;
}

void
_mesa_map_renderbuffer(struct gl_context *ctx,
                       struct gl_renderbuffer *rb,
                       GLuint x, GLuint y, GLuint w, GLuint h,
                       GLbitfield mode,
                       GLubyte **mapOut, GLint *rowStrideOut,
                       bool flip_y)
{
   struct pipe_context *pipe = ctx->pipe;
   const GLboolean invert = flip_y;
   GLuint y2;
   GLubyte *map;

   if (rb->software) {
      /* software-allocated renderbuffer (probably an accum buffer) */
      if (rb->data) {
         GLint bpp = _mesa_get_format_bytes(rb->Format);
         GLint stride = _mesa_format_row_stride(rb->Format,
                                                rb->Width);
         *mapOut = (GLubyte *) rb->data + y * stride + x * bpp;
         *rowStrideOut = stride;
      }
      else {
         *mapOut = NULL;
         *rowStrideOut = 0;
      }
      return;
   }

   /* Check for unexpected flags */
   assert((mode & ~(GL_MAP_READ_BIT |
                    GL_MAP_WRITE_BIT |
                    GL_MAP_INVALIDATE_RANGE_BIT)) == 0);

   const enum pipe_map_flags transfer_flags =
      _mesa_access_flags_to_transfer_flags(mode, false);

   /* Note: y=0=bottom of buffer while y2=0=top of buffer.
    * 'invert' will be true for window-system buffers and false for
    * user-allocated renderbuffers and textures.
    */
   if (invert)
      y2 = rb->Height - y - h;
   else
      y2 = y;

    map = pipe_texture_map(pipe,
                            rb->texture,
                            rb->surface->u.tex.level,
                            rb->surface->u.tex.first_layer,
                            transfer_flags, x, y2, w, h, &rb->transfer);
   if (map) {
      if (invert) {
         *rowStrideOut = -(int) rb->transfer->stride;
         map += (h - 1) * rb->transfer->stride;
      }
      else {
         *rowStrideOut = rb->transfer->stride;
      }
      *mapOut = map;
   }
   else {
      *mapOut = NULL;
      *rowStrideOut = 0;
   }
}

void
_mesa_unmap_renderbuffer(struct gl_context *ctx,
                         struct gl_renderbuffer *rb)
{
   struct pipe_context *pipe = ctx->pipe;

   if (rb->software) {
      /* software-allocated renderbuffer (probably an accum buffer) */
      return;
   }

   pipe_texture_unmap(pipe, rb->transfer);
   rb->transfer = NULL;
}

void
_mesa_regen_renderbuffer_surface(struct gl_context *ctx,
                                 struct gl_renderbuffer *rb)
{
   struct pipe_context *pipe = ctx->pipe;
   struct pipe_resource *resource = rb->texture;

   struct pipe_surface **psurf =
      rb->surface_srgb ? &rb->surface_srgb : &rb->surface_linear;
   struct pipe_surface *surf = *psurf;
   /* create a new pipe_surface */
   struct pipe_surface surf_tmpl;
   memset(&surf_tmpl, 0, sizeof(surf_tmpl));
   surf_tmpl.format = surf->format;
   surf_tmpl.nr_samples = rb->rtt_nr_samples;
   surf_tmpl.u.tex.level = surf->u.tex.level;
   surf_tmpl.u.tex.first_layer = surf->u.tex.first_layer;
   surf_tmpl.u.tex.last_layer = surf->u.tex.last_layer;

   /* create -> destroy to avoid blowing up cached surfaces */
   surf = pipe->create_surface(pipe, resource, &surf_tmpl);
   pipe_surface_release(pipe, psurf);
   *psurf = surf;

   rb->surface = *psurf;
}

/**
 * Create or update the pipe_surface of a FBO renderbuffer.
 * This is usually called after st_finalize_texture.
 */
void
_mesa_update_renderbuffer_surface(struct gl_context *ctx,
                                  struct gl_renderbuffer *rb)
{
   struct pipe_context *pipe = ctx->pipe;
   struct pipe_resource *resource = rb->texture;
   const struct gl_texture_object *stTexObj = NULL;
   unsigned rtt_width = rb->Width;
   unsigned rtt_height = rb->Height;
   unsigned rtt_depth = rb->Depth;

   /*
    * For winsys fbo, it is possible that the renderbuffer is sRGB-capable but
    * the format of rb->texture is linear (because we have no control over
    * the format).  Check rb->Format instead of rb->texture->format
    * to determine if the rb is sRGB-capable.
    */
   bool enable_srgb = ctx->Color.sRGBEnabled &&
      _mesa_is_format_srgb(rb->Format);
   enum pipe_format format = resource->format;

   if (rb->is_rtt) {
      stTexObj = rb->TexImage->TexObject;
      if (stTexObj->surface_based)
         format = stTexObj->surface_format;
   }

   format = enable_srgb ? util_format_srgb(format) : util_format_linear(format);

   if (resource->target == PIPE_TEXTURE_1D_ARRAY) {
      rtt_depth = rtt_height;
      rtt_height = 1;
   }

   /* find matching mipmap level size */
   unsigned level;
   for (level = 0; level <= resource->last_level; level++) {
      if (u_minify(resource->width0, level) == rtt_width &&
          u_minify(resource->height0, level) == rtt_height &&
          (resource->target != PIPE_TEXTURE_3D ||
           u_minify(resource->depth0, level) == rtt_depth)) {
         break;
      }
   }
   assert(level <= resource->last_level);

   /* determine the layer bounds */
   unsigned first_layer, last_layer;
   if (rb->rtt_layered) {
      first_layer = 0;
      last_layer = util_max_layer(rb->texture, level);
   }
   else {
      first_layer =
      last_layer = rb->rtt_face + rb->rtt_slice;
   }

   /* Adjust for texture views */
   if (rb->is_rtt && resource->array_size > 1 &&
       stTexObj->Immutable) {
      const struct gl_texture_object *tex = stTexObj;
      first_layer += tex->Attrib.MinLayer;
      if (!rb->rtt_layered)
         last_layer += tex->Attrib.MinLayer;
      else
         last_layer = MIN2(first_layer + tex->Attrib.NumLayers - 1,
                           last_layer);
   }

   struct pipe_surface **psurf =
      enable_srgb ? &rb->surface_srgb : &rb->surface_linear;
   struct pipe_surface *surf = *psurf;

   if (!surf ||
       surf->texture->nr_samples != rb->NumSamples ||
       surf->texture->nr_storage_samples != rb->NumStorageSamples ||
       surf->format != format ||
       surf->texture != resource ||
       surf->width != rtt_width ||
       surf->height != rtt_height ||
       surf->nr_samples != rb->rtt_nr_samples ||
       surf->u.tex.level != level ||
       surf->u.tex.first_layer != first_layer ||
       surf->u.tex.last_layer != last_layer) {
      /* create a new pipe_surface */
      struct pipe_surface surf_tmpl;
      memset(&surf_tmpl, 0, sizeof(surf_tmpl));
      surf_tmpl.format = format;
      surf_tmpl.nr_samples = rb->rtt_nr_samples;
      surf_tmpl.u.tex.level = level;
      surf_tmpl.u.tex.first_layer = first_layer;
      surf_tmpl.u.tex.last_layer = last_layer;

      /* create -> destroy to avoid blowing up cached surfaces */
      struct pipe_surface *surf = pipe->create_surface(pipe, resource, &surf_tmpl);
      pipe_surface_release(pipe, psurf);
      *psurf = surf;
   }
   rb->surface = *psurf;
}
