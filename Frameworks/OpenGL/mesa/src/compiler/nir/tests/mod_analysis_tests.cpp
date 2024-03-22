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

#include "nir_test.h"
#include "util/u_math.h"

static inline bool
nir_mod_analysis_comp0(nir_def *val, nir_alu_type val_type, unsigned div, unsigned *mod)
{
   return nir_mod_analysis(nir_get_scalar(val, 0), val_type, div, mod);
}

class nir_mod_analysis_test : public nir_test {
protected:
   nir_mod_analysis_test();

   nir_def *nir_imul_vec2y(nir_builder *b, nir_def *src0, nir_def *src1);

   nir_def *v[50];
   nir_def *invocation;
};

nir_mod_analysis_test::nir_mod_analysis_test()
   : nir_test::nir_test("nir_mod_analysis_test")
{
   for (int i = 0; i < 50; ++i)
      v[i] = nir_imm_int(b, i);
   invocation = nir_load_local_invocation_index(b);
}

/* returns src0 * src1.y */
nir_def *
nir_mod_analysis_test::nir_imul_vec2y(nir_builder *b, nir_def *src0, nir_def *src1)
{
   nir_alu_instr *instr = nir_alu_instr_create(b->shader, nir_op_imul);

   instr->src[0].src = nir_src_for_ssa(src0);
   instr->src[1].src = nir_src_for_ssa(src1);
   instr->src[1].swizzle[0] = 1;

   nir_def_init(&instr->instr, &instr->def, 1, 32);

   nir_builder_instr_insert(b, &instr->instr);
   return &instr->def;
}

TEST_F(nir_mod_analysis_test, const_val)
{
   /* const % const_mod should be always known */
   for (unsigned const_mod = 1; const_mod <= 1024; const_mod *= 2) {
      for (int cnst = 0; cnst < 10; ++cnst) {
         unsigned mod = INT32_MAX;

         EXPECT_TRUE(nir_mod_analysis_comp0(v[cnst], nir_type_uint, const_mod, &mod));
         EXPECT_EQ(mod, cnst % const_mod);
      }
   }
}

TEST_F(nir_mod_analysis_test, dynamic)
{
   /* invocation % const_mod should never be known unless const_mod is 1 */

   unsigned mod = INT32_MAX;

   EXPECT_TRUE(nir_mod_analysis_comp0(invocation, nir_type_uint, 1, &mod));
   EXPECT_EQ(mod, 0);

   for (unsigned const_mod = 2; const_mod <= 1024; const_mod *= 2)
      EXPECT_FALSE(nir_mod_analysis_comp0(invocation, nir_type_uint, const_mod, &mod));
}

TEST_F(nir_mod_analysis_test, const_plus_const)
{
   /* (const1 + const2) % const_mod should always be known */
   for (unsigned const_mod = 1; const_mod <= 1024; const_mod *= 2) {
      for (unsigned c1 = 0; c1 < 10; ++c1) {
         for (unsigned c2 = 0; c2 < 10; ++c2) {
            nir_def *sum = nir_iadd(b, v[c1], v[c2]);

            unsigned mod = INT32_MAX;

            EXPECT_TRUE(nir_mod_analysis_comp0(sum, nir_type_uint, const_mod, &mod));
            EXPECT_EQ(mod, (c1 + c2) % const_mod);
         }
      }
   }
}

TEST_F(nir_mod_analysis_test, dynamic_plus_const)
{
   /* (invocation + const) % const_mod should never be known unless const_mod is 1 */
   for (unsigned const_mod = 1; const_mod <= 1024; const_mod *= 2) {
      for (unsigned c = 0; c < 10; ++c) {
         nir_def *sum = nir_iadd(b, invocation, v[c]);

         unsigned mod = INT32_MAX;

         if (const_mod == 1) {
            EXPECT_TRUE(nir_mod_analysis_comp0(sum, nir_type_uint, const_mod, &mod));
            EXPECT_EQ(mod, 0);
         } else {
            EXPECT_FALSE(nir_mod_analysis_comp0(sum, nir_type_uint, const_mod, &mod));
         }
      }
   }
}

