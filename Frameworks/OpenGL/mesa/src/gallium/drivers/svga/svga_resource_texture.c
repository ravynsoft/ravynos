/**********************************************************
 * Copyright 2008-2023 VMware, Inc.  All rights reserved.
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

#include "svga3d_reg.h"
#include "svga3d_surfacedefs.h"

#include "pipe/p_state.h"
#include "pipe/p_defines.h"
#include "util/u_thread.h"
#include "util/format/u_format.h"
#include "util/u_inlines.h"
#include "util/u_math.h"
#include "util/u_memory.h"
#include "util/u_resource.h"
#include "util/u_upload_mgr.h"

#include "svga_cmd.h"
#include "svga_format.h"
#include "svga_screen.h"
#include "svga_context.h"
#include "svga_resource_texture.h"
#include "svga_resource_buffer.h"
#include "svga_sampler_view.h"
#include "svga_surface.h"
#include "svga_winsys.h"
#include "svga_debug.h"


static void
svga_transfer_dma_band(struct svga_context *svga,
                       struct svga_transfer *st,
                       SVGA3dTransferType transfer,
                       unsigned x, unsigned y, unsigned z,
                       unsigned w, unsigned h, unsigned d,
                       unsigned srcx, unsigned srcy, unsigned srcz,
                       SVGA3dSurfaceDMAFlags flags)
{
   struct svga_texture *texture = svga_texture(st->base.resource);
   SVGA3dCopyBox box;

   assert(!st->use_direct_map);

   box.x = x;
   box.y = y;
   box.z = z;
   box.w = w;
   box.h = h;
   box.d = d;
   box.srcx = srcx;
   box.srcy = srcy;
   box.srcz = srcz;

   SVGA_DBG(DEBUG_DMA, "dma %s sid %p, face %u, (%u, %u, %u) - "
            "(%u, %u, %u), %ubpp\n",
            transfer == SVGA3D_WRITE_HOST_VRAM ? "to" : "from",
            texture->handle,
            st->slice,
            x,
            y,
            z,
            x + w,
            y + h,
            z + 1,
            util_format_get_blocksize(texture->b.format) * 8 /
            (util_format_get_blockwidth(texture->b.format)
             * util_format_get_blockheight(texture->b.format)));

   SVGA_RETRY(svga, SVGA3D_SurfaceDMA(svga->swc, st, transfer, &box, 1, flags));
}


static void
svga_transfer_dma(struct svga_context *svga,
                  struct svga_transfer *st,
                  SVGA3dTransferType transfer,
                  SVGA3dSurfaceDMAFlags flags)
{
   struct svga_texture *texture = svga_texture(st->base.resource);
   struct svga_screen *screen = svga_screen(texture->b.screen);
   struct svga_winsys_screen *sws = screen->sws;
   struct pipe_fence_handle *fence = NULL;

   assert(!st->use_direct_map);

   if (transfer == SVGA3D_READ_HOST_VRAM) {
      SVGA_DBG(DEBUG_PERF, "%s: readback transfer\n", __func__);
   }

   /* Ensure any pending operations on host surfaces are queued on the command
    * buffer first.
    */
   svga_surfaces_flush(svga);

   if (!st->swbuf) {
      /* Do the DMA transfer in a single go */
      svga_transfer_dma_band(svga, st, transfer,
                             st->box.x, st->box.y, st->box.z,
                             st->box.w, st->box.h, st->box.d,
                             0, 0, 0,
                             flags);

      if (transfer == SVGA3D_READ_HOST_VRAM) {
         svga_context_flush(svga, &fence);
         sws->fence_finish(sws, fence, OS_TIMEOUT_INFINITE, 0);
         sws->fence_reference(sws, &fence, NULL);
      }
   }
   else {
      int y, h, srcy;
      unsigned blockheight =
         util_format_get_blockheight(st->base.resource->format);

      h = st->hw_nblocksy * blockheight;
      srcy = 0;

      for (y = 0; y < st->box.h; y += h) {
         unsigned offset, length;
         void *hw, *sw;

         if (y + h > st->box.h)
            h = st->box.h - y;

         /* Transfer band must be aligned to pixel block boundaries */
         assert(y % blockheight == 0);
         assert(h % blockheight == 0);

         offset = y * st->base.stride / blockheight;
         length = h * st->base.stride / blockheight;

         sw = (uint8_t *) st->swbuf + offset;

         if (transfer == SVGA3D_WRITE_HOST_VRAM) {
            unsigned usage = PIPE_MAP_WRITE;

            /* Wait for the previous DMAs to complete */
            /* TODO: keep one DMA (at half the size) in the background */
            if (y) {
               svga_context_flush(svga, NULL);
               usage |= PIPE_MAP_DISCARD_WHOLE_RESOURCE;
            }

            hw = sws->buffer_map(sws, st->hwbuf, usage);
            assert(hw);
            if (hw) {
               memcpy(hw, sw, length);
               sws->buffer_unmap(sws, st->hwbuf);
            }
         }

         svga_transfer_dma_band(svga, st, transfer,
                                st->box.x, y, st->box.z,
                                st->box.w, h, st->box.d,
                                0, srcy, 0, flags);

         /*
          * Prevent the texture contents to be discarded on the next band
          * upload.
          */
         flags.discard = false;

         if (transfer == SVGA3D_READ_HOST_VRAM) {
            svga_context_flush(svga, &fence);
            sws->fence_finish(sws, fence, OS_TIMEOUT_INFINITE, 0);

            hw = sws->buffer_map(sws, st->hwbuf, PIPE_MAP_READ);
            assert(hw);
            if (hw) {
               memcpy(sw, hw, length);
               sws->buffer_unmap(sws, st->hwbuf);
            }
         }
      }
   }
}



bool
svga_resource_get_handle(struct pipe_screen *screen,
                         struct pipe_context *context,
                         struct pipe_resource *texture,
                         struct winsys_handle *whandle,
                         unsigned usage)
{
   struct svga_winsys_screen *sws = svga_winsys_screen(texture->screen);
   unsigned stride;

   if (texture->target == PIPE_BUFFER)
      return false;

   SVGA_DBG(DEBUG_DMA, "%s: texture=%p cachable=%d\n", __FUNCTION__,
            texture, svga_texture(texture)->key.cachable);

   svga_texture(texture)->key.cachable = 0;

   stride = util_format_get_nblocksx(texture->format, texture->width0) *
            util_format_get_blocksize(texture->format);

   return sws->surface_get_handle(sws, svga_texture(texture)->handle,
                                  stride, whandle);
}


