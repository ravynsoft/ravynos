/*
 * Copyright © 2014 Broadcom
 * Copyright (C) 2012 Rob Clark <robclark@freedesktop.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "pipe/p_defines.h"
#include "util/u_memory.h"
#include "util/format/u_format.h"
#include "util/u_inlines.h"
#include "util/u_resource.h"
#include "util/u_surface.h"
#include "util/u_transfer_helper.h"
#include "util/u_upload_mgr.h"
#include "util/u_drm.h"

#include "drm-uapi/drm_fourcc.h"
#include "drm-uapi/vc4_drm.h"
#include "vc4_screen.h"
#include "vc4_context.h"
#include "vc4_resource.h"
#include "vc4_tiling.h"

static bool
vc4_resource_bo_alloc(struct vc4_resource *rsc)
{
        struct pipe_resource *prsc = &rsc->base;
        struct pipe_screen *pscreen = prsc->screen;
        struct vc4_bo *bo;

        if (VC4_DBG(SURFACE)) {
                fprintf(stderr, "alloc %p: size %d + offset %d -> %d\n",
                        rsc,
                        rsc->slices[0].size,
                        rsc->slices[0].offset,
                        rsc->slices[0].offset +
                        rsc->slices[0].size +
                        rsc->cube_map_stride * (prsc->array_size - 1));
        }

        bo = vc4_bo_alloc(vc4_screen(pscreen),
                          rsc->slices[0].offset +
                          rsc->slices[0].size +
                          rsc->cube_map_stride * (prsc->array_size - 1),
                          "resource");
        if (bo) {
                vc4_bo_unreference(&rsc->bo);
                rsc->bo = bo;
                return true;
        } else {
                return false;
        }
}

static void
vc4_resource_transfer_unmap(struct pipe_context *pctx,
                            struct pipe_transfer *ptrans)
{
        struct vc4_context *vc4 = vc4_context(pctx);
        struct vc4_transfer *trans = vc4_transfer(ptrans);

        if (trans->map) {
                struct vc4_resource *rsc = vc4_resource(ptrans->resource);
                struct vc4_resource_slice *slice = &rsc->slices[ptrans->level];

                if (ptrans->usage & PIPE_MAP_WRITE) {
                        vc4_store_tiled_image(rsc->bo->map + slice->offset +
                                              ptrans->box.z * rsc->cube_map_stride,
                                              slice->stride,
                                              trans->map, ptrans->stride,
                                              slice->tiling, rsc->cpp,
                                              &ptrans->box);
                }
                free(trans->map);
        }

        pipe_resource_reference(&ptrans->resource, NULL);
        slab_free(&vc4->transfer_pool, ptrans);
}

static void
vc4_map_usage_prep(struct pipe_context *pctx,
                   struct pipe_resource *prsc,
                   unsigned usage)
{
        struct vc4_context *vc4 = vc4_context(pctx);
        struct vc4_resource *rsc = vc4_resource(prsc);

        if (usage & PIPE_MAP_DISCARD_WHOLE_RESOURCE) {
                if (vc4_resource_bo_alloc(rsc)) {
                        /* If it might be bound as one of our vertex buffers,
                         * make sure we re-emit vertex buffer state.
                         */
                        if (prsc->bind & PIPE_BIND_VERTEX_BUFFER)
                                vc4->dirty |= VC4_DIRTY_VTXBUF;
                        if (prsc->bind & PIPE_BIND_CONSTANT_BUFFER)
                                vc4->dirty |= VC4_DIRTY_CONSTBUF;
                } else {
                        /* If we failed to reallocate, flush users so that we
                         * don't violate any syncing requirements.
                         */
                        vc4_flush_jobs_reading_resource(vc4, prsc);
                }
        } else if (!(usage & PIPE_MAP_UNSYNCHRONIZED)) {
                /* If we're writing and the buffer is being used by the CL, we
                 * have to flush the CL first. If we're only reading, we need
                 * to flush if the CL has written our buffer.
                 */
                if (usage & PIPE_MAP_WRITE)
                        vc4_flush_jobs_reading_resource(vc4, prsc);
                else
                        vc4_flush_jobs_writing_resource(vc4, prsc);
        }

        if (usage & PIPE_MAP_WRITE) {
                rsc->writes++;
                rsc->initialized_buffers = ~0;
        }
}

static void *
vc4_resource_transfer_map(struct pipe_context *pctx,
                          struct pipe_resource *prsc,
                          unsigned level, unsigned usage,
                          const struct pipe_box *box,
                          struct pipe_transfer **pptrans)
{
        struct vc4_context *vc4 = vc4_context(pctx);
        struct vc4_resource *rsc = vc4_resource(prsc);
        struct vc4_transfer *trans;
        struct pipe_transfer *ptrans;
        enum pipe_format format = prsc->format;
        char *buf;

        /* Upgrade DISCARD_RANGE to WHOLE_RESOURCE if the whole resource is
         * being mapped.
         */
        if ((usage & PIPE_MAP_DISCARD_RANGE) &&
            !(usage & PIPE_MAP_UNSYNCHRONIZED) &&
            !(prsc->flags & PIPE_RESOURCE_FLAG_MAP_PERSISTENT) &&
            prsc->last_level == 0 &&
            prsc->width0 == box->width &&
            prsc->height0 == box->height &&
            prsc->depth0 == box->depth &&
            prsc->array_size == 1 &&
            rsc->bo->private) {
                usage |= PIPE_MAP_DISCARD_WHOLE_RESOURCE;
        }

