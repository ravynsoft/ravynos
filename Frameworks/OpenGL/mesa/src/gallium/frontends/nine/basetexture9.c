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

#include "basetexture9.h"
#include "device9.h"

/* For UploadSelf: */
#include "texture9.h"
#include "cubetexture9.h"
#include "volumetexture9.h"
#include "nine_pipe.h"

#if defined(DEBUG) || !defined(NDEBUG)
#include "nine_dump.h"
#endif

#include "util/format/u_format.h"

#define DBG_CHANNEL DBG_BASETEXTURE

HRESULT
NineBaseTexture9_ctor( struct NineBaseTexture9 *This,
                       struct NineUnknownParams *pParams,
                       struct pipe_resource *initResource,
                       D3DRESOURCETYPE Type,
                       D3DFORMAT format,
                       D3DPOOL Pool,
                       DWORD Usage)
{
    BOOL alloc = (Pool == D3DPOOL_DEFAULT) && !initResource &&
        (format != D3DFMT_NULL);
    HRESULT hr;

    DBG("This=%p, pParams=%p initResource=%p Type=%d format=%d Pool=%d Usage=%d\n",
        This, pParams, initResource, Type, format, Pool, Usage);

    user_assert(!(Usage & (D3DUSAGE_RENDERTARGET | D3DUSAGE_DEPTHSTENCIL)) ||
                Pool == D3DPOOL_DEFAULT, D3DERR_INVALIDCALL);
    user_assert(!(Usage & D3DUSAGE_DYNAMIC) ||
                !(Pool == D3DPOOL_MANAGED ||
                  Pool == D3DPOOL_SCRATCH), D3DERR_INVALIDCALL);

    hr = NineResource9_ctor(&This->base, pParams, initResource, alloc, Type, Pool, Usage);
    if (FAILED(hr))
        return hr;

    This->format = format;
    This->mipfilter = (Usage & D3DUSAGE_AUTOGENMIPMAP) ?
        D3DTEXF_LINEAR : D3DTEXF_NONE;
    /* In the case of D3DUSAGE_AUTOGENMIPMAP, only the first level is accessible,
     * and thus needs a surface created. */
    This->level_count = (Usage & D3DUSAGE_AUTOGENMIPMAP) ? 1 : (This->base.info.last_level+1);
    This->managed.lod = 0;
    This->managed.lod_resident = -1;
    /* Mark the texture as dirty to trigger first upload when we need the texture,
     * even if it wasn't set by the application */
    if (Pool == D3DPOOL_MANAGED)
        This->managed.dirty = true;
    /* When a depth buffer is sampled, it is for shadow mapping, except for
     * D3DFMT_INTZ, D3DFMT_DF16 and D3DFMT_DF24.
     * In addition D3DFMT_INTZ can be used for both texturing and depth buffering
     * if z write is disabled. This particular feature may not work for us in
     * practice because OGL doesn't have that. However apparently it is known
     * some cards have performance issues with this feature, so real apps
     * shouldn't use it. */
    This->shadow = (This->format != D3DFMT_INTZ && This->format != D3DFMT_DF16 &&
                    This->format != D3DFMT_DF24) &&
                   util_format_has_depth(util_format_description(This->base.info.format));
    This->fetch4_compatible = fetch4_compatible_format(This->format);

    list_inithead(&This->list);
    list_inithead(&This->list2);
    if (Pool == D3DPOOL_MANAGED)
        list_add(&This->list2, &This->base.base.device->managed_textures);

    return D3D_OK;
}

void
NineBaseTexture9_dtor( struct NineBaseTexture9 *This )
{
    DBG("This=%p\n", This);

    pipe_sampler_view_reference(&This->view[0], NULL);
    pipe_sampler_view_reference(&This->view[1], NULL);

    if (list_is_linked(&This->list))
        list_del(&This->list);
    if (list_is_linked(&This->list2))
        list_del(&This->list2);

    NineResource9_dtor(&This->base);
}

