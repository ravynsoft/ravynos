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

#ifndef _NINE_VOLUME9_H_
#define _NINE_VOLUME9_H_

#include "iunknown.h"

#include "pipe/p_state.h"
#include "util/u_inlines.h"

struct hash_table;

struct NineDevice9;

struct NineVolume9
{
    struct NineUnknown base;

    struct pipe_resource *resource;
    unsigned level;
    unsigned level_actual;

    uint8_t *data; /* system memory backing */
    uint8_t *data_internal; /* for conversions */

    D3DVOLUME_DESC desc;
    struct pipe_resource info;
    enum pipe_format format_internal;
    unsigned stride;
    unsigned stride_internal;
    unsigned layer_stride;
    unsigned layer_stride_internal;

    struct pipe_transfer *transfer;
    unsigned lock_count;

    unsigned pending_uploads_counter; /* pending uploads */
};
static inline struct NineVolume9 *
NineVolume9( void *data )
{
    return (struct NineVolume9 *)data;
}

HRESULT
NineVolume9_new( struct NineDevice9 *pDevice,
                 struct NineUnknown *pContainer,
                 struct pipe_resource *pResource,
                 unsigned Level,
                 D3DVOLUME_DESC *pDesc,
                 struct NineVolume9 **ppOut );

/*** Nine private ***/

static inline void
NineVolume9_SetResource( struct NineVolume9 *This,
                         struct pipe_resource *resource, unsigned level )
{
    This->level = level;
    pipe_resource_reference(&This->resource, resource);
}

void
NineVolume9_AddDirtyRegion( struct NineVolume9 *This,
                            const struct pipe_box *box );

void
NineVolume9_CopyMemToDefault( struct NineVolume9 *This,
                              struct NineVolume9 *From,
                              unsigned dstx, unsigned dsty, unsigned dstz,
                              struct pipe_box *pSrcBox );

HRESULT
NineVolume9_UploadSelf( struct NineVolume9 *This,
                        const struct pipe_box *damaged );


/*** Direct3D public ***/

HRESULT NINE_WINAPI
NineVolume9_GetContainer( struct NineVolume9 *This,
                          REFIID riid,
                          void **ppContainer );

HRESULT NINE_WINAPI
NineVolume9_GetDesc( struct NineVolume9 *This,
                     D3DVOLUME_DESC *pDesc );

HRESULT NINE_WINAPI
NineVolume9_LockBox( struct NineVolume9 *This,
                     D3DLOCKED_BOX *pLockedVolume,
                     const D3DBOX *pBox,
                     DWORD Flags );

HRESULT NINE_WINAPI
NineVolume9_UnlockBox( struct NineVolume9 *This );

#endif /* _NINE_VOLUME9_H_ */
