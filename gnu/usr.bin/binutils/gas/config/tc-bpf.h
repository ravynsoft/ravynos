/* tc-bpf.h -- Header file for tc-ebpf.c.
   Copyright (C) 2019-2023 Free Software Foundation, Inc.
   Contributed by Oracle, Inc.

   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

#define TC_BPF

#define LISTING_HEADER "BPF GAS "

/* The target BFD architecture.  */
#define TARGET_ARCH bfd_arch_bpf
#define TARGET_MACH 0 /* The default.  */

#define TARGET_FORMAT                                                   \
  (target_big_endian ? "elf64-bpfbe" : "elf64-bpfle")

/* This is used to set the default value for `target_big_endian'.  */
#ifndef TARGET_BYTES_BIG_ENDIAN
#define TARGET_BYTES_BIG_ENDIAN 0
#endif

/* .-foo gets turned into PC relative relocs.  */
#define DIFF_EXPR_OK    1
#define GAS_CGEN_PCREL_R_TYPE(R_TYPE) gas_cgen_pcrel_r_type (R_TYPE)

/* Call md_pcrel_from_section(), not md_pcrel_from().  */
#define MD_PCREL_FROM_SECTION(FIXP, SEC) md_pcrel_from_section (FIXP, SEC)

/* We don't need to handle .word strangely.  */
#define WORKING_DOT_WORD

/* Values passed to md_apply_fix don't include the symbol value.  */
#define MD_APPLY_SYM_VALUE(FIX) 0

/* The Linux kernel verifier expects NOPs to be encoded in this way;
   a jump to offset 0 means jump to the next instruction.  */
#define md_single_noop_insn "ja 0"

#define TC_EQUAL_IN_INSN(c, s) 1
