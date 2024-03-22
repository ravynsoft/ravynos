/*
 * Copyright 2021 Red Hat, Inc.
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

/** VK_EXT_headless_surface */

#include "util/macros.h"
#include "util/hash_table.h"
#include "util/timespec.h"
#include "util/u_thread.h"
#include "util/xmlconfig.h"
#include "vk_util.h"
#include "vk_enum_to_str.h"
#include "vk_instance.h"
#include "vk_physical_device.h"
#include "wsi_common_entrypoints.h"
#include "wsi_common_private.h"
#include "wsi_common_queue.h"

#include "drm-uapi/drm_fourcc.h"

struct wsi_headless_format {
   VkFormat        format;
   struct u_vector modifiers;
};

struct wsi_headless {
   struct wsi_interface base;

   struct wsi_device *wsi;

   const VkAllocationCallbacks *alloc;
   VkPhysicalDevice physical_device;
};

static VkResult
wsi_headless_surface_get_support(VkIcdSurfaceBase *surface,
                                 struct wsi_device *wsi_device,
                                 uint32_t queueFamilyIndex,
                                 VkBool32* pSupported)
{
   *pSupported = true;

   return VK_SUCCESS;
}

static const VkPresentModeKHR present_modes[] = {
   VK_PRESENT_MODE_MAILBOX_KHR,
   VK_PRESENT_MODE_FIFO_KHR,
};

static VkResult
wsi_headless_surface_get_capabilities(VkIcdSurfaceBase *surface,
                                      struct wsi_device *wsi_device,
                                      VkSurfaceCapabilitiesKHR* caps)
{
   /* For true mailbox mode, we need at least 4 images:
    *  1) One to scan out from
    *  2) One to have queued for scan-out
    *  3) One to be currently held by the Wayland compositor
    *  4) One to render to
    */
   caps->minImageCount = 4;
   /* There is no real maximum */
   caps->maxImageCount = 0;

   caps->currentExtent = (VkExtent2D) { -1, -1 };
   caps->minImageExtent = (VkExtent2D) { 1, 1 };
   caps->maxImageExtent = (VkExtent2D) {
      wsi_device->maxImageDimension2D,
      wsi_device->maxImageDimension2D,
   };

   caps->supportedTransforms = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
   caps->currentTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
   caps->maxImageArrayLayers = 1;

   caps->supportedCompositeAlpha =
      VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR |
      VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;

   caps->supportedUsageFlags = wsi_caps_get_image_usage();

   VK_FROM_HANDLE(vk_physical_device, pdevice, wsi_device->pdevice);
   if (pdevice->supported_extensions.EXT_attachment_feedback_loop_layout)
      caps->supportedUsageFlags |= VK_IMAGE_USAGE_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT;

   return VK_SUCCESS;
}

static VkResult
wsi_headless_surface_get_capabilities2(VkIcdSurfaceBase *surface,
                                       struct wsi_device *wsi_device,
                                       const void *info_next,
                                       VkSurfaceCapabilities2KHR* caps)
{
   assert(caps->sType == VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR);

   VkResult result =
      wsi_headless_surface_get_capabilities(surface, wsi_device,
                                      &caps->surfaceCapabilities);

   vk_foreach_struct(ext, caps->pNext) {
      switch (ext->sType) {
      case VK_STRUCTURE_TYPE_SURFACE_PROTECTED_CAPABILITIES_KHR: {
         VkSurfaceProtectedCapabilitiesKHR *protected = (void *)ext;
         protected->supportsProtected = VK_FALSE;
         break;
      }

      default:
         /* Ignored */
         break;
      }
   }

   return result;
}

