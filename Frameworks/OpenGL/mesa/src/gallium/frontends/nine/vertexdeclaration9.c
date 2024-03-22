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

#include "vertexdeclaration9.h"
#include "vertexbuffer9.h"
#include "device9.h"
#include "nine_helpers.h"
#include "nine_shader.h"

#include "util/format/u_formats.h"
#include "pipe/p_context.h"
#include "util/u_math.h"
#include "util/format/u_format.h"
#include "translate/translate.h"

#define DBG_CHANNEL DBG_VERTEXDECLARATION

static inline enum pipe_format decltype_format(BYTE type)
{
    switch (type) {
    case D3DDECLTYPE_FLOAT1:    return PIPE_FORMAT_R32_FLOAT;
    case D3DDECLTYPE_FLOAT2:    return PIPE_FORMAT_R32G32_FLOAT;
    case D3DDECLTYPE_FLOAT3:    return PIPE_FORMAT_R32G32B32_FLOAT;
    case D3DDECLTYPE_FLOAT4:    return PIPE_FORMAT_R32G32B32A32_FLOAT;
    case D3DDECLTYPE_D3DCOLOR:  return PIPE_FORMAT_B8G8R8A8_UNORM;
    case D3DDECLTYPE_UBYTE4:    return PIPE_FORMAT_R8G8B8A8_USCALED;
    case D3DDECLTYPE_SHORT2:    return PIPE_FORMAT_R16G16_SSCALED;
    case D3DDECLTYPE_SHORT4:    return PIPE_FORMAT_R16G16B16A16_SSCALED;
    case D3DDECLTYPE_UBYTE4N:   return PIPE_FORMAT_R8G8B8A8_UNORM;
    case D3DDECLTYPE_SHORT2N:   return PIPE_FORMAT_R16G16_SNORM;
    case D3DDECLTYPE_SHORT4N:   return PIPE_FORMAT_R16G16B16A16_SNORM;
    case D3DDECLTYPE_USHORT2N:  return PIPE_FORMAT_R16G16_UNORM;
    case D3DDECLTYPE_USHORT4N:  return PIPE_FORMAT_R16G16B16A16_UNORM;
    case D3DDECLTYPE_UDEC3:     return PIPE_FORMAT_R10G10B10X2_USCALED;
    case D3DDECLTYPE_DEC3N:     return PIPE_FORMAT_R10G10B10X2_SNORM;
    case D3DDECLTYPE_FLOAT16_2: return PIPE_FORMAT_R16G16_FLOAT;
    case D3DDECLTYPE_FLOAT16_4: return PIPE_FORMAT_R16G16B16A16_FLOAT;
    default:
        assert(!"Implementation error !");
    }
    return PIPE_FORMAT_NONE;
}

static inline unsigned decltype_size(BYTE type)
{
    switch (type) {
    case D3DDECLTYPE_FLOAT1: return 1 * sizeof(float);
    case D3DDECLTYPE_FLOAT2: return 2 * sizeof(float);
    case D3DDECLTYPE_FLOAT3: return 3 * sizeof(float);
    case D3DDECLTYPE_FLOAT4: return 4 * sizeof(float);
    case D3DDECLTYPE_D3DCOLOR: return 1 * sizeof(DWORD);
    case D3DDECLTYPE_UBYTE4: return 4 * sizeof(BYTE);
    case D3DDECLTYPE_SHORT2: return 2 * sizeof(short);
    case D3DDECLTYPE_SHORT4: return 4 * sizeof(short);
    case D3DDECLTYPE_UBYTE4N: return 4 * sizeof(BYTE);
    case D3DDECLTYPE_SHORT2N: return 2 * sizeof(short);
    case D3DDECLTYPE_SHORT4N: return 4 * sizeof(short);
    case D3DDECLTYPE_USHORT2N: return 2 * sizeof(short);
    case D3DDECLTYPE_USHORT4N: return 4 * sizeof(short);
    case D3DDECLTYPE_UDEC3: return 4;
    case D3DDECLTYPE_DEC3N: return 4;
    case D3DDECLTYPE_FLOAT16_2: return 2 * 2;
    case D3DDECLTYPE_FLOAT16_4: return 4 * 2;
    default:
        assert(!"Implementation error !");
    }
    return 0;
}

