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
#ifndef VK_INSTANCE_H
#define VK_INSTANCE_H

#include "vk_dispatch_table.h"
#include "vk_extensions.h"
#include "vk_object.h"

#include "c11/threads.h"
#include "util/list.h"
#include "util/u_debug.h"

#ifdef __cplusplus
extern "C" {
#endif

struct vk_app_info {
   /** VkApplicationInfo::pApplicationName */
   const char*        app_name;

   /** VkApplicationInfo::applicationVersion */
   uint32_t           app_version;

   /** VkApplicationInfo::pEngineName */
   const char*        engine_name;

   /** VkApplicationInfo::engineVersion */
   uint32_t           engine_version;

   /** VkApplicationInfo::apiVersion or `VK_API_VERSION_1_0`
    *
    * If the application does not provide a `pApplicationInfo` or the
    * `apiVersion` field is 0, this is set to `VK_API_VERSION_1_0`.
    */
   uint32_t           api_version;
};

struct _drmDevice;
struct vk_physical_device;

enum vk_trace_mode {
   /** Radeon Memory Visualizer */
   VK_TRACE_MODE_RMV = 1 << 0,

   /** Number of common trace modes. */
   VK_TRACE_MODE_COUNT = 1,
};

/** Base struct for all `VkInstance` implementations
 *
 * This contains data structures necessary for detecting enabled extensions,
 * handling entrypoint dispatch, and implementing `vkGetInstanceProcAddr()`.
 * It also contains data copied from the `VkInstanceCreateInfo` such as the
 * application information.
 */
struct vk_instance {
   struct vk_object_base base;

   /** Allocator used when creating this instance
    *
    * This is used as a fall-back for when a NULL pAllocator is passed into a
    * device-level create function such as vkCreateImage().
    */
   VkAllocationCallbacks alloc;

   /** VkInstanceCreateInfo::pApplicationInfo */
   struct vk_app_info app_info;

   /** Table of all supported instance extensions
    *
    * This is the static const struct passed by the driver as the
    * `supported_extensions` parameter to `vk_instance_init()`.
    */
   const struct vk_instance_extension_table *supported_extensions;

   /** Table of all enabled instance extensions
    *
    * This is generated automatically as part of `vk_instance_init()` from
    * VkInstanceCreateInfo::ppEnabledExtensionNames.
    */
   struct vk_instance_extension_table enabled_extensions;

   /** Instance-level dispatch table */
   struct vk_instance_dispatch_table dispatch_table;

   /* VK_EXT_debug_report debug callbacks */
   struct {
      mtx_t callbacks_mutex;
      struct list_head callbacks;
   } debug_report;

   /* VK_EXT_debug_utils */
   struct {
      /* These callbacks are only used while creating or destroying an
       * instance
       */
      struct list_head instance_callbacks;
      mtx_t callbacks_mutex;
      /* Persistent callbacks */
      struct list_head callbacks;
   } debug_utils;

   /** List of all physical devices and callbacks
   *
   * This is used for automatic physical device creation,
   * deletion and enumeration.
   */
   struct {
      struct list_head list;
      bool enumerated;

      /** Enumerate physical devices for this instance
       *
       * The driver can implement this callback for custom physical device
       * enumeration. The returned value must be a valid return code of
       * vkEnumeratePhysicalDevices.
       *
       * Note that the loader calls vkEnumeratePhysicalDevices of all
       * installed ICDs and fails device enumeration when any of the calls
       * fails. The driver should return VK_SUCCESS when it does not find any
       * compatible device.
       *
       * If this callback is not set, try_create_for_drm will be used for
       * enumeration.
       */
      VkResult (*enumerate)(struct vk_instance *instance);

      /** Try to create a physical device for a drm device
       *
       * The returned value must be a valid return code of
       * vkEnumeratePhysicalDevices, or VK_ERROR_INCOMPATIBLE_DRIVER. When
       * VK_ERROR_INCOMPATIBLE_DRIVER is returned, the error and the drm
       * device are silently ignored.
       */
      VkResult (*try_create_for_drm)(struct vk_instance *instance,
                                     struct _drmDevice *device,
                                     struct vk_physical_device **out);

      /** Handle the destruction of a physical device
       *
       * This callback has to be implemented when using common physical device
       * management. The device pointer and any resource allocated for the
       * device should be freed here.
       */
      void (*destroy)(struct vk_physical_device *pdevice);

      mtx_t mutex;
   } physical_devices;

   /** Enabled tracing modes */
   uint64_t trace_mode;

   uint32_t trace_frame;
   char *trace_trigger_file;
};

VK_DEFINE_HANDLE_CASTS(vk_instance, base, VkInstance,
                       VK_OBJECT_TYPE_INSTANCE);

/** Initialize a vk_instance
 *
 * Along with initializing the data structures in `vk_instance`, this function
 * validates the Vulkan version number provided by the client and checks that
 * every extension specified by
 * ``VkInstanceCreateInfo::ppEnabledExtensionNames`` is actually supported by
 * the implementation and returns `VK_ERROR_EXTENSION_NOT_PRESENT` if an
 * unsupported extension is requested.
 *
 * :param instance:             |out| The instance to initialize
 * :param supported_extensions: |in|  Table of all instance extensions supported
 *                                    by this instance
 * :param dispatch_table:       |in|  Instance-level dispatch table
 * :param pCreateInfo:          |in|  VkInstanceCreateInfo pointer passed to
 *                                    `vkCreateInstance()`
 * :param alloc:                |in|  Allocation callbacks used to create this
 *                                    instance; must not be `NULL`
 */
VkResult MUST_CHECK
vk_instance_init(struct vk_instance *instance,
                 const struct vk_instance_extension_table *supported_extensions,
                 const struct vk_instance_dispatch_table *dispatch_table,
                 const VkInstanceCreateInfo *pCreateInfo,
                 const VkAllocationCallbacks *alloc);

/** Tears down a vk_instance
 *
 * :param instance:     |out| The instance to tear down
 */
void
vk_instance_finish(struct vk_instance *instance);

/** Implementaiton of vkEnumerateInstanceExtensionProperties() */
VkResult
vk_enumerate_instance_extension_properties(
    const struct vk_instance_extension_table *supported_extensions,
    uint32_t *pPropertyCount,
    VkExtensionProperties *pProperties);

/** Implementaiton of vkGetInstanceProcAddr() */
PFN_vkVoidFunction
vk_instance_get_proc_addr(const struct vk_instance *instance,
                          const struct vk_instance_entrypoint_table *entrypoints,
                          const char *name);

/** Unchecked version of vk_instance_get_proc_addr
 *
 * This is identical to `vk_instance_get_proc_addr()` except that it doesn't
 * check whether extensions are enabled before returning function pointers.
 * This is useful in window-system code where we may use extensions without
 * the client explicitly enabling them.
 */
PFN_vkVoidFunction
vk_instance_get_proc_addr_unchecked(const struct vk_instance *instance,
                                    const char *name);

/** Implementaiton of vk_icdGetPhysicalDeviceProcAddr() */
PFN_vkVoidFunction
vk_instance_get_physical_device_proc_addr(const struct vk_instance *instance,
                                          const char *name);

void
vk_instance_add_driver_trace_modes(struct vk_instance *instance,
                                   const struct debug_control *modes);

uint32_t
vk_get_negotiated_icd_version(void);

#ifdef __cplusplus
}
#endif

#endif /* VK_INSTANCE_H */
