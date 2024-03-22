/*
 * Copyright © 2017, Google Inc.
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

#ifdef ANDROID
#include <libsync.h>
#include <hardware/gralloc.h>
#include <hardware/hardware.h>
#include <hardware/hwvulkan.h>
#include <vulkan/vk_android_native_buffer.h>
#include <vulkan/vk_icd.h>

#if ANDROID_API_LEVEL >= 26
#include <hardware/gralloc1.h>
#endif
#endif

#include "util/os_file.h"

#include "radv_private.h"
#include "vk_android.h"
#include "vk_util.h"

#ifdef ANDROID

static int radv_hal_open(const struct hw_module_t *mod, const char *id, struct hw_device_t **dev);
static int radv_hal_close(struct hw_device_t *dev);

static_assert(HWVULKAN_DISPATCH_MAGIC == ICD_LOADER_MAGIC, "");

PUBLIC struct hwvulkan_module_t HAL_MODULE_INFO_SYM = {
   .common =
      {
         .tag = HARDWARE_MODULE_TAG,
         .module_api_version = HWVULKAN_MODULE_API_VERSION_0_1,
         .hal_api_version = HARDWARE_MAKE_API_VERSION(1, 0),
         .id = HWVULKAN_HARDWARE_MODULE_ID,
         .name = "AMD Vulkan HAL",
         .author = "Google",
         .methods =
            &(hw_module_methods_t){
               .open = radv_hal_open,
            },
      },
};

/* If any bits in test_mask are set, then unset them and return true. */
static inline bool
unmask32(uint32_t *inout_mask, uint32_t test_mask)
{
   uint32_t orig_mask = *inout_mask;
   *inout_mask &= ~test_mask;
   return *inout_mask != orig_mask;
}

static int
radv_hal_open(const struct hw_module_t *mod, const char *id, struct hw_device_t **dev)
{
   assert(mod == &HAL_MODULE_INFO_SYM.common);
   assert(strcmp(id, HWVULKAN_DEVICE_0) == 0);

   hwvulkan_device_t *hal_dev = malloc(sizeof(*hal_dev));
   if (!hal_dev)
      return -1;

   *hal_dev = (hwvulkan_device_t){
      .common =
         {
            .tag = HARDWARE_DEVICE_TAG,
            .version = HWVULKAN_DEVICE_API_VERSION_0_1,
            .module = &HAL_MODULE_INFO_SYM.common,
            .close = radv_hal_close,
         },
      .EnumerateInstanceExtensionProperties = radv_EnumerateInstanceExtensionProperties,
      .CreateInstance = radv_CreateInstance,
      .GetInstanceProcAddr = radv_GetInstanceProcAddr,
   };

   *dev = &hal_dev->common;
   return 0;
}

static int
radv_hal_close(struct hw_device_t *dev)
{
   /* hwvulkan.h claims that hw_device_t::close() is never called. */
   return -1;
}

VkResult
radv_image_from_gralloc(VkDevice device_h, const VkImageCreateInfo *base_info,
                        const VkNativeBufferANDROID *gralloc_info, const VkAllocationCallbacks *alloc,
                        VkImage *out_image_h)

