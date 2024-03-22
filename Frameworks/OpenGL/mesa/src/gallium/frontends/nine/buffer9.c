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

#include "buffer9.h"
#include "device9.h"
#include "indexbuffer9.h"
#include "nine_buffer_upload.h"
#include "nine_helpers.h"
#include "nine_pipe.h"

#include "pipe/p_screen.h"
#include "pipe/p_context.h"
#include "pipe/p_state.h"
#include "pipe/p_defines.h"
#include "util/format/u_formats.h"
#include "util/u_box.h"
#include "util/u_inlines.h"

#define DBG_CHANNEL (DBG_INDEXBUFFER|DBG_VERTEXBUFFER)

HRESULT
NineBuffer9_ctor( struct NineBuffer9 *This,
                        struct NineUnknownParams *pParams,
                        D3DRESOURCETYPE Type,
                        DWORD Usage,
                        UINT Size,
                        D3DPOOL Pool )
{
    struct pipe_resource *info = &This->base.info;
    HRESULT hr;

    DBG("This=%p Size=0x%x Usage=%x Pool=%u\n", This, Size, Usage, Pool);

    user_assert(Pool != D3DPOOL_SCRATCH, D3DERR_INVALIDCALL);

    This->maps = MALLOC(sizeof(struct NineTransfer));
    if (!This->maps)
        return E_OUTOFMEMORY;
    This->nlocks = 0;
    This->nmaps = 0;
    This->maxmaps = 1;
    This->size = Size;

    info->screen = pParams->device->screen;
    info->target = PIPE_BUFFER;
    info->format = PIPE_FORMAT_R8_UNORM;
    info->width0 = Size;
    info->flags = 0;

    /* Note: WRITEONLY is just tip for resource placement, the resource
     * can still be read (but slower). */
    info->bind = (Type == D3DRTYPE_INDEXBUFFER) ? PIPE_BIND_INDEX_BUFFER : PIPE_BIND_VERTEX_BUFFER;

    /* Software vertex processing:
     * If the device is full software vertex processing,
     * then the buffer is supposed to be used only for sw processing.
     * For mixed vertex processing, buffers with D3DUSAGE_SOFTWAREPROCESSING
     * can be used for both sw and hw processing.
     * These buffers are expected to be stored in RAM.
     * Apps expect locking the full buffer with no flags, then
     * render a a few primitive, then locking again, etc
     * to be a fast pattern. Only the SYSTEMMEM DYNAMIC path
     * will give that pattern ok performance in our case.
     * An alternative would be when sw processing is detected to
     * convert Draw* calls to Draw*Up calls. */
    if (Usage & D3DUSAGE_SOFTWAREPROCESSING ||
        pParams->device->params.BehaviorFlags & D3DCREATE_SOFTWARE_VERTEXPROCESSING) {
        Pool = D3DPOOL_SYSTEMMEM;
        Usage |= D3DUSAGE_DYNAMIC;
        /* Note: the application cannot retrieve Pool and Usage */
    }

    /* Always use the DYNAMIC path for SYSTEMMEM.
     * If the app uses the vertex buffer is a dynamic fashion,
     * this is going to be very significantly faster that way.
     * If the app uses the vertex buffer in a static fashion,
     * instead of being filled all at once, the buffer will be filled
     * little per little, until it is fully filled, thus the perf hit
     * will be very small. */
    if (Pool == D3DPOOL_SYSTEMMEM)
        Usage |= D3DUSAGE_DYNAMIC;

    /* It is hard to find clear information on where to place the buffer in
     * memory depending on the flag.
     * MSDN: resources are static, except for those with DYNAMIC, thus why you
     *   can only use DISCARD on them.
     * ATI doc: The driver has the liberty it wants for having things static
     *   or not.
     *   MANAGED: Ram + uploads to Vram copy at unlock (msdn and nvidia doc say
     *   at first draw call using the buffer)
     *   DEFAULT + Usage = 0 => System memory backing for easy read access
     *   (That doc is very unclear on the details, like whether some copies to
     *   vram copy are involved or not).
     *   DEFAULT + WRITEONLY => Vram
     *   DEFAULT + WRITEONLY + DYNAMIC => Either Vram buffer or GTT_WC, depending on what the driver wants.
     *   SYSTEMMEM: Same as MANAGED, but handled by the driver instead of the runtime (which means
     *   some small behavior differences between vendors). Implementing exactly as MANAGED should
     *   be fine.
     */
    if (Pool == D3DPOOL_SYSTEMMEM && Usage & D3DUSAGE_DYNAMIC)
        info->usage = PIPE_USAGE_STREAM;
    else if (Pool != D3DPOOL_DEFAULT)
        info->usage = PIPE_USAGE_DEFAULT;
    else if (Usage & D3DUSAGE_DYNAMIC && Usage & D3DUSAGE_WRITEONLY)
        info->usage = PIPE_USAGE_STREAM;
    else if (Usage & D3DUSAGE_WRITEONLY)
        info->usage = PIPE_USAGE_DEFAULT;
    /* For the remaining two, PIPE_USAGE_STAGING would probably be
     * a good fit according to the doc. However it seems rather a mistake
     * from apps to use these (mistakes that do really happen). Try
     * to put the flags that are the best compromise between the real
     * behaviour and what buggy apps should get for better performance. */
    else if (Usage & D3DUSAGE_DYNAMIC)
        info->usage = PIPE_USAGE_STREAM;
    else
        info->usage = PIPE_USAGE_DYNAMIC;

    /* When Writeonly is not set, we don't want to enable the
     * optimizations */
    This->discard_nooverwrite_only = !!(Usage & D3DUSAGE_WRITEONLY) &&
                                     pParams->device->buffer_upload;
    /* if (pDesc->Usage & D3DUSAGE_DONOTCLIP) { } */
    /* if (pDesc->Usage & D3DUSAGE_NONSECURE) { } */
    /* if (pDesc->Usage & D3DUSAGE_NPATCHES) { } */
    /* if (pDesc->Usage & D3DUSAGE_POINTS) { } */
    /* if (pDesc->Usage & D3DUSAGE_RTPATCHES) { } */
    /* if (pDesc->Usage & D3DUSAGE_TEXTAPI) { } */

    info->height0 = 1;
    info->depth0 = 1;
    info->array_size = 1;
    info->last_level = 0;
    info->nr_samples = 0;
    info->nr_storage_samples = 0;

    hr = NineResource9_ctor(&This->base, pParams, NULL, true,
                            Type, Pool, Usage);

    if (FAILED(hr))
        return hr;

    if (Pool != D3DPOOL_DEFAULT) {
        This->managed.data = align_calloc(
            nine_format_get_level_alloc_size(This->base.info.format,
                                             Size, 1, 0), 32);
        if (!This->managed.data)
            return E_OUTOFMEMORY;
        This->managed.dirty = true;
        u_box_1d(0, Size, &This->managed.dirty_box);
        u_box_1d(0, 0, &This->managed.valid_region);
        u_box_1d(0, 0, &This->managed.required_valid_region);
        u_box_1d(0, 0, &This->managed.filled_region);
        This->managed.can_unsynchronized = true;
        This->managed.num_worker_thread_syncs = 0;
        list_inithead(&This->managed.list);
        list_inithead(&This->managed.list2);
        list_add(&This->managed.list2, &pParams->device->managed_buffers);
    }

    return D3D_OK;
}

