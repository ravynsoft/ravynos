/*
 * Copyright (C) 2019-2020 Collabora, Ltd.
 * Copyright (C) 2019 Alyssa Rosenzweig
 * Copyright (C) 2014-2017 Broadcom
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
 */

#include <assert.h>

#include "util/format/u_format.h"
#include "util/hash_table.h"
#include "util/ralloc.h"
#include "util/rounding.h"
#include "util/u_framebuffer.h"
#include "util/u_pack_color.h"
#include "pan_bo.h"
#include "pan_context.h"
#include "pan_util.h"

#define foreach_batch(ctx, idx)                                                \
   BITSET_FOREACH_SET(idx, ctx->batches.active, PAN_MAX_BATCHES)

static unsigned
panfrost_batch_idx(struct panfrost_batch *batch)
{
   return batch - batch->ctx->batches.slots;
}

static bool
panfrost_any_batch_other_than(struct panfrost_context *ctx, unsigned index)
{
   unsigned i;
   foreach_batch(ctx, i) {
      if (i != index)
         return true;
   }

   return false;
}

/* Adds the BO backing surface to a batch if the surface is non-null */

static void
panfrost_batch_add_surface(struct panfrost_batch *batch,
                           struct pipe_surface *surf)
{
   if (surf) {
      struct panfrost_resource *rsrc = pan_resource(surf->texture);
      pan_legalize_afbc_format(batch->ctx, rsrc, surf->format, true, false);
      panfrost_batch_write_rsrc(batch, rsrc, PIPE_SHADER_FRAGMENT);
   }
}

static void
panfrost_batch_init(struct panfrost_context *ctx,
                    const struct pipe_framebuffer_state *key,
                    struct panfrost_batch *batch)
{
   struct pipe_screen *pscreen = ctx->base.screen;
   struct panfrost_screen *screen = pan_screen(pscreen);
   struct panfrost_device *dev = &screen->dev;

   batch->ctx = ctx;

   batch->seqnum = ++ctx->batches.seqnum;

   util_dynarray_init(&batch->bos, NULL);

   batch->minx = batch->miny = ~0;
   batch->maxx = batch->maxy = 0;

   util_copy_framebuffer_state(&batch->key, key);

   /* Preallocate the main pool, since every batch has at least one job
    * structure so it will be used */
   panfrost_pool_init(&batch->pool, NULL, dev, 0, 65536, "Batch pool", true,
                      true);

   /* Don't preallocate the invisible pool, since not every batch will use
    * the pre-allocation, particularly if the varyings are larger than the
    * preallocation and a reallocation is needed after anyway. */
   panfrost_pool_init(&batch->invisible_pool, NULL, dev, PAN_BO_INVISIBLE,
                      65536, "Varyings", false, true);

   for (unsigned i = 0; i < batch->key.nr_cbufs; ++i)
      panfrost_batch_add_surface(batch, batch->key.cbufs[i]);

   panfrost_batch_add_surface(batch, batch->key.zsbuf);

   screen->vtbl.init_batch(batch);
}

static void
panfrost_batch_cleanup(struct panfrost_context *ctx,
                       struct panfrost_batch *batch)
{
   struct panfrost_device *dev = pan_device(ctx->base.screen);

   assert(batch->seqnum);

   if (ctx->batch == batch)
      ctx->batch = NULL;

   unsigned batch_idx = panfrost_batch_idx(batch);

   pan_bo_access *flags = util_dynarray_begin(&batch->bos);
   unsigned end_bo = util_dynarray_num_elements(&batch->bos, pan_bo_access);

   for (int i = 0; i < end_bo; ++i) {
      if (!flags[i])
         continue;

      struct panfrost_bo *bo = pan_lookup_bo(dev, i);
      panfrost_bo_unreference(bo);
   }

   /* There is no more writer for anything we wrote */
   hash_table_foreach(ctx->writers, ent) {
      if (ent->data == batch)
         _mesa_hash_table_remove(ctx->writers, ent);
   }

   panfrost_pool_cleanup(&batch->pool);
   panfrost_pool_cleanup(&batch->invisible_pool);

   util_unreference_framebuffer_state(&batch->key);

   util_dynarray_fini(&batch->bos);