/* Actually, arbitrary usage index values are permitted, but a
 * simple lookup table won't work in that case. Let's just wait
 * with making this more generic until we need it.
 */
static inline bool
nine_d3ddeclusage_check(unsigned usage, unsigned usage_idx)
{
    switch (usage) {
    case D3DDECLUSAGE_POSITIONT:
    case D3DDECLUSAGE_TESSFACTOR:
    case D3DDECLUSAGE_DEPTH:
    case D3DDECLUSAGE_NORMAL:
    case D3DDECLUSAGE_TANGENT:
    case D3DDECLUSAGE_BINORMAL:
    case D3DDECLUSAGE_POSITION:
    case D3DDECLUSAGE_BLENDWEIGHT:
    case D3DDECLUSAGE_BLENDINDICES:
    case D3DDECLUSAGE_COLOR:
        return true;
    case D3DDECLUSAGE_PSIZE:
    case D3DDECLUSAGE_FOG:
    case D3DDECLUSAGE_SAMPLE:
        return usage_idx <= 0;
    case D3DDECLUSAGE_TEXCOORD:
        return usage_idx <= 15;
    default:
        return false;
    }
}

#define NINE_DECLUSAGE_CASE0(n) case D3DDECLUSAGE_##n: return NINE_DECLUSAGE_##n
#define NINE_DECLUSAGE_CASEi(n) case D3DDECLUSAGE_##n: return NINE_DECLUSAGE_i(n, usage_idx)
uint16_t
nine_d3d9_to_nine_declusage(unsigned usage, unsigned usage_idx)
{
    if (!nine_d3ddeclusage_check(usage, usage_idx))
        ERR("D3DDECLUSAGE_%u[%u]\n",usage,usage_idx);
    assert(nine_d3ddeclusage_check(usage, usage_idx));
    switch (usage) {
    NINE_DECLUSAGE_CASEi(POSITION);
    NINE_DECLUSAGE_CASEi(BLENDWEIGHT);
    NINE_DECLUSAGE_CASEi(BLENDINDICES);
    NINE_DECLUSAGE_CASEi(NORMAL);
    NINE_DECLUSAGE_CASE0(PSIZE);
    NINE_DECLUSAGE_CASEi(TEXCOORD);
    NINE_DECLUSAGE_CASEi(TANGENT);
    NINE_DECLUSAGE_CASEi(BINORMAL);
    NINE_DECLUSAGE_CASE0(TESSFACTOR);
    NINE_DECLUSAGE_CASEi(POSITIONT);
    NINE_DECLUSAGE_CASEi(COLOR);
    NINE_DECLUSAGE_CASE0(DEPTH);
    NINE_DECLUSAGE_CASE0(FOG);
    NINE_DECLUSAGE_CASE0(SAMPLE);
    default:
        assert(!"Invalid DECLUSAGE.");
        return NINE_DECLUSAGE_NONE;
    }
}

static const char *nine_declusage_names[] =
{
    [NINE_DECLUSAGE_POSITION]        = "POSITION",
    [NINE_DECLUSAGE_BLENDWEIGHT]     = "BLENDWEIGHT",
    [NINE_DECLUSAGE_BLENDINDICES]    = "BLENDINDICES",
    [NINE_DECLUSAGE_NORMAL]          = "NORMAL",
    [NINE_DECLUSAGE_PSIZE]           = "PSIZE",
    [NINE_DECLUSAGE_TEXCOORD]        = "TEXCOORD",
    [NINE_DECLUSAGE_TANGENT]         = "TANGENT",
    [NINE_DECLUSAGE_BINORMAL]        = "BINORMAL",
    [NINE_DECLUSAGE_TESSFACTOR]      = "TESSFACTOR",
    [NINE_DECLUSAGE_POSITIONT]       = "POSITIONT",
    [NINE_DECLUSAGE_COLOR]           = "DIFFUSE",
    [NINE_DECLUSAGE_DEPTH]           = "DEPTH",
    [NINE_DECLUSAGE_FOG]             = "FOG",
    [NINE_DECLUSAGE_NONE]            = "(NONE)",
};
static inline const char *
nine_declusage_name(unsigned ndcl)
{
    return nine_declusage_names[ndcl % NINE_DECLUSAGE_COUNT];
}

