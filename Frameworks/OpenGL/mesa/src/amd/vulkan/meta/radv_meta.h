/*
 * Copyright © 2016 Red Hat
 * based on intel anv code:
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef RADV_META_H
#define RADV_META_H

#include "radv_private.h"
#include "radv_shader.h"

#ifdef __cplusplus
extern "C" {
#endif

enum radv_meta_save_flags {
   RADV_META_SAVE_RENDER = (1 << 0),
   RADV_META_SAVE_CONSTANTS = (1 << 1),
   RADV_META_SAVE_DESCRIPTORS = (1 << 2),
   RADV_META_SAVE_GRAPHICS_PIPELINE = (1 << 3),
   RADV_META_SAVE_COMPUTE_PIPELINE = (1 << 4),
   RADV_META_SUSPEND_PREDICATING = (1 << 5),
};

struct radv_meta_saved_state {
   uint32_t flags;

   struct radv_descriptor_set *old_descriptor_set0;
   struct radv_graphics_pipeline *old_graphics_pipeline;
   struct radv_compute_pipeline *old_compute_pipeline;
   struct radv_dynamic_state dynamic;

   char push_constants[MAX_PUSH_CONSTANTS_SIZE];

   struct radv_rendering_state render;

   unsigned active_pipeline_gds_queries;
   unsigned active_prims_gen_gds_queries;
   unsigned active_prims_xfb_gds_queries;
   unsigned active_occlusion_queries;

   bool predicating;
};

enum radv_blit_ds_layout {
   RADV_BLIT_DS_LAYOUT_TILE_ENABLE,
   RADV_BLIT_DS_LAYOUT_TILE_DISABLE,
   RADV_BLIT_DS_LAYOUT_COUNT,
};

static inline enum radv_blit_ds_layout
radv_meta_blit_ds_to_type(VkImageLayout layout)
{
   return (layout == VK_IMAGE_LAYOUT_GENERAL) ? RADV_BLIT_DS_LAYOUT_TILE_DISABLE : RADV_BLIT_DS_LAYOUT_TILE_ENABLE;
}

static inline VkImageLayout
radv_meta_blit_ds_to_layout(enum radv_blit_ds_layout ds_layout)
{
   return ds_layout == RADV_BLIT_DS_LAYOUT_TILE_ENABLE ? VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL : VK_IMAGE_LAYOUT_GENERAL;
}

enum radv_meta_dst_layout {
   RADV_META_DST_LAYOUT_GENERAL,
   RADV_META_DST_LAYOUT_OPTIMAL,
   RADV_META_DST_LAYOUT_COUNT,
};

static inline enum radv_meta_dst_layout
radv_meta_dst_layout_from_layout(VkImageLayout layout)
{
   return (layout == VK_IMAGE_LAYOUT_GENERAL) ? RADV_META_DST_LAYOUT_GENERAL : RADV_META_DST_LAYOUT_OPTIMAL;
}

static inline VkImageLayout
radv_meta_dst_layout_to_layout(enum radv_meta_dst_layout layout)
{
   return layout == RADV_META_DST_LAYOUT_OPTIMAL ? VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL : VK_IMAGE_LAYOUT_GENERAL;
}

extern const VkFormat radv_fs_key_format_exemplars[NUM_META_FS_KEYS];
unsigned radv_format_meta_fs_key(struct radv_device *device, VkFormat format);

VkResult radv_device_init_meta(struct radv_device *device);
void radv_device_finish_meta(struct radv_device *device);

VkResult radv_device_init_meta_clear_state(struct radv_device *device, bool on_demand);
void radv_device_finish_meta_clear_state(struct radv_device *device);

VkResult radv_device_init_meta_resolve_state(struct radv_device *device, bool on_demand);
void radv_device_finish_meta_resolve_state(struct radv_device *device);

VkResult radv_device_init_meta_depth_decomp_state(struct radv_device *device, bool on_demand);
void radv_device_finish_meta_depth_decomp_state(struct radv_device *device);

VkResult radv_device_init_meta_fast_clear_flush_state(struct radv_device *device, bool on_demand);
void radv_device_finish_meta_fast_clear_flush_state(struct radv_device *device);

VkResult radv_device_init_meta_blit_state(struct radv_device *device, bool on_demand);
void radv_device_finish_meta_blit_state(struct radv_device *device);

VkResult radv_device_init_meta_blit2d_state(struct radv_device *device, bool on_demand);
void radv_device_finish_meta_blit2d_state(struct radv_device *device);

VkResult radv_device_init_meta_buffer_state(struct radv_device *device);
void radv_device_finish_meta_buffer_state(struct radv_device *device);

VkResult radv_device_init_meta_query_state(struct radv_device *device, bool on_demand);
void radv_device_finish_meta_query_state(struct radv_device *device);

VkResult radv_device_init_meta_resolve_compute_state(struct radv_device *device, bool on_demand);
void radv_device_finish_meta_resolve_compute_state(struct radv_device *device);

VkResult radv_device_init_meta_resolve_fragment_state(struct radv_device *device, bool on_demand);
void radv_device_finish_meta_resolve_fragment_state(struct radv_device *device);

VkResult radv_device_init_meta_fmask_copy_state(struct radv_device *device, bool on_demand);
void radv_device_finish_meta_fmask_copy_state(struct radv_device *device);

VkResult radv_device_init_meta_fmask_expand_state(struct radv_device *device, bool on_demand);
void radv_device_finish_meta_fmask_expand_state(struct radv_device *device);

void radv_device_finish_meta_dcc_retile_state(struct radv_device *device);

void radv_device_finish_meta_copy_vrs_htile_state(struct radv_device *device);

VkResult radv_device_init_null_accel_struct(struct radv_device *device);
VkResult radv_device_init_accel_struct_build_state(struct radv_device *device);
void radv_device_finish_accel_struct_build_state(struct radv_device *device);

VkResult radv_device_init_meta_etc_decode_state(struct radv_device *device, bool on_demand);
void radv_device_finish_meta_etc_decode_state(struct radv_device *device);

VkResult radv_device_init_meta_astc_decode_state(struct radv_device *device, bool on_demand);
void radv_device_finish_meta_astc_decode_state(struct radv_device *device);

VkResult radv_device_init_dgc_prepare_state(struct radv_device *device);
void radv_device_finish_dgc_prepare_state(struct radv_device *device);

void radv_meta_save(struct radv_meta_saved_state *saved_state, struct radv_cmd_buffer *cmd_buffer, uint32_t flags);

void radv_meta_restore(const struct radv_meta_saved_state *state, struct radv_cmd_buffer *cmd_buffer);

VkImageViewType radv_meta_get_view_type(const struct radv_image *image);

uint32_t radv_meta_get_iview_layer(const struct radv_image *dst_image, const VkImageSubresourceLayers *dst_subresource,
                                   const VkOffset3D *dst_offset);

struct radv_meta_blit2d_surf {
   /** The size of an element in bytes. */
   uint8_t bs;
   VkFormat format;

   struct radv_image *image;
   unsigned level;
   unsigned layer;
   VkImageAspectFlags aspect_mask;
   VkImageLayout current_layout;
   bool disable_compression;
};

