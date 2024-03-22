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

#include "genxml/gen_macros.h"

#include "panvk_private.h"

#include <assert.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include "util/mesa-sha1.h"
#include "vk_descriptor_update_template.h"
#include "vk_descriptors.h"
#include "vk_util.h"

#include "pan_bo.h"
#include "panvk_cs.h"

#define PANVK_DESCRIPTOR_ALIGN 8

struct panvk_bview_desc {
   uint32_t elems;
};

static void
panvk_fill_bview_desc(struct panvk_bview_desc *desc,
                      struct panvk_buffer_view *view)
{
   desc->elems = view->elems;
}

struct panvk_image_desc {
   uint16_t width;
   uint16_t height;
   uint16_t depth;
   uint8_t levels;
   uint8_t samples;
};

static void
panvk_fill_image_desc(struct panvk_image_desc *desc,
                      struct panvk_image_view *view)
{
   desc->width = view->vk.extent.width - 1;
   desc->height = view->vk.extent.height - 1;
   desc->depth = view->vk.extent.depth - 1;
   desc->levels = view->vk.level_count;
   desc->samples = view->vk.image->samples;

   /* Stick array layer count after the last valid size component */
   if (view->vk.image->image_type == VK_IMAGE_TYPE_1D)
      desc->height = view->vk.layer_count - 1;
   else if (view->vk.image->image_type == VK_IMAGE_TYPE_2D)
      desc->depth = view->vk.layer_count - 1;
}

VkResult
panvk_per_arch(CreateDescriptorSetLayout)(
   VkDevice _device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
   const VkAllocationCallbacks *pAllocator, VkDescriptorSetLayout *pSetLayout)
{
   VK_FROM_HANDLE(panvk_device, device, _device);
   struct panvk_descriptor_set_layout *set_layout;
   VkDescriptorSetLayoutBinding *bindings = NULL;
   unsigned num_bindings = 0;
   VkResult result;

   if (pCreateInfo->bindingCount) {
      result = vk_create_sorted_bindings(pCreateInfo->pBindings,
                                         pCreateInfo->bindingCount, &bindings);
      if (result != VK_SUCCESS)
         return vk_error(device, result);

      num_bindings = bindings[pCreateInfo->bindingCount - 1].binding + 1;
   }

   unsigned num_immutable_samplers = 0;
   for (unsigned i = 0; i < pCreateInfo->bindingCount; i++) {
      if (bindings[i].pImmutableSamplers)
         num_immutable_samplers += bindings[i].descriptorCount;
   }

   size_t size =
      sizeof(*set_layout) +
      (sizeof(struct panvk_descriptor_set_binding_layout) * num_bindings) +
      (sizeof(struct panvk_sampler *) * num_immutable_samplers);
   set_layout = vk_descriptor_set_layout_zalloc(&device->vk, size);
   if (!set_layout) {
      result = VK_ERROR_OUT_OF_HOST_MEMORY;
      goto err_free_bindings;
   }

   struct panvk_sampler **immutable_samplers =
      (struct panvk_sampler **)((uint8_t *)set_layout + sizeof(*set_layout) +
                                (sizeof(
                                    struct panvk_descriptor_set_binding_layout) *
                                 num_bindings));

   set_layout->binding_count = num_bindings;

   unsigned sampler_idx = 0, tex_idx = 0, ubo_idx = 0;
   unsigned dyn_ubo_idx = 0, dyn_ssbo_idx = 0, img_idx = 0;
   uint32_t desc_ubo_size = 0;

   for (unsigned i = 0; i < pCreateInfo->bindingCount; i++) {
      const VkDescriptorSetLayoutBinding *binding = &bindings[i];
      struct panvk_descriptor_set_binding_layout *binding_layout =
         &set_layout->bindings[binding->binding];

      binding_layout->type = binding->descriptorType;
      binding_layout->array_size = binding->descriptorCount;
      binding_layout->shader_stages = binding->stageFlags;
      binding_layout->desc_ubo_stride = 0;
      if (binding->pImmutableSamplers) {
         binding_layout->immutable_samplers = immutable_samplers;
         immutable_samplers += binding_layout->array_size;
         for (unsigned j = 0; j < binding_layout->array_size; j++) {
            VK_FROM_HANDLE(panvk_sampler, sampler,
                           binding->pImmutableSamplers[j]);
            binding_layout->immutable_samplers[j] = sampler;
         }
      }

      switch (binding_layout->type) {
      case VK_DESCRIPTOR_TYPE_SAMPLER:
         binding_layout->sampler_idx = sampler_idx;
         sampler_idx += binding_layout->array_size;
         break;
      case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
         binding_layout->sampler_idx = sampler_idx;
         binding_layout->tex_idx = tex_idx;
         sampler_idx += binding_layout->array_size;
         tex_idx += binding_layout->array_size;
         binding_layout->desc_ubo_stride = sizeof(struct panvk_image_desc);
         break;
      case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
      case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
         binding_layout->tex_idx = tex_idx;
         tex_idx += binding_layout->array_size;
         binding_layout->desc_ubo_stride = sizeof(struct panvk_image_desc);
         break;
      case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
         binding_layout->tex_idx = tex_idx;
         tex_idx += binding_layout->array_size;
         binding_layout->desc_ubo_stride = sizeof(struct panvk_bview_desc);
         break;
      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
         binding_layout->dyn_ubo_idx = dyn_ubo_idx;
         dyn_ubo_idx += binding_layout->array_size;
         break;
      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
         binding_layout->ubo_idx = ubo_idx;
         ubo_idx += binding_layout->array_size;
         break;
      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
         binding_layout->dyn_ssbo_idx = dyn_ssbo_idx;
         dyn_ssbo_idx += binding_layout->array_size;
         break;
      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
         binding_layout->desc_ubo_stride = sizeof(struct panvk_ssbo_addr);
         break;
      case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
         binding_layout->img_idx = img_idx;
         img_idx += binding_layout->array_size;
         binding_layout->desc_ubo_stride = sizeof(struct panvk_image_desc);
         break;
      case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
         binding_layout->img_idx = img_idx;
         img_idx += binding_layout->array_size;
         binding_layout->desc_ubo_stride = sizeof(struct panvk_bview_desc);
         break;
      default:
         unreachable("Invalid descriptor type");
      }

      desc_ubo_size = ALIGN_POT(desc_ubo_size, PANVK_DESCRIPTOR_ALIGN);
      binding_layout->desc_ubo_offset = desc_ubo_size;
      desc_ubo_size +=
         binding_layout->desc_ubo_stride * binding_layout->array_size;
   }

   set_layout->desc_ubo_size = desc_ubo_size;
   if (desc_ubo_size > 0)
      set_layout->desc_ubo_index = ubo_idx++;

   set_layout->num_samplers = sampler_idx;
   set_layout->num_textures = tex_idx;
   set_layout->num_ubos = ubo_idx;
   set_layout->num_dyn_ubos = dyn_ubo_idx;
   set_layout->num_dyn_ssbos = dyn_ssbo_idx;
   set_layout->num_imgs = img_idx;

   free(bindings);
   *pSetLayout = panvk_descriptor_set_layout_to_handle(set_layout);
   return VK_SUCCESS;

err_free_bindings:
   free(bindings);
   return vk_error(device, result);
}

