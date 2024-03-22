/*
 * Copyright 2022 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

/*
 * This lowering pass converts index based buffer/image/texture access to
 * explicite descriptor based, which simplify the compiler backend translation.
 *
 * For example: load_ubo(1) -> load_ubo(vec4), where the vec4 is the buffer
 * descriptor with index==1, so compiler backend don't need to do index-to-descriptor
 * finding which is the most complicated part (move to nir now).
 */

#include "nir_builder.h"

#include "ac_nir.h"
#include "si_pipe.h"
#include "si_shader_internal.h"
#include "sid.h"

struct lower_resource_state {
   struct si_shader *shader;
   struct si_shader_args *args;
};

static nir_def *load_ubo_desc_fast_path(nir_builder *b, nir_def *addr_lo,
                                            struct si_shader_selector *sel)
{
   nir_def *addr_hi =
      nir_imm_int(b, S_008F04_BASE_ADDRESS_HI(sel->screen->info.address32_hi));

   uint32_t rsrc3 =
      S_008F0C_DST_SEL_X(V_008F0C_SQ_SEL_X) | S_008F0C_DST_SEL_Y(V_008F0C_SQ_SEL_Y) |
      S_008F0C_DST_SEL_Z(V_008F0C_SQ_SEL_Z) | S_008F0C_DST_SEL_W(V_008F0C_SQ_SEL_W);

   if (sel->screen->info.gfx_level >= GFX11)
      rsrc3 |= S_008F0C_FORMAT(V_008F0C_GFX11_FORMAT_32_FLOAT) |
               S_008F0C_OOB_SELECT(V_008F0C_OOB_SELECT_RAW);
   else if (sel->screen->info.gfx_level >= GFX10)
      rsrc3 |= S_008F0C_FORMAT(V_008F0C_GFX10_FORMAT_32_FLOAT) |
               S_008F0C_OOB_SELECT(V_008F0C_OOB_SELECT_RAW) | S_008F0C_RESOURCE_LEVEL(1);
   else
      rsrc3 |= S_008F0C_NUM_FORMAT(V_008F0C_BUF_NUM_FORMAT_FLOAT) |
               S_008F0C_DATA_FORMAT(V_008F0C_BUF_DATA_FORMAT_32);

   return nir_vec4(b, addr_lo, addr_hi, nir_imm_int(b, sel->info.constbuf0_num_slots * 16),
                   nir_imm_int(b, rsrc3));
}

static nir_def *clamp_index(nir_builder *b, nir_def *index, unsigned max)
{
   if (util_is_power_of_two_or_zero(max))
      return nir_iand_imm(b, index, max - 1);
   else {
      nir_def *clamp = nir_imm_int(b, max - 1);
      nir_def *cond = nir_uge(b, clamp, index);
      return nir_bcsel(b, cond, index, clamp);
   }
}

static nir_def *load_ubo_desc(nir_builder *b, nir_def *index,
                                  struct lower_resource_state *s)
{
   struct si_shader_selector *sel = s->shader->selector;

   nir_def *addr = ac_nir_load_arg(b, &s->args->ac, s->args->const_and_shader_buffers);

   if (sel->info.base.num_ubos == 1 && sel->info.base.num_ssbos == 0)
      return load_ubo_desc_fast_path(b, addr, sel);

   index = clamp_index(b, index, sel->info.base.num_ubos);
   index = nir_iadd_imm(b, index, SI_NUM_SHADER_BUFFERS);

   nir_def *offset = nir_ishl_imm(b, index, 4);
   return nir_load_smem_amd(b, 4, addr, offset);
}

static nir_def *load_ssbo_desc(nir_builder *b, nir_src *index,
                                   struct lower_resource_state *s)
{
   struct si_shader_selector *sel = s->shader->selector;

   /* Fast path if the shader buffer is in user SGPRs. */
   if (nir_src_is_const(*index)) {
      unsigned slot = nir_src_as_uint(*index);
      if (slot < sel->cs_num_shaderbufs_in_user_sgprs)
         return ac_nir_load_arg(b, &s->args->ac, s->args->cs_shaderbuf[slot]);
   }

