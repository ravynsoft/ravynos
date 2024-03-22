/*
 * Copyright © 2015 Intel Corporation
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

class nir_cf_test : public nir_test {
protected:
   nir_cf_test()
      : nir_test::nir_test("nir_cf_test")
   {
   }
};

TEST_F(nir_cf_test, delete_break_in_loop)
{
   /* Create IR:
    *
    * while (...) { break; }
    */
   nir_loop *loop = nir_loop_create(b->shader);
   nir_cf_node_insert(nir_after_cf_list(&b->impl->body), &loop->cf_node);

   b->cursor = nir_after_cf_list(&loop->body);

   nir_jump_instr *jump = nir_jump_instr_create(b->shader, nir_jump_break);
   nir_builder_instr_insert(b, &jump->instr);

   /* At this point, we should have:
    *
    * impl main {
    *         block block_0:
    *         // preds:
    *         // succs: block_1
    *         loop {
    *                 block block_1:
    *                 // preds: block_0
    *                 break
    *                 // succs: block_2
    *         }
    *         block block_2:
    *         // preds: block_1
    *         // succs: block_3
    *         block block_3:
    * }
    */
   nir_block *block_0 = nir_start_block(b->impl);
   nir_block *block_1 = nir_loop_first_block(loop);
   nir_block *block_2 = nir_cf_node_as_block(nir_cf_node_next(&loop->cf_node));
   nir_block *block_3 = b->impl->end_block;
   ASSERT_EQ(nir_cf_node_block, block_0->cf_node.type);
   ASSERT_EQ(nir_cf_node_block, block_1->cf_node.type);
   ASSERT_EQ(nir_cf_node_block, block_2->cf_node.type);
   ASSERT_EQ(nir_cf_node_block, block_3->cf_node.type);

   /* Verify the successors and predecessors. */
   EXPECT_EQ(block_1, block_0->successors[0]);
   EXPECT_EQ(NULL,    block_0->successors[1]);
   EXPECT_EQ(block_2, block_1->successors[0]);
   EXPECT_EQ(NULL,    block_1->successors[1]);
   EXPECT_EQ(block_3, block_2->successors[0]);
   EXPECT_EQ(NULL,    block_2->successors[1]);
   EXPECT_EQ(NULL,    block_3->successors[0]);
   EXPECT_EQ(NULL,    block_3->successors[1]);
   EXPECT_EQ(0,       block_0->predecessors->entries);
   EXPECT_EQ(1,       block_1->predecessors->entries);
   EXPECT_EQ(1,       block_2->predecessors->entries);
   EXPECT_EQ(1,       block_3->predecessors->entries);
   EXPECT_TRUE(_mesa_set_search(block_1->predecessors, block_0));
   EXPECT_TRUE(_mesa_set_search(block_2->predecessors, block_1));
   EXPECT_TRUE(_mesa_set_search(block_3->predecessors, block_2));

   /* Now remove the break. */
   nir_instr_remove(&jump->instr);

   /* At this point, we should have:
    *
    * impl main {
    *         block block_0:
    *         // preds:
    *         // succs: block_1
    *         loop {
    *                 block block_1:
    *                 // preds: block_0 block_1
    *                 // succs: block_1
    *         }
    *         block block_2:
    *         // preds:
    *         // succs: block_3
    *         block block_3:
    * }
    *
    * Re-verify the predecessors and successors.
    */
   EXPECT_EQ(block_1, block_0->successors[0]);
   EXPECT_EQ(NULL,    block_0->successors[1]);
   EXPECT_EQ(block_1, block_1->successors[0]); /* back to itself */
   EXPECT_EQ(NULL,    block_1->successors[1]);
   EXPECT_EQ(block_3, block_2->successors[0]);
   EXPECT_EQ(NULL,    block_2->successors[1]);
   EXPECT_EQ(NULL,    block_3->successors[0]);
   EXPECT_EQ(NULL,    block_3->successors[1]);
   EXPECT_EQ(0,       block_0->predecessors->entries);
   EXPECT_EQ(2,       block_1->predecessors->entries);
   EXPECT_EQ(0,       block_2->predecessors->entries);
   EXPECT_EQ(1,       block_3->predecessors->entries);
   EXPECT_TRUE(_mesa_set_search(block_1->predecessors, block_0));
   EXPECT_TRUE(_mesa_set_search(block_1->predecessors, block_1));
   EXPECT_FALSE(_mesa_set_search(block_2->predecessors, block_1));
   EXPECT_TRUE(_mesa_set_search(block_3->predecessors, block_2));

   nir_metadata_require(b->impl, nir_metadata_dominance);
}
