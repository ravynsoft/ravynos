/*
 * Copyright © 2020 Mike Blumenkrantz
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
 * 
 * Authors:
 *    Mike Blumenkrantz <michael.blumenkrantz@gmail.com>
 */

#ifndef ZINK_DESCRIPTOR_H
# define  ZINK_DESCRIPTOR_H

#include "zink_types.h"
#ifdef __cplusplus
extern "C" {
#endif

#define ZINK_DESCRIPTOR_COMPACT 2


#define ZINK_BINDLESS_IS_BUFFER(HANDLE) (HANDLE >= ZINK_MAX_BINDLESS_HANDLES)

static inline enum zink_descriptor_size_index
zink_vktype_to_size_idx(VkDescriptorType type)
{
   switch (type) {
   case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
   case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
      return ZDS_INDEX_UBO;
   case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
   case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
      return ZDS_INDEX_COMBINED_SAMPLER;
   case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
      return ZDS_INDEX_UNIFORM_TEXELS;
   case VK_DESCRIPTOR_TYPE_SAMPLER:
      return ZDS_INDEX_SAMPLER;
   case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
      return ZDS_INDEX_STORAGE_BUFFER;
   case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
      return ZDS_INDEX_STORAGE_IMAGE;
   case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
      return ZDS_INDEX_STORAGE_TEXELS;
   default: break;
   }
   unreachable("unknown type");
}

static inline enum zink_descriptor_size_index_compact
zink_vktype_to_size_idx_comp(VkDescriptorType type)
{
   switch (type) {
   case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
   case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
      return ZDS_INDEX_COMP_UBO;
   case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
   case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
      return ZDS_INDEX_COMP_COMBINED_SAMPLER;
   case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
      return ZDS_INDEX_COMP_UNIFORM_TEXELS;
   case VK_DESCRIPTOR_TYPE_SAMPLER:
      return ZDS_INDEX_COMP_SAMPLER;
   case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
      return ZDS_INDEX_COMP_STORAGE_BUFFER;
   case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
      return ZDS_INDEX_COMP_STORAGE_IMAGE;
   case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
      return ZDS_INDEX_COMP_STORAGE_TEXELS;
   default: break;
   }
   unreachable("unknown type");
}

static inline enum zink_descriptor_size_index
zink_descriptor_type_to_size_idx(enum zink_descriptor_type type)
{
   switch (type) {
   case ZINK_DESCRIPTOR_TYPE_UBO:
      return ZDS_INDEX_UBO;
   case ZINK_DESCRIPTOR_TYPE_SAMPLER_VIEW:
      return ZDS_INDEX_COMBINED_SAMPLER;
   case ZINK_DESCRIPTOR_TYPE_SSBO:
      return ZDS_INDEX_STORAGE_BUFFER;
   case ZINK_DESCRIPTOR_TYPE_IMAGE:
      return ZDS_INDEX_STORAGE_IMAGE;
   default: break;
   }
   unreachable("unknown type");
}

static inline enum zink_descriptor_size_index_compact
zink_descriptor_type_to_size_idx_comp(enum zink_descriptor_type type)
{
   switch (type) {
   case ZINK_DESCRIPTOR_TYPE_UBO:
      return ZDS_INDEX_COMP_UBO;
   case ZINK_DESCRIPTOR_TYPE_SAMPLER_VIEW:
      return ZDS_INDEX_COMP_COMBINED_SAMPLER;
   case ZINK_DESCRIPTOR_TYPE_SSBO:
   case ZINK_DESCRIPTOR_TYPE_IMAGE:
   default: break;
   }
   unreachable("unknown type");
}

/* bindless descriptor bindings have their own struct indexing */
ALWAYS_INLINE static VkDescriptorType
zink_descriptor_type_from_bindless_index(unsigned idx)
{
   switch (idx) {
   case 0: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
   case 1: return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
   case 2: return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
   case 3: return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
   default:
      unreachable("unknown index");
   }
}

bool
zink_descriptor_layouts_init(struct zink_screen *screen);

void
zink_descriptor_layouts_deinit(struct zink_screen *screen);

bool
zink_descriptor_util_alloc_sets(struct zink_screen *screen, VkDescriptorSetLayout dsl, VkDescriptorPool pool, VkDescriptorSet *sets, unsigned num_sets);
void
zink_descriptor_util_init_fbfetch(struct zink_context *ctx);
bool
zink_descriptor_util_push_layouts_get(struct zink_context *ctx, struct zink_descriptor_layout **dsls, struct zink_descriptor_layout_key **layout_keys);
VkImageLayout
zink_descriptor_util_image_layout_eval(const struct zink_context *ctx, const struct zink_resource *res, bool is_compute);
void
zink_descriptors_init_bindless(struct zink_context *ctx);
void
zink_descriptors_deinit_bindless(struct zink_context *ctx);
void
zink_descriptors_update_bindless(struct zink_context *ctx);

void
zink_descriptor_shader_get_binding_offsets(const struct zink_shader *shader, unsigned *offsets);
void
zink_descriptor_shader_init(struct zink_screen *screen, struct zink_shader *shader);
void
zink_descriptor_shader_deinit(struct zink_screen *screen, struct zink_shader *shader);

bool
zink_descriptor_program_init(struct zink_context *ctx, struct zink_program *pg);

void
zink_descriptor_program_deinit(struct zink_screen *screen, struct zink_program *pg);

void
zink_descriptors_update(struct zink_context *ctx, bool is_compute);


void
zink_context_invalidate_descriptor_state(struct zink_context *ctx, gl_shader_stage shader, enum zink_descriptor_type type, unsigned, unsigned);
void
zink_context_invalidate_descriptor_state_compact(struct zink_context *ctx, gl_shader_stage shader, enum zink_descriptor_type type, unsigned, unsigned);

void
zink_batch_descriptor_deinit(struct zink_screen *screen, struct zink_batch_state *bs);
void
zink_batch_descriptor_reset(struct zink_screen *screen, struct zink_batch_state *bs);
bool
zink_batch_descriptor_init(struct zink_screen *screen, struct zink_batch_state *bs);

bool
zink_descriptors_init(struct zink_context *ctx);

void
zink_descriptors_deinit(struct zink_context *ctx);

void
zink_descriptors_update_masked(struct zink_context *ctx, bool is_compute, uint8_t changed_sets, uint8_t bind_sets);
#ifdef __cplusplus
}
#endif

#endif
