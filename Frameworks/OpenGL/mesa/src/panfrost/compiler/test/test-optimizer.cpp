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

static void
bi_optimizer(bi_context *ctx)
{
   bi_opt_mod_prop_forward(ctx);
   bi_opt_mod_prop_backward(ctx);
   bi_opt_dead_code_eliminate(ctx);
}

/* Define reg first so it has a consistent variable index, and pass it to an
 * instruction that cannot be dead code eliminated so the program is nontrivial.
 */
#define CASE(instr, expected)                                                  \
   INSTRUCTION_CASE(                                                           \
      {                                                                        \
         UNUSED bi_index reg = bi_temp(b->shader);                             \
         instr;                                                                \
         bi_kaboom(b, reg);                                                    \
      },                                                                       \
      {                                                                        \
         UNUSED bi_index reg = bi_temp(b->shader);                             \
         expected;                                                             \
         bi_kaboom(b, reg);                                                    \
      },                                                                       \
      bi_optimizer);

#define NEGCASE(instr) CASE(instr, instr)

class Optimizer : public testing::Test {
 protected:
   Optimizer()
   {
      mem_ctx = ralloc_context(NULL);

      x = bi_register(1);
      y = bi_register(2);
      negabsx = bi_neg(bi_abs(x));
   }

   ~Optimizer()
   {
      ralloc_free(mem_ctx);
   }

   void *mem_ctx;

   bi_index x;
   bi_index y;
   bi_index negabsx;
};

TEST_F(Optimizer, FusedFABSNEG)
{
   CASE(bi_fadd_f32_to(b, reg, bi_fabsneg_f32(b, bi_abs(x)), y),
        bi_fadd_f32_to(b, reg, bi_abs(x), y));

   CASE(bi_fadd_f32_to(b, reg, bi_fabsneg_f32(b, bi_neg(x)), y),
        bi_fadd_f32_to(b, reg, bi_neg(x), y));

   CASE(bi_fadd_f32_to(b, reg, bi_fabsneg_f32(b, negabsx), y),
        bi_fadd_f32_to(b, reg, negabsx, y));

   CASE(bi_fadd_f32_to(b, reg, bi_fabsneg_f32(b, x), y),
        bi_fadd_f32_to(b, reg, x, y));

   CASE(bi_fmin_f32_to(b, reg, bi_fabsneg_f32(b, negabsx), bi_neg(y)),
        bi_fmin_f32_to(b, reg, negabsx, bi_neg(y)));
}

TEST_F(Optimizer, FusedFABSNEGForFP16)
{
   CASE(bi_fadd_v2f16_to(b, reg, bi_fabsneg_v2f16(b, negabsx), y),
        bi_fadd_v2f16_to(b, reg, negabsx, y));

   CASE(bi_fmin_v2f16_to(b, reg, bi_fabsneg_v2f16(b, negabsx), bi_neg(y)),
        bi_fmin_v2f16_to(b, reg, negabsx, bi_neg(y)));
}

TEST_F(Optimizer, FuseFADD_F32WithEqualSourcesAbsAbsAndClamp)
{
   CASE(
      {
         bi_instr *I =
            bi_fadd_f32_to(b, reg, bi_fabsneg_f32(b, bi_abs(x)), bi_abs(x));
         I->clamp = BI_CLAMP_CLAMP_0_1;
      },
      {
         bi_instr *I = bi_fadd_f32_to(b, reg, bi_abs(x), bi_abs(x));
         I->clamp = BI_CLAMP_CLAMP_0_1;
      });

   CASE(
      {
         bi_instr *I =
            bi_fadd_f32_to(b, reg, bi_abs(x), bi_fabsneg_f32(b, bi_abs(x)));
         I->clamp = BI_CLAMP_CLAMP_0_1;
      },
      {
         bi_instr *I = bi_fadd_f32_to(b, reg, bi_abs(x), bi_abs(x));
         I->clamp = BI_CLAMP_CLAMP_0_1;
      });

   CASE(
      {
         bi_instr *I =
            bi_fclamp_f32_to(b, reg, bi_fadd_f32(b, bi_abs(x), bi_abs(x)));
         I->clamp = BI_CLAMP_CLAMP_0_INF;
      },
      {
         bi_instr *I = bi_fadd_f32_to(b, reg, bi_abs(x), bi_abs(x));
         I->clamp = BI_CLAMP_CLAMP_0_INF;
      });
}

