
#ifndef I915_DRM_PUBLIC_H
#define I915_DRM_PUBLIC_H

struct i915_winsys;

struct i915_winsys * i915_drm_winsys_create(int drmFD);

#endif
