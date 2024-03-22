/*
 * Copyright © 2021 Red Hat
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

#include "anv_private.h"

#include "vk_video/vulkan_video_codecs_common.h"

VkResult
anv_CreateVideoSessionKHR(VkDevice _device,
                           const VkVideoSessionCreateInfoKHR *pCreateInfo,
                           const VkAllocationCallbacks *pAllocator,
                           VkVideoSessionKHR *pVideoSession)
{
   ANV_FROM_HANDLE(anv_device, device, _device);

   struct anv_video_session *vid =
      vk_alloc2(&device->vk.alloc, pAllocator, sizeof(*vid), 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (!vid)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   memset(vid, 0, sizeof(struct anv_video_session));

   VkResult result = vk_video_session_init(&device->vk,
                                           &vid->vk,
                                           pCreateInfo);
   if (result != VK_SUCCESS) {
      vk_free2(&device->vk.alloc, pAllocator, vid);
      return result;
   }

   *pVideoSession = anv_video_session_to_handle(vid);
   return VK_SUCCESS;
}

void
anv_DestroyVideoSessionKHR(VkDevice _device,
                           VkVideoSessionKHR _session,
                           const VkAllocationCallbacks *pAllocator)
{
   ANV_FROM_HANDLE(anv_device, device, _device);
   ANV_FROM_HANDLE(anv_video_session, vid, _session);
   if (!_session)
      return;

   vk_object_base_finish(&vid->vk.base);
   vk_free2(&device->vk.alloc, pAllocator, vid);
}

VkResult
anv_CreateVideoSessionParametersKHR(VkDevice _device,
                                     const VkVideoSessionParametersCreateInfoKHR *pCreateInfo,
                                     const VkAllocationCallbacks *pAllocator,
                                     VkVideoSessionParametersKHR *pVideoSessionParameters)
{
   ANV_FROM_HANDLE(anv_device, device, _device);
   ANV_FROM_HANDLE(anv_video_session, vid, pCreateInfo->videoSession);
   ANV_FROM_HANDLE(anv_video_session_params, templ, pCreateInfo->videoSessionParametersTemplate);
   struct anv_video_session_params *params =
      vk_alloc2(&device->vk.alloc, pAllocator, sizeof(*params), 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (!params)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   VkResult result = vk_video_session_parameters_init(&device->vk,
                                                      &params->vk,
                                                      &vid->vk,
                                                      templ ? &templ->vk : NULL,
                                                      pCreateInfo);
   if (result != VK_SUCCESS) {
      vk_free2(&device->vk.alloc, pAllocator, params);
      return result;
   }

   *pVideoSessionParameters = anv_video_session_params_to_handle(params);
   return VK_SUCCESS;
}

void
anv_DestroyVideoSessionParametersKHR(VkDevice _device,
                                      VkVideoSessionParametersKHR _params,
                                      const VkAllocationCallbacks *pAllocator)
{
   ANV_FROM_HANDLE(anv_device, device, _device);
   ANV_FROM_HANDLE(anv_video_session_params, params, _params);
   if (!_params)
      return;
   vk_video_session_parameters_finish(&device->vk, &params->vk);
   vk_free2(&device->vk.alloc, pAllocator, params);
}

VkResult
anv_GetPhysicalDeviceVideoCapabilitiesKHR(VkPhysicalDevice physicalDevice,
                                           const VkVideoProfileInfoKHR *pVideoProfile,
                                           VkVideoCapabilitiesKHR *pCapabilities)
{
   ANV_FROM_HANDLE(anv_physical_device, pdevice, physicalDevice);

   pCapabilities->minBitstreamBufferOffsetAlignment = 32;
   pCapabilities->minBitstreamBufferSizeAlignment = 32;
   pCapabilities->maxCodedExtent.width = 4096;
   pCapabilities->maxCodedExtent.height = 4096;
   pCapabilities->flags = VK_VIDEO_CAPABILITY_SEPARATE_REFERENCE_IMAGES_BIT_KHR;

   struct VkVideoDecodeCapabilitiesKHR *dec_caps = (struct VkVideoDecodeCapabilitiesKHR *)
      vk_find_struct(pCapabilities->pNext, VIDEO_DECODE_CAPABILITIES_KHR);
   if (dec_caps)
      dec_caps->flags = VK_VIDEO_DECODE_CAPABILITY_DPB_AND_OUTPUT_COINCIDE_BIT_KHR;

   /* H264 allows different luma and chroma bit depths */
   if (pVideoProfile->lumaBitDepth != pVideoProfile->chromaBitDepth)
      return VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR;

   if (pVideoProfile->chromaSubsampling != VK_VIDEO_CHROMA_SUBSAMPLING_420_BIT_KHR)
      return VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR;

   switch (pVideoProfile->videoCodecOperation) {
   case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR: {
      struct VkVideoDecodeH264CapabilitiesKHR *ext = (struct VkVideoDecodeH264CapabilitiesKHR *)
         vk_find_struct(pCapabilities->pNext, VIDEO_DECODE_H264_CAPABILITIES_KHR);

      if (pVideoProfile->lumaBitDepth != VK_VIDEO_COMPONENT_BIT_DEPTH_8_BIT_KHR)
         return VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR;

      pCapabilities->maxDpbSlots = 17;
      pCapabilities->maxActiveReferencePictures = ANV_VIDEO_H264_MAX_NUM_REF_FRAME;
      pCapabilities->pictureAccessGranularity.width = ANV_MB_WIDTH;
      pCapabilities->pictureAccessGranularity.height = ANV_MB_HEIGHT;
      pCapabilities->minCodedExtent.width = ANV_MB_WIDTH;
      pCapabilities->minCodedExtent.height = ANV_MB_HEIGHT;

      ext->fieldOffsetGranularity.x = 0;
      ext->fieldOffsetGranularity.y = 0;
      ext->maxLevelIdc = STD_VIDEO_H264_LEVEL_IDC_5_1;
      strcpy(pCapabilities->stdHeaderVersion.extensionName, VK_STD_VULKAN_VIDEO_CODEC_H264_DECODE_EXTENSION_NAME);
      pCapabilities->stdHeaderVersion.specVersion = VK_STD_VULKAN_VIDEO_CODEC_H264_DECODE_SPEC_VERSION;
      break;
   }
   case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR: {
      struct VkVideoDecodeH265CapabilitiesKHR *ext = (struct VkVideoDecodeH265CapabilitiesKHR *)
         vk_find_struct(pCapabilities->pNext, VIDEO_DECODE_H265_CAPABILITIES_KHR);

      const struct VkVideoDecodeH265ProfileInfoKHR *h265_profile =
         vk_find_struct_const(pVideoProfile->pNext,
                              VIDEO_DECODE_H265_PROFILE_INFO_KHR);

      /* No hardware supports the scc extension profile */
      if (h265_profile->stdProfileIdc != STD_VIDEO_H265_PROFILE_IDC_MAIN &&
          h265_profile->stdProfileIdc != STD_VIDEO_H265_PROFILE_IDC_MAIN_10 &&
          h265_profile->stdProfileIdc != STD_VIDEO_H265_PROFILE_IDC_MAIN_STILL_PICTURE &&
          h265_profile->stdProfileIdc != STD_VIDEO_H265_PROFILE_IDC_FORMAT_RANGE_EXTENSIONS)
         return VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR;

      /* Skylake only supports the main profile */
      if (h265_profile->stdProfileIdc != STD_VIDEO_H265_PROFILE_IDC_MAIN &&
          h265_profile->stdProfileIdc != STD_VIDEO_H265_PROFILE_IDC_MAIN_STILL_PICTURE &&
          pdevice->info.platform <= INTEL_PLATFORM_SKL)
         return VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR;

      /* Gfx10 and under don't support the range extension profile */
      if (h265_profile->stdProfileIdc != STD_VIDEO_H265_PROFILE_IDC_MAIN &&
          h265_profile->stdProfileIdc != STD_VIDEO_H265_PROFILE_IDC_MAIN_10 &&
          h265_profile->stdProfileIdc != STD_VIDEO_H265_PROFILE_IDC_MAIN_STILL_PICTURE &&
          pdevice->info.ver <= 10)
         return VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR;

      if (pVideoProfile->lumaBitDepth != VK_VIDEO_COMPONENT_BIT_DEPTH_8_BIT_KHR &&
          pVideoProfile->lumaBitDepth != VK_VIDEO_COMPONENT_BIT_DEPTH_10_BIT_KHR)
         return VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR;

      pCapabilities->pictureAccessGranularity.width = ANV_MAX_H265_CTB_SIZE;
      pCapabilities->pictureAccessGranularity.height = ANV_MAX_H265_CTB_SIZE;
      pCapabilities->minCodedExtent.width = ANV_MAX_H265_CTB_SIZE;
      pCapabilities->minCodedExtent.height = ANV_MAX_H265_CTB_SIZE;
      pCapabilities->maxDpbSlots = ANV_VIDEO_H265_MAX_NUM_REF_FRAME;
      pCapabilities->maxActiveReferencePictures = ANV_VIDEO_H265_HCP_NUM_REF_FRAME;

      ext->maxLevelIdc = STD_VIDEO_H265_LEVEL_IDC_6_2;

      strcpy(pCapabilities->stdHeaderVersion.extensionName, VK_STD_VULKAN_VIDEO_CODEC_H265_DECODE_EXTENSION_NAME);
      pCapabilities->stdHeaderVersion.specVersion = VK_STD_VULKAN_VIDEO_CODEC_H265_DECODE_SPEC_VERSION;
      break;
   }
   default:
      break;
   }
   return VK_SUCCESS;
}

