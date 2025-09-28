/* microblaze-opcm.h -- Header used in microblaze-opc.h

   Copyright (C) 2009-2023 Free Software Foundation, Inc.

   This file is part of the GNU opcodes library.

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   It is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with this file; see the file COPYING.  If not, write to the
   Free Software Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */


#ifndef MICROBLAZE_OPCM
#define MICROBLAZE_OPCM

enum microblaze_instr
{
  add, rsub, addc, rsubc, addk, rsubk, addkc, rsubkc, clz, cmp, cmpu,
  addi, rsubi, addic, rsubic, addik, rsubik, addikc, rsubikc, mul,
  mulh, mulhu, mulhsu,swapb,swaph,
  idiv, idivu, bsll, bsra, bsrl, get, put, nget, nput, cget, cput,
  ncget, ncput, muli, bslli, bsrai, bsrli, mului,
  /* 'or/and/xor' are C++ keywords.  */
  microblaze_or, microblaze_and, microblaze_xor,
  andn, pcmpbf, pcmpbc, pcmpeq, pcmpne, sra, src, srl, sext8, sext16,
  wic, wdc, wdcclear, wdcflush, mts, mfs, mbar, br, brd,
  brld, bra, brad, brald, microblaze_brk, beq, beqd, bne, bned, blt,
  bltd, ble, bled, bgt, bgtd, bge, bged, ori, andi, xori, andni,
  imm, rtsd, rtid, rtbd, rted, bri, brid, brlid, brai, braid, bralid,
  brki, beqi, beqid, bnei, bneid, blti, bltid, blei, bleid, bgti,
  bgtid, bgei, bgeid, lbu, lbur, lhu, lhur, lw, lwr, lwx, sb, sbr, sh,
  shr, sw, swr, swx, lbui, lhui, lwi,
  sbi, shi, swi, msrset, msrclr, tuqula, mbi_fadd, frsub, mbi_fmul, mbi_fdiv,
  fcmp_lt, fcmp_eq, fcmp_le, fcmp_gt, fcmp_ne, fcmp_ge, fcmp_un, flt,
  /* 'fsqrt' is a glibc:math.h symbol.  */
  fint, microblaze_fsqrt,
  tget, tcget, tnget, tncget, tput, tcput, tnput, tncput,
  eget, ecget, neget, necget, eput, ecput, neput, necput,
  teget, tecget, tneget, tnecget, teput, tecput, tneput, tnecput,
  aget, caget, naget, ncaget, aput, caput, naput, ncaput,
  taget, tcaget, tnaget, tncaget, taput, tcaput, tnaput, tncaput,
  eaget, ecaget, neaget, necaget, eaput, ecaput, neaput, necaput,
  teaget, tecaget, tneaget, tnecaget, teaput, tecaput, tneaput, tnecaput,
  getd, tgetd, cgetd, tcgetd, ngetd, tngetd, ncgetd, tncgetd,
  putd, tputd, cputd, tcputd, nputd, tnputd, ncputd, tncputd,
  egetd, tegetd, ecgetd, tecgetd, negetd, tnegetd, necgetd, tnecgetd,
  eputd, teputd, ecputd, tecputd, neputd, tneputd, necputd, tnecputd,
  agetd, tagetd, cagetd, tcagetd, nagetd, tnagetd, ncagetd, tncagetd,
  aputd, taputd, caputd, tcaputd, naputd, tnaputd, ncaputd, tncaputd,
  eagetd, teagetd, ecagetd, tecagetd, neagetd, tneagetd, necagetd, tnecagetd,
  eaputd, teaputd, ecaputd, tecaputd, neaputd, tneaputd, necaputd, tnecaputd,
  invalid_inst
};

enum microblaze_instr_type
{
  arithmetic_inst, logical_inst, mult_inst, div_inst, branch_inst,
  return_inst, immediate_inst, special_inst, memory_load_inst,
  memory_store_inst, barrel_shift_inst, anyware_inst
};

#define INST_WORD_SIZE 4

/* Gen purpose regs go from 0 to 31.  */
/* Mask is reg num - max_reg_num, ie reg_num - 32 in this case.  */

#define REG_PC_MASK 0x8000
#define REG_MSR_MASK 0x8001
#define REG_EAR_MASK 0x8003
#define REG_ESR_MASK 0x8005
#define REG_FSR_MASK 0x8007
#define REG_BTR_MASK 0x800b
#define REG_EDR_MASK 0x800d
#define REG_PVR_MASK 0xa000
#define REG_SLR_MASK 0x8800
#define REG_SHR_MASK 0x8802

#define REG_PID_MASK   0x9000
#define REG_ZPR_MASK   0x9001
#define REG_TLBX_MASK  0x9002
#define REG_TLBLO_MASK 0x9003
#define REG_TLBHI_MASK 0x9004
#define REG_TLBSX_MASK 0x9005

#define MIN_REGNUM 0
#define MAX_REGNUM 31

#define MIN_PVR_REGNUM 0
#define MAX_PVR_REGNUM 15

#define REG_PC  32 /* PC.  */
#define REG_MSR 33 /* Machine status reg.  */
#define REG_EAR 35 /* Exception reg.  */
#define REG_ESR 37 /* Exception reg.  */
#define REG_FSR 39 /* FPU Status reg.  */
#define REG_BTR 43 /* Branch Target reg.  */
#define REG_EDR 45 /* Exception reg.  */
#define REG_SHR 50 /* Stack High reg.  */
#define REG_SLR 51 /* Stack Low reg.  */
#define REG_PVR 40960 /* Program Verification reg.  */

#define REG_PID   36864 /* MMU: Process ID reg.  */
#define REG_ZPR   36865 /* MMU: Zone Protect reg.  */
#define REG_TLBX  36866 /* MMU: TLB Index reg.  */
#define REG_TLBLO 36867 /* MMU: TLB Low reg.  */
#define REG_TLBHI 36868 /* MMU: TLB High reg.  */
#define REG_TLBSX 36869 /* MMU: TLB Search Index reg.  */

/* Alternate names for gen purpose regs.  */
#define REG_SP  1 /* stack pointer.  */
#define REG_ROSDP 2 /* read-only small data pointer.  */
#define REG_RWSDP 13 /* read-write small data pointer.  */

/* Assembler Register - Used in Delay Slot Optimization.  */
#define REG_AS    18
#define REG_ZERO  0

#define RD_LOW  21 /* Low bit for RD.  */
#define RA_LOW  16 /* Low bit for RA.  */
#define RB_LOW  11 /* Low bit for RB.  */
#define IMM_LOW  0 /* Low bit for immediate.  */
#define IMM_MBAR 21 /* low bit for mbar instruction.  */

#define RD_MASK 0x03E00000
#define RA_MASK 0x001F0000
#define RB_MASK 0x0000F800
#define IMM_MASK 0x0000FFFF

/* Imm mask for barrel shifts.  */
#define IMM5_MASK 0x0000001F

/* Imm mask for mbar.  */
#define IMM5_MBAR_MASK 0x03E00000

/* FSL imm mask for get, put instructions.  */
#define  RFSL_MASK 0x000000F

/* Imm mask for msrset, msrclr instructions.  */
#define  IMM15_MASK 0x00007FFF

#endif /* MICROBLAZE-OPCM */
