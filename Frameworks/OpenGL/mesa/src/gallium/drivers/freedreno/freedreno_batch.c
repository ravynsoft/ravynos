/*
 * Copyright (C) 2016 Rob Clark <robclark@freedesktop.org>
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

#include "util/hash_table.h"
#include "util/list.h"
#include "util/set.h"
#include "util/u_string.h"

#include "freedreno_batch.h"
#include "freedreno_context.h"
#include "freedreno_fence.h"
#include "freedreno_query_hw.h"
#include "freedreno_resource.h"

static struct fd_ringbuffer *
alloc_ring(struct fd_batch *batch, unsigned sz, enum fd_ringbuffer_flags flags)
{
   struct fd_context *ctx = batch->ctx;

   /* if kernel is too old to support unlimited # of cmd buffers, we
    * have no option but to allocate large worst-case sizes so that
    * we don't need to grow the ringbuffer.  Performance is likely to
    * suffer, but there is no good alternative.
    *
    * Otherwise if supported, allocate a growable ring with initial
    * size of zero.
    */
   if ((fd_device_version(ctx->screen->dev) >= FD_VERSION_UNLIMITED_CMDS) &&
       !FD_DBG(NOGROW)) {
      flags |= FD_RINGBUFFER_GROWABLE;
      sz = 0;
   }

   return fd_submit_new_ringbuffer(batch->submit, sz, flags);
}

static struct fd_batch_subpass *
subpass_create(struct fd_batch *batch)
{
   struct fd_batch_subpass *subpass = CALLOC_STRUCT(fd_batch_subpass);

   subpass->draw = alloc_ring(batch, 0x100000, 0);

   /* Replace batch->draw with reference to current subpass, for
    * backwards compat with code that is not subpass aware.
    */
   if (batch->draw)
      fd_ringbuffer_del(batch->draw);
   batch->draw = fd_ringbuffer_ref(subpass->draw);

   list_addtail(&subpass->node, &batch->subpasses);

   return subpass;
}

static void
subpass_destroy(struct fd_batch_subpass *subpass)
{
   fd_ringbuffer_del(subpass->draw);
   if (subpass->subpass_clears)
      fd_ringbuffer_del(subpass->subpass_clears);
   list_del(&subpass->node);
   if (subpass->lrz)
      fd_bo_del(subpass->lrz);
   free(subpass);
}

struct fd_batch *
fd_batch_create(struct fd_context *ctx, bool nondraw)
{
   struct fd_batch *batch = CALLOC_STRUCT(fd_batch);

   if (!batch)
      return NULL;

   DBG("%p", batch);

   pipe_reference_init(&batch->reference, 1);
   batch->ctx = ctx;
   batch->nondraw = nondraw;

   batch->resources =
      _mesa_set_create(NULL, _mesa_hash_pointer, _mesa_key_pointer_equal);

   list_inithead(&batch->subpasses);

   batch->submit = fd_submit_new(ctx->pipe);
   if (batch->nondraw) {
      batch->gmem = alloc_ring(batch, 0x1000, FD_RINGBUFFER_PRIMARY);
   } else {
      batch->gmem = alloc_ring(batch, 0x100000, FD_RINGBUFFER_PRIMARY);

      /* a6xx+ re-uses draw rb for both draw and binning pass: */
      if (ctx->screen->gen < 6) {
         batch->binning = alloc_ring(batch, 0x100000, 0);
      }
   }

   /* Pre-attach private BOs: */
   for (unsigned i = 0; i < ctx->num_private_bos; i++)
      fd_ringbuffer_attach_bo(batch->gmem, ctx->private_bos[i]);

   batch->subpass = subpass_create(batch);

   batch->in_fence_fd = -1;
   batch->fence = NULL;

   /* Work around problems on earlier gens with submit merging, etc,
    * by always creating a fence to request that the submit is flushed
    * immediately:
    */
   if (ctx->screen->gen < 6)
      batch->fence = fd_pipe_fence_create(batch);

   fd_reset_wfi(batch);

   util_dynarray_init(&batch->draw_patches, NULL);
   util_dynarray_init(&(batch->fb_read_patches), NULL);

   if (is_a2xx(ctx->screen)) {
      util_dynarray_init(&batch->shader_patches, NULL);
      util_dynarray_init(&batch->gmem_patches, NULL);
   }

   if (is_a3xx(ctx->screen))
      util_dynarray_init(&batch->rbrc_patches, NULL);

   util_dynarray_init(&batch->samples, NULL);

   u_trace_init(&batch->trace, &ctx->trace_context);
   batch->last_timestamp_cmd = NULL;

   return batch;
}

