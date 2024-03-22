/*
 * Copyright 2019 Google LLC
 * SPDX-License-Identifier: MIT
 *
 * based in part on anv and radv which are:
 * Copyright © 2015 Intel Corporation
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 */

#ifndef VN_PHYSICAL_DEVICE_H
#define VN_PHYSICAL_DEVICE_H

#include "vn_common.h"

#include "util/sparse_array.h"

#include "vn_wsi.h"

struct vn_physical_device_properties {
   VkPhysicalDeviceProperties vulkan_1_0;
   VkPhysicalDeviceVulkan11Properties vulkan_1_1;
   VkPhysicalDeviceVulkan12Properties vulkan_1_2;
   VkPhysicalDeviceVulkan13Properties vulkan_1_3;

   /* KHR */
   VkPhysicalDevicePushDescriptorPropertiesKHR push_descriptor;

   /* EXT */
   VkPhysicalDeviceConservativeRasterizationPropertiesEXT
      conservative_rasterization;
   VkPhysicalDeviceCustomBorderColorPropertiesEXT custom_border_color;
   VkPhysicalDeviceExtendedDynamicState3PropertiesEXT extended_dynamic_state_3;
   VkPhysicalDeviceGraphicsPipelineLibraryPropertiesEXT
      graphics_pipeline_library;
   VkPhysicalDeviceLineRasterizationPropertiesEXT line_rasterization;
   VkPhysicalDeviceMultiDrawPropertiesEXT multi_draw;
   VkPhysicalDevicePCIBusInfoPropertiesEXT pci_bus_info;
   VkPhysicalDeviceProvokingVertexPropertiesEXT provoking_vertex;
   VkPhysicalDeviceRobustness2PropertiesEXT robustness_2;
   VkPhysicalDeviceTransformFeedbackPropertiesEXT transform_feedback;
   VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT
      vertex_attribute_divisor;
};

struct vn_format_properties_entry {
   atomic_bool valid;
   VkFormatProperties properties;
};

struct vn_physical_device {
   struct vn_physical_device_base base;

   struct vn_instance *instance;

   /* Between the driver and the app, properties.properties.apiVersion is what
    * we advertise and is capped by VN_MAX_API_VERSION and others.
    *
    * Between the driver and the renderer, renderer_version is the device
    * version we can use internally.
    */
   uint32_t renderer_version;

   /* Between the driver and the app, base.base.supported_extensions is what
    * we advertise.
    *
    * Between the driver and the renderer, renderer_extensions is what we can
    * use internally (after enabling).
    */
   struct vk_device_extension_table renderer_extensions;
   uint32_t *extension_spec_versions;

   struct vn_physical_device_properties properties;
   enum VkDriverId renderer_driver_id;

   VkQueueFamilyProperties2 *queue_family_properties;
   uint32_t queue_family_count;
   bool sparse_binding_disabled;

   VkPhysicalDeviceMemoryProperties memory_properties;
   uint32_t coherent_uncached;
   uint32_t incoherent_cached;

   struct {
      VkExternalMemoryHandleTypeFlagBits renderer_handle_type;
      VkExternalMemoryHandleTypeFlags supported_handle_types;
   } external_memory;

   struct {
      bool fence_exportable;
      bool semaphore_exportable;
      bool semaphore_importable;
   } renderer_sync_fd;

   VkExternalFenceHandleTypeFlags external_fence_handles;
   VkExternalSemaphoreHandleTypeFlags external_binary_semaphore_handles;
   VkExternalSemaphoreHandleTypeFlags external_timeline_semaphore_handles;

   struct wsi_device wsi_device;

   simple_mtx_t format_update_mutex;
   struct util_sparse_array format_properties;
};
VK_DEFINE_HANDLE_CASTS(vn_physical_device,
                       base.base.base,
                       VkPhysicalDevice,
                       VK_OBJECT_TYPE_PHYSICAL_DEVICE)

void
vn_physical_device_fini(struct vn_physical_device *physical_dev);

#endif /* VN_PHYSICAL_DEVICE_H */