   memset(batch, 0, sizeof(*batch));
   BITSET_CLEAR(ctx->batches.active, batch_idx);
}

static void panfrost_batch_submit(struct panfrost_context *ctx,
                                  struct panfrost_batch *batch);

static struct panfrost_batch *
panfrost_get_batch(struct panfrost_context *ctx,
                   const struct pipe_framebuffer_state *key)
{
   struct panfrost_batch *batch = NULL;

   for (unsigned i = 0; i < PAN_MAX_BATCHES; i++) {
      if (ctx->batches.slots[i].seqnum &&
          util_framebuffer_state_equal(&ctx->batches.slots[i].key, key)) {
         /* We found a match, increase the seqnum for the LRU
          * eviction logic.
          */
         ctx->batches.slots[i].seqnum = ++ctx->batches.seqnum;
         return &ctx->batches.slots[i];
      }

      if (!batch || batch->seqnum > ctx->batches.slots[i].seqnum)
         batch = &ctx->batches.slots[i];
   }

   assert(batch);

   /* The selected slot is used, we need to flush the batch */
   if (batch->seqnum) {
      perf_debug_ctx(ctx, "Flushing batch due to seqnum overflow");
      panfrost_batch_submit(ctx, batch);
   }

   panfrost_batch_init(ctx, key, batch);

   unsigned batch_idx = panfrost_batch_idx(batch);
   BITSET_SET(ctx->batches.active, batch_idx);

   return batch;
}

/* Get the job corresponding to the FBO we're currently rendering into */

struct panfrost_batch *
panfrost_get_batch_for_fbo(struct panfrost_context *ctx)
{
   /* If we already began rendering, use that */

   if (ctx->batch) {
      assert(util_framebuffer_state_equal(&ctx->batch->key,
                                          &ctx->pipe_framebuffer));
      return ctx->batch;
   }

   /* If not, look up the job */
   struct panfrost_batch *batch =
      panfrost_get_batch(ctx, &ctx->pipe_framebuffer);

   /* Set this job as the current FBO job. Will be reset when updating the
    * FB state and when submitting or releasing a job.
    */
   ctx->batch = batch;
   panfrost_dirty_state_all(ctx);
   return batch;
}

struct panfrost_batch *
panfrost_get_fresh_batch_for_fbo(struct panfrost_context *ctx,
                                 const char *reason)
{
   struct panfrost_batch *batch;

   batch = panfrost_get_batch(ctx, &ctx->pipe_framebuffer);
   panfrost_dirty_state_all(ctx);

   /* We only need to submit and get a fresh batch if there is no
    * draw/clear queued. Otherwise we may reuse the batch. */

   if (batch->draw_count + batch->compute_count > 0) {
      perf_debug_ctx(ctx, "Flushing the current FBO due to: %s", reason);
      panfrost_batch_submit(ctx, batch);
      batch = panfrost_get_batch(ctx, &ctx->pipe_framebuffer);
   }

   ctx->batch = batch;
   return batch;
}

static bool panfrost_batch_uses_resource(struct panfrost_batch *batch,
                                         struct panfrost_resource *rsrc);

static void
panfrost_batch_update_access(struct panfrost_batch *batch,
                             struct panfrost_resource *rsrc, bool writes)
{
   struct panfrost_context *ctx = batch->ctx;
   uint32_t batch_idx = panfrost_batch_idx(batch);

   if (writes) {
      _mesa_hash_table_insert(ctx->writers, rsrc, batch);
   }

   /* The rest of this routine is just about flushing other batches. If there
    * aren't any, we can skip a lot of work.
    */
   if (!panfrost_any_batch_other_than(ctx, batch_idx))
      return;

   struct hash_entry *entry = _mesa_hash_table_search(ctx->writers, rsrc);
   struct panfrost_batch *writer = entry ? entry->data : NULL;

   /* Both reads and writes flush the existing writer */
   if (writer != NULL && writer != batch)
      panfrost_batch_submit(ctx, writer);

   /* Writes (only) flush readers too */
   if (writes) {
      unsigned i;
      foreach_batch(ctx, i) {
         struct panfrost_batch *batch = &ctx->batches.slots[i];

         /* Skip the entry if this our batch. */
         if (i == batch_idx)
            continue;

         /* Submit if it's a user */
         if (panfrost_batch_uses_resource(batch, rsrc))
            panfrost_batch_submit(ctx, batch);
      }
   }
}

