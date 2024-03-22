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
#include "vk_descriptors.h"
#include "vk_util.h"
#include "util/u_math.h"
#include "util/u_inlines.h"

static bool
binding_has_immutable_samplers(const VkDescriptorSetLayoutBinding *binding)
{
   switch (binding->descriptorType) {
   case VK_DESCRIPTOR_TYPE_SAMPLER:
   case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
      return binding->pImmutableSamplers != NULL;

   default:
      return false;
   }
}

static void
lvp_descriptor_set_layout_destroy(struct vk_device *_device, struct vk_descriptor_set_layout *_layout)
{
   struct lvp_device *device = container_of(_device, struct lvp_device, vk);
   struct lvp_descriptor_set_layout *set_layout = (void*)vk_to_lvp_descriptor_set_layout(_layout);

   _layout->ref_cnt = UINT32_MAX;
   lvp_descriptor_set_destroy(device, set_layout->immutable_set);

   vk_descriptor_set_layout_destroy(_device, _layout);
}

VKAPI_ATTR VkResult VKAPI_CALL lvp_CreateDescriptorSetLayout(
    VkDevice                                    _device,
    const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorSetLayout*                      pSetLayout)
{
   LVP_FROM_HANDLE(lvp_device, device, _device);
   struct lvp_descriptor_set_layout *set_layout;

   assert(pCreateInfo->sType == VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO);
   uint32_t num_bindings = 0;
   uint32_t immutable_sampler_count = 0;
   for (uint32_t j = 0; j < pCreateInfo->bindingCount; j++) {
      num_bindings = MAX2(num_bindings, pCreateInfo->pBindings[j].binding + 1);
      /* From the Vulkan 1.1.97 spec for VkDescriptorSetLayoutBinding:
       *
       *    "If descriptorType specifies a VK_DESCRIPTOR_TYPE_SAMPLER or
       *    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER type descriptor, then
       *    pImmutableSamplers can be used to initialize a set of immutable
       *    samplers. [...]  If descriptorType is not one of these descriptor
       *    types, then pImmutableSamplers is ignored.
       *
       * We need to be careful here and only parse pImmutableSamplers if we
       * have one of the right descriptor types.
       */
      if (binding_has_immutable_samplers(&pCreateInfo->pBindings[j]))
         immutable_sampler_count += pCreateInfo->pBindings[j].descriptorCount;
   }

   size_t size = sizeof(struct lvp_descriptor_set_layout) +
                 num_bindings * sizeof(set_layout->binding[0]) +
                 immutable_sampler_count * sizeof(struct lvp_sampler *);

   set_layout = vk_descriptor_set_layout_zalloc(&device->vk, size);
   if (!set_layout)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   set_layout->immutable_sampler_count = immutable_sampler_count;
   /* We just allocate all the samplers at the end of the struct */
   struct lvp_sampler **samplers =
      (struct lvp_sampler **)&set_layout->binding[num_bindings];

   set_layout->binding_count = num_bindings;
   set_layout->shader_stages = 0;
   set_layout->size = 0;

   VkDescriptorSetLayoutBinding *bindings = NULL;
   VkResult result = vk_create_sorted_bindings(pCreateInfo->pBindings,
                                               pCreateInfo->bindingCount,
                                               &bindings);
   if (result != VK_SUCCESS) {
      vk_descriptor_set_layout_unref(&device->vk, &set_layout->vk);
      return vk_error(device, result);
   }

   uint32_t uniform_block_size = 0;

   uint32_t dynamic_offset_count = 0;
   for (uint32_t j = 0; j < pCreateInfo->bindingCount; j++) {
      const VkDescriptorSetLayoutBinding *binding = bindings + j;
      uint32_t b = binding->binding;

      uint32_t descriptor_count = binding->descriptorCount;
      if (binding->descriptorType == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK)
         descriptor_count = 1;

      set_layout->binding[b].array_size = descriptor_count;
      set_layout->binding[b].descriptor_index = set_layout->size;
      set_layout->binding[b].type = binding->descriptorType;
      set_layout->binding[b].valid = true;
      set_layout->binding[b].uniform_block_offset = 0;
      set_layout->binding[b].uniform_block_size = 0;

      if (binding->descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
          binding->descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC) {
         set_layout->binding[b].dynamic_index = dynamic_offset_count;
         dynamic_offset_count += binding->descriptorCount;
      }

      uint8_t max_plane_count = 1;
      if (binding_has_immutable_samplers(binding)) {
         set_layout->binding[b].immutable_samplers = samplers;
         samplers += binding->descriptorCount;

         for (uint32_t i = 0; i < binding->descriptorCount; i++) {
            VK_FROM_HANDLE(lvp_sampler, sampler, binding->pImmutableSamplers[i]);
            set_layout->binding[b].immutable_samplers[i] = sampler;
            const uint8_t sampler_plane_count = sampler->vk.ycbcr_conversion ?
               vk_format_get_plane_count(sampler->vk.ycbcr_conversion->state.format) : 1;
            if (max_plane_count < sampler_plane_count)
               max_plane_count = sampler_plane_count;
         }
      }

      set_layout->binding[b].stride = max_plane_count;
      set_layout->size += descriptor_count * max_plane_count;

      switch (binding->descriptorType) {
      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
         break;
      case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK:
         set_layout->binding[b].uniform_block_offset = uniform_block_size;
         set_layout->binding[b].uniform_block_size = binding->descriptorCount;
         uniform_block_size += binding->descriptorCount;
         break;
      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
         break;

      case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
      case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
      case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
         break;
      case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
      case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
      case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
         break;
      default:
         break;
      }

      set_layout->shader_stages |= binding->stageFlags;
   }

   for (uint32_t i = 0; i < pCreateInfo->bindingCount; i++)
      set_layout->binding[i].uniform_block_offset += set_layout->size * sizeof(struct lp_descriptor);

   free(bindings);

   set_layout->dynamic_offset_count = dynamic_offset_count;

   if (set_layout->binding_count == set_layout->immutable_sampler_count) {
      /* create a bindable set with all the immutable samplers */
      lvp_descriptor_set_create(device, set_layout, &set_layout->immutable_set);
      vk_descriptor_set_layout_unref(&device->vk, &set_layout->vk);
      set_layout->vk.destroy = lvp_descriptor_set_layout_destroy;
   }

   *pSetLayout = lvp_descriptor_set_layout_to_handle(set_layout);

   return VK_SUCCESS;
}

