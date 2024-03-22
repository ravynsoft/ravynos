/*
 * Copyright © 2023 Collabora, Ltd.
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

#include "vk_device_memory.h"

#include "vk_android.h"
#include "vk_common_entrypoints.h"
#include "vk_util.h"

#if defined(ANDROID) && ANDROID_API_LEVEL >= 26
#include <vndk/hardware_buffer.h>
#endif

void *
vk_device_memory_create(struct vk_device *device,
                        const VkMemoryAllocateInfo *pAllocateInfo,
                        const VkAllocationCallbacks *alloc,
                        size_t size)
{
   struct vk_device_memory *mem =
      vk_object_zalloc(device, alloc, size, VK_OBJECT_TYPE_DEVICE_MEMORY);
   if (mem == NULL)
      return NULL;

   assert(pAllocateInfo->sType == VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);

   mem->size = pAllocateInfo->allocationSize;
   mem->memory_type_index = pAllocateInfo->memoryTypeIndex;

   vk_foreach_struct_const(ext, pAllocateInfo->pNext) {
      switch (ext->sType) {
      case VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO: {
         const VkExportMemoryAllocateInfo *export_info = (void *)ext;
         mem->export_handle_types = export_info->handleTypes;
         break;
      }

      case VK_STRUCTURE_TYPE_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID: {
#if defined(ANDROID) && ANDROID_API_LEVEL >= 26
         const VkImportAndroidHardwareBufferInfoANDROID *ahb_info = (void *)ext;

         assert(mem->import_handle_type == 0);
         mem->import_handle_type =
            VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;

         /* From the Vulkan 1.3.242 spec:
          *
          *    "If the vkAllocateMemory command succeeds, the implementation
          *    must acquire a reference to the imported hardware buffer, which
          *    it must release when the device memory object is freed. If the
          *    command fails, the implementation must not retain a
          *    reference."
          *
          * We assume that if the driver fails to create its memory object,
          * it will call vk_device_memory_destroy which will delete our
          * reference.
          */
         AHardwareBuffer_acquire(ahb_info->buffer);
         mem->ahardware_buffer = ahb_info->buffer;
         break;
#else
         unreachable("AHardwareBuffer import requires Android >= 26");
#endif /* ANDROID_API_LEVEL >= 26 */
      }

      case VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR: {
         const VkImportMemoryFdInfoKHR *fd_info = (void *)ext;
         if (fd_info->handleType) {
            assert(fd_info->handleType == VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT ||
                   fd_info->handleType == VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT);
            assert(mem->import_handle_type == 0);
            mem->import_handle_type = fd_info->handleType;
         }
         break;
      }

      case VK_STRUCTURE_TYPE_IMPORT_MEMORY_HOST_POINTER_INFO_EXT: {
         const VkImportMemoryHostPointerInfoEXT *host_ptr_info = (void *)ext;
         if (host_ptr_info->handleType) {
            assert(host_ptr_info->handleType == VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT ||
                   host_ptr_info->handleType == VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_MAPPED_FOREIGN_MEMORY_BIT_EXT);

            assert(mem->import_handle_type == 0);
            mem->import_handle_type = host_ptr_info->handleType;
            mem->host_ptr = host_ptr_info->pHostPointer;
         }
         break;
      }

      case VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR: {
#ifdef VK_USE_PLATFORM_WIN32_KHR
         const VkImportMemoryWin32HandleInfoKHR *w32h_info = (void *)ext;
         if (w32h_info->handleType) {
            assert(w32h_info->handleType == VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT ||
                   w32h_info->handleType == VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT ||
                   w32h_info->handleType == VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT ||
                   w32h_info->handleType == VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_KMT_BIT ||
                   w32h_info->handleType == VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_HEAP_BIT ||
                   w32h_info->handleType == VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_RESOURCE_BIT);
            assert(mem->import_handle_type == 0);
            mem->import_handle_type = w32h_info->handleType;
         }
         break;
#else
         unreachable("Win32 platform support disabled");
#endif
      }

      case VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO: {
         const VkMemoryAllocateFlagsInfo *flags_info = (void *)ext;
         mem->alloc_flags = flags_info->flags;
         break;
      }

      default:
         break;
      }
   }

   /* From the Vulkan Specification 1.3.261:
    *
    *    VUID-VkMemoryAllocateInfo-allocationSize-07897
    *
    *   "If the parameters do not define an import or export operation,
    *   allocationSize must be greater than 0."
    */
   if (!mem->import_handle_type && !mem->export_handle_types)
      assert(pAllocateInfo->allocationSize > 0);

   /* From the Vulkan Specification 1.3.261:
    *
    *    VUID-VkMemoryAllocateInfo-allocationSize-07899
    *
    *    "If the parameters define an export operation and the handle type is
    *    not VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID,
    *    allocationSize must be greater than 0."
    */
    if (mem->export_handle_types &&
        mem->export_handle_types !=
          VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID)
      assert(pAllocateInfo->allocationSize > 0);

   if ((mem->export_handle_types &
        VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID) &&
       mem->ahardware_buffer == NULL) {
      /* If we need to be able to export an Android hardware buffer but none
       * is provided as an import, create a new one.
       */
      mem->ahardware_buffer = vk_alloc_ahardware_buffer(pAllocateInfo);
      if (mem->ahardware_buffer == NULL) {
         vk_device_memory_destroy(device, alloc, mem);
         return NULL;
      }
   }

   return mem;
}

void
vk_device_memory_destroy(struct vk_device *device,
                         const VkAllocationCallbacks *alloc,
                         struct vk_device_memory *mem)
{
#if defined(ANDROID) && ANDROID_API_LEVEL >= 26
   if (mem->ahardware_buffer)
      AHardwareBuffer_release(mem->ahardware_buffer);
#endif /* ANDROID_API_LEVEL >= 26 */

   vk_object_free(device, alloc, mem);
}

#if defined(ANDROID) && ANDROID_API_LEVEL >= 26
VkResult
vk_common_GetMemoryAndroidHardwareBufferANDROID(
   VkDevice _device,
   const VkMemoryGetAndroidHardwareBufferInfoANDROID *pInfo,
   struct AHardwareBuffer **pBuffer)
{
   VK_FROM_HANDLE(vk_device_memory, mem, pInfo->memory);

   /* Some quotes from Vulkan spec:
    *
    *    "If the device memory was created by importing an Android hardware
    *    buffer, vkGetMemoryAndroidHardwareBufferANDROID must return that same
    *    Android hardware buffer object."
    *
    *    "VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID
    *    must have been included in VkExportMemoryAllocateInfo::handleTypes
    *    when memory was created."
    */
   if (mem->ahardware_buffer) {
      *pBuffer = mem->ahardware_buffer;
      /* Increase refcount. */
      AHardwareBuffer_acquire(*pBuffer);
      return VK_SUCCESS;
   }

   return VK_ERROR_INVALID_EXTERNAL_HANDLE;
}
#endif
