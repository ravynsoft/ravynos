/*
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 * SPDX-License-Identifier: MIT
 *
 * based in part on anv driver which is:
 * Copyright © 2015 Intel Corporation
 */

#ifndef TU_IMAGE_H
#define TU_IMAGE_H

#include "tu_common.h"

#define tu_image_view_stencil(iview, x) \
   ((iview->view.x & ~A6XX_##x##_COLOR_FORMAT__MASK) | A6XX_##x##_COLOR_FORMAT(FMT6_8_UINT))

#define tu_image_view_depth(iview, x) \
   ((iview->view.x & ~A6XX_##x##_COLOR_FORMAT__MASK) | A6XX_##x##_COLOR_FORMAT(FMT6_32_FLOAT))

struct tu_image
{
   struct vk_image vk;

   struct fdl_layout layout[3];
   uint32_t total_size;

#ifdef ANDROID
   /* For VK_ANDROID_native_buffer, the WSI image owns the memory, */
   VkDeviceMemory owned_memory;
#endif

   /* Set when bound */
   struct tu_bo *bo;
   uint64_t iova;

   /* For fragment density map */
   void *map;

   uint32_t lrz_height;
   uint32_t lrz_pitch;
   uint32_t lrz_offset;
   uint32_t lrz_fc_offset;
   uint32_t lrz_fc_size;
};
VK_DEFINE_NONDISP_HANDLE_CASTS(tu_image, vk.base, VkImage, VK_OBJECT_TYPE_IMAGE)

struct tu_image_view
{
   struct vk_image_view vk;

   struct tu_image *image; /**< VkImageViewCreateInfo::image */

   struct fdl6_view view;

   unsigned char swizzle[4];

   /* for d32s8 separate depth */
   uint64_t depth_base_addr;
   uint32_t depth_layer_size;
   uint32_t depth_pitch;

   /* for d32s8 separate stencil */
   uint64_t stencil_base_addr;
   uint32_t stencil_layer_size;
   uint32_t stencil_pitch;
};
VK_DEFINE_NONDISP_HANDLE_CASTS(tu_image_view, vk.base, VkImageView,
                               VK_OBJECT_TYPE_IMAGE_VIEW);

struct tu_buffer_view
{
   struct vk_object_base base;

   uint32_t descriptor[A6XX_TEX_CONST_DWORDS];

   struct tu_buffer *buffer;
};
VK_DEFINE_NONDISP_HANDLE_CASTS(tu_buffer_view, base, VkBufferView,
                               VK_OBJECT_TYPE_BUFFER_VIEW)

uint32_t tu6_plane_count(VkFormat format);
enum pipe_format tu6_plane_format(VkFormat format, uint32_t plane);

uint32_t tu6_plane_index(VkFormat format, VkImageAspectFlags aspect_mask);

enum pipe_format tu_format_for_aspect(enum pipe_format format,
                                      VkImageAspectFlags aspect_mask);

void
tu_cs_image_ref(struct tu_cs *cs, const struct fdl6_view *iview, uint32_t layer);

template <chip CHIP>
void
tu_cs_image_ref_2d(struct tu_cs *cs, const struct fdl6_view *iview, uint32_t layer, bool src);

void
tu_cs_image_flag_ref(struct tu_cs *cs, const struct fdl6_view *iview, uint32_t layer);

void
tu_cs_image_stencil_ref(struct tu_cs *cs, const struct tu_image_view *iview, uint32_t layer);

void
tu_cs_image_depth_ref(struct tu_cs *cs, const struct tu_image_view *iview, uint32_t layer);

bool
tiling_possible(VkFormat format);

bool
ubwc_possible(struct tu_device *device,
              VkFormat format,
              VkImageType type,
              VkImageUsageFlags usage,
              VkImageUsageFlags stencil_usage,
              const struct fd_dev_info *info,
              VkSampleCountFlagBits samples,
              bool use_z24uint_s8uint);

void
tu_buffer_view_init(struct tu_buffer_view *view,
                    struct tu_device *device,
                    const VkBufferViewCreateInfo *pCreateInfo);

struct tu_frag_area {
   float width;
   float height;
};

void
tu_fragment_density_map_sample(const struct tu_image_view *fdm,
                               uint32_t x, uint32_t y,
                               uint32_t width, uint32_t height,
                               uint32_t layers, struct tu_frag_area *areas);

#endif /* TU_IMAGE_H */
