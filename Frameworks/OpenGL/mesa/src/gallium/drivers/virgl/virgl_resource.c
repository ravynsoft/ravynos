/*
 * Copyright 2014, 2015 Red Hat.
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
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "util/format/u_format.h"
#include "util/u_inlines.h"
#include "util/u_memory.h"
#include "util/u_upload_mgr.h"
#include "virgl_context.h"
#include "virgl_resource.h"
#include "virgl_screen.h"
#include "virgl_staging_mgr.h"
#include "virgl_encode.h" // for declaration of virgl_encode_copy_transfer

/* A (soft) limit for the amount of memory we want to allow for queued staging
 * resources. This is used to decide when we should force a flush, in order to
 * avoid exhausting virtio-gpu memory.
 */
#define VIRGL_QUEUED_STAGING_RES_SIZE_LIMIT (128 * 1024 * 1024)

enum virgl_transfer_map_type {
   VIRGL_TRANSFER_MAP_ERROR = -1,
   VIRGL_TRANSFER_MAP_HW_RES,

   /* Map a range of a staging buffer. The updated contents should be transferred
    * with a copy transfer.
    */
   VIRGL_TRANSFER_MAP_WRITE_TO_STAGING,

   /* Reallocate the underlying virgl_hw_res. */
   VIRGL_TRANSFER_MAP_REALLOC,

   /* Map type for read of texture data from host to guest
    * using staging buffer. */
   VIRGL_TRANSFER_MAP_READ_FROM_STAGING,
   /* Map type for write of texture data to host using staging
    * buffer that needs a readback first. */
   VIRGL_TRANSFER_MAP_WRITE_TO_STAGING_WITH_READBACK,
};

/* Check if copy transfer from host can be used:
 *  1. if resource is a texture,
 *  2. if renderer supports copy transfer from host,
 *  3. the host is not GLES (no fake FP64)
 *  4. the format can be rendered to and the format is a readback format
 *     or the format is a scanout format and we can read back from scanout
 */
static bool virgl_can_readback_from_rendertarget(struct virgl_screen *vs,
                                                 struct virgl_resource *res)
{
   return res->b.nr_samples < 2 &&
         vs->base.is_format_supported(&vs->base, res->b.format, res->b.target,
                                      res->b.nr_samples, res->b.nr_samples,
                                      PIPE_BIND_RENDER_TARGET);
}

static bool virgl_can_readback_from_scanout(struct virgl_screen *vs,
                                            struct virgl_resource *res,
                                            int bind)
{
   return (vs->caps.caps.v2.capability_bits_v2 & VIRGL_CAP_V2_SCANOUT_USES_GBM) &&
         (bind & VIRGL_BIND_SCANOUT) &&
         virgl_has_scanout_format(vs, res->b.format, true);
}

static bool virgl_can_use_staging(struct virgl_screen *vs,
                                  struct virgl_resource *res)
{
   return (vs->caps.caps.v2.capability_bits_v2 & VIRGL_CAP_V2_COPY_TRANSFER_BOTH_DIRECTIONS) &&
         (res->b.target != PIPE_BUFFER);
}

static bool is_stencil_array(struct virgl_resource *res)
{
   const struct util_format_description *descr = util_format_description(res->b.format);
   return (res->b.array_size > 1 || res->b.depth0 > 1) && util_format_has_stencil(descr);
}

static bool virgl_can_copy_transfer_from_host(struct virgl_screen *vs,
                                              struct virgl_resource *res,
                                              int bind)
{
   return virgl_can_use_staging(vs, res) &&
         !is_stencil_array(res) &&
         !(bind & VIRGL_BIND_SHARED) &&
         virgl_has_readback_format(&vs->base, pipe_to_virgl_format(res->b.format), false) &&
         ((!(vs->caps.caps.v2.capability_bits & VIRGL_CAP_HOST_IS_GLES)) ||
          virgl_can_readback_from_rendertarget(vs, res) ||
          virgl_can_readback_from_scanout(vs, res, bind));
}

/* We need to flush to properly sync the transfer with the current cmdbuf.
 * But there are cases where the flushing can be skipped:
 *
 *  - synchronization is disabled
 *  - the resource is not referenced by the current cmdbuf
 */
static bool virgl_res_needs_flush(struct virgl_context *vctx,
                                  struct virgl_transfer *trans)
{
   struct virgl_winsys *vws = virgl_screen(vctx->base.screen)->vws;
   struct virgl_resource *res = virgl_resource(trans->base.resource);

   if (trans->base.usage & PIPE_MAP_UNSYNCHRONIZED)
      return false;

