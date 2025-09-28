/* tc-d10v.h -- Header file for tc-d10v.c.
   Copyright (C) 1996-2023 Free Software Foundation, Inc.
   Written by Martin Hunt, Cygnus Support.

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

#define TC_D10V

struct fix;

#define TARGET_BYTES_BIG_ENDIAN 1

/* The target BFD architecture.  */
#define TARGET_ARCH bfd_arch_d10v

#define TARGET_FORMAT "elf32-d10v"

/* Call md_pcrel_from_section, not md_pcrel_from.  */
#define MD_PCREL_FROM_SECTION(FIX, SEC) md_pcrel_from_section (FIX, SEC)

/* Permit temporary numeric labels.  */
#define LOCAL_LABELS_FB 1

/* .-foo gets turned into PC relative relocs.  */
#define DIFF_EXPR_OK

/* We don't need to handle .word strangely.  */
#define WORKING_DOT_WORD

#define md_number_to_chars	     number_to_chars_bigendian

int d10v_cleanup (void);
void d10v_frob_label (symbolS *);
#define md_cleanup()		     d10v_cleanup ()
#define md_do_align(a,b,c,d,e)	     d10v_cleanup ()
#define tc_frob_label(sym)	     d10v_frob_label (sym)

#define tc_fix_adjustable(FIX) d10v_fix_adjustable(FIX)
bool d10v_fix_adjustable (struct fix *);

/* Values passed to md_apply_fix don't include the symbol value.  */
#define MD_APPLY_SYM_VALUE(FIX) 0

/* No shared lib support, so we don't need to ensure externally
   visible symbols can be overridden.  */
#define EXTERN_FORCE_RELOC 0

#define md_flush_pending_output  d10v_cleanup
