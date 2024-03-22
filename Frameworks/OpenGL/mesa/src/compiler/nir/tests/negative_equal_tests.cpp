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
#include <gtest/gtest.h>
#include "nir.h"
#include "nir_builder.h"
#include "util/half_float.h"

static void count_sequence(nir_const_value c[NIR_MAX_VEC_COMPONENTS],
                           nir_alu_type full_type, int first);
static void negate(nir_const_value dst[NIR_MAX_VEC_COMPONENTS],
                   const nir_const_value src[NIR_MAX_VEC_COMPONENTS],
                   nir_alu_type full_type, unsigned components);

class const_value_negative_equal_test : public ::testing::Test {
protected:
   const_value_negative_equal_test()
   {
      glsl_type_singleton_init_or_ref();

      memset(c1, 0, sizeof(c1));
      memset(c2, 0, sizeof(c2));
   }

   ~const_value_negative_equal_test()
   {
      glsl_type_singleton_decref();
   }

   nir_const_value c1[NIR_MAX_VEC_COMPONENTS];
   nir_const_value c2[NIR_MAX_VEC_COMPONENTS];
};

class alu_srcs_negative_equal_test : public ::testing::Test {
protected:
   alu_srcs_negative_equal_test()
   {
      glsl_type_singleton_init_or_ref();

      static const nir_shader_compiler_options options = { };
      bld = nir_builder_init_simple_shader(MESA_SHADER_VERTEX, &options,
                                           "negative equal tests");
      memset(c1, 0, sizeof(c1));
      memset(c2, 0, sizeof(c2));
   }

   ~alu_srcs_negative_equal_test()
   {
      ralloc_free(bld.shader);
      glsl_type_singleton_decref();
   }

   struct nir_builder bld;
   nir_const_value c1[NIR_MAX_VEC_COMPONENTS];
   nir_const_value c2[NIR_MAX_VEC_COMPONENTS];
};

TEST_F(const_value_negative_equal_test, float32_zero)
{
   /* Verify that 0.0 negative-equals 0.0. */
   EXPECT_TRUE(nir_const_value_negative_equal(c1[0], c1[0], nir_type_float32));
}

TEST_F(const_value_negative_equal_test, float64_zero)
{
   /* Verify that 0.0 negative-equals 0.0. */
   EXPECT_TRUE(nir_const_value_negative_equal(c1[0], c1[0], nir_type_float64));
}

/* Compare an object with non-zero values to itself.  This should always be
 * false.
 */
