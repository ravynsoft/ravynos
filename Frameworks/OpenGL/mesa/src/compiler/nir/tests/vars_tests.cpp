/*
 * Copyright © 2018 Intel Corporation
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

#include "nir_test.h"
#include "nir_deref.h"

namespace {

class nir_vars_test : public nir_test {
protected:
   nir_vars_test();
   ~nir_vars_test();

   nir_variable *create_var(nir_variable_mode mode, const glsl_type *type,
                            const char *name) {
      if (mode == nir_var_function_temp)
         return nir_local_variable_create(b->impl, type, name);
      else
         return nir_variable_create(b->shader, mode, type, name);
   }

   nir_variable *create_int(nir_variable_mode mode, const char *name) {
      return create_var(mode, glsl_int_type(), name);
   }

   nir_variable *create_ivec2(nir_variable_mode mode, const char *name) {
      return create_var(mode, glsl_vector_type(GLSL_TYPE_INT, 2), name);
   }

   nir_variable *create_ivec4(nir_variable_mode mode, const char *name) {
      return create_var(mode, glsl_vector_type(GLSL_TYPE_INT, 4), name);
   }

   nir_variable **create_many_int(nir_variable_mode mode, const char *prefix, unsigned count) {
      nir_variable **result = (nir_variable **)linear_alloc_child(lin_ctx, sizeof(nir_variable *) * count);
      for (unsigned i = 0; i < count; i++)
         result[i] = create_int(mode, linear_asprintf(lin_ctx, "%s%u", prefix, i));
      return result;
   }

   nir_variable **create_many_ivec2(nir_variable_mode mode, const char *prefix, unsigned count) {
      nir_variable **result = (nir_variable **)linear_alloc_child(lin_ctx, sizeof(nir_variable *) * count);
      for (unsigned i = 0; i < count; i++)
         result[i] = create_ivec2(mode, linear_asprintf(lin_ctx, "%s%u", prefix, i));
      return result;
   }

   nir_variable **create_many_ivec4(nir_variable_mode mode, const char *prefix, unsigned count) {
      nir_variable **result = (nir_variable **)linear_alloc_child(lin_ctx, sizeof(nir_variable *) * count);
      for (unsigned i = 0; i < count; i++)
         result[i] = create_ivec4(mode, linear_asprintf(lin_ctx, "%s%u", prefix, i));
      return result;
   }

   unsigned count_derefs(nir_deref_type deref_type);
   unsigned count_intrinsics(nir_intrinsic_op intrinsic);
   unsigned count_function_temp_vars(void) {
      return exec_list_length(&b->impl->locals);
   }

   unsigned count_shader_temp_vars(void) {
      unsigned count = 0;
      nir_foreach_variable_with_modes(var, b->shader, nir_var_shader_temp)
         count++;
      return count;
   }

   nir_intrinsic_instr *get_intrinsic(nir_intrinsic_op intrinsic,
                                      unsigned index);

   nir_deref_instr *get_deref(nir_deref_type deref_type,
                              unsigned index);
   linear_ctx *lin_ctx;
};

nir_vars_test::nir_vars_test()
   : nir_test::nir_test("nir_vars_test")
{
   lin_ctx = linear_context(b->shader);
}

nir_vars_test::~nir_vars_test()
{
   if (HasFailure()) {
      printf("\nShader from the failed test:\n\n");
      nir_print_shader(b->shader, stdout);
   }
}

unsigned
nir_vars_test::count_intrinsics(nir_intrinsic_op intrinsic)
{
   unsigned count = 0;
   nir_foreach_block(block, b->impl) {
      nir_foreach_instr(instr, block) {
         if (instr->type != nir_instr_type_intrinsic)
            continue;
         nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
         if (intrin->intrinsic == intrinsic)
            count++;
      }
   }
   return count;
}

unsigned
nir_vars_test::count_derefs(nir_deref_type deref_type)
{
   unsigned count = 0;
   nir_foreach_block(block, b->impl) {
      nir_foreach_instr(instr, block) {
         if (instr->type != nir_instr_type_deref)
            continue;
         nir_deref_instr *intrin = nir_instr_as_deref(instr);
         if (intrin->deref_type == deref_type)
            count++;
      }
   }
   return count;
}

nir_intrinsic_instr *
nir_vars_test::get_intrinsic(nir_intrinsic_op intrinsic,
                             unsigned index)
{
   nir_foreach_block(block, b->impl) {
      nir_foreach_instr(instr, block) {
         if (instr->type != nir_instr_type_intrinsic)
            continue;
         nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
         if (intrin->intrinsic == intrinsic) {
            if (index == 0)
               return intrin;
            index--;
         }
      }
   }
   return NULL;
}

nir_deref_instr *
nir_vars_test::get_deref(nir_deref_type deref_type,
                         unsigned index)
{
   nir_foreach_block(block, b->impl) {
      nir_foreach_instr(instr, block) {
         if (instr->type != nir_instr_type_deref)
            continue;
         nir_deref_instr *deref = nir_instr_as_deref(instr);
         if (deref->deref_type == deref_type) {
            if (index == 0)
               return deref;
            index--;
         }
      }
   }
   return NULL;
}

/* Allow grouping the tests while still sharing the helpers. */
class nir_redundant_load_vars_test : public nir_vars_test {};
class nir_copy_prop_vars_test : public nir_vars_test {};
class nir_dead_write_vars_test : public nir_vars_test {};
class nir_combine_stores_test : public nir_vars_test {};
class nir_split_vars_test : public nir_vars_test {};
class nir_remove_dead_variables_test : public nir_vars_test {};

} // namespace

static nir_def *
nir_load_var_volatile(nir_builder *b, nir_variable *var)
{
   return nir_load_deref_with_access(b, nir_build_deref_var(b, var),
                                     ACCESS_VOLATILE);
}

static void
nir_store_var_volatile(nir_builder *b, nir_variable *var,
                       nir_def *value, nir_component_mask_t writemask)
{
   nir_store_deref_with_access(b, nir_build_deref_var(b, var),
                               value, writemask, ACCESS_VOLATILE);
}

TEST_F(nir_redundant_load_vars_test, duplicated_load)
{
   /* Load a variable twice in the same block.  One should be removed. */

   nir_variable *in = create_int(nir_var_mem_global, "in");
   nir_variable **out = create_many_int(nir_var_shader_out, "out", 2);

   nir_store_var(b, out[0], nir_load_var(b, in), 1);
   nir_store_var(b, out[1], nir_load_var(b, in), 1);

   nir_validate_shader(b->shader, NULL);

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 2);

   bool progress = nir_opt_copy_prop_vars(b->shader);
   EXPECT_TRUE(progress);

   nir_validate_shader(b->shader, NULL);

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 1);
}

TEST_F(nir_redundant_load_vars_test, duplicated_load_volatile)
{
   /* Load a variable twice in the same block.  One should be removed. */

   nir_variable *in = create_int(nir_var_mem_global, "in");
   nir_variable **out = create_many_int(nir_var_shader_out, "out", 3);

   /* Volatile prevents us from eliminating a load by combining it with
    * another.  It shouldn't however, prevent us from combing other
    * non-volatile loads.
    */
   nir_store_var(b, out[0], nir_load_var(b, in), 1);
   nir_store_var(b, out[1], nir_load_var_volatile(b, in), 1);
   nir_store_var(b, out[2], nir_load_var(b, in), 1);

   nir_validate_shader(b->shader, NULL);

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 3);

   bool progress = nir_opt_copy_prop_vars(b->shader);
   EXPECT_TRUE(progress);

   nir_validate_shader(b->shader, NULL);

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 2);

   nir_intrinsic_instr *first_store = get_intrinsic(nir_intrinsic_store_deref, 0);

   nir_intrinsic_instr *third_store = get_intrinsic(nir_intrinsic_store_deref, 2);

   EXPECT_EQ(first_store->src[1].ssa, third_store->src[1].ssa);
}

TEST_F(nir_redundant_load_vars_test, duplicated_load_in_two_blocks)
{
   /* Load a variable twice in different blocks.  One should be removed. */

   nir_variable *in = create_int(nir_var_mem_global, "in");
   nir_variable **out = create_many_int(nir_var_shader_out, "out", 2);

   nir_store_var(b, out[0], nir_load_var(b, in), 1);

   /* Forces the stores to be in different blocks. */
   nir_pop_if(b, nir_push_if(b, nir_imm_int(b, 0)));

   nir_store_var(b, out[1], nir_load_var(b, in), 1);

   nir_validate_shader(b->shader, NULL);

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 2);

   bool progress = nir_opt_copy_prop_vars(b->shader);
   EXPECT_TRUE(progress);

   nir_validate_shader(b->shader, NULL);

   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 1);
}

TEST_F(nir_redundant_load_vars_test, invalidate_inside_if_block)
{
   /* Load variables, then write to some of then in different branches of the
    * if statement.  They should be invalidated accordingly.
    */

   nir_variable **g = create_many_int(nir_var_shader_temp, "g", 3);
   nir_variable **out = create_many_int(nir_var_shader_out, "out", 3);

   nir_load_var(b, g[0]);
   nir_load_var(b, g[1]);
   nir_load_var(b, g[2]);

   nir_if *if_stmt = nir_push_if(b, nir_imm_int(b, 0));
   nir_store_var(b, g[0], nir_imm_int(b, 10), 1);

   nir_push_else(b, if_stmt);
   nir_store_var(b, g[1], nir_imm_int(b, 20), 1);

   nir_pop_if(b, if_stmt);

   nir_store_var(b, out[0], nir_load_var(b, g[0]), 1);
   nir_store_var(b, out[1], nir_load_var(b, g[1]), 1);
   nir_store_var(b, out[2], nir_load_var(b, g[2]), 1);

   nir_validate_shader(b->shader, NULL);

   bool progress = nir_opt_copy_prop_vars(b->shader);
   EXPECT_TRUE(progress);

   /* There are 3 initial loads, plus 2 loads for the values invalidated
    * inside the if statement.
    */
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 5);

   /* We only load g[2] once. */
   unsigned g2_load_count = 0;
   for (int i = 0; i < 5; i++) {
         nir_intrinsic_instr *load = get_intrinsic(nir_intrinsic_load_deref, i);
         if (nir_intrinsic_get_var(load, 0) == g[2])
            g2_load_count++;
   }
   EXPECT_EQ(g2_load_count, 1);
}

TEST_F(nir_redundant_load_vars_test, invalidate_live_load_in_the_end_of_loop)
{
   /* Invalidating a load in the end of loop body will apply to the whole loop
    * body.
    */

   nir_variable *v = create_int(nir_var_mem_global, "v");

   nir_load_var(b, v);

   nir_loop *loop = nir_push_loop(b);

   nir_if *if_stmt = nir_push_if(b, nir_imm_int(b, 0));
   nir_jump(b, nir_jump_break);
   nir_pop_if(b, if_stmt);

   nir_load_var(b, v);
   nir_store_var(b, v, nir_imm_int(b, 10), 1);

   nir_pop_loop(b, loop);

   bool progress = nir_opt_copy_prop_vars(b->shader);
   ASSERT_FALSE(progress);
}

TEST_F(nir_copy_prop_vars_test, simple_copies)
{
   nir_variable *in   = create_int(nir_var_shader_in,     "in");
   nir_variable *temp = create_int(nir_var_function_temp, "temp");
   nir_variable *out  = create_int(nir_var_shader_out,    "out");

   nir_copy_var(b, temp, in);
   nir_copy_var(b, out, temp);

   nir_validate_shader(b->shader, NULL);

   bool progress = nir_opt_copy_prop_vars(b->shader);
   EXPECT_TRUE(progress);

   nir_validate_shader(b->shader, NULL);

   ASSERT_EQ(count_intrinsics(nir_intrinsic_copy_deref), 2);

   nir_intrinsic_instr *first_copy = get_intrinsic(nir_intrinsic_copy_deref, 0);

   nir_intrinsic_instr *second_copy = get_intrinsic(nir_intrinsic_copy_deref, 1);

   EXPECT_EQ(first_copy->src[1].ssa, second_copy->src[1].ssa);
}

TEST_F(nir_copy_prop_vars_test, self_copy)
{
   nir_variable *v = create_int(nir_var_mem_global, "v");

   nir_copy_var(b, v, v);

   nir_validate_shader(b->shader, NULL);

   bool progress = nir_opt_copy_prop_vars(b->shader);
   EXPECT_TRUE(progress);

   nir_validate_shader(b->shader, NULL);

   ASSERT_EQ(count_intrinsics(nir_intrinsic_copy_deref), 0);
}

