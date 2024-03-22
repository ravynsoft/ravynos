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

#ifndef DZN_PRIVATE_H
#define DZN_PRIVATE_H

#define COBJMACROS

#include "vk_command_pool.h"
#include "vk_command_buffer.h"
#include "vk_cmd_queue.h"
#include "vk_debug_report.h"
#include "vk_descriptor_set_layout.h"
#include "vk_device.h"
#include "vk_image.h"
#include "vk_log.h"
#include "vk_physical_device.h"
#include "vk_pipeline_layout.h"
#include "vk_render_pass.h"
#include "vk_sync.h"
#include "vk_sync_binary.h"
#include "vk_queue.h"
#include "vk_shader_module.h"
#include "wsi_common.h"

#include "util/bitset.h"
#include "util/blob.h"
#include "util/hash_table.h"
#include "util/u_dynarray.h"
#include "util/log.h"
#include "util/xmlconfig.h"

#include "shader_enums.h"

#include "dzn_entrypoints.h"
#include "dzn_nir.h"
#include "dzn_physical_device_enum.h"

#include <vulkan/vulkan.h>
#include <vulkan/vk_icd.h>

#define D3D12_IGNORE_SDK_LAYERS
#include <unknwn.h>
#include <directx/d3d12.h>

#include "spirv_to_dxil.h"
#include "dzn_abi_helper.h"

#define DZN_SWAP(t, a, b) \
   do { \
      t __tmp = a; \
      a = b; \
      b = __tmp; \
   } while (0)

#define dzn_stub() unreachable("Unsupported feature")

struct dxil_validator;
struct util_dl_library;

struct dzn_instance;
struct dzn_device;

struct dzn_meta_indirect_draw {
   ID3D12RootSignature *root_sig;
   ID3D12PipelineState *pipeline_state;
};

enum dzn_index_type {
   DZN_NO_INDEX,
   DZN_INDEX_2B,
   DZN_INDEX_4B,
   DZN_INDEX_2B_WITH_PRIM_RESTART,
   DZN_INDEX_4B_WITH_PRIM_RESTART,
   DZN_NUM_INDEX_TYPE,
};

static inline enum dzn_index_type
dzn_index_type_from_size(uint8_t index_size)
{
   switch (index_size) {
   case 0: return DZN_NO_INDEX;
   case 2: return DZN_INDEX_2B;
   case 4: return DZN_INDEX_4B;
   default: unreachable("Invalid index size");
   }
}

static inline enum dzn_index_type
dzn_index_type_from_dxgi_format(DXGI_FORMAT format, bool prim_restart)
{
   switch (format) {
   case DXGI_FORMAT_UNKNOWN: return DZN_NO_INDEX;
   case DXGI_FORMAT_R16_UINT:
      return prim_restart ? DZN_INDEX_2B_WITH_PRIM_RESTART : DZN_INDEX_2B;
   case DXGI_FORMAT_R32_UINT:
      return prim_restart ? DZN_INDEX_4B_WITH_PRIM_RESTART : DZN_INDEX_4B;
   default: unreachable("Invalid index format");
   }
}

static inline uint8_t
dzn_index_size(enum dzn_index_type type)
{
   switch (type) {
   case DZN_NO_INDEX:
      return 0;
   case DZN_INDEX_2B_WITH_PRIM_RESTART:
   case DZN_INDEX_2B:
      return 2;
   case DZN_INDEX_4B_WITH_PRIM_RESTART:
   case DZN_INDEX_4B:
      return 4;
   default: unreachable("Invalid index type");
   }
}

struct dzn_meta_triangle_fan_rewrite_index {
   ID3D12RootSignature *root_sig;
   ID3D12PipelineState *pipeline_state;
   ID3D12CommandSignature *cmd_sig;
};

struct dzn_meta_blit_key {
   union {
      struct {
         DXGI_FORMAT out_format;
         uint32_t samples : 6;
         uint32_t loc : 4;
         uint32_t out_type : 4;
         uint32_t sampler_dim : 4;
         uint32_t src_is_array : 1;
         uint32_t resolve_mode : 3;
         uint32_t linear_filter : 1;
         uint32_t padding : 9;
      };
      const uint64_t u64;
   };
};

struct dzn_meta_blit {
   ID3D12RootSignature *root_sig;
   ID3D12PipelineState *pipeline_state;
};

struct dzn_meta_blits {
   mtx_t shaders_lock;
   D3D12_SHADER_BYTECODE vs;
   struct hash_table *fs;
   mtx_t contexts_lock;
   struct hash_table_u64 *contexts;
};

const struct dzn_meta_blit *
dzn_meta_blits_get_context(struct dzn_device *device,
                           const struct dzn_meta_blit_key *key);

#define MAX_SYNC_TYPES 3
#define MAX_QUEUE_FAMILIES 2

struct dzn_physical_device {
   struct vk_physical_device vk;

   struct vk_physical_device_dispatch_table dispatch;

   IUnknown *adapter;
   struct dzn_physical_device_desc desc;

   uint32_t queue_family_count;
   struct dzn_queue_family {
      VkQueueFamilyProperties props;
      D3D12_COMMAND_QUEUE_DESC desc;
   } queue_families[MAX_QUEUE_FAMILIES];

   uint8_t pipeline_cache_uuid[VK_UUID_SIZE];
   uint8_t device_uuid[VK_UUID_SIZE];
   uint8_t driver_uuid[VK_UUID_SIZE];

   struct wsi_device wsi_device;

   ID3D12Device4 *dev;
   ID3D12Device10 *dev10;
   ID3D12Device11 *dev11;
   ID3D12Device12 *dev12;
   ID3D12Device13 *dev13;
   D3D_FEATURE_LEVEL feature_level;
   D3D_SHADER_MODEL shader_model;
   D3D_ROOT_SIGNATURE_VERSION root_sig_version;
   D3D12_FEATURE_DATA_ARCHITECTURE1 architecture;
   D3D12_FEATURE_DATA_D3D12_OPTIONS options;
   D3D12_FEATURE_DATA_D3D12_OPTIONS1 options1;
   D3D12_FEATURE_DATA_D3D12_OPTIONS2 options2;
   D3D12_FEATURE_DATA_D3D12_OPTIONS3 options3;
   D3D12_FEATURE_DATA_D3D12_OPTIONS4 options4;
   D3D12_FEATURE_DATA_D3D12_OPTIONS12 options12;
   D3D12_FEATURE_DATA_D3D12_OPTIONS13 options13;
   D3D12_FEATURE_DATA_D3D12_OPTIONS14 options14;
   D3D12_FEATURE_DATA_D3D12_OPTIONS15 options15;
   D3D12_FEATURE_DATA_D3D12_OPTIONS16 options16;
   D3D12_FEATURE_DATA_D3D12_OPTIONS17 options17;
   D3D12_FEATURE_DATA_D3D12_OPTIONS19 options19;
   VkPhysicalDeviceMemoryProperties memory;
   D3D12_HEAP_FLAGS heap_flags_for_mem_type[VK_MAX_MEMORY_TYPES];
   const struct vk_sync_type *sync_types[MAX_SYNC_TYPES + 1];
   float timestamp_period;
   bool support_a4b4g4r4;
};

