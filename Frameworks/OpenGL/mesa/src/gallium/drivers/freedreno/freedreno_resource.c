/*
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *    Rob Clark <robclark@freedesktop.org>
 */

#include "util/format/u_format.h"
#include "util/format/u_format_rgtc.h"
#include "util/format/u_format_zs.h"
#include "util/set.h"
#include "util/u_drm.h"
#include "util/u_inlines.h"
#include "util/u_sample_positions.h"
#include "util/u_string.h"
#include "util/u_surface.h"
#include "util/u_transfer.h"

#include "decode/util.h"

#include "freedreno_batch_cache.h"
#include "freedreno_blitter.h"
#include "freedreno_context.h"
#include "freedreno_fence.h"
#include "freedreno_query_hw.h"
#include "freedreno_resource.h"
#include "freedreno_screen.h"
#include "freedreno_surface.h"
#include "freedreno_util.h"

#include <errno.h>
#include "drm-uapi/drm_fourcc.h"

/* XXX this should go away, needed for 'struct winsys_handle' */
#include "frontend/drm_driver.h"

/**
 * Go through the entire state and see if the resource is bound
 * anywhere. If it is, mark the relevant state as dirty. This is
 * called on realloc_bo to ensure the necessary state is re-
 * emitted so the GPU looks at the new backing bo.
 */
static void
rebind_resource_in_ctx(struct fd_context *ctx,
                       struct fd_resource *rsc) assert_dt
{
   struct pipe_resource *prsc = &rsc->b.b;

   if (ctx->rebind_resource)
      ctx->rebind_resource(ctx, rsc);

   /* VBOs */
   if (rsc->dirty & FD_DIRTY_VTXBUF) {
      struct fd_vertexbuf_stateobj *vb = &ctx->vtx.vertexbuf;
      for (unsigned i = 0; i < vb->count && !(ctx->dirty & FD_DIRTY_VTXBUF);
           i++) {
         if (vb->vb[i].buffer.resource == prsc)
            fd_dirty_resource(ctx, prsc, FD_DIRTY_VTXBUF, false);
      }
   }

   /* xfb/so buffers: */
   if (rsc->dirty & FD_DIRTY_STREAMOUT) {
      struct fd_streamout_stateobj *so = &ctx->streamout;

      for (unsigned i = 0;
            i < so->num_targets && !(ctx->dirty & FD_DIRTY_STREAMOUT);
            i++) {
         if (so->targets[i]->buffer == prsc)
            fd_dirty_resource(ctx, prsc, FD_DIRTY_STREAMOUT, true);
      }
   }

   const enum fd_dirty_3d_state per_stage_dirty =
      FD_DIRTY_CONST | FD_DIRTY_TEX | FD_DIRTY_IMAGE | FD_DIRTY_SSBO;

   if (!(rsc->dirty & per_stage_dirty))
      return;

   /* per-shader-stage resources: */
   for (unsigned stage = 0; stage < PIPE_SHADER_TYPES; stage++) {
      /* Constbufs.. note that constbuf[0] is normal uniforms emitted in
       * cmdstream rather than by pointer..
       */
      if ((rsc->dirty & FD_DIRTY_CONST) &&
          !(ctx->dirty_shader[stage] & FD_DIRTY_CONST)) {
         struct fd_constbuf_stateobj *cb = &ctx->constbuf[stage];
         const unsigned num_ubos = util_last_bit(cb->enabled_mask);
         for (unsigned i = 1; i < num_ubos; i++) {
            if (cb->cb[i].buffer == prsc) {
               fd_dirty_shader_resource(ctx, prsc, stage,
                                        FD_DIRTY_SHADER_CONST, false);
               break;
            }
         }
      }

      /* Textures */
      if ((rsc->dirty & FD_DIRTY_TEX) &&
          !(ctx->dirty_shader[stage] & FD_DIRTY_TEX)) {
         struct fd_texture_stateobj *tex = &ctx->tex[stage];
         for (unsigned i = 0; i < tex->num_textures; i++) {
            if (tex->textures[i] && (tex->textures[i]->texture == prsc)) {
               fd_dirty_shader_resource(ctx, prsc, stage,
                                        FD_DIRTY_SHADER_TEX, false);
               break;
            }
         }
      }

      /* Images */
      if ((rsc->dirty & FD_DIRTY_IMAGE) &&
          !(ctx->dirty_shader[stage] & FD_DIRTY_IMAGE)) {
         struct fd_shaderimg_stateobj *si = &ctx->shaderimg[stage];
         const unsigned num_images = util_last_bit(si->enabled_mask);
         for (unsigned i = 0; i < num_images; i++) {
            if (si->si[i].resource == prsc) {
               bool write = si->si[i].access & PIPE_IMAGE_ACCESS_WRITE;
               fd_dirty_shader_resource(ctx, prsc, stage,
                                        FD_DIRTY_SHADER_IMAGE, write);
               break;
            }
         }
      }

      /* SSBOs */
      if ((rsc->dirty & FD_DIRTY_SSBO) &&
          !(ctx->dirty_shader[stage] & FD_DIRTY_SSBO)) {
         struct fd_shaderbuf_stateobj *sb = &ctx->shaderbuf[stage];
         const unsigned num_ssbos = util_last_bit(sb->enabled_mask);
         for (unsigned i = 0; i < num_ssbos; i++) {
            if (sb->sb[i].buffer == prsc) {
               bool write = sb->writable_mask & BIT(i);
               fd_dirty_shader_resource(ctx, prsc, stage,
                                        FD_DIRTY_SHADER_SSBO, write);
               break;
            }
         }
      }
   }
}

static void
rebind_resource(struct fd_resource *rsc) assert_dt
{
   struct fd_screen *screen = fd_screen(rsc->b.b.screen);

   fd_screen_lock(screen);
   fd_resource_lock(rsc);

   if (rsc->dirty)
      list_for_each_entry (struct fd_context, ctx, &screen->context_list, node)
         rebind_resource_in_ctx(ctx, rsc);

   fd_resource_unlock(rsc);
   fd_screen_unlock(screen);
}

static inline void
fd_resource_set_bo(struct fd_resource *rsc, struct fd_bo *bo)
{
   struct fd_screen *screen = fd_screen(rsc->b.b.screen);

   rsc->bo = bo;
   rsc->seqno = seqno_next_u16(&screen->rsc_seqno);
}

int
__fd_resource_wait(struct fd_context *ctx, struct fd_resource *rsc, unsigned op,
                   const char *func)
{
   if (op & FD_BO_PREP_NOSYNC)
      return fd_bo_cpu_prep(rsc->bo, ctx->pipe, op);

   int ret;

   perf_time_ctx (ctx, 10000, "%s: a busy \"%" PRSC_FMT "\" BO stalled", func,
                  PRSC_ARGS(&rsc->b.b)) {
      ret = fd_bo_cpu_prep(rsc->bo, ctx->pipe, op);
   }

   return ret;
}

static void
realloc_bo(struct fd_resource *rsc, uint32_t size)
{
   struct pipe_resource *prsc = &rsc->b.b;
   struct fd_screen *screen = fd_screen(rsc->b.b.screen);
   uint32_t flags =
      COND(rsc->layout.tile_mode, FD_BO_NOMAP) |
      COND((prsc->usage & PIPE_USAGE_STAGING) &&
           (prsc->flags & PIPE_RESOURCE_FLAG_MAP_COHERENT),
           FD_BO_CACHED_COHERENT) |
      COND(prsc->bind & PIPE_BIND_SHARED, FD_BO_SHARED) |
      COND(prsc->bind & PIPE_BIND_SCANOUT, FD_BO_SCANOUT);
   /* TODO other flags? */

   /* if we start using things other than write-combine,
    * be sure to check for PIPE_RESOURCE_FLAG_MAP_COHERENT
    */

   if (rsc->bo)
      fd_bo_del(rsc->bo);

   struct fd_bo *bo =
      fd_bo_new(screen->dev, size, flags, "%ux%ux%u@%u:%x", prsc->width0,
                prsc->height0, prsc->depth0, rsc->layout.cpp, prsc->bind);
   fd_resource_set_bo(rsc, bo);

   /* Zero out the UBWC area on allocation.  This fixes intermittent failures
    * with UBWC, which I suspect are due to the HW having a hard time
    * interpreting arbitrary values populating the flags buffer when the BO
    * was recycled through the bo cache (instead of fresh allocations from
    * the kernel, which are zeroed).  sleep(1) in this spot didn't work
    * around the issue, but any memset value seems to.
    */
   if (rsc->layout.ubwc) {
      rsc->needs_ubwc_clear = true;
   }

   util_range_set_empty(&rsc->valid_buffer_range);
   fd_bc_invalidate_resource(rsc, true);
}

