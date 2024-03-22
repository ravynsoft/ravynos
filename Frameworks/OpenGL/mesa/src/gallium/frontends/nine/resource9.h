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

#ifndef _NINE_RESOURCE9_H_
#define _NINE_RESOURCE9_H_

#include "iunknown.h"
#include "pipe/p_state.h"

struct pipe_screen;
struct hash_table;
struct NineDevice9;

struct NineResource9
{
    struct NineUnknown base;

    struct pipe_resource *resource; /* device resource */

    D3DRESOURCETYPE type;
    D3DPOOL pool;
    DWORD priority;
    DWORD usage;

    struct pipe_resource info; /* resource configuration */

    long long size;
};
static inline struct NineResource9 *
NineResource9( void *data )
{
    return (struct NineResource9 *)data;
}

HRESULT
NineResource9_ctor( struct NineResource9 *This,
                    struct NineUnknownParams *pParams,
                    struct pipe_resource *initResource,
                    BOOL Allocate,
                    D3DRESOURCETYPE Type,
                    D3DPOOL Pool,
                    DWORD Usage);

void
NineResource9_dtor( struct NineResource9 *This );

/*** Nine private methods ***/

struct pipe_resource *
NineResource9_GetResource( struct NineResource9 *This );

D3DPOOL
NineResource9_GetPool( struct NineResource9 *This );

/*** Direct3D public methods ***/

DWORD NINE_WINAPI
NineResource9_SetPriority( struct NineResource9 *This,
                           DWORD PriorityNew );

DWORD NINE_WINAPI
NineResource9_GetPriority( struct NineResource9 *This );

void NINE_WINAPI
NineResource9_PreLoad( struct NineResource9 *This );

D3DRESOURCETYPE NINE_WINAPI
NineResource9_GetType( struct NineResource9 *This );

#endif /* _NINE_RESOURCE9_H_ */
