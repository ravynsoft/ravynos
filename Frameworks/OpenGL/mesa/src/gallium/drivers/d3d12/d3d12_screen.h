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

#ifndef D3D12_SCREEN_H
#define D3D12_SCREEN_H

#include "pipe/p_screen.h"

#include "util/slab.h"
#include "d3d12_descriptor_pool.h"

#include "nir.h"
#include "dxil_versions.h"

#include "d3d12_common.h"

struct pb_manager;
struct util_dl_library;

enum resource_dimension
{
   RESOURCE_DIMENSION_UNKNOWN = 0,
   RESOURCE_DIMENSION_BUFFER = 1,
   RESOURCE_DIMENSION_TEXTURE1D = 2,
   RESOURCE_DIMENSION_TEXTURE2D = 3,
   RESOURCE_DIMENSION_TEXTURE2DMS = 4,
   RESOURCE_DIMENSION_TEXTURE3D = 5,
   RESOURCE_DIMENSION_TEXTURECUBE = 6,
   RESOURCE_DIMENSION_TEXTURE1DARRAY = 7,
   RESOURCE_DIMENSION_TEXTURE2DARRAY = 8,
   RESOURCE_DIMENSION_TEXTURE2DMSARRAY = 9,
   RESOURCE_DIMENSION_TEXTURECUBEARRAY = 10,
   RESOURCE_DIMENSION_COUNT
};

struct d3d12_memory_info {
   uint64_t usage;
   uint64_t budget;
};

struct d3d12_screen {
   struct pipe_screen base;
   struct sw_winsys *winsys;
   LUID adapter_luid;
   char driver_uuid[PIPE_UUID_SIZE];
   char device_uuid[PIPE_UUID_SIZE];

   util_dl_library *d3d12_mod;
   ID3D12Device3 *dev;
   ID3D12Device10 *dev10;
   ID3D12CommandQueue *cmdqueue;
   bool (*init)(struct d3d12_screen *screen);
   void (*deinit)(struct d3d12_screen *screen);
   void (*get_memory_info)(struct d3d12_screen *screen, struct d3d12_memory_info *output);

   mtx_t submit_mutex;
   ID3D12Fence *fence;
   uint64_t fence_value;

   struct list_head residency_list;
   ID3D12Fence *residency_fence;
   uint64_t residency_fence_value;

   struct list_head context_list;
   unsigned context_id_list[16];
   unsigned context_id_count;

   struct set* varying_info_set;
   mtx_t varying_info_mutex;

   struct slab_parent_pool transfer_pool;
   struct pb_manager *bufmgr;
   struct pb_manager *cache_bufmgr;
   struct pb_manager *slab_cache_bufmgr;
   struct pb_manager *slab_bufmgr;
   struct pb_manager *readback_slab_cache_bufmgr;
   struct pb_manager *readback_slab_bufmgr;

   mtx_t descriptor_pool_mutex;
   struct d3d12_descriptor_pool *rtv_pool;
   struct d3d12_descriptor_pool *dsv_pool;
   struct d3d12_descriptor_pool *view_pool;

   struct d3d12_descriptor_handle null_srvs[RESOURCE_DIMENSION_COUNT];
   struct d3d12_descriptor_handle null_uavs[RESOURCE_DIMENSION_COUNT];
   struct d3d12_descriptor_handle null_rtv;

   volatile uint32_t ctx_count;
   volatile uint64_t resource_id_generator;

   /* capabilities */
   D3D_FEATURE_LEVEL max_feature_level;
   enum dxil_shader_model max_shader_model;
   D3D12_FEATURE_DATA_ARCHITECTURE architecture;
   D3D12_FEATURE_DATA_D3D12_OPTIONS opts;
   D3D12_FEATURE_DATA_D3D12_OPTIONS1 opts1;
   D3D12_FEATURE_DATA_D3D12_OPTIONS2 opts2;
   D3D12_FEATURE_DATA_D3D12_OPTIONS3 opts3;
   D3D12_FEATURE_DATA_D3D12_OPTIONS4 opts4;
   D3D12_FEATURE_DATA_D3D12_OPTIONS12 opts12;
   D3D12_FEATURE_DATA_D3D12_OPTIONS14 opts14;
#ifndef _GAMING_XBOX
   D3D12_FEATURE_DATA_D3D12_OPTIONS19 opts19;
#endif

   nir_shader_compiler_options nir_options;

   /* description */
   uint32_t vendor_id;
   uint32_t device_id;
   uint32_t subsys_id;
   uint32_t revision;
   uint64_t driver_version;
   uint64_t memory_size_megabytes;
   float timestamp_multiplier;
   bool have_load_at_vertex;
   bool support_shader_images;
   bool support_create_not_resident;

#ifdef _GAMING_XBOX
   UINT64 frame_token;
#endif
};

static inline struct d3d12_screen *
d3d12_screen(struct pipe_screen *pipe)
{
   return (struct d3d12_screen *)pipe;
}

struct d3d12_dxgi_screen {
   struct d3d12_screen base;

#ifndef _GAMING_XBOX
   struct IDXGIFactory4 *factory;
   struct IDXGIAdapter3 *adapter;
#else
   struct IDXGIAdapter *adapter;
#endif
   wchar_t description[128];
};

static inline struct d3d12_dxgi_screen *
d3d12_dxgi_screen(struct d3d12_screen *screen)
{
   return (struct d3d12_dxgi_screen *)screen;
}

struct d3d12_dxcore_screen {
   struct d3d12_screen base;

   struct IDXCoreAdapterFactory *factory;
   struct IDXCoreAdapter *adapter;
   char description[256];
};

static inline struct d3d12_dxcore_screen *
d3d12_dxcore_screen(struct d3d12_screen *screen)
{
   return (struct d3d12_dxcore_screen *)screen;
}

bool
d3d12_init_screen_base(struct d3d12_screen *screen, struct sw_winsys *winsys, LUID *adapter_luid);

bool
d3d12_init_screen(struct d3d12_screen *screen, IUnknown *adapter);

void
d3d12_deinit_screen(struct d3d12_screen *screen);

void
d3d12_destroy_screen(struct d3d12_screen *screen);

#endif
