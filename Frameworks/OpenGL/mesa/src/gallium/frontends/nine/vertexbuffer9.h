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

#ifndef _NINE_VERTEXBUFFER9_H_
#define _NINE_VERTEXBUFFER9_H_
#include "resource9.h"
#include "buffer9.h"

struct pipe_screen;
struct pipe_context;
struct pipe_transfer;

struct NineVertexBuffer9
{
    struct NineBuffer9 base;

    /* G3D */
    struct pipe_context *pipe;
    D3DVERTEXBUFFER_DESC desc;
};
static inline struct NineVertexBuffer9 *
NineVertexBuffer9( void *data )
{
    return (struct NineVertexBuffer9 *)data;
}

HRESULT
NineVertexBuffer9_new( struct NineDevice9 *pDevice,
                       D3DVERTEXBUFFER_DESC *pDesc,
                       struct NineVertexBuffer9 **ppOut );

HRESULT
NineVertexBuffer9_ctor( struct NineVertexBuffer9 *This,
                        struct NineUnknownParams *pParams,
                        D3DVERTEXBUFFER_DESC *pDesc );

void
NineVertexBuffer9_dtor( struct NineVertexBuffer9 *This );
/*** Nine private ***/

struct pipe_resource *
NineVertexBuffer9_GetResource( struct NineVertexBuffer9 *This, unsigned *offset );

/*** Direct3D public ***/

HRESULT NINE_WINAPI
NineVertexBuffer9_Lock( struct NineVertexBuffer9 *This,
                        UINT OffsetToLock,
                        UINT SizeToLock,
                        void **ppbData,
                        DWORD Flags );

HRESULT NINE_WINAPI
NineVertexBuffer9_Unlock( struct NineVertexBuffer9 *This );

HRESULT NINE_WINAPI
NineVertexBuffer9_GetDesc( struct NineVertexBuffer9 *This,
                           D3DVERTEXBUFFER_DESC *pDesc );

#endif /* _NINE_VERTEXBUFFER9_H_ */