        vc4_map_usage_prep(pctx, prsc, usage);

        trans = slab_zalloc(&vc4->transfer_pool);
        if (!trans)
                return NULL;

        /* XXX: Handle DONTBLOCK, DISCARD_RANGE, PERSISTENT, COHERENT. */

        ptrans = &trans->base;

        pipe_resource_reference(&ptrans->resource, prsc);
        ptrans->level = level;
        ptrans->usage = usage;
        ptrans->box = *box;

        if (usage & PIPE_MAP_UNSYNCHRONIZED)
                buf = vc4_bo_map_unsynchronized(rsc->bo);
        else
                buf = vc4_bo_map(rsc->bo);
        if (!buf) {
                fprintf(stderr, "Failed to map bo\n");
                goto fail;
        }

        *pptrans = ptrans;

        struct vc4_resource_slice *slice = &rsc->slices[level];
        if (rsc->tiled) {
                /* No direct mappings of tiled, since we need to manually
                 * tile/untile.
                 */
                if (usage & PIPE_MAP_DIRECTLY)
                        return NULL;

                /* Our load/store routines work on entire compressed blocks. */
                u_box_pixels_to_blocks(&ptrans->box, &ptrans->box, format);

                ptrans->stride = ptrans->box.width * rsc->cpp;
                ptrans->layer_stride = ptrans->stride * ptrans->box.height;

                trans->map = malloc(ptrans->layer_stride * ptrans->box.depth);

                if (usage & PIPE_MAP_READ) {
                        vc4_load_tiled_image(trans->map, ptrans->stride,
                                             buf + slice->offset +
                                             ptrans->box.z * rsc->cube_map_stride,
                                             slice->stride,
                                             slice->tiling, rsc->cpp,
                                             &ptrans->box);
                }
                return trans->map;
        } else {
                ptrans->stride = slice->stride;
                ptrans->layer_stride = ptrans->stride;

                return buf + slice->offset +
                        ptrans->box.y / util_format_get_blockheight(format) * ptrans->stride +
                        ptrans->box.x / util_format_get_blockwidth(format) * rsc->cpp +
                        ptrans->box.z * rsc->cube_map_stride;
        }


fail:
        vc4_resource_transfer_unmap(pctx, ptrans);
        return NULL;
}

static void
vc4_texture_subdata(struct pipe_context *pctx,
                    struct pipe_resource *prsc,
                    unsigned level,
                    unsigned usage,
                    const struct pipe_box *box,
                    const void *data,
                    unsigned stride,
                    uintptr_t layer_stride)
{
        struct vc4_resource *rsc = vc4_resource(prsc);
        struct vc4_resource_slice *slice = &rsc->slices[level];

        /* For a direct mapping, we can just take the u_transfer path. */
        if (!rsc->tiled ||
            box->depth != 1 ||
            (usage & PIPE_MAP_DISCARD_WHOLE_RESOURCE)) {
                return u_default_texture_subdata(pctx, prsc, level, usage, box,
                                                 data, stride, layer_stride);
        }

        /* Otherwise, map and store the texture data directly into the tiled
         * texture.  Note that gallium's texture_subdata may be called with
         * obvious usage flags missing!
         */
        vc4_map_usage_prep(pctx, prsc, usage | (PIPE_MAP_WRITE |
                                                PIPE_MAP_DISCARD_RANGE));

        void *buf;
        if (usage & PIPE_MAP_UNSYNCHRONIZED)
                buf = vc4_bo_map_unsynchronized(rsc->bo);
        else
                buf = vc4_bo_map(rsc->bo);

        vc4_store_tiled_image(buf + slice->offset +
                              box->z * rsc->cube_map_stride,
                              slice->stride,
                              (void *)data, stride,
                              slice->tiling, rsc->cpp,
                              box);
}

static void
vc4_resource_destroy(struct pipe_screen *pscreen,
                     struct pipe_resource *prsc)
{
        struct vc4_screen *screen = vc4_screen(pscreen);
        struct vc4_resource *rsc = vc4_resource(prsc);
        vc4_bo_unreference(&rsc->bo);

        if (rsc->scanout)
                renderonly_scanout_destroy(rsc->scanout, screen->ro);

        free(rsc);
}

static uint64_t
vc4_resource_modifier(struct vc4_resource *rsc)
{
        if (rsc->tiled)
                return DRM_FORMAT_MOD_BROADCOM_VC4_T_TILED;
        else
                return DRM_FORMAT_MOD_LINEAR;
}