   if (!vws->res_is_referenced(vws, vctx->cbuf, res->hw_res))
      return false;

   return true;
}

/* We need to read back from the host storage to make sure the guest storage
 * is up-to-date.  But there are cases where the readback can be skipped:
 *
 *  - the content can be discarded
 *  - the host storage is read-only
 *
 * Note that PIPE_MAP_WRITE without discard bits requires readback.
 * PIPE_MAP_READ becomes irrelevant.  PIPE_MAP_UNSYNCHRONIZED and
 * PIPE_MAP_FLUSH_EXPLICIT are also irrelevant.
 */
static bool virgl_res_needs_readback(struct virgl_context *vctx,
                                     struct virgl_resource *res,
                                     unsigned usage, unsigned level)
{
   if (usage & (PIPE_MAP_DISCARD_RANGE |
                PIPE_MAP_DISCARD_WHOLE_RESOURCE))
      return false;

   if (res->clean_mask & (1 << level))
      return false;

   return true;
}

static enum virgl_transfer_map_type
virgl_resource_transfer_prepare(struct virgl_context *vctx,
                                struct virgl_transfer *xfer,
                                bool is_blob)
{
   struct virgl_screen *vs = virgl_screen(vctx->base.screen);
   struct virgl_winsys *vws = vs->vws;
   struct virgl_resource *res = virgl_resource(xfer->base.resource);
   enum virgl_transfer_map_type map_type = VIRGL_TRANSFER_MAP_HW_RES;
   bool flush;
   bool readback;
   bool wait;

   /* there is no way to map the host storage currently */
   if (xfer->base.usage & PIPE_MAP_DIRECTLY)
      return VIRGL_TRANSFER_MAP_ERROR;

   /* We break the logic down into four steps
    *
    * step 1: determine the required operations independently
    * step 2: look for chances to skip the operations
    * step 3: resolve dependencies between the operations
    * step 4: execute the operations
    */

   flush = virgl_res_needs_flush(vctx, xfer);
   readback = virgl_res_needs_readback(vctx, res, xfer->base.usage,
                                       xfer->base.level);
   /* We need to wait for all cmdbufs, current or previous, that access the
    * resource to finish unless synchronization is disabled.
    */
   wait = !(xfer->base.usage & PIPE_MAP_UNSYNCHRONIZED);

   /* When the transfer range consists of only uninitialized data, we can
    * assume the GPU is not accessing the range and readback is unnecessary.
    * We can proceed as if PIPE_MAP_UNSYNCHRONIZED and
    * PIPE_MAP_DISCARD_RANGE are set.
    */
   if (res->b.target == PIPE_BUFFER &&
       !util_ranges_intersect(&res->valid_buffer_range, xfer->base.box.x,
                              xfer->base.box.x + xfer->base.box.width) &&
       likely(!(virgl_debug & VIRGL_DEBUG_XFER))) {
      flush = false;
      readback = false;
      wait = false;
   }

   /* When the resource is busy but its content can be discarded, we can
    * replace its HW resource or use a staging buffer to avoid waiting.
    */
   if (wait && !is_blob &&
       (xfer->base.usage & (PIPE_MAP_DISCARD_RANGE |
                            PIPE_MAP_DISCARD_WHOLE_RESOURCE)) &&
       likely(!(virgl_debug & VIRGL_DEBUG_XFER))) {
      bool can_realloc = false;

      /* A PIPE_MAP_DISCARD_WHOLE_RESOURCE transfer may be followed by
       * PIPE_MAP_UNSYNCHRONIZED transfers to non-overlapping regions.
       * It cannot be treated as a PIPE_MAP_DISCARD_RANGE transfer,
       * otherwise those following unsynchronized transfers may overwrite
       * valid data.
       */
      if (xfer->base.usage & PIPE_MAP_DISCARD_WHOLE_RESOURCE) {
         can_realloc = virgl_can_rebind_resource(vctx, &res->b);
      }

      /* discard implies no readback */
      assert(!readback);

      if (can_realloc || vctx->supports_staging) {
         /* Both map types have some costs.  Do them only when the resource is
          * (or will be) busy for real.  Otherwise, set wait to false.
          */
         wait = (flush || vws->resource_is_busy(vws, res->hw_res));
         if (wait) {
            map_type = (can_realloc) ?
               VIRGL_TRANSFER_MAP_REALLOC :
               VIRGL_TRANSFER_MAP_WRITE_TO_STAGING;

            wait = false;

            /* There is normally no need to flush either, unless the amount of
             * memory we are using for staging resources starts growing, in
             * which case we want to flush to keep our memory consumption in
             * check.
             */
            flush = (vctx->queued_staging_res_size >
               VIRGL_QUEUED_STAGING_RES_SIZE_LIMIT);
         }
      }
   }

   /* readback has some implications */
   if (readback) {
      /* If we are performing readback for textures and renderer supports
       * copy_transfer_from_host, then we can return here with proper map.
       */
      if (res->use_staging) {
         if (xfer->base.usage & PIPE_MAP_READ)
            return VIRGL_TRANSFER_MAP_READ_FROM_STAGING;
         else
            return VIRGL_TRANSFER_MAP_WRITE_TO_STAGING_WITH_READBACK;
      }

      /* When the transfer queue has pending writes to this transfer's region,
       * we have to flush before readback.
       */
      if (!flush && virgl_transfer_queue_is_queued(&vctx->queue, xfer))
         flush = true;
   }

   if (flush)
      vctx->base.flush(&vctx->base, NULL, 0);

   /* If we are not allowed to block, and we know that we will have to wait,
    * either because the resource is busy, or because it will become busy due
    * to a readback, return early to avoid performing an incomplete
    * transfer_get. Such an incomplete transfer_get may finish at any time,
    * during which another unsynchronized map could write to the resource
    * contents, leaving the contents in an undefined state.
    */
   if ((xfer->base.usage & PIPE_MAP_DONTBLOCK) &&
       (readback || (wait && vws->resource_is_busy(vws, res->hw_res))))
      return VIRGL_TRANSFER_MAP_ERROR;

   if (readback) {
      /* Readback is yet another command and is transparent to the state
       * trackers.  It should be waited for in all cases, including when
       * PIPE_MAP_UNSYNCHRONIZED is set.
       */
      if (!is_blob) {
         vws->resource_wait(vws, res->hw_res);
         vws->transfer_get(vws, res->hw_res, &xfer->base.box, xfer->base.stride,
                           xfer->l_stride, xfer->offset, xfer->base.level);
      }
      /* transfer_get puts the resource into a maybe_busy state, so we will have
       * to wait another time if we want to use that resource. */
      wait = true;
   }

   if (wait)
      vws->resource_wait(vws, res->hw_res);

   if (res->use_staging) {
      map_type = VIRGL_TRANSFER_MAP_WRITE_TO_STAGING;
   }

   return map_type;
}

