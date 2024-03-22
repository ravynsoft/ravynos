/*
 * Copyright © 2017, Google Inc.
 * SPDX-License-Identifier: MIT
 */

#include "tu_android.h"

#include <hardware/gralloc.h>

#if ANDROID_API_LEVEL >= 26
#include <hardware/gralloc1.h>
#endif

#include <hardware/hardware.h>
#include <hardware/hwvulkan.h>

#include "drm-uapi/drm_fourcc.h"

#include "util/libsync.h"
#include "util/os_file.h"

#include "tu_device.h"
#include "tu_image.h"

static int
tu_hal_open(const struct hw_module_t *mod,
            const char *id,
            struct hw_device_t **dev);
static int
tu_hal_close(struct hw_device_t *dev);

static_assert(HWVULKAN_DISPATCH_MAGIC == ICD_LOADER_MAGIC, "");

struct hw_module_methods_t HAL_MODULE_METHODS = {
   .open = tu_hal_open,
};

PUBLIC struct hwvulkan_module_t HAL_MODULE_INFO_SYM = {
   .common =
     {
       .tag = HARDWARE_MODULE_TAG,
       .module_api_version = HWVULKAN_MODULE_API_VERSION_0_1,
       .hal_api_version = HARDWARE_MAKE_API_VERSION(1, 0),
       .id = HWVULKAN_HARDWARE_MODULE_ID,
       .name = "Turnip Vulkan HAL",
       .author = "Google",
       .methods = &HAL_MODULE_METHODS,
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
tu_hal_open(const struct hw_module_t *mod,
            const char *id,
            struct hw_device_t **dev)
{
   assert(mod == &HAL_MODULE_INFO_SYM.common);
   assert(strcmp(id, HWVULKAN_DEVICE_0) == 0);

   hwvulkan_device_t *hal_dev = (hwvulkan_device_t *) malloc(sizeof(*hal_dev));
   if (!hal_dev)
      return -1;

   *hal_dev = (hwvulkan_device_t){
      .common =
        {
          .tag = HARDWARE_DEVICE_TAG,
          .version = HWVULKAN_DEVICE_API_VERSION_0_1,
          .module = &HAL_MODULE_INFO_SYM.common,
          .close = tu_hal_close,
        },
      .EnumerateInstanceExtensionProperties =
        tu_EnumerateInstanceExtensionProperties,
      .CreateInstance = tu_CreateInstance,
      .GetInstanceProcAddr = tu_GetInstanceProcAddr,
   };

   *dev = &hal_dev->common;
   return 0;
}

static int
tu_hal_close(struct hw_device_t *dev)
{
   /* hwvulkan.h claims that hw_device_t::close() is never called. */
   return -1;
}

/* get dma-buf and modifier from gralloc info */
static VkResult
tu_gralloc_info_other(struct tu_device *device,
                      const VkNativeBufferANDROID *gralloc_info,
                      int *dma_buf,
                      uint64_t *modifier)

{
   const uint32_t *handle_fds = (uint32_t *)gralloc_info->handle->data;
   const uint32_t *handle_data = &handle_fds[gralloc_info->handle->numFds];
   bool ubwc = false;

   if (gralloc_info->handle->numFds == 1) {
      /* gbm_gralloc.  TODO: modifiers support */
      *dma_buf = handle_fds[0];
   } else if (gralloc_info->handle->numFds == 2) {
      /* Qualcomm gralloc, find it at:
       *
       * https://android.googlesource.com/platform/hardware/qcom/display/.
       *
       * The gralloc_info->handle is a pointer to a struct private_handle_t
       * from your platform's gralloc.  On msm8996 (a5xx) and newer grallocs
       * that's libgralloc1/gr_priv_handle.h, while previously it was
       * libgralloc/gralloc_priv.h.
       */

      if (gralloc_info->handle->numInts < 2) {
         return vk_errorf(device, VK_ERROR_INVALID_EXTERNAL_HANDLE,
                          "VkNativeBufferANDROID::handle::numInts is %d, "
                          "expected at least 2 for qcom gralloc",
                          gralloc_info->handle->numFds);
      }

      uint32_t gmsm = ('g' << 24) | ('m' << 16) | ('s' << 8) | 'm';
      if (handle_data[0] != gmsm) {
         return vk_errorf(device, VK_ERROR_INVALID_EXTERNAL_HANDLE,
                          "private_handle_t::magic is %x, expected %x",
                          handle_data[0], gmsm);
      }

      /* This UBWC flag was introduced in a5xx. */
      ubwc = handle_data[1] & 0x08000000;

      /* QCOM gralloc has two fds passed in: the actual GPU buffer, and a buffer
       * of CPU-side metadata.  I haven't found any need for the metadata buffer
       * yet.  See qdMetaData.h for what's in the metadata fd.
       */
      *dma_buf = handle_fds[0];
   } else {
      return vk_errorf(device, VK_ERROR_INVALID_EXTERNAL_HANDLE,
                       "VkNativeBufferANDROID::handle::numFds is %d, "
                       "expected 1 (gbm_gralloc) or 2 (qcom gralloc)",
                       gralloc_info->handle->numFds);
   }

   *modifier = ubwc ? DRM_FORMAT_MOD_QCOM_COMPRESSED : DRM_FORMAT_MOD_LINEAR;
   return VK_SUCCESS;
}

static const char cros_gralloc_module_name[] = "CrOS Gralloc";

#define CROS_GRALLOC_DRM_GET_BUFFER_INFO 4
#define CROS_GRALLOC_DRM_GET_USAGE 5
#define CROS_GRALLOC_DRM_GET_USAGE_FRONT_RENDERING_BIT 0x1

struct cros_gralloc0_buffer_info {
   uint32_t drm_fourcc;
   int num_fds;
   int fds[4];
   uint64_t modifier;
   int offset[4];
   int stride[4];
};

static VkResult
tu_gralloc_info_cros(struct tu_device *device,
                     const VkNativeBufferANDROID *gralloc_info,
                     int *dma_buf,
                     uint64_t *modifier)

{
   const gralloc_module_t *gralloc = (const gralloc_module_t *) device->gralloc;
   struct cros_gralloc0_buffer_info info;
   int ret;

   ret = gralloc->perform(gralloc, CROS_GRALLOC_DRM_GET_BUFFER_INFO,
                          gralloc_info->handle, &info);
   if (ret)
      return VK_ERROR_INVALID_EXTERNAL_HANDLE;

   *dma_buf = info.fds[0];
   *modifier = info.modifier;

   return VK_SUCCESS;
}

VkResult
tu_gralloc_info(struct tu_device *device,
                const VkNativeBufferANDROID *gralloc_info,
                int *dma_buf,
                uint64_t *modifier)

{
   if (!device->gralloc) {
      /* get gralloc module for gralloc buffer info query */
      int ret = hw_get_module(GRALLOC_HARDWARE_MODULE_ID,
                              (const hw_module_t **)&device->gralloc);

      if (ret) {
         /* This is *slightly* awkward, but if we are asked to import
          * a gralloc handle, and there is no gralloc, it is some sort
          * of invalid handle.
          */
         return vk_startup_errorf(device->instance,
                                  VK_ERROR_INVALID_EXTERNAL_HANDLE,
                                  "Could not open gralloc\n");
      }

      const gralloc_module_t *gralloc = (const gralloc_module_t *) device->gralloc;

      mesa_logi("opened gralloc module name: %s", gralloc->common.name);

      /* TODO not sure qcom gralloc module name, but we should check
       * for it here and move the special gmsm handling out of
       * tu_gralloc_info_other()
       */
      if (!strcmp(gralloc->common.name, cros_gralloc_module_name) && gralloc->perform) {
         device->gralloc_type = TU_GRALLOC_CROS;
      } else {
         device->gralloc_type = TU_GRALLOC_OTHER;
      }
   }

   if (device->gralloc_type == TU_GRALLOC_CROS) {
      return tu_gralloc_info_cros(device, gralloc_info, dma_buf, modifier);
   } else {
      return tu_gralloc_info_other(device, gralloc_info, dma_buf, modifier);
   }
}

/**
 * Creates the VkImage using the gralloc handle in *gralloc_info.
 *
 * We support two different grallocs here, gbm_gralloc, and the qcom gralloc
 * used on Android phones.
 */
VkResult
tu_import_memory_from_gralloc_handle(VkDevice device_h,
                                     int dma_buf,
                                     const VkAllocationCallbacks *alloc,
                                     VkImage image_h)

{
   struct tu_image *image = NULL;
   VkResult result;

   image = tu_image_from_handle(image_h);

   VkDeviceMemory memory_h;

   const VkMemoryDedicatedAllocateInfo ded_alloc = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO,
      .pNext = NULL,
      .image = image_h,
      .buffer = VK_NULL_HANDLE,
   };

   const VkImportMemoryFdInfoKHR import_info = {
      .sType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR,
      .pNext = &ded_alloc,
      .handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT,
      .fd = os_dupfd_cloexec(dma_buf),
   };

   const VkMemoryAllocateInfo alloc_info = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .pNext = &import_info,
      .allocationSize = image->total_size,
      .memoryTypeIndex = 0,
   };
   result = tu_AllocateMemory(device_h, &alloc_info, alloc, &memory_h);

   if (result != VK_SUCCESS) {
      tu_DestroyImage(device_h, image_h, alloc);
      return result;
   }

   VkBindImageMemoryInfo bind_info = {
      .sType = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO,
      .image = image_h,
      .memory = memory_h,
      .memoryOffset = 0,
   };
   tu_BindImageMemory2(device_h, 1, &bind_info);

   image->owned_memory = memory_h;

   return VK_SUCCESS;
}