static VkResult
wsi_headless_surface_get_formats(VkIcdSurfaceBase *icd_surface,
                                 struct wsi_device *wsi_device,
                                 uint32_t* pSurfaceFormatCount,
                                 VkSurfaceFormatKHR* pSurfaceFormats)
{
   struct wsi_headless *wsi =
      (struct wsi_headless *)wsi_device->wsi[VK_ICD_WSI_PLATFORM_HEADLESS];

   VK_OUTARRAY_MAKE_TYPED(VkSurfaceFormatKHR, out, pSurfaceFormats, pSurfaceFormatCount);

   if (wsi->wsi->force_bgra8_unorm_first) {
      vk_outarray_append_typed(VkSurfaceFormatKHR, &out, out_fmt) {
         out_fmt->format = VK_FORMAT_B8G8R8A8_UNORM;
         out_fmt->colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
      }
      vk_outarray_append_typed(VkSurfaceFormatKHR, &out, out_fmt) {
         out_fmt->format = VK_FORMAT_R8G8B8A8_UNORM;
         out_fmt->colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
      }
   } else {
      vk_outarray_append_typed(VkSurfaceFormatKHR, &out, out_fmt) {
         out_fmt->format = VK_FORMAT_R8G8B8A8_UNORM;
         out_fmt->colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
      }
      vk_outarray_append_typed(VkSurfaceFormatKHR, &out, out_fmt) {
         out_fmt->format = VK_FORMAT_B8G8R8A8_UNORM;
         out_fmt->colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
      }
   }

   return vk_outarray_status(&out);
}

static VkResult
wsi_headless_surface_get_formats2(VkIcdSurfaceBase *icd_surface,
                                  struct wsi_device *wsi_device,
                                  const void *info_next,
                                  uint32_t* pSurfaceFormatCount,
                                  VkSurfaceFormat2KHR* pSurfaceFormats)
{
   struct wsi_headless *wsi =
      (struct wsi_headless *)wsi_device->wsi[VK_ICD_WSI_PLATFORM_HEADLESS];

   VK_OUTARRAY_MAKE_TYPED(VkSurfaceFormat2KHR, out, pSurfaceFormats, pSurfaceFormatCount);

   if (wsi->wsi->force_bgra8_unorm_first) {
      vk_outarray_append_typed(VkSurfaceFormat2KHR, &out, out_fmt) {
         out_fmt->surfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
         out_fmt->surfaceFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
      }
      vk_outarray_append_typed(VkSurfaceFormat2KHR, &out, out_fmt) {
         out_fmt->surfaceFormat.format = VK_FORMAT_R8G8B8A8_UNORM;
         out_fmt->surfaceFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
      }
   } else {
      vk_outarray_append_typed(VkSurfaceFormat2KHR, &out, out_fmt) {
         out_fmt->surfaceFormat.format = VK_FORMAT_R8G8B8A8_UNORM;
         out_fmt->surfaceFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
      }
      vk_outarray_append_typed(VkSurfaceFormat2KHR, &out, out_fmt) {
         out_fmt->surfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
         out_fmt->surfaceFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
      }
   }

   return vk_outarray_status(&out);
}

static VkResult
wsi_headless_surface_get_present_modes(VkIcdSurfaceBase *surface,
                                       struct wsi_device *wsi_device,
                                       uint32_t* pPresentModeCount,
                                       VkPresentModeKHR* pPresentModes)
{
   if (pPresentModes == NULL) {
      *pPresentModeCount = ARRAY_SIZE(present_modes);
      return VK_SUCCESS;
   }

   *pPresentModeCount = MIN2(*pPresentModeCount, ARRAY_SIZE(present_modes));
   typed_memcpy(pPresentModes, present_modes, *pPresentModeCount);

   if (*pPresentModeCount < ARRAY_SIZE(present_modes))
      return VK_INCOMPLETE;
   else
      return VK_SUCCESS;
}

