
#ifndef INTEL_DRM_WINSYS_H
#define INTEL_DRM_WINSYS_H

#include "i915/i915_batchbuffer.h"

#include "drm-uapi/drm.h"
#include "intel_bufmgr.h"


/*
 * Winsys
 */


struct i915_drm_winsys
{
   struct i915_winsys base;

   bool dump_cmd;
   const char *dump_raw_file;
   bool send_cmd;

   int fd; /**< Drm file discriptor */

   size_t max_batch_size;

   drm_intel_bufmgr *gem_manager;
};

static inline struct i915_drm_winsys *
i915_drm_winsys(struct i915_winsys *iws)
{
   return (struct i915_drm_winsys *)iws;
}

struct pipe_fence_handle * i915_drm_fence_create(drm_intel_bo *bo);

void i915_drm_winsys_init_batchbuffer_functions(struct i915_drm_winsys *idws);
void i915_drm_winsys_init_buffer_functions(struct i915_drm_winsys *idws);
void i915_drm_winsys_init_fence_functions(struct i915_drm_winsys *idws);


/*
 * Buffer
 */


struct i915_drm_buffer {
   unsigned magic;

   drm_intel_bo *bo;

   void *ptr;
   unsigned map_count;

   bool flinked;
   unsigned flink;
};

static inline struct i915_drm_buffer *
i915_drm_buffer(struct i915_winsys_buffer *buffer)
{
   return (struct i915_drm_buffer *)buffer;
}

static inline drm_intel_bo *
intel_bo(struct i915_winsys_buffer *buffer)
{
   return i915_drm_buffer(buffer)->bo;
}

#endif
