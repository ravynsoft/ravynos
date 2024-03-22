/**************************************************************************
 *
 * Copyright 2009, VMware, Inc.
 * All Rights Reserved.
 * Copyright 2010 George Sapountzis <gsapountzis@gmail.com>
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

#include "GL/internal/mesa_interface.h"
#include "git_sha1.h"
#include "util/format/u_format.h"
#include "util/u_memory.h"
#include "util/u_inlines.h"
#include "util/u_box.h"
#include "pipe/p_context.h"
#include "pipe-loader/pipe_loader.h"
#include "frontend/drisw_api.h"
#include "state_tracker/st_context.h"

#include "dri_screen.h"
#include "dri_context.h"
#include "dri_drawable.h"
#include "dri_helpers.h"
#include "dri_query_renderer.h"

DEBUG_GET_ONCE_BOOL_OPTION(swrast_no_present, "SWRAST_NO_PRESENT", false);

static inline void
get_drawable_info(struct dri_drawable *drawable, int *x, int *y, int *w, int *h)
{
   const __DRIswrastLoaderExtension *loader = drawable->screen->swrast_loader;

   loader->getDrawableInfo(opaque_dri_drawable(drawable),
                           x, y, w, h,
                           drawable->loaderPrivate);
}

static inline void
put_image(struct dri_drawable *drawable, void *data, unsigned width, unsigned height)
{
   const __DRIswrastLoaderExtension *loader = drawable->screen->swrast_loader;

   loader->putImage(opaque_dri_drawable(drawable), __DRI_SWRAST_IMAGE_OP_SWAP,
                    0, 0, width, height,
                    data, drawable->loaderPrivate);
}

static inline void
put_image2(struct dri_drawable *drawable, void *data, int x, int y,
           unsigned width, unsigned height, unsigned stride)
{
   const __DRIswrastLoaderExtension *loader = drawable->screen->swrast_loader;

   loader->putImage2(opaque_dri_drawable(drawable), __DRI_SWRAST_IMAGE_OP_SWAP,
                     x, y, width, height, stride,
                     data, drawable->loaderPrivate);
}

static inline void
put_image_shm(struct dri_drawable *drawable, int shmid, char *shmaddr,
              unsigned offset, unsigned offset_x, int x, int y,
              unsigned width, unsigned height, unsigned stride)
{
   const __DRIswrastLoaderExtension *loader = drawable->screen->swrast_loader;

   /* if we have the newer interface, don't have to add the offset_x here. */
   if (loader->base.version > 4 && loader->putImageShm2)
     loader->putImageShm2(opaque_dri_drawable(drawable), __DRI_SWRAST_IMAGE_OP_SWAP,
                          x, y, width, height, stride,
                          shmid, shmaddr, offset, drawable->loaderPrivate);
   else
     loader->putImageShm(opaque_dri_drawable(drawable), __DRI_SWRAST_IMAGE_OP_SWAP,
                         x, y, width, height, stride,
                         shmid, shmaddr, offset + offset_x, drawable->loaderPrivate);
}

static inline void
get_image(struct dri_drawable *drawable, int x, int y, int width, int height, void *data)
{
   const __DRIswrastLoaderExtension *loader = drawable->screen->swrast_loader;

   loader->getImage(opaque_dri_drawable(drawable),
                    x, y, width, height,
                    data, drawable->loaderPrivate);
}

static inline void
get_image2(struct dri_drawable *drawable, int x, int y, int width, int height, int stride, void *data)
{
   const __DRIswrastLoaderExtension *loader = drawable->screen->swrast_loader;

   /* getImage2 support is only in version 3 or newer */
   if (loader->base.version < 3)
      return;

   loader->getImage2(opaque_dri_drawable(drawable),
                     x, y, width, height, stride,
                     data, drawable->loaderPrivate);
}

static inline bool
get_image_shm(struct dri_drawable *drawable, int x, int y, int width, int height,
              struct pipe_resource *res)
{
   const __DRIswrastLoaderExtension *loader = drawable->screen->swrast_loader;
   struct winsys_handle whandle;

   whandle.type = WINSYS_HANDLE_TYPE_SHMID;

   if (loader->base.version < 4 || !loader->getImageShm)
      return false;

   if (!res->screen->resource_get_handle(res->screen, NULL, res, &whandle, PIPE_HANDLE_USAGE_FRAMEBUFFER_WRITE))
      return false;

