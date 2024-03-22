
#include "util/u_inlines.h"
#include "util/u_memory.h"
#include "util/u_math.h"
#include "util/u_surface.h"

#include "nouveau_screen.h"
#include "nouveau_context.h"
#include "nouveau_winsys.h"
#include "nouveau_fence.h"
#include "nouveau_buffer.h"
#include "nouveau_mm.h"

struct nouveau_transfer {
   struct pipe_transfer base;

   uint8_t *map;
   struct nouveau_bo *bo;
   struct nouveau_mm_allocation *mm;
   uint32_t offset;
};

static void *
nouveau_user_ptr_transfer_map(struct pipe_context *pipe,
                              struct pipe_resource *resource,
                              unsigned level, unsigned usage,
                              const struct pipe_box *box,
                              struct pipe_transfer **ptransfer);

static void
nouveau_user_ptr_transfer_unmap(struct pipe_context *pipe,
                                struct pipe_transfer *transfer);

static inline struct nouveau_transfer *
nouveau_transfer(struct pipe_transfer *transfer)
{
   return (struct nouveau_transfer *)transfer;
}

static inline bool
nouveau_buffer_malloc(struct nv04_resource *buf)
{
   if (!buf->data)
      buf->data = align_malloc(buf->base.width0, NOUVEAU_MIN_BUFFER_MAP_ALIGN);
   return !!buf->data;
}

static inline bool
nouveau_buffer_allocate(struct nouveau_screen *screen,
                        struct nv04_resource *buf, unsigned domain)
{
   uint32_t size = align(buf->base.width0, 0x100);

   if (domain == NOUVEAU_BO_VRAM) {
      buf->mm = nouveau_mm_allocate(screen->mm_VRAM, size,
                                    &buf->bo, &buf->offset);
      if (!buf->bo)
         return nouveau_buffer_allocate(screen, buf, NOUVEAU_BO_GART);
      NOUVEAU_DRV_STAT(screen, buf_obj_current_bytes_vid, buf->base.width0);
   } else
   if (domain == NOUVEAU_BO_GART) {
      buf->mm = nouveau_mm_allocate(screen->mm_GART, size,
                                    &buf->bo, &buf->offset);
      if (!buf->bo)
         return false;
      NOUVEAU_DRV_STAT(screen, buf_obj_current_bytes_sys, buf->base.width0);
   } else {
      assert(domain == 0);
      if (!nouveau_buffer_malloc(buf))
         return false;
   }
   buf->domain = domain;
   if (buf->bo)
      buf->address = buf->bo->offset + buf->offset;

   util_range_set_empty(&buf->valid_buffer_range);

   return true;
}

static inline void
release_allocation(struct nouveau_mm_allocation **mm,
                   struct nouveau_fence *fence)
{
   nouveau_fence_work(fence, nouveau_mm_free_work, *mm);
   (*mm) = NULL;
}

inline void
nouveau_buffer_release_gpu_storage(struct nv04_resource *buf)
{
   assert(!(buf->status & NOUVEAU_BUFFER_STATUS_USER_PTR));

   nouveau_fence_work(buf->fence, nouveau_fence_unref_bo, buf->bo);
   buf->bo = NULL;

   if (buf->mm)
      release_allocation(&buf->mm, buf->fence);

   if (buf->domain == NOUVEAU_BO_VRAM)
      NOUVEAU_DRV_STAT_RES(buf, buf_obj_current_bytes_vid, -(uint64_t)buf->base.width0);
   if (buf->domain == NOUVEAU_BO_GART)
      NOUVEAU_DRV_STAT_RES(buf, buf_obj_current_bytes_sys, -(uint64_t)buf->base.width0);

   buf->domain = 0;
}

static inline bool
nouveau_buffer_reallocate(struct nouveau_screen *screen,
                          struct nv04_resource *buf, unsigned domain)
{
   nouveau_buffer_release_gpu_storage(buf);

   nouveau_fence_ref(NULL, &buf->fence);
   nouveau_fence_ref(NULL, &buf->fence_wr);

   buf->status &= NOUVEAU_BUFFER_STATUS_REALLOC_MASK;

   return nouveau_buffer_allocate(screen, buf, domain);
}

void
nouveau_buffer_destroy(struct pipe_screen *pscreen,
                       struct pipe_resource *presource)
{
   struct nv04_resource *res = nv04_resource(presource);

   if (res->status & NOUVEAU_BUFFER_STATUS_USER_PTR) {
      FREE(res);
      return;
   }

   nouveau_buffer_release_gpu_storage(res);

   if (res->data && !(res->status & NOUVEAU_BUFFER_STATUS_USER_MEMORY))
      align_free(res->data);

   nouveau_fence_ref(NULL, &res->fence);
   nouveau_fence_ref(NULL, &res->fence_wr);

   util_range_destroy(&res->valid_buffer_range);

   FREE(res);

   NOUVEAU_DRV_STAT(nouveau_screen(pscreen), buf_obj_current_count, -1);
}