/**
 * Determine if we need to read back a texture image before mapping it.
 */
static inline bool
need_tex_readback(struct svga_transfer *st)
{
   if (st->base.usage & PIPE_MAP_READ)
      return true;

   if ((st->base.usage & PIPE_MAP_WRITE) &&
       ((st->base.usage & PIPE_MAP_DISCARD_WHOLE_RESOURCE) == 0)) {
      return svga_was_texture_rendered_to(svga_texture(st->base.resource));
   }

   return false;
}


static void
readback_texture_surface(struct svga_context *svga,
                         struct svga_texture *tex,
                         struct svga_winsys_surface *surf)
{
   SVGA_RETRY(svga, SVGA3D_ReadbackGBSurface(svga->swc, surf));

   /* Mark the texture surface as UPDATED */
   tex->surface_state = SVGA_SURFACE_STATE_UPDATED;

   svga->hud.num_readbacks++;
   SVGA_STATS_COUNT_INC(svga_sws(svga), SVGA_STATS_COUNT_TEXREADBACK);
}

/**
 * Use DMA for the transfer request
 */
static void *
svga_texture_transfer_map_dma(struct svga_context *svga,
                              struct svga_transfer *st)
{
   struct svga_winsys_screen *sws = svga_screen(svga->pipe.screen)->sws;
   struct pipe_resource *texture = st->base.resource;
   unsigned nblocksx, nblocksy;
   unsigned d;
   unsigned usage = st->base.usage;

   /* we'll put the data into a tightly packed buffer */
   nblocksx = util_format_get_nblocksx(texture->format, st->box.w);
   nblocksy = util_format_get_nblocksy(texture->format, st->box.h);
   d = st->box.d;

   st->base.stride = nblocksx*util_format_get_blocksize(texture->format);
   st->base.layer_stride = st->base.stride * nblocksy;
   st->hw_nblocksy = nblocksy;

   st->hwbuf = svga_winsys_buffer_create(svga, 1, 0,
                                         st->hw_nblocksy * st->base.stride * d);

   while (!st->hwbuf && (st->hw_nblocksy /= 2)) {
      st->hwbuf =
         svga_winsys_buffer_create(svga, 1, 0,
                                   st->hw_nblocksy * st->base.stride * d);
   }

   if (!st->hwbuf)
      return NULL;

   if (st->hw_nblocksy < nblocksy) {
      /* We couldn't allocate a hardware buffer big enough for the transfer,
       * so allocate regular malloc memory instead
       */
      if (0) {
         debug_printf("%s: failed to allocate %u KB of DMA, "
                      "splitting into %u x %u KB DMA transfers\n",
                      __func__,
                      (nblocksy * st->base.stride + 1023) / 1024,
                      (nblocksy + st->hw_nblocksy - 1) / st->hw_nblocksy,
                      (st->hw_nblocksy * st->base.stride + 1023) / 1024);
      }

      st->swbuf = MALLOC(nblocksy * st->base.stride * d);
      if (!st->swbuf) {
         sws->buffer_destroy(sws, st->hwbuf);
         return NULL;
      }
   }

   if (usage & PIPE_MAP_READ) {
      SVGA3dSurfaceDMAFlags flags;
      memset(&flags, 0, sizeof flags);
      svga_transfer_dma(svga, st, SVGA3D_READ_HOST_VRAM, flags);
   }

   if (st->swbuf) {
      return st->swbuf;
   }
   else {
      return sws->buffer_map(sws, st->hwbuf, usage);
   }
}


/**
 * Use direct map for the transfer request
 */
static void *
svga_texture_transfer_map_direct(struct svga_context *svga,
                                 struct svga_transfer *st)
{
   struct svga_winsys_screen *sws = svga_screen(svga->pipe.screen)->sws;
   struct pipe_transfer *transfer = &st->base;
   struct pipe_resource *texture = transfer->resource;
   struct svga_texture *tex = svga_texture(texture);
   struct svga_winsys_surface *surf = tex->handle;
   unsigned level = st->base.level;
   unsigned w, h, nblocksx, nblocksy;
   unsigned usage = st->base.usage;

   if (need_tex_readback(st)) {
      svga_surfaces_flush(svga);

      if (!svga->swc->force_coherent || tex->imported) {
         /* Readback the whole surface */
         readback_texture_surface(svga, tex, surf);

         svga_context_finish(svga);
      }
      /*
       * Note: if PIPE_MAP_DISCARD_WHOLE_RESOURCE were specified
       * we could potentially clear the flag for all faces/layers/mips.
       */
      svga_clear_texture_rendered_to(tex);
   }
   else {
      assert(usage & PIPE_MAP_WRITE);
      if ((usage & PIPE_MAP_UNSYNCHRONIZED) == 0) {
         if (svga_is_texture_level_dirty(tex, st->slice, level)) {
            /*
             * do a surface flush if the subresource has been modified
             * in this command buffer.
             */
            svga_surfaces_flush(svga);
            if (!sws->surface_is_flushed(sws, surf)) {
               svga->hud.surface_write_flushes++;
               SVGA_STATS_COUNT_INC(sws, SVGA_STATS_COUNT_SURFACEWRITEFLUSH);
               svga_context_flush(svga, NULL);
            }
         }
      }
   }

   /* we'll directly access the guest-backed surface */
   w = u_minify(texture->width0, level);
   h = u_minify(texture->height0, level);
   nblocksx = util_format_get_nblocksx(texture->format, w);
   nblocksy = util_format_get_nblocksy(texture->format, h);
   st->hw_nblocksy = nblocksy;
   st->base.stride = nblocksx*util_format_get_blocksize(texture->format);
   st->base.layer_stride = st->base.stride * nblocksy;