   if (loader->base.version > 5 && loader->getImageShm2)
      return loader->getImageShm2(opaque_dri_drawable(drawable), x, y, width, height, whandle.handle, drawable->loaderPrivate);

   loader->getImageShm(opaque_dri_drawable(drawable), x, y, width, height, whandle.handle, drawable->loaderPrivate);
   return true;
}

static void
drisw_update_drawable_info(struct dri_drawable *drawable)
{
   int x, y;

   get_drawable_info(drawable, &x, &y, &drawable->w, &drawable->h);
}

static void
drisw_get_image(struct dri_drawable *drawable,
                int x, int y, unsigned width, unsigned height, unsigned stride,
                void *data)
{
   int draw_x, draw_y, draw_w, draw_h;

   get_drawable_info(drawable, &draw_x, &draw_y, &draw_w, &draw_h);
   get_image2(drawable, x, y, draw_w, draw_h, stride, data);
}

static void
drisw_put_image(struct dri_drawable *drawable,
                void *data, unsigned width, unsigned height)
{
   put_image(drawable, data, width, height);
}

static void
drisw_put_image2(struct dri_drawable *drawable,
                 void *data, int x, int y, unsigned width, unsigned height,
                 unsigned stride)
{
   put_image2(drawable, data, x, y, width, height, stride);
}

static inline void
drisw_put_image_shm(struct dri_drawable *drawable,
                    int shmid, char *shmaddr, unsigned offset,
                    unsigned offset_x,
                    int x, int y, unsigned width, unsigned height,
                    unsigned stride)
{
   put_image_shm(drawable, shmid, shmaddr, offset, offset_x, x, y, width, height, stride);
}

static inline void
drisw_present_texture(struct pipe_context *pipe, struct dri_drawable *drawable,
                      struct pipe_resource *ptex, struct pipe_box *sub_box)
{
   struct dri_screen *screen = drawable->screen;

   if (screen->swrast_no_present)
      return;

   screen->base.screen->flush_frontbuffer(screen->base.screen, pipe, ptex, 0, 0, drawable, sub_box);
}

static inline void
drisw_invalidate_drawable(struct dri_drawable *drawable)
{
   drawable->texture_stamp = drawable->lastStamp - 1;

   p_atomic_inc(&drawable->base.stamp);
}

static inline void
drisw_copy_to_front(struct pipe_context *pipe,
                    struct dri_drawable *drawable,
                    struct pipe_resource *ptex)
{
   drisw_present_texture(pipe, drawable, ptex, NULL);

   drisw_invalidate_drawable(drawable);
}

/*
 * Backend functions for pipe_frontend_drawable and swap_buffers.
 */

static void
drisw_swap_buffers(struct dri_drawable *drawable)
{
   struct dri_context *ctx = dri_get_current();
   struct dri_screen *screen = drawable->screen;
   struct pipe_resource *ptex;

   if (!ctx)
      return;

   /* Wait for glthread to finish because we can't use pipe_context from
    * multiple threads.
    */
   _mesa_glthread_finish(ctx->st->ctx);

   ptex = drawable->textures[ST_ATTACHMENT_BACK_LEFT];

   if (ptex) {
      struct pipe_fence_handle *fence = NULL;
      if (ctx->pp)
         pp_run(ctx->pp, ptex, ptex, drawable->textures[ST_ATTACHMENT_DEPTH_STENCIL]);

      if (ctx->hud)
         hud_run(ctx->hud, ctx->st->cso_context, ptex);

      st_context_flush(ctx->st, ST_FLUSH_FRONT, &fence, NULL, NULL);

      if (drawable->stvis.samples > 1) {
         /* Resolve the back buffer. */
         dri_pipe_blit(ctx->st->pipe,
                       drawable->textures[ST_ATTACHMENT_BACK_LEFT],
                       drawable->msaa_textures[ST_ATTACHMENT_BACK_LEFT]);
      }

      screen->base.screen->fence_finish(screen->base.screen, ctx->st->pipe,
                                        fence, OS_TIMEOUT_INFINITE);
      screen->base.screen->fence_reference(screen->base.screen, &fence, NULL);
      drisw_copy_to_front(ctx->st->pipe, drawable, ptex);

      /* TODO: remove this if the framebuffer state doesn't change. */
      st_context_invalidate_state(ctx->st, ST_INVALIDATE_FB_STATE);
   }
}

