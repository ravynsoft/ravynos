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

#include <stdio.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>

#include "compiler/shader_enums.h"
#include "nir/nir.h"
#include "nir/nir_builder.h"
#include "pvr_private.h"
#include "pvr_shader.h"
#include "rogue/rogue.h"
#include "spirv/nir_spirv.h"
#include "vk_format.h"
#include "vk_shader_module.h"
#include "vk_util.h"

/**
 * \file pvr_shader.c
 *
 * \brief Contains top-level functions to compile SPIR-V -> NIR -> Rogue, and
 * interfaces with the compiler.
 */

/**
 * \brief Converts a SPIR-V shader to NIR.
 *
 * \param[in] ctx Shared multi-stage build context.
 * \param[in] stage Shader stage.
 * \param[in] create_info Shader creation info from Vulkan pipeline.
 * \return A nir_shader* if successful, or NULL if unsuccessful.
 */
nir_shader *pvr_spirv_to_nir(rogue_build_ctx *ctx,
                             gl_shader_stage stage,
                             const VkPipelineShaderStageCreateInfo *create_info)
{
   VK_FROM_HANDLE(vk_shader_module, module, create_info->module);
   struct nir_spirv_specialization *spec;
   unsigned num_spec = 0;
   nir_shader *nir;

   spec =
      vk_spec_info_to_nir_spirv(create_info->pSpecializationInfo, &num_spec);

   nir = rogue_spirv_to_nir(ctx,
                            stage,
                            create_info->pName,
                            module->size / sizeof(uint32_t),
                            (uint32_t *)module->data,
                            num_spec,
                            spec);

   free(spec);

   return nir;
}

/**
 * \brief Converts a NIR shader to Rogue.
 *
 * \param[in] ctx Shared multi-stage build context.
 * \param[in] nir NIR shader.
 * \return A rogue_shader* if successful, or NULL if unsuccessful.
 */
rogue_shader *pvr_nir_to_rogue(rogue_build_ctx *ctx, nir_shader *nir)
{
   return rogue_nir_to_rogue(ctx, nir);
}

/**
 * \brief Converts a Rogue shader to binary.
 *
 * \param[in] ctx Shared multi-stage build context.
 * \param[in] shader Rogue shader.
 * \param[out] binary Array containing shader binary.
 */
void pvr_rogue_to_binary(rogue_build_ctx *ctx,
                         rogue_shader *shader,
                         struct util_dynarray *binary)
{
   rogue_encode_shader(ctx, shader, binary);
}
