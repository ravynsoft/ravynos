/**********************************************************
 * Copyright 2022 VMware, Inc.  All rights reserved.
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

#include "pipe/p_defines.h"
#include "util/u_bitmask.h"
#include "util/format/u_format.h"
#include "util/u_inlines.h"
#include "util/u_math.h"
#include "util/u_memory.h"

#include "svga_context.h"
#include "svga_cmd.h"
#include "svga_debug.h"
#include "svga_resource_buffer.h"
#include "svga_resource_texture.h"
#include "svga_surface.h"
#include "svga_sampler_view.h"
#include "svga_format.h"


/**
 * Initialize uav cache.
 */
void
svga_uav_cache_init(struct svga_context *svga)
{
   struct svga_cache_uav *cache = &svga->cache_uav;

   for (unsigned i = 0; i < ARRAY_SIZE(cache->uaViews); i++) {
      cache->uaViews[i].uaViewId = SVGA3D_INVALID_ID;
      cache->uaViews[i].next_uaView = i + 1;
   }
   cache->num_uaViews = 0;
   cache->next_uaView = 0;
}


/**
 * Helper function to compare two image view descriptions.
 * Return TRUE if they are identical.
 */
static bool
image_view_desc_identical(struct pipe_image_view *img1,
                          struct pipe_image_view *img2)
{
   if ((img1->resource != img2->resource) ||
       (img1->format != img2->format) ||
       (img1->access != img2->access) ||
       (img1->shader_access != img2->shader_access))
      return false;

   if (img1->resource->target == PIPE_BUFFER) {
      if ((img1->u.buf.offset != img2->u.buf.offset) ||
          (img1->u.buf.size != img2->u.buf.size))
         return false;
   }

   return true;
}


/**
 * Helper function to compare two shader buffer descriptions.
 * Return TRUE if they are identical.
 */
static bool
shader_buffer_desc_identical(struct pipe_shader_buffer *buf1,
                             struct pipe_shader_buffer *buf2)
{
   return memcmp(buf1, buf2, sizeof(*buf1)) == 0;
}


/**
 * Helper function to compare two uav cache entry descriptions.
 * Return TRUE if they are identical.
 */
static bool
uav_desc_identical(enum svga_uav_type uav_type,
                   void *desc, void *uav_desc)
{
   if (uav_type == SVGA_IMAGE_VIEW) {
      struct svga_image_view *img = (struct svga_image_view *)desc;
      struct svga_image_view *uav_img = (struct svga_image_view *)uav_desc;
      if (img->resource != uav_img->resource)
         return false;

      return image_view_desc_identical(&img->desc, &uav_img->desc);
   }
   else {
      struct svga_shader_buffer *buf = (struct svga_shader_buffer *)desc;
      struct svga_shader_buffer *uav_buf =
         (struct svga_shader_buffer *)uav_desc;

      if (buf->resource != uav_buf->resource)
         return false;

      if (buf->handle != uav_buf->handle)
         return false;

      return shader_buffer_desc_identical(&buf->desc, &uav_buf->desc);
   }
}


/**
 * Find a uav object for the specified image view or shader buffer.
 * Returns uav entry if there is a match; otherwise returns NULL.
 */
static struct svga_uav *
svga_uav_cache_find_uav(struct svga_context *svga,
                        enum svga_uav_type uav_type,
                        void *desc,
                        unsigned desc_len)
{
   struct svga_cache_uav *cache = &svga->cache_uav;

   for (unsigned i = 0; i < cache->num_uaViews; i++) {
      if ((cache->uaViews[i].type == uav_type) &&
          (cache->uaViews[i].uaViewId != SVGA3D_INVALID_ID) &&
          uav_desc_identical(uav_type, desc, &cache->uaViews[i].desc)) {
         return &cache->uaViews[i];
      }
   }
   return NULL;
}


/**
 * Add a uav entry to the cache for the specified image view or
 * shaderr bufferr.
 */
