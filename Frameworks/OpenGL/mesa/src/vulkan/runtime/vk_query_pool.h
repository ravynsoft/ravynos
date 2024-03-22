/*
 * Copyright © 2022 Collabora, Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */
#ifndef VK_QUERY_POOL_H
#define VK_QUERY_POOL_H

#include "vk_object.h"

#ifdef __cplusplus
extern "C" {
#endif

struct vk_query_pool {
   struct vk_object_base base;

   /** VkQueryPoolCreateInfo::queryType */
   VkQueryType query_type;

   /** VkQueryPoolCreateInfo::queryCount */
   uint32_t  query_count;

   /** VkQueryPoolCreateInfo::pipelineStatistics
    *
    * If query_type != VK_QUERY_TYPE_PIPELINE_STATISTICS, this will be zero.
    */
   VkQueryPipelineStatisticFlags pipeline_statistics;
};

void vk_query_pool_init(struct vk_device *device,
                        struct vk_query_pool *query_pool,
                        const VkQueryPoolCreateInfo *pCreateInfo);
void vk_query_pool_finish(struct vk_query_pool *query_pool);
void *vk_query_pool_create(struct vk_device *device,
                           const VkQueryPoolCreateInfo *pCreateInfo,
                           const VkAllocationCallbacks *alloc,
                           size_t size);
void vk_query_pool_destroy(struct vk_device *device,
                           const VkAllocationCallbacks *alloc,
                           struct vk_query_pool *query_pool);

#ifdef __cplusplus
}
#endif

#endif /* VK_QUERY_POOL_H */
