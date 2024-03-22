/*
 * Copyright 2016 VMware, Inc.
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
 */

#include "pipe/p_context.h"
#include "util/format/u_format.h"
#include "util/u_inlines.h"

#include "main/context.h"
#include "main/macros.h"
#include "main/mtypes.h"
#include "main/teximage.h"
#include "main/texobj.h"
#include "program/prog_instruction.h"

#include "st_context.h"
#include "st_sampler_view.h"
#include "st_texture.h"
#include "st_format.h"
#include "st_cb_texture.h"

/* Subtract remaining private references. Typically used before
 * destruction. See the header file for explanation.
 */
static void
st_remove_private_references(struct st_sampler_view *sv)
{
   if (sv->private_refcount) {
      assert(sv->private_refcount > 0);
      p_atomic_add(&sv->view->reference.count, -sv->private_refcount);
      sv->private_refcount = 0;
   }
}

/* Return a sampler view while incrementing the refcount by 1. */
static struct pipe_sampler_view *
get_sampler_view_reference(struct st_sampler_view *sv,
                           struct pipe_sampler_view *view)
{
   if (unlikely(sv->private_refcount <= 0)) {
      assert(sv->private_refcount == 0);

      /* This is the number of atomic increments we will skip. */
      sv->private_refcount = 100000000;
      p_atomic_add(&view->reference.count, sv->private_refcount);
   }

   /* Return a reference while decrementing the private refcount. */
   sv->private_refcount--;
   return view;
}

/**
 * Set the given view as the current context's view for the texture.
 *
 * Overwrites any pre-existing view of the context.
 *
 * Takes ownership of the view (i.e., stores the view without incrementing the
 * reference count).
 *
 * \return the view, or NULL on error. In case of error, the reference to the
 * view is released.
 */
static struct pipe_sampler_view *
st_texture_set_sampler_view(struct st_context *st,
                            struct gl_texture_object *stObj,
                            struct pipe_sampler_view *view,
                            bool glsl130_or_later, bool srgb_skip_decode,
                            bool get_reference, bool locked)
{
   struct st_sampler_views *views;
   struct st_sampler_view *free = NULL;
   struct st_sampler_view *sv;
   GLuint i;

   if (!locked)
      simple_mtx_lock(&stObj->validate_mutex);
   views = stObj->sampler_views;

   for (i = 0; i < views->count; ++i) {
      sv = &views->views[i];

      /* Is the array entry used ? */
      if (sv->view) {
         /* check if the context matches */
         if (sv->view->context == st->pipe) {
            st_remove_private_references(sv);
            pipe_sampler_view_reference(&sv->view, NULL);
            goto found;
         }
      } else {
         /* Found a free slot, remember that */
         free = sv;
      }
   }

   /* Couldn't find a slot for our context, create a new one */
   if (free) {
      sv = free;
   } else {
      if (views->count >= views->max) {
         /* Allocate a larger container. */
         unsigned new_max = 2 * views->max;
         unsigned new_size = sizeof(*views) + new_max * sizeof(views->views[0]);

         if (new_max < views->max ||
             new_max > (UINT_MAX - sizeof(*views)) / sizeof(views->views[0])) {
            pipe_sampler_view_reference(&view, NULL);
            goto out;
         }

         struct st_sampler_views *new_views = malloc(new_size);
         if (!new_views) {
            pipe_sampler_view_reference(&view, NULL);
            goto out;
         }

         new_views->count = views->count;
         new_views->max = new_max;
         memcpy(&new_views->views[0], &views->views[0],
               views->count * sizeof(views->views[0]));

         /* Initialize the pipe_sampler_view pointers to zero so that we don't
          * have to worry about racing against readers when incrementing
          * views->count.
          */
         memset(&new_views->views[views->count], 0,
                (new_max - views->count) * sizeof(views->views[0]));

         /* Use memory release semantics to ensure that concurrent readers will
          * get the correct contents of the new container.
          *
          * Also, the write should be atomic, but that's guaranteed anyway on
          * all supported platforms.
          */
         p_atomic_set(&stObj->sampler_views, new_views);

         /* We keep the old container around until the texture object is
          * deleted, because another thread may still be reading from it. We
          * double the size of the container each time, so we end up with
          * at most twice the total memory allocation.
          */
         views->next = stObj->sampler_views_old;
         stObj->sampler_views_old = views;

         views = new_views;
      }

      sv = &views->views[views->count];

      /* Since modification is guarded by the lock, only the write part of the
       * increment has to be atomic, and that's already guaranteed on all
       * supported platforms without using an atomic intrinsic.
       */
      views->count++;
   }

found:
   assert(sv->view == NULL);

   sv->glsl130_or_later = glsl130_or_later;
   sv->srgb_skip_decode = srgb_skip_decode;
   sv->view = view;
   sv->st = st;

   if (get_reference)
      view = get_sampler_view_reference(sv, view);

out:
   if (!locked)
      simple_mtx_unlock(&stObj->validate_mutex);
   return view;
}


