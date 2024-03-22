/*
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 * SPDX-License-Identifier: MIT
 *
 * based in part on anv driver which is:
 * Copyright © 2015 Intel Corporation
 */

#ifndef TU_ANDROID_H
#define TU_ANDROID_H

#include "tu_common.h"

VkResult
tu_gralloc_info(struct tu_device *device,
                const VkNativeBufferANDROID *gralloc_info,
                int *dma_buf,
                uint64_t *modifier);

VkResult
tu_import_memory_from_gralloc_handle(VkDevice device_h,
                                     int dma_buf,
                                     const VkAllocationCallbacks *alloc,
                                     VkImage image_h);

#endif /* TU_ANDROID_H */
