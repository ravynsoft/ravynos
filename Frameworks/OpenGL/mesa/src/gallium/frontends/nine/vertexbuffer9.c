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

#include "vertexbuffer9.h"
#include "device9.h"
#include "nine_helpers.h"
#include "nine_pipe.h"

#include "pipe/p_screen.h"
#include "pipe/p_context.h"
#include "pipe/p_state.h"
#include "pipe/p_defines.h"
#include "util/format/u_formats.h"
#include "util/u_box.h"

#define DBG_CHANNEL DBG_VERTEXBUFFER

HRESULT
NineVertexBuffer9_ctor( struct NineVertexBuffer9 *This,
                        struct NineUnknownParams *pParams,
                        D3DVERTEXBUFFER_DESC *pDesc )
{
    HRESULT hr;

    DBG("This=%p Size=0x%x Usage=%x Pool=%u\n", This,
        pDesc->Size, pDesc->Usage, pDesc->Pool);

    hr = NineBuffer9_ctor(&This->base, pParams, D3DRTYPE_VERTEXBUFFER,
                          pDesc->Usage, pDesc->Size, pDesc->Pool);
    if (FAILED(hr))
        return hr;

    pDesc->Type = D3DRTYPE_VERTEXBUFFER;
    pDesc->Format = D3DFMT_VERTEXDATA;
    This->desc = *pDesc;

    return D3D_OK;
}

void
NineVertexBuffer9_dtor( struct NineVertexBuffer9 *This )
{
    NineBuffer9_dtor(&This->base);
}

struct pipe_resource *
NineVertexBuffer9_GetResource( struct NineVertexBuffer9 *This, unsigned *offset )
{
    return NineBuffer9_GetResource(&This->base, offset);
}

HRESULT NINE_WINAPI
NineVertexBuffer9_Lock( struct NineVertexBuffer9 *This,
                       UINT OffsetToLock,
                       UINT SizeToLock,
                       void **ppbData,
                       DWORD Flags )
{
    return NineBuffer9_Lock(&This->base, OffsetToLock, SizeToLock, ppbData, Flags);
}

HRESULT NINE_WINAPI
NineVertexBuffer9_Unlock( struct NineVertexBuffer9 *This )
{
    return NineBuffer9_Unlock(&This->base);
}

HRESULT NINE_WINAPI
NineVertexBuffer9_GetDesc( struct NineVertexBuffer9 *This,
                           D3DVERTEXBUFFER_DESC *pDesc )
{
    user_assert(pDesc, E_POINTER);
    *pDesc = This->desc;
    return D3D_OK;
}

IDirect3DVertexBuffer9Vtbl NineVertexBuffer9_vtable = {
    (void *)NineUnknown_QueryInterface,
    (void *)NineUnknown_AddRef,
    (void *)NineUnknown_Release,
    (void *)NineUnknown_GetDevice, /* actually part of Resource9 iface */
    (void *)NineUnknown_SetPrivateData,
    (void *)NineUnknown_GetPrivateData,
    (void *)NineUnknown_FreePrivateData,
    (void *)NineResource9_SetPriority,
    (void *)NineResource9_GetPriority,
    (void *)NineResource9_PreLoad,
    (void *)NineResource9_GetType,
    (void *)NineVertexBuffer9_Lock,
    (void *)NineVertexBuffer9_Unlock,
    (void *)NineVertexBuffer9_GetDesc
};

static const GUID *NineVertexBuffer9_IIDs[] = {
    &IID_IDirect3DVertexBuffer9,
    &IID_IDirect3DResource9,
    &IID_IUnknown,
    NULL
};

HRESULT
NineVertexBuffer9_new( struct NineDevice9 *pDevice,
                       D3DVERTEXBUFFER_DESC *pDesc,
                       struct NineVertexBuffer9 **ppOut )
{
    NINE_DEVICE_CHILD_NEW(VertexBuffer9, ppOut, /* args */ pDevice, pDesc);
}