{
   RADV_FROM_HANDLE(radv_device, device, device_h);
   VkImage image_h = VK_NULL_HANDLE;
   struct radv_image *image = NULL;
   VkResult result;

   if (gralloc_info->handle->numFds != 1) {
      return vk_errorf(device, VK_ERROR_INVALID_EXTERNAL_HANDLE,
                       "VkNativeBufferANDROID::handle::numFds is %d, "
                       "expected 1",
                       gralloc_info->handle->numFds);
   }

   /* Do not close the gralloc handle's dma_buf. The lifetime of the dma_buf
    * must exceed that of the gralloc handle, and we do not own the gralloc
    * handle.
    */
   int dma_buf = gralloc_info->handle->data[0];

   VkDeviceMemory memory_h;

   const VkImportMemoryFdInfoKHR import_info = {
      .sType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR,
      .handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT,
      .fd = os_dupfd_cloexec(dma_buf),
   };

   /* Find the first VRAM memory type, or GART for PRIME images. */
   int memory_type_index = -1;
   for (int i = 0; i < device->physical_device->memory_properties.memoryTypeCount; ++i) {
      bool is_local = !!(device->physical_device->memory_properties.memoryTypes[i].propertyFlags &
                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
      bool is_32bit = !!(device->physical_device->memory_types_32bit & (1u << i));
      if (is_local && !is_32bit) {
         memory_type_index = i;
         break;
      }
   }

   /* fallback */
   if (memory_type_index == -1)
      memory_type_index = 0;

   result = radv_AllocateMemory(device_h,
                                &(VkMemoryAllocateInfo){
                                   .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                                   .pNext = &import_info,
                                   /* Max buffer size, unused for imports */
                                   .allocationSize = 0x7FFFFFFF,
                                   .memoryTypeIndex = memory_type_index,
                                },
                                alloc, &memory_h);
   if (result != VK_SUCCESS)
      return result;

   struct radeon_bo_metadata md;
   device->ws->buffer_get_metadata(device->ws, radv_device_memory_from_handle(memory_h)->bo, &md);

   VkImageCreateInfo updated_base_info = *base_info;

   VkExternalMemoryImageCreateInfo external_memory_info = {
      .sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO,
      .pNext = updated_base_info.pNext,
      .handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT,
   };

   updated_base_info.pNext = &external_memory_info;

   result = radv_image_create(device_h,
                              &(struct radv_image_create_info){
                                 .vk_info = &updated_base_info,
                                 .no_metadata_planes = true,
                                 .bo_metadata = &md,
                              },
                              alloc, &image_h, false);

   if (result != VK_SUCCESS)
      goto fail_create_image;

   image = radv_image_from_handle(image_h);

   radv_image_override_offset_stride(device, image, 0, gralloc_info->stride);

   VkBindImageMemoryInfo bind_info = {.sType = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO,
                                      .image = image_h,
                                      .memory = memory_h,
                                      .memoryOffset = 0};
   radv_BindImageMemory2(device_h, 1, &bind_info);

   image->owned_memory = memory_h;
   /* Don't clobber the out-parameter until success is certain. */
   *out_image_h = image_h;

   return VK_SUCCESS;

fail_create_image:
   radv_FreeMemory(device_h, memory_h, alloc);
   return result;
}

VkResult
radv_GetSwapchainGrallocUsageANDROID(VkDevice device_h, VkFormat format, VkImageUsageFlags imageUsage,
                                     int *grallocUsage)
{
   RADV_FROM_HANDLE(radv_device, device, device_h);
   struct radv_physical_device *phys_dev = device->physical_device;
   VkPhysicalDevice phys_dev_h = radv_physical_device_to_handle(phys_dev);
   VkResult result;

   *grallocUsage = 0;

   /* WARNING: Android Nougat's libvulkan.so hardcodes the VkImageUsageFlags
    * returned to applications via VkSurfaceCapabilitiesKHR::supportedUsageFlags.
    * The relevant code in libvulkan/swapchain.cpp contains this fun comment:
    *
    *     TODO(jessehall): I think these are right, but haven't thought hard
    *     about it. Do we need to query the driver for support of any of
    *     these?
    *
    * Any disagreement between this function and the hardcoded
    * VkSurfaceCapabilitiesKHR:supportedUsageFlags causes tests
    * dEQP-VK.wsi.android.swapchain.*.image_usage to fail.
    */

   const VkPhysicalDeviceImageFormatInfo2 image_format_info = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2,
      .format = format,
      .type = VK_IMAGE_TYPE_2D,
      .tiling = VK_IMAGE_TILING_OPTIMAL,
      .usage = imageUsage,
   };

   VkImageFormatProperties2 image_format_props = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2,
   };

   /* Check that requested format and usage are supported. */
   result = radv_GetPhysicalDeviceImageFormatProperties2(phys_dev_h, &image_format_info, &image_format_props);
   if (result != VK_SUCCESS) {
      return vk_errorf(device, result,
                       "radv_GetPhysicalDeviceImageFormatProperties2 failed "
                       "inside %s",
                       __func__);
   }

   if (unmask32(&imageUsage, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT))
      *grallocUsage |= GRALLOC_USAGE_HW_RENDER;

   if (unmask32(&imageUsage, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT |
                                VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT))
      *grallocUsage |= GRALLOC_USAGE_HW_TEXTURE;

   /* All VkImageUsageFlags not explicitly checked here are unsupported for
    * gralloc swapchains.
    */
   if (imageUsage != 0) {
      return vk_errorf(device, VK_ERROR_FORMAT_NOT_SUPPORTED,
                       "unsupported VkImageUsageFlags(0x%x) for gralloc "
                       "swapchain",
                       imageUsage);
   }

   /*
    * FINISHME: Advertise all display-supported formats. Mostly
    * DRM_FORMAT_ARGB2101010 and DRM_FORMAT_ABGR2101010, but need to check
    * what we need for 30-bit colors.
    */
   if (format == VK_FORMAT_B8G8R8A8_UNORM || format == VK_FORMAT_B5G6R5_UNORM_PACK16) {
      *grallocUsage |= GRALLOC_USAGE_HW_FB | GRALLOC_USAGE_HW_COMPOSER | GRALLOC_USAGE_EXTERNAL_DISP;
   }

   if (*grallocUsage == 0)
      return VK_ERROR_FORMAT_NOT_SUPPORTED;

   return VK_SUCCESS;
}

