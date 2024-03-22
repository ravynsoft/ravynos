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

namespace {

class nir_core_test : public nir_test {
protected:
   nir_core_test()
      : nir_test::nir_test("nir_core_test")
   {
   }

   bool shader_contains_def(nir_def *def);
};

struct contains_def_state {
   nir_def *def;
   bool found;
};

static bool
contains_def_cb(nir_def *def, void *_state)
{
   struct contains_def_state *state = (struct contains_def_state *)_state;
   if (def == state->def)
      state->found = true;

   return true;
}

bool
nir_core_test::shader_contains_def(nir_def *def)
{
   nir_foreach_block(block, b->impl) {
      nir_foreach_instr(instr, block) {
         struct contains_def_state state = {
            def, false
         };
         nir_foreach_def(instr, contains_def_cb, &state);
         if (state.found)
            return true;
      }
   }
   return false;
}

TEST_F(nir_core_test, nir_instr_free_and_dce_test)
{
   nir_def *zero = nir_imm_int(b, 0);
   nir_def *one = nir_imm_int(b, 1);
   nir_def *add01 = nir_iadd(b, zero, one);
   nir_def *add11 = nir_iadd(b, one, one);

   nir_cursor c = nir_instr_free_and_dce(add01->parent_instr);
   ASSERT_FALSE(shader_contains_def(add01));
   ASSERT_TRUE(shader_contains_def(add11));
   ASSERT_FALSE(shader_contains_def(zero));
   ASSERT_TRUE(shader_contains_def(one));

   ASSERT_TRUE(nir_cursors_equal(c, nir_before_instr(add11->parent_instr)));

   nir_validate_shader(b->shader, "after remove_and_dce");
}

TEST_F(nir_core_test, nir_instr_free_and_dce_all_test)
{
   nir_def *one = nir_imm_int(b, 1);
   nir_def *add = nir_iadd(b, one, one);

   nir_cursor c = nir_instr_free_and_dce(add->parent_instr);
   ASSERT_FALSE(shader_contains_def(add));
   ASSERT_FALSE(shader_contains_def(one));

   ASSERT_TRUE(nir_cursors_equal(c, nir_before_block(nir_start_block(b->impl))));

   nir_validate_shader(b->shader, "after remove_and_dce");
}

TEST_F(nir_core_test, nir_instr_free_and_dce_multiple_src_test)
{
   nir_def *one = nir_imm_int(b, 1);
   nir_def *add = nir_iadd(b, one, one);

   /* This risks triggering removing add multiple times, which can segfault in
    * nir_instr_remove for instructions with srcs. */
   nir_def *add2 = nir_iadd(b, add, add);

   nir_cursor c = nir_instr_free_and_dce(add2->parent_instr);
   ASSERT_FALSE(shader_contains_def(add2));
   ASSERT_FALSE(shader_contains_def(add));
   ASSERT_FALSE(shader_contains_def(one));

   ASSERT_TRUE(nir_cursors_equal(c, nir_before_block(nir_start_block(b->impl))));

   nir_validate_shader(b->shader, "after remove_and_dce");
}

}