/* Calculate the minimum size of the memory required to service a resource
 * transfer map. Also return the stride and layer_stride for the corresponding
 * layout.
 */
static unsigned
virgl_transfer_map_size(struct virgl_transfer *vtransfer,
                        unsigned *out_stride,
                        uintptr_t *out_layer_stride)
{
   struct pipe_resource *pres = vtransfer->base.resource;
   struct pipe_box *box = &vtransfer->base.box;
   unsigned stride;
   uintptr_t layer_stride;
   unsigned size;

   assert(out_stride);
   assert(out_layer_stride);

   stride = util_format_get_stride(pres->format, box->width);
   layer_stride = util_format_get_2d_size(pres->format, stride, box->height);

   if (pres->target == PIPE_TEXTURE_CUBE ||
       pres->target == PIPE_TEXTURE_CUBE_ARRAY ||
       pres->target == PIPE_TEXTURE_3D ||
       pres->target == PIPE_TEXTURE_2D_ARRAY) {
      size = box->depth * layer_stride;
   } else if (pres->target == PIPE_TEXTURE_1D_ARRAY) {
      size = box->depth * stride;
   } else {
      size = layer_stride;
   }

   *out_stride = stride;
   *out_layer_stride = layer_stride;

   return size;
}

/* Maps a region from staging to service the transfer. */
static void *
virgl_staging_map(struct virgl_context *vctx,
                  struct virgl_transfer *vtransfer)
{
   struct virgl_resource *vres = virgl_resource(vtransfer->base.resource);
   unsigned size;
   unsigned align_offset;
   unsigned stride;
   uintptr_t layer_stride;
   void *map_addr;
   bool alloc_succeeded;

   assert(vctx->supports_staging);

   size = virgl_transfer_map_size(vtransfer, &stride, &layer_stride);

   /* For buffers we need to ensure that the start of the buffer would be
    * aligned to VIRGL_MAP_BUFFER_ALIGNMENT, even if our transfer doesn't
    * actually include it. To achieve this we may need to allocate a slightly
    * larger range from the upload buffer, and later update the uploader
    * resource offset and map address to point to the requested x coordinate
    * within that range.
    *
    * 0       A       2A      3A
    * |-------|---bbbb|bbbbb--|
    *             |--------|    ==> size
    *         |---|             ==> align_offset
    *         |------------|    ==> allocation of size + align_offset
    */
   align_offset = vres->b.target == PIPE_BUFFER ?
                  vtransfer->base.box.x % VIRGL_MAP_BUFFER_ALIGNMENT :
                  0;

