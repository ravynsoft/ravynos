/*
 * Copyright © 2021 Collabora Ltd.
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "panvk_private.h"

VkResult
panvk_CreateQueryPool(VkDevice _device,
                      const VkQueryPoolCreateInfo *pCreateInfo,
                      const VkAllocationCallbacks *pAllocator,
                      VkQueryPool *pQueryPool)
{
   panvk_stub();
   return VK_SUCCESS;
}

void
panvk_DestroyQueryPool(VkDevice _device, VkQueryPool _pool,
                       const VkAllocationCallbacks *pAllocator)
{
   panvk_stub();
}

VkResult
panvk_GetQueryPoolResults(VkDevice _device, VkQueryPool queryPool,
                          uint32_t firstQuery, uint32_t queryCount,
                          size_t dataSize, void *pData, VkDeviceSize stride,
                          VkQueryResultFlags flags)
{
   panvk_stub();
   return VK_SUCCESS;
}

void
panvk_CmdCopyQueryPoolResults(VkCommandBuffer commandBuffer,
                              VkQueryPool queryPool, uint32_t firstQuery,
                              uint32_t queryCount, VkBuffer dstBuffer,
                              VkDeviceSize dstOffset, VkDeviceSize stride,
                              VkQueryResultFlags flags)
{
   panvk_stub();
}

void
panvk_CmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool,
                        uint32_t firstQuery, uint32_t queryCount)
{
   panvk_stub();
}

void
panvk_CmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool,
                    uint32_t query, VkQueryControlFlags flags)
{
   panvk_stub();
}

void
panvk_CmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool,
                  uint32_t query)
{
   panvk_stub();
}

void
panvk_CmdWriteTimestamp2(VkCommandBuffer commandBuffer,
                         VkPipelineStageFlags2 stage, VkQueryPool queryPool,
                         uint32_t query)
{
   panvk_stub();
}