/* Set up a staging area for the transfer. This is either done in "regular"
 * system memory if the driver supports push_data (nv50+) and the data is
 * small enough (and permit_pb == true), or in GART memory.
 */
static uint8_t *
nouveau_transfer_staging(struct nouveau_context *nv,
                         struct nouveau_transfer *tx, bool permit_pb)
{
   const unsigned adj = tx->base.box.x & NOUVEAU_MIN_BUFFER_MAP_ALIGN_MASK;
   const unsigned size = align(tx->base.box.width, 4) + adj;

   if (!nv->push_data)
      permit_pb = false;

   if ((size <= nv->screen->transfer_pushbuf_threshold) && permit_pb) {
      tx->map = align_malloc(size, NOUVEAU_MIN_BUFFER_MAP_ALIGN);
      if (tx->map)
         tx->map += adj;
   } else {
      tx->mm =
         nouveau_mm_allocate(nv->screen->mm_GART, size, &tx->bo, &tx->offset);
      if (tx->bo) {
         tx->offset += adj;
         if (!BO_MAP(nv->screen, tx->bo, 0, NULL))
            tx->map = (uint8_t *)tx->bo->map + tx->offset;
      }
   }
   return tx->map;
}

/* Copies data from the resource into the transfer's temporary GART
 * buffer. Also updates buf->data if present.
 *
 * Maybe just migrate to GART right away if we actually need to do this. */
static bool
nouveau_transfer_read(struct nouveau_context *nv, struct nouveau_transfer *tx)
{
   struct nv04_resource *buf = nv04_resource(tx->base.resource);
   const unsigned base = tx->base.box.x;
   const unsigned size = tx->base.box.width;

   NOUVEAU_DRV_STAT(nv->screen, buf_read_bytes_staging_vid, size);

   nv->copy_data(nv, tx->bo, tx->offset, NOUVEAU_BO_GART,
                 buf->bo, buf->offset + base, buf->domain, size);

   if (BO_WAIT(nv->screen, tx->bo, NOUVEAU_BO_RD, nv->client))
      return false;

   if (buf->data)
      memcpy(buf->data + base, tx->map, size);

   return true;
}

static void
nouveau_transfer_write(struct nouveau_context *nv, struct nouveau_transfer *tx,
                       unsigned offset, unsigned size)
{
   struct nv04_resource *buf = nv04_resource(tx->base.resource);
   uint8_t *data = tx->map + offset;
   const unsigned base = tx->base.box.x + offset;
   const bool can_cb = !((base | size) & 3);

   if (buf->data)
      memcpy(data, buf->data + base, size);
   else
      buf->status |= NOUVEAU_BUFFER_STATUS_DIRTY;

   if (buf->domain == NOUVEAU_BO_VRAM)
      NOUVEAU_DRV_STAT(nv->screen, buf_write_bytes_staging_vid, size);
   if (buf->domain == NOUVEAU_BO_GART)
      NOUVEAU_DRV_STAT(nv->screen, buf_write_bytes_staging_sys, size);

   if (tx->bo)
      nv->copy_data(nv, buf->bo, buf->offset + base, buf->domain,
                    tx->bo, tx->offset + offset, NOUVEAU_BO_GART, size);
   else
   if (nv->push_cb && can_cb)
      nv->push_cb(nv, buf,
                  base, size / 4, (const uint32_t *)data);
   else
      nv->push_data(nv, buf->bo, buf->offset + base, buf->domain, size, data);

   nouveau_fence_ref(nv->fence, &buf->fence);
   nouveau_fence_ref(nv->fence, &buf->fence_wr);
}

/* Does a CPU wait for the buffer's backing data to become reliably accessible
 * for write/read by waiting on the buffer's relevant fences.
 */
static inline bool
nouveau_buffer_sync(struct nouveau_context *nv,
                    struct nv04_resource *buf, unsigned rw)
{
   if (rw == PIPE_MAP_READ) {
      if (!buf->fence_wr)
         return true;
      NOUVEAU_DRV_STAT_RES(buf, buf_non_kernel_fence_sync_count,
                           !nouveau_fence_signalled(buf->fence_wr));
      if (!nouveau_fence_wait(buf->fence_wr, &nv->debug))
         return false;
   } else {
      if (!buf->fence)
         return true;
      NOUVEAU_DRV_STAT_RES(buf, buf_non_kernel_fence_sync_count,
                           !nouveau_fence_signalled(buf->fence));
      if (!nouveau_fence_wait(buf->fence, &nv->debug))
         return false;

      nouveau_fence_ref(NULL, &buf->fence);
   }
   nouveau_fence_ref(NULL, &buf->fence_wr);

   return true;
}

static inline bool
nouveau_buffer_busy(struct nv04_resource *buf, unsigned rw)
{
   if (rw == PIPE_MAP_READ)
      return (buf->fence_wr && !nouveau_fence_signalled(buf->fence_wr));
   else
      return (buf->fence && !nouveau_fence_signalled(buf->fence));
}

