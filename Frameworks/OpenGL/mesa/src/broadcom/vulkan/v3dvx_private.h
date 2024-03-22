/*
 * Copyright © 2021 Raspberry Pi Ltd
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

/* This file generates the per-v3d-version function prototypes.  It must only
 * be included from v3dv_private.h.
 */

#ifndef V3DV_PRIVATE_H
#error This file is included by means other than v3dv_private.h
#endif

/* Used at v3dv_cmd_buffer */
void
v3dX(job_emit_binning_flush)(struct v3dv_job *job);

void
v3dX(cmd_buffer_emit_color_write_mask)(struct v3dv_cmd_buffer *cmd_buffer);

void
v3dX(cmd_buffer_end_render_pass_secondary)(struct v3dv_cmd_buffer *cmd_buffer);

void
v3dX(job_emit_clip_window)(struct v3dv_job *job, const VkRect2D *rect);

void
v3dX(cmd_buffer_emit_render_pass_rcl)(struct v3dv_cmd_buffer *cmd_buffer);

void
v3dX(cmd_buffer_emit_viewport)(struct v3dv_cmd_buffer *cmd_buffer);

void
v3dX(cmd_buffer_emit_stencil)(struct v3dv_cmd_buffer *cmd_buffer);

void
v3dX(cmd_buffer_emit_depth_bias)(struct v3dv_cmd_buffer *cmd_buffer);

void
v3dX(cmd_buffer_emit_depth_bounds)(struct v3dv_cmd_buffer *cmd_buffer);

void
v3dX(cmd_buffer_emit_line_width)(struct v3dv_cmd_buffer *cmd_buffer);

void
v3dX(cmd_buffer_emit_sample_state)(struct v3dv_cmd_buffer *cmd_buffer);

void
v3dX(cmd_buffer_emit_blend)(struct v3dv_cmd_buffer *cmd_buffer);

void
v3dX(cmd_buffer_emit_varyings_state)(struct v3dv_cmd_buffer *cmd_buffer);

void
v3dX(cmd_buffer_emit_configuration_bits)(struct v3dv_cmd_buffer *cmd_buffer);

void
v3dX(job_emit_binning_prolog)(struct v3dv_job *job,
                              const struct v3dv_frame_tiling *tiling,
                              uint32_t layers);

void
v3dX(job_emit_enable_double_buffer)(struct v3dv_job *job);

void
v3dX(cmd_buffer_execute_inside_pass)(struct v3dv_cmd_buffer *primary,
                                     uint32_t cmd_buffer_count,
                                     const VkCommandBuffer *cmd_buffers);

void
v3dX(cmd_buffer_emit_occlusion_query)(struct v3dv_cmd_buffer *cmd_buffer);

void
v3dX(cmd_buffer_emit_gl_shader_state)(struct v3dv_cmd_buffer *cmd_buffer);


void
v3dX(cmd_buffer_emit_draw)(struct v3dv_cmd_buffer *cmd_buffer,
                           struct v3dv_draw_info *info);


void
v3dX(cmd_buffer_emit_index_buffer)(struct v3dv_cmd_buffer *cmd_buffer);

void
v3dX(cmd_buffer_emit_draw_indexed)(struct v3dv_cmd_buffer *cmd_buffer,
                                   uint32_t indexCount,
                                   uint32_t instanceCount,
                                   uint32_t firstIndex,
                                   int32_t vertexOffset,
                                   uint32_t firstInstance);

void
v3dX(cmd_buffer_emit_draw_indirect)(struct v3dv_cmd_buffer *cmd_buffer,
                                    struct v3dv_buffer *buffer,
                                    VkDeviceSize offset,
                                    uint32_t drawCount,
                                    uint32_t stride);

void
v3dX(cmd_buffer_emit_indexed_indirect)(struct v3dv_cmd_buffer *cmd_buffer,
                                       struct v3dv_buffer *buffer,
                                       VkDeviceSize offset,
                                       uint32_t drawCount,
                                       uint32_t stride);

void
v3dX(get_hw_clear_color)(const VkClearColorValue *color,
                         uint32_t internal_type,
                         uint32_t internal_size,
                         uint32_t *hw_color);

/* Used at v3dv_device */

