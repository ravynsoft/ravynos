/*
 * Copyright 2011 Joakim Sindholt <opensource@zhasha.com>
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

#include "device9.h"
#include "volume9.h"
#include "basetexture9.h" /* for marking dirty */
#include "volumetexture9.h"
#include "nine_helpers.h"
#include "nine_pipe.h"
#include "nine_dump.h"

#include "util/format/u_format.h"
#include "util/u_surface.h"

#define DBG_CHANNEL DBG_VOLUME


static HRESULT
NineVolume9_AllocateData( struct NineVolume9 *This )
{
    unsigned size = This->layer_stride * This->desc.Depth;

    DBG("(%p(This=%p),level=%u) Allocating 0x%x bytes of system memory.\n",
        This->base.container, This, This->level, size);

    This->data = (uint8_t *)align_calloc(size, 32);
    if (!This->data)
        return E_OUTOFMEMORY;
    return D3D_OK;
}

static HRESULT
NineVolume9_ctor( struct NineVolume9 *This,
                  struct NineUnknownParams *pParams,
                  struct NineUnknown *pContainer,
                  struct pipe_resource *pResource,
                  unsigned Level,
                  D3DVOLUME_DESC *pDesc )
{
    HRESULT hr;

    assert(pContainer); /* stand-alone volumes can't be created */

    DBG("This=%p pContainer=%p pDevice=%p pResource=%p Level=%u pDesc=%p\n",
        This, pContainer, pParams->device, pResource, Level, pDesc);

    /* Mark this as a special surface held by another internal resource. */
    pParams->container = pContainer;

    user_assert(!(pDesc->Usage & D3DUSAGE_DYNAMIC) ||
                (pDesc->Pool != D3DPOOL_MANAGED), D3DERR_INVALIDCALL);

    assert(pResource || pDesc->Pool != D3DPOOL_DEFAULT);

    hr = NineUnknown_ctor(&This->base, pParams);
    if (FAILED(hr))
        return hr;

    pipe_resource_reference(&This->resource, pResource);

    This->transfer = NULL;
    This->lock_count = 0;

    This->level = Level;
    This->level_actual = Level;
    This->desc = *pDesc;

    This->info.screen = pParams->device->screen;
    This->info.target = PIPE_TEXTURE_3D;
    This->info.width0 = pDesc->Width;
    This->info.height0 = pDesc->Height;
    This->info.depth0 = pDesc->Depth;
    This->info.last_level = 0;
    This->info.array_size = 1;
    This->info.nr_samples = 0;
    This->info.nr_storage_samples = 0;
    This->info.usage = PIPE_USAGE_DEFAULT;
    This->info.bind = PIPE_BIND_SAMPLER_VIEW;
    This->info.flags = 0;
    This->info.format = d3d9_to_pipe_format_checked(This->info.screen,
                                                    pDesc->Format,
                                                    This->info.target,
                                                    This->info.nr_samples,
                                                    This->info.bind, false,
                                                    pDesc->Pool == D3DPOOL_SCRATCH);

    if (This->info.format == PIPE_FORMAT_NONE)
        return D3DERR_DRIVERINTERNALERROR;

    This->stride = util_format_get_stride(This->info.format, pDesc->Width);
    This->stride = align(This->stride, 4);
    This->layer_stride = util_format_get_2d_size(This->info.format,
                                                 This->stride, pDesc->Height);

    /* Get true format */
    This->format_internal = d3d9_to_pipe_format_checked(This->info.screen,
                                                         pDesc->Format,
                                                         This->info.target,
                                                         This->info.nr_samples,
                                                         This->info.bind, false,
                                                         true);
    if (This->info.format != This->format_internal ||
        /* See surface9.c */
        (pParams->device->workarounds.dynamic_texture_workaround &&
         pDesc->Pool == D3DPOOL_DEFAULT && pDesc->Usage & D3DUSAGE_DYNAMIC)) {
        This->stride_internal = nine_format_get_stride(This->format_internal,
                                                         pDesc->Width);
        This->layer_stride_internal = util_format_get_2d_size(This->format_internal,
                                                                This->stride_internal,
                                                                pDesc->Height);
        This->data_internal = align_calloc(This->layer_stride_internal *
                                             This->desc.Depth, 32);
        if (!This->data_internal)
            return E_OUTOFMEMORY;
    }

    if (!This->resource) {
        hr = NineVolume9_AllocateData(This);
        if (FAILED(hr))
            return hr;
    }
    return D3D_OK;
}

