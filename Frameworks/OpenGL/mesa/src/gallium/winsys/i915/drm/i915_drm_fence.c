
#include "i915_drm_winsys.h"
#include "util/u_memory.h"
#include "util/u_atomic.h"
#include "util/u_inlines.h"

/**
 * Because gem does not have fence's we have to create our own fences.
 *
 * They work by keeping the batchbuffer around and checking if that has
 * been idled. If bo is NULL fence has expired.
 */
struct i915_drm_fence
{
   struct pipe_reference reference;
   drm_intel_bo *bo;
};


struct pipe_fence_handle *
i915_drm_fence_create(drm_intel_bo *bo)
{
   struct i915_drm_fence *fence = CALLOC_STRUCT(i915_drm_fence);

   pipe_reference_init(&fence->reference, 1);
   /* bo is null if fence already expired */
   if (bo) {
      drm_intel_bo_reference(bo);
      fence->bo = bo;
   }

   return (struct pipe_fence_handle *)fence;
}

static void
i915_drm_fence_reference(struct i915_winsys *iws,
                          struct pipe_fence_handle **ptr,
                          struct pipe_fence_handle *fence)
{
   struct i915_drm_fence *old = (struct i915_drm_fence *)*ptr;
   struct i915_drm_fence *f = (struct i915_drm_fence *)fence;

   if (pipe_reference(&((struct i915_drm_fence *)(*ptr))->reference, &f->reference)) {
      if (old->bo)
         drm_intel_bo_unreference(old->bo);
      FREE(old);
   }
   *ptr = fence;
}

static int
i915_drm_fence_signalled(struct i915_winsys *iws,
                          struct pipe_fence_handle *fence)
{
   struct i915_drm_fence *f = (struct i915_drm_fence *)fence;

   /* fence already expired */
   if (!f->bo)
	   return 1;

   return !drm_intel_bo_busy(f->bo);
}

static int
i915_drm_fence_finish(struct i915_winsys *iws,
                       struct pipe_fence_handle *fence)
{
   struct i915_drm_fence *f = (struct i915_drm_fence *)fence;

   /* fence already expired */
   if (!f->bo)
      return 0;

   drm_intel_bo_wait_rendering(f->bo);
   drm_intel_bo_unreference(f->bo);
   f->bo = NULL;

   return 0;
}

void
i915_drm_winsys_init_fence_functions(struct i915_drm_winsys *idws)
{
   idws->base.fence_reference = i915_drm_fence_reference;
   idws->base.fence_signalled = i915_drm_fence_signalled;
   idws->base.fence_finish = i915_drm_fence_finish;
}
