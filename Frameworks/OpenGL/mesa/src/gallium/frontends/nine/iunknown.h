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

#ifndef _NINE_IUNKNOWN_H_
#define _NINE_IUNKNOWN_H_

#include "util/compiler.h"

#include "util/u_atomic.h"
#include "util/u_memory.h"

#include "guid.h"
#include "nine_flags.h"
#include "nine_debug.h"
#include "nine_quirk.h"

#include "d3d9.h"

struct Nine9;
struct NineDevice9;

struct NineUnknown
{
    /* pointer to vtable (can be overriden outside gallium nine) */
    void *vtable;
    /* pointer to internal vtable  */
    void *vtable_internal;

    int32_t refs; /* external reference count */
    int32_t bind; /* internal bind count */
    bool forward; /* whether to forward references to the container */

    /* container: for surfaces and volumes only.
     * Can be a texture, a volume texture or a swapchain.
     * forward is set to false for the swapchain case.
     * If forward is set, refs are passed to the container if forward is set
     * and the container has bind increased if the object has non null bind. */
    struct NineUnknown *container;
    struct NineDevice9 *device;    /* referenced if (refs) */

    const GUID **guids; /* for QueryInterface */

    /* for [GS]etPrivateData/FreePrivateData */
    struct hash_table *pdata;

    void (*dtor)(void *data); /* top-level dtor */
};
static inline struct NineUnknown *
NineUnknown( void *data )
{
    return (struct NineUnknown *)data;
}

/* Use this instead of a shitload of arguments: */
struct NineUnknownParams
{
    void *vtable;
    const GUID **guids;
    void (*dtor)(void *data);
    struct NineUnknown *container;
    struct NineDevice9 *device;
    bool start_with_bind_not_ref;
};

HRESULT
NineUnknown_ctor( struct NineUnknown *This,
                  struct NineUnknownParams *pParams );

void
NineUnknown_dtor( struct NineUnknown *This );

/*** Direct3D public methods ***/

HRESULT NINE_WINAPI
NineUnknown_QueryInterface( struct NineUnknown *This,
                            REFIID riid,
                            void **ppvObject );

ULONG NINE_WINAPI
NineUnknown_AddRef( struct NineUnknown *This );

ULONG NINE_WINAPI
NineUnknown_Release( struct NineUnknown *This );

ULONG NINE_WINAPI
NineUnknown_ReleaseWithDtorLock( struct NineUnknown *This );

HRESULT NINE_WINAPI
NineUnknown_GetDevice( struct NineUnknown *This,
                       IDirect3DDevice9 **ppDevice );

HRESULT NINE_WINAPI
NineUnknown_SetPrivateData( struct NineUnknown *This,
                            REFGUID refguid,
                            const void *pData,
                            DWORD SizeOfData,
                            DWORD Flags );

HRESULT NINE_WINAPI
NineUnknown_GetPrivateData( struct NineUnknown *This,
                            REFGUID refguid,
                            void *pData,
                            DWORD *pSizeOfData );

HRESULT NINE_WINAPI
NineUnknown_FreePrivateData( struct NineUnknown *This,
                             REFGUID refguid );

/*** Nine private methods ***/

static inline void
NineUnknown_Destroy( struct NineUnknown *This )
{
    assert(!(This->refs | This->bind));
    This->dtor(This);
}

static inline UINT
NineUnknown_Bind( struct NineUnknown *This )
{
    UINT b = p_atomic_inc_return(&This->bind);
    assert(b);

    if (b == 1 && This->forward)
        NineUnknown_Bind(This->container);

    return b;
}

static inline UINT
NineUnknown_Unbind( struct NineUnknown *This )
{
    UINT b = p_atomic_dec_return(&This->bind);

    if (b == 0 && This->forward)
        NineUnknown_Unbind(This->container);
    else if (b == 0 && This->refs == 0 && !This->container)
        This->dtor(This);

    return b;
}

static inline void
NineUnknown_ConvertRefToBind( struct NineUnknown *This )
{
    NineUnknown_Bind(This);
    NineUnknown_Release(This);
}

/* Detach from container. */
static inline void
NineUnknown_Detach( struct NineUnknown *This )
{
    assert(This->container && !This->forward);

    This->container = NULL;
    if (!(This->refs | This->bind))
        This->dtor(This);
}

#endif /* _NINE_IUNKNOWN_H_ */