static void
NineVolume9_dtor( struct NineVolume9 *This )
{
    DBG("This=%p\n", This);

    if (This->transfer) {
        struct pipe_context *pipe = nine_context_get_pipe_multithread(This->base.device);
        pipe->texture_unmap(pipe, This->transfer);
        This->transfer = NULL;
    }

    /* Note: Following condition cannot happen currently, since we
     * refcount the volume in the functions increasing
     * pending_uploads_counter. */
    if (p_atomic_read(&This->pending_uploads_counter))
        nine_csmt_process(This->base.device);

    if (This->data)
        align_free(This->data);
    if (This->data_internal)
        align_free(This->data_internal);

    pipe_resource_reference(&This->resource, NULL);

    NineUnknown_dtor(&This->base);
}

HRESULT NINE_WINAPI
NineVolume9_GetContainer( struct NineVolume9 *This,
                          REFIID riid,
                          void **ppContainer )
{
    char guid_str[64];

    DBG("This=%p riid=%p id=%s ppContainer=%p\n",
        This, riid, riid ? GUID_sprintf(guid_str, riid) : "", ppContainer);

    (void)guid_str;

    if (!NineUnknown(This)->container)
        return E_NOINTERFACE;
    return NineUnknown_QueryInterface(NineUnknown(This)->container, riid, ppContainer);
}

static inline void
NineVolume9_MarkContainerDirty( struct NineVolume9 *This )
{
    struct NineBaseTexture9 *tex;
#if defined(DEBUG) || !defined(NDEBUG)
    /* This is always contained by a NineVolumeTexture9. */
    GUID id = IID_IDirect3DVolumeTexture9;
    REFIID ref = &id;
    assert(NineUnknown_QueryInterface(This->base.container, ref, (void **)&tex)
           == S_OK);
    assert(NineUnknown_Release(NineUnknown(tex)) != 0);
#endif

    tex = NineBaseTexture9(This->base.container);
    assert(tex);
    if (This->desc.Pool == D3DPOOL_MANAGED)
        tex->managed.dirty = true;

    BASETEX_REGISTER_UPDATE(tex);
}

HRESULT NINE_WINAPI
NineVolume9_GetDesc( struct NineVolume9 *This,
                     D3DVOLUME_DESC *pDesc )
{
    user_assert(pDesc != NULL, E_POINTER);
    *pDesc = This->desc;
    return D3D_OK;
}

inline void
NineVolume9_AddDirtyRegion( struct NineVolume9 *This,
                            const struct pipe_box *box )
{
    D3DBOX dirty_region;
    struct NineVolumeTexture9 *tex = NineVolumeTexture9(This->base.container);

    if (!box) {
        NineVolumeTexture9_AddDirtyBox(tex, NULL);
    } else {
        dirty_region.Left = box->x << This->level_actual;
        dirty_region.Top = box->y << This->level_actual;
        dirty_region.Front = box->z << This->level_actual;
        dirty_region.Right = dirty_region.Left + (box->width << This->level_actual);
        dirty_region.Bottom = dirty_region.Top + (box->height << This->level_actual);
        dirty_region.Back = dirty_region.Front + (box->depth << This->level_actual);
        NineVolumeTexture9_AddDirtyBox(tex, &dirty_region);
    }
}

static inline unsigned
NineVolume9_GetSystemMemOffset(enum pipe_format format, unsigned stride,
                               unsigned layer_stride,
                               int x, int y, int z)
{
    unsigned x_offset = util_format_get_stride(format, x);

    y = util_format_get_nblocksy(format, y);

    return z * layer_stride + y * stride + x_offset;
}

