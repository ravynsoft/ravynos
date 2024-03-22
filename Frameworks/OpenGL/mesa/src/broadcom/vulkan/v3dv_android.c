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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "v3dv_private.h"
#include <hardware/gralloc.h>

#if ANDROID_API_LEVEL >= 26
#include <hardware/gralloc1.h>
#endif

#include "drm-uapi/drm_fourcc.h"
#include <hardware/hardware.h>
#include <hardware/hwvulkan.h>

#include <vulkan/vk_android_native_buffer.h>
#include <vulkan/vk_icd.h>

#include "vk_android.h"
#include "vulkan/util/vk_enum_defines.h"

#include "util/libsync.h"
#include "util/log.h"
#include "util/os_file.h"

static int
v3dv_hal_open(const struct hw_module_t *mod,
              const char *id,
              struct hw_device_t **dev);
static int
v3dv_hal_close(struct hw_device_t *dev);

static_assert(HWVULKAN_DISPATCH_MAGIC == ICD_LOADER_MAGIC, "");

PUBLIC struct hwvulkan_module_t HAL_MODULE_INFO_SYM = {
   .common =
     {
       .tag = HARDWARE_MODULE_TAG,
       .module_api_version = HWVULKAN_MODULE_API_VERSION_0_1,
       .hal_api_version = HARDWARE_MAKE_API_VERSION(1, 0),
       .id = HWVULKAN_HARDWARE_MODULE_ID,
       .name = "Broadcom Vulkan HAL",
       .author = "Mesa3D",
       .methods =
         &(hw_module_methods_t) {
           .open = v3dv_hal_open,
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
v3dv_hal_open(const struct hw_module_t *mod,
              const char *id,
              struct hw_device_t **dev)
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
          .close = v3dv_hal_close,
        },
     .EnumerateInstanceExtensionProperties =
        v3dv_EnumerateInstanceExtensionProperties,
     .CreateInstance = v3dv_CreateInstance,
     .GetInstanceProcAddr = v3dv_GetInstanceProcAddr,
   };

   mesa_logi("v3dv: Warning: Android Vulkan implementation is experimental");

   *dev = &hal_dev->common;
   return 0;
}

static int
v3dv_hal_close(struct hw_device_t *dev)
{
   /* hwvulkan.h claims that hw_device_t::close() is never called. */
   return -1;
}

VkResult
v3dv_gralloc_to_drm_explicit_layout(struct u_gralloc *gralloc,
                                    struct u_gralloc_buffer_handle *in_hnd,
                                    VkImageDrmFormatModifierExplicitCreateInfoEXT *out,
                                    VkSubresourceLayout *out_layouts,
                                    int max_planes)
{
   struct u_gralloc_buffer_basic_info info;

   if (u_gralloc_get_buffer_basic_info(gralloc, in_hnd, &info) != 0)
      return VK_ERROR_INVALID_EXTERNAL_HANDLE;

   if (info.num_planes > max_planes)
      return VK_ERROR_INVALID_EXTERNAL_HANDLE;

   bool is_disjoint = false;
   for (int i = 1; i < info.num_planes; i++) {
      if (info.offsets[i] == 0) {
         is_disjoint = true;
         break;
      }
   }

   if (is_disjoint) {
      /* We don't support disjoint planes yet */
      return VK_ERROR_INVALID_EXTERNAL_HANDLE;
   }

   memset(out_layouts, 0, sizeof(*out_layouts) * info.num_planes);
   memset(out, 0, sizeof(*out));

   out->sType = VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_EXPLICIT_CREATE_INFO_EXT;
   out->pPlaneLayouts = out_layouts;

   out->drmFormatModifier = info.modifier;
   out->drmFormatModifierPlaneCount = info.num_planes;
   for (int i = 0; i < info.num_planes; i++) {
      out_layouts[i].offset = info.offsets[i];
      out_layouts[i].rowPitch = info.strides[i];
   }

   if (info.drm_fourcc == DRM_FORMAT_YVU420) {
      /* Swap the U and V planes to match the VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM */
      VkSubresourceLayout tmp = out_layouts[1];
      out_layouts[1] = out_layouts[2];
      out_layouts[2] = tmp;
   }

   return VK_SUCCESS;
}

