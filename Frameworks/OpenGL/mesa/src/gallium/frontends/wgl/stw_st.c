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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Chia-I Wu <olv@lunarg.com>
 */

#include "util/u_memory.h"
#include "util/u_inlines.h"
#include "util/u_atomic.h"
#include "pipe/p_state.h"

#include "state_tracker/st_context.h"

#include "stw_st.h"
#include "stw_device.h"
#include "stw_framebuffer.h"
#include "stw_pixelformat.h"
#include "stw_winsys.h"

#ifdef GALLIUM_ZINK
#include <vulkan/vulkan.h>
#include "kopper_interface.h"
#endif

struct stw_st_framebuffer {
   struct pipe_frontend_drawable base;

   struct stw_framebuffer *fb;
   struct st_visual stvis;

   struct pipe_resource *textures[ST_ATTACHMENT_COUNT];
   struct pipe_resource *msaa_textures[ST_ATTACHMENT_COUNT];
   struct pipe_resource *back_texture;
   bool needs_fake_front;
   unsigned texture_width, texture_height;
   unsigned texture_mask;
};

static uint32_t stwfb_ID = 0;

/**
 * Is the given mutex held by the calling thread?
 */
bool
stw_own_mutex(const CRITICAL_SECTION *cs)
{
   // We can't compare OwningThread with our thread handle/id (see
   // http://stackoverflow.com/a/12675635 ) but we can compare with the
   // OwningThread member of a critical section we know we own.
   CRITICAL_SECTION dummy;
   InitializeCriticalSection(&dummy);
   EnterCriticalSection(&dummy);
   if (0)
      _debug_printf("%p %p\n", cs->OwningThread, dummy.OwningThread);
   bool ret = cs->OwningThread == dummy.OwningThread;
   LeaveCriticalSection(&dummy);
   DeleteCriticalSection(&dummy);
   return ret;
}

static void
stw_pipe_blit(struct pipe_context *pipe,
              struct pipe_resource *dst,
              struct pipe_resource *src)
{
   struct pipe_blit_info blit;

   if (!dst || !src)
      return;

   /* From the GL spec, version 4.2, section 4.1.11 (Additional Multisample
    *  Fragment Operations):
    *
    *      If a framebuffer object is not bound, after all operations have
    *      been completed on the multisample buffer, the sample values for
    *      each color in the multisample buffer are combined to produce a
    *      single color value, and that value is written into the
    *      corresponding color buffers selected by DrawBuffer or
    *      DrawBuffers. An implementation may defer the writing of the color
    *      buffers until a later time, but the state of the framebuffer must
    *      behave as if the color buffers were updated as each fragment was
    *      processed. The method of combination is not specified. If the
    *      framebuffer contains sRGB values, then it is recommended that the
    *      an average of sample values is computed in a linearized space, as
    *      for blending (see section 4.1.7).
    *
    * In other words, to do a resolve operation in a linear space, we have
    * to set sRGB formats if the original resources were sRGB, so don't use
    * util_format_linear.
    */

   memset(&blit, 0, sizeof(blit));
   blit.dst.resource = dst;
   blit.dst.box.width = dst->width0;
   blit.dst.box.height = dst->height0;
   blit.dst.box.depth = 1;
   blit.dst.format = dst->format;
   blit.src.resource = src;
   blit.src.box.width = src->width0;
   blit.src.box.height = src->height0;
   blit.src.box.depth = 1;
   blit.src.format = src->format;
   blit.mask = PIPE_MASK_RGBA;
   blit.filter = PIPE_TEX_FILTER_NEAREST;

   pipe->blit(pipe, &blit);
}

#ifdef GALLIUM_ZINK

static_assert(sizeof(struct kopper_vk_surface_create_storage) >= sizeof(VkWin32SurfaceCreateInfoKHR), "");

static void
stw_st_fill_private_loader_data(struct stw_st_framebuffer *stwfb, struct kopper_loader_info *out)
{
   VkWin32SurfaceCreateInfoKHR *win32 = (VkWin32SurfaceCreateInfoKHR *)&out->bos;
   win32->sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
   win32->pNext = NULL;
   win32->flags = 0;
   win32->hinstance = GetModuleHandle(NULL);
   win32->hwnd = stwfb->fb->hWnd;
   out->has_alpha = true;
}
#endif
/**
 * Remove outdated textures and create the requested ones.
 */
