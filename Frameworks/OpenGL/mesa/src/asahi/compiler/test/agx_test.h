/*
 * Copyright 2020-2021 Collabora Ltd.
 * SPDX-License-Identifier: MIT
 */

#ifndef __AGX_TEST_H
#define __AGX_TEST_H

#include <inttypes.h>
#include <stdio.h>
#include "agx_builder.h"
#include "agx_compiler.h"

/* Helper to generate a agx_builder suitable for creating test instructions */
static inline agx_builder *
agx_test_builder(void *memctx)
{
   agx_context *ctx = rzalloc(memctx, agx_context);
   list_inithead(&ctx->blocks);

   agx_block *blk = rzalloc(ctx, agx_block);
   util_dynarray_init(&blk->predecessors, NULL);

   list_addtail(&blk->link, &ctx->blocks);
   list_inithead(&blk->instructions);

   agx_builder *b = rzalloc(memctx, agx_builder);
   b->shader = ctx;
   b->cursor = agx_after_block(blk);

   return b;
}

/* Helper to compare for logical equality of instructions. Need to compare the
 * pointers, then compare raw data.
 */
static inline bool
agx_instr_equal(agx_instr *A, agx_instr *B)
{
   unsigned pointers = sizeof(struct list_head) + sizeof(agx_index *) * 2;

   if (A->nr_srcs != B->nr_srcs)
      return false;

   if (memcmp(A->src, B->src, A->nr_srcs * sizeof(agx_index)))
      return false;

   if (A->nr_dests != B->nr_dests)
      return false;

   if (memcmp(A->dest, B->dest, A->nr_dests * sizeof(agx_index)))
      return false;

   return memcmp((uint8_t *)A + pointers, (uint8_t *)B + pointers,
                 sizeof(agx_instr) - pointers) == 0;
}

static inline bool
agx_block_equal(agx_block *A, agx_block *B)
{
   if (list_length(&A->instructions) != list_length(&B->instructions))
      return false;

   list_pair_for_each_entry(agx_instr, insA, insB, &A->instructions,
                            &B->instructions, link) {
      if (!agx_instr_equal(insA, insB))
         return false;
   }

   return true;
}

static inline bool
agx_shader_equal(agx_context *A, agx_context *B)
{
   if (list_length(&A->blocks) != list_length(&B->blocks))
      return false;

   list_pair_for_each_entry(agx_block, blockA, blockB, &A->blocks, &B->blocks,
                            link) {
      if (!agx_block_equal(blockA, blockB))
         return false;
   }

   return true;
}

#define ASSERT_SHADER_EQUAL(A, B)                                              \
   if (!agx_shader_equal(A, B)) {                                              \
      ADD_FAILURE();                                                           \
      fprintf(stderr, "Pass produced unexpected results");                     \
      fprintf(stderr, "  Actual:\n");                                          \
      agx_print_shader(A, stderr);                                             \
      fprintf(stderr, " Expected:\n");                                         \
      agx_print_shader(B, stderr);                                             \
      fprintf(stderr, "\n");                                                   \
   }

#define INSTRUCTION_CASE(instr, expected, pass)                                \
   do {                                                                        \
      agx_builder *A = agx_test_builder(mem_ctx);                              \
      agx_builder *B = agx_test_builder(mem_ctx);                              \
      {                                                                        \
         agx_builder *b = A;                                                   \
         instr;                                                                \
      }                                                                        \
      {                                                                        \
         agx_builder *b = B;                                                   \
         expected;                                                             \
      }                                                                        \
      pass(A->shader);                                                         \
      ASSERT_SHADER_EQUAL(A->shader, B->shader);                               \
   } while (0)

#endif