VkResult
radv_GetSwapchainGrallocUsage2ANDROID(VkDevice device_h, VkFormat format, VkImageUsageFlags imageUsage,
                                      VkSwapchainImageUsageFlagsANDROID swapchainImageUsage,
                                      uint64_t *grallocConsumerUsage, uint64_t *grallocProducerUsage)
{
   /* Before level 26 (Android 8.0/Oreo) the loader uses
    * vkGetSwapchainGrallocUsageANDROID. */
#if ANDROID_API_LEVEL >= 26
   RADV_FROM_HANDLE(radv_device, device, device_h);
   struct radv_physical_device *phys_dev = device->physical_device;
   VkPhysicalDevice phys_dev_h = radv_physical_device_to_handle(phys_dev);
   VkResult result;

   *grallocConsumerUsage = 0;
   *grallocProducerUsage = 0;

   if (swapchainImageUsage & VK_SWAPCHAIN_IMAGE_USAGE_SHARED_BIT_ANDROID)
      return vk_errorf(device, VK_ERROR_FORMAT_NOT_SUPPORTED,
                       "The Vulkan loader tried to query shared presentable image support");

   const VkPhysicalDeviceImageFormatInfo2 image_format_info = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2,
      .format = format,
      .type = VK_IMAGE_TYPE_2D,
      .tiling = VK_IMAGE_TILING_OPTIMAL,
      .usage = imageUsage,
   };

   VkImageFormatProperties2 image_format_props = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2,
   };

   /* Check that requested format and usage are supported. */
   result = radv_GetPhysicalDeviceImageFormatProperties2(phys_dev_h, &image_format_info, &image_format_props);
   if (result != VK_SUCCESS) {
      return vk_errorf(device, result,
                       "radv_GetPhysicalDeviceImageFormatProperties2 failed "
                       "inside %s",
                       __func__);
   }

   if (unmask32(&imageUsage, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)) {
      *grallocProducerUsage |= GRALLOC1_PRODUCER_USAGE_GPU_RENDER_TARGET;
      *grallocConsumerUsage |= GRALLOC1_CONSUMER_USAGE_CLIENT_TARGET;
   }

   if (unmask32(&imageUsage, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT |
                                VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)) {
      *grallocConsumerUsage |= GRALLOC1_CONSUMER_USAGE_GPU_TEXTURE;
   }

   if (imageUsage != 0) {
      return vk_errorf(device, VK_ERROR_FORMAT_NOT_SUPPORTED,
                       "unsupported VkImageUsageFlags(0x%x) for gralloc "
                       "swapchain",
                       imageUsage);
   }

   /*
    * FINISHME: Advertise all display-supported formats. Mostly
    * DRM_FORMAT_ARGB2101010 and DRM_FORMAT_ABGR2101010, but need to check
    * what we need for 30-bit colors.
    */
   if (format == VK_FORMAT_B8G8R8A8_UNORM || format == VK_FORMAT_B5G6R5_UNORM_PACK16) {
      *grallocProducerUsage |= GRALLOC1_PRODUCER_USAGE_GPU_RENDER_TARGET;
      *grallocConsumerUsage |= GRALLOC1_CONSUMER_USAGE_HWCOMPOSER;
   }

   if (!*grallocProducerUsage && !*grallocConsumerUsage)
      return VK_ERROR_FORMAT_NOT_SUPPORTED;

   return VK_SUCCESS;