HRESULT
NineVertexDeclaration9_ctor( struct NineVertexDeclaration9 *This,
                             struct NineUnknownParams *pParams,
                             const D3DVERTEXELEMENT9 *pElements )
{
    const D3DCAPS9 *caps;
    unsigned i, nelems;
    DBG("This=%p pParams=%p pElements=%p\n", This, pParams, pElements);

    /* wine */
    for (nelems = 0;
         pElements[nelems].Stream != 0xFF;
         ++nelems) {
        user_assert(pElements[nelems].Type != D3DDECLTYPE_UNUSED, E_FAIL);
        user_assert(!(pElements[nelems].Offset & 3), E_FAIL);
    }

    caps = NineDevice9_GetCaps(pParams->device);
    user_assert(nelems <= caps->MaxStreams, D3DERR_INVALIDCALL);

    HRESULT hr = NineUnknown_ctor(&This->base, pParams);
    if (FAILED(hr)) { return hr; }

    This->nelems = nelems;
    This->decls = CALLOC(This->nelems+1, sizeof(D3DVERTEXELEMENT9));
    This->elems = CALLOC(This->nelems, sizeof(struct pipe_vertex_element));
    This->usage_map = CALLOC(This->nelems, sizeof(uint16_t));
    if (!This->decls || !This->elems || !This->usage_map) { return E_OUTOFMEMORY; }
    memcpy(This->decls, pElements, sizeof(D3DVERTEXELEMENT9)*(This->nelems+1));

    for (i = 0; i < This->nelems; ++i) {
        uint16_t usage = nine_d3d9_to_nine_declusage(This->decls[i].Usage,
                                                     This->decls[i].UsageIndex);
        This->usage_map[i] = usage;

        if (This->decls[i].Usage == D3DDECLUSAGE_POSITIONT)
            This->position_t = true;

        This->elems[i].src_offset = This->decls[i].Offset;
        This->elems[i].instance_divisor = 0;
        This->elems[i].vertex_buffer_index = This->decls[i].Stream;
        This->elems[i].src_format = decltype_format(This->decls[i].Type);
        This->elems[i].dual_slot = false;
        /* XXX Remember Method (tesselation), Usage, UsageIndex */

        DBG("VERTEXELEMENT[%u]: Stream=%u Offset=%u Type=%s DeclUsage=%s%d\n", i,
            This->decls[i].Stream,
            This->decls[i].Offset,
            util_format_name(This->elems[i].src_format),
            nine_declusage_name(usage),
            usage / NINE_DECLUSAGE_COUNT);
    }

    return D3D_OK;
}

void
NineVertexDeclaration9_dtor( struct NineVertexDeclaration9 *This )
{
    DBG("This=%p\n", This);

    FREE(This->decls);
    FREE(This->elems);
    FREE(This->usage_map);

    NineUnknown_dtor(&This->base);
}

HRESULT NINE_WINAPI
NineVertexDeclaration9_GetDeclaration( struct NineVertexDeclaration9 *This,
                                       D3DVERTEXELEMENT9 *pElement,
                                       UINT *pNumElements )
{
    if (!pElement) {
        user_assert(pNumElements, D3DERR_INVALIDCALL);
        *pNumElements = This->nelems+1;
        return D3D_OK;
    }
    if (pNumElements) { *pNumElements = This->nelems+1; }
    memcpy(pElement, This->decls, sizeof(D3DVERTEXELEMENT9)*(This->nelems+1));
    return D3D_OK;
}

