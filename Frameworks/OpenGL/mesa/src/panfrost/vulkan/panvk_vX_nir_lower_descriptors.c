/*
 * Copyright © 2021 Collabora Ltd.
 *
 * Derived from tu_shader.c which is:
 * Copyright © 2019 Google LLC
 *
 * Also derived from anv_pipeline.c which is
 * Copyright © 2015 Intel Corporation
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

#include "panvk_private.h"

#include "nir.h"
#include "nir_builder.h"

struct apply_descriptors_ctx {
   const struct panvk_pipeline_layout *layout;
   bool add_bounds_checks;
   bool has_img_access;
   nir_address_format desc_addr_format;
   nir_address_format ubo_addr_format;
   nir_address_format ssbo_addr_format;
};

static nir_address_format
addr_format_for_desc_type(VkDescriptorType desc_type,
                          const struct apply_descriptors_ctx *ctx)
{
   switch (desc_type) {
   case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
   case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
      return ctx->ubo_addr_format;

   case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
   case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
      return ctx->ssbo_addr_format;

   default:
      unreachable("Unsupported descriptor type");
   }
}

static const struct panvk_descriptor_set_layout *
get_set_layout(uint32_t set, const struct apply_descriptors_ctx *ctx)
{
   return vk_to_panvk_descriptor_set_layout(ctx->layout->vk.set_layouts[set]);
}

static const struct panvk_descriptor_set_binding_layout *
get_binding_layout(uint32_t set, uint32_t binding,
                   const struct apply_descriptors_ctx *ctx)
{
   return &get_set_layout(set, ctx)->bindings[binding];
}

/** Build a Vulkan resource index
 *
 * A "resource index" is the term used by our SPIR-V parser and the relevant
 * NIR intrinsics for a reference into a descriptor set.  It acts much like a
 * deref in NIR except that it accesses opaque descriptors instead of memory.
 *
 * Coming out of SPIR-V, both the resource indices (in the form of
 * vulkan_resource_[re]index intrinsics) and the memory derefs (in the form
 * of nir_deref_instr) use the same vector component/bit size.  The meaning
 * of those values for memory derefs (nir_deref_instr) is given by the
 * nir_address_format associated with the descriptor type.  For resource
 * indices, it's an entirely internal to panvk encoding which describes, in
 * some sense, the address of the descriptor.  Thanks to the NIR/SPIR-V rules,
 * it must be packed into the same size SSA values as a memory address.  For
 * this reason, the actual encoding may depend both on the address format for
 * memory derefs and the descriptor address format.
 *
 * The load_vulkan_descriptor intrinsic exists to provide a transition point
 * between these two forms of derefs: descriptor and memory.
 */
static nir_def *
build_res_index(nir_builder *b, uint32_t set, uint32_t binding,
                nir_def *array_index, nir_address_format addr_format,
                const struct apply_descriptors_ctx *ctx)
{
   const struct panvk_descriptor_set_layout *set_layout =
      get_set_layout(set, ctx);
   const struct panvk_descriptor_set_binding_layout *bind_layout =
      &set_layout->bindings[binding];

   uint32_t array_size = bind_layout->array_size;

   switch (bind_layout->type) {
   case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
   case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC: {
      assert(addr_format == nir_address_format_32bit_index_offset);

      const unsigned ubo_idx =
         panvk_pipeline_layout_ubo_index(ctx->layout, set, binding, 0);

      const uint32_t packed = (array_size - 1) << 16 | ubo_idx;

      return nir_vec2(b, nir_imm_int(b, packed), array_index);
   }

   case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER: {
      assert(addr_format == nir_address_format_64bit_bounded_global ||
             addr_format == nir_address_format_64bit_global_32bit_offset);

      const unsigned set_ubo_idx =
         panvk_pipeline_layout_ubo_start(ctx->layout, set, false) +
         set_layout->desc_ubo_index;

      const uint32_t packed =
         (bind_layout->desc_ubo_stride << 16) | set_ubo_idx;

      return nir_vec4(b, nir_imm_int(b, packed),
                      nir_imm_int(b, bind_layout->desc_ubo_offset),
                      nir_imm_int(b, array_size - 1), array_index);
   }

   case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: {
      assert(addr_format == nir_address_format_64bit_bounded_global ||
             addr_format == nir_address_format_64bit_global_32bit_offset);

      const unsigned dyn_ssbo_idx =
         ctx->layout->sets[set].dyn_ssbo_offset + bind_layout->dyn_ssbo_idx;

      const unsigned ubo_idx = PANVK_SYSVAL_UBO_INDEX;
      const unsigned desc_stride = sizeof(struct panvk_ssbo_addr);
      const uint32_t ubo_offset =
         offsetof(struct panvk_sysvals, dyn_ssbos) + dyn_ssbo_idx * desc_stride;

      const uint32_t packed = (desc_stride << 16) | ubo_idx;

      return nir_vec4(b, nir_imm_int(b, packed), nir_imm_int(b, ubo_offset),
                      nir_imm_int(b, array_size - 1), array_index);
   }

   default:
      unreachable("Unsupported descriptor type");
   }
}

