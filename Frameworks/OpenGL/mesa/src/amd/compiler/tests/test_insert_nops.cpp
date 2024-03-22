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

void
create_mubuf(unsigned offset, PhysReg dst = PhysReg(256), PhysReg vaddr = PhysReg(256))
{
   bld.mubuf(aco_opcode::buffer_load_dword, Definition(dst, v1), Operand(PhysReg(0), s4),
             Operand(vaddr, v1), Operand::zero(), offset, true);
}

void
create_mubuf_store(PhysReg src = PhysReg(256))
{
   bld.mubuf(aco_opcode::buffer_store_dword, Operand(PhysReg(0), s4), Operand(src, v1),
             Operand::zero(), Operand(src, v1), 0, true);
}

void
create_mimg(bool nsa, unsigned addrs, unsigned instr_dwords)
{
   aco_ptr<MIMG_instruction> mimg{
      create_instruction<MIMG_instruction>(aco_opcode::image_sample, Format::MIMG, 3 + addrs, 1)};
   mimg->definitions[0] = Definition(PhysReg(256), v1);
   mimg->operands[0] = Operand(PhysReg(0), s8);
   mimg->operands[1] = Operand(PhysReg(0), s4);
   mimg->operands[2] = Operand(v1);
   for (unsigned i = 0; i < addrs; i++)
      mimg->operands[3 + i] = Operand(PhysReg(256 + (nsa ? i * 2 : i)), v1);
   mimg->dmask = 0x1;
   mimg->dim = ac_image_2d;

   assert(get_mimg_nsa_dwords(mimg.get()) + 2 == instr_dwords);

   bld.insert(std::move(mimg));
}

BEGIN_TEST(insert_nops.nsa_to_vmem_bug)
   if (!setup_cs(NULL, GFX10))
      return;

   /* no nop needed because offset&6==0 */
   //>> p_unit_test 0
   //! v1: %0:v[0] = image_sample %0:s[0-7], %0:s[0-3],  v1: undef, %0:v[0], %0:v[2], %0:v[4], %0:v[6], %0:v[8], %0:v[10] 2d
   //! v1: %0:v[0] = buffer_load_dword %0:s[0-3], %0:v[0], 0 offset:8 offen
   bld.pseudo(aco_opcode::p_unit_test, Operand::zero());
   create_mimg(true, 6, 4);
   create_mubuf(8);

   /* nop needed */
   //! p_unit_test 1
   //! v1: %0:v[0] = image_sample %0:s[0-7], %0:s[0-3],  v1: undef, %0:v[0], %0:v[2], %0:v[4], %0:v[6], %0:v[8], %0:v[10] 2d
   //! s_nop
   //! v1: %0:v[0] = buffer_load_dword %0:s[0-3], %0:v[0], 0 offset:4 offen
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(1u));
   create_mimg(true, 6, 4);
   create_mubuf(4);

   /* no nop needed because the MIMG is not NSA */
   //! p_unit_test 2
   //! v1: %0:v[0] = image_sample %0:s[0-7], %0:s[0-3],  v1: undef, %0:v[0], %0:v[1], %0:v[2], %0:v[3], %0:v[4], %0:v[5] 2d
   //! v1: %0:v[0] = buffer_load_dword %0:s[0-3], %0:v[0], 0 offset:4 offen
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(2u));
   create_mimg(false, 6, 2);
   create_mubuf(4);

   /* no nop needed because there's already an instruction in-between */
   //! p_unit_test 3
   //! v1: %0:v[0] = image_sample %0:s[0-7], %0:s[0-3],  v1: undef, %0:v[0], %0:v[2], %0:v[4], %0:v[6], %0:v[8], %0:v[10] 2d
   //! v_nop
   //! v1: %0:v[0] = buffer_load_dword %0:s[0-3], %0:v[0], 0 offset:4 offen
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(3u));
   create_mimg(true, 6, 4);
   bld.vop1(aco_opcode::v_nop);
   create_mubuf(4);

   /* no nop needed because the NSA instruction is under 4 dwords */
   //! p_unit_test 4
   //! v1: %0:v[0] = image_sample %0:s[0-7], %0:s[0-3],  v1: undef, %0:v[0], %0:v[2] 2d
   //! v1: %0:v[0] = buffer_load_dword %0:s[0-3], %0:v[0], 0 offset:4 offen
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(4u));
   create_mimg(true, 2, 3);
   create_mubuf(4);

   /* NSA instruction and MUBUF/MTBUF in a different block */
   //! p_unit_test 5
   //! v1: %0:v[0] = image_sample %0:s[0-7], %0:s[0-3],  v1: undef, %0:v[0], %0:v[2], %0:v[4], %0:v[6], %0:v[8], %0:v[10] 2d
   //! BB1
   //! /* logical preds: / linear preds: BB0, / kind: uniform, */
   //! s_nop
   //! v1: %0:v[0] = buffer_load_dword %0:s[0-3], %0:v[0], 0 offset:4 offen
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(5u));
   create_mimg(true, 6, 4);
   bld.reset(program->create_and_insert_block());
   create_mubuf(4);
   program->blocks[0].linear_succs.push_back(1);
   program->blocks[1].linear_preds.push_back(0);

   finish_insert_nops_test();
END_TEST

BEGIN_TEST(insert_nops.writelane_to_nsa_bug)
   if (!setup_cs(NULL, GFX10))
      return;

   /* nop needed */
   //>> p_unit_test 0
   //! v1: %0:v[255] = v_writelane_b32_e64 0, 0, %0:v[255]
   //! s_nop
   //! v1: %0:v[0] = image_sample %0:s[0-7], %0:s[0-3],  v1: undef, %0:v[0], %0:v[2] 2d
   bld.pseudo(aco_opcode::p_unit_test, Operand::zero());
   bld.writelane(Definition(PhysReg(511), v1), Operand::zero(), Operand::zero(),
                 Operand(PhysReg(511), v1));
   create_mimg(true, 2, 3);

   /* no nop needed because the MIMG is not NSA */
   //! p_unit_test 1
   //! v1: %0:v[255] = v_writelane_b32_e64 0, 0, %0:v[255]
   //! v1: %0:v[0] = image_sample %0:s[0-7], %0:s[0-3],  v1: undef, %0:v[0], %0:v[1] 2d
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(1u));
   bld.writelane(Definition(PhysReg(511), v1), Operand::zero(), Operand::zero(),
                 Operand(PhysReg(511), v1));
   create_mimg(false, 2, 2);

   /* no nop needed because there's already an instruction in-between */
   //! p_unit_test 2
   //! v1: %0:v[255] = v_writelane_b32_e64 0, 0, %0:v[255]
   //! v_nop
   //! v1: %0:v[0] = image_sample %0:s[0-7], %0:s[0-3],  v1: undef, %0:v[0], %0:v[2] 2d
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(2u));
   bld.writelane(Definition(PhysReg(511), v1), Operand::zero(), Operand::zero(),
                 Operand(PhysReg(511), v1));
   bld.vop1(aco_opcode::v_nop);
   create_mimg(true, 2, 3);

   /* writelane and NSA instruction in different blocks */
   //! p_unit_test 3
   //! v1: %0:v[255] = v_writelane_b32_e64 0, 0, %0:v[255]
   //! BB1
   //! /* logical preds: / linear preds: BB0, / kind: uniform, */
   //! s_nop
   //! v1: %0:v[0] = image_sample %0:s[0-7], %0:s[0-3],  v1: undef, %0:v[0], %0:v[2] 2d
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(3u));
   bld.writelane(Definition(PhysReg(511), v1), Operand::zero(), Operand::zero(),
                 Operand(PhysReg(511), v1));
   bld.reset(program->create_and_insert_block());
   create_mimg(true, 2, 3);
   program->blocks[0].linear_succs.push_back(1);
   program->blocks[1].linear_preds.push_back(0);

   finish_insert_nops_test();
END_TEST

