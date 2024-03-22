/*
 * Copyright © 2017 Intel Corporation
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

#include "vk_shader_module.h"

#include "vk_alloc.h"
#include "vk_common_entrypoints.h"
#include "vk_device.h"
#include "vk_log.h"
#include "vk_nir.h"
#include "vk_pipeline.h"
#include "vk_util.h"

void vk_shader_module_init(struct vk_device *device,
                           struct vk_shader_module *module,
                           const VkShaderModuleCreateInfo *create_info)
{
   vk_object_base_init(device, &module->base, VK_OBJECT_TYPE_SHADER_MODULE);

   module->nir = NULL;

   module->size = create_info->codeSize;
   memcpy(module->data, create_info->pCode, module->size);

   _mesa_blake3_compute(module->data, module->size, module->hash);
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_common_CreateShaderModule(VkDevice _device,
                             const VkShaderModuleCreateInfo *pCreateInfo,
                             const VkAllocationCallbacks *pAllocator,
                             VkShaderModule *pShaderModule)
{
    VK_FROM_HANDLE(vk_device, device, _device);
    struct vk_shader_module *module;

    assert(pCreateInfo->sType == VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO);
    assert(pCreateInfo->flags == 0);

    module = vk_alloc2(&device->alloc, pAllocator,
                       sizeof(*module) + pCreateInfo->codeSize, 8,
                       VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
    if (module == NULL)
       return VK_ERROR_OUT_OF_HOST_MEMORY;

    vk_shader_module_init(device, module, pCreateInfo);

    *pShaderModule = vk_shader_module_to_handle(module);

    return VK_SUCCESS;
}

const uint8_t vk_shaderModuleIdentifierAlgorithmUUID[VK_UUID_SIZE] = "MESA-BLAKE3";

VKAPI_ATTR void VKAPI_CALL
vk_common_GetShaderModuleIdentifierEXT(VkDevice _device,
                                       VkShaderModule _module,
                                       VkShaderModuleIdentifierEXT *pIdentifier)
{
   VK_FROM_HANDLE(vk_shader_module, module, _module);
   memcpy(pIdentifier->identifier, module->hash, sizeof(module->hash));
   pIdentifier->identifierSize = sizeof(module->hash);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_GetShaderModuleCreateInfoIdentifierEXT(VkDevice _device,
                                                 const VkShaderModuleCreateInfo *pCreateInfo,
                                                 VkShaderModuleIdentifierEXT *pIdentifier)
{
   _mesa_blake3_compute(pCreateInfo->pCode, pCreateInfo->codeSize,
                        pIdentifier->identifier);
   pIdentifier->identifierSize = sizeof(blake3_hash);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_DestroyShaderModule(VkDevice _device,
                              VkShaderModule _module,
                              const VkAllocationCallbacks *pAllocator)
{
   VK_FROM_HANDLE(vk_device, device, _device);
   VK_FROM_HANDLE(vk_shader_module, module, _module);

   if (!module)
      return;

   /* NIR modules (which are only created internally by the driver) are not
    * dynamically allocated so we should never call this for them.
    * Instead the driver is responsible for freeing the NIR code when it is
    * no longer needed.
    */
   assert(module->nir == NULL);

   vk_object_free(device, pAllocator, module);
}

#define SPIR_V_MAGIC_NUMBER 0x07230203

uint32_t
vk_shader_module_spirv_version(const struct vk_shader_module *mod)
{
   if (mod->nir != NULL)
      return 0;

   return vk_spirv_version((uint32_t *)mod->data, mod->size);
}

VkResult
vk_shader_module_to_nir(struct vk_device *device,
                        const struct vk_shader_module *mod,
                        gl_shader_stage stage,
                        const char *entrypoint_name,
                        const VkSpecializationInfo *spec_info,
                        const struct spirv_to_nir_options *spirv_options,
                        const nir_shader_compiler_options *nir_options,
                        void *mem_ctx, nir_shader **nir_out)
{
   const VkPipelineShaderStageCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = mesa_to_vk_shader_stage(stage),
      .module = vk_shader_module_to_handle((struct vk_shader_module *)mod),
      .pName = entrypoint_name,
      .pSpecializationInfo = spec_info,
   };
   return vk_pipeline_shader_stage_to_nir(device, &info,
                                          spirv_options, nir_options,
                                          mem_ctx, nir_out);
}