   /*
    * Begin mapping code
    */
   {
      SVGA3dSize baseLevelSize;
      uint8_t *map;
      bool retry, rebind;
      unsigned offset, mip_width, mip_height;
      struct svga_winsys_context *swc = svga->swc;

      if (swc->force_coherent) {
         usage |= PIPE_MAP_PERSISTENT | PIPE_MAP_COHERENT;
      }

      map = SVGA_TRY_MAP(svga->swc->surface_map
                         (svga->swc, surf, usage, &retry, &rebind), retry);

      if (map == NULL && retry) {
         /*
          * At this point, the svga_surfaces_flush() should already have
          * called in svga_texture_get_transfer().
          */
         svga->hud.surface_write_flushes++;
         svga_retry_enter(svga);
         svga_context_flush(svga, NULL);
         map = svga->swc->surface_map(svga->swc, surf, usage, &retry, &rebind);
         svga_retry_exit(svga);
      }
      if (map && rebind) {
         enum pipe_error ret;

         ret = SVGA3D_BindGBSurface(swc, surf);
         if (ret != PIPE_OK) {
            svga_context_flush(svga, NULL);
            ret = SVGA3D_BindGBSurface(swc, surf);
            assert(ret == PIPE_OK);
         }
         svga_context_flush(svga, NULL);
      }

      /*
       * Make sure we return NULL if the map fails
       */
      if (!map) {
         return NULL;
      }

      /**
       * Compute the offset to the specific texture slice in the buffer.
       */
      baseLevelSize.width = tex->b.width0;
      baseLevelSize.height = tex->b.height0;
      baseLevelSize.depth = tex->b.depth0;

      if ((tex->b.target == PIPE_TEXTURE_1D_ARRAY) ||
          (tex->b.target == PIPE_TEXTURE_2D_ARRAY) ||
          (tex->b.target == PIPE_TEXTURE_CUBE_ARRAY)) {
         st->base.layer_stride =
            svga3dsurface_get_image_offset(tex->key.format, baseLevelSize,
                                           tex->b.last_level + 1, 1, 0);
      }

      offset = svga3dsurface_get_image_offset(tex->key.format, baseLevelSize,
                                              tex->b.last_level + 1, /* numMips */
                                              st->slice, level);
      if (level > 0) {
         assert(offset > 0);
      }

      mip_width = u_minify(tex->b.width0, level);
      mip_height = u_minify(tex->b.height0, level);

      offset += svga3dsurface_get_pixel_offset(tex->key.format,
                                               mip_width, mip_height,
                                               st->box.x,
                                               st->box.y,
                                               st->box.z);

      return (void *) (map + offset);
   }
}


/**
 * Request a transfer map to the texture resource
 */
void *
svga_texture_transfer_map(struct pipe_context *pipe,
                          struct pipe_resource *texture,
                          unsigned level,
                          unsigned usage,
                          const struct pipe_box *box,
                          struct pipe_transfer **ptransfer)
{
   struct svga_context *svga = svga_context(pipe);
   struct svga_winsys_screen *sws = svga_screen(pipe->screen)->sws;
   struct svga_texture *tex = svga_texture(texture);
   struct svga_transfer *st;
   struct svga_winsys_surface *surf = tex->handle;
   bool use_direct_map = svga_have_gb_objects(svga) &&
                         (!svga_have_gb_dma(svga) || (usage & PIPE_MAP_WRITE));
   void *map = NULL;
   int64_t begin = svga_get_time(svga);

   SVGA_STATS_TIME_PUSH(sws, SVGA_STATS_TIME_TEXTRANSFERMAP);

   if (!surf)
      goto done;

   /* We can't map texture storage directly unless we have GB objects */
   if (usage & PIPE_MAP_DIRECTLY) {
      if (svga_have_gb_objects(svga))
         use_direct_map = true;
      else
         goto done;
   }

   st = CALLOC_STRUCT(svga_transfer);
   if (!st)
      goto done;

   st->base.level = level;
   st->base.usage = usage;
   st->base.box = *box;

   /* The modified transfer map box with the array index removed from z.
    * The array index is specified in slice.
    */
   st->box.x = box->x;
   st->box.y = box->y;
   st->box.z = box->z;
   st->box.w = box->width;
   st->box.h = box->height;
   st->box.d = box->depth;

   switch (tex->b.target) {
   case PIPE_TEXTURE_CUBE:
      st->slice = st->base.box.z;
      st->box.z = 0;   /* so we don't apply double offsets below */
      break;
   case PIPE_TEXTURE_1D_ARRAY:
   case PIPE_TEXTURE_2D_ARRAY:
   case PIPE_TEXTURE_CUBE_ARRAY:
      st->slice = st->base.box.z;
      st->box.z = 0;   /* so we don't apply double offsets below */

      /* Force direct map for transfering multiple slices */
      if (st->base.box.depth > 1)
         use_direct_map = svga_have_gb_objects(svga);

      break;
   default:
      st->slice = 0;
      break;
   }

   /* We never want to use DMA transfers on systems with GBObjects because
    * it causes serialization issues and in SVGAv3 vram is gone which
    * makes it impossible to support both at the same time.
    */
   if (svga_have_gb_objects(svga)) {
      use_direct_map = true;
   }

   st->use_direct_map = use_direct_map;
   pipe_resource_reference(&st->base.resource, texture);

   /* If this is the first time mapping to the surface in this
    * command buffer and there is no pending primitives, clear
    * the dirty masks of this surface.
    */
   if (sws->surface_is_flushed(sws, surf) &&
       (svga_have_vgpu10(svga) ||
        !svga_hwtnl_has_pending_prim(svga->hwtnl))) {
      svga_clear_texture_dirty(tex);
   }

   if (!use_direct_map) {
      /* upload to the DMA buffer */
      map = svga_texture_transfer_map_dma(svga, st);
   }
   else {
      bool can_use_upload = tex->can_use_upload &&
                            !(st->base.usage & PIPE_MAP_READ);
      bool was_rendered_to =
         svga_was_texture_rendered_to(svga_texture(texture));
      bool is_dirty = svga_is_texture_dirty(svga_texture(texture));

      /* If the texture was already rendered to or has pending changes and
       * upload buffer is supported, then we will use upload buffer to
       * avoid the need to read back the texture content; otherwise,
       * we'll first try to map directly to the GB surface, if it is blocked,
       * then we'll try the upload buffer.
       */
      if ((was_rendered_to || is_dirty) && can_use_upload) {
         map = svga_texture_transfer_map_upload(svga, st);
      }
      else {
         unsigned orig_usage = st->base.usage;

         /* First try directly map to the GB surface */
         if (can_use_upload)
            st->base.usage |= PIPE_MAP_DONTBLOCK;
         map = svga_texture_transfer_map_direct(svga, st);
         st->base.usage = orig_usage;

         if (!map && can_use_upload) {
            /* if direct map with DONTBLOCK fails, then try upload to the
             * texture upload buffer.
             */
            map = svga_texture_transfer_map_upload(svga, st);
         }
      }

      /* If upload fails, then try direct map again without forcing it
       * to DONTBLOCK.
       */
      if (!map) {
         map = svga_texture_transfer_map_direct(svga, st);
      }
   }

