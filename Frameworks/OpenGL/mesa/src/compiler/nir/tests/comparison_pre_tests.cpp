/*
 * Copyright © 2019 Intel Corporation
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
#include <gtest/gtest.h>
#include "nir.h"
#include "nir_builder.h"

class comparison_pre_test : public ::testing::Test {
protected:
   comparison_pre_test()
   {
      glsl_type_singleton_init_or_ref();

      static const nir_shader_compiler_options options = { };
      bld = nir_builder_init_simple_shader(MESA_SHADER_VERTEX, &options,
                                           "comparison test");

      v1 = nir_imm_vec4(&bld, -2.0, -1.0,  1.0,  2.0);
      v2 = nir_imm_vec4(&bld,  2.0,  1.0, -1.0, -2.0);
      v3 = nir_imm_vec4(&bld,  3.0,  4.0,  5.0,  6.0);
   }

   ~comparison_pre_test()
   {
      ralloc_free(bld.shader);
      glsl_type_singleton_decref();
   }

   struct nir_builder bld;

   nir_def *v1;
   nir_def *v2;
   nir_def *v3;

   const uint8_t xxxx[4] = { 0, 0, 0, 0 };
   const uint8_t wwww[4] = { 3, 3, 3, 3 };
};

TEST_F(comparison_pre_test, a_lt_b_vs_neg_a_plus_b)
{
   /* Before:
    *
    * vec4 32 ssa_0 = load_const (-2.0, -1.0,  1.0,  2.0)
    * vec4 32 ssa_1 = load_const ( 2.0,  1.0, -1.0, -2.0)
    * vec4 32 ssa_2 = load_const ( 3.0,  4.0,  5.0,  6.0)
    * vec1 32 ssa_3 = load_const ( 1.0)
    * vec4 32 ssa_4 = fadd ssa_0, ssa_2
    * vec1 32 ssa_5 = mov ssa_4.x
    * vec1 1 ssa_6 = flt ssa_5, ssa_3
    *
    * if ssa_6 {
    *    vec1 32 ssa_7 = fneg ssa_5
    *    vec1 32 ssa_8 = fadd ssa_7, ssa_3
    * } else {
    * }
    *
    * After:
    *
    * vec4 32 ssa_0 = load_const (-2.0, -1.0,  1.0,  2.0)
    * vec4 32 ssa_1 = load_const ( 2.0,  1.0, -1.0, -2.0)
    * vec4 32 ssa_2 = load_const ( 3.0,  4.0,  5.0,  6.0)
    * vec1 32 ssa_3 = load_const ( 1.0)
    * vec4 32 ssa_4 = fadd ssa_0, ssa_2
    * vec1 32 ssa_5 = mov ssa_4.x
    * vec1 32 ssa_9 = fneg ssa_5
    * vec1 32 ssa_10 = fadd ssa_3, ssa_9
    * vec1 32 ssa_11 = load_const (0.0)
    * vec1 1 ssa_12 = flt ssa_11, ssa_10
    * vec1 32 ssa_13 = mov ssa_10
    * vec1 1 ssa_14 = mov ssa_12
    *
    * if ssa_14 {
    *    vec1 32 ssa_7 = fneg ssa_5
    * } else {
    * }
    */
   nir_def *one = nir_imm_float(&bld, 1.0f);
   nir_def *a = nir_channel(&bld, nir_fadd(&bld, v1, v3), 0);

   nir_def *flt = nir_flt(&bld, a, one);

   nir_if *nif = nir_push_if(&bld, flt);

   nir_fadd(&bld, nir_fneg(&bld, a), one);

   nir_pop_if(&bld, nif);

   EXPECT_TRUE(nir_opt_comparison_pre_impl(bld.impl));
}

