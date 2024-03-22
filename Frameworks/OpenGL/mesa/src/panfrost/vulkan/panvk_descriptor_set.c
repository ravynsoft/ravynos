/*
 * Copyright © 2021 Collabora Ltd.
 *
 * Derived from:
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
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

#include <assert.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include "util/mesa-sha1.h"
#include "vk_descriptors.h"
#include "vk_util.h"

#include "pan_bo.h"

/* FIXME: make sure those values are correct */
#define PANVK_MAX_TEXTURES (1 << 16)
#define PANVK_MAX_IMAGES   (1 << 8)
#define PANVK_MAX_SAMPLERS (1 << 16)
#define PANVK_MAX_UBOS     255

void
panvk_GetDescriptorSetLayoutSupport(
   VkDevice _device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
   VkDescriptorSetLayoutSupport *pSupport)
{
   VK_FROM_HANDLE(panvk_device, device, _device);

   pSupport->supported = false;

   VkDescriptorSetLayoutBinding *bindings;
   VkResult result = vk_create_sorted_bindings(
      pCreateInfo->pBindings, pCreateInfo->bindingCount, &bindings);
   if (result != VK_SUCCESS) {
      vk_error(device, result);
      return;
   }

   unsigned sampler_idx = 0, tex_idx = 0, ubo_idx = 0;
   unsigned img_idx = 0;
   UNUSED unsigned dynoffset_idx = 0;

   for (unsigned i = 0; i < pCreateInfo->bindingCount; i++) {
      const VkDescriptorSetLayoutBinding *binding = &bindings[i];

      switch (binding->descriptorType) {
      case VK_DESCRIPTOR_TYPE_SAMPLER:
         sampler_idx += binding->descriptorCount;
         break;
      case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
         sampler_idx += binding->descriptorCount;
         tex_idx += binding->descriptorCount;
         break;
      case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
      case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
      case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
         tex_idx += binding->descriptorCount;
         break;
      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
         dynoffset_idx += binding->descriptorCount;
         FALLTHROUGH;
      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
         ubo_idx += binding->descriptorCount;
         break;
      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
         dynoffset_idx += binding->descriptorCount;
         FALLTHROUGH;
      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
         break;
      case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
      case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
         img_idx += binding->descriptorCount;
         break;
      default:
         unreachable("Invalid descriptor type");
      }
   }

   /* The maximum values apply to all sets attached to a pipeline since all
    * sets descriptors have to be merged in a single array.
    */
   if (tex_idx > PANVK_MAX_TEXTURES / MAX_SETS ||
       sampler_idx > PANVK_MAX_SAMPLERS / MAX_SETS ||
       ubo_idx > PANVK_MAX_UBOS / MAX_SETS ||
       img_idx > PANVK_MAX_IMAGES / MAX_SETS)
      return;

   pSupport->supported = true;
}

/*
 * Pipeline layouts.  These have nothing to do with the pipeline.  They are
 * just multiple descriptor set layouts pasted together.
 */

VkResult
panvk_CreatePipelineLayout(VkDevice _device,
                           const VkPipelineLayoutCreateInfo *pCreateInfo,
                           const VkAllocationCallbacks *pAllocator,
                           VkPipelineLayout *pPipelineLayout)
{
   VK_FROM_HANDLE(panvk_device, device, _device);
   struct panvk_pipeline_layout *layout;
   struct mesa_sha1 ctx;

   layout =
      vk_pipeline_layout_zalloc(&device->vk, sizeof(*layout), pCreateInfo);
   if (layout == NULL)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   _mesa_sha1_init(&ctx);

   unsigned sampler_idx = 0, tex_idx = 0, ubo_idx = 0;
   unsigned dyn_ubo_idx = 0, dyn_ssbo_idx = 0, img_idx = 0;
   for (unsigned set = 0; set < pCreateInfo->setLayoutCount; set++) {
      const struct panvk_descriptor_set_layout *set_layout =
         vk_to_panvk_descriptor_set_layout(layout->vk.set_layouts[set]);

      layout->sets[set].sampler_offset = sampler_idx;
      layout->sets[set].tex_offset = tex_idx;
      layout->sets[set].ubo_offset = ubo_idx;
      layout->sets[set].dyn_ubo_offset = dyn_ubo_idx;
      layout->sets[set].dyn_ssbo_offset = dyn_ssbo_idx;
      layout->sets[set].img_offset = img_idx;
      sampler_idx += set_layout->num_samplers;
      tex_idx += set_layout->num_textures;
      ubo_idx += set_layout->num_ubos;
      dyn_ubo_idx += set_layout->num_dyn_ubos;
      dyn_ssbo_idx += set_layout->num_dyn_ssbos;
      img_idx += set_layout->num_imgs;

      for (unsigned b = 0; b < set_layout->binding_count; b++) {
         const struct panvk_descriptor_set_binding_layout *binding_layout =
            &set_layout->bindings[b];

         if (binding_layout->immutable_samplers) {
            for (unsigned s = 0; s < binding_layout->array_size; s++) {
               struct panvk_sampler *sampler =
                  binding_layout->immutable_samplers[s];

               _mesa_sha1_update(&ctx, &sampler->desc, sizeof(sampler->desc));
            }
         }
         _mesa_sha1_update(&ctx, &binding_layout->type,
                           sizeof(binding_layout->type));
         _mesa_sha1_update(&ctx, &binding_layout->array_size,
                           sizeof(binding_layout->array_size));
         _mesa_sha1_update(&ctx, &binding_layout->shader_stages,
                           sizeof(binding_layout->shader_stages));
      }
   }

   for (unsigned range = 0; range < pCreateInfo->pushConstantRangeCount;
        range++) {
      layout->push_constants.size =
         MAX2(pCreateInfo->pPushConstantRanges[range].offset +
                 pCreateInfo->pPushConstantRanges[range].size,
              layout->push_constants.size);
   }

   layout->num_samplers = sampler_idx;
   layout->num_textures = tex_idx;
   layout->num_ubos = ubo_idx;
   layout->num_dyn_ubos = dyn_ubo_idx;
   layout->num_dyn_ssbos = dyn_ssbo_idx;
   layout->num_imgs = img_idx;

   /* Some NIR texture operations don't require a sampler, but Bifrost/Midgard
    * ones always expect one. Add a dummy sampler to deal with this limitation.
    */
   if (layout->num_textures) {
      layout->num_samplers++;
      for (unsigned set = 0; set < pCreateInfo->setLayoutCount; set++)
         layout->sets[set].sampler_offset++;
   }

   _mesa_sha1_final(&ctx, layout->sha1);

   *pPipelineLayout = panvk_pipeline_layout_to_handle(layout);
   return VK_SUCCESS;
}