   if (!map) {
      FREE(st);
   }
   else {
      *ptransfer = &st->base;
      svga->hud.num_textures_mapped++;
      if (usage & PIPE_MAP_WRITE) {
         /* record texture upload for HUD */
         svga->hud.num_bytes_uploaded +=
            st->base.layer_stride * st->box.d;

         /* mark this texture level as dirty */
         svga_set_texture_dirty(tex, st->slice, level);
      }
   }

done:
   svga->hud.map_buffer_time += (svga_get_time(svga) - begin);
   SVGA_STATS_TIME_POP(sws);
   (void) sws;

   return map;
}

/**
 * Unmap a GB texture surface.
 */
static void
svga_texture_surface_unmap(struct svga_context *svga,
                           struct pipe_transfer *transfer)
{
   struct svga_winsys_surface *surf = svga_texture(transfer->resource)->handle;
   struct svga_winsys_context *swc = svga->swc;
   bool rebind;

   assert(surf);

   swc->surface_unmap(swc, surf, &rebind);
   if (rebind) {
      SVGA_RETRY(svga, SVGA3D_BindGBSurface(swc, surf));
   }
}


static void
update_image_vgpu9(struct svga_context *svga,
                   struct svga_winsys_surface *surf,
                   const SVGA3dBox *box,
                   unsigned slice,
                   unsigned level)
{
   SVGA_RETRY(svga, SVGA3D_UpdateGBImage(svga->swc, surf, box, slice, level));
}


static void
update_image_vgpu10(struct svga_context *svga,
                    struct svga_winsys_surface *surf,
                    const SVGA3dBox *box,
                    unsigned slice,
                    unsigned level,
                    unsigned numMipLevels)
{
   unsigned subResource;

   subResource = slice * numMipLevels + level;

   SVGA_RETRY(svga, SVGA3D_vgpu10_UpdateSubResource(svga->swc, surf, box,
                                                    subResource));
}


/**
 * unmap DMA transfer request
 */
static void
svga_texture_transfer_unmap_dma(struct svga_context *svga,
                                struct svga_transfer *st)
{
   struct svga_winsys_screen *sws = svga_screen(svga->pipe.screen)->sws;

   if (!st->swbuf)
      sws->buffer_unmap(sws, st->hwbuf);

   if (st->base.usage & PIPE_MAP_WRITE) {
      /* Use DMA to transfer texture data */
      SVGA3dSurfaceDMAFlags flags;
      struct pipe_resource *texture = st->base.resource;
      struct svga_texture *tex = svga_texture(texture);


      memset(&flags, 0, sizeof flags);
      if (st->base.usage & PIPE_MAP_DISCARD_WHOLE_RESOURCE) {
         flags.discard = true;
      }
      if (st->base.usage & PIPE_MAP_UNSYNCHRONIZED) {
         flags.unsynchronized = true;
      }

      svga_transfer_dma(svga, st, SVGA3D_WRITE_HOST_VRAM, flags);
      svga_set_texture_rendered_to(tex);
   }

   FREE(st->swbuf);
   sws->buffer_destroy(sws, st->hwbuf);
}


/**
 * unmap direct map transfer request
 */
static void
svga_texture_transfer_unmap_direct(struct svga_context *svga,
                                   struct svga_transfer *st)
{
   struct pipe_transfer *transfer = &st->base;
   struct svga_texture *tex = svga_texture(transfer->resource);

   svga_texture_surface_unmap(svga, transfer);

   /* Now send an update command to update the content in the backend. */
   if (st->base.usage & PIPE_MAP_WRITE) {
      struct svga_winsys_surface *surf = tex->handle;

      assert(svga_have_gb_objects(svga));

      /* update the effected region */
      SVGA3dBox box = st->box;
      unsigned nlayers;

      switch (tex->b.target) {
      case PIPE_TEXTURE_2D_ARRAY:
      case PIPE_TEXTURE_CUBE_ARRAY:
      case PIPE_TEXTURE_1D_ARRAY:
         nlayers = box.d;
         box.d = 1;
         break;
      default:
         nlayers = 1;
         break;
      }


      if (0)
         debug_printf("%s %d, %d, %d  %d x %d x %d\n",
                      __func__,
                      box.x, box.y, box.z,
                      box.w, box.h, box.d);

      if (!svga->swc->force_coherent || tex->imported) {
         if (svga_have_vgpu10(svga)) {
            unsigned i;

            for (i = 0; i < nlayers; i++) {
               update_image_vgpu10(svga, surf, &box,
                                   st->slice + i, transfer->level,
                                   tex->b.last_level + 1);
            }
         } else {
            assert(nlayers == 1);
            update_image_vgpu9(svga, surf, &box, st->slice,
                               transfer->level);
         }
      }

      /* Mark the texture surface state as UPDATED */
      tex->surface_state = SVGA_SURFACE_STATE_UPDATED;
   }
}


void
svga_texture_transfer_unmap(struct pipe_context *pipe,
                            struct pipe_transfer *transfer)
{
   struct svga_context *svga = svga_context(pipe);
   struct svga_screen *ss = svga_screen(pipe->screen);
   struct svga_winsys_screen *sws = ss->sws;
   struct svga_transfer *st = svga_transfer(transfer);
   struct svga_texture *tex = svga_texture(transfer->resource);

   SVGA_STATS_TIME_PUSH(sws, SVGA_STATS_TIME_TEXTRANSFERUNMAP);

   if (!st->use_direct_map) {
      svga_texture_transfer_unmap_dma(svga, st);
   }
   else if (st->upload.buf) {
      svga_texture_transfer_unmap_upload(svga, st);
   }
   else {
      svga_texture_transfer_unmap_direct(svga, st);
   }

   if (st->base.usage & PIPE_MAP_WRITE) {
      svga->hud.num_resource_updates++;

      /* Mark the texture level as dirty */
      ss->texture_timestamp++;
      svga_age_texture_view(tex, transfer->level);
      if (transfer->resource->target == PIPE_TEXTURE_CUBE)
         svga_define_texture_level(tex, st->slice, transfer->level);
      else
         svga_define_texture_level(tex, 0, transfer->level);
   }

   pipe_resource_reference(&st->base.resource, NULL);
   FREE(st);
   SVGA_STATS_TIME_POP(sws);
   (void) sws;
}


/**
 * Does format store depth values?
 */