TEST_F(nir_copy_prop_vars_test, simple_store_load)
{
   nir_variable **v = create_many_ivec2(nir_var_function_temp, "v", 2);
   unsigned mask = 1 | 2;

   nir_def *stored_value = nir_imm_ivec2(b, 10, 20);
   nir_store_var(b, v[0], stored_value, mask);

   nir_def *read_value = nir_load_var(b, v[0]);
   nir_store_var(b, v[1], read_value, mask);

   nir_validate_shader(b->shader, NULL);

   bool progress = nir_opt_copy_prop_vars(b->shader);
   EXPECT_TRUE(progress);

   nir_validate_shader(b->shader, NULL);

   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_deref), 2);

   for (int i = 0; i < 2; i++) {
      nir_intrinsic_instr *store = get_intrinsic(nir_intrinsic_store_deref, i);
      EXPECT_EQ(store->src[1].ssa, stored_value);
   }
}

TEST_F(nir_copy_prop_vars_test, store_store_load)
{
   nir_variable **v = create_many_ivec2(nir_var_function_temp, "v", 2);
   unsigned mask = 1 | 2;

   nir_def *first_value = nir_imm_ivec2(b, 10, 20);
   nir_store_var(b, v[0], first_value, mask);

   nir_def *second_value = nir_imm_ivec2(b, 30, 40);
   nir_store_var(b, v[0], second_value, mask);

   nir_def *read_value = nir_load_var(b, v[0]);
   nir_store_var(b, v[1], read_value, mask);

   nir_validate_shader(b->shader, NULL);

   bool progress = nir_opt_copy_prop_vars(b->shader);
   EXPECT_TRUE(progress);

   nir_validate_shader(b->shader, NULL);

   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_deref), 3);

   /* Store to v[1] should use second_value directly. */
   nir_intrinsic_instr *store_to_v1 = get_intrinsic(nir_intrinsic_store_deref, 2);
   ASSERT_EQ(nir_intrinsic_get_var(store_to_v1, 0), v[1]);
   EXPECT_EQ(store_to_v1->src[1].ssa, second_value);
}

TEST_F(nir_copy_prop_vars_test, store_store_load_different_components)
{
   nir_variable **v = create_many_ivec2(nir_var_function_temp, "v", 2);

   nir_def *first_value = nir_imm_ivec2(b, 10, 20);
   nir_store_var(b, v[0], first_value, 1 << 1);

   nir_def *second_value = nir_imm_ivec2(b, 30, 40);
   nir_store_var(b, v[0], second_value, 1 << 0);

   nir_def *read_value = nir_load_var(b, v[0]);
   nir_store_var(b, v[1], read_value, 1 << 1);

   nir_validate_shader(b->shader, NULL);

   bool progress = nir_opt_copy_prop_vars(b->shader);
   EXPECT_TRUE(progress);

   nir_validate_shader(b->shader, NULL);

   nir_opt_constant_folding(b->shader);
   nir_validate_shader(b->shader, NULL);

   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_deref), 3);

   /* Store to v[1] should use first_value directly.  The write of
    * second_value did not overwrite the component it uses.
    */
   nir_intrinsic_instr *store_to_v1 = get_intrinsic(nir_intrinsic_store_deref, 2);
   ASSERT_EQ(nir_intrinsic_get_var(store_to_v1, 0), v[1]);
   ASSERT_EQ(nir_src_comp_as_uint(store_to_v1->src[1], 1), 20);
}

TEST_F(nir_copy_prop_vars_test, store_store_load_different_components_in_many_blocks)
{
   nir_variable **v = create_many_ivec2(nir_var_function_temp, "v", 2);

   nir_def *first_value = nir_imm_ivec2(b, 10, 20);
   nir_store_var(b, v[0], first_value, 1 << 1);

   /* Adding an if statement will cause blocks to be created. */
   nir_pop_if(b, nir_push_if(b, nir_imm_int(b, 0)));

   nir_def *second_value = nir_imm_ivec2(b, 30, 40);
   nir_store_var(b, v[0], second_value, 1 << 0);

   /* Adding an if statement will cause blocks to be created. */
   nir_pop_if(b, nir_push_if(b, nir_imm_int(b, 0)));

   nir_def *read_value = nir_load_var(b, v[0]);
   nir_store_var(b, v[1], read_value, 1 << 1);

   nir_validate_shader(b->shader, NULL);

   bool progress = nir_opt_copy_prop_vars(b->shader);
   EXPECT_TRUE(progress);

   nir_validate_shader(b->shader, NULL);

   nir_opt_constant_folding(b->shader);
   nir_validate_shader(b->shader, NULL);

   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_deref), 3);

   /* Store to v[1] should use first_value directly.  The write of
    * second_value did not overwrite the component it uses.
    */
   nir_intrinsic_instr *store_to_v1 = get_intrinsic(nir_intrinsic_store_deref, 2);
   ASSERT_EQ(nir_intrinsic_get_var(store_to_v1, 0), v[1]);
   ASSERT_EQ(nir_src_comp_as_uint(store_to_v1->src[1], 1), 20);
}

TEST_F(nir_copy_prop_vars_test, store_volatile)
{
   nir_variable **v = create_many_ivec2(nir_var_function_temp, "v", 2);
   unsigned mask = 1 | 2;

   nir_def *first_value = nir_imm_ivec2(b, 10, 20);
   nir_store_var(b, v[0], first_value, mask);

   nir_def *second_value = nir_imm_ivec2(b, 30, 40);
   nir_store_var_volatile(b, v[0], second_value, mask);

   nir_def *third_value = nir_imm_ivec2(b, 50, 60);
   nir_store_var(b, v[0], third_value, mask);

   nir_def *read_value = nir_load_var(b, v[0]);
   nir_store_var(b, v[1], read_value, mask);

   nir_validate_shader(b->shader, NULL);

   bool progress = nir_opt_copy_prop_vars(b->shader);
   EXPECT_TRUE(progress);

   nir_validate_shader(b->shader, NULL);

   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_deref), 4);

   /* Our approach here is a bit scorched-earth.  We expect the volatile store
    * in the middle to cause both that store and the one before it to be kept.
    * Technically, volatile only prevents combining the volatile store with
    * another store and one could argue that the store before the volatile and
    * the one after it could be combined.  However, it seems safer to just
    * treat a volatile store like an atomic and prevent any combining across
    * it.
    */
   nir_intrinsic_instr *store_to_v1 = get_intrinsic(nir_intrinsic_store_deref, 3);
   ASSERT_EQ(nir_intrinsic_get_var(store_to_v1, 0), v[1]);
   EXPECT_EQ(store_to_v1->src[1].ssa, third_value);
}

TEST_F(nir_copy_prop_vars_test, self_copy_volatile)
{
   nir_variable *v = create_int(nir_var_mem_global, "v");

   nir_copy_var(b, v, v);
   nir_copy_deref_with_access(b, nir_build_deref_var(b, v),
                                 nir_build_deref_var(b, v),
                                 (gl_access_qualifier)0, ACCESS_VOLATILE);
   nir_copy_deref_with_access(b, nir_build_deref_var(b, v),
                                 nir_build_deref_var(b, v),
                                 ACCESS_VOLATILE, (gl_access_qualifier)0);
   nir_copy_var(b, v, v);

   nir_validate_shader(b->shader, NULL);

   bool progress = nir_opt_copy_prop_vars(b->shader);
   EXPECT_TRUE(progress);

   nir_validate_shader(b->shader, NULL);

   ASSERT_EQ(count_intrinsics(nir_intrinsic_copy_deref), 2);

   /* Store to v[1] should use second_value directly. */
   nir_intrinsic_instr *first = get_intrinsic(nir_intrinsic_copy_deref, 0);
   nir_intrinsic_instr *second = get_intrinsic(nir_intrinsic_copy_deref, 1);
   ASSERT_EQ(nir_intrinsic_src_access(first), ACCESS_VOLATILE);
   ASSERT_EQ(nir_intrinsic_dst_access(first), (gl_access_qualifier)0);
   ASSERT_EQ(nir_intrinsic_src_access(second), (gl_access_qualifier)0);
   ASSERT_EQ(nir_intrinsic_dst_access(second), ACCESS_VOLATILE);
}

TEST_F(nir_copy_prop_vars_test, memory_barrier_in_two_blocks)
{
   nir_variable **v = create_many_int(nir_var_mem_global, "v", 4);

   nir_store_var(b, v[0], nir_imm_int(b, 1), 1);
   nir_store_var(b, v[1], nir_imm_int(b, 2), 1);

   /* Split into many blocks. */
   nir_pop_if(b, nir_push_if(b, nir_imm_int(b, 0)));

   nir_store_var(b, v[2], nir_load_var(b, v[0]), 1);

   nir_scoped_memory_barrier(b, SCOPE_DEVICE, NIR_MEMORY_ACQ_REL,
                             nir_var_mem_global);

   nir_store_var(b, v[3], nir_load_var(b, v[1]), 1);

   bool progress = nir_opt_copy_prop_vars(b->shader);
   ASSERT_TRUE(progress);

   /* Only the second load will remain after the optimization. */
   ASSERT_EQ(1, count_intrinsics(nir_intrinsic_load_deref));
   nir_intrinsic_instr *load = get_intrinsic(nir_intrinsic_load_deref, 0);
   ASSERT_EQ(nir_intrinsic_get_var(load, 0), v[1]);
}

TEST_F(nir_redundant_load_vars_test, acquire_barrier_prevents_load_removal)
{
   nir_variable **x = create_many_int(nir_var_mem_global, "x", 1);

   nir_load_var(b, x[0]);

   nir_scoped_memory_barrier(b, SCOPE_DEVICE, NIR_MEMORY_ACQUIRE,
                             nir_var_mem_global);

   nir_load_var(b, x[0]);

   bool progress = nir_opt_copy_prop_vars(b->shader);
   ASSERT_FALSE(progress);

   ASSERT_EQ(2, count_intrinsics(nir_intrinsic_load_deref));
}

TEST_F(nir_redundant_load_vars_test, acquire_barrier_prevents_same_mode_load_removal)
{
   nir_variable **x = create_many_int(nir_var_mem_global, "x", 2);

   nir_load_var(b, x[0]);
   nir_load_var(b, x[1]);

   nir_scoped_memory_barrier(b, SCOPE_DEVICE, NIR_MEMORY_ACQUIRE,
                             nir_var_mem_global);

   nir_load_var(b, x[0]);
   nir_load_var(b, x[1]);

   bool progress = nir_opt_copy_prop_vars(b->shader);
   ASSERT_FALSE(progress);

   ASSERT_EQ(4, count_intrinsics(nir_intrinsic_load_deref));
}

TEST_F(nir_redundant_load_vars_test, acquire_barrier_allows_different_mode_load_removal)
{
   nir_variable **x = create_many_int(nir_var_mem_global, "x", 2);
   nir_variable **y = create_many_int(nir_var_mem_shared, "y", 2);

   nir_load_var(b, x[0]);
   nir_load_var(b, x[1]);
   nir_load_var(b, y[0]);
   nir_load_var(b, y[1]);

   nir_scoped_memory_barrier(b, SCOPE_DEVICE, NIR_MEMORY_ACQUIRE,
                             nir_var_mem_global);

   nir_load_var(b, x[0]);
   nir_load_var(b, x[1]);
   nir_load_var(b, y[0]);
   nir_load_var(b, y[1]);

   bool progress = nir_opt_copy_prop_vars(b->shader);
   ASSERT_TRUE(progress);

   ASSERT_EQ(6, count_intrinsics(nir_intrinsic_load_deref));

   nir_intrinsic_instr *load;

   load = get_intrinsic(nir_intrinsic_load_deref, 0);
   ASSERT_EQ(nir_intrinsic_get_var(load, 0), x[0]);
   load = get_intrinsic(nir_intrinsic_load_deref, 1);
   ASSERT_EQ(nir_intrinsic_get_var(load, 0), x[1]);

   load = get_intrinsic(nir_intrinsic_load_deref, 2);
   ASSERT_EQ(nir_intrinsic_get_var(load, 0), y[0]);
   load = get_intrinsic(nir_intrinsic_load_deref, 3);
   ASSERT_EQ(nir_intrinsic_get_var(load, 0), y[1]);

   load = get_intrinsic(nir_intrinsic_load_deref, 4);
   ASSERT_EQ(nir_intrinsic_get_var(load, 0), x[0]);
   load = get_intrinsic(nir_intrinsic_load_deref, 5);
   ASSERT_EQ(nir_intrinsic_get_var(load, 0), x[1]);
}

TEST_F(nir_redundant_load_vars_test, release_barrier_allows_load_removal)
{
   nir_variable **x = create_many_int(nir_var_mem_global, "x", 1);

   nir_load_var(b, x[0]);

   nir_scoped_memory_barrier(b, SCOPE_DEVICE, NIR_MEMORY_RELEASE,
                             nir_var_mem_global);

   nir_load_var(b, x[0]);

   bool progress = nir_opt_copy_prop_vars(b->shader);
   ASSERT_TRUE(progress);

   ASSERT_EQ(1, count_intrinsics(nir_intrinsic_load_deref));
}

