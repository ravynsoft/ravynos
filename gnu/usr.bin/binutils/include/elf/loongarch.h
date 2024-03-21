/* Copyright (C) 2021-2023 Free Software Foundation, Inc.
   Contributed by Loongson Ltd.

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the license, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING3.  If not,
   see <http://www.gnu.org/licenses/>.  */

#ifndef _ELF_LOONGARCH_H
#define _ELF_LOONGARCH_H

#include "elf/reloc-macros.h"
#include "libiberty.h"

START_RELOC_NUMBERS (elf_loongarch_reloc_type)
/* Used by the dynamic linker.  */
RELOC_NUMBER (R_LARCH_NONE, 0)
RELOC_NUMBER (R_LARCH_32, 1)
RELOC_NUMBER (R_LARCH_64, 2)
RELOC_NUMBER (R_LARCH_RELATIVE, 3)
RELOC_NUMBER (R_LARCH_COPY, 4)
RELOC_NUMBER (R_LARCH_JUMP_SLOT, 5)
RELOC_NUMBER (R_LARCH_TLS_DTPMOD32, 6)
RELOC_NUMBER (R_LARCH_TLS_DTPMOD64, 7)
RELOC_NUMBER (R_LARCH_TLS_DTPREL32, 8)
RELOC_NUMBER (R_LARCH_TLS_DTPREL64, 9)
RELOC_NUMBER (R_LARCH_TLS_TPREL32, 10)
RELOC_NUMBER (R_LARCH_TLS_TPREL64, 11)
RELOC_NUMBER (R_LARCH_IRELATIVE, 12)

/* Reserved for future relocs that the dynamic linker must understand.  */

/* Used by the static linker for relocating .text.  */
RELOC_NUMBER (R_LARCH_MARK_LA, 20)
RELOC_NUMBER (R_LARCH_MARK_PCREL, 21)

RELOC_NUMBER (R_LARCH_SOP_PUSH_PCREL, 22)

RELOC_NUMBER (R_LARCH_SOP_PUSH_ABSOLUTE, 23)

RELOC_NUMBER (R_LARCH_SOP_PUSH_DUP, 24)
RELOC_NUMBER (R_LARCH_SOP_PUSH_GPREL, 25)
RELOC_NUMBER (R_LARCH_SOP_PUSH_TLS_TPREL, 26)
RELOC_NUMBER (R_LARCH_SOP_PUSH_TLS_GOT, 27)
RELOC_NUMBER (R_LARCH_SOP_PUSH_TLS_GD, 28)
RELOC_NUMBER (R_LARCH_SOP_PUSH_PLT_PCREL, 29)

RELOC_NUMBER (R_LARCH_SOP_ASSERT, 30)
RELOC_NUMBER (R_LARCH_SOP_NOT, 31)
RELOC_NUMBER (R_LARCH_SOP_SUB, 32)
RELOC_NUMBER (R_LARCH_SOP_SL, 33)
RELOC_NUMBER (R_LARCH_SOP_SR, 34)
RELOC_NUMBER (R_LARCH_SOP_ADD, 35)
RELOC_NUMBER (R_LARCH_SOP_AND, 36)
RELOC_NUMBER (R_LARCH_SOP_IF_ELSE, 37)
RELOC_NUMBER (R_LARCH_SOP_POP_32_S_10_5, 38)
RELOC_NUMBER (R_LARCH_SOP_POP_32_U_10_12, 39)
RELOC_NUMBER (R_LARCH_SOP_POP_32_S_10_12, 40)
RELOC_NUMBER (R_LARCH_SOP_POP_32_S_10_16, 41)
RELOC_NUMBER (R_LARCH_SOP_POP_32_S_10_16_S2, 42)
RELOC_NUMBER (R_LARCH_SOP_POP_32_S_5_20, 43)
RELOC_NUMBER (R_LARCH_SOP_POP_32_S_0_5_10_16_S2, 44)
RELOC_NUMBER (R_LARCH_SOP_POP_32_S_0_10_10_16_S2, 45)
RELOC_NUMBER (R_LARCH_SOP_POP_32_U, 46)

/* Used by the static linker for relocating non .text.  */
RELOC_NUMBER (R_LARCH_ADD8, 47)
RELOC_NUMBER (R_LARCH_ADD16, 48)
RELOC_NUMBER (R_LARCH_ADD24, 49)
RELOC_NUMBER (R_LARCH_ADD32, 50)
RELOC_NUMBER (R_LARCH_ADD64, 51)
RELOC_NUMBER (R_LARCH_SUB8, 52)
RELOC_NUMBER (R_LARCH_SUB16, 53)
RELOC_NUMBER (R_LARCH_SUB24, 54)
RELOC_NUMBER (R_LARCH_SUB32, 55)
RELOC_NUMBER (R_LARCH_SUB64, 56)