void
NineBuffer9_dtor( struct NineBuffer9 *This )
{
    DBG("This=%p\n", This);

    if (This->maps) {
        while (This->nlocks) {
            NineBuffer9_Unlock(This);
        }
        assert(!This->nmaps);
        FREE(This->maps);
    }

    if (This->base.pool != D3DPOOL_DEFAULT) {
        if (This->managed.data)
            align_free(This->managed.data);
        if (list_is_linked(&This->managed.list))
            list_del(&This->managed.list);
        if (list_is_linked(&This->managed.list2))
            list_del(&This->managed.list2);
    }

    if (This->buf)
        nine_upload_release_buffer(This->base.base.device->buffer_upload, This->buf);

    NineResource9_dtor(&This->base);
}

struct pipe_resource *
NineBuffer9_GetResource( struct NineBuffer9 *This, unsigned *offset )
{
    if (This->buf)
        return nine_upload_buffer_resource_and_offset(This->buf, offset);
    *offset = 0;
    return NineResource9_GetResource(&This->base);
}

static void
NineBuffer9_RebindIfRequired( struct NineBuffer9 *This,
                              struct NineDevice9 *device,
                              struct pipe_resource *resource,
                              unsigned offset )
{
    int i;

    if (!This->bind_count)
        return;
    for (i = 0; i < device->caps.MaxStreams; i++) {
        if (device->state.stream[i] == (struct NineVertexBuffer9 *)This)
            nine_context_set_stream_source_apply(device, i,
                                                 resource,
                                                 device->state.vtxbuf[i].buffer_offset + offset,
                                                 device->state.vtxstride[i]);
    }
    if (device->state.idxbuf == (struct NineIndexBuffer9 *)This)
        nine_context_set_indices_apply(device, resource,
                                       ((struct NineIndexBuffer9 *)This)->index_size,
                                       offset);
}