VkResult
anv_GetPhysicalDeviceVideoFormatPropertiesKHR(VkPhysicalDevice physicalDevice,
                                               const VkPhysicalDeviceVideoFormatInfoKHR *pVideoFormatInfo,
                                               uint32_t *pVideoFormatPropertyCount,
                                               VkVideoFormatPropertiesKHR *pVideoFormatProperties)
{
   VK_OUTARRAY_MAKE_TYPED(VkVideoFormatPropertiesKHR, out,
                          pVideoFormatProperties,
                          pVideoFormatPropertyCount);

   bool need_10bit = false;
   const struct VkVideoProfileListInfoKHR *prof_list = (struct VkVideoProfileListInfoKHR *)
      vk_find_struct_const(pVideoFormatInfo->pNext, VIDEO_PROFILE_LIST_INFO_KHR);

   if (prof_list) {
      for (unsigned i = 0; i < prof_list->profileCount; i++) {
         const VkVideoProfileInfoKHR *profile = &prof_list->pProfiles[i];
         if (profile->lumaBitDepth & VK_VIDEO_COMPONENT_BIT_DEPTH_10_BIT_KHR ||
             profile->chromaBitDepth & VK_VIDEO_COMPONENT_BIT_DEPTH_10_BIT_KHR)
            need_10bit = true;
      }
   }

   vk_outarray_append_typed(VkVideoFormatPropertiesKHR, &out, p) {
      p->format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
      p->imageType = VK_IMAGE_TYPE_2D;
      p->imageTiling = VK_IMAGE_TILING_OPTIMAL;
      p->imageUsageFlags = pVideoFormatInfo->imageUsage;
   }

   if (need_10bit) {
      vk_outarray_append_typed(VkVideoFormatPropertiesKHR, &out, p) {
         p->format = VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16;
         p->imageType = VK_IMAGE_TYPE_2D;
         p->imageTiling = VK_IMAGE_TILING_OPTIMAL;
         p->imageUsageFlags = pVideoFormatInfo->imageUsage;
      }
   }

   return vk_outarray_status(&out);
}