#define compare_with_self(full_type)                                    \
TEST_F(const_value_negative_equal_test, full_type ## _self)             \
{                                                                       \
   count_sequence(c1, full_type, 1);                                    \
   EXPECT_FALSE(nir_const_value_negative_equal(c1[0], c1[0], full_type)); \
}

compare_with_self(nir_type_float16)
compare_with_self(nir_type_float32)
compare_with_self(nir_type_float64)
compare_with_self(nir_type_int8)
compare_with_self(nir_type_uint8)
compare_with_self(nir_type_int16)
compare_with_self(nir_type_uint16)
compare_with_self(nir_type_int32)
compare_with_self(nir_type_uint32)
compare_with_self(nir_type_int64)
compare_with_self(nir_type_uint64)
#undef compare_with_self

/* Compare an object with the negation of itself.  This should always be true.
 */
#define compare_with_negation(full_type)                                \
TEST_F(const_value_negative_equal_test, full_type ## _trivially_true)   \
{                                                                       \
   count_sequence(c1, full_type, 1);                                    \
   negate(c2, c1, full_type, 1);                                        \
   EXPECT_TRUE(nir_const_value_negative_equal(c1[0], c2[0], full_type)); \
}

compare_with_negation(nir_type_float16)
compare_with_negation(nir_type_float32)
compare_with_negation(nir_type_float64)
compare_with_negation(nir_type_int8)
compare_with_negation(nir_type_uint8)
compare_with_negation(nir_type_int16)
compare_with_negation(nir_type_uint16)
compare_with_negation(nir_type_int32)
compare_with_negation(nir_type_uint32)
compare_with_negation(nir_type_int64)
compare_with_negation(nir_type_uint64)
#undef compare_with_negation

TEST_F(alu_srcs_negative_equal_test, trivial_float)
{
   nir_def *two = nir_imm_float(&bld, 2.0f);
   nir_def *negative_two = nir_imm_float(&bld, -2.0f);

   nir_def *result = nir_fadd(&bld, two, negative_two);
   nir_alu_instr *instr = nir_instr_as_alu(result->parent_instr);

   ASSERT_NE((void *) 0, instr);
   EXPECT_TRUE(nir_alu_srcs_negative_equal(instr, instr, 0, 1));
   EXPECT_FALSE(nir_alu_srcs_negative_equal(instr, instr, 0, 0));
   EXPECT_FALSE(nir_alu_srcs_negative_equal(instr, instr, 1, 1));
}

TEST_F(alu_srcs_negative_equal_test, trivial_int)
{
   nir_def *two = nir_imm_int(&bld, 2);
   nir_def *negative_two = nir_imm_int(&bld, -2);

   nir_def *result = nir_iadd(&bld, two, negative_two);
   nir_alu_instr *instr = nir_instr_as_alu(result->parent_instr);

   ASSERT_NE((void *) 0, instr);
   EXPECT_TRUE(nir_alu_srcs_negative_equal(instr, instr, 0, 1));
   EXPECT_FALSE(nir_alu_srcs_negative_equal(instr, instr, 0, 0));
   EXPECT_FALSE(nir_alu_srcs_negative_equal(instr, instr, 1, 1));
}

TEST_F(alu_srcs_negative_equal_test, trivial_negation_float)
{
   /* Cannot just do the negation of a nir_load_const_instr because
    * nir_alu_srcs_negative_equal expects that constant folding will convert
    * fneg(2.0) to just -2.0.
    */
   nir_def *two = nir_imm_float(&bld, 2.0f);
   nir_def *two_plus_two = nir_fadd(&bld, two, two);
   nir_def *negation = nir_fneg(&bld, two_plus_two);

   nir_def *result = nir_fadd(&bld, two_plus_two, negation);

   nir_alu_instr *instr = nir_instr_as_alu(result->parent_instr);

   ASSERT_NE((void *) 0, instr);
   EXPECT_TRUE(nir_alu_srcs_negative_equal(instr, instr, 0, 1));
   EXPECT_FALSE(nir_alu_srcs_negative_equal(instr, instr, 0, 0));
   EXPECT_FALSE(nir_alu_srcs_negative_equal(instr, instr, 1, 1));
}

TEST_F(alu_srcs_negative_equal_test, trivial_negation_int)
{
   /* Cannot just do the negation of a nir_load_const_instr because
    * nir_alu_srcs_negative_equal expects that constant folding will convert
    * ineg(2) to just -2.
    */
   nir_def *two = nir_imm_int(&bld, 2);
   nir_def *two_plus_two = nir_iadd(&bld, two, two);
   nir_def *negation = nir_ineg(&bld, two_plus_two);

   nir_def *result = nir_iadd(&bld, two_plus_two, negation);

   nir_alu_instr *instr = nir_instr_as_alu(result->parent_instr);

   ASSERT_NE((void *) 0, instr);
   EXPECT_TRUE(nir_alu_srcs_negative_equal(instr, instr, 0, 1));
   EXPECT_FALSE(nir_alu_srcs_negative_equal(instr, instr, 0, 0));
   EXPECT_FALSE(nir_alu_srcs_negative_equal(instr, instr, 1, 1));
}

/* Compare an object with non-zero values to itself.  This should always be
 * false.
 */
#define compare_with_self(full_type)                                    \
TEST_F(alu_srcs_negative_equal_test, full_type ## _self)                \
{                                                                       \
   count_sequence(c1, full_type, 1);                                    \
   nir_def *a = nir_build_imm(&bld,                                 \
                                  NIR_MAX_VEC_COMPONENTS,               \
                                  nir_alu_type_get_type_size(full_type), \
                                  c1);                                  \
   nir_def *result;                                                 \
   if (nir_alu_type_get_base_type(full_type) == nir_type_float)         \
      result = nir_fadd(&bld, a, a);                                    \
   else                                                                 \
      result = nir_iadd(&bld, a, a);                                    \
   nir_alu_instr *instr = nir_instr_as_alu(result->parent_instr);       \
   ASSERT_NE((void *) 0, instr);                                        \
   EXPECT_FALSE(nir_alu_srcs_negative_equal(instr, instr, 0, 0));       \
   EXPECT_FALSE(nir_alu_srcs_negative_equal(instr, instr, 0, 1));       \
   EXPECT_FALSE(nir_alu_srcs_negative_equal(instr, instr, 1, 0));       \
   EXPECT_FALSE(nir_alu_srcs_negative_equal(instr, instr, 1, 1));       \
}

compare_with_self(nir_type_float16)
compare_with_self(nir_type_float32)
compare_with_self(nir_type_float64)
compare_with_self(nir_type_int8)
compare_with_self(nir_type_uint8)
compare_with_self(nir_type_int16)
compare_with_self(nir_type_uint16)
compare_with_self(nir_type_int32)
compare_with_self(nir_type_uint32)
compare_with_self(nir_type_int64)
compare_with_self(nir_type_uint64)

/* Compare an object with the negation of itself.  This should always be true.
 */
#define compare_with_negation(full_type)                                \
TEST_F(alu_srcs_negative_equal_test, full_type ## _trivially_true)      \
{                                                                       \
   count_sequence(c1, full_type, 1);                                    \
   negate(c2, c1, full_type, NIR_MAX_VEC_COMPONENTS);                   \
   nir_def *a = nir_build_imm(&bld,                                 \
                                  NIR_MAX_VEC_COMPONENTS,               \
                                  nir_alu_type_get_type_size(full_type), \
                                  c1);                                  \
   nir_def *b = nir_build_imm(&bld,                                 \
                                  NIR_MAX_VEC_COMPONENTS,               \
                                  nir_alu_type_get_type_size(full_type), \
                                  c2);                                  \
   nir_def *result;                                                 \
   if (nir_alu_type_get_base_type(full_type) == nir_type_float)         \
      result = nir_fadd(&bld, a, b);                                    \
   else                                                                 \
      result = nir_iadd(&bld, a, b);                                    \
   nir_alu_instr *instr = nir_instr_as_alu(result->parent_instr);       \
   ASSERT_NE((void *) 0, instr);                                        \
   EXPECT_FALSE(nir_alu_srcs_negative_equal(instr, instr, 0, 0));       \
   EXPECT_TRUE(nir_alu_srcs_negative_equal(instr, instr, 0, 1));        \
   EXPECT_TRUE(nir_alu_srcs_negative_equal(instr, instr, 1, 0));        \
   EXPECT_FALSE(nir_alu_srcs_negative_equal(instr, instr, 1, 1));       \
}

compare_with_negation(nir_type_float16)
compare_with_negation(nir_type_float32)
compare_with_negation(nir_type_float64)
compare_with_negation(nir_type_int8)
compare_with_negation(nir_type_uint8)
compare_with_negation(nir_type_int16)
compare_with_negation(nir_type_uint16)
compare_with_negation(nir_type_int32)
compare_with_negation(nir_type_uint32)
compare_with_negation(nir_type_int64)
compare_with_negation(nir_type_uint64)

TEST_F(alu_srcs_negative_equal_test, swizzle_scalar_to_vector)
{
   nir_def *v = nir_imm_vec2(&bld, 1.0, -1.0);
   const uint8_t s0[4] = { 0, 0, 0, 0 };
   const uint8_t s1[4] = { 1, 1, 1, 1 };

   /* We can't use nir_swizzle here because it inserts an extra MOV. */
   nir_alu_instr *instr = nir_alu_instr_create(bld.shader, nir_op_fadd);

   instr->src[0].src = nir_src_for_ssa(v);
   instr->src[1].src = nir_src_for_ssa(v);

   memcpy(&instr->src[0].swizzle, s0, sizeof(s0));
   memcpy(&instr->src[1].swizzle, s1, sizeof(s1));

   nir_builder_alu_instr_finish_and_insert(&bld, instr);

   EXPECT_TRUE(nir_alu_srcs_negative_equal(instr, instr, 0, 1));
}

static void
count_sequence(nir_const_value c[NIR_MAX_VEC_COMPONENTS],
               nir_alu_type full_type, int first)
{
   switch (full_type) {
   case nir_type_float16:
      for (unsigned i = 0; i < NIR_MAX_VEC_COMPONENTS; i++)
         c[i].u16 = _mesa_float_to_half(float(i + first));

      break;

   case nir_type_float32:
      for (unsigned i = 0; i < NIR_MAX_VEC_COMPONENTS; i++)
         c[i].f32 = float(i + first);

      break;

   case nir_type_float64:
      for (unsigned i = 0; i < NIR_MAX_VEC_COMPONENTS; i++)
         c[i].f64 = double(i + first);

      break;

   case nir_type_int8:
   case nir_type_uint8:
      for (unsigned i = 0; i < NIR_MAX_VEC_COMPONENTS; i++)
         c[i].i8 = i + first;

      break;

   case nir_type_int16:
   case nir_type_uint16:
      for (unsigned i = 0; i < NIR_MAX_VEC_COMPONENTS; i++)
         c[i].i16 = i + first;

      break;

   case nir_type_int32:
   case nir_type_uint32:
      for (unsigned i = 0; i < NIR_MAX_VEC_COMPONENTS; i++)
         c[i].i32 = i + first;

      break;

   case nir_type_int64:
   case nir_type_uint64:
      for (unsigned i = 0; i < NIR_MAX_VEC_COMPONENTS; i++)
         c[i].i64 = i + first;

      break;

   case nir_type_bool:
   default:
      unreachable("invalid base type");
   }
}

static void
negate(nir_const_value dst[NIR_MAX_VEC_COMPONENTS],
       const nir_const_value src[NIR_MAX_VEC_COMPONENTS],
       nir_alu_type full_type, unsigned components)
{
   switch (full_type) {
   case nir_type_float16:
      for (unsigned i = 0; i < components; i++)
         dst[i].u16 = _mesa_float_to_half(-_mesa_half_to_float(src[i].u16));

      break;

   case nir_type_float32:
      for (unsigned i = 0; i < components; i++)
         dst[i].f32 = -src[i].f32;

      break;

   case nir_type_float64:
      for (unsigned i = 0; i < components; i++)
         dst[i].f64 = -src[i].f64;

      break;

   case nir_type_int8:
   case nir_type_uint8:
      for (unsigned i = 0; i < components; i++)
         dst[i].i8 = -src[i].i8;

      break;

   case nir_type_int16:
   case nir_type_uint16:
      for (unsigned i = 0; i < components; i++)
         dst[i].i16 = -src[i].i16;

      break;

   case nir_type_int32:
   case nir_type_uint32:
      for (unsigned i = 0; i < components; i++)
         dst[i].i32 = -src[i].i32;

      break;

   case nir_type_int64:
   case nir_type_uint64:
      for (unsigned i = 0; i < components; i++)
         dst[i].i64 = -src[i].i64;

      break;

   case nir_type_bool:
   default:
      unreachable("invalid base type");
   }
}