   alloc_succeeded =
      virgl_staging_alloc(&vctx->staging, size + align_offset,
                          VIRGL_MAP_BUFFER_ALIGNMENT,
                          &vtransfer->copy_src_offset,
                          &vtransfer->copy_src_hw_res,
                          &map_addr);
   if (alloc_succeeded) {
      /* Update source offset and address to point to the requested x coordinate
       * if we have an align_offset (see above for more information). */
      vtransfer->copy_src_offset += align_offset;
      map_addr += align_offset;

      /* Mark as dirty, since we are updating the host side resource
       * without going through the corresponding guest side resource, and
       * hence the two will diverge.
       */
      virgl_resource_dirty(vres, vtransfer->base.level);

      /* We are using the minimum required size to hold the contents,
       * possibly using a layout different from the layout of the resource,
       * so update the transfer strides accordingly.
       */
      vtransfer->base.stride = stride;
      vtransfer->base.layer_stride = layer_stride;

      /* Track the total size of active staging resources. */
      vctx->queued_staging_res_size += size + align_offset;
   }

   return map_addr;
}

/* Maps a region from staging to service the transfer from host.
 * This function should be called only for texture readbacks
 * from host. */
static void *
virgl_staging_read_map(struct virgl_context *vctx,
                  struct virgl_transfer *vtransfer)
{
   struct virgl_screen *vscreen = virgl_screen(vctx->base.screen);
   struct virgl_winsys *vws = vscreen->vws;
   assert(vtransfer->base.resource->target != PIPE_BUFFER);
   void *map_addr;

   /* There are two possibilities to perform readback via:
    * a) calling transfer_get();
    * b) calling submit_cmd() with encoded transfer inside cmd.
    * 
    * For b) we need:
    *   1. select offset from staging buffer
    *   2. encode this transfer in wire
    *   3. flush the execbuffer to the host
    *   4. wait till copy on the host is done
    */
   map_addr = virgl_staging_map(vctx, vtransfer);
   vtransfer->direction = VIRGL_TRANSFER_FROM_HOST;
   virgl_encode_copy_transfer(vctx, vtransfer);
   vctx->base.flush(&vctx->base, NULL, 0);
   vws->resource_wait(vws, vtransfer->copy_src_hw_res);
   return map_addr;
}

static bool
virgl_resource_realloc(struct virgl_context *vctx, struct virgl_resource *res)
{
   struct virgl_screen *vs = virgl_screen(vctx->base.screen);
   const struct pipe_resource *templ = &res->b;
   unsigned vbind, vflags;
   struct virgl_hw_res *hw_res;

   vbind = pipe_to_virgl_bind(vs, templ->bind);
   vflags = pipe_to_virgl_flags(vs, templ->flags);

   int alloc_size = res->use_staging ? 1 : res->metadata.total_size;

   hw_res = vs->vws->resource_create(vs->vws,
                                     templ->target,
                                     NULL,
                                     templ->format,
                                     vbind,
                                     templ->width0,
                                     templ->height0,
                                     templ->depth0,
                                     templ->array_size,
                                     templ->last_level,
                                     templ->nr_samples,
                                     vflags,
                                     alloc_size);
   if (!hw_res)
      return false;

   vs->vws->resource_reference(vs->vws, &res->hw_res, NULL);
   res->hw_res = hw_res;

   /* We can safely clear the range here, since it will be repopulated in the
    * following rebind operation, according to the active buffer binds.
    */
   util_range_set_empty(&res->valid_buffer_range);

   /* count toward the staging resource size limit */
   vctx->queued_staging_res_size += res->metadata.total_size;

   virgl_rebind_resource(vctx, &res->b);

   return true;
}

void *
virgl_resource_transfer_map(struct pipe_context *ctx,
                            struct pipe_resource *resource,
                            unsigned level,
                            unsigned usage,
                            const struct pipe_box *box,
                            struct pipe_transfer **transfer)
{
   struct virgl_context *vctx = virgl_context(ctx);
   struct virgl_screen *vscreen = virgl_screen(ctx->screen);
   struct virgl_winsys *vws = vscreen->vws;
   struct virgl_resource *vres = virgl_resource(resource);
   struct virgl_transfer *trans;
   enum virgl_transfer_map_type map_type;
   void *map_addr;

   /* Multisampled resources require resolve before mapping. */
   assert(resource->nr_samples <= 1);