TEST_F(Optimizer, FuseFADD_V2F16WithDifferentSourcesAbsAbsAndClamp)
{
   CASE(
      {
         bi_instr *I =
            bi_fadd_v2f16_to(b, reg, bi_fabsneg_v2f16(b, bi_abs(x)), bi_abs(y));
         I->clamp = BI_CLAMP_CLAMP_0_1;
      },
      {
         bi_instr *I = bi_fadd_v2f16_to(b, reg, bi_abs(x), bi_abs(y));
         I->clamp = BI_CLAMP_CLAMP_0_1;
      });

   CASE(
      {
         bi_instr *I =
            bi_fadd_v2f16_to(b, reg, bi_abs(x), bi_fabsneg_v2f16(b, bi_abs(y)));
         I->clamp = BI_CLAMP_CLAMP_0_1;
      },
      {
         bi_instr *I = bi_fadd_v2f16_to(b, reg, bi_abs(x), bi_abs(y));
         I->clamp = BI_CLAMP_CLAMP_0_1;
      });

   CASE(
      {
         bi_instr *I =
            bi_fclamp_v2f16_to(b, reg, bi_fadd_v2f16(b, bi_abs(x), bi_abs(y)));
         I->clamp = BI_CLAMP_CLAMP_0_INF;
      },
      {
         bi_instr *I = bi_fadd_v2f16_to(b, reg, bi_abs(x), bi_abs(y));
         I->clamp = BI_CLAMP_CLAMP_0_INF;
      });
}

TEST_F(Optimizer, AvoidFADD_V2F16WithEqualSourcesAbsAbsAndClamp)
{
   NEGCASE({
      bi_instr *I =
         bi_fadd_v2f16_to(b, reg, bi_fabsneg_v2f16(b, bi_abs(x)), bi_abs(x));
      I->clamp = BI_CLAMP_CLAMP_0_1;
   });

   NEGCASE({
      bi_instr *I =
         bi_fadd_v2f16_to(b, reg, bi_abs(x), bi_fabsneg_v2f16(b, bi_abs(x)));
      I->clamp = BI_CLAMP_CLAMP_0_1;
   });

   NEGCASE({
      bi_instr *I =
         bi_fclamp_v2f16_to(b, reg, bi_fadd_v2f16(b, bi_abs(x), bi_abs(x)));
      I->clamp = BI_CLAMP_CLAMP_0_INF;
   });
}

TEST_F(Optimizer, SwizzlesComposedForFP16)
{
   CASE(bi_fadd_v2f16_to(
           b, reg, bi_fabsneg_v2f16(b, bi_swz_16(negabsx, true, false)), y),
        bi_fadd_v2f16_to(b, reg, bi_swz_16(negabsx, true, false), y));

   CASE(bi_fadd_v2f16_to(
           b, reg, bi_swz_16(bi_fabsneg_v2f16(b, negabsx), true, false), y),
        bi_fadd_v2f16_to(b, reg, bi_swz_16(negabsx, true, false), y));

   CASE(bi_fadd_v2f16_to(
           b, reg,
           bi_swz_16(bi_fabsneg_v2f16(b, bi_swz_16(negabsx, true, false)), true,
                     false),
           y),
        bi_fadd_v2f16_to(b, reg, negabsx, y));

   CASE(bi_fadd_v2f16_to(
           b, reg,
           bi_swz_16(bi_fabsneg_v2f16(b, bi_half(negabsx, false)), true, false),
           y),
        bi_fadd_v2f16_to(b, reg, bi_half(negabsx, false), y));

   CASE(bi_fadd_v2f16_to(
           b, reg,
           bi_swz_16(bi_fabsneg_v2f16(b, bi_half(negabsx, true)), true, false),
           y),
        bi_fadd_v2f16_to(b, reg, bi_half(negabsx, true), y));
}