TEST_F(nir_redundant_load_vars_test, release_barrier_allows_same_mode_load_removal)
{
   nir_variable **x = create_many_int(nir_var_mem_global, "x", 2);

   nir_load_var(b, x[0]);
   nir_load_var(b, x[1]);

   nir_scoped_memory_barrier(b, SCOPE_DEVICE, NIR_MEMORY_RELEASE,
                             nir_var_mem_global);

   nir_load_var(b, x[0]);
   nir_load_var(b, x[1]);

   bool progress = nir_opt_copy_prop_vars(b->shader);
   ASSERT_TRUE(progress);

   ASSERT_EQ(2, count_intrinsics(nir_intrinsic_load_deref));
}

TEST_F(nir_redundant_load_vars_test, release_barrier_allows_different_mode_load_removal)
{
   nir_variable **x = create_many_int(nir_var_mem_global, "x", 2);
   nir_variable **y = create_many_int(nir_var_mem_shared, "y", 2);

   nir_load_var(b, x[0]);
   nir_load_var(b, x[1]);
   nir_load_var(b, y[0]);
   nir_load_var(b, y[1]);

   nir_scoped_memory_barrier(b, SCOPE_DEVICE, NIR_MEMORY_RELEASE,
                             nir_var_mem_global);

   nir_load_var(b, x[0]);
   nir_load_var(b, x[1]);
   nir_load_var(b, y[0]);
   nir_load_var(b, y[1]);

   bool progress = nir_opt_copy_prop_vars(b->shader);
   ASSERT_TRUE(progress);

   ASSERT_EQ(4, count_intrinsics(nir_intrinsic_load_deref));

   nir_intrinsic_instr *load;

   load = get_intrinsic(nir_intrinsic_load_deref, 0);
   ASSERT_EQ(nir_intrinsic_get_var(load, 0), x[0]);
   load = get_intrinsic(nir_intrinsic_load_deref, 1);
   ASSERT_EQ(nir_intrinsic_get_var(load, 0), x[1]);

   load = get_intrinsic(nir_intrinsic_load_deref, 2);
   ASSERT_EQ(nir_intrinsic_get_var(load, 0), y[0]);
   load = get_intrinsic(nir_intrinsic_load_deref, 3);
   ASSERT_EQ(nir_intrinsic_get_var(load, 0), y[1]);
}

TEST_F(nir_copy_prop_vars_test, acquire_barrier_prevents_propagation)
{
   nir_variable **x = create_many_int(nir_var_mem_global, "x", 1);

   nir_store_var(b, x[0], nir_imm_int(b, 10), 1);

   nir_scoped_memory_barrier(b, SCOPE_DEVICE, NIR_MEMORY_ACQUIRE,
                             nir_var_mem_global);

   nir_load_var(b, x[0]);

   bool progress = nir_opt_copy_prop_vars(b->shader);
   ASSERT_FALSE(progress);

   ASSERT_EQ(1, count_intrinsics(nir_intrinsic_store_deref));
   ASSERT_EQ(1, count_intrinsics(nir_intrinsic_load_deref));
}

TEST_F(nir_copy_prop_vars_test, acquire_barrier_prevents_same_mode_propagation)
{
   nir_variable **x = create_many_int(nir_var_mem_global, "x", 2);

   nir_store_var(b, x[0], nir_imm_int(b, 10), 1);
   nir_store_var(b, x[1], nir_imm_int(b, 20), 1);

   nir_scoped_memory_barrier(b, SCOPE_DEVICE, NIR_MEMORY_ACQUIRE,
                             nir_var_mem_global);

   nir_load_var(b, x[0]);
   nir_load_var(b, x[1]);

   bool progress = nir_opt_copy_prop_vars(b->shader);
   ASSERT_FALSE(progress);

   ASSERT_EQ(2, count_intrinsics(nir_intrinsic_store_deref));
   ASSERT_EQ(2, count_intrinsics(nir_intrinsic_load_deref));
}

TEST_F(nir_copy_prop_vars_test, acquire_barrier_allows_different_mode_propagation)
{
   nir_variable **x = create_many_int(nir_var_mem_global, "x", 2);
   nir_variable **y = create_many_int(nir_var_mem_shared, "y", 2);

   nir_store_var(b, x[0], nir_imm_int(b, 10), 1);
   nir_store_var(b, x[1], nir_imm_int(b, 20), 1);
   nir_store_var(b, y[0], nir_imm_int(b, 30), 1);
   nir_store_var(b, y[1], nir_imm_int(b, 40), 1);

   nir_scoped_memory_barrier(b, SCOPE_DEVICE, NIR_MEMORY_ACQUIRE,
                             nir_var_mem_global);

   nir_load_var(b, x[0]);
   nir_load_var(b, x[1]);
   nir_load_var(b, y[0]);
   nir_load_var(b, y[1]);

   bool progress = nir_opt_copy_prop_vars(b->shader);
   ASSERT_TRUE(progress);

   ASSERT_EQ(4, count_intrinsics(nir_intrinsic_store_deref));
   ASSERT_EQ(2, count_intrinsics(nir_intrinsic_load_deref));

   nir_intrinsic_instr *store;

   store = get_intrinsic(nir_intrinsic_store_deref, 0);
   ASSERT_EQ(nir_intrinsic_get_var(store, 0), x[0]);
   store = get_intrinsic(nir_intrinsic_store_deref, 1);
   ASSERT_EQ(nir_intrinsic_get_var(store, 0), x[1]);

   store = get_intrinsic(nir_intrinsic_store_deref, 2);
   ASSERT_EQ(nir_intrinsic_get_var(store, 0), y[0]);
   store = get_intrinsic(nir_intrinsic_store_deref, 3);
   ASSERT_EQ(nir_intrinsic_get_var(store, 0), y[1]);

   nir_intrinsic_instr *load;

   load = get_intrinsic(nir_intrinsic_load_deref, 0);
   ASSERT_EQ(nir_intrinsic_get_var(load, 0), x[0]);
   load = get_intrinsic(nir_intrinsic_load_deref, 1);
   ASSERT_EQ(nir_intrinsic_get_var(load, 0), x[1]);
}

TEST_F(nir_copy_prop_vars_test, release_barrier_allows_propagation)
{
   nir_variable **x = create_many_int(nir_var_mem_global, "x", 1);

   nir_store_var(b, x[0], nir_imm_int(b, 10), 1);

   nir_scoped_memory_barrier(b, SCOPE_DEVICE, NIR_MEMORY_RELEASE,
                             nir_var_mem_global);

   nir_load_var(b, x[0]);

   bool progress = nir_opt_copy_prop_vars(b->shader);
   ASSERT_TRUE(progress);

   ASSERT_EQ(1, count_intrinsics(nir_intrinsic_store_deref));
}

TEST_F(nir_copy_prop_vars_test, release_barrier_allows_same_mode_propagation)
{
   nir_variable **x = create_many_int(nir_var_mem_global, "x", 2);

   nir_store_var(b, x[0], nir_imm_int(b, 10), 1);
   nir_store_var(b, x[1], nir_imm_int(b, 20), 1);

   nir_scoped_memory_barrier(b, SCOPE_DEVICE, NIR_MEMORY_RELEASE,
                             nir_var_mem_global);

   nir_load_var(b, x[0]);
   nir_load_var(b, x[1]);

   bool progress = nir_opt_copy_prop_vars(b->shader);
   ASSERT_TRUE(progress);

   ASSERT_EQ(2, count_intrinsics(nir_intrinsic_store_deref));
   ASSERT_EQ(0, count_intrinsics(nir_intrinsic_load_deref));
}

TEST_F(nir_copy_prop_vars_test, release_barrier_allows_different_mode_propagation)
{
   nir_variable **x = create_many_int(nir_var_mem_global, "x", 2);
   nir_variable **y = create_many_int(nir_var_mem_shared, "y", 2);

   nir_store_var(b, x[0], nir_imm_int(b, 10), 1);
   nir_store_var(b, x[1], nir_imm_int(b, 20), 1);
   nir_store_var(b, y[0], nir_imm_int(b, 30), 1);
   nir_store_var(b, y[1], nir_imm_int(b, 40), 1);

   nir_scoped_memory_barrier(b, SCOPE_DEVICE, NIR_MEMORY_RELEASE,
                             nir_var_mem_global);

   nir_load_var(b, x[0]);
   nir_load_var(b, x[1]);
   nir_load_var(b, y[0]);
   nir_load_var(b, y[1]);

   bool progress = nir_opt_copy_prop_vars(b->shader);
   ASSERT_TRUE(progress);

   ASSERT_EQ(4, count_intrinsics(nir_intrinsic_store_deref));
   ASSERT_EQ(0, count_intrinsics(nir_intrinsic_load_deref));

   nir_intrinsic_instr *store;

   store = get_intrinsic(nir_intrinsic_store_deref, 0);
   ASSERT_EQ(nir_intrinsic_get_var(store, 0), x[0]);
   store = get_intrinsic(nir_intrinsic_store_deref, 1);
   ASSERT_EQ(nir_intrinsic_get_var(store, 0), x[1]);

   store = get_intrinsic(nir_intrinsic_store_deref, 2);
   ASSERT_EQ(nir_intrinsic_get_var(store, 0), y[0]);
   store = get_intrinsic(nir_intrinsic_store_deref, 3);
   ASSERT_EQ(nir_intrinsic_get_var(store, 0), y[1]);
}

TEST_F(nir_copy_prop_vars_test, acquire_barrier_prevents_propagation_from_copy)
{
   nir_variable **x = create_many_int(nir_var_mem_global, "x", 3);

   nir_copy_var(b, x[1], x[0]);

   nir_scoped_memory_barrier(b, SCOPE_DEVICE, NIR_MEMORY_ACQUIRE,
                             nir_var_mem_global);

   nir_copy_var(b, x[2], x[1]);

   bool progress = nir_opt_copy_prop_vars(b->shader);
   ASSERT_FALSE(progress);

   ASSERT_EQ(2, count_intrinsics(nir_intrinsic_copy_deref));

   nir_intrinsic_instr *copy;

   copy = get_intrinsic(nir_intrinsic_copy_deref, 0);
   ASSERT_EQ(nir_intrinsic_get_var(copy, 1), x[0]);

   copy = get_intrinsic(nir_intrinsic_copy_deref, 1);
   ASSERT_EQ(nir_intrinsic_get_var(copy, 1), x[1]);
}

TEST_F(nir_copy_prop_vars_test, acquire_barrier_prevents_propagation_from_copy_to_different_mode)
{
   nir_variable **x = create_many_int(nir_var_mem_global, "x", 2);
   nir_variable **y = create_many_int(nir_var_mem_shared, "y", 1);

   nir_copy_var(b, y[0], x[0]);

   nir_scoped_memory_barrier(b, SCOPE_DEVICE, NIR_MEMORY_ACQUIRE,
                             nir_var_mem_global);

   nir_copy_var(b, x[1], y[0]);

   bool progress = nir_opt_copy_prop_vars(b->shader);
   ASSERT_FALSE(progress);

   ASSERT_EQ(2, count_intrinsics(nir_intrinsic_copy_deref));

   nir_intrinsic_instr *copy;

   copy = get_intrinsic(nir_intrinsic_copy_deref, 0);
   ASSERT_EQ(nir_intrinsic_get_var(copy, 1), x[0]);

   copy = get_intrinsic(nir_intrinsic_copy_deref, 1);
   ASSERT_EQ(nir_intrinsic_get_var(copy, 1), y[0]);
}

TEST_F(nir_copy_prop_vars_test, release_barrier_allows_propagation_from_copy)
{
   nir_variable **x = create_many_int(nir_var_mem_global, "x", 3);

   nir_copy_var(b, x[1], x[0]);

   nir_scoped_memory_barrier(b, SCOPE_DEVICE, NIR_MEMORY_RELEASE,
                             nir_var_mem_global);

   nir_copy_var(b, x[2], x[1]);

   bool progress = nir_opt_copy_prop_vars(b->shader);
   ASSERT_TRUE(progress);

   ASSERT_EQ(2, count_intrinsics(nir_intrinsic_copy_deref));

   nir_intrinsic_instr *copy;

   copy = get_intrinsic(nir_intrinsic_copy_deref, 0);
   ASSERT_EQ(nir_intrinsic_get_var(copy, 1), x[0]);

   copy = get_intrinsic(nir_intrinsic_copy_deref, 1);
   ASSERT_EQ(nir_intrinsic_get_var(copy, 1), x[0]);
}

