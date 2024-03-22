/*
 * Copyright © 2021 Intel Corporation
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

#ifndef VK_DEBUG_UTILS_H
#define VK_DEBUG_UTILS_H

#include "vk_instance.h"

#ifdef __cplusplus
extern "C" {
#endif

struct vk_debug_utils_messenger {
   struct vk_object_base base;
   VkAllocationCallbacks alloc;

   struct list_head link;

   VkDebugUtilsMessageSeverityFlagsEXT severity;
   VkDebugUtilsMessageTypeFlagsEXT type;
   PFN_vkDebugUtilsMessengerCallbackEXT callback;
   void *data;
};

VK_DEFINE_NONDISP_HANDLE_CASTS(vk_debug_utils_messenger, base,
                               VkDebugUtilsMessengerEXT,
                               VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT)

void
vk_debug_message(struct vk_instance *instance,
                 VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                 VkDebugUtilsMessageTypeFlagsEXT types,
                 const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData);

void
vk_debug_message_instance(struct vk_instance *instance,
                          VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                          VkDebugUtilsMessageTypeFlagsEXT types,
                          const char *pMessageIdName,
                          int32_t messageIdNumber,
                          const char *pMessage);

#ifdef __cplusplus
}
#endif

#endif /* VK_DEBUG_UTILS_H */