static bool
vc4_resource_get_handle(struct pipe_screen *pscreen,
                        struct pipe_context *pctx,
                        struct pipe_resource *prsc,
                        struct winsys_handle *whandle,
                        unsigned usage)
{
        struct vc4_screen *screen = vc4_screen(pscreen);
        struct vc4_resource *rsc = vc4_resource(prsc);

        whandle->stride = rsc->slices[0].stride;
        whandle->offset = 0;
        whandle->modifier = vc4_resource_modifier(rsc);

        /* If we're passing some reference to our BO out to some other part of
         * the system, then we can't do any optimizations about only us being
         * the ones seeing it (like BO caching or shadow update avoidance).
         */
        rsc->bo->private = false;

        switch (whandle->type) {
        case WINSYS_HANDLE_TYPE_SHARED:
                if (screen->ro) {
                        /* This could probably be supported, assuming that a
                         * control node was used for pl111.
                         */
                        fprintf(stderr, "flink unsupported with pl111\n");
                        return false;
                }

                return vc4_bo_flink(rsc->bo, &whandle->handle);
        case WINSYS_HANDLE_TYPE_KMS:
                if (screen->ro) {
                        return renderonly_get_handle(rsc->scanout, whandle);
                }
                whandle->handle = rsc->bo->handle;
                return true;
        case WINSYS_HANDLE_TYPE_FD:
                /* FDs are cross-device, so we can export directly from vc4.
                 */
                whandle->handle = vc4_bo_get_dmabuf(rsc->bo);
                return whandle->handle != -1;
        }

        return false;
}

static bool
vc4_resource_get_param(struct pipe_screen *pscreen,
                       struct pipe_context *pctx, struct pipe_resource *prsc,
                       unsigned plane, unsigned layer, unsigned level,
                       enum pipe_resource_param param,
                       unsigned usage, uint64_t *value)
{
        struct vc4_resource *rsc =
                (struct vc4_resource *)util_resource_at_index(prsc, plane);

        switch (param) {
        case PIPE_RESOURCE_PARAM_STRIDE:
                *value = rsc->slices[level].stride;
                return true;
        case PIPE_RESOURCE_PARAM_OFFSET:
                *value = rsc->slices[level].offset;
                return true;
        case PIPE_RESOURCE_PARAM_MODIFIER:
                *value = vc4_resource_modifier(rsc);
                return true;
        case PIPE_RESOURCE_PARAM_NPLANES:
                *value = util_resource_num(prsc);
                return true;
        default:
                return false;
        }
}

static void
vc4_setup_slices(struct vc4_resource *rsc, const char *caller)
{
        struct pipe_resource *prsc = &rsc->base;
        uint32_t width = prsc->width0;
        uint32_t height = prsc->height0;
        if (prsc->format == PIPE_FORMAT_ETC1_RGB8) {
                width = (width + 3) >> 2;
                height = (height + 3) >> 2;
        }

        uint32_t pot_width = util_next_power_of_two(width);
        uint32_t pot_height = util_next_power_of_two(height);
        uint32_t offset = 0;
        uint32_t utile_w = vc4_utile_width(rsc->cpp);
        uint32_t utile_h = vc4_utile_height(rsc->cpp);

        for (int i = prsc->last_level; i >= 0; i--) {
                struct vc4_resource_slice *slice = &rsc->slices[i];

                uint32_t level_width, level_height;
                if (i == 0) {
                        level_width = width;
                        level_height = height;
                } else {
                        level_width = u_minify(pot_width, i);
                        level_height = u_minify(pot_height, i);
                }

                if (!rsc->tiled) {
                        slice->tiling = VC4_TILING_FORMAT_LINEAR;
                        if (prsc->nr_samples > 1) {
                                /* MSAA (4x) surfaces are stored as raw tile buffer contents. */
                                level_width = align(level_width, 32);
                                level_height = align(level_height, 32);
                        } else {
                                level_width = align(level_width, utile_w);
                        }
                } else {
                        if (vc4_size_is_lt(level_width, level_height,
                                           rsc->cpp)) {
                                slice->tiling = VC4_TILING_FORMAT_LT;
                                level_width = align(level_width, utile_w);
                                level_height = align(level_height, utile_h);
                        } else {
                                slice->tiling = VC4_TILING_FORMAT_T;
                                level_width = align(level_width,
                                                    4 * 2 * utile_w);
                                level_height = align(level_height,
                                                     4 * 2 * utile_h);
                        }
                }

                slice->offset = offset;
                slice->stride = (level_width * rsc->cpp *
                                 MAX2(prsc->nr_samples, 1));
                slice->size = level_height * slice->stride;

                offset += slice->size;

                if (VC4_DBG(SURFACE)) {
                        static const char tiling_chars[] = {
                                [VC4_TILING_FORMAT_LINEAR] = 'R',
                                [VC4_TILING_FORMAT_LT] = 'L',
                                [VC4_TILING_FORMAT_T] = 'T'
                        };
                        fprintf(stderr,
                                "rsc %s %p (format %s: vc4 %d), %dx%d: "
                                "level %d (%c) -> %dx%d, stride %d@0x%08x\n",
                                caller, rsc,
                                util_format_short_name(prsc->format),
                                rsc->vc4_format,
                                prsc->width0, prsc->height0,
                                i, tiling_chars[slice->tiling],
                                level_width, level_height,
                                slice->stride, slice->offset);
                }
        }

        /* The texture base pointer that has to point to level 0 doesn't have
         * intra-page bits, so we have to align it, and thus shift up all the
         * smaller slices.
         */
        uint32_t page_align_offset = (align(rsc->slices[0].offset, 4096) -
                                      rsc->slices[0].offset);
        if (page_align_offset) {
                for (int i = 0; i <= prsc->last_level; i++)
                        rsc->slices[i].offset += page_align_offset;
        }

        /* Cube map faces appear as whole miptrees at a page-aligned offset
         * from the first face's miptree.
         */
        if (prsc->target == PIPE_TEXTURE_CUBE) {
                rsc->cube_map_stride = align(rsc->slices[0].offset +
                                             rsc->slices[0].size, 4096);
        }
}

