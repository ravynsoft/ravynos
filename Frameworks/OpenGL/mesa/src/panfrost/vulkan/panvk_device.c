/*
 * Copyright © 2021 Collabora Ltd.
 *
 * Derived from tu_device.c which is:
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 * Copyright © 2015 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "panvk_private.h"

#include "pan_bo.h"
#include "pan_encoder.h"
#include "pan_util.h"
#include "vk_cmd_enqueue_entrypoints.h"
#include "vk_common_entrypoints.h"

#include <fcntl.h>
#include <libsync.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <xf86drm.h>
#include <sys/mman.h>
#include <sys/sysinfo.h>

#include "drm-uapi/panfrost_drm.h"

#include "util/disk_cache.h"
#include "util/strtod.h"
#include "util/u_debug.h"
#include "vk_drm_syncobj.h"
#include "vk_format.h"
#include "vk_util.h"

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
#include "wayland-drm-client-protocol.h"
#include <wayland-client.h>
#endif

#include "panvk_cs.h"

VkResult
_panvk_device_set_lost(struct panvk_device *device, const char *file, int line,
                       const char *msg, ...)
{
   /* Set the flag indicating that waits should return in finite time even
    * after device loss.
    */
   p_atomic_inc(&device->_lost);

   /* TODO: Report the log message through VkDebugReportCallbackEXT instead */
   fprintf(stderr, "%s:%d: ", file, line);
   va_list ap;
   va_start(ap, msg);
   vfprintf(stderr, msg, ap);
   va_end(ap);

   if (debug_get_bool_option("PANVK_ABORT_ON_DEVICE_LOSS", false))
      abort();

   return VK_ERROR_DEVICE_LOST;
}

static int
panvk_device_get_cache_uuid(uint16_t family, void *uuid)
{
   uint32_t mesa_timestamp;
   uint16_t f = family;

   if (!disk_cache_get_function_timestamp(panvk_device_get_cache_uuid,
                                          &mesa_timestamp))
      return -1;

   memset(uuid, 0, VK_UUID_SIZE);
   memcpy(uuid, &mesa_timestamp, 4);
   memcpy((char *)uuid + 4, &f, 2);
   snprintf((char *)uuid + 6, VK_UUID_SIZE - 10, "pan");
   return 0;
}

static void
panvk_get_driver_uuid(void *uuid)
{
   memset(uuid, 0, VK_UUID_SIZE);
   snprintf(uuid, VK_UUID_SIZE, "panfrost");
}

static void
panvk_get_device_uuid(void *uuid)
{
   memset(uuid, 0, VK_UUID_SIZE);
}

static const struct debug_control panvk_debug_options[] = {
   {"startup", PANVK_DEBUG_STARTUP},
   {"nir", PANVK_DEBUG_NIR},
   {"trace", PANVK_DEBUG_TRACE},
   {"sync", PANVK_DEBUG_SYNC},
   {"afbc", PANVK_DEBUG_AFBC},
   {"linear", PANVK_DEBUG_LINEAR},
   {"dump", PANVK_DEBUG_DUMP},
   {"no_known_warn", PANVK_DEBUG_NO_KNOWN_WARN},
   {NULL, 0}};

#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
#define PANVK_USE_WSI_PLATFORM
#endif

#define PANVK_API_VERSION VK_MAKE_VERSION(1, 0, VK_HEADER_VERSION)

VkResult
panvk_EnumerateInstanceVersion(uint32_t *pApiVersion)
{
   *pApiVersion = PANVK_API_VERSION;
   return VK_SUCCESS;
}

static const struct vk_instance_extension_table panvk_instance_extensions = {
   .KHR_get_physical_device_properties2 = true,
   .EXT_debug_report = true,
   .EXT_debug_utils = true,

#ifdef PANVK_USE_WSI_PLATFORM
   .KHR_surface = true,
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
   .KHR_wayland_surface = true,
#endif
};

static void
panvk_get_device_extensions(const struct panvk_physical_device *device,
                            struct vk_device_extension_table *ext)
{
   *ext = (struct vk_device_extension_table){
      .KHR_copy_commands2 = true,
      .KHR_storage_buffer_storage_class = true,
      .KHR_descriptor_update_template = true,
#ifdef PANVK_USE_WSI_PLATFORM
      .KHR_swapchain = true,
#endif
      .KHR_synchronization2 = true,
      .KHR_variable_pointers = true,
      .EXT_custom_border_color = true,
      .EXT_index_type_uint8 = true,
      .EXT_vertex_attribute_divisor = true,
   };
}

static void
panvk_get_features(const struct panvk_physical_device *device,
                   struct vk_features *features)
{
   *features = (struct vk_features){
      /* Vulkan 1.0 */
      .robustBufferAccess = true,
      .fullDrawIndexUint32 = true,
      .independentBlend = true,
      .logicOp = true,
      .wideLines = true,
      .largePoints = true,
      .textureCompressionETC2 = true,
      .textureCompressionASTC_LDR = true,
      .shaderUniformBufferArrayDynamicIndexing = true,
      .shaderSampledImageArrayDynamicIndexing = true,
      .shaderStorageBufferArrayDynamicIndexing = true,
      .shaderStorageImageArrayDynamicIndexing = true,

      /* Vulkan 1.1 */
      .storageBuffer16BitAccess = false,
      .uniformAndStorageBuffer16BitAccess = false,
      .storagePushConstant16 = false,
      .storageInputOutput16 = false,
      .multiview = false,
      .multiviewGeometryShader = false,
      .multiviewTessellationShader = false,
      .variablePointersStorageBuffer = true,
      .variablePointers = true,
      .protectedMemory = false,
      .samplerYcbcrConversion = false,
      .shaderDrawParameters = false,

      /* Vulkan 1.2 */
      .samplerMirrorClampToEdge = false,
      .drawIndirectCount = false,
      .storageBuffer8BitAccess = false,
      .uniformAndStorageBuffer8BitAccess = false,
      .storagePushConstant8 = false,
      .shaderBufferInt64Atomics = false,
      .shaderSharedInt64Atomics = false,
      .shaderFloat16 = false,
      .shaderInt8 = false,

      .descriptorIndexing = false,
      .shaderInputAttachmentArrayDynamicIndexing = false,
      .shaderUniformTexelBufferArrayDynamicIndexing = false,
      .shaderStorageTexelBufferArrayDynamicIndexing = false,
      .shaderUniformBufferArrayNonUniformIndexing = false,
      .shaderSampledImageArrayNonUniformIndexing = false,
      .shaderStorageBufferArrayNonUniformIndexing = false,
      .shaderStorageImageArrayNonUniformIndexing = false,
      .shaderInputAttachmentArrayNonUniformIndexing = false,
      .shaderUniformTexelBufferArrayNonUniformIndexing = false,
      .shaderStorageTexelBufferArrayNonUniformIndexing = false,
      .descriptorBindingUniformBufferUpdateAfterBind = false,
      .descriptorBindingSampledImageUpdateAfterBind = false,
      .descriptorBindingStorageImageUpdateAfterBind = false,
      .descriptorBindingStorageBufferUpdateAfterBind = false,
      .descriptorBindingUniformTexelBufferUpdateAfterBind = false,
      .descriptorBindingStorageTexelBufferUpdateAfterBind = false,
      .descriptorBindingUpdateUnusedWhilePending = false,
      .descriptorBindingPartiallyBound = false,
      .descriptorBindingVariableDescriptorCount = false,
      .runtimeDescriptorArray = false,

      .samplerFilterMinmax = false,
      .scalarBlockLayout = false,
      .imagelessFramebuffer = false,
      .uniformBufferStandardLayout = false,
      .shaderSubgroupExtendedTypes = false,
      .separateDepthStencilLayouts = false,
      .hostQueryReset = false,
      .timelineSemaphore = false,
      .bufferDeviceAddress = false,
      .bufferDeviceAddressCaptureReplay = false,
      .bufferDeviceAddressMultiDevice = false,
      .vulkanMemoryModel = false,
      .vulkanMemoryModelDeviceScope = false,
      .vulkanMemoryModelAvailabilityVisibilityChains = false,
      .shaderOutputViewportIndex = false,
      .shaderOutputLayer = false,
      .subgroupBroadcastDynamicId = false,

      /* Vulkan 1.3 */
      .robustImageAccess = false,
      .inlineUniformBlock = false,
      .descriptorBindingInlineUniformBlockUpdateAfterBind = false,
      .pipelineCreationCacheControl = false,
      .privateData = true,
      .shaderDemoteToHelperInvocation = false,
      .shaderTerminateInvocation = false,
      .subgroupSizeControl = false,
      .computeFullSubgroups = false,
      .synchronization2 = true,
      .textureCompressionASTC_HDR = false,
      .shaderZeroInitializeWorkgroupMemory = false,
      .dynamicRendering = false,
      .shaderIntegerDotProduct = false,
      .maintenance4 = false,

      /* VK_EXT_index_type_uint8 */
      .indexTypeUint8 = true,

      /* VK_EXT_vertex_attribute_divisor */
      .vertexAttributeInstanceRateDivisor = true,
      .vertexAttributeInstanceRateZeroDivisor = true,

      /* VK_EXT_depth_clip_enable */
      .depthClipEnable = true,

      /* VK_EXT_4444_formats */
      .formatA4R4G4B4 = true,
      .formatA4B4G4R4 = true,

      /* VK_EXT_custom_border_color */
      .customBorderColors = true,
      .customBorderColorWithoutFormat = true,
   };
}