   nir_def *addr = ac_nir_load_arg(b, &s->args->ac, s->args->const_and_shader_buffers);
   nir_def *slot = clamp_index(b, index->ssa, sel->info.base.num_ssbos);
   slot = nir_isub_imm(b, SI_NUM_SHADER_BUFFERS - 1, slot);

   nir_def *offset = nir_ishl_imm(b, slot, 4);
   return nir_load_smem_amd(b, 4, addr, offset);
}

static nir_def *fixup_image_desc(nir_builder *b, nir_def *rsrc, bool uses_store,
                                     struct lower_resource_state *s)
{
   struct si_shader_selector *sel = s->shader->selector;
   struct si_screen *screen = sel->screen;

   /**
    * Given a 256-bit resource descriptor, force the DCC enable bit to off.
    *
    * At least on Tonga, executing image stores on images with DCC enabled and
    * non-trivial can eventually lead to lockups. This can occur when an
    * application binds an image as read-only but then uses a shader that writes
    * to it. The OpenGL spec allows almost arbitrarily bad behavior (including
    * program termination) in this case, but it doesn't cost much to be a bit
    * nicer: disabling DCC in the shader still leads to undefined results but
    * avoids the lockup.
    */
   if (uses_store &&
       screen->info.gfx_level <= GFX9 &&
       screen->info.gfx_level >= GFX8) {
      nir_def *tmp = nir_channel(b, rsrc, 6);
      tmp = nir_iand_imm(b, tmp, C_008F28_COMPRESSION_EN);
      rsrc = nir_vector_insert_imm(b, rsrc, tmp, 6);
   }

   if (!uses_store &&
       screen->info.has_image_load_dcc_bug &&
       screen->always_allow_dcc_stores) {
      nir_def *tmp = nir_channel(b, rsrc, 6);
      tmp = nir_iand_imm(b, tmp, C_00A018_WRITE_COMPRESS_ENABLE);
      rsrc = nir_vector_insert_imm(b, rsrc, tmp, 6);
   }

   return rsrc;
}

/* AC_DESC_FMASK is handled exactly like AC_DESC_IMAGE. The caller should
 * adjust "index" to point to FMASK.
 */
static nir_def *load_image_desc(nir_builder *b, nir_def *list, nir_def *index,
                                    enum ac_descriptor_type desc_type, bool uses_store,
                                    struct lower_resource_state *s)
{
   /* index is in uvec8 unit, convert to offset in bytes */
   nir_def *offset = nir_ishl_imm(b, index, 5);

   unsigned num_channels;
   if (desc_type == AC_DESC_BUFFER) {
      offset = nir_iadd_imm(b, offset, 16);
      num_channels = 4;
   } else {
      assert(desc_type == AC_DESC_IMAGE || desc_type == AC_DESC_FMASK);
      num_channels = 8;
   }

   nir_def *rsrc = nir_load_smem_amd(b, num_channels, list, offset);

   if (desc_type == AC_DESC_IMAGE)
      rsrc = fixup_image_desc(b, rsrc, uses_store, s);

   return rsrc;
}

