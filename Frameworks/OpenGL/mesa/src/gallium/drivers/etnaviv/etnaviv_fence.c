/*
 * Copyright (c) 2012-2015 Etnaviv Project
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
 * Authors:
 *    Wladimir J. van der Laan <laanwj@gmail.com>
 *    Rob Clark <robclark@freedesktop.org>
 */

#include <libsync.h>

#include "etnaviv_fence.h"
#include "etnaviv_context.h"
#include "etnaviv_screen.h"

#include "util/os_file.h"
#include "util/u_inlines.h"
#include "util/u_memory.h"

struct pipe_fence_handle {
   struct pipe_reference reference;
   struct etna_screen *screen;
   int fence_fd;
   uint32_t timestamp;
};

static void
etna_fence_destroy(struct pipe_fence_handle *fence)
{
   if (fence->fence_fd != -1)
      close(fence->fence_fd);
   FREE(fence);
}

static void
etna_screen_fence_reference(struct pipe_screen *pscreen,
                            struct pipe_fence_handle **ptr,
                            struct pipe_fence_handle *fence)
{
   if (pipe_reference(&(*ptr)->reference, &fence->reference))
      etna_fence_destroy(*ptr);

   *ptr = fence;
}

static bool
etna_screen_fence_finish(struct pipe_screen *pscreen, struct pipe_context *ctx,
                         struct pipe_fence_handle *fence, uint64_t timeout)
{
   if (fence->fence_fd != -1)
	return !sync_wait(fence->fence_fd, timeout / 1000000);

   if (etna_pipe_wait_ns(fence->screen->pipe, fence->timestamp, timeout))
      return false;

   return true;
}

void
etna_create_fence_fd(struct pipe_context *pctx,
                     struct pipe_fence_handle **pfence, int fd,
                     enum pipe_fd_type type)
{
   assert(type == PIPE_FD_TYPE_NATIVE_SYNC);
   *pfence = etna_fence_create(pctx, os_dupfd_cloexec(fd));
}

void
etna_fence_server_sync(struct pipe_context *pctx,
                       struct pipe_fence_handle *pfence)
{
   struct etna_context *ctx = etna_context(pctx);

   if (pfence->fence_fd != -1)
      sync_accumulate("etnaviv", &ctx->in_fence_fd, pfence->fence_fd);
}

static int
etna_screen_fence_get_fd(struct pipe_screen *pscreen,
                         struct pipe_fence_handle *pfence)
{
   return os_dupfd_cloexec(pfence->fence_fd);
}

struct pipe_fence_handle *
etna_fence_create(struct pipe_context *pctx, int fence_fd)
{
   struct pipe_fence_handle *fence;
   struct etna_context *ctx = etna_context(pctx);

   fence = CALLOC_STRUCT(pipe_fence_handle);
   if (!fence)
      return NULL;

   pipe_reference_init(&fence->reference, 1);

   fence->screen = ctx->screen;
   fence->timestamp = etna_cmd_stream_timestamp(ctx->stream);
   fence->fence_fd = fence_fd;

   return fence;
}

void
etna_fence_screen_init(struct pipe_screen *pscreen)
{
   pscreen->fence_reference = etna_screen_fence_reference;
   pscreen->fence_finish = etna_screen_fence_finish;
   pscreen->fence_get_fd = etna_screen_fence_get_fd;
}