VkResult panvk_physical_device_try_create(struct vk_instance *vk_instance,
                                          struct _drmDevice *drm_device,
                                          struct vk_physical_device **out);

static void
panvk_physical_device_finish(struct panvk_physical_device *device)
{
   panvk_wsi_finish(device);

   panvk_arch_dispatch(device->pdev.arch, meta_cleanup, device);
   panfrost_close_device(&device->pdev);
   if (device->master_fd != -1)
      close(device->master_fd);

   vk_physical_device_finish(&device->vk);
}

static void
panvk_destroy_physical_device(struct vk_physical_device *device)
{
   panvk_physical_device_finish((struct panvk_physical_device *)device);
   vk_free(&device->instance->alloc, device);
}

VkResult
panvk_CreateInstance(const VkInstanceCreateInfo *pCreateInfo,
                     const VkAllocationCallbacks *pAllocator,
                     VkInstance *pInstance)
{
   struct panvk_instance *instance;
   VkResult result;

   assert(pCreateInfo->sType == VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO);

   pAllocator = pAllocator ?: vk_default_allocator();
   instance = vk_zalloc(pAllocator, sizeof(*instance), 8,
                        VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
   if (!instance)
      return vk_error(NULL, VK_ERROR_OUT_OF_HOST_MEMORY);

   struct vk_instance_dispatch_table dispatch_table;

   vk_instance_dispatch_table_from_entrypoints(
      &dispatch_table, &panvk_instance_entrypoints, true);
   vk_instance_dispatch_table_from_entrypoints(
      &dispatch_table, &wsi_instance_entrypoints, false);
   result = vk_instance_init(&instance->vk, &panvk_instance_extensions,
                             &dispatch_table, pCreateInfo, pAllocator);
   if (result != VK_SUCCESS) {
      vk_free(pAllocator, instance);
      return vk_error(NULL, result);
   }

   instance->vk.physical_devices.try_create_for_drm =
      panvk_physical_device_try_create;
   instance->vk.physical_devices.destroy = panvk_destroy_physical_device;

   instance->debug_flags =
      parse_debug_string(getenv("PANVK_DEBUG"), panvk_debug_options);

   if (instance->debug_flags & PANVK_DEBUG_STARTUP)
      vk_logi(VK_LOG_NO_OBJS(instance), "Created an instance");

   VG(VALGRIND_CREATE_MEMPOOL(instance, 0, false));

   *pInstance = panvk_instance_to_handle(instance);

   return VK_SUCCESS;
}

void
panvk_DestroyInstance(VkInstance _instance,
                      const VkAllocationCallbacks *pAllocator)
{
   VK_FROM_HANDLE(panvk_instance, instance, _instance);

   if (!instance)
      return;

   vk_instance_finish(&instance->vk);
   vk_free(&instance->vk.alloc, instance);
}

static VkResult
panvk_physical_device_init(struct panvk_physical_device *device,
                           struct panvk_instance *instance,
                           drmDevicePtr drm_device)
{
   const char *path = drm_device->nodes[DRM_NODE_RENDER];
   VkResult result = VK_SUCCESS;
   drmVersionPtr version;
   int fd;
   int master_fd = -1;

   if (!getenv("PAN_I_WANT_A_BROKEN_VULKAN_DRIVER")) {
      return vk_errorf(
         instance, VK_ERROR_INCOMPATIBLE_DRIVER,
         "WARNING: panvk is not a conformant vulkan implementation, "
         "pass PAN_I_WANT_A_BROKEN_VULKAN_DRIVER=1 if you know what you're doing.");
   }

   fd = open(path, O_RDWR | O_CLOEXEC);
   if (fd < 0) {
      return vk_errorf(instance, VK_ERROR_INCOMPATIBLE_DRIVER,
                       "failed to open device %s", path);
   }

   version = drmGetVersion(fd);
   if (!version) {
      close(fd);
      return vk_errorf(instance, VK_ERROR_INCOMPATIBLE_DRIVER,
                       "failed to query kernel driver version for device %s",
                       path);
   }

   if (strcmp(version->name, "panfrost")) {
      drmFreeVersion(version);
      close(fd);
      return vk_errorf(instance, VK_ERROR_INCOMPATIBLE_DRIVER,
                       "device %s does not use the panfrost kernel driver",
                       path);
   }

   drmFreeVersion(version);

   if (instance->debug_flags & PANVK_DEBUG_STARTUP)
      vk_logi(VK_LOG_NO_OBJS(instance), "Found compatible device '%s'.", path);

   struct vk_device_extension_table supported_extensions;
   panvk_get_device_extensions(device, &supported_extensions);

   struct vk_features supported_features;
   panvk_get_features(device, &supported_features);

   struct vk_physical_device_dispatch_table dispatch_table;
   vk_physical_device_dispatch_table_from_entrypoints(
      &dispatch_table, &panvk_physical_device_entrypoints, true);
   vk_physical_device_dispatch_table_from_entrypoints(
      &dispatch_table, &wsi_physical_device_entrypoints, false);

   result =
      vk_physical_device_init(&device->vk, &instance->vk, &supported_extensions,
                              &supported_features, NULL, &dispatch_table);

   if (result != VK_SUCCESS) {
      vk_error(instance, result);
      goto fail;
   }

   device->instance = instance;

   if (instance->vk.enabled_extensions.KHR_display) {
      master_fd = open(drm_device->nodes[DRM_NODE_PRIMARY], O_RDWR | O_CLOEXEC);
      if (master_fd >= 0) {
         /* TODO: free master_fd is accel is not working? */
      }
   }

   device->master_fd = master_fd;
   if (instance->debug_flags & PANVK_DEBUG_TRACE)
      device->pdev.debug |= PAN_DBG_TRACE;

   device->pdev.debug |= PAN_DBG_NO_CACHE;
   panfrost_open_device(NULL, fd, &device->pdev);
   fd = -1;

   if (device->pdev.arch <= 5 || device->pdev.arch >= 8) {
      result = vk_errorf(instance, VK_ERROR_INCOMPATIBLE_DRIVER,
                         "%s not supported", device->pdev.model->name);
      goto fail;
   }

   panvk_arch_dispatch(device->pdev.arch, meta_init, device);

   memset(device->name, 0, sizeof(device->name));
   sprintf(device->name, "%s", device->pdev.model->name);

   if (panvk_device_get_cache_uuid(panfrost_device_gpu_id(&device->pdev),
                                   device->cache_uuid)) {
      result = vk_errorf(instance, VK_ERROR_INITIALIZATION_FAILED,
                         "cannot generate UUID");
      goto fail_close_device;
   }

   vk_warn_non_conformant_implementation("panvk");

   panvk_get_driver_uuid(&device->device_uuid);
   panvk_get_device_uuid(&device->device_uuid);

   device->drm_syncobj_type =
      vk_drm_syncobj_get_type(panfrost_device_fd(&device->pdev));
   /* We don't support timelines in the uAPI yet and we don't want it getting
    * suddenly turned on by vk_drm_syncobj_get_type() without us adding panvk
    * code for it first.
    */
   device->drm_syncobj_type.features &= ~VK_SYNC_FEATURE_TIMELINE;

   device->sync_types[0] = &device->drm_syncobj_type;
   device->sync_types[1] = NULL;
   device->vk.supported_sync_types = device->sync_types;

   result = panvk_wsi_init(device);
   if (result != VK_SUCCESS) {
      vk_error(instance, result);
      goto fail_close_device;
   }

   return VK_SUCCESS;

fail_close_device:
   panfrost_close_device(&device->pdev);
fail:
   if (fd != -1)
      close(fd);
   if (master_fd != -1)
      close(master_fd);
   return result;
}

VkResult
panvk_physical_device_try_create(struct vk_instance *vk_instance,
                                 struct _drmDevice *drm_device,
                                 struct vk_physical_device **out)
{
   struct panvk_instance *instance =
      container_of(vk_instance, struct panvk_instance, vk);

   if (!(drm_device->available_nodes & (1 << DRM_NODE_RENDER)) ||
       drm_device->bustype != DRM_BUS_PLATFORM)
      return VK_ERROR_INCOMPATIBLE_DRIVER;

   struct panvk_physical_device *device =
      vk_zalloc(&instance->vk.alloc, sizeof(*device), 8,
                VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
   if (!device)
      return vk_error(instance, VK_ERROR_OUT_OF_HOST_MEMORY);

   VkResult result = panvk_physical_device_init(device, instance, drm_device);
   if (result != VK_SUCCESS) {
      vk_free(&instance->vk.alloc, device);
      return result;
   }

   *out = &device->vk;
   return VK_SUCCESS;
}

void
panvk_GetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice,
                                   VkPhysicalDeviceProperties2 *pProperties)
{
   VK_FROM_HANDLE(panvk_physical_device, pdevice, physicalDevice);

   VkSampleCountFlags sample_counts =
      VK_SAMPLE_COUNT_1_BIT | VK_SAMPLE_COUNT_4_BIT;

   /* make sure that the entire descriptor set is addressable with a signed
    * 32-bit int. So the sum of all limits scaled by descriptor size has to
    * be at most 2 GiB. the combined image & samples object count as one of
    * both. This limit is for the pipeline layout, not for the set layout, but
    * there is no set limit, so we just set a pipeline limit. I don't think
    * any app is going to hit this soon. */
   size_t max_descriptor_set_size =
      ((1ull << 31) - 16 * MAX_DYNAMIC_BUFFERS) /
      (32 /* uniform buffer, 32 due to potential space wasted on alignment */ +
       32 /* storage buffer, 32 due to potential space wasted on alignment */ +
       32 /* sampler, largest when combined with image */ +
       64 /* sampled image */ + 64 /* storage image */);

   const VkPhysicalDeviceLimits limits = {
      .maxImageDimension1D = (1 << 14),
      .maxImageDimension2D = (1 << 14),
      .maxImageDimension3D = (1 << 11),
      .maxImageDimensionCube = (1 << 14),
      .maxImageArrayLayers = (1 << 11),
      .maxTexelBufferElements = 128 * 1024 * 1024,
      .maxUniformBufferRange = UINT32_MAX,
      .maxStorageBufferRange = UINT32_MAX,
      .maxPushConstantsSize = MAX_PUSH_CONSTANTS_SIZE,
      .maxMemoryAllocationCount = UINT32_MAX,
      .maxSamplerAllocationCount = 64 * 1024,
      .bufferImageGranularity = 64,          /* A cache line */
      .sparseAddressSpaceSize = 0xffffffffu, /* buffer max size */
      .maxBoundDescriptorSets = MAX_SETS,
      .maxPerStageDescriptorSamplers = max_descriptor_set_size,
      .maxPerStageDescriptorUniformBuffers = max_descriptor_set_size,
      .maxPerStageDescriptorStorageBuffers = max_descriptor_set_size,
      .maxPerStageDescriptorSampledImages = max_descriptor_set_size,
      .maxPerStageDescriptorStorageImages = max_descriptor_set_size,
      .maxPerStageDescriptorInputAttachments = max_descriptor_set_size,
      .maxPerStageResources = max_descriptor_set_size,
      .maxDescriptorSetSamplers = max_descriptor_set_size,
      .maxDescriptorSetUniformBuffers = max_descriptor_set_size,
      .maxDescriptorSetUniformBuffersDynamic = MAX_DYNAMIC_UNIFORM_BUFFERS,
      .maxDescriptorSetStorageBuffers = max_descriptor_set_size,
      .maxDescriptorSetStorageBuffersDynamic = MAX_DYNAMIC_STORAGE_BUFFERS,
      .maxDescriptorSetSampledImages = max_descriptor_set_size,
      .maxDescriptorSetStorageImages = max_descriptor_set_size,
      .maxDescriptorSetInputAttachments = max_descriptor_set_size,
      .maxVertexInputAttributes = 32,
      .maxVertexInputBindings = 32,
      .maxVertexInputAttributeOffset = 2047,
      .maxVertexInputBindingStride = 2048,
      .maxVertexOutputComponents = 128,
      .maxTessellationGenerationLevel = 64,
      .maxTessellationPatchSize = 32,
      .maxTessellationControlPerVertexInputComponents = 128,
      .maxTessellationControlPerVertexOutputComponents = 128,
      .maxTessellationControlPerPatchOutputComponents = 120,
      .maxTessellationControlTotalOutputComponents = 4096,
      .maxTessellationEvaluationInputComponents = 128,
      .maxTessellationEvaluationOutputComponents = 128,
      .maxGeometryShaderInvocations = 127,
      .maxGeometryInputComponents = 64,
      .maxGeometryOutputComponents = 128,
      .maxGeometryOutputVertices = 256,
      .maxGeometryTotalOutputComponents = 1024,
      .maxFragmentInputComponents = 128,
      .maxFragmentOutputAttachments = 8,
      .maxFragmentDualSrcAttachments = 1,
      .maxFragmentCombinedOutputResources =
         MAX_RTS + max_descriptor_set_size * 2,
      .maxComputeSharedMemorySize = 32768,
      .maxComputeWorkGroupCount = {65535, 65535, 65535},
      .maxComputeWorkGroupInvocations = 2048,
      .maxComputeWorkGroupSize = {2048, 2048, 2048},
      .subPixelPrecisionBits = 4 /* FIXME */,
      .subTexelPrecisionBits = 4 /* FIXME */,
      .mipmapPrecisionBits = 4 /* FIXME */,
      .maxDrawIndexedIndexValue = UINT32_MAX,
      .maxDrawIndirectCount = UINT32_MAX,
      .maxSamplerLodBias = 16,
      .maxSamplerAnisotropy = 16,
      .maxViewports = MAX_VIEWPORTS,
      .maxViewportDimensions = {(1 << 14), (1 << 14)},
      .viewportBoundsRange = {INT16_MIN, INT16_MAX},
      .viewportSubPixelBits = 8,
      .minMemoryMapAlignment = 4096, /* A page */
      .minTexelBufferOffsetAlignment = 64,
      .minUniformBufferOffsetAlignment = 16,
      .minStorageBufferOffsetAlignment = 4,
      .minTexelOffset = -32,
      .maxTexelOffset = 31,
      .minTexelGatherOffset = -32,
      .maxTexelGatherOffset = 31,
      .minInterpolationOffset = -2,
      .maxInterpolationOffset = 2,
      .subPixelInterpolationOffsetBits = 8,
      .maxFramebufferWidth = (1 << 14),
      .maxFramebufferHeight = (1 << 14),
      .maxFramebufferLayers = (1 << 10),
      .framebufferColorSampleCounts = sample_counts,
      .framebufferDepthSampleCounts = sample_counts,
      .framebufferStencilSampleCounts = sample_counts,
      .framebufferNoAttachmentsSampleCounts = sample_counts,
      .maxColorAttachments = MAX_RTS,
      .sampledImageColorSampleCounts = sample_counts,
      .sampledImageIntegerSampleCounts = VK_SAMPLE_COUNT_1_BIT,
      .sampledImageDepthSampleCounts = sample_counts,
      .sampledImageStencilSampleCounts = sample_counts,
      .storageImageSampleCounts = VK_SAMPLE_COUNT_1_BIT,
      .maxSampleMaskWords = 1,
      .timestampComputeAndGraphics = true,
      .timestampPeriod = 1,
      .maxClipDistances = 8,
      .maxCullDistances = 8,
      .maxCombinedClipAndCullDistances = 8,
      .discreteQueuePriorities = 1,
      .pointSizeRange = {0.125, 4095.9375},
      .lineWidthRange = {0.0, 7.9921875},
      .pointSizeGranularity = (1.0 / 16.0),
      .lineWidthGranularity = (1.0 / 128.0),
      .strictLines = false, /* FINISHME */
      .standardSampleLocations = true,
      .optimalBufferCopyOffsetAlignment = 128,
      .optimalBufferCopyRowPitchAlignment = 128,
      .nonCoherentAtomSize = 64,
   };

   pProperties->properties = (VkPhysicalDeviceProperties){
      .apiVersion = PANVK_API_VERSION,
      .driverVersion = vk_get_driver_version(),
      .vendorID = 0, /* TODO */
      .deviceID = 0,
      .deviceType = VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU,
      .limits = limits,
      .sparseProperties = {0},
   };

   strcpy(pProperties->properties.deviceName, pdevice->name);
   memcpy(pProperties->properties.pipelineCacheUUID, pdevice->cache_uuid,
          VK_UUID_SIZE);

   VkPhysicalDeviceVulkan11Properties core_1_1 = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES,
      .deviceLUIDValid = false,
      .pointClippingBehavior = VK_POINT_CLIPPING_BEHAVIOR_ALL_CLIP_PLANES,
      .maxMultiviewViewCount = 0,
      .maxMultiviewInstanceIndex = 0,
      .protectedNoFault = false,
      /* Make sure everything is addressable by a signed 32-bit int, and
       * our largest descriptors are 96 bytes. */
      .maxPerSetDescriptors = (1ull << 31) / 96,
      /* Our buffer size fields allow only this much */
      .maxMemoryAllocationSize = 0xFFFFFFFFull,
   };
   memcpy(core_1_1.driverUUID, pdevice->driver_uuid, VK_UUID_SIZE);
   memcpy(core_1_1.deviceUUID, pdevice->device_uuid, VK_UUID_SIZE);

   const VkPhysicalDeviceVulkan12Properties core_1_2 = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES,
   };

   const VkPhysicalDeviceVulkan13Properties core_1_3 = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES,
   };

   vk_foreach_struct(ext, pProperties->pNext) {
      if (vk_get_physical_device_core_1_1_property_ext(ext, &core_1_1))
         continue;
      if (vk_get_physical_device_core_1_2_property_ext(ext, &core_1_2))
         continue;
      if (vk_get_physical_device_core_1_3_property_ext(ext, &core_1_3))
         continue;

      switch (ext->sType) {
      case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PUSH_DESCRIPTOR_PROPERTIES_KHR: {
         VkPhysicalDevicePushDescriptorPropertiesKHR *properties =
            (VkPhysicalDevicePushDescriptorPropertiesKHR *)ext;
         properties->maxPushDescriptors = MAX_PUSH_DESCRIPTORS;
         break;
      }
      case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_PROPERTIES_EXT: {
         VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT *properties =
            (VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT *)ext;
         /* We have to restrict this a bit for multiview */
         properties->maxVertexAttribDivisor = UINT32_MAX / (16 * 2048);
         break;
      }
      default:
         break;
      }
   }
}

