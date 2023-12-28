/* Mach-O object file format for gas, the assembler.
   Copyright (C) 2009-2023 Free Software Foundation, Inc.

   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 3,
   or (at your option) any later version.

   GAS is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
   the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

#ifndef _OBJ_MACH_O_H
#define _OBJ_MACH_O_H

/* Tag to validate Mach-O object file format processing */
#define OBJ_MACH_O 1

#include "bfd/mach-o.h"

#include "targ-cpu.h"

#define OUTPUT_FLAVOR bfd_target_mach_o_flavour

/* We want to control how the sections are pre-defined on startup.  */
#define obj_begin() mach_o_begin ()
extern void mach_o_begin (void);

/* All our align expressions are power of two.  */
#define USE_ALIGN_PTWO 1

/* Common symbols can carry alignment information.  */
#ifndef S_SET_ALIGN
#define S_SET_ALIGN(S,V) do {\
  bfd_mach_o_asymbol *___s = (bfd_mach_o_asymbol *) symbol_get_bfdsym (S);\
  ___s->n_desc = (___s->n_desc & 0xf0ff) | (((V) & 0x0f) << 8);\
} while (0)
#endif

extern const pseudo_typeS mach_o_pseudo_table[];

#ifndef obj_pop_insert
#define obj_pop_insert() pop_insert (mach_o_pseudo_table)
#endif

#define obj_sec_sym_ok_for_reloc(SEC)	1

#define obj_read_begin_hook()	{;}
#define obj_symbol_new_hook(s)	{;}

#define EMIT_SECTION_SYMBOLS		0

struct obj_mach_o_symbol_data
{
  /* If the symbol represents a subsection, this is the size of the subsection.
     This is used to check whether a local symbol belongs to a subsection.  */
  valueT subsection_size;
};
#define OBJ_SYMFIELD_TYPE struct obj_mach_o_symbol_data

#define obj_frob_colon obj_mach_o_frob_colon
extern void obj_mach_o_frob_colon (const char *);

/* Called when a label is defined.  Mach-O uses this to create subsections.  */
#define obj_frob_label obj_mach_o_frob_label
extern void obj_mach_o_frob_label (symbolS *);

#define obj_frob_symbol(s, punt) punt = obj_mach_o_frob_symbol(s)
extern int obj_mach_o_frob_symbol (struct symbol *);

#define OBJ_PROCESS_STAB(SEG,W,S,T,O,D)	obj_mach_o_process_stab(W,S,T,O,D)
extern void obj_mach_o_process_stab (int, const char *,int, int, int);

struct obj_mach_o_frag_data
{
  /* Symbol that corresponds to the subsection.  */
  symbolS *subsection;
};

#define OBJ_FRAG_TYPE struct obj_mach_o_frag_data

#define md_pre_output_hook obj_mach_o_pre_output_hook()
extern void obj_mach_o_pre_output_hook(void);

#define md_pre_relax_hook obj_mach_o_pre_relax_hook()
extern void obj_mach_o_pre_relax_hook (void);

#define md_post_relax_hook obj_mach_o_post_relax_hook()
extern void obj_mach_o_post_relax_hook (void);

#define obj_frob_file_after_relocs obj_mach_o_frob_file_after_relocs
extern void obj_mach_o_frob_file_after_relocs (void);

#define SET_SECTION_RELOCS(sec, relocs, n) \
  obj_mach_o_reorder_section_relocs (sec, relocs, n)
extern void obj_mach_o_reorder_section_relocs (asection *, arelent **,
					       unsigned int);

/* Emit relocs for local subtracts, to cater for subsections-via-symbols.  */
#define md_allow_local_subtract(LEFT, RIGHT, SECTION) \
 obj_mach_o_allow_local_subtract (LEFT, RIGHT, SECTION)
extern int obj_mach_o_allow_local_subtract (expressionS *, expressionS *,
					    segT);

struct fix;
extern int obj_mach_o_in_different_subsection (symbolS *a, symbolS *b);
extern int obj_mach_o_force_reloc (struct fix *fix);
extern int obj_mach_o_force_reloc_sub_same (struct fix *fix, segT seg);
extern int obj_mach_o_force_reloc_sub_local (struct fix *fix, segT seg);

#endif /* _OBJ_MACH_O_H */
