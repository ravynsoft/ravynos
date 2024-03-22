/*
 * Copyright 2013 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#include "si_pipe.h"
#include "util/u_memory.h"
#include "util/u_transfer.h"
#include "util/u_upload_mgr.h"

#include <inttypes.h>
#include <stdio.h>

bool si_cs_is_buffer_referenced(struct si_context *sctx, struct pb_buffer_lean *buf,
                                unsigned usage)
{
   return sctx->ws->cs_is_buffer_referenced(&sctx->gfx_cs, buf, usage);
}

void *si_buffer_map(struct si_context *sctx, struct si_resource *resource,
                    unsigned usage)
{
   return sctx->ws->buffer_map(sctx->ws, resource->buf, &sctx->gfx_cs, usage);
}

void si_init_resource_fields(struct si_screen *sscreen, struct si_resource *res, uint64_t size,
                             unsigned alignment)
{
   struct si_texture *tex = (struct si_texture *)res;

   res->bo_size = size;
   res->bo_alignment_log2 = util_logbase2(alignment);
   res->flags = 0;
   res->texture_handle_allocated = false;
   res->image_handle_allocated = false;

   switch (res->b.b.usage) {
   case PIPE_USAGE_STREAM:
      res->flags |= RADEON_FLAG_GTT_WC;
      res->domains = RADEON_DOMAIN_GTT;
      break;
   case PIPE_USAGE_STAGING:
      /* Transfers are likely to occur more often with these
       * resources. */
      res->domains = RADEON_DOMAIN_GTT;
      break;
   case PIPE_USAGE_DYNAMIC:
   case PIPE_USAGE_DEFAULT:
   case PIPE_USAGE_IMMUTABLE:
   default:
      /* Not listing GTT here improves performance in some
       * apps. */
      res->domains = RADEON_DOMAIN_VRAM;
      res->flags |= RADEON_FLAG_GTT_WC;
      break;
   }

   if (res->b.b.target == PIPE_BUFFER && res->b.b.flags & PIPE_RESOURCE_FLAG_MAP_PERSISTENT) {
      /* Use GTT for all persistent mappings with older
       * kernels, because they didn't always flush the HDP
       * cache before CS execution.
       *
       * Write-combined CPU mappings are fine, the kernel
       * ensures all CPU writes finish before the GPU
       * executes a command stream.
       *
       * radeon doesn't have good BO move throttling, so put all
       * persistent buffers into GTT to prevent VRAM CPU page faults.
       */
      if (!sscreen->info.is_amdgpu)
         res->domains = RADEON_DOMAIN_GTT;
   }

   /* Tiled textures are unmappable. Always put them in VRAM. */
   if ((res->b.b.target != PIPE_BUFFER && !tex->surface.is_linear) ||
       res->b.b.flags & PIPE_RESOURCE_FLAG_UNMAPPABLE) {
      res->domains = RADEON_DOMAIN_VRAM;
      res->flags |= RADEON_FLAG_NO_CPU_ACCESS | RADEON_FLAG_GTT_WC;
   }

   /* Displayable and shareable surfaces are not suballocated. */
   if (res->b.b.bind & (PIPE_BIND_SHARED | PIPE_BIND_SCANOUT))
      res->flags |= RADEON_FLAG_NO_SUBALLOC; /* shareable */
   else
      res->flags |= RADEON_FLAG_NO_INTERPROCESS_SHARING;

   /* PIPE_BIND_CUSTOM is used by si_vid_create_buffer which wants
    * non-suballocated buffers.
    */
   if (res->b.b.bind & PIPE_BIND_CUSTOM)
      res->flags |= RADEON_FLAG_NO_SUBALLOC;

   if (res->b.b.bind & PIPE_BIND_PROTECTED ||
       /* Force scanout/depth/stencil buffer allocation to be encrypted */
       (sscreen->debug_flags & DBG(TMZ) &&
        res->b.b.bind & (PIPE_BIND_RENDER_TARGET | PIPE_BIND_DEPTH_STENCIL)))
      res->flags |= RADEON_FLAG_ENCRYPTED;

   if (res->b.b.flags & PIPE_RESOURCE_FLAG_ENCRYPTED)
      res->flags |= RADEON_FLAG_ENCRYPTED;

   if (sscreen->debug_flags & DBG(NO_WC))
      res->flags &= ~RADEON_FLAG_GTT_WC;

   if (res->b.b.flags & SI_RESOURCE_FLAG_READ_ONLY)
      res->flags |= RADEON_FLAG_READ_ONLY;

   if (res->b.b.flags & SI_RESOURCE_FLAG_32BIT)
      res->flags |= RADEON_FLAG_32BIT;

   if (res->b.b.flags & SI_RESOURCE_FLAG_DRIVER_INTERNAL)
      res->flags |= RADEON_FLAG_DRIVER_INTERNAL;

   if (res->b.b.flags & PIPE_RESOURCE_FLAG_SPARSE)
      res->flags |= RADEON_FLAG_SPARSE;

   /* For higher throughput and lower latency over PCIe assuming sequential access.
    * Only CP DMA and optimized compute benefit from this.
    * GFX8 and older don't support RADEON_FLAG_GL2_BYPASS.
    */
   if (sscreen->info.gfx_level >= GFX9 &&
       res->b.b.flags & SI_RESOURCE_FLAG_GL2_BYPASS)
      res->flags |= RADEON_FLAG_GL2_BYPASS;

   if (res->b.b.flags & SI_RESOURCE_FLAG_DISCARDABLE &&
       sscreen->info.drm_major == 3 && sscreen->info.drm_minor >= 47) {
      /* Assume VRAM, so that we can use BIG_PAGE. */
      assert(res->domains == RADEON_DOMAIN_VRAM);
      res->flags |= RADEON_FLAG_DISCARDABLE;
   }

   if (res->domains & RADEON_DOMAIN_VRAM) {
      /* We don't want to evict buffers from VRAM by mapping them for CPU access,
       * because they might never be moved back again. If a buffer is large enough,
       * upload data by copying from a temporary GTT buffer.
       */
      if (sscreen->info.has_dedicated_vram &&
          !res->b.cpu_storage && /* TODO: The CPU storage breaks this. */
          size >= sscreen->options.max_vram_map_size)
         res->b.b.flags |= PIPE_RESOURCE_FLAG_DONT_MAP_DIRECTLY;
   }
}

