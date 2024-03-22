/*
 * Copyright © 2023 Imagination Technologies Ltd.
 *
 * based in part on anv driver which is:
 * Copyright © 2015 Intel Corporation
 *
 * based in part on radv driver which is:
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
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

#ifndef PVR_COMMON_H
#define PVR_COMMON_H

#include <stdbool.h>
#include <stdint.h>
#include <vulkan/vulkan.h>

/* FIXME: Rename this, and ensure it only contains what's
 * relevant for the driver/compiler interface (no Vulkan types).
 */

#include "hwdef/rogue_hw_defs.h"
#include "pvr_limits.h"
#include "pvr_types.h"
#include "util/list.h"
#include "util/macros.h"
#include "vk_object.h"
#include "vk_sampler.h"
#include "vk_sync.h"

#define VK_VENDOR_ID_IMAGINATION 0x1010

#define PVR_WORKGROUP_DIMENSIONS 3U

#define PVR_SAMPLER_DESCRIPTOR_SIZE 4U
#define PVR_IMAGE_DESCRIPTOR_SIZE 4U

#define PVR_STATE_PBE_DWORDS 2U

#define PVR_PIPELINE_LAYOUT_SUPPORTED_DESCRIPTOR_TYPE_COUNT \
   (uint32_t)(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT + 1U)

#define PVR_TRANSFER_MAX_LAYERS 1U
#define PVR_TRANSFER_MAX_LOADS 4U
#define PVR_TRANSFER_MAX_IMAGES \
   (PVR_TRANSFER_MAX_LAYERS * PVR_TRANSFER_MAX_LOADS)

/* TODO: move into a common surface library? */
enum pvr_memlayout {
   PVR_MEMLAYOUT_UNDEFINED = 0, /* explicitly treat 0 as undefined */
   PVR_MEMLAYOUT_LINEAR,
   PVR_MEMLAYOUT_TWIDDLED,
   PVR_MEMLAYOUT_3DTWIDDLED,
};

enum pvr_texture_state {
   PVR_TEXTURE_STATE_SAMPLE,
   PVR_TEXTURE_STATE_STORAGE,
   PVR_TEXTURE_STATE_ATTACHMENT,
   PVR_TEXTURE_STATE_MAX_ENUM,
};

enum pvr_sub_cmd_type {
   PVR_SUB_CMD_TYPE_INVALID = 0, /* explicitly treat 0 as invalid */
   PVR_SUB_CMD_TYPE_GRAPHICS,
   PVR_SUB_CMD_TYPE_COMPUTE,
   PVR_SUB_CMD_TYPE_TRANSFER,
   PVR_SUB_CMD_TYPE_OCCLUSION_QUERY,
   PVR_SUB_CMD_TYPE_EVENT,
};

enum pvr_event_type {
   PVR_EVENT_TYPE_SET,
   PVR_EVENT_TYPE_RESET,
   PVR_EVENT_TYPE_WAIT,
   PVR_EVENT_TYPE_BARRIER,
};

enum pvr_depth_stencil_usage {
   PVR_DEPTH_STENCIL_USAGE_UNDEFINED = 0, /* explicitly treat 0 as undefined */
   PVR_DEPTH_STENCIL_USAGE_NEEDED,
   PVR_DEPTH_STENCIL_USAGE_NEVER,
};

enum pvr_job_type {
   PVR_JOB_TYPE_GEOM,
   PVR_JOB_TYPE_FRAG,
   PVR_JOB_TYPE_COMPUTE,
   PVR_JOB_TYPE_TRANSFER,
   PVR_JOB_TYPE_OCCLUSION_QUERY,
   PVR_JOB_TYPE_MAX
};

enum pvr_pipeline_type {
   PVR_PIPELINE_TYPE_INVALID = 0, /* explicitly treat 0 as undefined */
   PVR_PIPELINE_TYPE_GRAPHICS,
   PVR_PIPELINE_TYPE_COMPUTE,
};

enum pvr_pipeline_stage_bits {
   PVR_PIPELINE_STAGE_GEOM_BIT = BITFIELD_BIT(PVR_JOB_TYPE_GEOM),
   PVR_PIPELINE_STAGE_FRAG_BIT = BITFIELD_BIT(PVR_JOB_TYPE_FRAG),
   PVR_PIPELINE_STAGE_COMPUTE_BIT = BITFIELD_BIT(PVR_JOB_TYPE_COMPUTE),
   PVR_PIPELINE_STAGE_TRANSFER_BIT = BITFIELD_BIT(PVR_JOB_TYPE_TRANSFER),
   /* Note that this doesn't map to VkPipelineStageFlagBits so be careful with
    * this.
    */
   PVR_PIPELINE_STAGE_OCCLUSION_QUERY_BIT =
      BITFIELD_BIT(PVR_JOB_TYPE_OCCLUSION_QUERY),
};