/**
 * Return the most-recently validated sampler view for the texture \p stObj
 * in the given context, if any.
 *
 * Performs no additional validation.
 */
struct st_sampler_view *
st_texture_get_current_sampler_view(const struct st_context *st,
                                    const struct gl_texture_object *stObj)
{
   struct st_sampler_views *views = p_atomic_read(&stObj->sampler_views);

   for (unsigned i = 0; i < views->count; ++i) {
      struct st_sampler_view *sv = &views->views[i];
      if (sv->view && sv->view->context == st->pipe)
         return sv;
   }

   return NULL;
}


/**
 * For the given texture object, release any sampler views which belong
 * to the calling context.  This is used to free any sampler views
 * which belong to the context before the context is destroyed.
 */
void
st_texture_release_context_sampler_view(struct st_context *st,
                                        struct gl_texture_object *stObj)
{
   GLuint i;

   simple_mtx_lock(&stObj->validate_mutex);
   struct st_sampler_views *views = stObj->sampler_views;
   for (i = 0; i < views->count; ++i) {
      struct st_sampler_view *sv = &views->views[i];

      if (sv->view && sv->view->context == st->pipe) {
         st_remove_private_references(sv);
         pipe_sampler_view_reference(&sv->view, NULL);
         break;
      }
   }
   simple_mtx_unlock(&stObj->validate_mutex);
}


/**
 * Release all sampler views attached to the given texture object, regardless
 * of the context.  This is called fairly frequently.  For example, whenever
 * the texture's base level, max level or swizzle change.
 */
void
st_texture_release_all_sampler_views(struct st_context *st,
                                     struct gl_texture_object *stObj)
{
   /* TODO: This happens while a texture is deleted, because the Driver API
    * is asymmetric: the driver allocates the texture object memory, but
    * mesa/main frees it.
    */
   if (!stObj->sampler_views)
      return;

   simple_mtx_lock(&stObj->validate_mutex);
   struct st_sampler_views *views = stObj->sampler_views;
   for (unsigned i = 0; i < views->count; ++i) {
      struct st_sampler_view *stsv = &views->views[i];
      if (stsv->view) {
         st_remove_private_references(stsv);

         if (stsv->st && stsv->st != st) {
            /* Transfer this reference to the zombie list.  It will
             * likely be freed when the zombie list is freed.
             */
            st_save_zombie_sampler_view(stsv->st, stsv->view);
            stsv->view = NULL;
         } else {
            pipe_sampler_view_reference(&stsv->view, NULL);
         }
      }
   }
   views->count = 0;
   simple_mtx_unlock(&stObj->validate_mutex);
}


/*
 * Delete the texture's sampler views and st_sampler_views containers.
 * This is to be called just before a texture is deleted.
 */
void
st_delete_texture_sampler_views(struct st_context *st,
                                struct gl_texture_object *stObj)
{
   st_texture_release_all_sampler_views(st, stObj);

   /* Free the container of the current per-context sampler views */
   assert(stObj->sampler_views->count == 0);
   free(stObj->sampler_views);
   stObj->sampler_views = NULL;

   /* Free old sampler view containers */
   while (stObj->sampler_views_old) {
      struct st_sampler_views *views = stObj->sampler_views_old;
      stObj->sampler_views_old = views->next;
      free(views);
   }
}

