/* Meta ELF support for BFD.
   Copyright (C) 2013-2023 Free Software Foundation, Inc.
   Contributed by Imagination Technologies Ltd.

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
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.  */

#ifndef _ELF_METAG_H
#define _ELF_METAG_H

#include "elf/reloc-macros.h"

/* Relocation types.  */

START_RELOC_NUMBERS (elf_metag_reloc_type)
     RELOC_NUMBER (R_METAG_HIADDR16,      0)
     RELOC_NUMBER (R_METAG_LOADDR16,      1)
     RELOC_NUMBER (R_METAG_ADDR32,        2)
     RELOC_NUMBER (R_METAG_NONE,          3)
     RELOC_NUMBER (R_METAG_RELBRANCH,     4)
     RELOC_NUMBER (R_METAG_GETSETOFF,     5)

     /* Backward compatability */
     RELOC_NUMBER (R_METAG_REG32OP1,      6)
     RELOC_NUMBER (R_METAG_REG32OP2,      7)
     RELOC_NUMBER (R_METAG_REG32OP3,      8)
     RELOC_NUMBER (R_METAG_REG16OP1,      9)
     RELOC_NUMBER (R_METAG_REG16OP2,     10)
     RELOC_NUMBER (R_METAG_REG16OP3,     11)
     RELOC_NUMBER (R_METAG_REG32OP4,     12)

     RELOC_NUMBER (R_METAG_HIOG,         13)
     RELOC_NUMBER (R_METAG_LOOG,         14)

     RELOC_NUMBER (R_METAG_REL8,         15)
     RELOC_NUMBER (R_METAG_REL16,        16)

     /* GNU */
     RELOC_NUMBER (R_METAG_GNU_VTINHERIT,30)
     RELOC_NUMBER (R_METAG_GNU_VTENTRY,  31)

     /* PIC relocations */
     RELOC_NUMBER (R_METAG_HI16_GOTOFF,  32)
     RELOC_NUMBER (R_METAG_LO16_GOTOFF,  33)
     RELOC_NUMBER (R_METAG_GETSET_GOTOFF,34)
     RELOC_NUMBER (R_METAG_GETSET_GOT,   35)
     RELOC_NUMBER (R_METAG_HI16_GOTPC,   36)
     RELOC_NUMBER (R_METAG_LO16_GOTPC,   37)
     RELOC_NUMBER (R_METAG_HI16_PLT,     38)
     RELOC_NUMBER (R_METAG_LO16_PLT,     39)
     RELOC_NUMBER (R_METAG_RELBRANCH_PLT,40)
     RELOC_NUMBER (R_METAG_GOTOFF,       41)
     RELOC_NUMBER (R_METAG_PLT,          42)
     RELOC_NUMBER (R_METAG_COPY,         43)
     RELOC_NUMBER (R_METAG_JMP_SLOT,     44)
     RELOC_NUMBER (R_METAG_RELATIVE,     45)
     RELOC_NUMBER (R_METAG_GLOB_DAT,     46)

     /* TLS relocations */
     RELOC_NUMBER (R_METAG_TLS_GD,       47)
     RELOC_NUMBER (R_METAG_TLS_LDM,      48)
     RELOC_NUMBER (R_METAG_TLS_LDO_HI16, 49)
     RELOC_NUMBER (R_METAG_TLS_LDO_LO16, 50)
     RELOC_NUMBER (R_METAG_TLS_LDO,      51)
     RELOC_NUMBER (R_METAG_TLS_IE,       52)
     RELOC_NUMBER (R_METAG_TLS_IENONPIC, 53)
     RELOC_NUMBER (R_METAG_TLS_IENONPIC_HI16,54)
     RELOC_NUMBER (R_METAG_TLS_IENONPIC_LO16,55)
     RELOC_NUMBER (R_METAG_TLS_TPOFF,    56)
     RELOC_NUMBER (R_METAG_TLS_DTPMOD,   57)
     RELOC_NUMBER (R_METAG_TLS_DTPOFF,   58)
     RELOC_NUMBER (R_METAG_TLS_LE,       59)
     RELOC_NUMBER (R_METAG_TLS_LE_HI16,  60)
     RELOC_NUMBER (R_METAG_TLS_LE_LO16,  61)

END_RELOC_NUMBERS (R_METAG_MAX)

#endif /* _ELF_METAG_H */