struct fd_batch_subpass *
fd_batch_create_subpass(struct fd_batch *batch)
{
   assert(!batch->nondraw);

   struct fd_batch_subpass *subpass = subpass_create(batch);

   /* This new subpass inherits the current subpass.. this is replaced
    * if there is a depth clear
    */
   if (batch->subpass->lrz)
      subpass->lrz = fd_bo_ref(batch->subpass->lrz);

   batch->subpass = subpass;

   return subpass;
}

/**
 * Cleanup that we normally do when the submit is flushed, like dropping
 * rb references.  But also called when batch is destroyed just in case
 * it wasn't flushed.
 */
static void
cleanup_submit(struct fd_batch *batch)
{
   if (!batch->submit)
      return;

   foreach_subpass_safe (subpass, batch) {
      subpass_destroy(subpass);
   }

   fd_ringbuffer_del(batch->draw);
   fd_ringbuffer_del(batch->gmem);

   if (batch->binning) {
      fd_ringbuffer_del(batch->binning);
      batch->binning = NULL;
   }

   if (batch->prologue) {
      fd_ringbuffer_del(batch->prologue);
      batch->prologue = NULL;
   }

   if (batch->tile_epilogue) {
      fd_ringbuffer_del(batch->tile_epilogue);
      batch->tile_epilogue = NULL;
   }

   if (batch->epilogue) {
      fd_ringbuffer_del(batch->epilogue);
      batch->epilogue = NULL;
   }

   if (batch->tile_loads) {
      fd_ringbuffer_del(batch->tile_loads);
      batch->tile_loads = NULL;
   }

   if (batch->tile_store) {
      fd_ringbuffer_del(batch->tile_store);
      batch->tile_store = NULL;
   }

   fd_submit_del(batch->submit);
   batch->submit = NULL;
}

static void
batch_flush_dependencies(struct fd_batch *batch) assert_dt
{
   struct fd_batch_cache *cache = &batch->ctx->screen->batch_cache;
   struct fd_batch *dep;

   foreach_batch (dep, cache, batch->dependents_mask) {
      assert(dep->ctx == batch->ctx);
      fd_batch_flush(dep);
      fd_batch_reference(&dep, NULL);
   }

   batch->dependents_mask = 0;
}

static void
batch_reset_dependencies(struct fd_batch *batch)
{
   struct fd_batch_cache *cache = &batch->ctx->screen->batch_cache;
   struct fd_batch *dep;

   foreach_batch (dep, cache, batch->dependents_mask) {
      fd_batch_reference(&dep, NULL);
   }

   batch->dependents_mask = 0;
}

static void
batch_reset_resources(struct fd_batch *batch)
{
   fd_screen_assert_locked(batch->ctx->screen);

   set_foreach (batch->resources, entry) {
      struct fd_resource *rsc = (struct fd_resource *)entry->key;
      _mesa_set_remove(batch->resources, entry);
      assert(rsc->track->batch_mask & (1 << batch->idx));
      rsc->track->batch_mask &= ~(1 << batch->idx);
      if (rsc->track->write_batch == batch)
         fd_batch_reference_locked(&rsc->track->write_batch, NULL);
   }
}