/**
 * Return TRUE if the texture's sampler view swizzle is not equal to
 * the texture's swizzle.
 *
 * \param texObj  the st texture object,
 */
ASSERTED static bool
check_sampler_swizzle(const struct st_context *st,
                      const struct gl_texture_object *texObj,
                      const struct pipe_sampler_view *sv,
                      bool glsl130_or_later)
{
   unsigned swizzle = glsl130_or_later ? texObj->SwizzleGLSL130 : texObj->Swizzle;

   return ((sv->swizzle_r != GET_SWZ(swizzle, 0)) ||
           (sv->swizzle_g != GET_SWZ(swizzle, 1)) ||
           (sv->swizzle_b != GET_SWZ(swizzle, 2)) ||
           (sv->swizzle_a != GET_SWZ(swizzle, 3)));
}


static unsigned
last_level(const struct gl_texture_object *texObj)
{
   unsigned ret = MIN2(texObj->Attrib.MinLevel + texObj->_MaxLevel,
                       texObj->pt->last_level);
   if (texObj->Immutable)
      ret = MIN2(ret, texObj->Attrib.MinLevel +
                 texObj->Attrib.NumLevels - 1);
   return ret;
}


static unsigned
last_layer(const struct gl_texture_object *texObj)
{
   if (texObj->Immutable && texObj->pt->array_size > 1)
      return MIN2(texObj->Attrib.MinLayer +
                  texObj->Attrib.NumLayers - 1,
                  texObj->pt->array_size - 1);
   return texObj->pt->array_size - 1;
}


/**
 * Determine the format for the texture sampler view.
 */
enum pipe_format
st_get_sampler_view_format(const struct st_context *st,
                           const struct gl_texture_object *texObj,
                           bool srgb_skip_decode)
{
   enum pipe_format format;

   GLenum baseFormat = _mesa_base_tex_image(texObj)->_BaseFormat;
   format = texObj->surface_based ? texObj->surface_format : texObj->pt->format;

   /* From OpenGL 4.3 spec, "Combined Depth/Stencil Textures":
    *
    *    "The DEPTH_STENCIL_TEXTURE_MODE is ignored for non
    *     depth/stencil textures.
    */
   const bool has_combined_ds =
      baseFormat == GL_DEPTH_STENCIL;

   if (baseFormat == GL_DEPTH_COMPONENT ||
       baseFormat == GL_DEPTH_STENCIL ||
       baseFormat == GL_STENCIL_INDEX) {
      if ((texObj->StencilSampling && has_combined_ds) ||
          baseFormat == GL_STENCIL_INDEX)
         format = util_format_stencil_only(format);

      return format;
   }

   /* If sRGB decoding is off, use the linear format */
   if (srgb_skip_decode)
      format = util_format_linear(format);

   /* if resource format matches then YUV wasn't lowered */
   if (format == texObj->pt->format)
      return format;

   /* Use R8_UNORM for video formats */
   switch (format) {
   case PIPE_FORMAT_NV12:
      if (texObj->pt->format == PIPE_FORMAT_R8_G8B8_420_UNORM) {
         format = PIPE_FORMAT_R8_G8B8_420_UNORM;
         break;
      }
      FALLTHROUGH;
   case PIPE_FORMAT_NV21:
      if (texObj->pt->format == PIPE_FORMAT_R8_B8G8_420_UNORM) {
         format = PIPE_FORMAT_R8_B8G8_420_UNORM;
         break;
      }
      FALLTHROUGH;
   case PIPE_FORMAT_IYUV:
      if (texObj->pt->format == PIPE_FORMAT_R8_G8_B8_420_UNORM ||
          texObj->pt->format == PIPE_FORMAT_R8_B8_G8_420_UNORM) {
         format = texObj->pt->format;
         break;
      }
      format = PIPE_FORMAT_R8_UNORM;
      break;
   case PIPE_FORMAT_P010:
   case PIPE_FORMAT_P012:
   case PIPE_FORMAT_P016:
   case PIPE_FORMAT_P030:
      format = PIPE_FORMAT_R16_UNORM;
      break;
   case PIPE_FORMAT_Y210:
   case PIPE_FORMAT_Y212:
   case PIPE_FORMAT_Y216:
      format = PIPE_FORMAT_R16G16_UNORM;
      break;
   case PIPE_FORMAT_Y410:
      format = PIPE_FORMAT_R10G10B10A2_UNORM;
      break;
   case PIPE_FORMAT_Y412:
   case PIPE_FORMAT_Y416:
      format = PIPE_FORMAT_R16G16B16A16_UNORM;
      break;
   case PIPE_FORMAT_YUYV:
   case PIPE_FORMAT_YVYU:
   case PIPE_FORMAT_UYVY:
   case PIPE_FORMAT_VYUY:
      if (texObj->pt->format == PIPE_FORMAT_R8G8_R8B8_UNORM ||
          texObj->pt->format == PIPE_FORMAT_R8B8_R8G8_UNORM ||
          texObj->pt->format == PIPE_FORMAT_B8R8_G8R8_UNORM ||
          texObj->pt->format == PIPE_FORMAT_G8R8_B8R8_UNORM) {
         format = texObj->pt->format;
         break;
      }
      format = PIPE_FORMAT_R8G8_UNORM;
      break;
   case PIPE_FORMAT_AYUV:
      format = PIPE_FORMAT_RGBA8888_UNORM;
      break;
   case PIPE_FORMAT_XYUV:
      format = PIPE_FORMAT_RGBX8888_UNORM;
      break;
   default:
      break;
   }
   return format;
}