TEST_F(nir_mod_analysis_test, const_mul_const)
{
   /* (const1 * const2) % const_mod should always be known */
   for (unsigned const_mod = 1; const_mod <= 1024; const_mod *= 2) {
      for (unsigned c1 = 0; c1 < 10; ++c1) {
         for (unsigned c2 = 0; c2 < 10; ++c2) {
            nir_def *mul = nir_imul(b, v[c1], v[c2]);

            unsigned mod = INT32_MAX;

            EXPECT_TRUE(nir_mod_analysis_comp0(mul, nir_type_uint, const_mod, &mod));
            EXPECT_EQ(mod, (c1 * c2) % const_mod);
         }
      }
   }
}

TEST_F(nir_mod_analysis_test, dynamic_mul_const)
{
   /* (invocation * const) % const_mod == 0 only if const % const_mod == 0, unknown otherwise */
   for (unsigned const_mod = 2; const_mod <= 1024; const_mod *= 2) {
      for (unsigned c = 0; c < 10; ++c) {
         nir_def *mul = nir_imul(b, invocation, v[c]);

         unsigned mod = INT32_MAX;

         if (c % const_mod == 0) {
            EXPECT_TRUE(nir_mod_analysis_comp0(mul, nir_type_uint, const_mod, &mod));
            EXPECT_EQ(mod, 0);
         } else {
            EXPECT_FALSE(nir_mod_analysis_comp0(mul, nir_type_uint, const_mod, &mod));
         }
      }
   }
}

TEST_F(nir_mod_analysis_test, dynamic_mul_const_swizzled)
{
   /* (invocation * const.y) % const_mod == 0 only if const.y % const_mod == 0, unknown otherwise */
   for (unsigned const_mod = 2; const_mod <= 1024; const_mod *= 2) {
      for (unsigned c = 0; c < 10; ++c) {
         nir_def *vec2 = nir_imm_ivec2(b, 10 - c, c);
         nir_def *mul = nir_imul_vec2y(b, invocation, vec2);

         unsigned mod = INT32_MAX;

         if (c % const_mod == 0) {
            EXPECT_TRUE(nir_mod_analysis_comp0(mul, nir_type_uint, const_mod, &mod));
            EXPECT_EQ(mod, 0);
         } else {
            EXPECT_FALSE(nir_mod_analysis_comp0(mul, nir_type_uint, const_mod, &mod));
         }
      }
   }
}

TEST_F(nir_mod_analysis_test, dynamic_mul32x16_const)
{
   /* (invocation mul32x16 const) % const_mod == 0 only if const % const_mod == 0
    * and const_mod <= 2^16, unknown otherwise
    */
   for (unsigned const_mod = 1; const_mod <= (1u << 24); const_mod *= 2) {
      for (unsigned c = 0; c < 10; ++c) {
         nir_def *mul = nir_imul_32x16(b, invocation, v[c]);

         unsigned mod = INT32_MAX;

         if (c % const_mod == 0 && const_mod <= (1u << 16)) {
            EXPECT_TRUE(nir_mod_analysis_comp0(mul, nir_type_uint, const_mod, &mod));
            EXPECT_EQ(mod, 0);
         } else {
            EXPECT_FALSE(nir_mod_analysis_comp0(mul, nir_type_uint, const_mod, &mod));
         }
      }
   }
}

TEST_F(nir_mod_analysis_test, dynamic_shl_const)
{
   /* (invocation << const) % const_mod == 0 only if const >= log2(const_mod), unknown otherwise */
   for (unsigned const_mod = 1; const_mod <= 1024; const_mod *= 2) {
      for (unsigned c = 0; c < 10; ++c) {
         nir_def *shl = nir_ishl(b, invocation, v[c]);

         unsigned mod = INT32_MAX;

         if (c >= util_logbase2(const_mod)) {
            EXPECT_TRUE(nir_mod_analysis_comp0(shl, nir_type_uint, const_mod, &mod));
            EXPECT_EQ(mod, 0);
         } else {
            EXPECT_FALSE(nir_mod_analysis_comp0(shl, nir_type_uint, const_mod, &mod));
         }
      }
   }
}

TEST_F(nir_mod_analysis_test, dynamic_shr_const)
{
   /* (invocation >> const) % const_mod should never be known, unless const_mod is 1 */
   for (unsigned const_mod = 1; const_mod <= 1024; const_mod *= 2) {
      for (unsigned i = 0; i < 10; ++i) {
         nir_def *shr = nir_ishr(b, invocation, v[i]);

         unsigned mod = INT32_MAX;

         if (const_mod == 1) {
            EXPECT_TRUE(nir_mod_analysis_comp0(shr, nir_type_uint, const_mod, &mod));
            EXPECT_EQ(mod, 0);
         } else {
            EXPECT_FALSE(nir_mod_analysis_comp0(shr, nir_type_uint, const_mod, &mod));
         }
      }
   }
}