static void
stw_st_framebuffer_validate_locked(struct st_context *st,
                                   struct pipe_frontend_drawable *drawable,
                                   unsigned width, unsigned height,
                                   unsigned mask)
{
   struct stw_st_framebuffer *stwfb = stw_st_framebuffer(drawable);
   struct pipe_resource templ;
   unsigned i;

   memset(&templ, 0, sizeof(templ));
   templ.target = PIPE_TEXTURE_2D;
   templ.width0 = width;
   templ.height0 = height;
   templ.depth0 = 1;
   templ.array_size = 1;
   templ.last_level = 0;

   /* As of now, the only stw_winsys_framebuffer implementation is
    * d3d12_wgl_framebuffer and it doesn't support front buffer
    * drawing. A fake front texture is needed to handle that scenario.
    * For MSAA, we just need to make sure that the back buffer also
    * exists, so we can blt to it during flush_frontbuffer. */
   if (mask & ST_ATTACHMENT_FRONT_LEFT_MASK &&
       stwfb->fb->winsys_framebuffer) {
      if (stwfb->stvis.samples <= 1)
         stwfb->needs_fake_front = true;
      else
         mask |= ST_ATTACHMENT_BACK_LEFT_MASK;
   }

   /* remove outdated textures */
   if (stwfb->texture_width != width || stwfb->texture_height != height) {
      for (i = 0; i < ST_ATTACHMENT_COUNT; i++) {
         pipe_resource_reference(&stwfb->msaa_textures[i], NULL);
         pipe_resource_reference(&stwfb->textures[i], NULL);
      }
      pipe_resource_reference(&stwfb->back_texture, NULL);

      if (stwfb->fb->winsys_framebuffer) {
         templ.nr_samples = templ.nr_storage_samples = 1;
         templ.format = stwfb->stvis.color_format;
         stwfb->fb->winsys_framebuffer->resize(stwfb->fb->winsys_framebuffer, st->pipe, &templ);
      }
   }

   for (i = 0; i < ST_ATTACHMENT_COUNT; i++) {
      enum pipe_format format;
      unsigned bind;

      /* the texture already exists or not requested */
      if (stwfb->textures[i] || !(mask & (1 << i))) {
         /* remember the texture */
         if (stwfb->textures[i])
            mask |= (1 << i);
         continue;
      }

      switch (i) {
      case ST_ATTACHMENT_FRONT_LEFT:
      case ST_ATTACHMENT_BACK_LEFT:
         format = stwfb->stvis.color_format;
         bind = PIPE_BIND_DISPLAY_TARGET |
                PIPE_BIND_SAMPLER_VIEW |
                PIPE_BIND_RENDER_TARGET;

#ifdef GALLIUM_ZINK
         if (stw_dev->zink) {
            /* Covers the case where we have already created a drawable that
             * then got swapped and now we have to make a new back buffer.
             * For Zink, we just alias the front buffer in that case.
             */
            if (i == ST_ATTACHMENT_BACK_LEFT && stwfb->textures[ST_ATTACHMENT_FRONT_LEFT])
               bind &= ~PIPE_BIND_DISPLAY_TARGET;
         }
#endif

         break;
      case ST_ATTACHMENT_DEPTH_STENCIL:
         format = stwfb->stvis.depth_stencil_format;
         bind = PIPE_BIND_DEPTH_STENCIL;

#ifdef GALLIUM_ZINK
         if (stw_dev->zink)
            bind |= PIPE_BIND_DISPLAY_TARGET;
#endif

         break;
      default:
         format = PIPE_FORMAT_NONE;
         break;
      }

      if (format != PIPE_FORMAT_NONE) {
         templ.format = format;

         if (bind != PIPE_BIND_DEPTH_STENCIL && stwfb->stvis.samples > 1) {
            templ.bind = PIPE_BIND_SAMPLER_VIEW | PIPE_BIND_RENDER_TARGET;
            templ.nr_samples = templ.nr_storage_samples =
               stwfb->stvis.samples;

            stwfb->msaa_textures[i] =
               stw_dev->screen->resource_create(stw_dev->screen, &templ);
         }

         templ.bind = bind;
         templ.nr_samples = templ.nr_storage_samples = 1;

#ifdef GALLIUM_ZINK
         if (stw_dev->zink &&
             i < ST_ATTACHMENT_DEPTH_STENCIL &&
             stw_dev->screen->resource_create_drawable) {

            struct kopper_loader_info loader_info;
            void *data;

            if (bind & PIPE_BIND_DISPLAY_TARGET) {
               stw_st_fill_private_loader_data(stwfb, &loader_info);
               data = &loader_info;
            } else
               data = stwfb->textures[ST_ATTACHMENT_FRONT_LEFT];

            assert(data);
            stwfb->textures[i] =
               stw_dev->screen->resource_create_drawable(stw_dev->screen,
                                                             &templ,
                                                             data);
         } else {
#endif
            if (stwfb->fb->winsys_framebuffer)
               stwfb->textures[i] = stwfb->fb->winsys_framebuffer->get_resource(
                  stwfb->fb->winsys_framebuffer, i);
            else
               stwfb->textures[i] =
                  stw_dev->screen->resource_create(stw_dev->screen, &templ);
#ifdef GALLIUM_ZINK
         }
#endif
      }
   }

   /* When a fake front buffer is needed for drawing, we use the back buffer
    * as it reduces the number of blits needed:
    *
    *   - When flushing the front buffer, we only need to swap the real buffers
    *   - When swapping buffers, we can safely overwrite the fake front buffer
    *     as it is now invalid anyway.
    *
    * A new texture is created to store the back buffer content.
    */
   if (stwfb->needs_fake_front) {
      if (!stwfb->back_texture) {
         templ.format = stwfb->stvis.color_format;
         templ.bind = PIPE_BIND_SAMPLER_VIEW | PIPE_BIND_RENDER_TARGET;
         templ.nr_samples = templ.nr_storage_samples = 1;

         stwfb->back_texture =
            stw_dev->screen->resource_create(stw_dev->screen, &templ);

         /* TODO Only blit if there is something currently drawn on the back buffer */
         stw_pipe_blit(st->pipe,
                       stwfb->back_texture,
                       stwfb->textures[ST_ATTACHMENT_BACK_LEFT]);
      }

      /* Copying front texture content to fake front texture (back texture) */
      stw_pipe_blit(st->pipe,
                    stwfb->textures[ST_ATTACHMENT_BACK_LEFT],
                    stwfb->textures[ST_ATTACHMENT_FRONT_LEFT]);
   }

   stwfb->texture_width = width;
   stwfb->texture_height = height;
   stwfb->texture_mask = mask;
}