static void panvk_write_sampler_desc_raw(struct panvk_descriptor_set *set,
                                         uint32_t binding, uint32_t elem,
                                         struct panvk_sampler *sampler);

static VkResult
panvk_per_arch(descriptor_set_create)(
   struct panvk_device *device, struct panvk_descriptor_pool *pool,
   const struct panvk_descriptor_set_layout *layout,
   struct panvk_descriptor_set **out_set)
{
   struct panvk_descriptor_set *set;

   /* TODO: Allocate from the pool! */
   set =
      vk_object_zalloc(&device->vk, NULL, sizeof(struct panvk_descriptor_set),
                       VK_OBJECT_TYPE_DESCRIPTOR_SET);
   if (!set)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   set->layout = layout;

   if (layout->num_ubos) {
      set->ubos = vk_zalloc(&device->vk.alloc,
                            pan_size(UNIFORM_BUFFER) * layout->num_ubos, 8,
                            VK_OBJECT_TYPE_DESCRIPTOR_SET);
      if (!set->ubos)
         goto err_free_set;
   }

   if (layout->num_dyn_ubos) {
      set->dyn_ubos = vk_zalloc(&device->vk.alloc,
                                sizeof(*set->dyn_ubos) * layout->num_dyn_ubos,
                                8, VK_OBJECT_TYPE_DESCRIPTOR_SET);
      if (!set->dyn_ubos)
         goto err_free_set;
   }

   if (layout->num_dyn_ssbos) {
      set->dyn_ssbos = vk_zalloc(
         &device->vk.alloc, sizeof(*set->dyn_ssbos) * layout->num_dyn_ssbos, 8,
         VK_OBJECT_TYPE_DESCRIPTOR_SET);
      if (!set->dyn_ssbos)
         goto err_free_set;
   }

