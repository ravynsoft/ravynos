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

#ifndef _D3DADAPTER_PRESENT_H_
#define _D3DADAPTER_PRESENT_H_

#include <d3d9.h>

#ifndef D3DOK_WINDOW_OCCLUDED
#define D3DOK_WINDOW_OCCLUDED MAKE_D3DSTATUS(2531)
#endif /* D3DOK_WINDOW_OCCLUDED */

#ifndef __cplusplus
typedef struct ID3DPresent ID3DPresent;
typedef struct ID3DPresentGroup ID3DPresentGroup;
typedef struct ID3DAdapter9 ID3DAdapter9;
typedef struct D3DWindowBuffer D3DWindowBuffer;

/* Available since version 1.3 */
typedef struct _D3DPRESENT_PARAMETERS2_ {
    /* Whether D3DSWAPEFFECT_DISCARD is allowed to release the
     * D3DWindowBuffers in any order, and eventually with a delay.
     * FALSE (Default): buffers should be released as soon as possible.
     * TRUE: it is allowed to release some buffers with a delay, and in
     * a random order. */
    BOOL AllowDISCARDDelayedRelease;
    /* User preference for D3DSWAPEFFECT_DISCARD with D3DPRESENT_INTERVAL_IMMEDIATE.
     * FALSE (Default): User prefers presentation to occur as soon as possible,
     * with potential tearings.
     * TRUE: User prefers presentation to be tear free. Requires
     * AllowDISCARDDelayedRelease to have any effect. */
    BOOL TearFreeDISCARD;
} D3DPRESENT_PARAMETERS2, *PD3DPRESENT_PARAMETERS2, *LPD3DPRESENT_PARAMETERS2;

/* Presentation backend for drivers to display their brilliant work */
typedef struct ID3DPresentVtbl
{
    /* IUnknown */
    HRESULT (WINAPI *QueryInterface)(ID3DPresent *This, REFIID riid, void **ppvObject);
    ULONG (WINAPI *AddRef)(ID3DPresent *This);
    ULONG (WINAPI *Release)(ID3DPresent *This);

    /* ID3DPresent */
    /* This function initializes the screen and window provided at creation.
     * Hence why this should always be called as the one of first things a new
     * swap chain does */
    HRESULT (WINAPI *SetPresentParameters)(ID3DPresent *This, D3DPRESENT_PARAMETERS *pPresentationParameters, D3DDISPLAYMODEEX *pFullscreenDisplayMode);
    /* Make a buffer visible to the window system via dma-buf fd.
     * For better compatibility, it must be 32bpp and format ARGB/XRGB */
    HRESULT (WINAPI *NewD3DWindowBufferFromDmaBuf)(ID3DPresent *This, int dmaBufFd, int width, int height, int stride, int depth, int bpp, D3DWindowBuffer **out);
    HRESULT (WINAPI *DestroyD3DWindowBuffer)(ID3DPresent *This, D3DWindowBuffer *buffer);
    /* After presenting a buffer to the window system, the buffer
     * may be used as is (no copy of the content) by the window system.
     * You must not use a non-released buffer, else the user may see undefined content.
     * Note: This function waits as well that the buffer content was displayed (this
     * can be after the release of the buffer if the window system decided to make
     * an internal copy and release early. */
    HRESULT (WINAPI *WaitBufferReleased)(ID3DPresent *This, D3DWindowBuffer *buffer);
    HRESULT (WINAPI *FrontBufferCopy)(ID3DPresent *This, D3DWindowBuffer *buffer);
    /* It is possible to do partial copy, but impossible to do resizing, which must
     * be done by the client after checking the front buffer size */
    HRESULT (WINAPI *PresentBuffer)(ID3DPresent *This, D3DWindowBuffer *buffer, HWND hWndOverride, const RECT *pSourceRect, const RECT *pDestRect, const RGNDATA *pDirtyRegion, DWORD Flags);
    HRESULT (WINAPI *GetRasterStatus)(ID3DPresent *This, D3DRASTER_STATUS *pRasterStatus);
    HRESULT (WINAPI *GetDisplayMode)(ID3DPresent *This, D3DDISPLAYMODEEX *pMode, D3DDISPLAYROTATION *pRotation);
    HRESULT (WINAPI *GetPresentStats)(ID3DPresent *This, D3DPRESENTSTATS *pStats);
    HRESULT (WINAPI *GetCursorPos)(ID3DPresent *This, POINT *pPoint);
    HRESULT (WINAPI *SetCursorPos)(ID3DPresent *This, POINT *pPoint);
    /* Cursor size is always 32x32. pBitmap and pHotspot can be NULL. */
    HRESULT (WINAPI *SetCursor)(ID3DPresent *This, void *pBitmap, POINT *pHotspot, BOOL bShow);
    HRESULT (WINAPI *SetGammaRamp)(ID3DPresent *This, const D3DGAMMARAMP *pRamp, HWND hWndOverride);
    HRESULT (WINAPI *GetWindowInfo)(ID3DPresent *This,  HWND hWnd, int *width, int *height, int *depth);
    /* Available since version 1.1 */
    BOOL (WINAPI *GetWindowOccluded)(ID3DPresent *This);
    /* Available since version 1.2 */
    BOOL (WINAPI *ResolutionMismatch)(ID3DPresent *This);
    HANDLE (WINAPI *CreateThread)(ID3DPresent *This, void *pThreadfunc, void *pParam);
    BOOL (WINAPI *WaitForThread)(ID3DPresent *This, HANDLE thread);
    /* Available since version 1.3 */
    HRESULT (WINAPI *SetPresentParameters2)(ID3DPresent *This, D3DPRESENT_PARAMETERS2 *pParameters);
    BOOL (WINAPI *IsBufferReleased)(ID3DPresent *This, D3DWindowBuffer *buffer);
    /* Wait a buffer gets released. */
    HRESULT (WINAPI *WaitBufferReleaseEvent)(ID3DPresent *This);
} ID3DPresentVtbl;