static inline void
nouveau_buffer_transfer_init(struct nouveau_transfer *tx,
                             struct pipe_resource *resource,
                             const struct pipe_box *box,
                             unsigned usage)
{
   tx->base.resource = resource;
   tx->base.level = 0;
   tx->base.usage = usage;
   tx->base.box.x = box->x;
   tx->base.box.y = 0;
   tx->base.box.z = 0;
   tx->base.box.width = box->width;
   tx->base.box.height = 1;
   tx->base.box.depth = 1;
   tx->base.stride = 0;
   tx->base.layer_stride = 0;

   tx->bo = NULL;
   tx->map = NULL;
}

static inline void
nouveau_buffer_transfer_del(struct nouveau_context *nv,
                            struct nouveau_transfer *tx)
{
   if (tx->map) {
      if (likely(tx->bo)) {
         nouveau_fence_work(nv->fence, nouveau_fence_unref_bo, tx->bo);
         if (tx->mm)
            release_allocation(&tx->mm, nv->fence);
      } else {
         align_free(tx->map -
                    (tx->base.box.x & NOUVEAU_MIN_BUFFER_MAP_ALIGN_MASK));
      }
   }
}

/* Creates a cache in system memory of the buffer data. */
static bool
nouveau_buffer_cache(struct nouveau_context *nv, struct nv04_resource *buf)
{
   struct nouveau_transfer tx;
   bool ret;
   tx.base.resource = &buf->base;
   tx.base.box.x = 0;
   tx.base.box.width = buf->base.width0;
   tx.bo = NULL;
   tx.map = NULL;

   if (!buf->data)
      if (!nouveau_buffer_malloc(buf))
         return false;
   if (!(buf->status & NOUVEAU_BUFFER_STATUS_DIRTY))
      return true;
   nv->stats.buf_cache_count++;

   if (!nouveau_transfer_staging(nv, &tx, false))
      return false;

   ret = nouveau_transfer_read(nv, &tx);
   if (ret) {
      buf->status &= ~NOUVEAU_BUFFER_STATUS_DIRTY;
      memcpy(buf->data, tx.map, buf->base.width0);
   }
   nouveau_buffer_transfer_del(nv, &tx);
   return ret;
}


#define NOUVEAU_TRANSFER_DISCARD \
   (PIPE_MAP_DISCARD_RANGE | PIPE_MAP_DISCARD_WHOLE_RESOURCE)

/* Checks whether it is possible to completely discard the memory backing this
 * resource. This can be useful if we would otherwise have to wait for a read
 * operation to complete on this data.
 */
static inline bool
nouveau_buffer_should_discard(struct nv04_resource *buf, unsigned usage)
{
   if (!(usage & PIPE_MAP_DISCARD_WHOLE_RESOURCE))
      return false;
   if (unlikely(buf->base.bind & PIPE_BIND_SHARED))
      return false;
   if (unlikely(usage & PIPE_MAP_PERSISTENT))
      return false;
   return buf->mm && nouveau_buffer_busy(buf, PIPE_MAP_WRITE);
}

/* Returns a pointer to a memory area representing a window into the
 * resource's data.
 *
 * This may or may not be the _actual_ memory area of the resource. However
 * when calling nouveau_buffer_transfer_unmap, if it wasn't the actual memory
 * area, the contents of the returned map are copied over to the resource.
 *
 * The usage indicates what the caller plans to do with the map:
 *
 *   WRITE means that the user plans to write to it
 *
 *   READ means that the user plans on reading from it
 *
 *   DISCARD_WHOLE_RESOURCE means that the whole resource is going to be
 *   potentially overwritten, and even if it isn't, the bits that aren't don't
 *   need to be maintained.
 *
 *   DISCARD_RANGE means that all the data in the specified range is going to
 *   be overwritten.
 *
 * The strategy for determining what kind of memory area to return is complex,
 * see comments inside of the function.
 */
void *
nouveau_buffer_transfer_map(struct pipe_context *pipe,
                            struct pipe_resource *resource,
                            unsigned level, unsigned usage,
                            const struct pipe_box *box,
                            struct pipe_transfer **ptransfer)
{
   struct nouveau_context *nv = nouveau_context(pipe);
   struct nv04_resource *buf = nv04_resource(resource);

   if (buf->status & NOUVEAU_BUFFER_STATUS_USER_PTR)
      return nouveau_user_ptr_transfer_map(pipe, resource, level, usage, box, ptransfer);

   struct nouveau_transfer *tx = MALLOC_STRUCT(nouveau_transfer);
   uint8_t *map;
   int ret;

   if (!tx)
      return NULL;
   nouveau_buffer_transfer_init(tx, resource, box, usage);
   *ptransfer = &tx->base;

   if (usage & PIPE_MAP_READ)
      NOUVEAU_DRV_STAT(nv->screen, buf_transfers_rd, 1);
   if (usage & PIPE_MAP_WRITE)
      NOUVEAU_DRV_STAT(nv->screen, buf_transfers_wr, 1);

