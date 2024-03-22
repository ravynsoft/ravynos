/*
 * Mesa 3-D graphics library
 *
 * Copyright © 2017, Google Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#include "panvk_private.h"

#include <hardware/hardware.h>
#include <hardware/hwvulkan.h>
#include <vulkan/vk_icd.h>

#include "util/log.h"

static int panvk_hal_open(const struct hw_module_t *mod, const char *id,
                          struct hw_device_t **dev);
static int panvk_hal_close(struct hw_device_t *dev);

static_assert(HWVULKAN_DISPATCH_MAGIC == ICD_LOADER_MAGIC, "");

PUBLIC struct hwvulkan_module_t HAL_MODULE_INFO_SYM = {
   .common =
      {
         .tag = HARDWARE_MODULE_TAG,
         .module_api_version = HWVULKAN_MODULE_API_VERSION_0_1,
         .hal_api_version = HARDWARE_MAKE_API_VERSION(1, 0),
         .id = HWVULKAN_HARDWARE_MODULE_ID,
         .name = "ARM Vulkan HAL",
         .author = "Mesa3D",
         .methods =
            &(hw_module_methods_t){
               .open = panvk_hal_open,
            },
      },
};

static int
panvk_hal_open(const struct hw_module_t *mod, const char *id,
               struct hw_device_t **dev)
{
   assert(mod == &HAL_MODULE_INFO_SYM.common);
   assert(strcmp(id, HWVULKAN_DEVICE_0) == 0);

   hwvulkan_device_t *hal_dev = malloc(sizeof(*hal_dev));
   if (!hal_dev)
      return -1;

   *hal_dev = (hwvulkan_device_t){
      .common =
         {
            .tag = HARDWARE_DEVICE_TAG,
            .version = HWVULKAN_DEVICE_API_VERSION_0_1,
            .module = &HAL_MODULE_INFO_SYM.common,
            .close = panvk_hal_close,
         },
      .EnumerateInstanceExtensionProperties =
         panvk_EnumerateInstanceExtensionProperties,
      .CreateInstance = panvk_CreateInstance,
      .GetInstanceProcAddr = panvk_GetInstanceProcAddr,
   };

   mesa_logi("panvk: Warning: Android Vulkan implementation is experimental");

   *dev = &hal_dev->common;
   return 0;
}

static int
panvk_hal_close(struct hw_device_t *dev)
{
   /* hwvulkan.h claims that hw_device_t::close() is never called. */
   return -1;
}