/** Adjust a Vulkan resource index
 *
 * This is the equivalent of nir_deref_type_ptr_as_array for resource indices.
 * For array descriptors, it allows us to adjust the array index.  Thanks to
 * variable pointers, we cannot always fold this re-index operation into the
 * vulkan_resource_index intrinsic and we have to do it based on nothing but
 * the address format.
 */
static nir_def *
build_res_reindex(nir_builder *b, nir_def *orig, nir_def *delta,
                  nir_address_format addr_format)
{
   switch (addr_format) {
   case nir_address_format_32bit_index_offset:
      return nir_vec2(b, nir_channel(b, orig, 0),
                      nir_iadd(b, nir_channel(b, orig, 1), delta));

   case nir_address_format_64bit_bounded_global:
   case nir_address_format_64bit_global_32bit_offset:
      return nir_vec4(b, nir_channel(b, orig, 0), nir_channel(b, orig, 1),
                      nir_channel(b, orig, 2),
                      nir_iadd(b, nir_channel(b, orig, 3), delta));

   default:
      unreachable("Unhandled address format");
   }
}

/** Convert a Vulkan resource index into a buffer address
 *
 * In some cases, this does a  memory load from the descriptor set and, in
 * others, it simply converts from one form to another.
 *
 * See build_res_index for details about each resource index format.
 */
static nir_def *
build_buffer_addr_for_res_index(nir_builder *b, nir_def *res_index,
                                nir_address_format addr_format,
                                const struct apply_descriptors_ctx *ctx)
{
   switch (addr_format) {
   case nir_address_format_32bit_index_offset: {
      nir_def *packed = nir_channel(b, res_index, 0);
      nir_def *array_index = nir_channel(b, res_index, 1);
      nir_def *surface_index = nir_extract_u16(b, packed, nir_imm_int(b, 0));
      nir_def *array_max = nir_extract_u16(b, packed, nir_imm_int(b, 1));

      if (ctx->add_bounds_checks)
         array_index = nir_umin(b, array_index, array_max);

      return nir_vec2(b, nir_iadd(b, surface_index, array_index),
                      nir_imm_int(b, 0));
   }

   case nir_address_format_64bit_bounded_global:
   case nir_address_format_64bit_global_32bit_offset: {
      nir_def *packed = nir_channel(b, res_index, 0);
      nir_def *desc_ubo_offset = nir_channel(b, res_index, 1);
      nir_def *array_max = nir_channel(b, res_index, 2);
      nir_def *array_index = nir_channel(b, res_index, 3);

      nir_def *desc_ubo_idx = nir_extract_u16(b, packed, nir_imm_int(b, 0));
      nir_def *desc_ubo_stride = nir_extract_u16(b, packed, nir_imm_int(b, 1));

      if (ctx->add_bounds_checks)
         array_index = nir_umin(b, array_index, array_max);

      desc_ubo_offset = nir_iadd(b, desc_ubo_offset,
                                 nir_imul(b, array_index, desc_ubo_stride));

      nir_def *desc = nir_load_ubo(b, 4, 32, desc_ubo_idx, desc_ubo_offset,
                                   .align_mul = 16, .range = ~0);

      /* The offset in the descriptor is guaranteed to be zero when it's
       * written into the descriptor set.  This lets us avoid some unnecessary
       * adds.
       */
      return nir_vec4(b, nir_channel(b, desc, 0), nir_channel(b, desc, 1),
                      nir_channel(b, desc, 2), nir_imm_int(b, 0));
   }

   default:
      unreachable("Unhandled address format");
   }
}