struct lvp_pipeline_layout *
lvp_pipeline_layout_create(struct lvp_device *device,
                           const VkPipelineLayoutCreateInfo*           pCreateInfo,
                           const VkAllocationCallbacks*                pAllocator)
{
   struct lvp_pipeline_layout *layout = vk_pipeline_layout_zalloc(&device->vk, sizeof(*layout),
                                                                  pCreateInfo);

   layout->push_constant_size = 0;
   for (unsigned i = 0; i < pCreateInfo->pushConstantRangeCount; ++i) {
      const VkPushConstantRange *range = pCreateInfo->pPushConstantRanges + i;
      layout->push_constant_size = MAX2(layout->push_constant_size,
                                        range->offset + range->size);
      layout->push_constant_stages |= (range->stageFlags & LVP_STAGE_MASK);
   }
   layout->push_constant_size = align(layout->push_constant_size, 16);
   return layout;
}

VKAPI_ATTR VkResult VKAPI_CALL lvp_CreatePipelineLayout(
    VkDevice                                    _device,
    const VkPipelineLayoutCreateInfo*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPipelineLayout*                           pPipelineLayout)
{
   LVP_FROM_HANDLE(lvp_device, device, _device);
   struct lvp_pipeline_layout *layout = lvp_pipeline_layout_create(device, pCreateInfo, pAllocator);
   *pPipelineLayout = lvp_pipeline_layout_to_handle(layout);

   return VK_SUCCESS;
}

static struct pipe_resource *
get_buffer_resource(struct pipe_context *ctx, const VkDescriptorAddressInfoEXT *bda)
{
   struct pipe_screen *pscreen = ctx->screen;
   struct pipe_resource templ = {0};

   templ.screen = pscreen;
   templ.target = PIPE_BUFFER;
   templ.format = PIPE_FORMAT_R8_UNORM;
   templ.width0 = bda->range;
   templ.height0 = 1;
   templ.depth0 = 1;
   templ.array_size = 1;
   templ.bind |= PIPE_BIND_SAMPLER_VIEW;
   templ.bind |= PIPE_BIND_SHADER_IMAGE;
   templ.flags = PIPE_RESOURCE_FLAG_DONT_OVER_ALLOCATE;

   uint64_t size;
   struct pipe_resource *pres = pscreen->resource_create_unbacked(pscreen, &templ, &size);
   assert(size == bda->range);
   pscreen->resource_bind_backing(pscreen, pres, (void *)(uintptr_t)bda->address, 0);
   return pres;
}

static struct lp_texture_handle
get_texture_handle_bda(struct lvp_device *device, const VkDescriptorAddressInfoEXT *bda, enum pipe_format format)
{
   struct pipe_context *ctx = device->queue.ctx;

   struct pipe_resource *pres = get_buffer_resource(ctx, bda);

   struct pipe_sampler_view templ;
   memset(&templ, 0, sizeof(templ));
   templ.target = PIPE_BUFFER;
   templ.swizzle_r = PIPE_SWIZZLE_X;
   templ.swizzle_g = PIPE_SWIZZLE_Y;
   templ.swizzle_b = PIPE_SWIZZLE_Z;
   templ.swizzle_a = PIPE_SWIZZLE_W;
   templ.format = format;
   templ.u.buf.size = bda->range;
   templ.texture = pres;
   templ.context = ctx;
   struct pipe_sampler_view *view = ctx->create_sampler_view(ctx, pres, &templ);

   simple_mtx_lock(&device->queue.lock);

   struct lp_texture_handle *handle = (void *)(uintptr_t)ctx->create_texture_handle(ctx, view, NULL);
   util_dynarray_append(&device->bda_texture_handles, struct lp_texture_handle *, handle);

   simple_mtx_unlock(&device->queue.lock);

   ctx->sampler_view_destroy(ctx, view);
   pipe_resource_reference(&pres, NULL);

   return *handle;
}

static struct lp_texture_handle
get_image_handle_bda(struct lvp_device *device, const VkDescriptorAddressInfoEXT *bda, enum pipe_format format)
{
   struct pipe_context *ctx = device->queue.ctx;

   struct pipe_resource *pres = get_buffer_resource(ctx, bda);
   struct pipe_image_view view = {0};
   view.resource = pres;
   view.format = format;
   view.u.buf.size = bda->range;

   simple_mtx_lock(&device->queue.lock);

   struct lp_texture_handle *handle = (void *)(uintptr_t)ctx->create_image_handle(ctx, &view);
   util_dynarray_append(&device->bda_image_handles, struct lp_texture_handle *, handle);

   simple_mtx_unlock(&device->queue.lock);

   pipe_resource_reference(&pres, NULL);

   return *handle;
}