static bool
stw_st_framebuffer_validate(struct st_context *st,
                            struct pipe_frontend_drawable *drawable,
                            const enum st_attachment_type *statts,
                            unsigned count,
                            struct pipe_resource **out,
                            struct pipe_resource **resolve)
{
   struct stw_st_framebuffer *stwfb = stw_st_framebuffer(drawable);
   unsigned statt_mask, i;

   statt_mask = 0x0;
   for (i = 0; i < count; i++)
      statt_mask |= 1 << statts[i];

   stw_framebuffer_lock(stwfb->fb);

   if (stwfb->fb->must_resize || stwfb->needs_fake_front || (statt_mask & ~stwfb->texture_mask)) {
      stw_st_framebuffer_validate_locked(st, &stwfb->base,
            stwfb->fb->width, stwfb->fb->height, statt_mask);
      stwfb->fb->must_resize = false;
   }

   struct pipe_resource **textures =
      stwfb->stvis.samples > 1 ? stwfb->msaa_textures
                               : stwfb->textures;

   for (i = 0; i < count; i++) {
      struct pipe_resource *texture = NULL;

      if (stwfb->needs_fake_front) {
         if (statts[i] == ST_ATTACHMENT_FRONT_LEFT)
            texture = textures[ST_ATTACHMENT_BACK_LEFT]; /* Fake front buffer */
         else if (statts[i] == ST_ATTACHMENT_BACK_LEFT)
            texture = stwfb->back_texture;
      } else {
         texture = textures[statts[i]];
      }
      pipe_resource_reference(&out[i], texture);
   }

   if (resolve && stwfb->stvis.samples > 1) {
      if (statt_mask & BITFIELD_BIT(ST_ATTACHMENT_FRONT_LEFT))
         pipe_resource_reference(resolve, stwfb->textures[ST_ATTACHMENT_FRONT_LEFT]);
      else if (statt_mask & BITFIELD_BIT(ST_ATTACHMENT_BACK_LEFT))
         pipe_resource_reference(resolve, stwfb->textures[ST_ATTACHMENT_BACK_LEFT]);
   }

