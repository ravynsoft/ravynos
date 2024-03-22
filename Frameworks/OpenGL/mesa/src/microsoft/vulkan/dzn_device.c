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

#include "dzn_private.h"

#include "vk_alloc.h"
#include "vk_common_entrypoints.h"
#include "vk_cmd_enqueue_entrypoints.h"
#include "vk_debug_report.h"
#include "vk_format.h"
#include "vk_sync_dummy.h"
#include "vk_util.h"

#include "git_sha1.h"

#include "util/u_debug.h"
#include "util/disk_cache.h"
#include "util/macros.h"
#include "util/mesa-sha1.h"
#include "util/u_dl.h"

#include "util/driconf.h"

#include "glsl_types.h"

#include "dxil_validator.h"

#include "git_sha1.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <c99_alloca.h>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#include "dzn_dxgi.h"
#endif

#include <directx/d3d12sdklayers.h>

#if defined(VK_USE_PLATFORM_WIN32_KHR) || \
    defined(VK_USE_PLATFORM_WAYLAND_KHR) || \
    defined(VK_USE_PLATFORM_XCB_KHR) || \
    defined(VK_USE_PLATFORM_XLIB_KHR)
#define DZN_USE_WSI_PLATFORM
#endif

#define DZN_API_VERSION VK_MAKE_VERSION(1, 2, VK_HEADER_VERSION)

#define MAX_TIER2_MEMORY_TYPES 4

const VkExternalMemoryHandleTypeFlags opaque_external_flag =
#ifdef _WIN32
   VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
#else
   VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
#endif

static const struct vk_instance_extension_table instance_extensions = {
   .KHR_get_physical_device_properties2      = true,
   .KHR_device_group_creation                = true,
#ifdef DZN_USE_WSI_PLATFORM
   .KHR_surface                              = true,
   .KHR_get_surface_capabilities2            = true,
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
   .KHR_win32_surface                        = true,
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
   .KHR_xcb_surface                          = true,
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
   .KHR_wayland_surface                      = true,
#endif
#ifdef VK_USE_PLATFORM_XLIB_KHR
   .KHR_xlib_surface                         = true,
#endif
   .EXT_debug_report                         = true,
   .EXT_debug_utils                          = true,
};

static void
dzn_physical_device_get_extensions(struct dzn_physical_device *pdev)
{
   pdev->vk.supported_extensions = (struct vk_device_extension_table) {
      .KHR_16bit_storage                     = pdev->options4.Native16BitShaderOpsSupported,
      .KHR_bind_memory2                      = true,
      .KHR_create_renderpass2                = true,
      .KHR_dedicated_allocation              = true,
      .KHR_depth_stencil_resolve             = true,
      .KHR_descriptor_update_template        = true,
      .KHR_device_group                      = true,
      .KHR_draw_indirect_count               = true,
      .KHR_driver_properties                 = true,
      .KHR_dynamic_rendering                 = true,
      .KHR_external_memory                   = true,
      .KHR_external_semaphore                = true,
#ifdef _WIN32
      .KHR_external_memory_win32             = true,
      .KHR_external_semaphore_win32          = true,
#else
      .KHR_external_memory_fd                = true,
      .KHR_external_semaphore_fd             = true,
#endif
      .KHR_image_format_list                 = true,
      .KHR_imageless_framebuffer             = true,
      .KHR_get_memory_requirements2          = true,
      .KHR_maintenance1                      = true,
      .KHR_maintenance2                      = true,
      .KHR_maintenance3                      = true,
      .KHR_multiview                         = true,
      .KHR_relaxed_block_layout              = true,
      .KHR_sampler_mirror_clamp_to_edge      = true,
      .KHR_separate_depth_stencil_layouts    = true,
      .KHR_shader_draw_parameters            = true,
      .KHR_shader_float16_int8               = pdev->options4.Native16BitShaderOpsSupported,
      .KHR_shader_float_controls             = true,
      .KHR_shader_integer_dot_product        = true,
      .KHR_spirv_1_4                         = true,
      .KHR_storage_buffer_storage_class      = true,
#ifdef DZN_USE_WSI_PLATFORM
      .KHR_swapchain                         = true,
#endif
      .KHR_synchronization2                  = true,
      .KHR_timeline_semaphore                = true,
      .KHR_uniform_buffer_standard_layout    = true,
      .EXT_descriptor_indexing               = pdev->shader_model >= D3D_SHADER_MODEL_6_6,
#if defined(_WIN32)
      .EXT_external_memory_host              = pdev->dev13,
#endif
      .EXT_scalar_block_layout               = true,
      .EXT_separate_stencil_usage            = true,
      .EXT_shader_subgroup_ballot            = true,
      .EXT_shader_subgroup_vote              = true,
      .EXT_subgroup_size_control             = true,
      .EXT_vertex_attribute_divisor          = true,
      .MSFT_layered_driver                   = true,
   };
}

VKAPI_ATTR VkResult VKAPI_CALL
dzn_EnumerateInstanceExtensionProperties(const char *pLayerName,
                                         uint32_t *pPropertyCount,
                                         VkExtensionProperties *pProperties)
{
   /* We don't support any layers  */
   if (pLayerName)
      return vk_error(NULL, VK_ERROR_LAYER_NOT_PRESENT);

   return vk_enumerate_instance_extension_properties(
      &instance_extensions, pPropertyCount, pProperties);
}

static const struct debug_control dzn_debug_options[] = {
   { "sync", DZN_DEBUG_SYNC },
   { "nir", DZN_DEBUG_NIR },
   { "dxil", DZN_DEBUG_DXIL },
   { "warp", DZN_DEBUG_WARP },
   { "internal", DZN_DEBUG_INTERNAL },
   { "signature", DZN_DEBUG_SIG },
   { "gbv", DZN_DEBUG_GBV },
   { "d3d12", DZN_DEBUG_D3D12 },
   { "debugger", DZN_DEBUG_DEBUGGER },
   { "redirects", DZN_DEBUG_REDIRECTS },
   { "bindless", DZN_DEBUG_BINDLESS },
   { "nobindless", DZN_DEBUG_NO_BINDLESS },
   { NULL, 0 }
};

static void
dzn_physical_device_destroy(struct vk_physical_device *physical)
{
   struct dzn_physical_device *pdev = container_of(physical, struct dzn_physical_device, vk);
   struct dzn_instance *instance = container_of(pdev->vk.instance, struct dzn_instance, vk);

   if (pdev->dev)
      ID3D12Device1_Release(pdev->dev);

   if (pdev->dev10)
      ID3D12Device1_Release(pdev->dev10);

   if (pdev->dev11)
      ID3D12Device1_Release(pdev->dev11);

   if (pdev->dev12)
      ID3D12Device1_Release(pdev->dev12);

   if (pdev->dev13)
      ID3D12Device1_Release(pdev->dev13);

   if (pdev->adapter)
      IUnknown_Release(pdev->adapter);

   dzn_wsi_finish(pdev);
   vk_physical_device_finish(&pdev->vk);
   vk_free(&instance->vk.alloc, pdev);
}

static void
dzn_instance_destroy(struct dzn_instance *instance, const VkAllocationCallbacks *alloc)
{
   if (!instance)
      return;

   vk_instance_finish(&instance->vk);

#ifdef _WIN32
   dxil_destroy_validator(instance->dxil_validator);
#endif

   if (instance->factory)
      ID3D12DeviceFactory_Release(instance->factory);

   if (instance->d3d12_mod)
      util_dl_close(instance->d3d12_mod);

   vk_free2(vk_default_allocator(), alloc, instance);
}

#ifdef _WIN32
extern IMAGE_DOS_HEADER __ImageBase;
static const char *
try_find_d3d12core_next_to_self(char *path, size_t path_arr_size)
{
   uint32_t path_size = GetModuleFileNameA((HINSTANCE)&__ImageBase,
                                           path, path_arr_size);
   if (!path_arr_size || path_size == path_arr_size) {
      mesa_loge("Unable to get path to self\n");
      return NULL;
   }

   char *last_slash = strrchr(path, '\\');
   if (!last_slash) {
      mesa_loge("Unable to get path to self\n");
      return NULL;
   }

   *(last_slash + 1) = '\0';
   if (strcat_s(path, path_arr_size, "D3D12Core.dll") != 0) {
      mesa_loge("Unable to get path to D3D12Core.dll next to self\n");
      return NULL;
   }

   if (GetFileAttributesA(path) == INVALID_FILE_ATTRIBUTES) {
      return NULL;
   }

   return path;
}
#endif

static ID3D12DeviceFactory *
try_create_device_factory(struct util_dl_library *d3d12_mod)
{
   /* A device factory allows us to isolate things like debug layer enablement from other callers,
   * and can potentially even refer to a different D3D12 redist implementation from others.
   */
   ID3D12DeviceFactory *factory = NULL;

   PFN_D3D12_GET_INTERFACE D3D12GetInterface = (PFN_D3D12_GET_INTERFACE)util_dl_get_proc_address(d3d12_mod, "D3D12GetInterface");
   if (!D3D12GetInterface) {
      mesa_loge("Failed to retrieve D3D12GetInterface\n");
      return NULL;
   }

#ifdef _WIN32
   /* First, try to create a device factory from a DLL-parallel D3D12Core.dll */
   ID3D12SDKConfiguration *sdk_config = NULL;
   if (SUCCEEDED(D3D12GetInterface(&CLSID_D3D12SDKConfiguration, &IID_ID3D12SDKConfiguration, (void **)&sdk_config))) {
      ID3D12SDKConfiguration1 *sdk_config1 = NULL;
      if (SUCCEEDED(IUnknown_QueryInterface(sdk_config, &IID_ID3D12SDKConfiguration1, (void **)&sdk_config1))) {
         char self_path[MAX_PATH];
         const char *d3d12core_path = try_find_d3d12core_next_to_self(self_path, sizeof(self_path));
         if (d3d12core_path) {
            if (SUCCEEDED(ID3D12SDKConfiguration1_CreateDeviceFactory(sdk_config1, D3D12_PREVIEW_SDK_VERSION, d3d12core_path, &IID_ID3D12DeviceFactory, (void **)&factory)) ||
                SUCCEEDED(ID3D12SDKConfiguration1_CreateDeviceFactory(sdk_config1, D3D12_SDK_VERSION, d3d12core_path, &IID_ID3D12DeviceFactory, (void **)&factory))) {
               ID3D12SDKConfiguration_Release(sdk_config);
               ID3D12SDKConfiguration1_Release(sdk_config1);
               return factory;
            }
         }

         /* Nope, seems we don't have a matching D3D12Core.dll next to ourselves */
         ID3D12SDKConfiguration1_Release(sdk_config1);
      }

      /* It's possible there's a D3D12Core.dll next to the .exe, for development/testing purposes. If so, we'll be notified
      * by environment variables what the relative path is and the version to use.
      */
      const char *d3d12core_relative_path = getenv("DZN_AGILITY_RELATIVE_PATH");
      const char *d3d12core_sdk_version = getenv("DZN_AGILITY_SDK_VERSION");
      if (d3d12core_relative_path && d3d12core_sdk_version) {
         ID3D12SDKConfiguration_SetSDKVersion(sdk_config, atoi(d3d12core_sdk_version), d3d12core_relative_path);
      }
      ID3D12SDKConfiguration_Release(sdk_config);
   }
#endif

   (void)D3D12GetInterface(&CLSID_D3D12DeviceFactory, &IID_ID3D12DeviceFactory, (void **)&factory);
   return factory;
}

VKAPI_ATTR void VKAPI_CALL
dzn_DestroyInstance(VkInstance instance,
                    const VkAllocationCallbacks *pAllocator)
{
   dzn_instance_destroy(dzn_instance_from_handle(instance), pAllocator);
}

static void
dzn_physical_device_init_uuids(struct dzn_physical_device *pdev)
{
   const char *mesa_version = "Mesa " PACKAGE_VERSION MESA_GIT_SHA1;

   struct mesa_sha1 sha1_ctx;
   uint8_t sha1[SHA1_DIGEST_LENGTH];
   STATIC_ASSERT(VK_UUID_SIZE <= sizeof(sha1));

   /* The pipeline cache UUID is used for determining when a pipeline cache is
    * invalid. Our cache is device-agnostic, but it does depend on the features
    * provided by the D3D12 driver, so let's hash the build ID plus some
    * caps that might impact our NIR lowering passes.
    */
   _mesa_sha1_init(&sha1_ctx);
   _mesa_sha1_update(&sha1_ctx,  mesa_version, strlen(mesa_version));
   disk_cache_get_function_identifier(dzn_physical_device_init_uuids, &sha1_ctx);
   _mesa_sha1_update(&sha1_ctx,  &pdev->options, sizeof(pdev->options));
   _mesa_sha1_update(&sha1_ctx,  &pdev->options2, sizeof(pdev->options2));
   _mesa_sha1_final(&sha1_ctx, sha1);
   memcpy(pdev->pipeline_cache_uuid, sha1, VK_UUID_SIZE);

   /* The driver UUID is used for determining sharability of images and memory
    * between two Vulkan instances in separate processes.  People who want to
    * share memory need to also check the device UUID (below) so all this
    * needs to be is the build-id.
    */
   _mesa_sha1_compute(mesa_version, strlen(mesa_version), sha1);
   memcpy(pdev->driver_uuid, sha1, VK_UUID_SIZE);

   /* The device UUID uniquely identifies the given device within the machine. */
   _mesa_sha1_init(&sha1_ctx);
   _mesa_sha1_update(&sha1_ctx, &pdev->desc.vendor_id, sizeof(pdev->desc.vendor_id));
   _mesa_sha1_update(&sha1_ctx, &pdev->desc.device_id, sizeof(pdev->desc.device_id));
   _mesa_sha1_update(&sha1_ctx, &pdev->desc.subsys_id, sizeof(pdev->desc.subsys_id));
   _mesa_sha1_update(&sha1_ctx, &pdev->desc.revision, sizeof(pdev->desc.revision));
   _mesa_sha1_final(&sha1_ctx, sha1);
   memcpy(pdev->device_uuid, sha1, VK_UUID_SIZE);
}

const struct vk_pipeline_cache_object_ops *const dzn_pipeline_cache_import_ops[] = {
   &dzn_cached_blob_ops,
   NULL,
};

static void
dzn_physical_device_cache_caps(struct dzn_physical_device *pdev)
{
   D3D_FEATURE_LEVEL checklist[] = {
      D3D_FEATURE_LEVEL_11_0,
      D3D_FEATURE_LEVEL_11_1,
      D3D_FEATURE_LEVEL_12_0,
      D3D_FEATURE_LEVEL_12_1,
      D3D_FEATURE_LEVEL_12_2,
   };

   D3D12_FEATURE_DATA_FEATURE_LEVELS levels = {
      .NumFeatureLevels = ARRAY_SIZE(checklist),
      .pFeatureLevelsRequested = checklist,
   };

   ID3D12Device1_CheckFeatureSupport(pdev->dev, D3D12_FEATURE_FEATURE_LEVELS, &levels, sizeof(levels));
   pdev->feature_level = levels.MaxSupportedFeatureLevel;

   static const D3D_SHADER_MODEL valid_shader_models[] = {
      D3D_SHADER_MODEL_6_7, D3D_SHADER_MODEL_6_6, D3D_SHADER_MODEL_6_5, D3D_SHADER_MODEL_6_4,
      D3D_SHADER_MODEL_6_3, D3D_SHADER_MODEL_6_2, D3D_SHADER_MODEL_6_1,
   };
   for (UINT i = 0; i < ARRAY_SIZE(valid_shader_models); ++i) {
      D3D12_FEATURE_DATA_SHADER_MODEL shader_model = { valid_shader_models[i] };
      if (SUCCEEDED(ID3D12Device1_CheckFeatureSupport(pdev->dev, D3D12_FEATURE_SHADER_MODEL, &shader_model, sizeof(shader_model)))) {
         pdev->shader_model = shader_model.HighestShaderModel;
         break;
      }
   }

   D3D_ROOT_SIGNATURE_VERSION root_sig_versions[] = {
      D3D_ROOT_SIGNATURE_VERSION_1_2,
      D3D_ROOT_SIGNATURE_VERSION_1_1
   };
   for (UINT i = 0; i < ARRAY_SIZE(root_sig_versions); ++i) {
      D3D12_FEATURE_DATA_ROOT_SIGNATURE root_sig = { root_sig_versions[i] };
      if (SUCCEEDED(ID3D12Device1_CheckFeatureSupport(pdev->dev, D3D12_FEATURE_ROOT_SIGNATURE, &root_sig, sizeof(root_sig)))) {
         pdev->root_sig_version = root_sig.HighestVersion;
         break;
      }
   }

   ID3D12Device1_CheckFeatureSupport(pdev->dev, D3D12_FEATURE_ARCHITECTURE1, &pdev->architecture, sizeof(pdev->architecture));
   ID3D12Device1_CheckFeatureSupport(pdev->dev, D3D12_FEATURE_D3D12_OPTIONS, &pdev->options, sizeof(pdev->options));
   ID3D12Device1_CheckFeatureSupport(pdev->dev, D3D12_FEATURE_D3D12_OPTIONS1, &pdev->options1, sizeof(pdev->options1));
   ID3D12Device1_CheckFeatureSupport(pdev->dev, D3D12_FEATURE_D3D12_OPTIONS2, &pdev->options2, sizeof(pdev->options2));
   ID3D12Device1_CheckFeatureSupport(pdev->dev, D3D12_FEATURE_D3D12_OPTIONS3, &pdev->options3, sizeof(pdev->options3));
   ID3D12Device1_CheckFeatureSupport(pdev->dev, D3D12_FEATURE_D3D12_OPTIONS4, &pdev->options4, sizeof(pdev->options4));
   ID3D12Device1_CheckFeatureSupport(pdev->dev, D3D12_FEATURE_D3D12_OPTIONS12, &pdev->options12, sizeof(pdev->options12));
   ID3D12Device1_CheckFeatureSupport(pdev->dev, D3D12_FEATURE_D3D12_OPTIONS13, &pdev->options13, sizeof(pdev->options13));
   ID3D12Device1_CheckFeatureSupport(pdev->dev, D3D12_FEATURE_D3D12_OPTIONS14, &pdev->options14, sizeof(pdev->options14));
   ID3D12Device1_CheckFeatureSupport(pdev->dev, D3D12_FEATURE_D3D12_OPTIONS15, &pdev->options15, sizeof(pdev->options15));
   ID3D12Device1_CheckFeatureSupport(pdev->dev, D3D12_FEATURE_D3D12_OPTIONS16, &pdev->options16, sizeof(pdev->options16));
   ID3D12Device1_CheckFeatureSupport(pdev->dev, D3D12_FEATURE_D3D12_OPTIONS17, &pdev->options17, sizeof(pdev->options17));
   if (FAILED(ID3D12Device1_CheckFeatureSupport(pdev->dev, D3D12_FEATURE_D3D12_OPTIONS19, &pdev->options19, sizeof(pdev->options19)))) {
      pdev->options19.MaxSamplerDescriptorHeapSize = D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE;
      pdev->options19.MaxSamplerDescriptorHeapSizeWithStaticSamplers = pdev->options19.MaxSamplerDescriptorHeapSize;
      pdev->options19.MaxViewDescriptorHeapSize = D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_1;
   }
   {
      D3D12_FEATURE_DATA_FORMAT_SUPPORT a4b4g4r4_support = {
         .Format = DXGI_FORMAT_A4B4G4R4_UNORM
      };
      pdev->support_a4b4g4r4 =
         SUCCEEDED(ID3D12Device1_CheckFeatureSupport(pdev->dev, D3D12_FEATURE_FORMAT_SUPPORT, &a4b4g4r4_support, sizeof(a4b4g4r4_support)));
   }

   pdev->queue_families[pdev->queue_family_count++] = (struct dzn_queue_family) {
      .props = {
         .queueFlags = VK_QUEUE_GRAPHICS_BIT |
                       VK_QUEUE_COMPUTE_BIT |
                       VK_QUEUE_TRANSFER_BIT,
         .queueCount = 4,
         .timestampValidBits = 64,
         .minImageTransferGranularity = { 0, 0, 0 },
      },
      .desc = {
         .Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
      },
   };

   pdev->queue_families[pdev->queue_family_count++] = (struct dzn_queue_family) {
      .props = {
         .queueFlags = VK_QUEUE_COMPUTE_BIT |
                       VK_QUEUE_TRANSFER_BIT,
         .queueCount = 8,
         .timestampValidBits = 64,
         .minImageTransferGranularity = { 0, 0, 0 },
      },
      .desc = {
         .Type = D3D12_COMMAND_LIST_TYPE_COMPUTE,
      },
   };

   assert(pdev->queue_family_count <= ARRAY_SIZE(pdev->queue_families));

   D3D12_COMMAND_QUEUE_DESC queue_desc = {
      .Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
      .Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
      .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
      .NodeMask = 0,
   };

   ID3D12CommandQueue *cmdqueue;
   ID3D12Device1_CreateCommandQueue(pdev->dev, &queue_desc,
                                    &IID_ID3D12CommandQueue,
                                    (void **)&cmdqueue);

   uint64_t ts_freq;
   ID3D12CommandQueue_GetTimestampFrequency(cmdqueue, &ts_freq);
   pdev->timestamp_period = 1000000000.0f / ts_freq;
   ID3D12CommandQueue_Release(cmdqueue);
}

