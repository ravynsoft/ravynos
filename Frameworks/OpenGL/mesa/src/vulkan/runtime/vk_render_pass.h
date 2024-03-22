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
#ifndef VK_RENDER_PASS_H
#define VK_RENDER_PASS_H

#include "vk_object.h"

#ifdef __cplusplus
extern "C" {
#endif

struct vk_command_buffer;
struct vk_image;

/**
 * Pseudo-extension struct that may be chained into VkRenderingAttachmentInfo
 * to indicate an initial layout for the attachment.  This is only allowed if
 * all of the following conditions are met:
 *
 *    1. VkRenderingAttachmentInfo::loadOp == LOAD_OP_CLEAR
 *
 *    2. VkRenderingInfo::renderArea is tne entire image view LOD
 *
 *    3. For 3D image attachments, VkRenderingInfo::viewMask == 0 AND
 *       VkRenderingInfo::layerCount references the entire bound image view
 *       OR VkRenderingInfo::viewMask is dense (no holes) and references the
 *       entire bound image view.  (2D and 2D array images have no such
 *       requirement.)
 *
 * If this struct is included in the pNext chain of a
 * VkRenderingAttachmentInfo, the driver is responsible for transitioning the
 * bound region of the image from
 * VkRenderingAttachmentInitialLayoutInfoMESA::initialLayout to
 * VkRenderingAttachmentInfo::imageLayout prior to rendering.
 */
typedef struct VkRenderingAttachmentInitialLayoutInfoMESA {
    VkStructureType    sType;
#define VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INITIAL_LAYOUT_INFO_MESA (VkStructureType)1000044901
#define VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INITIAL_LAYOUT_INFO_MESA_cast VkRenderingAttachmentInitialLayoutInfoMESA
    const void*        pNext;

    /** Initial layout of the attachment */
    VkImageLayout      initialLayout;
} VkRenderingAttachmentInitialLayoutInfoMESA;

/***/
struct vk_subpass_attachment {
   /** VkAttachmentReference2::attachment */
   uint32_t attachment;

   /** Aspects referenced by this attachment
    *
    * For an input attachment, this is VkAttachmentReference2::aspectMask.
    * For all others, it's equal to the vk_render_pass_attachment::aspects.
    */
   VkImageAspectFlags aspects;

   /** Usage for this attachment
    *
    * This is a single VK_IMAGE_USAGE_* describing the usage of this subpass
    * attachment.  Resolve attachments are VK_IMAGE_USAGE_TRANSFER_DST_BIT.
    */
   VkImageUsageFlagBits usage;

   /** VkAttachmentReference2::layout */
   VkImageLayout layout;

   /** VkAttachmentReferenceStencilLayout::stencilLayout
    *
    * If VK_KHR_separate_depth_stencil_layouts is not used, this will be
    * layout if the attachment contains stencil and VK_IMAGE_LAYOUT_UNDEFINED
    * otherwise.
    */
   VkImageLayout stencil_layout;

   /** A per-view mask for if this is the last use of this attachment
    *
    * If the same render pass attachment is used multiple ways within a
    * subpass, corresponding last_subpass bits will be set in all of them.
    * For the non-multiview case, only the first bit is used.
    */
   uint32_t last_subpass;

   /** Resolve attachment, if any */
   struct vk_subpass_attachment *resolve;
};

/***/
struct vk_subpass {
   /** Count of all attachments referenced by this subpass */
   uint32_t attachment_count;

   /** Array of all attachments referenced by this subpass */
   struct vk_subpass_attachment *attachments;

   /** VkSubpassDescription2::inputAttachmentCount */
   uint32_t input_count;

   /** VkSubpassDescription2::pInputAttachments */
   struct vk_subpass_attachment *input_attachments;

   /** VkSubpassDescription2::colorAttachmentCount */
   uint32_t color_count;

   /** VkSubpassDescription2::pColorAttachments */
   struct vk_subpass_attachment *color_attachments;

   /** VkSubpassDescription2::colorAttachmentCount or zero */
   uint32_t color_resolve_count;

   /** VkSubpassDescription2::pResolveAttachments */
   struct vk_subpass_attachment *color_resolve_attachments;

   /** VkSubpassDescription2::pDepthStencilAttachment */
   struct vk_subpass_attachment *depth_stencil_attachment;

   /** VkSubpassDescriptionDepthStencilResolve::pDepthStencilResolveAttachment */
   struct vk_subpass_attachment *depth_stencil_resolve_attachment;

   /** VkFragmentShadingRateAttachmentInfoKHR::pFragmentShadingRateAttachment */
   struct vk_subpass_attachment *fragment_shading_rate_attachment;

   /** VkSubpassDescription2::viewMask or 1 for non-multiview
    *
    * For all view masks in the vk_render_pass data structure, we use a mask
    * of 1 for non-multiview instead of a mask of 0.  To determine if the
    * render pass is multiview or not, see vk_render_pass::is_multiview.
    */
   uint32_t view_mask;

   /** VkSubpassDescriptionDepthStencilResolve::depthResolveMode */
   VkResolveModeFlagBits depth_resolve_mode;

   /** VkSubpassDescriptionDepthStencilResolve::stencilResolveMode */
   VkResolveModeFlagBits stencil_resolve_mode;

   /** VkFragmentShadingRateAttachmentInfoKHR::shadingRateAttachmentTexelSize */
   VkExtent2D fragment_shading_rate_attachment_texel_size;

   /** Extra VkPipelineCreateFlags for this subpass */
   VkPipelineCreateFlagBits2KHR pipeline_flags;

   /** VkAttachmentSampleCountInfoAMD for this subpass
    *
    * This is in the pNext chain of pipeline_info and inheritance_info.
    */
   VkAttachmentSampleCountInfoAMD sample_count_info_amd;

   /** VkPipelineRenderingCreateInfo for this subpass
    *
    * Returned by vk_get_pipeline_rendering_create_info() if
    * VkGraphicsPipelineCreateInfo::renderPass != VK_NULL_HANDLE.
    */
   VkPipelineRenderingCreateInfo pipeline_info;

   /** VkCommandBufferInheritanceRenderingInfo for this subpass
    *
    * Returned by vk_get_command_buffer_inheritance_rendering_info() if
    * VkCommandBufferInheritanceInfo::renderPass != VK_NULL_HANDLE.
    */
   VkCommandBufferInheritanceRenderingInfo inheritance_info;

   /** VkMultisampledRenderToSingleSampledInfoEXT for this subpass */
   VkMultisampledRenderToSingleSampledInfoEXT mrtss;
};

/***/
struct vk_render_pass_attachment {
   /** VkAttachmentDescription2::format */
   VkFormat format;

   /** Aspects contained in format */
   VkImageAspectFlags aspects;

   /** VkAttachmentDescription2::samples */
   uint32_t samples;

   /** Views in which this attachment is used, 0 for unused
    *
    * For non-multiview, this will be 1 if the attachment is used.
    */
   uint32_t view_mask;

   /** VkAttachmentDescription2::loadOp */
   VkAttachmentLoadOp load_op;

   /** VkAttachmentDescription2::storeOp */
   VkAttachmentStoreOp store_op;

   /** VkAttachmentDescription2::stencilLoadOp */
   VkAttachmentLoadOp stencil_load_op;

   /** VkAttachmentDescription2::stencilStoreOp */
   VkAttachmentStoreOp stencil_store_op;

   /** VkAttachmentDescription2::initialLayout */
   VkImageLayout initial_layout;

   /** VkAttachmentDescription2::finalLayout */
   VkImageLayout final_layout;

   /** VkAttachmentDescriptionStencilLayout::stencilInitialLayout
    *
    * If VK_KHR_separate_depth_stencil_layouts is not used, this will be
    * initial_layout if format contains stencil and VK_IMAGE_LAYOUT_UNDEFINED
    * otherwise.
    */
   VkImageLayout initial_stencil_layout;

   /** VkAttachmentDescriptionStencilLayout::stencilFinalLayout
    *
    * If VK_KHR_separate_depth_stencil_layouts is not used, this will be
    * final_layout if format contains stencil and VK_IMAGE_LAYOUT_UNDEFINED
    * otherwise.
    */
   VkImageLayout final_stencil_layout;
};

/***/
struct vk_subpass_dependency {
   /** VkSubpassDependency2::dependencyFlags */
   VkDependencyFlags flags;

   /** VkSubpassDependency2::srcSubpass */
   uint32_t src_subpass;

   /** VkSubpassDependency2::dstSubpass */
   uint32_t dst_subpass;

   /** VkSubpassDependency2::srcStageMask */
   VkPipelineStageFlags2 src_stage_mask;

   /** VkSubpassDependency2::dstStageMask */
   VkPipelineStageFlags2 dst_stage_mask;

   /** VkSubpassDependency2::srcAccessMask */
   VkAccessFlags2 src_access_mask;

   /** VkSubpassDependency2::dstAccessMask */
   VkAccessFlags2 dst_access_mask;

   /** VkSubpassDependency2::viewOffset */
   int32_t view_offset;
};

/***/
struct vk_render_pass {
   struct vk_object_base base;

   /** True if this render pass uses multiview
    *
    * This is true if all subpasses have viewMask != 0.
    */
   bool is_multiview;

   /** Views used by this render pass or 1 for non-multiview */
   uint32_t view_mask;

   /** VkRenderPassCreateInfo2::attachmentCount */
   uint32_t attachment_count;

   /** VkRenderPassCreateInfo2::pAttachments */
   struct vk_render_pass_attachment *attachments;

   /** VkRenderPassCreateInfo2::subpassCount */
   uint32_t subpass_count;

   /** VkRenderPassCreateInfo2::subpasses */
   struct vk_subpass *subpasses;

   /** VkRenderPassCreateInfo2::dependencyCount */
   uint32_t dependency_count;

   /** VkRenderPassFragmentDensityMapCreateInfoEXT::fragmentDensityMapAttachment */
   VkAttachmentReference fragment_density_map;

   /** VkRenderPassCreateInfo2::pDependencies */
   struct vk_subpass_dependency *dependencies;
};

VK_DEFINE_NONDISP_HANDLE_CASTS(vk_render_pass, base, VkRenderPass,
                               VK_OBJECT_TYPE_RENDER_PASS);

/** Returns the VkPipelineRenderingCreateInfo for a graphics pipeline
 *
 * For render-pass-free drivers, this can be used in the implementation of
 * vkCreateGraphicsPipelines to get the VkPipelineRenderingCreateInfo.  If
 * VkGraphicsPipelineCreateInfo::renderPass is not VK_NULL_HANDLE, it will
 * return a representation of the specified subpass as a
 * VkPipelineRenderingCreateInfo.  If VkGraphicsPipelineCreateInfo::renderPass
 * is VK_NULL_HANDLE and there is a VkPipelineRenderingCreateInfo in the pNext
 * chain of VkGraphicsPipelineCreateInfo, it will return that.
 *
 * :param info: |in|  One of the pCreateInfos from vkCreateGraphicsPipelines
 */
const VkPipelineRenderingCreateInfo *
vk_get_pipeline_rendering_create_info(const VkGraphicsPipelineCreateInfo *info);

/** Returns any extra VkPipelineCreateFlags from the render pass
 *
 * For render-pass-free drivers, this can be used to get any extra pipeline
 * create flags implied by the render pass.  In particular, a render pass may
 * want to add one or both of the following:
 *
 *  - VK_PIPELINE_CREATE_COLOR_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT
 *  - VK_PIPELINE_CREATE_DEPTH_STENCIL_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT
 *  - VK_PIPELINE_CREATE_RENDERING_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR
 *  - VK_PIPELINE_CREATE_RENDERING_FRAGMENT_DENSITY_MAP_ATTACHMENT_BIT_EXT
 *
 * If VkGraphicsPipelineCreateInfo::renderPass is VK_NULL_HANDLE, the relevant
 * flags from VkGraphicsPipelineCreateInfo::flags will be returned.
 *
 * :param info: |in|  One of the pCreateInfos from vkCreateGraphicsPipelines
 */
VkPipelineCreateFlags2KHR
vk_get_pipeline_rendering_flags(const VkGraphicsPipelineCreateInfo *info);

/** Returns the VkAttachmentSampleCountInfoAMD for a graphics pipeline
 *
 * For render-pass-free drivers, this can be used in the implementaiton of
 * vkCreateGraphicsPipelines to get the VkAttachmentSampleCountInfoAMD.  If
 * VkGraphicsPipelineCreateInfo::renderPass is not VK_NULL_HANDLE, it will
 * return the sample counts from the specified subpass as a
 * VkAttachmentSampleCountInfoAMD.  If VkGraphicsPipelineCreateInfo::renderPass
 * is VK_NULL_HANDLE and there is a VkAttachmentSampleCountInfoAMD in the pNext
 * chain of VkGraphicsPipelineCreateInfo, it will return that.
 *
 * :param info: |in|  One of the pCreateInfos from vkCreateGraphicsPipelines
 */
const VkAttachmentSampleCountInfoAMD *
vk_get_pipeline_sample_count_info_amd(const VkGraphicsPipelineCreateInfo *info);

/**
 * Returns the VkCommandBufferInheritanceRenderingInfo for secondary command
 * buffer execution
 *
 * For render-pass-free drivers, this can be used in the implementation of
 * vkCmdExecuteCommands to get the VkCommandBufferInheritanceRenderingInfo.
 * If VkCommandBufferInheritanceInfo::renderPass is not VK_NULL_HANDLE, it
 * will return a representation of the specified subpass as a
 * VkCommandBufferInheritanceRenderingInfo.  If
 * VkCommandBufferInheritanceInfo::renderPass is not VK_NULL_HANDLE and there
 * is a VkCommandBufferInheritanceRenderingInfo in the pNext chain of
 * VkCommandBufferBeginInfo, it will return that.
 *
 * :param level:        |in|  The nesting level of this command buffer
 * :param pBeginInfo:   |in|  The pBeginInfo from vkBeginCommandBuffer
 */
const VkCommandBufferInheritanceRenderingInfo *
vk_get_command_buffer_inheritance_rendering_info(
   VkCommandBufferLevel level,
   const VkCommandBufferBeginInfo *pBeginInfo);

struct vk_gcbiarr_data {
   VkRenderingInfo rendering;
   VkRenderingFragmentShadingRateAttachmentInfoKHR fsr_att;
   VkRenderingAttachmentInfo attachments[];
};

#define VK_GCBIARR_DATA_SIZE(max_color_rts) (\
   sizeof(struct vk_gcbiarr_data) + \
   sizeof(VkRenderingAttachmentInfo) * ((max_color_rts) + 2) \
)

