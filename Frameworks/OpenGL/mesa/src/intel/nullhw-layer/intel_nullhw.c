/*
 * Copyright © 2019 Intel Corporation
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

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include <vulkan/vulkan.h>
#include <vulkan/vk_layer.h>

#include "util/u_debug.h"
#include "util/hash_table.h"
#include "util/macros.h"
#include "util/simple_mtx.h"

#include "vk_dispatch_table.h"
#include "vk_enum_to_str.h"
#include "vk_util.h"

struct instance_data {
   struct vk_instance_dispatch_table vtable;
   VkInstance instance;
};

struct device_data {
   struct instance_data *instance;

   PFN_vkSetDeviceLoaderData set_device_loader_data;

   struct vk_device_dispatch_table vtable;
   VkPhysicalDevice physical_device;
   VkDevice device;
};

static struct hash_table_u64 *vk_object_to_data = NULL;
static simple_mtx_t vk_object_to_data_mutex = SIMPLE_MTX_INITIALIZER;

static inline void ensure_vk_object_map(void)
{
   if (!vk_object_to_data)
      vk_object_to_data = _mesa_hash_table_u64_create(NULL);
}

#define HKEY(obj) ((uint64_t)(obj))
#define FIND(type, obj) ((type *)find_object_data(HKEY(obj)))

static void *find_object_data(uint64_t obj)
{
   simple_mtx_lock(&vk_object_to_data_mutex);
   ensure_vk_object_map();
   void *data = _mesa_hash_table_u64_search(vk_object_to_data, obj);
   simple_mtx_unlock(&vk_object_to_data_mutex);
   return data;
}

static void map_object(uint64_t obj, void *data)
{
   simple_mtx_lock(&vk_object_to_data_mutex);
   ensure_vk_object_map();
   _mesa_hash_table_u64_insert(vk_object_to_data, obj, data);
   simple_mtx_unlock(&vk_object_to_data_mutex);
}

static void unmap_object(uint64_t obj)
{
   simple_mtx_lock(&vk_object_to_data_mutex);
   _mesa_hash_table_u64_remove(vk_object_to_data, obj);
   simple_mtx_unlock(&vk_object_to_data_mutex);
}

/**/

#define VK_CHECK(expr) \
   do { \
      VkResult __result = (expr); \
      if (__result != VK_SUCCESS) { \
         fprintf(stderr, "'%s' line %i failed with %s\n", \
                 #expr, __LINE__, vk_Result_to_str(__result)); \
      } \
   } while (0)

/**/

static void override_queue(struct device_data *device_data,
                           VkDevice device,
                           uint32_t queue_family_index,
                           VkQueue queue)
{
   VkCommandPoolCreateInfo cmd_buffer_pool_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .queueFamilyIndex = queue_family_index,
   };
   VkCommandPool cmd_pool;
   VK_CHECK(device_data->vtable.CreateCommandPool(device,
                                                  &cmd_buffer_pool_info,
                                                  NULL, &cmd_pool));


   VkCommandBufferAllocateInfo cmd_buffer_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = cmd_pool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1,
   };
   VkCommandBuffer cmd_buffer;
   VK_CHECK(device_data->vtable.AllocateCommandBuffers(device,
                                                       &cmd_buffer_info,
                                                       &cmd_buffer));
   VK_CHECK(device_data->set_device_loader_data(device, cmd_buffer));

   VkCommandBufferBeginInfo buffer_begin_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
   };
   device_data->vtable.BeginCommandBuffer(cmd_buffer, &buffer_begin_info);

   VkPerformanceOverrideInfoINTEL override_info = {
      .sType = VK_STRUCTURE_TYPE_PERFORMANCE_OVERRIDE_INFO_INTEL,
      .type = VK_PERFORMANCE_OVERRIDE_TYPE_NULL_HARDWARE_INTEL,
      .enable = VK_TRUE,
   };
   device_data->vtable.CmdSetPerformanceOverrideINTEL(cmd_buffer, &override_info);

   device_data->vtable.EndCommandBuffer(cmd_buffer);

   VkSubmitInfo submit_info = {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .commandBufferCount = 1,
      .pCommandBuffers = &cmd_buffer,
   };
   VK_CHECK(device_data->vtable.QueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE));

   VK_CHECK(device_data->vtable.QueueWaitIdle(queue));

   device_data->vtable.DestroyCommandPool(device, cmd_pool, NULL);
}

