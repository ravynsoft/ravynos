/*
 * Copyright © 2022 Collabora Ltd. and Red Hat Inc.
 * SPDX-License-Identifier: MIT
 */
#include "nvk_cmd_buffer.h"
#include "nvk_descriptor_set.h"
#include "nvk_descriptor_set_layout.h"
#include "nvk_shader.h"

#include "vk_pipeline.h"
#include "vk_pipeline_layout.h"

#include "nir_builder.h"
#include "nir_deref.h"

struct lower_desc_cbuf {
   struct nvk_cbuf key;

   uint32_t use_count;

   uint64_t start;
   uint64_t end;
};

static uint32_t
hash_cbuf(const void *data)
{
   return _mesa_hash_data(data, sizeof(struct nvk_cbuf));
}

static bool
cbufs_equal(const void *a, const void *b)
{
   return memcmp(a, b, sizeof(struct nvk_cbuf)) == 0;
}

static int
compar_cbufs(const void *_a, const void *_b)
{
   const struct lower_desc_cbuf *a = _a;
   const struct lower_desc_cbuf *b = _b;

#define COMPAR(field, pos) \
   if (a->field < b->field) return -(pos); \
   if (a->field > b->field) return (pos);

   /* Sort by most used first */
   COMPAR(use_count, -1)

   /* Keep the list stable by then sorting by key fields. */
   COMPAR(key.type, 1)
   COMPAR(key.desc_set, 1)
   COMPAR(key.dynamic_idx, 1)
   COMPAR(key.desc_offset, 1)

#undef COMPAR

   return 0;
}

struct lower_descriptors_ctx {
   const struct vk_pipeline_layout *layout;
   bool clamp_desc_array_bounds;
   nir_address_format ubo_addr_format;
   nir_address_format ssbo_addr_format;

   struct hash_table *cbufs;
   struct nvk_cbuf_map *cbuf_map;
};

static void
record_cbuf_use(const struct nvk_cbuf *key, uint64_t start, uint64_t end,
                struct lower_descriptors_ctx *ctx)
{
   struct hash_entry *entry = _mesa_hash_table_search(ctx->cbufs, key);
   if (entry != NULL) {
      struct lower_desc_cbuf *cbuf = entry->data;
      cbuf->use_count++;
      cbuf->start = MIN2(cbuf->start, start);
      cbuf->end = MAX2(cbuf->end, end);
   } else {
      struct lower_desc_cbuf *cbuf =
         ralloc(ctx->cbufs, struct lower_desc_cbuf);
      *cbuf = (struct lower_desc_cbuf) {
         .key = *key,
         .use_count = 1,
         .start = start,
         .end = end,
      };
      _mesa_hash_table_insert(ctx->cbufs, &cbuf->key, cbuf);
   }
}

static const struct nvk_descriptor_set_binding_layout *
get_binding_layout(uint32_t set, uint32_t binding,
                   const struct lower_descriptors_ctx *ctx)
{
   const struct vk_pipeline_layout *layout = ctx->layout;

   assert(set < layout->set_count);
   const struct nvk_descriptor_set_layout *set_layout =
      vk_to_nvk_descriptor_set_layout(layout->set_layouts[set]);

   assert(binding < set_layout->binding_count);
   return &set_layout->binding[binding];
}

static void
record_descriptor_cbuf_use(uint32_t set, uint32_t binding, nir_src *index,
                           struct lower_descriptors_ctx *ctx)
{
   const struct nvk_descriptor_set_binding_layout *binding_layout =
      get_binding_layout(set, binding, ctx);

   const struct nvk_cbuf key = {
      .type = NVK_CBUF_TYPE_DESC_SET,
      .desc_set = set,
   };

   uint64_t start, end;
   if (index == NULL) {
      /* When we don't have an index, assume 0 */
      start = binding_layout->offset;
      end = start + binding_layout->stride;
   } else if (nir_src_is_const(*index)) {
      start = binding_layout->offset +
              nir_src_as_uint(*index) * binding_layout->stride;
      end = start + binding_layout->stride;
   } else {
      start = binding_layout->offset;
      end = start + binding_layout->array_size * binding_layout->stride;
   }

   record_cbuf_use(&key, start, end, ctx);
}

static void
record_vulkan_resource_cbuf_use(nir_intrinsic_instr *intrin,
                                struct lower_descriptors_ctx *ctx)
{
   assert(intrin->intrinsic == nir_intrinsic_vulkan_resource_index);
   const VkDescriptorType desc_type = nir_intrinsic_desc_type(intrin);

   /* These we'll handle later */
   if (desc_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
       desc_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)
      return;

   record_descriptor_cbuf_use(nir_intrinsic_desc_set(intrin),
                              nir_intrinsic_binding(intrin),
                              &intrin->src[0], ctx);
}

static void
record_deref_descriptor_cbuf_use(nir_deref_instr *deref,
                                 struct lower_descriptors_ctx *ctx)
{
   nir_src *index_src = NULL;
   if (deref->deref_type == nir_deref_type_array) {
      index_src = &deref->arr.index;
      deref = nir_deref_instr_parent(deref);
   }

   assert(deref->deref_type == nir_deref_type_var);
   nir_variable *var = deref->var;

   record_descriptor_cbuf_use(var->data.descriptor_set,
                              var->data.binding,
                              index_src, ctx);
}