static inline bool
format_has_depth(enum pipe_format format)
{
   const struct util_format_description *desc = util_format_description(format);
   return util_format_has_depth(desc);
}

struct pipe_resource *
svga_texture_create(struct pipe_screen *screen,
                    const struct pipe_resource *template)
{
   struct svga_screen *svgascreen = svga_screen(screen);
   struct svga_texture *tex;
   unsigned bindings = template->bind;

   SVGA_STATS_TIME_PUSH(svgascreen->sws,
                        SVGA_STATS_TIME_CREATETEXTURE);

   assert(template->last_level < SVGA_MAX_TEXTURE_LEVELS);
   if (template->last_level >= SVGA_MAX_TEXTURE_LEVELS) {
      goto fail_notex;
   }

   /* Verify the number of mipmap levels isn't impossibly large.  For example,
    * if the base 2D image is 16x16, we can't have 8 mipmap levels.
    * the gallium frontend should never ask us to create a resource with invalid
    * parameters.
    */
   {
      unsigned max_dim = template->width0;

      switch (template->target) {
      case PIPE_TEXTURE_1D:
      case PIPE_TEXTURE_1D_ARRAY:
         // nothing
         break;
      case PIPE_TEXTURE_2D:
      case PIPE_TEXTURE_CUBE:
      case PIPE_TEXTURE_CUBE_ARRAY:
      case PIPE_TEXTURE_2D_ARRAY:
         max_dim = MAX2(max_dim, template->height0);
         break;
      case PIPE_TEXTURE_3D:
         max_dim = MAX3(max_dim, template->height0, template->depth0);
         break;
      case PIPE_TEXTURE_RECT:
      case PIPE_BUFFER:
         assert(template->last_level == 0);
         /* the assertion below should always pass */
         break;
      default:
         debug_printf("Unexpected texture target type\n");
      }
      assert(1 << template->last_level <= max_dim);
   }

   tex = CALLOC_STRUCT(svga_texture);
   if (!tex) {
      goto fail_notex;
   }

   tex->defined = CALLOC(template->depth0 * template->array_size,
                         sizeof(tex->defined[0]));
   if (!tex->defined) {
      FREE(tex);
      goto fail_notex;
   }

   tex->dirty = CALLOC(template->depth0 * template->array_size,
                             sizeof(tex->dirty[0]));
   if (!tex->dirty) {
      goto fail;
   }

   tex->b = *template;
   pipe_reference_init(&tex->b.reference, 1);
   tex->b.screen = screen;

   tex->key.flags = 0;
   tex->key.size.width = template->width0;
   tex->key.size.height = template->height0;
   tex->key.size.depth = template->depth0;
   tex->key.arraySize = 1;
   tex->key.numFaces = 1;

   /* nr_samples=1 must be treated as a non-multisample texture */
   if (tex->b.nr_samples == 1) {
      tex->b.nr_samples = 0;
   }
   else if (tex->b.nr_samples > 1) {
      assert(svgascreen->sws->have_sm4_1);
      tex->key.flags |= SVGA3D_SURFACE_MULTISAMPLE;
   }

   tex->key.sampleCount = tex->b.nr_samples;

   if (svgascreen->sws->have_vgpu10) {
      switch (template->target) {
      case PIPE_TEXTURE_1D:
         tex->key.flags |= SVGA3D_SURFACE_1D;
         break;
      case PIPE_TEXTURE_1D_ARRAY:
         tex->key.flags |= SVGA3D_SURFACE_1D;
         FALLTHROUGH;
      case PIPE_TEXTURE_2D_ARRAY:
         tex->key.flags |= SVGA3D_SURFACE_ARRAY;
         tex->key.arraySize = template->array_size;
         break;
      case PIPE_TEXTURE_3D:
         tex->key.flags |= SVGA3D_SURFACE_VOLUME;
         break;
      case PIPE_TEXTURE_CUBE:
         tex->key.flags |= (SVGA3D_SURFACE_CUBEMAP | SVGA3D_SURFACE_ARRAY);
         tex->key.numFaces = 6;
         break;
      case PIPE_TEXTURE_CUBE_ARRAY:
         assert(svgascreen->sws->have_sm4_1);
         tex->key.flags |= (SVGA3D_SURFACE_CUBEMAP | SVGA3D_SURFACE_ARRAY);
         tex->key.numFaces = 1;  // arraySize already includes the 6 faces
         tex->key.arraySize = template->array_size;
         break;
      default:
         break;
      }
   }
   else {
      switch (template->target) {
      case PIPE_TEXTURE_3D:
         tex->key.flags |= SVGA3D_SURFACE_VOLUME;
         break;
      case PIPE_TEXTURE_CUBE:
         tex->key.flags |= SVGA3D_SURFACE_CUBEMAP;
         tex->key.numFaces = 6;
         break;
      default:
         break;
      }
   }

   tex->key.cachable = 1;

   if ((bindings & (PIPE_BIND_RENDER_TARGET | PIPE_BIND_DEPTH_STENCIL)) &&
       !(bindings & PIPE_BIND_SAMPLER_VIEW)) {
      /* Also check if the format can be sampled from */
      if (screen->is_format_supported(screen, template->format,
                                      template->target,
                                      template->nr_samples,
                                      template->nr_storage_samples,
                                      PIPE_BIND_SAMPLER_VIEW)) {
         bindings |= PIPE_BIND_SAMPLER_VIEW;
      }
   }

   if (bindings & PIPE_BIND_SAMPLER_VIEW) {
      tex->key.flags |= SVGA3D_SURFACE_HINT_TEXTURE;
      tex->key.flags |= SVGA3D_SURFACE_BIND_SHADER_RESOURCE;

      if (!(bindings & PIPE_BIND_RENDER_TARGET)) {
         /* Also check if the format is color renderable */
         if (screen->is_format_supported(screen, template->format,
                                         template->target,
                                         template->nr_samples,
                                         template->nr_storage_samples,
                                         PIPE_BIND_RENDER_TARGET)) {
            bindings |= PIPE_BIND_RENDER_TARGET;
         }
      }

      if (!(bindings & PIPE_BIND_DEPTH_STENCIL)) {
         /* Also check if the format is depth/stencil renderable */
         if (screen->is_format_supported(screen, template->format,
                                         template->target,
                                         template->nr_samples,
                                         template->nr_storage_samples,
                                         PIPE_BIND_DEPTH_STENCIL)) {
            bindings |= PIPE_BIND_DEPTH_STENCIL;
         }
      }
   }

   if (bindings & PIPE_BIND_DISPLAY_TARGET) {
      tex->key.cachable = 0;
   }

   if (bindings & PIPE_BIND_SHARED) {
      tex->key.cachable = 0;
   }

   if (bindings & (PIPE_BIND_SCANOUT | PIPE_BIND_CURSOR)) {
      tex->key.scanout = 1;
      tex->key.cachable = 0;
   }

   /*
    * Note: Previously we never passed the
    * SVGA3D_SURFACE_HINT_RENDERTARGET hint. Mesa cannot
    * know beforehand whether a texture will be used as a rendertarget or not
    * and it always requests PIPE_BIND_RENDER_TARGET, therefore
    * passing the SVGA3D_SURFACE_HINT_RENDERTARGET here defeats its purpose.
    *
    * However, this was changed since other gallium frontends
    * (XA for example) uses it accurately and certain device versions
    * relies on it in certain situations to render correctly.
    */
   if ((bindings & PIPE_BIND_RENDER_TARGET) &&
       !util_format_is_s3tc(template->format)) {
      tex->key.flags |= SVGA3D_SURFACE_HINT_RENDERTARGET;
      tex->key.flags |= SVGA3D_SURFACE_BIND_RENDER_TARGET;
   }

   if (bindings & PIPE_BIND_DEPTH_STENCIL) {
      tex->key.flags |= SVGA3D_SURFACE_HINT_DEPTHSTENCIL;
      tex->key.flags |= SVGA3D_SURFACE_BIND_DEPTH_STENCIL;
   }

   tex->key.numMipLevels = template->last_level + 1;

   tex->key.format = svga_translate_format(svgascreen, template->format,
                                           bindings);
   if (tex->key.format == SVGA3D_FORMAT_INVALID) {
      goto fail;
   }

   bool use_typeless = false;
   if (svgascreen->sws->have_gl43) {
      /* Do not use typeless for SHARED, SCANOUT or DISPLAY_TARGET surfaces. */
      use_typeless = !(bindings & (PIPE_BIND_SHARED | PIPE_BIND_SCANOUT |
                                   PIPE_BIND_DISPLAY_TARGET));
   } else if (svgascreen->sws->have_vgpu10) {
      /* For VGPU10 device, use typeless formats only for sRGB and depth resources
       * if they do not have SHARED, SCANOUT or DISPLAY_TARGET bind flags
       */
      use_typeless = (util_format_is_srgb(template->format) ||
                      format_has_depth(template->format)) &&
                     !(bindings & (PIPE_BIND_SHARED | PIPE_BIND_SCANOUT |
                                   PIPE_BIND_DISPLAY_TARGET));
   }

   if (use_typeless) {
      SVGA3dSurfaceFormat typeless = svga_typeless_format(tex->key.format);
      if (0) {
         debug_printf("Convert resource type %s -> %s (bind 0x%x)\n",
                      svga_format_name(tex->key.format),
                      svga_format_name(typeless),
                      bindings);
      }

      if (svga_format_is_uncompressed_snorm(tex->key.format)) {
         /* We can't normally render to snorm surfaces, but once we
          * substitute a typeless format, we can if the rendertarget view
          * is unorm.  This can happen with GL_ARB_copy_image.
          */
         tex->key.flags |= SVGA3D_SURFACE_HINT_RENDERTARGET;
         tex->key.flags |= SVGA3D_SURFACE_BIND_RENDER_TARGET;
      }

      tex->key.format = typeless;
   }

   if (svgascreen->sws->have_sm5 &&
       bindings & (PIPE_BIND_SAMPLER_VIEW | PIPE_BIND_RENDER_TARGET)) {
       if (template->nr_samples < 2 &&
           screen->is_format_supported(screen, template->format,
                                       template->target,
                                       template->nr_samples,
                                       template->nr_storage_samples,
                                       PIPE_BIND_SHADER_IMAGE)) {
          /* Any non multi-samples texture that can be used as a render target
           * or sampler view can be bound to an image unit.
           * So make sure to set the UAV flag here.
           */
          tex->key.flags |= SVGA3D_SURFACE_BIND_UAVIEW;
       }
   }

   SVGA_DBG(DEBUG_DMA, "surface_create for texture\n");
   bool invalidated;
   tex->handle = svga_screen_surface_create(svgascreen, bindings,
                                            tex->b.usage,
                                            &invalidated, &tex->key);
   if (!tex->handle) {
      goto fail;
   }
   if (invalidated) {
      tex->surface_state = SVGA_SURFACE_STATE_INVALIDATED;
   } else {
      tex->surface_state = SVGA_SURFACE_STATE_CREATED;
   }

   SVGA_DBG(DEBUG_DMA, "  --> got sid %p (texture)\n", tex->handle);

   debug_reference(&tex->b.reference,
                   (debug_reference_descriptor)debug_describe_resource, 0);

   tex->size = util_resource_size(template);

   /* Determine if texture upload buffer can be used to upload this texture */
   tex->can_use_upload = svga_texture_transfer_map_can_upload(svgascreen,
                                                              &tex->b);

   /* Initialize the backing resource cache */
   tex->backed_handle = NULL;

   svgascreen->hud.total_resource_bytes += tex->size;
   svgascreen->hud.num_resources++;

   SVGA_STATS_TIME_POP(svgascreen->sws);

   return &tex->b;

fail:
   if (tex->dirty)
      FREE(tex->dirty);
   if (tex->defined)
      FREE(tex->defined);
   FREE(tex);
fail_notex:
   SVGA_STATS_TIME_POP(svgascreen->sws);
   return NULL;
}


