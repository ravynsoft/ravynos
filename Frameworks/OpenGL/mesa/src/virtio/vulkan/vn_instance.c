/*
 * Copyright 2019 Google LLC
 * SPDX-License-Identifier: MIT
 *
 * based in part on anv and radv which are:
 * Copyright © 2015 Intel Corporation
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 */

#include "vn_instance.h"

#include "util/driconf.h"
#include "venus-protocol/vn_protocol_driver_info.h"
#include "venus-protocol/vn_protocol_driver_instance.h"
#include "venus-protocol/vn_protocol_driver_transport.h"

#include "vn_icd.h"
#include "vn_physical_device.h"
#include "vn_renderer.h"
#include "vn_ring.h"

#define VN_INSTANCE_RING_SIZE (128 * 1024)

/*
 * Instance extensions add instance-level or physical-device-level
 * functionalities.  It seems renderer support is either unnecessary or
 * optional.  We should be able to advertise them or lie about them locally.
 */
static const struct vk_instance_extension_table
   vn_instance_supported_extensions = {
      /* promoted to VK_VERSION_1_1 */
      .KHR_device_group_creation = true,
      .KHR_external_fence_capabilities = true,
      .KHR_external_memory_capabilities = true,
      .KHR_external_semaphore_capabilities = true,
      .KHR_get_physical_device_properties2 = true,

#ifdef VN_USE_WSI_PLATFORM
      .KHR_get_surface_capabilities2 = true,
      .KHR_surface = true,
      .KHR_surface_protected_capabilities = true,
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
      .KHR_wayland_surface = true,
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
      .KHR_xcb_surface = true,
#endif
#ifdef VK_USE_PLATFORM_XLIB_KHR
      .KHR_xlib_surface = true,
#endif
   };

static const driOptionDescription vn_dri_options[] = {
   /* clang-format off */
   DRI_CONF_SECTION_PERFORMANCE
      DRI_CONF_VK_X11_ENSURE_MIN_IMAGE_COUNT(false)
      DRI_CONF_VK_X11_OVERRIDE_MIN_IMAGE_COUNT(0)
      DRI_CONF_VK_X11_STRICT_IMAGE_COUNT(false)
      DRI_CONF_VK_XWAYLAND_WAIT_READY(true)
      DRI_CONF_VENUS_IMPLICIT_FENCING(false)
      DRI_CONF_VENUS_WSI_MULTI_PLANE_MODIFIERS(false)
   DRI_CONF_SECTION_END
   DRI_CONF_SECTION_DEBUG
      DRI_CONF_VK_WSI_FORCE_BGRA8_UNORM_FIRST(false)
      DRI_CONF_VK_WSI_FORCE_SWAPCHAIN_TO_CURRENT_EXTENT(false)
   DRI_CONF_SECTION_END
   /* clang-format on */
};

static VkResult
vn_instance_init_renderer_versions(struct vn_instance *instance)
{
   uint32_t instance_version = 0;
   VkResult result = vn_call_vkEnumerateInstanceVersion(instance->ring.ring,
                                                        &instance_version);
   if (result != VK_SUCCESS) {
      if (VN_DEBUG(INIT))
         vn_log(instance, "failed to enumerate renderer instance version");
      return result;
   }

   if (instance_version < VN_MIN_RENDERER_VERSION) {
      if (VN_DEBUG(INIT)) {
         vn_log(instance, "unsupported renderer instance version %d.%d",
                VK_VERSION_MAJOR(instance_version),
                VK_VERSION_MINOR(instance_version));
      }
      return VK_ERROR_INITIALIZATION_FAILED;
   }

   if (VN_DEBUG(INIT)) {
      vn_log(instance, "renderer instance version %d.%d.%d",
             VK_VERSION_MAJOR(instance_version),
             VK_VERSION_MINOR(instance_version),
             VK_VERSION_PATCH(instance_version));
   }

   /* request at least VN_MIN_RENDERER_VERSION internally */
   instance->renderer_api_version =
      MAX2(instance->base.base.app_info.api_version, VN_MIN_RENDERER_VERSION);

   /* instance version for internal use is capped */
   instance_version = MIN3(instance_version, instance->renderer_api_version,
                           instance->renderer->info.vk_xml_version);
   assert(instance_version >= VN_MIN_RENDERER_VERSION);

   instance->renderer_version = instance_version;

   return VK_SUCCESS;
}