   if (layout->num_samplers) {
      set->samplers =
         vk_zalloc(&device->vk.alloc, pan_size(SAMPLER) * layout->num_samplers,
                   8, VK_OBJECT_TYPE_DESCRIPTOR_SET);
      if (!set->samplers)
         goto err_free_set;
   }

   if (layout->num_textures) {
      set->textures =
         vk_zalloc(&device->vk.alloc, pan_size(TEXTURE) * layout->num_textures,
                   8, VK_OBJECT_TYPE_DESCRIPTOR_SET);
      if (!set->textures)
         goto err_free_set;
   }

   if (layout->num_imgs) {
      set->img_fmts =
         vk_zalloc(&device->vk.alloc, sizeof(*set->img_fmts) * layout->num_imgs,
                   8, VK_OBJECT_TYPE_DESCRIPTOR_SET);
      if (!set->img_fmts)
         goto err_free_set;

      set->img_attrib_bufs = vk_zalloc(
         &device->vk.alloc, pan_size(ATTRIBUTE_BUFFER) * 2 * layout->num_imgs,
         8, VK_OBJECT_TYPE_DESCRIPTOR_SET);
      if (!set->img_attrib_bufs)
         goto err_free_set;
   }

   if (layout->desc_ubo_size) {
      set->desc_bo =
         panfrost_bo_create(&device->physical_device->pdev,
                            layout->desc_ubo_size, 0, "Descriptor set");
      if (!set->desc_bo)
         goto err_free_set;

      struct mali_uniform_buffer_packed *ubos = set->ubos;

      panvk_per_arch(emit_ubo)(set->desc_bo->ptr.gpu, layout->desc_ubo_size,
                               &ubos[layout->desc_ubo_index]);
   }

   for (unsigned i = 0; i < layout->binding_count; i++) {
      if (!layout->bindings[i].immutable_samplers)
         continue;

      for (unsigned j = 0; j < layout->bindings[i].array_size; j++) {
         struct panvk_sampler *sampler =
            layout->bindings[i].immutable_samplers[j];
         panvk_write_sampler_desc_raw(set, i, j, sampler);
      }
   }

   *out_set = set;
   return VK_SUCCESS;

err_free_set:
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
   return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);
}

VkResult
panvk_per_arch(AllocateDescriptorSets)(
   VkDevice _device, const VkDescriptorSetAllocateInfo *pAllocateInfo,
   VkDescriptorSet *pDescriptorSets)
{
   VK_FROM_HANDLE(panvk_device, device, _device);
   VK_FROM_HANDLE(panvk_descriptor_pool, pool, pAllocateInfo->descriptorPool);
   VkResult result;
   unsigned i;

   for (i = 0; i < pAllocateInfo->descriptorSetCount; i++) {
      VK_FROM_HANDLE(panvk_descriptor_set_layout, layout,
                     pAllocateInfo->pSetLayouts[i]);
      struct panvk_descriptor_set *set = NULL;

      result =
         panvk_per_arch(descriptor_set_create)(device, pool, layout, &set);
      if (result != VK_SUCCESS)
         goto err_free_sets;

      pDescriptorSets[i] = panvk_descriptor_set_to_handle(set);
   }

   return VK_SUCCESS;

err_free_sets:
   panvk_FreeDescriptorSets(_device, pAllocateInfo->descriptorPool, i,
                            pDescriptorSets);
   for (i = 0; i < pAllocateInfo->descriptorSetCount; i++)
      pDescriptorSets[i] = VK_NULL_HANDLE;

   return result;
}

static void *
panvk_desc_ubo_data(struct panvk_descriptor_set *set, uint32_t binding,
                    uint32_t elem)
{
   const struct panvk_descriptor_set_binding_layout *binding_layout =
      &set->layout->bindings[binding];

   return (char *)set->desc_bo->ptr.cpu + binding_layout->desc_ubo_offset +
          elem * binding_layout->desc_ubo_stride;
}

static struct mali_sampler_packed *
panvk_sampler_desc(struct panvk_descriptor_set *set, uint32_t binding,
                   uint32_t elem)
{
   const struct panvk_descriptor_set_binding_layout *binding_layout =
      &set->layout->bindings[binding];

   uint32_t sampler_idx = binding_layout->sampler_idx + elem;

   return &((struct mali_sampler_packed *)set->samplers)[sampler_idx];
}

static void
panvk_write_sampler_desc_raw(struct panvk_descriptor_set *set, uint32_t binding,
                             uint32_t elem, struct panvk_sampler *sampler)
{
   memcpy(panvk_sampler_desc(set, binding, elem), &sampler->desc,
          sizeof(sampler->desc));
}

