/*
 * Copyright 2021 Google LLC
 * SPDX-License-Identifier: MIT
 *
 * based in part on anv and radv which are:
 * Copyright © 2015 Intel Corporation
 * Copyright © 2016 Red Hat
 * Copyright © 2016 Bas Nieuwenhuizen
 */

#include "vn_android.h"

#include <dlfcn.h>
#include <hardware/hwvulkan.h>
#include <vndk/hardware_buffer.h>
#include <vulkan/vk_icd.h>

#include "util/os_file.h"
#include "util/u_gralloc/u_gralloc.h"
#include "vk_android.h"

#include "vn_buffer.h"
#include "vn_device.h"
#include "vn_device_memory.h"
#include "vn_image.h"
#include "vn_instance.h"
#include "vn_physical_device.h"
#include "vn_queue.h"

struct vn_android_gralloc {
   struct u_gralloc *gralloc;
   uint64_t front_rendering_usage;
};

static struct vn_android_gralloc _vn_android_gralloc;

static int
vn_android_gralloc_init()
{
   assert(!_vn_android_gralloc.gralloc);

   struct u_gralloc *gralloc = u_gralloc_create(U_GRALLOC_TYPE_AUTO);
   if (!gralloc) {
      vn_log(NULL, "u_gralloc failed to create a gralloc module instance");
      return -1;
   }

   const int gralloc_type = u_gralloc_get_type(gralloc);
   if (gralloc_type != U_GRALLOC_TYPE_CROS &&
       gralloc_type != U_GRALLOC_TYPE_GRALLOC4) {
      u_gralloc_destroy(&gralloc);
      vn_log(NULL, "only CrOS and IMapper v4 grallocs are supported for "
                   "Venus Vulkan HAL");
      return -1;
   }

   _vn_android_gralloc.gralloc = gralloc;

   return 0;
}

static inline void
vn_android_gralloc_fini()
{
   u_gralloc_destroy(&_vn_android_gralloc.gralloc);
}

static void
vn_android_gralloc_shared_present_usage_init_once()
{
   assert(_vn_android_gralloc.gralloc);

   int ret = u_gralloc_get_front_rendering_usage(
      _vn_android_gralloc.gralloc,
      &_vn_android_gralloc.front_rendering_usage);

   if (ret == 0)
      assert(_vn_android_gralloc.front_rendering_usage);
}

uint64_t
vn_android_gralloc_get_shared_present_usage()
{
   static once_flag once = ONCE_FLAG_INIT;
   call_once(&once, vn_android_gralloc_shared_present_usage_init_once);
   return _vn_android_gralloc.front_rendering_usage;
}

struct vn_android_gralloc_buffer_properties {
   uint32_t drm_fourcc;
   uint32_t num_planes;
   uint64_t modifier;

   /* plane order matches VkImageDrmFormatModifierExplicitCreateInfoEXT */
   uint32_t offset[4];
   uint32_t stride[4];
};

static bool
vn_android_gralloc_get_buffer_properties(
   buffer_handle_t handle,
   struct vn_android_gralloc_buffer_properties *out_props)
{
   struct u_gralloc *gralloc = _vn_android_gralloc.gralloc;
   struct u_gralloc_buffer_basic_info info;

   /*
    * We only support (and care of) CrOS and IMapper v4 gralloc modules
    * at this point. They don't need the pixel stride and HAL format
    * to be provided externally to them. It allows integrating u_gralloc
    * with minimal modifications at this point.
    */
   struct u_gralloc_buffer_handle ugb_handle = {
      .handle = handle,
      .pixel_stride = 0,
      .hal_format = 0,
   };

   if (u_gralloc_get_buffer_basic_info(gralloc, &ugb_handle, &info) != 0) {
      vn_log(NULL, "u_gralloc_get_buffer_basic_info failed");
      return false;
   }

   if (info.modifier == DRM_FORMAT_MOD_INVALID) {
      vn_log(NULL, "Unexpected DRM_FORMAT_MOD_INVALID");
      return false;
   }

   assert(info.num_planes <= 4);

   out_props->drm_fourcc = info.drm_fourcc;
   out_props->num_planes = info.num_planes;
   for (uint32_t i = 0; i < info.num_planes; i++) {
      if (!info.strides[i]) {
         out_props->num_planes = i;
         break;
      }
      out_props->stride[i] = info.strides[i];
      out_props->offset[i] = info.offsets[i];
   }

   /* YVU420 has a chroma order of CrCb. So we must swap the planes for CrCb
    * to align with VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM. This is to serve
    * VkImageDrmFormatModifierExplicitCreateInfoEXT explicit plane layouts.
    */
   if (info.drm_fourcc == DRM_FORMAT_YVU420) {
      out_props->stride[1] = info.strides[2];
      out_props->offset[1] = info.offsets[2];
      out_props->stride[2] = info.strides[1];
      out_props->offset[2] = info.offsets[1];
   }

   out_props->modifier = info.modifier;