BEGIN_TEST(insert_nops.vmem_to_scalar_write)
   if (!setup_cs(NULL, GFX10))
      return;

   /* WaR: VMEM load */
   //>> p_unit_test 0
   //! v1: %0:v[0] = buffer_load_dword %0:s[0-3], %0:v[0], 0 offen
   //! s_waitcnt_depctr vm_vsrc(0)
   //! s1: %0:s[0] = s_mov_b32 0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(0));
   create_mubuf(0);
   bld.sop1(aco_opcode::s_mov_b32, Definition(PhysReg(0), s1), Operand::zero());

   //! p_unit_test 1
   //! v1: %0:v[0] = buffer_load_dword %0:s[0-3], %0:v[0], 0 offen
   //! s_waitcnt_depctr vm_vsrc(0)
   //! s2: %0:exec = s_mov_b64 -1
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(1));
   create_mubuf(0);
   bld.sop1(aco_opcode::s_mov_b64, Definition(exec, s2), Operand::c64(-1));

   /* no hazard: VMEM load */
   //! p_unit_test 2
   //! v1: %0:v[0] = buffer_load_dword %0:s[0-3], %0:v[0], 0 offen
   //! s1: %0:s[4] = s_mov_b32 0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(2));
   create_mubuf(0);
   bld.sop1(aco_opcode::s_mov_b32, Definition(PhysReg(4), s1), Operand::zero());

   /* no hazard: VMEM load with VALU in-between */
   //! p_unit_test 3
   //! v1: %0:v[0] = buffer_load_dword %0:s[0-3], %0:v[0], 0 offen
   //! v_nop
   //! s1: %0:s[0] = s_mov_b32 0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(3));
   create_mubuf(0);
   bld.vop1(aco_opcode::v_nop);
   bld.sop1(aco_opcode::s_mov_b32, Definition(PhysReg(0), s1), Operand::zero());

   /* WaR: LDS */
   //! p_unit_test 4
   //! v1: %0:v[0] = ds_read_b32 %0:v[0], %0:m0
   //! s_waitcnt_depctr vm_vsrc(0)
   //! s1: %0:m0 = s_mov_b32 0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(4));
   bld.ds(aco_opcode::ds_read_b32, Definition(PhysReg(256), v1), Operand(PhysReg(256), v1),
          Operand(m0, s1));
   bld.sop1(aco_opcode::s_mov_b32, Definition(m0, s1), Operand::zero());

   //! p_unit_test 5
   //! v1: %0:v[0] = ds_read_b32 %0:v[0], %0:m0
   //! s_waitcnt_depctr vm_vsrc(0)
   //! s2: %0:exec = s_mov_b64 -1
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(5));
   bld.ds(aco_opcode::ds_read_b32, Definition(PhysReg(256), v1), Operand(PhysReg(256), v1),
          Operand(m0, s1));
   bld.sop1(aco_opcode::s_mov_b64, Definition(exec, s2), Operand::c64(-1));

   /* no hazard: LDS */
   //! p_unit_test 6
   //! v1: %0:v[0] = ds_read_b32 %0:v[0], %0:m0
   //! s1: %0:s[0] = s_mov_b32 0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(6));
   bld.ds(aco_opcode::ds_read_b32, Definition(PhysReg(256), v1), Operand(PhysReg(256), v1),
          Operand(m0, s1));
   bld.sop1(aco_opcode::s_mov_b32, Definition(PhysReg(0), s1), Operand::zero());

   /* no hazard: LDS with VALU in-between */
   //! p_unit_test 7
   //! v1: %0:v[0] = ds_read_b32 %0:v[0], %0:m0
   //! v_nop
   //! s1: %0:m0 = s_mov_b32 0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(7));
   bld.ds(aco_opcode::ds_read_b32, Definition(PhysReg(256), v1), Operand(PhysReg(256), v1),
          Operand(m0, s1));
   bld.vop1(aco_opcode::v_nop);
   bld.sop1(aco_opcode::s_mov_b32, Definition(m0, s1), Operand::zero());

   /* no hazard: VMEM/LDS with the correct waitcnt in-between */
   //! p_unit_test 8
   //! v1: %0:v[0] = buffer_load_dword %0:s[0-3], %0:v[0], 0 offen
   //! s_waitcnt vmcnt(0)
   //! s1: %0:s[0] = s_mov_b32 0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(8));
   create_mubuf(0);
   bld.sopp(aco_opcode::s_waitcnt, -1, 0x3f70);
   bld.sop1(aco_opcode::s_mov_b32, Definition(PhysReg(0), s1), Operand::zero());

   //! p_unit_test 9
   //! buffer_store_dword %0:s[0-3], %0:v[0], 0, %0:v[0] offen
   //! s_waitcnt_vscnt %0:null imm:0
   //! s1: %0:s[0] = s_mov_b32 0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(9));
   create_mubuf_store();
   bld.sopk(aco_opcode::s_waitcnt_vscnt, Operand(sgpr_null, s1), 0);
   bld.sop1(aco_opcode::s_mov_b32, Definition(PhysReg(0), s1), Operand::zero());

   //! p_unit_test 10
   //! v1: %0:v[0] = ds_read_b32 %0:v[0], %0:m0
   //! s_waitcnt lgkmcnt(0)
   //! s1: %0:m0 = s_mov_b32 0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(10));
   bld.ds(aco_opcode::ds_read_b32, Definition(PhysReg(256), v1), Operand(PhysReg(256), v1),
          Operand(m0, s1));
   bld.sopp(aco_opcode::s_waitcnt, -1, 0xc07f);
   bld.sop1(aco_opcode::s_mov_b32, Definition(m0, s1), Operand::zero());

   /* VMEM/LDS with the wrong waitcnt in-between */
   //! p_unit_test 11
   //! v1: %0:v[0] = buffer_load_dword %0:s[0-3], %0:v[0], 0 offen
   //! s_waitcnt_vscnt %0:null imm:0
   //! s_waitcnt_depctr vm_vsrc(0)
   //! s1: %0:s[0] = s_mov_b32 0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(11));
   create_mubuf(0);
   bld.sopk(aco_opcode::s_waitcnt_vscnt, Operand(sgpr_null, s1), 0);
   bld.sop1(aco_opcode::s_mov_b32, Definition(PhysReg(0), s1), Operand::zero());

   //! p_unit_test 12
   //! buffer_store_dword %0:s[0-3], %0:v[0], 0, %0:v[0] offen
   //! s_waitcnt lgkmcnt(0)
   //! s_waitcnt_depctr vm_vsrc(0)
   //! s1: %0:s[0] = s_mov_b32 0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(12));
   create_mubuf_store();
   bld.sopp(aco_opcode::s_waitcnt, -1, 0xc07f);
   bld.sop1(aco_opcode::s_mov_b32, Definition(PhysReg(0), s1), Operand::zero());

   //! p_unit_test 13
   //! v1: %0:v[0] = ds_read_b32 %0:v[0], %0:m0
   //! s_waitcnt vmcnt(0)
   //! s_waitcnt_depctr vm_vsrc(0)
   //! s1: %0:m0 = s_mov_b32 0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(13));
   bld.ds(aco_opcode::ds_read_b32, Definition(PhysReg(256), v1), Operand(PhysReg(256), v1),
          Operand(m0, s1));
   bld.sopp(aco_opcode::s_waitcnt, -1, 0x3f70);
   bld.sop1(aco_opcode::s_mov_b32, Definition(m0, s1), Operand::zero());

   finish_insert_nops_test();
END_TEST

