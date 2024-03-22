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

#include <windows.h>

#include "d3d12_screen.h"
#include "d3d12_public.h"
#include "d3d12_debug.h"

#include "util/u_debug.h"
#include "util/u_memory.h"
#include "util/u_dl.h"

static const char*
dxgi_get_name(struct pipe_screen *screen)
{
   struct d3d12_dxgi_screen *dxgi_screen = d3d12_dxgi_screen(d3d12_screen(screen));
   static char buf[1000];
   if (dxgi_screen->description[0] == L'\0')
      return "D3D12 (Unknown)";

   snprintf(buf, sizeof(buf), "D3D12 (%S)", dxgi_screen->description);
   return buf;
}

static void
dxgi_get_memory_info(struct d3d12_screen *screen, struct d3d12_memory_info *output)
{
   assert(0);
}

static void
d3d12_deinit_dxgi_screen(struct d3d12_screen *dscreen)
{
   d3d12_deinit_screen(dscreen);
   struct d3d12_dxgi_screen *screen = d3d12_dxgi_screen(dscreen);
   if (screen->adapter) {
      screen->adapter->Release();
      screen->adapter = nullptr;
   }
}

static void
d3d12_destroy_dxgi_screen(struct pipe_screen *pscreen)
{
   struct d3d12_screen *screen = d3d12_screen(pscreen);
   d3d12_deinit_dxgi_screen(screen);
   d3d12_destroy_screen(screen);
}

static bool
d3d12_init_dxgi_screen(struct d3d12_screen *dscreen)
{
   if (!d3d12_init_screen(dscreen, NULL)) {
      debug_printf("D3D12: failed to initialize DXGI screen\n");
      return false;
   }

   struct d3d12_dxgi_screen *screen = d3d12_dxgi_screen(dscreen);
   IDXGIDevice1 *dxgiDevice = nullptr;

   if (FAILED(dscreen->dev->QueryInterface(IID_PPV_ARGS(&dxgiDevice)))) {
      debug_printf("D3D12: failed to query dxgi interface\n");
      return false;
   }

   if (FAILED(dxgiDevice->GetAdapter(&screen->adapter))) {
      debug_printf("D3D12: failed to get adapter\n");
      return false;
   }

   dxgiDevice->Release();
   dxgiDevice = nullptr;

   DXGI_ADAPTER_DESC adapter_desc = {};
   HRESULT res = screen->adapter->GetDesc(&adapter_desc);
   if (FAILED(res)) {
      debug_printf("D3D12: failed to retrieve adapter description\n");
      return false;
   }

   screen->base.driver_version = 0;
   screen->base.vendor_id = adapter_desc.VendorId;
   screen->base.device_id = adapter_desc.DeviceId;
   screen->base.subsys_id = adapter_desc.SubSysId;
   screen->base.revision = adapter_desc.Revision;
   // Note: memory sizes in bytes, but stored in size_t, so may be capped at 4GB.
   // In that case, adding before conversion to MB can easily overflow.
   screen->base.memory_size_megabytes = (adapter_desc.DedicatedVideoMemory >> 20) +
      (adapter_desc.DedicatedSystemMemory >> 20) +
      (adapter_desc.SharedSystemMemory >> 20);
   wcsncpy(screen->description, adapter_desc.Description, ARRAY_SIZE(screen->description));
   screen->base.base.get_name = dxgi_get_name;
   screen->base.get_memory_info = dxgi_get_memory_info;

   return true;
}

struct pipe_screen*
   d3d12_create_dxgi_screen(struct sw_winsys *winsys, LUID *adapter_luid)
{
   struct d3d12_dxgi_screen *screen = CALLOC_STRUCT(d3d12_dxgi_screen);
   if (!screen)
      return nullptr;

   d3d12_init_screen_base(&screen->base, winsys, adapter_luid);
   screen->base.base.destroy = d3d12_destroy_dxgi_screen;
   screen->base.init = d3d12_init_dxgi_screen;
   screen->base.deinit = d3d12_deinit_dxgi_screen;

   if (!d3d12_init_dxgi_screen(&screen->base)) {
      d3d12_destroy_dxgi_screen(&screen->base.base);
      return nullptr;
   }

   return &screen->base.base;
}