TEST_F(nir_copy_prop_vars_test, release_barrier_allows_propagation_from_copy_to_different_mode)
{
   nir_variable **x = create_many_int(nir_var_mem_global, "x", 2);
   nir_variable **y = create_many_int(nir_var_mem_shared, "y", 1);

   nir_copy_var(b, y[0], x[0]);

   nir_scoped_memory_barrier(b, SCOPE_DEVICE, NIR_MEMORY_RELEASE,
                             nir_var_mem_global);

   nir_copy_var(b, x[1], y[0]);

   bool progress = nir_opt_copy_prop_vars(b->shader);
   ASSERT_TRUE(progress);

   ASSERT_EQ(2, count_intrinsics(nir_intrinsic_copy_deref));

   nir_intrinsic_instr *copy;

   copy = get_intrinsic(nir_intrinsic_copy_deref, 0);
   ASSERT_EQ(nir_intrinsic_get_var(copy, 1), x[0]);

   copy = get_intrinsic(nir_intrinsic_copy_deref, 1);
   ASSERT_EQ(nir_intrinsic_get_var(copy, 1), x[0]);
}

TEST_F(nir_copy_prop_vars_test, simple_store_load_in_two_blocks)
{
   nir_variable **v = create_many_ivec2(nir_var_function_temp, "v", 2);
   unsigned mask = 1 | 2;

   nir_def *stored_value = nir_imm_ivec2(b, 10, 20);
   nir_store_var(b, v[0], stored_value, mask);

   /* Adding an if statement will cause blocks to be created. */
   nir_pop_if(b, nir_push_if(b, nir_imm_int(b, 0)));

   nir_def *read_value = nir_load_var(b, v[0]);
   nir_store_var(b, v[1], read_value, mask);

   nir_validate_shader(b->shader, NULL);

   bool progress = nir_opt_copy_prop_vars(b->shader);
   EXPECT_TRUE(progress);

   nir_validate_shader(b->shader, NULL);

   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_deref), 2);

   for (int i = 0; i < 2; i++) {
      nir_intrinsic_instr *store = get_intrinsic(nir_intrinsic_store_deref, i);
      EXPECT_EQ(store->src[1].ssa, stored_value);
   }
}

TEST_F(nir_copy_prop_vars_test, load_direct_array_deref_on_vector_reuses_previous_load)
{
   nir_variable *in0 = create_ivec2(nir_var_mem_global, "in0");
   nir_variable *in1 = create_ivec2(nir_var_mem_global, "in1");
   nir_variable *vec = create_ivec2(nir_var_mem_global, "vec");
   nir_variable *out = create_int(nir_var_mem_global, "out");

   nir_store_var(b, vec, nir_load_var(b, in0), 1 << 0);
   nir_store_var(b, vec, nir_load_var(b, in1), 1 << 1);

   /* This load will be dropped, as vec.y (or vec[1]) is already known. */
   nir_deref_instr *deref =
      nir_build_deref_array_imm(b, nir_build_deref_var(b, vec), 1);
   nir_def *loaded_from_deref = nir_load_deref(b, deref);

   /* This store should use the value loaded from in1. */
   nir_store_var(b, out, loaded_from_deref, 1 << 0);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 3);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_deref), 3);

   bool progress = nir_opt_copy_prop_vars(b->shader);
   EXPECT_TRUE(progress);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 2);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_deref), 3);

   nir_intrinsic_instr *store = get_intrinsic(nir_intrinsic_store_deref, 2);

   /* NOTE: The ALU instruction is how we get the vec.y. */
   ASSERT_TRUE(nir_src_as_alu_instr(store->src[1]));
}

TEST_F(nir_copy_prop_vars_test, load_direct_array_deref_on_vector_reuses_previous_copy)
{
   nir_variable *in0 = create_ivec2(nir_var_mem_global, "in0");
   nir_variable *vec = create_ivec2(nir_var_mem_global, "vec");

   nir_copy_var(b, vec, in0);

   /* This load will be replaced with one from in0. */
   nir_deref_instr *deref =
      nir_build_deref_array_imm(b, nir_build_deref_var(b, vec), 1);
   nir_load_deref(b, deref);

   nir_validate_shader(b->shader, NULL);

   bool progress = nir_opt_copy_prop_vars(b->shader);
   EXPECT_TRUE(progress);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 1);

   nir_intrinsic_instr *load = get_intrinsic(nir_intrinsic_load_deref, 0);
   ASSERT_EQ(nir_intrinsic_get_var(load, 0), in0);
}

TEST_F(nir_copy_prop_vars_test, load_direct_array_deref_on_vector_gets_reused)
{
   nir_variable *in0 = create_ivec2(nir_var_mem_global, "in0");
   nir_variable *vec = create_ivec2(nir_var_mem_global, "vec");
   nir_variable *out = create_ivec2(nir_var_mem_global, "out");

   /* Loading "vec[1]" deref will save the information about vec.y. */
   nir_deref_instr *deref =
      nir_build_deref_array_imm(b, nir_build_deref_var(b, vec), 1);
   nir_load_deref(b, deref);

   /* Store to vec.x. */
   nir_store_var(b, vec, nir_load_var(b, in0), 1 << 0);

   /* This load will be dropped, since both vec.x and vec.y are known. */
   nir_def *loaded_from_vec = nir_load_var(b, vec);
   nir_store_var(b, out, loaded_from_vec, 0x3);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 3);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_deref), 2);

   bool progress = nir_opt_copy_prop_vars(b->shader);
   EXPECT_TRUE(progress);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 2);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_deref), 2);

   nir_intrinsic_instr *store = get_intrinsic(nir_intrinsic_store_deref, 1);
   ASSERT_TRUE(nir_src_as_alu_instr(store->src[1]));
}

TEST_F(nir_copy_prop_vars_test, store_load_direct_array_deref_on_vector)
{
   nir_variable *vec = create_ivec2(nir_var_mem_global, "vec");
   nir_variable *out0 = create_int(nir_var_mem_global, "out0");
   nir_variable *out1 = create_ivec2(nir_var_mem_global, "out1");

   /* Store to "vec[1]" and "vec[0]". */
   nir_deref_instr *store_deref_y =
      nir_build_deref_array_imm(b, nir_build_deref_var(b, vec), 1);
   nir_store_deref(b, store_deref_y, nir_imm_int(b, 20), 1);

   nir_deref_instr *store_deref_x =
      nir_build_deref_array_imm(b, nir_build_deref_var(b, vec), 0);
   nir_store_deref(b, store_deref_x, nir_imm_int(b, 10), 1);

   /* Both loads below will be dropped, because the values are already known. */
   nir_deref_instr *load_deref_y =
      nir_build_deref_array_imm(b, nir_build_deref_var(b, vec), 1);
   nir_store_var(b, out0, nir_load_deref(b, load_deref_y), 1);

   nir_store_var(b, out1, nir_load_var(b, vec), 1);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 2);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_deref), 4);

   bool progress = nir_opt_copy_prop_vars(b->shader);
   EXPECT_TRUE(progress);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 0);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_deref), 4);

   /* Third store will just use the value from first store. */
   nir_intrinsic_instr *first_store = get_intrinsic(nir_intrinsic_store_deref, 0);
   nir_intrinsic_instr *third_store = get_intrinsic(nir_intrinsic_store_deref, 2);
   EXPECT_EQ(third_store->src[1].ssa, first_store->src[1].ssa);

   /* Fourth store will compose first and second store values. */
   nir_intrinsic_instr *fourth_store = get_intrinsic(nir_intrinsic_store_deref, 3);
   EXPECT_TRUE(nir_src_as_alu_instr(fourth_store->src[1]));
}

TEST_F(nir_copy_prop_vars_test, store_load_indirect_array_deref_on_vector)
{
   nir_variable *vec = create_ivec2(nir_var_mem_global, "vec");
   nir_variable *idx = create_int(nir_var_mem_global, "idx");
   nir_variable *out = create_int(nir_var_mem_global, "out");

   nir_def *idx_ssa = nir_load_var(b, idx);

   /* Store to vec[idx]. */
   nir_deref_instr *store_deref =
      nir_build_deref_array(b, nir_build_deref_var(b, vec), idx_ssa);
   nir_store_deref(b, store_deref, nir_imm_int(b, 20), 1);

   /* Load from vec[idx] to store in out. This load should be dropped. */
   nir_deref_instr *load_deref =
      nir_build_deref_array(b, nir_build_deref_var(b, vec), idx_ssa);
   nir_store_var(b, out, nir_load_deref(b, load_deref), 1);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 2);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_deref), 2);

   bool progress = nir_opt_copy_prop_vars(b->shader);
   EXPECT_TRUE(progress);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 1);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_deref), 2);

   /* Store to vec[idx] propagated to out. */
   nir_intrinsic_instr *first = get_intrinsic(nir_intrinsic_store_deref, 0);
   nir_intrinsic_instr *second = get_intrinsic(nir_intrinsic_store_deref, 1);
   EXPECT_EQ(first->src[1].ssa, second->src[1].ssa);
}

TEST_F(nir_copy_prop_vars_test, store_load_direct_and_indirect_array_deref_on_vector)
{
   nir_variable *vec = create_ivec2(nir_var_mem_global, "vec");
   nir_variable *idx = create_int(nir_var_mem_global, "idx");
   nir_variable **out = create_many_int(nir_var_mem_global, "out", 2);

   nir_def *idx_ssa = nir_load_var(b, idx);

   /* Store to vec. */
   nir_store_var(b, vec, nir_imm_ivec2(b, 10, 10), 1 | 2);

   /* Load from vec[idx]. This load is currently not dropped. */
   nir_deref_instr *indirect =
      nir_build_deref_array(b, nir_build_deref_var(b, vec), idx_ssa);
   nir_store_var(b, out[0], nir_load_deref(b, indirect), 1);

   /* Load from vec[idx] again. This load should be dropped. */
   nir_store_var(b, out[1], nir_load_deref(b, indirect), 1);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 3);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_deref), 3);

   bool progress = nir_opt_copy_prop_vars(b->shader);
   EXPECT_TRUE(progress);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 2);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_deref), 3);

   /* Store to vec[idx] propagated to out. */
   nir_intrinsic_instr *second = get_intrinsic(nir_intrinsic_store_deref, 1);
   nir_intrinsic_instr *third = get_intrinsic(nir_intrinsic_store_deref, 2);
   EXPECT_EQ(second->src[1].ssa, third->src[1].ssa);
}

TEST_F(nir_copy_prop_vars_test, store_load_indirect_array_deref)
{
   nir_variable *arr = create_var(nir_var_mem_global,
                                  glsl_array_type(glsl_int_type(), 10, 0),
                                  "arr");
   nir_variable *idx = create_int(nir_var_mem_global, "idx");
   nir_variable *out = create_int(nir_var_mem_global, "out");

   nir_def *idx_ssa = nir_load_var(b, idx);

   /* Store to arr[idx]. */
   nir_deref_instr *store_deref =
      nir_build_deref_array(b, nir_build_deref_var(b, arr), idx_ssa);
   nir_store_deref(b, store_deref, nir_imm_int(b, 20), 1);

   /* Load from arr[idx] to store in out. This load should be dropped. */
   nir_deref_instr *load_deref =
      nir_build_deref_array(b, nir_build_deref_var(b, arr), idx_ssa);
   nir_store_var(b, out, nir_load_deref(b, load_deref), 1);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 2);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_deref), 2);

   bool progress = nir_opt_copy_prop_vars(b->shader);
   EXPECT_TRUE(progress);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 1);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_deref), 2);

   /* Store to arr[idx] propagated to out. */
   nir_intrinsic_instr *first = get_intrinsic(nir_intrinsic_store_deref, 0);
   nir_intrinsic_instr *second = get_intrinsic(nir_intrinsic_store_deref, 1);
   EXPECT_EQ(first->src[1].ssa, second->src[1].ssa);
}