   stw_framebuffer_unlock(stwfb->fb);

   return true;
}

struct notify_before_flush_cb_args {
   struct st_context *st;
   struct stw_st_framebuffer *stwfb;
   unsigned flags;
};

static void
notify_before_flush_cb(void* _args)
{
   struct notify_before_flush_cb_args *args = (struct notify_before_flush_cb_args *) _args;
   struct st_context *st = args->st;
   struct pipe_context *pipe = st->pipe;

   if (args->stwfb->stvis.samples > 1) {
      /* Resolve the MSAA back buffer. */
      stw_pipe_blit(pipe,
                    args->stwfb->textures[ST_ATTACHMENT_BACK_LEFT],
                    args->stwfb->msaa_textures[ST_ATTACHMENT_BACK_LEFT]);

      /* FRONT_LEFT is resolved in flush_frontbuffer. */
   } else if (args->stwfb->back_texture) {
      /* Fake front texture is dirty ?? */
      stw_pipe_blit(pipe,
                    args->stwfb->textures[ST_ATTACHMENT_BACK_LEFT],
                    args->stwfb->back_texture);
   }

   if (args->stwfb->textures[ST_ATTACHMENT_BACK_LEFT])
      pipe->flush_resource(pipe, args->stwfb->textures[ST_ATTACHMENT_BACK_LEFT]);
}

void
stw_st_flush(struct st_context *st,
             struct pipe_frontend_drawable *drawable,
             unsigned flags)
{
   struct stw_st_framebuffer *stwfb = stw_st_framebuffer(drawable);
   struct notify_before_flush_cb_args args;
   struct pipe_fence_handle **pfence = NULL;
   struct pipe_fence_handle *fence = NULL;

   args.st = st;
   args.stwfb = stwfb;
   args.flags = flags;

   if (flags & ST_FLUSH_END_OF_FRAME && !stwfb->fb->winsys_framebuffer)
      flags |= ST_FLUSH_WAIT;

   if (flags & ST_FLUSH_WAIT)
      pfence = &fence;
   st_context_flush(st, flags, pfence, notify_before_flush_cb, &args);

   /* TODO: remove this if the framebuffer state doesn't change. */
   st_context_invalidate_state(st, ST_INVALIDATE_FB_STATE);
}

/**
 * Present an attachment of the framebuffer.
 */
static bool
stw_st_framebuffer_present_locked(HDC hdc,
                                  struct st_context *st,
                                  struct pipe_frontend_drawable *drawable,
                                  enum st_attachment_type statt)
{
   struct stw_st_framebuffer *stwfb = stw_st_framebuffer(drawable);
   struct pipe_resource *resource;

   assert(stw_own_mutex(&stwfb->fb->mutex));

   resource = stwfb->textures[statt];
   if (resource) {
      stw_framebuffer_present_locked(hdc, stwfb->fb, resource);
   }
   else {
      stw_framebuffer_unlock(stwfb->fb);
   }

   assert(!stw_own_mutex(&stwfb->fb->mutex));

   return true;
}

static bool
stw_st_framebuffer_flush_front(struct st_context *st,
                               struct pipe_frontend_drawable *drawable,
                               enum st_attachment_type statt)
{
   struct stw_st_framebuffer *stwfb = stw_st_framebuffer(drawable);
   struct pipe_context *pipe = st->pipe;
   bool ret;
   HDC hDC;
   bool need_swap_textures = false;

   if (statt != ST_ATTACHMENT_FRONT_LEFT)
      return false;

   stw_framebuffer_lock(stwfb->fb);

   /* Resolve the front buffer. */
   if (stwfb->stvis.samples > 1) {
      enum st_attachment_type blit_target = statt;
      if (stwfb->fb->winsys_framebuffer) {
         blit_target = ST_ATTACHMENT_BACK_LEFT;
         need_swap_textures = true;
      }

      stw_pipe_blit(pipe, stwfb->textures[blit_target],
                    stwfb->msaa_textures[statt]);
   } else if (stwfb->needs_fake_front) {
      /* fake front texture is now invalid */
      p_atomic_inc(&stwfb->base.stamp);
      need_swap_textures = true;
   }

