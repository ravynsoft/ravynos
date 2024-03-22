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

#include "vk_semaphore.h"

#include "util/os_time.h"
#include "util/perf/cpu_trace.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "vk_common_entrypoints.h"
#include "vk_device.h"
#include "vk_log.h"
#include "vk_physical_device.h"
#include "vk_util.h"

static VkExternalSemaphoreHandleTypeFlags
vk_sync_semaphore_import_types(const struct vk_sync_type *type,
                               VkSemaphoreType semaphore_type)
{
   VkExternalSemaphoreHandleTypeFlags handle_types = 0;

   if (type->import_opaque_fd)
      handle_types |= VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT;

   if (type->export_sync_file && semaphore_type == VK_SEMAPHORE_TYPE_BINARY)
      handle_types |= VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT;

   if (type->import_win32_handle) {
      handle_types |= VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT;
      if (type->features & VK_SYNC_FEATURE_TIMELINE)
         handle_types |= VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_D3D12_FENCE_BIT;
   }

   return handle_types;
}

static VkExternalSemaphoreHandleTypeFlags
vk_sync_semaphore_export_types(const struct vk_sync_type *type,
                               VkSemaphoreType semaphore_type)
{
   VkExternalSemaphoreHandleTypeFlags handle_types = 0;

   if (type->export_opaque_fd)
      handle_types |= VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT;

   if (type->export_sync_file && semaphore_type == VK_SEMAPHORE_TYPE_BINARY)
      handle_types |= VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT;

   if (type->export_win32_handle) {
      handle_types |= VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT;
      if (type->features & VK_SYNC_FEATURE_TIMELINE)
         handle_types |= VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_D3D12_FENCE_BIT;
   }

   return handle_types;
}

static VkExternalSemaphoreHandleTypeFlags
vk_sync_semaphore_handle_types(const struct vk_sync_type *type,
                               VkSemaphoreType semaphore_type)
{
   return vk_sync_semaphore_export_types(type, semaphore_type) &
          vk_sync_semaphore_import_types(type, semaphore_type);
}

static const struct vk_sync_type *
get_semaphore_sync_type(struct vk_physical_device *pdevice,
                        VkSemaphoreType semaphore_type,
                        VkExternalSemaphoreHandleTypeFlags handle_types)
{
   assert(semaphore_type == VK_SEMAPHORE_TYPE_BINARY ||
          semaphore_type == VK_SEMAPHORE_TYPE_TIMELINE);

   enum vk_sync_features req_features = VK_SYNC_FEATURE_GPU_WAIT;
   if (semaphore_type == VK_SEMAPHORE_TYPE_TIMELINE) {
      req_features |= VK_SYNC_FEATURE_TIMELINE |
                      VK_SYNC_FEATURE_CPU_WAIT;
   } else {
      req_features |= VK_SYNC_FEATURE_BINARY;
   }

   for (const struct vk_sync_type *const *t =
        pdevice->supported_sync_types; *t; t++) {
      if (req_features & ~(*t)->features)
         continue;

      if (handle_types & ~vk_sync_semaphore_handle_types(*t, semaphore_type))
         continue;

      return *t;
   }

   return NULL;
}