HRESULT NINE_WINAPI
NineVolume9_LockBox( struct NineVolume9 *This,
                     D3DLOCKED_BOX *pLockedVolume,
                     const D3DBOX *pBox,
                     DWORD Flags )
{
    struct pipe_context *pipe;
    struct pipe_resource *resource = This->resource;
    struct pipe_box box;
    unsigned usage;

    DBG("This=%p(%p) pLockedVolume=%p pBox=%p[%u..%u,%u..%u,%u..%u] Flags=%s\n",
        This, This->base.container, pLockedVolume, pBox,
        pBox ? pBox->Left : 0, pBox ? pBox->Right : 0,
        pBox ? pBox->Top : 0, pBox ? pBox->Bottom : 0,
        pBox ? pBox->Front : 0, pBox ? pBox->Back : 0,
        nine_D3DLOCK_to_str(Flags));

    /* check if it's already locked */
    user_assert(This->lock_count == 0, D3DERR_INVALIDCALL);

    /* set pBits to NULL after lock_count check */
    user_assert(pLockedVolume, E_POINTER);
    pLockedVolume->pBits = NULL;

    user_assert(This->desc.Pool != D3DPOOL_DEFAULT ||
                (This->desc.Usage & D3DUSAGE_DYNAMIC), D3DERR_INVALIDCALL);

    user_assert(!((Flags & D3DLOCK_DISCARD) && (Flags & D3DLOCK_READONLY)),
                D3DERR_INVALIDCALL);

    if (pBox && compressed_format (This->desc.Format)) { /* For volume all pools are checked */
        const unsigned w = util_format_get_blockwidth(This->info.format);
        const unsigned h = util_format_get_blockheight(This->info.format);
        user_assert((pBox->Left == 0 && pBox->Right == This->desc.Width &&
                     pBox->Top == 0 && pBox->Bottom == This->desc.Height) ||
                    (!(pBox->Left % w) && !(pBox->Right % w) &&
                     !(pBox->Top % h) && !(pBox->Bottom % h)),
                    D3DERR_INVALIDCALL);
    }

    if (Flags & D3DLOCK_DISCARD) {
        usage = PIPE_MAP_WRITE | PIPE_MAP_DISCARD_RANGE;
    } else {
        usage = (Flags & D3DLOCK_READONLY) ?
            PIPE_MAP_READ : PIPE_MAP_READ_WRITE;
    }
    if (Flags & D3DLOCK_DONOTWAIT)
        usage |= PIPE_MAP_DONTBLOCK;

    if (pBox) {
        user_assert(pBox->Right > pBox->Left, D3DERR_INVALIDCALL);
        user_assert(pBox->Bottom > pBox->Top, D3DERR_INVALIDCALL);
        user_assert(pBox->Back > pBox->Front, D3DERR_INVALIDCALL);
        user_assert(pBox->Right <= This->desc.Width, D3DERR_INVALIDCALL);
        user_assert(pBox->Bottom <= This->desc.Height, D3DERR_INVALIDCALL);
        user_assert(pBox->Back <= This->desc.Depth, D3DERR_INVALIDCALL);

        d3dbox_to_pipe_box(&box, pBox);
        if (u_box_clip_2d(&box, &box, This->desc.Width, This->desc.Height) < 0) {
            DBG("Locked volume intersection empty.\n");
            return D3DERR_INVALIDCALL;
        }
    } else {
        u_box_3d(0, 0, 0, This->desc.Width, This->desc.Height, This->desc.Depth,
                 &box);
    }

    if (p_atomic_read(&This->pending_uploads_counter))
        nine_csmt_process(This->base.device);

    if (This->data_internal || This->data) {
        enum pipe_format format = This->info.format;
        unsigned stride = This->stride;
        unsigned layer_stride = This->layer_stride;
        uint8_t *data = This->data;
        if (This->data_internal) {
            format = This->format_internal;
            stride = This->stride_internal;
            layer_stride = This->layer_stride_internal;
            data = This->data_internal;
        }
        pLockedVolume->RowPitch = stride;
        pLockedVolume->SlicePitch = layer_stride;
        pLockedVolume->pBits = data +
            NineVolume9_GetSystemMemOffset(format, stride,
                                           layer_stride,
                                           box.x, box.y, box.z);
    } else {
        bool no_refs = !p_atomic_read(&This->base.bind) &&
            !p_atomic_read(&This->base.container->bind);
        if (no_refs)
            pipe = nine_context_get_pipe_acquire(This->base.device);
        else
            pipe = NineDevice9_GetPipe(This->base.device);
        pLockedVolume->pBits =
            pipe->texture_map(pipe, resource, This->level, usage,
                               &box, &This->transfer);
        if (no_refs)
            nine_context_get_pipe_release(This->base.device);
        if (!This->transfer) {
            if (Flags & D3DLOCK_DONOTWAIT)
                return D3DERR_WASSTILLDRAWING;
            return D3DERR_DRIVERINTERNALERROR;
        }
        pLockedVolume->RowPitch = This->transfer->stride;
        pLockedVolume->SlicePitch = This->transfer->layer_stride;
    }

    if (!(Flags & (D3DLOCK_NO_DIRTY_UPDATE | D3DLOCK_READONLY))) {
        NineVolume9_MarkContainerDirty(This);
        NineVolume9_AddDirtyRegion(This, &box);
    }

    ++This->lock_count;
    return D3D_OK;
}

HRESULT NINE_WINAPI
NineVolume9_UnlockBox( struct NineVolume9 *This )
{
    struct pipe_context *pipe;

    DBG("This=%p lock_count=%u\n", This, This->lock_count);
    user_assert(This->lock_count, D3DERR_INVALIDCALL);
    if (This->transfer) {
        pipe = nine_context_get_pipe_acquire(This->base.device);
        pipe->texture_unmap(pipe, This->transfer);
        This->transfer = NULL;
        nine_context_get_pipe_release(This->base.device);
    }
    --This->lock_count;

    if (This->data_internal) {
        struct pipe_box box;

        u_box_3d(0, 0, 0, This->desc.Width, This->desc.Height, This->desc.Depth,
                 &box);


        if (This->data) {
            (void) util_format_translate_3d(This->info.format,
                                            This->data, This->stride,
                                            This->layer_stride,
                                            0, 0, 0,
                                            This->format_internal,
                                            This->data_internal,
                                            This->stride_internal,
                                            This->layer_stride_internal,
                                            0, 0, 0,
                                            This->desc.Width, This->desc.Height,
                                            This->desc.Depth);
        } else {
            nine_context_box_upload(This->base.device,
                                    &This->pending_uploads_counter,
                                    (struct NineUnknown *)This,
                                    This->resource,
                                    This->level,
                                    &box,
                                    This->format_internal,
                                    This->data_internal,
                                    This->stride_internal,
                                    This->layer_stride_internal,
                                    &box);
        }
    }

    return D3D_OK;
}