   return true;
}

static int
vn_android_gralloc_get_dma_buf_fd(const native_handle_t *handle)
{
   /* There can be multiple fds wrapped inside a native_handle_t, but we
    * expect the 1st one pointing to the dma_buf. For multi-planar format,
    * there should only exist one undelying dma_buf. The other fd(s) could be
    * dups to the same dma_buf or point to the shared memory used to store
    * gralloc buffer metadata.
    */
   assert(handle);

   if (handle->numFds < 1) {
      vn_log(NULL, "handle->numFds is %d, expected >= 1", handle->numFds);
      return -1;
   }

   if (handle->data[0] < 0) {
      vn_log(NULL, "handle->data[0] < 0");
      return -1;
   }

   return handle->data[0];
}

static int
vn_hal_open(const struct hw_module_t *mod,
            const char *id,
            struct hw_device_t **dev);

static_assert(HWVULKAN_DISPATCH_MAGIC == ICD_LOADER_MAGIC, "");

PUBLIC struct hwvulkan_module_t HAL_MODULE_INFO_SYM = {
   .common = {
      .tag = HARDWARE_MODULE_TAG,
      .module_api_version = HWVULKAN_MODULE_API_VERSION_0_1,
      .hal_api_version = HARDWARE_HAL_API_VERSION,
      .id = HWVULKAN_HARDWARE_MODULE_ID,
      .name = "Venus Vulkan HAL",
      .author = "Google LLC",
      .methods = &(hw_module_methods_t) {
         .open = vn_hal_open,
      },
   },
};

static int
vn_hal_close(UNUSED struct hw_device_t *dev)
{
   vn_android_gralloc_fini();
   return 0;
}

static hwvulkan_device_t vn_hal_dev = {
  .common = {
     .tag = HARDWARE_DEVICE_TAG,
     .version = HWVULKAN_DEVICE_API_VERSION_0_1,
     .module = &HAL_MODULE_INFO_SYM.common,
     .close = vn_hal_close,
  },
 .EnumerateInstanceExtensionProperties = vn_EnumerateInstanceExtensionProperties,
 .CreateInstance = vn_CreateInstance,
 .GetInstanceProcAddr = vn_GetInstanceProcAddr,
};

static int
vn_hal_open(const struct hw_module_t *mod,
            const char *id,
            struct hw_device_t **dev)
{
   int ret;

   assert(mod == &HAL_MODULE_INFO_SYM.common);
   assert(strcmp(id, HWVULKAN_DEVICE_0) == 0);

   ret = vn_android_gralloc_init();
   if (ret)
      return ret;

   *dev = &vn_hal_dev.common;

   return 0;
}

const VkFormat *
vn_android_format_to_view_formats(VkFormat format, uint32_t *out_count)
{
   /* For AHB image prop query and creation, venus overrides the tiling to
    * VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT, which requires to chain
    * VkImageFormatListCreateInfo struct in the corresponding pNext when the
    * VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT is set. Those AHB images are assumed
    * to be mutable no more than sRGB-ness, and the implementations can fail
    * whenever going beyond.
    *
    * This helper provides the view formats that have sRGB variants for the
    * image format that venus supports.
    */
   static const VkFormat view_formats_r8g8b8a8[] = {
      VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_SRGB
   };
   static const VkFormat view_formats_r8g8b8[] = { VK_FORMAT_R8G8B8_UNORM,
                                                   VK_FORMAT_R8G8B8_SRGB };

   switch (format) {
   case VK_FORMAT_R8G8B8A8_UNORM:
      *out_count = ARRAY_SIZE(view_formats_r8g8b8a8);
      return view_formats_r8g8b8a8;
      break;
   case VK_FORMAT_R8G8B8_UNORM:
      *out_count = ARRAY_SIZE(view_formats_r8g8b8);
      return view_formats_r8g8b8;
      break;
   default:
      /* let the caller handle the fallback case */
      *out_count = 0;
      return NULL;
   }
}

VkFormat
vn_android_drm_format_to_vk_format(uint32_t format)
{
   switch (format) {
   case DRM_FORMAT_ABGR8888:
   case DRM_FORMAT_XBGR8888:
      return VK_FORMAT_R8G8B8A8_UNORM;
   case DRM_FORMAT_BGR888:
      return VK_FORMAT_R8G8B8_UNORM;
   case DRM_FORMAT_RGB565:
      return VK_FORMAT_R5G6B5_UNORM_PACK16;
   case DRM_FORMAT_ABGR16161616F:
      return VK_FORMAT_R16G16B16A16_SFLOAT;
   case DRM_FORMAT_ABGR2101010:
      return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
   case DRM_FORMAT_YVU420:
      return VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;
   case DRM_FORMAT_NV12:
      return VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
   default:
      return VK_FORMAT_UNDEFINED;
   }
}