static void
drisw_copy_sub_buffer(struct dri_drawable *drawable, int x, int y,
                      int w, int h)
{
   struct dri_context *ctx = dri_get_current();
   struct dri_screen *screen = drawable->screen;
   struct pipe_resource *ptex;
   struct pipe_box box;
   if (!ctx)
      return;

   ptex = drawable->textures[ST_ATTACHMENT_BACK_LEFT];

   if (ptex) {
      /* Wait for glthread to finish because we can't use pipe_context from
       * multiple threads.
       */
      _mesa_glthread_finish(ctx->st->ctx);

      struct pipe_fence_handle *fence = NULL;
      if (ctx->pp && drawable->textures[ST_ATTACHMENT_DEPTH_STENCIL])
         pp_run(ctx->pp, ptex, ptex, drawable->textures[ST_ATTACHMENT_DEPTH_STENCIL]);

      st_context_flush(ctx->st, ST_FLUSH_FRONT, &fence, NULL, NULL);

      screen->base.screen->fence_finish(screen->base.screen, ctx->st->pipe,
                                        fence, OS_TIMEOUT_INFINITE);
      screen->base.screen->fence_reference(screen->base.screen, &fence, NULL);

      if (drawable->stvis.samples > 1) {
         /* Resolve the back buffer. */
         dri_pipe_blit(ctx->st->pipe,
                       drawable->textures[ST_ATTACHMENT_BACK_LEFT],
                       drawable->msaa_textures[ST_ATTACHMENT_BACK_LEFT]);
      }

      u_box_2d(x, drawable->h - y - h, w, h, &box);
      drisw_present_texture(ctx->st->pipe, drawable, ptex, &box);
   }
}

static bool
drisw_flush_frontbuffer(struct dri_context *ctx,
                        struct dri_drawable *drawable,
                        enum st_attachment_type statt)
{
   struct pipe_resource *ptex;

   if (!ctx || statt != ST_ATTACHMENT_FRONT_LEFT)
      return false;

   /* Wait for glthread to finish because we can't use pipe_context from
    * multiple threads.
    */
   _mesa_glthread_finish(ctx->st->ctx);

   if (drawable->stvis.samples > 1) {
      /* Resolve the front buffer. */
      dri_pipe_blit(ctx->st->pipe,
                    drawable->textures[ST_ATTACHMENT_FRONT_LEFT],
                    drawable->msaa_textures[ST_ATTACHMENT_FRONT_LEFT]);
   }
   ptex = drawable->textures[statt];

   if (ptex) {
      drisw_copy_to_front(ctx->st->pipe, ctx->draw, ptex);
   }

   return true;
}

/**
 * Allocate framebuffer attachments.
 *
 * During fixed-size operation, the function keeps allocating new attachments
 * as they are requested. Unused attachments are not removed, not until the
 * framebuffer is resized or destroyed.
 */
