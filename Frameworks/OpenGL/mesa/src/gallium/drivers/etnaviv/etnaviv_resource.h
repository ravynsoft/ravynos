/*
 * Copyright (c) 2012-2015 Etnaviv Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Wladimir J. van der Laan <laanwj@gmail.com>
 */

#ifndef H_ETNAVIV_RESOURCE
#define H_ETNAVIV_RESOURCE

#include "etnaviv_internal.h"
#include "etnaviv_tiling.h"
#include "pipe/p_state.h"
#include "util/format/u_format.h"
#include "util/list.h"
#include "util/hash_table.h"
#include "util/u_helpers.h"
#include "util/u_range.h"

#include "drm-uapi/drm_fourcc.h"

struct etna_context;
struct pipe_screen;
struct util_dynarray;

struct etna_ts_sw_meta {
   uint16_t version;
   struct {
      uint16_t data_offset;
      uint32_t data_size;
      uint32_t layer_stride;
      uint32_t comp_format;
      uint64_t clear_value;
      uint32_t seqno;
      uint32_t flush_seqno;
      uint8_t valid;
      uint8_t pad[3];
   } v0;
};

struct etna_resource_level {
   unsigned width, height; /* in pixels */
   unsigned padded_width, padded_height; /* in samples */
   unsigned depth;
   unsigned offset; /* offset into memory area */
   uint32_t stride; /* row stride */
   uint32_t layer_stride; /* layer stride */
   unsigned size; /* total size of memory area */

   uint32_t ts_offset;
   uint32_t ts_layer_stride;
   uint32_t ts_size;
   uint64_t clear_value; /* clear value of resource level (mainly for TS) */
   bool ts_valid;
   uint8_t ts_mode;
   int8_t ts_compress_fmt; /* COLOR_COMPRESSION_FORMAT_* (-1 = disable) */

   struct etna_ts_sw_meta *ts_meta; /* metadata for shared TS */

   /* keep track if we have done some per block patching */
   bool patched;
   struct util_dynarray *patch_offsets;

   uint32_t seqno;
   uint32_t flush_seqno;
};

/* returns TRUE if a is newer than b */
static inline bool
etna_resource_level_newer(struct etna_resource_level *a,
                          struct etna_resource_level *b)
{
   uint32_t a_seqno = a->ts_meta ? a->ts_meta->v0.seqno : a->seqno;
   uint32_t b_seqno = b->ts_meta ? b->ts_meta->v0.seqno : b->seqno;

   return (int)(a_seqno - b_seqno) > 0;
}

/* returns TRUE if a is older than b */
static inline bool
etna_resource_level_older(struct etna_resource_level *a,
                          struct etna_resource_level *b)
{
   uint32_t a_seqno = a->ts_meta ? a->ts_meta->v0.seqno : a->seqno;
   uint32_t b_seqno = b->ts_meta ? b->ts_meta->v0.seqno : b->seqno;

   return (int)(a_seqno - b_seqno) < 0;
}

static inline bool
etna_resource_level_ts_valid(struct etna_resource_level *lvl)
{
   if (unlikely(lvl->ts_meta))
      return lvl->ts_meta->v0.valid;
   else
      return lvl->ts_valid;
}

static inline void
etna_resource_level_ts_mark_valid(struct etna_resource_level *lvl)
{
   if (unlikely(lvl->ts_meta))
      lvl->ts_meta->v0.valid = 1;
   else
      lvl->ts_valid = true;
}

static inline void
etna_resource_level_ts_mark_invalid(struct etna_resource_level *lvl)
{
   if (unlikely(lvl->ts_meta))
      lvl->ts_meta->v0.valid = 0;
   else
      lvl->ts_valid = false;
}

/* returns TRUE if a is older than b */
static inline bool
etna_resource_level_needs_flush(struct etna_resource_level *lvl)
{
   if (!etna_resource_level_ts_valid(lvl))
      return false;

   if (unlikely(lvl->ts_meta))
      return ((int)(lvl->ts_meta->v0.seqno - lvl->ts_meta->v0.flush_seqno) > 0);
   else
      return ((int)(lvl->seqno - lvl->flush_seqno) > 0);
}

static inline void
etna_resource_level_mark_flushed(struct etna_resource_level *lvl)
{
   if (unlikely(lvl->ts_meta))
      lvl->ts_meta->v0.flush_seqno = lvl->ts_meta->v0.seqno;
   else
      lvl->flush_seqno = lvl->seqno;
}

static inline void
etna_resource_level_mark_changed(struct etna_resource_level *lvl)
{
   if (unlikely(lvl->ts_meta))
      lvl->ts_meta->v0.seqno++;
   else
      lvl->seqno++;
}

