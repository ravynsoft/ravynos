/*
 * Copyright © 2022 Collabora, LTD
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

#ifndef VK_NIR_H
#define VK_NIR_H

#include "nir.h"
#include "vulkan/vulkan_core.h"

struct spirv_to_nir_options;
struct vk_device;

#ifdef __cplusplus
extern "C" {
#endif

uint32_t vk_spirv_version(const uint32_t *spirv_data, size_t spirv_size_B);

bool
nir_vk_is_not_xfb_output(nir_variable *var, void *data);

nir_shader *
vk_spirv_to_nir(struct vk_device *device,
                const uint32_t *spirv_data, size_t spirv_size_B,
                gl_shader_stage stage, const char *entrypoint_name,
                enum gl_subgroup_size subgroup_size,
                const VkSpecializationInfo *spec_info,
                const struct spirv_to_nir_options *spirv_options,
                const struct nir_shader_compiler_options *nir_options,
                bool internal,
                void *mem_ctx);

#ifdef __cplusplus
}
#endif

#endif /* VK_NIR_H */
