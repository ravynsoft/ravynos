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

#ifndef _D3DADAPTER9_H_
#define _D3DADAPTER9_H_

#include "present.h"

#ifndef __cplusplus

/* Representation of an adapter group, although since this is implemented by
 * the driver, it knows nothing about the windowing system it's on */
typedef struct ID3DAdapter9Vtbl
{
    /* IUnknown */
    HRESULT (WINAPI *QueryInterface)(ID3DAdapter9 *This, REFIID riid, void **ppvObject);
    ULONG (WINAPI *AddRef)(ID3DAdapter9 *This);
    ULONG (WINAPI *Release)(ID3DAdapter9 *This);

    /* ID3DAdapter9 */
    HRESULT (WINAPI *GetAdapterIdentifier)(ID3DAdapter9 *This, DWORD Flags, D3DADAPTER_IDENTIFIER9 *pIdentifier);
    HRESULT (WINAPI *CheckDeviceType)(ID3DAdapter9 *This, D3DDEVTYPE DevType, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, BOOL bWindowed);
    HRESULT (WINAPI *CheckDeviceFormat)(ID3DAdapter9 *This, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat);
    HRESULT (WINAPI *CheckDeviceMultiSampleType)(ID3DAdapter9 *This, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD *pQualityLevels);
    HRESULT (WINAPI *CheckDepthStencilMatch)(ID3DAdapter9 *This, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat);
    HRESULT (WINAPI *CheckDeviceFormatConversion)(ID3DAdapter9 *This, D3DDEVTYPE DeviceType, D3DFORMAT SourceFormat, D3DFORMAT TargetFormat);
    HRESULT (WINAPI *GetDeviceCaps)(ID3DAdapter9 *This, D3DDEVTYPE DeviceType, D3DCAPS9 *pCaps);
    HRESULT (WINAPI *CreateDevice)(ID3DAdapter9 *This, UINT RealAdapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS *pPresentationParameters, IDirect3D9 *pD3D9, ID3DPresentGroup *pPresentationFactory, IDirect3DDevice9 **ppReturnedDeviceInterface);
    HRESULT (WINAPI *CreateDeviceEx)(ID3DAdapter9 *This, UINT RealAdapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS *pPresentationParameters, D3DDISPLAYMODEEX *pFullscreenDisplayMode, IDirect3D9Ex *pD3D9Ex, ID3DPresentGroup *pPresentationFactory, IDirect3DDevice9Ex **ppReturnedDeviceInterface);
} ID3DAdapter9Vtbl;

struct ID3DAdapter9
{
    ID3DAdapter9Vtbl *lpVtbl;
};

/* IUnknown macros */
#define ID3DAdapter9_QueryInterface(p,a,b) (p)->lpVtbl->QueryInterface(p,a,b)
#define ID3DAdapter9_AddRef(p) (p)->lpVtbl->AddRef(p)
#define ID3DAdapter9_Release(p) (p)->lpVtbl->Release(p)
/* ID3DAdapter9 macros */
#define ID3DAdapter9_GetAdapterIdentifier(p,a,b) (p)->lpVtbl->GetAdapterIdentifier(p,a,b)
#define ID3DAdapter9_CheckDeviceType(p,a,b,c,d) (p)->lpVtbl->CheckDeviceType(p,a,b,c,d)
#define ID3DAdapter9_CheckDeviceFormat(p,a,b,c,d,e) (p)->lpVtbl->CheckDeviceFormat(p,a,b,c,d,e)
#define ID3DAdapter9_CheckDeviceMultiSampleType(p,a,b,c,d,e) (p)->lpVtbl->CheckDeviceMultiSampleType(p,a,b,c,d,e)
#define ID3DAdapter9_CheckDepthStencilMatch(p,a,b,c,d) (p)->lpVtbl->CheckDepthStencilMatch(p,a,b,c,d)
#define ID3DAdapter9_CheckDeviceFormatConversion(p,a,b,c) (p)->lpVtbl->CheckDeviceFormatConversion(p,a,b,c)
#define ID3DAdapter9_GetDeviceCaps(p,a,b) (p)->lpVtbl->GetDeviceCaps(p,a,b)
#define ID3DAdapter9_CreateDevice(p,a,b,c,d,e,f,g,h) (p)->lpVtbl->CreateDevice(p,a,b,c,d,e,f,g,h)
#define ID3DAdapter9_CreateDeviceEx(p,a,b,c,d,e,f,g,h,i) (p)->lpVtbl->CreateDeviceEx(p,a,b,c,d,e,f,g,h,i)

#else /* __cplusplus */

struct ID3DAdapter9 : public IUnknown
{
    HRESULT WINAPI GetAdapterIdentifier(DWORD Flags, D3DADAPTER_IDENTIFIER9 *pIdentifier);
    HRESULT WINAPI CheckDeviceType(D3DDEVTYPE DevType, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, BOOL bWindowed);
    HRESULT WINAPI CheckDeviceFormat(D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat);
    HRESULT WINAPI CheckDeviceMultiSampleType(D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD *pQualityLevels);
    HRESULT WINAPI CheckDepthStencilMatch(D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat);
    HRESULT WINAPI CheckDeviceFormatConversion(D3DDEVTYPE DeviceType, D3DFORMAT SourceFormat, D3DFORMAT TargetFormat);
    HRESULT WINAPI GetDeviceCaps(D3DDEVTYPE DeviceType, D3DCAPS9 *pCaps);
    HRESULT WINAPI CreateDevice(UINT RealAdapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS *pPresentationParameters, IDirect3D9 *pD3D9, ID3DPresentGroup *pPresentationFactory, IDirect3DDevice9 **ppReturnedDeviceInterface);
    HRESULT WINAPI CreateDeviceEx(UINT RealAdapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS *pPresentationParameters, D3DDISPLAYMODEEX *pFullscreenDisplayMode, IDirect3D9Ex *pD3D9Ex, ID3DPresentGroup *pPresentationFactory, IDirect3DDevice9Ex **ppReturnedDeviceInterface);
};

#endif /* __cplusplus */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* acquire a const struct D3DAdapter9* structure describing the interface
 * queried. See  */
const void * WINAPI
D3DAdapter9GetProc( const char *name );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _D3DADAPTER9_H_ */
