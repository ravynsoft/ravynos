/*
 * Copyright © 2022 Collabora Ltd. and Red Hat Inc.
 * SPDX-License-Identifier: MIT
 */
#ifndef NVK_INSTANCE_H
#define NVK_INSTANCE_H 1

#include "nvk_private.h"

#include "vulkan/runtime/vk_instance.h"
#include "util/xmlconfig.h"

struct nvk_instance {
   struct vk_instance vk;

   struct driOptionCache dri_options;
   struct driOptionCache available_dri_options;

   uint8_t driver_build_sha[20];
};

VK_DEFINE_HANDLE_CASTS(nvk_instance, vk.base, VkInstance, VK_OBJECT_TYPE_INSTANCE)

#endif