#else
   *grallocConsumerUsage = 0;
   *grallocProducerUsage = 0;
   return VK_ERROR_FORMAT_NOT_SUPPORTED;
#endif
}
#endif

#if RADV_SUPPORT_ANDROID_HARDWARE_BUFFER

enum {
   /* Usage bit equal to GRALLOC_USAGE_HW_CAMERA_MASK */
   BUFFER_USAGE_CAMERA_MASK = 0x00060000U,
};

static inline VkFormat
vk_format_from_android(unsigned android_format, unsigned android_usage)
{
   switch (android_format) {
   case AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420:
      return VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
   case AHARDWAREBUFFER_FORMAT_IMPLEMENTATION_DEFINED:
      if (android_usage & BUFFER_USAGE_CAMERA_MASK)
         return VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
      else
         return VK_FORMAT_R8G8B8_UNORM;
   default:
      return vk_ahb_format_to_image_format(android_format);
   }
}

unsigned
radv_ahb_format_for_vk_format(VkFormat vk_format)
{
   switch (vk_format) {
   case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
      return AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420;
   default:
      return vk_image_format_to_ahb_format(vk_format);
   }
}

static VkResult
get_ahb_buffer_format_properties(VkDevice device_h, const struct AHardwareBuffer *buffer,
                                 VkAndroidHardwareBufferFormatPropertiesANDROID *pProperties)
{
   RADV_FROM_HANDLE(radv_device, device, device_h);

   /* Get a description of buffer contents . */
   AHardwareBuffer_Desc desc;
   AHardwareBuffer_describe(buffer, &desc);

   /* Verify description. */
   const uint64_t gpu_usage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE | AHARDWAREBUFFER_USAGE_GPU_COLOR_OUTPUT |
                              AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER;

   /* "Buffer must be a valid Android hardware buffer object with at least
    * one of the AHARDWAREBUFFER_USAGE_GPU_* usage flags."
    */
   if (!(desc.usage & (gpu_usage)))
      return VK_ERROR_INVALID_EXTERNAL_HANDLE;

   /* Fill properties fields based on description. */
   VkAndroidHardwareBufferFormatPropertiesANDROID *p = pProperties;

   p->format = vk_format_from_android(desc.format, desc.usage);
   p->externalFormat = (uint64_t)(uintptr_t)p->format;

   VkFormatProperties2 format_properties = {.sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2};

   radv_GetPhysicalDeviceFormatProperties2(radv_physical_device_to_handle(device->physical_device), p->format,
                                           &format_properties);

   if (desc.usage & AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER)
      p->formatFeatures = format_properties.formatProperties.linearTilingFeatures;
   else
      p->formatFeatures = format_properties.formatProperties.optimalTilingFeatures;

   /* "Images can be created with an external format even if the Android hardware
    *  buffer has a format which has an equivalent Vulkan format to enable
    *  consistent handling of images from sources that might use either category
    *  of format. However, all images created with an external format are subject
    *  to the valid usage requirements associated with external formats, even if
    *  the Android hardware buffer’s format has a Vulkan equivalent."
    *
    * "The formatFeatures member *must* include
    *  VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT and at least one of
    *  VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT or
    *  VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT"
    */
   assert(p->formatFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);

   p->formatFeatures |= VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT;

   /* "Implementations may not always be able to determine the color model,
    *  numerical range, or chroma offsets of the image contents, so the values
    *  in VkAndroidHardwareBufferFormatPropertiesANDROID are only suggestions.
    *  Applications should treat these values as sensible defaults to use in
    *  the absence of more reliable information obtained through some other
    *  means."
    */
   p->samplerYcbcrConversionComponents.r = VK_COMPONENT_SWIZZLE_IDENTITY;
   p->samplerYcbcrConversionComponents.g = VK_COMPONENT_SWIZZLE_IDENTITY;
   p->samplerYcbcrConversionComponents.b = VK_COMPONENT_SWIZZLE_IDENTITY;
   p->samplerYcbcrConversionComponents.a = VK_COMPONENT_SWIZZLE_IDENTITY;

   p->suggestedYcbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_601;
   p->suggestedYcbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_FULL;

   p->suggestedXChromaOffset = VK_CHROMA_LOCATION_MIDPOINT;
   p->suggestedYChromaOffset = VK_CHROMA_LOCATION_MIDPOINT;

   return VK_SUCCESS;
}