TEST_F(nir_copy_prop_vars_test, restrict_ssbo_bindings)
{
   glsl_struct_field field = glsl_struct_field();
   field.type = glsl_int_type();
   field.name = "x";
   const glsl_type *ifc_type =
      glsl_interface_type(&field, 1, GLSL_INTERFACE_PACKING_STD430,
                          false /* row_major */, "b");
   nir_variable *ssbo0 = create_var(nir_var_mem_ssbo, ifc_type, "ssbo0");
   nir_variable *ssbo1 = create_var(nir_var_mem_ssbo, ifc_type, "ssbo1");
   ssbo0->data.access = ssbo1->data.access = ACCESS_RESTRICT;
   nir_variable *out = create_var(nir_var_mem_ssbo, ifc_type, "out");
   out->data.access = ACCESS_RESTRICT;

   nir_deref_instr *ssbo0_x =
      nir_build_deref_struct(b, nir_build_deref_var(b, ssbo0), 0);
   nir_store_deref(b, ssbo0_x, nir_imm_int(b, 20), 1);

   nir_deref_instr *ssbo1_x =
      nir_build_deref_struct(b, nir_build_deref_var(b, ssbo1), 0);
   nir_store_deref(b, ssbo1_x, nir_imm_int(b, 30), 1);

   /* Load ssbo0.x and store it in out.x.  This load should be dropped */
   nir_deref_instr *out_x =
      nir_build_deref_struct(b, nir_build_deref_var(b, out), 0);
   nir_store_deref(b, out_x, nir_load_deref(b, ssbo0_x), 1);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 1);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_deref), 3);

   bool progress = nir_opt_copy_prop_vars(b->shader);
   EXPECT_TRUE(progress);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 0);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_deref), 3);

   /* Store to b0.x propagated to out. */
   nir_intrinsic_instr *first = get_intrinsic(nir_intrinsic_store_deref, 0);
   nir_intrinsic_instr *third = get_intrinsic(nir_intrinsic_store_deref, 2);
   EXPECT_EQ(first->src[1].ssa, third->src[1].ssa);
}

TEST_F(nir_copy_prop_vars_test, aliasing_ssbo_bindings)
{
   glsl_struct_field field = glsl_struct_field();
   field.type = glsl_int_type();
   field.name = "x";
   const glsl_type *ifc_type =
      glsl_interface_type(&field, 1, GLSL_INTERFACE_PACKING_STD430,
                          false /* row_major */, "b");
   nir_variable *ssbo0 = create_var(nir_var_mem_ssbo, ifc_type, "ssbo0");
   nir_variable *ssbo1 = create_var(nir_var_mem_ssbo, ifc_type, "ssbo1");
   nir_variable *out = create_var(nir_var_mem_ssbo, ifc_type, "out");
   out->data.access = ACCESS_RESTRICT;

   nir_deref_instr *ssbo0_x =
      nir_build_deref_struct(b, nir_build_deref_var(b, ssbo0), 0);
   nir_store_deref(b, ssbo0_x, nir_imm_int(b, 20), 1);

   nir_deref_instr *ssbo1_x =
      nir_build_deref_struct(b, nir_build_deref_var(b, ssbo1), 0);
   nir_store_deref(b, ssbo1_x, nir_imm_int(b, 30), 1);

   /* Load ssbo0.x and store it in out.x.  This load should not be dropped */
   nir_deref_instr *out_x =
      nir_build_deref_struct(b, nir_build_deref_var(b, out), 0);
   nir_store_deref(b, out_x, nir_load_deref(b, ssbo0_x), 1);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 1);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_deref), 3);

   bool progress = nir_opt_copy_prop_vars(b->shader);
   EXPECT_FALSE(progress);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 1);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_deref), 3);
}

TEST_F(nir_copy_prop_vars_test, ssbo_array_binding_indirect)
{
   glsl_struct_field field = glsl_struct_field();
   field.type = glsl_int_type();
   field.name = "x";
   const glsl_type *ifc_type =
      glsl_interface_type(&field, 1, GLSL_INTERFACE_PACKING_STD430,
                          false /* row_major */, "b");
   const glsl_type *arr_ifc_type = glsl_array_type(ifc_type, 2, 0);
   nir_variable *ssbo_arr = create_var(nir_var_mem_ssbo, arr_ifc_type,
                                       "ssbo_arr");
   ssbo_arr->data.access = ACCESS_RESTRICT;
   nir_variable *out = create_var(nir_var_mem_ssbo, ifc_type, "out");
   out->data.access = ACCESS_RESTRICT;

   nir_deref_instr *ssbo_0 =
      nir_build_deref_array_imm(b, nir_build_deref_var(b, ssbo_arr), 0);
   nir_deref_instr *ssbo_0_x = nir_build_deref_struct(b, ssbo_0, 0);
   nir_store_deref(b, ssbo_0_x, nir_imm_int(b, 20), 1);

   nir_deref_instr *ssbo_i =
      nir_build_deref_array(b, nir_build_deref_var(b, ssbo_arr),
                               nir_load_local_invocation_index(b));
   nir_deref_instr *ssbo_i_x = nir_build_deref_struct(b, ssbo_i, 0);
   nir_store_deref(b, ssbo_i_x, nir_imm_int(b, 30), 1);

   /* Load ssbo_arr[0].x and store it in out.x.  This load should not be dropped */
   nir_deref_instr *out_x =
      nir_build_deref_struct(b, nir_build_deref_var(b, out), 0);
   nir_store_deref(b, out_x, nir_load_deref(b, ssbo_0_x), 1);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 1);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_deref), 3);

   bool progress = nir_opt_copy_prop_vars(b->shader);
   EXPECT_FALSE(progress);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 1);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_deref), 3);
}

TEST_F(nir_copy_prop_vars_test, restrict_ssbo_array_binding)
{
   glsl_struct_field field = glsl_struct_field();
   field.type = glsl_int_type();
   field.name = "x";
   const glsl_type *ifc_type =
      glsl_interface_type(&field, 1, GLSL_INTERFACE_PACKING_STD430,
                          false /* row_major */, "b");
   const glsl_type *arr_ifc_type = glsl_array_type(ifc_type, 2, 0);
   nir_variable *ssbo_arr = create_var(nir_var_mem_ssbo, arr_ifc_type,
                                       "ssbo_arr");
   ssbo_arr->data.access = ACCESS_RESTRICT;
   nir_variable *out = create_var(nir_var_mem_ssbo, ifc_type, "out");
   out->data.access = ACCESS_RESTRICT;

   nir_deref_instr *ssbo_0 =
      nir_build_deref_array_imm(b, nir_build_deref_var(b, ssbo_arr), 0);
   nir_deref_instr *ssbo_0_x = nir_build_deref_struct(b, ssbo_0, 0);
   nir_store_deref(b, ssbo_0_x, nir_imm_int(b, 20), 1);

   nir_deref_instr *ssbo_1 =
      nir_build_deref_array_imm(b, nir_build_deref_var(b, ssbo_arr), 1);
   nir_deref_instr *ssbo_1_x = nir_build_deref_struct(b, ssbo_1, 0);
   nir_store_deref(b, ssbo_1_x, nir_imm_int(b, 30), 1);

   /* Load ssbo_arr[0].x and store it in out.x.  This load should be dropped */
   nir_deref_instr *out_x =
      nir_build_deref_struct(b, nir_build_deref_var(b, out), 0);
   nir_store_deref(b, out_x, nir_load_deref(b, ssbo_0_x), 1);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 1);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_deref), 3);

   bool progress = nir_opt_copy_prop_vars(b->shader);
   EXPECT_TRUE(progress);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 0);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_deref), 3);

   /* Store to b0.x propagated to out. */
   nir_intrinsic_instr *first = get_intrinsic(nir_intrinsic_store_deref, 0);
   nir_intrinsic_instr *third = get_intrinsic(nir_intrinsic_store_deref, 2);
   EXPECT_EQ(first->src[1].ssa, third->src[1].ssa);
}

TEST_F(nir_copy_prop_vars_test, aliasing_ssbo_array_binding)
{
   glsl_struct_field field = glsl_struct_field();
   field.type = glsl_int_type();
   field.name = "x";
   const glsl_type *ifc_type =
      glsl_interface_type(&field, 1, GLSL_INTERFACE_PACKING_STD430,
                          false /* row_major */, "b");
   const glsl_type *arr_ifc_type = glsl_array_type(ifc_type, 2, 0);
   nir_variable *ssbo_arr = create_var(nir_var_mem_ssbo, arr_ifc_type,
                                       "ssbo_arr");
   nir_variable *out = create_var(nir_var_mem_ssbo, ifc_type, "out");
   out->data.access = ACCESS_RESTRICT;

   nir_deref_instr *ssbo_0 =
      nir_build_deref_array_imm(b, nir_build_deref_var(b, ssbo_arr), 0);
   nir_deref_instr *ssbo_0_x = nir_build_deref_struct(b, ssbo_0, 0);
   nir_store_deref(b, ssbo_0_x, nir_imm_int(b, 20), 1);

   nir_deref_instr *ssbo_1 =
      nir_build_deref_array_imm(b, nir_build_deref_var(b, ssbo_arr), 1);
   nir_deref_instr *ssbo_1_x = nir_build_deref_struct(b, ssbo_1, 0);
   nir_store_deref(b, ssbo_1_x, nir_imm_int(b, 30), 1);

   /* Load ssbo_arr[0].x and store it in out.x.  This load should not be dropped */
   nir_deref_instr *out_x =
      nir_build_deref_struct(b, nir_build_deref_var(b, out), 0);
   nir_store_deref(b, out_x, nir_load_deref(b, ssbo_0_x), 1);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 1);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_deref), 3);

   bool progress = nir_opt_copy_prop_vars(b->shader);
   EXPECT_FALSE(progress);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_load_deref), 1);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_deref), 3);
}

TEST_F(nir_dead_write_vars_test, no_dead_writes_in_block)
{
   nir_variable **v = create_many_int(nir_var_mem_global, "v", 2);

   nir_store_var(b, v[0], nir_load_var(b, v[1]), 1);

   bool progress = nir_opt_dead_write_vars(b->shader);
   ASSERT_FALSE(progress);
}

TEST_F(nir_dead_write_vars_test, no_dead_writes_different_components_in_block)
{
   nir_variable **v = create_many_ivec2(nir_var_mem_global, "v", 3);

   nir_store_var(b, v[0], nir_load_var(b, v[1]), 1 << 0);
   nir_store_var(b, v[0], nir_load_var(b, v[2]), 1 << 1);

   bool progress = nir_opt_dead_write_vars(b->shader);
   ASSERT_FALSE(progress);
}

TEST_F(nir_dead_write_vars_test, volatile_write)
{
   nir_variable *v = create_int(nir_var_mem_global, "v");

   nir_store_var(b, v, nir_imm_int(b, 0), 0x1);
   nir_store_var_volatile(b, v, nir_imm_int(b, 1), 0x1);
   nir_store_var(b, v, nir_imm_int(b, 2), 0x1);

   /* Our approach here is a bit scorched-earth.  We expect the volatile store
    * in the middle to cause both that store and the one before it to be kept.
    * Technically, volatile only prevents combining the volatile store with
    * another store and one could argue that the store before the volatile and
    * the one after it could be combined.  However, it seems safer to just
    * treat a volatile store like an atomic and prevent any combining across
    * it.
    */
   bool progress = nir_opt_dead_write_vars(b->shader);
   ASSERT_FALSE(progress);
}

TEST_F(nir_dead_write_vars_test, volatile_copies)
{
   nir_variable **v = create_many_int(nir_var_mem_global, "v", 2);

   nir_copy_var(b, v[0], v[1]);
   nir_copy_deref_with_access(b, nir_build_deref_var(b, v[0]),
                                 nir_build_deref_var(b, v[1]),
                                 ACCESS_VOLATILE, (gl_access_qualifier)0);
   nir_copy_var(b, v[0], v[1]);

   /* Our approach here is a bit scorched-earth.  We expect the volatile store
    * in the middle to cause both that store and the one before it to be kept.
    * Technically, volatile only prevents combining the volatile store with
    * another store and one could argue that the store before the volatile and
    * the one after it could be combined.  However, it seems safer to just
    * treat a volatile store like an atomic and prevent any combining across
    * it.
    */
   bool progress = nir_opt_dead_write_vars(b->shader);
   ASSERT_FALSE(progress);
}

TEST_F(nir_dead_write_vars_test, no_dead_writes_in_if_statement)
{
   nir_variable **v = create_many_int(nir_var_mem_global, "v", 6);

   nir_store_var(b, v[2], nir_load_var(b, v[0]), 1);
   nir_store_var(b, v[3], nir_load_var(b, v[1]), 1);

   /* Each arm of the if statement will overwrite one store. */
   nir_if *if_stmt = nir_push_if(b, nir_imm_int(b, 0));
   nir_store_var(b, v[2], nir_load_var(b, v[4]), 1);

   nir_push_else(b, if_stmt);
   nir_store_var(b, v[3], nir_load_var(b, v[5]), 1);

   nir_pop_if(b, if_stmt);

   bool progress = nir_opt_dead_write_vars(b->shader);
   ASSERT_FALSE(progress);
}

TEST_F(nir_dead_write_vars_test, no_dead_writes_in_loop_statement)
{
   nir_variable **v = create_many_int(nir_var_mem_global, "v", 3);

   nir_store_var(b, v[0], nir_load_var(b, v[1]), 1);

   /* Loop will write other value.  Since it might not be executed, it doesn't
    * kill the first write.
    */
   nir_loop *loop = nir_push_loop(b);

   nir_if *if_stmt = nir_push_if(b, nir_imm_int(b, 0));
   nir_jump(b, nir_jump_break);
   nir_pop_if(b, if_stmt);

   nir_store_var(b, v[0], nir_load_var(b, v[2]), 1);
   nir_pop_loop(b, loop);

   bool progress = nir_opt_dead_write_vars(b->shader);
   ASSERT_FALSE(progress);
}