static void
do_blit(struct fd_context *ctx, const struct pipe_blit_info *blit,
        bool fallback) assert_dt
{
   struct pipe_context *pctx = &ctx->base;

   assert(!ctx->in_blit);
   ctx->in_blit = true;

   /* TODO size threshold too?? */
   if (fallback || !fd_blit(pctx, blit)) {
      /* do blit on cpu: */
      util_resource_copy_region(pctx, blit->dst.resource, blit->dst.level,
                                blit->dst.box.x, blit->dst.box.y,
                                blit->dst.box.z, blit->src.resource,
                                blit->src.level, &blit->src.box);
   }

   ctx->in_blit = false;
}

/**
 * Replace the storage of dst with src.  This is only used by TC in the
 * DISCARD_WHOLE_RESOURCE path, and src is a freshly allocated buffer.
 */
void
fd_replace_buffer_storage(struct pipe_context *pctx, struct pipe_resource *pdst,
                          struct pipe_resource *psrc, unsigned num_rebinds, uint32_t rebind_mask,
                          uint32_t delete_buffer_id)
{
   struct fd_context *ctx = fd_context(pctx);
   struct fd_resource *dst = fd_resource(pdst);
   struct fd_resource *src = fd_resource(psrc);

   DBG("pdst=%p, psrc=%p", pdst, psrc);

   /* This should only be called with buffers.. which side-steps some tricker
    * cases, like a rsc that is in a batch-cache key...
    */
   assert(pdst->target == PIPE_BUFFER);
   assert(psrc->target == PIPE_BUFFER);
   assert(dst->track->bc_batch_mask == 0);
   assert(src->track->bc_batch_mask == 0);
   assert(src->track->batch_mask == 0);
   assert(src->track->write_batch == NULL);
   assert(memcmp(&dst->layout, &src->layout, sizeof(dst->layout)) == 0);

   /* get rid of any references that batch-cache might have to us (which
    * should empty/destroy rsc->batches hashset)
    *
    * Note that we aren't actually destroying dst, but we are replacing
    * it's storage so we want to go thru the same motions of decoupling
    * it's batch connections.
    */
   fd_bc_invalidate_resource(dst, true);
   rebind_resource(dst);

   util_idalloc_mt_free(&ctx->screen->buffer_ids, delete_buffer_id);

   fd_screen_lock(ctx->screen);

   fd_bo_del(dst->bo);
   dst->bo = fd_bo_ref(src->bo);

   fd_resource_tracking_reference(&dst->track, src->track);
   src->is_replacement = true;

   dst->seqno = seqno_next_u16(&ctx->screen->rsc_seqno);

   fd_screen_unlock(ctx->screen);
}

static unsigned
translate_usage(unsigned usage)
{
   uint32_t op = 0;

   if (usage & PIPE_MAP_READ)
      op |= FD_BO_PREP_READ;

   if (usage & PIPE_MAP_WRITE)
      op |= FD_BO_PREP_WRITE;

   return op;
}

bool
fd_resource_busy(struct pipe_screen *pscreen, struct pipe_resource *prsc,
                 unsigned usage)
{
   struct fd_resource *rsc = fd_resource(prsc);

   if (pending(rsc, !!(usage & PIPE_MAP_WRITE)))
      return true;

   if (resource_busy(rsc, translate_usage(usage)))
      return true;

   return false;
}

static void flush_resource(struct fd_context *ctx, struct fd_resource *rsc,
                           unsigned usage);

/**
 * Helper to check if the format is something that we can blit/render
 * to.. if the format is not renderable, there is no point in trying
 * to do a staging blit (as it will still end up being a cpu copy)
 */
static bool
is_renderable(struct pipe_resource *prsc)
{
   struct pipe_screen *pscreen = prsc->screen;
   return pscreen->is_format_supported(
         pscreen, prsc->format, prsc->target, prsc->nr_samples,
         prsc->nr_storage_samples, PIPE_BIND_RENDER_TARGET);
}

/**
 * @rsc: the resource to shadow
 * @level: the level to discard (if box != NULL, otherwise ignored)
 * @box: the box to discard (or NULL if none)
 * @modifier: the modifier for the new buffer state
 */