static uint64_t
get_h264_video_mem_size(struct anv_video_session *vid, uint32_t mem_idx)
{
   uint32_t width_in_mb =
      align(vid->vk.max_coded.width, ANV_MB_WIDTH) / ANV_MB_WIDTH;

   switch (mem_idx) {
   case ANV_VID_MEM_H264_INTRA_ROW_STORE:
      return width_in_mb * 64;
   case ANV_VID_MEM_H264_DEBLOCK_FILTER_ROW_STORE:
      return width_in_mb * 64 * 4;
   case ANV_VID_MEM_H264_BSD_MPC_ROW_SCRATCH:
      return width_in_mb * 64 * 2;
   case ANV_VID_MEM_H264_MPR_ROW_SCRATCH:
      return width_in_mb * 64 * 2;
   default:
      unreachable("unknown memory");
   }
}

static uint64_t
get_h265_video_mem_size(struct anv_video_session *vid, uint32_t mem_idx)
{
   uint32_t bit_shift =
      vid->vk.h265.profile_idc == STD_VIDEO_H265_PROFILE_IDC_MAIN_10 ? 2 : 3;

   /* TODO. these sizes can be determined dynamically depending on ctb sizes of each slice. */
   uint32_t width_in_ctb =
      align(vid->vk.max_coded.width, ANV_MAX_H265_CTB_SIZE) / ANV_MAX_H265_CTB_SIZE;
   uint32_t height_in_ctb =
      align(vid->vk.max_coded.height, ANV_MAX_H265_CTB_SIZE) / ANV_MAX_H265_CTB_SIZE;
   uint64_t size;

   switch (mem_idx) {
   case ANV_VID_MEM_H265_DEBLOCK_FILTER_ROW_STORE_LINE:
   case ANV_VID_MEM_H265_DEBLOCK_FILTER_ROW_STORE_TILE_LINE:
      size = align(vid->vk.max_coded.width, 32) >> bit_shift;
      break;
   case ANV_VID_MEM_H265_DEBLOCK_FILTER_ROW_STORE_TILE_COLUMN:
      size = align(vid->vk.max_coded.height + 6 * height_in_ctb, 32) >> bit_shift;
      break;
   case ANV_VID_MEM_H265_METADATA_LINE:
      size = (((vid->vk.max_coded.width + 15) >> 4) * 188 + width_in_ctb * 9 + 1023) >> 9;
      break;
   case ANV_VID_MEM_H265_METADATA_TILE_LINE:
      size = (((vid->vk.max_coded.width + 15) >> 4) * 172 + width_in_ctb * 9 + 1023) >> 9;
      break;
   case ANV_VID_MEM_H265_METADATA_TILE_COLUMN:
      size = (((vid->vk.max_coded.height + 15) >> 4) * 176 + height_in_ctb * 89 + 1023) >> 9;
      break;
   case ANV_VID_MEM_H265_SAO_LINE:
      size = align((vid->vk.max_coded.width >> 1) + width_in_ctb * 3, 16) >> bit_shift;
      break;
   case ANV_VID_MEM_H265_SAO_TILE_LINE:
      size = align((vid->vk.max_coded.width >> 1) + width_in_ctb * 6, 16) >> bit_shift;
      break;
   case ANV_VID_MEM_H265_SAO_TILE_COLUMN:
      size = align((vid->vk.max_coded.height >> 1) + height_in_ctb * 6, 16) >> bit_shift;
      break;
   default:
      unreachable("unknown memory");
   }

   return size << 6;
}

