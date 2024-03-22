/*
 * Copyright 2011 Joakim Sindholt <opensource@zhasha.com>
 * Copyright 2015 Patrick Rudolph <siro@das-labor.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE. */

#ifndef _NINE_BUFFER9_H_
#define _NINE_BUFFER9_H_

#include "device9.h"
#include "nine_buffer_upload.h"
#include "nine_state.h"
#include "resource9.h"
#include "pipe/p_context.h"
#include "pipe/p_defines.h"
#include "pipe/p_state.h"
#include "util/list.h"
#include "util/u_box.h"
#include "util/u_upload_mgr.h"

struct pipe_screen;
struct pipe_context;
struct pipe_transfer;

struct NineTransfer {
    struct pipe_transfer *transfer;
    bool is_pipe_secondary;
    struct nine_subbuffer *buf; /* NULL unless subbuffer are used */
    bool should_destroy_buf; /* If the subbuffer should be destroyed */
};

struct NineBuffer9
{
    struct NineResource9 base;

    /* G3D */
    struct NineTransfer *maps;
    int nlocks, nmaps, maxmaps;
    UINT size;

    int16_t bind_count; /* to Device9->state.stream */
    /* Whether only discard and nooverwrite were used so far
     * for this buffer. Allows some optimization. */
    bool discard_nooverwrite_only;
    bool need_sync_if_nooverwrite;
    struct nine_subbuffer *buf;

    /* Specific to managed buffers */
    struct {
        void *data;
        bool dirty;
        struct pipe_box dirty_box; /* region in the resource to update */
        struct pipe_box upload_pending_regions; /* region with uploads pending */
        struct list_head list; /* for update_buffers */
        struct list_head list2; /* for managed_buffers */
        unsigned pending_upload; /* for uploads */
        /* SYSTEMMEM DYNAMIC */
        bool can_unsynchronized; /* Whether the upload can use nooverwrite */
        struct pipe_box valid_region; /* Region in the GPU buffer with valid content */
        struct pipe_box required_valid_region; /* Region that needs to be valid right now. */
        struct pipe_box filled_region; /* Region in the GPU buffer filled since last discard */
        unsigned num_worker_thread_syncs;
        unsigned frame_count_last_discard;
    } managed;
};
static inline struct NineBuffer9 *
NineBuffer9( void *data )
{
    return (struct NineBuffer9 *)data;
}

HRESULT
NineBuffer9_ctor( struct NineBuffer9 *This,
                        struct NineUnknownParams *pParams,
                        D3DRESOURCETYPE Type,
                        DWORD Usage,
                        UINT Size,
                        D3DPOOL Pool );

void
NineBuffer9_dtor( struct NineBuffer9 *This );

struct pipe_resource *
NineBuffer9_GetResource( struct NineBuffer9 *This, unsigned *offset );

HRESULT NINE_WINAPI
NineBuffer9_Lock( struct NineBuffer9 *This,
                        UINT OffsetToLock,
                        UINT SizeToLock,
                        void **ppbData,
                        DWORD Flags );

HRESULT NINE_WINAPI
NineBuffer9_Unlock( struct NineBuffer9 *This );

void
NineBuffer9_Upload( struct NineBuffer9 *This );

static void inline
NineBindBufferToDevice( struct NineDevice9 *device,
                        struct NineBuffer9 **slot,
                        struct NineBuffer9 *buf )
{
    struct NineBuffer9 *old = *slot;

    if (buf) {
        if ((buf->managed.dirty) && list_is_empty(&buf->managed.list))
            list_add(&buf->managed.list, &device->update_buffers);
        buf->bind_count++;
    }
    if (old) {
        old->bind_count--;
        if (!old->bind_count && old->managed.dirty)
            list_delinit(&old->managed.list);
    }

    nine_bind(slot, buf);
}

void
NineBuffer9_SetDirty( struct NineBuffer9 *This );

#define BASEBUF_REGISTER_UPDATE(b) { \
    if ((b)->managed.dirty && (b)->bind_count) \
        if (list_is_empty(&(b)->managed.list)) \
            list_add(&(b)->managed.list, &(b)->base.base.device->update_buffers); \
    }

#endif /* _NINE_BUFFER9_H_ */