struct ID3DPresent
{
    ID3DPresentVtbl *lpVtbl;
};

/* IUnknown macros */
#define ID3DPresent_QueryInterface(p,a,b) (p)->lpVtbl->QueryInterface(p,a,b)
#define ID3DPresent_AddRef(p) (p)->lpVtbl->AddRef(p)
#define ID3DPresent_Release(p) (p)->lpVtbl->Release(p)
/* ID3DPresent macros */
#define ID3DPresent_GetPresentParameters(p,a) (p)->lpVtbl->GetPresentParameters(p,a)
#define ID3DPresent_SetPresentParameters(p,a,b) (p)->lpVtbl->SetPresentParameters(p,a,b)
#define ID3DPresent_NewD3DWindowBufferFromDmaBuf(p,a,b,c,d,e,f,g) (p)->lpVtbl->NewD3DWindowBufferFromDmaBuf(p,a,b,c,d,e,f,g)
#define ID3DPresent_DestroyD3DWindowBuffer(p,a) (p)->lpVtbl->DestroyD3DWindowBuffer(p,a)
#define ID3DPresent_WaitBufferReleased(p,a) (p)->lpVtbl->WaitBufferReleased(p,a)
#define ID3DPresent_FrontBufferCopy(p,a) (p)->lpVtbl->FrontBufferCopy(p,a)
#define ID3DPresent_PresentBuffer(p,a,b,c,d,e,f) (p)->lpVtbl->PresentBuffer(p,a,b,c,d,e,f)
#define ID3DPresent_GetRasterStatus(p,a) (p)->lpVtbl->GetRasterStatus(p,a)
#define ID3DPresent_GetDisplayMode(p,a,b) (p)->lpVtbl->GetDisplayMode(p,a,b)
#define ID3DPresent_GetPresentStats(p,a) (p)->lpVtbl->GetPresentStats(p,a)
#define ID3DPresent_GetCursorPos(p,a) (p)->lpVtbl->GetCursorPos(p,a)
#define ID3DPresent_SetCursorPos(p,a) (p)->lpVtbl->SetCursorPos(p,a)
#define ID3DPresent_SetCursor(p,a,b,c) (p)->lpVtbl->SetCursor(p,a,b,c)
#define ID3DPresent_SetGammaRamp(p,a,b) (p)->lpVtbl->SetGammaRamp(p,a,b)
#define ID3DPresent_GetWindowInfo(p,a,b,c,d) (p)->lpVtbl->GetWindowInfo(p,a,b,c,d)
#define ID3DPresent_GetWindowOccluded(p) (p)->lpVtbl->GetWindowOccluded(p)
#define ID3DPresent_ResolutionMismatch(p) (p)->lpVtbl->ResolutionMismatch(p)
#define ID3DPresent_CreateThread(p,a,b) (p)->lpVtbl->CreateThread(p,a,b)
#define ID3DPresent_WaitForThread(p,a) (p)->lpVtbl->WaitForThread(p,a)
#define ID3DPresent_SetPresentParameters2(p,a) (p)->lpVtbl->SetPresentParameters2(p,a)
#define ID3DPresent_IsBufferReleased(p,a) (p)->lpVtbl->IsBufferReleased(p,a)
#define ID3DPresent_WaitBufferReleaseEvent(p) (p)->lpVtbl->WaitBufferReleaseEvent(p)

