/*
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 * SPDX-License-Identifier: MIT
 *
 * based in part on anv driver which is:
 * Copyright © 2015 Intel Corporation
 */

#ifndef TU_DYNAMIC_RENDERING_H
#define TU_DYNAMIC_RENDERING_H

#include "tu_common.h"

VkResult tu_init_dynamic_rendering(struct tu_device *dev);

void tu_destroy_dynamic_rendering(struct tu_device *dev);

VkResult tu_insert_dynamic_cmdbufs(struct tu_device *dev,
                                   struct tu_cmd_buffer ***cmds_ptr,
                                   uint32_t *size);

#endif /* TU_DYNAMIC_RENDERING_H */