BEGIN_TEST(insert_nops.lds_direct_valu)
   if (!setup_cs(NULL, GFX11))
      return;

   /* WaW */
   //>> p_unit_test 0
   //! v1: %0:v[0] = v_mov_b32 0
   //! v1: %0:v[0] = lds_direct_load %0:m0 wait_vdst:0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(0));
   bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg(256), v1), Operand::zero());
   bld.ldsdir(aco_opcode::lds_direct_load, Definition(PhysReg(256), v1), Operand(m0, s1));

   /* WaR */
   //! p_unit_test 1
   //! v1: %0:v[1] = v_mov_b32 %0:v[0]
   //! v1: %0:v[0] = lds_direct_load %0:m0 wait_vdst:0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(1));
   bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg(257), v1), Operand(PhysReg(256), v1));
   bld.ldsdir(aco_opcode::lds_direct_load, Definition(PhysReg(256), v1), Operand(m0, s1));

   /* No hazard. */
   //! p_unit_test 2
   //! v1: %0:v[1] = v_mov_b32 0
   //! v1: %0:v[0] = lds_direct_load %0:m0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(2));
   bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg(257), v1), Operand::zero());
   bld.ldsdir(aco_opcode::lds_direct_load, Definition(PhysReg(256), v1), Operand(m0, s1));

   /* multiples hazards, nearest should be considered */
   //! p_unit_test 3
   //! v1: %0:v[1] = v_mov_b32 %0:v[0]
   //! v1: %0:v[0] = v_mov_b32 0
   //! v1: %0:v[0] = lds_direct_load %0:m0 wait_vdst:0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(3));
   bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg(257), v1), Operand(PhysReg(256), v1));
   bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg(256), v1), Operand::zero());
   bld.ldsdir(aco_opcode::lds_direct_load, Definition(PhysReg(256), v1), Operand(m0, s1));

   /* independent VALU increase wait_vdst */
   //! p_unit_test 4
   //! v1: %0:v[0] = v_mov_b32 0
   //! v_nop
   //! v1: %0:v[0] = lds_direct_load %0:m0 wait_vdst:1
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(4));
   bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg(256), v1), Operand::zero());
   bld.vop1(aco_opcode::v_nop);
   bld.ldsdir(aco_opcode::lds_direct_load, Definition(PhysReg(256), v1), Operand(m0, s1));

   //! p_unit_test 5
   //! v1: %0:v[0] = v_mov_b32 0
   //; for i in range(10): insert_pattern('v_nop')
   //! v1: %0:v[0] = lds_direct_load %0:m0 wait_vdst:10
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(5));
   bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg(256), v1), Operand::zero());
   for (unsigned i = 0; i < 10; i++)
      bld.vop1(aco_opcode::v_nop);
   bld.ldsdir(aco_opcode::lds_direct_load, Definition(PhysReg(256), v1), Operand(m0, s1));

   //! p_unit_test 6
   //! v1: %0:v[0] = v_mov_b32 0
   //; for i in range(20): insert_pattern('v_nop')
   //! v1: %0:v[0] = lds_direct_load %0:m0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(6));
   bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg(256), v1), Operand::zero());
   for (unsigned i = 0; i < 20; i++)
      bld.vop1(aco_opcode::v_nop);
   bld.ldsdir(aco_opcode::lds_direct_load, Definition(PhysReg(256), v1), Operand(m0, s1));

   /* transcendental requires wait_vdst=0 */
   //! p_unit_test 7
   //! v1: %0:v[0] = v_mov_b32 0
   //! v_nop
   //! v1: %0:v[1] = v_sqrt_f32 %0:v[1]
   //! v1: %0:v[0] = lds_direct_load %0:m0 wait_vdst:0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(7));
   bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg(256), v1), Operand::zero());
   bld.vop1(aco_opcode::v_nop);
   bld.vop1(aco_opcode::v_sqrt_f32, Definition(PhysReg(257), v1), Operand(PhysReg(257), v1));
   bld.ldsdir(aco_opcode::lds_direct_load, Definition(PhysReg(256), v1), Operand(m0, s1));

   //! p_unit_test 8
   //! v1: %0:v[0] = v_sqrt_f32 %0:v[0]
   //! v_nop
   //! v1: %0:v[0] = lds_direct_load %0:m0 wait_vdst:0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(8));
   bld.vop1(aco_opcode::v_sqrt_f32, Definition(PhysReg(256), v1), Operand(PhysReg(256), v1));
   bld.vop1(aco_opcode::v_nop);
   bld.ldsdir(aco_opcode::lds_direct_load, Definition(PhysReg(256), v1), Operand(m0, s1));

   /* transcendental is fine if it's before the instruction */
   //! p_unit_test 9
   //! v1: %0:v[1] = v_sqrt_f32 %0:v[1]
   //! v1: %0:v[0] = v_mov_b32 0
   //! v_nop
   //! v1: %0:v[0] = lds_direct_load %0:m0 wait_vdst:1
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(9));
   bld.vop1(aco_opcode::v_sqrt_f32, Definition(PhysReg(257), v1), Operand(PhysReg(257), v1));
   bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg(256), v1), Operand::zero());
   bld.vop1(aco_opcode::v_nop);
   bld.ldsdir(aco_opcode::lds_direct_load, Definition(PhysReg(256), v1), Operand(m0, s1));

   /* non-VALU does not increase wait_vdst */
   //! p_unit_test 10
   //! v1: %0:v[0] = v_mov_b32 0
   //! s1: %0:m0 = s_mov_b32 0
   //! v1: %0:v[0] = lds_direct_load %0:m0 wait_vdst:0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(10));
   bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg(256), v1), Operand::zero());
   bld.sop1(aco_opcode::s_mov_b32, Definition(m0, s1), Operand::zero());
   bld.ldsdir(aco_opcode::lds_direct_load, Definition(PhysReg(256), v1), Operand(m0, s1));

   /* consider instructions which wait on vdst */
   //! p_unit_test 11
   //! v1: %0:v[0] = v_mov_b32 0
   //! v_nop
   //! s_waitcnt_depctr va_vdst(0)
   //! v1: %0:v[0] = lds_direct_load %0:m0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(11));
   bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg(256), v1), Operand::zero());
   bld.vop1(aco_opcode::v_nop);
   bld.sopp(aco_opcode::s_waitcnt_depctr, -1, 0x0fff);
   bld.ldsdir(aco_opcode::lds_direct_load, Definition(PhysReg(256), v1), Operand(m0, s1));

   finish_insert_nops_test();
END_TEST