static struct svga_uav *
svga_uav_cache_add_uav(struct svga_context *svga,
                       enum svga_uav_type uav_type,
                       void *desc,
                       unsigned desc_len,
                       struct pipe_resource *res,
                       SVGA3dUAViewId uaViewId)
{
   struct svga_cache_uav *cache = &svga->cache_uav;
   unsigned i = cache->next_uaView;
   struct svga_uav *uav;

   if (i > ARRAY_SIZE(cache->uaViews)) {
      debug_printf("No room to add uav to the cache.\n");
      return NULL;
   }

   uav = &cache->uaViews[i];

   /* update the next available uav slot index */
   cache->next_uaView = uav->next_uaView;

   uav->type = uav_type;
   memcpy(&uav->desc, desc, desc_len);
   pipe_resource_reference(&uav->resource, res);
   uav->uaViewId = uaViewId;

   cache->num_uaViews = MAX2(i+1, cache->num_uaViews);

   return uav;
}


/**
 * Bump the timestamp of the specified uav for the specified pipeline,
 * so the uav will not be prematurely purged.
 */
static void
svga_uav_cache_use_uav(struct svga_context *svga,
                       enum svga_pipe_type pipe_type,
                       struct svga_uav *uav)
{
   assert(uav != NULL);
   assert(uav->uaViewId != SVGA3D_INVALID_ID);

   uav->timestamp[pipe_type] = svga->state.uav_timestamp[pipe_type];
}


/**
 * Purge any unused uav from the cache.
 */
static void
svga_uav_cache_purge(struct svga_context *svga, enum svga_pipe_type pipe_type)
{
   struct svga_cache_uav *cache = &svga->cache_uav;
   unsigned timestamp = svga->state.uav_timestamp[pipe_type];
   unsigned other_pipe_type = !pipe_type;
   struct svga_uav *uav = &cache->uaViews[0];

   unsigned last_uav = -1;
   for (unsigned i = 0; i < cache->num_uaViews; i++, uav++) {
      if (uav->uaViewId != SVGA3D_INVALID_ID) {
         last_uav = i;

         if (uav->timestamp[pipe_type] < timestamp) {

            /* Reset the timestamp for this uav in the specified
             * pipeline first.
             */
            uav->timestamp[pipe_type] = 0;

            /* Then check if the uav is currently in use in other pipeline.
             * If yes, then don't delete the uav yet.
             * If no, then we can mark the uav as to be destroyed.
             */
            if (uav->timestamp[other_pipe_type] == 0) {

               /* The unused uav can be destroyed, but will be destroyed
                * in the next set_image_views or set_shader_buffers,
                * or at context destroy time, because we do not want to
                * restart the state update if the Destroy command cannot be
                * executed in this command buffer.
                */
               util_bitmask_set(svga->uav_to_free_id_bm, uav->uaViewId);

               /* Mark this entry as available */
               uav->next_uaView = cache->next_uaView;
               uav->uaViewId = SVGA3D_INVALID_ID;
               cache->next_uaView = i;
            }
         }
      }
   }
   cache->num_uaViews = last_uav + 1;
}


/**
 * A helper function to create an uav.
 */
SVGA3dUAViewId
svga_create_uav(struct svga_context *svga,
                SVGA3dUAViewDesc *desc,
                SVGA3dSurfaceFormat svga_format,
                unsigned resourceDim,
                struct svga_winsys_surface *surf)
{
   SVGA3dUAViewId uaViewId;
   enum pipe_error ret;

   /* allocate a uav id */
   uaViewId = util_bitmask_add(svga->uav_id_bm);

   SVGA_DBG(DEBUG_UAV, "%s: uavId=%d surf=0x%x\n", __func__, uaViewId, surf);

   ret = SVGA3D_sm5_DefineUAView(svga->swc, uaViewId, surf,
                                 svga_format, resourceDim, desc);

   if (ret != PIPE_OK) {
      util_bitmask_clear(svga->uav_id_bm, uaViewId);
      uaViewId = SVGA3D_INVALID_ID;
   }

   return uaViewId;
}


