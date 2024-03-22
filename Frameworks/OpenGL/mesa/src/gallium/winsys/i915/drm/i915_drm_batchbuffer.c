
#include "i915_drm_winsys.h"
#include "util/u_memory.h"

#include "drm-uapi/i915_drm.h"
#include "i915/i915_debug.h"
#include <xf86drm.h>
#include <stdio.h>

#define BATCH_RESERVED 16

#define INTEL_DEFAULT_RELOCS 100
#define INTEL_MAX_RELOCS 400

#define INTEL_BATCH_NO_CLIPRECTS 0x1
#define INTEL_BATCH_CLIPRECTS    0x2

#undef INTEL_RUN_SYNC

struct i915_drm_batchbuffer
{
   struct i915_winsys_batchbuffer base;

   size_t actual_size;

   drm_intel_bo *bo;
};

static inline struct i915_drm_batchbuffer *
i915_drm_batchbuffer(struct i915_winsys_batchbuffer *batch)
{
   return (struct i915_drm_batchbuffer *)batch;
}

static void
i915_drm_batchbuffer_reset(struct i915_drm_batchbuffer *batch)
{
   struct i915_drm_winsys *idws = i915_drm_winsys(batch->base.iws);

   if (batch->bo)
      drm_intel_bo_unreference(batch->bo);
   batch->bo = drm_intel_bo_alloc(idws->gem_manager,
                                  "gallium3d_batchbuffer",
                                  batch->actual_size,
                                  4096);

   memset(batch->base.map, 0, batch->actual_size);
   batch->base.ptr = batch->base.map;
   batch->base.size = batch->actual_size - BATCH_RESERVED;
   batch->base.relocs = 0;
}

static struct i915_winsys_batchbuffer *
i915_drm_batchbuffer_create(struct i915_winsys *iws)
{
   struct i915_drm_winsys *idws = i915_drm_winsys(iws);
   struct i915_drm_batchbuffer *batch = CALLOC_STRUCT(i915_drm_batchbuffer);

   batch->actual_size = idws->max_batch_size;

   batch->base.map = MALLOC(batch->actual_size);
   batch->base.ptr = NULL;
   batch->base.size = 0;

   batch->base.relocs = 0;

   batch->base.iws = iws;

   i915_drm_batchbuffer_reset(batch);

   return &batch->base;
}

static bool
i915_drm_batchbuffer_validate_buffers(struct i915_winsys_batchbuffer *batch,
				      struct i915_winsys_buffer **buffer,
				      int num_of_buffers)
{
   struct i915_drm_batchbuffer *drm_batch = i915_drm_batchbuffer(batch);
   drm_intel_bo *bos[num_of_buffers + 1];
   int i, ret;

   bos[0] = drm_batch->bo;
   for (i = 0; i < num_of_buffers; i++)
      bos[i+1] = intel_bo(buffer[i]);

   ret = drm_intel_bufmgr_check_aperture_space(bos, num_of_buffers);
   if (ret != 0)
      return false;

   return true;
}

static int
i915_drm_batchbuffer_reloc(struct i915_winsys_batchbuffer *ibatch,
                            struct i915_winsys_buffer *buffer,
                            enum i915_winsys_buffer_usage usage,
                            unsigned pre_add, bool fenced)
{
   struct i915_drm_batchbuffer *batch = i915_drm_batchbuffer(ibatch);
   unsigned write_domain = 0;
   unsigned read_domain = 0;
   unsigned offset;
   int ret = 0;

   switch (usage) {
   case I915_USAGE_SAMPLER:
      write_domain = 0;
      read_domain = I915_GEM_DOMAIN_SAMPLER;
      break;
   case I915_USAGE_RENDER:
      write_domain = I915_GEM_DOMAIN_RENDER;
      read_domain = I915_GEM_DOMAIN_RENDER;
      break;
   case I915_USAGE_2D_TARGET:
      write_domain = I915_GEM_DOMAIN_RENDER;
      read_domain = I915_GEM_DOMAIN_RENDER;
      break;
   case I915_USAGE_2D_SOURCE:
      write_domain = 0;
      read_domain = I915_GEM_DOMAIN_RENDER;
      break;
   case I915_USAGE_VERTEX:
      write_domain = 0;
      read_domain = I915_GEM_DOMAIN_VERTEX;
      break;
   default:
      assert(0);
      return -1;
   }

   offset = (unsigned)(batch->base.ptr - batch->base.map);

   if (fenced)
      ret = drm_intel_bo_emit_reloc_fence(batch->bo, offset,
				    intel_bo(buffer), pre_add,
				    read_domain,
				    write_domain);
   else
      ret = drm_intel_bo_emit_reloc(batch->bo, offset,
				    intel_bo(buffer), pre_add,
				    read_domain,
				    write_domain);

   ((uint32_t*)batch->base.ptr)[0] = intel_bo(buffer)->offset + pre_add;
   batch->base.ptr += 4;

   if (!ret)
      batch->base.relocs++;

   return ret;
}