DWORD NINE_WINAPI
NineBaseTexture9_SetLOD( struct NineBaseTexture9 *This,
                         DWORD LODNew )
{
    DWORD old = This->managed.lod;

    DBG("This=%p LODNew=%d\n", This, LODNew);

    user_assert(This->base.pool == D3DPOOL_MANAGED, 0);

    This->managed.lod = MIN2(LODNew, This->level_count-1);

    if (This->managed.lod != old && This->bind_count && list_is_empty(&This->list))
       list_add(&This->list, &This->base.base.device->update_textures);

    return old;
}

DWORD NINE_WINAPI
NineBaseTexture9_GetLOD( struct NineBaseTexture9 *This )
{
    DBG("This=%p\n", This);

    return This->managed.lod;
}

DWORD NINE_WINAPI
NineBaseTexture9_GetLevelCount( struct NineBaseTexture9 *This )
{
    DBG("This=%p\n", This);

    return This->level_count;
}

HRESULT NINE_WINAPI
NineBaseTexture9_SetAutoGenFilterType( struct NineBaseTexture9 *This,
                                       D3DTEXTUREFILTERTYPE FilterType )
{
    DBG("This=%p FilterType=%d\n", This, FilterType);

    if (!(This->base.usage & D3DUSAGE_AUTOGENMIPMAP))
        return D3D_OK;
    user_assert(FilterType != D3DTEXF_NONE, D3DERR_INVALIDCALL);

    This->mipfilter = FilterType;
    This->dirty_mip = true;
    NineBaseTexture9_GenerateMipSubLevels(This);

    return D3D_OK;
}

D3DTEXTUREFILTERTYPE NINE_WINAPI
NineBaseTexture9_GetAutoGenFilterType( struct NineBaseTexture9 *This )
{
    DBG("This=%p\n", This);

    return This->mipfilter;
}

