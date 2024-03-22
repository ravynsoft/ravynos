/*
 * Copyright © 2019 Red Hat.
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

#include "lvp_private.h"
#include "lvp_conv.h"

#include "pipe-loader/pipe_loader.h"
#include "git_sha1.h"
#include "vk_cmd_enqueue_entrypoints.h"
#include "vk_sampler.h"
#include "vk_util.h"
#include "util/detect.h"
#include "pipe/p_defines.h"
#include "pipe/p_state.h"
#include "pipe/p_context.h"
#include "frontend/drisw_api.h"

#include "util/u_inlines.h"
#include "util/os_memory.h"
#include "util/os_time.h"
#include "util/u_thread.h"
#include "util/u_atomic.h"
#include "util/timespec.h"
#include "util/ptralloc.h"
#include "nir.h"
#include "nir_builder.h"

#if DETECT_OS_LINUX
#include <sys/mman.h>
#endif

#if defined(VK_USE_PLATFORM_WAYLAND_KHR) || \
    defined(VK_USE_PLATFORM_WIN32_KHR) || \
    defined(VK_USE_PLATFORM_XCB_KHR) || \
    defined(VK_USE_PLATFORM_XLIB_KHR)
#define LVP_USE_WSI_PLATFORM
#endif
#define LVP_API_VERSION VK_MAKE_VERSION(1, 3, VK_HEADER_VERSION)

VKAPI_ATTR VkResult VKAPI_CALL lvp_EnumerateInstanceVersion(uint32_t* pApiVersion)
{
   *pApiVersion = LVP_API_VERSION;
   return VK_SUCCESS;
}

static const struct vk_instance_extension_table lvp_instance_extensions_supported = {
   .KHR_device_group_creation                = true,
   .KHR_external_fence_capabilities          = true,
   .KHR_external_memory_capabilities         = true,
   .KHR_external_semaphore_capabilities      = true,
   .KHR_get_physical_device_properties2      = true,
   .EXT_debug_report                         = true,
   .EXT_debug_utils                          = true,
#ifdef LVP_USE_WSI_PLATFORM
   .KHR_get_surface_capabilities2            = true,
   .KHR_surface                              = true,
   .KHR_surface_protected_capabilities       = true,
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
   .KHR_wayland_surface                      = true,
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
   .KHR_win32_surface                        = true,
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
   .KHR_xcb_surface                          = true,
#endif
#ifdef VK_USE_PLATFORM_XLIB_KHR
   .KHR_xlib_surface                         = true,
#endif
};

static const struct vk_device_extension_table lvp_device_extensions_supported = {
   .KHR_8bit_storage                      = true,
   .KHR_16bit_storage                     = true,
   .KHR_bind_memory2                      = true,
   .KHR_buffer_device_address             = true,
   .KHR_create_renderpass2                = true,
   .KHR_copy_commands2                    = true,
   .KHR_dedicated_allocation              = true,
   .KHR_depth_stencil_resolve             = true,
   .KHR_descriptor_update_template        = true,
   .KHR_device_group                      = true,
   .KHR_draw_indirect_count               = true,
   .KHR_driver_properties                 = true,
   .KHR_dynamic_rendering                 = true,
   .KHR_format_feature_flags2             = true,
   .KHR_external_fence                    = true,
   .KHR_external_memory                   = true,
#ifdef PIPE_MEMORY_FD
   .KHR_external_memory_fd                = true,
#endif
   .KHR_external_semaphore                = true,
   .KHR_shader_float_controls             = true,
   .KHR_get_memory_requirements2          = true,
#ifdef LVP_USE_WSI_PLATFORM
   .KHR_incremental_present               = true,
#endif
   .KHR_image_format_list                 = true,
   .KHR_imageless_framebuffer             = true,
   .KHR_maintenance1                      = true,
   .KHR_maintenance2                      = true,
   .KHR_maintenance3                      = true,
   .KHR_maintenance4                      = true,
   .KHR_maintenance5                      = true,
   .KHR_maintenance6                      = true,
   .KHR_map_memory2                       = true,
   .KHR_multiview                         = true,
   .KHR_push_descriptor                   = true,
   .KHR_pipeline_library                  = true,
   .KHR_relaxed_block_layout              = true,
   .KHR_sampler_mirror_clamp_to_edge      = true,
   .KHR_sampler_ycbcr_conversion          = true,
   .KHR_separate_depth_stencil_layouts    = true,
   .KHR_shader_atomic_int64               = true,
   .KHR_shader_clock                      = true,
   .KHR_shader_draw_parameters            = true,
   .KHR_shader_float16_int8               = true,
   .KHR_shader_integer_dot_product        = true,
   .KHR_shader_non_semantic_info          = true,
   .KHR_shader_subgroup_extended_types    = true,
   .KHR_shader_terminate_invocation       = true,
   .KHR_spirv_1_4                         = true,
   .KHR_storage_buffer_storage_class      = true,
#ifdef LVP_USE_WSI_PLATFORM
   .KHR_swapchain                         = true,
   .KHR_swapchain_mutable_format          = true,
#endif
   .KHR_synchronization2                  = true,
   .KHR_timeline_semaphore                = true,
   .KHR_uniform_buffer_standard_layout    = true,
   .KHR_variable_pointers                 = true,
   .KHR_vulkan_memory_model               = true,
   .KHR_zero_initialize_workgroup_memory  = true,
   .ARM_rasterization_order_attachment_access = true,
   .EXT_4444_formats                      = true,
   .EXT_attachment_feedback_loop_layout   = true,
   .EXT_attachment_feedback_loop_dynamic_state = true,
   .EXT_border_color_swizzle              = true,
   .EXT_calibrated_timestamps             = true,
   .EXT_color_write_enable                = true,
   .EXT_conditional_rendering             = true,
   .EXT_depth_clip_enable                 = true,
   .EXT_depth_clip_control                = true,
   .EXT_depth_range_unrestricted          = true,
   .EXT_dynamic_rendering_unused_attachments = true,
   .EXT_descriptor_buffer                 = true,
   .EXT_descriptor_indexing               = true,
   .EXT_extended_dynamic_state            = true,
   .EXT_extended_dynamic_state2           = true,
   .EXT_extended_dynamic_state3           = true,
   .EXT_external_memory_host              = true,
   .EXT_graphics_pipeline_library         = true,
   .EXT_host_image_copy                   = true,
   .EXT_host_query_reset                  = true,
   .EXT_image_2d_view_of_3d               = true,
   .EXT_image_sliced_view_of_3d           = true,
   .EXT_image_robustness                  = true,
   .EXT_index_type_uint8                  = true,
   .EXT_inline_uniform_block              = true,
   .EXT_load_store_op_none                = true,
   .EXT_memory_budget                     = true,
#if DETECT_OS_LINUX
   .EXT_memory_priority                   = true,
#endif
   .EXT_mesh_shader                       = true,
   .EXT_multisampled_render_to_single_sampled = true,
   .EXT_multi_draw                        = true,
   .EXT_mutable_descriptor_type           = true,
   .EXT_nested_command_buffer             = true,
   .EXT_non_seamless_cube_map             = true,
#if DETECT_OS_LINUX
   .EXT_pageable_device_local_memory      = true,
#endif
   .EXT_pipeline_creation_feedback        = true,
   .EXT_pipeline_creation_cache_control   = true,
   .EXT_post_depth_coverage               = true,
   .EXT_private_data                      = true,
   .EXT_primitives_generated_query        = true,
   .EXT_primitive_topology_list_restart   = true,
   .EXT_rasterization_order_attachment_access = true,
   .EXT_sampler_filter_minmax             = true,
   .EXT_scalar_block_layout               = true,
   .EXT_separate_stencil_usage            = true,
   .EXT_shader_atomic_float               = true,
   .EXT_shader_atomic_float2              = true,
   .EXT_shader_demote_to_helper_invocation= true,
   .EXT_shader_object                     = true,
   .EXT_shader_stencil_export             = true,
   .EXT_shader_subgroup_ballot            = true,
   .EXT_shader_subgroup_vote              = true,
   .EXT_shader_viewport_index_layer       = true,
   .EXT_subgroup_size_control             = true,
   .EXT_texel_buffer_alignment            = true,
   .EXT_transform_feedback                = true,
   .EXT_vertex_attribute_divisor          = true,
   .EXT_vertex_input_dynamic_state        = true,
   .EXT_ycbcr_image_arrays                = true,
   .EXT_ycbcr_2plane_444_formats          = true,
   .EXT_custom_border_color               = true,
   .EXT_provoking_vertex                  = true,
   .EXT_line_rasterization                = true,
   .EXT_robustness2                       = true,
   .AMDX_shader_enqueue                   = true,
   .GOOGLE_decorate_string                = true,
   .GOOGLE_hlsl_functionality1            = true,
   .NV_device_generated_commands          = true,
};

static int
min_vertex_pipeline_param(struct pipe_screen *pscreen, enum pipe_shader_cap param)
{
   int val = INT_MAX;
   for (int i = 0; i < MESA_SHADER_COMPUTE; ++i) {
      if (i == MESA_SHADER_FRAGMENT ||
          !pscreen->get_shader_param(pscreen, i,
                                     PIPE_SHADER_CAP_MAX_INSTRUCTIONS))
         continue;

      val = MAX2(val, pscreen->get_shader_param(pscreen, i, param));
   }
   return val;
}

static int
min_shader_param(struct pipe_screen *pscreen, enum pipe_shader_cap param)
{
   return MIN3(min_vertex_pipeline_param(pscreen, param),
               pscreen->get_shader_param(pscreen, MESA_SHADER_FRAGMENT, param),
               pscreen->get_shader_param(pscreen, MESA_SHADER_COMPUTE, param));
}

static void
lvp_get_features(const struct lvp_physical_device *pdevice,
                 struct vk_features *features)
{
   bool instance_divisor = pdevice->pscreen->get_param(pdevice->pscreen, PIPE_CAP_VERTEX_ELEMENT_INSTANCE_DIVISOR) != 0;

   *features = (struct vk_features){
      /* Vulkan 1.0 */
      .robustBufferAccess                       = true,
      .fullDrawIndexUint32                      = true,
      .imageCubeArray                           = (pdevice->pscreen->get_param(pdevice->pscreen, PIPE_CAP_CUBE_MAP_ARRAY) != 0),
      .independentBlend                         = true,
      .geometryShader                           = (pdevice->pscreen->get_shader_param(pdevice->pscreen, MESA_SHADER_GEOMETRY, PIPE_SHADER_CAP_MAX_INSTRUCTIONS) != 0),
      .tessellationShader                       = (pdevice->pscreen->get_shader_param(pdevice->pscreen, MESA_SHADER_TESS_EVAL, PIPE_SHADER_CAP_MAX_INSTRUCTIONS) != 0),
      .sampleRateShading                        = (pdevice->pscreen->get_param(pdevice->pscreen, PIPE_CAP_SAMPLE_SHADING) != 0),
      .dualSrcBlend                             = (pdevice->pscreen->get_param(pdevice->pscreen, PIPE_CAP_MAX_DUAL_SOURCE_RENDER_TARGETS) != 0),
      .logicOp                                  = true,
      .multiDrawIndirect                        = (pdevice->pscreen->get_param(pdevice->pscreen, PIPE_CAP_MULTI_DRAW_INDIRECT) != 0),
      .drawIndirectFirstInstance                = true,
      .depthClamp                               = (pdevice->pscreen->get_param(pdevice->pscreen, PIPE_CAP_DEPTH_CLIP_DISABLE) != 0),
      .depthBiasClamp                           = true,
      .fillModeNonSolid                         = true,
      .depthBounds                              = (pdevice->pscreen->get_param(pdevice->pscreen, PIPE_CAP_DEPTH_BOUNDS_TEST) != 0),
      .wideLines                                = true,
      .largePoints                              = true,
      .alphaToOne                               = true,
      .multiViewport                            = true,
      .samplerAnisotropy                        = true,
      .textureCompressionETC2                   = false,
      .textureCompressionASTC_LDR               = false,
      .textureCompressionBC                     = true,
      .occlusionQueryPrecise                    = true,
      .pipelineStatisticsQuery                  = true,
      .vertexPipelineStoresAndAtomics           = (min_vertex_pipeline_param(pdevice->pscreen, PIPE_SHADER_CAP_MAX_SHADER_BUFFERS) != 0),
      .fragmentStoresAndAtomics                 = (pdevice->pscreen->get_shader_param(pdevice->pscreen, MESA_SHADER_FRAGMENT, PIPE_SHADER_CAP_MAX_SHADER_BUFFERS) != 0),
      .shaderTessellationAndGeometryPointSize   = true,
      .shaderImageGatherExtended                = true,
      .shaderStorageImageExtendedFormats        = (min_shader_param(pdevice->pscreen, PIPE_SHADER_CAP_MAX_SHADER_IMAGES) != 0),
      .shaderStorageImageMultisample            = (pdevice->pscreen->get_param(pdevice->pscreen, PIPE_CAP_TEXTURE_MULTISAMPLE) != 0),
      .shaderUniformBufferArrayDynamicIndexing  = true,
      .shaderSampledImageArrayDynamicIndexing   = true,
      .shaderStorageBufferArrayDynamicIndexing  = true,
      .shaderStorageImageArrayDynamicIndexing   = true,
      .shaderStorageImageReadWithoutFormat      = true,
      .shaderStorageImageWriteWithoutFormat     = true,
      .shaderClipDistance                       = true,
      .shaderCullDistance                       = (pdevice->pscreen->get_param(pdevice->pscreen, PIPE_CAP_CULL_DISTANCE) == 1),
      .shaderFloat64                            = (pdevice->pscreen->get_param(pdevice->pscreen, PIPE_CAP_DOUBLES) == 1),
      .shaderInt64                              = (pdevice->pscreen->get_param(pdevice->pscreen, PIPE_CAP_INT64) == 1),
      .shaderInt16                              = (min_shader_param(pdevice->pscreen, PIPE_SHADER_CAP_INT16) == 1),
      .variableMultisampleRate                  = false,
      .inheritedQueries                         = false,

      /* Vulkan 1.1 */
      .storageBuffer16BitAccess            = true,
      .uniformAndStorageBuffer16BitAccess  = true,
      .storagePushConstant16               = true,
      .storageInputOutput16                = false,
      .multiview                           = true,
      .multiviewGeometryShader             = true,
      .multiviewTessellationShader         = true,
      .variablePointersStorageBuffer       = true,
      .variablePointers                    = true,
      .protectedMemory                     = false,
      .samplerYcbcrConversion              = true,
      .shaderDrawParameters                = true,

      /* Vulkan 1.2 */
      .samplerMirrorClampToEdge = true,
      .drawIndirectCount = true,
      .storageBuffer8BitAccess = true,
      .uniformAndStorageBuffer8BitAccess = true,
      .storagePushConstant8 = true,
      .shaderBufferInt64Atomics = true,
      .shaderSharedInt64Atomics = true,
      .shaderFloat16 = pdevice->pscreen->get_shader_param(pdevice->pscreen, MESA_SHADER_FRAGMENT, PIPE_SHADER_CAP_FP16) != 0,
      .shaderInt8 = true,

      .descriptorIndexing = true,
      .shaderInputAttachmentArrayDynamicIndexing = true,
      .shaderUniformTexelBufferArrayDynamicIndexing = true,
      .shaderStorageTexelBufferArrayDynamicIndexing = true,
      .shaderUniformBufferArrayNonUniformIndexing = true,
      .shaderSampledImageArrayNonUniformIndexing = true,
      .shaderStorageBufferArrayNonUniformIndexing = true,
      .shaderStorageImageArrayNonUniformIndexing = true,
      .shaderInputAttachmentArrayNonUniformIndexing = true,
      .shaderUniformTexelBufferArrayNonUniformIndexing = true,
      .shaderStorageTexelBufferArrayNonUniformIndexing = true,
      .descriptorBindingUniformBufferUpdateAfterBind = true,
      .descriptorBindingSampledImageUpdateAfterBind = true,
      .descriptorBindingStorageImageUpdateAfterBind = true,
      .descriptorBindingStorageBufferUpdateAfterBind = true,
      .descriptorBindingUniformTexelBufferUpdateAfterBind = true,
      .descriptorBindingStorageTexelBufferUpdateAfterBind = true,
      .descriptorBindingUpdateUnusedWhilePending = true,
      .descriptorBindingPartiallyBound = true,
      .descriptorBindingVariableDescriptorCount = true,
      .runtimeDescriptorArray = true,

      .samplerFilterMinmax = true,
      .scalarBlockLayout = true,
      .imagelessFramebuffer = true,
      .uniformBufferStandardLayout = true,
      .shaderSubgroupExtendedTypes = true,
      .separateDepthStencilLayouts = true,
      .hostQueryReset = true,
      .timelineSemaphore = true,
      .bufferDeviceAddress = true,
      .bufferDeviceAddressCaptureReplay = false,
      .bufferDeviceAddressMultiDevice = false,
      .vulkanMemoryModel = true,
      .vulkanMemoryModelDeviceScope = true,
      .vulkanMemoryModelAvailabilityVisibilityChains = true,
      .shaderOutputViewportIndex = true,
      .shaderOutputLayer = true,
      .subgroupBroadcastDynamicId = true,

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
      .textureCompressionASTC_HDR = VK_FALSE,
      .shaderZeroInitializeWorkgroupMemory = true,
      .dynamicRendering = true,
      .shaderIntegerDotProduct = true,
      .maintenance4 = true,

      /* VK_EXT_descriptor_buffer */
      .descriptorBuffer = true,
      .descriptorBufferCaptureReplay = false,
      .descriptorBufferPushDescriptors = true,
      .descriptorBufferImageLayoutIgnored = true,

      /* VK_EXT_primitives_generated_query */
      .primitivesGeneratedQuery = true,
      .primitivesGeneratedQueryWithRasterizerDiscard = true,
      .primitivesGeneratedQueryWithNonZeroStreams = true,

      /* VK_EXT_border_color_swizzle */
      .borderColorSwizzle = true,
      .borderColorSwizzleFromImage = true,

      /* VK_EXT_non_seamless_cube_map */
      .nonSeamlessCubeMap = true,

      /* VK_EXT_attachment_feedback_loop_layout */
      .attachmentFeedbackLoopLayout = true,

      /* VK_EXT_rasterization_order_attachment_access */
      .rasterizationOrderColorAttachmentAccess = true,
      .rasterizationOrderDepthAttachmentAccess = true,
      .rasterizationOrderStencilAttachmentAccess = true,

      /* VK_EXT_line_rasterization */
      .rectangularLines = true,
      .bresenhamLines = true,
      .smoothLines = true,
      .stippledRectangularLines = true,
      .stippledBresenhamLines = true,
      .stippledSmoothLines = true,

      /* VK_EXT_vertex_attribute_divisor */
      .vertexAttributeInstanceRateZeroDivisor = instance_divisor,
      .vertexAttributeInstanceRateDivisor = instance_divisor,

      /* VK_EXT_multisampled_render_to_single_sampled */
      .multisampledRenderToSingleSampled = true,

      /* VK_EXT_mutable_descriptor_type */
      .mutableDescriptorType = true,

      /* VK_EXT_index_type_uint8 */
      .indexTypeUint8 = true,

      /* VK_EXT_vertex_input_dynamic_state */
      .vertexInputDynamicState = true,

      /* VK_EXT_image_sliced_view_of_3d */
      .imageSlicedViewOf3D = true,

      /* VK_EXT_depth_clip_control */
      .depthClipControl = true,

      /* VK_EXT_attachment_feedback_loop_layout_dynamic_state */
      .attachmentFeedbackLoopDynamicState = true,

      /* VK_EXT_shader_object */
      .shaderObject = true,

      /* VK_KHR_shader_clock */
      .shaderSubgroupClock = true,
      .shaderDeviceClock = true,

      /* VK_EXT_texel_buffer_alignment */
      .texelBufferAlignment = true,

      /* VK_EXT_transform_feedback */
      .transformFeedback = true,
      .geometryStreams = true,

      /* VK_EXT_conditional_rendering */
      .conditionalRendering = true,
      .inheritedConditionalRendering = false,

      /* VK_EXT_extended_dynamic_state */
      .extendedDynamicState = true,

      /* VK_EXT_4444_formats */
      .formatA4R4G4B4 = true,
      .formatA4B4G4R4 = true,

      /* VK_EXT_custom_border_color */
      .customBorderColors = true,
      .customBorderColorWithoutFormat = true,

      /* VK_EXT_color_write_enable */
      .colorWriteEnable = true,

      /* VK_EXT_image_2d_view_of_3d  */
      .image2DViewOf3D = true,
      .sampler2DViewOf3D = true,

      /* VK_EXT_provoking_vertex */
      .provokingVertexLast = true,
      .transformFeedbackPreservesProvokingVertex = true,

      /* VK_EXT_multi_draw */
      .multiDraw = true,

      /* VK_EXT_depth_clip_enable */
      .depthClipEnable = (pdevice->pscreen->get_param(pdevice->pscreen, PIPE_CAP_DEPTH_CLAMP_ENABLE) != 0),

      /* VK_EXT_extended_dynamic_state2 */
      .extendedDynamicState2 = true,
      .extendedDynamicState2LogicOp = true,
      .extendedDynamicState2PatchControlPoints = true,

      /* VK_EXT_extended_dynamic_state3 */
      .extendedDynamicState3PolygonMode = true,
      .extendedDynamicState3TessellationDomainOrigin = true,
      .extendedDynamicState3DepthClampEnable = true,
      .extendedDynamicState3DepthClipEnable = true,
      .extendedDynamicState3LogicOpEnable = true,
      .extendedDynamicState3SampleMask = true,
      .extendedDynamicState3RasterizationSamples = true,
      .extendedDynamicState3AlphaToCoverageEnable = true,
      .extendedDynamicState3AlphaToOneEnable = true,
      .extendedDynamicState3DepthClipNegativeOneToOne = true,
      .extendedDynamicState3RasterizationStream = false,
      .extendedDynamicState3ConservativeRasterizationMode = false,
      .extendedDynamicState3ExtraPrimitiveOverestimationSize = false,
      .extendedDynamicState3LineRasterizationMode = true,
      .extendedDynamicState3LineStippleEnable = true,
      .extendedDynamicState3ProvokingVertexMode = true,
      .extendedDynamicState3SampleLocationsEnable = false,
      .extendedDynamicState3ColorBlendEnable = true,
      .extendedDynamicState3ColorBlendEquation = true,
      .extendedDynamicState3ColorWriteMask = true,
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

      /* VK_EXT_dynamic_rendering_unused_attachments */
      .dynamicRenderingUnusedAttachments = true,

      /* VK_EXT_robustness2 */
      .robustBufferAccess2 = true,
      .robustImageAccess2 = true,
      .nullDescriptor = true,

      /* VK_NV_device_generated_commands */
      .deviceGeneratedCommands = true,

      /* VK_EXT_primitive_topology_list_restart */
      .primitiveTopologyListRestart = true,
      .primitiveTopologyPatchListRestart = true,

      /* VK_EXT_graphics_pipeline_library */
      .graphicsPipelineLibrary = true,

      /* VK_EXT_shader_atomic_float */
      .shaderBufferFloat32Atomics =    true,
      .shaderBufferFloat32AtomicAdd =  true,
      .shaderBufferFloat64Atomics =    false,
      .shaderBufferFloat64AtomicAdd =  false,
      .shaderSharedFloat32Atomics =    true,
      .shaderSharedFloat32AtomicAdd =  true,
      .shaderSharedFloat64Atomics =    false,
      .shaderSharedFloat64AtomicAdd =  false,
      .shaderImageFloat32Atomics =     true,
      .shaderImageFloat32AtomicAdd =   true,
      .sparseImageFloat32Atomics =     false,
      .sparseImageFloat32AtomicAdd =   false,

      /* VK_EXT_shader_atomic_float2 */
      .shaderBufferFloat16Atomics      = false,
      .shaderBufferFloat16AtomicAdd    = false,
      .shaderBufferFloat16AtomicMinMax = false,
      .shaderBufferFloat32AtomicMinMax = LLVM_VERSION_MAJOR >= 15,
      .shaderBufferFloat64AtomicMinMax = false,
      .shaderSharedFloat16Atomics      = false,
      .shaderSharedFloat16AtomicAdd    = false,
      .shaderSharedFloat16AtomicMinMax = false,
      .shaderSharedFloat32AtomicMinMax = LLVM_VERSION_MAJOR >= 15,
      .shaderSharedFloat64AtomicMinMax = false,
      .shaderImageFloat32AtomicMinMax  = LLVM_VERSION_MAJOR >= 15,
      .sparseImageFloat32AtomicMinMax  = false,

      /* VK_EXT_memory_priority */
      .memoryPriority = true,

      /* VK_EXT_pageable_device_local_memory */
      .pageableDeviceLocalMemory = true,

      /* VK_EXT_nested_command_buffer */
      .nestedCommandBuffer = true,
      .nestedCommandBufferRendering = true,
      .nestedCommandBufferSimultaneousUse = true,

      /* VK_EXT_mesh_shader */
      .taskShader = true,
      .meshShader = true,
      .multiviewMeshShader = false,
      .primitiveFragmentShadingRateMeshShader = false,
      .meshShaderQueries = true,

      /* host_image_copy */
      .hostImageCopy = true,

      /* maintenance5 */
      .maintenance5 = true,

      /* VK_EXT_ycbcr_2plane_444_formats */
      .ycbcr2plane444Formats = true,

      /* VK_EXT_ycbcr_image_arrays */
      .ycbcrImageArrays = true,

      /* maintenance6 */
      .maintenance6 = true,

      /* VK_AMDX_shader_enqueue */
