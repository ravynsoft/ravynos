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

#ifndef _NINE_BASETEXTURE9_H_
#define _NINE_BASETEXTURE9_H_

#include "device9.h"
#include "resource9.h"
#include "util/u_inlines.h"
#include "util/list.h"

struct NineBaseTexture9
{
    struct NineResource9 base;
    struct list_head list; /* for update_textures */
    struct list_head list2; /* for managed_textures */

    /* g3d */
    struct pipe_sampler_view *view[2]; /* linear and sRGB */

    D3DFORMAT format;

    int16_t bind_count; /* to Device9->state.texture */

    bool shadow;
    bool fetch4_compatible;
    uint8_t pstype; /* 0: 2D, 1: 1D, 2: CUBE, 3: 3D */

    bool dirty_mip;
    D3DTEXTUREFILTERTYPE mipfilter;

    unsigned level_count;

    /* Specific to managed textures */
    struct {
        bool dirty;
        DWORD lod;
        DWORD lod_resident;
    } managed;
};
static inline struct NineBaseTexture9 *
NineBaseTexture9( void *data )
{
    return (struct NineBaseTexture9 *)data;
}

HRESULT
NineBaseTexture9_ctor( struct NineBaseTexture9 *This,
                       struct NineUnknownParams *pParams,
                       struct pipe_resource *initResource,
                       D3DRESOURCETYPE Type,
                       D3DFORMAT format,
                       D3DPOOL Pool,
                       DWORD Usage);

void
NineBaseTexture9_dtor( struct NineBaseTexture9 *This );

DWORD NINE_WINAPI
NineBaseTexture9_SetLOD( struct NineBaseTexture9 *This,
                         DWORD LODNew );

DWORD NINE_WINAPI
NineBaseTexture9_GetLOD( struct NineBaseTexture9 *This );

DWORD NINE_WINAPI
NineBaseTexture9_GetLevelCount( struct NineBaseTexture9 *This );

HRESULT NINE_WINAPI
NineBaseTexture9_SetAutoGenFilterType( struct NineBaseTexture9 *This,
                                       D3DTEXTUREFILTERTYPE FilterType );

D3DTEXTUREFILTERTYPE NINE_WINAPI
NineBaseTexture9_GetAutoGenFilterType( struct NineBaseTexture9 *This );

void NINE_WINAPI
NineBaseTexture9_GenerateMipSubLevels( struct NineBaseTexture9 *This );

void NINE_WINAPI
NineBaseTexture9_PreLoad( struct NineBaseTexture9 *This );

void
NineBaseTexture9_UnLoad( struct NineBaseTexture9 *This );

/* For D3DPOOL_MANAGED only (after SetLOD change): */
HRESULT
NineBaseTexture9_CreatePipeResource( struct NineBaseTexture9 *This,
                                     BOOL CopyData );

/* For D3DPOOL_MANAGED only: */
HRESULT
NineBaseTexture9_UploadSelf( struct NineBaseTexture9 *This );

HRESULT
NineBaseTexture9_UpdateSamplerView( struct NineBaseTexture9 *This,
                                    const int sRGB );

static inline void
NineBaseTexture9_Validate( struct NineBaseTexture9 *This )
{
    DBG_FLAG(DBG_BASETEXTURE, "This=%p dirty=%i dirty_mip=%i lod=%u/%u\n",
             This, This->managed.dirty, This->dirty_mip, This->managed.lod, This->managed.lod_resident);
    if ((This->base.pool == D3DPOOL_MANAGED) &&
        (This->managed.dirty || This->managed.lod != This->managed.lod_resident))
        NineBaseTexture9_UploadSelf(This);
    if (This->dirty_mip)
        NineBaseTexture9_GenerateMipSubLevels(This);
}

static inline struct pipe_sampler_view *
NineBaseTexture9_GetSamplerView( struct NineBaseTexture9 *This, const int sRGB )
{
    if (!This->view[sRGB])
        NineBaseTexture9_UpdateSamplerView(This, sRGB);
    return This->view[sRGB];
}

static void inline
NineBindTextureToDevice( struct NineDevice9 *device,
                         struct NineBaseTexture9 **slot,
                         struct NineBaseTexture9 *tex )
{
    struct NineBaseTexture9 *old = *slot;

    if (tex) {
        if ((tex->managed.dirty | tex->dirty_mip) && list_is_empty(&tex->list))
            list_add(&tex->list, &device->update_textures);

        tex->bind_count++;
    }
    if (old) {
        old->bind_count--;
        if (!old->bind_count)
            list_delinit(&old->list);
    }

    nine_bind(slot, tex);
}

#if defined(DEBUG) || !defined(NDEBUG)
void
NineBaseTexture9_Dump( struct NineBaseTexture9 *This );
#else
static inline void
NineBaseTexture9_Dump( struct NineBaseTexture9 *This ) { }
#endif

#define BASETEX_REGISTER_UPDATE(t) do { \
    if (((t)->managed.dirty | ((t)->dirty_mip)) && (t)->bind_count) \
        if (list_is_empty(&(t)->list)) \
            list_add(&(t)->list, &(t)->base.base.device->update_textures); \
    } while(0)

#endif /* _NINE_BASETEXTURE9_H_ */
