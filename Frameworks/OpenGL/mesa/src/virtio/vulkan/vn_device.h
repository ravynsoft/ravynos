/*
 * Copyright 2019 Google LLC
 * SPDX-License-Identifier: MIT
 *
 * based in part on anv and radv which are:
 * Copyright © 2015 Intel Corporation
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 */

#ifndef VN_DEVICE_H
#define VN_DEVICE_H

#include "vn_common.h"

#include "vn_buffer.h"
#include "vn_device_memory.h"
#include "vn_feedback.h"
#include "vn_image.h"

struct vn_device_memory_report {
   PFN_vkDeviceMemoryReportCallbackEXT callback;
   void *data;
};

struct vn_device {
   struct vn_device_base base;

   struct vn_instance *instance;
   struct vn_physical_device *physical_device;
   struct vn_renderer *renderer;
   struct vn_ring *primary_ring;

   struct vn_device_memory_report *memory_reports;
   uint32_t memory_report_count;

   /* unique queue family indices in which to create the device queues */
   uint32_t *queue_families;
   uint32_t queue_family_count;

   struct vn_device_memory_pool memory_pools[VK_MAX_MEMORY_TYPES];

   struct vn_feedback_pool feedback_pool;

   /* feedback cmd pool per queue family used by the device
    * - length matches queue_family_count
    * - order matches queue_families
    */
   struct vn_feedback_cmd_pool *cmd_pools;

   struct vn_queue *queues;
   uint32_t queue_count;

   struct vn_buffer_reqs_cache buffer_reqs_cache;
   struct vn_image_reqs_cache image_reqs_cache;
};
VK_DEFINE_HANDLE_CASTS(vn_device,
                       base.base.base,
                       VkDevice,
                       VK_OBJECT_TYPE_DEVICE)

static inline void
vn_device_emit_device_memory_report(struct vn_device *dev,
                                    VkDeviceMemoryReportEventTypeEXT type,
                                    uint64_t mem_obj_id,
                                    VkDeviceSize size,
                                    VkObjectType obj_type,
                                    uint64_t obj_handle,
                                    uint32_t heap_index)
{
   assert(dev->memory_reports);
   const VkDeviceMemoryReportCallbackDataEXT report = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_MEMORY_REPORT_CALLBACK_DATA_EXT,
      .type = type,
      .memoryObjectId = mem_obj_id,
      .size = size,
      .objectType = obj_type,
      .objectHandle = obj_handle,
      .heapIndex = heap_index,
   };
   for (uint32_t i = 0; i < dev->memory_report_count; i++)
      dev->memory_reports[i].callback(&report, dev->memory_reports[i].data);
}

#endif /* VN_DEVICE_H */
