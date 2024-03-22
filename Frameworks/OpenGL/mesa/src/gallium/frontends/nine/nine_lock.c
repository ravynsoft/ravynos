/*
 * Copyright 2013 Joakim Sindholt <opensource@zhasha.com>
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

#include "authenticatedchannel9.h"
#include "basetexture9.h"
#include "cryptosession9.h"
#include "cubetexture9.h"
#include "device9.h"
#include "device9ex.h"
#include "device9video.h"
#include "indexbuffer9.h"
#include "pixelshader9.h"
#include "query9.h"
#include "resource9.h"
#include "stateblock9.h"
#include "surface9.h"
#include "swapchain9.h"
#include "swapchain9ex.h"
#include "texture9.h"
#include "vertexbuffer9.h"
#include "vertexdeclaration9.h"
#include "vertexshader9.h"
#include "volume9.h"
#include "volumetexture9.h"

#include "d3d9.h"
#include "nine_lock.h"

#include "util/simple_mtx.h"
#include "util/u_thread.h"

/* Global mutex as described by MSDN */
static simple_mtx_t d3dlock_global = SIMPLE_MTX_INITIALIZER;

void
NineLockGlobalMutex()
{
    simple_mtx_lock(&d3dlock_global);
}

void
NineUnlockGlobalMutex()
{
    simple_mtx_unlock(&d3dlock_global);
}

static HRESULT NINE_WINAPI
LockAuthenticatedChannel9_GetCertificateSize( struct NineAuthenticatedChannel9 *This,
                                              UINT *pCertificateSize )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineAuthenticatedChannel9_GetCertificateSize(This, pCertificateSize);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockAuthenticatedChannel9_GetCertificate( struct NineAuthenticatedChannel9 *This,
                                          UINT CertifacteSize,
                                          BYTE *ppCertificate )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineAuthenticatedChannel9_GetCertificate(This, CertifacteSize, ppCertificate);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockAuthenticatedChannel9_NegotiateKeyExchange( struct NineAuthenticatedChannel9 *This,
                                                UINT DataSize,
                                                void *pData )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineAuthenticatedChannel9_NegotiateKeyExchange(This, DataSize, pData);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockAuthenticatedChannel9_Query( struct NineAuthenticatedChannel9 *This,
                                 UINT InputSize,
                                 const void *pInput,
                                 UINT OutputSize,
                                 void *pOutput )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineAuthenticatedChannel9_Query(This, InputSize, pInput, OutputSize, pOutput);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockAuthenticatedChannel9_Configure( struct NineAuthenticatedChannel9 *This,
                                     UINT InputSize,
                                     const void *pInput,
                                     D3DAUTHENTICATEDCHANNEL_CONFIGURE_OUTPUT *pOutput )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineAuthenticatedChannel9_Configure(This, InputSize, pInput, pOutput);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

IDirect3DAuthenticatedChannel9Vtbl LockAuthenticatedChannel9_vtable = {
    (void *)NineUnknown_QueryInterface,
    (void *)NineUnknown_AddRef,
    (void *)NineUnknown_ReleaseWithDtorLock,
    (void *)LockAuthenticatedChannel9_GetCertificateSize,
    (void *)LockAuthenticatedChannel9_GetCertificate,
    (void *)LockAuthenticatedChannel9_NegotiateKeyExchange,
    (void *)LockAuthenticatedChannel9_Query,
    (void *)LockAuthenticatedChannel9_Configure
};

static HRESULT NINE_WINAPI
LockUnknown_SetPrivateData( struct NineUnknown *This,
                            REFGUID refguid,
                            const void *pData,
                            DWORD SizeOfData,
                            DWORD Flags )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineUnknown_SetPrivateData(This, refguid, pData, SizeOfData, Flags);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockUnknown_GetPrivateData( struct NineUnknown *This,
                            REFGUID refguid,
                            void *pData,
                            DWORD *pSizeOfData )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineUnknown_GetPrivateData(This, refguid, pData, pSizeOfData);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockUnknown_FreePrivateData( struct NineUnknown *This,
                             REFGUID refguid )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineUnknown_FreePrivateData(This, refguid);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

#if 0
static HRESULT NINE_WINAPI
LockResource9_GetDevice( struct NineResource9 *This,
                         IDirect3DDevice9 **ppDevice )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineUnknown_GetDevice(NineUnknown(This), ppDevice);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}
#endif