#ifdef VK_ENABLE_BETA_EXTENSIONS
      .shaderEnqueue = true,
#endif
   };
}

extern unsigned lp_native_vector_width;

static VkImageLayout lvp_host_copy_image_layouts[] = {
   VK_IMAGE_LAYOUT_GENERAL,
   VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
   VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
   VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
   VK_IMAGE_LAYOUT_PREINITIALIZED,
   VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL,
   VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL,
   VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
   VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL,
   VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL,
   VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL,
   VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
   VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
   VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
   VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR,
   VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR,
   VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR,
   VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR,
   VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT,
   VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR,
};

static void
lvp_get_properties(const struct lvp_physical_device *device, struct vk_properties *p)
{
   VkSampleCountFlags sample_counts = VK_SAMPLE_COUNT_1_BIT | VK_SAMPLE_COUNT_4_BIT;

   uint64_t grid_size[3], block_size[3];
   uint64_t max_threads_per_block, max_local_size;

   device->pscreen->get_compute_param(device->pscreen, PIPE_SHADER_IR_NIR,
                                       PIPE_COMPUTE_CAP_MAX_GRID_SIZE, grid_size);
   device->pscreen->get_compute_param(device->pscreen, PIPE_SHADER_IR_NIR,
                                       PIPE_COMPUTE_CAP_MAX_BLOCK_SIZE, block_size);
   device->pscreen->get_compute_param(device->pscreen, PIPE_SHADER_IR_NIR,
                                       PIPE_COMPUTE_CAP_MAX_THREADS_PER_BLOCK,
                                       &max_threads_per_block);
   device->pscreen->get_compute_param(device->pscreen, PIPE_SHADER_IR_NIR,
                                       PIPE_COMPUTE_CAP_MAX_LOCAL_SIZE,
                                       &max_local_size);

   const uint64_t max_render_targets = device->pscreen->get_param(device->pscreen, PIPE_CAP_MAX_RENDER_TARGETS);

   int texel_buffer_alignment = device->pscreen->get_param(device->pscreen, PIPE_CAP_TEXTURE_BUFFER_OFFSET_ALIGNMENT);

   STATIC_ASSERT(sizeof(struct lp_descriptor) <= 256);
   *p = (struct vk_properties) {
      /* Vulkan 1.0 */
      .apiVersion = LVP_API_VERSION,
      .driverVersion = 1,
      .vendorID = VK_VENDOR_ID_MESA,
      .deviceID = 0,
      .deviceType = VK_PHYSICAL_DEVICE_TYPE_CPU,
      .maxImageDimension1D                      = device->pscreen->get_param(device->pscreen, PIPE_CAP_MAX_TEXTURE_2D_SIZE),
      .maxImageDimension2D                      = device->pscreen->get_param(device->pscreen, PIPE_CAP_MAX_TEXTURE_2D_SIZE),
      .maxImageDimension3D                      = (1 << device->pscreen->get_param(device->pscreen, PIPE_CAP_MAX_TEXTURE_3D_LEVELS)),
      .maxImageDimensionCube                    = (1 << device->pscreen->get_param(device->pscreen, PIPE_CAP_MAX_TEXTURE_CUBE_LEVELS)),
      .maxImageArrayLayers                      = device->pscreen->get_param(device->pscreen, PIPE_CAP_MAX_TEXTURE_ARRAY_LAYERS),
      .maxTexelBufferElements                   = device->pscreen->get_param(device->pscreen, PIPE_CAP_MAX_TEXEL_BUFFER_ELEMENTS_UINT),
      .maxUniformBufferRange                    = min_shader_param(device->pscreen, PIPE_SHADER_CAP_MAX_CONST_BUFFER0_SIZE),
      .maxStorageBufferRange                    = device->pscreen->get_param(device->pscreen, PIPE_CAP_MAX_SHADER_BUFFER_SIZE_UINT),
      .maxPushConstantsSize                     = MAX_PUSH_CONSTANTS_SIZE,
      .maxMemoryAllocationCount                 = UINT32_MAX,
      .maxSamplerAllocationCount                = 32 * 1024,
      .bufferImageGranularity                   = 64, /* A cache line */
      .sparseAddressSpaceSize                   = 0,
      .maxBoundDescriptorSets                   = MAX_SETS,
      .maxPerStageDescriptorSamplers            = MAX_DESCRIPTORS,
      .maxPerStageDescriptorUniformBuffers      = MAX_DESCRIPTORS,
      .maxPerStageDescriptorStorageBuffers      = MAX_DESCRIPTORS,
      .maxPerStageDescriptorSampledImages       = MAX_DESCRIPTORS,
      .maxPerStageDescriptorStorageImages       = MAX_DESCRIPTORS,
      .maxPerStageDescriptorInputAttachments    = MAX_DESCRIPTORS,
      .maxPerStageResources                     = MAX_DESCRIPTORS,
      .maxDescriptorSetSamplers                 = MAX_DESCRIPTORS,
      .maxDescriptorSetUniformBuffers           = MAX_DESCRIPTORS,
      .maxDescriptorSetUniformBuffersDynamic    = MAX_DESCRIPTORS,
      .maxDescriptorSetStorageBuffers           = MAX_DESCRIPTORS,
      .maxDescriptorSetStorageBuffersDynamic    = MAX_DESCRIPTORS,
      .maxDescriptorSetSampledImages            = MAX_DESCRIPTORS,
      .maxDescriptorSetStorageImages            = MAX_DESCRIPTORS,
      .maxDescriptorSetInputAttachments         = MAX_DESCRIPTORS,
      .maxVertexInputAttributes                 = 32,
      .maxVertexInputBindings                   = 32,
      .maxVertexInputAttributeOffset            = 2047,
      .maxVertexInputBindingStride              = 2048,
      .maxVertexOutputComponents                = 128,
      .maxTessellationGenerationLevel           = 64,
      .maxTessellationPatchSize                 = 32,
      .maxTessellationControlPerVertexInputComponents = 128,
      .maxTessellationControlPerVertexOutputComponents = 128,
      .maxTessellationControlPerPatchOutputComponents = 128,
      .maxTessellationControlTotalOutputComponents = 4096,
      .maxTessellationEvaluationInputComponents = 128,
      .maxTessellationEvaluationOutputComponents = 128,
      .maxGeometryShaderInvocations             = device->pscreen->get_param(device->pscreen, PIPE_CAP_MAX_GS_INVOCATIONS),
      .maxGeometryInputComponents               = 64,
      .maxGeometryOutputComponents              = 128,
      .maxGeometryOutputVertices                = device->pscreen->get_param(device->pscreen, PIPE_CAP_MAX_GEOMETRY_OUTPUT_VERTICES),
      .maxGeometryTotalOutputComponents         = device->pscreen->get_param(device->pscreen, PIPE_CAP_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS),
      .maxFragmentInputComponents               = 128,
      .maxFragmentOutputAttachments             = 8,
      .maxFragmentDualSrcAttachments            = 2,
      .maxFragmentCombinedOutputResources       = max_render_targets +
                                                  device->pscreen->get_shader_param(device->pscreen, MESA_SHADER_FRAGMENT,
                                                     PIPE_SHADER_CAP_MAX_SHADER_BUFFERS) +
                                                  device->pscreen->get_shader_param(device->pscreen, MESA_SHADER_FRAGMENT,
                                                     PIPE_SHADER_CAP_MAX_SHADER_IMAGES),
      .maxComputeSharedMemorySize               = max_local_size,
      .maxComputeWorkGroupCount                 = { grid_size[0], grid_size[1], grid_size[2] },
      .maxComputeWorkGroupInvocations           = max_threads_per_block,
      .maxComputeWorkGroupSize                  = { block_size[0], block_size[1], block_size[2] },
      .subPixelPrecisionBits                    = device->pscreen->get_param(device->pscreen, PIPE_CAP_RASTERIZER_SUBPIXEL_BITS),
      .subTexelPrecisionBits                    = 8,
      .mipmapPrecisionBits                      = 4,
      .maxDrawIndexedIndexValue                 = UINT32_MAX,
      .maxDrawIndirectCount                     = UINT32_MAX,
      .maxSamplerLodBias                        = 16,
      .maxSamplerAnisotropy                     = 16,
      .maxViewports                             = device->pscreen->get_param(device->pscreen, PIPE_CAP_MAX_VIEWPORTS),
      .maxViewportDimensions                    = { (1 << 14), (1 << 14) },
      .viewportBoundsRange                      = { -32768.0, 32768.0 },
      .viewportSubPixelBits                     = device->pscreen->get_param(device->pscreen, PIPE_CAP_VIEWPORT_SUBPIXEL_BITS),
      .minMemoryMapAlignment                    = device->pscreen->get_param(device->pscreen, PIPE_CAP_MIN_MAP_BUFFER_ALIGNMENT),
      .minTexelBufferOffsetAlignment            = device->pscreen->get_param(device->pscreen, PIPE_CAP_TEXTURE_BUFFER_OFFSET_ALIGNMENT),
      .minUniformBufferOffsetAlignment          = device->pscreen->get_param(device->pscreen, PIPE_CAP_CONSTANT_BUFFER_OFFSET_ALIGNMENT),
      .minStorageBufferOffsetAlignment          = device->pscreen->get_param(device->pscreen, PIPE_CAP_SHADER_BUFFER_OFFSET_ALIGNMENT),
      .minTexelOffset                           = device->pscreen->get_param(device->pscreen, PIPE_CAP_MIN_TEXEL_OFFSET),
      .maxTexelOffset                           = device->pscreen->get_param(device->pscreen, PIPE_CAP_MAX_TEXEL_OFFSET),
      .minTexelGatherOffset                     = device->pscreen->get_param(device->pscreen, PIPE_CAP_MIN_TEXTURE_GATHER_OFFSET),
      .maxTexelGatherOffset                     = device->pscreen->get_param(device->pscreen, PIPE_CAP_MAX_TEXTURE_GATHER_OFFSET),
      .minInterpolationOffset                   = -2, /* FIXME */
      .maxInterpolationOffset                   = 2, /* FIXME */
      .subPixelInterpolationOffsetBits          = 8, /* FIXME */
      .maxFramebufferWidth                      = device->pscreen->get_param(device->pscreen, PIPE_CAP_MAX_TEXTURE_2D_SIZE),
      .maxFramebufferHeight                     = device->pscreen->get_param(device->pscreen, PIPE_CAP_MAX_TEXTURE_2D_SIZE),
      .maxFramebufferLayers                     = device->pscreen->get_param(device->pscreen, PIPE_CAP_MAX_TEXTURE_ARRAY_LAYERS),
      .framebufferColorSampleCounts             = sample_counts,
      .framebufferDepthSampleCounts             = sample_counts,
      .framebufferStencilSampleCounts           = sample_counts,
      .framebufferNoAttachmentsSampleCounts     = sample_counts,
      .maxColorAttachments                      = max_render_targets,
      .sampledImageColorSampleCounts            = sample_counts,
      .sampledImageIntegerSampleCounts          = sample_counts,
      .sampledImageDepthSampleCounts            = sample_counts,
      .sampledImageStencilSampleCounts          = sample_counts,
      .storageImageSampleCounts                 = sample_counts,
      .maxSampleMaskWords                       = 1,
      .timestampComputeAndGraphics              = true,
      .timestampPeriod                          = 1,
      .maxClipDistances                         = 8,
      .maxCullDistances                         = 8,
      .maxCombinedClipAndCullDistances          = 8,
      .discreteQueuePriorities                  = 2,
      .pointSizeRange                           = { 0.0, device->pscreen->get_paramf(device->pscreen, PIPE_CAPF_MAX_POINT_SIZE) },
      .lineWidthRange                           = { 1.0, device->pscreen->get_paramf(device->pscreen, PIPE_CAPF_MAX_LINE_WIDTH) },
      .pointSizeGranularity                     = (1.0 / 8.0),
      .lineWidthGranularity                     = 1.0 / 128.0,
      .strictLines                              = true,
      .standardSampleLocations                  = true,
      .optimalBufferCopyOffsetAlignment         = 128,
      .optimalBufferCopyRowPitchAlignment       = 128,
      .nonCoherentAtomSize                      = 64,

      /* Vulkan 1.1 */
      /* The LUID is for Windows. */
      .deviceLUIDValid = false,
      .deviceNodeMask = 0,

      .subgroupSize = lp_native_vector_width / 32,
      .subgroupSupportedStages = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT,
      .subgroupSupportedOperations = VK_SUBGROUP_FEATURE_BASIC_BIT | VK_SUBGROUP_FEATURE_VOTE_BIT | VK_SUBGROUP_FEATURE_ARITHMETIC_BIT | VK_SUBGROUP_FEATURE_BALLOT_BIT,
      .subgroupQuadOperationsInAllStages = false,

      .pointClippingBehavior = VK_POINT_CLIPPING_BEHAVIOR_ALL_CLIP_PLANES,
      .maxMultiviewViewCount = 6,
      .maxMultiviewInstanceIndex = INT_MAX,
      .protectedNoFault = false,
      .maxPerSetDescriptors = MAX_DESCRIPTORS,
      .maxMemoryAllocationSize = (1u << 31),

      /* Vulkan 1.2 */
      .driverID = VK_DRIVER_ID_MESA_LLVMPIPE,

      .conformanceVersion = (VkConformanceVersion){
         .major = 1,
         .minor = 3,
         .subminor = 1,
         .patch = 1,
      },

      .denormBehaviorIndependence = VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_ALL,
      .roundingModeIndependence = VK_SHADER_FLOAT_CONTROLS_INDEPENDENCE_ALL,
      .shaderDenormFlushToZeroFloat16 = false,
      .shaderDenormPreserveFloat16 = false,
      .shaderRoundingModeRTEFloat16 = true,
      .shaderRoundingModeRTZFloat16 = false,
      .shaderSignedZeroInfNanPreserveFloat16 = true,

      .shaderDenormFlushToZeroFloat32 = false,
      .shaderDenormPreserveFloat32 = false,
      .shaderRoundingModeRTEFloat32 = true,
      .shaderRoundingModeRTZFloat32 = false,
      .shaderSignedZeroInfNanPreserveFloat32 = true,

      .shaderDenormFlushToZeroFloat64 = false,
      .shaderDenormPreserveFloat64 = false,
      .shaderRoundingModeRTEFloat64 = true,
      .shaderRoundingModeRTZFloat64 = false,
      .shaderSignedZeroInfNanPreserveFloat64 = true,

      .maxUpdateAfterBindDescriptorsInAllPools = UINT32_MAX,
      .shaderUniformBufferArrayNonUniformIndexingNative = true,
      .shaderSampledImageArrayNonUniformIndexingNative = true,
      .shaderStorageBufferArrayNonUniformIndexingNative = true,
      .shaderStorageImageArrayNonUniformIndexingNative = true,
      .shaderInputAttachmentArrayNonUniformIndexingNative = true,
      .robustBufferAccessUpdateAfterBind = true,
      .quadDivergentImplicitLod = true,
      .maxPerStageDescriptorUpdateAfterBindSamplers = MAX_DESCRIPTORS,
      .maxPerStageDescriptorUpdateAfterBindUniformBuffers = MAX_DESCRIPTORS,
      .maxPerStageDescriptorUpdateAfterBindStorageBuffers = MAX_DESCRIPTORS,
      .maxPerStageDescriptorUpdateAfterBindSampledImages = MAX_DESCRIPTORS,
      .maxPerStageDescriptorUpdateAfterBindStorageImages = MAX_DESCRIPTORS,
      .maxPerStageDescriptorUpdateAfterBindInputAttachments = MAX_DESCRIPTORS,
      .maxPerStageUpdateAfterBindResources = MAX_DESCRIPTORS,
      .maxDescriptorSetUpdateAfterBindSamplers = MAX_DESCRIPTORS,
      .maxDescriptorSetUpdateAfterBindUniformBuffers = MAX_DESCRIPTORS,
      .maxDescriptorSetUpdateAfterBindUniformBuffersDynamic = MAX_DESCRIPTORS,
      .maxDescriptorSetUpdateAfterBindStorageBuffers = MAX_DESCRIPTORS,
      .maxDescriptorSetUpdateAfterBindStorageBuffersDynamic = MAX_DESCRIPTORS,
      .maxDescriptorSetUpdateAfterBindSampledImages = MAX_DESCRIPTORS,
      .maxDescriptorSetUpdateAfterBindStorageImages = MAX_DESCRIPTORS,
      .maxDescriptorSetUpdateAfterBindInputAttachments = MAX_DESCRIPTORS,

      .supportedDepthResolveModes = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT | VK_RESOLVE_MODE_AVERAGE_BIT,
      .supportedStencilResolveModes = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT,
      .independentResolveNone = false,
      .independentResolve = false,

      .filterMinmaxImageComponentMapping = true,
      .filterMinmaxSingleComponentFormats = true,

      .maxTimelineSemaphoreValueDifference = UINT64_MAX,
      .framebufferIntegerColorSampleCounts = VK_SAMPLE_COUNT_1_BIT,

      /* Vulkan 1.3 */
      .minSubgroupSize = lp_native_vector_width / 32,
      .maxSubgroupSize = lp_native_vector_width / 32,
      .maxComputeWorkgroupSubgroups = 32,
      .requiredSubgroupSizeStages = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT,
      .maxInlineUniformTotalSize = MAX_DESCRIPTOR_UNIFORM_BLOCK_SIZE * MAX_PER_STAGE_DESCRIPTOR_UNIFORM_BLOCKS * MAX_SETS,
      .maxInlineUniformBlockSize = MAX_DESCRIPTOR_UNIFORM_BLOCK_SIZE,
      .maxPerStageDescriptorInlineUniformBlocks = MAX_PER_STAGE_DESCRIPTOR_UNIFORM_BLOCKS,
      .maxPerStageDescriptorUpdateAfterBindInlineUniformBlocks = MAX_PER_STAGE_DESCRIPTOR_UNIFORM_BLOCKS,
      .maxDescriptorSetInlineUniformBlocks = MAX_PER_STAGE_DESCRIPTOR_UNIFORM_BLOCKS,
      .maxDescriptorSetUpdateAfterBindInlineUniformBlocks = MAX_PER_STAGE_DESCRIPTOR_UNIFORM_BLOCKS,
      .storageTexelBufferOffsetAlignmentBytes = texel_buffer_alignment,
      .storageTexelBufferOffsetSingleTexelAlignment = true,
      .uniformTexelBufferOffsetAlignmentBytes = texel_buffer_alignment,
      .uniformTexelBufferOffsetSingleTexelAlignment = true,
      .maxBufferSize = UINT32_MAX,

      /* VK_KHR_push_descriptor */
      .maxPushDescriptors = MAX_PUSH_DESCRIPTORS,

      /* VK_EXT_host_image_copy */
      .pCopySrcLayouts = lvp_host_copy_image_layouts,
      .copySrcLayoutCount = ARRAY_SIZE(lvp_host_copy_image_layouts),
      .pCopyDstLayouts = lvp_host_copy_image_layouts,
      .copyDstLayoutCount = ARRAY_SIZE(lvp_host_copy_image_layouts),
      .identicalMemoryTypeRequirements = VK_FALSE,

      /* VK_EXT_transform_feedback */
      .maxTransformFeedbackStreams = device->pscreen->get_param(device->pscreen, PIPE_CAP_MAX_VERTEX_STREAMS),
      .maxTransformFeedbackBuffers = device->pscreen->get_param(device->pscreen, PIPE_CAP_MAX_STREAM_OUTPUT_BUFFERS),
      .maxTransformFeedbackBufferSize = UINT32_MAX,
      .maxTransformFeedbackStreamDataSize = 512,
      .maxTransformFeedbackBufferDataSize = 512,
      .maxTransformFeedbackBufferDataStride = 512,
      .transformFeedbackQueries = true,
      .transformFeedbackStreamsLinesTriangles = false,
      .transformFeedbackRasterizationStreamSelect = false,
      .transformFeedbackDraw = true,

      /* VK_KHR_maintenance5 */
      /* FIXME No idea about most of these ones. */
      .earlyFragmentMultisampleCoverageAfterSampleCounting = true,
      .earlyFragmentSampleMaskTestBeforeSampleCounting = false,
      .depthStencilSwizzleOneSupport = false,
      .polygonModePointSize = true, /* This one is correct. */
      .nonStrictSinglePixelWideLinesUseParallelogram = false,
      .nonStrictWideLinesUseParallelogram = false,

      /* maintenance6 */
      .maxCombinedImageSamplerDescriptorCount = 3,

      /* VK_EXT_extended_dynamic_state3 */
      .dynamicPrimitiveTopologyUnrestricted = VK_TRUE,

      /* VK_EXT_line_rasterization */
      .lineSubPixelPrecisionBits = device->pscreen->get_param(device->pscreen, PIPE_CAP_RASTERIZER_SUBPIXEL_BITS),

      /* VK_NV_device_generated_commands */
      .maxGraphicsShaderGroupCount = 1<<12,
      .maxIndirectSequenceCount = 1<<20,
      .maxIndirectCommandsTokenCount = MAX_DGC_TOKENS,
      .maxIndirectCommandsStreamCount = MAX_DGC_STREAMS,
      .maxIndirectCommandsTokenOffset = 2047,
      .maxIndirectCommandsStreamStride = 2048,
      .minSequencesCountBufferOffsetAlignment = 4,
      .minSequencesIndexBufferOffsetAlignment = 4,
      .minIndirectCommandsBufferOffsetAlignment = 4,

      /* VK_EXT_external_memory_host */
      .minImportedHostPointerAlignment = 4096,

      /* VK_EXT_custom_border_color */
      .maxCustomBorderColorSamplers = 32 * 1024,

      /* VK_EXT_provoking_vertex */
      .provokingVertexModePerPipeline = true,
      .transformFeedbackPreservesTriangleFanProvokingVertex = true,

      /* VK_EXT_multi_draw */
      .maxMultiDrawCount = 2048,

      /* VK_EXT_descriptor_buffer */
      .combinedImageSamplerDescriptorSingleArray = VK_TRUE,
      .bufferlessPushDescriptors = VK_TRUE,
      .descriptorBufferOffsetAlignment = 4,
      .maxDescriptorBufferBindings = MAX_SETS,
      .maxResourceDescriptorBufferBindings = MAX_SETS,
      .maxSamplerDescriptorBufferBindings = MAX_SETS,
      .maxEmbeddedImmutableSamplerBindings = MAX_SETS,
      .maxEmbeddedImmutableSamplers = 2032,
      .bufferCaptureReplayDescriptorDataSize = 0,
      .imageCaptureReplayDescriptorDataSize = 0,
      .imageViewCaptureReplayDescriptorDataSize = 0,
      .samplerCaptureReplayDescriptorDataSize = 0,
      .accelerationStructureCaptureReplayDescriptorDataSize = 0,
      .samplerDescriptorSize = sizeof(struct lp_descriptor),
      .combinedImageSamplerDescriptorSize = sizeof(struct lp_descriptor),
      .sampledImageDescriptorSize = sizeof(struct lp_descriptor),
      .storageImageDescriptorSize = sizeof(struct lp_descriptor),
      .uniformTexelBufferDescriptorSize = sizeof(struct lp_descriptor),
      .robustUniformTexelBufferDescriptorSize = sizeof(struct lp_descriptor),
      .storageTexelBufferDescriptorSize = sizeof(struct lp_descriptor),
      .robustStorageTexelBufferDescriptorSize = sizeof(struct lp_descriptor),
      .uniformBufferDescriptorSize = sizeof(struct lp_descriptor),
      .robustUniformBufferDescriptorSize = sizeof(struct lp_descriptor),
      .storageBufferDescriptorSize = sizeof(struct lp_descriptor),
      .robustStorageBufferDescriptorSize = sizeof(struct lp_descriptor),
      .inputAttachmentDescriptorSize = sizeof(struct lp_descriptor),
      .accelerationStructureDescriptorSize = 0,
      .maxSamplerDescriptorBufferRange = 1<<27, //spec minimum
      .maxResourceDescriptorBufferRange = 1<<28, //spec minimum
      .resourceDescriptorBufferAddressSpaceSize = 1<<27, //spec minimum
      .samplerDescriptorBufferAddressSpaceSize = 1<<27, //spec minimum
      .descriptorBufferAddressSpaceSize = 1<<27, //spec minimum

      /* VK_EXT_graphics_pipeline_library */
      .graphicsPipelineLibraryFastLinking = VK_TRUE,
      .graphicsPipelineLibraryIndependentInterpolationDecoration = VK_TRUE,

      /* VK_EXT_robustness2 */
      .robustStorageBufferAccessSizeAlignment = 1,
      .robustUniformBufferAccessSizeAlignment = 1,

      /* VK_EXT_mesh_shader */
      .maxTaskWorkGroupTotalCount = 4194304,
      .maxTaskWorkGroupCount[0] = 65536,
      .maxTaskWorkGroupCount[1] = 65536,
      .maxTaskWorkGroupCount[2] = 65536,
      .maxTaskWorkGroupInvocations = 1024,
      .maxTaskWorkGroupSize[0] = 1024,
      .maxTaskWorkGroupSize[1] = 1024,
      .maxTaskWorkGroupSize[2] = 1024,
      .maxTaskPayloadSize = 16384,
      .maxTaskSharedMemorySize = 32768,
      .maxTaskPayloadAndSharedMemorySize = 32768,

      .maxMeshWorkGroupTotalCount = 4194304,
      .maxMeshWorkGroupCount[0] = 65536,
      .maxMeshWorkGroupCount[1] = 65536,
      .maxMeshWorkGroupCount[2] = 65536,
      .maxMeshWorkGroupInvocations = 1024,
      .maxMeshWorkGroupSize[0] = 1024,
      .maxMeshWorkGroupSize[1] = 1024,
      .maxMeshWorkGroupSize[2] = 1024,
      .maxMeshOutputMemorySize = 32768, /* 32K min required */
      .maxMeshSharedMemorySize = 28672,     /* 28K min required */
      .maxMeshOutputComponents = 128, /* 32x vec4 min required */
      .maxMeshOutputVertices = 256,
      .maxMeshOutputPrimitives = 256,
      .maxMeshOutputLayers = 8,
      .meshOutputPerVertexGranularity = 1,
      .meshOutputPerPrimitiveGranularity = 1,
      .maxPreferredTaskWorkGroupInvocations = 64,
      .maxPreferredMeshWorkGroupInvocations = 128,
      .prefersLocalInvocationVertexOutput = true,
      .prefersLocalInvocationPrimitiveOutput = true,
      .prefersCompactVertexOutput = true,
      .prefersCompactPrimitiveOutput = false,

      /* VK_AMDX_shader_enqueue */
#ifdef VK_ENABLE_BETA_EXTENSIONS
      .maxExecutionGraphDepth = 32,
      .maxExecutionGraphShaderOutputNodes = LVP_MAX_EXEC_GRAPH_PAYLOADS,
      .maxExecutionGraphShaderPayloadSize = 0xFFFF,
      .maxExecutionGraphShaderPayloadCount = LVP_MAX_EXEC_GRAPH_PAYLOADS,
      .executionGraphDispatchAddressAlignment = 4,
#endif
   };

   /* Vulkan 1.0 */
   strcpy(p->deviceName, device->pscreen->get_name(device->pscreen));
   lvp_device_get_cache_uuid(p->pipelineCacheUUID);

   /* Vulkan 1.1 */
   device->pscreen->get_device_uuid(device->pscreen, (char*)(p->deviceUUID));
   device->pscreen->get_driver_uuid(device->pscreen, (char*)(p->driverUUID));
   memset(p->deviceLUID, 0, VK_LUID_SIZE);

#if LLVM_VERSION_MAJOR >= 10
   p->subgroupSupportedOperations |= VK_SUBGROUP_FEATURE_SHUFFLE_BIT | VK_SUBGROUP_FEATURE_SHUFFLE_RELATIVE_BIT | VK_SUBGROUP_FEATURE_QUAD_BIT;
#endif

   /* Vulkan 1.2 */
   snprintf(p->driverName, VK_MAX_DRIVER_NAME_SIZE, "llvmpipe");
   snprintf(p->driverInfo, VK_MAX_DRIVER_INFO_SIZE, "Mesa " PACKAGE_VERSION MESA_GIT_SHA1
#ifdef MESA_LLVM_VERSION_STRING
            " (LLVM " MESA_LLVM_VERSION_STRING ")"
#endif
           );

   /* VK_EXT_nested_command_buffer */
   p->maxCommandBufferNestingLevel = UINT32_MAX;

   /* VK_EXT_host_image_copy */
   lvp_device_get_cache_uuid(p->optimalTilingLayoutUUID);

   /* VK_EXT_vertex_attribute_divisor */
   if (device->pscreen->get_param(device->pscreen, PIPE_CAP_VERTEX_ELEMENT_INSTANCE_DIVISOR) != 0)
      p->maxVertexAttribDivisor = UINT32_MAX;
   else
      p->maxVertexAttribDivisor = 1;

   /* maintenance6 */
   p->blockTexelViewCompatibleMultipleLayers = true,

   /* VK_EXT_shader_object */
   /* this is basically unsupported */
   lvp_device_get_cache_uuid(p->shaderBinaryUUID);
   p->shaderBinaryVersion = 1;

   /* VK_EXT_mesh_shader */
   p->maxMeshPayloadAndSharedMemorySize = p->maxTaskPayloadSize + p->maxMeshSharedMemorySize; /* 28K min required */
   p->maxMeshPayloadAndOutputMemorySize = p->maxTaskPayloadSize + p->maxMeshOutputMemorySize; /* 47K min required */
}

