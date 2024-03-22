/*
 * Copyright © 2021 Intel Corporation
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "vk_physical_device.h"

#include "vk_common_entrypoints.h"
#include "vk_util.h"

VkResult
vk_physical_device_init(struct vk_physical_device *pdevice,
                        struct vk_instance *instance,
                        const struct vk_device_extension_table *supported_extensions,
                        const struct vk_features *supported_features,
                        const struct vk_properties *properties,
                        const struct vk_physical_device_dispatch_table *dispatch_table)
{
   memset(pdevice, 0, sizeof(*pdevice));
   vk_object_base_instance_init(instance, &pdevice->base, VK_OBJECT_TYPE_PHYSICAL_DEVICE);
   pdevice->instance = instance;

   if (supported_extensions != NULL)
      pdevice->supported_extensions = *supported_extensions;

   if (supported_features != NULL)
      pdevice->supported_features = *supported_features;

   if (properties != NULL)
      pdevice->properties = *properties;

   pdevice->dispatch_table = *dispatch_table;

   /* Add common entrypoints without overwriting driver-provided ones. */
   vk_physical_device_dispatch_table_from_entrypoints(
      &pdevice->dispatch_table, &vk_common_physical_device_entrypoints, false);

   /* TODO */
   pdevice->disk_cache = NULL;

   return VK_SUCCESS;
}

void
vk_physical_device_finish(struct vk_physical_device *physical_device)
{
   vk_object_base_finish(&physical_device->base);
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_common_EnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice,
                                         uint32_t *pPropertyCount,
                                         VkLayerProperties *pProperties)
{
   if (pProperties == NULL) {
      *pPropertyCount = 0;
      return VK_SUCCESS;
   }

   /* None supported at this time */
   return VK_ERROR_LAYER_NOT_PRESENT;
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_common_EnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice,
                                             const char *pLayerName,
                                             uint32_t *pPropertyCount,
                                             VkExtensionProperties *pProperties)
{
   VK_FROM_HANDLE(vk_physical_device, pdevice, physicalDevice);
   VK_OUTARRAY_MAKE_TYPED(VkExtensionProperties, out, pProperties, pPropertyCount);

   for (int i = 0; i < VK_DEVICE_EXTENSION_COUNT; i++) {
      if (!pdevice->supported_extensions.extensions[i])
         continue;

#ifdef ANDROID_STRICT
      if (!vk_android_allowed_device_extensions.extensions[i])
         continue;
#endif

      vk_outarray_append_typed(VkExtensionProperties, &out, prop) {
         *prop = vk_device_extensions[i];
      }
   }

   return vk_outarray_status(&out);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_GetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice,
                                    VkPhysicalDeviceFeatures *pFeatures)
{
   VK_FROM_HANDLE(vk_physical_device, pdevice, physicalDevice);

   /* Don't zero-init this struct since the driver fills it out entirely */
   VkPhysicalDeviceFeatures2 features2;
   features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
   features2.pNext = NULL;

   pdevice->dispatch_table.GetPhysicalDeviceFeatures2(physicalDevice,
                                                      &features2);
   *pFeatures = features2.features;
}

VKAPI_ATTR void VKAPI_CALL
vk_common_GetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice,
                                      VkPhysicalDeviceProperties *pProperties)
{
   VK_FROM_HANDLE(vk_physical_device, pdevice, physicalDevice);

   /* Don't zero-init this struct since the driver fills it out entirely */
   VkPhysicalDeviceProperties2 props2;
   props2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
   props2.pNext = NULL;

   pdevice->dispatch_table.GetPhysicalDeviceProperties2(physicalDevice,
                                                        &props2);
   *pProperties = props2.properties;
}

VKAPI_ATTR void VKAPI_CALL
vk_common_GetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice,
                                                 uint32_t *pQueueFamilyPropertyCount,
                                                 VkQueueFamilyProperties *pQueueFamilyProperties)
{
   VK_FROM_HANDLE(vk_physical_device, pdevice, physicalDevice);

   if (!pQueueFamilyProperties) {
      pdevice->dispatch_table.GetPhysicalDeviceQueueFamilyProperties2(physicalDevice,
                                                                      pQueueFamilyPropertyCount,
                                                                      NULL);
      return;
   }

   STACK_ARRAY(VkQueueFamilyProperties2, props2, *pQueueFamilyPropertyCount);

   for (unsigned i = 0; i < *pQueueFamilyPropertyCount; ++i) {
      props2[i].sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;
      props2[i].pNext = NULL;
   }

   pdevice->dispatch_table.GetPhysicalDeviceQueueFamilyProperties2(physicalDevice,
                                                                   pQueueFamilyPropertyCount,
                                                                   props2);

   for (unsigned i = 0; i < *pQueueFamilyPropertyCount; ++i)
      pQueueFamilyProperties[i] = props2[i].queueFamilyProperties;

   STACK_ARRAY_FINISH(props2);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_GetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice,
                                            VkPhysicalDeviceMemoryProperties *pMemoryProperties)
{
   VK_FROM_HANDLE(vk_physical_device, pdevice, physicalDevice);

   /* Don't zero-init this struct since the driver fills it out entirely */
   VkPhysicalDeviceMemoryProperties2 props2;
   props2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
   props2.pNext = NULL;

   pdevice->dispatch_table.GetPhysicalDeviceMemoryProperties2(physicalDevice,
                                                              &props2);
   *pMemoryProperties = props2.memoryProperties;
}

