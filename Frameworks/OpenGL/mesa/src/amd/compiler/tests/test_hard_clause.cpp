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
#include "sid.h"

using namespace aco;

static void
create_mubuf(Temp desc = Temp(0, s8))
{
   Operand desc_op(desc);
   desc_op.setFixed(PhysReg(0));
   bld.mubuf(aco_opcode::buffer_load_dword, Definition(PhysReg(256), v1), desc_op,
             Operand(PhysReg(256), v1), Operand::zero(), 0, false);
}

static void
create_mubuf_store()
{
   bld.mubuf(aco_opcode::buffer_store_dword, Operand(PhysReg(0), s4), Operand(PhysReg(256), v1),
             Operand(PhysReg(256), v1), Operand::zero(), 0, false);
}

static void
create_mtbuf(Temp desc = Temp(0, s8))
{
   Operand desc_op(desc);
   desc_op.setFixed(PhysReg(0));
   bld.mtbuf(aco_opcode::tbuffer_load_format_x, Definition(PhysReg(256), v1), desc_op,
             Operand(PhysReg(256), v1), Operand::zero(), V_008F0C_BUF_DATA_FORMAT_32,
             V_008F0C_BUF_NUM_FORMAT_FLOAT, 0, false);
}

static void
create_flat()
{
   bld.flat(aco_opcode::flat_load_dword, Definition(PhysReg(256), v1), Operand(PhysReg(256), v2),
            Operand(s2));
}

static void
create_global()
{
   bld.global(aco_opcode::global_load_dword, Definition(PhysReg(256), v1),
              Operand(PhysReg(256), v2), Operand(s2));
}

static void
create_mimg(bool nsa, Temp desc = Temp(0, s8))
{
   aco_ptr<MIMG_instruction> mimg{
      create_instruction<MIMG_instruction>(aco_opcode::image_sample, Format::MIMG, 5, 1)};
   mimg->definitions[0] = Definition(PhysReg(256), v1);
   mimg->operands[0] = Operand(desc);
   mimg->operands[0].setFixed(PhysReg(0));
   mimg->operands[1] = Operand(PhysReg(0), s4);
   mimg->operands[2] = Operand(v1);
   for (unsigned i = 0; i < 2; i++)
      mimg->operands[3 + i] = Operand(PhysReg(256 + (nsa ? i * 2 : i)), v1);
   mimg->dmask = 0x1;
   mimg->dim = ac_image_2d;

   bld.insert(std::move(mimg));
}

static void
create_smem()
{
   bld.smem(aco_opcode::s_load_dword, Definition(PhysReg(0), s1), Operand(PhysReg(0), s2),
            Operand::zero());
}

static void
create_smem_buffer(Temp desc = Temp(0, s4))
{
   Operand desc_op(desc);
   desc_op.setFixed(PhysReg(0));
   bld.smem(aco_opcode::s_buffer_load_dword, Definition(PhysReg(0), s1), desc_op, Operand::zero());
}

BEGIN_TEST(form_hard_clauses.type_restrictions)
   if (!setup_cs(NULL, GFX10))
      return;

   //>> p_unit_test 0
   //! s_clause imm:1
   //; search_re('image_sample')
   //; search_re('image_sample')
   bld.pseudo(aco_opcode::p_unit_test, Operand::zero());
   create_mimg(false);
   create_mimg(false);

   //>> p_unit_test 1
   //! s_clause imm:1
   //; search_re('buffer_load_dword')
   //; search_re('buffer_load_dword')
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(1u));
   create_mubuf();
   create_mubuf();

   //>> p_unit_test 2
   //! s_clause imm:1
   //; search_re('global_load_dword')
   //; search_re('global_load_dword')
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(2u));
   create_global();
   create_global();

   //>> p_unit_test 3
   //! s_clause imm:1
   //; search_re('flat_load_dword')
   //; search_re('flat_load_dword')
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(3u));
   create_flat();
   create_flat();

   //>> p_unit_test 4
   //! s_clause imm:1
   //; search_re('s_load_dword')
   //; search_re('s_load_dword')
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(4u));
   create_smem();
   create_smem();

   //>> p_unit_test 5
   //; search_re('buffer_load_dword')
   //; search_re('flat_load_dword')
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(5u));
   create_mubuf();
   create_flat();

   //>> p_unit_test 6
   //; search_re('buffer_load_dword')
   //; search_re('s_load_dword')
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(6u));
   create_mubuf();
   create_smem();

   //>> p_unit_test 7
   //; search_re('flat_load_dword')
   //; search_re('s_load_dword')
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(7u));
   create_flat();
   create_smem();

   finish_form_hard_clause_test();
END_TEST

BEGIN_TEST(form_hard_clauses.size)
   if (!setup_cs(NULL, GFX10))
      return;

   //>> p_unit_test 0
   //; search_re('s_load_dword')
   bld.pseudo(aco_opcode::p_unit_test, Operand::zero());
   create_smem();

   //>> p_unit_test 1
   //! s_clause imm:62
   //; for i in range(63):
   //;    search_re('s_load_dword')
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(1u));
   for (unsigned i = 0; i < 63; i++)
      create_smem();

   //>> p_unit_test 2
   //! s_clause imm:62
   //; for i in range(64):
   //;    search_re('s_load_dword')
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(2u));
   for (unsigned i = 0; i < 64; i++)
      create_smem();

   //>> p_unit_test 3
   //! s_clause imm:62
   //; for i in range(63):
   //;    search_re('s_load_dword')
   //! s_clause imm:1
   //; search_re('s_load_dword')
   //; search_re('s_load_dword')
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(3u));
   for (unsigned i = 0; i < 65; i++)
      create_smem();

   finish_form_hard_clause_test();
