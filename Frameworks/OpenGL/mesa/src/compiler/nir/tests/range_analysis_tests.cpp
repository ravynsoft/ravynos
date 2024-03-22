/*
 * Copyright © 2021 Intel Corporation
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
#include "nir_range_analysis.h"

class ssa_def_bits_used_test : public nir_test {
protected:
   ssa_def_bits_used_test()
      : nir_test::nir_test("nir_def_bits_used_test")
   {
   }

   nir_alu_instr *build_alu_instr(nir_op op, nir_def *, nir_def *);
};

class unsigned_upper_bound_test : public nir_test {
protected:
   unsigned_upper_bound_test()
      : nir_test::nir_test("nir_unsigned_upper_bound_test")
   {
   }
};

static bool
is_used_once(const nir_def *def)
{
   return list_is_singular(&def->uses);
}

nir_alu_instr *
ssa_def_bits_used_test::build_alu_instr(nir_op op,
                                        nir_def *src0, nir_def *src1)
{
   nir_def *def = nir_build_alu(b, op, src0, src1, NULL, NULL);

   if (def == NULL)
      return NULL;

   nir_alu_instr *alu = nir_instr_as_alu(def->parent_instr);

   if (alu == NULL)
      return NULL;

   alu->def.num_components = 1;

   return alu;
}

TEST_F(ssa_def_bits_used_test, iand_with_const_vector)
{
   static const unsigned src0_imm[4] = { 255u << 24, 255u << 16, 255u << 8, 255u };

   nir_def *src0 = nir_imm_ivec4(b,
                                     src0_imm[0], src0_imm[1],
                                     src0_imm[2], src0_imm[3]);
   nir_def *src1 = nir_imm_int(b, 0xffffffff);

   nir_alu_instr *alu = build_alu_instr(nir_op_iand, src0, src1);

   ASSERT_NE((void *) 0, alu);

   for (unsigned i = 0; i < 4; i++) {
      /* If the test is changed, and somehow src1 is used multiple times,
       * nir_def_bits_used will accumulate *all* the uses (as it should).
       * This isn't what we're trying to test here.
       */
      ASSERT_TRUE(is_used_once(src1));

      alu->src[0].swizzle[0] = i;

      const uint64_t bits_used = nir_def_bits_used(alu->src[1].src.ssa);

      /* The answer should be the value swizzled from src0. */
      EXPECT_EQ(src0_imm[i], bits_used);
   }
}

TEST_F(ssa_def_bits_used_test, ior_with_const_vector)
{
   static const unsigned src0_imm[4] = { 255u << 24, 255u << 16, 255u << 8, 255u };

   nir_def *src0 = nir_imm_ivec4(b,
                                     src0_imm[0], src0_imm[1],
                                     src0_imm[2], src0_imm[3]);
   nir_def *src1 = nir_imm_int(b, 0xffffffff);

   nir_alu_instr *alu = build_alu_instr(nir_op_ior, src0, src1);

   ASSERT_NE((void *) 0, alu);

   for (unsigned i = 0; i < 4; i++) {
      /* If the test is changed, and somehow src1 is used multiple times,
       * nir_def_bits_used will accumulate *all* the uses (as it should).
       * This isn't what we're trying to test here.
       */
      ASSERT_TRUE(is_used_once(src1));

      alu->src[0].swizzle[0] = i;

      const uint64_t bits_used = nir_def_bits_used(alu->src[1].src.ssa);

      /* The answer should be the value swizzled from ~src0. */
      EXPECT_EQ(~src0_imm[i], bits_used);
   }
}

TEST_F(ssa_def_bits_used_test, extract_i16_with_const_index)
{
   nir_def *src0 = nir_imm_int(b, 0xffffffff);

   static const unsigned src1_imm[4] = { 9, 1, 0, 9 };

   nir_def *src1 = nir_imm_ivec4(b,
                                     src1_imm[0],
                                     src1_imm[1],
                                     src1_imm[2],
                                     src1_imm[3]);

   nir_alu_instr *alu = build_alu_instr(nir_op_extract_i16, src0, src1);

   ASSERT_NE((void *) 0, alu);

   for (unsigned i = 1; i < 3; i++) {
      /* If the test is changed, and somehow src1 is used multiple times,
       * nir_def_bits_used will accumulate *all* the uses (as it should).
       * This isn't what we're trying to test here.
       */
      ASSERT_TRUE(is_used_once(src1));

      alu->src[1].swizzle[0] = i;

      const uint64_t bits_used = nir_def_bits_used(alu->src[0].src.ssa);

      EXPECT_EQ(0xffffu << (16 * src1_imm[i]), bits_used);
   }
}