static void
dzn_physical_device_init_memory(struct dzn_physical_device *pdev)
{
   VkPhysicalDeviceMemoryProperties *mem = &pdev->memory;

   /* For each pair of elements X and Y returned in memoryTypes, X must be placed at a lower index position than Y if:
    * - the set of bit flags returned in the propertyFlags member of X is a strict subset of the set of bit flags
    *   returned in the propertyFlags member of Y; or
    * - the propertyFlags members of X and Y are equal, and X belongs to a memory heap with greater performance
    *   (as determined in an implementation-specific manner) ; or
    * - the propertyFlags members of Y includes VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD or
    *   VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD and X does not
    * See: https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPhysicalDeviceMemoryProperties.html
   */

   mem->memoryHeapCount = 0;
   mem->memoryTypeCount = 0;

   VkMemoryPropertyFlags ram_device_local_property = 0;
   VkMemoryHeapFlags ram_device_local_heap_flag = 0;

   if (pdev->architecture.UMA) {
      /* All memory is considered device-local for UMA even though it's just RAM */
      ram_device_local_property = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
      ram_device_local_heap_flag = VK_MEMORY_HEAP_DEVICE_LOCAL_BIT;
   }

   mem->memoryHeaps[mem->memoryHeapCount++] = (VkMemoryHeap) {
      .size = pdev->desc.shared_system_memory,
      .flags = ram_device_local_heap_flag,
   };

   /* Three non-device-local memory types: host non-visible, host write-combined, and host cached */
   mem->memoryTypes[mem->memoryTypeCount++] = (VkMemoryType){
      .propertyFlags = ram_device_local_property,
      .heapIndex = mem->memoryHeapCount - 1,
   };
   mem->memoryTypes[mem->memoryTypeCount++] = (VkMemoryType){
      .propertyFlags = ram_device_local_property |
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                       VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      .heapIndex = mem->memoryHeapCount - 1,
   };
   mem->memoryTypes[mem->memoryTypeCount++] = (VkMemoryType) {
      .propertyFlags = ram_device_local_property |
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                       VK_MEMORY_PROPERTY_HOST_CACHED_BIT |
                       VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
     .heapIndex = mem->memoryHeapCount - 1,
   };

   if (!pdev->architecture.UMA) {
      /* Add a device-local memory heap/type */
      mem->memoryHeaps[mem->memoryHeapCount++] = (VkMemoryHeap){
         .size = pdev->desc.dedicated_video_memory,
         .flags = VK_MEMORY_HEAP_DEVICE_LOCAL_BIT,
      };
      mem->memoryTypes[mem->memoryTypeCount++] = (VkMemoryType){
         .propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
         .heapIndex = mem->memoryHeapCount - 1,
      };
   }

   assert(mem->memoryTypeCount <= MAX_TIER2_MEMORY_TYPES);

   if (pdev->options.ResourceHeapTier == D3D12_RESOURCE_HEAP_TIER_1) {
      unsigned oldMemoryTypeCount = mem->memoryTypeCount;
      VkMemoryType oldMemoryTypes[MAX_TIER2_MEMORY_TYPES];

      memcpy(oldMemoryTypes, mem->memoryTypes, oldMemoryTypeCount * sizeof(VkMemoryType));

      mem->memoryTypeCount = 0;
      for (unsigned oldMemoryTypeIdx = 0; oldMemoryTypeIdx < oldMemoryTypeCount; ++oldMemoryTypeIdx) {
         D3D12_HEAP_FLAGS flags[] = {
            D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS,
            D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES,
            /* Note: Vulkan requires *all* images to come from the same memory type as long as
             * the tiling property (and a few other misc properties) are the same. So, this
             * non-RT/DS texture flag will only be used for TILING_LINEAR textures, which
             * can't be render targets.
             */
            D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES
         };
         for (int i = 0; i < ARRAY_SIZE(flags); ++i) {
            D3D12_HEAP_FLAGS flag = flags[i];
            pdev->heap_flags_for_mem_type[mem->memoryTypeCount] = flag;
            mem->memoryTypes[mem->memoryTypeCount] = oldMemoryTypes[oldMemoryTypeIdx];
            mem->memoryTypeCount++;
         }
      }
   }
}

static D3D12_HEAP_FLAGS
dzn_physical_device_get_heap_flags_for_mem_type(const struct dzn_physical_device *pdev,
                                                uint32_t mem_type)
{
   return pdev->heap_flags_for_mem_type[mem_type];
}

