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

#ifndef _NINE_VOLUMETEXTURE9_H_
#define _NINE_VOLUMETEXTURE9_H_

#include "basetexture9.h"
#include "volume9.h"

struct NineVolumeTexture9
{
    struct NineBaseTexture9 base;
    struct NineVolume9 **volumes;
    struct pipe_box dirty_box;
};
static inline struct NineVolumeTexture9 *
NineVolumeTexture9( void *data )
{
    return (struct NineVolumeTexture9 *)data;
}

HRESULT
NineVolumeTexture9_new( struct NineDevice9 *pDevice,
                        UINT Width, UINT Height, UINT Depth, UINT Levels,
                        DWORD Usage,
                        D3DFORMAT Format,
                        D3DPOOL Pool,
                        struct NineVolumeTexture9 **ppOut,
                        HANDLE *pSharedHandle );

HRESULT NINE_WINAPI
NineVolumeTexture9_GetLevelDesc( struct NineVolumeTexture9 *This,
                                 UINT Level,
                                 D3DVOLUME_DESC *pDesc );

HRESULT NINE_WINAPI
NineVolumeTexture9_GetVolumeLevel( struct NineVolumeTexture9 *This,
                                   UINT Level,
                                   IDirect3DVolume9 **ppVolumeLevel );

HRESULT NINE_WINAPI
NineVolumeTexture9_LockBox( struct NineVolumeTexture9 *This,
                            UINT Level,
                            D3DLOCKED_BOX *pLockedVolume,
                            const D3DBOX *pBox,
                            DWORD Flags );

HRESULT NINE_WINAPI
NineVolumeTexture9_UnlockBox( struct NineVolumeTexture9 *This,
                              UINT Level );

HRESULT NINE_WINAPI
NineVolumeTexture9_AddDirtyBox( struct NineVolumeTexture9 *This,
                                const D3DBOX *pDirtyBox );

#endif /* _NINE_VOLUMETEXTURE9_H_ */
