/*
 * Copyright © 2014 Broadcom
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/** @file v3d_fence.c
 *
 * Seqno-based fence management.
 *
 * We have two mechanisms for waiting in our kernel API: You can wait on a BO
 * to have all rendering to from any process to be completed, or wait on a
 * seqno for that particular seqno to be passed.  The fence API we're
 * implementing is based on waiting for all rendering in the context to have
 * completed (with no reference to what other processes might be doing with
 * the same BOs), so we can just use the seqno of the last rendering we'd
 * fired off as our fence marker.
 */
#include <fcntl.h>
#include <libsync.h>

#include "util/u_inlines.h"
#include "util/os_time.h"

#include "v3d_context.h"
#include "v3d_bufmgr.h"

struct v3d_fence {
        struct pipe_reference reference;
        int fd;
};

static void
v3d_fence_reference(struct pipe_screen *pscreen,
                    struct pipe_fence_handle **pp,
                    struct pipe_fence_handle *pf)
{
        struct v3d_fence **p = (struct v3d_fence **)pp;
        struct v3d_fence *f = (struct v3d_fence *)pf;
        struct v3d_fence *old = *p;

        if (pipe_reference(&(*p)->reference, &f->reference)) {
                close(old->fd);
                free(old);
        }
        *p = f;
}

void
v3d_fence_unreference(struct v3d_fence **fence)
{
        assert(fence);

        if (!*fence)
                return;

        v3d_fence_reference(NULL, (struct pipe_fence_handle **)fence, NULL);
}

bool
v3d_fence_wait(struct v3d_screen *screen,
               struct v3d_fence *fence,
               uint64_t timeout_ns)
{
        int ret;
        unsigned syncobj;

        ret = drmSyncobjCreate(screen->fd, 0, &syncobj);
        if (ret) {
                fprintf(stderr, "Failed to create syncobj to wait on: %d\n",
                        ret);
                return false;
        }

        ret = drmSyncobjImportSyncFile(screen->fd, syncobj, fence->fd);
        if (ret) {
                fprintf(stderr, "Failed to import fence to syncobj: %d\n", ret);
                return false;
        }

        uint64_t abs_timeout = os_time_get_absolute_timeout(timeout_ns);
        if (abs_timeout == OS_TIMEOUT_INFINITE)
                abs_timeout = INT64_MAX;

        ret = drmSyncobjWait(screen->fd, &syncobj, 1, abs_timeout, 0, NULL);

        drmSyncobjDestroy(screen->fd, syncobj);

        return ret >= 0;
}

static bool
v3d_fence_finish(struct pipe_screen *pscreen,
		 struct pipe_context *ctx,
                 struct pipe_fence_handle *pf,
                 uint64_t timeout_ns)
{
        struct v3d_screen *screen = v3d_screen(pscreen);
        struct v3d_fence *fence = (struct v3d_fence *)pf;

        return v3d_fence_wait(screen, fence, timeout_ns);
}

struct v3d_fence *
v3d_fence_create(struct v3d_context *v3d, int fd)
{
        struct v3d_fence *f = calloc(1, sizeof(*f));
        if (!f)
                return NULL;

        f->fd = fd;
        pipe_reference_init(&f->reference, 1);

        return f;
}

static void
v3d_fence_create_fd(struct pipe_context *pctx, struct pipe_fence_handle **pf,
                    int fd, enum pipe_fd_type type)
{
        struct v3d_context *v3d = (struct v3d_context *)pctx;
        struct v3d_fence **fence = (struct v3d_fence **)pf;

        assert(type == PIPE_FD_TYPE_NATIVE_SYNC);
        *fence = v3d_fence_create(v3d, fcntl(fd, F_DUPFD_CLOEXEC, 3));
}

static void
v3d_fence_server_sync(struct pipe_context *pctx,
                      struct pipe_fence_handle *pfence)
{
        struct v3d_context *v3d = (struct v3d_context*)pctx;
        struct v3d_fence *fence = (struct v3d_fence *)pfence;

        sync_accumulate("v3d", &v3d->in_fence_fd, fence->fd);
}

static int
v3d_fence_get_fd(struct pipe_screen *screen, struct pipe_fence_handle *pfence)
{
        struct v3d_fence *fence = (struct v3d_fence *) pfence;
        return fcntl(fence->fd, F_DUPFD_CLOEXEC, 3);
}

int
v3d_fence_context_init(struct v3d_context *v3d)
{
        v3d->base.create_fence_fd = v3d_fence_create_fd;
        v3d->base.fence_server_sync = v3d_fence_server_sync;
        v3d->in_fence_fd = -1;

        /* Since we initialize the in_fence_fd to -1 (no wait necessary),
         * we also need to initialize our in_syncobj as signaled.
         */
        return drmSyncobjCreate(v3d->fd, DRM_SYNCOBJ_CREATE_SIGNALED,
                                &v3d->in_syncobj);
}

void
v3d_fence_context_finish(struct v3d_context *v3d)
{
        drmSyncobjDestroy(v3d->fd, v3d->in_syncobj);
        if (v3d->in_fence_fd >= 0) {
                close(v3d->in_fence_fd);
                v3d->in_fence_fd = -1;
        }
}

void
v3d_fence_screen_init(struct v3d_screen *screen)
{
        screen->base.fence_reference = v3d_fence_reference;
        screen->base.fence_finish = v3d_fence_finish;
        screen->base.fence_get_fd = v3d_fence_get_fd;
}
