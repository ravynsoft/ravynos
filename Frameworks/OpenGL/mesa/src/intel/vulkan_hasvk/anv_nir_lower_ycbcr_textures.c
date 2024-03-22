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

#include "anv_nir.h"
#include "anv_private.h"
#include "nir/nir.h"
#include "nir/nir_builder.h"
#include "vk_nir_convert_ycbcr.h"

struct ycbcr_state {
   nir_builder *builder;
   nir_def *image_size;
   nir_tex_instr *origin_tex;
   nir_deref_instr *tex_deref;
   const struct vk_ycbcr_conversion *conversion;
};

/* TODO: we should probably replace this with a push constant/uniform. */
static nir_def *
get_texture_size(struct ycbcr_state *state, nir_deref_instr *texture)
{
   if (state->image_size)
      return state->image_size;

   nir_builder *b = state->builder;
   const struct glsl_type *type = texture->type;
   nir_tex_instr *tex = nir_tex_instr_create(b->shader, 1);

   tex->op = nir_texop_txs;
   tex->sampler_dim = glsl_get_sampler_dim(type);
   tex->is_array = glsl_sampler_type_is_array(type);
   tex->is_shadow = glsl_sampler_type_is_shadow(type);
   tex->dest_type = nir_type_int32;

   tex->src[0] = nir_tex_src_for_ssa(nir_tex_src_texture_deref,
                                     &texture->def);

   nir_def_init(&tex->instr, &tex->def, nir_tex_instr_dest_size(tex), 32);
   nir_builder_instr_insert(b, &tex->instr);

   state->image_size = nir_i2f32(b, &tex->def);

   return state->image_size;
}

static nir_def *
implicit_downsampled_coord(nir_builder *b,
                           nir_def *value,
                           nir_def *max_value,
                           int div_scale)
{
   return nir_fadd(b,
                   value,
                   nir_frcp(b,
                            nir_fmul(b,
                                     nir_imm_float(b, div_scale),
                                     max_value)));
}

static nir_def *
implicit_downsampled_coords(struct ycbcr_state *state,
                            nir_def *old_coords,
                            const struct anv_format_plane *plane_format)
{
   nir_builder *b = state->builder;
   const struct vk_ycbcr_conversion *conversion = state->conversion;
   nir_def *image_size = get_texture_size(state, state->tex_deref);
   nir_def *comp[4] = { NULL, };
   int c;

   for (c = 0; c < ARRAY_SIZE(conversion->state.chroma_offsets); c++) {
      if (plane_format->denominator_scales[c] > 1 &&
          conversion->state.chroma_offsets[c] == VK_CHROMA_LOCATION_COSITED_EVEN) {
         comp[c] = implicit_downsampled_coord(b,
                                              nir_channel(b, old_coords, c),
                                              nir_channel(b, image_size, c),
                                              plane_format->denominator_scales[c]);
      } else {
         comp[c] = nir_channel(b, old_coords, c);
      }
   }

   /* Leave other coordinates untouched */
   for (; c < old_coords->num_components; c++)
      comp[c] = nir_channel(b, old_coords, c);

   return nir_vec(b, comp, old_coords->num_components);
}

