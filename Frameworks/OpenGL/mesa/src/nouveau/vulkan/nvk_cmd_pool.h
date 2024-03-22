/*
 * Copyright © 2022 Collabora Ltd. and Red Hat Inc.
 * SPDX-License-Identifier: MIT
 */
#ifndef NVK_CMD_POOL_H
#define NVK_CMD_POOL_H

#include "nvk_private.h"

#include "vk_command_pool.h"

#define NVK_CMD_BO_SIZE 64*1024

/* Recyclable command buffer BO, used for both push buffers and upload */
struct nvk_cmd_bo {
   struct nouveau_ws_bo *bo;

   void *map;

   /** Link in nvk_cmd_pool::free_bos or nvk_cmd_buffer::bos */
   struct list_head link;
};

struct nvk_cmd_pool {
   struct vk_command_pool vk;

   /** List of nvk_cmd_bo */
   struct list_head free_bos;
   struct list_head free_gart_bos;
};

VK_DEFINE_NONDISP_HANDLE_CASTS(nvk_cmd_pool, vk.base, VkCommandPool,
                               VK_OBJECT_TYPE_COMMAND_POOL)

static inline struct nvk_device *
nvk_cmd_pool_device(struct nvk_cmd_pool *pool)
{
   return (struct nvk_device *)pool->vk.base.device;
}

VkResult nvk_cmd_pool_alloc_bo(struct nvk_cmd_pool *pool,
                               bool force_gart,
                               struct nvk_cmd_bo **bo_out);

void nvk_cmd_pool_free_bo_list(struct nvk_cmd_pool *pool,
                               struct list_head *bos);
void nvk_cmd_pool_free_gart_bo_list(struct nvk_cmd_pool *pool,
                                    struct list_head *bos);
#endif /* NVK_CMD_POOL_H */
