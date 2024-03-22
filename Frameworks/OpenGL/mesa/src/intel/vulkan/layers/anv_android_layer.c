/*
 * Copyright © 2023 Intel Corporation
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

VKAPI_ATTR VkResult VKAPI_CALL
android_CreateImageView(VkDevice _device,
                        const VkImageViewCreateInfo *pCreateInfo,
                        const VkAllocationCallbacks *pAllocator,
                        VkImageView *pView)
{
   ANV_FROM_HANDLE(anv_device, device, _device);
   const struct util_format_description *fmt =
      vk_format_description(pCreateInfo->format);

   /* Throw error in case application tries to create ASTC view on gfx125.
    * This is done to avoid gpu hang that can result in using the unsupported
    * format.
    */
   if (fmt && fmt->layout == UTIL_FORMAT_LAYOUT_ASTC &&
       device->info->verx10 >= 125) {
      return vk_errorf(device, VK_ERROR_OUT_OF_HOST_MEMORY,
                       "ASTC format not supported (%s).", __func__);
   }
   return anv_CreateImageView(_device, pCreateInfo, pAllocator, pView);
}
