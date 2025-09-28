/* ELF object file format.
   Copyright (C) 1992-2023 Free Software Foundation, Inc.

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

/* HP PA-RISC support was contributed by the Center for Software Science
   at the University of Utah.  */

#ifndef _OBJ_ELF_H
#define _OBJ_ELF_H

#define OBJ_ELF 1

/* Note that all macros in this file should be wrapped in #ifndef, for
   sake of obj-multi.h which includes this file.  */

#ifndef OUTPUT_FLAVOR
#define OUTPUT_FLAVOR bfd_target_elf_flavour
#endif

#include "bfd/elf-bfd.h"

#include "targ-cpu.h"

#ifdef TC_ALPHA
#define ECOFF_DEBUGGING (alpha_flag_mdebug > 0)
extern int alpha_flag_mdebug;
#endif

/* For now, always set ECOFF_DEBUGGING for a MIPS target.  */
#ifdef TC_MIPS
#define ECOFF_DEBUGGING mips_flag_mdebug
extern int mips_flag_mdebug;
#endif /* TC_MIPS */

#ifdef OBJ_MAYBE_ECOFF
#ifndef ECOFF_DEBUGGING
#define ECOFF_DEBUGGING 1
#endif
#endif

enum elf_visibility
{
  visibility_unchanged = 0,
  visibility_local,
  visibility_hidden,
  visibility_remove
};

struct elf_versioned_name_list
{
  char *name;
  struct elf_versioned_name_list *next;
};

/* Additional information we keep for each symbol.  */
struct elf_obj_sy
{
  /* Whether the symbol has been marked as local.  */
  unsigned int local : 1;

  /* Whether the symbol has been marked for rename with @@@.  */
  unsigned int rename : 1;

  /* Whether the symbol has a bad version name.  */
  unsigned int bad_version : 1;

  /* Whether visibility of the symbol should be changed.  */
  ENUM_BITFIELD (elf_visibility) visibility : 2;

  /* Use this to keep track of .size expressions that involve
     differences that we can't compute yet.  */
  expressionS *size;

  /* The list of names specified by the .symver directive.  */
  struct elf_versioned_name_list *versioned_name;

#ifdef ECOFF_DEBUGGING
  /* If we are generating ECOFF debugging information, we need some
     additional fields for each symbol.  */
  struct efdr *ecoff_file;
  struct localsym *ecoff_symbol;
  valueT ecoff_extern_size;
#endif
};

/* Match section group name, the sh_info field and the section_id
   field.  */
struct elf_section_match
{
  const char *   group_name;
  const char *   linked_to_symbol_name;
  unsigned int   section_id;
  unsigned int   sh_info;		/* ELF section information.  */
  bfd_vma        sh_flags;		/* ELF section flags.  */
  flagword       flags;
};

#define OBJ_SYMFIELD_TYPE struct elf_obj_sy

#ifndef obj_begin
#define obj_begin() elf_begin ()
#endif
extern void elf_begin (void);

#ifndef obj_end
#define obj_end() elf_end ()
#endif
extern void elf_end (void);

#ifndef LOCAL_LABEL_PREFIX
#define LOCAL_LABEL_PREFIX '.'
#endif

/* should be conditional on address size! */
#define elf_symbol(asymbol) ((elf_symbol_type *) (&(asymbol)->the_bfd))

#ifndef S_GET_SIZE
#define S_GET_SIZE(S) \
  (elf_symbol (symbol_get_bfdsym (S))->internal_elf_sym.st_size)
#endif
#ifndef S_SET_SIZE
#define S_SET_SIZE(S,V) \
  (elf_symbol (symbol_get_bfdsym (S))->internal_elf_sym.st_size = (V))
#endif

#ifndef S_GET_ALIGN
#define S_GET_ALIGN(S) \
  (elf_symbol (symbol_get_bfdsym (S))->internal_elf_sym.st_value)
#endif
#ifndef S_SET_ALIGN
#define S_SET_ALIGN(S,V) \
  (elf_symbol (symbol_get_bfdsym (S))->internal_elf_sym.st_value = (V))
#endif

int elf_s_get_other (symbolS *);
#ifndef S_GET_OTHER
#define S_GET_OTHER(S)	(elf_s_get_other (S))
#endif
#ifndef S_SET_OTHER
#define S_SET_OTHER(S,V) \
  (elf_symbol (symbol_get_bfdsym (S))->internal_elf_sym.st_other = (V))
#endif

extern asection *gdb_section;

#ifndef obj_frob_file
#define obj_frob_file  elf_frob_file
#endif
extern void elf_frob_file (void);

#ifndef obj_frob_file_before_adjust
#define obj_frob_file_before_adjust  elf_frob_file_before_adjust
#endif
extern void elf_frob_file_before_adjust (void);

#ifndef obj_frob_file_after_relocs
#define obj_frob_file_after_relocs  elf_frob_file_after_relocs
#endif
extern void elf_frob_file_after_relocs (void);

/* If the target doesn't have special processing for labels, take care of
   dwarf2 output at the object file level.  */
