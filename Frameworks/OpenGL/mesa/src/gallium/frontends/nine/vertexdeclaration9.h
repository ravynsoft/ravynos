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

#ifndef _NINE_VERTEXDECLARATION9_H_
#define _NINE_VERTEXDECLARATION9_H_

#include "nine_defines.h"
#include "iunknown.h"

struct pipe_resource;
struct pipe_vertex_element;
struct pipe_stream_output_info;
struct NineDevice9;
struct NineVertexBuffer9;
struct nine_vs_output_info;

struct NineVertexDeclaration9
{
    struct NineUnknown base;

    /* G3D state */
    struct pipe_vertex_element *elems;
    unsigned nelems;

    /* index -> DECLUSAGE, for selecting the vertex element
     * for each VS input */
    uint16_t *usage_map;

    D3DVERTEXELEMENT9 *decls;
    DWORD fvf;

    BOOL position_t;
};
static inline struct NineVertexDeclaration9 *
NineVertexDeclaration9( void *data )
{
    return (struct NineVertexDeclaration9 *)data;
}

HRESULT
NineVertexDeclaration9_new( struct NineDevice9 *pDevice,
                            const D3DVERTEXELEMENT9 *pElements,
                            struct NineVertexDeclaration9 **ppOut );

HRESULT
NineVertexDeclaration9_new_from_fvf( struct NineDevice9 *pDevice,
                                     DWORD FVF,
                                     struct NineVertexDeclaration9 **ppOut );

HRESULT
NineVertexDeclaration9_ctor( struct NineVertexDeclaration9 *This,
                             struct NineUnknownParams *pParams,
                             const D3DVERTEXELEMENT9 *pElements );

void
NineVertexDeclaration9_dtor( struct NineVertexDeclaration9 *This );

HRESULT NINE_WINAPI
NineVertexDeclaration9_GetDeclaration( struct NineVertexDeclaration9 *This,
                                       D3DVERTEXELEMENT9 *pElement,
                                       UINT *pNumElements );

void
NineVertexDeclaration9_FillStreamOutputInfo(
    struct NineVertexDeclaration9 *This,
    struct nine_vs_output_info *ShaderOutputsInfo,
    unsigned numOutputs,
    struct pipe_stream_output_info *so );

/* Convert stream output data to the vertex declaration's format. */
HRESULT
NineVertexDeclaration9_ConvertStreamOutput(
    struct NineVertexDeclaration9 *This,
    struct NineVertexBuffer9 *pDstBuf,
    UINT DestIndex,
    UINT VertexCount,
    void *pSrcBuf,
    const struct pipe_stream_output_info *so );

#endif /* _NINE_VERTEXDECLARATION9_H_ */
