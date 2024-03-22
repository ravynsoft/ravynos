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

#ifndef _NINE_SWAPCHAIN9_H_
#define _NINE_SWAPCHAIN9_H_

#include "iunknown.h"
#include "adapter9.h"

#include "d3dadapter/d3dadapter9.h"

#include "threadpool.h"

struct NineDevice9;
struct NineSurface9;
struct nine_winsys_swapchain;
struct blit_state;

#define DRI_SWAP_FENCES_MAX 4
#define DRI_SWAP_FENCES_MASK 3

struct NineSwapChain9
{
    struct NineUnknown base;

    /* G3D stuff */
    struct pipe_screen *screen;

    /* presentation backend */
    ID3DPresent *present;
    D3DPRESENT_PARAMETERS params;
    D3DDISPLAYMODEEX *mode;
    struct d3dadapter9_context *actx;
    BOOL implicit;
    unsigned num_back_buffers;

    /* buffer handles */
    struct NineSurface9 *buffers[D3DPRESENT_BACK_BUFFERS_MAX_EX + 1]; /* 0 to BackBufferCount-1 : the back buffers. BackBufferCount : additional buffer */
    struct pipe_resource *present_buffers[D3DPRESENT_BACK_BUFFERS_MAX_EX + 1];
    D3DWindowBuffer *present_handles[D3DPRESENT_BACK_BUFFERS_MAX_EX + 1];
    D3DWindowBuffer *present_handles_pending_release[D3DPRESENT_BACK_BUFFERS_MAX_EX + 1];

    struct pipe_fence_handle *swap_fences[DRI_SWAP_FENCES_MAX];
    unsigned int cur_fences;
    unsigned int head;
    unsigned int tail;
    unsigned int desired_fences;

    BOOL rendering_done;

    struct NineSurface9 *zsbuf;

    D3DGAMMARAMP gamma;

    struct threadpool *pool;
    struct threadpool_task *tasks[D3DPRESENT_BACK_BUFFERS_MAX_EX + 1];
    BOOL *pending_presentation[D3DPRESENT_BACK_BUFFERS_MAX_EX + 1];
    BOOL enable_threadpool;
};

static inline struct NineSwapChain9 *
NineSwapChain9( void *data )
{
    return (struct NineSwapChain9 *)data;
}

HRESULT
NineSwapChain9_new( struct NineDevice9 *pDevice,
                    BOOL implicit,
                    ID3DPresent *pPresent,
                    D3DPRESENT_PARAMETERS *pPresentationParameters,
                    struct d3dadapter9_context *pCTX,
                    HWND hFocusWindow,
                    struct NineSwapChain9 **ppOut );

HRESULT
NineSwapChain9_ctor( struct NineSwapChain9 *This,
                     struct NineUnknownParams *pParams,
                     BOOL implicit,
                     ID3DPresent *pPresent,
                     D3DPRESENT_PARAMETERS *pPresentationParameters,
                     struct d3dadapter9_context *pCTX,
                     HWND hFocusWindow,
                     D3DDISPLAYMODEEX *mode );

void
NineSwapChain9_dtor( struct NineSwapChain9 *This );

HRESULT
NineSwapChain9_Resize( struct NineSwapChain9 *This,
                       D3DPRESENT_PARAMETERS *pParams,
                       D3DDISPLAYMODEEX *mode );

HRESULT NINE_WINAPI
NineSwapChain9_Present( struct NineSwapChain9 *This,
                        const RECT *pSourceRect,
                        const RECT *pDestRect,
                        HWND hDestWindowOverride,
                        const RGNDATA *pDirtyRegion,
                        DWORD dwFlags );

HRESULT NINE_WINAPI
NineSwapChain9_GetFrontBufferData( struct NineSwapChain9 *This,
                                   IDirect3DSurface9 *pDestSurface );

HRESULT NINE_WINAPI
NineSwapChain9_GetBackBuffer( struct NineSwapChain9 *This,
                              UINT iBackBuffer,
                              D3DBACKBUFFER_TYPE Type,
                              IDirect3DSurface9 **ppBackBuffer );

HRESULT NINE_WINAPI
NineSwapChain9_GetRasterStatus( struct NineSwapChain9 *This,
                                D3DRASTER_STATUS *pRasterStatus );

HRESULT NINE_WINAPI
NineSwapChain9_GetDisplayMode( struct NineSwapChain9 *This,
                               D3DDISPLAYMODE *pMode );

HRESULT NINE_WINAPI
NineSwapChain9_GetPresentParameters( struct NineSwapChain9 *This,
                                     D3DPRESENT_PARAMETERS *pPresentationParameters );

BOOL
NineSwapChain9_GetOccluded( struct NineSwapChain9 *This );

BOOL
NineSwapChain9_ResolutionMismatch( struct NineSwapChain9 *This );

HANDLE
NineSwapChain9_CreateThread( struct NineSwapChain9 *This,
                                 void *pFuncAddress,
                                 void *pParam );

void
NineSwapChain9_WaitForThread( struct NineSwapChain9 *This,
                                  HANDLE thread );

#endif /* _NINE_SWAPCHAIN9_H_ */
