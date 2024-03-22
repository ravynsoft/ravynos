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
#include "device9ex.h"
#include "nine_pipe.h"
#include "swapchain9ex.h"

#include "nine_helpers.h"

#include "util/macros.h"

#define DBG_CHANNEL DBG_DEVICE

static HRESULT
NineDevice9Ex_ctor( struct NineDevice9Ex *This,
                    struct NineUnknownParams *pParams,
                    struct pipe_screen *pScreen,
                    D3DDEVICE_CREATION_PARAMETERS *pCreationParameters,
                    D3DCAPS9 *pCaps,
                    D3DPRESENT_PARAMETERS *pPresentationParameters,
                    D3DDISPLAYMODEEX *pFullscreenDisplayMode,
                    IDirect3D9Ex *pD3D9Ex,
                    ID3DPresentGroup *pPresentationGroup,
                    struct d3dadapter9_context *pCTX,
                    int minorVersionNum )
{
    DBG("This=%p pParams=%p pScreen=%p pCreationParameters=%p pCaps=%p "
        "pPresentationParameters=%p pFullscreenDisplayMode=%p "
        "pD3D9Ex=%p pPresentationGroup=%p pCTX=%p\n",
        This, pParams, pScreen, pCreationParameters, pCaps,
        pPresentationParameters, pFullscreenDisplayMode,
        pD3D9Ex, pPresentationGroup, pCTX);

    return NineDevice9_ctor(&This->base, pParams,
                            pScreen, pCreationParameters, pCaps,
                            pPresentationParameters,
                            (IDirect3D9 *)pD3D9Ex, pPresentationGroup, pCTX,
                            true, pFullscreenDisplayMode, minorVersionNum);
}

static void
NineDevice9Ex_dtor( struct NineDevice9Ex *This )
{
    NineDevice9_dtor(&This->base);
}

HRESULT NINE_WINAPI
NineDevice9Ex_SetConvolutionMonoKernel( UNUSED struct NineDevice9Ex *This,
                                        UNUSED UINT width,
                                        UNUSED UINT height,
                                        UNUSED float *rows,
                                        UNUSED float *columns )
{
    DBG("This\n");
    STUB(D3D_OK);
}

HRESULT NINE_WINAPI
NineDevice9Ex_ComposeRects( UNUSED struct NineDevice9Ex *This,
                            UNUSED IDirect3DSurface9 *pSrc,
                            UNUSED IDirect3DSurface9 *pDst,
                            UNUSED IDirect3DVertexBuffer9 *pSrcRectDescs,
                            UNUSED UINT NumRects,
                            UNUSED IDirect3DVertexBuffer9 *pDstRectDescs,
                            UNUSED D3DCOMPOSERECTSOP Operation,
                            UNUSED int Xoffset,
                            UNUSED int Yoffset )
{
    DBG("This\n");
    STUB(D3D_OK);
}