TEST_F(ssa_def_bits_used_test, extract_u16_with_const_index)
{
   nir_def *src0 = nir_imm_int(b, 0xffffffff);

   static const unsigned src1_imm[4] = { 9, 1, 0, 9 };

   nir_def *src1 = nir_imm_ivec4(b,
                                     src1_imm[0],
                                     src1_imm[1],
                                     src1_imm[2],
                                     src1_imm[3]);

   nir_alu_instr *alu = build_alu_instr(nir_op_extract_u16, src0, src1);

   ASSERT_NE((void *) 0, alu);

   for (unsigned i = 1; i < 3; i++) {
      /* If the test is changed, and somehow src1 is used multiple times,
       * nir_def_bits_used will accumulate *all* the uses (as it should).
       * This isn't what we're trying to test here.
       */
      ASSERT_TRUE(is_used_once(src1));

      alu->src[1].swizzle[0] = i;

      const uint64_t bits_used = nir_def_bits_used(alu->src[0].src.ssa);

      EXPECT_EQ(0xffffu << (16 * src1_imm[i]), bits_used);
   }
}

TEST_F(ssa_def_bits_used_test, extract_i8_with_const_index)
{
   nir_def *src0 = nir_imm_int(b, 0xffffffff);

   static const unsigned src1_imm[4] = { 3, 2, 1, 0 };

   nir_def *src1 = nir_imm_ivec4(b,
                                     src1_imm[0],
                                     src1_imm[1],
                                     src1_imm[2],
                                     src1_imm[3]);

   nir_alu_instr *alu = build_alu_instr(nir_op_extract_i8, src0, src1);

   ASSERT_NE((void *) 0, alu);

   for (unsigned i = 0; i < 4; i++) {
      /* If the test is changed, and somehow src1 is used multiple times,
       * nir_def_bits_used will accumulate *all* the uses (as it should).
       * This isn't what we're trying to test here.
       */
      ASSERT_TRUE(is_used_once(src1));

      alu->src[1].swizzle[0] = i;

      const uint64_t bits_used = nir_def_bits_used(alu->src[0].src.ssa);

      EXPECT_EQ(0xffu << (8 * src1_imm[i]), bits_used);
   }
}

TEST_F(ssa_def_bits_used_test, extract_u8_with_const_index)
{
   nir_def *src0 = nir_imm_int(b, 0xffffffff);

   static const unsigned src1_imm[4] = { 3, 2, 1, 0 };

   nir_def *src1 = nir_imm_ivec4(b,
                                     src1_imm[0],
                                     src1_imm[1],
                                     src1_imm[2],
                                     src1_imm[3]);

   nir_alu_instr *alu = build_alu_instr(nir_op_extract_u8, src0, src1);

   ASSERT_NE((void *) 0, alu);

   for (unsigned i = 0; i < 4; i++) {
      /* If the test is changed, and somehow src1 is used multiple times,
       * nir_def_bits_used will accumulate *all* the uses (as it should).
       * This isn't what we're trying to test here.
       */
      ASSERT_TRUE(is_used_once(src1));

      alu->src[1].swizzle[0] = i;

      const uint64_t bits_used = nir_def_bits_used(alu->src[0].src.ssa);

      EXPECT_EQ(0xffu << (8 * src1_imm[i]), bits_used);
   }
}

/* Unsigned upper bound analysis should look through a bcsel which uses the phi. */
TEST_F(unsigned_upper_bound_test, loop_phi_bcsel)
{
   /*
    * impl main {
    *     block b0:  // preds:
    *     32    %0 = load_const (0x00000000 = 0.000000)
    *     32    %1 = load_const (0x00000002 = 0.000000)
    *     1     %2 = load_const (false)
    *                // succs: b1
    *     loop {
    *         block b1:  // preds: b0 b1
    *         32    %4 = phi b0: %0 (0x0), b1: %3
    *         32    %3 = bcsel %2 (false), %4, %1 (0x2)
    *                    // succs: b1
    *     }
    *     block b2:  // preds: , succs: b3
    *     block b3:
    * }
    */
   nir_def *zero = nir_imm_int(b, 0);
   nir_def *two = nir_imm_int(b, 2);
   nir_def *cond = nir_imm_false(b);

   nir_phi_instr *const phi = nir_phi_instr_create(b->shader);
   nir_def_init(&phi->instr, &phi->def, 1, 32);

   nir_push_loop(b);
   nir_def *sel = nir_bcsel(b, cond, &phi->def, two);
   nir_pop_loop(b, NULL);

   nir_phi_instr_add_src(phi, zero->parent_instr->block, zero);
   nir_phi_instr_add_src(phi, sel->parent_instr->block, sel);
   b->cursor = nir_before_instr(sel->parent_instr);
   nir_builder_instr_insert(b, &phi->instr);

   nir_validate_shader(b->shader, NULL);

   struct hash_table *range_ht = _mesa_pointer_hash_table_create(NULL);
   nir_scalar scalar = nir_get_scalar(&phi->def, 0);
   EXPECT_EQ(nir_unsigned_upper_bound(b->shader, range_ht, scalar, NULL), 2);
   _mesa_hash_table_destroy(range_ht, NULL);
}