TEST_F(nir_mod_analysis_test, dynamic_mul_const_shr_const)
{
   /* ((invocation * 32) >> const) % const_mod == 0 only if
    *   const_mod is 1 or
    *   (32 >> const) is not 0 and (32 >> const) % const_mod == 0
    *
    */
   nir_def *inv_mul_32 = nir_imul(b, invocation, v[32]);
   for (unsigned const_mod = 1; const_mod <= 1024; const_mod *= 2) {
      for (unsigned c = 0; c < 8; ++c) {
         nir_def *shr = nir_ishr(b, inv_mul_32, v[c]);

         unsigned mod = INT32_MAX;

         if (const_mod == 1 || ((32 >> c) > 0 && (32 >> c) % const_mod == 0)) {
            EXPECT_TRUE(nir_mod_analysis_comp0(shr, nir_type_uint, const_mod, &mod));
            EXPECT_EQ(mod, 0);
         } else {
            EXPECT_FALSE(nir_mod_analysis_comp0(shr, nir_type_uint, const_mod, &mod));
         }
      }
   }
}

TEST_F(nir_mod_analysis_test, dynamic_mul_const_swizzled_shr_const)
{
   /* ((invocation * ivec2(31, 32).y) >> const) % const_mod == 0 only if
    *   const_mod is 1 or
    *   (32 >> const) is not 0 and (32 >> const) % const_mod == 0
    *
    */
   nir_def *vec2 = nir_imm_ivec2(b, 31, 32);
   nir_def *inv_mul_32 = nir_imul_vec2y(b, invocation, vec2);

   for (unsigned const_mod = 1; const_mod <= 1024; const_mod *= 2) {
      for (unsigned c = 0; c < 8; ++c) {
         nir_def *shr = nir_ishr(b, inv_mul_32, v[c]);

         unsigned mod = INT32_MAX;

         if (const_mod == 1 || ((32 >> c) > 0 && (32 >> c) % const_mod == 0)) {
            EXPECT_TRUE(nir_mod_analysis_comp0(shr, nir_type_uint, const_mod, &mod));
            EXPECT_EQ(mod, 0);
         } else {
            EXPECT_FALSE(nir_mod_analysis_comp0(shr, nir_type_uint, const_mod, &mod));
         }
      }
   }
}

TEST_F(nir_mod_analysis_test, const_shr_const)
{
   /* (const >> const) % const_mod should always be known */
   for (unsigned const_mod = 1; const_mod <= 1024; const_mod *= 2) {
      for (unsigned i = 0; i < 50; ++i) {
         for (unsigned j = 0; j < 6; ++j) {
            nir_def *shr = nir_ishr(b, v[i], v[j]);

            unsigned mod = INT32_MAX;

            EXPECT_TRUE(nir_mod_analysis_comp0(shr, nir_type_uint, const_mod, &mod));
            EXPECT_EQ(mod, (i >> j) % const_mod);
         }
      }
   }
}

TEST_F(nir_mod_analysis_test, const_shr_const_overflow)
{
   /* (large_const >> const_shr) % const_mod should be known if
    * const_mod << const_shr is still below UINT32_MAX.
    */
   unsigned large_const_int = 0x12345678;
   nir_def *large_const = nir_imm_int(b, large_const_int);

   for (unsigned shift = 0; shift < 30; ++shift) {
      nir_def *shr = nir_ishr(b, large_const, v[shift]);

      for (unsigned const_mod = 1; const_mod <= 1024; const_mod *= 2) {
         unsigned mod = INT32_MAX;

         if ((((uint64_t)const_mod) << shift) > UINT32_MAX) {
            EXPECT_FALSE(nir_mod_analysis_comp0(shr, nir_type_uint, const_mod, &mod));
         } else {
            EXPECT_TRUE(nir_mod_analysis_comp0(shr, nir_type_uint, const_mod, &mod));
            EXPECT_EQ(mod, (large_const_int >> shift) % const_mod);
         }
      }
   }
}