D3D12_FEATURE_DATA_FORMAT_SUPPORT
dzn_physical_device_get_format_support(struct dzn_physical_device *pdev,
                                       VkFormat format,
                                       VkImageCreateFlags create_flags);

uint32_t
dzn_physical_device_get_mem_type_mask_for_resource(const struct dzn_physical_device *pdev,
                                                   const D3D12_RESOURCE_DESC *desc,
                                                   bool shared);

enum dxil_shader_model
dzn_get_shader_model(const struct dzn_physical_device *pdev);

#define dzn_debug_ignored_stype(sType) \
   mesa_logd("%s: ignored VkStructureType %u\n", __func__, (sType))

PFN_D3D12_SERIALIZE_VERSIONED_ROOT_SIGNATURE
d3d12_get_serialize_root_sig(struct util_dl_library *d3d12_mod);

void
d3d12_enable_debug_layer(struct util_dl_library *d3d12_mod, ID3D12DeviceFactory *factory);

void
d3d12_enable_gpu_validation(struct util_dl_library *d3d12_mod, ID3D12DeviceFactory *factory);

ID3D12Device4 *
d3d12_create_device(struct util_dl_library *d3d12_mod, IUnknown *adapter, ID3D12DeviceFactory *factory, bool experimental_features);

struct dzn_queue {
   struct vk_queue vk;

   ID3D12CommandQueue *cmdqueue;
   ID3D12Fence *fence;
   uint64_t fence_point;
};

struct dzn_descriptor_heap {
   ID3D12DescriptorHeap *heap;
   SIZE_T cpu_base;
   uint64_t gpu_base;
   uint32_t desc_count;
   uint32_t desc_sz;
};

struct dzn_device_descriptor_heap {
   struct dzn_descriptor_heap heap;
   mtx_t lock;
   struct util_dynarray slot_freelist;
   uint32_t next_alloc_slot;
};

#define NUM_POOL_TYPES D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER + 1

struct dzn_device {
   struct vk_device vk;
   struct vk_device_extension_table enabled_extensions;
   struct vk_device_dispatch_table cmd_dispatch;

   ID3D12Device4 *dev;
   ID3D12Device10 *dev10;
   ID3D12Device11 *dev11;
   ID3D12Device12 *dev12;
   ID3D12Device13 *dev13;
   ID3D12DeviceConfiguration *dev_config;

   struct dzn_meta_indirect_draw indirect_draws[DZN_NUM_INDIRECT_DRAW_TYPES];
   struct dzn_meta_triangle_fan_rewrite_index triangle_fan[DZN_NUM_INDEX_TYPE];
   struct dzn_meta_blits blits;

   struct {
#define DZN_QUERY_REFS_SECTION_SIZE 4096
#define DZN_QUERY_REFS_ALL_ONES_OFFSET 0
#define DZN_QUERY_REFS_ALL_ZEROS_OFFSET (DZN_QUERY_REFS_ALL_ONES_OFFSET + DZN_QUERY_REFS_SECTION_SIZE)
#define DZN_QUERY_REFS_RES_SIZE (DZN_QUERY_REFS_ALL_ZEROS_OFFSET + DZN_QUERY_REFS_SECTION_SIZE)
      ID3D12Resource *refs;
   } queries;

   /* Will be the app's graphics queue if there's exactly one, otherwise this will be 
    * a dedicated graphics queue to host swapchain blits.
    */
   bool need_swapchain_blits;
   struct dzn_queue *swapchain_queue;

   bool bindless;
   bool support_static_samplers;
   struct dzn_device_descriptor_heap device_heaps[NUM_POOL_TYPES];
};

void dzn_meta_finish(struct dzn_device *device);

VkResult dzn_meta_init(struct dzn_device *device);

const struct dzn_meta_blit *
dzn_meta_blits_get_context(struct dzn_device *device,
                           const struct dzn_meta_blit_key *key);

ID3D12RootSignature *
dzn_device_create_root_sig(struct dzn_device *device,
                           const D3D12_VERSIONED_ROOT_SIGNATURE_DESC *desc);

struct dzn_device_memory {
   struct vk_object_base base;

   struct list_head link;

   /* Dedicated image/buffer resource. Can be used for import (e.g. from a swapchain)
    * or just from a dedicated allocation request.
    */
   ID3D12Resource *dedicated_res;

   ID3D12Heap *heap;
   VkDeviceSize size;

   /* A buffer-resource spanning the entire heap, used for mapping memory */
   ID3D12Resource *map_res;

   VkDeviceSize map_size;
   void *map;

   /* If the resource is exportable, this is the pre-created handle for that */
   HANDLE export_handle;

   /* These flags need to be added into all resources created on this heap */
   D3D12_RESOURCE_FLAGS res_flags;
};

enum dzn_cmd_bindpoint_dirty {
   DZN_CMD_BINDPOINT_DIRTY_PIPELINE = 1 << 0,
   DZN_CMD_BINDPOINT_DIRTY_DYNAMIC_BUFFERS = 1 << 1,
   DZN_CMD_BINDPOINT_DIRTY_SYSVALS = 1 << 2,
   DZN_CMD_BINDPOINT_DIRTY_DESC_SET0 = 1 << 3,
   DZN_CMD_BINDPOINT_DIRTY_DESC_SET1 = 1 << 4,
   DZN_CMD_BINDPOINT_DIRTY_DESC_SET2 = 1 << 5,
   DZN_CMD_BINDPOINT_DIRTY_DESC_SET3 = 1 << 6,
   DZN_CMD_BINDPOINT_DIRTY_DESC_SET4 = 1 << 7,
   DZN_CMD_BINDPOINT_DIRTY_DESC_SET5 = 1 << 8,
   DZN_CMD_BINDPOINT_DIRTY_DESC_SET6 = 1 << 9,
   DZN_CMD_BINDPOINT_DIRTY_DESC_SET7 = 1 << 10,
   DZN_CMD_BINDPOINT_DIRTY_DESC_SETS =
      DZN_CMD_BINDPOINT_DIRTY_DESC_SET0 |
      DZN_CMD_BINDPOINT_DIRTY_DESC_SET1 |
      DZN_CMD_BINDPOINT_DIRTY_DESC_SET2 |
      DZN_CMD_BINDPOINT_DIRTY_DESC_SET3 |
      DZN_CMD_BINDPOINT_DIRTY_DESC_SET4 |
      DZN_CMD_BINDPOINT_DIRTY_DESC_SET5 |
      DZN_CMD_BINDPOINT_DIRTY_DESC_SET6 |
      DZN_CMD_BINDPOINT_DIRTY_DESC_SET7,
   DZN_CMD_BINDPOINT_DIRTY_HEAPS =
      DZN_CMD_BINDPOINT_DIRTY_DYNAMIC_BUFFERS |
      DZN_CMD_BINDPOINT_DIRTY_SYSVALS |
      DZN_CMD_BINDPOINT_DIRTY_DESC_SETS,
};