void
__fd_batch_destroy_locked(struct fd_batch *batch)
{
   struct fd_context *ctx = batch->ctx;

   DBG("%p", batch);

   fd_screen_assert_locked(batch->ctx->screen);

   fd_bc_invalidate_batch(batch, true);

   batch_reset_resources(batch);
   assert(batch->resources->entries == 0);
   _mesa_set_destroy(batch->resources, NULL);

   fd_screen_unlock(ctx->screen);
   batch_reset_dependencies(batch);
   assert(batch->dependents_mask == 0);

   util_copy_framebuffer_state(&batch->framebuffer, NULL);

   pipe_resource_reference(&batch->query_buf, NULL);

   if (batch->in_fence_fd != -1)
      close(batch->in_fence_fd);

   /* in case batch wasn't flushed but fence was created: */
   if (batch->fence)
      fd_pipe_fence_set_batch(batch->fence, NULL);

   fd_pipe_fence_ref(&batch->fence, NULL);

   cleanup_submit(batch);

   util_dynarray_fini(&batch->draw_patches);
   util_dynarray_fini(&(batch->fb_read_patches));

   if (is_a2xx(batch->ctx->screen)) {
      util_dynarray_fini(&batch->shader_patches);
      util_dynarray_fini(&batch->gmem_patches);
   }

   if (is_a3xx(batch->ctx->screen))
      util_dynarray_fini(&batch->rbrc_patches);

   while (batch->samples.size > 0) {
      struct fd_hw_sample *samp =
         util_dynarray_pop(&batch->samples, struct fd_hw_sample *);
      fd_hw_sample_reference(batch->ctx, &samp, NULL);
   }
   util_dynarray_fini(&batch->samples);

   u_trace_fini(&batch->trace);

   free(batch->key);
   free(batch);
   fd_screen_lock(ctx->screen);
}

void
__fd_batch_destroy(struct fd_batch *batch)
{
   struct fd_screen *screen = batch->ctx->screen;
   fd_screen_lock(screen);
   __fd_batch_destroy_locked(batch);
   fd_screen_unlock(screen);
}

void
__fd_batch_describe(char *buf, const struct fd_batch *batch)
{
   sprintf(buf, "fd_batch<%u>", batch->seqno);
}

/* Get per-batch prologue */
struct fd_ringbuffer *
fd_batch_get_prologue(struct fd_batch *batch)
{
   if (!batch->prologue)
      batch->prologue = alloc_ring(batch, 0x1000, 0);
   return batch->prologue;
}

/* Only called from fd_batch_flush() */
static void
batch_flush(struct fd_batch *batch) assert_dt
{
   DBG("%p: needs_flush=%d", batch, batch->needs_flush);

   if (batch->flushed)
      return;

   tc_assert_driver_thread(batch->ctx->tc);

   batch->needs_flush = false;

   /* close out the draw cmds by making sure any active queries are
    * paused:
    */
   fd_batch_finish_queries(batch);

   batch_flush_dependencies(batch);

   fd_screen_lock(batch->ctx->screen);
   batch_reset_resources(batch);
   /* NOTE: remove=false removes the batch from the hashtable, so future
    * lookups won't cache-hit a flushed batch, but leaves the weak reference
    * to the batch to avoid having multiple batches with same batch->idx, as
    * that causes all sorts of hilarity.
    */
   fd_bc_invalidate_batch(batch, false);
   batch->flushed = true;

   if (batch == batch->ctx->batch)
      fd_batch_reference_locked(&batch->ctx->batch, NULL);

   if (batch == batch->ctx->batch_nondraw)
      fd_batch_reference_locked(&batch->ctx->batch_nondraw, NULL);

   fd_screen_unlock(batch->ctx->screen);

   if (batch->fence)
      fd_pipe_fence_ref(&batch->ctx->last_fence, batch->fence);

   fd_gmem_render_tiles(batch);

   assert(batch->reference.count > 0);

   cleanup_submit(batch);
}

