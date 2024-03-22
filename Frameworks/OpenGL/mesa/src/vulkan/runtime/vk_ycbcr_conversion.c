/*
 * Copyright © 2020 Intel Corporation
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

#include "vk_ycbcr_conversion.h"

#include <vulkan/vulkan_android.h>

#include "vk_common_entrypoints.h"
#include "vk_device.h"
#include "vk_format.h"
#include "vk_util.h"

VKAPI_ATTR VkResult VKAPI_CALL
vk_common_CreateSamplerYcbcrConversion(VkDevice _device,
                                       const VkSamplerYcbcrConversionCreateInfo *pCreateInfo,
                                       const VkAllocationCallbacks *pAllocator,
                                       VkSamplerYcbcrConversion *pYcbcrConversion)
{
   VK_FROM_HANDLE(vk_device, device, _device);
   struct vk_ycbcr_conversion *conversion;

   assert(pCreateInfo->sType == VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO);

   conversion = vk_object_zalloc(device, pAllocator, sizeof(*conversion),
                                 VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION);
   if (!conversion)
      return VK_ERROR_OUT_OF_HOST_MEMORY;

   struct vk_ycbcr_conversion_state *state = &conversion->state;

   state->format = pCreateInfo->format;
   state->ycbcr_model = pCreateInfo->ycbcrModel;
   state->ycbcr_range = pCreateInfo->ycbcrRange;

   /* Search for VkExternalFormatANDROID and resolve the format. */
   const VkExternalFormatANDROID *android_ext_info =
      vk_find_struct_const(pCreateInfo->pNext, EXTERNAL_FORMAT_ANDROID);

   /* We assume that Android externalFormat is just a VkFormat */
   if (android_ext_info && android_ext_info->externalFormat) {
      assert(pCreateInfo->format == VK_FORMAT_UNDEFINED);
      state->format = android_ext_info->externalFormat;
   } else {
      /* The Vulkan 1.1.95 spec says:
       *
       *    "When creating an external format conversion, the value of
       *    components if ignored."
       */
      state->mapping[0] = pCreateInfo->components.r;
      state->mapping[1] = pCreateInfo->components.g;
      state->mapping[2] = pCreateInfo->components.b;
      state->mapping[3] = pCreateInfo->components.a;
   }

   state->chroma_offsets[0] = pCreateInfo->xChromaOffset;
   state->chroma_offsets[1] = pCreateInfo->yChromaOffset;
   state->chroma_filter = pCreateInfo->chromaFilter;

   const struct vk_format_ycbcr_info *ycbcr_info =
      vk_format_get_ycbcr_info(state->format);

   bool has_chroma_subsampled = false;
   if (ycbcr_info) {
      for (uint32_t p = 0; p < ycbcr_info->n_planes; p++) {
         if (ycbcr_info->planes[p].has_chroma &&
             (ycbcr_info->planes[p].denominator_scales[0] > 1 ||
              ycbcr_info->planes[p].denominator_scales[1] > 1))
            has_chroma_subsampled = true;
      }
   }
   state->chroma_reconstruction = has_chroma_subsampled &&
      (state->chroma_offsets[0] == VK_CHROMA_LOCATION_COSITED_EVEN ||
       state->chroma_offsets[1] == VK_CHROMA_LOCATION_COSITED_EVEN);

   *pYcbcrConversion = vk_ycbcr_conversion_to_handle(conversion);

   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL
vk_common_DestroySamplerYcbcrConversion(VkDevice _device,
                                        VkSamplerYcbcrConversion YcbcrConversion,
                                        const VkAllocationCallbacks *pAllocator)
{
   VK_FROM_HANDLE(vk_device, device, _device);
   VK_FROM_HANDLE(vk_ycbcr_conversion, conversion, YcbcrConversion);

   if (!conversion)
      return;

   vk_object_free(device, pAllocator, conversion);
}