IDirect3DVertexDeclaration9Vtbl NineVertexDeclaration9_vtable = {
    (void *)NineUnknown_QueryInterface,
    (void *)NineUnknown_AddRef,
    (void *)NineUnknown_Release,
    (void *)NineUnknown_GetDevice, /* actually part of VertexDecl9 iface */
    (void *)NineVertexDeclaration9_GetDeclaration
};

static const GUID *NineVertexDeclaration9_IIDs[] = {
    &IID_IDirect3DVertexDeclaration9,
    &IID_IUnknown,
    NULL
};

HRESULT
NineVertexDeclaration9_new( struct NineDevice9 *pDevice,
                            const D3DVERTEXELEMENT9 *pElements,
                            struct NineVertexDeclaration9 **ppOut )
{
    NINE_DEVICE_CHILD_NEW(VertexDeclaration9, ppOut, /* args */ pDevice, pElements);
}

HRESULT
NineVertexDeclaration9_new_from_fvf( struct NineDevice9 *pDevice,
                                     DWORD FVF,
                                     struct NineVertexDeclaration9 **ppOut )
{
    D3DVERTEXELEMENT9 elems[16], decl_end = D3DDECL_END();
    unsigned texcount, i, betas, nelems = 0;
    BYTE beta_index = 0xFF;

    switch (FVF & D3DFVF_POSITION_MASK) {
        case D3DFVF_XYZ: /* simple XYZ */
        case D3DFVF_XYZB1:
        case D3DFVF_XYZB2:
        case D3DFVF_XYZB3:
        case D3DFVF_XYZB4:
        case D3DFVF_XYZB5: /* XYZ with beta values */
            elems[nelems].Type = D3DDECLTYPE_FLOAT3;
            elems[nelems].Usage = D3DDECLUSAGE_POSITION;
            elems[nelems].UsageIndex = 0;
            ++nelems;
            /* simple XYZ has no beta values. break. */
            if ((FVF & D3DFVF_POSITION_MASK) == D3DFVF_XYZ) { break; }

            betas = (((FVF & D3DFVF_XYZB5)-D3DFVF_XYZB1)>>1)+1;
            if (FVF & D3DFVF_LASTBETA_D3DCOLOR) {
                beta_index = D3DDECLTYPE_D3DCOLOR;
            } else if (FVF & D3DFVF_LASTBETA_UBYTE4) {
                beta_index = D3DDECLTYPE_UBYTE4;
            } else if ((FVF & D3DFVF_XYZB5) == D3DFVF_XYZB5) {
                beta_index = D3DDECLTYPE_FLOAT1;
            }
            if (beta_index != 0xFF) { --betas; }

            if (betas > 0) {
                switch (betas) {
                    case 1: elems[nelems].Type = D3DDECLTYPE_FLOAT1; break;
                    case 2: elems[nelems].Type = D3DDECLTYPE_FLOAT2; break;
                    case 3: elems[nelems].Type = D3DDECLTYPE_FLOAT3; break;
                    case 4: elems[nelems].Type = D3DDECLTYPE_FLOAT4; break;
                    default:
                        assert(!"Implementation error!");
                }
                elems[nelems].Usage = D3DDECLUSAGE_BLENDWEIGHT;
                elems[nelems].UsageIndex = 0;
                ++nelems;
            }

            if (beta_index != 0xFF) {
                elems[nelems].Type = beta_index;
                elems[nelems].Usage = D3DDECLUSAGE_BLENDINDICES;
                elems[nelems].UsageIndex = 0;
                ++nelems;
            }
            break;

        case D3DFVF_XYZW: /* simple XYZW */
        case D3DFVF_XYZRHW: /* pretransformed XYZW */
            elems[nelems].Type = D3DDECLTYPE_FLOAT4;
            elems[nelems].Usage =
                ((FVF & D3DFVF_POSITION_MASK) == D3DFVF_XYZW) ?
                D3DDECLUSAGE_POSITION : D3DDECLUSAGE_POSITIONT;
            elems[nelems].UsageIndex = 0;
            ++nelems;
            break;

        default:
            (void)user_error(!"Position doesn't match any known combination");
    }