struct pipe_resource *
svga_texture_from_handle(struct pipe_screen *screen,
                         const struct pipe_resource *template,
                         struct winsys_handle *whandle)
{
   struct svga_winsys_screen *sws = svga_winsys_screen(screen);
   struct svga_screen *ss = svga_screen(screen);
   struct svga_winsys_surface *srf;
   struct svga_texture *tex;
   enum SVGA3dSurfaceFormat format = 0;
   assert(screen);

   /* Only supports one type */
   if ((template->target != PIPE_TEXTURE_2D &&
       template->target != PIPE_TEXTURE_RECT) ||
       template->last_level != 0 ||
       template->depth0 != 1) {
      return NULL;
   }

   srf = sws->surface_from_handle(sws, whandle, &format);

   if (!srf)
      return NULL;

   if (!svga_format_is_shareable(ss, template->format, format,
                                 template->bind, true))
      goto out_unref;

   tex = CALLOC_STRUCT(svga_texture);
   if (!tex)
      goto out_unref;

   tex->defined = CALLOC(template->depth0 * template->array_size,
                         sizeof(tex->defined[0]));
   if (!tex->defined)
      goto out_no_defined;

   tex->b = *template;
   pipe_reference_init(&tex->b.reference, 1);
   tex->b.screen = screen;

