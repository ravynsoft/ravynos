/*
 * Copyright © 2020 Mike Blumenkrantz
 * Copyright © 2022 Valve Corporation
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

#include "zink_context.h"
#include "zink_compiler.h"
#include "zink_descriptors.h"
#include "zink_program.h"
#include "zink_render_pass.h"
#include "zink_resource.h"
#include "zink_screen.h"

#define XXH_INLINE_ALL
#include "util/xxhash.h"

static VkDescriptorSetLayout
descriptor_layout_create(struct zink_screen *screen, enum zink_descriptor_type t, VkDescriptorSetLayoutBinding *bindings, unsigned num_bindings)
{
   VkDescriptorSetLayout dsl;
   VkDescriptorSetLayoutCreateInfo dcslci = {0};
   dcslci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
   dcslci.pNext = NULL;
   VkDescriptorSetLayoutBindingFlagsCreateInfo fci = {0};
   VkDescriptorBindingFlags flags[ZINK_MAX_DESCRIPTORS_PER_TYPE];
   dcslci.pNext = &fci;
   /* TODO bindless */
   if (zink_descriptor_mode == ZINK_DESCRIPTOR_MODE_DB && t != ZINK_DESCRIPTOR_BINDLESS)
      dcslci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
   else if (t == ZINK_DESCRIPTOR_TYPE_UNIFORMS)
      dcslci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
   fci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
   fci.bindingCount = num_bindings;
   fci.pBindingFlags = flags;
   for (unsigned i = 0; i < num_bindings; i++) {
      flags[i] = 0;
   }
   dcslci.bindingCount = num_bindings;
   dcslci.pBindings = bindings;
   VkDescriptorSetLayoutSupport supp;
   supp.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_SUPPORT;
   supp.pNext = NULL;
   supp.supported = VK_FALSE;
   if (VKSCR(GetDescriptorSetLayoutSupport)) {
      VKSCR(GetDescriptorSetLayoutSupport)(screen->dev, &dcslci, &supp);
      if (supp.supported == VK_FALSE) {
         debug_printf("vkGetDescriptorSetLayoutSupport claims layout is unsupported\n");
         return VK_NULL_HANDLE;
      }
   }
   VkResult result = VKSCR(CreateDescriptorSetLayout)(screen->dev, &dcslci, 0, &dsl);
   if (result != VK_SUCCESS)
      mesa_loge("ZINK: vkCreateDescriptorSetLayout failed (%s)", vk_Result_to_str(result));
   return dsl;
}

static uint32_t
hash_descriptor_layout(const void *key)
{
   uint32_t hash = 0;
   const struct zink_descriptor_layout_key *k = key;
   hash = XXH32(&k->num_bindings, sizeof(unsigned), hash);
   /* only hash first 3 members: no holes and the rest are always constant */
   for (unsigned i = 0; i < k->num_bindings; i++)
      hash = XXH32(&k->bindings[i], offsetof(VkDescriptorSetLayoutBinding, stageFlags), hash);

   return hash;
}

static bool
equals_descriptor_layout(const void *a, const void *b)
{
   const struct zink_descriptor_layout_key *a_k = a;
   const struct zink_descriptor_layout_key *b_k = b;
   return a_k->num_bindings == b_k->num_bindings &&
          (!a_k->num_bindings || !memcmp(a_k->bindings, b_k->bindings, a_k->num_bindings * sizeof(VkDescriptorSetLayoutBinding)));
}

static struct zink_descriptor_layout *
create_layout(struct zink_screen *screen, enum zink_descriptor_type type,
              VkDescriptorSetLayoutBinding *bindings, unsigned num_bindings,
              struct zink_descriptor_layout_key **layout_key)
{
   VkDescriptorSetLayout dsl = descriptor_layout_create(screen, type, bindings, num_bindings);
   if (!dsl)
      return NULL;

   size_t bindings_size = num_bindings * sizeof(VkDescriptorSetLayoutBinding);
   struct zink_descriptor_layout_key *k = ralloc_size(screen, sizeof(struct zink_descriptor_layout_key) + bindings_size);
   k->num_bindings = num_bindings;
   if (num_bindings) {
      k->bindings = (void *)(k + 1);
      memcpy(k->bindings, bindings, bindings_size);
   }

   struct zink_descriptor_layout *layout = rzalloc(screen, struct zink_descriptor_layout);
   layout->layout = dsl;
   *layout_key = k;
   return layout;
}

static struct zink_descriptor_layout *
descriptor_util_layout_get(struct zink_screen *screen, enum zink_descriptor_type type,
                      VkDescriptorSetLayoutBinding *bindings, unsigned num_bindings,
                      struct zink_descriptor_layout_key **layout_key)
{
   uint32_t hash = 0;
   struct zink_descriptor_layout_key key = {
      .num_bindings = num_bindings,
      .bindings = bindings,
   };

   /* push descriptor layouts are unique and can't be reused */
   if (type != ZINK_DESCRIPTOR_TYPE_UNIFORMS) {
      hash = hash_descriptor_layout(&key);
      simple_mtx_lock(&screen->desc_set_layouts_lock);
      struct hash_entry *he = _mesa_hash_table_search_pre_hashed(&screen->desc_set_layouts[type], hash, &key);
      simple_mtx_unlock(&screen->desc_set_layouts_lock);
      if (he) {
         *layout_key = (void*)he->key;
         return he->data;
      }
   }

   struct zink_descriptor_layout *layout = create_layout(screen, type, bindings, num_bindings, layout_key);
   if (layout && type != ZINK_DESCRIPTOR_TYPE_UNIFORMS) {
      simple_mtx_lock(&screen->desc_set_layouts_lock);
      _mesa_hash_table_insert_pre_hashed(&screen->desc_set_layouts[type], hash, *layout_key, layout);
      simple_mtx_unlock(&screen->desc_set_layouts_lock);
   }
   return layout;
}


static uint32_t
hash_descriptor_pool_key(const void *key)
{
   uint32_t hash = 0;
   const struct zink_descriptor_pool_key *k = key;
   hash = XXH32(&k->layout, sizeof(void*), hash);
   for (unsigned i = 0; i < k->num_type_sizes; i++)
      hash = XXH32(&k->sizes[i], sizeof(VkDescriptorPoolSize), hash);

   return hash;
}

static bool
equals_descriptor_pool_key(const void *a, const void *b)
{
   const struct zink_descriptor_pool_key *a_k = a;
   const struct zink_descriptor_pool_key *b_k = b;
   const unsigned a_num_type_sizes = a_k->num_type_sizes;
   const unsigned b_num_type_sizes = b_k->num_type_sizes;
   return a_k->layout == b_k->layout &&
          a_num_type_sizes == b_num_type_sizes &&
          !memcmp(a_k->sizes, b_k->sizes, b_num_type_sizes * sizeof(VkDescriptorPoolSize));
}

static struct zink_descriptor_pool_key *
descriptor_util_pool_key_get(struct zink_screen *screen, enum zink_descriptor_type type,
                                  struct zink_descriptor_layout_key *layout_key,
                                  VkDescriptorPoolSize *sizes, unsigned num_type_sizes)
{
   uint32_t hash = 0;
   struct zink_descriptor_pool_key key;
   key.num_type_sizes = num_type_sizes;
   /* push descriptor pools can't be shared/reused by other types */
   if (type != ZINK_DESCRIPTOR_TYPE_UNIFORMS) {
      key.layout = layout_key;
      memcpy(key.sizes, sizes, num_type_sizes * sizeof(VkDescriptorPoolSize));
      hash = hash_descriptor_pool_key(&key);
      simple_mtx_lock(&screen->desc_pool_keys_lock);
      struct set_entry *he = _mesa_set_search_pre_hashed(&screen->desc_pool_keys[type], hash, &key);
      simple_mtx_unlock(&screen->desc_pool_keys_lock);
      if (he)
         return (void*)he->key;
   }

   struct zink_descriptor_pool_key *pool_key = rzalloc(screen, struct zink_descriptor_pool_key);
   pool_key->layout = layout_key;
   pool_key->num_type_sizes = num_type_sizes;
   assert(pool_key->num_type_sizes);
   memcpy(pool_key->sizes, sizes, num_type_sizes * sizeof(VkDescriptorPoolSize));
   if (type != ZINK_DESCRIPTOR_TYPE_UNIFORMS) {
      simple_mtx_lock(&screen->desc_pool_keys_lock);
      _mesa_set_add_pre_hashed(&screen->desc_pool_keys[type], hash, pool_key);
      pool_key->id = screen->desc_pool_keys[type].entries - 1;
      simple_mtx_unlock(&screen->desc_pool_keys_lock);
   }
   return pool_key;
}

static void
init_push_binding(VkDescriptorSetLayoutBinding *binding, unsigned i, VkDescriptorType type)
{
   binding->binding = i;
   binding->descriptorType = type;
   binding->descriptorCount = 1;
   binding->stageFlags = mesa_to_vk_shader_stage(i);
   binding->pImmutableSamplers = NULL;
}

static VkDescriptorType
get_push_types(struct zink_screen *screen, enum zink_descriptor_type *dsl_type)
{
   *dsl_type = screen->info.have_KHR_push_descriptor ? ZINK_DESCRIPTOR_TYPE_UNIFORMS : ZINK_DESCRIPTOR_TYPE_UBO;
   return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
}

static struct zink_descriptor_layout *
create_gfx_layout(struct zink_context *ctx, struct zink_descriptor_layout_key **layout_key, bool fbfetch)
{
   struct zink_screen *screen = zink_screen(ctx->base.screen);
   VkDescriptorSetLayoutBinding bindings[MESA_SHADER_STAGES];
   enum zink_descriptor_type dsl_type;
   VkDescriptorType vktype = get_push_types(screen, &dsl_type);
   for (unsigned i = 0; i < ZINK_GFX_SHADER_COUNT; i++)
      init_push_binding(&bindings[i], i, vktype);
   if (fbfetch) {
      bindings[ZINK_GFX_SHADER_COUNT].binding = ZINK_FBFETCH_BINDING;
      bindings[ZINK_GFX_SHADER_COUNT].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
      bindings[ZINK_GFX_SHADER_COUNT].descriptorCount = 1;
      bindings[ZINK_GFX_SHADER_COUNT].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
      bindings[ZINK_GFX_SHADER_COUNT].pImmutableSamplers = NULL;
   }
   return create_layout(screen, dsl_type, bindings, fbfetch ? ARRAY_SIZE(bindings) : ARRAY_SIZE(bindings) - 1, layout_key);
}

bool
zink_descriptor_util_push_layouts_get(struct zink_context *ctx, struct zink_descriptor_layout **dsls, struct zink_descriptor_layout_key **layout_keys)
{
   struct zink_screen *screen = zink_screen(ctx->base.screen);
   VkDescriptorSetLayoutBinding compute_binding;
   enum zink_descriptor_type dsl_type;
   VkDescriptorType vktype = get_push_types(screen, &dsl_type);
   init_push_binding(&compute_binding, MESA_SHADER_COMPUTE, vktype);
   dsls[0] = create_gfx_layout(ctx, &layout_keys[0], false);
   dsls[1] = create_layout(screen, dsl_type, &compute_binding, 1, &layout_keys[1]);
   return dsls[0] && dsls[1];
}

