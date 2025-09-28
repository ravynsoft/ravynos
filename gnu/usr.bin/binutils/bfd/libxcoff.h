/* BFD XCOFF object file private structure.
   Copyright (C) 2001-2023 Free Software Foundation, Inc.
   Written by Tom Rix, Redhat.

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

#ifndef LIBXCOFF_H
#define LIBXCOFF_H

/* This is the backend information kept for XCOFF files.  This
   structure is constant for a particular backend.  The first element
   is the COFF backend data structure, so that XCOFF targets can use
   the generic COFF code.  */

struct xcoff_backend_data_rec
{
  /* COFF backend information.  */
  bfd_coff_backend_data coff;

  /* Magic number.  */
  unsigned short _xcoff_magic_number;

  /* Architecture and machine for coff_set_arch_mach_hook.  */
  enum bfd_architecture _xcoff_architecture;
  long _xcoff_machine;

  /* Function pointers to xcoff specific swap routines.  */
  void (* _xcoff_swap_ldhdr_in) (bfd *, const void *, struct internal_ldhdr *);
  void (* _xcoff_swap_ldhdr_out)(bfd *, const struct internal_ldhdr *, void *);
  void (* _xcoff_swap_ldsym_in) (bfd *, const void *, struct internal_ldsym *);
  void (* _xcoff_swap_ldsym_out)(bfd *, const struct internal_ldsym *, void *);
  void (* _xcoff_swap_ldrel_in) (bfd *, const void *, struct internal_ldrel *);
  void (* _xcoff_swap_ldrel_out)(bfd *, const struct internal_ldrel *, void *);

  /* Size of the external struct.  */
  unsigned int _xcoff_ldhdrsz;
  unsigned int _xcoff_ldsymsz;
  unsigned int _xcoff_ldrelsz;

  /* Size an entry in a descriptor section.  */
  unsigned int _xcoff_function_descriptor_size;

  /* Size of the small aout file header.  */
  unsigned int _xcoff_small_aout_header_size;

  /* Loader version
     1 : XCOFF32
     2 : XCOFF64.  */
  unsigned long _xcoff_ldhdr_version;

  bool (* _xcoff_put_symbol_name)
    (struct bfd_link_info *, struct bfd_strtab_hash *,
     struct internal_syment *, const char *);

  bool (* _xcoff_put_ldsymbol_name)
    (bfd *, struct xcoff_loader_info *, struct internal_ldsym *,
     const char *);

  reloc_howto_type *_xcoff_dynamic_reloc;

  asection * (* _xcoff_create_csect_from_smclas)
    (bfd *, union internal_auxent *, const char *);

  /* Line number and relocation overflow.
     XCOFF32 overflows to another section when the line number or the
     relocation count exceeds 0xffff.  XCOFF64 does not overflow.  */
  bool (*_xcoff_is_lineno_count_overflow) (bfd *, bfd_vma);
  bool (*_xcoff_is_reloc_count_overflow)  (bfd *, bfd_vma);

  /* Loader section symbol and relocation table offset
     XCOFF32 is after the .loader header
     XCOFF64 is offset in .loader header.  */
  bfd_vma (*_xcoff_loader_symbol_offset) (bfd *, struct internal_ldhdr *);
  bfd_vma (*_xcoff_loader_reloc_offset)  (bfd *, struct internal_ldhdr *);

  /* Global linkage.  The first word of global linkage code must be be
     modified by filling in the correct TOC offset.  */
  const unsigned long *_xcoff_glink_code;

  /* Size of the global link code in bytes of the xcoff_glink_code table.  */
  unsigned long _xcoff_glink_size;

  /* rtinit.  */
  unsigned int _xcoff_rtinit_size;
  bool (*_xcoff_generate_rtinit)
    (bfd *, const char *, const char *, bool);

  /* Stubs code generation.
     The code part is an array which might need to be modified by
     some relocations.
     The size is in bytes.  */
  const unsigned long *_xcoff_stub_indirect_call_code;
  unsigned long _xcoff_stub_indirect_call_size;
  const unsigned long *_xcoff_stub_shared_call_code;
  unsigned long _xcoff_stub_shared_call_size;
};

/* Look up an entry in an XCOFF link hash table.  */
#define xcoff_link_hash_lookup(table, string, create, copy, follow) \
  ((struct xcoff_link_hash_entry *) \
   bfd_link_hash_lookup (&(table)->root, (string), (create), (copy),\
			 (follow)))

/* Traverse an XCOFF link hash table.  */
#define xcoff_link_hash_traverse(table, func, info)			\
  (bfd_link_hash_traverse						\
   (&(table)->root,							\
    (bool (*) (struct bfd_link_hash_entry *, void *)) (func),		\
    (info)))

/* Get the XCOFF link hash table from the info structure.  This is
   just a cast.  */