static struct vc4_resource *
vc4_resource_setup(struct pipe_screen *pscreen,
                   const struct pipe_resource *tmpl)
{
        struct vc4_resource *rsc = CALLOC_STRUCT(vc4_resource);
        if (!rsc)
                return NULL;
        struct pipe_resource *prsc = &rsc->base;

        *prsc = *tmpl;

        pipe_reference_init(&prsc->reference, 1);
        prsc->screen = pscreen;

        if (prsc->nr_samples <= 1)
                rsc->cpp = util_format_get_blocksize(tmpl->format);
        else
                rsc->cpp = sizeof(uint32_t);

        assert(rsc->cpp);

        return rsc;
}

static enum vc4_texture_data_type
get_resource_texture_format(struct pipe_resource *prsc)
{
        struct vc4_resource *rsc = vc4_resource(prsc);
        uint8_t format = vc4_get_tex_format(prsc->format);

        if (!rsc->tiled) {
                if (prsc->nr_samples > 1) {
                        return ~0;
                } else {
                        if (format == VC4_TEXTURE_TYPE_RGBA8888)
                                return VC4_TEXTURE_TYPE_RGBA32R;
                        else
                                return ~0;
                }
        }

        return format;
}

static struct pipe_resource *
vc4_resource_create_with_modifiers(struct pipe_screen *pscreen,
                                   const struct pipe_resource *tmpl,
                                   const uint64_t *modifiers,
                                   int count)
{
        struct vc4_screen *screen = vc4_screen(pscreen);
        struct vc4_resource *rsc = vc4_resource_setup(pscreen, tmpl);
        struct pipe_resource *prsc = &rsc->base;
        bool linear_ok = drm_find_modifier(DRM_FORMAT_MOD_LINEAR, modifiers, count);
        /* Use a tiled layout if we can, for better 3D performance. */
        bool should_tile = true;

        /* VBOs/PBOs are untiled (and 1 height). */
        if (tmpl->target == PIPE_BUFFER)
                should_tile = false;

        /* MSAA buffers are linear. */
        if (tmpl->nr_samples > 1)
                should_tile = false;

        /* No tiling when we're sharing with another device (pl111). */
        if (screen->ro && (tmpl->bind & PIPE_BIND_SCANOUT))
                should_tile = false;

        /* Cursors are always linear, and the user can request linear as well.
         */
        if (tmpl->bind & (PIPE_BIND_LINEAR | PIPE_BIND_CURSOR))
                should_tile = false;

        /* No shared objects with LT format -- the kernel only has T-format
         * metadata.  LT objects are small enough it's not worth the trouble to
         * give them metadata to tile.
         */
        if ((tmpl->bind & (PIPE_BIND_SHARED | PIPE_BIND_SCANOUT)) &&
            vc4_size_is_lt(prsc->width0, prsc->height0, rsc->cpp))
                should_tile = false;

        /* If we're sharing or scanning out, we need the ioctl present to
         * inform the kernel or the other side.
         */
        if ((tmpl->bind & (PIPE_BIND_SHARED |
                           PIPE_BIND_SCANOUT)) && !screen->has_tiling_ioctl)
                should_tile = false;

        /* No user-specified modifier; determine our own. */
        if (count == 1 && modifiers[0] == DRM_FORMAT_MOD_INVALID) {
                linear_ok = true;
                rsc->tiled = should_tile;
        } else if (should_tile &&
                   drm_find_modifier(DRM_FORMAT_MOD_BROADCOM_VC4_T_TILED,
                                 modifiers, count)) {
                rsc->tiled = true;
        } else if (linear_ok) {
                rsc->tiled = false;
        } else {
                fprintf(stderr, "Unsupported modifier requested\n");
                return NULL;
        }

        if (tmpl->target != PIPE_BUFFER)
                rsc->vc4_format = get_resource_texture_format(prsc);

        vc4_setup_slices(rsc, "create");
        if (!vc4_resource_bo_alloc(rsc))
                goto fail;

        if (screen->has_tiling_ioctl) {
                uint64_t modifier;
                if (rsc->tiled)
                        modifier = DRM_FORMAT_MOD_BROADCOM_VC4_T_TILED;
                else
                        modifier = DRM_FORMAT_MOD_LINEAR;
                struct drm_vc4_set_tiling set_tiling = {
                        .handle = rsc->bo->handle,
                        .modifier = modifier,
                };
                int ret = vc4_ioctl(screen->fd, DRM_IOCTL_VC4_SET_TILING,
                                    &set_tiling);
                if (ret != 0)
                        goto fail;
        }

        /* Set up the "scanout resource" (the dmabuf export of our buffer to
         * the KMS handle) if the buffer might ever have
         * resource_get_handle(WINSYS_HANDLE_TYPE_KMS) called on it.
         * create_with_modifiers() doesn't give us usage flags, so we have to
         * assume that all calls with modifiers are scanout-possible.
         */
        if (screen->ro &&
            ((tmpl->bind & PIPE_BIND_SCANOUT) ||
             !(count == 1 && modifiers[0] == DRM_FORMAT_MOD_INVALID))) {
                rsc->scanout =
                        renderonly_scanout_for_resource(prsc, screen->ro, NULL);
                if (!rsc->scanout)
                        goto fail;
        }

        vc4_bo_label(screen, rsc->bo, "%sresource %dx%d@%d/%d",
                     (tmpl->bind & PIPE_BIND_SCANOUT) ? "scanout " : "",
                     tmpl->width0, tmpl->height0,
                     rsc->cpp * 8, prsc->last_level);

        return prsc;
fail:
        vc4_resource_destroy(pscreen, prsc);
        return NULL;
}