static nir_def *deref_to_index(nir_builder *b,
                                   nir_deref_instr *deref,
                                   unsigned max_slots,
                                   nir_def **dynamic_index_ret,
                                   unsigned *const_index_ret)
{
   unsigned const_index = 0;
   nir_def *dynamic_index = NULL;
   while (deref->deref_type != nir_deref_type_var) {
      assert(deref->deref_type == nir_deref_type_array);
      unsigned array_size = MAX2(glsl_get_aoa_size(deref->type), 1);

      if (nir_src_is_const(deref->arr.index)) {
         const_index += array_size * nir_src_as_uint(deref->arr.index);
      } else {
         nir_def *tmp = nir_imul_imm(b, deref->arr.index.ssa, array_size);
         dynamic_index = dynamic_index ? nir_iadd(b, dynamic_index, tmp) : tmp;
      }

      deref = nir_deref_instr_parent(deref);
   }

   unsigned base_index = deref->var->data.binding;
   const_index += base_index;

   /* Redirect invalid resource indices to the first array element. */
   if (const_index >= max_slots)
      const_index = base_index;

   nir_def *index = nir_imm_int(b, const_index);
   if (dynamic_index) {
      index = nir_iadd(b, dynamic_index, index);

      /* From the GL_ARB_shader_image_load_store extension spec:
       *
       *    If a shader performs an image load, store, or atomic
       *    operation using an image variable declared as an array,
       *    and if the index used to select an individual element is
       *    negative or greater than or equal to the size of the
       *    array, the results of the operation are undefined but may
       *    not lead to termination.
       */
      index = clamp_index(b, index, max_slots);
   }

   if (dynamic_index_ret)
      *dynamic_index_ret = dynamic_index;
   if (const_index_ret)
      *const_index_ret = const_index;

   return index;
}

static nir_def *load_deref_image_desc(nir_builder *b, nir_deref_instr *deref,
                                          enum ac_descriptor_type desc_type, bool is_load,
                                          struct lower_resource_state *s)
{
   unsigned const_index;
   nir_def *dynamic_index;
   nir_def *index = deref_to_index(b, deref, s->shader->selector->info.base.num_images,
                                       &dynamic_index, &const_index);

   nir_def *desc;
   if (!dynamic_index && desc_type != AC_DESC_FMASK &&
       const_index < s->shader->selector->cs_num_images_in_user_sgprs) {
      /* Fast path if the image is in user SGPRs. */
      desc = ac_nir_load_arg(b, &s->args->ac, s->args->cs_image[const_index]);

      if (desc_type == AC_DESC_IMAGE)
         desc = fixup_image_desc(b, desc, !is_load, s);
   } else {
      /* FMASKs are separate from images. */
      if (desc_type == AC_DESC_FMASK)
         index = nir_iadd_imm(b, index, SI_NUM_IMAGES);

      index = nir_isub_imm(b, SI_NUM_IMAGE_SLOTS - 1, index);

      nir_def *list = ac_nir_load_arg(b, &s->args->ac, s->args->samplers_and_images);
      desc = load_image_desc(b, list, index, desc_type, !is_load, s);
   }

   return desc;
}

static nir_def *load_bindless_image_desc(nir_builder *b, nir_def *index,
                                             enum ac_descriptor_type desc_type, bool is_load,
                                             struct lower_resource_state *s)
{
   /* Bindless image descriptors use 16-dword slots. */
   index = nir_ishl_imm(b, index, 1);

   /* FMASK is right after the image. */
   if (desc_type == AC_DESC_FMASK)
      index = nir_iadd_imm(b, index, 1);

   nir_def *list = ac_nir_load_arg(b, &s->args->ac, s->args->bindless_samplers_and_images);
   return load_image_desc(b, list, index, desc_type, !is_load, s);
}