static bool
vn_android_drm_format_is_yuv(uint32_t format)
{
   assert(vn_android_drm_format_to_vk_format(format) != VK_FORMAT_UNDEFINED);

   switch (format) {
   case DRM_FORMAT_YVU420:
   case DRM_FORMAT_NV12:
      return true;
   default:
      return false;
   }
}

VkResult
vn_GetSwapchainGrallocUsage2ANDROID(
   VkDevice device,
   VkFormat format,
   VkImageUsageFlags imageUsage,
   VkSwapchainImageUsageFlagsANDROID swapchainImageUsage,
   uint64_t *grallocConsumerUsage,
   uint64_t *grallocProducerUsage)
{
   struct vn_device *dev = vn_device_from_handle(device);

   if (VN_DEBUG(WSI)) {
      vn_log(dev->instance,
             "format=%d, imageUsage=0x%x, swapchainImageUsage=0x%x", format,
             imageUsage, swapchainImageUsage);
   }

   *grallocConsumerUsage = 0;
   *grallocProducerUsage = 0;
   if (imageUsage & (VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT))
      *grallocProducerUsage |= AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER;

   if (imageUsage &
       (VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
        VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT))
      *grallocProducerUsage |= AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;

   if (swapchainImageUsage & VK_SWAPCHAIN_IMAGE_USAGE_SHARED_BIT_ANDROID)
      *grallocProducerUsage |= vn_android_gralloc_get_shared_present_usage();

   vn_tls_set_async_pipeline_create();

   return VK_SUCCESS;
}