   /* If virgl resource was created using persistence and coherency flags,
    * then its memory mapping can be only made in accordance to these
    * flags. We record the "usage" flags in struct virgl_transfer and
    * then virgl_buffer_transfer_unmap() uses them to differentiate
    * unmapping of a host blob resource from guest.
    */
   if (resource->flags & PIPE_RESOURCE_FLAG_MAP_PERSISTENT)
      usage |= PIPE_MAP_PERSISTENT;

   if (resource->flags & PIPE_RESOURCE_FLAG_MAP_COHERENT)
      usage |= PIPE_MAP_COHERENT;

   bool is_blob = usage & (PIPE_MAP_COHERENT | PIPE_MAP_PERSISTENT);

   trans = virgl_resource_create_transfer(vctx, resource,
                                          &vres->metadata, level, usage, box);

   map_type = virgl_resource_transfer_prepare(vctx, trans, is_blob);
   switch (map_type) {
   case VIRGL_TRANSFER_MAP_REALLOC:
      if (!virgl_resource_realloc(vctx, vres)) {
         map_addr = NULL;
         break;
      }
      vws->resource_reference(vws, &trans->hw_res, vres->hw_res);
      FALLTHROUGH;
   case VIRGL_TRANSFER_MAP_HW_RES:
      trans->hw_res_map = vws->resource_map(vws, vres->hw_res);
      if (trans->hw_res_map)
         map_addr = trans->hw_res_map + trans->offset;
      else
         map_addr = NULL;
      break;
   case VIRGL_TRANSFER_MAP_WRITE_TO_STAGING:
      map_addr = virgl_staging_map(vctx, trans);
      /* Copy transfers don't make use of hw_res_map at the moment. */
      trans->hw_res_map = NULL;
      trans->direction = VIRGL_TRANSFER_TO_HOST;
      break;
   case VIRGL_TRANSFER_MAP_READ_FROM_STAGING:
      map_addr = virgl_staging_read_map(vctx, trans);
      /* Copy transfers don't make use of hw_res_map at the moment. */
      trans->hw_res_map = NULL;
      break;
   case VIRGL_TRANSFER_MAP_WRITE_TO_STAGING_WITH_READBACK:
      map_addr = virgl_staging_read_map(vctx, trans);
      /* Copy transfers don't make use of hw_res_map at the moment. */
      trans->hw_res_map = NULL;
      trans->direction = VIRGL_TRANSFER_TO_HOST;
      break;
   case VIRGL_TRANSFER_MAP_ERROR:
   default:
      trans->hw_res_map = NULL;
      map_addr = NULL;
      break;
   }

   if (!map_addr) {
      virgl_resource_destroy_transfer(vctx, trans);
      return NULL;
   }

   if (vres->b.target == PIPE_BUFFER) {
      /* For the checks below to be able to use 'usage', we assume that
       * transfer preparation doesn't affect the usage.
       */
      assert(usage == trans->base.usage);

      /* If we are doing a whole resource discard with a hw_res map, the buffer
       * storage can now be considered unused and we don't care about previous
       * contents.  We can thus mark the storage as uninitialized, but only if
       * the buffer is not host writable (in which case we can't clear the
       * valid range, since that would result in missed readbacks in future
       * transfers).  We only do this for VIRGL_TRANSFER_MAP_HW_RES, since for
       * VIRGL_TRANSFER_MAP_REALLOC we already take care of the buffer range
       * when reallocating and rebinding, and VIRGL_TRANSFER_MAP_STAGING is not
       * currently used for whole resource discards.
       */
      if (map_type == VIRGL_TRANSFER_MAP_HW_RES &&
          (usage & PIPE_MAP_DISCARD_WHOLE_RESOURCE) &&
          (vres->clean_mask & 1)) {
         util_range_set_empty(&vres->valid_buffer_range);
      }

      if (usage & PIPE_MAP_WRITE)
          util_range_add(&vres->b, &vres->valid_buffer_range, box->x, box->x + box->width);
   }

   *transfer = &trans->base;
   return map_addr;
}