void
fd_batch_set_fb(struct fd_batch *batch, const struct pipe_framebuffer_state *pfb)
{
   assert(!batch->nondraw);

   util_copy_framebuffer_state(&batch->framebuffer, pfb);

   if (!pfb->zsbuf)
      return;

   struct fd_resource *zsbuf = fd_resource(pfb->zsbuf->texture);

   /* Switching back to a batch we'd previously started constructing shouldn't
    * result in a different lrz.  The dependency tracking should avoid another
    * batch writing/clearing our depth buffer.
    */
   if (batch->subpass->lrz) {
      assert(batch->subpass->lrz == zsbuf->lrz);
   } else if (zsbuf->lrz) {
      batch->subpass->lrz = fd_bo_ref(zsbuf->lrz);
   }
}


/* NOTE: could drop the last ref to batch
 */
void
fd_batch_flush(struct fd_batch *batch)
{
   struct fd_batch *tmp = NULL;

   /* NOTE: we need to hold an extra ref across the body of flush,
    * since the last ref to this batch could be dropped when cleaning
    * up used_resources
    */
   fd_batch_reference(&tmp, batch);
   batch_flush(tmp);
   fd_batch_reference(&tmp, NULL);
}

/* find a batches dependents mask, including recursive dependencies: */
static uint32_t
recursive_dependents_mask(struct fd_batch *batch)
{
   struct fd_batch_cache *cache = &batch->ctx->screen->batch_cache;
   struct fd_batch *dep;
   uint32_t dependents_mask = batch->dependents_mask;

   foreach_batch (dep, cache, batch->dependents_mask)
      dependents_mask |= recursive_dependents_mask(dep);

   return dependents_mask;
}

bool
fd_batch_has_dep(struct fd_batch *batch, struct fd_batch *dep)
{
   return !!(batch->dependents_mask & (1 << dep->idx));
}

void
fd_batch_add_dep(struct fd_batch *batch, struct fd_batch *dep)
{
   fd_screen_assert_locked(batch->ctx->screen);

   assert(batch->ctx == dep->ctx);

   if (fd_batch_has_dep(batch, dep))
      return;

   /* a loop should not be possible */
   assert(!((1 << batch->idx) & recursive_dependents_mask(dep)));

   struct fd_batch *other = NULL;
   fd_batch_reference_locked(&other, dep);
   batch->dependents_mask |= (1 << dep->idx);
   DBG("%p: added dependency on %p", batch, dep);
}

static void
flush_write_batch(struct fd_resource *rsc) assert_dt
{
   struct fd_batch *b = NULL;
   fd_batch_reference_locked(&b, rsc->track->write_batch);

   fd_screen_unlock(b->ctx->screen);
   fd_batch_flush(b);
   fd_screen_lock(b->ctx->screen);

   fd_batch_reference_locked(&b, NULL);
}

static void
fd_batch_add_resource(struct fd_batch *batch, struct fd_resource *rsc)
{
   if (likely(fd_batch_references_resource(batch, rsc))) {
      assert(_mesa_set_search_pre_hashed(batch->resources, rsc->hash, rsc));
      return;
   }

   assert(!_mesa_set_search(batch->resources, rsc));

   _mesa_set_add_pre_hashed(batch->resources, rsc->hash, rsc);
   rsc->track->batch_mask |= (1 << batch->idx);

   fd_ringbuffer_attach_bo(batch->draw, rsc->bo);
   if (unlikely(rsc->b.b.next)) {
      struct fd_resource *n = fd_resource(rsc->b.b.next);
      fd_ringbuffer_attach_bo(batch->draw, n->bo);
   }
}