VkResult
v3dv_import_native_buffer_fd(VkDevice device_h,
                             int native_buffer_fd,
                             const VkAllocationCallbacks *alloc,
                             VkImage image_h)
{
   VkResult result;

   VkDeviceMemory memory_h;

   const VkMemoryDedicatedAllocateInfo ded_alloc = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO,
      .pNext = NULL,
      .buffer = VK_NULL_HANDLE,
      .image = image_h
   };

   const VkImportMemoryFdInfoKHR import_info = {
      .sType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR,
      .pNext = &ded_alloc,
      .handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT,
      .fd = os_dupfd_cloexec(native_buffer_fd),
   };

   result =
      v3dv_AllocateMemory(device_h,
                          &(VkMemoryAllocateInfo) {
                             .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                             .pNext = &import_info,
                             .allocationSize = lseek(native_buffer_fd, 0, SEEK_END),
                             .memoryTypeIndex = 0,
                          },
                          alloc, &memory_h);

   if (result != VK_SUCCESS)
      goto fail_create_image;

   VkBindImageMemoryInfo bind_info = {
      .sType = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO,
      .image = image_h,
      .memory = memory_h,
      .memoryOffset = 0,
   };
   v3dv_BindImageMemory2(device_h, 1, &bind_info);

   return VK_SUCCESS;

fail_create_image:
   close(import_info.fd);

   return result;
}

static VkResult
format_supported_with_usage(VkDevice device_h,
                            VkFormat format,
                            VkImageUsageFlags imageUsage)
{
   V3DV_FROM_HANDLE(v3dv_device, device, device_h);
   struct v3dv_physical_device *phys_dev = device->pdevice;
   VkPhysicalDevice phys_dev_h = v3dv_physical_device_to_handle(phys_dev);
   VkResult result;

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
   result = v3dv_GetPhysicalDeviceImageFormatProperties2(
      phys_dev_h, &image_format_info, &image_format_props);
   if (result != VK_SUCCESS) {
      return vk_errorf(device, result,
                       "v3dv_GetPhysicalDeviceImageFormatProperties2 failed "
                       "inside %s",
                       __func__);
   }

   return VK_SUCCESS;
}