HRESULT NINE_WINAPI
NineBuffer9_Lock( struct NineBuffer9 *This,
                        UINT OffsetToLock,
                        UINT SizeToLock,
                        void **ppbData,
                        DWORD Flags )
{
    struct NineDevice9 *device = This->base.base.device;
    struct pipe_box box;
    struct pipe_context *pipe;
    void *data;
    unsigned usage;

    DBG("This=%p(pipe=%p) OffsetToLock=0x%x, SizeToLock=0x%x, Flags=0x%x\n",
        This, This->base.resource,
        OffsetToLock, SizeToLock, Flags);

    user_assert(ppbData, E_POINTER);

    if (SizeToLock == 0) {
        SizeToLock = This->size - OffsetToLock;
        user_warn(OffsetToLock != 0);
    }

    /* Write out of bound seems to have to be taken into account for these.
     * TODO: Do more tests (is it only at buffer first lock ? etc).
     * Since these buffers are supposed to be locked once and never
     * writen again (MANAGED or DYNAMIC is used for the other uses cases),
     * performance should be unaffected. */
    if (!(This->base.usage & D3DUSAGE_DYNAMIC) && This->base.pool == D3DPOOL_DEFAULT)
        SizeToLock = This->size - OffsetToLock;

    SizeToLock = MIN2(SizeToLock, This->size - OffsetToLock); /* Do not read or track out of the buffer */
    u_box_1d(OffsetToLock, SizeToLock, &box);

    if (This->base.pool != D3DPOOL_DEFAULT) {
        /* MANAGED: READONLY doesn't dirty the buffer, nor
         * wait the upload in the worker thread
         * SYSTEMMEM: AMD/NVidia: All locks dirty the full buffer. Not on Intel
         * For Nvidia, SYSTEMMEM behaves are if there is no worker thread.
         * On AMD, READONLY and NOOVERWRITE do dirty the buffer, but do not sync the previous uploads
         * in the worker thread. On Intel only NOOVERWRITE has that effect.
         * We implement the AMD behaviour. */
        if (This->base.pool == D3DPOOL_MANAGED) {
            if (!(Flags & D3DLOCK_READONLY)) {
                if (!This->managed.dirty) {
                    assert(list_is_empty(&This->managed.list));
                    This->managed.dirty = true;
                    This->managed.dirty_box = box;
                    /* Flush if regions pending to be uploaded would be dirtied */
                    if (p_atomic_read(&This->managed.pending_upload)) {
                        u_box_intersect_1d(&box, &box, &This->managed.upload_pending_regions);
                        if (box.width != 0)
                            nine_csmt_process(This->base.base.device);
                    }
                } else
                    u_box_union_1d(&This->managed.dirty_box, &This->managed.dirty_box, &box);
                /* Tests trying to draw while the buffer is locked show that
                 * SYSTEMMEM/MANAGED buffers are made dirty at Lock time */
                BASEBUF_REGISTER_UPDATE(This);
            }
        } else {
            if (!(Flags & (D3DLOCK_READONLY|D3DLOCK_NOOVERWRITE)) &&
                p_atomic_read(&This->managed.pending_upload)) {
                This->managed.num_worker_thread_syncs++;
                /* If we sync too often, pick the vertex_uploader path */
                if (This->managed.num_worker_thread_syncs >= 3)
                    This->managed.can_unsynchronized = false;
                nine_csmt_process(This->base.base.device);
                /* Note: AS DISCARD is not relevant for SYSTEMMEM,
                 * NOOVERWRITE might have a similar meaning as what is
                 * in D3D7 doc. Basically that data from previous draws
                 * OF THIS FRAME are unaffected. As we flush csmt in Present(),
                 * we should be correct. In some parts of the doc, the notion
                 * of frame is implied to be related to Begin/EndScene(),
                 * but tests show NOOVERWRITE after EndScene() doesn't flush
                 * the csmt thread. */
            }
            This->managed.dirty = true;
            u_box_1d(0, This->size, &This->managed.dirty_box); /* systemmem non-dynamic */
            u_box_1d(0, 0, &This->managed.valid_region); /* systemmem dynamic */
            BASEBUF_REGISTER_UPDATE(This);
        }

        *ppbData = (int8_t *)This->managed.data + OffsetToLock;
        DBG("returning pointer %p\n", *ppbData);
        This->nlocks++;
        return D3D_OK;
    }

    /* Driver ddi doc: READONLY is never passed to the device. So it can only
     * have effect on things handled by the driver (MANAGED pool for example).
     * Msdn doc: DISCARD and NOOVERWRITE are only for DYNAMIC.
     * ATI doc: You can use DISCARD and NOOVERWRITE without DYNAMIC.
     * Msdn doc: D3DLOCK_DONOTWAIT is not among the valid flags for buffers.
     * Our tests: On win 7 nvidia, D3DLOCK_DONOTWAIT does return
     * D3DERR_WASSTILLDRAWING if the resource is in use, except for DYNAMIC.
     * Our tests: some apps do use both DISCARD and NOOVERWRITE at the same
     * time. On windows it seems to return different pointer in some conditions,
     * creation flags and drivers. However these tests indicate having
     * NOOVERWRITE win is a valid behaviour (NVidia).
     */

    /* Have NOOVERWRITE win over DISCARD. This is allowed (see above) and
     * it prevents overconsuming buffers if apps do use both at the same time. */
    if ((Flags & (D3DLOCK_DISCARD | D3DLOCK_NOOVERWRITE)) == (D3DLOCK_DISCARD | D3DLOCK_NOOVERWRITE))
        Flags &= ~D3DLOCK_DISCARD;

    if (Flags & D3DLOCK_DISCARD)
        usage = PIPE_MAP_WRITE | PIPE_MAP_DISCARD_WHOLE_RESOURCE;
    else if (Flags & D3DLOCK_NOOVERWRITE)
        usage = PIPE_MAP_WRITE | PIPE_MAP_UNSYNCHRONIZED;
    else
        /* Do not ask for READ if writeonly and default pool (should be safe enough,
         * as the doc says app shouldn't expect reading to work with writeonly). */
        usage = (This->base.usage & D3DUSAGE_WRITEONLY) ?
            PIPE_MAP_WRITE :
            PIPE_MAP_READ_WRITE;
    if (Flags & D3DLOCK_DONOTWAIT && !(This->base.usage & D3DUSAGE_DYNAMIC))
        usage |= PIPE_MAP_DONTBLOCK;

    This->discard_nooverwrite_only &= !!(Flags & (D3DLOCK_DISCARD | D3DLOCK_NOOVERWRITE));

    if (This->nmaps == This->maxmaps) {
        struct NineTransfer *newmaps =
            REALLOC(This->maps, sizeof(struct NineTransfer)*This->maxmaps,
                    sizeof(struct NineTransfer)*(This->maxmaps << 1));
        if (newmaps == NULL)
            return E_OUTOFMEMORY;

        This->maxmaps <<= 1;
        This->maps = newmaps;
    }

    if (This->buf && !This->discard_nooverwrite_only) {
        struct pipe_box src_box;
        unsigned offset;
        struct pipe_resource *src_res;
        DBG("Disabling nine_subbuffer for a buffer having"
            "used a nine_subbuffer buffer\n");
        /* Copy buffer content to the buffer resource, which
         * we will now use.
         * Note: The behaviour may be different from what is expected
         * with double lock. However applications can't really make expectations
         * about double locks, and don't really use them, so that's ok. */
        src_res = nine_upload_buffer_resource_and_offset(This->buf, &offset);
        u_box_1d(offset, This->size, &src_box);

        pipe = NineDevice9_GetPipe(device);
        pipe->resource_copy_region(pipe, This->base.resource, 0, 0, 0, 0,
                                   src_res, 0, &src_box);
        /* Release previous resource */
        if (This->nmaps >= 1)
            This->maps[This->nmaps-1].should_destroy_buf = true;
        else
            nine_upload_release_buffer(device->buffer_upload, This->buf);
        This->buf = NULL;
        /* Rebind buffer */
        NineBuffer9_RebindIfRequired(This, device, This->base.resource, 0);
    }

    This->maps[This->nmaps].transfer = NULL;
    This->maps[This->nmaps].is_pipe_secondary = false;
    This->maps[This->nmaps].buf = NULL;
    This->maps[This->nmaps].should_destroy_buf = false;

    if (This->discard_nooverwrite_only) {
        if (This->buf && (Flags & D3DLOCK_DISCARD)) {
            /* Release previous buffer */
            if (This->nmaps >= 1)
                This->maps[This->nmaps-1].should_destroy_buf = true;
            else
                nine_upload_release_buffer(device->buffer_upload, This->buf);
            This->buf = NULL;
        }

        if (!This->buf) {
            unsigned offset;
            struct pipe_resource *res;
            This->buf = nine_upload_create_buffer(device->buffer_upload, This->base.info.width0);
            res = nine_upload_buffer_resource_and_offset(This->buf, &offset);
            NineBuffer9_RebindIfRequired(This, device, res, offset);
        }

        if (This->buf) {
            This->maps[This->nmaps].buf = This->buf;
            This->nmaps++;
            This->nlocks++;
            DBG("Returning %p\n", nine_upload_buffer_get_map(This->buf) + OffsetToLock);
            *ppbData = nine_upload_buffer_get_map(This->buf) + OffsetToLock;
            return D3D_OK;
        } else {
            /* Fallback to normal path, and don't try again */
            This->discard_nooverwrite_only = false;
        }
    }

    /* Previous mappings may need pending commands to write to the
     * buffer (staging buffer for example). Before a NOOVERWRITE,
     * we thus need a finish, to guarantee any upload is finished.
     * Note for discard_nooverwrite_only we don't need to do this
     * check as neither discard nor nooverwrite have issues there */
    if (This->need_sync_if_nooverwrite && !(Flags & D3DLOCK_DISCARD) &&
        (Flags & D3DLOCK_NOOVERWRITE)) {
        struct pipe_screen *screen = NineDevice9_GetScreen(device);
        struct pipe_fence_handle *fence = NULL;

        pipe = NineDevice9_GetPipe(device);
        pipe->flush(pipe, &fence, 0);
        (void) screen->fence_finish(screen, NULL, fence, OS_TIMEOUT_INFINITE);
        screen->fence_reference(screen, &fence, NULL);
    }
    This->need_sync_if_nooverwrite = !(Flags & (D3DLOCK_DISCARD | D3DLOCK_NOOVERWRITE));

    /* When csmt is active, we want to avoid stalls as much as possible,
     * and thus we want to create a new resource on discard and map it
     * with the secondary pipe, instead of waiting on the main pipe. */
    if (Flags & D3DLOCK_DISCARD && device->csmt_active) {
        struct pipe_screen *screen = NineDevice9_GetScreen(device);
        struct pipe_resource *new_res = nine_resource_create_with_retry(device, screen, &This->base.info);
        if (new_res) {
            /* Use the new resource */
            pipe_resource_reference(&This->base.resource, new_res);
            pipe_resource_reference(&new_res, NULL);
            usage = PIPE_MAP_WRITE | PIPE_MAP_UNSYNCHRONIZED;
            NineBuffer9_RebindIfRequired(This, device, This->base.resource, 0);
            This->maps[This->nmaps].is_pipe_secondary = true;
        }
    } else if (Flags & D3DLOCK_NOOVERWRITE && device->csmt_active)
        This->maps[This->nmaps].is_pipe_secondary = true;

    if (This->maps[This->nmaps].is_pipe_secondary)
        pipe = device->pipe_secondary;
    else
        pipe = NineDevice9_GetPipe(device);

    data = pipe->buffer_map(pipe, This->base.resource, 0,
                              usage, &box, &This->maps[This->nmaps].transfer);

    if (!data) {
        DBG("pipe::buffer_map failed\n"
            " usage = %x\n"
            " box.x = %u\n"
            " box.width = %u\n",
            usage, box.x, box.width);

        if (Flags & D3DLOCK_DONOTWAIT)
            return D3DERR_WASSTILLDRAWING;
        return D3DERR_INVALIDCALL;
    }

    DBG("returning pointer %p\n", data);
    This->nmaps++;
    This->nlocks++;
    *ppbData = data;

    return D3D_OK;
}