static pan_bo_access *
panfrost_batch_get_bo_access(struct panfrost_batch *batch, unsigned handle)
{
   unsigned size = util_dynarray_num_elements(&batch->bos, pan_bo_access);

   if (handle >= size) {
      unsigned grow = handle + 1 - size;

      memset(util_dynarray_grow(&batch->bos, pan_bo_access, grow), 0,
             grow * sizeof(pan_bo_access));
   }

   return util_dynarray_element(&batch->bos, pan_bo_access, handle);
}

static bool
panfrost_batch_uses_resource(struct panfrost_batch *batch,
                             struct panfrost_resource *rsrc)
{
   /* A resource is used iff its current BO is used */
   uint32_t handle = panfrost_bo_handle(rsrc->image.data.bo);
   unsigned size = util_dynarray_num_elements(&batch->bos, pan_bo_access);

   /* If out of bounds, certainly not used */
   if (handle >= size)
      return false;

   /* Otherwise check if nonzero access */
   return !!(*util_dynarray_element(&batch->bos, pan_bo_access, handle));
}

static void
panfrost_batch_add_bo_old(struct panfrost_batch *batch, struct panfrost_bo *bo,
                          uint32_t flags)
{
   if (!bo)
      return;

   pan_bo_access *entry =
      panfrost_batch_get_bo_access(batch, panfrost_bo_handle(bo));
   pan_bo_access old_flags = *entry;

   if (!old_flags) {
      batch->num_bos++;
      panfrost_bo_reference(bo);
   }

   if (old_flags == flags)
      return;

   flags |= old_flags;
   *entry = flags;
}

static uint32_t
panfrost_access_for_stage(enum pipe_shader_type stage)
{
   return (stage == PIPE_SHADER_FRAGMENT) ? PAN_BO_ACCESS_FRAGMENT
                                          : PAN_BO_ACCESS_VERTEX_TILER;
}

void
panfrost_batch_add_bo(struct panfrost_batch *batch, struct panfrost_bo *bo,
                      enum pipe_shader_type stage)
{
   panfrost_batch_add_bo_old(
      batch, bo, PAN_BO_ACCESS_READ | panfrost_access_for_stage(stage));
}

void
panfrost_batch_write_bo(struct panfrost_batch *batch, struct panfrost_bo *bo,
                        enum pipe_shader_type stage)
{
   panfrost_batch_add_bo_old(
      batch, bo, PAN_BO_ACCESS_WRITE | panfrost_access_for_stage(stage));
}

void
panfrost_batch_read_rsrc(struct panfrost_batch *batch,
                         struct panfrost_resource *rsrc,
                         enum pipe_shader_type stage)
{
   uint32_t access = PAN_BO_ACCESS_READ | panfrost_access_for_stage(stage);

   panfrost_batch_add_bo_old(batch, rsrc->image.data.bo, access);

   if (rsrc->separate_stencil)
      panfrost_batch_add_bo_old(batch, rsrc->separate_stencil->image.data.bo,
                                access);

   panfrost_batch_update_access(batch, rsrc, false);
}

void
panfrost_batch_write_rsrc(struct panfrost_batch *batch,
                          struct panfrost_resource *rsrc,
                          enum pipe_shader_type stage)
{
   uint32_t access = PAN_BO_ACCESS_WRITE | panfrost_access_for_stage(stage);

   panfrost_batch_add_bo_old(batch, rsrc->image.data.bo, access);

   if (rsrc->separate_stencil)
      panfrost_batch_add_bo_old(batch, rsrc->separate_stencil->image.data.bo,
                                access);

   panfrost_batch_update_access(batch, rsrc, true);
}

struct panfrost_bo *
panfrost_batch_create_bo(struct panfrost_batch *batch, size_t size,
                         uint32_t create_flags, enum pipe_shader_type stage,
                         const char *label)
{
   struct panfrost_bo *bo;

   bo = panfrost_bo_create(pan_device(batch->ctx->base.screen), size,
                           create_flags, label);
   panfrost_batch_add_bo(batch, bo, stage);

   /* panfrost_batch_add_bo() has retained a reference and
    * panfrost_bo_create() initialize the refcnt to 1, so let's
    * unreference the BO here so it gets released when the batch is
    * destroyed (unless it's retained by someone else in the meantime).
    */
   panfrost_bo_unreference(bo);
   return bo;
}