static VkResult VKAPI_CALL
lvp_physical_device_init(struct lvp_physical_device *device,
                         struct lvp_instance *instance,
                         struct pipe_loader_device *pld)
{
   VkResult result;

   struct vk_physical_device_dispatch_table dispatch_table;
   vk_physical_device_dispatch_table_from_entrypoints(
      &dispatch_table, &lvp_physical_device_entrypoints, true);
   vk_physical_device_dispatch_table_from_entrypoints(
      &dispatch_table, &wsi_physical_device_entrypoints, false);
   result = vk_physical_device_init(&device->vk, &instance->vk,
                                    NULL, NULL, NULL, &dispatch_table);
   if (result != VK_SUCCESS) {
      vk_error(instance, result);
      goto fail;
   }
   device->pld = pld;

   device->pscreen = pipe_loader_create_screen_vk(device->pld, true);
   if (!device->pscreen)
      return vk_error(instance, VK_ERROR_OUT_OF_HOST_MEMORY);
   for (unsigned i = 0; i < ARRAY_SIZE(device->drv_options); i++)
      device->drv_options[i] = device->pscreen->get_compiler_options(device->pscreen, PIPE_SHADER_IR_NIR, i);

   device->sync_timeline_type = vk_sync_timeline_get_type(&lvp_pipe_sync_type);
   device->sync_types[0] = &lvp_pipe_sync_type;
   device->sync_types[1] = &device->sync_timeline_type.sync;
   device->sync_types[2] = NULL;
   device->vk.supported_sync_types = device->sync_types;