static void
panvk_write_sampler_desc(UNUSED struct panvk_device *dev,
                         struct panvk_descriptor_set *set, uint32_t binding,
                         uint32_t elem,
                         const VkDescriptorImageInfo *const pImageInfo)
{
   const struct panvk_descriptor_set_binding_layout *binding_layout =
      &set->layout->bindings[binding];

   if (binding_layout->immutable_samplers)
      return;

   VK_FROM_HANDLE(panvk_sampler, sampler, pImageInfo->sampler);
   panvk_write_sampler_desc_raw(set, binding, elem, sampler);
}

static void
panvk_copy_sampler_desc(struct panvk_descriptor_set *dst_set,
                        uint32_t dst_binding, uint32_t dst_elem,
                        struct panvk_descriptor_set *src_set,
                        uint32_t src_binding, uint32_t src_elem)
{
   const struct panvk_descriptor_set_binding_layout *dst_binding_layout =
      &dst_set->layout->bindings[dst_binding];

   if (dst_binding_layout->immutable_samplers)
      return;

   memcpy(panvk_sampler_desc(dst_set, dst_binding, dst_elem),
          panvk_sampler_desc(src_set, src_binding, src_elem),
          sizeof(struct mali_sampler_packed));
}

static struct mali_texture_packed *
panvk_tex_desc(struct panvk_descriptor_set *set, uint32_t binding,
               uint32_t elem)
{
   const struct panvk_descriptor_set_binding_layout *binding_layout =
      &set->layout->bindings[binding];

   unsigned tex_idx = binding_layout->tex_idx + elem;

   return &((struct mali_texture_packed *)set->textures)[tex_idx];
}

static void
panvk_write_tex_desc(UNUSED struct panvk_device *dev,
                     struct panvk_descriptor_set *set, uint32_t binding,
                     uint32_t elem,
                     const VkDescriptorImageInfo *const pImageInfo)
{
   VK_FROM_HANDLE(panvk_image_view, view, pImageInfo->imageView);

   memcpy(panvk_tex_desc(set, binding, elem), view->descs.tex,
          pan_size(TEXTURE));

   panvk_fill_image_desc(panvk_desc_ubo_data(set, binding, elem), view);
}

static void
panvk_copy_tex_desc(struct panvk_descriptor_set *dst_set, uint32_t dst_binding,
                    uint32_t dst_elem, struct panvk_descriptor_set *src_set,
                    uint32_t src_binding, uint32_t src_elem)
{
   *panvk_tex_desc(dst_set, dst_binding, dst_elem) =
      *panvk_tex_desc(src_set, src_binding, src_elem);

   /* Descriptor UBO data gets copied automatically */
}

static void
panvk_write_tex_buf_desc(UNUSED struct panvk_device *dev,
                         struct panvk_descriptor_set *set, uint32_t binding,
                         uint32_t elem, const VkBufferView bufferView)
{
   VK_FROM_HANDLE(panvk_buffer_view, view, bufferView);

   memcpy(panvk_tex_desc(set, binding, elem), view->descs.tex,
          pan_size(TEXTURE));

   panvk_fill_bview_desc(panvk_desc_ubo_data(set, binding, elem), view);
}

static uint32_t
panvk_img_idx(struct panvk_descriptor_set *set, uint32_t binding, uint32_t elem)
{
   const struct panvk_descriptor_set_binding_layout *binding_layout =
      &set->layout->bindings[binding];

   return binding_layout->img_idx + elem;
}

static void
panvk_write_img_desc(struct panvk_device *dev, struct panvk_descriptor_set *set,
                     uint32_t binding, uint32_t elem,
                     const VkDescriptorImageInfo *pImageInfo)
{
   const struct panfrost_device *pdev = &dev->physical_device->pdev;
   VK_FROM_HANDLE(panvk_image_view, view, pImageInfo->imageView);

   unsigned img_idx = panvk_img_idx(set, binding, elem);
   void *attrib_buf = (uint8_t *)set->img_attrib_bufs +
                      (pan_size(ATTRIBUTE_BUFFER) * 2 * img_idx);

   set->img_fmts[img_idx] = pdev->formats[view->pview.format].hw;
   memcpy(attrib_buf, view->descs.img_attrib_buf,
          pan_size(ATTRIBUTE_BUFFER) * 2);

   panvk_fill_image_desc(panvk_desc_ubo_data(set, binding, elem), view);
}

