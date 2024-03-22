/*
 * Copyright © 2022 Collabora Ltd. and Red Hat Inc.
 * SPDX-License-Identifier: MIT
 */
#ifndef NVK_QUERY_POOL_H
#define NVK_QUERY_POOL_H 1

#include "nvk_private.h"

#include "vk_query_pool.h"

struct nouveau_ws_bo;

struct nvk_query_pool {
   struct vk_query_pool vk;

   uint32_t query_start;
   uint32_t query_stride;

   struct nouveau_ws_bo *bo;
   void *bo_map;
};

VK_DEFINE_NONDISP_HANDLE_CASTS(nvk_query_pool, vk.base, VkQueryPool,
                               VK_OBJECT_TYPE_QUERY_POOL)

#endif /* NVK_QUERY_POOL_H */