static nir_def *
create_plane_tex_instr_implicit(struct ycbcr_state *state,
                                uint32_t plane)
{
   nir_builder *b = state->builder;
   const struct vk_ycbcr_conversion *conversion = state->conversion;
   const struct anv_format_plane *plane_format =
      &anv_get_format(conversion->state.format)->planes[plane];
   nir_tex_instr *old_tex = state->origin_tex;
   nir_tex_instr *tex = nir_tex_instr_create(b->shader, old_tex->num_srcs + 1);

   for (uint32_t i = 0; i < old_tex->num_srcs; i++) {
      tex->src[i].src_type = old_tex->src[i].src_type;

      switch (old_tex->src[i].src_type) {
      case nir_tex_src_coord:
         if (plane_format->has_chroma && conversion->state.chroma_reconstruction) {
            tex->src[i].src =
               nir_src_for_ssa(implicit_downsampled_coords(state,
                                                           old_tex->src[i].src.ssa,
                                                           plane_format));
            break;
         }
         FALLTHROUGH;
      default:
         tex->src[i].src = nir_src_for_ssa(old_tex->src[i].src.ssa);
         break;
      }
   }
   tex->src[tex->num_srcs - 1] = nir_tex_src_for_ssa(nir_tex_src_plane,
                                                     nir_imm_int(b, plane));

   tex->sampler_dim = old_tex->sampler_dim;
   tex->dest_type = old_tex->dest_type;

   tex->op = old_tex->op;
   tex->coord_components = old_tex->coord_components;
   tex->is_new_style_shadow = old_tex->is_new_style_shadow;
   tex->component = old_tex->component;

   tex->texture_index = old_tex->texture_index;
   tex->sampler_index = old_tex->sampler_index;
   tex->is_array = old_tex->is_array;

   nir_def_init(&tex->instr, &tex->def, old_tex->def.num_components,
                old_tex->def.bit_size);
   nir_builder_instr_insert(b, &tex->instr);

   return &tex->def;
}

static unsigned
channel_to_component(enum isl_channel_select channel)
{
   switch (channel) {
   case ISL_CHANNEL_SELECT_RED:
      return 0;
   case ISL_CHANNEL_SELECT_GREEN:
      return 1;
   case ISL_CHANNEL_SELECT_BLUE:
      return 2;
   case ISL_CHANNEL_SELECT_ALPHA:
      return 3;
   default:
      unreachable("invalid channel");
      return 0;
   }
}

static enum isl_channel_select
swizzle_channel(struct isl_swizzle swizzle, unsigned channel)
{
   switch (channel) {
   case 0:
      return swizzle.r;
   case 1:
      return swizzle.g;
   case 2:
      return swizzle.b;
   case 3:
      return swizzle.a;
   default:
      unreachable("invalid channel");
      return 0;
   }
}

