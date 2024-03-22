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

#ifndef PVR_SRV_SYNC_PRIM_H
#define PVR_SRV_SYNC_PRIM_H

#include <stdint.h>
#include <vulkan/vulkan_core.h>

#include "util/u_idalloc.h"

struct pvr_srv_winsys;

struct pvr_srv_sync_prim_ctx {
   /* Sync block used for allocating sync primitives. */
   void *block_handle;
   uint32_t block_fw_addr;

   struct util_idalloc_mt allocator;
   /* Top limit for how many sync prims can be allocated from the block. */
   uint32_t max_count;
};

struct pvr_srv_sync_prim {
   struct pvr_srv_sync_prim_ctx *ctx;
   uint32_t offset;
   uint32_t value;
};

VkResult pvr_srv_sync_prim_block_init(struct pvr_srv_winsys *srv_ws);
void pvr_srv_sync_prim_block_finish(struct pvr_srv_winsys *srv_ws);

struct pvr_srv_sync_prim *
pvr_srv_sync_prim_alloc(struct pvr_srv_winsys *srv_ws);
void pvr_srv_sync_prim_free(struct pvr_srv_winsys *srv_ws,
                            struct pvr_srv_sync_prim *sync_prim);

static inline uint32_t
pvr_srv_sync_prim_get_fw_addr(const struct pvr_srv_sync_prim *const sync_prim)
{
   return sync_prim->ctx->block_fw_addr + sync_prim->offset;
}

#endif /* PVR_SRV_SYNC_PRIM_H */
