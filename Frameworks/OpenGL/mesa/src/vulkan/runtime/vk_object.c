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

#include "vk_object.h"

#include "vk_alloc.h"
#include "vk_common_entrypoints.h"
#include "vk_instance.h"
#include "vk_device.h"
#include "util/hash_table.h"
#include "util/ralloc.h"
#include "vk_enum_to_str.h"

void
vk_object_base_init(struct vk_device *device,
                    struct vk_object_base *base,
                    VkObjectType obj_type)
{
   base->_loader_data.loaderMagic = ICD_LOADER_MAGIC;
   base->type = obj_type;
   base->device = device;
   base->instance = NULL;
   base->client_visible = false;
   base->object_name = NULL;
   util_sparse_array_init(&base->private_data, sizeof(uint64_t), 8);
}

void vk_object_base_instance_init(struct vk_instance *instance,
                                  struct vk_object_base *base,
                                  VkObjectType obj_type)
{
   base->_loader_data.loaderMagic = ICD_LOADER_MAGIC;
   base->type = obj_type;
   base->device = NULL;
   base->instance = instance;
   base->client_visible = false;
   base->object_name = NULL;
   util_sparse_array_init(&base->private_data, sizeof(uint64_t), 8);
}

void
vk_object_base_finish(struct vk_object_base *base)
{
   util_sparse_array_finish(&base->private_data);

   if (base->object_name == NULL)
      return;

   assert(base->device != NULL || base->instance != NULL);
   if (base->device)
      vk_free(&base->device->alloc, base->object_name);
   else
      vk_free(&base->instance->alloc, base->object_name);
}

void
vk_object_base_recycle(struct vk_object_base *base)
{
   struct vk_device *device = base->device;
   VkObjectType obj_type = base->type;
   vk_object_base_finish(base);
   vk_object_base_init(device, base, obj_type);
}

