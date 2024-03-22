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

#include "main/bufferobj.h"
#include "main/image.h"
#include "main/pbo.h"

#include "main/readpix.h"
#include "main/enums.h"
#include "main/framebuffer.h"
#include "util/u_inlines.h"
#include "util/format/u_format.h"
#include "cso_cache/cso_context.h"

#include "st_atom.h"
#include "st_context.h"
#include "st_cb_bitmap.h"
#include "st_cb_readpixels.h"
#include "st_debug.h"
#include "state_tracker/st_cb_texture.h"
#include "state_tracker/st_format.h"
#include "state_tracker/st_pbo.h"
#include "state_tracker/st_texture.h"
#include "state_tracker/st_util.h"


/* The readpixels cache caches a blitted staging texture so that back-to-back
 * calls to glReadPixels with user pointers require less CPU-GPU synchronization.
 *
 * Assumptions:
 *
 * (1) Blits have high synchronization overheads, and it is beneficial to
 *     use a single blit of the entire framebuffer instead of many smaller
 *     blits (because the smaller blits cannot be batched, and we have to wait
 *     for the GPU after each one).
 *
 * (2) texture_map implicitly involves a blit as well (for de-tiling, copy
 *     from VRAM, etc.), so that it is beneficial to replace the
 *     _mesa_readpixels path as well when possible.
 *
 * Change this #define to true to fill and use the cache whenever possible
 * (this is inefficient and only meant for testing / debugging).
 */
#define ALWAYS_READPIXELS_CACHE false

static bool
needs_integer_signed_unsigned_conversion(const struct gl_context *ctx,
                                         GLenum format, GLenum type)
{
   struct gl_renderbuffer *rb =
      _mesa_get_read_renderbuffer_for_format(ctx, format);

   assert(rb);

   GLenum srcType = _mesa_get_format_datatype(rb->Format);

    if ((srcType == GL_INT &&
        (type == GL_UNSIGNED_INT ||
         type == GL_UNSIGNED_SHORT ||
         type == GL_UNSIGNED_BYTE)) ||
       (srcType == GL_UNSIGNED_INT &&
        (type == GL_INT ||
         type == GL_SHORT ||
         type == GL_BYTE))) {
      return true;
   }

   return false;
}