/**
 * Destroy any pending unused uav
 */
void
svga_destroy_uav(struct svga_context *svga)
{
   unsigned index = 0;

   SVGA_DBG(DEBUG_UAV, "%s: ", __func__);

   while ((index = util_bitmask_get_next_index(svga->uav_to_free_id_bm, index))
          != UTIL_BITMASK_INVALID_INDEX) {
      SVGA_DBG(DEBUG_UAV, "%d ", index);

      SVGA_RETRY(svga, SVGA3D_sm5_DestroyUAView(svga->swc, index));
      util_bitmask_clear(svga->uav_id_bm, index);
      util_bitmask_clear(svga->uav_to_free_id_bm, index);
   }

   SVGA_DBG(DEBUG_UAV, "\n");
}


/**
 * Rebind ua views.
 * This function is called at the beginning of each new command buffer to make sure
 * the resources associated with the ua views are properly paged-in.
 */
enum pipe_error
svga_rebind_uav(struct svga_context *svga)
{
   struct svga_winsys_context *swc = svga->swc;
   struct svga_hw_draw_state *hw = &svga->state.hw_draw;
   enum pipe_error ret;

   assert(svga_have_sm5(svga));

   for (unsigned i = 0; i < hw->num_uavs; i++) {
      if (hw->uaViews[i]) {
         ret = swc->resource_rebind(swc, hw->uaViews[i], NULL,
                                    SVGA_RELOC_READ | SVGA_RELOC_WRITE);
         if (ret != PIPE_OK)
            return ret;
      }
   }
   svga->rebind.flags.uav = 0;

   return PIPE_OK;
}

static int
svga_find_uav_from_list(struct svga_context *svga, SVGA3dUAViewId uaViewId,
                        unsigned num_uavs, SVGA3dUAViewId *uaViewsId)
{
   for (unsigned i = 0; i < num_uavs; i++) {
      if (uaViewsId[i] == uaViewId)
         return i;
   }
   return -1;
}

/**
 * A helper function to create the uaView lists from the
 * bound shader images and shader buffers.
 */
static enum pipe_error
svga_create_uav_list(struct svga_context *svga,
                     enum svga_pipe_type pipe_type,
                     unsigned num_free_uavs,
                     unsigned *num_uavs,
                     SVGA3dUAViewId *uaViewIds,
                     struct svga_winsys_surface **uaViews)
{
   enum pipe_shader_type first_shader, last_shader;
   struct svga_uav *uav;
   int uav_index = -1;

   /* Increase uav timestamp */
   svga->state.uav_timestamp[pipe_type]++;

   if (pipe_type == SVGA_PIPE_GRAPHICS) {
      first_shader = PIPE_SHADER_VERTEX;
      last_shader = PIPE_SHADER_COMPUTE;
   } else {
      first_shader = PIPE_SHADER_COMPUTE;
      last_shader = first_shader + 1;
   }

