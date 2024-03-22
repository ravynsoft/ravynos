/*
 * Copyright (C) 2022 Collabora, Ltd.
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
#include "valhall_enums.h"

#include <gtest/gtest.h>

#define CASE(test, expected)                                                   \
   do {                                                                        \
      bi_builder *A = bit_builder(mem_ctx);                                    \
      bi_builder *B = bit_builder(mem_ctx);                                    \
      {                                                                        \
         bi_builder *b = A;                                                    \
         A->shader->stage = MESA_SHADER_FRAGMENT;                              \
         test;                                                                 \
      }                                                                        \
      va_merge_flow(A->shader);                                                \
      {                                                                        \
         bi_builder *b = B;                                                    \
         B->shader->stage = MESA_SHADER_FRAGMENT;                              \
         expected;                                                             \
      }                                                                        \
      ASSERT_SHADER_EQUAL(A->shader, B->shader);                               \
   } while (0)

#define NEGCASE(test) CASE(test, test)

#define flow(f) bi_nop(b)->flow = VA_FLOW_##f

class MergeFlow : public testing::Test {
 protected:
   MergeFlow()
   {
      mem_ctx = ralloc_context(NULL);
      atest = bi_fau(BIR_FAU_ATEST_PARAM, false);
   }

   ~MergeFlow()
   {
      ralloc_free(mem_ctx);
   }

   void *mem_ctx;
   bi_instr *I;
   bi_index atest;
};

TEST_F(MergeFlow, End)
{
   CASE(
      {
         bi_fadd_f32_to(b, bi_register(0), bi_register(0), bi_register(0));
         bi_blend_to(b, bi_register(0), bi_register(4), bi_register(5),
                     bi_register(6), bi_register(7), bi_register(8),
                     BI_REGISTER_FORMAT_AUTO, 4, 4);
         flow(END);
      },
      {
         bi_fadd_f32_to(b, bi_register(0), bi_register(0), bi_register(0));
         I = bi_blend_to(b, bi_register(0), bi_register(4), bi_register(5),
                         bi_register(6), bi_register(7), bi_register(8),
                         BI_REGISTER_FORMAT_AUTO, 4, 4);
         I->flow = VA_FLOW_END;
      });
}

TEST_F(MergeFlow, Reconverge)
{
   CASE(
      {
         bi_fadd_f32_to(b, bi_register(0), bi_register(0), bi_register(0));
         bi_blend_to(b, bi_register(0), bi_register(4), bi_register(5),
                     bi_register(6), bi_register(7), bi_register(8),
                     BI_REGISTER_FORMAT_AUTO, 4, 4);
         flow(RECONVERGE);
      },
      {
         bi_fadd_f32_to(b, bi_register(0), bi_register(0), bi_register(0));
         I = bi_blend_to(b, bi_register(0), bi_register(4), bi_register(5),
                         bi_register(6), bi_register(7), bi_register(8),
                         BI_REGISTER_FORMAT_AUTO, 4, 4);
         I->flow = VA_FLOW_RECONVERGE;
      });
}

TEST_F(MergeFlow, TrivialWait)
{
   CASE(
      {
         bi_fadd_f32_to(b, bi_register(0), bi_register(0), bi_register(0));
         flow(WAIT0126);
         bi_atest_to(b, bi_register(0), bi_register(4), bi_register(5), atest);
      },
      {
         I = bi_fadd_f32_to(b, bi_register(0), bi_register(0), bi_register(0));
         I->flow = VA_FLOW_WAIT0126;
         bi_atest_to(b, bi_register(0), bi_register(4), bi_register(5), atest);
      });
}

TEST_F(MergeFlow, LoadThenUnrelatedThenUse)
{
   CASE(
      {
         bi_ld_attr_imm_to(b, bi_register(16), bi_register(60), bi_register(61),
                           BI_REGISTER_FORMAT_F32, BI_VECSIZE_V4, 1);
         bi_fadd_f32_to(b, bi_register(0), bi_register(0), bi_register(0));
         flow(WAIT0);
         bi_fadd_f32_to(b, bi_register(0), bi_register(0), bi_register(19));
         flow(END);
      },
      {
         bi_ld_attr_imm_to(b, bi_register(16), bi_register(60), bi_register(61),
                           BI_REGISTER_FORMAT_F32, BI_VECSIZE_V4, 1);
         I = bi_fadd_f32_to(b, bi_register(0), bi_register(0), bi_register(0));
         I->flow = VA_FLOW_WAIT0;
         I = bi_fadd_f32_to(b, bi_register(0), bi_register(0), bi_register(19));
         I->flow = VA_FLOW_END;
      });
}

TEST_F(MergeFlow, TrivialDiscard)
{
   CASE(
      {
         bi_fadd_f32_to(b, bi_register(0), bi_register(0), bi_register(0));
         bi_clper_i32_to(b, bi_register(0), bi_register(4), bi_register(8),
                         BI_INACTIVE_RESULT_ZERO, BI_LANE_OP_NONE,
                         BI_SUBGROUP_SUBGROUP4);
         flow(DISCARD);
         bi_fadd_f32_to(b, bi_register(0), bi_register(0), bi_register(0));
         flow(END);
      },
      {
         bi_fadd_f32_to(b, bi_register(0), bi_register(0), bi_register(0));
         I = bi_clper_i32_to(b, bi_register(0), bi_register(4), bi_register(8),
                             BI_INACTIVE_RESULT_ZERO, BI_LANE_OP_NONE,
                             BI_SUBGROUP_SUBGROUP4);
         I->flow = VA_FLOW_DISCARD;
         I = bi_fadd_f32_to(b, bi_register(0), bi_register(0), bi_register(0));
         I->flow = VA_FLOW_END;
      });
}

TEST_F(MergeFlow, TrivialDiscardAtTheStart)
{
   CASE(
      {
         flow(DISCARD);
         bi_fadd_f32_to(b, bi_register(0), bi_register(0), bi_register(0));
      },
      {
         I = bi_fadd_f32_to(b, bi_register(0), bi_register(0), bi_register(0));
         I->flow = VA_FLOW_DISCARD;
      });
}

TEST_F(MergeFlow, MoveDiscardPastWait)
{
   CASE(
      {
         bi_fadd_f32_to(b, bi_register(0), bi_register(0), bi_register(0));
         bi_clper_i32_to(b, bi_register(0), bi_register(4), bi_register(8),
                         BI_INACTIVE_RESULT_ZERO, BI_LANE_OP_NONE,
                         BI_SUBGROUP_SUBGROUP4);
         flow(DISCARD);
         flow(WAIT0);
         bi_fadd_f32_to(b, bi_register(0), bi_register(0), bi_register(0));
      },
      {
         bi_fadd_f32_to(b, bi_register(0), bi_register(0), bi_register(0));
         I = bi_clper_i32_to(b, bi_register(0), bi_register(4), bi_register(8),
                             BI_INACTIVE_RESULT_ZERO, BI_LANE_OP_NONE,
                             BI_SUBGROUP_SUBGROUP4);
         I->flow = VA_FLOW_WAIT0;
         I = bi_fadd_f32_to(b, bi_register(0), bi_register(0), bi_register(0));
         I->flow = VA_FLOW_DISCARD;
      });
}

TEST_F(MergeFlow, OccludedWaitsAndDiscard)
{
   CASE(
      {
         bi_fadd_f32_to(b, bi_register(0), bi_register(0), bi_register(0));
         bi_clper_i32_to(b, bi_register(0), bi_register(4), bi_register(8),
                         BI_INACTIVE_RESULT_ZERO, BI_LANE_OP_NONE,
                         BI_SUBGROUP_SUBGROUP4);
         flow(WAIT0);
         flow(DISCARD);
         flow(WAIT2);
         bi_fadd_f32_to(b, bi_register(0), bi_register(0), bi_register(0));
      },
      {
         bi_fadd_f32_to(b, bi_register(0), bi_register(0), bi_register(0));
         I = bi_clper_i32_to(b, bi_register(0), bi_register(4), bi_register(8),
                             BI_INACTIVE_RESULT_ZERO, BI_LANE_OP_NONE,
                             BI_SUBGROUP_SUBGROUP4);
         I->flow = VA_FLOW_WAIT02;
         I = bi_fadd_f32_to(b, bi_register(0), bi_register(0), bi_register(0));
         I->flow = VA_FLOW_DISCARD;
      });
}

TEST_F(MergeFlow, DeleteUselessWaits)
{
   CASE(
      {
         bi_fadd_f32_to(b, bi_register(0), bi_register(0), bi_register(0));
         bi_fadd_f32_to(b, bi_register(0), bi_register(0), bi_register(0));
         flow(WAIT0);
         flow(WAIT2);
         flow(END);
      },
      {
         bi_fadd_f32_to(b, bi_register(0), bi_register(0), bi_register(0));
         I = bi_fadd_f32_to(b, bi_register(0), bi_register(0), bi_register(0));
         I->flow = VA_FLOW_END;
      });
}

TEST_F(MergeFlow, BlockFullOfUselessWaits)
{
   CASE(
      {
         flow(WAIT0);
         flow(WAIT2);
         flow(DISCARD);
         flow(END);
      },
      { flow(END); });
}

TEST_F(MergeFlow, WaitWithMessage)
{
   CASE(
      {
         bi_ld_attr_imm_to(b, bi_register(16), bi_register(60), bi_register(61),
                           BI_REGISTER_FORMAT_F32, BI_VECSIZE_V4, 1);
         flow(WAIT0);
      },
      {
         I = bi_ld_attr_imm_to(b, bi_register(16), bi_register(60),
                               bi_register(61), BI_REGISTER_FORMAT_F32,
                               BI_VECSIZE_V4, 1);
         I->flow = VA_FLOW_WAIT0;
      });
}

TEST_F(MergeFlow, CantMoveWaitPastMessage)
{
   NEGCASE({
      bi_fadd_f32_to(b, bi_register(0), bi_register(0), bi_register(0));
      I =
         bi_ld_attr_imm_to(b, bi_register(16), bi_register(60), bi_register(61),
                           BI_REGISTER_FORMAT_F32, BI_VECSIZE_V4, 1);

      /* Pretend it's blocked for some reason. This doesn't actually happen
       * with the current algorithm, but it's good to handle the special
       * cases correctly in case we change later on.
       */
      I->flow = VA_FLOW_DISCARD;
      flow(WAIT0);
   });
}

