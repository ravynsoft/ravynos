/*
 * Copyright © 2022 Valve Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 */
#include "helpers.h"

using namespace aco;

BEGIN_TEST(setup_reduce_temp.divergent_if_phi)
   /*
    * This must have an end_linear_vgpr after the phi (to ensure it's live during the phi copies):
    * v0 = start_linear_vgpr
    * divergent_if (...) {
    *
    * } else {
    *    use_linear_vgpr(v0)
    * }
    * ... = phi ...
    */
   // TODO: fix the RA validator to spot this
   //>> s2: %_, v1: %a = p_startpgm
   if (!setup_cs("s2 v1", GFX9))
      return;

   //>> lv1: %lv = p_start_linear_vgpr
   emit_divergent_if_else(
      program.get(), bld, Operand(inputs[0]),
      [&]() -> void
      {
         //>> s1: %_, s2: %_, s1: %_:scc = p_reduce %a, %lv, lv1: undef op:umin32 cluster_size:64
         Instruction* reduce =
            bld.reduction(aco_opcode::p_reduce, bld.def(s1), bld.def(bld.lm), bld.def(s1, scc),
                          inputs[1], Operand(v1.as_linear()), Operand(v1.as_linear()), umin32);
         reduce->reduction().cluster_size = bld.lm.bytes() * 8;
      },
      [&]() -> void
      {
         /* nothing */
      });
   bld.pseudo(aco_opcode::p_phi, bld.def(v1), Operand::c32(1), Operand::zero());
   //>> /* logical preds: BB1, BB4, / linear preds: BB4, BB5, / kind: uniform, top-level, merge, */
   //! p_end_linear_vgpr %lv

   finish_setup_reduce_temp_test();
END_TEST