bool si_alloc_resource(struct si_screen *sscreen, struct si_resource *res)
{
   struct pb_buffer_lean *old_buf, *new_buf;

   /* Allocate a new resource. */
   new_buf = sscreen->ws->buffer_create(sscreen->ws, res->bo_size, 1 << res->bo_alignment_log2,
                                        res->domains, res->flags);
   if (!new_buf) {
      return false;
   }

   /* Replace the pointer such that if res->buf wasn't NULL, it won't be
    * NULL. This should prevent crashes with multiple contexts using
    * the same buffer where one of the contexts invalidates it while
    * the others are using it. */
   old_buf = res->buf;
   res->buf = new_buf; /* should be atomic */
   res->gpu_address = sscreen->ws->buffer_get_virtual_address(res->buf);

   if (res->flags & RADEON_FLAG_32BIT) {
      uint64_t start = res->gpu_address;
      uint64_t last = start + res->bo_size - 1;
      (void)start;
      (void)last;

      assert((start >> 32) == sscreen->info.address32_hi);
      assert((last >> 32) == sscreen->info.address32_hi);
   }

   radeon_bo_reference(sscreen->ws, &old_buf, NULL);

   util_range_set_empty(&res->valid_buffer_range);
   res->TC_L2_dirty = false;

   /* Print debug information. */
   if (sscreen->debug_flags & DBG(VM) && res->b.b.target == PIPE_BUFFER) {
      fprintf(stderr, "VM start=0x%" PRIX64 "  end=0x%" PRIX64 " | Buffer %" PRIu64 " bytes | Flags: ",
              res->gpu_address, res->gpu_address + res->buf->size, res->buf->size);
      si_res_print_flags(res->flags);
      fprintf(stderr, "\n");
   }

   if (res->b.b.flags & SI_RESOURCE_FLAG_CLEAR) {
      struct si_context *ctx = si_get_aux_context(&sscreen->aux_context.general);
      uint32_t value = 0;

      si_clear_buffer(ctx, &res->b.b, 0, res->bo_size, &value, 4, SI_OP_SYNC_AFTER,
                      SI_COHERENCY_SHADER, SI_AUTO_SELECT_CLEAR_METHOD);
      si_put_aux_context_flush(&sscreen->aux_context.general);
   }

   return true;
}