static VkResult
get_ahb_buffer_format_properties2(VkDevice device_h, const struct AHardwareBuffer *buffer,
                                  VkAndroidHardwareBufferFormatProperties2ANDROID *pProperties)
{
   RADV_FROM_HANDLE(radv_device, device, device_h);

   /* Get a description of buffer contents . */
   AHardwareBuffer_Desc desc;
   AHardwareBuffer_describe(buffer, &desc);

   /* Verify description. */
   const uint64_t gpu_usage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE | AHARDWAREBUFFER_USAGE_GPU_COLOR_OUTPUT |
                              AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER;

   /* "Buffer must be a valid Android hardware buffer object with at least
    * one of the AHARDWAREBUFFER_USAGE_GPU_* usage flags."
    */
   if (!(desc.usage & (gpu_usage)))
      return VK_ERROR_INVALID_EXTERNAL_HANDLE;

   /* Fill properties fields based on description. */
   VkAndroidHardwareBufferFormatProperties2ANDROID *p = pProperties;

   p->format = vk_format_from_android(desc.format, desc.usage);
   p->externalFormat = (uint64_t)(uintptr_t)p->format;

   VkFormatProperties2 format_properties = {.sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2};

   radv_GetPhysicalDeviceFormatProperties2(radv_physical_device_to_handle(device->physical_device), p->format,
                                           &format_properties);

   if (desc.usage & AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER)
      p->formatFeatures = format_properties.formatProperties.linearTilingFeatures;
   else
      p->formatFeatures = format_properties.formatProperties.optimalTilingFeatures;

   /* "Images can be created with an external format even if the Android hardware
    *  buffer has a format which has an equivalent Vulkan format to enable
    *  consistent handling of images from sources that might use either category
    *  of format. However, all images created with an external format are subject
    *  to the valid usage requirements associated with external formats, even if
    *  the Android hardware buffer’s format has a Vulkan equivalent."
    *
    * "The formatFeatures member *must* include
    *  VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_BIT and at least one of
    *  VK_FORMAT_FEATURE_2_MIDPOINT_CHROMA_SAMPLES_BIT or
    *  VK_FORMAT_FEATURE_2_COSITED_CHROMA_SAMPLES_BIT"
    */
   assert(p->formatFeatures & VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_BIT);

   p->formatFeatures |= VK_FORMAT_FEATURE_2_MIDPOINT_CHROMA_SAMPLES_BIT;

   /* "Implementations may not always be able to determine the color model,
    *  numerical range, or chroma offsets of the image contents, so the values
    *  in VkAndroidHardwareBufferFormatPropertiesANDROID are only suggestions.
    *  Applications should treat these values as sensible defaults to use in
    *  the absence of more reliable information obtained through some other
    *  means."
    */
   p->samplerYcbcrConversionComponents.r = VK_COMPONENT_SWIZZLE_IDENTITY;
   p->samplerYcbcrConversionComponents.g = VK_COMPONENT_SWIZZLE_IDENTITY;
   p->samplerYcbcrConversionComponents.b = VK_COMPONENT_SWIZZLE_IDENTITY;
   p->samplerYcbcrConversionComponents.a = VK_COMPONENT_SWIZZLE_IDENTITY;

   p->suggestedYcbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_601;
   p->suggestedYcbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_FULL;

   p->suggestedXChromaOffset = VK_CHROMA_LOCATION_MIDPOINT;
   p->suggestedYChromaOffset = VK_CHROMA_LOCATION_MIDPOINT;

   return VK_SUCCESS;
}