static void virgl_resource_layout(struct pipe_resource *pt,
                                  struct virgl_resource_metadata *metadata,
                                  uint32_t plane,
                                  uint32_t winsys_stride,
                                  uint32_t plane_offset,
                                  uint64_t modifier)
{
   unsigned level, nblocksy;
   unsigned width = pt->width0;
   unsigned height = pt->height0;
   unsigned depth = pt->depth0;
   unsigned buffer_size = 0;

   for (level = 0; level <= pt->last_level; level++) {
      unsigned slices;

      if (pt->target == PIPE_TEXTURE_CUBE)
         slices = 6;
      else if (pt->target == PIPE_TEXTURE_3D)
         slices = depth;
      else
         slices = pt->array_size;

      nblocksy = util_format_get_nblocksy(pt->format, height);
      metadata->stride[level] = winsys_stride ? winsys_stride :
                                util_format_get_stride(pt->format, width);
      metadata->layer_stride[level] = nblocksy * metadata->stride[level];
      metadata->level_offset[level] = buffer_size;

      buffer_size += slices * metadata->layer_stride[level];

      width = u_minify(width, 1);
      height = u_minify(height, 1);
      depth = u_minify(depth, 1);
   }

   metadata->plane = plane;
   metadata->plane_offset = plane_offset;
   metadata->modifier = modifier;
   if (pt->nr_samples <= 1)
      metadata->total_size = buffer_size;
   else /* don't create guest backing store for MSAA */
      metadata->total_size = 0;
}

static struct pipe_resource *virgl_resource_create_front(struct pipe_screen *screen,
                                                         const struct pipe_resource *templ,
                                                         const void *map_front_private)
{
   unsigned vbind, vflags;
   struct virgl_screen *vs = virgl_screen(screen);
   struct virgl_resource *res = CALLOC_STRUCT(virgl_resource);
   uint32_t alloc_size;

   res->b = *templ;
   res->b.screen = &vs->base;
   pipe_reference_init(&res->b.reference, 1);
   vbind = pipe_to_virgl_bind(vs, templ->bind);
   vflags = pipe_to_virgl_flags(vs, templ->flags);
   virgl_resource_layout(&res->b, &res->metadata, 0, 0, 0, 0);

   if ((vs->caps.caps.v2.capability_bits & VIRGL_CAP_APP_TWEAK_SUPPORT) &&
       vs->tweak_gles_emulate_bgra &&
      (templ->format == PIPE_FORMAT_B8G8R8A8_SRGB ||
        templ->format == PIPE_FORMAT_B8G8R8A8_UNORM ||
        templ->format == PIPE_FORMAT_B8G8R8X8_SRGB ||
        templ->format == PIPE_FORMAT_B8G8R8X8_UNORM)) {
      vbind |= VIRGL_BIND_PREFER_EMULATED_BGRA;
   }

   // If renderer supports copy transfer from host, and we either have support
   // for then for textures alloc minimum size of bo
   // This size is not passed to the host
   res->use_staging = virgl_can_copy_transfer_from_host(vs, res, vbind);

   if (res->use_staging)
      alloc_size = 1;
   else
      alloc_size = res->metadata.total_size;
   
   res->hw_res = vs->vws->resource_create(vs->vws, templ->target,
                                          map_front_private,
                                          templ->format, vbind,
                                          templ->width0,
                                          templ->height0,
                                          templ->depth0,
                                          templ->array_size,
                                          templ->last_level,
                                          templ->nr_samples,
                                          vflags,
                                          alloc_size);
   if (!res->hw_res) {
      FREE(res);
      return NULL;
   }

   res->clean_mask = (1 << VR_MAX_TEXTURE_2D_LEVELS) - 1;

   if (templ->target == PIPE_BUFFER) {
      util_range_init(&res->valid_buffer_range);
      virgl_buffer_init(res);
   } else {
      virgl_texture_init(res);
   }

   return &res->b;

}

static struct pipe_resource *virgl_resource_create(struct pipe_screen *screen,
                                                   const struct pipe_resource *templ)
{
   return virgl_resource_create_front(screen, templ, NULL);
}