VKAPI_ATTR void VKAPI_CALL
vk_common_GetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice,
                                            VkFormat format,
                                            VkFormatProperties *pFormatProperties)
{
   VK_FROM_HANDLE(vk_physical_device, pdevice, physicalDevice);

   /* Don't zero-init this struct since the driver fills it out entirely */
   VkFormatProperties2 props2;
   props2.sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2;
   props2.pNext = NULL;

   pdevice->dispatch_table.GetPhysicalDeviceFormatProperties2(physicalDevice,
                                                              format, &props2);
   *pFormatProperties = props2.formatProperties;
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_common_GetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice,
                                                 VkFormat format,
                                                 VkImageType type,
                                                 VkImageTiling tiling,
                                                 VkImageUsageFlags usage,
                                                 VkImageCreateFlags flags,
                                                 VkImageFormatProperties *pImageFormatProperties)
{
   VK_FROM_HANDLE(vk_physical_device, pdevice, physicalDevice);

   VkPhysicalDeviceImageFormatInfo2 info = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2,
      .format = format,
      .type = type,
      .tiling = tiling,
      .usage = usage,
      .flags = flags
   };

   /* Don't zero-init this struct since the driver fills it out entirely */
   VkImageFormatProperties2 props2;
   props2.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2;
   props2.pNext = NULL;

   VkResult result =
      pdevice->dispatch_table.GetPhysicalDeviceImageFormatProperties2(physicalDevice,
                                                                      &info, &props2);
   *pImageFormatProperties = props2.imageFormatProperties;

   return result;
}

VKAPI_ATTR void VKAPI_CALL
vk_common_GetPhysicalDeviceSparseImageFormatProperties(VkPhysicalDevice physicalDevice,
                                                       VkFormat format,
                                                       VkImageType type,
                                                       VkSampleCountFlagBits samples,
                                                       VkImageUsageFlags usage,
                                                       VkImageTiling tiling,
                                                       uint32_t *pNumProperties,
                                                       VkSparseImageFormatProperties *pProperties)
{
   VK_FROM_HANDLE(vk_physical_device, pdevice, physicalDevice);

   VkPhysicalDeviceSparseImageFormatInfo2 info = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SPARSE_IMAGE_FORMAT_INFO_2,
      .format = format,
      .type = type,
      .samples = samples,
      .usage = usage,
      .tiling = tiling
   };

   if (!pProperties) {
      pdevice->dispatch_table.GetPhysicalDeviceSparseImageFormatProperties2(physicalDevice,
                                                                            &info,
                                                                            pNumProperties,
                                                                            NULL);
      return;
   }

   STACK_ARRAY(VkSparseImageFormatProperties2, props2, *pNumProperties);

   for (unsigned i = 0; i < *pNumProperties; ++i) {
      props2[i].sType = VK_STRUCTURE_TYPE_SPARSE_IMAGE_FORMAT_PROPERTIES_2;
      props2[i].pNext = NULL;
   }

   pdevice->dispatch_table.GetPhysicalDeviceSparseImageFormatProperties2(physicalDevice,
                                                                         &info,
                                                                         pNumProperties,
                                                                         props2);

   for (unsigned i = 0; i < *pNumProperties; ++i)
      pProperties[i] = props2[i].properties;

   STACK_ARRAY_FINISH(props2);
}

/* VK_EXT_tooling_info */
VKAPI_ATTR VkResult VKAPI_CALL
vk_common_GetPhysicalDeviceToolProperties(VkPhysicalDevice physicalDevice,
                                          uint32_t *pToolCount,
                                          VkPhysicalDeviceToolProperties *pToolProperties)
{
   VK_OUTARRAY_MAKE_TYPED(VkPhysicalDeviceToolProperties, out, pToolProperties, pToolCount);

   return vk_outarray_status(&out);
}