HRESULT NINE_WINAPI
NineBuffer9_Unlock( struct NineBuffer9 *This )
{
    struct NineDevice9 *device = This->base.base.device;
    struct pipe_context *pipe;
    int i;
    DBG("This=%p\n", This);

    user_assert(This->nlocks > 0, D3DERR_INVALIDCALL);
    This->nlocks--;
    if (This->nlocks > 0)
        return D3D_OK; /* Pending unlocks. Wait all unlocks before unmapping */

    if (This->base.pool == D3DPOOL_DEFAULT) {
        for (i = 0; i < This->nmaps; i++) {
            if (!This->maps[i].buf) {
                pipe = This->maps[i].is_pipe_secondary ?
                    device->pipe_secondary :
                    nine_context_get_pipe_acquire(device);
                pipe->buffer_unmap(pipe, This->maps[i].transfer);
                /* We need to flush in case the driver does implicit copies */
                if (This->maps[i].is_pipe_secondary)
                    pipe->flush(pipe, NULL, 0);
                else
                    nine_context_get_pipe_release(device);
            } else if (This->maps[i].should_destroy_buf)
                nine_upload_release_buffer(device->buffer_upload, This->maps[i].buf);
        }
        This->nmaps = 0;
    }
    return D3D_OK;
}

void
NineBuffer9_SetDirty( struct NineBuffer9 *This )
{
    assert(This->base.pool != D3DPOOL_DEFAULT);

    This->managed.dirty = true;
    u_box_1d(0, This->size, &This->managed.dirty_box);
    BASEBUF_REGISTER_UPDATE(This);
}