static void
record_tex_descriptor_cbuf_use(nir_tex_instr *tex,
                               struct lower_descriptors_ctx *ctx)
{
   const int texture_src_idx =
      nir_tex_instr_src_index(tex, nir_tex_src_texture_deref);
   const int sampler_src_idx =
      nir_tex_instr_src_index(tex, nir_tex_src_sampler_deref);

   if (texture_src_idx >= 0) {
      nir_deref_instr *deref = nir_src_as_deref(tex->src[texture_src_idx].src);
      record_deref_descriptor_cbuf_use(deref, ctx);
   }

   if (sampler_src_idx >= 0) {
      nir_deref_instr *deref = nir_src_as_deref(tex->src[sampler_src_idx].src);
      record_deref_descriptor_cbuf_use(deref, ctx);
   }
}

static struct nvk_cbuf
ubo_deref_to_cbuf(nir_deref_instr *deref,
                  nir_intrinsic_instr **resource_index_out,
                  uint64_t *offset_out,
                  uint64_t *start_out, uint64_t *end_out,
                  const struct lower_descriptors_ctx *ctx)
{
   assert(nir_deref_mode_is(deref, nir_var_mem_ubo));

   /* In case we early return */
   *offset_out = 0;
   *start_out = 0;
   *end_out = UINT64_MAX;
   *resource_index_out = NULL;

   const struct nvk_cbuf invalid = {
      .type = NVK_CBUF_TYPE_INVALID,
   };

   uint64_t offset = 0;
   uint64_t range = glsl_get_explicit_size(deref->type, false);
   bool offset_valid = true;
   while (deref->deref_type != nir_deref_type_cast) {
      nir_deref_instr *parent = nir_deref_instr_parent(deref);

      switch (deref->deref_type) {
      case nir_deref_type_var:
         unreachable("Buffers don't use variables in Vulkan");

      case nir_deref_type_array:
      case nir_deref_type_array_wildcard: {
         uint32_t stride = nir_deref_instr_array_stride(deref);
         if (deref->deref_type == nir_deref_type_array &&
             nir_src_is_const(deref->arr.index)) {
            offset += nir_src_as_uint(deref->arr.index) * stride;
         } else {
            range = glsl_get_length(parent->type) * stride;
         }
         break;
      }

      case nir_deref_type_ptr_as_array:
         /* All bets are off.  We shouldn't see these most of the time
          * anyway, even with variable pointers.
          */
         offset_valid = false;
         unreachable("Variable pointers aren't allowed on UBOs");
         break;

      case nir_deref_type_struct: {
         offset += glsl_get_struct_field_offset(parent->type,
                                                deref->strct.index);
         break;
      }

      default:
         unreachable("Unknown deref type");
      }

      deref = parent;
   }

   nir_intrinsic_instr *load_desc = nir_src_as_intrinsic(deref->parent);
   if (load_desc == NULL ||
       load_desc->intrinsic != nir_intrinsic_load_vulkan_descriptor)
      return invalid;

   nir_intrinsic_instr *res_index = nir_src_as_intrinsic(load_desc->src[0]);
   if (res_index == NULL ||
       res_index->intrinsic != nir_intrinsic_vulkan_resource_index)
      return invalid;

   /* We try to early return as little as possible prior to this point so we
    * can return the resource index intrinsic in as many cases as possible.
    * After this point, though, early returns are fair game.
    */
   *resource_index_out = res_index;

   if (!offset_valid || !nir_src_is_const(res_index->src[0]))
      return invalid;

   uint32_t set = nir_intrinsic_desc_set(res_index);
   uint32_t binding = nir_intrinsic_binding(res_index);
   uint32_t index = nir_src_as_uint(res_index->src[0]);

   const struct nvk_descriptor_set_binding_layout *binding_layout =
      get_binding_layout(set, binding, ctx);

   switch (binding_layout->type) {
   case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER: {
      *offset_out = 0;
      *start_out = offset;
      *end_out = offset + range;
      return (struct nvk_cbuf) {
         .type = NVK_CBUF_TYPE_UBO_DESC,
         .desc_set = set,
         .desc_offset = binding_layout->offset +
                        index * binding_layout->stride,
      };
   }

   case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC: {
      uint8_t dynamic_buffer_index =
         nvk_descriptor_set_layout_dynbuf_start(ctx->layout, set) +
         binding_layout->dynamic_buffer_index + index;

      *offset_out = 0;
      *start_out = offset;
      *end_out = offset + range;

      return (struct nvk_cbuf) {
         .type = NVK_CBUF_TYPE_DYNAMIC_UBO,
         .dynamic_idx = dynamic_buffer_index,
      };
   }

   case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK: {
      *offset_out = binding_layout->offset;
      *start_out = binding_layout->offset + offset;
      *end_out = *start_out + range;

      return (struct nvk_cbuf) {
         .type = NVK_CBUF_TYPE_DESC_SET,
         .desc_set = set,
      };
   }

   default:
      return invalid;
   }
}

static void
record_load_ubo_cbuf_uses(nir_deref_instr *deref,
                          struct lower_descriptors_ctx *ctx)
{
   assert(nir_deref_mode_is(deref, nir_var_mem_ubo));

   UNUSED uint64_t offset;
   uint64_t start, end;
   nir_intrinsic_instr *res_index;
   struct nvk_cbuf cbuf =
      ubo_deref_to_cbuf(deref, &res_index, &offset, &start, &end, ctx);

   if (cbuf.type != NVK_CBUF_TYPE_INVALID) {
      record_cbuf_use(&cbuf, start, end, ctx);
   } else if (res_index != NULL) {
      record_vulkan_resource_cbuf_use(res_index, ctx);
   }
}

