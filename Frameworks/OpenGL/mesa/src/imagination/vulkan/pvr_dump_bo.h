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

#ifndef PVR_DUMP_BO_H
#define PVR_DUMP_BO_H

#include <stdbool.h>

#include "pvr_dump.h"

struct pvr_bo;
struct pvr_device;

struct pvr_dump_bo_ctx {
   struct pvr_dump_buffer_ctx base;

   struct pvr_device *device;
   struct pvr_bo *bo;
   bool bo_mapped_in_ctx;

   /* No user-modifiable values */
};

bool pvr_dump_bo_ctx_push(struct pvr_dump_bo_ctx *ctx,
                          struct pvr_dump_ctx *parent_ctx,
                          struct pvr_device *device,
                          struct pvr_bo *bo);
bool pvr_dump_bo_ctx_pop(struct pvr_dump_bo_ctx *ctx);

#endif /* PVR_DUMP_BO_H */