static void
drisw_allocate_textures(struct dri_context *stctx,
                        struct dri_drawable *drawable,
                        const enum st_attachment_type *statts,
                        unsigned count)
{
   struct dri_screen *screen = drawable->screen;
   const __DRIswrastLoaderExtension *loader = drawable->screen->swrast_loader;
   struct pipe_resource templ;
   unsigned width, height;
   bool resized;
   unsigned i;

   /* Wait for glthread to finish because we can't use pipe_context from
    * multiple threads.
    */
   _mesa_glthread_finish(stctx->st->ctx);

   width  = drawable->w;
   height = drawable->h;

   resized = (drawable->old_w != width ||
              drawable->old_h != height);

   /* remove outdated textures */
   if (resized) {
      for (i = 0; i < ST_ATTACHMENT_COUNT; i++) {
         pipe_resource_reference(&drawable->textures[i], NULL);
         pipe_resource_reference(&drawable->msaa_textures[i], NULL);
      }
   }

   memset(&templ, 0, sizeof(templ));
   templ.target = screen->target;
   templ.width0 = width;
   templ.height0 = height;
   templ.depth0 = 1;
   templ.array_size = 1;
   templ.last_level = 0;

   for (i = 0; i < count; i++) {
      enum pipe_format format;
      unsigned bind;

      /* the texture already exists or not requested */
      if (drawable->textures[statts[i]])
         continue;

      dri_drawable_get_format(drawable, statts[i], &format, &bind);

      /* if we don't do any present, no need for display targets */
      if (statts[i] != ST_ATTACHMENT_DEPTH_STENCIL && !screen->swrast_no_present)
         bind |= PIPE_BIND_DISPLAY_TARGET;

      if (format == PIPE_FORMAT_NONE)
         continue;

      templ.format = format;
      templ.bind = bind;
      templ.nr_samples = 0;
      templ.nr_storage_samples = 0;

      if (statts[i] == ST_ATTACHMENT_FRONT_LEFT &&
                 screen->base.screen->resource_create_front &&
                 loader->base.version >= 3) {
         drawable->textures[statts[i]] =
            screen->base.screen->resource_create_front(screen->base.screen, &templ, (const void *)drawable);
      } else
         drawable->textures[statts[i]] =
            screen->base.screen->resource_create(screen->base.screen, &templ);

      if (drawable->stvis.samples > 1) {
         templ.bind = templ.bind &
            ~(PIPE_BIND_SCANOUT | PIPE_BIND_SHARED | PIPE_BIND_DISPLAY_TARGET);
         templ.nr_samples = drawable->stvis.samples;
         templ.nr_storage_samples = drawable->stvis.samples;
         drawable->msaa_textures[statts[i]] =
            screen->base.screen->resource_create(screen->base.screen, &templ);

         dri_pipe_blit(stctx->st->pipe,
                       drawable->msaa_textures[statts[i]],
                       drawable->textures[statts[i]]);
      }
   }

   drawable->old_w = width;
   drawable->old_h = height;
}

static void
drisw_update_tex_buffer(struct dri_drawable *drawable,
                        struct dri_context *ctx,
                        struct pipe_resource *res)
{
   struct st_context *st_ctx = (struct st_context *)ctx->st;
   struct pipe_context *pipe = st_ctx->pipe;
   struct pipe_transfer *transfer;
   char *map;
   int x, y, w, h;
   int ximage_stride, line;
   int cpp = util_format_get_blocksize(res->format);

   /* Wait for glthread to finish because we can't use pipe_context from
    * multiple threads.
    */
   _mesa_glthread_finish(ctx->st->ctx);

   get_drawable_info(drawable, &x, &y, &w, &h);

   map = pipe_texture_map(pipe, res,
                           0, 0, // level, layer,
                           PIPE_MAP_WRITE,
                           x, y, w, h, &transfer);

   /* Copy the Drawable content to the mapped texture buffer */
   if (!get_image_shm(drawable, x, y, w, h, res))
      get_image(drawable, x, y, w, h, map);

   /* The pipe transfer has a pitch rounded up to the nearest 64 pixels.
      get_image() has a pitch rounded up to 4 bytes.  */
   ximage_stride = ((w * cpp) + 3) & -4;
   for (line = h-1; line; --line) {
      memmove(&map[line * transfer->stride],
              &map[line * ximage_stride],
              ximage_stride);
   }

   pipe_texture_unmap(pipe, transfer);
}

static __DRIimageExtension driSWImageExtension = {
    .base = { __DRI_IMAGE, 6 },

    .createImageFromRenderbuffer  = dri2_create_image_from_renderbuffer,
    .createImageFromTexture = dri2_create_from_texture,
    .destroyImage = dri2_destroy_image,
};

static const __DRIrobustnessExtension dri2Robustness = {
   .base = { __DRI2_ROBUSTNESS, 1 }
};

/*
 * Backend function for init_screen.
 */

static const __DRIextension *drisw_screen_extensions[] = {
   &driTexBufferExtension.base,
   &dri2RendererQueryExtension.base,
   &dri2ConfigQueryExtension.base,
   &dri2FenceExtension.base,
   &driSWImageExtension.base,
   &dri2FlushControlExtension.base,
   NULL
};

static const __DRIextension *drisw_robust_screen_extensions[] = {
   &driTexBufferExtension.base,
   &dri2RendererQueryExtension.base,
   &dri2ConfigQueryExtension.base,
   &dri2FenceExtension.base,
   &dri2Robustness.base,
   &driSWImageExtension.base,
   &dri2FlushControlExtension.base,
   NULL
};