static bool
record_cbuf_uses_instr(UNUSED nir_builder *b, nir_instr *instr, void *_ctx)
{
   struct lower_descriptors_ctx *ctx = _ctx;

   switch (instr->type) {
   case nir_instr_type_intrinsic: {
      nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
      switch (intrin->intrinsic) {
      case nir_intrinsic_vulkan_resource_index:
         record_vulkan_resource_cbuf_use(intrin, ctx);
         return false;

      case nir_intrinsic_load_deref: {
         nir_deref_instr *deref = nir_src_as_deref(intrin->src[0]);
         if (nir_deref_mode_is(deref, nir_var_mem_ubo))
            record_load_ubo_cbuf_uses(deref, ctx);
         return false;
      }

      case nir_intrinsic_image_deref_load:
      case nir_intrinsic_image_deref_store:
      case nir_intrinsic_image_deref_atomic:
      case nir_intrinsic_image_deref_atomic_swap:
      case nir_intrinsic_image_deref_size:
      case nir_intrinsic_image_deref_samples:
      case nir_intrinsic_image_deref_load_param_intel:
      case nir_intrinsic_image_deref_load_raw_intel:
      case nir_intrinsic_image_deref_store_raw_intel: {
         nir_deref_instr *deref = nir_src_as_deref(intrin->src[0]);
         record_deref_descriptor_cbuf_use(deref, ctx);
         return false;
      }

      default:
         return false;
      }
      unreachable("All cases return false");
   }

   case nir_instr_type_tex:
      record_tex_descriptor_cbuf_use(nir_instr_as_tex(instr), ctx);
      return false;

   default:
      return false;
   }
}

static void
build_cbuf_map(nir_shader *nir, struct lower_descriptors_ctx *ctx)
{
   ctx->cbufs = _mesa_hash_table_create(NULL, hash_cbuf, cbufs_equal);

   nir_shader_instructions_pass(nir, record_cbuf_uses_instr,
                                nir_metadata_all, (void *)ctx);

   struct lower_desc_cbuf *cbufs =
      ralloc_array(ctx->cbufs, struct lower_desc_cbuf,
                   _mesa_hash_table_num_entries(ctx->cbufs));

   uint32_t num_cbufs = 0;
   hash_table_foreach(ctx->cbufs, entry) {
      struct lower_desc_cbuf *cbuf = entry->data;

      /* We currently only start cbufs at the beginning so if it starts after
       * the max cbuf size, there's no point in including it in the list.
       */
      if (cbuf->start > NVK_MAX_CBUF_SIZE)
         continue;

      cbufs[num_cbufs++] = *cbuf;
   }

   qsort(cbufs, num_cbufs, sizeof(*cbufs), compar_cbufs);

   uint32_t mapped_cbuf_count = 0;

   /* Root descriptors always go in cbuf 0 */
   ctx->cbuf_map->cbufs[mapped_cbuf_count++] = (struct nvk_cbuf) {
      .type = NVK_CBUF_TYPE_ROOT_DESC,
   };

   uint8_t max_cbuf_bindings;
   if (nir->info.stage == MESA_SHADER_COMPUTE ||
       nir->info.stage == MESA_SHADER_KERNEL) {
      max_cbuf_bindings = 8;
   } else {
      max_cbuf_bindings = 16;
   }

   for (uint32_t i = 0; i < num_cbufs; i++) {
      if (mapped_cbuf_count >= max_cbuf_bindings)
         break;

      /* We can't support indirect cbufs in compute yet */
      if ((nir->info.stage == MESA_SHADER_COMPUTE ||
           nir->info.stage == MESA_SHADER_KERNEL) &&
          cbufs[i].key.type == NVK_CBUF_TYPE_UBO_DESC)
         continue;

      ctx->cbuf_map->cbufs[mapped_cbuf_count++] = cbufs[i].key;
   }
   ctx->cbuf_map->cbuf_count = mapped_cbuf_count;

   ralloc_free(ctx->cbufs);
   ctx->cbufs = NULL;
}

static int
get_mapped_cbuf_idx(const struct nvk_cbuf *key,
                    const struct lower_descriptors_ctx *ctx)
{
   if (ctx->cbuf_map == NULL)
      return -1;

   for (uint32_t c = 0; c < ctx->cbuf_map->cbuf_count; c++) {
      if (cbufs_equal(&ctx->cbuf_map->cbufs[c], key)) {
         return c;
      }
   }

   return -1;
}

static bool
lower_load_ubo_intrin(nir_builder *b, nir_intrinsic_instr *load, void *_ctx)
{
   const struct lower_descriptors_ctx *ctx = _ctx;

   if (load->intrinsic != nir_intrinsic_load_deref)
      return false;

   nir_deref_instr *deref = nir_src_as_deref(load->src[0]);
   if (!nir_deref_mode_is(deref, nir_var_mem_ubo))
      return false;

   uint64_t offset, end;
   UNUSED uint64_t start;
   UNUSED nir_intrinsic_instr *res_index;
   struct nvk_cbuf cbuf_key =
      ubo_deref_to_cbuf(deref, &res_index, &offset, &start, &end, ctx);

   if (cbuf_key.type == NVK_CBUF_TYPE_INVALID)
      return false;

   if (end > NVK_MAX_CBUF_SIZE)
      return false;

   int cbuf_idx = get_mapped_cbuf_idx(&cbuf_key, ctx);
   if (cbuf_idx < 0)
      return false;

   b->cursor = nir_before_instr(&load->instr);

   nir_deref_path path;
   nir_deref_path_init(&path, deref, NULL);

   nir_def *addr = nir_imm_ivec2(b, cbuf_idx, offset);
   nir_address_format addr_format = nir_address_format_32bit_index_offset;
   for (nir_deref_instr **p = &path.path[1]; *p != NULL; p++)
      addr = nir_explicit_io_address_from_deref(b, *p, addr, addr_format);

   nir_deref_path_finish(&path);

   nir_lower_explicit_io_instr(b, load, addr, addr_format);

   return true;
}