BEGIN_TEST(insert_nops.lds_direct_vmem)
   if (!setup_cs(NULL, GFX11))
      return;

   /* WaR: VMEM */
   //>> p_unit_test 0
   //! v1: %0:v[1] = buffer_load_dword %0:s[0-3], %0:v[0], 0 offen
   //! s_waitcnt_depctr vm_vsrc(0)
   //! v1: %0:v[0] = lds_direct_load %0:m0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(0));
   create_mubuf(0, PhysReg(257));
   bld.ldsdir(aco_opcode::lds_direct_load, Definition(PhysReg(256), v1), Operand(m0, s1));

   /* WaW: VMEM */
   //! p_unit_test 1
   //! v1: %0:v[0] = buffer_load_dword %0:s[0-3], %0:v[1], 0 offen
   //! s_waitcnt_depctr vm_vsrc(0)
   //! v1: %0:v[0] = lds_direct_load %0:m0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(1));
   create_mubuf(0, PhysReg(256), PhysReg(257));
   bld.ldsdir(aco_opcode::lds_direct_load, Definition(PhysReg(256), v1), Operand(m0, s1));

   /* no hazard: VMEM */
   //! p_unit_test 2
   //! v1: %0:v[1] = buffer_load_dword %0:s[0-3], %0:v[1], 0 offen
   //! v1: %0:v[0] = lds_direct_load %0:m0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(2));
   create_mubuf(0, PhysReg(257), PhysReg(257));
   bld.ldsdir(aco_opcode::lds_direct_load, Definition(PhysReg(256), v1), Operand(m0, s1));

   /* no hazard: VMEM with VALU in-between */
   //! p_unit_test 3
   //! v1: %0:v[1] = buffer_load_dword %0:s[0-3], %0:v[0], 0 offen
   //! v_nop
   //! v1: %0:v[0] = lds_direct_load %0:m0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(3));
   create_mubuf(0, PhysReg(257));
   bld.vop1(aco_opcode::v_nop);
   bld.ldsdir(aco_opcode::lds_direct_load, Definition(PhysReg(256), v1), Operand(m0, s1));

   /* WaR: LDS */
   //! p_unit_test 4
   //! v1: %0:v[1] = ds_read_b32 %0:v[0]
   //! s_waitcnt_depctr vm_vsrc(0)
   //! v1: %0:v[0] = lds_direct_load %0:m0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(4));
   bld.ds(aco_opcode::ds_read_b32, Definition(PhysReg(257), v1), Operand(PhysReg(256), v1));
   bld.ldsdir(aco_opcode::lds_direct_load, Definition(PhysReg(256), v1), Operand(m0, s1));

   /* WaW: LDS */
   //! p_unit_test 5
   //! v1: %0:v[0] = ds_read_b32 %0:v[1]
   //! s_waitcnt_depctr vm_vsrc(0)
   //! v1: %0:v[0] = lds_direct_load %0:m0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(5));
   bld.ds(aco_opcode::ds_read_b32, Definition(PhysReg(256), v1), Operand(PhysReg(257), v1));
   bld.ldsdir(aco_opcode::lds_direct_load, Definition(PhysReg(256), v1), Operand(m0, s1));

   /* no hazard: LDS */
   //! p_unit_test 6
   //! v1: %0:v[1] = ds_read_b32 %0:v[1]
   //! v1: %0:v[0] = lds_direct_load %0:m0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(6));
   bld.ds(aco_opcode::ds_read_b32, Definition(PhysReg(257), v1), Operand(PhysReg(257), v1));
   bld.ldsdir(aco_opcode::lds_direct_load, Definition(PhysReg(256), v1), Operand(m0, s1));

   /* no hazard: LDS with VALU in-between */
   //! p_unit_test 7
   //! v1: %0:v[1] = ds_read_b32 %0:v[0]
   //! v_nop
   //! v1: %0:v[0] = lds_direct_load %0:m0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(7));
   bld.ds(aco_opcode::ds_read_b32, Definition(PhysReg(257), v1), Operand(PhysReg(256), v1));
   bld.vop1(aco_opcode::v_nop);
   bld.ldsdir(aco_opcode::lds_direct_load, Definition(PhysReg(256), v1), Operand(m0, s1));

   /* no hazard: VMEM/LDS with the correct waitcnt in-between */
   //! p_unit_test 8
   //! v1: %0:v[1] = buffer_load_dword %0:s[0-3], %0:v[0], 0 offen
   //! s_waitcnt vmcnt(0)
   //! v1: %0:v[0] = lds_direct_load %0:m0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(8));
   create_mubuf(0, PhysReg(257));
   bld.sopp(aco_opcode::s_waitcnt, -1, 0x3ff);
   bld.ldsdir(aco_opcode::lds_direct_load, Definition(PhysReg(256), v1), Operand(m0, s1));

   //! p_unit_test 9
   //! buffer_store_dword %0:s[0-3], %0:v[0], 0, %0:v[0] offen
   //! s_waitcnt_vscnt %0:null imm:0
   //! v1: %0:v[0] = lds_direct_load %0:m0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(9));
   create_mubuf_store();
   bld.sopk(aco_opcode::s_waitcnt_vscnt, Operand(sgpr_null, s1), 0);
   bld.ldsdir(aco_opcode::lds_direct_load, Definition(PhysReg(256), v1), Operand(m0, s1));

   //! p_unit_test 10
   //! v1: %0:v[1] = ds_read_b32 %0:v[0]
   //! s_waitcnt lgkmcnt(0)
   //! v1: %0:v[0] = lds_direct_load %0:m0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(10));
   bld.ds(aco_opcode::ds_read_b32, Definition(PhysReg(257), v1), Operand(PhysReg(256), v1));
   bld.sopp(aco_opcode::s_waitcnt, -1, 0xfc0f);
   bld.ldsdir(aco_opcode::lds_direct_load, Definition(PhysReg(256), v1), Operand(m0, s1));

   /* VMEM/LDS with the wrong waitcnt in-between */
   //! p_unit_test 11
   //! v1: %0:v[1] = buffer_load_dword %0:s[0-3], %0:v[0], 0 offen
   //! s_waitcnt_vscnt %0:null imm:0
   //! s_waitcnt_depctr vm_vsrc(0)
   //! v1: %0:v[0] = lds_direct_load %0:m0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(11));
   create_mubuf(0, PhysReg(257));
   bld.sopk(aco_opcode::s_waitcnt_vscnt, Operand(sgpr_null, s1), 0);
   bld.ldsdir(aco_opcode::lds_direct_load, Definition(PhysReg(256), v1), Operand(m0, s1));

   //! p_unit_test 12
   //! buffer_store_dword %0:s[0-3], %0:v[0], 0, %0:v[0] offen
   //! s_waitcnt lgkmcnt(0)
   //! s_waitcnt_depctr vm_vsrc(0)
   //! v1: %0:v[0] = lds_direct_load %0:m0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(12));
   create_mubuf_store();
   bld.sopp(aco_opcode::s_waitcnt, -1, 0xfc0f);
   bld.ldsdir(aco_opcode::lds_direct_load, Definition(PhysReg(256), v1), Operand(m0, s1));

   //! p_unit_test 13
   //! v1: %0:v[1] = ds_read_b32 %0:v[0]
   //! s_waitcnt vmcnt(0)
   //! s_waitcnt_depctr vm_vsrc(0)
   //! v1: %0:v[0] = lds_direct_load %0:m0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(13));
   bld.ds(aco_opcode::ds_read_b32, Definition(PhysReg(257), v1), Operand(PhysReg(256), v1));
   bld.sopp(aco_opcode::s_waitcnt, -1, 0x3ff);
   bld.ldsdir(aco_opcode::lds_direct_load, Definition(PhysReg(256), v1), Operand(m0, s1));

   //! p_unit_test 14
   //! v1: %0:v[0] = buffer_load_dword %0:s[0-3], %0:v[1], 0 offen
   //! s_waitcnt_vscnt %0:null imm:0
   //! s_waitcnt_depctr vm_vsrc(0)
   //! v1: %0:v[0] = lds_direct_load %0:m0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(14));
   create_mubuf(0, PhysReg(256), PhysReg(257));
   bld.sopk(aco_opcode::s_waitcnt_vscnt, Operand(sgpr_null, s1), 0);
   bld.ldsdir(aco_opcode::lds_direct_load, Definition(PhysReg(256), v1), Operand(m0, s1));

   finish_insert_nops_test();
END_TEST

BEGIN_TEST(insert_nops.valu_trans_use)
   if (!setup_cs(NULL, GFX11))
      return;

   //>> p_unit_test 0
   //! v1: %0:v[0] = v_rcp_f32 %0:v[1]
   //! s_waitcnt_depctr va_vdst(0)
   //! v1: %0:v[1] = v_mov_b32 %0:v[0]
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(0));
   bld.vop1(aco_opcode::v_rcp_f32, Definition(PhysReg(256), v1), Operand(PhysReg(257), v1));
   bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg(257), v1), Operand(PhysReg(256), v1));

   /* Sufficient VALU mitigates the hazard. */
   //! p_unit_test 1
   //! v1: %0:v[0] = v_rcp_f32 %0:v[1]
   //; for i in range(4): insert_pattern('v_nop')
   //! s_waitcnt_depctr va_vdst(0)
   //! v1: %0:v[1] = v_mov_b32 %0:v[0]
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(1));
   bld.vop1(aco_opcode::v_rcp_f32, Definition(PhysReg(256), v1), Operand(PhysReg(257), v1));
   for (unsigned i = 0; i < 4; i++)
      bld.vop1(aco_opcode::v_nop);
   bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg(257), v1), Operand(PhysReg(256), v1));

   //! p_unit_test 2
   //! v1: %0:v[0] = v_rcp_f32 %0:v[1]
   //; for i in range(8): insert_pattern('v_nop')
   //! v1: %0:v[1] = v_mov_b32 %0:v[0]
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(2));
   bld.vop1(aco_opcode::v_rcp_f32, Definition(PhysReg(256), v1), Operand(PhysReg(257), v1));
   for (unsigned i = 0; i < 8; i++)
      bld.vop1(aco_opcode::v_nop);
   bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg(257), v1), Operand(PhysReg(256), v1));

   /* Sufficient transcendental VALU mitigates the hazard. */
   //! p_unit_test 3
   //! v1: %0:v[0] = v_rcp_f32 %0:v[1]
   //! v1: %0:v[2] = v_sqrt_f32 %0:v[3]
   //! s_waitcnt_depctr va_vdst(0)
   //! v1: %0:v[1] = v_mov_b32 %0:v[0]
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(3));
   bld.vop1(aco_opcode::v_rcp_f32, Definition(PhysReg(256), v1), Operand(PhysReg(257), v1));
   bld.vop1(aco_opcode::v_sqrt_f32, Definition(PhysReg(258), v1), Operand(PhysReg(259), v1));
   bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg(257), v1), Operand(PhysReg(256), v1));

   //! p_unit_test 4
   //! v1: %0:v[0] = v_rcp_f32 %0:v[1]
   //! v1: %0:v[2] = v_sqrt_f32 %0:v[3]
   //! v1: %0:v[2] = v_sqrt_f32 %0:v[3]
   //! v1: %0:v[1] = v_mov_b32 %0:v[0]
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(4));
   bld.vop1(aco_opcode::v_rcp_f32, Definition(PhysReg(256), v1), Operand(PhysReg(257), v1));
   for (unsigned i = 0; i < 2; i++)
      bld.vop1(aco_opcode::v_sqrt_f32, Definition(PhysReg(258), v1), Operand(PhysReg(259), v1));
   bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg(257), v1), Operand(PhysReg(256), v1));

   /* Transcendental VALU should be counted towards VALU */
   //! p_unit_test 5
   //! v1: %0:v[0] = v_rcp_f32 %0:v[1]
   //; for i in range(5): insert_pattern('v_nop')
   //! v1: %0:v[2] = v_sqrt_f32 %0:v[3]
   //! v1: %0:v[1] = v_mov_b32 %0:v[0]
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(5));
   bld.vop1(aco_opcode::v_rcp_f32, Definition(PhysReg(256), v1), Operand(PhysReg(257), v1));
   for (unsigned i = 0; i < 5; i++)
      bld.vop1(aco_opcode::v_nop);
   bld.vop1(aco_opcode::v_sqrt_f32, Definition(PhysReg(258), v1), Operand(PhysReg(259), v1));
   bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg(257), v1), Operand(PhysReg(256), v1));

   /* non-VALU does not mitigate the hazard. */
   //! p_unit_test 6
   //! v1: %0:v[0] = v_rcp_f32 %0:v[1]
   //; for i in range(8): insert_pattern('s_nop')
   //! s_waitcnt_depctr va_vdst(0)
   //! v1: %0:v[1] = v_mov_b32 %0:v[0]
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(6));
   bld.vop1(aco_opcode::v_rcp_f32, Definition(PhysReg(256), v1), Operand(PhysReg(257), v1));
   for (unsigned i = 0; i < 8; i++)
      bld.sopp(aco_opcode::s_nop, -1, 0);
   bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg(257), v1), Operand(PhysReg(256), v1));

   finish_insert_nops_test();