static void
panvk_copy_img_desc(struct panvk_descriptor_set *dst_set, uint32_t dst_binding,
                    uint32_t dst_elem, struct panvk_descriptor_set *src_set,
                    uint32_t src_binding, uint32_t src_elem)
{
   unsigned dst_img_idx = panvk_img_idx(dst_set, dst_binding, dst_elem);
   unsigned src_img_idx = panvk_img_idx(src_set, src_binding, src_elem);

   void *dst_attrib_buf = (uint8_t *)dst_set->img_attrib_bufs +
                          (pan_size(ATTRIBUTE_BUFFER) * 2 * dst_img_idx);
   void *src_attrib_buf = (uint8_t *)src_set->img_attrib_bufs +
                          (pan_size(ATTRIBUTE_BUFFER) * 2 * src_img_idx);

   dst_set->img_fmts[dst_img_idx] = src_set->img_fmts[src_img_idx];
   memcpy(dst_attrib_buf, src_attrib_buf, pan_size(ATTRIBUTE_BUFFER) * 2);

   /* Descriptor UBO data gets copied automatically */
}

static void
panvk_write_img_buf_desc(struct panvk_device *dev,
                         struct panvk_descriptor_set *set, uint32_t binding,
                         uint32_t elem, const VkBufferView bufferView)
{
   const struct panfrost_device *pdev = &dev->physical_device->pdev;
   VK_FROM_HANDLE(panvk_buffer_view, view, bufferView);

   unsigned img_idx = panvk_img_idx(set, binding, elem);
   void *attrib_buf = (uint8_t *)set->img_attrib_bufs +
                      (pan_size(ATTRIBUTE_BUFFER) * 2 * img_idx);

   set->img_fmts[img_idx] = pdev->formats[view->fmt].hw;
   memcpy(attrib_buf, view->descs.img_attrib_buf,
          pan_size(ATTRIBUTE_BUFFER) * 2);

   panvk_fill_bview_desc(panvk_desc_ubo_data(set, binding, elem), view);
}

static struct mali_uniform_buffer_packed *
panvk_ubo_desc(struct panvk_descriptor_set *set, uint32_t binding,
               uint32_t elem)
{
   const struct panvk_descriptor_set_binding_layout *binding_layout =
      &set->layout->bindings[binding];

   unsigned ubo_idx = binding_layout->ubo_idx + elem;

   return &((struct mali_uniform_buffer_packed *)set->ubos)[ubo_idx];
}

static void
panvk_write_ubo_desc(UNUSED struct panvk_device *dev,
                     struct panvk_descriptor_set *set, uint32_t binding,
                     uint32_t elem, const VkDescriptorBufferInfo *pBufferInfo)
{
   VK_FROM_HANDLE(panvk_buffer, buffer, pBufferInfo->buffer);

   mali_ptr ptr = panvk_buffer_gpu_ptr(buffer, pBufferInfo->offset);
   size_t size =
      panvk_buffer_range(buffer, pBufferInfo->offset, pBufferInfo->range);

   panvk_per_arch(emit_ubo)(ptr, size, panvk_ubo_desc(set, binding, elem));
}

static void
panvk_copy_ubo_desc(struct panvk_descriptor_set *dst_set, uint32_t dst_binding,
                    uint32_t dst_elem, struct panvk_descriptor_set *src_set,
                    uint32_t src_binding, uint32_t src_elem)
{
   *panvk_ubo_desc(dst_set, dst_binding, dst_elem) =
      *panvk_ubo_desc(src_set, src_binding, src_elem);
}

static struct panvk_buffer_desc *
panvk_dyn_ubo_desc(struct panvk_descriptor_set *set, uint32_t binding,
                   uint32_t elem)
{
   const struct panvk_descriptor_set_binding_layout *binding_layout =
      &set->layout->bindings[binding];

   return &set->dyn_ubos[binding_layout->dyn_ubo_idx + elem];
}

static void
panvk_write_dyn_ubo_desc(UNUSED struct panvk_device *dev,
                         struct panvk_descriptor_set *set, uint32_t binding,
                         uint32_t elem,
                         const VkDescriptorBufferInfo *pBufferInfo)
{
   VK_FROM_HANDLE(panvk_buffer, buffer, pBufferInfo->buffer);

   *panvk_dyn_ubo_desc(set, binding, elem) = (struct panvk_buffer_desc){
      .buffer = buffer,
      .offset = pBufferInfo->offset,
      .size = pBufferInfo->range,
   };
}