static bool lower_resource_intrinsic(nir_builder *b, nir_intrinsic_instr *intrin,
                                     struct lower_resource_state *s)
{
   switch (intrin->intrinsic) {
   case nir_intrinsic_load_ubo: {
      assert(!(nir_intrinsic_access(intrin) & ACCESS_NON_UNIFORM));

      nir_def *desc = load_ubo_desc(b, intrin->src[0].ssa, s);
      nir_src_rewrite(&intrin->src[0], desc);
      break;
   }
   case nir_intrinsic_load_ssbo:
   case nir_intrinsic_ssbo_atomic:
   case nir_intrinsic_ssbo_atomic_swap: {
      assert(!(nir_intrinsic_access(intrin) & ACCESS_NON_UNIFORM));

      nir_def *desc = load_ssbo_desc(b, &intrin->src[0], s);
      nir_src_rewrite(&intrin->src[0], desc);
      break;
   }
   case nir_intrinsic_store_ssbo: {
      assert(!(nir_intrinsic_access(intrin) & ACCESS_NON_UNIFORM));

      nir_def *desc = load_ssbo_desc(b, &intrin->src[1], s);
      nir_src_rewrite(&intrin->src[1], desc);
      break;
   }
   case nir_intrinsic_get_ssbo_size: {
      assert(!(nir_intrinsic_access(intrin) & ACCESS_NON_UNIFORM));

      nir_def *desc = load_ssbo_desc(b, &intrin->src[0], s);
      nir_def *size = nir_channel(b, desc, 2);
      nir_def_rewrite_uses(&intrin->def, size);
      nir_instr_remove(&intrin->instr);
      break;
   }
   case nir_intrinsic_image_deref_load:
   case nir_intrinsic_image_deref_sparse_load:
   case nir_intrinsic_image_deref_fragment_mask_load_amd:
   case nir_intrinsic_image_deref_store:
   case nir_intrinsic_image_deref_atomic:
   case nir_intrinsic_image_deref_atomic_swap:
   case nir_intrinsic_image_deref_descriptor_amd: {
      assert(!(nir_intrinsic_access(intrin) & ACCESS_NON_UNIFORM));

      nir_deref_instr *deref = nir_src_as_deref(intrin->src[0]);

      enum ac_descriptor_type desc_type;
      if (intrin->intrinsic == nir_intrinsic_image_deref_fragment_mask_load_amd) {
         desc_type = AC_DESC_FMASK;
      } else {
         enum glsl_sampler_dim dim = glsl_get_sampler_dim(deref->type);
         desc_type = dim == GLSL_SAMPLER_DIM_BUF ? AC_DESC_BUFFER : AC_DESC_IMAGE;
      }

      bool is_load =
         intrin->intrinsic == nir_intrinsic_image_deref_load ||
         intrin->intrinsic == nir_intrinsic_image_deref_sparse_load ||
         intrin->intrinsic == nir_intrinsic_image_deref_fragment_mask_load_amd ||
         intrin->intrinsic == nir_intrinsic_image_deref_descriptor_amd;

      nir_def *desc = load_deref_image_desc(b, deref, desc_type, is_load, s);

      if (intrin->intrinsic == nir_intrinsic_image_deref_descriptor_amd) {
         nir_def_rewrite_uses(&intrin->def, desc);
         nir_instr_remove(&intrin->instr);
      } else {
         nir_intrinsic_set_image_dim(intrin, glsl_get_sampler_dim(deref->type));
         nir_intrinsic_set_image_array(intrin, glsl_sampler_type_is_array(deref->type));
         nir_rewrite_image_intrinsic(intrin, desc, true);
      }
      break;
   }
   case nir_intrinsic_bindless_image_load:
   case nir_intrinsic_bindless_image_sparse_load:
   case nir_intrinsic_bindless_image_fragment_mask_load_amd:
   case nir_intrinsic_bindless_image_store:
   case nir_intrinsic_bindless_image_atomic:
   case nir_intrinsic_bindless_image_atomic_swap: {
      assert(!(nir_intrinsic_access(intrin) & ACCESS_NON_UNIFORM));

      enum ac_descriptor_type desc_type;
      if (intrin->intrinsic == nir_intrinsic_bindless_image_fragment_mask_load_amd) {
         desc_type = AC_DESC_FMASK;
      } else {
         enum glsl_sampler_dim dim = nir_intrinsic_image_dim(intrin);
         desc_type = dim == GLSL_SAMPLER_DIM_BUF ? AC_DESC_BUFFER : AC_DESC_IMAGE;
      }

      bool is_load =
         intrin->intrinsic == nir_intrinsic_bindless_image_load ||
         intrin->intrinsic == nir_intrinsic_bindless_image_sparse_load ||
         intrin->intrinsic == nir_intrinsic_bindless_image_fragment_mask_load_amd ||
         intrin->intrinsic == nir_intrinsic_bindless_image_descriptor_amd;

      nir_def *index = nir_u2u32(b, intrin->src[0].ssa);

      nir_def *desc = load_bindless_image_desc(b, index, desc_type, is_load, s);

      if (intrin->intrinsic == nir_intrinsic_bindless_image_descriptor_amd) {
         nir_def_rewrite_uses(&intrin->def, desc);
         nir_instr_remove(&intrin->instr);
      } else {
         nir_src_rewrite(&intrin->src[0], desc);
      }
      break;
   }
   default:
      return false;
   }