uint32_t
dzn_physical_device_get_mem_type_mask_for_resource(const struct dzn_physical_device *pdev,
                                                   const D3D12_RESOURCE_DESC *desc,
                                                   bool shared)
{
   if (pdev->options.ResourceHeapTier > D3D12_RESOURCE_HEAP_TIER_1 && !shared)
      return (1u << pdev->memory.memoryTypeCount) - 1;

   D3D12_HEAP_FLAGS deny_flag = D3D12_HEAP_FLAG_NONE;
   if (pdev->options.ResourceHeapTier <= D3D12_RESOURCE_HEAP_TIER_1) {
      if (desc->Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
         deny_flag = D3D12_HEAP_FLAG_DENY_BUFFERS;
      else if (desc->Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))
         deny_flag = D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES;
      else
         deny_flag = D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES;
   }

   uint32_t mask = 0;
   for (unsigned i = 0; i < pdev->memory.memoryTypeCount; ++i) {
      if (shared && (pdev->memory.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
         continue;
      if ((pdev->heap_flags_for_mem_type[i] & deny_flag) == D3D12_HEAP_FLAG_NONE)
         mask |= (1 << i);
   }
   return mask;
}

static uint32_t
dzn_physical_device_get_max_mip_level(bool is_3d)
{
   return is_3d ? 11 : 14;
}

static uint32_t
dzn_physical_device_get_max_extent(bool is_3d)
{
   uint32_t max_mip = dzn_physical_device_get_max_mip_level(is_3d);

   return 1 << max_mip;
}

static uint32_t
dzn_physical_device_get_max_array_layers()
{
   return dzn_physical_device_get_max_extent(false);
}

static void
dzn_physical_device_get_features(const struct dzn_physical_device *pdev,
                                 struct vk_features *features)
{
   struct dzn_instance *instance = container_of(pdev->vk.instance, struct dzn_instance, vk);

   bool support_descriptor_indexing = pdev->shader_model >= D3D_SHADER_MODEL_6_6 &&
      !(instance->debug_flags & DZN_DEBUG_NO_BINDLESS);
   bool support_8bit = driQueryOptionb(&instance->dri_options, "dzn_enable_8bit_loads_stores") &&
      pdev->options4.Native16BitShaderOpsSupported;

   *features = (struct vk_features) {
      .robustBufferAccess = true, /* This feature is mandatory */
      .fullDrawIndexUint32 = false,
      .imageCubeArray = true,
      .independentBlend = true,
      .geometryShader = true,
      .tessellationShader = false,
      .sampleRateShading = true,
      .dualSrcBlend = false,
      .logicOp = false,
      .multiDrawIndirect = true,
      .drawIndirectFirstInstance = true,
      .depthClamp = true,
      .depthBiasClamp = true,
      .fillModeNonSolid = true,
      .depthBounds = pdev->options2.DepthBoundsTestSupported,
      .wideLines = driQueryOptionb(&instance->dri_options, "dzn_claim_wide_lines"),
      .largePoints = false,
      .alphaToOne = false,
      .multiViewport = false,
      .samplerAnisotropy = true,
      .textureCompressionETC2 = false,
      .textureCompressionASTC_LDR = false,
      .textureCompressionBC = true,
      .occlusionQueryPrecise = true,
      .pipelineStatisticsQuery = true,
      .vertexPipelineStoresAndAtomics = true,
      .fragmentStoresAndAtomics = true,
      .shaderTessellationAndGeometryPointSize = false,
      .shaderImageGatherExtended = true,
      .shaderStorageImageExtendedFormats = pdev->options.TypedUAVLoadAdditionalFormats,
      .shaderStorageImageMultisample = false,
      .shaderStorageImageReadWithoutFormat = true,
      .shaderStorageImageWriteWithoutFormat = true,
      .shaderUniformBufferArrayDynamicIndexing = true,
      .shaderSampledImageArrayDynamicIndexing = true,
      .shaderStorageBufferArrayDynamicIndexing = true,
      .shaderStorageImageArrayDynamicIndexing = true,
      .shaderClipDistance = true,
      .shaderCullDistance = true,
      .shaderFloat64 = pdev->options.DoublePrecisionFloatShaderOps,
      .shaderInt64 = pdev->options1.Int64ShaderOps,
      .shaderInt16 = pdev->options4.Native16BitShaderOpsSupported,
      .shaderResourceResidency = false,
      .shaderResourceMinLod = false,
      .sparseBinding = false,
      .sparseResidencyBuffer = false,
      .sparseResidencyImage2D = false,
      .sparseResidencyImage3D = false,
      .sparseResidency2Samples = false,
      .sparseResidency4Samples = false,
      .sparseResidency8Samples = false,
      .sparseResidency16Samples = false,
      .sparseResidencyAliased = false,
      .variableMultisampleRate = false,
      .inheritedQueries = false,

      .storageBuffer16BitAccess           = pdev->options4.Native16BitShaderOpsSupported,
      .uniformAndStorageBuffer16BitAccess = pdev->options4.Native16BitShaderOpsSupported,
      .storagePushConstant16              = false,
      .storageInputOutput16               = false,
      .multiview                          = true,
      .multiviewGeometryShader            = true,
      .multiviewTessellationShader        = false,
      .variablePointersStorageBuffer      = false,
      .variablePointers                   = false,
      .protectedMemory                    = false,
      .samplerYcbcrConversion             = false,
      .shaderDrawParameters               = true,

      .samplerMirrorClampToEdge           = true,
      .drawIndirectCount                  = true,
      .storageBuffer8BitAccess            = support_8bit,
      .uniformAndStorageBuffer8BitAccess  = support_8bit,
      .storagePushConstant8               = support_8bit,
      .shaderBufferInt64Atomics           = false,
      .shaderSharedInt64Atomics           = false,
      .shaderFloat16                      = pdev->options4.Native16BitShaderOpsSupported,
      .shaderInt8                         = support_8bit,

      .descriptorIndexing                                   = support_descriptor_indexing,
      .shaderInputAttachmentArrayDynamicIndexing            = true,
      .shaderUniformTexelBufferArrayDynamicIndexing         = true,
      .shaderStorageTexelBufferArrayDynamicIndexing         = true,
      .shaderUniformBufferArrayNonUniformIndexing           = support_descriptor_indexing,
      .shaderSampledImageArrayNonUniformIndexing            = support_descriptor_indexing,
      .shaderStorageBufferArrayNonUniformIndexing           = support_descriptor_indexing,
      .shaderStorageImageArrayNonUniformIndexing            = support_descriptor_indexing,
      .shaderInputAttachmentArrayNonUniformIndexing         = support_descriptor_indexing,
      .shaderUniformTexelBufferArrayNonUniformIndexing      = support_descriptor_indexing,
      .shaderStorageTexelBufferArrayNonUniformIndexing      = support_descriptor_indexing,
      .descriptorBindingUniformBufferUpdateAfterBind        = support_descriptor_indexing,
      .descriptorBindingSampledImageUpdateAfterBind         = support_descriptor_indexing,
      .descriptorBindingStorageImageUpdateAfterBind         = support_descriptor_indexing,
      .descriptorBindingStorageBufferUpdateAfterBind        = support_descriptor_indexing,
      .descriptorBindingUniformTexelBufferUpdateAfterBind   = support_descriptor_indexing,
      .descriptorBindingStorageTexelBufferUpdateAfterBind   = support_descriptor_indexing,
      .descriptorBindingUpdateUnusedWhilePending            = support_descriptor_indexing,
      .descriptorBindingPartiallyBound                      = support_descriptor_indexing,
      .descriptorBindingVariableDescriptorCount             = support_descriptor_indexing,
      .runtimeDescriptorArray                               = support_descriptor_indexing,

      .samplerFilterMinmax                = false,
      .scalarBlockLayout                  = true,
      .imagelessFramebuffer               = true,
      .uniformBufferStandardLayout        = true,
      .shaderSubgroupExtendedTypes        = true,
      .separateDepthStencilLayouts        = true,
      .hostQueryReset                     = true,
      .timelineSemaphore                  = true,
      .bufferDeviceAddress                = false,
      .bufferDeviceAddressCaptureReplay   = false,
      .bufferDeviceAddressMultiDevice     = false,
      .vulkanMemoryModel                  = false,
      .vulkanMemoryModelDeviceScope       = false,
      .vulkanMemoryModelAvailabilityVisibilityChains = false,
      .shaderOutputViewportIndex          = false,
      .shaderOutputLayer                  = false,
      .subgroupBroadcastDynamicId         = true,

      .robustImageAccess                  = false,
      .inlineUniformBlock                 = false,
      .descriptorBindingInlineUniformBlockUpdateAfterBind = false,
      .pipelineCreationCacheControl       = false,
      .privateData                        = true,
      .shaderDemoteToHelperInvocation     = false,
      .shaderTerminateInvocation          = false,
      .subgroupSizeControl                = pdev->options1.WaveOps && pdev->shader_model >= D3D_SHADER_MODEL_6_6,
      .computeFullSubgroups               = true,
      .synchronization2                   = true,
      .textureCompressionASTC_HDR         = false,
      .shaderZeroInitializeWorkgroupMemory = false,
      .dynamicRendering                   = true,
      .shaderIntegerDotProduct            = true,
      .maintenance4                       = false,

      .vertexAttributeInstanceRateDivisor = true,
      .vertexAttributeInstanceRateZeroDivisor = true,
   };
}

static void
dzn_physical_device_get_properties(const struct dzn_physical_device *pdev,
                                   struct vk_properties *properties)
{
   /* minimum from the D3D and Vulkan specs */
   const VkSampleCountFlags supported_sample_counts = VK_SAMPLE_COUNT_1_BIT | VK_SAMPLE_COUNT_4_BIT;

   VkPhysicalDeviceType devtype = VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
   if (pdev->desc.is_warp)
      devtype = VK_PHYSICAL_DEVICE_TYPE_CPU;
   else if (!pdev->architecture.UMA) {
      devtype = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
   }

   *properties = (struct vk_properties){
      .apiVersion = DZN_API_VERSION,
      .driverVersion = vk_get_driver_version(),

      .vendorID = pdev->desc.vendor_id,
      .deviceID = pdev->desc.device_id,
      .deviceType = devtype,

      /* Limits */
      .maxImageDimension1D = D3D12_REQ_TEXTURE1D_U_DIMENSION,
      .maxImageDimension2D = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION,
      .maxImageDimension3D = D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION,
      .maxImageDimensionCube = D3D12_REQ_TEXTURECUBE_DIMENSION,
      .maxImageArrayLayers = D3D12_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION,

      /* from here on, we simply use the minimum values from the spec for now */
      .maxTexelBufferElements = 1 << D3D12_REQ_BUFFER_RESOURCE_TEXEL_COUNT_2_TO_EXP,
      .maxUniformBufferRange = D3D12_REQ_CONSTANT_BUFFER_ELEMENT_COUNT * D3D12_STANDARD_VECTOR_SIZE * sizeof(float),
      .maxStorageBufferRange = 1 << D3D12_REQ_BUFFER_RESOURCE_TEXEL_COUNT_2_TO_EXP,
      .maxPushConstantsSize = 128,
      .maxMemoryAllocationCount = 4096,
      .maxSamplerAllocationCount = 4000,
      .bufferImageGranularity = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
      .sparseAddressSpaceSize = 0,
      .maxBoundDescriptorSets = MAX_SETS,
      .maxPerStageDescriptorSamplers =
         pdev->options.ResourceBindingTier == D3D12_RESOURCE_BINDING_TIER_1 ?
         16u : MAX_DESCS_PER_SAMPLER_HEAP,
      .maxPerStageDescriptorUniformBuffers =
         pdev->options.ResourceBindingTier <= D3D12_RESOURCE_BINDING_TIER_2 ?
         14u : MAX_DESCS_PER_CBV_SRV_UAV_HEAP,
      .maxPerStageDescriptorStorageBuffers =
         pdev->options.ResourceBindingTier <= D3D12_RESOURCE_BINDING_TIER_2 ?
         64u : MAX_DESCS_PER_CBV_SRV_UAV_HEAP,
      .maxPerStageDescriptorSampledImages =
         pdev->options.ResourceBindingTier == D3D12_RESOURCE_BINDING_TIER_1 ?
         128u : MAX_DESCS_PER_CBV_SRV_UAV_HEAP,
      .maxPerStageDescriptorStorageImages =
         pdev->options.ResourceBindingTier <= D3D12_RESOURCE_BINDING_TIER_2 ?
         64u : MAX_DESCS_PER_CBV_SRV_UAV_HEAP,
      .maxPerStageDescriptorInputAttachments =
         pdev->options.ResourceBindingTier == D3D12_RESOURCE_BINDING_TIER_1 ?
         128u : MAX_DESCS_PER_CBV_SRV_UAV_HEAP,
      .maxPerStageResources = MAX_DESCS_PER_CBV_SRV_UAV_HEAP,
      .maxDescriptorSetSamplers = MAX_DESCS_PER_SAMPLER_HEAP,
      .maxDescriptorSetUniformBuffers = MAX_DESCS_PER_CBV_SRV_UAV_HEAP,
      .maxDescriptorSetUniformBuffersDynamic = MAX_DYNAMIC_UNIFORM_BUFFERS,
      .maxDescriptorSetStorageBuffers = MAX_DESCS_PER_CBV_SRV_UAV_HEAP,
      .maxDescriptorSetStorageBuffersDynamic = MAX_DYNAMIC_STORAGE_BUFFERS,
      .maxDescriptorSetSampledImages = MAX_DESCS_PER_CBV_SRV_UAV_HEAP,
      .maxDescriptorSetStorageImages = MAX_DESCS_PER_CBV_SRV_UAV_HEAP,
      .maxDescriptorSetInputAttachments = MAX_DESCS_PER_CBV_SRV_UAV_HEAP,
      .maxVertexInputAttributes = MIN2(D3D12_STANDARD_VERTEX_ELEMENT_COUNT, MAX_VERTEX_GENERIC_ATTRIBS),
      .maxVertexInputBindings = MAX_VBS,
      .maxVertexInputAttributeOffset = D3D12_REQ_MULTI_ELEMENT_STRUCTURE_SIZE_IN_BYTES - 1,
      .maxVertexInputBindingStride = D3D12_REQ_MULTI_ELEMENT_STRUCTURE_SIZE_IN_BYTES,
      .maxVertexOutputComponents = D3D12_VS_OUTPUT_REGISTER_COUNT * D3D12_VS_OUTPUT_REGISTER_COMPONENTS,
      .maxTessellationGenerationLevel = 0,
      .maxTessellationPatchSize = 0,
      .maxTessellationControlPerVertexInputComponents = 0,
      .maxTessellationControlPerVertexOutputComponents = 0,
      .maxTessellationControlPerPatchOutputComponents = 0,
      .maxTessellationControlTotalOutputComponents = 0,
      .maxTessellationEvaluationInputComponents = 0,
      .maxTessellationEvaluationOutputComponents = 0,
      .maxGeometryShaderInvocations = D3D12_GS_MAX_INSTANCE_COUNT,
      .maxGeometryInputComponents = D3D12_GS_INPUT_REGISTER_COUNT * D3D12_GS_INPUT_REGISTER_COMPONENTS,
      .maxGeometryOutputComponents = D3D12_GS_OUTPUT_REGISTER_COUNT * D3D12_GS_OUTPUT_REGISTER_COMPONENTS,
      .maxGeometryOutputVertices = D3D12_GS_MAX_OUTPUT_VERTEX_COUNT_ACROSS_INSTANCES,
      .maxGeometryTotalOutputComponents = D3D12_REQ_GS_INVOCATION_32BIT_OUTPUT_COMPONENT_LIMIT,
      .maxFragmentInputComponents = D3D12_PS_INPUT_REGISTER_COUNT * D3D12_PS_INPUT_REGISTER_COMPONENTS,
      .maxFragmentOutputAttachments = D3D12_PS_OUTPUT_REGISTER_COUNT,
      .maxFragmentDualSrcAttachments = 0,
      .maxFragmentCombinedOutputResources = D3D12_PS_OUTPUT_REGISTER_COUNT,
      .maxComputeSharedMemorySize = D3D12_CS_TGSM_REGISTER_COUNT * sizeof(float),
      .maxComputeWorkGroupCount = { D3D12_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION,
                                                    D3D12_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION,
                                                    D3D12_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION },
      .maxComputeWorkGroupInvocations = D3D12_CS_THREAD_GROUP_MAX_THREADS_PER_GROUP,
      .maxComputeWorkGroupSize = { D3D12_CS_THREAD_GROUP_MAX_X, D3D12_CS_THREAD_GROUP_MAX_Y, D3D12_CS_THREAD_GROUP_MAX_Z },
      .subPixelPrecisionBits = D3D12_SUBPIXEL_FRACTIONAL_BIT_COUNT,
      .subTexelPrecisionBits = D3D12_SUBTEXEL_FRACTIONAL_BIT_COUNT,
      .mipmapPrecisionBits = D3D12_MIP_LOD_FRACTIONAL_BIT_COUNT,
      .maxDrawIndexedIndexValue = 0x00ffffff,
      .maxDrawIndirectCount = UINT32_MAX,
      .maxSamplerLodBias = D3D12_MIP_LOD_BIAS_MAX,
      .maxSamplerAnisotropy = D3D12_REQ_MAXANISOTROPY,
      .maxViewports = MAX_VP,
      .maxViewportDimensions = { D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION, D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION },
      .viewportBoundsRange = { D3D12_VIEWPORT_BOUNDS_MIN, D3D12_VIEWPORT_BOUNDS_MAX },
      .viewportSubPixelBits = 0,
      .minMemoryMapAlignment = 64,
      .minTexelBufferOffsetAlignment = 32,
      .minUniformBufferOffsetAlignment = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT,
      .minStorageBufferOffsetAlignment = D3D12_RAW_UAV_SRV_BYTE_ALIGNMENT,
      .minTexelOffset = D3D12_COMMONSHADER_TEXEL_OFFSET_MAX_NEGATIVE,
      .maxTexelOffset = D3D12_COMMONSHADER_TEXEL_OFFSET_MAX_POSITIVE,
      .minTexelGatherOffset = -32,
      .maxTexelGatherOffset = 31,
      .minInterpolationOffset = -0.5f,
      .maxInterpolationOffset = 0.5f,
      .subPixelInterpolationOffsetBits = 4,
      .maxFramebufferWidth = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION,
      .maxFramebufferHeight = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION,
      .maxFramebufferLayers = D3D12_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION,
      .framebufferColorSampleCounts = supported_sample_counts,
      .framebufferDepthSampleCounts = supported_sample_counts,
      .framebufferStencilSampleCounts = supported_sample_counts,
      .framebufferNoAttachmentsSampleCounts = supported_sample_counts,
      .maxColorAttachments = MAX_RTS,
      .sampledImageColorSampleCounts = supported_sample_counts,
      .sampledImageIntegerSampleCounts = VK_SAMPLE_COUNT_1_BIT,
      .sampledImageDepthSampleCounts = supported_sample_counts,
      .sampledImageStencilSampleCounts = supported_sample_counts,
      .storageImageSampleCounts = VK_SAMPLE_COUNT_1_BIT,
      .maxSampleMaskWords = 1,
      .timestampComputeAndGraphics = true,
      .timestampPeriod = pdev->timestamp_period,
      .maxClipDistances = D3D12_CLIP_OR_CULL_DISTANCE_COUNT,
      .maxCullDistances = D3D12_CLIP_OR_CULL_DISTANCE_COUNT,
      .maxCombinedClipAndCullDistances = D3D12_CLIP_OR_CULL_DISTANCE_COUNT,
      .discreteQueuePriorities = 2,
      .pointSizeRange = { 1.0f, 1.0f },
      .lineWidthRange = { 1.0f, 1.0f },
      .pointSizeGranularity = 0.0f,
      .lineWidthGranularity = 0.0f,
      .strictLines = 0,
      .standardSampleLocations = true,
      .optimalBufferCopyOffsetAlignment = D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT,
      .optimalBufferCopyRowPitchAlignment = D3D12_TEXTURE_DATA_PITCH_ALIGNMENT,
      .nonCoherentAtomSize = 256,

      /* Core 1.1 */
      .deviceLUIDValid = true,
      .pointClippingBehavior = VK_POINT_CLIPPING_BEHAVIOR_ALL_CLIP_PLANES,
      .maxMultiviewViewCount = 6,
      .maxMultiviewInstanceIndex = UINT_MAX,
      .protectedNoFault = false,
      /* Vulkan 1.1 wants this value to be at least 1024. Let's stick to this
       * minimum requirement for now, and hope the total number of samplers
       * across all descriptor sets doesn't exceed 2048, otherwise we'd exceed
       * the maximum number of samplers per heap. For any descriptor set
       * containing more than 1024 descriptors,
       * vkGetDescriptorSetLayoutSupport() can be called to determine if the
       * layout is within D3D12 descriptor heap bounds.
       */
      .maxPerSetDescriptors = 1024,
      /* According to the spec, the maximum D3D12 resource size is
       * min(max(128MB, 0.25f * (amount of dedicated VRAM)), 2GB),
       * but the limit actually depends on the max(system_ram, VRAM) not
       * just the VRAM.
       */
      .maxMemoryAllocationSize =
         CLAMP(MAX2(pdev->desc.dedicated_video_memory,
                    pdev->desc.dedicated_system_memory +
                    pdev->desc.shared_system_memory) / 4,
               128ull * 1024 * 1024, 2ull * 1024 * 1024 * 1024),
      .subgroupSupportedOperations = VK_SUBGROUP_FEATURE_BASIC_BIT |
                                     VK_SUBGROUP_FEATURE_BALLOT_BIT |
                                     VK_SUBGROUP_FEATURE_VOTE_BIT |
                                     VK_SUBGROUP_FEATURE_SHUFFLE_BIT |
                                     VK_SUBGROUP_FEATURE_SHUFFLE_RELATIVE_BIT |
                                     VK_SUBGROUP_FEATURE_QUAD_BIT |
                                     VK_SUBGROUP_FEATURE_ARITHMETIC_BIT,
      .subgroupSupportedStages = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT |
                                 VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_VERTEX_BIT,
      .subgroupQuadOperationsInAllStages = true,
      .subgroupSize = pdev->options1.WaveOps ? pdev->options1.WaveLaneCountMin : 1,
         
      /* Core 1.2 */
      .driverID = VK_DRIVER_ID_MESA_DOZEN,
      .conformanceVersion = (VkConformanceVersion){
         .major = 0,
         .minor = 0,
         .subminor = 0,
         .patch = 0,
      },
      .denormBehaviorIndependence = VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_ALL,
      .roundingModeIndependence = VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_ALL,
      .shaderSignedZeroInfNanPreserveFloat16 = false,
      .shaderSignedZeroInfNanPreserveFloat32 = false,
      .shaderSignedZeroInfNanPreserveFloat64 = false,
      .shaderDenormPreserveFloat16 = true,
      .shaderDenormPreserveFloat32 = pdev->shader_model >= D3D_SHADER_MODEL_6_2,
      .shaderDenormPreserveFloat64 = true,
      .shaderDenormFlushToZeroFloat16 = false,
      .shaderDenormFlushToZeroFloat32 = true,
      .shaderDenormFlushToZeroFloat64 = false,
      .shaderRoundingModeRTEFloat16 = true,
      .shaderRoundingModeRTEFloat32 = true,
      .shaderRoundingModeRTEFloat64 = true,
      .shaderRoundingModeRTZFloat16 = false,
      .shaderRoundingModeRTZFloat32 = false,
      .shaderRoundingModeRTZFloat64 = false,
      .shaderUniformBufferArrayNonUniformIndexingNative = true,
      .shaderSampledImageArrayNonUniformIndexingNative = true,
      .shaderStorageBufferArrayNonUniformIndexingNative = true,
      .shaderStorageImageArrayNonUniformIndexingNative = true,
      .shaderInputAttachmentArrayNonUniformIndexingNative = true,
      .robustBufferAccessUpdateAfterBind = true,
      .quadDivergentImplicitLod = false,
      .maxUpdateAfterBindDescriptorsInAllPools = MAX_DESCS_PER_CBV_SRV_UAV_HEAP,
      .maxPerStageDescriptorUpdateAfterBindSamplers = MAX_DESCS_PER_CBV_SRV_UAV_HEAP,
      .maxPerStageDescriptorUpdateAfterBindUniformBuffers = MAX_DESCS_PER_CBV_SRV_UAV_HEAP,
      .maxPerStageDescriptorUpdateAfterBindStorageBuffers = MAX_DESCS_PER_CBV_SRV_UAV_HEAP,
      .maxPerStageDescriptorUpdateAfterBindSampledImages = MAX_DESCS_PER_CBV_SRV_UAV_HEAP,
      .maxPerStageDescriptorUpdateAfterBindStorageImages = MAX_DESCS_PER_CBV_SRV_UAV_HEAP,
      .maxPerStageDescriptorUpdateAfterBindInputAttachments = MAX_DESCS_PER_CBV_SRV_UAV_HEAP,
      .maxPerStageUpdateAfterBindResources = MAX_DESCS_PER_CBV_SRV_UAV_HEAP,
      .maxDescriptorSetUpdateAfterBindSamplers = MAX_DESCS_PER_CBV_SRV_UAV_HEAP,
      .maxDescriptorSetUpdateAfterBindUniformBuffers = MAX_DESCS_PER_CBV_SRV_UAV_HEAP,
      .maxDescriptorSetUpdateAfterBindUniformBuffersDynamic = MAX_DYNAMIC_UNIFORM_BUFFERS,
      .maxDescriptorSetUpdateAfterBindStorageBuffers = MAX_DESCS_PER_CBV_SRV_UAV_HEAP,
      .maxDescriptorSetUpdateAfterBindStorageBuffersDynamic = MAX_DYNAMIC_STORAGE_BUFFERS,
      .maxDescriptorSetUpdateAfterBindSampledImages = MAX_DESCS_PER_CBV_SRV_UAV_HEAP,
      .maxDescriptorSetUpdateAfterBindStorageImages = MAX_DESCS_PER_CBV_SRV_UAV_HEAP,
      .maxDescriptorSetUpdateAfterBindInputAttachments = MAX_DESCS_PER_CBV_SRV_UAV_HEAP,

      .supportedDepthResolveModes = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT | VK_RESOLVE_MODE_AVERAGE_BIT |
         VK_RESOLVE_MODE_MIN_BIT | VK_RESOLVE_MODE_MAX_BIT,
      .supportedStencilResolveModes = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT | VK_RESOLVE_MODE_MIN_BIT | VK_RESOLVE_MODE_MAX_BIT,
      .independentResolveNone = true,
      .independentResolve = true,
      .filterMinmaxSingleComponentFormats = false,
      .filterMinmaxImageComponentMapping = false,
      .maxTimelineSemaphoreValueDifference = UINT64_MAX,
      .framebufferIntegerColorSampleCounts = VK_SAMPLE_COUNT_1_BIT,
         
      /* Core 1.3 */
      .minSubgroupSize = pdev->options1.WaveOps ? pdev->options1.WaveLaneCountMin : 1,
      .maxSubgroupSize = pdev->options1.WaveOps ? pdev->options1.WaveLaneCountMax : 1,
      .maxComputeWorkgroupSubgroups = D3D12_CS_THREAD_GROUP_MAX_THREADS_PER_GROUP /
         (pdev->options1.WaveOps ? pdev->options1.WaveLaneCountMin : 1),
      .requiredSubgroupSizeStages = VK_SHADER_STAGE_COMPUTE_BIT,
      .integerDotProduct4x8BitPackedSignedAccelerated = pdev->shader_model >= D3D_SHADER_MODEL_6_4,
      .integerDotProduct4x8BitPackedUnsignedAccelerated = pdev->shader_model >= D3D_SHADER_MODEL_6_4,
      .integerDotProductAccumulatingSaturating4x8BitPackedSignedAccelerated = pdev->shader_model >= D3D_SHADER_MODEL_6_4,
      .integerDotProductAccumulatingSaturating4x8BitPackedUnsignedAccelerated = pdev->shader_model >= D3D_SHADER_MODEL_6_4,

      /* VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT */
      .maxVertexAttribDivisor = UINT32_MAX,

      /* VkPhysicalDeviceExternalMemoryHostPropertiesEXT */
      .minImportedHostPointerAlignment = 65536,

      /* VkPhysicalDeviceLayeredDriverPropertiesMSFT */
      .underlyingAPI = VK_LAYERED_DRIVER_UNDERLYING_API_D3D12_MSFT,
   };

   snprintf(properties->deviceName,
            sizeof(properties->deviceName),
            "Microsoft Direct3D12 (%s)", pdev->desc.description);
   memcpy(properties->pipelineCacheUUID,
          pdev->pipeline_cache_uuid, VK_UUID_SIZE);
   memcpy(properties->driverUUID, pdev->driver_uuid, VK_UUID_SIZE);
   memcpy(properties->deviceUUID, pdev->device_uuid, VK_UUID_SIZE);
   memcpy(properties->deviceLUID, &pdev->desc.adapter_luid, VK_LUID_SIZE);

   STATIC_ASSERT(sizeof(pdev->desc.adapter_luid) == sizeof(properties->deviceLUID));

   snprintf(properties->driverName, VK_MAX_DRIVER_NAME_SIZE, "Dozen");
   snprintf(properties->driverInfo, VK_MAX_DRIVER_INFO_SIZE, "Mesa " PACKAGE_VERSION MESA_GIT_SHA1);
}

static VkResult
dzn_physical_device_create(struct vk_instance *instance,
                           IUnknown *adapter,
                           const struct dzn_physical_device_desc *desc)
{
   struct dzn_physical_device *pdev =
      vk_zalloc(&instance->alloc, sizeof(*pdev), 8,
                VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);

   if (!pdev)
      return vk_error(instance, VK_ERROR_OUT_OF_HOST_MEMORY);

   struct vk_physical_device_dispatch_table dispatch_table;
   vk_physical_device_dispatch_table_from_entrypoints(&dispatch_table,
                                                      &dzn_physical_device_entrypoints,
                                                      true);
   vk_physical_device_dispatch_table_from_entrypoints(&dispatch_table,
                                                      &wsi_physical_device_entrypoints,
                                                      false);

   VkResult result =
      vk_physical_device_init(&pdev->vk, instance,
                              NULL, NULL, NULL, /* We set up extensions later */
                              &dispatch_table);
   if (result != VK_SUCCESS) {
      vk_free(&instance->alloc, pdev);
      return result;
   }

   pdev->desc = *desc;
   pdev->adapter = adapter;
   IUnknown_AddRef(adapter);
   list_addtail(&pdev->vk.link, &instance->physical_devices.list);

   vk_warn_non_conformant_implementation("dzn");

   struct dzn_instance *dzn_instance = container_of(instance, struct dzn_instance, vk);

   uint32_t num_sync_types = 0;
   pdev->sync_types[num_sync_types++] = &dzn_sync_type;
   pdev->sync_types[num_sync_types++] = &dzn_instance->sync_binary_type.sync;
   pdev->sync_types[num_sync_types++] = &vk_sync_dummy_type;
   pdev->sync_types[num_sync_types] = NULL;
   assert(num_sync_types <= MAX_SYNC_TYPES);
   pdev->vk.supported_sync_types = pdev->sync_types;

   pdev->vk.pipeline_cache_import_ops = dzn_pipeline_cache_import_ops;

   pdev->dev = d3d12_create_device(dzn_instance->d3d12_mod,
                                   pdev->adapter,
                                   dzn_instance->factory,
                                   !dzn_instance->dxil_validator);
   if (!pdev->dev) {
      list_del(&pdev->vk.link);
      dzn_physical_device_destroy(&pdev->vk);
      return vk_error(instance, VK_ERROR_INITIALIZATION_FAILED);
   }

   if (FAILED(ID3D12Device1_QueryInterface(pdev->dev, &IID_ID3D12Device10, (void **)&pdev->dev10)))
      pdev->dev10 = NULL;
   if (FAILED(ID3D12Device1_QueryInterface(pdev->dev, &IID_ID3D12Device11, (void **)&pdev->dev11)))
      pdev->dev11 = NULL;
   if (FAILED(ID3D12Device1_QueryInterface(pdev->dev, &IID_ID3D12Device12, (void **)&pdev->dev12)))
      pdev->dev12 = NULL;
   if (FAILED(ID3D12Device1_QueryInterface(pdev->dev, &IID_ID3D12Device13, (void **)&pdev->dev13)))
      pdev->dev13 = NULL;
   dzn_physical_device_cache_caps(pdev);
   dzn_physical_device_init_memory(pdev);
   dzn_physical_device_init_uuids(pdev);

   dzn_physical_device_get_extensions(pdev);
   if (driQueryOptionb(&dzn_instance->dri_options, "dzn_enable_8bit_loads_stores") &&
       pdev->options4.Native16BitShaderOpsSupported)
      pdev->vk.supported_extensions.KHR_8bit_storage = true;
   if (dzn_instance->debug_flags & DZN_DEBUG_NO_BINDLESS)
      pdev->vk.supported_extensions.EXT_descriptor_indexing = false;
   dzn_physical_device_get_features(pdev, &pdev->vk.supported_features);
   dzn_physical_device_get_properties(pdev, &pdev->vk.properties);

   result = dzn_wsi_init(pdev);
   if (result != VK_SUCCESS || !pdev->dev) {
      list_del(&pdev->vk.link);
      dzn_physical_device_destroy(&pdev->vk);
      return result;
   }

   return VK_SUCCESS;
}

static DXGI_FORMAT
dzn_get_most_capable_format_for_casting(VkFormat format, VkImageCreateFlags create_flags)
{
   enum pipe_format pfmt = vk_format_to_pipe_format(format);
   bool block_compressed = util_format_is_compressed(pfmt);
   if (block_compressed &&
       !(create_flags & VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT))
      return dzn_image_get_dxgi_format(NULL, format, 0, 0);
   unsigned blksz = util_format_get_blocksize(pfmt);
   switch (blksz) {
   case 1: return DXGI_FORMAT_R8_UNORM;
   case 2: return DXGI_FORMAT_R16_UNORM;
   case 4: return DXGI_FORMAT_R32_FLOAT;
   case 8: return DXGI_FORMAT_R32G32_FLOAT;
   case 12: return DXGI_FORMAT_R32G32B32_FLOAT;
   case 16: return DXGI_FORMAT_R32G32B32A32_FLOAT;
   default: unreachable("Unsupported format bit size");;
   }
}

D3D12_FEATURE_DATA_FORMAT_SUPPORT
dzn_physical_device_get_format_support(struct dzn_physical_device *pdev,
                                       VkFormat format,
                                       VkImageCreateFlags create_flags)
{
   VkImageUsageFlags usage =
      vk_format_is_depth_or_stencil(format) ?
      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT : 0;
   VkImageAspectFlags aspects = 0;

   if (vk_format_has_depth(format))
      aspects = VK_IMAGE_ASPECT_DEPTH_BIT;
   if (vk_format_has_stencil(format))
      aspects = VK_IMAGE_ASPECT_STENCIL_BIT;

   D3D12_FEATURE_DATA_FORMAT_SUPPORT dfmt_info = {
     .Format = dzn_image_get_dxgi_format(pdev, format, usage, aspects),
   };

   /* KHR_maintenance2: If an image is created with the extended usage flag
    * (or if properties are queried with that flag), then if any compatible
    * format can support a given usage, it should be considered supported.
    * With the exception of depth, which are limited in their cast set,
    * we can do this by just picking a single most-capable format to query
    * the support for, instead of the originally requested format. */
   if (aspects == 0 && dfmt_info.Format != DXGI_FORMAT_UNKNOWN &&
       (create_flags & VK_IMAGE_CREATE_EXTENDED_USAGE_BIT)) {
      dfmt_info.Format = dzn_get_most_capable_format_for_casting(format, create_flags);
   }

   ASSERTED HRESULT hres =
      ID3D12Device1_CheckFeatureSupport(pdev->dev, D3D12_FEATURE_FORMAT_SUPPORT,
                                        &dfmt_info, sizeof(dfmt_info));
   assert(!FAILED(hres));

   if (usage != VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
      return dfmt_info;

   /* Depth/stencil resources have different format when they're accessed
    * as textures, query the capabilities for this format too.
    */
   dzn_foreach_aspect(aspect, aspects) {
      D3D12_FEATURE_DATA_FORMAT_SUPPORT dfmt_info2 = {
        .Format = dzn_image_get_dxgi_format(pdev, format, 0, aspect),
      };

      hres = ID3D12Device1_CheckFeatureSupport(pdev->dev, D3D12_FEATURE_FORMAT_SUPPORT,
                                      &dfmt_info2, sizeof(dfmt_info2));
      assert(!FAILED(hres));

#define DS_SRV_FORMAT_SUPPORT1_MASK \
        (D3D12_FORMAT_SUPPORT1_SHADER_LOAD | \
         D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE | \
         D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE_COMPARISON | \
         D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE_MONO_TEXT | \
         D3D12_FORMAT_SUPPORT1_MULTISAMPLE_RESOLVE | \
         D3D12_FORMAT_SUPPORT1_MULTISAMPLE_LOAD | \
         D3D12_FORMAT_SUPPORT1_SHADER_GATHER | \
         D3D12_FORMAT_SUPPORT1_TYPED_UNORDERED_ACCESS_VIEW | \
         D3D12_FORMAT_SUPPORT1_SHADER_GATHER_COMPARISON)

      dfmt_info.Support1 |= dfmt_info2.Support1 & DS_SRV_FORMAT_SUPPORT1_MASK;
      dfmt_info.Support2 |= dfmt_info2.Support2;
   }

   return dfmt_info;
}

static void
dzn_physical_device_get_format_properties(struct dzn_physical_device *pdev,
                                          VkFormat format,
                                          VkFormatProperties2 *properties)
{
   D3D12_FEATURE_DATA_FORMAT_SUPPORT dfmt_info =
      dzn_physical_device_get_format_support(pdev, format, 0);
   VkFormatProperties *base_props = &properties->formatProperties;

   vk_foreach_struct(ext, properties->pNext) {
      dzn_debug_ignored_stype(ext->sType);
   }

   if (dfmt_info.Format == DXGI_FORMAT_UNKNOWN) {
      if (dzn_graphics_pipeline_patch_vi_format(format) != format)
         *base_props = (VkFormatProperties){
            .bufferFeatures = VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT | VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT,
         };
      else
         *base_props = (VkFormatProperties) { 0 };
      return;
   }

   *base_props = (VkFormatProperties) {
      .linearTilingFeatures = VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
      .optimalTilingFeatures = VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
      .bufferFeatures = VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
   };

   if (dfmt_info.Support1 & D3D12_FORMAT_SUPPORT1_IA_VERTEX_BUFFER)
      base_props->bufferFeatures |= VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT;

#define TEX_FLAGS (D3D12_FORMAT_SUPPORT1_TEXTURE1D | \
                   D3D12_FORMAT_SUPPORT1_TEXTURE2D | \
                   D3D12_FORMAT_SUPPORT1_TEXTURE3D | \
                   D3D12_FORMAT_SUPPORT1_TEXTURECUBE)
   if ((dfmt_info.Support1 & TEX_FLAGS) &&
       (dfmt_info.Support1 & D3D12_FORMAT_SUPPORT1_SHADER_LOAD)) {
      base_props->optimalTilingFeatures |=
         VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT;
   }

   if (dfmt_info.Support1 & D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE) {
      base_props->optimalTilingFeatures |=
         VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
   }

   if ((dfmt_info.Support1 & D3D12_FORMAT_SUPPORT1_SHADER_LOAD) &&
       (dfmt_info.Support1 & D3D12_FORMAT_SUPPORT1_TYPED_UNORDERED_ACCESS_VIEW)) {
      base_props->optimalTilingFeatures |= VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT;
      if (dfmt_info.Support1 & D3D12_FORMAT_SUPPORT1_BUFFER)
         base_props->bufferFeatures |= VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT;
   }

#define ATOMIC_FLAGS (D3D12_FORMAT_SUPPORT2_UAV_ATOMIC_ADD | \
                      D3D12_FORMAT_SUPPORT2_UAV_ATOMIC_BITWISE_OPS | \
                      D3D12_FORMAT_SUPPORT2_UAV_ATOMIC_COMPARE_STORE_OR_COMPARE_EXCHANGE | \
                      D3D12_FORMAT_SUPPORT2_UAV_ATOMIC_EXCHANGE | \
                      D3D12_FORMAT_SUPPORT2_UAV_ATOMIC_SIGNED_MIN_OR_MAX | \
                      D3D12_FORMAT_SUPPORT2_UAV_ATOMIC_UNSIGNED_MIN_OR_MAX)
   if ((dfmt_info.Support2 & ATOMIC_FLAGS) == ATOMIC_FLAGS) {
      base_props->optimalTilingFeatures |= VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT;
      base_props->bufferFeatures |= VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT;
   }

   if (dfmt_info.Support1 & D3D12_FORMAT_SUPPORT1_BUFFER)
      base_props->bufferFeatures |= VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT;

   /* Color/depth/stencil attachment cap implies input attachement cap, and input
    * attachment loads are lowered to texture loads in dozen, hence the requirement
    * to have shader-load support.
    */
   if (dfmt_info.Support1 & D3D12_FORMAT_SUPPORT1_SHADER_LOAD) {
      if (dfmt_info.Support1 & D3D12_FORMAT_SUPPORT1_RENDER_TARGET) {
         base_props->optimalTilingFeatures |=
            VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT;
      }

      if (dfmt_info.Support1 & D3D12_FORMAT_SUPPORT1_BLENDABLE)
         base_props->optimalTilingFeatures |= VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT;

      if (dfmt_info.Support1 & D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL) {
         base_props->optimalTilingFeatures |=
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT;
      }
   }

   /* B4G4R4A4 support is required, but d3d12 doesn't support it. The needed
    * d3d12 format would be A4R4G4B4. We map this format to d3d12's B4G4R4A4,
    * which is Vulkan's A4R4G4B4, and adjust the SRV component-mapping to fake
    * B4G4R4A4, but that forces us to limit the usage to sampling, which,
    * luckily, is exactly what we need to support the required features.
    *
    * However, since this involves swizzling the alpha channel, it can cause
    * problems for border colors. Fortunately, d3d12 added an A4B4G4R4 format,
    * which still isn't quite right (it'd be Vulkan R4G4B4A4), but can be
    * swizzled by just swapping R and B, so no border color issues arise.
    */
   if (format == VK_FORMAT_B4G4R4A4_UNORM_PACK16) {
      VkFormatFeatureFlags bgra4_req_features =
         VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
         VK_FORMAT_FEATURE_TRANSFER_DST_BIT |
         VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT |
         VK_FORMAT_FEATURE_BLIT_SRC_BIT |
         VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
      base_props->optimalTilingFeatures &= bgra4_req_features;
      base_props->bufferFeatures =
         VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT;
   }

   /* depth/stencil format shouldn't advertise buffer features */
   if (vk_format_is_depth_or_stencil(format))
      base_props->bufferFeatures = 0;
}

static VkResult
dzn_physical_device_get_image_format_properties(struct dzn_physical_device *pdev,
                                                const VkPhysicalDeviceImageFormatInfo2 *info,
                                                VkImageFormatProperties2 *properties)
{
   const VkPhysicalDeviceExternalImageFormatInfo *external_info = NULL;
   VkExternalImageFormatProperties *external_props = NULL;

   properties->imageFormatProperties = (VkImageFormatProperties) { 0 };

   VkImageUsageFlags usage = info->usage;

   /* Extract input structs */
   vk_foreach_struct_const(s, info->pNext) {
      switch (s->sType) {
      case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO:
         external_info = (const VkPhysicalDeviceExternalImageFormatInfo *)s;
         break;
      case VK_STRUCTURE_TYPE_IMAGE_STENCIL_USAGE_CREATE_INFO:
         usage |= ((const VkImageStencilUsageCreateInfo *)s)->stencilUsage;
         break;
      default:
         dzn_debug_ignored_stype(s->sType);
         break;
      }
   }

   assert(info->tiling == VK_IMAGE_TILING_OPTIMAL || info->tiling == VK_IMAGE_TILING_LINEAR);

   /* Extract output structs */
   vk_foreach_struct(s, properties->pNext) {
      switch (s->sType) {
      case VK_STRUCTURE_TYPE_EXTERNAL_IMAGE_FORMAT_PROPERTIES:
         external_props = (VkExternalImageFormatProperties *)s;
         external_props->externalMemoryProperties = (VkExternalMemoryProperties) { 0 };
         break;
      default:
         dzn_debug_ignored_stype(s->sType);
         break;
      }
   }

   if (external_info && external_info->handleType != 0) {
      const VkExternalMemoryHandleTypeFlags d3d12_resource_handle_types =
         VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_RESOURCE_BIT | opaque_external_flag;
      const VkExternalMemoryHandleTypeFlags d3d11_texture_handle_types =
         VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT | d3d12_resource_handle_types;
      const VkExternalMemoryFeatureFlags import_export_feature_flags =
         VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT | VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT;
      const VkExternalMemoryFeatureFlags dedicated_feature_flags =
         VK_EXTERNAL_MEMORY_FEATURE_DEDICATED_ONLY_BIT | import_export_feature_flags;

      switch (external_info->handleType) {
      case VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT:
         external_props->externalMemoryProperties.compatibleHandleTypes = d3d11_texture_handle_types;
         external_props->externalMemoryProperties.exportFromImportedHandleTypes = d3d11_texture_handle_types;
         external_props->externalMemoryProperties.externalMemoryFeatures = dedicated_feature_flags;
         break;
      case VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_RESOURCE_BIT:
         external_props->externalMemoryProperties.compatibleHandleTypes = d3d12_resource_handle_types;
         external_props->externalMemoryProperties.exportFromImportedHandleTypes = d3d12_resource_handle_types;
         external_props->externalMemoryProperties.externalMemoryFeatures = dedicated_feature_flags;
         break;
      case VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_HEAP_BIT:
         external_props->externalMemoryProperties.compatibleHandleTypes =
            external_props->externalMemoryProperties.exportFromImportedHandleTypes =
            VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_HEAP_BIT | opaque_external_flag;
         external_props->externalMemoryProperties.externalMemoryFeatures = import_export_feature_flags;
         break;
#ifdef _WIN32
      case VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT:
#else
      case VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT:
#endif
         external_props->externalMemoryProperties.compatibleHandleTypes = d3d11_texture_handle_types;
         external_props->externalMemoryProperties.exportFromImportedHandleTypes = d3d11_texture_handle_types;
         external_props->externalMemoryProperties.externalMemoryFeatures = import_export_feature_flags;
         break;
#if defined(_WIN32)
      case VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT:
         if (pdev->dev13) {
            external_props->externalMemoryProperties.compatibleHandleTypes =
               external_props->externalMemoryProperties.exportFromImportedHandleTypes =
               VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_HEAP_BIT | opaque_external_flag;
            external_props->externalMemoryProperties.externalMemoryFeatures = import_export_feature_flags;
            break;
         }
         FALLTHROUGH;
#endif
      default:
         return VK_ERROR_FORMAT_NOT_SUPPORTED;
      }

      /* Linear textures not supported, but there's nothing else we can deduce from just a handle type */
      if (info->tiling != VK_IMAGE_TILING_OPTIMAL &&
          external_info->handleType != VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT)
         return VK_ERROR_FORMAT_NOT_SUPPORTED;
   }

   if (info->tiling != VK_IMAGE_TILING_OPTIMAL &&
       (usage & ~(VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT)))
      return VK_ERROR_FORMAT_NOT_SUPPORTED;

   if (info->tiling != VK_IMAGE_TILING_OPTIMAL &&
       vk_format_is_depth_or_stencil(info->format))
      return VK_ERROR_FORMAT_NOT_SUPPORTED;

   D3D12_FEATURE_DATA_FORMAT_SUPPORT dfmt_info =
      dzn_physical_device_get_format_support(pdev, info->format, info->flags);
   if (dfmt_info.Format == DXGI_FORMAT_UNKNOWN)
      return VK_ERROR_FORMAT_NOT_SUPPORTED;

   bool is_bgra4 = info->format == VK_FORMAT_B4G4R4A4_UNORM_PACK16 &&
      !(info->flags & VK_IMAGE_CREATE_EXTENDED_USAGE_BIT);

   if ((info->type == VK_IMAGE_TYPE_1D && !(dfmt_info.Support1 & D3D12_FORMAT_SUPPORT1_TEXTURE1D)) ||
       (info->type == VK_IMAGE_TYPE_2D && !(dfmt_info.Support1 & D3D12_FORMAT_SUPPORT1_TEXTURE2D)) ||
       (info->type == VK_IMAGE_TYPE_3D && !(dfmt_info.Support1 & D3D12_FORMAT_SUPPORT1_TEXTURE3D)) ||
       ((info->flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) &&
        !(dfmt_info.Support1 & D3D12_FORMAT_SUPPORT1_TEXTURECUBE)))
      return VK_ERROR_FORMAT_NOT_SUPPORTED;

   /* Due to extended capability querying, we might see 1D support for BC, but we don't actually have it */
   if (vk_format_is_block_compressed(info->format) && info->type == VK_IMAGE_TYPE_1D)
      return VK_ERROR_FORMAT_NOT_SUPPORTED;

   if ((usage & VK_IMAGE_USAGE_SAMPLED_BIT) &&
       /* Note: format support for SAMPLED is not necessarily accurate for integer formats */
       !(dfmt_info.Support1 & D3D12_FORMAT_SUPPORT1_SHADER_LOAD))
      return VK_ERROR_FORMAT_NOT_SUPPORTED;

   if ((usage & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT) &&
       (!(dfmt_info.Support1 & D3D12_FORMAT_SUPPORT1_SHADER_LOAD) || is_bgra4))
      return VK_ERROR_FORMAT_NOT_SUPPORTED;

   if ((usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) &&
       (!(dfmt_info.Support1 & D3D12_FORMAT_SUPPORT1_RENDER_TARGET) || is_bgra4))
      return VK_ERROR_FORMAT_NOT_SUPPORTED;

   if ((usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) &&
       (!(dfmt_info.Support1 & D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL) || is_bgra4))
      return VK_ERROR_FORMAT_NOT_SUPPORTED;

   if ((usage & VK_IMAGE_USAGE_STORAGE_BIT) &&
       (!(dfmt_info.Support1 & D3D12_FORMAT_SUPPORT1_TYPED_UNORDERED_ACCESS_VIEW) || is_bgra4))
      return VK_ERROR_FORMAT_NOT_SUPPORTED;

   if (info->type == VK_IMAGE_TYPE_3D && info->tiling != VK_IMAGE_TILING_OPTIMAL)
      return VK_ERROR_FORMAT_NOT_SUPPORTED;

   bool is_3d = info->type == VK_IMAGE_TYPE_3D;
   uint32_t max_extent = dzn_physical_device_get_max_extent(is_3d);

   if (info->tiling == VK_IMAGE_TILING_OPTIMAL &&
       dfmt_info.Support1 & D3D12_FORMAT_SUPPORT1_MIP)
      properties->imageFormatProperties.maxMipLevels = dzn_physical_device_get_max_mip_level(is_3d) + 1;
   else
      properties->imageFormatProperties.maxMipLevels = 1;

   if (info->tiling == VK_IMAGE_TILING_OPTIMAL && info->type != VK_IMAGE_TYPE_3D)
      properties->imageFormatProperties.maxArrayLayers = dzn_physical_device_get_max_array_layers();
   else
      properties->imageFormatProperties.maxArrayLayers = 1;

   switch (info->type) {
   case VK_IMAGE_TYPE_1D:
      properties->imageFormatProperties.maxExtent.width = max_extent;
      properties->imageFormatProperties.maxExtent.height = 1;
      properties->imageFormatProperties.maxExtent.depth = 1;
      break;
   case VK_IMAGE_TYPE_2D:
      properties->imageFormatProperties.maxExtent.width = max_extent;
      properties->imageFormatProperties.maxExtent.height = max_extent;
      properties->imageFormatProperties.maxExtent.depth = 1;
      break;
   case VK_IMAGE_TYPE_3D:
      properties->imageFormatProperties.maxExtent.width = max_extent;
      properties->imageFormatProperties.maxExtent.height = max_extent;
      properties->imageFormatProperties.maxExtent.depth = max_extent;
      break;
   default:
      unreachable("bad VkImageType");
   }

   /* From the Vulkan 1.0 spec, section 34.1.1. Supported Sample Counts:
    *
    * sampleCounts will be set to VK_SAMPLE_COUNT_1_BIT if at least one of the
    * following conditions is true:
    *
    *   - tiling is VK_IMAGE_TILING_LINEAR
    *   - type is not VK_IMAGE_TYPE_2D
    *   - flags contains VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT
    *   - neither the VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT flag nor the
    *     VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT flag in
    *     VkFormatProperties::optimalTilingFeatures returned by
    *     vkGetPhysicalDeviceFormatProperties is set.
    *
    * D3D12 has a few more constraints:
    *   - no UAVs on multisample resources
    */
   properties->imageFormatProperties.sampleCounts = VK_SAMPLE_COUNT_1_BIT;
   if (info->tiling != VK_IMAGE_TILING_LINEAR &&
       info->type == VK_IMAGE_TYPE_2D &&
       !(info->flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) &&
       (dfmt_info.Support1 & D3D12_FORMAT_SUPPORT1_MULTISAMPLE_LOAD) &&
       !is_bgra4 &&
       !(usage & VK_IMAGE_USAGE_STORAGE_BIT)) {
      for (uint32_t s = VK_SAMPLE_COUNT_2_BIT; s < VK_SAMPLE_COUNT_64_BIT; s <<= 1) {
         D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS ms_info = {
            .Format = dfmt_info.Format,
            .SampleCount = s,
         };

         HRESULT hres =
            ID3D12Device1_CheckFeatureSupport(pdev->dev, D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
                                     &ms_info, sizeof(ms_info));
         if (!FAILED(hres) && ms_info.NumQualityLevels > 0)
            properties->imageFormatProperties.sampleCounts |= s;
      }
   }

   /* TODO: set correct value here */
   properties->imageFormatProperties.maxResourceSize = UINT32_MAX;

   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL
dzn_GetPhysicalDeviceFormatProperties2(VkPhysicalDevice physicalDevice,
                                       VkFormat format,
                                       VkFormatProperties2 *pFormatProperties)
{
   VK_FROM_HANDLE(dzn_physical_device, pdev, physicalDevice);

   dzn_physical_device_get_format_properties(pdev, format, pFormatProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL
dzn_GetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice,
                                            const VkPhysicalDeviceImageFormatInfo2 *info,
                                            VkImageFormatProperties2 *props)
{
   VK_FROM_HANDLE(dzn_physical_device, pdev, physicalDevice);

   return dzn_physical_device_get_image_format_properties(pdev, info, props);
}

VKAPI_ATTR VkResult VKAPI_CALL
dzn_GetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice,
                                           VkFormat format,
                                           VkImageType type,
                                           VkImageTiling tiling,
                                           VkImageUsageFlags usage,
                                           VkImageCreateFlags createFlags,
                                           VkImageFormatProperties *pImageFormatProperties)
{
   const VkPhysicalDeviceImageFormatInfo2 info = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2,
      .format = format,
      .type = type,
      .tiling = tiling,
      .usage = usage,
      .flags = createFlags,
   };

   VkImageFormatProperties2 props = { 0 };

   VkResult result =
      dzn_GetPhysicalDeviceImageFormatProperties2(physicalDevice, &info, &props);
   *pImageFormatProperties = props.imageFormatProperties;

   return result;
}

VKAPI_ATTR void VKAPI_CALL
dzn_GetPhysicalDeviceSparseImageFormatProperties(VkPhysicalDevice physicalDevice,
                                                 VkFormat format,
                                                 VkImageType type,
                                                 VkSampleCountFlagBits samples,
                                                 VkImageUsageFlags usage,
                                                 VkImageTiling tiling,
                                                 uint32_t *pPropertyCount,
                                                 VkSparseImageFormatProperties *pProperties)
{
   *pPropertyCount = 0;
}

VKAPI_ATTR void VKAPI_CALL
dzn_GetPhysicalDeviceSparseImageFormatProperties2(VkPhysicalDevice physicalDevice,
                                                  const VkPhysicalDeviceSparseImageFormatInfo2 *pFormatInfo,
                                                  uint32_t *pPropertyCount,
                                                  VkSparseImageFormatProperties2 *pProperties)
{
   *pPropertyCount = 0;
}

VKAPI_ATTR void VKAPI_CALL
dzn_GetPhysicalDeviceExternalBufferProperties(VkPhysicalDevice physicalDevice,
                                              const VkPhysicalDeviceExternalBufferInfo *pExternalBufferInfo,
                                              VkExternalBufferProperties *pExternalBufferProperties)
{
#if defined(_WIN32)
   VK_FROM_HANDLE(dzn_physical_device, pdev, physicalDevice);
#endif

   const VkExternalMemoryHandleTypeFlags d3d12_resource_handle_types =
      VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_RESOURCE_BIT | opaque_external_flag;
   const VkExternalMemoryFeatureFlags import_export_feature_flags =
      VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT | VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT;
   const VkExternalMemoryFeatureFlags dedicated_feature_flags =
      VK_EXTERNAL_MEMORY_FEATURE_DEDICATED_ONLY_BIT | import_export_feature_flags;
   switch (pExternalBufferInfo->handleType) {
   case VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_RESOURCE_BIT:
      pExternalBufferProperties->externalMemoryProperties.compatibleHandleTypes = d3d12_resource_handle_types;
      pExternalBufferProperties->externalMemoryProperties.exportFromImportedHandleTypes = d3d12_resource_handle_types;
      pExternalBufferProperties->externalMemoryProperties.externalMemoryFeatures = dedicated_feature_flags;
      break;
   case VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_HEAP_BIT:
      pExternalBufferProperties->externalMemoryProperties.compatibleHandleTypes =
         pExternalBufferProperties->externalMemoryProperties.exportFromImportedHandleTypes =
         VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_HEAP_BIT | opaque_external_flag;
      pExternalBufferProperties->externalMemoryProperties.externalMemoryFeatures = import_export_feature_flags;
      break;
#ifdef _WIN32
   case VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT:
#else
   case VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT:
#endif
      pExternalBufferProperties->externalMemoryProperties.compatibleHandleTypes =
         pExternalBufferProperties->externalMemoryProperties.exportFromImportedHandleTypes =
         VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_HEAP_BIT | d3d12_resource_handle_types;
      pExternalBufferProperties->externalMemoryProperties.externalMemoryFeatures = import_export_feature_flags;
      break;
#if defined(_WIN32)
   case VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT:
      if (pdev->dev13) {
         pExternalBufferProperties->externalMemoryProperties.compatibleHandleTypes =
            pExternalBufferProperties->externalMemoryProperties.exportFromImportedHandleTypes =
            VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_HEAP_BIT | opaque_external_flag;
         pExternalBufferProperties->externalMemoryProperties.externalMemoryFeatures = import_export_feature_flags;
         break;
      }
      FALLTHROUGH;
#endif
   default:
      pExternalBufferProperties->externalMemoryProperties = (VkExternalMemoryProperties){ 0 };
      break;
   }
}

VkResult
dzn_instance_add_physical_device(struct vk_instance *instance,
                                 IUnknown *adapter,
                                 const struct dzn_physical_device_desc *desc)
{
   struct dzn_instance *dzn_instance = container_of(instance, struct dzn_instance, vk);
   if ((dzn_instance->debug_flags & DZN_DEBUG_WARP) &&
       !desc->is_warp)
      return VK_SUCCESS;

   return dzn_physical_device_create(instance, adapter, desc);
}

static VkResult
dzn_enumerate_physical_devices(struct vk_instance *instance)
{
   VkResult result = dzn_enumerate_physical_devices_dxcore(instance);
#ifdef _WIN32
   if (result != VK_SUCCESS)
      result = dzn_enumerate_physical_devices_dxgi(instance);
#endif

   return result;
}

static const driOptionDescription dzn_dri_options[] = {
   DRI_CONF_SECTION_DEBUG
      DRI_CONF_DZN_CLAIM_WIDE_LINES(false)
      DRI_CONF_DZN_ENABLE_8BIT_LOADS_STORES(false)
      DRI_CONF_VK_WSI_FORCE_SWAPCHAIN_TO_CURRENT_EXTENT(false)
   DRI_CONF_SECTION_END
};

static void
dzn_init_dri_config(struct dzn_instance *instance)
{
   driParseOptionInfo(&instance->available_dri_options, dzn_dri_options,
                      ARRAY_SIZE(dzn_dri_options));
   driParseConfigFiles(&instance->dri_options, &instance->available_dri_options, 0, "dzn", NULL, NULL,
                       instance->vk.app_info.app_name, instance->vk.app_info.app_version,
                       instance->vk.app_info.engine_name, instance->vk.app_info.engine_version);
}

static VkResult
dzn_instance_create(const VkInstanceCreateInfo *pCreateInfo,
                    const VkAllocationCallbacks *pAllocator,
                    VkInstance *out)
{
   struct dzn_instance *instance =
      vk_zalloc2(vk_default_allocator(), pAllocator, sizeof(*instance), 8,
                 VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
   if (!instance)
      return vk_error(NULL, VK_ERROR_OUT_OF_HOST_MEMORY);

   struct vk_instance_dispatch_table dispatch_table;
   vk_instance_dispatch_table_from_entrypoints(&dispatch_table,
                                               &dzn_instance_entrypoints,
                                               true);
   vk_instance_dispatch_table_from_entrypoints(&dispatch_table,
                                               &wsi_instance_entrypoints,
                                               false);

   VkResult result =
      vk_instance_init(&instance->vk, &instance_extensions,
                       &dispatch_table, pCreateInfo,
                       pAllocator ? pAllocator : vk_default_allocator());
   if (result != VK_SUCCESS) {
      vk_free2(vk_default_allocator(), pAllocator, instance);
      return result;
   }

   instance->vk.physical_devices.enumerate = dzn_enumerate_physical_devices;
   instance->vk.physical_devices.destroy = dzn_physical_device_destroy;
   instance->debug_flags =
      parse_debug_string(getenv("DZN_DEBUG"), dzn_debug_options);

#ifdef _WIN32
   if (instance->debug_flags & DZN_DEBUG_DEBUGGER) {
      /* wait for debugger to attach... */
      while (!IsDebuggerPresent()) {
         Sleep(100);
      }
   }

   if (instance->debug_flags & DZN_DEBUG_REDIRECTS) {
      char home[MAX_PATH], path[MAX_PATH];
      if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, home))) {
         snprintf(path, sizeof(path), "%s\\stderr.txt", home);
         freopen(path, "w", stderr);
         snprintf(path, sizeof(path), "%s\\stdout.txt", home);
         freopen(path, "w", stdout);
      }
   }