HRESULT NINE_WINAPI
NineDevice9Ex_PresentEx( struct NineDevice9Ex *This,
                         const RECT *pSourceRect,
                         const RECT *pDestRect,
                         HWND hDestWindowOverride,
                         const RGNDATA *pDirtyRegion,
                         DWORD dwFlags )
{
    unsigned i;
    HRESULT hr;

    DBG("This=%p pSourceRect=%p pDestRect=%p hDestWindowOverride=%p "
        "pDirtyRegion=%p dwFlags=%d\n",
        This, pSourceRect, pDestRect, hDestWindowOverride,
        pDirtyRegion, dwFlags);

    for (i = 0; i < This->base.nswapchains; i++) {
        hr = NineSwapChain9_Present(This->base.swapchains[i], pSourceRect, pDestRect,
                                    hDestWindowOverride, pDirtyRegion, dwFlags);
        if (FAILED(hr)) { return hr; }
    }

    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9Ex_GetGPUThreadPriority( struct NineDevice9Ex *This,
                                    INT *pPriority )
{
    DBG("This\n");
    user_assert(pPriority != NULL, D3DERR_INVALIDCALL);
    *pPriority = This->base.gpu_priority;
    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9Ex_SetGPUThreadPriority( struct NineDevice9Ex *This,
                                    INT Priority )
{
    DBG("This\n");
    user_assert(Priority >= -7 && Priority <= 7, D3DERR_INVALIDCALL);
    This->base.gpu_priority = Priority;
    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9Ex_WaitForVBlank( UNUSED struct NineDevice9Ex *This,
                             UNUSED UINT iSwapChain )
{
    DBG("This\n");
    STUB(D3D_OK);
}

HRESULT NINE_WINAPI
NineDevice9Ex_CheckResourceResidency( UNUSED struct NineDevice9Ex *This,
                                      UNUSED IDirect3DResource9 **pResourceArray,
                                      UNUSED UINT32 NumResources )
{
    DBG("This\n");
    STUB(D3D_OK);
}

HRESULT NINE_WINAPI
NineDevice9Ex_SetMaximumFrameLatency( struct NineDevice9Ex *This,
                                      UINT MaxLatency )
{
    DBG("This\n");
    This->base.max_frame_latency = MaxLatency;
    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9Ex_GetMaximumFrameLatency( struct NineDevice9Ex *This,
                                      UINT *pMaxLatency )
{
    DBG("This\n");
    user_assert(pMaxLatency != NULL, D3DERR_INVALIDCALL);
    *pMaxLatency = This->base.max_frame_latency;
    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9Ex_CheckDeviceState( struct NineDevice9Ex *This,
                                HWND hDestinationWindow )
{
    DBG("This=%p hDestinationWindow=%p\n",
        This, hDestinationWindow);

    user_assert(!This->base.swapchains[0]->params.Windowed, D3D_OK);

    if (This->base.params.hFocusWindow == hDestinationWindow) {
        if (NineSwapChain9_GetOccluded(This->base.swapchains[0]))
            return S_PRESENT_OCCLUDED;
    } else if(!NineSwapChain9_GetOccluded(This->base.swapchains[0])) {
        return S_PRESENT_OCCLUDED;
    }
    /* TODO: handle the other return values */
    return D3D_OK;
}

HRESULT NINE_WINAPI
NineDevice9Ex_CreateRenderTargetEx( struct NineDevice9Ex *This,
                                    UINT Width,
                                    UINT Height,
                                    D3DFORMAT Format,
                                    D3DMULTISAMPLE_TYPE MultiSample,
                                    DWORD MultisampleQuality,
                                    BOOL Lockable,
                                    IDirect3DSurface9 **ppSurface,
                                    HANDLE *pSharedHandle,
                                    UNUSED DWORD Usage )
{
    DBG("This\n");
    /* The Create*Ex functions only purpose seem to introduce the
     * Usage field, to pass the new d3d9ex flags on secure/restricted
     * content.
     * TODO: Return error on invalid Usage.
     * TODO: Store Usage in the surface descriptor, in case the
     * app checks */
    return NineDevice9_CreateRenderTarget(&This->base,
                                          Width,
                                          Height,
                                          Format,
                                          MultiSample,
                                          MultisampleQuality,
                                          Lockable,
                                          ppSurface,
                                          pSharedHandle);
}

HRESULT NINE_WINAPI
NineDevice9Ex_CreateOffscreenPlainSurfaceEx( struct NineDevice9Ex *This,
                                             UINT Width,
                                             UINT Height,
                                             D3DFORMAT Format,
                                             D3DPOOL Pool,
                                             IDirect3DSurface9 **ppSurface,
                                             HANDLE *pSharedHandle,
                                             UNUSED DWORD Usage )
{
    DBG("This\n");
    /* The Create*Ex functions only purpose seem to introduce the
     * Usage field, to pass the new d3d9ex flags on secure/restricted
     * content.
     * TODO: Return error on invalid Usage.
     * TODO: Store Usage in the surface descriptor, in case the
     * app checks */
    return NineDevice9_CreateOffscreenPlainSurface(&This->base,
                                                   Width,
                                                   Height,
                                                   Format,
                                                   Pool,
                                                   ppSurface,
                                                   pSharedHandle);
}

HRESULT NINE_WINAPI
NineDevice9Ex_CreateDepthStencilSurfaceEx( struct NineDevice9Ex *This,
                                           UINT Width,
                                           UINT Height,
                                           D3DFORMAT Format,
                                           D3DMULTISAMPLE_TYPE MultiSample,
                                           DWORD MultisampleQuality,
                                           BOOL Discard,
                                           IDirect3DSurface9 **ppSurface,
                                           HANDLE *pSharedHandle,
                                           UNUSED DWORD Usage )
{
    DBG("This\n");
    /* The Create*Ex functions only purpose seem to introduce the
     * Usage field, to pass the new d3d9ex flags on secure/restricted
     * content.
     * TODO: Return error on invalid Usage.
     * TODO: Store Usage in the surface descriptor, in case the
     * app checks */
    return NineDevice9_CreateDepthStencilSurface(&This->base,
                                                 Width,
                                                 Height,
                                                 Format,
                                                 MultiSample,
                                                 MultisampleQuality,
                                                 Discard,
                                                 ppSurface,
                                                 pSharedHandle);
}

HRESULT NINE_WINAPI
NineDevice9Ex_ResetEx( struct NineDevice9Ex *This,
                       D3DPRESENT_PARAMETERS *pPresentationParameters,
                       D3DDISPLAYMODEEX *pFullscreenDisplayMode )
{
    HRESULT hr = D3D_OK;
    float MinZ, MaxZ;
    unsigned i;

    DBG("This=%p pPresentationParameters=%p pFullscreenDisplayMode=%p\n", This, pPresentationParameters, pFullscreenDisplayMode);

    for (i = 0; i < This->base.nswapchains; ++i) {
        D3DDISPLAYMODEEX *mode = NULL;
        D3DPRESENT_PARAMETERS *params = &pPresentationParameters[i];
        if (pFullscreenDisplayMode) mode = &(pFullscreenDisplayMode[i]);
        hr = NineSwapChain9_Resize(This->base.swapchains[i], params, mode);
        if (FAILED(hr))
            break;
    }

    MinZ = This->base.state.viewport.MinZ; /* These are preserved */
    MaxZ = This->base.state.viewport.MaxZ;
    NineDevice9_SetRenderTarget(
        (struct NineDevice9 *)This, 0, (IDirect3DSurface9 *)This->base.swapchains[0]->buffers[0]);
    This->base.state.viewport.MinZ = MinZ;
    This->base.state.viewport.MaxZ = MaxZ;
    nine_context_set_viewport(&This->base, &This->base.state.viewport);

    if (This->base.nswapchains && This->base.swapchains[0]->params.EnableAutoDepthStencil)
        NineDevice9_SetDepthStencilSurface(
            &This->base, (IDirect3DSurface9 *)This->base.swapchains[0]->zsbuf);

    return hr;
}

HRESULT NINE_WINAPI
NineDevice9Ex_Reset( struct NineDevice9Ex *This,
                     D3DPRESENT_PARAMETERS *pPresentationParameters )
{
    HRESULT hr = D3D_OK;
    float MinZ, MaxZ;
    unsigned i;

    DBG("This=%p pPresentationParameters=%p\n", This, pPresentationParameters);

    for (i = 0; i < This->base.nswapchains; ++i) {
        D3DPRESENT_PARAMETERS *params = &pPresentationParameters[i];
        hr = NineSwapChain9_Resize(This->base.swapchains[i], params, NULL);
        if (FAILED(hr))
            break;
    }

    MinZ = This->base.state.viewport.MinZ; /* These are preserved */
    MaxZ = This->base.state.viewport.MaxZ;
    NineDevice9_SetRenderTarget(
        (struct NineDevice9 *)This, 0, (IDirect3DSurface9 *)This->base.swapchains[0]->buffers[0]);
    This->base.state.viewport.MinZ = MinZ;
    This->base.state.viewport.MaxZ = MaxZ;
    nine_context_set_viewport(&This->base, &This->base.state.viewport);

    if (This->base.nswapchains && This->base.swapchains[0]->params.EnableAutoDepthStencil)
        NineDevice9_SetDepthStencilSurface(
            &This->base, (IDirect3DSurface9 *)This->base.swapchains[0]->zsbuf);

    return hr;
}

HRESULT NINE_WINAPI
NineDevice9Ex_GetDisplayModeEx( struct NineDevice9Ex *This,
                                UINT iSwapChain,
                                D3DDISPLAYMODEEX *pMode,
                                D3DDISPLAYROTATION *pRotation )
{
    struct NineSwapChain9Ex *swapchain;

    DBG("This=%p iSwapChain=%u pMode=%p pRotation=%p\n",
        This, iSwapChain, pMode, pRotation);

    user_assert(iSwapChain < This->base.nswapchains, D3DERR_INVALIDCALL);

    swapchain = NineSwapChain9Ex(This->base.swapchains[iSwapChain]);
    return NineSwapChain9Ex_GetDisplayModeEx(swapchain, pMode, pRotation);
}

HRESULT NINE_WINAPI
NineDevice9Ex_TestCooperativeLevel( UNUSED struct NineDevice9Ex *This )
{
    DBG("This\n");
    return D3D_OK;
}


IDirect3DDevice9ExVtbl NineDevice9Ex_vtable = {
    (void *)NineUnknown_QueryInterface,
    (void *)NineUnknown_AddRef,
    (void *)NineUnknown_Release,
    (void *)NineDevice9Ex_TestCooperativeLevel,
    (void *)NineDevice9_GetAvailableTextureMem,
    (void *)NineDevice9_EvictManagedResources,
    (void *)NineDevice9_GetDirect3D,
    (void *)NineDevice9_GetDeviceCaps,
    (void *)NineDevice9_GetDisplayMode,
    (void *)NineDevice9_GetCreationParameters,
    (void *)NineDevice9_SetCursorProperties,
    (void *)NineDevice9_SetCursorPosition,
    (void *)NineDevice9_ShowCursor,
    (void *)NineDevice9_CreateAdditionalSwapChain,
    (void *)NineDevice9_GetSwapChain,
    (void *)NineDevice9_GetNumberOfSwapChains,
    (void *)NineDevice9Ex_Reset,
    (void *)NineDevice9_Present,
    (void *)NineDevice9_GetBackBuffer,
    (void *)NineDevice9_GetRasterStatus,
    (void *)NineDevice9_SetDialogBoxMode,
    (void *)NineDevice9_SetGammaRamp,
    (void *)NineDevice9_GetGammaRamp,
    (void *)NineDevice9_CreateTexture,
    (void *)NineDevice9_CreateVolumeTexture,
    (void *)NineDevice9_CreateCubeTexture,
    (void *)NineDevice9_CreateVertexBuffer,
    (void *)NineDevice9_CreateIndexBuffer,
    (void *)NineDevice9_CreateRenderTarget,
    (void *)NineDevice9_CreateDepthStencilSurface,
    (void *)NineDevice9_UpdateSurface,
    (void *)NineDevice9_UpdateTexture,
    (void *)NineDevice9_GetRenderTargetData,
    (void *)NineDevice9_GetFrontBufferData,
    (void *)NineDevice9_StretchRect,
    (void *)NineDevice9_ColorFill,
    (void *)NineDevice9_CreateOffscreenPlainSurface,
    (void *)NineDevice9_SetRenderTarget,
    (void *)NineDevice9_GetRenderTarget,
    (void *)NineDevice9_SetDepthStencilSurface,
    (void *)NineDevice9_GetDepthStencilSurface,
    (void *)NineDevice9_BeginScene,
    (void *)NineDevice9_EndScene,
    (void *)NineDevice9_Clear,
    (void *)NineDevice9_SetTransform,
    (void *)NineDevice9_GetTransform,
    (void *)NineDevice9_MultiplyTransform,
    (void *)NineDevice9_SetViewport,
    (void *)NineDevice9_GetViewport,
    (void *)NineDevice9_SetMaterial,
    (void *)NineDevice9_GetMaterial,
    (void *)NineDevice9_SetLight,
    (void *)NineDevice9_GetLight,
    (void *)NineDevice9_LightEnable,
    (void *)NineDevice9_GetLightEnable,
    (void *)NineDevice9_SetClipPlane,
    (void *)NineDevice9_GetClipPlane,
    (void *)NineDevice9_SetRenderState,
    (void *)NineDevice9_GetRenderState,
    (void *)NineDevice9_CreateStateBlock,
    (void *)NineDevice9_BeginStateBlock,
    (void *)NineDevice9_EndStateBlock,
    (void *)NineDevice9_SetClipStatus,
    (void *)NineDevice9_GetClipStatus,
    (void *)NineDevice9_GetTexture,
    (void *)NineDevice9_SetTexture,
    (void *)NineDevice9_GetTextureStageState,
    (void *)NineDevice9_SetTextureStageState,
    (void *)NineDevice9_GetSamplerState,
    (void *)NineDevice9_SetSamplerState,
    (void *)NineDevice9_ValidateDevice,
    (void *)NineDevice9_SetPaletteEntries,
    (void *)NineDevice9_GetPaletteEntries,
    (void *)NineDevice9_SetCurrentTexturePalette,
    (void *)NineDevice9_GetCurrentTexturePalette,
    (void *)NineDevice9_SetScissorRect,
    (void *)NineDevice9_GetScissorRect,
    (void *)NineDevice9_SetSoftwareVertexProcessing,
    (void *)NineDevice9_GetSoftwareVertexProcessing,
    (void *)NineDevice9_SetNPatchMode,
    (void *)NineDevice9_GetNPatchMode,
    (void *)NineDevice9_DrawPrimitive,
    (void *)NineDevice9_DrawIndexedPrimitive,
    (void *)NineDevice9_DrawPrimitiveUP,
    (void *)NineDevice9_DrawIndexedPrimitiveUP,
    (void *)NineDevice9_ProcessVertices,
    (void *)NineDevice9_CreateVertexDeclaration,
    (void *)NineDevice9_SetVertexDeclaration,
    (void *)NineDevice9_GetVertexDeclaration,
    (void *)NineDevice9_SetFVF,
    (void *)NineDevice9_GetFVF,
    (void *)NineDevice9_CreateVertexShader,
    (void *)NineDevice9_SetVertexShader,
    (void *)NineDevice9_GetVertexShader,
    (void *)NineDevice9_SetVertexShaderConstantF,
    (void *)NineDevice9_GetVertexShaderConstantF,
    (void *)NineDevice9_SetVertexShaderConstantI,
    (void *)NineDevice9_GetVertexShaderConstantI,
    (void *)NineDevice9_SetVertexShaderConstantB,
    (void *)NineDevice9_GetVertexShaderConstantB,
    (void *)NineDevice9_SetStreamSource,
    (void *)NineDevice9_GetStreamSource,
    (void *)NineDevice9_SetStreamSourceFreq,
    (void *)NineDevice9_GetStreamSourceFreq,
    (void *)NineDevice9_SetIndices,
    (void *)NineDevice9_GetIndices,
    (void *)NineDevice9_CreatePixelShader,
    (void *)NineDevice9_SetPixelShader,
    (void *)NineDevice9_GetPixelShader,
    (void *)NineDevice9_SetPixelShaderConstantF,
    (void *)NineDevice9_GetPixelShaderConstantF,
    (void *)NineDevice9_SetPixelShaderConstantI,
    (void *)NineDevice9_GetPixelShaderConstantI,
    (void *)NineDevice9_SetPixelShaderConstantB,
    (void *)NineDevice9_GetPixelShaderConstantB,
    (void *)NineDevice9_DrawRectPatch,
    (void *)NineDevice9_DrawTriPatch,
    (void *)NineDevice9_DeletePatch,
    (void *)NineDevice9_CreateQuery,
    (void *)NineDevice9Ex_SetConvolutionMonoKernel,
    (void *)NineDevice9Ex_ComposeRects,
    (void *)NineDevice9Ex_PresentEx,
    (void *)NineDevice9Ex_GetGPUThreadPriority,
    (void *)NineDevice9Ex_SetGPUThreadPriority,
    (void *)NineDevice9Ex_WaitForVBlank,
    (void *)NineDevice9Ex_CheckResourceResidency,
    (void *)NineDevice9Ex_SetMaximumFrameLatency,
    (void *)NineDevice9Ex_GetMaximumFrameLatency,
    (void *)NineDevice9Ex_CheckDeviceState,
    (void *)NineDevice9Ex_CreateRenderTargetEx,
    (void *)NineDevice9Ex_CreateOffscreenPlainSurfaceEx,
    (void *)NineDevice9Ex_CreateDepthStencilSurfaceEx,
    (void *)NineDevice9Ex_ResetEx,
    (void *)NineDevice9Ex_GetDisplayModeEx
};

static const GUID *NineDevice9Ex_IIDs[] = {
    &IID_IDirect3DDevice9Ex,
    &IID_IDirect3DDevice9,
    &IID_IUnknown,
    NULL
};

HRESULT
NineDevice9Ex_new( struct pipe_screen *pScreen,
                   D3DDEVICE_CREATION_PARAMETERS *pCreationParameters,
                   D3DCAPS9 *pCaps,
                   D3DPRESENT_PARAMETERS *pPresentationParameters,
                   D3DDISPLAYMODEEX *pFullscreenDisplayMode,
                   IDirect3D9Ex *pD3D9Ex,
                   ID3DPresentGroup *pPresentationGroup,
                   struct d3dadapter9_context *pCTX,
                   struct NineDevice9Ex **ppOut,
                   int minorVersionNum )
{
    BOOL lock;
    lock = !!(pCreationParameters->BehaviorFlags & D3DCREATE_MULTITHREADED);

    NINE_NEW(Device9Ex, ppOut, lock,
             pScreen, pCreationParameters, pCaps, pPresentationParameters,
             pFullscreenDisplayMode, pD3D9Ex, pPresentationGroup, pCTX, minorVersionNum );
}

