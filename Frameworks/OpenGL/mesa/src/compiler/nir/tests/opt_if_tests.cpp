/*
 * Copyright © 2020 Intel Corporation
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

class nir_opt_if_test : public nir_test {
protected:
   nir_opt_if_test();

   nir_builder bld;

   nir_def *in_def;
   nir_variable *out_var;
};

nir_opt_if_test::nir_opt_if_test()
   : nir_test::nir_test("nir_opt_if_test")
{
   nir_variable *var = nir_variable_create(b->shader, nir_var_shader_in, glsl_int_type(), "in");
   in_def = nir_load_var(b, var);

   out_var = nir_variable_create(b->shader, nir_var_shader_out, glsl_int_type(), "out");
}

TEST_F(nir_opt_if_test, opt_if_simplification)
{
   /* Tests that opt_if_simplification correctly optimizes a simple case:
    *
    * vec1 1 ssa_2 = ieq ssa_0, ssa_1
    * if ssa_2 {
    *    block block_2:
    * } else {
    *    block block_3:
    *    do_work()
    * }
    */

   nir_def *one = nir_imm_int(b, 1);

   nir_def *cmp_result = nir_ieq(b, in_def, one);
   nir_if *nif = nir_push_if(b, cmp_result);

   nir_push_else(b, NULL);

   // do_work
   nir_store_var(b, out_var, one, 1);

   nir_pop_if(b, NULL);

   ASSERT_TRUE(nir_opt_if(b->shader, nir_opt_if_optimize_phi_true_false));

   nir_validate_shader(b->shader, NULL);

   ASSERT_TRUE(!exec_list_is_empty((&nir_if_first_then_block(nif)->instr_list)));
   ASSERT_TRUE(exec_list_is_empty((&nir_if_first_else_block(nif)->instr_list)));
}

TEST_F(nir_opt_if_test, opt_if_simplification_single_source_phi_after_if)
{
   /* Tests that opt_if_simplification correctly handles single-source
    * phis after the if.
    *
    * vec1 1 ssa_2 = ieq ssa_0, ssa_1
    * if ssa_2 {
    *    block block_2:
    * } else {
    *    block block_3:
    *    do_work()
    *    return
    * }
    * block block_4:
    * vec1 32 ssa_3 = phi block_2: ssa_0
    */

   nir_def *one = nir_imm_int(b, 1);

   nir_def *cmp_result = nir_ieq(b, in_def, one);
   nir_if *nif = nir_push_if(b, cmp_result);

   nir_push_else(b, NULL);

   // do_work
   nir_store_var(b, out_var, one, 1);

   nir_jump_instr *jump = nir_jump_instr_create(b->shader, nir_jump_return);
   nir_builder_instr_insert(b, &jump->instr);

   nir_pop_if(b, NULL);

   nir_block *then_block = nir_if_last_then_block(nif);

   nir_phi_instr *const phi = nir_phi_instr_create(b->shader);

   nir_phi_instr_add_src(phi, then_block, one);

   nir_def_init(&phi->instr, &phi->def,
                one->num_components, one->bit_size);

   nir_builder_instr_insert(b, &phi->instr);

   ASSERT_TRUE(nir_opt_if(b->shader, nir_opt_if_optimize_phi_true_false));

   nir_validate_shader(b->shader, NULL);

   ASSERT_TRUE(nir_block_ends_in_jump(nir_if_last_then_block(nif)));
   ASSERT_TRUE(exec_list_is_empty((&nir_if_first_else_block(nif)->instr_list)));
}

TEST_F(nir_opt_if_test, opt_if_alu_of_phi_progress)
{
   nir_def *two = nir_imm_int(b, 2);
   nir_def *x = nir_imm_int(b, 0);

   nir_phi_instr *phi = nir_phi_instr_create(b->shader);

   nir_loop *loop = nir_push_loop(b);
   {
      nir_def_init(&phi->instr, &phi->def,
                   x->num_components, x->bit_size);

      nir_phi_instr_add_src(phi, x->parent_instr->block, x);

      nir_def *y = nir_iadd(b, &phi->def, two);
      nir_store_var(b, out_var,
                    nir_imul(b, &phi->def, two), 1);

      nir_phi_instr_add_src(phi, nir_cursor_current_block(b->cursor), y);
   }
   nir_pop_loop(b, loop);

   b->cursor = nir_before_block(nir_loop_first_block(loop));
   nir_builder_instr_insert(b, &phi->instr);

   nir_validate_shader(b->shader, "input");

   bool progress;

   int progress_count = 0;
   for (int i = 0; i < 10; i++) {
      progress = nir_opt_if(b->shader, nir_opt_if_optimize_phi_true_false);
      if (progress)
         progress_count++;
      else
         break;
      nir_opt_constant_folding(b->shader);
   }

   EXPECT_LE(progress_count, 2);
   ASSERT_FALSE(progress);
}