#endif

   bool missing_validator = false;
#ifdef _WIN32
   instance->dxil_validator = dxil_create_validator(NULL);
   missing_validator = !instance->dxil_validator;
#endif

   if (missing_validator) {
      dzn_instance_destroy(instance, pAllocator);
      return vk_error(NULL, VK_ERROR_INITIALIZATION_FAILED);
   }

   instance->d3d12_mod = util_dl_open(UTIL_DL_PREFIX "d3d12" UTIL_DL_EXT);
   if (!instance->d3d12_mod) {
      dzn_instance_destroy(instance, pAllocator);
      return vk_error(NULL, VK_ERROR_INITIALIZATION_FAILED);
   }

   instance->d3d12.serialize_root_sig = d3d12_get_serialize_root_sig(instance->d3d12_mod);
   if (!instance->d3d12.serialize_root_sig) {
      dzn_instance_destroy(instance, pAllocator);
      return vk_error(NULL, VK_ERROR_INITIALIZATION_FAILED);
   }

   instance->factory = try_create_device_factory(instance->d3d12_mod);

   if (instance->debug_flags & DZN_DEBUG_D3D12)
      d3d12_enable_debug_layer(instance->d3d12_mod, instance->factory);
   if (instance->debug_flags & DZN_DEBUG_GBV)
      d3d12_enable_gpu_validation(instance->d3d12_mod, instance->factory);

   instance->sync_binary_type = vk_sync_binary_get_type(&dzn_sync_type);
   dzn_init_dri_config(instance);

   *out = dzn_instance_to_handle(instance);
   return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL
