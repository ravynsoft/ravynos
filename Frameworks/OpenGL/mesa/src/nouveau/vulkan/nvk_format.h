/*
 * Copyright © 2022 Collabora Ltd. and Red Hat Inc.
 * SPDX-License-Identifier: MIT
 */
#ifndef NVK_FORMAT_H
#define NVK_FORMAT_H 1

#include "nvk_private.h"

struct nvk_physical_device;

struct nvk_va_format {
   uint8_t bit_widths;
   uint8_t swap_rb:1;
   uint8_t type:7;
};

const struct nvk_va_format *
nvk_get_va_format(const struct nvk_physical_device *pdev, VkFormat format);

#endif