TEST_F(comparison_pre_test, a_lt_b_vs_a_minus_b)
{
   /* Before:
    *
    * vec4 32 ssa_0 = load_const (-2.0, -1.0,  1.0,  2.0)
    * vec4 32 ssa_1 = load_const ( 2.0,  1.0, -1.0, -2.0)
    * vec4 32 ssa_2 = load_const ( 3.0,  4.0,  5.0,  6.0)
    * vec1 32 ssa_3 = load_const ( 1.0)
    * vec4 32 ssa_4 = fadd ssa_0, ssa_2
    * vec1 32 ssa_5 = mov ssa_4.x
    * vec1 1 ssa_6 = flt ssa_3, ssa_5
    *
    * if ssa_6 {
    *    vec1 32 ssa_7 = fneg ssa_5
    *    vec1 32 ssa_8 = fadd ssa_3, ssa_7
    * } else {
    * }
    *
    * After:
    *
    * vec4 32 ssa_0 = load_const (-2.0, -1.0,  1.0,  2.0)
    * vec4 32 ssa_1 = load_const ( 2.0,  1.0, -1.0, -2.0)
    * vec4 32 ssa_2 = load_const ( 3.0,  4.0,  5.0,  6.0)
    * vec1 32 ssa_3 = load_const ( 1.0)
    * vec4 32 ssa_4 = fadd ssa_0, ssa_2
    * vec1 32 ssa_5 = mov ssa_4.x
    * vec1 32 ssa_9 = fneg ssa_5
    * vec1 32 ssa_10 = fadd ssa_3, ssa_9
    * vec1 32 ssa_11 = load_const (0.0)
    * vec1 1 ssa_12 = flt ssa_10, ssa_11
    * vec1 32 ssa_13 = mov ssa_10
    * vec1 1 ssa_14 = mov ssa_12
    *
    * if ssa_14 {
    *    vec1 32 ssa_7 = fneg ssa_5
    * } else {
    * }
    */
   nir_def *one = nir_imm_float(&bld, 1.0f);
   nir_def *b = nir_channel(&bld, nir_fadd(&bld, v1, v3), 0);

   nir_def *flt = nir_flt(&bld, one, b);

   nir_if *nif = nir_push_if(&bld, flt);

   nir_fadd(&bld, one, nir_fneg(&bld, b));

   nir_pop_if(&bld, nif);

   EXPECT_TRUE(nir_opt_comparison_pre_impl(bld.impl));
}

TEST_F(comparison_pre_test, neg_a_lt_b_vs_a_plus_b)
{
   /* Before:
    *
    * vec4 32 ssa_0 = load_const (-2.0, -1.0,  1.0,  2.0)
    * vec4 32 ssa_1 = load_const ( 2.0,  1.0, -1.0, -2.0)
    * vec4 32 ssa_2 = load_const ( 3.0,  4.0,  5.0,  6.0)
    * vec1 32 ssa_3 = load_const ( 1.0)
    * vec4 32 ssa_4 = fadd ssa_0, ssa_2
    * vec1 32 ssa_5 = mov ssa_4.x
    * vec1 32 ssa_6 = fneg ssa_5
    * vec1 1 ssa_7 = flt ssa_6, ssa_3
    *
    * if ssa_7 {
    *    vec1 32 ssa_8 = fadd ssa_5, ssa_3
    * } else {
    * }
    *
    * After:
    *
    * vec4 32 ssa_0 = load_const (-2.0, -1.0,  1.0,  2.0)
    * vec4 32 ssa_1 = load_const ( 2.0,  1.0, -1.0, -2.0)
    * vec4 32 ssa_2 = load_const ( 3.0,  4.0,  5.0,  6.0)
    * vec1 32 ssa_3 = load_const ( 1.0)
    * vec4 32 ssa_4 = fadd ssa_0, ssa_2
    * vec1 32 ssa_5 = mov ssa_4.x
    * vec1 32 ssa_9 = fneg ssa_5
    * vec1 32 ssa_9 = fneg ssa_6
    * vec1 32 ssa_10 = fadd ssa_3, ssa_9
    * vec1 32 ssa_11 = load_const ( 0.0)
    * vec1 1 ssa_12 = flt ssa_11, ssa_10
    * vec1 32 ssa_13 = mov ssa_10
    * vec1 1 ssa_14 = mov ssa_12
    *
    * if ssa_14 {
    * } else {
    * }
    */

   nir_def *one = nir_imm_float(&bld, 1.0f);
   nir_def *a = nir_channel(&bld, nir_fadd(&bld, v1, v3), 0);

   nir_def *flt = nir_flt(&bld, nir_fneg(&bld, a), one);

   nir_if *nif = nir_push_if(&bld, flt);

   nir_fadd(&bld, a, one);

   nir_pop_if(&bld, nif);

   EXPECT_TRUE(nir_opt_comparison_pre_impl(bld.impl));
}