static bool
fd_try_shadow_resource(struct fd_context *ctx, struct fd_resource *rsc,
                       unsigned level, const struct pipe_box *box,
                       uint64_t modifier) assert_dt
{
   struct pipe_context *pctx = &ctx->base;
   struct pipe_resource *prsc = &rsc->b.b;
   struct fd_screen *screen = fd_screen(pctx->screen);
   struct fd_batch *batch;
   bool fallback = false;

   if (prsc->next)
      return false;

   /* Flush any pending batches writing the resource before we go mucking around
    * in its insides.  The blit would immediately cause the batch to be flushed,
    * anyway.
    */
   fd_bc_flush_writer(ctx, rsc);

   /* Because IB1 ("gmem") cmdstream is built only when we flush the
    * batch, we need to flush any batches that reference this rsc as
    * a render target.  Otherwise the framebuffer state emitted in
    * IB1 will reference the resources new state, and not the state
    * at the point in time that the earlier draws referenced it.
    *
    * Note that being in the gmem key doesn't necessarily mean the
    * batch was considered a writer!
    */
   foreach_batch (batch, &screen->batch_cache, rsc->track->bc_batch_mask) {
      fd_batch_flush(batch);
   }

   /* TODO: somehow munge dimensions and format to copy unsupported
    * render target format to something that is supported?
    */
   if (!is_renderable(prsc))
      fallback = true;

   /* do shadowing back-blits on the cpu for buffers -- requires about a page of
    * DMA to make GPU copies worth it according to robclark.  Note, if you
    * decide to do it on the GPU then you'll need to update valid_buffer_range
    * in the swap()s below.
    */
   if (prsc->target == PIPE_BUFFER)
      fallback = true;

   bool discard_whole_level = box && util_texrange_covers_whole_level(
                                        prsc, level, box->x, box->y, box->z,
                                        box->width, box->height, box->depth);

   /* TODO need to be more clever about current level */
   if ((prsc->target >= PIPE_TEXTURE_2D) && box && !discard_whole_level)
      return false;

   struct pipe_resource *pshadow = pctx->screen->resource_create_with_modifiers(
      pctx->screen, prsc, &modifier, 1);

   if (!pshadow)
      return false;

   assert(!ctx->in_shadow);
   ctx->in_shadow = true;

   /* get rid of any references that batch-cache might have to us (which
    * should empty/destroy rsc->batches hashset)
    */
   fd_bc_invalidate_resource(rsc, false);
   rebind_resource(rsc);

   fd_screen_lock(ctx->screen);

   /* Swap the backing bo's, so shadow becomes the old buffer,
    * blit from shadow to new buffer.  From here on out, we
    * cannot fail.
    *
    * Note that we need to do it in this order, otherwise if
    * we go down cpu blit path, the recursive transfer_map()
    * sees the wrong status..
    */
   struct fd_resource *shadow = fd_resource(pshadow);

   DBG("shadow: %p (%d, %p) -> %p (%d, %p)", rsc, rsc->b.b.reference.count,
       rsc->track, shadow, shadow->b.b.reference.count, shadow->track);

   SWAP(rsc->bo, shadow->bo);
   SWAP(rsc->valid, shadow->valid);

   /* swap() doesn't work because you can't typeof() the bitfield. */
   bool temp = shadow->needs_ubwc_clear;
   shadow->needs_ubwc_clear = rsc->needs_ubwc_clear;
   rsc->needs_ubwc_clear = temp;

   SWAP(rsc->layout, shadow->layout);
   rsc->seqno = seqno_next_u16(&ctx->screen->rsc_seqno);

   /* at this point, the newly created shadow buffer is not referenced
    * by any batches, but the existing rsc (probably) is.  We need to
    * transfer those references over:
    */
   assert(shadow->track->batch_mask == 0);
   foreach_batch (batch, &ctx->screen->batch_cache, rsc->track->batch_mask) {
      struct set_entry *entry = _mesa_set_search_pre_hashed(batch->resources, rsc->hash, rsc);
      _mesa_set_remove(batch->resources, entry);
      _mesa_set_add_pre_hashed(batch->resources, shadow->hash, shadow);
   }
   SWAP(rsc->track, shadow->track);

   fd_screen_unlock(ctx->screen);

   struct pipe_blit_info blit = {};
   blit.dst.resource = prsc;
   blit.dst.format = prsc->format;
   blit.src.resource = pshadow;
   blit.src.format = pshadow->format;
   blit.mask = util_format_get_mask(prsc->format);
   blit.filter = PIPE_TEX_FILTER_NEAREST;

#define set_box(field, val)                                                    \
   do {                                                                        \
      blit.dst.field = (val);                                                  \
      blit.src.field = (val);                                                  \
   } while (0)

   /* Disable occlusion queries during shadow blits. */
   bool saved_active_queries = ctx->active_queries;
   pctx->set_active_query_state(pctx, false);

   /* blit the other levels in their entirety: */
   for (unsigned l = 0; l <= prsc->last_level; l++) {
      if (box && l == level)
         continue;

      /* just blit whole level: */
      set_box(level, l);
      set_box(box.width, u_minify(prsc->width0, l));
      set_box(box.height, u_minify(prsc->height0, l));
      set_box(box.depth, u_minify(prsc->depth0, l));

      for (int i = 0; i < prsc->array_size; i++) {
         set_box(box.z, i);
         do_blit(ctx, &blit, fallback);
      }
   }

   /* deal w/ current level specially, since we might need to split
    * it up into a couple blits:
    */
   if (box && !discard_whole_level) {
      set_box(level, level);

      switch (prsc->target) {
      case PIPE_BUFFER:
      case PIPE_TEXTURE_1D:
         set_box(box.y, 0);
         set_box(box.z, 0);
         set_box(box.height, 1);
         set_box(box.depth, 1);

         if (box->x > 0) {
            set_box(box.x, 0);
            set_box(box.width, box->x);

            do_blit(ctx, &blit, fallback);
         }
         if ((box->x + box->width) < u_minify(prsc->width0, level)) {
            set_box(box.x, box->x + box->width);
            set_box(box.width,
                    u_minify(prsc->width0, level) - (box->x + box->width));

            do_blit(ctx, &blit, fallback);
         }
         break;
      case PIPE_TEXTURE_2D:
         /* TODO */
      default:
         unreachable("TODO");
      }
   }

   pctx->set_active_query_state(pctx, saved_active_queries);

   ctx->in_shadow = false;

   pipe_resource_reference(&pshadow, NULL);

   return true;
}

/**
 * Uncompress an UBWC compressed buffer "in place".  This works basically
 * like resource shadowing, creating a new resource, and doing an uncompress
 * blit, and swapping the state between shadow and original resource so it
 * appears to the gallium frontends as if nothing changed.
 */
void
fd_resource_uncompress(struct fd_context *ctx, struct fd_resource *rsc, bool linear)
{
   tc_assert_driver_thread(ctx->tc);

   uint64_t modifier = linear ? DRM_FORMAT_MOD_LINEAR : DRM_FORMAT_MOD_QCOM_TILED3;

   ASSERTED bool success = fd_try_shadow_resource(ctx, rsc, 0, NULL, modifier);

   /* shadow should not fail in any cases where we need to uncompress: */
   assert(success);
}

/**
 * Debug helper to hexdump a resource.
 */
void
fd_resource_dump(struct fd_resource *rsc, const char *name)
{
   fd_bo_cpu_prep(rsc->bo, NULL, FD_BO_PREP_READ);
   printf("%s: \n", name);
   dump_hex(fd_bo_map(rsc->bo), fd_bo_size(rsc->bo));
}

static struct fd_resource *
fd_alloc_staging(struct fd_context *ctx, struct fd_resource *rsc,
                 unsigned level, const struct pipe_box *box, unsigned usage)
   assert_dt
{
   struct pipe_context *pctx = &ctx->base;
   struct pipe_resource tmpl = rsc->b.b;

   /* We cannot currently do stencil export on earlier gens, and
    * u_blitter cannot do blits involving stencil otherwise:
    */
   if ((ctx->screen->gen < 6) && !ctx->blit &&
       (util_format_get_mask(tmpl.format) & PIPE_MASK_S))
      return NULL;

   tmpl.width0 = box->width;
   tmpl.height0 = box->height;
   /* for array textures, box->depth is the array_size, otherwise
    * for 3d textures, it is the depth:
    */
   if (tmpl.array_size > 1) {
      if (tmpl.target == PIPE_TEXTURE_CUBE)
         tmpl.target = PIPE_TEXTURE_2D_ARRAY;
      tmpl.array_size = box->depth;
      tmpl.depth0 = 1;
   } else {
      tmpl.array_size = 1;
      tmpl.depth0 = box->depth;
   }
   tmpl.last_level = 0;
   tmpl.bind |= PIPE_BIND_LINEAR;
   tmpl.usage = PIPE_USAGE_STAGING;
   tmpl.flags = (usage & PIPE_MAP_READ) ? PIPE_RESOURCE_FLAG_MAP_COHERENT : 0;

   struct pipe_resource *pstaging =
      pctx->screen->resource_create(pctx->screen, &tmpl);
   if (!pstaging)
      return NULL;

   return fd_resource(pstaging);
}

static void
fd_blit_from_staging(struct fd_context *ctx,
                     struct fd_transfer *trans) assert_dt
{
   DBG("");
   struct pipe_resource *dst = trans->b.b.resource;
   struct pipe_blit_info blit = {};

   blit.dst.resource = dst;
   blit.dst.format = dst->format;
   blit.dst.level = trans->b.b.level;
   blit.dst.box = trans->b.b.box;
   blit.src.resource = trans->staging_prsc;
   blit.src.format = trans->staging_prsc->format;
   blit.src.level = 0;
   blit.src.box = trans->staging_box;
   blit.mask = util_format_get_mask(trans->staging_prsc->format);
   blit.filter = PIPE_TEX_FILTER_NEAREST;

   do_blit(ctx, &blit, false);
}

static void
fd_blit_to_staging(struct fd_context *ctx, struct fd_transfer *trans) assert_dt
{
   DBG("");
   struct pipe_resource *src = trans->b.b.resource;
   struct pipe_blit_info blit = {};

   blit.src.resource = src;
   blit.src.format = src->format;
   blit.src.level = trans->b.b.level;
   blit.src.box = trans->b.b.box;
   blit.dst.resource = trans->staging_prsc;
   blit.dst.format = trans->staging_prsc->format;
   blit.dst.level = 0;
   blit.dst.box = trans->staging_box;
   blit.mask = util_format_get_mask(trans->staging_prsc->format);
   blit.filter = PIPE_TEX_FILTER_NEAREST;

   do_blit(ctx, &blit, false);
}