   SVGA_DBG(DEBUG_DMA, "wrap surface sid %p\n", srf);

   tex->key.cachable = 0;
   tex->key.format = format;
   tex->handle = srf;


   /* set bind flags for the imported texture handle according to the bind
    * flags in the template
    */
   if (template->bind & PIPE_BIND_RENDER_TARGET){
      tex->key.flags |= SVGA3D_SURFACE_HINT_RENDERTARGET;
      tex->key.flags |= SVGA3D_SURFACE_BIND_RENDER_TARGET;
   }

   if (template->bind & PIPE_BIND_DEPTH_STENCIL) {
      tex->key.flags |= SVGA3D_SURFACE_HINT_DEPTHSTENCIL;
      tex->key.flags |= SVGA3D_SURFACE_BIND_DEPTH_STENCIL;
   }

   if (template->bind & PIPE_BIND_SAMPLER_VIEW) {
      tex->key.flags |= SVGA3D_SURFACE_HINT_TEXTURE;
      tex->key.flags |= SVGA3D_SURFACE_BIND_SHADER_RESOURCE;
   }

   tex->dirty = CALLOC(1, sizeof(tex->dirty[0]));
   if (!tex->dirty)
      goto out_no_dirty;

   tex->imported = true;

   ss->hud.num_resources++;

   return &tex->b;

out_no_dirty:
   FREE(tex->defined);
out_no_defined:
   FREE(tex);
out_unref:
   sws->surface_reference(sws, &srf, NULL);
   return NULL;
}

bool
svga_texture_generate_mipmap(struct pipe_context *pipe,
                             struct pipe_resource *pt,
                             enum pipe_format format,
                             unsigned base_level,
                             unsigned last_level,
                             unsigned first_layer,
                             unsigned last_layer)
{
   struct pipe_sampler_view templ, *psv;
   struct svga_pipe_sampler_view *sv;
   struct svga_context *svga = svga_context(pipe);
   struct svga_texture *tex = svga_texture(pt);

   assert(svga_have_vgpu10(svga));

   /* Fallback to the mipmap generation utility for those formats that
    * do not support hw generate mipmap
    */
   if (!svga_format_support_gen_mips(format))
      return false;

   /* Make sure the texture surface was created with
    * SVGA3D_SURFACE_BIND_RENDER_TARGET
    */
   if (!tex->handle || !(tex->key.flags & SVGA3D_SURFACE_BIND_RENDER_TARGET))
      return false;

   templ.format = format;
   templ.target = pt->target;
   templ.u.tex.first_layer = first_layer;
   templ.u.tex.last_layer = last_layer;
   templ.u.tex.first_level = base_level;
   templ.u.tex.last_level = last_level;

   if (pt->target == PIPE_TEXTURE_CUBE) {
      /**
       * state tracker generates mipmap one face at a time.
       * But SVGA generates mipmap for the entire cubemap.
       */
      templ.u.tex.first_layer = 0;
      templ.u.tex.last_layer = 5;
   }

   psv = pipe->create_sampler_view(pipe, pt, &templ);
   if (psv == NULL)
      return false;

   sv = svga_pipe_sampler_view(psv);
   SVGA_RETRY(svga, svga_validate_pipe_sampler_view(svga, sv));

   SVGA_RETRY(svga, SVGA3D_vgpu10_GenMips(svga->swc, sv->id, tex->handle));
   pipe_sampler_view_reference(&psv, NULL);

   /* Mark the texture surface as RENDERED */
   svga_set_texture_rendered_to(tex);

   svga->hud.num_generate_mipmap++;

   return true;
}


/* texture upload buffer default size in bytes */
#define TEX_UPLOAD_DEFAULT_SIZE (1024 * 1024)

/**
 * Create a texture upload buffer
 */
bool
svga_texture_transfer_map_upload_create(struct svga_context *svga)
{
   svga->tex_upload = u_upload_create(&svga->pipe, TEX_UPLOAD_DEFAULT_SIZE,
                                      PIPE_BIND_CUSTOM, PIPE_USAGE_STAGING, 0);
   if (svga->tex_upload)
      u_upload_disable_persistent(svga->tex_upload);

   return svga->tex_upload != NULL;
}


/**
 * Destroy the texture upload buffer
 */
void
svga_texture_transfer_map_upload_destroy(struct svga_context *svga)
{
   u_upload_destroy(svga->tex_upload);
}


/**
 * Returns true if this transfer map request can use the upload buffer.
 */
bool
svga_texture_transfer_map_can_upload(const struct svga_screen *svgascreen,
                                     const struct pipe_resource *texture)
{
   if (svgascreen->sws->have_transfer_from_buffer_cmd == false)
      return false;

   /* TransferFromBuffer command is not well supported with multi-samples surface */
   if (texture->nr_samples > 1)
      return false;

   if (util_format_is_compressed(texture->format)) {
      /* XXX Need to take a closer look to see why texture upload
       * with 3D texture with compressed format fails
       */ 
      if (texture->target == PIPE_TEXTURE_3D)
          return false;
   }
   else if (texture->format == PIPE_FORMAT_R9G9B9E5_FLOAT) {
      return false;
   }

   return true;
}


/**
 *  Return TRUE if the same texture is bound to the specified
 *  surface view and a backing resource is created for the surface view.
 */
static bool
need_update_texture_resource(struct pipe_surface *surf,
		             struct svga_texture *tex)
{
   struct svga_texture *stex = svga_texture(surf->texture);
   struct svga_surface *s = svga_surface(surf);

   return (stex == tex && s->handle != tex->handle);
}


/**
 *  Make sure the texture resource is up-to-date. If the texture is
 *  currently bound to a render target view and a backing resource is
 *  created, we will need to update the original resource with the
 *  changes in the backing resource.
 */
static void
svga_validate_texture_resource(struct svga_context *svga,
		               struct svga_texture *tex)
{
   if (svga_was_texture_rendered_to(tex) == false)
      return;

   if ((svga->state.hw_draw.has_backed_views == false) ||
       (tex->backed_handle == NULL))
      return;