static void
get_h264_video_session_mem_reqs(struct anv_video_session *vid,
                                VkVideoSessionMemoryRequirementsKHR *mem_reqs,
                                uint32_t *pVideoSessionMemoryRequirementsCount,
                                uint32_t memory_types)
{
   VK_OUTARRAY_MAKE_TYPED(VkVideoSessionMemoryRequirementsKHR,
                          out,
                          mem_reqs,
                          pVideoSessionMemoryRequirementsCount);

   for (unsigned i = 0; i < ANV_VIDEO_MEM_REQS_H264; i++) {
      uint32_t bind_index = ANV_VID_MEM_H264_INTRA_ROW_STORE + i;
      uint64_t size = get_h264_video_mem_size(vid, i);

      vk_outarray_append_typed(VkVideoSessionMemoryRequirementsKHR, &out, p) {
         p->memoryBindIndex = bind_index;
         p->memoryRequirements.size = size;
         p->memoryRequirements.alignment = 4096;
         p->memoryRequirements.memoryTypeBits = memory_types;
      }
   }
}

static void
get_h265_video_session_mem_reqs(struct anv_video_session *vid,
                                VkVideoSessionMemoryRequirementsKHR *mem_reqs,
                                uint32_t *pVideoSessionMemoryRequirementsCount,
                                uint32_t memory_types)
{
   VK_OUTARRAY_MAKE_TYPED(VkVideoSessionMemoryRequirementsKHR,
                          out,
                          mem_reqs,
                          pVideoSessionMemoryRequirementsCount);

   for (unsigned i = 0; i < ANV_VIDEO_MEM_REQS_H265; i++) {
      uint32_t bind_index =
         ANV_VID_MEM_H265_DEBLOCK_FILTER_ROW_STORE_LINE + i;
      uint64_t size = get_h265_video_mem_size(vid, i);

      vk_outarray_append_typed(VkVideoSessionMemoryRequirementsKHR, &out, p) {
         p->memoryBindIndex = bind_index;
         p->memoryRequirements.size = size;
         p->memoryRequirements.alignment = 4096;
         p->memoryRequirements.memoryTypeBits = memory_types;
      }
   }
}