struct panfrost_bo *
panfrost_batch_get_scratchpad(struct panfrost_batch *batch,
                              unsigned size_per_thread,
                              unsigned thread_tls_alloc, unsigned core_id_range)
{
   unsigned size = panfrost_get_total_stack_size(
      size_per_thread, thread_tls_alloc, core_id_range);

   if (batch->scratchpad) {
      assert(panfrost_bo_size(batch->scratchpad) >= size);
   } else {
      batch->scratchpad =
         panfrost_batch_create_bo(batch, size, PAN_BO_INVISIBLE,
                                  PIPE_SHADER_VERTEX, "Thread local storage");

      panfrost_batch_add_bo(batch, batch->scratchpad, PIPE_SHADER_FRAGMENT);
   }

   return batch->scratchpad;
}

struct panfrost_bo *
panfrost_batch_get_shared_memory(struct panfrost_batch *batch, unsigned size,
                                 unsigned workgroup_count)
{
   if (batch->shared_memory) {
      assert(panfrost_bo_size(batch->shared_memory) >= size);
   } else {
      batch->shared_memory = panfrost_batch_create_bo(
         batch, size, PAN_BO_INVISIBLE, PIPE_SHADER_VERTEX,
         "Workgroup shared memory");
   }

   return batch->shared_memory;
}

