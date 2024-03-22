/*
 * Copyright 2022 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

/* This lowers image and texture opcodes to typed buffer opcodes (equivalent to image buffers)
 * for some CDNA chips. Sampler buffers and image buffers are not lowered.
 *
 * Only the subset of opcodes and states that is used by VAAPI and OpenMAX is lowered.
 * That means CLAMP_TO_EDGE is always used. Only level 0 can be accessed. The minification
 * and magnification filter settings are assumed to be equal.
 *
 * This uses a custom image descriptor that is used in conjunction with this pass. The first
 * 4 dwords of the descriptor contain the buffer descriptor where the format matches the image
 * format and the stride matches the pixel size, and the last 4 dwords contain parameters
 * for manual address computations and bounds checking like the pitch, the number of elements
 * per slice, etc.
 *
 */

#include "ac_nir.h"
#include "nir_builder.h"
#include "amdgfxregs.h"

static nir_def *get_field(nir_builder *b, nir_def *desc, unsigned index, unsigned mask)
{
   return nir_ubfe_imm(b, nir_channel(b, desc, index), ffs(mask) - 1, util_bitcount(mask));
}

static unsigned get_coord_components(enum glsl_sampler_dim dim, bool is_array)
{
   switch (dim) {
   case GLSL_SAMPLER_DIM_1D:
      return is_array ? 2 : 1;
   case GLSL_SAMPLER_DIM_2D:
   case GLSL_SAMPLER_DIM_RECT:
      return is_array ? 3 : 2;
   case GLSL_SAMPLER_DIM_3D:
      return 3;
   default:
      unreachable("unexpected sampler type");
   }
}

/* Lower image coordinates to a buffer element index. Return UINT_MAX if the image coordinates
 * are out of bounds.
 */
static nir_def *lower_image_coords(nir_builder *b, nir_def *desc, nir_def *coord,
                                       enum glsl_sampler_dim dim, bool is_array,
                                       bool handle_out_of_bounds)
{
   unsigned num_coord_components = get_coord_components(dim, is_array);
   nir_def *zero = nir_imm_int(b, 0);

   /* Get coordinates. */
   nir_def *x = nir_channel(b, coord, 0);
   nir_def *y = num_coord_components >= 2 ? nir_channel(b, coord, 1) : NULL;
   nir_def *z = num_coord_components >= 3 ? nir_channel(b, coord, 2) : NULL;

   if (dim == GLSL_SAMPLER_DIM_1D && is_array) {
      z = y;
      y = NULL;
   }

   if (is_array) {
      nir_def *first_layer = get_field(b, desc, 5, 0xffff0000);
      z = nir_iadd(b, z, first_layer);
   }

   /* Compute the buffer element index. */
   nir_def *index = x;
   if (y) {
      nir_def *pitch = nir_channel(b, desc, 6);
      index = nir_iadd(b, index, nir_imul(b, pitch, y));
   }
   if (z) {
      nir_def *slice_elements = nir_channel(b, desc, 7);
      index = nir_iadd(b, index, nir_imul(b, slice_elements, z));
   }

   /* Determine whether the coordinates are out of bounds. */
   nir_def *out_of_bounds = NULL;

   if (handle_out_of_bounds) {
      nir_def *width = get_field(b, desc, 4, 0xffff);
      out_of_bounds = nir_ior(b, nir_ilt(b, x, zero), nir_ige(b, x, width));

      if (y) {
         nir_def *height = get_field(b, desc, 4, 0xffff0000);
         out_of_bounds = nir_ior(b, out_of_bounds,
                                 nir_ior(b, nir_ilt(b, y, zero), nir_ige(b, y, height)));
      }
      if (z) {
         nir_def *depth = get_field(b, desc, 5, 0xffff);
         out_of_bounds = nir_ior(b, out_of_bounds,
                                 nir_ior(b, nir_ilt(b, z, zero), nir_ige(b, z, depth)));
      }

      /* Make the buffer opcode out of bounds by setting UINT_MAX. */
      index = nir_bcsel(b, out_of_bounds, nir_imm_int(b, UINT_MAX), index);
   }

   return index;
}