   for (enum pipe_shader_type shader = first_shader;
        shader < last_shader; shader++) {

      unsigned num_image_views = svga->curr.num_image_views[shader];
      unsigned num_shader_buffers = svga->curr.num_shader_buffers[shader];

      SVGA_DBG(DEBUG_UAV,
            "%s: shader=%d num_image_views=%d num_shader_buffers=%d\n",
            __func__, shader, num_image_views, num_shader_buffers);

      /* add enabled shader images to the uav list */
      if (num_image_views) {
         num_image_views = MIN2(num_image_views, num_free_uavs-*num_uavs);
         for (unsigned i = 0; i < num_image_views; i++) {
            struct svga_image_view *cur_image_view =
                &svga->curr.image_views[shader][i];
            struct pipe_resource *res = cur_image_view->resource;
            SVGA3dUAViewId uaViewId;

            if (res) {

               /* First check if there is already a uav defined for this
                * image view.
                */
               uav = svga_uav_cache_find_uav(svga, SVGA_IMAGE_VIEW,
                                             cur_image_view,
                                             sizeof(*cur_image_view));

               /* If there isn't one, create a uav for this image view. */
               if (uav == NULL) {
                  uaViewId = svga_create_uav_image(svga, &cur_image_view->desc);
                  if (uaViewId == SVGA3D_INVALID_ID)
                     return PIPE_ERROR_OUT_OF_MEMORY;

                  /* Add the uav to the cache */
                  uav = svga_uav_cache_add_uav(svga, SVGA_IMAGE_VIEW,
                                               cur_image_view,
                                               sizeof(*cur_image_view),
                                               res,
                                               uaViewId);
                  if (uav == NULL)
                     return PIPE_ERROR_OUT_OF_MEMORY;
               }

               /* Mark this uav as being used */
               svga_uav_cache_use_uav(svga, pipe_type, uav);

               /* Check if the uav is already bound in the uav list */
               uav_index = svga_find_uav_from_list(svga, uav->uaViewId,
                                                   *num_uavs, uaViewIds);

               /* The uav is not already on the uaView list, add it */
               if (uav_index == -1) {
                  uav_index = *num_uavs;
                  (*num_uavs)++;
                  if (res->target == PIPE_BUFFER)
                     uaViews[uav_index] = svga_buffer(res)->handle;
                  else
                     uaViews[uav_index] = svga_texture(res)->handle;

                  uaViewIds[uav_index] = uav->uaViewId;
               }

               /* Save the uav slot index for the image view for later reference
                * to create the uav mapping in the shader key.
                */
               cur_image_view->uav_index = uav_index;
            }
         }
      }

      /* add enabled shader buffers to the uav list */
      if (num_shader_buffers) {
         num_shader_buffers = MIN2(num_shader_buffers, num_free_uavs-*num_uavs);
         for (unsigned i = 0; i < num_shader_buffers; i++) {
            struct svga_shader_buffer *cur_sbuf =
                &svga->curr.shader_buffers[shader][i];
            struct pipe_resource *res = cur_sbuf->resource;
            SVGA3dUAViewId uaViewId;
	    enum pipe_error ret;

            /* Use srv rawbuffer to access readonly shader buffer */
	    if (svga_shader_buffer_can_use_srv(svga, shader, i, cur_sbuf)) {
               ret = svga_shader_buffer_bind_srv(svga, shader, i, cur_sbuf);
               if (ret != PIPE_OK)
                  return ret;
               continue;
	    } else {
               ret = svga_shader_buffer_unbind_srv(svga, shader, i, cur_sbuf);
               if (ret != PIPE_OK)
                  return ret;
            }

            if (res) {
               /* Get the buffer handle that can be bound as uav. */
               cur_sbuf->handle = svga_buffer_handle(svga, res,
                                                    PIPE_BIND_SHADER_BUFFER);

               /* First check if there is already a uav defined for this
                * shader buffer.
                */
               uav = svga_uav_cache_find_uav(svga, SVGA_SHADER_BUFFER,
                                             cur_sbuf,
                                             sizeof(*cur_sbuf));

               /* If there isn't one, create a uav for this shader buffer. */
               if (uav == NULL) {
                  uaViewId = svga_create_uav_buffer(svga, &cur_sbuf->desc,
                                                    SVGA3D_R32_TYPELESS,
                                                    SVGA3D_UABUFFER_RAW);

                  if (uaViewId == SVGA3D_INVALID_ID)
                     return PIPE_ERROR_OUT_OF_MEMORY;

                  /* Add the uav to the cache */
                  uav = svga_uav_cache_add_uav(svga, SVGA_SHADER_BUFFER,
                                               cur_sbuf,
                                               sizeof(*cur_sbuf),
                                               res,
                                               uaViewId);
                  if (uav == NULL)
                     return PIPE_ERROR_OUT_OF_MEMORY;
               }

               /* Mark this uav as being used */
               svga_uav_cache_use_uav(svga, pipe_type, uav);

               uav_index = svga_find_uav_from_list(svga, uav->uaViewId,
                                                   *num_uavs, uaViewIds);

               /* The uav is not already on the uaView list, add it */
               if (uav_index == -1) {
                  uav_index = *num_uavs;
                  (*num_uavs)++;
                  uaViews[uav_index] = svga_buffer(res)->handle;
                  uaViewIds[uav_index] = uav->uaViewId;
               }

               /* Save the uav slot index for later reference
                * to create the uav mapping in the shader key.
                */
               cur_sbuf->uav_index = uav_index;
            }
         }
      }
   }