TEST_F(comparison_pre_test, a_lt_neg_b_vs_a_plus_b)
{
   /* Before:
    *
    * vec4 32 ssa_0 = load_const (-2.0, -1.0,  1.0,  2.0)
    * vec4 32 ssa_1 = load_const ( 2.0,  1.0, -1.0, -2.0)
    * vec4 32 ssa_2 = load_const ( 3.0,  4.0,  5.0,  6.0)
    * vec1 32 ssa_3 = load_const ( 1.0)
    * vec4 32 ssa_4 = fadd ssa_0, ssa_2
    * vec1 32 ssa_5 = mov ssa_4.x
    * vec1 32 ssa_6 = fneg ssa_5
    * vec1 1 ssa_7 = flt ssa_3, ssa_6
    *
    * if ssa_7 {
    *    vec1 32 ssa_8 = fadd ssa_3, ssa_5
    * } else {
    * }
    *
    * After:
    *
    * vec4 32 ssa_0 = load_const (-2.0, -1.0,  1.0,  2.0)
    * vec4 32 ssa_1 = load_const ( 2.0,  1.0, -1.0, -2.0)
    * vec4 32 ssa_2 = load_const ( 3.0,  4.0,  5.0,  6.0)
    * vec1 32 ssa_3 = load_const ( 1.0)
    * vec4 32 ssa_4 = fadd ssa_0, ssa_2
    * vec1 32 ssa_5 = mov ssa_4.x
    * vec1 32 ssa_9 = fneg ssa_5
    * vec1 32 ssa_9 = fneg ssa_6
    * vec1 32 ssa_10 = fadd ssa_3, ssa_9
    * vec1 32 ssa_11 = load_const ( 0.0)
    * vec1 1 ssa_12 = flt ssa_10, ssa_11
    * vec1 32 ssa_13 = mov ssa_10
    * vec1 1 ssa_14 = mov ssa_12
    *
    * if ssa_14 {
    * } else {
    * }
    */
   nir_def *one = nir_imm_float(&bld, 1.0f);
   nir_def *b = nir_channel(&bld, nir_fadd(&bld, v1, v3), 0);

   nir_def *flt = nir_flt(&bld, one, nir_fneg(&bld, b));

   nir_if *nif = nir_push_if(&bld, flt);

   nir_fadd(&bld, one, b);

   nir_pop_if(&bld, nif);

   EXPECT_TRUE(nir_opt_comparison_pre_impl(bld.impl));
}

TEST_F(comparison_pre_test, imm_lt_b_vs_neg_imm_plus_b)
{
   /* Before:
    *
    * vec4 32 ssa_0 = load_const (-2.0, -1.0,  1.0,  2.0)
    * vec4 32 ssa_1 = load_const ( 2.0,  1.0, -1.0, -2.0)
    * vec4 32 ssa_2 = load_const ( 3.0,  4.0,  5.0,  6.0)
    * vec1 32 ssa_3 = load_const ( 1.0)
    * vec1 32 ssa_4 = load_const (-1.0)
    * vec4 32 ssa_5 = fadd ssa_0, ssa_2
    * vec1 32 ssa_6 = mov ssa_5.x
    * vec1 1 ssa_7 = flt ssa_3, ssa_6
    *
    * if ssa_7 {
    *    vec1 32 ssa_8 = fadd ssa_4, ssa_6
    * } else {
    * }
    *
    * After:
    *
    * vec4 32 ssa_0 = load_const (-2.0, -1.0,  1.0,  2.0)
    * vec4 32 ssa_1 = load_const ( 2.0,  1.0, -1.0, -2.0)
    * vec4 32 ssa_2 = load_const ( 3.0,  4.0,  5.0,  6.0)
    * vec1 32 ssa_3 = load_const ( 1.0)
    * vec1 32 ssa_4 = load_const (-1.0)
    * vec4 32 ssa_5 = fadd ssa_0, ssa_2
    * vec1 32 ssa_6 = mov ssa_5.x
    * vec1 32 ssa_9 = fneg ssa_3
    * vec1 32 ssa_10 = fadd ssa_6, ssa_9
    * vec1 32 ssa_11 = load_const ( 0.0)
    * vec1 1 ssa_12 = flt ssa_11, ssa_10
    * vec1 32 ssa_13 = mov ssa_10
    * vec1 1 ssa_14 = mov ssa_12
    *
    * if ssa_14 {
    * } else {
    * }
    */
   nir_def *one = nir_imm_float(&bld, 1.0f);
   nir_def *neg_one = nir_imm_float(&bld, -1.0f);
   nir_def *a = nir_channel(&bld, nir_fadd(&bld, v1, v3), 0);

   nir_def *flt = nir_flt(&bld, one, a);

   nir_if *nif = nir_push_if(&bld, flt);

   nir_fadd(&bld, neg_one, a);

   nir_pop_if(&bld, nif);

   EXPECT_TRUE(nir_opt_comparison_pre_impl(bld.impl));
}

