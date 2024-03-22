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
#include "nine_state.h"
#include "query9.h"
#include "nine_helpers.h"
#include "pipe/p_screen.h"
#include "pipe/p_context.h"
#include "util/u_math.h"
#include "nine_dump.h"

#define DBG_CHANNEL DBG_QUERY

static inline unsigned
d3dquerytype_to_pipe_query(struct pipe_screen *screen, D3DQUERYTYPE type)
{
    switch (type) {
    case D3DQUERYTYPE_EVENT:
        return PIPE_QUERY_GPU_FINISHED;
    case D3DQUERYTYPE_OCCLUSION:
        return screen->get_param(screen, PIPE_CAP_OCCLUSION_QUERY) ?
               PIPE_QUERY_OCCLUSION_COUNTER : PIPE_QUERY_TYPES;
    case D3DQUERYTYPE_TIMESTAMP:
        return screen->get_param(screen, PIPE_CAP_QUERY_TIMESTAMP) ?
               PIPE_QUERY_TIMESTAMP : PIPE_QUERY_TYPES;
    case D3DQUERYTYPE_TIMESTAMPDISJOINT:
    case D3DQUERYTYPE_TIMESTAMPFREQ:
        return screen->get_param(screen, PIPE_CAP_QUERY_TIMESTAMP) ?
               PIPE_QUERY_TIMESTAMP_DISJOINT : PIPE_QUERY_TYPES;
    case D3DQUERYTYPE_VERTEXSTATS:
        return screen->get_param(screen,
                                 PIPE_CAP_QUERY_PIPELINE_STATISTICS) ?
               PIPE_QUERY_PIPELINE_STATISTICS : PIPE_QUERY_TYPES;
    default:
        return PIPE_QUERY_TYPES; /* Query not supported */
    }
}

#define GET_DATA_SIZE_CASE2(a, b) case D3DQUERYTYPE_##a: return sizeof(D3DDEVINFO_##b)
#define GET_DATA_SIZE_CASET(a, b) case D3DQUERYTYPE_##a: return sizeof(b)
static inline DWORD
nine_query_result_size(D3DQUERYTYPE type)
{
    switch (type) {
    GET_DATA_SIZE_CASE2(VERTEXSTATS, D3DVERTEXSTATS);
    GET_DATA_SIZE_CASET(EVENT, BOOL);
    GET_DATA_SIZE_CASET(OCCLUSION, DWORD);
    GET_DATA_SIZE_CASET(TIMESTAMP, UINT64);
    GET_DATA_SIZE_CASET(TIMESTAMPDISJOINT, BOOL);
    GET_DATA_SIZE_CASET(TIMESTAMPFREQ, UINT64);
    default:
        assert(0);
        return 0;
    }
}

HRESULT
nine_is_query_supported(struct pipe_screen *screen, D3DQUERYTYPE type)
{
    const unsigned ptype = d3dquerytype_to_pipe_query(screen, type);

    user_assert(ptype != ~0, D3DERR_INVALIDCALL);

    if (ptype == PIPE_QUERY_TYPES) {
        DBG("Query type %u (%s) not supported.\n",
            type, nine_D3DQUERYTYPE_to_str(type));
        return D3DERR_NOTAVAILABLE;
    }
    return D3D_OK;
}

HRESULT
NineQuery9_ctor( struct NineQuery9 *This,
                 struct NineUnknownParams *pParams,
                 D3DQUERYTYPE Type )
{
    struct NineDevice9 *device = pParams->device;
    const unsigned ptype = d3dquerytype_to_pipe_query(device->screen, Type);
    HRESULT hr;

    DBG("This=%p pParams=%p Type=%d\n", This, pParams, Type);

    hr = NineUnknown_ctor(&This->base, pParams);
    if (FAILED(hr))
        return hr;

    This->state = NINE_QUERY_STATE_FRESH;
    This->type = Type;

    user_assert(ptype != ~0, D3DERR_INVALIDCALL);

    if (ptype < PIPE_QUERY_TYPES) {
        This->pq = nine_context_create_query(device, ptype);
        if (!This->pq)
            return E_OUTOFMEMORY;
    } else {
        assert(0); /* we have checked this case before */
    }

    This->instant =
        Type == D3DQUERYTYPE_EVENT ||
        Type == D3DQUERYTYPE_RESOURCEMANAGER ||
        Type == D3DQUERYTYPE_TIMESTAMP ||
        Type == D3DQUERYTYPE_TIMESTAMPFREQ ||
        Type == D3DQUERYTYPE_VCACHE ||
        Type == D3DQUERYTYPE_VERTEXSTATS;

    This->result_size = nine_query_result_size(Type);

    return D3D_OK;
}

void
NineQuery9_dtor( struct NineQuery9 *This )
{
    struct NineDevice9 *device = This->base.device;

    DBG("This=%p\n", This);

    if (This->pq) {
        if (This->state == NINE_QUERY_STATE_RUNNING)
            nine_context_end_query(device, &This->counter, This->pq);
        nine_context_destroy_query(device, This->pq);
    }

    NineUnknown_dtor(&This->base);
}

D3DQUERYTYPE NINE_WINAPI
NineQuery9_GetType( struct NineQuery9 *This )
{
    return This->type;
}

DWORD NINE_WINAPI
NineQuery9_GetDataSize( struct NineQuery9 *This )
{
    return This->result_size;
}