/**
 * Constructs a VkRenderingInfo for the inheritance rendering info
 *
 * For render-pass-free drivers, this can be used in the implementaiton of
 * vkCmdExecuteCommands to get a VkRenderingInfo representing the subpass and
 * framebuffer provided via the inheritance info for a command buffer created
 * with VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT.  The mental model
 * here is that VkExecuteCommands() implicitly suspends the render pass and
 * VkBeginCommandBuffer() resumes it.  If a VkRenderingInfo cannot be
 * constructed due to a missing framebuffer or similar, NULL will be
 * returned.
 *
 * :param level:        |in|  The nesting level of this command buffer
 * :param pBeginInfo:   |in|  The pBeginInfo from vkBeginCommandBuffer
 * :param stack_data:   |out| An opaque blob of data which will be overwritten by
 *                            this function, passed in from the caller to avoid
 *                            heap allocations.  It must be at least
 *                            VK_GCBIARR_DATA_SIZE(max_color_rts) bytes.
 */
const VkRenderingInfo *
vk_get_command_buffer_inheritance_as_rendering_resume(
   VkCommandBufferLevel level,
   const VkCommandBufferBeginInfo *pBeginInfo,
   void *stack_data);

/**
 * Return true if the subpass dependency is framebuffer-local.
 */