/* Try to remove b from a, supposed to include b */
static void u_box_try_remove_region_1d(struct pipe_box *dst,
                                       const struct pipe_box *a,
                                       const struct pipe_box *b)
{
    int x, width;
    if (a->x == b->x) {
        x = a->x + b->width;
        width = a->width - b->width;
    } else if ((a->x + a->width) == (b->x + b->width)) {
        x = a->x;
        width = a->width - b->width;
    } else {
        x = a->x;
        width = a->width;
    }
    dst->x = x;
    dst->width = width;
}

void
NineBuffer9_Upload( struct NineBuffer9 *This )
{
    struct NineDevice9 *device = This->base.base.device;
    unsigned upload_flags = 0;
    struct pipe_box box_upload;

    assert(This->base.pool != D3DPOOL_DEFAULT && This->managed.dirty);

    if (This->base.pool == D3DPOOL_SYSTEMMEM && This->base.usage & D3DUSAGE_DYNAMIC) {
        struct pipe_box region_already_valid;
        struct pipe_box conflicting_region;
        struct pipe_box *valid_region = &This->managed.valid_region;
        struct pipe_box *required_valid_region = &This->managed.required_valid_region;
        struct pipe_box *filled_region = &This->managed.filled_region;
        /* Try to upload SYSTEMMEM DYNAMIC in an efficient fashion.
         * Unlike non-dynamic for which we upload the whole dirty region, try to
         * only upload the data needed for the draw. The draw call preparation
         * fills This->managed.required_valid_region for that */
        u_box_intersect_1d(&region_already_valid,
                           valid_region,
                           required_valid_region);
        /* If the required valid region is already valid, nothing to do */
        if (region_already_valid.x == required_valid_region->x &&
            region_already_valid.width == required_valid_region->width) {
            /* Rebind if the region happens to be valid in the original buffer
             * but we have since used vertex_uploader */
            if (!This->managed.can_unsynchronized)
                NineBuffer9_RebindIfRequired(This, device, This->base.resource, 0);
            u_box_1d(0, 0, required_valid_region);
            return;
        }
        /* (Try to) Remove valid areas from the region to upload */
        u_box_try_remove_region_1d(&box_upload,
                                   required_valid_region,
                                   &region_already_valid);
        assert(box_upload.width > 0);
        /* To maintain correctly the valid region, as we will do union later with
         * box_upload, we must ensure box_upload is consecutive with valid_region */
        if (box_upload.x > valid_region->x + valid_region->width && valid_region->width > 0) {
            box_upload.width = box_upload.x + box_upload.width - (valid_region->x + valid_region->width);
            box_upload.x = valid_region->x + valid_region->width;
        } else if (box_upload.x + box_upload.width < valid_region->x && valid_region->width > 0) {
            box_upload.width = valid_region->x - box_upload.x;
        }
        /* There is conflict if some areas, that are not valid but are filled for previous draw calls,
         * intersect with the region we plan to upload. Note by construction valid_region IS
         * included in filled_region, thus so is region_already_valid. */
        u_box_intersect_1d(&conflicting_region, &box_upload, filled_region);
        /* As box_upload could still contain region_already_valid, check the intersection
         * doesn't happen to be exactly region_already_valid (it cannot be smaller, see above) */
        if (This->managed.can_unsynchronized && (conflicting_region.width == 0 ||
            (conflicting_region.x == region_already_valid.x &&
             conflicting_region.width == region_already_valid.width))) {
            /* No conflicts. */
            upload_flags |= PIPE_MAP_UNSYNCHRONIZED;
        } else {
            /* We cannot use PIPE_MAP_UNSYNCHRONIZED. We must choose between no flag and DISCARD.
             * Criterias to discard:
             * . Most of the resource was filled (but some apps do allocate a big buffer
             * to only use a small part in a round fashion)
             * . The region to upload is very small compared to the filled region and
             * at the start of the buffer (hints at round usage starting again)
             * . The region to upload is very big compared to the required region
             * . We have not discarded yet this frame
             * If the buffer use pattern seems to sync the worker thread too often,
             * revert to the vertex_uploader */
            if (This->managed.num_worker_thread_syncs < 3 &&
                (filled_region->width > (This->size / 2) ||
                 (10 * box_upload.width < filled_region->width &&
                  box_upload.x < (filled_region->x + filled_region->width)/2) ||
                 box_upload.width > 2 * required_valid_region->width ||
                 This->managed.frame_count_last_discard != device->frame_count)) {
                /* Avoid DISCARDING too much by discarding only if most of the buffer
                 * has been used */
                DBG_FLAG(DBG_INDEXBUFFER|DBG_VERTEXBUFFER,
             "Uploading %p DISCARD: valid %d %d, filled %d %d, required %d %d, box_upload %d %d, required already_valid %d %d, conficting %d %d\n",
             This, valid_region->x, valid_region->width, filled_region->x, filled_region->width,
             required_valid_region->x, required_valid_region->width, box_upload.x, box_upload.width,
             region_already_valid.x, region_already_valid.width, conflicting_region.x, conflicting_region.width
                );
                upload_flags |= PIPE_MAP_DISCARD_WHOLE_RESOURCE;
                u_box_1d(0, 0, filled_region);
                u_box_1d(0, 0, valid_region);
                box_upload = This->managed.required_valid_region;
                /* Rebind the buffer if we used intermediate alternative buffer */
                if (!This->managed.can_unsynchronized)
                    NineBuffer9_RebindIfRequired(This, device, This->base.resource, 0);
                This->managed.can_unsynchronized = true;
                This->managed.frame_count_last_discard = device->frame_count;
            } else {
                /* Once we use without UNSYNCHRONIZED, we cannot use it anymore.
                 * Use a different buffer. */
                unsigned buffer_offset = 0;
                struct pipe_resource *resource = NULL;
                This->managed.can_unsynchronized = false;
                u_upload_data(device->vertex_uploader,
                    required_valid_region->x,
                    required_valid_region->width,
                    64,
                    This->managed.data + required_valid_region->x,
                    &buffer_offset,
                    &resource);
                buffer_offset -= required_valid_region->x;
                u_upload_unmap(device->vertex_uploader);
                if (resource) {
                    NineBuffer9_RebindIfRequired(This, device, resource, buffer_offset);
                    /* Note: This only works because for these types of buffers this function
                     * is called before every draw call. Else it wouldn't work when the app
                     * rebinds buffers. In addition it needs this function to be called only
                     * once per buffers even if bound several times, which we do. */
                    u_box_1d(0, 0, required_valid_region);
                    pipe_resource_reference(&resource, NULL);
                    return;
                }
            }
        }

        u_box_union_1d(filled_region,
                       filled_region,
                       &box_upload);
        u_box_union_1d(valid_region,
                       valid_region,
                       &box_upload);
        u_box_1d(0, 0, required_valid_region);
    } else
        box_upload = This->managed.dirty_box;

    if (box_upload.x == 0 && box_upload.width == This->size) {
        upload_flags |= PIPE_MAP_DISCARD_WHOLE_RESOURCE;
    }

    if (This->managed.pending_upload) {
        u_box_union_1d(&This->managed.upload_pending_regions,
                       &This->managed.upload_pending_regions,
                       &box_upload);
    } else {
        This->managed.upload_pending_regions = box_upload;
    }

    DBG_FLAG(DBG_INDEXBUFFER|DBG_VERTEXBUFFER,
             "Uploading %p, offset=%d, size=%d, Flags=0x%x\n",
             This, box_upload.x, box_upload.width, upload_flags);
    nine_context_range_upload(device, &This->managed.pending_upload,
                              (struct NineUnknown *)This,
                              This->base.resource,
                              box_upload.x,
                              box_upload.width,
                              upload_flags,
                              (int8_t *)This->managed.data + box_upload.x);
    This->managed.dirty = false;
}