#define PVR_PIPELINE_STAGE_ALL_GRAPHICS_BITS \
   (PVR_PIPELINE_STAGE_GEOM_BIT | PVR_PIPELINE_STAGE_FRAG_BIT)

#define PVR_PIPELINE_STAGE_ALL_BITS                                         \
   (PVR_PIPELINE_STAGE_ALL_GRAPHICS_BITS | PVR_PIPELINE_STAGE_COMPUTE_BIT | \
    PVR_PIPELINE_STAGE_TRANSFER_BIT)

#define PVR_NUM_SYNC_PIPELINE_STAGES 4U

/* Warning: Do not define an invalid stage as 0 since other code relies on 0
 * being the first shader stage. This allows for stages to be split or added
 * in the future. Defining 0 as invalid will very likely cause problems.
 */
enum pvr_stage_allocation {
   PVR_STAGE_ALLOCATION_VERTEX_GEOMETRY,
   PVR_STAGE_ALLOCATION_FRAGMENT,
   PVR_STAGE_ALLOCATION_COMPUTE,
   PVR_STAGE_ALLOCATION_COUNT
};

enum pvr_filter {
   PVR_FILTER_DONTCARE, /* Any filtering mode is acceptable. */
   PVR_FILTER_POINT,
   PVR_FILTER_LINEAR,
   PVR_FILTER_BICUBIC,
};

enum pvr_resolve_op {
   PVR_RESOLVE_BLEND,
   PVR_RESOLVE_MIN,
   PVR_RESOLVE_MAX,
   PVR_RESOLVE_SAMPLE0,
   PVR_RESOLVE_SAMPLE1,
   PVR_RESOLVE_SAMPLE2,
   PVR_RESOLVE_SAMPLE3,
   PVR_RESOLVE_SAMPLE4,
   PVR_RESOLVE_SAMPLE5,
   PVR_RESOLVE_SAMPLE6,
   PVR_RESOLVE_SAMPLE7,
};

enum pvr_event_state {
   PVR_EVENT_STATE_SET_BY_HOST,
   PVR_EVENT_STATE_RESET_BY_HOST,
   PVR_EVENT_STATE_SET_BY_DEVICE,
   PVR_EVENT_STATE_RESET_BY_DEVICE
};

enum pvr_deferred_cs_command_type {
   PVR_DEFERRED_CS_COMMAND_TYPE_DBSC,
   PVR_DEFERRED_CS_COMMAND_TYPE_DBSC2,
};

enum pvr_query_type {
   PVR_QUERY_TYPE_AVAILABILITY_WRITE,
   PVR_QUERY_TYPE_RESET_QUERY_POOL,
   PVR_QUERY_TYPE_COPY_QUERY_RESULTS,
};

union pvr_sampler_descriptor {
   uint32_t words[PVR_SAMPLER_DESCRIPTOR_SIZE];

   struct {
      /* Packed PVRX(TEXSTATE_SAMPLER). */
      uint64_t sampler_word;
      uint32_t compare_op;
      /* TODO: Figure out what this word is for and rename.
       * Sampler state word 1?
       */
      uint32_t word3;
   } data;
};

struct pvr_combined_image_sampler_descriptor {
   /* | TEXSTATE_IMAGE_WORD0 | TEXSTATE_{STRIDE_,}IMAGE_WORD1 | */
   uint64_t image[ROGUE_NUM_TEXSTATE_IMAGE_WORDS];
   union pvr_sampler_descriptor sampler;
};

#define CHECK_STRUCT_FIELD_SIZE(_struct_type, _field_name, _size)      \
   static_assert(sizeof(((struct _struct_type *)NULL)->_field_name) == \
                    (_size),                                           \
                 "Size of '" #_field_name "' in '" #_struct_type       \
                 "' differs from expected")

CHECK_STRUCT_FIELD_SIZE(pvr_combined_image_sampler_descriptor,
                        image,
                        ROGUE_NUM_TEXSTATE_IMAGE_WORDS * sizeof(uint64_t));
CHECK_STRUCT_FIELD_SIZE(pvr_combined_image_sampler_descriptor,
                        image,
                        PVR_DW_TO_BYTES(PVR_IMAGE_DESCRIPTOR_SIZE));
#if 0
/* TODO: Don't really want to include pvr_csb.h in here since this header is
 * shared with the compiler. Figure out a better place for these.
 */
CHECK_STRUCT_FIELD_SIZE(pvr_combined_image_sampler_descriptor,
                        image,
                        (pvr_cmd_length(TEXSTATE_IMAGE_WORD0) +
                         pvr_cmd_length(TEXSTATE_IMAGE_WORD1)) *
                           sizeof(uint32_t));