static bool
anv_nir_lower_ycbcr_textures_instr(nir_builder *builder,
                                   nir_instr *instr,
                                   void *cb_data)
{
   const struct anv_pipeline_layout *layout = cb_data;

   if (instr->type != nir_instr_type_tex)
      return false;

   nir_tex_instr *tex = nir_instr_as_tex(instr);

   int deref_src_idx = nir_tex_instr_src_index(tex, nir_tex_src_texture_deref);
   assert(deref_src_idx >= 0);
   nir_deref_instr *deref = nir_src_as_deref(tex->src[deref_src_idx].src);

   nir_variable *var = nir_deref_instr_get_variable(deref);
   const struct anv_descriptor_set_layout *set_layout =
      layout->set[var->data.descriptor_set].layout;
   const struct anv_descriptor_set_binding_layout *binding =
      &set_layout->binding[var->data.binding];

   /* For the following instructions, we don't apply any change and let the
    * instruction apply to the first plane.
    */
   if (tex->op == nir_texop_txs ||
       tex->op == nir_texop_query_levels ||
       tex->op == nir_texop_lod)
      return false;

   if (binding->immutable_samplers == NULL)
      return false;

   assert(tex->texture_index == 0);
   unsigned array_index = 0;
   if (deref->deref_type != nir_deref_type_var) {
      assert(deref->deref_type == nir_deref_type_array);
      if (!nir_src_is_const(deref->arr.index))
         return false;
      array_index = nir_src_as_uint(deref->arr.index);
      array_index = MIN2(array_index, binding->array_size - 1);
   }
   const struct anv_sampler *sampler = binding->immutable_samplers[array_index];

   if (sampler->conversion == NULL)
      return false;

   struct ycbcr_state state = {
      .builder = builder,
      .origin_tex = tex,
      .tex_deref = deref,
      .conversion = sampler->conversion,
   };

   builder->cursor = nir_before_instr(&tex->instr);

   const struct anv_format *format = anv_get_format(state.conversion->state.format);
   const struct isl_format_layout *y_isl_layout = NULL;
   for (uint32_t p = 0; p < format->n_planes; p++) {
      if (!format->planes[p].has_chroma)
         y_isl_layout = isl_format_get_layout(format->planes[p].isl_format);
   }
   assert(y_isl_layout != NULL);
   uint8_t y_bpc = y_isl_layout->channels_array[0].bits;

   /* |ycbcr_comp| holds components in the order : Cr-Y-Cb */
   nir_def *zero = nir_imm_float(builder, 0.0f);
   nir_def *one = nir_imm_float(builder, 1.0f);
   /* Use extra 2 channels for following swizzle */
   nir_def *ycbcr_comp[5] = { zero, zero, zero, one, zero };

   uint8_t ycbcr_bpcs[5];
   memset(ycbcr_bpcs, y_bpc, sizeof(ycbcr_bpcs));

   /* Go through all the planes and gather the samples into a |ycbcr_comp|
    * while applying a swizzle required by the spec:
    *
    *    R, G, B should respectively map to Cr, Y, Cb
    */
   for (uint32_t p = 0; p < format->n_planes; p++) {
      const struct anv_format_plane *plane_format = &format->planes[p];
      nir_def *plane_sample = create_plane_tex_instr_implicit(&state, p);

      for (uint32_t pc = 0; pc < 4; pc++) {
         enum isl_channel_select ycbcr_swizzle =
            swizzle_channel(plane_format->ycbcr_swizzle, pc);
         if (ycbcr_swizzle == ISL_CHANNEL_SELECT_ZERO)
            continue;

         unsigned ycbcr_component = channel_to_component(ycbcr_swizzle);
         ycbcr_comp[ycbcr_component] = nir_channel(builder, plane_sample, pc);

         /* Also compute the number of bits for each component. */
         const struct isl_format_layout *isl_layout =
            isl_format_get_layout(plane_format->isl_format);
         ycbcr_bpcs[ycbcr_component] = isl_layout->channels_array[pc].bits;
      }
   }

   /* Now remaps components to the order specified by the conversion. */
   nir_def *swizzled_comp[4] = { NULL, };
   uint32_t swizzled_bpcs[4] = { 0, };

   for (uint32_t i = 0; i < ARRAY_SIZE(state.conversion->state.mapping); i++) {
      /* Maps to components in |ycbcr_comp| */
      static const uint32_t swizzle_mapping[] = {
         [VK_COMPONENT_SWIZZLE_ZERO] = 4,
         [VK_COMPONENT_SWIZZLE_ONE]  = 3,
         [VK_COMPONENT_SWIZZLE_R]    = 0,
         [VK_COMPONENT_SWIZZLE_G]    = 1,
         [VK_COMPONENT_SWIZZLE_B]    = 2,
         [VK_COMPONENT_SWIZZLE_A]    = 3,
      };
      const VkComponentSwizzle m = state.conversion->state.mapping[i];

      if (m == VK_COMPONENT_SWIZZLE_IDENTITY) {
         swizzled_comp[i] = ycbcr_comp[i];
         swizzled_bpcs[i] = ycbcr_bpcs[i];
      } else {
         swizzled_comp[i] = ycbcr_comp[swizzle_mapping[m]];
         swizzled_bpcs[i] = ycbcr_bpcs[swizzle_mapping[m]];
      }
   }

   nir_def *result = nir_vec(builder, swizzled_comp, 4);
   if (state.conversion->state.ycbcr_model != VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY) {
      result = nir_convert_ycbcr_to_rgb(builder,
                                        state.conversion->state.ycbcr_model,
                                        state.conversion->state.ycbcr_range,
                                        result,
                                        swizzled_bpcs);
   }

   nir_def_rewrite_uses(&tex->def, result);
   nir_instr_remove(&tex->instr);

   return true;
}

bool
anv_nir_lower_ycbcr_textures(nir_shader *shader,
                             const struct anv_pipeline_layout *layout)
{
   return nir_shader_instructions_pass(shader,
                                       anv_nir_lower_ycbcr_textures_instr,
                                       nir_metadata_block_index |
                                       nir_metadata_dominance,
                                       (void *)layout);
}
