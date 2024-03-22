/*
 * Copyright © 2022 Imagination Technologies Ltd.
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

#include <stdbool.h>
#include <stdint.h>

#include "pvr_bo.h"
#include "pvr_dump_bo.h"
#include "pvr_dump.h"
#include "pvr_winsys.h"
#include "util/u_math.h"

struct pvr_device;

bool pvr_dump_bo_ctx_push(struct pvr_dump_bo_ctx *const ctx,
                          struct pvr_dump_ctx *const parent_ctx,
                          struct pvr_device *const device,
                          struct pvr_bo *const bo)
{
   bool did_map_bo = false;

   if (!bo->bo->map) {
      if (pvr_bo_cpu_map_unchanged(device, bo) != VK_SUCCESS)
         goto err_out;

      did_map_bo = true;
   }

   if (bo->bo->size > UINT32_MAX) {
      mesa_logw_once("Attempted to dump a BO larger than 4GiB; time to rework"
                     "pvr_dump_buffer_ctx to use 64-bit sizes.");
      goto err_unmap_bo;
   }

   if (!pvr_dump_buffer_ctx_push(&ctx->base,
                                 parent_ctx,
                                 bo->bo->map,
                                 bo->bo->size)) {
      goto err_unmap_bo;
   }

   ctx->device = device;
   ctx->bo = bo;
   ctx->bo_mapped_in_ctx = did_map_bo;

   return true;

err_unmap_bo:
   if (did_map_bo)
      pvr_bo_cpu_unmap(device, bo);

err_out:
   return false;
}

bool pvr_dump_bo_ctx_pop(struct pvr_dump_bo_ctx *const ctx)
{
   if (ctx->bo_mapped_in_ctx)
      pvr_bo_cpu_unmap(ctx->device, ctx->bo);

   return pvr_dump_buffer_ctx_pop(&ctx->base);
}