static void
panvk_copy_dyn_ubo_desc(struct panvk_descriptor_set *dst_set,
                        uint32_t dst_binding, uint32_t dst_elem,
                        struct panvk_descriptor_set *src_set,
                        uint32_t src_binding, uint32_t src_elem)
{
   *panvk_dyn_ubo_desc(dst_set, dst_binding, dst_elem) =
      *panvk_dyn_ubo_desc(src_set, src_binding, src_elem);
}

static void
panvk_write_ssbo_desc(UNUSED struct panvk_device *dev,
                      struct panvk_descriptor_set *set, uint32_t binding,
                      uint32_t elem, const VkDescriptorBufferInfo *pBufferInfo)
{
   VK_FROM_HANDLE(panvk_buffer, buffer, pBufferInfo->buffer);

   struct panvk_ssbo_addr *desc = panvk_desc_ubo_data(set, binding, elem);
   *desc = (struct panvk_ssbo_addr){
      .base_addr = panvk_buffer_gpu_ptr(buffer, pBufferInfo->offset),
      .size =
         panvk_buffer_range(buffer, pBufferInfo->offset, pBufferInfo->range),
   };
}

static void
panvk_copy_ssbo_desc(struct panvk_descriptor_set *dst_set, uint32_t dst_binding,
                     uint32_t dst_elem, struct panvk_descriptor_set *src_set,
                     uint32_t src_binding, uint32_t src_elem)
{
   /* Descriptor UBO data gets copied automatically */
}

static struct panvk_buffer_desc *
panvk_dyn_ssbo_desc(struct panvk_descriptor_set *set, uint32_t binding,
                    uint32_t elem)
{
   const struct panvk_descriptor_set_binding_layout *binding_layout =
      &set->layout->bindings[binding];

   return &set->dyn_ssbos[binding_layout->dyn_ssbo_idx + elem];
}

static void
panvk_write_dyn_ssbo_desc(UNUSED struct panvk_device *dev,
                          struct panvk_descriptor_set *set, uint32_t binding,
                          uint32_t elem,
                          const VkDescriptorBufferInfo *pBufferInfo)
{
   VK_FROM_HANDLE(panvk_buffer, buffer, pBufferInfo->buffer);

   *panvk_dyn_ssbo_desc(set, binding, elem) = (struct panvk_buffer_desc){
      .buffer = buffer,
      .offset = pBufferInfo->offset,
      .size = pBufferInfo->range,
   };
}

static void
panvk_copy_dyn_ssbo_desc(struct panvk_descriptor_set *dst_set,
                         uint32_t dst_binding, uint32_t dst_elem,
                         struct panvk_descriptor_set *src_set,
                         uint32_t src_binding, uint32_t src_elem)
{
   *panvk_dyn_ssbo_desc(dst_set, dst_binding, dst_elem) =
      *panvk_dyn_ssbo_desc(src_set, src_binding, src_elem);
}