static struct pipe_resource *virgl_resource_from_handle(struct pipe_screen *screen,
                                                        const struct pipe_resource *templ,
                                                        struct winsys_handle *whandle,
                                                        unsigned usage)
{
   uint32_t winsys_stride, plane_offset, plane;
   uint64_t modifier;
   uint32_t storage_size;

   struct virgl_screen *vs = virgl_screen(screen);
   if (templ->target == PIPE_BUFFER)
      return NULL;

   struct virgl_resource *res = CALLOC_STRUCT(virgl_resource);
   res->b = *templ;
   res->b.screen = &vs->base;
   pipe_reference_init(&res->b.reference, 1);

   plane = winsys_stride = plane_offset = modifier = 0;
   res->hw_res = vs->vws->resource_create_from_handle(vs->vws, whandle,
                                                      &plane,
                                                      &winsys_stride,
                                                      &plane_offset,
                                                      &modifier,
                                                      &res->blob_mem);

   /* do not use winsys returns for guest storage info of classic resource */
   if (!res->blob_mem) {
      winsys_stride = 0;
      plane_offset = 0;
      modifier = 0;
   }

   virgl_resource_layout(&res->b, &res->metadata, plane, winsys_stride,
                         plane_offset, modifier);
   if (!res->hw_res) {
      FREE(res);
      return NULL;
   }

   /*
   *  If the overall resource is larger than a single page in size, we can
   *  compare it with the amount of memory allocated on the guest to determine
   *  if we should be using the staging path.
   *
   *  If not, the decision is not as clear. However, since the resource can
   *  fit within a single page, the import will function correctly.
   */
  storage_size = vs->vws->resource_get_storage_size(vs->vws, res->hw_res);

   if (res->metadata.total_size > storage_size)
      res->use_staging = 1;

   /* assign blob resource a type in case it was created untyped */
   if (res->blob_mem && plane == 0 &&
       (vs->caps.caps.v2.host_feature_check_version >= 18 ||
	(vs->caps.caps.v2.capability_bits_v2 & VIRGL_CAP_V2_UNTYPED_RESOURCE))) {
      uint32_t plane_strides[VIRGL_MAX_PLANE_COUNT];
      uint32_t plane_offsets[VIRGL_MAX_PLANE_COUNT];
      uint32_t plane_count = 0;
      struct pipe_resource *iter = &res->b;

      do {
         struct virgl_resource *plane = virgl_resource(iter);

         /* must be a plain 2D texture sharing the same hw_res */
         if (plane->b.target != PIPE_TEXTURE_2D ||
             plane->b.depth0 != 1 ||
             plane->b.array_size != 1 ||
             plane->b.last_level != 0 ||
             plane->b.nr_samples > 1 ||
             plane->hw_res != res->hw_res ||
             plane_count >= VIRGL_MAX_PLANE_COUNT) {
            vs->vws->resource_reference(vs->vws, &res->hw_res, NULL);
            FREE(res);
            return NULL;
         }

         plane_strides[plane_count] = plane->metadata.stride[0];
         plane_offsets[plane_count] = plane->metadata.plane_offset;
         plane_count++;
         iter = iter->next;
      } while (iter);

      vs->vws->resource_set_type(vs->vws,
                                 res->hw_res,
                                 pipe_to_virgl_format(res->b.format),
                                 pipe_to_virgl_bind(vs, res->b.bind),
                                 res->b.width0,
                                 res->b.height0,
                                 usage,
                                 res->metadata.modifier,
                                 plane_count,
                                 plane_strides,
                                 plane_offsets);
   }

   virgl_texture_init(res);

   return &res->b;
}

static bool
virgl_resource_get_param(struct pipe_screen *screen,
                         struct pipe_context *context,
                         struct pipe_resource *resource,
                         unsigned plane,
                         unsigned layer,
                         unsigned level,
                         enum pipe_resource_param param,
                         unsigned handle_usage,
                         uint64_t *value)
{
   struct virgl_resource *res = virgl_resource(resource);

   switch(param) {
   case PIPE_RESOURCE_PARAM_MODIFIER:
      *value = res->metadata.modifier;
      return true;
   default:
      return false;
   }
}

void virgl_init_screen_resource_functions(struct pipe_screen *screen)
{
    screen->resource_create_front = virgl_resource_create_front;
    screen->resource_create = virgl_resource_create;
    screen->resource_from_handle = virgl_resource_from_handle;
    screen->resource_get_handle = virgl_resource_get_handle;
    screen->resource_destroy = virgl_resource_destroy;
    screen->resource_get_param = virgl_resource_get_param;
}

static void virgl_buffer_subdata(struct pipe_context *pipe,
                                 struct pipe_resource *resource,
                                 unsigned usage, unsigned offset,
                                 unsigned size, const void *data)
{
   struct virgl_context *vctx = virgl_context(pipe);
   struct virgl_resource *vbuf = virgl_resource(resource);

   /* We can try virgl_transfer_queue_extend_buffer when there is no
    * flush/readback/wait required.  Based on virgl_resource_transfer_prepare,
    * the simplest way to make sure that is the case is to check the valid
    * buffer range.
    */
   if (!util_ranges_intersect(&vbuf->valid_buffer_range,
                              offset, offset + size) &&
       likely(!(virgl_debug & VIRGL_DEBUG_XFER)) &&
       virgl_transfer_queue_extend_buffer(&vctx->queue,
                                          vbuf->hw_res, offset, size, data)) {
      util_range_add(&vbuf->b, &vbuf->valid_buffer_range, offset, offset + size);
      return;
   }

   u_default_buffer_subdata(pipe, resource, usage, offset, size, data);
}

