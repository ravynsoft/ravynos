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

#ifndef __VK_ANDROID_NATIVE_BUFFER_H__
#define __VK_ANDROID_NATIVE_BUFFER_H__

/* MESA: A hack to avoid #ifdefs in driver code. */
#ifdef ANDROID
#include <cutils/native_handle.h>
#include <vulkan/vulkan.h>

#if ANDROID_API_LEVEL < 28
/* buffer_handle_t was defined in the deprecated system/window.h */
typedef const native_handle_t *buffer_handle_t;
#endif

#else
typedef void *buffer_handle_t;
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define VK_ANDROID_native_buffer 1

#define VK_ANDROID_NATIVE_BUFFER_EXTENSION_NUMBER 11
/*
 * NOTE ON VK_ANDROID_NATIVE_BUFFER_SPEC_VERSION 6
 *
 * This version of the extension transitions from gralloc0 to gralloc1 usage
 * flags (int -> 2x uint64_t). The WSI implementation will temporarily continue
 * to fill out deprecated fields in VkNativeBufferANDROID, and will call the
 * deprecated vkGetSwapchainGrallocUsageANDROID if the new
 * vkGetSwapchainGrallocUsage2ANDROID is not supported. This transitionary
 * backwards-compatibility support is temporary, and will likely be removed
 * (along with all gralloc0 support) in a future release.
 */
/*
 * NOTE ON VK_ANDROID_NATIVE_BUFFER_SPEC_VERSION 8
 *
 * This version of the extension doesn't introduce new types or structs, but is
 * to accommodate the new struct VkBindImageMemorySwapchainInfoKHR added in
 * VK_KHR_swapchain spec version 69. When VkBindImageMemorySwapchainInfoKHR is
 * chained in the pNext chain of VkBindImageMemoryInfo, a VkNativeBufferANDROID
 * that holds the correct gralloc handle according to the imageIndex specified
 * in VkBindImageMemorySwapchainInfoKHR will be additionally chained to the
 * pNext chain of VkBindImageMemoryInfo and passed down to the driver.
 */
/*
 * NOTE ON VK_ANDROID_NATIVE_BUFFER_SPEC_VERSION 9
 *
 * This version of the extension is largely designed to clean up the mix of
 * GrallocUsage and GrallocUsage2
 */
#define VK_ANDROID_NATIVE_BUFFER_SPEC_VERSION 9
#define VK_ANDROID_NATIVE_BUFFER_EXTENSION_NAME "VK_ANDROID_native_buffer"

#define VK_ANDROID_NATIVE_BUFFER_ENUM(type, id) \
    ((type)(1000000000 +                        \
            (1000 * (VK_ANDROID_NATIVE_BUFFER_EXTENSION_NUMBER - 1)) + (id)))
#define VK_STRUCTURE_TYPE_NATIVE_BUFFER_ANDROID \
    VK_ANDROID_NATIVE_BUFFER_ENUM(VkStructureType, 0)
#define VK_STRUCTURE_TYPE_SWAPCHAIN_IMAGE_CREATE_INFO_ANDROID \
    VK_ANDROID_NATIVE_BUFFER_ENUM(VkStructureType, 1)
#define VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENTATION_PROPERTIES_ANDROID \
    VK_ANDROID_NATIVE_BUFFER_ENUM(VkStructureType, 2)
#define VK_STRUCTURE_TYPE_GRALLOC_USAGE_INFO_ANDROID \
    VK_ANDROID_NATIVE_BUFFER_ENUM(VkStructureType, 3)

/* clang-format off */
typedef enum VkSwapchainImageUsageFlagBitsANDROID {
    VK_SWAPCHAIN_IMAGE_USAGE_SHARED_BIT_ANDROID = 0x00000001,
    VK_SWAPCHAIN_IMAGE_USAGE_FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
} VkSwapchainImageUsageFlagBitsANDROID;
typedef VkFlags VkSwapchainImageUsageFlagsANDROID;

/*
 * struct VkNativeBufferUsage2ANDROID
 *
 * consumer: gralloc1 consumer usage flag
 * producer: gralloc1 producer usage flag
 */
typedef struct {
    uint64_t                          consumer;
    uint64_t                          producer;
} VkNativeBufferUsage2ANDROID;

/*
 * struct VkNativeBufferANDROID
 *
 * sType: VK_STRUCTURE_TYPE_NATIVE_BUFFER_ANDROID
 * pNext: NULL or a pointer to a structure extending this structure
 * handle: buffer handle returned from gralloc alloc()
 * stride: stride returned from gralloc alloc()
 * format: gralloc format requested when the buffer was allocated
 * usage: gralloc usage requested when the buffer was allocated
 * usage2: gralloc usage requested when the buffer was allocated
 * usage3: gralloc usage requested when the buffer was allocated
 */
typedef struct {
    VkStructureType                   sType;
    const void*                       pNext;
    buffer_handle_t                   handle;
    int                               stride;
    int                               format;
    int                               usage; /* DEPRECATED in SPEC_VERSION 6 */
    VkNativeBufferUsage2ANDROID       usage2; /* DEPRECATED in SPEC_VERSION 9 */
    uint64_t                          usage3; /* ADDED in SPEC_VERSION 9 */
} VkNativeBufferANDROID;