dzn_CreateInstance(const VkInstanceCreateInfo *pCreateInfo,
                   const VkAllocationCallbacks *pAllocator,
                   VkInstance *pInstance)
{
   return dzn_instance_create(pCreateInfo, pAllocator, pInstance);
}

VKAPI_ATTR VkResult VKAPI_CALL
dzn_EnumerateInstanceVersion(uint32_t *pApiVersion)
{
   *pApiVersion = DZN_API_VERSION;
   return VK_SUCCESS;
}


VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
dzn_GetInstanceProcAddr(VkInstance _instance,
                        const char *pName)
{
   VK_FROM_HANDLE(dzn_instance, instance, _instance);
   return vk_instance_get_proc_addr(&instance->vk,
                                    &dzn_instance_entrypoints,
                                    pName);
}

/* Windows will use a dll definition file to avoid build errors. */
#ifdef _WIN32
#undef PUBLIC
#define PUBLIC
#endif

PUBLIC VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
vk_icdGetInstanceProcAddr(VkInstance instance,
                          const char *pName)
{
   return dzn_GetInstanceProcAddr(instance, pName);
}

VKAPI_ATTR void VKAPI_CALL
dzn_GetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice,
                                            uint32_t *pQueueFamilyPropertyCount,
                                            VkQueueFamilyProperties2 *pQueueFamilyProperties)
{
   VK_FROM_HANDLE(dzn_physical_device, pdev, physicalDevice);
   VK_OUTARRAY_MAKE_TYPED(VkQueueFamilyProperties2, out,
                          pQueueFamilyProperties, pQueueFamilyPropertyCount);

   for (uint32_t i = 0; i < pdev->queue_family_count; i++) {
      vk_outarray_append_typed(VkQueueFamilyProperties2, &out, p) {
         p->queueFamilyProperties = pdev->queue_families[i].props;

         vk_foreach_struct(ext, pQueueFamilyProperties->pNext) {
            dzn_debug_ignored_stype(ext->sType);
         }
      }
   }
}

VKAPI_ATTR void VKAPI_CALL
dzn_GetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice,
                                      VkPhysicalDeviceMemoryProperties *pMemoryProperties)
{
   VK_FROM_HANDLE(dzn_physical_device, pdev, physicalDevice);

   *pMemoryProperties = pdev->memory;
}

VKAPI_ATTR void VKAPI_CALL
dzn_GetPhysicalDeviceMemoryProperties2(VkPhysicalDevice physicalDevice,
                                       VkPhysicalDeviceMemoryProperties2 *pMemoryProperties)
{
   dzn_GetPhysicalDeviceMemoryProperties(physicalDevice,
                                         &pMemoryProperties->memoryProperties);

   vk_foreach_struct(ext, pMemoryProperties->pNext) {
      dzn_debug_ignored_stype(ext->sType);
   }
}

VKAPI_ATTR VkResult VKAPI_CALL
dzn_EnumerateInstanceLayerProperties(uint32_t *pPropertyCount,
                                     VkLayerProperties *pProperties)
{
   if (pProperties == NULL) {
      *pPropertyCount = 0;
      return VK_SUCCESS;
   }

   return vk_error(NULL, VK_ERROR_LAYER_NOT_PRESENT);
}

static VkResult
dzn_queue_sync_wait(struct dzn_queue *queue, const struct vk_sync_wait *wait)
{
   if (wait->sync->type == &vk_sync_dummy_type)
      return VK_SUCCESS;

   struct dzn_device *device = container_of(queue->vk.base.device, struct dzn_device, vk);
   assert(wait->sync->type == &dzn_sync_type);
   struct dzn_sync *sync = container_of(wait->sync, struct dzn_sync, vk);
   uint64_t value =
      (sync->vk.flags & VK_SYNC_IS_TIMELINE) ? wait->wait_value : 1;

   assert(sync->fence != NULL);

   if (value > 0 && FAILED(ID3D12CommandQueue_Wait(queue->cmdqueue, sync->fence, value)))
      return vk_error(device, VK_ERROR_UNKNOWN);

   return VK_SUCCESS;
}

static VkResult
dzn_queue_sync_signal(struct dzn_queue *queue, const struct vk_sync_signal *signal)
{
   if (signal->sync->type == &vk_sync_dummy_type)
      return VK_SUCCESS;

   struct dzn_device *device = container_of(queue->vk.base.device, struct dzn_device, vk);
   assert(signal->sync->type == &dzn_sync_type);
   struct dzn_sync *sync = container_of(signal->sync, struct dzn_sync, vk);
   uint64_t value =
      (sync->vk.flags & VK_SYNC_IS_TIMELINE) ? signal->signal_value : 1;
   assert(value > 0);

   assert(sync->fence != NULL);

   if (FAILED(ID3D12CommandQueue_Signal(queue->cmdqueue, sync->fence, value)))
      return vk_error(device, VK_ERROR_UNKNOWN);

   return VK_SUCCESS;
}

static VkResult
dzn_queue_submit(struct vk_queue *q,
                 struct vk_queue_submit *info)
{
   struct dzn_queue *queue = container_of(q, struct dzn_queue, vk);
   struct dzn_device *device = container_of(q->base.device, struct dzn_device, vk);
   VkResult result = VK_SUCCESS;

   for (uint32_t i = 0; i < info->wait_count; i++) {
      result = dzn_queue_sync_wait(queue, &info->waits[i]);
      if (result != VK_SUCCESS)
         return result;
   }

   ID3D12CommandList **cmdlists = alloca(info->command_buffer_count * sizeof(ID3D12CommandList*));

   for (uint32_t i = 0; i < info->command_buffer_count; i++) {
      struct dzn_cmd_buffer *cmd_buffer =
         container_of(info->command_buffers[i], struct dzn_cmd_buffer, vk);

      cmdlists[i] = (ID3D12CommandList *)cmd_buffer->cmdlist;

      util_dynarray_foreach(&cmd_buffer->queries.reset, struct dzn_cmd_buffer_query_range, range) {
         mtx_lock(&range->qpool->queries_lock);
         for (uint32_t q = range->start; q < range->start + range->count; q++) {
            struct dzn_query *query = &range->qpool->queries[q];
            if (query->fence) {
               ID3D12Fence_Release(query->fence);
               query->fence = NULL;
            }
            query->fence_value = 0;
         }
         mtx_unlock(&range->qpool->queries_lock);
      }
   }

   ID3D12CommandQueue_ExecuteCommandLists(queue->cmdqueue, info->command_buffer_count, cmdlists);

   for (uint32_t i = 0; i < info->command_buffer_count; i++) {
      struct dzn_cmd_buffer* cmd_buffer =
         container_of(info->command_buffers[i], struct dzn_cmd_buffer, vk);

      util_dynarray_foreach(&cmd_buffer->events.signal, struct dzn_cmd_event_signal, evt) {
         if (FAILED(ID3D12CommandQueue_Signal(queue->cmdqueue, evt->event->fence, evt->value ? 1 : 0)))
            return vk_error(device, VK_ERROR_UNKNOWN);
      }

      util_dynarray_foreach(&cmd_buffer->queries.signal, struct dzn_cmd_buffer_query_range, range) {
         mtx_lock(&range->qpool->queries_lock);
         for (uint32_t q = range->start; q < range->start + range->count; q++) {
            struct dzn_query *query = &range->qpool->queries[q];
            query->fence_value = queue->fence_point + 1;
            query->fence = queue->fence;
            ID3D12Fence_AddRef(query->fence);
         }
         mtx_unlock(&range->qpool->queries_lock);
      }
   }

   for (uint32_t i = 0; i < info->signal_count; i++) {
      result = dzn_queue_sync_signal(queue, &info->signals[i]);
      if (result != VK_SUCCESS)
         return vk_error(device, VK_ERROR_UNKNOWN);
   }

   if (FAILED(ID3D12CommandQueue_Signal(queue->cmdqueue, queue->fence, ++queue->fence_point)))
      return vk_error(device, VK_ERROR_UNKNOWN);

   return VK_SUCCESS;
}

static void
dzn_queue_finish(struct dzn_queue *queue)
{
   if (queue->cmdqueue)
      ID3D12CommandQueue_Release(queue->cmdqueue);

   if (queue->fence)
      ID3D12Fence_Release(queue->fence);

   vk_queue_finish(&queue->vk);
}

static VkResult
dzn_queue_init(struct dzn_queue *queue,
               struct dzn_device *device,
               const VkDeviceQueueCreateInfo *pCreateInfo,
               uint32_t index_in_family)
{
   struct dzn_physical_device *pdev = container_of(device->vk.physical, struct dzn_physical_device, vk);

   VkResult result = vk_queue_init(&queue->vk, &device->vk, pCreateInfo, index_in_family);
   if (result != VK_SUCCESS)
      return result;

   queue->vk.driver_submit = dzn_queue_submit;

   assert(pCreateInfo->queueFamilyIndex < pdev->queue_family_count);

   D3D12_COMMAND_QUEUE_DESC queue_desc =
      pdev->queue_families[pCreateInfo->queueFamilyIndex].desc;

   float priority_in = pCreateInfo->pQueuePriorities[index_in_family];
   queue_desc.Priority =
      priority_in > 0.5f ? D3D12_COMMAND_QUEUE_PRIORITY_HIGH : D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
   queue_desc.NodeMask = 0;

   if (FAILED(ID3D12Device1_CreateCommandQueue(device->dev, &queue_desc,
                                               &IID_ID3D12CommandQueue,
                                               (void **)&queue->cmdqueue))) {
      dzn_queue_finish(queue);
      return vk_error(device->vk.physical->instance, VK_ERROR_INITIALIZATION_FAILED);
   }

   if (FAILED(ID3D12Device1_CreateFence(device->dev, 0, D3D12_FENCE_FLAG_NONE,
                                        &IID_ID3D12Fence,
                                        (void **)&queue->fence))) {
      dzn_queue_finish(queue);
      return vk_error(device->vk.physical->instance, VK_ERROR_INITIALIZATION_FAILED);
   }

   return VK_SUCCESS;
}

static VkResult
dzn_device_create_sync_for_memory(struct vk_device *device,
                                  VkDeviceMemory memory,
                                  bool signal_memory,
                                  struct vk_sync **sync_out)
{
   return vk_sync_create(device, &vk_sync_dummy_type,
                         0, 1, sync_out);
}

static VkResult
dzn_device_query_init(struct dzn_device *device)
{
   /* FIXME: create the resource in the default heap */
   D3D12_HEAP_PROPERTIES hprops = dzn_ID3D12Device4_GetCustomHeapProperties(device->dev, 0, D3D12_HEAP_TYPE_UPLOAD);
   D3D12_RESOURCE_DESC rdesc = {
      .Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
      .Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
      .Width = DZN_QUERY_REFS_RES_SIZE,
      .Height = 1,
      .DepthOrArraySize = 1,
      .MipLevels = 1,
      .Format = DXGI_FORMAT_UNKNOWN,
      .SampleDesc = { .Count = 1, .Quality = 0 },
      .Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
      .Flags = D3D12_RESOURCE_FLAG_NONE,
   };

   if (FAILED(ID3D12Device1_CreateCommittedResource(device->dev, &hprops,
                                                   D3D12_HEAP_FLAG_NONE,
                                                   &rdesc,
                                                   D3D12_RESOURCE_STATE_COMMON,
                                                   NULL,
                                                   &IID_ID3D12Resource,
                                                   (void **)&device->queries.refs)))
      return vk_error(device->vk.physical, VK_ERROR_OUT_OF_DEVICE_MEMORY);

   uint8_t *queries_ref;
   if (FAILED(ID3D12Resource_Map(device->queries.refs, 0, NULL, (void **)&queries_ref)))
      return vk_error(device->vk.physical, VK_ERROR_OUT_OF_HOST_MEMORY);

   memset(queries_ref + DZN_QUERY_REFS_ALL_ONES_OFFSET, 0xff, DZN_QUERY_REFS_SECTION_SIZE);
   memset(queries_ref + DZN_QUERY_REFS_ALL_ZEROS_OFFSET, 0x0, DZN_QUERY_REFS_SECTION_SIZE);
   ID3D12Resource_Unmap(device->queries.refs, 0, NULL);

   return VK_SUCCESS;
}

static void
dzn_device_query_finish(struct dzn_device *device)
{
   if (device->queries.refs)
      ID3D12Resource_Release(device->queries.refs);
}

static void
dzn_device_destroy(struct dzn_device *device, const VkAllocationCallbacks *pAllocator)
{
   if (!device)
      return;

   struct dzn_instance *instance =
      container_of(device->vk.physical->instance, struct dzn_instance, vk);

   vk_foreach_queue_safe(q, &device->vk) {
      struct dzn_queue *queue = container_of(q, struct dzn_queue, vk);

      dzn_queue_finish(queue);
   }

   dzn_device_query_finish(device);
   dzn_meta_finish(device);

   dzn_foreach_pool_type(type) {
      dzn_descriptor_heap_finish(&device->device_heaps[type].heap);
      util_dynarray_fini(&device->device_heaps[type].slot_freelist);
      mtx_destroy(&device->device_heaps[type].lock);
   }

   if (device->dev_config)
      ID3D12DeviceConfiguration_Release(device->dev_config);

   if (device->dev)
      ID3D12Device1_Release(device->dev);

   if (device->dev10)
      ID3D12Device1_Release(device->dev10);

   if (device->dev11)
      ID3D12Device1_Release(device->dev11);

   if (device->dev12)
      ID3D12Device1_Release(device->dev12);

   if (device->dev13)
      ID3D12Device1_Release(device->dev13);

   vk_device_finish(&device->vk);
   vk_free2(&instance->vk.alloc, pAllocator, device);
}

static VkResult
dzn_device_check_status(struct vk_device *dev)
{
   struct dzn_device *device = container_of(dev, struct dzn_device, vk);

   if (FAILED(ID3D12Device_GetDeviceRemovedReason(device->dev)))
      return vk_device_set_lost(&device->vk, "D3D12 device removed");

   return VK_SUCCESS;
}