static nir_def *
load_descriptor_set_addr(nir_builder *b, uint32_t set,
                         UNUSED const struct lower_descriptors_ctx *ctx)
{
   uint32_t set_addr_offset =
      nvk_root_descriptor_offset(sets) + set * sizeof(uint64_t);

   return nir_load_ubo(b, 1, 64, nir_imm_int(b, 0),
                       nir_imm_int(b, set_addr_offset),
                       .align_mul = 8, .align_offset = 0, .range = ~0);
}

static nir_def *
load_descriptor(nir_builder *b, unsigned num_components, unsigned bit_size,
                uint32_t set, uint32_t binding, nir_def *index,
                unsigned offset_B, const struct lower_descriptors_ctx *ctx)
{
   const struct nvk_descriptor_set_binding_layout *binding_layout =
      get_binding_layout(set, binding, ctx);

   if (ctx->clamp_desc_array_bounds)
      index = nir_umin(b, index, nir_imm_int(b, binding_layout->array_size - 1));

   switch (binding_layout->type) {
   case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
   case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: {
      /* Get the index in the root descriptor table dynamic_buffers array. */
      uint8_t dynamic_buffer_start =
         nvk_descriptor_set_layout_dynbuf_start(ctx->layout, set);

      index = nir_iadd_imm(b, index,
                           dynamic_buffer_start +
                           binding_layout->dynamic_buffer_index);

      nir_def *root_desc_offset =
         nir_iadd_imm(b, nir_imul_imm(b, index, sizeof(struct nvk_buffer_address)),
                      nvk_root_descriptor_offset(dynamic_buffers));

      assert(num_components == 4 && bit_size == 32);
      nir_def *desc =
         nir_load_ubo(b, 4, 32, nir_imm_int(b, 0), root_desc_offset,
                      .align_mul = 16, .align_offset = 0, .range = ~0);
      /* We know a priori that the the .w compnent (offset) is zero */
      return nir_vec4(b, nir_channel(b, desc, 0),
                         nir_channel(b, desc, 1),
                         nir_channel(b, desc, 2),
                         nir_imm_int(b, 0));
   }

   case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK: {
      nir_def *base_addr =
         nir_iadd_imm(b, load_descriptor_set_addr(b, set, ctx),
                          binding_layout->offset);

      assert(binding_layout->stride == 1);
      const uint32_t binding_size = binding_layout->array_size;

      /* Convert it to nir_address_format_64bit_bounded_global */
      assert(num_components == 4 && bit_size == 32);
      return nir_vec4(b, nir_unpack_64_2x32_split_x(b, base_addr),
                         nir_unpack_64_2x32_split_y(b, base_addr),
                         nir_imm_int(b, binding_size),
                         nir_imm_int(b, 0));
   }

   default: {
      assert(binding_layout->stride > 0);
      nir_def *desc_ubo_offset =
         nir_iadd_imm(b, nir_imul_imm(b, index, binding_layout->stride),
                         binding_layout->offset + offset_B);

      unsigned desc_align_mul = (1 << (ffs(binding_layout->stride) - 1));
      desc_align_mul = MIN2(desc_align_mul, 16);
      unsigned desc_align_offset = binding_layout->offset + offset_B;
      desc_align_offset %= desc_align_mul;

      const struct nvk_cbuf cbuf_key = {
         .type = NVK_CBUF_TYPE_DESC_SET,
         .desc_set = set,
      };
      int cbuf_idx = get_mapped_cbuf_idx(&cbuf_key, ctx);

      nir_def *desc;
      if (cbuf_idx >= 0) {
         desc = nir_load_ubo(b, num_components, bit_size,
                             nir_imm_int(b, cbuf_idx),
                             desc_ubo_offset,
                             .align_mul = desc_align_mul,
                             .align_offset = desc_align_offset,
                             .range = ~0);
      } else {
         nir_def *set_addr = load_descriptor_set_addr(b, set, ctx);
         desc = nir_load_global_constant_offset(b, num_components, bit_size,
                                                set_addr, desc_ubo_offset,
                                                .align_mul = desc_align_mul,
                                                .align_offset = desc_align_offset);
      }
      if (binding_layout->type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
          binding_layout->type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) {
         /* We know a priori that the the .w compnent (offset) is zero */
         assert(num_components == 4 && bit_size == 32);
         desc = nir_vec4(b, nir_channel(b, desc, 0),
                            nir_channel(b, desc, 1),
                            nir_channel(b, desc, 2),
                            nir_imm_int(b, 0));
      }
      return desc;
   }
   }
}

static bool
is_idx_intrin(nir_intrinsic_instr *intrin)
{
   while (intrin->intrinsic == nir_intrinsic_vulkan_resource_reindex) {
      intrin = nir_src_as_intrinsic(intrin->src[0]);
      if (intrin == NULL)
         return false;
   }

   return intrin->intrinsic == nir_intrinsic_vulkan_resource_index;
}

