/* C-SKY assembler/disassembler support.
   Copyright (C) 2004-2023 Free Software Foundation, Inc.
   Contributed by C-SKY Microsystems and Mentor Graphics.

   This file is part of GDB and GAS.

   GDB and GAS are free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 3, or (at
   your option) any later version.

   GDB and GAS are distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GDB or GAS; see the file COPYING3.  If not, write to the
   Free Software Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "dis-asm.h"

/* The following bitmasks control instruction set architecture.  */
#define CSKYV1_ISA_E1       ((uint64_t) 1 << 0)
#define CSKYV2_ISA_E1       ((uint64_t) 1 << 1)
#define CSKYV2_ISA_1E2      ((uint64_t) 1 << 2)
#define CSKYV2_ISA_2E3      ((uint64_t) 1 << 3)
#define CSKYV2_ISA_3E7      ((uint64_t) 1 << 4)
#define CSKYV2_ISA_7E10     ((uint64_t) 1 << 5)
#define CSKYV2_ISA_3E3R1    ((uint64_t) 1 << 6)
#define CSKYV2_ISA_3E3R2    ((uint64_t) 1 << 7)
#define CSKYV2_ISA_10E60    ((uint64_t) 1 << 8)
#define CSKYV2_ISA_3E3R3    ((uint64_t) 1 << 9)

#define CSKY_ISA_TRUST      ((uint64_t) 1 << 11)
#define CSKY_ISA_CACHE      ((uint64_t) 1 << 12)
#define CSKY_ISA_NVIC       ((uint64_t) 1 << 13)
#define CSKY_ISA_CP         ((uint64_t) 1 << 14)
#define CSKY_ISA_MP         ((uint64_t) 1 << 15)
#define CSKY_ISA_MP_1E2     ((uint64_t) 1 << 16)
#define CSKY_ISA_JAVA       ((uint64_t) 1 << 17)
#define CSKY_ISA_MAC        ((uint64_t) 1 << 18)
#define CSKY_ISA_MAC_DSP    ((uint64_t) 1 << 19)

/* Base ISA for csky v1 and v2.  */
#define CSKY_ISA_DSP        ((uint64_t) 1 << 20)
#define CSKY_ISA_DSP_1E2    ((uint64_t) 1 << 21)
#define CSKY_ISA_DSP_ENHANCE ((uint64_t) 1 << 22)
#define CSKY_ISA_DSPE60     ((uint64_t) 1 << 23)

/* Base float instruction (803f & 810f).  */
#define CSKY_ISA_FLOAT_E1   ((uint64_t) 1 << 25)
/* M_FLOAT support (810f).  */
#define CSKY_ISA_FLOAT_1E2  ((uint64_t) 1 << 26)
/* 803 support (803f).  */
#define CSKY_ISA_FLOAT_1E3  ((uint64_t) 1 << 27)
/* 807 support (803f & 807f).  */
#define CSKY_ISA_FLOAT_3E4  ((uint64_t) 1 << 28)
/* 860 support.  */
#define CSKY_ISA_FLOAT_7E60 ((uint64_t) 1 << 36)
/* Vector DSP support.  */
#define CSKY_ISA_VDSP       ((uint64_t) 1 << 29)
#define CSKY_ISA_VDSP_2     ((uint64_t) 1 << 30)

/* The following bitmasks control cpu architecture for CSKY.  */
#define CSKY_ABI_V1         (1 << 28)
#define CSKY_ABI_V2         (2 << 28)
#define CSKY_ARCH_MASK      0x0000001F
#define CSKY_ABI_MASK       0xF0000000

#define CSKY_ARCH_510       0x1
#define CSKY_ARCH_610       0x2
#define CSKY_ARCH_801       0xa
#define CSKY_ARCH_802       0x10
#define CSKY_ARCH_803       0x9
/* 804  use the same arch flag as 803 yet.  */
#define CSKY_ARCH_804       0x9
#define CSKY_ARCH_805       0x11
#define CSKY_ARCH_807       0x6
#define CSKY_ARCH_810       0x8
#define CSKY_ARCH_860       0xb
/* 800 is a special arch supporting all instructions for ABIV2.  */
#define CSKY_ARCH_800       0x1f

#define CSKY_ARCH_MAC       (1 << 15)
#define CSKY_ARCH_DSP       (1 << 14)
#define CSKY_ARCH_FLOAT     (1 << 13)
#define CSKY_ARCH_SIMD      (1 << 12)
#define CSKY_ARCH_CP        (1 << 11)
#define CSKY_ARCH_MP        (1 << 10)
#define CSKY_ARCH_CACHE     (1 << 9)
#define CSKY_ARCH_JAVA      (1 << 8)
#define CSKY_ARCH_APS       (1 << 7)

/* eflag's Versions.  */
#define CSKY_VERSION_V1     (1 << 24)
#define CSKY_VERSION_V2     (2 << 24)
#define CSKY_VERSION_V3     (3 << 24)

#define IS_CSKY_V1(a) \
  (((a) & CSKY_ABI_MASK) == CSKY_ABI_V1)
#define IS_CSKY_V2(a) \
  (((a) & CSKY_ABI_MASK) == CSKY_ABI_V2)
#define IS_CSKY_ARCH_V1(a) \
  (((a) & CSKY_ARCH_MASK) == CSKY_ARCH_510				\
   || ((a) & CSKY_ARCH_MASK) == CSKY_ARCH_610)
#define IS_CSKY_ARCH_V2(a)  \
  (!(IS_CSKY_ARCH_V1 (a)))

#define IS_CSKY_ARCH_510(a)	(((a) & CSKY_ARCH_MASK) == CSKY_ARCH_510)
#define IS_CSKY_ARCH_610(a)	(((a) & CSKY_ARCH_MASK) == CSKY_ARCH_610)
#define IS_CSKY_ARCH_801(a)	(((a) & CSKY_ARCH_MASK) == CSKY_ARCH_801)
#define IS_CSKY_ARCH_802(a)	(((a) & CSKY_ARCH_MASK) == CSKY_ARCH_802)
#define IS_CSKY_ARCH_803(a)	(((a) & CSKY_ARCH_MASK) == CSKY_ARCH_803)
#define IS_CSKY_ARCH_807(a)	(((a) & CSKY_ARCH_MASK) == CSKY_ARCH_807)
#define IS_CSKY_ARCH_810(a)	(((a) & CSKY_ARCH_MASK) == CSKY_ARCH_810)

#define CPU_ARCH_MASK \
  (CSKY_ARCH_JAVA | CSKY_ARCH_FLOAT | CSKY_ARCH_DSP | CSKY_ARCH_MASK)

#ifdef __cplusplus
extern "C" {
#endif
extern int print_insn_csky (bfd_vma memaddr, struct disassemble_info *info);
#ifdef __cplusplus
}
#endif
