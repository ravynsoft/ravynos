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

#include "xe/anv_queue.h"

#include "anv_private.h"

#include "common/xe/intel_engine.h"
#include "common/intel_gem.h"

#include "xe/anv_device.h"

#include "drm-uapi/xe_drm.h"
#include "drm-uapi/gpu_scheduler.h"

static enum drm_sched_priority
anv_vk_priority_to_drm_sched_priority(VkQueueGlobalPriorityKHR vk_priority)
{
   switch (vk_priority) {
   case VK_QUEUE_GLOBAL_PRIORITY_LOW_KHR:
      return DRM_SCHED_PRIORITY_MIN;
   case VK_QUEUE_GLOBAL_PRIORITY_MEDIUM_KHR:
      return DRM_SCHED_PRIORITY_NORMAL;
   case VK_QUEUE_GLOBAL_PRIORITY_HIGH_KHR:
      return DRM_SCHED_PRIORITY_HIGH;
   default:
      unreachable("Invalid priority");
      return DRM_SCHED_PRIORITY_MIN;
   }
}

static VkResult
create_engine(struct anv_device *device,
              struct anv_queue *queue,
              const VkDeviceQueueCreateInfo *pCreateInfo,
              bool create_companion_rcs_engine)
{
   struct anv_physical_device *physical = device->physical;
   uint32_t queue_family_index =
      create_companion_rcs_engine ?
      anv_get_first_render_queue_index(physical) :
      pCreateInfo->queueFamilyIndex;
   struct anv_queue_family *queue_family =
      &physical->queue.families[queue_family_index];
   const struct intel_query_engine_info *engines = physical->engine_info;
   struct drm_xe_engine_class_instance *instances;
   const VkDeviceQueueGlobalPriorityCreateInfoKHR *queue_priority =
      vk_find_struct_const(pCreateInfo->pNext,
                           DEVICE_QUEUE_GLOBAL_PRIORITY_CREATE_INFO_KHR);
   const VkQueueGlobalPriorityKHR priority = queue_priority ?
                                             queue_priority->globalPriority :
                                             VK_QUEUE_GLOBAL_PRIORITY_MEDIUM_KHR;

   /* As per spec, the driver implementation may deny requests to acquire
    * a priority above the default priority (MEDIUM) if the caller does not
    * have sufficient privileges. In this scenario VK_ERROR_NOT_PERMITTED_KHR
    * is returned.
    */
   if (physical->max_context_priority >= VK_QUEUE_GLOBAL_PRIORITY_MEDIUM_KHR) {
      if (priority > physical->max_context_priority)
         return vk_error(device, VK_ERROR_NOT_PERMITTED_KHR);
   }

   instances = vk_alloc(&device->vk.alloc,
                        sizeof(*instances) * queue_family->queueCount, 8,
                        VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
   if (!instances)
      return VK_ERROR_OUT_OF_HOST_MEMORY;

   /* Build a list of all compatible HW engines */
   uint32_t count = 0;
   for (uint32_t i = 0; i < engines->num_engines; i++) {
      const struct intel_engine_class_instance engine = engines->engines[i];
      if (engine.engine_class != queue_family->engine_class)
         continue;

      instances[count].engine_class = intel_engine_class_to_xe(engine.engine_class);
      instances[count].engine_instance = engine.engine_instance;
      instances[count++].gt_id = engine.gt_id;
   }

   assert(device->vm_id != 0);
   struct drm_xe_ext_set_property ext = {
      .base.name = DRM_XE_EXEC_QUEUE_EXTENSION_SET_PROPERTY,
      .property = DRM_XE_EXEC_QUEUE_SET_PROPERTY_PRIORITY,
      .value = anv_vk_priority_to_drm_sched_priority(priority),
   };
   struct drm_xe_exec_queue_create create = {
         /* Allows KMD to pick one of those engines for the submission queue */
         .instances = (uintptr_t)instances,
         .vm_id = device->vm_id,
         .width = 1,
         .num_placements = count,
         .extensions = (uintptr_t)&ext,
   };
   int ret = intel_ioctl(device->fd, DRM_IOCTL_XE_EXEC_QUEUE_CREATE, &create);
   vk_free(&device->vk.alloc, instances);
   if (ret)
      return vk_errorf(device, VK_ERROR_UNKNOWN, "Unable to create exec queue");

   if (create_companion_rcs_engine)
      queue->companion_rcs_id = create.exec_queue_id;
   else
      queue->exec_queue_id = create.exec_queue_id;

   return VK_SUCCESS;
}

VkResult
anv_xe_create_engine(struct anv_device *device,
                     struct anv_queue *queue,
                     const VkDeviceQueueCreateInfo *pCreateInfo)
{
   VkResult result = create_engine(device, queue, pCreateInfo,
                                   false /* create_companion_rcs_engine */);

   if (result != VK_SUCCESS)
      return result;

   if (queue->family->engine_class == INTEL_ENGINE_CLASS_COPY ||
       queue->family->engine_class == INTEL_ENGINE_CLASS_COMPUTE) {
      result = create_engine(device, queue, pCreateInfo,
                             true /* create_companion_rcs_engine */);
   }

   return result;
}

static void
destroy_engine(struct anv_device *device, uint32_t exec_queue_id)
{
   struct drm_xe_exec_queue_destroy destroy = {
      .exec_queue_id = exec_queue_id,
   };
   intel_ioctl(device->fd, DRM_IOCTL_XE_EXEC_QUEUE_DESTROY, &destroy);
}

void
anv_xe_destroy_engine(struct anv_device *device, struct anv_queue *queue)
{
   destroy_engine(device, queue->exec_queue_id);

   if (queue->companion_rcs_id != 0)
      destroy_engine(device, queue->companion_rcs_id);
}