TEST_F(comparison_pre_test, a_lt_imm_vs_a_minus_imm)
{
   /* Before:
    *
    * vec4 32 ssa_0 = load_const (-2.0, -1.0,  1.0,  2.0)
    * vec4 32 ssa_1 = load_const ( 2.0,  1.0, -1.0, -2.0)
    * vec4 32 ssa_2 = load_const ( 3.0,  4.0,  5.0,  6.0)
    * vec1 32 ssa_3 = load_const ( 1.0)
    * vec1 32 ssa_4 = load_const (-1.0)
    * vec4 32 ssa_5 = fadd ssa_0, ssa_2
    * vec1 32 ssa_6 = mov ssa_5.x
    * vec1 1 ssa_7 = flt ssa_6, ssa_3
    *
    * if ssa_6 {
    *    vec1 32 ssa_8 = fadd ssa_6, ssa_4
    * } else {
    * }
    *
    * After:
    *
    * vec4 32 ssa_0 = load_const (-2.0, -1.0,  1.0,  2.0)
    * vec4 32 ssa_1 = load_const ( 2.0,  1.0, -1.0, -2.0)
    * vec4 32 ssa_2 = load_const ( 3.0,  4.0,  5.0,  6.0)
    * vec1 32 ssa_3 = load_const ( 1.0)
    * vec1 32 ssa_4 = load_const (-1.0)
    * vec4 32 ssa_5 = fadd ssa_0, ssa_2
    * vec1 32 ssa_6 = mov ssa_5.x
    * vec1 32 ssa_9 = fneg ssa_3
    * vec1 32 ssa_10 = fadd ssa_6, ssa_9
    * vec1 32 ssa_11 = load_const ( 0.0)
    * vec1 1 ssa_12 = flt ssa_10, ssa_11
    * vec1 32 ssa_13 = mov ssa_10
    * vec1 1 ssa_14 = mov ssa_12
    *
    * if ssa_14 {
    * } else {
    * }
    */
   nir_def *one = nir_imm_float(&bld, 1.0f);
   nir_def *neg_one = nir_imm_float(&bld, -1.0f);
   nir_def *a = nir_channel(&bld, nir_fadd(&bld, v1, v3), 0);

   nir_def *flt = nir_flt(&bld, a, one);

   nir_if *nif = nir_push_if(&bld, flt);

   nir_fadd(&bld, a, neg_one);

   nir_pop_if(&bld, nif);

   EXPECT_TRUE(nir_opt_comparison_pre_impl(bld.impl));
}

