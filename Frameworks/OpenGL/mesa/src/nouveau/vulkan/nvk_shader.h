/*
 * Copyright © 2022 Collabora Ltd. and Red Hat Inc.
 * SPDX-License-Identifier: MIT
 */
#ifndef NVK_SHADER_H
#define NVK_SHADER_H 1

#include "nvk_private.h"
#include "nvk_device_memory.h"

#include "vk_pipeline_cache.h"

#include "nak.h"
#include "nir.h"
#include "nouveau_bo.h"

struct nak_shader_bin;
struct nvk_device;
struct nvk_physical_device;
struct nvk_pipeline_compilation_ctx;
struct vk_pipeline_cache;
struct vk_pipeline_layout;
struct vk_pipeline_robustness_state;
struct vk_shader_module;

#define GF100_SHADER_HEADER_SIZE (20 * 4)
#define TU102_SHADER_HEADER_SIZE (32 * 4)
#define NVC0_MAX_SHADER_HEADER_SIZE TU102_SHADER_HEADER_SIZE

enum ENUM_PACKED nvk_cbuf_type {
   NVK_CBUF_TYPE_INVALID = 0,
   NVK_CBUF_TYPE_ROOT_DESC,
   NVK_CBUF_TYPE_DESC_SET,
   NVK_CBUF_TYPE_DYNAMIC_UBO,
   NVK_CBUF_TYPE_UBO_DESC,
};

struct nvk_cbuf {
   enum nvk_cbuf_type type;
   uint8_t desc_set;
   uint8_t dynamic_idx;
   uint32_t desc_offset;
};

struct nvk_cbuf_map {
   uint32_t cbuf_count;
   struct nvk_cbuf cbufs[16];
};

struct nvk_shader {
   struct vk_pipeline_cache_object base;

   struct nak_shader_info info;
   struct nvk_cbuf_map cbuf_map;

   struct nak_shader_bin *nak;
   const void *code_ptr;
   uint32_t code_size;

   uint32_t upload_size;
   uint64_t upload_addr;
   uint32_t upload_padding;
};

static inline uint64_t
nvk_shader_address(const struct nvk_shader *shader)
{
   return shader->upload_addr + shader->upload_padding;
}

static inline bool
nvk_shader_is_enabled(const struct nvk_shader *shader)
{
   return shader && shader->upload_size > 0;
}

VkShaderStageFlags nvk_nak_stages(const struct nv_device_info *info);

uint64_t
nvk_physical_device_compiler_flags(const struct nvk_physical_device *pdev);

const nir_shader_compiler_options *
nvk_physical_device_nir_options(const struct nvk_physical_device *pdev,
                                gl_shader_stage stage);

static inline nir_address_format
nvk_buffer_addr_format(VkPipelineRobustnessBufferBehaviorEXT robustness)
{
   switch (robustness) {
   case VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_DISABLED_EXT:
      return nir_address_format_64bit_global_32bit_offset;
   case VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_EXT:
   case VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_2_EXT:
      return nir_address_format_64bit_bounded_global;
   default:
      unreachable("Invalid robust buffer access behavior");
   }
}

struct spirv_to_nir_options
nvk_physical_device_spirv_options(const struct nvk_physical_device *pdev,
                                  const struct vk_pipeline_robustness_state *rs);

bool
nvk_nir_lower_descriptors(nir_shader *nir,
                          const struct vk_pipeline_robustness_state *rs,
                          const struct vk_pipeline_layout *layout,
                          struct nvk_cbuf_map *cbuf_map_out);

VkResult
nvk_shader_stage_to_nir(struct nvk_device *dev,
                        const VkPipelineShaderStageCreateInfo *sinfo,
                        const struct vk_pipeline_robustness_state *rstate,
                        struct vk_pipeline_cache *cache,
                        void *mem_ctx, struct nir_shader **nir_out);

void
nvk_lower_nir(struct nvk_device *dev, nir_shader *nir,
              const struct vk_pipeline_robustness_state *rs,
              bool is_multiview,
              const struct vk_pipeline_layout *layout,
              struct nvk_shader *shader);

VkResult
nvk_compile_nir(struct nvk_device *dev, nir_shader *nir,
                VkPipelineCreateFlagBits2KHR pipeline_flags,
                const struct vk_pipeline_robustness_state *rstate,
                const struct nak_fs_key *fs_key,
                struct vk_pipeline_cache *cache,
                struct nvk_shader *shader);

VkResult
nvk_shader_upload(struct nvk_device *dev, struct nvk_shader *shader);

struct nvk_shader *
nvk_shader_init(struct nvk_device *dev, const void *key_data, size_t key_size);

extern const struct vk_pipeline_cache_object_ops nvk_shader_ops;

void
nvk_shader_finish(struct nvk_device *dev, struct nvk_shader *shader);

void
nvk_hash_shader(unsigned char *hash,
                const VkPipelineShaderStageCreateInfo *sinfo,
                const struct vk_pipeline_robustness_state *rstate,
                bool is_multiview,
                const struct vk_pipeline_layout *layout,
                const struct nak_fs_key *fs_key);

void
nvk_shader_destroy(struct vk_device *dev,
                   struct vk_pipeline_cache_object *object);

/* Codegen wrappers.
 *
 * TODO: Delete these once NAK supports everything.
 */
uint64_t nvk_cg_get_prog_debug(void);
uint64_t nvk_cg_get_prog_optimize(void);

const nir_shader_compiler_options *
nvk_cg_nir_options(const struct nvk_physical_device *pdev,
                   gl_shader_stage stage);

void nvk_cg_preprocess_nir(nir_shader *nir);
void nvk_cg_optimize_nir(nir_shader *nir);

VkResult nvk_cg_compile_nir(struct nvk_physical_device *pdev, nir_shader *nir,
                            const struct nak_fs_key *fs_key,
                            struct nvk_shader *shader);

#endif
