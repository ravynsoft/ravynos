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

#define R(x)  bi_register(x)
#define DR(x) bi_discard(R(x))

static void
strip_discard(bi_context *ctx)
{
   bi_foreach_instr_global(ctx, I) {
      bi_foreach_src(I, s)
         I->src[s].discard = false;
   }
}

#define CASE(test)                                                             \
   do {                                                                        \
      void *mem_ctx = ralloc_context(NULL);                                    \
      bi_builder *A = bit_builder(mem_ctx);                                    \
      bi_builder *B = bit_builder(mem_ctx);                                    \
      {                                                                        \
         UNUSED bi_builder *b = A;                                             \
         test;                                                                 \
      }                                                                        \
      strip_discard(A->shader);                                                \
      va_mark_last(A->shader);                                                 \
      {                                                                        \
         UNUSED bi_builder *b = B;                                             \
         test;                                                                 \
      }                                                                        \
      ASSERT_SHADER_EQUAL(A->shader, B->shader);                               \
      ralloc_free(mem_ctx);                                                    \
   } while (0)

TEST(MarkLast, Simple)
{
   CASE(bi_fadd_f32_to(b, R(0), DR(0), DR(1)));

   CASE({
      bi_fadd_f32_to(b, R(2), R(0), DR(1));
      bi_fadd_f32_to(b, R(0), DR(0), DR(2));
   });
}

TEST(MarkLast, SameSourceAndDestination)
{
   CASE({
      bi_fadd_f32_to(b, R(0), DR(0), DR(0));
      bi_fadd_f32_to(b, R(0), DR(0), DR(0));
      bi_fadd_f32_to(b, R(0), DR(0), DR(0));
   });
}

TEST(MarkLast, StagingReadBefore)
{
   CASE({
      bi_fadd_f32_to(b, R(9), R(2), DR(7));
      bi_st_tile(b, R(0), DR(4), DR(5), DR(6), BI_REGISTER_FORMAT_F32,
                 BI_VECSIZE_V4);
   });
}

TEST(MarkLast, StagingReadAfter)
{
   CASE({
      bi_st_tile(b, R(0), DR(4), DR(5), DR(6), BI_REGISTER_FORMAT_F32,
                 BI_VECSIZE_V4);
      bi_fadd_f32_to(b, R(9), R(2), DR(7));
   });
}

TEST(MarkLast, NonstagingSourceToAsync)
{
   CASE({
      bi_st_tile(b, R(0), R(4), R(5), DR(6), BI_REGISTER_FORMAT_F32,
                 BI_VECSIZE_V4);
      bi_fadd_f32_to(b, R(9), DR(4), DR(5));
   });
}

TEST(MarkLast, Both64)
{
   CASE(bi_load_i32_to(b, R(0), DR(8), DR(9), BI_SEG_NONE, 0));
}

TEST(MarkLast, Neither64ThenBoth)
{
   CASE({
      bi_load_i32_to(b, R(0), R(8), R(9), BI_SEG_NONE, 0);
      bi_load_i32_to(b, R(1), DR(8), DR(9), BI_SEG_NONE, 8);
   });
}

TEST(MarkLast, Half64)
{
   CASE({
      bi_load_i32_to(b, R(0), R(8), R(9), BI_SEG_NONE, 0);
      bi_fadd_f32_to(b, R(8), DR(8), DR(8));
   });

   CASE({
      bi_load_i32_to(b, R(0), R(8), R(9), BI_SEG_NONE, 0);
      bi_fadd_f32_to(b, R(9), DR(9), DR(9));
   });
}

TEST(MarkLast, RegisterBlendDescriptor)
{
   CASE({
      bi_blend_to(b, R(48), R(0), DR(60), DR(4), DR(5), bi_null(),
                  BI_REGISTER_FORMAT_F32, 4, 0);
   });

   CASE({
      bi_blend_to(b, R(48), R(0), DR(60), R(4), R(5), bi_null(),
                  BI_REGISTER_FORMAT_F32, 4, 0);
      bi_fadd_f32_to(b, R(4), DR(4), DR(7));
   });

   CASE({
      bi_blend_to(b, R(48), R(0), DR(60), R(4), R(5), bi_null(),
                  BI_REGISTER_FORMAT_F32, 4, 0);
      bi_fadd_f32_to(b, R(4), DR(5), DR(7));
   });
}

TEST(MarkLast, ControlFlowAllFeatures)
{
   /*      A
    *     / \
    *    B   C
    */
   CASE({
      bi_block *A = bi_start_block(&b->shader->blocks);
      bi_block *B = bit_block(b->shader);
      bi_block *C = bit_block(b->shader);

      bi_block_add_successor(A, B);
      bi_block_add_successor(A, C);

      b->cursor = bi_after_block(A);
      {
         bi_instr *I = bi_st_tile(b, R(10), DR(14), DR(15), DR(16),
                                  BI_REGISTER_FORMAT_F32, BI_VECSIZE_V4);
         I->slot = 2;

         bi_load_i32_to(b, R(20), R(28), R(29), BI_SEG_NONE, 0);

         bi_fadd_f32_to(b, R(0), DR(0), R(3));
         bi_fadd_f32_to(b, R(0), DR(0), R(7));
         bi_fma_f32_to(b, R(2), R(0), R(1), DR(4));
      }

      b->cursor = bi_after_block(B);
      {
         bi_fadd_f32_to(b, R(2), DR(0), DR(2));
         bi_fadd_f32_to(b, R(2), DR(2), DR(3));
         bi_fadd_f32_to(b, R(2), DR(2), DR(7));
         bi_fadd_f32_to(b, R(2), DR(2), DR(29));
      }

      b->cursor = bi_after_block(C);
      {
         bi_fadd_f32_to(b, R(2), DR(1), DR(2));
         bi_fadd_f32_to(b, R(2), DR(2), DR(7));

         bi_instr *I = bi_fadd_f32_to(b, R(2), DR(2), R(13));
         I->flow = VA_FLOW_WAIT2;

         bi_fadd_f32_to(b, R(2), DR(2), DR(11));
      }
   });
}
