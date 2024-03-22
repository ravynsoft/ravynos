/*
 * Copyright (C) 2021 Collabora Ltd.
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

#ifndef PANVK_PRIVATE_H
#error "Must be included from panvk_private.h"
#endif

#ifndef PAN_ARCH
#error "no arch"
#endif

#include "compiler/shader_enums.h"
#include <vulkan/vulkan.h>

void panvk_per_arch(emit_varying)(const struct panvk_device *dev,
                                  const struct panvk_varyings_info *varyings,
                                  gl_shader_stage stage, unsigned idx,
                                  void *attrib);

void panvk_per_arch(emit_varyings)(const struct panvk_device *dev,
                                   const struct panvk_varyings_info *varyings,
                                   gl_shader_stage stage, void *descs);

void
   panvk_per_arch(emit_varying_bufs)(const struct panvk_varyings_info *varyings,
                                     void *descs);

void panvk_per_arch(emit_attrib_bufs)(const struct panvk_attribs_info *info,
                                      const struct panvk_attrib_buf *bufs,
                                      unsigned buf_count,
                                      const struct panvk_draw_info *draw,
                                      void *descs);

void panvk_per_arch(emit_attribs)(const struct panvk_device *dev,
                                  const struct panvk_draw_info *draw,
                                  const struct panvk_attribs_info *attribs,
                                  const struct panvk_attrib_buf *bufs,
                                  unsigned buf_count, void *descs);

void panvk_per_arch(emit_ubo)(mali_ptr address, size_t size, void *desc);

void panvk_per_arch(emit_ubos)(const struct panvk_pipeline *pipeline,
                               const struct panvk_descriptor_state *state,
                               void *descs);

void panvk_per_arch(emit_sampler)(const VkSamplerCreateInfo *pCreateInfo,
                                  void *desc);

void panvk_per_arch(emit_vertex_job)(const struct panvk_pipeline *pipeline,
                                     const struct panvk_draw_info *draw,
                                     void *job);

void
   panvk_per_arch(emit_compute_job)(const struct panvk_pipeline *pipeline,
                                    const struct panvk_dispatch_info *dispatch,
                                    void *job);

void panvk_per_arch(emit_tiler_job)(const struct panvk_pipeline *pipeline,
                                    const struct panvk_draw_info *draw,
                                    void *job);

void panvk_per_arch(emit_viewport)(const VkViewport *viewport,
                                   const VkRect2D *scissor, void *vpd);

void panvk_per_arch(emit_blend)(const struct panvk_device *dev,
                                const struct panvk_pipeline *pipeline,
                                unsigned rt, void *bd);

void panvk_per_arch(emit_blend_constant)(const struct panvk_device *dev,
                                         const struct panvk_pipeline *pipeline,
                                         unsigned rt, const float *constants,
                                         void *bd);

void panvk_per_arch(emit_dyn_fs_rsd)(const struct panvk_pipeline *pipeline,
                                     const struct panvk_cmd_state *state,
                                     void *rsd);

void panvk_per_arch(emit_base_fs_rsd)(const struct panvk_device *dev,
                                      const struct panvk_pipeline *pipeline,
                                      void *rsd);

void panvk_per_arch(emit_non_fs_rsd)(const struct panvk_device *dev,
                                     const struct pan_shader_info *shader_info,
                                     mali_ptr shader_ptr, void *rsd);

void panvk_per_arch(emit_tiler_context)(const struct panvk_device *dev,
                                        unsigned width, unsigned height,
                                        const struct panfrost_ptr *descs);