static VkResult
format_supported_with_usage(VkDevice device_h, VkFormat format,
                            VkImageUsageFlags imageUsage)
{
   TU_FROM_HANDLE(tu_device, device, device_h);
   struct tu_physical_device *phys_dev = device->physical_device;
   VkPhysicalDevice phys_dev_h = tu_physical_device_to_handle(phys_dev);
   VkResult result;

   /* WARNING: Android Nougat's libvulkan.so hardcodes the VkImageUsageFlags
    * returned to applications via
    * VkSurfaceCapabilitiesKHR::supportedUsageFlags.
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
   result = tu_GetPhysicalDeviceImageFormatProperties2(
      phys_dev_h, &image_format_info, &image_format_props);
   if (result != VK_SUCCESS) {
      return vk_errorf(device, result,
                       "tu_GetPhysicalDeviceImageFormatProperties2 failed "
                       "inside %s",
                       __func__);
   }

   return VK_SUCCESS;
}

static VkResult
setup_gralloc0_usage(struct tu_device *device, VkFormat format,
                     VkImageUsageFlags imageUsage, int *grallocUsage)
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

   /*
    * FINISHME: Advertise all display-supported formats. Mostly
    * DRM_FORMAT_ARGB2101010 and DRM_FORMAT_ABGR2101010, but need to check
    * what we need for 30-bit colors.
    */
   if (format == VK_FORMAT_B8G8R8A8_UNORM ||
       format == VK_FORMAT_B5G6R5_UNORM_PACK16) {
      *grallocUsage |= GRALLOC_USAGE_HW_FB | GRALLOC_USAGE_HW_COMPOSER |
                       GRALLOC_USAGE_EXTERNAL_DISP;
   }

   if (*grallocUsage == 0)
      return VK_ERROR_FORMAT_NOT_SUPPORTED;

   return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL
tu_GetSwapchainGrallocUsageANDROID(VkDevice device_h,
                                   VkFormat format,
                                   VkImageUsageFlags imageUsage,
                                   int *grallocUsage)
{
   TU_FROM_HANDLE(tu_device, device, device_h);
   VkResult result;

   result = format_supported_with_usage(device_h, format, imageUsage);
   if (result != VK_SUCCESS)
      return result;

   *grallocUsage = 0;
   return setup_gralloc0_usage(device, format, imageUsage, grallocUsage);
}

#if ANDROID_API_LEVEL >= 26
VKAPI_ATTR VkResult VKAPI_CALL
tu_GetSwapchainGrallocUsage2ANDROID(VkDevice device_h,
                                    VkFormat format,
                                    VkImageUsageFlags imageUsage,
                                    VkSwapchainImageUsageFlagsANDROID swapchainImageUsage,
                                    uint64_t *grallocConsumerUsage,
                                    uint64_t *grallocProducerUsage)
{
   TU_FROM_HANDLE(tu_device, device, device_h);
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

   return VK_SUCCESS;
}
#endif