#define xcoff_hash_table(p) ((struct xcoff_link_hash_table *) ((p)->hash))


#define xcoff_backend(abfd) \
  ((struct xcoff_backend_data_rec *) (abfd)->xvec->backend_data)

#define bfd_xcoff_magic_number(a) ((xcoff_backend (a)->_xcoff_magic_number))
#define bfd_xcoff_architecture(a) ((xcoff_backend (a)->_xcoff_architecture))
#define bfd_xcoff_machine(a)      ((xcoff_backend (a)->_xcoff_machine))

#define bfd_xcoff_swap_ldhdr_in(a, b, c) \
  ((xcoff_backend (a)->_xcoff_swap_ldhdr_in) ((a), (b), (c)))

#define bfd_xcoff_swap_ldhdr_out(a, b, c) \
  ((xcoff_backend (a)->_xcoff_swap_ldhdr_out) ((a), (b), (c)))

#define bfd_xcoff_swap_ldsym_in(a, b, c) \
  ((xcoff_backend (a)->_xcoff_swap_ldsym_in) ((a), (b), (c)))

#define bfd_xcoff_swap_ldsym_out(a, b, c) \
  ((xcoff_backend (a)->_xcoff_swap_ldsym_out) ((a), (b), (c)))

#define bfd_xcoff_swap_ldrel_in(a, b, c) \
  ((xcoff_backend (a)->_xcoff_swap_ldrel_in) ((a), (b), (c)))

#define bfd_xcoff_swap_ldrel_out(a, b, c) \
  ((xcoff_backend (a)->_xcoff_swap_ldrel_out) ((a), (b), (c)))

#define bfd_xcoff_ldhdrsz(a) ((xcoff_backend (a)->_xcoff_ldhdrsz))
#define bfd_xcoff_ldsymsz(a) ((xcoff_backend (a)->_xcoff_ldsymsz))
#define bfd_xcoff_ldrelsz(a) ((xcoff_backend (a)->_xcoff_ldrelsz))
#define bfd_xcoff_function_descriptor_size(a) \
  ((xcoff_backend (a)->_xcoff_function_descriptor_size))
#define bfd_xcoff_small_aout_header_size(a) \
  ((xcoff_backend (a)->_xcoff_small_aout_header_size))

#define bfd_xcoff_ldhdr_version(a) ((xcoff_backend (a)->_xcoff_ldhdr_version))

#define bfd_xcoff_put_symbol_name(a, b, c, d, e) \
  ((xcoff_backend (a)->_xcoff_put_symbol_name) ((b), (c), (d), (e)))

#define bfd_xcoff_put_ldsymbol_name(a, b, c, d) \
  ((xcoff_backend (a)->_xcoff_put_ldsymbol_name) ((a), (b), (c), (d)))

/* Get the XCOFF hash table entries for a BFD.  */
#define obj_xcoff_sym_hashes(bfd) \
  ((struct xcoff_link_hash_entry **) obj_coff_sym_hashes (bfd))

#define bfd_xcoff_dynamic_reloc_howto(a) \
   ((xcoff_backend (a)->_xcoff_dynamic_reloc))

#define bfd_xcoff_create_csect_from_smclas(a, b, c) \
   ((xcoff_backend (a)->_xcoff_create_csect_from_smclas((a), (b), (c))))

#define bfd_xcoff_is_lineno_count_overflow(a, b) \
   ((xcoff_backend (a)->_xcoff_is_lineno_count_overflow((a), (b))))

#define bfd_xcoff_is_reloc_count_overflow(a, b) \
   ((xcoff_backend (a)->_xcoff_is_reloc_count_overflow((a), (b))))

#define bfd_xcoff_loader_symbol_offset(a, b) \
 ((xcoff_backend (a)->_xcoff_loader_symbol_offset((a), (b))))

#define bfd_xcoff_loader_reloc_offset(a, b) \
 ((xcoff_backend (a)->_xcoff_loader_reloc_offset((a), (b))))

#define bfd_xcoff_glink_code(a, b)   ((xcoff_backend (a)->_xcoff_glink_code[(b)]))
#define bfd_xcoff_glink_code_size(a) ((xcoff_backend (a)->_xcoff_glink_size))

#define bfd_xcoff_stub_indirect_call_code(a, b)   ((xcoff_backend (a)->_xcoff_stub_indirect_call_code[(b)]))
#define bfd_xcoff_stub_indirect_call_size(a) ((xcoff_backend (a)->_xcoff_stub_indirect_call_size))
#define bfd_xcoff_stub_shared_call_code(a, b)   ((xcoff_backend (a)->_xcoff_stub_shared_call_code[(b)]))
#define bfd_xcoff_stub_shared_call_size(a) ((xcoff_backend (a)->_xcoff_stub_shared_call_size))

