// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//
//
//

#include "barrier.h"

//
//
//

void
vk_barrier_compute_w_to_compute_r(VkCommandBuffer cb)
{
  static VkMemoryBarrier const mb = {

    .sType         = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
    .pNext         = NULL,
    .srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT,
    .dstAccessMask = VK_ACCESS_SHADER_READ_BIT
  };

  vkCmdPipelineBarrier(cb,
                       VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                       VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                       0,
                       1,
                       &mb,
                       0,
                       NULL,
                       0,
                       NULL);
}

//
//
//

void
vk_barrier_compute_w_to_transfer_r(VkCommandBuffer cb)
{
  static VkMemoryBarrier const mb = {

    .sType         = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
    .pNext         = NULL,
    .srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT,
    .dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT
  };

  vkCmdPipelineBarrier(cb,
                       VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                       VK_PIPELINE_STAGE_TRANSFER_BIT,
                       0,
                       1,
                       &mb,
                       0,
                       NULL,
                       0,
                       NULL);
}

//
//
//

void
vk_barrier_transfer_w_to_compute_r(VkCommandBuffer cb)
{
  static VkMemoryBarrier const mb = {

    .sType         = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
    .pNext         = NULL,
    .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
    .dstAccessMask = VK_ACCESS_SHADER_READ_BIT
  };

  vkCmdPipelineBarrier(cb,
                       VK_PIPELINE_STAGE_TRANSFER_BIT,
                       VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                       0,
                       1,
                       &mb,
                       0,
                       NULL,
                       0,
                       NULL);
}

//
//
//

void
vk_barrier_transfer_w_to_compute_w(VkCommandBuffer cb)
{
  static VkMemoryBarrier const mb = {

    .sType         = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
    .pNext         = NULL,
    .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
    .dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT
  };

  vkCmdPipelineBarrier(cb,
                       VK_PIPELINE_STAGE_TRANSFER_BIT,
                       VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                       0,
                       1,
                       &mb,
                       0,
                       NULL,
                       0,
                       NULL);
}

//
//
//

void
vk_barrier_compute_w_to_indirect_compute_r(VkCommandBuffer cb)
{
  static VkMemoryBarrier const mb = {

    .sType         = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
    .pNext         = NULL,
    .srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT,
    .dstAccessMask = VK_ACCESS_INDIRECT_COMMAND_READ_BIT |  //
                     VK_ACCESS_SHADER_READ_BIT
  };

  vkCmdPipelineBarrier(cb,
                       VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                       VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                       0,
                       1,
                       &mb,
                       0,
                       NULL,
                       0,
                       NULL);
}

//
//
//

void
vk_barrier_transfer_w_compute_w_to_transfer_r(VkCommandBuffer cb)
{
  static VkMemoryBarrier const mb = {

    .sType         = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
    .pNext         = NULL,
    .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT |  //
                     VK_ACCESS_SHADER_WRITE_BIT,
    .dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT
  };

  vkCmdPipelineBarrier(cb,
                       VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                       VK_PIPELINE_STAGE_TRANSFER_BIT,
                       0,
                       1,
                       &mb,
                       0,
                       NULL,
                       0,
                       NULL);
}

//
//
//

void
vk_barrier_compute_w_to_host_r(VkCommandBuffer cb)
{
  static VkMemoryBarrier const mb = {

    .sType         = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
    .pNext         = NULL,
    .srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT,
    .dstAccessMask = VK_ACCESS_HOST_READ_BIT
  };

  vkCmdPipelineBarrier(cb,
                       VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                       VK_PIPELINE_STAGE_HOST_BIT,
                       0,
                       1,
                       &mb,
                       0,
                       NULL,
                       0,
                       NULL);
}

//
//
//

void
vk_barrier_transfer_w_to_host_r(VkCommandBuffer cb)
{
  static VkMemoryBarrier const mb = {

    .sType         = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
    .pNext         = NULL,
    .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
    .dstAccessMask = VK_ACCESS_HOST_READ_BIT
  };

  vkCmdPipelineBarrier(cb,
                       VK_PIPELINE_STAGE_TRANSFER_BIT,
                       VK_PIPELINE_STAGE_HOST_BIT,
                       0,
                       1,
                       &mb,
                       0,
                       NULL,
                       0,
                       NULL);
}

//
//
//

void
vk_memory_barrier(VkCommandBuffer      cb,
                  VkPipelineStageFlags src_stage,
                  VkAccessFlags        src_mask,
                  VkPipelineStageFlags dst_stage,
                  VkAccessFlags        dst_mask)
{
  VkMemoryBarrier const mb = { .sType         = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
                               .pNext         = NULL,
                               .srcAccessMask = src_mask,
                               .dstAccessMask = dst_mask };

  vkCmdPipelineBarrier(cb, src_stage, dst_stage, 0, 1, &mb, 0, NULL, 0, NULL);
}

//
//
//

void
vk_barrier_debug(VkCommandBuffer cb)
{
  static VkMemoryBarrier const mb = {

    .sType         = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
    .pNext         = NULL,
    .srcAccessMask = VK_ACCESS_INDIRECT_COMMAND_READ_BIT |           //
                     VK_ACCESS_INDEX_READ_BIT |                      //
                     VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT |           //
                     VK_ACCESS_UNIFORM_READ_BIT |                    //
                     VK_ACCESS_INPUT_ATTACHMENT_READ_BIT |           //
                     VK_ACCESS_SHADER_READ_BIT |                     //
                     VK_ACCESS_SHADER_WRITE_BIT |                    //
                     VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |           //
                     VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |          //
                     VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |   //
                     VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |  //
                     VK_ACCESS_TRANSFER_READ_BIT |                   //
                     VK_ACCESS_TRANSFER_WRITE_BIT |                  //
                     VK_ACCESS_HOST_READ_BIT |                       //
                     VK_ACCESS_HOST_WRITE_BIT,
    .dstAccessMask = VK_ACCESS_INDIRECT_COMMAND_READ_BIT |           //
                     VK_ACCESS_INDEX_READ_BIT |                      //
                     VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT |           //
                     VK_ACCESS_UNIFORM_READ_BIT |                    //
                     VK_ACCESS_INPUT_ATTACHMENT_READ_BIT |           //
                     VK_ACCESS_SHADER_READ_BIT |                     //
                     VK_ACCESS_SHADER_WRITE_BIT |                    //
                     VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |           //
                     VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |          //
                     VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |   //
                     VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |  //
                     VK_ACCESS_TRANSFER_READ_BIT |                   //
                     VK_ACCESS_TRANSFER_WRITE_BIT |                  //
                     VK_ACCESS_HOST_READ_BIT |                       //
                     VK_ACCESS_HOST_WRITE_BIT
  };

  vkCmdPipelineBarrier(cb,
                       VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                       VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                       0,
                       1,
                       &mb,
                       0,
                       NULL,
                       0,
                       NULL);
}

//
//
//