static const VkQueueFamilyProperties panvk_queue_family_properties = {
   .queueFlags =
      VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT,
   .queueCount = 1,
   .timestampValidBits = 64,
   .minImageTransferGranularity = {1, 1, 1},
};

void
panvk_GetPhysicalDeviceQueueFamilyProperties2(
   VkPhysicalDevice physicalDevice, uint32_t *pQueueFamilyPropertyCount,
   VkQueueFamilyProperties2 *pQueueFamilyProperties)
{
   VK_OUTARRAY_MAKE_TYPED(VkQueueFamilyProperties2, out, pQueueFamilyProperties,
                          pQueueFamilyPropertyCount);

   vk_outarray_append_typed(VkQueueFamilyProperties2, &out, p)
   {
      p->queueFamilyProperties = panvk_queue_family_properties;
   }
}

static uint64_t
panvk_get_system_heap_size()
{
   struct sysinfo info;
   sysinfo(&info);

   uint64_t total_ram = (uint64_t)info.totalram * info.mem_unit;

   /* We don't want to burn too much ram with the GPU.  If the user has 4GiB
    * or less, we use at most half.  If they have more than 4GiB, we use 3/4.
    */
   uint64_t available_ram;
   if (total_ram <= 4ull * 1024 * 1024 * 1024)
      available_ram = total_ram / 2;
   else
      available_ram = total_ram * 3 / 4;

   return available_ram;
}