static VkSemaphoreType
get_semaphore_type(const void *pNext, uint64_t *initial_value)
{
   const VkSemaphoreTypeCreateInfo *type_info =
      vk_find_struct_const(pNext, SEMAPHORE_TYPE_CREATE_INFO);

   if (!type_info)
      return VK_SEMAPHORE_TYPE_BINARY;

   if (initial_value)
      *initial_value = type_info->initialValue;
   return type_info->semaphoreType;
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_common_CreateSemaphore(VkDevice _device,
                          const VkSemaphoreCreateInfo *pCreateInfo,
                          const VkAllocationCallbacks *pAllocator,
                          VkSemaphore *pSemaphore)
{
   VK_FROM_HANDLE(vk_device, device, _device);
   struct vk_semaphore *semaphore;

   assert(pCreateInfo->sType == VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO);

   uint64_t initial_value = 0;
   const VkSemaphoreType semaphore_type =
      get_semaphore_type(pCreateInfo->pNext, &initial_value);

   if (semaphore_type == VK_SEMAPHORE_TYPE_TIMELINE)
      assert(device->timeline_mode != VK_DEVICE_TIMELINE_MODE_NONE);

   const VkExportSemaphoreCreateInfo *export =
      vk_find_struct_const(pCreateInfo->pNext, EXPORT_SEMAPHORE_CREATE_INFO);
   VkExternalSemaphoreHandleTypeFlags handle_types =
      export ? export->handleTypes : 0;

   const struct vk_sync_type *sync_type =
      get_semaphore_sync_type(device->physical, semaphore_type, handle_types);
   if (sync_type == NULL) {
      /* We should always be able to get a semaphore type for internal */
      assert(get_semaphore_sync_type(device->physical, semaphore_type, 0) != NULL);
      return vk_errorf(device, VK_ERROR_INVALID_EXTERNAL_HANDLE,
                       "Combination of external handle types is unsupported "
                       "for VkSemaphore creation.");
   }

   /* If the timeline mode is ASSISTED, then any permanent binary semaphore
    * types need to be able to support move.  We don't require this for
    * temporary unless that temporary is also used as a semaphore signal
    * operation which is much trickier to assert early.
    */
   if (semaphore_type == VK_SEMAPHORE_TYPE_BINARY &&
       vk_device_supports_threaded_submit(device))
      assert(sync_type->move);

   /* Allocate a vk_semaphore + vk_sync implementation. Because the permanent
    * field of vk_semaphore is the base field of the vk_sync implementation,
    * we can make the 2 structures overlap.
    */
   size_t size = offsetof(struct vk_semaphore, permanent) + sync_type->size;
   semaphore = vk_object_zalloc(device, pAllocator, size,
                                VK_OBJECT_TYPE_SEMAPHORE);
   if (semaphore == NULL)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   semaphore->type = semaphore_type;

   enum vk_sync_flags sync_flags = 0;
   if (semaphore_type == VK_SEMAPHORE_TYPE_TIMELINE)
      sync_flags |= VK_SYNC_IS_TIMELINE;
   if (handle_types)
      sync_flags |= VK_SYNC_IS_SHAREABLE;

   VkResult result = vk_sync_init(device, &semaphore->permanent,
                                  sync_type, sync_flags, initial_value);
   if (result != VK_SUCCESS) {
      vk_object_free(device, pAllocator, semaphore);
      return result;
   }

#ifdef _WIN32
   const VkExportSemaphoreWin32HandleInfoKHR *export_win32 =
      vk_find_struct_const(pCreateInfo->pNext, EXPORT_SEMAPHORE_WIN32_HANDLE_INFO_KHR);
   if (export_win32) {
      result = vk_sync_set_win32_export_params(device, &semaphore->permanent, export_win32->pAttributes,
                                               export_win32->dwAccess, export_win32->name);
      if (result != VK_SUCCESS) {
         vk_sync_finish(device, &semaphore->permanent);
         vk_object_free(device, pAllocator, semaphore);
         return result;
      }
   }
#endif

   *pSemaphore = vk_semaphore_to_handle(semaphore);

   return VK_SUCCESS;
}

void
vk_semaphore_reset_temporary(struct vk_device *device,
                             struct vk_semaphore *semaphore)
{
   if (semaphore->temporary == NULL)
      return;

   vk_sync_destroy(device, semaphore->temporary);
   semaphore->temporary = NULL;
}

VKAPI_ATTR void VKAPI_CALL
vk_common_DestroySemaphore(VkDevice _device,
                           VkSemaphore _semaphore,
                           const VkAllocationCallbacks *pAllocator)
{
   VK_FROM_HANDLE(vk_device, device, _device);
   VK_FROM_HANDLE(vk_semaphore, semaphore, _semaphore);

   if (semaphore == NULL)
      return;

   vk_semaphore_reset_temporary(device, semaphore);
   vk_sync_finish(device, &semaphore->permanent);

   vk_object_free(device, pAllocator, semaphore);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_GetPhysicalDeviceExternalSemaphoreProperties(
   VkPhysicalDevice physicalDevice,
   const VkPhysicalDeviceExternalSemaphoreInfo *pExternalSemaphoreInfo,
   VkExternalSemaphoreProperties *pExternalSemaphoreProperties)
{
   VK_FROM_HANDLE(vk_physical_device, pdevice, physicalDevice);

   assert(pExternalSemaphoreInfo->sType ==
          VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_SEMAPHORE_INFO);
   const VkExternalSemaphoreHandleTypeFlagBits handle_type =
      pExternalSemaphoreInfo->handleType;

   const VkSemaphoreType semaphore_type =
      get_semaphore_type(pExternalSemaphoreInfo->pNext, NULL);

   const struct vk_sync_type *sync_type =
      get_semaphore_sync_type(pdevice, semaphore_type, handle_type);
   if (sync_type == NULL) {
      pExternalSemaphoreProperties->exportFromImportedHandleTypes = 0;
      pExternalSemaphoreProperties->compatibleHandleTypes = 0;
      pExternalSemaphoreProperties->externalSemaphoreFeatures = 0;
      return;
   }

   VkExternalSemaphoreHandleTypeFlagBits import =
      vk_sync_semaphore_import_types(sync_type, semaphore_type);
   VkExternalSemaphoreHandleTypeFlagBits export =
      vk_sync_semaphore_export_types(sync_type, semaphore_type);

   VkExternalSemaphoreHandleTypeFlagBits opaque_types[] = {
      VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT,
      VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT,
   };
   for (uint32_t i = 0; i < ARRAY_SIZE(opaque_types); ++i) {
      if (handle_type != opaque_types[i]) {
         const struct vk_sync_type *opaque_sync_type =
            get_semaphore_sync_type(pdevice, semaphore_type, opaque_types[i]);

         /* If we're a different vk_sync_type than the one selected when only
          * an opaque type is set, then we can't import/export that opaque type. Put
          * differently, there can only be one OPAQUE_FD/WIN32_HANDLE sync type.
          */
         if (sync_type != opaque_sync_type) {
            import &= ~opaque_types[i];
            export &= ~opaque_types[i];
         }
      }
   }

   VkExternalSemaphoreHandleTypeFlags compatible = import & export;
   VkExternalSemaphoreFeatureFlags features = 0;
   if (handle_type & export)
      features |= VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT;
   if (handle_type & import)
      features |= VK_EXTERNAL_SEMAPHORE_FEATURE_IMPORTABLE_BIT;

   pExternalSemaphoreProperties->exportFromImportedHandleTypes = export;
   pExternalSemaphoreProperties->compatibleHandleTypes = compatible;
   pExternalSemaphoreProperties->externalSemaphoreFeatures = features;
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_common_GetSemaphoreCounterValue(VkDevice _device,
                                   VkSemaphore _semaphore,
                                   uint64_t *pValue)
{
   VK_FROM_HANDLE(vk_device, device, _device);
   VK_FROM_HANDLE(vk_semaphore, semaphore, _semaphore);

   if (vk_device_is_lost(device))
      return VK_ERROR_DEVICE_LOST;

   struct vk_sync *sync = vk_semaphore_get_active_sync(semaphore);
   return vk_sync_get_value(device, sync, pValue);
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_common_WaitSemaphores(VkDevice _device,
                         const VkSemaphoreWaitInfo *pWaitInfo,
                         uint64_t timeout)
{
   MESA_TRACE_FUNC();

   VK_FROM_HANDLE(vk_device, device, _device);

   if (vk_device_is_lost(device))
      return VK_ERROR_DEVICE_LOST;

   if (pWaitInfo->semaphoreCount == 0)
      return VK_SUCCESS;

   uint64_t abs_timeout_ns = os_time_get_absolute_timeout(timeout);

   const uint32_t wait_count = pWaitInfo->semaphoreCount;
   STACK_ARRAY(struct vk_sync_wait, waits, pWaitInfo->semaphoreCount);

   for (uint32_t i = 0; i < wait_count; i++) {
      VK_FROM_HANDLE(vk_semaphore, semaphore, pWaitInfo->pSemaphores[i]);
      assert(semaphore->type == VK_SEMAPHORE_TYPE_TIMELINE);

      waits[i] = (struct vk_sync_wait) {
         .sync = vk_semaphore_get_active_sync(semaphore),
         .stage_mask = ~(VkPipelineStageFlags2)0,
         .wait_value = pWaitInfo->pValues[i],
      };
   }

   enum vk_sync_wait_flags wait_flags = VK_SYNC_WAIT_COMPLETE;
   if (pWaitInfo->flags & VK_SEMAPHORE_WAIT_ANY_BIT)
      wait_flags |= VK_SYNC_WAIT_ANY;

   VkResult result = vk_sync_wait_many(device, wait_count, waits,
                                       wait_flags, abs_timeout_ns);

   STACK_ARRAY_FINISH(waits);

   VkResult device_status = vk_device_check_status(device);
   if (device_status != VK_SUCCESS)
      return device_status;

   return result;
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_common_SignalSemaphore(VkDevice _device,
                          const VkSemaphoreSignalInfo *pSignalInfo)
{
   VK_FROM_HANDLE(vk_device, device, _device);
   VK_FROM_HANDLE(vk_semaphore, semaphore, pSignalInfo->semaphore);
   struct vk_sync *sync = vk_semaphore_get_active_sync(semaphore);
   VkResult result;

   /* From the Vulkan 1.2.194 spec:
    *
    *    UID-VkSemaphoreSignalInfo-semaphore-03257
    *
    *    "semaphore must have been created with a VkSemaphoreType of
    *    VK_SEMAPHORE_TYPE_TIMELINE."
    */
   assert(semaphore->type == VK_SEMAPHORE_TYPE_TIMELINE);

   /* From the Vulkan 1.2.194 spec:
    *
    *    VUID-VkSemaphoreSignalInfo-value-03258
    *
    *    "value must have a value greater than the current value of the
    *    semaphore"
    *
    * Since 0 is the lowest possible semaphore timeline value, we can assert
    * that a non-zero signal value is provided.
    */
   if (unlikely(pSignalInfo->value == 0)) {
      return vk_device_set_lost(device,
         "Tried to signal a timeline with value 0");
   }

   result = vk_sync_signal(device, sync, pSignalInfo->value);
   if (unlikely(result != VK_SUCCESS))
      return result;

   if (device->submit_mode == VK_QUEUE_SUBMIT_MODE_DEFERRED) {
      result = vk_device_flush(device);
      if (unlikely(result != VK_SUCCESS))
         return result;
   }

   return VK_SUCCESS;
}

#ifdef _WIN32

VKAPI_ATTR VkResult VKAPI_CALL
vk_common_ImportSemaphoreWin32HandleKHR(VkDevice _device,
                                        const VkImportSemaphoreWin32HandleInfoKHR *pImportSemaphoreWin32HandleInfo)
{
   VK_FROM_HANDLE(vk_device, device, _device);
   VK_FROM_HANDLE(vk_semaphore, semaphore, pImportSemaphoreWin32HandleInfo->semaphore);

   assert(pImportSemaphoreWin32HandleInfo->sType ==
          VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_WIN32_HANDLE_INFO_KHR);

   const HANDLE handle = pImportSemaphoreWin32HandleInfo->handle;
   const wchar_t *name = pImportSemaphoreWin32HandleInfo->name;
   const VkExternalSemaphoreHandleTypeFlagBits handle_type =
      pImportSemaphoreWin32HandleInfo->handleType;

   struct vk_sync *temporary = NULL, *sync;
   if (pImportSemaphoreWin32HandleInfo->flags & VK_SEMAPHORE_IMPORT_TEMPORARY_BIT) {
      /* From the Vulkan 1.2.194 spec:
       *
       *    VUID-VkImportSemaphoreWin32HandleInfoKHR-flags-03322
       *
       *    "If flags contains VK_SEMAPHORE_IMPORT_TEMPORARY_BIT, the
       *    VkSemaphoreTypeCreateInfo::semaphoreType field of the semaphore
       *    from which handle or name was exported must not be
       *    VK_SEMAPHORE_TYPE_TIMELINE"
       */
      if (unlikely(semaphore->type == VK_SEMAPHORE_TYPE_TIMELINE)) {
         return vk_errorf(device, VK_ERROR_UNKNOWN,
                          "Cannot temporarily import into a timeline "
                          "semaphore");
      }

      const struct vk_sync_type *sync_type =
         get_semaphore_sync_type(device->physical, semaphore->type, handle_type);

      VkResult result = vk_sync_create(device, sync_type, 0 /* flags */,
                                       0 /* initial_value */, &temporary);
      if (result != VK_SUCCESS)
         return result;

      sync = temporary;
   } else {
      sync = &semaphore->permanent;
   }
   assert(handle_type &
          vk_sync_semaphore_handle_types(sync->type, semaphore->type));

   VkResult result;
   switch (pImportSemaphoreWin32HandleInfo->handleType) {
   case VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT:
   case VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_D3D12_FENCE_BIT:
      result = vk_sync_import_win32_handle(device, sync, handle, name);
      break;

   default:
      result = vk_error(semaphore, VK_ERROR_INVALID_EXTERNAL_HANDLE);
   }

   if (result != VK_SUCCESS) {
      if (temporary != NULL)
         vk_sync_destroy(device, temporary);
      return result;
   }

   /* From a spec correctness point of view, we could probably replace the
    * semaphore's temporary payload with the new vk_sync at the top.  However,
    * we choose to be nice to applications and only replace the semaphore if
    * the import succeeded.
    */
   if (temporary) {
      vk_semaphore_reset_temporary(device, semaphore);
      semaphore->temporary = temporary;
   }

   return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_common_GetSemaphoreWin32HandleKHR(VkDevice _device,
                                     const VkSemaphoreGetWin32HandleInfoKHR *pGetWin32HandleInfo,
                                     HANDLE *pHandle)
{
   VK_FROM_HANDLE(vk_device, device, _device);
   VK_FROM_HANDLE(vk_semaphore, semaphore, pGetWin32HandleInfo->semaphore);

   assert(pGetWin32HandleInfo->sType == VK_STRUCTURE_TYPE_SEMAPHORE_GET_WIN32_HANDLE_INFO_KHR);

   struct vk_sync *sync = vk_semaphore_get_active_sync(semaphore);

   VkResult result;
   switch (pGetWin32HandleInfo->handleType) {
   case VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT:
   case VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_D3D12_FENCE_BIT:
      result = vk_sync_export_win32_handle(device, sync, pHandle);
      if (result != VK_SUCCESS)
         return result;
      break;

   default:
      unreachable("Invalid semaphore export handle type");
   }

   /* From the Vulkan 1.2.194 spec:
    *
    *    "Export operations have the same transference as the specified
    *    handle type’s import operations. [...] If the semaphore was using
    *    a temporarily imported payload, the semaphore’s prior permanent
    *    payload will be restored."
    */
   vk_semaphore_reset_temporary(device, semaphore);

   return VK_SUCCESS;
}

#else

VKAPI_ATTR VkResult VKAPI_CALL
vk_common_ImportSemaphoreFdKHR(VkDevice _device,
                               const VkImportSemaphoreFdInfoKHR *pImportSemaphoreFdInfo)
{
   VK_FROM_HANDLE(vk_device, device, _device);
   VK_FROM_HANDLE(vk_semaphore, semaphore, pImportSemaphoreFdInfo->semaphore);

   assert(pImportSemaphoreFdInfo->sType ==
          VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_FD_INFO_KHR);

   const int fd = pImportSemaphoreFdInfo->fd;
   const VkExternalSemaphoreHandleTypeFlagBits handle_type =
      pImportSemaphoreFdInfo->handleType;

   struct vk_sync *temporary = NULL, *sync;
   if (pImportSemaphoreFdInfo->flags & VK_SEMAPHORE_IMPORT_TEMPORARY_BIT) {
      /* From the Vulkan 1.2.194 spec:
       *
       *    VUID-VkImportSemaphoreFdInfoKHR-flags-03323
       *
       *    "If flags contains VK_SEMAPHORE_IMPORT_TEMPORARY_BIT, the
       *    VkSemaphoreTypeCreateInfo::semaphoreType field of the semaphore
       *    from which handle or name was exported must not be
       *    VK_SEMAPHORE_TYPE_TIMELINE"
       */
      if (unlikely(semaphore->type == VK_SEMAPHORE_TYPE_TIMELINE)) {
         return vk_errorf(device, VK_ERROR_UNKNOWN,
                          "Cannot temporarily import into a timeline "
                          "semaphore");
      }

      const struct vk_sync_type *sync_type =
         get_semaphore_sync_type(device->physical, semaphore->type, handle_type);

      VkResult result = vk_sync_create(device, sync_type, 0 /* flags */,
                                       0 /* initial_value */, &temporary);
      if (result != VK_SUCCESS)
         return result;

      sync = temporary;
   } else {
      sync = &semaphore->permanent;
   }
   assert(handle_type &
          vk_sync_semaphore_handle_types(sync->type, semaphore->type));

   VkResult result;
   switch (pImportSemaphoreFdInfo->handleType) {
   case VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT:
      result = vk_sync_import_opaque_fd(device, sync, fd);
      break;

   case VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT:
      result = vk_sync_import_sync_file(device, sync, fd);
      break;

   default:
      result = vk_error(semaphore, VK_ERROR_INVALID_EXTERNAL_HANDLE);
   }

   if (result != VK_SUCCESS) {
      if (temporary != NULL)
         vk_sync_destroy(device, temporary);
      return result;
   }

   /* From the Vulkan 1.2.194 spec:
    *
    *    "Importing a semaphore payload from a file descriptor transfers
    *    ownership of the file descriptor from the application to the Vulkan
    *    implementation. The application must not perform any operations on
    *    the file descriptor after a successful import."
    *
    * If the import fails, we leave the file descriptor open.
    */
   if (fd != -1)
      close(fd);

   /* From a spec correctness point of view, we could probably replace the
    * semaphore's temporary payload with the new vk_sync at the top.  However,
    * we choose to be nice to applications and only replace the semaphore if
    * the import succeeded.
    */
   if (temporary) {
      vk_semaphore_reset_temporary(device, semaphore);
      semaphore->temporary = temporary;
   }

   return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_common_GetSemaphoreFdKHR(VkDevice _device,
                            const VkSemaphoreGetFdInfoKHR *pGetFdInfo,
                            int *pFd)
{
   VK_FROM_HANDLE(vk_device, device, _device);
   VK_FROM_HANDLE(vk_semaphore, semaphore, pGetFdInfo->semaphore);

   assert(pGetFdInfo->sType == VK_STRUCTURE_TYPE_SEMAPHORE_GET_FD_INFO_KHR);

   struct vk_sync *sync = vk_semaphore_get_active_sync(semaphore);

   VkResult result;
   switch (pGetFdInfo->handleType) {
   case VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT:
      result = vk_sync_export_opaque_fd(device, sync, pFd);
      if (result != VK_SUCCESS)
         return result;
      break;

   case VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT:
      /* From the Vulkan 1.2.194 spec:
       *
       *    VUID-VkSemaphoreGetFdInfoKHR-handleType-03253
       *
       *    "If handleType refers to a handle type with copy payload
       *    transference semantics, semaphore must have been created with a
       *    VkSemaphoreType of VK_SEMAPHORE_TYPE_BINARY."
       */
      if (unlikely(semaphore->type != VK_SEMAPHORE_TYPE_BINARY)) {
         return vk_errorf(device, VK_ERROR_INVALID_EXTERNAL_HANDLE,
                          "Cannot export a timeline semaphore as SYNC_FD");
      }

      /* From the Vulkan 1.2.194 spec:
       *    VUID-VkSemaphoreGetFdInfoKHR-handleType-03254
       *
       *    "If handleType refers to a handle type with copy payload
       *    transference semantics, semaphore must have an associated
       *    semaphore signal operation that has been submitted for execution
       *    and any semaphore signal operations on which it depends (if any)
       *    must have also been submitted for execution."
       *
       * If we have real timelines, it's possible that the time point doesn't
       * exist yet and is waiting for one of our submit threads to trigger.
       * However, thanks to the above bit of spec text, that wait should never
       * block for long.
       */
      if (vk_device_supports_threaded_submit(device)) {
         result = vk_sync_wait(device, sync, 0,
                               VK_SYNC_WAIT_PENDING,
                               UINT64_MAX);
         if (unlikely(result != VK_SUCCESS))
            return result;
      }

      result = vk_sync_export_sync_file(device, sync, pFd);
      if (unlikely(result != VK_SUCCESS))
         return result;

      /* From the Vulkan 1.2.194 spec:
       *
       *    "Export operations have the same transference as the specified
       *    handle type’s import operations. Additionally, exporting a
       *    semaphore payload to a handle with copy transference has the same
       *    side effects on the source semaphore’s payload as executing a
       *    semaphore wait operation."
       *
       * In other words, exporting a sync file also resets the semaphore.  We
       * only care about this for the permanent payload because the temporary
       * payload will be destroyed below.
       */
      if (sync == &semaphore->permanent) {
         result = vk_sync_reset(device, sync);
         if (unlikely(result != VK_SUCCESS))
            return result;
      }
      break;

   default:
      unreachable("Invalid semaphore export handle type");
   }

   /* From the Vulkan 1.2.194 spec:
    *
    *    "Export operations have the same transference as the specified
    *    handle type’s import operations. [...] If the semaphore was using
    *    a temporarily imported payload, the semaphore’s prior permanent
    *    payload will be restored."
    */
   vk_semaphore_reset_temporary(device, semaphore);

   return VK_SUCCESS;
}

#endif /* !defined(_WIN32) */