   device->max_images = device->pscreen->get_shader_param(device->pscreen, MESA_SHADER_FRAGMENT, PIPE_SHADER_CAP_MAX_SHADER_IMAGES);
   device->vk.supported_extensions = lvp_device_extensions_supported;
   lvp_get_features(device, &device->vk.supported_features);
   lvp_get_properties(device, &device->vk.properties);

   result = lvp_init_wsi(device);
   if (result != VK_SUCCESS) {
      vk_physical_device_finish(&device->vk);
      vk_error(instance, result);
      goto fail;
   }

   return VK_SUCCESS;
 fail:
   return result;
}

static void VKAPI_CALL
lvp_physical_device_finish(struct lvp_physical_device *device)
{
   lvp_finish_wsi(device);
   device->pscreen->destroy(device->pscreen);
   vk_physical_device_finish(&device->vk);
}

static void
lvp_destroy_physical_device(struct vk_physical_device *device)
{
   lvp_physical_device_finish((struct lvp_physical_device *)device);
   vk_free(&device->instance->alloc, device);
}

static VkResult
lvp_enumerate_physical_devices(struct vk_instance *vk_instance);

VKAPI_ATTR VkResult VKAPI_CALL lvp_CreateInstance(
   const VkInstanceCreateInfo*                 pCreateInfo,
   const VkAllocationCallbacks*                pAllocator,
   VkInstance*                                 pInstance)
{
   struct lvp_instance *instance;
   VkResult result;

   assert(pCreateInfo->sType == VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO);

   if (pAllocator == NULL)
      pAllocator = vk_default_allocator();

   instance = vk_zalloc(pAllocator, sizeof(*instance), 8,
                        VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
   if (!instance)
      return vk_error(NULL, VK_ERROR_OUT_OF_HOST_MEMORY);

   struct vk_instance_dispatch_table dispatch_table;
   vk_instance_dispatch_table_from_entrypoints(
      &dispatch_table, &lvp_instance_entrypoints, true);
   vk_instance_dispatch_table_from_entrypoints(
      &dispatch_table, &wsi_instance_entrypoints, false);

   result = vk_instance_init(&instance->vk,
                             &lvp_instance_extensions_supported,
                             &dispatch_table,
                             pCreateInfo,
                             pAllocator);
   if (result != VK_SUCCESS) {
      vk_free(pAllocator, instance);
      return vk_error(NULL, result);
   }

   instance->apiVersion = LVP_API_VERSION;

   instance->vk.physical_devices.enumerate = lvp_enumerate_physical_devices;
   instance->vk.physical_devices.destroy = lvp_destroy_physical_device;

   //   _mesa_locale_init();
   //   VG(VALGRIND_CREATE_MEMPOOL(instance, 0, false));

   *pInstance = lvp_instance_to_handle(instance);

   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL lvp_DestroyInstance(
   VkInstance                                  _instance,
   const VkAllocationCallbacks*                pAllocator)
{
   LVP_FROM_HANDLE(lvp_instance, instance, _instance);

   if (!instance)
      return;
   //   _mesa_locale_fini();

   pipe_loader_release(&instance->devs, instance->num_devices);

   vk_instance_finish(&instance->vk);
   vk_free(&instance->vk.alloc, instance);
}

#if defined(HAVE_DRI)
static void lvp_get_image(struct dri_drawable *dri_drawable,
                          int x, int y, unsigned width, unsigned height, unsigned stride,
                          void *data)
{

}

static void lvp_put_image(struct dri_drawable *dri_drawable,
                          void *data, unsigned width, unsigned height)
{
   fprintf(stderr, "put image %dx%d\n", width, height);
}

static void lvp_put_image2(struct dri_drawable *dri_drawable,
                           void *data, int x, int y, unsigned width, unsigned height,
                           unsigned stride)
{
   fprintf(stderr, "put image 2 %d,%d %dx%d\n", x, y, width, height);
}

static struct drisw_loader_funcs lvp_sw_lf = {
   .get_image = lvp_get_image,
   .put_image = lvp_put_image,
   .put_image2 = lvp_put_image2,
};
#endif

static VkResult
lvp_enumerate_physical_devices(struct vk_instance *vk_instance)
{
   struct lvp_instance *instance =
      container_of(vk_instance, struct lvp_instance, vk);

   /* sw only for now */
   instance->num_devices = pipe_loader_sw_probe(NULL, 0);

   assert(instance->num_devices == 1);

#if defined(HAVE_DRI)
   pipe_loader_sw_probe_dri(&instance->devs, &lvp_sw_lf);
#else
   pipe_loader_sw_probe_null(&instance->devs);
#endif

   struct lvp_physical_device *device =
      vk_zalloc2(&instance->vk.alloc, NULL, sizeof(*device), 8,
                 VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
   if (!device)
      return vk_error(instance, VK_ERROR_OUT_OF_HOST_MEMORY);

   VkResult result = lvp_physical_device_init(device, instance, &instance->devs[0]);
   if (result == VK_SUCCESS)
      list_addtail(&device->vk.link, &instance->vk.physical_devices.list);
   else
      vk_free(&vk_instance->alloc, device);

   return result;
}

void
lvp_device_get_cache_uuid(void *uuid)
{
   memset(uuid, 'a', VK_UUID_SIZE);
   if (MESA_GIT_SHA1[0])
      /* debug build */
      memcpy(uuid, &MESA_GIT_SHA1[4], MIN2(strlen(MESA_GIT_SHA1) - 4, VK_UUID_SIZE));
   else
      /* release build */
      memcpy(uuid, PACKAGE_VERSION, MIN2(strlen(PACKAGE_VERSION), VK_UUID_SIZE));
}

VKAPI_ATTR void VKAPI_CALL lvp_GetPhysicalDeviceQueueFamilyProperties2(
   VkPhysicalDevice                            physicalDevice,
   uint32_t*                                   pCount,
   VkQueueFamilyProperties2                   *pQueueFamilyProperties)
{
   VK_OUTARRAY_MAKE_TYPED(VkQueueFamilyProperties2, out, pQueueFamilyProperties, pCount);

   vk_outarray_append_typed(VkQueueFamilyProperties2, &out, p) {
      p->queueFamilyProperties = (VkQueueFamilyProperties) {
         .queueFlags = VK_QUEUE_GRAPHICS_BIT |
         VK_QUEUE_COMPUTE_BIT |
         VK_QUEUE_TRANSFER_BIT,
         .queueCount = 1,
         .timestampValidBits = 64,
         .minImageTransferGranularity = (VkExtent3D) { 1, 1, 1 },
      };
   }
}

VKAPI_ATTR void VKAPI_CALL lvp_GetPhysicalDeviceMemoryProperties(
   VkPhysicalDevice                            physicalDevice,
   VkPhysicalDeviceMemoryProperties*           pMemoryProperties)
{
   pMemoryProperties->memoryTypeCount = 1;
   pMemoryProperties->memoryTypes[0] = (VkMemoryType) {
      .propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
      VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
      VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
      .heapIndex = 0,
   };

   VkDeviceSize low_size = 3ULL*1024*1024*1024;
   VkDeviceSize total_size;
   os_get_total_physical_memory(&total_size);
   pMemoryProperties->memoryHeapCount = 1;
   pMemoryProperties->memoryHeaps[0] = (VkMemoryHeap) {
      .size = low_size,
      .flags = VK_MEMORY_HEAP_DEVICE_LOCAL_BIT,
   };
   if (sizeof(void*) > sizeof(uint32_t))
      pMemoryProperties->memoryHeaps[0].size = total_size;
}

VKAPI_ATTR void VKAPI_CALL lvp_GetPhysicalDeviceMemoryProperties2(
   VkPhysicalDevice                            physicalDevice,
   VkPhysicalDeviceMemoryProperties2          *pMemoryProperties)
{
   lvp_GetPhysicalDeviceMemoryProperties(physicalDevice,
                                         &pMemoryProperties->memoryProperties);
   VkPhysicalDeviceMemoryBudgetPropertiesEXT *props = vk_find_struct(pMemoryProperties, PHYSICAL_DEVICE_MEMORY_BUDGET_PROPERTIES_EXT);
   if (props) {
      props->heapBudget[0] = pMemoryProperties->memoryProperties.memoryHeaps[0].size;
      os_get_available_system_memory(&props->heapUsage[0]);
      props->heapUsage[0] = props->heapBudget[0] - props->heapUsage[0];
      memset(&props->heapBudget[1], 0, sizeof(props->heapBudget[0]) * (VK_MAX_MEMORY_HEAPS - 1));
      memset(&props->heapUsage[1], 0, sizeof(props->heapUsage[0]) * (VK_MAX_MEMORY_HEAPS - 1));
   }
}

VKAPI_ATTR VkResult VKAPI_CALL
lvp_GetMemoryHostPointerPropertiesEXT(
   VkDevice _device,
   VkExternalMemoryHandleTypeFlagBits handleType,
   const void *pHostPointer,
   VkMemoryHostPointerPropertiesEXT *pMemoryHostPointerProperties)
{
   switch (handleType) {
   case VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT: {
      pMemoryHostPointerProperties->memoryTypeBits = 1;
      return VK_SUCCESS;
   }
   default:
      return VK_ERROR_INVALID_EXTERNAL_HANDLE;
   }
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL lvp_GetInstanceProcAddr(
   VkInstance                                  _instance,
   const char*                                 pName)
{
   VK_FROM_HANDLE(vk_instance, instance, _instance);
   return vk_instance_get_proc_addr(instance,
                                    &lvp_instance_entrypoints,
                                    pName);
}

/* Windows will use a dll definition file to avoid build errors. */
#ifdef _WIN32
#undef PUBLIC
#define PUBLIC
#endif

/* The loader wants us to expose a second GetInstanceProcAddr function
 * to work around certain LD_PRELOAD issues seen in apps.
 */
PUBLIC
VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vk_icdGetInstanceProcAddr(
   VkInstance                                  instance,
   const char*                                 pName)
{
   return lvp_GetInstanceProcAddr(instance, pName);
}

static void
destroy_pipelines(struct lvp_queue *queue)
{
   simple_mtx_lock(&queue->lock);
   while (util_dynarray_contains(&queue->pipeline_destroys, struct lvp_pipeline*)) {
      lvp_pipeline_destroy(queue->device, util_dynarray_pop(&queue->pipeline_destroys, struct lvp_pipeline*), true);
   }
   simple_mtx_unlock(&queue->lock);
}

static VkResult
lvp_queue_submit(struct vk_queue *vk_queue,
                 struct vk_queue_submit *submit)
{
   struct lvp_queue *queue = container_of(vk_queue, struct lvp_queue, vk);

   VkResult result = vk_sync_wait_many(&queue->device->vk,
                                       submit->wait_count, submit->waits,
                                       VK_SYNC_WAIT_COMPLETE, UINT64_MAX);
   if (result != VK_SUCCESS)
      return result;

   simple_mtx_lock(&queue->lock);

   for (uint32_t i = 0; i < submit->command_buffer_count; i++) {
      struct lvp_cmd_buffer *cmd_buffer =
         container_of(submit->command_buffers[i], struct lvp_cmd_buffer, vk);

      lvp_execute_cmds(queue->device, queue, cmd_buffer);
   }

   simple_mtx_unlock(&queue->lock);

   if (submit->command_buffer_count > 0)
      queue->ctx->flush(queue->ctx, &queue->last_fence, 0);

   for (uint32_t i = 0; i < submit->signal_count; i++) {
      struct lvp_pipe_sync *sync =
         vk_sync_as_lvp_pipe_sync(submit->signals[i].sync);
      lvp_pipe_sync_signal_with_fence(queue->device, sync, queue->last_fence);
   }
   destroy_pipelines(queue);

   return VK_SUCCESS;
}

static VkResult
lvp_queue_init(struct lvp_device *device, struct lvp_queue *queue,
               const VkDeviceQueueCreateInfo *create_info,
               uint32_t index_in_family)
{
   VkResult result = vk_queue_init(&queue->vk, &device->vk, create_info,
                                   index_in_family);
   if (result != VK_SUCCESS)
      return result;

   result = vk_queue_enable_submit_thread(&queue->vk);
   if (result != VK_SUCCESS) {
      vk_queue_finish(&queue->vk);
      return result;
   }

   queue->device = device;

   queue->ctx = device->pscreen->context_create(device->pscreen, NULL, PIPE_CONTEXT_ROBUST_BUFFER_ACCESS);
   queue->cso = cso_create_context(queue->ctx, CSO_NO_VBUF);
   queue->uploader = u_upload_create(queue->ctx, 1024 * 1024, PIPE_BIND_CONSTANT_BUFFER, PIPE_USAGE_STREAM, 0);

   queue->vk.driver_submit = lvp_queue_submit;

   simple_mtx_init(&queue->lock, mtx_plain);
   util_dynarray_init(&queue->pipeline_destroys, NULL);

   return VK_SUCCESS;
}

static void
lvp_queue_finish(struct lvp_queue *queue)
{
   vk_queue_finish(&queue->vk);

   destroy_pipelines(queue);
   simple_mtx_destroy(&queue->lock);
   util_dynarray_fini(&queue->pipeline_destroys);

   u_upload_destroy(queue->uploader);
   cso_destroy_context(queue->cso);
   queue->ctx->destroy(queue->ctx);
}

VKAPI_ATTR VkResult VKAPI_CALL lvp_CreateDevice(
   VkPhysicalDevice                            physicalDevice,
   const VkDeviceCreateInfo*                   pCreateInfo,
   const VkAllocationCallbacks*                pAllocator,
   VkDevice*                                   pDevice)
{
   LVP_FROM_HANDLE(lvp_physical_device, physical_device, physicalDevice);
   struct lvp_device *device;
   struct lvp_instance *instance = (struct lvp_instance *)physical_device->vk.instance;

   assert(pCreateInfo->sType == VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO);

   size_t state_size = lvp_get_rendering_state_size();
   device = vk_zalloc2(&physical_device->vk.instance->alloc, pAllocator,
                       sizeof(*device) + state_size, 8,
                       VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
   if (!device)
      return vk_error(instance, VK_ERROR_OUT_OF_HOST_MEMORY);

   device->queue.state = device + 1;
   device->poison_mem = debug_get_bool_option("LVP_POISON_MEMORY", false);
   device->print_cmds = debug_get_bool_option("LVP_CMD_DEBUG", false);

   struct vk_device_dispatch_table dispatch_table;
   vk_device_dispatch_table_from_entrypoints(&dispatch_table,
      &lvp_device_entrypoints, true);
   lvp_add_enqueue_cmd_entrypoints(&dispatch_table);
   vk_device_dispatch_table_from_entrypoints(&dispatch_table,
      &wsi_device_entrypoints, false);
   VkResult result = vk_device_init(&device->vk,
                                    &physical_device->vk,
                                    &dispatch_table, pCreateInfo,
                                    pAllocator);
   if (result != VK_SUCCESS) {
      vk_free(&device->vk.alloc, device);
      return result;
   }

   vk_device_enable_threaded_submit(&device->vk);
   device->vk.command_buffer_ops = &lvp_cmd_buffer_ops;

   device->instance = (struct lvp_instance *)physical_device->vk.instance;
   device->physical_device = physical_device;

   device->pscreen = physical_device->pscreen;

   assert(pCreateInfo->queueCreateInfoCount == 1);
   assert(pCreateInfo->pQueueCreateInfos[0].queueFamilyIndex == 0);
   assert(pCreateInfo->pQueueCreateInfos[0].queueCount == 1);
   result = lvp_queue_init(device, &device->queue, pCreateInfo->pQueueCreateInfos, 0);
   if (result != VK_SUCCESS) {
      vk_free(&device->vk.alloc, device);
      return result;
   }

   nir_builder b = nir_builder_init_simple_shader(MESA_SHADER_FRAGMENT, NULL, "dummy_frag");
   struct pipe_shader_state shstate = {0};
   shstate.type = PIPE_SHADER_IR_NIR;
   shstate.ir.nir = b.shader;
   device->noop_fs = device->queue.ctx->create_fs_state(device->queue.ctx, &shstate);
   _mesa_hash_table_init(&device->bda, NULL, _mesa_hash_pointer, _mesa_key_pointer_equal);
   simple_mtx_init(&device->bda_lock, mtx_plain);

   uint32_t zero = 0;
   device->zero_buffer = pipe_buffer_create_with_data(device->queue.ctx, 0, PIPE_USAGE_IMMUTABLE, sizeof(uint32_t), &zero);

   device->null_texture_handle = (void *)(uintptr_t)device->queue.ctx->create_texture_handle(device->queue.ctx,
      &(struct pipe_sampler_view){ 0 }, NULL);
   device->null_image_handle = (void *)(uintptr_t)device->queue.ctx->create_image_handle(device->queue.ctx,
      &(struct pipe_image_view){ 0 });

   util_dynarray_init(&device->bda_texture_handles, NULL);
   util_dynarray_init(&device->bda_image_handles, NULL);

   *pDevice = lvp_device_to_handle(device);

   return VK_SUCCESS;

}

VKAPI_ATTR void VKAPI_CALL lvp_DestroyDevice(
   VkDevice                                    _device,
   const VkAllocationCallbacks*                pAllocator)
{
   LVP_FROM_HANDLE(lvp_device, device, _device);

   util_dynarray_foreach(&device->bda_texture_handles, struct lp_texture_handle *, handle)
      device->queue.ctx->delete_texture_handle(device->queue.ctx, (uint64_t)(uintptr_t)*handle);

   util_dynarray_fini(&device->bda_texture_handles);

   util_dynarray_foreach(&device->bda_image_handles, struct lp_texture_handle *, handle)
      device->queue.ctx->delete_image_handle(device->queue.ctx, (uint64_t)(uintptr_t)*handle);

   util_dynarray_fini(&device->bda_image_handles);

   device->queue.ctx->delete_texture_handle(device->queue.ctx, (uint64_t)(uintptr_t)device->null_texture_handle);
   device->queue.ctx->delete_image_handle(device->queue.ctx, (uint64_t)(uintptr_t)device->null_image_handle);

   device->queue.ctx->delete_fs_state(device->queue.ctx, device->noop_fs);

   if (device->queue.last_fence)
      device->pscreen->fence_reference(device->pscreen, &device->queue.last_fence, NULL);
   ralloc_free(device->bda.table);
   simple_mtx_destroy(&device->bda_lock);
   pipe_resource_reference(&device->zero_buffer, NULL);

   lvp_queue_finish(&device->queue);
   vk_device_finish(&device->vk);
   vk_free(&device->vk.alloc, device);
}

VKAPI_ATTR VkResult VKAPI_CALL lvp_EnumerateInstanceExtensionProperties(
   const char*                                 pLayerName,
   uint32_t*                                   pPropertyCount,
   VkExtensionProperties*                      pProperties)
{
   if (pLayerName)
      return vk_error(NULL, VK_ERROR_LAYER_NOT_PRESENT);

   return vk_enumerate_instance_extension_properties(
      &lvp_instance_extensions_supported, pPropertyCount, pProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL lvp_EnumerateInstanceLayerProperties(
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

VKAPI_ATTR VkResult VKAPI_CALL lvp_EnumerateDeviceLayerProperties(
   VkPhysicalDevice                            physicalDevice,
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

static void
set_mem_priority(struct lvp_device_memory *mem, int priority)
{
#if DETECT_OS_LINUX
   if (priority) {
      int advice = 0;
#ifdef MADV_COLD
      if (priority < 0)
         advice |= MADV_COLD;
#endif
      if (priority > 0)
         advice |= MADV_WILLNEED;
      if (advice)
         madvise(mem->pmem, mem->size, advice);
   }
#endif
}

static int
get_mem_priority(float priority)
{
   if (priority < 0.3)
      return -1;
   if (priority < 0.6)
      return 0;
   return priority = 1;
}

VKAPI_ATTR VkResult VKAPI_CALL lvp_AllocateMemory(
   VkDevice                                    _device,
   const VkMemoryAllocateInfo*                 pAllocateInfo,
   const VkAllocationCallbacks*                pAllocator,
   VkDeviceMemory*                             pMem)
{
   LVP_FROM_HANDLE(lvp_device, device, _device);
   struct lvp_device_memory *mem;
   ASSERTED const VkExportMemoryAllocateInfo *export_info = NULL;
   ASSERTED const VkImportMemoryFdInfoKHR *import_info = NULL;
   const VkImportMemoryHostPointerInfoEXT *host_ptr_info = NULL;
   VkResult error = VK_ERROR_OUT_OF_DEVICE_MEMORY;
   assert(pAllocateInfo->sType == VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);
   int priority = 0;

   if (pAllocateInfo->allocationSize == 0) {
      /* Apparently, this is allowed */
      *pMem = VK_NULL_HANDLE;
      return VK_SUCCESS;
   }

   vk_foreach_struct_const(ext, pAllocateInfo->pNext) {
      switch ((unsigned)ext->sType) {
      case VK_STRUCTURE_TYPE_IMPORT_MEMORY_HOST_POINTER_INFO_EXT:
         host_ptr_info = (VkImportMemoryHostPointerInfoEXT*)ext;
         assert(host_ptr_info->handleType == VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT);
         break;
      case VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO:
         export_info = (VkExportMemoryAllocateInfo*)ext;
         assert(!export_info->handleTypes || export_info->handleTypes == VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT);
         break;
      case VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR:
         import_info = (VkImportMemoryFdInfoKHR*)ext;
         assert(import_info->handleType == VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT);
         break;
      case VK_STRUCTURE_TYPE_MEMORY_PRIORITY_ALLOCATE_INFO_EXT: {
         VkMemoryPriorityAllocateInfoEXT *prio = (VkMemoryPriorityAllocateInfoEXT*)ext;
         priority = get_mem_priority(prio->priority);
         break;
      }
      default:
         break;
      }
   }

#ifdef PIPE_MEMORY_FD
   if (import_info != NULL && import_info->fd < 0) {
      return vk_error(device->instance, VK_ERROR_INVALID_EXTERNAL_HANDLE);
   }
#endif

   mem = vk_alloc2(&device->vk.alloc, pAllocator, sizeof(*mem), 8,
                   VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (mem == NULL)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   vk_object_base_init(&device->vk, &mem->base,
                       VK_OBJECT_TYPE_DEVICE_MEMORY);

   mem->memory_type = LVP_DEVICE_MEMORY_TYPE_DEFAULT;
   mem->backed_fd = -1;
   mem->size = pAllocateInfo->allocationSize;

   if (host_ptr_info) {
      mem->pmem = host_ptr_info->pHostPointer;
      mem->memory_type = LVP_DEVICE_MEMORY_TYPE_USER_PTR;
   }
#ifdef PIPE_MEMORY_FD
   else if(import_info) {
      uint64_t size;
      if(!device->pscreen->import_memory_fd(device->pscreen, import_info->fd, &mem->pmem, &size)) {
         close(import_info->fd);
         error = VK_ERROR_INVALID_EXTERNAL_HANDLE;
         goto fail;
      }
      if(size < pAllocateInfo->allocationSize) {
         device->pscreen->free_memory_fd(device->pscreen, mem->pmem);
         close(import_info->fd);
         goto fail;
      }
      if (export_info && export_info->handleTypes) {
         mem->backed_fd = import_info->fd;
      }
      else {
         close(import_info->fd);
      }
      mem->memory_type = LVP_DEVICE_MEMORY_TYPE_OPAQUE_FD;
   }
   else if (export_info && export_info->handleTypes) {
      mem->pmem = device->pscreen->allocate_memory_fd(device->pscreen, pAllocateInfo->allocationSize, &mem->backed_fd);
      if (!mem->pmem || mem->backed_fd < 0) {
         goto fail;
      }
      mem->memory_type = LVP_DEVICE_MEMORY_TYPE_OPAQUE_FD;
   }
#endif
   else {
      mem->pmem = device->pscreen->allocate_memory(device->pscreen, pAllocateInfo->allocationSize);
      if (!mem->pmem) {
         goto fail;
      }
      if (device->poison_mem)
         /* this is a value that will definitely break things */
         memset(mem->pmem, UINT8_MAX / 2 + 1, pAllocateInfo->allocationSize);
      set_mem_priority(mem, priority);
   }

   mem->type_index = pAllocateInfo->memoryTypeIndex;

   *pMem = lvp_device_memory_to_handle(mem);

   return VK_SUCCESS;

fail:
   vk_free2(&device->vk.alloc, pAllocator, mem);
   return vk_error(device, error);
}

VKAPI_ATTR void VKAPI_CALL lvp_FreeMemory(
   VkDevice                                    _device,
   VkDeviceMemory                              _mem,
   const VkAllocationCallbacks*                pAllocator)
{
   LVP_FROM_HANDLE(lvp_device, device, _device);
   LVP_FROM_HANDLE(lvp_device_memory, mem, _mem);

   if (mem == NULL)
      return;

   switch(mem->memory_type) {
   case LVP_DEVICE_MEMORY_TYPE_DEFAULT:
      device->pscreen->free_memory(device->pscreen, mem->pmem);
      break;
#ifdef PIPE_MEMORY_FD
   case LVP_DEVICE_MEMORY_TYPE_OPAQUE_FD:
      device->pscreen->free_memory_fd(device->pscreen, mem->pmem);
      if(mem->backed_fd >= 0)
         close(mem->backed_fd);
      break;
#endif
   case LVP_DEVICE_MEMORY_TYPE_USER_PTR:
   default:
      break;
   }
   vk_object_base_finish(&mem->base);
   vk_free2(&device->vk.alloc, pAllocator, mem);

}

VKAPI_ATTR VkResult VKAPI_CALL lvp_MapMemory2KHR(
    VkDevice                                    _device,
    const VkMemoryMapInfoKHR*                   pMemoryMapInfo,
    void**                                      ppData)
{
   LVP_FROM_HANDLE(lvp_device, device, _device);
   LVP_FROM_HANDLE(lvp_device_memory, mem, pMemoryMapInfo->memory);
   void *map;
   if (mem == NULL) {
      *ppData = NULL;
      return VK_SUCCESS;
   }

   map = device->pscreen->map_memory(device->pscreen, mem->pmem);

   *ppData = (char *)map + pMemoryMapInfo->offset;
   return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL lvp_UnmapMemory2KHR(
    VkDevice                                    _device,
    const VkMemoryUnmapInfoKHR*                 pMemoryUnmapInfo)
{
   LVP_FROM_HANDLE(lvp_device, device, _device);
   LVP_FROM_HANDLE(lvp_device_memory, mem, pMemoryUnmapInfo->memory);

   if (mem == NULL)
      return VK_SUCCESS;

   device->pscreen->unmap_memory(device->pscreen, mem->pmem);
   return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL lvp_FlushMappedMemoryRanges(
   VkDevice                                    _device,
   uint32_t                                    memoryRangeCount,
   const VkMappedMemoryRange*                  pMemoryRanges)
{
   return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL lvp_InvalidateMappedMemoryRanges(
   VkDevice                                    _device,
   uint32_t                                    memoryRangeCount,
   const VkMappedMemoryRange*                  pMemoryRanges)
{
   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL lvp_GetDeviceBufferMemoryRequirements(
    VkDevice                                    _device,
    const VkDeviceBufferMemoryRequirements*     pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements)
{
   pMemoryRequirements->memoryRequirements.memoryTypeBits = 1;
   pMemoryRequirements->memoryRequirements.alignment = 64;
   pMemoryRequirements->memoryRequirements.size = 0;

   VkBuffer _buffer;
   if (lvp_CreateBuffer(_device, pInfo->pCreateInfo, NULL, &_buffer) != VK_SUCCESS)
      return;
   LVP_FROM_HANDLE(lvp_buffer, buffer, _buffer);
   pMemoryRequirements->memoryRequirements.size = buffer->total_size;
   lvp_DestroyBuffer(_device, _buffer, NULL);
}

VKAPI_ATTR void VKAPI_CALL lvp_GetDeviceImageSparseMemoryRequirements(
    VkDevice                                    device,
    const VkDeviceImageMemoryRequirements*      pInfo,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2*           pSparseMemoryRequirements)
{
   stub();
}

VKAPI_ATTR void VKAPI_CALL lvp_GetDeviceImageMemoryRequirements(
    VkDevice                                    _device,
    const VkDeviceImageMemoryRequirements*     pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements)
{
   pMemoryRequirements->memoryRequirements.memoryTypeBits = 1;
   pMemoryRequirements->memoryRequirements.alignment = 0;
   pMemoryRequirements->memoryRequirements.size = 0;

   VkImage _image;
   if (lvp_CreateImage(_device, pInfo->pCreateInfo, NULL, &_image) != VK_SUCCESS)
      return;
   LVP_FROM_HANDLE(lvp_image, image, _image);
   pMemoryRequirements->memoryRequirements.size = image->size;
   pMemoryRequirements->memoryRequirements.alignment = image->alignment;
   lvp_DestroyImage(_device, _image, NULL);
}

VKAPI_ATTR void VKAPI_CALL lvp_GetBufferMemoryRequirements(
   VkDevice                                    device,
   VkBuffer                                    _buffer,
   VkMemoryRequirements*                       pMemoryRequirements)
{
   LVP_FROM_HANDLE(lvp_buffer, buffer, _buffer);

   /* The Vulkan spec (git aaed022) says:
    *
    *    memoryTypeBits is a bitfield and contains one bit set for every
    *    supported memory type for the resource. The bit `1<<i` is set if and
    *    only if the memory type `i` in the VkPhysicalDeviceMemoryProperties
    *    structure for the physical device is supported.
    *
    * We support exactly one memory type.
    */
   pMemoryRequirements->memoryTypeBits = 1;

   pMemoryRequirements->size = buffer->total_size;
   pMemoryRequirements->alignment = 64;
}

VKAPI_ATTR void VKAPI_CALL lvp_GetBufferMemoryRequirements2(
   VkDevice                                     device,
   const VkBufferMemoryRequirementsInfo2       *pInfo,
   VkMemoryRequirements2                       *pMemoryRequirements)
{
   lvp_GetBufferMemoryRequirements(device, pInfo->buffer,
                                   &pMemoryRequirements->memoryRequirements);
   vk_foreach_struct(ext, pMemoryRequirements->pNext) {
      switch (ext->sType) {
      case VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS: {
         VkMemoryDedicatedRequirements *req =
            (VkMemoryDedicatedRequirements *) ext;
         req->requiresDedicatedAllocation = false;
         req->prefersDedicatedAllocation = req->requiresDedicatedAllocation;
         break;
      }
      default:
         break;
      }
   }
}

VKAPI_ATTR void VKAPI_CALL lvp_GetImageMemoryRequirements(
   VkDevice                                    device,
   VkImage                                     _image,
   VkMemoryRequirements*                       pMemoryRequirements)
{
   LVP_FROM_HANDLE(lvp_image, image, _image);
   pMemoryRequirements->memoryTypeBits = 1;

   pMemoryRequirements->size = image->size;
   pMemoryRequirements->alignment = image->alignment;
}

VKAPI_ATTR void VKAPI_CALL lvp_GetImageMemoryRequirements2(
   VkDevice                                    device,
   const VkImageMemoryRequirementsInfo2       *pInfo,
   VkMemoryRequirements2                      *pMemoryRequirements)
{
   lvp_GetImageMemoryRequirements(device, pInfo->image,
                                  &pMemoryRequirements->memoryRequirements);

   vk_foreach_struct(ext, pMemoryRequirements->pNext) {
      switch (ext->sType) {
      case VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS: {
         VkMemoryDedicatedRequirements *req =
            (VkMemoryDedicatedRequirements *) ext;
         req->requiresDedicatedAllocation = false;
         req->prefersDedicatedAllocation = req->requiresDedicatedAllocation;
         break;
      }
      default:
         break;
      }
   }
}

VKAPI_ATTR void VKAPI_CALL lvp_GetImageSparseMemoryRequirements(
   VkDevice                                    device,
   VkImage                                     image,
   uint32_t*                                   pSparseMemoryRequirementCount,
   VkSparseImageMemoryRequirements*            pSparseMemoryRequirements)
{
   stub();
}

VKAPI_ATTR void VKAPI_CALL lvp_GetImageSparseMemoryRequirements2(
   VkDevice                                    device,
   const VkImageSparseMemoryRequirementsInfo2* pInfo,
   uint32_t* pSparseMemoryRequirementCount,
   VkSparseImageMemoryRequirements2* pSparseMemoryRequirements)
{
   stub();
}

VKAPI_ATTR void VKAPI_CALL lvp_GetDeviceMemoryCommitment(
   VkDevice                                    device,
   VkDeviceMemory                              memory,
   VkDeviceSize*                               pCommittedMemoryInBytes)
{
   *pCommittedMemoryInBytes = 0;
}

VKAPI_ATTR VkResult VKAPI_CALL lvp_BindBufferMemory2(VkDevice _device,
                               uint32_t bindInfoCount,
                               const VkBindBufferMemoryInfo *pBindInfos)
{
   LVP_FROM_HANDLE(lvp_device, device, _device);
   for (uint32_t i = 0; i < bindInfoCount; ++i) {
      LVP_FROM_HANDLE(lvp_device_memory, mem, pBindInfos[i].memory);
      LVP_FROM_HANDLE(lvp_buffer, buffer, pBindInfos[i].buffer);
      VkBindMemoryStatusKHR *status = (void*)vk_find_struct_const(&pBindInfos[i], BIND_MEMORY_STATUS_KHR);

      buffer->pmem = mem->pmem;
      buffer->offset = pBindInfos[i].memoryOffset;
      device->pscreen->resource_bind_backing(device->pscreen,
                                             buffer->bo,
                                             mem->pmem,
                                             pBindInfos[i].memoryOffset);
      if (status)
         *status->pResult = VK_SUCCESS;
   }
   return VK_SUCCESS;
}

static VkResult
lvp_image_plane_bind(struct lvp_device *device,
                     struct lvp_image_plane *plane,
                     struct lvp_device_memory *mem,
                     VkDeviceSize memory_offset,
                     VkDeviceSize *plane_offset)
{
   if (!device->pscreen->resource_bind_backing(device->pscreen,
                                               plane->bo,
                                               mem->pmem,
                                               memory_offset + *plane_offset)) {
      /* This is probably caused by the texture being too large, so let's
       * report this as the *closest* allowed error-code. It's not ideal,
       * but it's unlikely that anyone will care too much.
       */
      return vk_error(device, VK_ERROR_OUT_OF_DEVICE_MEMORY);
   }
   plane->pmem = mem->pmem;
   plane->memory_offset = memory_offset;
   plane->plane_offset = *plane_offset;
   *plane_offset += plane->size;
   return VK_SUCCESS;
}


VKAPI_ATTR VkResult VKAPI_CALL lvp_BindImageMemory2(VkDevice _device,
                              uint32_t bindInfoCount,
                              const VkBindImageMemoryInfo *pBindInfos)
{
   LVP_FROM_HANDLE(lvp_device, device, _device);
   VkResult res = VK_SUCCESS;
   for (uint32_t i = 0; i < bindInfoCount; ++i) {
      const VkBindImageMemoryInfo *bind_info = &pBindInfos[i];
      LVP_FROM_HANDLE(lvp_device_memory, mem, bind_info->memory);
      LVP_FROM_HANDLE(lvp_image, image, bind_info->image);
      VkBindMemoryStatusKHR *status = (void*)vk_find_struct_const(&pBindInfos[i], BIND_MEMORY_STATUS_KHR);
      bool did_bind = false;

      vk_foreach_struct_const(s, bind_info->pNext) {
         switch (s->sType) {
         case VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_SWAPCHAIN_INFO_KHR: {
            const VkBindImageMemorySwapchainInfoKHR *swapchain_info =
               (const VkBindImageMemorySwapchainInfoKHR *) s;
            struct lvp_image *swapchain_image =
               lvp_swapchain_get_image(swapchain_info->swapchain,
                                       swapchain_info->imageIndex);

            image->planes[0].pmem = swapchain_image->planes[0].pmem;
            image->planes[0].memory_offset = swapchain_image->planes[0].memory_offset;
            device->pscreen->resource_bind_backing(device->pscreen,
                                                   image->planes[0].bo,
                                                   image->planes[0].pmem,
                                                   image->planes[0].memory_offset);
            did_bind = true;
            if (status)
               *status->pResult = VK_SUCCESS;
            break;
         }
         default:
            break;
         }
      }

      if (!did_bind) {
         uint64_t offset_B = 0;
         VkResult result;
         if (image->disjoint) {
            const VkBindImagePlaneMemoryInfo *plane_info =
               vk_find_struct_const(pBindInfos[i].pNext, BIND_IMAGE_PLANE_MEMORY_INFO);
            uint8_t plane = lvp_image_aspects_to_plane(image, plane_info->planeAspect);
            result = lvp_image_plane_bind(device, &image->planes[plane],
                                          mem, bind_info->memoryOffset, &offset_B);
            if (status)
               *status->pResult = result;
            if (result != VK_SUCCESS)
               return result;
         } else {
            VkResult fail = VK_SUCCESS;
            for (unsigned plane = 0; plane < image->plane_count; plane++) {
               result = lvp_image_plane_bind(device, &image->planes[plane],
                                             mem, bind_info->memoryOffset, &offset_B);
               if (status)
                  *status->pResult = res;
               if (result != VK_SUCCESS)
                  fail = result;
            }
            if (fail != VK_SUCCESS)
               return fail;
         }
      }
   }
   return res;
}

#ifdef PIPE_MEMORY_FD

VkResult
lvp_GetMemoryFdKHR(VkDevice _device, const VkMemoryGetFdInfoKHR *pGetFdInfo, int *pFD)
{
   LVP_FROM_HANDLE(lvp_device_memory, memory, pGetFdInfo->memory);

   assert(pGetFdInfo->sType == VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR);
   assert(pGetFdInfo->handleType == VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT);

   *pFD = dup(memory->backed_fd);
   assert(*pFD >= 0);
   return VK_SUCCESS;
}

VkResult
lvp_GetMemoryFdPropertiesKHR(VkDevice _device,
                             VkExternalMemoryHandleTypeFlagBits handleType,
                             int fd,
                             VkMemoryFdPropertiesKHR *pMemoryFdProperties)
{
   LVP_FROM_HANDLE(lvp_device, device, _device);

   assert(pMemoryFdProperties->sType == VK_STRUCTURE_TYPE_MEMORY_FD_PROPERTIES_KHR);

   if(handleType == VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT) {
      // There is only one memoryType so select this one
      pMemoryFdProperties->memoryTypeBits = 1;
   }
   else
      return vk_error(device->instance, VK_ERROR_INVALID_EXTERNAL_HANDLE);
   return VK_SUCCESS;
}

#endif

VKAPI_ATTR VkResult VKAPI_CALL lvp_QueueBindSparse(
   VkQueue                                     queue,
   uint32_t                                    bindInfoCount,
   const VkBindSparseInfo*                     pBindInfo,
   VkFence                                     fence)
{
   stub_return(VK_ERROR_INCOMPATIBLE_DRIVER);
}

VKAPI_ATTR VkResult VKAPI_CALL lvp_CreateEvent(
   VkDevice                                    _device,
   const VkEventCreateInfo*                    pCreateInfo,
   const VkAllocationCallbacks*                pAllocator,
   VkEvent*                                    pEvent)
{
   LVP_FROM_HANDLE(lvp_device, device, _device);
   struct lvp_event *event = vk_alloc2(&device->vk.alloc, pAllocator,
                                       sizeof(*event), 8,
                                       VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

   if (!event)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   vk_object_base_init(&device->vk, &event->base, VK_OBJECT_TYPE_EVENT);
   *pEvent = lvp_event_to_handle(event);
   event->event_storage = 0;

   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL lvp_DestroyEvent(
   VkDevice                                    _device,
   VkEvent                                     _event,
   const VkAllocationCallbacks*                pAllocator)
{
   LVP_FROM_HANDLE(lvp_device, device, _device);
   LVP_FROM_HANDLE(lvp_event, event, _event);

   if (!event)
      return;

   vk_object_base_finish(&event->base);
   vk_free2(&device->vk.alloc, pAllocator, event);
}

VKAPI_ATTR VkResult VKAPI_CALL lvp_GetEventStatus(
   VkDevice                                    _device,
   VkEvent                                     _event)
{
   LVP_FROM_HANDLE(lvp_event, event, _event);
   if (event->event_storage == 1)
      return VK_EVENT_SET;
   return VK_EVENT_RESET;
}

VKAPI_ATTR VkResult VKAPI_CALL lvp_SetEvent(
   VkDevice                                    _device,
   VkEvent                                     _event)
{
   LVP_FROM_HANDLE(lvp_event, event, _event);
   event->event_storage = 1;

   return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL lvp_ResetEvent(
   VkDevice                                    _device,
   VkEvent                                     _event)
{
   LVP_FROM_HANDLE(lvp_event, event, _event);
   event->event_storage = 0;

   return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL lvp_CreateSampler(
   VkDevice                                    _device,
   const VkSamplerCreateInfo*                  pCreateInfo,
   const VkAllocationCallbacks*                pAllocator,
   VkSampler*                                  pSampler)
{
   LVP_FROM_HANDLE(lvp_device, device, _device);
   struct lvp_sampler *sampler;

   sampler = vk_sampler_create(&device->vk, pCreateInfo,
                               pAllocator, sizeof(*sampler));
   if (!sampler)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   struct pipe_sampler_state state = {0};
   VkClearColorValue border_color =
      vk_sampler_border_color_value(pCreateInfo, NULL);
   STATIC_ASSERT(sizeof(state.border_color) == sizeof(border_color));

   state.wrap_s = vk_conv_wrap_mode(pCreateInfo->addressModeU);
   state.wrap_t = vk_conv_wrap_mode(pCreateInfo->addressModeV);
   state.wrap_r = vk_conv_wrap_mode(pCreateInfo->addressModeW);
   state.min_img_filter = pCreateInfo->minFilter == VK_FILTER_LINEAR ? PIPE_TEX_FILTER_LINEAR : PIPE_TEX_FILTER_NEAREST;
   state.min_mip_filter = pCreateInfo->mipmapMode == VK_SAMPLER_MIPMAP_MODE_LINEAR ? PIPE_TEX_MIPFILTER_LINEAR : PIPE_TEX_MIPFILTER_NEAREST;
   state.mag_img_filter = pCreateInfo->magFilter == VK_FILTER_LINEAR ? PIPE_TEX_FILTER_LINEAR : PIPE_TEX_FILTER_NEAREST;
   state.min_lod = pCreateInfo->minLod;
   state.max_lod = pCreateInfo->maxLod;
   state.lod_bias = pCreateInfo->mipLodBias;
   if (pCreateInfo->anisotropyEnable)
      state.max_anisotropy = pCreateInfo->maxAnisotropy;
   else
      state.max_anisotropy = 1;
   state.unnormalized_coords = pCreateInfo->unnormalizedCoordinates;
   state.compare_mode = pCreateInfo->compareEnable ? PIPE_TEX_COMPARE_R_TO_TEXTURE : PIPE_TEX_COMPARE_NONE;
   state.compare_func = pCreateInfo->compareOp;
   state.seamless_cube_map = !(pCreateInfo->flags & VK_SAMPLER_CREATE_NON_SEAMLESS_CUBE_MAP_BIT_EXT);
   STATIC_ASSERT((unsigned)VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE == (unsigned)PIPE_TEX_REDUCTION_WEIGHTED_AVERAGE);
   STATIC_ASSERT((unsigned)VK_SAMPLER_REDUCTION_MODE_MIN == (unsigned)PIPE_TEX_REDUCTION_MIN);
   STATIC_ASSERT((unsigned)VK_SAMPLER_REDUCTION_MODE_MAX == (unsigned)PIPE_TEX_REDUCTION_MAX);
   state.reduction_mode = (enum pipe_tex_reduction_mode)sampler->vk.reduction_mode;
   memcpy(&state.border_color, &border_color, sizeof(border_color));

   simple_mtx_lock(&device->queue.lock);
   sampler->texture_handle = (void *)(uintptr_t)device->queue.ctx->create_texture_handle(device->queue.ctx, NULL, &state);
   simple_mtx_unlock(&device->queue.lock);

   lp_jit_sampler_from_pipe(&sampler->desc.sampler, &state);
   sampler->desc.texture.sampler_index = sampler->texture_handle->sampler_index;

   *pSampler = lvp_sampler_to_handle(sampler);

   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL lvp_DestroySampler(
   VkDevice                                    _device,
   VkSampler                                   _sampler,
   const VkAllocationCallbacks*                pAllocator)
{
   LVP_FROM_HANDLE(lvp_device, device, _device);
   LVP_FROM_HANDLE(lvp_sampler, sampler, _sampler);

   if (!_sampler)
      return;

   simple_mtx_lock(&device->queue.lock);
   device->queue.ctx->delete_texture_handle(device->queue.ctx, (uint64_t)(uintptr_t)sampler->texture_handle);
   simple_mtx_unlock(&device->queue.lock);

   vk_sampler_destroy(&device->vk, pAllocator, &sampler->vk);
}

VKAPI_ATTR VkResult VKAPI_CALL lvp_CreatePrivateDataSlot(
   VkDevice                                    _device,
   const VkPrivateDataSlotCreateInfo*          pCreateInfo,
   const VkAllocationCallbacks*                pAllocator,
   VkPrivateDataSlot*                          pPrivateDataSlot)
{
   LVP_FROM_HANDLE(lvp_device, device, _device);
   return vk_private_data_slot_create(&device->vk, pCreateInfo, pAllocator,
                                      pPrivateDataSlot);
}

VKAPI_ATTR void VKAPI_CALL lvp_DestroyPrivateDataSlot(
   VkDevice                                    _device,
   VkPrivateDataSlot                           privateDataSlot,
   const VkAllocationCallbacks*                pAllocator)
{
   LVP_FROM_HANDLE(lvp_device, device, _device);
   vk_private_data_slot_destroy(&device->vk, privateDataSlot, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL lvp_SetPrivateData(
   VkDevice                                    _device,
   VkObjectType                                objectType,
   uint64_t                                    objectHandle,
   VkPrivateDataSlot                           privateDataSlot,
   uint64_t                                    data)
{
   LVP_FROM_HANDLE(lvp_device, device, _device);
   return vk_object_base_set_private_data(&device->vk, objectType,
                                          objectHandle, privateDataSlot,
                                          data);
}

VKAPI_ATTR void VKAPI_CALL lvp_GetPrivateData(
   VkDevice                                    _device,
   VkObjectType                                objectType,
   uint64_t                                    objectHandle,
   VkPrivateDataSlot                           privateDataSlot,
   uint64_t*                                   pData)
{
   LVP_FROM_HANDLE(lvp_device, device, _device);
   vk_object_base_get_private_data(&device->vk, objectType, objectHandle,
                                   privateDataSlot, pData);
}

VKAPI_ATTR VkResult VKAPI_CALL lvp_CreateIndirectCommandsLayoutNV(
    VkDevice                                    _device,
    const VkIndirectCommandsLayoutCreateInfoNV* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkIndirectCommandsLayoutNV*                 pIndirectCommandsLayout)
{
   LVP_FROM_HANDLE(lvp_device, device, _device);
   struct lvp_indirect_command_layout *dlayout;

   size_t size = sizeof(*dlayout) + pCreateInfo->tokenCount * sizeof(VkIndirectCommandsLayoutTokenNV);

   dlayout =
      vk_zalloc2(&device->vk.alloc, pAllocator, size, alignof(struct lvp_indirect_command_layout),
                VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (!dlayout)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   vk_object_base_init(&device->vk, &dlayout->base, VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NV);

   dlayout->stream_count = pCreateInfo->streamCount;
   dlayout->token_count = pCreateInfo->tokenCount;
   for (unsigned i = 0; i < pCreateInfo->streamCount; i++)
      dlayout->stream_strides[i] = pCreateInfo->pStreamStrides[i];
   typed_memcpy(dlayout->tokens, pCreateInfo->pTokens, pCreateInfo->tokenCount);

   *pIndirectCommandsLayout = lvp_indirect_command_layout_to_handle(dlayout);
   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL lvp_DestroyIndirectCommandsLayoutNV(
    VkDevice                                    _device,
    VkIndirectCommandsLayoutNV                  indirectCommandsLayout,
    const VkAllocationCallbacks*                pAllocator)
{
   LVP_FROM_HANDLE(lvp_device, device, _device);
   VK_FROM_HANDLE(lvp_indirect_command_layout, layout, indirectCommandsLayout);

   if (!layout)
      return;

   vk_object_base_finish(&layout->base);
   vk_free2(&device->vk.alloc, pAllocator, layout);
}

enum vk_cmd_type
lvp_nv_dgc_token_to_cmd_type(const VkIndirectCommandsLayoutTokenNV *token)
{
   switch (token->tokenType) {
      case VK_INDIRECT_COMMANDS_TOKEN_TYPE_SHADER_GROUP_NV:
         return VK_CMD_BIND_PIPELINE_SHADER_GROUP_NV;
      case VK_INDIRECT_COMMANDS_TOKEN_TYPE_STATE_FLAGS_NV:
         if (token->indirectStateFlags & VK_INDIRECT_STATE_FLAG_FRONTFACE_BIT_NV) {
            return VK_CMD_SET_FRONT_FACE;
         }
         assert(!"unknown token type!");
         break;
      case VK_INDIRECT_COMMANDS_TOKEN_TYPE_PUSH_CONSTANT_NV:
         return VK_CMD_PUSH_CONSTANTS2_KHR;
      case VK_INDIRECT_COMMANDS_TOKEN_TYPE_INDEX_BUFFER_NV:
         return VK_CMD_BIND_INDEX_BUFFER;
      case VK_INDIRECT_COMMANDS_TOKEN_TYPE_VERTEX_BUFFER_NV:
        return VK_CMD_BIND_VERTEX_BUFFERS2;
      case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_INDEXED_NV:
         return VK_CMD_DRAW_INDEXED_INDIRECT;
      case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_NV:
         return VK_CMD_DRAW_INDIRECT;
      // only available if VK_EXT_mesh_shader is supported
      case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_NV:
         return VK_CMD_DRAW_MESH_TASKS_INDIRECT_EXT;
      // only available if VK_NV_mesh_shader is supported
      case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_TASKS_NV:
         unreachable("NV_mesh_shader unsupported!");
      default:
         unreachable("unknown token type");
   }
   return UINT32_MAX;
}

VKAPI_ATTR void VKAPI_CALL lvp_GetGeneratedCommandsMemoryRequirementsNV(
    VkDevice                                    device,
    const VkGeneratedCommandsMemoryRequirementsInfoNV* pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements)
{
   VK_FROM_HANDLE(lvp_indirect_command_layout, dlayout, pInfo->indirectCommandsLayout);

   size_t size = sizeof(struct list_head);

   for (unsigned i = 0; i < dlayout->token_count; i++) {
      const VkIndirectCommandsLayoutTokenNV *token = &dlayout->tokens[i];
      UNUSED struct vk_cmd_queue_entry *cmd;
      enum vk_cmd_type type = lvp_nv_dgc_token_to_cmd_type(token);
      size += vk_cmd_queue_type_sizes[type];

      switch (token->tokenType) {
      case VK_INDIRECT_COMMANDS_TOKEN_TYPE_VERTEX_BUFFER_NV:
         size += sizeof(*cmd->u.bind_vertex_buffers.buffers);
         size += sizeof(*cmd->u.bind_vertex_buffers.offsets);
         size += sizeof(*cmd->u.bind_vertex_buffers2.sizes) + sizeof(*cmd->u.bind_vertex_buffers2.strides);
         break;
      case VK_INDIRECT_COMMANDS_TOKEN_TYPE_PUSH_CONSTANT_NV:
         size += token->pushconstantSize + sizeof(VkPushConstantsInfoKHR);
         break;
      case VK_INDIRECT_COMMANDS_TOKEN_TYPE_SHADER_GROUP_NV:
      case VK_INDIRECT_COMMANDS_TOKEN_TYPE_INDEX_BUFFER_NV:
      case VK_INDIRECT_COMMANDS_TOKEN_TYPE_STATE_FLAGS_NV:
      case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_INDEXED_NV:
      case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_NV:
      case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_TASKS_NV:
      case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_NV:
         break;
      default:
         unreachable("unknown type!");
      }
   }

   size *= pInfo->maxSequencesCount;

   pMemoryRequirements->memoryRequirements.memoryTypeBits = 1;
   pMemoryRequirements->memoryRequirements.alignment = 4;
   pMemoryRequirements->memoryRequirements.size = align(size, pMemoryRequirements->memoryRequirements.alignment);
}

VKAPI_ATTR void VKAPI_CALL lvp_GetPhysicalDeviceExternalFenceProperties(
   VkPhysicalDevice                           physicalDevice,
   const VkPhysicalDeviceExternalFenceInfo    *pExternalFenceInfo,
   VkExternalFenceProperties                  *pExternalFenceProperties)
{
   pExternalFenceProperties->exportFromImportedHandleTypes = 0;
   pExternalFenceProperties->compatibleHandleTypes = 0;
   pExternalFenceProperties->externalFenceFeatures = 0;
}

VKAPI_ATTR void VKAPI_CALL lvp_GetPhysicalDeviceExternalSemaphoreProperties(
   VkPhysicalDevice                            physicalDevice,
   const VkPhysicalDeviceExternalSemaphoreInfo *pExternalSemaphoreInfo,
   VkExternalSemaphoreProperties               *pExternalSemaphoreProperties)
{
   pExternalSemaphoreProperties->exportFromImportedHandleTypes = 0;
   pExternalSemaphoreProperties->compatibleHandleTypes = 0;
   pExternalSemaphoreProperties->externalSemaphoreFeatures = 0;
}

static const VkTimeDomainEXT lvp_time_domains[] = {
        VK_TIME_DOMAIN_DEVICE_EXT,
        VK_TIME_DOMAIN_CLOCK_MONOTONIC_EXT,
};

VKAPI_ATTR VkResult VKAPI_CALL lvp_GetPhysicalDeviceCalibrateableTimeDomainsEXT(
   VkPhysicalDevice physicalDevice,
   uint32_t *pTimeDomainCount,
   VkTimeDomainEXT *pTimeDomains)
{
   int d;
   VK_OUTARRAY_MAKE_TYPED(VkTimeDomainEXT, out, pTimeDomains,
                          pTimeDomainCount);

   for (d = 0; d < ARRAY_SIZE(lvp_time_domains); d++) {
      vk_outarray_append_typed(VkTimeDomainEXT, &out, i) {
         *i = lvp_time_domains[d];
      }
    }

    return vk_outarray_status(&out);
}

VKAPI_ATTR VkResult VKAPI_CALL lvp_GetCalibratedTimestampsEXT(
   VkDevice device,
   uint32_t timestampCount,
   const VkCalibratedTimestampInfoEXT *pTimestampInfos,
   uint64_t *pTimestamps,
   uint64_t *pMaxDeviation)
{
   *pMaxDeviation = 1;

   uint64_t now = os_time_get_nano();
   for (unsigned i = 0; i < timestampCount; i++) {
      pTimestamps[i] = now;
   }
   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL lvp_GetDeviceGroupPeerMemoryFeatures(
    VkDevice device,
    uint32_t heapIndex,
    uint32_t localDeviceIndex,
    uint32_t remoteDeviceIndex,
    VkPeerMemoryFeatureFlags *pPeerMemoryFeatures)
{
   *pPeerMemoryFeatures = 0;
}

VKAPI_ATTR void VKAPI_CALL lvp_SetDeviceMemoryPriorityEXT(
    VkDevice                                    _device,
    VkDeviceMemory                              _memory,
    float                                       priority)
{
   LVP_FROM_HANDLE(lvp_device_memory, mem, _memory);
   set_mem_priority(mem, get_mem_priority(priority));
}

VKAPI_ATTR void VKAPI_CALL lvp_GetRenderingAreaGranularityKHR(
    VkDevice                                    device,
    const VkRenderingAreaInfoKHR*               pRenderingAreaInfo,
    VkExtent2D*                                 pGranularity)
{
   VkExtent2D tile_size = {64, 64};
   *pGranularity = tile_size;
}
