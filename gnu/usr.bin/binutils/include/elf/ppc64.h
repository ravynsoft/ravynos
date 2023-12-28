/* PPC64 ELF support for BFD.
   Copyright (C) 2003-2023 Free Software Foundation, Inc.

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
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#ifndef _ELF_PPC64_H
#define _ELF_PPC64_H

#include "elf/reloc-macros.h"

/* Relocations.  */
START_RELOC_NUMBERS (elf_ppc64_reloc_type)
  RELOC_NUMBER (R_PPC64_NONE,		     0)
  RELOC_NUMBER (R_PPC64_ADDR32,		     1)
  RELOC_NUMBER (R_PPC64_ADDR24,		     2)
  RELOC_NUMBER (R_PPC64_ADDR16,		     3)
  RELOC_NUMBER (R_PPC64_ADDR16_LO,	     4)
  RELOC_NUMBER (R_PPC64_ADDR16_HI,	     5)
  RELOC_NUMBER (R_PPC64_ADDR16_HA,	     6)
  RELOC_NUMBER (R_PPC64_ADDR14,		     7)
  RELOC_NUMBER (R_PPC64_ADDR14_BRTAKEN,	     8)
  RELOC_NUMBER (R_PPC64_ADDR14_BRNTAKEN,     9)
  RELOC_NUMBER (R_PPC64_REL24,		    10)
  RELOC_NUMBER (R_PPC64_REL14,		    11)
  RELOC_NUMBER (R_PPC64_REL14_BRTAKEN,	    12)
  RELOC_NUMBER (R_PPC64_REL14_BRNTAKEN,	    13)
  RELOC_NUMBER (R_PPC64_GOT16,		    14)
  RELOC_NUMBER (R_PPC64_GOT16_LO,	    15)
  RELOC_NUMBER (R_PPC64_GOT16_HI,	    16)
  RELOC_NUMBER (R_PPC64_GOT16_HA,	    17)
  /* 18 unused.  32-bit reloc is R_PPC_PLTREL24.  */
  RELOC_NUMBER (R_PPC64_COPY,		    19)
  RELOC_NUMBER (R_PPC64_GLOB_DAT,	    20)
  RELOC_NUMBER (R_PPC64_JMP_SLOT,	    21)
  RELOC_NUMBER (R_PPC64_RELATIVE,	    22)
  /* 23 unused.  32-bit reloc is R_PPC_LOCAL24PC.  */
  RELOC_NUMBER (R_PPC64_UADDR32,	    24)
  RELOC_NUMBER (R_PPC64_UADDR16,	    25)
  RELOC_NUMBER (R_PPC64_REL32,		    26)
  RELOC_NUMBER (R_PPC64_PLT32,		    27)
  RELOC_NUMBER (R_PPC64_PLTREL32,	    28)
  RELOC_NUMBER (R_PPC64_PLT16_LO,	    29)
  RELOC_NUMBER (R_PPC64_PLT16_HI,	    30)
  RELOC_NUMBER (R_PPC64_PLT16_HA,	    31)
  /* 32 unused.  32-bit reloc is R_PPC_SDAREL16.  */
  RELOC_NUMBER (R_PPC64_SECTOFF,	    33)
  RELOC_NUMBER (R_PPC64_SECTOFF_LO,	    34)
  RELOC_NUMBER (R_PPC64_SECTOFF_HI,	    35)
  RELOC_NUMBER (R_PPC64_SECTOFF_HA,	    36)
  RELOC_NUMBER (R_PPC64_REL30,		    37)
  RELOC_NUMBER (R_PPC64_ADDR64,		    38)
  RELOC_NUMBER (R_PPC64_ADDR16_HIGHER,	    39)
  RELOC_NUMBER (R_PPC64_ADDR16_HIGHERA,	    40)
  RELOC_NUMBER (R_PPC64_ADDR16_HIGHEST,	    41)
  RELOC_NUMBER (R_PPC64_ADDR16_HIGHESTA,    42)
  RELOC_NUMBER (R_PPC64_UADDR64,	    43)
  RELOC_NUMBER (R_PPC64_REL64,		    44)
  RELOC_NUMBER (R_PPC64_PLT64,		    45)
  RELOC_NUMBER (R_PPC64_PLTREL64,	    46)
  RELOC_NUMBER (R_PPC64_TOC16,		    47)
  RELOC_NUMBER (R_PPC64_TOC16_LO,	    48)
  RELOC_NUMBER (R_PPC64_TOC16_HI,	    49)
  RELOC_NUMBER (R_PPC64_TOC16_HA,	    50)
  RELOC_NUMBER (R_PPC64_TOC,		    51)
  RELOC_NUMBER (R_PPC64_PLTGOT16,	    52)
  RELOC_NUMBER (R_PPC64_PLTGOT16_LO,	    53)
  RELOC_NUMBER (R_PPC64_PLTGOT16_HI,	    54)
  RELOC_NUMBER (R_PPC64_PLTGOT16_HA,	    55)

  /* The following relocs were added in the 64-bit PowerPC ELF ABI
     revision 1.2. */
  RELOC_NUMBER (R_PPC64_ADDR16_DS,	    56)
  RELOC_NUMBER (R_PPC64_ADDR16_LO_DS,	    57)
  RELOC_NUMBER (R_PPC64_GOT16_DS,	    58)
  RELOC_NUMBER (R_PPC64_GOT16_LO_DS,	    59)
  RELOC_NUMBER (R_PPC64_PLT16_LO_DS,	    60)
  RELOC_NUMBER (R_PPC64_SECTOFF_DS,	    61)
  RELOC_NUMBER (R_PPC64_SECTOFF_LO_DS,	    62)
  RELOC_NUMBER (R_PPC64_TOC16_DS,	    63)
  RELOC_NUMBER (R_PPC64_TOC16_LO_DS,	    64)
  RELOC_NUMBER (R_PPC64_PLTGOT16_DS,	    65)
  RELOC_NUMBER (R_PPC64_PLTGOT16_LO_DS,	    66)

  /* Relocs added to support TLS.  PowerPC64 ELF ABI revision 1.5.  */
  RELOC_NUMBER (R_PPC64_TLS,		    67)
  RELOC_NUMBER (R_PPC64_DTPMOD64,	    68)
  RELOC_NUMBER (R_PPC64_TPREL16,	    69)
  RELOC_NUMBER (R_PPC64_TPREL16_LO,	    70)
  RELOC_NUMBER (R_PPC64_TPREL16_HI,	    71)
  RELOC_NUMBER (R_PPC64_TPREL16_HA,	    72)
  RELOC_NUMBER (R_PPC64_TPREL64,	    73)
  RELOC_NUMBER (R_PPC64_DTPREL16,	    74)
  RELOC_NUMBER (R_PPC64_DTPREL16_LO,	    75)
  RELOC_NUMBER (R_PPC64_DTPREL16_HI,	    76)
  RELOC_NUMBER (R_PPC64_DTPREL16_HA,	    77)
  RELOC_NUMBER (R_PPC64_DTPREL64,	    78)
  RELOC_NUMBER (R_PPC64_GOT_TLSGD16,	    79)
  RELOC_NUMBER (R_PPC64_GOT_TLSGD16_LO,	    80)
  RELOC_NUMBER (R_PPC64_GOT_TLSGD16_HI,	    81)
  RELOC_NUMBER (R_PPC64_GOT_TLSGD16_HA,	    82)
  RELOC_NUMBER (R_PPC64_GOT_TLSLD16,	    83)
  RELOC_NUMBER (R_PPC64_GOT_TLSLD16_LO,	    84)
  RELOC_NUMBER (R_PPC64_GOT_TLSLD16_HI,	    85)
  RELOC_NUMBER (R_PPC64_GOT_TLSLD16_HA,	    86)
  RELOC_NUMBER (R_PPC64_GOT_TPREL16_DS,	    87)
  RELOC_NUMBER (R_PPC64_GOT_TPREL16_LO_DS,  88)
  RELOC_NUMBER (R_PPC64_GOT_TPREL16_HI,	    89)
  RELOC_NUMBER (R_PPC64_GOT_TPREL16_HA,	    90)
  RELOC_NUMBER (R_PPC64_GOT_DTPREL16_DS,    91)
  RELOC_NUMBER (R_PPC64_GOT_DTPREL16_LO_DS, 92)
  RELOC_NUMBER (R_PPC64_GOT_DTPREL16_HI,    93)
  RELOC_NUMBER (R_PPC64_GOT_DTPREL16_HA,    94)
  RELOC_NUMBER (R_PPC64_TPREL16_DS,	    95)
  RELOC_NUMBER (R_PPC64_TPREL16_LO_DS,	    96)
  RELOC_NUMBER (R_PPC64_TPREL16_HIGHER,	    97)
  RELOC_NUMBER (R_PPC64_TPREL16_HIGHERA,    98)
  RELOC_NUMBER (R_PPC64_TPREL16_HIGHEST,    99)
  RELOC_NUMBER (R_PPC64_TPREL16_HIGHESTA,  100)
  RELOC_NUMBER (R_PPC64_DTPREL16_DS,	   101)
  RELOC_NUMBER (R_PPC64_DTPREL16_LO_DS,	   102)
  RELOC_NUMBER (R_PPC64_DTPREL16_HIGHER,   103)
  RELOC_NUMBER (R_PPC64_DTPREL16_HIGHERA,  104)
  RELOC_NUMBER (R_PPC64_DTPREL16_HIGHEST,  105)
  RELOC_NUMBER (R_PPC64_DTPREL16_HIGHESTA, 106)
  RELOC_NUMBER (R_PPC64_TLSGD,		   107)
  RELOC_NUMBER (R_PPC64_TLSLD,		   108)
  RELOC_NUMBER (R_PPC64_TOCSAVE,	   109)

