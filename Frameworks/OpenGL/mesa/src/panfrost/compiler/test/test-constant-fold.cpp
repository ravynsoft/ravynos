/*
 * Copyright (C) 2021 Collabora, Ltd.
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "bi_builder.h"
#include "bi_test.h"
#include "compiler.h"

#include <gtest/gtest.h>

static std::string
to_string(const bi_instr *I)
{
   char *cstr = NULL;
   size_t size = 0;
   FILE *f = open_memstream(&cstr, &size);
   bi_print_instr(I, f);
   fclose(f);
   auto str = std::string(cstr);
   free(cstr);
   return str;
}

static testing::AssertionResult
constant_fold_pred(const char *I_expr, const char *expected_expr, bi_instr *I,
                   uint32_t expected)
{
   bool unsupported = false;
   uint32_t v = bi_fold_constant(I, &unsupported);
   if (unsupported) {
      return testing::AssertionFailure()
             << "Constant fold unsupported for instruction \n\n"
             << "  " << to_string(I);
   } else if (v != expected) {
      return testing::AssertionFailure()
             << "Unexpected result when constant folding instruction\n\n"
             << "  " << to_string(I) << "\n"
             << "  Actual: " << v << "\n"
             << "Expected: " << expected << "\n";
   } else {
      return testing::AssertionSuccess();
   }
}

#define EXPECT_FOLD(i, e) EXPECT_PRED_FORMAT2(constant_fold_pred, i, e)

static testing::AssertionResult
not_constant_fold_pred(const char *I_expr, bi_instr *I)
{
   bool unsupported = false;
   uint32_t v = bi_fold_constant(I, &unsupported);
   if (unsupported) {
      return testing::AssertionSuccess();
   } else {
      return testing::AssertionFailure()
             << "Instruction\n\n"
             << "  " << to_string(I) << "\n"
             << "shouldn't have constant folded, but folded to: " << v;
   }
}

#define EXPECT_NOT_FOLD(i) EXPECT_PRED_FORMAT1(not_constant_fold_pred, i)

class ConstantFold : public testing::Test {
 protected:
   ConstantFold()
   {
      mem_ctx = ralloc_context(NULL);
      b = bit_builder(mem_ctx);
   }
   ~ConstantFold()
   {
      ralloc_free(mem_ctx);
   }

   void *mem_ctx;
   bi_builder *b;
};

TEST_F(ConstantFold, Swizzles)
{
   bi_index reg = bi_register(0);

   EXPECT_FOLD(bi_swz_v2i16_to(b, reg, bi_imm_u32(0xCAFEBABE)), 0xCAFEBABE);

   EXPECT_FOLD(
      bi_swz_v2i16_to(b, reg, bi_swz_16(bi_imm_u32(0xCAFEBABE), false, false)),
      0xBABEBABE);

   EXPECT_FOLD(
      bi_swz_v2i16_to(b, reg, bi_swz_16(bi_imm_u32(0xCAFEBABE), true, false)),
      0xBABECAFE);

   EXPECT_FOLD(
      bi_swz_v2i16_to(b, reg, bi_swz_16(bi_imm_u32(0xCAFEBABE), true, true)),
      0xCAFECAFE);
}

TEST_F(ConstantFold, VectorConstructions2i16)
{
   bi_index reg = bi_register(0);

   EXPECT_FOLD(
      bi_mkvec_v2i16_to(b, reg, bi_imm_u16(0xCAFE), bi_imm_u16(0xBABE)),
      0xBABECAFE);

   EXPECT_FOLD(
      bi_mkvec_v2i16_to(b, reg, bi_swz_16(bi_imm_u32(0xCAFEBABE), true, true),
                        bi_imm_u16(0xBABE)),
      0xBABECAFE);

   EXPECT_FOLD(
      bi_mkvec_v2i16_to(b, reg, bi_swz_16(bi_imm_u32(0xCAFEBABE), true, true),
                        bi_swz_16(bi_imm_u32(0xCAFEBABE), false, false)),
      0xBABECAFE);
}

TEST_F(ConstantFold, VectorConstructions4i8)
{
   bi_index reg = bi_register(0);
   bi_index u32 = bi_imm_u32(0xCAFEBABE);

   bi_index a = bi_byte(u32, 0); /* 0xBE */
   bi_index c = bi_byte(u32, 2); /* 0xFE */

   EXPECT_FOLD(bi_mkvec_v4i8_to(b, reg, a, a, a, a), 0xBEBEBEBE);
   EXPECT_FOLD(bi_mkvec_v4i8_to(b, reg, a, c, a, c), 0xFEBEFEBE);
   EXPECT_FOLD(bi_mkvec_v4i8_to(b, reg, c, a, c, a), 0xBEFEBEFE);
   EXPECT_FOLD(bi_mkvec_v4i8_to(b, reg, c, c, c, c), 0xFEFEFEFE);
}