struct radv_meta_blit2d_buffer {
   struct radv_buffer *buffer;
   uint32_t offset;
   uint32_t pitch;
   uint8_t bs;
   VkFormat format;
};

struct radv_meta_blit2d_rect {
   uint32_t src_x, src_y;
   uint32_t dst_x, dst_y;
   uint32_t width, height;
};

void radv_meta_begin_blit2d(struct radv_cmd_buffer *cmd_buffer, struct radv_meta_saved_state *save);

void radv_meta_blit2d(struct radv_cmd_buffer *cmd_buffer, struct radv_meta_blit2d_surf *src_img,
                      struct radv_meta_blit2d_buffer *src_buf, struct radv_meta_blit2d_surf *dst, unsigned num_rects,
                      struct radv_meta_blit2d_rect *rects);

void radv_meta_end_blit2d(struct radv_cmd_buffer *cmd_buffer, struct radv_meta_saved_state *save);

VkResult radv_device_init_meta_bufimage_state(struct radv_device *device);
void radv_device_finish_meta_bufimage_state(struct radv_device *device);
void radv_meta_image_to_buffer(struct radv_cmd_buffer *cmd_buffer, struct radv_meta_blit2d_surf *src,
                               struct radv_meta_blit2d_buffer *dst, unsigned num_rects,
                               struct radv_meta_blit2d_rect *rects);