TEST_F(MergeFlow, DeletePointlessDiscard)
{
   CASE(
      {
         bi_fadd_f32_to(b, bi_register(0), bi_register(0), bi_register(0));
         bi_tex_single_to(b, bi_register(0), bi_register(4), bi_register(8),
                          bi_register(12), false, BI_DIMENSION_2D,
                          BI_REGISTER_FORMAT_F32, false, false,
                          BI_VA_LOD_MODE_COMPUTED_LOD, BI_WRITE_MASK_RGBA, 4);
         flow(DISCARD);
         flow(WAIT0);
         flow(WAIT0126);
         bi_atest_to(b, bi_register(0), bi_register(4), bi_register(5), atest);
         flow(WAIT);
         bi_blend_to(b, bi_register(0), bi_register(4), bi_register(5),
                     bi_register(6), bi_register(7), bi_register(8),
                     BI_REGISTER_FORMAT_AUTO, 4, 4);
         flow(END);
      },
      {
         bi_fadd_f32_to(b, bi_register(0), bi_register(0), bi_register(0));
         I = bi_tex_single_to(
            b, bi_register(0), bi_register(4), bi_register(8), bi_register(12),
            false, BI_DIMENSION_2D, BI_REGISTER_FORMAT_F32, false, false,
            BI_VA_LOD_MODE_COMPUTED_LOD, BI_WRITE_MASK_RGBA, 4);
         I->flow = VA_FLOW_WAIT0126;
         I = bi_atest_to(b, bi_register(0), bi_register(4), bi_register(5),
                         atest);
         I->flow = VA_FLOW_WAIT;
         I = bi_blend_to(b, bi_register(0), bi_register(4), bi_register(5),
                         bi_register(6), bi_register(7), bi_register(8),
                         BI_REGISTER_FORMAT_AUTO, 4, 4);
         I->flow = VA_FLOW_END;
      });
}

TEST_F(MergeFlow, PreserveTerminalBarriers)
{
   CASE(
      {
         bi_barrier(b);
         flow(WAIT);
         flow(END);
      },
      {
         bi_barrier(b)->flow = VA_FLOW_WAIT;
         flow(END);
      });
}