void
panvk_GetPhysicalDeviceMemoryProperties2(
   VkPhysicalDevice physicalDevice,
   VkPhysicalDeviceMemoryProperties2 *pMemoryProperties)
{
   pMemoryProperties->memoryProperties = (VkPhysicalDeviceMemoryProperties){
      .memoryHeapCount = 1,
      .memoryHeaps[0].size = panvk_get_system_heap_size(),
      .memoryHeaps[0].flags = VK_MEMORY_HEAP_DEVICE_LOCAL_BIT,
      .memoryTypeCount = 1,
      .memoryTypes[0].propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
                                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                      VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      .memoryTypes[0].heapIndex = 0,
   };
}

static VkResult
panvk_queue_init(struct panvk_device *device, struct panvk_queue *queue,
                 int idx, const VkDeviceQueueCreateInfo *create_info)
{
   const struct panfrost_device *pdev = &device->physical_device->pdev;

   VkResult result = vk_queue_init(&queue->vk, &device->vk, create_info, idx);
   if (result != VK_SUCCESS)
      return result;
   queue->device = device;

   struct drm_syncobj_create create = {
      .flags = DRM_SYNCOBJ_CREATE_SIGNALED,
   };

   int ret =
      drmIoctl(panfrost_device_fd(pdev), DRM_IOCTL_SYNCOBJ_CREATE, &create);
   if (ret) {
      vk_queue_finish(&queue->vk);
      return VK_ERROR_OUT_OF_HOST_MEMORY;
   }

   switch (pdev->arch) {
   case 6:
      queue->vk.driver_submit = panvk_v6_queue_submit;
      break;
   case 7:
      queue->vk.driver_submit = panvk_v7_queue_submit;
      break;
   default:
      unreachable("Unsupported architecture");
   }

   queue->sync = create.handle;
   return VK_SUCCESS;
}