   /* If we are trying to write to an uninitialized range, the user shouldn't
    * care what was there before. So we can treat the write as if the target
    * range were being discarded. Furthermore, since we know that even if this
    * buffer is busy due to GPU activity, because the contents were
    * uninitialized, the GPU can't care what was there, and so we can treat
    * the write as being unsynchronized.
    */
   if ((usage & PIPE_MAP_WRITE) &&
       !util_ranges_intersect(&buf->valid_buffer_range, box->x, box->x + box->width))
      usage |= PIPE_MAP_DISCARD_RANGE | PIPE_MAP_UNSYNCHRONIZED;

   if (buf->domain == NOUVEAU_BO_VRAM) {
      if (usage & NOUVEAU_TRANSFER_DISCARD) {
         /* Set up a staging area for the user to write to. It will be copied
          * back into VRAM on unmap. */
         if (usage & PIPE_MAP_DISCARD_WHOLE_RESOURCE)
            buf->status &= NOUVEAU_BUFFER_STATUS_REALLOC_MASK;
         nouveau_transfer_staging(nv, tx, true);
      } else {
         if (buf->status & NOUVEAU_BUFFER_STATUS_GPU_WRITING) {
            /* The GPU is currently writing to this buffer. Copy its current
             * contents to a staging area in the GART. This is necessary since
             * not the whole area being mapped is being discarded.
             */
            if (buf->data) {
               align_free(buf->data);
               buf->data = NULL;
            }
            nouveau_transfer_staging(nv, tx, false);
            nouveau_transfer_read(nv, tx);
         } else {
            /* The buffer is currently idle. Create a staging area for writes,
             * and make sure that the cached data is up-to-date. */
            if (usage & PIPE_MAP_WRITE)
               nouveau_transfer_staging(nv, tx, true);
            if (!buf->data)
               nouveau_buffer_cache(nv, buf);
         }
      }
      return buf->data ? (buf->data + box->x) : tx->map;
   } else
   if (unlikely(buf->domain == 0)) {
      return buf->data + box->x;
   }

   /* At this point, buf->domain == GART */

   if (nouveau_buffer_should_discard(buf, usage)) {
      int ref = buf->base.reference.count - 1;
      nouveau_buffer_reallocate(nv->screen, buf, buf->domain);
      if (ref > 0) /* any references inside context possible ? */
         nv->invalidate_resource_storage(nv, &buf->base, ref);
   }

   /* Note that nouveau_bo_map ends up doing a nouveau_bo_wait with the
    * relevant flags. If buf->mm is set, that means this resource is part of a
    * larger slab bo that holds multiple resources. So in that case, don't
    * wait on the whole slab and instead use the logic below to return a
    * reasonable buffer for that case.
    */
   ret = BO_MAP(nv->screen, buf->bo,
                buf->mm ? 0 : nouveau_screen_transfer_flags(usage),
                nv->client);
   if (ret) {
      FREE(tx);
      return NULL;
   }
   map = (uint8_t *)buf->bo->map + buf->offset + box->x;

   /* using kernel fences only if !buf->mm */
   if ((usage & PIPE_MAP_UNSYNCHRONIZED) || !buf->mm)
      return map;

   /* If the GPU is currently reading/writing this buffer, we shouldn't
    * interfere with its progress. So instead we either wait for the GPU to
    * complete its operation, or set up a staging area to perform our work in.
    */
   if (nouveau_buffer_busy(buf, usage & PIPE_MAP_READ_WRITE)) {
      if (unlikely(usage & (PIPE_MAP_DISCARD_WHOLE_RESOURCE |
                            PIPE_MAP_PERSISTENT))) {
         /* Discarding was not possible, must sync because
          * subsequent transfers might use UNSYNCHRONIZED. */
         nouveau_buffer_sync(nv, buf, usage & PIPE_MAP_READ_WRITE);
      } else
      if (usage & PIPE_MAP_DISCARD_RANGE) {
         /* The whole range is being discarded, so it doesn't matter what was
          * there before. No need to copy anything over. */
         nouveau_transfer_staging(nv, tx, true);
         map = tx->map;
      } else
      if (nouveau_buffer_busy(buf, PIPE_MAP_READ)) {
         if (usage & PIPE_MAP_DONTBLOCK)
            map = NULL;
         else
            nouveau_buffer_sync(nv, buf, usage & PIPE_MAP_READ_WRITE);
      } else {
         /* It is expected that the returned buffer be a representation of the
          * data in question, so we must copy it over from the buffer. */
         nouveau_transfer_staging(nv, tx, true);
         if (tx->map)
            memcpy(tx->map, map, box->width);
         map = tx->map;
      }
   }
   if (!map)
      FREE(tx);
   return map;
}