/* Added when HA and HI relocs were changed to report overflows.  */
  RELOC_NUMBER (R_PPC64_ADDR16_HIGH,	   110)
  RELOC_NUMBER (R_PPC64_ADDR16_HIGHA,	   111)
  RELOC_NUMBER (R_PPC64_TPREL16_HIGH,	   112)
  RELOC_NUMBER (R_PPC64_TPREL16_HIGHA,	   113)
  RELOC_NUMBER (R_PPC64_DTPREL16_HIGH,	   114)
  RELOC_NUMBER (R_PPC64_DTPREL16_HIGHA,	   115)

/* Added for ELFv2.  */
  RELOC_NUMBER (R_PPC64_REL24_NOTOC,	   116)
  RELOC_NUMBER (R_PPC64_ADDR64_LOCAL,	   117)
  RELOC_NUMBER (R_PPC64_ENTRY,		   118)

/* Marker reloc for inline plt call insns.  */
  RELOC_NUMBER (R_PPC64_PLTSEQ,		   119)
  RELOC_NUMBER (R_PPC64_PLTCALL,	   120)

/* Power10 support.  */
  RELOC_NUMBER (R_PPC64_PLTSEQ_NOTOC,	   121)
  RELOC_NUMBER (R_PPC64_PLTCALL_NOTOC,	   122)
  RELOC_NUMBER (R_PPC64_PCREL_OPT,	   123)
  RELOC_NUMBER (R_PPC64_REL24_P9NOTOC,	   124)

  RELOC_NUMBER (R_PPC64_D34,		   128)
  RELOC_NUMBER (R_PPC64_D34_LO,		   129)
  RELOC_NUMBER (R_PPC64_D34_HI30,	   130)
  RELOC_NUMBER (R_PPC64_D34_HA30,	   131)
  RELOC_NUMBER (R_PPC64_PCREL34,	   132)
  RELOC_NUMBER (R_PPC64_GOT_PCREL34,	   133)
  RELOC_NUMBER (R_PPC64_PLT_PCREL34,	   134)
  RELOC_NUMBER (R_PPC64_PLT_PCREL34_NOTOC, 135)
  RELOC_NUMBER (R_PPC64_ADDR16_HIGHER34,   136)
  RELOC_NUMBER (R_PPC64_ADDR16_HIGHERA34,  137)
  RELOC_NUMBER (R_PPC64_ADDR16_HIGHEST34,  138)
  RELOC_NUMBER (R_PPC64_ADDR16_HIGHESTA34, 139)
  RELOC_NUMBER (R_PPC64_REL16_HIGHER34,    140)
  RELOC_NUMBER (R_PPC64_REL16_HIGHERA34,   141)
  RELOC_NUMBER (R_PPC64_REL16_HIGHEST34,   142)
  RELOC_NUMBER (R_PPC64_REL16_HIGHESTA34,  143)
  RELOC_NUMBER (R_PPC64_D28,		   144)
  RELOC_NUMBER (R_PPC64_PCREL28,	   145)
  RELOC_NUMBER (R_PPC64_TPREL34,	   146)
  RELOC_NUMBER (R_PPC64_DTPREL34,	   147)
  RELOC_NUMBER (R_PPC64_GOT_TLSGD_PCREL34, 148)
  RELOC_NUMBER (R_PPC64_GOT_TLSLD_PCREL34, 149)
  RELOC_NUMBER (R_PPC64_GOT_TPREL_PCREL34, 150)
  RELOC_NUMBER (R_PPC64_GOT_DTPREL_PCREL34, 151)