static VkResult
wsi_headless_surface_get_present_rectangles(VkIcdSurfaceBase *surface,
                                            struct wsi_device *wsi_device,
                                            uint32_t* pRectCount,
                                            VkRect2D* pRects)
{
   VK_OUTARRAY_MAKE_TYPED(VkRect2D, out, pRects, pRectCount);

   vk_outarray_append_typed(VkRect2D, &out, rect) {
      /* We don't know a size so just return the usual "I don't know." */
      *rect = (VkRect2D) {
         .offset = { 0, 0 },
         .extent = { UINT32_MAX, UINT32_MAX },
      };
   }

   return vk_outarray_status(&out);
}

struct wsi_headless_image {
   struct wsi_image                             base;
   bool                                         busy;
};

struct wsi_headless_swapchain {
   struct wsi_swapchain                        base;

   VkExtent2D                                  extent;
   VkFormat                                    vk_format;

   struct u_vector                             modifiers;

   VkPresentModeKHR                            present_mode;
   bool                                        fifo_ready;

   struct wsi_headless_image                       images[0];
};
VK_DEFINE_NONDISP_HANDLE_CASTS(wsi_headless_swapchain, base.base, VkSwapchainKHR,
                               VK_OBJECT_TYPE_SWAPCHAIN_KHR)

static struct wsi_image *
wsi_headless_swapchain_get_wsi_image(struct wsi_swapchain *wsi_chain,
                                     uint32_t image_index)
{
   struct wsi_headless_swapchain *chain =
      (struct wsi_headless_swapchain *)wsi_chain;
   return &chain->images[image_index].base;
}

static VkResult
wsi_headless_swapchain_acquire_next_image(struct wsi_swapchain *wsi_chain,
                                          const VkAcquireNextImageInfoKHR *info,
                                          uint32_t *image_index)
{
   struct wsi_headless_swapchain *chain =
      (struct wsi_headless_swapchain *)wsi_chain;
   struct timespec start_time, end_time;
   struct timespec rel_timeout;

   timespec_from_nsec(&rel_timeout, info->timeout);

   clock_gettime(CLOCK_MONOTONIC, &start_time);
   timespec_add(&end_time, &rel_timeout, &start_time);

   while (1) {
      /* Try to find a free image. */
      for (uint32_t i = 0; i < chain->base.image_count; i++) {
         if (!chain->images[i].busy) {
            /* We found a non-busy image */
            *image_index = i;
            chain->images[i].busy = true;
            return VK_SUCCESS;
         }
      }

      /* Check for timeout. */
      struct timespec current_time;
      clock_gettime(CLOCK_MONOTONIC, &current_time);
      if (timespec_after(&current_time, &end_time))
         return VK_NOT_READY;
   }
}

static VkResult
wsi_headless_swapchain_queue_present(struct wsi_swapchain *wsi_chain,
                                     uint32_t image_index,
                                     uint64_t present_id,
                                     const VkPresentRegionKHR *damage)
{
   struct wsi_headless_swapchain *chain =
      (struct wsi_headless_swapchain *)wsi_chain;

   assert(image_index < chain->base.image_count);

   chain->images[image_index].busy = false;

   return VK_SUCCESS;
}

static VkResult
wsi_headless_swapchain_destroy(struct wsi_swapchain *wsi_chain,
                               const VkAllocationCallbacks *pAllocator)
{
   struct wsi_headless_swapchain *chain =
      (struct wsi_headless_swapchain *)wsi_chain;

   for (uint32_t i = 0; i < chain->base.image_count; i++) {
      if (chain->images[i].base.image != VK_NULL_HANDLE)
         wsi_destroy_image(&chain->base, &chain->images[i].base);
   }

   u_vector_finish(&chain->modifiers);

   wsi_swapchain_finish(&chain->base);

   vk_free(pAllocator, chain);

   return VK_SUCCESS;
}

static const struct VkDrmFormatModifierPropertiesEXT *
get_modifier_props(const struct wsi_image_info *info, uint64_t modifier)
{
   for (uint32_t i = 0; i < info->modifier_prop_count; i++) {
      if (info->modifier_props[i].drmFormatModifier == modifier)
         return &info->modifier_props[i];
   }
   return NULL;
}