HRESULT
NineBaseTexture9_UploadSelf( struct NineBaseTexture9 *This )
{
    HRESULT hr;
    unsigned l, min_level_dirty = This->managed.lod;
    BOOL update_lod;

    DBG("This=%p dirty=%i type=%s\n", This, This->managed.dirty,
        nine_D3DRTYPE_to_str(This->base.type));

    assert(This->base.pool == D3DPOOL_MANAGED);

    update_lod = This->managed.lod_resident != This->managed.lod;
    if (!update_lod && !This->managed.dirty)
        return D3D_OK;

    /* Allocate a new resource with the correct number of levels,
     * Mark states for update, and tell the nine surfaces/volumes
     * their new resource. */
    if (update_lod) {
        struct pipe_resource *res;

        DBG("updating LOD from %u to %u ...\n", This->managed.lod_resident, This->managed.lod);

        pipe_sampler_view_reference(&This->view[0], NULL);
        pipe_sampler_view_reference(&This->view[1], NULL);

        /* Allocate a new resource */
        hr = NineBaseTexture9_CreatePipeResource(This, This->managed.lod_resident != -1);
        if (FAILED(hr))
            return hr;
        res = This->base.resource;

        if (This->managed.lod_resident == -1) {/* no levels were resident */
            This->managed.dirty = false; /* We are going to upload everything. */
            This->managed.lod_resident = This->level_count;
        }

        if (This->base.type == D3DRTYPE_TEXTURE) {
            struct NineTexture9 *tex = NineTexture9(This);

            /* last content (if apply) has been copied to the new resource.
             * Note: We cannot render to surfaces of managed textures.
             * Note2: the level argument passed is to get the level offset
             * right when the texture is uploaded (the texture first level
             * corresponds to This->managed.lod).
             * Note3: We don't care about the value passed for the surfaces
             * before This->managed.lod, negative with this implementation. */
            for (l = 0; l < This->level_count; ++l)
                NineSurface9_SetResource(tex->surfaces[l], res, l - This->managed.lod);
        } else
        if (This->base.type == D3DRTYPE_CUBETEXTURE) {
            struct NineCubeTexture9 *tex = NineCubeTexture9(This);
            unsigned z;

            for (l = 0; l < This->level_count; ++l) {
                for (z = 0; z < 6; ++z)
                    NineSurface9_SetResource(tex->surfaces[l * 6 + z],
                                             res, l - This->managed.lod);
            }
        } else
        if (This->base.type == D3DRTYPE_VOLUMETEXTURE) {
            struct NineVolumeTexture9 *tex = NineVolumeTexture9(This);

            for (l = 0; l < This->level_count; ++l)
                NineVolume9_SetResource(tex->volumes[l], res, l - This->managed.lod);
        } else {
            assert(!"invalid texture type");
        }

        /* We are going to fully upload the new levels,
         * no need to update dirty parts of the texture for these */
        min_level_dirty = MAX2(This->managed.lod, This->managed.lod_resident);
    }

    /* Update dirty parts of the texture */
    if (This->managed.dirty) {
        if (This->base.type == D3DRTYPE_TEXTURE) {
            struct NineTexture9 *tex = NineTexture9(This);
            struct pipe_box box;
            box.z = 0;
            box.depth = 1;

            DBG("TEXTURE: dirty rect=(%u,%u) (%ux%u)\n",
                tex->dirty_rect.x, tex->dirty_rect.y,
                tex->dirty_rect.width, tex->dirty_rect.height);

            /* Note: for l < min_level_dirty, the resource is
             * either non-existing (and thus will be entirely re-uploaded
             * if the lod changes) or going to have a full upload */
            if (tex->dirty_rect.width) {
                for (l = min_level_dirty; l < This->level_count; ++l) {
                    u_box_minify_2d(&box, &tex->dirty_rect, l);
                    NineSurface9_UploadSelf(tex->surfaces[l], &box);
                }
                memset(&tex->dirty_rect, 0, sizeof(tex->dirty_rect));
                tex->dirty_rect.depth = 1;
            }
        } else
        if (This->base.type == D3DRTYPE_CUBETEXTURE) {
            struct NineCubeTexture9 *tex = NineCubeTexture9(This);
            unsigned z;
            struct pipe_box box;
            box.z = 0;
            box.depth = 1;

            for (z = 0; z < 6; ++z) {
                DBG("FACE[%u]: dirty rect=(%u,%u) (%ux%u)\n", z,
                    tex->dirty_rect[z].x, tex->dirty_rect[z].y,
                    tex->dirty_rect[z].width, tex->dirty_rect[z].height);

                if (tex->dirty_rect[z].width) {
                    for (l = min_level_dirty; l < This->level_count; ++l) {
                        u_box_minify_2d(&box, &tex->dirty_rect[z], l);
                        NineSurface9_UploadSelf(tex->surfaces[l * 6 + z], &box);
                    }
                    memset(&tex->dirty_rect[z], 0, sizeof(tex->dirty_rect[z]));
                    tex->dirty_rect[z].depth = 1;
                }
            }
        } else
        if (This->base.type == D3DRTYPE_VOLUMETEXTURE) {
            struct NineVolumeTexture9 *tex = NineVolumeTexture9(This);
            struct pipe_box box;

            DBG("VOLUME: dirty_box=(%u,%u,%u) (%ux%ux%u)\n",
                tex->dirty_box.x, tex->dirty_box.y, tex->dirty_box.y,
                tex->dirty_box.width, tex->dirty_box.height, tex->dirty_box.depth);

            if (tex->dirty_box.width) {
                for (l = min_level_dirty; l < This->level_count; ++l) {
                    u_box_minify_3d(&box, &tex->dirty_box, l);
                    NineVolume9_UploadSelf(tex->volumes[l], &box);
                }
                memset(&tex->dirty_box, 0, sizeof(tex->dirty_box));
            }
        } else {
            assert(!"invalid texture type");
        }
        This->managed.dirty = false;
    }

    /* Upload the new levels */
    if (update_lod) {
        if (This->base.type == D3DRTYPE_TEXTURE) {
            struct NineTexture9 *tex = NineTexture9(This);
            struct pipe_box box;

            box.x = box.y = box.z = 0;
            box.depth = 1;
            for (l = This->managed.lod; l < This->managed.lod_resident; ++l) {
                box.width = u_minify(This->base.info.width0, l);
                box.height = u_minify(This->base.info.height0, l);
                NineSurface9_UploadSelf(tex->surfaces[l], &box);
            }
        } else
        if (This->base.type == D3DRTYPE_CUBETEXTURE) {
            struct NineCubeTexture9 *tex = NineCubeTexture9(This);
            struct pipe_box box;
            unsigned z;

            box.x = box.y = box.z = 0;
            box.depth = 1;
            for (l = This->managed.lod; l < This->managed.lod_resident; ++l) {
                box.width = u_minify(This->base.info.width0, l);
                box.height = u_minify(This->base.info.height0, l);
                for (z = 0; z < 6; ++z)
                    NineSurface9_UploadSelf(tex->surfaces[l * 6 + z], &box);
            }
        } else
        if (This->base.type == D3DRTYPE_VOLUMETEXTURE) {
            struct NineVolumeTexture9 *tex = NineVolumeTexture9(This);
            struct pipe_box box;

            box.x = box.y = box.z = 0;
            for (l = This->managed.lod; l < This->managed.lod_resident; ++l) {
                box.width = u_minify(This->base.info.width0, l);
                box.height = u_minify(This->base.info.height0, l);
                box.depth = u_minify(This->base.info.depth0, l);
                NineVolume9_UploadSelf(tex->volumes[l], &box);
            }
        } else {
            assert(!"invalid texture type");
        }

        This->managed.lod_resident = This->managed.lod;
    }

    if (This->base.usage & D3DUSAGE_AUTOGENMIPMAP)
        This->dirty_mip = true;

    /* Set again the textures currently bound to update the texture data */
    if (This->bind_count) {
        struct nine_state *state = &This->base.base.device->state;
        unsigned s;
        for (s = 0; s < NINE_MAX_SAMPLERS; ++s)
            /* Dirty tracking is done in device9 state, not nine_context. */
            if (state->texture[s] == This)
                nine_context_set_texture(This->base.base.device, s, This);
    }

    DBG("DONE, generate mip maps = %i\n", This->dirty_mip);
    return D3D_OK;
}