static void si_resource_destroy(struct pipe_screen *screen, struct pipe_resource *buf)
{
   if (buf->target == PIPE_BUFFER) {
      struct si_screen *sscreen = (struct si_screen *)screen;
      struct si_resource *buffer = si_resource(buf);

      threaded_resource_deinit(buf);
      util_range_destroy(&buffer->valid_buffer_range);
      radeon_bo_reference(((struct si_screen*)screen)->ws, &buffer->buf, NULL);
      util_idalloc_mt_free(&sscreen->buffer_ids, buffer->b.buffer_id_unique);
      FREE_CL(buffer);
   } else if (buf->flags & SI_RESOURCE_AUX_PLANE) {
      struct si_auxiliary_texture *tex = (struct si_auxiliary_texture *)buf;

      radeon_bo_reference(((struct si_screen*)screen)->ws, &tex->buffer, NULL);
      FREE_CL(tex);
   } else {
      struct si_texture *tex = (struct si_texture *)buf;
      struct si_resource *resource = &tex->buffer;

      si_texture_reference(&tex->flushed_depth_texture, NULL);

      if (tex->cmask_buffer != &tex->buffer) {
         si_resource_reference(&tex->cmask_buffer, NULL);
      }
      radeon_bo_reference(((struct si_screen*)screen)->ws, &resource->buf, NULL);
      FREE_CL(tex);
   }
}

/* Reallocate the buffer a update all resource bindings where the buffer is
 * bound.
 *
 * This is used to avoid CPU-GPU synchronizations, because it makes the buffer
 * idle by discarding its contents.
 */
static bool si_invalidate_buffer(struct si_context *sctx, struct si_resource *buf)
{
   /* Shared buffers can't be reallocated. */
   if (buf->b.is_shared)
      return false;

   /* Sparse buffers can't be reallocated. */
   if (buf->flags & RADEON_FLAG_SPARSE)
      return false;

   /* In AMD_pinned_memory, the user pointer association only gets
    * broken when the buffer is explicitly re-allocated.
    */
   if (buf->b.is_user_ptr)
      return false;

   /* Check if mapping this buffer would cause waiting for the GPU. */
   if (si_cs_is_buffer_referenced(sctx, buf->buf, RADEON_USAGE_READWRITE) ||
       !sctx->ws->buffer_wait(sctx->ws, buf->buf, 0, RADEON_USAGE_READWRITE)) {
      /* Reallocate the buffer in the same pipe_resource. */
      si_alloc_resource(sctx->screen, buf);
      si_rebind_buffer(sctx, &buf->b.b);
   } else {
      util_range_set_empty(&buf->valid_buffer_range);
   }

   return true;
}

/* Replace the storage of dst with src. */
void si_replace_buffer_storage(struct pipe_context *ctx, struct pipe_resource *dst,
                               struct pipe_resource *src, unsigned num_rebinds, uint32_t rebind_mask,
                               uint32_t delete_buffer_id)
{
   struct si_context *sctx = (struct si_context *)ctx;
   struct si_resource *sdst = si_resource(dst);
   struct si_resource *ssrc = si_resource(src);

   radeon_bo_reference(sctx->screen->ws, &sdst->buf, ssrc->buf);
   sdst->gpu_address = ssrc->gpu_address;
   sdst->b.b.bind = ssrc->b.b.bind;
   sdst->flags = ssrc->flags;

   assert(sdst->bo_size == ssrc->bo_size);
   assert(sdst->bo_alignment_log2 == ssrc->bo_alignment_log2);
   assert(sdst->domains == ssrc->domains);

   si_rebind_buffer(sctx, dst);

   util_idalloc_mt_free(&sctx->screen->buffer_ids, delete_buffer_id);
}