void radv_meta_buffer_to_image_cs(struct radv_cmd_buffer *cmd_buffer, struct radv_meta_blit2d_buffer *src,
                                  struct radv_meta_blit2d_surf *dst, unsigned num_rects,
                                  struct radv_meta_blit2d_rect *rects);
void radv_meta_image_to_image_cs(struct radv_cmd_buffer *cmd_buffer, struct radv_meta_blit2d_surf *src,
                                 struct radv_meta_blit2d_surf *dst, unsigned num_rects,
                                 struct radv_meta_blit2d_rect *rects);
void radv_meta_clear_image_cs(struct radv_cmd_buffer *cmd_buffer, struct radv_meta_blit2d_surf *dst,
                              const VkClearColorValue *clear_color);

void radv_expand_depth_stencil(struct radv_cmd_buffer *cmd_buffer, struct radv_image *image,
                               const VkImageSubresourceRange *subresourceRange,
                               struct radv_sample_locations_state *sample_locs);
void radv_resummarize_depth_stencil(struct radv_cmd_buffer *cmd_buffer, struct radv_image *image,
                                    const VkImageSubresourceRange *subresourceRange,
                                    struct radv_sample_locations_state *sample_locs);
void radv_fast_clear_flush_image_inplace(struct radv_cmd_buffer *cmd_buffer, struct radv_image *image,
                                         const VkImageSubresourceRange *subresourceRange);
void radv_decompress_dcc(struct radv_cmd_buffer *cmd_buffer, struct radv_image *image,
                         const VkImageSubresourceRange *subresourceRange);
void radv_retile_dcc(struct radv_cmd_buffer *cmd_buffer, struct radv_image *image);
void radv_expand_fmask_image_inplace(struct radv_cmd_buffer *cmd_buffer, struct radv_image *image,
                                     const VkImageSubresourceRange *subresourceRange);
void radv_copy_vrs_htile(struct radv_cmd_buffer *cmd_buffer, struct radv_image *vrs_image, const VkRect2D *rect,
                         struct radv_image *dst_image, struct radv_buffer *htile_buffer, bool read_htile_value);

bool radv_can_use_fmask_copy(struct radv_cmd_buffer *cmd_buffer, const struct radv_image *src_image,
                             const struct radv_image *dst_image, unsigned num_rects,
                             const struct radv_meta_blit2d_rect *rects);
void radv_fmask_copy(struct radv_cmd_buffer *cmd_buffer, struct radv_meta_blit2d_surf *src,
                     struct radv_meta_blit2d_surf *dst);

void radv_meta_resolve_compute_image(struct radv_cmd_buffer *cmd_buffer, struct radv_image *src_image,
                                     VkFormat src_format, VkImageLayout src_image_layout, struct radv_image *dst_image,
                                     VkFormat dst_format, VkImageLayout dst_image_layout,
                                     const VkImageResolve2 *region);

void radv_meta_resolve_fragment_image(struct radv_cmd_buffer *cmd_buffer, struct radv_image *src_image,
                                      VkImageLayout src_image_layout, struct radv_image *dst_image,
                                      VkImageLayout dst_image_layout, const VkImageResolve2 *region);

void radv_decompress_resolve_rendering_src(struct radv_cmd_buffer *cmd_buffer);

void radv_decompress_resolve_src(struct radv_cmd_buffer *cmd_buffer, struct radv_image *src_image,
                                 VkImageLayout src_image_layout, const VkImageResolve2 *region);