   /* Since atomic buffers are not specific to a particular shader type,
    * add any enabled atomic buffers to the uav list when we are done adding
    * shader specific uavs.
    */

   unsigned num_atomic_buffers = svga->curr.num_atomic_buffers;

   SVGA_DBG(DEBUG_UAV,
            "%s: num_atomic_buffers=%d\n", __func__, num_atomic_buffers);

   if (num_atomic_buffers) {
      num_atomic_buffers = MIN2(num_atomic_buffers, num_free_uavs-*num_uavs);

      for (unsigned i = 0; i < num_atomic_buffers; i++) {
         struct svga_shader_buffer *cur_sbuf = &svga->curr.atomic_buffers[i];
         struct pipe_resource *res = cur_sbuf->resource;
         SVGA3dUAViewId uaViewId;

         if (res) {
            /* Get the buffer handle that can be bound as uav. */
            cur_sbuf->handle = svga_buffer_handle(svga, res,
                                                  PIPE_BIND_SHADER_BUFFER);

            /* First check if there is already a uav defined for this
             * shader buffer.
             */
            uav = svga_uav_cache_find_uav(svga, SVGA_SHADER_BUFFER,
                                          cur_sbuf,
                                          sizeof(*cur_sbuf));

            /* If there isn't one, create a uav for this shader buffer. */
            if (uav == NULL) {
               uaViewId = svga_create_uav_buffer(svga, &cur_sbuf->desc,
                                                 SVGA3D_R32_TYPELESS,
                                                 SVGA3D_UABUFFER_RAW);

               if (uaViewId == SVGA3D_INVALID_ID)
                  return PIPE_ERROR_OUT_OF_MEMORY;

               /* Add the uav to the cache */
               uav = svga_uav_cache_add_uav(svga, SVGA_SHADER_BUFFER,
                                            cur_sbuf,
                                            sizeof(*cur_sbuf),
                                            res,
                                            uaViewId);
               if (uav == NULL)
                  return PIPE_ERROR_OUT_OF_MEMORY;
            }

            /* Mark this uav as being used */
            svga_uav_cache_use_uav(svga, pipe_type, uav);

            uav_index = svga_find_uav_from_list(svga, uav->uaViewId,
                                                *num_uavs, uaViewIds);

            /* The uav is not already on the uaView list, add it */
            if (uav_index == -1) {
               uav_index = *num_uavs;
               (*num_uavs)++;
               uaViews[uav_index] = svga_buffer(res)->handle;
               uaViewIds[uav_index] = uav->uaViewId;
            }
         }

         /* Save the uav slot index for the atomic buffer for later reference
          * to create the uav mapping in the shader key.
          */
         cur_sbuf->uav_index = uav_index;
      }
   }

   /* Reset the rest of the ua views list */
   for (unsigned u = *num_uavs;
        u < ARRAY_SIZE(svga->state.hw_draw.uaViewIds); u++) {
      uaViewIds[u] = SVGA3D_INVALID_ID;
      uaViews[u] = NULL;
   }

   return PIPE_OK;
}


/**
 * A helper function to save the current hw uav state.
 */
static void
svga_save_uav_state(struct svga_context *svga,
                    enum svga_pipe_type pipe_type,
                    unsigned num_uavs,
                    SVGA3dUAViewId *uaViewIds,
                    struct svga_winsys_surface **uaViews)
{
   enum pipe_shader_type first_shader, last_shader;
   unsigned i;

   if (pipe_type == SVGA_PIPE_GRAPHICS) {
      first_shader = PIPE_SHADER_VERTEX;
      last_shader = PIPE_SHADER_COMPUTE;
   } else {
      first_shader = PIPE_SHADER_COMPUTE;
      last_shader = first_shader + 1;
   }