static void
panfrost_batch_to_fb_info(const struct panfrost_batch *batch,
                          struct pan_fb_info *fb, struct pan_image_view *rts,
                          struct pan_image_view *zs, struct pan_image_view *s,
                          bool reserve)
{
   memset(fb, 0, sizeof(*fb));
   memset(rts, 0, sizeof(*rts) * 8);
   memset(zs, 0, sizeof(*zs));
   memset(s, 0, sizeof(*s));

   fb->width = batch->key.width;
   fb->height = batch->key.height;
   fb->extent.minx = batch->minx;
   fb->extent.miny = batch->miny;
   fb->extent.maxx = batch->maxx - 1;
   fb->extent.maxy = batch->maxy - 1;
   fb->nr_samples = util_framebuffer_get_num_samples(&batch->key);
   fb->rt_count = batch->key.nr_cbufs;
   fb->sprite_coord_origin = pan_tristate_get(batch->sprite_coord_origin);
   fb->first_provoking_vertex = pan_tristate_get(batch->first_provoking_vertex);

   static const unsigned char id_swz[] = {
      PIPE_SWIZZLE_X,
      PIPE_SWIZZLE_Y,
      PIPE_SWIZZLE_Z,
      PIPE_SWIZZLE_W,
   };

   for (unsigned i = 0; i < fb->rt_count; i++) {
      struct pipe_surface *surf = batch->key.cbufs[i];

      if (!surf)
         continue;

      struct panfrost_resource *prsrc = pan_resource(surf->texture);
      unsigned mask = PIPE_CLEAR_COLOR0 << i;

      if (batch->clear & mask) {
         fb->rts[i].clear = true;
         memcpy(fb->rts[i].clear_value, batch->clear_color[i],
                sizeof((fb->rts[i].clear_value)));
      }

      fb->rts[i].discard = !reserve && !(batch->resolve & mask);

      /* Clamp the rendering area to the damage extent. The
       * KHR_partial_update spec states that trying to render outside of
       * the damage region is "undefined behavior", so we should be safe.
       */
      if (!fb->rts[i].discard) {
         fb->extent.minx = MAX2(fb->extent.minx, prsrc->damage.extent.minx);
         fb->extent.miny = MAX2(fb->extent.miny, prsrc->damage.extent.miny);
         fb->extent.maxx = MIN2(fb->extent.maxx, prsrc->damage.extent.maxx - 1);
         fb->extent.maxy = MIN2(fb->extent.maxy, prsrc->damage.extent.maxy - 1);
         assert(fb->extent.minx <= fb->extent.maxx);
         assert(fb->extent.miny <= fb->extent.maxy);
      }

      rts[i].format = surf->format;
      rts[i].dim = MALI_TEXTURE_DIMENSION_2D;
      rts[i].last_level = rts[i].first_level = surf->u.tex.level;
      rts[i].first_layer = surf->u.tex.first_layer;
      rts[i].last_layer = surf->u.tex.last_layer;
      panfrost_set_image_view_planes(&rts[i], surf->texture);
      rts[i].nr_samples =
         surf->nr_samples ?: MAX2(surf->texture->nr_samples, 1);
      memcpy(rts[i].swizzle, id_swz, sizeof(rts[i].swizzle));
      fb->rts[i].crc_valid = &prsrc->valid.crc;
      fb->rts[i].view = &rts[i];

      /* Preload if the RT is read or updated */
      if (!(batch->clear & mask) &&
          ((batch->read & mask) ||
           ((batch->draws & mask) &&
            BITSET_TEST(prsrc->valid.data, fb->rts[i].view->first_level))))
         fb->rts[i].preload = true;
   }

   const struct pan_image_view *s_view = NULL, *z_view = NULL;
   struct panfrost_resource *z_rsrc = NULL, *s_rsrc = NULL;

   if (batch->key.zsbuf) {
      struct pipe_surface *surf = batch->key.zsbuf;
      z_rsrc = pan_resource(surf->texture);

      zs->format = surf->format == PIPE_FORMAT_Z32_FLOAT_S8X24_UINT
                      ? PIPE_FORMAT_Z32_FLOAT
                      : surf->format;
      zs->dim = MALI_TEXTURE_DIMENSION_2D;
      zs->last_level = zs->first_level = surf->u.tex.level;
      zs->first_layer = surf->u.tex.first_layer;
      zs->last_layer = surf->u.tex.last_layer;
      zs->planes[0] = &z_rsrc->image;
      zs->nr_samples = surf->nr_samples ?: MAX2(surf->texture->nr_samples, 1);
      memcpy(zs->swizzle, id_swz, sizeof(zs->swizzle));
      fb->zs.view.zs = zs;
      z_view = zs;
      if (util_format_is_depth_and_stencil(zs->format)) {
         s_view = zs;
         s_rsrc = z_rsrc;
      }

      if (z_rsrc->separate_stencil) {
         s_rsrc = z_rsrc->separate_stencil;
         s->format = PIPE_FORMAT_S8_UINT;
         s->dim = MALI_TEXTURE_DIMENSION_2D;
         s->last_level = s->first_level = surf->u.tex.level;
         s->first_layer = surf->u.tex.first_layer;
         s->last_layer = surf->u.tex.last_layer;
         s->planes[0] = &s_rsrc->image;
         s->nr_samples = surf->nr_samples ?: MAX2(surf->texture->nr_samples, 1);
         memcpy(s->swizzle, id_swz, sizeof(s->swizzle));
         fb->zs.view.s = s;
         s_view = s;
      }
   }

   if (batch->clear & PIPE_CLEAR_DEPTH) {
      fb->zs.clear.z = true;
      fb->zs.clear_value.depth = batch->clear_depth;
   }

   if (batch->clear & PIPE_CLEAR_STENCIL) {
      fb->zs.clear.s = true;
      fb->zs.clear_value.stencil = batch->clear_stencil;
   }

   fb->zs.discard.z = !reserve && !(batch->resolve & PIPE_CLEAR_DEPTH);
   fb->zs.discard.s = !reserve && !(batch->resolve & PIPE_CLEAR_STENCIL);

   if (!fb->zs.clear.z && z_rsrc &&
       ((batch->read & PIPE_CLEAR_DEPTH) ||
        ((batch->draws & PIPE_CLEAR_DEPTH) &&
         BITSET_TEST(z_rsrc->valid.data, z_view->first_level))))
      fb->zs.preload.z = true;

   if (!fb->zs.clear.s && s_rsrc &&
       ((batch->read & PIPE_CLEAR_STENCIL) ||
        ((batch->draws & PIPE_CLEAR_STENCIL) &&
         BITSET_TEST(s_rsrc->valid.data, s_view->first_level))))
      fb->zs.preload.s = true;

   /* Preserve both component if we have a combined ZS view and
    * one component needs to be preserved.
    */
   if (z_view && z_view == s_view && fb->zs.discard.z != fb->zs.discard.s) {
      bool valid = BITSET_TEST(z_rsrc->valid.data, z_view->first_level);

      fb->zs.discard.z = false;
      fb->zs.discard.s = false;
      fb->zs.preload.z = !fb->zs.clear.z && valid;
      fb->zs.preload.s = !fb->zs.clear.s && valid;
   }
}

