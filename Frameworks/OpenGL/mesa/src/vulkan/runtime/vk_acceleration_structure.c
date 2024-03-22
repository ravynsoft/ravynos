/*
 * Copyright © 2021 Bas Nieuwenhuizen
 * Copyright © 2023 Valve Corporation
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

#include "vk_acceleration_structure.h"

#include "vk_alloc.h"
#include "vk_common_entrypoints.h"
#include "vk_device.h"
#include "vk_log.h"

VkDeviceAddress
vk_acceleration_structure_get_va(struct vk_acceleration_structure *accel_struct)
{
   VkBufferDeviceAddressInfo info = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
      .buffer = accel_struct->buffer,
   };

   VkDeviceAddress base_addr = accel_struct->base.device->dispatch_table.GetBufferDeviceAddress(
      vk_device_to_handle(accel_struct->base.device), &info);

   return base_addr + accel_struct->offset;
}


VKAPI_ATTR VkResult VKAPI_CALL
vk_common_CreateAccelerationStructureKHR(VkDevice _device,
                                         const VkAccelerationStructureCreateInfoKHR *pCreateInfo,
                                         const VkAllocationCallbacks *pAllocator,
                                         VkAccelerationStructureKHR *pAccelerationStructure)
{
   VK_FROM_HANDLE(vk_device, device, _device);

   struct vk_acceleration_structure *accel_struct = vk_object_alloc(
      device, pAllocator, sizeof(struct vk_acceleration_structure),
      VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR);

   if (!accel_struct)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   accel_struct->buffer = pCreateInfo->buffer;
   accel_struct->offset = pCreateInfo->offset;
   accel_struct->size = pCreateInfo->size;

   if (pCreateInfo->deviceAddress &&
       vk_acceleration_structure_get_va(accel_struct) != pCreateInfo->deviceAddress)
      return vk_error(device, VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS);

   *pAccelerationStructure = vk_acceleration_structure_to_handle(accel_struct);
   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL
vk_common_DestroyAccelerationStructureKHR(VkDevice _device,
                                     VkAccelerationStructureKHR accelerationStructure,
                                     const VkAllocationCallbacks *pAllocator)
{
   VK_FROM_HANDLE(vk_device, device, _device);
   VK_FROM_HANDLE(vk_acceleration_structure, accel_struct, accelerationStructure);

   if (!accel_struct)
      return;

   vk_object_free(device, pAllocator, accel_struct);
}

VKAPI_ATTR VkDeviceAddress VKAPI_CALL
vk_common_GetAccelerationStructureDeviceAddressKHR(
   VkDevice _device, const VkAccelerationStructureDeviceAddressInfoKHR *pInfo)
{
   VK_FROM_HANDLE(vk_acceleration_structure, accel_struct, pInfo->accelerationStructure);
   return vk_acceleration_structure_get_va(accel_struct);
}