TEST_F(Optimizer, PreserveWidens)
{
   /* Check that widens are passed through */
   CASE(bi_fadd_f32_to(b, reg, bi_fabsneg_f32(b, bi_half(negabsx, false)), y),
        bi_fadd_f32_to(b, reg, bi_half(negabsx, false), y));

   CASE(bi_fadd_f32_to(b, reg, bi_fabsneg_f32(b, bi_half(negabsx, true)), y),
        bi_fadd_f32_to(b, reg, bi_half(negabsx, true), y));

   CASE(bi_fadd_f32_to(b, reg, bi_fabsneg_f32(b, bi_half(x, true)),
                       bi_fabsneg_f32(b, bi_half(x, false))),
        bi_fadd_f32_to(b, reg, bi_half(x, true), bi_half(x, false)));
}

TEST_F(Optimizer, DoNotMixSizesForFABSNEG)
{
   /* Refuse to mix sizes for fabsneg, that's wrong */
   NEGCASE(bi_fadd_f32_to(b, reg, bi_fabsneg_v2f16(b, negabsx), y));
   NEGCASE(bi_fadd_v2f16_to(b, reg, bi_fabsneg_f32(b, negabsx), y));
}

TEST_F(Optimizer, AvoidZeroAndFABSNEGFootguns)
{
   /* It's tempting to use addition by 0.0 as the absneg primitive, but that
    * has footguns around signed zero and round modes. Check we don't
    * incorrectly fuse these rules. */

   bi_index zero = bi_zero();

   NEGCASE(bi_fadd_f32_to(b, reg, bi_fadd_f32(b, bi_abs(x), zero), y));
   NEGCASE(bi_fadd_f32_to(b, reg, bi_fadd_f32(b, bi_neg(x), zero), y));
   NEGCASE(bi_fadd_f32_to(b, reg, bi_fadd_f32(b, bi_neg(bi_abs(x)), zero), y));
   NEGCASE(bi_fadd_f32_to(b, reg, bi_fadd_f32(b, x, zero), y));
}

TEST_F(Optimizer, ClampsPropagated)
{
   CASE(
      {
         bi_instr *I = bi_fclamp_f32_to(b, reg, bi_fadd_f32(b, x, y));
         I->clamp = BI_CLAMP_CLAMP_0_INF;
      },
      {
         bi_instr *I = bi_fadd_f32_to(b, reg, x, y);
         I->clamp = BI_CLAMP_CLAMP_0_INF;
      });

   CASE(
      {
         bi_instr *I = bi_fclamp_v2f16_to(b, reg, bi_fadd_v2f16(b, x, y));
         I->clamp = BI_CLAMP_CLAMP_0_1;
      },
      {
         bi_instr *I = bi_fadd_v2f16_to(b, reg, x, y);
         I->clamp = BI_CLAMP_CLAMP_0_1;
      });
}

TEST_F(Optimizer, ClampsComposed)
{
   CASE(
      {
         bi_instr *I = bi_fadd_f32_to(b, bi_temp(b->shader), x, y);
         bi_instr *J = bi_fclamp_f32_to(b, reg, I->dest[0]);
         I->clamp = BI_CLAMP_CLAMP_M1_1;
         J->clamp = BI_CLAMP_CLAMP_0_INF;
      },
      {
         bi_instr *I = bi_fadd_f32_to(b, reg, x, y);
         I->clamp = BI_CLAMP_CLAMP_0_1;
      });

   CASE(
      {
         bi_instr *I = bi_fadd_f32_to(b, bi_temp(b->shader), x, y);
         bi_instr *J = bi_fclamp_f32_to(b, reg, I->dest[0]);
         I->clamp = BI_CLAMP_CLAMP_0_1;
         J->clamp = BI_CLAMP_CLAMP_0_INF;
      },
      {
         bi_instr *I = bi_fadd_f32_to(b, reg, x, y);
         I->clamp = BI_CLAMP_CLAMP_0_1;
      });

   CASE(
      {
         bi_instr *I = bi_fadd_f32_to(b, bi_temp(b->shader), x, y);
         bi_instr *J = bi_fclamp_f32_to(b, reg, I->dest[0]);
         I->clamp = BI_CLAMP_CLAMP_0_INF;
         J->clamp = BI_CLAMP_CLAMP_0_INF;
      },
      {
         bi_instr *I = bi_fadd_f32_to(b, reg, x, y);
         I->clamp = BI_CLAMP_CLAMP_0_INF;
      });

   CASE(
      {
         bi_instr *I = bi_fadd_v2f16_to(b, bi_temp(b->shader), x, y);
         bi_instr *J = bi_fclamp_v2f16_to(b, reg, I->dest[0]);
         I->clamp = BI_CLAMP_CLAMP_M1_1;
         J->clamp = BI_CLAMP_CLAMP_0_INF;
      },
      {
         bi_instr *I = bi_fadd_v2f16_to(b, reg, x, y);
         I->clamp = BI_CLAMP_CLAMP_0_1;
      });

   CASE(
      {
         bi_instr *I = bi_fadd_v2f16_to(b, bi_temp(b->shader), x, y);
         bi_instr *J = bi_fclamp_v2f16_to(b, reg, I->dest[0]);
         I->clamp = BI_CLAMP_CLAMP_0_1;
         J->clamp = BI_CLAMP_CLAMP_0_INF;
      },
      {
         bi_instr *I = bi_fadd_v2f16_to(b, reg, x, y);
         I->clamp = BI_CLAMP_CLAMP_0_1;
      });

   CASE(
      {
         bi_instr *I = bi_fadd_v2f16_to(b, bi_temp(b->shader), x, y);
         bi_instr *J = bi_fclamp_v2f16_to(b, reg, I->dest[0]);
         I->clamp = BI_CLAMP_CLAMP_0_INF;
         J->clamp = BI_CLAMP_CLAMP_0_INF;
      },
      {
         bi_instr *I = bi_fadd_v2f16_to(b, reg, x, y);
         I->clamp = BI_CLAMP_CLAMP_0_INF;
      });
}

