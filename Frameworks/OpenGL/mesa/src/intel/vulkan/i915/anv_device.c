/*
 * Copyright © 2022 Intel Corporation
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

#include "i915/anv_device.h"
#include "anv_private.h"

#include "common/intel_defines.h"
#include "common/i915/intel_gem.h"

#include "drm-uapi/i915_drm.h"

static int
vk_priority_to_i915(VkQueueGlobalPriorityKHR priority)
{
   switch (priority) {
   case VK_QUEUE_GLOBAL_PRIORITY_LOW_KHR:
      return INTEL_CONTEXT_LOW_PRIORITY;
   case VK_QUEUE_GLOBAL_PRIORITY_MEDIUM_KHR:
      return INTEL_CONTEXT_MEDIUM_PRIORITY;
   case VK_QUEUE_GLOBAL_PRIORITY_HIGH_KHR:
      return INTEL_CONTEXT_HIGH_PRIORITY;
   case VK_QUEUE_GLOBAL_PRIORITY_REALTIME_KHR:
      return INTEL_CONTEXT_REALTIME_PRIORITY;
   default:
      unreachable("Invalid priority");
   }
}

int
anv_gem_set_context_param(int fd, uint32_t context, uint32_t param, uint64_t value)
{
   if (param == I915_CONTEXT_PARAM_PRIORITY)
      value = vk_priority_to_i915(value);

   int err = 0;
   if (!intel_gem_set_context_param(fd, context, param, value))
      err = -errno;
   return err;
}

static bool
anv_gem_has_context_priority(int fd, VkQueueGlobalPriorityKHR priority)
{
   return !anv_gem_set_context_param(fd, 0, I915_CONTEXT_PARAM_PRIORITY,
                                     priority);
}

VkResult
anv_i915_physical_device_get_parameters(struct anv_physical_device *device)
{
   VkResult result = VK_SUCCESS;
   int val, fd = device->local_fd;
   uint64_t value;

   if (!intel_gem_get_param(fd, I915_PARAM_HAS_WAIT_TIMEOUT, &val) || !val) {
       result = vk_errorf(device, VK_ERROR_INITIALIZATION_FAILED,
                          "kernel missing gem wait");
       return result;
   }

   if (!intel_gem_get_param(fd, I915_PARAM_HAS_EXECBUF2, &val) || !val) {
      result = vk_errorf(device, VK_ERROR_INITIALIZATION_FAILED,
                         "kernel missing execbuf2");
      return result;
   }

   if (!device->info.has_llc &&
       (!intel_gem_get_param(fd, I915_PARAM_MMAP_VERSION, &val) || val < 1)) {
       result = vk_errorf(device, VK_ERROR_INITIALIZATION_FAILED,
                          "kernel missing wc mmap");
       return result;
   }

   if (!intel_gem_get_param(fd, I915_PARAM_HAS_EXEC_SOFTPIN, &val) || !val) {
      result = vk_errorf(device, VK_ERROR_INITIALIZATION_FAILED,
                         "kernel missing softpin");
      return result;
   }

   if (!intel_gem_get_param(fd, I915_PARAM_HAS_EXEC_FENCE_ARRAY, &val) || !val) {
      result = vk_errorf(device, VK_ERROR_INITIALIZATION_FAILED,
                         "kernel missing syncobj support");
      return result;
   }

   if (intel_gem_get_param(fd, I915_PARAM_HAS_EXEC_ASYNC, &val))
      device->has_exec_async = val;
   if (intel_gem_get_param(fd, I915_PARAM_HAS_EXEC_CAPTURE, &val))
      device->has_exec_capture = val;

   /* Start with medium; sorted low to high */
   const VkQueueGlobalPriorityKHR priorities[] = {
         VK_QUEUE_GLOBAL_PRIORITY_LOW_KHR,
         VK_QUEUE_GLOBAL_PRIORITY_MEDIUM_KHR,
         VK_QUEUE_GLOBAL_PRIORITY_HIGH_KHR,
         VK_QUEUE_GLOBAL_PRIORITY_REALTIME_KHR,
   };
   device->max_context_priority = VK_QUEUE_GLOBAL_PRIORITY_LOW_KHR;
   for (unsigned i = 0; i < ARRAY_SIZE(priorities); i++) {
      if (!anv_gem_has_context_priority(fd, priorities[i]))
         break;
      device->max_context_priority = priorities[i];
   }

   if (intel_gem_get_param(fd, I915_PARAM_HAS_EXEC_TIMELINE_FENCES, &val))
      device->has_exec_timeline = val;

   if (intel_gem_get_context_param(fd, 0, I915_CONTEXT_PARAM_VM, &value))
      device->has_vm_control = value;

   return result;
}

