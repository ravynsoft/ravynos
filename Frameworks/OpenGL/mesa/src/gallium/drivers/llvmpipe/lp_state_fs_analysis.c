/**************************************************************************
 *
 * Copyright 2010-2021 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS, AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 **************************************************************************/


#include "util/u_memory.h"
#include "util/u_math.h"
#include "lp_debug.h"
#include "lp_state.h"
#include "nir.h"

/*
 * Check if the given nir_src comes directly from a FS input.
 */
static bool
is_fs_input(const nir_src *src)
{
   const nir_instr *parent = src->ssa[0].parent_instr;
   if (!parent) {
      return false;
   }

   if (parent->type == nir_instr_type_alu) {
      const nir_alu_instr *alu = nir_instr_as_alu(parent);
      if (alu->op == nir_op_vec2 ||
          alu->op == nir_op_vec3 ||
          alu->op == nir_op_vec4) {
         /* Check if any of the components come from an FS input */
         unsigned num_src = nir_op_infos[alu->op].num_inputs;
         for (unsigned i = 0; i < num_src; i++) {
            if (is_fs_input(&alu->src[i].src)) {
               return true;
            }
         }
      }
   } else if (parent->type == nir_instr_type_intrinsic) {
      const nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(parent);
      /* loading from an FS input? */
      if (intrin->intrinsic == nir_intrinsic_load_deref) {
         if (is_fs_input(&intrin->src[0])) {
            return true;
         }
      }
   } else if (parent->type == nir_instr_type_deref) {
      const nir_deref_instr *deref = nir_instr_as_deref(parent);
      /* deref'ing an FS input? */
      if (deref &&
          deref->deref_type == nir_deref_type_var &&
          deref->modes == nir_var_shader_in) {
         return true;
      }
   }

   return false;
}


/*
 * Determine whether the given alu src comes directly from an input
 * register.  If so, return true and the input register index and
 * component.  Return false otherwise.
 */
static bool
get_nir_input_info(const nir_alu_src *src,
                   unsigned *input_index,
                   int *input_component)
{
   // The parent instr should be a nir_intrinsic_load_deref.
   const nir_instr *parent = src->src.ssa[0].parent_instr;
   if (!parent || parent->type != nir_instr_type_intrinsic) {
      return false;
   }
   nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(parent);
   if (!intrin ||
       intrin->intrinsic != nir_intrinsic_load_deref) {
      return false;
   }

   // The parent of the load should be a type_deref.
   parent = intrin->src->ssa->parent_instr;
   if (!parent || parent->type != nir_instr_type_deref) {
      return false;
   }

   // The var being deref'd should be a shader input register.
   nir_deref_instr *deref = nir_instr_as_deref(parent);
   if (!deref || deref->deref_type != nir_deref_type_var ||
       deref->modes != nir_var_shader_in) {
      return false;
   }

   /*
    * If the texture coordinate input is declared as two variables like this:
    * decl_var shader_in INTERP_MODE_NONE float coord (VARYING_SLOT_VAR0.x, 0, 0)
    * decl_var shader_in INTERP_MODE_NONE float coord@0 (VARYING_SLOT_VAR0.y, 0, 0)
    * Then deref->var->data.location_frac will be 0 for the first var and 1
    * for the second var and the texcoord will be set up with:
    *   vec2 32 ssa_5 = vec2 ssa_2, ssa_4  (note: no swizzles)
    *
    * Alternately, if the texture coordinate input is declared as one
    * variable like this:
    * decl_var shader_in INTERP_MODE_NONE vec4 i1xyzw (VARYING_SLOT_VAR1.xyzw, 0, 0)
    * then deref->var->data.location_frac will be 0 and the
    * tex coord will be setup with:
    *   vec2 32 ssa_2 = vec2 ssa_1.x, ssa_1.y
    *
    * We can handle both cases by adding deref->var->data.location_frac and
    * src->swizzle[0].
    */
   *input_index = deref->var->data.driver_location;
   *input_component = deref->var->data.location_frac + src->swizzle[0];
   assert(*input_component >= 0);
   assert(*input_component <= 3);

   return true;
}


/*
 * Examine the texcoord argument to a texture instruction to determine
 * if the texcoord comes directly from a fragment shader input.  If so
 * return true and return the FS input register index for the coordinate
 * and the (2-component) swizzle.  Return false otherwise.
 */