static VkResult
vn_android_get_modifier_properties(struct vn_device *dev,
                                   VkFormat format,
                                   uint64_t modifier,
                                   const VkAllocationCallbacks *alloc,
                                   VkDrmFormatModifierPropertiesEXT *out_props)
{
   VkPhysicalDevice physical_device =
      vn_physical_device_to_handle(dev->physical_device);
   VkDrmFormatModifierPropertiesListEXT mod_prop_list = {
      .sType = VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_EXT,
      .pNext = NULL,
      .drmFormatModifierCount = 0,
      .pDrmFormatModifierProperties = NULL,
   };
   VkFormatProperties2 format_prop = {
      .sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2,
      .pNext = &mod_prop_list,
   };
   VkDrmFormatModifierPropertiesEXT *mod_props = NULL;
   bool modifier_found = false;

   vn_GetPhysicalDeviceFormatProperties2(physical_device, format,
                                         &format_prop);

   if (!mod_prop_list.drmFormatModifierCount) {
      vn_log(dev->instance, "No compatible modifier for VkFormat(%u)",
             format);
      return VK_ERROR_INVALID_EXTERNAL_HANDLE;
   }

   mod_props = vk_zalloc(
      alloc, sizeof(*mod_props) * mod_prop_list.drmFormatModifierCount,
      VN_DEFAULT_ALIGN, VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
   if (!mod_props)
      return VK_ERROR_OUT_OF_HOST_MEMORY;

   mod_prop_list.pDrmFormatModifierProperties = mod_props;
   vn_GetPhysicalDeviceFormatProperties2(physical_device, format,
                                         &format_prop);

   for (uint32_t i = 0; i < mod_prop_list.drmFormatModifierCount; i++) {
      if (mod_props[i].drmFormatModifier == modifier) {
         *out_props = mod_props[i];
         modifier_found = true;
         break;
      }
   }

   vk_free(alloc, mod_props);

   if (!modifier_found) {
      vn_log(dev->instance,
             "No matching modifier(%" PRIu64 ") properties for VkFormat(%u)",
             modifier, format);
      return VK_ERROR_INVALID_EXTERNAL_HANDLE;
   }

   return VK_SUCCESS;
}

struct vn_android_image_builder {
   VkImageCreateInfo create;
   VkSubresourceLayout layouts[4];
   VkImageDrmFormatModifierExplicitCreateInfoEXT modifier;
   VkExternalMemoryImageCreateInfo external;
   VkImageFormatListCreateInfo list;
};

static VkResult
vn_android_get_image_builder(struct vn_device *dev,
                             const VkImageCreateInfo *create_info,
                             const native_handle_t *handle,
                             struct vn_android_image_builder *out_builder)
{
   /* Android image builder is only used by ANB or AHB. For ANB, Android
    * Vulkan loader will never pass the below structs. For AHB, struct
    * vn_image_create_deferred_info will never carry below either.
    */
   assert(!vk_find_struct_const(
      create_info->pNext,
      IMAGE_DRM_FORMAT_MODIFIER_EXPLICIT_CREATE_INFO_EXT));
   assert(!vk_find_struct_const(create_info->pNext,
                                EXTERNAL_MEMORY_IMAGE_CREATE_INFO));

   struct vn_android_gralloc_buffer_properties buf_props;
   if (!vn_android_gralloc_get_buffer_properties(handle, &buf_props))
      return VK_ERROR_INVALID_EXTERNAL_HANDLE;

   /* fill VkImageCreateInfo */
   memset(out_builder, 0, sizeof(*out_builder));
   out_builder->create = *create_info;
   out_builder->create.tiling = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;

   /* fill VkImageDrmFormatModifierExplicitCreateInfoEXT */
   for (uint32_t i = 0; i < buf_props.num_planes; i++) {
      out_builder->layouts[i].offset = buf_props.offset[i];
      out_builder->layouts[i].rowPitch = buf_props.stride[i];
   }
   out_builder->modifier = (VkImageDrmFormatModifierExplicitCreateInfoEXT){
      .sType =
         VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_EXPLICIT_CREATE_INFO_EXT,
      .pNext = out_builder->create.pNext,
      .drmFormatModifier = buf_props.modifier,
      .drmFormatModifierPlaneCount = buf_props.num_planes,
      .pPlaneLayouts = out_builder->layouts,
   };
   out_builder->create.pNext = &out_builder->modifier;

   /* fill VkExternalMemoryImageCreateInfo */
   out_builder->external = (VkExternalMemoryImageCreateInfo){
      .sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO,
      .pNext = out_builder->create.pNext,
      .handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT,
   };
   out_builder->create.pNext = &out_builder->external;

   /* fill VkImageFormatListCreateInfo if needed
    *
    * vn_image::deferred_info only stores VkImageFormatListCreateInfo with a
    * non-zero viewFormatCount, and that stored struct will be respected.
    */
   if ((create_info->flags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT) &&
       !vk_find_struct_const(create_info->pNext,
                             IMAGE_FORMAT_LIST_CREATE_INFO)) {
      /* 12.3. Images
       *
       * If tiling is VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT and flags
       * contains VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT, then the pNext chain
       * must include a VkImageFormatListCreateInfo structure with non-zero
       * viewFormatCount.
       */
      uint32_t vcount = 0;
      const VkFormat *vformats =
         vn_android_format_to_view_formats(create_info->format, &vcount);
      if (!vformats) {
         /* image builder struct persists through the image creation call */
         vformats = &out_builder->create.format;
         vcount = 1;
      }
      out_builder->list = (VkImageFormatListCreateInfo){
         .sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO,
         .pNext = out_builder->create.pNext,
         .viewFormatCount = vcount,
         .pViewFormats = vformats,
      };
      out_builder->create.pNext = &out_builder->list;
   }

   return VK_SUCCESS;
}

VkResult
vn_android_image_from_anb(struct vn_device *dev,
                          const VkImageCreateInfo *create_info,
                          const VkNativeBufferANDROID *anb_info,
                          const VkAllocationCallbacks *alloc,
                          struct vn_image **out_img)
{
   /* If anb_info->handle points to a classic resouce created from
    * virtio_gpu_cmd_resource_create_3d, anb_info->stride is the stride of the
    * guest shadow storage other than the host gpu storage.
    *
    * We also need to pass the correct stride to vn_CreateImage, which will be
    * done via VkImageDrmFormatModifierExplicitCreateInfoEXT and will require
    * VK_EXT_image_drm_format_modifier support in the host driver. The struct
    * needs host storage info which can be queried from cros gralloc.
    */
   VkResult result = VK_SUCCESS;
   VkDevice device = vn_device_to_handle(dev);
   VkDeviceMemory memory = VK_NULL_HANDLE;
   VkImage image = VK_NULL_HANDLE;
   struct vn_image *img = NULL;
   uint64_t alloc_size = 0;
   uint32_t mem_type_bits = 0;
   int dma_buf_fd = -1;
   int dup_fd = -1;
   VkImageCreateInfo local_create_info;
   struct vn_android_image_builder builder;

   dma_buf_fd = vn_android_gralloc_get_dma_buf_fd(anb_info->handle);
   if (dma_buf_fd < 0) {
      result = VK_ERROR_INVALID_EXTERNAL_HANDLE;
      goto fail;
   }

   assert(!(create_info->flags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT));
   assert(!vk_find_struct_const(create_info->pNext,
                                IMAGE_FORMAT_LIST_CREATE_INFO));
   assert(!vk_find_struct_const(create_info->pNext,
                                IMAGE_STENCIL_USAGE_CREATE_INFO));

   /* strip VkNativeBufferANDROID and VkSwapchainImageCreateInfoANDROID */
   local_create_info = *create_info;
   local_create_info.pNext = NULL;
   result = vn_android_get_image_builder(dev, &local_create_info,
                                         anb_info->handle, &builder);
   if (result != VK_SUCCESS)
      goto fail;

   /* encoder will strip the Android specific pNext structs */
   result = vn_image_create(dev, &builder.create, alloc, &img);
   if (result != VK_SUCCESS) {
      if (VN_DEBUG(WSI))
         vn_log(dev->instance, "vn_image_create failed");
      goto fail;
   }

   image = vn_image_to_handle(img);

   const VkMemoryRequirements *mem_req =
      &img->requirements[0].memory.memoryRequirements;
   if (!mem_req->memoryTypeBits) {
      if (VN_DEBUG(WSI))
         vn_log(dev->instance, "mem_req->memoryTypeBits cannot be zero");
      result = VK_ERROR_INVALID_EXTERNAL_HANDLE;
      goto fail;
   }

   result = vn_get_memory_dma_buf_properties(dev, dma_buf_fd, &alloc_size,
                                             &mem_type_bits);
   if (result != VK_SUCCESS)
      goto fail;

   if (VN_DEBUG(WSI)) {
      vn_log(dev->instance,
             "size = img(%" PRIu64 ") fd(%" PRIu64 "), "
             "memoryTypeBits = img(0x%X) & fd(0x%X)",
             mem_req->size, alloc_size, mem_req->memoryTypeBits,
             mem_type_bits);
   }

   if (alloc_size < mem_req->size) {
      if (VN_DEBUG(WSI)) {
         vn_log(dev->instance,
                "alloc_size(%" PRIu64 ") mem_req->size(%" PRIu64 ")",
                alloc_size, mem_req->size);
      }
      result = VK_ERROR_INVALID_EXTERNAL_HANDLE;
      goto fail;
   }

   mem_type_bits &= mem_req->memoryTypeBits;
   if (!mem_type_bits) {
      result = VK_ERROR_INVALID_EXTERNAL_HANDLE;
      goto fail;
   }

   dup_fd = os_dupfd_cloexec(dma_buf_fd);
   if (dup_fd < 0) {
      result = (errno == EMFILE) ? VK_ERROR_TOO_MANY_OBJECTS
                                 : VK_ERROR_OUT_OF_HOST_MEMORY;
      goto fail;
   }

   const VkImportMemoryFdInfoKHR import_fd_info = {
      .sType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR,
      .pNext = NULL,
      .handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT,
      .fd = dup_fd,
   };
   const VkMemoryAllocateInfo memory_info = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .pNext = &import_fd_info,
      .allocationSize = mem_req->size,
      .memoryTypeIndex = ffs(mem_type_bits) - 1,
   };
   result = vn_AllocateMemory(device, &memory_info, alloc, &memory);
   if (result != VK_SUCCESS) {
      /* only need to close the dup_fd on import failure */
      close(dup_fd);
      goto fail;
   }

   const VkBindImageMemoryInfo bind_info = {
      .sType = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO,
      .pNext = NULL,
      .image = image,
      .memory = memory,
      .memoryOffset = 0,
   };
   result = vn_BindImageMemory2(device, 1, &bind_info);
   if (result != VK_SUCCESS)
      goto fail;

   img->wsi.is_wsi = true;
   img->wsi.tiling_override = builder.create.tiling;
   img->wsi.drm_format_modifier = builder.modifier.drmFormatModifier;
   /* Android WSI image owns the memory */
   img->wsi.memory = vn_device_memory_from_handle(memory);
   img->wsi.memory_owned = true;
   *out_img = img;

   return VK_SUCCESS;

fail:
   if (image != VK_NULL_HANDLE)
      vn_DestroyImage(device, image, alloc);
   if (memory != VK_NULL_HANDLE)
      vn_FreeMemory(device, memory, alloc);
   return vn_error(dev->instance, result);
}