VkResult
anv_GetVideoSessionMemoryRequirementsKHR(VkDevice _device,
                                         VkVideoSessionKHR videoSession,
                                         uint32_t *pVideoSessionMemoryRequirementsCount,
                                         VkVideoSessionMemoryRequirementsKHR *mem_reqs)
{
   ANV_FROM_HANDLE(anv_device, device, _device);
   ANV_FROM_HANDLE(anv_video_session, vid, videoSession);

   uint32_t memory_types = 0;
   for (uint32_t i = 0; i < device->physical->memory.type_count; i++) {
      /* Have the protected buffer bit match only the memory types with the
       * equivalent bit.
       */
      if (!!(vid->vk.flags & VK_VIDEO_SESSION_CREATE_PROTECTED_CONTENT_BIT_KHR) !=
          !!(device->physical->memory.types[i].propertyFlags &
             VK_MEMORY_PROPERTY_PROTECTED_BIT))
         continue;

      memory_types |= 1ull << i;
   }

   switch (vid->vk.op) {
   case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR:
      get_h264_video_session_mem_reqs(vid,
                                      mem_reqs,
                                      pVideoSessionMemoryRequirementsCount,
                                      memory_types);
      break;
   case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR:
      get_h265_video_session_mem_reqs(vid,
                                      mem_reqs,
                                      pVideoSessionMemoryRequirementsCount,
                                      memory_types);
      break;
   default:
      unreachable("unknown codec");
   }

   return VK_SUCCESS;
}

VkResult
anv_UpdateVideoSessionParametersKHR(VkDevice _device,
                                     VkVideoSessionParametersKHR _params,
                                     const VkVideoSessionParametersUpdateInfoKHR *pUpdateInfo)
{
   ANV_FROM_HANDLE(anv_video_session_params, params, _params);
   return vk_video_session_parameters_update(&params->vk, pUpdateInfo);
}

static void
copy_bind(struct anv_vid_mem *dst,
          const VkBindVideoSessionMemoryInfoKHR *src)
{
   dst->mem = anv_device_memory_from_handle(src->memory);
   dst->offset = src->memoryOffset;
   dst->size = src->memorySize;
}

VkResult
anv_BindVideoSessionMemoryKHR(VkDevice _device,
                              VkVideoSessionKHR videoSession,
                              uint32_t bind_mem_count,
                              const VkBindVideoSessionMemoryInfoKHR *bind_mem)
{
   ANV_FROM_HANDLE(anv_video_session, vid, videoSession);

   switch (vid->vk.op) {
   case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR:
   case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR:
      for (unsigned i = 0; i < bind_mem_count; i++) {
         copy_bind(&vid->vid_mem[bind_mem[i].memoryBindIndex], &bind_mem[i]);
      }
      break;
   default:
      unreachable("unknown codec");
   }
   return VK_SUCCESS;
}
