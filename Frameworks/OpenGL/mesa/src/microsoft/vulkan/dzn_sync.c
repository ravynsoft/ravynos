/*
 * Copyright © Microsoft Corporation
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "dzn_private.h"

#include "vk_alloc.h"
#include "vk_debug_report.h"
#include "vk_util.h"

#include "util/macros.h"
#include "util/os_time.h"

#ifndef _WIN32
#include <sys/eventfd.h>
#include <libsync.h>
#endif

static VkResult
dzn_sync_init(struct vk_device *device,
              struct vk_sync *sync,
              uint64_t initial_value)
{
   struct dzn_sync *dsync = container_of(sync, struct dzn_sync, vk);
   struct dzn_device *ddev = container_of(device, struct dzn_device, vk);

   D3D12_FENCE_FLAGS flags = (sync->flags & VK_SYNC_IS_SHAREABLE) ?
      D3D12_FENCE_FLAG_SHARED : D3D12_FENCE_FLAG_NONE;

   if (FAILED(ID3D12Device1_CreateFence(ddev->dev, initial_value,
                                        flags,
                                        &IID_ID3D12Fence,
                                        (void **)&dsync->fence)))
      return vk_error(device, VK_ERROR_OUT_OF_DEVICE_MEMORY);

   return VK_SUCCESS;
}

static void
dzn_sync_finish(struct vk_device *device,
                struct vk_sync *sync)
{
   struct dzn_sync *dsync = container_of(sync, struct dzn_sync, vk);

   ID3D12Fence_Release(dsync->fence);

#ifdef _WIN32
   if (dsync->export_handle)
      CloseHandle(dsync->export_handle);
#endif
}

static VkResult
dzn_sync_signal(struct vk_device *device,
                struct vk_sync *sync,
                uint64_t value)
{
   struct dzn_sync *dsync = container_of(sync, struct dzn_sync, vk);

   if (!(sync->flags & VK_SYNC_IS_TIMELINE))
      value = 1;

   if (FAILED(ID3D12Fence_Signal(dsync->fence, value)))
      return vk_error(device, VK_ERROR_OUT_OF_DEVICE_MEMORY);

   return VK_SUCCESS;
}

static VkResult
dzn_sync_get_value(struct vk_device *device,
                   struct vk_sync *sync,
                   uint64_t *value)
{
   struct dzn_sync *dsync = container_of(sync, struct dzn_sync, vk);

   *value = ID3D12Fence_GetCompletedValue(dsync->fence);
   return VK_SUCCESS;
}

static VkResult
dzn_sync_reset(struct vk_device *device,
               struct vk_sync *sync)
{
   struct dzn_sync *dsync = container_of(sync, struct dzn_sync, vk);

   if (FAILED(ID3D12Fence_Signal(dsync->fence, 0)))
      return vk_error(device, VK_ERROR_OUT_OF_DEVICE_MEMORY);

   return VK_SUCCESS;
}

static VkResult
dzn_sync_move(struct vk_device *device,
              struct vk_sync *dst,
              struct vk_sync *src)
{
   struct dzn_device *ddev = container_of(device, struct dzn_device, vk);
   struct dzn_sync *ddst = container_of(dst, struct dzn_sync, vk);
   struct dzn_sync *dsrc = container_of(src, struct dzn_sync, vk);
   ID3D12Fence *new_fence;

   if (FAILED(ID3D12Device1_CreateFence(ddev->dev, 0,
                                        D3D12_FENCE_FLAG_NONE,
                                        &IID_ID3D12Fence,
                                        (void **)&new_fence)))
      return vk_error(device, VK_ERROR_OUT_OF_DEVICE_MEMORY);

   ID3D12Fence_Release(ddst->fence);
   ddst->fence = dsrc->fence;
   dsrc->fence = new_fence;
   return VK_SUCCESS;
}

static VkResult
dzn_sync_wait(struct vk_device *device,
              uint32_t wait_count,
              const struct vk_sync_wait *waits,
              enum vk_sync_wait_flags wait_flags,
              uint64_t abs_timeout_ns)
{
   struct dzn_device *ddev = container_of(device, struct dzn_device, vk);

#ifdef _WIN32
   HANDLE event = CreateEventA(NULL, false, false, NULL);
   if (event == NULL)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);
#else
   int event_fd = eventfd(0, EFD_CLOEXEC);
   if (event_fd == -1)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);
   /* The D3D12 event-based APIs in WSL expect an eventfd file descriptor cast to HANDLE */
   HANDLE event = (HANDLE)(intptr_t)event_fd;