static VkResult
vn_android_get_ahb_format_properties(
   struct vn_device *dev,
   const struct AHardwareBuffer *ahb,
   VkAndroidHardwareBufferFormatPropertiesANDROID *out_props)
{
   AHardwareBuffer_Desc desc;
   VkFormat format;
   struct vn_android_gralloc_buffer_properties buf_props;
   VkDrmFormatModifierPropertiesEXT mod_props;

   AHardwareBuffer_describe(ahb, &desc);
   if (!(desc.usage & (AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE |
                       AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER |
                       AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER))) {
      vn_log(dev->instance,
             "AHB usage(%" PRIu64 ") must include at least one GPU bit",
             desc.usage);
      return VK_ERROR_INVALID_EXTERNAL_HANDLE;
   }

   /* Handle the special AHARDWAREBUFFER_FORMAT_BLOB for VkBuffer case. */
   if (desc.format == AHARDWAREBUFFER_FORMAT_BLOB) {
      out_props->format = VK_FORMAT_UNDEFINED;
      return VK_SUCCESS;
   }

   if (!vn_android_gralloc_get_buffer_properties(
          AHardwareBuffer_getNativeHandle(ahb), &buf_props))
      return VK_ERROR_INVALID_EXTERNAL_HANDLE;

   /* We implement AHB extension support with EXT_image_drm_format_modifier.
    * It requires us to have a compatible VkFormat but not DRM formats. So if
    * the ahb is not intended for backing a VkBuffer, error out early if the
    * format is VK_FORMAT_UNDEFINED.
    */
   format = vn_android_drm_format_to_vk_format(buf_props.drm_fourcc);
   if (format == VK_FORMAT_UNDEFINED) {
      vn_log(dev->instance, "Unknown drm_fourcc(%u) from AHB format(0x%X)",
             buf_props.drm_fourcc, desc.format);
      return VK_ERROR_INVALID_EXTERNAL_HANDLE;
   }

   VkResult result = vn_android_get_modifier_properties(
      dev, format, buf_props.modifier, &dev->base.base.alloc, &mod_props);
   if (result != VK_SUCCESS)
      return result;

   if (mod_props.drmFormatModifierPlaneCount != buf_props.num_planes) {
      vn_log(dev->instance,
             "drmFormatModifierPlaneCount(%u) != buf_props.num_planes(%u) "
             "for DRM format modifier(%" PRIu64 ")",
             mod_props.drmFormatModifierPlaneCount, buf_props.num_planes,
             buf_props.modifier);
      return VK_ERROR_INVALID_EXTERNAL_HANDLE;
   }

   /* The spec requires that formatFeatures must include at least one of
    * VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT or
    * VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT.
    */
   const VkFormatFeatureFlags format_features =
      mod_props.drmFormatModifierTilingFeatures |
      VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT;

   /* 11.2.7. Android Hardware Buffer External Memory
    *
    * Implementations may not always be able to determine the color model,
    * numerical range, or chroma offsets of the image contents, so the values
    * in VkAndroidHardwareBufferFormatPropertiesANDROID are only suggestions.
    * Applications should treat these values as sensible defaults to use in the
    * absence of more reliable information obtained through some other means.
    */
   const bool is_yuv = vn_android_drm_format_is_yuv(buf_props.drm_fourcc);
   const VkSamplerYcbcrModelConversion model =
      is_yuv ? VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_601
             : VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY;

   /* ANGLE expects VK_FORMAT_UNDEFINED with externalFormat resolved from
    * AHARDWAREBUFFER_FORMAT_IMPLEMENTATION_DEFINED and any supported planar
    * AHB formats. Venus supports below explicit ones:
    * - AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420 (DRM_FORMAT_NV12)
    * - AHARDWAREBUFFER_FORMAT_YV12 (DRM_FORMAT_YVU420)
    */
   if (desc.format == AHARDWAREBUFFER_FORMAT_IMPLEMENTATION_DEFINED || is_yuv)
      format = VK_FORMAT_UNDEFINED;

   *out_props = (VkAndroidHardwareBufferFormatPropertiesANDROID) {
      .sType = out_props->sType,
      .pNext = out_props->pNext,
      .format = format,
      .externalFormat = buf_props.drm_fourcc,
      .formatFeatures = format_features,
      .samplerYcbcrConversionComponents = {
         .r = VK_COMPONENT_SWIZZLE_IDENTITY,
         .g = VK_COMPONENT_SWIZZLE_IDENTITY,
         .b = VK_COMPONENT_SWIZZLE_IDENTITY,
         .a = VK_COMPONENT_SWIZZLE_IDENTITY,
      },
      .suggestedYcbcrModel = model,
      /* match EGL_YUV_NARROW_RANGE_EXT used in egl platform_android */
      .suggestedYcbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_NARROW,
      .suggestedXChromaOffset = VK_CHROMA_LOCATION_MIDPOINT,
      .suggestedYChromaOffset = VK_CHROMA_LOCATION_MIDPOINT,
   };

   return VK_SUCCESS;
}