enum dzn_cmd_dirty {
   DZN_CMD_DIRTY_VIEWPORTS = 1 << 0,
   DZN_CMD_DIRTY_SCISSORS = 1 << 1,
   DZN_CMD_DIRTY_IB = 1 << 2,
   DZN_CMD_DIRTY_STENCIL_REF = 1 << 3,
   DZN_CMD_DIRTY_STENCIL_COMPARE_MASK = 1 << 4,
   DZN_CMD_DIRTY_STENCIL_WRITE_MASK = 1 << 5,
   DZN_CMD_DIRTY_BLEND_CONSTANTS = 1 << 6,
   DZN_CMD_DIRTY_DEPTH_BOUNDS = 1 << 7,
   DZN_CMD_DIRTY_DEPTH_BIAS = 1 << 8,
};

#define MAX_VBS D3D12_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT
#define MAX_VP D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE
#define MAX_SCISSOR D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE
#define MAX_SETS 8
#define MAX_DYNAMIC_UNIFORM_BUFFERS 8
#define MAX_DYNAMIC_STORAGE_BUFFERS 4
#define MAX_DYNAMIC_BUFFERS                                                  \
   (MAX_DYNAMIC_UNIFORM_BUFFERS + MAX_DYNAMIC_STORAGE_BUFFERS)
#define MAX_PUSH_CONSTANT_DWORDS 32

#define NUM_BIND_POINT VK_PIPELINE_BIND_POINT_COMPUTE + 1

#define dzn_foreach_pool_type(type) \
   for (D3D12_DESCRIPTOR_HEAP_TYPE type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; \
        type <= D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER; \
        type = (D3D12_DESCRIPTOR_HEAP_TYPE)(type + 1))

struct dzn_cmd_event_signal {
   struct dzn_event *event;
   bool value;
};

struct dzn_cmd_buffer;

struct dzn_descriptor_state {
   struct {
      const struct dzn_descriptor_set *set;
      uint32_t dynamic_offsets[MAX_DYNAMIC_BUFFERS];
   } sets[MAX_SETS];
   struct dzn_descriptor_heap *heaps[NUM_POOL_TYPES];
};

struct dzn_sampler;
struct dzn_image_view;
struct dzn_buffer_view;

struct dzn_buffer_desc {
   VkDescriptorType type;
   struct dzn_buffer *buffer;
   VkDeviceSize range;
   VkDeviceSize offset;
};

#define MAX_DESCS_PER_SAMPLER_HEAP     D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE
#define MAX_DESCS_PER_CBV_SRV_UAV_HEAP D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_1

VkResult
dzn_descriptor_heap_init(struct dzn_descriptor_heap *heap,
                         struct dzn_device *device,
                         D3D12_DESCRIPTOR_HEAP_TYPE type,
                         uint32_t desc_count,
                         bool shader_visible);

void
dzn_descriptor_heap_finish(struct dzn_descriptor_heap *heap);

D3D12_CPU_DESCRIPTOR_HANDLE
dzn_descriptor_heap_get_cpu_handle(const struct dzn_descriptor_heap *heap, uint32_t slot);

D3D12_GPU_DESCRIPTOR_HANDLE
dzn_descriptor_heap_get_gpu_handle(const struct dzn_descriptor_heap *heap, uint32_t slot);

void
dzn_descriptor_heap_write_image_view_desc(struct dzn_device *device,
                                          struct dzn_descriptor_heap *heap,
                                          uint32_t heap_offset,
                                          bool writeable,
                                          bool cube_as_2darray,
                                          const struct dzn_image_view *iview);

void
dzn_descriptor_heap_write_buffer_view_desc(struct dzn_device *device,
                                           struct dzn_descriptor_heap *heap,
                                           uint32_t heap_offset,
                                           bool writeable,
                                           const struct dzn_buffer_view *bview);

void
dzn_descriptor_heap_write_buffer_desc(struct dzn_device *device,
                                      struct dzn_descriptor_heap *heap,
                                      uint32_t heap_offset,
                                      bool writeable,
                                      const struct dzn_buffer_desc *bdesc);

void
dzn_descriptor_heap_write_sampler_desc(struct dzn_device *device,
                                       struct dzn_descriptor_heap *heap,
                                       uint32_t desc_offset,
                                       const struct dzn_sampler *sampler);

void
dzn_descriptor_heap_copy(struct dzn_device *device,
                         struct dzn_descriptor_heap *dst_heap, uint32_t dst_heap_offset,
                         const struct dzn_descriptor_heap *src_heap, uint32_t src_heap_offset,
                         uint32_t desc_count, D3D12_DESCRIPTOR_HEAP_TYPE type);

struct dzn_descriptor_heap_pool_entry {
   struct list_head link;
   struct dzn_descriptor_heap heap;
};

struct dzn_descriptor_heap_pool {
   const VkAllocationCallbacks *alloc;
   D3D12_DESCRIPTOR_HEAP_TYPE type;
   bool shader_visible;
   struct list_head active_heaps, free_heaps;
   uint32_t offset;
   uint32_t desc_sz;
};

void
dzn_descriptor_heap_pool_init(struct dzn_descriptor_heap_pool *pool,
                              struct dzn_device *device,
                              D3D12_DESCRIPTOR_HEAP_TYPE type,
                              bool shader_visible,
                              const VkAllocationCallbacks *alloc);

void
dzn_descriptor_heap_pool_finish(struct dzn_descriptor_heap_pool *pool);

void
dzn_descriptor_heap_pool_reset(struct dzn_descriptor_heap_pool *pool);

VkResult
dzn_descriptor_heap_pool_alloc_slots(struct dzn_descriptor_heap_pool *pool,
                                     struct dzn_device *device,
                                     uint32_t num_slots,
                                     struct dzn_descriptor_heap **heap,
                                     uint32_t *first_slot);

int
dzn_device_descriptor_heap_alloc_slot(struct dzn_device *device,
                                      D3D12_DESCRIPTOR_HEAP_TYPE type);

void
dzn_device_descriptor_heap_free_slot(struct dzn_device *device,
                                     D3D12_DESCRIPTOR_HEAP_TYPE type,
                                     int slot);