static void si_invalidate_resource(struct pipe_context *ctx, struct pipe_resource *resource)
{
   struct si_context *sctx = (struct si_context *)ctx;
   struct si_resource *buf = si_resource(resource);

   /* We currently only do anything here for buffers */
   if (resource->target == PIPE_BUFFER)
      (void)si_invalidate_buffer(sctx, buf);
}

static void *si_buffer_get_transfer(struct pipe_context *ctx, struct pipe_resource *resource,
                                    unsigned usage, const struct pipe_box *box,
                                    struct pipe_transfer **ptransfer, void *data,
                                    struct si_resource *staging, unsigned offset)
{
   struct si_context *sctx = (struct si_context *)ctx;
   struct si_transfer *transfer;

   if (usage & PIPE_MAP_THREAD_SAFE)
      transfer = calloc(1, sizeof(*transfer));
   else if (usage & TC_TRANSFER_MAP_THREADED_UNSYNC)
      transfer = slab_zalloc(&sctx->pool_transfers_unsync);
   else
      transfer = slab_zalloc(&sctx->pool_transfers);

   pipe_resource_reference(&transfer->b.b.resource, resource);
   transfer->b.b.usage = usage;
   transfer->b.b.box = *box;
   transfer->b.b.offset = offset;
   transfer->staging = staging;
   *ptransfer = &transfer->b.b;
   return data;
}

static void *si_buffer_transfer_map(struct pipe_context *ctx, struct pipe_resource *resource,
                                    unsigned level, unsigned usage, const struct pipe_box *box,
                                    struct pipe_transfer **ptransfer)
{
   struct si_context *sctx = (struct si_context *)ctx;
   struct si_resource *buf = si_resource(resource);
   uint8_t *data;

   assert(resource->target == PIPE_BUFFER);
   assert(box->x + box->width <= resource->width0);

   /* From GL_AMD_pinned_memory issues:
    *
    *     4) Is glMapBuffer on a shared buffer guaranteed to return the
    *        same system address which was specified at creation time?
    *
    *        RESOLVED: NO. The GL implementation might return a different
    *        virtual mapping of that memory, although the same physical
    *        page will be used.
    *
    * So don't ever use staging buffers.
    */
   if (buf->b.is_user_ptr)
      usage |= PIPE_MAP_PERSISTENT;
   if (usage & PIPE_MAP_ONCE)
      usage |= RADEON_MAP_TEMPORARY;

   /* See if the buffer range being mapped has never been initialized,
    * in which case it can be mapped unsynchronized. */
   if (!(usage & (PIPE_MAP_UNSYNCHRONIZED | TC_TRANSFER_MAP_NO_INFER_UNSYNCHRONIZED)) &&
       usage & PIPE_MAP_WRITE && !buf->b.is_shared &&
       !util_ranges_intersect(&buf->valid_buffer_range, box->x, box->x + box->width)) {
      usage |= PIPE_MAP_UNSYNCHRONIZED;
   }

   /* If discarding the entire range, discard the whole resource instead. */
   if (usage & PIPE_MAP_DISCARD_RANGE && box->x == 0 && box->width == resource->width0) {
      usage |= PIPE_MAP_DISCARD_WHOLE_RESOURCE;
   }

   /* If a buffer in VRAM is too large and the range is discarded, don't
    * map it directly. This makes sure that the buffer stays in VRAM.
    */
   bool force_discard_range = false;
   if (usage & (PIPE_MAP_DISCARD_WHOLE_RESOURCE | PIPE_MAP_DISCARD_RANGE) &&
       !(usage & PIPE_MAP_PERSISTENT) &&
       buf->b.b.flags & PIPE_RESOURCE_FLAG_DONT_MAP_DIRECTLY) {
      usage &= ~(PIPE_MAP_DISCARD_WHOLE_RESOURCE | PIPE_MAP_UNSYNCHRONIZED);
      usage |= PIPE_MAP_DISCARD_RANGE;
      force_discard_range = true;
   }