   for (enum pipe_shader_type shader = first_shader;
        shader < last_shader; shader++) {

      /**
       * Save the current shader images
       */
      for (i = 0; i < ARRAY_SIZE(svga->curr.image_views[0]); i++) {
         struct svga_image_view *cur_image_view =
            &svga->curr.image_views[shader][i];
         struct svga_image_view *hw_image_view =
            &svga->state.hw_draw.image_views[shader][i];

         /* Save the hw state for image view */
         *hw_image_view = *cur_image_view;
      }

      /**
       * Save the current shader buffers
       */
      for (i = 0; i < ARRAY_SIZE(svga->curr.shader_buffers[0]); i++) {
         struct svga_shader_buffer *cur_shader_buffer =
            &svga->curr.shader_buffers[shader][i];
         struct svga_shader_buffer *hw_shader_buffer =
            &svga->state.hw_draw.shader_buffers[shader][i];

         /* Save the hw state for image view */
         *hw_shader_buffer = *cur_shader_buffer;
      }

      svga->state.hw_draw.num_image_views[shader] =
         svga->curr.num_image_views[shader];
      svga->state.hw_draw.num_shader_buffers[shader] =
         svga->curr.num_shader_buffers[shader];
   }

   /**
    * Save the current atomic buffers
    */
   for (i = 0; i < ARRAY_SIZE(svga->curr.atomic_buffers); i++) {
      struct svga_shader_buffer *cur_buf = &svga->curr.atomic_buffers[i];
      struct svga_shader_buffer *hw_buf = &svga->state.hw_draw.atomic_buffers[i];

      /* Save the hw state for atomic buffers */
      *hw_buf = *cur_buf;
   }

   svga->state.hw_draw.num_atomic_buffers = svga->curr.num_atomic_buffers;

   /**
    * Save the hw state for uaviews
    */
   if (pipe_type == SVGA_PIPE_COMPUTE) {
      svga->state.hw_draw.num_cs_uavs = num_uavs;
      memcpy(svga->state.hw_draw.csUAViewIds, uaViewIds,
             sizeof svga->state.hw_draw.csUAViewIds);
      memcpy(svga->state.hw_draw.csUAViews, uaViews,
             sizeof svga->state.hw_draw.csUAViews);
   }
   else {
      svga->state.hw_draw.num_uavs = num_uavs;
      memcpy(svga->state.hw_draw.uaViewIds, uaViewIds,
             sizeof svga->state.hw_draw.uaViewIds);
      memcpy(svga->state.hw_draw.uaViews, uaViews,
             sizeof svga->state.hw_draw.uaViews);
   }

   /* purge the uav cache */
   svga_uav_cache_purge(svga, pipe_type);
}


/**
 * A helper function to determine if we need to resend the SetUAViews command.
 * We need to resend the SetUAViews command when uavSpliceIndex is to
 * be changed because the existing index overlaps with render target views, or
 * the image views/shader buffers are changed.
 */
static bool
need_to_set_uav(struct svga_context *svga,
                int uavSpliceIndex,
                unsigned num_uavs,
                SVGA3dUAViewId *uaViewIds,
                struct svga_winsys_surface **uaViews)
{
   /* If number of render target views changed */
   if (uavSpliceIndex != svga->state.hw_draw.uavSpliceIndex)
      return true;

   /* If number of render target views + number of ua views exceeds
    * the max uav count, we will need to trim the ua views.
    */
   if ((uavSpliceIndex + num_uavs) > SVGA_MAX_UAVIEWS)
      return true;

   /* If uavs are different */
   if (memcmp(svga->state.hw_draw.uaViewIds, uaViewIds,
              sizeof svga->state.hw_draw.uaViewIds) ||
       memcmp(svga->state.hw_draw.uaViews, uaViews,
              sizeof svga->state.hw_draw.uaViews))
      return true;

