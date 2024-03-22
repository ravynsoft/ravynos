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

#ifndef PVR_SRV_SYNC_H
#define PVR_SRV_SYNC_H

#include <stdbool.h>
#include <stddef.h>

#include "pvr_winsys.h"
#include "util/macros.h"
#include "vk_sync.h"

struct vk_device;

struct pvr_srv_sync {
   struct vk_sync base;

   /* Cached version of completion. */
   bool signaled;

   int fd;
};

extern const struct vk_sync_type pvr_srv_sync_type;

void pvr_srv_sync_finish(struct vk_device *device, struct vk_sync *sync);
void pvr_srv_set_sync_payload(struct pvr_srv_sync *srv_sync, int payload);

static inline bool pvr_sync_type_is_srv_sync(const struct vk_sync_type *type)
{
   return type->finish == pvr_srv_sync_finish;
}

static inline struct pvr_srv_sync *to_srv_sync(struct vk_sync *sync)
{
   assert(!sync || pvr_sync_type_is_srv_sync(sync->type));
   return container_of(sync, struct pvr_srv_sync, base);
}

#endif /* PVR_SRV_SYNC_H */
