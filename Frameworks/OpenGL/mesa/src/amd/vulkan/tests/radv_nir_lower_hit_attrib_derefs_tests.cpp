/*
 * Copyright © 2023 Valve Corporation
 * SPDX-License-Identifier: MIT
 */

#include "nir/radv_nir.h"
#include "tests/nir_test.h"
#include "radv_constants.h"

class radv_nir_lower_hit_attrib_derefs_test : public nir_test {
protected:
   radv_nir_lower_hit_attrib_derefs_test(): nir_test("radv_nir_lower_hit_attrib_derefs_test")
   {
      b->shader->info.stage = MESA_SHADER_INTERSECTION;
   }

   void validate(uint32_t used_bits[RADV_MAX_HIT_ATTRIB_DWORDS], uint32_t used_dwords, bool constant_fold);
};

void
radv_nir_lower_hit_attrib_derefs_test::validate(uint32_t used_bits[RADV_MAX_HIT_ATTRIB_DWORDS], uint32_t used_dwords,
                                                bool constant_fold)
{
   EXPECT_TRUE(radv_nir_lower_hit_attrib_derefs(b->shader));
   nir_validate_shader(b->shader, "After radv_nir_lower_hit_attrib_derefs");

   srand(918763498);

   uint32_t values[RADV_MAX_HIT_ATTRIB_DWORDS];
   for (uint32_t i = 0; i < ARRAY_SIZE(values); i++)
      values[i] = ((uint32_t)rand() ^ ((uint32_t)rand() << 1)) & used_bits[i];

   nir_function_impl *impl = nir_shader_get_entrypoint(b->shader);

   if (constant_fold) {
      nir_foreach_block (block, impl) {
         nir_foreach_instr_safe (instr, block) {
            if (instr->type != nir_instr_type_intrinsic)
               continue;

            nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
            if (intr->intrinsic != nir_intrinsic_load_hit_attrib_amd)
               continue;

            b->cursor = nir_after_instr(instr);
            nir_def *value = nir_imm_int(b, values[nir_intrinsic_base(intr)]);
            nir_def_rewrite_uses(&intr->def, value);
         }
      }
      NIR_PASS(_, b->shader, nir_opt_constant_folding);
   }

   NIR_PASS(_, b->shader, nir_opt_dce);

   uint32_t stored_dwords = 0;
   nir_foreach_block (block, impl) {
      nir_foreach_instr_safe (instr, block) {
         /* Make sure that all ray_hit_attrib variables have been lowered. */
         if (instr->type == nir_instr_type_deref) {
            EXPECT_FALSE(nir_deref_mode_is(nir_instr_as_deref(instr), nir_var_ray_hit_attrib));
         }

         if (instr->type != nir_instr_type_intrinsic)
            continue;

         nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
         if (intr->intrinsic != nir_intrinsic_store_hit_attrib_amd)
            continue;

         uint32_t base = nir_intrinsic_base(intr);
         EXPECT_LT(base, RADV_MAX_HIT_ATTRIB_DWORDS);
         stored_dwords |= BITFIELD_BIT(base);

         bool is_const = nir_src_is_const(intr->src[0]);
         EXPECT_TRUE(is_const || !constant_fold);
         if (!is_const)
            continue;

         uint32_t src = nir_src_as_uint(intr->src[0]);
         EXPECT_EQ(src, values[base] & used_bits[base]);
      }
   }

   EXPECT_EQ(stored_dwords, used_dwords);
}

TEST_F(radv_nir_lower_hit_attrib_derefs_test, types)
{
   nir_variable *vec3_var = nir_variable_create(b->shader, nir_var_ray_hit_attrib, glsl_vec_type(3), "vec3");
   nir_variable *uint64_var = nir_variable_create(b->shader, nir_var_ray_hit_attrib, glsl_uint64_t_type(), "uint64_t");
   nir_variable *uint16_var = nir_variable_create(b->shader, nir_var_ray_hit_attrib, glsl_uint16_t_type(), "uint16_t");
   nir_variable *uint8_var = nir_variable_create(b->shader, nir_var_ray_hit_attrib, glsl_uint8_t_type(), "uint8_t");
   nir_variable *bool_var = nir_variable_create(b->shader, nir_var_ray_hit_attrib, glsl_bool_type(), "bool");

   nir_variable *vars[5] = {
      vec3_var, uint64_var, uint16_var, uint8_var, bool_var,
   };

   for (uint32_t i = 0; i < ARRAY_SIZE(vars); i++) {
      nir_def *load = nir_load_var(b, vars[i]);
      nir_store_var(b, vars[i], load, (1 << load->num_components) - 1);
   }

   uint32_t masks[RADV_MAX_HIT_ATTRIB_DWORDS] = {
      /* vec3 */
      0xFFFFFFFF,
      0xFFFFFFFF,
      0xFFFFFFFF,
      /* padding */
      0,
      /* uint64_t */
      0xFFFFFFFF,
      0xFFFFFFFF,
      /* uint16_t uint8_t */
      0xFFFFFFFF,
      /* bool */
      1,
   };
   validate(masks, 0b11110111, true);
}