END_TEST

BEGIN_TEST(insert_nops.valu_partial_forwarding.basic)
   if (!setup_cs(NULL, GFX11))
      return;

   /* Basic case. */
   //>> p_unit_test 0
   //! v1: %0:v[0] = v_mov_b32 0
   //! s2: %0:exec = s_mov_b64 -1
   //! v1: %0:v[1] = v_mov_b32 1
   //! s_waitcnt_depctr va_vdst(0)
   //! v1: %0:v[2] = v_max_f32 %0:v[0], %0:v[1]
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(0));
   bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg(256), v1), Operand::zero());
   bld.sop1(aco_opcode::s_mov_b64, Definition(exec, s2), Operand::c64(-1));
   bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg(257), v1), Operand::c32(1));
   bld.vop2(aco_opcode::v_max_f32, Definition(PhysReg(258), v1), Operand(PhysReg(256), v1),
            Operand(PhysReg(257), v1));

   /* We should consider both the closest and further VALU after the exec write. */
   //! p_unit_test 1
   //! v1: %0:v[0] = v_mov_b32 0
   //! s2: %0:exec = s_mov_b64 -1
   //! v1: %0:v[1] = v_mov_b32 1
   //; for i in range(2): insert_pattern('v_nop')
   //! v1: %0:v[2] = v_mov_b32 2
   //! s_waitcnt_depctr va_vdst(0)
   //! v1: %0:v[2] = v_max3_f32 %0:v[0], %0:v[1], %0:v[2]
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(1));
   bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg(256), v1), Operand::zero());
   bld.sop1(aco_opcode::s_mov_b64, Definition(exec, s2), Operand::c64(-1));
   bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg(257), v1), Operand::c32(1));
   bld.vop1(aco_opcode::v_nop);
   bld.vop1(aco_opcode::v_nop);
   bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg(258), v1), Operand::c32(2));
   bld.vop3(aco_opcode::v_max3_f32, Definition(PhysReg(258), v1), Operand(PhysReg(256), v1),
            Operand(PhysReg(257), v1), Operand(PhysReg(258), v1));

   //! p_unit_test 2
   //! v1: %0:v[0] = v_mov_b32 0
   //! s2: %0:exec = s_mov_b64 -1
   //! v1: %0:v[1] = v_mov_b32 1
   //! v1: %0:v[2] = v_mov_b32 2
   //; for i in range(4): insert_pattern('v_nop')
   //! s_waitcnt_depctr va_vdst(0)
   //! v1: %0:v[2] = v_max3_f32 %0:v[0], %0:v[1], %0:v[2]
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(2));
   bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg(256), v1), Operand::zero());
   bld.sop1(aco_opcode::s_mov_b64, Definition(exec, s2), Operand::c64(-1));
   bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg(257), v1), Operand::c32(1));
   bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg(258), v1), Operand::c32(2));
   for (unsigned i = 0; i < 4; i++)
      bld.vop1(aco_opcode::v_nop);
   bld.vop3(aco_opcode::v_max3_f32, Definition(PhysReg(258), v1), Operand(PhysReg(256), v1),
            Operand(PhysReg(257), v1), Operand(PhysReg(258), v1));

   /* If a VALU writes a read VGPR in-between the first and second writes, it should still be
    * counted towards the distance between the first and second writes.
    */
   //! p_unit_test 3
   //! v1: %0:v[0] = v_mov_b32 0
   //! s2: %0:exec = s_mov_b64 -1
   //! v1: %0:v[1] = v_mov_b32 1
   //; for i in range(2): insert_pattern('v_nop')
   //! v1: %0:v[2] = v_mov_b32 2
   //; for i in range(3): insert_pattern('v_nop')
   //! v1: %0:v[2] = v_max3_f32 %0:v[0], %0:v[1], %0:v[2]
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(3));
   bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg(256), v1), Operand::zero());
   bld.sop1(aco_opcode::s_mov_b64, Definition(exec, s2), Operand::c64(-1));
   bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg(257), v1), Operand::c32(1));
   bld.vop1(aco_opcode::v_nop);
   bld.vop1(aco_opcode::v_nop);
   bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg(258), v1), Operand::c32(2));
   for (unsigned i = 0; i < 3; i++)
      bld.vop1(aco_opcode::v_nop);
   bld.vop3(aco_opcode::v_max3_f32, Definition(PhysReg(258), v1), Operand(PhysReg(256), v1),
            Operand(PhysReg(257), v1), Operand(PhysReg(258), v1));

   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(4));

   finish_insert_nops_test();
END_TEST

BEGIN_TEST(insert_nops.valu_partial_forwarding.multiple_exec_writes)
   if (!setup_cs(NULL, GFX11))
      return;

   //>> p_unit_test 0
   //! v1: %0:v[0] = v_mov_b32 0
   //! s2: %0:exec = s_mov_b64 0
   //! s2: %0:exec = s_mov_b64 -1
   //! v1: %0:v[1] = v_mov_b32 1
   //! s_waitcnt_depctr va_vdst(0)
   //! v1: %0:v[2] = v_max_f32 %0:v[0], %0:v[1]
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(0));
   bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg(256), v1), Operand::zero());
   bld.sop1(aco_opcode::s_mov_b64, Definition(exec, s2), Operand::c64(0));
   bld.sop1(aco_opcode::s_mov_b64, Definition(exec, s2), Operand::c64(-1));
   bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg(257), v1), Operand::c32(1));
   bld.vop2(aco_opcode::v_max_f32, Definition(PhysReg(258), v1), Operand(PhysReg(256), v1),
            Operand(PhysReg(257), v1));

   //! p_unit_test 1
   //! v1: %0:v[0] = v_mov_b32 0
   //! s2: %0:exec = s_mov_b64 0
   //! v1: %0:v[1] = v_mov_b32 1
   //! s2: %0:exec = s_mov_b64 -1
   //! s_waitcnt_depctr va_vdst(0)
   //! v1: %0:v[2] = v_max_f32 %0:v[0], %0:v[1]
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(1));
   bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg(256), v1), Operand::zero());
   bld.sop1(aco_opcode::s_mov_b64, Definition(exec, s2), Operand::c64(0));
   bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg(257), v1), Operand::c32(1));
   bld.sop1(aco_opcode::s_mov_b64, Definition(exec, s2), Operand::c64(-1));
   bld.vop2(aco_opcode::v_max_f32, Definition(PhysReg(258), v1), Operand(PhysReg(256), v1),
            Operand(PhysReg(257), v1));

   finish_insert_nops_test();
END_TEST