static bool
vk_subpass_dependency_is_fb_local(const VkSubpassDependency2 *dep,
                                  VkPipelineStageFlags2 src_stage_mask,
                                  VkPipelineStageFlags2 dst_stage_mask)
{
   if (dep->srcSubpass == VK_SUBPASS_EXTERNAL ||
       dep->dstSubpass == VK_SUBPASS_EXTERNAL)
      return true;

  /* This is straight from the Vulkan 1.2 spec, section 7.1.4 "Framebuffer
   * Region Dependencies":
   */
   const VkPipelineStageFlags2 framebuffer_space_stages =
      VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT |
      VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT |
      VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT |
      VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

   const VkPipelineStageFlags2 src_framebuffer_space_stages =
      framebuffer_space_stages | VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
   const VkPipelineStageFlags2 dst_framebuffer_space_stages =
      framebuffer_space_stages | VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;

   /* Check for frambuffer-space dependency. */
   if ((src_stage_mask & ~src_framebuffer_space_stages) ||
       (dst_stage_mask & ~dst_framebuffer_space_stages))
      return false;

   /* Check for framebuffer-local dependency. */
   return dep->dependencyFlags & VK_DEPENDENCY_BY_REGION_BIT;
}

uint32_t
vk_command_buffer_get_attachment_layout(const struct vk_command_buffer *cmd_buffer,
                                        const struct vk_image *image,
                                        VkImageLayout *out_layout,
                                        VkImageLayout *out_stencil_layout);

void
vk_command_buffer_set_attachment_layout(struct vk_command_buffer *cmd_buffer,
                                        uint32_t att_idx,
                                        VkImageLayout layout,
                                        VkImageLayout stencil_layout);

#ifdef __cplusplus
}
#endif

#endif /* VK_RENDER_PASS_H */