struct dzn_cmd_buffer_query_range {
   struct dzn_query_pool *qpool;
   uint32_t start, count;
};

struct dzn_cmd_buffer_query_pool_state {
   struct util_dynarray reset, collect, signal, zero;
};

struct dzn_internal_resource {
   struct list_head link;
   ID3D12Resource *res;
   uint64_t size;
};

enum dzn_event_state {
   DZN_EVENT_STATE_RESET = 0,
   DZN_EVENT_STATE_SET = 1,
};

struct dzn_cmd_buffer_push_constant_state {
   uint32_t offset;
   uint32_t end;
   uint32_t values[MAX_PUSH_CONSTANT_DWORDS];
};

struct dzn_rendering_attachment {
   struct dzn_image_view *iview;
   VkImageLayout layout;
   struct {
      VkResolveModeFlagBits mode;
      struct dzn_image_view *iview;
      VkImageLayout layout;
   } resolve;
   VkAttachmentStoreOp store_op;
};

struct dzn_graphics_pipeline_variant_key {
   D3D12_INDEX_BUFFER_STRIP_CUT_VALUE ib_strip_cut;
   struct {
      int constant_factor;
      float slope_factor;
      float clamp;
   } depth_bias;
   struct {
      struct {
         uint32_t ref, compare_mask, write_mask;
      } front, back;
   } stencil_test;
};

struct dzn_graphics_pipeline_variant {
   struct dzn_graphics_pipeline_variant_key key;
   ID3D12PipelineState *state;
};

#define MAX_RTS D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT

struct dzn_cmd_buffer_state {
   const struct dzn_pipeline *pipeline;
   struct dzn_descriptor_heap *heaps[NUM_POOL_TYPES];
   struct dzn_graphics_pipeline_variant_key pipeline_variant;
   struct {
      VkRenderingFlags flags;
      D3D12_RECT area;
      uint32_t layer_count;
      uint32_t view_mask;
      struct {
         uint32_t color_count;
         struct dzn_rendering_attachment colors[MAX_RTS];
         struct dzn_rendering_attachment depth, stencil;
      } attachments;
   } render;
   struct {
      BITSET_DECLARE(dirty, MAX_VBS);
      D3D12_VERTEX_BUFFER_VIEW views[MAX_VBS];
   } vb;
   struct {
      D3D12_INDEX_BUFFER_VIEW view;
   } ib;
   struct {
      struct {
         struct {
            uint32_t ref, compare_mask, write_mask;
         } front, back;
      } stencil_test;
      struct {
         float min, max;
      } depth_bounds;
   } zsa;
   struct {
      float constants[4];
   } blend;
   D3D12_VIEWPORT viewports[MAX_VP];
   D3D12_RECT scissors[MAX_SCISSOR];
   struct {
      struct dzn_cmd_buffer_push_constant_state gfx, compute;
   } push_constant;
   uint32_t dirty;
   struct {
      struct dzn_pipeline *pipeline;
      ID3D12RootSignature *root_sig;
      struct dzn_descriptor_state desc_state;
      uint32_t dirty;
   } bindpoint[NUM_BIND_POINT];
   union {
      struct dxil_spirv_vertex_runtime_data gfx;
      struct dxil_spirv_compute_runtime_data compute;
   } sysvals;
   struct {
      uint32_t num_views;
      uint32_t view_mask;
   } multiview;
};

struct dzn_cmd_buffer_rtv_key {
   const struct dzn_image *image;
   D3D12_RENDER_TARGET_VIEW_DESC desc;
};

struct dzn_cmd_buffer_rtv_entry {
   struct dzn_cmd_buffer_rtv_key key;
   D3D12_CPU_DESCRIPTOR_HANDLE handle;
};

struct dzn_cmd_buffer_dsv_key {
   const struct dzn_image *image;
   D3D12_DEPTH_STENCIL_VIEW_DESC desc;
};

struct dzn_cmd_buffer_dsv_entry {
   struct dzn_cmd_buffer_dsv_key key;
   D3D12_CPU_DESCRIPTOR_HANDLE handle;
};

enum dzn_internal_buf_bucket {
   DZN_INTERNAL_BUF_UPLOAD,
   DZN_INTERNAL_BUF_DEFAULT,
   DZN_INTERNAL_BUF_BUCKET_COUNT,
};

struct dzn_cmd_buffer {
   struct vk_command_buffer vk;
   struct dzn_cmd_buffer_state state;

   struct {
      struct hash_table *ht;
      struct util_dynarray reset;
      struct util_dynarray signal;
   } queries;

   struct {
      struct hash_table *ht;
      struct util_dynarray signal;
   } events;

   struct {
      struct hash_table *ht;
      struct dzn_descriptor_heap_pool pool;
   } rtvs, dsvs;

   bool enhanced_barriers;
   struct hash_table *transition_barriers;

   struct dzn_descriptor_heap_pool cbv_srv_uav_pool, sampler_pool;
   D3D12_CPU_DESCRIPTOR_HANDLE null_rtv;

   struct list_head internal_bufs[DZN_INTERNAL_BUF_BUCKET_COUNT];
   struct dzn_internal_resource *cur_upload_buf;
   uint64_t cur_upload_buf_offset;

   ID3D12CommandAllocator *cmdalloc;
   ID3D12GraphicsCommandList1 *cmdlist;
   ID3D12GraphicsCommandList8 *cmdlist8;
   ID3D12GraphicsCommandList9 *cmdlist9;

   D3D12_COMMAND_LIST_TYPE type;
   D3D12_BARRIER_SYNC valid_sync;
   D3D12_BARRIER_ACCESS valid_access;
};

struct dxil_spirv_bindless_entry;
struct dzn_descriptor_pool {
   struct vk_object_base base;
   VkAllocationCallbacks alloc;

   uint32_t set_count;
   uint32_t used_set_count;
   struct dzn_descriptor_set *sets;
   union {
      struct dzn_descriptor_heap heaps[NUM_POOL_TYPES];
      struct {
         ID3D12Resource *buf;
         volatile struct dxil_spirv_bindless_entry *map;
         uint64_t gpuva;
      } bindless;
   };
   uint32_t desc_count[NUM_POOL_TYPES];
   uint32_t used_desc_count[NUM_POOL_TYPES];
   uint32_t free_offset[NUM_POOL_TYPES];
};

#define MAX_SHADER_VISIBILITIES (D3D12_SHADER_VISIBILITY_PIXEL + 1)
#define STATIC_SAMPLER_TAG (~0u - 1)