BEGIN_TEST(insert_nops.valu_partial_forwarding.control_flow)
   if (!setup_cs(NULL, GFX11))
      return;

   /* Control flow merges: one branch shouldn't interfere with the other (clobbering VALU closer
    * than interesting one).
    */
   //>> p_unit_test 0
   //! s_cbranch_scc1 block:BB2
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(0u));
   bld.sopp(aco_opcode::s_cbranch_scc1, 2);

   //! BB1
   //! /* logical preds: / linear preds: BB0, / kind: */
   //! v1: %0:v[0] = v_mov_b32 0
   //! s2: %0:exec = s_mov_b64 -1
   //! v_nop
   //! s_branch block:BB3
   bld.reset(program->create_and_insert_block());
   program->blocks[0].linear_succs.push_back(1);
   program->blocks[1].linear_preds.push_back(0);
   bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg(256), v1), Operand::zero());
   bld.sop1(aco_opcode::s_mov_b64, Definition(exec, s2), Operand::c64(-1));
   bld.vop1(aco_opcode::v_nop);
   bld.sopp(aco_opcode::s_branch, 3);

   //! BB2
   //! /* logical preds: / linear preds: BB0, / kind: */
   //! v1: %0:v[0] = v_mov_b32 0
   bld.reset(program->create_and_insert_block());
   program->blocks[0].linear_succs.push_back(2);
   program->blocks[2].linear_preds.push_back(0);
   bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg(256), v1), Operand::zero());

   //! BB3
   //! /* logical preds: / linear preds: BB1, BB2, / kind: */
   //! v1: %0:v[1] = v_mov_b32 1
   //! s_waitcnt_depctr va_vdst(0)
   //! v1: %0:v[2] = v_max_f32 %0:v[0], %0:v[1]
   bld.reset(program->create_and_insert_block());
   program->blocks[1].linear_succs.push_back(3);
   program->blocks[2].linear_succs.push_back(3);
   program->blocks[3].linear_preds.push_back(1);
   program->blocks[3].linear_preds.push_back(2);
   bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg(257), v1), Operand::c32(1));
   bld.vop2(aco_opcode::v_max_f32, Definition(PhysReg(258), v1), Operand(PhysReg(256), v1),
            Operand(PhysReg(257), v1));

   /* Control flow merges: one branch shouldn't interfere with the other (should consider furthest
    * VALU writes after exec).
    */
   //! p_unit_test 1
   //! s_cbranch_scc1 block:BB5
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(1u));
   bld.sopp(aco_opcode::s_cbranch_scc1, 5);

   //! BB4
   //! /* logical preds: / linear preds: BB3, / kind: */
   //! v1: %0:v[0] = v_mov_b32 0
   //! s2: %0:exec = s_mov_b64 -1
   //; for i in range(2): insert_pattern('v_nop')
   //! v1: %0:v[1] = v_mov_b32 1
   //! v_nop
   //! s_branch block:BB6
   bld.reset(program->create_and_insert_block());
   program->blocks[3].linear_succs.push_back(4);
   program->blocks[4].linear_preds.push_back(3);
   bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg(256), v1), Operand::zero());
   bld.sop1(aco_opcode::s_mov_b64, Definition(exec, s2), Operand::c64(-1));
   bld.vop1(aco_opcode::v_nop);
   bld.vop1(aco_opcode::v_nop);
   bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg(257), v1), Operand::c32(1));
   bld.vop1(aco_opcode::v_nop);
   bld.sopp(aco_opcode::s_branch, 6);

   //! BB5
   //! /* logical preds: / linear preds: BB3, / kind: */
   //! v1: %0:v[1] = v_mov_b32 1
   bld.reset(program->create_and_insert_block());
   program->blocks[3].linear_succs.push_back(5);
   program->blocks[5].linear_preds.push_back(3);
   bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg(257), v1), Operand::c32(1));

   //! BB6
   //! /* logical preds: / linear preds: BB4, BB5, / kind: */
   //! s_waitcnt_depctr va_vdst(0)
   //! v1: %0:v[2] = v_max_f32 %0:v[0], %0:v[1]
   bld.reset(program->create_and_insert_block());
   program->blocks[4].linear_succs.push_back(6);
   program->blocks[5].linear_succs.push_back(6);
   program->blocks[6].linear_preds.push_back(4);
   program->blocks[6].linear_preds.push_back(5);
   bld.vop2(aco_opcode::v_max_f32, Definition(PhysReg(258), v1), Operand(PhysReg(256), v1),
            Operand(PhysReg(257), v1));

   /* Control flow merges: one branch shouldn't interfere with the other (should consider closest
    * VALU writes after exec).
    */
   //! p_unit_test 2
   //! s_cbranch_scc1 block:BB8
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(2u));
   bld.sopp(aco_opcode::s_cbranch_scc1, 8);

   //! BB7
   //! /* logical preds: / linear preds: BB6, / kind: */
   //! v1: %0:v[0] = v_mov_b32 0
   //! s2: %0:exec = s_mov_b64 -1
   //! v1: %0:v[1] = v_mov_b32 1
   //; for i in range(4): insert_pattern('v_nop')
   //! s_branch block:BB9
   bld.reset(program->create_and_insert_block());
   program->blocks[6].linear_succs.push_back(7);
   program->blocks[7].linear_preds.push_back(6);
   bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg(256), v1), Operand::zero());
   bld.sop1(aco_opcode::s_mov_b64, Definition(exec, s2), Operand::c64(-1));
   bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg(257), v1), Operand::c32(1));
   for (unsigned i = 0; i < 4; i++)
      bld.vop1(aco_opcode::v_nop);
   bld.sopp(aco_opcode::s_branch, 9);

   //! BB8
   //! /* logical preds: / linear preds: BB6, / kind: */
   //! v1: %0:v[1] = v_mov_b32 1
   //; for i in range(5): insert_pattern('v_nop')
   bld.reset(program->create_and_insert_block());
   program->blocks[6].linear_succs.push_back(8);
   program->blocks[8].linear_preds.push_back(6);
   bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg(257), v1), Operand::c32(1));
   for (unsigned i = 0; i < 5; i++)
      bld.vop1(aco_opcode::v_nop);

   //! BB9
   //! /* logical preds: / linear preds: BB7, BB8, / kind: uniform, */
   //! s_waitcnt_depctr va_vdst(0)
   //! v1: %0:v[2] = v_max_f32 %0:v[0], %0:v[1]
   bld.reset(program->create_and_insert_block());
   program->blocks[7].linear_succs.push_back(9);
   program->blocks[8].linear_succs.push_back(9);
   program->blocks[9].linear_preds.push_back(7);
   program->blocks[9].linear_preds.push_back(8);
   bld.vop2(aco_opcode::v_max_f32, Definition(PhysReg(258), v1), Operand(PhysReg(256), v1),
            Operand(PhysReg(257), v1));

   finish_insert_nops_test();
END_TEST

BEGIN_TEST(insert_nops.valu_mask_write)
   if (!setup_cs(NULL, GFX11))
      return;

   /* Basic case. */
   //>> p_unit_test 0
   //! v1: %0:v[0] = v_cndmask_b32 0, 0, %0:s[0-1]
   //! s1: %0:s[1] = s_mov_b32 0
   //! s_waitcnt_depctr sa_sdst(0)
   //! s1: %0:s[2] = s_mov_b32 %0:s[1]
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(0));
   bld.vop2_e64(aco_opcode::v_cndmask_b32, Definition(PhysReg(256), v1), Operand::zero(),
                Operand::zero(), Operand(PhysReg(0), s2));
   bld.sop1(aco_opcode::s_mov_b32, Definition(PhysReg(1), s1), Operand::zero());
   bld.sop1(aco_opcode::s_mov_b32, Definition(PhysReg(2), s1), Operand(PhysReg(1), s1));

   /* Mitigation. */
   //! p_unit_test 1
   //! v1: %0:v[0] = v_cndmask_b32 0, 0, %0:s[0-1]
   //! v1: %0:v[1] = v_mov_b32 %0:s[1]
   //! s1: %0:s[1] = s_mov_b32 0
   //! s1: %0:s[2] = s_mov_b32 %0:s[1]
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(1));
   bld.vop2_e64(aco_opcode::v_cndmask_b32, Definition(PhysReg(256), v1), Operand::zero(),
                Operand::zero(), Operand(PhysReg(0), s2));
   bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg(257), v1), Operand(PhysReg(1), s1));
   bld.sop1(aco_opcode::s_mov_b32, Definition(PhysReg(1), s1), Operand::zero());
   bld.sop1(aco_opcode::s_mov_b32, Definition(PhysReg(2), s1), Operand(PhysReg(1), s1));

   //! p_unit_test 2
   //! v1: %0:v[0] = v_cndmask_b32 0, 0, %0:s[0-1]
   //! s1: %0:s[1] = s_mov_b32 0
   //! s_waitcnt_depctr sa_sdst(0)
   //! s1: %0:s[2] = s_mov_b32 %0:s[1]
   //! s1: %0:s[2] = s_mov_b32 %0:s[1]
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(2));
   bld.vop2_e64(aco_opcode::v_cndmask_b32, Definition(PhysReg(256), v1), Operand::zero(),
                Operand::zero(), Operand(PhysReg(0), s2));
   bld.sop1(aco_opcode::s_mov_b32, Definition(PhysReg(1), s1), Operand::zero());
   bld.sop1(aco_opcode::s_mov_b32, Definition(PhysReg(2), s1), Operand(PhysReg(1), s1));
   bld.sop1(aco_opcode::s_mov_b32, Definition(PhysReg(2), s1), Operand(PhysReg(1), s1));

   //! p_unit_test 3
   //! v1: %0:v[0] = v_cndmask_b32 0, 0, %0:s[0-1]
   //! s1: %0:s[1] = s_mov_b32 0
   //! s_waitcnt_depctr sa_sdst(0)
   //! s1: %0:s[2] = s_mov_b32 %0:s[1]
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(3));
   bld.vop2_e64(aco_opcode::v_cndmask_b32, Definition(PhysReg(256), v1), Operand::zero(),
                Operand::zero(), Operand(PhysReg(0), s2));
   bld.sop1(aco_opcode::s_mov_b32, Definition(PhysReg(1), s1), Operand::zero());
   bld.sopp(aco_opcode::s_waitcnt_depctr, -1, 0xfffe);
   bld.sop1(aco_opcode::s_mov_b32, Definition(PhysReg(2), s1), Operand(PhysReg(1), s1));

   /* Instruction which is both involved in the hazard and is a mitigation. */
   //! p_unit_test 4
   //! v1: %0:v[0] = v_cndmask_b32 %0:s[2], 0, %0:s[0-1]
   //! s1: %0:s[1] = s_mov_b32 0
   //! s_waitcnt_depctr sa_sdst(0)
   //! s1: %0:s[2] = s_mov_b32 %0:s[1]
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(4));
   bld.vop2_e64(aco_opcode::v_cndmask_b32, Definition(PhysReg(256), v1), Operand(PhysReg(2), s1),
                Operand::zero(), Operand(PhysReg(0), s2));
   bld.sop1(aco_opcode::s_mov_b32, Definition(PhysReg(1), s1), Operand::zero());
   bld.sop1(aco_opcode::s_mov_b32, Definition(PhysReg(2), s1), Operand(PhysReg(1), s1));

   finish_insert_nops_test();