   if (need_swap_textures) {
      struct pipe_resource *ptex = stwfb->textures[ST_ATTACHMENT_FRONT_LEFT];
      stwfb->textures[ST_ATTACHMENT_FRONT_LEFT] = stwfb->textures[ST_ATTACHMENT_BACK_LEFT];
      stwfb->textures[ST_ATTACHMENT_BACK_LEFT] = ptex;
   }

   if (stwfb->textures[statt])
      pipe->flush_resource(pipe, stwfb->textures[statt]);

   pipe->flush(pipe, NULL, 0);

   /* We must not cache HDCs anywhere, as they can be invalidated by the
    * application, or screen resolution changes. */

   hDC = GetDC(stwfb->fb->hWnd);

   ret = stw_st_framebuffer_present_locked(hDC, st, &stwfb->base, statt);

   ReleaseDC(stwfb->fb->hWnd, hDC);

   return ret;
}

/**
 * Create a framebuffer interface.
 */
struct pipe_frontend_drawable *
stw_st_create_framebuffer(struct stw_framebuffer *fb, struct pipe_frontend_screen *fscreen)
{
   struct stw_st_framebuffer *stwfb;

   stwfb = CALLOC_STRUCT(stw_st_framebuffer);
   if (!stwfb)
      return NULL;

   stwfb->fb = fb;
   stwfb->stvis = fb->pfi->stvis;
   stwfb->base.ID = p_atomic_inc_return(&stwfb_ID);
   stwfb->base.fscreen = fscreen;

   stwfb->base.visual = &stwfb->stvis;
   p_atomic_set(&stwfb->base.stamp, 1);
   stwfb->base.flush_front = stw_st_framebuffer_flush_front;
   stwfb->base.validate = stw_st_framebuffer_validate;

   return &stwfb->base;
}

/**
 * Destroy a framebuffer interface.
 */
void
stw_st_destroy_framebuffer_locked(struct pipe_frontend_drawable *drawable)
{
   struct stw_st_framebuffer *stwfb = stw_st_framebuffer(drawable);
   int i;

   for (i = 0; i < ST_ATTACHMENT_COUNT; i++) {
      pipe_resource_reference(&stwfb->msaa_textures[i], NULL);
      pipe_resource_reference(&stwfb->textures[i], NULL);
   }
   pipe_resource_reference(&stwfb->back_texture, NULL);

   /* Notify the st manager that the framebuffer interface is no
    * longer valid.
    */
   st_api_destroy_drawable(&stwfb->base);

   FREE(stwfb);
}

/**
 * Swap the buffers of the given framebuffer.
 */
bool
stw_st_swap_framebuffer_locked(HDC hdc, struct st_context *st,
                               struct pipe_frontend_drawable *drawable)
{
   struct stw_st_framebuffer *stwfb = stw_st_framebuffer(drawable);
   unsigned front = ST_ATTACHMENT_FRONT_LEFT, back = ST_ATTACHMENT_BACK_LEFT;
   struct pipe_resource *ptex;
   unsigned mask;

   /* swap the textures */
   ptex = stwfb->textures[front];
   stwfb->textures[front] = stwfb->textures[back];
   stwfb->textures[back] = ptex;

   /* swap msaa_textures */
   ptex = stwfb->msaa_textures[front];
   stwfb->msaa_textures[front] = stwfb->msaa_textures[back];
   stwfb->msaa_textures[back] = ptex;


   /* Fake front texture is now dirty */
   if (stwfb->needs_fake_front)
      p_atomic_inc(&stwfb->base.stamp);

   /* convert to mask */
   front = 1 << front;
   back = 1 << back;

   /* swap the bits in mask */
   mask = stwfb->texture_mask & ~(front | back);
   if (stwfb->texture_mask & front)
      mask |= back;
   if (stwfb->texture_mask & back)
      mask |= front;
   stwfb->texture_mask = mask;

   front = ST_ATTACHMENT_FRONT_LEFT;
   return stw_st_framebuffer_present_locked(hdc, st, &stwfb->base, front);
}


/**
 * Return the pipe_resource that correspond to given buffer.
 */
struct pipe_resource *
stw_get_framebuffer_resource(struct pipe_frontend_drawable *drawable,
                             enum st_attachment_type att)
{
   struct stw_st_framebuffer *stwfb = stw_st_framebuffer(drawable);
   return stwfb->textures[att];
}
