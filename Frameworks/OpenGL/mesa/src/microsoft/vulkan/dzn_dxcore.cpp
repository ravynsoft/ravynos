/*
 * Copyright © Microsoft Corporation
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "dzn_physical_device_enum.h"
#include <directx/dxcore.h>
#include <dxguids/dxguids.h>

#include "util/u_dl.h"
#include "util/log.h"

VkResult
dzn_enumerate_physical_devices_dxcore(struct vk_instance *instance)
{
   util_dl_library *dxcore = util_dl_open(UTIL_DL_PREFIX "dxcore" UTIL_DL_EXT);
   if (!dxcore) {
      mesa_loge("Failed to load DXCore\n");
      return VK_ERROR_INITIALIZATION_FAILED;
   }

   using PFNDXCoreCreateAdapterFactory = HRESULT (APIENTRY*)(REFIID, void **);
   PFNDXCoreCreateAdapterFactory create_func = (PFNDXCoreCreateAdapterFactory)util_dl_get_proc_address(dxcore, "DXCoreCreateAdapterFactory");
   if (!create_func) {
      mesa_loge("Failed to load DXCoreCreateAdapterFactory\n");
      return VK_ERROR_INITIALIZATION_FAILED;
   }

   IDXCoreAdapterFactory *factory;
   if (FAILED(create_func(IID_PPV_ARGS(&factory)))) {
      mesa_loge("Failed to create DXCore adapter factory\n");
      return VK_ERROR_INITIALIZATION_FAILED;
   }

   IDXCoreAdapterList *list;
   if (FAILED(factory->CreateAdapterList(1, &DXCORE_ADAPTER_ATTRIBUTE_D3D12_GRAPHICS, IID_PPV_ARGS(&list)))) {
      factory->Release();
      mesa_loge("Failed to create DXCore adapter list\n");
      return VK_ERROR_INITIALIZATION_FAILED;
   }

   VkResult result = VK_SUCCESS;
   uint32_t adapter_count = list->GetAdapterCount();
   IDXCoreAdapter *adapter;
   for (uint32_t i = 0; i < adapter_count && result == VK_SUCCESS; ++i) {
      result = VK_ERROR_INITIALIZATION_FAILED;
      if (SUCCEEDED(list->GetAdapter(i, IID_PPV_ARGS(&adapter)))) {
         dzn_physical_device_desc desc = { 0 };
         DXCoreHardwareID hardware_id;
         bool is_hardware;
         if (FAILED(adapter->GetProperty(DXCoreAdapterProperty::HardwareID, &hardware_id)) ||
             FAILED(adapter->GetProperty(DXCoreAdapterProperty::DedicatedAdapterMemory, &desc.dedicated_video_memory)) ||
             FAILED(adapter->GetProperty(DXCoreAdapterProperty::SharedSystemMemory, &desc.shared_system_memory)) ||
             FAILED(adapter->GetProperty(DXCoreAdapterProperty::DedicatedSystemMemory, &desc.dedicated_system_memory)) ||
             FAILED(adapter->GetProperty(DXCoreAdapterProperty::InstanceLuid, &desc.adapter_luid)) ||
             FAILED(adapter->GetProperty(DXCoreAdapterProperty::IsHardware, &is_hardware)) ||
             FAILED(adapter->GetProperty(DXCoreAdapterProperty::DriverDescription, sizeof(desc.description), desc.description))) {
            mesa_loge("Failed to retrieve DXCore adapter properties\n");
            result = VK_ERROR_INITIALIZATION_FAILED;
         } else {
            desc.vendor_id = hardware_id.vendorID;
            desc.device_id = hardware_id.deviceID;
            desc.subsys_id = hardware_id.subSysID;
            desc.revision = hardware_id.revision;
            desc.is_warp = !is_hardware;
            result = dzn_instance_add_physical_device(instance, adapter, &desc);
         }

         adapter->Release();
      }
   }

   list->Release();
   factory->Release();
   return result;
}

