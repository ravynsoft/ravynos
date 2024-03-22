/*
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 *
 * based in part on anv driver which is:
 * Copyright © 2015 Intel Corporation
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

#include "radv_private.h"

#include "vk_sampler.h"

static unsigned
radv_tex_wrap(VkSamplerAddressMode address_mode)
{
   switch (address_mode) {
   case VK_SAMPLER_ADDRESS_MODE_REPEAT:
      return V_008F30_SQ_TEX_WRAP;
   case VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT:
      return V_008F30_SQ_TEX_MIRROR;
   case VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE:
      return V_008F30_SQ_TEX_CLAMP_LAST_TEXEL;
   case VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER:
      return V_008F30_SQ_TEX_CLAMP_BORDER;
   case VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE:
      return V_008F30_SQ_TEX_MIRROR_ONCE_LAST_TEXEL;
   default:
      unreachable("illegal tex wrap mode");
      break;
   }
   return 0;
}

static unsigned
radv_tex_compare(VkCompareOp op)
{
   switch (op) {
   case VK_COMPARE_OP_NEVER:
      return V_008F30_SQ_TEX_DEPTH_COMPARE_NEVER;
   case VK_COMPARE_OP_LESS:
      return V_008F30_SQ_TEX_DEPTH_COMPARE_LESS;
   case VK_COMPARE_OP_EQUAL:
      return V_008F30_SQ_TEX_DEPTH_COMPARE_EQUAL;
   case VK_COMPARE_OP_LESS_OR_EQUAL:
      return V_008F30_SQ_TEX_DEPTH_COMPARE_LESSEQUAL;
   case VK_COMPARE_OP_GREATER:
      return V_008F30_SQ_TEX_DEPTH_COMPARE_GREATER;
   case VK_COMPARE_OP_NOT_EQUAL:
      return V_008F30_SQ_TEX_DEPTH_COMPARE_NOTEQUAL;
   case VK_COMPARE_OP_GREATER_OR_EQUAL:
      return V_008F30_SQ_TEX_DEPTH_COMPARE_GREATEREQUAL;
   case VK_COMPARE_OP_ALWAYS:
      return V_008F30_SQ_TEX_DEPTH_COMPARE_ALWAYS;
   default:
      unreachable("illegal compare mode");
      break;
   }
   return 0;
}

static unsigned
radv_tex_filter(VkFilter filter, unsigned max_ansio)
{
   switch (filter) {
   case VK_FILTER_NEAREST:
      return (max_ansio > 1 ? V_008F38_SQ_TEX_XY_FILTER_ANISO_POINT : V_008F38_SQ_TEX_XY_FILTER_POINT);
   case VK_FILTER_LINEAR:
      return (max_ansio > 1 ? V_008F38_SQ_TEX_XY_FILTER_ANISO_BILINEAR : V_008F38_SQ_TEX_XY_FILTER_BILINEAR);
   case VK_FILTER_CUBIC_EXT:
   default:
      fprintf(stderr, "illegal texture filter");
      return 0;
   }
}

static unsigned
radv_tex_mipfilter(VkSamplerMipmapMode mode)
{
   switch (mode) {
   case VK_SAMPLER_MIPMAP_MODE_NEAREST:
      return V_008F38_SQ_TEX_Z_FILTER_POINT;
   case VK_SAMPLER_MIPMAP_MODE_LINEAR:
      return V_008F38_SQ_TEX_Z_FILTER_LINEAR;
   default:
      return V_008F38_SQ_TEX_Z_FILTER_NONE;
   }
}

static unsigned
radv_tex_bordercolor(VkBorderColor bcolor)
{
   switch (bcolor) {
   case VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK:
   case VK_BORDER_COLOR_INT_TRANSPARENT_BLACK:
      return V_008F3C_SQ_TEX_BORDER_COLOR_TRANS_BLACK;
   case VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK:
   case VK_BORDER_COLOR_INT_OPAQUE_BLACK:
      return V_008F3C_SQ_TEX_BORDER_COLOR_OPAQUE_BLACK;
   case VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE:
   case VK_BORDER_COLOR_INT_OPAQUE_WHITE:
      return V_008F3C_SQ_TEX_BORDER_COLOR_OPAQUE_WHITE;
   case VK_BORDER_COLOR_FLOAT_CUSTOM_EXT:
   case VK_BORDER_COLOR_INT_CUSTOM_EXT:
      return V_008F3C_SQ_TEX_BORDER_COLOR_REGISTER;
   default:
      break;
   }
   return 0;
}

static unsigned
radv_tex_aniso_filter(unsigned filter)
{
   return MIN2(util_logbase2(filter), 4);
}

static unsigned
radv_tex_filter_mode(VkSamplerReductionMode mode)
{
   switch (mode) {
   case VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE:
      return V_008F30_SQ_IMG_FILTER_MODE_BLEND;
   case VK_SAMPLER_REDUCTION_MODE_MIN:
      return V_008F30_SQ_IMG_FILTER_MODE_MIN;
   case VK_SAMPLER_REDUCTION_MODE_MAX:
      return V_008F30_SQ_IMG_FILTER_MODE_MAX;
   default:
      break;
   }
   return 0;
}

static uint32_t
radv_get_max_anisotropy(struct radv_device *device, const VkSamplerCreateInfo *pCreateInfo)
{
   if (device->force_aniso >= 0)
      return device->force_aniso;

   if (pCreateInfo->anisotropyEnable && pCreateInfo->maxAnisotropy > 1.0f)
      return (uint32_t)pCreateInfo->maxAnisotropy;

   return 0;
}

static uint32_t
radv_register_border_color(struct radv_device *device, VkClearColorValue value)
{
   uint32_t slot;

   mtx_lock(&device->border_color_data.mutex);

   for (slot = 0; slot < RADV_BORDER_COLOR_COUNT; slot++) {
      if (!device->border_color_data.used[slot]) {
         /* Copy to the GPU wrt endian-ness. */
         util_memcpy_cpu_to_le32(&device->border_color_data.colors_gpu_ptr[slot], &value, sizeof(VkClearColorValue));

         device->border_color_data.used[slot] = true;
         break;
      }
   }

   mtx_unlock(&device->border_color_data.mutex);

   return slot;
}

