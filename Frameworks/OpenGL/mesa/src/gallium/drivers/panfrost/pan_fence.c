/*
 * Copyright (c) 2022 Amazon.com, Inc. or its affiliates.
 * Copyright (C) 2008 VMware, Inc.
 * Copyright (C) 2014 Broadcom
 * Copyright (C) 2018 Alyssa Rosenzweig
 * Copyright (C) 2019 Collabora, Ltd.
 * Copyright (C) 2012 Rob Clark <robclark@freedesktop.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "pan_fence.h"
#include "pan_context.h"
#include "pan_screen.h"

#include "util/os_time.h"
#include "util/u_inlines.h"

void
panfrost_fence_reference(struct pipe_screen *pscreen,
                         struct pipe_fence_handle **ptr,
                         struct pipe_fence_handle *fence)
{
   struct panfrost_device *dev = pan_device(pscreen);
   struct pipe_fence_handle *old = *ptr;

   if (pipe_reference(&old->reference, &fence->reference)) {
      drmSyncobjDestroy(panfrost_device_fd(dev), old->syncobj);
      free(old);
   }

   *ptr = fence;
}

bool
panfrost_fence_finish(struct pipe_screen *pscreen, struct pipe_context *ctx,
                      struct pipe_fence_handle *fence, uint64_t timeout)
{
   struct panfrost_device *dev = pan_device(pscreen);
   int ret;

   if (fence->signaled)
      return true;

   uint64_t abs_timeout = os_time_get_absolute_timeout(timeout);
   if (abs_timeout == OS_TIMEOUT_INFINITE)
      abs_timeout = INT64_MAX;

   ret = drmSyncobjWait(panfrost_device_fd(dev), &fence->syncobj, 1,
                        abs_timeout, DRM_SYNCOBJ_WAIT_FLAGS_WAIT_ALL, NULL);

   fence->signaled = (ret >= 0);
   return fence->signaled;
}

int
panfrost_fence_get_fd(struct pipe_screen *screen, struct pipe_fence_handle *f)
{
   struct panfrost_device *dev = pan_device(screen);
   int fd = -1;

   drmSyncobjExportSyncFile(panfrost_device_fd(dev), f->syncobj, &fd);
   return fd;
}

struct pipe_fence_handle *
panfrost_fence_from_fd(struct panfrost_context *ctx, int fd,
                       enum pipe_fd_type type)
{
   struct panfrost_device *dev = pan_device(ctx->base.screen);
   int ret;

   struct pipe_fence_handle *f = calloc(1, sizeof(*f));
   if (!f)
      return NULL;

   if (type == PIPE_FD_TYPE_NATIVE_SYNC) {
      ret = drmSyncobjCreate(panfrost_device_fd(dev), 0, &f->syncobj);
      if (ret) {
         fprintf(stderr, "create syncobj failed\n");
         goto err_free_fence;
      }

      ret = drmSyncobjImportSyncFile(panfrost_device_fd(dev), f->syncobj, fd);
      if (ret) {
         fprintf(stderr, "import syncfile failed\n");
         goto err_destroy_syncobj;
      }
   } else {
      assert(type == PIPE_FD_TYPE_SYNCOBJ);
      ret = drmSyncobjFDToHandle(panfrost_device_fd(dev), fd, &f->syncobj);
      if (ret) {
         fprintf(stderr, "import syncobj FD failed\n");
         goto err_free_fence;
      }
   }

   pipe_reference_init(&f->reference, 1);

   return f;

err_destroy_syncobj:
   drmSyncobjDestroy(panfrost_device_fd(dev), f->syncobj);
err_free_fence:
   free(f);
   return NULL;
}

struct pipe_fence_handle *
panfrost_fence_create(struct panfrost_context *ctx)
{
   struct panfrost_device *dev = pan_device(ctx->base.screen);
   int fd = -1, ret;

   /* Snapshot the last rendering out fence. We'd rather have another
    * syncobj instead of a sync file, but this is all we get.
    * (HandleToFD/FDToHandle just gives you another syncobj ID for the
    * same syncobj).
    */
   ret = drmSyncobjExportSyncFile(panfrost_device_fd(dev), ctx->syncobj, &fd);
   if (ret || fd == -1) {
      fprintf(stderr, "export failed\n");
      return NULL;
   }

   struct pipe_fence_handle *f =
      panfrost_fence_from_fd(ctx, fd, PIPE_FD_TYPE_NATIVE_SYNC);

   close(fd);

   return f;
}
