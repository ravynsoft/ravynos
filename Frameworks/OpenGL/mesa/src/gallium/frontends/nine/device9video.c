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

#include "device9video.h"

#define DBG_CHANNEL DBG_DEVICEVIDEO

HRESULT NINE_WINAPI
NineDevice9Video_GetContentProtectionCaps( struct NineDevice9Video *This,
                                           const GUID *pCryptoType,
                                           const GUID *pDecodeProfile,
                                           D3DCONTENTPROTECTIONCAPS *pCaps )
{
    STUB(D3DERR_INVALIDCALL);
}

HRESULT NINE_WINAPI
NineDevice9Video_CreateAuthenticatedChannel( struct NineDevice9Video *This,
                                             D3DAUTHENTICATEDCHANNELTYPE ChannelType,
                                             IDirect3DAuthenticatedChannel9 **ppAuthenticatedChannel,
                                             HANDLE *pChannelHandle )
{
    STUB(D3DERR_INVALIDCALL);
}

HRESULT NINE_WINAPI
NineDevice9Video_CreateCryptoSession( struct NineDevice9Video *This,
                                      const GUID *pCryptoType,
                                      const GUID *pDecodeProfile,
                                      IDirect3DCryptoSession9 **ppCryptoSession,
                                      HANDLE *pCryptoHandle )
{
    STUB(D3DERR_INVALIDCALL);
}

IDirect3DDevice9VideoVtbl NineDevice9Video_vtable = {
    (void *)NineUnknown_QueryInterface,
    (void *)NineUnknown_AddRef,
    (void *)NineUnknown_Release,
    (void *)NineDevice9Video_GetContentProtectionCaps,
    (void *)NineDevice9Video_CreateAuthenticatedChannel,
    (void *)NineDevice9Video_CreateCryptoSession
};