static const struct drisw_loader_funcs drisw_lf = {
   .get_image = drisw_get_image,
   .put_image = drisw_put_image,
   .put_image2 = drisw_put_image2
};

static const struct drisw_loader_funcs drisw_shm_lf = {
   .get_image = drisw_get_image,
   .put_image = drisw_put_image,
   .put_image2 = drisw_put_image2,
   .put_image_shm = drisw_put_image_shm
};

static struct dri_drawable *
drisw_create_drawable(struct dri_screen *screen, const struct gl_config * visual,
                      bool isPixmap, void *loaderPrivate)
{
   struct dri_drawable *drawable = dri_create_drawable(screen, visual, isPixmap,
                                                       loaderPrivate);
   if (!drawable)
      return NULL;

   drawable->allocate_textures = drisw_allocate_textures;
   drawable->update_drawable_info = drisw_update_drawable_info;
   drawable->flush_frontbuffer = drisw_flush_frontbuffer;
   drawable->update_tex_buffer = drisw_update_tex_buffer;
   drawable->swap_buffers = drisw_swap_buffers;

   return drawable;
}

static const __DRIconfig **
drisw_init_screen(struct dri_screen *screen)
{
   const __DRIswrastLoaderExtension *loader = screen->swrast_loader;
   const __DRIconfig **configs;
   struct pipe_screen *pscreen = NULL;
   const struct drisw_loader_funcs *lf = &drisw_lf;

   screen->swrast_no_present = debug_get_option_swrast_no_present();

   if (loader->base.version >= 4) {
      if (loader->putImageShm)
         lf = &drisw_shm_lf;
   }

   bool success = false;
#ifdef HAVE_DRISW_KMS
   if (screen->fd != -1)
      success = pipe_loader_sw_probe_kms(&screen->dev, screen->fd);
#endif
   if (!success)
      success = pipe_loader_sw_probe_dri(&screen->dev, lf);

   if (success)
      pscreen = pipe_loader_create_screen(screen->dev);

   if (!pscreen)
      goto fail;

   dri_init_options(screen);
   configs = dri_init_screen(screen, pscreen);
   if (!configs)
      goto fail;

   if (pscreen->get_param(pscreen, PIPE_CAP_DEVICE_RESET_STATUS_QUERY)) {
      screen->extensions = drisw_robust_screen_extensions;
      screen->has_reset_status_query = true;
   }
   else
      screen->extensions = drisw_screen_extensions;
   screen->lookup_egl_image = dri2_lookup_egl_image;

   const __DRIimageLookupExtension *image = screen->dri2.image;
   if (image &&
       image->base.version >= 2 &&
       image->validateEGLImage &&
       image->lookupEGLImageValidated) {
      screen->validate_egl_image = dri2_validate_egl_image;
      screen->lookup_egl_image_validated = dri2_lookup_egl_image_validated;
   }

   screen->create_drawable = drisw_create_drawable;

   return configs;
fail:
   dri_release_screen(screen);
   return NULL;
}

/* swrast copy sub buffer entrypoint. */
static void driswCopySubBuffer(__DRIdrawable *pdp, int x, int y,
                               int w, int h)
{
   struct dri_drawable *drawable = dri_drawable(pdp);

   assert(drawable->screen->swrast_loader);

   drisw_copy_sub_buffer(drawable, x, y, w, h);
}

/* for swrast only */
const __DRIcopySubBufferExtension driSWCopySubBufferExtension = {
   .base = { __DRI_COPY_SUB_BUFFER, 1 },

   .copySubBuffer               = driswCopySubBuffer,
};

static const struct __DRImesaCoreExtensionRec mesaCoreExtension = {
   .base = { __DRI_MESA, 1 },
   .version_string = MESA_INTERFACE_VERSION_STRING,
   .createNewScreen = driCreateNewScreen2,
   .createContext = driCreateContextAttribs,
   .initScreen = drisw_init_screen,
};

/* This is the table of extensions that the loader will dlsym() for. */
const __DRIextension *galliumsw_driver_extensions[] = {
    &driCoreExtension.base,
    &mesaCoreExtension.base,
    &driSWRastExtension.base,
    &driSWCopySubBufferExtension.base,
    &gallium_config_options.base,
    NULL
};

/* vim: set sw=3 ts=8 sts=3 expandtab: */