static bool
get_texcoord_provenance(const nir_tex_src *texcoord,
                        unsigned *coord_fs_input_index, // out
                        int swizzle[4]) // out
{
   assert(texcoord->src_type == nir_tex_src_coord);

   // The parent instr of the coord should be an nir_op_vec2 alu op
   const nir_instr *parent = texcoord->src.ssa->parent_instr;
   if (!parent || parent->type != nir_instr_type_alu) {
      return false;
   }
   const nir_alu_instr *alu = nir_instr_as_alu(parent);
   if (!alu || alu->op != nir_op_vec2) {
      return false;
   }

   // Loop over nir_op_vec2 instruction arguments to find the
   // input register index and component.
   unsigned input_reg_indexes[2];
   for (unsigned comp = 0; comp < 2; comp++) {
      if (!get_nir_input_info(&alu->src[comp],
                              &input_reg_indexes[comp], &swizzle[comp])) {
         return false;
      }
   }

   // Both texcoord components should come from the same input register.
   if (input_reg_indexes[0] != input_reg_indexes[1]) {
      return false;
   }

   *coord_fs_input_index = input_reg_indexes[0];

   return true;
}


/*
 * Check if all the values of a nir_load_const_instr are 32-bit
 * floats in the range [0,1].  If so, return true, else return false.
 */
static bool
check_load_const_in_zero_one(const nir_load_const_instr *load)
{
   if (load->def.bit_size != 32)
      return false;
   for (unsigned c = 0; c < load->def.num_components; c++) {
      float val = load->value[c].f32;
      if (val < 0.0 || val > 1.0 || isnan(val)) {
         return false;
      }
   }
   return true;
}


/*
 * Examine the NIR shader to determine if it's "linear".
 * For the linear path, we're optimizing the case of rendering a window-
 * aligned, textured quad.  Basically, FS must get the output color from
 * a texture lookup and, possibly, a constant color.  If the color comes
 * from some other sort of computation or from a VS output (FS input), we
 * can't use the linear path.
 */