TEST_F(comparison_pre_test, neg_imm_lt_a_vs_a_plus_imm)
{
   /* Before:
    *
    * vec4 32 ssa_0 = load_const (-2.0, -1.0,  1.0,  2.0)
    * vec4 32 ssa_1 = load_const ( 2.0,  1.0, -1.0, -2.0)
    * vec4 32 ssa_2 = load_const ( 3.0,  4.0,  5.0,  6.0)
    * vec1 32 ssa_3 = load_const ( 1.0)
    * vec1 32 ssa_4 = load_const (-1.0)
    * vec4 32 ssa_5 = fadd ssa_0, ssa_2
    * vec1 32 ssa_6 = mov ssa_5.x
    * vec1 1 ssa_7 = flt ssa_4, ssa_6
    *
    * if ssa_7 {
    *    vec1 32 ssa_8 = fadd ssa_6, ssa_3
    * } else {
    * }
    *
    * After:
    *
    * vec4 32 ssa_0 = load_const (-2.0, -1.0,  1.0,  2.0)
    * vec4 32 ssa_1 = load_const ( 2.0,  1.0, -1.0, -2.0)
    * vec4 32 ssa_2 = load_const ( 3.0,  4.0,  5.0,  6.0)
    * vec1 32 ssa_3 = load_const ( 1.0)
    * vec1 32 ssa_4 = load_const (-1.0)
    * vec4 32 ssa_5 = fadd ssa_0, ssa_2
    * vec1 32 ssa_6 = mov ssa_5.x
    * vec1 32 ssa_9 = fneg ssa_4
    * vec1 32 ssa_10 = fadd ssa_6, ssa_9
    * vec1 32 ssa_11 = load_const ( 0.0)
    * vec1 1 ssa_12 = flt ssa_11, ssa_10
    * vec1 32 ssa_13 = mov ssa_10
    * vec1 1 ssa_14 = mov ssa_12
    *
    * if ssa_14 {
    * } else {
    * }
    */

   nir_def *one = nir_imm_float(&bld, 1.0f);
   nir_def *neg_one = nir_imm_float(&bld, -1.0f);
   nir_def *a = nir_channel(&bld, nir_fadd(&bld, v1, v3), 0);

   nir_def *flt = nir_flt(&bld, neg_one, a);

   nir_if *nif = nir_push_if(&bld, flt);

   nir_fadd(&bld, a, one);

   nir_pop_if(&bld, nif);

   EXPECT_TRUE(nir_opt_comparison_pre_impl(bld.impl));
}

TEST_F(comparison_pre_test, a_lt_neg_imm_vs_a_plus_imm)
{
   /* Before:
    *
    * vec4 32 ssa_0 = load_const (-2.0, -1.0,  1.0,  2.0)
    * vec4 32 ssa_1 = load_const ( 2.0,  1.0, -1.0, -2.0)
    * vec4 32 ssa_2 = load_const ( 3.0,  4.0,  5.0,  6.0)
    * vec1 32 ssa_3 = load_const ( 1.0)
    * vec1 32 ssa_4 = load_const (-1.0)
    * vec4 32 ssa_5 = fadd ssa_0, ssa_2
    * vec1 32 ssa_6 = mov ssa_5.x
    * vec1 1 ssa_7 = flt ssa_6, ssa_4
    *
    * if ssa_7 {
    *    vec1 32 ssa_8 = fadd ssa_6, ssa_3
    * } else {
    * }
    *
    * After:
    *
    * vec4 32 ssa_0 = load_const (-2.0, -1.0,  1.0,  2.0)
    * vec4 32 ssa_1 = load_const ( 2.0,  1.0, -1.0, -2.0)
    * vec4 32 ssa_2 = load_const ( 3.0,  4.0,  5.0,  6.0)
    * vec1 32 ssa_3 = load_const ( 1.0)
    * vec1 32 ssa_4 = load_const (-1.0)
    * vec4 32 ssa_5 = fadd ssa_0, ssa_2
    * vec1 32 ssa_6 = mov ssa_5.x
    * vec1 32 ssa_9 = fneg ssa_4
    * vec1 32 ssa_10 = fadd ssa_6, ssa_9
    * vec1 32 ssa_11 = load_const ( 0.0)
    * vec1 1 ssa_12 = flt ssa_10, ssa_11
    * vec1 32 ssa_13 = mov ssa_10
    * vec1 1 ssa_14 = mov ssa_12
    *
    * if ssa_14 {
    * } else {
    * }
    */
   nir_def *one = nir_imm_float(&bld, 1.0f);
   nir_def *neg_one = nir_imm_float(&bld, -1.0f);
   nir_def *a = nir_channel(&bld, nir_fadd(&bld, v1, v3), 0);

   nir_def *flt = nir_flt(&bld, a, neg_one);

   nir_if *nif = nir_push_if(&bld, flt);

   nir_fadd(&bld, a, one);

   nir_pop_if(&bld, nif);

   EXPECT_TRUE(nir_opt_comparison_pre_impl(bld.impl));
}