static VkResult
dzn_device_create(struct dzn_physical_device *pdev,
                  const VkDeviceCreateInfo *pCreateInfo,
                  const VkAllocationCallbacks *pAllocator,
                  VkDevice *out)
{
   struct dzn_instance *instance = container_of(pdev->vk.instance, struct dzn_instance, vk);

   uint32_t graphics_queue_count = 0;
   uint32_t queue_count = 0;
   for (uint32_t qf = 0; qf < pCreateInfo->queueCreateInfoCount; qf++) {
      const VkDeviceQueueCreateInfo *qinfo = &pCreateInfo->pQueueCreateInfos[qf];
      queue_count += qinfo->queueCount;
      if (pdev->queue_families[qinfo->queueFamilyIndex].props.queueFlags & VK_QUEUE_GRAPHICS_BIT)
         graphics_queue_count += qinfo->queueCount;
   }

   /* Add a swapchain queue if there's no or too many graphics queues */
   if (graphics_queue_count != 1)
      queue_count++;

   VK_MULTIALLOC(ma);
   VK_MULTIALLOC_DECL(&ma, struct dzn_device, device, 1);
   VK_MULTIALLOC_DECL(&ma, struct dzn_queue, queues, queue_count);

   if (!vk_multialloc_zalloc2(&ma, &instance->vk.alloc, pAllocator,
                              VK_SYSTEM_ALLOCATION_SCOPE_DEVICE))
      return vk_error(pdev, VK_ERROR_OUT_OF_HOST_MEMORY);

   struct vk_device_dispatch_table dispatch_table;

   /* For secondary command buffer support, overwrite any command entrypoints
    * in the main device-level dispatch table with
    * vk_cmd_enqueue_unless_primary_Cmd*.
    */
   vk_device_dispatch_table_from_entrypoints(&dispatch_table,
      &vk_cmd_enqueue_unless_primary_device_entrypoints, true);
   vk_device_dispatch_table_from_entrypoints(&dispatch_table,
      &dzn_device_entrypoints, false);
   vk_device_dispatch_table_from_entrypoints(&dispatch_table,
      &wsi_device_entrypoints, false);

   /* Populate our primary cmd_dispatch table. */
   vk_device_dispatch_table_from_entrypoints(&device->cmd_dispatch,
      &dzn_device_entrypoints, true);
   vk_device_dispatch_table_from_entrypoints(&device->cmd_dispatch,
                                             &vk_common_device_entrypoints,
                                             false);

   /* Override entrypoints with alternatives based on supported features. */
   if (pdev->options12.EnhancedBarriersSupported) {
      device->cmd_dispatch.CmdPipelineBarrier2 = dzn_CmdPipelineBarrier2_enhanced;
   }

   VkResult result =
      vk_device_init(&device->vk, &pdev->vk, &dispatch_table, pCreateInfo, pAllocator);
   if (result != VK_SUCCESS) {
      vk_free2(&device->vk.alloc, pAllocator, device);
      return result;
   }

   /* Must be done after vk_device_init() because this function memset(0) the
    * whole struct.
    */
   device->vk.command_dispatch_table = &device->cmd_dispatch;
   device->vk.create_sync_for_memory = dzn_device_create_sync_for_memory;
   device->vk.check_status = dzn_device_check_status;

   device->dev = pdev->dev;

   ID3D12Device1_AddRef(device->dev);

   if (pdev->dev10) {
      device->dev10 = pdev->dev10;
      ID3D12Device1_AddRef(device->dev10);
   }
   if (pdev->dev11) {
      device->dev11 = pdev->dev11;
      ID3D12Device1_AddRef(device->dev11);
   }

   if (pdev->dev12) {
      device->dev12 = pdev->dev12;
      ID3D12Device1_AddRef(device->dev12);
   }

   if (pdev->dev13) {
      device->dev13 = pdev->dev13;
      ID3D12Device1_AddRef(device->dev13);
   }

   ID3D12InfoQueue *info_queue;
   if (SUCCEEDED(ID3D12Device1_QueryInterface(device->dev,
                                              &IID_ID3D12InfoQueue,
                                              (void **)&info_queue))) {
      D3D12_MESSAGE_SEVERITY severities[] = {
         D3D12_MESSAGE_SEVERITY_INFO,
         D3D12_MESSAGE_SEVERITY_WARNING,
      };

      D3D12_MESSAGE_ID msg_ids[] = {
         D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
      };

      D3D12_INFO_QUEUE_FILTER NewFilter = { 0 };
      NewFilter.DenyList.NumSeverities = ARRAY_SIZE(severities);
      NewFilter.DenyList.pSeverityList = severities;
      NewFilter.DenyList.NumIDs = ARRAY_SIZE(msg_ids);
      NewFilter.DenyList.pIDList = msg_ids;

      ID3D12InfoQueue_PushStorageFilter(info_queue, &NewFilter);
      ID3D12InfoQueue_Release(info_queue);
   }

   IUnknown_QueryInterface(device->dev, &IID_ID3D12DeviceConfiguration, (void **)&device->dev_config);

   result = dzn_meta_init(device);
   if (result != VK_SUCCESS) {
      dzn_device_destroy(device, pAllocator);
      return result;
   }

   result = dzn_device_query_init(device);
   if (result != VK_SUCCESS) {
      dzn_device_destroy(device, pAllocator);
      return result;
   }

   uint32_t qindex = 0;
   for (uint32_t qf = 0; qf < pCreateInfo->queueCreateInfoCount; qf++) {
      const VkDeviceQueueCreateInfo *qinfo = &pCreateInfo->pQueueCreateInfos[qf];

      for (uint32_t q = 0; q < qinfo->queueCount; q++) {
         result =
            dzn_queue_init(&queues[qindex++], device, qinfo, q);
         if (result != VK_SUCCESS) {
            dzn_device_destroy(device, pAllocator);
            return result;
         }
         if (graphics_queue_count == 1 &&
             pdev->queue_families[qinfo->queueFamilyIndex].props.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            device->swapchain_queue = &queues[qindex - 1];
      }
   }

   if (!device->swapchain_queue) {
      const float swapchain_queue_priority = 0.0f;
      VkDeviceQueueCreateInfo swapchain_queue_info = {
         .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
         .flags = 0,
         .queueCount = 1,
         .pQueuePriorities = &swapchain_queue_priority,
      };
      for (uint32_t qf = 0; qf < pdev->queue_family_count; qf++) {
         if (pdev->queue_families[qf].props.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            swapchain_queue_info.queueFamilyIndex = qf;
            break;
         }
      }
      result = dzn_queue_init(&queues[qindex], device, &swapchain_queue_info, 0);
      if (result != VK_SUCCESS) {
         dzn_device_destroy(device, pAllocator);
         return result;
      }
      device->swapchain_queue = &queues[qindex++];
      device->need_swapchain_blits = true;
   }

   device->support_static_samplers = true;
   device->bindless = (instance->debug_flags & DZN_DEBUG_BINDLESS) != 0 ||
      device->vk.enabled_features.descriptorIndexing ||
      device->vk.enabled_extensions.EXT_descriptor_indexing;

   if (device->bindless) {
      uint32_t sampler_count = MIN2(pdev->options19.MaxSamplerDescriptorHeapSize, 4000);
      device->support_static_samplers = pdev->options19.MaxSamplerDescriptorHeapSizeWithStaticSamplers >= sampler_count;
      dzn_foreach_pool_type(type) {
         uint32_t descriptor_count = type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER ?
            sampler_count : D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_1;
         result = dzn_descriptor_heap_init(&device->device_heaps[type].heap, device, type, descriptor_count, true);
         if (result != VK_SUCCESS) {
            dzn_device_destroy(device, pAllocator);
            return result;
         }

         mtx_init(&device->device_heaps[type].lock, mtx_plain);
         util_dynarray_init(&device->device_heaps[type].slot_freelist, NULL);
         device->device_heaps[type].next_alloc_slot = 0;
      }
   }

   assert(queue_count == qindex);
   *out = dzn_device_to_handle(device);
   return VK_SUCCESS;
}

static ID3DBlob *
serialize_root_sig(struct dzn_device *device,
                   const D3D12_VERSIONED_ROOT_SIGNATURE_DESC *desc)
{
   struct dzn_instance *instance =
      container_of(device->vk.physical->instance, struct dzn_instance, vk);
   ID3DBlob *sig = NULL, *error = NULL;

   HRESULT hr = device->dev_config ?
         ID3D12DeviceConfiguration_SerializeVersionedRootSignature(device->dev_config, desc, &sig, &error) :
         instance->d3d12.serialize_root_sig(desc, &sig, &error);

   if (FAILED(hr)) {
      if (instance->debug_flags & DZN_DEBUG_SIG) {
         const char *error_msg = (const char *)ID3D10Blob_GetBufferPointer(error);
         fprintf(stderr,
                 "== SERIALIZE ROOT SIG ERROR =============================================\n"
                 "%s\n"
                 "== END ==========================================================\n",
                 error_msg);
      }
   }

   if (error)
      ID3D10Blob_Release(error);

   return sig;
}

ID3D12RootSignature *
dzn_device_create_root_sig(struct dzn_device *device,
                           const D3D12_VERSIONED_ROOT_SIGNATURE_DESC *desc)
{
   ID3DBlob *sig = serialize_root_sig(device, desc);
   if (!sig)
      return NULL;

   ID3D12RootSignature *root_sig = NULL;
   ID3D12Device1_CreateRootSignature(device->dev, 0,
                                     ID3D10Blob_GetBufferPointer(sig),
                                     ID3D10Blob_GetBufferSize(sig),
                                     &IID_ID3D12RootSignature,
                                     (void **)&root_sig);
   ID3D10Blob_Release(sig);
   return root_sig;
}

VKAPI_ATTR VkResult VKAPI_CALL
dzn_CreateDevice(VkPhysicalDevice physicalDevice,
                 const VkDeviceCreateInfo *pCreateInfo,
                 const VkAllocationCallbacks *pAllocator,
                 VkDevice *pDevice)
{
   VK_FROM_HANDLE(dzn_physical_device, physical_device, physicalDevice);
   VkResult result;

   assert(pCreateInfo->sType == VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO);

   /* Check enabled features */
   if (pCreateInfo->pEnabledFeatures) {
      result = vk_physical_device_check_device_features(&physical_device->vk, pCreateInfo);
      if (result != VK_SUCCESS)
         return vk_error(physical_device, result);
   }

   /* Check requested queues and fail if we are requested to create any
    * queues with flags we don't support.
    */
   assert(pCreateInfo->queueCreateInfoCount > 0);
   for (uint32_t i = 0; i < pCreateInfo->queueCreateInfoCount; i++) {
      if (pCreateInfo->pQueueCreateInfos[i].flags != 0)
         return vk_error(physical_device, VK_ERROR_INITIALIZATION_FAILED);
   }

   return dzn_device_create(physical_device, pCreateInfo, pAllocator, pDevice);
}

VKAPI_ATTR void VKAPI_CALL
dzn_DestroyDevice(VkDevice dev,
                  const VkAllocationCallbacks *pAllocator)
{
   VK_FROM_HANDLE(dzn_device, device, dev);

   device->vk.dispatch_table.DeviceWaitIdle(dev);

   dzn_device_destroy(device, pAllocator);
}

static void
dzn_device_memory_destroy(struct dzn_device_memory *mem,
                          const VkAllocationCallbacks *pAllocator)
{
   if (!mem)
      return;

   struct dzn_device *device = container_of(mem->base.device, struct dzn_device, vk);

   if (mem->map && mem->map_res)
      ID3D12Resource_Unmap(mem->map_res, 0, NULL);

   if (mem->map_res)
      ID3D12Resource_Release(mem->map_res);

   if (mem->heap)
      ID3D12Heap_Release(mem->heap);

   if (mem->dedicated_res)
      ID3D12Resource_Release(mem->dedicated_res);

#ifdef _WIN32
   if (mem->export_handle)
      CloseHandle(mem->export_handle);
#else
   if ((intptr_t)mem->export_handle >= 0)
      close((int)(intptr_t)mem->export_handle);
#endif

   vk_object_base_finish(&mem->base);
   vk_free2(&device->vk.alloc, pAllocator, mem);
}

static D3D12_HEAP_PROPERTIES
deduce_heap_properties_from_memory(struct dzn_physical_device *pdevice,
                                   const VkMemoryType *mem_type)
{
   D3D12_HEAP_PROPERTIES properties = { .Type = D3D12_HEAP_TYPE_CUSTOM };
   properties.MemoryPoolPreference =
      ((mem_type->propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) &&
       !pdevice->architecture.UMA) ?
      D3D12_MEMORY_POOL_L1 : D3D12_MEMORY_POOL_L0;
   if ((mem_type->propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) ||
       ((mem_type->propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) && pdevice->architecture.CacheCoherentUMA)) {
      properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
   } else if (mem_type->propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
      properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE;
   } else {
      properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE;
   }
   return properties;
}

static VkResult
dzn_device_memory_create(struct dzn_device *device,
                         const VkMemoryAllocateInfo *pAllocateInfo,
                         const VkAllocationCallbacks *pAllocator,
                         VkDeviceMemory *out)
{
   struct dzn_physical_device *pdevice =
      container_of(device->vk.physical, struct dzn_physical_device, vk);

   const struct dzn_buffer *buffer = NULL;
   const struct dzn_image *image = NULL;

   VkExternalMemoryHandleTypeFlags export_flags = 0;
   HANDLE import_handle = NULL;
   bool imported_from_d3d11 = false;
   void *host_pointer = NULL;
#ifdef _WIN32
   const wchar_t *import_name = NULL;
   const VkExportMemoryWin32HandleInfoKHR *win32_export = NULL;
#endif
   vk_foreach_struct_const(ext, pAllocateInfo->pNext) {
      switch (ext->sType) {
      case VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO: {
         const VkExportMemoryAllocateInfo *exp =
            (const VkExportMemoryAllocateInfo *)ext;

         export_flags = exp->handleTypes;
         break;
      }
#ifdef _WIN32
      case VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR: {
         const VkImportMemoryWin32HandleInfoKHR *imp =
            (const VkImportMemoryWin32HandleInfoKHR *)ext;
         switch (imp->handleType) {
         case VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT:
            imported_from_d3d11 = true;
            FALLTHROUGH;
         case VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT:
         case VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_RESOURCE_BIT:
         case VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_HEAP_BIT:
            break;
         default:
            return vk_error(device, VK_ERROR_INVALID_EXTERNAL_HANDLE);
         }
         import_handle = imp->handle;
         import_name = imp->name;
         break;
      }
      case VK_STRUCTURE_TYPE_EXPORT_MEMORY_WIN32_HANDLE_INFO_KHR:
         win32_export = (const VkExportMemoryWin32HandleInfoKHR *)ext;
         break;
      case VK_STRUCTURE_TYPE_IMPORT_MEMORY_HOST_POINTER_INFO_EXT: {
         const VkImportMemoryHostPointerInfoEXT *imp =
            (const VkImportMemoryHostPointerInfoEXT *)ext;
         host_pointer = imp->pHostPointer;
         break;
      }
#else
      case VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR: {
         const VkImportMemoryFdInfoKHR *imp =
            (const VkImportMemoryFdInfoKHR *)ext;
         switch (imp->handleType) {
         case VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT:
         case VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_RESOURCE_BIT:
         case VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_HEAP_BIT:
            break;
         default:
            return vk_error(device, VK_ERROR_INVALID_EXTERNAL_HANDLE);
         }
         import_handle = (HANDLE)(intptr_t)imp->fd;
         break;
      }
#endif
      case VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO: {
         const VkMemoryDedicatedAllocateInfo *dedicated =
           (const VkMemoryDedicatedAllocateInfo *)ext;

         buffer = dzn_buffer_from_handle(dedicated->buffer);
         image = dzn_image_from_handle(dedicated->image);
         assert(!buffer || !image);
         break;
      }
      default:
         dzn_debug_ignored_stype(ext->sType);
         break;
      }
   }

   const VkMemoryType *mem_type =
      &pdevice->memory.memoryTypes[pAllocateInfo->memoryTypeIndex];

   D3D12_HEAP_DESC heap_desc = { 0 };

   heap_desc.SizeInBytes = pAllocateInfo->allocationSize;
   if (buffer) {
      heap_desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
   } else if (image) {
      heap_desc.Alignment =
         image->vk.samples > 1 ?
         D3D12_DEFAULT_MSAA_RESOURCE_PLACEMENT_ALIGNMENT :
         D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
   } else {
      heap_desc.Alignment =
         heap_desc.SizeInBytes >= D3D12_DEFAULT_MSAA_RESOURCE_PLACEMENT_ALIGNMENT ?
         D3D12_DEFAULT_MSAA_RESOURCE_PLACEMENT_ALIGNMENT :
         D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
   }

   if (mem_type->propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
      image = NULL;

   VkExternalMemoryHandleTypeFlags valid_flags =
      opaque_external_flag |
      (buffer || image ?
       VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_RESOURCE_BIT :
       VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_HEAP_BIT);
   if (image && imported_from_d3d11)
      valid_flags |= VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT;

   if (export_flags & ~valid_flags)
      return vk_error(device, VK_ERROR_INVALID_EXTERNAL_HANDLE);

   struct dzn_device_memory *mem =
      vk_zalloc2(&device->vk.alloc, pAllocator, sizeof(*mem), 8,
                 VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (!mem)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   vk_object_base_init(&device->vk, &mem->base, VK_OBJECT_TYPE_DEVICE_MEMORY);
#ifndef _WIN32
   mem->export_handle = (HANDLE)(intptr_t)-1;
#endif

   /* The Vulkan 1.0.33 spec says "allocationSize must be greater than 0". */
   assert(pAllocateInfo->allocationSize > 0);

   mem->size = pAllocateInfo->allocationSize;

   heap_desc.SizeInBytes = ALIGN_POT(heap_desc.SizeInBytes, heap_desc.Alignment);
   if (!image && !buffer)
      heap_desc.Flags =
         dzn_physical_device_get_heap_flags_for_mem_type(pdevice, pAllocateInfo->memoryTypeIndex);
   heap_desc.Properties = deduce_heap_properties_from_memory(pdevice, mem_type);
   if (export_flags) {
      heap_desc.Flags |= D3D12_HEAP_FLAG_SHARED;
      assert(host_pointer || heap_desc.Properties.CPUPageProperty == D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE);
   }

   VkResult error = VK_ERROR_OUT_OF_DEVICE_MEMORY;

#ifdef _WIN32
   HANDLE handle_from_name = NULL;
   if (import_name) {
      if (FAILED(ID3D12Device_OpenSharedHandleByName(device->dev, import_name, GENERIC_ALL, &handle_from_name))) {
         error = VK_ERROR_INVALID_EXTERNAL_HANDLE;
         goto cleanup;
      }
      import_handle = handle_from_name;
   }
#endif

   if (host_pointer) {
      error = VK_ERROR_INVALID_EXTERNAL_HANDLE;

#if defined(_WIN32)
      if (!device->dev13)
         goto cleanup;

      if (FAILED(ID3D12Device13_OpenExistingHeapFromAddress1(device->dev13, host_pointer, heap_desc.SizeInBytes, &IID_ID3D12Heap, (void**)&mem->heap)))
         goto cleanup;

      D3D12_HEAP_DESC desc = dzn_ID3D12Heap_GetDesc(mem->heap);
      if (desc.Properties.Type != D3D12_HEAP_TYPE_CUSTOM)
         desc.Properties = dzn_ID3D12Device4_GetCustomHeapProperties(device->dev, 0, desc.Properties.Type);

      if ((heap_desc.Flags & ~desc.Flags) ||
          desc.Properties.CPUPageProperty != heap_desc.Properties.CPUPageProperty ||
          desc.Properties.MemoryPoolPreference != heap_desc.Properties.MemoryPoolPreference)
         goto cleanup;

      mem->map = host_pointer;
      mem->res_flags = D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER;
#else
      goto cleanup;
#endif
   } else if (import_handle) {
      error = VK_ERROR_INVALID_EXTERNAL_HANDLE;
      if (image || buffer) {
         if (FAILED(ID3D12Device_OpenSharedHandle(device->dev, import_handle, &IID_ID3D12Resource, (void **)&mem->dedicated_res)))
            goto cleanup;

         /* Verify compatibility */
         D3D12_RESOURCE_DESC desc = dzn_ID3D12Resource_GetDesc(mem->dedicated_res);
         D3D12_HEAP_PROPERTIES opened_props = { 0 };
         D3D12_HEAP_FLAGS opened_flags = 0;
         ID3D12Resource_GetHeapProperties(mem->dedicated_res, &opened_props, &opened_flags);
         if (opened_props.Type != D3D12_HEAP_TYPE_CUSTOM)
            opened_props = dzn_ID3D12Device4_GetCustomHeapProperties(device->dev, 0, opened_props.Type);

         /* Don't validate format, cast lists aren't reflectable so it could be valid */
         if (image) {
            if (desc.Dimension != image->desc.Dimension ||
                desc.MipLevels != image->desc.MipLevels ||
                desc.Width != image->desc.Width ||
                desc.Height != image->desc.Height ||
                desc.DepthOrArraySize != image->desc.DepthOrArraySize ||
                (image->desc.Flags & ~desc.Flags) ||
                desc.SampleDesc.Count != image->desc.SampleDesc.Count)
               goto cleanup;
         } else if (desc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER ||
                    desc.Width != buffer->desc.Width ||
                    buffer->desc.Flags & ~(desc.Flags))
            goto cleanup;
         if (opened_props.CPUPageProperty != heap_desc.Properties.CPUPageProperty ||
             opened_props.MemoryPoolPreference != heap_desc.Properties.MemoryPoolPreference)
            goto cleanup;
         if ((heap_desc.Flags & D3D12_HEAP_FLAG_DENY_BUFFERS) && desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
            goto cleanup;
         if ((heap_desc.Flags & D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES) && (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET))
            goto cleanup;
         else if ((heap_desc.Flags & D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES) && !(desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET))
            goto cleanup;
      } else {
         if (FAILED(ID3D12Device_OpenSharedHandle(device->dev, import_handle, &IID_ID3D12Heap, (void **)&mem->heap)))
            goto cleanup;

         D3D12_HEAP_DESC desc = dzn_ID3D12Heap_GetDesc(mem->heap);
         if (desc.Properties.Type != D3D12_HEAP_TYPE_CUSTOM)
            desc.Properties = dzn_ID3D12Device4_GetCustomHeapProperties(device->dev, 0, desc.Properties.Type);

         if (desc.Alignment < heap_desc.Alignment ||
             desc.SizeInBytes < heap_desc.SizeInBytes ||
             (heap_desc.Flags & ~desc.Flags) ||
             desc.Properties.CPUPageProperty != heap_desc.Properties.CPUPageProperty ||
             desc.Properties.MemoryPoolPreference != heap_desc.Properties.MemoryPoolPreference)
            goto cleanup;
      }
   } else if (image) {
      if (device->dev10 && image->castable_format_count > 0) {
         D3D12_RESOURCE_DESC1 desc = {
            .Dimension = image->desc.Dimension,
            .Alignment = image->desc.Alignment,
            .Width = image->desc.Width,
            .Height = image->desc.Height,
            .DepthOrArraySize = image->desc.DepthOrArraySize,
            .MipLevels = image->desc.MipLevels,
            .Format = image->desc.Format,
            .SampleDesc = image->desc.SampleDesc,
            .Layout = image->desc.Layout,
            .Flags = image->desc.Flags,
         };
         if (FAILED(ID3D12Device10_CreateCommittedResource3(device->dev10, &heap_desc.Properties,
                                                            heap_desc.Flags, &desc,
                                                            D3D12_BARRIER_LAYOUT_COMMON,
                                                            NULL, NULL,
                                                            image->castable_format_count,
                                                            image->castable_formats,
                                                            &IID_ID3D12Resource,
                                                            (void **)&mem->dedicated_res)))
            goto cleanup;
      } else if (FAILED(ID3D12Device1_CreateCommittedResource(device->dev, &heap_desc.Properties,
                                                              heap_desc.Flags, &image->desc,
                                                              D3D12_RESOURCE_STATE_COMMON,
                                                              NULL,
                                                              &IID_ID3D12Resource,
                                                              (void **)&mem->dedicated_res)))
         goto cleanup;
   } else if (buffer) {
      if (FAILED(ID3D12Device1_CreateCommittedResource(device->dev, &heap_desc.Properties,
                                                       heap_desc.Flags, &buffer->desc,
                                                       D3D12_RESOURCE_STATE_COMMON,
                                                       NULL,
                                                       &IID_ID3D12Resource,
                                                       (void **)&mem->dedicated_res)))
         goto cleanup;
   } else {
      if (FAILED(ID3D12Device1_CreateHeap(device->dev, &heap_desc,
                                          &IID_ID3D12Heap,
                                          (void **)&mem->heap)))
         goto cleanup;
   }

   if ((mem_type->propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) &&
       !(heap_desc.Flags & D3D12_HEAP_FLAG_DENY_BUFFERS) &&
       !mem->map){
      assert(!image);
      if (buffer) {
         mem->map_res = mem->dedicated_res;
         ID3D12Resource_AddRef(mem->map_res);
      } else {
         D3D12_RESOURCE_DESC res_desc = { 0 };
         res_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
         res_desc.Format = DXGI_FORMAT_UNKNOWN;
         res_desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
         res_desc.Width = heap_desc.SizeInBytes;
         res_desc.Height = 1;
         res_desc.DepthOrArraySize = 1;
         res_desc.MipLevels = 1;
         res_desc.SampleDesc.Count = 1;
         res_desc.SampleDesc.Quality = 0;
         res_desc.Flags = D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
         res_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
         HRESULT hr = ID3D12Device1_CreatePlacedResource(device->dev, mem->heap, 0, &res_desc,
                                                         D3D12_RESOURCE_STATE_COMMON,
                                                         NULL,
                                                         &IID_ID3D12Resource,
                                                         (void **)&mem->map_res);
         if (FAILED(hr))
            goto cleanup;
      }
   }

   if (export_flags) {
      error = VK_ERROR_INVALID_EXTERNAL_HANDLE;
      ID3D12DeviceChild *shareable = mem->heap ? (void *)mem->heap : (void *)mem->dedicated_res;
      DWORD dwAccess = GENERIC_ALL; /* Ignore any provided access, this is the only one D3D allows */
#ifdef _WIN32
      const SECURITY_ATTRIBUTES *pAttributes = win32_export ? win32_export->pAttributes : NULL;
      const wchar_t *name = win32_export ? win32_export->name : NULL;
#else
      const SECURITY_ATTRIBUTES *pAttributes = NULL;
      const wchar_t *name = NULL;
#endif
      if (FAILED(ID3D12Device_CreateSharedHandle(device->dev, shareable, pAttributes,
                                                 dwAccess, name, &mem->export_handle)))
         goto cleanup;
   }

   *out = dzn_device_memory_to_handle(mem);
   return VK_SUCCESS;

cleanup:
#ifdef _WIN32
   if (handle_from_name)
      CloseHandle(handle_from_name);
#endif
   dzn_device_memory_destroy(mem, pAllocator);
   return vk_error(device, error);
}

VKAPI_ATTR VkResult VKAPI_CALL
dzn_AllocateMemory(VkDevice device,
                   const VkMemoryAllocateInfo *pAllocateInfo,
                   const VkAllocationCallbacks *pAllocator,
                   VkDeviceMemory *pMem)
{
   return dzn_device_memory_create(dzn_device_from_handle(device),
                                   pAllocateInfo, pAllocator, pMem);
}

VKAPI_ATTR void VKAPI_CALL
dzn_FreeMemory(VkDevice device,
               VkDeviceMemory mem,
               const VkAllocationCallbacks *pAllocator)
{
   dzn_device_memory_destroy(dzn_device_memory_from_handle(mem), pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL
dzn_MapMemory(VkDevice _device,
              VkDeviceMemory _memory,
              VkDeviceSize offset,
              VkDeviceSize size,
              VkMemoryMapFlags flags,
              void **ppData)
{
   VK_FROM_HANDLE(dzn_device, device, _device);
   VK_FROM_HANDLE(dzn_device_memory, mem, _memory);

   if (mem == NULL) {
      *ppData = NULL;
      return VK_SUCCESS;
   }

   if (mem->map && !mem->map_res) {
      *ppData = ((uint8_t *)mem->map) + offset;
      return VK_SUCCESS;
   }

   if (size == VK_WHOLE_SIZE)
      size = mem->size - offset;

   /* From the Vulkan spec version 1.0.32 docs for MapMemory:
    *
    *  * If size is not equal to VK_WHOLE_SIZE, size must be greater than 0
    *    assert(size != 0);
    *  * If size is not equal to VK_WHOLE_SIZE, size must be less than or
    *    equal to the size of the memory minus offset
    */
   assert(size > 0);
   assert(offset + size <= mem->size);

   assert(mem->map_res);
   D3D12_RANGE range = { 0 };
   range.Begin = offset;
   range.End = offset + size;
   void *map = NULL;
   if (FAILED(ID3D12Resource_Map(mem->map_res, 0, &range, &map)))
      return vk_error(device, VK_ERROR_MEMORY_MAP_FAILED);

   mem->map = map;
   mem->map_size = size;

   *ppData = ((uint8_t *) map) + offset;

   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL
dzn_UnmapMemory(VkDevice _device,
                VkDeviceMemory _memory)
{
   VK_FROM_HANDLE(dzn_device_memory, mem, _memory);

   if (mem == NULL)
      return;

   if (!mem->map_res)
      return;

   ID3D12Resource_Unmap(mem->map_res, 0, NULL);

   mem->map = NULL;
   mem->map_size = 0;
}

VKAPI_ATTR VkResult VKAPI_CALL
dzn_FlushMappedMemoryRanges(VkDevice _device,
                            uint32_t memoryRangeCount,
                            const VkMappedMemoryRange *pMemoryRanges)
{
   return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL
dzn_InvalidateMappedMemoryRanges(VkDevice _device,
                                 uint32_t memoryRangeCount,
                                 const VkMappedMemoryRange *pMemoryRanges)
{
   return VK_SUCCESS;
}

static void
dzn_buffer_destroy(struct dzn_buffer *buf, const VkAllocationCallbacks *pAllocator)
{
   if (!buf)
      return;

   struct dzn_device *device = container_of(buf->base.device, struct dzn_device, vk);

   if (buf->res)
      ID3D12Resource_Release(buf->res);

   dzn_device_descriptor_heap_free_slot(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, buf->cbv_bindless_slot);
   dzn_device_descriptor_heap_free_slot(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, buf->uav_bindless_slot);
   if (buf->custom_views) {
      hash_table_foreach(buf->custom_views, entry) {
         free((void *)entry->key);
         dzn_device_descriptor_heap_free_slot(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, (int)(intptr_t)entry->data);
      }
      _mesa_hash_table_destroy(buf->custom_views, NULL);
   }

   vk_object_base_finish(&buf->base);
   vk_free2(&device->vk.alloc, pAllocator, buf);
}

static VkResult
dzn_buffer_create(struct dzn_device *device,
                  const VkBufferCreateInfo *pCreateInfo,
                  const VkAllocationCallbacks *pAllocator,
                  VkBuffer *out)
{
   struct dzn_buffer *buf =
      vk_zalloc2(&device->vk.alloc, pAllocator, sizeof(*buf), 8,
                 VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (!buf)
     return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   vk_object_base_init(&device->vk, &buf->base, VK_OBJECT_TYPE_BUFFER);
   buf->create_flags = pCreateInfo->flags;
   buf->size = pCreateInfo->size;
   buf->usage = pCreateInfo->usage;

   if (buf->usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
      buf->size = MAX2(buf->size, ALIGN_POT(buf->size, 256));
   if (buf->usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
      buf->size = MAX2(buf->size, ALIGN_POT(buf->size, 4));

   buf->desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
   buf->desc.Format = DXGI_FORMAT_UNKNOWN;
   buf->desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
   buf->desc.Width = buf->size;
   buf->desc.Height = 1;
   buf->desc.DepthOrArraySize = 1;
   buf->desc.MipLevels = 1;
   buf->desc.SampleDesc.Count = 1;
   buf->desc.SampleDesc.Quality = 0;
   buf->desc.Flags = D3D12_RESOURCE_FLAG_NONE;
   buf->desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
   buf->valid_access =
      D3D12_BARRIER_ACCESS_VERTEX_BUFFER |
      D3D12_BARRIER_ACCESS_CONSTANT_BUFFER |
      D3D12_BARRIER_ACCESS_INDEX_BUFFER |
      D3D12_BARRIER_ACCESS_SHADER_RESOURCE |
      D3D12_BARRIER_ACCESS_STREAM_OUTPUT |
      D3D12_BARRIER_ACCESS_INDIRECT_ARGUMENT |
      D3D12_BARRIER_ACCESS_PREDICATION |
      D3D12_BARRIER_ACCESS_COPY_DEST |
      D3D12_BARRIER_ACCESS_COPY_SOURCE |
      D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_READ |
      D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_WRITE;

   if (buf->usage &
       (VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
        VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT)) {
      buf->desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
      buf->valid_access |= D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;
   }

   buf->cbv_bindless_slot = buf->uav_bindless_slot = -1;
   if (device->bindless) {
      if (buf->usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) {
         buf->cbv_bindless_slot = dzn_device_descriptor_heap_alloc_slot(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
         if (buf->cbv_bindless_slot < 0) {
            dzn_buffer_destroy(buf, pAllocator);
            return vk_error(device, VK_ERROR_OUT_OF_DEVICE_MEMORY);
         }
      }
      if (buf->usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) {
         buf->uav_bindless_slot = dzn_device_descriptor_heap_alloc_slot(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
         if (buf->uav_bindless_slot < 0) {
            dzn_buffer_destroy(buf, pAllocator);
            return vk_error(device, VK_ERROR_OUT_OF_DEVICE_MEMORY);
         }
      }
   }

   if (device->bindless)
      mtx_init(&buf->bindless_view_lock, mtx_plain);

   const VkExternalMemoryBufferCreateInfo *external_info =
      vk_find_struct_const(pCreateInfo->pNext, EXTERNAL_MEMORY_BUFFER_CREATE_INFO);
   if (external_info && external_info->handleTypes != 0)
      buf->shared = true;

   *out = dzn_buffer_to_handle(buf);
   return VK_SUCCESS;
}

DXGI_FORMAT
dzn_buffer_get_dxgi_format(VkFormat format)
{
   enum pipe_format pfmt = vk_format_to_pipe_format(format);

   return dzn_pipe_to_dxgi_format(pfmt);
}

D3D12_TEXTURE_COPY_LOCATION
dzn_buffer_get_copy_loc(const struct dzn_buffer *buf,
                        VkFormat format,
                        const VkBufferImageCopy2 *region,
                        VkImageAspectFlagBits aspect,
                        uint32_t layer)
{
   struct dzn_physical_device *pdev =
      container_of(buf->base.device->physical, struct dzn_physical_device, vk);
   const uint32_t buffer_row_length =
      region->bufferRowLength ? region->bufferRowLength : region->imageExtent.width;

   VkFormat plane_format = dzn_image_get_plane_format(format, aspect);

   enum pipe_format pfmt = vk_format_to_pipe_format(plane_format);
   uint32_t blksz = util_format_get_blocksize(pfmt);
   uint32_t blkw = util_format_get_blockwidth(pfmt);
   uint32_t blkh = util_format_get_blockheight(pfmt);

   D3D12_TEXTURE_COPY_LOCATION loc = {
     .pResource = buf->res,
     .Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT,
     .PlacedFootprint = {
        .Footprint = {
           .Format =
              dzn_image_get_placed_footprint_format(pdev, format, aspect),
           .Width = region->imageExtent.width,
           .Height = region->imageExtent.height,
           .Depth = region->imageExtent.depth,
           .RowPitch = blksz * DIV_ROUND_UP(buffer_row_length, blkw),
        },
     },
   };

   uint32_t buffer_layer_stride =
      loc.PlacedFootprint.Footprint.RowPitch *
      DIV_ROUND_UP(loc.PlacedFootprint.Footprint.Height, blkh);

   loc.PlacedFootprint.Offset =
      region->bufferOffset + (layer * buffer_layer_stride);

   return loc;
}

D3D12_TEXTURE_COPY_LOCATION
dzn_buffer_get_line_copy_loc(const struct dzn_buffer *buf, VkFormat format,
                             const VkBufferImageCopy2 *region,
                             const D3D12_TEXTURE_COPY_LOCATION *loc,
                             uint32_t y, uint32_t z, uint32_t *start_x)
{
   uint32_t buffer_row_length =
      region->bufferRowLength ? region->bufferRowLength : region->imageExtent.width;
   uint32_t buffer_image_height =
      region->bufferImageHeight ? region->bufferImageHeight : region->imageExtent.height;

   format = dzn_image_get_plane_format(format, region->imageSubresource.aspectMask);

   enum pipe_format pfmt = vk_format_to_pipe_format(format);
   uint32_t blksz = util_format_get_blocksize(pfmt);
   uint32_t blkw = util_format_get_blockwidth(pfmt);
   uint32_t blkh = util_format_get_blockheight(pfmt);
   uint32_t blkd = util_format_get_blockdepth(pfmt);
   D3D12_TEXTURE_COPY_LOCATION new_loc = *loc;
   uint32_t buffer_row_stride =
      DIV_ROUND_UP(buffer_row_length, blkw) * blksz;
   uint32_t buffer_layer_stride =
      buffer_row_stride *
      DIV_ROUND_UP(buffer_image_height, blkh);

   uint64_t tex_offset =
      ((y / blkh) * buffer_row_stride) +
      ((z / blkd) * buffer_layer_stride);
   uint64_t offset = loc->PlacedFootprint.Offset + tex_offset;
   uint32_t offset_alignment = D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT;

   while (offset_alignment % blksz)
      offset_alignment += D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT;

   new_loc.PlacedFootprint.Footprint.Height = blkh;
   new_loc.PlacedFootprint.Footprint.Depth = 1;
   new_loc.PlacedFootprint.Offset = (offset / offset_alignment) * offset_alignment;
   *start_x = ((offset % offset_alignment) / blksz) * blkw;
   new_loc.PlacedFootprint.Footprint.Width = *start_x + region->imageExtent.width;
   new_loc.PlacedFootprint.Footprint.RowPitch =
      ALIGN_POT(DIV_ROUND_UP(new_loc.PlacedFootprint.Footprint.Width, blkw) * blksz,
                D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
   return new_loc;
}

bool
dzn_buffer_supports_region_copy(struct dzn_physical_device *pdev,
                                const D3D12_TEXTURE_COPY_LOCATION *loc)
{
   if (pdev->options13.UnrestrictedBufferTextureCopyPitchSupported)
      return true;
   return !(loc->PlacedFootprint.Offset & (D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT - 1)) &&
          !(loc->PlacedFootprint.Footprint.RowPitch & (D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1));
}

VKAPI_ATTR VkResult VKAPI_CALL
dzn_CreateBuffer(VkDevice device,
                 const VkBufferCreateInfo *pCreateInfo,
                 const VkAllocationCallbacks *pAllocator,
                 VkBuffer *pBuffer)
{
   return dzn_buffer_create(dzn_device_from_handle(device),
                            pCreateInfo, pAllocator, pBuffer);
}

VKAPI_ATTR void VKAPI_CALL
dzn_DestroyBuffer(VkDevice device,
                  VkBuffer buffer,
                  const VkAllocationCallbacks *pAllocator)
{
   dzn_buffer_destroy(dzn_buffer_from_handle(buffer), pAllocator);
}

VKAPI_ATTR void VKAPI_CALL
dzn_GetBufferMemoryRequirements2(VkDevice dev,
                                 const VkBufferMemoryRequirementsInfo2 *pInfo,
                                 VkMemoryRequirements2 *pMemoryRequirements)
{
   VK_FROM_HANDLE(dzn_device, device, dev);
   VK_FROM_HANDLE(dzn_buffer, buffer, pInfo->buffer);
   struct dzn_physical_device *pdev =
      container_of(device->vk.physical, struct dzn_physical_device, vk);

   uint32_t alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
   VkDeviceSize size = buffer->size;

   if (buffer->usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) {
      alignment = MAX2(alignment, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
      size = ALIGN_POT(size, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
   }

   pMemoryRequirements->memoryRequirements.size = size;
   pMemoryRequirements->memoryRequirements.alignment = alignment;
   pMemoryRequirements->memoryRequirements.memoryTypeBits =
      dzn_physical_device_get_mem_type_mask_for_resource(pdev, &buffer->desc, buffer->shared);

   vk_foreach_struct(ext, pMemoryRequirements->pNext) {
      switch (ext->sType) {
      case VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS: {
         VkMemoryDedicatedRequirements *requirements =
            (VkMemoryDedicatedRequirements *)ext;
         requirements->requiresDedicatedAllocation = false;
         requirements->prefersDedicatedAllocation = false;
         break;
      }

      default:
         dzn_debug_ignored_stype(ext->sType);
         break;
      }
   }
}

VKAPI_ATTR VkResult VKAPI_CALL
dzn_BindBufferMemory2(VkDevice _device,
                      uint32_t bindInfoCount,
                      const VkBindBufferMemoryInfo *pBindInfos)
{
   VK_FROM_HANDLE(dzn_device, device, _device);

   for (uint32_t i = 0; i < bindInfoCount; i++) {
      assert(pBindInfos[i].sType == VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO);

      VK_FROM_HANDLE(dzn_device_memory, mem, pBindInfos[i].memory);
      VK_FROM_HANDLE(dzn_buffer, buffer, pBindInfos[i].buffer);

      if (mem->dedicated_res) {
         assert(pBindInfos[i].memoryOffset == 0 &&
                buffer->size == mem->size);
         buffer->res = mem->dedicated_res;
         ID3D12Resource_AddRef(buffer->res);
      } else {
         D3D12_RESOURCE_DESC desc = buffer->desc;
         desc.Flags |= mem->res_flags;
         if (FAILED(ID3D12Device1_CreatePlacedResource(device->dev, mem->heap,
                                                       pBindInfos[i].memoryOffset,
                                                       &buffer->desc,
                                                       D3D12_RESOURCE_STATE_COMMON,
                                                       NULL,
                                                       &IID_ID3D12Resource,
                                                       (void **)&buffer->res)))
            return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);
      }

      buffer->gpuva = ID3D12Resource_GetGPUVirtualAddress(buffer->res);

      if (device->bindless) {
         struct dzn_buffer_desc buf_desc = {
            .buffer = buffer,
            .offset = 0,
            .range = VK_WHOLE_SIZE,
         };
         if (buffer->cbv_bindless_slot >= 0) {
            buf_desc.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            dzn_descriptor_heap_write_buffer_desc(device,
                                                  &device->device_heaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].heap,
                                                  buffer->cbv_bindless_slot,
                                                  false,
                                                  &buf_desc);
         }
         if (buffer->uav_bindless_slot >= 0) {
            buf_desc.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            dzn_descriptor_heap_write_buffer_desc(device,
                                                  &device->device_heaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].heap,
                                                  buffer->uav_bindless_slot,
                                                  true,
                                                  &buf_desc);
         }
      }
   }

   return VK_SUCCESS;
}

static void
dzn_event_destroy(struct dzn_event *event,
                  const VkAllocationCallbacks *pAllocator)
{
   if (!event)
      return;

   struct dzn_device *device =
      container_of(event->base.device, struct dzn_device, vk);

   if (event->fence)
      ID3D12Fence_Release(event->fence);

   vk_object_base_finish(&event->base);
   vk_free2(&device->vk.alloc, pAllocator, event);
}

static VkResult
dzn_event_create(struct dzn_device *device,
                 const VkEventCreateInfo *pCreateInfo,
                 const VkAllocationCallbacks *pAllocator,
                 VkEvent *out)
{
   struct dzn_event *event =
      vk_zalloc2(&device->vk.alloc, pAllocator, sizeof(*event), 8,
                 VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (!event)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   vk_object_base_init(&device->vk, &event->base, VK_OBJECT_TYPE_EVENT);

   if (FAILED(ID3D12Device1_CreateFence(device->dev, 0, D3D12_FENCE_FLAG_NONE,
                                        &IID_ID3D12Fence,
                                        (void **)&event->fence))) {
      dzn_event_destroy(event, pAllocator);
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);
   }

   *out = dzn_event_to_handle(event);
   return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL
dzn_CreateEvent(VkDevice device,
                const VkEventCreateInfo *pCreateInfo,
                const VkAllocationCallbacks *pAllocator,
                VkEvent *pEvent)
{
   return dzn_event_create(dzn_device_from_handle(device),
                           pCreateInfo, pAllocator, pEvent);
}

VKAPI_ATTR void VKAPI_CALL
dzn_DestroyEvent(VkDevice device,
                 VkEvent event,
                 const VkAllocationCallbacks *pAllocator)
{
   dzn_event_destroy(dzn_event_from_handle(event), pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL
dzn_ResetEvent(VkDevice dev,
               VkEvent evt)
{
   VK_FROM_HANDLE(dzn_device, device, dev);
   VK_FROM_HANDLE(dzn_event, event, evt);

   if (FAILED(ID3D12Fence_Signal(event->fence, 0)))
      return vk_error(device, VK_ERROR_OUT_OF_DEVICE_MEMORY);

   return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL
dzn_SetEvent(VkDevice dev,
             VkEvent evt)
{
   VK_FROM_HANDLE(dzn_device, device, dev);
   VK_FROM_HANDLE(dzn_event, event, evt);

   if (FAILED(ID3D12Fence_Signal(event->fence, 1)))
      return vk_error(device, VK_ERROR_OUT_OF_DEVICE_MEMORY);

   return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL
dzn_GetEventStatus(VkDevice device,
                   VkEvent evt)
{
   VK_FROM_HANDLE(dzn_event, event, evt);

   return ID3D12Fence_GetCompletedValue(event->fence) == 0 ?
          VK_EVENT_RESET : VK_EVENT_SET;
}

VKAPI_ATTR void VKAPI_CALL
dzn_GetDeviceMemoryCommitment(VkDevice device,
                              VkDeviceMemory memory,
                              VkDeviceSize *pCommittedMemoryInBytes)
{
   VK_FROM_HANDLE(dzn_device_memory, mem, memory);

   // TODO: find if there's a way to query/track actual heap residency
   *pCommittedMemoryInBytes = mem->size;
}

VKAPI_ATTR VkResult VKAPI_CALL
dzn_QueueBindSparse(VkQueue queue,
                    uint32_t bindInfoCount,
                    const VkBindSparseInfo *pBindInfo,
                    VkFence fence)
{
   // FIXME: add proper implem
   dzn_stub();
   return VK_SUCCESS;
}

static D3D12_TEXTURE_ADDRESS_MODE
dzn_sampler_translate_addr_mode(VkSamplerAddressMode in)
{
   switch (in) {
   case VK_SAMPLER_ADDRESS_MODE_REPEAT: return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
   case VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT: return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
   case VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE: return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
   case VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER: return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
   case VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE: return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
   default: unreachable("Invalid address mode");
   }
}

static void
dzn_sampler_destroy(struct dzn_sampler *sampler,
                    const VkAllocationCallbacks *pAllocator)
{
   if (!sampler)
      return;

   struct dzn_device *device =
      container_of(sampler->base.device, struct dzn_device, vk);

   dzn_device_descriptor_heap_free_slot(device, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, sampler->bindless_slot);

   vk_object_base_finish(&sampler->base);
   vk_free2(&device->vk.alloc, pAllocator, sampler);
}

static VkResult
dzn_sampler_create(struct dzn_device *device,
                   const VkSamplerCreateInfo *pCreateInfo,
                   const VkAllocationCallbacks *pAllocator,
                   VkSampler *out)
{
   struct dzn_physical_device *pdev = container_of(device->vk.physical, struct dzn_physical_device, vk);
   struct dzn_sampler *sampler =
      vk_zalloc2(&device->vk.alloc, pAllocator, sizeof(*sampler), 8,
                 VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (!sampler)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   vk_object_base_init(&device->vk, &sampler->base, VK_OBJECT_TYPE_SAMPLER);

   const VkSamplerCustomBorderColorCreateInfoEXT *pBorderColor = (const VkSamplerCustomBorderColorCreateInfoEXT *)
      vk_find_struct_const(pCreateInfo->pNext, SAMPLER_CUSTOM_BORDER_COLOR_CREATE_INFO_EXT);

   /* TODO: have a sampler pool to allocate shader-invisible descs which we
    * can copy to the desc_set when UpdateDescriptorSets() is called.
    */
   sampler->desc.Filter = dzn_translate_sampler_filter(pdev, pCreateInfo);
   sampler->desc.AddressU = dzn_sampler_translate_addr_mode(pCreateInfo->addressModeU);
   sampler->desc.AddressV = dzn_sampler_translate_addr_mode(pCreateInfo->addressModeV);
   sampler->desc.AddressW = dzn_sampler_translate_addr_mode(pCreateInfo->addressModeW);
   sampler->desc.MipLODBias = pCreateInfo->mipLodBias;
   sampler->desc.MaxAnisotropy = pCreateInfo->maxAnisotropy;
   sampler->desc.MinLOD = pCreateInfo->minLod;
   sampler->desc.MaxLOD = pCreateInfo->maxLod;

   if (pCreateInfo->compareEnable)
      sampler->desc.ComparisonFunc = dzn_translate_compare_op(pCreateInfo->compareOp);

   bool reads_border_color =
      pCreateInfo->addressModeU == VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER ||
      pCreateInfo->addressModeV == VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER ||
      pCreateInfo->addressModeW == VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;

   if (reads_border_color) {
      switch (pCreateInfo->borderColor) {
      case VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK:
      case VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK:
         sampler->desc.FloatBorderColor[0] = 0.0f;
         sampler->desc.FloatBorderColor[1] = 0.0f;
         sampler->desc.FloatBorderColor[2] = 0.0f;
         sampler->desc.FloatBorderColor[3] =
            pCreateInfo->borderColor == VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK ? 0.0f : 1.0f;
         sampler->static_border_color =
            pCreateInfo->borderColor == VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK ?
            D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK :
            D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
         break;
      case VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE:
         sampler->desc.FloatBorderColor[0] = sampler->desc.FloatBorderColor[1] = 1.0f;
         sampler->desc.FloatBorderColor[2] = sampler->desc.FloatBorderColor[3] = 1.0f;
         sampler->static_border_color = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
         break;
      case VK_BORDER_COLOR_FLOAT_CUSTOM_EXT:
         sampler->static_border_color = (D3D12_STATIC_BORDER_COLOR)-1;
         for (unsigned i = 0; i < ARRAY_SIZE(sampler->desc.FloatBorderColor); i++)
            sampler->desc.FloatBorderColor[i] = pBorderColor->customBorderColor.float32[i];
         break;
      case VK_BORDER_COLOR_INT_TRANSPARENT_BLACK:
      case VK_BORDER_COLOR_INT_OPAQUE_BLACK:
         sampler->desc.UintBorderColor[0] = 0;
         sampler->desc.UintBorderColor[1] = 0;
         sampler->desc.UintBorderColor[2] = 0;
         sampler->desc.UintBorderColor[3] =
            pCreateInfo->borderColor == VK_BORDER_COLOR_INT_TRANSPARENT_BLACK ? 0 : 1;
         sampler->static_border_color =
            pCreateInfo->borderColor == VK_BORDER_COLOR_INT_TRANSPARENT_BLACK ?
            D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK :
            D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK_UINT;
         sampler->desc.Flags = D3D12_SAMPLER_FLAG_UINT_BORDER_COLOR;
         break;
      case VK_BORDER_COLOR_INT_OPAQUE_WHITE:
         sampler->desc.UintBorderColor[0] = sampler->desc.UintBorderColor[1] = 1;
         sampler->desc.UintBorderColor[2] = sampler->desc.UintBorderColor[3] = 1;
         sampler->static_border_color = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE_UINT;
         sampler->desc.Flags = D3D12_SAMPLER_FLAG_UINT_BORDER_COLOR;
         break;
      case VK_BORDER_COLOR_INT_CUSTOM_EXT:
         sampler->static_border_color = (D3D12_STATIC_BORDER_COLOR)-1;
         for (unsigned i = 0; i < ARRAY_SIZE(sampler->desc.UintBorderColor); i++)
            sampler->desc.UintBorderColor[i] = pBorderColor->customBorderColor.uint32[i];
         sampler->desc.Flags = D3D12_SAMPLER_FLAG_UINT_BORDER_COLOR;
         break;
      default:
         unreachable("Unsupported border color");
      }
   }

   if (pCreateInfo->unnormalizedCoordinates && pdev->options17.NonNormalizedCoordinateSamplersSupported)
      sampler->desc.Flags |= D3D12_SAMPLER_FLAG_NON_NORMALIZED_COORDINATES;

   sampler->bindless_slot = -1;
   if (device->bindless) {
      sampler->bindless_slot = dzn_device_descriptor_heap_alloc_slot(device, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
      if (sampler->bindless_slot < 0) {
         dzn_sampler_destroy(sampler, pAllocator);
         return vk_error(device, VK_ERROR_OUT_OF_DEVICE_MEMORY);
      }

      dzn_descriptor_heap_write_sampler_desc(device,
                                             &device->device_heaps[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER].heap,
                                             sampler->bindless_slot,
                                             sampler);
   }

   *out = dzn_sampler_to_handle(sampler);
   return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL
dzn_CreateSampler(VkDevice device,
                  const VkSamplerCreateInfo *pCreateInfo,
                  const VkAllocationCallbacks *pAllocator,
                  VkSampler *pSampler)
{
   return dzn_sampler_create(dzn_device_from_handle(device),
                             pCreateInfo, pAllocator, pSampler);
}

VKAPI_ATTR void VKAPI_CALL
dzn_DestroySampler(VkDevice device,
                   VkSampler sampler,
                   const VkAllocationCallbacks *pAllocator)
{
   dzn_sampler_destroy(dzn_sampler_from_handle(sampler), pAllocator);
}

int
dzn_device_descriptor_heap_alloc_slot(struct dzn_device *device,
                                      D3D12_DESCRIPTOR_HEAP_TYPE type)
{
   struct dzn_device_descriptor_heap *heap = &device->device_heaps[type];
   mtx_lock(&heap->lock);

   int ret = -1;
   if (heap->slot_freelist.size)
      ret = util_dynarray_pop(&heap->slot_freelist, int);
   else if (heap->next_alloc_slot < heap->heap.desc_count)
      ret = heap->next_alloc_slot++;

   mtx_unlock(&heap->lock);
   return ret;
}

void
dzn_device_descriptor_heap_free_slot(struct dzn_device *device,
                                     D3D12_DESCRIPTOR_HEAP_TYPE type,
                                     int slot)
{
   struct dzn_device_descriptor_heap *heap = &device->device_heaps[type];
   assert(slot < 0 || slot < heap->heap.desc_count);

   if (slot < 0)
      return;

   mtx_lock(&heap->lock);
   util_dynarray_append(&heap->slot_freelist, int, slot);
   mtx_unlock(&heap->lock);
}

VKAPI_ATTR void VKAPI_CALL
dzn_GetDeviceGroupPeerMemoryFeatures(VkDevice device,
                                     uint32_t heapIndex,
                                     uint32_t localDeviceIndex,
                                     uint32_t remoteDeviceIndex,
                                     VkPeerMemoryFeatureFlags *pPeerMemoryFeatures)
{
   *pPeerMemoryFeatures = 0;
}

VKAPI_ATTR void VKAPI_CALL
dzn_GetImageSparseMemoryRequirements2(VkDevice device,
                                      const VkImageSparseMemoryRequirementsInfo2* pInfo,
                                      uint32_t *pSparseMemoryRequirementCount,
                                      VkSparseImageMemoryRequirements2 *pSparseMemoryRequirements)
{
   *pSparseMemoryRequirementCount = 0;
}

VKAPI_ATTR VkResult VKAPI_CALL
dzn_CreateSamplerYcbcrConversion(VkDevice device,
                                 const VkSamplerYcbcrConversionCreateInfo *pCreateInfo,
                                 const VkAllocationCallbacks *pAllocator,
                                 VkSamplerYcbcrConversion *pYcbcrConversion)
{
   unreachable("Ycbcr sampler conversion is not supported");
   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL
dzn_DestroySamplerYcbcrConversion(VkDevice device,
                                  VkSamplerYcbcrConversion YcbcrConversion,
                                  const VkAllocationCallbacks *pAllocator)
{
   unreachable("Ycbcr sampler conversion is not supported");
}

VKAPI_ATTR VkDeviceAddress VKAPI_CALL
dzn_GetBufferDeviceAddress(VkDevice device,
                           const VkBufferDeviceAddressInfo* pInfo)
{
   struct dzn_buffer *buffer = dzn_buffer_from_handle(pInfo->buffer);

   return buffer->gpuva;
}

VKAPI_ATTR uint64_t VKAPI_CALL
dzn_GetBufferOpaqueCaptureAddress(VkDevice device,
                                  const VkBufferDeviceAddressInfo *pInfo)
{
   return 0;
}

VKAPI_ATTR uint64_t VKAPI_CALL
dzn_GetDeviceMemoryOpaqueCaptureAddress(VkDevice device,
                                        const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo)
{
   return 0;
}

#ifdef _WIN32
VKAPI_ATTR VkResult VKAPI_CALL
dzn_GetMemoryWin32HandleKHR(VkDevice device,
                            const VkMemoryGetWin32HandleInfoKHR *pGetWin32HandleInfo,
                            HANDLE *pHandle)
{
   VK_FROM_HANDLE(dzn_device_memory, mem, pGetWin32HandleInfo->memory);
   if (!mem->export_handle)
      return vk_error(device, VK_ERROR_INVALID_EXTERNAL_HANDLE);

   switch (pGetWin32HandleInfo->handleType) {
   case VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT:
   case VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT:
   case VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_RESOURCE_BIT:
   case VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_HEAP_BIT:
      if (!DuplicateHandle(GetCurrentProcess(), mem->export_handle, GetCurrentProcess(), pHandle,
                           0, false, DUPLICATE_SAME_ACCESS))
         return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);
      return VK_SUCCESS;
   default:
      return vk_error(device, VK_ERROR_INVALID_EXTERNAL_HANDLE);
   }
}
#else
VKAPI_ATTR VkResult VKAPI_CALL
dzn_GetMemoryFdKHR(VkDevice device,
                   const VkMemoryGetFdInfoKHR *pGetFdInfo,
                   int *pFd)
{
   VK_FROM_HANDLE(dzn_device_memory, mem, pGetFdInfo->memory);
   if (!mem->export_handle)
      return vk_error(device, VK_ERROR_INVALID_EXTERNAL_HANDLE);

   switch (pGetFdInfo->handleType) {
   case VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT:
   case VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_RESOURCE_BIT:
   case VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_HEAP_BIT:
      *pFd = (int)(intptr_t)mem->export_handle;
      mem->export_handle = (HANDLE)(intptr_t)-1;
      return VK_SUCCESS;
   default:
      return vk_error(device, VK_ERROR_INVALID_EXTERNAL_HANDLE);
   }
}
#endif

#ifdef _WIN32
VKAPI_ATTR VkResult VKAPI_CALL
dzn_GetMemoryWin32HandlePropertiesKHR(VkDevice _device,
                                      VkExternalMemoryHandleTypeFlagBits handleType,
                                      HANDLE handle,
                                      VkMemoryWin32HandlePropertiesKHR *pProperties)
{
#else
VKAPI_ATTR VkResult VKAPI_CALL
dzn_GetMemoryFdPropertiesKHR(VkDevice _device,
                             VkExternalMemoryHandleTypeFlagBits handleType,
                             int fd,
                             VkMemoryFdPropertiesKHR *pProperties)
{
   HANDLE handle = (HANDLE)(intptr_t)fd;
#endif
   VK_FROM_HANDLE(dzn_device, device, _device);
   IUnknown *opened_object;
   if (FAILED(ID3D12Device_OpenSharedHandle(device->dev, handle, &IID_IUnknown, (void **)&opened_object)))
      return vk_error(device, VK_ERROR_INVALID_EXTERNAL_HANDLE);

   VkResult result = VK_ERROR_INVALID_EXTERNAL_HANDLE;
   ID3D12Resource *res = NULL;
   ID3D12Heap *heap = NULL;
   struct dzn_physical_device *pdev = container_of(device->vk.physical, struct dzn_physical_device, vk);

   switch (handleType) {
   case VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT:
      (void)IUnknown_QueryInterface(opened_object, &IID_ID3D12Resource, (void **)&res);
      (void)IUnknown_QueryInterface(opened_object, &IID_ID3D12Heap, (void **)&heap);
      break;
   case VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT:
   case VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_RESOURCE_BIT:
      (void)IUnknown_QueryInterface(opened_object, &IID_ID3D12Resource, (void **)&res);
      break;
   case VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_HEAP_BIT:
      (void)IUnknown_QueryInterface(opened_object, &IID_ID3D12Heap, (void **)&heap);
      break;
   default:
      goto cleanup;
   }
   if (!res && !heap)
      goto cleanup;

   D3D12_HEAP_DESC heap_desc;
   if (res)
      ID3D12Resource_GetHeapProperties(res, &heap_desc.Properties, &heap_desc.Flags);
   else
      heap_desc = dzn_ID3D12Heap_GetDesc(heap);
   if (heap_desc.Properties.Type != D3D12_HEAP_TYPE_CUSTOM)
      heap_desc.Properties = dzn_ID3D12Device4_GetCustomHeapProperties(device->dev, 0, heap_desc.Properties.Type);

   for (uint32_t i = 0; i < pdev->memory.memoryTypeCount; ++i) {
      const VkMemoryType *mem_type = &pdev->memory.memoryTypes[i];
      D3D12_HEAP_PROPERTIES required_props = deduce_heap_properties_from_memory(pdev, mem_type);
      if (heap_desc.Properties.CPUPageProperty != required_props.CPUPageProperty ||
          heap_desc.Properties.MemoryPoolPreference != required_props.MemoryPoolPreference)
         continue;

      D3D12_HEAP_FLAGS required_flags = dzn_physical_device_get_heap_flags_for_mem_type(pdev, i);
      if ((heap_desc.Flags & required_flags) != required_flags)
         continue;

      pProperties->memoryTypeBits |= (1 << i);
   }
   result = VK_SUCCESS;

cleanup:
   IUnknown_Release(opened_object);
   if (res)
      ID3D12Resource_Release(res);
   if (heap)
      ID3D12Heap_Release(heap);
   return result;
}

#if defined(_WIN32)
VKAPI_ATTR VkResult VKAPI_CALL
dzn_GetMemoryHostPointerPropertiesEXT(VkDevice _device,
                                      VkExternalMemoryHandleTypeFlagBits handleType,
                                      const void *pHostPointer,
                                      VkMemoryHostPointerPropertiesEXT *pMemoryHostPointerProperties)
{
   VK_FROM_HANDLE(dzn_device, device, _device);

   if (!device->dev13)
      return VK_ERROR_FEATURE_NOT_PRESENT;

   ID3D12Heap *heap;
   if (FAILED(ID3D12Device13_OpenExistingHeapFromAddress1(device->dev13, pHostPointer, 1, &IID_ID3D12Heap, (void **)&heap)))
      return VK_ERROR_INVALID_EXTERNAL_HANDLE;

   struct dzn_physical_device *pdev = container_of(device->vk.physical, struct dzn_physical_device, vk);
   D3D12_HEAP_DESC heap_desc = dzn_ID3D12Heap_GetDesc(heap);
   for (uint32_t i = 0; i < pdev->memory.memoryTypeCount; ++i) {
      const VkMemoryType *mem_type = &pdev->memory.memoryTypes[i];
      D3D12_HEAP_PROPERTIES required_props = deduce_heap_properties_from_memory(pdev, mem_type);
      if (heap_desc.Properties.CPUPageProperty != required_props.CPUPageProperty ||
          heap_desc.Properties.MemoryPoolPreference != required_props.MemoryPoolPreference)
         continue;

      pMemoryHostPointerProperties->memoryTypeBits |= (1 << i);
   }
   ID3D12Heap_Release(heap);
   return VK_SUCCESS;
}
#endif