static bool
try_pbo_readpixels(struct st_context *st, struct gl_renderbuffer *rb,
                   bool invert_y,
                   GLint x, GLint y, GLsizei width, GLsizei height,
                   GLenum gl_format,
                   enum pipe_format src_format, enum pipe_format dst_format,
                   const struct gl_pixelstore_attrib *pack, void *pixels)
{
   struct pipe_context *pipe = st->pipe;
   struct pipe_screen *screen = st->screen;
   struct cso_context *cso = st->cso_context;
   struct pipe_surface *surface = rb->surface;
   struct pipe_resource *texture = rb->texture;
   const struct util_format_description *desc;
   struct st_pbo_addresses addr;
   struct pipe_framebuffer_state fb;
   enum pipe_texture_target view_target;
   bool success = false;

   /* Make sure we have stencil format in case of GL_STENCIL_INDEX to
    * create correct type of a sampler view.
    */
   if (gl_format == GL_STENCIL_INDEX)
      src_format = util_format_stencil_only(src_format);

   if (texture->nr_samples > 1)
      return false;

   if (!screen->is_format_supported(screen, dst_format, PIPE_BUFFER, 0, 0,
                                    PIPE_BIND_SHADER_IMAGE))
      return false;

   desc = util_format_description(dst_format);

   /* Compute PBO addresses */
   addr.bytes_per_pixel = desc->block.bits / 8;
   addr.xoffset = x;
   addr.yoffset = y;
   addr.width = width;
   addr.height = height;
   addr.depth = 1;
   if (!st_pbo_addresses_pixelstore(st, GL_TEXTURE_2D, false, pack, pixels, &addr))
      return false;

   cso_save_state(cso, (CSO_BIT_FRAGMENT_SAMPLERS |
                        CSO_BIT_BLEND |
                        CSO_BIT_VERTEX_ELEMENTS |
                        CSO_BIT_FRAMEBUFFER |
                        CSO_BIT_VIEWPORT |
                        CSO_BIT_RASTERIZER |
                        CSO_BIT_DEPTH_STENCIL_ALPHA |
                        CSO_BIT_STREAM_OUTPUTS |
                        (st->active_queries ? CSO_BIT_PAUSE_QUERIES : 0) |
                        CSO_BIT_SAMPLE_MASK |
                        CSO_BIT_MIN_SAMPLES |
                        CSO_BIT_RENDER_CONDITION |
                        CSO_BITS_ALL_SHADERS));

   cso_set_sample_mask(cso, ~0);
   cso_set_min_samples(cso, 1);
   cso_set_render_condition(cso, NULL, false, 0);

   /* Set up the sampler_view */
   {
      struct pipe_sampler_view templ;
      struct pipe_sampler_view *sampler_view;
      struct pipe_sampler_state sampler = {0};
      const struct pipe_sampler_state *samplers[1] = {&sampler};

      u_sampler_view_default_template(&templ, texture, src_format);

      switch (texture->target) {
      case PIPE_TEXTURE_CUBE:
      case PIPE_TEXTURE_CUBE_ARRAY:
         view_target = PIPE_TEXTURE_2D_ARRAY;
         break;
      default:
         view_target = texture->target;
         break;
      }

      templ.target = view_target;
      templ.u.tex.first_level = surface->u.tex.level;
      templ.u.tex.last_level = templ.u.tex.first_level;

      if (view_target != PIPE_TEXTURE_3D) {
         templ.u.tex.first_layer = surface->u.tex.first_layer;
         templ.u.tex.last_layer = templ.u.tex.first_layer;
      } else {
         addr.constants.layer_offset = surface->u.tex.first_layer;
      }

      sampler_view = pipe->create_sampler_view(pipe, texture, &templ);
      if (sampler_view == NULL)
         goto fail;

      pipe->set_sampler_views(pipe, PIPE_SHADER_FRAGMENT, 0, 1, 0,
                              false, &sampler_view);
      st->state.num_sampler_views[PIPE_SHADER_FRAGMENT] =
         MAX2(st->state.num_sampler_views[PIPE_SHADER_FRAGMENT], 1);

      pipe_sampler_view_reference(&sampler_view, NULL);

      cso_set_samplers(cso, PIPE_SHADER_FRAGMENT, 1, samplers);
   }

   /* Set up destination image */
   {
      struct pipe_image_view image;

      memset(&image, 0, sizeof(image));
      image.resource = addr.buffer;
      image.format = dst_format;
      image.access = PIPE_IMAGE_ACCESS_WRITE;
      image.shader_access = PIPE_IMAGE_ACCESS_WRITE;
      image.u.buf.offset = addr.first_element * addr.bytes_per_pixel;
      image.u.buf.size = (addr.last_element - addr.first_element + 1) *
                         addr.bytes_per_pixel;

      pipe->set_shader_images(pipe, PIPE_SHADER_FRAGMENT, 0, 1, 0, &image);
   }

   /* Set up no-attachment framebuffer */
   memset(&fb, 0, sizeof(fb));
   fb.width = surface->width;
   fb.height = surface->height;
   fb.samples = 1;
   fb.layers = addr.depth;
   cso_set_framebuffer(cso, &fb);

   /* Any blend state would do. Set this just to prevent drivers having
    * blend == NULL.
    */
   cso_set_blend(cso, &st->pbo.upload_blend);

   cso_set_viewport_dims(cso, fb.width, fb.height, invert_y);

   if (invert_y)
      st_pbo_addresses_invert_y(&addr, fb.height);

   {
      struct pipe_depth_stencil_alpha_state dsa;
      memset(&dsa, 0, sizeof(dsa));
      cso_set_depth_stencil_alpha(cso, &dsa);
   }

   /* Set up the fragment shader */
   {
      void *fs = st_pbo_get_download_fs(st, view_target, src_format, dst_format, addr.depth != 1);
      if (!fs)
         goto fail;

      cso_set_fragment_shader_handle(cso, fs);
   }

   success = st_pbo_draw(st, &addr, fb.width, fb.height);

   /* Buffer written via shader images needs explicit synchronization. */
   pipe->memory_barrier(pipe, PIPE_BARRIER_ALL);

fail:
   /* Unbind all because st/mesa won't do it if the current shader doesn't
    * use them.
    */
   cso_restore_state(cso, CSO_UNBIND_FS_SAMPLERVIEWS | CSO_UNBIND_FS_IMAGE0);
   st->state.num_sampler_views[PIPE_SHADER_FRAGMENT] = 0;

   st->ctx->Array.NewVertexElements = true;
   st->ctx->NewDriverState |= ST_NEW_FS_CONSTANTS |
                              ST_NEW_FS_IMAGES |
                              ST_NEW_FS_SAMPLER_VIEWS |
                              ST_NEW_VERTEX_ARRAYS;

   return success;
}