TEST_F(ConstantFold, VectorConstructions2i8)
{
   bi_index reg = bi_register(0);
   bi_index u32 = bi_imm_u32(0xCAFEBABE);
   bi_index rem = bi_imm_u32(0xABCD1234);

   bi_index a = bi_byte(u32, 0); /* 0xBE */
   bi_index B = bi_byte(u32, 1); /* 0xBA */
   bi_index c = bi_byte(u32, 2); /* 0xFE */
   bi_index d = bi_byte(u32, 3); /* 0xCA */

   EXPECT_FOLD(bi_mkvec_v2i8_to(b, reg, a, B, rem), 0x1234BABE);
   EXPECT_FOLD(bi_mkvec_v2i8_to(b, reg, a, d, rem), 0x1234CABE);
   EXPECT_FOLD(bi_mkvec_v2i8_to(b, reg, c, d, rem), 0x1234CAFE);
   EXPECT_FOLD(bi_mkvec_v2i8_to(b, reg, d, d, rem), 0x1234CACA);
}

TEST_F(ConstantFold, LimitedShiftsForTexturing)
{
   bi_index reg = bi_register(0);

   EXPECT_FOLD(bi_lshift_or_i32_to(b, reg, bi_imm_u32(0xCAFE),
                                   bi_imm_u32(0xA0000), bi_imm_u8(4)),
               (0xCAFE << 4) | 0xA0000);

   EXPECT_NOT_FOLD(bi_lshift_or_i32_to(
      b, reg, bi_imm_u32(0xCAFE), bi_not(bi_imm_u32(0xA0000)), bi_imm_u8(4)));

   EXPECT_NOT_FOLD(bi_lshift_or_i32_to(b, reg, bi_not(bi_imm_u32(0xCAFE)),
                                       bi_imm_u32(0xA0000), bi_imm_u8(4)));

   bi_instr *I = bi_lshift_or_i32_to(b, reg, bi_imm_u32(0xCAFE),
                                     bi_imm_u32(0xA0000), bi_imm_u8(4));
   I->not_result = true;
   EXPECT_NOT_FOLD(I);
}

TEST_F(ConstantFold, NonConstantSourcesCannotBeFolded)
{
   bi_index reg = bi_register(0);

   EXPECT_NOT_FOLD(bi_swz_v2i16_to(b, reg, bi_temp(b->shader)));
   EXPECT_NOT_FOLD(
      bi_mkvec_v2i16_to(b, reg, bi_temp(b->shader), bi_temp(b->shader)));
   EXPECT_NOT_FOLD(
      bi_mkvec_v2i16_to(b, reg, bi_temp(b->shader), bi_imm_u32(0xDEADBEEF)));
   EXPECT_NOT_FOLD(
      bi_mkvec_v2i16_to(b, reg, bi_imm_u32(0xDEADBEEF), bi_temp(b->shader)));
}

TEST_F(ConstantFold, OtherOperationsShouldNotFold)
{
   bi_index zero = bi_fau(bir_fau(BIR_FAU_IMMEDIATE | 0), false);
   bi_index reg = bi_register(0);

   EXPECT_NOT_FOLD(bi_fma_f32_to(b, reg, zero, zero, zero));
   EXPECT_NOT_FOLD(bi_fadd_f32_to(b, reg, zero, zero));
}
