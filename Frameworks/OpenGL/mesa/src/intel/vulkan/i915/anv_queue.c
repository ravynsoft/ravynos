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

#include "i915/anv_queue.h"

#include "anv_private.h"

#include "common/i915/intel_engine.h"
#include "common/intel_gem.h"

#include "i915/anv_device.h"

#include "drm-uapi/i915_drm.h"

VkResult
anv_i915_create_engine(struct anv_device *device,
                       struct anv_queue *queue,
                       const VkDeviceQueueCreateInfo *pCreateInfo)
{
   struct anv_physical_device *physical = device->physical;
   struct anv_queue_family *queue_family =
      &physical->queue.families[pCreateInfo->queueFamilyIndex];

   if (device->physical->engine_info == NULL) {
      switch (queue_family->engine_class) {
      case INTEL_ENGINE_CLASS_COPY:
         queue->exec_flags = I915_EXEC_BLT;
         break;
      case INTEL_ENGINE_CLASS_RENDER:
         queue->exec_flags = I915_EXEC_RENDER;
         break;
      case INTEL_ENGINE_CLASS_VIDEO:
         /* We want VCS0 (with ring1) for HW lacking HEVC on VCS1. */
         queue->exec_flags = I915_EXEC_BSD | I915_EXEC_BSD_RING1;
         break;
      default:
         unreachable("Unsupported legacy engine");
      }
   } else if (device->physical->has_vm_control) {
      assert(pCreateInfo->queueFamilyIndex < physical->queue.family_count);
      enum intel_engine_class engine_classes[1];
      engine_classes[0] = queue_family->engine_class;
      if (!intel_gem_create_context_engines(device->fd, 0 /* flags */,
                                            physical->engine_info,
                                            1, engine_classes,
                                            device->vm_id,
                                            (uint32_t *)&queue->context_id))
         return vk_errorf(device, VK_ERROR_INITIALIZATION_FAILED,
                          "engine creation failed");

      /* Create a companion RCS logical engine to support MSAA copy/clear
       * operation on compute/copy engine.
       */
      if (queue_family->engine_class == INTEL_ENGINE_CLASS_COPY ||
          queue_family->engine_class == INTEL_ENGINE_CLASS_COMPUTE) {
         uint32_t *context_id = (uint32_t *)&queue->companion_rcs_id;
         engine_classes[0] = INTEL_ENGINE_CLASS_RENDER;
         if (!intel_gem_create_context_engines(device->fd, 0 /* flags */,
                                               physical->engine_info,
                                               1, engine_classes,
                                               device->vm_id,
                                               context_id))
            return vk_errorf(device, VK_ERROR_INITIALIZATION_FAILED,
                             "companion RCS engine creation failed");
      }

      /* Check if client specified queue priority. */
      const VkDeviceQueueGlobalPriorityCreateInfoKHR *queue_priority =
         vk_find_struct_const(pCreateInfo->pNext,
                              DEVICE_QUEUE_GLOBAL_PRIORITY_CREATE_INFO_KHR);

      VkResult result = anv_i915_set_queue_parameters(device,
                                                      queue->context_id,
                                                      queue_priority);
      if (result != VK_SUCCESS) {
         intel_gem_destroy_context(device->fd, queue->context_id);
         if (queue->companion_rcs_id != 0) {
            intel_gem_destroy_context(device->fd, queue->companion_rcs_id);
         }
         return result;
      }
   } else {
      /* When using the new engine creation uAPI, the exec_flags value is the
       * index of the engine in the group specified at GEM context creation.
       */
      queue->exec_flags = device->queue_count;
   }

   return VK_SUCCESS;
}

void
anv_i915_destroy_engine(struct anv_device *device, struct anv_queue *queue)
{
   if (device->physical->has_vm_control) {
      intel_gem_destroy_context(device->fd, queue->context_id);

      if (queue->companion_rcs_id != 0) {
         intel_gem_destroy_context(device->fd, queue->companion_rcs_id);
      }
   }
}