void
fd_batch_resource_write(struct fd_batch *batch, struct fd_resource *rsc)
{
   struct fd_resource_tracking *track = rsc->track;

   fd_screen_assert_locked(batch->ctx->screen);

   DBG("%p: write %p", batch, rsc);

   /* Must do this before the early out, so we unset a previous resource
    * invalidate (which may have left the write_batch state in place).
    */
   rsc->valid = true;

   if (track->write_batch == batch)
      return;

   if (rsc->stencil)
      fd_batch_resource_write(batch, rsc->stencil);

   /* note, invalidate write batch, to avoid further writes to rsc
    * resulting in a write-after-read hazard.
    */

   /* if we are pending read or write by any other batch, they need to
    * be ordered before the current batch:
    */
   if (unlikely(track->batch_mask & ~(1 << batch->idx))) {
      struct fd_batch_cache *cache = &batch->ctx->screen->batch_cache;
      struct fd_batch *dep;

      if (track->write_batch) {
         /* Cross-context writes without flush/barrier are undefined.
          * Lets simply protect ourself from crashing by avoiding cross-
          * ctx dependencies and let the app have the undefined behavior
          * it asked for:
          */
         if (track->write_batch->ctx != batch->ctx) {
            fd_ringbuffer_attach_bo(batch->draw, rsc->bo);
            return;
         }

         flush_write_batch(rsc);
      }

      foreach_batch (dep, cache, track->batch_mask) {
         struct fd_batch *b = NULL;
         if ((dep == batch) || (dep->ctx != batch->ctx))
            continue;
         /* note that batch_add_dep could flush and unref dep, so
          * we need to hold a reference to keep it live for the
          * fd_bc_invalidate_batch()
          */
         fd_batch_reference(&b, dep);
         fd_batch_add_dep(batch, b);
         fd_bc_invalidate_batch(b, false);
         fd_batch_reference_locked(&b, NULL);
      }
   }
   fd_batch_reference_locked(&track->write_batch, batch);

   fd_batch_add_resource(batch, rsc);

   fd_batch_write_prep(batch, rsc);
}

void
fd_batch_resource_read_slowpath(struct fd_batch *batch, struct fd_resource *rsc)
{
   fd_screen_assert_locked(batch->ctx->screen);

   if (rsc->stencil)
      fd_batch_resource_read(batch, rsc->stencil);

   DBG("%p: read %p", batch, rsc);

   struct fd_resource_tracking *track = rsc->track;

   /* If reading a resource pending a write, go ahead and flush the
    * writer.  This avoids situations where we end up having to
    * flush the current batch in _resource_used()
    */
   if (unlikely(track->write_batch && track->write_batch != batch)) {
      if (track->write_batch->ctx != batch->ctx) {
         /* Reading results from another context without flush/barrier
          * is undefined.  Let's simply protect ourself from crashing
          * by avoiding cross-ctx dependencies and let the app have the
          * undefined behavior it asked for:
          */
         fd_ringbuffer_attach_bo(batch->draw, rsc->bo);
         return;
      }

      flush_write_batch(rsc);
   }

   fd_batch_add_resource(batch, rsc);
}

void
fd_batch_check_size(struct fd_batch *batch)
{
   if (batch->num_draws > 100000) {
      fd_batch_flush(batch);
      return;
   }

   /* Place a reasonable upper bound on prim/draw stream buffer size: */
   const unsigned limit_bits = 8 * 8 * 1024 * 1024;
   if ((batch->prim_strm_bits > limit_bits) ||
       (batch->draw_strm_bits > limit_bits)) {
      fd_batch_flush(batch);
      return;
   }

   if (!fd_ringbuffer_check_size(batch->draw))
      fd_batch_flush(batch);
}

/* emit a WAIT_FOR_IDLE only if needed, ie. if there has not already
 * been one since last draw:
 */
void
fd_wfi(struct fd_batch *batch, struct fd_ringbuffer *ring)
{
   if (batch->needs_wfi) {
      if (batch->ctx->screen->gen >= 5)
         OUT_WFI5(ring);
      else
         OUT_WFI(ring);
      batch->needs_wfi = false;
   }
}
