/*
 * Copyright © 2021 Collabora Ltd.
 *
 * Derived from tu_pass.c which is:
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

#include "vk_format.h"
#include "vk_util.h"

VkResult
panvk_CreateRenderPass2(VkDevice _device,
                        const VkRenderPassCreateInfo2 *pCreateInfo,
                        const VkAllocationCallbacks *pAllocator,
                        VkRenderPass *pRenderPass)
{
   VK_FROM_HANDLE(panvk_device, device, _device);
   struct panvk_render_pass *pass;
   size_t size;
   size_t attachments_offset;
   VkRenderPassMultiviewCreateInfo *multiview_info = NULL;

   assert(pCreateInfo->sType == VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2);

   size = sizeof(*pass);
   size += pCreateInfo->subpassCount * sizeof(pass->subpasses[0]);
   attachments_offset = size;
   size += pCreateInfo->attachmentCount * sizeof(pass->attachments[0]);

   pass = vk_object_zalloc(&device->vk, pAllocator, size,
                           VK_OBJECT_TYPE_RENDER_PASS);
   if (pass == NULL)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   pass->attachment_count = pCreateInfo->attachmentCount;
   pass->subpass_count = pCreateInfo->subpassCount;
   pass->attachments = (void *)pass + attachments_offset;

   vk_foreach_struct_const(ext, pCreateInfo->pNext) {
      switch (ext->sType) {
      case VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO:
         multiview_info = (VkRenderPassMultiviewCreateInfo *)ext;
         break;
      default:
         break;
      }
   }

   for (uint32_t i = 0; i < pCreateInfo->attachmentCount; i++) {
      struct panvk_render_pass_attachment *att = &pass->attachments[i];

      att->format =
         vk_format_to_pipe_format(pCreateInfo->pAttachments[i].format);
      att->samples = pCreateInfo->pAttachments[i].samples;
      att->load_op = pCreateInfo->pAttachments[i].loadOp;
      att->stencil_load_op = pCreateInfo->pAttachments[i].stencilLoadOp;
      att->initial_layout = pCreateInfo->pAttachments[i].initialLayout;
      att->final_layout = pCreateInfo->pAttachments[i].finalLayout;
      att->store_op = pCreateInfo->pAttachments[i].storeOp;
      att->stencil_store_op = pCreateInfo->pAttachments[i].stencilStoreOp;
      att->first_used_in_subpass = ~0;
   }

   uint32_t subpass_attachment_count = 0;
   struct panvk_subpass_attachment *p;
   for (uint32_t i = 0; i < pCreateInfo->subpassCount; i++) {
      const VkSubpassDescription2 *desc = &pCreateInfo->pSubpasses[i];

      subpass_attachment_count +=
         desc->inputAttachmentCount + desc->colorAttachmentCount +
         (desc->pResolveAttachments ? desc->colorAttachmentCount : 0) +
         (desc->pDepthStencilAttachment != NULL);
   }

   if (subpass_attachment_count) {
      pass->subpass_attachments = vk_alloc2(
         &device->vk.alloc, pAllocator,
         subpass_attachment_count * sizeof(struct panvk_subpass_attachment), 8,
         VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
      if (pass->subpass_attachments == NULL) {
         vk_object_free(&device->vk, pAllocator, pass);
         return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);
      }
   }

   p = pass->subpass_attachments;
   for (uint32_t i = 0; i < pCreateInfo->subpassCount; i++) {
      const VkSubpassDescription2 *desc = &pCreateInfo->pSubpasses[i];
      struct panvk_subpass *subpass = &pass->subpasses[i];

      subpass->input_count = desc->inputAttachmentCount;
      subpass->color_count = desc->colorAttachmentCount;
      if (multiview_info)
         subpass->view_mask = multiview_info->pViewMasks[i];

      if (desc->inputAttachmentCount > 0) {
         subpass->input_attachments = p;
         p += desc->inputAttachmentCount;

         for (uint32_t j = 0; j < desc->inputAttachmentCount; j++) {
            subpass->input_attachments[j] = (struct panvk_subpass_attachment){
               .idx = desc->pInputAttachments[j].attachment,
               .layout = desc->pInputAttachments[j].layout,
            };
            if (desc->pInputAttachments[j].attachment != VK_ATTACHMENT_UNUSED)
               pass->attachments[desc->pInputAttachments[j].attachment]
                  .view_mask |= subpass->view_mask;
         }
      }

      if (desc->colorAttachmentCount > 0) {
         subpass->color_attachments = p;
         p += desc->colorAttachmentCount;

         for (uint32_t j = 0; j < desc->colorAttachmentCount; j++) {
            uint32_t idx = desc->pColorAttachments[j].attachment;

            subpass->color_attachments[j] = (struct panvk_subpass_attachment){
               .idx = idx,
               .layout = desc->pColorAttachments[j].layout,
            };

            if (idx != VK_ATTACHMENT_UNUSED) {
               pass->attachments[idx].view_mask |= subpass->view_mask;
               if (pass->attachments[idx].first_used_in_subpass == ~0) {
                  pass->attachments[idx].first_used_in_subpass = i;
                  if (pass->attachments[idx].load_op ==
                      VK_ATTACHMENT_LOAD_OP_CLEAR)
                     subpass->color_attachments[j].clear = true;
                  else if (pass->attachments[idx].load_op ==
                           VK_ATTACHMENT_LOAD_OP_LOAD)
                     subpass->color_attachments[j].preload = true;
               } else {
                  subpass->color_attachments[j].preload = true;
               }
            }
         }
      }

      if (desc->pResolveAttachments) {
         subpass->resolve_attachments = p;
         p += desc->colorAttachmentCount;

         for (uint32_t j = 0; j < desc->colorAttachmentCount; j++) {
            uint32_t idx = desc->pResolveAttachments[j].attachment;

            subpass->resolve_attachments[j] = (struct panvk_subpass_attachment){
               .idx = idx,
               .layout = desc->pResolveAttachments[j].layout,
            };

            if (idx != VK_ATTACHMENT_UNUSED)
               pass->attachments[idx].view_mask |= subpass->view_mask;
         }
      }

      unsigned idx = desc->pDepthStencilAttachment
                        ? desc->pDepthStencilAttachment->attachment
                        : VK_ATTACHMENT_UNUSED;
      subpass->zs_attachment.idx = idx;
      if (idx != VK_ATTACHMENT_UNUSED) {
         subpass->zs_attachment.layout = desc->pDepthStencilAttachment->layout;
         pass->attachments[idx].view_mask |= subpass->view_mask;

         if (pass->attachments[idx].first_used_in_subpass == ~0) {
            pass->attachments[idx].first_used_in_subpass = i;
            if (pass->attachments[idx].load_op == VK_ATTACHMENT_LOAD_OP_CLEAR)
               subpass->zs_attachment.clear = true;
            else if (pass->attachments[idx].load_op ==
                     VK_ATTACHMENT_LOAD_OP_LOAD)
               subpass->zs_attachment.preload = true;
         } else {
            subpass->zs_attachment.preload = true;
         }
      }
   }

   *pRenderPass = panvk_render_pass_to_handle(pass);
   return VK_SUCCESS;
}

void
panvk_DestroyRenderPass(VkDevice _device, VkRenderPass _pass,
                        const VkAllocationCallbacks *pAllocator)
{
   VK_FROM_HANDLE(panvk_device, device, _device);
   VK_FROM_HANDLE(panvk_render_pass, pass, _pass);

   if (!pass)
      return;

   vk_free2(&device->vk.alloc, pAllocator, pass->subpass_attachments);
   vk_object_free(&device->vk, pAllocator, pass);
}

void
panvk_GetRenderAreaGranularity(VkDevice _device, VkRenderPass renderPass,
                               VkExtent2D *pGranularity)
{
   /* TODO: Return the actual tile size for the render pass? */
   *pGranularity = (VkExtent2D){1, 1};
}