static void device_override_queues(struct device_data *device_data,
                                   const VkDeviceCreateInfo *pCreateInfo)
{
   for (uint32_t i = 0; i < pCreateInfo->queueCreateInfoCount; i++) {
      for (uint32_t j = 0; j < pCreateInfo->pQueueCreateInfos[i].queueCount; j++) {
         VkQueue queue;
         device_data->vtable.GetDeviceQueue(device_data->device,
                                            pCreateInfo->pQueueCreateInfos[i].queueFamilyIndex,
                                            j, &queue);

         VK_CHECK(device_data->set_device_loader_data(device_data->device, queue));

         override_queue(device_data, device_data->device,
                        pCreateInfo->pQueueCreateInfos[i].queueFamilyIndex, queue);
      }
   }
}

static VkLayerDeviceCreateInfo *get_device_chain_info(const VkDeviceCreateInfo *pCreateInfo,
                                                      VkLayerFunction func)
{
   vk_foreach_struct_const(item, pCreateInfo->pNext) {
      if (item->sType == VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO &&
          ((VkLayerDeviceCreateInfo *) item)->function == func)
         return (VkLayerDeviceCreateInfo *)item;
   }
   unreachable("device chain info not found");
   return NULL;
}

static struct device_data *new_device_data(VkDevice device, struct instance_data *instance)
{
   struct device_data *data = calloc(1, sizeof(*data));
   data->instance = instance;
   data->device = device;
   map_object(HKEY(data->device), data);
   return data;
}

static void destroy_device_data(struct device_data *data)
{
   unmap_object(HKEY(data->device));
   free(data);
}

static VkResult nullhw_CreateDevice(
    VkPhysicalDevice                            physicalDevice,
    const VkDeviceCreateInfo*                   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDevice*                                   pDevice)
{
   VkLayerDeviceCreateInfo *chain_info =
      get_device_chain_info(pCreateInfo, VK_LAYER_LINK_INFO);

   assert(chain_info->u.pLayerInfo);
   PFN_vkGetInstanceProcAddr fpGetInstanceProcAddr = chain_info->u.pLayerInfo->pfnNextGetInstanceProcAddr;
   PFN_vkGetDeviceProcAddr fpGetDeviceProcAddr = chain_info->u.pLayerInfo->pfnNextGetDeviceProcAddr;
   PFN_vkCreateDevice fpCreateDevice = (PFN_vkCreateDevice)fpGetInstanceProcAddr(NULL, "vkCreateDevice");
   if (fpCreateDevice == NULL) {
      return VK_ERROR_INITIALIZATION_FAILED;
   }

   // Advance the link info for the next element on the chain
   chain_info->u.pLayerInfo = chain_info->u.pLayerInfo->pNext;

   VkDeviceCreateInfo device_info = *pCreateInfo;
   const char **extensions = calloc(device_info.enabledExtensionCount + 1, sizeof(*extensions));
   bool found = false;
   for (uint32_t i = 0; i < device_info.enabledExtensionCount; i++) {
      if (!strcmp(device_info.ppEnabledExtensionNames[i], "VK_INTEL_performance_query")) {
         found = true;
         break;
      }
   }
   if (!found) {
      memcpy(extensions, device_info.ppEnabledExtensionNames,
             sizeof(*extensions) * device_info.enabledExtensionCount);
      extensions[device_info.enabledExtensionCount++] = "VK_INTEL_performance_query";
      device_info.ppEnabledExtensionNames = extensions;
   }

   VkResult result = fpCreateDevice(physicalDevice, &device_info, pAllocator, pDevice);
   free(extensions);
   if (result != VK_SUCCESS) return result;

   struct instance_data *instance_data = FIND(struct instance_data, physicalDevice);
   struct device_data *device_data = new_device_data(*pDevice, instance_data);
   device_data->physical_device = physicalDevice;
   vk_device_dispatch_table_load(&device_data->vtable, fpGetDeviceProcAddr, *pDevice);

   VkLayerDeviceCreateInfo *load_data_info =
      get_device_chain_info(pCreateInfo, VK_LOADER_DATA_CALLBACK);
   device_data->set_device_loader_data = load_data_info->u.pfnSetDeviceLoaderData;

   device_override_queues(device_data, pCreateInfo);

   return result;
}