VkResult
anv_i915_physical_device_init_memory_types(struct anv_physical_device *device)
{
   if (anv_physical_device_has_vram(device)) {
      device->memory.type_count = 3;
      device->memory.types[0] = (struct anv_memory_type) {
         .propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
         .heapIndex = 0,
      };
      device->memory.types[1] = (struct anv_memory_type) {
         .propertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                          VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
         .heapIndex = 1,
      };
      device->memory.types[2] = (struct anv_memory_type) {
         .propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
         /* This memory type either comes from heaps[0] if there is only
          * mappable vram region, or from heaps[2] if there is both mappable &
          * non-mappable vram regions.
          */
         .heapIndex = device->vram_non_mappable.size > 0 ? 2 : 0,
      };
   } else if (device->info.has_llc) {
      /* Big core GPUs share LLC with the CPU and thus one memory type can be
       * both cached and coherent at the same time.
       *
       * But some game engines can't handle single type well
       * https://gitlab.freedesktop.org/mesa/mesa/-/issues/7360#note_1719438
       *
       * The second memory type w/out HOST_CACHED_BIT will get write-combining.
       * See anv_AllocateMemory()).
       *
       * The Intel Vulkan driver for Windows also advertises these memory types.
       */
      device->memory.type_count = 3;
      device->memory.types[0] = (struct anv_memory_type) {
         .propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
         .heapIndex = 0,
      };
      device->memory.types[1] = (struct anv_memory_type) {
         .propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
         .heapIndex = 0,
      };
      device->memory.types[2] = (struct anv_memory_type) {
         .propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                          VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
         .heapIndex = 0,
      };
   } else {
      /* The spec requires that we expose a host-visible, coherent memory
       * type, but Atom GPUs don't share LLC. Thus we offer two memory types
       * to give the application a choice between cached, but not coherent and
       * coherent but uncached (WC though).
       */
      device->memory.type_count = 2;
      device->memory.types[0] = (struct anv_memory_type) {
         .propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
         .heapIndex = 0,
      };
      device->memory.types[1] = (struct anv_memory_type) {
         .propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                          VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
         .heapIndex = 0,
      };
   }

   if (device->has_protected_contexts) {
      /* Add a memory type for protected buffers, local and not host
       * visible.
       */
      device->memory.types[device->memory.type_count++] =
         (struct anv_memory_type) {
            .propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
                             VK_MEMORY_PROPERTY_PROTECTED_BIT,
            .heapIndex = 0,
      };
   }

   return VK_SUCCESS;
}

VkResult
anv_i915_set_queue_parameters(
      struct anv_device *device,
      uint32_t context_id,
      const VkDeviceQueueGlobalPriorityCreateInfoKHR *queue_priority)
{
   struct anv_physical_device *physical_device = device->physical;

   /* Here we tell the kernel not to attempt to recover our context but
    * immediately (on the next batchbuffer submission) report that the
    * context is lost, and we will do the recovery ourselves.  In the case
    * of Vulkan, recovery means throwing VK_ERROR_DEVICE_LOST and letting
    * the client clean up the pieces.
    */
   anv_gem_set_context_param(device->fd, context_id,
                             I915_CONTEXT_PARAM_RECOVERABLE, false);

   VkQueueGlobalPriorityKHR priority =
      queue_priority ? queue_priority->globalPriority :
         VK_QUEUE_GLOBAL_PRIORITY_MEDIUM_KHR;

   /* As per spec, the driver implementation may deny requests to acquire
    * a priority above the default priority (MEDIUM) if the caller does not
    * have sufficient privileges. In this scenario VK_ERROR_NOT_PERMITTED_KHR
    * is returned.
    */
   if (physical_device->max_context_priority >= VK_QUEUE_GLOBAL_PRIORITY_MEDIUM_KHR) {
      int err = anv_gem_set_context_param(device->fd, context_id,
                                          I915_CONTEXT_PARAM_PRIORITY,
                                          priority);
      if (err != 0 && priority > VK_QUEUE_GLOBAL_PRIORITY_MEDIUM_KHR) {
         return vk_error(device, VK_ERROR_NOT_PERMITTED_KHR);
      }
   }

   return VK_SUCCESS;
}