struct pipe_resource *
vc4_resource_create(struct pipe_screen *pscreen,
                    const struct pipe_resource *tmpl)
{
        const uint64_t mod = DRM_FORMAT_MOD_INVALID;
        return vc4_resource_create_with_modifiers(pscreen, tmpl, &mod, 1);
}

static struct pipe_resource *
vc4_resource_from_handle(struct pipe_screen *pscreen,
                         const struct pipe_resource *tmpl,
                         struct winsys_handle *whandle,
                         unsigned usage)
{
        struct vc4_screen *screen = vc4_screen(pscreen);
        struct vc4_resource *rsc = vc4_resource_setup(pscreen, tmpl);
        struct pipe_resource *prsc = &rsc->base;
        struct vc4_resource_slice *slice = &rsc->slices[0];

        if (!rsc)
                return NULL;

        switch (whandle->type) {
        case WINSYS_HANDLE_TYPE_SHARED:
                rsc->bo = vc4_bo_open_name(screen, whandle->handle);
                break;
        case WINSYS_HANDLE_TYPE_FD:
                rsc->bo = vc4_bo_open_dmabuf(screen, whandle->handle);
                break;
        default:
                fprintf(stderr,
                        "Attempt to import unsupported handle type %d\n",
                        whandle->type);
        }

        if (!rsc->bo)
                goto fail;

        struct drm_vc4_get_tiling get_tiling = {
                .handle = rsc->bo->handle,
        };
        int ret = vc4_ioctl(screen->fd, DRM_IOCTL_VC4_GET_TILING, &get_tiling);

        if (ret != 0) {
                whandle->modifier = DRM_FORMAT_MOD_LINEAR;
        } else if (whandle->modifier == DRM_FORMAT_MOD_INVALID) {
                whandle->modifier = get_tiling.modifier;
        } else if (whandle->modifier != get_tiling.modifier) {
                fprintf(stderr,
                        "Modifier 0x%llx vs. tiling (0x%llx) mismatch\n",
                        (long long)whandle->modifier, get_tiling.modifier);
                goto fail;
        }

        switch (whandle->modifier) {
        case DRM_FORMAT_MOD_LINEAR:
                rsc->tiled = false;
                break;
        case DRM_FORMAT_MOD_BROADCOM_VC4_T_TILED:
                rsc->tiled = true;
                break;
        default:
                fprintf(stderr,
                        "Attempt to import unsupported modifier 0x%llx\n",
                        (long long)whandle->modifier);
                goto fail;
        }

        rsc->vc4_format = get_resource_texture_format(prsc);
        vc4_setup_slices(rsc, "import");

        if (whandle->offset != 0) {
                if (rsc->tiled) {
                        fprintf(stderr,
                                "Attempt to import unsupported "
                                "winsys offset %u\n",
                                whandle->offset);
                        goto fail;
                }

                rsc->slices[0].offset += whandle->offset;

                if (rsc->slices[0].offset + rsc->slices[0].size >
                    rsc->bo->size) {
                        fprintf(stderr, "Attempt to import "
                                "with overflowing offset (%d + %d > %d)\n",
                                whandle->offset,
                                rsc->slices[0].size,
                                rsc->bo->size);
                        goto fail;
                }
        }

        if (screen->ro) {
                /* Make sure that renderonly has a handle to our buffer in the
                 * display's fd, so that a later renderonly_get_handle()
                 * returns correct handles or GEM names.
                 */
                rsc->scanout =
                        renderonly_create_gpu_import_for_resource(prsc,
                                                                  screen->ro,
                                                                  NULL);
        }

        if (rsc->tiled && whandle->stride != slice->stride) {
                static bool warned = false;
                if (!warned) {
                        warned = true;
                        fprintf(stderr,
                                "Attempting to import %dx%d %s with "
                                "unsupported stride %d instead of %d\n",
                                prsc->width0, prsc->height0,
                                util_format_short_name(prsc->format),
                                whandle->stride,
                                slice->stride);
                }
                goto fail;
        } else if (!rsc->tiled) {
                slice->stride = whandle->stride;
        }

        return prsc;

fail:
        vc4_resource_destroy(pscreen, prsc);
        return NULL;
}

