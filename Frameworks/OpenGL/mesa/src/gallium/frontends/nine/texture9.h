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

#ifndef _NINE_TEXTURE9_H_
#define _NINE_TEXTURE9_H_

#include "basetexture9.h"
#include "nine_memory_helper.h"
#include "surface9.h"

struct NineTexture9
{
    struct NineBaseTexture9 base;
    struct NineSurface9 **surfaces;
    struct pipe_box dirty_rect; /* covers all mip levels */
    struct nine_allocation *managed_buffer;
};
static inline struct NineTexture9 *
NineTexture9( void *data )
{
    return (struct NineTexture9 *)data;
}

HRESULT
NineTexture9_new( struct NineDevice9 *pDevice,
                  UINT Width, UINT Height, UINT Levels,
                  DWORD Usage,
                  D3DFORMAT Format,
                  D3DPOOL Pool,
                  struct NineTexture9 **ppOut,
                  HANDLE *pSharedHandle );

HRESULT NINE_WINAPI
NineTexture9_GetLevelDesc( struct NineTexture9 *This,
                           UINT Level,
                           D3DSURFACE_DESC *pDesc );

HRESULT NINE_WINAPI
NineTexture9_GetSurfaceLevel( struct NineTexture9 *This,
                              UINT Level,
                              IDirect3DSurface9 **ppSurfaceLevel );

HRESULT NINE_WINAPI
NineTexture9_LockRect( struct NineTexture9 *This,
                       UINT Level,
                       D3DLOCKED_RECT *pLockedRect,
                       const RECT *pRect,
                       DWORD Flags );

HRESULT NINE_WINAPI
NineTexture9_UnlockRect( struct NineTexture9 *This,
                         UINT Level );

HRESULT NINE_WINAPI
NineTexture9_AddDirtyRect( struct NineTexture9 *This,
                           const RECT *pDirtyRect );

#endif /* _NINE_TEXTURE9_H_ */
