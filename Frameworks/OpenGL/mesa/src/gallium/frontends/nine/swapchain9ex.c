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

#include "swapchain9ex.h"
#include "device9.h"

#include "nine_helpers.h"

#define DBG_CHANNEL DBG_SWAPCHAIN

static HRESULT
NineSwapChain9Ex_ctor( struct NineSwapChain9Ex *This,
                       struct NineUnknownParams *pParams,
                       BOOL implicit,
                       ID3DPresent *pPresent,
                       D3DPRESENT_PARAMETERS *pPresentationParameters,
                       struct d3dadapter9_context *pCTX,
                       HWND hFocusWindow,
                       D3DDISPLAYMODEEX *mode )
{
    DBG("This=%p pParams=%p implicit=%d pPresent=%p pPresentationParameters=%p "
        "pCTX=%p hFocusWindow=%p mode=%p",
        This, pParams, (int) implicit, pPresent, pPresentationParameters, pCTX, hFocusWindow, mode);

    return NineSwapChain9_ctor(&This->base, pParams, implicit, pPresent,
                               pPresentationParameters, pCTX, hFocusWindow, mode);
}

static void
NineSwapChain9Ex_dtor( struct NineSwapChain9Ex *This )
{
    NineSwapChain9_dtor(&This->base);
}

HRESULT NINE_WINAPI
NineSwapChain9Ex_GetLastPresentCount( struct NineSwapChain9Ex *This,
                                      UINT *pLastPresentCount )
{
    STUB(D3DERR_INVALIDCALL);
}

HRESULT NINE_WINAPI
NineSwapChain9Ex_GetPresentStats( struct NineSwapChain9Ex *This,
                                  D3DPRESENTSTATS *pPresentationStatistics )
{
    STUB(D3DERR_INVALIDCALL);
}

HRESULT NINE_WINAPI
NineSwapChain9Ex_GetDisplayModeEx( struct NineSwapChain9Ex *This,
                                   D3DDISPLAYMODEEX *pMode,
                                   D3DDISPLAYROTATION *pRotation )
{
    D3DDISPLAYROTATION rot;

    user_assert(pMode != NULL, E_POINTER);
    if (!pRotation) { pRotation = &rot; }

    return ID3DPresent_GetDisplayMode(This->base.present, pMode, pRotation);
}

IDirect3DSwapChain9ExVtbl NineSwapChain9Ex_vtable = {
    (void *)NineUnknown_QueryInterface,
    (void *)NineUnknown_AddRef,
    (void *)NineUnknown_Release,
    (void *)NineSwapChain9_Present,
    (void *)NineSwapChain9_GetFrontBufferData,
    (void *)NineSwapChain9_GetBackBuffer,
    (void *)NineSwapChain9_GetRasterStatus,
    (void *)NineSwapChain9_GetDisplayMode,
    (void *)NineUnknown_GetDevice, /* actually part of NineSwapChain9 iface */
    (void *)NineSwapChain9_GetPresentParameters,
    (void *)NineSwapChain9Ex_GetLastPresentCount,
    (void *)NineSwapChain9Ex_GetPresentStats,
    (void *)NineSwapChain9Ex_GetDisplayModeEx
};

static const GUID *NineSwapChain9Ex_IIDs[] = {
    &IID_IDirect3DSwapChain9Ex,
    &IID_IDirect3DSwapChain9,
    &IID_IUnknown,
    NULL
};

HRESULT
NineSwapChain9Ex_new( struct NineDevice9 *pDevice,
                      BOOL implicit,
                      ID3DPresent *pPresent,
                      D3DPRESENT_PARAMETERS *pPresentationParameters,
                      struct d3dadapter9_context *pCTX,
                      HWND hFocusWindow,
                      D3DDISPLAYMODEEX *mode,
                      struct NineSwapChain9Ex **ppOut )
{
    NINE_DEVICE_CHILD_NEW(SwapChain9Ex, ppOut, pDevice, /* args */
                          implicit, pPresent, pPresentationParameters,
                          pCTX, hFocusWindow, mode);
}
