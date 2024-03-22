/*
 * Copyright 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_HWVULKAN_H
#define ANDROID_HWVULKAN_H

#include <hardware/hardware.h>
#include <vulkan/vulkan.h>

__BEGIN_DECLS

#define HWVULKAN_HARDWARE_MODULE_ID "vulkan"

#define HWVULKAN_MODULE_API_VERSION_0_1 HARDWARE_MODULE_API_VERSION(0, 1)
#define HWVULKAN_DEVICE_API_VERSION_0_1 HARDWARE_DEVICE_API_VERSION_2(0, 1, 0)

#define HWVULKAN_DEVICE_0 "vk0"

typedef struct hwvulkan_module_t {
    struct hw_module_t common;
} hwvulkan_module_t;

/* Dispatchable Vulkan object handles must be pointers, which must point to
 * instances of hwvulkan_dispatch_t (potentially followed by additional
 * implementation-defined data). On return from the creation function, the
 * 'magic' field must contain HWVULKAN_DISPATCH_MAGIC; the loader will overwrite
 * the 'vtbl' field.
 *
 * NOTE: The magic value and the layout of hwvulkan_dispatch_t match the LunarG
 * loader used on platforms, to avoid pointless annoying differences for
 * multi-platform drivers. Don't change them without a good reason. If there is
 * an opportunity to change it, using a magic value that doesn't leave the
 * upper 32-bits zero on 64-bit platforms would be nice.
 */
#define HWVULKAN_DISPATCH_MAGIC 0x01CDC0DE
typedef union {
    uintptr_t magic;
    const void* vtbl;
} hwvulkan_dispatch_t;

/* A hwvulkan_device_t corresponds to an ICD on other systems. Currently there
 * can only be one on a system (HWVULKAN_DEVICE_0). It is opened once per
 * process when the Vulkan API is first used; the hw_device_t::close() function
 * is never called. Any non-trivial resource allocation should be done when
 * the VkInstance is created rather than when the hwvulkan_device_t is opened.
 */
typedef struct hwvulkan_device_t {
    struct hw_device_t common;

    PFN_vkEnumerateInstanceExtensionProperties
        EnumerateInstanceExtensionProperties;
    PFN_vkCreateInstance CreateInstance;
    PFN_vkGetInstanceProcAddr GetInstanceProcAddr;
} hwvulkan_device_t;

__END_DECLS

#endif  // ANDROID_HWVULKAN_H
