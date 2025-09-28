/* Or1k ELF support for BFD.
   Copyright (C) 2001-2023 Free Software Foundation, Inc.

   This file is part of BFD, the Binary File Descriptor library.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, see <http://www.gnu.org/licenses/> */

#ifndef _ELF_OR1K_H
#define _ELF_OR1K_H

#include "elf/reloc-macros.h"

/* Relocations.  */
START_RELOC_NUMBERS (elf_or1k_reloc_type)
  RELOC_NUMBER (R_OR1K_NONE,           0)
  RELOC_NUMBER (R_OR1K_32,             1)
  RELOC_NUMBER (R_OR1K_16,             2)
  RELOC_NUMBER (R_OR1K_8,              3)
  RELOC_NUMBER (R_OR1K_LO_16_IN_INSN,  4)
  RELOC_NUMBER (R_OR1K_HI_16_IN_INSN,  5)
  RELOC_NUMBER (R_OR1K_INSN_REL_26,    6)
  RELOC_NUMBER (R_OR1K_GNU_VTENTRY,    7)
  RELOC_NUMBER (R_OR1K_GNU_VTINHERIT,  8)
  RELOC_NUMBER (R_OR1K_32_PCREL,       9)
  RELOC_NUMBER (R_OR1K_16_PCREL,      10)
  RELOC_NUMBER (R_OR1K_8_PCREL,       11)
  RELOC_NUMBER (R_OR1K_GOTPC_HI16,    12)
  RELOC_NUMBER (R_OR1K_GOTPC_LO16,    13)
  RELOC_NUMBER (R_OR1K_GOT16,         14)
  RELOC_NUMBER (R_OR1K_PLT26,         15)
  RELOC_NUMBER (R_OR1K_GOTOFF_HI16,   16)
  RELOC_NUMBER (R_OR1K_GOTOFF_LO16,   17)
  RELOC_NUMBER (R_OR1K_COPY,          18)
  RELOC_NUMBER (R_OR1K_GLOB_DAT,      19)
  RELOC_NUMBER (R_OR1K_JMP_SLOT,      20)
  RELOC_NUMBER (R_OR1K_RELATIVE,      21)
  RELOC_NUMBER (R_OR1K_TLS_GD_HI16,   22)
  RELOC_NUMBER (R_OR1K_TLS_GD_LO16,   23)
  RELOC_NUMBER (R_OR1K_TLS_LDM_HI16,  24)
  RELOC_NUMBER (R_OR1K_TLS_LDM_LO16,  25)
  RELOC_NUMBER (R_OR1K_TLS_LDO_HI16,  26)
  RELOC_NUMBER (R_OR1K_TLS_LDO_LO16,  27)
  RELOC_NUMBER (R_OR1K_TLS_IE_HI16,   28)
  RELOC_NUMBER (R_OR1K_TLS_IE_LO16,   29)
  RELOC_NUMBER (R_OR1K_TLS_LE_HI16,   30)
  RELOC_NUMBER (R_OR1K_TLS_LE_LO16,   31)
  RELOC_NUMBER (R_OR1K_TLS_TPOFF,     32)
  RELOC_NUMBER (R_OR1K_TLS_DTPOFF,    33)
  RELOC_NUMBER (R_OR1K_TLS_DTPMOD,    34)
  RELOC_NUMBER (R_OR1K_AHI16,         35)
  RELOC_NUMBER (R_OR1K_GOTOFF_AHI16,  36)
  RELOC_NUMBER (R_OR1K_TLS_IE_AHI16,  37)
  RELOC_NUMBER (R_OR1K_TLS_LE_AHI16,  38)
  RELOC_NUMBER (R_OR1K_SLO16,         39)
  RELOC_NUMBER (R_OR1K_GOTOFF_SLO16,  40)
  RELOC_NUMBER (R_OR1K_TLS_LE_SLO16,  41)
  RELOC_NUMBER (R_OR1K_PCREL_PG21,    42)
  RELOC_NUMBER (R_OR1K_GOT_PG21,      43)
  RELOC_NUMBER (R_OR1K_TLS_GD_PG21,   44)
  RELOC_NUMBER (R_OR1K_TLS_LDM_PG21,  45)
  RELOC_NUMBER (R_OR1K_TLS_IE_PG21,   46)
  RELOC_NUMBER (R_OR1K_LO13,          47)
  RELOC_NUMBER (R_OR1K_GOT_LO13,      48)
  RELOC_NUMBER (R_OR1K_TLS_GD_LO13,   49)
  RELOC_NUMBER (R_OR1K_TLS_LDM_LO13,  50)
  RELOC_NUMBER (R_OR1K_TLS_IE_LO13,   51)
  RELOC_NUMBER (R_OR1K_SLO13,         52)
  RELOC_NUMBER (R_OR1K_PLTA26,        53)
  RELOC_NUMBER (R_OR1K_GOT_AHI16,     54)
END_RELOC_NUMBERS (R_OR1K_max)

#define EF_OR1K_NODELAY (1UL << 0)

#endif /* _ELF_OR1K_H */