#ifndef tc_frob_label
#include "dwarf2dbg.h"
#define obj_frob_label  dwarf2_emit_label
#endif

#ifndef obj_app_file
#define obj_app_file elf_file_symbol
#endif
extern void elf_file_symbol (const char *);

extern void obj_elf_section_change_hook (void);

extern void obj_elf_section (int);
extern const char * obj_elf_section_name (void);
extern void obj_elf_previous (int);
extern void obj_elf_version (int);
extern void obj_elf_common (int);
extern void obj_elf_bss (int);
extern void obj_elf_data (int);
extern void obj_elf_text (int);
extern void obj_elf_change_section
  (const char *, unsigned int, bfd_vma, int, struct elf_section_match *,
   int, int);
extern void obj_elf_vtable_inherit (int);
extern void obj_elf_vtable_entry (int);
extern struct fix * obj_elf_get_vtable_inherit (void);
extern struct fix * obj_elf_get_vtable_entry (void);
extern bool obj_elf_seen_attribute
  (int, unsigned int);
extern int obj_elf_vendor_attribute (int);

/* BFD wants to write the udata field, which is a no-no for the
   predefined section symbols in bfd/section.c.  They are read-only.  */
#ifndef obj_sec_sym_ok_for_reloc
#define obj_sec_sym_ok_for_reloc(SEC)	((SEC)->owner != 0)
#endif

void elf_obj_read_begin_hook (void);
#ifndef obj_read_begin_hook
#define obj_read_begin_hook	elf_obj_read_begin_hook
#endif

void elf_obj_symbol_new_hook (symbolS *);
#ifndef obj_symbol_new_hook
#define obj_symbol_new_hook	elf_obj_symbol_new_hook
#endif

void elf_obj_symbol_clone_hook (symbolS *, symbolS *);
#ifndef obj_symbol_clone_hook
#define obj_symbol_clone_hook	elf_obj_symbol_clone_hook
#endif

void elf_copy_symbol_attributes (symbolS *, symbolS *);
#ifndef OBJ_COPY_SYMBOL_ATTRIBUTES
#define OBJ_COPY_SYMBOL_ATTRIBUTES(DEST, SRC) \
  (elf_copy_symbol_attributes (DEST, SRC))
#endif

void elf_adjust_symtab (void);
#ifndef obj_adjust_symtab
#define obj_adjust_symtab	elf_adjust_symtab
#endif

#ifndef SEPARATE_STAB_SECTIONS
/* Avoid ifndef each separate macro setting by wrapping the whole of the
   stab group on the assumption that whoever sets SEPARATE_STAB_SECTIONS
   caters to ECOFF_DEBUGGING and the right setting of INIT_STAB_SECTIONS
   and OBJ_PROCESS_STAB too, without needing the tweaks below.  */

/* Stabs go in a separate section.  */
#define SEPARATE_STAB_SECTIONS 1

/* We need 12 bytes at the start of the section to hold some initial
   information.  */
extern void obj_elf_init_stab_section (segT);
#define INIT_STAB_SECTION(seg) obj_elf_init_stab_section (seg)

#ifdef ECOFF_DEBUGGING
/* We smuggle stabs in ECOFF rather than using a separate section.
   The Irix linker can not handle a separate stabs section.  */

#undef  SEPARATE_STAB_SECTIONS
#define SEPARATE_STAB_SECTIONS (!ECOFF_DEBUGGING)

#undef  INIT_STAB_SECTION
#define INIT_STAB_SECTION(seg) \
  ((void) (ECOFF_DEBUGGING ? 0 : (obj_elf_init_stab_section (seg), 0)))

#undef OBJ_PROCESS_STAB
#define OBJ_PROCESS_STAB(seg, what, string, type, other, desc)		\
  if (ECOFF_DEBUGGING)							\
    ecoff_stab ((seg), (what), (string), (type), (other), (desc))
#endif /* ECOFF_DEBUGGING */

#endif /* SEPARATE_STAB_SECTIONS not defined.  */

extern void elf_frob_symbol (symbolS *, int *);
#ifndef obj_frob_symbol
#define obj_frob_symbol(symp, punt) elf_frob_symbol (symp, &punt)
#endif

extern void elf_fixup_removed_symbol (symbolS **);
#ifndef obj_fixup_removed_symbol
#define obj_fixup_removed_symbol(sympp) elf_fixup_removed_symbol (sympp)
#endif

extern void elf_pop_insert (void);
#ifndef obj_pop_insert
#define obj_pop_insert()	elf_pop_insert()
#endif

#ifndef OBJ_MAYBE_ELF
/* If OBJ_MAYBE_ELF then obj-multi.h will define obj_ecoff_set_ext.  */
#define obj_ecoff_set_ext elf_ecoff_set_ext
struct ecoff_extr;
extern void elf_ecoff_set_ext (symbolS *, struct ecoff_extr *);
#endif
extern asection *elf_com_section_ptr;
extern symbolS * elf_common_parse (int ignore ATTRIBUTE_UNUSED, symbolS *symbolP,
				   addressT size);

#endif /* _OBJ_ELF_H */
