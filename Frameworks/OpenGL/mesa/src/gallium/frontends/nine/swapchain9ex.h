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

#ifndef _NINE_SWAPCHAIN9EX_H_
#define _NINE_SWAPCHAIN9EX_H_

#include "swapchain9.h"

struct NineSwapChain9Ex
{
    struct NineSwapChain9 base;
};
static inline struct NineSwapChain9Ex *
NineSwapChain9Ex( void *data )
{
    return (struct NineSwapChain9Ex *)data;
}

HRESULT
NineSwapChain9Ex_new( struct NineDevice9 *pDevice,
                      BOOL implicit,
                      ID3DPresent *pPresent,
                      D3DPRESENT_PARAMETERS *pPresentationParameters,
                      struct d3dadapter9_context *pCTX,
                      HWND hFocusWindow,
                      D3DDISPLAYMODEEX *mode,
                      struct NineSwapChain9Ex **ppOut );

HRESULT NINE_WINAPI
NineSwapChain9Ex_GetLastPresentCount( struct NineSwapChain9Ex *This,
                                      UINT *pLastPresentCount );

HRESULT NINE_WINAPI
NineSwapChain9Ex_GetPresentStats( struct NineSwapChain9Ex *This,
                                  D3DPRESENTSTATS *pPresentationStatistics );

HRESULT NINE_WINAPI
NineSwapChain9Ex_GetDisplayModeEx( struct NineSwapChain9Ex *This,
                                   D3DDISPLAYMODEEX *pMode,
                                   D3DDISPLAYROTATION *pRotation );

#endif /* _NINE_SWAPCHAIN9EX_H_ */