void NINE_WINAPI
NineBaseTexture9_GenerateMipSubLevels( struct NineBaseTexture9 *This )
{
    unsigned base_level = 0;
    unsigned last_level = This->base.info.last_level - This->managed.lod;
    unsigned first_layer = 0;
    unsigned last_layer;
    unsigned filter = This->mipfilter == D3DTEXF_POINT ? PIPE_TEX_FILTER_NEAREST
                                                       : PIPE_TEX_FILTER_LINEAR;
    DBG("This=%p\n", This);

    if (This->base.pool == D3DPOOL_MANAGED)
        NineBaseTexture9_UploadSelf(This);
    if (!This->dirty_mip)
        return;
    if (This->managed.lod) {
        ERR("AUTOGENMIPMAP if level 0 is not resident not supported yet !\n");
        return;
    }

    if (!This->view[0])
        NineBaseTexture9_UpdateSamplerView(This, 0);

    last_layer = util_max_layer(This->view[0]->texture, base_level);

    nine_context_gen_mipmap(This->base.base.device, (struct NineUnknown *)This,
                            This->base.resource,
                            base_level, last_level,
                            first_layer, last_layer, filter);

    This->dirty_mip = false;
}

HRESULT
NineBaseTexture9_CreatePipeResource( struct NineBaseTexture9 *This,
                                     BOOL CopyData )
{
    struct pipe_context *pipe;
    struct pipe_screen *screen = This->base.info.screen;
    struct pipe_resource templ;
    unsigned l, m;
    struct pipe_resource *res;
    struct pipe_resource *old = This->base.resource;

    DBG("This=%p lod=%u last_level=%u\n", This,
        This->managed.lod, This->base.info.last_level);

    assert(This->base.pool == D3DPOOL_MANAGED);

    templ = This->base.info;

    if (This->managed.lod) {
        templ.width0 = u_minify(templ.width0, This->managed.lod);
        templ.height0 = u_minify(templ.height0, This->managed.lod);
        templ.depth0 = u_minify(templ.depth0, This->managed.lod);
    }
    templ.last_level = This->base.info.last_level - This->managed.lod;

    if (old) {
        /* LOD might have changed. */
        if (old->width0 == templ.width0 &&
            old->height0 == templ.height0 &&
            old->depth0 == templ.depth0)
            return D3D_OK;
    }

    res = nine_resource_create_with_retry(This->base.base.device, screen, &templ);
    if (!res)
        return D3DERR_OUTOFVIDEOMEMORY;
    This->base.resource = res;

    if (old && CopyData) { /* Don't return without releasing old ! */
        struct pipe_box box;
        box.x = 0;
        box.y = 0;
        box.z = 0;

        l = (This->managed.lod < This->managed.lod_resident) ? This->managed.lod_resident - This->managed.lod : 0;
        m = (This->managed.lod < This->managed.lod_resident) ? 0 : This->managed.lod - This->managed.lod_resident;

        box.width = u_minify(templ.width0, l);
        box.height = u_minify(templ.height0, l);
        box.depth = u_minify(templ.depth0, l);

        pipe = nine_context_get_pipe_acquire(This->base.base.device);

        for (; l <= templ.last_level; ++l, ++m) {
            pipe->resource_copy_region(pipe,
                                       res, l, 0, 0, 0,
                                       old, m, &box);
            box.width = u_minify(box.width, 1);
            box.height = u_minify(box.height, 1);
            box.depth = u_minify(box.depth, 1);
        }

        nine_context_get_pipe_release(This->base.base.device);
    }
    pipe_resource_reference(&old, NULL);

    return D3D_OK;
}

