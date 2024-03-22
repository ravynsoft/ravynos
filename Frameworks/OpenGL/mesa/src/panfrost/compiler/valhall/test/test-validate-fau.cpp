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
#include "va_compiler.h"

#include <gtest/gtest.h>

#define CASE(instr, expected)                                                  \
   do {                                                                        \
      if (va_validate_fau(instr) != expected) {                                \
         fprintf(stderr, "Incorrect validation for:\n");                       \
         bi_print_instr(instr, stderr);                                        \
         fprintf(stderr, "\n");                                                \
         ADD_FAILURE();                                                        \
      }                                                                        \
   } while (0)

#define VALID(instr)   CASE(instr, true)
#define INVALID(instr) CASE(instr, false)

class ValidateFau : public testing::Test {
 protected:
   ValidateFau()
   {
      mem_ctx = ralloc_context(NULL);
      b = bit_builder(mem_ctx);

      zero = bi_fau((enum bir_fau)(BIR_FAU_IMMEDIATE | 0), false);
      imm1 = bi_fau((enum bir_fau)(BIR_FAU_IMMEDIATE | 1), false);
      imm2 = bi_fau((enum bir_fau)(BIR_FAU_IMMEDIATE | 2), false);
      unif = bi_fau((enum bir_fau)(BIR_FAU_UNIFORM | 5), false);
      unif_hi = bi_fau((enum bir_fau)(BIR_FAU_UNIFORM | 5), true);
      unif2 = bi_fau((enum bir_fau)(BIR_FAU_UNIFORM | 6), false);
      core_id = bi_fau(BIR_FAU_CORE_ID, false);
      lane_id = bi_fau(BIR_FAU_LANE_ID, false);
   }

   ~ValidateFau()
   {
      ralloc_free(mem_ctx);
   }

   void *mem_ctx;
   bi_builder *b;
   bi_index zero, imm1, imm2, unif, unif_hi, unif2, core_id, lane_id;
};

TEST_F(ValidateFau, One64BitUniformSlot)
{
   VALID(
      bi_fma_f32_to(b, bi_register(1), bi_register(2), bi_register(3), unif));
   VALID(bi_fma_f32_to(b, bi_register(1), bi_register(2), unif_hi, unif));
   VALID(bi_fma_f32_to(b, bi_register(1), unif, unif, unif_hi));
   INVALID(bi_fma_f32_to(b, bi_register(1), unif, unif2, bi_register(1)));
   INVALID(bi_fma_f32_to(b, bi_register(1), unif, unif2, unif_hi));

   /* Crafted case that appears correct at first glance and was erronously
    * marked as valid in early versions of the validator.
    */
   INVALID(bi_fma_f32_to(b, bi_register(1), bi_register(2),
                         bi_fau((enum bir_fau)(BIR_FAU_UNIFORM | 0), false),
                         bi_fau((enum bir_fau)(BIR_FAU_UNIFORM | 1), true)));
}

TEST_F(ValidateFau, Combined64BitUniformsConstants)
{
   VALID(bi_fma_f32_to(b, bi_register(1), bi_register(2), unif_hi, unif));
   VALID(bi_fma_f32_to(b, bi_register(1), bi_register(2), zero, unif));
   VALID(bi_fma_f32_to(b, bi_register(1), zero, imm1, imm1));
   INVALID(bi_fma_f32_to(b, bi_register(1), zero, unif_hi, unif));
   INVALID(bi_fma_f32_to(b, bi_register(1), zero, imm1, imm2));
}

TEST_F(ValidateFau, UniformsOnlyInDefaultMode)
{
   INVALID(bi_fma_f32_to(b, bi_register(1), bi_register(2), unif_hi, lane_id));
   INVALID(bi_fma_f32_to(b, bi_register(1), bi_register(2), unif_hi, core_id));
}

TEST_F(ValidateFau, SingleSpecialImmediate)
{
   VALID(bi_fma_f32_to(b, bi_register(1), bi_register(2), bi_register(2),
                       lane_id));
   VALID(bi_fma_f32_to(b, bi_register(1), bi_register(2), bi_register(2),
                       core_id));
   INVALID(bi_fma_f32_to(b, bi_register(1), bi_register(2), lane_id, core_id));
}

TEST_F(ValidateFau, SmokeTests)
{
   VALID(bi_mov_i32_to(b, bi_register(1), bi_register(2)));
   VALID(bi_mov_i32_to(b, bi_register(1), unif));
   VALID(bi_fma_f32_to(b, bi_register(1), bi_discard(bi_register(1)), unif,
                       bi_neg(zero)));
}