static void
fd_resource_transfer_flush_region(struct pipe_context *pctx,
                                  struct pipe_transfer *ptrans,
                                  const struct pipe_box *box)
{
   struct fd_resource *rsc = fd_resource(ptrans->resource);

   if (ptrans->resource->target == PIPE_BUFFER)
      util_range_add(&rsc->b.b, &rsc->valid_buffer_range,
                     ptrans->box.x + box->x,
                     ptrans->box.x + box->x + box->width);
}

static void
flush_resource(struct fd_context *ctx, struct fd_resource *rsc,
               unsigned usage) assert_dt
{
   if (usage & PIPE_MAP_WRITE) {
      fd_bc_flush_readers(ctx, rsc);
   } else {
      fd_bc_flush_writer(ctx, rsc);
   }
}

static void
fd_flush_resource(struct pipe_context *pctx, struct pipe_resource *prsc)
   in_dt
{
   struct fd_context *ctx = fd_context(pctx);
   struct fd_resource *rsc = fd_resource(prsc);

   /* Flushing the resource is only required if we are relying on
    * implicit-sync, in which case the rendering must be flushed
    * to the kernel for the fence to be added to the backing GEM
    * object.
    */
   if (ctx->no_implicit_sync)
      return;

   flush_resource(ctx, rsc, PIPE_MAP_READ);

   /* If we had to flush a batch, make sure it makes it's way all the
    * way to the kernel:
    */
   fd_resource_wait(ctx, rsc, FD_BO_PREP_FLUSH);
}

static void
fd_resource_transfer_unmap(struct pipe_context *pctx,
                           struct pipe_transfer *ptrans)
   in_dt /* TODO for threaded-ctx we'll need to split out unsynchronized path */
{
   struct fd_context *ctx = fd_context(pctx);
   struct fd_resource *rsc = fd_resource(ptrans->resource);
   struct fd_transfer *trans = fd_transfer(ptrans);

   if (trans->staging_prsc) {
      if (ptrans->usage & PIPE_MAP_WRITE)
         fd_blit_from_staging(ctx, trans);
      pipe_resource_reference(&trans->staging_prsc, NULL);
   }

   if (trans->upload_ptr) {
      fd_bo_upload(rsc->bo, trans->upload_ptr, ptrans->box.x, ptrans->box.width);
      free(trans->upload_ptr);
   }

   util_range_add(&rsc->b.b, &rsc->valid_buffer_range, ptrans->box.x,
                  ptrans->box.x + ptrans->box.width);

   pipe_resource_reference(&ptrans->resource, NULL);

   assert(trans->b.staging == NULL); /* for threaded context only */

   /* Don't use pool_transfers_unsync. We are always in the driver
    * thread. Freeing an object into a different pool is allowed.
    */
   slab_free(&ctx->transfer_pool, ptrans);
}

static void
invalidate_resource(struct fd_resource *rsc, unsigned usage) assert_dt
{
   bool needs_flush = pending(rsc, !!(usage & PIPE_MAP_WRITE));
   unsigned op = translate_usage(usage);

   if (needs_flush || resource_busy(rsc, op)) {
      rebind_resource(rsc);
      realloc_bo(rsc, fd_bo_size(rsc->bo));
   } else {
      util_range_set_empty(&rsc->valid_buffer_range);
   }
}

static bool
valid_range(struct fd_resource *rsc, const struct pipe_box *box)
{
   return util_ranges_intersect(&rsc->valid_buffer_range, box->x, box->x + box->width);
}

static void *
resource_transfer_map_staging(struct pipe_context *pctx,
                              struct pipe_resource *prsc,
                              unsigned level, unsigned usage,
                              const struct pipe_box *box,
                              struct fd_transfer *trans)
   in_dt
{
   struct fd_context *ctx = fd_context(pctx);
   struct fd_resource *rsc = fd_resource(prsc);
   struct fd_resource *staging_rsc;

   assert(prsc->target != PIPE_BUFFER);

   staging_rsc = fd_alloc_staging(ctx, rsc, level, box, usage);
   if (!staging_rsc)
      return NULL;

   trans->staging_prsc = &staging_rsc->b.b;
   trans->b.b.stride = fd_resource_pitch(staging_rsc, 0);
   trans->b.b.layer_stride = fd_resource_layer_stride(staging_rsc, 0);
   trans->staging_box = *box;
   trans->staging_box.x = 0;
   trans->staging_box.y = 0;
   trans->staging_box.z = 0;

   if (usage & PIPE_MAP_READ) {
      fd_blit_to_staging(ctx, trans);

      fd_resource_wait(ctx, staging_rsc, FD_BO_PREP_READ);
   }

   ctx->stats.staging_uploads++;

   return fd_bo_map(staging_rsc->bo);
}

static void *
resource_transfer_map_unsync(struct pipe_context *pctx,
                             struct pipe_resource *prsc, unsigned level,
                             unsigned usage, const struct pipe_box *box,
                             struct fd_transfer *trans)
{
   struct fd_resource *rsc = fd_resource(prsc);
   enum pipe_format format = prsc->format;
   uint32_t offset;
   char *buf;

   if ((prsc->target == PIPE_BUFFER) &&
       !(usage & (PIPE_MAP_READ | PIPE_MAP_DIRECTLY | PIPE_MAP_PERSISTENT)) &&
       ((usage & PIPE_MAP_DISCARD_RANGE) || !valid_range(rsc, box)) &&
       fd_bo_prefer_upload(rsc->bo, box->width)) {
      trans->upload_ptr = malloc(box->width);
      return trans->upload_ptr;
   }

   buf = fd_bo_map(rsc->bo);

   /* With imported bo's allocated by something outside of mesa, when
    * running in a VM (using virtio_gpu kernel driver) we could end up in
    * a situation where we have a linear bo, but are unable to mmap it
    * because it was allocated without the VIRTGPU_BLOB_FLAG_USE_MAPPABLE
    * flag.  So we need end up needing to do a staging blit instead:
    */
   if (!buf)
      return resource_transfer_map_staging(pctx, prsc, level, usage, box, trans);

   offset = box->y / util_format_get_blockheight(format) * trans->b.b.stride +
            box->x / util_format_get_blockwidth(format) * rsc->layout.cpp +
            fd_resource_offset(rsc, level, box->z);

   if (usage & PIPE_MAP_WRITE)
      rsc->valid = true;

   return buf + offset;
}

/**
 * Note, with threaded_context, resource_transfer_map() is only called
 * in driver thread, but resource_transfer_map_unsync() can be called in
 * either driver or frontend thread.
 */
static void *
resource_transfer_map(struct pipe_context *pctx, struct pipe_resource *prsc,
                      unsigned level, unsigned usage,
                      const struct pipe_box *box,
                      struct fd_transfer *trans) in_dt
{
   struct fd_context *ctx = fd_context(pctx);
   struct fd_resource *rsc = fd_resource(prsc);
   char *buf;
   int ret = 0;

   tc_assert_driver_thread(ctx->tc);

   /* Strip the read flag if the buffer has been invalidated (or is freshly
    * created). Avoids extra staging blits of undefined data on glTexSubImage of
    * a fresh DEPTH_COMPONENT or STENCIL_INDEX texture being stored as z24s8.
    */
   if (!rsc->valid)
      usage &= ~PIPE_MAP_READ;

   /* we always need a staging texture for tiled buffers:
    *
    * TODO we might sometimes want to *also* shadow the resource to avoid
    * splitting a batch.. for ex, mid-frame texture uploads to a tiled
    * texture.
    */
   if (rsc->layout.tile_mode) {
      return resource_transfer_map_staging(pctx, prsc, level, usage, box, trans);
   } else if ((usage & PIPE_MAP_READ) && !fd_bo_is_cached(rsc->bo)) {
      perf_debug_ctx(ctx, "wc readback: prsc=%p, level=%u, usage=%x, box=%dx%d+%d,%d",
                     prsc, level, usage, box->width, box->height, box->x, box->y);
   }

