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

#ifndef V3D_BUFMGR_H
#define V3D_BUFMGR_H

#include <stdint.h>
#include "util/u_hash_table.h"
#include "util/u_inlines.h"
#include "util/list.h"
#include "v3d_screen.h"

struct v3d_context;

struct v3d_bo {
        struct pipe_reference reference;
        struct v3d_screen *screen;
        void *map;
        const char *name;
        uint32_t handle;
        uint32_t size;

        /* Address of the BO in our page tables. */
        uint32_t offset;

        /** Entry in the linked list of buffers freed, by age. */
        struct list_head time_list;
        /** Entry in the per-page-count linked list of buffers freed (by age). */
        struct list_head size_list;
        /** Approximate second when the bo was freed. */
        time_t free_time;
        /**
         * Whether only our process has a reference to the BO (meaning that
         * it's safe to reuse it in the BO cache).
         */
        bool private;
};

struct v3d_bo *v3d_bo_alloc(struct v3d_screen *screen, uint32_t size,
                            const char *name);
void v3d_bo_last_unreference(struct v3d_bo *bo);
void v3d_bo_last_unreference_locked_timed(struct v3d_bo *bo, time_t time);
struct v3d_bo *v3d_bo_open_name(struct v3d_screen *screen, uint32_t name);
struct v3d_bo *v3d_bo_open_dmabuf(struct v3d_screen *screen, int fd);
bool v3d_bo_flink(struct v3d_bo *bo, uint32_t *name);
int v3d_bo_get_dmabuf(struct v3d_bo *bo);

static inline void
v3d_bo_set_reference(struct v3d_bo **old_bo, struct v3d_bo *new_bo)
{
        if (pipe_reference(&(*old_bo)->reference, &new_bo->reference))
                v3d_bo_last_unreference(*old_bo);
        *old_bo = new_bo;
}

static inline struct v3d_bo *
v3d_bo_reference(struct v3d_bo *bo)
{
        pipe_reference(NULL, &bo->reference);
        return bo;
}

static inline void
v3d_bo_unreference(struct v3d_bo **bo)
{
        struct v3d_screen *screen;
        if (!*bo)
                return;

        if ((*bo)->private) {
                /* Avoid the mutex for private BOs */
                if (pipe_reference(&(*bo)->reference, NULL))
                        v3d_bo_last_unreference(*bo);
        } else {
                screen = (*bo)->screen;
                mtx_lock(&screen->bo_handles_mutex);

                if (pipe_reference(&(*bo)->reference, NULL)) {
                        _mesa_hash_table_remove_key(screen->bo_handles,
                                               (void *)(uintptr_t)(*bo)->handle);
                        v3d_bo_last_unreference(*bo);
                }

                mtx_unlock(&screen->bo_handles_mutex);
        }

        *bo = NULL;
}

static inline void
v3d_bo_unreference_locked_timed(struct v3d_bo **bo, time_t time)
{
        if (!*bo)
                return;

        if (pipe_reference(&(*bo)->reference, NULL))
                v3d_bo_last_unreference_locked_timed(*bo, time);
        *bo = NULL;
}

void *
v3d_bo_map(struct v3d_bo *bo);

void *
v3d_bo_map_unsynchronized(struct v3d_bo *bo);

bool
v3d_bo_wait(struct v3d_bo *bo, uint64_t timeout_ns, const char *reason);

bool
v3d_wait_seqno(struct v3d_screen *screen, uint64_t seqno, uint64_t timeout_ns,
               const char *reason);

void
v3d_bufmgr_destroy(struct pipe_screen *pscreen);

#endif /* V3D_BUFMGR_H */