/**
 * Create a staging texture and blit the requested region to it.
 */
static struct pipe_resource *
blit_to_staging(struct st_context *st, struct gl_renderbuffer *rb,
                   bool invert_y,
                   GLint x, GLint y, GLsizei width, GLsizei height,
                   GLenum format,
                   enum pipe_format src_format, enum pipe_format dst_format)
{
   struct pipe_screen *screen = st->screen;
   struct pipe_resource dst_templ;
   struct pipe_resource *dst;
   struct pipe_blit_info blit;

   /* We are creating a texture of the size of the region being read back.
    * Need to check for NPOT texture support. */
   if (!screen->get_param(screen, PIPE_CAP_NPOT_TEXTURES) &&
       (!util_is_power_of_two_or_zero(width) ||
        !util_is_power_of_two_or_zero(height)))
      return NULL;

   /* create the destination texture */
   memset(&dst_templ, 0, sizeof(dst_templ));
   dst_templ.target = PIPE_TEXTURE_2D;
   dst_templ.format = dst_format;
   if (util_format_is_depth_or_stencil(dst_format))
      dst_templ.bind |= PIPE_BIND_DEPTH_STENCIL;
   else
      dst_templ.bind |= PIPE_BIND_RENDER_TARGET;
   dst_templ.usage = PIPE_USAGE_STAGING;

   st_gl_texture_dims_to_pipe_dims(GL_TEXTURE_2D, width, height, 1,
                                   &dst_templ.width0, &dst_templ.height0,
                                   &dst_templ.depth0, &dst_templ.array_size);

   dst = screen->resource_create(screen, &dst_templ);
   if (!dst)
      return NULL;

   memset(&blit, 0, sizeof(blit));
   blit.src.resource = rb->texture;
   blit.src.level = rb->surface->u.tex.level;
   blit.src.format = src_format;
   blit.dst.resource = dst;
   blit.dst.level = 0;
   blit.dst.format = dst->format;
   blit.src.box.x = x;
   blit.dst.box.x = 0;
   blit.src.box.y = y;
   blit.dst.box.y = 0;
   blit.src.box.z = rb->surface->u.tex.first_layer;
   blit.dst.box.z = 0;
   blit.src.box.width = blit.dst.box.width = width;
   blit.src.box.height = blit.dst.box.height = height;
   blit.src.box.depth = blit.dst.box.depth = 1;
   blit.mask = st_get_blit_mask(rb->_BaseFormat, format);
   blit.filter = PIPE_TEX_FILTER_NEAREST;
   blit.scissor_enable = false;

   if (invert_y) {
      blit.src.box.y = rb->Height - blit.src.box.y;
      blit.src.box.height = -blit.src.box.height;
   }

   /* blit */
   st->pipe->blit(st->pipe, &blit);

   return dst;
}