   if (usage & PIPE_MAP_DISCARD_WHOLE_RESOURCE) {
      invalidate_resource(rsc, usage);
   } else {
      unsigned op = translate_usage(usage);
      bool needs_flush = pending(rsc, !!(usage & PIPE_MAP_WRITE));

      /* If the GPU is writing to the resource, or if it is reading from the
       * resource and we're trying to write to it, flush the renders.
       */
      bool busy = needs_flush || resource_busy(rsc, op);

      /* if we need to flush/stall, see if we can make a shadow buffer
       * to avoid this:
       *
       * TODO we could go down this path !reorder && !busy_for_read
       * ie. we only *don't* want to go down this path if the blit
       * will trigger a flush!
       */
      if (ctx->screen->reorder && busy && !(usage & PIPE_MAP_READ) &&
          (usage & PIPE_MAP_DISCARD_RANGE)) {

         /* try shadowing only if it avoids a flush, otherwise staging would
          * be better:
          */
         if (needs_flush && !(usage & TC_TRANSFER_MAP_NO_INVALIDATE) &&
               fd_try_shadow_resource(ctx, rsc, level, box, DRM_FORMAT_MOD_LINEAR)) {
            needs_flush = busy = false;
            ctx->stats.shadow_uploads++;
         } else {
            struct fd_resource *staging_rsc = NULL;

            if (needs_flush) {
               perf_debug_ctx(ctx, "flushing: %" PRSC_FMT, PRSC_ARGS(prsc));
               flush_resource(ctx, rsc, usage);
               needs_flush = false;
            }

            /* in this case, we don't need to shadow the whole resource,
             * since any draw that references the previous contents has
             * already had rendering flushed for all tiles.  So we can
             * use a staging buffer to do the upload.
             */
            if (is_renderable(prsc))
               staging_rsc = fd_alloc_staging(ctx, rsc, level, box, usage);
            if (staging_rsc) {
               trans->staging_prsc = &staging_rsc->b.b;
               trans->b.b.stride = fd_resource_pitch(staging_rsc, 0);
               trans->b.b.layer_stride =
                  fd_resource_layer_stride(staging_rsc, 0);
               trans->staging_box = *box;
               trans->staging_box.x = 0;
               trans->staging_box.y = 0;
               trans->staging_box.z = 0;
               buf = fd_bo_map(staging_rsc->bo);

               ctx->stats.staging_uploads++;

               return buf;
            }
         }
      }

      if (needs_flush) {
         flush_resource(ctx, rsc, usage);
         needs_flush = false;
      }

      /* The GPU keeps track of how the various bo's are being used, and
       * will wait if necessary for the proper operation to have
       * completed.
       */
      if (busy) {
         ret = fd_resource_wait(ctx, rsc, op);
         if (ret)
            return NULL;
      }
   }

   return resource_transfer_map_unsync(pctx, prsc, level, usage, box, trans);
}

static unsigned
improve_transfer_map_usage(struct fd_context *ctx, struct fd_resource *rsc,
                           unsigned usage, const struct pipe_box *box)
   /* Not *strictly* true, but the access to things that must only be in driver-
    * thread are protected by !(usage & TC_TRANSFER_MAP_THREADED_UNSYNC):
    */
   in_dt
{
   if (usage & TC_TRANSFER_MAP_NO_INVALIDATE) {
      usage &= ~PIPE_MAP_DISCARD_WHOLE_RESOURCE;
   }

   if (usage & TC_TRANSFER_MAP_THREADED_UNSYNC)
      usage |= PIPE_MAP_UNSYNCHRONIZED;

   if (!(usage &
         (TC_TRANSFER_MAP_NO_INFER_UNSYNCHRONIZED | PIPE_MAP_UNSYNCHRONIZED))) {
      if (ctx->in_shadow && !(usage & PIPE_MAP_READ)) {
         usage |= PIPE_MAP_UNSYNCHRONIZED;
      } else if ((usage & PIPE_MAP_WRITE) && (rsc->b.b.target == PIPE_BUFFER) &&
                 !valid_range(rsc, box)) {
         /* We are trying to write to a previously uninitialized range. No need
          * to synchronize.
          */
         usage |= PIPE_MAP_UNSYNCHRONIZED;
      }
   }

   return usage;
}

static void *
fd_resource_transfer_map(struct pipe_context *pctx, struct pipe_resource *prsc,
                         unsigned level, unsigned usage,
                         const struct pipe_box *box,
                         struct pipe_transfer **pptrans)
{
   struct fd_context *ctx = fd_context(pctx);
   struct fd_resource *rsc = fd_resource(prsc);
   struct fd_transfer *trans;
   struct pipe_transfer *ptrans;

   DBG("prsc=%p, level=%u, usage=%x, box=%dx%d+%d,%d", prsc, level, usage,
       box->width, box->height, box->x, box->y);

   if ((usage & PIPE_MAP_DIRECTLY) && rsc->layout.tile_mode) {
      DBG("CANNOT MAP DIRECTLY!\n");
      return NULL;
   }

   if (usage & TC_TRANSFER_MAP_THREADED_UNSYNC) {
      ptrans = slab_zalloc(&ctx->transfer_pool_unsync);
   } else {
      ptrans = slab_zalloc(&ctx->transfer_pool);
   }

   if (!ptrans)
      return NULL;

   trans = fd_transfer(ptrans);

   usage = improve_transfer_map_usage(ctx, rsc, usage, box);

   pipe_resource_reference(&ptrans->resource, prsc);
   ptrans->level = level;
   ptrans->usage = usage;
   ptrans->box = *box;
   ptrans->stride = fd_resource_pitch(rsc, level);
   ptrans->layer_stride = fd_resource_layer_stride(rsc, level);

   void *ret;
   if (usage & PIPE_MAP_UNSYNCHRONIZED) {
      ret = resource_transfer_map_unsync(pctx, prsc, level, usage, box, trans);
   } else {
      ret = resource_transfer_map(pctx, prsc, level, usage, box, trans);
   }

   if (ret) {
      *pptrans = ptrans;
   } else {
      fd_resource_transfer_unmap(pctx, ptrans);
   }

   return ret;
}

static void
fd_resource_destroy(struct pipe_screen *pscreen, struct pipe_resource *prsc)
{
   struct fd_screen *screen = fd_screen(prsc->screen);
   struct fd_resource *rsc = fd_resource(prsc);

   if (!rsc->is_replacement)
      fd_bc_invalidate_resource(rsc, true);
   if (rsc->bo)
      fd_bo_del(rsc->bo);
   if (rsc->lrz)
      fd_bo_del(rsc->lrz);
   if (rsc->scanout)
      renderonly_scanout_destroy(rsc->scanout, fd_screen(pscreen)->ro);

   if (prsc->target == PIPE_BUFFER)
      util_idalloc_mt_free(&screen->buffer_ids, rsc->b.buffer_id_unique);

   threaded_resource_deinit(prsc);

   util_range_destroy(&rsc->valid_buffer_range);
   simple_mtx_destroy(&rsc->lock);
   fd_resource_tracking_reference(&rsc->track, NULL);

   FREE(rsc);
}

static uint64_t
fd_resource_modifier(struct fd_resource *rsc)
{
   if (rsc->layout.ubwc_layer_size)
      return DRM_FORMAT_MOD_QCOM_COMPRESSED;

   switch (rsc->layout.tile_mode) {
   case 3: return DRM_FORMAT_MOD_QCOM_TILED3;
   case 2: return DRM_FORMAT_MOD_QCOM_TILED2;
   case 0: return DRM_FORMAT_MOD_LINEAR;
   default: return DRM_FORMAT_MOD_INVALID;
   }
}