TEST_F(comparison_pre_test, swizzle_of_same_immediate_vector)
{
   /* Before:
    *
    * vec4 32 ssa_0 = load_const (-2.0, -1.0,  1.0,  2.0)
    * vec4 32 ssa_1 = load_const ( 2.0,  1.0, -1.0, -2.0)
    * vec4 32 ssa_2 = load_const ( 3.0,  4.0,  5.0,  6.0)
    * vec4 32 ssa_3 = fadd ssa_0, ssa_2
    * vec1 1 ssa_4 = flt ssa_0.x, ssa_3.x
    *
    * if ssa_4 {
    *    vec1 32 ssa_5 = fadd ssa_0.w, ssa_3.x
    * } else {
    * }
    */
   nir_def *a = nir_fadd(&bld, v1, v3);

   nir_alu_instr *flt = nir_alu_instr_create(bld.shader, nir_op_flt);

   flt->src[0].src = nir_src_for_ssa(v1);
   flt->src[1].src = nir_src_for_ssa(a);

   memcpy(&flt->src[0].swizzle, xxxx, sizeof(xxxx));
   memcpy(&flt->src[1].swizzle, xxxx, sizeof(xxxx));

   nir_builder_alu_instr_finish_and_insert(&bld, flt);

   flt->def.num_components = 1;

   nir_if *nif = nir_push_if(&bld, &flt->def);

   nir_alu_instr *fadd = nir_alu_instr_create(bld.shader, nir_op_fadd);

   fadd->src[0].src = nir_src_for_ssa(v1);
   fadd->src[1].src = nir_src_for_ssa(a);

   memcpy(&fadd->src[0].swizzle, wwww, sizeof(wwww));
   memcpy(&fadd->src[1].swizzle, xxxx, sizeof(xxxx));

   nir_builder_alu_instr_finish_and_insert(&bld, fadd);

   fadd->def.num_components = 1;

   nir_pop_if(&bld, nif);

   EXPECT_TRUE(nir_opt_comparison_pre_impl(bld.impl));
}

TEST_F(comparison_pre_test, non_scalar_add_result)
{
   /* The optimization pass should not do anything because the result of the
    * fadd is not a scalar.
    *
    * Before:
    *
    * vec4 32 ssa_0 = load_const (-2.0, -1.0,  1.0,  2.0)
    * vec4 32 ssa_1 = load_const ( 2.0,  1.0, -1.0, -2.0)
    * vec4 32 ssa_2 = load_const ( 3.0,  4.0,  5.0,  6.0)
    * vec4 32 ssa_3 = fadd ssa_0, ssa_2
    * vec1 1 ssa_4 = flt ssa_0.x, ssa_3.x
    *
    * if ssa_4 {
    *    vec2 32 ssa_5 = fadd ssa_1.xx, ssa_3.xx
    * } else {
    * }
    *
    * After:
    *
    * No change.
    */
   nir_def *a = nir_fadd(&bld, v1, v3);

   nir_alu_instr *flt = nir_alu_instr_create(bld.shader, nir_op_flt);

   flt->src[0].src = nir_src_for_ssa(v1);
   flt->src[1].src = nir_src_for_ssa(a);

   memcpy(&flt->src[0].swizzle, xxxx, sizeof(xxxx));
   memcpy(&flt->src[1].swizzle, xxxx, sizeof(xxxx));

   nir_builder_alu_instr_finish_and_insert(&bld, flt);

   flt->def.num_components = 1;

   nir_if *nif = nir_push_if(&bld, &flt->def);

   nir_alu_instr *fadd = nir_alu_instr_create(bld.shader, nir_op_fadd);

   fadd->src[0].src = nir_src_for_ssa(v2);
   fadd->src[1].src = nir_src_for_ssa(a);

   memcpy(&fadd->src[0].swizzle, xxxx, sizeof(xxxx));
   memcpy(&fadd->src[1].swizzle, xxxx, sizeof(xxxx));

   nir_builder_alu_instr_finish_and_insert(&bld, fadd);

   fadd->def.num_components = 2;

   nir_pop_if(&bld, nif);

   EXPECT_FALSE(nir_opt_comparison_pre_impl(bld.impl));
}