VkResult
lvp_descriptor_set_create(struct lvp_device *device,
                          struct lvp_descriptor_set_layout *layout,
                          struct lvp_descriptor_set **out_set)
{
   struct lvp_descriptor_set *set = vk_zalloc(&device->vk.alloc /* XXX: Use the pool */,
      sizeof(struct lvp_descriptor_set), 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (!set)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   vk_object_base_init(&device->vk, &set->base,
                       VK_OBJECT_TYPE_DESCRIPTOR_SET);
   set->layout = layout;
   vk_descriptor_set_layout_ref(&layout->vk);

   uint64_t bo_size = layout->size * sizeof(struct lp_descriptor);

   for (unsigned i = 0; i < layout->binding_count; i++)
      bo_size += layout->binding[i].uniform_block_size;

   struct pipe_resource template = {
      .bind = PIPE_BIND_CONSTANT_BUFFER,
      .screen = device->pscreen,
      .target = PIPE_BUFFER,
      .format = PIPE_FORMAT_R8_UNORM,
      .width0 = bo_size,
      .height0 = 1,
      .depth0 = 1,
      .array_size = 1,
      .flags = PIPE_RESOURCE_FLAG_DONT_OVER_ALLOCATE,
   };

   set->bo = device->pscreen->resource_create_unbacked(device->pscreen, &template, &bo_size);
   set->pmem = device->pscreen->allocate_memory(device->pscreen, bo_size);

   set->map = device->pscreen->map_memory(device->pscreen, set->pmem);
   memset(set->map, 0, bo_size);

   device->pscreen->resource_bind_backing(device->pscreen, set->bo, set->pmem, 0);

   for (uint32_t binding_index = 0; binding_index < layout->binding_count; binding_index++) {
      const struct lvp_descriptor_set_binding_layout *bind_layout = &set->layout->binding[binding_index];
      if (!bind_layout->immutable_samplers)
         continue;

      struct lp_descriptor *desc = set->map;
      desc += bind_layout->descriptor_index;

      for (uint32_t sampler_index = 0; sampler_index < bind_layout->array_size; sampler_index++) {
         if (bind_layout->immutable_samplers[sampler_index]) {
            for (uint32_t s = 0; s < bind_layout->stride; s++)  {
               int idx = sampler_index * bind_layout->stride + s;
               desc[idx] = bind_layout->immutable_samplers[sampler_index]->desc;
            }
         }
      }
   }

   *out_set = set;

   return VK_SUCCESS;
}

void
lvp_descriptor_set_destroy(struct lvp_device *device,
                           struct lvp_descriptor_set *set)
{
   pipe_resource_reference(&set->bo, NULL);
   device->pscreen->unmap_memory(device->pscreen, set->pmem);
   device->pscreen->free_memory(device->pscreen, set->pmem);

