/*
 * Copyright 2019 Google LLC
 * SPDX-License-Identifier: MIT
 *
 * based in part on anv and radv which are:
 * Copyright © 2015 Intel Corporation
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 */

#include "vn_pipeline.h"

#include "venus-protocol/vn_protocol_driver_pipeline.h"
#include "venus-protocol/vn_protocol_driver_pipeline_cache.h"
#include "venus-protocol/vn_protocol_driver_pipeline_layout.h"
#include "venus-protocol/vn_protocol_driver_shader_module.h"

#include "vn_descriptor_set.h"
#include "vn_device.h"
#include "vn_physical_device.h"
#include "vn_render_pass.h"

/**
 * Fields in the VkGraphicsPipelineCreateInfo pNext chain that we must track
 * to determine which fields are valid and which must be erased.
 */
struct vn_graphics_pipeline_info_self {
   union {
      /* Bitmask exists for testing if any field is set. */
      uint32_t mask;

      /* Group the fixes by Vulkan struct. Within each group, sort by struct
       * order.
       */
      struct {
         /** VkGraphicsPipelineCreateInfo::pStages */
         bool shader_stages : 1;
         /** VkGraphicsPipelineCreateInfo::pVertexInputState */
         bool vertex_input_state : 1;
         /** VkGraphicsPipelineCreateInfo::pInputAssemblyState */
         bool input_assembly_state : 1;
         /** VkGraphicsPipelineCreateInfo::pTessellationState */
         bool tessellation_state : 1;
         /** VkGraphicsPipelineCreateInfo::pViewportState */
         bool viewport_state : 1;
         /** VkGraphicsPipelineCreateInfo::pRasterizationState */
         bool rasterization_state : 1;
         /** VkGraphicsPipelineCreateInfo::pMultisampleState */
         bool multisample_state : 1;
         /** VkGraphicsPipelineCreateInfo::pDepthStencilState */
         bool depth_stencil_state : 1;
         /** VkGraphicsPipelineCreateInfo::pColorBlendState */
         bool color_blend_state : 1;
         /** VkGraphicsPipelineCreateInfo::layout */
         bool pipeline_layout : 1;
         /** VkGraphicsPipelineCreateInfo::renderPass */
         bool render_pass : 1;
         /** VkGraphicsPipelineCreateInfo::basePipelineHandle */
         bool base_pipeline_handle : 1;

         /** VkPipelineViewportStateCreateInfo::pViewports */
         bool viewport_state_viewports : 1;
         /** VkPipelineViewportStateCreateInfo::pScissors */
         bool viewport_state_scissors : 1;

         /** VkPipelineMultisampleStateCreateInfo::pSampleMask */
         bool multisample_state_sample_mask : 1;
      };
   };
};

static_assert(sizeof(struct vn_graphics_pipeline_info_self) ==
                 sizeof(((struct vn_graphics_pipeline_info_self){}).mask),
              "vn_graphics_pipeline_create_info_self::mask is too small");

/**
 * Fields in the VkGraphicsPipelineCreateInfo pNext chain that we must track
 * to determine which fields are valid and which must be erased.
 */
struct vn_graphics_pipeline_info_pnext {
   union {
      /* Bitmask exists for testing if any field is set. */
      uint32_t mask;

      /* Group the fixes by Vulkan struct. Within each group, sort by struct
       * order.
       */
      struct {
         /** VkPipelineRenderingCreateInfo, all format fields */
         bool rendering_info_formats : 1;
      };
   };
};

static_assert(sizeof(struct vn_graphics_pipeline_info_pnext) ==
                 sizeof(((struct vn_graphics_pipeline_info_pnext){}).mask),
              "vn_graphics_pipeline_create_info_pnext::mask is too small");

/**
 * Description of fixes needed for a single VkGraphicsPipelineCreateInfo
 * pNext chain.
 */
struct vn_graphics_pipeline_fix_desc {
   struct vn_graphics_pipeline_info_self self;
   struct vn_graphics_pipeline_info_pnext pnext;
};

/**
 * Typesafe bitmask for VkGraphicsPipelineLibraryFlagsEXT. Named members
 * reduce long lines.
 *
 * From the Vulkan 1.3.215 spec:
 *
 *    The state required for a graphics pipeline is divided into vertex input
 *    state, pre-rasterization shader state, fragment shader state, and
 *    fragment output state.
 */
struct vn_graphics_pipeline_library_state {
   union {
      VkGraphicsPipelineLibraryFlagsEXT mask;

      struct {
         /** VK_GRAPHICS_PIPELINE_LIBRARY_VERTEX_INPUT_INTERFACE_BIT_EXT */
         bool vertex_input : 1;
         /** VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT */
         bool pre_raster_shaders : 1;
         /** VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT */
         bool fragment_shader : 1;
         /** VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT */
         bool fragment_output : 1;
      };
   };
};

/**
 * Compact bitmask for the subset of graphics VkDynamicState that
 * venus needs to track. Named members reduce long lines.
 *
 * We want a *compact* bitmask because enum VkDynamicState has large gaps due
 * to extensions.
 */
struct vn_graphics_dynamic_state {
   union {
      uint32_t mask;

      struct {
         /** VK_DYNAMIC_STATE_VERTEX_INPUT_EXT **/
         bool vertex_input : 1;
         /** VK_DYNAMIC_STATE_VIEWPORT */
         bool viewport : 1;
         /** VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT */
         bool viewport_with_count : 1;
         /** VK_DYNAMIC_STATE_SAMPLE_MASK_EXT */
         bool sample_mask : 1;
         /** VK_DYNAMIC_STATE_SCISSOR */
         bool scissor : 1;
         /** VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT */
         bool scissor_with_count : 1;
         /** VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE */
         bool rasterizer_discard_enable : 1;
      };
   };
};

/**
 * Graphics pipeline state that Venus tracks to determine which fixes are
 * required in the VkGraphicsPipelineCreateInfo pNext chain.
 *
 * This is the pipeline's fully linked state. That is, it includes the state
 * provided directly in VkGraphicsPipelineCreateInfo and the state provided
 * indirectly in VkPipelineLibraryCreateInfoKHR.
 */
struct vn_graphics_pipeline_state {
   /** The GPL state subsets that the pipeline provides. */
   struct vn_graphics_pipeline_library_state gpl;

   struct vn_graphics_dynamic_state dynamic;
   VkShaderStageFlags shader_stages;

   struct vn_render_pass_state {
      /**
       * The attachment aspects accessed by the pipeline.
       *
       * Valid if and only if VK_IMAGE_ASPECT_METADATA_BIT is unset.
       *
       * In a complete pipeline, this must be valid (and may be empty). In
       * a pipeline library, this may be invalid. We initialize this to be
       * invalid, and it remains invalid until we read the attachment info in
       * the VkGraphicsPipelineCreateInfo chain.
       *
       * The app provides the attachment info in
       * VkGraphicsPipelineCreateInfo::renderPass or
       * VkPipelineRenderingCreateInfo, but the validity of that info depends
       * on VkGraphicsPipelineLibraryFlagsEXT.
       */
      VkImageAspectFlags attachment_aspects;
   } render_pass;

   /** VkPipelineRasterizationStateCreateInfo::rasterizerDiscardEnable
    *
    * Valid if and only if gpl.pre_raster_shaders is set.
    */
   bool rasterizer_discard_enable;
};

struct vn_graphics_pipeline {
   struct vn_pipeline base;
   struct vn_graphics_pipeline_state state;
};

/**
 * Temporary storage for fixes in vkCreateGraphicsPipelines.
 *
 * Length of each array is vkCreateGraphicsPipelines::createInfoCount.
 */
struct vn_graphics_pipeline_fix_tmp {
   VkGraphicsPipelineCreateInfo *infos;
   VkPipelineMultisampleStateCreateInfo *multisample_state_infos;
   VkPipelineViewportStateCreateInfo *viewport_state_infos;

