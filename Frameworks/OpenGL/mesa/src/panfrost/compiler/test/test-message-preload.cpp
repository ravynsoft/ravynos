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
#include "compiler.h"

#include <gtest/gtest.h>

#define CASE(instr, expected)                                                  \
   do {                                                                        \
      bi_builder *A = bit_builder(mem_ctx);                                    \
      bi_builder *B = bit_builder(mem_ctx);                                    \
      A->shader->info.bifrost = rzalloc(mem_ctx, struct bifrost_shader_info);  \
      B->shader->info.bifrost = rzalloc(mem_ctx, struct bifrost_shader_info);  \
      {                                                                        \
         bi_builder *b = A;                                                    \
         bi_index u = bi_temp(b->shader);                                      \
         UNUSED bi_index v = bi_temp(b->shader);                               \
         UNUSED bi_index w = bi_temp(b->shader);                               \
         instr;                                                                \
      }                                                                        \
      {                                                                        \
         bi_builder *b = B;                                                    \
         bi_index u = bi_temp(b->shader);                                      \
         UNUSED bi_index v = bi_temp(b->shader);                               \
         UNUSED bi_index w = bi_temp(b->shader);                               \
         expected;                                                             \
      }                                                                        \
      bi_opt_message_preload(A->shader);                                       \
      if (!bit_shader_equal(A->shader, B->shader)) {                           \
         ADD_FAILURE();                                                        \
         fprintf(stderr, "Optimization produce unexpected result");            \
         fprintf(stderr, "  Actual:\n");                                       \
         bi_print_shader(A->shader, stderr);                                   \
         fprintf(stderr, "Expected:\n");                                       \
         bi_print_shader(B->shader, stderr);                                   \
         fprintf(stderr, "\n");                                                \
      }                                                                        \
   } while (0)

#define NEGCASE(instr) CASE(instr, instr)

class MessagePreload : public testing::Test {
 protected:
   MessagePreload()
   {
      mem_ctx = ralloc_context(NULL);

      x = bi_register(16);
      y = bi_register(32);
   }

   ~MessagePreload()
   {
      ralloc_free(mem_ctx);
   }

   void *mem_ctx;

   bi_index reg, x, y;

   static void preload_moves(bi_builder *b, bi_index dest, int count, int idx)
   {
      bi_instr *I = bi_collect_i32_to(b, dest, count);

      b->cursor = bi_before_block(bi_start_block(&b->shader->blocks));
      bi_foreach_src(I, i)
         I->src[i] = bi_mov_i32(b, bi_register(idx * 4 + i));

      b->cursor = bi_after_instr(I);
   }
};

TEST_F(MessagePreload, PreloadLdVarSample)
{
   CASE(
      {
         bi_ld_var_imm_to(b, u, bi_register(61), BI_REGISTER_FORMAT_F32,
                          BI_SAMPLE_SAMPLE, BI_UPDATE_STORE, BI_VECSIZE_V4, 0);
      },
      { preload_moves(b, u, 4, 0); });
}

TEST_F(MessagePreload, PreloadLdVarLdVar)
{
   CASE(
      {
         bi_ld_var_imm_to(b, u, bi_register(61), BI_REGISTER_FORMAT_F32,
                          BI_SAMPLE_SAMPLE, BI_UPDATE_STORE, BI_VECSIZE_V4, 2);
         bi_ld_var_imm_to(b, v, bi_register(61), BI_REGISTER_FORMAT_F32,
                          BI_SAMPLE_SAMPLE, BI_UPDATE_STORE, BI_VECSIZE_V4, 1);
      },
      {
         preload_moves(b, u, 4, 0);
         preload_moves(b, v, 4, 1);
      });
}

TEST_F(MessagePreload, MaxTwoMessages)
{
   CASE(
      {
         bi_ld_var_imm_to(b, u, bi_register(61), BI_REGISTER_FORMAT_F32,
                          BI_SAMPLE_SAMPLE, BI_UPDATE_STORE, BI_VECSIZE_V4, 2);
         bi_ld_var_imm_to(b, v, bi_register(61), BI_REGISTER_FORMAT_F32,
                          BI_SAMPLE_SAMPLE, BI_UPDATE_STORE, BI_VECSIZE_V4, 1);
         bi_ld_var_imm_to(b, w, bi_register(61), BI_REGISTER_FORMAT_F32,
                          BI_SAMPLE_SAMPLE, BI_UPDATE_STORE, BI_VECSIZE_V4, 0);
      },
      {
         preload_moves(b, u, 4, 0);
         preload_moves(b, v, 4, 1);
         bi_ld_var_imm_to(b, w, bi_register(61), BI_REGISTER_FORMAT_F32,
                          BI_SAMPLE_SAMPLE, BI_UPDATE_STORE, BI_VECSIZE_V4, 0);
      });

   CASE(
      {
         bi_var_tex_f32_to(b, u, false, BI_SAMPLE_CENTER, BI_UPDATE_STORE, 0,
                           0);
         bi_var_tex_f16_to(b, v, false, BI_SAMPLE_CENTER, BI_UPDATE_STORE, 1,
                           2);
         bi_var_tex_f16_to(b, w, false, BI_SAMPLE_CENTER, BI_UPDATE_STORE, 3,
                           3);
      },
      {
         preload_moves(b, u, 4, 0);
         preload_moves(b, v, 2, 1);
         bi_var_tex_f16_to(b, w, false, BI_SAMPLE_CENTER, BI_UPDATE_STORE, 3,
                           3);
      });
}

TEST_F(MessagePreload, PreloadVartexF16)
{
   CASE(
      {
         bi_var_tex_f16_to(b, u, false, BI_SAMPLE_CENTER, BI_UPDATE_STORE, 0,
                           0);
      },
      { preload_moves(b, u, 2, 0); });
}

TEST_F(MessagePreload, PreloadVartexF32)
{
   CASE(
      {
         bi_var_tex_f32_to(b, u, false, BI_SAMPLE_CENTER, BI_UPDATE_STORE, 0,
                           0);
      },
      { preload_moves(b, u, 4, 0); });
}

TEST_F(MessagePreload, PreloadVartexF32VartexF16)
{
   CASE(
      {
         bi_var_tex_f32_to(b, u, false, BI_SAMPLE_CENTER, BI_UPDATE_STORE, 0,
                           0);
         bi_var_tex_f16_to(b, v, false, BI_SAMPLE_CENTER, BI_UPDATE_STORE, 1,
                           2);
      },
      {
         preload_moves(b, u, 4, 0);
         preload_moves(b, v, 2, 1);
      });
}

TEST_F(MessagePreload, PreloadVartexLodModes)
{
   CASE(
      {
         bi_var_tex_f32_to(b, u, true, BI_SAMPLE_CENTER, BI_UPDATE_STORE, 0, 0);
         bi_var_tex_f32_to(b, v, false, BI_SAMPLE_CENTER, BI_UPDATE_STORE, 0,
                           0);
      },
      {
         preload_moves(b, u, 4, 0);
         preload_moves(b, v, 4, 1);
      });
}
