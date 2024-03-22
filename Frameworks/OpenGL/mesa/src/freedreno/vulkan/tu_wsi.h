/*
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 * SPDX-License-Identifier: MIT
 *
 * based in part on anv driver which is:
 * Copyright © 2015 Intel Corporation
 */

#ifndef TU_WSI_H
#define TU_WSI_H

#include "tu_common.h"

VkResult
tu_wsi_init(struct tu_physical_device *physical_device);

void
tu_wsi_finish(struct tu_physical_device *physical_device);

#endif /* TU_WSI_H */