VkResult
panvk_CreateDescriptorPool(VkDevice _device,
                           const VkDescriptorPoolCreateInfo *pCreateInfo,
                           const VkAllocationCallbacks *pAllocator,
                           VkDescriptorPool *pDescriptorPool)
{
   VK_FROM_HANDLE(panvk_device, device, _device);
   struct panvk_descriptor_pool *pool;

   pool = vk_object_zalloc(&device->vk, pAllocator,
                           sizeof(struct panvk_descriptor_pool),
                           VK_OBJECT_TYPE_DESCRIPTOR_POOL);
   if (!pool)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   pool->max.sets = pCreateInfo->maxSets;

   for (unsigned i = 0; i < pCreateInfo->poolSizeCount; ++i) {
      unsigned desc_count = pCreateInfo->pPoolSizes[i].descriptorCount;

      switch (pCreateInfo->pPoolSizes[i].type) {
      case VK_DESCRIPTOR_TYPE_SAMPLER:
         pool->max.samplers += desc_count;
         break;
      case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
         pool->max.combined_image_samplers += desc_count;
         break;
      case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
         pool->max.sampled_images += desc_count;
         break;
      case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
         pool->max.storage_images += desc_count;
         break;
      case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
         pool->max.uniform_texel_bufs += desc_count;
         break;
      case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
         pool->max.storage_texel_bufs += desc_count;
         break;
      case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
         pool->max.input_attachments += desc_count;
         break;
      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
         pool->max.uniform_bufs += desc_count;
         break;
      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
         pool->max.storage_bufs += desc_count;
         break;
      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
         pool->max.uniform_dyn_bufs += desc_count;
         break;
      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
         pool->max.storage_dyn_bufs += desc_count;
         break;
      default:
         unreachable("Invalid descriptor type");
      }
   }

   *pDescriptorPool = panvk_descriptor_pool_to_handle(pool);
   return VK_SUCCESS;
}

void
panvk_DestroyDescriptorPool(VkDevice _device, VkDescriptorPool _pool,
                            const VkAllocationCallbacks *pAllocator)
{
   VK_FROM_HANDLE(panvk_device, device, _device);
   VK_FROM_HANDLE(panvk_descriptor_pool, pool, _pool);

   if (pool)
      vk_object_free(&device->vk, pAllocator, pool);
}

VkResult
panvk_ResetDescriptorPool(VkDevice _device, VkDescriptorPool _pool,
                          VkDescriptorPoolResetFlags flags)
{
   VK_FROM_HANDLE(panvk_descriptor_pool, pool, _pool);
   memset(&pool->cur, 0, sizeof(pool->cur));
   return VK_SUCCESS;
}

static void
panvk_descriptor_set_destroy(struct panvk_device *device,
                             struct panvk_descriptor_pool *pool,
                             struct panvk_descriptor_set *set)
{
   vk_free(&device->vk.alloc, set->textures);
   vk_free(&device->vk.alloc, set->samplers);
   vk_free(&device->vk.alloc, set->ubos);
   vk_free(&device->vk.alloc, set->dyn_ubos);
   vk_free(&device->vk.alloc, set->dyn_ssbos);
   vk_free(&device->vk.alloc, set->img_fmts);
   vk_free(&device->vk.alloc, set->img_attrib_bufs);
   if (set->desc_bo)
      panfrost_bo_unreference(set->desc_bo);
   vk_object_free(&device->vk, NULL, set);
}

VkResult
panvk_FreeDescriptorSets(VkDevice _device, VkDescriptorPool descriptorPool,
                         uint32_t count, const VkDescriptorSet *pDescriptorSets)
{
   VK_FROM_HANDLE(panvk_device, device, _device);
   VK_FROM_HANDLE(panvk_descriptor_pool, pool, descriptorPool);

   for (unsigned i = 0; i < count; i++) {
      VK_FROM_HANDLE(panvk_descriptor_set, set, pDescriptorSets[i]);

      if (set)
         panvk_descriptor_set_destroy(device, pool, set);
   }
   return VK_SUCCESS;
}

VkResult
panvk_CreateSamplerYcbcrConversion(
   VkDevice device, const VkSamplerYcbcrConversionCreateInfo *pCreateInfo,
   const VkAllocationCallbacks *pAllocator,
   VkSamplerYcbcrConversion *pYcbcrConversion)
{
   panvk_stub();
   return VK_SUCCESS;
}

void
panvk_DestroySamplerYcbcrConversion(VkDevice device,
                                    VkSamplerYcbcrConversion ycbcrConversion,
                                    const VkAllocationCallbacks *pAllocator)
{
   panvk_stub();
}