void
panvk_per_arch(UpdateDescriptorSets)(
   VkDevice _device, uint32_t descriptorWriteCount,
   const VkWriteDescriptorSet *pDescriptorWrites, uint32_t descriptorCopyCount,
   const VkCopyDescriptorSet *pDescriptorCopies)
{
   VK_FROM_HANDLE(panvk_device, dev, _device);

   for (unsigned i = 0; i < descriptorWriteCount; i++) {
      const VkWriteDescriptorSet *write = &pDescriptorWrites[i];
      VK_FROM_HANDLE(panvk_descriptor_set, set, write->dstSet);

      switch (write->descriptorType) {
      case VK_DESCRIPTOR_TYPE_SAMPLER:
         for (uint32_t j = 0; j < write->descriptorCount; j++) {
            panvk_write_sampler_desc(dev, set, write->dstBinding,
                                     write->dstArrayElement + j,
                                     &write->pImageInfo[j]);
         }
         break;

      case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
         for (uint32_t j = 0; j < write->descriptorCount; j++) {
            panvk_write_sampler_desc(dev, set, write->dstBinding,
                                     write->dstArrayElement + j,
                                     &write->pImageInfo[j]);
            panvk_write_tex_desc(dev, set, write->dstBinding,
                                 write->dstArrayElement + j,
                                 &write->pImageInfo[j]);
         }
         break;

      case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
         for (uint32_t j = 0; j < write->descriptorCount; j++) {
            panvk_write_tex_desc(dev, set, write->dstBinding,
                                 write->dstArrayElement + j,
                                 &write->pImageInfo[j]);
         }
         break;

      case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
      case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
         for (uint32_t j = 0; j < write->descriptorCount; j++) {
            panvk_write_img_desc(dev, set, write->dstBinding,
                                 write->dstArrayElement + j,
                                 &write->pImageInfo[j]);
         }
         break;

      case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
         for (uint32_t j = 0; j < write->descriptorCount; j++) {
            panvk_write_tex_buf_desc(dev, set, write->dstBinding,
                                     write->dstArrayElement + j,
                                     write->pTexelBufferView[j]);
         }
         break;

      case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
         for (uint32_t j = 0; j < write->descriptorCount; j++) {
            panvk_write_img_buf_desc(dev, set, write->dstBinding,
                                     write->dstArrayElement + j,
                                     write->pTexelBufferView[j]);
         }
         break;

      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
         for (uint32_t j = 0; j < write->descriptorCount; j++) {
            panvk_write_ubo_desc(dev, set, write->dstBinding,
                                 write->dstArrayElement + j,
                                 &write->pBufferInfo[j]);
         }
         break;

      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
         for (uint32_t j = 0; j < write->descriptorCount; j++) {
            panvk_write_dyn_ubo_desc(dev, set, write->dstBinding,
                                     write->dstArrayElement + j,
                                     &write->pBufferInfo[j]);
         }
         break;

      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
         for (uint32_t j = 0; j < write->descriptorCount; j++) {
            panvk_write_ssbo_desc(dev, set, write->dstBinding,
                                  write->dstArrayElement + j,
                                  &write->pBufferInfo[j]);
         }
         break;

      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
         for (uint32_t j = 0; j < write->descriptorCount; j++) {
            panvk_write_dyn_ssbo_desc(dev, set, write->dstBinding,
                                      write->dstArrayElement + j,
                                      &write->pBufferInfo[j]);
         }
         break;

      default:
         unreachable("Unsupported descriptor type");
      }
   }

   for (unsigned i = 0; i < descriptorCopyCount; i++) {
      const VkCopyDescriptorSet *copy = &pDescriptorCopies[i];
      VK_FROM_HANDLE(panvk_descriptor_set, src_set, copy->srcSet);
      VK_FROM_HANDLE(panvk_descriptor_set, dst_set, copy->dstSet);

      const struct panvk_descriptor_set_binding_layout *dst_binding_layout =
         &dst_set->layout->bindings[copy->dstBinding];
      const struct panvk_descriptor_set_binding_layout *src_binding_layout =
         &src_set->layout->bindings[copy->srcBinding];

      assert(dst_binding_layout->type == src_binding_layout->type);

      if (dst_binding_layout->desc_ubo_stride > 0 &&
          src_binding_layout->desc_ubo_stride > 0) {
         for (uint32_t j = 0; j < copy->descriptorCount; j++) {
            memcpy(panvk_desc_ubo_data(dst_set, copy->dstBinding,
                                       copy->dstArrayElement + j),
                   panvk_desc_ubo_data(src_set, copy->srcBinding,
                                       copy->srcArrayElement + j),
                   MIN2(dst_binding_layout->desc_ubo_stride,
                        src_binding_layout->desc_ubo_stride));
         }
      }

      switch (src_binding_layout->type) {
      case VK_DESCRIPTOR_TYPE_SAMPLER:
         for (uint32_t j = 0; j < copy->descriptorCount; j++) {
            panvk_copy_sampler_desc(
               dst_set, copy->dstBinding, copy->dstArrayElement + j, src_set,
               copy->srcBinding, copy->srcArrayElement + j);
         }
         break;

      case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
         for (uint32_t j = 0; j < copy->descriptorCount; j++) {
            panvk_copy_sampler_desc(
               dst_set, copy->dstBinding, copy->dstArrayElement + j, src_set,
               copy->srcBinding, copy->srcArrayElement + j);
            panvk_copy_tex_desc(dst_set, copy->dstBinding,
                                copy->dstArrayElement + j, src_set,
                                copy->srcBinding, copy->srcArrayElement + j);
         }
         break;

      case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
      case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
         for (uint32_t j = 0; j < copy->descriptorCount; j++) {
            panvk_copy_tex_desc(dst_set, copy->dstBinding,
                                copy->dstArrayElement + j, src_set,
                                copy->srcBinding, copy->srcArrayElement + j);
         }
         break;

      case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
      case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
      case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
         for (uint32_t j = 0; j < copy->descriptorCount; j++) {
            panvk_copy_img_desc(dst_set, copy->dstBinding,
                                copy->dstArrayElement + j, src_set,
                                copy->srcBinding, copy->srcArrayElement + j);
         }
         break;

      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
         for (uint32_t j = 0; j < copy->descriptorCount; j++) {
            panvk_copy_ubo_desc(dst_set, copy->dstBinding,
                                copy->dstArrayElement + j, src_set,
                                copy->srcBinding, copy->srcArrayElement + j);
         }
         break;

      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
         for (uint32_t j = 0; j < copy->descriptorCount; j++) {
            panvk_copy_dyn_ubo_desc(
               dst_set, copy->dstBinding, copy->dstArrayElement + j, src_set,
               copy->srcBinding, copy->srcArrayElement + j);
         }
         break;

      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
         for (uint32_t j = 0; j < copy->descriptorCount; j++) {
            panvk_copy_ssbo_desc(dst_set, copy->dstBinding,
                                 copy->dstArrayElement + j, src_set,
                                 copy->srcBinding, copy->srcArrayElement + j);
         }
         break;

      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
         for (uint32_t j = 0; j < copy->descriptorCount; j++) {
            panvk_copy_dyn_ssbo_desc(
               dst_set, copy->dstBinding, copy->dstArrayElement + j, src_set,
               copy->srcBinding, copy->srcArrayElement + j);
         }
         break;

      default:
         unreachable("Unsupported descriptor type");
      }
   }
}