    /* normals, psize and colors */
    if (FVF & D3DFVF_NORMAL) {
        elems[nelems].Type = D3DDECLTYPE_FLOAT3;
        elems[nelems].Usage = D3DDECLUSAGE_NORMAL;
        elems[nelems].UsageIndex = 0;
        ++nelems;
    }
    if (FVF & D3DFVF_PSIZE) {
        elems[nelems].Type = D3DDECLTYPE_FLOAT1;
        elems[nelems].Usage = D3DDECLUSAGE_PSIZE;
        elems[nelems].UsageIndex = 0;
        ++nelems;
    }
    if (FVF & D3DFVF_DIFFUSE) {
        elems[nelems].Type = D3DDECLTYPE_D3DCOLOR;
        elems[nelems].Usage = D3DDECLUSAGE_COLOR;
        elems[nelems].UsageIndex = 0;
        ++nelems;
    }
    if (FVF & D3DFVF_SPECULAR) {
        elems[nelems].Type = D3DDECLTYPE_D3DCOLOR;
        elems[nelems].Usage = D3DDECLUSAGE_COLOR;
        elems[nelems].UsageIndex = 1;
        ++nelems;
    }

    /* textures */
    texcount = (FVF & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT;
    if (user_error(texcount <= 8)) { texcount = 8; }

    for (i = 0; i < texcount; ++i) {
        switch ((FVF >> (16+i*2)) & 0x3) {
            case D3DFVF_TEXTUREFORMAT1:
                elems[nelems].Type = D3DDECLTYPE_FLOAT1;
                break;

            case D3DFVF_TEXTUREFORMAT2:
                elems[nelems].Type = D3DDECLTYPE_FLOAT2;
                break;

            case D3DFVF_TEXTUREFORMAT3:
                elems[nelems].Type = D3DDECLTYPE_FLOAT3;
                break;

            case D3DFVF_TEXTUREFORMAT4:
                elems[nelems].Type = D3DDECLTYPE_FLOAT4;
                break;

            default:
                assert(!"Implementation error!");
        }
        elems[nelems].Usage = D3DDECLUSAGE_TEXCOORD;
        elems[nelems].UsageIndex = i;
        ++nelems;
    }

    /* fill out remaining data */
    for (i = 0; i < nelems; ++i) {
        elems[i].Stream = 0;
        elems[i].Offset = (i == 0) ? 0 : (elems[i-1].Offset +
                                          decltype_size(elems[i-1].Type));
        elems[i].Method = D3DDECLMETHOD_DEFAULT;
    }
    elems[nelems++] = decl_end;

    NINE_DEVICE_CHILD_NEW(VertexDeclaration9, ppOut, /* args */ pDevice, elems);
}

void
NineVertexDeclaration9_FillStreamOutputInfo(
    struct NineVertexDeclaration9 *This,
    struct nine_vs_output_info *ShaderOutputsInfo,
    unsigned numOutputs,
    struct pipe_stream_output_info *so )
{
    unsigned so_outputs = 0;
    int i, j;

    memset(so, 0, sizeof(struct pipe_stream_output_info));