   /* Fixing the pNext chain
    *
    * TODO: extend when below or more extensions are supported:
    * - VK_KHR_maintenance5
    * - VK_KHR_fragment_shading_rate
    * - VK_EXT_pipeline_robustness
    */
   VkGraphicsPipelineLibraryCreateInfoEXT *gpl_infos;
   VkPipelineCreationFeedbackCreateInfo *feedback_infos;
   VkPipelineLibraryCreateInfoKHR *library_infos;
   VkPipelineRenderingCreateInfo *rendering_infos;
};

/* shader module commands */

VkResult
vn_CreateShaderModule(VkDevice device,
                      const VkShaderModuleCreateInfo *pCreateInfo,
                      const VkAllocationCallbacks *pAllocator,
                      VkShaderModule *pShaderModule)
{
   struct vn_device *dev = vn_device_from_handle(device);
   const VkAllocationCallbacks *alloc =
      pAllocator ? pAllocator : &dev->base.base.alloc;

   struct vn_shader_module *mod =
      vk_zalloc(alloc, sizeof(*mod), VN_DEFAULT_ALIGN,
                VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (!mod)
      return vn_error(dev->instance, VK_ERROR_OUT_OF_HOST_MEMORY);

   vn_object_base_init(&mod->base, VK_OBJECT_TYPE_SHADER_MODULE, &dev->base);

   VkShaderModule mod_handle = vn_shader_module_to_handle(mod);
   vn_async_vkCreateShaderModule(dev->primary_ring, device, pCreateInfo, NULL,
                                 &mod_handle);

   *pShaderModule = mod_handle;

   return VK_SUCCESS;
}

void
vn_DestroyShaderModule(VkDevice device,
                       VkShaderModule shaderModule,
                       const VkAllocationCallbacks *pAllocator)
{
   struct vn_device *dev = vn_device_from_handle(device);
   struct vn_shader_module *mod = vn_shader_module_from_handle(shaderModule);
   const VkAllocationCallbacks *alloc =
      pAllocator ? pAllocator : &dev->base.base.alloc;

   if (!mod)
      return;

   vn_async_vkDestroyShaderModule(dev->primary_ring, device, shaderModule,
                                  NULL);

   vn_object_base_fini(&mod->base);
   vk_free(alloc, mod);
}

/* pipeline layout commands */

static void
vn_pipeline_layout_destroy(struct vn_device *dev,
                           struct vn_pipeline_layout *pipeline_layout)
{
   const VkAllocationCallbacks *alloc = &dev->base.base.alloc;
   if (pipeline_layout->push_descriptor_set_layout) {
      vn_descriptor_set_layout_unref(
         dev, pipeline_layout->push_descriptor_set_layout);
   }
   vn_async_vkDestroyPipelineLayout(
      dev->primary_ring, vn_device_to_handle(dev),
      vn_pipeline_layout_to_handle(pipeline_layout), NULL);

   vn_object_base_fini(&pipeline_layout->base);
   vk_free(alloc, pipeline_layout);
}

static inline struct vn_pipeline_layout *
vn_pipeline_layout_ref(struct vn_device *dev,
                       struct vn_pipeline_layout *pipeline_layout)
{
   vn_refcount_inc(&pipeline_layout->refcount);
   return pipeline_layout;
}

static inline void
vn_pipeline_layout_unref(struct vn_device *dev,
                         struct vn_pipeline_layout *pipeline_layout)
{
   if (vn_refcount_dec(&pipeline_layout->refcount))
      vn_pipeline_layout_destroy(dev, pipeline_layout);
}

VkResult
vn_CreatePipelineLayout(VkDevice device,
                        const VkPipelineLayoutCreateInfo *pCreateInfo,
                        const VkAllocationCallbacks *pAllocator,
                        VkPipelineLayout *pPipelineLayout)
{
   struct vn_device *dev = vn_device_from_handle(device);
   /* ignore pAllocator as the pipeline layout is reference-counted */
   const VkAllocationCallbacks *alloc = &dev->base.base.alloc;

   struct vn_pipeline_layout *layout =
      vk_zalloc(alloc, sizeof(*layout), VN_DEFAULT_ALIGN,
                VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
   if (!layout)
      return vn_error(dev->instance, VK_ERROR_OUT_OF_HOST_MEMORY);

   vn_object_base_init(&layout->base, VK_OBJECT_TYPE_PIPELINE_LAYOUT,
                       &dev->base);
   layout->refcount = VN_REFCOUNT_INIT(1);

   for (uint32_t i = 0; i < pCreateInfo->setLayoutCount; i++) {
      struct vn_descriptor_set_layout *descriptor_set_layout =
         vn_descriptor_set_layout_from_handle(pCreateInfo->pSetLayouts[i]);

      /* Avoid null derefs. pSetLayouts may contain VK_NULL_HANDLE.
       *
       * From the Vulkan 1.3.254 spec:
       *    VUID-VkPipelineLayoutCreateInfo-pSetLayouts-parameter
       *
       *    If setLayoutCount is not 0, pSetLayouts must be a valid pointer to
       *    an array of setLayoutCount valid or VK_NULL_HANDLE
       *    VkDescriptorSetLayout handles
       */
      if (descriptor_set_layout &&
          descriptor_set_layout->is_push_descriptor) {
         layout->push_descriptor_set_layout =
            vn_descriptor_set_layout_ref(dev, descriptor_set_layout);
         break;
      }
   }

   layout->has_push_constant_ranges = pCreateInfo->pushConstantRangeCount > 0;

   VkPipelineLayout layout_handle = vn_pipeline_layout_to_handle(layout);
   vn_async_vkCreatePipelineLayout(dev->primary_ring, device, pCreateInfo,
                                   NULL, &layout_handle);

   *pPipelineLayout = layout_handle;

   return VK_SUCCESS;
}

void
vn_DestroyPipelineLayout(VkDevice device,
                         VkPipelineLayout pipelineLayout,
                         const VkAllocationCallbacks *pAllocator)
{
   struct vn_device *dev = vn_device_from_handle(device);
   struct vn_pipeline_layout *layout =
      vn_pipeline_layout_from_handle(pipelineLayout);

   if (!layout)
      return;

   vn_pipeline_layout_unref(dev, layout);
}

/* pipeline cache commands */

VkResult
vn_CreatePipelineCache(VkDevice device,
                       const VkPipelineCacheCreateInfo *pCreateInfo,
                       const VkAllocationCallbacks *pAllocator,
                       VkPipelineCache *pPipelineCache)
{
   VN_TRACE_FUNC();
   struct vn_device *dev = vn_device_from_handle(device);
   const VkAllocationCallbacks *alloc =
      pAllocator ? pAllocator : &dev->base.base.alloc;

   struct vn_pipeline_cache *cache =
      vk_zalloc(alloc, sizeof(*cache), VN_DEFAULT_ALIGN,
                VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (!cache)
      return vn_error(dev->instance, VK_ERROR_OUT_OF_HOST_MEMORY);

   vn_object_base_init(&cache->base, VK_OBJECT_TYPE_PIPELINE_CACHE,
                       &dev->base);

   VkPipelineCacheCreateInfo local_create_info;
   if (pCreateInfo->initialDataSize) {
      const struct vk_pipeline_cache_header *header =
         pCreateInfo->pInitialData;

      local_create_info = *pCreateInfo;
      local_create_info.initialDataSize -= header->header_size;
      local_create_info.pInitialData += header->header_size;
      pCreateInfo = &local_create_info;
   }

   VkPipelineCache cache_handle = vn_pipeline_cache_to_handle(cache);
   vn_async_vkCreatePipelineCache(dev->primary_ring, device, pCreateInfo,
                                  NULL, &cache_handle);

   *pPipelineCache = cache_handle;

   return VK_SUCCESS;
}

void
vn_DestroyPipelineCache(VkDevice device,
                        VkPipelineCache pipelineCache,
                        const VkAllocationCallbacks *pAllocator)
{
   VN_TRACE_FUNC();
   struct vn_device *dev = vn_device_from_handle(device);
   struct vn_pipeline_cache *cache =
      vn_pipeline_cache_from_handle(pipelineCache);
   const VkAllocationCallbacks *alloc =
      pAllocator ? pAllocator : &dev->base.base.alloc;

   if (!cache)
      return;

   vn_async_vkDestroyPipelineCache(dev->primary_ring, device, pipelineCache,
                                   NULL);

   vn_object_base_fini(&cache->base);
   vk_free(alloc, cache);
}

static struct vn_ring *
vn_get_target_ring(struct vn_device *dev)
{
   if (vn_tls_get_async_pipeline_create())
      return dev->primary_ring;

   struct vn_ring *ring = vn_tls_get_ring(dev->instance);
   if (!ring)
      return NULL;

   if (ring != dev->primary_ring) {
      /* Ensure pipeline create and pipeline cache retrieval dependencies are
       * ready on the renderer side.
       *
       * TODO:
       * - For pipeline objects, avoid object id re-use between async pipeline
       *   destroy on the primary ring and sync pipeline create on TLS ring.
       * - For pipeline create, track ring seqnos of layout and renderpass
       *   objects it depends on, and only wait for those seqnos once.
       * - For pipeline cache retrieval, track ring seqno of pipeline cache
       *   object it depends on. Treat different sync mode separately.
       */
      vn_ring_wait_all(dev->primary_ring);
   }
   return ring;
}

VkResult
vn_GetPipelineCacheData(VkDevice device,
                        VkPipelineCache pipelineCache,
                        size_t *pDataSize,
                        void *pData)
{
   VN_TRACE_FUNC();
   struct vn_device *dev = vn_device_from_handle(device);
   struct vn_physical_device *physical_dev = dev->physical_device;
   struct vn_ring *target_ring = vn_get_target_ring(dev);

   struct vk_pipeline_cache_header *header = pData;
   VkResult result;
   if (!pData) {
      result = vn_call_vkGetPipelineCacheData(target_ring, device,
                                              pipelineCache, pDataSize, NULL);
      if (result != VK_SUCCESS)
         return vn_error(dev->instance, result);

      *pDataSize += sizeof(*header);
      return VK_SUCCESS;
   }

   if (*pDataSize <= sizeof(*header)) {
      *pDataSize = 0;
      return VK_INCOMPLETE;
   }

   const VkPhysicalDeviceProperties *props =
      &physical_dev->properties.vulkan_1_0;
   header->header_size = sizeof(*header);
   header->header_version = VK_PIPELINE_CACHE_HEADER_VERSION_ONE;
   header->vendor_id = props->vendorID;
   header->device_id = props->deviceID;
   memcpy(header->uuid, props->pipelineCacheUUID, VK_UUID_SIZE);

   *pDataSize -= header->header_size;
   result =
      vn_call_vkGetPipelineCacheData(target_ring, device, pipelineCache,
                                     pDataSize, pData + header->header_size);
   if (result < VK_SUCCESS)
      return vn_error(dev->instance, result);

   *pDataSize += header->header_size;

   return result;
}

VkResult
vn_MergePipelineCaches(VkDevice device,
                       VkPipelineCache dstCache,
                       uint32_t srcCacheCount,
                       const VkPipelineCache *pSrcCaches)
{
   VN_TRACE_FUNC();
   struct vn_device *dev = vn_device_from_handle(device);

   vn_async_vkMergePipelineCaches(dev->primary_ring, device, dstCache,
                                  srcCacheCount, pSrcCaches);

   return VK_SUCCESS;
}

/* pipeline commands */

static struct vn_graphics_pipeline *
vn_graphics_pipeline_from_handle(VkPipeline pipeline_h)
{
   struct vn_pipeline *p = vn_pipeline_from_handle(pipeline_h);
   assert(p->type == VN_PIPELINE_TYPE_GRAPHICS);
   return (struct vn_graphics_pipeline *)p;
}

static bool
vn_create_pipeline_handles(struct vn_device *dev,
                           enum vn_pipeline_type type,
                           uint32_t pipeline_count,
                           VkPipeline *pipeline_handles,
                           const VkAllocationCallbacks *alloc)
{
   size_t pipeline_size;

   switch (type) {
   case VN_PIPELINE_TYPE_GRAPHICS:
      pipeline_size = sizeof(struct vn_graphics_pipeline);
      break;
   case VN_PIPELINE_TYPE_COMPUTE:
      pipeline_size = sizeof(struct vn_pipeline);
      break;
   }

   for (uint32_t i = 0; i < pipeline_count; i++) {
      struct vn_pipeline *pipeline =
         vk_zalloc(alloc, pipeline_size, VN_DEFAULT_ALIGN,
                   VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

      if (!pipeline) {
         for (uint32_t j = 0; j < i; j++) {
            pipeline = vn_pipeline_from_handle(pipeline_handles[j]);
            vn_object_base_fini(&pipeline->base);
            vk_free(alloc, pipeline);
         }

         memset(pipeline_handles, 0,
                pipeline_count * sizeof(pipeline_handles[0]));
         return false;
      }

      vn_object_base_init(&pipeline->base, VK_OBJECT_TYPE_PIPELINE,
                          &dev->base);
      pipeline->type = type;
      pipeline_handles[i] = vn_pipeline_to_handle(pipeline);
   }

   return true;
}

/** For vkCreate*Pipelines.  */
static void
vn_destroy_failed_pipelines(struct vn_device *dev,
                            uint32_t create_info_count,
                            VkPipeline *pipelines,
                            const VkAllocationCallbacks *alloc)
{
   for (uint32_t i = 0; i < create_info_count; i++) {
      struct vn_pipeline *pipeline = vn_pipeline_from_handle(pipelines[i]);

      if (pipeline->base.id == 0) {
         if (pipeline->layout) {
            vn_pipeline_layout_unref(dev, pipeline->layout);
         }
         vn_object_base_fini(&pipeline->base);
         vk_free(alloc, pipeline);
         pipelines[i] = VK_NULL_HANDLE;
      }
   }
}

#define VN_PIPELINE_CREATE_SYNC_MASK                                         \
   (VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT |               \
    VK_PIPELINE_CREATE_EARLY_RETURN_ON_FAILURE_BIT)

static struct vn_graphics_pipeline_fix_tmp *
vn_graphics_pipeline_fix_tmp_alloc(const VkAllocationCallbacks *alloc,
                                   uint32_t info_count,
                                   bool alloc_pnext)
{
   struct vn_graphics_pipeline_fix_tmp *tmp;
   VkGraphicsPipelineCreateInfo *infos;
   VkPipelineMultisampleStateCreateInfo *multisample_state_infos;
   VkPipelineViewportStateCreateInfo *viewport_state_infos;

   /* for pNext */
   VkGraphicsPipelineLibraryCreateInfoEXT *gpl_infos;
   VkPipelineCreationFeedbackCreateInfo *feedback_infos;
   VkPipelineLibraryCreateInfoKHR *library_infos;
   VkPipelineRenderingCreateInfo *rendering_infos;

   VK_MULTIALLOC(ma);
   vk_multialloc_add(&ma, &tmp, __typeof__(*tmp), 1);
   vk_multialloc_add(&ma, &infos, __typeof__(*infos), info_count);
   vk_multialloc_add(&ma, &multisample_state_infos,
                     __typeof__(*multisample_state_infos), info_count);
   vk_multialloc_add(&ma, &viewport_state_infos,
                     __typeof__(*viewport_state_infos), info_count);

   if (alloc_pnext) {
      vk_multialloc_add(&ma, &gpl_infos, __typeof__(*gpl_infos), info_count);
      vk_multialloc_add(&ma, &feedback_infos, __typeof__(*feedback_infos),
                        info_count);
      vk_multialloc_add(&ma, &library_infos, __typeof__(*library_infos),
                        info_count);
      vk_multialloc_add(&ma, &rendering_infos, __typeof__(*rendering_infos),
                        info_count);
   }

   if (!vk_multialloc_zalloc(&ma, alloc, VK_SYSTEM_ALLOCATION_SCOPE_COMMAND))
      return NULL;

   tmp->infos = infos;
   tmp->multisample_state_infos = multisample_state_infos;
   tmp->viewport_state_infos = viewport_state_infos;

   if (alloc_pnext) {
      tmp->gpl_infos = gpl_infos;
      tmp->feedback_infos = feedback_infos;
      tmp->library_infos = library_infos;
      tmp->rendering_infos = rendering_infos;
   }

   return tmp;
}

/**
 * Update \a gpl with the VkGraphicsPipelineLibraryFlagsEXT that the pipeline
 * provides directly (without linking). The spec says that the pipeline always
 * provides flags, but may do it implicitly.
 *
 * From the Vulkan 1.3.251 spec:
 *
 *    If this structure [VkGraphicsPipelineLibraryCreateInfoEXT] is
 *    omitted, and either VkGraphicsPipelineCreateInfo::flags includes
 *    VK_PIPELINE_CREATE_LIBRARY_BIT_KHR or the
 *    VkGraphicsPipelineCreateInfo::pNext chain includes
 *    a VkPipelineLibraryCreateInfoKHR structure with a libraryCount
 *    greater than 0, it is as if flags is 0. Otherwise if this
 *    structure is omitted, it is as if flags includes all possible subsets
 *    of the graphics pipeline (i.e. a complete graphics pipeline).
 */
static void
vn_graphics_pipeline_library_state_update(
   const VkGraphicsPipelineCreateInfo *info,
   struct vn_graphics_pipeline_library_state *restrict gpl)
{
   const VkGraphicsPipelineLibraryCreateInfoEXT *gpl_info =
      vk_find_struct_const(info->pNext,
                           GRAPHICS_PIPELINE_LIBRARY_CREATE_INFO_EXT);
   const VkPipelineLibraryCreateInfoKHR *lib_info =
      vk_find_struct_const(info->pNext, PIPELINE_LIBRARY_CREATE_INFO_KHR);
   const uint32_t lib_count = lib_info ? lib_info->libraryCount : 0;

   if (gpl_info) {
      gpl->mask |= gpl_info->flags;
   } else if ((info->flags & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR) ||
              lib_count > 0) {
      gpl->mask |= 0;
   } else {
      gpl->mask |=
         VK_GRAPHICS_PIPELINE_LIBRARY_VERTEX_INPUT_INTERFACE_BIT_EXT |
         VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT |
         VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT |
         VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT;
   }
}

/**
 * Update \a dynamic with the VkDynamicState that the pipeline provides
 * directly (without linking).
 *
 * \a direct_gpl The VkGraphicsPipelineLibraryFlagsEXT that the pipeline sets
 *    directly (without linking).
 */
static void
vn_graphics_dynamic_state_update(
   const VkGraphicsPipelineCreateInfo *info,
   struct vn_graphics_pipeline_library_state direct_gpl,
   struct vn_graphics_dynamic_state *restrict dynamic)
{
   const VkPipelineDynamicStateCreateInfo *dyn_info = info->pDynamicState;
   if (!dyn_info)
      return;

   struct vn_graphics_dynamic_state raw = { 0 };

   for (uint32_t i = 0; i < dyn_info->dynamicStateCount; i++) {
      switch (dyn_info->pDynamicStates[i]) {
      case VK_DYNAMIC_STATE_VERTEX_INPUT_EXT:
         raw.vertex_input = true;
         break;
      case VK_DYNAMIC_STATE_VIEWPORT:
         raw.viewport = true;
         break;
      case VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT:
         raw.viewport_with_count = true;
         break;
      case VK_DYNAMIC_STATE_SAMPLE_MASK_EXT:
         raw.sample_mask = true;
         break;
      case VK_DYNAMIC_STATE_SCISSOR:
         raw.scissor = true;
         break;
      case VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT:
         raw.scissor_with_count = true;
         break;
      case VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE:
         raw.rasterizer_discard_enable = true;
         break;
      default:
         break;
      }
   }

   /* We must ignore VkDynamicState unrelated to the
    * VkGraphicsPipelineLibraryFlagsEXT that the pipeline provides directly
    * (without linking).
    *
    *    [Vulkan 1.3.252]
    *    Dynamic state values set via pDynamicState must be ignored if the
    *    state they correspond to is not otherwise statically set by one of
    *    the state subsets used to create the pipeline.
    *
    * In general, we must update dynamic state bits with `|=` rather than `=`
    * because multiple GPL state subsets can enable the same dynamic state.
    *
    *    [Vulkan 1.3.252]
    *    Any linked library that has dynamic state enabled that same dynamic
    *    state must also be enabled in all the other linked libraries to which
    *    that dynamic state applies.
    */
   if (direct_gpl.vertex_input) {
      dynamic->vertex_input |= raw.vertex_input;
   }
   if (direct_gpl.pre_raster_shaders) {
      dynamic->viewport |= raw.viewport;
      dynamic->viewport_with_count |= raw.viewport_with_count;
      dynamic->scissor |= raw.scissor;
      dynamic->scissor_with_count |= raw.scissor_with_count;
      dynamic->rasterizer_discard_enable |= raw.rasterizer_discard_enable;
   }
   if (direct_gpl.fragment_shader) {
      dynamic->sample_mask |= raw.sample_mask;
   }
   if (direct_gpl.fragment_output) {
      dynamic->sample_mask |= raw.sample_mask;
   }
}

/**
 * Update \a shader_stages with the VkShaderStageFlags that the pipeline
 * provides directly (without linking).
 *
 * \a direct_gpl The VkGraphicsPipelineLibraryFlagsEXT that the pipeline sets
 *    directly (without linking).
 */
static void
vn_graphics_shader_stages_update(
   const VkGraphicsPipelineCreateInfo *info,
   struct vn_graphics_pipeline_library_state direct_gpl,
   struct vn_graphics_pipeline_fix_desc *restrict valid,
   VkShaderStageFlags *restrict shader_stages)
{
   /* From the Vulkan 1.3.251 spec:
    *
    *    VUID-VkGraphicsPipelineCreateInfo-flags-06640
    *
    *    If VkGraphicsPipelineLibraryCreateInfoEXT::flags includes
    *    VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT or
    *    VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT, pStages must be
    *    a valid pointer to an array of stageCount valid
    *    VkPipelineShaderStageCreateInfo structures
    */
   if (!direct_gpl.pre_raster_shaders && !direct_gpl.fragment_shader)
      return;

   valid->self.shader_stages = true;

   for (uint32_t i = 0; i < info->stageCount; i++) {
      /* We do not need to ignore the stages irrelevant to the GPL flags.
       * The following VUs require the app to provide only relevant stages.
       *
       * VUID-VkGraphicsPipelineCreateInfo-pStages-06894
       * VUID-VkGraphicsPipelineCreateInfo-pStages-06895
       * VUID-VkGraphicsPipelineCreateInfo-pStages-06896
       */
      *shader_stages |= info->pStages[i].stage;
   }
}

/**
 * Update the render pass state with the state that the pipeline provides
 * directly (without linking).
 *
 * \a direct_gpl The VkGraphicsPipelineLibraryFlagsEXT that the pipeline sets
 *    directly (without linking).
 */
static void
vn_render_pass_state_update(
   const VkGraphicsPipelineCreateInfo *info,
   struct vn_graphics_pipeline_library_state direct_gpl,
   struct vn_graphics_pipeline_fix_desc *restrict valid,
   struct vn_render_pass_state *restrict state)
{
   /* We must set validity before early returns, to ensure we don't erase
    * valid info during fixup.  We must not erase valid info because, even if
    * we don't read it, the host driver may read it.
    */

   /* XXX: Should this ignore the render pass for some state subsets when
    * rasterization is statically disabled? The spec suggests "yes" and "no".
    */

   /* VUID-VkGraphicsPipelineCreateInfo-flags-06643
    *
    * If VkGraphicsPipelineLibraryCreateInfoEXT::flags includes
    * VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT, or
    * VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT,
    * VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT, and
    * renderPass is not VK_NULL_HANDLE, renderPass must be a valid
    * VkRenderPass handle
    */
   valid->self.render_pass |= direct_gpl.pre_raster_shaders ||
                              direct_gpl.fragment_shader ||
                              direct_gpl.fragment_output;

   /* VUID-VkGraphicsPipelineCreateInfo-renderPass-06579
    *
    * If the pipeline requires fragment output interface state, and renderPass
    * is VK_NULL_HANDLE, and
    * VkPipelineRenderingCreateInfo::colorAttachmentCount is not 0,
    * VkPipelineRenderingCreateInfo::pColorAttachmentFormats must be a valid
    * pointer to an array of colorAttachmentCount valid VkFormat values
    *
    * VUID-VkGraphicsPipelineCreateInfo-renderPass-06580
    *
    * If the pipeline requires fragment output interface state, and renderPass
    * is VK_NULL_HANDLE, each element of
    * VkPipelineRenderingCreateInfo::pColorAttachmentFormats must be a valid
    * VkFormat value
    */
   valid->pnext.rendering_info_formats |=
      direct_gpl.fragment_output && !info->renderPass;

   if (state->attachment_aspects != VK_IMAGE_ASPECT_METADATA_BIT) {
      /* We have previously collected the pipeline's attachment aspects.  We
       * do not need to inspect the attachment info again because VUs ensure
       * that all valid render pass info used to create the pipeline and its
       * linked pipelines are compatible.  Ignored info is not required to be
       * compatible across linked pipeline libraries. An example of ignored
       * info is VkPipelineRenderingCreateInfo::pColorAttachmentFormats
       * without
       * VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT.
       *
       * VUID-VkGraphicsPipelineCreateInfo-renderpass-06625
       * VUID-VkGraphicsPipelineCreateInfo-pLibraries-06628
       */
      return;
   }

   if (valid->self.render_pass && info->renderPass) {
      struct vn_render_pass *pass =
         vn_render_pass_from_handle(info->renderPass);
      state->attachment_aspects =
         pass->subpasses[info->subpass].attachment_aspects;
      return;
   }

   if (valid->pnext.rendering_info_formats) {
      state->attachment_aspects = 0;

      /* From the Vulkan 1.3.255 spec:
       *
       *    When a pipeline is created without a VkRenderPass, if this
       *    structure [VkPipelineRenderingCreateInfo] is present in the pNext
       *    chain of VkGraphicsPipelineCreateInfo, it specifies the view mask
       *    and format of attachments used for rendering.  If this structure
       *    is not specified, and the pipeline does not include
       *    a VkRenderPass, viewMask and colorAttachmentCount are 0, and
       *    depthAttachmentFormat and stencilAttachmentFormat are
       *    VK_FORMAT_UNDEFINED. If a graphics pipeline is created with
       *    a valid VkRenderPass, parameters of this structure are ignored.
       *
       * However, other spec text clearly states that the format members of
       * VkPipelineRenderingCreateInfo are ignored unless the pipeline
       * provides fragment output interface state directly (without linking).
       */
      const VkPipelineRenderingCreateInfo *r_info =
         vk_find_struct_const(info->pNext, PIPELINE_RENDERING_CREATE_INFO);

      if (r_info) {
         for (uint32_t i = 0; i < r_info->colorAttachmentCount; i++) {
            if (r_info->pColorAttachmentFormats[i]) {
               state->attachment_aspects |= VK_IMAGE_ASPECT_COLOR_BIT;
               break;
            }
         }
         if (r_info->depthAttachmentFormat)
            state->attachment_aspects |= VK_IMAGE_ASPECT_DEPTH_BIT;
         if (r_info->stencilAttachmentFormat)
            state->attachment_aspects |= VK_IMAGE_ASPECT_STENCIL_BIT;
      }

      return;
   }

   /* Aspects remain invalid. */
   assert(state->attachment_aspects == VK_IMAGE_ASPECT_METADATA_BIT);
}

static void
vn_graphics_pipeline_state_merge(
   struct vn_graphics_pipeline_state *restrict dst,
   const struct vn_graphics_pipeline_state *restrict src)
{
   /* The Vulkan 1.3.251 spec says:
    *    VUID-VkGraphicsPipelineCreateInfo-pLibraries-06611
    *
    *    Any pipeline libraries included via
    *    VkPipelineLibraryCreateInfoKHR::pLibraries must not include any state
    *    subset already defined by this structure or defined by any other
    *    pipeline library in VkPipelineLibraryCreateInfoKHR::pLibraries
    */
   assert(!(dst->gpl.mask & src->gpl.mask));

   dst->gpl.mask |= src->gpl.mask;
   dst->dynamic.mask |= src->dynamic.mask;
   dst->shader_stages |= src->shader_stages;

   VkImageAspectFlags src_aspects = src->render_pass.attachment_aspects;
   VkImageAspectFlags *dst_aspects = &dst->render_pass.attachment_aspects;

   if (src_aspects != VK_IMAGE_ASPECT_METADATA_BIT) {
      if (*dst_aspects != VK_IMAGE_ASPECT_METADATA_BIT) {
         /* All linked pipelines must have compatible render pass info. */
         assert(*dst_aspects == src_aspects);
      } else {
         *dst_aspects = src_aspects;
      }
   }

   if (dst->gpl.pre_raster_shaders)
      dst->rasterizer_discard_enable = src->rasterizer_discard_enable;
}

/**
 * Fill \a state by reading the VkGraphicsPipelineCreateInfo pNext chain,
 * including any linked pipeline libraries. Return in \a out_fix_desc
 * a description of required fixes to the VkGraphicsPipelineCreateInfo chain.
 *
 * \pre state is zero-filled
 *
 * The logic for choosing which struct members to ignore, and which members
 * have valid values, is derived from the Vulkan spec sections for
 * VkGraphicsPipelineCreateInfo, VkGraphicsPipelineLibraryCreateInfoEXT, and
 * VkPipelineLibraryCreateInfoKHR. As of Vulkan 1.3.255, the spec text and VUs
 * still contain inconsistencies regarding the validity of struct members, so
 * read it carefully. Many of the VUs were written before
 * VK_EXT_graphics_pipeline_library and never updated. (Lina's advice: Focus
 * primarily on understanding the non-VU text, and use VUs to verify your
 * comprehension).
 */
static void
vn_graphics_pipeline_state_fill(
   const VkGraphicsPipelineCreateInfo *info,
   struct vn_graphics_pipeline_state *restrict state,
   struct vn_graphics_pipeline_fix_desc *out_fix_desc)
{
   /* Assume that state is already zero-filled.
    *
    * Invalidate attachment_aspects.
    */
   state->render_pass.attachment_aspects = VK_IMAGE_ASPECT_METADATA_BIT;

   const VkPipelineRenderingCreateInfo *rendering_info =
      vk_find_struct_const(info->pNext, PIPELINE_RENDERING_CREATE_INFO);
   const VkPipelineLibraryCreateInfoKHR *lib_info =
      vk_find_struct_const(info->pNext, PIPELINE_LIBRARY_CREATE_INFO_KHR);
   const uint32_t lib_count = lib_info ? lib_info->libraryCount : 0;

   /* This tracks which fields have valid values in the
    * VkGraphicsPipelineCreateInfo pNext chain.
    *
    * We initially assume that all fields are invalid. We flip fields from
    * invalid to valid as we dig through the pNext chain.
    *
    * A single field may be updated at multiple locations, therefore we update
    * with `|=` instead of `=`.
    *
    * If `valid.foo` is set, then foo has a valid value if foo exists in the
    * pNext chain. Even though NULL is not a valid pointer, NULL is considered
    * a valid *value* for a pointer-typed variable. Same for VK_NULL_HANDLE
    * and Vulkan handle-typed variables.
    *
    * Conversely, if `valid.foo` remains false at the end of this function,
    * then the Vulkan spec permits foo to have any value. If foo has a pointer
    * type, it may be an invalid pointer. If foo has a Vulkan handle type, it
    * may be an invalid handle.
    */
   struct vn_graphics_pipeline_fix_desc valid = { 0 };

   /* Merge the linked pipeline libraries. */
   for (uint32_t i = 0; i < lib_count; i++) {
      struct vn_graphics_pipeline *p =
         vn_graphics_pipeline_from_handle(lib_info->pLibraries[i]);
      vn_graphics_pipeline_state_merge(state, &p->state);
   }

   /* The VkGraphicsPipelineLibraryFlagsEXT that this pipeline provides
    * directly (without linking).
    */
   struct vn_graphics_pipeline_library_state direct_gpl = { 0 };
   vn_graphics_pipeline_library_state_update(info, &direct_gpl);

   /* From the Vulkan 1.3.251 spec:
    *    VUID-VkGraphicsPipelineCreateInfo-pLibraries-06611
    *
    *    Any pipeline libraries included via
    *    VkPipelineLibraryCreateInfoKHR::pLibraries must not include any state
    *    subset already defined by this structure or defined by any other
    *    pipeline library in VkPipelineLibraryCreateInfoKHR::pLibraries
    */
   assert(!(direct_gpl.mask & state->gpl.mask));

   /* Collect orthogonal state that is common to multiple GPL state subsets. */
   vn_graphics_dynamic_state_update(info, direct_gpl, &state->dynamic);
   vn_graphics_shader_stages_update(info, direct_gpl, &valid,
                                    &state->shader_stages);
   vn_render_pass_state_update(info, direct_gpl, &valid, &state->render_pass);

   /* Collect remaining pre-raster shaders state.
    *
    * Of the remaining state, we must first collect the pre-raster shaders
    * state because it influences how the other state is collected.
    */
   if (direct_gpl.pre_raster_shaders) {
      valid.self.tessellation_state |=
         (bool)(state->shader_stages &
                (VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT |
                 VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT));
      valid.self.rasterization_state = true;
      valid.self.pipeline_layout = true;

      if (info->pRasterizationState) {
         state->rasterizer_discard_enable =
            info->pRasterizationState->rasterizerDiscardEnable;
      }

      const bool is_raster_statically_disabled =
         !state->dynamic.rasterizer_discard_enable &&
         state->rasterizer_discard_enable;

      if (!is_raster_statically_disabled) {
         valid.self.viewport_state = true;

         valid.self.viewport_state_viewports =
            !state->dynamic.viewport && !state->dynamic.viewport_with_count;

         valid.self.viewport_state_scissors =
            !state->dynamic.scissor && !state->dynamic.scissor_with_count;
      }

      /* Defer setting the flag until all its state is filled. */
      state->gpl.pre_raster_shaders = true;
   }

   /* Collect remaining vertex input interface state.
    *
    * TODO(VK_EXT_mesh_shader): Update.
    */
   if (direct_gpl.vertex_input) {
      const bool may_have_vertex_shader =
         !state->gpl.pre_raster_shaders ||
         (state->shader_stages & VK_SHADER_STAGE_VERTEX_BIT);

      valid.self.vertex_input_state |=
         may_have_vertex_shader && !state->dynamic.vertex_input;

      valid.self.input_assembly_state |= may_have_vertex_shader;

      /* Defer setting the flag until all its state is filled. */
      state->gpl.vertex_input = true;
   }

   /* Does this pipeline have rasterization statically disabled? If disabled,
    * then this pipeline does not directly provide fragment shader state nor
    * fragment output state.
    *
    * About fragment shader state, the Vulkan 1.3.254 spec says:
    *
    *    If a pipeline specifies pre-rasterization state either directly or by
    *    including it as a pipeline library and rasterizerDiscardEnable is set
    *    to VK_FALSE or VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE is used,
    *    this state must be specified to create a complete graphics pipeline.
    *
    *    If a pipeline includes
    *    VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT in
    *    VkGraphicsPipelineLibraryCreateInfoEXT::flags either explicitly or as
    *    a default, and either the conditions requiring this state for
    *    a complete graphics pipeline are met or this pipeline does not
    *    specify pre-rasterization state in any way, that pipeline must
    *    specify this state directly.
    *
    * About fragment output state, the Vulkan 1.3.254 spec says the same, but
    * with VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT.
    */
   const bool is_raster_statically_disabled =
      state->gpl.pre_raster_shaders &&
      !state->dynamic.rasterizer_discard_enable &&
      state->rasterizer_discard_enable;

   /* Collect remaining fragment shader state. */
   if (direct_gpl.fragment_shader) {
      if (!is_raster_statically_disabled) {
         /* Validity of pMultisampleState is easy here.
          *
          *    VUID-VkGraphicsPipelineCreateInfo-pMultisampleState-06629
          *
          *    If the pipeline requires fragment shader state
          *    pMultisampleState must be NULL or a valid pointer to a valid
          *    VkPipelineMultisampleStateCreateInfo structure
          */
         valid.self.multisample_state = true;

         valid.self.multisample_state_sample_mask =
            !state->dynamic.sample_mask;

         if ((state->render_pass.attachment_aspects &
              (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT))) {
            valid.self.depth_stencil_state = true;
         } else if (state->render_pass.attachment_aspects ==
                       VK_IMAGE_ASPECT_METADATA_BIT &&
                    (info->flags & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR)) {
            /* The app has not yet provided render pass info, neither directly
             * in this VkGraphicsPipelineCreateInfo nor in any linked pipeline
             * libraries. Therefore we do not know if the final complete
             * pipeline will have any depth or stencil attachments. If the
             * final complete pipeline does have depth or stencil attachments,
             * then the pipeline will use
             * VkPipelineDepthStencilStateCreateInfo. Therefore, we must not
             * ignore it.
             */
            valid.self.depth_stencil_state = true;
         }

         valid.self.pipeline_layout = true;
      }

      /* Defer setting the flag until all its state is filled. */
      state->gpl.fragment_shader = true;
   }

   /* Collect remaining fragment output interface state. */
   if (direct_gpl.fragment_output) {
      if (!is_raster_statically_disabled) {
         /* Validity of pMultisampleState is easy here.
          *
          *    VUID-VkGraphicsPipelineCreateInfo-rasterizerDiscardEnable-00751
          *
          *    If the pipeline requires fragment output interface state,
          *    pMultisampleState must be a valid pointer to a valid
          *    VkPipelineMultisampleStateCreateInfo structure
          */
         valid.self.multisample_state = true;

         valid.self.multisample_state_sample_mask =
            !state->dynamic.sample_mask;

         valid.self.color_blend_state |=
            (bool)(state->render_pass.attachment_aspects &
                   VK_IMAGE_ASPECT_COLOR_BIT);
         valid.self.depth_stencil_state |=
            (bool)(state->render_pass.attachment_aspects &
                   (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT));
      }

      /* Defer setting the flag until all its state is filled. */
      state->gpl.fragment_output = true;
   }

   /* After direct_gpl states collection, check the final state to validate
    * VkPipelineLayout in case of being the final layout in linked pipeline.
    *
    * From the Vulkan 1.3.275 spec:
    *    VUID-VkGraphicsPipelineCreateInfo-layout-06602
    *
    *    If the pipeline requires fragment shader state or pre-rasterization
    *    shader state, layout must be a valid VkPipelineLayout handle
    */
   if ((state->gpl.fragment_shader && !is_raster_statically_disabled) ||
       state->gpl.pre_raster_shaders)
      valid.self.pipeline_layout = true;

   /* Pipeline Derivatives
    *
    *    VUID-VkGraphicsPipelineCreateInfo-flags-07984
    *
    *    If flags contains the VK_PIPELINE_CREATE_DERIVATIVE_BIT flag, and
    *    basePipelineIndex is -1, basePipelineHandle must be a valid graphics
    *    VkPipeline handle
    */
   if ((info->flags & VK_PIPELINE_CREATE_DERIVATIVE_BIT) &&
       info->basePipelineIndex == -1)
      valid.self.base_pipeline_handle = true;

   *out_fix_desc = (struct vn_graphics_pipeline_fix_desc) {
      .self = {
         /* clang-format off */
         .shader_stages =
            !valid.self.shader_stages &&
            info->pStages,
         .vertex_input_state =
            !valid.self.vertex_input_state &&
            info->pVertexInputState,
         .input_assembly_state =
            !valid.self.input_assembly_state &&
            info->pInputAssemblyState,
         .tessellation_state =
            !valid.self.tessellation_state &&
            info->pTessellationState,
         .viewport_state =
            !valid.self.viewport_state &&
            info->pViewportState,
         .viewport_state_viewports =
            !valid.self.viewport_state_viewports &&
            valid.self.viewport_state &&
            info->pViewportState &&
            info->pViewportState->pViewports &&
            info->pViewportState->viewportCount,
         .viewport_state_scissors =
            !valid.self.viewport_state_scissors &&
            valid.self.viewport_state &&
            info->pViewportState &&
            info->pViewportState->pScissors &&
            info->pViewportState->scissorCount,
         .rasterization_state =
            !valid.self.rasterization_state &&
            info->pRasterizationState,
         .multisample_state =
            !valid.self.multisample_state &&
            info->pMultisampleState,
         .multisample_state_sample_mask =
            !valid.self.multisample_state_sample_mask &&
            valid.self.multisample_state &&
            info->pMultisampleState &&
            info->pMultisampleState->pSampleMask,
         .depth_stencil_state =
            !valid.self.depth_stencil_state &&
            info->pDepthStencilState,
         .color_blend_state =
            !valid.self.color_blend_state &&
            info->pColorBlendState,
         .pipeline_layout =
            !valid.self.pipeline_layout &&
            info->layout,
         .render_pass =
            !valid.self.render_pass &&
            info->renderPass,
         .base_pipeline_handle =
            !valid.self.base_pipeline_handle &&
            info->basePipelineHandle,
         /* clang-format on */
      },
      .pnext = {
         /* clang-format off */
         .rendering_info_formats =
            !valid.pnext.rendering_info_formats &&
            rendering_info &&
            rendering_info->pColorAttachmentFormats &&
            rendering_info->colorAttachmentCount,
         /* clang-format on */
      },
   };
}

static void
vn_fix_graphics_pipeline_create_info_self(
   const struct vn_graphics_pipeline_info_self *ignore,
   const VkGraphicsPipelineCreateInfo *info,
   struct vn_graphics_pipeline_fix_tmp *fix_tmp,
   uint32_t index)
{
   /* VkGraphicsPipelineCreateInfo */
   if (ignore->shader_stages) {
      fix_tmp->infos[index].stageCount = 0;
      fix_tmp->infos[index].pStages = NULL;
   }
   if (ignore->vertex_input_state)
      fix_tmp->infos[index].pVertexInputState = NULL;
   if (ignore->input_assembly_state)
      fix_tmp->infos[index].pInputAssemblyState = NULL;
   if (ignore->tessellation_state)
      fix_tmp->infos[index].pTessellationState = NULL;
   if (ignore->viewport_state)
      fix_tmp->infos[index].pViewportState = NULL;
   if (ignore->rasterization_state)
      fix_tmp->infos[index].pRasterizationState = NULL;
   if (ignore->multisample_state)
      fix_tmp->infos[index].pMultisampleState = NULL;
   if (ignore->depth_stencil_state)
      fix_tmp->infos[index].pDepthStencilState = NULL;
   if (ignore->color_blend_state)
      fix_tmp->infos[index].pColorBlendState = NULL;
   if (ignore->pipeline_layout)
      fix_tmp->infos[index].layout = VK_NULL_HANDLE;
   if (ignore->base_pipeline_handle)
      fix_tmp->infos[index].basePipelineHandle = VK_NULL_HANDLE;

   /* VkPipelineMultisampleStateCreateInfo */
   if (ignore->multisample_state_sample_mask) {
      /* Swap original pMultisampleState with temporary state. */
      fix_tmp->multisample_state_infos[index] = *info->pMultisampleState;
      fix_tmp->infos[index].pMultisampleState =
         &fix_tmp->multisample_state_infos[index];

      fix_tmp->multisample_state_infos[index].pSampleMask = NULL;
   }

   /* VkPipelineViewportStateCreateInfo */
   if (ignore->viewport_state_viewports || ignore->viewport_state_scissors) {
      /* Swap original pViewportState with temporary state. */
      fix_tmp->viewport_state_infos[index] = *info->pViewportState;
      fix_tmp->infos[index].pViewportState =
         &fix_tmp->viewport_state_infos[index];

      if (ignore->viewport_state_viewports)
         fix_tmp->viewport_state_infos[index].pViewports = NULL;
      if (ignore->viewport_state_scissors)
         fix_tmp->viewport_state_infos[index].pScissors = NULL;
   }
}

static void
vn_graphics_pipeline_create_info_pnext_init(
   const VkGraphicsPipelineCreateInfo *info,
   struct vn_graphics_pipeline_fix_tmp *fix_tmp,
   uint32_t index)
{
   VkGraphicsPipelineLibraryCreateInfoEXT *gpl = &fix_tmp->gpl_infos[index];
   VkPipelineCreationFeedbackCreateInfo *feedback =
      &fix_tmp->feedback_infos[index];
   VkPipelineLibraryCreateInfoKHR *library = &fix_tmp->library_infos[index];
   VkPipelineRenderingCreateInfo *rendering =
      &fix_tmp->rendering_infos[index];

   VkBaseOutStructure *cur = (void *)&fix_tmp->infos[index];

   vk_foreach_struct_const(src, info->pNext) {
      void *next = NULL;
      switch (src->sType) {
      case VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_LIBRARY_CREATE_INFO_EXT:
         memcpy(gpl, src, sizeof(*gpl));
         next = gpl;
         break;
      case VK_STRUCTURE_TYPE_PIPELINE_CREATION_FEEDBACK_CREATE_INFO:
         memcpy(feedback, src, sizeof(*feedback));
         next = feedback;
         break;
      case VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR:
         memcpy(library, src, sizeof(*library));
         next = library;
         break;
      case VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO:
         memcpy(rendering, src, sizeof(*rendering));
         next = rendering;
         break;
      default:
         break;
      }

      if (next) {
         cur->pNext = next;
         cur = next;
      }
   }

   cur->pNext = NULL;
}

static void
vn_fix_graphics_pipeline_create_info_pnext(
   const struct vn_graphics_pipeline_info_pnext *ignore,
   const VkGraphicsPipelineCreateInfo *info,
   struct vn_graphics_pipeline_fix_tmp *fix_tmp,
   uint32_t index)
{
   /* initialize pNext chain with allocated tmp storage */
   vn_graphics_pipeline_create_info_pnext_init(info, fix_tmp, index);

   /* VkPipelineRenderingCreateInfo */
   if (ignore->rendering_info_formats) {
      fix_tmp->rendering_infos[index].colorAttachmentCount = 0;
      fix_tmp->rendering_infos[index].pColorAttachmentFormats = NULL;
   }
}

static const VkGraphicsPipelineCreateInfo *
vn_fix_graphics_pipeline_create_infos(
   struct vn_device *dev,
   uint32_t info_count,
   const VkGraphicsPipelineCreateInfo *infos,
   const struct vn_graphics_pipeline_fix_desc fix_descs[info_count],
   struct vn_graphics_pipeline_fix_tmp **out_fix_tmp,
   const VkAllocationCallbacks *alloc)
{
   uint32_t self_mask = 0;
   uint32_t pnext_mask = 0;
   for (uint32_t i = 0; i < info_count; i++) {
      self_mask |= fix_descs[i].self.mask;
      pnext_mask |= fix_descs[i].pnext.mask;
   }

   if (!self_mask && !pnext_mask) {
      /* No fix is needed. */
      *out_fix_tmp = NULL;
      return infos;
   }

   /* tell whether fixes are applied in tracing */
   VN_TRACE_SCOPE("apply_fixes");

   struct vn_graphics_pipeline_fix_tmp *fix_tmp =
      vn_graphics_pipeline_fix_tmp_alloc(alloc, info_count, pnext_mask);
   if (!fix_tmp)
      return NULL;

   memcpy(fix_tmp->infos, infos, info_count * sizeof(infos[0]));

   for (uint32_t i = 0; i < info_count; i++) {
      if (fix_descs[i].self.mask) {
         vn_fix_graphics_pipeline_create_info_self(&fix_descs[i].self,
                                                   &infos[i], fix_tmp, i);
      }
      if (fix_descs[i].pnext.mask) {
         vn_fix_graphics_pipeline_create_info_pnext(&fix_descs[i].pnext,
                                                    &infos[i], fix_tmp, i);
      }
   }

   *out_fix_tmp = fix_tmp;
   return fix_tmp->infos;
}

/**
 * We invalidate each VkPipelineCreationFeedback. This is a legal but useless
 * implementation.
 *
 * We invalidate because the venus protocol (as of 2022-08-25) does not know
 * that the VkPipelineCreationFeedback structs in the
 * VkGraphicsPipelineCreateInfo pNext are output parameters. Before
 * VK_EXT_pipeline_creation_feedback, the pNext chain was input-only.
 */
static void
vn_invalidate_pipeline_creation_feedback(const VkBaseInStructure *chain)
{
   const VkPipelineCreationFeedbackCreateInfo *feedback_info =
      vk_find_struct_const(chain, PIPELINE_CREATION_FEEDBACK_CREATE_INFO);

   if (!feedback_info)
      return;

   feedback_info->pPipelineCreationFeedback->flags = 0;

   for (uint32_t i = 0; i < feedback_info->pipelineStageCreationFeedbackCount;
        i++)
      feedback_info->pPipelineStageCreationFeedbacks[i].flags = 0;
}

VkResult
vn_CreateGraphicsPipelines(VkDevice device,
                           VkPipelineCache pipelineCache,
                           uint32_t createInfoCount,
                           const VkGraphicsPipelineCreateInfo *pCreateInfos,
                           const VkAllocationCallbacks *pAllocator,
                           VkPipeline *pPipelines)
{
   VN_TRACE_FUNC();
   struct vn_device *dev = vn_device_from_handle(device);
   const VkAllocationCallbacks *alloc =
      pAllocator ? pAllocator : &dev->base.base.alloc;
   bool want_sync = false;
   VkResult result;

   memset(pPipelines, 0, sizeof(*pPipelines) * createInfoCount);

   if (!vn_create_pipeline_handles(dev, VN_PIPELINE_TYPE_GRAPHICS,
                                   createInfoCount, pPipelines, alloc)) {
      return vn_error(dev->instance, VK_ERROR_OUT_OF_HOST_MEMORY);
   }

   STACK_ARRAY(struct vn_graphics_pipeline_fix_desc, fix_descs,
               createInfoCount);
   for (uint32_t i = 0; i < createInfoCount; i++) {
      struct vn_graphics_pipeline *pipeline =
         vn_graphics_pipeline_from_handle(pPipelines[i]);
      vn_graphics_pipeline_state_fill(&pCreateInfos[i], &pipeline->state,
                                      &fix_descs[i]);
   }

   struct vn_graphics_pipeline_fix_tmp *fix_tmp = NULL;
   pCreateInfos = vn_fix_graphics_pipeline_create_infos(
      dev, createInfoCount, pCreateInfos, fix_descs, &fix_tmp, alloc);
   if (!pCreateInfos) {
      vn_destroy_failed_pipelines(dev, createInfoCount, pPipelines, alloc);
      STACK_ARRAY_FINISH(fix_descs);
      return vn_error(dev->instance, VK_ERROR_OUT_OF_HOST_MEMORY);
   }

   for (uint32_t i = 0; i < createInfoCount; i++) {
      struct vn_pipeline *pipeline = vn_pipeline_from_handle(pPipelines[i]);

      /* Grab a refcount on the pipeline layout when needed. Take care; the
       * pipeline layout may be omitted or ignored in incomplete pipelines.
       */
      struct vn_pipeline_layout *layout =
         vn_pipeline_layout_from_handle(pCreateInfos[i].layout);
      if (layout && (layout->push_descriptor_set_layout ||
                     layout->has_push_constant_ranges)) {
         pipeline->layout = vn_pipeline_layout_ref(dev, layout);
      }

      if ((pCreateInfos[i].flags & VN_PIPELINE_CREATE_SYNC_MASK))
         want_sync = true;

      vn_invalidate_pipeline_creation_feedback(
         (const VkBaseInStructure *)pCreateInfos[i].pNext);
   }

   struct vn_ring *target_ring = vn_get_target_ring(dev);
   if (!target_ring) {
      vk_free(alloc, fix_tmp);
      vn_destroy_failed_pipelines(dev, createInfoCount, pPipelines, alloc);
      STACK_ARRAY_FINISH(fix_descs);
      return vn_error(dev->instance, VK_ERROR_OUT_OF_HOST_MEMORY);
   }

   if (want_sync || target_ring != dev->primary_ring) {
      result = vn_call_vkCreateGraphicsPipelines(
         target_ring, device, pipelineCache, createInfoCount, pCreateInfos,
         NULL, pPipelines);
      if (result != VK_SUCCESS)
         vn_destroy_failed_pipelines(dev, createInfoCount, pPipelines, alloc);
   } else {
      vn_async_vkCreateGraphicsPipelines(target_ring, device, pipelineCache,
                                         createInfoCount, pCreateInfos, NULL,
                                         pPipelines);
      result = VK_SUCCESS;
   }

   vk_free(alloc, fix_tmp);
   STACK_ARRAY_FINISH(fix_descs);
   return vn_result(dev->instance, result);
}

VkResult
vn_CreateComputePipelines(VkDevice device,
                          VkPipelineCache pipelineCache,
                          uint32_t createInfoCount,
                          const VkComputePipelineCreateInfo *pCreateInfos,
                          const VkAllocationCallbacks *pAllocator,
                          VkPipeline *pPipelines)
{
   VN_TRACE_FUNC();
   struct vn_device *dev = vn_device_from_handle(device);
   const VkAllocationCallbacks *alloc =
      pAllocator ? pAllocator : &dev->base.base.alloc;
   bool want_sync = false;
   VkResult result;

   memset(pPipelines, 0, sizeof(*pPipelines) * createInfoCount);

   if (!vn_create_pipeline_handles(dev, VN_PIPELINE_TYPE_COMPUTE,
                                   createInfoCount, pPipelines, alloc))
      return vn_error(dev->instance, VK_ERROR_OUT_OF_HOST_MEMORY);

   for (uint32_t i = 0; i < createInfoCount; i++) {
      struct vn_pipeline *pipeline = vn_pipeline_from_handle(pPipelines[i]);
      struct vn_pipeline_layout *layout =
         vn_pipeline_layout_from_handle(pCreateInfos[i].layout);
      if (layout->push_descriptor_set_layout ||
          layout->has_push_constant_ranges) {
         pipeline->layout = vn_pipeline_layout_ref(dev, layout);
      }
      if ((pCreateInfos[i].flags & VN_PIPELINE_CREATE_SYNC_MASK))
         want_sync = true;

      vn_invalidate_pipeline_creation_feedback(
         (const VkBaseInStructure *)pCreateInfos[i].pNext);
   }

   struct vn_ring *target_ring = vn_get_target_ring(dev);
   if (!target_ring) {
      vn_destroy_failed_pipelines(dev, createInfoCount, pPipelines, alloc);
      return vn_error(dev->instance, VK_ERROR_OUT_OF_HOST_MEMORY);
   }

   if (want_sync || target_ring != dev->primary_ring) {
      result = vn_call_vkCreateComputePipelines(
         target_ring, device, pipelineCache, createInfoCount, pCreateInfos,
         NULL, pPipelines);
      if (result != VK_SUCCESS)
         vn_destroy_failed_pipelines(dev, createInfoCount, pPipelines, alloc);
   } else {
      vn_async_vkCreateComputePipelines(target_ring, device, pipelineCache,
                                        createInfoCount, pCreateInfos, NULL,
                                        pPipelines);
      result = VK_SUCCESS;
   }

   return vn_result(dev->instance, result);
}

void
vn_DestroyPipeline(VkDevice device,
                   VkPipeline _pipeline,
                   const VkAllocationCallbacks *pAllocator)
{
   VN_TRACE_FUNC();
   struct vn_device *dev = vn_device_from_handle(device);
   struct vn_pipeline *pipeline = vn_pipeline_from_handle(_pipeline);
   const VkAllocationCallbacks *alloc =
      pAllocator ? pAllocator : &dev->base.base.alloc;

   if (!pipeline)
      return;

   if (pipeline->layout) {
      vn_pipeline_layout_unref(dev, pipeline->layout);
   }

   vn_async_vkDestroyPipeline(dev->primary_ring, device, _pipeline, NULL);

   vn_object_base_fini(&pipeline->base);
   vk_free(alloc, pipeline);
}