static nir_def *emulated_image_load(nir_builder *b, unsigned num_components, unsigned bit_size,
                                        nir_def *desc, nir_def *coord,
                                        enum gl_access_qualifier access, enum glsl_sampler_dim dim,
                                        bool is_array, bool handle_out_of_bounds)
{
   nir_def *zero = nir_imm_int(b, 0);

   return nir_load_buffer_amd(b, num_components, bit_size, nir_channels(b, desc, 0xf),
                              zero, zero,
                              lower_image_coords(b, desc, coord, dim, is_array,
                                                 handle_out_of_bounds),
                              .base = 0,
                              .memory_modes = nir_var_image,
                              .access = access | ACCESS_USES_FORMAT_AMD);
}

static void emulated_image_store(nir_builder *b, nir_def *desc, nir_def *coord,
                                 nir_def *data, enum gl_access_qualifier access,
                                 enum glsl_sampler_dim dim, bool is_array)
{
   nir_def *zero = nir_imm_int(b, 0);

   nir_store_buffer_amd(b, data, nir_channels(b, desc, 0xf), zero, zero,
                        lower_image_coords(b, desc, coord, dim, is_array, true),
                        .base = 0,
                        .memory_modes = nir_var_image,
                        .access = access | ACCESS_USES_FORMAT_AMD);
}

/* Return the width, height, or depth for dim=0,1,2. */
static nir_def *get_dim(nir_builder *b, nir_def *desc, unsigned dim)
{
   return get_field(b, desc, 4 + dim / 2, 0xffff << (16 * (dim % 2)));
}

/* Lower txl with lod=0 to typed buffer loads. This is based on the equations in the GL spec.
 * This basically converts the tex opcode into 1 or more image_load opcodes.
 */
static nir_def *emulated_tex_level_zero(nir_builder *b, unsigned num_components,
                                            unsigned bit_size, nir_def *desc,
                                            nir_def *sampler_desc, nir_def *coord_vec,
                                            enum glsl_sampler_dim sampler_dim, bool is_array)
{
   const enum gl_access_qualifier access =
      ACCESS_RESTRICT | ACCESS_NON_WRITEABLE | ACCESS_CAN_REORDER;
   const unsigned num_coord_components = get_coord_components(sampler_dim, is_array);
   const unsigned num_dim_coords = num_coord_components - is_array;
   const unsigned array_comp = num_coord_components - 1;

   nir_def *zero = nir_imm_int(b, 0);
   nir_def *fp_one = nir_imm_floatN_t(b, 1, bit_size);
   nir_def *coord[3] = {0};

   assert(num_coord_components <= 3);
   for (unsigned i = 0; i < num_coord_components; i++)
      coord[i] = nir_channel(b, coord_vec, i);

   /* Convert to unnormalized coordinates. */
   if (sampler_dim != GLSL_SAMPLER_DIM_RECT) {
      for (unsigned dim = 0; dim < num_dim_coords; dim++)
         coord[dim] = nir_fmul(b, coord[dim], nir_u2f32(b, get_dim(b, desc, dim)));
   }

   /* The layer index is handled differently and ignores the filter and wrap mode. */
   if (is_array) {
      coord[array_comp] = nir_f2i32(b, nir_fround_even(b, coord[array_comp]));
      coord[array_comp] = nir_iclamp(b, coord[array_comp], zero,
                                     nir_iadd_imm(b, get_dim(b, desc, 2), -1));
   }

   /* Determine the filter by reading the first bit of the XY_MAG_FILTER field,
    * which is 1 for linear, 0 for nearest.
    *
    * We assume that XY_MIN_FILTER and Z_FILTER are identical.
    */
   nir_def *is_nearest =
      nir_ieq_imm(b, nir_iand_imm(b, nir_channel(b, sampler_desc, 2), 1 << 20), 0);
   nir_def *result_nearest, *result_linear;

