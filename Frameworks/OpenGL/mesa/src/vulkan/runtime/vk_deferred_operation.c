/*
 * Copyright © 2020 Intel Corporation
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

#include "vk_deferred_operation.h"

#include "vk_alloc.h"
#include "vk_common_entrypoints.h"
#include "vk_device.h"

VKAPI_ATTR VkResult VKAPI_CALL
vk_common_CreateDeferredOperationKHR(VkDevice _device,
                                     const VkAllocationCallbacks *pAllocator,
                                     VkDeferredOperationKHR *pDeferredOperation)
{
   VK_FROM_HANDLE(vk_device, device, _device);

   struct vk_deferred_operation *op =
      vk_alloc2(&device->alloc, pAllocator, sizeof(*op), 8,
                       VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (op == NULL)
      return VK_ERROR_OUT_OF_HOST_MEMORY;

   vk_object_base_init(device, &op->base,
                       VK_OBJECT_TYPE_DEFERRED_OPERATION_KHR);

   *pDeferredOperation = vk_deferred_operation_to_handle(op);

   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL
vk_common_DestroyDeferredOperationKHR(VkDevice _device,
                                      VkDeferredOperationKHR operation,
                                      const VkAllocationCallbacks *pAllocator)
{
   VK_FROM_HANDLE(vk_device, device, _device);
   VK_FROM_HANDLE(vk_deferred_operation, op, operation);

   if (op == NULL)
      return;

   vk_object_base_finish(&op->base);
   vk_free2(&device->alloc, pAllocator, op);
}

VKAPI_ATTR uint32_t VKAPI_CALL
vk_common_GetDeferredOperationMaxConcurrencyKHR(UNUSED VkDevice device,
                                                UNUSED VkDeferredOperationKHR operation)
{
   return 1;
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_common_GetDeferredOperationResultKHR(UNUSED VkDevice device,
                                        UNUSED VkDeferredOperationKHR operation)
{
   return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_common_DeferredOperationJoinKHR(UNUSED VkDevice device,
                                   UNUSED VkDeferredOperationKHR operation)
{
   return VK_SUCCESS;
}