/* I don't know what it is.  Existing in almost all other arch.  */
RELOC_NUMBER (R_LARCH_GNU_VTINHERIT, 57)
RELOC_NUMBER (R_LARCH_GNU_VTENTRY, 58)


/* B16:
   beq/bne/blt/bge/bltu/bgeu/jirl
   %b16 (sym).  */
RELOC_NUMBER (R_LARCH_B16, 64)
/* B21:
   beqz/bnez
   %b16 (sym).  */
RELOC_NUMBER (R_LARCH_B21, 65)
/* B26:
   b/bl
   %b26 (sym) or %plt (sym).  */
RELOC_NUMBER (R_LARCH_B26, 66)

/* ABS: 32/64
   lu12i.w
   %abs_hi20 (sym).  */
RELOC_NUMBER (R_LARCH_ABS_HI20, 67)
/* ABS: 32/64
   ori
   %abs_lo12 (sym).  */
RELOC_NUMBER (R_LARCH_ABS_LO12, 68)

/* ABS: 64
   lu32i.d
   %abs64_lo20 (sym).  */
RELOC_NUMBER (R_LARCH_ABS64_LO20, 69)
/* ABS: 64
   lu52i.d
   %abs64_hi12 (sym).  */
RELOC_NUMBER (R_LARCH_ABS64_HI12, 70)

/* PCREL: 32/64
   pcalau12i
   %pc_hi20 (sym).  */
RELOC_NUMBER (R_LARCH_PCALA_HI20, 71)
/* PCREL: 32/64
   addi.w/addi.d
   %pc_lo12 (sym).  */
RELOC_NUMBER (R_LARCH_PCALA_LO12, 72)
/* PCREL: 64
   lu32i.d
   %pc64_lo20 (sym).  */
RELOC_NUMBER (R_LARCH_PCALA64_LO20, 73)
/* PCREL: 64
   lu52i.d
   %pc64_hi12 (sym).  */
RELOC_NUMBER (R_LARCH_PCALA64_HI12, 74)

/* GOT: 32/64
   pcalau12i
   %got_pc_hi20 (got).  */
RELOC_NUMBER (R_LARCH_GOT_PC_HI20, 75)
/* GOT: 32/64
   ld.w/ld.d
   %got_pc_lo12 (got).  */
RELOC_NUMBER (R_LARCH_GOT_PC_LO12, 76)
/* GOT: 32/64
   lu32i.d
   %got_pc_lo12 (got).  */
RELOC_NUMBER (R_LARCH_GOT64_PC_LO20, 77)
/* GOT64: PCREL
   lu52i.d
   %got64_pc_hi12 (got).  */
RELOC_NUMBER (R_LARCH_GOT64_PC_HI12, 78)
/* GOT32/64: ABS
   lu12i.w
   %got_hi20 (got).  */
RELOC_NUMBER (R_LARCH_GOT_HI20, 79)
/* GOT: 32/64: ABS
   ori
   %got_lo12 (got).  */
RELOC_NUMBER (R_LARCH_GOT_LO12, 80)
/* GOT64: ABS
   lu32i.d
   %got64_lo20 (got).  */
RELOC_NUMBER (R_LARCH_GOT64_LO20, 81)
/* GOT64: ABS
   lu52i.d
   %got64_hi12 (got).  */
RELOC_NUMBER (R_LARCH_GOT64_HI12, 82)

/* TLS-LE: 32/64
   lu12i.w
   %le_hi20 (sym).  */
RELOC_NUMBER (R_LARCH_TLS_LE_HI20, 83)
/* TLS-LE: 32/64
   ori
   %le_lo12 (sym).  */
RELOC_NUMBER (R_LARCH_TLS_LE_LO12, 84)
/* TLS-LE: 64
   lu32i.d
   %le64_lo20 (sym).  */
RELOC_NUMBER (R_LARCH_TLS_LE64_LO20, 85)
/* TLS-LE: 64
   lu52i.d
   %le64_hi12 (sym).  */
RELOC_NUMBER (R_LARCH_TLS_LE64_HI12, 86)

/* TLS-IE: 32/64
   pcalau12i
   %ie_pc_hi20 (sym).  */
RELOC_NUMBER (R_LARCH_TLS_IE_PC_HI20, 87)
RELOC_NUMBER (R_LARCH_TLS_IE_PC_LO12, 88)
RELOC_NUMBER (R_LARCH_TLS_IE64_PC_LO20, 89)
RELOC_NUMBER (R_LARCH_TLS_IE64_PC_HI12, 90)

