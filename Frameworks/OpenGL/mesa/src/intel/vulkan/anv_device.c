/*
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>
#ifdef MAJOR_IN_MKDEV
#include <sys/mkdev.h>
#endif
#ifdef MAJOR_IN_SYSMACROS
#include <sys/sysmacros.h>
#endif
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "drm-uapi/drm_fourcc.h"
#include "drm-uapi/drm.h"
#include <xf86drm.h>

#include "anv_private.h"
#include "anv_measure.h"
#include "util/u_debug.h"
#include "util/build_id.h"
#include "util/disk_cache.h"
#include "util/mesa-sha1.h"
#include "util/os_file.h"
#include "util/os_misc.h"
#include "util/u_atomic.h"
#ifdef ANDROID
#include "util/u_gralloc/u_gralloc.h"
#endif
#include "util/u_string.h"
#include "util/driconf.h"
#include "git_sha1.h"
#include "vk_common_entrypoints.h"
#include "vk_util.h"
#include "vk_deferred_operation.h"
#include "vk_drm_syncobj.h"
#include "common/intel_aux_map.h"
#include "common/intel_uuid.h"
#include "common/i915/intel_gem.h"
#include "perf/intel_perf.h"

#include "i915/anv_device.h"
#include "xe/anv_device.h"
#include "xe/anv_queue.h"

#include "genxml/gen7_pack.h"
#include "genxml/genX_bits.h"

static const driOptionDescription anv_dri_options[] = {
   DRI_CONF_SECTION_PERFORMANCE
      DRI_CONF_ADAPTIVE_SYNC(true)
      DRI_CONF_VK_X11_OVERRIDE_MIN_IMAGE_COUNT(0)
      DRI_CONF_VK_X11_STRICT_IMAGE_COUNT(false)
      DRI_CONF_VK_KHR_PRESENT_WAIT(false)
      DRI_CONF_VK_XWAYLAND_WAIT_READY(true)
      DRI_CONF_ANV_ASSUME_FULL_SUBGROUPS(0)
      DRI_CONF_ANV_DISABLE_FCV(false)
      DRI_CONF_ANV_SAMPLE_MASK_OUT_OPENGL_BEHAVIOUR(false)
      DRI_CONF_ANV_FORCE_FILTER_ADDR_ROUNDING(false)
      DRI_CONF_ANV_FP64_WORKAROUND_ENABLED(false)
      DRI_CONF_ANV_GENERATED_INDIRECT_THRESHOLD(4)
      DRI_CONF_ANV_GENERATED_INDIRECT_RING_THRESHOLD(100)
      DRI_CONF_NO_16BIT(false)
      DRI_CONF_INTEL_ENABLE_WA_14018912822(false)
      DRI_CONF_ANV_QUERY_CLEAR_WITH_BLORP_THRESHOLD(6)
      DRI_CONF_ANV_QUERY_COPY_WITH_SHADER_THRESHOLD(6)
      DRI_CONF_ANV_FORCE_INDIRECT_DESCRIPTORS(false)
      DRI_CONF_SHADER_SPILLING_RATE(0)
      DRI_CONF_OPT_B(intel_tbimr, true, "Enable TBIMR tiled rendering")
   DRI_CONF_SECTION_END

   DRI_CONF_SECTION_DEBUG
      DRI_CONF_ALWAYS_FLUSH_CACHE(false)
      DRI_CONF_VK_WSI_FORCE_BGRA8_UNORM_FIRST(false)
      DRI_CONF_VK_WSI_FORCE_SWAPCHAIN_TO_CURRENT_EXTENT(false)
      DRI_CONF_LIMIT_TRIG_INPUT_RANGE(false)
      DRI_CONF_ANV_MESH_CONV_PRIM_ATTRS_TO_VERT_ATTRS(-2)
      DRI_CONF_FORCE_VK_VENDOR(0)
      DRI_CONF_FAKE_SPARSE(false)
#if defined(ANDROID) && ANDROID_API_LEVEL >= 34
      DRI_CONF_VK_REQUIRE_ASTC(true)
#else
      DRI_CONF_VK_REQUIRE_ASTC(false)
#endif
   DRI_CONF_SECTION_END

   DRI_CONF_SECTION_QUALITY
      DRI_CONF_PP_LOWER_DEPTH_RANGE_RATE()
   DRI_CONF_SECTION_END
};

/* This is probably far to big but it reflects the max size used for messages
 * in OpenGLs KHR_debug.
 */
#define MAX_DEBUG_MESSAGE_LENGTH    4096

/* The "RAW" clocks on Linux are called "FAST" on FreeBSD */
#if !defined(CLOCK_MONOTONIC_RAW) && defined(CLOCK_MONOTONIC_FAST)
#define CLOCK_MONOTONIC_RAW CLOCK_MONOTONIC_FAST
#endif

static void
compiler_debug_log(void *data, UNUSED unsigned *id, const char *fmt, ...)
{
   char str[MAX_DEBUG_MESSAGE_LENGTH];
   struct anv_device *device = (struct anv_device *)data;
   UNUSED struct anv_instance *instance = device->physical->instance;

   va_list args;
   va_start(args, fmt);
   (void) vsnprintf(str, MAX_DEBUG_MESSAGE_LENGTH, fmt, args);
   va_end(args);

   //vk_logd(VK_LOG_NO_OBJS(&instance->vk), "%s", str);
}

static void
compiler_perf_log(UNUSED void *data, UNUSED unsigned *id, const char *fmt, ...)
{
   va_list args;
   va_start(args, fmt);

   if (INTEL_DEBUG(DEBUG_PERF))
      mesa_logd_v(fmt, args);

   va_end(args);
}

#if defined(VK_USE_PLATFORM_WAYLAND_KHR) || \
    defined(VK_USE_PLATFORM_XCB_KHR) || \
    defined(VK_USE_PLATFORM_XLIB_KHR) || \
    defined(VK_USE_PLATFORM_DISPLAY_KHR)
#define ANV_USE_WSI_PLATFORM
#endif

#ifdef ANDROID_STRICT
#if ANDROID_API_LEVEL >= 33
#define ANV_API_VERSION VK_MAKE_VERSION(1, 3, VK_HEADER_VERSION)
#else
#define ANV_API_VERSION VK_MAKE_VERSION(1, 1, VK_HEADER_VERSION)
#endif
#else
#define ANV_API_VERSION VK_MAKE_VERSION(1, 3, VK_HEADER_VERSION)
#endif

VkResult anv_EnumerateInstanceVersion(
    uint32_t*                                   pApiVersion)
{
    *pApiVersion = ANV_API_VERSION;
    return VK_SUCCESS;
}

static const struct vk_instance_extension_table instance_extensions = {
   .KHR_device_group_creation                = true,
   .KHR_external_fence_capabilities          = true,
   .KHR_external_memory_capabilities         = true,
   .KHR_external_semaphore_capabilities      = true,
   .KHR_get_physical_device_properties2      = true,
   .EXT_debug_report                         = true,
   .EXT_debug_utils                          = true,

#ifdef ANV_USE_WSI_PLATFORM
   .KHR_get_surface_capabilities2            = true,
   .KHR_surface                              = true,
   .KHR_surface_protected_capabilities       = true,
   .EXT_swapchain_colorspace                 = true,
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
   .KHR_wayland_surface                      = true,
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
   .KHR_xcb_surface                          = true,
#endif
#ifdef VK_USE_PLATFORM_XLIB_KHR
   .KHR_xlib_surface                         = true,
#endif
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
   .EXT_acquire_xlib_display                 = true,
#endif
#ifdef VK_USE_PLATFORM_DISPLAY_KHR
   .KHR_display                              = true,
   .KHR_get_display_properties2              = true,
   .EXT_direct_mode_display                  = true,
   .EXT_display_surface_counter              = true,
   .EXT_acquire_drm_display                  = true,
#endif
};

static void
get_device_extensions(const struct anv_physical_device *device,
                      struct vk_device_extension_table *ext)
{
   const bool has_syncobj_wait =
      (device->sync_syncobj_type.features & VK_SYNC_FEATURE_CPU_WAIT) != 0;

   const bool rt_enabled = ANV_SUPPORT_RT && device->info.has_ray_tracing;

   *ext = (struct vk_device_extension_table) {
      .KHR_8bit_storage                      = true,
      .KHR_16bit_storage                     = !device->instance->no_16bit,
      .KHR_acceleration_structure            = rt_enabled,
      .KHR_bind_memory2                      = true,
      .KHR_buffer_device_address             = true,
      .KHR_calibrated_timestamps             = device->has_reg_timestamp,
      .KHR_copy_commands2                    = true,
      .KHR_cooperative_matrix                = anv_has_cooperative_matrix(device),
      .KHR_create_renderpass2                = true,
      .KHR_dedicated_allocation              = true,
      .KHR_deferred_host_operations          = true,
      .KHR_depth_stencil_resolve             = true,
      .KHR_descriptor_update_template        = true,
      .KHR_device_group                      = true,
      .KHR_draw_indirect_count               = true,
      .KHR_driver_properties                 = true,
      .KHR_dynamic_rendering                 = true,
      .KHR_external_fence                    = has_syncobj_wait,
      .KHR_external_fence_fd                 = has_syncobj_wait,
      .KHR_external_memory                   = true,
      .KHR_external_memory_fd                = true,
      .KHR_external_semaphore                = true,
      .KHR_external_semaphore_fd             = true,
      .KHR_format_feature_flags2             = true,
      .KHR_fragment_shading_rate             = device->info.ver >= 11,
      .KHR_get_memory_requirements2          = true,
      .KHR_global_priority                   = device->max_context_priority >=
                                               VK_QUEUE_GLOBAL_PRIORITY_MEDIUM_KHR,
      .KHR_image_format_list                 = true,
      .KHR_imageless_framebuffer             = true,
#ifdef ANV_USE_WSI_PLATFORM
      .KHR_incremental_present               = true,
#endif
      .KHR_maintenance1                      = true,
      .KHR_maintenance2                      = true,
      .KHR_maintenance3                      = true,
      .KHR_maintenance4                      = true,
      .KHR_maintenance5                      = true,
      .KHR_maintenance6                      = true,
      .KHR_map_memory2                       = true,
      .KHR_multiview                         = true,
      .KHR_performance_query =
         device->perf &&
         (device->perf->i915_perf_version >= 3 ||
          INTEL_DEBUG(DEBUG_NO_OACONFIG)) &&
         device->use_call_secondary,
      .KHR_pipeline_executable_properties    = true,
      .KHR_pipeline_library                  = true,
      /* Hide these behind dri configs for now since we cannot implement it reliably on
       * all surfaces yet. There is no surface capability query for present wait/id,
       * but the feature is useful enough to hide behind an opt-in mechanism for now.
       * If the instance only enables surface extensions that unconditionally support present wait,
       * we can also expose the extension that way. */
      .KHR_present_id =
         driQueryOptionb(&device->instance->dri_options, "vk_khr_present_wait") ||
         wsi_common_vk_instance_supports_present_wait(&device->instance->vk),
      .KHR_present_wait =
         driQueryOptionb(&device->instance->dri_options, "vk_khr_present_wait") ||
         wsi_common_vk_instance_supports_present_wait(&device->instance->vk),
      .KHR_push_descriptor                   = true,
      .KHR_ray_query                         = rt_enabled,
      .KHR_ray_tracing_maintenance1          = rt_enabled,
      .KHR_ray_tracing_pipeline              = rt_enabled,
      .KHR_ray_tracing_position_fetch        = rt_enabled,
      .KHR_relaxed_block_layout              = true,
      .KHR_sampler_mirror_clamp_to_edge      = true,
      .KHR_sampler_ycbcr_conversion          = true,
      .KHR_separate_depth_stencil_layouts    = true,
      .KHR_shader_atomic_int64               = true,
      .KHR_shader_clock                      = true,
      .KHR_shader_draw_parameters            = true,
      .KHR_shader_float16_int8               = !device->instance->no_16bit,
      .KHR_shader_float_controls             = true,
      .KHR_shader_integer_dot_product        = true,
      .KHR_shader_non_semantic_info          = true,
      .KHR_shader_subgroup_extended_types    = true,
      .KHR_shader_subgroup_uniform_control_flow = true,
      .KHR_shader_terminate_invocation       = true,
      .KHR_spirv_1_4                         = true,
      .KHR_storage_buffer_storage_class      = true,
#ifdef ANV_USE_WSI_PLATFORM
      .KHR_swapchain                         = true,
      .KHR_swapchain_mutable_format          = true,
#endif
      .KHR_synchronization2                  = true,
      .KHR_timeline_semaphore                = true,
      .KHR_uniform_buffer_standard_layout    = true,
      .KHR_variable_pointers                 = true,
      .KHR_vertex_attribute_divisor          = true,
      .KHR_video_queue                       = device->video_decode_enabled,
      .KHR_video_decode_queue                = device->video_decode_enabled,
      .KHR_video_decode_h264                 = VIDEO_CODEC_H264DEC && device->video_decode_enabled,
      .KHR_video_decode_h265                 = VIDEO_CODEC_H265DEC && device->video_decode_enabled,
      .KHR_vulkan_memory_model               = true,
      .KHR_workgroup_memory_explicit_layout  = true,
      .KHR_zero_initialize_workgroup_memory  = true,
      .EXT_4444_formats                      = true,
      .EXT_border_color_swizzle              = true,
      .EXT_buffer_device_address             = true,
      .EXT_calibrated_timestamps             = device->has_reg_timestamp,
      .EXT_color_write_enable                = true,
      .EXT_conditional_rendering             = true,
      .EXT_conservative_rasterization        = true,
      .EXT_custom_border_color               = true,
      .EXT_depth_bias_control                = true,
      .EXT_depth_clamp_zero_one              = true,
      .EXT_depth_clip_control                = true,
      .EXT_depth_range_unrestricted          = device->info.ver >= 20,
      .EXT_depth_clip_enable                 = true,
      .EXT_descriptor_indexing               = true,
#ifdef VK_USE_PLATFORM_DISPLAY_KHR
      .EXT_display_control                   = true,
#endif
      .EXT_dynamic_rendering_unused_attachments = true,
      .EXT_extended_dynamic_state            = true,
      .EXT_extended_dynamic_state2           = true,
      .EXT_extended_dynamic_state3           = true,
      .EXT_external_memory_dma_buf           = true,
      .EXT_external_memory_host              = true,
      .EXT_fragment_shader_interlock         = true,
      .EXT_global_priority                   = device->max_context_priority >=
                                               VK_QUEUE_GLOBAL_PRIORITY_MEDIUM_KHR,
      .EXT_global_priority_query             = device->max_context_priority >=
                                               VK_QUEUE_GLOBAL_PRIORITY_MEDIUM_KHR,
      .EXT_graphics_pipeline_library         = !debug_get_bool_option("ANV_NO_GPL", false),
      .EXT_host_query_reset                  = true,
      .EXT_image_2d_view_of_3d               = true,
      .EXT_image_robustness                  = true,
      .EXT_image_drm_format_modifier         = true,
      .EXT_image_sliced_view_of_3d           = true,
      .EXT_image_view_min_lod                = true,
      .EXT_index_type_uint8                  = true,
      .EXT_inline_uniform_block              = true,
      .EXT_line_rasterization                = true,
      .EXT_load_store_op_none                = true,
      /* Enable the extension only if we have support on both the local &
       * system memory
       */
      .EXT_memory_budget                     = (!device->info.has_local_mem ||
                                                device->vram_mappable.available > 0) &&
                                               device->sys.available,
      .EXT_mesh_shader                       = device->info.has_mesh_shading,
      .EXT_mutable_descriptor_type           = true,
      .EXT_nested_command_buffer             = true,
      .EXT_non_seamless_cube_map             = true,
      .EXT_pci_bus_info                      = true,
      .EXT_physical_device_drm               = true,
      .EXT_pipeline_creation_cache_control   = true,
      .EXT_pipeline_creation_feedback        = true,
      .EXT_pipeline_library_group_handles    = rt_enabled,
      .EXT_pipeline_robustness               = true,
      .EXT_post_depth_coverage               = true,
      .EXT_primitives_generated_query        = true,
      .EXT_primitive_topology_list_restart   = true,
      .EXT_private_data                      = true,
      .EXT_provoking_vertex                  = true,
      .EXT_queue_family_foreign              = true,
      .EXT_robustness2                       = true,
      .EXT_sample_locations                  = true,
      .EXT_sampler_filter_minmax             = true,
      .EXT_scalar_block_layout               = true,
      .EXT_separate_stencil_usage            = true,
      .EXT_shader_atomic_float               = true,
      .EXT_shader_atomic_float2              = true,
      .EXT_shader_demote_to_helper_invocation = true,
      .EXT_shader_module_identifier          = true,
      .EXT_shader_stencil_export             = true,
      .EXT_shader_subgroup_ballot            = true,
      .EXT_shader_subgroup_vote              = true,
      .EXT_shader_viewport_index_layer       = true,
      .EXT_subgroup_size_control             = true,
      .EXT_texel_buffer_alignment            = true,
      .EXT_tooling_info                      = true,
      .EXT_transform_feedback                = true,
      .EXT_vertex_attribute_divisor          = true,
      .EXT_vertex_input_dynamic_state        = true,
      .EXT_ycbcr_image_arrays                = true,
      .AMD_buffer_marker                     = true,
#ifdef ANDROID
      .ANDROID_external_memory_android_hardware_buffer = true,
      .ANDROID_native_buffer                 = true,
#endif
      .GOOGLE_decorate_string                = true,
      .GOOGLE_hlsl_functionality1            = true,
      .GOOGLE_user_type                      = true,
      .INTEL_performance_query               = device->perf &&
                                               device->perf->i915_perf_version >= 3,
      .INTEL_shader_integer_functions2       = true,
      .EXT_multi_draw                        = true,
      .NV_compute_shader_derivatives         = true,
      .VALVE_mutable_descriptor_type         = true,
   };
}

static void
get_features(const struct anv_physical_device *pdevice,
             struct vk_features *features)
{
   struct vk_app_info *app_info = &pdevice->instance->vk.app_info;

   const bool rt_enabled = ANV_SUPPORT_RT && pdevice->info.has_ray_tracing;

   const bool mesh_shader =
      pdevice->vk.supported_extensions.EXT_mesh_shader;

   const bool has_sparse_or_fake = pdevice->instance->has_fake_sparse ||
                                   pdevice->has_sparse;