TEST_F(nir_dead_write_vars_test, dead_write_in_block)
{
   nir_variable **v = create_many_int(nir_var_mem_global, "v", 3);

   nir_store_var(b, v[0], nir_load_var(b, v[1]), 1);
   nir_def *load_v2 = nir_load_var(b, v[2]);
   nir_store_var(b, v[0], load_v2, 1);

   bool progress = nir_opt_dead_write_vars(b->shader);
   ASSERT_TRUE(progress);

   EXPECT_EQ(1, count_intrinsics(nir_intrinsic_store_deref));

   nir_intrinsic_instr *store = get_intrinsic(nir_intrinsic_store_deref, 0);
   EXPECT_EQ(store->src[1].ssa, load_v2);
}

TEST_F(nir_dead_write_vars_test, dead_write_components_in_block)
{
   nir_variable **v = create_many_ivec2(nir_var_mem_global, "v", 3);

   nir_store_var(b, v[0], nir_load_var(b, v[1]), 1 << 0);
   nir_def *load_v2 = nir_load_var(b, v[2]);
   nir_store_var(b, v[0], load_v2, 1 << 0);

   bool progress = nir_opt_dead_write_vars(b->shader);
   ASSERT_TRUE(progress);

   EXPECT_EQ(1, count_intrinsics(nir_intrinsic_store_deref));

   nir_intrinsic_instr *store = get_intrinsic(nir_intrinsic_store_deref, 0);
   EXPECT_EQ(store->src[1].ssa, load_v2);
}


/* TODO: The DISABLED tests below depend on the dead write removal be able to
 * identify dead writes between multiple blocks.  This is still not
 * implemented.
 */

TEST_F(nir_dead_write_vars_test, DISABLED_dead_write_in_two_blocks)
{
   nir_variable **v = create_many_int(nir_var_mem_global, "v", 3);

   nir_store_var(b, v[0], nir_load_var(b, v[1]), 1);
   nir_def *load_v2 = nir_load_var(b, v[2]);

   /* Causes the stores to be in different blocks. */
   nir_pop_if(b, nir_push_if(b, nir_imm_int(b, 0)));

   nir_store_var(b, v[0], load_v2, 1);

   bool progress = nir_opt_dead_write_vars(b->shader);
   ASSERT_TRUE(progress);

   EXPECT_EQ(1, count_intrinsics(nir_intrinsic_store_deref));

   nir_intrinsic_instr *store = get_intrinsic(nir_intrinsic_store_deref, 0);
   EXPECT_EQ(store->src[1].ssa, load_v2);
}

TEST_F(nir_dead_write_vars_test, DISABLED_dead_write_components_in_two_blocks)
{
   nir_variable **v = create_many_ivec2(nir_var_mem_global, "v", 3);

   nir_store_var(b, v[0], nir_load_var(b, v[1]), 1 << 0);

   /* Causes the stores to be in different blocks. */
   nir_pop_if(b, nir_push_if(b, nir_imm_int(b, 0)));

   nir_def *load_v2 = nir_load_var(b, v[2]);
   nir_store_var(b, v[0], load_v2, 1 << 0);

   bool progress = nir_opt_dead_write_vars(b->shader);
   ASSERT_TRUE(progress);

   EXPECT_EQ(1, count_intrinsics(nir_intrinsic_store_deref));

   nir_intrinsic_instr *store = get_intrinsic(nir_intrinsic_store_deref, 0);
   EXPECT_EQ(store->src[1].ssa, load_v2);
}

TEST_F(nir_dead_write_vars_test, DISABLED_dead_writes_in_if_statement)
{
   nir_variable **v = create_many_int(nir_var_mem_global, "v", 4);

   /* Both branches will overwrite, making the previous store dead. */
   nir_store_var(b, v[0], nir_load_var(b, v[1]), 1);

   nir_if *if_stmt = nir_push_if(b, nir_imm_int(b, 0));
   nir_def *load_v2 = nir_load_var(b, v[2]);
   nir_store_var(b, v[0], load_v2, 1);

   nir_push_else(b, if_stmt);
   nir_def *load_v3 = nir_load_var(b, v[3]);
   nir_store_var(b, v[0], load_v3, 1);

   nir_pop_if(b, if_stmt);

   bool progress = nir_opt_dead_write_vars(b->shader);
   ASSERT_TRUE(progress);
   EXPECT_EQ(2, count_intrinsics(nir_intrinsic_store_deref));

   nir_intrinsic_instr *first_store = get_intrinsic(nir_intrinsic_store_deref, 0);
   EXPECT_EQ(first_store->src[1].ssa, load_v2);

   nir_intrinsic_instr *second_store = get_intrinsic(nir_intrinsic_store_deref, 1);
   EXPECT_EQ(second_store->src[1].ssa, load_v3);
}

TEST_F(nir_dead_write_vars_test, DISABLED_memory_barrier_in_two_blocks)
{
   nir_variable **v = create_many_int(nir_var_mem_global, "v", 2);

   nir_store_var(b, v[0], nir_imm_int(b, 1), 1);
   nir_store_var(b, v[1], nir_imm_int(b, 2), 1);

   /* Split into many blocks. */
   nir_pop_if(b, nir_push_if(b, nir_imm_int(b, 0)));

   /* Because it is before the barrier, this will kill the previous store to that target. */
   nir_store_var(b, v[0], nir_imm_int(b, 3), 1);

   nir_scoped_memory_barrier(b, SCOPE_DEVICE, NIR_MEMORY_ACQ_REL,
                             nir_var_mem_global);

   nir_store_var(b, v[1], nir_imm_int(b, 4), 1);

   bool progress = nir_opt_dead_write_vars(b->shader);
   ASSERT_TRUE(progress);

   EXPECT_EQ(3, count_intrinsics(nir_intrinsic_store_deref));
}

TEST_F(nir_dead_write_vars_test, DISABLED_unrelated_barrier_in_two_blocks)
{
   nir_variable **v = create_many_int(nir_var_mem_global, "v", 3);
   nir_variable *out = create_int(nir_var_shader_out, "out");

   nir_store_var(b, out, nir_load_var(b, v[1]), 1);
   nir_store_var(b, v[0], nir_load_var(b, v[1]), 1);

   /* Split into many blocks. */
   nir_pop_if(b, nir_push_if(b, nir_imm_int(b, 0)));

   /* Emit vertex will ensure writes to output variables are considered used,
    * but should not affect other types of variables. */

   nir_emit_vertex(b);

   nir_store_var(b, out, nir_load_var(b, v[2]), 1);
   nir_store_var(b, v[0], nir_load_var(b, v[2]), 1);

   bool progress = nir_opt_dead_write_vars(b->shader);
   ASSERT_TRUE(progress);

   /* Verify the first write to v[0] was removed. */
   EXPECT_EQ(3, count_intrinsics(nir_intrinsic_store_deref));

   nir_intrinsic_instr *first_store = get_intrinsic(nir_intrinsic_store_deref, 0);
   EXPECT_EQ(nir_intrinsic_get_var(first_store, 0), out);

   nir_intrinsic_instr *second_store = get_intrinsic(nir_intrinsic_store_deref, 1);
   EXPECT_EQ(nir_intrinsic_get_var(second_store, 0), out);

   nir_intrinsic_instr *third_store = get_intrinsic(nir_intrinsic_store_deref, 2);
   EXPECT_EQ(nir_intrinsic_get_var(third_store, 0), v[0]);
}

TEST_F(nir_combine_stores_test, non_overlapping_stores)
{
   nir_variable **v = create_many_ivec4(nir_var_mem_global, "v", 4);
   nir_variable *out = create_ivec4(nir_var_shader_out, "out");

   for (int i = 0; i < 4; i++)
      nir_store_var(b, out, nir_load_var(b, v[i]), 1 << i);

   nir_validate_shader(b->shader, NULL);

   bool progress = nir_opt_combine_stores(b->shader, nir_var_shader_out);
   ASSERT_TRUE(progress);

   nir_validate_shader(b->shader, NULL);

   /* Clean up to verify from where the values in combined store are coming. */
   nir_copy_prop(b->shader);
   nir_opt_dce(b->shader);

   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_deref), 1);
   nir_intrinsic_instr *combined = get_intrinsic(nir_intrinsic_store_deref, 0);
   ASSERT_EQ(nir_intrinsic_write_mask(combined), 0xf);
   ASSERT_EQ(nir_intrinsic_get_var(combined, 0), out);

   nir_alu_instr *vec = nir_src_as_alu_instr(combined->src[1]);
   ASSERT_TRUE(vec);
   for (int i = 0; i < 4; i++) {
      nir_intrinsic_instr *load = nir_src_as_intrinsic(vec->src[i].src);
      ASSERT_EQ(load->intrinsic, nir_intrinsic_load_deref);
      ASSERT_EQ(nir_intrinsic_get_var(load, 0), v[i])
         << "Source value for component " << i << " of store is wrong";
      ASSERT_EQ(vec->src[i].swizzle[0], i)
         << "Source component for component " << i << " of store is wrong";
   }
}

TEST_F(nir_combine_stores_test, overlapping_stores)
{
   nir_variable **v = create_many_ivec4(nir_var_mem_global, "v", 3);
   nir_variable *out = create_ivec4(nir_var_shader_out, "out");

   /* Make stores with xy, yz and zw masks. */
   for (int i = 0; i < 3; i++) {
      nir_component_mask_t mask = (1 << i) | (1 << (i + 1));
      nir_store_var(b, out, nir_load_var(b, v[i]), mask);
   }

   nir_validate_shader(b->shader, NULL);

   bool progress = nir_opt_combine_stores(b->shader, nir_var_shader_out);
   ASSERT_TRUE(progress);

   nir_validate_shader(b->shader, NULL);

   /* Clean up to verify from where the values in combined store are coming. */
   nir_copy_prop(b->shader);
   nir_opt_dce(b->shader);

   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_deref), 1);
   nir_intrinsic_instr *combined = get_intrinsic(nir_intrinsic_store_deref, 0);
   ASSERT_EQ(nir_intrinsic_write_mask(combined), 0xf);
   ASSERT_EQ(nir_intrinsic_get_var(combined, 0), out);

   nir_alu_instr *vec = nir_src_as_alu_instr(combined->src[1]);
   ASSERT_TRUE(vec);

   /* Component x comes from v[0]. */
   nir_intrinsic_instr *load_for_x = nir_src_as_intrinsic(vec->src[0].src);
   ASSERT_EQ(nir_intrinsic_get_var(load_for_x, 0), v[0]);
   ASSERT_EQ(vec->src[0].swizzle[0], 0);

   /* Component y comes from v[1]. */
   nir_intrinsic_instr *load_for_y = nir_src_as_intrinsic(vec->src[1].src);
   ASSERT_EQ(nir_intrinsic_get_var(load_for_y, 0), v[1]);
   ASSERT_EQ(vec->src[1].swizzle[0], 1);

   /* Components z and w come from v[2]. */
   nir_intrinsic_instr *load_for_z = nir_src_as_intrinsic(vec->src[2].src);
   nir_intrinsic_instr *load_for_w = nir_src_as_intrinsic(vec->src[3].src);
   ASSERT_EQ(load_for_z, load_for_w);
   ASSERT_EQ(nir_intrinsic_get_var(load_for_z, 0), v[2]);
   ASSERT_EQ(vec->src[2].swizzle[0], 2);
   ASSERT_EQ(vec->src[3].swizzle[0], 3);
}

