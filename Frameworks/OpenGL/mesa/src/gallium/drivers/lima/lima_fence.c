/*
 * Copyright (c) 2018-2019 Lima Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

#include <fcntl.h>
#include <libsync.h>

#include "util/os_file.h"
#include <util/u_memory.h>
#include <util/u_inlines.h>

#include "drm-uapi/lima_drm.h"

#include "lima_screen.h"
#include "lima_context.h"
#include "lima_fence.h"
#include "lima_job.h"

struct pipe_fence_handle {
   struct pipe_reference reference;
   int fd;
};

static void
lima_create_fence_fd(struct pipe_context *pctx,
                     struct pipe_fence_handle **fence,
                     int fd, enum pipe_fd_type type)
{
   assert(type == PIPE_FD_TYPE_NATIVE_SYNC);
   *fence = lima_fence_create(os_dupfd_cloexec(fd));
}

static void
lima_fence_server_sync(struct pipe_context *pctx,
                       struct pipe_fence_handle *fence)
{
   struct lima_context *ctx = lima_context(pctx);

   sync_accumulate("lima", &ctx->in_sync_fd, fence->fd);
}

void lima_fence_context_init(struct lima_context *ctx)
{
   ctx->base.create_fence_fd = lima_create_fence_fd;
   ctx->base.fence_server_sync = lima_fence_server_sync;
}

struct pipe_fence_handle *
lima_fence_create(int fd)
{
   struct pipe_fence_handle *fence;

   fence = CALLOC_STRUCT(pipe_fence_handle);
   if (!fence)
      return NULL;

   pipe_reference_init(&fence->reference, 1);
   fence->fd = fd;

   return fence;
}

static int
lima_fence_get_fd(struct pipe_screen *pscreen,
                  struct pipe_fence_handle *fence)
{
   return os_dupfd_cloexec(fence->fd);
}

static void
lima_fence_destroy(struct pipe_fence_handle *fence)
{
   if (fence->fd >= 0)
      close(fence->fd);
   FREE(fence);
}

static void
lima_fence_reference(struct pipe_screen *pscreen,
                     struct pipe_fence_handle **ptr,
                     struct pipe_fence_handle *fence)
{
   if (pipe_reference(&(*ptr)->reference, &fence->reference))
      lima_fence_destroy(*ptr);
   *ptr = fence;
}

static bool
lima_fence_finish(struct pipe_screen *pscreen, struct pipe_context *pctx,
                  struct pipe_fence_handle *fence, uint64_t timeout)
{
   return !sync_wait(fence->fd, timeout / 1000000);
}

void
lima_fence_screen_init(struct lima_screen *screen)
{
   screen->base.fence_reference = lima_fence_reference;
   screen->base.fence_finish = lima_fence_finish;
   screen->base.fence_get_fd = lima_fence_get_fd;
}
