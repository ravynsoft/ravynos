/*
 * Copyright © 2022 Collabora Ltd. and Red Hat Inc.
 * SPDX-License-Identifier: MIT
 */
#include "nvk_instance.h"

#include "nvk_entrypoints.h"
#include "nvk_physical_device.h"

#include "vulkan/wsi/wsi_common.h"

#include "util/build_id.h"
#include "util/driconf.h"
#include "util/mesa-sha1.h"

VKAPI_ATTR VkResult VKAPI_CALL
nvk_EnumerateInstanceVersion(uint32_t *pApiVersion)
{
   uint32_t version_override = vk_get_version_override();
   *pApiVersion = version_override ? version_override :
                  VK_MAKE_VERSION(1, 1, VK_HEADER_VERSION);

   return VK_SUCCESS;
}

static const struct vk_instance_extension_table instance_extensions = {
#ifdef NVK_USE_WSI_PLATFORM
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
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
   .EXT_acquire_xlib_display = true,
#endif
   .KHR_device_group_creation = true,
   .KHR_external_fence_capabilities = true,
   .KHR_external_memory_capabilities = true,
   .KHR_external_semaphore_capabilities = true,
   .KHR_get_physical_device_properties2 = true,
   .EXT_debug_report = true,
   .EXT_debug_utils = true,
};

VKAPI_ATTR VkResult VKAPI_CALL
nvk_EnumerateInstanceExtensionProperties(const char *pLayerName,
                                         uint32_t *pPropertyCount,
                                         VkExtensionProperties *pProperties)
{
   if (pLayerName)
      return vk_error(NULL, VK_ERROR_LAYER_NOT_PRESENT);

   return vk_enumerate_instance_extension_properties(
      &instance_extensions, pPropertyCount, pProperties);
}

static const driOptionDescription nvk_dri_options[] = {
   DRI_CONF_SECTION_PERFORMANCE
      DRI_CONF_ADAPTIVE_SYNC(true)
      DRI_CONF_VK_X11_OVERRIDE_MIN_IMAGE_COUNT(0)
      DRI_CONF_VK_X11_STRICT_IMAGE_COUNT(false)
      DRI_CONF_VK_X11_ENSURE_MIN_IMAGE_COUNT(false)
      DRI_CONF_VK_KHR_PRESENT_WAIT(false)
      DRI_CONF_VK_XWAYLAND_WAIT_READY(true)
   DRI_CONF_SECTION_END
};

static void
nvk_init_dri_options(struct nvk_instance *instance)
{
   driParseOptionInfo(&instance->available_dri_options, nvk_dri_options, ARRAY_SIZE(nvk_dri_options));
   driParseConfigFiles(&instance->dri_options, &instance->available_dri_options, 0, "nvk", NULL, NULL,
                       instance->vk.app_info.app_name, instance->vk.app_info.app_version,
                       instance->vk.app_info.engine_name, instance->vk.app_info.engine_version);
}

VKAPI_ATTR VkResult VKAPI_CALL
nvk_CreateInstance(const VkInstanceCreateInfo *pCreateInfo,
                   const VkAllocationCallbacks *pAllocator,
                   VkInstance *pInstance)
{
   struct nvk_instance *instance;
   VkResult result;

   if (pAllocator == NULL)
      pAllocator = vk_default_allocator();

   instance = vk_alloc(pAllocator, sizeof(*instance), 8,
                       VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
   if (!instance)
      return vk_error(NULL, VK_ERROR_OUT_OF_HOST_MEMORY);

   struct vk_instance_dispatch_table dispatch_table;
   vk_instance_dispatch_table_from_entrypoints(&dispatch_table,
                                               &nvk_instance_entrypoints,
                                               true);
   vk_instance_dispatch_table_from_entrypoints(&dispatch_table,
                                               &wsi_instance_entrypoints,
                                               false);

   result = vk_instance_init(&instance->vk, &instance_extensions,
                             &dispatch_table, pCreateInfo, pAllocator);
   if (result != VK_SUCCESS)
      goto fail_alloc;

   nvk_init_dri_options(instance);

   instance->vk.physical_devices.try_create_for_drm =
      nvk_create_drm_physical_device;
   instance->vk.physical_devices.destroy = nvk_physical_device_destroy;

   const struct build_id_note *note =
      build_id_find_nhdr_for_addr(nvk_CreateInstance);
   if (!note) {
      result = vk_errorf(NULL, VK_ERROR_INITIALIZATION_FAILED,
                         "Failed to find build-id");
      goto fail_init;
   }

   unsigned build_id_len = build_id_length(note);
   if (build_id_len < SHA1_DIGEST_LENGTH) {
      result = vk_errorf(NULL, VK_ERROR_INITIALIZATION_FAILED,
                        "build-id too short.  It needs to be a SHA");
      goto fail_init;
   }

   STATIC_ASSERT(sizeof(instance->driver_build_sha) == SHA1_DIGEST_LENGTH);
   memcpy(instance->driver_build_sha, build_id_data(note), SHA1_DIGEST_LENGTH);

   *pInstance = nvk_instance_to_handle(instance);
   return VK_SUCCESS;

fail_init:
   vk_instance_finish(&instance->vk);
fail_alloc:
   vk_free(pAllocator, instance);

   return result;
}

VKAPI_ATTR void VKAPI_CALL
nvk_DestroyInstance(VkInstance _instance,
                    const VkAllocationCallbacks *pAllocator)
{
   VK_FROM_HANDLE(nvk_instance, instance, _instance);

   if (!instance)
      return;

   driDestroyOptionCache(&instance->dri_options);
   driDestroyOptionInfo(&instance->available_dri_options);

   vk_instance_finish(&instance->vk);
   vk_free(&instance->vk.alloc, instance);
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
nvk_GetInstanceProcAddr(VkInstance _instance, const char *pName)
{
   VK_FROM_HANDLE(nvk_instance, instance, _instance);
   return vk_instance_get_proc_addr(&instance->vk,
                                    &nvk_instance_entrypoints,
                                    pName);
}

PUBLIC VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
vk_icdGetInstanceProcAddr(VkInstance instance, const char *pName)
{
   return nvk_GetInstanceProcAddr(instance, pName);
}