static bool
lower_res_intrinsic(nir_builder *b, nir_intrinsic_instr *intrin,
                    const struct apply_descriptors_ctx *ctx)
{
   b->cursor = nir_before_instr(&intrin->instr);

   const VkDescriptorType desc_type = nir_intrinsic_desc_type(intrin);
   nir_address_format addr_format = addr_format_for_desc_type(desc_type, ctx);

   nir_def *res;
   switch (intrin->intrinsic) {
   case nir_intrinsic_vulkan_resource_index:
      res = build_res_index(b, nir_intrinsic_desc_set(intrin),
                            nir_intrinsic_binding(intrin), intrin->src[0].ssa,
                            addr_format, ctx);
      break;

   case nir_intrinsic_vulkan_resource_reindex:
      res = build_res_reindex(b, intrin->src[0].ssa, intrin->src[1].ssa,
                              addr_format);
      break;

   case nir_intrinsic_load_vulkan_descriptor:
      res = build_buffer_addr_for_res_index(b, intrin->src[0].ssa, addr_format,
                                            ctx);
      break;

   default:
      unreachable("Unhandled resource intrinsic");
   }

   assert(intrin->def.bit_size == res->bit_size);
   assert(intrin->def.num_components == res->num_components);
   nir_def_rewrite_uses(&intrin->def, res);
   nir_instr_remove(&intrin->instr);

   return true;
}

static void
get_resource_deref_binding(nir_deref_instr *deref, uint32_t *set,
                           uint32_t *binding, uint32_t *index_imm,
                           nir_def **index_ssa)
{
   *index_imm = 0;
   *index_ssa = NULL;

   if (deref->deref_type == nir_deref_type_array) {
      if (index_imm != NULL && nir_src_is_const(deref->arr.index))
         *index_imm = nir_src_as_uint(deref->arr.index);
      else
         *index_ssa = deref->arr.index.ssa;

      deref = nir_deref_instr_parent(deref);
   }

   assert(deref->deref_type == nir_deref_type_var);
   nir_variable *var = deref->var;

   *set = var->data.descriptor_set;
   *binding = var->data.binding;
}

static nir_def *
load_resource_deref_desc(nir_builder *b, nir_deref_instr *deref,
                         unsigned desc_offset, unsigned num_components,
                         unsigned bit_size,
                         const struct apply_descriptors_ctx *ctx)
{
   uint32_t set, binding, index_imm;
   nir_def *index_ssa;
   get_resource_deref_binding(deref, &set, &binding, &index_imm, &index_ssa);

   const struct panvk_descriptor_set_layout *set_layout =
      get_set_layout(set, ctx);
   const struct panvk_descriptor_set_binding_layout *bind_layout =
      &set_layout->bindings[binding];

   assert(index_ssa == NULL || index_imm == 0);
   if (index_ssa == NULL)
      index_ssa = nir_imm_int(b, index_imm);

   const unsigned set_ubo_idx =
      panvk_pipeline_layout_ubo_start(ctx->layout, set, false) +
      set_layout->desc_ubo_index;

   nir_def *desc_ubo_offset =
      nir_iadd_imm(b, nir_imul_imm(b, index_ssa, bind_layout->desc_ubo_stride),
                   bind_layout->desc_ubo_offset + desc_offset);

   assert(bind_layout->desc_ubo_stride > 0);
   unsigned desc_align = (1 << (ffs(bind_layout->desc_ubo_stride) - 1));
   desc_align = MIN2(desc_align, 16);

   return nir_load_ubo(b, num_components, bit_size, nir_imm_int(b, set_ubo_idx),
                       desc_ubo_offset, .align_mul = desc_align,
                       .align_offset = (desc_offset % desc_align), .range = ~0);
}

static nir_def *
load_tex_img_size(nir_builder *b, nir_deref_instr *deref,
                  enum glsl_sampler_dim dim,
                  const struct apply_descriptors_ctx *ctx)
{
   if (dim == GLSL_SAMPLER_DIM_BUF) {
      return load_resource_deref_desc(b, deref, 0, 1, 32, ctx);
   } else {
      nir_def *desc = load_resource_deref_desc(b, deref, 0, 4, 16, ctx);

      /* The sizes are provided as 16-bit values with 1 subtracted so
       * convert to 32-bit and add 1.
       */
      return nir_iadd_imm(b, nir_u2u32(b, desc), 1);
   }
}