   if (usage & PIPE_MAP_DISCARD_WHOLE_RESOURCE &&
       !(usage & (PIPE_MAP_UNSYNCHRONIZED | TC_TRANSFER_MAP_NO_INVALIDATE))) {
      assert(usage & PIPE_MAP_WRITE);

      if (si_invalidate_buffer(sctx, buf)) {
         /* At this point, the buffer is always idle. */
         usage |= PIPE_MAP_UNSYNCHRONIZED;
      } else {
         /* Fall back to a temporary buffer. */
         usage |= PIPE_MAP_DISCARD_RANGE;
      }
   }

   if (usage & PIPE_MAP_DISCARD_RANGE &&
       ((!(usage & (PIPE_MAP_UNSYNCHRONIZED | PIPE_MAP_PERSISTENT))) ||
        (buf->flags & RADEON_FLAG_SPARSE))) {
      assert(usage & PIPE_MAP_WRITE);

      /* Check if mapping this buffer would cause waiting for the GPU.
       */
      if (buf->flags & (RADEON_FLAG_SPARSE | RADEON_FLAG_NO_CPU_ACCESS) ||
          force_discard_range ||
          si_cs_is_buffer_referenced(sctx, buf->buf, RADEON_USAGE_READWRITE) ||
          !sctx->ws->buffer_wait(sctx->ws, buf->buf, 0, RADEON_USAGE_READWRITE)) {
         /* Do a wait-free write-only transfer using a temporary buffer. */
         struct u_upload_mgr *uploader;
         struct si_resource *staging = NULL;
         unsigned offset;

         /* If we are not called from the driver thread, we have
          * to use the uploader from u_threaded_context, which is
          * local to the calling thread.
          */
         if (usage & TC_TRANSFER_MAP_THREADED_UNSYNC)
            uploader = sctx->tc->base.stream_uploader;
         else
            uploader = sctx->b.stream_uploader;

         u_upload_alloc(uploader, 0, box->width + (box->x % SI_MAP_BUFFER_ALIGNMENT),
                        sctx->screen->info.tcc_cache_line_size, &offset,
                        (struct pipe_resource **)&staging, (void **)&data);

         if (staging) {
            data += box->x % SI_MAP_BUFFER_ALIGNMENT;
            return si_buffer_get_transfer(ctx, resource, usage, box, ptransfer, data, staging,
                                          offset);
         } else if (buf->flags & RADEON_FLAG_SPARSE) {
            return NULL;
         }
      } else {
         /* At this point, the buffer is always idle (we checked it above). */
         usage |= PIPE_MAP_UNSYNCHRONIZED;
      }
   }
   /* Use a staging buffer in cached GTT for reads. */
   else if (((usage & PIPE_MAP_READ) && !(usage & PIPE_MAP_PERSISTENT) &&
             (buf->domains & RADEON_DOMAIN_VRAM || buf->flags & RADEON_FLAG_GTT_WC)) ||
            (buf->flags & (RADEON_FLAG_SPARSE | RADEON_FLAG_NO_CPU_ACCESS))) {
      struct si_resource *staging;

      assert(!(usage & (TC_TRANSFER_MAP_THREADED_UNSYNC | PIPE_MAP_THREAD_SAFE)));
      staging = si_aligned_buffer_create(ctx->screen,
                                         SI_RESOURCE_FLAG_GL2_BYPASS | SI_RESOURCE_FLAG_DRIVER_INTERNAL,
                                         PIPE_USAGE_STAGING,
                                         box->width + (box->x % SI_MAP_BUFFER_ALIGNMENT), 256);
      if (staging) {
         /* Copy the VRAM buffer to the staging buffer. */
         si_copy_buffer(sctx, &staging->b.b, resource, box->x % SI_MAP_BUFFER_ALIGNMENT,
                        box->x, box->width, SI_OP_SYNC_BEFORE_AFTER);

         data = si_buffer_map(sctx, staging, usage & ~PIPE_MAP_UNSYNCHRONIZED);
         if (!data) {
            si_resource_reference(&staging, NULL);
            return NULL;
         }
         data += box->x % SI_MAP_BUFFER_ALIGNMENT;

         return si_buffer_get_transfer(ctx, resource, usage, box, ptransfer, data, staging, 0);
      } else if (buf->flags & RADEON_FLAG_SPARSE) {
         return NULL;
      }
   }

