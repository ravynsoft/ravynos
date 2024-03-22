/*
 * Copyright © 2023 Imagination Technologies Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stddef.h>
#include <stdint.h>
#include <vulkan/vulkan_core.h>

#include "pvr_srv.h"
#include "pvr_srv_bridge.h"
#include "pvr_srv_sync_prim.h"
#include "util/log.h"
#include "util/simple_mtx.h"
#include "util/u_atomic.h"
#include "util/u_idalloc.h"
#include "vk_alloc.h"
#include "vk_log.h"

/* Amount of space used to hold sync prim values (in bytes). */
#define PVR_SRV_SYNC_PRIM_VALUE_SIZE 4

VkResult pvr_srv_sync_prim_block_init(struct pvr_srv_winsys *srv_ws)
{
   /* We don't currently make use of this value, but we're required to provide
    * a valid pointer to pvr_srv_alloc_sync_primitive_block.
    */
   void *sync_block_pmr;
   uint32_t max_size;
   VkResult result;

   result =
      pvr_srv_alloc_sync_primitive_block(srv_ws->base.render_fd,
                                         &srv_ws->sync_prim_ctx.block_handle,
                                         &sync_block_pmr,
                                         &max_size,
                                         &srv_ws->sync_prim_ctx.block_fw_addr);
   if (result != VK_SUCCESS)
      return result;

   srv_ws->sync_prim_ctx.max_count = max_size / PVR_SRV_SYNC_PRIM_VALUE_SIZE;

   /* TODO: This uses ralloc() should we be using vk_alloc()? */
   util_idalloc_mt_init(&srv_ws->sync_prim_ctx.allocator,
                        srv_ws->sync_prim_ctx.max_count,
                        false);

   return VK_SUCCESS;
}

void pvr_srv_sync_prim_block_finish(struct pvr_srv_winsys *srv_ws)
{
   util_idalloc_mt_fini(&srv_ws->sync_prim_ctx.allocator);
   pvr_srv_free_sync_primitive_block(srv_ws->base.render_fd,
                                     srv_ws->sync_prim_ctx.block_handle);
   srv_ws->sync_prim_ctx.block_handle = NULL;
}

struct pvr_srv_sync_prim *pvr_srv_sync_prim_alloc(struct pvr_srv_winsys *srv_ws)
{
   struct pvr_srv_sync_prim_ctx *ctx = &srv_ws->sync_prim_ctx;
   struct pvr_srv_sync_prim *sync_prim;
   unsigned id;

   sync_prim = vk_alloc(srv_ws->base.alloc,
                        sizeof(*sync_prim),
                        8,
                        VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
   if (!sync_prim) {
      vk_error(NULL, VK_ERROR_OUT_OF_HOST_MEMORY);
      return NULL;
   }

   simple_mtx_lock(&srv_ws->sync_prim_ctx.allocator.mutex);

   id = util_idalloc_alloc(&srv_ws->sync_prim_ctx.allocator.buf);
   if (id >= ctx->max_count) {
      /* FIXME: The last alloc expanded the idalloc. Assuming that the app can
       * recover, we'll end up having memory that we'll never use.
       */

      util_idalloc_free(&srv_ws->sync_prim_ctx.allocator.buf, id);
      simple_mtx_unlock(&srv_ws->sync_prim_ctx.allocator.mutex);
      vk_free(srv_ws->base.alloc, sync_prim);

      vk_errorf(NULL,
                VK_ERROR_OUT_OF_DEVICE_MEMORY,
                "Out of sync prim block space.");

      return NULL;
   }

   simple_mtx_unlock(&srv_ws->sync_prim_ctx.allocator.mutex);

   sync_prim->offset = id * PVR_SRV_SYNC_PRIM_VALUE_SIZE;
   sync_prim->ctx = &srv_ws->sync_prim_ctx;
   sync_prim->value = 0;

   return sync_prim;
}

/* FIXME: Add support for freeing offsets back to the sync block. */
void pvr_srv_sync_prim_free(struct pvr_srv_winsys *srv_ws,
                            struct pvr_srv_sync_prim *sync_prim)
{
   if (sync_prim) {
      const uint32_t id = sync_prim->offset / PVR_SRV_SYNC_PRIM_VALUE_SIZE;
      VkResult result;

      result = pvr_srv_set_sync_primitive(srv_ws->base.render_fd,
                                          srv_ws->sync_prim_ctx.block_handle,
                                          id,
                                          0);
      if (result != VK_SUCCESS) {
         /* Let's keep the id allocated so no one else ends up using it. Using a
          * non zeroed out sync prim will crash things.
          */

         mesa_logw("Failed to free sync prim. "
                   "Some sync prim block space will be lost.");

         vk_free(srv_ws->base.alloc, sync_prim);
         return;
      }

      util_idalloc_mt_free(&srv_ws->sync_prim_ctx.allocator, id);

      vk_free(srv_ws->base.alloc, sync_prim);
   }
}