   *features = (struct vk_features) {
      /* Vulkan 1.0 */
      .robustBufferAccess                       = true,
      .fullDrawIndexUint32                      = true,
      .imageCubeArray                           = true,
      .independentBlend                         = true,
      .geometryShader                           = true,
      .tessellationShader                       = true,
      .sampleRateShading                        = true,
      .dualSrcBlend                             = true,
      .logicOp                                  = true,
      .multiDrawIndirect                        = true,
      .drawIndirectFirstInstance                = true,
      .depthClamp                               = true,
      .depthBiasClamp                           = true,
      .fillModeNonSolid                         = true,
      .depthBounds                              = pdevice->info.ver >= 12,
      .wideLines                                = true,
      .largePoints                              = true,
      .alphaToOne                               = true,
      .multiViewport                            = true,
      .samplerAnisotropy                        = true,
      .textureCompressionETC2                   = true,
      .textureCompressionASTC_LDR               = pdevice->has_astc_ldr ||
                                                  pdevice->emu_astc_ldr,
      .textureCompressionBC                     = true,
      .occlusionQueryPrecise                    = true,
      .pipelineStatisticsQuery                  = true,
      /* We can't do image stores in vec4 shaders */
      .vertexPipelineStoresAndAtomics =
         pdevice->compiler->scalar_stage[MESA_SHADER_VERTEX] &&
         pdevice->compiler->scalar_stage[MESA_SHADER_GEOMETRY],
      .fragmentStoresAndAtomics                 = true,
      .shaderTessellationAndGeometryPointSize   = true,
      .shaderImageGatherExtended                = true,
      .shaderStorageImageExtendedFormats        = true,
      .shaderStorageImageMultisample            = false,
      /* Gfx12.5 has all the required format supported in HW for typed
       * read/writes
       */
      .shaderStorageImageReadWithoutFormat      = pdevice->info.verx10 >= 125,
      .shaderStorageImageWriteWithoutFormat     = true,
      .shaderUniformBufferArrayDynamicIndexing  = true,
      .shaderSampledImageArrayDynamicIndexing   = true,
      .shaderStorageBufferArrayDynamicIndexing  = true,
      .shaderStorageImageArrayDynamicIndexing   = true,
      .shaderClipDistance                       = true,
      .shaderCullDistance                       = true,
      .shaderFloat64                            = pdevice->info.has_64bit_float,
      .shaderInt64                              = true,
      .shaderInt16                              = true,
      .shaderResourceMinLod                     = true,
      .shaderResourceResidency                  = has_sparse_or_fake,
      .sparseBinding                            = has_sparse_or_fake,
      .sparseResidencyAliased                   = has_sparse_or_fake,
      .sparseResidencyBuffer                    = has_sparse_or_fake,
      .sparseResidencyImage2D                   = has_sparse_or_fake,
      .sparseResidencyImage3D                   = has_sparse_or_fake,
      .sparseResidency2Samples                  = false,
      .sparseResidency4Samples                  = false,
      .sparseResidency8Samples                  = false,
      .sparseResidency16Samples                 = false,
      .variableMultisampleRate                  = true,
      .inheritedQueries                         = true,

      /* Vulkan 1.1 */
      .storageBuffer16BitAccess            = !pdevice->instance->no_16bit,
      .uniformAndStorageBuffer16BitAccess  = !pdevice->instance->no_16bit,
      .storagePushConstant16               = true,
      .storageInputOutput16                = false,
      .multiview                           = true,
      .multiviewGeometryShader             = true,
      .multiviewTessellationShader         = true,
      .variablePointersStorageBuffer       = true,
      .variablePointers                    = true,
      .protectedMemory                     = pdevice->has_protected_contexts,
      .samplerYcbcrConversion              = true,
      .shaderDrawParameters                = true,

      /* Vulkan 1.2 */
      .samplerMirrorClampToEdge            = true,
      .drawIndirectCount                   = true,
      .storageBuffer8BitAccess             = true,
      .uniformAndStorageBuffer8BitAccess   = true,
      .storagePushConstant8                = true,
      .shaderBufferInt64Atomics            = true,
      .shaderSharedInt64Atomics            = false,
      .shaderFloat16                       = !pdevice->instance->no_16bit,
      .shaderInt8                          = !pdevice->instance->no_16bit,

      .descriptorIndexing                                 = true,
      .shaderInputAttachmentArrayDynamicIndexing          = false,
      .shaderUniformTexelBufferArrayDynamicIndexing       = true,
      .shaderStorageTexelBufferArrayDynamicIndexing       = true,
      .shaderUniformBufferArrayNonUniformIndexing         = true,
      .shaderSampledImageArrayNonUniformIndexing          = true,
      .shaderStorageBufferArrayNonUniformIndexing         = true,
      .shaderStorageImageArrayNonUniformIndexing          = true,
      .shaderInputAttachmentArrayNonUniformIndexing       = false,
      .shaderUniformTexelBufferArrayNonUniformIndexing    = true,
      .shaderStorageTexelBufferArrayNonUniformIndexing    = true,
      .descriptorBindingUniformBufferUpdateAfterBind      = true,
      .descriptorBindingSampledImageUpdateAfterBind       = true,
      .descriptorBindingStorageImageUpdateAfterBind       = true,
      .descriptorBindingStorageBufferUpdateAfterBind      = true,
      .descriptorBindingUniformTexelBufferUpdateAfterBind = true,
      .descriptorBindingStorageTexelBufferUpdateAfterBind = true,
      .descriptorBindingUpdateUnusedWhilePending          = true,
      .descriptorBindingPartiallyBound                    = true,
      .descriptorBindingVariableDescriptorCount           = true,
      .runtimeDescriptorArray                             = true,

      .samplerFilterMinmax                 = true,
      .scalarBlockLayout                   = true,
      .imagelessFramebuffer                = true,
      .uniformBufferStandardLayout         = true,
      .shaderSubgroupExtendedTypes         = true,
      .separateDepthStencilLayouts         = true,
      .hostQueryReset                      = true,
      .timelineSemaphore                   = true,
      .bufferDeviceAddress                 = true,
      .bufferDeviceAddressCaptureReplay    = true,
      .bufferDeviceAddressMultiDevice      = false,
      .vulkanMemoryModel                   = true,
      .vulkanMemoryModelDeviceScope        = true,
      .vulkanMemoryModelAvailabilityVisibilityChains = true,
      .shaderOutputViewportIndex           = true,
      .shaderOutputLayer                   = true,
      .subgroupBroadcastDynamicId          = true,

      /* Vulkan 1.3 */
      .robustImageAccess = true,
      .inlineUniformBlock = true,
      .descriptorBindingInlineUniformBlockUpdateAfterBind = true,
      .pipelineCreationCacheControl = true,
      .privateData = true,
      .shaderDemoteToHelperInvocation = true,
      .shaderTerminateInvocation = true,
      .subgroupSizeControl = true,
      .computeFullSubgroups = true,
      .synchronization2 = true,
      .textureCompressionASTC_HDR = false,
      .shaderZeroInitializeWorkgroupMemory = true,
      .dynamicRendering = true,
      .shaderIntegerDotProduct = true,
      .maintenance4 = true,

      /* VK_EXT_4444_formats */
      .formatA4R4G4B4 = true,
      .formatA4B4G4R4 = false,

      /* VK_KHR_acceleration_structure */
      .accelerationStructure = rt_enabled,
      .accelerationStructureCaptureReplay = false, /* TODO */
      .accelerationStructureIndirectBuild = false, /* TODO */
      .accelerationStructureHostCommands = false,
      .descriptorBindingAccelerationStructureUpdateAfterBind = rt_enabled,

      /* VK_EXT_border_color_swizzle */
      .borderColorSwizzle = true,
      .borderColorSwizzleFromImage = true,

      /* VK_EXT_color_write_enable */
      .colorWriteEnable = true,

      /* VK_EXT_image_2d_view_of_3d  */
      .image2DViewOf3D = true,
      .sampler2DViewOf3D = true,

      /* VK_EXT_image_sliced_view_of_3d */
      .imageSlicedViewOf3D = true,

      /* VK_NV_compute_shader_derivatives */
      .computeDerivativeGroupQuads = true,
      .computeDerivativeGroupLinear = true,

      /* VK_EXT_conditional_rendering */
      .conditionalRendering = true,
      .inheritedConditionalRendering = true,

      /* VK_EXT_custom_border_color */
      .customBorderColors = true,
      .customBorderColorWithoutFormat = true,

      /* VK_EXT_depth_clamp_zero_one */
      .depthClampZeroOne = true,

      /* VK_EXT_depth_clip_enable */
      .depthClipEnable = true,

      /* VK_EXT_fragment_shader_interlock */
      .fragmentShaderSampleInterlock = true,
      .fragmentShaderPixelInterlock = true,
      .fragmentShaderShadingRateInterlock = false,

      /* VK_EXT_global_priority_query */
      .globalPriorityQuery = true,

      /* VK_EXT_graphics_pipeline_library */
      .graphicsPipelineLibrary =
         pdevice->vk.supported_extensions.EXT_graphics_pipeline_library,

      /* VK_KHR_fragment_shading_rate */
      .pipelineFragmentShadingRate = true,
      .primitiveFragmentShadingRate =
         pdevice->info.has_coarse_pixel_primitive_and_cb,
      .attachmentFragmentShadingRate =
         pdevice->info.has_coarse_pixel_primitive_and_cb,

      /* VK_EXT_image_view_min_lod */
      .minLod = true,

      /* VK_EXT_index_type_uint8 */
      .indexTypeUint8 = true,

      /* VK_EXT_line_rasterization */
      /* Rectangular lines must use the strict algorithm, which is not
       * supported for wide lines prior to ICL.  See rasterization_mode for
       * details and how the HW states are programmed.
       */
      .rectangularLines = pdevice->info.ver >= 10,
      .bresenhamLines = true,
      /* Support for Smooth lines with MSAA was removed on gfx11.  From the
       * BSpec section "Multisample ModesState" table for "AA Line Support
       * Requirements":
       *
       *    GFX10:BUG:######## 	NUM_MULTISAMPLES == 1
       *
       * Fortunately, this isn't a case most people care about.
       */
      .smoothLines = pdevice->info.ver < 10,
      .stippledRectangularLines = false,
      .stippledBresenhamLines = true,
      .stippledSmoothLines = false,

      /* VK_NV_mesh_shader */
      .taskShaderNV = false,
      .meshShaderNV = false,

      /* VK_EXT_mesh_shader */
      .taskShader = mesh_shader,
      .meshShader = mesh_shader,
      .multiviewMeshShader = false,
      .primitiveFragmentShadingRateMeshShader = mesh_shader,
      .meshShaderQueries = false,

      /* VK_EXT_mutable_descriptor_type */
      .mutableDescriptorType = true,

      /* VK_KHR_performance_query */
      .performanceCounterQueryPools = true,
      /* HW only supports a single configuration at a time. */
      .performanceCounterMultipleQueryPools = false,

      /* VK_KHR_pipeline_executable_properties */
      .pipelineExecutableInfo = true,

      /* VK_EXT_primitives_generated_query */
      .primitivesGeneratedQuery = true,
      .primitivesGeneratedQueryWithRasterizerDiscard = false,
      .primitivesGeneratedQueryWithNonZeroStreams = false,

      /* VK_EXT_pipeline_library_group_handles */
      .pipelineLibraryGroupHandles = true,

      /* VK_EXT_provoking_vertex */
      .provokingVertexLast = true,
      .transformFeedbackPreservesProvokingVertex = true,

      /* VK_KHR_ray_query */
      .rayQuery = rt_enabled,

      /* VK_KHR_ray_tracing_maintenance1 */
      .rayTracingMaintenance1 = rt_enabled,
      .rayTracingPipelineTraceRaysIndirect2 = rt_enabled,

      /* VK_KHR_ray_tracing_pipeline */
      .rayTracingPipeline = rt_enabled,
      .rayTracingPipelineShaderGroupHandleCaptureReplay = false,
      .rayTracingPipelineShaderGroupHandleCaptureReplayMixed = false,
      .rayTracingPipelineTraceRaysIndirect = rt_enabled,
      .rayTraversalPrimitiveCulling = rt_enabled,

      /* VK_EXT_robustness2 */
      .robustBufferAccess2 = true,
      .robustImageAccess2 = true,
      .nullDescriptor = true,

      /* VK_EXT_shader_atomic_float */
      .shaderBufferFloat32Atomics =    true,
      .shaderBufferFloat32AtomicAdd =  pdevice->info.has_lsc,
      .shaderBufferFloat64Atomics =
         pdevice->info.has_64bit_float && pdevice->info.has_lsc,
      .shaderBufferFloat64AtomicAdd =  false,
      .shaderSharedFloat32Atomics =    true,
      .shaderSharedFloat32AtomicAdd =  false,
      .shaderSharedFloat64Atomics =    false,
      .shaderSharedFloat64AtomicAdd =  false,
      .shaderImageFloat32Atomics =     true,
      .shaderImageFloat32AtomicAdd =   false,
      .sparseImageFloat32Atomics =     false,
      .sparseImageFloat32AtomicAdd =   false,

      /* VK_EXT_shader_atomic_float2 */
      .shaderBufferFloat16Atomics      = pdevice->info.has_lsc,
      .shaderBufferFloat16AtomicAdd    = false,
      .shaderBufferFloat16AtomicMinMax = pdevice->info.has_lsc,
      .shaderBufferFloat32AtomicMinMax = true,
      .shaderBufferFloat64AtomicMinMax =
         pdevice->info.has_64bit_float && pdevice->info.has_lsc,
      .shaderSharedFloat16Atomics      = pdevice->info.has_lsc,
      .shaderSharedFloat16AtomicAdd    = false,
      .shaderSharedFloat16AtomicMinMax = pdevice->info.has_lsc,
      .shaderSharedFloat32AtomicMinMax = true,
      .shaderSharedFloat64AtomicMinMax = false,
      .shaderImageFloat32AtomicMinMax  = false,
      .sparseImageFloat32AtomicMinMax  = false,

      /* VK_KHR_shader_clock */
      .shaderSubgroupClock = true,
      .shaderDeviceClock = false,

      /* VK_INTEL_shader_integer_functions2 */
      .shaderIntegerFunctions2 = true,

      /* VK_EXT_shader_module_identifier */
      .shaderModuleIdentifier = true,

      /* VK_KHR_shader_subgroup_uniform_control_flow */
      .shaderSubgroupUniformControlFlow = true,

      /* VK_EXT_texel_buffer_alignment */
      .texelBufferAlignment = true,

      /* VK_EXT_transform_feedback */
      .transformFeedback = true,
      .geometryStreams = true,

      /* VK_KHR_vertex_attribute_divisor */
      .vertexAttributeInstanceRateDivisor = true,
      .vertexAttributeInstanceRateZeroDivisor = true,

      /* VK_KHR_workgroup_memory_explicit_layout */
      .workgroupMemoryExplicitLayout = true,
      .workgroupMemoryExplicitLayoutScalarBlockLayout = true,
      .workgroupMemoryExplicitLayout8BitAccess = true,
      .workgroupMemoryExplicitLayout16BitAccess = true,

      /* VK_EXT_ycbcr_image_arrays */
      .ycbcrImageArrays = true,

      /* VK_EXT_extended_dynamic_state */
      .extendedDynamicState = true,

      /* VK_EXT_extended_dynamic_state2 */
      .extendedDynamicState2 = true,
      .extendedDynamicState2LogicOp = true,
      .extendedDynamicState2PatchControlPoints = true,

      /* VK_EXT_extended_dynamic_state3 */
      .extendedDynamicState3PolygonMode = true,
      .extendedDynamicState3TessellationDomainOrigin = true,
      .extendedDynamicState3RasterizationStream = true,
      .extendedDynamicState3LineStippleEnable = true,
      .extendedDynamicState3LineRasterizationMode = true,
      .extendedDynamicState3LogicOpEnable = true,
      .extendedDynamicState3AlphaToOneEnable = true,
      .extendedDynamicState3DepthClipEnable = true,
      .extendedDynamicState3DepthClampEnable = true,
      .extendedDynamicState3DepthClipNegativeOneToOne = true,
      .extendedDynamicState3ProvokingVertexMode = true,
      .extendedDynamicState3ColorBlendEnable = true,
      .extendedDynamicState3ColorWriteMask = true,
      .extendedDynamicState3ColorBlendEquation = true,
      .extendedDynamicState3SampleLocationsEnable = true,
      .extendedDynamicState3SampleMask = true,
      .extendedDynamicState3ConservativeRasterizationMode = true,

      .extendedDynamicState3RasterizationSamples = false,
      .extendedDynamicState3AlphaToCoverageEnable = false,
      .extendedDynamicState3ExtraPrimitiveOverestimationSize = false,
      .extendedDynamicState3ViewportWScalingEnable = false,
      .extendedDynamicState3ViewportSwizzle = false,
      .extendedDynamicState3ShadingRateImageEnable = false,
      .extendedDynamicState3CoverageToColorEnable = false,
      .extendedDynamicState3CoverageToColorLocation = false,
      .extendedDynamicState3CoverageModulationMode = false,
      .extendedDynamicState3CoverageModulationTableEnable = false,
      .extendedDynamicState3CoverageModulationTable = false,
      .extendedDynamicState3CoverageReductionMode = false,
      .extendedDynamicState3RepresentativeFragmentTestEnable = false,
      .extendedDynamicState3ColorBlendAdvanced = false,

      /* VK_EXT_multi_draw */
      .multiDraw = true,

      /* VK_EXT_non_seamless_cube_map */
      .nonSeamlessCubeMap = true,

      /* VK_EXT_primitive_topology_list_restart */
      .primitiveTopologyListRestart = true,
      .primitiveTopologyPatchListRestart = true,

      /* VK_EXT_depth_clip_control */
      .depthClipControl = true,

      /* VK_KHR_present_id */
      .presentId = pdevice->vk.supported_extensions.KHR_present_id,

      /* VK_KHR_present_wait */
      .presentWait = pdevice->vk.supported_extensions.KHR_present_wait,

      /* VK_EXT_vertex_input_dynamic_state */
      .vertexInputDynamicState = true,

      /* VK_KHR_ray_tracing_position_fetch */
      .rayTracingPositionFetch = rt_enabled,

      /* VK_EXT_dynamic_rendering_unused_attachments */
      .dynamicRenderingUnusedAttachments = true,

      /* VK_EXT_depth_bias_control */
      .depthBiasControl = true,
      .floatRepresentation = true,
      .leastRepresentableValueForceUnormRepresentation = false,
      .depthBiasExact = true,

      /* VK_EXT_pipeline_robustness */
      .pipelineRobustness = true,

      /* VK_KHR_maintenance5 */
      .maintenance5 = true,

      /* VK_KHR_maintenance6 */
      .maintenance6 = true,

      /* VK_EXT_nested_command_buffer */
      .nestedCommandBuffer = true,
      .nestedCommandBufferRendering = true,
      .nestedCommandBufferSimultaneousUse = false,

      /* VK_KHR_cooperative_matrix */
      .cooperativeMatrix = anv_has_cooperative_matrix(pdevice),
   };

   /* The new DOOM and Wolfenstein games require depthBounds without
    * checking for it.  They seem to run fine without it so just claim it's
    * there and accept the consequences.
    */
   if (app_info->engine_name && strcmp(app_info->engine_name, "idTech") == 0)
      features->depthBounds = true;
}

#define MAX_PER_STAGE_DESCRIPTOR_UNIFORM_BUFFERS   64

#define MAX_PER_STAGE_DESCRIPTOR_INPUT_ATTACHMENTS 64
#define MAX_DESCRIPTOR_SET_INPUT_ATTACHMENTS       256

#define MAX_CUSTOM_BORDER_COLORS                   4096

static VkDeviceSize
anx_get_physical_device_max_heap_size(const struct anv_physical_device *pdevice)
{
   VkDeviceSize ret = 0;

   for (uint32_t i = 0; i < pdevice->memory.heap_count; i++) {
      if (pdevice->memory.heaps[i].size > ret)
         ret = pdevice->memory.heaps[i].size;
   }

   return ret;
}

static void
get_properties_1_1(const struct anv_physical_device *pdevice,
                   struct vk_properties *p)
{
   memcpy(p->deviceUUID, pdevice->device_uuid, VK_UUID_SIZE);
   memcpy(p->driverUUID, pdevice->driver_uuid, VK_UUID_SIZE);
   memset(p->deviceLUID, 0, VK_LUID_SIZE);
   p->deviceNodeMask = 0;
   p->deviceLUIDValid = false;

   p->subgroupSize = BRW_SUBGROUP_SIZE;
   VkShaderStageFlags scalar_stages = 0;
   for (unsigned stage = 0; stage < MESA_SHADER_STAGES; stage++) {
      if (pdevice->compiler->scalar_stage[stage])
         scalar_stages |= mesa_to_vk_shader_stage(stage);
   }
   if (pdevice->vk.supported_extensions.KHR_ray_tracing_pipeline) {
      scalar_stages |= VK_SHADER_STAGE_RAYGEN_BIT_KHR |
                       VK_SHADER_STAGE_ANY_HIT_BIT_KHR |
                       VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR |
                       VK_SHADER_STAGE_MISS_BIT_KHR |
                       VK_SHADER_STAGE_INTERSECTION_BIT_KHR |
                       VK_SHADER_STAGE_CALLABLE_BIT_KHR;
   }
   if (pdevice->vk.supported_extensions.EXT_mesh_shader) {
      scalar_stages |= VK_SHADER_STAGE_TASK_BIT_EXT |
                       VK_SHADER_STAGE_MESH_BIT_EXT;
   }
   p->subgroupSupportedStages = scalar_stages;
   p->subgroupSupportedOperations = VK_SUBGROUP_FEATURE_BASIC_BIT |
                                    VK_SUBGROUP_FEATURE_VOTE_BIT |
                                    VK_SUBGROUP_FEATURE_BALLOT_BIT |
                                    VK_SUBGROUP_FEATURE_SHUFFLE_BIT |
                                    VK_SUBGROUP_FEATURE_SHUFFLE_RELATIVE_BIT |
                                    VK_SUBGROUP_FEATURE_QUAD_BIT |
                                    VK_SUBGROUP_FEATURE_ARITHMETIC_BIT |
                                    VK_SUBGROUP_FEATURE_CLUSTERED_BIT;
   p->subgroupQuadOperationsInAllStages = true;

   p->pointClippingBehavior      = VK_POINT_CLIPPING_BEHAVIOR_USER_CLIP_PLANES_ONLY;
   p->maxMultiviewViewCount      = 16;
   p->maxMultiviewInstanceIndex  = UINT32_MAX / 16;
   /* Our protected implementation is a memory encryption mechanism, it
    * doesn't page fault.
    */
   p->protectedNoFault           = true;
   /* This value doesn't matter for us today as our per-stage descriptors are
    * the real limit.
    */
   p->maxPerSetDescriptors       = 1024;

   for (uint32_t i = 0; i < pdevice->memory.heap_count; i++) {
      p->maxMemoryAllocationSize = MAX2(p->maxMemoryAllocationSize,
                                        pdevice->memory.heaps[i].size);
   }
}

static void
get_properties_1_2(const struct anv_physical_device *pdevice,
                   struct vk_properties *p)
{
   p->driverID = VK_DRIVER_ID_INTEL_OPEN_SOURCE_MESA;
   memset(p->driverName, 0, sizeof(p->driverName));
   snprintf(p->driverName, VK_MAX_DRIVER_NAME_SIZE,
            "Intel open-source Mesa driver");
   memset(p->driverInfo, 0, sizeof(p->driverInfo));
   snprintf(p->driverInfo, VK_MAX_DRIVER_INFO_SIZE,
            "Mesa " PACKAGE_VERSION MESA_GIT_SHA1);

   p->conformanceVersion = (VkConformanceVersion) {
      .major = 1,
      .minor = 3,
      .subminor = 6,
      .patch = 0,
   };

   p->denormBehaviorIndependence =
      VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_ALL;
   p->roundingModeIndependence =
      VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_NONE;

   /* Broadwell does not support HF denorms and there are restrictions
    * other gens. According to Kabylake's PRM:
    *
    * "math - Extended Math Function
    * [...]
    * Restriction : Half-float denorms are always retained."
    */
   p->shaderDenormFlushToZeroFloat16         = false;
   p->shaderDenormPreserveFloat16            = true;
   p->shaderRoundingModeRTEFloat16           = true;
   p->shaderRoundingModeRTZFloat16           = true;
   p->shaderSignedZeroInfNanPreserveFloat16  = true;

   p->shaderDenormFlushToZeroFloat32         = true;
   p->shaderDenormPreserveFloat32            = true;
   p->shaderRoundingModeRTEFloat32           = true;
   p->shaderRoundingModeRTZFloat32           = true;
   p->shaderSignedZeroInfNanPreserveFloat32  = true;

   p->shaderDenormFlushToZeroFloat64         = true;
   p->shaderDenormPreserveFloat64            = true;
   p->shaderRoundingModeRTEFloat64           = true;
   p->shaderRoundingModeRTZFloat64           = true;
   p->shaderSignedZeroInfNanPreserveFloat64  = true;

   /* It's a bit hard to exactly map our implementation to the limits
    * described by Vulkan.  The bindless surface handle in the extended
    * message descriptors is 20 bits and it's an index into the table of
    * RENDER_SURFACE_STATE structs that starts at bindless surface base
    * address.  This means that we can have at must 1M surface states
    * allocated at any given time.  Since most image views take two
    * descriptors, this means we have a limit of about 500K image views.
    *
    * However, since we allocate surface states at vkCreateImageView time,
    * this means our limit is actually something on the order of 500K image
    * views allocated at any time.  The actual limit describe by Vulkan, on
    * the other hand, is a limit of how many you can have in a descriptor set.
    * Assuming anyone using 1M descriptors will be using the same image view
    * twice a bunch of times (or a bunch of null descriptors), we can safely
    * advertise a larger limit here.
    */
   const unsigned max_bindless_views =
      anv_physical_device_bindless_heap_size(pdevice) / ANV_SURFACE_STATE_SIZE;
   p->maxUpdateAfterBindDescriptorsInAllPools            = max_bindless_views;
   p->shaderUniformBufferArrayNonUniformIndexingNative   = false;
   p->shaderSampledImageArrayNonUniformIndexingNative    = false;
   p->shaderStorageBufferArrayNonUniformIndexingNative   = true;
   p->shaderStorageImageArrayNonUniformIndexingNative    = false;
   p->shaderInputAttachmentArrayNonUniformIndexingNative = false;
   p->robustBufferAccessUpdateAfterBind                  = true;
   p->quadDivergentImplicitLod                           = false;
   p->maxPerStageDescriptorUpdateAfterBindSamplers       = max_bindless_views;
   p->maxPerStageDescriptorUpdateAfterBindUniformBuffers = MAX_PER_STAGE_DESCRIPTOR_UNIFORM_BUFFERS;
   p->maxPerStageDescriptorUpdateAfterBindStorageBuffers = UINT32_MAX;
   p->maxPerStageDescriptorUpdateAfterBindSampledImages  = max_bindless_views;
   p->maxPerStageDescriptorUpdateAfterBindStorageImages  = max_bindless_views;
   p->maxPerStageDescriptorUpdateAfterBindInputAttachments = MAX_PER_STAGE_DESCRIPTOR_INPUT_ATTACHMENTS;
   p->maxPerStageUpdateAfterBindResources                = UINT32_MAX;
   p->maxDescriptorSetUpdateAfterBindSamplers            = max_bindless_views;
   p->maxDescriptorSetUpdateAfterBindUniformBuffers      = 6 * MAX_PER_STAGE_DESCRIPTOR_UNIFORM_BUFFERS;
   p->maxDescriptorSetUpdateAfterBindUniformBuffersDynamic = MAX_DYNAMIC_BUFFERS / 2;
   p->maxDescriptorSetUpdateAfterBindStorageBuffers      = UINT32_MAX;
   p->maxDescriptorSetUpdateAfterBindStorageBuffersDynamic = MAX_DYNAMIC_BUFFERS / 2;
   p->maxDescriptorSetUpdateAfterBindSampledImages       = max_bindless_views;
   p->maxDescriptorSetUpdateAfterBindStorageImages       = max_bindless_views;
   p->maxDescriptorSetUpdateAfterBindInputAttachments    = MAX_DESCRIPTOR_SET_INPUT_ATTACHMENTS;

   /* We support all of the depth resolve modes */
   p->supportedDepthResolveModes    = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT |
                                      VK_RESOLVE_MODE_AVERAGE_BIT |
                                      VK_RESOLVE_MODE_MIN_BIT |
                                      VK_RESOLVE_MODE_MAX_BIT;
   /* Average doesn't make sense for stencil so we don't support that */
   p->supportedStencilResolveModes  = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT |
                                      VK_RESOLVE_MODE_MIN_BIT |
                                      VK_RESOLVE_MODE_MAX_BIT;
   p->independentResolveNone  = true;
   p->independentResolve      = true;

   p->filterMinmaxSingleComponentFormats  = true;
   p->filterMinmaxImageComponentMapping   = true;

   p->maxTimelineSemaphoreValueDifference = UINT64_MAX;

   p->framebufferIntegerColorSampleCounts =
      isl_device_get_sample_counts(&pdevice->isl_dev);
}

static void
get_properties_1_3(const struct anv_physical_device *pdevice,
                   struct vk_properties *p)
{
   if (pdevice->info.ver >= 20)
      p->minSubgroupSize = 16;
   else
      p->minSubgroupSize = 8;
   p->maxSubgroupSize = 32;
   p->maxComputeWorkgroupSubgroups = pdevice->info.max_cs_workgroup_threads;
   p->requiredSubgroupSizeStages = VK_SHADER_STAGE_COMPUTE_BIT |
                                   VK_SHADER_STAGE_TASK_BIT_EXT |
                                   VK_SHADER_STAGE_MESH_BIT_EXT;

   p->maxInlineUniformBlockSize = MAX_INLINE_UNIFORM_BLOCK_SIZE;
   p->maxPerStageDescriptorInlineUniformBlocks =
      MAX_INLINE_UNIFORM_BLOCK_DESCRIPTORS;
   p->maxPerStageDescriptorUpdateAfterBindInlineUniformBlocks =
      MAX_INLINE_UNIFORM_BLOCK_DESCRIPTORS;
   p->maxDescriptorSetInlineUniformBlocks =
      MAX_INLINE_UNIFORM_BLOCK_DESCRIPTORS;
   p->maxDescriptorSetUpdateAfterBindInlineUniformBlocks =
      MAX_INLINE_UNIFORM_BLOCK_DESCRIPTORS;
   p->maxInlineUniformTotalSize = UINT16_MAX;

   p->integerDotProduct8BitUnsignedAccelerated = false;
   p->integerDotProduct8BitSignedAccelerated = false;
   p->integerDotProduct8BitMixedSignednessAccelerated = false;
   p->integerDotProduct4x8BitPackedUnsignedAccelerated = pdevice->info.ver >= 12;
   p->integerDotProduct4x8BitPackedSignedAccelerated = pdevice->info.ver >= 12;
   p->integerDotProduct4x8BitPackedMixedSignednessAccelerated = pdevice->info.ver >= 12;
   p->integerDotProduct16BitUnsignedAccelerated = false;
   p->integerDotProduct16BitSignedAccelerated = false;
   p->integerDotProduct16BitMixedSignednessAccelerated = false;
   p->integerDotProduct32BitUnsignedAccelerated = false;
   p->integerDotProduct32BitSignedAccelerated = false;
   p->integerDotProduct32BitMixedSignednessAccelerated = false;
   p->integerDotProduct64BitUnsignedAccelerated = false;
   p->integerDotProduct64BitSignedAccelerated = false;
   p->integerDotProduct64BitMixedSignednessAccelerated = false;
   p->integerDotProductAccumulatingSaturating8BitUnsignedAccelerated = false;
   p->integerDotProductAccumulatingSaturating8BitSignedAccelerated = false;
   p->integerDotProductAccumulatingSaturating8BitMixedSignednessAccelerated = false;
   p->integerDotProductAccumulatingSaturating4x8BitPackedUnsignedAccelerated = pdevice->info.ver >= 12;
   p->integerDotProductAccumulatingSaturating4x8BitPackedSignedAccelerated = pdevice->info.ver >= 12;
   p->integerDotProductAccumulatingSaturating4x8BitPackedMixedSignednessAccelerated = pdevice->info.ver >= 12;
   p->integerDotProductAccumulatingSaturating16BitUnsignedAccelerated = false;
   p->integerDotProductAccumulatingSaturating16BitSignedAccelerated = false;
   p->integerDotProductAccumulatingSaturating16BitMixedSignednessAccelerated = false;
   p->integerDotProductAccumulatingSaturating32BitUnsignedAccelerated = false;
   p->integerDotProductAccumulatingSaturating32BitSignedAccelerated = false;
   p->integerDotProductAccumulatingSaturating32BitMixedSignednessAccelerated = false;
   p->integerDotProductAccumulatingSaturating64BitUnsignedAccelerated = false;
   p->integerDotProductAccumulatingSaturating64BitSignedAccelerated = false;
   p->integerDotProductAccumulatingSaturating64BitMixedSignednessAccelerated = false;

   /* From the SKL PRM Vol. 2d, docs for RENDER_SURFACE_STATE::Surface
    * Base Address:
    *
    *    "For SURFTYPE_BUFFER non-rendertarget surfaces, this field
    *    specifies the base address of the first element of the surface,
    *    computed in software by adding the surface base address to the
    *    byte offset of the element in the buffer. The base address must
    *    be aligned to element size."
    *
    * The typed dataport messages require that things be texel aligned.
    * Otherwise, we may just load/store the wrong data or, in the worst
    * case, there may be hangs.
    */
   p->storageTexelBufferOffsetAlignmentBytes = 16;
   p->storageTexelBufferOffsetSingleTexelAlignment = true;

   /* The sampler, however, is much more forgiving and it can handle
    * arbitrary byte alignment for linear and buffer surfaces.  It's
    * hard to find a good PRM citation for this but years of empirical
    * experience demonstrate that this is true.
    */
   p->uniformTexelBufferOffsetAlignmentBytes = 1;
   p->uniformTexelBufferOffsetSingleTexelAlignment = true;

   p->maxBufferSize = pdevice->isl_dev.max_buffer_size;
}

static void
get_properties(const struct anv_physical_device *pdevice,
               struct vk_properties *props)
{

      const struct intel_device_info *devinfo = &pdevice->info;

   const uint32_t max_ssbos = UINT16_MAX;
   const uint32_t max_textures = UINT16_MAX;
   const uint32_t max_samplers = UINT16_MAX;
   const uint32_t max_images = UINT16_MAX;
   const VkDeviceSize max_heap_size = anx_get_physical_device_max_heap_size(pdevice);

   /* Claim a high per-stage limit since we have bindless. */
   const uint32_t max_per_stage = UINT32_MAX;

   const uint32_t max_workgroup_size =
      MIN2(1024, 32 * devinfo->max_cs_workgroup_threads);

   const bool has_sparse_or_fake = pdevice->instance->has_fake_sparse ||
                                   pdevice->has_sparse;

   uint64_t sparse_addr_space_size =
      !has_sparse_or_fake ? 0 :
      pdevice->sparse_uses_trtt ? pdevice->va.trtt.size :
      pdevice->va.high_heap.size;

   VkSampleCountFlags sample_counts =
      isl_device_get_sample_counts(&pdevice->isl_dev);


