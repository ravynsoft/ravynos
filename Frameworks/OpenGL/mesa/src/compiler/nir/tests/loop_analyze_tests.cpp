/*
 * Copyright © 2022 Intel Corporation
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

class nir_loop_analyze_test : public ::testing::Test {
protected:
   nir_loop_analyze_test();
   ~nir_loop_analyze_test();

   nir_builder b;
};

nir_loop_analyze_test::nir_loop_analyze_test()
{
   glsl_type_singleton_init_or_ref();

   static nir_shader_compiler_options options = { };

   options.max_unroll_iterations = 32;

   b = nir_builder_init_simple_shader(MESA_SHADER_VERTEX, &options,
                                      "loop analyze");
}

nir_loop_analyze_test::~nir_loop_analyze_test()
{
   ralloc_free(b.shader);
   glsl_type_singleton_decref();
}

struct loop_builder_param {
   uint32_t init_value;
   uint32_t cond_value;
   uint32_t incr_value;
   nir_def *(*cond_instr)(nir_builder *,
                              nir_def *,
                              nir_def *);
   nir_def *(*incr_instr)(nir_builder *,
                              nir_def *,
                              nir_def *);
};

static nir_loop *
loop_builder(nir_builder *b, loop_builder_param p)
{
   /* Create IR:
    *
    *    auto i = init_value;
    *    while (true) {
    *       if (cond_instr(i, cond_value))
    *          break;
    *
    *       i = incr_instr(i, incr_value);
    *    }
    */
   nir_def *ssa_0 = nir_imm_int(b, p.init_value);
   nir_def *ssa_1 = nir_imm_int(b, p.cond_value);
   nir_def *ssa_2 = nir_imm_int(b, p.incr_value);

   nir_phi_instr *const phi = nir_phi_instr_create(b->shader);

   nir_loop *loop = nir_push_loop(b);
   {
      nir_def_init(&phi->instr, &phi->def, ssa_0->num_components,
                   ssa_0->bit_size);

      nir_phi_instr_add_src(phi, ssa_0->parent_instr->block, ssa_0);

      nir_def *ssa_5 = &phi->def;
      nir_def *ssa_3 = p.cond_instr(b, ssa_5, ssa_1);

      nir_if *nif = nir_push_if(b, ssa_3);
      {
         nir_jump_instr *jump = nir_jump_instr_create(b->shader, nir_jump_break);
         nir_builder_instr_insert(b, &jump->instr);
      }
      nir_pop_if(b, nif);

      nir_def *ssa_4 = p.incr_instr(b, ssa_5, ssa_2);

      nir_phi_instr_add_src(phi, ssa_4->parent_instr->block, ssa_4);
   }
   nir_pop_loop(b, loop);

   b->cursor = nir_before_block(nir_loop_first_block(loop));
   nir_builder_instr_insert(b, &phi->instr);

   return loop;
}

struct loop_builder_invert_param {
   uint32_t init_value;
   uint32_t incr_value;
   uint32_t cond_value;
   nir_def *(*cond_instr)(nir_builder *,
                              nir_def *,
                              nir_def *);
   nir_def *(*incr_instr)(nir_builder *,
                              nir_def *,
                              nir_def *);
};

/**
 * Build an "inverted" loop.
 *
 * Like \c loop_builder, but the exit condition for the loop is at the bottom
 * of the loop instead of the top. In compiler literature, the optimization
 * that moves the exit condition from the top to the bottom is called "loop
 * inversion," hence the name of this function.
 */
static nir_loop *
loop_builder_invert(nir_builder *b, loop_builder_invert_param p)
{
   /* Create IR:
    *
    *    auto i = init_value;
    *    while (true) {
    *       i = incr_instr(i, incr_value);
    *
    *       if (cond_instr(i, cond_value))
    *          break;
    *    }
    */
   nir_def *ssa_0 = nir_imm_int(b, p.init_value);
   nir_def *ssa_1 = nir_imm_int(b, p.incr_value);
   nir_def *ssa_2 = nir_imm_int(b, p.cond_value);

   nir_phi_instr *const phi = nir_phi_instr_create(b->shader);

   nir_loop *loop = nir_push_loop(b);
   {
      nir_def_init(&phi->instr, &phi->def, ssa_0->num_components,
                   ssa_0->bit_size);

      nir_phi_instr_add_src(phi, ssa_0->parent_instr->block, ssa_0);

      nir_def *ssa_5 = &phi->def;

      nir_def *ssa_3 = p.incr_instr(b, ssa_5, ssa_1);

      nir_def *ssa_4 = p.cond_instr(b, ssa_3, ssa_2);

      nir_if *nif = nir_push_if(b, ssa_4);
      {
         nir_jump_instr *jump = nir_jump_instr_create(b->shader, nir_jump_break);
         nir_builder_instr_insert(b, &jump->instr);
      }
      nir_pop_if(b, nif);

      nir_phi_instr_add_src(phi, nir_cursor_current_block(b->cursor), ssa_3);
   }
   nir_pop_loop(b, loop);

   b->cursor = nir_before_block(nir_loop_first_block(loop));
   nir_builder_instr_insert(b, &phi->instr);

   return loop;
}

TEST_F(nir_loop_analyze_test, one_iteration_fneu)
{
   /* Create IR:
    *
    *    float i = uintBitsToFloat(0xe7000000);
    *    while (true) {
    *       if (i != uintBitsToFloat(0xe7000000))
    *          break;
    *
    *       i = i + uintBitsToFloat(0x5b000000);
    *    }
    *
    * Going towards smaller magnitude (i.e., adding a small positive value to
    * a large negative value) requires a smaller delta to make a difference
    * than going towards a larger magnitude. For this reason, ssa_0 + ssa_1 !=
    * ssa_0, but ssa_0 - ssa_1 == ssa_0. Math class is tough.
    */
   nir_loop *loop =
      loop_builder(&b, {.init_value = 0xe7000000, .cond_value = 0xe7000000,
                        .incr_value = 0x5b000000,
                        .cond_instr = nir_fneu, .incr_instr = nir_fadd});

   /* At this point, we should have:
    *
    * impl main {
    *         block block_0:
    *         // preds:
    *         vec1 32 ssa_0 = load_const (0xe7000000 = -604462909807314587353088.0)
    *         vec1 32 ssa_1 = load_const (0xe7000000 = -604462909807314587353088.0)
    *         vec1 32 ssa_2 = load_const (0x5b000000 = 36028797018963968.0)
    *         // succs: block_1
    *         loop {
    *                 block block_1:
    *                 // preds: block_0 block_4
    *                 vec1 32 ssa_5 = phi block_0: ssa_0, block_4: ssa_4
    *                 vec1  1 ssa_3 = fneu ssa_5, ssa_1
    *                 // succs: block_2 block_3
    *                 if ssa_3 {
    *                         block block_2:
    *                         // preds: block_1
    *                         break
    *                         // succs: block_5
    *                 } else {
    *                         block block_3:
    *                         // preds: block_1
    *                         // succs: block_4
    *                 }
    *                 block block_4:
    *                 // preds: block_3
    *                 vec1 32 ssa_4 = fadd ssa_5, ssa_2
    *                 // succs: block_1
    *         }
    *         block block_5:
    *         // preds: block_2
    *         // succs: block_6
    *         block block_6:
    * }
    */
   nir_validate_shader(b.shader, "input");

   nir_loop_analyze_impl(b.impl, nir_var_all, false);

   ASSERT_NE((void *)0, loop->info);
   EXPECT_EQ(1, loop->info->max_trip_count);
   EXPECT_TRUE(loop->info->exact_trip_count_known);

   /* Loop should have an induction variable for ssa_5 and ssa_4. */
   EXPECT_EQ(2, loop->info->num_induction_vars);
   ASSERT_NE((void *)0, loop->info->induction_vars);

   /* The def field should not be NULL. The init_src field should point to a
    * load_const. The update_src field should point to a load_const.
    */
   const nir_loop_induction_variable *const ivars = loop->info->induction_vars;

   for (unsigned i = 0; i < loop->info->num_induction_vars; i++) {
      EXPECT_NE((void *)0, ivars[i].def);
      ASSERT_NE((void *)0, ivars[i].init_src);
      EXPECT_TRUE(nir_src_is_const(*ivars[i].init_src));
      ASSERT_NE((void *)0, ivars[i].update_src);
      EXPECT_TRUE(nir_src_is_const(ivars[i].update_src->src));
   }
}