void
nouveau_buffer_transfer_flush_region(struct pipe_context *pipe,
                                     struct pipe_transfer *transfer,
                                     const struct pipe_box *box)
{
   struct nouveau_transfer *tx = nouveau_transfer(transfer);
   struct nv04_resource *buf = nv04_resource(transfer->resource);

   if (tx->map)
      nouveau_transfer_write(nouveau_context(pipe), tx, box->x, box->width);

   util_range_add(&buf->base, &buf->valid_buffer_range,
                  tx->base.box.x + box->x,
                  tx->base.box.x + box->x + box->width);
}

/* Unmap stage of the transfer. If it was a WRITE transfer and the map that
 * was returned was not the real resource's data, this needs to transfer the
 * data back to the resource.
 *
 * Also marks vbo dirty based on the buffer's binding
 */
void
nouveau_buffer_transfer_unmap(struct pipe_context *pipe,
                              struct pipe_transfer *transfer)
{
   struct nouveau_context *nv = nouveau_context(pipe);
   struct nv04_resource *buf = nv04_resource(transfer->resource);

   if (buf->status & NOUVEAU_BUFFER_STATUS_USER_PTR)
      return nouveau_user_ptr_transfer_unmap(pipe, transfer);

   struct nouveau_transfer *tx = nouveau_transfer(transfer);

   if (tx->base.usage & PIPE_MAP_WRITE) {
      if (!(tx->base.usage & PIPE_MAP_FLUSH_EXPLICIT)) {
         if (tx->map)
            nouveau_transfer_write(nv, tx, 0, tx->base.box.width);

         util_range_add(&buf->base, &buf->valid_buffer_range,
                        tx->base.box.x, tx->base.box.x + tx->base.box.width);
      }

      if (likely(buf->domain)) {
         const uint8_t bind = buf->base.bind;
         /* make sure we invalidate dedicated caches */
         if (bind & (PIPE_BIND_VERTEX_BUFFER | PIPE_BIND_INDEX_BUFFER))
            nv->vbo_dirty = true;
      }
   }

   if (!tx->bo && (tx->base.usage & PIPE_MAP_WRITE))
      NOUVEAU_DRV_STAT(nv->screen, buf_write_bytes_direct, tx->base.box.width);

   nouveau_buffer_transfer_del(nv, tx);
   FREE(tx);
}


void
nouveau_copy_buffer(struct nouveau_context *nv,
                    struct nv04_resource *dst, unsigned dstx,
                    struct nv04_resource *src, unsigned srcx, unsigned size)
{
   assert(dst->base.target == PIPE_BUFFER && src->base.target == PIPE_BUFFER);

   if (likely(dst->domain) && likely(src->domain)) {
      nv->copy_data(nv,
                    dst->bo, dst->offset + dstx, dst->domain,
                    src->bo, src->offset + srcx, src->domain, size);

      dst->status |= NOUVEAU_BUFFER_STATUS_GPU_WRITING;
      nouveau_fence_ref(nv->fence, &dst->fence);
      nouveau_fence_ref(nv->fence, &dst->fence_wr);

      src->status |= NOUVEAU_BUFFER_STATUS_GPU_READING;
      nouveau_fence_ref(nv->fence, &src->fence);
   } else {
      struct pipe_box src_box;
      src_box.x = srcx;
      src_box.y = 0;
      src_box.z = 0;
      src_box.width = size;
      src_box.height = 1;
      src_box.depth = 1;
      util_resource_copy_region(&nv->pipe,
                                &dst->base, 0, dstx, 0, 0,
                                &src->base, 0, &src_box);
   }

   util_range_add(&dst->base, &dst->valid_buffer_range, dstx, dstx + size);
}


void *
nouveau_resource_map_offset(struct nouveau_context *nv,
                            struct nv04_resource *res, uint32_t offset,
                            uint32_t flags)
{
   if (unlikely(res->status & NOUVEAU_BUFFER_STATUS_USER_MEMORY) ||
       unlikely(res->status & NOUVEAU_BUFFER_STATUS_USER_PTR))
      return res->data + offset;

   if (res->domain == NOUVEAU_BO_VRAM) {
      if (!res->data || (res->status & NOUVEAU_BUFFER_STATUS_GPU_WRITING))
         nouveau_buffer_cache(nv, res);
   }
   if (res->domain != NOUVEAU_BO_GART)
      return res->data + offset;

   if (res->mm) {
      unsigned rw;
      rw = (flags & NOUVEAU_BO_WR) ? PIPE_MAP_WRITE : PIPE_MAP_READ;
      nouveau_buffer_sync(nv, res, rw);
      if (BO_MAP(nv->screen, res->bo, 0, NULL))
         return NULL;
   } else {
      if (BO_MAP(nv->screen, res->bo, flags, nv->client))
         return NULL;
   }
   return (uint8_t *)res->bo->map + res->offset + offset;
}

static void *
nouveau_user_ptr_transfer_map(struct pipe_context *pipe,
                              struct pipe_resource *resource,
                              unsigned level, unsigned usage,
                              const struct pipe_box *box,
                              struct pipe_transfer **ptransfer)
{
   struct nouveau_transfer *tx = MALLOC_STRUCT(nouveau_transfer);
   if (!tx)
      return NULL;
   nouveau_buffer_transfer_init(tx, resource, box, usage);
   *ptransfer = &tx->base;
   return nv04_resource(resource)->data;
}