static nir_def *
load_tex_img_levels(nir_builder *b, nir_deref_instr *deref,
                    enum glsl_sampler_dim dim,
                    const struct apply_descriptors_ctx *ctx)
{
   assert(dim != GLSL_SAMPLER_DIM_BUF);
   nir_def *desc = load_resource_deref_desc(b, deref, 0, 4, 16, ctx);
   return nir_u2u32(b, nir_iand_imm(b, nir_channel(b, desc, 3), 0xff));
}

static nir_def *
load_tex_img_samples(nir_builder *b, nir_deref_instr *deref,
                     enum glsl_sampler_dim dim,
                     const struct apply_descriptors_ctx *ctx)
{
   assert(dim != GLSL_SAMPLER_DIM_BUF);
   nir_def *desc = load_resource_deref_desc(b, deref, 0, 4, 16, ctx);
   return nir_u2u32(b, nir_ushr_imm(b, nir_channel(b, desc, 3), 8));
}

static bool
lower_tex(nir_builder *b, nir_tex_instr *tex,
          const struct apply_descriptors_ctx *ctx)
{
   bool progress = false;

   b->cursor = nir_before_instr(&tex->instr);

   if (tex->op == nir_texop_txs || tex->op == nir_texop_query_levels ||
       tex->op == nir_texop_texture_samples) {
      int tex_src_idx = nir_tex_instr_src_index(tex, nir_tex_src_texture_deref);
      assert(tex_src_idx >= 0);
      nir_deref_instr *deref = nir_src_as_deref(tex->src[tex_src_idx].src);

      const enum glsl_sampler_dim dim = tex->sampler_dim;

      nir_def *res;
      switch (tex->op) {
      case nir_texop_txs:
         res = nir_channels(b, load_tex_img_size(b, deref, dim, ctx),
                            nir_component_mask(tex->def.num_components));
         break;
      case nir_texop_query_levels:
         assert(tex->def.num_components == 1);
         res = load_tex_img_levels(b, deref, dim, ctx);
         break;
      case nir_texop_texture_samples:
         assert(tex->def.num_components == 1);
         res = load_tex_img_samples(b, deref, dim, ctx);
         break;
      default:
         unreachable("Unsupported texture query op");
      }

      nir_def_rewrite_uses(&tex->def, res);
      nir_instr_remove(&tex->instr);
      return true;
   }

   int sampler_src_idx =
      nir_tex_instr_src_index(tex, nir_tex_src_sampler_deref);
   if (sampler_src_idx >= 0) {
      nir_deref_instr *deref = nir_src_as_deref(tex->src[sampler_src_idx].src);
      nir_tex_instr_remove_src(tex, sampler_src_idx);

      uint32_t set, binding, index_imm;
      nir_def *index_ssa;
      get_resource_deref_binding(deref, &set, &binding, &index_imm, &index_ssa);

      const struct panvk_descriptor_set_binding_layout *bind_layout =
         get_binding_layout(set, binding, ctx);

      tex->sampler_index = ctx->layout->sets[set].sampler_offset +
                           bind_layout->sampler_idx + index_imm;

      if (index_ssa != NULL) {
         nir_tex_instr_add_src(tex, nir_tex_src_sampler_offset, index_ssa);
      }
      progress = true;
   }

   int tex_src_idx = nir_tex_instr_src_index(tex, nir_tex_src_texture_deref);
   if (tex_src_idx >= 0) {
      nir_deref_instr *deref = nir_src_as_deref(tex->src[tex_src_idx].src);
      nir_tex_instr_remove_src(tex, tex_src_idx);

      uint32_t set, binding, index_imm;
      nir_def *index_ssa;
      get_resource_deref_binding(deref, &set, &binding, &index_imm, &index_ssa);

      const struct panvk_descriptor_set_binding_layout *bind_layout =
         get_binding_layout(set, binding, ctx);

      tex->texture_index =
         ctx->layout->sets[set].tex_offset + bind_layout->tex_idx + index_imm;

      if (index_ssa != NULL) {
         nir_tex_instr_add_src(tex, nir_tex_src_texture_offset, index_ssa);
      }
      progress = true;
   }

   return progress;
}