uint32_t radv_clear_cmask(struct radv_cmd_buffer *cmd_buffer, struct radv_image *image,
                          const VkImageSubresourceRange *range, uint32_t value);
uint32_t radv_clear_fmask(struct radv_cmd_buffer *cmd_buffer, struct radv_image *image,
                          const VkImageSubresourceRange *range, uint32_t value);
uint32_t radv_clear_dcc(struct radv_cmd_buffer *cmd_buffer, struct radv_image *image,
                        const VkImageSubresourceRange *range, uint32_t value);
uint32_t radv_clear_htile(struct radv_cmd_buffer *cmd_buffer, const struct radv_image *image,
                          const VkImageSubresourceRange *range, uint32_t value);

void radv_update_buffer_cp(struct radv_cmd_buffer *cmd_buffer, uint64_t va, const void *data, uint64_t size);

void radv_meta_decode_etc(struct radv_cmd_buffer *cmd_buffer, struct radv_image *image, VkImageLayout layout,
                          const VkImageSubresourceLayers *subresource, VkOffset3D offset, VkExtent3D extent);
void radv_meta_decode_astc(struct radv_cmd_buffer *cmd_buffer, struct radv_image *image, VkImageLayout layout,
                           const VkImageSubresourceLayers *subresource, VkOffset3D offset, VkExtent3D extent);

/* common nir builder helpers */
#include "nir/nir_builder.h"

nir_builder PRINTFLIKE(3, 4)
   radv_meta_init_shader(struct radv_device *dev, gl_shader_stage stage, const char *name, ...);

nir_shader *radv_meta_build_nir_vs_generate_vertices(struct radv_device *dev);
nir_shader *radv_meta_build_nir_fs_noop(struct radv_device *dev);

void radv_meta_build_resolve_shader_core(struct radv_device *device, nir_builder *b, bool is_integer, int samples,
                                         nir_variable *input_img, nir_variable *color, nir_def *img_coord);

nir_def *radv_meta_load_descriptor(nir_builder *b, unsigned desc_set, unsigned binding);

nir_def *get_global_ids(nir_builder *b, unsigned num_components);

void radv_break_on_count(nir_builder *b, nir_variable *var, nir_def *count);

uint32_t radv_fill_buffer(struct radv_cmd_buffer *cmd_buffer, const struct radv_image *image,
                          struct radeon_winsys_bo *bo, uint64_t va, uint64_t size, uint32_t value);

void radv_copy_buffer(struct radv_cmd_buffer *cmd_buffer, struct radeon_winsys_bo *src_bo,
                      struct radeon_winsys_bo *dst_bo, uint64_t src_offset, uint64_t dst_offset, uint64_t size);

void radv_cmd_buffer_clear_attachment(struct radv_cmd_buffer *cmd_buffer, const VkClearAttachment *attachment);

void radv_cmd_buffer_clear_rendering(struct radv_cmd_buffer *cmd_buffer, const VkRenderingInfo *render_info);

void radv_cmd_buffer_resolve_rendering(struct radv_cmd_buffer *cmd_buffer);

void radv_cmd_buffer_resolve_rendering_cs(struct radv_cmd_buffer *cmd_buffer, struct radv_image_view *src_iview,
                                          VkImageLayout src_layout, struct radv_image_view *dst_iview,
                                          VkImageLayout dst_layout, const VkImageResolve2 *region);

void radv_depth_stencil_resolve_rendering_cs(struct radv_cmd_buffer *cmd_buffer, VkImageAspectFlags aspects,
                                             VkResolveModeFlagBits resolve_mode);

void radv_cmd_buffer_resolve_rendering_fs(struct radv_cmd_buffer *cmd_buffer, struct radv_image_view *src_iview,
                                          VkImageLayout src_layout, struct radv_image_view *dst_iview,
                                          VkImageLayout dst_layout);

void radv_depth_stencil_resolve_rendering_fs(struct radv_cmd_buffer *cmd_buffer, VkImageAspectFlags aspects,
                                             VkResolveModeFlagBits resolve_mode);

#ifdef __cplusplus
}
#endif

#endif /* RADV_META_H */
