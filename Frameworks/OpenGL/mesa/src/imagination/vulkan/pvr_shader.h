/*
 * Copyright © 2022 Imagination Technologies Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef PVR_SHADER_H
#define PVR_SHADER_H

#include <stddef.h>

#include "compiler/shader_enums.h"
#include "nir/nir.h"
#include "rogue/rogue.h"
#include "util/u_dynarray.h"
#include "vulkan/vulkan.h"

nir_shader *
pvr_spirv_to_nir(rogue_build_ctx *ctx,
                 gl_shader_stage stage,
                 const VkPipelineShaderStageCreateInfo *create_info);

rogue_shader *pvr_nir_to_rogue(rogue_build_ctx *ctx, nir_shader *nir);

void pvr_rogue_to_binary(rogue_build_ctx *ctx,
                         rogue_shader *shader,
                         struct util_dynarray *binary);

#endif /* PVR_SHADER_H */