typedef struct ID3DPresentGroupVtbl
{
    /* IUnknown */
    HRESULT (WINAPI *QueryInterface)(ID3DPresentGroup *This, REFIID riid, void **ppvObject);
    ULONG (WINAPI *AddRef)(ID3DPresentGroup *This);
    ULONG (WINAPI *Release)(ID3DPresentGroup *This);

    /* ID3DPresentGroup */
    /* When creating a device, it's relevant for the driver to know how many
     * implicit swap chains to create. It has to create one per monitor in a
     * multi-monitor setup */
    UINT (WINAPI *GetMultiheadCount)(ID3DPresentGroup *This);
    /* returns only the implicit present interfaces */
    HRESULT (WINAPI *GetPresent)(ID3DPresentGroup *This, UINT Index, ID3DPresent **ppPresent);
    /* used to create additional presentation interfaces along the way */
    HRESULT (WINAPI *CreateAdditionalPresent)(ID3DPresentGroup *This, D3DPRESENT_PARAMETERS *pPresentationParameters, ID3DPresent **ppPresent);
    void (WINAPI *GetVersion) (ID3DPresentGroup *This, int *major, int *minor);
} ID3DPresentGroupVtbl;

struct ID3DPresentGroup
{
    ID3DPresentGroupVtbl *lpVtbl;
};

/* IUnknown macros */
#define ID3DPresentGroup_QueryInterface(p,a,b) (p)->lpVtbl->QueryInterface(p,a,b)
#define ID3DPresentGroup_AddRef(p) (p)->lpVtbl->AddRef(p)
#define ID3DPresentGroup_Release(p) (p)->lpVtbl->Release(p)
/* ID3DPresentGroup */
#define ID3DPresentGroup_GetMultiheadCount(p) (p)->lpVtbl->GetMultiheadCount(p)
#define ID3DPresentGroup_GetPresent(p,a,b) (p)->lpVtbl->GetPresent(p,a,b)
#define ID3DPresentGroup_CreateAdditionalPresent(p,a,b) (p)->lpVtbl->CreateAdditionalPresent(p,a,b)
#define ID3DPresentGroup_GetVersion(p,a,b) (p)->lpVtbl->GetVersion(p,a,b)

#endif /* __cplusplus */

#endif /* _D3DADAPTER_PRESENT_H_ */
