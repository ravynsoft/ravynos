/*
 * Copyright 2019 Google LLC
 * SPDX-License-Identifier: MIT
 *
 * based in part on anv and radv which are:
 * Copyright © 2015 Intel Corporation
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 */

#include "vn_icd.h"

#include "vn_instance.h"

PFN_vkVoidFunction
vk_icdGetInstanceProcAddr(VkInstance instance, const char *pName)
{
   return vn_GetInstanceProcAddr(instance, pName);
}

bool
vn_icd_supports_api_version(uint32_t api_version)
{
   return vk_get_negotiated_icd_version() >= 5 ||
          api_version < VK_API_VERSION_1_1;
}