static struct pipe_resource *
try_cached_readpixels(struct st_context *st, struct gl_renderbuffer *rb,
                      bool invert_y,
                      GLsizei width, GLsizei height,
                      GLenum format,
                      enum pipe_format src_format, enum pipe_format dst_format)
{
   struct pipe_resource *src = rb->texture;
   struct pipe_resource *dst = NULL;

   if (ST_DEBUG & DEBUG_NOREADPIXCACHE)
      return NULL;

   /* Reset cache after invalidation or switch of parameters. */
   if (st->readpix_cache.src != src ||
       st->readpix_cache.dst_format != dst_format ||
       st->readpix_cache.level != rb->surface->u.tex.level ||
       st->readpix_cache.layer != rb->surface->u.tex.first_layer) {
      pipe_resource_reference(&st->readpix_cache.src, src);
      pipe_resource_reference(&st->readpix_cache.cache, NULL);
      st->readpix_cache.dst_format = dst_format;
      st->readpix_cache.level = rb->surface->u.tex.level;
      st->readpix_cache.layer = rb->surface->u.tex.first_layer;
      st->readpix_cache.hits = 0;
   }

   /* Decide whether to trigger the cache. */
   if (!st->readpix_cache.cache) {
      if (!rb->use_readpix_cache && !ALWAYS_READPIXELS_CACHE) {
         /* Heuristic: If previous successive calls read at least a fraction
          * of the surface _and_ we read again, trigger the cache.
          */
         unsigned threshold = MAX2(1, rb->Width * rb->Height / 8);

         if (st->readpix_cache.hits < threshold) {
            st->readpix_cache.hits += width * height;
            return NULL;
         }

         rb->use_readpix_cache = true;
      }

      /* Fill the cache */
      st->readpix_cache.cache = blit_to_staging(st, rb, invert_y,
                                                0, 0,
                                                rb->Width,
                                                rb->Height, format,
                                                src_format, dst_format);
   }

   /* Return an owning reference to stay consistent with the non-cached path */
   pipe_resource_reference(&dst, st->readpix_cache.cache);

   return dst;
}

/**
 * This uses a blit to copy the read buffer to a texture format which matches
 * the format and type combo and then a fast read-back is done using memcpy.
 * We can do arbitrary X/Y/Z/W/0/1 swizzling here as long as there is
 * a format which matches the swizzling.
 *
 * If such a format isn't available, we fall back to _mesa_readpixels.
 *
 * NOTE: Some drivers use a blit to convert between tiled and linear
 *       texture layouts during texture uploads/downloads, so the blit
 *       we do here should be free in such cases.
 */