#ifndef RELOC_MACROS_GEN_FUNC
/* Relocation only used internally by gas or ld.  If you need to use
   these reloc numbers, you can change them to some other unused value
   without affecting the ABI.  They will never appear in object files.  */
  RELOC_NUMBER (R_PPC64_LO_DS_OPT,	   200)
  RELOC_NUMBER (R_PPC64_16DX_HA,	   201)
#endif

  RELOC_NUMBER (R_PPC64_REL16_HIGH,	   240)
  RELOC_NUMBER (R_PPC64_REL16_HIGHA,	   241)
  RELOC_NUMBER (R_PPC64_REL16_HIGHER,	   242)
  RELOC_NUMBER (R_PPC64_REL16_HIGHERA,	   243)
  RELOC_NUMBER (R_PPC64_REL16_HIGHEST,	   244)
  RELOC_NUMBER (R_PPC64_REL16_HIGHESTA,	   245)

/* Power9 split rel16 for addpcis.  */
  RELOC_NUMBER (R_PPC64_REL16DX_HA,	   246)

/* Support STT_GNU_IFUNC plt calls.  */
  RELOC_NUMBER (R_PPC64_JMP_IREL,	   247)
  RELOC_NUMBER (R_PPC64_IRELATIVE,	   248)

/* These are GNU extensions used in PIC code sequences.  */
  RELOC_NUMBER (R_PPC64_REL16,		   249)
  RELOC_NUMBER (R_PPC64_REL16_LO,	   250)
  RELOC_NUMBER (R_PPC64_REL16_HI,	   251)
  RELOC_NUMBER (R_PPC64_REL16_HA,	   252)

  /* These are GNU extensions to enable C++ vtable garbage collection.  */
  RELOC_NUMBER (R_PPC64_GNU_VTINHERIT,	   253)
  RELOC_NUMBER (R_PPC64_GNU_VTENTRY,	   254)