static inline void
etna_resource_level_copy_seqno(struct etna_resource_level *dst,
                               struct etna_resource_level *src)
{
   uint32_t src_seqno = src->ts_meta ? src->ts_meta->v0.seqno : src->seqno;

   if (unlikely(dst->ts_meta))
      dst->ts_meta->v0.seqno = src_seqno;
   else
      dst->seqno = src_seqno;
}

/* status of queued up but not flushed reads and write operations.
 * In _transfer_map() we need to know if queued up rendering needs
 * to be flushed to preserve the order of cpu and gpu access. */
enum etna_resource_status {
   ETNA_PENDING_WRITE = 0x01,
   ETNA_PENDING_READ = 0x02,
};

struct etna_resource {
   struct pipe_resource base;
   struct renderonly_scanout *scanout;

   /* only lod 0 used for non-texture buffers */
   /* Layout for surface (tiled, multitiled, split tiled, ...) */
   enum etna_surface_layout layout;
   uint64_t modifier;
   /* Horizontal alignment for texture unit (TEXTURE_HALIGN_*) */
   unsigned halign;
   struct etna_bo *bo; /* Surface video memory */
   struct etna_bo *ts_bo; /* Tile status video memory */
   struct renderonly_scanout *ts_scanout; /* display compatible TS */

   struct etna_resource_level levels[ETNA_NUM_LOD];

   /* buffer range that has been initialized */
   struct util_range valid_buffer_range;

   /* for when TE doesn't support the base layout */
   struct pipe_resource *texture;
   /* for when PE doesn't support the base layout */
   struct pipe_resource *render;
   /* frontend flushes resource via an explicit call to flush_resource */
   bool explicit_flush;
};

/* returns TRUE if a is newer than b */
static inline bool
etna_resource_newer(struct etna_resource *a, struct etna_resource *b)
{
   assert(a->base.last_level == b->base.last_level);

   for (int level = 0; level <= a->base.last_level; level++)
      if (etna_resource_level_newer(&a->levels[level], &b->levels[level]))
         return true;

   return false;
}

/* returns TRUE if a is older than b */
static inline bool
etna_resource_older(struct etna_resource *a, struct etna_resource *b)
{
   assert(a->base.last_level == b->base.last_level);

   for (int level = 0; level <= a->base.last_level; level++)
      if (etna_resource_level_older(&a->levels[level], &b->levels[level]))
         return true;

   return false;
}

/* returns TRUE if the resource needs a resolve to itself */
bool etna_resource_needs_flush(struct etna_resource *res);

/* is the resource only used on the sampler? */
static inline bool
etna_resource_sampler_only(const struct pipe_resource *pres)
{
   return (pres->bind & (PIPE_BIND_SAMPLER_VIEW | PIPE_BIND_RENDER_TARGET |
                         PIPE_BIND_DEPTH_STENCIL | PIPE_BIND_BLENDABLE)) ==
          PIPE_BIND_SAMPLER_VIEW;
}

static inline bool
etna_resource_hw_tileable(bool use_blt, const struct pipe_resource *pres)
{
   if (use_blt)
      return true;

   /* RS can only tile 16bpp or 32bpp formats */
   return util_format_get_blocksize(pres->format) == 2 ||
          util_format_get_blocksize(pres->format) == 4;
}

/* returns TRUE if resource TS buffer is exposed externally */
static inline bool
etna_resource_ext_ts(const struct etna_resource *res)
{
   return res->modifier & VIVANTE_MOD_TS_MASK;
}

static inline struct etna_resource *
etna_resource(struct pipe_resource *p)
{
   return (struct etna_resource *)p;
}

void
etna_resource_used(struct etna_context *ctx, struct pipe_resource *prsc,
                   enum etna_resource_status status);

static inline void
resource_read(struct etna_context *ctx, struct pipe_resource *prsc)
{
   etna_resource_used(ctx, prsc, ETNA_PENDING_READ);
}

static inline void
resource_written(struct etna_context *ctx, struct pipe_resource *prsc)
{
   etna_resource_used(ctx, prsc, ETNA_PENDING_WRITE);
}

enum etna_resource_status
etna_resource_status(struct etna_context *ctx, struct etna_resource *res);

/* Allocate Tile Status for an etna resource.
 * Tile status is a cache of the clear status per tile. This means a smaller
 * surface has to be cleared which is faster.
 * This is also called "fast clear". */
bool
etna_screen_resource_alloc_ts(struct pipe_screen *pscreen,
                              struct etna_resource *prsc,
                              uint64_t modifier);

struct pipe_resource *
etna_resource_alloc(struct pipe_screen *pscreen, unsigned layout,
                    uint64_t modifier, const struct pipe_resource *templat);

void
etna_resource_screen_init(struct pipe_screen *pscreen);

#endif