void
panvk_per_arch(UpdateDescriptorSetWithTemplate)(
   VkDevice _device, VkDescriptorSet descriptorSet,
   VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void *data)
{
   VK_FROM_HANDLE(panvk_device, dev, _device);
   VK_FROM_HANDLE(panvk_descriptor_set, set, descriptorSet);
   VK_FROM_HANDLE(vk_descriptor_update_template, template,
                  descriptorUpdateTemplate);

   const struct panvk_descriptor_set_layout *layout = set->layout;

   for (uint32_t i = 0; i < template->entry_count; i++) {
      const struct vk_descriptor_template_entry *entry = &template->entries[i];
      const struct panvk_descriptor_set_binding_layout *binding_layout =
         &layout->bindings[entry->binding];

      switch (entry->type) {
      case VK_DESCRIPTOR_TYPE_SAMPLER:
      case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
      case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
         for (unsigned j = 0; j < entry->array_count; j++) {
            const VkDescriptorImageInfo *info =
               data + entry->offset + j * entry->stride;

            if ((entry->type == VK_DESCRIPTOR_TYPE_SAMPLER ||
                 entry->type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) &&
                !binding_layout->immutable_samplers) {

               panvk_write_sampler_desc(dev, set, entry->binding,
                                        entry->array_element + j, info);
            }

            if (entry->type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE ||
                entry->type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {

               panvk_write_tex_desc(dev, set, entry->binding,
                                    entry->array_element + j, info);
            }
         }
         break;

      case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
      case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
         for (unsigned j = 0; j < entry->array_count; j++) {
            const VkDescriptorImageInfo *info =
               data + entry->offset + j * entry->stride;

            panvk_write_img_desc(dev, set, entry->binding,
                                 entry->array_element + j, info);
         }
         break;

      case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
         for (unsigned j = 0; j < entry->array_count; j++) {
            const VkBufferView *view = data + entry->offset + j * entry->stride;

            panvk_write_tex_buf_desc(dev, set, entry->binding,
                                     entry->array_element + j, *view);
         }
         break;

      case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
         for (unsigned j = 0; j < entry->array_count; j++) {
            const VkBufferView *view = data + entry->offset + j * entry->stride;

            panvk_write_img_buf_desc(dev, set, entry->binding,
                                     entry->array_element + j, *view);
         }
         break;

      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
         for (unsigned j = 0; j < entry->array_count; j++) {
            const VkDescriptorBufferInfo *info =
               data + entry->offset + j * entry->stride;

            panvk_write_ubo_desc(dev, set, entry->binding,
                                 entry->array_element + j, info);
         }
         break;

      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
         for (unsigned j = 0; j < entry->array_count; j++) {
            const VkDescriptorBufferInfo *info =
               data + entry->offset + j * entry->stride;

            panvk_write_dyn_ubo_desc(dev, set, entry->binding,
                                     entry->array_element + j, info);
         }
         break;

      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
         for (unsigned j = 0; j < entry->array_count; j++) {
            const VkDescriptorBufferInfo *info =
               data + entry->offset + j * entry->stride;

            panvk_write_ssbo_desc(dev, set, entry->binding,
                                  entry->array_element + j, info);
         }
         break;

      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
         for (unsigned j = 0; j < entry->array_count; j++) {
            const VkDescriptorBufferInfo *info =
               data + entry->offset + j * entry->stride;

            panvk_write_dyn_ssbo_desc(dev, set, entry->binding,
                                      entry->array_element + j, info);
         }
         break;
      default:
         unreachable("Invalid type");
      }
   }
}