TEST_F(nir_combine_stores_test, direct_array_derefs)
{
   nir_variable **v = create_many_ivec4(nir_var_mem_global, "vec", 2);
   nir_variable **s = create_many_int(nir_var_mem_global, "scalar", 2);
   nir_variable *out = create_ivec4(nir_var_mem_global, "out");

   nir_deref_instr *out_deref = nir_build_deref_var(b, out);

   /* Store to vector with mask x. */
   nir_store_deref(b, out_deref, nir_load_var(b, v[0]),
                   1 << 0);

   /* Store to vector with mask yz. */
   nir_store_deref(b, out_deref, nir_load_var(b, v[1]),
                   (1 << 2) | (1 << 1));

   /* Store to vector[2], overlapping with previous store. */
   nir_store_deref(b,
                   nir_build_deref_array_imm(b, out_deref, 2),
                   nir_load_var(b, s[0]),
                   1 << 0);

   /* Store to vector[3], no overlap. */
   nir_store_deref(b,
                   nir_build_deref_array_imm(b, out_deref, 3),
                   nir_load_var(b, s[1]),
                   1 << 0);

   nir_validate_shader(b->shader, NULL);

   bool progress = nir_opt_combine_stores(b->shader, nir_var_mem_global);
   ASSERT_TRUE(progress);

   nir_validate_shader(b->shader, NULL);

   /* Clean up to verify from where the values in combined store are coming. */
   nir_copy_prop(b->shader);
   nir_opt_dce(b->shader);

   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_deref), 1);
   nir_intrinsic_instr *combined = get_intrinsic(nir_intrinsic_store_deref, 0);
   ASSERT_EQ(nir_intrinsic_write_mask(combined), 0xf);
   ASSERT_EQ(nir_intrinsic_get_var(combined, 0), out);

   nir_alu_instr *vec = nir_src_as_alu_instr(combined->src[1]);
   ASSERT_TRUE(vec);

   /* Component x comes from v[0]. */
   nir_intrinsic_instr *load_for_x = nir_src_as_intrinsic(vec->src[0].src);
   ASSERT_EQ(nir_intrinsic_get_var(load_for_x, 0), v[0]);
   ASSERT_EQ(vec->src[0].swizzle[0], 0);

   /* Component y comes from v[1]. */
   nir_intrinsic_instr *load_for_y = nir_src_as_intrinsic(vec->src[1].src);
   ASSERT_EQ(nir_intrinsic_get_var(load_for_y, 0), v[1]);
   ASSERT_EQ(vec->src[1].swizzle[0], 1);

   /* Components z comes from s[0]. */
   nir_intrinsic_instr *load_for_z = nir_src_as_intrinsic(vec->src[2].src);
   ASSERT_EQ(nir_intrinsic_get_var(load_for_z, 0), s[0]);
   ASSERT_EQ(vec->src[2].swizzle[0], 0);

   /* Component w comes from s[1]. */
   nir_intrinsic_instr *load_for_w = nir_src_as_intrinsic(vec->src[3].src);
   ASSERT_EQ(nir_intrinsic_get_var(load_for_w, 0), s[1]);
   ASSERT_EQ(vec->src[3].swizzle[0], 0);
}

static int64_t
vec_src_comp_as_int(nir_src src, unsigned comp)
{
   if (nir_src_is_const(src))
      return nir_src_comp_as_int(src, comp);

   nir_scalar s = { src.ssa, comp };
   assert(nir_op_is_vec_or_mov(nir_scalar_alu_op(s)));
   return nir_scalar_as_int(nir_scalar_chase_alu_src(s, comp));
}

TEST_F(nir_combine_stores_test, store_volatile)
{
   nir_variable *out = create_ivec4(nir_var_shader_out, "out");

   nir_store_var(b, out, nir_imm_ivec4(b, 0, 0, 0, 0), 1 << 0);
   nir_store_var(b, out, nir_imm_ivec4(b, 1, 1, 1, 1), 1 << 1);
   nir_store_var_volatile(b, out, nir_imm_ivec4(b, -1, -2, -3, -4), 0xf);
   nir_store_var(b, out, nir_imm_ivec4(b, 2, 2, 2, 2), 1 << 2);
   nir_store_var(b, out, nir_imm_ivec4(b, 3, 3, 3, 3), 1 << 3);

   nir_validate_shader(b->shader, NULL);

   bool progress = nir_opt_combine_stores(b->shader, nir_var_shader_out);
   ASSERT_TRUE(progress);

   nir_validate_shader(b->shader, NULL);

   /* Clean up the stored values */
   nir_opt_constant_folding(b->shader);
   nir_opt_dce(b->shader);

   ASSERT_EQ(count_intrinsics(nir_intrinsic_store_deref), 3);

   nir_intrinsic_instr *first = get_intrinsic(nir_intrinsic_store_deref, 0);
   ASSERT_EQ(nir_intrinsic_write_mask(first), 0x3);
   ASSERT_EQ(vec_src_comp_as_int(first->src[1], 0), 0);
   ASSERT_EQ(vec_src_comp_as_int(first->src[1], 1), 1);

   nir_intrinsic_instr *second = get_intrinsic(nir_intrinsic_store_deref, 1);
   ASSERT_EQ(nir_intrinsic_write_mask(second), 0xf);
   ASSERT_EQ(vec_src_comp_as_int(second->src[1], 0), -1);
   ASSERT_EQ(vec_src_comp_as_int(second->src[1], 1), -2);
   ASSERT_EQ(vec_src_comp_as_int(second->src[1], 2), -3);
   ASSERT_EQ(vec_src_comp_as_int(second->src[1], 3), -4);

   nir_intrinsic_instr *third = get_intrinsic(nir_intrinsic_store_deref, 2);
   ASSERT_EQ(nir_intrinsic_write_mask(third), 0xc);
   ASSERT_EQ(vec_src_comp_as_int(third->src[1], 2), 2);
   ASSERT_EQ(vec_src_comp_as_int(third->src[1], 3), 3);
}

TEST_F(nir_split_vars_test, simple_split)
{
   nir_variable **in = create_many_int(nir_var_shader_in, "in", 4);
   nir_variable *temp = create_var(nir_var_function_temp, glsl_array_type(glsl_int_type(), 4, 0),
                                   "temp");
   nir_deref_instr *temp_deref = nir_build_deref_var(b, temp);
   for (int i = 0; i < 4; i++)
      nir_store_deref(b, nir_build_deref_array_imm(b, temp_deref, i), nir_load_var(b, in[i]), 1);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_derefs(nir_deref_type_array), 4);
   ASSERT_EQ(count_function_temp_vars(), 1);

   bool progress = nir_split_array_vars(b->shader, nir_var_function_temp);
   EXPECT_TRUE(progress);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_derefs(nir_deref_type_array), 0);
   ASSERT_EQ(count_function_temp_vars(), 4);
}

TEST_F(nir_split_vars_test, simple_no_split_array_struct)
{
   nir_variable **in = create_many_int(nir_var_shader_in, "in", 4);
   struct glsl_struct_field field;

   field.type = glsl_float_type();
   field.name = ralloc_asprintf(b->shader, "field1");
   field.location = -1;
   field.offset = 0;

   const struct glsl_type *st_type = glsl_struct_type(&field, 1, "struct", false);
   nir_variable *temp = create_var(nir_var_function_temp, glsl_array_type(st_type, 4, 0),
                                   "temp");

   nir_variable *temp2 = create_var(nir_var_function_temp, glsl_array_type(glsl_int_type(), 4, 0), "temp2");

   nir_deref_instr *temp_deref = nir_build_deref_var(b, temp);
   nir_deref_instr *temp2_deref = nir_build_deref_var(b, temp2);
   for (int i = 0; i < 4; i++)
      nir_store_deref(b, nir_build_deref_array_imm(b, temp2_deref, i), nir_load_var(b, in[i]), 1);

   for (int i = 0; i < 4; i++)
      nir_store_deref(b, nir_build_deref_struct(b, nir_build_deref_array_imm(b, temp_deref, i), 0), nir_load_var(b, in[i]), 1);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_derefs(nir_deref_type_array), 8);
   ASSERT_EQ(count_derefs(nir_deref_type_struct), 4);
   ASSERT_EQ(count_function_temp_vars(), 2);

   bool progress = nir_split_array_vars(b->shader, nir_var_function_temp);
   EXPECT_TRUE(progress);

   nir_validate_shader(b->shader, NULL);

   ASSERT_EQ(count_derefs(nir_deref_type_array), 4);
   ASSERT_EQ(count_derefs(nir_deref_type_struct), 4);
   for (int i = 0; i < 4; i++) {
      nir_deref_instr *deref = get_deref(nir_deref_type_array, i);
      ASSERT_TRUE(deref);
      ASSERT_TRUE(glsl_type_is_struct(deref->type));
   }

   ASSERT_EQ(count_function_temp_vars(), 5);
}

TEST_F(nir_split_vars_test, simple_split_shader_temp)
{
   nir_variable **in = create_many_int(nir_var_shader_in, "in", 4);
   nir_variable *temp = create_var(nir_var_shader_temp, glsl_array_type(glsl_int_type(), 4, 0),
                                   "temp");
   nir_deref_instr *temp_deref = nir_build_deref_var(b, temp);

   for (int i = 0; i < 4; i++)
      nir_store_deref(b, nir_build_deref_array_imm(b, temp_deref, i), nir_load_var(b, in[i]), 1);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_derefs(nir_deref_type_array), 4);
   ASSERT_EQ(count_shader_temp_vars(), 1);

   bool progress = nir_split_array_vars(b->shader, nir_var_shader_temp);
   EXPECT_TRUE(progress);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_derefs(nir_deref_type_array), 0);
   ASSERT_EQ(count_shader_temp_vars(), 4);
}

TEST_F(nir_split_vars_test, simple_oob)
{
   nir_variable **in = create_many_int(nir_var_shader_in, "in", 6);
   nir_variable *temp = create_var(nir_var_function_temp, glsl_array_type(glsl_int_type(), 4, 0),
                                   "temp");
   nir_deref_instr *temp_deref = nir_build_deref_var(b, temp);

   for (int i = 0; i < 6; i++)
      nir_store_deref(b, nir_build_deref_array_imm(b, temp_deref, i), nir_load_var(b, in[i]), 1);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_derefs(nir_deref_type_array), 6);
   ASSERT_EQ(count_function_temp_vars(), 1);

   bool progress = nir_split_array_vars(b->shader, nir_var_function_temp);
   EXPECT_TRUE(progress);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_derefs(nir_deref_type_array), 0);
   ASSERT_EQ(count_function_temp_vars(), 4);
}

TEST_F(nir_split_vars_test, simple_unused)
{
   nir_variable **in = create_many_int(nir_var_shader_in, "in", 2);
   nir_variable *temp = create_var(nir_var_function_temp, glsl_array_type(glsl_int_type(), 4, 0),
                                   "temp");
   nir_deref_instr *temp_deref = nir_build_deref_var(b, temp);

   for (int i = 0; i < 2; i++)
      nir_store_deref(b, nir_build_deref_array_imm(b, temp_deref, i), nir_load_var(b, in[i]), 1);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_derefs(nir_deref_type_array), 2);
   ASSERT_EQ(count_function_temp_vars(), 1);

   bool progress = nir_split_array_vars(b->shader, nir_var_function_temp);
   EXPECT_TRUE(progress);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_derefs(nir_deref_type_array), 0);
   /* this pass doesn't remove the unused ones */
   ASSERT_EQ(count_function_temp_vars(), 4);
}

TEST_F(nir_split_vars_test, two_level_split)
{
   nir_variable **in = create_many_int(nir_var_shader_in, "in", 4);
   nir_variable *temp = create_var(nir_var_function_temp, glsl_array_type(glsl_array_type(glsl_int_type(), 4, 0), 4, 0),
                                   "temp");
   nir_deref_instr *temp_deref = nir_build_deref_var(b, temp);
   for (int i = 0; i < 4; i++) {
      nir_deref_instr *level0 = nir_build_deref_array_imm(b, temp_deref, i);
      for (int j = 0; j < 4; j++) {
         nir_deref_instr *level1 = nir_build_deref_array_imm(b, level0, j);
         nir_store_deref(b, level1, nir_load_var(b, in[i]), 1);
      }
   }

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_derefs(nir_deref_type_array), 20);
   ASSERT_EQ(count_function_temp_vars(), 1);

   bool progress = nir_split_array_vars(b->shader, nir_var_function_temp);
   EXPECT_TRUE(progress);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_derefs(nir_deref_type_array), 0);
   ASSERT_EQ(count_function_temp_vars(), 16);
}

TEST_F(nir_split_vars_test, simple_dont_split)
{
   nir_variable **in = create_many_int(nir_var_shader_in, "in", 4);
   nir_variable *temp = create_var(nir_var_function_temp, glsl_array_type(glsl_int_type(), 4, 0),
                                   "temp");
   nir_variable *ind = create_int(nir_var_shader_in, "ind");

   nir_deref_instr *ind_deref = nir_build_deref_var(b, ind);
   nir_deref_instr *temp_deref = nir_build_deref_var(b, temp);

   for (int i = 0; i < 4; i++)
      nir_store_deref(b, nir_build_deref_array(b, temp_deref, &ind_deref->def), nir_load_var(b, in[i]), 1);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_derefs(nir_deref_type_array), 4);
   ASSERT_EQ(count_function_temp_vars(), 1);

   bool progress = nir_split_array_vars(b->shader, nir_var_function_temp);
   EXPECT_FALSE(progress);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_derefs(nir_deref_type_array), 4);
   ASSERT_EQ(count_function_temp_vars(), 1);
}