/* When this function is called, we have already checked
 * The copy regions fit the volumes */
void
NineVolume9_CopyMemToDefault( struct NineVolume9 *This,
                              struct NineVolume9 *From,
                              unsigned dstx, unsigned dsty, unsigned dstz,
                              struct pipe_box *pSrcBox )
{
    struct pipe_resource *r_dst = This->resource;
    struct pipe_box src_box;
    struct pipe_box dst_box;

    DBG("This=%p From=%p dstx=%u dsty=%u dstz=%u pSrcBox=%p\n",
        This, From, dstx, dsty, dstz, pSrcBox);

    assert(This->desc.Pool == D3DPOOL_DEFAULT &&
           From->desc.Pool == D3DPOOL_SYSTEMMEM);

    dst_box.x = dstx;
    dst_box.y = dsty;
    dst_box.z = dstz;

    if (pSrcBox) {
        src_box = *pSrcBox;
    } else {
        src_box.x = 0;
        src_box.y = 0;
        src_box.z = 0;
        src_box.width = From->desc.Width;
        src_box.height = From->desc.Height;
        src_box.depth = From->desc.Depth;
    }

    dst_box.width = src_box.width;
    dst_box.height = src_box.height;
    dst_box.depth = src_box.depth;

    nine_context_box_upload(This->base.device,
                            &From->pending_uploads_counter,
                            (struct NineUnknown *)From,
                            r_dst,
                            This->level,
                            &dst_box,
                            From->info.format,
                            From->data, From->stride,
                            From->layer_stride,
                            &src_box);

    if (This->data_internal)
        (void) util_format_translate_3d(This->format_internal,
                                        This->data_internal,
                                        This->stride_internal,
                                        This->layer_stride_internal,
                                        dstx, dsty, dstz,
                                        From->info.format,
                                        From->data, From->stride,
                                        From->layer_stride,
                                        src_box.x, src_box.y,
                                        src_box.z,
                                        src_box.width,
                                        src_box.height,
                                        src_box.depth);

    NineVolume9_MarkContainerDirty(This);

    return;
}

HRESULT
NineVolume9_UploadSelf( struct NineVolume9 *This,
                        const struct pipe_box *damaged )
{
    struct pipe_resource *res = This->resource;
    struct pipe_box box;

    DBG("This=%p damaged=%p data=%p res=%p\n", This, damaged,
        This->data, res);

    assert(This->desc.Pool == D3DPOOL_MANAGED);
    assert(res);

    if (damaged) {
        box = *damaged;
    } else {
        box.x = 0;
        box.y = 0;
        box.z = 0;
        box.width = This->desc.Width;
        box.height = This->desc.Height;
        box.depth = This->desc.Depth;
    }

    nine_context_box_upload(This->base.device,
                            &This->pending_uploads_counter,
                            (struct NineUnknown *)This,
                            res,
                            This->level,
                            &box,
                            res->format,
                            This->data, This->stride,
                            This->layer_stride,
                            &box);

    return D3D_OK;
}


IDirect3DVolume9Vtbl NineVolume9_vtable = {
    (void *)NineUnknown_QueryInterface,
    (void *)NineUnknown_AddRef,
    (void *)NineUnknown_Release,
    (void *)NineUnknown_GetDevice, /* actually part of Volume9 iface */
    (void *)NineUnknown_SetPrivateData,
    (void *)NineUnknown_GetPrivateData,
    (void *)NineUnknown_FreePrivateData,
    (void *)NineVolume9_GetContainer,
    (void *)NineVolume9_GetDesc,
    (void *)NineVolume9_LockBox,
    (void *)NineVolume9_UnlockBox
};

static const GUID *NineVolume9_IIDs[] = {
    &IID_IDirect3DVolume9,
    &IID_IUnknown,
    NULL
};

HRESULT
NineVolume9_new( struct NineDevice9 *pDevice,
                 struct NineUnknown *pContainer,
                 struct pipe_resource *pResource,
                 unsigned Level,
                 D3DVOLUME_DESC *pDesc,
                 struct NineVolume9 **ppOut )
{
    NINE_DEVICE_CHILD_NEW(Volume9, ppOut, pDevice, /* args */
                          pContainer, pResource, Level, pDesc);
}
