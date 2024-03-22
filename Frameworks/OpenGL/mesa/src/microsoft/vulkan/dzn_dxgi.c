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

#define COBJMACROS
#include "dzn_physical_device_enum.h"
#include "dzn_dxgi.h"

#include "util/log.h"

VkResult
dzn_enumerate_physical_devices_dxgi(struct vk_instance *instance)
{
   IDXGIFactory4 *factory = dxgi_get_factory(false);
   IDXGIAdapter1 *adapter = NULL;
   VkResult result = VK_SUCCESS;
   for (UINT i = 0; SUCCEEDED(IDXGIFactory4_EnumAdapters1(factory, i, &adapter)); ++i) {
      DXGI_ADAPTER_DESC1 dxgi_desc;
      IDXGIAdapter1_GetDesc1(adapter, &dxgi_desc);

      struct dzn_physical_device_desc desc = {
         .adapter_luid = dxgi_desc.AdapterLuid,
         .vendor_id = dxgi_desc.VendorId,
         .device_id = dxgi_desc.DeviceId,
         .subsys_id = dxgi_desc.SubSysId,
         .revision = dxgi_desc.Revision,
         .shared_system_memory = dxgi_desc.SharedSystemMemory,
         .dedicated_system_memory = dxgi_desc.DedicatedSystemMemory,
         .dedicated_video_memory = dxgi_desc.DedicatedVideoMemory,
         .is_warp = (dxgi_desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0,
      };
      WideCharToMultiByte(CP_ACP, 0, dxgi_desc.Description, ARRAYSIZE(dxgi_desc.Description),
                          desc.description, ARRAYSIZE(desc.description), NULL, NULL);
      result =
         dzn_instance_add_physical_device(instance, (IUnknown *)adapter, &desc);

      IDXGIAdapter1_Release(adapter);

      if (result != VK_SUCCESS)
         break;
    }

   IDXGIFactory4_Release(factory);

   return result;
}

IDXGIFactory4 *
dxgi_get_factory(bool debug)
{
   static const GUID IID_IDXGIFactory4 = {
      0x1bc6ea02, 0xef36, 0x464f,
      { 0xbf, 0x0c, 0x21, 0xca, 0x39, 0xe5, 0x16, 0x8a }
   };

   HMODULE dxgi_mod = LoadLibraryA("DXGI.DLL");
   if (!dxgi_mod) {
      mesa_loge("failed to load DXGI.DLL\n");
      return NULL;
   }

   typedef HRESULT(WINAPI *PFN_CREATE_DXGI_FACTORY2)(UINT flags, REFIID riid, void **ppFactory);
   PFN_CREATE_DXGI_FACTORY2 CreateDXGIFactory2;

   CreateDXGIFactory2 = (PFN_CREATE_DXGI_FACTORY2)GetProcAddress(dxgi_mod, "CreateDXGIFactory2");
   if (!CreateDXGIFactory2) {
      mesa_loge("failed to load CreateDXGIFactory2 from DXGI.DLL\n");
      return NULL;
   }

   UINT flags = 0;
   if (debug)
      flags |= DXGI_CREATE_FACTORY_DEBUG;

   IDXGIFactory4 *factory;
   HRESULT hr = CreateDXGIFactory2(flags, &IID_IDXGIFactory4, (void **)&factory);
   if (FAILED(hr)) {
      mesa_loge("CreateDXGIFactory2 failed: %08x\n", (int32_t)hr);
      return NULL;
   }

   return factory;
}
