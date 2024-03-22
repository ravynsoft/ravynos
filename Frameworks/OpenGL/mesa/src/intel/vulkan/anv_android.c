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

#include <hardware/gralloc.h>

#if ANDROID_API_LEVEL >= 26
#include <hardware/gralloc1.h>
#endif

#include <hardware/hardware.h>
#include <hardware/hwvulkan.h>
#include <vulkan/vk_android_native_buffer.h>
#include <vulkan/vk_icd.h>
#include <sync/sync.h>

#include "anv_private.h"
#include "vk_android.h"
#include "vk_common_entrypoints.h"
#include "vk_util.h"

static int anv_hal_open(const struct hw_module_t* mod, const char* id, struct hw_device_t** dev);
static int anv_hal_close(struct hw_device_t *dev);

static_assert(HWVULKAN_DISPATCH_MAGIC == ICD_LOADER_MAGIC, "");

PUBLIC struct hwvulkan_module_t HAL_MODULE_INFO_SYM = {
   .common = {
      .tag = HARDWARE_MODULE_TAG,
      .module_api_version = HWVULKAN_MODULE_API_VERSION_0_1,
      .hal_api_version = HARDWARE_MAKE_API_VERSION(1, 0),
      .id = HWVULKAN_HARDWARE_MODULE_ID,
      .name = "Intel Vulkan HAL",
      .author = "Intel",
      .methods = &(hw_module_methods_t) {
         .open = anv_hal_open,
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
anv_hal_open(const struct hw_module_t* mod, const char* id,
             struct hw_device_t** dev)
{
   assert(mod == &HAL_MODULE_INFO_SYM.common);
   assert(strcmp(id, HWVULKAN_DEVICE_0) == 0);

   hwvulkan_device_t *hal_dev = malloc(sizeof(*hal_dev));
   if (!hal_dev)
      return -1;

   *hal_dev = (hwvulkan_device_t) {
      .common = {
         .tag = HARDWARE_DEVICE_TAG,
         .version = HWVULKAN_DEVICE_API_VERSION_0_1,
         .module = &HAL_MODULE_INFO_SYM.common,
         .close = anv_hal_close,
      },
     .EnumerateInstanceExtensionProperties = anv_EnumerateInstanceExtensionProperties,
     .CreateInstance = anv_CreateInstance,
     .GetInstanceProcAddr = anv_GetInstanceProcAddr,
   };

   *dev = &hal_dev->common;
   return 0;
}

static int
anv_hal_close(struct hw_device_t *dev)
{
   /* hwvulkan.h claims that hw_device_t::close() is never called. */
   return -1;
}

#if ANDROID_API_LEVEL >= 26
#include <vndk/hardware_buffer.h>
/* See i915_private_android_types.h in minigbm. */
#define HAL_PIXEL_FORMAT_NV12_Y_TILED_INTEL 0x100

enum {
   /* Usage bit equal to GRALLOC_USAGE_HW_CAMERA_MASK */
   BUFFER_USAGE_CAMERA_MASK = 0x00060000U,
};

inline VkFormat
vk_format_from_android(unsigned android_format, unsigned android_usage)
{
   switch (android_format) {
   case AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM:
      return VK_FORMAT_R8G8B8_UNORM;
   case AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420:
   case HAL_PIXEL_FORMAT_NV12_Y_TILED_INTEL:
      return VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
   case AHARDWAREBUFFER_FORMAT_YV12:
      return VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;
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
anv_ahb_format_for_vk_format(VkFormat vk_format)
{
   switch (vk_format) {
   case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
#ifdef HAVE_CROS_GRALLOC
      return AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420;
#else
      return HAL_PIXEL_FORMAT_NV12_Y_TILED_INTEL;
#endif
   default:
      return vk_image_format_to_ahb_format(vk_format);
   }
}

static VkResult
get_ahw_buffer_format_properties2(
   VkDevice device_h,
   const struct AHardwareBuffer *buffer,
   VkAndroidHardwareBufferFormatProperties2ANDROID *pProperties)
{
   ANV_FROM_HANDLE(anv_device, device, device_h);

   /* Get a description of buffer contents . */
   AHardwareBuffer_Desc desc;
   AHardwareBuffer_describe(buffer, &desc);

   /* Verify description. */
   uint64_t gpu_usage =
      AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE |
      AHARDWAREBUFFER_USAGE_GPU_COLOR_OUTPUT |
      AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER;

   /* "Buffer must be a valid Android hardware buffer object with at least
    * one of the AHARDWAREBUFFER_USAGE_GPU_* usage flags."
    */
   if (!(desc.usage & (gpu_usage)))
      return VK_ERROR_INVALID_EXTERNAL_HANDLE;

   /* Fill properties fields based on description. */
   VkAndroidHardwareBufferFormatProperties2ANDROID *p = pProperties;

   p->format = vk_format_from_android(desc.format, desc.usage);
   p->externalFormat = p->format;

   const struct anv_format *anv_format = anv_get_format(p->format);

   /* Default to OPTIMAL tiling but set to linear in case
    * of AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER usage.
    */
   VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;

   if (desc.usage & AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER)
      tiling = VK_IMAGE_TILING_LINEAR;

   p->formatFeatures =
      anv_get_image_format_features2(device->physical, p->format, anv_format,
                                     tiling, NULL);

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
   p->formatFeatures |=
      VK_FORMAT_FEATURE_2_MIDPOINT_CHROMA_SAMPLES_BIT;

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
anv_GetAndroidHardwareBufferPropertiesANDROID(
   VkDevice device_h,
   const struct AHardwareBuffer *buffer,
   VkAndroidHardwareBufferPropertiesANDROID *pProperties)
{
   ANV_FROM_HANDLE(anv_device, dev, device_h);

   VkAndroidHardwareBufferFormatPropertiesANDROID *format_prop =
      vk_find_struct(pProperties->pNext,
                     ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID);
   /* Fill format properties of an Android hardware buffer. */
   if (format_prop) {
      VkAndroidHardwareBufferFormatProperties2ANDROID format_prop2 = {
         .sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_2_ANDROID,
      };
      get_ahw_buffer_format_properties2(device_h, buffer, &format_prop2);

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
      vk_find_struct(pProperties->pNext,
                     ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_2_ANDROID);
   if (format_prop2)
      get_ahw_buffer_format_properties2(device_h, buffer, format_prop2);

   /* NOTE - We support buffers with only one handle but do not error on
    * multiple handle case. Reason is that we want to support YUV formats
    * where we have many logical planes but they all point to the same
    * buffer, like is the case with VK_FORMAT_G8_B8R8_2PLANE_420_UNORM.
    */
   const native_handle_t *handle =
      AHardwareBuffer_getNativeHandle(buffer);
   int dma_buf = (handle && handle->numFds) ? handle->data[0] : -1;
   if (dma_buf < 0)
      return VK_ERROR_INVALID_EXTERNAL_HANDLE;

   /* All memory types. */
   uint32_t memory_types = (1ull << dev->physical->memory.type_count) - 1;

   pProperties->allocationSize = lseek(dma_buf, 0, SEEK_END);
   pProperties->memoryTypeBits = memory_types;

   return VK_SUCCESS;
}

#endif

/*
 * Called from anv_AllocateMemory when import AHardwareBuffer.
 */
VkResult
anv_import_ahw_memory(VkDevice device_h,
                      struct anv_device_memory *mem)
{
#if ANDROID_API_LEVEL >= 26
   ANV_FROM_HANDLE(anv_device, device, device_h);

   /* Import from AHardwareBuffer to anv_device_memory. */
   const native_handle_t *handle =
      AHardwareBuffer_getNativeHandle(mem->vk.ahardware_buffer);

   /* NOTE - We support buffers with only one handle but do not error on
    * multiple handle case. Reason is that we want to support YUV formats
    * where we have many logical planes but they all point to the same
    * buffer, like is the case with VK_FORMAT_G8_B8R8_2PLANE_420_UNORM.
    */
   int dma_buf = (handle && handle->numFds) ? handle->data[0] : -1;
   if (dma_buf < 0)
      return VK_ERROR_INVALID_EXTERNAL_HANDLE;

   VkResult result = anv_device_import_bo(device, dma_buf, 0,
                                          0 /* client_address */,
                                          &mem->bo);
   assert(result == VK_SUCCESS);

   return VK_SUCCESS;
#else
   return VK_ERROR_EXTENSION_NOT_PRESENT;
#endif
}

VkResult
anv_image_init_from_gralloc(struct anv_device *device,
                            struct anv_image *image,
                            const VkImageCreateInfo *base_info,
                            const VkNativeBufferANDROID *gralloc_info)
{
   struct anv_bo *bo = NULL;
   VkResult result;

   struct anv_image_create_info anv_info = {
      .vk_info = base_info,
      .isl_extra_usage_flags = ISL_SURF_USAGE_DISABLE_AUX_BIT,
   };

   /* Do not close the gralloc handle's dma_buf. The lifetime of the dma_buf
    * must exceed that of the gralloc handle, and we do not own the gralloc
    * handle.
    */
   int dma_buf = gralloc_info->handle->data[0];

   /* We need to set the WRITE flag on window system buffers so that GEM will
    * know we're writing to them and synchronize uses on other rings (for
    * example, if the display server uses the blitter ring).
    *
    * If this function fails and if the imported bo was resident in the cache,
    * we should avoid updating the bo's flags. Therefore, we defer updating
    * the flags until success is certain.
    *
    */
   result = anv_device_import_bo(device, dma_buf,
                                 ANV_BO_ALLOC_EXTERNAL |
                                 ANV_BO_ALLOC_IMPLICIT_SYNC |
                                 ANV_BO_ALLOC_IMPLICIT_WRITE,
                                 0 /* client_address */,
                                 &bo);
   if (result != VK_SUCCESS) {
      return vk_errorf(device, result,
                       "failed to import dma-buf from VkNativeBufferANDROID");
   }

   enum isl_tiling tiling;
   result = anv_device_get_bo_tiling(device, bo, &tiling);
   if (result != VK_SUCCESS) {
      return vk_errorf(device, result,
                       "failed to get tiling from VkNativeBufferANDROID");
   }
   anv_info.isl_tiling_flags = 1u << tiling;

   anv_info.stride = gralloc_info->stride;

   result = anv_image_init(device, image, &anv_info);
   if (result != VK_SUCCESS)
      goto fail_init;

   VkMemoryRequirements2 mem_reqs = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2,
   };

   anv_image_get_memory_requirements(device, image, image->vk.aspects,
                                     &mem_reqs);

   VkDeviceSize aligned_image_size =
      align64(mem_reqs.memoryRequirements.size,
              mem_reqs.memoryRequirements.alignment);

   if (bo->size < aligned_image_size) {
      result = vk_errorf(device, VK_ERROR_INVALID_EXTERNAL_HANDLE,
                         "dma-buf from VkNativeBufferANDROID is too small for "
                         "VkImage: %"PRIu64"B < %"PRIu64"B",
                         bo->size, aligned_image_size);
      goto fail_size;
   }

   assert(!image->disjoint);
   assert(image->n_planes == 1);
   assert(image->planes[0].primary_surface.memory_range.binding ==
          ANV_IMAGE_MEMORY_BINDING_MAIN);
   assert(image->bindings[ANV_IMAGE_MEMORY_BINDING_MAIN].address.bo == NULL);
   assert(image->bindings[ANV_IMAGE_MEMORY_BINDING_MAIN].address.offset == 0);
   image->bindings[ANV_IMAGE_MEMORY_BINDING_MAIN].address.bo = bo;
   image->from_gralloc = true;

   return VK_SUCCESS;

 fail_size:
   anv_image_finish(image);
 fail_init:
   anv_device_release_bo(device, bo);

   return result;
}

VkResult
anv_image_bind_from_gralloc(struct anv_device *device,
                            struct anv_image *image,
                            const VkNativeBufferANDROID *gralloc_info)
{
   /* Do not close the gralloc handle's dma_buf. The lifetime of the dma_buf
    * must exceed that of the gralloc handle, and we do not own the gralloc
    * handle.
    */
   int dma_buf = gralloc_info->handle->data[0];

   /* We need to set the WRITE flag on window system buffers so that GEM will
    * know we're writing to them and synchronize uses on other rings (for
    * example, if the display server uses the blitter ring).
    *
    * If this function fails and if the imported bo was resident in the cache,
    * we should avoid updating the bo's flags. Therefore, we defer updating
    * the flags until success is certain.
    *
    */
   struct anv_bo *bo = NULL;
   VkResult result = anv_device_import_bo(device, dma_buf,
                                          ANV_BO_ALLOC_EXTERNAL |
                                          ANV_BO_ALLOC_IMPLICIT_SYNC |
                                          ANV_BO_ALLOC_IMPLICIT_WRITE,
                                          0 /* client_address */,
                                          &bo);
   if (result != VK_SUCCESS) {
      return vk_errorf(device, result,
                       "failed to import dma-buf from VkNativeBufferANDROID");
   }

   uint64_t img_size = image->bindings[ANV_IMAGE_MEMORY_BINDING_MAIN].memory_range.size;
   if (img_size < bo->size) {
      result = vk_errorf(device, VK_ERROR_INVALID_EXTERNAL_HANDLE,
                         "dma-buf from VkNativeBufferANDROID is too small for "
                         "VkImage: %"PRIu64"B < %"PRIu64"B",
                         bo->size, img_size);
      anv_device_release_bo(device, bo);
      return result;
   }

   assert(!image->disjoint);
   assert(image->n_planes == 1);
   assert(image->planes[0].primary_surface.memory_range.binding ==
          ANV_IMAGE_MEMORY_BINDING_MAIN);
   assert(image->bindings[ANV_IMAGE_MEMORY_BINDING_MAIN].address.bo == NULL);
   assert(image->bindings[ANV_IMAGE_MEMORY_BINDING_MAIN].address.offset == 0);
   image->bindings[ANV_IMAGE_MEMORY_BINDING_MAIN].address.bo = bo;
   image->from_gralloc = true;

   return VK_SUCCESS;
}

static VkResult
format_supported_with_usage(VkDevice device_h, VkFormat format,
                            VkImageUsageFlags imageUsage)
{
   ANV_FROM_HANDLE(anv_device, device, device_h);
   VkPhysicalDevice phys_dev_h = anv_physical_device_to_handle(device->physical);
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
   result = anv_GetPhysicalDeviceImageFormatProperties2(phys_dev_h,
               &image_format_info, &image_format_props);
   if (result != VK_SUCCESS) {
      return vk_errorf(device, result,
                       "anv_GetPhysicalDeviceImageFormatProperties2 failed "
                       "inside %s", __func__);
   }
   return VK_SUCCESS;
}


static VkResult
setup_gralloc0_usage(struct anv_device *device, VkFormat format,
                     VkImageUsageFlags imageUsage, int *grallocUsage)
{
   /* WARNING: Android's libvulkan.so hardcodes the VkImageUsageFlags
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
                       "swapchain", imageUsage);
   }

   /* The below formats support GRALLOC_USAGE_HW_FB (that is, display
    * scanout). This short list of formats is univserally supported on Intel
    * but is incomplete.  The full set of supported formats is dependent on
    * kernel and hardware.
    *
    * FINISHME: Advertise all display-supported formats.
    */
   switch (format) {
      case VK_FORMAT_B8G8R8A8_UNORM:
      case VK_FORMAT_R5G6B5_UNORM_PACK16:
      case VK_FORMAT_R8G8B8A8_UNORM:
      case VK_FORMAT_R8G8B8A8_SRGB:
         *grallocUsage |= GRALLOC_USAGE_HW_FB |
                          GRALLOC_USAGE_HW_COMPOSER |
                          GRALLOC_USAGE_EXTERNAL_DISP;
         break;
      default:
         mesa_logw("%s: unsupported format=%d", __func__, format);
   }

   if (*grallocUsage == 0)
      return VK_ERROR_FORMAT_NOT_SUPPORTED;

   return VK_SUCCESS;
}

#if ANDROID_API_LEVEL >= 26
VkResult anv_GetSwapchainGrallocUsage2ANDROID(
    VkDevice            device_h,
    VkFormat            format,
    VkImageUsageFlags   imageUsage,
    VkSwapchainImageUsageFlagsANDROID swapchainImageUsage,
    uint64_t*           grallocConsumerUsage,
    uint64_t*           grallocProducerUsage)
{
   ANV_FROM_HANDLE(anv_device, device, device_h);
   VkResult result;

   *grallocConsumerUsage = 0;
   *grallocProducerUsage = 0;
   mesa_logd("%s: format=%d, usage=0x%x, swapchainUsage=0x%x", __func__, format,
             imageUsage, swapchainImageUsage);

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
      *grallocConsumerUsage |= GRALLOC1_CONSUMER_USAGE_CLIENT_TARGET;
   }

   if (grallocUsage & GRALLOC_USAGE_HW_TEXTURE) {
      *grallocConsumerUsage |= GRALLOC1_CONSUMER_USAGE_GPU_TEXTURE;
   }

   if (grallocUsage & (GRALLOC_USAGE_HW_FB |
                       GRALLOC_USAGE_HW_COMPOSER |
                       GRALLOC_USAGE_EXTERNAL_DISP)) {
      *grallocProducerUsage |= GRALLOC1_PRODUCER_USAGE_GPU_RENDER_TARGET;
      *grallocConsumerUsage |= GRALLOC1_CONSUMER_USAGE_HWCOMPOSER;
   }

   if ((swapchainImageUsage & VK_SWAPCHAIN_IMAGE_USAGE_SHARED_BIT_ANDROID) &&
       device->u_gralloc != NULL) {
      uint64_t front_rendering_usage = 0;
      u_gralloc_get_front_rendering_usage(device->u_gralloc, &front_rendering_usage);
      *grallocProducerUsage |= front_rendering_usage;
   }

   return VK_SUCCESS;
}
#endif

VkResult anv_GetSwapchainGrallocUsageANDROID(
    VkDevice            device_h,
    VkFormat            format,
    VkImageUsageFlags   imageUsage,
    int*                grallocUsage)
{
   ANV_FROM_HANDLE(anv_device, device, device_h);
   VkResult result;

   *grallocUsage = 0;
   mesa_logd("%s: format=%d, usage=0x%x", __func__, format, imageUsage);

   result = format_supported_with_usage(device_h, format, imageUsage);
   if (result != VK_SUCCESS)
      return result;

   return setup_gralloc0_usage(device, format, imageUsage, grallocUsage);
}