   struct pipe_surface *s;
   for (unsigned i = 0; i < svga->state.hw_clear.num_rendertargets; i++) {
      s = svga->state.hw_clear.rtv[i];
      if (s && need_update_texture_resource(s, tex))
         svga_propagate_surface(svga, s, true);
   }

   s = svga->state.hw_clear.dsv;
   if (s && need_update_texture_resource(s, tex))
      svga_propagate_surface(svga, s, true);
}


/**
 * Use upload buffer for the transfer map request.
 */
void *
svga_texture_transfer_map_upload(struct svga_context *svga,
                                 struct svga_transfer *st)
{
   struct pipe_resource *texture = st->base.resource;
   struct pipe_resource *tex_buffer = NULL;
   struct svga_texture *tex = svga_texture(texture);
   void *tex_map;
   unsigned nblocksx, nblocksy;
   unsigned offset;
   unsigned upload_size;

   assert(svga->tex_upload);

   /* Validate the texture resource in case there is any changes
    * in the backing resource that needs to be updated to the original
    * texture resource first before the transfer upload occurs, otherwise,
    * the later update from backing resource to original will overwrite the
    * changes in this transfer map update.
    */
   svga_validate_texture_resource(svga, tex);

   st->upload.box.x = st->base.box.x;
   st->upload.box.y = st->base.box.y;
   st->upload.box.z = st->base.box.z;
   st->upload.box.w = st->base.box.width;
   st->upload.box.h = st->base.box.height;
   st->upload.box.d = st->base.box.depth;
   st->upload.nlayers = 1;

   switch (texture->target) {
   case PIPE_TEXTURE_CUBE:
      st->upload.box.z = 0;
      break;
   case PIPE_TEXTURE_2D_ARRAY:
   case PIPE_TEXTURE_CUBE_ARRAY:
      st->upload.nlayers = st->base.box.depth;
      st->upload.box.z = 0;
      st->upload.box.d = 1;
      break;
   case PIPE_TEXTURE_1D_ARRAY:
      st->upload.nlayers = st->base.box.depth;
      st->upload.box.y = st->upload.box.z = 0;
      st->upload.box.d = 1;
      break;
   default:
      break;
   }

   nblocksx = util_format_get_nblocksx(texture->format, st->base.box.width);
   nblocksy = util_format_get_nblocksy(texture->format, st->base.box.height);

   st->base.stride = nblocksx * util_format_get_blocksize(texture->format);
   st->base.layer_stride = st->base.stride * nblocksy;

   /* In order to use the TransferFromBuffer command to update the
    * texture content from the buffer, the layer stride for a multi-layers
    * surface needs to be in multiples of 16 bytes.
    */
   if (st->upload.nlayers > 1 && st->base.layer_stride & 15)
      return NULL;

   upload_size = st->base.layer_stride * st->base.box.depth;
   upload_size = align(upload_size, 16);

#ifdef DEBUG
   if (util_format_is_compressed(texture->format)) {
      unsigned blockw, blockh, bytesPerBlock;

      svga_format_size(tex->key.format, &blockw, &blockh, &bytesPerBlock);

      /* dest box must start on block boundary */
      assert((st->base.box.x % blockw) == 0);
      assert((st->base.box.y % blockh) == 0);
   }
#endif

   /* If the upload size exceeds the default buffer size, the
    * upload buffer manager code will try to allocate a new buffer
    * with the new buffer size.
    */
   u_upload_alloc(svga->tex_upload, 0, upload_size, 16,
                  &offset, &tex_buffer, &tex_map);

   if (!tex_map) {
      return NULL;
   }

   st->upload.buf = tex_buffer;
   st->upload.map = tex_map;
   st->upload.offset = offset;

   return tex_map;
}


/**
 * Unmap upload map transfer request
 */
void
svga_texture_transfer_unmap_upload(struct svga_context *svga,
                                   struct svga_transfer *st)
{
   struct svga_winsys_surface *srcsurf;
   struct svga_winsys_surface *dstsurf;
   struct pipe_resource *texture = st->base.resource;
   struct svga_texture *tex = svga_texture(texture);
   unsigned subResource;
   unsigned numMipLevels;
   unsigned i, layer;
   unsigned offset = st->upload.offset;

   assert(svga->tex_upload);
   assert(st->upload.buf);

   /* unmap the texture upload buffer */
   u_upload_unmap(svga->tex_upload);

   srcsurf = svga_buffer_handle(svga, st->upload.buf, 0);
   dstsurf = svga_texture(texture)->handle;
   assert(dstsurf);

   numMipLevels = texture->last_level + 1;

   for (i = 0, layer = st->slice; i < st->upload.nlayers; i++, layer++) {
      subResource = layer * numMipLevels + st->base.level;

      /* send a transferFromBuffer command to update the host texture surface */
      assert((offset & 15) == 0);

      SVGA_RETRY(svga, SVGA3D_vgpu10_TransferFromBuffer(svga->swc, srcsurf,
                                                        offset,
                                                        st->base.stride,
                                                        st->base.layer_stride,
                                                        dstsurf, subResource,
                                                        &st->upload.box));
      offset += st->base.layer_stride;
   }

   /* Mark the texture surface state as RENDERED */
   svga_set_texture_rendered_to(tex);

   pipe_resource_reference(&st->upload.buf, NULL);
}

/**
 * Does the device format backing this surface have an
 * alpha channel?
 *
 * \param texture[in]  The texture whose format we're querying
 * \return TRUE if the format has an alpha channel, FALSE otherwise
 *
 * For locally created textures, the device (svga) format is typically
 * identical to svga_format(texture->format), and we can use the gallium
 * format tests to determine whether the device format has an alpha channel
 * or not. However, for textures backed by imported svga surfaces that is
 * not always true, and we have to look at the SVGA3D utilities.
 */
bool
svga_texture_device_format_has_alpha(struct pipe_resource *texture)
{
   /* the svga_texture() call below is invalid for PIPE_BUFFER resources */
   assert(texture->target != PIPE_BUFFER);

   const struct svga3d_surface_desc *surf_desc =
      svga3dsurface_get_desc(svga_texture(texture)->key.format);

   enum svga3d_block_desc block_desc = surf_desc->block_desc;

   return !!((block_desc & SVGA3DBLOCKDESC_ALPHA) ||
             ((block_desc == SVGA3DBLOCKDESC_TYPELESS) &&
              (surf_desc->bitDepth.alpha > 0)));
}