TEST_F(comparison_pre_test, multi_comps_load)
{
   /* Before:
    *
    * vec1 32 ssa_0 = load_ubo (...)
    * vec4 32 ssa_1 = load_ubo (...)
    * vec1 1  ssa_2 = flt ssa_0, ssa_1.w
    *
    * if ssa_2 {
    *    vec1 32 ssa_3 = fneg ssa_1.x
    *    vec1 32 ssa_4 = fadd ssa_0, ssa_3
    * } else {
    * }
    */
   nir_def *ssa_0 = nir_load_ubo(&bld, 1, 32,
                                 nir_imm_int(&bld, 0),
                                 nir_imm_int(&bld, 0));
   nir_def *ssa_1 = nir_load_ubo(&bld, 4, 32,
                                 nir_imm_int(&bld, 1),
                                 nir_imm_int(&bld, 0));

   nir_alu_instr *flt = nir_alu_instr_create(bld.shader, nir_op_flt);
   flt->src[0].src = nir_src_for_ssa(ssa_0);
   flt->src[1].src = nir_src_for_ssa(ssa_1);
   memcpy(&flt->src[0].swizzle, xxxx, sizeof(xxxx));
   memcpy(&flt->src[1].swizzle, wwww, sizeof(wwww));
   nir_builder_alu_instr_finish_and_insert(&bld, flt);
   flt->def.num_components = 1;
   nir_def *ssa_2 = &flt->def;

   nir_if *nif = nir_push_if(&bld, ssa_2);
   {
      nir_alu_instr *fneg = nir_alu_instr_create(bld.shader, nir_op_fneg);
      fneg->src[0].src = nir_src_for_ssa(ssa_1);
      memcpy(&fneg->src[0].swizzle, xxxx, sizeof(xxxx));
      nir_builder_alu_instr_finish_and_insert(&bld, fneg);
      fneg->def.num_components = 1;
      nir_def *ssa_3 = &fneg->def;

      nir_fadd(&bld, ssa_0, ssa_3);
   }
   nir_pop_if(&bld, nif);

   EXPECT_FALSE(nir_opt_comparison_pre_impl(bld.impl));
}

TEST_F(comparison_pre_test, multi_comps_load2)
{
   /* Before:
    *
    * vec1 32 ssa_0 = load_ubo (...)
    * vec4 32 ssa_1 = load_ubo (...)
    * vec1 1  ssa_2 = flt ssa_0, ssa_1.x
    *
    * if ssa_2 {
    *    vec1 32 ssa_3 = fneg ssa_1.w
    *    vec1 32 ssa_4 = fadd ssa_0, ssa_3
    * } else {
    * }
    */
   nir_def *ssa_0 = nir_load_ubo(&bld, 1, 32,
                                 nir_imm_int(&bld, 0),
                                 nir_imm_int(&bld, 0));
   nir_def *ssa_1 = nir_load_ubo(&bld, 4, 32,
                                 nir_imm_int(&bld, 1),
                                 nir_imm_int(&bld, 0));

   nir_alu_instr *flt = nir_alu_instr_create(bld.shader, nir_op_flt);
   flt->src[0].src = nir_src_for_ssa(ssa_0);
   flt->src[1].src = nir_src_for_ssa(ssa_1);
   memcpy(&flt->src[0].swizzle, xxxx, sizeof(xxxx));
   memcpy(&flt->src[1].swizzle, xxxx, sizeof(xxxx));
   nir_builder_alu_instr_finish_and_insert(&bld, flt);
   flt->def.num_components = 1;
   nir_def *ssa_2 = &flt->def;

   nir_if *nif = nir_push_if(&bld, ssa_2);
   {
      nir_alu_instr *fneg = nir_alu_instr_create(bld.shader, nir_op_fneg);
      fneg->src[0].src = nir_src_for_ssa(ssa_1);
      memcpy(&fneg->src[0].swizzle, wwww, sizeof(wwww));
      nir_builder_alu_instr_finish_and_insert(&bld, fneg);
      fneg->def.num_components = 1;
      nir_def *ssa_3 = &fneg->def;

      nir_fadd(&bld, ssa_0, ssa_3);
   }
   nir_pop_if(&bld, nif);

   EXPECT_FALSE(nir_opt_comparison_pre_impl(bld.impl));
}