   nir_if *if_nearest = nir_push_if(b, is_nearest);
   {
      /* Nearest filter. */
      nir_def *coord0[3] = {0};
      memcpy(coord0, coord, sizeof(coord));

      for (unsigned dim = 0; dim < num_dim_coords; dim++) {
         /* Convert to integer coordinates. (floor is required) */
         coord0[dim] = nir_f2i32(b, nir_ffloor(b, coord0[dim]));

         /* Apply the wrap mode. We assume it's always CLAMP_TO_EDGE, so clamp. */
         coord0[dim] = nir_iclamp(b, coord0[dim], zero, nir_iadd_imm(b, get_dim(b, desc, dim), -1));
      }

      /* Load the texel. */
      result_nearest = emulated_image_load(b, num_components, bit_size, desc,
                                           nir_vec(b, coord0, num_coord_components),
                                           access, sampler_dim, is_array, false);
   }
   nir_push_else(b, if_nearest);
   {
      /* Linear filter. */
      nir_def *coord0[3] = {0};
      nir_def *coord1[3] = {0};
      nir_def *weight[3] = {0};

      memcpy(coord0, coord, sizeof(coord));

      for (unsigned dim = 0; dim < num_dim_coords; dim++) {
         /* First subtract 0.5. */
         coord0[dim] = nir_fadd_imm(b, coord0[dim], -0.5);

         /* Use fract to compute the filter weights. (FP16 results will get FP16 filter precision) */
         weight[dim] = nir_f2fN(b, nir_ffract(b, coord0[dim]), bit_size);

         /* Floor to get the top-left texel of the filter. */
         /* Add 1 to get the bottom-right texel. */
         coord0[dim] = nir_f2i32(b, nir_ffloor(b, coord0[dim]));
         coord1[dim] = nir_iadd_imm(b, coord0[dim], 1);

         /* Apply the wrap mode. We assume it's always CLAMP_TO_EDGE, so clamp. */
         coord0[dim] = nir_iclamp(b, coord0[dim], zero, nir_iadd_imm(b, get_dim(b, desc, dim), -1));
         coord1[dim] = nir_iclamp(b, coord1[dim], zero, nir_iadd_imm(b, get_dim(b, desc, dim), -1));
      }

      /* Load all texels for the linear filter.
       * This is 2 texels for 1D, 4 texels for 2D, and 8 texels for 3D.
       */
      nir_def *texel[8];

      for (unsigned i = 0; i < (1 << num_dim_coords); i++) {
         nir_def *texel_coord[3];

         /* Determine whether the current texel should use channels from coord0
          * or coord1. The i-th bit of the texel index determines that.
          */
         for (unsigned dim = 0; dim < num_dim_coords; dim++)
            texel_coord[dim] = (i >> dim) & 0x1 ? coord1[dim] : coord0[dim];

         /* Add the layer index, which doesn't change between texels. */
         if (is_array)
            texel_coord[array_comp] = coord0[array_comp];

         /* Compute how much the texel contributes to the final result. */
         nir_def *texel_weight = fp_one;
         for (unsigned dim = 0; dim < num_dim_coords; dim++) {
            /* Let's see what "i" represents:
             *    Texel i=0 = 000
             *    Texel i=1 = 001
             *    Texel i=2 = 010 (2D & 3D only)
             *    Texel i=3 = 011 (2D & 3D only)
             *    Texel i=4 = 100 (3D only)
             *    Texel i=5 = 101 (3D only)
             *    Texel i=6 = 110 (3D only)
             *    Texel i=7 = 111 (3D only)
             *
             * The rightmost bit (LSB) represents the X direction, the middle bit represents
             * the Y direction, and the leftmost bit (MSB) represents the Z direction.
             * If we shift the texel index "i" by the dimension "dim", we'll get whether that
             * texel value should be multiplied by (1 - weight[dim]) or (weight[dim]).
             */
            texel_weight = nir_fmul(b, texel_weight,
                                     (i >> dim) & 0x1 ? weight[dim] :
                                                      nir_fadd(b, fp_one, nir_fneg(b, weight[dim])));
         }

         /* Load the linear filter texel. */
         texel[i] = emulated_image_load(b, num_components, bit_size, desc,
                                         nir_vec(b, texel_coord, num_coord_components),
                                         access, sampler_dim, is_array, false);

         /* Multiply the texel by the weight. */
         texel[i] = nir_fmul(b, texel[i], texel_weight);
      }

      /* Sum up all weighted texels to get the final result of linear filtering. */
      result_linear = zero;
      for (unsigned i = 0; i < (1 << num_dim_coords); i++)
         result_linear = nir_fadd(b, result_linear, texel[i]);
   }
   nir_pop_if(b, if_nearest);