static DWORD NINE_WINAPI
LockResource9_SetPriority( struct NineResource9 *This,
                           DWORD PriorityNew )
{
    DWORD r;
    simple_mtx_lock(&d3dlock_global);
    r = NineResource9_SetPriority(This, PriorityNew);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static DWORD NINE_WINAPI
LockResource9_GetPriority( struct NineResource9 *This )
{
    DWORD r;
    simple_mtx_lock(&d3dlock_global);
    r = NineResource9_GetPriority(This);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

#if 0
static void NINE_WINAPI
LockResource9_PreLoad( struct NineResource9 *This )
{
    simple_mtx_lock(&d3dlock_global);
    NineResource9_PreLoad(This);
    simple_mtx_unlock(&d3dlock_global);
}
#endif

#if 0
static D3DRESOURCETYPE NINE_WINAPI
LockResource9_GetType( struct NineResource9 *This )
{
    D3DRESOURCETYPE r;
    simple_mtx_lock(&d3dlock_global);
    r = NineResource9_GetType(This);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}
#endif

static DWORD NINE_WINAPI
LockBaseTexture9_SetLOD( struct NineBaseTexture9 *This,
                         DWORD LODNew )
{
    DWORD r;
    simple_mtx_lock(&d3dlock_global);
    r = NineBaseTexture9_SetLOD(This, LODNew);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static DWORD NINE_WINAPI
LockBaseTexture9_GetLOD( struct NineBaseTexture9 *This )
{
    DWORD r;
    simple_mtx_lock(&d3dlock_global);
    r = NineBaseTexture9_GetLOD(This);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static DWORD NINE_WINAPI
LockBaseTexture9_GetLevelCount( struct NineBaseTexture9 *This )
{
    DWORD r;
    simple_mtx_lock(&d3dlock_global);
    r = NineBaseTexture9_GetLevelCount(This);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockBaseTexture9_SetAutoGenFilterType( struct NineBaseTexture9 *This,
                                       D3DTEXTUREFILTERTYPE FilterType )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineBaseTexture9_SetAutoGenFilterType(This, FilterType);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static D3DTEXTUREFILTERTYPE NINE_WINAPI
LockBaseTexture9_GetAutoGenFilterType( struct NineBaseTexture9 *This )
{
    D3DTEXTUREFILTERTYPE r;
    simple_mtx_lock(&d3dlock_global);
    r = NineBaseTexture9_GetAutoGenFilterType(This);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static void NINE_WINAPI
LockBaseTexture9_PreLoad( struct NineBaseTexture9 *This )
{
    simple_mtx_lock(&d3dlock_global);
    NineBaseTexture9_PreLoad(This);
    simple_mtx_unlock(&d3dlock_global);
}

static void NINE_WINAPI
LockBaseTexture9_GenerateMipSubLevels( struct NineBaseTexture9 *This )
{
    simple_mtx_lock(&d3dlock_global);
    NineBaseTexture9_GenerateMipSubLevels(This);
    simple_mtx_unlock(&d3dlock_global);
}

static HRESULT NINE_WINAPI
LockCryptoSession9_GetCertificateSize( struct NineCryptoSession9 *This,
                                       UINT *pCertificateSize )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineCryptoSession9_GetCertificateSize(This, pCertificateSize);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockCryptoSession9_GetCertificate( struct NineCryptoSession9 *This,
                                   UINT CertifacteSize,
                                   BYTE *ppCertificate )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineCryptoSession9_GetCertificate(This, CertifacteSize, ppCertificate);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockCryptoSession9_NegotiateKeyExchange( struct NineCryptoSession9 *This,
                                         UINT DataSize,
                                         void *pData )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineCryptoSession9_NegotiateKeyExchange(This, DataSize, pData);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockCryptoSession9_EncryptionBlt( struct NineCryptoSession9 *This,
                                  IDirect3DSurface9 *pSrcSurface,
                                  IDirect3DSurface9 *pDstSurface,
                                  UINT DstSurfaceSize,
                                  void *pIV )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineCryptoSession9_EncryptionBlt(This, pSrcSurface, pDstSurface, DstSurfaceSize, pIV);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockCryptoSession9_DecryptionBlt( struct NineCryptoSession9 *This,
                                  IDirect3DSurface9 *pSrcSurface,
                                  IDirect3DSurface9 *pDstSurface,
                                  UINT SrcSurfaceSize,
                                  D3DENCRYPTED_BLOCK_INFO *pEncryptedBlockInfo,
                                  void *pContentKey,
                                  void *pIV )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineCryptoSession9_DecryptionBlt(This, pSrcSurface, pDstSurface, SrcSurfaceSize, pEncryptedBlockInfo, pContentKey, pIV);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockCryptoSession9_GetSurfacePitch( struct NineCryptoSession9 *This,
                                    IDirect3DSurface9 *pSrcSurface,
                                    UINT *pSurfacePitch )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineCryptoSession9_GetSurfacePitch(This, pSrcSurface, pSurfacePitch);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockCryptoSession9_StartSessionKeyRefresh( struct NineCryptoSession9 *This,
                                           void *pRandomNumber,
                                           UINT RandomNumberSize )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineCryptoSession9_StartSessionKeyRefresh(This, pRandomNumber, RandomNumberSize);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockCryptoSession9_FinishSessionKeyRefresh( struct NineCryptoSession9 *This )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineCryptoSession9_FinishSessionKeyRefresh(This);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockCryptoSession9_GetEncryptionBltKey( struct NineCryptoSession9 *This,
                                        void *pReadbackKey,
                                        UINT KeySize )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineCryptoSession9_GetEncryptionBltKey(This, pReadbackKey, KeySize);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

IDirect3DCryptoSession9Vtbl LockCryptoSession9_vtable = {
    (void *)NineUnknown_QueryInterface,
    (void *)NineUnknown_AddRef,
    (void *)NineUnknown_ReleaseWithDtorLock,
    (void *)LockCryptoSession9_GetCertificateSize,
    (void *)LockCryptoSession9_GetCertificate,
    (void *)LockCryptoSession9_NegotiateKeyExchange,
    (void *)LockCryptoSession9_EncryptionBlt,
    (void *)LockCryptoSession9_DecryptionBlt,
    (void *)LockCryptoSession9_GetSurfacePitch,
    (void *)LockCryptoSession9_StartSessionKeyRefresh,
    (void *)LockCryptoSession9_FinishSessionKeyRefresh,
    (void *)LockCryptoSession9_GetEncryptionBltKey
};

#if 0
static HRESULT NINE_WINAPI
LockCubeTexture9_GetLevelDesc( struct NineCubeTexture9 *This,
                               UINT Level,
                               D3DSURFACE_DESC *pDesc )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineCubeTexture9_GetLevelDesc(This, Level, pDesc);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}
#endif

#if 0
static HRESULT NINE_WINAPI
LockCubeTexture9_GetCubeMapSurface( struct NineCubeTexture9 *This,
                                    D3DCUBEMAP_FACES FaceType,
                                    UINT Level,
                                    IDirect3DSurface9 **ppCubeMapSurface )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineCubeTexture9_GetCubeMapSurface(This, FaceType, Level, ppCubeMapSurface);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}
#endif

static HRESULT NINE_WINAPI
LockCubeTexture9_LockRect( struct NineCubeTexture9 *This,
                           D3DCUBEMAP_FACES FaceType,
                           UINT Level,
                           D3DLOCKED_RECT *pLockedRect,
                           const RECT *pRect,
                           DWORD Flags )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineCubeTexture9_LockRect(This, FaceType, Level, pLockedRect, pRect, Flags);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockCubeTexture9_UnlockRect( struct NineCubeTexture9 *This,
                             D3DCUBEMAP_FACES FaceType,
                             UINT Level )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineCubeTexture9_UnlockRect(This, FaceType, Level);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockCubeTexture9_AddDirtyRect( struct NineCubeTexture9 *This,
                               D3DCUBEMAP_FACES FaceType,
                               const RECT *pDirtyRect )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineCubeTexture9_AddDirtyRect(This, FaceType, pDirtyRect);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

IDirect3DCubeTexture9Vtbl LockCubeTexture9_vtable = {
    (void *)NineUnknown_QueryInterface,
    (void *)NineUnknown_AddRef,
    (void *)NineUnknown_ReleaseWithDtorLock,
    (void *)NineUnknown_GetDevice, /* actually part of Resource9 iface */
    (void *)LockUnknown_SetPrivateData,
    (void *)LockUnknown_GetPrivateData,
    (void *)LockUnknown_FreePrivateData,
    (void *)LockResource9_SetPriority,
    (void *)LockResource9_GetPriority,
    (void *)LockBaseTexture9_PreLoad,
    (void *)NineResource9_GetType, /* immutable */
    (void *)LockBaseTexture9_SetLOD,
    (void *)LockBaseTexture9_GetLOD,
    (void *)LockBaseTexture9_GetLevelCount,
    (void *)LockBaseTexture9_SetAutoGenFilterType,
    (void *)LockBaseTexture9_GetAutoGenFilterType,
    (void *)LockBaseTexture9_GenerateMipSubLevels,
    (void *)NineCubeTexture9_GetLevelDesc, /* immutable */
    (void *)NineCubeTexture9_GetCubeMapSurface, /* AddRef */
    (void *)LockCubeTexture9_LockRect,
    (void *)LockCubeTexture9_UnlockRect,
    (void *)LockCubeTexture9_AddDirtyRect
};

static HRESULT NINE_WINAPI
LockDevice9_TestCooperativeLevel( struct NineDevice9 *This )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_TestCooperativeLevel(This);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static UINT NINE_WINAPI
LockDevice9_GetAvailableTextureMem( struct NineDevice9 *This )
{
    UINT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_GetAvailableTextureMem(This);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_EvictManagedResources( struct NineDevice9 *This )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_EvictManagedResources(This);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_GetDirect3D( struct NineDevice9 *This,
                         IDirect3D9 **ppD3D9 )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_GetDirect3D(This, ppD3D9);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

#if 0
static HRESULT NINE_WINAPI
LockDevice9_GetDeviceCaps( struct NineDevice9 *This,
                           D3DCAPS9 *pCaps )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_GetDeviceCaps(This, pCaps);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}
#endif

static HRESULT NINE_WINAPI
LockDevice9_GetDisplayMode( struct NineDevice9 *This,
                            UINT iSwapChain,
                            D3DDISPLAYMODE *pMode )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_GetDisplayMode(This, iSwapChain, pMode);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

#if 0
static HRESULT NINE_WINAPI
LockDevice9_GetCreationParameters( struct NineDevice9 *This,
                                   D3DDEVICE_CREATION_PARAMETERS *pParameters )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_GetCreationParameters(This, pParameters);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}
#endif

static HRESULT NINE_WINAPI
LockDevice9_SetCursorProperties( struct NineDevice9 *This,
                                 UINT XHotSpot,
                                 UINT YHotSpot,
                                 IDirect3DSurface9 *pCursorBitmap )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_SetCursorProperties(This, XHotSpot, YHotSpot, pCursorBitmap);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static void NINE_WINAPI
LockDevice9_SetCursorPosition( struct NineDevice9 *This,
                               int X,
                               int Y,
                               DWORD Flags )
{
    simple_mtx_lock(&d3dlock_global);
    NineDevice9_SetCursorPosition(This, X, Y, Flags);
    simple_mtx_unlock(&d3dlock_global);
}

static BOOL NINE_WINAPI
LockDevice9_ShowCursor( struct NineDevice9 *This,
                        BOOL bShow )
{
    BOOL r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_ShowCursor(This, bShow);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_CreateAdditionalSwapChain( struct NineDevice9 *This,
                                       D3DPRESENT_PARAMETERS *pPresentationParameters,
                                       IDirect3DSwapChain9 **pSwapChain )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_CreateAdditionalSwapChain(This, pPresentationParameters, pSwapChain);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_GetSwapChain( struct NineDevice9 *This,
                          UINT iSwapChain,
                          IDirect3DSwapChain9 **pSwapChain )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_GetSwapChain(This, iSwapChain, pSwapChain);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static UINT NINE_WINAPI
LockDevice9_GetNumberOfSwapChains( struct NineDevice9 *This )
{
    UINT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_GetNumberOfSwapChains(This);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_Reset( struct NineDevice9 *This,
                   D3DPRESENT_PARAMETERS *pPresentationParameters )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_Reset(This, pPresentationParameters);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_Present( struct NineDevice9 *This,
                     const RECT *pSourceRect,
                     const RECT *pDestRect,
                     HWND hDestWindowOverride,
                     const RGNDATA *pDirtyRegion )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_Present(This, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_GetBackBuffer( struct NineDevice9 *This,
                           UINT iSwapChain,
                           UINT iBackBuffer,
                           D3DBACKBUFFER_TYPE Type,
                           IDirect3DSurface9 **ppBackBuffer )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_GetBackBuffer(This, iSwapChain, iBackBuffer, Type, ppBackBuffer);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_GetRasterStatus( struct NineDevice9 *This,
                             UINT iSwapChain,
                             D3DRASTER_STATUS *pRasterStatus )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_GetRasterStatus(This, iSwapChain, pRasterStatus);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_SetDialogBoxMode( struct NineDevice9 *This,
                              BOOL bEnableDialogs )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_SetDialogBoxMode(This, bEnableDialogs);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static void NINE_WINAPI
LockDevice9_SetGammaRamp( struct NineDevice9 *This,
                          UINT iSwapChain,
                          DWORD Flags,
                          const D3DGAMMARAMP *pRamp )
{
    simple_mtx_lock(&d3dlock_global);
    NineDevice9_SetGammaRamp(This, iSwapChain, Flags, pRamp);
    simple_mtx_unlock(&d3dlock_global);
}

static void NINE_WINAPI
LockDevice9_GetGammaRamp( struct NineDevice9 *This,
                          UINT iSwapChain,
                          D3DGAMMARAMP *pRamp )
{
    simple_mtx_lock(&d3dlock_global);
    NineDevice9_GetGammaRamp(This, iSwapChain, pRamp);
    simple_mtx_unlock(&d3dlock_global);
}

static HRESULT NINE_WINAPI
LockDevice9_CreateTexture( struct NineDevice9 *This,
                           UINT Width,
                           UINT Height,
                           UINT Levels,
                           DWORD Usage,
                           D3DFORMAT Format,
                           D3DPOOL Pool,
                           IDirect3DTexture9 **ppTexture,
                           HANDLE *pSharedHandle )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_CreateTexture(This, Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_CreateVolumeTexture( struct NineDevice9 *This,
                                 UINT Width,
                                 UINT Height,
                                 UINT Depth,
                                 UINT Levels,
                                 DWORD Usage,
                                 D3DFORMAT Format,
                                 D3DPOOL Pool,
                                 IDirect3DVolumeTexture9 **ppVolumeTexture,
                                 HANDLE *pSharedHandle )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_CreateVolumeTexture(This, Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, pSharedHandle);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_CreateCubeTexture( struct NineDevice9 *This,
                               UINT EdgeLength,
                               UINT Levels,
                               DWORD Usage,
                               D3DFORMAT Format,
                               D3DPOOL Pool,
                               IDirect3DCubeTexture9 **ppCubeTexture,
                               HANDLE *pSharedHandle )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_CreateCubeTexture(This, EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pSharedHandle);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_CreateVertexBuffer( struct NineDevice9 *This,
                                UINT Length,
                                DWORD Usage,
                                DWORD FVF,
                                D3DPOOL Pool,
                                IDirect3DVertexBuffer9 **ppVertexBuffer,
                                HANDLE *pSharedHandle )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_CreateVertexBuffer(This, Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_CreateIndexBuffer( struct NineDevice9 *This,
                               UINT Length,
                               DWORD Usage,
                               D3DFORMAT Format,
                               D3DPOOL Pool,
                               IDirect3DIndexBuffer9 **ppIndexBuffer,
                               HANDLE *pSharedHandle )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_CreateIndexBuffer(This, Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_CreateRenderTarget( struct NineDevice9 *This,
                                UINT Width,
                                UINT Height,
                                D3DFORMAT Format,
                                D3DMULTISAMPLE_TYPE MultiSample,
                                DWORD MultisampleQuality,
                                BOOL Lockable,
                                IDirect3DSurface9 **ppSurface,
                                HANDLE *pSharedHandle )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_CreateRenderTarget(This, Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_CreateDepthStencilSurface( struct NineDevice9 *This,
                                       UINT Width,
                                       UINT Height,
                                       D3DFORMAT Format,
                                       D3DMULTISAMPLE_TYPE MultiSample,
                                       DWORD MultisampleQuality,
                                       BOOL Discard,
                                       IDirect3DSurface9 **ppSurface,
                                       HANDLE *pSharedHandle )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_CreateDepthStencilSurface(This, Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_UpdateSurface( struct NineDevice9 *This,
                           IDirect3DSurface9 *pSourceSurface,
                           const RECT *pSourceRect,
                           IDirect3DSurface9 *pDestinationSurface,
                           const POINT *pDestPoint )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_UpdateSurface(This, pSourceSurface, pSourceRect, pDestinationSurface, pDestPoint);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_UpdateTexture( struct NineDevice9 *This,
                           IDirect3DBaseTexture9 *pSourceTexture,
                           IDirect3DBaseTexture9 *pDestinationTexture )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_UpdateTexture(This, pSourceTexture, pDestinationTexture);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_GetRenderTargetData( struct NineDevice9 *This,
                                 IDirect3DSurface9 *pRenderTarget,
                                 IDirect3DSurface9 *pDestSurface )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_GetRenderTargetData(This, pRenderTarget, pDestSurface);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_GetFrontBufferData( struct NineDevice9 *This,
                                UINT iSwapChain,
                                IDirect3DSurface9 *pDestSurface )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_GetFrontBufferData(This, iSwapChain, pDestSurface);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_StretchRect( struct NineDevice9 *This,
                         IDirect3DSurface9 *pSourceSurface,
                         const RECT *pSourceRect,
                         IDirect3DSurface9 *pDestSurface,
                         const RECT *pDestRect,
                         D3DTEXTUREFILTERTYPE Filter )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_StretchRect(This, pSourceSurface, pSourceRect, pDestSurface, pDestRect, Filter);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_ColorFill( struct NineDevice9 *This,
                       IDirect3DSurface9 *pSurface,
                       const RECT *pRect,
                       D3DCOLOR color )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_ColorFill(This, pSurface, pRect, color);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_CreateOffscreenPlainSurface( struct NineDevice9 *This,
                                         UINT Width,
                                         UINT Height,
                                         D3DFORMAT Format,
                                         D3DPOOL Pool,
                                         IDirect3DSurface9 **ppSurface,
                                         HANDLE *pSharedHandle )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_CreateOffscreenPlainSurface(This, Width, Height, Format, Pool, ppSurface, pSharedHandle);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_SetRenderTarget( struct NineDevice9 *This,
                             DWORD RenderTargetIndex,
                             IDirect3DSurface9 *pRenderTarget )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_SetRenderTarget(This, RenderTargetIndex, pRenderTarget);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_GetRenderTarget( struct NineDevice9 *This,
                             DWORD RenderTargetIndex,
                             IDirect3DSurface9 **ppRenderTarget )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_GetRenderTarget(This, RenderTargetIndex, ppRenderTarget);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_SetDepthStencilSurface( struct NineDevice9 *This,
                                    IDirect3DSurface9 *pNewZStencil )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_SetDepthStencilSurface(This, pNewZStencil);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_GetDepthStencilSurface( struct NineDevice9 *This,
                                    IDirect3DSurface9 **ppZStencilSurface )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_GetDepthStencilSurface(This, ppZStencilSurface);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_BeginScene( struct NineDevice9 *This )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_BeginScene(This);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_EndScene( struct NineDevice9 *This )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_EndScene(This);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_Clear( struct NineDevice9 *This,
                   DWORD Count,
                   const D3DRECT *pRects,
                   DWORD Flags,
                   D3DCOLOR Color,
                   float Z,
                   DWORD Stencil )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_Clear(This, Count, pRects, Flags, Color, Z, Stencil);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_SetTransform( struct NineDevice9 *This,
                          D3DTRANSFORMSTATETYPE State,
                          const D3DMATRIX *pMatrix )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_SetTransform(This, State, pMatrix);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_GetTransform( struct NineDevice9 *This,
                          D3DTRANSFORMSTATETYPE State,
                          D3DMATRIX *pMatrix )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_GetTransform(This, State, pMatrix);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_MultiplyTransform( struct NineDevice9 *This,
                               D3DTRANSFORMSTATETYPE State,
                               const D3DMATRIX *pMatrix )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_MultiplyTransform(This, State, pMatrix);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_SetViewport( struct NineDevice9 *This,
                         const D3DVIEWPORT9 *pViewport )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_SetViewport(This, pViewport);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_GetViewport( struct NineDevice9 *This,
                         D3DVIEWPORT9 *pViewport )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_GetViewport(This, pViewport);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_SetMaterial( struct NineDevice9 *This,
                         const D3DMATERIAL9 *pMaterial )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_SetMaterial(This, pMaterial);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_GetMaterial( struct NineDevice9 *This,
                         D3DMATERIAL9 *pMaterial )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_GetMaterial(This, pMaterial);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_SetLight( struct NineDevice9 *This,
                      DWORD Index,
                      const D3DLIGHT9 *pLight )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_SetLight(This, Index, pLight);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_GetLight( struct NineDevice9 *This,
                      DWORD Index,
                      D3DLIGHT9 *pLight )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_GetLight(This, Index, pLight);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_LightEnable( struct NineDevice9 *This,
                         DWORD Index,
                         BOOL Enable )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_LightEnable(This, Index, Enable);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_GetLightEnable( struct NineDevice9 *This,
                            DWORD Index,
                            BOOL *pEnable )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_GetLightEnable(This, Index, pEnable);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_SetClipPlane( struct NineDevice9 *This,
                          DWORD Index,
                          const float *pPlane )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_SetClipPlane(This, Index, pPlane);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_GetClipPlane( struct NineDevice9 *This,
                          DWORD Index,
                          float *pPlane )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_GetClipPlane(This, Index, pPlane);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_SetRenderState( struct NineDevice9 *This,
                            D3DRENDERSTATETYPE State,
                            DWORD Value )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_SetRenderState(This, State, Value);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_GetRenderState( struct NineDevice9 *This,
                            D3DRENDERSTATETYPE State,
                            DWORD *pValue )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_GetRenderState(This, State, pValue);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_CreateStateBlock( struct NineDevice9 *This,
                              D3DSTATEBLOCKTYPE Type,
                              IDirect3DStateBlock9 **ppSB )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_CreateStateBlock(This, Type, ppSB);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_BeginStateBlock( struct NineDevice9 *This )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_BeginStateBlock(This);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_EndStateBlock( struct NineDevice9 *This,
                           IDirect3DStateBlock9 **ppSB )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_EndStateBlock(This, ppSB);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_SetClipStatus( struct NineDevice9 *This,
                           const D3DCLIPSTATUS9 *pClipStatus )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_SetClipStatus(This, pClipStatus);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_GetClipStatus( struct NineDevice9 *This,
                           D3DCLIPSTATUS9 *pClipStatus )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_GetClipStatus(This, pClipStatus);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_GetTexture( struct NineDevice9 *This,
                        DWORD Stage,
                        IDirect3DBaseTexture9 **ppTexture )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_GetTexture(This, Stage, ppTexture);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_SetTexture( struct NineDevice9 *This,
                        DWORD Stage,
                        IDirect3DBaseTexture9 *pTexture )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_SetTexture(This, Stage, pTexture);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_GetTextureStageState( struct NineDevice9 *This,
                                  DWORD Stage,
                                  D3DTEXTURESTAGESTATETYPE Type,
                                  DWORD *pValue )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_GetTextureStageState(This, Stage, Type, pValue);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_SetTextureStageState( struct NineDevice9 *This,
                                  DWORD Stage,
                                  D3DTEXTURESTAGESTATETYPE Type,
                                  DWORD Value )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_SetTextureStageState(This, Stage, Type, Value);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_GetSamplerState( struct NineDevice9 *This,
                             DWORD Sampler,
                             D3DSAMPLERSTATETYPE Type,
                             DWORD *pValue )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_GetSamplerState(This, Sampler, Type, pValue);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_SetSamplerState( struct NineDevice9 *This,
                             DWORD Sampler,
                             D3DSAMPLERSTATETYPE Type,
                             DWORD Value )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_SetSamplerState(This, Sampler, Type, Value);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_ValidateDevice( struct NineDevice9 *This,
                            DWORD *pNumPasses )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_ValidateDevice(This, pNumPasses);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_SetPaletteEntries( struct NineDevice9 *This,
                               UINT PaletteNumber,
                               const PALETTEENTRY *pEntries )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_SetPaletteEntries(This, PaletteNumber, pEntries);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_GetPaletteEntries( struct NineDevice9 *This,
                               UINT PaletteNumber,
                               PALETTEENTRY *pEntries )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_GetPaletteEntries(This, PaletteNumber, pEntries);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_SetCurrentTexturePalette( struct NineDevice9 *This,
                                      UINT PaletteNumber )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_SetCurrentTexturePalette(This, PaletteNumber);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_GetCurrentTexturePalette( struct NineDevice9 *This,
                                      UINT *PaletteNumber )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_GetCurrentTexturePalette(This, PaletteNumber);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_SetScissorRect( struct NineDevice9 *This,
                            const RECT *pRect )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_SetScissorRect(This, pRect);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_GetScissorRect( struct NineDevice9 *This,
                            RECT *pRect )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_GetScissorRect(This, pRect);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_SetSoftwareVertexProcessing( struct NineDevice9 *This,
                                         BOOL bSoftware )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_SetSoftwareVertexProcessing(This, bSoftware);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static BOOL NINE_WINAPI
LockDevice9_GetSoftwareVertexProcessing( struct NineDevice9 *This )
{
    BOOL r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_GetSoftwareVertexProcessing(This);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_SetNPatchMode( struct NineDevice9 *This,
                           float nSegments )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_SetNPatchMode(This, nSegments);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static float NINE_WINAPI
LockDevice9_GetNPatchMode( struct NineDevice9 *This )
{
    float r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_GetNPatchMode(This);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_DrawPrimitive( struct NineDevice9 *This,
                           D3DPRIMITIVETYPE PrimitiveType,
                           UINT StartVertex,
                           UINT PrimitiveCount )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_DrawPrimitive(This, PrimitiveType, StartVertex, PrimitiveCount);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_DrawIndexedPrimitive( struct NineDevice9 *This,
                                  D3DPRIMITIVETYPE PrimitiveType,
                                  INT BaseVertexIndex,
                                  UINT MinVertexIndex,
                                  UINT NumVertices,
                                  UINT startIndex,
                                  UINT primCount )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_DrawIndexedPrimitive(This, PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_DrawPrimitiveUP( struct NineDevice9 *This,
                             D3DPRIMITIVETYPE PrimitiveType,
                             UINT PrimitiveCount,
                             const void *pVertexStreamZeroData,
                             UINT VertexStreamZeroStride )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_DrawPrimitiveUP(This, PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_DrawIndexedPrimitiveUP( struct NineDevice9 *This,
                                    D3DPRIMITIVETYPE PrimitiveType,
                                    UINT MinVertexIndex,
                                    UINT NumVertices,
                                    UINT PrimitiveCount,
                                    const void *pIndexData,
                                    D3DFORMAT IndexDataFormat,
                                    const void *pVertexStreamZeroData,
                                    UINT VertexStreamZeroStride )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_DrawIndexedPrimitiveUP(This, PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_ProcessVertices( struct NineDevice9 *This,
                             UINT SrcStartIndex,
                             UINT DestIndex,
                             UINT VertexCount,
                             IDirect3DVertexBuffer9 *pDestBuffer,
                             IDirect3DVertexDeclaration9 *pVertexDecl,
                             DWORD Flags )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_ProcessVertices(This, SrcStartIndex, DestIndex, VertexCount, pDestBuffer, pVertexDecl, Flags);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_CreateVertexDeclaration( struct NineDevice9 *This,
                                     const D3DVERTEXELEMENT9 *pVertexElements,
                                     IDirect3DVertexDeclaration9 **ppDecl )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_CreateVertexDeclaration(This, pVertexElements, ppDecl);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_SetVertexDeclaration( struct NineDevice9 *This,
                                  IDirect3DVertexDeclaration9 *pDecl )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_SetVertexDeclaration(This, pDecl);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_GetVertexDeclaration( struct NineDevice9 *This,
                                  IDirect3DVertexDeclaration9 **ppDecl )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_GetVertexDeclaration(This, ppDecl);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_SetFVF( struct NineDevice9 *This,
                    DWORD FVF )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_SetFVF(This, FVF);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_GetFVF( struct NineDevice9 *This,
                    DWORD *pFVF )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_GetFVF(This, pFVF);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_CreateVertexShader( struct NineDevice9 *This,
                                const DWORD *pFunction,
                                IDirect3DVertexShader9 **ppShader )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_CreateVertexShader(This, pFunction, ppShader);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_SetVertexShader( struct NineDevice9 *This,
                             IDirect3DVertexShader9 *pShader )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_SetVertexShader(This, pShader);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_GetVertexShader( struct NineDevice9 *This,
                             IDirect3DVertexShader9 **ppShader )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_GetVertexShader(This, ppShader);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_SetVertexShaderConstantF( struct NineDevice9 *This,
                                      UINT StartRegister,
                                      const float *pConstantData,
                                      UINT Vector4fCount )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_SetVertexShaderConstantF(This, StartRegister, pConstantData, Vector4fCount);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_GetVertexShaderConstantF( struct NineDevice9 *This,
                                      UINT StartRegister,
                                      float *pConstantData,
                                      UINT Vector4fCount )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_GetVertexShaderConstantF(This, StartRegister, pConstantData, Vector4fCount);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_SetVertexShaderConstantI( struct NineDevice9 *This,
                                      UINT StartRegister,
                                      const int *pConstantData,
                                      UINT Vector4iCount )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_SetVertexShaderConstantI(This, StartRegister, pConstantData, Vector4iCount);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_GetVertexShaderConstantI( struct NineDevice9 *This,
                                      UINT StartRegister,
                                      int *pConstantData,
                                      UINT Vector4iCount )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_GetVertexShaderConstantI(This, StartRegister, pConstantData, Vector4iCount);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_SetVertexShaderConstantB( struct NineDevice9 *This,
                                      UINT StartRegister,
                                      const BOOL *pConstantData,
                                      UINT BoolCount )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_SetVertexShaderConstantB(This, StartRegister, pConstantData, BoolCount);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_GetVertexShaderConstantB( struct NineDevice9 *This,
                                      UINT StartRegister,
                                      BOOL *pConstantData,
                                      UINT BoolCount )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_GetVertexShaderConstantB(This, StartRegister, pConstantData, BoolCount);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_SetStreamSource( struct NineDevice9 *This,
                             UINT StreamNumber,
                             IDirect3DVertexBuffer9 *pStreamData,
                             UINT OffsetInBytes,
                             UINT Stride )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_SetStreamSource(This, StreamNumber, pStreamData, OffsetInBytes, Stride);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_GetStreamSource( struct NineDevice9 *This,
                             UINT StreamNumber,
                             IDirect3DVertexBuffer9 **ppStreamData,
                             UINT *pOffsetInBytes,
                             UINT *pStride )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_GetStreamSource(This, StreamNumber, ppStreamData, pOffsetInBytes, pStride);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_SetStreamSourceFreq( struct NineDevice9 *This,
                                 UINT StreamNumber,
                                 UINT Setting )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_SetStreamSourceFreq(This, StreamNumber, Setting);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_GetStreamSourceFreq( struct NineDevice9 *This,
                                 UINT StreamNumber,
                                 UINT *pSetting )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_GetStreamSourceFreq(This, StreamNumber, pSetting);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_SetIndices( struct NineDevice9 *This,
                        IDirect3DIndexBuffer9 *pIndexData )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_SetIndices(This, pIndexData);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_GetIndices( struct NineDevice9 *This,
                        IDirect3DIndexBuffer9 **ppIndexData )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_GetIndices(This, ppIndexData);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_CreatePixelShader( struct NineDevice9 *This,
                               const DWORD *pFunction,
                               IDirect3DPixelShader9 **ppShader )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_CreatePixelShader(This, pFunction, ppShader);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_SetPixelShader( struct NineDevice9 *This,
                            IDirect3DPixelShader9 *pShader )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_SetPixelShader(This, pShader);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_GetPixelShader( struct NineDevice9 *This,
                            IDirect3DPixelShader9 **ppShader )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_GetPixelShader(This, ppShader);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_SetPixelShaderConstantF( struct NineDevice9 *This,
                                     UINT StartRegister,
                                     const float *pConstantData,
                                     UINT Vector4fCount )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_SetPixelShaderConstantF(This, StartRegister, pConstantData, Vector4fCount);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_GetPixelShaderConstantF( struct NineDevice9 *This,
                                     UINT StartRegister,
                                     float *pConstantData,
                                     UINT Vector4fCount )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_GetPixelShaderConstantF(This, StartRegister, pConstantData, Vector4fCount);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_SetPixelShaderConstantI( struct NineDevice9 *This,
                                     UINT StartRegister,
                                     const int *pConstantData,
                                     UINT Vector4iCount )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_SetPixelShaderConstantI(This, StartRegister, pConstantData, Vector4iCount);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_GetPixelShaderConstantI( struct NineDevice9 *This,
                                     UINT StartRegister,
                                     int *pConstantData,
                                     UINT Vector4iCount )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_GetPixelShaderConstantI(This, StartRegister, pConstantData, Vector4iCount);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_SetPixelShaderConstantB( struct NineDevice9 *This,
                                     UINT StartRegister,
                                     const BOOL *pConstantData,
                                     UINT BoolCount )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_SetPixelShaderConstantB(This, StartRegister, pConstantData, BoolCount);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_GetPixelShaderConstantB( struct NineDevice9 *This,
                                     UINT StartRegister,
                                     BOOL *pConstantData,
                                     UINT BoolCount )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_GetPixelShaderConstantB(This, StartRegister, pConstantData, BoolCount);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_DrawRectPatch( struct NineDevice9 *This,
                           UINT Handle,
                           const float *pNumSegs,
                           const D3DRECTPATCH_INFO *pRectPatchInfo )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_DrawRectPatch(This, Handle, pNumSegs, pRectPatchInfo);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_DrawTriPatch( struct NineDevice9 *This,
                          UINT Handle,
                          const float *pNumSegs,
                          const D3DTRIPATCH_INFO *pTriPatchInfo )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_DrawTriPatch(This, Handle, pNumSegs, pTriPatchInfo);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_DeletePatch( struct NineDevice9 *This,
                         UINT Handle )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_DeletePatch(This, Handle);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9_CreateQuery( struct NineDevice9 *This,
                         D3DQUERYTYPE Type,
                         IDirect3DQuery9 **ppQuery )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9_CreateQuery(This, Type, ppQuery);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

IDirect3DDevice9Vtbl LockDevice9_vtable = {
    (void *)NineUnknown_QueryInterface,
    (void *)NineUnknown_AddRef,
    (void *)NineUnknown_ReleaseWithDtorLock,
    (void *)LockDevice9_TestCooperativeLevel,
    (void *)LockDevice9_GetAvailableTextureMem,
    (void *)LockDevice9_EvictManagedResources,
    (void *)LockDevice9_GetDirect3D,
    (void *)NineDevice9_GetDeviceCaps, /* immutable */
    (void *)LockDevice9_GetDisplayMode,
    (void *)NineDevice9_GetCreationParameters, /* immutable */
    (void *)LockDevice9_SetCursorProperties,
    (void *)LockDevice9_SetCursorPosition,
    (void *)LockDevice9_ShowCursor,
    (void *)LockDevice9_CreateAdditionalSwapChain,
    (void *)LockDevice9_GetSwapChain,
    (void *)LockDevice9_GetNumberOfSwapChains,
    (void *)LockDevice9_Reset,
    (void *)LockDevice9_Present,
    (void *)LockDevice9_GetBackBuffer,
    (void *)LockDevice9_GetRasterStatus,
    (void *)LockDevice9_SetDialogBoxMode,
    (void *)LockDevice9_SetGammaRamp,
    (void *)LockDevice9_GetGammaRamp,
    (void *)LockDevice9_CreateTexture,
    (void *)LockDevice9_CreateVolumeTexture,
    (void *)LockDevice9_CreateCubeTexture,
    (void *)LockDevice9_CreateVertexBuffer,
    (void *)LockDevice9_CreateIndexBuffer,
    (void *)LockDevice9_CreateRenderTarget,
    (void *)LockDevice9_CreateDepthStencilSurface,
    (void *)LockDevice9_UpdateSurface,
    (void *)LockDevice9_UpdateTexture,
    (void *)LockDevice9_GetRenderTargetData,
    (void *)LockDevice9_GetFrontBufferData,
    (void *)LockDevice9_StretchRect,
    (void *)LockDevice9_ColorFill,
    (void *)LockDevice9_CreateOffscreenPlainSurface,
    (void *)LockDevice9_SetRenderTarget,
    (void *)LockDevice9_GetRenderTarget,
    (void *)LockDevice9_SetDepthStencilSurface,
    (void *)LockDevice9_GetDepthStencilSurface,
    (void *)LockDevice9_BeginScene,
    (void *)LockDevice9_EndScene,
    (void *)LockDevice9_Clear,
    (void *)LockDevice9_SetTransform,
    (void *)LockDevice9_GetTransform,
    (void *)LockDevice9_MultiplyTransform,
    (void *)LockDevice9_SetViewport,
    (void *)LockDevice9_GetViewport,
    (void *)LockDevice9_SetMaterial,
    (void *)LockDevice9_GetMaterial,
    (void *)LockDevice9_SetLight,
    (void *)LockDevice9_GetLight,
    (void *)LockDevice9_LightEnable,
    (void *)LockDevice9_GetLightEnable,
    (void *)LockDevice9_SetClipPlane,
    (void *)LockDevice9_GetClipPlane,
    (void *)LockDevice9_SetRenderState,
    (void *)LockDevice9_GetRenderState,
    (void *)LockDevice9_CreateStateBlock,
    (void *)LockDevice9_BeginStateBlock,
    (void *)LockDevice9_EndStateBlock,
    (void *)LockDevice9_SetClipStatus,
    (void *)LockDevice9_GetClipStatus,
    (void *)LockDevice9_GetTexture,
    (void *)LockDevice9_SetTexture,
    (void *)LockDevice9_GetTextureStageState,
    (void *)LockDevice9_SetTextureStageState,
    (void *)LockDevice9_GetSamplerState,
    (void *)LockDevice9_SetSamplerState,
    (void *)LockDevice9_ValidateDevice,
    (void *)LockDevice9_SetPaletteEntries,
    (void *)LockDevice9_GetPaletteEntries,
    (void *)LockDevice9_SetCurrentTexturePalette,
    (void *)LockDevice9_GetCurrentTexturePalette,
    (void *)LockDevice9_SetScissorRect,
    (void *)LockDevice9_GetScissorRect,
    (void *)LockDevice9_SetSoftwareVertexProcessing,
    (void *)LockDevice9_GetSoftwareVertexProcessing,
    (void *)LockDevice9_SetNPatchMode,
    (void *)LockDevice9_GetNPatchMode,
    (void *)LockDevice9_DrawPrimitive,
    (void *)LockDevice9_DrawIndexedPrimitive,
    (void *)LockDevice9_DrawPrimitiveUP,
    (void *)LockDevice9_DrawIndexedPrimitiveUP,
    (void *)LockDevice9_ProcessVertices,
    (void *)LockDevice9_CreateVertexDeclaration,
    (void *)LockDevice9_SetVertexDeclaration,
    (void *)LockDevice9_GetVertexDeclaration,
    (void *)LockDevice9_SetFVF,
    (void *)LockDevice9_GetFVF,
    (void *)LockDevice9_CreateVertexShader,
    (void *)LockDevice9_SetVertexShader,
    (void *)LockDevice9_GetVertexShader,
    (void *)LockDevice9_SetVertexShaderConstantF,
    (void *)LockDevice9_GetVertexShaderConstantF,
    (void *)LockDevice9_SetVertexShaderConstantI,
    (void *)LockDevice9_GetVertexShaderConstantI,
    (void *)LockDevice9_SetVertexShaderConstantB,
    (void *)LockDevice9_GetVertexShaderConstantB,
    (void *)LockDevice9_SetStreamSource,
    (void *)LockDevice9_GetStreamSource,
    (void *)LockDevice9_SetStreamSourceFreq,
    (void *)LockDevice9_GetStreamSourceFreq,
    (void *)LockDevice9_SetIndices,
    (void *)LockDevice9_GetIndices,
    (void *)LockDevice9_CreatePixelShader,
    (void *)LockDevice9_SetPixelShader,
    (void *)LockDevice9_GetPixelShader,
    (void *)LockDevice9_SetPixelShaderConstantF,
    (void *)LockDevice9_GetPixelShaderConstantF,
    (void *)LockDevice9_SetPixelShaderConstantI,
    (void *)LockDevice9_GetPixelShaderConstantI,
    (void *)LockDevice9_SetPixelShaderConstantB,
    (void *)LockDevice9_GetPixelShaderConstantB,
    (void *)LockDevice9_DrawRectPatch,
    (void *)LockDevice9_DrawTriPatch,
    (void *)LockDevice9_DeletePatch,
    (void *)LockDevice9_CreateQuery
};

static HRESULT NINE_WINAPI
LockDevice9Ex_SetConvolutionMonoKernel( struct NineDevice9Ex *This,
                                        UINT width,
                                        UINT height,
                                        float *rows,
                                        float *columns )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9Ex_SetConvolutionMonoKernel(This, width, height, rows, columns);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9Ex_ComposeRects( struct NineDevice9Ex *This,
                            IDirect3DSurface9 *pSrc,
                            IDirect3DSurface9 *pDst,
                            IDirect3DVertexBuffer9 *pSrcRectDescs,
                            UINT NumRects,
                            IDirect3DVertexBuffer9 *pDstRectDescs,
                            D3DCOMPOSERECTSOP Operation,
                            int Xoffset,
                            int Yoffset )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9Ex_ComposeRects(This, pSrc, pDst, pSrcRectDescs, NumRects, pDstRectDescs, Operation, Xoffset, Yoffset);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9Ex_PresentEx( struct NineDevice9Ex *This,
                         const RECT *pSourceRect,
                         const RECT *pDestRect,
                         HWND hDestWindowOverride,
                         const RGNDATA *pDirtyRegion,
                         DWORD dwFlags )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9Ex_PresentEx(This, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9Ex_GetGPUThreadPriority( struct NineDevice9Ex *This,
                                    INT *pPriority )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9Ex_GetGPUThreadPriority(This, pPriority);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9Ex_SetGPUThreadPriority( struct NineDevice9Ex *This,
                                    INT Priority )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9Ex_SetGPUThreadPriority(This, Priority);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9Ex_WaitForVBlank( struct NineDevice9Ex *This,
                             UINT iSwapChain )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9Ex_WaitForVBlank(This, iSwapChain);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9Ex_CheckResourceResidency( struct NineDevice9Ex *This,
                                      IDirect3DResource9 **pResourceArray,
                                      UINT32 NumResources )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9Ex_CheckResourceResidency(This, pResourceArray, NumResources);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9Ex_SetMaximumFrameLatency( struct NineDevice9Ex *This,
                                      UINT MaxLatency )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9Ex_SetMaximumFrameLatency(This, MaxLatency);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9Ex_GetMaximumFrameLatency( struct NineDevice9Ex *This,
                                      UINT *pMaxLatency )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9Ex_GetMaximumFrameLatency(This, pMaxLatency);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9Ex_CheckDeviceState( struct NineDevice9Ex *This,
                                HWND hDestinationWindow )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9Ex_CheckDeviceState(This, hDestinationWindow);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9Ex_CreateRenderTargetEx( struct NineDevice9Ex *This,
                                    UINT Width,
                                    UINT Height,
                                    D3DFORMAT Format,
                                    D3DMULTISAMPLE_TYPE MultiSample,
                                    DWORD MultisampleQuality,
                                    BOOL Lockable,
                                    IDirect3DSurface9 **ppSurface,
                                    HANDLE *pSharedHandle,
                                    DWORD Usage )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9Ex_CreateRenderTargetEx(This, Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle, Usage);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9Ex_CreateOffscreenPlainSurfaceEx( struct NineDevice9Ex *This,
                                             UINT Width,
                                             UINT Height,
                                             D3DFORMAT Format,
                                             D3DPOOL Pool,
                                             IDirect3DSurface9 **ppSurface,
                                             HANDLE *pSharedHandle,
                                             DWORD Usage )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9Ex_CreateOffscreenPlainSurfaceEx(This, Width, Height, Format, Pool, ppSurface, pSharedHandle, Usage);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9Ex_CreateDepthStencilSurfaceEx( struct NineDevice9Ex *This,
                                           UINT Width,
                                           UINT Height,
                                           D3DFORMAT Format,
                                           D3DMULTISAMPLE_TYPE MultiSample,
                                           DWORD MultisampleQuality,
                                           BOOL Discard,
                                           IDirect3DSurface9 **ppSurface,
                                           HANDLE *pSharedHandle,
                                           DWORD Usage )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9Ex_CreateDepthStencilSurfaceEx(This, Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle, Usage);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9Ex_ResetEx( struct NineDevice9Ex *This,
                       D3DPRESENT_PARAMETERS *pPresentationParameters,
                       D3DDISPLAYMODEEX *pFullscreenDisplayMode )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9Ex_ResetEx(This, pPresentationParameters, pFullscreenDisplayMode);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9Ex_GetDisplayModeEx( struct NineDevice9Ex *This,
                                UINT iSwapChain,
                                D3DDISPLAYMODEEX *pMode,
                                D3DDISPLAYROTATION *pRotation )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9Ex_GetDisplayModeEx(This, iSwapChain, pMode, pRotation);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

IDirect3DDevice9ExVtbl LockDevice9Ex_vtable = {
    (void *)NineUnknown_QueryInterface,
    (void *)NineUnknown_AddRef,
    (void *)NineUnknown_ReleaseWithDtorLock,
    (void *)LockDevice9_TestCooperativeLevel,
    (void *)LockDevice9_GetAvailableTextureMem,
    (void *)LockDevice9_EvictManagedResources,
    (void *)LockDevice9_GetDirect3D,
    (void *)NineDevice9_GetDeviceCaps,
    (void *)LockDevice9_GetDisplayMode,
    (void *)NineDevice9_GetCreationParameters,
    (void *)LockDevice9_SetCursorProperties,
    (void *)LockDevice9_SetCursorPosition,
    (void *)LockDevice9_ShowCursor,
    (void *)LockDevice9_CreateAdditionalSwapChain,
    (void *)LockDevice9_GetSwapChain,
    (void *)LockDevice9_GetNumberOfSwapChains,
    (void *)LockDevice9_Reset,
    (void *)LockDevice9_Present,
    (void *)LockDevice9_GetBackBuffer,
    (void *)LockDevice9_GetRasterStatus,
    (void *)LockDevice9_SetDialogBoxMode,
    (void *)LockDevice9_SetGammaRamp,
    (void *)LockDevice9_GetGammaRamp,
    (void *)LockDevice9_CreateTexture,
    (void *)LockDevice9_CreateVolumeTexture,
    (void *)LockDevice9_CreateCubeTexture,
    (void *)LockDevice9_CreateVertexBuffer,
    (void *)LockDevice9_CreateIndexBuffer,
    (void *)LockDevice9_CreateRenderTarget,
    (void *)LockDevice9_CreateDepthStencilSurface,
    (void *)LockDevice9_UpdateSurface,
    (void *)LockDevice9_UpdateTexture,
    (void *)LockDevice9_GetRenderTargetData,
    (void *)LockDevice9_GetFrontBufferData,
    (void *)LockDevice9_StretchRect,
    (void *)LockDevice9_ColorFill,
    (void *)LockDevice9_CreateOffscreenPlainSurface,
    (void *)LockDevice9_SetRenderTarget,
    (void *)LockDevice9_GetRenderTarget,
    (void *)LockDevice9_SetDepthStencilSurface,
    (void *)LockDevice9_GetDepthStencilSurface,
    (void *)LockDevice9_BeginScene,
    (void *)LockDevice9_EndScene,
    (void *)LockDevice9_Clear,
    (void *)LockDevice9_SetTransform,
    (void *)LockDevice9_GetTransform,
    (void *)LockDevice9_MultiplyTransform,
    (void *)LockDevice9_SetViewport,
    (void *)LockDevice9_GetViewport,
    (void *)LockDevice9_SetMaterial,
    (void *)LockDevice9_GetMaterial,
    (void *)LockDevice9_SetLight,
    (void *)LockDevice9_GetLight,
    (void *)LockDevice9_LightEnable,
    (void *)LockDevice9_GetLightEnable,
    (void *)LockDevice9_SetClipPlane,
    (void *)LockDevice9_GetClipPlane,
    (void *)LockDevice9_SetRenderState,
    (void *)LockDevice9_GetRenderState,
    (void *)LockDevice9_CreateStateBlock,
    (void *)LockDevice9_BeginStateBlock,
    (void *)LockDevice9_EndStateBlock,
    (void *)LockDevice9_SetClipStatus,
    (void *)LockDevice9_GetClipStatus,
    (void *)LockDevice9_GetTexture,
    (void *)LockDevice9_SetTexture,
    (void *)LockDevice9_GetTextureStageState,
    (void *)LockDevice9_SetTextureStageState,
    (void *)LockDevice9_GetSamplerState,
    (void *)LockDevice9_SetSamplerState,
    (void *)LockDevice9_ValidateDevice,
    (void *)LockDevice9_SetPaletteEntries,
    (void *)LockDevice9_GetPaletteEntries,
    (void *)LockDevice9_SetCurrentTexturePalette,
    (void *)LockDevice9_GetCurrentTexturePalette,
    (void *)LockDevice9_SetScissorRect,
    (void *)LockDevice9_GetScissorRect,
    (void *)LockDevice9_SetSoftwareVertexProcessing,
    (void *)LockDevice9_GetSoftwareVertexProcessing,
    (void *)LockDevice9_SetNPatchMode,
    (void *)LockDevice9_GetNPatchMode,
    (void *)LockDevice9_DrawPrimitive,
    (void *)LockDevice9_DrawIndexedPrimitive,
    (void *)LockDevice9_DrawPrimitiveUP,
    (void *)LockDevice9_DrawIndexedPrimitiveUP,
    (void *)LockDevice9_ProcessVertices,
    (void *)LockDevice9_CreateVertexDeclaration,
    (void *)LockDevice9_SetVertexDeclaration,
    (void *)LockDevice9_GetVertexDeclaration,
    (void *)LockDevice9_SetFVF,
    (void *)LockDevice9_GetFVF,
    (void *)LockDevice9_CreateVertexShader,
    (void *)LockDevice9_SetVertexShader,
    (void *)LockDevice9_GetVertexShader,
    (void *)LockDevice9_SetVertexShaderConstantF,
    (void *)LockDevice9_GetVertexShaderConstantF,
    (void *)LockDevice9_SetVertexShaderConstantI,
    (void *)LockDevice9_GetVertexShaderConstantI,
    (void *)LockDevice9_SetVertexShaderConstantB,
    (void *)LockDevice9_GetVertexShaderConstantB,
    (void *)LockDevice9_SetStreamSource,
    (void *)LockDevice9_GetStreamSource,
    (void *)LockDevice9_SetStreamSourceFreq,
    (void *)LockDevice9_GetStreamSourceFreq,
    (void *)LockDevice9_SetIndices,
    (void *)LockDevice9_GetIndices,
    (void *)LockDevice9_CreatePixelShader,
    (void *)LockDevice9_SetPixelShader,
    (void *)LockDevice9_GetPixelShader,
    (void *)LockDevice9_SetPixelShaderConstantF,
    (void *)LockDevice9_GetPixelShaderConstantF,
    (void *)LockDevice9_SetPixelShaderConstantI,
    (void *)LockDevice9_GetPixelShaderConstantI,
    (void *)LockDevice9_SetPixelShaderConstantB,
    (void *)LockDevice9_GetPixelShaderConstantB,
    (void *)LockDevice9_DrawRectPatch,
    (void *)LockDevice9_DrawTriPatch,
    (void *)LockDevice9_DeletePatch,
    (void *)LockDevice9_CreateQuery,
    (void *)LockDevice9Ex_SetConvolutionMonoKernel,
    (void *)LockDevice9Ex_ComposeRects,
    (void *)LockDevice9Ex_PresentEx,
    (void *)LockDevice9Ex_GetGPUThreadPriority,
    (void *)LockDevice9Ex_SetGPUThreadPriority,
    (void *)LockDevice9Ex_WaitForVBlank,
    (void *)LockDevice9Ex_CheckResourceResidency,
    (void *)LockDevice9Ex_SetMaximumFrameLatency,
    (void *)LockDevice9Ex_GetMaximumFrameLatency,
    (void *)LockDevice9Ex_CheckDeviceState,
    (void *)LockDevice9Ex_CreateRenderTargetEx,
    (void *)LockDevice9Ex_CreateOffscreenPlainSurfaceEx,
    (void *)LockDevice9Ex_CreateDepthStencilSurfaceEx,
    (void *)LockDevice9Ex_ResetEx,
    (void *)LockDevice9Ex_GetDisplayModeEx
};

static HRESULT NINE_WINAPI
LockDevice9Video_GetContentProtectionCaps( struct NineDevice9Video *This,
                                           const GUID *pCryptoType,
                                           const GUID *pDecodeProfile,
                                           D3DCONTENTPROTECTIONCAPS *pCaps )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9Video_GetContentProtectionCaps(This, pCryptoType, pDecodeProfile, pCaps);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9Video_CreateAuthenticatedChannel( struct NineDevice9Video *This,
                                             D3DAUTHENTICATEDCHANNELTYPE ChannelType,
                                             IDirect3DAuthenticatedChannel9 **ppAuthenticatedChannel,
                                             HANDLE *pChannelHandle )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9Video_CreateAuthenticatedChannel(This, ChannelType, ppAuthenticatedChannel, pChannelHandle);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockDevice9Video_CreateCryptoSession( struct NineDevice9Video *This,
                                      const GUID *pCryptoType,
                                      const GUID *pDecodeProfile,
                                      IDirect3DCryptoSession9 **ppCryptoSession,
                                      HANDLE *pCryptoHandle )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineDevice9Video_CreateCryptoSession(This, pCryptoType, pDecodeProfile, ppCryptoSession, pCryptoHandle);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

IDirect3DDevice9VideoVtbl LockDevice9Video_vtable = {
    (void *)NineUnknown_QueryInterface,
    (void *)NineUnknown_AddRef,
    (void *)NineUnknown_ReleaseWithDtorLock,
    (void *)LockDevice9Video_GetContentProtectionCaps,
    (void *)LockDevice9Video_CreateAuthenticatedChannel,
    (void *)LockDevice9Video_CreateCryptoSession
};

static HRESULT NINE_WINAPI
LockIndexBuffer9_Lock( struct NineIndexBuffer9 *This,
                       UINT OffsetToLock,
                       UINT SizeToLock,
                       void **ppbData,
                       DWORD Flags )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineIndexBuffer9_Lock(This, OffsetToLock, SizeToLock, ppbData, Flags);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockIndexBuffer9_Unlock( struct NineIndexBuffer9 *This )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineIndexBuffer9_Unlock(This);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

#if 0
static HRESULT NINE_WINAPI
LockIndexBuffer9_GetDesc( struct NineIndexBuffer9 *This,
                          D3DINDEXBUFFER_DESC *pDesc )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineIndexBuffer9_GetDesc(This, pDesc);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}
#endif

IDirect3DIndexBuffer9Vtbl LockIndexBuffer9_vtable = {
    (void *)NineUnknown_QueryInterface,
    (void *)NineUnknown_AddRef,
    (void *)NineUnknown_ReleaseWithDtorLock,
    (void *)NineUnknown_GetDevice, /* actually part of Resource9 iface */
    (void *)LockUnknown_SetPrivateData,
    (void *)LockUnknown_GetPrivateData,
    (void *)LockUnknown_FreePrivateData,
    (void *)LockResource9_SetPriority,
    (void *)LockResource9_GetPriority,
    (void *)NineResource9_PreLoad, /* nop */
    (void *)NineResource9_GetType, /* immutable */
    (void *)LockIndexBuffer9_Lock,
    (void *)LockIndexBuffer9_Unlock,
    (void *)NineIndexBuffer9_GetDesc /* immutable */
};

#if 0
static HRESULT NINE_WINAPI
LockPixelShader9_GetDevice( struct NinePixelShader9 *This,
                            IDirect3DDevice9 **ppDevice )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineUnknown_GetDevice(NineUnknown(This), ppDevice);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}
#endif

static HRESULT NINE_WINAPI
LockPixelShader9_GetFunction( struct NinePixelShader9 *This,
                              void *pData,
                              UINT *pSizeOfData )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NinePixelShader9_GetFunction(This, pData, pSizeOfData);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

IDirect3DPixelShader9Vtbl LockPixelShader9_vtable = {
    (void *)NineUnknown_QueryInterface,
    (void *)NineUnknown_AddRef,
    (void *)NineUnknown_ReleaseWithDtorLock,
    (void *)NineUnknown_GetDevice,
    (void *)LockPixelShader9_GetFunction
};

#if 0
static HRESULT NINE_WINAPI
LockQuery9_GetDevice( struct NineQuery9 *This,
                      IDirect3DDevice9 **ppDevice )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineUnknown_GetDevice(NineUnknown(This), ppDevice);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}
#endif

#if 0
static D3DQUERYTYPE NINE_WINAPI
LockQuery9_GetType( struct NineQuery9 *This )
{
    D3DQUERYTYPE r;
    simple_mtx_lock(&d3dlock_global);
    r = NineQuery9_GetType(This);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}
#endif

#if 0
static DWORD NINE_WINAPI
LockQuery9_GetDataSize( struct NineQuery9 *This )
{
    DWORD r;
    simple_mtx_lock(&d3dlock_global);
    r = NineQuery9_GetDataSize(This);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}
#endif

static HRESULT NINE_WINAPI
LockQuery9_Issue( struct NineQuery9 *This,
                  DWORD dwIssueFlags )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineQuery9_Issue(This, dwIssueFlags);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockQuery9_GetData( struct NineQuery9 *This,
                    void *pData,
                    DWORD dwSize,
                    DWORD dwGetDataFlags )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineQuery9_GetData(This, pData, dwSize, dwGetDataFlags);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

IDirect3DQuery9Vtbl LockQuery9_vtable = {
    (void *)NineUnknown_QueryInterface,
    (void *)NineUnknown_AddRef,
    (void *)NineUnknown_ReleaseWithDtorLock,
    (void *)NineUnknown_GetDevice, /* actually part of Query9 iface */
    (void *)NineQuery9_GetType, /* immutable */
    (void *)NineQuery9_GetDataSize, /* immutable */
    (void *)LockQuery9_Issue,
    (void *)LockQuery9_GetData
};

#if 0
static HRESULT NINE_WINAPI
LockStateBlock9_GetDevice( struct NineStateBlock9 *This,
                           IDirect3DDevice9 **ppDevice )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineUnknown_GetDevice(NineUnknown(This), ppDevice);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}
#endif

static HRESULT NINE_WINAPI
LockStateBlock9_Capture( struct NineStateBlock9 *This )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineStateBlock9_Capture(This);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockStateBlock9_Apply( struct NineStateBlock9 *This )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineStateBlock9_Apply(This);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

IDirect3DStateBlock9Vtbl LockStateBlock9_vtable = {
    (void *)NineUnknown_QueryInterface,
    (void *)NineUnknown_AddRef,
    (void *)NineUnknown_ReleaseWithDtorLock,
    (void *)NineUnknown_GetDevice, /* actually part of StateBlock9 iface */
    (void *)LockStateBlock9_Capture,
    (void *)LockStateBlock9_Apply
};

static HRESULT NINE_WINAPI
LockSurface9_GetContainer( struct NineSurface9 *This,
                           REFIID riid,
                           void **ppContainer )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineSurface9_GetContainer(This, riid, ppContainer);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

#if 0
static HRESULT NINE_WINAPI
LockSurface9_GetDesc( struct NineSurface9 *This,
                      D3DSURFACE_DESC *pDesc )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineSurface9_GetDesc(This, pDesc);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}
#endif

static HRESULT NINE_WINAPI
LockSurface9_LockRect( struct NineSurface9 *This,
                       D3DLOCKED_RECT *pLockedRect,
                       const RECT *pRect,
                       DWORD Flags )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineSurface9_LockRect(This, pLockedRect, pRect, Flags);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockSurface9_UnlockRect( struct NineSurface9 *This )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineSurface9_UnlockRect(This);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockSurface9_GetDC( struct NineSurface9 *This,
                    HDC *phdc )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineSurface9_GetDC(This, phdc);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockSurface9_ReleaseDC( struct NineSurface9 *This,
                        HDC hdc )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineSurface9_ReleaseDC(This, hdc);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

IDirect3DSurface9Vtbl LockSurface9_vtable = {
    (void *)NineUnknown_QueryInterface,
    (void *)NineUnknown_AddRef,
    (void *)NineUnknown_ReleaseWithDtorLock,
    (void *)NineUnknown_GetDevice, /* actually part of Resource9 iface */
    (void *)LockUnknown_SetPrivateData,
    (void *)LockUnknown_GetPrivateData,
    (void *)LockUnknown_FreePrivateData,
    (void *)LockResource9_SetPriority,
    (void *)LockResource9_GetPriority,
    (void *)NineResource9_PreLoad, /* nop */
    (void *)NineResource9_GetType, /* immutable */
    (void *)LockSurface9_GetContainer,
    (void *)NineSurface9_GetDesc, /* immutable */
    (void *)LockSurface9_LockRect,
    (void *)LockSurface9_UnlockRect,
    (void *)LockSurface9_GetDC,
    (void *)LockSurface9_ReleaseDC
};

static HRESULT NINE_WINAPI
LockSwapChain9_Present( struct NineSwapChain9 *This,
                        const RECT *pSourceRect,
                        const RECT *pDestRect,
                        HWND hDestWindowOverride,
                        const RGNDATA *pDirtyRegion,
                        DWORD dwFlags )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineSwapChain9_Present(This, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockSwapChain9_GetFrontBufferData( struct NineSwapChain9 *This,
                                   IDirect3DSurface9 *pDestSurface )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineSwapChain9_GetFrontBufferData(This, pDestSurface);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockSwapChain9_GetBackBuffer( struct NineSwapChain9 *This,
                              UINT iBackBuffer,
                              D3DBACKBUFFER_TYPE Type,
                              IDirect3DSurface9 **ppBackBuffer )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineSwapChain9_GetBackBuffer(This, iBackBuffer, Type, ppBackBuffer);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockSwapChain9_GetRasterStatus( struct NineSwapChain9 *This,
                                D3DRASTER_STATUS *pRasterStatus )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineSwapChain9_GetRasterStatus(This, pRasterStatus);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockSwapChain9_GetDisplayMode( struct NineSwapChain9 *This,
                               D3DDISPLAYMODE *pMode )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineSwapChain9_GetDisplayMode(This, pMode);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

#if 0
static HRESULT NINE_WINAPI
LockSwapChain9_GetDevice( struct NineSwapChain9 *This,
                          IDirect3DDevice9 **ppDevice )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineUnknown_GetDevice(NineUnknown(This), ppDevice);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}
#endif

static HRESULT NINE_WINAPI
LockSwapChain9_GetPresentParameters( struct NineSwapChain9 *This,
                                     D3DPRESENT_PARAMETERS *pPresentationParameters )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineSwapChain9_GetPresentParameters(This, pPresentationParameters);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

IDirect3DSwapChain9Vtbl LockSwapChain9_vtable = {
    (void *)NineUnknown_QueryInterface,
    (void *)NineUnknown_AddRef,
    (void *)NineUnknown_ReleaseWithDtorLock,
    (void *)LockSwapChain9_Present,
    (void *)LockSwapChain9_GetFrontBufferData,
    (void *)LockSwapChain9_GetBackBuffer,
    (void *)LockSwapChain9_GetRasterStatus,
    (void *)LockSwapChain9_GetDisplayMode,
    (void *)NineUnknown_GetDevice, /* actually part of SwapChain9 iface */
    (void *)LockSwapChain9_GetPresentParameters
};

static HRESULT NINE_WINAPI
LockSwapChain9Ex_GetLastPresentCount( struct NineSwapChain9Ex *This,
                                      UINT *pLastPresentCount )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineSwapChain9Ex_GetLastPresentCount(This, pLastPresentCount);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockSwapChain9Ex_GetPresentStats( struct NineSwapChain9Ex *This,
                                  D3DPRESENTSTATS *pPresentationStatistics )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineSwapChain9Ex_GetPresentStats(This, pPresentationStatistics);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockSwapChain9Ex_GetDisplayModeEx( struct NineSwapChain9Ex *This,
                                   D3DDISPLAYMODEEX *pMode,
                                   D3DDISPLAYROTATION *pRotation )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineSwapChain9Ex_GetDisplayModeEx(This, pMode, pRotation);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

IDirect3DSwapChain9ExVtbl LockSwapChain9Ex_vtable = {
    (void *)NineUnknown_QueryInterface,
    (void *)NineUnknown_AddRef,
    (void *)NineUnknown_ReleaseWithDtorLock,
    (void *)LockSwapChain9_Present,
    (void *)LockSwapChain9_GetFrontBufferData,
    (void *)LockSwapChain9_GetBackBuffer,
    (void *)LockSwapChain9_GetRasterStatus,
    (void *)LockSwapChain9_GetDisplayMode,
    (void *)NineUnknown_GetDevice, /* actually part of NineSwapChain9 iface */
    (void *)LockSwapChain9_GetPresentParameters,
    (void *)LockSwapChain9Ex_GetLastPresentCount,
    (void *)LockSwapChain9Ex_GetPresentStats,
    (void *)LockSwapChain9Ex_GetDisplayModeEx
};

#if 0
static HRESULT NINE_WINAPI
LockTexture9_GetLevelDesc( struct NineTexture9 *This,
                           UINT Level,
                           D3DSURFACE_DESC *pDesc )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineTexture9_GetLevelDesc(This, Level, pDesc);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}
#endif

#if 0
static HRESULT NINE_WINAPI
LockTexture9_GetSurfaceLevel( struct NineTexture9 *This,
                              UINT Level,
                              IDirect3DSurface9 **ppSurfaceLevel )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineTexture9_GetSurfaceLevel(This, Level, ppSurfaceLevel);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}
#endif

static HRESULT NINE_WINAPI
LockTexture9_LockRect( struct NineTexture9 *This,
                       UINT Level,
                       D3DLOCKED_RECT *pLockedRect,
                       const RECT *pRect,
                       DWORD Flags )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineTexture9_LockRect(This, Level, pLockedRect, pRect, Flags);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockTexture9_UnlockRect( struct NineTexture9 *This,
                         UINT Level )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineTexture9_UnlockRect(This, Level);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockTexture9_AddDirtyRect( struct NineTexture9 *This,
                           const RECT *pDirtyRect )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineTexture9_AddDirtyRect(This, pDirtyRect);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

IDirect3DTexture9Vtbl LockTexture9_vtable = {
    (void *)NineUnknown_QueryInterface,
    (void *)NineUnknown_AddRef,
    (void *)NineUnknown_ReleaseWithDtorLock,
    (void *)NineUnknown_GetDevice, /* actually part of Resource9 iface */
    (void *)LockUnknown_SetPrivateData,
    (void *)LockUnknown_GetPrivateData,
    (void *)LockUnknown_FreePrivateData,
    (void *)LockResource9_SetPriority,
    (void *)LockResource9_GetPriority,
    (void *)LockBaseTexture9_PreLoad,
    (void *)NineResource9_GetType, /* immutable */
    (void *)LockBaseTexture9_SetLOD,
    (void *)LockBaseTexture9_GetLOD,
    (void *)LockBaseTexture9_GetLevelCount,
    (void *)LockBaseTexture9_SetAutoGenFilterType,
    (void *)LockBaseTexture9_GetAutoGenFilterType,
    (void *)LockBaseTexture9_GenerateMipSubLevels,
    (void *)NineTexture9_GetLevelDesc, /* immutable */
    (void *)NineTexture9_GetSurfaceLevel, /* AddRef */
    (void *)LockTexture9_LockRect,
    (void *)LockTexture9_UnlockRect,
    (void *)LockTexture9_AddDirtyRect
};

static HRESULT NINE_WINAPI
LockVertexBuffer9_Lock( struct NineVertexBuffer9 *This,
                        UINT OffsetToLock,
                        UINT SizeToLock,
                        void **ppbData,
                        DWORD Flags )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineVertexBuffer9_Lock(This, OffsetToLock, SizeToLock, ppbData, Flags);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockVertexBuffer9_Unlock( struct NineVertexBuffer9 *This )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineVertexBuffer9_Unlock(This);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

#if 0
static HRESULT NINE_WINAPI
LockVertexBuffer9_GetDesc( struct NineVertexBuffer9 *This,
                           D3DVERTEXBUFFER_DESC *pDesc )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineVertexBuffer9_GetDesc(This, pDesc);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}
#endif

IDirect3DVertexBuffer9Vtbl LockVertexBuffer9_vtable = {
    (void *)NineUnknown_QueryInterface,
    (void *)NineUnknown_AddRef,
    (void *)NineUnknown_ReleaseWithDtorLock,
    (void *)NineUnknown_GetDevice, /* actually part of Resource9 iface */
    (void *)LockUnknown_SetPrivateData,
    (void *)LockUnknown_GetPrivateData,
    (void *)LockUnknown_FreePrivateData,
    (void *)LockResource9_SetPriority,
    (void *)LockResource9_GetPriority,
    (void *)NineResource9_PreLoad, /* nop */
    (void *)NineResource9_GetType, /* immutable */
    (void *)LockVertexBuffer9_Lock,
    (void *)LockVertexBuffer9_Unlock,
    (void *)NineVertexBuffer9_GetDesc /* immutable */
};

#if 0
static HRESULT NINE_WINAPI
LockVertexDeclaration9_GetDevice( struct NineVertexDeclaration9 *This,
                                  IDirect3DDevice9 **ppDevice )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineUnknown_GetDevice(NineUnknown(This), ppDevice);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}
#endif

static HRESULT NINE_WINAPI
LockVertexDeclaration9_GetDeclaration( struct NineVertexDeclaration9 *This,
                                       D3DVERTEXELEMENT9 *pElement,
                                       UINT *pNumElements )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineVertexDeclaration9_GetDeclaration(This, pElement, pNumElements);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

IDirect3DVertexDeclaration9Vtbl LockVertexDeclaration9_vtable = {
    (void *)NineUnknown_QueryInterface,
    (void *)NineUnknown_AddRef,
    (void *)NineUnknown_ReleaseWithDtorLock,
    (void *)NineUnknown_GetDevice, /* actually part of VertexDecl9 iface */
    (void *)LockVertexDeclaration9_GetDeclaration
};

#if 0
static HRESULT NINE_WINAPI
LockVertexShader9_GetDevice( struct NineVertexShader9 *This,
                             IDirect3DDevice9 **ppDevice )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineUnknown_GetDevice(NineUnknown(This), ppDevice);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}
#endif

static HRESULT NINE_WINAPI
LockVertexShader9_GetFunction( struct NineVertexShader9 *This,
                               void *pData,
                               UINT *pSizeOfData )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineVertexShader9_GetFunction(This, pData, pSizeOfData);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

IDirect3DVertexShader9Vtbl LockVertexShader9_vtable = {
    (void *)NineUnknown_QueryInterface,
    (void *)NineUnknown_AddRef,
    (void *)NineUnknown_ReleaseWithDtorLock,
    (void *)NineUnknown_GetDevice,
    (void *)LockVertexShader9_GetFunction
};

#if 0
static HRESULT NINE_WINAPI
LockVolume9_GetDevice( struct NineVolume9 *This,
                       IDirect3DDevice9 **ppDevice )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineUnknown_GetDevice(NineUnknown(This), ppDevice);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}
#endif

static HRESULT NINE_WINAPI
LockVolume9_GetContainer( struct NineVolume9 *This,
                          REFIID riid,
                          void **ppContainer )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineVolume9_GetContainer(This, riid, ppContainer);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

#if 0
static HRESULT NINE_WINAPI
LockVolume9_GetDesc( struct NineVolume9 *This,
                     D3DVOLUME_DESC *pDesc )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineVolume9_GetDesc(This, pDesc);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}
#endif

static HRESULT NINE_WINAPI
LockVolume9_LockBox( struct NineVolume9 *This,
                     D3DLOCKED_BOX *pLockedVolume,
                     const D3DBOX *pBox,
                     DWORD Flags )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineVolume9_LockBox(This, pLockedVolume, pBox, Flags);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockVolume9_UnlockBox( struct NineVolume9 *This )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineVolume9_UnlockBox(This);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

IDirect3DVolume9Vtbl LockVolume9_vtable = {
    (void *)NineUnknown_QueryInterface,
    (void *)NineUnknown_AddRef,
    (void *)NineUnknown_ReleaseWithDtorLock,
    (void *)NineUnknown_GetDevice, /* actually part of Volume9 iface */
    (void *)LockUnknown_SetPrivateData,
    (void *)LockUnknown_GetPrivateData,
    (void *)LockUnknown_FreePrivateData,
    (void *)LockVolume9_GetContainer,
    (void *)NineVolume9_GetDesc, /* immutable */
    (void *)LockVolume9_LockBox,
    (void *)LockVolume9_UnlockBox
};

#if 0
static HRESULT NINE_WINAPI
LockVolumeTexture9_GetLevelDesc( struct NineVolumeTexture9 *This,
                                 UINT Level,
                                 D3DVOLUME_DESC *pDesc )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineVolumeTexture9_GetLevelDesc(This, Level, pDesc);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}
#endif

#if 0
static HRESULT NINE_WINAPI
LockVolumeTexture9_GetVolumeLevel( struct NineVolumeTexture9 *This,
                                   UINT Level,
                                   IDirect3DVolume9 **ppVolumeLevel )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineVolumeTexture9_GetVolumeLevel(This, Level, ppVolumeLevel);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}
#endif

static HRESULT NINE_WINAPI
LockVolumeTexture9_LockBox( struct NineVolumeTexture9 *This,
                            UINT Level,
                            D3DLOCKED_BOX *pLockedVolume,
                            const D3DBOX *pBox,
                            DWORD Flags )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineVolumeTexture9_LockBox(This, Level, pLockedVolume, pBox, Flags);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockVolumeTexture9_UnlockBox( struct NineVolumeTexture9 *This,
                              UINT Level )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineVolumeTexture9_UnlockBox(This, Level);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

static HRESULT NINE_WINAPI
LockVolumeTexture9_AddDirtyBox( struct NineVolumeTexture9 *This,
                                const D3DBOX *pDirtyBox )
{
    HRESULT r;
    simple_mtx_lock(&d3dlock_global);
    r = NineVolumeTexture9_AddDirtyBox(This, pDirtyBox);
    simple_mtx_unlock(&d3dlock_global);
    return r;
}

IDirect3DVolumeTexture9Vtbl LockVolumeTexture9_vtable = {
    (void *)NineUnknown_QueryInterface,
    (void *)NineUnknown_AddRef,
    (void *)NineUnknown_ReleaseWithDtorLock,
    (void *)NineUnknown_GetDevice, /* actually part of Resource9 iface */
    (void *)LockUnknown_SetPrivateData,
    (void *)LockUnknown_GetPrivateData,
    (void *)LockUnknown_FreePrivateData,
    (void *)LockResource9_SetPriority,
    (void *)LockResource9_GetPriority,
    (void *)LockBaseTexture9_PreLoad,
    (void *)NineResource9_GetType, /* immutable */
    (void *)LockBaseTexture9_SetLOD,
    (void *)LockBaseTexture9_GetLOD,
    (void *)LockBaseTexture9_GetLevelCount,
    (void *)LockBaseTexture9_SetAutoGenFilterType,
    (void *)LockBaseTexture9_GetAutoGenFilterType,
    (void *)LockBaseTexture9_GenerateMipSubLevels,
    (void *)NineVolumeTexture9_GetLevelDesc, /* immutable */
    (void *)NineVolumeTexture9_GetVolumeLevel, /* AddRef */
    (void *)LockVolumeTexture9_LockBox,
    (void *)LockVolumeTexture9_UnlockBox,
    (void *)LockVolumeTexture9_AddDirtyBox
};

ID3DAdapter9Vtbl LockAdapter9_vtable = { /* not used */
    (void *)NULL,
    (void *)NULL,
    (void *)NULL,
    (void *)NULL,
    (void *)NULL,
    (void *)NULL,
    (void *)NULL,
    (void *)NULL,
    (void *)NULL,
    (void *)NULL,
    (void *)NULL,
    (void *)NULL
};