    for (i = 0; i < numOutputs; i++) {
        BYTE output_semantic = ShaderOutputsInfo[i].output_semantic;
        unsigned output_semantic_index = ShaderOutputsInfo[i].output_semantic_index;

        for (j = 0; j < This->nelems; j++) {
            if ((This->decls[j].Usage == output_semantic ||
                 (output_semantic == D3DDECLUSAGE_POSITION &&
                  This->decls[j].Usage == D3DDECLUSAGE_POSITIONT)) &&
                This->decls[j].UsageIndex == output_semantic_index) {
                DBG("Matching %s %d: o%d -> %d\n",
                    nine_declusage_name(nine_d3d9_to_nine_declusage(This->decls[j].Usage, 0)),
                    This->decls[j].UsageIndex, i, j);
                so->output[so_outputs].register_index = ShaderOutputsInfo[i].output_index;
                so->output[so_outputs].start_component = 0;
                if (ShaderOutputsInfo[i].mask & 8)
                    so->output[so_outputs].num_components = 4;
                else if (ShaderOutputsInfo[i].mask & 4)
                    so->output[so_outputs].num_components = 3;
                else if (ShaderOutputsInfo[i].mask & 2)
                    so->output[so_outputs].num_components = 2;
                else
                    so->output[so_outputs].num_components = 1;
                so->output[so_outputs].output_buffer = 0;
                so->output[so_outputs].dst_offset = so_outputs * sizeof(float[4])/4;
                so->output[so_outputs].stream = 0;
                so_outputs++;
                break;
            }
        }
    }

    so->num_outputs = so_outputs;
    so->stride[0] = so_outputs * sizeof(float[4])/4;
}

/* ProcessVertices runs stream output into a temporary buffer to capture
 * all outputs.
 * Now we have to convert them to the format and order set by the vertex
 * declaration, for which we use u_translate.
 * This is necessary if the vertex declaration contains elements using a
 * non float32 format, because stream output only supports f32/u32/s32.
 */
HRESULT
NineVertexDeclaration9_ConvertStreamOutput(
    struct NineVertexDeclaration9 *This,
    struct NineVertexBuffer9 *pDstBuf,
    UINT DestIndex,
    UINT VertexCount,
    void *pSrcBuf,
    const struct pipe_stream_output_info *so )
{
    struct translate *translate;
    struct translate_key transkey;
    HRESULT hr;
    unsigned i;
    void *dst_map;

    DBG("This=%p pDstBuf=%p DestIndex=%u VertexCount=%u pSrcBuf=%p so=%p\n",
        This, pDstBuf, DestIndex, VertexCount, pSrcBuf, so);

    transkey.output_stride = 0;
    for (i = 0; i < This->nelems; ++i) {
        enum pipe_format format;

        switch (so->output[i].num_components) {
        case 1: format = PIPE_FORMAT_R32_FLOAT; break;
        case 2: format = PIPE_FORMAT_R32G32_FLOAT; break;
        case 3: format = PIPE_FORMAT_R32G32B32_FLOAT; break;
        default:
            assert(so->output[i].num_components == 4);
            format = PIPE_FORMAT_R32G32B32A32_FLOAT;
            break;
        }
        transkey.element[i].type = TRANSLATE_ELEMENT_NORMAL;
        transkey.element[i].input_format = format;
        transkey.element[i].input_buffer = 0;
        transkey.element[i].input_offset = so->output[i].dst_offset * 4;
        transkey.element[i].instance_divisor = 0;

        transkey.element[i].output_format = This->elems[i].src_format;
        transkey.element[i].output_offset = This->elems[i].src_offset;
        transkey.output_stride +=
            util_format_get_blocksize(This->elems[i].src_format);

        assert(!(transkey.output_stride & 3));
    }
    transkey.nr_elements = This->nelems;

    translate = translate_create(&transkey);
    if (!translate)
        return E_OUTOFMEMORY;

    hr = NineVertexBuffer9_Lock(pDstBuf,
                                transkey.output_stride * DestIndex,
                                transkey.output_stride * VertexCount,
                                &dst_map, D3DLOCK_DISCARD);
    if (FAILED(hr))
        goto out;

    translate->set_buffer(translate, 0, pSrcBuf, so->stride[0] * 4, ~0);

    translate->run(translate, 0, VertexCount, 0, 0, dst_map);

    NineVertexBuffer9_Unlock(pDstBuf);
out:
    translate->release(translate); /* TODO: cache these */
    return hr;
}