void
v3dX(pack_sampler_state)(const struct v3dv_device *device,
                         struct v3dv_sampler *sampler,
                         const VkSamplerCreateInfo *pCreateInfo,
                         const VkSamplerCustomBorderColorCreateInfoEXT *bc_info);

void
v3dX(framebuffer_compute_internal_bpp_msaa)(const struct v3dv_framebuffer *framebuffer,
                                            const struct v3dv_cmd_buffer_attachment_state *attachments,
                                            const struct v3dv_subpass *subpass,
                                            uint8_t *max_internal_bpp,
                                            uint8_t *total_color_bpp,
                                            bool *msaa);

#ifdef DEBUG
void
v3dX(device_check_prepacked_sizes)(void);
#endif

/* Used at v3dv_format */
const struct v3dv_format *
v3dX(get_format)(VkFormat);

void
v3dX(get_internal_type_bpp_for_output_format)(uint32_t format,
                                              uint32_t *type,
                                              uint32_t *bpp);

bool
v3dX(format_supports_tlb_resolve)(const struct v3dv_format *format);

bool
v3dX(format_supports_blending)(const struct v3dv_format *format);

/* FIXME: tex_format should be `enum V3DX(Texture_Data_Formats)`, but using
 * that enum type in the header requires including v3dx_pack.h, which triggers
 * circular include dependencies issues, so we're using a `uint32_t` for now.
 */
bool
v3dX(tfu_supports_tex_format)(uint32_t tex_format);

/* Used at v3dv_image */

void
v3dX(pack_texture_shader_state)(struct v3dv_device *device,
                                struct v3dv_image_view *iview);

void
v3dX(pack_texture_shader_state_from_buffer_view)(struct v3dv_device *device,
                                                 struct v3dv_buffer_view *buffer_view);

/* Used at v3dv_meta_* */

uint32_t
v3dX(zs_buffer_from_aspect_bits)(VkImageAspectFlags aspects);

uint8_t
v3dX(get_internal_depth_type)(VkFormat format);

struct v3dv_meta_framebuffer;

void
v3dX(meta_emit_copy_image_to_buffer_rcl)(struct v3dv_job *job,
                                         struct v3dv_buffer *buffer,
                                         struct v3dv_image *image,
                                         struct v3dv_meta_framebuffer *framebuffer,
                                         const VkBufferImageCopy2 *region);

void
v3dX(meta_emit_resolve_image_rcl)(struct v3dv_job *job,
                                  struct v3dv_image *dst,
                                  struct v3dv_image *src,
                                  struct v3dv_meta_framebuffer *framebuffer,
                                  const VkImageResolve2 *region);

void
v3dX(meta_emit_copy_buffer)(struct v3dv_job *job,
                            struct v3dv_bo *dst,
                            struct v3dv_bo *src,
                            uint32_t dst_offset,
                            uint32_t src_offset,
                            struct v3dv_meta_framebuffer *framebuffer,
                            uint32_t format,
                            uint32_t item_size);

void
v3dX(meta_emit_copy_buffer_rcl)(struct v3dv_job *job,
                                struct v3dv_bo *dst,
                                struct v3dv_bo *src,
                                uint32_t dst_offset,
                                uint32_t src_offset,
                                struct v3dv_meta_framebuffer *framebuffer,
                                uint32_t format,
                                uint32_t item_size);

void
v3dX(meta_emit_copy_image_rcl)(struct v3dv_job *job,
                               struct v3dv_image *dst,
                               struct v3dv_image *src,
                               struct v3dv_meta_framebuffer *framebuffer,
                               const VkImageCopy2 *region);

void
v3dX(meta_emit_tfu_job)(struct v3dv_cmd_buffer *cmd_buffer,
                        uint32_t dst_bo_handle,
                        uint32_t dst_offset,
                        enum v3d_tiling_mode dst_tiling,
                        uint32_t dst_padded_height_or_stride,
                        uint32_t dst_cpp,
                        uint32_t src_bo_handle,
                        uint32_t src_offset,
                        enum v3d_tiling_mode src_tiling,
                        uint32_t src_padded_height_or_stride,
                        uint32_t src_cpp,
                        uint32_t width,
                        uint32_t height,
                        const struct v3dv_format_plane *format_plane);