void *
vk_object_alloc(struct vk_device *device,
                const VkAllocationCallbacks *alloc,
                size_t size,
                VkObjectType obj_type)
{
   void *ptr = vk_alloc2(&device->alloc, alloc, size, 8,
                         VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (ptr == NULL)
      return NULL;

   vk_object_base_init(device, (struct vk_object_base *)ptr, obj_type);

   return ptr;
}

void *
vk_object_zalloc(struct vk_device *device,
                const VkAllocationCallbacks *alloc,
                size_t size,
                VkObjectType obj_type)
{
   void *ptr = vk_zalloc2(&device->alloc, alloc, size, 8,
                         VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (ptr == NULL)
      return NULL;

   vk_object_base_init(device, (struct vk_object_base *)ptr, obj_type);

   return ptr;
}

void *
vk_object_multialloc(struct vk_device *device,
                     struct vk_multialloc *ma,
                     const VkAllocationCallbacks *alloc,
                     VkObjectType obj_type)
{
   void *ptr = vk_multialloc_alloc2(ma, &device->alloc, alloc,
                                    VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (ptr == NULL)
      return NULL;

   vk_object_base_init(device, (struct vk_object_base *)ptr, obj_type);

   return ptr;
}

void *
vk_object_multizalloc(struct vk_device *device,
                      struct vk_multialloc *ma,
                      const VkAllocationCallbacks *alloc,
                      VkObjectType obj_type)
{
   void *ptr = vk_multialloc_zalloc2(ma, &device->alloc, alloc,
                                     VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (ptr == NULL)
      return NULL;

   vk_object_base_init(device, (struct vk_object_base *)ptr, obj_type);

   return ptr;
}

void
vk_object_free(struct vk_device *device,
               const VkAllocationCallbacks *alloc,
               void *data)
{
   vk_object_base_finish((struct vk_object_base *)data);
   vk_free2(&device->alloc, alloc, data);
}

VkResult
vk_private_data_slot_create(struct vk_device *device,
                            const VkPrivateDataSlotCreateInfo* pCreateInfo,
                            const VkAllocationCallbacks* pAllocator,
                            VkPrivateDataSlot* pPrivateDataSlot)
{
   struct vk_private_data_slot *slot =
      vk_alloc2(&device->alloc, pAllocator, sizeof(*slot), 8,
                VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
   if (slot == NULL)
      return VK_ERROR_OUT_OF_HOST_MEMORY;

   vk_object_base_init(device, &slot->base,
                       VK_OBJECT_TYPE_PRIVATE_DATA_SLOT);
   slot->index = p_atomic_inc_return(&device->private_data_next_index);

   *pPrivateDataSlot = vk_private_data_slot_to_handle(slot);

   return VK_SUCCESS;
}

void
vk_private_data_slot_destroy(struct vk_device *device,
                             VkPrivateDataSlot privateDataSlot,
                             const VkAllocationCallbacks *pAllocator)
{
   VK_FROM_HANDLE(vk_private_data_slot, slot, privateDataSlot);
   if (slot == NULL)
      return;

   vk_object_base_finish(&slot->base);
   vk_free2(&device->alloc, pAllocator, slot);
}

static VkResult
get_swapchain_private_data_locked(struct vk_device *device,
                                  uint64_t objectHandle,
                                  struct vk_private_data_slot *slot,
                                  uint64_t **private_data)
{
   if (unlikely(device->swapchain_private == NULL)) {
      /* Even though VkSwapchain/Surface are non-dispatchable objects, we know
       * a priori that these are actually pointers so we can use
       * the pointer hash table for them.
       */
      device->swapchain_private = _mesa_pointer_hash_table_create(NULL);
      if (device->swapchain_private == NULL)
         return VK_ERROR_OUT_OF_HOST_MEMORY;
   }

   struct hash_entry *entry =
      _mesa_hash_table_search(device->swapchain_private,
                              (void *)(uintptr_t)objectHandle);
   if (unlikely(entry == NULL)) {
      struct util_sparse_array *swapchain_private =
         ralloc(device->swapchain_private, struct util_sparse_array);
      util_sparse_array_init(swapchain_private, sizeof(uint64_t), 8);

      entry = _mesa_hash_table_insert(device->swapchain_private,
                                      (void *)(uintptr_t)objectHandle,
                                      swapchain_private);
      if (entry == NULL)
         return VK_ERROR_OUT_OF_HOST_MEMORY;
   }

   struct util_sparse_array *swapchain_private = entry->data;
   *private_data = util_sparse_array_get(swapchain_private, slot->index);

   return VK_SUCCESS;
}

static VkResult
vk_object_base_private_data(struct vk_device *device,
                            VkObjectType objectType,
                            uint64_t objectHandle,
                            VkPrivateDataSlot privateDataSlot,
                            uint64_t **private_data)
{
   VK_FROM_HANDLE(vk_private_data_slot, slot, privateDataSlot);

   /* There is an annoying spec corner here on Android.  Because WSI is
    * implemented in the Vulkan loader which doesn't know about the
    * VK_EXT_private_data extension, we have to handle VkSwapchainKHR in the
    * driver as a special case.  On future versions of Android where the
    * loader does understand VK_EXT_private_data, we'll never see a
    * vkGet/SetPrivateData call on a swapchain because the loader will
    * handle it.
    */
#ifdef ANDROID
   if (objectType == VK_OBJECT_TYPE_SWAPCHAIN_KHR ||
       objectType == VK_OBJECT_TYPE_SURFACE_KHR) {
#else
   if (objectType == VK_OBJECT_TYPE_SURFACE_KHR) {
#endif
      mtx_lock(&device->swapchain_private_mtx);
      VkResult result = get_swapchain_private_data_locked(device, objectHandle,
                                                          slot, private_data);
      mtx_unlock(&device->swapchain_private_mtx);
      return result;
   }

   struct vk_object_base *obj =
      vk_object_base_from_u64_handle(objectHandle, objectType);
   *private_data = util_sparse_array_get(&obj->private_data, slot->index);

   return VK_SUCCESS;
}

VkResult
vk_object_base_set_private_data(struct vk_device *device,
                                VkObjectType objectType,
                                uint64_t objectHandle,
                                VkPrivateDataSlot privateDataSlot,
                                uint64_t data)
{
   uint64_t *private_data;
   VkResult result = vk_object_base_private_data(device,
                                                 objectType, objectHandle,
                                                 privateDataSlot,
                                                 &private_data);
   if (unlikely(result != VK_SUCCESS))
      return result;

   *private_data = data;
   return VK_SUCCESS;
}

void
vk_object_base_get_private_data(struct vk_device *device,
                                VkObjectType objectType,
                                uint64_t objectHandle,
                                VkPrivateDataSlot privateDataSlot,
                                uint64_t *pData)
{
   uint64_t *private_data;
   VkResult result = vk_object_base_private_data(device,
                                                 objectType, objectHandle,
                                                 privateDataSlot,
                                                 &private_data);
   if (likely(result == VK_SUCCESS)) {
      *pData = *private_data;
   } else {
      *pData = 0;
   }
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_common_CreatePrivateDataSlot(VkDevice _device,
                                const VkPrivateDataSlotCreateInfo *pCreateInfo,
                                const VkAllocationCallbacks *pAllocator,
                                VkPrivateDataSlot *pPrivateDataSlot)
{
   VK_FROM_HANDLE(vk_device, device, _device);
   return vk_private_data_slot_create(device, pCreateInfo, pAllocator,
                                      pPrivateDataSlot);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_DestroyPrivateDataSlot(VkDevice _device,
                                 VkPrivateDataSlot privateDataSlot,
                                 const VkAllocationCallbacks *pAllocator)
{
   VK_FROM_HANDLE(vk_device, device, _device);
   vk_private_data_slot_destroy(device, privateDataSlot, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_common_SetPrivateData(VkDevice _device,
                         VkObjectType objectType,
                         uint64_t objectHandle,
                         VkPrivateDataSlot privateDataSlot,
                         uint64_t data)
{
   VK_FROM_HANDLE(vk_device, device, _device);
   return vk_object_base_set_private_data(device,
                                          objectType, objectHandle,
                                          privateDataSlot, data);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_GetPrivateData(VkDevice _device,
                         VkObjectType objectType,
                         uint64_t objectHandle,
                         VkPrivateDataSlot privateDataSlot,
                         uint64_t *pData)
{
   VK_FROM_HANDLE(vk_device, device, _device);
   vk_object_base_get_private_data(device,
                                   objectType, objectHandle,
                                   privateDataSlot, pData);
}

const char *
vk_object_base_name(struct vk_object_base *obj)
{
   if (obj->object_name)
      return obj->object_name;

   obj->object_name = vk_asprintf(&obj->device->alloc,
                                  VK_SYSTEM_ALLOCATION_SCOPE_DEVICE,
                                  "%s(0x%"PRIx64")",
                                  vk_ObjectType_to_ObjectName(obj->type),
                                  (uint64_t)(uintptr_t)obj);

   return obj->object_name;
}