struct dzn_descriptor_set_layout_binding {
   VkDescriptorType type;
   uint32_t stages;
   D3D12_SHADER_VISIBILITY visibility;
   uint32_t base_shader_register;
   uint32_t range_idx[NUM_POOL_TYPES];
   union {
      /* For sampler types, index into the set layout's immutable sampler list,
       * or STATIC_SAMPLER_TAG for static samplers, or ~0 for dynamic samplers. */
      uint32_t immutable_sampler_idx;
      /* For dynamic buffer types, index into the set's dynamic buffer list.
       * For non-dynamic buffer types, index into the set's buffer descriptor slot list when bindless. */
      uint32_t buffer_idx;
   };
   bool variable_size;
};

struct dzn_descriptor_set_layout {
   struct vk_descriptor_set_layout vk;

   /* Ranges are bucketed by shader visibility so that each visibility can have
    * a single descriptor table in the root signature, with all ranges concatenated. */
   uint32_t range_count[MAX_SHADER_VISIBILITIES][NUM_POOL_TYPES];
   const D3D12_DESCRIPTOR_RANGE1 *ranges[MAX_SHADER_VISIBILITIES][NUM_POOL_TYPES];

   /* The number of descriptors across all ranges/visibilities for this type */
   uint32_t range_desc_count[NUM_POOL_TYPES];

   /* Static samplers actually go into the D3D12 root signature.
    * Immutable samplers are only stored here to be copied into descriptor tables later. */
   uint32_t static_sampler_count;
   const D3D12_STATIC_SAMPLER_DESC1 *static_samplers;
   uint32_t immutable_sampler_count;
   const struct dzn_sampler **immutable_samplers;

   struct {
      uint32_t bindings[MAX_DYNAMIC_BUFFERS];
      uint32_t count;
      uint32_t desc_count;
      uint32_t range_offset;
   } dynamic_buffers;
   uint32_t buffer_count;
   uint32_t stages;

   uint32_t binding_count;
   const struct dzn_descriptor_set_layout_binding *bindings;
};

struct dzn_descriptor_set {
   struct vk_object_base base;
   struct dzn_buffer_desc dynamic_buffers[MAX_DYNAMIC_BUFFERS];
   struct dzn_descriptor_pool *pool;
   /* The offset in the current active staging descriptor heap for the set's pool. */
   uint32_t heap_offsets[NUM_POOL_TYPES];
   /* The number of descriptors needed for this set */
   uint32_t heap_sizes[NUM_POOL_TYPES];
   /* Layout (and pool) is null for a freed descriptor set */
   const struct dzn_descriptor_set_layout *layout;
};

struct dzn_pipeline_layout_set {
   /* The offset from the start of a descriptor table where the set should be copied */
   uint32_t heap_offsets[NUM_POOL_TYPES];
   struct {
      uint32_t primary, alt;
   } dynamic_buffer_heap_offsets[MAX_DYNAMIC_BUFFERS];
   uint32_t dynamic_buffer_count;
   uint32_t range_desc_count[NUM_POOL_TYPES];
};

enum dzn_pipeline_binding_class {
   DZN_PIPELINE_BINDING_NORMAL,
   DZN_PIPELINE_BINDING_DYNAMIC_BUFFER,
   DZN_PIPELINE_BINDING_STATIC_SAMPLER,
};

struct dzn_pipeline_layout {
   struct vk_pipeline_layout vk;
   struct dzn_pipeline_layout_set sets[MAX_SETS];
   struct {
      uint32_t binding_count;
      /* A mapping from a binding value, which can be shared among multiple descriptors
       * in an array, to unique 0-based registers. This mapping is applied to the shaders
       * during pipeline creation. */
      uint32_t *base_reg;
      uint8_t *binding_class;
   } binding_translation[MAX_SETS];
   uint32_t set_count;
   /* How much space needs to be allocated to copy descriptors during cmdbuf recording? */
   uint32_t desc_count[NUM_POOL_TYPES];
   uint32_t dynamic_buffer_count;
   struct {
      uint32_t param_count;
      uint32_t sets_param_count;
      uint32_t sysval_cbv_param_idx;
      uint32_t push_constant_cbv_param_idx;
      uint32_t dynamic_buffer_bindless_param_idx;
      D3D12_DESCRIPTOR_HEAP_TYPE type[MAX_SHADER_VISIBILITIES];
      ID3D12RootSignature *sig;
   } root;
   struct {
      uint8_t hash[SHA1_DIGEST_LENGTH];
   } stages[MESA_VULKAN_SHADER_STAGES];
};

struct dzn_descriptor_update_template_entry {
   VkDescriptorType type;
   uint32_t desc_count;
   uint32_t buffer_idx;
   struct {
      uint32_t cbv_srv_uav;
      union {
         uint32_t sampler, extra_srv;
      };
   } heap_offsets;
   struct {
      size_t offset;
      size_t stride;
   } user_data;
};

struct dzn_descriptor_update_template {
   struct vk_object_base base;
   uint32_t entry_count;
   const struct dzn_descriptor_update_template_entry *entries;
};

enum dzn_register_space {
   DZN_REGISTER_SPACE_SYSVALS = MAX_SETS,
   DZN_REGISTER_SPACE_PUSH_CONSTANT,
};

#define D3D12_PIPELINE_STATE_STREAM_DESC_SIZE(__type) \
   ALIGN_POT(ALIGN_POT(sizeof(D3D12_PIPELINE_STATE_SUBOBJECT_TYPE), alignof(__type)) + sizeof(__type), alignof(void *))

static_assert(sizeof(D3D12_DEPTH_STENCIL_DESC2) > sizeof(D3D12_DEPTH_STENCIL_DESC1),
              "Using just one of these descs in the max size calculation");
static_assert(sizeof(D3D12_RASTERIZER_DESC) >= sizeof(D3D12_RASTERIZER_DESC1) &&
              sizeof(D3D12_RASTERIZER_DESC) >= sizeof(D3D12_RASTERIZER_DESC2),
              "Using just one of these descs in the max size calculation");