   return nir_if_phi(b, result_nearest, result_linear);
}

static bool lower_image_opcodes(nir_builder *b, nir_instr *instr, void *data)
{
   if (instr->type == nir_instr_type_intrinsic) {
      nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
      nir_deref_instr *deref;
      enum gl_access_qualifier access;
      enum glsl_sampler_dim dim;
      bool is_array;
      nir_def *desc = NULL, *result = NULL;
      ASSERTED const char *intr_name;

      nir_def *dst = &intr->def;
      b->cursor = nir_before_instr(instr);

      switch (intr->intrinsic) {
      case nir_intrinsic_image_load:
      case nir_intrinsic_image_store:
         access = nir_intrinsic_access(intr);
         dim = nir_intrinsic_image_dim(intr);
         if (dim == GLSL_SAMPLER_DIM_BUF)
            return false;
         is_array = nir_intrinsic_image_array(intr);
         desc = nir_image_descriptor_amd(b, dim == GLSL_SAMPLER_DIM_BUF ? 4 : 8,
                                         32, intr->src[0].ssa);
         break;

      case nir_intrinsic_image_deref_load:
      case nir_intrinsic_image_deref_store:
         deref = nir_instr_as_deref(intr->src[0].ssa->parent_instr);
         access = nir_deref_instr_get_variable(deref)->data.access;
         dim = glsl_get_sampler_dim(deref->type);
         if (dim == GLSL_SAMPLER_DIM_BUF)
            return false;
         is_array = glsl_sampler_type_is_array(deref->type);
         desc = nir_image_deref_descriptor_amd(b, dim == GLSL_SAMPLER_DIM_BUF ? 4 : 8,
                                               32, intr->src[0].ssa);
         break;

      case nir_intrinsic_bindless_image_load:
      case nir_intrinsic_bindless_image_store:
         access = nir_intrinsic_access(intr);
         dim = nir_intrinsic_image_dim(intr);
         if (dim == GLSL_SAMPLER_DIM_BUF)
            return false;
         is_array = nir_intrinsic_image_array(intr);
         desc = nir_bindless_image_descriptor_amd(b, dim == GLSL_SAMPLER_DIM_BUF ? 4 : 8,
                                                  32, intr->src[0].ssa);
         break;

      default:
         intr_name = nir_intrinsic_infos[intr->intrinsic].name;

         /* No other intrinsics are expected from VAAPI and OpenMAX.
          * (this lowering is only used by CDNA, which only uses those frontends)
          */
         if (strstr(intr_name, "image") == intr_name ||
             strstr(intr_name, "bindless_image") == intr_name) {
            fprintf(stderr, "Unexpected image opcode: ");
            nir_print_instr(instr, stderr);
            fprintf(stderr, "\nAborting to prevent a hang.");
            abort();
         }
         return false;
      }

      switch (intr->intrinsic) {
      case nir_intrinsic_image_load:
      case nir_intrinsic_image_deref_load:
      case nir_intrinsic_bindless_image_load:
         result = emulated_image_load(b, intr->def.num_components, intr->def.bit_size,
                                      desc, intr->src[1].ssa, access, dim, is_array, true);
         nir_def_rewrite_uses_after(dst, result, instr);
         nir_instr_remove(instr);
         return true;

      case nir_intrinsic_image_store:
      case nir_intrinsic_image_deref_store:
      case nir_intrinsic_bindless_image_store:
         emulated_image_store(b, desc, intr->src[1].ssa, intr->src[3].ssa, access, dim, is_array);
         nir_instr_remove(instr);
         return true;

      default:
         unreachable("shouldn't get here");
      }
   } else if (instr->type == nir_instr_type_tex) {
      nir_tex_instr *tex = nir_instr_as_tex(instr);
      nir_tex_instr *new_tex;
      nir_def *coord = NULL, *desc = NULL, *sampler_desc = NULL, *result = NULL;

      nir_def *dst = &tex->def;
      b->cursor = nir_before_instr(instr);

      switch (tex->op) {
      case nir_texop_tex:
      case nir_texop_txl:
      case nir_texop_txf:
         for (unsigned i = 0; i < tex->num_srcs; i++) {
            switch (tex->src[i].src_type) {
            case nir_tex_src_texture_deref:
            case nir_tex_src_texture_handle:
               if (tex->sampler_dim == GLSL_SAMPLER_DIM_BUF)
                  return false;
               new_tex = nir_tex_instr_create(b->shader, 1);
               new_tex->op = nir_texop_descriptor_amd;
               new_tex->sampler_dim = tex->sampler_dim;
               new_tex->is_array = tex->is_array;
               new_tex->texture_index = tex->texture_index;
               new_tex->sampler_index = tex->sampler_index;
               new_tex->dest_type = nir_type_int32;
               new_tex->src[0].src = nir_src_for_ssa(tex->src[i].src.ssa);
               new_tex->src[0].src_type = tex->src[i].src_type;
               nir_def_init(&new_tex->instr, &new_tex->def,
                            nir_tex_instr_dest_size(new_tex), 32);
               nir_builder_instr_insert(b, &new_tex->instr);
               desc = &new_tex->def;
               break;

            case nir_tex_src_sampler_deref:
            case nir_tex_src_sampler_handle:
               if (tex->sampler_dim == GLSL_SAMPLER_DIM_BUF)
                  return false;
               new_tex = nir_tex_instr_create(b->shader, 1);
               new_tex->op = nir_texop_sampler_descriptor_amd;
               new_tex->sampler_dim = tex->sampler_dim;
               new_tex->is_array = tex->is_array;
               new_tex->texture_index = tex->texture_index;
               new_tex->sampler_index = tex->sampler_index;
               new_tex->dest_type = nir_type_int32;
               new_tex->src[0].src = nir_src_for_ssa(tex->src[i].src.ssa);
               new_tex->src[0].src_type = tex->src[i].src_type;
               nir_def_init(&new_tex->instr, &new_tex->def,
                            nir_tex_instr_dest_size(new_tex), 32);
               nir_builder_instr_insert(b, &new_tex->instr);
               sampler_desc = &new_tex->def;
               break;

            case nir_tex_src_coord:
               coord = tex->src[i].src.ssa;
               break;

            case nir_tex_src_projector:
            case nir_tex_src_comparator:
            case nir_tex_src_offset:
            case nir_tex_src_texture_offset:
            case nir_tex_src_sampler_offset:
            case nir_tex_src_plane:
               unreachable("unsupported texture src");

            default:;
            }
         }

         switch (tex->op) {
         case nir_texop_txf:
            result = emulated_image_load(b, tex->def.num_components, tex->def.bit_size,
                                         desc, coord,
                                         ACCESS_RESTRICT | ACCESS_NON_WRITEABLE | ACCESS_CAN_REORDER,
                                         tex->sampler_dim, tex->is_array, true);
            nir_def_rewrite_uses_after(dst, result, instr);
            nir_instr_remove(instr);
            return true;

         case nir_texop_tex:
         case nir_texop_txl:
            result = emulated_tex_level_zero(b, tex->def.num_components, tex->def.bit_size,
                                  desc, sampler_desc, coord, tex->sampler_dim, tex->is_array);
            nir_def_rewrite_uses_after(dst, result, instr);
            nir_instr_remove(instr);
            return true;

         default:
            unreachable("shouldn't get here");
         }
         break;

      case nir_texop_descriptor_amd:
      case nir_texop_sampler_descriptor_amd:
         return false;

      default:
         fprintf(stderr, "Unexpected texture opcode: ");
         nir_print_instr(instr, stderr);
         fprintf(stderr, "\nAborting to prevent a hang.");
         abort();
      }
   }

   return false;
}

bool ac_nir_lower_image_opcodes(nir_shader *nir)
{
   return nir_shader_instructions_pass(nir, lower_image_opcodes,
                                       nir_metadata_dominance |
                                       nir_metadata_block_index,
                                       NULL);
}