TEST_F(Optimizer, DoNotMixSizesWhenClamping)
{
   NEGCASE({
      bi_instr *I = bi_fclamp_f32_to(b, reg, bi_fadd_v2f16(b, x, y));
      I->clamp = BI_CLAMP_CLAMP_0_1;
   });

   NEGCASE({
      bi_instr *I = bi_fclamp_v2f16_to(b, reg, bi_fadd_f32(b, x, y));
      I->clamp = BI_CLAMP_CLAMP_0_1;
   });
}

TEST_F(Optimizer, DoNotUseAdditionByZeroForClamps)
{
   bi_index zero = bi_zero();

   /* We can't use addition by 0.0 for clamps due to signed zeros. */
   NEGCASE({
      bi_instr *I = bi_fadd_f32_to(b, reg, bi_fadd_f32(b, x, y), zero);
      I->clamp = BI_CLAMP_CLAMP_M1_1;
   });

   NEGCASE({
      bi_instr *I = bi_fadd_v2f16_to(b, reg, bi_fadd_v2f16(b, x, y), zero);
      I->clamp = BI_CLAMP_CLAMP_0_1;
   });
}

TEST_F(Optimizer, FuseComparisonsWithDISCARD)
{
   CASE(bi_discard_b32(b, bi_fcmp_f32(b, x, y, BI_CMPF_LE, BI_RESULT_TYPE_F1)),
        bi_discard_f32(b, x, y, BI_CMPF_LE));

   CASE(bi_discard_b32(b, bi_fcmp_f32(b, x, y, BI_CMPF_NE, BI_RESULT_TYPE_I1)),
        bi_discard_f32(b, x, y, BI_CMPF_NE));

   CASE(bi_discard_b32(b, bi_fcmp_f32(b, x, y, BI_CMPF_EQ, BI_RESULT_TYPE_M1)),
        bi_discard_f32(b, x, y, BI_CMPF_EQ));

   for (unsigned h = 0; h < 2; ++h) {
      CASE(bi_discard_b32(
              b, bi_half(bi_fcmp_v2f16(b, x, y, BI_CMPF_LE, BI_RESULT_TYPE_F1),
                         h)),
           bi_discard_f32(b, bi_half(x, h), bi_half(y, h), BI_CMPF_LE));

      CASE(bi_discard_b32(
              b, bi_half(bi_fcmp_v2f16(b, x, y, BI_CMPF_NE, BI_RESULT_TYPE_I1),
                         h)),
           bi_discard_f32(b, bi_half(x, h), bi_half(y, h), BI_CMPF_NE));

      CASE(bi_discard_b32(
              b, bi_half(bi_fcmp_v2f16(b, x, y, BI_CMPF_EQ, BI_RESULT_TYPE_M1),
                         h)),
           bi_discard_f32(b, bi_half(x, h), bi_half(y, h), BI_CMPF_EQ));
   }
}