static inline void
vn_instance_fini_ring(struct vn_instance *instance)
{
   mtx_destroy(&instance->ring.roundtrip_mutex);

   vn_watchdog_fini(&instance->ring.watchdog);

   list_for_each_entry_safe(struct vn_tls_ring, tls_ring,
                            &instance->ring.tls_rings, vk_head)
      vn_tls_destroy_ring(tls_ring);

   vn_ring_destroy(instance->ring.ring);
}

static VkResult
vn_instance_init_ring(struct vn_instance *instance)
{
   /* 32-bit seqno for renderer roundtrips */
   static const size_t extra_size = sizeof(uint32_t);
   struct vn_ring_layout layout;
   vn_ring_get_layout(VN_INSTANCE_RING_SIZE, extra_size, &layout);

   instance->ring.ring = vn_ring_create(instance, &layout);
   if (!instance->ring.ring)
      return VK_ERROR_OUT_OF_HOST_MEMORY;

   list_inithead(&instance->ring.tls_rings);

   vn_watchdog_init(&instance->ring.watchdog);

   mtx_init(&instance->ring.roundtrip_mutex, mtx_plain);
   instance->ring.roundtrip_next = 1;

   return VK_SUCCESS;
}

static VkResult
vn_instance_init_renderer(struct vn_instance *instance)
{
   const VkAllocationCallbacks *alloc = &instance->base.base.alloc;

   VkResult result = vn_renderer_create(instance, alloc, &instance->renderer);
   if (result != VK_SUCCESS)
      return result;

   struct vn_renderer_info *renderer_info = &instance->renderer->info;
   uint32_t version = vn_info_wire_format_version();
   if (renderer_info->wire_format_version != version) {
      if (VN_DEBUG(INIT)) {
         vn_log(instance, "wire format version %d != %d",
                renderer_info->wire_format_version, version);
      }
      return VK_ERROR_INITIALIZATION_FAILED;
   }

   version = vn_info_vk_xml_version();
   if (renderer_info->vk_xml_version > version)
      renderer_info->vk_xml_version = version;
   if (renderer_info->vk_xml_version < VN_MIN_RENDERER_VERSION) {
      if (VN_DEBUG(INIT)) {
         vn_log(instance, "vk xml version %d.%d.%d < %d.%d.%d",
                VK_VERSION_MAJOR(renderer_info->vk_xml_version),
                VK_VERSION_MINOR(renderer_info->vk_xml_version),
                VK_VERSION_PATCH(renderer_info->vk_xml_version),
                VK_VERSION_MAJOR(VN_MIN_RENDERER_VERSION),
                VK_VERSION_MINOR(VN_MIN_RENDERER_VERSION),
                VK_VERSION_PATCH(VN_MIN_RENDERER_VERSION));
      }
      return VK_ERROR_INITIALIZATION_FAILED;
   }

   uint32_t spec_version =
      vn_extension_get_spec_version("VK_EXT_command_serialization");
   if (renderer_info->vk_ext_command_serialization_spec_version >
       spec_version) {
      renderer_info->vk_ext_command_serialization_spec_version = spec_version;
   }

   spec_version = vn_extension_get_spec_version("VK_MESA_venus_protocol");
   if (renderer_info->vk_mesa_venus_protocol_spec_version > spec_version)
      renderer_info->vk_mesa_venus_protocol_spec_version = spec_version;

   if (VN_DEBUG(INIT)) {
      vn_log(instance, "connected to renderer");
      vn_log(instance, "wire format version %d",
             renderer_info->wire_format_version);
      vn_log(instance, "vk xml version %d.%d.%d",
             VK_VERSION_MAJOR(renderer_info->vk_xml_version),
             VK_VERSION_MINOR(renderer_info->vk_xml_version),
             VK_VERSION_PATCH(renderer_info->vk_xml_version));
      vn_log(instance, "VK_EXT_command_serialization spec version %d",
             renderer_info->vk_ext_command_serialization_spec_version);
      vn_log(instance, "VK_MESA_venus_protocol spec version %d",
             renderer_info->vk_mesa_venus_protocol_spec_version);
      vn_log(instance, "supports blob id 0: %d",
             renderer_info->supports_blob_id_0);
      vn_log(instance, "allow_vk_wait_syncs: %d",
             renderer_info->allow_vk_wait_syncs);
      vn_log(instance, "supports_multiple_timelines: %d",
             renderer_info->supports_multiple_timelines);
   }

   return VK_SUCCESS;
}

