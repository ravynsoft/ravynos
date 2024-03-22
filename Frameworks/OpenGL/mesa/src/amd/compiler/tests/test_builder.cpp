/*
 * Copyright © 2020 Valve Corporation
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

BEGIN_TEST(builder.v_mul_imm)
   for (unsigned i = GFX8; i <= GFX10; i++) {
      //>> v1: %a, v1: %b, s1: %c, s1: %d = p_startpgm
      if (!setup_cs("v1 v1 s1 s1", (amd_gfx_level)i))
         continue;

      /* simple optimizations */

      //! p_unit_test 0, 0
      writeout(0, bld.v_mul_imm(bld.def(v1), inputs[0], 0));

      //! p_unit_test 1, %a
      writeout(1, bld.v_mul_imm(bld.def(v1), inputs[0], 1));

      //! v1: %res2 = v_lshlrev_b32 2, %a
      //! p_unit_test 2, %res2
      writeout(2, bld.v_mul_imm(bld.def(v1), inputs[0], 4));

      //! v1: %res3 = v_lshlrev_b32 31, %a
      //! p_unit_test 3, %res3
      writeout(3, bld.v_mul_imm(bld.def(v1), inputs[0], 2147483648u));

      /* single lshl+add/sub */

      //~gfx8! v1: %res4_tmp = v_lshlrev_b32 3, %a
      //~gfx8! v1: %res4,  s2: %_ = v_add_co_u32 %res4_tmp, %a
      //~gfx(9|10)! v1: %res4 = v_lshl_add_u32 %a, 3, %a
      //! p_unit_test 4, %res4
      writeout(4, bld.v_mul_imm(bld.def(v1), inputs[0], 9));

      //~gfx[89]! v1: %res5_tmp = v_lshlrev_b32 3, %a
      //~gfx8! v1: %res5,  s2: %_ = v_sub_co_u32 %res5_tmp, %a
      //~gfx9! v1: %res5 = v_sub_u32 %res5_tmp, %a
      //~gfx10! v1: %res5 = v_mul_lo_u32 7, %a
      //! p_unit_test 5, %res5
      writeout(5, bld.v_mul_imm(bld.def(v1), inputs[0], 7));

      /* lshl+add optimization with literal */

      //~gfx8! v1: %res6_tmp0 = v_lshlrev_b32 2, %a
      //~gfx8! v1: %res6_tmp1 = v_lshlrev_b32 6, %a
      //~gfx8! v1: %res6,  s2: %_ = v_add_co_u32 %res6_tmp1, %res6_tmp0
      //~gfx9! v1: %res6_tmp = v_lshlrev_b32 2, %a
      //~gfx9! v1: %res6 = v_lshl_add_u32 %a, 6, %res6_tmp
      //~gfx10! v1: %res6 = v_mul_lo_u32 0x44, %a
      //! p_unit_test 6, %res6
      writeout(6, bld.v_mul_imm(bld.def(v1), inputs[0], 4 | 64));

      //~gfx8! s1: %res7_tmp = p_parallelcopy 0x144
      //~gfx8! v1: %res7 = v_mul_lo_u32 %res7_tmp, %a
      //~gfx9! v1: %res7_tmp0 = v_lshlrev_b32 2, %a
      //~gfx9! v1: %res7_tmp1 = v_lshl_add_u32 %a, 6, %res7_tmp0
      //~gfx9! v1: %res7 = v_lshl_add_u32 %a, 8, %res7_tmp1
      //~gfx10! v1: %res7 = v_mul_lo_u32 0x144, %a
      //! p_unit_test 7, %res7
      writeout(7, bld.v_mul_imm(bld.def(v1), inputs[0], 4 | 64 | 256));

      //~gfx8! s1: %res8_tmp = p_parallelcopy 0x944
      //~gfx8! v1: %res8 = v_mul_lo_u32 %res8_tmp, %a
      //~gfx9! v1: %res8_tmp0 = v_lshlrev_b32 2, %a
      //~gfx9! v1: %res8_tmp1 = v_lshl_add_u32 %a, 6, %res8_tmp0
      //~gfx9! v1: %res8_tmp2 = v_lshl_add_u32 %a, 8, %res8_tmp1
      //~gfx9! v1: %res8 = v_lshl_add_u32 %a, 11, %res8_tmp2
      //~gfx10! v1: %res8 = v_mul_lo_u32 0x944, %a
      //! p_unit_test 8, %res8
      writeout(8, bld.v_mul_imm(bld.def(v1), inputs[0], 4 | 64 | 256 | 2048));

      /* lshl+add optimization with inline constant */

      //~gfx8! v1: %res9_tmp0 = v_lshlrev_b32 1, %a
      //~gfx8! v1: %res9_tmp1 = v_lshlrev_b32 2, %a
      //~gfx8! v1: %res9,  s2: %_ = v_add_co_u32 %res9_tmp1, %res9_tmp0
      //~gfx9! v1: %res9_tmp0 = v_lshlrev_b32 1, %a
      //~gfx9! v1: %res9 = v_lshl_add_u32 %a, 2, %res9_tmp0
      //~gfx10! v1: %res9 = v_mul_lo_u32 6, %a
      //! p_unit_test 9, %res9
      writeout(9, bld.v_mul_imm(bld.def(v1), inputs[0], 2 | 4));

      //~gfx(8|10)! v1: %res10 = v_mul_lo_u32 14, %a
      //~gfx9! v1: %res10_tmp0 = v_lshlrev_b32 1, %a
      //~gfx9! v1: %res10_tmp1 = v_lshl_add_u32 %a, 2, %res10_tmp0
      //~gfx9! v1: %res10 = v_lshl_add_u32 %a, 3, %res10_tmp1
      //! p_unit_test 10, %res10
      writeout(10, bld.v_mul_imm(bld.def(v1), inputs[0], 2 | 4 | 8));

      //! v1: %res11 = v_mul_lo_u32 30, %a
      //! p_unit_test 11, %res11
      writeout(11, bld.v_mul_imm(bld.def(v1), inputs[0], 2 | 4 | 8 | 16));

      finish_opt_test();
   }
END_TEST