   *props = (struct vk_properties) {
      .apiVersion = ANV_API_VERSION,
      .driverVersion = vk_get_driver_version(),
      .vendorID = pdevice->instance->force_vk_vendor != 0 ?
                  pdevice->instance->force_vk_vendor : 0x8086,
      .deviceID = pdevice->info.pci_device_id,
      .deviceType = pdevice->info.has_local_mem ?
                    VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU :
                    VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU,

      /* Limits: */
      .maxImageDimension1D                      = (1 << 14),
      .maxImageDimension2D                      = (1 << 14),
      .maxImageDimension3D                      = (1 << 11),
      .maxImageDimensionCube                    = (1 << 14),
      .maxImageArrayLayers                      = (1 << 11),
      .maxTexelBufferElements                   = 128 * 1024 * 1024,
      .maxUniformBufferRange                    = pdevice->compiler->indirect_ubos_use_sampler ? (1u << 27) : (1u << 30),
      .maxStorageBufferRange                    = MIN3(pdevice->isl_dev.max_buffer_size, max_heap_size, UINT32_MAX),
      .maxPushConstantsSize                     = MAX_PUSH_CONSTANTS_SIZE,
      .maxMemoryAllocationCount                 = UINT32_MAX,
      .maxSamplerAllocationCount                = 64 * 1024,
      .bufferImageGranularity                   = 1,
      .sparseAddressSpaceSize                   = sparse_addr_space_size,
      .maxBoundDescriptorSets                   = MAX_SETS,
      .maxPerStageDescriptorSamplers            = max_samplers,
      .maxPerStageDescriptorUniformBuffers      = MAX_PER_STAGE_DESCRIPTOR_UNIFORM_BUFFERS,
      .maxPerStageDescriptorStorageBuffers      = max_ssbos,
      .maxPerStageDescriptorSampledImages       = max_textures,
      .maxPerStageDescriptorStorageImages       = max_images,
      .maxPerStageDescriptorInputAttachments    = MAX_PER_STAGE_DESCRIPTOR_INPUT_ATTACHMENTS,
      .maxPerStageResources                     = max_per_stage,
      .maxDescriptorSetSamplers                 = 6 * max_samplers, /* number of stages * maxPerStageDescriptorSamplers */
      .maxDescriptorSetUniformBuffers           = 6 * MAX_PER_STAGE_DESCRIPTOR_UNIFORM_BUFFERS,           /* number of stages * maxPerStageDescriptorUniformBuffers */
      .maxDescriptorSetUniformBuffersDynamic    = MAX_DYNAMIC_BUFFERS / 2,
      .maxDescriptorSetStorageBuffers           = 6 * max_ssbos,    /* number of stages * maxPerStageDescriptorStorageBuffers */
      .maxDescriptorSetStorageBuffersDynamic    = MAX_DYNAMIC_BUFFERS / 2,
      .maxDescriptorSetSampledImages            = 6 * max_textures, /* number of stages * maxPerStageDescriptorSampledImages */
      .maxDescriptorSetStorageImages            = 6 * max_images,   /* number of stages * maxPerStageDescriptorStorageImages */
      .maxDescriptorSetInputAttachments         = MAX_DESCRIPTOR_SET_INPUT_ATTACHMENTS,
      .maxVertexInputAttributes                 = MAX_VES,
      .maxVertexInputBindings                   = MAX_VBS,
      /* Broadwell PRMs: Volume 2d: Command Reference: Structures:
       *
       * VERTEX_ELEMENT_STATE::Source Element Offset: [0,2047]
       */
      .maxVertexInputAttributeOffset            = 2047,
      /* Skylake PRMs: Volume 2d: Command Reference: Structures:
       *
       * VERTEX_BUFFER_STATE::Buffer Pitch: [0,4095]
       */
      .maxVertexInputBindingStride              = 4095,
      .maxVertexOutputComponents                = 128,
      .maxTessellationGenerationLevel           = 64,
      .maxTessellationPatchSize                 = 32,
      .maxTessellationControlPerVertexInputComponents = 128,
      .maxTessellationControlPerVertexOutputComponents = 128,
      .maxTessellationControlPerPatchOutputComponents = 128,
      .maxTessellationControlTotalOutputComponents = 2048,
      .maxTessellationEvaluationInputComponents = 128,
      .maxTessellationEvaluationOutputComponents = 128,
      .maxGeometryShaderInvocations             = 32,
      .maxGeometryInputComponents               = 128,
      .maxGeometryOutputComponents              = 128,
      .maxGeometryOutputVertices                = 256,
      .maxGeometryTotalOutputComponents         = 1024,
      .maxFragmentInputComponents               = 116, /* 128 components - (PSIZ, CLIP_DIST0, CLIP_DIST1) */
      .maxFragmentOutputAttachments             = 8,
      .maxFragmentDualSrcAttachments            = 1,
      .maxFragmentCombinedOutputResources       = MAX_RTS + max_ssbos + max_images,
      .maxComputeSharedMemorySize               = 64 * 1024,
      .maxComputeWorkGroupCount                 = { 65535, 65535, 65535 },
      .maxComputeWorkGroupInvocations           = max_workgroup_size,
      .maxComputeWorkGroupSize = {
         max_workgroup_size,
         max_workgroup_size,
         max_workgroup_size,
      },
      .subPixelPrecisionBits                    = 8,
      .subTexelPrecisionBits                    = 8,
      .mipmapPrecisionBits                      = 8,
      .maxDrawIndexedIndexValue                 = UINT32_MAX,
      .maxDrawIndirectCount                     = UINT32_MAX,
      .maxSamplerLodBias                        = 16,
      .maxSamplerAnisotropy                     = 16,
      .maxViewports                             = MAX_VIEWPORTS,
      .maxViewportDimensions                    = { (1 << 14), (1 << 14) },
      .viewportBoundsRange                      = { INT16_MIN, INT16_MAX },
      .viewportSubPixelBits                     = 13, /* We take a float? */
      .minMemoryMapAlignment                    = 4096, /* A page */
      /* The dataport requires texel alignment so we need to assume a worst
       * case of R32G32B32A32 which is 16 bytes.
       */
      .minTexelBufferOffsetAlignment            = 16,
      .minUniformBufferOffsetAlignment          = ANV_UBO_ALIGNMENT,
      .minStorageBufferOffsetAlignment          = ANV_SSBO_ALIGNMENT,
      .minTexelOffset                           = -8,
      .maxTexelOffset                           = 7,
      .minTexelGatherOffset                     = -32,
      .maxTexelGatherOffset                     = 31,
      .minInterpolationOffset                   = -0.5,
      .maxInterpolationOffset                   = 0.4375,
      .subPixelInterpolationOffsetBits          = 4,
      .maxFramebufferWidth                      = (1 << 14),
      .maxFramebufferHeight                     = (1 << 14),
      .maxFramebufferLayers                     = (1 << 11),
      .framebufferColorSampleCounts             = sample_counts,
      .framebufferDepthSampleCounts             = sample_counts,
      .framebufferStencilSampleCounts           = sample_counts,
      .framebufferNoAttachmentsSampleCounts     = sample_counts,
      .maxColorAttachments                      = MAX_RTS,
      .sampledImageColorSampleCounts            = sample_counts,
      .sampledImageIntegerSampleCounts          = sample_counts,
      .sampledImageDepthSampleCounts            = sample_counts,
      .sampledImageStencilSampleCounts          = sample_counts,
      .storageImageSampleCounts                 = VK_SAMPLE_COUNT_1_BIT,
      .maxSampleMaskWords                       = 1,
      .timestampComputeAndGraphics              = true,
      .timestampPeriod                          = 1000000000.0 / devinfo->timestamp_frequency,
      .maxClipDistances                         = 8,
      .maxCullDistances                         = 8,
      .maxCombinedClipAndCullDistances          = 8,
      .discreteQueuePriorities                  = 2,
      .pointSizeRange                           = { 0.125, 255.875 },
      /* While SKL and up support much wider lines than we are setting here,
       * in practice we run into conformance issues if we go past this limit.
       * Since the Windows driver does the same, it's probably fair to assume
       * that no one needs more than this.
       */
      .lineWidthRange                           = { 0.0, 8.0 },
      .pointSizeGranularity                     = (1.0 / 8.0),
      .lineWidthGranularity                     = (1.0 / 128.0),
      .strictLines                              = false,
      .standardSampleLocations                  = true,
      .optimalBufferCopyOffsetAlignment         = 128,
      .optimalBufferCopyRowPitchAlignment       = 128,
      .nonCoherentAtomSize                      = 64,

      /* Sparse: */
      .sparseResidencyStandard2DBlockShape = has_sparse_or_fake,
      .sparseResidencyStandard2DMultisampleBlockShape = false,
      .sparseResidencyStandard3DBlockShape = has_sparse_or_fake,
      .sparseResidencyAlignedMipSize = false,
      .sparseResidencyNonResidentStrict = has_sparse_or_fake,

      /* VK_KHR_cooperative_matrix */
      .cooperativeMatrixSupportedStages = VK_SHADER_STAGE_COMPUTE_BIT,
   };

   snprintf(props->deviceName, sizeof(props->deviceName),
            "%s", pdevice->info.name);
   memcpy(props->pipelineCacheUUID,
          pdevice->pipeline_cache_uuid, VK_UUID_SIZE);

   get_properties_1_1(pdevice, props);
   get_properties_1_2(pdevice, props);
   get_properties_1_3(pdevice, props);

   /* VK_KHR_acceleration_structure */
   {
      props->maxGeometryCount = (1u << 24) - 1;
      props->maxInstanceCount = (1u << 24) - 1;
      props->maxPrimitiveCount = (1u << 29) - 1;
      props->maxPerStageDescriptorAccelerationStructures = UINT16_MAX;
      props->maxPerStageDescriptorUpdateAfterBindAccelerationStructures = UINT16_MAX;
      props->maxDescriptorSetAccelerationStructures = UINT16_MAX;
      props->maxDescriptorSetUpdateAfterBindAccelerationStructures = UINT16_MAX;
      props->minAccelerationStructureScratchOffsetAlignment = 64;
   }

   /* VK_KHR_fragment_shading_rate */
   {
      props->primitiveFragmentShadingRateWithMultipleViewports =
         pdevice->info.has_coarse_pixel_primitive_and_cb;
      props->layeredShadingRateAttachments =
      pdevice->info.has_coarse_pixel_primitive_and_cb;
      props->fragmentShadingRateNonTrivialCombinerOps =
         pdevice->info.has_coarse_pixel_primitive_and_cb;
      props->maxFragmentSize = (VkExtent2D) { 4, 4 };
      props->maxFragmentSizeAspectRatio =
         pdevice->info.has_coarse_pixel_primitive_and_cb ?
         2 : 4;
      props->maxFragmentShadingRateCoverageSamples = 4 * 4 *
         (pdevice->info.has_coarse_pixel_primitive_and_cb ? 4 : 16);
      props->maxFragmentShadingRateRasterizationSamples =
      pdevice->info.has_coarse_pixel_primitive_and_cb ?
         VK_SAMPLE_COUNT_4_BIT :  VK_SAMPLE_COUNT_16_BIT;
      props->fragmentShadingRateWithShaderDepthStencilWrites = false;
      props->fragmentShadingRateWithSampleMask = true;
      props->fragmentShadingRateWithShaderSampleMask = false;
      props->fragmentShadingRateWithConservativeRasterization = true;
      props->fragmentShadingRateWithFragmentShaderInterlock = true;
      props->fragmentShadingRateWithCustomSampleLocations = true;
      props->fragmentShadingRateStrictMultiplyCombiner = true;

      if (pdevice->info.has_coarse_pixel_primitive_and_cb) {
         props->minFragmentShadingRateAttachmentTexelSize = (VkExtent2D) { 8, 8 };
         props->maxFragmentShadingRateAttachmentTexelSize = (VkExtent2D) { 8, 8 };
         props->maxFragmentShadingRateAttachmentTexelSizeAspectRatio = 1;
      } else {
         /* Those must be 0 if attachmentFragmentShadingRate is not supported. */
         props->minFragmentShadingRateAttachmentTexelSize = (VkExtent2D) { 0, 0 };
         props->maxFragmentShadingRateAttachmentTexelSize = (VkExtent2D) { 0, 0 };
         props->maxFragmentShadingRateAttachmentTexelSizeAspectRatio = 0;
      }
   }

   /* VK_KHR_maintenance5 */
   {
      props->earlyFragmentMultisampleCoverageAfterSampleCounting = false;
      props->earlyFragmentSampleMaskTestBeforeSampleCounting = false;
      props->depthStencilSwizzleOneSupport = true;
      props->polygonModePointSize = true;
      props->nonStrictSinglePixelWideLinesUseParallelogram = false;
      props->nonStrictWideLinesUseParallelogram = false;
   }

   /* VK_KHR_maintenance6 */
   {
      props->blockTexelViewCompatibleMultipleLayers = true;
      props->maxCombinedImageSamplerDescriptorCount = 3;
      props->fragmentShadingRateClampCombinerInputs = true;
   }

   /* VK_KHR_performance_query */
   {
      props->allowCommandBufferQueryCopies = false;
   }

   /* VK_KHR_push_descriptor */
   {
      props->maxPushDescriptors = MAX_PUSH_DESCRIPTORS;
   }

   /* VK_KHR_ray_tracing_pipeline */
   {
      /* TODO */
      props->shaderGroupHandleSize = 32;
      props->maxRayRecursionDepth = 31;
      /* MemRay::hitGroupSRStride is 16 bits */
      props->maxShaderGroupStride = UINT16_MAX;
      /* MemRay::hitGroupSRBasePtr requires 16B alignment */
      props->shaderGroupBaseAlignment = 16;
      props->shaderGroupHandleAlignment = 16;
      props->shaderGroupHandleCaptureReplaySize = 32;
      props->maxRayDispatchInvocationCount = 1U << 30; /* required min limit */
      props->maxRayHitAttributeSize = BRW_RT_SIZEOF_HIT_ATTRIB_DATA;
   }

   /* VK_KHR_vertex_attribute_divisor */
   {
      props->maxVertexAttribDivisor = UINT32_MAX / 16;
      props->supportsNonZeroFirstInstance = true;
   }

   /* VK_EXT_conservative_rasterization */
   {
      /* There's nothing in the public docs about this value as far as I can
       * tell. However, this is the value the Windows driver reports and
       * there's a comment on a rejected HW feature in the internal docs that
       * says:
       *
       *    "This is similar to conservative rasterization, except the
       *    primitive area is not extended by 1/512 and..."
       *
       * That's a bit of an obtuse reference but it's the best we've got for
       * now.
       */
      props->primitiveOverestimationSize = 1.0f / 512.0f;
      props->maxExtraPrimitiveOverestimationSize = 0.0f;
      props->extraPrimitiveOverestimationSizeGranularity = 0.0f;
      props->primitiveUnderestimation = false;
      props->conservativePointAndLineRasterization = false;
      props->degenerateTrianglesRasterized = true;
      props->degenerateLinesRasterized = false;
      props->fullyCoveredFragmentShaderInputVariable = false;
      props->conservativeRasterizationPostDepthCoverage = true;
   }

   /* VK_EXT_custom_border_color */
   {
      props->maxCustomBorderColorSamplers = MAX_CUSTOM_BORDER_COLORS;
   }

   /* VK_EXT_extended_dynamic_state3 */
   {
      props->dynamicPrimitiveTopologyUnrestricted = true;
   }

   /* VK_EXT_external_memory_host */
   {
      props->minImportedHostPointerAlignment = 4096;
   }

   /* VK_EXT_graphics_pipeline_library */
   {
      props->graphicsPipelineLibraryFastLinking = true;
      props->graphicsPipelineLibraryIndependentInterpolationDecoration = true;
   }

   /* VK_EXT_line_rasterization */
   {
      /* In the Skylake PRM Vol. 7, subsection titled "GIQ (Diamond) Sampling
       * Rules - Legacy Mode", it says the following:
       *
       *    "Note that the device divides a pixel into a 16x16 array of
       *     subpixels, referenced by their upper left corners."
       *
       * This is the only known reference in the PRMs to the subpixel
       * precision of line rasterization and a "16x16 array of subpixels"
       * implies 4 subpixel precision bits. Empirical testing has shown that 4
       * subpixel precision bits applies to all line rasterization types.
       */
      props->lineSubPixelPrecisionBits = 4;
   }

   /* VK_EXT_mesh_shader */
   {
      /* Bounded by the maximum representable size in
       * 3DSTATE_MESH_SHADER_BODY::SharedLocalMemorySize.  Same for Task.
       */
      const uint32_t max_slm_size = 64 * 1024;

      /* Bounded by the maximum representable size in
       * 3DSTATE_MESH_SHADER_BODY::LocalXMaximum.  Same for Task.
       */
      const uint32_t max_workgroup_size = 1 << 10;

      /* 3DMESH_3D limitation. */
      const uint32_t max_threadgroup_count = 1 << 22;

      /* 3DMESH_3D limitation. */
      const uint32_t max_threadgroup_xyz = 65535;

      const uint32_t max_urb_size = 64 * 1024;

      props->maxTaskWorkGroupTotalCount = max_threadgroup_count;
      props->maxTaskWorkGroupCount[0] = max_threadgroup_xyz;
      props->maxTaskWorkGroupCount[1] = max_threadgroup_xyz;
      props->maxTaskWorkGroupCount[2] = max_threadgroup_xyz;

      props->maxTaskWorkGroupInvocations = max_workgroup_size;
      props->maxTaskWorkGroupSize[0] = max_workgroup_size;
      props->maxTaskWorkGroupSize[1] = max_workgroup_size;
      props->maxTaskWorkGroupSize[2] = max_workgroup_size;

      /* TUE header with padding */
      const uint32_t task_payload_reserved = 32;

      props->maxTaskPayloadSize = max_urb_size - task_payload_reserved;
      props->maxTaskSharedMemorySize = max_slm_size;
      props->maxTaskPayloadAndSharedMemorySize =
         props->maxTaskPayloadSize +
         props->maxTaskSharedMemorySize;

      props->maxMeshWorkGroupTotalCount = max_threadgroup_count;
      props->maxMeshWorkGroupCount[0] = max_threadgroup_xyz;
      props->maxMeshWorkGroupCount[1] = max_threadgroup_xyz;
      props->maxMeshWorkGroupCount[2] = max_threadgroup_xyz;

      props->maxMeshWorkGroupInvocations = max_workgroup_size;
      props->maxMeshWorkGroupSize[0] = max_workgroup_size;
      props->maxMeshWorkGroupSize[1] = max_workgroup_size;
      props->maxMeshWorkGroupSize[2] = max_workgroup_size;

      props->maxMeshSharedMemorySize = max_slm_size;
      props->maxMeshPayloadAndSharedMemorySize =
         props->maxTaskPayloadSize +
         props->maxMeshSharedMemorySize;

      /* Unfortunately spec's formula for the max output size doesn't match our hardware
       * (because some per-primitive and per-vertex attributes have alignment restrictions),
       * so we have to advertise the minimum value mandated by the spec to not overflow it.
       */
      props->maxMeshOutputPrimitives = 256;
      props->maxMeshOutputVertices = 256;

      /* NumPrim + Primitive Data List */
      const uint32_t max_indices_memory =
         ALIGN(sizeof(uint32_t) +
               sizeof(uint32_t) * props->maxMeshOutputVertices, 32);

      props->maxMeshOutputMemorySize = MIN2(max_urb_size - max_indices_memory, 32768);

      props->maxMeshPayloadAndOutputMemorySize =
         props->maxTaskPayloadSize +
         props->maxMeshOutputMemorySize;

      props->maxMeshOutputComponents = 128;

      /* RTAIndex is 11-bits wide */
      props->maxMeshOutputLayers = 1 << 11;

      props->maxMeshMultiviewViewCount = 1;

      /* Elements in Vertex Data Array must be aligned to 32 bytes (8 dwords). */
      props->meshOutputPerVertexGranularity = 8;
      /* Elements in Primitive Data Array must be aligned to 32 bytes (8 dwords). */
      props->meshOutputPerPrimitiveGranularity = 8;

      /* SIMD16 */
      props->maxPreferredTaskWorkGroupInvocations = 16;
      props->maxPreferredMeshWorkGroupInvocations = 16;

      props->prefersLocalInvocationVertexOutput = false;
      props->prefersLocalInvocationPrimitiveOutput = false;
      props->prefersCompactVertexOutput = false;
      props->prefersCompactPrimitiveOutput = false;

      /* Spec minimum values */
      assert(props->maxTaskWorkGroupTotalCount >= (1U << 22));
      assert(props->maxTaskWorkGroupCount[0] >= 65535);
      assert(props->maxTaskWorkGroupCount[1] >= 65535);
      assert(props->maxTaskWorkGroupCount[2] >= 65535);

      assert(props->maxTaskWorkGroupInvocations >= 128);
      assert(props->maxTaskWorkGroupSize[0] >= 128);
      assert(props->maxTaskWorkGroupSize[1] >= 128);
      assert(props->maxTaskWorkGroupSize[2] >= 128);

      assert(props->maxTaskPayloadSize >= 16384);
      assert(props->maxTaskSharedMemorySize >= 32768);
      assert(props->maxTaskPayloadAndSharedMemorySize >= 32768);


      assert(props->maxMeshWorkGroupTotalCount >= (1U << 22));
      assert(props->maxMeshWorkGroupCount[0] >= 65535);
      assert(props->maxMeshWorkGroupCount[1] >= 65535);
      assert(props->maxMeshWorkGroupCount[2] >= 65535);

      assert(props->maxMeshWorkGroupInvocations >= 128);
      assert(props->maxMeshWorkGroupSize[0] >= 128);
      assert(props->maxMeshWorkGroupSize[1] >= 128);
      assert(props->maxMeshWorkGroupSize[2] >= 128);

      assert(props->maxMeshSharedMemorySize >= 28672);
      assert(props->maxMeshPayloadAndSharedMemorySize >= 28672);
      assert(props->maxMeshOutputMemorySize >= 32768);
      assert(props->maxMeshPayloadAndOutputMemorySize >= 48128);

      assert(props->maxMeshOutputComponents >= 128);

      assert(props->maxMeshOutputVertices >= 256);
      assert(props->maxMeshOutputPrimitives >= 256);
      assert(props->maxMeshOutputLayers >= 8);
      assert(props->maxMeshMultiviewViewCount >= 1);
   }

   /* VK_EXT_multi_draw */
   {
      props->maxMultiDrawCount = 2048;
   }

   /* VK_EXT_nested_command_buffer */
   {
      props->maxCommandBufferNestingLevel = UINT32_MAX;
   }

   /* VK_EXT_pci_bus_info */
   {
      props->pciDomain = pdevice->info.pci_domain;
      props->pciBus = pdevice->info.pci_bus;
      props->pciDevice = pdevice->info.pci_dev;
      props->pciFunction = pdevice->info.pci_func;
   }

   /* VK_EXT_physical_device_drm */
   {
      props->drmHasPrimary = pdevice->has_master;
      props->drmPrimaryMajor = pdevice->master_major;
      props->drmPrimaryMinor = pdevice->master_minor;
      props->drmHasRender = pdevice->has_local;
      props->drmRenderMajor = pdevice->local_major;
      props->drmRenderMinor = pdevice->local_minor;
   }

   /* VK_EXT_pipeline_robustness */
   {
      props->defaultRobustnessStorageBuffers =
         VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_DISABLED_EXT;
      props->defaultRobustnessUniformBuffers =
         VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_DISABLED_EXT;
      props->defaultRobustnessVertexInputs =
         VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_2_EXT;
      props->defaultRobustnessImages =
         VK_PIPELINE_ROBUSTNESS_IMAGE_BEHAVIOR_ROBUST_IMAGE_ACCESS_2_EXT;
   }

   /* VK_EXT_provoking_vertex */
   {
      props->provokingVertexModePerPipeline = true;
      props->transformFeedbackPreservesTriangleFanProvokingVertex = false;
   }

   /* VK_EXT_robustness2 */
   {
      props->robustStorageBufferAccessSizeAlignment =
         ANV_SSBO_BOUNDS_CHECK_ALIGNMENT;
      props->robustUniformBufferAccessSizeAlignment =
         ANV_UBO_ALIGNMENT;
   }

   /* VK_EXT_sample_locations */
   {
      props->sampleLocationSampleCounts =
         isl_device_get_sample_counts(&pdevice->isl_dev);

      /* See also anv_GetPhysicalDeviceMultisamplePropertiesEXT */
      props->maxSampleLocationGridSize.width = 1;
      props->maxSampleLocationGridSize.height = 1;

      props->sampleLocationCoordinateRange[0] = 0;
      props->sampleLocationCoordinateRange[1] = 0.9375;
      props->sampleLocationSubPixelBits = 4;

      props->variableSampleLocations = true;
   }

   /* VK_EXT_shader_module_identifier */
   {
      STATIC_ASSERT(sizeof(vk_shaderModuleIdentifierAlgorithmUUID) ==
                    sizeof(props->shaderModuleIdentifierAlgorithmUUID));
      memcpy(props->shaderModuleIdentifierAlgorithmUUID,
             vk_shaderModuleIdentifierAlgorithmUUID,
             sizeof(props->shaderModuleIdentifierAlgorithmUUID));
   }

   /* VK_EXT_transform_feedback */
   {
      props->maxTransformFeedbackStreams = MAX_XFB_STREAMS;
      props->maxTransformFeedbackBuffers = MAX_XFB_BUFFERS;
      props->maxTransformFeedbackBufferSize = (1ull << 32);
      props->maxTransformFeedbackStreamDataSize = 128 * 4;
      props->maxTransformFeedbackBufferDataSize = 128 * 4;
      props->maxTransformFeedbackBufferDataStride = 2048;
      props->transformFeedbackQueries = true;
      props->transformFeedbackStreamsLinesTriangles = false;
      props->transformFeedbackRasterizationStreamSelect = false;
      props->transformFeedbackDraw = true;
   }
}

static uint64_t
anv_compute_sys_heap_size(struct anv_physical_device *device,
                          uint64_t total_ram)
{
   /* We don't want to burn too much ram with the GPU.  If the user has 4GiB
    * or less, we use at most half.  If they have more than 4GiB, we use 3/4.
    */
   uint64_t available_ram;
   if (total_ram <= 4ull * 1024ull * 1024ull * 1024ull)
      available_ram = total_ram / 2;
   else
      available_ram = total_ram * 3 / 4;

   return available_ram;
}

static VkResult MUST_CHECK
anv_init_meminfo(struct anv_physical_device *device, int fd)
{
   const struct intel_device_info *devinfo = &device->info;

   device->sys.region = &devinfo->mem.sram.mem;
   device->sys.size =
      anv_compute_sys_heap_size(device, devinfo->mem.sram.mappable.size);
   device->sys.available = devinfo->mem.sram.mappable.free;

   device->vram_mappable.region = &devinfo->mem.vram.mem;
   device->vram_mappable.size = devinfo->mem.vram.mappable.size;
   device->vram_mappable.available = devinfo->mem.vram.mappable.free;

   device->vram_non_mappable.region = &devinfo->mem.vram.mem;
   device->vram_non_mappable.size = devinfo->mem.vram.unmappable.size;
   device->vram_non_mappable.available = devinfo->mem.vram.unmappable.free;

   return VK_SUCCESS;
}