END_RELOC_NUMBERS (R_PPC64_max)

#define IS_PPC64_TLS_RELOC(R)						\
  (((R) >= R_PPC64_TLS && (R) <= R_PPC64_DTPREL16_HIGHESTA)		\
   || ((R) >= R_PPC64_TPREL16_HIGH && (R) <= R_PPC64_DTPREL16_HIGHA)	\
   || ((R) >= R_PPC64_TPREL34 && (R) <= R_PPC64_GOT_DTPREL_PCREL34))

/* e_flags bits specifying ABI.
   1 for original function descriptor using ABI,
   2 for revised ABI without function descriptors,
   0 for unspecified or not using any features affected by the differences.  */
#define EF_PPC64_ABI	3

/* The ELFv2 ABI uses three bits in the symbol st_other field of a
   function definition to specify the number of bytes between a
   function's global entry point and local entry point.
   Values of two to six specify powers of two from four to sixty four
   bytes.  For such functions:
   The global entry point is used when it is necessary to set up the
   toc pointer (r2) for the function.  Callers must enter the global
   entry point with r12 set to the global entry point address.  On
   return from the function r2 will contain the toc pointer for the
   function.
   The local entry point is used when r2 is known to already be valid
   for the function.  There is no requirement on r12 when using the
   local entry point, and on return r2 will contain the same value as
   at entry.
   A value of zero in these bits means that the function has a single
   entry point with no requirement on r12 or r2, and that on return r2
   will contain the same value as at entry.
   A value of one means that the function has a single entry point
   with no requirement on r12 or r2, and that r2 is *not* preserved.
   A value of seven is reserved.  */
#define STO_PPC64_LOCAL_BIT		5
#define STO_PPC64_LOCAL_MASK		(7 << STO_PPC64_LOCAL_BIT)

/* 3 bit other field to bytes.  */
static inline unsigned int
ppc64_decode_local_entry(unsigned int other)
{
  return ((1 << other) >> 2) << 2;
}

/* bytes to field value.  */
static inline unsigned int
ppc64_encode_local_entry(unsigned int val)
{
  return (val >= 4 * 4
	  ? (val >= 8 * 4
	     ? (val >= 16 * 4 ? 6 : 5)
	     : 4)
	  : (val >= 2 * 4
	     ? 3
	     : (val >= 1 * 4 ? 2 : 0)));
}

/* st_other to number of bytes.  */
#define PPC64_LOCAL_ENTRY_OFFSET(other)				\
  ppc64_decode_local_entry (((other) & STO_PPC64_LOCAL_MASK)	\
			    >> STO_PPC64_LOCAL_BIT)
/* number of bytes to st_other.  */
#define PPC64_SET_LOCAL_ENTRY_OFFSET(val)		\
  ppc64_encode_local_entry (val) << STO_PPC64_LOCAL_BIT

/* Specify the start of the .glink section.  */
#define DT_PPC64_GLINK		DT_LOPROC

/* Specify the start and size of the .opd section.  */
#define DT_PPC64_OPD		(DT_LOPROC + 1)
#define DT_PPC64_OPDSZ		(DT_LOPROC + 2)

/* Specify whether various optimisations are possible.  */
#define DT_PPC64_OPT		(DT_LOPROC + 3)
#define PPC64_OPT_TLS		1
#define PPC64_OPT_MULTI_TOC	2
#define PPC64_OPT_LOCALENTRY	4

#endif /* _ELF_PPC64_H */