CHECK_STRUCT_FIELD_SIZE(pvr_combined_image_sampler_descriptor,
                        image,
                        (pvr_cmd_length(TEXSTATE_IMAGE_WORD0) +
                         pvr_cmd_length(TEXSTATE_STRIDE_IMAGE_WORD1)) *
                           sizeof(uint32_t));
#endif

#undef CHECK_STRUCT_FIELD_SIZE

struct pvr_sampler {
   struct vk_sampler vk;

   union pvr_sampler_descriptor descriptor;
};

struct pvr_descriptor_size_info {
   /* Non-spillable size for storage in the common store. */
   uint32_t primary;

   /* Spillable size to accommodate limitation of the common store. */
   uint32_t secondary;

   uint32_t alignment;
};

struct pvr_descriptor_set_layout_binding {
   VkDescriptorType type;

   /* "M" in layout(set = N, binding = M)
    * Can be used to index bindings in the descriptor_set_layout.
    */
   uint32_t binding_number;

   uint32_t descriptor_count;

   /* Index into the flattened descriptor set */
   uint16_t descriptor_index;

   /* Mask of enum pvr_stage_allocation. */
   uint8_t shader_stage_mask;

   struct {
      uint32_t primary;
      uint32_t secondary;
   } per_stage_offset_in_dwords[PVR_STAGE_ALLOCATION_COUNT];

   bool has_immutable_samplers;
   /* Index at which the samplers can be found in the descriptor_set_layout.
    * 0 when the samplers are at index 0 or no samplers are present.
    */
   uint32_t immutable_samplers_index;
};

/* All sizes are in dwords. */
struct pvr_descriptor_set_layout_mem_layout {
   uint32_t primary_offset;
   uint32_t primary_size;

   uint32_t secondary_offset;
   uint32_t secondary_size;

   uint32_t primary_dynamic_size;
   uint32_t secondary_dynamic_size;
};

struct pvr_descriptor_set_layout {
   struct vk_object_base base;

   /* Total amount of descriptors contained in this set. */
   uint32_t descriptor_count;

   /* Count of dynamic buffers. */
   uint32_t dynamic_buffer_count;
   uint32_t total_dynamic_size_in_dwords;

   uint32_t binding_count;
   struct pvr_descriptor_set_layout_binding *bindings;

   uint32_t immutable_sampler_count;
   const struct pvr_sampler **immutable_samplers;

   /* Shader stages requiring access to descriptors in this set. */
   /* Mask of enum pvr_stage_allocation. */
   uint8_t shader_stage_mask;

   /* Count of each VkDescriptorType per shader stage. Dynamically allocated
    * arrays per stage as to not hard code the max descriptor type here.
    *
    * Note: when adding a new type, it might not numerically follow the
    * previous type so a sparse array will be created. You might want to
    * readjust how these arrays are created and accessed.
    */
   uint32_t *per_stage_descriptor_count[PVR_STAGE_ALLOCATION_COUNT];

   uint32_t total_size_in_dwords;
   struct pvr_descriptor_set_layout_mem_layout
      memory_layout_in_dwords_per_stage[PVR_STAGE_ALLOCATION_COUNT];
};

struct pvr_descriptor_pool {
   struct vk_object_base base;

   VkAllocationCallbacks alloc;

   /* Saved information from pCreateInfo. */
   uint32_t max_sets;

   uint32_t total_size_in_dwords;
   uint32_t current_size_in_dwords;

   /* Derived and other state. */
   /* List of the descriptor sets created using this pool. */
   struct list_head descriptor_sets;
};

struct pvr_descriptor {
   VkDescriptorType type;

   union {
      struct {
         struct pvr_buffer_view *bview;
         pvr_dev_addr_t buffer_dev_addr;
         VkDeviceSize buffer_desc_range;
         VkDeviceSize buffer_whole_range;
      };

      struct {
         VkImageLayout layout;
         const struct pvr_image_view *iview;
         const struct pvr_sampler *sampler;
      };
   };
};

struct pvr_descriptor_set {
   struct vk_object_base base;

   const struct pvr_descriptor_set_layout *layout;
   const struct pvr_descriptor_pool *pool;

   struct pvr_suballoc_bo *pvr_bo;

   /* Links this descriptor set into pvr_descriptor_pool::descriptor_sets list.
    */
   struct list_head link;

   /* Array of size layout::descriptor_count. */
   struct pvr_descriptor descriptors[0];
};

struct pvr_event {
   struct vk_object_base base;

   enum pvr_event_state state;
   struct vk_sync *sync;
};

#define PVR_MAX_DYNAMIC_BUFFERS                      \
   (PVR_MAX_DESCRIPTOR_SET_UNIFORM_DYNAMIC_BUFFERS + \
    PVR_MAX_DESCRIPTOR_SET_STORAGE_DYNAMIC_BUFFERS)

