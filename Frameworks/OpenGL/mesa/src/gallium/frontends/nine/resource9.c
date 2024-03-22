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

#include "resource9.h"
#include "device9.h"
#include "nine_helpers.h"
#include "nine_defines.h"

#include "util/u_inlines.h"
#include "util/u_resource.h"

#include "pipe/p_screen.h"

#define DBG_CHANNEL DBG_RESOURCE

HRESULT
NineResource9_ctor( struct NineResource9 *This,
                    struct NineUnknownParams *pParams,
                    struct pipe_resource *initResource,
                    BOOL Allocate,
                    D3DRESOURCETYPE Type,
                    D3DPOOL Pool,
                    DWORD Usage)
{
    struct pipe_screen *screen;
    HRESULT hr;

    DBG("This=%p pParams=%p initResource=%p Allocate=%d "
        "Type=%d Pool=%d Usage=%d\n",
        This, pParams, initResource, (int) Allocate,
        Type, Pool, Usage);

    hr = NineUnknown_ctor(&This->base, pParams);
    if (FAILED(hr))
        return hr;

    This->info.screen = screen = This->base.device->screen;
    if (initResource)
        pipe_resource_reference(&This->resource, initResource);

    if (Allocate) {
        assert(!initResource);

        /* On Windows it is possible allocation fails when
         * IDirect3DDevice9::GetAvailableTextureMem() still reports
         * enough free space.
         *
         * Some games allocate surfaces
         * in a loop until they receive D3DERR_OUTOFVIDEOMEMORY to measure
         * the available texture memory size.
         *
         * We are not using the drivers VRAM statistics because:
         *  * This would add overhead to each resource allocation.
         *  * Freeing memory is lazy and takes some time, but applications
         *    expects the memory counter to change immediately after allocating
         *    or freeing memory.
         *
         * Vertexbuffers and indexbuffers are not accounted !
         */
        if (This->info.target != PIPE_BUFFER) {
            This->size = util_resource_size(&This->info);

            p_atomic_add(&This->base.device->available_texture_mem, -This->size);
            /* Before failing allocation, evict MANAGED memory */
            if (This->base.device &&
                p_atomic_read(&This->base.device->available_texture_mem) <=
                    This->base.device->available_texture_limit)
                NineDevice9_EvictManagedResourcesInternal(This->base.device);
            if (p_atomic_read(&This->base.device->available_texture_mem) <=
                    This->base.device->available_texture_limit) {
                DBG("Memory allocation failure: software limit\n");
                return D3DERR_OUTOFVIDEOMEMORY;
            }
        }

        DBG("(%p) Creating pipe_resource.\n", This);
        This->resource = nine_resource_create_with_retry(This->base.device, screen, &This->info);
        if (!This->resource)
            return D3DERR_OUTOFVIDEOMEMORY;
    }

    DBG("Current texture memory count: (%d/%d)KB\n",
        (int)(This->base.device->available_texture_mem >> 10),
        (int)(This->base.device->available_texture_limit >> 10));

    This->type = Type;
    This->pool = Pool;
    This->usage = Usage;
    This->priority = 0;

    return D3D_OK;
}

void
NineResource9_dtor( struct NineResource9 *This )
{
    DBG("This=%p\n", This);

    /* NOTE: We do have to use refcounting, the driver might
     * still hold a reference. */
    pipe_resource_reference(&This->resource, NULL);

    /* NOTE: size is 0, unless something has actually been allocated */
    if (This->base.device)
        p_atomic_add(&This->base.device->available_texture_mem, This->size);

    NineUnknown_dtor(&This->base);
}

struct pipe_resource *
NineResource9_GetResource( struct NineResource9 *This )
{
    return This->resource;
}

D3DPOOL
NineResource9_GetPool( struct NineResource9 *This )
{
    return This->pool;
}

DWORD NINE_WINAPI
NineResource9_SetPriority( struct NineResource9 *This,
                           DWORD PriorityNew )
{
    DWORD prev;
    DBG("This=%p, PriorityNew=%d\n", This, PriorityNew);

    if (This->pool != D3DPOOL_MANAGED || This->type == D3DRTYPE_SURFACE)
        return 0;

    prev = This->priority;
    This->priority = PriorityNew;
    return prev;
}

DWORD NINE_WINAPI
NineResource9_GetPriority( struct NineResource9 *This )
{
    if (This->pool != D3DPOOL_MANAGED || This->type == D3DRTYPE_SURFACE)
        return 0;

    return This->priority;
}

/* NOTE: Don't forget to adjust locked vtable if you change this ! */
void NINE_WINAPI
NineResource9_PreLoad( struct NineResource9 *This )
{
    if (This->pool != D3DPOOL_MANAGED)
        return;
    /* We don't treat managed vertex or index buffers different from
     * default ones (are managed vertex buffers even allowed ?), and
     * the PreLoad for textures is overridden by superclass.
     */
}

D3DRESOURCETYPE NINE_WINAPI
NineResource9_GetType( struct NineResource9 *This )
{
    return This->type;
}