#define SWIZZLE_TO_REPLACE(s) (s == PIPE_SWIZZLE_0 || \
                               s == PIPE_SWIZZLE_1 || \
                               s == PIPE_SWIZZLE_NONE)

HRESULT
NineBaseTexture9_UpdateSamplerView( struct NineBaseTexture9 *This,
                                    const int sRGB )
{
    const struct util_format_description *desc;
    struct pipe_context *pipe;
    struct pipe_screen *screen = NineDevice9_GetScreen(This->base.base.device);
    struct pipe_resource *resource = This->base.resource;
    struct pipe_sampler_view templ;
    enum pipe_format srgb_format;
    unsigned i;
    uint8_t swizzle[4];
    memset(&templ, 0, sizeof(templ));

    DBG("This=%p sRGB=%d\n", This, sRGB);

    if (unlikely(!resource)) {
	if (unlikely(This->format == D3DFMT_NULL))
            return D3D_OK;
        NineBaseTexture9_Dump(This);
    }
    assert(resource);

    pipe_sampler_view_reference(&This->view[sRGB], NULL);

    swizzle[0] = PIPE_SWIZZLE_X;
    swizzle[1] = PIPE_SWIZZLE_Y;
    swizzle[2] = PIPE_SWIZZLE_Z;
    swizzle[3] = PIPE_SWIZZLE_W;
    desc = util_format_description(resource->format);
    if (desc->colorspace == UTIL_FORMAT_COLORSPACE_ZS) {
        /* msdn doc is incomplete here and wrong.
         * The only formats that can be read directly here
         * are DF16, DF24 and INTZ.
         * Tested on win the swizzle is
         * R = depth, G = B = 0, A = 1 for DF16 and DF24
         * R = G = B = A = depth for INTZ
         * For the other ZS formats that can't be read directly
         * but can be used as shadow map, the result is duplicated on
         * all channel */
        if (This->format == D3DFMT_DF16 ||
            This->format == D3DFMT_DF24) {
            swizzle[1] = PIPE_SWIZZLE_0;
            swizzle[2] = PIPE_SWIZZLE_0;
            swizzle[3] = PIPE_SWIZZLE_1;
        } else {
            swizzle[1] = PIPE_SWIZZLE_X;
            swizzle[2] = PIPE_SWIZZLE_X;
            swizzle[3] = PIPE_SWIZZLE_X;
        }
    } else if (resource->format == PIPE_FORMAT_RGTC2_UNORM) {
        swizzle[0] = PIPE_SWIZZLE_Y;
        swizzle[1] = PIPE_SWIZZLE_X;
        swizzle[2] = PIPE_SWIZZLE_1;
        swizzle[3] = PIPE_SWIZZLE_1;
    } else if (resource->format != PIPE_FORMAT_A8_UNORM &&
               resource->format != PIPE_FORMAT_RGTC1_UNORM) {
        /* exceptions:
         * A8 should have 0.0 as default values for RGB.
         * ATI1/RGTC1 should be r 0 0 1 (tested on windows).
         * It is already what gallium does. All the other ones
         * should have 1.0 for non-defined values */
        for (i = 0; i < 4; i++) {
            if (SWIZZLE_TO_REPLACE(desc->swizzle[i]))
                swizzle[i] = PIPE_SWIZZLE_1;
        }
    }

    /* if requested and supported, convert to the sRGB format */
    srgb_format = util_format_srgb(resource->format);
    if (sRGB && srgb_format != PIPE_FORMAT_NONE &&
        screen->is_format_supported(screen, srgb_format,
                                    resource->target, 0, 0, resource->bind))
        templ.format = srgb_format;
    else
        templ.format = resource->format;
    templ.u.tex.first_layer = 0;
    templ.u.tex.last_layer = resource->target == PIPE_TEXTURE_3D ?
                             0 : resource->array_size - 1;
    templ.u.tex.first_level = 0;
    templ.u.tex.last_level = resource->last_level;
    templ.swizzle_r = swizzle[0];
    templ.swizzle_g = swizzle[1];
    templ.swizzle_b = swizzle[2];
    templ.swizzle_a = swizzle[3];
    templ.target = resource->target;

    pipe = nine_context_get_pipe_acquire(This->base.base.device);
    This->view[sRGB] = pipe->create_sampler_view(pipe, resource, &templ);
    nine_context_get_pipe_release(This->base.base.device);

    DBG("sampler view = %p(resource = %p)\n", This->view[sRGB], resource);

    return This->view[sRGB] ? D3D_OK : D3DERR_DRIVERINTERNALERROR;
}