   /* If image views are different */
   for (enum pipe_shader_type shader = PIPE_SHADER_VERTEX;
        shader < PIPE_SHADER_COMPUTE; shader++) {
      unsigned num_image_views = svga->curr.num_image_views[shader];
      if ((num_image_views != svga->state.hw_draw.num_image_views[shader]) ||
          memcmp(svga->state.hw_draw.image_views[shader],
                 svga->curr.image_views[shader],
                 num_image_views * sizeof(struct svga_image_view)))
         return true;

      /* If shader buffers are different */
      unsigned num_shader_buffers = svga->curr.num_shader_buffers[shader];
      if ((num_shader_buffers != svga->state.hw_draw.num_shader_buffers[shader]) ||
          memcmp(svga->state.hw_draw.shader_buffers[shader],
                 svga->curr.shader_buffers[shader],
                 num_shader_buffers * sizeof(struct svga_shader_buffer)))
         return true;
   }

   /* If atomic buffers are different */
   unsigned num_atomic_buffers = svga->curr.num_atomic_buffers;
   if ((num_atomic_buffers != svga->state.hw_draw.num_atomic_buffers) ||
       memcmp(svga->state.hw_draw.atomic_buffers, svga->curr.atomic_buffers,
              num_atomic_buffers * sizeof(struct svga_shader_buffer)))
      return true;

   return false;
}


/**
 * Update ua views in the HW for the draw pipeline by sending the
 * SetUAViews command.
 */
static enum pipe_error
update_uav(struct svga_context *svga, uint64_t dirty)
{
   enum pipe_error ret = PIPE_OK;
   unsigned num_uavs = 0;
   SVGA3dUAViewId uaViewIds[SVGA_MAX_UAVIEWS];
   struct svga_winsys_surface *uaViews[SVGA_MAX_UAVIEWS];

   /* Determine the uavSpliceIndex since uav and render targets view share the
    * same bind points.
    */
   int uavSpliceIndex = svga->state.hw_clear.num_rendertargets;

   /* Number of free uav entries available for shader images and buffers */
   unsigned num_free_uavs = SVGA_MAX_UAVIEWS - uavSpliceIndex;

   SVGA_STATS_TIME_PUSH(svga_sws(svga), SVGA_STATS_TIME_UPDATEUAV);

   /* Create the uav list for graphics pipeline */
   ret = svga_create_uav_list(svga, SVGA_PIPE_GRAPHICS, num_free_uavs,
                              &num_uavs, uaViewIds, uaViews);
   if (ret != PIPE_OK)
      goto done;

   /* check to see if we need to resend the SetUAViews command */
   if (!need_to_set_uav(svga, uavSpliceIndex, num_uavs, uaViewIds, uaViews))
      goto done;

   /* Send the SetUAViews command */
   SVGA_DBG(DEBUG_UAV, "%s: SetUAViews uavSpliceIndex=%d", __func__,
            uavSpliceIndex);

#ifdef DEBUG
   for (unsigned i = 0; i < ARRAY_SIZE(uaViewIds); i++) {
      SVGA_DBG(DEBUG_UAV, " %d ", uaViewIds[i]);
   }
   SVGA_DBG(DEBUG_UAV, "\n");
#endif

   ret = SVGA3D_sm5_SetUAViews(svga->swc, uavSpliceIndex, SVGA_MAX_UAVIEWS,
                               uaViewIds, uaViews);
   if (ret != PIPE_OK)
      goto done;

   /* Save the uav hw state */
   svga_save_uav_state(svga, SVGA_PIPE_GRAPHICS, num_uavs, uaViewIds, uaViews);

   /* Save the uavSpliceIndex as this determines the starting register index
    * for the first uav used in the shader
    */
   svga->state.hw_draw.uavSpliceIndex = uavSpliceIndex;

done:
   SVGA_STATS_TIME_POP(svga_sws(svga));
   return ret;
}


struct svga_tracked_state svga_hw_uav = {
   "shader image view",
   (SVGA_NEW_IMAGE_VIEW |
    SVGA_NEW_SHADER_BUFFER |
    SVGA_NEW_FRAME_BUFFER),
   update_uav
};