static bool
fd_resource_get_handle(struct pipe_screen *pscreen, struct pipe_context *pctx,
                       struct pipe_resource *prsc, struct winsys_handle *handle,
                       unsigned usage)
   assert_dt
{
   struct fd_resource *rsc = fd_resource(prsc);

   rsc->b.is_shared = true;

   if (prsc->target == PIPE_BUFFER)
      tc_buffer_disable_cpu_storage(&rsc->b.b);

   handle->modifier = fd_resource_modifier(rsc);

   if (prsc->target != PIPE_BUFFER) {
      struct fdl_metadata metadata = {
         .modifier = handle->modifier,
      };
      fd_bo_set_metadata(rsc->bo, &metadata, sizeof(metadata));
   }

   DBG("%" PRSC_FMT ", modifier=%" PRIx64, PRSC_ARGS(prsc), handle->modifier);

   bool ret = fd_screen_bo_get_handle(pscreen, rsc->bo, rsc->scanout,
                                      fd_resource_pitch(rsc, 0), handle);

   if (!ret && !(prsc->bind & PIPE_BIND_SHARED)) {

      pctx = threaded_context_unwrap_sync(pctx);

      struct fd_context *ctx = pctx ?
            fd_context(pctx) : fd_screen_aux_context_get(pscreen);

      /* Since gl is horrible, we can end up getting asked to export a handle
       * for a rsc which was not originally allocated in a way that can be
       * exported (for ex, sub-allocation or in the case of virtgpu we need
       * to tell the kernel at allocation time that the buffer can be shared)
       *
       * If we get into this scenario we can try to reallocate.
       */

      prsc->bind |= PIPE_BIND_SHARED;

      ret = fd_try_shadow_resource(ctx, rsc, 0, NULL, handle->modifier);

      if (!pctx)
         fd_screen_aux_context_put(pscreen);

      if (!ret)
         return false;

      return fd_resource_get_handle(pscreen, pctx, prsc, handle, usage);
   }

   return ret;
}

/* special case to resize query buf after allocated.. */
void
fd_resource_resize(struct pipe_resource *prsc, uint32_t sz)
{
   struct fd_resource *rsc = fd_resource(prsc);

   assert(prsc->width0 == 0);
   assert(prsc->target == PIPE_BUFFER);
   assert(prsc->bind == PIPE_BIND_QUERY_BUFFER);

   prsc->width0 = sz;
   realloc_bo(rsc, fd_screen(prsc->screen)->setup_slices(rsc));
}

static void
fd_resource_layout_init(struct pipe_resource *prsc)
{
   struct fd_resource *rsc = fd_resource(prsc);
   struct fdl_layout *layout = &rsc->layout;

   layout->format = prsc->format;

   layout->width0 = prsc->width0;
   layout->height0 = prsc->height0;
   layout->depth0 = prsc->depth0;

   layout->cpp = util_format_get_blocksize(prsc->format);
   layout->cpp *= fd_resource_nr_samples(prsc);
   layout->cpp_shift = ffs(layout->cpp) - 1;
}

static struct fd_resource *
alloc_resource_struct(struct pipe_screen *pscreen,
                      const struct pipe_resource *tmpl)
{
   struct fd_screen *screen = fd_screen(pscreen);
   struct fd_resource *rsc = CALLOC_STRUCT(fd_resource);

   if (!rsc)
      return NULL;

   struct pipe_resource *prsc = &rsc->b.b;
   *prsc = *tmpl;

   pipe_reference_init(&prsc->reference, 1);
   prsc->screen = pscreen;
   rsc->hash = _mesa_hash_pointer(rsc);

   util_range_init(&rsc->valid_buffer_range);
   simple_mtx_init(&rsc->lock, mtx_plain);

   rsc->track = CALLOC_STRUCT(fd_resource_tracking);
   if (!rsc->track) {
      free(rsc);
      return NULL;
   }

   pipe_reference_init(&rsc->track->reference, 1);

   bool allow_cpu_storage = (tmpl->target == PIPE_BUFFER) &&
                            (tmpl->width0 < 0x1000);
   threaded_resource_init(prsc, allow_cpu_storage);

   if (tmpl->target == PIPE_BUFFER)
      rsc->b.buffer_id_unique = util_idalloc_mt_alloc(&screen->buffer_ids);

   return rsc;
}

enum fd_layout_type {
   ERROR,
   LINEAR,
   TILED,
   UBWC,
};

static bool
has_implicit_modifier(const uint64_t *modifiers, int count)
{
    return count == 0 ||
           drm_find_modifier(DRM_FORMAT_MOD_INVALID, modifiers, count);
}

static bool
has_explicit_modifier(const uint64_t *modifiers, int count)
{
    for (int i = 0; i < count; i++) {
        if (modifiers[i] != DRM_FORMAT_MOD_INVALID)
            return true;
    }
    return false;
}

static enum fd_layout_type
get_best_layout(struct fd_screen *screen,
                const struct pipe_resource *tmpl, const uint64_t *modifiers,
                int count)
{
   const bool can_implicit = has_implicit_modifier(modifiers, count);
   const bool can_explicit = has_explicit_modifier(modifiers, count);

   /* First, find all the conditions which would force us to linear */
   if (!screen->tile_mode)
      return LINEAR;

   if (!screen->tile_mode(tmpl))
      return LINEAR;

   if (tmpl->target == PIPE_BUFFER)
      return LINEAR;

   if (tmpl->bind & PIPE_BIND_LINEAR) {
      if (tmpl->usage != PIPE_USAGE_STAGING)
         perf_debug("%" PRSC_FMT ": forcing linear: bind flags",
                    PRSC_ARGS(tmpl));
      return LINEAR;
   }

   if (FD_DBG(NOTILE))
       return LINEAR;

   /* Shared resources without explicit modifiers must always be linear */
   if (!can_explicit && (tmpl->bind & PIPE_BIND_SHARED)) {
      perf_debug("%" PRSC_FMT
                 ": forcing linear: shared resource + implicit modifiers",
                 PRSC_ARGS(tmpl));
      return LINEAR;
   }

   bool ubwc_ok = is_a6xx(screen);
   if (FD_DBG(NOUBWC))
      ubwc_ok = false;

   /* Disallow UBWC for front-buffer rendering.  The GPU does not atomically
    * write pixel and header data, nor does the display atomically read it.
    * The result can be visual corruption (ie. moreso than normal tearing).
    */
   if (tmpl->bind & PIPE_BIND_USE_FRONT_RENDERING)
      ubwc_ok = false;

   /* Disallow UBWC when asked not to use data dependent bandwidth compression:
    */
   if (tmpl->bind & PIPE_BIND_CONST_BW)
      ubwc_ok = false;

   if (ubwc_ok && !can_implicit &&
       !drm_find_modifier(DRM_FORMAT_MOD_QCOM_COMPRESSED, modifiers, count)) {
      perf_debug("%" PRSC_FMT
                 ": not using UBWC: not in acceptable modifier set",
                 PRSC_ARGS(tmpl));
      ubwc_ok = false;
   }

   if (ubwc_ok)
      return UBWC;

   if (can_implicit ||
       drm_find_modifier(DRM_FORMAT_MOD_QCOM_TILED3, modifiers, count))
      return TILED;

   if (!drm_find_modifier(DRM_FORMAT_MOD_LINEAR, modifiers, count)) {
      perf_debug("%" PRSC_FMT ": need linear but not in modifier set",
                 PRSC_ARGS(tmpl));
      return ERROR;
   }

   perf_debug("%" PRSC_FMT ": not using tiling: explicit modifiers and no UBWC",
              PRSC_ARGS(tmpl));
   return LINEAR;
}

/**
 * Helper that allocates a resource and resolves its layout (but doesn't
 * allocate its bo).
 *
 * It returns a pipe_resource (as fd_resource_create_with_modifiers()
 * would do), and also bo's minimum required size as an output argument.
 */
