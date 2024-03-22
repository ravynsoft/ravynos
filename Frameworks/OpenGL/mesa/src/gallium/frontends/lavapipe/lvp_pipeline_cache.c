/*
 * Copyright © 2019 Red Hat.
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

#include "lvp_private.h"

VKAPI_ATTR VkResult VKAPI_CALL lvp_CreatePipelineCache(
    VkDevice                                    _device,
    const VkPipelineCacheCreateInfo*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPipelineCache*                            pPipelineCache)
{
   LVP_FROM_HANDLE(lvp_device, device, _device);
   struct lvp_pipeline_cache *cache;

   assert(pCreateInfo->sType == VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO);

   cache = vk_alloc2(&device->vk.alloc, pAllocator,
                       sizeof(*cache), 8,
                       VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (cache == NULL)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   vk_object_base_init(&device->vk, &cache->base,
                       VK_OBJECT_TYPE_PIPELINE_CACHE);
   if (pAllocator)
     cache->alloc = *pAllocator;
   else
     cache->alloc = device->vk.alloc;

   cache->device = device;
   *pPipelineCache = lvp_pipeline_cache_to_handle(cache);

   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL lvp_DestroyPipelineCache(
    VkDevice                                    _device,
    VkPipelineCache                             _cache,
    const VkAllocationCallbacks*                pAllocator)
{
   LVP_FROM_HANDLE(lvp_device, device, _device);
   LVP_FROM_HANDLE(lvp_pipeline_cache, cache, _cache);

   if (!_cache)
      return;
//   lvp_pipeline_cache_finish(cache);
   vk_object_base_finish(&cache->base);
   vk_free2(&device->vk.alloc, pAllocator, cache);
}

VKAPI_ATTR VkResult VKAPI_CALL lvp_GetPipelineCacheData(
        VkDevice                                    _device,
        VkPipelineCache                             _cache,
        size_t*                                     pDataSize,
        void*                                       pData)
{
   VkResult result = VK_SUCCESS;
   if (pData) {
      if (*pDataSize < 32) {
         *pDataSize = 0;
         result = VK_INCOMPLETE;
      } else {
         uint32_t *hdr = (uint32_t *)pData;
         hdr[0] = 32;
         hdr[1] = 1;
         hdr[2] = VK_VENDOR_ID_MESA;
         hdr[3] = 0;
         lvp_device_get_cache_uuid(&hdr[4]);
      }
   } else
      *pDataSize = 32;
   return result;
}

VKAPI_ATTR VkResult VKAPI_CALL lvp_MergePipelineCaches(
        VkDevice                                    _device,
        VkPipelineCache                             destCache,
        uint32_t                                    srcCacheCount,
        const VkPipelineCache*                      pSrcCaches)
{
   return VK_SUCCESS;
}
