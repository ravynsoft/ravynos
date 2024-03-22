/*
 * Copyright © 2021 Intel Corporation
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

#include "vk_sync.h"

#include <assert.h>
#include <string.h>

#include "util/u_debug.h"
#include "util/macros.h"
#include "util/os_time.h"

#include "vk_alloc.h"
#include "vk_device.h"
#include "vk_log.h"

static void
vk_sync_type_validate(const struct vk_sync_type *type)
{
   assert(type->init);
   assert(type->finish);

   assert(type->features & (VK_SYNC_FEATURE_BINARY |
                            VK_SYNC_FEATURE_TIMELINE));

   if (type->features & VK_SYNC_FEATURE_TIMELINE) {
      assert(type->features & VK_SYNC_FEATURE_GPU_WAIT);
      assert(type->features & VK_SYNC_FEATURE_CPU_WAIT);
      assert(type->features & VK_SYNC_FEATURE_CPU_SIGNAL);
      assert(type->features & (VK_SYNC_FEATURE_WAIT_BEFORE_SIGNAL |
                               VK_SYNC_FEATURE_WAIT_PENDING));
      assert(type->signal);
      assert(type->get_value);
   }

   if (!(type->features & VK_SYNC_FEATURE_BINARY)) {
      assert(!(type->features & (VK_SYNC_FEATURE_GPU_MULTI_WAIT |
                                 VK_SYNC_FEATURE_CPU_RESET)));
      assert(!type->import_sync_file);
      assert(!type->export_sync_file);
   }

   if (type->features & VK_SYNC_FEATURE_CPU_WAIT) {
      assert(type->wait || type->wait_many);
   } else {
      assert(!(type->features & (VK_SYNC_FEATURE_WAIT_ANY |
                                 VK_SYNC_FEATURE_WAIT_PENDING)));
   }

   if (type->features & VK_SYNC_FEATURE_GPU_MULTI_WAIT)
      assert(type->features & VK_SYNC_FEATURE_GPU_WAIT);

   if (type->features & VK_SYNC_FEATURE_CPU_RESET)
      assert(type->reset);

   if (type->features & VK_SYNC_FEATURE_CPU_SIGNAL)
      assert(type->signal);
}

VkResult
vk_sync_init(struct vk_device *device,
             struct vk_sync *sync,
             const struct vk_sync_type *type,
             enum vk_sync_flags flags,
             uint64_t initial_value)
{
   vk_sync_type_validate(type);

   if (flags & VK_SYNC_IS_TIMELINE)
      assert(type->features & VK_SYNC_FEATURE_TIMELINE);
   else
      assert(type->features & VK_SYNC_FEATURE_BINARY);

   assert(type->size >= sizeof(*sync));
   memset(sync, 0, type->size);
   sync->type = type;
   sync->flags = flags;

   return type->init(device, sync, initial_value);
}

void
vk_sync_finish(struct vk_device *device,
               struct vk_sync *sync)
{
   sync->type->finish(device, sync);
}

VkResult
vk_sync_create(struct vk_device *device,
               const struct vk_sync_type *type,
               enum vk_sync_flags flags,
               uint64_t initial_value,
               struct vk_sync **sync_out)
{
   struct vk_sync *sync;

   sync = vk_alloc(&device->alloc, type->size, 8,
                   VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
   if (sync == NULL)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   VkResult result = vk_sync_init(device, sync, type, flags, initial_value);
   if (result != VK_SUCCESS) {
      vk_free(&device->alloc, sync);
      return result;
   }

   *sync_out = sync;

   return VK_SUCCESS;
}

void
vk_sync_destroy(struct vk_device *device,
                struct vk_sync *sync)
{
   vk_sync_finish(device, sync);
   vk_free(&device->alloc, sync);
}

VkResult
vk_sync_signal(struct vk_device *device,
               struct vk_sync *sync,
               uint64_t value)
{
   assert(sync->type->features & VK_SYNC_FEATURE_CPU_SIGNAL);

   if (sync->flags & VK_SYNC_IS_TIMELINE)
      assert(value > 0);
   else
      assert(value == 0);

   return sync->type->signal(device, sync, value);
}

VkResult
vk_sync_get_value(struct vk_device *device,
                  struct vk_sync *sync,
                  uint64_t *value)
{
   assert(sync->flags & VK_SYNC_IS_TIMELINE);
   return sync->type->get_value(device, sync, value);
}

VkResult
vk_sync_reset(struct vk_device *device,
              struct vk_sync *sync)
{
   assert(sync->type->features & VK_SYNC_FEATURE_CPU_RESET);
   assert(!(sync->flags & VK_SYNC_IS_TIMELINE));
   return sync->type->reset(device, sync);
}

VkResult vk_sync_move(struct vk_device *device,
                      struct vk_sync *dst,
                      struct vk_sync *src)
{
   assert(!(dst->flags & VK_SYNC_IS_TIMELINE));
   assert(!(src->flags & VK_SYNC_IS_TIMELINE));
   assert(dst->type == src->type);

   return src->type->move(device, dst, src);
}

static void
assert_valid_wait(struct vk_sync *sync,
                  uint64_t wait_value,
                  enum vk_sync_wait_flags wait_flags)
{
   assert(sync->type->features & VK_SYNC_FEATURE_CPU_WAIT);

   if (!(sync->flags & VK_SYNC_IS_TIMELINE))
      assert(wait_value == 0);

   if (wait_flags & VK_SYNC_WAIT_PENDING)
      assert(sync->type->features & VK_SYNC_FEATURE_WAIT_PENDING);
}

static uint64_t
get_max_abs_timeout_ns(void)
{
   static int max_timeout_ms = -1;
   if (max_timeout_ms < 0)
      max_timeout_ms = debug_get_num_option("MESA_VK_MAX_TIMEOUT", 0);

   if (max_timeout_ms == 0)
      return UINT64_MAX;
   else
      return os_time_get_absolute_timeout(max_timeout_ms * 1000000ull);
}

static VkResult
__vk_sync_wait(struct vk_device *device,
               struct vk_sync *sync,
               uint64_t wait_value,
               enum vk_sync_wait_flags wait_flags,
               uint64_t abs_timeout_ns)
{
   assert_valid_wait(sync, wait_value, wait_flags);

   /* This doesn't make sense for a single wait */
   assert(!(wait_flags & VK_SYNC_WAIT_ANY));

   if (sync->type->wait) {
      return sync->type->wait(device, sync, wait_value,
                              wait_flags, abs_timeout_ns);
   } else {
      struct vk_sync_wait wait = {
         .sync = sync,
         .stage_mask = ~(VkPipelineStageFlags2)0,
         .wait_value = wait_value,
      };
      return sync->type->wait_many(device, 1, &wait, wait_flags,
                                   abs_timeout_ns);
   }
}