   data = si_buffer_map(sctx, buf, usage);
   if (!data) {
      return NULL;
   }
   data += box->x;

   return si_buffer_get_transfer(ctx, resource, usage, box, ptransfer, data, NULL, 0);
}

static void si_buffer_do_flush_region(struct pipe_context *ctx, struct pipe_transfer *transfer,
                                      const struct pipe_box *box)
{
   struct si_context *sctx = (struct si_context *)ctx;
   struct si_transfer *stransfer = (struct si_transfer *)transfer;
   struct si_resource *buf = si_resource(transfer->resource);

   if (stransfer->staging) {
      unsigned src_offset =
         stransfer->b.b.offset + transfer->box.x % SI_MAP_BUFFER_ALIGNMENT + (box->x - transfer->box.x);

      /* Copy the staging buffer into the original one. */
      si_copy_buffer(sctx, transfer->resource, &stransfer->staging->b.b, box->x, src_offset,
                     box->width, SI_OP_SYNC_BEFORE_AFTER);
   }

   util_range_add(&buf->b.b, &buf->valid_buffer_range, box->x, box->x + box->width);
}

static void si_buffer_flush_region(struct pipe_context *ctx, struct pipe_transfer *transfer,
                                   const struct pipe_box *rel_box)
{
   unsigned required_usage = PIPE_MAP_WRITE | PIPE_MAP_FLUSH_EXPLICIT;

   if ((transfer->usage & required_usage) == required_usage) {
      struct pipe_box box;

      u_box_1d(transfer->box.x + rel_box->x, rel_box->width, &box);
      si_buffer_do_flush_region(ctx, transfer, &box);
   }
}

static void si_buffer_transfer_unmap(struct pipe_context *ctx, struct pipe_transfer *transfer)
{
   struct si_context *sctx = (struct si_context *)ctx;
   struct si_transfer *stransfer = (struct si_transfer *)transfer;

   if (transfer->usage & PIPE_MAP_WRITE && !(transfer->usage & PIPE_MAP_FLUSH_EXPLICIT))
      si_buffer_do_flush_region(ctx, transfer, &transfer->box);

   if (transfer->usage & (PIPE_MAP_ONCE | RADEON_MAP_TEMPORARY) &&
       !stransfer->staging)
      sctx->ws->buffer_unmap(sctx->ws, si_resource(stransfer->b.b.resource)->buf);

   si_resource_reference(&stransfer->staging, NULL);
   assert(stransfer->b.staging == NULL); /* for threaded context only */
   pipe_resource_reference(&transfer->resource, NULL);

   if (transfer->usage & PIPE_MAP_THREAD_SAFE) {
      free(transfer);
   } else {
      /* Don't use pool_transfers_unsync. We are always in the driver
       * thread. Freeing an object into a different pool is allowed.
       */
      slab_free(&sctx->pool_transfers, transfer);
   }
}

static void si_buffer_subdata(struct pipe_context *ctx, struct pipe_resource *buffer,
                              unsigned usage, unsigned offset, unsigned size, const void *data)
{
   struct pipe_transfer *transfer = NULL;
   struct pipe_box box;
   uint8_t *map = NULL;

   usage |= PIPE_MAP_WRITE;

   if (!(usage & PIPE_MAP_DIRECTLY))
      usage |= PIPE_MAP_DISCARD_RANGE;

   u_box_1d(offset, size, &box);
   map = si_buffer_transfer_map(ctx, buffer, 0, usage, &box, &transfer);
   if (!map)
      return;

   memcpy(map, data, size);
   si_buffer_transfer_unmap(ctx, transfer);
}

static struct si_resource *si_alloc_buffer_struct(struct pipe_screen *screen,
                                                  const struct pipe_resource *templ,
                                                  bool allow_cpu_storage)
{
   struct si_resource *buf = MALLOC_STRUCT_CL(si_resource);

   buf->b.b = *templ;
   buf->b.b.next = NULL;
   pipe_reference_init(&buf->b.b.reference, 1);
   buf->b.b.screen = screen;

   threaded_resource_init(&buf->b.b, allow_cpu_storage);

   buf->buf = NULL;
   buf->bind_history = 0;
   buf->TC_L2_dirty = false;
   util_range_init(&buf->valid_buffer_range);
   return buf;
}

