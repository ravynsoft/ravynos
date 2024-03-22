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

#ifndef VC4_BUFMGR_H
#define VC4_BUFMGR_H

#include <stdint.h>
#include "util/u_hash_table.h"
#include "util/u_inlines.h"
#include "vc4_qir.h"

struct vc4_context;

struct vc4_bo {
        struct pipe_reference reference;
        struct vc4_screen *screen;
        void *map;
        const char *name;
        uint32_t handle;
        uint32_t size;

        /* This will be read/written by multiple threads without a lock -- you
         * should take a snapshot and use it to see if you happen to be in the
         * CL's handles at this position, to make most lookups O(1).  It's
         * volatile to make sure that the compiler doesn't emit multiple loads
         * from the address, which would make the lookup racy.
         */
        volatile uint32_t last_hindex;

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

struct vc4_bo *vc4_bo_alloc(struct vc4_screen *screen, uint32_t size,
                            const char *name);
struct vc4_bo *vc4_bo_alloc_shader(struct vc4_screen *screen, const void *data,
                                   uint32_t size);
void vc4_bo_last_unreference(struct vc4_bo *bo);
void vc4_bo_last_unreference_locked_timed(struct vc4_bo *bo, time_t time);
struct vc4_bo *vc4_bo_open_name(struct vc4_screen *screen, uint32_t name);
struct vc4_bo *vc4_bo_open_dmabuf(struct vc4_screen *screen, int fd);
bool vc4_bo_flink(struct vc4_bo *bo, uint32_t *name);
int vc4_bo_get_dmabuf(struct vc4_bo *bo);

void vc4_bo_debug_describe(char* buf, const struct vc4_bo *ptr);
static inline struct vc4_bo *
vc4_bo_reference(struct vc4_bo *bo)
{
        pipe_reference_described(NULL, &bo->reference,
                                 (debug_reference_descriptor)
                                 vc4_bo_debug_describe);
        return bo;
}

static inline void
vc4_bo_unreference(struct vc4_bo **bo)
{
        struct vc4_screen *screen;
        if (!*bo)
                return;

        if ((*bo)->private) {
                /* Avoid the mutex for private BOs */
                if (pipe_reference_described(&(*bo)->reference, NULL,
                                             (debug_reference_descriptor)
                                             vc4_bo_debug_describe)) {
                        vc4_bo_last_unreference(*bo);
                }
        } else {
                screen = (*bo)->screen;
                mtx_lock(&screen->bo_handles_mutex);

                if (pipe_reference_described(&(*bo)->reference, NULL,
                                             (debug_reference_descriptor)
                                             vc4_bo_debug_describe)) {
                        _mesa_hash_table_remove_key(screen->bo_handles,
                                               (void *)(uintptr_t)(*bo)->handle);
                        vc4_bo_last_unreference(*bo);
                }

                mtx_unlock(&screen->bo_handles_mutex);
        }

        *bo = NULL;
}

static inline void
vc4_bo_unreference_locked_timed(struct vc4_bo **bo, time_t time)
{
        if (!*bo)
                return;

        if (pipe_reference_described(&(*bo)->reference, NULL,
                                     (debug_reference_descriptor)
                                     vc4_bo_debug_describe)) {
                vc4_bo_last_unreference_locked_timed(*bo, time);
        }
        *bo = NULL;
}

void *
vc4_bo_map(struct vc4_bo *bo);

void *
vc4_bo_map_unsynchronized(struct vc4_bo *bo);

bool
vc4_bo_wait(struct vc4_bo *bo, uint64_t timeout_ns, const char *reason);

bool
vc4_wait_seqno(struct vc4_screen *screen, uint64_t seqno, uint64_t timeout_ns,
               const char *reason);

void
vc4_bo_label(struct vc4_screen *screen, struct vc4_bo *bo, const char *fmt, ...);

void
vc4_bufmgr_destroy(struct pipe_screen *pscreen);

#endif /* VC4_BUFMGR_H */

