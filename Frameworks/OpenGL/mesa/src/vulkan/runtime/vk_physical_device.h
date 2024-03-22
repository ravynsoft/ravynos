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
#ifndef VK_PHYSICAL_DEVICE_H
#define VK_PHYSICAL_DEVICE_H

#include "vk_dispatch_table.h"
#include "vk_extensions.h"
#include "vk_object.h"
#include "vk_physical_device_features.h"
#include "vk_physical_device_properties.h"

#include "util/list.h"

#ifdef __cplusplus
extern "C" {
#endif

struct disk_cache;
struct wsi_device;
struct vk_sync_type;
struct vk_pipeline_cache_object_ops;

/** Base struct for all VkPhysicalDevice implementations
 */
struct vk_physical_device {
   struct vk_object_base base;

   /* See vk_instance::pdevices::list */
   struct list_head link;

   /** Instance which is the parent of this physical device */
   struct vk_instance *instance;

   /** Table of all supported device extensions
    *
    * This table is initialized from the `supported_extensions` parameter
    * passed to `vk_physical_device_init()` if not `NULL`.  If a `NULL`
    * extension table is passed, all extensions are initialized to false and
    * it's the responsibility of the driver to populate the table.  This may
    * be useful if the driver's physical device initialization order is such
    * that extension support cannot be determined until significant physical
    * device setup work has already been done.
    */
   struct vk_device_extension_table supported_extensions;

   /** Table of all supported features
    *
    * This table is initialized from the `supported_features` parameter
    * passed to `vk_physical_device_init()` if not `NULL`.  If a `NULL`
    * features table is passed, all features are initialized to false and
    * it's the responsibility of the driver to populate the table.  This may
    * be useful if the driver's physical device initialization order is such
    * that feature support cannot be determined until significant physical
    * device setup work has already been done.
    */
   struct vk_features supported_features;

   /** Table of all physical device properties which is initialized similarly
    * to supported_features
    */
   struct vk_properties properties;

   /** Physical-device-level dispatch table */
   struct vk_physical_device_dispatch_table dispatch_table;

   /** Disk cache, or NULL */
   struct disk_cache *disk_cache;

   /** WSI device, or NULL */
   struct wsi_device *wsi_device;

   /** A null-terminated array of supported sync types, in priority order
    *
    * The common implementations of VkFence and VkSemaphore use this list to
    * determine what vk_sync_type to use for each scenario.  The list is
    * walked and the first vk_sync_type matching their criterion is taken.
    * For instance, VkFence requires that it not be a timeline and support
    * reset and CPU wait.  If an external handle type is requested, that is
    * considered just one more criterion.
    */
   const struct vk_sync_type *const *supported_sync_types;

   /** A null-terminated array of supported pipeline cache object types
    *
    * The common implementation of VkPipelineCache uses this to remember the
    * type of objects stored in the cache and deserialize them immediately
    * when importing the cache. If an object type isn't in this list, then it
    * will be loaded as a raw data object and then deserialized when we first
    * look it up. Deserializing immediately avoids a copy but may be more
    * expensive for objects that aren't hit.
    */
   const struct vk_pipeline_cache_object_ops *const *pipeline_cache_import_ops;
};

VK_DEFINE_HANDLE_CASTS(vk_physical_device, base, VkPhysicalDevice,
                       VK_OBJECT_TYPE_PHYSICAL_DEVICE);

/** Initialize a vk_physical_device
 *
 * :param physical_device:      |out| The physical device to initialize
 * :param instance:             |in|  The instance which is the parent of this
 *                                    physical device
 * :param supported_extensions: |in|  Table of all device extensions supported
 *                                    by this physical device
 * :param supported_features:   |in|  Table of all features supported by this
 *                                    physical device
 * :param dispatch_table:       |in|  Physical-device-level dispatch table
 */
VkResult MUST_CHECK
vk_physical_device_init(struct vk_physical_device *physical_device,
                        struct vk_instance *instance,
                        const struct vk_device_extension_table *supported_extensions,
                        const struct vk_features *supported_features,
                        const struct vk_properties *properties,
                        const struct vk_physical_device_dispatch_table *dispatch_table);

/** Tears down a vk_physical_device
 *
 * :param physical_device:      |out| The physical device to tear down
 */
void
vk_physical_device_finish(struct vk_physical_device *physical_device);

VkResult
vk_physical_device_check_device_features(struct vk_physical_device *physical_device,
                                         const VkDeviceCreateInfo *pCreateInfo);

#ifdef __cplusplus
}
#endif

#endif /* VK_PHYSICAL_DEVICE_H */