/* TLS-IE: 32/64: ABS
   lu12i.w
   %ie_hi20 (sym).  */
RELOC_NUMBER (R_LARCH_TLS_IE_HI20, 91)
RELOC_NUMBER (R_LARCH_TLS_IE_LO12, 92)
RELOC_NUMBER (R_LARCH_TLS_IE64_LO20, 93)
RELOC_NUMBER (R_LARCH_TLS_IE64_HI12, 94)

/* TLS-LD: 32/64
   pcalau12i
   %ld_pc_hi20 (sym).  */
RELOC_NUMBER (R_LARCH_TLS_LD_PC_HI20, 95)
/* TLS-LD: 32/64: ABS
   lu12i.w
   %ld_hi20 (sym).  */
RELOC_NUMBER (R_LARCH_TLS_LD_HI20, 96)

/* TLS-GD: 32/64
   pcalau12i
   %gd_pc_hi20 (sym).  */
RELOC_NUMBER (R_LARCH_TLS_GD_PC_HI20, 97)
/* TLS-GD: 32/64: ABS
   lu12i.w
   %gd_hi20 (sym).  */
RELOC_NUMBER (R_LARCH_TLS_GD_HI20, 98)

/* For eh_frame and debug info.  */
RELOC_NUMBER (R_LARCH_32_PCREL, 99)

/* RELAX.  */
RELOC_NUMBER (R_LARCH_RELAX, 100)

/* relax delete.  */
RELOC_NUMBER (R_LARCH_DELETE, 101)

/* relax align.  */
RELOC_NUMBER (R_LARCH_ALIGN, 102)

/* pcaddi.  */
RELOC_NUMBER (R_LARCH_PCREL20_S2, 103)

/* cfa.  */
RELOC_NUMBER (R_LARCH_CFA, 104)

/* DW_CFA_advance_loc.  */
RELOC_NUMBER (R_LARCH_ADD6, 105)
RELOC_NUMBER (R_LARCH_SUB6, 106)

/* unsigned leb128.  */
RELOC_NUMBER (R_LARCH_ADD_ULEB128, 107)
RELOC_NUMBER (R_LARCH_SUB_ULEB128, 108)

RELOC_NUMBER (R_LARCH_64_PCREL, 109)

END_RELOC_NUMBERS (R_LARCH_count)

/* Processor specific flags for the ELF header e_flags field.  */
/* Base ABI modifier, 3bits.  */
#define EF_LOONGARCH_ABI_SOFT_FLOAT	0x1
#define EF_LOONGARCH_ABI_SINGLE_FLOAT	0x2
#define EF_LOONGARCH_ABI_DOUBLE_FLOAT	0x3
#define EF_LOONGARCH_ABI_MODIFIER_MASK	0x7

#define EF_LOONGARCH_OBJABI_V1  	0x40
#define EF_LOONGARCH_OBJABI_MASK	0xC0

#define EF_LOONGARCH_ABI_MASK \
      (EF_LOONGARCH_OBJABI_MASK | EF_LOONGARCH_ABI_MODIFIER_MASK)

#define EF_LOONGARCH_ABI_MODIFIER(abi) \
      (EF_LOONGARCH_ABI_MODIFIER_MASK & (abi))
#define EF_LOONGARCH_OBJABI(abi) \
      (EF_LOONGARCH_OBJABI_MASK & (abi))

#define EF_LOONGARCH_ABI(abi) ((abi) & EF_LOONGARCH_ABI_MASK)

#define EF_LOONGARCH_IS_SOFT_FLOAT(abi) \
  (EF_LOONGARCH_ABI_MODIFIER (abi) == EF_LOONGARCH_ABI_SOFT_FLOAT)
#define EF_LOONGARCH_IS_SINGLE_FLOAT(abi) \
  (EF_LOONGARCH_ABI_MODIFIER (abi) == EF_LOONGARCH_ABI_SINGLE_FLOAT)
#define EF_LOONGARCH_IS_DOUBLE_FLOAT(abi) \
  (EF_LOONGARCH_ABI_MODIFIER (abi) == EF_LOONGARCH_ABI_DOUBLE_FLOAT)

#define EF_LOONGARCH_IS_OBJ_V0(abi) (!EF_LOONGARCH_OBJABI (abi))
#define EF_LOONGARCH_IS_OBJ_V1(abi) \
      (EF_LOONGARCH_OBJABI (abi) == EF_LOONGARCH_OBJABI_V1)

#endif /* _ELF_LOONGARCH_H */