static struct pipe_resource *si_buffer_create(struct pipe_screen *screen,
                                              const struct pipe_resource *templ, unsigned alignment)
{
   struct si_screen *sscreen = (struct si_screen *)screen;
   struct si_resource *buf =
      si_alloc_buffer_struct(screen, templ,
                             templ->width0 <= sscreen->options.tc_max_cpu_storage_size);

   if (templ->flags & PIPE_RESOURCE_FLAG_SPARSE)
      buf->b.b.flags |= PIPE_RESOURCE_FLAG_UNMAPPABLE;

   si_init_resource_fields(sscreen, buf, templ->width0, alignment);

   buf->b.buffer_id_unique = util_idalloc_mt_alloc(&sscreen->buffer_ids);

   if (!si_alloc_resource(sscreen, buf)) {
      si_resource_destroy(screen, &buf->b.b);
      return NULL;
   }

   return &buf->b.b;
}

struct pipe_resource *pipe_aligned_buffer_create(struct pipe_screen *screen, unsigned flags,
                                                 unsigned usage, unsigned size, unsigned alignment)
{
   struct pipe_resource buffer;

   memset(&buffer, 0, sizeof buffer);
   buffer.target = PIPE_BUFFER;
   buffer.format = PIPE_FORMAT_R8_UNORM;
   buffer.bind = 0;
   buffer.usage = usage;
   buffer.flags = flags;
   buffer.width0 = size;
   buffer.height0 = 1;
   buffer.depth0 = 1;
   buffer.array_size = 1;
   return si_buffer_create(screen, &buffer, alignment);
}

struct si_resource *si_aligned_buffer_create(struct pipe_screen *screen, unsigned flags,
                                             unsigned usage, unsigned size, unsigned alignment)
{
   return si_resource(pipe_aligned_buffer_create(screen, flags, usage, size, alignment));
}

static struct pipe_resource *si_buffer_from_user_memory(struct pipe_screen *screen,
                                                        const struct pipe_resource *templ,
                                                        void *user_memory)
{
   if (templ->target != PIPE_BUFFER)
      return NULL;

   struct si_screen *sscreen = (struct si_screen *)screen;
   struct radeon_winsys *ws = sscreen->ws;
   struct si_resource *buf = si_alloc_buffer_struct(screen, templ, false);

   buf->domains = RADEON_DOMAIN_GTT;
   buf->flags = 0;
   buf->b.is_user_ptr = true;
   util_range_add(&buf->b.b, &buf->valid_buffer_range, 0, templ->width0);
   util_range_add(&buf->b.b, &buf->b.valid_buffer_range, 0, templ->width0);

   buf->b.buffer_id_unique = util_idalloc_mt_alloc(&sscreen->buffer_ids);

   /* Convert a user pointer to a buffer. */
   buf->buf = ws->buffer_from_ptr(ws, user_memory, templ->width0, 0);
   if (!buf->buf) {
      si_resource_destroy(screen, &buf->b.b);
      return NULL;
   }

   buf->gpu_address = ws->buffer_get_virtual_address(buf->buf);
   return &buf->b.b;
}