static void
panfrost_emit_tile_map(struct panfrost_batch *batch, struct pan_fb_info *fb)
{
   if (batch->key.nr_cbufs < 1 || !batch->key.cbufs[0])
      return;

   struct pipe_surface *surf = batch->key.cbufs[0];
   struct panfrost_resource *pres = surf ? pan_resource(surf->texture) : NULL;

   if (pres && pres->damage.tile_map.enable) {
      fb->tile_map.base =
         pan_pool_upload_aligned(&batch->pool.base, pres->damage.tile_map.data,
                                 pres->damage.tile_map.size, 64);
      fb->tile_map.stride = pres->damage.tile_map.stride;
   }
}

static void
panfrost_batch_submit(struct panfrost_context *ctx,
                      struct panfrost_batch *batch)
{
   struct pipe_screen *pscreen = ctx->base.screen;
   struct panfrost_screen *screen = pan_screen(pscreen);
   bool has_frag = panfrost_has_fragment_job(batch);
   int ret;

   /* Nothing to do! */
   if (!has_frag && batch->compute_count == 0)
      goto out;

   if (batch->key.zsbuf && has_frag) {
      struct pipe_surface *surf = batch->key.zsbuf;
      struct panfrost_resource *z_rsrc = pan_resource(surf->texture);

      /* Shared depth/stencil resources are not supported, and would
       * break this optimisation. */
      assert(!(z_rsrc->base.bind & PAN_BIND_SHARED_MASK));

      if (batch->clear & PIPE_CLEAR_STENCIL) {
         z_rsrc->stencil_value = batch->clear_stencil;
         z_rsrc->constant_stencil = true;
      } else if (z_rsrc->constant_stencil) {
         batch->clear_stencil = z_rsrc->stencil_value;
         batch->clear |= PIPE_CLEAR_STENCIL;
      }

      if (batch->draws & PIPE_CLEAR_STENCIL)
         z_rsrc->constant_stencil = false;
   }

   struct pan_fb_info fb;
   struct pan_image_view rts[8], zs, s;

   panfrost_batch_to_fb_info(batch, &fb, rts, &zs, &s, false);
   panfrost_emit_tile_map(batch, &fb);

   ret = screen->vtbl.submit_batch(batch, &fb);
   if (ret)
      fprintf(stderr, "panfrost_batch_submit failed: %d\n", ret);

   /* We must reset the damage info of our render targets here even
    * though a damage reset normally happens when the DRI layer swaps
    * buffers. That's because there can be implicit flushes the GL
    * app is not aware of, and those might impact the damage region: if
    * part of the damaged portion is drawn during those implicit flushes,
    * you have to reload those areas before next draws are pushed, and
    * since the driver can't easily know what's been modified by the draws
    * it flushed, the easiest solution is to reload everything.
    */
   for (unsigned i = 0; i < batch->key.nr_cbufs; i++) {
      if (!batch->key.cbufs[i])
         continue;

      panfrost_resource_set_damage_region(
         ctx->base.screen, batch->key.cbufs[i]->texture, 0, NULL);
   }

out:
   panfrost_batch_cleanup(ctx, batch);
}

/* Submit all batches */

void
panfrost_flush_all_batches(struct panfrost_context *ctx, const char *reason)
{
   if (reason)
      perf_debug_ctx(ctx, "Flushing everything due to: %s", reason);

   struct panfrost_batch *batch = panfrost_get_batch_for_fbo(ctx);
   panfrost_batch_submit(ctx, batch);

   for (unsigned i = 0; i < PAN_MAX_BATCHES; i++) {
      if (ctx->batches.slots[i].seqnum)
         panfrost_batch_submit(ctx, &ctx->batches.slots[i]);
   }
}

void
panfrost_flush_writer(struct panfrost_context *ctx,
                      struct panfrost_resource *rsrc, const char *reason)
{
   struct hash_entry *entry = _mesa_hash_table_search(ctx->writers, rsrc);