#endif

   STACK_ARRAY(ID3D12Fence *, fences, wait_count);
   STACK_ARRAY(uint64_t, values, wait_count);

   for (uint32_t i = 0; i < wait_count; i++) {
      struct dzn_sync *sync = container_of(waits[i].sync, struct dzn_sync, vk);

      fences[i] = sync->fence;
      values[i] = (sync->vk.flags & VK_SYNC_IS_TIMELINE) ? waits[i].wait_value : 1;
   }

   D3D12_MULTIPLE_FENCE_WAIT_FLAGS flags =
      (wait_flags & VK_SYNC_WAIT_ANY) ?
      D3D12_MULTIPLE_FENCE_WAIT_FLAG_ANY :
      D3D12_MULTIPLE_FENCE_WAIT_FLAG_ALL;

   if (FAILED(ID3D12Device1_SetEventOnMultipleFenceCompletion(ddev->dev,
                                                              fences,
                                                              values,
                                                              wait_count,
                                                              flags,
                                                              event))) {
      STACK_ARRAY_FINISH(fences);
      STACK_ARRAY_FINISH(values);
#ifdef _WIN32
      CloseHandle(event);
#else
      close(event_fd);
#endif
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);
   }

#ifdef _WIN32
   DWORD timeout_ms;
   static const DWORD timeout_infinite = INFINITE;
#else
   int timeout_ms;
   static const int timeout_infinite = -1;
#endif

   if (abs_timeout_ns == OS_TIMEOUT_INFINITE) {
      timeout_ms = timeout_infinite;
   } else {
      uint64_t cur_time = os_time_get_nano();
      uint64_t rel_timeout_ns =
         abs_timeout_ns > cur_time ? abs_timeout_ns - cur_time : 0;

      timeout_ms = (rel_timeout_ns / 1000000) + (rel_timeout_ns % 1000000 ? 1 : 0);
   }

#ifdef _WIN32
   DWORD res =
      WaitForSingleObject(event, timeout_ms);

   CloseHandle(event);
   VkResult ret = VK_SUCCESS;
   if (res == WAIT_TIMEOUT)
      ret = VK_TIMEOUT;
   else if (res != WAIT_OBJECT_0)
      ret = vk_error(device, VK_ERROR_UNKNOWN);
#else
   VkResult ret = sync_wait(event_fd, timeout_ms) != 0 ? VK_TIMEOUT : VK_SUCCESS;
   close(event_fd);
#endif

   STACK_ARRAY_FINISH(fences);
   STACK_ARRAY_FINISH(values);

   return ret;
}

#ifdef _WIN32
static VkResult
dzn_sync_import_win32_handle(struct vk_device *device, struct vk_sync *sync,
                             HANDLE handle, LPCWSTR name)
{
   struct dzn_sync *dsync = container_of(sync, struct dzn_sync, vk);
   struct dzn_device *ddev = container_of(device, struct dzn_device, vk);

   dzn_sync_finish(device, sync);

   HANDLE handle_to_close = NULL;
   if (name) {
      if (FAILED(ID3D12Device_OpenSharedHandleByName(ddev->dev, name, GENERIC_ALL, &handle_to_close)))
         return vk_error(device, VK_ERROR_INVALID_EXTERNAL_HANDLE);
      handle = handle_to_close;
   }

   HRESULT hr = ID3D12Device_OpenSharedHandle(ddev->dev, handle, &IID_ID3D12Fence, (void **)&dsync->fence);
   if (handle_to_close)
      CloseHandle(handle_to_close);
   if (FAILED(hr))
      return vk_error(device, VK_ERROR_INVALID_EXTERNAL_HANDLE);
   return VK_SUCCESS;
}