TEST_F(nir_split_vars_test, twolevel_dont_split_lvl_0)
{
   nir_variable **in = create_many_int(nir_var_shader_in, "in", 4);
   nir_variable *temp = create_var(nir_var_function_temp, glsl_array_type(glsl_array_type(glsl_int_type(), 6, 0), 4, 0),
                                   "temp");
   nir_variable *ind = create_int(nir_var_shader_in, "ind");

   nir_deref_instr *ind_deref = nir_build_deref_var(b, ind);
   nir_deref_instr *temp_deref = nir_build_deref_var(b, temp);

   for (int i = 0; i < 4; i++) {
      nir_deref_instr *level0 = nir_build_deref_array(b, temp_deref, &ind_deref->def);
      for (int j = 0; j < 6; j++) {
         nir_deref_instr *level1 = nir_build_deref_array_imm(b, level0, j);
         nir_store_deref(b, level1, nir_load_var(b, in[i]), 1);
      }
   }

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_derefs(nir_deref_type_array), 28);
   ASSERT_EQ(count_function_temp_vars(), 1);

   bool progress = nir_split_array_vars(b->shader, nir_var_function_temp);
   EXPECT_TRUE(progress);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_derefs(nir_deref_type_array), 24);
   ASSERT_EQ(count_function_temp_vars(), 6);
}

TEST_F(nir_split_vars_test, twolevel_dont_split_lvl_1)
{
   nir_variable **in = create_many_int(nir_var_shader_in, "in", 6);
   nir_variable *temp = create_var(nir_var_function_temp, glsl_array_type(glsl_array_type(glsl_int_type(), 6, 0), 4, 0),
                                   "temp");
   nir_variable *ind = create_int(nir_var_shader_in, "ind");

   nir_deref_instr *ind_deref = nir_build_deref_var(b, ind);
   nir_deref_instr *temp_deref = nir_build_deref_var(b, temp);

   for (int i = 0; i < 4; i++) {
      nir_deref_instr *level0 = nir_build_deref_array_imm(b, temp_deref, i);
      for (int j = 0; j < 6; j++) {
         /* just add the inner index to get some different derefs */
         nir_deref_instr *level1 = nir_build_deref_array(b, level0, nir_iadd_imm(b, &ind_deref->def, j));
         nir_store_deref(b, level1, nir_load_var(b, in[i]), 1);
      }
   }

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_derefs(nir_deref_type_array), 28);
   ASSERT_EQ(count_function_temp_vars(), 1);

   bool progress = nir_split_array_vars(b->shader, nir_var_function_temp);
   EXPECT_TRUE(progress);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_derefs(nir_deref_type_array), 24);
   ASSERT_EQ(count_function_temp_vars(), 4);
}

TEST_F(nir_split_vars_test, split_multiple_store)
{
   nir_variable **in = create_many_int(nir_var_shader_in, "in", 4);
   nir_variable *temp = create_var(nir_var_function_temp, glsl_array_type(glsl_int_type(), 4, 0),
                                   "temp");
   nir_variable *temp2 = create_var(nir_var_function_temp, glsl_array_type(glsl_int_type(), 4, 0),
                                    "temp2");

   nir_deref_instr *temp_deref = nir_build_deref_var(b, temp);
   nir_deref_instr *temp2_deref = nir_build_deref_var(b, temp2);

   for (int i = 0; i < 4; i++)
      nir_store_deref(b, nir_build_deref_array_imm(b, temp_deref, i), nir_load_var(b, in[i]), 1);

   for (int i = 0; i < 4; i++)
      nir_store_deref(b, nir_build_deref_array_imm(b, temp2_deref, i), nir_load_var(b, in[i]), 1);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_derefs(nir_deref_type_array), 8);
   ASSERT_EQ(count_function_temp_vars(), 2);

   bool progress = nir_split_array_vars(b->shader, nir_var_function_temp);
   EXPECT_TRUE(progress);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_derefs(nir_deref_type_array), 0);
   ASSERT_EQ(count_function_temp_vars(), 8);
}

TEST_F(nir_split_vars_test, split_load_store)
{
   nir_variable **in = create_many_int(nir_var_shader_in, "in", 4);
   nir_variable *temp = create_var(nir_var_function_temp, glsl_array_type(glsl_int_type(), 4, 0),
                                   "temp");
   nir_variable *temp2 = create_var(nir_var_function_temp, glsl_array_type(glsl_int_type(), 4, 0),
                                    "temp2");

   nir_deref_instr *temp_deref = nir_build_deref_var(b, temp);
   nir_deref_instr *temp2_deref = nir_build_deref_var(b, temp2);

   for (int i = 0; i < 4; i++)
      nir_store_deref(b, nir_build_deref_array_imm(b, temp_deref, i), nir_load_var(b, in[i]), 1);

   for (int i = 0; i < 4; i++) {
      nir_deref_instr *store_deref = nir_build_deref_array_imm(b, temp2_deref, i);
      nir_deref_instr *load_deref = nir_build_deref_array_imm(b, temp_deref, i);
      nir_store_deref(b, store_deref, nir_load_deref(b, load_deref), 1);
   }

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_derefs(nir_deref_type_array), 12);
   ASSERT_EQ(count_function_temp_vars(), 2);

   bool progress = nir_split_array_vars(b->shader, nir_var_function_temp);
   EXPECT_TRUE(progress);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_derefs(nir_deref_type_array), 0);
   ASSERT_EQ(count_function_temp_vars(), 8);
}

TEST_F(nir_split_vars_test, split_copy)
{
   nir_variable **in = create_many_int(nir_var_shader_in, "in", 4);
   nir_variable *temp = create_var(nir_var_function_temp, glsl_array_type(glsl_int_type(), 4, 0),
                                   "temp");
   nir_variable *temp2 = create_var(nir_var_function_temp, glsl_array_type(glsl_int_type(), 4, 0),
                                    "temp2");

   nir_deref_instr *temp_deref = nir_build_deref_var(b, temp);
   nir_deref_instr *temp2_deref = nir_build_deref_var(b, temp2);

   for (int i = 0; i < 4; i++)
      nir_store_deref(b, nir_build_deref_array_imm(b, temp_deref, i), nir_load_var(b, in[i]), 1);

   for (int i = 0; i < 4; i++) {
      nir_deref_instr *store_deref = nir_build_deref_array_imm(b, temp2_deref, i);
      nir_deref_instr *load_deref = nir_build_deref_array_imm(b, temp_deref, i);
      nir_copy_deref(b, store_deref, load_deref);
   }

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_derefs(nir_deref_type_array), 12);
   ASSERT_EQ(count_function_temp_vars(), 2);

   bool progress = nir_split_array_vars(b->shader, nir_var_function_temp);
   EXPECT_TRUE(progress);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_derefs(nir_deref_type_array), 0);
   ASSERT_EQ(count_function_temp_vars(), 8);
}

TEST_F(nir_split_vars_test, split_wildcard_copy)
{
   nir_variable **in = create_many_int(nir_var_shader_in, "in", 4);
   nir_variable *temp = create_var(nir_var_function_temp, glsl_array_type(glsl_int_type(), 4, 0),
                                   "temp");
   nir_variable *temp2 = create_var(nir_var_function_temp, glsl_array_type(glsl_int_type(), 4, 0),
                                    "temp2");

   nir_deref_instr *temp_deref = nir_build_deref_var(b, temp);
   nir_deref_instr *temp2_deref = nir_build_deref_var(b, temp2);

   for (int i = 0; i < 4; i++)
      nir_store_deref(b, nir_build_deref_array_imm(b, temp_deref, i), nir_load_var(b, in[i]), 1);

   nir_deref_instr *src_wildcard = nir_build_deref_array_wildcard(b, temp_deref);
   nir_deref_instr *dst_wildcard = nir_build_deref_array_wildcard(b, temp2_deref);

   nir_copy_deref(b, dst_wildcard, src_wildcard);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_derefs(nir_deref_type_array), 4);
   ASSERT_EQ(count_derefs(nir_deref_type_array_wildcard), 2);
   ASSERT_EQ(count_function_temp_vars(), 2);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_copy_deref), 1);

   bool progress = nir_split_array_vars(b->shader, nir_var_function_temp);
   EXPECT_TRUE(progress);

   nir_validate_shader(b->shader, NULL);
   ASSERT_EQ(count_derefs(nir_deref_type_array), 0);
   ASSERT_EQ(count_derefs(nir_deref_type_array_wildcard), 0);
   ASSERT_EQ(count_function_temp_vars(), 8);
   ASSERT_EQ(count_intrinsics(nir_intrinsic_copy_deref), 4);
}

TEST_F(nir_split_vars_test, split_nested_struct_const_init)
{
   const struct glsl_struct_field inner_struct_types[] = {
      { glsl_int_type(), "a"},
      { glsl_int_type(), "b"},
   };
   const struct glsl_type *inner_struct = glsl_struct_type(inner_struct_types, 2, "inner", false);
   const struct glsl_struct_field outer_struct_types[] = {
      { glsl_array_type(inner_struct, 2, 0), "as" },
      { glsl_array_type(inner_struct, 2, 0), "bs" },
   };
   const struct glsl_type *outer_struct = glsl_struct_type(outer_struct_types, 2, "outer", false);
   nir_variable *var = create_var(nir_var_mem_constant, glsl_array_type(outer_struct, 2, 0), "consts");

   uint32_t literal_val = 0;
   auto get_inner_struct_val = [&]() {
      nir_constant ret = {};
      ret.values[0].u32 = literal_val++;
      return ret;
   };
   auto get_nested_constant = [&](auto &get_inner_val) {
      nir_constant *arr = ralloc_array(b->shader, nir_constant, 2);
      arr[0] = get_inner_val();
      arr[1] = get_inner_val();
      nir_constant **arr2 = ralloc_array(b->shader, nir_constant *, 2);
      arr2[0] = &arr[0];
      arr2[1] = &arr[1];
      nir_constant ret = {};
      ret.num_elements = 2;
      ret.elements = arr2;
      return ret;
   };
   auto get_inner_struct_constant = [&]() { return get_nested_constant(get_inner_struct_val); };
   auto get_inner_array_constant = [&]() { return get_nested_constant(get_inner_struct_constant); };
   auto get_outer_struct_constant = [&]() { return get_nested_constant(get_inner_array_constant); };
   auto get_outer_array_constant = [&]() { return get_nested_constant(get_outer_struct_constant); };
   nir_constant var_constant = get_outer_array_constant();
   var->constant_initializer = &var_constant;

   nir_variable *out = create_int(nir_var_shader_out, "out");
   nir_store_var(b, out,
      nir_load_deref(b,
         nir_build_deref_struct(b,
            nir_build_deref_array_imm(b,
               nir_build_deref_struct(b,
                  nir_build_deref_array_imm(b, nir_build_deref_var(b, var), 1),
                                      0),
                                      1),
                                1)
                     ),
                 0xff);

   nir_validate_shader(b->shader, NULL);

   bool progress = nir_split_struct_vars(b->shader, nir_var_mem_constant);
   EXPECT_TRUE(progress);

   nir_validate_shader(b->shader, NULL);
   
   unsigned count = 0;
   nir_foreach_variable_with_modes(var, b->shader, nir_var_mem_constant) {
      EXPECT_EQ(glsl_get_aoa_size(var->type), 4);
      EXPECT_EQ(glsl_get_length(var->type), 2);
      EXPECT_EQ(glsl_without_array(var->type), glsl_int_type());
      count++;
   }

   ASSERT_EQ(count, 4);
}

TEST_F(nir_remove_dead_variables_test, pointer_initializer_used)
{
   nir_variable *x = create_int(nir_var_shader_temp, "x");
   nir_variable *y = create_int(nir_var_shader_temp, "y");
   y->pointer_initializer = x;
   nir_variable *out = create_int(nir_var_shader_out, "out");

   nir_validate_shader(b->shader, NULL);

   nir_copy_var(b, out, y);

   bool progress = nir_remove_dead_variables(b->shader, nir_var_all, NULL);
   EXPECT_FALSE(progress);

   nir_validate_shader(b->shader, NULL);

   unsigned count = 0;
   nir_foreach_variable_in_shader(var, b->shader)
      count++;

   ASSERT_EQ(count, 3);
}

TEST_F(nir_remove_dead_variables_test, pointer_initializer_dead)
{
   nir_variable *x = create_int(nir_var_shader_temp, "x");
   nir_variable *y = create_int(nir_var_shader_temp, "y");
   nir_variable *z = create_int(nir_var_shader_temp, "z");
   y->pointer_initializer = x;
   z->pointer_initializer = y;

   nir_validate_shader(b->shader, NULL);

   bool progress = nir_remove_dead_variables(b->shader, nir_var_all, NULL);
   EXPECT_TRUE(progress);

   nir_validate_shader(b->shader, NULL);

   unsigned count = 0;
   nir_foreach_variable_in_shader(var, b->shader)
      count++;

   ASSERT_EQ(count, 0);
}