static void
i915_drm_throttle(struct i915_drm_winsys *idws)
{
   drmIoctl(idws->fd, DRM_IOCTL_I915_GEM_THROTTLE, NULL);
}

static void
i915_drm_batchbuffer_flush(struct i915_winsys_batchbuffer *ibatch,
                           struct pipe_fence_handle **fence,
                           enum i915_winsys_flush_flags flags)
{
   struct i915_drm_batchbuffer *batch = i915_drm_batchbuffer(ibatch);
   unsigned used;
   int ret;

   /* MI_BATCH_BUFFER_END */
   i915_winsys_batchbuffer_dword_unchecked(ibatch, (0xA<<23));

   used = batch->base.ptr - batch->base.map;
   if (used & 4) {
      /* MI_NOOP */
      i915_winsys_batchbuffer_dword_unchecked(ibatch, 0);
      used += 4;
   }

   /* Do the sending to HW */
   ret = drm_intel_bo_subdata(batch->bo, 0, used, batch->base.map);
   if (ret == 0 && i915_drm_winsys(ibatch->iws)->send_cmd)
      ret = drm_intel_bo_exec(batch->bo, used, NULL, 0, 0);

   if (flags & I915_FLUSH_END_OF_FRAME)
      i915_drm_throttle(i915_drm_winsys(ibatch->iws));

   if (ret != 0 || i915_drm_winsys(ibatch->iws)->dump_cmd) {
      i915_dump_batchbuffer(ibatch);
      assert(ret == 0);
   }

   if (i915_drm_winsys(ibatch->iws)->dump_raw_file) {
      FILE *file = fopen(i915_drm_winsys(ibatch->iws)->dump_raw_file, "a");
      if (file) {
	 fwrite(batch->base.map, used, 1, file);
	 fclose(file);
      }
   }

#ifdef INTEL_RUN_SYNC
   drm_intel_bo_wait_rendering(batch->bo);
#endif

   if (fence) {
      ibatch->iws->fence_reference(ibatch->iws, fence, NULL);

#ifdef INTEL_RUN_SYNC
      /* we run synced to GPU so just pass null */
      (*fence) = i915_drm_fence_create(NULL);
#else
      (*fence) = i915_drm_fence_create(batch->bo);
#endif
   }

   i915_drm_batchbuffer_reset(batch);
}

static void
i915_drm_batchbuffer_destroy(struct i915_winsys_batchbuffer *ibatch)
{
   struct i915_drm_batchbuffer *batch = i915_drm_batchbuffer(ibatch);

   if (batch->bo)
      drm_intel_bo_unreference(batch->bo);

   FREE(batch->base.map);
   FREE(batch);
}

void i915_drm_winsys_init_batchbuffer_functions(struct i915_drm_winsys *idws)
{
   idws->base.batchbuffer_create = i915_drm_batchbuffer_create;
   idws->base.validate_buffers = i915_drm_batchbuffer_validate_buffers;
   idws->base.batchbuffer_reloc = i915_drm_batchbuffer_reloc;
   idws->base.batchbuffer_flush = i915_drm_batchbuffer_flush;
   idws->base.batchbuffer_destroy = i915_drm_batchbuffer_destroy;
}