   return true;
}

static nir_def *load_sampler_desc(nir_builder *b, nir_def *list, nir_def *index,
                                      enum ac_descriptor_type desc_type)
{
   /* index is in 16 dword unit, convert to offset in bytes */
   nir_def *offset = nir_ishl_imm(b, index, 6);

   unsigned num_channels = 0;
   switch (desc_type) {
   case AC_DESC_IMAGE:
      /* The image is at [0:7]. */
      num_channels = 8;
      break;
   case AC_DESC_BUFFER:
      /* The buffer is in [4:7]. */
      offset = nir_iadd_imm(b, offset, 16);
      num_channels = 4;
      break;
   case AC_DESC_FMASK:
      /* The FMASK is at [8:15]. */
      offset = nir_iadd_imm(b, offset, 32);
      num_channels = 8;
      break;
   case AC_DESC_SAMPLER:
      /* The sampler state is at [12:15]. */
      offset = nir_iadd_imm(b, offset, 48);
      num_channels = 4;
      break;
   default:
      unreachable("invalid desc type");
      break;
   }

   return nir_load_smem_amd(b, num_channels, list, offset);
}

static nir_def *load_deref_sampler_desc(nir_builder *b, nir_deref_instr *deref,
                                            enum ac_descriptor_type desc_type,
                                            struct lower_resource_state *s,
                                            bool return_descriptor)
{
   unsigned max_slots = BITSET_LAST_BIT(b->shader->info.textures_used);
   nir_def *index = deref_to_index(b, deref, max_slots, NULL, NULL);
   index = nir_iadd_imm(b, index, SI_NUM_IMAGE_SLOTS / 2);

   /* return actual desc when required by caller */
   if (return_descriptor) {
      nir_def *list = ac_nir_load_arg(b, &s->args->ac, s->args->samplers_and_images);
      return load_sampler_desc(b, list, index, desc_type);
   }

   /* Just use index here and let nir-to-llvm backend to translate to actual
    * descriptor. This is because we need waterfall to handle non-dynamic-uniform
    * index there.
    */
   return index;
}

static nir_def *load_bindless_sampler_desc(nir_builder *b, nir_def *index,
                                               enum ac_descriptor_type desc_type,
                                               struct lower_resource_state *s)
{
   nir_def *list = ac_nir_load_arg(b, &s->args->ac, s->args->bindless_samplers_and_images);

   /* 64 bit to 32 bit */
   index = nir_u2u32(b, index);

   return load_sampler_desc(b, list, index, desc_type);
}

static nir_def *fixup_sampler_desc(nir_builder *b,
                                       nir_tex_instr *tex,
                                       nir_def *sampler,
                                       struct lower_resource_state *s)
{
   const struct si_shader_selector *sel = s->shader->selector;

   if (tex->op != nir_texop_tg4 || sel->screen->info.conformant_trunc_coord)
      return sampler;

   /* Set TRUNC_COORD=0 for textureGather(). */
   nir_def *dword0 = nir_channel(b, sampler, 0);
   dword0 = nir_iand_imm(b, dword0, C_008F30_TRUNC_COORD);
   sampler = nir_vector_insert_imm(b, sampler, dword0, 0);
   return sampler;
}

static bool lower_resource_tex(nir_builder *b, nir_tex_instr *tex,
                               struct lower_resource_state *s)
{
   nir_deref_instr *texture_deref = NULL;
   nir_deref_instr *sampler_deref = NULL;
   nir_def *texture_handle = NULL;
   nir_def *sampler_handle = NULL;