static void
nouveau_user_ptr_transfer_unmap(struct pipe_context *pipe,
                                struct pipe_transfer *transfer)
{
   struct nouveau_transfer *tx = nouveau_transfer(transfer);
   FREE(tx);
}

struct pipe_resource *
nouveau_buffer_create(struct pipe_screen *pscreen,
                      const struct pipe_resource *templ)
{
   struct nouveau_screen *screen = nouveau_screen(pscreen);
   struct nv04_resource *buffer;
   bool ret;

   buffer = CALLOC_STRUCT(nv04_resource);
   if (!buffer)
      return NULL;

   buffer->base = *templ;
   pipe_reference_init(&buffer->base.reference, 1);
   buffer->base.screen = pscreen;

   if (buffer->base.flags & (PIPE_RESOURCE_FLAG_MAP_PERSISTENT |
                             PIPE_RESOURCE_FLAG_MAP_COHERENT)) {
      buffer->domain = NOUVEAU_BO_GART;
   } else if (buffer->base.bind == 0 || (buffer->base.bind &
              (screen->vidmem_bindings & screen->sysmem_bindings))) {
      switch (buffer->base.usage) {
      case PIPE_USAGE_DEFAULT:
      case PIPE_USAGE_IMMUTABLE:
         buffer->domain = NV_VRAM_DOMAIN(screen);
         break;
      case PIPE_USAGE_DYNAMIC:
         /* For most apps, we'd have to do staging transfers to avoid sync
          * with this usage, and GART -> GART copies would be suboptimal.
          */
         buffer->domain = NV_VRAM_DOMAIN(screen);
         break;
      case PIPE_USAGE_STAGING:
      case PIPE_USAGE_STREAM:
         buffer->domain = NOUVEAU_BO_GART;
         break;
      default:
         assert(0);
         break;
      }
   } else {
      if (buffer->base.bind & screen->vidmem_bindings)
         buffer->domain = NV_VRAM_DOMAIN(screen);
      else
      if (buffer->base.bind & screen->sysmem_bindings)
         buffer->domain = NOUVEAU_BO_GART;
   }

   ret = nouveau_buffer_allocate(screen, buffer, buffer->domain);

   if (ret == false)
      goto fail;

   if (buffer->domain == NOUVEAU_BO_VRAM && screen->hint_buf_keep_sysmem_copy)
      nouveau_buffer_cache(NULL, buffer);

   NOUVEAU_DRV_STAT(screen, buf_obj_current_count, 1);

   util_range_init(&buffer->valid_buffer_range);

   return &buffer->base;

fail:
   FREE(buffer);
   return NULL;
}

struct pipe_resource *
nouveau_buffer_create_from_user(struct pipe_screen *pscreen,
                                const struct pipe_resource *templ,
                                void *user_ptr)
{
   struct nv04_resource *buffer;

   buffer = CALLOC_STRUCT(nv04_resource);
   if (!buffer)
      return NULL;

   buffer->base = *templ;
   /* set address and data to the same thing for higher compatibility with
    * existing code. It's correct nonetheless as the same pointer is equally
    * valid on the CPU and the GPU.
    */
   buffer->address = (uintptr_t)user_ptr;
   buffer->data = user_ptr;
   buffer->status = NOUVEAU_BUFFER_STATUS_USER_PTR;
   buffer->base.screen = pscreen;

   pipe_reference_init(&buffer->base.reference, 1);

   return &buffer->base;
}

struct pipe_resource *
nouveau_user_buffer_create(struct pipe_screen *pscreen, void *ptr,
                           unsigned bytes, unsigned bind)
{
   struct nv04_resource *buffer;

   buffer = CALLOC_STRUCT(nv04_resource);
   if (!buffer)
      return NULL;

   pipe_reference_init(&buffer->base.reference, 1);
   buffer->base.screen = pscreen;
   buffer->base.format = PIPE_FORMAT_R8_UNORM;
   buffer->base.usage = PIPE_USAGE_IMMUTABLE;
   buffer->base.bind = bind;
   buffer->base.width0 = bytes;
   buffer->base.height0 = 1;
   buffer->base.depth0 = 1;

   buffer->data = ptr;
   buffer->status = NOUVEAU_BUFFER_STATUS_USER_MEMORY;

   util_range_init(&buffer->valid_buffer_range);
   util_range_add(&buffer->base, &buffer->valid_buffer_range, 0, bytes);

   return &buffer->base;
}

static inline bool
nouveau_buffer_data_fetch(struct nouveau_context *nv, struct nv04_resource *buf,
                          struct nouveau_bo *bo, unsigned offset, unsigned size)
{
   if (!nouveau_buffer_malloc(buf))
      return false;
   if (BO_MAP(nv->screen, bo, NOUVEAU_BO_RD, nv->client))
      return false;
   memcpy(buf->data, (uint8_t *)bo->map + offset, size);
   return true;
}