static struct pipe_resource *
fd_resource_allocate_and_resolve(struct pipe_screen *pscreen,
                                 const struct pipe_resource *tmpl,
                                 const uint64_t *modifiers, int count,
                                 uint32_t *psize)
{
   struct fd_screen *screen = fd_screen(pscreen);
   struct fd_resource *rsc;
   struct pipe_resource *prsc;
   enum pipe_format format = tmpl->format;
   uint32_t size;

   rsc = alloc_resource_struct(pscreen, tmpl);
   if (!rsc)
      return NULL;

   prsc = &rsc->b.b;

   /* Clover creates buffers with PIPE_FORMAT_NONE: */
   if ((prsc->target == PIPE_BUFFER) && (format == PIPE_FORMAT_NONE))
      format = prsc->format = PIPE_FORMAT_R8_UNORM;

   DBG("%" PRSC_FMT, PRSC_ARGS(prsc));

   if (tmpl->bind & PIPE_BIND_SHARED)
      rsc->b.is_shared = true;

   fd_resource_layout_init(prsc);

   enum fd_layout_type layout =
      get_best_layout(screen, tmpl, modifiers, count);
   if (layout == ERROR) {
      free(prsc);
      return NULL;
   }

   if (layout >= TILED)
      rsc->layout.tile_mode = screen->tile_mode(prsc);
   if (layout == UBWC)
      rsc->layout.ubwc = true;

   rsc->internal_format = format;

   if (prsc->target == PIPE_BUFFER) {
      assert(prsc->format == PIPE_FORMAT_R8_UNORM);
      size = prsc->width0;
      fdl_layout_buffer(&rsc->layout, size);
   } else {
      size = screen->setup_slices(rsc);
   }

   /* special case for hw-query buffer, which we need to allocate before we
    * know the size:
    */
   if (size == 0) {
      /* note, semi-intention == instead of & */
      assert(prsc->bind == PIPE_BIND_QUERY_BUFFER);
      *psize = 0;
      return prsc;
   }

   /* Set the layer size if the (non-a6xx) backend hasn't done so. */
   if (rsc->layout.layer_first && !rsc->layout.layer_size) {
      rsc->layout.layer_size = align(size, 4096);
      size = rsc->layout.layer_size * prsc->array_size;
   }

   if (FD_DBG(LAYOUT))
      fdl_dump_layout(&rsc->layout);

   /* Hand out the resolved size. */
   if (psize)
      *psize = size;

   return prsc;
}

/**
 * Create a new texture object, using the given template info.
 */
static struct pipe_resource *
fd_resource_create_with_modifiers(struct pipe_screen *pscreen,
                                  const struct pipe_resource *tmpl,
                                  const uint64_t *modifiers, int count)
{
   struct fd_screen *screen = fd_screen(pscreen);
   struct fd_resource *rsc;
   struct pipe_resource *prsc;
   uint32_t size;

   /* when using kmsro, scanout buffers are allocated on the display device
    * create_with_modifiers() doesn't give us usage flags, so we have to
    * assume that all calls with modifiers are scanout-possible
    */
   if (screen->ro &&
       ((tmpl->bind & PIPE_BIND_SCANOUT) ||
        has_explicit_modifier(modifiers, count))) {
      struct pipe_resource scanout_templat = *tmpl;
      struct renderonly_scanout *scanout;
      struct winsys_handle handle;

      /* note: alignment is wrong for a6xx */
      scanout_templat.width0 = align(tmpl->width0, screen->info->gmem_align_w);

      scanout =
         renderonly_scanout_for_resource(&scanout_templat, screen->ro, &handle);
      if (!scanout)
         return NULL;

      renderonly_scanout_destroy(scanout, screen->ro);

      assert(handle.type == WINSYS_HANDLE_TYPE_FD);
      rsc = fd_resource(pscreen->resource_from_handle(
         pscreen, tmpl, &handle, PIPE_HANDLE_USAGE_FRAMEBUFFER_WRITE));
      close(handle.handle);
      if (!rsc)
         return NULL;

      return &rsc->b.b;
   }

   prsc =
      fd_resource_allocate_and_resolve(pscreen, tmpl, modifiers, count, &size);
   if (!prsc)
      return NULL;
   rsc = fd_resource(prsc);

   realloc_bo(rsc, size);
   if (!rsc->bo)
      goto fail;

   return prsc;
fail:
   fd_resource_destroy(pscreen, prsc);
   return NULL;
}

static struct pipe_resource *
fd_resource_create(struct pipe_screen *pscreen,
                   const struct pipe_resource *tmpl)
{
   const uint64_t mod = DRM_FORMAT_MOD_INVALID;
   return fd_resource_create_with_modifiers(pscreen, tmpl, &mod, 1);
}

/**
 * Create a texture from a winsys_handle. The handle is often created in
 * another process by first creating a pipe texture and then calling
 * resource_get_handle.
 */
static struct pipe_resource *
fd_resource_from_handle(struct pipe_screen *pscreen,
                        const struct pipe_resource *tmpl,
                        struct winsys_handle *handle, unsigned usage)
{
   struct fd_screen *screen = fd_screen(pscreen);
   struct fd_resource *rsc = alloc_resource_struct(pscreen, tmpl);

   if (!rsc)
      return NULL;

   if (tmpl->target == PIPE_BUFFER)
      tc_buffer_disable_cpu_storage(&rsc->b.b);

   struct fdl_slice *slice = fd_resource_slice(rsc, 0);
   struct pipe_resource *prsc = &rsc->b.b;

   DBG("%" PRSC_FMT ", modifier=%" PRIx64, PRSC_ARGS(prsc), handle->modifier);

   rsc->b.is_shared = true;

   fd_resource_layout_init(prsc);

   struct fd_bo *bo = fd_screen_bo_from_handle(pscreen, handle);
   if (!bo)
      goto fail;

   fd_resource_set_bo(rsc, bo);

   rsc->internal_format = tmpl->format;
   rsc->layout.layer_first = true;
   rsc->layout.pitch0 = handle->stride;
   slice->offset = handle->offset;
   slice->size0 = handle->stride * prsc->height0;

   /* use a pitchalign of gmem_align_w pixels, because GMEM resolve for
    * lower alignments is not implemented (but possible for a6xx at least)
    *
    * for UBWC-enabled resources, layout_resource_for_modifier will further
    * validate the pitch and set the right pitchalign
    */
   rsc->layout.pitchalign =
      fdl_cpp_shift(&rsc->layout) + util_logbase2(screen->info->gmem_align_w);

   /* apply the minimum pitchalign (note: actually 4 for a3xx but doesn't
    * matter) */
   if (is_a6xx(screen) || is_a5xx(screen))
      rsc->layout.pitchalign = MAX2(rsc->layout.pitchalign, 6);
   else
      rsc->layout.pitchalign = MAX2(rsc->layout.pitchalign, 5);

   if (rsc->layout.pitch0 < (prsc->width0 * rsc->layout.cpp) ||
       fd_resource_pitch(rsc, 0) != rsc->layout.pitch0)
      goto fail;

   assert(rsc->layout.cpp);

   if (screen->layout_resource_for_modifier(rsc, handle->modifier) < 0)
      goto fail;

   if (screen->ro) {
      rsc->scanout =
         renderonly_create_gpu_import_for_resource(prsc, screen->ro, NULL);
      /* failure is expected in some cases.. */
   }

   rsc->valid = true;

   return prsc;

fail:
   fd_resource_destroy(pscreen, prsc);
   return NULL;
}

bool
fd_render_condition_check(struct pipe_context *pctx)
{
   struct fd_context *ctx = fd_context(pctx);

   if (!ctx->cond_query)
      return true;

   perf_debug("Implementing conditional rendering using a CPU read instaed of HW conditional rendering.");

   union pipe_query_result res = {0};
   bool wait = ctx->cond_mode != PIPE_RENDER_COND_NO_WAIT &&
               ctx->cond_mode != PIPE_RENDER_COND_BY_REGION_NO_WAIT;

   if (pctx->get_query_result(pctx, ctx->cond_query, wait, &res))
      return (bool)res.u64 != ctx->cond_cond;

   return true;
}

static void
fd_invalidate_resource(struct pipe_context *pctx,
                       struct pipe_resource *prsc) in_dt
{
   struct fd_context *ctx = fd_context(pctx);
   struct fd_resource *rsc = fd_resource(prsc);