TEST_F(radv_nir_lower_hit_attrib_derefs_test, array)
{
   nir_variable *array_var =
      nir_variable_create(b->shader, nir_var_ray_hit_attrib,
                          glsl_array_type(glsl_uint_type(), RADV_MAX_HIT_ATTRIB_DWORDS, 0), "uint32_t[]");

   for (uint32_t i = 0; i < RADV_MAX_HIT_ATTRIB_DWORDS; i++) {
      nir_def *load = nir_load_array_var_imm(b, array_var, i);
      nir_store_array_var_imm(b, array_var, i, load, 0x1);
   }

   uint32_t masks[RADV_MAX_HIT_ATTRIB_DWORDS] = {
      0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
   };
   validate(masks, 0xFF, true);
}

TEST_F(radv_nir_lower_hit_attrib_derefs_test, dynamic_array)
{
   nir_variable *array_var =
      nir_variable_create(b->shader, nir_var_ray_hit_attrib,
                          glsl_array_type(glsl_uint_type(), RADV_MAX_HIT_ATTRIB_DWORDS, 0), "uint32_t[]");

   nir_def *index = nir_load_local_invocation_index(b);
   nir_def *load = nir_load_array_var(b, array_var, index);
   nir_store_array_var(b, array_var, index, load, 0x1);

   uint32_t masks[RADV_MAX_HIT_ATTRIB_DWORDS] = {
      0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
   };
   validate(masks, 0xFF, false);
}

TEST_F(radv_nir_lower_hit_attrib_derefs_test, struct)
{
   glsl_struct_field fields[5] = {
      glsl_struct_field(glsl_vec_type(3), "vec3"),         glsl_struct_field(glsl_uint64_t_type(), "uint64_t"),
      glsl_struct_field(glsl_uint16_t_type(), "uint16_t"), glsl_struct_field(glsl_uint8_t_type(), "uint8_t"),
      glsl_struct_field(glsl_bool_type(), "bool"),
   };

   const glsl_type *var_type = glsl_struct_type(fields, ARRAY_SIZE(fields), "hit_attrib_struct", false);
   nir_variable *struct_var = nir_variable_create(b->shader, nir_var_ray_hit_attrib, var_type, "hit_attrib_struct");

   nir_deref_instr *var_deref = nir_build_deref_var(b, struct_var);

   for (uint32_t i = 0; i < ARRAY_SIZE(fields); i++) {
      nir_deref_instr *member_deref = nir_build_deref_struct(b, var_deref, i);
      nir_def *load = nir_load_deref(b, member_deref);
      nir_store_deref(b, member_deref, load, (1 << load->num_components) - 1);
   }

   uint32_t masks[RADV_MAX_HIT_ATTRIB_DWORDS] = {
      /* vec3 */
      0xFFFFFFFF,
      0xFFFFFFFF,
      0xFFFFFFFF,
      /* padding */
      0,
      /* uint64_t */
      0xFFFFFFFF,
      0xFFFFFFFF,
      /* uint16_t uint8_t */
      0xFFFFFFFF,
      /* bool */
      1,
   };
   validate(masks, 0b11110111, true);
}

TEST_F(radv_nir_lower_hit_attrib_derefs_test, array_inside_struct)
{
   glsl_struct_field field =
      glsl_struct_field(glsl_array_type(glsl_uint_type(), RADV_MAX_HIT_ATTRIB_DWORDS, 0), "array");

   const glsl_type *var_type = glsl_struct_type(&field, 1, "hit_attrib_struct", false);
   nir_variable *struct_var = nir_variable_create(b->shader, nir_var_ray_hit_attrib, var_type, "hit_attrib_struct");

   nir_deref_instr *var_deref = nir_build_deref_var(b, struct_var);
   nir_deref_instr *member_deref = nir_build_deref_struct(b, var_deref, 0);

   for (uint32_t i = 0; i < RADV_MAX_HIT_ATTRIB_DWORDS; i++) {
      nir_deref_instr *element_deref = nir_build_deref_array_imm(b, member_deref, i);
      nir_def *load = nir_load_deref(b, element_deref);
      nir_store_deref(b, element_deref, load, (1 << load->num_components) - 1);
   }

   uint32_t masks[RADV_MAX_HIT_ATTRIB_DWORDS] = {
      0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
   };
   validate(masks, 0xFF, true);
}

TEST_F(radv_nir_lower_hit_attrib_derefs_test, struct_inside_array)
{
   glsl_struct_field field = glsl_struct_field(glsl_uint_type(), "array");
   const glsl_type *struct_type = glsl_struct_type(&field, 1, "hit_attrib_struct", false);
   nir_variable *array_var =
      nir_variable_create(b->shader, nir_var_ray_hit_attrib,
                          glsl_array_type(struct_type, RADV_MAX_HIT_ATTRIB_DWORDS, 0), "hit_attrib_struct[]");

   nir_deref_instr *var_deref = nir_build_deref_var(b, array_var);
   for (uint32_t i = 0; i < RADV_MAX_HIT_ATTRIB_DWORDS; i++) {
      nir_deref_instr *element_deref = nir_build_deref_array_imm(b, var_deref, i);
      nir_deref_instr *member_deref = nir_build_deref_struct(b, element_deref, 0);

      nir_def *load = nir_load_deref(b, member_deref);
      nir_store_deref(b, member_deref, load, (1 << load->num_components) - 1);
   }

   uint32_t masks[RADV_MAX_HIT_ATTRIB_DWORDS] = {
      0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
   };
   validate(masks, 0xFF, true);
}
