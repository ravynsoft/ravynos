/*
 * Copyright © 2022 Collabora Ltd
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
#ifndef VK_META_H
#define VK_META_H

#include "vk_limits.h"
#include "vk_object.h"

#include "util/simple_mtx.h"
#include "util/u_dynarray.h"

#ifdef __cplusplus
extern "C" {
#endif

struct hash_table;
struct vk_command_buffer;
struct vk_device;
struct vk_image;

struct vk_meta_rect {
   uint32_t x0, y0, x1, y1;
   float z;
   uint32_t layer;
};

#define VK_PRIMITIVE_TOPOLOGY_META_RECT_LIST_MESA (VkPrimitiveTopology)11

struct vk_meta_device {
   struct hash_table *cache;
   simple_mtx_t cache_mtx;

   uint32_t max_bind_map_buffer_size_B;
   bool use_layered_rendering;
   bool use_gs_for_layer;
   bool use_stencil_export;

   VkResult (*cmd_bind_map_buffer)(struct vk_command_buffer *cmd,
                                   struct vk_meta_device *meta,
                                   VkBuffer buffer,
                                   void **map_out);

   void (*cmd_draw_rects)(struct vk_command_buffer *cmd,
                          struct vk_meta_device *meta,
                          uint32_t rect_count,
                          const struct vk_meta_rect *rects);

   void (*cmd_draw_volume)(struct vk_command_buffer *cmd,
                           struct vk_meta_device *meta,
                           const struct vk_meta_rect *rect,
                           uint32_t layer_count);
};

VkResult vk_meta_device_init(struct vk_device *device,
                             struct vk_meta_device *meta);
void vk_meta_device_finish(struct vk_device *device,
                           struct vk_meta_device *meta);

/** Keys should start with one of these to ensure uniqueness */
enum vk_meta_object_key_type {
   VK_META_OBJECT_KEY_TYPE_INVALD = 0,
   VK_META_OBJECT_KEY_CLEAR_PIPELINE,
   VK_META_OBJECT_KEY_BLIT_PIPELINE,
   VK_META_OBJECT_KEY_BLIT_SAMPLER,
};

uint64_t vk_meta_lookup_object(struct vk_meta_device *meta,
                                VkObjectType obj_type,
                                const void *key_data, size_t key_size);

uint64_t vk_meta_cache_object(struct vk_device *device,
                              struct vk_meta_device *meta,
                              const void *key_data, size_t key_size,
                              VkObjectType obj_type,
                              uint64_t handle);

static inline VkDescriptorSetLayout
vk_meta_lookup_descriptor_set_layout(struct vk_meta_device *meta,
                                     const void *key_data, size_t key_size)
{
   return (VkDescriptorSetLayout)
      vk_meta_lookup_object(meta, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT,
                            key_data, key_size);
}

static inline VkPipelineLayout
vk_meta_lookup_pipeline_layout(struct vk_meta_device *meta,
                               const void *key_data, size_t key_size)
{
   return (VkPipelineLayout)
      vk_meta_lookup_object(meta, VK_OBJECT_TYPE_PIPELINE_LAYOUT,
                            key_data, key_size);
}

static inline VkPipeline
vk_meta_lookup_pipeline(struct vk_meta_device *meta,
                        const void *key_data, size_t key_size)
{
   return (VkPipeline)vk_meta_lookup_object(meta, VK_OBJECT_TYPE_PIPELINE,
                                            key_data, key_size);
}

static inline VkSampler
vk_meta_lookup_sampler(struct vk_meta_device *meta,
                       const void *key_data, size_t key_size)
{
   return (VkSampler)vk_meta_lookup_object(meta, VK_OBJECT_TYPE_SAMPLER,
                                           key_data, key_size);
}

struct vk_meta_rendering_info {
   uint32_t view_mask;
   uint32_t samples;
   uint32_t color_attachment_count;
   VkFormat color_attachment_formats[MESA_VK_MAX_COLOR_ATTACHMENTS];
   VkFormat depth_attachment_format;
   VkFormat stencil_attachment_format;
};

VkResult
vk_meta_create_descriptor_set_layout(struct vk_device *device,
                                     struct vk_meta_device *meta,
                                     const VkDescriptorSetLayoutCreateInfo *info,
                                     const void *key_data, size_t key_size,
                                     VkDescriptorSetLayout *layout_out);

VkResult
vk_meta_create_pipeline_layout(struct vk_device *device,
                               struct vk_meta_device *meta,
                               const VkPipelineLayoutCreateInfo *info,
                               const void *key_data, size_t key_size,
                               VkPipelineLayout *layout_out);

VkResult
vk_meta_get_pipeline_layout(struct vk_device *device,
                            struct vk_meta_device *meta,
                            const VkDescriptorSetLayoutCreateInfo *desc_info,
                            const VkPushConstantRange *push_range,
                            const void *key_data, size_t key_size,
                            VkPipelineLayout *layout_out);