static struct pipe_surface *
vc4_create_surface(struct pipe_context *pctx,
                   struct pipe_resource *ptex,
                   const struct pipe_surface *surf_tmpl)
{
        struct vc4_surface *surface = CALLOC_STRUCT(vc4_surface);
        struct vc4_resource *rsc = vc4_resource(ptex);

        if (!surface)
                return NULL;

        assert(surf_tmpl->u.tex.first_layer == surf_tmpl->u.tex.last_layer);

        struct pipe_surface *psurf = &surface->base;
        unsigned level = surf_tmpl->u.tex.level;

        pipe_reference_init(&psurf->reference, 1);
        pipe_resource_reference(&psurf->texture, ptex);

        psurf->context = pctx;
        psurf->format = surf_tmpl->format;
        psurf->width = u_minify(ptex->width0, level);
        psurf->height = u_minify(ptex->height0, level);
        psurf->u.tex.level = level;
        psurf->u.tex.first_layer = surf_tmpl->u.tex.first_layer;
        psurf->u.tex.last_layer = surf_tmpl->u.tex.last_layer;
        surface->offset = (rsc->slices[level].offset +
                           psurf->u.tex.first_layer * rsc->cube_map_stride);
        surface->tiling = rsc->slices[level].tiling;

        return &surface->base;
}

static void
vc4_surface_destroy(struct pipe_context *pctx, struct pipe_surface *psurf)
{
        pipe_resource_reference(&psurf->texture, NULL);
        FREE(psurf);
}

static void
vc4_dump_surface_non_msaa(struct pipe_surface *psurf)
{
        struct pipe_resource *prsc = psurf->texture;
        struct vc4_resource *rsc = vc4_resource(prsc);
        uint32_t *map = vc4_bo_map(rsc->bo);
        uint32_t stride = rsc->slices[0].stride / 4;
        uint32_t width = psurf->width;
        uint32_t height = psurf->height;
        uint32_t chunk_w = width / 79;
        uint32_t chunk_h = height / 40;
        uint32_t found_colors[10] = { 0 };
        uint32_t num_found_colors = 0;

        if (rsc->vc4_format != VC4_TEXTURE_TYPE_RGBA32R) {
                fprintf(stderr, "%s: Unsupported format %s\n",
                        __func__, util_format_short_name(psurf->format));
                return;
        }

        for (int by = 0; by < height; by += chunk_h) {
                for (int bx = 0; bx < width; bx += chunk_w) {
                        int all_found_color = -1; /* nothing found */

                        for (int y = by; y < MIN2(height, by + chunk_h); y++) {
                                for (int x = bx; x < MIN2(width, bx + chunk_w); x++) {
                                        uint32_t pix = map[y * stride + x];

                                        int i;
                                        for (i = 0; i < num_found_colors; i++) {
                                                if (pix == found_colors[i])
                                                        break;
                                        }
                                        if (i == num_found_colors &&
                                            num_found_colors <
                                            ARRAY_SIZE(found_colors)) {
                                                found_colors[num_found_colors++] = pix;
                                        }

                                        if (i < num_found_colors) {
                                                if (all_found_color == -1)
                                                        all_found_color = i;
                                                else if (i != all_found_color)
                                                        all_found_color = ARRAY_SIZE(found_colors);
                                        }
                                }
                        }
                        /* If all pixels for this chunk have a consistent
                         * value, then print a character for it.  Either a
                         * fixed name (particularly common for piglit tests),
                         * or a runtime-generated number.
                         */
                        if (all_found_color >= 0 &&
                            all_found_color < ARRAY_SIZE(found_colors)) {
                                static const struct {
                                        uint32_t val;
                                        const char *c;
                                } named_colors[] = {
                                        { 0xff000000, "█" },
                                        { 0x00000000, "█" },
                                        { 0xffff0000, "r" },
                                        { 0xff00ff00, "g" },
                                        { 0xff0000ff, "b" },
                                        { 0xffffffff, "w" },
                                };
                                int i;
                                for (i = 0; i < ARRAY_SIZE(named_colors); i++) {
                                        if (named_colors[i].val ==
                                            found_colors[all_found_color]) {
                                                fprintf(stderr, "%s",
                                                        named_colors[i].c);
                                                break;
                                        }
                                }
                                /* For unnamed colors, print a number and the
                                 * numbers will have values printed at the
                                 * end.
                                 */
                                if (i == ARRAY_SIZE(named_colors)) {
                                        fprintf(stderr, "%c",
                                                '0' + all_found_color);
                                }
                        } else {
                                /* If there's no consistent color, print this.
                                 */
                                fprintf(stderr, ".");
                        }
                }
                fprintf(stderr, "\n");
        }

        for (int i = 0; i < num_found_colors; i++) {
                fprintf(stderr, "color %d: 0x%08x\n", i, found_colors[i]);
        }
}