VkResult
vn_GetAndroidHardwareBufferPropertiesANDROID(
   VkDevice device,
   const struct AHardwareBuffer *buffer,
   VkAndroidHardwareBufferPropertiesANDROID *pProperties)
{
   VN_TRACE_FUNC();
   struct vn_device *dev = vn_device_from_handle(device);
   VkResult result = VK_SUCCESS;
   int dma_buf_fd = -1;
   uint64_t alloc_size = 0;
   uint32_t mem_type_bits = 0;

   VkAndroidHardwareBufferFormatProperties2ANDROID *format_props2 =
      vk_find_struct(pProperties->pNext,
                     ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_2_ANDROID);
   VkAndroidHardwareBufferFormatPropertiesANDROID *format_props =
      vk_find_struct(pProperties->pNext,
                     ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID);
   if (format_props2 || format_props) {
      VkAndroidHardwareBufferFormatPropertiesANDROID local_props = {
         .sType =
            VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID,
      };
      if (!format_props)
         format_props = &local_props;

      result =
         vn_android_get_ahb_format_properties(dev, buffer, format_props);
      if (result != VK_SUCCESS)
         return vn_error(dev->instance, result);

      if (format_props2) {
         format_props2->format = format_props->format;
         format_props2->externalFormat = format_props->externalFormat;
         format_props2->formatFeatures =
            (VkFormatFeatureFlags2)format_props->formatFeatures;
         format_props2->samplerYcbcrConversionComponents =
            format_props->samplerYcbcrConversionComponents;
         format_props2->suggestedYcbcrModel =
            format_props->suggestedYcbcrModel;
         format_props2->suggestedYcbcrRange =
            format_props->suggestedYcbcrRange;
         format_props2->suggestedXChromaOffset =
            format_props->suggestedXChromaOffset;
         format_props2->suggestedYChromaOffset =
            format_props->suggestedYChromaOffset;
      }
   }

   dma_buf_fd = vn_android_gralloc_get_dma_buf_fd(
      AHardwareBuffer_getNativeHandle(buffer));
   if (dma_buf_fd < 0)
      return vn_error(dev->instance, VK_ERROR_INVALID_EXTERNAL_HANDLE);

   result = vn_get_memory_dma_buf_properties(dev, dma_buf_fd, &alloc_size,
                                             &mem_type_bits);
   if (result != VK_SUCCESS)
      return vn_error(dev->instance, result);

   pProperties->allocationSize = alloc_size;
   pProperties->memoryTypeBits = mem_type_bits;

   return VK_SUCCESS;
}