static VkResult
wsi_create_null_image_mem(const struct wsi_swapchain *chain,
                          const struct wsi_image_info *info,
                          struct wsi_image *image)
{
   const struct wsi_device *wsi = chain->wsi;
   VkResult result;

   VkMemoryRequirements reqs;
   wsi->GetImageMemoryRequirements(chain->device, image->image, &reqs);

   const VkMemoryDedicatedAllocateInfo memory_dedicated_info = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO,
      .pNext = NULL,
      .image = image->image,
      .buffer = VK_NULL_HANDLE,
   };
   const VkMemoryAllocateInfo memory_info = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .pNext = &memory_dedicated_info,
      .allocationSize = reqs.size,
      .memoryTypeIndex =
         wsi_select_device_memory_type(wsi, reqs.memoryTypeBits),
   };
   result = wsi->AllocateMemory(chain->device, &memory_info,
                                &chain->alloc, &image->memory);
   if (result != VK_SUCCESS)
      return result;

   image->dma_buf_fd = -1;

   if (info->drm_mod_list.drmFormatModifierCount > 0) {
      VkImageDrmFormatModifierPropertiesEXT image_mod_props = {
         .sType = VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_PROPERTIES_EXT,
      };
      result = wsi->GetImageDrmFormatModifierPropertiesEXT(chain->device,
                                                           image->image,
                                                           &image_mod_props);
      if (result != VK_SUCCESS)
         return result;

      image->drm_modifier = image_mod_props.drmFormatModifier;
      assert(image->drm_modifier != DRM_FORMAT_MOD_INVALID);

      const struct VkDrmFormatModifierPropertiesEXT *mod_props =
         get_modifier_props(info, image->drm_modifier);
      image->num_planes = mod_props->drmFormatModifierPlaneCount;

      for (uint32_t p = 0; p < image->num_planes; p++) {
         const VkImageSubresource image_subresource = {
            .aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT << p,
            .mipLevel = 0,
            .arrayLayer = 0,
         };
         VkSubresourceLayout image_layout;
         wsi->GetImageSubresourceLayout(chain->device, image->image,
                                        &image_subresource, &image_layout);
         image->sizes[p] = image_layout.size;
         image->row_pitches[p] = image_layout.rowPitch;
         image->offsets[p] = image_layout.offset;
      }
   } else {
      const VkImageSubresource image_subresource = {
         .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
         .mipLevel = 0,
         .arrayLayer = 0,
      };
      VkSubresourceLayout image_layout;
      wsi->GetImageSubresourceLayout(chain->device, image->image,
                                     &image_subresource, &image_layout);

      image->drm_modifier = DRM_FORMAT_MOD_INVALID;
      image->num_planes = 1;
      image->sizes[0] = reqs.size;
      image->row_pitches[0] = image_layout.rowPitch;
      image->offsets[0] = 0;
   }

   return VK_SUCCESS;
}

static VkResult
wsi_headless_surface_create_swapchain(VkIcdSurfaceBase *icd_surface,
                                      VkDevice device,
                                      struct wsi_device *wsi_device,
                                      const VkSwapchainCreateInfoKHR* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator,
                                      struct wsi_swapchain **swapchain_out)
{
   struct wsi_headless_swapchain *chain;
   VkResult result;

   assert(pCreateInfo->sType == VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR);

   int num_images = pCreateInfo->minImageCount;