VkResult
vk_sync_wait(struct vk_device *device,
             struct vk_sync *sync,
             uint64_t wait_value,
             enum vk_sync_wait_flags wait_flags,
             uint64_t abs_timeout_ns)
{
   uint64_t max_abs_timeout_ns = get_max_abs_timeout_ns();
   if (abs_timeout_ns > max_abs_timeout_ns) {
      VkResult result =
         __vk_sync_wait(device, sync, wait_value, wait_flags,
                        max_abs_timeout_ns);
      if (unlikely(result == VK_TIMEOUT))
         return vk_device_set_lost(device, "Maximum timeout exceeded!");
      return result;
   } else {
      return __vk_sync_wait(device, sync, wait_value, wait_flags,
                            abs_timeout_ns);
   }
}

static bool
can_wait_many(uint32_t wait_count,
              const struct vk_sync_wait *waits,
              enum vk_sync_wait_flags wait_flags)
{
   if (waits[0].sync->type->wait_many == NULL)
      return false;

   if ((wait_flags & VK_SYNC_WAIT_ANY) &&
       !(waits[0].sync->type->features & VK_SYNC_FEATURE_WAIT_ANY))
      return false;

   for (uint32_t i = 0; i < wait_count; i++) {
      assert_valid_wait(waits[i].sync, waits[i].wait_value, wait_flags);
      if (waits[i].sync->type != waits[0].sync->type)
         return false;
   }

   return true;
}

static VkResult
__vk_sync_wait_many(struct vk_device *device,
                    uint32_t wait_count,
                    const struct vk_sync_wait *waits,
                    enum vk_sync_wait_flags wait_flags,
                    uint64_t abs_timeout_ns)
{
   if (wait_count == 0)
      return VK_SUCCESS;

   if (wait_count == 1) {
      return __vk_sync_wait(device, waits[0].sync, waits[0].wait_value,
                            wait_flags & ~VK_SYNC_WAIT_ANY, abs_timeout_ns);
   }

   if (can_wait_many(wait_count, waits, wait_flags)) {
      return waits[0].sync->type->wait_many(device, wait_count, waits,
                                            wait_flags, abs_timeout_ns);
   } else if (wait_flags & VK_SYNC_WAIT_ANY) {
      /* If we have multiple syncs and they don't support wait_any or they're
       * not all the same type, there's nothing better we can do than spin.
       */
      do {
         for (uint32_t i = 0; i < wait_count; i++) {
            VkResult result = __vk_sync_wait(device, waits[i].sync,
                                             waits[i].wait_value,
                                             wait_flags & ~VK_SYNC_WAIT_ANY,
                                             0 /* abs_timeout_ns */);
            if (result != VK_TIMEOUT)
               return result;
         }
      } while (os_time_get_nano() < abs_timeout_ns);

      return VK_TIMEOUT;
   } else {
      for (uint32_t i = 0; i < wait_count; i++) {
         VkResult result = __vk_sync_wait(device, waits[i].sync,
                                          waits[i].wait_value,
                                          wait_flags, abs_timeout_ns);
         if (result != VK_SUCCESS)
            return result;
      }
      return VK_SUCCESS;
   }
}