struct pvr_descriptor_state {
   struct pvr_descriptor_set *descriptor_sets[PVR_MAX_DESCRIPTOR_SETS];
   uint32_t valid_mask;

   uint32_t dynamic_offsets[PVR_MAX_DYNAMIC_BUFFERS];
};

#undef PVR_MAX_DYNAMIC_BUFFERS

/**
 * \brief Indicates the layout of shared registers allocated by the driver.
 *
 * 'present' fields indicate if a certain resource was allocated for, and
 * whether it will be present in the shareds.
 * 'offset' fields indicate at which shared reg the resource starts at.
 */
struct pvr_sh_reg_layout {
   /* If this is present, it will always take up 2 sh regs in size and contain
    * the device address of the descriptor set addrs table.
    */
   struct {
      bool present;
      uint32_t offset;
   } descriptor_set_addrs_table;

   /* If this is present, it will always take up 2 sh regs in size and contain
    * the device address of the push constants buffer.
    */
   struct {
      bool present;
      uint32_t offset;
   } push_consts;

   /* If this is present, it will always take up 2 sh regs in size and contain
    * the device address of the blend constants buffer.
    */
   struct {
      bool present;
      uint32_t offset;
   } blend_consts;
};

struct pvr_pipeline_layout {
   struct vk_object_base base;

   uint32_t set_count;
   /* Contains set_count amount of descriptor set layouts. */
   struct pvr_descriptor_set_layout *set_layout[PVR_MAX_DESCRIPTOR_SETS];

   /* Mask of enum pvr_stage_allocation. */
   uint8_t push_constants_shader_stages;
   uint32_t vert_push_constants_offset;
   uint32_t frag_push_constants_offset;
   uint32_t compute_push_constants_offset;

   /* Mask of enum pvr_stage_allocation. */
   uint8_t shader_stage_mask;

   /* Per stage masks indicating which set in the layout contains any
    * descriptor of the appropriate types: VK..._{SAMPLER, SAMPLED_IMAGE,
    * UNIFORM_TEXEL_BUFFER, UNIFORM_BUFFER, STORAGE_BUFFER}.
    * Shift by the set's number to check the mask (1U << set_num).
    */
   uint32_t per_stage_descriptor_masks[PVR_STAGE_ALLOCATION_COUNT];

   /* Array of descriptor offsets at which the set's descriptors' start, per
    * stage, within all the sets in the pipeline layout per descriptor type.
    * Note that we only store into for specific descriptor types
    * VK_DESCRIPTOR_TYPE_{SAMPLER, SAMPLED_IMAGE, UNIFORM_TEXEL_BUFFER,
    * UNIFORM_BUFFER, STORAGE_BUFFER}, the rest will be 0.
    */
   uint32_t
      descriptor_offsets[PVR_MAX_DESCRIPTOR_SETS][PVR_STAGE_ALLOCATION_COUNT]
                        [PVR_PIPELINE_LAYOUT_SUPPORTED_DESCRIPTOR_TYPE_COUNT];

   /* There is no accounting for dynamics in here. They will be garbage values.
    */
   struct pvr_descriptor_set_layout_mem_layout
      register_layout_in_dwords_per_stage[PVR_STAGE_ALLOCATION_COUNT]
                                         [PVR_MAX_DESCRIPTOR_SETS];

   /* TODO: Consider whether this needs to be here. */
   struct pvr_sh_reg_layout sh_reg_layout_per_stage[PVR_STAGE_ALLOCATION_COUNT];

   /* All sizes in dwords. */
   struct pvr_pipeline_layout_reg_info {
      uint32_t primary_dynamic_size_in_dwords;
      uint32_t secondary_dynamic_size_in_dwords;
   } per_stage_reg_info[PVR_STAGE_ALLOCATION_COUNT];
};

static int pvr_compare_layout_binding(const void *a, const void *b)
{
   uint32_t binding_a;
   uint32_t binding_b;

   binding_a = ((struct pvr_descriptor_set_layout_binding *)a)->binding_number;
   binding_b = ((struct pvr_descriptor_set_layout_binding *)b)->binding_number;

   if (binding_a < binding_b)
      return -1;

   if (binding_a > binding_b)
      return 1;

   return 0;
}

/* This function does not assume that the binding will always exist for a
 * particular binding_num. Caller should check before using the return pointer.
 */
static struct pvr_descriptor_set_layout_binding *
pvr_get_descriptor_binding(const struct pvr_descriptor_set_layout *layout,
                           const uint32_t binding_num)
{
   struct pvr_descriptor_set_layout_binding binding;
   binding.binding_number = binding_num;

   return bsearch(&binding,
                  layout->bindings,
                  layout->binding_count,
                  sizeof(binding),
                  pvr_compare_layout_binding);
}

#endif /* PVR_COMMON_H */