END_TEST

BEGIN_TEST(insert_nops.setpc_gfx6)
   if (!setup_cs(NULL, GFX6))
      return;

   /* SGPR->SMEM hazards */
   //>> p_unit_test 0
   //! s1: %0:s[0] = s_mov_b32 0
   //! s_nop imm:2
   //! s_setpc_b64 0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(0));
   bld.sop1(aco_opcode::s_mov_b32, Definition(PhysReg(0), s1), Operand::zero());
   bld.sop1(aco_opcode::s_setpc_b64, Operand::zero(8));

   //! p_unit_test 1
   //! s1: %0:s[0] = s_mov_b32 0
   //! s_nop imm:2
   //! s_setpc_b64 0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(1));
   bld.sop1(aco_opcode::s_mov_b32, Definition(PhysReg(0), s1), Operand::zero());
   bld.sopp(aco_opcode::s_nop, -1, 2);
   bld.sop1(aco_opcode::s_setpc_b64, Operand::zero(8));

   finish_insert_nops_test();

   /* This hazard can't be tested using s_setpc_b64, because the s_setpc_b64 itself resolves it. */

   /* VINTRP->v_readlane_b32/etc */
   //>> p_unit_test 2
   //! v1: %0:v[0] = v_interp_mov_f32 2, %0:m0 attr0.x
   //! s_nop
   create_program(GFX6, compute_cs, 64, CHIP_UNKNOWN);
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(2));
   bld.vintrp(aco_opcode::v_interp_mov_f32, Definition(PhysReg(256), v1), Operand::c32(2u),
              Operand(m0, s1), 0, 0);
   finish_insert_nops_test(false);
END_TEST

BEGIN_TEST(insert_nops.setpc_gfx7)
   for (amd_gfx_level gfx : {GFX7, GFX9}) {
      if (!setup_cs(NULL, gfx))
         continue;

      //>> p_unit_test 0
      //! s_setpc_b64 0
      bld.pseudo(aco_opcode::p_unit_test, Operand::c32(0));
      bld.sop1(aco_opcode::s_setpc_b64, Operand::zero(8));

      /* Break up SMEM clauses: resolved by the s_setpc_b64 itself */
      //! p_unit_test 1
      //! s1: %0:s[0] = s_load_dword %0:s[0-1]
      //! s_setpc_b64 0
      bld.pseudo(aco_opcode::p_unit_test, Operand::c32(1));
      bld.smem(aco_opcode::s_load_dword, Definition(PhysReg(0), s1), Operand(PhysReg(0), s2));
      bld.sop1(aco_opcode::s_setpc_b64, Operand::zero(8));

      /* SALU and GDS hazards */
      //! p_unit_test 2
      //! s_setreg_imm32_b32 0x0 imm:14337
      //! s_nop
      //! s_setpc_b64 0
      bld.pseudo(aco_opcode::p_unit_test, Operand::c32(2));
      bld.sopk(aco_opcode::s_setreg_imm32_b32, Operand::literal32(0), (7 << 11) | 1);
      bld.sop1(aco_opcode::s_setpc_b64, Operand::zero(8));

      /* VALU writes vcc -> vccz/v_div_fmas */
      //! p_unit_test 3
      //! s2: %0:vcc = v_cmp_eq_u32 0, 0
      //! s_nop imm:3
      //! s_setpc_b64 0
      bld.pseudo(aco_opcode::p_unit_test, Operand::c32(3));
      bld.vopc_e64(aco_opcode::v_cmp_eq_u32, Definition(vcc, s2), Operand::zero(), Operand::zero());
      bld.sop1(aco_opcode::s_setpc_b64, Operand::zero(8));

      /* VALU writes exec -> execz/DPP */
      //! p_unit_test 4
      //! s2: %0:exec = v_cmpx_eq_u32 0, 0
      //! s_nop imm:3
      //! s_setpc_b64 0
      bld.pseudo(aco_opcode::p_unit_test, Operand::c32(4));
      bld.vopc_e64(aco_opcode::v_cmpx_eq_u32, Definition(exec, s2), Operand::zero(),
                   Operand::zero());
      bld.sop1(aco_opcode::s_setpc_b64, Operand::zero(8));

      /* VALU->DPP */
      //! p_unit_test 5
      //! v1: %0:v[0] = v_mov_b32 0
      //~gfx9! s_nop
      //! s_setpc_b64 0
      bld.pseudo(aco_opcode::p_unit_test, Operand::c32(5));
      bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg(256), v1), Operand::zero());
      bld.sop1(aco_opcode::s_setpc_b64, Operand::zero(8));

      /* VALU->v_readlane_b32/VMEM/etc */
      //! p_unit_test 6
      //! s1: %0:s[0] = v_readfirstlane_b32 %0:v[0]
      //! s_nop imm:3
      //! s_setpc_b64 0
      bld.pseudo(aco_opcode::p_unit_test, Operand::c32(6));
      bld.vop1(aco_opcode::v_readfirstlane_b32, Definition(PhysReg(0), s1),
               Operand(PhysReg(256), v1));
      bld.sop1(aco_opcode::s_setpc_b64, Operand::zero(8));

      finish_insert_nops_test();

      /* These hazards can't be tested using s_setpc_b64, because the s_setpc_b64 itself resolves
       * them. */

      //>> p_unit_test 7
      //! buffer_store_dwordx3 %0:s[0-3], %0:v[0], 0, %0:v[0-2] offen
      //! s_nop
      create_program(gfx, compute_cs, 64, CHIP_UNKNOWN);
      bld.pseudo(aco_opcode::p_unit_test, Operand::c32(7));
      bld.mubuf(aco_opcode::buffer_store_dwordx3, Operand(PhysReg(0), s4),
                Operand(PhysReg(256), v1), Operand::zero(), Operand(PhysReg(256), v3), 0, true);
      finish_insert_nops_test(false);

      //>> p_unit_test 8
      //! s1: %0:m0 = s_mov_b32 0
      //! s_nop
      create_program(gfx, compute_cs, 64, CHIP_UNKNOWN);
      bld.pseudo(aco_opcode::p_unit_test, Operand::c32(8));
      bld.sop1(aco_opcode::s_mov_b32, Definition(PhysReg(m0), s1), Operand::zero());
      finish_insert_nops_test(false);

      /* Break up SMEM clauses */
      //>> p_unit_test 9
      //! s1: %0:s[0] = s_load_dword %0:s[0-1]
      //! s_nop
      create_program(gfx, compute_cs, 64, CHIP_UNKNOWN);
      bld.pseudo(aco_opcode::p_unit_test, Operand::c32(9));
      bld.smem(aco_opcode::s_load_dword, Definition(PhysReg(0), s1), Operand(PhysReg(0), s2));
      finish_insert_nops_test(false);
   }
END_TEST