#define MAX_GFX_PIPELINE_STATE_STREAM_SIZE \
   D3D12_PIPELINE_STATE_STREAM_DESC_SIZE(ID3D12RootSignature *) + \
   (D3D12_PIPELINE_STATE_STREAM_DESC_SIZE(D3D12_SHADER_BYTECODE) * 5) + /* VS, PS, DS, HS, GS */ \
   D3D12_PIPELINE_STATE_STREAM_DESC_SIZE(D3D12_STREAM_OUTPUT_DESC) + \
   D3D12_PIPELINE_STATE_STREAM_DESC_SIZE(D3D12_BLEND_DESC) + \
   D3D12_PIPELINE_STATE_STREAM_DESC_SIZE(UINT) + /* SampleMask */ \
   D3D12_PIPELINE_STATE_STREAM_DESC_SIZE(D3D12_RASTERIZER_DESC) + \
   D3D12_PIPELINE_STATE_STREAM_DESC_SIZE(D3D12_INPUT_LAYOUT_DESC) + \
   D3D12_PIPELINE_STATE_STREAM_DESC_SIZE(D3D12_INDEX_BUFFER_STRIP_CUT_VALUE) + \
   D3D12_PIPELINE_STATE_STREAM_DESC_SIZE(D3D12_PRIMITIVE_TOPOLOGY_TYPE) + \
   D3D12_PIPELINE_STATE_STREAM_DESC_SIZE(struct D3D12_RT_FORMAT_ARRAY) + \
   D3D12_PIPELINE_STATE_STREAM_DESC_SIZE(DXGI_FORMAT) + /* DS format */ \
   D3D12_PIPELINE_STATE_STREAM_DESC_SIZE(DXGI_SAMPLE_DESC) + \
   D3D12_PIPELINE_STATE_STREAM_DESC_SIZE(D3D12_NODE_MASK) + \
   D3D12_PIPELINE_STATE_STREAM_DESC_SIZE(D3D12_CACHED_PIPELINE_STATE) + \
   D3D12_PIPELINE_STATE_STREAM_DESC_SIZE(D3D12_PIPELINE_STATE_FLAGS) + \
   D3D12_PIPELINE_STATE_STREAM_DESC_SIZE(D3D12_DEPTH_STENCIL_DESC2) + \
   D3D12_PIPELINE_STATE_STREAM_DESC_SIZE(D3D12_VIEW_INSTANCING_DESC) + \
   D3D12_PIPELINE_STATE_STREAM_DESC_SIZE(D3D12_PIPELINE_STATE_FLAGS)

#define MAX_COMPUTE_PIPELINE_STATE_STREAM_SIZE \
   D3D12_PIPELINE_STATE_STREAM_DESC_SIZE(ID3D12RootSignature *) + \
   D3D12_PIPELINE_STATE_STREAM_DESC_SIZE(D3D12_SHADER_BYTECODE) + \
   D3D12_PIPELINE_STATE_STREAM_DESC_SIZE(D3D12_CACHED_PIPELINE_STATE)

struct dzn_pipeline {
   struct vk_object_base base;
   VkPipelineBindPoint type;
   struct dzn_device *device;
   struct {
      uint32_t sets_param_count;
      uint32_t sysval_cbv_param_idx;
      uint32_t push_constant_cbv_param_idx;
      uint32_t dynamic_buffer_bindless_param_idx;
      D3D12_DESCRIPTOR_HEAP_TYPE type[MAX_SHADER_VISIBILITIES];
      ID3D12RootSignature *sig;
   } root;
   struct dzn_pipeline_layout_set sets[MAX_SETS];
   uint32_t set_count;
   uint32_t desc_count[NUM_POOL_TYPES];
   uint32_t dynamic_buffer_count;
   ID3D12PipelineState *state;
};

extern const struct vk_pipeline_cache_object_ops dzn_cached_blob_ops;

enum dzn_indirect_draw_cmd_sig_type {
   DZN_INDIRECT_DRAW_CMD_SIG,
   DZN_INDIRECT_INDEXED_DRAW_CMD_SIG,
   DZN_INDIRECT_DRAW_TRIANGLE_FAN_CMD_SIG,
   DZN_NUM_INDIRECT_DRAW_CMD_SIGS,
};

struct dzn_graphics_pipeline {
   struct dzn_pipeline base;
   struct {
      unsigned count;
      uint32_t strides[MAX_VBS];
   } vb;

   struct {
      bool triangle_fan;
      D3D_PRIMITIVE_TOPOLOGY topology;
   } ia;

   struct {
      unsigned count;
      bool dynamic;
      D3D12_VIEWPORT desc[MAX_VP];
   } vp;

   struct {
      unsigned count;
      bool dynamic;
      D3D12_RECT desc[MAX_SCISSOR];
   } scissor;

   struct {
      struct {
         bool enable;
         bool dynamic_ref;
         bool dynamic_write_mask;
         bool dynamic_compare_mask;
         struct {
            uint32_t ref;
            uint32_t write_mask;
            uint32_t compare_mask;
            bool uses_ref;
        } front, back;
      } stencil_test;
      struct {
         bool enable;
         bool dynamic;
         float min, max;
      } depth_bounds;
      bool dynamic_depth_bias;
      DXGI_FORMAT ds_fmt;
   } zsa;

   struct {
      bool dynamic_constants;
      float constants[4];
   } blend;

   bool rast_disabled_from_missing_position;
   bool use_gs_for_polygon_mode_point;

   struct {
      uint32_t view_mask;
      bool native_view_instancing;
   } multiview;

   struct {
      uintptr_t stream_buf[MAX_GFX_PIPELINE_STATE_STREAM_SIZE / sizeof(uintptr_t)];
      D3D12_PIPELINE_STATE_STREAM_DESC stream_desc;
      struct {
         uint32_t ib_strip_cut;
         uint32_t rast;
         uint32_t ds;
      } desc_offsets;
      D3D12_INPUT_ELEMENT_DESC inputs[D3D12_VS_INPUT_REGISTER_COUNT];
      struct {
         D3D12_SHADER_BYTECODE *bc;
         nir_shader *nir;
      } shaders[MESA_VULKAN_SHADER_STAGES];
   } templates;

   struct hash_table *variants;

   ID3D12CommandSignature *indirect_cmd_sigs[DZN_NUM_INDIRECT_DRAW_CMD_SIGS];
};

#define dzn_graphics_pipeline_get_desc(pipeline, streambuf, name) \
   (void *)(pipeline->templates.desc_offsets.name == 0 ? NULL : \
            (uint8_t *)streambuf + pipeline->templates.desc_offsets.name)

#define dzn_graphics_pipeline_get_desc_template(pipeline, name) \
   (const void *)dzn_graphics_pipeline_get_desc(pipeline, pipeline->templates.stream_buf, name)

ID3D12PipelineState *
dzn_graphics_pipeline_get_state(struct dzn_graphics_pipeline *pipeline,
                                const struct dzn_graphics_pipeline_variant_key *key);

ID3D12CommandSignature *
dzn_graphics_pipeline_get_indirect_cmd_sig(struct dzn_graphics_pipeline *pipeline,
                                           enum dzn_indirect_draw_cmd_sig_type cmd_sig_type);

VkFormat dzn_graphics_pipeline_patch_vi_format(VkFormat format);

struct dzn_compute_pipeline {
   struct dzn_pipeline base;
   struct {
      uint32_t x, y, z;
   } local_size;

   ID3D12CommandSignature *indirect_cmd_sig;
};

ID3D12CommandSignature *
dzn_compute_pipeline_get_indirect_cmd_sig(struct dzn_compute_pipeline *pipeline);

#define MAX_MIP_LEVELS 14

struct dzn_image {
   struct vk_image vk;