static AHardwareBuffer *
vn_android_ahb_allocate(uint32_t width,
                        uint32_t height,
                        uint32_t layers,
                        uint32_t format,
                        uint64_t usage)
{
   AHardwareBuffer *ahb = NULL;
   AHardwareBuffer_Desc desc;
   int ret = 0;

   memset(&desc, 0, sizeof(desc));
   desc.width = width;
   desc.height = height;
   desc.layers = layers;
   desc.format = format;
   desc.usage = usage;

   ret = AHardwareBuffer_allocate(&desc, &ahb);
   if (ret) {
      /* We just log the error code here for now since the platform falsely
       * maps all gralloc allocation failures to oom.
       */
      vn_log(NULL, "AHB alloc(w=%u,h=%u,l=%u,f=%u,u=%" PRIu64 ") failed(%d)",
             width, height, layers, format, usage, ret);
      return NULL;
   }

   return ahb;
}

bool
vn_android_get_drm_format_modifier_info(
   const VkPhysicalDeviceImageFormatInfo2 *format_info,
   VkPhysicalDeviceImageDrmFormatModifierInfoEXT *out_info)
{
   /* To properly fill VkPhysicalDeviceImageDrmFormatModifierInfoEXT, we have
    * to allocate an ahb to retrieve the drm format modifier. For the image
    * sharing mode, we assume VK_SHARING_MODE_EXCLUSIVE for now.
    */
   AHardwareBuffer *ahb = NULL;
   uint32_t format = 0;
   uint64_t usage = 0;
   struct vn_android_gralloc_buffer_properties buf_props;

   assert(format_info->tiling == VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT);

   format = vk_image_format_to_ahb_format(format_info->format);
   if (!format)
      return false;

   usage =
      vk_image_usage_to_ahb_usage(format_info->flags, format_info->usage);
   ahb = vn_android_ahb_allocate(16, 16, 1, format, usage);
   if (!ahb)
      return false;

   if (!vn_android_gralloc_get_buffer_properties(
          AHardwareBuffer_getNativeHandle(ahb), &buf_props)) {
      AHardwareBuffer_release(ahb);
      return false;
   }

   *out_info = (VkPhysicalDeviceImageDrmFormatModifierInfoEXT){
      .sType =
         VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_DRM_FORMAT_MODIFIER_INFO_EXT,
      .pNext = NULL,
      .drmFormatModifier = buf_props.modifier,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 0,
      .pQueueFamilyIndices = NULL,
   };

   AHardwareBuffer_release(ahb);
   return true;
}