static bool
llvmpipe_nir_fn_is_linear_compat(const struct nir_shader *shader,
                                 nir_function_impl *impl,
                                 struct lp_tgsi_info *info)
{
   nir_foreach_block(block, impl) {
      nir_foreach_instr_safe(instr, block) {
         switch (instr->type) {
         case nir_instr_type_deref: {
            nir_deref_instr *deref = nir_instr_as_deref(instr);
            if (deref->deref_type != nir_deref_type_var)
               return false;
            if (deref->var->data.mode == nir_var_shader_out &&
                deref->var->data.location_frac != 0)
               return false;
            break;
         }
         case nir_instr_type_load_const: {
            nir_load_const_instr *load = nir_instr_as_load_const(instr);
            if (!check_load_const_in_zero_one(load)) {
               return false;
            }
            break;
         }
         case nir_instr_type_intrinsic: {
            nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
            if (intrin->intrinsic != nir_intrinsic_load_deref &&
                intrin->intrinsic != nir_intrinsic_store_deref &&
                intrin->intrinsic != nir_intrinsic_load_ubo)
               return false;

            if (intrin->intrinsic == nir_intrinsic_load_ubo) {
               if (!nir_src_is_const(intrin->src[0]))
                  return false;
               nir_load_const_instr *load =
                  nir_instr_as_load_const(intrin->src[0].ssa->parent_instr);
               if (load->value[0].u32 != 0 || load->def.num_components > 1)
                  return false;
            } else if (intrin->intrinsic == nir_intrinsic_store_deref) {
               /*
                * Assume the store destination is the FS output color.
                * Check if the store src comes directly from a FS input.
                * If so, we cannot use the linear path since we don't have
                * code to convert VS outputs / FS inputs to ubyte with the
                * needed swizzling.
                */
               if (is_fs_input(&intrin->src[1])) {
                  return false;
               }
            }
            break;
         }
         case nir_instr_type_tex: {
            nir_tex_instr *tex = nir_instr_as_tex(instr);
            struct lp_tgsi_texture_info *tex_info = &info->tex[info->num_texs];
            int texcoord_swizzle[4] = {-1, -1, -1, -1};
            unsigned coord_fs_input_index = 0;

            for (unsigned i = 0; i < tex->num_srcs; i++) {
               if (tex->src[i].src_type == nir_tex_src_coord) {
                  if (!get_texcoord_provenance(&tex->src[i],
                                               &coord_fs_input_index,
                                               texcoord_swizzle)) {
                     //debug nir_print_shader((nir_shader *) shader, stdout);
                     return false;
                  }
               } else if (tex->src[i].src_type == nir_tex_src_texture_handle ||
                          tex->src[i].src_type == nir_tex_src_sampler_handle) {
                  return false;
               }
            }

            switch (tex->op) {
            case nir_texop_tex:
               tex_info->modifier = LP_BLD_TEX_MODIFIER_NONE;
               break;
            default:
               /* inaccurate but sufficient. */
               tex_info->modifier = LP_BLD_TEX_MODIFIER_EXPLICIT_LOD;
               return false;
            }
            switch (tex->sampler_dim) {
            case GLSL_SAMPLER_DIM_2D:
               tex_info->target = TGSI_TEXTURE_2D;
               break;
            default:
               /* inaccurate but sufficient. */
               tex_info->target = TGSI_TEXTURE_1D;
               return false;
            }

            tex_info->sampler_unit = tex->sampler_index;
            tex_info->texture_unit = tex->texture_index;

            /* this is enforced in the scanner previously. */
            tex_info->coord[0].file = TGSI_FILE_INPUT;  // S
            tex_info->coord[1].file = TGSI_FILE_INPUT;  // T
            assert(texcoord_swizzle[0] >= 0);
            assert(texcoord_swizzle[1] >= 0);
            tex_info->coord[0].swizzle = texcoord_swizzle[0]; // S
            tex_info->coord[1].swizzle = texcoord_swizzle[1]; // T
            tex_info->coord[0].u.index = coord_fs_input_index;
            tex_info->coord[1].u.index = coord_fs_input_index;

            info->num_texs++;
            break;
         }
         case nir_instr_type_alu: {
            const nir_alu_instr *alu = nir_instr_as_alu(instr);
            switch (alu->op) {
            case nir_op_mov:
            case nir_op_vec2:
            case nir_op_vec4:
               // these instructions are OK
               break;
            case nir_op_fmul: {
               unsigned num_src = nir_op_infos[alu->op].num_inputs;;
               for (unsigned s = 0; s < num_src; s++) {
                  /* If the MUL uses immediate values, the values must
                   * be 32-bit floats in the range [0,1].
                   */
                  if (nir_src_is_const(alu->src[s].src)) {
                     nir_load_const_instr *load =
                        nir_instr_as_load_const(alu->src[s].src.ssa->parent_instr);
                     if (!check_load_const_in_zero_one(load)) {
                        return false;
                     }
                  } else if (is_fs_input(&alu->src[s].src)) {
                     /* we don't know if the fs inputs are in [0,1] */
                     return false;
                  }
               }
               break;
            }
            default:
               // disallowed instruction
               return false;
            }
            break;
         }
         default:
            return false;
         }
      }
   }
   return true;
}


static bool
llvmpipe_nir_is_linear_compat(struct nir_shader *shader,
                              struct lp_tgsi_info *info)
{
   int num_tex = info->num_texs;

   if (util_bitcount64(shader->info.inputs_read) > LP_MAX_LINEAR_INPUTS)
      return false;
   
   if (!shader->info.outputs_written || shader->info.fs.color_is_dual_source ||
       (shader->info.outputs_written & ~BITFIELD64_BIT(FRAG_RESULT_DATA0)))
      return false;

   info->num_texs = 0;
   nir_foreach_function_impl(impl, shader) {
      if (!llvmpipe_nir_fn_is_linear_compat(shader, impl, info))
         return false;
   }
   info->num_texs = num_tex;
   return true;
}


/*
 * Analyze the given NIR fragment shader and set its shader->kind field
 * to LP_FS_KIND_x.
 */
void
llvmpipe_fs_analyse_nir(struct lp_fragment_shader *shader)
{
   if (!shader->info.indirect_textures &&
       !shader->info.sampler_texture_units_different &&
       shader->info.num_texs <= LP_MAX_LINEAR_TEXTURES &&
       llvmpipe_nir_is_linear_compat(shader->base.ir.nir, &shader->info)) {
      shader->kind = LP_FS_KIND_LLVM_LINEAR;
   } else {
      shader->kind = LP_FS_KIND_GENERAL;
   }
}