VkResult
radv_GetAndroidHardwareBufferPropertiesANDROID(VkDevice device_h, const struct AHardwareBuffer *buffer,
                                               VkAndroidHardwareBufferPropertiesANDROID *pProperties)
{
   RADV_FROM_HANDLE(radv_device, dev, device_h);
   struct radv_physical_device *pdevice = dev->physical_device;

   VkAndroidHardwareBufferFormatPropertiesANDROID *format_prop =
      vk_find_struct(pProperties->pNext, ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID);

   /* Fill format properties of an Android hardware buffer. */
   if (format_prop)
      get_ahb_buffer_format_properties(device_h, buffer, format_prop);

   VkAndroidHardwareBufferFormatProperties2ANDROID *format_prop2 =
      vk_find_struct(pProperties->pNext, ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_2_ANDROID);
   if (format_prop2)
      get_ahb_buffer_format_properties2(device_h, buffer, format_prop2);

   /* NOTE - We support buffers with only one handle but do not error on
    * multiple handle case. Reason is that we want to support YUV formats
    * where we have many logical planes but they all point to the same
    * buffer, like is the case with VK_FORMAT_G8_B8R8_2PLANE_420_UNORM.
    */
   const native_handle_t *handle = AHardwareBuffer_getNativeHandle(buffer);
   int dma_buf = (handle && handle->numFds) ? handle->data[0] : -1;
   if (dma_buf < 0)
      return VK_ERROR_INVALID_EXTERNAL_HANDLE;

   /* All memory types. */
   uint32_t memory_types = (1u << pdevice->memory_properties.memoryTypeCount) - 1;

   pProperties->allocationSize = lseek(dma_buf, 0, SEEK_END);
   pProperties->memoryTypeBits = memory_types & ~pdevice->memory_types_32bit;

   return VK_SUCCESS;
}

VkResult
radv_GetMemoryAndroidHardwareBufferANDROID(VkDevice device_h, const VkMemoryGetAndroidHardwareBufferInfoANDROID *pInfo,
                                           struct AHardwareBuffer **pBuffer)
{
   RADV_FROM_HANDLE(radv_device_memory, mem, pInfo->memory);

   /* This should always be set due to the export handle types being set on
    * allocation. */
   assert(mem->android_hardware_buffer);

   /* Some quotes from Vulkan spec:
    *
    * "If the device memory was created by importing an Android hardware
    * buffer, vkGetMemoryAndroidHardwareBufferANDROID must return that same
    * Android hardware buffer object."
    *
    * "VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID must
    * have been included in VkExportMemoryAllocateInfo::handleTypes when
    * memory was created."
    */
   *pBuffer = mem->android_hardware_buffer;
   /* Increase refcount. */
   AHardwareBuffer_acquire(mem->android_hardware_buffer);
   return VK_SUCCESS;
}

#endif

VkFormat
radv_select_android_external_format(const void *next, VkFormat default_format)
{
#if RADV_SUPPORT_ANDROID_HARDWARE_BUFFER
   const VkExternalFormatANDROID *android_format = vk_find_struct_const(next, EXTERNAL_FORMAT_ANDROID);

   if (android_format && android_format->externalFormat) {
      return (VkFormat)android_format->externalFormat;
   }
#endif

   return default_format;
}