static nir_def *
load_descriptor_for_idx_intrin(nir_builder *b, nir_intrinsic_instr *intrin,
                               const struct lower_descriptors_ctx *ctx)
{
   nir_def *index = nir_imm_int(b, 0);

   while (intrin->intrinsic == nir_intrinsic_vulkan_resource_reindex) {
      index = nir_iadd(b, index, intrin->src[1].ssa);
      intrin = nir_src_as_intrinsic(intrin->src[0]);
   }

   assert(intrin->intrinsic == nir_intrinsic_vulkan_resource_index);
   uint32_t set = nir_intrinsic_desc_set(intrin);
   uint32_t binding = nir_intrinsic_binding(intrin);
   index = nir_iadd(b, index, intrin->src[0].ssa);

   return load_descriptor(b, 4, 32, set, binding, index, 0, ctx);
}

static bool
try_lower_load_vulkan_descriptor(nir_builder *b, nir_intrinsic_instr *intrin,
                                 const struct lower_descriptors_ctx *ctx)
{
   ASSERTED const VkDescriptorType desc_type = nir_intrinsic_desc_type(intrin);
   b->cursor = nir_before_instr(&intrin->instr);

   nir_intrinsic_instr *idx_intrin = nir_src_as_intrinsic(intrin->src[0]);
   if (idx_intrin == NULL || !is_idx_intrin(idx_intrin)) {
      assert(desc_type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER ||
             desc_type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC);
      return false;
   }

   nir_def *desc = load_descriptor_for_idx_intrin(b, idx_intrin, ctx);

   nir_def_rewrite_uses(&intrin->def, desc);

   return true;
}

static bool
_lower_sysval_to_root_table(nir_builder *b, nir_intrinsic_instr *intrin,
                            uint32_t root_table_offset,
                            const struct lower_descriptors_ctx *ctx)
{
   b->cursor = nir_instr_remove(&intrin->instr);

   nir_def *val = nir_load_ubo(b, intrin->def.num_components,
                               intrin->def.bit_size,
                               nir_imm_int(b, 0), /* Root table */
                               nir_imm_int(b, root_table_offset),
                               .align_mul = 4,
                               .align_offset = 0,
                               .range = root_table_offset + 3 * 4);

   nir_def_rewrite_uses(&intrin->def, val);

   return true;
}

#define lower_sysval_to_root_table(b, intrin, member, ctx)           \
   _lower_sysval_to_root_table(b, intrin,                            \
                               nvk_root_descriptor_offset(member),   \
                               ctx)

static bool
lower_load_push_constant(nir_builder *b, nir_intrinsic_instr *load,
                         const struct lower_descriptors_ctx *ctx)
{
   const uint32_t push_region_offset =
      nvk_root_descriptor_offset(push);
   const uint32_t base = nir_intrinsic_base(load);

   b->cursor = nir_before_instr(&load->instr);

   nir_def *offset = nir_iadd_imm(b, load->src[0].ssa,
                                         push_region_offset + base);

   nir_def *val =
      nir_load_ubo(b, load->def.num_components, load->def.bit_size,
                   nir_imm_int(b, 0), offset,
                   .align_mul = load->def.bit_size / 8,
                   .align_offset = 0,
                   .range = push_region_offset + base +
                            nir_intrinsic_range(load));

   nir_def_rewrite_uses(&load->def, val);

   return true;
}

static void
get_resource_deref_binding(nir_builder *b, nir_deref_instr *deref,
                           uint32_t *set, uint32_t *binding,
                           nir_def **index)
{
   if (deref->deref_type == nir_deref_type_array) {
      *index = deref->arr.index.ssa;
      deref = nir_deref_instr_parent(deref);
   } else {
      *index = nir_imm_int(b, 0);
   }

   assert(deref->deref_type == nir_deref_type_var);
   nir_variable *var = deref->var;

   *set = var->data.descriptor_set;
   *binding = var->data.binding;
}

static nir_def *
load_resource_deref_desc(nir_builder *b, 
                         unsigned num_components, unsigned bit_size,
                         nir_deref_instr *deref, unsigned offset_B,
                         const struct lower_descriptors_ctx *ctx)
{
   uint32_t set, binding;
   nir_def *index;
   get_resource_deref_binding(b, deref, &set, &binding, &index);
   return load_descriptor(b, num_components, bit_size,
                          set, binding, index, offset_B, ctx);
}

static bool
lower_image_intrin(nir_builder *b, nir_intrinsic_instr *intrin,
                   const struct lower_descriptors_ctx *ctx)
{
   b->cursor = nir_before_instr(&intrin->instr);
   nir_deref_instr *deref = nir_src_as_deref(intrin->src[0]);
   nir_def *desc = load_resource_deref_desc(b, 1, 32, deref, 0, ctx);
   nir_rewrite_image_intrinsic(intrin, desc, true);

   /* We treat 3D images as 2D arrays */
   if (nir_intrinsic_image_dim(intrin) == GLSL_SAMPLER_DIM_3D) {
      assert(!nir_intrinsic_image_array(intrin));
      nir_intrinsic_set_image_dim(intrin, GLSL_SAMPLER_DIM_2D);
      nir_intrinsic_set_image_array(intrin, true);
   }

   return true;
}

