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

#ifndef VC4_RESOURCE_H
#define VC4_RESOURCE_H

#include "vc4_screen.h"
#include "kernel/vc4_packet.h"
#include "util/u_transfer.h"

struct vc4_transfer {
        struct pipe_transfer base;
        void *map;
};

struct vc4_resource_slice {
        uint32_t offset;
        uint32_t stride;
        uint32_t size;
        /** One of VC4_TILING_FORMAT_* */
        uint8_t tiling;
};

struct vc4_surface {
        struct pipe_surface base;
        uint32_t offset;
        uint8_t tiling;
};

struct vc4_resource {
        struct pipe_resource base;
        struct vc4_bo *bo;
        struct renderonly_scanout *scanout;
        struct vc4_resource_slice slices[VC4_MAX_MIP_LEVELS];
        uint32_t cube_map_stride;
        int cpp;
        bool tiled;
        /** One of VC4_TEXTURE_TYPE_* */
        enum vc4_texture_data_type vc4_format;

        /**
         * Number of times the resource has been written to.
         *
         * This is used to track when we need to update this shadow resource
         * from its parent in the case of GL_TEXTURE_BASE_LEVEL (which we
         * can't support in hardware) or GL_UNSIGNED_INTEGER index buffers.
         */
        uint64_t writes;

        /**
         * Bitmask of PIPE_CLEAR_COLOR0, PIPE_CLEAR_DEPTH, PIPE_CLEAR_STENCIL
         * for which parts of the resource are defined.
         *
         * Used for avoiding fallback to quad clears for clearing just depth,
         * when the stencil contents have never been initialized.  Note that
         * we're lazy and fields not present in the buffer (DEPTH in a color
         * buffer) may get marked.
         */
        uint32_t initialized_buffers;
};

static inline struct vc4_resource *
vc4_resource(struct pipe_resource *prsc)
{
        return (struct vc4_resource *)prsc;
}

static inline struct vc4_surface *
vc4_surface(struct pipe_surface *psurf)
{
        return (struct vc4_surface *)psurf;
}

static inline struct vc4_transfer *
vc4_transfer(struct pipe_transfer *ptrans)
{
        return (struct vc4_transfer *)ptrans;
}

void vc4_resource_screen_init(struct pipe_screen *pscreen);
void vc4_resource_context_init(struct pipe_context *pctx);
struct pipe_resource *vc4_resource_create(struct pipe_screen *pscreen,
                                          const struct pipe_resource *tmpl);
void vc4_update_shadow_baselevel_texture(struct pipe_context *pctx,
                                         struct pipe_sampler_view *view);
struct pipe_resource *vc4_get_shadow_index_buffer(struct pipe_context *pctx,
                                                  const struct pipe_draw_info *info,
                                                  uint32_t offset,
                                                  uint32_t count,
                                                  uint32_t *shadow_offset);
void vc4_dump_surface(struct pipe_surface *psurf);

#endif /* VC4_RESOURCE_H */
