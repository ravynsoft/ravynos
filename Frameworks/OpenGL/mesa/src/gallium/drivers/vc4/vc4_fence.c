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

/** @file vc4_fence.c
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

#include <libsync.h>
#include <fcntl.h>

#include "util/os_file.h"
#include "util/u_inlines.h"

#include "vc4_screen.h"
#include "vc4_context.h"
#include "vc4_bufmgr.h"

struct vc4_fence {
        struct pipe_reference reference;
        uint64_t seqno;
        int fd;
};

static inline struct vc4_fence *
vc4_fence(struct pipe_fence_handle *pfence)
{
        return (struct vc4_fence *)pfence;
}

static void
vc4_fence_reference(struct pipe_screen *pscreen,
                    struct pipe_fence_handle **pp,
                    struct pipe_fence_handle *pf)
{
        struct vc4_fence **p = (struct vc4_fence **)pp;
        struct vc4_fence *f = vc4_fence(pf);
        struct vc4_fence *old = *p;

        if (pipe_reference(&(*p)->reference, &f->reference)) {
                if (old->fd >= 0)
                        close(old->fd);
                free(old);
        }
        *p = f;
}

static bool
vc4_fence_finish(struct pipe_screen *pscreen,
		 struct pipe_context *ctx,
                 struct pipe_fence_handle *pf,
                 uint64_t timeout_ns)
{
        struct vc4_screen *screen = vc4_screen(pscreen);
        struct vc4_fence *f = vc4_fence(pf);

        if (f->fd >= 0)
                return sync_wait(f->fd, timeout_ns / 1000000) == 0;

        return vc4_wait_seqno(screen, f->seqno, timeout_ns, "fence wait");
}

struct vc4_fence *
vc4_fence_create(struct vc4_screen *screen, uint64_t seqno, int fd)
{
        struct vc4_fence *f = calloc(1, sizeof(*f));

        if (!f)
                return NULL;

        pipe_reference_init(&f->reference, 1);
        f->seqno = seqno;
        f->fd = fd;

        return f;
}

static void
vc4_fence_create_fd(struct pipe_context *pctx, struct pipe_fence_handle **pf,
                    int fd, enum pipe_fd_type type)
{
        struct vc4_context *vc4 = vc4_context(pctx);
        struct vc4_fence **fence = (struct vc4_fence **)pf;

        assert(type == PIPE_FD_TYPE_NATIVE_SYNC);
        *fence = vc4_fence_create(vc4->screen, vc4->last_emit_seqno,
                                  os_dupfd_cloexec(fd));
}

static void
vc4_fence_server_sync(struct pipe_context *pctx,
                      struct pipe_fence_handle *pfence)
{
        struct vc4_context *vc4 = vc4_context(pctx);
        struct vc4_fence *fence = vc4_fence(pfence);

        if (fence->fd >= 0)
                sync_accumulate("vc4", &vc4->in_fence_fd, fence->fd);
}

static int
vc4_fence_get_fd(struct pipe_screen *screen, struct pipe_fence_handle *pfence)
{
        struct vc4_fence *fence = vc4_fence(pfence);

        return os_dupfd_cloexec(fence->fd);
}

int
vc4_fence_context_init(struct vc4_context *vc4)
{
        vc4->base.create_fence_fd = vc4_fence_create_fd;
        vc4->base.fence_server_sync = vc4_fence_server_sync;
        vc4->in_fence_fd = -1;

        /* Since we initialize the in_fence_fd to -1 (no wait necessary),
         * we also need to initialize our in_syncobj as signaled.
         */
        if (vc4->screen->has_syncobj) {
                return drmSyncobjCreate(vc4->fd, DRM_SYNCOBJ_CREATE_SIGNALED,
                                        &vc4->in_syncobj);
        } else {
                return 0;
        }
}

void
vc4_fence_screen_init(struct vc4_screen *screen)
{
        screen->base.fence_reference = vc4_fence_reference;
        screen->base.fence_finish = vc4_fence_finish;
        screen->base.fence_get_fd = vc4_fence_get_fd;
}