void
st_ReadPixels(struct gl_context *ctx, GLint x, GLint y,
              GLsizei width, GLsizei height,
              GLenum format, GLenum type,
              const struct gl_pixelstore_attrib *pack,
              void *pixels)
{
   struct st_context *st = st_context(ctx);
   struct gl_renderbuffer *rb =
         _mesa_get_read_renderbuffer_for_format(ctx, format);
   struct pipe_context *pipe = st->pipe;
   struct pipe_screen *screen = st->screen;
   struct pipe_resource *src;
   struct pipe_resource *dst = NULL;
   enum pipe_format dst_format, src_format;
   unsigned bind;
   struct pipe_transfer *tex_xfer;
   uint8_t *map = NULL;
   int dst_x, dst_y;

   if (rb == NULL)
      return;

   /* Validate state (to be sure we have up-to-date framebuffer surfaces)
    * and flush the bitmap cache prior to reading. */
   st_validate_state(st, ST_PIPELINE_UPDATE_FB_STATE_MASK);
   st_flush_bitmap_cache(st);

   if (!st->prefer_blit_based_texture_transfer) {
      goto fallback;
   }

   /* This must be done after state validation. */
   src = rb->texture;

   /* XXX Fallback for depth-stencil formats due to an incomplete
    * stencil blit implementation in some drivers. */
   if (format == GL_DEPTH_STENCIL) {
      goto fallback;
   }

   /* If the base internal format and the texture format don't match, we have
    * to use the slow path. */
   if (rb->_BaseFormat !=
       _mesa_get_format_base_format(rb->Format)) {
      goto fallback;
   }

   if (_mesa_readpixels_needs_slow_path(ctx, format, type, GL_TRUE)) {
      goto fallback;
   }

   /* Convert the source format to what is expected by ReadPixels
    * and see if it's supported. */
   src_format = util_format_linear(rb->Format);
   src_format = util_format_luminance_to_red(src_format);
   src_format = util_format_intensity_to_red(src_format);

   if (!src_format ||
       !screen->is_format_supported(screen, src_format, src->target,
                                    src->nr_samples, src->nr_storage_samples,
                                    PIPE_BIND_SAMPLER_VIEW)) {
      goto fallback;
   }

   if (format == GL_DEPTH_COMPONENT || format == GL_DEPTH_STENCIL)
      bind = PIPE_BIND_DEPTH_STENCIL;
   else
      bind = PIPE_BIND_RENDER_TARGET;

   /* Choose the destination format by finding the best match
    * for the format+type combo. */
   dst_format = st_choose_matching_format(st, bind, format, type,
                                          pack->SwapBytes);
   if (dst_format == PIPE_FORMAT_NONE) {
      goto fallback;
   }

   if (st->pbo.download_enabled && pack->BufferObj) {
      if (try_pbo_readpixels(st, rb,
                             _mesa_fb_orientation(ctx->ReadBuffer) == Y_0_TOP,
                             x, y, width, height,
                             format, src_format, dst_format,
                             pack, pixels))
         return;
   }

   if (needs_integer_signed_unsigned_conversion(ctx, format, type)) {
      goto fallback;
   }

   /* Cache a staging texture for back-to-back ReadPixels, to avoid CPU-GPU
    * synchronization overhead.
    */
   dst = try_cached_readpixels(st, rb,
                               _mesa_fb_orientation(ctx->ReadBuffer) == Y_0_TOP,
                               width, height, format, src_format, dst_format);
   if (dst) {
      dst_x = x;
      dst_y = y;
   } else {
      /* See if the texture format already matches the format and type,
       * in which case the memcpy-based fast path will likely be used and
       * we don't have to blit. */
      if (_mesa_format_matches_format_and_type(rb->Format, format,
                                               type, pack->SwapBytes, NULL)) {
         goto fallback;
      }

      dst = blit_to_staging(st, rb,
                            _mesa_fb_orientation(ctx->ReadBuffer) == Y_0_TOP,
                            x, y, width, height, format,
                            src_format, dst_format);
      if (!dst)
         goto fallback;

      dst_x = 0;
      dst_y = 0;
   }

   /* map resources */
   pixels = _mesa_map_pbo_dest(ctx, pack, pixels);

   map = pipe_texture_map_3d(pipe, dst, 0, PIPE_MAP_READ,
                              dst_x, dst_y, 0, width, height, 1, &tex_xfer);
   if (!map) {
      _mesa_unmap_pbo_dest(ctx, pack);
      pipe_resource_reference(&dst, NULL);
      goto fallback;
   }

   /* memcpy data into a user buffer */
   {
      const uint bytesPerRow = width * util_format_get_blocksize(dst_format);
      const int destStride = _mesa_image_row_stride(pack, width, format, type);
      char *dest = _mesa_image_address2d(pack, pixels,
                                         width, height, format,
                                         type, 0, 0);

      if (tex_xfer->stride == bytesPerRow && destStride == bytesPerRow) {
         memcpy(dest, map, bytesPerRow * height);
      } else {
         GLuint row;

         for (row = 0; row < (unsigned) height; row++) {
            memcpy(dest, map, bytesPerRow);
            map += tex_xfer->stride;
            dest += destStride;
         }
      }
   }

   pipe_texture_unmap(pipe, tex_xfer);
   _mesa_unmap_pbo_dest(ctx, pack);
   pipe_resource_reference(&dst, NULL);
   return;

fallback:
   _mesa_readpixels(ctx, x, y, width, height, format, type, pack, pixels);
}