static void
panvk_queue_finish(struct panvk_queue *queue)
{
   vk_queue_finish(&queue->vk);
}

VkResult
panvk_CreateDevice(VkPhysicalDevice physicalDevice,
                   const VkDeviceCreateInfo *pCreateInfo,
                   const VkAllocationCallbacks *pAllocator, VkDevice *pDevice)
{
   VK_FROM_HANDLE(panvk_physical_device, physical_device, physicalDevice);
   VkResult result;
   struct panvk_device *device;

   device = vk_zalloc2(&physical_device->instance->vk.alloc, pAllocator,
                       sizeof(*device), 8, VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
   if (!device)
      return vk_error(physical_device, VK_ERROR_OUT_OF_HOST_MEMORY);

   const struct vk_device_entrypoint_table *dev_entrypoints;
   const struct vk_command_buffer_ops *cmd_buffer_ops;
   struct vk_device_dispatch_table dispatch_table;

   switch (physical_device->pdev.arch) {
   case 6:
      dev_entrypoints = &panvk_v6_device_entrypoints;
      cmd_buffer_ops = &panvk_v6_cmd_buffer_ops;
      break;
   case 7:
      dev_entrypoints = &panvk_v7_device_entrypoints;
      cmd_buffer_ops = &panvk_v7_cmd_buffer_ops;
      break;
   default:
      unreachable("Unsupported architecture");
   }

   /* For secondary command buffer support, overwrite any command entrypoints
    * in the main device-level dispatch table with
    * vk_cmd_enqueue_unless_primary_Cmd*.
    */
   vk_device_dispatch_table_from_entrypoints(
      &dispatch_table, &vk_cmd_enqueue_unless_primary_device_entrypoints, true);

   vk_device_dispatch_table_from_entrypoints(&dispatch_table, dev_entrypoints,
                                             false);
   vk_device_dispatch_table_from_entrypoints(&dispatch_table,
                                             &panvk_device_entrypoints, false);
   vk_device_dispatch_table_from_entrypoints(&dispatch_table,
                                             &wsi_device_entrypoints, false);

   /* Populate our primary cmd_dispatch table. */
   vk_device_dispatch_table_from_entrypoints(&device->cmd_dispatch,
                                             dev_entrypoints, true);
   vk_device_dispatch_table_from_entrypoints(&device->cmd_dispatch,
                                             &panvk_device_entrypoints, false);
   vk_device_dispatch_table_from_entrypoints(
      &device->cmd_dispatch, &vk_common_device_entrypoints, false);

   result = vk_device_init(&device->vk, &physical_device->vk, &dispatch_table,
                           pCreateInfo, pAllocator);
   if (result != VK_SUCCESS) {
      vk_free(&device->vk.alloc, device);
      return result;
   }

   /* Must be done after vk_device_init() because this function memset(0) the
    * whole struct.
    */
   device->vk.command_dispatch_table = &device->cmd_dispatch;
   device->vk.command_buffer_ops = cmd_buffer_ops;

   device->instance = physical_device->instance;
   device->physical_device = physical_device;

   const struct panfrost_device *pdev = &physical_device->pdev;
   vk_device_set_drm_fd(&device->vk, panfrost_device_fd(pdev));

   for (unsigned i = 0; i < pCreateInfo->queueCreateInfoCount; i++) {
      const VkDeviceQueueCreateInfo *queue_create =
         &pCreateInfo->pQueueCreateInfos[i];
      uint32_t qfi = queue_create->queueFamilyIndex;
      device->queues[qfi] =
         vk_alloc(&device->vk.alloc,
                  queue_create->queueCount * sizeof(struct panvk_queue), 8,
                  VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
      if (!device->queues[qfi]) {
         result = VK_ERROR_OUT_OF_HOST_MEMORY;
         goto fail;
      }

      memset(device->queues[qfi], 0,
             queue_create->queueCount * sizeof(struct panvk_queue));

      device->queue_count[qfi] = queue_create->queueCount;

      for (unsigned q = 0; q < queue_create->queueCount; q++) {
         result =
            panvk_queue_init(device, &device->queues[qfi][q], q, queue_create);
         if (result != VK_SUCCESS)
            goto fail;
      }
   }

   *pDevice = panvk_device_to_handle(device);
   return VK_SUCCESS;

fail:
   for (unsigned i = 0; i < PANVK_MAX_QUEUE_FAMILIES; i++) {
      for (unsigned q = 0; q < device->queue_count[i]; q++)
         panvk_queue_finish(&device->queues[i][q]);
      if (device->queue_count[i])
         vk_object_free(&device->vk, NULL, device->queues[i]);
   }

   vk_free(&device->vk.alloc, device);
   return result;
}

void
panvk_DestroyDevice(VkDevice _device, const VkAllocationCallbacks *pAllocator)
{
   VK_FROM_HANDLE(panvk_device, device, _device);

   if (!device)
      return;

   for (unsigned i = 0; i < PANVK_MAX_QUEUE_FAMILIES; i++) {
      for (unsigned q = 0; q < device->queue_count[i]; q++)
         panvk_queue_finish(&device->queues[i][q]);
      if (device->queue_count[i])
         vk_object_free(&device->vk, NULL, device->queues[i]);
   }

   vk_free(&device->vk.alloc, device);
}

VkResult
panvk_EnumerateInstanceLayerProperties(uint32_t *pPropertyCount,
                                       VkLayerProperties *pProperties)
{
   *pPropertyCount = 0;
   return VK_SUCCESS;
}

VkResult
panvk_QueueWaitIdle(VkQueue _queue)
{
   VK_FROM_HANDLE(panvk_queue, queue, _queue);

   if (panvk_device_is_lost(queue->device))
      return VK_ERROR_DEVICE_LOST;

   const struct panfrost_device *pdev = &queue->device->physical_device->pdev;
   struct drm_syncobj_wait wait = {
      .handles = (uint64_t)(uintptr_t)(&queue->sync),
      .count_handles = 1,
      .timeout_nsec = INT64_MAX,
      .flags = DRM_SYNCOBJ_WAIT_FLAGS_WAIT_ALL,
   };
   int ret;

   ret = drmIoctl(panfrost_device_fd(pdev), DRM_IOCTL_SYNCOBJ_WAIT, &wait);
   assert(!ret);

   return VK_SUCCESS;
}

VkResult
panvk_EnumerateInstanceExtensionProperties(const char *pLayerName,
                                           uint32_t *pPropertyCount,
                                           VkExtensionProperties *pProperties)
{
   if (pLayerName)
      return vk_error(NULL, VK_ERROR_LAYER_NOT_PRESENT);

   return vk_enumerate_instance_extension_properties(
      &panvk_instance_extensions, pPropertyCount, pProperties);
}

PFN_vkVoidFunction
panvk_GetInstanceProcAddr(VkInstance _instance, const char *pName)
{
   VK_FROM_HANDLE(panvk_instance, instance, _instance);
   return vk_instance_get_proc_addr(&instance->vk, &panvk_instance_entrypoints,
                                    pName);
}

/* The loader wants us to expose a second GetInstanceProcAddr function
 * to work around certain LD_PRELOAD issues seen in apps.
 */
PUBLIC
VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
vk_icdGetInstanceProcAddr(VkInstance instance, const char *pName)
{
   return panvk_GetInstanceProcAddr(instance, pName);
}

VkResult
panvk_AllocateMemory(VkDevice _device,
                     const VkMemoryAllocateInfo *pAllocateInfo,
                     const VkAllocationCallbacks *pAllocator,
                     VkDeviceMemory *pMem)
{
   VK_FROM_HANDLE(panvk_device, device, _device);
   struct panvk_device_memory *mem;
   bool can_be_exported = false;

   assert(pAllocateInfo->sType == VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);

   if (pAllocateInfo->allocationSize == 0) {
      /* Apparently, this is allowed */
      *pMem = VK_NULL_HANDLE;
      return VK_SUCCESS;
   }

   const VkExportMemoryAllocateInfo *export_info =
      vk_find_struct_const(pAllocateInfo->pNext, EXPORT_MEMORY_ALLOCATE_INFO);

   if (export_info) {
      if (export_info->handleTypes &
          ~(VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT |
            VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT))
         return vk_error(device, VK_ERROR_INVALID_EXTERNAL_HANDLE);
      else if (export_info->handleTypes)
         can_be_exported = true;
   }

   mem = vk_object_alloc(&device->vk, pAllocator, sizeof(*mem),
                         VK_OBJECT_TYPE_DEVICE_MEMORY);
   if (mem == NULL)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   const VkImportMemoryFdInfoKHR *fd_info =
      vk_find_struct_const(pAllocateInfo->pNext, IMPORT_MEMORY_FD_INFO_KHR);

   if (fd_info && !fd_info->handleType)
      fd_info = NULL;

   if (fd_info) {
      assert(
         fd_info->handleType == VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT ||
         fd_info->handleType == VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT);

      /*
       * TODO Importing the same fd twice gives us the same handle without
       * reference counting.  We need to maintain a per-instance handle-to-bo
       * table and add reference count to panvk_bo.
       */
      mem->bo = panfrost_bo_import(&device->physical_device->pdev, fd_info->fd);
      /* take ownership and close the fd */
      close(fd_info->fd);
   } else {
      mem->bo = panfrost_bo_create(
         &device->physical_device->pdev, pAllocateInfo->allocationSize,
         can_be_exported ? PAN_BO_SHAREABLE : 0, "User-requested memory");
   }

   assert(mem->bo);

   *pMem = panvk_device_memory_to_handle(mem);

   return VK_SUCCESS;
}

void
panvk_FreeMemory(VkDevice _device, VkDeviceMemory _mem,
                 const VkAllocationCallbacks *pAllocator)
{
   VK_FROM_HANDLE(panvk_device, device, _device);
   VK_FROM_HANDLE(panvk_device_memory, mem, _mem);

   if (mem == NULL)
      return;

   panfrost_bo_unreference(mem->bo);
   vk_object_free(&device->vk, pAllocator, mem);
}

VkResult
panvk_MapMemory(VkDevice _device, VkDeviceMemory _memory, VkDeviceSize offset,
                VkDeviceSize size, VkMemoryMapFlags flags, void **ppData)
{
   VK_FROM_HANDLE(panvk_device, device, _device);
   VK_FROM_HANDLE(panvk_device_memory, mem, _memory);

   if (mem == NULL) {
      *ppData = NULL;
      return VK_SUCCESS;
   }

   if (!mem->bo->ptr.cpu)
      panfrost_bo_mmap(mem->bo);

   *ppData = mem->bo->ptr.cpu;

   if (*ppData) {
      *ppData += offset;
      return VK_SUCCESS;
   }

   return vk_error(device, VK_ERROR_MEMORY_MAP_FAILED);
}

void
panvk_UnmapMemory(VkDevice _device, VkDeviceMemory _memory)
{
}

VkResult
panvk_FlushMappedMemoryRanges(VkDevice _device, uint32_t memoryRangeCount,
                              const VkMappedMemoryRange *pMemoryRanges)
{
   return VK_SUCCESS;
}

VkResult
panvk_InvalidateMappedMemoryRanges(VkDevice _device, uint32_t memoryRangeCount,
                                   const VkMappedMemoryRange *pMemoryRanges)
{
   return VK_SUCCESS;
}

void
panvk_GetBufferMemoryRequirements2(VkDevice device,
                                   const VkBufferMemoryRequirementsInfo2 *pInfo,
                                   VkMemoryRequirements2 *pMemoryRequirements)
{
   VK_FROM_HANDLE(panvk_buffer, buffer, pInfo->buffer);

   const uint64_t alignment = 64;
   const uint64_t size = align64(buffer->vk.size, alignment);

   pMemoryRequirements->memoryRequirements.memoryTypeBits = 1;
   pMemoryRequirements->memoryRequirements.alignment = alignment;
   pMemoryRequirements->memoryRequirements.size = size;
}

void
panvk_GetImageMemoryRequirements2(VkDevice device,
                                  const VkImageMemoryRequirementsInfo2 *pInfo,
                                  VkMemoryRequirements2 *pMemoryRequirements)
{
   VK_FROM_HANDLE(panvk_image, image, pInfo->image);

   const uint64_t alignment = 4096;
   const uint64_t size = panvk_image_get_total_size(image);

   pMemoryRequirements->memoryRequirements.memoryTypeBits = 1;
   pMemoryRequirements->memoryRequirements.alignment = alignment;
   pMemoryRequirements->memoryRequirements.size = size;
}

void
panvk_GetImageSparseMemoryRequirements2(
   VkDevice device, const VkImageSparseMemoryRequirementsInfo2 *pInfo,
   uint32_t *pSparseMemoryRequirementCount,
   VkSparseImageMemoryRequirements2 *pSparseMemoryRequirements)
{
   panvk_stub();
}

void
panvk_GetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory,
                                VkDeviceSize *pCommittedMemoryInBytes)
{
   *pCommittedMemoryInBytes = 0;
}

VkResult
panvk_BindBufferMemory2(VkDevice device, uint32_t bindInfoCount,
                        const VkBindBufferMemoryInfo *pBindInfos)
{
   for (uint32_t i = 0; i < bindInfoCount; ++i) {
      VK_FROM_HANDLE(panvk_device_memory, mem, pBindInfos[i].memory);
      VK_FROM_HANDLE(panvk_buffer, buffer, pBindInfos[i].buffer);

      if (mem) {
         buffer->bo = mem->bo;
         buffer->bo_offset = pBindInfos[i].memoryOffset;
      } else {
         buffer->bo = NULL;
      }
   }
   return VK_SUCCESS;
}

VkResult
panvk_BindImageMemory2(VkDevice device, uint32_t bindInfoCount,
                       const VkBindImageMemoryInfo *pBindInfos)
{
   for (uint32_t i = 0; i < bindInfoCount; ++i) {
      VK_FROM_HANDLE(panvk_image, image, pBindInfos[i].image);
      VK_FROM_HANDLE(panvk_device_memory, mem, pBindInfos[i].memory);

      if (mem) {
         image->pimage.data.bo = mem->bo;
         image->pimage.data.offset = pBindInfos[i].memoryOffset;
         /* Reset the AFBC headers */
         if (drm_is_afbc(image->pimage.layout.modifier)) {
            void *base =
               image->pimage.data.bo->ptr.cpu + image->pimage.data.offset;

            for (unsigned layer = 0; layer < image->pimage.layout.array_size;
                 layer++) {
               for (unsigned level = 0; level < image->pimage.layout.nr_slices;
                    level++) {
                  void *header = base +
                                 (layer * image->pimage.layout.array_stride) +
                                 image->pimage.layout.slices[level].offset;
                  memset(header, 0,
                         image->pimage.layout.slices[level].afbc.header_size);
               }
            }
         }
      } else {
         image->pimage.data.bo = NULL;
         image->pimage.data.offset = pBindInfos[i].memoryOffset;
      }
   }

   return VK_SUCCESS;
}

VkResult
panvk_CreateEvent(VkDevice _device, const VkEventCreateInfo *pCreateInfo,
                  const VkAllocationCallbacks *pAllocator, VkEvent *pEvent)
{
   VK_FROM_HANDLE(panvk_device, device, _device);
   const struct panfrost_device *pdev = &device->physical_device->pdev;
   struct panvk_event *event = vk_object_zalloc(
      &device->vk, pAllocator, sizeof(*event), VK_OBJECT_TYPE_EVENT);
   if (!event)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   struct drm_syncobj_create create = {
      .flags = 0,
   };

   int ret =
      drmIoctl(panfrost_device_fd(pdev), DRM_IOCTL_SYNCOBJ_CREATE, &create);
   if (ret)
      return VK_ERROR_OUT_OF_HOST_MEMORY;

   event->syncobj = create.handle;
   *pEvent = panvk_event_to_handle(event);

   return VK_SUCCESS;
}

void
panvk_DestroyEvent(VkDevice _device, VkEvent _event,
                   const VkAllocationCallbacks *pAllocator)
{
   VK_FROM_HANDLE(panvk_device, device, _device);
   VK_FROM_HANDLE(panvk_event, event, _event);
   const struct panfrost_device *pdev = &device->physical_device->pdev;

   if (!event)
      return;

   struct drm_syncobj_destroy destroy = {.handle = event->syncobj};
   drmIoctl(panfrost_device_fd(pdev), DRM_IOCTL_SYNCOBJ_DESTROY, &destroy);

   vk_object_free(&device->vk, pAllocator, event);
}

VkResult
panvk_GetEventStatus(VkDevice _device, VkEvent _event)
{
   VK_FROM_HANDLE(panvk_device, device, _device);
   VK_FROM_HANDLE(panvk_event, event, _event);
   const struct panfrost_device *pdev = &device->physical_device->pdev;
   bool signaled;

   struct drm_syncobj_wait wait = {
      .handles = (uintptr_t)&event->syncobj,
      .count_handles = 1,
      .timeout_nsec = 0,
      .flags = DRM_SYNCOBJ_WAIT_FLAGS_WAIT_FOR_SUBMIT,
   };

   int ret = drmIoctl(panfrost_device_fd(pdev), DRM_IOCTL_SYNCOBJ_WAIT, &wait);
   if (ret) {
      if (errno == ETIME)
         signaled = false;
      else {
         assert(0);
         return VK_ERROR_DEVICE_LOST; /* TODO */
      }
   } else
      signaled = true;

   return signaled ? VK_EVENT_SET : VK_EVENT_RESET;
}

VkResult
panvk_SetEvent(VkDevice _device, VkEvent _event)
{
   VK_FROM_HANDLE(panvk_device, device, _device);
   VK_FROM_HANDLE(panvk_event, event, _event);
   const struct panfrost_device *pdev = &device->physical_device->pdev;

   struct drm_syncobj_array objs = {
      .handles = (uint64_t)(uintptr_t)&event->syncobj,
      .count_handles = 1};

   /* This is going to just replace the fence for this syncobj with one that
    * is already in signaled state. This won't be a problem because the spec
    * mandates that the event will have been set before the vkCmdWaitEvents
    * command executes.
    * https://www.khronos.org/registry/vulkan/specs/1.2/html/chap6.html#commandbuffers-submission-progress
    */
   if (drmIoctl(panfrost_device_fd(pdev), DRM_IOCTL_SYNCOBJ_SIGNAL, &objs))
      return VK_ERROR_DEVICE_LOST;

   return VK_SUCCESS;
}

VkResult
panvk_ResetEvent(VkDevice _device, VkEvent _event)
{
   VK_FROM_HANDLE(panvk_device, device, _device);
   VK_FROM_HANDLE(panvk_event, event, _event);
   const struct panfrost_device *pdev = &device->physical_device->pdev;

   struct drm_syncobj_array objs = {
      .handles = (uint64_t)(uintptr_t)&event->syncobj,
      .count_handles = 1};

   if (drmIoctl(panfrost_device_fd(pdev), DRM_IOCTL_SYNCOBJ_RESET, &objs))
      return VK_ERROR_DEVICE_LOST;

   return VK_SUCCESS;
}

VkResult
panvk_CreateBuffer(VkDevice _device, const VkBufferCreateInfo *pCreateInfo,
                   const VkAllocationCallbacks *pAllocator, VkBuffer *pBuffer)
{
   VK_FROM_HANDLE(panvk_device, device, _device);
   struct panvk_buffer *buffer;

   assert(pCreateInfo->sType == VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO);

   buffer =
      vk_buffer_create(&device->vk, pCreateInfo, pAllocator, sizeof(*buffer));
   if (buffer == NULL)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   *pBuffer = panvk_buffer_to_handle(buffer);

   return VK_SUCCESS;
}

void
panvk_DestroyBuffer(VkDevice _device, VkBuffer _buffer,
                    const VkAllocationCallbacks *pAllocator)
{
   VK_FROM_HANDLE(panvk_device, device, _device);
   VK_FROM_HANDLE(panvk_buffer, buffer, _buffer);

   if (!buffer)
      return;

   vk_buffer_destroy(&device->vk, pAllocator, &buffer->vk);
}

VkResult
panvk_CreateFramebuffer(VkDevice _device,
                        const VkFramebufferCreateInfo *pCreateInfo,
                        const VkAllocationCallbacks *pAllocator,
                        VkFramebuffer *pFramebuffer)
{
   VK_FROM_HANDLE(panvk_device, device, _device);
   struct panvk_framebuffer *framebuffer;

   assert(pCreateInfo->sType == VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO);

   size_t size = sizeof(*framebuffer) + sizeof(struct panvk_attachment_info) *
                                           pCreateInfo->attachmentCount;
   framebuffer = vk_object_alloc(&device->vk, pAllocator, size,
                                 VK_OBJECT_TYPE_FRAMEBUFFER);
   if (framebuffer == NULL)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   framebuffer->attachment_count = pCreateInfo->attachmentCount;
   framebuffer->width = pCreateInfo->width;
   framebuffer->height = pCreateInfo->height;
   framebuffer->layers = pCreateInfo->layers;
   for (uint32_t i = 0; i < pCreateInfo->attachmentCount; i++) {
      VkImageView _iview = pCreateInfo->pAttachments[i];
      struct panvk_image_view *iview = panvk_image_view_from_handle(_iview);
      framebuffer->attachments[i].iview = iview;
   }

   *pFramebuffer = panvk_framebuffer_to_handle(framebuffer);
   return VK_SUCCESS;
}

void
panvk_DestroyFramebuffer(VkDevice _device, VkFramebuffer _fb,
                         const VkAllocationCallbacks *pAllocator)
{
   VK_FROM_HANDLE(panvk_device, device, _device);
   VK_FROM_HANDLE(panvk_framebuffer, fb, _fb);

   if (fb)
      vk_object_free(&device->vk, pAllocator, fb);
}

void
panvk_DestroySampler(VkDevice _device, VkSampler _sampler,
                     const VkAllocationCallbacks *pAllocator)
{
   VK_FROM_HANDLE(panvk_device, device, _device);
   VK_FROM_HANDLE(panvk_sampler, sampler, _sampler);

   if (!sampler)
      return;

   vk_object_free(&device->vk, pAllocator, sampler);
}

VkResult
panvk_GetMemoryFdKHR(VkDevice _device, const VkMemoryGetFdInfoKHR *pGetFdInfo,
                     int *pFd)
{
   VK_FROM_HANDLE(panvk_device, device, _device);
   VK_FROM_HANDLE(panvk_device_memory, memory, pGetFdInfo->memory);

   assert(pGetFdInfo->sType == VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR);

   /* At the moment, we support only the below handle types. */
   assert(
      pGetFdInfo->handleType == VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT ||
      pGetFdInfo->handleType == VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT);

   int prime_fd = panfrost_bo_export(memory->bo);
   if (prime_fd < 0)
      return vk_error(device, VK_ERROR_OUT_OF_DEVICE_MEMORY);

   *pFd = prime_fd;
   return VK_SUCCESS;
}

VkResult
panvk_GetMemoryFdPropertiesKHR(VkDevice _device,
                               VkExternalMemoryHandleTypeFlagBits handleType,
                               int fd,
                               VkMemoryFdPropertiesKHR *pMemoryFdProperties)
{
   assert(handleType == VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT);
   pMemoryFdProperties->memoryTypeBits = 1;
   return VK_SUCCESS;
}

void
panvk_GetPhysicalDeviceExternalSemaphoreProperties(
   VkPhysicalDevice physicalDevice,
   const VkPhysicalDeviceExternalSemaphoreInfo *pExternalSemaphoreInfo,
   VkExternalSemaphoreProperties *pExternalSemaphoreProperties)
{
   if ((pExternalSemaphoreInfo->handleType ==
           VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT ||
        pExternalSemaphoreInfo->handleType ==
           VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT)) {
      pExternalSemaphoreProperties->exportFromImportedHandleTypes =
         VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT |
         VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT;
      pExternalSemaphoreProperties->compatibleHandleTypes =
         VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT |
         VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT;
      pExternalSemaphoreProperties->externalSemaphoreFeatures =
         VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT |
         VK_EXTERNAL_SEMAPHORE_FEATURE_IMPORTABLE_BIT;
   } else {
      pExternalSemaphoreProperties->exportFromImportedHandleTypes = 0;
      pExternalSemaphoreProperties->compatibleHandleTypes = 0;
      pExternalSemaphoreProperties->externalSemaphoreFeatures = 0;
   }
}

void
panvk_GetPhysicalDeviceExternalFenceProperties(
   VkPhysicalDevice physicalDevice,
   const VkPhysicalDeviceExternalFenceInfo *pExternalFenceInfo,
   VkExternalFenceProperties *pExternalFenceProperties)
{
   pExternalFenceProperties->exportFromImportedHandleTypes = 0;
   pExternalFenceProperties->compatibleHandleTypes = 0;
   pExternalFenceProperties->externalFenceFeatures = 0;
}