static bool
lower_interp_at_sample(nir_builder *b, nir_intrinsic_instr *interp,
                       const struct lower_descriptors_ctx *ctx)
{
   const uint32_t root_table_offset =
      nvk_root_descriptor_offset(draw.sample_locations);

   nir_def *sample = interp->src[1].ssa;

   b->cursor = nir_before_instr(&interp->instr);

   nir_def *loc = nir_load_ubo(b, 1, 64,
                               nir_imm_int(b, 0), /* Root table */
                               nir_imm_int(b, root_table_offset),
                               .align_mul = 8,
                               .align_offset = 0,
                               .range = root_table_offset + 8);

   /* Yay little endian */
   loc = nir_ushr(b, loc, nir_imul_imm(b, sample, 8));
   nir_def *loc_x_u4 = nir_iand_imm(b, loc, 0xf);
   nir_def *loc_y_u4 = nir_iand_imm(b, nir_ushr_imm(b, loc, 4), 0xf);
   nir_def *loc_u4 = nir_vec2(b, loc_x_u4, loc_y_u4);
   nir_def *loc_f = nir_fmul_imm(b, nir_i2f32(b, loc_u4), 1.0 / 16.0);
   nir_def *offset = nir_fadd_imm(b, loc_f, -0.5);

   assert(interp->intrinsic == nir_intrinsic_interp_deref_at_sample);
   interp->intrinsic = nir_intrinsic_interp_deref_at_offset;
   nir_src_rewrite(&interp->src[1], offset);

   return true;
}

static bool
try_lower_intrin(nir_builder *b, nir_intrinsic_instr *intrin,
                 const struct lower_descriptors_ctx *ctx)
{
   switch (intrin->intrinsic) {
   case nir_intrinsic_load_vulkan_descriptor:
      return try_lower_load_vulkan_descriptor(b, intrin, ctx);

   case nir_intrinsic_load_workgroup_size:
      unreachable("Should have been lowered by nir_lower_cs_intrinsics()");

   case nir_intrinsic_load_num_workgroups:
      return lower_sysval_to_root_table(b, intrin, cs.group_count, ctx);

   case nir_intrinsic_load_base_workgroup_id:
      return lower_sysval_to_root_table(b, intrin, cs.base_group, ctx);

   case nir_intrinsic_load_push_constant:
      return lower_load_push_constant(b, intrin, ctx);

   case nir_intrinsic_load_base_vertex:
   case nir_intrinsic_load_first_vertex:
      return lower_sysval_to_root_table(b, intrin, draw.base_vertex, ctx);

   case nir_intrinsic_load_base_instance:
      return lower_sysval_to_root_table(b, intrin, draw.base_instance, ctx);

   case nir_intrinsic_load_draw_id:
      return lower_sysval_to_root_table(b, intrin, draw.draw_id, ctx);

   case nir_intrinsic_load_view_index:
      return lower_sysval_to_root_table(b, intrin, draw.view_index, ctx);

   case nir_intrinsic_image_deref_load:
   case nir_intrinsic_image_deref_store:
   case nir_intrinsic_image_deref_atomic:
   case nir_intrinsic_image_deref_atomic_swap:
   case nir_intrinsic_image_deref_size:
   case nir_intrinsic_image_deref_samples:
   case nir_intrinsic_image_deref_load_param_intel:
   case nir_intrinsic_image_deref_load_raw_intel:
   case nir_intrinsic_image_deref_store_raw_intel:
      return lower_image_intrin(b, intrin, ctx);

   case nir_intrinsic_interp_deref_at_sample:
      return lower_interp_at_sample(b, intrin, ctx);

   default:
      return false;
   }
}

static bool
lower_tex(nir_builder *b, nir_tex_instr *tex,
          const struct lower_descriptors_ctx *ctx)
{
   b->cursor = nir_before_instr(&tex->instr);

   const int texture_src_idx =
      nir_tex_instr_src_index(tex, nir_tex_src_texture_deref);
   const int sampler_src_idx =
      nir_tex_instr_src_index(tex, nir_tex_src_sampler_deref);
   if (texture_src_idx < 0) {
      assert(sampler_src_idx < 0);
      return false;
   }

   nir_deref_instr *texture = nir_src_as_deref(tex->src[texture_src_idx].src);
   nir_deref_instr *sampler = sampler_src_idx < 0 ? NULL :
                              nir_src_as_deref(tex->src[sampler_src_idx].src);
   assert(texture);

   nir_def *plane_ssa = nir_steal_tex_src(tex, nir_tex_src_plane);
   const uint32_t plane =
      plane_ssa ? nir_src_as_uint(nir_src_for_ssa(plane_ssa)) : 0;
   const uint64_t plane_offset_B = plane * sizeof(struct nvk_image_descriptor);

   nir_def *combined_handle;
   if (texture == sampler) {
      combined_handle = load_resource_deref_desc(b, 1, 32, texture, plane_offset_B, ctx);
   } else {
      nir_def *texture_desc =
         load_resource_deref_desc(b, 1, 32, texture, plane_offset_B, ctx);
      combined_handle = nir_iand_imm(b, texture_desc,
                                     NVK_IMAGE_DESCRIPTOR_IMAGE_INDEX_MASK);

      if (sampler != NULL) {
         nir_def *sampler_desc =
            load_resource_deref_desc(b, 1, 32, sampler, plane_offset_B, ctx);
         nir_def *sampler_index =
            nir_iand_imm(b, sampler_desc,
                         NVK_IMAGE_DESCRIPTOR_SAMPLER_INDEX_MASK);
         combined_handle = nir_ior(b, combined_handle, sampler_index);
      }
   }

   /* TODO: The nv50 back-end assumes it's 64-bit because of GL */
   combined_handle = nir_u2u64(b, combined_handle);

   /* TODO: The nv50 back-end assumes it gets handles both places, even for
    * texelFetch.
    */
   nir_src_rewrite(&tex->src[texture_src_idx].src, combined_handle);
   tex->src[texture_src_idx].src_type = nir_tex_src_texture_handle;

   if (sampler_src_idx < 0) {
      nir_tex_instr_add_src(tex, nir_tex_src_sampler_handle, combined_handle);
   } else {
      nir_src_rewrite(&tex->src[sampler_src_idx].src, combined_handle);
      tex->src[sampler_src_idx].src_type = nir_tex_src_sampler_handle;
   }

   return true;
}