static struct pipe_sampler_view *
st_create_texture_sampler_view_from_stobj(struct st_context *st,
                                          struct gl_texture_object *texObj,
                                          enum pipe_format format,
                                          bool glsl130_or_later)
{
   /* There is no need to clear this structure (consider CPU overhead). */
   struct pipe_sampler_view templ;
   unsigned swizzle = glsl130_or_later ? texObj->SwizzleGLSL130 : texObj->Swizzle;

   templ.format = format;
   templ.is_tex2d_from_buf = false;

   if (texObj->level_override >= 0) {
      templ.u.tex.first_level = templ.u.tex.last_level = texObj->level_override;
   } else {
      templ.u.tex.first_level = texObj->Attrib.MinLevel +
                                texObj->Attrib.BaseLevel;
      templ.u.tex.last_level = last_level(texObj);
   }
   if (texObj->layer_override >= 0) {
      templ.u.tex.first_layer = templ.u.tex.last_layer = texObj->layer_override;
   } else {
      templ.u.tex.first_layer = texObj->Attrib.MinLayer;
      templ.u.tex.last_layer = last_layer(texObj);
   }
   assert(templ.u.tex.first_layer <= templ.u.tex.last_layer);
   assert(templ.u.tex.first_level <= templ.u.tex.last_level);
   templ.target = gl_target_to_pipe(texObj->Target);

   templ.swizzle_r = GET_SWZ(swizzle, 0);
   templ.swizzle_g = GET_SWZ(swizzle, 1);
   templ.swizzle_b = GET_SWZ(swizzle, 2);
   templ.swizzle_a = GET_SWZ(swizzle, 3);

   return st->pipe->create_sampler_view(st->pipe, texObj->pt, &templ);
}

