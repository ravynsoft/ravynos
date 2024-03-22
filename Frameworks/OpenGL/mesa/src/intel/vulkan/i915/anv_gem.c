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
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "i915/anv_gem.h"
#include "anv_private.h"

#include "drm-uapi/i915_drm.h"

int
anv_i915_gem_get_tiling(struct anv_device *device, uint32_t gem_handle)
{
   if (!device->info->has_tiling_uapi)
      return -1;

   struct drm_i915_gem_get_tiling get_tiling = {
      .handle = gem_handle,
   };

   /* FIXME: On discrete platforms we don't have DRM_IOCTL_I915_GEM_GET_TILING
    * anymore, so we will need another way to get the tiling. Apparently this
    * is only used in Android code, so we may need some other way to
    * communicate the tiling mode.
    */
   if (intel_ioctl(device->fd, DRM_IOCTL_I915_GEM_GET_TILING, &get_tiling)) {
      assert(!"Failed to get BO tiling");
      return -1;
   }

   return get_tiling.tiling_mode;
}

int
anv_i915_gem_set_tiling(struct anv_device *device, uint32_t gem_handle,
                        uint32_t stride, uint32_t tiling)
{
   /* On discrete platforms we don't have DRM_IOCTL_I915_GEM_SET_TILING. So
    * nothing needs to be done.
    */
   if (!device->info->has_tiling_uapi)
      return 0;

   /* set_tiling overwrites the input on the error path, so we have to open
    * code intel_ioctl.
    */
   struct drm_i915_gem_set_tiling set_tiling = {
      .handle = gem_handle,
      .tiling_mode = tiling,
      .stride = stride,
   };

   return intel_ioctl(device->fd, DRM_IOCTL_I915_GEM_SET_TILING, &set_tiling);
}

int
anv_i915_gem_wait(struct anv_device *device, uint32_t gem_handle,
                  int64_t *timeout_ns)
{
   struct drm_i915_gem_wait wait = {
      .bo_handle = gem_handle,
      .timeout_ns = *timeout_ns,
      .flags = 0,
   };

   int ret = intel_ioctl(device->fd, DRM_IOCTL_I915_GEM_WAIT, &wait);
   *timeout_ns = wait.timeout_ns;

   return ret;
}

VkResult
anv_i915_gem_import_bo_alloc_flags_to_bo_flags(struct anv_device *device,
                                               struct anv_bo *bo,
                                               enum anv_bo_alloc_flags alloc_flags,
                                               uint32_t *out_bo_flags)
{
   const uint32_t bo_flags =
         device->kmd_backend->bo_alloc_flags_to_bo_flags(device, alloc_flags);
   if (bo->refcount == 0) {
      *out_bo_flags = bo_flags;
      return VK_SUCCESS;
   }

   /* We have to be careful how we combine flags so that it makes sense.
    * Really, though, if we get to this case and it actually matters, the
    * client has imported a BO twice in different ways and they get what
    * they have coming.
    */
   uint32_t new_flags = 0;
   new_flags |= (bo->flags | bo_flags) & EXEC_OBJECT_WRITE;
   new_flags |= (bo->flags & bo_flags) & EXEC_OBJECT_ASYNC;
   new_flags |= (bo->flags & bo_flags) & EXEC_OBJECT_SUPPORTS_48B_ADDRESS;
   new_flags |= (bo->flags | bo_flags) & EXEC_OBJECT_PINNED;
   new_flags |= (bo->flags | bo_flags) & EXEC_OBJECT_CAPTURE;

   /* It's theoretically possible for a BO to get imported such that it's
    * both pinned and not pinned.  The only way this can happen is if it
    * gets imported as both a semaphore and a memory object and that would
    * be an application error.  Just fail out in that case.
    */
   if ((bo->flags & EXEC_OBJECT_PINNED) !=
       (bo_flags & EXEC_OBJECT_PINNED))
      return vk_errorf(device, VK_ERROR_INVALID_EXTERNAL_HANDLE,
                       "The same BO was imported two different ways");

   /* It's also theoretically possible that someone could export a BO from
    * one heap and import it into another or to import the same BO into two
    * different heaps.  If this happens, we could potentially end up both
    * allowing and disallowing 48-bit addresses.  There's not much we can
    * do about it if we're pinning so we just throw an error and hope no
    * app is actually that stupid.
    */
   if ((new_flags & EXEC_OBJECT_PINNED) &&
       (bo->flags & EXEC_OBJECT_SUPPORTS_48B_ADDRESS) !=
       (bo_flags & EXEC_OBJECT_SUPPORTS_48B_ADDRESS))
      return vk_errorf(device, VK_ERROR_INVALID_EXTERNAL_HANDLE,
                       "The same BO was imported on two different heaps");

   *out_bo_flags = new_flags;
   return VK_SUCCESS;
}