static bool
try_lower_descriptors_instr(nir_builder *b, nir_instr *instr,
                            void *_data)
{
   const struct lower_descriptors_ctx *ctx = _data;

   switch (instr->type) {
   case nir_instr_type_tex:
      return lower_tex(b, nir_instr_as_tex(instr), ctx);
   case nir_instr_type_intrinsic:
      return try_lower_intrin(b, nir_instr_as_intrinsic(instr), ctx);
   default:
      return false;
   }
}

static bool
lower_ssbo_resource_index(nir_builder *b, nir_intrinsic_instr *intrin,
                          const struct lower_descriptors_ctx *ctx)
{
   const VkDescriptorType desc_type = nir_intrinsic_desc_type(intrin);
   if (desc_type != VK_DESCRIPTOR_TYPE_STORAGE_BUFFER &&
       desc_type != VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)
      return false;

   b->cursor = nir_instr_remove(&intrin->instr);

   uint32_t set = nir_intrinsic_desc_set(intrin);
   uint32_t binding = nir_intrinsic_binding(intrin);
   nir_def *index = intrin->src[0].ssa;

   const struct nvk_descriptor_set_binding_layout *binding_layout =
      get_binding_layout(set, binding, ctx);

   nir_def *binding_addr;
   uint8_t binding_stride;
   switch (binding_layout->type) {
   case VK_DESCRIPTOR_TYPE_MUTABLE_EXT:
   case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER: {
      nir_def *set_addr = load_descriptor_set_addr(b, set, ctx);
      binding_addr = nir_iadd_imm(b, set_addr, binding_layout->offset);
      binding_stride = binding_layout->stride;
      break;
   }

   case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: {
      const uint32_t root_desc_addr_offset =
         nvk_root_descriptor_offset(root_desc_addr);

      nir_def *root_desc_addr =
         nir_load_ubo(b, 1, 64, nir_imm_int(b, 0),
                      nir_imm_int(b, root_desc_addr_offset),
                      .align_mul = 8, .align_offset = 0, .range = ~0);

      const uint8_t dynamic_buffer_start =
         nvk_descriptor_set_layout_dynbuf_start(ctx->layout, set) +
         binding_layout->dynamic_buffer_index;

      const uint32_t dynamic_binding_offset =
         nvk_root_descriptor_offset(dynamic_buffers) +
         dynamic_buffer_start * sizeof(struct nvk_buffer_address);

      binding_addr = nir_iadd_imm(b, root_desc_addr, dynamic_binding_offset);
      binding_stride = sizeof(struct nvk_buffer_address);
      break;
   }

   default:
      unreachable("Not an SSBO descriptor");
   }

   /* Tuck the stride in the top 8 bits of the binding address */
   binding_addr = nir_ior_imm(b, binding_addr, (uint64_t)binding_stride << 56);

   const uint32_t binding_size = binding_layout->array_size * binding_stride;
   nir_def *offset_in_binding = nir_imul_imm(b, index, binding_stride);

   nir_def *addr;
   switch (ctx->ssbo_addr_format) {
   case nir_address_format_64bit_global:
      addr = nir_iadd(b, binding_addr, nir_u2u64(b, offset_in_binding));
      break;

   case nir_address_format_64bit_global_32bit_offset:
   case nir_address_format_64bit_bounded_global:
      addr = nir_vec4(b, nir_unpack_64_2x32_split_x(b, binding_addr),
                         nir_unpack_64_2x32_split_y(b, binding_addr),
                         nir_imm_int(b, binding_size),
                         offset_in_binding);
      break;

   default:
      unreachable("Unknown address mode");
   }

   nir_def_rewrite_uses(&intrin->def, addr);

   return true;
}

static bool
lower_ssbo_resource_reindex(nir_builder *b, nir_intrinsic_instr *intrin,
                            const struct lower_descriptors_ctx *ctx)
{
   const VkDescriptorType desc_type = nir_intrinsic_desc_type(intrin);
   if (desc_type != VK_DESCRIPTOR_TYPE_STORAGE_BUFFER &&
       desc_type != VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)
      return false;

   b->cursor = nir_instr_remove(&intrin->instr);

   nir_def *addr = intrin->src[0].ssa;
   nir_def *index = intrin->src[1].ssa;

   nir_def *addr_high32;
   switch (ctx->ssbo_addr_format) {
   case nir_address_format_64bit_global:
      addr_high32 = nir_unpack_64_2x32_split_y(b, addr);
      break;

   case nir_address_format_64bit_global_32bit_offset:
   case nir_address_format_64bit_bounded_global:
      addr_high32 = nir_channel(b, addr, 1);
      break;

   default:
      unreachable("Unknown address mode");
   }

   nir_def *stride = nir_ushr_imm(b, addr_high32, 24);
   nir_def *offset = nir_imul(b, index, stride);

   addr = nir_build_addr_iadd(b, addr, ctx->ssbo_addr_format,
                              nir_var_mem_ssbo, offset);
   nir_def_rewrite_uses(&intrin->def, addr);

   return true;
}