/* Migrate a linear buffer (vertex, index, constants) USER -> GART -> VRAM. */
bool
nouveau_buffer_migrate(struct nouveau_context *nv,
                       struct nv04_resource *buf, const unsigned new_domain)
{
   assert(!(buf->status & NOUVEAU_BUFFER_STATUS_USER_PTR));

   struct nouveau_screen *screen = nv->screen;
   struct nouveau_bo *bo;
   const unsigned old_domain = buf->domain;
   unsigned size = buf->base.width0;
   unsigned offset;
   int ret;

   assert(new_domain != old_domain);

   if (new_domain == NOUVEAU_BO_GART && old_domain == 0) {
      if (!nouveau_buffer_allocate(screen, buf, new_domain))
         return false;
      ret = BO_MAP(nv->screen, buf->bo, 0, nv->client);
      if (ret)
         return ret;
      memcpy((uint8_t *)buf->bo->map + buf->offset, buf->data, size);
      align_free(buf->data);
   } else
   if (old_domain != 0 && new_domain != 0) {
      struct nouveau_mm_allocation *mm = buf->mm;

      if (new_domain == NOUVEAU_BO_VRAM) {
         /* keep a system memory copy of our data in case we hit a fallback */
         if (!nouveau_buffer_data_fetch(nv, buf, buf->bo, buf->offset, size))
            return false;
         if (nouveau_mesa_debug)
            debug_printf("migrating %u KiB to VRAM\n", size / 1024);
      }

      offset = buf->offset;
      bo = buf->bo;
      buf->bo = NULL;
      buf->mm = NULL;
      nouveau_buffer_allocate(screen, buf, new_domain);

      nv->copy_data(nv, buf->bo, buf->offset, new_domain,
                    bo, offset, old_domain, buf->base.width0);

      nouveau_fence_work(nv->fence, nouveau_fence_unref_bo, bo);
      if (mm)
         release_allocation(&mm, nv->fence);
   } else
   if (new_domain == NOUVEAU_BO_VRAM && old_domain == 0) {
      struct nouveau_transfer tx;
      if (!nouveau_buffer_allocate(screen, buf, NOUVEAU_BO_VRAM))
         return false;
      tx.base.resource = &buf->base;
      tx.base.box.x = 0;
      tx.base.box.width = buf->base.width0;
      tx.bo = NULL;
      tx.map = NULL;
      if (!nouveau_transfer_staging(nv, &tx, false))
         return false;
      nouveau_transfer_write(nv, &tx, 0, tx.base.box.width);
      nouveau_buffer_transfer_del(nv, &tx);
   } else
      return false;

   assert(buf->domain == new_domain);
   return true;
}

/* Migrate data from glVertexAttribPointer(non-VBO) user buffers to GART.
 * We'd like to only allocate @size bytes here, but then we'd have to rebase
 * the vertex indices ...
 */
bool
nouveau_user_buffer_upload(struct nouveau_context *nv,
                           struct nv04_resource *buf,
                           unsigned base, unsigned size)
{
   assert(!(buf->status & NOUVEAU_BUFFER_STATUS_USER_PTR));

   struct nouveau_screen *screen = nouveau_screen(buf->base.screen);
   int ret;

   assert(buf->status & NOUVEAU_BUFFER_STATUS_USER_MEMORY);

   buf->base.width0 = base + size;
   if (!nouveau_buffer_reallocate(screen, buf, NOUVEAU_BO_GART))
      return false;

   ret = BO_MAP(nv->screen, buf->bo, 0, nv->client);
   if (ret)
      return false;
   memcpy((uint8_t *)buf->bo->map + buf->offset + base, buf->data + base, size);

   return true;
}

/* Invalidate underlying buffer storage, reset fences, reallocate to non-busy
 * buffer.
 */
void
nouveau_buffer_invalidate(struct pipe_context *pipe,
                          struct pipe_resource *resource)
{
   struct nouveau_context *nv = nouveau_context(pipe);
   struct nv04_resource *buf = nv04_resource(resource);
   int ref = buf->base.reference.count - 1;

   assert(!(buf->status & NOUVEAU_BUFFER_STATUS_USER_PTR));

   /* Shared buffers shouldn't get reallocated */
   if (unlikely(buf->base.bind & PIPE_BIND_SHARED))
      return;

   /* If the buffer is sub-allocated and not currently being written, just
    * wipe the valid buffer range. Otherwise we have to create fresh
    * storage. (We don't keep track of fences for non-sub-allocated BO's.)
    */
   if (buf->mm && !nouveau_buffer_busy(buf, PIPE_MAP_WRITE)) {
      util_range_set_empty(&buf->valid_buffer_range);
   } else {
      nouveau_buffer_reallocate(nv->screen, buf, buf->domain);
      if (ref > 0) /* any references inside context possible ? */
         nv->invalidate_resource_storage(nv, &buf->base, ref);
   }
}