static VkResult
dzn_sync_export_win32_handle(struct vk_device *device, struct vk_sync *sync, HANDLE *handle)
{
   struct dzn_sync *dsync = container_of(sync, struct dzn_sync, vk);
   if (dsync->export_handle) {
      if (!DuplicateHandle(GetCurrentProcess(), dsync->export_handle,
                           GetCurrentProcess(), handle,
                           0, false, DUPLICATE_SAME_ACCESS))
         return vk_error(device, VK_ERROR_OUT_OF_DEVICE_MEMORY);
      return VK_SUCCESS;
   }

   struct dzn_device *ddev = container_of(device, struct dzn_device, vk);
   if (FAILED(ID3D12Device_CreateSharedHandle(ddev->dev, (ID3D12DeviceChild *)dsync->fence,
                                              NULL, GENERIC_ALL, NULL, handle)))
      return vk_error(device, VK_ERROR_OUT_OF_DEVICE_MEMORY);
   return VK_SUCCESS;
}

static VkResult
dzn_sync_prep_win32_export(struct vk_device *device, struct vk_sync *sync,
                           const void *security_attributes, uint32_t access,
                           const wchar_t *name)
{
   struct dzn_sync *dsync = container_of(sync, struct dzn_sync, vk);
   struct dzn_device *ddev = container_of(device, struct dzn_device, vk);
   if (FAILED(ID3D12Device_CreateSharedHandle(ddev->dev, (ID3D12DeviceChild *)dsync->fence,
                                              (const LPSECURITY_ATTRIBUTES)security_attributes,
                                              GENERIC_ALL, name, &dsync->export_handle)))
      return vk_error(device, VK_ERROR_OUT_OF_DEVICE_MEMORY);
   return VK_SUCCESS;
}
#else
static VkResult
dzn_sync_import_opaque_fd(struct vk_device *device, struct vk_sync *sync,
                          int fd)
{
   struct dzn_sync *dsync = container_of(sync, struct dzn_sync, vk);
   struct dzn_device *ddev = container_of(device, struct dzn_device, vk);

   dzn_sync_finish(device, sync);

   HANDLE handle = (HANDLE)(intptr_t)fd;
   if (FAILED(ID3D12Device_OpenSharedHandle(ddev->dev, handle, &IID_ID3D12Fence, (void **)&dsync->fence)))
      return vk_error(device, VK_ERROR_INVALID_EXTERNAL_HANDLE);
   return VK_SUCCESS;
}

static VkResult
dzn_sync_export_opaque_fd(struct vk_device *device, struct vk_sync *sync, int *fd)
{
   struct dzn_sync *dsync = container_of(sync, struct dzn_sync, vk);
   struct dzn_device *ddev = container_of(device, struct dzn_device, vk);
   HANDLE handle;
   if (FAILED(ID3D12Device_CreateSharedHandle(ddev->dev, (ID3D12DeviceChild *)dsync->fence,
                                              NULL, GENERIC_ALL, NULL, &handle)))
      return vk_error(device, VK_ERROR_OUT_OF_DEVICE_MEMORY);
   *fd = (int)(intptr_t)handle;
   return VK_SUCCESS;
}
#endif

const struct vk_sync_type dzn_sync_type = {
   .size = sizeof(struct dzn_sync),
   .features = (enum vk_sync_features)
      (VK_SYNC_FEATURE_TIMELINE |
       VK_SYNC_FEATURE_GPU_WAIT |
       VK_SYNC_FEATURE_CPU_WAIT |
       VK_SYNC_FEATURE_CPU_SIGNAL |
       VK_SYNC_FEATURE_WAIT_ANY |
       VK_SYNC_FEATURE_WAIT_BEFORE_SIGNAL),

   .init = dzn_sync_init,
   .finish = dzn_sync_finish,
   .signal = dzn_sync_signal,
   .get_value = dzn_sync_get_value,
   .reset = dzn_sync_reset,
   .move = dzn_sync_move,
   .wait_many = dzn_sync_wait,
#ifdef _WIN32
   .import_win32_handle = dzn_sync_import_win32_handle,
   .export_win32_handle = dzn_sync_export_win32_handle,
   .set_win32_export_params = dzn_sync_prep_win32_export,
#else
   .import_opaque_fd = dzn_sync_import_opaque_fd,
   .export_opaque_fd = dzn_sync_export_opaque_fd,
#endif
};