   if (prsc->target == PIPE_BUFFER) {
      /* Handle the glInvalidateBufferData() case:
       */
      invalidate_resource(rsc, PIPE_MAP_READ | PIPE_MAP_WRITE);
   } else if (rsc->track->write_batch) {
      /* Handle the glInvalidateFramebuffer() case, telling us that
       * we can skip resolve.
       */

      struct fd_batch *batch = rsc->track->write_batch;
      struct pipe_framebuffer_state *pfb = &batch->framebuffer;

      if (pfb->zsbuf && pfb->zsbuf->texture == prsc) {
         batch->resolve &= ~(FD_BUFFER_DEPTH | FD_BUFFER_STENCIL);
         fd_dirty_resource(ctx, prsc, FD_DIRTY_ZSA, true);
      }

      for (unsigned i = 0; i < pfb->nr_cbufs; i++) {
         if (pfb->cbufs[i] && pfb->cbufs[i]->texture == prsc) {
            batch->resolve &= ~(PIPE_CLEAR_COLOR0 << i);
            fd_dirty_resource(ctx, prsc, FD_DIRTY_FRAMEBUFFER, true);
         }
      }
   }

   rsc->valid = false;
}

static enum pipe_format
fd_resource_get_internal_format(struct pipe_resource *prsc)
{
   return fd_resource(prsc)->internal_format;
}

static void
fd_resource_set_stencil(struct pipe_resource *prsc,
                        struct pipe_resource *stencil)
{
   fd_resource(prsc)->stencil = fd_resource(stencil);
}

static struct pipe_resource *
fd_resource_get_stencil(struct pipe_resource *prsc)
{
   struct fd_resource *rsc = fd_resource(prsc);
   if (rsc->stencil)
      return &rsc->stencil->b.b;
   return NULL;
}

static const struct u_transfer_vtbl transfer_vtbl = {
   .resource_create = fd_resource_create,
   .resource_destroy = fd_resource_destroy,
   .transfer_map = fd_resource_transfer_map,
   .transfer_flush_region = fd_resource_transfer_flush_region,
   .transfer_unmap = fd_resource_transfer_unmap,
   .get_internal_format = fd_resource_get_internal_format,
   .set_stencil = fd_resource_set_stencil,
   .get_stencil = fd_resource_get_stencil,
};

static int
fd_layout_resource_for_modifier(struct fd_resource *rsc, uint64_t modifier)
{
   switch (modifier) {
   case DRM_FORMAT_MOD_LINEAR:
      /* The dri gallium frontend will pass DRM_FORMAT_MOD_INVALID to us
       * when it's called through any of the non-modifier BO create entry
       * points.  Other drivers will determine tiling from the kernel or
       * other legacy backchannels, but for freedreno it just means
       * LINEAR. */
   case DRM_FORMAT_MOD_INVALID:
      return 0;
   default:
      return -1;
   }
}

static struct pipe_resource *
fd_resource_from_memobj(struct pipe_screen *pscreen,
                        const struct pipe_resource *tmpl,
                        struct pipe_memory_object *pmemobj, uint64_t offset)
{
   struct fd_screen *screen = fd_screen(pscreen);
   struct fd_memory_object *memobj = fd_memory_object(pmemobj);
   struct pipe_resource *prsc;
   struct fd_resource *rsc;
   struct fdl_metadata metadata;
   uint32_t size;

   assert(memobj->bo);
   assert(offset == 0);

   /* We shouldn't get a scanout buffer here. */
   assert(!(tmpl->bind & PIPE_BIND_SCANOUT));

   uint64_t modifiers = DRM_FORMAT_MOD_INVALID;
   if (pmemobj->dedicated &&
       !fd_bo_get_metadata(memobj->bo, &metadata, sizeof(metadata))) {
      modifiers = metadata.modifier;
   } else if (tmpl->bind & PIPE_BIND_LINEAR) {
      modifiers = DRM_FORMAT_MOD_LINEAR;
   } else if (is_a6xx(screen) && tmpl->width0 >= FDL_MIN_UBWC_WIDTH) {
      modifiers = DRM_FORMAT_MOD_QCOM_COMPRESSED;
   }

   /* Allocate new pipe resource. */
   prsc = fd_resource_allocate_and_resolve(pscreen, tmpl, &modifiers, 1, &size);
   if (!prsc)
      return NULL;
   rsc = fd_resource(prsc);
   rsc->b.is_shared = true;

   /* bo's size has to be large enough, otherwise cleanup resource and fail
    * gracefully.
    */
   if (fd_bo_size(memobj->bo) < size) {
      fd_resource_destroy(pscreen, prsc);
      return NULL;
   }

   /* Share the bo with the memory object. */
   fd_resource_set_bo(rsc, fd_bo_ref(memobj->bo));

   return prsc;
}

static struct pipe_memory_object *
fd_memobj_create_from_handle(struct pipe_screen *pscreen,
                             struct winsys_handle *whandle, bool dedicated)
{
   struct fd_memory_object *memobj = CALLOC_STRUCT(fd_memory_object);
   if (!memobj)
      return NULL;

   struct fd_bo *bo = fd_screen_bo_from_handle(pscreen, whandle);
   if (!bo) {
      free(memobj);
      return NULL;
   }

   memobj->b.dedicated = dedicated;
   memobj->bo = bo;

   return &memobj->b;
}

static void
fd_memobj_destroy(struct pipe_screen *pscreen,
                  struct pipe_memory_object *pmemobj)
{
   struct fd_memory_object *memobj = fd_memory_object(pmemobj);

   assert(memobj->bo);
   fd_bo_del(memobj->bo);

   free(pmemobj);
}

void
fd_resource_screen_init(struct pipe_screen *pscreen)
{
   struct fd_screen *screen = fd_screen(pscreen);

   pscreen->resource_create = u_transfer_helper_resource_create;
   /* NOTE: u_transfer_helper does not yet support the _with_modifiers()
    * variant:
    */
   pscreen->resource_create_with_modifiers = fd_resource_create_with_modifiers;
   pscreen->resource_from_handle = fd_resource_from_handle;
   pscreen->resource_get_handle = fd_resource_get_handle;
   pscreen->resource_destroy = u_transfer_helper_resource_destroy;

   pscreen->transfer_helper =
      u_transfer_helper_create(&transfer_vtbl,
                               U_TRANSFER_HELPER_SEPARATE_Z32S8 |
                               U_TRANSFER_HELPER_MSAA_MAP);

   if (!screen->layout_resource_for_modifier)
      screen->layout_resource_for_modifier = fd_layout_resource_for_modifier;

   /* GL_EXT_memory_object */
   pscreen->memobj_create_from_handle = fd_memobj_create_from_handle;
   pscreen->memobj_destroy = fd_memobj_destroy;
   pscreen->resource_from_memobj = fd_resource_from_memobj;
}

static void
fd_blit_pipe(struct pipe_context *pctx,
             const struct pipe_blit_info *blit_info) in_dt
{
   /* wrap fd_blit to return void */
   fd_blit(pctx, blit_info);
}

void
fd_resource_context_init(struct pipe_context *pctx)
{
   pctx->buffer_map = u_transfer_helper_transfer_map;
   pctx->texture_map = u_transfer_helper_transfer_map;
   pctx->transfer_flush_region = u_transfer_helper_transfer_flush_region;
   pctx->buffer_unmap = u_transfer_helper_transfer_unmap;
   pctx->texture_unmap = u_transfer_helper_transfer_unmap;
   pctx->buffer_subdata = u_default_buffer_subdata;
   pctx->texture_subdata = u_default_texture_subdata;
   pctx->create_surface = fd_create_surface;
   pctx->surface_destroy = fd_surface_destroy;
   pctx->resource_copy_region = fd_resource_copy_region;
   pctx->blit = fd_blit_pipe;
   pctx->flush_resource = fd_flush_resource;
   pctx->invalidate_resource = fd_invalidate_resource;
   pctx->get_sample_position = u_default_get_sample_position;
}