VkResult
vn_instance_submit_roundtrip(struct vn_instance *instance,
                             uint64_t *roundtrip_seqno)
{
   const uint64_t ring_id = vn_ring_get_id(instance->ring.ring);
   uint32_t local_data[8];
   struct vn_cs_encoder local_enc =
      VN_CS_ENCODER_INITIALIZER_LOCAL(local_data, sizeof(local_data));

   mtx_lock(&instance->ring.roundtrip_mutex);
   const uint64_t seqno = instance->ring.roundtrip_next++;
   vn_encode_vkSubmitVirtqueueSeqnoMESA(&local_enc, 0, ring_id, seqno);
   VkResult result = vn_renderer_submit_simple(
      instance->renderer, local_data, vn_cs_encoder_get_len(&local_enc));
   mtx_unlock(&instance->ring.roundtrip_mutex);

   *roundtrip_seqno = seqno;
   return result;
}

void
vn_instance_wait_roundtrip(struct vn_instance *instance,
                           uint64_t roundtrip_seqno)
{
   vn_async_vkWaitVirtqueueSeqnoMESA(instance->ring.ring, roundtrip_seqno);
}

/* instance commands */

VkResult
vn_EnumerateInstanceVersion(uint32_t *pApiVersion)
{
   *pApiVersion = VN_MAX_API_VERSION;
   return VK_SUCCESS;
}

VkResult
vn_EnumerateInstanceExtensionProperties(const char *pLayerName,
                                        uint32_t *pPropertyCount,
                                        VkExtensionProperties *pProperties)
{
   if (pLayerName)
      return vn_error(NULL, VK_ERROR_LAYER_NOT_PRESENT);

   return vk_enumerate_instance_extension_properties(
      &vn_instance_supported_extensions, pPropertyCount, pProperties);
}

VkResult
vn_EnumerateInstanceLayerProperties(uint32_t *pPropertyCount,
                                    VkLayerProperties *pProperties)
{
   *pPropertyCount = 0;
   return VK_SUCCESS;
}