static VkResult
setup_gralloc0_usage(struct v3dv_device *device,
                     VkFormat format,
                     VkImageUsageFlags imageUsage,
                     int *grallocUsage)
{
   if (unmask32(&imageUsage, VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                             VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT))
      *grallocUsage |= GRALLOC_USAGE_HW_RENDER;

   if (unmask32(&imageUsage, VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                             VK_IMAGE_USAGE_SAMPLED_BIT |
                             VK_IMAGE_USAGE_STORAGE_BIT |
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

   /* Swapchain assumes direct displaying, therefore enable COMPOSER flag,
    * In case format is not supported by display controller, gralloc will
    * drop this flag and still allocate the buffer in VRAM
    */
   *grallocUsage |= GRALLOC_USAGE_HW_COMPOSER;

   if (*grallocUsage == 0)
      return VK_ERROR_FORMAT_NOT_SUPPORTED;

   return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL
v3dv_GetSwapchainGrallocUsageANDROID(VkDevice device_h,
                                     VkFormat format,
                                     VkImageUsageFlags imageUsage,
                                     int *grallocUsage)
{
   V3DV_FROM_HANDLE(v3dv_device, device, device_h);
   VkResult result;

   result = format_supported_with_usage(device_h, format, imageUsage);
   if (result != VK_SUCCESS)
      return result;

   *grallocUsage = 0;
   return setup_gralloc0_usage(device, format, imageUsage, grallocUsage);
}

#if ANDROID_API_LEVEL >= 26
VKAPI_ATTR VkResult VKAPI_CALL
v3dv_GetSwapchainGrallocUsage2ANDROID(
   VkDevice device_h,
   VkFormat format,
   VkImageUsageFlags imageUsage,
   VkSwapchainImageUsageFlagsANDROID swapchainImageUsage,
   uint64_t *grallocConsumerUsage,
   uint64_t *grallocProducerUsage)
{
   V3DV_FROM_HANDLE(v3dv_device, device, device_h);
   VkResult result;

   *grallocConsumerUsage = 0;
   *grallocProducerUsage = 0;
   mesa_logd("%s: format=%d, usage=0x%x", __func__, format, imageUsage);

   result = format_supported_with_usage(device_h, format, imageUsage);
   if (result != VK_SUCCESS)
      return result;

   int32_t grallocUsage = 0;
   result = setup_gralloc0_usage(device, format, imageUsage, &grallocUsage);
   if (result != VK_SUCCESS)
      return result;

   /* Setup gralloc1 usage flags from gralloc0 flags. */

   if (grallocUsage & GRALLOC_USAGE_HW_RENDER) {
      *grallocProducerUsage |= GRALLOC1_PRODUCER_USAGE_GPU_RENDER_TARGET;
   }

   if (grallocUsage & GRALLOC_USAGE_HW_TEXTURE) {
      *grallocConsumerUsage |= GRALLOC1_CONSUMER_USAGE_GPU_TEXTURE;
   }

   if (grallocUsage & GRALLOC_USAGE_HW_COMPOSER) {
      /* GPU composing case */
      *grallocConsumerUsage |= GRALLOC1_CONSUMER_USAGE_GPU_TEXTURE;
      /* Hardware composing case */
      *grallocConsumerUsage |= GRALLOC1_CONSUMER_USAGE_HWCOMPOSER;
   }

   if (swapchainImageUsage & VK_SWAPCHAIN_IMAGE_USAGE_SHARED_BIT_ANDROID) {
      uint64_t front_rendering_usage = 0;
      u_gralloc_get_front_rendering_usage(device->gralloc, &front_rendering_usage);
      *grallocProducerUsage |= front_rendering_usage;
   }

   return VK_SUCCESS;
}
#endif

/* ----------------------------- AHardwareBuffer --------------------------- */

static VkResult
get_ahb_buffer_format_properties2(VkDevice device_h, const struct AHardwareBuffer *buffer,
                                  VkAndroidHardwareBufferFormatProperties2ANDROID *pProperties)
{
   V3DV_FROM_HANDLE(v3dv_device, device, device_h);

   /* Get a description of buffer contents . */
   AHardwareBuffer_Desc desc;
   AHardwareBuffer_describe(buffer, &desc);

   /* Verify description. */
   const uint64_t gpu_usage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE |
                              AHARDWAREBUFFER_USAGE_GPU_COLOR_OUTPUT |
                              AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER;

   /* "Buffer must be a valid Android hardware buffer object with at least
    * one of the AHARDWAREBUFFER_USAGE_GPU_* usage flags."
    */
   if (!(desc.usage & (gpu_usage)))
      return VK_ERROR_INVALID_EXTERNAL_HANDLE;

   /* Fill properties fields based on description. */
   VkAndroidHardwareBufferFormatProperties2ANDROID *p = pProperties;

   p->samplerYcbcrConversionComponents.r = VK_COMPONENT_SWIZZLE_IDENTITY;
   p->samplerYcbcrConversionComponents.g = VK_COMPONENT_SWIZZLE_IDENTITY;
   p->samplerYcbcrConversionComponents.b = VK_COMPONENT_SWIZZLE_IDENTITY;
   p->samplerYcbcrConversionComponents.a = VK_COMPONENT_SWIZZLE_IDENTITY;

   p->suggestedYcbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_601;
   p->suggestedYcbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_FULL;

   p->suggestedXChromaOffset = VK_CHROMA_LOCATION_MIDPOINT;
   p->suggestedYChromaOffset = VK_CHROMA_LOCATION_MIDPOINT;

   VkFormatProperties2 format_properties = {.sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2};

   p->format = vk_ahb_format_to_image_format(desc.format);

   VkFormat external_format = p->format;

   if (p->format != VK_FORMAT_UNDEFINED)
      goto finish;

   /* External format only case
    *
    * From vkGetAndroidHardwareBufferPropertiesANDROID spec:
    * "If the Android hardware buffer has one of the formats listed in the Format
    * Equivalence table (see spec.), then format must have the equivalent Vulkan
    * format listed in the table. Otherwise, format may be VK_FORMAT_UNDEFINED,
    * indicating the Android hardware buffer can only be used with an external format."
    *
    * From SKIA source code analysis: p->format MUST be VK_FORMAT_UNDEFINED, if the
    * format is not in the Equivalence table.
    */

   struct u_gralloc_buffer_handle gr_handle = {
      .handle = AHardwareBuffer_getNativeHandle(buffer),
      .pixel_stride = desc.stride,
      .hal_format = desc.format,
   };

   struct u_gralloc_buffer_basic_info info;

   if (u_gralloc_get_buffer_basic_info(device->gralloc, &gr_handle, &info) != 0)
      return VK_ERROR_INVALID_EXTERNAL_HANDLE;

   switch (info.drm_fourcc) {
   case DRM_FORMAT_YVU420:
      /* Assuming that U and V planes are swapped earlier */
      external_format = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;
      break;
   case DRM_FORMAT_NV12:
      external_format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
      break;
   default:;
      mesa_loge("Unsupported external DRM format: %d", info.drm_fourcc);
      return VK_ERROR_INVALID_EXTERNAL_HANDLE;
   }

   struct u_gralloc_buffer_color_info color_info;
   if (u_gralloc_get_buffer_color_info(device->gralloc, &gr_handle, &color_info) == 0) {
      switch (color_info.yuv_color_space) {
      case __DRI_YUV_COLOR_SPACE_ITU_REC601:
         p->suggestedYcbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_601;
         break;
      case __DRI_YUV_COLOR_SPACE_ITU_REC709:
         p->suggestedYcbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709;
         break;
      case __DRI_YUV_COLOR_SPACE_ITU_REC2020:
         p->suggestedYcbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_2020;
         break;
      default:
         break;
      }

      p->suggestedYcbcrRange = (color_info.sample_range == __DRI_YUV_NARROW_RANGE) ?
         VK_SAMPLER_YCBCR_RANGE_ITU_NARROW : VK_SAMPLER_YCBCR_RANGE_ITU_FULL;
      p->suggestedXChromaOffset = (color_info.horizontal_siting == __DRI_YUV_CHROMA_SITING_0_5) ?
         VK_CHROMA_LOCATION_MIDPOINT : VK_CHROMA_LOCATION_COSITED_EVEN;
      p->suggestedYChromaOffset = (color_info.vertical_siting == __DRI_YUV_CHROMA_SITING_0_5) ?
         VK_CHROMA_LOCATION_MIDPOINT : VK_CHROMA_LOCATION_COSITED_EVEN;
   }

finish:

   v3dv_GetPhysicalDeviceFormatProperties2(v3dv_physical_device_to_handle(device->pdevice),
                                           external_format, &format_properties);

   /* v3dv doesn't support direct sampling from linear images but has a logic to copy
    * from linear to tiled images implicitly before sampling. Therefore expose optimal
    * features for both linear and optimal tiling.
    */
   p->formatFeatures = format_properties.formatProperties.optimalTilingFeatures;
   p->externalFormat = external_format;

   /* From vkGetAndroidHardwareBufferPropertiesANDROID spec:
    * "The formatFeatures member *must* include
    *  VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_BIT and at least one of
    *  VK_FORMAT_FEATURE_2_MIDPOINT_CHROMA_SAMPLES_BIT or
    *  VK_FORMAT_FEATURE_2_COSITED_CHROMA_SAMPLES_BIT"
    */
   p->formatFeatures |= VK_FORMAT_FEATURE_2_MIDPOINT_CHROMA_SAMPLES_BIT;

   return VK_SUCCESS;
}

VkResult
v3dv_GetAndroidHardwareBufferPropertiesANDROID(VkDevice device_h,
                                               const struct AHardwareBuffer *buffer,
                                               VkAndroidHardwareBufferPropertiesANDROID *pProperties)
{
   V3DV_FROM_HANDLE(v3dv_device, dev, device_h);
   struct v3dv_physical_device *pdevice = dev->pdevice;

   VkResult result;

   VkAndroidHardwareBufferFormatPropertiesANDROID *format_prop =
      vk_find_struct(pProperties->pNext, ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID);

   /* Fill format properties of an Android hardware buffer. */
   if (format_prop) {
      VkAndroidHardwareBufferFormatProperties2ANDROID format_prop2 = {
         .sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_2_ANDROID,
      };
      result = get_ahb_buffer_format_properties2(device_h, buffer, &format_prop2);
      if (result != VK_SUCCESS)
         return result;

      format_prop->format                 = format_prop2.format;
      format_prop->externalFormat         = format_prop2.externalFormat;
      format_prop->formatFeatures         =
         vk_format_features2_to_features(format_prop2.formatFeatures);
      format_prop->samplerYcbcrConversionComponents =
         format_prop2.samplerYcbcrConversionComponents;
      format_prop->suggestedYcbcrModel    = format_prop2.suggestedYcbcrModel;
      format_prop->suggestedYcbcrRange    = format_prop2.suggestedYcbcrRange;
      format_prop->suggestedXChromaOffset = format_prop2.suggestedXChromaOffset;
      format_prop->suggestedYChromaOffset = format_prop2.suggestedYChromaOffset;
   }

   VkAndroidHardwareBufferFormatProperties2ANDROID *format_prop2 =
      vk_find_struct(pProperties->pNext, ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_2_ANDROID);
   if (format_prop2) {
      result = get_ahb_buffer_format_properties2(device_h, buffer, format_prop2);
      if (result != VK_SUCCESS)
         return result;
   }

   const native_handle_t *handle = AHardwareBuffer_getNativeHandle(buffer);
   assert(handle && handle->numFds > 0);
   pProperties->allocationSize = lseek(handle->data[0], 0, SEEK_END);

   /* All memory types. */
   pProperties->memoryTypeBits = (1u << pdevice->memory.memoryTypeCount) - 1;

   return VK_SUCCESS;
}