/**
 * A helper function to determine if we need to resend the SetCSUAViews command.
 */
static bool
need_to_set_cs_uav(struct svga_context *svga,
                   unsigned num_uavs,
                   SVGA3dUAViewId *uaViewIds,
                   struct svga_winsys_surface **uaViews)
{
   enum pipe_shader_type shader = PIPE_SHADER_COMPUTE;

   if (svga->state.hw_draw.num_cs_uavs != num_uavs)
      return true;

   /* If uavs are different */
   if (memcmp(svga->state.hw_draw.csUAViewIds, uaViewIds,
              sizeof svga->state.hw_draw.csUAViewIds) ||
       memcmp(svga->state.hw_draw.csUAViews, uaViews,
              sizeof svga->state.hw_draw.csUAViews))
      return true;

   /* If image views are different */
   unsigned num_image_views = svga->curr.num_image_views[shader];
   if ((num_image_views != svga->state.hw_draw.num_image_views[shader]) ||
       memcmp(svga->state.hw_draw.image_views[shader],
              svga->curr.image_views[shader],
              num_image_views * sizeof(struct svga_image_view)))
      return true;

   /* If atomic buffers are different */
   unsigned num_atomic_buffers = svga->curr.num_atomic_buffers;
   if ((num_atomic_buffers != svga->state.hw_draw.num_atomic_buffers) ||
       memcmp(svga->state.hw_draw.atomic_buffers, svga->curr.atomic_buffers,
              num_atomic_buffers * sizeof(struct svga_shader_buffer)))
      return true;

   return false;
}


/**
 * Update ua views in the HW for the compute pipeline by sending the
 * SetCSUAViews command.
 */
static enum pipe_error
update_cs_uav(struct svga_context *svga, uint64_t dirty)
{
   enum pipe_error ret = PIPE_OK;
   unsigned num_uavs = 0;
   SVGA3dUAViewId uaViewIds[SVGA_MAX_UAVIEWS];
   struct svga_winsys_surface *uaViews[SVGA_MAX_UAVIEWS];

   /* Number of free uav entries available for shader images and buffers */
   unsigned num_free_uavs = SVGA_MAX_UAVIEWS;

   SVGA_STATS_TIME_PUSH(svga_sws(svga), SVGA_STATS_TIME_UPDATECSUAV);

   /* Create the uav list */
   ret = svga_create_uav_list(svga, SVGA_PIPE_COMPUTE, num_free_uavs,
                              &num_uavs, uaViewIds, uaViews);
   if (ret != PIPE_OK)
      goto done;

   /* Check to see if we need to resend the CSSetUAViews command */
   if (!need_to_set_cs_uav(svga, num_uavs, uaViewIds, uaViews))
      goto done;

   /* Send the uaviews to compute */

   SVGA_DBG(DEBUG_UAV, "%s: SetCSUAViews", __func__);

#ifdef DEBUG
   for (unsigned i = 0; i < ARRAY_SIZE(uaViewIds); i++) {
      SVGA_DBG(DEBUG_UAV, " %d ", uaViewIds[i]);
   }
   SVGA_DBG(DEBUG_UAV, "\n");
#endif

   ret = SVGA3D_sm5_SetCSUAViews(svga->swc, SVGA_MAX_UAVIEWS,
                                 uaViewIds, uaViews);
   if (ret != PIPE_OK)
      goto done;

   /* Save the uav hw state */
   svga_save_uav_state(svga, SVGA_PIPE_COMPUTE, num_uavs, uaViewIds, uaViews);

done:
   SVGA_STATS_TIME_POP(svga_sws(svga));
   return ret;
}


struct svga_tracked_state svga_hw_cs_uav = {
   "shader image view",
   (SVGA_NEW_IMAGE_VIEW |
    SVGA_NEW_SHADER_BUFFER |
    SVGA_NEW_FRAME_BUFFER),
   update_cs_uav
};