   struct {
      uint32_t row_stride;
      uint32_t size;
   } linear;
   D3D12_RESOURCE_DESC desc;
   ID3D12Resource *res;
   struct dzn_device_memory *mem;
   uint32_t castable_format_count;
   const DXGI_FORMAT *castable_formats;

   D3D12_BARRIER_ACCESS valid_access;
};

bool
dzn_image_formats_are_compatible(const struct dzn_device *device,
                                 VkFormat orig_fmt, VkFormat new_fmt,
                                 VkImageUsageFlags usage,
                                 VkImageAspectFlagBits aspect);

void
dzn_image_align_extent(const struct dzn_image *image,
                       VkExtent3D *extent);

DXGI_FORMAT
dzn_image_get_dxgi_format(const struct dzn_physical_device *pdev,
                          VkFormat format,
                          VkImageUsageFlags usage,
                          VkImageAspectFlags aspects);

VkFormat
dzn_image_get_plane_format(VkFormat fmt, VkImageAspectFlags aspect);

DXGI_FORMAT
dzn_image_get_placed_footprint_format(const struct dzn_physical_device *pdev,
                                      VkFormat fmt, VkImageAspectFlags aspect);

D3D12_DEPTH_STENCIL_VIEW_DESC
dzn_image_get_dsv_desc(const struct dzn_image *image,
                       const VkImageSubresourceRange *range,
                       uint32_t level);

D3D12_RENDER_TARGET_VIEW_DESC
dzn_image_get_rtv_desc(const struct dzn_image *image,
                       const VkImageSubresourceRange *range,
                       uint32_t level);

D3D12_RESOURCE_STATES
dzn_image_layout_to_state(const struct dzn_image *image,
                          VkImageLayout layout,
                          VkImageAspectFlagBits aspect,
                          D3D12_COMMAND_LIST_TYPE type);

D3D12_BARRIER_LAYOUT
dzn_vk_layout_to_d3d_layout(VkImageLayout layout,
                            D3D12_COMMAND_LIST_TYPE type,
                            VkImageAspectFlags aspect);

uint32_t
dzn_image_layers_get_subresource_index(const struct dzn_image *image,
                                       const VkImageSubresourceLayers *subres,
                                       VkImageAspectFlagBits aspect,
                                       uint32_t layer);
uint32_t
dzn_image_range_get_subresource_index(const struct dzn_image *image,
                                      const VkImageSubresourceRange *range,
                                      VkImageAspectFlagBits aspect,
                                      uint32_t level, uint32_t layer);

D3D12_TEXTURE_COPY_LOCATION
dzn_image_get_copy_loc(const struct dzn_image *image,
                       const VkImageSubresourceLayers *layers,
                       VkImageAspectFlagBits aspect,
                       uint32_t layer);

struct dzn_image_view {
   struct vk_image_view vk;
   D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc;
   D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc;
   D3D12_RENDER_TARGET_VIEW_DESC rtv_desc;
   D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc;
   int srv_bindless_slot;
   int uav_bindless_slot;
};

void
dzn_image_view_init(struct dzn_device *device,
                    struct dzn_image_view *iview,
                    const VkImageViewCreateInfo *info);

void
dzn_image_view_finish(struct dzn_image_view *iview);

struct dzn_buffer {
   struct vk_object_base base;

   VkDeviceSize size;

   D3D12_RESOURCE_DESC desc;
   ID3D12Resource *res;

   VkBufferCreateFlags create_flags;
   VkBufferUsageFlags usage;
   bool shared;

   D3D12_BARRIER_ACCESS valid_access;
   D3D12_GPU_VIRTUAL_ADDRESS gpuva;

   mtx_t bindless_view_lock;
   int cbv_bindless_slot;
   int uav_bindless_slot;
   struct hash_table *custom_views;
};

void
dzn_buffer_get_bindless_buffer_descriptor(struct dzn_device *device,
                                          const struct dzn_buffer_desc *bdesc,
                                          volatile struct dxil_spirv_bindless_entry *out);

DXGI_FORMAT
dzn_buffer_get_dxgi_format(VkFormat format);

D3D12_TEXTURE_COPY_LOCATION
dzn_buffer_get_copy_loc(const struct dzn_buffer *buf, VkFormat format,
                        const VkBufferImageCopy2 *info,
                        VkImageAspectFlagBits aspect,
                        uint32_t layer);

D3D12_TEXTURE_COPY_LOCATION
dzn_buffer_get_line_copy_loc(const struct dzn_buffer *buf, VkFormat format,
                             const VkBufferImageCopy2 *region,
                             const D3D12_TEXTURE_COPY_LOCATION *loc,
                             uint32_t y, uint32_t z, uint32_t *start_x);

bool
dzn_buffer_supports_region_copy(struct dzn_physical_device *pdev,
                                const D3D12_TEXTURE_COPY_LOCATION *loc);

struct dzn_buffer_view {
   struct vk_object_base base;

   const struct dzn_buffer *buffer;

   D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc;
   D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc;
   int srv_bindless_slot;
   int uav_bindless_slot;
};

struct dzn_sampler {
   struct vk_object_base base;
   D3D12_SAMPLER_DESC2 desc;
   D3D12_STATIC_BORDER_COLOR static_border_color;
   int bindless_slot;
};

/* This is defined as a macro so that it works for both
 * VkImageSubresourceRange and VkImageSubresourceLayers
 */
#define dzn_get_layer_count(_image, _range) \
   ((_range)->layerCount == VK_REMAINING_ARRAY_LAYERS ? \
    (_image)->vk.array_layers - (_range)->baseArrayLayer : (_range)->layerCount)

#define dzn_get_level_count(_image, _range) \
   ((_range)->levelCount == VK_REMAINING_MIP_LEVELS ? \
    (_image)->vk.mip_levels - (_range)->baseMipLevel : (_range)->levelCount)

DXGI_FORMAT dzn_pipe_to_dxgi_format(enum pipe_format in);
DXGI_FORMAT dzn_get_typeless_dxgi_format(DXGI_FORMAT in);
D3D12_FILTER dzn_translate_sampler_filter(const struct dzn_physical_device *pdev,
                                          const VkSamplerCreateInfo *create_info);
D3D12_COMPARISON_FUNC dzn_translate_compare_op(VkCompareOp in);
void dzn_translate_viewport(D3D12_VIEWPORT *out, const VkViewport *in);
void dzn_translate_rect(D3D12_RECT *out, const VkRect2D *in);

#define dzn_foreach_aspect(aspect, mask) \
        for (VkImageAspectFlagBits aspect = VK_IMAGE_ASPECT_COLOR_BIT; \
             aspect <= VK_IMAGE_ASPECT_STENCIL_BIT; \
             aspect = (VkImageAspectFlagBits)(aspect << 1)) \
           if (mask & aspect)