VkResult
vk_sync_wait_many(struct vk_device *device,
                  uint32_t wait_count,
                  const struct vk_sync_wait *waits,
                  enum vk_sync_wait_flags wait_flags,
                  uint64_t abs_timeout_ns)
{
   uint64_t max_abs_timeout_ns = get_max_abs_timeout_ns();
   if (abs_timeout_ns > max_abs_timeout_ns) {
      VkResult result =
         __vk_sync_wait_many(device, wait_count, waits, wait_flags,
                             max_abs_timeout_ns);
      if (unlikely(result == VK_TIMEOUT))
         return vk_device_set_lost(device, "Maximum timeout exceeded!");
      return result;
   } else {
      return __vk_sync_wait_many(device, wait_count, waits, wait_flags,
                                 abs_timeout_ns);
   }
}

VkResult
vk_sync_import_opaque_fd(struct vk_device *device,
                         struct vk_sync *sync,
                         int fd)
{
   VkResult result = sync->type->import_opaque_fd(device, sync, fd);
   if (unlikely(result != VK_SUCCESS))
      return result;

   sync->flags |= VK_SYNC_IS_SHAREABLE |
                  VK_SYNC_IS_SHARED;

   return VK_SUCCESS;
}

VkResult
vk_sync_export_opaque_fd(struct vk_device *device,
                         struct vk_sync *sync,
                         int *fd)
{
   assert(sync->flags & VK_SYNC_IS_SHAREABLE);

   VkResult result = sync->type->export_opaque_fd(device, sync, fd);
   if (unlikely(result != VK_SUCCESS))
      return result;

   sync->flags |= VK_SYNC_IS_SHARED;

   return VK_SUCCESS;
}

VkResult
vk_sync_import_sync_file(struct vk_device *device,
                         struct vk_sync *sync,
                         int sync_file)
{
   assert(!(sync->flags & VK_SYNC_IS_TIMELINE));

   /* Silently handle negative file descriptors in case the driver doesn't
    * want to bother.
    */
   if (sync_file < 0 && sync->type->signal)
      return sync->type->signal(device, sync, 0);

   return sync->type->import_sync_file(device, sync, sync_file);
}

VkResult
vk_sync_export_sync_file(struct vk_device *device,
                         struct vk_sync *sync,
                         int *sync_file)
{
   assert(!(sync->flags & VK_SYNC_IS_TIMELINE));
   return sync->type->export_sync_file(device, sync, sync_file);
}

VkResult
vk_sync_import_win32_handle(struct vk_device *device,
                            struct vk_sync *sync,
                            void *handle,
                            const wchar_t *name)
{
   VkResult result = sync->type->import_win32_handle(device, sync, handle, name);
   if (unlikely(result != VK_SUCCESS))
      return result;

   sync->flags |= VK_SYNC_IS_SHAREABLE |
                  VK_SYNC_IS_SHARED;

   return VK_SUCCESS;
}

VkResult
vk_sync_export_win32_handle(struct vk_device *device,
                            struct vk_sync *sync,
                            void **handle)
{
   assert(sync->flags & VK_SYNC_IS_SHAREABLE);

   VkResult result = sync->type->export_win32_handle(device, sync, handle);
   if (unlikely(result != VK_SUCCESS))
      return result;

   sync->flags |= VK_SYNC_IS_SHARED;

   return VK_SUCCESS;
}

VkResult
vk_sync_set_win32_export_params(struct vk_device *device,
                                struct vk_sync *sync,
                                const void *security_attributes,
                                uint32_t access,
                                const wchar_t *name)
{
   assert(sync->flags & VK_SYNC_IS_SHARED);

   return sync->type->set_win32_export_params(device, sync, security_attributes, access, name);
}