static nir_def *
get_img_index(nir_builder *b, nir_deref_instr *deref,
              const struct apply_descriptors_ctx *ctx)
{
   uint32_t set, binding, index_imm;
   nir_def *index_ssa;
   get_resource_deref_binding(deref, &set, &binding, &index_imm, &index_ssa);

   const struct panvk_descriptor_set_binding_layout *bind_layout =
      get_binding_layout(set, binding, ctx);
   assert(bind_layout->type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE ||
          bind_layout->type == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER ||
          bind_layout->type == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER);

   unsigned img_offset =
      ctx->layout->sets[set].img_offset + bind_layout->img_idx;

   if (index_ssa == NULL) {
      return nir_imm_int(b, img_offset + index_imm);
   } else {
      assert(index_imm == 0);
      return nir_iadd_imm(b, index_ssa, img_offset);
   }
}

static bool
lower_img_intrinsic(nir_builder *b, nir_intrinsic_instr *intr,
                    struct apply_descriptors_ctx *ctx)
{
   b->cursor = nir_before_instr(&intr->instr);
   nir_deref_instr *deref = nir_src_as_deref(intr->src[0]);

   if (intr->intrinsic == nir_intrinsic_image_deref_size ||
       intr->intrinsic == nir_intrinsic_image_deref_samples) {
      const enum glsl_sampler_dim dim = nir_intrinsic_image_dim(intr);

      nir_def *res;
      switch (intr->intrinsic) {
      case nir_intrinsic_image_deref_size:
         res = nir_channels(b, load_tex_img_size(b, deref, dim, ctx),
                            nir_component_mask(intr->def.num_components));
         break;
      case nir_intrinsic_image_deref_samples:
         res = load_tex_img_samples(b, deref, dim, ctx);
         break;
      default:
         unreachable("Unsupported image query op");
      }

      nir_def_rewrite_uses(&intr->def, res);
      nir_instr_remove(&intr->instr);
   } else {
      nir_rewrite_image_intrinsic(intr, get_img_index(b, deref, ctx), false);
      ctx->has_img_access = true;
   }

   return true;
}

static bool
lower_intrinsic(nir_builder *b, nir_intrinsic_instr *intr,
                struct apply_descriptors_ctx *ctx)
{
   switch (intr->intrinsic) {
   case nir_intrinsic_vulkan_resource_index:
   case nir_intrinsic_vulkan_resource_reindex:
   case nir_intrinsic_load_vulkan_descriptor:
      return lower_res_intrinsic(b, intr, ctx);
   case nir_intrinsic_image_deref_store:
   case nir_intrinsic_image_deref_load:
   case nir_intrinsic_image_deref_atomic:
   case nir_intrinsic_image_deref_atomic_swap:
   case nir_intrinsic_image_deref_size:
   case nir_intrinsic_image_deref_samples:
      return lower_img_intrinsic(b, intr, ctx);
   default:
      return false;
   }
}

static bool
lower_descriptors_instr(nir_builder *b, nir_instr *instr, void *data)
{
   struct apply_descriptors_ctx *ctx = data;

   switch (instr->type) {
   case nir_instr_type_tex:
      return lower_tex(b, nir_instr_as_tex(instr), ctx);
   case nir_instr_type_intrinsic:
      return lower_intrinsic(b, nir_instr_as_intrinsic(instr), ctx);
   default:
      return false;
   }
}

bool
panvk_per_arch(nir_lower_descriptors)(nir_shader *nir, struct panvk_device *dev,
                                      const struct panvk_pipeline_layout *layout,
                                      bool *has_img_access_out)
{
   struct apply_descriptors_ctx ctx = {
      .layout = layout,
      .desc_addr_format = nir_address_format_32bit_index_offset,
      .ubo_addr_format = nir_address_format_32bit_index_offset,
      .ssbo_addr_format = dev->vk.enabled_features.robustBufferAccess
                             ? nir_address_format_64bit_bounded_global
                             : nir_address_format_64bit_global_32bit_offset,
   };

   bool progress = nir_shader_instructions_pass(
      nir, lower_descriptors_instr,
      nir_metadata_block_index | nir_metadata_dominance, (void *)&ctx);
   if (has_img_access_out)
      *has_img_access_out = ctx.has_img_access;

   return progress;
}