   if (entry) {
      perf_debug_ctx(ctx, "Flushing writer due to: %s", reason);
      panfrost_batch_submit(ctx, entry->data);
   }
}

void
panfrost_flush_batches_accessing_rsrc(struct panfrost_context *ctx,
                                      struct panfrost_resource *rsrc,
                                      const char *reason)
{
   unsigned i;
   foreach_batch(ctx, i) {
      struct panfrost_batch *batch = &ctx->batches.slots[i];

      if (!panfrost_batch_uses_resource(batch, rsrc))
         continue;

      perf_debug_ctx(ctx, "Flushing user due to: %s", reason);
      panfrost_batch_submit(ctx, batch);
   }
}

bool
panfrost_any_batch_reads_rsrc(struct panfrost_context *ctx,
                              struct panfrost_resource *rsrc)
{
   unsigned i;
   foreach_batch(ctx, i) {
      struct panfrost_batch *batch = &ctx->batches.slots[i];

      if (panfrost_batch_uses_resource(batch, rsrc))
         return true;
   }

   return false;
}

bool
panfrost_any_batch_writes_rsrc(struct panfrost_context *ctx,
                               struct panfrost_resource *rsrc)
{
   return _mesa_hash_table_search(ctx->writers, rsrc) != NULL;
}

void
panfrost_batch_adjust_stack_size(struct panfrost_batch *batch)
{
   struct panfrost_context *ctx = batch->ctx;

   for (unsigned i = 0; i < PIPE_SHADER_TYPES; ++i) {
      struct panfrost_compiled_shader *ss = ctx->prog[i];

      if (!ss)
         continue;

      batch->stack_size = MAX2(batch->stack_size, ss->info.tls_size);
   }
}

void
panfrost_batch_clear(struct panfrost_batch *batch, unsigned buffers,
                     const union pipe_color_union *color, double depth,
                     unsigned stencil)
{
   struct panfrost_context *ctx = batch->ctx;
   struct panfrost_device *dev = pan_device(ctx->base.screen);

   if (buffers & PIPE_CLEAR_COLOR) {
      for (unsigned i = 0; i < ctx->pipe_framebuffer.nr_cbufs; ++i) {
         if (!(buffers & (PIPE_CLEAR_COLOR0 << i)))
            continue;

         enum pipe_format format = ctx->pipe_framebuffer.cbufs[i]->format;
         pan_pack_color(dev->blendable_formats, batch->clear_color[i], color,
                        format, false);
      }
   }

   if (buffers & PIPE_CLEAR_DEPTH) {
      batch->clear_depth = depth;
   }

   if (buffers & PIPE_CLEAR_STENCIL) {
      batch->clear_stencil = stencil;
   }

   batch->clear |= buffers;
   batch->resolve |= buffers;

   /* Clearing affects the entire framebuffer (by definition -- this is
    * the Gallium clear callback, which clears the whole framebuffer. If
    * the scissor test were enabled from the GL side, the gallium frontend
    * would emit a quad instead and we wouldn't go down this code path) */

   panfrost_batch_union_scissor(batch, 0, 0, ctx->pipe_framebuffer.width,
                                ctx->pipe_framebuffer.height);
}

/* Given a new bounding rectangle (scissor), let the job cover the union of the
 * new and old bounding rectangles */

void
panfrost_batch_union_scissor(struct panfrost_batch *batch, unsigned minx,
                             unsigned miny, unsigned maxx, unsigned maxy)
{
   batch->minx = MIN2(batch->minx, minx);
   batch->miny = MIN2(batch->miny, miny);
   batch->maxx = MAX2(batch->maxx, maxx);
   batch->maxy = MAX2(batch->maxy, maxy);
}

/**
 * Checks if rasterization should be skipped. If not, a TILER job must be
 * created for each draw, or the IDVS flow must be used.
 *
 * As a special case, if there is no vertex shader, no primitives are generated,
 * meaning the whole pipeline (including rasterization) should be skipped.
 */
bool
panfrost_batch_skip_rasterization(struct panfrost_batch *batch)
{
   struct panfrost_context *ctx = batch->ctx;
   struct pipe_rasterizer_state *rast = (void *)ctx->rasterizer;

   return (rast->rasterizer_discard || batch->scissor_culls_everything ||
           !batch->rsd[PIPE_SHADER_VERTEX]);
}