struct pipe_resource *si_buffer_from_winsys_buffer(struct pipe_screen *screen,
                                                   const struct pipe_resource *templ,
                                                   struct pb_buffer_lean *imported_buf,
                                                   uint64_t offset)
{
   if (offset + templ->width0 > imported_buf->size)
      return NULL;

   struct si_screen *sscreen = (struct si_screen *)screen;
   struct si_resource *res = si_alloc_buffer_struct(screen, templ, false);

   if (!res)
      return NULL;

   enum radeon_bo_domain domains = sscreen->ws->buffer_get_initial_domain(imported_buf);

   /* Get or guess the BO flags. */
   unsigned flags = RADEON_FLAG_NO_SUBALLOC;

   if (sscreen->ws->buffer_get_flags)
      res->flags |= sscreen->ws->buffer_get_flags(imported_buf);
   else
      flags |= RADEON_FLAG_GTT_WC; /* unknown flags, guess them */

   /* Deduce the usage. */
   switch (domains) {
   case RADEON_DOMAIN_VRAM:
   case RADEON_DOMAIN_VRAM_GTT:
      res->b.b.usage = PIPE_USAGE_DEFAULT;
      break;

   default:
      /* Other values are interpreted as GTT. */
      domains = RADEON_DOMAIN_GTT;

      if (flags & RADEON_FLAG_GTT_WC)
         res->b.b.usage = PIPE_USAGE_STREAM;
      else
         res->b.b.usage = PIPE_USAGE_STAGING;
   }

   si_init_resource_fields(sscreen, res, imported_buf->size,
                           1 << imported_buf->alignment_log2);

   res->b.is_shared = true;
   res->b.buffer_id_unique = util_idalloc_mt_alloc(&sscreen->buffer_ids);
   res->buf = imported_buf;
   res->gpu_address = sscreen->ws->buffer_get_virtual_address(res->buf) + offset;
   res->domains = domains;
   res->flags = flags;

   if (res->flags & RADEON_FLAG_NO_CPU_ACCESS)
      res->b.b.flags |= PIPE_RESOURCE_FLAG_UNMAPPABLE;

   util_range_add(&res->b.b, &res->valid_buffer_range, 0, templ->width0);
   util_range_add(&res->b.b, &res->b.valid_buffer_range, 0, templ->width0);

   return &res->b.b;
}

static struct pipe_resource *si_resource_create(struct pipe_screen *screen,
                                                const struct pipe_resource *templ)
{
   if (templ->target == PIPE_BUFFER) {
      return si_buffer_create(screen, templ, 256);
   } else {
      return si_texture_create(screen, templ);
   }
}

static bool si_buffer_commit(struct si_context *ctx, struct si_resource *res,
                             struct pipe_box *box, bool commit)
{
   return ctx->ws->buffer_commit(ctx->ws, res->buf, box->x, box->width, commit);
}

static bool si_resource_commit(struct pipe_context *pctx, struct pipe_resource *resource,
                               unsigned level, struct pipe_box *box, bool commit)
{
   struct si_context *ctx = (struct si_context *)pctx;
   struct si_resource *res = si_resource(resource);

   /*
    * Since buffer commitment changes cannot be pipelined, we need to
    * (a) flush any pending commands that refer to the buffer we're about
    *     to change, and
    * (b) wait for threaded submit to finish, including those that were
    *     triggered by some other, earlier operation.
    */
   if (radeon_emitted(&ctx->gfx_cs, ctx->initial_gfx_cs_size) &&
       ctx->ws->cs_is_buffer_referenced(&ctx->gfx_cs, res->buf, RADEON_USAGE_READWRITE)) {
      si_flush_gfx_cs(ctx, RADEON_FLUSH_ASYNC_START_NEXT_GFX_IB_NOW, NULL);
   }
   ctx->ws->cs_sync_flush(&ctx->gfx_cs);

   if (resource->target == PIPE_BUFFER)
      return si_buffer_commit(ctx, res, box, commit);
   else
      return si_texture_commit(ctx, res, level, box, commit);
}

void si_init_screen_buffer_functions(struct si_screen *sscreen)
{
   sscreen->b.resource_create = si_resource_create;
   sscreen->b.resource_destroy = si_resource_destroy;
   sscreen->b.resource_from_user_memory = si_buffer_from_user_memory;
}

void si_init_buffer_functions(struct si_context *sctx)
{
   sctx->b.invalidate_resource = si_invalidate_resource;
   sctx->b.buffer_map = si_buffer_transfer_map;
   sctx->b.transfer_flush_region = si_buffer_flush_region;
   sctx->b.buffer_unmap = si_buffer_transfer_unmap;
   sctx->b.texture_subdata = u_default_texture_subdata;
   sctx->b.buffer_subdata = si_buffer_subdata;
   sctx->b.resource_commit = si_resource_commit;
}