struct pipe_sampler_view *
st_get_texture_sampler_view_from_stobj(struct st_context *st,
                                       struct gl_texture_object *texObj,
                                       const struct gl_sampler_object *samp,
                                       bool glsl130_or_later,
                                       bool ignore_srgb_decode,
                                       bool get_reference)
{
   struct st_sampler_view *sv;
   bool srgb_skip_decode = false;

   if (!ignore_srgb_decode && samp->Attrib.sRGBDecode == GL_SKIP_DECODE_EXT)
      srgb_skip_decode = true;

   simple_mtx_lock(&texObj->validate_mutex);
   sv = st_texture_get_current_sampler_view(st, texObj);

   if (sv &&
       sv->glsl130_or_later == glsl130_or_later &&
       sv->srgb_skip_decode == srgb_skip_decode) {
      /* Debug check: make sure that the sampler view's parameters are
       * what they're supposed to be.
       */
      struct pipe_sampler_view *view = sv->view;
      assert(texObj->pt == view->texture);
      assert(!check_sampler_swizzle(st, texObj, view, glsl130_or_later));
      assert(st_get_sampler_view_format(st, texObj, srgb_skip_decode) == view->format);
      assert(gl_target_to_pipe(texObj->Target) == view->target);
      assert(texObj->level_override >= 0 ||
             texObj->Attrib.MinLevel +
             texObj->Attrib.BaseLevel == view->u.tex.first_level);
      assert(texObj->level_override >= 0 || last_level(texObj) == view->u.tex.last_level);
      assert(texObj->layer_override >= 0 ||
             texObj->Attrib.MinLayer == view->u.tex.first_layer);
      assert(texObj->layer_override >= 0 || last_layer(texObj) == view->u.tex.last_layer);
      assert(texObj->layer_override < 0 ||
             (texObj->layer_override == view->u.tex.first_layer &&
              texObj->layer_override == view->u.tex.last_layer));
      if (get_reference)
         view = get_sampler_view_reference(sv, view);
      simple_mtx_unlock(&texObj->validate_mutex);
      return view;
   }

   /* create new sampler view */
   enum pipe_format format = st_get_sampler_view_format(st, texObj,
                                                        srgb_skip_decode);
   struct pipe_sampler_view *view =
         st_create_texture_sampler_view_from_stobj(st, texObj, format,
                                                   glsl130_or_later);

   view = st_texture_set_sampler_view(st, texObj, view,
                                      glsl130_or_later, srgb_skip_decode,
                                      get_reference, true);
   simple_mtx_unlock(&texObj->validate_mutex);

   return view;
}


struct pipe_sampler_view *
st_get_buffer_sampler_view_from_stobj(struct st_context *st,
                                      struct gl_texture_object *texObj,
                                      bool get_reference)
{
   struct st_sampler_view *sv;
   struct gl_buffer_object *stBuf =
      texObj->BufferObject;

   if (!stBuf || !stBuf->buffer)
      return NULL;

   sv = st_texture_get_current_sampler_view(st, texObj);

   struct pipe_resource *buf = stBuf->buffer;

   if (sv) {
      struct pipe_sampler_view *view = sv->view;

      if (view->texture == buf) {
         /* Debug check: make sure that the sampler view's parameters are
          * what they're supposed to be.
          */
         assert(st_mesa_format_to_pipe_format(st,
                                              texObj->_BufferObjectFormat)
             == view->format);
         assert(view->target == PIPE_BUFFER);
         ASSERTED unsigned base = texObj->BufferOffset;
         ASSERTED unsigned size = MIN2(buf->width0 - base,
                           (unsigned) texObj->BufferSize);
         assert(view->u.buf.offset == base);
         assert(view->u.buf.size == size);
         if (get_reference)
            view = get_sampler_view_reference(sv, view);
         return view;
      }
   }

   unsigned base = texObj->BufferOffset;

   if (base >= buf->width0)
      return NULL;

   unsigned size = buf->width0 - base;
   size = MIN2(size, (unsigned)texObj->BufferSize);
   if (!size)
      return NULL;

   /* Create a new sampler view. There is no need to clear the entire
    * structure (consider CPU overhead).
    */
   struct pipe_sampler_view templ;

   templ.is_tex2d_from_buf = false;
   templ.format =
      st_mesa_format_to_pipe_format(st, texObj->_BufferObjectFormat);
   templ.target = PIPE_BUFFER;
   templ.swizzle_r = PIPE_SWIZZLE_X;
   templ.swizzle_g = PIPE_SWIZZLE_Y;
   templ.swizzle_b = PIPE_SWIZZLE_Z;
   templ.swizzle_a = PIPE_SWIZZLE_W;
   templ.u.buf.offset = base;
   templ.u.buf.size = size;

   struct pipe_sampler_view *view =
      st->pipe->create_sampler_view(st->pipe, buf, &templ);

   view = st_texture_set_sampler_view(st, texObj, view, false, false,
                                      get_reference, false);

   return view;
}
