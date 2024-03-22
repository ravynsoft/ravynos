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

#include "d3d12_screen.h"
#include "d3d12_public.h"

#include "util/u_debug.h"
#include "util/u_memory.h"
#include "util/u_dl.h"

#include <directx/dxcore.h>
#include <dxguids/dxguids.h>

static IDXCoreAdapterFactory *
get_dxcore_factory()
{
   typedef HRESULT(WINAPI *PFN_CREATE_DXCORE_ADAPTER_FACTORY)(REFIID riid, void **ppFactory);
   PFN_CREATE_DXCORE_ADAPTER_FACTORY DXCoreCreateAdapterFactory;

   util_dl_library *dxcore_mod = util_dl_open(UTIL_DL_PREFIX "dxcore" UTIL_DL_EXT);
   if (!dxcore_mod) {
      debug_printf("D3D12: failed to load DXCore.DLL\n");
      return NULL;
   }

   DXCoreCreateAdapterFactory = (PFN_CREATE_DXCORE_ADAPTER_FACTORY)util_dl_get_proc_address(dxcore_mod, "DXCoreCreateAdapterFactory");
   if (!DXCoreCreateAdapterFactory) {
      debug_printf("D3D12: failed to load DXCoreCreateAdapterFactory from DXCore.DLL\n");
      return NULL;
   }

   IDXCoreAdapterFactory *factory = NULL;
   HRESULT hr = DXCoreCreateAdapterFactory(IID_IDXCoreAdapterFactory, (void **)&factory);
   if (FAILED(hr)) {
      debug_printf("D3D12: DXCoreCreateAdapterFactory failed: %08x\n", hr);
      return NULL;
   }

   return factory;
}

static IDXCoreAdapter *
choose_dxcore_adapter(IDXCoreAdapterFactory *factory, LUID *adapter_luid)
{
   IDXCoreAdapter *adapter = nullptr;
   if (adapter_luid) {
      if (SUCCEEDED(factory->GetAdapterByLuid(*adapter_luid, &adapter)))
         return adapter;
      debug_printf("D3D12: requested adapter missing, falling back to auto-detection...\n");
   }

   IDXCoreAdapterList *list = nullptr;
   if (SUCCEEDED(factory->CreateAdapterList(1, &DXCORE_ADAPTER_ATTRIBUTE_D3D12_GRAPHICS, &list))) {

#ifndef _WIN32
      // Pick the user selected adapter if any
      char *adapter_name = getenv("MESA_D3D12_DEFAULT_ADAPTER_NAME");
      if (adapter_name) {
         for (unsigned i=0; i<list->GetAdapterCount(); i++) {
            if (SUCCEEDED(list->GetAdapter(i, &adapter))) {

               size_t desc_size;
               if (!SUCCEEDED(adapter->GetPropertySize(DXCoreAdapterProperty::DriverDescription, &desc_size))) {
                  adapter->Release();
                  continue;
               }

               char *desc = (char*)malloc(desc_size);
               if (!desc) {
                  adapter->Release();
                  continue;
               }

               if (!SUCCEEDED(adapter->GetProperty(DXCoreAdapterProperty::DriverDescription, desc_size, desc))) {
                  free(desc);
                  adapter->Release();
                  continue;
               }

               if (strcasestr(desc, adapter_name)) {
                  free(desc);
                  return adapter;
               } else {
                  free(desc);
                  adapter->Release();
               }
            }
         }
         debug_printf("D3D12: Couldn't find an adapter containing the substring (%s)\n", adapter_name);
      }
#endif

      // Adapter not specified or not found, so pick an integrated adapter if possible
      for (unsigned i = 0; i < list->GetAdapterCount(); ++i) {
         if (SUCCEEDED(list->GetAdapter(i, &adapter))) {
            bool is_integrated;
            if (SUCCEEDED(adapter->GetProperty(DXCoreAdapterProperty::IsIntegrated, &is_integrated)) && is_integrated)
               return adapter;
            adapter->Release();
         }
      }

      // No integrated GPUs, so pick the first valid one
      if (list->GetAdapterCount() > 0 && SUCCEEDED(list->GetAdapter(0, &adapter)))
         return adapter;
   }

   return NULL;
}

static const char *
dxcore_get_name(struct pipe_screen *screen)
{
   struct d3d12_dxcore_screen *dxcore_screen = d3d12_dxcore_screen(d3d12_screen(screen));
   static char buf[1000];
   if (dxcore_screen->description[0] == '\0')
      return "D3D12 (Unknown)";

   snprintf(buf, sizeof(buf), "D3D12 (%s)", dxcore_screen->description);
   return buf;
}

static void
dxcore_get_memory_info(struct d3d12_screen *screen, struct d3d12_memory_info *output)
{
   struct d3d12_dxcore_screen *dxcore_screen = d3d12_dxcore_screen(screen);
   DXCoreAdapterMemoryBudget local_info, nonlocal_info;
   DXCoreAdapterMemoryBudgetNodeSegmentGroup local_node_segment = { 0, DXCoreSegmentGroup::Local };
   DXCoreAdapterMemoryBudgetNodeSegmentGroup nonlocal_node_segment = { 0, DXCoreSegmentGroup::NonLocal };
   dxcore_screen->adapter->QueryState(DXCoreAdapterState::AdapterMemoryBudget, &local_node_segment, &local_info);
   dxcore_screen->adapter->QueryState(DXCoreAdapterState::AdapterMemoryBudget, &nonlocal_node_segment, &nonlocal_info);
   output->budget = local_info.budget + nonlocal_info.budget;
   output->usage = local_info.currentUsage + nonlocal_info.currentUsage;
}