BEGIN_TEST(insert_nops.setpc_gfx10)
   if (!setup_cs(NULL, GFX10))
      return;

   //>> p_unit_test 0
   //! s_setpc_b64 0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(0));
   bld.sop1(aco_opcode::s_setpc_b64, Operand::zero(8));

   /* VcmpxPermlaneHazard */
   //! p_unit_test 1
   //! s2: %0:exec = v_cmpx_eq_u32 0, 0
   //! v1: %0:v[0] = v_mov_b32 %0:v[0]
   //! s_setpc_b64 0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(1));
   bld.vopc_e64(aco_opcode::v_cmpx_eq_u32, Definition(exec, s2), Operand::zero(), Operand::zero());
   bld.sop1(aco_opcode::s_setpc_b64, Operand::zero(8));

   /* VMEMtoScalarWriteHazard */
   //! p_unit_test 2
   //! v1: %0:v[0] = ds_read_b32 %0:v[0]
   //! s_waitcnt_vscnt %0:null imm:0
   //! s_waitcnt_depctr vm_vsrc(0)
   //! s_setpc_b64 0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(2));
   bld.ds(aco_opcode::ds_read_b32, Definition(PhysReg(256), v1), Operand(PhysReg(256), v1));
   bld.sopk(aco_opcode::s_waitcnt_vscnt, Operand(sgpr_null, s1),
            0); /* reset LdsBranchVmemWARHazard */
   bld.sop1(aco_opcode::s_setpc_b64, Operand::zero(8));

   /* VcmpxExecWARHazard */
   //! p_unit_test 3
   //! s1: %0:s[0] = s_mov_b32 %0:s[127]
   //! s_waitcnt_depctr sa_sdst(0)
   //! s_setpc_b64 0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(3));
   bld.sop1(aco_opcode::s_mov_b32, Definition(PhysReg(0), s1), Operand(exec_hi, s1));
   bld.sop1(aco_opcode::s_setpc_b64, Operand::zero(8));

   /* LdsBranchVmemWARHazard */
   //! p_unit_test 4
   //! v1: %0:v[0] = ds_read_b32 %0:v[0]
   //! v_nop
   //! s_branch
   //! s_waitcnt_vscnt %0:null imm:0
   //! s_setpc_b64 0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(4));
   bld.ds(aco_opcode::ds_read_b32, Definition(PhysReg(256), v1), Operand(PhysReg(256), v1));
   bld.vop1(aco_opcode::v_nop); /* reset VMEMtoScalarWriteHazard */
   bld.sopp(aco_opcode::s_branch, -1, 0);
   bld.sop1(aco_opcode::s_setpc_b64, Operand::zero(8));

   //! p_unit_test 5
   //! v1: %0:v[0] = ds_read_b32 %0:v[0]
   //! v_nop
   //! s_waitcnt_vscnt %0:null imm:0
   //! s_setpc_b64 0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(5));
   bld.ds(aco_opcode::ds_read_b32, Definition(PhysReg(256), v1), Operand(PhysReg(256), v1));
   bld.vop1(aco_opcode::v_nop); /* reset VMEMtoScalarWriteHazard */
   bld.sop1(aco_opcode::s_setpc_b64, Operand::zero(8));

   /* waNsaCannotFollowWritelane: resolved by the s_setpc_b64 */
   //! p_unit_test 6
   //! v1: %0:v[0] = v_writelane_b32_e64 %0:v[1], 0, %0:v[0]
   //! s_setpc_b64 0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(6));
   bld.vop3(aco_opcode::v_writelane_b32_e64, Definition(PhysReg(256), v1),
            Operand(PhysReg(257), v1), Operand::zero(4), Operand(PhysReg(256), v1));
   bld.sop1(aco_opcode::s_setpc_b64, Operand::zero(8));

   finish_insert_nops_test();

   /* These hazards can't be tested using s_setpc_b64, because the s_setpc_b64 itself resolves them.
    */

   /* SMEMtoVectorWriteHazard */
   //>> p_unit_test 7
   //! s1: %0:s[0] = s_load_dword %0:s[0-1]
   //! s1: %0:null = s_mov_b32 0
   create_program(GFX10, compute_cs, 64, CHIP_UNKNOWN);
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(7));
   bld.smem(aco_opcode::s_load_dword, Definition(PhysReg(0), s1), Operand(PhysReg(0), s2));
   finish_insert_nops_test(false);

   /* NSAToVMEMBug is already resolved indirectly through VMEMtoScalarWriteHazard and
    * LdsBranchVmemWARHazard. */
   //>> p_unit_test 8
   //! v1: %0:v[0] = image_sample %0:s[0-7], %0:s[0-3],  v1: undef, %0:v[0], %0:v[2], %0:v[4], %0:v[6], %0:v[8], %0:v[10] 2d
   //! s_waitcnt_depctr vm_vsrc(0)
   //! s_waitcnt_vscnt %0:null imm:0
   create_program(GFX10, compute_cs, 64, CHIP_UNKNOWN);
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(8));
   create_mimg(true, 6, 4);
   finish_insert_nops_test(false);

   /* waNsaCannotFollowWritelane */
   //>> p_unit_test 9
   //! v1: %0:v[0] = v_writelane_b32_e64 %0:v[1], 0, %0:v[0]
   //! s_nop
   create_program(GFX10, compute_cs, 64, CHIP_UNKNOWN);
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(9));
   bld.vop3(aco_opcode::v_writelane_b32_e64, Definition(PhysReg(256), v1),
            Operand(PhysReg(257), v1), Operand::zero(4), Operand(PhysReg(256), v1));
   finish_insert_nops_test(false);
END_TEST

BEGIN_TEST(insert_nops.setpc_gfx11)
   if (!setup_cs(NULL, GFX11))
      return;

   //>> p_unit_test 0
   //! s_setpc_b64 0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(0));
   bld.sop1(aco_opcode::s_setpc_b64, Operand::zero(8));

   /* LdsDirectVALUHazard */
   //! p_unit_test 1
   //! s2: %0:vcc = v_cmp_eq_u32 %0:v[0], 0
   //! s_waitcnt_depctr va_vdst(0)
   //! s_setpc_b64 0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(1));
   bld.vopc_e64(aco_opcode::v_cmp_eq_u32, Definition(vcc, s2), Operand(PhysReg(256), v1),
                Operand::zero());
   bld.sop1(aco_opcode::s_setpc_b64, Operand::zero(8));

   /* VALUPartialForwardingHazard */
   //! p_unit_test 2
   //! v1: %0:v[0] = v_mov_b32 0
   //! s_waitcnt_depctr va_vdst(0)
   //! s_setpc_b64 0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(2));
   bld.vop1(aco_opcode::v_mov_b32, Definition(PhysReg(256), v1), Operand::zero());
   bld.sop1(aco_opcode::s_setpc_b64, Operand::zero(8));

   /* VcmpxPermlaneHazard */
   //! p_unit_test 2
   //! s2: %0:exec = v_cmpx_eq_u32 0, 0
   //! v1: %0:v[0] = v_mov_b32 %0:v[0]
   //! s_setpc_b64 0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(2));
   bld.vopc_e64(aco_opcode::v_cmpx_eq_u32, Definition(exec, s2), Operand::zero(), Operand::zero());
   bld.sop1(aco_opcode::s_setpc_b64, Operand::zero(8));

   /* VALUTransUseHazard */
   //! p_unit_test 3
   //! v1: %0:v[0] = v_rcp_f32 0
   //! s_waitcnt_depctr va_vdst(0)
   //! s_setpc_b64 0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(3));
   bld.vop1(aco_opcode::v_rcp_f32, Definition(PhysReg(256), v1), Operand::zero());
   bld.sop1(aco_opcode::s_setpc_b64, Operand::zero(8));

   /* VALUMaskWriteHazard */
   //! p_unit_test 4
   //! v1: %0:v[0] = v_cndmask_b32 0, 0, %0:vcc
   //! s_waitcnt_depctr va_vdst(0) sa_sdst(0)
   //! s_setpc_b64 0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(4));
   bld.vop2(aco_opcode::v_cndmask_b32, Definition(PhysReg(256), v1), Operand::zero(),
            Operand::zero(), Operand(vcc, s2));
   bld.sop1(aco_opcode::s_setpc_b64, Operand::zero(8));

   //! p_unit_test 5
   //! v1: %0:v[0] = v_cndmask_b32 0, 0, %0:vcc
   //! s2: %0:vcc = s_mov_b64 0
   //! s_waitcnt_depctr va_vdst(0) sa_sdst(0)
   //! s_setpc_b64 0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(5));
   bld.vop2(aco_opcode::v_cndmask_b32, Definition(PhysReg(256), v1), Operand::zero(),
            Operand::zero(), Operand(vcc, s2));
   bld.sop1(aco_opcode::s_mov_b64, Definition(vcc, s2), Operand::zero(8));
   bld.sop1(aco_opcode::s_setpc_b64, Operand::zero(8));

   /* LdsDirectVMEMHazard */
   //! p_unit_test 6
   //! v1: %0:v[0] = ds_read_b32 %0:v[0]
   //! s_waitcnt_depctr vm_vsrc(0)
   //! s_setpc_b64 0
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(6));
   bld.ds(aco_opcode::ds_read_b32, Definition(PhysReg(256), v1), Operand(PhysReg(256), v1));
   bld.sop1(aco_opcode::s_setpc_b64, Operand::zero(8));

   finish_insert_nops_test(true);
}
