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

class nir_opt_dce_test : public nir_test {
protected:
   nir_opt_dce_test()
      : nir_test::nir_test("nir_opt_dce_test")
   {
   }
};

class nir_opt_dead_cf_test : public nir_test {
protected:
   nir_opt_dead_cf_test()
      : nir_test::nir_test("nir_opt_dead_cf_test")
   {
   }
};

nir_phi_instr *create_one_source_phi(nir_shader *shader, nir_block *pred,
                                     nir_def *def)
{
   nir_phi_instr *phi = nir_phi_instr_create(shader);
   nir_phi_instr_add_src(phi, pred, def);
   nir_def_init(&phi->instr, &phi->def, def->num_components,
                def->bit_size);

   return phi;
}

TEST_F(nir_opt_dce_test, return_before_loop)
{
   /* Test that nir_opt_dce() works correctly with loops immediately following a jump.
    * nir_opt_dce() has a fast path for loops without continues, and this test ensures that it
    * looks at the actual predecessors of the header instead of just counting.
    *
    *  block block_0:
    *  // preds:
    *  return
    *  // succs: block_3
    *  loop {
    *     block block_1:
    *     // preds: block_1
    *     vec1 32 ssa_1 = phi block_1: ssa_0
    *     vec1 32 ssa_0 = load_const (0x00000001)
    *     vec1 32 ssa_2 = deref_var &out (shader_out int) 
    *     intrinsic store_deref (ssa_2, ssa_1) (1, 0)
    *     // succs: block_1
    *  }
    *  block block_2:
    *  // preds:
    *  // succs: block_3
    *  block block_3:
    *
    * If the fast path is taken here, ssa_0 will be incorrectly DCE'd.
    */

   nir_variable *var = nir_variable_create(b->shader, nir_var_shader_out, glsl_int_type(), "out");

   nir_jump(b, nir_jump_return);

   nir_loop *loop = nir_push_loop(b);

   nir_def *one = nir_imm_int(b, 1);

   nir_phi_instr *phi = create_one_source_phi(b->shader, one->parent_instr->block, one);
   nir_instr_insert_before_block(one->parent_instr->block, &phi->instr);

   nir_store_var(b, var, &phi->def, 0x1);

   nir_pop_loop(b, loop);

   ASSERT_FALSE(nir_opt_dce(b->shader));

   nir_validate_shader(b->shader, NULL);
}

TEST_F(nir_opt_dead_cf_test, jump_before_constant_if)
{
   /*
    * Test that nir_opt_dead_cf removes everything after the jump, instead of trying and failing to
    * stitch b0 and b3.
    *
    * block b0:  // preds:
    * 1     %0 = load_const (false)
    *            return
    *            // succs: b4
    * if %0 (false) {
    *     block b1:  // preds:
    *     32    %1 = load_const (0x00000001)
    *     32    %2 = deref_var &out (shader_out int)
    *                @store_deref (%2, %1 (0x1)) (wrmask=x, access=0)
    *                // succs: b3
    * } else {
    *     block b2:  // preds: , succs: b3
    * }
    * block b3:  // preds: b1 b2
    * 32    %3 = load_const (0x00000000)
    * 32    %4 = deref_var &out (shader_out int)
    *            @store_deref (%4, %3 (0x0)) (wrmask=x, access=0)
    *            // succs: b4
    * block b4:
    */
   nir_variable *var = nir_variable_create(b->shader, nir_var_shader_out, glsl_int_type(), "out");

   nir_def *cond = nir_imm_false(b);
   nir_jump(b, nir_jump_return);
   nir_push_if(b, cond);
   nir_store_var(b, var, nir_imm_int(b, 1), 0x1);
   nir_pop_if(b, NULL);

   nir_store_var(b, var, nir_imm_int(b, 0), 0x1);

   ASSERT_TRUE(nir_opt_dead_cf(b->shader));

   nir_validate_shader(b->shader, NULL);
}