static uint32_t
vc4_surface_msaa_get_sample(struct pipe_surface *psurf,
                            uint32_t x, uint32_t y, uint32_t sample)
{
        struct pipe_resource *prsc = psurf->texture;
        struct vc4_resource *rsc = vc4_resource(prsc);
        uint32_t tile_w = 32, tile_h = 32;
        uint32_t tiles_w = DIV_ROUND_UP(psurf->width, 32);

        uint32_t tile_x = x / tile_w;
        uint32_t tile_y = y / tile_h;
        uint32_t *tile = (vc4_bo_map(rsc->bo) +
                          VC4_TILE_BUFFER_SIZE * (tile_y * tiles_w + tile_x));
        uint32_t subtile_x = x % tile_w;
        uint32_t subtile_y = y % tile_h;

        uint32_t quad_samples = VC4_MAX_SAMPLES * 4;
        uint32_t tile_stride = quad_samples * tile_w / 2;

        return *((uint32_t *)tile +
                 (subtile_y >> 1) * tile_stride +
                 (subtile_x >> 1) * quad_samples +
                 ((subtile_y & 1) << 1) +
                 (subtile_x & 1) +
                 sample);
}

static void
vc4_dump_surface_msaa_char(struct pipe_surface *psurf,
                           uint32_t start_x, uint32_t start_y,
                           uint32_t w, uint32_t h)
{
        bool all_same_color = true;
        uint32_t all_pix = 0;

        for (int y = start_y; y < start_y + h; y++) {
                for (int x = start_x; x < start_x + w; x++) {
                        for (int s = 0; s < VC4_MAX_SAMPLES; s++) {
                                uint32_t pix = vc4_surface_msaa_get_sample(psurf,
                                                                           x, y,
                                                                           s);
                                if (x == start_x && y == start_y)
                                        all_pix = pix;
                                else if (all_pix != pix)
                                        all_same_color = false;
                        }
                }
        }
        if (all_same_color) {
                static const struct {
                        uint32_t val;
                        const char *c;
                } named_colors[] = {
                        { 0xff000000, "█" },
                        { 0x00000000, "█" },
                        { 0xffff0000, "r" },
                        { 0xff00ff00, "g" },
                        { 0xff0000ff, "b" },
                        { 0xffffffff, "w" },
                };
                int i;
                for (i = 0; i < ARRAY_SIZE(named_colors); i++) {
                        if (named_colors[i].val == all_pix) {
                                fprintf(stderr, "%s",
                                        named_colors[i].c);
                                return;
                        }
                }
                fprintf(stderr, "x");
        } else {
                fprintf(stderr, ".");
        }
}

static void
vc4_dump_surface_msaa(struct pipe_surface *psurf)
{
        uint32_t tile_w = 32, tile_h = 32;
        uint32_t tiles_w = DIV_ROUND_UP(psurf->width, tile_w);
        uint32_t tiles_h = DIV_ROUND_UP(psurf->height, tile_h);
        uint32_t char_w = 140, char_h = 60;
        uint32_t char_w_per_tile = char_w / tiles_w - 1;
        uint32_t char_h_per_tile = char_h / tiles_h - 1;

        fprintf(stderr, "Surface: %dx%d (%dx MSAA)\n",
                psurf->width, psurf->height, psurf->texture->nr_samples);

        for (int x = 0; x < (char_w_per_tile + 1) * tiles_w; x++)
                fprintf(stderr, "-");
        fprintf(stderr, "\n");

        for (int ty = 0; ty < psurf->height; ty += tile_h) {
                for (int y = 0; y < char_h_per_tile; y++) {

                        for (int tx = 0; tx < psurf->width; tx += tile_w) {
                                for (int x = 0; x < char_w_per_tile; x++) {
                                        uint32_t bx1 = (x * tile_w /
                                                        char_w_per_tile);
                                        uint32_t bx2 = ((x + 1) * tile_w /
                                                        char_w_per_tile);
                                        uint32_t by1 = (y * tile_h /
                                                        char_h_per_tile);
                                        uint32_t by2 = ((y + 1) * tile_h /
                                                        char_h_per_tile);

                                        vc4_dump_surface_msaa_char(psurf,
                                                                   tx + bx1,
                                                                   ty + by1,
                                                                   bx2 - bx1,
                                                                   by2 - by1);
                                }
                                fprintf(stderr, "|");
                        }
                        fprintf(stderr, "\n");
                }

                for (int x = 0; x < (char_w_per_tile + 1) * tiles_w; x++)
                        fprintf(stderr, "-");
                fprintf(stderr, "\n");
        }
}

/** Debug routine to dump the contents of an 8888 surface to the console */
void
vc4_dump_surface(struct pipe_surface *psurf)
{
        if (!psurf)
                return;

        if (psurf->texture->nr_samples > 1)
                vc4_dump_surface_msaa(psurf);
        else
                vc4_dump_surface_non_msaa(psurf);
}

static void
vc4_flush_resource(struct pipe_context *pctx, struct pipe_resource *resource)
{
        /* All calls to flush_resource are followed by a flush of the context,
         * so there's nothing to do.
         */
}

void
vc4_update_shadow_baselevel_texture(struct pipe_context *pctx,
                                    struct pipe_sampler_view *pview)
{
        struct vc4_context *vc4 = vc4_context(pctx);
        struct vc4_sampler_view *view = vc4_sampler_view(pview);
        struct vc4_resource *shadow = vc4_resource(view->texture);
        struct vc4_resource *orig = vc4_resource(pview->texture);

        assert(view->texture != pview->texture);

        if (shadow->writes == orig->writes && orig->bo->private)
                return;

        perf_debug("Updating %dx%d@%d shadow texture due to %s\n",
                   orig->base.width0, orig->base.height0,
                   pview->u.tex.first_level,
                   pview->u.tex.first_level ? "base level" : "raster layout");