VkResult
vk_meta_create_graphics_pipeline(struct vk_device *device,
                                 struct vk_meta_device *meta,
                                 const VkGraphicsPipelineCreateInfo *info,
                                 const struct vk_meta_rendering_info *render,
                                 const void *key_data, size_t key_size,
                                 VkPipeline *pipeline_out);

VkResult
vk_meta_create_compute_pipeline(struct vk_device *device,
                                struct vk_meta_device *meta,
                                const VkComputePipelineCreateInfo *info,
                                const void *key_data, size_t key_size,
                                VkPipeline *pipeline_out);

VkResult
vk_meta_create_sampler(struct vk_device *device,
                       struct vk_meta_device *meta,
                       const VkSamplerCreateInfo *info,
                       const void *key_data, size_t key_size,
                       VkSampler *sampler_out);

struct vk_meta_object_list {
   struct util_dynarray arr;
};

void vk_meta_object_list_init(struct vk_meta_object_list *mol);
void vk_meta_object_list_reset(struct vk_device *device,
                               struct vk_meta_object_list *mol);
void vk_meta_object_list_finish(struct vk_device *device,
                                struct vk_meta_object_list *mol);

static inline void
vk_meta_object_list_add_obj(struct vk_meta_object_list *mol,
                            struct vk_object_base *obj)
{
   util_dynarray_append(&mol->arr, struct vk_object_base *, obj);
}

static inline void
vk_meta_object_list_add_handle(struct vk_meta_object_list *mol,
                               VkObjectType obj_type,
                               uint64_t handle)
{
   vk_meta_object_list_add_obj(mol,
      vk_object_base_from_u64_handle(handle, obj_type));
}

VkResult vk_meta_create_buffer(struct vk_command_buffer *cmd,
                               struct vk_meta_device *meta,
                               const VkBufferCreateInfo *info,
                               VkBuffer *buffer_out);
VkResult vk_meta_create_image_view(struct vk_command_buffer *cmd,
                                   struct vk_meta_device *meta,
                                   const VkImageViewCreateInfo *info,
                                   VkImageView *image_view_out);

void vk_meta_draw_rects(struct vk_command_buffer *cmd,
                        struct vk_meta_device *meta,
                        uint32_t rect_count,
                        const struct vk_meta_rect *rects);

void vk_meta_draw_volume(struct vk_command_buffer *cmd,
                         struct vk_meta_device *meta,
                         const struct vk_meta_rect *rect,
                         uint32_t layer_count);

void vk_meta_clear_attachments(struct vk_command_buffer *cmd,
                               struct vk_meta_device *meta,
                               const struct vk_meta_rendering_info *render,
                               uint32_t attachment_count,
                               const VkClearAttachment *attachments,
                               uint32_t rect_count,
                               const VkClearRect *rects);

void vk_meta_clear_rendering(struct vk_meta_device *meta,
                             struct vk_command_buffer *cmd,
                             const VkRenderingInfo *pRenderingInfo);

void vk_meta_clear_color_image(struct vk_command_buffer *cmd,
                               struct vk_meta_device *meta,
                               struct vk_image *image,
                               VkImageLayout image_layout,
                               VkFormat format,
                               const VkClearColorValue *color,
                               uint32_t range_count,
                               const VkImageSubresourceRange *ranges);

void vk_meta_clear_depth_stencil_image(struct vk_command_buffer *cmd,
                                       struct vk_meta_device *meta,
                                       struct vk_image *image,
                                       VkImageLayout image_layout,
                                       const VkClearDepthStencilValue *depth_stencil,
                                       uint32_t range_count,
                                       const VkImageSubresourceRange *ranges);

void vk_meta_blit_image(struct vk_command_buffer *cmd,
                        struct vk_meta_device *meta,
                        struct vk_image *src_image,
                        VkFormat src_format,
                        VkImageLayout src_image_layout,
                        struct vk_image *dst_image,
                        VkFormat dst_format,
                        VkImageLayout dst_image_layout,
                        uint32_t region_count,
                        const VkImageBlit2 *regions,
                        VkFilter filter);

void vk_meta_blit_image2(struct vk_command_buffer *cmd,
                         struct vk_meta_device *meta,
                         const VkBlitImageInfo2 *blit);

void vk_meta_resolve_image(struct vk_command_buffer *cmd,
                           struct vk_meta_device *meta,
                           struct vk_image *src_image,
                           VkFormat src_format,
                           VkImageLayout src_image_layout,
                           struct vk_image *dst_image,
                           VkFormat dst_format,
                           VkImageLayout dst_image_layout,
                           uint32_t region_count,
                           const VkImageResolve2 *regions,
                           VkResolveModeFlagBits resolve_mode,
                           VkResolveModeFlagBits stencil_resolve_mode);

void vk_meta_resolve_image2(struct vk_command_buffer *cmd,
                            struct vk_meta_device *meta,
                            const VkResolveImageInfo2 *resolve);

void vk_meta_resolve_rendering(struct vk_command_buffer *cmd,
                               struct vk_meta_device *meta,
                               const VkRenderingInfo *pRenderingInfo);

#ifdef __cplusplus
}
#endif

#endif /* VK_META_H */