   for (unsigned i = 0; i < tex->num_srcs; i++) {
      switch (tex->src[i].src_type) {
      case nir_tex_src_texture_deref:
         texture_deref = nir_src_as_deref(tex->src[i].src);
         break;
      case nir_tex_src_sampler_deref:
         sampler_deref = nir_src_as_deref(tex->src[i].src);
         break;
      case nir_tex_src_texture_handle:
         texture_handle = tex->src[i].src.ssa;
         break;
      case nir_tex_src_sampler_handle:
         sampler_handle = tex->src[i].src.ssa;
         break;
      default:
         break;
      }
   }

   enum ac_descriptor_type desc_type;
   if (tex->op == nir_texop_fragment_mask_fetch_amd)
      desc_type = AC_DESC_FMASK;
   else
      desc_type = tex->sampler_dim == GLSL_SAMPLER_DIM_BUF ? AC_DESC_BUFFER : AC_DESC_IMAGE;

   if (tex->op == nir_texop_descriptor_amd) {
      nir_def *image;
      if (texture_deref)
         image = load_deref_sampler_desc(b, texture_deref, desc_type, s, true);
      else
         image = load_bindless_sampler_desc(b, texture_handle, desc_type, s);
      nir_def_rewrite_uses(&tex->def, image);
      nir_instr_remove(&tex->instr);
      return true;
   }

   if (tex->op == nir_texop_sampler_descriptor_amd) {
      nir_def *sampler;
      if (sampler_deref)
         sampler = load_deref_sampler_desc(b, sampler_deref, AC_DESC_SAMPLER, s, true);
      else
         sampler = load_bindless_sampler_desc(b, sampler_handle, AC_DESC_SAMPLER, s);
      nir_def_rewrite_uses(&tex->def, sampler);
      nir_instr_remove(&tex->instr);
      return true;
   }

   nir_def *image = texture_deref ?
      load_deref_sampler_desc(b, texture_deref, desc_type, s, !tex->texture_non_uniform) :
      load_bindless_sampler_desc(b, texture_handle, desc_type, s);

   nir_def *sampler = NULL;
   if (sampler_deref)
      sampler = load_deref_sampler_desc(b, sampler_deref, AC_DESC_SAMPLER, s, !tex->sampler_non_uniform);
   else if (sampler_handle)
      sampler = load_bindless_sampler_desc(b, sampler_handle, AC_DESC_SAMPLER, s);

   if (sampler && sampler->num_components > 1)
      sampler = fixup_sampler_desc(b, tex, sampler, s);

   for (unsigned i = 0; i < tex->num_srcs; i++) {
      switch (tex->src[i].src_type) {
      case nir_tex_src_texture_deref:
         tex->src[i].src_type = nir_tex_src_texture_handle;
         FALLTHROUGH;
      case nir_tex_src_texture_handle:
         nir_src_rewrite(&tex->src[i].src, image);
         break;
      case nir_tex_src_sampler_deref:
         tex->src[i].src_type = nir_tex_src_sampler_handle;
         FALLTHROUGH;
      case nir_tex_src_sampler_handle:
         nir_src_rewrite(&tex->src[i].src, sampler);
         break;
      default:
         break;
      }
   }

   return true;
}

static bool lower_resource_instr(nir_builder *b, nir_instr *instr, void *state)
{
   struct lower_resource_state *s = (struct lower_resource_state *)state;

   b->cursor = nir_before_instr(instr);

   switch (instr->type) {
   case nir_instr_type_intrinsic: {
      nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
      return lower_resource_intrinsic(b, intrin, s);
   }
   case nir_instr_type_tex: {
      nir_tex_instr *tex = nir_instr_as_tex(instr);
      return lower_resource_tex(b, tex, s);
   }
   default:
      return false;
   }
}

bool si_nir_lower_resource(nir_shader *nir, struct si_shader *shader,
                           struct si_shader_args *args)
{
   struct lower_resource_state state = {
      .shader = shader,
      .args = args,
   };

   return nir_shader_instructions_pass(nir, lower_resource_instr,
                                       nir_metadata_dominance | nir_metadata_block_index,
                                       &state);
}