        for (int i = 0; i <= shadow->base.last_level; i++) {
                unsigned width = u_minify(shadow->base.width0, i);
                unsigned height = u_minify(shadow->base.height0, i);
                struct pipe_blit_info info = {
                        .dst = {
                                .resource = &shadow->base,
                                .level = i,
                                .box = {
                                        .x = 0,
                                        .y = 0,
                                        .z = 0,
                                        .width = width,
                                        .height = height,
                                        .depth = 1,
                                },
                                .format = shadow->base.format,
                        },
                        .src = {
                                .resource = &orig->base,
                                .level = pview->u.tex.first_level + i,
                                .box = {
                                        .x = 0,
                                        .y = 0,
                                        .z = 0,
                                        .width = width,
                                        .height = height,
                                        .depth = 1,
                                },
                                .format = orig->base.format,
                        },
                        .mask = util_format_get_mask(orig->base.format),
                };
                pctx->blit(pctx, &info);
        }

        shadow->writes = orig->writes;
}

/**
 * Converts a 4-byte index buffer to 2 bytes.
 *
 * Since GLES2 only has support for 1 and 2-byte indices, the hardware doesn't
 * include 4-byte index support, and we have to shrink it down.
 *
 * There's no fallback support for when indices end up being larger than 2^16,
 * though it will at least assertion fail.  Also, if the original index data
 * was in user memory, it would be nice to not have uploaded it to a VBO
 * before translating.
 */
struct pipe_resource *
vc4_get_shadow_index_buffer(struct pipe_context *pctx,
                            const struct pipe_draw_info *info,
                            uint32_t offset,
                            uint32_t count,
                            uint32_t *shadow_offset)
{
        struct vc4_context *vc4 = vc4_context(pctx);
        struct vc4_resource *orig = vc4_resource(info->index.resource);
        perf_debug("Fallback conversion for %d uint indices\n", count);

        void *data;
        struct pipe_resource *shadow_rsc = NULL;
        u_upload_alloc(vc4->uploader, 0, count * 2, 4,
                       shadow_offset, &shadow_rsc, &data);
        uint16_t *dst = data;

        struct pipe_transfer *src_transfer = NULL;
        const uint32_t *src;
        if (info->has_user_indices) {
                src = (uint32_t*)((char*)info->index.user + offset);
        } else {
                src = pipe_buffer_map_range(pctx, &orig->base,
                                            offset,
                                            count * 4,
                                            PIPE_MAP_READ, &src_transfer);
        }

        for (int i = 0; i < count; i++) {
                uint32_t src_index = src[i];
                assert(src_index <= 0xffff);
                dst[i] = src_index;
        }

        if (src_transfer)
                pctx->buffer_unmap(pctx, src_transfer);

        return shadow_rsc;
}

static const struct u_transfer_vtbl transfer_vtbl = {
        .resource_create          = vc4_resource_create,
        .resource_destroy         = vc4_resource_destroy,
        .transfer_map             = vc4_resource_transfer_map,
        .transfer_unmap           = vc4_resource_transfer_unmap,
        .transfer_flush_region    = u_default_transfer_flush_region,
};

void
vc4_resource_screen_init(struct pipe_screen *pscreen)
{
        struct vc4_screen *screen = vc4_screen(pscreen);

        pscreen->resource_create = vc4_resource_create;
        pscreen->resource_create_with_modifiers =
                vc4_resource_create_with_modifiers;
        pscreen->resource_from_handle = vc4_resource_from_handle;
        pscreen->resource_get_handle = vc4_resource_get_handle;
        pscreen->resource_get_param = vc4_resource_get_param;
        pscreen->resource_destroy = vc4_resource_destroy;
        pscreen->transfer_helper = u_transfer_helper_create(&transfer_vtbl,
                                                            U_TRANSFER_HELPER_MSAA_MAP);

        /* Test if the kernel has GET_TILING; it will return -EINVAL if the
         * ioctl does not exist, but -ENOENT if we pass an impossible handle.
         * 0 cannot be a valid GEM object, so use that.
         */
        struct drm_vc4_get_tiling get_tiling = {
                .handle = 0x0,
        };
        int ret = vc4_ioctl(screen->fd, DRM_IOCTL_VC4_GET_TILING, &get_tiling);
        if (ret == -1 && errno == ENOENT)
                screen->has_tiling_ioctl = true;
}

void
vc4_resource_context_init(struct pipe_context *pctx)
{
        pctx->buffer_map = u_transfer_helper_transfer_map;
        pctx->texture_map = u_transfer_helper_transfer_map;
        pctx->transfer_flush_region = u_transfer_helper_transfer_flush_region;
        pctx->buffer_unmap = u_transfer_helper_transfer_unmap;
        pctx->texture_unmap = u_transfer_helper_transfer_unmap;
        pctx->buffer_subdata = u_default_buffer_subdata;
        pctx->texture_subdata = vc4_texture_subdata;
        pctx->create_surface = vc4_create_surface;
        pctx->surface_destroy = vc4_surface_destroy;
        pctx->resource_copy_region = util_resource_copy_region;
        pctx->blit = vc4_blit;
        pctx->flush_resource = vc4_flush_resource;
}
