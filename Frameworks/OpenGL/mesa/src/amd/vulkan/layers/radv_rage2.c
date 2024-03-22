/*
 * Copyright © 2023 Valve Corporation
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

#include "radv_private.h"
#include "vk_common_entrypoints.h"
#include "vk_framebuffer.h"

VKAPI_ATTR void VKAPI_CALL
rage2_CmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                         VkSubpassContents contents)
{
   VK_FROM_HANDLE(vk_framebuffer, framebuffer, pRenderPassBegin->framebuffer);
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   struct radv_device *device = cmd_buffer->device;

   VkRenderPassBeginInfo render_pass_begin = {
      .sType = pRenderPassBegin->sType,
      .pNext = pRenderPassBegin->pNext,
      .renderPass = pRenderPassBegin->renderPass,
      .framebuffer = pRenderPassBegin->framebuffer,
      .clearValueCount = pRenderPassBegin->clearValueCount,
      .pClearValues = pRenderPassBegin->pClearValues,
   };

   /* RAGE2 seems to incorrectly set the render area and with dynamic rendering the concept of
    * framebuffer dimensions goes away. Forcing the render area to be the framebuffer dimensions
    * restores previous logic and it fixes rendering issues.
    */
   render_pass_begin.renderArea.offset.x = 0;
   render_pass_begin.renderArea.offset.y = 0;
   render_pass_begin.renderArea.extent.width = framebuffer->width;
   render_pass_begin.renderArea.extent.height = framebuffer->height;

   device->layer_dispatch.app.CmdBeginRenderPass(commandBuffer, &render_pass_begin, contents);
}