VkResult dzn_wsi_init(struct dzn_physical_device *physical_device);
void dzn_wsi_finish(struct dzn_physical_device *physical_device);

struct dzn_app_info {
   const char *app_name;
   uint32_t app_version;
   const char *engine_name;
   uint32_t engine_version;
   uint32_t api_version;
};

enum dzn_debug_flags {
   DZN_DEBUG_SYNC = 1 << 0,
   DZN_DEBUG_NIR = 1 << 1,
   DZN_DEBUG_DXIL = 1 << 2,
   DZN_DEBUG_WARP = 1 << 3,
   DZN_DEBUG_INTERNAL = 1 << 4,
   DZN_DEBUG_SIG = 1 << 5,
   DZN_DEBUG_GBV = 1 << 6,
   DZN_DEBUG_D3D12 = 1 << 7,
   DZN_DEBUG_DEBUGGER = 1 << 8,
   DZN_DEBUG_REDIRECTS = 1 << 9,
   DZN_DEBUG_BINDLESS = 1 << 10,
   DZN_DEBUG_NO_BINDLESS = 1 << 11,
};

struct dzn_instance {
   struct vk_instance vk;

   struct dxil_validator *dxil_validator;
   struct util_dl_library *d3d12_mod;
   ID3D12DeviceFactory *factory;
   struct {
      PFN_D3D12_SERIALIZE_VERSIONED_ROOT_SIGNATURE serialize_root_sig;
   } d3d12;
   uint32_t debug_flags;

   struct vk_sync_binary_type sync_binary_type;

   struct driOptionCache dri_options;
   struct driOptionCache available_dri_options;
};

struct dzn_event {
   struct vk_object_base base;
   ID3D12Fence *fence;
};

struct dzn_sync {
   struct vk_sync vk;
   ID3D12Fence *fence;
   HANDLE export_handle;
};

extern const struct vk_sync_type dzn_sync_type;

struct dzn_query {
   D3D12_QUERY_TYPE type;
   ID3D12Fence *fence;
   uint64_t fence_value;
};

struct dzn_query_pool {
   struct vk_object_base base;

   D3D12_QUERY_HEAP_TYPE heap_type;
   ID3D12QueryHeap *heap;
   uint32_t query_count;
   struct dzn_query *queries;
   mtx_t queries_lock;
   ID3D12Resource *resolve_buffer;
   ID3D12Resource *collect_buffer;
   VkQueryPipelineStatisticFlags pipeline_statistics;
   uint32_t query_size;
   uint64_t *collect_map;
};

D3D12_QUERY_TYPE
dzn_query_pool_get_query_type(const struct dzn_query_pool *qpool, VkQueryControlFlags flag);

uint32_t
dzn_query_pool_get_result_offset(const struct dzn_query_pool *qpool, uint32_t query);

uint32_t
dzn_query_pool_get_availability_offset(const struct dzn_query_pool *qpool, uint32_t query);

uint32_t
dzn_query_pool_get_result_size(const struct dzn_query_pool *qpool, uint32_t count);

VKAPI_ATTR void VKAPI_CALL
dzn_CmdPipelineBarrier2_enhanced(VkCommandBuffer commandBuffer,
                                 const VkDependencyInfo *info);

VK_DEFINE_HANDLE_CASTS(dzn_cmd_buffer, vk.base, VkCommandBuffer, VK_OBJECT_TYPE_COMMAND_BUFFER)
VK_DEFINE_HANDLE_CASTS(dzn_device, vk.base, VkDevice, VK_OBJECT_TYPE_DEVICE)
VK_DEFINE_HANDLE_CASTS(dzn_instance, vk.base, VkInstance, VK_OBJECT_TYPE_INSTANCE)
VK_DEFINE_HANDLE_CASTS(dzn_physical_device, vk.base, VkPhysicalDevice, VK_OBJECT_TYPE_PHYSICAL_DEVICE)
VK_DEFINE_HANDLE_CASTS(dzn_queue, vk.base, VkQueue, VK_OBJECT_TYPE_QUEUE)

VK_DEFINE_NONDISP_HANDLE_CASTS(dzn_buffer, base, VkBuffer, VK_OBJECT_TYPE_BUFFER)
VK_DEFINE_NONDISP_HANDLE_CASTS(dzn_buffer_view, base, VkBufferView, VK_OBJECT_TYPE_BUFFER_VIEW)
VK_DEFINE_NONDISP_HANDLE_CASTS(dzn_device_memory, base, VkDeviceMemory, VK_OBJECT_TYPE_DEVICE_MEMORY)
VK_DEFINE_NONDISP_HANDLE_CASTS(dzn_descriptor_pool, base, VkDescriptorPool, VK_OBJECT_TYPE_DESCRIPTOR_POOL)
VK_DEFINE_NONDISP_HANDLE_CASTS(dzn_descriptor_set, base, VkDescriptorSet, VK_OBJECT_TYPE_DESCRIPTOR_SET)
VK_DEFINE_NONDISP_HANDLE_CASTS(dzn_descriptor_set_layout, vk.base, VkDescriptorSetLayout, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT)
VK_DEFINE_NONDISP_HANDLE_CASTS(dzn_descriptor_update_template, base, VkDescriptorUpdateTemplate, VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE)
VK_DEFINE_NONDISP_HANDLE_CASTS(dzn_event, base, VkEvent, VK_OBJECT_TYPE_EVENT)
VK_DEFINE_NONDISP_HANDLE_CASTS(dzn_image, vk.base, VkImage, VK_OBJECT_TYPE_IMAGE)
VK_DEFINE_NONDISP_HANDLE_CASTS(dzn_image_view, vk.base, VkImageView, VK_OBJECT_TYPE_IMAGE_VIEW)
VK_DEFINE_NONDISP_HANDLE_CASTS(dzn_pipeline, base, VkPipeline, VK_OBJECT_TYPE_PIPELINE)
VK_DEFINE_NONDISP_HANDLE_CASTS(dzn_graphics_pipeline, base.base, VkPipeline, VK_OBJECT_TYPE_PIPELINE)
VK_DEFINE_NONDISP_HANDLE_CASTS(dzn_compute_pipeline, base.base, VkPipeline, VK_OBJECT_TYPE_PIPELINE)
VK_DEFINE_NONDISP_HANDLE_CASTS(dzn_pipeline_layout, vk.base, VkPipelineLayout, VK_OBJECT_TYPE_PIPELINE_LAYOUT)
VK_DEFINE_NONDISP_HANDLE_CASTS(dzn_query_pool, base, VkQueryPool, VK_OBJECT_TYPE_QUERY_POOL)
VK_DEFINE_NONDISP_HANDLE_CASTS(dzn_sampler, base, VkSampler, VK_OBJECT_TYPE_SAMPLER)

#endif /* DZN_PRIVATE_H */