static void
d3d12_deinit_dxcore_screen(struct d3d12_screen *dscreen)
{
   d3d12_deinit_screen(dscreen);
   struct d3d12_dxcore_screen *screen = d3d12_dxcore_screen(dscreen);
   if (screen->adapter) {
      screen->adapter->Release();
      screen->adapter = nullptr;
   }
   if (screen->factory) {
      screen->factory->Release();
      screen->factory = nullptr;
   }
}

static void
d3d12_destroy_dxcore_screen(struct pipe_screen *pscreen)
{
   struct d3d12_screen *screen = d3d12_screen(pscreen);
   d3d12_deinit_dxcore_screen(screen);
   d3d12_destroy_screen(screen);
}

static bool
d3d12_init_dxcore_screen(struct d3d12_screen *dscreen)
{
   struct d3d12_dxcore_screen *screen = d3d12_dxcore_screen(dscreen);

   screen->factory = get_dxcore_factory();
   if (!screen->factory)
      return false;

   LUID *adapter_luid = &dscreen->adapter_luid;
   if (adapter_luid->HighPart == 0 && adapter_luid->LowPart == 0)
      adapter_luid = nullptr;

   screen->adapter = choose_dxcore_adapter(screen->factory, adapter_luid);
   if (!screen->adapter) {
      debug_printf("D3D12: no suitable adapter\n");
      return false;
   }

   DXCoreHardwareID hardware_ids = {};
   uint64_t dedicated_video_memory, dedicated_system_memory, shared_system_memory;
   if (FAILED(screen->adapter->GetProperty(DXCoreAdapterProperty::HardwareID, &hardware_ids)) ||
       FAILED(screen->adapter->GetProperty(DXCoreAdapterProperty::DedicatedAdapterMemory, &dedicated_video_memory)) ||
       FAILED(screen->adapter->GetProperty(DXCoreAdapterProperty::DedicatedSystemMemory, &dedicated_system_memory)) ||
       FAILED(screen->adapter->GetProperty(DXCoreAdapterProperty::SharedSystemMemory, &shared_system_memory)) ||
       FAILED(screen->adapter->GetProperty(DXCoreAdapterProperty::DriverVersion, &screen->base.driver_version)) ||
       FAILED(screen->adapter->GetProperty(DXCoreAdapterProperty::DriverDescription,
                                           sizeof(screen->description),
                                           screen->description))) {
      debug_printf("D3D12: failed to retrieve adapter description\n");
      return false;
   }

   screen->base.vendor_id = hardware_ids.vendorID;
   screen->base.device_id = hardware_ids.deviceID;
   screen->base.subsys_id = hardware_ids.subSysID;
   screen->base.revision = hardware_ids.revision;
   screen->base.memory_size_megabytes = (dedicated_video_memory + dedicated_system_memory + shared_system_memory) >> 20;
   screen->base.base.get_name = dxcore_get_name;
   screen->base.get_memory_info = dxcore_get_memory_info;

   if (!d3d12_init_screen(&screen->base, screen->adapter)) {
      debug_printf("D3D12: failed to initialize DXCore screen\n");
      return false;
   }

   return true;
}

struct pipe_screen *
d3d12_create_dxcore_screen(struct sw_winsys *winsys, LUID *adapter_luid)
{
   struct d3d12_dxcore_screen *screen = CALLOC_STRUCT(d3d12_dxcore_screen);
   if (!screen)
      return nullptr;

   if (!d3d12_init_screen_base(&screen->base, winsys, adapter_luid)) {
      d3d12_destroy_screen(&screen->base);
      return nullptr;
   }
   screen->base.base.destroy = d3d12_destroy_dxcore_screen;
   screen->base.init = d3d12_init_dxcore_screen;
   screen->base.deinit = d3d12_deinit_dxcore_screen;

   if (!d3d12_init_dxcore_screen(&screen->base)) {
      d3d12_destroy_dxcore_screen(&screen->base.base);
      return nullptr;
   }

   return &screen->base.base;
}

struct pipe_screen *
d3d12_create_dxcore_screen_from_d3d12_device(struct sw_winsys *winsys, IUnknown* pDevUnk, LUID **out_adapter_luid)
{
   struct d3d12_dxcore_screen *screen = CALLOC_STRUCT(d3d12_dxcore_screen);
   if (!screen)
      return nullptr;

   if (FAILED(pDevUnk->QueryInterface(IID_PPV_ARGS(&screen->base.dev)))) {
      d3d12_destroy_screen(&screen->base);
      return nullptr;
   }

   LUID adapter_luid = GetAdapterLuid(screen->base.dev);
   if (!d3d12_init_screen_base(&screen->base, winsys, &adapter_luid)) {
      d3d12_destroy_screen(&screen->base);
      return nullptr;
   }
   screen->base.base.destroy = d3d12_destroy_dxcore_screen;
   screen->base.init = d3d12_init_dxcore_screen;
   screen->base.deinit = d3d12_deinit_dxcore_screen;

   if (!d3d12_init_dxcore_screen(&screen->base)) {
      d3d12_destroy_dxcore_screen(&screen->base.base);
      return nullptr;
   }

   *out_adapter_luid = &screen->base.adapter_luid;
   return &screen->base.base;
}