VkResult
anv_i915_device_setup_context(struct anv_device *device,
                              const VkDeviceCreateInfo *pCreateInfo,
                              const uint32_t num_queues)
{
   device->protected_session_id = I915_PROTECTED_CONTENT_DEFAULT_SESSION;

   if (device->physical->has_vm_control)
      return anv_i915_device_setup_vm(device);

   struct anv_physical_device *physical_device = device->physical;
   VkResult result = VK_SUCCESS;

   if (device->physical->engine_info) {
      /* The kernel API supports at most 64 engines */
      assert(num_queues <= 64);
      enum intel_engine_class engine_classes[64];
      int engine_count = 0;
      enum intel_gem_create_context_flags flags = 0;
      for (uint32_t i = 0; i < pCreateInfo->queueCreateInfoCount; i++) {
         const VkDeviceQueueCreateInfo *queueCreateInfo =
            &pCreateInfo->pQueueCreateInfos[i];

         assert(queueCreateInfo->queueFamilyIndex <
                physical_device->queue.family_count);
         struct anv_queue_family *queue_family =
            &physical_device->queue.families[queueCreateInfo->queueFamilyIndex];

         for (uint32_t j = 0; j < queueCreateInfo->queueCount; j++)
            engine_classes[engine_count++] = queue_family->engine_class;

         if (pCreateInfo->pQueueCreateInfos[i].flags &
             VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT)
            flags |= INTEL_GEM_CREATE_CONTEXT_EXT_PROTECTED_FLAG;
      }
      if (!intel_gem_create_context_engines(device->fd, flags,
                                            physical_device->engine_info,
                                            engine_count, engine_classes,
                                            device->vm_id,
                                            (uint32_t *)&device->context_id))
         result = vk_errorf(device, VK_ERROR_INITIALIZATION_FAILED,
                            "kernel context creation failed");
   } else {
      assert(num_queues == 1);
      if (!intel_gem_create_context(device->fd, &device->context_id))
         result = vk_error(device, VK_ERROR_INITIALIZATION_FAILED);
   }

   if (result != VK_SUCCESS)
      return result;

   /* Check if client specified queue priority. */
   const VkDeviceQueueGlobalPriorityCreateInfoKHR *queue_priority =
      vk_find_struct_const(pCreateInfo->pQueueCreateInfos[0].pNext,
                           DEVICE_QUEUE_GLOBAL_PRIORITY_CREATE_INFO_KHR);

   result = anv_i915_set_queue_parameters(device, device->context_id,
                                          queue_priority);
   if (result != VK_SUCCESS)
      goto fail_context;

   return result;

fail_context:
   intel_gem_destroy_context(device->fd, device->context_id);
   return result;
}

static VkResult
anv_gem_context_get_reset_stats(struct anv_device *device, int context)
{
   struct drm_i915_reset_stats stats = {
      .ctx_id = context,
   };

   int ret = intel_ioctl(device->fd, DRM_IOCTL_I915_GET_RESET_STATS, &stats);
   if (ret == -1) {
      /* We don't know the real error. */
      return vk_device_set_lost(&device->vk, "get_reset_stats failed: %m");
   }

   if (stats.batch_active) {
      return vk_device_set_lost(&device->vk, "GPU hung on one of our command buffers");
   } else if (stats.batch_pending) {
      return vk_device_set_lost(&device->vk, "GPU hung with commands in-flight");
   }

   return VK_SUCCESS;
}

VkResult
anv_i915_device_check_status(struct vk_device *vk_device)
{
   struct anv_device *device = container_of(vk_device, struct anv_device, vk);
   VkResult result;

   if (device->physical->has_vm_control) {
      for (uint32_t i = 0; i < device->queue_count; i++) {
         result = anv_gem_context_get_reset_stats(device,
                                                  device->queues[i].context_id);
         if (result != VK_SUCCESS)
            return result;

         if (device->queues[i].companion_rcs_id != 0) {
            uint32_t context_id = device->queues[i].companion_rcs_id;
            result = anv_gem_context_get_reset_stats(device, context_id);
            if (result != VK_SUCCESS) {
               return result;
            }
         }
      }
   } else {
      result = anv_gem_context_get_reset_stats(device, device->context_id);
   }

   return result;
}

bool
anv_i915_device_destroy_vm(struct anv_device *device)
{
   struct drm_i915_gem_vm_control destroy = {
      .vm_id = device->vm_id,
   };

   return intel_ioctl(device->fd, DRM_IOCTL_I915_GEM_VM_DESTROY, &destroy) == 0;
}

VkResult
anv_i915_device_setup_vm(struct anv_device *device)
{
   struct drm_i915_gem_vm_control create = {};
   if (intel_ioctl(device->fd, DRM_IOCTL_I915_GEM_VM_CREATE, &create))
      return vk_errorf(device, VK_ERROR_INITIALIZATION_FAILED,
                       "vm creation failed");

   device->vm_id = create.vm_id;
   return VK_SUCCESS;
}