static bool
lower_load_ssbo_descriptor(nir_builder *b, nir_intrinsic_instr *intrin,
                           const struct lower_descriptors_ctx *ctx)
{
   const VkDescriptorType desc_type = nir_intrinsic_desc_type(intrin);
   if (desc_type != VK_DESCRIPTOR_TYPE_STORAGE_BUFFER &&
       desc_type != VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)
      return false;

   b->cursor = nir_instr_remove(&intrin->instr);

   nir_def *addr = intrin->src[0].ssa;

   nir_def *desc;
   switch (ctx->ssbo_addr_format) {
   case nir_address_format_64bit_global:
      /* Mask off the binding stride */
      addr = nir_iand_imm(b, addr, BITFIELD64_MASK(56));
      desc = nir_build_load_global_constant(b, 1, 64, addr,
                                            .align_mul = 16,
                                            .align_offset = 0);
      break;

   case nir_address_format_64bit_global_32bit_offset: {
      nir_def *base = nir_pack_64_2x32(b, nir_trim_vector(b, addr, 2));
      nir_def *offset = nir_channel(b, addr, 3);
      /* Mask off the binding stride */
      base = nir_iand_imm(b, base, BITFIELD64_MASK(56));
      desc = nir_load_global_constant_offset(b, 4, 32, base, offset,
                                             .align_mul = 16,
                                             .align_offset = 0);
      break;
   }

   case nir_address_format_64bit_bounded_global: {
      nir_def *base = nir_pack_64_2x32(b, nir_trim_vector(b, addr, 2));
      nir_def *size = nir_channel(b, addr, 2);
      nir_def *offset = nir_channel(b, addr, 3);
      /* Mask off the binding stride */
      base = nir_iand_imm(b, base, BITFIELD64_MASK(56));
      desc = nir_load_global_constant_bounded(b, 4, 32, base, offset, size,
                                              .align_mul = 16,
                                              .align_offset = 0);
      break;
   }

   default:
      unreachable("Unknown address mode");
   }

   nir_def_rewrite_uses(&intrin->def, desc);

   return true;
}

static bool
lower_ssbo_descriptor_instr(nir_builder *b, nir_instr *instr,
                            void *_data)
{
   const struct lower_descriptors_ctx *ctx = _data;

   if (instr->type != nir_instr_type_intrinsic)
      return false;

   nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
   switch (intrin->intrinsic) {
   case nir_intrinsic_vulkan_resource_index:
      return lower_ssbo_resource_index(b, intrin, ctx);
   case nir_intrinsic_vulkan_resource_reindex:
      return lower_ssbo_resource_reindex(b, intrin, ctx);
   case nir_intrinsic_load_vulkan_descriptor:
      return lower_load_ssbo_descriptor(b, intrin, ctx);
   default:
      return false;
   }
}

bool
nvk_nir_lower_descriptors(nir_shader *nir,
                          const struct vk_pipeline_robustness_state *rs,
                          const struct vk_pipeline_layout *layout,
                          struct nvk_cbuf_map *cbuf_map_out)
{
   struct lower_descriptors_ctx ctx = {
      .layout = layout,
      .clamp_desc_array_bounds =
         rs->storage_buffers != VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_DISABLED_EXT ||
         rs->uniform_buffers != VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_DISABLED_EXT ||
         rs->images != VK_PIPELINE_ROBUSTNESS_IMAGE_BEHAVIOR_DISABLED_EXT,
      .ssbo_addr_format = nvk_buffer_addr_format(rs->storage_buffers),
      .ubo_addr_format = nvk_buffer_addr_format(rs->uniform_buffers),
   };

   /* We run in four passes:
    *
    *  1. Find ranges of UBOs that we can promote to bound UBOs.  Nothing is
    *     actually lowered in this pass.  It's just analysis.
    *
    *  2. Try to lower UBO loads to cbufs based on the map we just created.
    *     We need to do this before the main lowering pass because it relies
    *     on the original descriptor load intrinsics.
    *
    *  3. Attempt to lower everything with direct descriptors.  This may fail
    *     to lower some SSBO intrinsics when variable pointers are used.
    *
    *  4. Clean up any SSBO intrinsics which are left and lower them to
    *     slightly less efficient but variable- pointers-correct versions.
    */

   bool pass_lower_ubo = false;
   if (cbuf_map_out != NULL) {
      ctx.cbuf_map = cbuf_map_out;
      build_cbuf_map(nir, &ctx);

      pass_lower_ubo =
         nir_shader_intrinsics_pass(nir, lower_load_ubo_intrin,
                                    nir_metadata_block_index |
                                    nir_metadata_dominance,
                                    (void *)&ctx);
   }

   bool pass_lower_descriptors =
      nir_shader_instructions_pass(nir, try_lower_descriptors_instr,
                                   nir_metadata_block_index |
                                   nir_metadata_dominance,
                                   (void *)&ctx);
   bool pass_lower_ssbo =
      nir_shader_instructions_pass(nir, lower_ssbo_descriptor_instr,
                                   nir_metadata_block_index |
                                   nir_metadata_dominance,
                                   (void *)&ctx);
   return pass_lower_ubo || pass_lower_descriptors || pass_lower_ssbo;
}
