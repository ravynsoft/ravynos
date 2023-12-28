/* Header only used inside opcodes library for disassemble.

   Copyright (C) 2017-2023 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor,
   Boston, MA 02110-1301, USA.  */

#ifndef DISASSEMBLE_H
#define DISASSEMBLE_H
#include "dis-asm.h"

extern int print_insn_aarch64		(bfd_vma, disassemble_info *);
extern int print_insn_alpha		(bfd_vma, disassemble_info *);
extern int print_insn_avr		(bfd_vma, disassemble_info *);
extern int print_insn_bfin		(bfd_vma, disassemble_info *);
extern int print_insn_big_arm		(bfd_vma, disassemble_info *);
extern int print_insn_big_mips		(bfd_vma, disassemble_info *);
extern int print_insn_big_nios2	(bfd_vma, disassemble_info *);
extern int print_insn_big_powerpc	(bfd_vma, disassemble_info *);
extern int print_insn_big_score         (bfd_vma, disassemble_info *);
extern int print_insn_cr16              (bfd_vma, disassemble_info *);
extern int print_insn_crx               (bfd_vma, disassemble_info *);
extern int print_insn_csky		(bfd_vma, disassemble_info *);
extern int print_insn_d10v		(bfd_vma, disassemble_info *);
extern int print_insn_d30v		(bfd_vma, disassemble_info *);
extern int print_insn_dlx		(bfd_vma, disassemble_info *);
extern int print_insn_bpf		(bfd_vma, disassemble_info *);
extern int print_insn_epiphany		(bfd_vma, disassemble_info *);
extern int print_insn_fr30		(bfd_vma, disassemble_info *);
extern int print_insn_frv		(bfd_vma, disassemble_info *);
extern int print_insn_ft32		(bfd_vma, disassemble_info *);
extern int print_insn_h8300		(bfd_vma, disassemble_info *);
extern int print_insn_h8300h		(bfd_vma, disassemble_info *);
extern int print_insn_h8300s		(bfd_vma, disassemble_info *);
extern int print_insn_hppa		(bfd_vma, disassemble_info *);
extern int print_insn_i386		(bfd_vma, disassemble_info *);
extern int print_insn_i386_att		(bfd_vma, disassemble_info *);
extern int print_insn_i386_intel	(bfd_vma, disassemble_info *);
extern int print_insn_ia64		(bfd_vma, disassemble_info *);
extern int print_insn_ip2k		(bfd_vma, disassemble_info *);
extern int print_insn_iq2000		(bfd_vma, disassemble_info *);
extern int print_insn_little_nios2	(bfd_vma, disassemble_info *);
extern int print_insn_riscv		(bfd_vma, disassemble_info *);
extern int print_insn_little_arm	(bfd_vma, disassemble_info *);
extern int print_insn_little_mips	(bfd_vma, disassemble_info *);
extern int print_insn_little_powerpc	(bfd_vma, disassemble_info *);
extern int print_insn_little_score      (bfd_vma, disassemble_info *);
extern int print_insn_lm32		(bfd_vma, disassemble_info *);
extern int print_insn_m32r		(bfd_vma, disassemble_info *);
extern int print_insn_m68hc11		(bfd_vma, disassemble_info *);
extern int print_insn_m68hc12		(bfd_vma, disassemble_info *);
extern int print_insn_m9s12x		(bfd_vma, disassemble_info *);
extern int print_insn_m9s12xg		(bfd_vma, disassemble_info *);
extern int print_insn_s12z		(bfd_vma, disassemble_info *);
extern int print_insn_m68k		(bfd_vma, disassemble_info *);
extern int print_insn_mcore		(bfd_vma, disassemble_info *);
extern int print_insn_metag		(bfd_vma, disassemble_info *);
extern int print_insn_microblaze	(bfd_vma, disassemble_info *);
extern int print_insn_mmix		(bfd_vma, disassemble_info *);
extern int print_insn_mn10200		(bfd_vma, disassemble_info *);
extern int print_insn_mn10300		(bfd_vma, disassemble_info *);
extern int print_insn_moxie		(bfd_vma, disassemble_info *);
extern int print_insn_msp430		(bfd_vma, disassemble_info *);
extern int print_insn_mt                (bfd_vma, disassemble_info *);
extern int print_insn_nds32		(bfd_vma, disassemble_info *);
extern int print_insn_nfp		(bfd_vma, disassemble_info *);
extern int print_insn_ns32k		(bfd_vma, disassemble_info *);
extern int print_insn_or1k		(bfd_vma, disassemble_info *);
extern int print_insn_pdp11		(bfd_vma, disassemble_info *);
extern int print_insn_pj		(bfd_vma, disassemble_info *);
extern int print_insn_pru		(bfd_vma, disassemble_info *);
extern int print_insn_s390		(bfd_vma, disassemble_info *);
extern int print_insn_spu		(bfd_vma, disassemble_info *);
extern int print_insn_tic30		(bfd_vma, disassemble_info *);
extern int print_insn_tic4x		(bfd_vma, disassemble_info *);
extern int print_insn_tic54x		(bfd_vma, disassemble_info *);
extern int print_insn_tic6x		(bfd_vma, disassemble_info *);
extern int print_insn_tilegx		(bfd_vma, disassemble_info *);
extern int print_insn_tilepro		(bfd_vma, disassemble_info *);
extern int print_insn_v850		(bfd_vma, disassemble_info *);
extern int print_insn_vax		(bfd_vma, disassemble_info *);
extern int print_insn_visium		(bfd_vma, disassemble_info *);
extern int print_insn_wasm32		(bfd_vma, disassemble_info *);
extern int print_insn_xgate             (bfd_vma, disassemble_info *);
extern int print_insn_xstormy16		(bfd_vma, disassemble_info *);
extern int print_insn_xtensa		(bfd_vma, disassemble_info *);
extern int print_insn_z80		(bfd_vma, disassemble_info *);
extern int print_insn_z8001		(bfd_vma, disassemble_info *);
extern int print_insn_z8002		(bfd_vma, disassemble_info *);
extern int print_insn_loongarch		(bfd_vma, disassemble_info *);

extern disassembler_ftype csky_get_disassembler (bfd *);
extern disassembler_ftype rl78_get_disassembler (bfd *);
extern disassembler_ftype riscv_get_disassembler (bfd *);

extern void disassemble_free_riscv (disassemble_info *);

extern void ATTRIBUTE_NORETURN opcodes_assert (const char *, int);

#define OPCODES_ASSERT(x) \
  do { if (!(x)) opcodes_assert (__FILE__, __LINE__); } while (0)

#endif /* DISASSEMBLE_H */