static void
anv_update_meminfo(struct anv_physical_device *device, int fd)
{
   if (!intel_device_info_update_memory_info(&device->info, fd))
      return;

   const struct intel_device_info *devinfo = &device->info;
   device->sys.available = devinfo->mem.sram.mappable.free;
   device->vram_mappable.available = devinfo->mem.vram.mappable.free;
   device->vram_non_mappable.available = devinfo->mem.vram.unmappable.free;
}

static VkResult
anv_physical_device_init_heaps(struct anv_physical_device *device, int fd)
{
   VkResult result = anv_init_meminfo(device, fd);
   if (result != VK_SUCCESS)
      return result;

   assert(device->sys.size != 0);

   if (anv_physical_device_has_vram(device)) {
      /* We can create 2 or 3 different heaps when we have local memory
       * support, first heap with local memory size and second with system
       * memory size and the third is added only if part of the vram is
       * mappable to the host.
       */
      device->memory.heap_count = 2;
      device->memory.heaps[0] = (struct anv_memory_heap) {
         /* If there is a vram_non_mappable, use that for the device only
          * heap. Otherwise use the vram_mappable.
          */
         .size = device->vram_non_mappable.size != 0 ?
                 device->vram_non_mappable.size : device->vram_mappable.size,
         .flags = VK_MEMORY_HEAP_DEVICE_LOCAL_BIT,
         .is_local_mem = true,
      };
      device->memory.heaps[1] = (struct anv_memory_heap) {
         .size = device->sys.size,
         .flags = 0,
         .is_local_mem = false,
      };
      /* Add an additional smaller vram mappable heap if we can't map all the
       * vram to the host.
       */
      if (device->vram_non_mappable.size > 0) {
         device->memory.heap_count++;
         device->memory.heaps[2] = (struct anv_memory_heap) {
            .size = device->vram_mappable.size,
            .flags = VK_MEMORY_HEAP_DEVICE_LOCAL_BIT,
            .is_local_mem = true,
         };
      }
   } else {
      device->memory.heap_count = 1;
      device->memory.heaps[0] = (struct anv_memory_heap) {
         .size = device->sys.size,
         .flags = VK_MEMORY_HEAP_DEVICE_LOCAL_BIT,
         .is_local_mem = false,
      };
   }

   switch (device->info.kmd_type) {
   case INTEL_KMD_TYPE_XE:
      result = anv_xe_physical_device_init_memory_types(device);
      break;
   case INTEL_KMD_TYPE_I915:
   default:
      result = anv_i915_physical_device_init_memory_types(device);
      break;
   }

   if (result != VK_SUCCESS)
      return result;

   for (unsigned i = 0; i < device->memory.type_count; i++) {
      VkMemoryPropertyFlags props = device->memory.types[i].propertyFlags;
      if ((props & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) &&
          !(props & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
#ifdef SUPPORT_INTEL_INTEGRATED_GPUS
         device->memory.need_flush = true;
#else
         return vk_errorf(device, VK_ERROR_INITIALIZATION_FAILED,
                          "Memory configuration requires flushing, but it's not implemented for this architecture");
#endif
   }

   return VK_SUCCESS;
}

static VkResult
anv_physical_device_init_uuids(struct anv_physical_device *device)
{
   const struct build_id_note *note =
      build_id_find_nhdr_for_addr(anv_physical_device_init_uuids);
   if (!note) {
      return vk_errorf(device, VK_ERROR_INITIALIZATION_FAILED,
                       "Failed to find build-id");
   }

   unsigned build_id_len = build_id_length(note);
   if (build_id_len < 20) {
      return vk_errorf(device, VK_ERROR_INITIALIZATION_FAILED,
                       "build-id too short.  It needs to be a SHA");
   }

   memcpy(device->driver_build_sha1, build_id_data(note), 20);

   struct mesa_sha1 sha1_ctx;
   uint8_t sha1[20];
   STATIC_ASSERT(VK_UUID_SIZE <= sizeof(sha1));

   /* The pipeline cache UUID is used for determining when a pipeline cache is
    * invalid.  It needs both a driver build and the PCI ID of the device.
    */
   _mesa_sha1_init(&sha1_ctx);
   _mesa_sha1_update(&sha1_ctx, build_id_data(note), build_id_len);
   _mesa_sha1_update(&sha1_ctx, &device->info.pci_device_id,
                     sizeof(device->info.pci_device_id));
   _mesa_sha1_update(&sha1_ctx, &device->always_use_bindless,
                     sizeof(device->always_use_bindless));
   _mesa_sha1_final(&sha1_ctx, sha1);
   memcpy(device->pipeline_cache_uuid, sha1, VK_UUID_SIZE);

   intel_uuid_compute_driver_id(device->driver_uuid, &device->info, VK_UUID_SIZE);
   intel_uuid_compute_device_id(device->device_uuid, &device->info, VK_UUID_SIZE);

   return VK_SUCCESS;
}

static void
anv_physical_device_init_disk_cache(struct anv_physical_device *device)
{
#ifdef ENABLE_SHADER_CACHE
   char renderer[10];
   ASSERTED int len = snprintf(renderer, sizeof(renderer), "anv_%04x",
                               device->info.pci_device_id);
   assert(len == sizeof(renderer) - 2);

   char timestamp[41];
   _mesa_sha1_format(timestamp, device->driver_build_sha1);

   const uint64_t driver_flags =
      brw_get_compiler_config_value(device->compiler);
   device->vk.disk_cache = disk_cache_create(renderer, timestamp, driver_flags);
#endif
}

static void
anv_physical_device_free_disk_cache(struct anv_physical_device *device)
{
#ifdef ENABLE_SHADER_CACHE
   if (device->vk.disk_cache) {
      disk_cache_destroy(device->vk.disk_cache);
      device->vk.disk_cache = NULL;
   }
#else
   assert(device->vk.disk_cache == NULL);
#endif
}

/* The ANV_QUEUE_OVERRIDE environment variable is a comma separated list of
 * queue overrides.
 *
 * To override the number queues:
 *  * "gc" is for graphics queues with compute support
 *  * "g" is for graphics queues with no compute support
 *  * "c" is for compute queues with no graphics support
 *  * "v" is for video queues with no graphics support
 *
 * For example, ANV_QUEUE_OVERRIDE=gc=2,c=1 would override the number of
 * advertised queues to be 2 queues with graphics+compute support, and 1 queue
 * with compute-only support.
 *
 * ANV_QUEUE_OVERRIDE=c=1 would override the number of advertised queues to
 * include 1 queue with compute-only support, but it will not change the
 * number of graphics+compute queues.
 *
 * ANV_QUEUE_OVERRIDE=gc=0,c=1 would override the number of advertised queues
 * to include 1 queue with compute-only support, and it would override the
 * number of graphics+compute queues to be 0.
 */
static void
anv_override_engine_counts(int *gc_count, int *g_count, int *c_count, int *v_count)
{
   int gc_override = -1;
   int g_override = -1;
   int c_override = -1;
   int v_override = -1;
   char *env = getenv("ANV_QUEUE_OVERRIDE");

   if (env == NULL)
      return;

   env = strdup(env);
   char *save = NULL;
   char *next = strtok_r(env, ",", &save);
   while (next != NULL) {
      if (strncmp(next, "gc=", 3) == 0) {
         gc_override = strtol(next + 3, NULL, 0);
      } else if (strncmp(next, "g=", 2) == 0) {
         g_override = strtol(next + 2, NULL, 0);
      } else if (strncmp(next, "c=", 2) == 0) {
         c_override = strtol(next + 2, NULL, 0);
      } else if (strncmp(next, "v=", 2) == 0) {
         v_override = strtol(next + 2, NULL, 0);
      } else {
         mesa_logw("Ignoring unsupported ANV_QUEUE_OVERRIDE token: %s", next);
      }
      next = strtok_r(NULL, ",", &save);
   }
   free(env);
   if (gc_override >= 0)
      *gc_count = gc_override;
   if (g_override >= 0)
      *g_count = g_override;
   if (*g_count > 0 && *gc_count <= 0 && (gc_override >= 0 || g_override >= 0))
      mesa_logw("ANV_QUEUE_OVERRIDE: gc=0 with g > 0 violates the "
                "Vulkan specification");
   if (c_override >= 0)
      *c_count = c_override;
   if (v_override >= 0)
      *v_count = v_override;
}

static void
anv_physical_device_init_queue_families(struct anv_physical_device *pdevice)
{
   uint32_t family_count = 0;
   VkQueueFlags sparse_flags = (pdevice->instance->has_fake_sparse ||
                                pdevice->has_sparse) ?
                               VK_QUEUE_SPARSE_BINDING_BIT : 0;

   if (pdevice->engine_info) {
      int gc_count =
         intel_engines_count(pdevice->engine_info,
                             INTEL_ENGINE_CLASS_RENDER);
      int v_count =
         intel_engines_count(pdevice->engine_info, INTEL_ENGINE_CLASS_VIDEO);
      int g_count = 0;
      int c_count = 0;
      const bool kernel_supports_non_render_engines =
         pdevice->info.kmd_type == INTEL_KMD_TYPE_XE || pdevice->has_vm_control;
      const bool sparse_supports_non_render_engines =
         !pdevice->has_sparse || !pdevice->sparse_uses_trtt;
      const bool can_use_non_render_engines =
         kernel_supports_non_render_engines &&
         sparse_supports_non_render_engines;

      if (debug_get_bool_option("INTEL_COMPUTE_CLASS", false)) {
         if (!can_use_non_render_engines)
            mesa_logw("cannot initialize compute engine");
         else
            c_count = intel_engines_count(pdevice->engine_info,
                                          INTEL_ENGINE_CLASS_COMPUTE);
      }
      enum intel_engine_class compute_class =
         c_count < 1 ? INTEL_ENGINE_CLASS_RENDER : INTEL_ENGINE_CLASS_COMPUTE;

      int blit_count = 0;
      if (debug_get_bool_option("INTEL_COPY_CLASS", true) &&
          pdevice->info.verx10 >= 125) {
         if (!can_use_non_render_engines)
            mesa_logw("cannot initialize blitter engine");
         else
            blit_count = intel_engines_count(pdevice->engine_info,
                                             INTEL_ENGINE_CLASS_COPY);
      }

      anv_override_engine_counts(&gc_count, &g_count, &c_count, &v_count);

      if (gc_count > 0) {
         pdevice->queue.families[family_count++] = (struct anv_queue_family) {
            .queueFlags = VK_QUEUE_GRAPHICS_BIT |
                          VK_QUEUE_COMPUTE_BIT |
                          VK_QUEUE_TRANSFER_BIT |
                          sparse_flags,
            .queueCount = gc_count,
            .engine_class = INTEL_ENGINE_CLASS_RENDER,
         };
      }
      if (g_count > 0) {
         pdevice->queue.families[family_count++] = (struct anv_queue_family) {
            .queueFlags = VK_QUEUE_GRAPHICS_BIT |
                          VK_QUEUE_TRANSFER_BIT |
                          sparse_flags,
            .queueCount = g_count,
            .engine_class = INTEL_ENGINE_CLASS_RENDER,
         };
      }
      if (c_count > 0) {
         pdevice->queue.families[family_count++] = (struct anv_queue_family) {
            .queueFlags = VK_QUEUE_COMPUTE_BIT |
                          VK_QUEUE_TRANSFER_BIT |
                          sparse_flags,
            .queueCount = c_count,
            .engine_class = compute_class,
         };
      }
      if (v_count > 0 && pdevice->video_decode_enabled) {
         /* HEVC support on Gfx9 is only available on VCS0. So limit the number of video queues
          * to the first VCS engine instance.
          *
          * We should be able to query HEVC support from the kernel using the engine query uAPI,
          * but this appears to be broken :
          *    https://gitlab.freedesktop.org/drm/intel/-/issues/8832
          *
          * When this bug is fixed we should be able to check HEVC support to determine the
          * correct number of queues.
          */
         pdevice->queue.families[family_count++] = (struct anv_queue_family) {
            .queueFlags = VK_QUEUE_VIDEO_DECODE_BIT_KHR,
            .queueCount = pdevice->info.ver == 9 ? MIN2(1, v_count) : v_count,
            .engine_class = INTEL_ENGINE_CLASS_VIDEO,
         };
      }
      if (blit_count > 0) {
         pdevice->queue.families[family_count++] = (struct anv_queue_family) {
            .queueFlags = VK_QUEUE_TRANSFER_BIT,
            .queueCount = blit_count,
            .engine_class = INTEL_ENGINE_CLASS_COPY,
         };
      }
   } else {
      /* Default to a single render queue */
      pdevice->queue.families[family_count++] = (struct anv_queue_family) {
         .queueFlags = VK_QUEUE_GRAPHICS_BIT |
                       VK_QUEUE_COMPUTE_BIT |
                       VK_QUEUE_TRANSFER_BIT |
                       sparse_flags,
         .queueCount = 1,
         .engine_class = INTEL_ENGINE_CLASS_RENDER,
      };
      family_count = 1;
   }
   assert(family_count <= ANV_MAX_QUEUE_FAMILIES);
   pdevice->queue.family_count = family_count;
}

static VkResult
anv_physical_device_get_parameters(struct anv_physical_device *device)
{
   switch (device->info.kmd_type) {
   case INTEL_KMD_TYPE_I915:
      return anv_i915_physical_device_get_parameters(device);
   case INTEL_KMD_TYPE_XE:
      return anv_xe_physical_device_get_parameters(device);
   default:
      unreachable("Missing");
      return VK_ERROR_UNKNOWN;
   }
}

static VkResult
anv_physical_device_try_create(struct vk_instance *vk_instance,
                               struct _drmDevice *drm_device,
                               struct vk_physical_device **out)
{
   struct anv_instance *instance =
      container_of(vk_instance, struct anv_instance, vk);

   if (!(drm_device->available_nodes & (1 << DRM_NODE_RENDER)) ||
       drm_device->bustype != DRM_BUS_PCI ||
       drm_device->deviceinfo.pci->vendor_id != 0x8086)
      return VK_ERROR_INCOMPATIBLE_DRIVER;

   const char *primary_path = drm_device->nodes[DRM_NODE_PRIMARY];
   const char *path = drm_device->nodes[DRM_NODE_RENDER];
   VkResult result;
   int fd;
   int master_fd = -1;

   brw_process_intel_debug_variable();

   fd = open(path, O_RDWR | O_CLOEXEC);
   if (fd < 0) {
      if (errno == ENOMEM) {
         return vk_errorf(instance, VK_ERROR_OUT_OF_HOST_MEMORY,
                          "Unable to open device %s: out of memory", path);
      }
      return vk_errorf(instance, VK_ERROR_INCOMPATIBLE_DRIVER,
                       "Unable to open device %s: %m", path);
   }

   struct intel_device_info devinfo;
   if (!intel_get_device_info_from_fd(fd, &devinfo)) {
      result = vk_error(instance, VK_ERROR_INCOMPATIBLE_DRIVER);
      goto fail_fd;
   }

   if (devinfo.ver == 20) {
      mesa_logw("Vulkan not yet supported on %s", devinfo.name);
   } else if (devinfo.ver > 12) {
      result = vk_errorf(instance, VK_ERROR_INCOMPATIBLE_DRIVER,
                         "Vulkan not yet supported on %s", devinfo.name);
      goto fail_fd;
   } else if (devinfo.ver < 9) {
      /* Silently fail here, hasvk should pick up this device. */
      result = VK_ERROR_INCOMPATIBLE_DRIVER;
      goto fail_fd;
   }

   /* Disable Wa_16013994831 on Gfx12.0 because we found other cases where we
    * need to always disable preemption :
    *    - https://gitlab.freedesktop.org/mesa/mesa/-/issues/5963
    *    - https://gitlab.freedesktop.org/mesa/mesa/-/issues/5662
    */
   if (devinfo.verx10 == 120)
      BITSET_CLEAR(devinfo.workarounds, INTEL_WA_16013994831);

   if (!devinfo.has_context_isolation) {
      result = vk_errorf(instance, VK_ERROR_INCOMPATIBLE_DRIVER,
                         "Vulkan requires context isolation for %s", devinfo.name);
      goto fail_fd;
   }

   struct anv_physical_device *device =
      vk_zalloc(&instance->vk.alloc, sizeof(*device), 8,
                VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
   if (device == NULL) {
      result = vk_error(instance, VK_ERROR_OUT_OF_HOST_MEMORY);
      goto fail_fd;
   }

   struct vk_physical_device_dispatch_table dispatch_table;
   vk_physical_device_dispatch_table_from_entrypoints(
      &dispatch_table, &anv_physical_device_entrypoints, true);
   vk_physical_device_dispatch_table_from_entrypoints(
      &dispatch_table, &wsi_physical_device_entrypoints, false);

   result = vk_physical_device_init(&device->vk, &instance->vk,
                                    NULL, NULL, NULL, /* We set up extensions later */
                                    &dispatch_table);
   if (result != VK_SUCCESS) {
      vk_error(instance, result);
      goto fail_alloc;
   }
   device->instance = instance;

   assert(strlen(path) < ARRAY_SIZE(device->path));
   snprintf(device->path, ARRAY_SIZE(device->path), "%s", path);

   device->info = devinfo;

   device->local_fd = fd;
   result = anv_physical_device_get_parameters(device);
   if (result != VK_SUCCESS)
      goto fail_base;

   device->gtt_size = device->info.gtt_size ? device->info.gtt_size :
                                              device->info.aperture_bytes;

   if (device->gtt_size < (4ULL << 30 /* GiB */)) {
      vk_errorf(instance, VK_ERROR_INCOMPATIBLE_DRIVER,
                "GTT size too small: 0x%016"PRIx64, device->gtt_size);
      goto fail_base;
   }

   /* We currently only have the right bits for instructions in Gen12+. If the
    * kernel ever starts supporting that feature on previous generations,
    * we'll need to edit genxml prior to enabling here.
    */
   device->has_protected_contexts = device->info.ver >= 12 &&
      intel_gem_supports_protected_context(fd, device->info.kmd_type);

   /* Just pick one; they're all the same */
   device->has_astc_ldr =
      isl_format_supports_sampling(&device->info,
                                   ISL_FORMAT_ASTC_LDR_2D_4X4_FLT16);
   if (!device->has_astc_ldr &&
       driQueryOptionb(&device->instance->dri_options, "vk_require_astc"))
      device->emu_astc_ldr = true;
   if (devinfo.ver == 9 && !intel_device_info_is_9lp(&devinfo)) {
      device->flush_astc_ldr_void_extent_denorms =
         device->has_astc_ldr && !device->emu_astc_ldr;
   }
   device->disable_fcv = intel_device_info_is_mtl(&device->info) ||
                         instance->disable_fcv;

   result = anv_physical_device_init_heaps(device, fd);
   if (result != VK_SUCCESS)
      goto fail_base;

   if (debug_get_bool_option("ANV_QUEUE_THREAD_DISABLE", false))
      device->has_exec_timeline = false;

   device->has_cooperative_matrix =
      device->info.cooperative_matrix_configurations[0].scope != SCOPE_NONE;

   unsigned st_idx = 0;

   device->sync_syncobj_type = vk_drm_syncobj_get_type(fd);
   if (!device->has_exec_timeline)
      device->sync_syncobj_type.features &= ~VK_SYNC_FEATURE_TIMELINE;
   device->sync_types[st_idx++] = &device->sync_syncobj_type;

   /* anv_bo_sync_type is only supported with i915 for now  */
   if (device->info.kmd_type == INTEL_KMD_TYPE_I915) {
      if (!(device->sync_syncobj_type.features & VK_SYNC_FEATURE_CPU_WAIT))
         device->sync_types[st_idx++] = &anv_bo_sync_type;

      if (!(device->sync_syncobj_type.features & VK_SYNC_FEATURE_TIMELINE)) {
         device->sync_timeline_type = vk_sync_timeline_get_type(&anv_bo_sync_type);
         device->sync_types[st_idx++] = &device->sync_timeline_type.sync;
      }
   } else {
      assert(device->sync_syncobj_type.features & VK_SYNC_FEATURE_TIMELINE);
      assert(device->sync_syncobj_type.features & VK_SYNC_FEATURE_CPU_WAIT);
   }

   device->sync_types[st_idx++] = NULL;
   assert(st_idx <= ARRAY_SIZE(device->sync_types));
   device->vk.supported_sync_types = device->sync_types;

   device->vk.pipeline_cache_import_ops = anv_cache_import_ops;

   device->always_use_bindless =
      debug_get_bool_option("ANV_ALWAYS_BINDLESS", false);

   device->use_call_secondary =
      !debug_get_bool_option("ANV_DISABLE_SECONDARY_CMD_BUFFER_CALLS", false);

   device->video_decode_enabled = debug_get_bool_option("ANV_VIDEO_DECODE", false);

   device->uses_ex_bso = device->info.verx10 >= 125;

   /* For now always use indirect descriptors. We'll update this
    * to !uses_ex_bso when all the infrastructure is built up.
    */
   device->indirect_descriptors =
      !device->uses_ex_bso ||
      driQueryOptionb(&instance->dri_options, "force_indirect_descriptors");

   /* Check if we can read the GPU timestamp register from the CPU */
   uint64_t u64_ignore;
   device->has_reg_timestamp = intel_gem_read_render_timestamp(fd,
                                                               device->info.kmd_type,
                                                               &u64_ignore);

   device->uses_relocs = device->info.kmd_type != INTEL_KMD_TYPE_XE;

   /* While xe.ko can use both vm_bind and TR-TT, i915.ko only has TR-TT. */
   if (device->info.kmd_type == INTEL_KMD_TYPE_XE) {
      device->has_sparse = true;
      device->sparse_uses_trtt =
         debug_get_bool_option("ANV_SPARSE_USE_TRTT", false);
   } else {
      device->has_sparse =
         device->info.ver >= 12 &&
         device->has_exec_timeline &&
         debug_get_bool_option("ANV_SPARSE", true);
      device->sparse_uses_trtt = true;
   }

   device->always_flush_cache = INTEL_DEBUG(DEBUG_STALL) ||
      driQueryOptionb(&instance->dri_options, "always_flush_cache");

   device->compiler = brw_compiler_create(NULL, &device->info);
   if (device->compiler == NULL) {
      result = vk_error(instance, VK_ERROR_OUT_OF_HOST_MEMORY);
      goto fail_base;
   }
   device->compiler->shader_debug_log = compiler_debug_log;
   device->compiler->shader_perf_log = compiler_perf_log;
   device->compiler->constant_buffer_0_is_relative = false;
   device->compiler->supports_shader_constants = true;
   device->compiler->indirect_ubos_use_sampler = device->info.ver < 12;
   device->compiler->extended_bindless_surface_offset = device->uses_ex_bso;
   device->compiler->use_bindless_sampler_offset = false;
   device->compiler->spilling_rate =
      driQueryOptioni(&instance->dri_options, "shader_spilling_rate");

   isl_device_init(&device->isl_dev, &device->info);
   device->isl_dev.buffer_length_in_aux_addr = true;

   result = anv_physical_device_init_uuids(device);
   if (result != VK_SUCCESS)
      goto fail_compiler;

   anv_physical_device_init_va_ranges(device);

   anv_physical_device_init_disk_cache(device);

   if (instance->vk.enabled_extensions.KHR_display) {
      master_fd = open(primary_path, O_RDWR | O_CLOEXEC);
      if (master_fd >= 0) {
         /* fail if we don't have permission to even render on this device */
         if (!intel_gem_can_render_on_fd(master_fd, device->info.kmd_type)) {
            close(master_fd);
            master_fd = -1;
         }
      }
   }
   device->master_fd = master_fd;

   device->engine_info = intel_engine_get_info(fd, device->info.kmd_type);
   device->info.has_compute_engine = device->engine_info &&
                                     intel_engines_count(device->engine_info,
                                                         INTEL_ENGINE_CLASS_COMPUTE);
   anv_physical_device_init_queue_families(device);

   anv_physical_device_init_perf(device, fd);

   /* Gather major/minor before WSI. */
   struct stat st;

   if (stat(primary_path, &st) == 0) {
      device->has_master = true;
      device->master_major = major(st.st_rdev);
      device->master_minor = minor(st.st_rdev);
   } else {
      device->has_master = false;
      device->master_major = 0;
      device->master_minor = 0;
   }

   if (stat(path, &st) == 0) {
      device->has_local = true;
      device->local_major = major(st.st_rdev);
      device->local_minor = minor(st.st_rdev);
   } else {
      device->has_local = false;
      device->local_major = 0;
      device->local_minor = 0;
   }

   get_device_extensions(device, &device->vk.supported_extensions);
   get_features(device, &device->vk.supported_features);
   get_properties(device, &device->vk.properties);

   result = anv_init_wsi(device);
   if (result != VK_SUCCESS)
      goto fail_perf;

   anv_measure_device_init(device);

   anv_genX(&device->info, init_physical_device_state)(device);

   *out = &device->vk;

   return VK_SUCCESS;

fail_perf:
   ralloc_free(device->perf);
   free(device->engine_info);
   anv_physical_device_free_disk_cache(device);
fail_compiler:
   ralloc_free(device->compiler);
fail_base:
   vk_physical_device_finish(&device->vk);
fail_alloc:
   vk_free(&instance->vk.alloc, device);
fail_fd:
   close(fd);
   if (master_fd != -1)
      close(master_fd);
   return result;
}

static void
anv_physical_device_destroy(struct vk_physical_device *vk_device)
{
   struct anv_physical_device *device =
      container_of(vk_device, struct anv_physical_device, vk);

   anv_finish_wsi(device);
   anv_measure_device_destroy(device);
   free(device->engine_info);
   anv_physical_device_free_disk_cache(device);
   ralloc_free(device->compiler);
   ralloc_free(device->perf);
   close(device->local_fd);
   if (device->master_fd >= 0)
      close(device->master_fd);
   vk_physical_device_finish(&device->vk);
   vk_free(&device->instance->vk.alloc, device);
}

VkResult anv_EnumerateInstanceExtensionProperties(
    const char*                                 pLayerName,
    uint32_t*                                   pPropertyCount,
    VkExtensionProperties*                      pProperties)
{
   if (pLayerName)
      return vk_error(NULL, VK_ERROR_LAYER_NOT_PRESENT);

   return vk_enumerate_instance_extension_properties(
      &instance_extensions, pPropertyCount, pProperties);
}

static void
anv_init_dri_options(struct anv_instance *instance)
{
   driParseOptionInfo(&instance->available_dri_options, anv_dri_options,
                      ARRAY_SIZE(anv_dri_options));
   driParseConfigFiles(&instance->dri_options,
                       &instance->available_dri_options, 0, "anv", NULL, NULL,
                       instance->vk.app_info.app_name,
                       instance->vk.app_info.app_version,
                       instance->vk.app_info.engine_name,
                       instance->vk.app_info.engine_version);

    instance->assume_full_subgroups =
            driQueryOptioni(&instance->dri_options, "anv_assume_full_subgroups");
    instance->limit_trig_input_range =
            driQueryOptionb(&instance->dri_options, "limit_trig_input_range");
    instance->sample_mask_out_opengl_behaviour =
            driQueryOptionb(&instance->dri_options, "anv_sample_mask_out_opengl_behaviour");
    instance->force_filter_addr_rounding =
            driQueryOptionb(&instance->dri_options, "anv_force_filter_addr_rounding");
    instance->lower_depth_range_rate =
            driQueryOptionf(&instance->dri_options, "lower_depth_range_rate");
    instance->no_16bit =
            driQueryOptionb(&instance->dri_options, "no_16bit");
    instance->intel_enable_wa_14018912822 =
            driQueryOptionb(&instance->dri_options, "intel_enable_wa_14018912822");
    instance->mesh_conv_prim_attrs_to_vert_attrs =
            driQueryOptioni(&instance->dri_options, "anv_mesh_conv_prim_attrs_to_vert_attrs");
    instance->fp64_workaround_enabled =
            driQueryOptionb(&instance->dri_options, "fp64_workaround_enabled");
    instance->generated_indirect_threshold =
            driQueryOptioni(&instance->dri_options, "generated_indirect_threshold");
    instance->generated_indirect_ring_threshold =
            driQueryOptioni(&instance->dri_options, "generated_indirect_ring_threshold");
    instance->query_clear_with_blorp_threshold =
       driQueryOptioni(&instance->dri_options, "query_clear_with_blorp_threshold");
    instance->query_copy_with_shader_threshold =
       driQueryOptioni(&instance->dri_options, "query_copy_with_shader_threshold");
    instance->force_vk_vendor =
       driQueryOptioni(&instance->dri_options, "force_vk_vendor");
    instance->has_fake_sparse =
       driQueryOptionb(&instance->dri_options, "fake_sparse");
    instance->enable_tbimr = driQueryOptionb(&instance->dri_options, "intel_tbimr");
    instance->disable_fcv =
            driQueryOptionb(&instance->dri_options, "anv_disable_fcv");
}

VkResult anv_CreateInstance(
    const VkInstanceCreateInfo*                 pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkInstance*                                 pInstance)
{
   struct anv_instance *instance;
   VkResult result;

   assert(pCreateInfo->sType == VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO);

   if (pAllocator == NULL)
      pAllocator = vk_default_allocator();

   instance = vk_alloc(pAllocator, sizeof(*instance), 8,
                       VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
   if (!instance)
      return vk_error(NULL, VK_ERROR_OUT_OF_HOST_MEMORY);

   struct vk_instance_dispatch_table dispatch_table;
   vk_instance_dispatch_table_from_entrypoints(
      &dispatch_table, &anv_instance_entrypoints, true);
   vk_instance_dispatch_table_from_entrypoints(
      &dispatch_table, &wsi_instance_entrypoints, false);

   result = vk_instance_init(&instance->vk, &instance_extensions,
                             &dispatch_table, pCreateInfo, pAllocator);
   if (result != VK_SUCCESS) {
      vk_free(pAllocator, instance);
      return vk_error(NULL, result);
   }

   instance->vk.physical_devices.try_create_for_drm = anv_physical_device_try_create;
   instance->vk.physical_devices.destroy = anv_physical_device_destroy;

   VG(VALGRIND_CREATE_MEMPOOL(instance, 0, false));

   anv_init_dri_options(instance);

   intel_driver_ds_init();

   *pInstance = anv_instance_to_handle(instance);

   return VK_SUCCESS;
}

void anv_DestroyInstance(
    VkInstance                                  _instance,
    const VkAllocationCallbacks*                pAllocator)
{
   ANV_FROM_HANDLE(anv_instance, instance, _instance);

   if (!instance)
      return;

   VG(VALGRIND_DESTROY_MEMPOOL(instance));

   driDestroyOptionCache(&instance->dri_options);
   driDestroyOptionInfo(&instance->available_dri_options);

   vk_instance_finish(&instance->vk);
   vk_free(&instance->vk.alloc, instance);
}

void anv_GetPhysicalDeviceProperties2(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceProperties2*                pProperties)
{
   vk_common_GetPhysicalDeviceProperties2(physicalDevice, pProperties);

   /* Unfortunately the runtime isn't handling ANDROID extensions. */
   vk_foreach_struct(ext, pProperties->pNext) {
      switch (ext->sType) {
#ifdef ANDROID
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
      case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENTATION_PROPERTIES_ANDROID: {
         VkPhysicalDevicePresentationPropertiesANDROID *props =
            (VkPhysicalDevicePresentationPropertiesANDROID *)ext;
         uint64_t front_rendering_usage = 0;
         struct u_gralloc *gralloc = u_gralloc_create(U_GRALLOC_TYPE_AUTO);
         if (gralloc != NULL) {
            u_gralloc_get_front_rendering_usage(gralloc, &front_rendering_usage);
            u_gralloc_destroy(&gralloc);
         }
         props->sharedImage = front_rendering_usage ? VK_TRUE : VK_FALSE;
         break;
      }
#pragma GCC diagnostic pop
#endif

      default:
         break;
      }
   }
}

static const VkQueueFamilyProperties
anv_queue_family_properties_template = {
   .timestampValidBits = 36, /* XXX: Real value here */
   .minImageTransferGranularity = { 1, 1, 1 },
};

static VkQueueFamilyProperties
anv_device_physical_get_queue_properties(const struct anv_physical_device *device,
                                         uint32_t family_index)
{
   const struct anv_queue_family *family = &device->queue.families[family_index];
   VkQueueFamilyProperties properties = anv_queue_family_properties_template;
   properties.queueFlags = family->queueFlags;
   properties.queueCount = family->queueCount;
   /* TODO: enable protected content on video queue */
   if (device->has_protected_contexts &&
       (family->queueFlags & VK_QUEUE_VIDEO_DECODE_BIT_KHR) == 0)
      properties.queueFlags |= VK_QUEUE_PROTECTED_BIT;
   return properties;
}

void anv_GetPhysicalDeviceQueueFamilyProperties2(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pQueueFamilyPropertyCount,
    VkQueueFamilyProperties2*                   pQueueFamilyProperties)
{
   ANV_FROM_HANDLE(anv_physical_device, pdevice, physicalDevice);
   VK_OUTARRAY_MAKE_TYPED(VkQueueFamilyProperties2, out,
                          pQueueFamilyProperties, pQueueFamilyPropertyCount);

   for (uint32_t i = 0; i < pdevice->queue.family_count; i++) {
      struct anv_queue_family *queue_family = &pdevice->queue.families[i];
      vk_outarray_append_typed(VkQueueFamilyProperties2, &out, p) {
         p->queueFamilyProperties =
            anv_device_physical_get_queue_properties(pdevice, i);

         vk_foreach_struct(ext, p->pNext) {
            switch (ext->sType) {
            case VK_STRUCTURE_TYPE_QUEUE_FAMILY_GLOBAL_PRIORITY_PROPERTIES_KHR: {
               VkQueueFamilyGlobalPriorityPropertiesKHR *properties =
                  (VkQueueFamilyGlobalPriorityPropertiesKHR *)ext;

               /* Deliberately sorted low to high */
               VkQueueGlobalPriorityKHR all_priorities[] = {
                  VK_QUEUE_GLOBAL_PRIORITY_LOW_KHR,
                  VK_QUEUE_GLOBAL_PRIORITY_MEDIUM_KHR,
                  VK_QUEUE_GLOBAL_PRIORITY_HIGH_KHR,
                  VK_QUEUE_GLOBAL_PRIORITY_REALTIME_KHR,
               };

               uint32_t count = 0;
               for (unsigned i = 0; i < ARRAY_SIZE(all_priorities); i++) {
                  if (all_priorities[i] > pdevice->max_context_priority)
                     break;

                  properties->priorities[count++] = all_priorities[i];
               }
               properties->priorityCount = count;
               break;
            }
            case VK_STRUCTURE_TYPE_QUEUE_FAMILY_QUERY_RESULT_STATUS_PROPERTIES_KHR: {
               VkQueueFamilyQueryResultStatusPropertiesKHR *prop =
                  (VkQueueFamilyQueryResultStatusPropertiesKHR *)ext;
               prop->queryResultStatusSupport = VK_TRUE;
               break;
            }
            case VK_STRUCTURE_TYPE_QUEUE_FAMILY_VIDEO_PROPERTIES_KHR: {
               VkQueueFamilyVideoPropertiesKHR *prop =
                  (VkQueueFamilyVideoPropertiesKHR *)ext;
               if (queue_family->queueFlags & VK_QUEUE_VIDEO_DECODE_BIT_KHR) {
                  prop->videoCodecOperations = VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR |
                                               VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR;
               }
               break;
            }
            default:
               anv_debug_ignored_stype(ext->sType);
            }
         }
      }
   }
}

void anv_GetPhysicalDeviceMemoryProperties(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceMemoryProperties*           pMemoryProperties)
{
   ANV_FROM_HANDLE(anv_physical_device, physical_device, physicalDevice);

   pMemoryProperties->memoryTypeCount = physical_device->memory.type_count;
   for (uint32_t i = 0; i < physical_device->memory.type_count; i++) {
      pMemoryProperties->memoryTypes[i] = (VkMemoryType) {
         .propertyFlags = physical_device->memory.types[i].propertyFlags,
         .heapIndex     = physical_device->memory.types[i].heapIndex,
      };
   }

   pMemoryProperties->memoryHeapCount = physical_device->memory.heap_count;
   for (uint32_t i = 0; i < physical_device->memory.heap_count; i++) {
      pMemoryProperties->memoryHeaps[i] = (VkMemoryHeap) {
         .size    = physical_device->memory.heaps[i].size,
         .flags   = physical_device->memory.heaps[i].flags,
      };
   }
}

static void
anv_get_memory_budget(VkPhysicalDevice physicalDevice,
                      VkPhysicalDeviceMemoryBudgetPropertiesEXT *memoryBudget)
{
   ANV_FROM_HANDLE(anv_physical_device, device, physicalDevice);

   if (!device->vk.supported_extensions.EXT_memory_budget)
      return;

   anv_update_meminfo(device, device->local_fd);

   VkDeviceSize total_sys_heaps_size = 0, total_vram_heaps_size = 0;
   for (size_t i = 0; i < device->memory.heap_count; i++) {
      if (device->memory.heaps[i].is_local_mem) {
         total_vram_heaps_size += device->memory.heaps[i].size;
      } else {
         total_sys_heaps_size += device->memory.heaps[i].size;
      }
   }

   for (size_t i = 0; i < device->memory.heap_count; i++) {
      VkDeviceSize heap_size = device->memory.heaps[i].size;
      VkDeviceSize heap_used = device->memory.heaps[i].used;
      VkDeviceSize heap_budget, total_heaps_size;
      uint64_t mem_available = 0;

      if (device->memory.heaps[i].is_local_mem) {
         total_heaps_size = total_vram_heaps_size;
         if (device->vram_non_mappable.size > 0 && i == 0) {
            mem_available = device->vram_non_mappable.available;
         } else {
            mem_available = device->vram_mappable.available;
         }
      } else {
         total_heaps_size = total_sys_heaps_size;
         mem_available = MIN2(device->sys.available, total_heaps_size);
      }

      double heap_proportion = (double) heap_size / total_heaps_size;
      VkDeviceSize available_prop = mem_available * heap_proportion;

      /*
       * Let's not incite the app to starve the system: report at most 90% of
       * the available heap memory.
       */
      uint64_t heap_available = available_prop * 9 / 10;
      heap_budget = MIN2(heap_size, heap_used + heap_available);

      /*
       * Round down to the nearest MB
       */
      heap_budget &= ~((1ull << 20) - 1);

      /*
       * The heapBudget value must be non-zero for array elements less than
       * VkPhysicalDeviceMemoryProperties::memoryHeapCount. The heapBudget
       * value must be less than or equal to VkMemoryHeap::size for each heap.
       */
      assert(0 < heap_budget && heap_budget <= heap_size);

      memoryBudget->heapUsage[i] = heap_used;
      memoryBudget->heapBudget[i] = heap_budget;
   }

   /* The heapBudget and heapUsage values must be zero for array elements
    * greater than or equal to VkPhysicalDeviceMemoryProperties::memoryHeapCount
    */
   for (uint32_t i = device->memory.heap_count; i < VK_MAX_MEMORY_HEAPS; i++) {
      memoryBudget->heapBudget[i] = 0;
      memoryBudget->heapUsage[i] = 0;
   }
}

void anv_GetPhysicalDeviceMemoryProperties2(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceMemoryProperties2*          pMemoryProperties)
{
   anv_GetPhysicalDeviceMemoryProperties(physicalDevice,
                                         &pMemoryProperties->memoryProperties);

   vk_foreach_struct(ext, pMemoryProperties->pNext) {
      switch (ext->sType) {
      case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_BUDGET_PROPERTIES_EXT:
         anv_get_memory_budget(physicalDevice, (void*)ext);
         break;
      default:
         anv_debug_ignored_stype(ext->sType);
         break;
      }
   }
}

PFN_vkVoidFunction anv_GetInstanceProcAddr(
    VkInstance                                  _instance,
    const char*                                 pName)
{
   ANV_FROM_HANDLE(anv_instance, instance, _instance);
   return vk_instance_get_proc_addr(&instance->vk,
                                    &anv_instance_entrypoints,
                                    pName);
}

/* With version 1+ of the loader interface the ICD should expose
 * vk_icdGetInstanceProcAddr to work around certain LD_PRELOAD issues seen in apps.
 */
PUBLIC
VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vk_icdGetInstanceProcAddr(
    VkInstance                                  instance,
    const char*                                 pName)
{
   return anv_GetInstanceProcAddr(instance, pName);
}

static struct anv_state
anv_state_pool_emit_data(struct anv_state_pool *pool, size_t size, size_t align, const void *p)
{
   struct anv_state state;

   state = anv_state_pool_alloc(pool, size, align);
   memcpy(state.map, p, size);

   return state;
}

static void
anv_device_init_border_colors(struct anv_device *device)
{
   static const struct gfx8_border_color border_colors[] = {
      [VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK] =  { .float32 = { 0.0, 0.0, 0.0, 0.0 } },
      [VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK] =       { .float32 = { 0.0, 0.0, 0.0, 1.0 } },
      [VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE] =       { .float32 = { 1.0, 1.0, 1.0, 1.0 } },
      [VK_BORDER_COLOR_INT_TRANSPARENT_BLACK] =    { .uint32 = { 0, 0, 0, 0 } },
      [VK_BORDER_COLOR_INT_OPAQUE_BLACK] =         { .uint32 = { 0, 0, 0, 1 } },
      [VK_BORDER_COLOR_INT_OPAQUE_WHITE] =         { .uint32 = { 1, 1, 1, 1 } },
   };

   device->border_colors =
      anv_state_pool_emit_data(&device->dynamic_state_pool,
                               sizeof(border_colors), 64, border_colors);
}

static VkResult
anv_device_init_trivial_batch(struct anv_device *device)
{
   VkResult result = anv_device_alloc_bo(device, "trivial-batch", 4096,
                                         ANV_BO_ALLOC_MAPPED |
                                         ANV_BO_ALLOC_HOST_COHERENT,
                                         0 /* explicit_address */,
                                         &device->trivial_batch_bo);
   if (result != VK_SUCCESS)
      return result;

   struct anv_batch batch = {
      .start = device->trivial_batch_bo->map,
      .next = device->trivial_batch_bo->map,
      .end = device->trivial_batch_bo->map + 4096,
   };

   anv_batch_emit(&batch, GFX7_MI_BATCH_BUFFER_END, bbe);
   anv_batch_emit(&batch, GFX7_MI_NOOP, noop);

   return VK_SUCCESS;
}

static bool
get_bo_from_pool(struct intel_batch_decode_bo *ret,
                 struct anv_block_pool *pool,
                 uint64_t address)
{
   anv_block_pool_foreach_bo(bo, pool) {
      uint64_t bo_address = intel_48b_address(bo->offset);
      if (address >= bo_address && address < (bo_address + bo->size)) {
         *ret = (struct intel_batch_decode_bo) {
            .addr = bo_address,
            .size = bo->size,
            .map = bo->map,
         };
         return true;
      }
   }
   return false;
}

/* Finding a buffer for batch decoding */
static struct intel_batch_decode_bo
decode_get_bo(void *v_batch, bool ppgtt, uint64_t address)
{
   struct anv_device *device = v_batch;
   struct intel_batch_decode_bo ret_bo = {};

   assert(ppgtt);

   if (get_bo_from_pool(&ret_bo, &device->dynamic_state_pool.block_pool, address))
      return ret_bo;
   if (get_bo_from_pool(&ret_bo, &device->instruction_state_pool.block_pool, address))
      return ret_bo;
   if (get_bo_from_pool(&ret_bo, &device->binding_table_pool.block_pool, address))
      return ret_bo;
   if (get_bo_from_pool(&ret_bo, &device->scratch_surface_state_pool.block_pool, address))
      return ret_bo;
   if (device->physical->indirect_descriptors &&
       get_bo_from_pool(&ret_bo, &device->bindless_surface_state_pool.block_pool, address))
      return ret_bo;
   if (get_bo_from_pool(&ret_bo, &device->internal_surface_state_pool.block_pool, address))
      return ret_bo;
   if (device->physical->indirect_descriptors &&
       get_bo_from_pool(&ret_bo, &device->indirect_push_descriptor_pool.block_pool, address))
      return ret_bo;

   if (!device->cmd_buffer_being_decoded)
      return (struct intel_batch_decode_bo) { };

   struct anv_batch_bo **bbo;
   u_vector_foreach(bbo, &device->cmd_buffer_being_decoded->seen_bbos) {
      /* The decoder zeroes out the top 16 bits, so we need to as well */
      uint64_t bo_address = (*bbo)->bo->offset & (~0ull >> 16);

      if (address >= bo_address && address < bo_address + (*bbo)->bo->size) {
         return (struct intel_batch_decode_bo) {
            .addr = bo_address,
            .size = (*bbo)->bo->size,
            .map = (*bbo)->bo->map,
         };
      }

      uint32_t dep_words = (*bbo)->relocs.dep_words;
      BITSET_WORD *deps = (*bbo)->relocs.deps;
      for (uint32_t w = 0; w < dep_words; w++) {
         BITSET_WORD mask = deps[w];
         while (mask) {
            int i = u_bit_scan(&mask);
            uint32_t gem_handle = w * BITSET_WORDBITS + i;
            struct anv_bo *bo = anv_device_lookup_bo(device, gem_handle);
            assert(bo->refcount > 0);
            bo_address = bo->offset & (~0ull >> 16);
            if (address >= bo_address && address < bo_address + bo->size) {
               return (struct intel_batch_decode_bo) {
                  .addr = bo_address,
                  .size = bo->size,
                  .map = bo->map,
               };
            }
         }
      }
   }

   return (struct intel_batch_decode_bo) { };
}

struct intel_aux_map_buffer {
   struct intel_buffer base;
   struct anv_state state;
};

static struct intel_buffer *
intel_aux_map_buffer_alloc(void *driver_ctx, uint32_t size)
{
   struct intel_aux_map_buffer *buf = malloc(sizeof(struct intel_aux_map_buffer));
   if (!buf)
      return NULL;

   struct anv_device *device = (struct anv_device*)driver_ctx;

   struct anv_state_pool *pool = &device->dynamic_state_pool;
   buf->state = anv_state_pool_alloc(pool, size, size);

   buf->base.gpu = pool->block_pool.bo->offset + buf->state.offset;
   buf->base.gpu_end = buf->base.gpu + buf->state.alloc_size;
   buf->base.map = buf->state.map;
   buf->base.driver_bo = &buf->state;
   return &buf->base;
}

static void
intel_aux_map_buffer_free(void *driver_ctx, struct intel_buffer *buffer)
{
   struct intel_aux_map_buffer *buf = (struct intel_aux_map_buffer*)buffer;
   struct anv_device *device = (struct anv_device*)driver_ctx;
   struct anv_state_pool *pool = &device->dynamic_state_pool;
   anv_state_pool_free(pool, buf->state);
   free(buf);
}

static struct intel_mapped_pinned_buffer_alloc aux_map_allocator = {
   .alloc = intel_aux_map_buffer_alloc,
   .free = intel_aux_map_buffer_free,
};

static VkResult
anv_device_setup_context_or_vm(struct anv_device *device,
                               const VkDeviceCreateInfo *pCreateInfo,
                               const uint32_t num_queues)
{
   switch (device->info->kmd_type) {
   case INTEL_KMD_TYPE_I915:
      return anv_i915_device_setup_context(device, pCreateInfo, num_queues);
   case INTEL_KMD_TYPE_XE:
      return anv_xe_device_setup_vm(device);
   default:
      unreachable("Missing");
      return VK_ERROR_UNKNOWN;
   }
}

static bool
anv_device_destroy_context_or_vm(struct anv_device *device)
{
   switch (device->info->kmd_type) {
   case INTEL_KMD_TYPE_I915:
      if (device->physical->has_vm_control)
         return anv_i915_device_destroy_vm(device);
      else
         return intel_gem_destroy_context(device->fd, device->context_id);
   case INTEL_KMD_TYPE_XE:
      return anv_xe_device_destroy_vm(device);
   default:
      unreachable("Missing");
      return false;
   }
}

static VkResult
anv_device_init_trtt(struct anv_device *device)
{
   struct anv_trtt *trtt = &device->trtt;

   if (pthread_mutex_init(&trtt->mutex, NULL) != 0)
      return vk_error(device, VK_ERROR_INITIALIZATION_FAILED);

   list_inithead(&trtt->in_flight_batches);

   return VK_SUCCESS;
}

static void
anv_device_finish_trtt(struct anv_device *device)
{
   struct anv_trtt *trtt = &device->trtt;

   if (trtt->timeline_val > 0) {
      struct drm_syncobj_timeline_wait wait = {
         .handles = (uintptr_t)&trtt->timeline_handle,
         .points = (uintptr_t)&trtt->timeline_val,
         .timeout_nsec = INT64_MAX,
         .count_handles = 1,
         .flags = DRM_SYNCOBJ_WAIT_FLAGS_WAIT_ALL,
         .first_signaled = false,
      };
      if (intel_ioctl(device->fd, DRM_IOCTL_SYNCOBJ_TIMELINE_WAIT, &wait))
         fprintf(stderr, "TR-TT syncobj wait failed!\n");

      list_for_each_entry_safe(struct anv_trtt_batch_bo, trtt_bbo,
                               &trtt->in_flight_batches, link)
         anv_trtt_batch_bo_free(device, trtt_bbo);

   }

   if (trtt->timeline_handle > 0) {
      struct drm_syncobj_destroy destroy = {
         .handle = trtt->timeline_handle,
      };
      if (intel_ioctl(device->fd, DRM_IOCTL_SYNCOBJ_DESTROY, &destroy))
         fprintf(stderr, "TR-TT syncobj destroy failed!\n");
   }

   pthread_mutex_destroy(&trtt->mutex);

   vk_free(&device->vk.alloc, trtt->l3_mirror);
   vk_free(&device->vk.alloc, trtt->l2_mirror);

   for (int i = 0; i < trtt->num_page_table_bos; i++)
      anv_device_release_bo(device, trtt->page_table_bos[i]);

   vk_free(&device->vk.alloc, trtt->page_table_bos);
}

VkResult anv_CreateDevice(
    VkPhysicalDevice                            physicalDevice,
    const VkDeviceCreateInfo*                   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDevice*                                   pDevice)
{
   ANV_FROM_HANDLE(anv_physical_device, physical_device, physicalDevice);
   VkResult result;
   struct anv_device *device;

   assert(pCreateInfo->sType == VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO);

   /* Check requested queues and fail if we are requested to create any
    * queues with flags we don't support.
    */
   assert(pCreateInfo->queueCreateInfoCount > 0);
   for (uint32_t i = 0; i < pCreateInfo->queueCreateInfoCount; i++) {
      if (pCreateInfo->pQueueCreateInfos[i].flags & ~VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT)
         return vk_error(physical_device, VK_ERROR_INITIALIZATION_FAILED);
   }

   device = vk_zalloc2(&physical_device->instance->vk.alloc, pAllocator,
                       sizeof(*device), 8,
                       VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
   if (!device)
      return vk_error(physical_device, VK_ERROR_OUT_OF_HOST_MEMORY);

   struct vk_device_dispatch_table dispatch_table;

   bool override_initial_entrypoints = true;
   if (physical_device->instance->vk.app_info.app_name &&
       !strcmp(physical_device->instance->vk.app_info.app_name, "HITMAN3.exe")) {
      vk_device_dispatch_table_from_entrypoints(&dispatch_table, &hitman3_device_entrypoints, true);
      override_initial_entrypoints = false;
   }
   if (physical_device->info.ver < 12 &&
       physical_device->instance->vk.app_info.app_name &&
       !strcmp(physical_device->instance->vk.app_info.app_name, "DOOM 64")) {
      vk_device_dispatch_table_from_entrypoints(&dispatch_table, &doom64_device_entrypoints, true);
      override_initial_entrypoints = false;
   }
#ifdef ANDROID
   vk_device_dispatch_table_from_entrypoints(&dispatch_table, &android_device_entrypoints, true);
   override_initial_entrypoints = false;
#endif
   vk_device_dispatch_table_from_entrypoints(&dispatch_table,
      anv_genX(&physical_device->info, device_entrypoints),
      override_initial_entrypoints);
   vk_device_dispatch_table_from_entrypoints(&dispatch_table,
      &anv_device_entrypoints, false);
   vk_device_dispatch_table_from_entrypoints(&dispatch_table,
      &wsi_device_entrypoints, false);

   result = vk_device_init(&device->vk, &physical_device->vk,
                           &dispatch_table, pCreateInfo, pAllocator);
   if (result != VK_SUCCESS)
      goto fail_alloc;

   if (INTEL_DEBUG(DEBUG_BATCH | DEBUG_BATCH_STATS)) {
      for (unsigned i = 0; i < physical_device->queue.family_count; i++) {
         struct intel_batch_decode_ctx *decoder = &device->decoder[i];

         const unsigned decode_flags = INTEL_BATCH_DECODE_DEFAULT_FLAGS;

         intel_batch_decode_ctx_init(decoder,
                                     &physical_device->compiler->isa,
                                     &physical_device->info,
                                     stderr, decode_flags, NULL,
                                     decode_get_bo, NULL, device);
         intel_batch_stats_reset(decoder);

         decoder->engine = physical_device->queue.families[i].engine_class;
         decoder->dynamic_base = physical_device->va.dynamic_state_pool.addr;
         decoder->surface_base = physical_device->va.internal_surface_state_pool.addr;
         decoder->instruction_base = physical_device->va.instruction_state_pool.addr;
      }
   }

   anv_device_set_physical(device, physical_device);
   device->kmd_backend = anv_kmd_backend_get(device->info->kmd_type);

   /* XXX(chadv): Can we dup() physicalDevice->fd here? */
   device->fd = open(physical_device->path, O_RDWR | O_CLOEXEC);
   if (device->fd == -1) {
      result = vk_error(device, VK_ERROR_INITIALIZATION_FAILED);
      goto fail_device;
   }

   switch (device->info->kmd_type) {
   case INTEL_KMD_TYPE_I915:
      device->vk.check_status = anv_i915_device_check_status;
      break;
   case INTEL_KMD_TYPE_XE:
      device->vk.check_status = anv_xe_device_check_status;
      break;
   default:
      unreachable("Missing");
   }

   device->vk.command_buffer_ops = &anv_cmd_buffer_ops;
   device->vk.create_sync_for_memory = anv_create_sync_for_memory;
   if (physical_device->info.kmd_type == INTEL_KMD_TYPE_I915)
      device->vk.create_sync_for_memory = anv_create_sync_for_memory;
   vk_device_set_drm_fd(&device->vk, device->fd);

   uint32_t num_queues = 0;
   for (uint32_t i = 0; i < pCreateInfo->queueCreateInfoCount; i++)
      num_queues += pCreateInfo->pQueueCreateInfos[i].queueCount;

   result = anv_device_setup_context_or_vm(device, pCreateInfo, num_queues);
   if (result != VK_SUCCESS)
      goto fail_fd;

   device->queues =
      vk_zalloc(&device->vk.alloc, num_queues * sizeof(*device->queues), 8,
                VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
   if (device->queues == NULL) {
      result = vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);
      goto fail_context_id;
   }

   device->queue_count = 0;
   for (uint32_t i = 0; i < pCreateInfo->queueCreateInfoCount; i++) {
      const VkDeviceQueueCreateInfo *queueCreateInfo =
         &pCreateInfo->pQueueCreateInfos[i];

      for (uint32_t j = 0; j < queueCreateInfo->queueCount; j++) {
         result = anv_queue_init(device, &device->queues[device->queue_count],
                                 queueCreateInfo, j);
         if (result != VK_SUCCESS)
            goto fail_queues;

         device->queue_count++;
      }
   }

   if (pthread_mutex_init(&device->vma_mutex, NULL) != 0) {
      result = vk_error(device, VK_ERROR_INITIALIZATION_FAILED);
      goto fail_queues;
   }

   /* keep the page with address zero out of the allocator */
   util_vma_heap_init(&device->vma_lo,
                      device->physical->va.low_heap.addr,
                      device->physical->va.low_heap.size);

   util_vma_heap_init(&device->vma_hi,
                      device->physical->va.high_heap.addr,
                      device->physical->va.high_heap.size);

   if (device->physical->indirect_descriptors) {
      util_vma_heap_init(&device->vma_desc,
                         device->physical->va.indirect_descriptor_pool.addr,
                         device->physical->va.indirect_descriptor_pool.size);
   } else {
      util_vma_heap_init(&device->vma_desc,
                         device->physical->va.bindless_surface_state_pool.addr,
                         device->physical->va.bindless_surface_state_pool.size);
   }

   util_vma_heap_init(&device->vma_samplers,
                      device->physical->va.sampler_state_pool.addr,
                      device->physical->va.sampler_state_pool.size);
   util_vma_heap_init(&device->vma_trtt,
                      device->physical->va.trtt.addr,
                      device->physical->va.trtt.size);

   list_inithead(&device->memory_objects);
   list_inithead(&device->image_private_objects);

   if (pthread_mutex_init(&device->mutex, NULL) != 0) {
      result = vk_error(device, VK_ERROR_INITIALIZATION_FAILED);
      goto fail_vmas;
   }

   pthread_condattr_t condattr;
   if (pthread_condattr_init(&condattr) != 0) {
      result = vk_error(device, VK_ERROR_INITIALIZATION_FAILED);
      goto fail_mutex;
   }
   if (pthread_condattr_setclock(&condattr, CLOCK_MONOTONIC) != 0) {
      pthread_condattr_destroy(&condattr);
      result = vk_error(device, VK_ERROR_INITIALIZATION_FAILED);
      goto fail_mutex;
   }
   if (pthread_cond_init(&device->queue_submit, &condattr) != 0) {
      pthread_condattr_destroy(&condattr);
      result = vk_error(device, VK_ERROR_INITIALIZATION_FAILED);
      goto fail_mutex;
   }
   pthread_condattr_destroy(&condattr);

   result = anv_bo_cache_init(&device->bo_cache, device);
   if (result != VK_SUCCESS)
      goto fail_queue_cond;

   anv_bo_pool_init(&device->batch_bo_pool, device, "batch",
                    ANV_BO_ALLOC_MAPPED |
                    ANV_BO_ALLOC_HOST_CACHED_COHERENT |
                    ANV_BO_ALLOC_CAPTURE);
   if (device->vk.enabled_extensions.KHR_acceleration_structure) {
      anv_bo_pool_init(&device->bvh_bo_pool, device, "bvh build",
                       0 /* alloc_flags */);
   }

   /* Because scratch is also relative to General State Base Address, we leave
    * the base address 0 and start the pool memory at an offset.  This way we
    * get the correct offsets in the anv_states that get allocated from it.
    */
   result = anv_state_pool_init(&device->general_state_pool, device,
                                &(struct anv_state_pool_params) {
                                   .name         = "general pool",
                                   .base_address = 0,
                                   .start_offset = device->physical->va.general_state_pool.addr,
                                   .block_size   = 16384,
                                   .max_size     = device->physical->va.general_state_pool.size
                                });
   if (result != VK_SUCCESS)
      goto fail_batch_bo_pool;

   result = anv_state_pool_init(&device->dynamic_state_pool, device,
                                &(struct anv_state_pool_params) {
                                   .name         = "dynamic pool",
                                   .base_address = device->physical->va.dynamic_state_pool.addr,
                                   .block_size   = 16384,
                                   .max_size     = device->physical->va.dynamic_state_pool.size,
                                });
   if (result != VK_SUCCESS)
      goto fail_general_state_pool;

   /* The border color pointer is limited to 24 bits, so we need to make
    * sure that any such color used at any point in the program doesn't
    * exceed that limit.
    * We achieve that by reserving all the custom border colors we support
    * right off the bat, so they are close to the base address.
    */
   anv_state_reserved_pool_init(&device->custom_border_colors,
                                &device->dynamic_state_pool,
                                MAX_CUSTOM_BORDER_COLORS,
                                sizeof(struct gfx8_border_color), 64);

   result = anv_state_pool_init(&device->instruction_state_pool, device,
                                &(struct anv_state_pool_params) {
                                   .name         = "instruction pool",
                                   .base_address = device->physical->va.instruction_state_pool.addr,
                                   .block_size   = 16384,
                                   .max_size     = device->physical->va.instruction_state_pool.size,
                                });
   if (result != VK_SUCCESS)
      goto fail_dynamic_state_pool;

   if (device->info->verx10 >= 125) {
      /* Put the scratch surface states at the beginning of the internal
       * surface state pool.
       */
      result = anv_state_pool_init(&device->scratch_surface_state_pool, device,
                                   &(struct anv_state_pool_params) {
                                      .name         = "scratch surface state pool",
                                      .base_address = device->physical->va.scratch_surface_state_pool.addr,
                                      .block_size   = 4096,
                                      .max_size     = device->physical->va.scratch_surface_state_pool.size,
                                   });
      if (result != VK_SUCCESS)
         goto fail_instruction_state_pool;

      result = anv_state_pool_init(&device->internal_surface_state_pool, device,
                                   &(struct anv_state_pool_params) {
                                      .name         = "internal surface state pool",
                                      .base_address = device->physical->va.internal_surface_state_pool.addr,
                                      .start_offset = device->physical->va.scratch_surface_state_pool.size,
                                      .block_size   = 4096,
                                      .max_size     = device->physical->va.internal_surface_state_pool.size,
                                   });
   } else {
      result = anv_state_pool_init(&device->internal_surface_state_pool, device,
                                   &(struct anv_state_pool_params) {
                                      .name         = "internal surface state pool",
                                      .base_address = device->physical->va.internal_surface_state_pool.addr,
                                      .block_size   = 4096,
                                      .max_size     = device->physical->va.internal_surface_state_pool.size,
                                   });
   }
   if (result != VK_SUCCESS)
      goto fail_scratch_surface_state_pool;

   if (device->physical->indirect_descriptors) {
      result = anv_state_pool_init(&device->bindless_surface_state_pool, device,
                                   &(struct anv_state_pool_params) {
                                      .name         = "bindless surface state pool",
                                      .base_address = device->physical->va.bindless_surface_state_pool.addr,
                                      .block_size   = 4096,
                                      .max_size     = device->physical->va.bindless_surface_state_pool.size,
                                   });
      if (result != VK_SUCCESS)
         goto fail_internal_surface_state_pool;
   }

   if (device->info->verx10 >= 125) {
      /* We're using 3DSTATE_BINDING_TABLE_POOL_ALLOC to give the binding
       * table its own base address separately from surface state base.
       */
      result = anv_state_pool_init(&device->binding_table_pool, device,
                                   &(struct anv_state_pool_params) {
                                      .name         = "binding table pool",
                                      .base_address = device->physical->va.binding_table_pool.addr,
                                      .block_size   = BINDING_TABLE_POOL_BLOCK_SIZE,
                                      .max_size     = device->physical->va.binding_table_pool.size,
                                   });
   } else {
      /* The binding table should be in front of the surface states in virtual
       * address space so that all surface states can be express as relative
       * offsets from the binding table location.
       */
      assert(device->physical->va.binding_table_pool.addr <
             device->physical->va.internal_surface_state_pool.addr);
      int64_t bt_pool_offset = (int64_t)device->physical->va.binding_table_pool.addr -
                               (int64_t)device->physical->va.internal_surface_state_pool.addr;
      assert(INT32_MIN < bt_pool_offset && bt_pool_offset < 0);
      result = anv_state_pool_init(&device->binding_table_pool, device,
                                   &(struct anv_state_pool_params) {
                                      .name         = "binding table pool",
                                      .base_address = device->physical->va.internal_surface_state_pool.addr,
                                      .start_offset = bt_pool_offset,
                                      .block_size   = BINDING_TABLE_POOL_BLOCK_SIZE,
                                      .max_size     = device->physical->va.internal_surface_state_pool.size,
                                   });
   }
   if (result != VK_SUCCESS)
      goto fail_bindless_surface_state_pool;

   if (device->physical->indirect_descriptors) {
      result = anv_state_pool_init(&device->indirect_push_descriptor_pool, device,
                                   &(struct anv_state_pool_params) {
                                      .name         = "indirect push descriptor pool",
                                      .base_address = device->physical->va.indirect_push_descriptor_pool.addr,
                                      .block_size   = 4096,
                                      .max_size     = device->physical->va.indirect_push_descriptor_pool.size,
                                   });
      if (result != VK_SUCCESS)
         goto fail_binding_table_pool;
   }

   if (device->info->has_aux_map) {
      device->aux_map_ctx = intel_aux_map_init(device, &aux_map_allocator,
                                               &physical_device->info);
      if (!device->aux_map_ctx)
         goto fail_indirect_push_descriptor_pool;
   }

   result = anv_device_alloc_bo(device, "workaround", 8192,
                                ANV_BO_ALLOC_CAPTURE |
                                ANV_BO_ALLOC_HOST_COHERENT |
                                ANV_BO_ALLOC_MAPPED,
                                0 /* explicit_address */,
                                &device->workaround_bo);
   if (result != VK_SUCCESS)
      goto fail_surface_aux_map_pool;

   device->workaround_address = (struct anv_address) {
      .bo = device->workaround_bo,
      .offset = align(intel_debug_write_identifiers(device->workaround_bo->map,
                                                    device->workaround_bo->size,
                                                    "Anv"), 32),
   };

   device->workarounds.doom64_images = NULL;

   device->rt_uuid_addr = anv_address_add(device->workaround_address, 8);
   memcpy(device->rt_uuid_addr.bo->map + device->rt_uuid_addr.offset,
          physical_device->rt_uuid,
          sizeof(physical_device->rt_uuid));

   device->debug_frame_desc =
      intel_debug_get_identifier_block(device->workaround_bo->map,
                                       device->workaround_bo->size,
                                       INTEL_DEBUG_BLOCK_TYPE_FRAME);

   if (device->vk.enabled_extensions.KHR_ray_query) {
      uint32_t ray_queries_size =
         align(brw_rt_ray_queries_hw_stacks_size(device->info), 4096);

      result = anv_device_alloc_bo(device, "ray queries",
                                   ray_queries_size,
                                   0,
                                   0 /* explicit_address */,
                                   &device->ray_query_bo);
      if (result != VK_SUCCESS)
         goto fail_workaround_bo;
   }

   result = anv_device_init_trivial_batch(device);
   if (result != VK_SUCCESS)
      goto fail_ray_query_bo;

   /* Emit the CPS states before running the initialization batch as those
    * structures are referenced.
    */
   if (device->info->ver >= 12) {
      uint32_t n_cps_states = 3 * 3; /* All combinaisons of X by Y CP sizes (1, 2, 4) */

      if (device->info->has_coarse_pixel_primitive_and_cb)
         n_cps_states *= 5 * 5; /* 5 combiners by 2 operators */

      n_cps_states += 1; /* Disable CPS */

       /* Each of the combinaison must be replicated on all viewports */
      n_cps_states *= MAX_VIEWPORTS;

      device->cps_states =
         anv_state_pool_alloc(&device->dynamic_state_pool,
                              n_cps_states * CPS_STATE_length(device->info) * 4,
                              32);
      if (device->cps_states.map == NULL)
         goto fail_trivial_batch;

      anv_genX(device->info, init_cps_device_state)(device);
   }

   if (device->physical->indirect_descriptors) {
      /* Allocate a null surface state at surface state offset 0. This makes
       * NULL descriptor handling trivial because we can just memset
       * structures to zero and they have a valid descriptor.
       */
      device->null_surface_state =
         anv_state_pool_alloc(&device->bindless_surface_state_pool,
                              device->isl_dev.ss.size,
                              device->isl_dev.ss.align);
      isl_null_fill_state(&device->isl_dev, device->null_surface_state.map,
                          .size = isl_extent3d(1, 1, 1) /* This shouldn't matter */);
      assert(device->null_surface_state.offset == 0);
   } else {
      /* When using direct descriptors, those can hold the null surface state
       * directly. We still need a null surface for the binding table entries
       * though but this one can live anywhere the internal surface state
       * pool.
       */
      device->null_surface_state =
         anv_state_pool_alloc(&device->internal_surface_state_pool,
                              device->isl_dev.ss.size,
                              device->isl_dev.ss.align);
      isl_null_fill_state(&device->isl_dev, device->null_surface_state.map,
                          .size = isl_extent3d(1, 1, 1) /* This shouldn't matter */);
   }

   anv_scratch_pool_init(device, &device->scratch_pool);

   /* TODO(RT): Do we want some sort of data structure for this? */
   memset(device->rt_scratch_bos, 0, sizeof(device->rt_scratch_bos));

   if (ANV_SUPPORT_RT && device->info->has_ray_tracing) {
      /* The docs say to always allocate 128KB per DSS */
      const uint32_t btd_fifo_bo_size =
         128 * 1024 * intel_device_info_dual_subslice_id_bound(device->info);
      result = anv_device_alloc_bo(device,
                                   "rt-btd-fifo",
                                   btd_fifo_bo_size,
                                   0 /* alloc_flags */,
                                   0 /* explicit_address */,
                                   &device->btd_fifo_bo);
      if (result != VK_SUCCESS)
         goto fail_trivial_batch_bo_and_scratch_pool;
   }

   result = anv_device_init_trtt(device);
   if (result != VK_SUCCESS)
      goto fail_btd_fifo_bo;

   result = anv_genX(device->info, init_device_state)(device);
   if (result != VK_SUCCESS)
      goto fail_trtt;

   struct vk_pipeline_cache_create_info pcc_info = { };
   device->default_pipeline_cache =
      vk_pipeline_cache_create(&device->vk, &pcc_info, NULL);
   if (!device->default_pipeline_cache) {
      result = vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);
      goto fail_trtt;
   }

   /* Internal shaders need their own pipeline cache because, unlike the rest
    * of ANV, it won't work at all without the cache. It depends on it for
    * shaders to remain resident while it runs. Therefore, we need a special
    * cache just for BLORP/RT that's forced to always be enabled.
    */
   pcc_info.force_enable = true;
   device->internal_cache =
      vk_pipeline_cache_create(&device->vk, &pcc_info, NULL);
   if (device->internal_cache == NULL) {
      result = vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);
      goto fail_default_pipeline_cache;
   }

   /* The device (currently is ICL/TGL) does not have float64 support. */
   if (!device->info->has_64bit_float &&
      device->physical->instance->fp64_workaround_enabled)
      anv_load_fp64_shader(device);

   result = anv_device_init_rt_shaders(device);
   if (result != VK_SUCCESS) {
      result = vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);
      goto fail_internal_cache;
   }

#ifdef ANDROID
   device->u_gralloc = u_gralloc_create(U_GRALLOC_TYPE_AUTO);
#endif

   device->robust_buffer_access =
      device->vk.enabled_features.robustBufferAccess ||
      device->vk.enabled_features.nullDescriptor;

   device->breakpoint = anv_state_pool_alloc(&device->dynamic_state_pool, 4,
                                             4);
   p_atomic_set(&device->draw_call_count, 0);

   /* Create a separate command pool for companion RCS command buffer. */
   if (device->info->verx10 >= 125) {
      VkCommandPoolCreateInfo pool_info = {
         .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
         .queueFamilyIndex =
             anv_get_first_render_queue_index(device->physical),
      };

      result = vk_common_CreateCommandPool(anv_device_to_handle(device),
                                           &pool_info, NULL,
                                           &device->companion_rcs_cmd_pool);
      if (result != VK_SUCCESS) {
         goto fail_internal_cache;
      }
   }

   anv_device_init_blorp(device);

   anv_device_init_border_colors(device);

   anv_device_init_internal_kernels(device);

   anv_device_init_astc_emu(device);

   anv_device_perf_init(device);

   anv_device_utrace_init(device);

   BITSET_ONES(device->gfx_dirty_state);
   BITSET_CLEAR(device->gfx_dirty_state, ANV_GFX_STATE_INDEX_BUFFER);
   BITSET_CLEAR(device->gfx_dirty_state, ANV_GFX_STATE_SO_DECL_LIST);
   if (device->info->ver < 11)
      BITSET_CLEAR(device->gfx_dirty_state, ANV_GFX_STATE_VF_SGVS_2);
   if (device->info->ver < 12) {
      BITSET_CLEAR(device->gfx_dirty_state, ANV_GFX_STATE_PRIMITIVE_REPLICATION);
      BITSET_CLEAR(device->gfx_dirty_state, ANV_GFX_STATE_DEPTH_BOUNDS);
   }
   if (!device->vk.enabled_extensions.EXT_sample_locations)
      BITSET_CLEAR(device->gfx_dirty_state, ANV_GFX_STATE_SAMPLE_PATTERN);
   if (!device->vk.enabled_extensions.KHR_fragment_shading_rate)
      BITSET_CLEAR(device->gfx_dirty_state, ANV_GFX_STATE_CPS);
   if (!device->vk.enabled_extensions.EXT_mesh_shader) {
      BITSET_CLEAR(device->gfx_dirty_state, ANV_GFX_STATE_SBE_MESH);
      BITSET_CLEAR(device->gfx_dirty_state, ANV_GFX_STATE_CLIP_MESH);
      BITSET_CLEAR(device->gfx_dirty_state, ANV_GFX_STATE_MESH_CONTROL);
      BITSET_CLEAR(device->gfx_dirty_state, ANV_GFX_STATE_MESH_SHADER);
      BITSET_CLEAR(device->gfx_dirty_state, ANV_GFX_STATE_MESH_DISTRIB);
      BITSET_CLEAR(device->gfx_dirty_state, ANV_GFX_STATE_TASK_CONTROL);
      BITSET_CLEAR(device->gfx_dirty_state, ANV_GFX_STATE_TASK_SHADER);
      BITSET_CLEAR(device->gfx_dirty_state, ANV_GFX_STATE_TASK_REDISTRIB);
   }
   if (!intel_needs_workaround(device->info, 18019816803))
      BITSET_CLEAR(device->gfx_dirty_state, ANV_GFX_STATE_WA_18019816803);
   if (device->info->ver > 9)
      BITSET_CLEAR(device->gfx_dirty_state, ANV_GFX_STATE_PMA_FIX);

   *pDevice = anv_device_to_handle(device);

   return VK_SUCCESS;

 fail_internal_cache:
   vk_pipeline_cache_destroy(device->internal_cache, NULL);
 fail_default_pipeline_cache:
   vk_pipeline_cache_destroy(device->default_pipeline_cache, NULL);
 fail_trtt:
   anv_device_finish_trtt(device);
 fail_btd_fifo_bo:
   if (ANV_SUPPORT_RT && device->info->has_ray_tracing)
      anv_device_release_bo(device, device->btd_fifo_bo);
 fail_trivial_batch_bo_and_scratch_pool:
   anv_scratch_pool_finish(device, &device->scratch_pool);
 fail_trivial_batch:
   anv_device_release_bo(device, device->trivial_batch_bo);
 fail_ray_query_bo:
   if (device->ray_query_bo)
      anv_device_release_bo(device, device->ray_query_bo);
 fail_workaround_bo:
   anv_device_release_bo(device, device->workaround_bo);
 fail_surface_aux_map_pool:
   if (device->info->has_aux_map) {
      intel_aux_map_finish(device->aux_map_ctx);
      device->aux_map_ctx = NULL;
   }
 fail_indirect_push_descriptor_pool:
   if (device->physical->indirect_descriptors)
      anv_state_pool_finish(&device->indirect_push_descriptor_pool);
 fail_binding_table_pool:
   anv_state_pool_finish(&device->binding_table_pool);
 fail_bindless_surface_state_pool:
   if (device->physical->indirect_descriptors)
      anv_state_pool_finish(&device->bindless_surface_state_pool);
 fail_internal_surface_state_pool:
   anv_state_pool_finish(&device->internal_surface_state_pool);
 fail_scratch_surface_state_pool:
   if (device->info->verx10 >= 125)
      anv_state_pool_finish(&device->scratch_surface_state_pool);
 fail_instruction_state_pool:
   anv_state_pool_finish(&device->instruction_state_pool);
 fail_dynamic_state_pool:
   anv_state_reserved_pool_finish(&device->custom_border_colors);
   anv_state_pool_finish(&device->dynamic_state_pool);
 fail_general_state_pool:
   anv_state_pool_finish(&device->general_state_pool);
 fail_batch_bo_pool:
   if (device->vk.enabled_extensions.KHR_acceleration_structure)
      anv_bo_pool_finish(&device->bvh_bo_pool);
   anv_bo_pool_finish(&device->batch_bo_pool);
   anv_bo_cache_finish(&device->bo_cache);
 fail_queue_cond:
   pthread_cond_destroy(&device->queue_submit);
 fail_mutex:
   pthread_mutex_destroy(&device->mutex);
 fail_vmas:
   util_vma_heap_finish(&device->vma_trtt);
   if (!device->physical->indirect_descriptors)
      util_vma_heap_finish(&device->vma_samplers);
   util_vma_heap_finish(&device->vma_desc);
   util_vma_heap_finish(&device->vma_hi);
   util_vma_heap_finish(&device->vma_lo);
   pthread_mutex_destroy(&device->vma_mutex);
 fail_queues:
   for (uint32_t i = 0; i < device->queue_count; i++)
      anv_queue_finish(&device->queues[i]);
   vk_free(&device->vk.alloc, device->queues);
 fail_context_id:
   anv_device_destroy_context_or_vm(device);
 fail_fd:
   close(device->fd);
 fail_device:
   vk_device_finish(&device->vk);
 fail_alloc:
   vk_free(&device->vk.alloc, device);

   return result;
}

void anv_DestroyDevice(
    VkDevice                                    _device,
    const VkAllocationCallbacks*                pAllocator)
{
   ANV_FROM_HANDLE(anv_device, device, _device);

   if (!device)
      return;

#ifdef ANDROID
   u_gralloc_destroy(&device->u_gralloc);
#endif

   struct anv_physical_device *pdevice = device->physical;

   for (uint32_t i = 0; i < device->queue_count; i++)
      anv_queue_finish(&device->queues[i]);
   vk_free(&device->vk.alloc, device->queues);

   anv_device_utrace_finish(device);

   anv_device_finish_blorp(device);

   anv_device_finish_rt_shaders(device);

   anv_device_finish_astc_emu(device);

   anv_device_finish_internal_kernels(device);

   vk_pipeline_cache_destroy(device->internal_cache, NULL);
   vk_pipeline_cache_destroy(device->default_pipeline_cache, NULL);

   anv_device_finish_trtt(device);

   if (ANV_SUPPORT_RT && device->info->has_ray_tracing)
      anv_device_release_bo(device, device->btd_fifo_bo);

   if (device->info->verx10 >= 125) {
      vk_common_DestroyCommandPool(anv_device_to_handle(device),
                                   device->companion_rcs_cmd_pool, NULL);
   }

#ifdef HAVE_VALGRIND
   /* We only need to free these to prevent valgrind errors.  The backing
    * BO will go away in a couple of lines so we don't actually leak.
    */
   anv_state_reserved_pool_finish(&device->custom_border_colors);
   anv_state_pool_free(&device->dynamic_state_pool, device->border_colors);
   anv_state_pool_free(&device->dynamic_state_pool, device->slice_hash);
   anv_state_pool_free(&device->dynamic_state_pool, device->cps_states);
   anv_state_pool_free(&device->dynamic_state_pool, device->breakpoint);
#endif

   for (unsigned i = 0; i < ARRAY_SIZE(device->rt_scratch_bos); i++) {
      if (device->rt_scratch_bos[i] != NULL)
         anv_device_release_bo(device, device->rt_scratch_bos[i]);
   }

   anv_scratch_pool_finish(device, &device->scratch_pool);

   if (device->vk.enabled_extensions.KHR_ray_query) {
      for (unsigned i = 0; i < ARRAY_SIZE(device->ray_query_shadow_bos); i++) {
         if (device->ray_query_shadow_bos[i] != NULL)
            anv_device_release_bo(device, device->ray_query_shadow_bos[i]);
      }
      anv_device_release_bo(device, device->ray_query_bo);
   }
   anv_device_release_bo(device, device->workaround_bo);
   anv_device_release_bo(device, device->trivial_batch_bo);

   if (device->info->has_aux_map) {
      intel_aux_map_finish(device->aux_map_ctx);
      device->aux_map_ctx = NULL;
   }

   if (device->physical->indirect_descriptors)
      anv_state_pool_finish(&device->indirect_push_descriptor_pool);
   anv_state_pool_finish(&device->binding_table_pool);
   if (device->info->verx10 >= 125)
      anv_state_pool_finish(&device->scratch_surface_state_pool);
   anv_state_pool_finish(&device->internal_surface_state_pool);
   if (device->physical->indirect_descriptors)
      anv_state_pool_finish(&device->bindless_surface_state_pool);
   anv_state_pool_finish(&device->instruction_state_pool);
   anv_state_pool_finish(&device->dynamic_state_pool);
   anv_state_pool_finish(&device->general_state_pool);

   if (device->vk.enabled_extensions.KHR_acceleration_structure)
      anv_bo_pool_finish(&device->bvh_bo_pool);
   anv_bo_pool_finish(&device->batch_bo_pool);

   anv_bo_cache_finish(&device->bo_cache);

   util_vma_heap_finish(&device->vma_trtt);
   if (!device->physical->indirect_descriptors)
      util_vma_heap_finish(&device->vma_samplers);
   util_vma_heap_finish(&device->vma_desc);
   util_vma_heap_finish(&device->vma_hi);
   util_vma_heap_finish(&device->vma_lo);
   pthread_mutex_destroy(&device->vma_mutex);

   pthread_cond_destroy(&device->queue_submit);
   pthread_mutex_destroy(&device->mutex);

   ralloc_free(device->fp64_nir);

   anv_device_destroy_context_or_vm(device);

   if (INTEL_DEBUG(DEBUG_BATCH | DEBUG_BATCH_STATS)) {
      for (unsigned i = 0; i < pdevice->queue.family_count; i++) {
         if (INTEL_DEBUG(DEBUG_BATCH_STATS))
            intel_batch_print_stats(&device->decoder[i]);
         intel_batch_decode_ctx_finish(&device->decoder[i]);
      }
   }

   close(device->fd);

   vk_device_finish(&device->vk);
   vk_free(&device->vk.alloc, device);
}

VkResult anv_EnumerateInstanceLayerProperties(
    uint32_t*                                   pPropertyCount,
    VkLayerProperties*                          pProperties)
{
   if (pProperties == NULL) {
      *pPropertyCount = 0;
      return VK_SUCCESS;
   }

   /* None supported at this time */
   return vk_error(NULL, VK_ERROR_LAYER_NOT_PRESENT);
}

VkResult
anv_device_wait(struct anv_device *device, struct anv_bo *bo,
                int64_t timeout)
{
   int ret = anv_gem_wait(device, bo->gem_handle, &timeout);
   if (ret == -1 && errno == ETIME) {
      return VK_TIMEOUT;
   } else if (ret == -1) {
      /* We don't know the real error. */
      return vk_device_set_lost(&device->vk, "gem wait failed: %m");
   } else {
      return VK_SUCCESS;
   }
}

static struct util_vma_heap *
anv_vma_heap_for_flags(struct anv_device *device,
                       enum anv_bo_alloc_flags alloc_flags)
{
   if (alloc_flags & ANV_BO_ALLOC_TRTT)
      return &device->vma_trtt;

   if (alloc_flags & ANV_BO_ALLOC_32BIT_ADDRESS)
      return &device->vma_lo;

   if (alloc_flags & ANV_BO_ALLOC_DESCRIPTOR_POOL)
      return &device->vma_desc;

   if (alloc_flags & ANV_BO_ALLOC_SAMPLER_POOL)
      return &device->vma_samplers;

   return &device->vma_hi;
}

uint64_t
anv_vma_alloc(struct anv_device *device,
              uint64_t size, uint64_t align,
              enum anv_bo_alloc_flags alloc_flags,
              uint64_t client_address,
              struct util_vma_heap **out_vma_heap)
{
   pthread_mutex_lock(&device->vma_mutex);

   uint64_t addr = 0;
   *out_vma_heap = anv_vma_heap_for_flags(device, alloc_flags);

   if (alloc_flags & ANV_BO_ALLOC_CLIENT_VISIBLE_ADDRESS) {
      assert(*out_vma_heap == &device->vma_hi ||
             *out_vma_heap == &device->vma_trtt);

      if (client_address) {
         if (util_vma_heap_alloc_addr(*out_vma_heap,
                                      client_address, size)) {
            addr = client_address;
         }
      } else {
         (*out_vma_heap)->alloc_high = false;
         addr = util_vma_heap_alloc(*out_vma_heap, size, align);
         (*out_vma_heap)->alloc_high = true;
      }
      /* We don't want to fall back to other heaps */
      goto done;
   }

   assert(client_address == 0);

   addr = util_vma_heap_alloc(*out_vma_heap, size, align);

done:
   pthread_mutex_unlock(&device->vma_mutex);

   assert(addr == intel_48b_address(addr));
   return intel_canonical_address(addr);
}

void
anv_vma_free(struct anv_device *device,
             struct util_vma_heap *vma_heap,
             uint64_t address, uint64_t size)
{
   assert(vma_heap == &device->vma_lo ||
          vma_heap == &device->vma_hi ||
          vma_heap == &device->vma_desc ||
          vma_heap == &device->vma_samplers ||
          vma_heap == &device->vma_trtt);

   const uint64_t addr_48b = intel_48b_address(address);

   pthread_mutex_lock(&device->vma_mutex);

   util_vma_heap_free(vma_heap, addr_48b, size);

   pthread_mutex_unlock(&device->vma_mutex);
}

VkResult anv_AllocateMemory(
    VkDevice                                    _device,
    const VkMemoryAllocateInfo*                 pAllocateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDeviceMemory*                             pMem)
{
   ANV_FROM_HANDLE(anv_device, device, _device);
   struct anv_physical_device *pdevice = device->physical;
   struct anv_device_memory *mem;
   VkResult result = VK_SUCCESS;

   assert(pAllocateInfo->sType == VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);

   VkDeviceSize aligned_alloc_size =
      align64(pAllocateInfo->allocationSize, 4096);

   assert(pAllocateInfo->memoryTypeIndex < pdevice->memory.type_count);
   const struct anv_memory_type *mem_type =
      &pdevice->memory.types[pAllocateInfo->memoryTypeIndex];
   assert(mem_type->heapIndex < pdevice->memory.heap_count);
   struct anv_memory_heap *mem_heap =
      &pdevice->memory.heaps[mem_type->heapIndex];

   if (aligned_alloc_size > mem_heap->size)
      return vk_error(device, VK_ERROR_OUT_OF_DEVICE_MEMORY);

   uint64_t mem_heap_used = p_atomic_read(&mem_heap->used);
   if (mem_heap_used + aligned_alloc_size > mem_heap->size)
      return vk_error(device, VK_ERROR_OUT_OF_DEVICE_MEMORY);

   mem = vk_device_memory_create(&device->vk, pAllocateInfo,
                                 pAllocator, sizeof(*mem));
   if (mem == NULL)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   mem->type = mem_type;
   mem->map = NULL;
   mem->map_size = 0;
   mem->map_delta = 0;

   enum anv_bo_alloc_flags alloc_flags = 0;

   const VkImportMemoryFdInfoKHR *fd_info = NULL;
   const VkMemoryDedicatedAllocateInfo *dedicated_info = NULL;
   uint64_t client_address = 0;

   vk_foreach_struct_const(ext, pAllocateInfo->pNext) {
      switch (ext->sType) {
      case VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO:
      case VK_STRUCTURE_TYPE_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID:
      case VK_STRUCTURE_TYPE_IMPORT_MEMORY_HOST_POINTER_INFO_EXT:
      case VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR:
      case VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO:
         /* handled by vk_device_memory_create */
         break;

      case VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR:
         fd_info = (void *)ext;
         break;

      case VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO:
         dedicated_info = (void *)ext;
         break;

      case VK_STRUCTURE_TYPE_MEMORY_OPAQUE_CAPTURE_ADDRESS_ALLOCATE_INFO: {
         const VkMemoryOpaqueCaptureAddressAllocateInfo *addr_info =
            (const VkMemoryOpaqueCaptureAddressAllocateInfo *)ext;
         client_address = addr_info->opaqueCaptureAddress;
         break;
      }

      default:
         /* VK_STRUCTURE_TYPE_WSI_MEMORY_ALLOCATE_INFO_MESA isn't a real
          * enum value, so use conditional to avoid compiler warn
          */
         if (ext->sType == VK_STRUCTURE_TYPE_WSI_MEMORY_ALLOCATE_INFO_MESA) {
            /* TODO: Android, ChromeOS and other applications may need another
             * way to allocate buffers that can be scanout to display but it
             * should pretty easy to catch those as Xe KMD driver will print
             * warnings in dmesg when scanning buffers allocated without
             * proper flag set.
             */
            alloc_flags |= ANV_BO_ALLOC_SCANOUT;
         } else {
            anv_debug_ignored_stype(ext->sType);
         }
         break;
      }
   }

   /* If i915 reported a mappable/non_mappable vram regions and the
    * application want lmem mappable, then we need to use the
    * I915_GEM_CREATE_EXT_FLAG_NEEDS_CPU_ACCESS flag to create our BO.
    */
   if (pdevice->vram_mappable.size > 0 &&
       pdevice->vram_non_mappable.size > 0 &&
       (mem_type->propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) &&
       (mem_type->propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
      alloc_flags |= ANV_BO_ALLOC_LOCAL_MEM_CPU_VISIBLE;

   if (!mem_heap->is_local_mem)
      alloc_flags |= ANV_BO_ALLOC_NO_LOCAL_MEM;

   if (mem->vk.alloc_flags & VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT)
      alloc_flags |= ANV_BO_ALLOC_CLIENT_VISIBLE_ADDRESS;

   if (mem->vk.alloc_flags & VK_MEMORY_PROPERTY_PROTECTED_BIT)
      alloc_flags |= ANV_BO_ALLOC_PROTECTED;

   /* For now, always allocated AUX-TT aligned memory, regardless of dedicated
    * allocations. An application can for example, suballocate a large
    * VkDeviceMemory and try to bind an image created with a CCS modifier. In
    * that case we cannot disable CCS if the alignment doesn´t meet the AUX-TT
    * requirements, so we need to ensure both the VkDeviceMemory and the
    * alignment reported through vkGetImageMemoryRequirements() meet the
    * AUX-TT requirement.
    *
    * TODO: when we enable EXT_descriptor_buffer, we'll be able to drop the
    * AUX-TT alignment for that type of allocation.
    */
   if (device->info->has_aux_map)
      alloc_flags |= ANV_BO_ALLOC_AUX_TT_ALIGNED;

   /* Anything imported or exported is EXTERNAL. Apply implicit sync to be
    * compatible with clients relying on implicit fencing. This matches the
    * behavior in iris i915_batch_submit. An example client is VA-API.
    */
   if (mem->vk.export_handle_types || mem->vk.import_handle_type)
      alloc_flags |= (ANV_BO_ALLOC_EXTERNAL | ANV_BO_ALLOC_IMPLICIT_SYNC);

   if (mem->vk.ahardware_buffer) {
      result = anv_import_ahw_memory(_device, mem);
      if (result != VK_SUCCESS)
         goto fail;

      goto success;
   }

   /* The Vulkan spec permits handleType to be 0, in which case the struct is
    * ignored.
    */
   if (fd_info && fd_info->handleType) {
      /* At the moment, we support only the below handle types. */
      assert(fd_info->handleType ==
               VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT ||
             fd_info->handleType ==
               VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT);

      result = anv_device_import_bo(device, fd_info->fd, alloc_flags,
                                    client_address, &mem->bo);
      if (result != VK_SUCCESS)
         goto fail;

      /* For security purposes, we reject importing the bo if it's smaller
       * than the requested allocation size.  This prevents a malicious client
       * from passing a buffer to a trusted client, lying about the size, and
       * telling the trusted client to try and texture from an image that goes
       * out-of-bounds.  This sort of thing could lead to GPU hangs or worse
       * in the trusted client.  The trusted client can protect itself against
       * this sort of attack but only if it can trust the buffer size.
       */
      if (mem->bo->size < aligned_alloc_size) {
         result = vk_errorf(device, VK_ERROR_INVALID_EXTERNAL_HANDLE,
                            "aligned allocationSize too large for "
                            "VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT: "
                            "%"PRIu64"B > %"PRIu64"B",
                            aligned_alloc_size, mem->bo->size);
         anv_device_release_bo(device, mem->bo);
         goto fail;
      }

      /* From the Vulkan spec:
       *
       *    "Importing memory from a file descriptor transfers ownership of
       *    the file descriptor from the application to the Vulkan
       *    implementation. The application must not perform any operations on
       *    the file descriptor after a successful import."
       *
       * If the import fails, we leave the file descriptor open.
       */
      close(fd_info->fd);
      goto success;
   }

   if (mem->vk.host_ptr) {
      if (mem->vk.import_handle_type ==
          VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_MAPPED_FOREIGN_MEMORY_BIT_EXT) {
         result = vk_error(device, VK_ERROR_INVALID_EXTERNAL_HANDLE);
         goto fail;
      }

      assert(mem->vk.import_handle_type ==
             VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT);

      result = anv_device_import_bo_from_host_ptr(device,
                                                  mem->vk.host_ptr,
                                                  mem->vk.size,
                                                  alloc_flags,
                                                  client_address,
                                                  &mem->bo);
      if (result != VK_SUCCESS)
         goto fail;

      goto success;
   }

   if (alloc_flags & (ANV_BO_ALLOC_EXTERNAL | ANV_BO_ALLOC_SCANOUT)) {
      alloc_flags |= ANV_BO_ALLOC_HOST_COHERENT;
   } else if (mem_type->propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
      if (mem_type->propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
         alloc_flags |= ANV_BO_ALLOC_HOST_COHERENT;
      if (mem_type->propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT)
         alloc_flags |= ANV_BO_ALLOC_HOST_CACHED;
   } else {
      /* Required to set some host mode to have a valid pat index set */
      alloc_flags |= ANV_BO_ALLOC_HOST_COHERENT;
   }

   /* Regular allocate (not importing memory). */

   result = anv_device_alloc_bo(device, "user", pAllocateInfo->allocationSize,
                                alloc_flags, client_address, &mem->bo);
   if (result != VK_SUCCESS)
      goto fail;

   if (dedicated_info && dedicated_info->image != VK_NULL_HANDLE) {
      ANV_FROM_HANDLE(anv_image, image, dedicated_info->image);

      /* Some legacy (non-modifiers) consumers need the tiling to be set on
       * the BO.  In this case, we have a dedicated allocation.
       */
      if (image->vk.wsi_legacy_scanout) {
         const struct isl_surf *surf = &image->planes[0].primary_surface.isl;
         result = anv_device_set_bo_tiling(device, mem->bo,
                                           surf->row_pitch_B,
                                           surf->tiling);
         if (result != VK_SUCCESS) {
            anv_device_release_bo(device, mem->bo);
            goto fail;
         }
      }
   }

 success:
   mem_heap_used = p_atomic_add_return(&mem_heap->used, mem->bo->size);
   if (mem_heap_used > mem_heap->size) {
      p_atomic_add(&mem_heap->used, -mem->bo->size);
      anv_device_release_bo(device, mem->bo);
      result = vk_errorf(device, VK_ERROR_OUT_OF_DEVICE_MEMORY,
                         "Out of heap memory");
      goto fail;
   }

   pthread_mutex_lock(&device->mutex);
   list_addtail(&mem->link, &device->memory_objects);
   pthread_mutex_unlock(&device->mutex);

   *pMem = anv_device_memory_to_handle(mem);

   return VK_SUCCESS;

 fail:
   vk_device_memory_destroy(&device->vk, pAllocator, &mem->vk);

   return result;
}

VkResult anv_GetMemoryFdKHR(
    VkDevice                                    device_h,
    const VkMemoryGetFdInfoKHR*                 pGetFdInfo,
    int*                                        pFd)
{
   ANV_FROM_HANDLE(anv_device, dev, device_h);
   ANV_FROM_HANDLE(anv_device_memory, mem, pGetFdInfo->memory);

   assert(pGetFdInfo->sType == VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR);

   assert(pGetFdInfo->handleType == VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT ||
          pGetFdInfo->handleType == VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT);

   return anv_device_export_bo(dev, mem->bo, pFd);
}

VkResult anv_GetMemoryFdPropertiesKHR(
    VkDevice                                    _device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    int                                         fd,
    VkMemoryFdPropertiesKHR*                    pMemoryFdProperties)
{
   ANV_FROM_HANDLE(anv_device, device, _device);

   switch (handleType) {
   case VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT:
      /* dma-buf can be imported as any memory type */
      pMemoryFdProperties->memoryTypeBits =
         (1 << device->physical->memory.type_count) - 1;
      return VK_SUCCESS;

   default:
      /* The valid usage section for this function says:
       *
       *    "handleType must not be one of the handle types defined as
       *    opaque."
       *
       * So opaque handle types fall into the default "unsupported" case.
       */
      return vk_error(device, VK_ERROR_INVALID_EXTERNAL_HANDLE);
   }
}

VkResult anv_GetMemoryHostPointerPropertiesEXT(
   VkDevice                                    _device,
   VkExternalMemoryHandleTypeFlagBits          handleType,
   const void*                                 pHostPointer,
   VkMemoryHostPointerPropertiesEXT*           pMemoryHostPointerProperties)
{
   ANV_FROM_HANDLE(anv_device, device, _device);

   assert(pMemoryHostPointerProperties->sType ==
          VK_STRUCTURE_TYPE_MEMORY_HOST_POINTER_PROPERTIES_EXT);

   switch (handleType) {
   case VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT:
      /* Host memory can be imported as any memory type. */
      pMemoryHostPointerProperties->memoryTypeBits =
         (1ull << device->physical->memory.type_count) - 1;

      return VK_SUCCESS;

   default:
      return VK_ERROR_INVALID_EXTERNAL_HANDLE;
   }
}

void anv_FreeMemory(
    VkDevice                                    _device,
    VkDeviceMemory                              _mem,
    const VkAllocationCallbacks*                pAllocator)
{
   ANV_FROM_HANDLE(anv_device, device, _device);
   ANV_FROM_HANDLE(anv_device_memory, mem, _mem);

   if (mem == NULL)
      return;

   pthread_mutex_lock(&device->mutex);
   list_del(&mem->link);
   pthread_mutex_unlock(&device->mutex);

   if (mem->map) {
      const VkMemoryUnmapInfoKHR unmap = {
         .sType = VK_STRUCTURE_TYPE_MEMORY_UNMAP_INFO_KHR,
         .memory = _mem,
      };
      anv_UnmapMemory2KHR(_device, &unmap);
   }

   p_atomic_add(&device->physical->memory.heaps[mem->type->heapIndex].used,
                -mem->bo->size);

   anv_device_release_bo(device, mem->bo);

   vk_device_memory_destroy(&device->vk, pAllocator, &mem->vk);
}

VkResult anv_MapMemory2KHR(
    VkDevice                                    _device,
    const VkMemoryMapInfoKHR*                   pMemoryMapInfo,
    void**                                      ppData)
{
   ANV_FROM_HANDLE(anv_device, device, _device);
   ANV_FROM_HANDLE(anv_device_memory, mem, pMemoryMapInfo->memory);

   if (mem == NULL) {
      *ppData = NULL;
      return VK_SUCCESS;
   }

   if (mem->vk.host_ptr) {
      *ppData = mem->vk.host_ptr + pMemoryMapInfo->offset;
      return VK_SUCCESS;
   }

   /* From the Vulkan spec version 1.0.32 docs for MapMemory:
    *
    *  * memory must have been created with a memory type that reports
    *    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
    */
   if (!(mem->type->propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
      return vk_errorf(device, VK_ERROR_MEMORY_MAP_FAILED,
                       "Memory object not mappable.");
   }

   assert(pMemoryMapInfo->size > 0);
   const VkDeviceSize offset = pMemoryMapInfo->offset;
   const VkDeviceSize size =
      vk_device_memory_range(&mem->vk, pMemoryMapInfo->offset,
                                       pMemoryMapInfo->size);

   if (size != (size_t)size) {
      return vk_errorf(device, VK_ERROR_MEMORY_MAP_FAILED,
                       "requested size 0x%"PRIx64" does not fit in %u bits",
                       size, (unsigned)(sizeof(size_t) * 8));
   }

   /* From the Vulkan 1.2.194 spec:
    *
    *    "memory must not be currently host mapped"
    */
   if (mem->map != NULL) {
      return vk_errorf(device, VK_ERROR_MEMORY_MAP_FAILED,
                       "Memory object already mapped.");
   }

   /* GEM will fail to map if the offset isn't 4k-aligned.  Round down. */
   uint64_t map_offset;
   if (!device->physical->info.has_mmap_offset)
      map_offset = offset & ~4095ull;
   else
      map_offset = 0;
   assert(offset >= map_offset);
   uint64_t map_size = (offset + size) - map_offset;

   /* Let's map whole pages */
   map_size = align64(map_size, 4096);

   void *map;
   VkResult result = anv_device_map_bo(device, mem->bo, map_offset, map_size, &map);
   if (result != VK_SUCCESS)
      return result;

   mem->map = map;
   mem->map_size = map_size;
   mem->map_delta = (offset - map_offset);
   *ppData = mem->map + mem->map_delta;

   return VK_SUCCESS;
}

VkResult anv_UnmapMemory2KHR(
    VkDevice                                    _device,
    const VkMemoryUnmapInfoKHR*                 pMemoryUnmapInfo)
{
   ANV_FROM_HANDLE(anv_device, device, _device);
   ANV_FROM_HANDLE(anv_device_memory, mem, pMemoryUnmapInfo->memory);

   if (mem == NULL || mem->vk.host_ptr)
      return VK_SUCCESS;

   anv_device_unmap_bo(device, mem->bo, mem->map, mem->map_size);

   mem->map = NULL;
   mem->map_size = 0;
   mem->map_delta = 0;

   return VK_SUCCESS;
}

VkResult anv_FlushMappedMemoryRanges(
    VkDevice                                    _device,
    uint32_t                                    memoryRangeCount,
    const VkMappedMemoryRange*                  pMemoryRanges)
{
#ifdef SUPPORT_INTEL_INTEGRATED_GPUS
   ANV_FROM_HANDLE(anv_device, device, _device);

   if (!device->physical->memory.need_flush)
      return VK_SUCCESS;

   /* Make sure the writes we're flushing have landed. */
   __builtin_ia32_mfence();

   for (uint32_t i = 0; i < memoryRangeCount; i++) {
      ANV_FROM_HANDLE(anv_device_memory, mem, pMemoryRanges[i].memory);
      if (mem->type->propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
         continue;

      uint64_t map_offset = pMemoryRanges[i].offset + mem->map_delta;
      if (map_offset >= mem->map_size)
         continue;

      intel_flush_range(mem->map + map_offset,
                        MIN2(pMemoryRanges[i].size,
                             mem->map_size - map_offset));
   }
#endif
   return VK_SUCCESS;
}

VkResult anv_InvalidateMappedMemoryRanges(
    VkDevice                                    _device,
    uint32_t                                    memoryRangeCount,
    const VkMappedMemoryRange*                  pMemoryRanges)
{
#ifdef SUPPORT_INTEL_INTEGRATED_GPUS
   ANV_FROM_HANDLE(anv_device, device, _device);

   if (!device->physical->memory.need_flush)
      return VK_SUCCESS;

   for (uint32_t i = 0; i < memoryRangeCount; i++) {
      ANV_FROM_HANDLE(anv_device_memory, mem, pMemoryRanges[i].memory);
      if (mem->type->propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
         continue;

      uint64_t map_offset = pMemoryRanges[i].offset + mem->map_delta;
      if (map_offset >= mem->map_size)
         continue;

      intel_invalidate_range(mem->map + map_offset,
                             MIN2(pMemoryRanges[i].size,
                                  mem->map_size - map_offset));
   }

   /* Make sure no reads get moved up above the invalidate. */
   __builtin_ia32_mfence();
#endif
   return VK_SUCCESS;
}

void anv_GetDeviceMemoryCommitment(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkDeviceSize*                               pCommittedMemoryInBytes)
{
   *pCommittedMemoryInBytes = 0;
}

static void
anv_bind_buffer_memory(const VkBindBufferMemoryInfo *pBindInfo)
{
   ANV_FROM_HANDLE(anv_device_memory, mem, pBindInfo->memory);
   ANV_FROM_HANDLE(anv_buffer, buffer, pBindInfo->buffer);

   assert(pBindInfo->sType == VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO);
   assert(!anv_buffer_is_sparse(buffer));

   const VkBindMemoryStatusKHR *bind_status =
      vk_find_struct_const(pBindInfo->pNext, BIND_MEMORY_STATUS_KHR);

   if (mem) {
      assert(pBindInfo->memoryOffset < mem->vk.size);
      assert(mem->vk.size - pBindInfo->memoryOffset >= buffer->vk.size);
      buffer->address = (struct anv_address) {
         .bo = mem->bo,
         .offset = pBindInfo->memoryOffset,
      };
   } else {
      buffer->address = ANV_NULL_ADDRESS;
   }

   if (bind_status)
      *bind_status->pResult = VK_SUCCESS;
}

VkResult anv_BindBufferMemory2(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindBufferMemoryInfo*               pBindInfos)
{
   for (uint32_t i = 0; i < bindInfoCount; i++)
      anv_bind_buffer_memory(&pBindInfos[i]);

   return VK_SUCCESS;
}

// Event functions

VkResult anv_CreateEvent(
    VkDevice                                    _device,
    const VkEventCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkEvent*                                    pEvent)
{
   ANV_FROM_HANDLE(anv_device, device, _device);
   struct anv_event *event;

   assert(pCreateInfo->sType == VK_STRUCTURE_TYPE_EVENT_CREATE_INFO);

   event = vk_object_alloc(&device->vk, pAllocator, sizeof(*event),
                           VK_OBJECT_TYPE_EVENT);
   if (event == NULL)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   event->state = anv_state_pool_alloc(&device->dynamic_state_pool,
                                       sizeof(uint64_t), 8);
   *(uint64_t *)event->state.map = VK_EVENT_RESET;

   *pEvent = anv_event_to_handle(event);

   return VK_SUCCESS;
}

void anv_DestroyEvent(
    VkDevice                                    _device,
    VkEvent                                     _event,
    const VkAllocationCallbacks*                pAllocator)
{
   ANV_FROM_HANDLE(anv_device, device, _device);
   ANV_FROM_HANDLE(anv_event, event, _event);

   if (!event)
      return;

   anv_state_pool_free(&device->dynamic_state_pool, event->state);

   vk_object_free(&device->vk, pAllocator, event);
}

VkResult anv_GetEventStatus(
    VkDevice                                    _device,
    VkEvent                                     _event)
{
   ANV_FROM_HANDLE(anv_device, device, _device);
   ANV_FROM_HANDLE(anv_event, event, _event);

   if (vk_device_is_lost(&device->vk))
      return VK_ERROR_DEVICE_LOST;

   return *(uint64_t *)event->state.map;
}

VkResult anv_SetEvent(
    VkDevice                                    _device,
    VkEvent                                     _event)
{
   ANV_FROM_HANDLE(anv_event, event, _event);

   *(uint64_t *)event->state.map = VK_EVENT_SET;

   return VK_SUCCESS;
}

VkResult anv_ResetEvent(
    VkDevice                                    _device,
    VkEvent                                     _event)
{
   ANV_FROM_HANDLE(anv_event, event, _event);

   *(uint64_t *)event->state.map = VK_EVENT_RESET;

   return VK_SUCCESS;
}

// Buffer functions

static void
anv_get_buffer_memory_requirements(struct anv_device *device,
                                   VkBufferCreateFlags flags,
                                   VkDeviceSize size,
                                   VkBufferUsageFlags usage,
                                   bool is_sparse,
                                   VkMemoryRequirements2* pMemoryRequirements)
{
   /* The Vulkan spec (git aaed022) says:
    *
    *    memoryTypeBits is a bitfield and contains one bit set for every
    *    supported memory type for the resource. The bit `1<<i` is set if and
    *    only if the memory type `i` in the VkPhysicalDeviceMemoryProperties
    *    structure for the physical device is supported.
    */
   uint32_t memory_types = 0;
   for (uint32_t i = 0; i < device->physical->memory.type_count; i++) {
      /* Have the protected buffer bit match only the memory types with the
       * equivalent bit.
       */
      if (!!(flags & VK_BUFFER_CREATE_PROTECTED_BIT) !=
          !!(device->physical->memory.types[i].propertyFlags &
             VK_MEMORY_PROPERTY_PROTECTED_BIT))
         continue;

      memory_types |= 1ull << i;
   }

   /* The GPU appears to write back to main memory in cachelines. Writes to a
    * buffers should not clobber with writes to another buffers so make sure
    * those are in different cachelines.
    */
   uint32_t alignment = 64;

   /* From the spec, section "Sparse Buffer and Fully-Resident Image Block
    * Size":
    *   "The sparse block size in bytes for sparse buffers and fully-resident
    *    images is reported as VkMemoryRequirements::alignment. alignment
    *    represents both the memory alignment requirement and the binding
    *    granularity (in bytes) for sparse resources."
    */
   if (is_sparse) {
      alignment = ANV_SPARSE_BLOCK_SIZE;
      size = align64(size, alignment);
   }

   pMemoryRequirements->memoryRequirements.size = size;
   pMemoryRequirements->memoryRequirements.alignment = alignment;

   /* Storage and Uniform buffers should have their size aligned to
    * 32-bits to avoid boundary checks when last DWord is not complete.
    * This would ensure that not internal padding would be needed for
    * 16-bit types.
    */
   if (device->robust_buffer_access &&
       (usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT ||
        usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT))
      pMemoryRequirements->memoryRequirements.size = align64(size, 4);

   pMemoryRequirements->memoryRequirements.memoryTypeBits = memory_types;

   vk_foreach_struct(ext, pMemoryRequirements->pNext) {
      switch (ext->sType) {
      case VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS: {
         VkMemoryDedicatedRequirements *requirements = (void *)ext;
         requirements->prefersDedicatedAllocation = false;
         requirements->requiresDedicatedAllocation = false;
         break;
      }

      default:
         anv_debug_ignored_stype(ext->sType);
         break;
      }
   }
}

void anv_GetDeviceBufferMemoryRequirements(
    VkDevice                                    _device,
    const VkDeviceBufferMemoryRequirements*     pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements)
{
   ANV_FROM_HANDLE(anv_device, device, _device);
   const bool is_sparse =
      pInfo->pCreateInfo->flags & VK_BUFFER_CREATE_SPARSE_BINDING_BIT;

   if (!device->physical->has_sparse &&
       INTEL_DEBUG(DEBUG_SPARSE) &&
       pInfo->pCreateInfo->flags & (VK_BUFFER_CREATE_SPARSE_BINDING_BIT |
                                    VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT |
                                    VK_BUFFER_CREATE_SPARSE_ALIASED_BIT))
      fprintf(stderr, "=== %s %s:%d flags:0x%08x\n", __func__, __FILE__,
              __LINE__, pInfo->pCreateInfo->flags);

   anv_get_buffer_memory_requirements(device,
                                      pInfo->pCreateInfo->flags,
                                      pInfo->pCreateInfo->size,
                                      pInfo->pCreateInfo->usage,
                                      is_sparse,
                                      pMemoryRequirements);
}

VkResult anv_CreateBuffer(
    VkDevice                                    _device,
    const VkBufferCreateInfo*                   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkBuffer*                                   pBuffer)
{
   ANV_FROM_HANDLE(anv_device, device, _device);
   struct anv_buffer *buffer;

   if (!device->physical->has_sparse &&
       INTEL_DEBUG(DEBUG_SPARSE) &&
       pCreateInfo->flags & (VK_BUFFER_CREATE_SPARSE_BINDING_BIT |
                             VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT |
                             VK_BUFFER_CREATE_SPARSE_ALIASED_BIT))
      fprintf(stderr, "=== %s %s:%d flags:0x%08x\n", __func__, __FILE__,
              __LINE__, pCreateInfo->flags);

   /* Don't allow creating buffers bigger than our address space.  The real
    * issue here is that we may align up the buffer size and we don't want
    * doing so to cause roll-over.  However, no one has any business
    * allocating a buffer larger than our GTT size.
    */
   if (pCreateInfo->size > device->physical->gtt_size)
      return vk_error(device, VK_ERROR_OUT_OF_DEVICE_MEMORY);

   buffer = vk_buffer_create(&device->vk, pCreateInfo,
                             pAllocator, sizeof(*buffer));
   if (buffer == NULL)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   buffer->address = ANV_NULL_ADDRESS;
   if (anv_buffer_is_sparse(buffer)) {
      const VkBufferOpaqueCaptureAddressCreateInfo *opaque_addr_info =
         vk_find_struct_const(pCreateInfo->pNext,
                              BUFFER_OPAQUE_CAPTURE_ADDRESS_CREATE_INFO);
      enum anv_bo_alloc_flags alloc_flags = 0;
      uint64_t client_address = 0;

      if (opaque_addr_info) {
         alloc_flags = ANV_BO_ALLOC_CLIENT_VISIBLE_ADDRESS;
         client_address = opaque_addr_info->opaqueCaptureAddress;
      }

      VkResult result = anv_init_sparse_bindings(device, buffer->vk.size,
                                                 &buffer->sparse_data,
                                                 alloc_flags, client_address,
                                                 &buffer->address);
      if (result != VK_SUCCESS) {
         vk_buffer_destroy(&device->vk, pAllocator, &buffer->vk);
         return result;
      }
   }

   *pBuffer = anv_buffer_to_handle(buffer);

   return VK_SUCCESS;
}

void anv_DestroyBuffer(
    VkDevice                                    _device,
    VkBuffer                                    _buffer,
    const VkAllocationCallbacks*                pAllocator)
{
   ANV_FROM_HANDLE(anv_device, device, _device);
   ANV_FROM_HANDLE(anv_buffer, buffer, _buffer);

   if (!buffer)
      return;

   if (anv_buffer_is_sparse(buffer)) {
      assert(buffer->address.offset == buffer->sparse_data.address);
      anv_free_sparse_bindings(device, &buffer->sparse_data);
   }

   vk_buffer_destroy(&device->vk, pAllocator, &buffer->vk);
}

VkDeviceAddress anv_GetBufferDeviceAddress(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo)
{
   ANV_FROM_HANDLE(anv_buffer, buffer, pInfo->buffer);

   assert(!anv_address_is_null(buffer->address));

   return anv_address_physical(buffer->address);
}

uint64_t anv_GetBufferOpaqueCaptureAddress(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo)
{
   ANV_FROM_HANDLE(anv_buffer, buffer, pInfo->buffer);

   return anv_address_physical(buffer->address);
}

uint64_t anv_GetDeviceMemoryOpaqueCaptureAddress(
    VkDevice                                    device,
    const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo)
{
   ANV_FROM_HANDLE(anv_device_memory, memory, pInfo->memory);

   assert(memory->bo->alloc_flags & ANV_BO_ALLOC_CLIENT_VISIBLE_ADDRESS);

   return intel_48b_address(memory->bo->offset);
}

void
anv_fill_buffer_surface_state(struct anv_device *device,
                              void *surface_state_ptr,
                              enum isl_format format,
                              struct isl_swizzle swizzle,
                              isl_surf_usage_flags_t usage,
                              struct anv_address address,
                              uint32_t range, uint32_t stride)
{
   isl_buffer_fill_state(&device->isl_dev, surface_state_ptr,
                         .address = anv_address_physical(address),
                         .mocs = isl_mocs(&device->isl_dev, usage,
                                          address.bo && anv_bo_is_external(address.bo)),
                         .size_B = range,
                         .format = format,
                         .swizzle = swizzle,
                         .stride_B = stride);
}

void anv_DestroySampler(
    VkDevice                                    _device,
    VkSampler                                   _sampler,
    const VkAllocationCallbacks*                pAllocator)
{
   ANV_FROM_HANDLE(anv_device, device, _device);
   ANV_FROM_HANDLE(anv_sampler, sampler, _sampler);

   if (!sampler)
      return;

   if (sampler->bindless_state.map) {
      anv_state_pool_free(&device->dynamic_state_pool,
                          sampler->bindless_state);
   }

   if (sampler->custom_border_color.map) {
      anv_state_reserved_pool_free(&device->custom_border_colors,
                                   sampler->custom_border_color);
   }

   vk_sampler_destroy(&device->vk, pAllocator, &sampler->vk);
}

static const VkTimeDomainKHR anv_time_domains[] = {
   VK_TIME_DOMAIN_DEVICE_KHR,
   VK_TIME_DOMAIN_CLOCK_MONOTONIC_KHR,
#ifdef CLOCK_MONOTONIC_RAW
   VK_TIME_DOMAIN_CLOCK_MONOTONIC_RAW_KHR,
#endif
};

VkResult anv_GetPhysicalDeviceCalibrateableTimeDomainsKHR(
   VkPhysicalDevice                             physicalDevice,
   uint32_t                                     *pTimeDomainCount,
   VkTimeDomainKHR                              *pTimeDomains)
{
   int d;
   VK_OUTARRAY_MAKE_TYPED(VkTimeDomainKHR, out, pTimeDomains, pTimeDomainCount);

   for (d = 0; d < ARRAY_SIZE(anv_time_domains); d++) {
      vk_outarray_append_typed(VkTimeDomainKHR, &out, i) {
         *i = anv_time_domains[d];
      }
   }

   return vk_outarray_status(&out);
}

static inline clockid_t
anv_get_default_cpu_clock_id(void)
{
#ifdef CLOCK_MONOTONIC_RAW
   return CLOCK_MONOTONIC_RAW;
#else
   return CLOCK_MONOTONIC;
#endif
}

static inline clockid_t
vk_time_domain_to_clockid(VkTimeDomainKHR domain)
{
   switch (domain) {
#ifdef CLOCK_MONOTONIC_RAW
   case VK_TIME_DOMAIN_CLOCK_MONOTONIC_RAW_KHR:
      return CLOCK_MONOTONIC_RAW;
#endif
   case VK_TIME_DOMAIN_CLOCK_MONOTONIC_KHR:
      return CLOCK_MONOTONIC;
   default:
      unreachable("Missing");
      return CLOCK_MONOTONIC;
   }
}

static inline bool
is_cpu_time_domain(VkTimeDomainKHR domain)
{
   return domain == VK_TIME_DOMAIN_CLOCK_MONOTONIC_KHR ||
          domain == VK_TIME_DOMAIN_CLOCK_MONOTONIC_RAW_KHR;
}

static inline bool
is_gpu_time_domain(VkTimeDomainKHR domain)
{
   return domain == VK_TIME_DOMAIN_DEVICE_KHR;
}

VkResult anv_GetCalibratedTimestampsKHR(
   VkDevice                                     _device,
   uint32_t                                     timestampCount,
   const VkCalibratedTimestampInfoKHR           *pTimestampInfos,
   uint64_t                                     *pTimestamps,
   uint64_t                                     *pMaxDeviation)
{
   ANV_FROM_HANDLE(anv_device, device, _device);
   const uint64_t timestamp_frequency = device->info->timestamp_frequency;
   const uint64_t device_period = DIV_ROUND_UP(1000000000, timestamp_frequency);
   uint32_t d, increment;
   uint64_t begin, end;
   uint64_t max_clock_period = 0;
   const enum intel_kmd_type kmd_type = device->physical->info.kmd_type;
   const bool has_correlate_timestamp = kmd_type == INTEL_KMD_TYPE_XE;
   clockid_t cpu_clock_id = -1;

   begin = end = vk_clock_gettime(anv_get_default_cpu_clock_id());

   for (d = 0, increment = 1; d < timestampCount; d += increment) {
      const VkTimeDomainKHR current = pTimestampInfos[d].timeDomain;
      /* If we have a request pattern like this :
       * - domain0 = VK_TIME_DOMAIN_CLOCK_MONOTONIC_KHR or VK_TIME_DOMAIN_CLOCK_MONOTONIC_RAW_KHR
       * - domain1 = VK_TIME_DOMAIN_DEVICE_KHR
       * - domain2 = domain0 (optional)
       *
       * We can combine all of those into a single ioctl for maximum accuracy.
       */
      if (has_correlate_timestamp && (d + 1) < timestampCount) {
         const VkTimeDomainKHR next = pTimestampInfos[d + 1].timeDomain;

         if ((is_cpu_time_domain(current) && is_gpu_time_domain(next)) ||
             (is_gpu_time_domain(current) && is_cpu_time_domain(next))) {
            /* We'll consume at least 2 elements. */
            increment = 2;

            if (is_cpu_time_domain(current))
               cpu_clock_id = vk_time_domain_to_clockid(current);
            else
               cpu_clock_id = vk_time_domain_to_clockid(next);

            uint64_t cpu_timestamp, gpu_timestamp, cpu_delta_timestamp, cpu_end_timestamp;
            if (!intel_gem_read_correlate_cpu_gpu_timestamp(device->fd,
                                                            kmd_type,
                                                            INTEL_ENGINE_CLASS_RENDER,
                                                            0 /* engine_instance */,
                                                            cpu_clock_id,
                                                            &cpu_timestamp,
                                                            &gpu_timestamp,
                                                            &cpu_delta_timestamp))
               return vk_device_set_lost(&device->vk, "Failed to read correlate timestamp %m");

            cpu_end_timestamp = cpu_timestamp + cpu_delta_timestamp;
            if (is_cpu_time_domain(current)) {
               pTimestamps[d] = cpu_timestamp;
               pTimestamps[d + 1] = gpu_timestamp;
            } else {
               pTimestamps[d] = gpu_timestamp;
               pTimestamps[d + 1] = cpu_end_timestamp;
            }
            max_clock_period = MAX2(max_clock_period, device_period);

            /* If we can consume a third element */
            if ((d + 2) < timestampCount &&
                is_cpu_time_domain(current) &&
                current == pTimestampInfos[d + 2].timeDomain) {
               pTimestamps[d + 2] = cpu_end_timestamp;
               increment++;
            }

            /* If we're the first element, we can replace begin */
            if (d == 0 && cpu_clock_id == anv_get_default_cpu_clock_id())
               begin = cpu_timestamp;

            /* If we're in the same clock domain as begin/end. We can set the end. */
            if (cpu_clock_id == anv_get_default_cpu_clock_id())
               end = cpu_end_timestamp;

            continue;
         }
      }

      /* fallback to regular method */
      increment = 1;
      switch (current) {
      case VK_TIME_DOMAIN_DEVICE_KHR:
         if (!intel_gem_read_render_timestamp(device->fd,
                                              device->info->kmd_type,
                                              &pTimestamps[d])) {
            return vk_device_set_lost(&device->vk, "Failed to read the "
                                      "TIMESTAMP register: %m");
         }
         max_clock_period = MAX2(max_clock_period, device_period);
         break;
      case VK_TIME_DOMAIN_CLOCK_MONOTONIC_KHR:
         pTimestamps[d] = vk_clock_gettime(CLOCK_MONOTONIC);
         max_clock_period = MAX2(max_clock_period, 1);
         break;

#ifdef CLOCK_MONOTONIC_RAW
      case VK_TIME_DOMAIN_CLOCK_MONOTONIC_RAW_KHR:
         pTimestamps[d] = begin;
         break;
#endif
      default:
         pTimestamps[d] = 0;
         break;
      }
   }

   /* If last timestamp was not get with has_correlate_timestamp method or
    * if it was but last cpu clock is not the default one, get time again
    */
   if (increment == 1 || cpu_clock_id != anv_get_default_cpu_clock_id())
      end = vk_clock_gettime(anv_get_default_cpu_clock_id());

   *pMaxDeviation = vk_time_max_deviation(begin, end, max_clock_period);

   return VK_SUCCESS;
}

void anv_GetPhysicalDeviceMultisamplePropertiesEXT(
    VkPhysicalDevice                            physicalDevice,
    VkSampleCountFlagBits                       samples,
    VkMultisamplePropertiesEXT*                 pMultisampleProperties)
{
   ANV_FROM_HANDLE(anv_physical_device, physical_device, physicalDevice);

   assert(pMultisampleProperties->sType ==
          VK_STRUCTURE_TYPE_MULTISAMPLE_PROPERTIES_EXT);

   VkExtent2D grid_size;
   if (samples & isl_device_get_sample_counts(&physical_device->isl_dev)) {
      grid_size.width = 1;
      grid_size.height = 1;
   } else {
      grid_size.width = 0;
      grid_size.height = 0;
   }
   pMultisampleProperties->maxSampleLocationGridSize = grid_size;

   vk_foreach_struct(ext, pMultisampleProperties->pNext)
      anv_debug_ignored_stype(ext->sType);
}

VkResult anv_GetPhysicalDeviceFragmentShadingRatesKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pFragmentShadingRateCount,
    VkPhysicalDeviceFragmentShadingRateKHR*     pFragmentShadingRates)
{
   ANV_FROM_HANDLE(anv_physical_device, physical_device, physicalDevice);
   VK_OUTARRAY_MAKE_TYPED(VkPhysicalDeviceFragmentShadingRateKHR, out,
                          pFragmentShadingRates, pFragmentShadingRateCount);

#define append_rate(_samples, _width, _height)                                      \
   do {                                                                             \
      vk_outarray_append_typed(VkPhysicalDeviceFragmentShadingRateKHR, &out, __r) { \
         __r->sampleCounts = _samples;                                              \
         __r->fragmentSize = (VkExtent2D) {                                         \
            .width = _width,                                                        \
            .height = _height,                                                      \
         };                                                                         \
      }                                                                             \
   } while (0)

   VkSampleCountFlags sample_counts =
      isl_device_get_sample_counts(&physical_device->isl_dev);

   /* BSpec 47003: There are a number of restrictions on the sample count
    * based off the coarse pixel size.
    */
   static const VkSampleCountFlags cp_size_sample_limits[] = {
      [1]  = ISL_SAMPLE_COUNT_16_BIT | ISL_SAMPLE_COUNT_8_BIT |
             ISL_SAMPLE_COUNT_4_BIT | ISL_SAMPLE_COUNT_2_BIT | ISL_SAMPLE_COUNT_1_BIT,
      [2]  = ISL_SAMPLE_COUNT_4_BIT | ISL_SAMPLE_COUNT_2_BIT | ISL_SAMPLE_COUNT_1_BIT,
      [4]  = ISL_SAMPLE_COUNT_4_BIT | ISL_SAMPLE_COUNT_2_BIT | ISL_SAMPLE_COUNT_1_BIT,
      [8]  = ISL_SAMPLE_COUNT_2_BIT | ISL_SAMPLE_COUNT_1_BIT,
      [16] = ISL_SAMPLE_COUNT_1_BIT,
   };

   for (uint32_t x = 4; x >= 1; x /= 2) {
       for (uint32_t y = 4; y >= 1; y /= 2) {
          if (physical_device->info.has_coarse_pixel_primitive_and_cb) {
             /* BSpec 47003:
              *   "CPsize 1x4 and 4x1 are not supported"
              */
             if ((x == 1 && y == 4) || (x == 4 && y == 1))
                continue;

             /* For size {1, 1}, the sample count must be ~0
              *
              * 4x2 is also a specially case.
              */
             if (x == 1 && y == 1)
                append_rate(~0, x, y);
             else if (x == 4 && y == 2)
                append_rate(ISL_SAMPLE_COUNT_1_BIT, x, y);
             else
                append_rate(cp_size_sample_limits[x * y], x, y);
          } else {
             /* For size {1, 1}, the sample count must be ~0 */
             if (x == 1 && y == 1)
                append_rate(~0, x, y);
             else
                append_rate(sample_counts, x, y);
          }
       }
   }

#undef append_rate

   return vk_outarray_status(&out);
}

const struct intel_device_info_pat_entry *
anv_device_get_pat_entry(struct anv_device *device,
                         enum anv_bo_alloc_flags alloc_flags)
{
   if (alloc_flags & ANV_BO_ALLOC_IMPORTED)
      return &device->info->pat.cached_coherent;

   /* PAT indexes has no actual effect in DG2 and DG1, smem caches will always
    * be snopped by GPU and lmem will always be WC.
    * This might change in future discrete platforms.
    */
   if (anv_physical_device_has_vram(device->physical)) {
      if (alloc_flags & ANV_BO_ALLOC_NO_LOCAL_MEM)
         return &device->info->pat.cached_coherent;
      return &device->info->pat.writecombining;
   }

   if ((alloc_flags & (ANV_BO_ALLOC_HOST_CACHED_COHERENT)) == ANV_BO_ALLOC_HOST_CACHED_COHERENT)
      return &device->info->pat.cached_coherent;
   else if (alloc_flags & (ANV_BO_ALLOC_EXTERNAL | ANV_BO_ALLOC_SCANOUT))
      return &device->info->pat.scanout;
   else if (alloc_flags & ANV_BO_ALLOC_HOST_CACHED)
      return &device->info->pat.writeback_incoherent;
   else
      return &device->info->pat.writecombining;
}

static VkComponentTypeKHR
convert_component_type(enum intel_cooperative_matrix_component_type t)
{
   switch (t) {
   case INTEL_CMAT_FLOAT16: return VK_COMPONENT_TYPE_FLOAT16_KHR;
   case INTEL_CMAT_FLOAT32: return VK_COMPONENT_TYPE_FLOAT32_KHR;
   case INTEL_CMAT_SINT32:  return VK_COMPONENT_TYPE_SINT32_KHR;
   case INTEL_CMAT_SINT8:   return VK_COMPONENT_TYPE_SINT8_KHR;
   case INTEL_CMAT_UINT32:  return VK_COMPONENT_TYPE_UINT32_KHR;
   case INTEL_CMAT_UINT8:   return VK_COMPONENT_TYPE_UINT8_KHR;
   }
   unreachable("invalid cooperative matrix component type in configuration");
}

static VkScopeKHR
convert_scope(mesa_scope scope)
{
   switch (scope) {
   case SCOPE_DEVICE:       return VK_SCOPE_DEVICE_KHR;
   case SCOPE_WORKGROUP:    return VK_SCOPE_WORKGROUP_KHR;
   case SCOPE_SUBGROUP:     return VK_SCOPE_SUBGROUP_KHR;
   case SCOPE_QUEUE_FAMILY: return VK_SCOPE_QUEUE_FAMILY_KHR;
   default:
      unreachable("invalid cooperative matrix scope in configuration");
   }
}

VkResult anv_GetPhysicalDeviceCooperativeMatrixPropertiesKHR(
   VkPhysicalDevice                            physicalDevice,
   uint32_t*                                   pPropertyCount,
   VkCooperativeMatrixPropertiesKHR*           pProperties)
{
   ANV_FROM_HANDLE(anv_physical_device, pdevice, physicalDevice);
   const struct intel_device_info *devinfo = &pdevice->info;

   assert(anv_has_cooperative_matrix(pdevice));

   VK_OUTARRAY_MAKE_TYPED(VkCooperativeMatrixPropertiesKHR, out, pProperties, pPropertyCount);

   for (int i = 0; i < ARRAY_SIZE(devinfo->cooperative_matrix_configurations); i++) {
      const struct intel_cooperative_matrix_configuration *cfg =
         &devinfo->cooperative_matrix_configurations[i];

      if (cfg->scope == SCOPE_NONE)
         break;

      vk_outarray_append_typed(VkCooperativeMatrixPropertiesKHR, &out, prop) {
         prop->sType = VK_STRUCTURE_TYPE_COOPERATIVE_MATRIX_PROPERTIES_KHR;

         prop->MSize = cfg->m;
         prop->NSize = cfg->n;
         prop->KSize = cfg->k;

         prop->AType      = convert_component_type(cfg->a);
         prop->BType      = convert_component_type(cfg->b);
         prop->CType      = convert_component_type(cfg->c);
         prop->ResultType = convert_component_type(cfg->result);

         prop->saturatingAccumulation = VK_FALSE;
         prop->scope = convert_scope(cfg->scope);
      }

      /* VUID-RuntimeSpirv-saturatingAccumulation-08983 says:
       *
       *    For OpCooperativeMatrixMulAddKHR, the SaturatingAccumulation
       *    cooperative matrix operand must be present if and only if
       *    VkCooperativeMatrixPropertiesKHR::saturatingAccumulation is
       *    VK_TRUE.
       *
       * As a result, we have to advertise integer configs both with and
       * without this flag set.
       *
       * The DPAS instruction does not support the .sat modifier, so only
       * advertise the configurations when the DPAS would be lowered.
       *
       * FINISHME: It should be possible to do better than full lowering on
       * platforms that support DPAS. Emit a DPAS with a NULL accumulator
       * argument, then perform the correct sequence of saturating add
       * instructions.
       */
      if (cfg->a != INTEL_CMAT_FLOAT16 &&
          (devinfo->verx10 < 125 || debug_get_bool_option("INTEL_LOWER_DPAS", false))) {
         vk_outarray_append_typed(VkCooperativeMatrixPropertiesKHR, &out, prop) {
            prop->sType = VK_STRUCTURE_TYPE_COOPERATIVE_MATRIX_PROPERTIES_KHR;

            prop->MSize = cfg->m;
            prop->NSize = cfg->n;
            prop->KSize = cfg->k;

            prop->AType      = convert_component_type(cfg->a);
            prop->BType      = convert_component_type(cfg->b);
            prop->CType      = convert_component_type(cfg->c);
            prop->ResultType = convert_component_type(cfg->result);

            prop->saturatingAccumulation = VK_TRUE;
            prop->scope = convert_scope(cfg->scope);
         }
      }
   }

   return vk_outarray_status(&out);
}