VkImageLayout
zink_descriptor_util_image_layout_eval(const struct zink_context *ctx, const struct zink_resource *res, bool is_compute)
{
   if (res->bindless[0] || res->bindless[1]) {
      /* bindless needs most permissive layout */
      if (res->image_bind_count[0] || res->image_bind_count[1])
         return VK_IMAGE_LAYOUT_GENERAL;
      return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
   }
   if (res->image_bind_count[is_compute])
      return VK_IMAGE_LAYOUT_GENERAL;
   if (!is_compute && res->fb_bind_count && res->sampler_bind_count[0]) {
      /* feedback loop */
      if (!(res->obj->vkusage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) || zink_is_zsbuf_write(ctx)) {
         if (zink_screen(ctx->base.screen)->info.have_EXT_attachment_feedback_loop_layout)
            return VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT;
         return VK_IMAGE_LAYOUT_GENERAL;
      }
   }
   if (res->obj->vkusage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
      return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
   return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}

bool
zink_descriptor_util_alloc_sets(struct zink_screen *screen, VkDescriptorSetLayout dsl, VkDescriptorPool pool, VkDescriptorSet *sets, unsigned num_sets)
{
   VkDescriptorSetAllocateInfo dsai;
   VkDescriptorSetLayout layouts[100];
   assert(num_sets <= ARRAY_SIZE(layouts));
   memset((void *)&dsai, 0, sizeof(dsai));
   dsai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
   dsai.pNext = NULL;
   dsai.descriptorPool = pool;
   dsai.descriptorSetCount = num_sets;
   for (unsigned i = 0; i < num_sets; i ++)
      layouts[i] = dsl;
   dsai.pSetLayouts = layouts;

   VkResult result = VKSCR(AllocateDescriptorSets)(screen->dev, &dsai, sets);
   if (result != VK_SUCCESS) {
      mesa_loge("ZINK: %" PRIu64 " failed to allocate descriptor set :/ (%s)", (uint64_t)dsl, vk_Result_to_str(result));
      return false;
   }
   return true;
}

static void
init_db_template_entry(struct zink_screen *screen, struct zink_shader *shader, enum zink_descriptor_type type,
                       unsigned idx, struct zink_descriptor_template *entry, unsigned *entry_idx)
{
    int index = shader->bindings[type][idx].index;
    gl_shader_stage stage = clamp_stage(&shader->info);
    entry->count = shader->bindings[type][idx].size;

    switch (shader->bindings[type][idx].type) {
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
       entry->offset = offsetof(struct zink_context, di.db.ubos[stage][index]);
       entry->stride = sizeof(VkDescriptorAddressInfoEXT);
       entry->db_size = screen->info.db_props.robustUniformBufferDescriptorSize;
       break;
    case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
       entry->offset = offsetof(struct zink_context, di.textures[stage][index]);
       entry->stride = sizeof(VkDescriptorImageInfo);
       entry->db_size = screen->info.db_props.combinedImageSamplerDescriptorSize;
       break;
    case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
       entry->offset = offsetof(struct zink_context, di.textures[stage][index]);
       entry->stride = sizeof(VkDescriptorImageInfo);
       entry->db_size = screen->info.db_props.sampledImageDescriptorSize;
       break;
    case VK_DESCRIPTOR_TYPE_SAMPLER:
       entry->offset = offsetof(struct zink_context, di.textures[stage][index]);
       entry->stride = sizeof(VkDescriptorImageInfo);
       entry->db_size = screen->info.db_props.samplerDescriptorSize;
       break;
    case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
       entry->offset = offsetof(struct zink_context, di.db.tbos[stage][index]);
       entry->stride = sizeof(VkDescriptorAddressInfoEXT);
       entry->db_size = screen->info.db_props.robustUniformTexelBufferDescriptorSize;
       break;
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
       entry->offset = offsetof(struct zink_context, di.db.ssbos[stage][index]);
       entry->stride = sizeof(VkDescriptorAddressInfoEXT);
       entry->db_size = screen->info.db_props.robustStorageBufferDescriptorSize;
       break;
    case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
       entry->offset = offsetof(struct zink_context, di.images[stage][index]);
       entry->stride = sizeof(VkDescriptorImageInfo);
       entry->db_size = screen->info.db_props.storageImageDescriptorSize;
       break;
    case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
       entry->offset = offsetof(struct zink_context, di.db.texel_images[stage][index]);
       entry->stride = sizeof(VkDescriptorAddressInfoEXT);
       entry->db_size = screen->info.db_props.robustStorageTexelBufferDescriptorSize;
       break;
    default:
       unreachable("unknown type");
    }
    (*entry_idx)++;
}

static void
init_template_entry(struct zink_shader *shader, enum zink_descriptor_type type,
                    unsigned idx, VkDescriptorUpdateTemplateEntry *entry, unsigned *entry_idx)
{
    int index = shader->bindings[type][idx].index;
    gl_shader_stage stage = clamp_stage(&shader->info);
    entry->dstArrayElement = 0;
    entry->dstBinding = shader->bindings[type][idx].binding;
    entry->descriptorCount = shader->bindings[type][idx].size;
    if (shader->bindings[type][idx].type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)
       /* filter out DYNAMIC type here since this is just the uniform set */
       entry->descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    else
       entry->descriptorType = shader->bindings[type][idx].type;
    switch (shader->bindings[type][idx].type) {
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
       entry->offset = offsetof(struct zink_context, di.t.ubos[stage][index]);
       entry->stride = sizeof(VkDescriptorBufferInfo);
       break;
    case VK_DESCRIPTOR_TYPE_SAMPLER:
    case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
    case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
       entry->offset = offsetof(struct zink_context, di.textures[stage][index]);
       entry->stride = sizeof(VkDescriptorImageInfo);
       break;
    case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
       entry->offset = offsetof(struct zink_context, di.t.tbos[stage][index]);
       entry->stride = sizeof(VkBufferView);
       break;
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
       entry->offset = offsetof(struct zink_context, di.t.ssbos[stage][index]);
       entry->stride = sizeof(VkDescriptorBufferInfo);
       break;
    case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
       entry->offset = offsetof(struct zink_context, di.images[stage][index]);
       entry->stride = sizeof(VkDescriptorImageInfo);
       break;
    case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
       entry->offset = offsetof(struct zink_context, di.t.texel_images[stage][index]);
       entry->stride = sizeof(VkBufferView);
       break;
    default:
       unreachable("unknown type");
    }
    (*entry_idx)++;
}

static void
init_program_db(struct zink_screen *screen, struct zink_program *pg, enum zink_descriptor_type type, VkDescriptorSetLayoutBinding *bindings, unsigned num_bindings, VkDescriptorSetLayout dsl)
{
   VkDeviceSize val;
   VKSCR(GetDescriptorSetLayoutSizeEXT)(screen->dev, dsl, &val);
   pg->dd.db_size[type] = val;
   pg->dd.db_offset[type] = rzalloc_array(pg, uint32_t, num_bindings);
   for (unsigned i = 0; i < num_bindings; i++) {
      VKSCR(GetDescriptorSetLayoutBindingOffsetEXT)(screen->dev, dsl, bindings[i].binding, &val);
      pg->dd.db_offset[type][i] = val;
   }
}

static uint16_t
descriptor_program_num_sizes(VkDescriptorPoolSize *sizes, enum zink_descriptor_type type)
{
   switch (type) {
   case ZINK_DESCRIPTOR_TYPE_UBO:
      return !!sizes[ZDS_INDEX_UBO].descriptorCount;
   case ZINK_DESCRIPTOR_TYPE_SAMPLER_VIEW:
      return !!sizes[ZDS_INDEX_COMBINED_SAMPLER].descriptorCount +
             !!sizes[ZDS_INDEX_UNIFORM_TEXELS].descriptorCount +
             !!sizes[ZDS_INDEX_SAMPLER].descriptorCount;
   case ZINK_DESCRIPTOR_TYPE_SSBO:
      return !!sizes[ZDS_INDEX_STORAGE_BUFFER].descriptorCount;
   case ZINK_DESCRIPTOR_TYPE_IMAGE:
      return !!sizes[ZDS_INDEX_STORAGE_IMAGE].descriptorCount +
             !!sizes[ZDS_INDEX_STORAGE_TEXELS].descriptorCount;
   default: break;
   }
   unreachable("unknown type");
}

static uint16_t
descriptor_program_num_sizes_compact(VkDescriptorPoolSize *sizes, unsigned desc_set)
{
   switch (desc_set) {
   case ZINK_DESCRIPTOR_TYPE_UBO:
      return !!sizes[ZDS_INDEX_COMP_UBO].descriptorCount + !!sizes[ZDS_INDEX_COMP_STORAGE_BUFFER].descriptorCount;
   case ZINK_DESCRIPTOR_TYPE_SAMPLER_VIEW:
      return !!sizes[ZDS_INDEX_COMP_COMBINED_SAMPLER].descriptorCount +
             !!sizes[ZDS_INDEX_COMP_UNIFORM_TEXELS].descriptorCount +
             !!sizes[ZDS_INDEX_COMP_SAMPLER].descriptorCount +
             !!sizes[ZDS_INDEX_COMP_STORAGE_IMAGE].descriptorCount +
             !!sizes[ZDS_INDEX_COMP_STORAGE_TEXELS].descriptorCount;
   case ZINK_DESCRIPTOR_TYPE_SSBO:
   case ZINK_DESCRIPTOR_TYPE_IMAGE:
   default: break;
   }
   unreachable("unknown type");
}

/* create all the descriptor objects for a program:
 * called during program creation
 * may be called from threads (no unsafe ctx use!)
 */
bool
zink_descriptor_program_init(struct zink_context *ctx, struct zink_program *pg)
{
   struct zink_screen *screen = zink_screen(ctx->base.screen);
   VkDescriptorSetLayoutBinding bindings[ZINK_DESCRIPTOR_BASE_TYPES][MESA_SHADER_STAGES * 64];
   VkDescriptorUpdateTemplateEntry entries[ZINK_DESCRIPTOR_BASE_TYPES][MESA_SHADER_STAGES * 64];
   unsigned num_bindings[ZINK_DESCRIPTOR_BASE_TYPES] = {0};
   uint8_t has_bindings = 0;
   unsigned push_count = 0;
   uint16_t num_type_sizes[ZINK_DESCRIPTOR_BASE_TYPES];
   VkDescriptorPoolSize sizes[ZDS_INDEX_MAX] = {0}; //zink_descriptor_size_index

   struct zink_shader **stages;
   if (pg->is_compute)
      stages = &((struct zink_compute_program*)pg)->shader;
   else
      stages = ((struct zink_gfx_program*)pg)->shaders;

   if (!pg->is_compute && stages[MESA_SHADER_FRAGMENT]->info.fs.uses_fbfetch_output) {
      push_count = 1;
      pg->dd.fbfetch = true;
   }

   unsigned entry_idx[ZINK_DESCRIPTOR_BASE_TYPES] = {0};
   if (zink_descriptor_mode == ZINK_DESCRIPTOR_MODE_DB) {
      unsigned desc_set_size[ZINK_DESCRIPTOR_BASE_TYPES];
      for (unsigned i = 0; i < ZINK_DESCRIPTOR_BASE_TYPES; i++)
         desc_set_size[i] = zink_program_num_bindings_typed(pg, i);
      if (screen->compact_descriptors) {
         desc_set_size[ZINK_DESCRIPTOR_TYPE_UBO] += desc_set_size[ZINK_DESCRIPTOR_TYPE_SSBO];
         desc_set_size[ZINK_DESCRIPTOR_TYPE_SAMPLER_VIEW] += desc_set_size[ZINK_DESCRIPTOR_TYPE_IMAGE];
         desc_set_size[ZINK_DESCRIPTOR_TYPE_SSBO] = 0;
         desc_set_size[ZINK_DESCRIPTOR_TYPE_IMAGE] = 0;
      }
      for (unsigned i = 0; i < ZINK_DESCRIPTOR_BASE_TYPES; i++) {
         if (desc_set_size[i])
            pg->dd.db_template[i] = rzalloc_array(pg, struct zink_descriptor_template, desc_set_size[i]);
      }
   }

   unsigned num_shaders = pg->is_compute ? 1 : ZINK_GFX_SHADER_COUNT;
   bool have_push = screen->info.have_KHR_push_descriptor;
   /* iterate over the shaders and generate binding/layout/template structs */
   for (int i = 0; i < num_shaders; i++) {
      struct zink_shader *shader = stages[i];
      if (!shader)
         continue;

      gl_shader_stage stage = clamp_stage(&shader->info);
      VkShaderStageFlagBits stage_flags = mesa_to_vk_shader_stage(stage);
      /* uniform ubos handled in push */
      if (shader->has_uniforms) {
         pg->dd.push_usage |= BITFIELD64_BIT(stage);
         push_count++;
      }
      for (int j = 0; j < ZINK_DESCRIPTOR_BASE_TYPES; j++) {
         unsigned desc_type = screen->desc_set_id[j] - 1;
         for (int k = 0; k < shader->num_bindings[j]; k++) {
            assert(num_bindings[desc_type] < ARRAY_SIZE(bindings[desc_type]));
            VkDescriptorSetLayoutBinding *binding = &bindings[desc_type][num_bindings[desc_type]];
            binding->binding = shader->bindings[j][k].binding;
            binding->descriptorType = shader->bindings[j][k].type;
            binding->descriptorCount = shader->bindings[j][k].size;
            binding->stageFlags = stage_flags;
            binding->pImmutableSamplers = NULL;

            unsigned idx = screen->compact_descriptors ? zink_vktype_to_size_idx_comp(shader->bindings[j][k].type) :
                                                         zink_vktype_to_size_idx(shader->bindings[j][k].type);
            sizes[idx].descriptorCount += shader->bindings[j][k].size;
            sizes[idx].type = shader->bindings[j][k].type;
            if (zink_descriptor_mode == ZINK_DESCRIPTOR_MODE_DB)
               init_db_template_entry(screen, shader, j, k, &pg->dd.db_template[desc_type][entry_idx[desc_type]], &entry_idx[desc_type]);
            else
               init_template_entry(shader, j, k, &entries[desc_type][entry_idx[desc_type]], &entry_idx[desc_type]);
            num_bindings[desc_type]++;
            has_bindings |= BITFIELD_BIT(desc_type);
         }
         num_type_sizes[desc_type] = screen->compact_descriptors ?
                                    descriptor_program_num_sizes_compact(sizes, desc_type) :
                                    descriptor_program_num_sizes(sizes, j);
      }
      pg->dd.bindless |= shader->bindless;
   }
   pg->dd.binding_usage = has_bindings;
   if (!has_bindings && !push_count && !pg->dd.bindless) {
      pg->layout = zink_pipeline_layout_create(screen, pg->dsl, pg->num_dsl, pg->is_compute, 0);
      if (pg->layout)
         pg->compat_id = _mesa_hash_data(pg->dsl, pg->num_dsl * sizeof(pg->dsl[0]));
      return !!pg->layout;
   }

   pg->dsl[pg->num_dsl++] = push_count ? ctx->dd.push_dsl[pg->is_compute]->layout : ctx->dd.dummy_dsl->layout;
   /* iterate over the found descriptor types and create layouts / pool keys */
   if (has_bindings) {
      for (unsigned i = 0; i < ARRAY_SIZE(sizes); i++)
         sizes[i].descriptorCount *= MAX_LAZY_DESCRIPTORS;
      u_foreach_bit(desc_type, has_bindings) {
         /* descriptor sets must be bound contiguously, so add null sets for any that are "missing" */
         for (unsigned i = 0; i < desc_type; i++) {
            /* push set is always 0 */
            if (!pg->dsl[i + 1]) {
               /* inject a null dsl */
               pg->dsl[pg->num_dsl++] = ctx->dd.dummy_dsl->layout;
               pg->dd.binding_usage |= BITFIELD_BIT(i);
            }
         }
         struct zink_descriptor_layout_key *key;
         pg->dd.layouts[pg->num_dsl] = descriptor_util_layout_get(screen, desc_type, bindings[desc_type], num_bindings[desc_type], &key);
         unsigned idx = screen->compact_descriptors ? zink_descriptor_type_to_size_idx_comp(desc_type) :
                                                      zink_descriptor_type_to_size_idx(desc_type);
         /* some sets can have multiple descriptor types: ensure the size arrays for these types are contiguous for creating the pool key */
         VkDescriptorPoolSize *sz = &sizes[idx];
         VkDescriptorPoolSize sz2[5];
         if (screen->compact_descriptors || (pg->is_compute && stages[0]->info.stage == MESA_SHADER_KERNEL)) {
            unsigned found = 0;
            while (found < num_type_sizes[desc_type]) {
               if (sz->descriptorCount) {
                  memcpy(&sz2[found], sz, sizeof(VkDescriptorPoolSize));
                  found++;
               }
               sz++;
            }
            sz = sz2;
         } else {
            if (!sz->descriptorCount)
               sz++;
         }
         pg->dd.pool_key[desc_type] = descriptor_util_pool_key_get(screen, desc_type, key, sz, num_type_sizes[desc_type]);
         pg->dd.pool_key[desc_type]->use_count++;
         pg->dsl[pg->num_dsl] = pg->dd.layouts[pg->num_dsl]->layout;
         if (zink_descriptor_mode == ZINK_DESCRIPTOR_MODE_DB)
            init_program_db(screen, pg, desc_type, bindings[desc_type], num_bindings[desc_type], pg->dsl[pg->num_dsl]);
         pg->num_dsl++;
      }
   }
   /* TODO: make this dynamic so that bindless set id can be 0 if no other descriptors are used? */
   if (pg->dd.bindless) {
      unsigned desc_set = screen->desc_set_id[ZINK_DESCRIPTOR_BINDLESS];
      pg->num_dsl = desc_set + 1;
      pg->dsl[desc_set] = screen->bindless_layout;
      /* separate handling for null set injection when only bindless descriptors are used */
      for (unsigned i = 0; i < desc_set; i++) {
         if (!pg->dsl[i]) {
            /* inject a null dsl */
            pg->dsl[i] = ctx->dd.dummy_dsl->layout;
            if (i != screen->desc_set_id[ZINK_DESCRIPTOR_TYPE_UNIFORMS])
               pg->dd.binding_usage |= BITFIELD_BIT(i);
         }
      }
      /* all lower id sets are guaranteed to be used */
      pg->dd.binding_usage |= BITFIELD_MASK(ZINK_DESCRIPTOR_BASE_TYPES);
   }

   pg->layout = zink_pipeline_layout_create(screen, pg->dsl, pg->num_dsl, pg->is_compute, 0);
   if (!pg->layout)
      return false;
   pg->compat_id = _mesa_hash_data(pg->dsl, pg->num_dsl * sizeof(pg->dsl[0]));

   if (zink_descriptor_mode == ZINK_DESCRIPTOR_MODE_DB)
      return true;

   VkDescriptorUpdateTemplateCreateInfo template[ZINK_DESCRIPTOR_NON_BINDLESS_TYPES] = {0};
   /* type of template */
   VkDescriptorUpdateTemplateType types[ZINK_DESCRIPTOR_NON_BINDLESS_TYPES] = {VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET};
   if (have_push)
      types[0] = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR;

   /* number of descriptors in template */
   unsigned wd_count[ZINK_DESCRIPTOR_NON_BINDLESS_TYPES];
   if (push_count)
      wd_count[0] = pg->is_compute ? 1 : (ZINK_GFX_SHADER_COUNT + !!ctx->dd.has_fbfetch);
   for (unsigned i = 0; i < ZINK_DESCRIPTOR_BASE_TYPES; i++)
      wd_count[i + 1] = pg->dd.pool_key[i] ? pg->dd.pool_key[i]->layout->num_bindings : 0;

   VkDescriptorUpdateTemplateEntry *push_entries[2] = {
      ctx->dd.push_entries,
      &ctx->dd.compute_push_entry,
   };
   for (unsigned i = 0; i < pg->num_dsl; i++) {
      bool is_push = i == 0;
      /* no need for empty templates */
      if (pg->dsl[i] == ctx->dd.dummy_dsl->layout ||
          pg->dsl[i] == screen->bindless_layout ||
          (!is_push && pg->dd.templates[i]))
         continue;
      template[i].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO;
      assert(wd_count[i]);
      template[i].descriptorUpdateEntryCount = wd_count[i];
      if (is_push)
         template[i].pDescriptorUpdateEntries = push_entries[pg->is_compute];
      else
         template[i].pDescriptorUpdateEntries = entries[i - 1];
      template[i].templateType = types[i];
      template[i].descriptorSetLayout = pg->dsl[i];
      template[i].pipelineBindPoint = pg->is_compute ? VK_PIPELINE_BIND_POINT_COMPUTE : VK_PIPELINE_BIND_POINT_GRAPHICS;
      template[i].pipelineLayout = pg->layout;
      template[i].set = i;
      VkDescriptorUpdateTemplate t;
      if (VKSCR(CreateDescriptorUpdateTemplate)(screen->dev, &template[i], NULL, &t) != VK_SUCCESS)
         return false;
      pg->dd.templates[i] = t;
   }
   return true;
}

void
zink_descriptor_shader_get_binding_offsets(const struct zink_shader *shader, unsigned *offsets)
{
   offsets[ZINK_DESCRIPTOR_TYPE_UBO] = 0;
   offsets[ZINK_DESCRIPTOR_TYPE_SAMPLER_VIEW] = (shader->num_bindings[ZINK_DESCRIPTOR_TYPE_UBO] ?
                                                shader->bindings[ZINK_DESCRIPTOR_TYPE_UBO][shader->num_bindings[ZINK_DESCRIPTOR_TYPE_UBO] - 1].binding + 1 :
                                                1);
   offsets[ZINK_DESCRIPTOR_TYPE_SSBO] = offsets[ZINK_DESCRIPTOR_TYPE_SAMPLER_VIEW] + (shader->num_bindings[ZINK_DESCRIPTOR_TYPE_SAMPLER_VIEW] ?
                                                                                     shader->bindings[ZINK_DESCRIPTOR_TYPE_SAMPLER_VIEW][shader->num_bindings[ZINK_DESCRIPTOR_TYPE_SAMPLER_VIEW] - 1].binding + 1 :
                                                                                     1);
   offsets[ZINK_DESCRIPTOR_TYPE_IMAGE] = offsets[ZINK_DESCRIPTOR_TYPE_SSBO] + (shader->num_bindings[ZINK_DESCRIPTOR_TYPE_SSBO] ?
                                                                              shader->bindings[ZINK_DESCRIPTOR_TYPE_SSBO][shader->num_bindings[ZINK_DESCRIPTOR_TYPE_SSBO] - 1].binding + 1 :
                                                                              1);
}

void
zink_descriptor_shader_init(struct zink_screen *screen, struct zink_shader *shader)
{
   VkDescriptorSetLayoutBinding bindings[ZINK_DESCRIPTOR_BASE_TYPES * ZINK_MAX_DESCRIPTORS_PER_TYPE];
   unsigned num_bindings = 0;
   VkShaderStageFlagBits stage_flags = mesa_to_vk_shader_stage(clamp_stage(&shader->info));

   unsigned desc_set_size = shader->has_uniforms;
   for (unsigned i = 0; i < ZINK_DESCRIPTOR_BASE_TYPES; i++)
      desc_set_size += shader->num_bindings[i];
   if (desc_set_size)
      shader->precompile.db_template = rzalloc_array(shader, struct zink_descriptor_template, desc_set_size);

   if (shader->has_uniforms) {
      VkDescriptorSetLayoutBinding *binding = &bindings[num_bindings];
      binding->binding = 0;
      binding->descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      binding->descriptorCount = 1;
      binding->stageFlags = stage_flags;
      binding->pImmutableSamplers = NULL;
      struct zink_descriptor_template *entry = &shader->precompile.db_template[num_bindings];
      entry->count = 1;
      entry->offset = offsetof(struct zink_context, di.db.ubos[clamp_stage(&shader->info)][0]);
      entry->stride = sizeof(VkDescriptorAddressInfoEXT);
      entry->db_size = screen->info.db_props.robustUniformBufferDescriptorSize;
      num_bindings++;
   }
   /* sync with zink_shader_compile_separate() */
   unsigned offsets[4];
   zink_descriptor_shader_get_binding_offsets(shader, offsets);
   for (int j = 0; j < ZINK_DESCRIPTOR_BASE_TYPES; j++) {
      for (int k = 0; k < shader->num_bindings[j]; k++) {
         VkDescriptorSetLayoutBinding *binding = &bindings[num_bindings];
         if (j == ZINK_DESCRIPTOR_TYPE_UBO)
            binding->binding = 1;
         else
            binding->binding = shader->bindings[j][k].binding + offsets[j];
         binding->descriptorType = shader->bindings[j][k].type;
         binding->descriptorCount = shader->bindings[j][k].size;
         binding->stageFlags = stage_flags;
         binding->pImmutableSamplers = NULL;

         unsigned temp = 0;
         init_db_template_entry(screen, shader, j, k, &shader->precompile.db_template[num_bindings], &temp);
         num_bindings++;
      }
   }
   if (num_bindings) {
      shader->precompile.dsl = descriptor_layout_create(screen, 0, bindings, num_bindings);
      shader->precompile.bindings = mem_dup(bindings, num_bindings * sizeof(VkDescriptorSetLayoutBinding));
      shader->precompile.num_bindings = num_bindings;
      VkDeviceSize val;
      VKSCR(GetDescriptorSetLayoutSizeEXT)(screen->dev, shader->precompile.dsl, &val);
      shader->precompile.db_size = val;
      shader->precompile.db_offset = rzalloc_array(shader, uint32_t, num_bindings);
      for (unsigned i = 0; i < num_bindings; i++) {
         VKSCR(GetDescriptorSetLayoutBindingOffsetEXT)(screen->dev, shader->precompile.dsl, bindings[i].binding, &val);
         shader->precompile.db_offset[i] = val;
      }
   }
   if (screen->info.have_EXT_shader_object)
      return;
   VkDescriptorSetLayout dsl[ZINK_DESCRIPTOR_ALL_TYPES] = {0};
   unsigned num_dsl = num_bindings ? 2 : 0;
   if (shader->bindless)
      num_dsl = screen->compact_descriptors ? ZINK_DESCRIPTOR_ALL_TYPES - ZINK_DESCRIPTOR_COMPACT : ZINK_DESCRIPTOR_ALL_TYPES;
   if (num_bindings || shader->bindless) {
      dsl[shader->info.stage == MESA_SHADER_FRAGMENT] = shader->precompile.dsl;
      if (shader->bindless)
         dsl[screen->desc_set_id[ZINK_DESCRIPTOR_BINDLESS]] = screen->bindless_layout;
   }
   shader->precompile.layout = zink_pipeline_layout_create(screen, dsl, num_dsl, false, VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);
}

void
zink_descriptor_shader_deinit(struct zink_screen *screen, struct zink_shader *shader)
{
   if (shader->precompile.dsl)
      VKSCR(DestroyDescriptorSetLayout)(screen->dev, shader->precompile.dsl, NULL);
   if (shader->precompile.layout)
      VKSCR(DestroyPipelineLayout)(screen->dev, shader->precompile.layout, NULL);
}

/* called during program destroy */
void
zink_descriptor_program_deinit(struct zink_screen *screen, struct zink_program *pg)
{
   for (unsigned i = 0; pg->num_dsl && i < ZINK_DESCRIPTOR_BASE_TYPES; i++) {
      if (pg->dd.pool_key[i]) {
         pg->dd.pool_key[i]->use_count--;
         pg->dd.pool_key[i] = NULL;
      }
   }
   for (unsigned i = 0; pg->num_dsl && i < ZINK_DESCRIPTOR_NON_BINDLESS_TYPES; i++) {
      if (zink_descriptor_mode == ZINK_DESCRIPTOR_MODE_LAZY && pg->dd.templates[i]) {
         VKSCR(DestroyDescriptorUpdateTemplate)(screen->dev, pg->dd.templates[i], NULL);
         pg->dd.templates[i] = VK_NULL_HANDLE;
      }
   }
}

static void
pool_destroy(struct zink_screen *screen, struct zink_descriptor_pool *pool)
{
   VKSCR(DestroyDescriptorPool)(screen->dev, pool->pool, NULL);
   FREE(pool);
}

static void
multi_pool_destroy(struct zink_screen *screen, struct zink_descriptor_pool_multi *mpool)
{
   if (mpool->pool)
      pool_destroy(screen, mpool->pool);
   FREE(mpool);
}

static bool
clear_multi_pool_overflow(struct zink_screen *screen, struct util_dynarray *overflowed_pools)
{
   bool found = false;
   while (util_dynarray_num_elements(overflowed_pools, struct zink_descriptor_pool*)) {
      struct zink_descriptor_pool *pool = util_dynarray_pop(overflowed_pools, struct zink_descriptor_pool*);
      pool_destroy(screen, pool);
      found = true;
   }
   return found;
}

static VkDescriptorPool
create_pool(struct zink_screen *screen, unsigned num_type_sizes, const VkDescriptorPoolSize *sizes, unsigned flags)
{
   VkDescriptorPool pool;
   VkDescriptorPoolCreateInfo dpci = {0};
   dpci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
   dpci.pPoolSizes = sizes;
   dpci.poolSizeCount = num_type_sizes;
   dpci.flags = flags;
   dpci.maxSets = MAX_LAZY_DESCRIPTORS;
   VkResult result = VKSCR(CreateDescriptorPool)(screen->dev, &dpci, 0, &pool);
   if (result != VK_SUCCESS) {
      mesa_loge("ZINK: vkCreateDescriptorPool failed (%s)", vk_Result_to_str(result));
      return VK_NULL_HANDLE;
   }
   return pool;
}

static struct zink_descriptor_pool *
get_descriptor_pool(struct zink_context *ctx, struct zink_program *pg, enum zink_descriptor_type type, struct zink_batch_state *bs, bool is_compute);

/* set a multi-pool to its zink_descriptor_pool_key::id-indexed array element on a given batch state */
static bool
set_pool(struct zink_batch_state *bs, struct zink_program *pg, struct zink_descriptor_pool_multi *mpool, enum zink_descriptor_type type)
{
   /* push descriptors should never reach this */
   assert(type != ZINK_DESCRIPTOR_TYPE_UNIFORMS);
   assert(mpool);
   const struct zink_descriptor_pool_key *pool_key = pg->dd.pool_key[type];
   size_t size = bs->dd.pools[type].capacity;
   /* ensure the pool array is big enough to have an element for this key */
   if (!util_dynarray_resize(&bs->dd.pools[type], struct zink_descriptor_pool_multi*, pool_key->id + 1))
      return false;
   if (size != bs->dd.pools[type].capacity) {
      /* when resizing, always zero the new data to avoid garbage */
      uint8_t *data = bs->dd.pools[type].data;
      memset(data + size, 0, bs->dd.pools[type].capacity - size);
   }
   /* dynarray can't track sparse array sizing, so the array size must be manually tracked */
   bs->dd.pool_size[type] = MAX2(bs->dd.pool_size[type], pool_key->id + 1);
   struct zink_descriptor_pool_multi **mppool = util_dynarray_element(&bs->dd.pools[type], struct zink_descriptor_pool_multi*, pool_key->id);
   *mppool = mpool;
   return true;
}

static struct zink_descriptor_pool *
alloc_new_pool(struct zink_screen *screen, struct zink_descriptor_pool_multi *mpool)
{
   struct zink_descriptor_pool *pool = CALLOC_STRUCT(zink_descriptor_pool);
   if (!pool)
      return NULL;
   const unsigned num_type_sizes = mpool->pool_key->sizes[1].descriptorCount ? 2 : 1;
   pool->pool = create_pool(screen, num_type_sizes, mpool->pool_key->sizes, 0);
   if (!pool->pool) {
      FREE(pool);
      return NULL;
   }
   return pool;
}

/* strictly for finding a usable pool in oom scenarios */
static void
find_pool(struct zink_screen *screen, struct zink_batch_state *bs, struct zink_descriptor_pool_multi *mpool, bool both)
{
   bool found = false;
   /* worst case: iterate all the pools for the batch until something can be recycled */
   for (unsigned type = 0; type < ZINK_DESCRIPTOR_BASE_TYPES; type++) {
      for (unsigned i = 0; i < bs->dd.pool_size[type]; i++) {
         struct zink_descriptor_pool_multi **mppool = util_dynarray_element(&bs->dd.pools[type], struct zink_descriptor_pool_multi *, i);
         if (mppool && *mppool && *mppool != mpool) {
            unsigned idx[] = {!(*mppool)->overflow_idx, (*mppool)->overflow_idx};
            for (unsigned j = 0; j < 1 + !!both; j++)
               found |= clear_multi_pool_overflow(screen, &(*mppool)->overflowed_pools[idx[j]]);
         }
      }
   }
   if (found)
      mpool->pool = alloc_new_pool(screen, mpool);
}

static struct zink_descriptor_pool *
check_pool_alloc(struct zink_context *ctx, struct zink_descriptor_pool_multi *mpool, struct zink_program *pg,
                 enum zink_descriptor_type type, struct zink_batch_state *bs, bool is_compute)
{
   struct zink_screen *screen = zink_screen(ctx->base.screen);
   assert(mpool->pool_key == pg->dd.pool_key[type]);
   /* a current pool may not exist */
   if (!mpool->pool) {
      /* first, try to recycle a pool from the idle overflowed sets */
      if (util_dynarray_contains(&mpool->overflowed_pools[!mpool->overflow_idx], struct zink_descriptor_pool*))
         mpool->pool = util_dynarray_pop(&mpool->overflowed_pools[!mpool->overflow_idx], struct zink_descriptor_pool*);
      else
         /* if none exist, try to create a new one */
         mpool->pool = alloc_new_pool(screen, mpool);
      /* OOM: force pool recycling from overflows */
      if (!mpool->pool) {
         find_pool(screen, bs, mpool, false);
         if (!mpool->pool) {
            /* bad case: iterate unused batches and recycle */
            for (struct zink_batch_state *state = ctx->free_batch_states; state; state = state->next)
               find_pool(screen, state, mpool, true);
            if (!mpool->pool) {
               /* worst case: iterate in-use batches and recycle (very safe) */
               for (struct zink_batch_state *state = ctx->batch_states; state; state = state->next)
                  find_pool(screen, state, mpool, false);
            }
         }
      }
      if (!mpool->pool)
         unreachable("out of descriptor memory!");
   }
   struct zink_descriptor_pool *pool = mpool->pool;
   /* allocate up to $current * 10, e.g., 10 -> 100;
    * never allocate more than 100 at a time to minimize unused descriptor sets
    */
   if (pool->set_idx == pool->sets_alloc) {
      unsigned sets_to_alloc = MIN2(MIN2(MAX2(pool->sets_alloc * 10, 10), MAX_LAZY_DESCRIPTORS) - pool->sets_alloc, 100);
      if (!sets_to_alloc) {
         /* overflowed pool: store for reuse */
         pool->set_idx = 0;
         util_dynarray_append(&mpool->overflowed_pools[mpool->overflow_idx], struct zink_descriptor_pool*, pool);
         mpool->pool = NULL;
         /* call recursively to get recycle/oom handling */
         return get_descriptor_pool(ctx, pg, type, bs, is_compute);
      }
      if (!zink_descriptor_util_alloc_sets(screen, pg->dsl[type + 1],
                                           pool->pool, &pool->sets[pool->sets_alloc], sets_to_alloc))
         return NULL;
      pool->sets_alloc += sets_to_alloc;
   }
   return pool;
}

static struct zink_descriptor_pool *
create_push_pool(struct zink_screen *screen, struct zink_batch_state *bs, bool is_compute, bool has_fbfetch)
{
   struct zink_descriptor_pool *pool = CALLOC_STRUCT(zink_descriptor_pool);
   VkDescriptorPoolSize sizes[2];
   sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
   if (is_compute)
      sizes[0].descriptorCount = MAX_LAZY_DESCRIPTORS;
   else {
      sizes[0].descriptorCount = ZINK_GFX_SHADER_COUNT * MAX_LAZY_DESCRIPTORS;
      sizes[1].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
      sizes[1].descriptorCount = MAX_LAZY_DESCRIPTORS;
   }
   pool->pool = create_pool(screen, !is_compute && has_fbfetch ? 2 : 1, sizes, 0);
   return pool;
}

static struct zink_descriptor_pool *
check_push_pool_alloc(struct zink_context *ctx, struct zink_descriptor_pool_multi *mpool, struct zink_batch_state *bs, bool is_compute)
{
   struct zink_screen *screen = zink_screen(ctx->base.screen);
   struct zink_descriptor_pool *pool = mpool->pool;
   /* allocate up to $current * 10, e.g., 10 -> 100 or 100 -> 1000 */
   if (pool->set_idx == pool->sets_alloc || unlikely(ctx->dd.has_fbfetch != bs->dd.has_fbfetch)) {
      unsigned sets_to_alloc = MIN2(MIN2(MAX2(pool->sets_alloc * 10, 10), MAX_LAZY_DESCRIPTORS) - pool->sets_alloc, 100);
      if (!sets_to_alloc || unlikely(ctx->dd.has_fbfetch != bs->dd.has_fbfetch)) {
         /* overflowed pool: store for reuse */
         pool->set_idx = 0;
         util_dynarray_append(&mpool->overflowed_pools[mpool->overflow_idx], struct zink_descriptor_pool*, pool);
         if (util_dynarray_contains(&mpool->overflowed_pools[!mpool->overflow_idx], struct zink_descriptor_pool*))
            bs->dd.push_pool[is_compute].pool = util_dynarray_pop(&mpool->overflowed_pools[!mpool->overflow_idx], struct zink_descriptor_pool*);
         else
            bs->dd.push_pool[is_compute].pool = create_push_pool(screen, bs, is_compute, ctx->dd.has_fbfetch);
         if (unlikely(ctx->dd.has_fbfetch != bs->dd.has_fbfetch))
            mpool->reinit_overflow = true;
         bs->dd.has_fbfetch = ctx->dd.has_fbfetch;
         return check_push_pool_alloc(ctx, &bs->dd.push_pool[is_compute], bs, is_compute);
      }
      if (!zink_descriptor_util_alloc_sets(screen, ctx->dd.push_dsl[is_compute]->layout,
                                           pool->pool, &pool->sets[pool->sets_alloc], sets_to_alloc)) {
         mesa_loge("ZINK: failed to allocate push set!");
         return NULL;
      }
      pool->sets_alloc += sets_to_alloc;
   }
   return pool;
}

static struct zink_descriptor_pool *
get_descriptor_pool(struct zink_context *ctx, struct zink_program *pg, enum zink_descriptor_type type, struct zink_batch_state *bs, bool is_compute)
{
   struct zink_screen *screen = zink_screen(ctx->base.screen);
   const struct zink_descriptor_pool_key *pool_key = pg->dd.pool_key[type];
   struct zink_descriptor_pool_multi **mppool = bs->dd.pool_size[type] > pool_key->id ?
                                         util_dynarray_element(&bs->dd.pools[type], struct zink_descriptor_pool_multi *, pool_key->id) :
                                         NULL;
   if (mppool && *mppool)
      return check_pool_alloc(ctx, *mppool, pg, type, bs, is_compute);
   struct zink_descriptor_pool_multi *mpool = CALLOC_STRUCT(zink_descriptor_pool_multi);
   if (!mpool)
      return NULL;
   util_dynarray_init(&mpool->overflowed_pools[0], NULL);
   util_dynarray_init(&mpool->overflowed_pools[1], NULL);
   mpool->pool_key = pool_key;
   if (!set_pool(bs, pg, mpool, type)) {
      multi_pool_destroy(screen, mpool);
      return NULL;
   }
   assert(pool_key->id < bs->dd.pool_size[type]);
   return check_pool_alloc(ctx, mpool, pg, type, bs, is_compute);
}

ALWAYS_INLINE static VkDescriptorSet
get_descriptor_set(struct zink_descriptor_pool *pool)
{
   if (!pool)
      return VK_NULL_HANDLE;

   assert(pool->set_idx < pool->sets_alloc);
   return pool->sets[pool->set_idx++];
}

static bool
populate_sets(struct zink_context *ctx, struct zink_batch_state *bs,
              struct zink_program *pg, uint8_t changed_sets, VkDescriptorSet *sets)
{
   u_foreach_bit(type, changed_sets) {
      if (pg->dd.pool_key[type]) {
         struct zink_descriptor_pool *pool = get_descriptor_pool(ctx, pg, type, bs, pg->is_compute);
         sets[type] = get_descriptor_set(pool);
         if (!sets[type])
            return false;
      } else
         sets[type] = VK_NULL_HANDLE;
   }
   return true;
}

static void
reinit_db(struct zink_screen *screen, struct zink_batch_state *bs)
{
   zink_batch_descriptor_deinit(screen, bs);
   zink_batch_descriptor_init(screen, bs);
}

static void
enlarge_db(struct zink_context *ctx)
{
   struct zink_screen *screen = zink_screen(ctx->base.screen);
   struct zink_batch_state *bs = ctx->batch.state;
   /* ensure current db surives */
   zink_batch_reference_resource(&ctx->batch, bs->dd.db);
   /* rebinding a db mid-batch is extremely costly: scaling by 10x should ensure it never happens more than twice */
   ctx->dd.db.max_db_size *= 10;
   reinit_db(screen, bs);
}

static void
update_separable(struct zink_context *ctx, struct zink_program *pg)
{
   struct zink_screen *screen = zink_screen(ctx->base.screen);
   struct zink_batch_state *bs = ctx->batch.state;

   unsigned use_buffer = 0;
   VkDescriptorGetInfoEXT info;
   info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT;
   info.pNext = NULL;
   struct zink_gfx_program *prog = (struct zink_gfx_program *)pg;
   size_t db_size = 0;
   for (unsigned i = 0; i < ZINK_GFX_SHADER_COUNT; i++) {
      if (prog->shaders[i])
         db_size += prog->shaders[i]->precompile.db_size;
   }

   if (bs->dd.db_offset + db_size >= bs->dd.db->base.b.width0)
      enlarge_db(ctx);

   if (!bs->dd.db_bound)
      zink_batch_bind_db(ctx);

   for (unsigned j = 0; j < ZINK_GFX_SHADER_COUNT; j++) {
      struct zink_shader *zs = prog->shaders[j];
      if (!zs || !zs->precompile.dsl)
         continue;
      uint64_t offset = bs->dd.db_offset;
      assert(bs->dd.db->base.b.width0 > bs->dd.db_offset + zs->precompile.db_size);
      for (unsigned i = 0; i < zs->precompile.num_bindings; i++) {
         info.type = zs->precompile.bindings[i].descriptorType;
         uint64_t desc_offset = offset + zs->precompile.db_offset[i];
         if (screen->info.db_props.combinedImageSamplerDescriptorSingleArray ||
               zs->precompile.bindings[i].descriptorType != VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
               zs->precompile.bindings[i].descriptorCount == 1) {
            for (unsigned k = 0; k < zs->precompile.bindings[i].descriptorCount; k++) {
               /* VkDescriptorDataEXT is a union of pointers; the member doesn't matter */
               info.data.pSampler = (void*)(((uint8_t*)ctx) + zs->precompile.db_template[i].offset + k * zs->precompile.db_template[i].stride);
               VKSCR(GetDescriptorEXT)(screen->dev, &info, zs->precompile.db_template[i].db_size, bs->dd.db_map + desc_offset + k * zs->precompile.db_template[i].db_size);
            }
         } else {
            assert(zs->precompile.bindings[i].descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
            char buf[1024];
            uint8_t *db = bs->dd.db_map + desc_offset;
            uint8_t *samplers = db + zs->precompile.bindings[i].descriptorCount * screen->info.db_props.sampledImageDescriptorSize;
            for (unsigned k = 0; k < zs->precompile.bindings[i].descriptorCount; k++) {
               /* VkDescriptorDataEXT is a union of pointers; the member doesn't matter */
               info.data.pSampler = (void*)(((uint8_t*)ctx) + zs->precompile.db_template[i].offset +
                                             k * zs->precompile.db_template[i].stride);
               VKSCR(GetDescriptorEXT)(screen->dev, &info, zs->precompile.db_template[i].db_size, buf);
               /* drivers that don't support combinedImageSamplerDescriptorSingleArray must have sampler arrays written in memory as
                  *
                  *   | array_of_samplers[] | array_of_sampled_images[] |
                  * 
                  * which means each descriptor's data must be split
                  */
               memcpy(db, buf, screen->info.db_props.samplerDescriptorSize);
               memcpy(samplers, &buf[screen->info.db_props.samplerDescriptorSize], screen->info.db_props.sampledImageDescriptorSize);
               db += screen->info.db_props.sampledImageDescriptorSize;
               samplers += screen->info.db_props.samplerDescriptorSize;
            }
         }
      }
      bs->dd.cur_db_offset[use_buffer] = bs->dd.db_offset;
      bs->dd.db_offset += zs->precompile.db_size;
      /* TODO: maybe compile multiple variants for different set counts for compact mode? */
      int set_idx = screen->info.have_EXT_shader_object ? j : j == MESA_SHADER_FRAGMENT;
      VKCTX(CmdSetDescriptorBufferOffsetsEXT)(bs->cmdbuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pg->layout, set_idx, 1, &use_buffer, &offset);
   }
}

/* updates the mask of changed_sets and binds the mask of bind_sets */
static void
zink_descriptors_update_masked_buffer(struct zink_context *ctx, bool is_compute, uint8_t changed_sets, uint8_t bind_sets)
{
   struct zink_screen *screen = zink_screen(ctx->base.screen);
   struct zink_batch_state *bs = ctx->batch.state;
   struct zink_program *pg = is_compute ? &ctx->curr_compute->base : &ctx->curr_program->base;

   /* skip if no descriptors are updated */
   if (!pg->dd.binding_usage || (!changed_sets && !bind_sets))
      return;

   unsigned use_buffer = 0;
   u_foreach_bit(type, changed_sets | bind_sets) {
      if (!pg->dd.pool_key[type])
         continue;
      assert(type + 1 < pg->num_dsl);
      assert(type < ZINK_DESCRIPTOR_BASE_TYPES);
      bool changed = (changed_sets & BITFIELD_BIT(type)) > 0;
      uint64_t offset = changed ? bs->dd.db_offset : bs->dd.cur_db_offset[type];
      if (pg->dd.db_template[type] && changed) {
         const struct zink_descriptor_layout_key *key = pg->dd.pool_key[type]->layout;
         VkDescriptorGetInfoEXT info;
         info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT;
         info.pNext = NULL;
         assert(bs->dd.db->base.b.width0 > bs->dd.db_offset + pg->dd.db_size[type]);
         for (unsigned i = 0; i < key->num_bindings; i++) {
            info.type = key->bindings[i].descriptorType;
            uint64_t desc_offset = offset + pg->dd.db_offset[type][i];
            if (screen->info.db_props.combinedImageSamplerDescriptorSingleArray ||
                key->bindings[i].descriptorType != VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
                key->bindings[i].descriptorCount == 1) {
               for (unsigned j = 0; j < key->bindings[i].descriptorCount; j++) {
                  /* VkDescriptorDataEXT is a union of pointers; the member doesn't matter */
                  info.data.pSampler = (void*)(((uint8_t*)ctx) + pg->dd.db_template[type][i].offset + j * pg->dd.db_template[type][i].stride);
                  VKSCR(GetDescriptorEXT)(screen->dev, &info, pg->dd.db_template[type][i].db_size, bs->dd.db_map + desc_offset + j * pg->dd.db_template[type][i].db_size);
               }
            } else {
               assert(key->bindings[i].descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
               char buf[1024];
               uint8_t *db = bs->dd.db_map + desc_offset;
               uint8_t *samplers = db + key->bindings[i].descriptorCount * screen->info.db_props.sampledImageDescriptorSize;
               for (unsigned j = 0; j < key->bindings[i].descriptorCount; j++) {
                  /* VkDescriptorDataEXT is a union of pointers; the member doesn't matter */
                  info.data.pSampler = (void*)(((uint8_t*)ctx) + pg->dd.db_template[ZINK_DESCRIPTOR_TYPE_SAMPLER_VIEW][i].offset +
                                               j * pg->dd.db_template[ZINK_DESCRIPTOR_TYPE_SAMPLER_VIEW][i].stride);
                  VKSCR(GetDescriptorEXT)(screen->dev, &info, pg->dd.db_template[type][ZINK_DESCRIPTOR_TYPE_SAMPLER_VIEW].db_size, buf);
                  /* drivers that don't support combinedImageSamplerDescriptorSingleArray must have sampler arrays written in memory as
                   *
                   *   | array_of_samplers[] | array_of_sampled_images[] |
                   * 
                   * which means each descriptor's data must be split
                   */
                  memcpy(db, buf, screen->info.db_props.samplerDescriptorSize);
                  memcpy(samplers, &buf[screen->info.db_props.samplerDescriptorSize], screen->info.db_props.sampledImageDescriptorSize);
                  db += screen->info.db_props.sampledImageDescriptorSize;
                  samplers += screen->info.db_props.samplerDescriptorSize;
               }
            }
         }
         bs->dd.cur_db_offset[type] = bs->dd.db_offset;
         bs->dd.db_offset += pg->dd.db_size[type];
      }
      zink_flush_dgc_if_enabled(ctx);
      /* templates are indexed by the set id, so increment type by 1
         * (this is effectively an optimization of indirecting through screen->desc_set_id)
         */
      VKCTX(CmdSetDescriptorBufferOffsetsEXT)(bs->cmdbuf,
                                                is_compute ? VK_PIPELINE_BIND_POINT_COMPUTE : VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                pg->layout,
                                                type + 1, 1,
                                                &use_buffer,
                                                &offset);
   }
}

/* updates the mask of changed_sets and binds the mask of bind_sets */
void
zink_descriptors_update_masked(struct zink_context *ctx, bool is_compute, uint8_t changed_sets, uint8_t bind_sets)
{
   struct zink_screen *screen = zink_screen(ctx->base.screen);
   struct zink_batch_state *bs = ctx->batch.state;
   struct zink_program *pg = is_compute ? &ctx->curr_compute->base : &ctx->curr_program->base;
   VkDescriptorSet desc_sets[ZINK_DESCRIPTOR_BASE_TYPES];

   /* skip if no descriptors are updated */
   if (!pg->dd.binding_usage || (!changed_sets && !bind_sets))
      return;

   /* populate usable sets for the changed_sets mask */
   if (!populate_sets(ctx, bs, pg, changed_sets, desc_sets)) {
      debug_printf("ZINK: couldn't get descriptor sets!\n");
      return;
   }
   /* no flushing allowed: sets are allocated to the batch, so this breaks everything */
   assert(ctx->batch.state == bs);

   u_foreach_bit(type, changed_sets) {
      assert(type + 1 < pg->num_dsl);
      if (pg->dd.pool_key[type]) {
         zink_flush_dgc_if_enabled(ctx);
         /* templates are indexed by the set id, so increment type by 1
          * (this is effectively an optimization of indirecting through screen->desc_set_id)
          */
         VKSCR(UpdateDescriptorSetWithTemplate)(screen->dev, desc_sets[type], pg->dd.templates[type + 1], ctx);
         VKSCR(CmdBindDescriptorSets)(bs->cmdbuf,
                                 is_compute ? VK_PIPELINE_BIND_POINT_COMPUTE : VK_PIPELINE_BIND_POINT_GRAPHICS,
                                 /* same set indexing as above */
                                 pg->layout, type + 1, 1, &desc_sets[type],
                                 0, NULL);
         bs->dd.sets[is_compute][type + 1] = desc_sets[type];
      }
   }
   /* these are the unchanged sets being rebound across pipeline changes when compat_id changes but the set is the same
    * also handles binding null sets
    */
   u_foreach_bit(type, bind_sets & ~changed_sets) {
      if (!pg->dd.pool_key[type])
         continue;
      /* same set indexing as above */
      assert(bs->dd.sets[is_compute][type + 1]);
      zink_flush_dgc_if_enabled(ctx);
      VKSCR(CmdBindDescriptorSets)(bs->cmdbuf,
                              is_compute ? VK_PIPELINE_BIND_POINT_COMPUTE : VK_PIPELINE_BIND_POINT_GRAPHICS,
                              /* same set indexing as above */
                              pg->layout, type + 1, 1, &bs->dd.sets[is_compute][type + 1],
                              0, NULL);
   }
}

static void
bind_bindless_db(struct zink_context *ctx, struct zink_program *pg)
{
   struct zink_batch_state *bs = ctx->batch.state;
   struct zink_screen *screen = zink_screen(ctx->base.screen);
   unsigned index = 1;
   VkDeviceSize offset = 0;
   VKCTX(CmdSetDescriptorBufferOffsetsEXT)(bs->cmdbuf,
                                           pg->is_compute ? VK_PIPELINE_BIND_POINT_COMPUTE : VK_PIPELINE_BIND_POINT_GRAPHICS,
                                           pg->layout,
                                           screen->desc_set_id[ZINK_DESCRIPTOR_BINDLESS], 1,
                                           &index,
                                           &offset);
   ctx->dd.bindless_bound = true;
}

/* entrypoint for all descriptor updating:
 * - update push set
 * - generate masks for updating other sets
 * - always called from driver thread
 */
void
zink_descriptors_update(struct zink_context *ctx, bool is_compute)
{
   struct zink_batch_state *bs = ctx->batch.state;
   struct zink_program *pg = is_compute ? &ctx->curr_compute->base : &ctx->curr_program->base;
   struct zink_screen *screen = zink_screen(ctx->base.screen);
   bool have_KHR_push_descriptor = screen->info.have_KHR_push_descriptor;

   bool batch_changed = !bs->dd.pg[is_compute];
   if (batch_changed) {
      /* update all sets and bind null sets */
      ctx->dd.state_changed[is_compute] = pg->dd.binding_usage & BITFIELD_MASK(ZINK_DESCRIPTOR_TYPE_UNIFORMS);
      ctx->dd.push_state_changed[is_compute] = !!pg->dd.push_usage || ctx->dd.has_fbfetch != bs->dd.has_fbfetch;
   }

   if (!is_compute) {
      struct zink_gfx_program *prog = (struct zink_gfx_program*)pg;
      if (prog->is_separable) {
         /* force all descriptors update on next pass: separables use different layouts */
         ctx->dd.state_changed[is_compute] = BITFIELD_MASK(ZINK_DESCRIPTOR_TYPE_UNIFORMS);
         ctx->dd.push_state_changed[is_compute] = true;
         update_separable(ctx, pg);
         if (pg->dd.bindless)
            bind_bindless_db(ctx, pg);
         return;
      }
   }

   if (pg != bs->dd.pg[is_compute]) {
      /* if we don't already know that we have to update all sets,
       * check to see if any dsls changed
       *
       * also always update the dsl pointers on program change
       */
       for (unsigned i = 0; i < ARRAY_SIZE(bs->dd.dsl[is_compute]); i++) {
          /* push set is already detected, start at 1 */
          if (bs->dd.dsl[is_compute][i] != pg->dsl[i + 1])
             ctx->dd.state_changed[is_compute] |= BITFIELD_BIT(i);
          bs->dd.dsl[is_compute][i] = pg->dsl[i + 1];
       }
       ctx->dd.push_state_changed[is_compute] |= bs->dd.push_usage[is_compute] != pg->dd.push_usage;
       bs->dd.push_usage[is_compute] = pg->dd.push_usage;
   }

   uint8_t changed_sets = pg->dd.binding_usage & ctx->dd.state_changed[is_compute];
   /*
    * when binding a pipeline, the pipeline can correctly access any previously bound
    * descriptor sets which were bound with compatible pipeline layouts
    * VK 14.2.2
    */
   uint8_t bind_sets = bs->dd.pg[is_compute] && bs->dd.compat_id[is_compute] == pg->compat_id ? 0 : pg->dd.binding_usage;

   if (zink_descriptor_mode == ZINK_DESCRIPTOR_MODE_DB) {
      size_t check_size = 0;
      if (pg->dd.push_usage && ctx->dd.push_state_changed[is_compute])
         check_size += ctx->dd.db_size[is_compute];
      for (unsigned i = 0; i < ZINK_DESCRIPTOR_BASE_TYPES; i++) {
         if (changed_sets & BITFIELD_BIT(i))
            check_size += pg->dd.db_size[i];
      }

      if (bs->dd.db_offset + check_size >= bs->dd.db->base.b.width0) {
         enlarge_db(ctx);
         changed_sets = pg->dd.binding_usage;
         ctx->dd.push_state_changed[is_compute] = true;
         zink_flush_dgc_if_enabled(ctx);
      }

      if (!bs->dd.db_bound)
         zink_batch_bind_db(ctx);
   }

   if (pg->dd.push_usage && (ctx->dd.push_state_changed[is_compute] || bind_sets)) {
      if (zink_descriptor_mode == ZINK_DESCRIPTOR_MODE_DB) {
         uint32_t index = 0;
         uint64_t offset = ctx->dd.push_state_changed[is_compute] ?
                           bs->dd.db_offset :
                           bs->dd.cur_db_offset[ZINK_DESCRIPTOR_TYPE_UNIFORMS];
         if (ctx->dd.push_state_changed[is_compute]) {
            assert(bs->dd.db->base.b.width0 > bs->dd.db_offset + ctx->dd.db_size[is_compute]);
            for (unsigned i = 0; i < (is_compute ? 1 : ZINK_GFX_SHADER_COUNT); i++) {
               VkDescriptorGetInfoEXT info;
               info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT;
               info.pNext = NULL;
               info.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
               info.data.pUniformBuffer = &ctx->di.db.ubos[is_compute ? MESA_SHADER_COMPUTE : i][0];
               uint64_t stage_offset = offset + (is_compute ? 0 : ctx->dd.db_offset[i]);
               VKSCR(GetDescriptorEXT)(screen->dev, &info, screen->info.db_props.robustUniformBufferDescriptorSize,
                                                           bs->dd.db_map + stage_offset);
            }
            if (!is_compute && ctx->dd.has_fbfetch) {
               uint64_t stage_offset = offset + ctx->dd.db_offset[MESA_SHADER_FRAGMENT + 1];
               if (pg->dd.fbfetch && screen->info.db_props.inputAttachmentDescriptorSize) {
                  /* real fbfetch descriptor */
                  VkDescriptorGetInfoEXT info;
                  info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT;
                  info.pNext = NULL;
                  info.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
                  info.data.pInputAttachmentImage = &ctx->di.fbfetch;
                  VKSCR(GetDescriptorEXT)(screen->dev, &info, screen->info.db_props.inputAttachmentDescriptorSize,
                                                            bs->dd.db_map + stage_offset);
               } else {
                  /* reuse cached dummy descriptor */
                  memcpy(bs->dd.db_map + stage_offset, ctx->di.fbfetch_db, screen->info.db_props.inputAttachmentDescriptorSize);
               }
            }
            bs->dd.cur_db_offset[ZINK_DESCRIPTOR_TYPE_UNIFORMS] = bs->dd.db_offset;
            bs->dd.db_offset += ctx->dd.db_size[is_compute];
         }
         zink_flush_dgc_if_enabled(ctx);
         VKCTX(CmdSetDescriptorBufferOffsetsEXT)(bs->cmdbuf,
                                                 is_compute ? VK_PIPELINE_BIND_POINT_COMPUTE : VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                 pg->layout,
                                                 0, 1,
                                                 &index,
                                                 &offset);
      } else {
         if (ctx->dd.push_state_changed[0]) {
            zink_flush_dgc_if_enabled(ctx);
         }
         if (have_KHR_push_descriptor) {
            if (ctx->dd.push_state_changed[is_compute])
               VKCTX(CmdPushDescriptorSetWithTemplateKHR)(bs->cmdbuf, pg->dd.templates[0],
                                                         pg->layout, 0, ctx);
         } else {
            if (ctx->dd.push_state_changed[is_compute]) {
               struct zink_descriptor_pool *pool = check_push_pool_alloc(ctx, &bs->dd.push_pool[pg->is_compute], bs, pg->is_compute);
               VkDescriptorSet push_set = get_descriptor_set(pool);
               if (!push_set)
                  mesa_loge("ZINK: failed to get push descriptor set! prepare to crash!");
               VKCTX(UpdateDescriptorSetWithTemplate)(screen->dev, push_set, pg->dd.templates[0], ctx);
               bs->dd.sets[is_compute][0] = push_set;
            }
            assert(bs->dd.sets[is_compute][0]);
            VKCTX(CmdBindDescriptorSets)(bs->cmdbuf,
                                    is_compute ? VK_PIPELINE_BIND_POINT_COMPUTE : VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    pg->layout, 0, 1, &bs->dd.sets[is_compute][0],
                                    0, NULL);
         }
      }
   }
   ctx->dd.push_state_changed[is_compute] = false;
   if (zink_descriptor_mode == ZINK_DESCRIPTOR_MODE_DB)
      zink_descriptors_update_masked_buffer(ctx, is_compute, changed_sets, bind_sets);
   else
      zink_descriptors_update_masked(ctx, is_compute, changed_sets, bind_sets);
   /* bindless descriptors are context-based and get updated elsewhere */
   if (pg->dd.bindless && unlikely(!ctx->dd.bindless_bound)) {
      if (zink_descriptor_mode == ZINK_DESCRIPTOR_MODE_DB) {
         bind_bindless_db(ctx, pg);
      } else {
         VKCTX(CmdBindDescriptorSets)(ctx->batch.state->cmdbuf, is_compute ? VK_PIPELINE_BIND_POINT_COMPUTE : VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    pg->layout, screen->desc_set_id[ZINK_DESCRIPTOR_BINDLESS], 1, &ctx->dd.t.bindless_set,
                                    0, NULL);
      }
      ctx->dd.bindless_bound = true;
   }
   bs->dd.pg[is_compute] = pg;
   ctx->dd.pg[is_compute] = pg;
   bs->dd.compat_id[is_compute] = pg->compat_id;
   ctx->dd.state_changed[is_compute] = 0;
}

/* called from gallium descriptor change hooks, e.g., set_sampler_views */
void
zink_context_invalidate_descriptor_state(struct zink_context *ctx, gl_shader_stage shader, enum zink_descriptor_type type, unsigned start, unsigned count)
{
   if (type == ZINK_DESCRIPTOR_TYPE_UBO && !start)
      ctx->dd.push_state_changed[shader == MESA_SHADER_COMPUTE] = true;
   else
      ctx->dd.state_changed[shader == MESA_SHADER_COMPUTE] |= BITFIELD_BIT(type);
}
void
zink_context_invalidate_descriptor_state_compact(struct zink_context *ctx, gl_shader_stage shader, enum zink_descriptor_type type, unsigned start, unsigned count)
{
   if (type == ZINK_DESCRIPTOR_TYPE_UBO && !start)
      ctx->dd.push_state_changed[shader == MESA_SHADER_COMPUTE] = true;
   else {
      if (type > ZINK_DESCRIPTOR_TYPE_SAMPLER_VIEW)
         type -= ZINK_DESCRIPTOR_COMPACT;
      ctx->dd.state_changed[shader == MESA_SHADER_COMPUTE] |= BITFIELD_BIT(type);
   }
}

static void
deinit_multi_pool_overflow(struct zink_screen *screen, struct zink_descriptor_pool_multi *mpool)
{
   for (unsigned i = 0; i < 2; i++) {
      clear_multi_pool_overflow(screen, &mpool->overflowed_pools[i]);
      util_dynarray_fini(&mpool->overflowed_pools[i]);
   }
}

/* called during batch state destroy */
void
zink_batch_descriptor_deinit(struct zink_screen *screen, struct zink_batch_state *bs)
{
   for (unsigned i = 0; i < ZINK_DESCRIPTOR_BASE_TYPES; i++) {
      for (unsigned j = 0; j < bs->dd.pools[i].capacity / sizeof(struct zink_descriptor_pool_multi *); j++) {
         struct zink_descriptor_pool_multi **mppool = util_dynarray_element(&bs->dd.pools[i], struct zink_descriptor_pool_multi *, j);
         if (mppool && *mppool) {
            deinit_multi_pool_overflow(screen, *mppool);
            multi_pool_destroy(screen, *mppool);
         }
      }
      util_dynarray_fini(&bs->dd.pools[i]);
   }
   for (unsigned i = 0; i < 2; i++) {
      if (bs->dd.push_pool[i].pool)
         pool_destroy(screen, bs->dd.push_pool[i].pool);
      deinit_multi_pool_overflow(screen, &bs->dd.push_pool[i]);
   }

   if (bs->dd.db_xfer)
      zink_screen_buffer_unmap(&screen->base, bs->dd.db_xfer);
   bs->dd.db_xfer = NULL;
   if (bs->dd.db)
      screen->base.resource_destroy(&screen->base, &bs->dd.db->base.b);
   bs->dd.db = NULL;
   bs->dd.db_bound = false;
   bs->dd.db_offset = 0;
   memset(bs->dd.cur_db_offset, 0, sizeof(bs->dd.cur_db_offset));
}

/* ensure the idle/usable overflow set array always has as many members as possible by merging both arrays on batch state reset */
static void
consolidate_pool_alloc(struct zink_screen *screen, struct zink_descriptor_pool_multi *mpool)
{
   unsigned sizes[] = {
      util_dynarray_num_elements(&mpool->overflowed_pools[0], struct zink_descriptor_pool*),
      util_dynarray_num_elements(&mpool->overflowed_pools[1], struct zink_descriptor_pool*),
   };
   if (!sizes[0] && !sizes[1])
      return;
   /* set idx to whichever overflow is smaller */
   mpool->overflow_idx = sizes[0] > sizes[1];
   if (!mpool->overflowed_pools[mpool->overflow_idx].size)
      return;

   /* attempt to consolidate all the overflow into one array to maximize reuse */
   util_dynarray_append_dynarray(&mpool->overflowed_pools[!mpool->overflow_idx], &mpool->overflowed_pools[mpool->overflow_idx]);
   util_dynarray_clear(&mpool->overflowed_pools[mpool->overflow_idx]);
}

/* called when a batch state is reset, i.e., just before a batch state becomes the current state */
void
zink_batch_descriptor_reset(struct zink_screen *screen, struct zink_batch_state *bs)
{
   if (zink_descriptor_mode == ZINK_DESCRIPTOR_MODE_DB) {
      bs->dd.db_offset = 0;
      if (bs->dd.db && bs->dd.db->base.b.width0 < bs->ctx->dd.db.max_db_size * screen->base_descriptor_size)
         reinit_db(screen, bs);
      bs->dd.db_bound = false;
   } else {
      for (unsigned i = 0; i < ZINK_DESCRIPTOR_BASE_TYPES; i++) {
         struct zink_descriptor_pool_multi **mpools = bs->dd.pools[i].data;
         for (unsigned j = 0; j < bs->dd.pool_size[i]; j++) {
            struct zink_descriptor_pool_multi *mpool = mpools[j];
            if (!mpool)
               continue;
            consolidate_pool_alloc(screen, mpool);

            /* if the pool is still in use, reset the current set index */
            if (mpool->pool_key->use_count)
               mpool->pool->set_idx = 0;
            else {
               /* otherwise destroy it to reclaim memory */
               multi_pool_destroy(screen, mpool);
               mpools[j] = NULL;
            }
         }
      }
      for (unsigned i = 0; i < 2; i++) {
         if (bs->dd.push_pool[i].reinit_overflow) {
            /* these don't match current fbfetch usage and can never be used again */
            clear_multi_pool_overflow(screen, &bs->dd.push_pool[i].overflowed_pools[bs->dd.push_pool[i].overflow_idx]);
         } else if (bs->dd.push_pool[i].pool) {
            consolidate_pool_alloc(screen, &bs->dd.push_pool[i]);
         }
         if (bs->dd.push_pool[i].pool)
            bs->dd.push_pool[i].pool->set_idx = 0;
      }
   }
   memset(bs->dd.pg, 0, sizeof(bs->dd.pg));
}

/* called on batch state creation */
bool
zink_batch_descriptor_init(struct zink_screen *screen, struct zink_batch_state *bs)
{
   for (unsigned i = 0; i < ZINK_DESCRIPTOR_BASE_TYPES; i++)
      util_dynarray_init(&bs->dd.pools[i], bs);
   if (!screen->info.have_KHR_push_descriptor) {
      for (unsigned i = 0; i < 2; i++) {
         bs->dd.push_pool[i].pool = create_push_pool(screen, bs, i, false);
         util_dynarray_init(&bs->dd.push_pool[i].overflowed_pools[0], bs);
         util_dynarray_init(&bs->dd.push_pool[i].overflowed_pools[1], bs);
      }
   }

   if (zink_descriptor_mode == ZINK_DESCRIPTOR_MODE_DB && !(bs->ctx->flags & ZINK_CONTEXT_COPY_ONLY)) {
      unsigned bind = ZINK_BIND_DESCRIPTOR;
      struct pipe_resource *pres = pipe_buffer_create(&screen->base, bind, 0, bs->ctx->dd.db.max_db_size * screen->base_descriptor_size);
      if (!pres)
         return false;
      bs->dd.db = zink_resource(pres);
      bs->dd.db_map = pipe_buffer_map(&bs->ctx->base, pres, PIPE_MAP_READ | PIPE_MAP_WRITE | PIPE_MAP_PERSISTENT | PIPE_MAP_COHERENT | PIPE_MAP_THREAD_SAFE, &bs->dd.db_xfer);
   }
   return true;
}

static void
init_push_template_entry(VkDescriptorUpdateTemplateEntry *entry, unsigned i)
{
   entry->dstBinding = i;
   entry->descriptorCount = 1;
   entry->descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
   entry->offset = offsetof(struct zink_context, di.t.ubos[i][0]);
   entry->stride = sizeof(VkDescriptorBufferInfo);
}

/* called on context creation */
bool
zink_descriptors_init(struct zink_context *ctx)
{
   struct zink_screen *screen = zink_screen(ctx->base.screen);
   for (unsigned i = 0; i < ZINK_GFX_SHADER_COUNT; i++) {
      VkDescriptorUpdateTemplateEntry *entry = &ctx->dd.push_entries[i];
      init_push_template_entry(entry, i);
   }
   init_push_template_entry(&ctx->dd.compute_push_entry, MESA_SHADER_COMPUTE);
   VkDescriptorUpdateTemplateEntry *entry = &ctx->dd.push_entries[ZINK_GFX_SHADER_COUNT]; //fbfetch
   entry->dstBinding = ZINK_FBFETCH_BINDING;
   entry->descriptorCount = 1;
   entry->descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
   entry->offset = offsetof(struct zink_context, di.fbfetch);
   entry->stride = sizeof(VkDescriptorImageInfo);
   struct zink_descriptor_layout_key *layout_key;
   if (!zink_descriptor_util_push_layouts_get(ctx, ctx->dd.push_dsl, ctx->dd.push_layout_keys))
      return false;

   ctx->dd.dummy_dsl = descriptor_util_layout_get(screen, 0, NULL, 0, &layout_key);
   if (!ctx->dd.dummy_dsl)
      return false;

   if (zink_descriptor_mode == ZINK_DESCRIPTOR_MODE_DB) {
      VkDeviceSize val;
      for (unsigned i = 0; i < 2; i++) {
         VKSCR(GetDescriptorSetLayoutSizeEXT)(screen->dev, ctx->dd.push_dsl[i]->layout, &val);
         ctx->dd.db_size[i] = val;
      }
      for (unsigned i = 0; i < ZINK_GFX_SHADER_COUNT; i++) {
         VKSCR(GetDescriptorSetLayoutBindingOffsetEXT)(screen->dev, ctx->dd.push_dsl[0]->layout, i, &val);
         ctx->dd.db_offset[i] = val;
      }
      /* start small */
      ctx->dd.db.max_db_size = 250;
   }

   return true;
}

/* called on context destroy */
void
zink_descriptors_deinit(struct zink_context *ctx)
{
   struct zink_screen *screen = zink_screen(ctx->base.screen);
   if (ctx->dd.push_dsl[0])
      VKSCR(DestroyDescriptorSetLayout)(screen->dev, ctx->dd.push_dsl[0]->layout, NULL);
   if (ctx->dd.push_dsl[1])
      VKSCR(DestroyDescriptorSetLayout)(screen->dev, ctx->dd.push_dsl[1]->layout, NULL);
}

/* called on screen creation */
bool
zink_descriptor_layouts_init(struct zink_screen *screen)
{
   for (unsigned i = 0; i < ZINK_DESCRIPTOR_BASE_TYPES; i++) {
      if (!_mesa_hash_table_init(&screen->desc_set_layouts[i], screen, hash_descriptor_layout, equals_descriptor_layout))
         return false;
      if (!_mesa_set_init(&screen->desc_pool_keys[i], screen, hash_descriptor_pool_key, equals_descriptor_pool_key))
         return false;
   }
   simple_mtx_init(&screen->desc_set_layouts_lock, mtx_plain);
   simple_mtx_init(&screen->desc_pool_keys_lock, mtx_plain);
   return true;
}

/* called on screen destroy */
void
zink_descriptor_layouts_deinit(struct zink_screen *screen)
{
   for (unsigned i = 0; i < ZINK_DESCRIPTOR_BASE_TYPES; i++) {
      hash_table_foreach(&screen->desc_set_layouts[i], he) {
         struct zink_descriptor_layout *layout = he->data;
         VKSCR(DestroyDescriptorSetLayout)(screen->dev, layout->layout, NULL);
         ralloc_free(layout);
         _mesa_hash_table_remove(&screen->desc_set_layouts[i], he);
      }
   }
   simple_mtx_destroy(&screen->desc_set_layouts_lock);
   simple_mtx_destroy(&screen->desc_pool_keys_lock);
}

/* fbfetch descriptor is not initialized by default since it is seldom used
 * once it is needed, new push layouts/sets are allocated and all previous layouts/sets are destroyed
 */
void
zink_descriptor_util_init_fbfetch(struct zink_context *ctx)
{
   if (ctx->dd.has_fbfetch)
      return;

   struct zink_screen *screen = zink_screen(ctx->base.screen);
   VKSCR(DestroyDescriptorSetLayout)(screen->dev, ctx->dd.push_dsl[0]->layout, NULL);
   //don't free these now, let ralloc free on teardown to avoid invalid access
   //ralloc_free(ctx->dd.push_dsl[0]);
   //ralloc_free(ctx->dd.push_layout_keys[0]);
   ctx->dd.push_dsl[0] = create_gfx_layout(ctx, &ctx->dd.push_layout_keys[0], true);
   ctx->dd.has_fbfetch = true;

   if (zink_descriptor_mode == ZINK_DESCRIPTOR_MODE_DB) {
      VkDeviceSize val;
      VKSCR(GetDescriptorSetLayoutSizeEXT)(screen->dev, ctx->dd.push_dsl[0]->layout, &val);
      ctx->dd.db_size[0] = val;
      for (unsigned i = 0; i < ARRAY_SIZE(ctx->dd.db_offset); i++) {
         VKSCR(GetDescriptorSetLayoutBindingOffsetEXT)(screen->dev, ctx->dd.push_dsl[0]->layout, i, &val);
         ctx->dd.db_offset[i] = val;
      }
   }
}

/* called when a shader that uses bindless is created */
void
zink_descriptors_init_bindless(struct zink_context *ctx)
{
   if (ctx->dd.bindless_init)
      return;
   struct zink_screen *screen = zink_screen(ctx->base.screen);
   assert(screen->bindless_layout);
   ctx->dd.bindless_init = true;

   if (zink_descriptor_mode == ZINK_DESCRIPTOR_MODE_DB) {
      unsigned bind = ZINK_BIND_DESCRIPTOR;
      VkDeviceSize size;
      VKSCR(GetDescriptorSetLayoutSizeEXT)(screen->dev, screen->bindless_layout, &size);
      struct pipe_resource *pres = pipe_buffer_create(&screen->base, bind, 0, size);
      ctx->dd.db.bindless_db = zink_resource(pres);
      ctx->dd.db.bindless_db_map = pipe_buffer_map(&ctx->base, pres, PIPE_MAP_READ | PIPE_MAP_WRITE | PIPE_MAP_PERSISTENT, &ctx->dd.db.bindless_db_xfer);
      zink_batch_bind_db(ctx);
      for (unsigned i = 0; i < 4; i++) {
         VkDeviceSize offset;
         VKSCR(GetDescriptorSetLayoutBindingOffsetEXT)(screen->dev, screen->bindless_layout, i, &offset);
         ctx->dd.db.bindless_db_offsets[i] = offset;
      }
   } else {
      VkDescriptorPoolCreateInfo dpci = {0};
      VkDescriptorPoolSize sizes[4];
      for (unsigned i = 0; i < 4; i++) {
         sizes[i].type = zink_descriptor_type_from_bindless_index(i);
         sizes[i].descriptorCount = ZINK_MAX_BINDLESS_HANDLES;
      }
      dpci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
      dpci.pPoolSizes = sizes;
      dpci.poolSizeCount = 4;
      dpci.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
      dpci.maxSets = 1;
      VkResult result = VKSCR(CreateDescriptorPool)(screen->dev, &dpci, 0, &ctx->dd.t.bindless_pool);
      if (result != VK_SUCCESS) {
         mesa_loge("ZINK: vkCreateDescriptorPool failed (%s)", vk_Result_to_str(result));
         return;
      }

      zink_descriptor_util_alloc_sets(screen, screen->bindless_layout, ctx->dd.t.bindless_pool, &ctx->dd.t.bindless_set, 1);
   }
}

/* called on context destroy */
void
zink_descriptors_deinit_bindless(struct zink_context *ctx)
{
   struct zink_screen *screen = zink_screen(ctx->base.screen);
   if (zink_descriptor_mode == ZINK_DESCRIPTOR_MODE_DB) {
      if (ctx->dd.db.bindless_db_xfer)
         pipe_buffer_unmap(&ctx->base, ctx->dd.db.bindless_db_xfer);
      if (ctx->dd.db.bindless_db) {
         struct pipe_resource *pres = &ctx->dd.db.bindless_db->base.b;
         pipe_resource_reference(&pres, NULL);
      }
   } else {
      if (ctx->dd.t.bindless_pool)
         VKSCR(DestroyDescriptorPool)(screen->dev, ctx->dd.t.bindless_pool, NULL);
   }
}

/* entrypoint for updating bindless descriptors: called from draw/dispatch */
void
zink_descriptors_update_bindless(struct zink_context *ctx)
{
   struct zink_screen *screen = zink_screen(ctx->base.screen);
   VkDescriptorGetInfoEXT info;
   info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT;
   info.pNext = NULL;
   /* bindless descriptors are split between images and buffers */
   for (unsigned i = 0; i < 2; i++) {
      if (!ctx->di.bindless_dirty[i])
         continue;
      while (util_dynarray_contains(&ctx->di.bindless[i].updates, uint32_t)) {
         /* updates are tracked by handle */
         uint32_t handle = util_dynarray_pop(&ctx->di.bindless[i].updates, uint32_t);
         bool is_buffer = ZINK_BINDLESS_IS_BUFFER(handle);
         unsigned binding = i * 2 + !!is_buffer;
         if (zink_descriptor_mode == ZINK_DESCRIPTOR_MODE_DB) {
            if (is_buffer) {
               size_t size = i ? screen->info.db_props.robustStorageTexelBufferDescriptorSize : screen->info.db_props.robustUniformTexelBufferDescriptorSize;
               info.type = i ? VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER : VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
               info.data.pSampler = (void*)&ctx->di.bindless[i].db.buffer_infos[handle - ZINK_MAX_BINDLESS_HANDLES];
               VKSCR(GetDescriptorEXT)(screen->dev, &info, size, ctx->dd.db.bindless_db_map + ctx->dd.db.bindless_db_offsets[binding] + handle * size);
            } else {
               info.type = i ? VK_DESCRIPTOR_TYPE_STORAGE_IMAGE : VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
               if (screen->info.db_props.combinedImageSamplerDescriptorSingleArray || i) {
                  size_t size = i ? screen->info.db_props.storageImageDescriptorSize : screen->info.db_props.combinedImageSamplerDescriptorSize;
                  info.data.pSampler = (void*)&ctx->di.bindless[i].img_infos[handle];
                  VKSCR(GetDescriptorEXT)(screen->dev, &info, size, ctx->dd.db.bindless_db_map + ctx->dd.db.bindless_db_offsets[binding] + handle * size);
               } else {
                  /* drivers that don't support combinedImageSamplerDescriptorSingleArray must have sampler arrays written in memory as
                   *
                   *   | array_of_samplers[] | array_of_sampled_images[] |
                   * 
                   * which means each descriptor's data must be split
                   */
                  uint8_t buf[1024];
                  size_t size = screen->info.db_props.combinedImageSamplerDescriptorSize;
                  info.data.pSampler = (void*)&ctx->di.bindless[i].img_infos[handle];
                  VKSCR(GetDescriptorEXT)(screen->dev, &info, size, buf);
                  memcpy(ctx->dd.db.bindless_db_map + ctx->dd.db.bindless_db_offsets[binding] + handle * screen->info.db_props.samplerDescriptorSize, buf, screen->info.db_props.samplerDescriptorSize);
                  size_t offset = screen->info.db_props.samplerDescriptorSize * ZINK_MAX_BINDLESS_HANDLES;
                  offset += handle * screen->info.db_props.sampledImageDescriptorSize;
                  memcpy(ctx->dd.db.bindless_db_map + ctx->dd.db.bindless_db_offsets[binding] + offset, &buf[screen->info.db_props.samplerDescriptorSize], screen->info.db_props.sampledImageDescriptorSize);
               }
            }
         } else {
            VkWriteDescriptorSet wd;
            wd.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            wd.pNext = NULL;
            wd.dstSet = ctx->dd.t.bindless_set;
            wd.dstBinding = binding;
            /* buffer handle ids are offset by ZINK_MAX_BINDLESS_HANDLES for internal tracking */
            wd.dstArrayElement = is_buffer ? handle - ZINK_MAX_BINDLESS_HANDLES : handle;
            wd.descriptorCount = 1;
            wd.descriptorType = zink_descriptor_type_from_bindless_index(wd.dstBinding);
            if (is_buffer)
               wd.pTexelBufferView = &ctx->di.bindless[i].t.buffer_infos[wd.dstArrayElement];
            else
               wd.pImageInfo = &ctx->di.bindless[i].img_infos[handle];
            /* this sucks, but sets must be singly updated to be handled correctly */
            VKSCR(UpdateDescriptorSets)(screen->dev, 1, &wd, 0, NULL);
         }
      }
   }
   ctx->di.any_bindless_dirty = 0;
}
