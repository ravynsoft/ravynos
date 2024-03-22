/*
 * Copyright © 2023 Intel Corporation
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */
#include "i915/iris_bufmgr.h"

#include "common/intel_gem.h"
#include "iris/iris_bufmgr.h"

#include "drm-uapi/i915_drm.h"

bool iris_i915_bo_busy_gem(struct iris_bo *bo)
{
   assert(iris_bo_is_real(bo));

   struct iris_bufmgr *bufmgr = bo->bufmgr;
   struct drm_i915_gem_busy busy = { .handle = bo->gem_handle };

   if (intel_ioctl(iris_bufmgr_get_fd(bufmgr), DRM_IOCTL_I915_GEM_BUSY, &busy))
      return false;

   return busy.busy;
}

int iris_i915_bo_wait_gem(struct iris_bo *bo, int64_t timeout_ns)
{
   assert(iris_bo_is_real(bo));

   struct iris_bufmgr *bufmgr = bo->bufmgr;
   struct drm_i915_gem_wait wait = {
      .bo_handle = bo->gem_handle,
      .timeout_ns = timeout_ns,
   };

   if (intel_ioctl(iris_bufmgr_get_fd(bufmgr), DRM_IOCTL_I915_GEM_WAIT, &wait))
      return -errno;

   return 0;
}

bool iris_i915_init_global_vm(struct iris_bufmgr *bufmgr, uint32_t *vm_id)
{
   uint64_t value;
   bool ret = intel_gem_get_context_param(iris_bufmgr_get_fd(bufmgr), 0,
                                          I915_CONTEXT_PARAM_VM, &value);
   if (ret)
      *vm_id = value;
   return ret;
}
