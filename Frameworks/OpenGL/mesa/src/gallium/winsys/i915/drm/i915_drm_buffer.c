#include "frontend/drm_driver.h"
#include "i915_drm_winsys.h"
#include "util/u_memory.h"

#include "drm-uapi/i915_drm.h"

static char *i915_drm_type_to_name(enum i915_winsys_buffer_type type)
{
   char *name;

   if (type == I915_NEW_TEXTURE) {
      name = "gallium3d_texture";
   } else if (type == I915_NEW_VERTEX) {
      name = "gallium3d_vertex";
   } else if (type == I915_NEW_SCANOUT) {
      name = "gallium3d_scanout";
   } else {
      assert(0);
      name = "gallium3d_unknown";
   }

   return name;
}

static struct i915_winsys_buffer *
i915_drm_buffer_create(struct i915_winsys *iws,
                        unsigned size,
                        enum i915_winsys_buffer_type type)
{
   struct i915_drm_buffer *buf = CALLOC_STRUCT(i915_drm_buffer);
   struct i915_drm_winsys *idws = i915_drm_winsys(iws);

   if (!buf)
      return NULL;

   buf->magic = 0xDEAD1337;
   buf->flinked = false;
   buf->flink = 0;

   buf->bo = drm_intel_bo_alloc(idws->gem_manager,
                                i915_drm_type_to_name(type), size, 0);

   if (!buf->bo)
      goto err;

   return (struct i915_winsys_buffer *)buf;

err:
   assert(0);
   FREE(buf);
   return NULL;
}

static struct i915_winsys_buffer *
i915_drm_buffer_create_tiled(struct i915_winsys *iws,
                             unsigned *stride, unsigned height, 
                             enum i915_winsys_buffer_tile *tiling,
                             enum i915_winsys_buffer_type type)
{
   struct i915_drm_buffer *buf = CALLOC_STRUCT(i915_drm_buffer);
   struct i915_drm_winsys *idws = i915_drm_winsys(iws);
   unsigned long pitch = 0;
   uint32_t tiling_mode = *tiling;

   if (!buf)
      return NULL;

   buf->magic = 0xDEAD1337;
   buf->flinked = false;
   buf->flink = 0;

   buf->bo = drm_intel_bo_alloc_tiled(idws->gem_manager,
                                      i915_drm_type_to_name(type),
                                      *stride, height, 1,
                                      &tiling_mode, &pitch, 0);

   if (!buf->bo)
      goto err;

   *stride = pitch;
   *tiling = tiling_mode;
   return (struct i915_winsys_buffer *)buf;

err:
   assert(0);
   FREE(buf);
   return NULL;
}

static struct i915_winsys_buffer *
i915_drm_buffer_from_handle(struct i915_winsys *iws,
                            struct winsys_handle *whandle,
                            unsigned height,
                            enum i915_winsys_buffer_tile *tiling,
                            unsigned *stride)
{
   struct i915_drm_winsys *idws = i915_drm_winsys(iws);
   struct i915_drm_buffer *buf;
   uint32_t tile = 0, swizzle = 0;

   if ((whandle->type != WINSYS_HANDLE_TYPE_SHARED) && (whandle->type != WINSYS_HANDLE_TYPE_FD))
      return NULL;

   if (whandle->offset != 0)
      return NULL;

   buf = CALLOC_STRUCT(i915_drm_buffer);
   if (!buf)
      return NULL;

   buf->magic = 0xDEAD1337;

   if (whandle->type == WINSYS_HANDLE_TYPE_SHARED)
       buf->bo = drm_intel_bo_gem_create_from_name(idws->gem_manager, "gallium3d_from_handle", whandle->handle);
   else if (whandle->type == WINSYS_HANDLE_TYPE_FD) {
       int fd = (int) whandle->handle;
       buf->bo = drm_intel_bo_gem_create_from_prime(idws->gem_manager, fd, height * whandle->stride);
   }

   buf->flinked = true;
   buf->flink = whandle->handle;

   if (!buf->bo)
      goto err;

   drm_intel_bo_get_tiling(buf->bo, &tile, &swizzle);

   *stride = whandle->stride;
   *tiling = tile;

   return (struct i915_winsys_buffer *)buf;

err:
   FREE(buf);
   return NULL;
}