void NINE_WINAPI
NineBaseTexture9_PreLoad( struct NineBaseTexture9 *This )
{
    DBG("This=%p\n", This);

    if (This->base.pool == D3DPOOL_MANAGED)
        NineBaseTexture9_UploadSelf(This);
}

void
NineBaseTexture9_UnLoad( struct NineBaseTexture9 *This )
{
    DBG("This=%p\n", This);

    if (This->base.pool != D3DPOOL_MANAGED ||
        This->managed.lod_resident == -1)
        return;

    DBG("This=%p, releasing resource\n", This);
    pipe_resource_reference(&This->base.resource, NULL);
    This->managed.lod_resident = -1;
    This->managed.dirty = true;

    /* If the texture is bound, we have to re-upload it */
    BASETEX_REGISTER_UPDATE(This);
}

#if defined(DEBUG) || !defined(NDEBUG)
void
NineBaseTexture9_Dump( struct NineBaseTexture9 *This )
{
    DBG("\nNineBaseTexture9(%p->NULL/%p): Pool=%s Type=%s Usage=%s\n"
        "Format=%s Dims=%ux%ux%u/%u LastLevel=%u Lod=%u(%u)\n", This,
        This->base.resource,
        nine_D3DPOOL_to_str(This->base.pool),
        nine_D3DRTYPE_to_str(This->base.type),
        nine_D3DUSAGE_to_str(This->base.usage),
        d3dformat_to_string(This->format),
        This->base.info.width0, This->base.info.height0, This->base.info.depth0,
        This->base.info.array_size, This->base.info.last_level,
        This->managed.lod, This->managed.lod_resident);
}
#endif /* DEBUG || !NDEBUG */