   size_t size = sizeof(*chain) + num_images * sizeof(chain->images[0]);
   chain = vk_zalloc(pAllocator, size, 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (chain == NULL)
      return VK_ERROR_OUT_OF_HOST_MEMORY;

   struct wsi_drm_image_params drm_params = {
      .base.image_type = WSI_IMAGE_TYPE_DRM,
      .same_gpu = true,
   };

   result = wsi_swapchain_init(wsi_device, &chain->base, device,
                               pCreateInfo, &drm_params.base, pAllocator);
   if (result != VK_SUCCESS) {
      vk_free(pAllocator, chain);
      return result;
   }

   chain->base.destroy = wsi_headless_swapchain_destroy;
   chain->base.get_wsi_image = wsi_headless_swapchain_get_wsi_image;
   chain->base.acquire_next_image = wsi_headless_swapchain_acquire_next_image;
   chain->base.queue_present = wsi_headless_swapchain_queue_present;
   chain->base.present_mode = wsi_swapchain_get_present_mode(wsi_device, pCreateInfo);
   chain->base.image_count = num_images;
   chain->extent = pCreateInfo->imageExtent;
   chain->vk_format = pCreateInfo->imageFormat;

   result = wsi_configure_image(&chain->base, pCreateInfo,
                                0, &chain->base.image_info);
   if (result != VK_SUCCESS) {
      goto fail;
   }
   chain->base.image_info.create_mem = wsi_create_null_image_mem;


   for (uint32_t i = 0; i < chain->base.image_count; i++) {
      result = wsi_create_image(&chain->base, &chain->base.image_info,
                                &chain->images[i].base);
      if (result != VK_SUCCESS)
         return result;

      chain->images[i].busy = false;
   }

   *swapchain_out = &chain->base;

   return VK_SUCCESS;

fail:
   wsi_headless_swapchain_destroy(&chain->base, pAllocator);

   return result;
}

VkResult
wsi_headless_init_wsi(struct wsi_device *wsi_device,
                      const VkAllocationCallbacks *alloc,
                      VkPhysicalDevice physical_device)
{
   struct wsi_headless *wsi;
   VkResult result;

   wsi = vk_alloc(alloc, sizeof(*wsi), 8,
                   VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
   if (!wsi) {
      result = VK_ERROR_OUT_OF_HOST_MEMORY;
      goto fail;
   }

   wsi->physical_device = physical_device;
   wsi->alloc = alloc;
   wsi->wsi = wsi_device;

   wsi->base.get_support = wsi_headless_surface_get_support;
   wsi->base.get_capabilities2 = wsi_headless_surface_get_capabilities2;
   wsi->base.get_formats = wsi_headless_surface_get_formats;
   wsi->base.get_formats2 = wsi_headless_surface_get_formats2;
   wsi->base.get_present_modes = wsi_headless_surface_get_present_modes;
   wsi->base.get_present_rectangles = wsi_headless_surface_get_present_rectangles;
   wsi->base.create_swapchain = wsi_headless_surface_create_swapchain;

   wsi_device->wsi[VK_ICD_WSI_PLATFORM_HEADLESS] = &wsi->base;

   return VK_SUCCESS;

fail:
   wsi_device->wsi[VK_ICD_WSI_PLATFORM_HEADLESS] = NULL;

   return result;
}

void
wsi_headless_finish_wsi(struct wsi_device *wsi_device,
                        const VkAllocationCallbacks *alloc)
{
   struct wsi_headless *wsi =
      (struct wsi_headless *)wsi_device->wsi[VK_ICD_WSI_PLATFORM_HEADLESS];
   if (!wsi)
      return;

   vk_free(alloc, wsi);
}

VkResult wsi_CreateHeadlessSurfaceEXT(
    VkInstance                                  _instance,
    const VkHeadlessSurfaceCreateInfoEXT*       pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface)
{
   VK_FROM_HANDLE(vk_instance, instance, _instance);
   VkIcdSurfaceHeadless *surface;

   surface = vk_alloc2(&instance->alloc, pAllocator, sizeof *surface, 8,
                       VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (surface == NULL)
      return VK_ERROR_OUT_OF_HOST_MEMORY;

   surface->base.platform = VK_ICD_WSI_PLATFORM_HEADLESS;

   *pSurface = VkIcdSurfaceBase_to_handle(&surface->base);
   return VK_SUCCESS;
}