/* Scratch data allocation. */

static inline int
nouveau_scratch_bo_alloc(struct nouveau_context *nv, struct nouveau_bo **pbo,
                         unsigned size)
{
   return nouveau_bo_new(nv->screen->device, NOUVEAU_BO_GART | NOUVEAU_BO_MAP,
                         4096, size, NULL, pbo);
}

static void
nouveau_scratch_unref_bos(void *d)
{
   struct runout *b = d;
   int i;

   for (i = 0; i < b->nr; ++i)
      nouveau_bo_ref(NULL, &b->bo[i]);

   FREE(b);
}

void
nouveau_scratch_runout_release(struct nouveau_context *nv)
{
   if (!nv->scratch.runout)
      return;

   if (!nouveau_fence_work(nv->fence, nouveau_scratch_unref_bos,
         nv->scratch.runout))
      return;

   nv->scratch.end = 0;
   nv->scratch.runout = NULL;
}

/* Allocate an extra bo if we can't fit everything we need simultaneously.
 * (Could happen for very large user arrays.)
 */
static inline bool
nouveau_scratch_runout(struct nouveau_context *nv, unsigned size)
{
   int ret;
   unsigned n;

   if (nv->scratch.runout)
      n = nv->scratch.runout->nr;
   else
      n = 0;
   nv->scratch.runout = REALLOC(nv->scratch.runout, n == 0 ? 0 :
                                (sizeof(*nv->scratch.runout) + (n + 0) * sizeof(void *)),
                                 sizeof(*nv->scratch.runout) + (n + 1) * sizeof(void *));
   nv->scratch.runout->nr = n + 1;
   nv->scratch.runout->bo[n] = NULL;

   ret = nouveau_scratch_bo_alloc(nv, &nv->scratch.runout->bo[n], size);
   if (!ret) {
      ret = BO_MAP(nv->screen, nv->scratch.runout->bo[n], 0, NULL);
      if (ret)
         nouveau_bo_ref(NULL, &nv->scratch.runout->bo[--nv->scratch.runout->nr]);
   }
   if (!ret) {
      nv->scratch.current = nv->scratch.runout->bo[n];
      nv->scratch.offset = 0;
      nv->scratch.end = size;
      nv->scratch.map = nv->scratch.current->map;
   }
   return !ret;
}

/* Continue to next scratch buffer, if available (no wrapping, large enough).
 * Allocate it if it has not yet been created.
 */
static inline bool
nouveau_scratch_next(struct nouveau_context *nv, unsigned size)
{
   struct nouveau_bo *bo;
   int ret;
   const unsigned i = (nv->scratch.id + 1) % NOUVEAU_MAX_SCRATCH_BUFS;

   if ((size > nv->scratch.bo_size) || (i == nv->scratch.wrap))
      return false;
   nv->scratch.id = i;

   bo = nv->scratch.bo[i];
   if (!bo) {
      ret = nouveau_scratch_bo_alloc(nv, &bo, nv->scratch.bo_size);
      if (ret)
         return false;
      nv->scratch.bo[i] = bo;
   }
   nv->scratch.current = bo;
   nv->scratch.offset = 0;
   nv->scratch.end = nv->scratch.bo_size;

   ret = BO_MAP(nv->screen, bo, NOUVEAU_BO_WR, nv->client);
   if (!ret)
      nv->scratch.map = bo->map;
   return !ret;
}

static bool
nouveau_scratch_more(struct nouveau_context *nv, unsigned min_size)
{
   bool ret;

   ret = nouveau_scratch_next(nv, min_size);
   if (!ret)
      ret = nouveau_scratch_runout(nv, min_size);
   return ret;
}


/* Copy data to a scratch buffer and return address & bo the data resides in. */
uint64_t
nouveau_scratch_data(struct nouveau_context *nv,
                     const void *data, unsigned base, unsigned size,
                     struct nouveau_bo **bo)
{
   unsigned bgn = MAX2(base, nv->scratch.offset);
   unsigned end = bgn + size;

   if (end >= nv->scratch.end) {
      end = base + size;
      if (!nouveau_scratch_more(nv, end))
         return 0;
      bgn = base;
   }
   nv->scratch.offset = align(end, 4);

   memcpy(nv->scratch.map + bgn, (const uint8_t *)data + base, size);

   *bo = nv->scratch.current;
   return (*bo)->offset + (bgn - base);
}

void *
nouveau_scratch_get(struct nouveau_context *nv,
                    unsigned size, uint64_t *gpu_addr, struct nouveau_bo **pbo)
{
   unsigned bgn = nv->scratch.offset;
   unsigned end = nv->scratch.offset + size;

   if (end >= nv->scratch.end) {
      end = size;
      if (!nouveau_scratch_more(nv, end))
         return NULL;
      bgn = 0;
   }
   nv->scratch.offset = align(end, 4);

   *pbo = nv->scratch.current;
   *gpu_addr = nv->scratch.current->offset + bgn;
   return nv->scratch.map + bgn;
}