END_TEST

BEGIN_TEST(form_hard_clauses.nsa)
   for (unsigned i = GFX10; i <= GFX10_3; i++) {
      if (!setup_cs(NULL, (amd_gfx_level)i))
         continue;

      //>> p_unit_test 0
      //! s_clause imm:1
      //; search_re('image_sample .* %0:v\[0\], %0:v\[1\]')
      //; search_re('image_sample .* %0:v\[0\], %0:v\[1\]')
      bld.pseudo(aco_opcode::p_unit_test, Operand::zero());
      create_mimg(false);
      create_mimg(false);

      //>> p_unit_test 1
      //~gfx10_3! s_clause imm:1
      //; search_re('image_sample .* %0:v\[0\], %0:v\[1\]')
      //; search_re('image_sample .* %0:v\[0\], %0:v\[2\]')
      bld.pseudo(aco_opcode::p_unit_test, Operand::c32(1u));
      create_mimg(false);
      create_mimg(true);

      //>> p_unit_test 2
      //~gfx10_3! s_clause imm:1
      //; search_re('image_sample .* %0:v\[0\], %0:v\[2\]')
      //; search_re('image_sample .* %0:v\[0\], %0:v\[2\]')
      bld.pseudo(aco_opcode::p_unit_test, Operand::c32(2u));
      create_mimg(true);
      create_mimg(true);

      finish_form_hard_clause_test();
   }
END_TEST

BEGIN_TEST(form_hard_clauses.heuristic)
   if (!setup_cs(NULL, GFX10))
      return;

   Temp img_desc0 = bld.tmp(s8);
   Temp img_desc1 = bld.tmp(s8);
   Temp buf_desc0 = bld.tmp(s4);
   Temp buf_desc1 = bld.tmp(s4);

   /* Don't form clause with different descriptors */
   //>> p_unit_test 0
   //! s_clause imm:1
   //; search_re('image_sample')
   //; search_re('image_sample')
   bld.pseudo(aco_opcode::p_unit_test, Operand::zero());
   create_mimg(false, img_desc0);
   create_mimg(false, img_desc0);

   //>> p_unit_test 1
   //; search_re('image_sample')
   //; search_re('image_sample')
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(1u));
   create_mimg(false, img_desc0);
   create_mimg(false, img_desc1);

   //>> p_unit_test 2
   //! s_clause imm:1
   //; search_re('buffer_load_dword')
   //; search_re('buffer_load_dword')
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(2u));
   create_mubuf(buf_desc0);
   create_mubuf(buf_desc0);

   //>> p_unit_test 3
   //; search_re('buffer_load_dword')
   //; search_re('buffer_load_dword')
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(3u));
   create_mubuf(buf_desc0);
   create_mubuf(buf_desc1);

   //>> p_unit_test 4
   //! s_clause imm:1
   //; search_re('s_buffer_load_dword')
   //; search_re('s_buffer_load_dword')
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(4u));
   create_smem_buffer(buf_desc0);
   create_smem_buffer(buf_desc0);

   //>> p_unit_test 5
   //; search_re('s_buffer_load_dword')
   //; search_re('s_buffer_load_dword')
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(5u));
   create_smem_buffer(buf_desc0);
   create_smem_buffer(buf_desc1);

   //>> p_unit_test 6
   //; search_re('s_buffer_load_dword')
   //; search_re('s_load_dword')
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(6u));
   create_smem_buffer(buf_desc0);
   create_smem();

   /* Only form clause between MUBUF and MTBUF if they load from the same binding. Ignore descriptor
    * if they're te same binding.
    */
   //>> p_unit_test 7
   //; search_re('buffer_load_dword')
   //; search_re('tbuffer_load_format_x')
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(7u));
   create_mubuf(buf_desc0);
   create_mtbuf(buf_desc0);

   finish_form_hard_clause_test();
END_TEST

BEGIN_TEST(form_hard_clauses.stores)
   for (amd_gfx_level gfx : {GFX10, GFX11}) {
      if (!setup_cs(NULL, gfx))
         continue;

      //>> p_unit_test 0
      //~gfx11! s_clause imm:1
      //; search_re('buffer_store_dword')
      //; search_re('buffer_store_dword')
      bld.pseudo(aco_opcode::p_unit_test, Operand::zero());
      create_mubuf_store();
      create_mubuf_store();

      //>> p_unit_test 1
      //! s_clause imm:1
      //; search_re('buffer_load_dword')
      //; search_re('buffer_load_dword')
      //; search_re('buffer_store_dword')
      bld.pseudo(aco_opcode::p_unit_test, Operand::c32(1u));
      create_mubuf();
      create_mubuf();
      create_mubuf_store();

      //>> p_unit_test 2
      //; search_re('buffer_store_dword')
      //! s_clause imm:1
      //; search_re('buffer_load_dword')
      //; search_re('buffer_load_dword')
      bld.pseudo(aco_opcode::p_unit_test, Operand::c32(2u));
      create_mubuf_store();
      create_mubuf();
      create_mubuf();

      /* Unclear whether this is the best behaviour */
      //>> p_unit_test 3
      //; search_re('buffer_load_dword')
      //; search_re('buffer_store_dword')
      //; search_re('buffer_load_dword')
      bld.pseudo(aco_opcode::p_unit_test, Operand::c32(3u));
      create_mubuf();
      create_mubuf_store();
      create_mubuf();

      finish_form_hard_clause_test();
   }
END_TEST