TEST_F(Optimizer, DoNotFuseSpecialComparisons)
{
   NEGCASE(
      bi_discard_b32(b, bi_fcmp_f32(b, x, y, BI_CMPF_GTLT, BI_RESULT_TYPE_F1)));
   NEGCASE(bi_discard_b32(
      b, bi_fcmp_f32(b, x, y, BI_CMPF_TOTAL, BI_RESULT_TYPE_F1)));
}

TEST_F(Optimizer, FuseResultType)
{
   CASE(bi_mux_i32_to(b, reg, bi_imm_f32(0.0), bi_imm_f32(1.0),
                      bi_fcmp_f32(b, x, y, BI_CMPF_LE, BI_RESULT_TYPE_M1),
                      BI_MUX_INT_ZERO),
        bi_fcmp_f32_to(b, reg, x, y, BI_CMPF_LE, BI_RESULT_TYPE_F1));

   CASE(bi_mux_i32_to(
           b, reg, bi_imm_f32(0.0), bi_imm_f32(1.0),
           bi_fcmp_f32(b, bi_abs(x), bi_neg(y), BI_CMPF_LE, BI_RESULT_TYPE_M1),
           BI_MUX_INT_ZERO),
        bi_fcmp_f32_to(b, reg, bi_abs(x), bi_neg(y), BI_CMPF_LE,
                       BI_RESULT_TYPE_F1));

   CASE(bi_mux_i32_to(
           b, reg, bi_imm_u32(0), bi_imm_u32(1),
           bi_fcmp_f32(b, bi_abs(x), bi_neg(y), BI_CMPF_LE, BI_RESULT_TYPE_M1),
           BI_MUX_INT_ZERO),
        bi_fcmp_f32_to(b, reg, bi_abs(x), bi_neg(y), BI_CMPF_LE,
                       BI_RESULT_TYPE_I1));

   CASE(bi_mux_v2i16_to(b, reg, bi_imm_f16(0.0), bi_imm_f16(1.0),
                        bi_fcmp_v2f16(b, bi_abs(x), bi_neg(y), BI_CMPF_LE,
                                      BI_RESULT_TYPE_M1),
                        BI_MUX_INT_ZERO),
        bi_fcmp_v2f16_to(b, reg, bi_abs(x), bi_neg(y), BI_CMPF_LE,
                         BI_RESULT_TYPE_F1));

   CASE(bi_mux_v2i16_to(b, reg, bi_imm_u16(0), bi_imm_u16(1),
                        bi_fcmp_v2f16(b, bi_abs(x), bi_neg(y), BI_CMPF_LE,
                                      BI_RESULT_TYPE_M1),
                        BI_MUX_INT_ZERO),
        bi_fcmp_v2f16_to(b, reg, bi_abs(x), bi_neg(y), BI_CMPF_LE,
                         BI_RESULT_TYPE_I1));

   CASE(bi_mux_i32_to(b, reg, bi_imm_u32(0), bi_imm_u32(1),
                      bi_icmp_u32(b, x, y, BI_CMPF_LE, BI_RESULT_TYPE_M1),
                      BI_MUX_INT_ZERO),
        bi_icmp_u32_to(b, reg, x, y, BI_CMPF_LE, BI_RESULT_TYPE_I1));

   CASE(bi_mux_v2i16_to(b, reg, bi_imm_u16(0), bi_imm_u16(1),
                        bi_icmp_v2u16(b, x, y, BI_CMPF_LE, BI_RESULT_TYPE_M1),
                        BI_MUX_INT_ZERO),
        bi_icmp_v2u16_to(b, reg, x, y, BI_CMPF_LE, BI_RESULT_TYPE_I1));

   CASE(bi_mux_v4i8_to(b, reg, bi_imm_u8(0), bi_imm_u8(1),
                       bi_icmp_v4u8(b, x, y, BI_CMPF_LE, BI_RESULT_TYPE_M1),
                       BI_MUX_INT_ZERO),
        bi_icmp_v4u8_to(b, reg, x, y, BI_CMPF_LE, BI_RESULT_TYPE_I1));

   CASE(bi_mux_i32_to(b, reg, bi_imm_u32(0), bi_imm_u32(1),
                      bi_icmp_s32(b, x, y, BI_CMPF_LE, BI_RESULT_TYPE_M1),
                      BI_MUX_INT_ZERO),
        bi_icmp_s32_to(b, reg, x, y, BI_CMPF_LE, BI_RESULT_TYPE_I1));

   CASE(bi_mux_v2i16_to(b, reg, bi_imm_u16(0), bi_imm_u16(1),
                        bi_icmp_v2s16(b, x, y, BI_CMPF_LE, BI_RESULT_TYPE_M1),
                        BI_MUX_INT_ZERO),
        bi_icmp_v2s16_to(b, reg, x, y, BI_CMPF_LE, BI_RESULT_TYPE_I1));

   CASE(bi_mux_v4i8_to(b, reg, bi_imm_u8(0), bi_imm_u8(1),
                       bi_icmp_v4s8(b, x, y, BI_CMPF_LE, BI_RESULT_TYPE_M1),
                       BI_MUX_INT_ZERO),
        bi_icmp_v4s8_to(b, reg, x, y, BI_CMPF_LE, BI_RESULT_TYPE_I1));
}

