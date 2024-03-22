/*
 * Copyright © 2022 Collabora Ltd. and Red Hat Inc.
 * SPDX-License-Identifier: MIT
 */
#ifndef NVK_WSI_H
#define NVK_WSI_H 1

#include "nvk_physical_device.h"

VkResult nvk_init_wsi(struct nvk_physical_device *pdev);
void nvk_finish_wsi(struct nvk_physical_device *pdev);

#endif