static void nullhw_DestroyDevice(
    VkDevice                                    device,
    const VkAllocationCallbacks*                pAllocator)
{
   struct device_data *device_data = FIND(struct device_data, device);
   device_data->vtable.DestroyDevice(device, pAllocator);
   destroy_device_data(device_data);
}

static struct instance_data *new_instance_data(VkInstance instance)
{
   struct instance_data *data = calloc(1, sizeof(*data));
   data->instance = instance;
   map_object(HKEY(data->instance), data);
   return data;
}

static void destroy_instance_data(struct instance_data *data)
{
   unmap_object(HKEY(data->instance));
   free(data);
}

static VkLayerInstanceCreateInfo *get_instance_chain_info(const VkInstanceCreateInfo *pCreateInfo,
                                                          VkLayerFunction func)
{
   vk_foreach_struct_const(item, pCreateInfo->pNext) {
      if (item->sType == VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO &&
          ((VkLayerInstanceCreateInfo *) item)->function == func)
         return (VkLayerInstanceCreateInfo *) item;
   }
   unreachable("instance chain info not found");
   return NULL;
}

static VkResult nullhw_CreateInstance(
    const VkInstanceCreateInfo*                 pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkInstance*                                 pInstance)
{
   VkLayerInstanceCreateInfo *chain_info =
      get_instance_chain_info(pCreateInfo, VK_LAYER_LINK_INFO);

   assert(chain_info->u.pLayerInfo);
   PFN_vkGetInstanceProcAddr fpGetInstanceProcAddr =
      chain_info->u.pLayerInfo->pfnNextGetInstanceProcAddr;
   PFN_vkCreateInstance fpCreateInstance =
      (PFN_vkCreateInstance)fpGetInstanceProcAddr(NULL, "vkCreateInstance");
   if (fpCreateInstance == NULL) {
      return VK_ERROR_INITIALIZATION_FAILED;
   }

   // Advance the link info for the next element on the chain
   chain_info->u.pLayerInfo = chain_info->u.pLayerInfo->pNext;

   VkResult result = fpCreateInstance(pCreateInfo, pAllocator, pInstance);
   if (result != VK_SUCCESS) return result;

   struct instance_data *instance_data = new_instance_data(*pInstance);
   vk_instance_dispatch_table_load(&instance_data->vtable,
                                   fpGetInstanceProcAddr,
                                   instance_data->instance);

   return result;
}

static void nullhw_DestroyInstance(
    VkInstance                                  instance,
    const VkAllocationCallbacks*                pAllocator)
{
   struct instance_data *instance_data = FIND(struct instance_data, instance);
   instance_data->vtable.DestroyInstance(instance, pAllocator);
   destroy_instance_data(instance_data);
}

static const struct {
   const char *name;
   void *ptr;
} name_to_funcptr_map[] = {
   { "vkGetDeviceProcAddr", (void *) vkGetDeviceProcAddr },
#define ADD_HOOK(fn) { "vk" # fn, (void *) nullhw_ ## fn }
   ADD_HOOK(CreateInstance),
   ADD_HOOK(DestroyInstance),
   ADD_HOOK(CreateDevice),
   ADD_HOOK(DestroyDevice),
};

static void *find_ptr(const char *name)
{
   for (uint32_t i = 0; i < ARRAY_SIZE(name_to_funcptr_map); i++) {
      if (strcmp(name, name_to_funcptr_map[i].name) == 0)
         return name_to_funcptr_map[i].ptr;
   }

   return NULL;
}

PUBLIC VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetDeviceProcAddr(VkDevice dev,
                                                                    const char *funcName)
{
   void *ptr = find_ptr(funcName);
   if (ptr) return (PFN_vkVoidFunction)(ptr);

   if (dev == NULL) return NULL;

   struct device_data *device_data = FIND(struct device_data, dev);
   if (device_data->vtable.GetDeviceProcAddr == NULL) return NULL;
   return device_data->vtable.GetDeviceProcAddr(dev, funcName);
}

PUBLIC VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetInstanceProcAddr(VkInstance instance,
                                                                      const char *funcName)
{
   void *ptr = find_ptr(funcName);
   if (ptr) return (PFN_vkVoidFunction) ptr;

   struct instance_data *instance_data = FIND(struct instance_data, instance);
   if (instance_data->vtable.GetInstanceProcAddr == NULL) return NULL;
   return instance_data->vtable.GetInstanceProcAddr(instance, funcName);
}