void
v3dX(meta_emit_clear_image_rcl)(struct v3dv_job *job,
                                struct v3dv_image *image,
                                struct v3dv_meta_framebuffer *framebuffer,
                                const union v3dv_clear_value *clear_value,
                                VkImageAspectFlags aspects,
                                uint32_t min_layer,
                                uint32_t max_layer,
                                uint32_t level);

void
v3dX(meta_emit_fill_buffer_rcl)(struct v3dv_job *job,
                                struct v3dv_bo *bo,
                                uint32_t offset,
                                struct v3dv_meta_framebuffer *framebuffer,
                                uint32_t data);

void
v3dX(meta_emit_copy_buffer_to_image_rcl)(struct v3dv_job *job,
                                         struct v3dv_image *image,
                                         struct v3dv_buffer *buffer,
                                         struct v3dv_meta_framebuffer *framebuffer,
                                         const VkBufferImageCopy2 *region);

void
v3dX(get_internal_type_bpp_for_image_aspects)(VkFormat vk_format,
                                              VkImageAspectFlags aspect_mask,
                                              uint32_t *internal_type,
                                              uint32_t *internal_bpp);

struct v3dv_job *
v3dX(meta_copy_buffer)(struct v3dv_cmd_buffer *cmd_buffer,
                       struct v3dv_bo *dst,
                       uint32_t dst_offset,
                       struct v3dv_bo *src,
                       uint32_t src_offset,
                       const VkBufferCopy2 *region);

void
v3dX(meta_fill_buffer)(struct v3dv_cmd_buffer *cmd_buffer,
                       struct v3dv_bo *bo,
                       uint32_t offset,
                       uint32_t size,
                       uint32_t data);

void
v3dX(meta_framebuffer_init)(struct v3dv_meta_framebuffer *fb,
                            VkFormat vk_format,
                            uint32_t internal_type,
                            const struct v3dv_frame_tiling *tiling);

/* Used at v3dv_pipeline */
void
v3dX(pipeline_pack_state)(struct v3dv_pipeline *pipeline,
                          const VkPipelineColorBlendStateCreateInfo *cb_info,
                          const VkPipelineDepthStencilStateCreateInfo *ds_info,
                          const VkPipelineRasterizationStateCreateInfo *rs_info,
                          const VkPipelineRasterizationProvokingVertexStateCreateInfoEXT *pv_info,
                          const VkPipelineRasterizationLineStateCreateInfoEXT *ls_info,
                          const VkPipelineMultisampleStateCreateInfo *ms_info);
void
v3dX(pipeline_pack_compile_state)(struct v3dv_pipeline *pipeline,
                                  const VkPipelineVertexInputStateCreateInfo *vi_info,
                                  const VkPipelineVertexInputDivisorStateCreateInfoEXT *vd_info);

bool
v3dX(pipeline_needs_default_attribute_values)(struct v3dv_pipeline *pipeline);

struct v3dv_bo *
v3dX(create_default_attribute_values)(struct v3dv_device *device,
                                      struct v3dv_pipeline *pipeline);

/* Used at v3dv_queue */
void
v3dX(job_emit_noop)(struct v3dv_job *job);

/* Used at v3dv_query */
VkResult
v3dX(enumerate_performance_query_counters)(uint32_t *pCounterCount,
                                           VkPerformanceCounterKHR *pCounters,
                                           VkPerformanceCounterDescriptionKHR *pCounterDescriptions);

/* Used at v3dv_descriptor_set, and other descriptor set utils */
uint32_t v3dX(descriptor_bo_size)(VkDescriptorType type);

uint32_t v3dX(max_descriptor_bo_size)(void);

uint32_t v3dX(combined_image_sampler_texture_state_offset)(uint8_t plane);

uint32_t v3dX(combined_image_sampler_sampler_state_offset)(uint8_t plane);

/* General utils */

uint32_t
v3dX(clamp_for_format_and_type)(uint32_t rt_type,
                                VkFormat vk_format);

#define V3D42_CLIPPER_XY_GRANULARITY 256.0f
#define V3D71_CLIPPER_XY_GRANULARITY 64.0f

uint32_t
v3dX(clamp_for_format_and_type)(uint32_t rt_type,
                                VkFormat vk_format);

void
v3dX(viewport_compute_xform)(const VkViewport *viewport,
                             float scale[3],
                             float translate[3]);