/*
 * struct VkSwapchainImageCreateInfoANDROID
 *
 * sType: VK_STRUCTURE_TYPE_SWAPCHAIN_IMAGE_CREATE_INFO_ANDROID
 * pNext: NULL or a pointer to a structure extending this structure
 * usage: is a bitmask of VkSwapchainImageUsageFlagsANDROID
 */
typedef struct {
    VkStructureType                   sType;
    const void*                       pNext;
    VkSwapchainImageUsageFlagsANDROID usage;
} VkSwapchainImageCreateInfoANDROID;

/*
 * struct VkPhysicalDevicePresentationPropertiesANDROID
 *
 * sType: VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENTATION_PROPERTIES_ANDROID
 * pNext: NULL or a pointer to a structure extending this structure
 * sharedImage: specifies if the image can be shared with the display system
 */
typedef struct {
    VkStructureType                   sType;
    const void*                       pNext;
    VkBool32                          sharedImage;
} VkPhysicalDevicePresentationPropertiesANDROID;

/*
 * struct VkGrallocUsageInfoANDROID
 *
 * sType: VK_STRUCTURE_TYPE_GRALLOC_USAGE_INFO_ANDROID
 * pNext: NULL or a pointer to a structure extending this structure
 * format: value specifying the format the image will be created with
 * imageUsage: bitmask of VkImageUsageFlagBits describing intended usage
 */
typedef struct {
    VkStructureType                   sType;
    const void*                       pNext;
    VkFormat                          format;
    VkImageUsageFlags                 imageUsage;
} VkGrallocUsageInfoANDROID;

/* DEPRECATED in SPEC_VERSION 6 */
typedef VkResult (VKAPI_PTR *PFN_vkGetSwapchainGrallocUsageANDROID)(
    VkDevice                          device,
    VkFormat                          format,
    VkImageUsageFlags                 imageUsage,
    int*                              grallocUsage);

/* DEPRECATED in SPEC_VERSION 9 */
typedef VkResult (VKAPI_PTR *PFN_vkGetSwapchainGrallocUsage2ANDROID)(
    VkDevice                          device,
    VkFormat                          format,
    VkImageUsageFlags                 imageUsage,
    VkSwapchainImageUsageFlagsANDROID swapchainImageUsage,
    uint64_t*                         grallocConsumerUsage,
    uint64_t*                         grallocProducerUsage);

/* ADDED in SPEC_VERSION 9 */
typedef VkResult (VKAPI_PTR *PFN_vkGetSwapchainGrallocUsage3ANDROID)(
    VkDevice                          device,
    const VkGrallocUsageInfoANDROID*  grallocUsageInfo,
    uint64_t*                         grallocUsage);

typedef VkResult (VKAPI_PTR *PFN_vkAcquireImageANDROID)(
    VkDevice                          device,
    VkImage                           image,
    int                               nativeFenceFd,
    VkSemaphore                       semaphore,
    VkFence                           fence);

typedef VkResult (VKAPI_PTR *PFN_vkQueueSignalReleaseImageANDROID)(
    VkQueue                           queue,
    uint32_t                          waitSemaphoreCount,
    const VkSemaphore*                pWaitSemaphores,
    VkImage                           image,
    int*                              pNativeFenceFd);

#ifndef VK_NO_PROTOTYPES

/* DEPRECATED in SPEC_VERSION 6 */
VKAPI_ATTR VkResult VKAPI_CALL vkGetSwapchainGrallocUsageANDROID(
    VkDevice                          device,
    VkFormat                          format,
    VkImageUsageFlags                 imageUsage,
    int*                              grallocUsage
);

/* DEPRECATED in SPEC_VERSION 9 */
VKAPI_ATTR VkResult VKAPI_CALL vkGetSwapchainGrallocUsage2ANDROID(
    VkDevice                          device,
    VkFormat                          format,
    VkImageUsageFlags                 imageUsage,
    VkSwapchainImageUsageFlagsANDROID swapchainImageUsage,
    uint64_t*                         grallocConsumerUsage,
    uint64_t*                         grallocProducerUsage
);

/* ADDED in SPEC_VERSION 9 */
VKAPI_ATTR VkResult VKAPI_CALL vkGetSwapchainGrallocUsage3ANDROID(
    VkDevice                          device,
    const VkGrallocUsageInfoANDROID*  grallocUsageInfo,
    uint64_t*                         grallocUsage
);

VKAPI_ATTR VkResult VKAPI_CALL vkAcquireImageANDROID(
    VkDevice                          device,
    VkImage                           image,
    int                               nativeFenceFd,
    VkSemaphore                       semaphore,
    VkFence                           fence
);

VKAPI_ATTR VkResult VKAPI_CALL vkQueueSignalReleaseImageANDROID(
    VkQueue                           queue,
    uint32_t                          waitSemaphoreCount,
    const VkSemaphore*                pWaitSemaphores,
    VkImage                           image,
    int*                              pNativeFenceFd
);

#endif
/* clang-format on */

#ifdef __cplusplus
}
#endif

#endif /* __VK_ANDROID_NATIVE_BUFFER_H__ */