   vk_descriptor_set_layout_unref(&device->vk, &set->layout->vk);
   vk_object_base_finish(&set->base);
   vk_free(&device->vk.alloc, set);
}

VKAPI_ATTR VkResult VKAPI_CALL lvp_AllocateDescriptorSets(
    VkDevice                                    _device,
    const VkDescriptorSetAllocateInfo*          pAllocateInfo,
    VkDescriptorSet*                            pDescriptorSets)
{
   LVP_FROM_HANDLE(lvp_device, device, _device);
   LVP_FROM_HANDLE(lvp_descriptor_pool, pool, pAllocateInfo->descriptorPool);
   VkResult result = VK_SUCCESS;
   struct lvp_descriptor_set *set;
   uint32_t i;

   for (i = 0; i < pAllocateInfo->descriptorSetCount; i++) {
      LVP_FROM_HANDLE(lvp_descriptor_set_layout, layout,
                      pAllocateInfo->pSetLayouts[i]);

      result = lvp_descriptor_set_create(device, layout, &set);
      if (result != VK_SUCCESS)
         break;

      list_addtail(&set->link, &pool->sets);
      pDescriptorSets[i] = lvp_descriptor_set_to_handle(set);
   }

   if (result != VK_SUCCESS)
      lvp_FreeDescriptorSets(_device, pAllocateInfo->descriptorPool,
                             i, pDescriptorSets);

   return result;
}

VKAPI_ATTR VkResult VKAPI_CALL lvp_FreeDescriptorSets(
    VkDevice                                    _device,
    VkDescriptorPool                            descriptorPool,
    uint32_t                                    count,
    const VkDescriptorSet*                      pDescriptorSets)
{
   LVP_FROM_HANDLE(lvp_device, device, _device);
   for (uint32_t i = 0; i < count; i++) {
      LVP_FROM_HANDLE(lvp_descriptor_set, set, pDescriptorSets[i]);

      if (!set)
         continue;
      list_del(&set->link);
      lvp_descriptor_set_destroy(device, set);
   }
   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL lvp_UpdateDescriptorSets(
    VkDevice                                    _device,
    uint32_t                                    descriptorWriteCount,
    const VkWriteDescriptorSet*                 pDescriptorWrites,
    uint32_t                                    descriptorCopyCount,
    const VkCopyDescriptorSet*                  pDescriptorCopies)
{
   LVP_FROM_HANDLE(lvp_device, device, _device);

   for (uint32_t i = 0; i < descriptorWriteCount; i++) {
      const VkWriteDescriptorSet *write = &pDescriptorWrites[i];
      LVP_FROM_HANDLE(lvp_descriptor_set, set, write->dstSet);
      const struct lvp_descriptor_set_binding_layout *bind_layout =
         &set->layout->binding[write->dstBinding];

      if (write->descriptorType == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK) {
         const VkWriteDescriptorSetInlineUniformBlock *uniform_data =
            vk_find_struct_const(write->pNext, WRITE_DESCRIPTOR_SET_INLINE_UNIFORM_BLOCK);
         assert(uniform_data);
         memcpy((uint8_t *)set->map + bind_layout->uniform_block_offset + write->dstArrayElement, uniform_data->pData, uniform_data->dataSize);
         continue;
      }

      struct lp_descriptor *desc = set->map;
      desc += bind_layout->descriptor_index + (write->dstArrayElement * bind_layout->stride);

      switch (write->descriptorType) {
      case VK_DESCRIPTOR_TYPE_SAMPLER:
         if (!bind_layout->immutable_samplers) {
            for (uint32_t j = 0; j < write->descriptorCount; j++) {
               LVP_FROM_HANDLE(lvp_sampler, sampler, write->pImageInfo[j].sampler);
               uint32_t didx = j * bind_layout->stride;

               for (unsigned k = 0; k < bind_layout->stride; k++) {
                  desc[didx + k].sampler = sampler->desc.sampler;
                  desc[didx + k].texture.sampler_index = sampler->desc.texture.sampler_index;
               }
            }
         }
         break;

      case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
         for (uint32_t j = 0; j < write->descriptorCount; j++) {
            LVP_FROM_HANDLE(lvp_image_view, iview,
                            write->pImageInfo[j].imageView);
            uint32_t didx = j * bind_layout->stride;
            if (iview) {
               unsigned plane_count = iview->plane_count;

               for (unsigned p = 0; p < plane_count; p++) {
                  lp_jit_texture_from_pipe(&desc[didx + p].texture, iview->planes[p].sv);
                  desc[didx + p].functions = iview->planes[p].texture_handle->functions;
               }

               if (!bind_layout->immutable_samplers) {
                  LVP_FROM_HANDLE(lvp_sampler, sampler,
                                  write->pImageInfo[j].sampler);

                  for (unsigned p = 0; p < plane_count; p++) {
                     desc[didx + p].sampler = sampler->desc.sampler;
                     desc[didx + p].texture.sampler_index = sampler->desc.texture.sampler_index;
                  }
               }
            } else {
               for (unsigned k = 0; k < bind_layout->stride; k++) {
                  desc[didx + k].functions = device->null_texture_handle->functions;
                  desc[didx + k].texture.sampler_index = 0;
               }
            }
         }
         break;

      case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
         for (uint32_t j = 0; j < write->descriptorCount; j++) {
            LVP_FROM_HANDLE(lvp_image_view, iview,
                            write->pImageInfo[j].imageView);
            uint32_t didx = j * bind_layout->stride;
            if (iview) {
               unsigned plane_count = iview->plane_count;

               for (unsigned p = 0; p < plane_count; p++) {
                  lp_jit_texture_from_pipe(&desc[didx + p].texture, iview->planes[p].sv);
                  desc[didx + p].functions = iview->planes[p].texture_handle->functions;
               }
            } else {
               for (unsigned k = 0; k < bind_layout->stride; k++) {
                  desc[didx + k].functions = device->null_texture_handle->functions;
                  desc[didx + k].texture.sampler_index = 0;
               }
            }
         }
         break;
      case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
      case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
         for (uint32_t j = 0; j < write->descriptorCount; j++) {
            LVP_FROM_HANDLE(lvp_image_view, iview,
                            write->pImageInfo[j].imageView);
            uint32_t didx = j * bind_layout->stride;
            if (iview) {
               unsigned plane_count = iview->plane_count;

               for (unsigned p = 0; p < plane_count; p++) {
                  lp_jit_image_from_pipe(&desc[didx + p].image, &iview->planes[p].iv);
                  desc[didx + p].functions = iview->planes[p].image_handle->functions;
               }
            } else {
               for (unsigned k = 0; k < bind_layout->stride; k++)
                  desc[didx + k].functions = device->null_image_handle->functions;
            }
         }
         break;

      case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
         for (uint32_t j = 0; j < write->descriptorCount; j++) {
            LVP_FROM_HANDLE(lvp_buffer_view, bview,
                            write->pTexelBufferView[j]);
            assert(bind_layout->stride == 1);
            if (bview) {
               lp_jit_texture_from_pipe(&desc[j].texture, bview->sv);
               desc[j].functions = bview->texture_handle->functions;
            } else {
               desc[j].functions = device->null_texture_handle->functions;
               desc[j].texture.sampler_index = 0;
            }
         }
         break;

      case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
         for (uint32_t j = 0; j < write->descriptorCount; j++) {
            LVP_FROM_HANDLE(lvp_buffer_view, bview,
                            write->pTexelBufferView[j]);
            assert(bind_layout->stride == 1);
            if (bview) {
               lp_jit_image_from_pipe(&desc[j].image, &bview->iv);
               desc[j].functions = bview->image_handle->functions;
            } else {
               desc[j].functions = device->null_image_handle->functions;
            }
         }
         break;

      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
         for (uint32_t j = 0; j < write->descriptorCount; j++) {
            LVP_FROM_HANDLE(lvp_buffer, buffer, write->pBufferInfo[j].buffer);
            assert(bind_layout->stride == 1);
            if (buffer) {
               struct pipe_constant_buffer ubo = {
                  .buffer = buffer->bo,
                  .buffer_offset = write->pBufferInfo[j].offset,
                  .buffer_size = write->pBufferInfo[j].range,
               };

               if (write->pBufferInfo[j].range == VK_WHOLE_SIZE)
                  ubo.buffer_size = buffer->bo->width0 - ubo.buffer_offset;

               lp_jit_buffer_from_pipe_const(&desc[j].buffer, &ubo, device->pscreen);
            } else {
               lp_jit_buffer_from_pipe_const(&desc[j].buffer, &((struct pipe_constant_buffer){0}), device->pscreen);
            }
         }
         break;

      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
         for (uint32_t j = 0; j < write->descriptorCount; j++) {
            LVP_FROM_HANDLE(lvp_buffer, buffer, write->pBufferInfo[j].buffer);
            assert(bind_layout->stride == 1);
            if (buffer) {
               struct pipe_shader_buffer ubo = {
                  .buffer = buffer->bo,
                  .buffer_offset = write->pBufferInfo[j].offset,
                  .buffer_size = write->pBufferInfo[j].range,
               };

               if (write->pBufferInfo[j].range == VK_WHOLE_SIZE)
                  ubo.buffer_size = buffer->bo->width0 - ubo.buffer_offset;

               lp_jit_buffer_from_pipe(&desc[j].buffer, &ubo);
            } else {
               lp_jit_buffer_from_pipe(&desc[j].buffer, &((struct pipe_shader_buffer){0}));
            }
         }
         break;

      default:
         break;
      }
   }

   for (uint32_t i = 0; i < descriptorCopyCount; i++) {
      const VkCopyDescriptorSet *copy = &pDescriptorCopies[i];
      LVP_FROM_HANDLE(lvp_descriptor_set, src, copy->srcSet);
      LVP_FROM_HANDLE(lvp_descriptor_set, dst, copy->dstSet);

      const struct lvp_descriptor_set_binding_layout *src_layout =
         &src->layout->binding[copy->srcBinding];
      struct lp_descriptor *src_desc = src->map;
      src_desc += src_layout->descriptor_index;

      const struct lvp_descriptor_set_binding_layout *dst_layout =
         &dst->layout->binding[copy->dstBinding];
      struct lp_descriptor *dst_desc = dst->map;
      dst_desc += dst_layout->descriptor_index;

      if (src_layout->type == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK) {
         memcpy((uint8_t *)dst->map + dst_layout->uniform_block_offset + copy->dstArrayElement,
                (uint8_t *)src->map + src_layout->uniform_block_offset + copy->srcArrayElement,
                copy->descriptorCount);
      } else {
         src_desc += copy->srcArrayElement;
         dst_desc += copy->dstArrayElement;

         for (uint32_t j = 0; j < copy->descriptorCount; j++)
            dst_desc[j] = src_desc[j];
      }
   }
}

VKAPI_ATTR VkResult VKAPI_CALL lvp_CreateDescriptorPool(
    VkDevice                                    _device,
    const VkDescriptorPoolCreateInfo*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorPool*                           pDescriptorPool)
{
   LVP_FROM_HANDLE(lvp_device, device, _device);
   struct lvp_descriptor_pool *pool;
   size_t size = sizeof(struct lvp_descriptor_pool);
   pool = vk_zalloc2(&device->vk.alloc, pAllocator, size, 8,
                     VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (!pool)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   vk_object_base_init(&device->vk, &pool->base,
                       VK_OBJECT_TYPE_DESCRIPTOR_POOL);
   pool->flags = pCreateInfo->flags;
   list_inithead(&pool->sets);
   *pDescriptorPool = lvp_descriptor_pool_to_handle(pool);
   return VK_SUCCESS;
}

static void lvp_reset_descriptor_pool(struct lvp_device *device,
                                      struct lvp_descriptor_pool *pool)
{
   struct lvp_descriptor_set *set, *tmp;
   LIST_FOR_EACH_ENTRY_SAFE(set, tmp, &pool->sets, link) {
      list_del(&set->link);
      lvp_descriptor_set_destroy(device, set);
   }
}

VKAPI_ATTR void VKAPI_CALL lvp_DestroyDescriptorPool(
    VkDevice                                    _device,
    VkDescriptorPool                            _pool,
    const VkAllocationCallbacks*                pAllocator)
{
   LVP_FROM_HANDLE(lvp_device, device, _device);
   LVP_FROM_HANDLE(lvp_descriptor_pool, pool, _pool);

   if (!_pool)
      return;

   lvp_reset_descriptor_pool(device, pool);
   vk_object_base_finish(&pool->base);
   vk_free2(&device->vk.alloc, pAllocator, pool);
}

VKAPI_ATTR VkResult VKAPI_CALL lvp_ResetDescriptorPool(
    VkDevice                                    _device,
    VkDescriptorPool                            _pool,
    VkDescriptorPoolResetFlags                  flags)
{
   LVP_FROM_HANDLE(lvp_device, device, _device);
   LVP_FROM_HANDLE(lvp_descriptor_pool, pool, _pool);

   lvp_reset_descriptor_pool(device, pool);
   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL lvp_GetDescriptorSetLayoutSupport(VkDevice device,
                                       const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                       VkDescriptorSetLayoutSupport* pSupport)
{
   const VkDescriptorSetLayoutBindingFlagsCreateInfo *variable_flags =
      vk_find_struct_const(pCreateInfo->pNext, DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO);
   VkDescriptorSetVariableDescriptorCountLayoutSupport *variable_count =
      vk_find_struct(pSupport->pNext, DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_LAYOUT_SUPPORT);
   if (variable_count) {
      variable_count->maxVariableDescriptorCount = 0;
      if (variable_flags) {
         for (unsigned i = 0; i < variable_flags->bindingCount; i++) {
            if (variable_flags->pBindingFlags[i] & VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT)
               variable_count->maxVariableDescriptorCount = MAX_DESCRIPTORS;
         }
      }
   }
   pSupport->supported = true;
}

VKAPI_ATTR VkResult VKAPI_CALL lvp_CreateDescriptorUpdateTemplate(VkDevice _device,
                                            const VkDescriptorUpdateTemplateCreateInfo *pCreateInfo,
                                            const VkAllocationCallbacks *pAllocator,
                                            VkDescriptorUpdateTemplate *pDescriptorUpdateTemplate)
{
   LVP_FROM_HANDLE(lvp_device, device, _device);
   const uint32_t entry_count = pCreateInfo->descriptorUpdateEntryCount;
   const size_t size = sizeof(struct lvp_descriptor_update_template) +
      sizeof(VkDescriptorUpdateTemplateEntry) * entry_count;

   struct lvp_descriptor_update_template *templ;

   templ = vk_alloc(&device->vk.alloc, size, 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (!templ)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   vk_object_base_init(&device->vk, &templ->base,
                       VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE);

   templ->ref_cnt = 1;
   templ->type = pCreateInfo->templateType;
   templ->bind_point = pCreateInfo->pipelineBindPoint;
   templ->set = pCreateInfo->set;
   /* This parameter is ignored if templateType is not VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR */
   if (pCreateInfo->templateType == VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR)
      templ->pipeline_layout = lvp_pipeline_layout_from_handle(pCreateInfo->pipelineLayout);
   else
      templ->pipeline_layout = NULL;
   templ->entry_count = entry_count;

   VkDescriptorUpdateTemplateEntry *entries = (VkDescriptorUpdateTemplateEntry *)(templ + 1);
   for (unsigned i = 0; i < entry_count; i++) {
      entries[i] = pCreateInfo->pDescriptorUpdateEntries[i];
   }

   *pDescriptorUpdateTemplate = lvp_descriptor_update_template_to_handle(templ);
   return VK_SUCCESS;
}

void
lvp_descriptor_template_destroy(struct lvp_device *device, struct lvp_descriptor_update_template *templ)
{
   if (!templ)
      return;

   vk_object_base_finish(&templ->base);
   vk_free(&device->vk.alloc, templ);
}

VKAPI_ATTR void VKAPI_CALL lvp_DestroyDescriptorUpdateTemplate(VkDevice _device,
                                         VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                         const VkAllocationCallbacks *pAllocator)
{
   LVP_FROM_HANDLE(lvp_device, device, _device);
   LVP_FROM_HANDLE(lvp_descriptor_update_template, templ, descriptorUpdateTemplate);
   lvp_descriptor_template_templ_unref(device, templ);
}

uint32_t
lvp_descriptor_update_template_entry_size(VkDescriptorType type)
{
   switch (type) {
   case VK_DESCRIPTOR_TYPE_SAMPLER:
   case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
   case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
   case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
   case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
      return sizeof(VkDescriptorImageInfo);
   case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
   case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
      return sizeof(VkBufferView);
   case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
   case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
   case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
   case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
   default:
      return sizeof(VkDescriptorBufferInfo);
   }
}

void
lvp_descriptor_set_update_with_template(VkDevice _device, VkDescriptorSet descriptorSet,
                                        VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                        const void *pData, bool push)
{
   LVP_FROM_HANDLE(lvp_device, device, _device);
   LVP_FROM_HANDLE(lvp_descriptor_set, set, descriptorSet);
   LVP_FROM_HANDLE(lvp_descriptor_update_template, templ, descriptorUpdateTemplate);
   uint32_t i, j;

   const uint8_t *pSrc = pData;

   for (i = 0; i < templ->entry_count; ++i) {
      VkDescriptorUpdateTemplateEntry *entry = &templ->entry[i];

      if (!push)
         pSrc = ((const uint8_t *) pData) + entry->offset;

      const struct lvp_descriptor_set_binding_layout *bind_layout =
         &set->layout->binding[entry->dstBinding];

      if (entry->descriptorType == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK) {
         memcpy((uint8_t *)set->map + bind_layout->uniform_block_offset + entry->dstArrayElement, pSrc, entry->descriptorCount);
         continue;
      }

      struct lp_descriptor *desc = set->map;
      desc += bind_layout->descriptor_index;

      for (j = 0; j < entry->descriptorCount; ++j) {
         unsigned idx = j + entry->dstArrayElement;

         idx *= bind_layout->stride;
         switch (entry->descriptorType) {
         case VK_DESCRIPTOR_TYPE_SAMPLER: {
            LVP_FROM_HANDLE(lvp_sampler, sampler,
                            *(VkSampler *)pSrc);

            for (unsigned k = 0; k < bind_layout->stride; k++) {
               desc[idx + k].sampler = sampler->desc.sampler;
               desc[idx + k].texture.sampler_index = sampler->desc.texture.sampler_index;
            }
            break;
         }
         case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: {
            VkDescriptorImageInfo *info = (VkDescriptorImageInfo *)pSrc;
            LVP_FROM_HANDLE(lvp_image_view, iview, info->imageView);

            if (iview) {
               for (unsigned p = 0; p < iview->plane_count; p++) {
                  lp_jit_texture_from_pipe(&desc[idx + p].texture, iview->planes[p].sv);
                  desc[idx + p].functions = iview->planes[p].texture_handle->functions;
               }

               if (!bind_layout->immutable_samplers) {
                  LVP_FROM_HANDLE(lvp_sampler, sampler, info->sampler);

                  for (unsigned p = 0; p < iview->plane_count; p++) {
                     desc[idx + p].sampler = sampler->desc.sampler;
                     desc[idx + p].texture.sampler_index = sampler->desc.texture.sampler_index;
                  }
               }
            } else {
               for (unsigned k = 0; k < bind_layout->stride; k++) {
                  desc[idx + k].functions = device->null_texture_handle->functions;
                  desc[idx + k].texture.sampler_index = 0;
               }
            }
            break;
         }
         case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE: {
            VkDescriptorImageInfo *info = (VkDescriptorImageInfo *)pSrc;
            LVP_FROM_HANDLE(lvp_image_view, iview, info->imageView);

            if (iview) {
               for (unsigned p = 0; p < iview->plane_count; p++) {
                  lp_jit_texture_from_pipe(&desc[idx + p].texture, iview->planes[p].sv);
                  desc[idx + p].functions = iview->planes[p].texture_handle->functions;
               }
            } else {
               for (unsigned k = 0; k < bind_layout->stride; k++) {
                  desc[idx + k].functions = device->null_texture_handle->functions;
                  desc[idx + k].texture.sampler_index = 0;
               }
            }
            break;
         }
         case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
         case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT: {
            LVP_FROM_HANDLE(lvp_image_view, iview,
                            ((VkDescriptorImageInfo *)pSrc)->imageView);

            if (iview) {
               for (unsigned p = 0; p < iview->plane_count; p++) {
                  lp_jit_image_from_pipe(&desc[idx + p].image, &iview->planes[p].iv);
                  desc[idx + p].functions = iview->planes[p].image_handle->functions;
               }
            } else {
               for (unsigned k = 0; k < bind_layout->stride; k++)
                  desc[idx + k].functions = device->null_image_handle->functions;
            }
            break;
         }
         case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER: {
            LVP_FROM_HANDLE(lvp_buffer_view, bview,
                            *(VkBufferView *)pSrc);
            assert(bind_layout->stride == 1);
            if (bview) {
               lp_jit_texture_from_pipe(&desc[idx].texture, bview->sv);
               desc[idx].functions = bview->texture_handle->functions;
            } else {
               desc[j].functions = device->null_texture_handle->functions;
               desc[j].texture.sampler_index = 0;
            }
            break;
         }
         case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER: {
            LVP_FROM_HANDLE(lvp_buffer_view, bview,
                            *(VkBufferView *)pSrc);
            assert(bind_layout->stride == 1);
            if (bview) {
               lp_jit_image_from_pipe(&desc[idx].image, &bview->iv);
               desc[idx].functions = bview->image_handle->functions;
            } else {
               desc[idx].functions = device->null_image_handle->functions;
            }
            break;
         }

         case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
         case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC: {
            VkDescriptorBufferInfo *info = (VkDescriptorBufferInfo *)pSrc;
            LVP_FROM_HANDLE(lvp_buffer, buffer, info->buffer);
            assert(bind_layout->stride == 1);
            if (buffer) {
               struct pipe_constant_buffer ubo = {
                  .buffer = buffer->bo,
                  .buffer_offset = info->offset,
                  .buffer_size = info->range,
               };

               if (info->range == VK_WHOLE_SIZE)
                  ubo.buffer_size = buffer->bo->width0 - ubo.buffer_offset;

               lp_jit_buffer_from_pipe_const(&desc[idx].buffer, &ubo, device->pscreen);
            } else {
               lp_jit_buffer_from_pipe_const(&desc[idx].buffer, &((struct pipe_constant_buffer){0}), device->pscreen);
            }
            break;
         }

         case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
         case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: {
            VkDescriptorBufferInfo *info = (VkDescriptorBufferInfo *)pSrc;
            LVP_FROM_HANDLE(lvp_buffer, buffer, info->buffer);
            assert(bind_layout->stride == 1);

            if (buffer) {
               struct pipe_shader_buffer ubo = {
                  .buffer = buffer->bo,
                  .buffer_offset = info->offset,
                  .buffer_size = info->range,
               };

               if (info->range == VK_WHOLE_SIZE)
                  ubo.buffer_size = buffer->bo->width0 - ubo.buffer_offset;

               lp_jit_buffer_from_pipe(&desc[idx].buffer, &ubo);
            } else {
               lp_jit_buffer_from_pipe(&desc[idx].buffer, &((struct pipe_shader_buffer){0}));
            }
            break;
         }
         default:
            break;
         }

         if (push)
            pSrc += lvp_descriptor_update_template_entry_size(entry->descriptorType);
         else
            pSrc += entry->stride;
      }
   }
}

VKAPI_ATTR void VKAPI_CALL
lvp_UpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet,
                                    VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                    const void *pData)
{
   lvp_descriptor_set_update_with_template(device, descriptorSet, descriptorUpdateTemplate, pData, false);
}

VKAPI_ATTR void VKAPI_CALL lvp_GetDescriptorSetLayoutSizeEXT(
    VkDevice                                    _device,
    VkDescriptorSetLayout                       _layout,
    VkDeviceSize*                               pSize)
{
   LVP_FROM_HANDLE(lvp_descriptor_set_layout, layout, _layout);

   *pSize = layout->size * sizeof(struct lp_descriptor);

   for (unsigned i = 0; i < layout->binding_count; i++)
      *pSize += layout->binding[i].uniform_block_size;
}

VKAPI_ATTR void VKAPI_CALL lvp_GetDescriptorSetLayoutBindingOffsetEXT(
    VkDevice                                    _device,
    VkDescriptorSetLayout                       _layout,
    uint32_t                                    binding,
    VkDeviceSize*                               pOffset)
{
   LVP_FROM_HANDLE(lvp_descriptor_set_layout, layout, _layout);
   assert(binding < layout->binding_count);

   const struct lvp_descriptor_set_binding_layout *bind_layout = &layout->binding[binding];
   if (bind_layout->type == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK)
      *pOffset = bind_layout->uniform_block_offset;
   else
      *pOffset = bind_layout->descriptor_index * sizeof(struct lp_descriptor);
}

VKAPI_ATTR void VKAPI_CALL lvp_GetDescriptorEXT(
    VkDevice                                        _device,
    const VkDescriptorGetInfoEXT*                   pCreateInfo,
    size_t                                          size,
    void*                                           pDescriptor)
{
   LVP_FROM_HANDLE(lvp_device, device, _device);

   struct lp_descriptor *desc = pDescriptor;

   struct pipe_sampler_state sampler = {
      .seamless_cube_map = 1,
      .max_lod = 0.25,
   };

   switch (pCreateInfo->type) {
   case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK: {
      unreachable("this is a spec violation");
      break;
   }
   case VK_DESCRIPTOR_TYPE_SAMPLER: {
      if (pCreateInfo->data.pSampler) {
         LVP_FROM_HANDLE(lvp_sampler, sampler, pCreateInfo->data.pSampler[0]);
         desc->sampler = sampler->desc.sampler;
         desc->texture.sampler_index = sampler->desc.texture.sampler_index;
      } else {
         lp_jit_sampler_from_pipe(&desc->sampler, &sampler);
         desc->texture.sampler_index = 0;
      }
      break;
   }

   case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: {
      const VkDescriptorImageInfo *info = pCreateInfo->data.pCombinedImageSampler;
      if (info && info->imageView) {
         LVP_FROM_HANDLE(lvp_image_view, iview, info->imageView);

         lp_jit_texture_from_pipe(&desc->texture, iview->planes[0].sv);
         desc->functions = iview->planes[0].texture_handle->functions;

         if (info->sampler) {
            LVP_FROM_HANDLE(lvp_sampler, sampler, info->sampler);
            desc->sampler = sampler->desc.sampler;
            desc->texture.sampler_index = sampler->desc.texture.sampler_index;
         } else {
            lp_jit_sampler_from_pipe(&desc->sampler, &sampler);
            desc->texture.sampler_index = 0;
         }
      } else {
         desc->functions = device->null_texture_handle->functions;
         desc->texture.sampler_index = 0;
      }

      break;
   }

   case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE: {
      if (pCreateInfo->data.pSampledImage && pCreateInfo->data.pSampledImage->imageView) {
         LVP_FROM_HANDLE(lvp_image_view, iview, pCreateInfo->data.pSampledImage->imageView);
         lp_jit_texture_from_pipe(&desc->texture, iview->planes[0].sv);
         desc->functions = iview->planes[0].texture_handle->functions;
      } else {
         desc->functions = device->null_texture_handle->functions;
         desc->texture.sampler_index = 0;
      }
      break;
   }

   /* technically these use different pointers, but it's a union, so they're all the same */
   case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
   case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT: {
      if (pCreateInfo->data.pStorageImage && pCreateInfo->data.pStorageImage->imageView) {
         LVP_FROM_HANDLE(lvp_image_view, iview, pCreateInfo->data.pStorageImage->imageView);
         lp_jit_image_from_pipe(&desc->image, &iview->planes[0].iv);
         desc->functions = iview->planes[0].image_handle->functions;
      } else {
         desc->functions = device->null_image_handle->functions;
      }
      break;
   }
   case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER: {
      const VkDescriptorAddressInfoEXT *bda = pCreateInfo->data.pUniformTexelBuffer;
      if (bda && bda->address) {
         enum pipe_format pformat = vk_format_to_pipe_format(bda->format);
         lp_jit_texture_buffer_from_bda(&desc->texture, (void*)(uintptr_t)bda->address, bda->range, pformat);
         desc->functions = get_texture_handle_bda(device, bda, pformat).functions;
      } else {
         desc->functions = device->null_texture_handle->functions;
         desc->texture.sampler_index = 0;
      }
      break;
   }
   case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER: {
      const VkDescriptorAddressInfoEXT *bda = pCreateInfo->data.pStorageTexelBuffer;
      if (bda && bda->address) {
         enum pipe_format pformat = vk_format_to_pipe_format(bda->format);
         lp_jit_image_buffer_from_bda(&desc->image, (void *)(uintptr_t)bda->address, bda->range, pformat);
         desc->functions = get_image_handle_bda(device, bda, pformat).functions;
      } else {
         desc->functions = device->null_image_handle->functions;
      }
      break;
   }
   case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER: {
      const VkDescriptorAddressInfoEXT *bda = pCreateInfo->data.pUniformBuffer;
      if (bda && bda->address) {
         struct pipe_constant_buffer ubo = {
            .user_buffer = (void *)(uintptr_t)bda->address,
            .buffer_size = bda->range,
         };

         lp_jit_buffer_from_pipe_const(&desc->buffer, &ubo, device->pscreen);
      } else {
         lp_jit_buffer_from_pipe_const(&desc->buffer, &((struct pipe_constant_buffer){0}), device->pscreen);
      }
      break;
   }
   case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER: {
      const VkDescriptorAddressInfoEXT *bda = pCreateInfo->data.pStorageBuffer;
      if (bda && bda->address) {
         lp_jit_buffer_from_bda(&desc->buffer, (void *)(uintptr_t)bda->address, bda->range);
      } else {
         lp_jit_buffer_from_pipe(&desc->buffer, &((struct pipe_shader_buffer){0}));
      }
      break;
   }
   default:
      break;
   }
}