#define COMPARE_REVERSE(comp)                                           \
   static nir_def *                                                 \
   nir_ ## comp ## _rev(nir_builder *b, nir_def *x, nir_def *y) \
   {                                                                    \
      return nir_ ## comp (b, y, x);                                    \
   }

COMPARE_REVERSE(ilt)
COMPARE_REVERSE(ige)
COMPARE_REVERSE(ult)
COMPARE_REVERSE(uge)
COMPARE_REVERSE(ishl)

#define INOT_COMPARE(comp)                                              \
   static nir_def *                                                 \
   nir_inot_ ## comp (nir_builder *b, nir_def *x, nir_def *y)   \
   {                                                                    \
      return nir_inot(b, nir_ ## comp (b, x, y));                       \
   }

INOT_COMPARE(ilt_rev)
INOT_COMPARE(ine)
INOT_COMPARE(uge_rev)

#define CMP_MIN(cmp, min)                                               \
   static nir_def *nir_##cmp##_##min(nir_builder *b, nir_def *counter, nir_def *limit) \
   {                                                                    \
      nir_def *unk = nir_load_vertex_id(b);                             \
      return nir_##cmp(b, counter, nir_##min(b, limit, unk));           \
   }

#define CMP_MIN_REV(cmp, min)                                           \
   static nir_def *nir_##cmp##_##min##_rev(nir_builder *b, nir_def *counter, nir_def *limit) \
   {                                                                    \
      nir_def *unk = nir_load_vertex_id(b);                             \
      return nir_##cmp(b, nir_##min(b, limit, unk), counter);           \
   }

CMP_MIN(ige, imin)
CMP_MIN_REV(ige, imin)
CMP_MIN(uge, umin)
CMP_MIN(ige, fmin)
CMP_MIN(uge, imin)
CMP_MIN(ilt, imin)
CMP_MIN(ilt, imax)
CMP_MIN_REV(ilt, imin)
INOT_COMPARE(ilt_imin_rev)

#define KNOWN_COUNT_TEST(_init_value, _cond_value, _incr_value, cond, incr, count) \
   TEST_F(nir_loop_analyze_test, incr ## _ ## cond ## _known_count_ ## count)    \
   {                                                                    \
      nir_loop *loop =                                                  \
         loop_builder(&b, {.init_value = _init_value,                   \
                           .cond_value = _cond_value,                   \
                           .incr_value = _incr_value,                   \
                           .cond_instr = nir_ ## cond,                  \
                           .incr_instr = nir_ ## incr});                \
                                                                        \
      nir_validate_shader(b.shader, "input");                           \
                                                                        \
      nir_loop_analyze_impl(b.impl, nir_var_all, false);                \
                                                                        \
      ASSERT_NE((void *)0, loop->info);                                 \
      EXPECT_NE((void *)0, loop->info->limiting_terminator);            \
      EXPECT_EQ(count, loop->info->max_trip_count);                     \
      EXPECT_TRUE(loop->info->exact_trip_count_known);                  \
                                                                        \
      EXPECT_EQ(2, loop->info->num_induction_vars);                     \
      ASSERT_NE((void *)0, loop->info->induction_vars);                 \
                                                                        \
      const nir_loop_induction_variable *const ivars =                  \
         loop->info->induction_vars;                                    \
                                                                        \
      for (unsigned i = 0; i < loop->info->num_induction_vars; i++) {   \
         EXPECT_NE((void *)0, ivars[i].def);                            \
         ASSERT_NE((void *)0, ivars[i].init_src);                       \
         EXPECT_TRUE(nir_src_is_const(*ivars[i].init_src));             \
         ASSERT_NE((void *)0, ivars[i].update_src);                     \
         EXPECT_TRUE(nir_src_is_const(ivars[i].update_src->src));       \
      }                                                                 \
   }

#define INEXACT_COUNT_TEST(_init_value, _cond_value, _incr_value, cond, incr, count) \
   TEST_F(nir_loop_analyze_test, incr ## _ ## cond ## _inexact_count_ ## count)    \
   {                                                                    \
      nir_loop *loop =                                                  \
         loop_builder(&b, {.init_value = _init_value,                   \
                           .cond_value = _cond_value,                   \
                           .incr_value = _incr_value,                   \
                           .cond_instr = nir_ ## cond,                  \
                           .incr_instr = nir_ ## incr});                \
                                                                        \
      nir_validate_shader(b.shader, "input");                           \
                                                                        \
      nir_loop_analyze_impl(b.impl, nir_var_all, false);                \
                                                                        \
      ASSERT_NE((void *)0, loop->info);                                 \
      EXPECT_NE((void *)0, loop->info->limiting_terminator);            \
      EXPECT_EQ(count, loop->info->max_trip_count);                     \
      EXPECT_FALSE(loop->info->exact_trip_count_known);                 \
                                                                        \
      EXPECT_EQ(2, loop->info->num_induction_vars);                     \
      ASSERT_NE((void *)0, loop->info->induction_vars);                 \
                                                                        \
      const nir_loop_induction_variable *const ivars =                  \
         loop->info->induction_vars;                                    \
                                                                        \
      for (unsigned i = 0; i < loop->info->num_induction_vars; i++) {   \
         EXPECT_NE((void *)0, ivars[i].def);                            \
         ASSERT_NE((void *)0, ivars[i].init_src);                       \
         EXPECT_TRUE(nir_src_is_const(*ivars[i].init_src));             \
         ASSERT_NE((void *)0, ivars[i].update_src);                     \
         EXPECT_TRUE(nir_src_is_const(ivars[i].update_src->src));       \
      }                                                                 \
   }

#define UNKNOWN_COUNT_TEST(_init_value, _cond_value, _incr_value, cond, incr) \
   TEST_F(nir_loop_analyze_test, incr ## _ ## cond ## _unknown_count)   \
   {                                                                    \
      nir_loop *loop =                                                  \
         loop_builder(&b, {.init_value = _init_value,                   \
                           .cond_value = _cond_value,                   \
                           .incr_value = _incr_value,                   \
                           .cond_instr = nir_ ## cond,                  \
                           .incr_instr = nir_ ## incr});                \
                                                                        \
      nir_validate_shader(b.shader, "input");                           \
                                                                        \
      nir_loop_analyze_impl(b.impl, nir_var_all, false);                \
                                                                        \
      ASSERT_NE((void *)0, loop->info);                                 \
      EXPECT_EQ((void *)0, loop->info->limiting_terminator);            \
      EXPECT_EQ(0, loop->info->max_trip_count);                         \
      EXPECT_FALSE(loop->info->exact_trip_count_known);                 \
   }

#define INFINITE_LOOP_UNKNOWN_COUNT_TEST(_init_value, _cond_value, _incr_value, cond, incr) \
   TEST_F(nir_loop_analyze_test, incr ## _ ## cond ## _infinite_loop_unknown_count)   \
   {                                                                    \
      nir_loop *loop =                                                  \
         loop_builder(&b, {.init_value = _init_value,                   \
                           .cond_value = _cond_value,                   \
                           .incr_value = _incr_value,                   \
                           .cond_instr = nir_ ## cond,                  \
                           .incr_instr = nir_ ## incr});                \
                                                                        \
      nir_validate_shader(b.shader, "input");                           \
                                                                        \
      nir_loop_analyze_impl(b.impl, nir_var_all, false);                \
                                                                        \
      ASSERT_NE((void *)0, loop->info);                                 \
      EXPECT_EQ((void *)0, loop->info->limiting_terminator);            \
      EXPECT_EQ(0, loop->info->max_trip_count);                         \
      EXPECT_FALSE(loop->info->exact_trip_count_known);                 \
   }

#define KNOWN_COUNT_TEST_INVERT(_init_value, _incr_value, _cond_value, cond, incr, count) \
   TEST_F(nir_loop_analyze_test, incr ## _ ## cond ## _known_count_invert_ ## count)   \
   {                                                                    \
      nir_loop *loop =                                                  \
         loop_builder_invert(&b, {.init_value = _init_value,            \
                                  .incr_value = _incr_value,            \
                                  .cond_value = _cond_value,            \
                                  .cond_instr = nir_ ## cond,           \
                                  .incr_instr = nir_ ## incr});         \
                                                                        \
      nir_validate_shader(b.shader, "input");                           \
                                                                        \
      nir_loop_analyze_impl(b.impl, nir_var_all, false);                \
                                                                        \
      ASSERT_NE((void *)0, loop->info);                                 \
      EXPECT_NE((void *)0, loop->info->limiting_terminator);            \
      EXPECT_EQ(count, loop->info->max_trip_count);                     \
      EXPECT_TRUE(loop->info->exact_trip_count_known);                  \
                                                                        \
      EXPECT_EQ(2, loop->info->num_induction_vars);                     \
      ASSERT_NE((void *)0, loop->info->induction_vars);                 \
                                                                        \
      const nir_loop_induction_variable *const ivars =                  \
         loop->info->induction_vars;                                    \
                                                                        \
      for (unsigned i = 0; i < loop->info->num_induction_vars; i++) {   \
         EXPECT_NE((void *)0, ivars[i].def);                            \
         ASSERT_NE((void *)0, ivars[i].init_src);                       \
         EXPECT_TRUE(nir_src_is_const(*ivars[i].init_src));             \
         ASSERT_NE((void *)0, ivars[i].update_src);                     \
         EXPECT_TRUE(nir_src_is_const(ivars[i].update_src->src));       \
      }                                                                 \
   }

#define UNKNOWN_COUNT_TEST_INVERT(_init_value, _incr_value, _cond_value, cond, incr) \
   TEST_F(nir_loop_analyze_test, incr ## _ ## cond ## _unknown_count_invert)   \
   {                                                                    \
      nir_loop *loop =                                                  \
         loop_builder_invert(&b, {.init_value = _init_value,            \
                                  .incr_value = _incr_value,            \
                                  .cond_value = _cond_value,            \
                                  .cond_instr = nir_ ## cond,           \
                                  .incr_instr = nir_ ## incr});         \
                                                                        \
      nir_validate_shader(b.shader, "input");                           \
                                                                        \
      nir_loop_analyze_impl(b.impl, nir_var_all, false);                \
                                                                        \
      ASSERT_NE((void *)0, loop->info);                                 \
      EXPECT_EQ((void *)0, loop->info->limiting_terminator);            \
      EXPECT_EQ(0, loop->info->max_trip_count);                         \
      EXPECT_FALSE(loop->info->exact_trip_count_known);                 \
   }

#define INFINITE_LOOP_UNKNOWN_COUNT_TEST_INVERT(_init_value, _incr_value, _cond_value, cond, incr) \
   TEST_F(nir_loop_analyze_test, incr ## _ ## cond ## _infinite_loop_unknown_count_invert)   \
   {                                                                    \
      nir_loop *loop =                                                  \
         loop_builder_invert(&b, {.init_value = _init_value,            \
                                  .incr_value = _incr_value,            \
                                  .cond_value = _cond_value,            \
                                  .cond_instr = nir_ ## cond,           \
                                  .incr_instr = nir_ ## incr});         \
                                                                        \
      nir_validate_shader(b.shader, "input");                           \
                                                                        \
      nir_loop_analyze_impl(b.impl, nir_var_all, false);                \
                                                                        \
      ASSERT_NE((void *)0, loop->info);                                 \
      EXPECT_EQ((void *)0, loop->info->limiting_terminator);            \
      EXPECT_EQ(0, loop->info->max_trip_count);                         \
      EXPECT_FALSE(loop->info->exact_trip_count_known);                 \
   }

/*    float i = 0.0;
 *    while (true) {
 *       if (i == 0.9)
 *          break;
 *
 *       i = i + 0.2;
 *    }
 */
INFINITE_LOOP_UNKNOWN_COUNT_TEST(0x00000000, 0x3e4ccccd, 0x3f666666, feq, fadd)

/*    uint i = 1;
 *    while (true) {
 *       if (i != 0)
 *          break;
 *
 *       i++;
 *    }
 *
 * This loop should have an iteration count of zero.  See also
 * https://gitlab.freedesktop.org/mesa/mesa/-/merge_requests/19732#note_1648999
 */
KNOWN_COUNT_TEST(0x00000001, 0x00000000, 0x00000001, ine, iadd, 0)

/*    uint i = 0;
 *    while (true) {
 *       if (i >= 1)
 *          break;
 *
 *       i++;
 *    }
 */
KNOWN_COUNT_TEST(0x00000000, 0x00000001, 0x00000001, uge, iadd, 1)

/*    uint i = 0;
 *    while (true) {
 *       if (i != 0)
 *          break;
 *
 *       i++;
 *    }
 */
KNOWN_COUNT_TEST(0x00000000, 0x00000000, 0x00000001, ine, iadd, 1)

/*    uint i = 0;
 *    while (true) {
 *       if (!(i != 6))
 *          break;
 *
 *       i++;
 *    }
 */
KNOWN_COUNT_TEST(0x00000000, 0x00000006, 0x00000001, inot_ine, iadd, 6)

/*    uint i = 0;
 *    while (true) {
 *       i++;
 *
 *       if (!(i != 8))
 *          break;
 *    }
 */
KNOWN_COUNT_TEST_INVERT(0x00000000, 0x00000001, 0x00000008, inot_ine, iadd, 7)

/*    uint i = 0;
 *    while (true) {
 *       if (i == 1)
 *          break;
 *
 *       i++;
 *    }
 */
KNOWN_COUNT_TEST(0x00000000, 0x00000001, 0x00000001, ieq, iadd, 1)

/*    uint i = 0;
 *    while (true) {
 *       if (i == 6)
 *          break;
 *
 *       i++;
 *    }
 */
KNOWN_COUNT_TEST(0x00000000, 0x00000006, 0x00000001, ieq, iadd, 6)

/*    uint i = 0;
 *    while (true) {
 *       i++;
 *
 *       if (i == 6)
 *          break;
 *    }
 */
KNOWN_COUNT_TEST_INVERT(0x00000000, 0x00000001, 0x00000006, ieq, iadd, 5)

/*    float i = 0.0;
 *    while (true) {
 *       if (i != 0.0)
 *          break;
 *
 *       i = i + 1.0;
 *    }
 */
KNOWN_COUNT_TEST(0x00000000, 0x00000000, 0x3f800000, fneu, fadd, 1)

/*    uint i = 0;
 *    while (true) {
 *       i++;
 *
 *       if (i != 0)
 *          break;
 *    }
 */
KNOWN_COUNT_TEST_INVERT(0x00000000, 0x00000001, 0x00000000, ine, iadd, 0)

/*    int i = 0;
 *    while (true) {
 *       i++;
 *
 *       if (i >= 6)
 *          break;
 *    }
 */
KNOWN_COUNT_TEST_INVERT(0x00000000, 0x00000001, 0x00000006, ige, iadd, 5)

/*    uint i = 10;
 *    while (true) {
 *       if (!(5 < i))
 *          break;
 *
 *       i += -1;
 *    }
 */
KNOWN_COUNT_TEST(0x0000000a, 0x00000005, 0xffffffff, inot_ilt_rev, iadd, 5)

/*    int i = 10;
 *    while (true) {
 *       if (!(imin(vertex_id, 5) < i))
 *          break;
 *
 *       i += -1;
 *    }
 */
UNKNOWN_COUNT_TEST(0x0000000a, 0x00000005, 0xffffffff, inot_ilt_imin_rev, iadd)

/*    uint i = 0;
 *    while (true) {
 *       if (!(0 >= i))
 *          break;
 *
 *       i += 1;
 *    }
 */
KNOWN_COUNT_TEST(0x00000000, 0x00000000, 0x00000001, inot_uge_rev, iadd, 1)

/*    uint i = 0;
 *    while (true) {
 *       if (i != 0)
 *          break;
 *
 *       i >>= 1;
 *    }
 */
INFINITE_LOOP_UNKNOWN_COUNT_TEST(0x00000000, 0x00000000, 0x00000001, ine, ushr)

/*    uint i = 0x80000000;
 *    while (true) {
 *       if (i == 0xDEADBEEF)
 *          break;
 *
 *       i >>= 1;
 *    }
 */
INFINITE_LOOP_UNKNOWN_COUNT_TEST(0x80000000, 0xDEADBEEF, 0x00000001, ieq, ushr)

/* There is no ult / ushr infinite loop test because, aside from the
 * contradiction ult(x, 0), there isn't a way to construct such a loop with
 * the loop induction variable on the left side of the comparison.
 */
/* INFINITE_LOOP_UNKNOWN_COUNT_TEST(0xBADDC0DE, 0xBADDC0DE, 0xBADDC0DE, ult, ushr) */

/*    uint i = 0x40000000;
 *    while (true) {
 *       if (0x43210000 < i)
 *          break;
 *
 *       i >>= 1;
 *    }
 */
INFINITE_LOOP_UNKNOWN_COUNT_TEST(0x40000000, 0x43210000, 0x00000001, ult_rev, ushr)

/*    uint i = 0x40000000;
 *    while (true) {
 *       if (i >= 0x80000000)
 *          break;
 *
 *       i >>= 1;
 *    }
 */
INFINITE_LOOP_UNKNOWN_COUNT_TEST(0x40000000, 0x80000000, 0x00000001, uge, ushr)

/* There is no uge_rev / ushr infinite loop test because I could not think of
 * a way to construct one.
 */
/* INFINITE_LOOP_UNKNOWN_COUNT_TEST(0xBADDC0DE, 0xBADDC0DE, 0xBADDC0DE, uge_rev, ushr) */

/*    uint i = 0x00001234;
 *    while (true) {
 *       i >>= 16;
 *
 *       if (i != 0)
 *          break;
 *    }
 */
INFINITE_LOOP_UNKNOWN_COUNT_TEST_INVERT(0x00001234, 0x00000010, 0x00000000, ine, ushr)

/*    uint i = 0x12345678;
 *    while (true) {
 *       i >>= 3;
 *
 *       if (i == 0x048d159e)
 *          break;
 *    }
 */
INFINITE_LOOP_UNKNOWN_COUNT_TEST_INVERT(0x12345678, 0x00000003, 0x048d159e, ieq, ushr)

/* There is no ult / ushr infinite inverted loop test because, aside from the
 * contradiction ult(x, 0), there isn't a way to construct such a loop with
 * the loop induction variable on the left side of the comparison.
 */
/* INFINITE_LOOP_UNKNOWN_COUNT_TEST_INVERT(0xBADDC0DE, 0xBADDC0DE, 0xBADDC0DE, ult, ushr) */

/*    uint i = 0x87654321;
 *    while (true) {
 *       i >>= 2;
 *
 *       if (0x77777777 < i)
 *          break;
 *    }
 */
INFINITE_LOOP_UNKNOWN_COUNT_TEST_INVERT(0x87654321, 0x00000002, 0x77777777, ult_rev, ushr)

/*    uint i = 0x80000000;
 *    while (true) {
 *       i >>= 3;
 *
 *       if (i >= 0x40000000)
 *          break;
 *    }
 */
INFINITE_LOOP_UNKNOWN_COUNT_TEST_INVERT(0x80000000, 0x00000003, 0x40000000, uge, ushr)

/* There is no uge_rev / ushr infinite loop test because I could not think of
 * a way to construct one.
 */
/* INFINITE_LOOP_UNKNOWN_COUNT_TEST_INVERT(0xBADDC0DE, 0xBADDC0DE, 0xBADDC0DE, uge_rev, ushr) */

/*    uint i = 0x80000000;
 *    while (true) {
 *       if (i != 0x80000000)
 *          break;
 *
 *       i >>= 1;
 *    }
 */
KNOWN_COUNT_TEST(0x80000000, 0x80000000, 0x00000001, ine, ushr, 1)

/*    uint i = 0x80000000;
 *    while (true) {
 *       if (i == 0)
 *          break;
 *
 *       i >>= 1;
 *    }
 */
KNOWN_COUNT_TEST(0x80000000, 0x00000000, 0x00000001, ieq, ushr, 32)

/*    uint i = 0x80000000;
 *    while (true) {
 *       if (i < 2)
 *          break;
 *
 *       i >>= 1;
 *    }
 */
KNOWN_COUNT_TEST(0x80000000, 0x00000002, 0x00000001, ult, ushr, 31)

/*    uint i = 0x80000000;
 *    while (true) {
 *       if (2 < i)
 *          break;
 *
 *       i >>= 1;
 *    }
 */
KNOWN_COUNT_TEST(0x80000000, 0x00000002, 0x00000001, ult_rev, ushr, 0)

/*    uint i = 0x80000000;
 *    while (true) {
 *       if (i >= 0x80000000)
 *          break;
 *
 *       i >>= 1;
 *    }
 */
KNOWN_COUNT_TEST(0x80000000, 0x80000000, 0x00000001, uge, ushr, 0)

/*    uint i = 0x80000000;
 *    while (true) {
 *       if (0x00008000 >= i)
 *          break;
 *
 *       i >>= 1;
 *    }
 */
KNOWN_COUNT_TEST(0x80000000, 0x00008000, 0x00000001, uge_rev, ushr, 16)

/*    uint i = 0x80000000;
 *    while (true) {
 *       i >>= 1;
 *
 *       if (i != 0x80000000)
 *          break;
 *    }
 */
KNOWN_COUNT_TEST_INVERT(0x80000000, 0x00000001, 0x80000000, ine, ushr, 0)

/*    uint i = 0x80000000;
 *    while (true) {
 *       i >>= 1;
 *
 *       if (i == 0x00000000)
 *          break;
 *    }
 */
KNOWN_COUNT_TEST_INVERT(0x80000000, 0x00000001, 0x00000000, ieq, ushr, 31)

/*    uint i = 0x80000000;
 *    while (true) {
 *       i >>= 1;
 *
 *       if (i < 0x80000000)
 *          break;
 *    }
 */
KNOWN_COUNT_TEST_INVERT(0x80000000, 0x00000001, 0x80000000, ult, ushr, 0)

/*    uint i = 0xAAAAAAAA;
 *    while (true) {
 *       i >>= 1;
 *
 *       if (0x08000000 < i)
 *          break;
 *    }
 */
KNOWN_COUNT_TEST_INVERT(0xAAAAAAAA, 0x00000001, 0x08000000, ult_rev, ushr, 0)

/*    uint i = 0x80000000;
 *    while (true) {
 *       i >>= 1;
 *
 *       if (i >= 0x00000000)
 *          break;
 *    }
 */
KNOWN_COUNT_TEST_INVERT(0x80000000, 0x00000001, 0x00000000, uge, ushr, 0)

/*    uint i = 0x80000000;
 *    while (true) {
 *       i >>= 1;
 *
 *       if (0x00000008 >= i)
 *          break;
 *    }
 */
KNOWN_COUNT_TEST_INVERT(0x80000000, 0x00000001, 0x00000008, uge_rev, ushr, 27)

/*    int i = 0xffffffff;
 *    while (true) {
 *       if (i != 0xffffffff)
 *          break;
 *
 *       i >>= 1;
 *    }
 */
INFINITE_LOOP_UNKNOWN_COUNT_TEST(0xffffffff, 0xffffffff, 0x00000001, ine, ishr)

/*    int i = 0x80000000;
 *    while (true) {
 *       if (i == 0)
 *          break;
 *
 *       i >>= 1;
 *    }
 */
INFINITE_LOOP_UNKNOWN_COUNT_TEST(0x80000000, 0x00000000, 0x00000001, ieq, ishr)

/*    int i = 0x7fffffff;
 *    while (true) {
 *       if (i < 0)
 *          break;
 *
 *       i >>= 1;
 *    }
 */
INFINITE_LOOP_UNKNOWN_COUNT_TEST(0x7fffffff, 0x00000000, 0x00000001, ilt, ishr)

/*    int i = 0x80000000;
 *    while (true) {
 *       if (0 < i)
 *          break;
 *
 *       i >>= 1;
 *    }
 */
INFINITE_LOOP_UNKNOWN_COUNT_TEST(0x80000000, 0x00000000, 0x00000001, ilt_rev, ishr)

/*    int i = 0x80000000;
 *    while (true) {
 *       if (i >= 0)
 *          break;
 *
 *       i >>= 1;
 *    }
 */
INFINITE_LOOP_UNKNOWN_COUNT_TEST(0x80000000, 0x00000000, 0x00000001, ige, ishr)

/*    int i = 0x76543210;
 *    while (true) {
 *       if (-1 >= i)
 *          break;
 *
 *       i >>= 1;
 *    }
 */
INFINITE_LOOP_UNKNOWN_COUNT_TEST(0x76543210, 0xffffffff, 0x00000001, ige_rev, ishr)

/*    int i = 0xffffffff;
 *    while (true) {
 *       i >>= 1;
 *
 *       if (i != 0xffffffff)
 *          break;
 *    }
 */
INFINITE_LOOP_UNKNOWN_COUNT_TEST_INVERT(0xffffffff, 0x00000001, 0xffffffff, ine, ishr)

/*    int i = 0xffffffff;
 *    while (true) {
 *       i >>= 1;
 *
 *       if (i == 0)
 *          break;
 *    }
 */
INFINITE_LOOP_UNKNOWN_COUNT_TEST_INVERT(0xffffffff, 0x00000001, 0x00000000, ieq, ishr)

/*    int i = 0x7fffffff;
 *    while (true) {
 *       i >>= 1;
 *
 *       if (i < 0)
 *          break;
 *    }
 */
INFINITE_LOOP_UNKNOWN_COUNT_TEST_INVERT(0x7fffffff, 0x00000001, 0x00000000, ilt, ishr)

/*    int i = 0x80000000;
 *    while (true) {
 *       i >>= 1;
 *
 *       if (1 < i)
 *          break;
 *    }
 */
INFINITE_LOOP_UNKNOWN_COUNT_TEST_INVERT(0x80000000, 0x00000001, 0x00000001, ilt_rev, ishr)

/*    int i = 0x80000000;
 *    while (true) {
 *       i >>= 1;
 *
 *       if (i >= 0)
 *          break;
 *    }
 */
INFINITE_LOOP_UNKNOWN_COUNT_TEST_INVERT(0x80000000, 0x00000001, 0x00000000, ige, ishr)

/*    int i = 0x76543210;
 *    while (true) {
 *       i >>= 7;
 *
 *       if (-1 >= i)
 *          break;
 *    }
 */
INFINITE_LOOP_UNKNOWN_COUNT_TEST_INVERT(0x76543210, 0x00000007, 0xffffffff, ige_rev, ishr)

/*    int i = 0x7fffffff;
 *    while (true) {
 *       if (i != 0)
 *          break;
 *
 *       i >>= 1;
 *    }
 */
KNOWN_COUNT_TEST(0x7fffffff, 0x00000000, 0x00000001, ine, ishr, 0)

/*    int i = 0x40000000;
 *    while (true) {
 *       if (i == 1)
 *          break;
 *
 *       i >>= 1;
 *    }
 */
KNOWN_COUNT_TEST(0x40000000, 0x00000001, 0x00000001, ieq, ishr, 30)

/*    int i = 0x7fffffff;
 *    while (true) {
 *       if (i < 1)
 *          break;
 *
 *       i >>= 1;
 *    }
 */
KNOWN_COUNT_TEST(0x7fffffff, 0x00000001, 0x00000001, ilt, ishr, 31)

/*    int i = 0x80000000;
 *    while (true) {
 *       if (0xffff0000 < i)
 *          break;
 *
 *       i >>= 1;
 *    }
 */
KNOWN_COUNT_TEST(0x80000000, 0xffff0000, 0x00000001, ilt_rev, ishr, 16)

/*    int i = 0x80000000;
 *    while (true) {
 *       if (i >= -1)
 *          break;
 *
 *       i >>= 1;
 *    }
 */
KNOWN_COUNT_TEST(0x80000000, 0xffffffff, 0x00000001, ige, ishr, 31)

/*    int i = 0x12345678;
 *    while (true) {
 *       if (1 >= i)
 *          break;
 *
 *       i >>= 4;
 *    }
 */
KNOWN_COUNT_TEST(0x12345678, 0x00000001, 0x00000004, ige_rev, ishr, 7)

/*    int i = 0x7fffffff;
 *    while (true) {
 *       i >>= 1;
 *
 *       if (i != 0)
 *          break;
 *    }
 */
KNOWN_COUNT_TEST_INVERT(0x7fffffff, 0x00000001, 0x00000000, ine, ishr, 0)

/*    int i = 0x7fffffff;
 *    while (true) {
 *       i >>= 1;
 *
 *       if (i == 0)
 *          break;
 *    }
 */
KNOWN_COUNT_TEST_INVERT(0x7fffffff, 0x00000001, 0x00000000, ieq, ishr, 30)

/*    int i = 0x7fffffff;
 *    while (true) {
 *       i >>= 1;
 *
 *       if (i < 1)
 *          break;
 *    }
 */
KNOWN_COUNT_TEST_INVERT(0x7fffffff, 0x00000001, 0x00000001, ilt, ishr, 30)

/*    int i = 0x80000000;
 *    while (true) {
 *       i >>= 1;
 *
 *       if (-2 < i)
 *          break;
 *    }
 */
KNOWN_COUNT_TEST_INVERT(0x80000000, 0x00000001, 0xfffffffe, ilt_rev, ishr, 30)

/*    int i = 0xbfffffff;
 *    while (true) {
 *       i >>= 1;
 *
 *       if (i >= -2)
 *          break;
 *    }
 */
KNOWN_COUNT_TEST_INVERT(0xbfffffff, 0x00000001, 0xfffffffe, ige, ishr, 29)

/*    int i = 0x7fffffff;
 *    while (true) {
 *       i >>= 1;
 *
 *       if (2 >= i)
 *          break;
 *    }
 */
KNOWN_COUNT_TEST_INVERT(0x7fffffff, 0x00000001, 0x00000002, ige_rev, ishr, 29)

/*    int i = 0;
 *    while (true) {
 *       if (i != 0)
 *          break;
 *
 *       i <<= 1;
 *    }
 */
INFINITE_LOOP_UNKNOWN_COUNT_TEST(0x00000000, 0x00000000, 0x00000001, ine, ishl)

/*    int i = 1;
 *    while (true) {
 *       if (i == 3)
 *          break;
 *
 *       i <<= 1;
 *    }
 */
INFINITE_LOOP_UNKNOWN_COUNT_TEST(0x00000001, 0x00000003, 0x00000001, ieq, ishl)

/*    int i = 1;
 *    while (true) {
 *       if (i < 0x80000001)
 *          break;
 *
 *       i <<= 2;
 *    }
 */
INFINITE_LOOP_UNKNOWN_COUNT_TEST(0x00000001, 0x80000001, 0x00000002, ilt, ishl)

/*    int i = 0xffff0000;
 *    while (true) {
 *       if (1 < i)
 *          break;
 *
 *       i <<= 2;
 *    }
 */
INFINITE_LOOP_UNKNOWN_COUNT_TEST(0xffff0000, 0x00000001, 0x00000002, ilt_rev, ishl)

/*    int i = 1;
 *    while (true) {
 *       if (i >= 0x70000000)
 *          break;
 *
 *       i <<= 1;
 *    }
 */
INFINITE_LOOP_UNKNOWN_COUNT_TEST(0x00000001, 0x70000000, 0x00000001, ige, ishl)

/*    int i = 1;
 *    while (true) {
 *       if (0xf0000000 >= i)
 *          break;
 *
 *       i <<= 2;
 *    }
 */
INFINITE_LOOP_UNKNOWN_COUNT_TEST(0x00000001, 0xf0000000, 0x00000002, ige_rev, ishl)

/*    int i = 0x80000000;
 *    while (true) {
 *       i <<= 1;
 *
 *       if (i != 0)
 *          break;
 *    }
 */
INFINITE_LOOP_UNKNOWN_COUNT_TEST_INVERT(0x80000000, 0x00000001, 0x00000000, ine, ishl)

/*    int i = 0xf0f0f0f0;
 *    while (true) {
 *       i <<= 2;
 *
 *       if (i == 0xe1e1e1e0)
 *          break;
 *    }
 */
INFINITE_LOOP_UNKNOWN_COUNT_TEST_INVERT(0xf0f0f0f0, 0x00000002, 0xe1e1e1e0, ieq, ishl)

/*    int i = 1;
 *    while (true) {
 *       i <<= 2;
 *
 *       if (i < 0)
 *          break;
 *    }
 */
INFINITE_LOOP_UNKNOWN_COUNT_TEST_INVERT(0x00000001, 0x00000002, 0x00000000, ilt, ishl)

/*    int i = 0xffffffff;
 *    while (true) {
 *       i <<= 2;
 *
 *       if (0 < i)
 *          break;
 *    }
 */
INFINITE_LOOP_UNKNOWN_COUNT_TEST_INVERT(0xffffffff, 0x00000002, 0x00000000, ilt_rev, ishl)

/*    int i = 0x88888888;
 *    while (true) {
 *       i <<= 4;
 *
 *       if (i >= 1)
 *          break;
 *    }
 */
INFINITE_LOOP_UNKNOWN_COUNT_TEST_INVERT(0x88888888, 0x00000004, 0x00000001, ige, ishl)

/*    int i = 0x77777777;
 *    while (true) {
 *       i <<= 4;
 *
 *       if (-1 >= i)
 *          break;
 *    }
 */
INFINITE_LOOP_UNKNOWN_COUNT_TEST_INVERT(0x77777777, 0x00000004, 0xffffffff, ige_rev, ishl)

/*    int i = 1;
 *    while (true) {
 *       if (i != 1)
 *          break;
 *
 *       i <<= 1;
 *    }
 */
KNOWN_COUNT_TEST(0x00000001, 0x00000001, 0x00000001, ine, ishl, 1)

/*    int i = 1;
 *    while (true) {
 *       if (i == 0x1000)
 *          break;
 *
 *       i <<= 4;
 *    }
 */
KNOWN_COUNT_TEST(0x00000001, 0x00001000, 0x00000004, ieq, ishl, 3)

/*    uint i = 1;
 *    while (true) {
 *       if (i < 1)
 *          break;
 *
 *       i <<= 1;
 *    }
 */
KNOWN_COUNT_TEST(0x00000001, 0x00000001, 0x00000001, ult, ishl, 32)

/*    int i = 1;
 *    while (true) {
 *       if (i < 1)
 *          break;
 *
 *       i <<= 1;
 *    }
 */
KNOWN_COUNT_TEST(0x00000001, 0x00000001, 0x00000001, ilt, ishl, 31)

/*    int i = 0xffff0000;
 *    while (true) {
 *       if (-1 < i)
 *          break;
 *
 *       i <<= 2;
 *    }
 */
KNOWN_COUNT_TEST(0xffff0000, 0xffffffff, 0x00000002, ilt_rev, ishl, 8)

/*    int i = 0xf;
 *    while (true) {
 *       if (i >= 0x0000ffff)
 *          break;
 *
 *       i <<= 3;
 *    }
 */
KNOWN_COUNT_TEST(0x0000000f, 0x0000ffff, 0x00000003, ige, ishl, 5)

/*    int i = 0x0000000f;
 *    while (true) {
 *       if (-196608 >= i)
 *          break;
 *
 *       i <<= 4;
 *    }
 */
KNOWN_COUNT_TEST(0x0000000f, 0xfffd0000, 0x00000004, ige_rev, ishl, 7)

/*    int i = 1;
 *    while (true) {
 *       i <<= 1;
 *
 *       if (i != 2)
 *          break;
 *    }
 */
KNOWN_COUNT_TEST_INVERT(0x00000001, 0x00000001, 0x00000002, ine, ishl, 1)

/*    int i = 1;
 *    while (true) {
 *       i <<= 8;
 *
 *       if (i == 0x01000000)
 *          break;
 *    }
 */
KNOWN_COUNT_TEST_INVERT(0x00000001, 0x00000008, 0x01000000, ieq, ishl, 2)

/*    int i = 0x7fffffff;
 *    while (true) {
 *       i <<= 1;
 *
 *       if (i < 1)
 *          break;
 *    }
 */
KNOWN_COUNT_TEST_INVERT(0x7fffffff, 0x00000001, 0x00000001, ilt, ishl, 0)

/*    int i = 0x7fff;
 *    while (true) {
 *       i <<= 2;
 *
 *       if (0x1fffffff < i)
 *          break;
 *    }
 */
KNOWN_COUNT_TEST_INVERT(0x00007fff, 0x00000002, 0x1fffffff, ilt_rev, ishl, 7)

/*    int i = 0xffff7fff;
 *    while (true) {
 *       i <<= 4;
 *
 *       if (i >= -2)
 *          break;
 *    }
 */
KNOWN_COUNT_TEST_INVERT(0xffff7fff, 0x00000004, 0xfffffffe, ige, ishl, 3)

/*    int i = 0x0000f0f0;
 *    while (true) {
 *       i <<= 4;
 *
 *       if (-2 >= i)
 *          break;
 *    }
 */
KNOWN_COUNT_TEST_INVERT(0x0000f0f0, 0x00000004, 0xfffffffe, ige_rev, ishl, 3)

/* This infinite loop makes no sense, but it's a good test to make sure the
 * loop analysis code doesn't incorrectly treat left-shift as a commutative
 * operation.
 *
 *    int i = 1;
 *    while (true) {
 *       if (i == 0)
 *          break;
 *
 *       i = 1 << i;
 *    }
 */
INFINITE_LOOP_UNKNOWN_COUNT_TEST(0x00000001, 0x00000000, 0x00000001, ieq, ishl_rev)

/*    int i = 0;
 *    while (true) {
 *       if (i != 0)
 *          break;
 *
 *       i = i * 7;
 *    }
 */
INFINITE_LOOP_UNKNOWN_COUNT_TEST(0x00000000, 0x00000000, 0x00000007, ine, imul)

/*    int i = 1;
 *    while (true) {
 *       if (i == 4)
 *          break;
 *
 *       i = i * 3;
 *    }
 */
INFINITE_LOOP_UNKNOWN_COUNT_TEST(0x00000001, 0x00000004, 0x00000003, ieq, imul)

/*    int i = 1;
 *    while (true) {
 *       // The only value less than 0x80000001 is 0x80000000, but the result
 *       // of the multiply can never be even.
 *       if (i < 0x80000001)
 *          break;
 *
 *       i = i * 5;
 *    }
 */
INFINITE_LOOP_UNKNOWN_COUNT_TEST(0x00000001, 0x80000001, 0x00000005, ilt, imul)

/*    int i = 2;
 *    while (true) {
 *       if (i >= 0x7f000000)
 *          break;
 *
 *       i = i * 6;
 *    }
 */
INFINITE_LOOP_UNKNOWN_COUNT_TEST(0x00000002, 0x7f000000, 0x00000006, ige, imul)

/*    int i = 0x80000000;
 *    while (true) {
 *       i = i * 6;
 *
 *       if (i != 0)
 *          break;
 *    }
 */
INFINITE_LOOP_UNKNOWN_COUNT_TEST_INVERT(0x80000000, 0x00000006, 0x00000000, ine, imul)

/*    int i = 0xf0f0f0f0;
 *    while (true) {
 *       i = i * 6;
 *
 *       if (i == 0xe1e1e1e1)
 *          break;
 *    }
 */
INFINITE_LOOP_UNKNOWN_COUNT_TEST_INVERT(0xf0f0f0f0, 0x00000006, 0xe1e1e1e1, ieq, imul)

/*    int i = 3;
 *    while (true) {
 *       i = i * 3;
 *
 *       // The only value less than 0x80000001 is 0x80000000, but the result
 *       // of the multiply can never be even.
 *       if (i < 0x80000001)
 *          break;
 *    }
 */
INFINITE_LOOP_UNKNOWN_COUNT_TEST_INVERT(0x00000003, 0x00000003, 0x80000001, ilt, imul)

/*    int i = 0x88888888;
 *    while (true) {
 *       i = i * 16;
 *
 *       if (i >= 1)
 *          break;
 *    }
 *
 * I'm not fond of this test because (i * 16) is the same as (i << 4), but I
 * could not think of another way.
 */
INFINITE_LOOP_UNKNOWN_COUNT_TEST_INVERT(0x88888888, 0x00000010, 0x00000001, ige, imul)

/*    int i = 1;
 *    while (true) {
 *       if (i != 1)
 *          break;
 *
 *       i = i * 7;
 *    }
 */
KNOWN_COUNT_TEST(0x00000001, 0x00000001, 0x00000007, ine, imul, 1)

/*    int i = 2;
 *    while (true) {
 *       if (i == 54)
 *          break;
 *
 *       i = i * 3;
 *    }
 */
KNOWN_COUNT_TEST(0x00000002, 0x00000036, 0x00000003, ieq, imul, 3)

/*    int i = 5;
 *    while (true) {
 *       if (i < 1)
 *          break;
 *
 *       i = i * -3;
 *    }
 */
KNOWN_COUNT_TEST(0x00000005, 0x00000001, 0xfffffffd, ilt, imul, 1)

/*    int i = 0xf;
 *    while (true) {
 *       if (i >= 0x0000ffff)
 *          break;
 *
 *       i = i * 11;
 *    }
 */
KNOWN_COUNT_TEST(0x0000000f, 0x0000ffff, 0x0000000b, ige, imul, 4)

/*    int i = 3;
 *    while (true) {
 *       i = i * -5;
 *
 *       if (i != -15)
 *          break;
 *    }
 */
KNOWN_COUNT_TEST_INVERT(0x00000003, 0xfffffffb, 0xfffffff1, ine, imul, 1)

/*    int i = 3;
 *    while (true) {
 *       i = i * -7;
 *
 *       if (i == 0x562b3)
 *          break;
 *    }
 */
KNOWN_COUNT_TEST_INVERT(0x00000003, 0xfffffff9, 0x000562b3, ieq, imul, 5)

/*    int i = 0x7f;
 *    while (true) {
 *       i = i * 3;
 *
 *       if (i < 1)
 *          break;
 *    }
 */
KNOWN_COUNT_TEST_INVERT(0x0000007f, 0x00000003, 0x00000001, ilt, imul, 16)

/*    int i = 0xffff7fff;
 *    while (true) {
 *       i = i * 15;
 *
 *       if (i >= 0x34cce9b0)
 *          break;
 *    }
 */
KNOWN_COUNT_TEST_INVERT(0xffff7fff, 0x0000000f, 0x34cce9b0, ige, imul, 4)

/*    int i = 0;
 *    while (true) {
 *       if (i >= imin(vertex_id, 4))
 *          break;
 *
 *       i++;
 *    }
 */
INEXACT_COUNT_TEST(0x00000000, 0x00000004, 0x00000001, ige_imin, iadd, 4)

/* This fmin is the wrong type to be useful.
 *
 *    int i = 0;
 *    while (true) {
 *       if (i >= fmin(vertex_id, 4))
 *          break;
 *
 *       i++;
 *    }
 */
UNKNOWN_COUNT_TEST(0x00000000, 0x00000004, 0x00000001, ige_fmin, iadd)

/* The comparison is unsigned, so this isn't safe if vertex_id is negative.
 *
 *    uint i = 0;
 *    while (true) {
 *       if (i >= imin(vertex_id, 4))
 *          break;
 *
 *       i++;
 *    }
 */
UNKNOWN_COUNT_TEST(0x00000000, 0x00000004, 0x00000001, uge_imin, iadd)

/*    int i = 8;
 *    while (true) {
 *       if (4 >= i)
 *          break;
 *
 *       i += -1;
 *    }
 */
KNOWN_COUNT_TEST(0x00000008, 0x00000004, 0xffffffff, ige_rev, iadd, 4)

/*    int i = 8;
 *    while (true) {
 *       if (i < 4)
 *          break;
 *
 *       i += -1;
 *    }
 */
KNOWN_COUNT_TEST(0x00000008, 0x00000004, 0xffffffff, ilt, iadd, 5)

/* This imin can increase the iteration count, not limit it.
 *
 *    int i = 8;
 *    while (true) {
 *       if (imin(vertex_id, 4) >= i)
 *          break;
 *
 *       i += -1;
 *    }
 */
UNKNOWN_COUNT_TEST(0x00000008, 0x00000004, 0xffffffff, ige_imin_rev, iadd)

/* This imin can increase the iteration count, not limit it.
 *
 *    int i = 8;
 *    while (true) {
 *       if (i < imin(vertex_id, 4))
 *          break;
 *
 *       i += -1;
 *    }
 */
UNKNOWN_COUNT_TEST(0x00000008, 0x00000004, 0xffffffff, ilt_imin, iadd)

/*    int i = 8;
 *    while (true) {
 *       if (i < imax(vertex_id, 4))
 *          break;
 *
 *       i--;
 *    }
 */
INEXACT_COUNT_TEST(0x00000008, 0x00000004, 0xffffffff, ilt_imax, iadd, 5)

/*    uint i = 0x00000001;
 *    while (true) {
 *       if (i >= umin(vertex_id, 0x00000100))
 *          break;
 *
 *       i <<= 1;
 *    }
 */
INEXACT_COUNT_TEST(0x00000001, 0x00000100, 0x00000001, uge_umin, ishl, 8)
