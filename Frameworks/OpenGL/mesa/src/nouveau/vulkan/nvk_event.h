/*
 * Copyright © 2022 Collabora Ltd. and Red Hat Inc.
 * SPDX-License-Identifier: MIT
 */
#ifndef NVK_EVENT_H
#define NVK_EVENT_H 1

#include "nvk_private.h"

#include "vulkan/runtime/vk_object.h"

struct nvk_event {
   struct vk_object_base base;

   uint64_t addr;
   VkResult *status;
};

VK_DEFINE_NONDISP_HANDLE_CASTS(nvk_event, base, VkEvent, VK_OBJECT_TYPE_EVENT)

uint32_t
vk_stage_flags_to_nv9097_pipeline_location(VkPipelineStageFlags2 flags);

#endif /* define NVK_EVENT_H */