void virgl_init_context_resource_functions(struct pipe_context *ctx)
{
    ctx->buffer_map = virgl_resource_transfer_map;
    ctx->texture_map = virgl_texture_transfer_map;
    ctx->transfer_flush_region = virgl_buffer_transfer_flush_region;
    ctx->buffer_unmap = virgl_buffer_transfer_unmap;
    ctx->texture_unmap = virgl_texture_transfer_unmap;
    ctx->buffer_subdata = virgl_buffer_subdata;
    ctx->texture_subdata = u_default_texture_subdata;
}


struct virgl_transfer *
virgl_resource_create_transfer(struct virgl_context *vctx,
                               struct pipe_resource *pres,
                               const struct virgl_resource_metadata *metadata,
                               unsigned level, unsigned usage,
                               const struct pipe_box *box)
{
   struct virgl_winsys *vws = virgl_screen(vctx->base.screen)->vws;
   struct virgl_transfer *trans;
   enum pipe_format format = pres->format;
   const unsigned blocksy = box->y / util_format_get_blockheight(format);
   const unsigned blocksx = box->x / util_format_get_blockwidth(format);

   unsigned offset = metadata->plane_offset + metadata->level_offset[level];
   if (pres->target == PIPE_TEXTURE_CUBE ||
       pres->target == PIPE_TEXTURE_CUBE_ARRAY ||
       pres->target == PIPE_TEXTURE_3D ||
       pres->target == PIPE_TEXTURE_2D_ARRAY) {
      offset += box->z * metadata->layer_stride[level];
   }
   else if (pres->target == PIPE_TEXTURE_1D_ARRAY) {
      offset += box->z * metadata->stride[level];
      assert(box->y == 0);
   } else if (pres->target == PIPE_BUFFER) {
      assert(box->y == 0 && box->z == 0);
   } else {
      assert(box->z == 0);
   }

   offset += blocksy * metadata->stride[level];
   offset += blocksx * util_format_get_blocksize(format);

   trans = slab_zalloc(&vctx->transfer_pool);
   if (!trans)
      return NULL;

   pipe_resource_reference(&trans->base.resource, pres);
   vws->resource_reference(vws, &trans->hw_res, virgl_resource(pres)->hw_res);

   trans->base.level = level;
   trans->base.usage = usage;
   trans->base.box = *box;
   trans->base.stride = metadata->stride[level];
   trans->base.layer_stride = metadata->layer_stride[level];
   trans->offset = offset;
   util_range_init(&trans->range);

   if (trans->base.resource->target != PIPE_TEXTURE_3D &&
       trans->base.resource->target != PIPE_TEXTURE_CUBE &&
       trans->base.resource->target != PIPE_TEXTURE_1D_ARRAY &&
       trans->base.resource->target != PIPE_TEXTURE_2D_ARRAY &&
       trans->base.resource->target != PIPE_TEXTURE_CUBE_ARRAY)
      trans->l_stride = 0;
   else
      trans->l_stride = trans->base.layer_stride;

   return trans;
}

void virgl_resource_destroy_transfer(struct virgl_context *vctx,
                                     struct virgl_transfer *trans)
{
   struct virgl_winsys *vws = virgl_screen(vctx->base.screen)->vws;

   vws->resource_reference(vws, &trans->copy_src_hw_res, NULL);

   util_range_destroy(&trans->range);
   vws->resource_reference(vws, &trans->hw_res, NULL);
   pipe_resource_reference(&trans->base.resource, NULL);
   slab_free(&vctx->transfer_pool, trans);
}

void virgl_resource_destroy(struct pipe_screen *screen,
                            struct pipe_resource *resource)
{
   struct virgl_screen *vs = virgl_screen(screen);
   struct virgl_resource *res = virgl_resource(resource);

   if (res->b.target == PIPE_BUFFER)
      util_range_destroy(&res->valid_buffer_range);

   vs->vws->resource_reference(vs->vws, &res->hw_res, NULL);
   FREE(res);
}

bool virgl_resource_get_handle(struct pipe_screen *screen,
                               struct pipe_context *context,
                               struct pipe_resource *resource,
                               struct winsys_handle *whandle,
                               unsigned usage)
{
   struct virgl_screen *vs = virgl_screen(screen);
   struct virgl_resource *res = virgl_resource(resource);

   if (res->b.target == PIPE_BUFFER)
      return false;

   return vs->vws->resource_get_handle(vs->vws, res->hw_res,
                                       res->metadata.stride[0],
                                       whandle);
}

void virgl_resource_dirty(struct virgl_resource *res, uint32_t level)
{
   if (res) {
      if (res->b.target == PIPE_BUFFER)
         res->clean_mask &= ~1;
      else
         res->clean_mask &= ~(1 << level);
   }
}