VkResult
vn_CreateInstance(const VkInstanceCreateInfo *pCreateInfo,
                  const VkAllocationCallbacks *pAllocator,
                  VkInstance *pInstance)
{
   vn_trace_init();
   VN_TRACE_FUNC();

   const VkAllocationCallbacks *alloc =
      pAllocator ? pAllocator : vk_default_allocator();
   struct vn_instance *instance;
   VkResult result;

   vn_env_init();

   instance = vk_zalloc(alloc, sizeof(*instance), VN_DEFAULT_ALIGN,
                        VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
   if (!instance)
      return vn_error(NULL, VK_ERROR_OUT_OF_HOST_MEMORY);

   struct vk_instance_dispatch_table dispatch_table;
   vk_instance_dispatch_table_from_entrypoints(
      &dispatch_table, &vn_instance_entrypoints, true);
   vk_instance_dispatch_table_from_entrypoints(
      &dispatch_table, &wsi_instance_entrypoints, false);
   result = vn_instance_base_init(&instance->base,
                                  &vn_instance_supported_extensions,
                                  &dispatch_table, pCreateInfo, alloc);
   if (result != VK_SUCCESS) {
      vk_free(alloc, instance);
      return vn_error(NULL, result);
   }

   /* ring_idx = 0 reserved for CPU timeline */
   instance->ring_idx_used_mask = 0x1;

   mtx_init(&instance->physical_device.mutex, mtx_plain);
   mtx_init(&instance->ring_idx_mutex, mtx_plain);

   if (!vn_icd_supports_api_version(
          instance->base.base.app_info.api_version)) {
      result = VK_ERROR_INCOMPATIBLE_DRIVER;
      goto out_mtx_destroy;
   }

   if (pCreateInfo->enabledLayerCount) {
      result = VK_ERROR_LAYER_NOT_PRESENT;
      goto out_mtx_destroy;
   }

   result = vn_instance_init_renderer(instance);
   if (result != VK_SUCCESS)
      goto out_mtx_destroy;

   vn_cs_renderer_protocol_info_init(instance);

   vn_renderer_shmem_pool_init(instance->renderer, &instance->cs_shmem_pool,
                               8u << 20);

   vn_renderer_shmem_pool_init(instance->renderer,
                               &instance->reply_shmem_pool, 1u << 20);

   result = vn_instance_init_ring(instance);
   if (result != VK_SUCCESS)
      goto out_shmem_pool_fini;

   result = vn_instance_init_renderer_versions(instance);
   if (result != VK_SUCCESS)
      goto out_ring_fini;

   VkInstanceCreateInfo local_create_info = *pCreateInfo;
   local_create_info.ppEnabledExtensionNames = NULL;
   local_create_info.enabledExtensionCount = 0;
   pCreateInfo = &local_create_info;

   VkApplicationInfo local_app_info;
   if (instance->base.base.app_info.api_version <
       instance->renderer_api_version) {
      if (pCreateInfo->pApplicationInfo) {
         local_app_info = *pCreateInfo->pApplicationInfo;
         local_app_info.apiVersion = instance->renderer_api_version;
      } else {
         local_app_info = (const VkApplicationInfo){
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .apiVersion = instance->renderer_api_version,
         };
      }
      local_create_info.pApplicationInfo = &local_app_info;
   }

   VkInstance instance_handle = vn_instance_to_handle(instance);
   result = vn_call_vkCreateInstance(instance->ring.ring, pCreateInfo, NULL,
                                     &instance_handle);
   if (result != VK_SUCCESS)
      goto out_ring_fini;

   driParseOptionInfo(&instance->available_dri_options, vn_dri_options,
                      ARRAY_SIZE(vn_dri_options));
   driParseConfigFiles(&instance->dri_options,
                       &instance->available_dri_options, 0, "venus", NULL,
                       NULL, instance->base.base.app_info.app_name,
                       instance->base.base.app_info.app_version,
                       instance->base.base.app_info.engine_name,
                       instance->base.base.app_info.engine_version);

   instance->renderer->info.has_implicit_fencing =
      driQueryOptionb(&instance->dri_options, "venus_implicit_fencing");
   instance->enable_wsi_multi_plane_modifiers = driQueryOptionb(
      &instance->dri_options, "venus_wsi_multi_plane_modifiers");

   if (VN_DEBUG(INIT)) {
      vn_log(instance, "supports multi-plane wsi format modifiers: %s",
             instance->enable_wsi_multi_plane_modifiers ? "yes" : "no");
   }

   *pInstance = instance_handle;

   return VK_SUCCESS;

out_ring_fini:
   vn_instance_fini_ring(instance);

out_shmem_pool_fini:
   vn_renderer_shmem_pool_fini(instance->renderer,
                               &instance->reply_shmem_pool);
   vn_renderer_shmem_pool_fini(instance->renderer, &instance->cs_shmem_pool);
   vn_renderer_destroy(instance->renderer, alloc);

out_mtx_destroy:
   mtx_destroy(&instance->physical_device.mutex);
   mtx_destroy(&instance->ring_idx_mutex);

   vn_instance_base_fini(&instance->base);
   vk_free(alloc, instance);

   return vn_error(NULL, result);
}

void
vn_DestroyInstance(VkInstance _instance,
                   const VkAllocationCallbacks *pAllocator)
{
   VN_TRACE_FUNC();
   struct vn_instance *instance = vn_instance_from_handle(_instance);
   const VkAllocationCallbacks *alloc =
      pAllocator ? pAllocator : &instance->base.base.alloc;

   if (!instance)
      return;

   if (instance->physical_device.initialized) {
      for (uint32_t i = 0; i < instance->physical_device.device_count; i++)
         vn_physical_device_fini(&instance->physical_device.devices[i]);
      vk_free(alloc, instance->physical_device.devices);
      vk_free(alloc, instance->physical_device.groups);
   }
   mtx_destroy(&instance->physical_device.mutex);
   mtx_destroy(&instance->ring_idx_mutex);

   vn_call_vkDestroyInstance(instance->ring.ring, _instance, NULL);

   vn_instance_fini_ring(instance);

   vn_renderer_shmem_pool_fini(instance->renderer,
                               &instance->reply_shmem_pool);

   vn_renderer_shmem_pool_fini(instance->renderer, &instance->cs_shmem_pool);

   vn_renderer_destroy(instance->renderer, alloc);

   driDestroyOptionCache(&instance->dri_options);
   driDestroyOptionInfo(&instance->available_dri_options);

   vn_instance_base_fini(&instance->base);
   vk_free(alloc, instance);
}

PFN_vkVoidFunction
vn_GetInstanceProcAddr(VkInstance _instance, const char *pName)
{
   struct vn_instance *instance = vn_instance_from_handle(_instance);
   return vk_instance_get_proc_addr(&instance->base.base,
                                    &vn_instance_entrypoints, pName);
}
