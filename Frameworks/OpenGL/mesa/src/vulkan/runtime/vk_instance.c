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

#include "vk_instance.h"

#include "util/libdrm.h"
#include "util/perf/cpu_trace.h"

#include "vk_alloc.h"
#include "vk_common_entrypoints.h"
#include "vk_dispatch_trampolines.h"
#include "vk_log.h"
#include "vk_util.h"
#include "vk_debug_utils.h"
#include "vk_physical_device.h"

#define VERSION_IS_1_0(version) \
   (VK_API_VERSION_MAJOR(version) == 1 && VK_API_VERSION_MINOR(version) == 0)

static const struct debug_control trace_options[] = {
   {"rmv", VK_TRACE_MODE_RMV},
   {NULL, 0},
};

VkResult
vk_instance_init(struct vk_instance *instance,
                 const struct vk_instance_extension_table *supported_extensions,
                 const struct vk_instance_dispatch_table *dispatch_table,
                 const VkInstanceCreateInfo *pCreateInfo,
                 const VkAllocationCallbacks *alloc)
{
   memset(instance, 0, sizeof(*instance));
   vk_object_base_instance_init(instance, &instance->base, VK_OBJECT_TYPE_INSTANCE);
   instance->alloc = *alloc;

   util_cpu_trace_init();

   /* VK_EXT_debug_utils */
   /* These messengers will only be used during vkCreateInstance or
    * vkDestroyInstance calls.  We do this first so that it's safe to use
    * vk_errorf and friends.
    */
   list_inithead(&instance->debug_utils.instance_callbacks);
   vk_foreach_struct_const(ext, pCreateInfo->pNext) {
      if (ext->sType ==
          VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT) {
         const VkDebugUtilsMessengerCreateInfoEXT *debugMessengerCreateInfo =
            (const VkDebugUtilsMessengerCreateInfoEXT *)ext;
         struct vk_debug_utils_messenger *messenger =
            vk_alloc2(alloc, alloc, sizeof(struct vk_debug_utils_messenger), 8,
                      VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

         if (!messenger)
            return vk_error(instance, VK_ERROR_OUT_OF_HOST_MEMORY);

         vk_object_base_instance_init(instance, &messenger->base,
                                      VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT);

         messenger->alloc = *alloc;
         messenger->severity = debugMessengerCreateInfo->messageSeverity;
         messenger->type = debugMessengerCreateInfo->messageType;
         messenger->callback = debugMessengerCreateInfo->pfnUserCallback;
         messenger->data = debugMessengerCreateInfo->pUserData;

         list_addtail(&messenger->link,
                      &instance->debug_utils.instance_callbacks);
      }
   }

   uint32_t instance_version = VK_API_VERSION_1_0;
   if (dispatch_table->EnumerateInstanceVersion)
      dispatch_table->EnumerateInstanceVersion(&instance_version);

   instance->app_info = (struct vk_app_info) { .api_version = 0 };
   if (pCreateInfo->pApplicationInfo) {
      const VkApplicationInfo *app = pCreateInfo->pApplicationInfo;

      instance->app_info.app_name =
         vk_strdup(&instance->alloc, app->pApplicationName,
                   VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
      instance->app_info.app_version = app->applicationVersion;

      instance->app_info.engine_name =
         vk_strdup(&instance->alloc, app->pEngineName,
                   VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
      instance->app_info.engine_version = app->engineVersion;

      instance->app_info.api_version = app->apiVersion;
   }

   /* From the Vulkan 1.2.199 spec:
    *
    *    "Note:
    *
    *    Providing a NULL VkInstanceCreateInfo::pApplicationInfo or providing
    *    an apiVersion of 0 is equivalent to providing an apiVersion of
    *    VK_MAKE_API_VERSION(0,1,0,0)."
    */
   if (instance->app_info.api_version == 0)
      instance->app_info.api_version = VK_API_VERSION_1_0;

   /* From the Vulkan 1.2.199 spec:
    *
    *    VUID-VkApplicationInfo-apiVersion-04010
    *
    *    "If apiVersion is not 0, then it must be greater than or equal to
    *    VK_API_VERSION_1_0"
    */
   assert(instance->app_info.api_version >= VK_API_VERSION_1_0);

   /* From the Vulkan 1.2.199 spec:
    *
    *    "Vulkan 1.0 implementations were required to return
    *    VK_ERROR_INCOMPATIBLE_DRIVER if apiVersion was larger than 1.0.
    *    Implementations that support Vulkan 1.1 or later must not return
    *    VK_ERROR_INCOMPATIBLE_DRIVER for any value of apiVersion."
    */
   if (VERSION_IS_1_0(instance_version) &&
       !VERSION_IS_1_0(instance->app_info.api_version))
      return VK_ERROR_INCOMPATIBLE_DRIVER;

   instance->supported_extensions = supported_extensions;

   for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
      int idx;
      for (idx = 0; idx < VK_INSTANCE_EXTENSION_COUNT; idx++) {
         if (strcmp(pCreateInfo->ppEnabledExtensionNames[i],
                    vk_instance_extensions[idx].extensionName) == 0)
            break;
      }

      if (idx >= VK_INSTANCE_EXTENSION_COUNT)
         return vk_errorf(instance, VK_ERROR_EXTENSION_NOT_PRESENT,
                          "%s not supported",
                          pCreateInfo->ppEnabledExtensionNames[i]);

      if (!supported_extensions->extensions[idx])
         return vk_errorf(instance, VK_ERROR_EXTENSION_NOT_PRESENT,
                          "%s not supported",
                          pCreateInfo->ppEnabledExtensionNames[i]);

#ifdef ANDROID_STRICT
      if (!vk_android_allowed_instance_extensions.extensions[idx])
         return vk_errorf(instance, VK_ERROR_EXTENSION_NOT_PRESENT,
                          "%s not supported",
                          pCreateInfo->ppEnabledExtensionNames[i]);
#endif

      instance->enabled_extensions.extensions[idx] = true;
   }

   instance->dispatch_table = *dispatch_table;

   /* Add common entrypoints without overwriting driver-provided ones. */
   vk_instance_dispatch_table_from_entrypoints(
      &instance->dispatch_table, &vk_common_instance_entrypoints, false);

   if (mtx_init(&instance->debug_report.callbacks_mutex, mtx_plain) != 0)
      return vk_error(instance, VK_ERROR_INITIALIZATION_FAILED);

   list_inithead(&instance->debug_report.callbacks);

   if (mtx_init(&instance->debug_utils.callbacks_mutex, mtx_plain) != 0) {
      mtx_destroy(&instance->debug_report.callbacks_mutex);
      return vk_error(instance, VK_ERROR_INITIALIZATION_FAILED);
   }

   list_inithead(&instance->debug_utils.callbacks);

   list_inithead(&instance->physical_devices.list);

   if (mtx_init(&instance->physical_devices.mutex, mtx_plain) != 0) {
      mtx_destroy(&instance->debug_report.callbacks_mutex);
      mtx_destroy(&instance->debug_utils.callbacks_mutex);
      return vk_error(instance, VK_ERROR_INITIALIZATION_FAILED);
   }

   instance->trace_mode = parse_debug_string(getenv("MESA_VK_TRACE"), trace_options);
   instance->trace_frame = (uint32_t)debug_get_num_option("MESA_VK_TRACE_FRAME", 0xFFFFFFFF);
   instance->trace_trigger_file = secure_getenv("MESA_VK_TRACE_TRIGGER");

   vk_compiler_cache_init();

   return VK_SUCCESS;
}

static void
destroy_physical_devices(struct vk_instance *instance)
{
   list_for_each_entry_safe(struct vk_physical_device, pdevice,
                            &instance->physical_devices.list, link) {
      list_del(&pdevice->link);
      instance->physical_devices.destroy(pdevice);
   }
}

void
vk_instance_finish(struct vk_instance *instance)
{
   destroy_physical_devices(instance);

   vk_compiler_cache_finish();
   if (unlikely(!list_is_empty(&instance->debug_utils.callbacks))) {
      list_for_each_entry_safe(struct vk_debug_utils_messenger, messenger,
                               &instance->debug_utils.callbacks, link) {
         list_del(&messenger->link);
         vk_object_base_finish(&messenger->base);
         vk_free2(&instance->alloc, &messenger->alloc, messenger);
      }
   }
   if (unlikely(!list_is_empty(&instance->debug_utils.instance_callbacks))) {
      list_for_each_entry_safe(struct vk_debug_utils_messenger, messenger,
                               &instance->debug_utils.instance_callbacks,
                               link) {
         list_del(&messenger->link);
         vk_object_base_finish(&messenger->base);
         vk_free2(&instance->alloc, &messenger->alloc, messenger);
      }
   }
   mtx_destroy(&instance->debug_report.callbacks_mutex);
   mtx_destroy(&instance->debug_utils.callbacks_mutex);
   mtx_destroy(&instance->physical_devices.mutex);
   vk_free(&instance->alloc, (char *)instance->app_info.app_name);
   vk_free(&instance->alloc, (char *)instance->app_info.engine_name);
   vk_object_base_finish(&instance->base);
}

VkResult
vk_enumerate_instance_extension_properties(
    const struct vk_instance_extension_table *supported_extensions,
    uint32_t *pPropertyCount,
    VkExtensionProperties *pProperties)
{
   VK_OUTARRAY_MAKE_TYPED(VkExtensionProperties, out, pProperties, pPropertyCount);

   for (int i = 0; i < VK_INSTANCE_EXTENSION_COUNT; i++) {
      if (!supported_extensions->extensions[i])
         continue;

#ifdef ANDROID_STRICT
      if (!vk_android_allowed_instance_extensions.extensions[i])
         continue;
#endif

      vk_outarray_append_typed(VkExtensionProperties, &out, prop) {
         *prop = vk_instance_extensions[i];
      }
   }

   return vk_outarray_status(&out);
}

PFN_vkVoidFunction
vk_instance_get_proc_addr(const struct vk_instance *instance,
                          const struct vk_instance_entrypoint_table *entrypoints,
                          const char *name)
{
   PFN_vkVoidFunction func;

   /* The Vulkan 1.0 spec for vkGetInstanceProcAddr has a table of exactly
    * when we have to return valid function pointers, NULL, or it's left
    * undefined.  See the table for exact details.
    */
   if (name == NULL)
      return NULL;

#define LOOKUP_VK_ENTRYPOINT(entrypoint) \
   if (strcmp(name, "vk" #entrypoint) == 0) \
      return (PFN_vkVoidFunction)entrypoints->entrypoint

   LOOKUP_VK_ENTRYPOINT(EnumerateInstanceExtensionProperties);
   LOOKUP_VK_ENTRYPOINT(EnumerateInstanceLayerProperties);
   LOOKUP_VK_ENTRYPOINT(EnumerateInstanceVersion);
   LOOKUP_VK_ENTRYPOINT(CreateInstance);

   /* GetInstanceProcAddr() can also be called with a NULL instance.
    * See https://gitlab.khronos.org/vulkan/vulkan/issues/2057
    */
   LOOKUP_VK_ENTRYPOINT(GetInstanceProcAddr);

#undef LOOKUP_VK_ENTRYPOINT

   /* Beginning with ICD interface v7, the following functions can also be
    * retrieved via vk_icdGetInstanceProcAddr.
    */

   if (strcmp(name, "vk_icdNegotiateLoaderICDInterfaceVersion") == 0)
      return (PFN_vkVoidFunction)vk_icdNegotiateLoaderICDInterfaceVersion;
   if (strcmp(name, "vk_icdGetPhysicalDeviceProcAddr") == 0)
      return (PFN_vkVoidFunction)vk_icdGetPhysicalDeviceProcAddr;
#ifdef _WIN32
   if (strcmp(name, "vk_icdEnumerateAdapterPhysicalDevices") == 0)
      return (PFN_vkVoidFunction)vk_icdEnumerateAdapterPhysicalDevices;
#endif

   if (instance == NULL)
      return NULL;

   func = vk_instance_dispatch_table_get_if_supported(&instance->dispatch_table,
                                                      name,
                                                      instance->app_info.api_version,
                                                      &instance->enabled_extensions);
   if (func != NULL)
      return func;

   func = vk_physical_device_dispatch_table_get_if_supported(&vk_physical_device_trampolines,
                                                             name,
                                                             instance->app_info.api_version,
                                                             &instance->enabled_extensions);
   if (func != NULL)
      return func;

   func = vk_device_dispatch_table_get_if_supported(&vk_device_trampolines,
                                                    name,
                                                    instance->app_info.api_version,
                                                    &instance->enabled_extensions,
                                                    NULL);
   if (func != NULL)
      return func;

   return NULL;
}

PFN_vkVoidFunction
vk_instance_get_proc_addr_unchecked(const struct vk_instance *instance,
                                    const char *name)
{
   PFN_vkVoidFunction func;

   if (instance == NULL || name == NULL)
      return NULL;

   func = vk_instance_dispatch_table_get(&instance->dispatch_table, name);
   if (func != NULL)
      return func;

   func = vk_physical_device_dispatch_table_get(
      &vk_physical_device_trampolines, name);
   if (func != NULL)
      return func;

   func = vk_device_dispatch_table_get(&vk_device_trampolines, name);
   if (func != NULL)
      return func;

   return NULL;
}

PFN_vkVoidFunction
vk_instance_get_physical_device_proc_addr(const struct vk_instance *instance,
                                          const char *name)
{
   if (instance == NULL || name == NULL)
      return NULL;

   return vk_physical_device_dispatch_table_get_if_supported(&vk_physical_device_trampolines,
                                                             name,
                                                             instance->app_info.api_version,
                                                             &instance->enabled_extensions);
}

void
vk_instance_add_driver_trace_modes(struct vk_instance *instance,
                                   const struct debug_control *modes)
{
   instance->trace_mode |= parse_debug_string(getenv("MESA_VK_TRACE"), modes);
}

static VkResult
enumerate_drm_physical_devices_locked(struct vk_instance *instance)
{
   /* TODO: Check for more devices ? */
   drmDevicePtr devices[8];
   int max_devices = drmGetDevices2(0, devices, ARRAY_SIZE(devices));

   if (max_devices < 1)
      return VK_SUCCESS;

   VkResult result;
   for (uint32_t i = 0; i < (uint32_t)max_devices; i++) {
      struct vk_physical_device *pdevice;
      result = instance->physical_devices.try_create_for_drm(instance, devices[i], &pdevice);

      /* Incompatible DRM device, skip. */
      if (result == VK_ERROR_INCOMPATIBLE_DRIVER) {
         result = VK_SUCCESS;
         continue;
      }

      /* Error creating the physical device, report the error. */
      if (result != VK_SUCCESS)
         break;

      list_addtail(&pdevice->link, &instance->physical_devices.list);
   }

   drmFreeDevices(devices, max_devices);
   return result;
}

static VkResult
enumerate_physical_devices_locked(struct vk_instance *instance)
{
   if (instance->physical_devices.enumerate) {
      VkResult result = instance->physical_devices.enumerate(instance);
      if (result != VK_ERROR_INCOMPATIBLE_DRIVER)
         return result;
   }

   VkResult result = VK_SUCCESS;

   if (instance->physical_devices.try_create_for_drm) {
      result = enumerate_drm_physical_devices_locked(instance);
      if (result != VK_SUCCESS) {
         destroy_physical_devices(instance);
         return result;
      }
   }

   return result;
}

static VkResult
enumerate_physical_devices(struct vk_instance *instance)
{
   VkResult result = VK_SUCCESS;

   mtx_lock(&instance->physical_devices.mutex);
   if (!instance->physical_devices.enumerated) {
      result = enumerate_physical_devices_locked(instance);
      if (result == VK_SUCCESS)
         instance->physical_devices.enumerated = true;
   }
   mtx_unlock(&instance->physical_devices.mutex);

   return result;
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_common_EnumeratePhysicalDevices(VkInstance _instance, uint32_t *pPhysicalDeviceCount,
                                   VkPhysicalDevice *pPhysicalDevices)
{
   VK_FROM_HANDLE(vk_instance, instance, _instance);
   VK_OUTARRAY_MAKE_TYPED(VkPhysicalDevice, out, pPhysicalDevices, pPhysicalDeviceCount);

   VkResult result = enumerate_physical_devices(instance);
   if (result != VK_SUCCESS)
      return result;

   list_for_each_entry(struct vk_physical_device, pdevice,
                       &instance->physical_devices.list, link) {
      vk_outarray_append_typed(VkPhysicalDevice, &out, element) {
         *element = vk_physical_device_to_handle(pdevice);
      }
   }

   return vk_outarray_status(&out);
}

#ifdef _WIN32
/* Note: This entrypoint is not exported from ICD DLLs, and is only exposed via
 * vk_icdGetInstanceProcAddr for loaders with interface v7. This is to avoid
 * a design flaw in the original loader implementation, which prevented enumeration
 * of physical devices that didn't have a LUID. This flaw was fixed prior to the
 * implementation of v7, so v7 loaders are unaffected, and it's safe to support this.
 */
VKAPI_ATTR VkResult VKAPI_CALL
vk_icdEnumerateAdapterPhysicalDevices(VkInstance _instance, LUID adapterLUID,
                                      uint32_t *pPhysicalDeviceCount,
                                      VkPhysicalDevice *pPhysicalDevices)
{
   VK_FROM_HANDLE(vk_instance, instance, _instance);
   VK_OUTARRAY_MAKE_TYPED(VkPhysicalDevice, out, pPhysicalDevices, pPhysicalDeviceCount);

   VkResult result = enumerate_physical_devices(instance);
   if (result != VK_SUCCESS)
      return result;

   list_for_each_entry(struct vk_physical_device, pdevice,
                       &instance->physical_devices.list, link) {
      if (pdevice->properties.deviceLUIDValid &&
          memcmp(pdevice->properties.deviceLUID, &adapterLUID, sizeof(adapterLUID)) == 0) {
         vk_outarray_append_typed(VkPhysicalDevice, &out, element) {
            *element = vk_physical_device_to_handle(pdevice);
         }
      }
   }

   return vk_outarray_status(&out);
}
#endif

VKAPI_ATTR VkResult VKAPI_CALL
vk_common_EnumeratePhysicalDeviceGroups(VkInstance _instance, uint32_t *pGroupCount,
                                        VkPhysicalDeviceGroupProperties *pGroupProperties)
{
   VK_FROM_HANDLE(vk_instance, instance, _instance);
   VK_OUTARRAY_MAKE_TYPED(VkPhysicalDeviceGroupProperties, out, pGroupProperties,
                          pGroupCount);

   VkResult result = enumerate_physical_devices(instance);
   if (result != VK_SUCCESS)
      return result;

   list_for_each_entry(struct vk_physical_device, pdevice,
                       &instance->physical_devices.list, link) {
      vk_outarray_append_typed(VkPhysicalDeviceGroupProperties, &out, p) {
         p->physicalDeviceCount = 1;
         memset(p->physicalDevices, 0, sizeof(p->physicalDevices));
         p->physicalDevices[0] = vk_physical_device_to_handle(pdevice);
         p->subsetAllocation = false;
      }
   }

   return vk_outarray_status(&out);
}

/* For Windows, PUBLIC is default-defined to __declspec(dllexport) to automatically export the
 * public entrypoints from a DLL. However, this declspec needs to match between declaration and
 * definition, and this attribute is not present on the prototypes specified in vk_icd.h. Instead,
 * we'll use a .def file to manually export these entrypoints on Windows.
 */
#ifdef _WIN32
#undef PUBLIC
#define PUBLIC
#endif

/* With version 4+ of the loader interface the ICD should expose
 * vk_icdGetPhysicalDeviceProcAddr()
 */
PUBLIC VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
vk_icdGetPhysicalDeviceProcAddr(VkInstance  _instance,
                                const char *pName)
{
   VK_FROM_HANDLE(vk_instance, instance, _instance);
   return vk_instance_get_physical_device_proc_addr(instance, pName);
}

static uint32_t vk_icd_version = 7;

uint32_t
vk_get_negotiated_icd_version(void)
{
   return vk_icd_version;
}

PUBLIC VKAPI_ATTR VkResult VKAPI_CALL
vk_icdNegotiateLoaderICDInterfaceVersion(uint32_t *pSupportedVersion)
{
   /* For the full details on loader interface versioning, see
    * <https://github.com/KhronosGroup/Vulkan-LoaderAndValidationLayers/blob/master/loader/LoaderAndLayerInterface.md>.
    * What follows is a condensed summary, to help you navigate the large and
    * confusing official doc.
    *
    *   - Loader interface v0 is incompatible with later versions. We don't
    *     support it.
    *
    *   - In loader interface v1:
    *       - The first ICD entrypoint called by the loader is
    *         vk_icdGetInstanceProcAddr(). The ICD must statically expose this
    *         entrypoint.
    *       - The ICD must statically expose no other Vulkan symbol unless it is
    *         linked with -Bsymbolic.
    *       - Each dispatchable Vulkan handle created by the ICD must be
    *         a pointer to a struct whose first member is VK_LOADER_DATA. The
    *         ICD must initialize VK_LOADER_DATA.loadMagic to ICD_LOADER_MAGIC.
    *       - The loader implements vkCreate{PLATFORM}SurfaceKHR() and
    *         vkDestroySurfaceKHR(). The ICD must be capable of working with
    *         such loader-managed surfaces.
    *
    *    - Loader interface v2 differs from v1 in:
    *       - The first ICD entrypoint called by the loader is
    *         vk_icdNegotiateLoaderICDInterfaceVersion(). The ICD must
    *         statically expose this entrypoint.
    *
    *    - Loader interface v3 differs from v2 in:
    *        - The ICD must implement vkCreate{PLATFORM}SurfaceKHR(),
    *          vkDestroySurfaceKHR(), and other API which uses VKSurfaceKHR,
    *          because the loader no longer does so.
    *
    *    - Loader interface v4 differs from v3 in:
    *        - The ICD must implement vk_icdGetPhysicalDeviceProcAddr().
    *
    *    - Loader interface v5 differs from v4 in:
    *        - The ICD must support Vulkan API version 1.1 and must not return
    *          VK_ERROR_INCOMPATIBLE_DRIVER from vkCreateInstance() unless a
    *          Vulkan Loader with interface v4 or smaller is being used and the
    *          application provides an API version that is greater than 1.0.
    *
    *    - Loader interface v6 differs from v5 in:
    *        - Windows ICDs may export vk_icdEnumerateAdapterPhysicalDevices,
    *          to tie a physical device to a WDDM adapter LUID. This allows the
    *          loader to sort physical devices according to the same policy as other
    *          graphics APIs.
    *        - Note: A design flaw in the loader implementation of v6 means we do
    *          not actually support returning this function to v6 loaders. See the
    *          comments around the implementation above. It's still fine to report
    *          version number 6 without this method being implemented, however.
    *
    *    - Loader interface v7 differs from v6 in:
    *        - If implemented, the ICD must return the following functions via
    *          vk_icdGetInstanceProcAddr:
    *            - vk_icdNegotiateLoaderICDInterfaceVersion
    *            - vk_icdGetPhysicalDeviceProcAddr
    *            - vk_icdEnumerateAdapterPhysicalDevices
    *          Exporting these functions from the ICD is optional. If
    *          vk_icdNegotiateLoaderICDInterfaceVersion is not exported from the
    *          module, or if VK_LUNARG_direct_driver_loading is being used, then
    *          vk_icdGetInstanceProcAddr will be the first method called, to query
    *          for vk_icdNegotiateLoaderICDInterfaceVersion.
    */
   vk_icd_version = MIN2(vk_icd_version, *pSupportedVersion);
   *pSupportedVersion = vk_icd_version;
   return VK_SUCCESS;
}