TEST_F(Optimizer, DoNotFuseMixedSizeResultType)
{
   NEGCASE(bi_mux_i32_to(
      b, reg, bi_imm_f32(0.0), bi_imm_f32(1.0),
      bi_fcmp_v2f16(b, bi_abs(x), bi_neg(y), BI_CMPF_LE, BI_RESULT_TYPE_M1),
      BI_MUX_INT_ZERO));

   NEGCASE(bi_mux_v2i16_to(
      b, reg, bi_imm_f16(0.0), bi_imm_f16(1.0),
      bi_fcmp_f32(b, bi_abs(x), bi_neg(y), BI_CMPF_LE, BI_RESULT_TYPE_M1),
      BI_MUX_INT_ZERO));
}

TEST_F(Optimizer, VarTexCoord32)
{
   CASE(
      {
         bi_index ld =
            bi_ld_var_imm(b, bi_null(), BI_REGISTER_FORMAT_F32,
                          BI_SAMPLE_CENTER, BI_UPDATE_STORE, BI_VECSIZE_V2, 0);

         bi_index x = bi_temp(b->shader);
         bi_index y = bi_temp(b->shader);
         bi_instr *split = bi_split_i32_to(b, 2, ld);
         split->dest[0] = x;
         split->dest[1] = y;

         bi_texs_2d_f32_to(b, reg, x, y, false, 0, 0);
      },
      {
         bi_var_tex_f32_to(b, reg, false, BI_SAMPLE_CENTER, BI_UPDATE_STORE, 0,
                           0);
      });
}

TEST_F(Optimizer, Int8ToFloat32)
{
   for (unsigned i = 0; i < 4; ++i) {
      CASE(bi_s32_to_f32_to(b, reg, bi_s8_to_s32(b, bi_byte(x, i))),
           bi_s8_to_f32_to(b, reg, bi_byte(x, i)));

      CASE(bi_s32_to_f32_to(b, reg, bi_u8_to_u32(b, bi_byte(x, i))),
           bi_u8_to_f32_to(b, reg, bi_byte(x, i)));

      CASE(bi_u32_to_f32_to(b, reg, bi_u8_to_u32(b, bi_byte(x, i))),
           bi_u8_to_f32_to(b, reg, bi_byte(x, i)));
   }
}

TEST_F(Optimizer, Int16ToFloat32)
{
   for (unsigned i = 0; i < 2; ++i) {
      CASE(bi_s32_to_f32_to(b, reg, bi_s16_to_s32(b, bi_half(x, i))),
           bi_s16_to_f32_to(b, reg, bi_half(x, i)));

      CASE(bi_s32_to_f32_to(b, reg, bi_u16_to_u32(b, bi_half(x, i))),
           bi_u16_to_f32_to(b, reg, bi_half(x, i)));

      CASE(bi_u32_to_f32_to(b, reg, bi_u16_to_u32(b, bi_half(x, i))),
           bi_u16_to_f32_to(b, reg, bi_half(x, i)));
   }
}