VkResult
radv_import_ahb_memory(struct radv_device *device, struct radv_device_memory *mem, unsigned priority,
                       const VkImportAndroidHardwareBufferInfoANDROID *info)
{
#if RADV_SUPPORT_ANDROID_HARDWARE_BUFFER
   /* Import from AHardwareBuffer to radv_device_memory. */
   const native_handle_t *handle = AHardwareBuffer_getNativeHandle(info->buffer);

   /* NOTE - We support buffers with only one handle but do not error on
    * multiple handle case. Reason is that we want to support YUV formats
    * where we have many logical planes but they all point to the same
    * buffer, like is the case with VK_FORMAT_G8_B8R8_2PLANE_420_UNORM.
    */
   int dma_buf = (handle && handle->numFds) ? handle->data[0] : -1;
   if (dma_buf < 0)
      return VK_ERROR_INVALID_EXTERNAL_HANDLE;

   uint64_t alloc_size = 0;
   VkResult result = device->ws->buffer_from_fd(device->ws, dma_buf, priority, &mem->bo, &alloc_size);
   if (result != VK_SUCCESS)
      return result;

   if (mem->image) {
      struct radeon_bo_metadata metadata;
      device->ws->buffer_get_metadata(device->ws, mem->bo, &metadata);

      struct radv_image_create_info create_info = {.no_metadata_planes = true, .bo_metadata = &metadata};

      result = radv_image_create_layout(device, create_info, NULL, NULL, mem->image);
      if (result != VK_SUCCESS) {
         device->ws->buffer_destroy(device->ws, mem->bo);
         mem->bo = NULL;
         return result;
      }

      if (alloc_size < mem->image->size) {
         device->ws->buffer_destroy(device->ws, mem->bo);
         mem->bo = NULL;
         return VK_ERROR_INVALID_EXTERNAL_HANDLE;
      }
   } else if (mem->buffer) {
      if (alloc_size < mem->buffer->vk.size) {
         device->ws->buffer_destroy(device->ws, mem->bo);
         mem->bo = NULL;
         return VK_ERROR_INVALID_EXTERNAL_HANDLE;
      }
   }

   /* "If the vkAllocateMemory command succeeds, the implementation must
    * acquire a reference to the imported hardware buffer, which it must
    * release when the device memory object is freed. If the command fails,
    * the implementation must not retain a reference."
    */
   AHardwareBuffer_acquire(info->buffer);
   mem->android_hardware_buffer = info->buffer;

   return VK_SUCCESS;
#else /* RADV_SUPPORT_ANDROID_HARDWARE_BUFFER */
   return VK_ERROR_EXTENSION_NOT_PRESENT;
#endif
}

VkResult
radv_create_ahb_memory(struct radv_device *device, struct radv_device_memory *mem, unsigned priority,
                       const VkMemoryAllocateInfo *pAllocateInfo)
{
#if RADV_SUPPORT_ANDROID_HARDWARE_BUFFER
   mem->android_hardware_buffer = vk_alloc_ahardware_buffer(pAllocateInfo);
   if (mem->android_hardware_buffer == NULL)
      return VK_ERROR_OUT_OF_HOST_MEMORY;

   const struct VkImportAndroidHardwareBufferInfoANDROID import_info = {
      .buffer = mem->android_hardware_buffer,
   };

   VkResult result = radv_import_ahb_memory(device, mem, priority, &import_info);

   /* Release a reference to avoid leak for AHB allocation. */
   AHardwareBuffer_release(mem->android_hardware_buffer);

   return result;
#else /* RADV_SUPPORT_ANDROID_HARDWARE_BUFFER */
   return VK_ERROR_EXTENSION_NOT_PRESENT;
#endif
}

bool
radv_android_gralloc_supports_format(VkFormat format, VkImageUsageFlagBits usage)
{
#if RADV_SUPPORT_ANDROID_HARDWARE_BUFFER
   /* Ideally we check AHardwareBuffer_isSupported.  But that test-allocates on most platforms and
    * seems a bit on the expensive side.  Return true as long as it is a format we understand.
    */
   (void)usage;
   return radv_ahb_format_for_vk_format(format);
#else
   (void)format;
   (void)usage;
   return false;
#endif
}
