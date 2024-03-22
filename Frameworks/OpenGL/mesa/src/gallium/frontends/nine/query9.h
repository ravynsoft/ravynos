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

#ifndef _NINE_QUERY9_H_
#define _NINE_QUERY9_H_

#include "iunknown.h"

enum nine_query_state
{
    NINE_QUERY_STATE_FRESH = 0,
    NINE_QUERY_STATE_RUNNING,
    NINE_QUERY_STATE_ENDED,
};

struct NineQuery9
{
    struct NineUnknown base;
    struct pipe_query *pq;
    DWORD result_size;
    D3DQUERYTYPE type;
    enum nine_query_state state;
    bool instant; /* true if D3DISSUE_BEGIN is not needed / invalid */
    unsigned counter; /* Number of pending Begin/End (0 if internal multithreading off) */
};
static inline struct NineQuery9 *
NineQuery9( void *data )
{
    return (struct NineQuery9 *)data;
}

HRESULT
nine_is_query_supported(struct pipe_screen *screen, D3DQUERYTYPE);

HRESULT
NineQuery9_new( struct NineDevice9 *Device,
                struct NineQuery9 **ppOut,
                D3DQUERYTYPE);

HRESULT
NineQuery9_ctor( struct NineQuery9 *,
                 struct NineUnknownParams *pParams,
                 D3DQUERYTYPE Type );

void
NineQuery9_dtor( struct NineQuery9 * );

D3DQUERYTYPE NINE_WINAPI
NineQuery9_GetType( struct NineQuery9 *This );

DWORD NINE_WINAPI
NineQuery9_GetDataSize( struct NineQuery9 *This );

HRESULT NINE_WINAPI
NineQuery9_Issue( struct NineQuery9 *This,
                  DWORD dwIssueFlags );

HRESULT NINE_WINAPI
NineQuery9_GetData( struct NineQuery9 *This,
                    void *pData,
                    DWORD dwSize,
                    DWORD dwGetDataFlags );

#endif /* _NINE_QUERY9_H_ */