/* Check for the magic number U803XTOCMAGIC or U64_TOCMAGIC for 64 bit
   targets.  */
#define bfd_xcoff_is_xcoff64(a) \
  (   (0x01EF == (bfd_xcoff_magic_number (a))) \
   || (0x01F7 == (bfd_xcoff_magic_number (a))))

/* Check for the magic number U802TOMAGIC for 32 bit targets.  */
#define bfd_xcoff_is_xcoff32(a) (0x01DF == (bfd_xcoff_magic_number (a)))

#define bfd_xcoff_rtinit_size(a)	      ((xcoff_backend (a)->_xcoff_rtinit_size))
#define bfd_xcoff_generate_rtinit(a, b, c, d) ((xcoff_backend (a)->_xcoff_generate_rtinit ((a), (b), (c), (d))))

/* Accessor macros for tdata.  */
#define bfd_xcoff_text_align_power(a) ((xcoff_data (a)->text_align_power))
#define bfd_xcoff_data_align_power(a) ((xcoff_data (a)->data_align_power))

/* xcoff*_ppc_relocate_section macros  */
#define XCOFF_MAX_CALCULATE_RELOCATION (0x32)
#define XCOFF_MAX_COMPLAIN_OVERFLOW (4)
/* N_ONES produces N one bits, without overflowing machine arithmetic.  */
#ifdef N_ONES
#undef N_ONES
#endif
#define N_ONES(n) (((((bfd_vma) 1 << ((n) - 1)) - 1) << 1) | 1)

typedef bool xcoff_reloc_function (bfd *, asection *, bfd *,
				   struct internal_reloc *,
				   struct internal_syment *,
				   struct reloc_howto_struct *,
				   bfd_vma, bfd_vma,
				   bfd_vma *, bfd_byte *,
				   struct bfd_link_info *);

typedef bool xcoff_complain_function (bfd *, bfd_vma, bfd_vma,
					     struct reloc_howto_struct *);

extern xcoff_reloc_function *const xcoff_calculate_relocation[];
extern xcoff_complain_function *const xcoff_complain_overflow[];

#define XCOFF_NO_LONG_SECTION_NAMES  (false), bfd_coff_set_long_section_names_disallowed

/* Relocation functions */
extern xcoff_reloc_function xcoff_reloc_type_noop;
extern xcoff_reloc_function xcoff_reloc_type_fail;
extern xcoff_reloc_function xcoff_reloc_type_pos;
extern xcoff_reloc_function xcoff_reloc_type_neg;
extern xcoff_reloc_function xcoff_reloc_type_rel;
extern xcoff_reloc_function xcoff_reloc_type_toc;
extern xcoff_reloc_function xcoff_reloc_type_ba;
extern xcoff_reloc_function xcoff_reloc_type_crel;
extern xcoff_reloc_function xcoff_reloc_type_tls;

/* Structure to describe dwarf sections.
   Useful to convert from XCOFF section name to flag and vice-versa.
   Also mark if section has a length field at the beginning.  */
struct xcoff_dwsect_name {
  /* A XCOFF dwarf section is identified by its name.  */
  unsigned int flag;

  /* Corresponding XCOFF section name.  */
  const char *xcoff_name;

  /* Corresponding DWARF section name.  */
  const char *dwarf_name;

  /* True if size must be prepended.  */
  bool def_size;
};

/* Number of entries in the array.  The number is known and public so that user
   can 'extend' this array by index.  */
#define XCOFF_DWSECT_NBR_NAMES	11

/* The dwarf sections array.  */
extern const struct xcoff_dwsect_name
  xcoff_dwsect_names[XCOFF_DWSECT_NBR_NAMES];

/* Structure and functions needed by backend in order to handle
   stubs created in xcofflink.c.  */

enum xcoff_stub_type
  {
    xcoff_stub_none,
    xcoff_stub_indirect_call,
    xcoff_stub_shared_call
  };

struct xcoff_stub_hash_entry
{
  /* Base hash table entry structure.  */
  struct bfd_hash_entry root;

  enum xcoff_stub_type stub_type;

  /* The hash table entry of the stub's csect.  */
  struct xcoff_link_hash_entry *hcsect;

  /* Offset in the stub's csect.  */
  bfd_vma stub_offset;

  /* The target's section.  */
  asection *target_section;

  /* The target's hash table entry.  */
  struct xcoff_link_hash_entry *htarget;
};


extern enum xcoff_stub_type bfd_xcoff_type_of_stub
  (asection *, const struct internal_reloc *, bfd_vma,
   struct xcoff_link_hash_entry *);

extern struct xcoff_stub_hash_entry *bfd_xcoff_get_stub_entry
  (asection *, struct xcoff_link_hash_entry *, struct bfd_link_info *);

#endif /* LIBXCOFF_H */