static void
radv_unregister_border_color(struct radv_device *device, uint32_t slot)
{
   mtx_lock(&device->border_color_data.mutex);

   device->border_color_data.used[slot] = false;

   mtx_unlock(&device->border_color_data.mutex);
}

static void
radv_init_sampler(struct radv_device *device, struct radv_sampler *sampler, const VkSamplerCreateInfo *pCreateInfo)
{
   uint32_t max_aniso = radv_get_max_anisotropy(device, pCreateInfo);
   uint32_t max_aniso_ratio = radv_tex_aniso_filter(max_aniso);
   bool compat_mode =
      device->physical_device->rad_info.gfx_level == GFX8 || device->physical_device->rad_info.gfx_level == GFX9;
   unsigned filter_mode = radv_tex_filter_mode(sampler->vk.reduction_mode);
   unsigned depth_compare_func = V_008F30_SQ_TEX_DEPTH_COMPARE_NEVER;
   bool trunc_coord = ((pCreateInfo->minFilter == VK_FILTER_NEAREST && pCreateInfo->magFilter == VK_FILTER_NEAREST) ||
                       device->physical_device->rad_info.conformant_trunc_coord) &&
                      !device->disable_trunc_coord;
   bool uses_border_color = pCreateInfo->addressModeU == VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER ||
                            pCreateInfo->addressModeV == VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER ||
                            pCreateInfo->addressModeW == VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
   VkBorderColor border_color = uses_border_color ? pCreateInfo->borderColor : VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
   uint32_t border_color_ptr;
   bool disable_cube_wrap = pCreateInfo->flags & VK_SAMPLER_CREATE_NON_SEAMLESS_CUBE_MAP_BIT_EXT;

   if (pCreateInfo->compareEnable)
      depth_compare_func = radv_tex_compare(pCreateInfo->compareOp);

   sampler->border_color_slot = RADV_BORDER_COLOR_COUNT;

   if (vk_border_color_is_custom(border_color)) {
      sampler->border_color_slot = radv_register_border_color(device, sampler->vk.border_color_value);

      /* Did we fail to find a slot? */
      if (sampler->border_color_slot == RADV_BORDER_COLOR_COUNT) {
         fprintf(stderr, "WARNING: no free border color slots, defaulting to TRANS_BLACK.\n");
         border_color = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
      }
   }

   /* If we don't have a custom color, set the ptr to 0 */
   border_color_ptr = sampler->border_color_slot != RADV_BORDER_COLOR_COUNT ? sampler->border_color_slot : 0;

   sampler->state[0] = (S_008F30_CLAMP_X(radv_tex_wrap(pCreateInfo->addressModeU)) |
                        S_008F30_CLAMP_Y(radv_tex_wrap(pCreateInfo->addressModeV)) |
                        S_008F30_CLAMP_Z(radv_tex_wrap(pCreateInfo->addressModeW)) |
                        S_008F30_MAX_ANISO_RATIO(max_aniso_ratio) | S_008F30_DEPTH_COMPARE_FUNC(depth_compare_func) |
                        S_008F30_FORCE_UNNORMALIZED(pCreateInfo->unnormalizedCoordinates ? 1 : 0) |
                        S_008F30_ANISO_THRESHOLD(max_aniso_ratio >> 1) | S_008F30_ANISO_BIAS(max_aniso_ratio) |
                        S_008F30_DISABLE_CUBE_WRAP(disable_cube_wrap) | S_008F30_COMPAT_MODE(compat_mode) |
                        S_008F30_FILTER_MODE(filter_mode) | S_008F30_TRUNC_COORD(trunc_coord));
   sampler->state[1] = (S_008F34_MIN_LOD(radv_float_to_ufixed(CLAMP(pCreateInfo->minLod, 0, 15), 8)) |
                        S_008F34_MAX_LOD(radv_float_to_ufixed(CLAMP(pCreateInfo->maxLod, 0, 15), 8)) |
                        S_008F34_PERF_MIP(max_aniso_ratio ? max_aniso_ratio + 6 : 0));
   sampler->state[2] = (S_008F38_XY_MAG_FILTER(radv_tex_filter(pCreateInfo->magFilter, max_aniso)) |
                        S_008F38_XY_MIN_FILTER(radv_tex_filter(pCreateInfo->minFilter, max_aniso)) |
                        S_008F38_MIP_FILTER(radv_tex_mipfilter(pCreateInfo->mipmapMode)));
   sampler->state[3] = S_008F3C_BORDER_COLOR_TYPE(radv_tex_bordercolor(border_color));

   if (device->physical_device->rad_info.gfx_level >= GFX10) {
      sampler->state[2] |= S_008F38_LOD_BIAS(radv_float_to_sfixed(CLAMP(pCreateInfo->mipLodBias, -32, 31), 8)) |
                           S_008F38_ANISO_OVERRIDE_GFX10(device->instance->drirc.disable_aniso_single_level);
   } else {
      sampler->state[2] |= S_008F38_LOD_BIAS(radv_float_to_sfixed(CLAMP(pCreateInfo->mipLodBias, -16, 16), 8)) |
                           S_008F38_DISABLE_LSB_CEIL(device->physical_device->rad_info.gfx_level <= GFX8) |
                           S_008F38_FILTER_PREC_FIX(1) |
                           S_008F38_ANISO_OVERRIDE_GFX8(device->instance->drirc.disable_aniso_single_level &&
                                                        device->physical_device->rad_info.gfx_level >= GFX8);
   }

   if (device->physical_device->rad_info.gfx_level >= GFX11) {
      sampler->state[3] |= S_008F3C_BORDER_COLOR_PTR_GFX11(border_color_ptr);
   } else {
      sampler->state[3] |= S_008F3C_BORDER_COLOR_PTR_GFX6(border_color_ptr);
   }
}

VKAPI_ATTR VkResult VKAPI_CALL
radv_CreateSampler(VkDevice _device, const VkSamplerCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
                   VkSampler *pSampler)
{
   RADV_FROM_HANDLE(radv_device, device, _device);
   struct radv_sampler *sampler;

   sampler = vk_sampler_create(&device->vk, pCreateInfo, pAllocator, sizeof(*sampler));
   if (!sampler)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   radv_init_sampler(device, sampler, pCreateInfo);

   *pSampler = radv_sampler_to_handle(sampler);

   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL
radv_DestroySampler(VkDevice _device, VkSampler _sampler, const VkAllocationCallbacks *pAllocator)
{
   RADV_FROM_HANDLE(radv_device, device, _device);
   RADV_FROM_HANDLE(radv_sampler, sampler, _sampler);

   if (!sampler)
      return;

   if (sampler->border_color_slot != RADV_BORDER_COLOR_COUNT)
      radv_unregister_border_color(device, sampler->border_color_slot);

   vk_sampler_destroy(&device->vk, pAllocator, &sampler->vk);
}