VkResult
vn_android_device_import_ahb(
   struct vn_device *dev,
   struct vn_device_memory *mem,
   const struct VkMemoryDedicatedAllocateInfo *dedicated_info)
{
   const struct vk_device_memory *mem_vk = &mem->base.base;
   const native_handle_t *handle = NULL;
   int dma_buf_fd = -1;
   int dup_fd = -1;
   uint64_t alloc_size = 0;
   uint32_t mem_type_bits = 0;
   uint32_t mem_type_index = mem_vk->memory_type_index;
   bool force_unmappable = false;
   VkResult result = VK_SUCCESS;

   handle = AHardwareBuffer_getNativeHandle(mem_vk->ahardware_buffer);
   dma_buf_fd = vn_android_gralloc_get_dma_buf_fd(handle);
   if (dma_buf_fd < 0)
      return VK_ERROR_INVALID_EXTERNAL_HANDLE;

   result = vn_get_memory_dma_buf_properties(dev, dma_buf_fd, &alloc_size,
                                             &mem_type_bits);
   if (result != VK_SUCCESS)
      return result;

   /* If ahb is for an image, finish the deferred image creation first */
   if (dedicated_info && dedicated_info->image != VK_NULL_HANDLE) {
      struct vn_image *img = vn_image_from_handle(dedicated_info->image);
      struct vn_android_image_builder builder;

      result = vn_android_get_image_builder(dev, &img->deferred_info->create,
                                            handle, &builder);
      if (result != VK_SUCCESS)
         return result;

      result = vn_image_init_deferred(dev, &builder.create, img);
      if (result != VK_SUCCESS)
         return result;

      const VkMemoryRequirements *mem_req =
         &img->requirements[0].memory.memoryRequirements;
      if (alloc_size < mem_req->size) {
         vn_log(dev->instance,
                "alloc_size(%" PRIu64 ") mem_req->size(%" PRIu64 ")",
                alloc_size, mem_req->size);
         return VK_ERROR_INVALID_EXTERNAL_HANDLE;
      }

      alloc_size = mem_req->size;

      /* Per spec 11.2.3. Device Memory Allocation
       *
       * If the parameters define an export operation and the external handle
       * type is
       * VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID,
       * implementations should not strictly follow memoryTypeIndex. Instead,
       * they should modify the allocation internally to use the required
       * memory type for the application’s given usage. This is because for an
       * export operation, there is currently no way for the client to know
       * the memory type index before allocating.
       */
      if (!(mem_vk->import_handle_type &
            VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID)) {
         if ((mem_type_bits & mem_req->memoryTypeBits) == 0) {
            vn_log(dev->instance, "memoryTypeBits: img(0x%X) fd(0x%X)",
                   mem_req->memoryTypeBits, mem_type_bits);
            return VK_ERROR_INVALID_EXTERNAL_HANDLE;
         }

         mem_type_index = ffs(mem_type_bits & mem_req->memoryTypeBits) - 1;
      }

      /* XXX Workaround before we use cross-domain backend in minigbm. The
       * blob_mem allocated from virgl backend can have a queried guest
       * mappable size smaller than the size returned from image memory
       * requirement.
       */
      force_unmappable = true;
   }

   if (dedicated_info && dedicated_info->buffer != VK_NULL_HANDLE) {
      struct vn_buffer *buf = vn_buffer_from_handle(dedicated_info->buffer);
      const VkMemoryRequirements *mem_req =
         &buf->requirements.memory.memoryRequirements;
      if (alloc_size < mem_req->size) {
         vn_log(dev->instance,
                "alloc_size(%" PRIu64 ") mem_req->size(%" PRIu64 ")",
                alloc_size, mem_req->size);
         return VK_ERROR_INVALID_EXTERNAL_HANDLE;
      }

      alloc_size = mem_req->size;

      assert((1 << mem_type_index) & mem_req->memoryTypeBits);
   }

   assert((1 << mem_type_index) & mem_type_bits);

   errno = 0;
   dup_fd = os_dupfd_cloexec(dma_buf_fd);
   if (dup_fd < 0)
      return (errno == EMFILE) ? VK_ERROR_TOO_MANY_OBJECTS
                               : VK_ERROR_OUT_OF_HOST_MEMORY;

   /* Spec requires AHB export info to be present, so we must strip it. In
    * practice, the AHB import path here only needs the main allocation info
    * and the dedicated_info.
    */
   VkMemoryDedicatedAllocateInfo local_dedicated_info;
   /* Override when dedicated_info exists and is not the tail struct. */
   if (dedicated_info && dedicated_info->pNext) {
      local_dedicated_info = *dedicated_info;
      local_dedicated_info.pNext = NULL;
      dedicated_info = &local_dedicated_info;
   }
   const VkMemoryAllocateInfo local_alloc_info = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .pNext = dedicated_info,
      .allocationSize = alloc_size,
      .memoryTypeIndex = mem_type_index,
   };
   result = vn_device_memory_import_dma_buf(dev, mem, &local_alloc_info,
                                            force_unmappable, dup_fd);
   if (result != VK_SUCCESS) {
      close(dup_fd);
      return result;
   }

   return VK_SUCCESS;
}

uint32_t
vn_android_get_ahb_buffer_memory_type_bits(struct vn_device *dev)
{
   static const uint32_t format = AHARDWAREBUFFER_FORMAT_BLOB;
   /* ensure dma_buf_memory_type_bits covers host visible usage */
   static const uint64_t usage = AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER |
                                 AHARDWAREBUFFER_USAGE_CPU_READ_RARELY |
                                 AHARDWAREBUFFER_USAGE_CPU_WRITE_RARELY;
   AHardwareBuffer *ahb = vn_android_ahb_allocate(4096, 1, 1, format, usage);
   if (!ahb)
      return 0;

   int dma_buf_fd =
      vn_android_gralloc_get_dma_buf_fd(AHardwareBuffer_getNativeHandle(ahb));
   if (dma_buf_fd < 0) {
      AHardwareBuffer_release(ahb);
      return 0;
   }

   uint64_t alloc_size = 0;
   uint32_t mem_type_bits = 0;
   VkResult ret = vn_get_memory_dma_buf_properties(
      dev, dma_buf_fd, &alloc_size, &mem_type_bits);
   /* release ahb first as below no longer needs it */
   AHardwareBuffer_release(ahb);

   if (ret != VK_SUCCESS) {
      vn_log(dev->instance, "AHB buffer mem type bits query failed %d", ret);
      return 0;
   }

   return mem_type_bits;
}