static bool
i915_drm_buffer_get_handle(struct i915_winsys *iws,
                            struct i915_winsys_buffer *buffer,
                            struct winsys_handle *whandle,
                            unsigned stride)
{
   struct i915_drm_buffer *buf = i915_drm_buffer(buffer);

   if (whandle->type == WINSYS_HANDLE_TYPE_SHARED) {
      if (!buf->flinked) {
         if (drm_intel_bo_flink(buf->bo, &buf->flink))
            return false;
         buf->flinked = true;
      }

      whandle->handle = buf->flink;
   } else if (whandle->type == WINSYS_HANDLE_TYPE_KMS) {
      whandle->handle = buf->bo->handle;
   } else if (whandle->type == WINSYS_HANDLE_TYPE_FD) {
      int fd;

      if (drm_intel_bo_gem_export_to_prime(buf->bo, &fd))
         return false;
      whandle->handle = fd;
   } else {
      assert(!"unknown usage");
      return false;
   }

   whandle->stride = stride;
   return true;
}

static void *
i915_drm_buffer_map(struct i915_winsys *iws,
                     struct i915_winsys_buffer *buffer,
                     bool write)
{
   struct i915_drm_buffer *buf = i915_drm_buffer(buffer);
   drm_intel_bo *bo = intel_bo(buffer);
   int ret = 0;

   assert(bo);

   if (buf->map_count)
      goto out;

   ret = drm_intel_gem_bo_map_gtt(bo);

   buf->ptr = bo->virtual;

   assert(ret == 0);
out:
   if (ret)
      return NULL;

   buf->map_count++;
   return buf->ptr;
}

static void
i915_drm_buffer_unmap(struct i915_winsys *iws,
                       struct i915_winsys_buffer *buffer)
{
   struct i915_drm_buffer *buf = i915_drm_buffer(buffer);

   if (--buf->map_count)
      return;

   drm_intel_gem_bo_unmap_gtt(intel_bo(buffer));
}

static int
i915_drm_buffer_write(struct i915_winsys *iws,
                       struct i915_winsys_buffer *buffer,
                       size_t offset,
                       size_t size,
                       const void *data)
{
   struct i915_drm_buffer *buf = i915_drm_buffer(buffer);

   return drm_intel_bo_subdata(buf->bo, offset, size, (void*)data);
}

static void
i915_drm_buffer_destroy(struct i915_winsys *iws,
                         struct i915_winsys_buffer *buffer)
{
   drm_intel_bo_unreference(intel_bo(buffer));

#ifdef DEBUG
   i915_drm_buffer(buffer)->magic = 0;
   i915_drm_buffer(buffer)->bo = NULL;
#endif

   FREE(buffer);
}

static bool
i915_drm_buffer_is_busy(struct i915_winsys *iws,
                        struct i915_winsys_buffer *buffer)
{
   struct i915_drm_buffer* i915_buffer = i915_drm_buffer(buffer);
   if (!i915_buffer)
      return false;
   return drm_intel_bo_busy(i915_buffer->bo);
}


void
i915_drm_winsys_init_buffer_functions(struct i915_drm_winsys *idws)
{
   idws->base.buffer_create = i915_drm_buffer_create;
   idws->base.buffer_create_tiled = i915_drm_buffer_create_tiled;
   idws->base.buffer_from_handle = i915_drm_buffer_from_handle;
   idws->base.buffer_get_handle = i915_drm_buffer_get_handle;
   idws->base.buffer_map = i915_drm_buffer_map;
   idws->base.buffer_unmap = i915_drm_buffer_unmap;
   idws->base.buffer_write = i915_drm_buffer_write;
   idws->base.buffer_destroy = i915_drm_buffer_destroy;
   idws->base.buffer_is_busy = i915_drm_buffer_is_busy;
}