HRESULT NINE_WINAPI
NineQuery9_Issue( struct NineQuery9 *This,
                  DWORD dwIssueFlags )
{
    struct NineDevice9 *device = This->base.device;

    DBG("This=%p dwIssueFlags=%d\n", This, dwIssueFlags);

    user_assert((dwIssueFlags == D3DISSUE_BEGIN) ||
                (dwIssueFlags == 0) ||
                (dwIssueFlags == D3DISSUE_END), D3DERR_INVALIDCALL);

    /* Wine tests: always return D3D_OK on D3DISSUE_BEGIN
     * even when the call is supposed to be forbidden */
    if (dwIssueFlags == D3DISSUE_BEGIN && This->instant)
        return D3D_OK;

    if (dwIssueFlags == D3DISSUE_BEGIN) {
        if (This->state == NINE_QUERY_STATE_RUNNING)
            nine_context_end_query(device, &This->counter, This->pq);
        nine_context_begin_query(device, &This->counter, This->pq);
        This->state = NINE_QUERY_STATE_RUNNING;
    } else {
        if (This->state != NINE_QUERY_STATE_RUNNING &&
            This->type != D3DQUERYTYPE_EVENT &&
            This->type != D3DQUERYTYPE_TIMESTAMP)
            nine_context_begin_query(device, &This->counter, This->pq);
        nine_context_end_query(device, &This->counter, This->pq);
        This->state = NINE_QUERY_STATE_ENDED;
    }
    return D3D_OK;
}

union nine_query_result
{
    D3DDEVINFO_D3DVERTEXSTATS vertexstats;
    DWORD dw;
    BOOL b;
    UINT64 u64;
};

HRESULT NINE_WINAPI
NineQuery9_GetData( struct NineQuery9 *This,
                    void *pData,
                    DWORD dwSize,
                    DWORD dwGetDataFlags )
{
    struct NineDevice9 *device = This->base.device;
    bool ok, wait_query_result = false;
    union pipe_query_result presult;
    union nine_query_result nresult;

    DBG("This=%p pData=%p dwSize=%d dwGetDataFlags=%d\n",
        This, pData, dwSize, dwGetDataFlags);

    /* according to spec we should return D3DERR_INVALIDCALL here, but
     * wine returns S_FALSE because it is apparently the behaviour
     * on windows */
    user_assert(This->state != NINE_QUERY_STATE_RUNNING, S_FALSE);
    user_assert(dwSize == 0 || pData, D3DERR_INVALIDCALL);
    user_assert(dwGetDataFlags == 0 ||
                dwGetDataFlags == D3DGETDATA_FLUSH, D3DERR_INVALIDCALL);

    if (This->state == NINE_QUERY_STATE_FRESH) {
        /* App forgot calling Issue. call it for it.
         * However Wine states that return value should
         * be S_OK, so wait for the result to return S_OK. */
        NineQuery9_Issue(This, D3DISSUE_END);
        wait_query_result = true;
    }

    /* The documention mentions no special case for D3DQUERYTYPE_TIMESTAMP.
     * However Windows tests show that the query always succeeds when
     * D3DGETDATA_FLUSH is specified. */
    if (This->type == D3DQUERYTYPE_TIMESTAMP &&
        (dwGetDataFlags & D3DGETDATA_FLUSH))
        wait_query_result = true;


    /* Note: We ignore dwGetDataFlags, because get_query_result will
     * flush automatically if needed */

    ok = nine_context_get_query_result(device, This->pq, &This->counter,
                                       !!(dwGetDataFlags & D3DGETDATA_FLUSH),
                                       wait_query_result, &presult);

    if (!ok) return S_FALSE;

    if (!dwSize)
        return S_OK;

    switch (This->type) {
    case D3DQUERYTYPE_EVENT:
        nresult.b = presult.b;
        break;
    case D3DQUERYTYPE_OCCLUSION:
        nresult.dw = presult.u64;
        break;
    case D3DQUERYTYPE_TIMESTAMP:
        nresult.u64 = presult.u64;
        break;
    case D3DQUERYTYPE_TIMESTAMPDISJOINT:
        nresult.b = presult.timestamp_disjoint.disjoint;
        break;
    case D3DQUERYTYPE_TIMESTAMPFREQ:
        /* Applications use it to convert the TIMESTAMP value to time.
           AMD drivers on win seem to return the actual hardware clock
           resolution and corresponding values in TIMESTAMP.
           However, this behaviour is not easy to replicate here.
           So instead we do what wine and opengl do, and use
           nanoseconds TIMESTAMPs.
           (Which is also the unit used by PIPE_QUERY_TIMESTAMP.)
        */
        nresult.u64 = 1000000000;
        break;
    case D3DQUERYTYPE_VERTEXSTATS:
        nresult.vertexstats.NumRenderedTriangles =
            presult.pipeline_statistics.c_invocations;
        nresult.vertexstats.NumExtraClippingTriangles =
            presult.pipeline_statistics.c_primitives;
        break;
    default:
        assert(0);
        break;
    }
    memcpy(pData, &nresult, MIN2(sizeof(nresult), dwSize));

    return S_OK;
}

IDirect3DQuery9Vtbl NineQuery9_vtable = {
    (void *)NineUnknown_QueryInterface,
    (void *)NineUnknown_AddRef,
    (void *)NineUnknown_Release,
    (void *)NineUnknown_GetDevice, /* actually part of Query9 iface */
    (void *)NineQuery9_GetType,
    (void *)NineQuery9_GetDataSize,
    (void *)NineQuery9_Issue,
    (void *)NineQuery9_GetData
};

static const GUID *NineQuery9_IIDs[] = {
    &IID_IDirect3DQuery9,
    &IID_IUnknown,
    NULL
};

HRESULT
NineQuery9_new( struct NineDevice9 *pDevice,
                struct NineQuery9 **ppOut,
                D3DQUERYTYPE Type )
{
    NINE_DEVICE_CHILD_NEW(Query9, ppOut, pDevice, Type);
}
