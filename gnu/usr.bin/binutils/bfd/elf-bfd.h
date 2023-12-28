/* BFD back-end data structures for ELF files.
   Copyright (C) 1992-2023 Free Software Foundation, Inc.
   Written by Cygnus Support.

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

#ifndef _LIBELF_H_
#define _LIBELF_H_ 1

#include <stdlib.h>

#include "elf/common.h"
#include "elf/external.h"
#include "elf/internal.h"
#include "bfdlink.h"

#ifndef ENABLE_CHECKING
#define ENABLE_CHECKING 0
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* The number of entries in a section is its size divided by the size
   of a single entry.  This is normally only applicable to reloc and
   symbol table sections.
   PR 9934: It is possible to have relocations that do not refer to
   symbols, thus it is also possible to have a relocation section in
   an object file, but no symbol table.  */
#define NUM_SHDR_ENTRIES(shdr) ((shdr)->sh_entsize > 0 ? (shdr)->sh_size / (shdr)->sh_entsize : 0)

/* If size isn't specified as 64 or 32, NAME macro should fail.  */
#ifndef NAME
#if ARCH_SIZE == 64
#define NAME(x, y) x ## 64 ## _ ## y
#endif
#if ARCH_SIZE == 32
#define NAME(x, y) x ## 32 ## _ ## y
#endif
#endif

#ifndef NAME
#define NAME(x, y) x ## NOSIZE ## _ ## y
#endif

#define ElfNAME(X)	NAME(Elf,X)
#define elfNAME(X)	NAME(elf,X)

/* Information held for an ELF symbol.  The first field is the
   corresponding asymbol.  Every symbol is an ELF file is actually a
   pointer to this structure, although it is often handled as a
   pointer to an asymbol.  */

typedef struct
{
  /* The BFD symbol.  */
  asymbol symbol;
  /* ELF symbol information.  */
  Elf_Internal_Sym internal_elf_sym;
  /* Backend specific information.  */
  union
    {
      unsigned int hppa_arg_reloc;
      void *mips_extr;
      void *any;
    }
  tc_data;

  /* Version information.  This is from an Elf_Internal_Versym
     structure in a SHT_GNU_versym section.  It is zero if there is no
     version information.  */
  unsigned short version;

} elf_symbol_type;

struct elf_strtab_hash;
struct got_entry;
struct plt_entry;

union gotplt_union
  {
    bfd_signed_vma refcount;
    bfd_vma offset;
    struct got_entry *glist;
    struct plt_entry *plist;
  };

struct elf_link_virtual_table_entry
  {
    /* Virtual table entry use information.  This array is nominally of size
       size/sizeof(target_void_pointer), though we have to be able to assume
       and track a size while the symbol is still undefined.  It is indexed
       via offset/sizeof(target_void_pointer).  */
    size_t size;
    bool *used;

    /* Virtual table derivation info.  */
    struct elf_link_hash_entry *parent;
  };

/* ELF symbol version.  */
enum elf_symbol_version
  {
    unknown = 0,
    unversioned,
    versioned,
    versioned_hidden
  };

/* ELF linker hash table entries.  */

struct elf_link_hash_entry
{
  struct bfd_link_hash_entry root;

  /* Symbol index in output file.  This is initialized to -1.  It is
     set to -2 if the symbol is used by a reloc.  It is set to -3 if
     this symbol is defined in a discarded section.  */
  long indx;

  /* Symbol index as a dynamic symbol.  Initialized to -1, and remains
     -1 if this is not a dynamic symbol.  */
  /* ??? Note that this is consistently used as a synonym for tests
     against whether we can perform various simplifying transformations
     to the code.  (E.g. changing a pc-relative jump to a PLT entry
     into a pc-relative jump to the target function.)  That test, which
     is often relatively complex, and someplaces wrong or incomplete,
     should really be replaced by a predicate in elflink.c.

     End result: this field -1 does not indicate that the symbol is
     not in the dynamic symbol table, but rather that the symbol is
     not visible outside this DSO.  */
  long dynindx;

  /* If this symbol requires an entry in the global offset table, the
     processor specific backend uses this field to track usage and
     final offset.  Two schemes are supported:  The first assumes that
     a symbol may only have one GOT entry, and uses REFCOUNT until
     size_dynamic_sections, at which point the contents of the .got is
     fixed.  Afterward, if OFFSET is -1, then the symbol does not
     require a global offset table entry.  The second scheme allows
     multiple GOT entries per symbol, managed via a linked list
     pointed to by GLIST.  */
  union gotplt_union got;

  /* Same, but tracks a procedure linkage table entry.  */
  union gotplt_union plt;

  /* Symbol size.  NB: All fields starting from here are cleared by
    _bfd_elf_link_hash_newfunc.  */
  bfd_size_type size;

  /* Track dynamic relocs copied for this symbol.  */
  struct elf_dyn_relocs *dyn_relocs;

  /* Symbol type (STT_NOTYPE, STT_OBJECT, etc.).  */
  unsigned int type : 8;

  /* Symbol st_other value, symbol visibility.  */
  unsigned int other : 8;

  /* The symbol's st_target_internal value (see Elf_Internal_Sym).  */
  unsigned int target_internal : 8;

  /* Symbol is referenced by a non-shared object (other than the object
     in which it is defined).  */
  unsigned int ref_regular : 1;
  /* Symbol is defined by a non-shared object.  */
  unsigned int def_regular : 1;
  /* Symbol is referenced by a shared object.  */
  unsigned int ref_dynamic : 1;
  /* Symbol is defined by a shared object.  */
  unsigned int def_dynamic : 1;
  /* Symbol has a non-weak reference from a non-shared object (other than
     the object in which it is defined).  */
  unsigned int ref_regular_nonweak : 1;
  /* Symbol has a non-weak reference from a LTO IR object file.  */
  unsigned int ref_ir_nonweak : 1;
  /* Dynamic symbol has been adjustd.  */
  unsigned int dynamic_adjusted : 1;
  /* Symbol needs a copy reloc.  */
  unsigned int needs_copy : 1;
  /* Symbol needs a procedure linkage table entry.  */
  unsigned int needs_plt : 1;
  /* Symbol appears in a non-ELF input file.  */
  unsigned int non_elf : 1;
  /* Symbol version information.  */
  ENUM_BITFIELD (elf_symbol_version) versioned : 2;
  /* Symbol was forced to local scope due to a version script file.  */
  unsigned int forced_local : 1;
  /* Symbol was forced to be dynamic due to a version script file.  */
  unsigned int dynamic : 1;
  /* Symbol was marked during garbage collection.  */
  unsigned int mark : 1;
  /* Symbol is referenced by a non-GOT/non-PLT relocation.  This is
     not currently set by all the backends.  */
  unsigned int non_got_ref : 1;
  /* Symbol has a definition in a shared object.
     FIXME: There is no real need for this field if def_dynamic is never
     cleared and all places that test def_dynamic also test def_regular.  */
  unsigned int dynamic_def : 1;
  /* Symbol has a non-weak reference from a shared object.  */
  unsigned int ref_dynamic_nonweak : 1;
  /* Symbol is referenced with a relocation where C/C++ pointer equality
     matters.  */
  unsigned int pointer_equality_needed : 1;
  /* Symbol is a unique global symbol.  */
  unsigned int unique_global : 1;
  /* Symbol is defined by a shared library with non-default visibility
     in a read/write section.  */
  unsigned int protected_def : 1;
  /* Symbol is __start_SECNAME or __stop_SECNAME to mark section
     SECNAME.  */
  unsigned int start_stop : 1;
  /* Symbol is or was a weak defined symbol from a dynamic object with
     a strong defined symbol alias.  U.ALIAS points to a list of aliases,
     the definition having is_weakalias clear.  */
  unsigned int is_weakalias : 1;

  /* String table index in .dynstr if this is a dynamic symbol.  */
  unsigned long dynstr_index;

  union
  {
    /* Points to a circular list of non-function symbol aliases.  */
    struct elf_link_hash_entry *alias;

    /* Hash value of the name computed using the ELF hash function.
       Used part way through size_dynamic_sections, after we've finished
       with aliases.  */
    unsigned long elf_hash_value;
  } u;

  /* Version information.  */
  union
  {
    /* This field is used for a symbol which is not defined in a
       regular object.  It points to the version information read in
       from the dynamic object.  */
    Elf_Internal_Verdef *verdef;
    /* This field is used for a symbol which is defined in a regular
       object.  It is set up in size_dynamic_sections.  It points to
       the version information we should write out for this symbol.  */
    struct bfd_elf_version_tree *vertree;
  } verinfo;

  union
  {
    /* For __start_SECNAME and __stop_SECNAME symbols, record the first
       input section whose section name is SECNAME.  */
    asection *start_stop_section;

    /* Vtable information. */
    struct elf_link_virtual_table_entry *vtable;
  } u2;
};

/* Return the strong definition for a weak symbol with aliases.  */

static inline struct elf_link_hash_entry *
weakdef (struct elf_link_hash_entry *h)
{
  while (h->is_weakalias)
    h = h->u.alias;
  return h;
}

/* Will references to this symbol always reference the symbol
   in this object?  */
#define SYMBOL_REFERENCES_LOCAL(INFO, H) \
  _bfd_elf_symbol_refs_local_p (H, INFO, 0)

/* Will _calls_ to this symbol always call the version in this object?  */
#define SYMBOL_CALLS_LOCAL(INFO, H) \
  _bfd_elf_symbol_refs_local_p (H, INFO, 1)

/* Whether an undefined weak symbol should resolve to its link-time
   value, even in PIC or PIE objects.  The linker_def test is to
   handle symbols like __ehdr_start that may be undefweak in early
   stages of linking but are guaranteed to be defined later.  */
#define UNDEFWEAK_NO_DYNAMIC_RELOC(INFO, H)		\
  ((H)->root.type == bfd_link_hash_undefweak		\
   && !(H)->root.linker_def				\
   && (ELF_ST_VISIBILITY ((H)->other) != STV_DEFAULT	\
       || (INFO)->dynamic_undefined_weak == 0))

/* Common symbols that are turned into definitions don't have the
   DEF_REGULAR flag set, so they might appear to be undefined.
   Symbols defined in linker scripts also don't have DEF_REGULAR set.  */
#define ELF_COMMON_DEF_P(H) \
  (!(H)->def_regular							\
   && !(H)->def_dynamic							\
   && (H)->root.type == bfd_link_hash_defined)

/* Records local symbols to be emitted in the dynamic symbol table.  */

struct elf_link_local_dynamic_entry
{
  struct elf_link_local_dynamic_entry *next;

  /* The input bfd this symbol came from.  */
  bfd *input_bfd;

  /* The index of the local symbol being copied.  */
  long input_indx;

  /* The index in the outgoing dynamic symbol table.  */
  long dynindx;

  /* A copy of the input symbol.  */
  Elf_Internal_Sym isym;
};

struct elf_link_loaded_list
{
  struct elf_link_loaded_list *next;
  bfd *abfd;
};

/* Structures used by the eh_frame optimization code.  */
struct eh_cie_fde
{
  union {
    struct {
      /* If REMOVED == 1, this is the CIE that the FDE originally used.
	 The CIE belongs to the same .eh_frame input section as the FDE.

	 If REMOVED == 0, this is the CIE that we have chosen to use for
	 the output FDE.  The CIE's REMOVED field is also 0, but the CIE
	 might belong to a different .eh_frame input section from the FDE.

	 May be NULL to signify that the FDE should be discarded.  */
      struct eh_cie_fde *cie_inf;
      struct eh_cie_fde *next_for_section;
    } fde;
    struct {
      /* CIEs have three states:

	 - REMOVED && !MERGED: Slated for removal because we haven't yet
	   proven that an FDE needs it.  FULL_CIE, if nonnull, points to
	   more detailed information about the CIE.

	 - REMOVED && MERGED: We have merged this CIE with MERGED_WITH,
	   which may not belong to the same input section.

	 - !REMOVED: We have decided to keep this CIE.  SEC is the
	   .eh_frame input section that contains the CIE.  */
      union {
	struct cie *full_cie;
	struct eh_cie_fde *merged_with;
	asection *sec;
      } u;

      /* The offset of the personality data from the start of the CIE,
	 or 0 if the CIE doesn't have any.  */
      unsigned int personality_offset : 8;

      /* Length of augmentation.  aug_str_len is the length of the
	 string including null terminator.  aug_data_len is the length
	 of the rest up to the initial insns.  */
      unsigned int aug_str_len : 3;
      unsigned int aug_data_len : 5;

      /* True if we have marked relocations associated with this CIE.  */
      unsigned int gc_mark : 1;

      /* True if we have decided to turn an absolute LSDA encoding into
	 a PC-relative one.  */
      unsigned int make_lsda_relative : 1;

      /* True if we have decided to turn an absolute personality
	 encoding into a PC-relative one.  */
      unsigned int make_per_encoding_relative : 1;

      /* True if the CIE contains personality data and if that
	 data uses a PC-relative encoding.  Always true when
	 make_per_encoding_relative is.  */
      unsigned int per_encoding_relative : 1;

      /* True if the CIE contains personality data aligned to a
	 multiple of eight bytes.  */
      unsigned int per_encoding_aligned8 : 1;

      /* True if we need to add an 'R' (FDE encoding) entry to the
	 CIE's augmentation data.  */
      unsigned int add_fde_encoding : 1;

      /* True if we have merged this CIE with another.  */
      unsigned int merged : 1;

      /* Unused bits.  */
      unsigned int pad1 : 9;
    } cie;
  } u;
  unsigned int reloc_index;
  unsigned int size;
  unsigned int offset;
  unsigned int new_offset;
  unsigned int fde_encoding : 8;
  unsigned int lsda_encoding : 8;
  unsigned int lsda_offset : 8;

  /* True if this entry represents a CIE, false if it represents an FDE.  */
  unsigned int cie : 1;

  /* True if this entry is currently marked for removal.  */
  unsigned int removed : 1;

  /* True if we need to add a 'z' (augmentation size) entry to the CIE's
     augmentation data, and an associated byte to each of the CIE's FDEs.  */
  unsigned int add_augmentation_size : 1;

  /* True if we have decided to convert absolute FDE relocations into
     relative ones.  This applies to the first relocation in the FDE,
     which is against the code that the FDE describes.  */
  unsigned int make_relative : 1;

  /* Unused bits.  */
  unsigned int pad1 : 4;

  unsigned int *set_loc;
};

struct eh_frame_sec_info
{
  unsigned int count;
  struct cie *cies;
  struct eh_cie_fde entry[1];
};

struct eh_frame_array_ent
{
  bfd_vma initial_loc;
  bfd_size_type range;
  bfd_vma fde;
};

struct htab;

#define DWARF2_EH_HDR 1
#define COMPACT_EH_HDR 2

/* Endian-neutral code indicating that a function cannot be unwound.  */
#define COMPACT_EH_CANT_UNWIND_OPCODE 0x015d5d01

struct dwarf_eh_frame_hdr_info
{
  struct htab *cies;
  unsigned int fde_count;
  /* TRUE if .eh_frame_hdr should contain the sorted search table.
     We build it if we successfully read all .eh_frame input sections
     and recognize them.  */
  bool table;
  struct eh_frame_array_ent *array;
};

struct compact_eh_frame_hdr_info
{
  unsigned int allocated_entries;
  /* eh_frame_entry fragments.  */
  asection **entries;
};

struct eh_frame_hdr_info
{
  asection *hdr_sec;
  unsigned int array_count;
  bool frame_hdr_is_compact;
  union
    {
      struct dwarf_eh_frame_hdr_info dwarf;
      struct compact_eh_frame_hdr_info compact;
    }
  u;
};

/* Additional information for each function (used at link time).  */
struct sframe_func_bfdinfo
{
  /* Whether the function has been discarded from the final output.  */
  bool func_deleted_p;
  /* Relocation offset.  */
  unsigned int func_r_offset;
  /* Relocation index.  */
  unsigned int func_reloc_index;
};

/* SFrame decoder info.
   Contains all information for a decoded .sframe section.  */
struct sframe_dec_info
{
  /* Decoder context.  */
  struct sframe_decoder_ctx *sfd_ctx;
  /* Number of function descriptor entries in this .sframe.  */
  unsigned int sfd_fde_count;
  /* Additional information for linking.  */
  struct sframe_func_bfdinfo *sfd_func_bfdinfo;
};

/* SFrame encoder info.
   Contains all information for an encoded .sframe section to be
   written out.  */
struct sframe_enc_info
{
  /* Encoder context.  */
  struct sframe_encoder_ctx *sfe_ctx;
  /* Output section.  */
  asection *sframe_section;
};

/* Enum used to identify target specific extensions to the elf_obj_tdata
   and elf_link_hash_table structures.  Note the enums deliberately start
   from 1 so that we can detect an uninitialized field.  The generic value
   is last so that additions to this enum do not need to modify more than
   one line.  */
enum elf_target_id
{
  AARCH64_ELF_DATA = 1,
  ALPHA_ELF_DATA,
  AMDGCN_ELF_DATA,
  ARC_ELF_DATA,
  ARM_ELF_DATA,
  AVR_ELF_DATA,
  BFIN_ELF_DATA,
  CRIS_ELF_DATA,
  CSKY_ELF_DATA,
  FRV_ELF_DATA,
  HPPA32_ELF_DATA,
  HPPA64_ELF_DATA,
  I386_ELF_DATA,
  IA64_ELF_DATA,
  LM32_ELF_DATA,
  LARCH_ELF_DATA,
  M32R_ELF_DATA,
  M68HC11_ELF_DATA,
  M68K_ELF_DATA,
  METAG_ELF_DATA,
  MICROBLAZE_ELF_DATA,
  MIPS_ELF_DATA,
  MN10300_ELF_DATA,
  NDS32_ELF_DATA,
  NIOS2_ELF_DATA,
  OR1K_ELF_DATA,
  PPC32_ELF_DATA,
  PPC64_ELF_DATA,
  PRU_ELF_DATA,
  S390_ELF_DATA,
  SH_ELF_DATA,
  SPARC_ELF_DATA,
  SPU_ELF_DATA,
  TIC6X_ELF_DATA,
  X86_64_ELF_DATA,
  XTENSA_ELF_DATA,
  TILEGX_ELF_DATA,
  TILEPRO_ELF_DATA,
  RISCV_ELF_DATA,
  GENERIC_ELF_DATA
};

struct elf_sym_strtab
{
  Elf_Internal_Sym sym;
  unsigned long dest_index;
};

struct bfd_link_needed_list
{
  struct bfd_link_needed_list *next;
  bfd *by;
  const char *name;
};

enum elf_target_os
{
  is_normal,
  is_solaris,	/* Solaris.  */
  is_vxworks,	/* VxWorks.  */
  is_nacl	/* Native Client.  */
};

/* Used by bfd_sym_from_r_symndx to cache a small number of local
   symbols.  */
#define LOCAL_SYM_CACHE_SIZE 32
struct sym_cache
{
  bfd *abfd;
  unsigned long indx[LOCAL_SYM_CACHE_SIZE];
  Elf_Internal_Sym sym[LOCAL_SYM_CACHE_SIZE];
};

/* ELF linker hash table.  */

struct elf_link_hash_table
{
  struct bfd_link_hash_table root;

  /* An identifier used to distinguish different target
     specific extensions to this structure.  */
  enum elf_target_id hash_table_id;

  /* Whether we have created the special dynamic sections required
     when linking against or generating a shared object.  */
  bool dynamic_sections_created;

  /* Whether dynamic relocations are present.  */
  bool dynamic_relocs;

  /* True if this target has relocatable executables, so needs dynamic
     section symbols.  */
  bool is_relocatable_executable;

  /* TRUE if there are IFUNC resolvers.  */
  bool ifunc_resolvers;

  /* TRUE if DT_PLTGOT is a required dynamic tag.  */
  bool dt_pltgot_required;

  /* TRUE if DT_JMPREL is a required dynamic tag.  */
  bool dt_jmprel_required;

  /* TRUE when we are handling DT_NEEDED entries.  */
  bool handling_dt_needed;

  /* The BFD used to hold special sections created by the linker.
     This will be the first BFD found which requires these sections to
     be created.  */
  bfd *dynobj;

  /* The value to use when initialising got.refcount/offset and
     plt.refcount/offset in an elf_link_hash_entry.  Set to zero when
     the values are refcounts.  Set to init_got_offset/init_plt_offset
     in size_dynamic_sections when the values may be offsets.  */
  union gotplt_union init_got_refcount;
  union gotplt_union init_plt_refcount;

  /* The value to use for got.refcount/offset and plt.refcount/offset
     when the values may be offsets.  Normally (bfd_vma) -1.  */
  union gotplt_union init_got_offset;
  union gotplt_union init_plt_offset;

  /* The number of symbols found in the link which is intended for the
     mandatory DT_SYMTAB tag (.dynsym section) in .dynamic section.  */
  bfd_size_type dynsymcount;
  bfd_size_type local_dynsymcount;

  /* The string table of dynamic symbols, which becomes the .dynstr
     section.  */
  struct elf_strtab_hash *dynstr;

  /* The array size of the symbol string table, which becomes the
     .strtab section.  */
  bfd_size_type strtabsize;

  /* The array of strings, which becomes the .strtab section.  */
  struct elf_sym_strtab *strtab;

  /* The number of buckets in the hash table in the .hash section.
     This is based on the number of dynamic symbols.  */
  bfd_size_type bucketcount;

  /* A linked list of DT_NEEDED names found in dynamic objects
     included in the link.  */
  struct bfd_link_needed_list *needed;

  /* Sections in the output bfd that provides a section symbol
     to be used by relocations emitted against local symbols.
     Most targets will not use data_index_section.  */
  asection *text_index_section;
  asection *data_index_section;

  /* The _GLOBAL_OFFSET_TABLE_ symbol.  */
  struct elf_link_hash_entry *hgot;

  /* The _PROCEDURE_LINKAGE_TABLE_ symbol.  */
  struct elf_link_hash_entry *hplt;

  /* The _DYNAMIC symbol.  */
  struct elf_link_hash_entry *hdynamic;

  /* A pointer to information used to merge SEC_MERGE sections.  */
  void *merge_info;

  /* Used to link stabs in sections.  */
  struct stab_info stab_info;

  /* Used by eh_frame code when editing .eh_frame.  */
  struct eh_frame_hdr_info eh_info;

  /* Used to link stack trace info in .sframe sections.  */
  struct sframe_enc_info sfe_info;

  /* A linked list of local symbols to be added to .dynsym.  */
  struct elf_link_local_dynamic_entry *dynlocal;

  /* A linked list of DT_RPATH/DT_RUNPATH names found in dynamic
     objects included in the link.  */
  struct bfd_link_needed_list *runpath;

  /* Cached first output tls section and size of PT_TLS segment.  */
  asection *tls_sec;
  bfd_size_type tls_size;  /* Bytes.  */

  /* The offset into splt of the PLT entry for the TLS descriptor
     resolver.  Special values are 0, if not necessary (or not found
     to be necessary yet), and -1 if needed but not determined
     yet.  */
  bfd_vma tlsdesc_plt;

  /* The GOT offset for the lazy trampoline.  Communicated to the
     loader via DT_TLSDESC_GOT.  The magic value (bfd_vma) -1
     indicates an offset is not allocated.  */
  bfd_vma tlsdesc_got;

  /* Target OS for linker output.  */
  enum elf_target_os target_os;

  /* A linked list of dynamic BFD's loaded in the link.  */
  struct elf_link_loaded_list *dyn_loaded;

  /* Small local sym cache.  */
  struct sym_cache sym_cache;

  /* Short-cuts to get to dynamic linker sections.  */
  asection *sgot;
  asection *sgotplt;
  asection *srelgot;
  asection *splt;
  asection *srelplt;
  asection *sdynbss;
  asection *srelbss;
  asection *sdynrelro;
  asection *sreldynrelro;
  asection *igotplt;
  asection *iplt;
  asection *irelplt;
  asection *irelifunc;
  asection *dynsym;
  asection *srelrdyn;
};

/* Returns TRUE if the hash table is a struct elf_link_hash_table.  */

static inline bool
is_elf_hash_table (const struct bfd_link_hash_table *htab)
{
  return htab->type == bfd_link_elf_hash_table;
}

/* Look up an entry in an ELF linker hash table.  */

static inline struct elf_link_hash_entry *
elf_link_hash_lookup (struct elf_link_hash_table *table, const char *string,
		      bool create, bool copy, bool follow)
{
  if (ENABLE_CHECKING && !is_elf_hash_table (&table->root))
    abort ();
  return (struct elf_link_hash_entry *)
    bfd_link_hash_lookup (&table->root, string, create, copy, follow);
}

/* Traverse an ELF linker hash table.  */

static inline void
elf_link_hash_traverse (struct elf_link_hash_table *table,
			bool (*f) (struct elf_link_hash_entry *, void *),
			void *info)
{
  if (ENABLE_CHECKING && !is_elf_hash_table (&table->root))
    abort ();
  bfd_link_hash_traverse (&table->root,
			  (bool (*) (struct bfd_link_hash_entry *, void *)) f,
			  info);
}

/* Get the ELF linker hash table from a link_info structure.  */

static inline struct elf_link_hash_table *
elf_hash_table (const struct bfd_link_info *info)
{
  return (struct elf_link_hash_table *) info->hash;
}

static inline enum elf_target_id
elf_hash_table_id (const struct elf_link_hash_table *table)
{
  return table->hash_table_id;
}

/* Constant information held for an ELF backend.  */

struct elf_size_info {
  unsigned char sizeof_ehdr, sizeof_phdr, sizeof_shdr;
  unsigned char sizeof_rel, sizeof_rela, sizeof_sym, sizeof_dyn, sizeof_note;

  /* The size of entries in the .hash section.  */
  unsigned char sizeof_hash_entry;

  /* The number of internal relocations to allocate per external
     relocation entry.  */
  unsigned char int_rels_per_ext_rel;
  /* We use some fixed size arrays.  This should be large enough to
     handle all back-ends.  */
#define MAX_INT_RELS_PER_EXT_REL 3

  unsigned char arch_size, log_file_align;
  unsigned char elfclass, ev_current;
  int (*write_out_phdrs)
    (bfd *, const Elf_Internal_Phdr *, unsigned int);
  bool (*write_shdrs_and_ehdr) (bfd *);
  bool (*checksum_contents)
    (bfd * , void (*) (const void *, size_t, void *), void *);
  void (*write_relocs)
    (bfd *, asection *, void *);
  bool (*swap_symbol_in)
    (bfd *, const void *, const void *, Elf_Internal_Sym *);
  void (*swap_symbol_out)
    (bfd *, const Elf_Internal_Sym *, void *, void *);
  bool (*slurp_reloc_table)
    (bfd *, asection *, asymbol **, bool);
  long (*slurp_symbol_table)
    (bfd *, asymbol **, bool);
  void (*swap_dyn_in)
    (bfd *, const void *, Elf_Internal_Dyn *);
  void (*swap_dyn_out)
    (bfd *, const Elf_Internal_Dyn *, void *);

  /* This function is called to swap in a REL relocation.  If an
     external relocation corresponds to more than one internal
     relocation, then all relocations are swapped in at once.  */
  void (*swap_reloc_in)
    (bfd *, const bfd_byte *, Elf_Internal_Rela *);

  /* This function is called to swap out a REL relocation.  */
  void (*swap_reloc_out)
    (bfd *, const Elf_Internal_Rela *, bfd_byte *);

  /* This function is called to swap in a RELA relocation.  If an
     external relocation corresponds to more than one internal
     relocation, then all relocations are swapped in at once.  */
  void (*swap_reloca_in)
    (bfd *, const bfd_byte *, Elf_Internal_Rela *);

  /* This function is called to swap out a RELA relocation.  */
  void (*swap_reloca_out)
    (bfd *, const Elf_Internal_Rela *, bfd_byte *);
};

#define elf_symbol_from(S) \
  ((((S)->flags & BSF_SYNTHETIC) == 0				\
    && (S)->the_bfd != NULL					\
    && (S)->the_bfd->xvec->flavour == bfd_target_elf_flavour	\
    && (S)->the_bfd->tdata.elf_obj_data != 0)			\
   ? (elf_symbol_type *) (S)					\
   : 0)

enum elf_reloc_type_class {
  reloc_class_normal,
  reloc_class_relative,
  reloc_class_copy,
  reloc_class_ifunc,
  reloc_class_plt
};

struct elf_reloc_cookie
{
  Elf_Internal_Rela *rels, *rel, *relend;
  Elf_Internal_Sym *locsyms;
  bfd *abfd;
  size_t locsymcount;
  size_t extsymoff;
  struct elf_link_hash_entry **sym_hashes;
  int r_sym_shift;
  bool bad_symtab;
};

/* The level of IRIX compatibility we're striving for.  */

typedef enum {
  ict_none,
  ict_irix5,
  ict_irix6
} irix_compat_t;

/* Mapping of ELF section names and types.  */
struct bfd_elf_special_section
{
  const char *prefix;
  unsigned int prefix_length;
  /* 0 means name must match PREFIX exactly.
     -1 means name must start with PREFIX followed by an arbitrary string.
     -2 means name must match PREFIX exactly or consist of PREFIX followed
     by a dot then anything.
     > 0 means name must start with the first PREFIX_LENGTH chars of
     PREFIX and finish with the last SUFFIX_LENGTH chars of PREFIX.  */
  signed int suffix_length;
  unsigned int type;
  bfd_vma attr;
};

enum action_discarded
  {
    COMPLAIN = 1,
    PRETEND = 2
  };

typedef asection * (*elf_gc_mark_hook_fn)
  (asection *, struct bfd_link_info *, Elf_Internal_Rela *,
   struct elf_link_hash_entry *, Elf_Internal_Sym *);

enum elf_property_kind
 {
    /* A new property.  */
    property_unknown = 0,
    /* A property ignored by backend.  */
    property_ignored,
    /* A corrupt property reported by backend.  */
    property_corrupt,
    /* A property should be removed due to property merge.  */
    property_remove,
    /* A property which is a number.  */
    property_number
 };

typedef struct elf_property
{
  unsigned int pr_type;
  unsigned int pr_datasz;
  union
    {
      /* For property_number, this is a number.  */
      bfd_vma number;
      /* Add a new one if elf_property_kind is updated.  */
    } u;
  enum elf_property_kind pr_kind;
} elf_property;

typedef struct elf_property_list
{
  struct elf_property_list *next;
  struct elf_property property;
} elf_property_list;

struct bfd_elf_section_reloc_data;

struct elf_backend_data
{
  /* The architecture for this backend.  */
  enum bfd_architecture arch;

  /* An identifier used to distinguish different target specific
     extensions to elf_obj_tdata and elf_link_hash_table structures.  */
  enum elf_target_id target_id;

  /* Target OS.  */
  enum elf_target_os target_os;

  /* The ELF machine code (EM_xxxx) for this backend.  */
  int elf_machine_code;

  /* EI_OSABI.  */
  int elf_osabi;

  /* The maximum page size for this backend.  */
  bfd_vma maxpagesize;

  /* The minimum page size for this backend.  An input object will not be
     considered page aligned unless its sections are correctly aligned for
     pages at least this large.  May be smaller than maxpagesize.  */
  bfd_vma minpagesize;

  /* The common page size for this backend.  */
  bfd_vma commonpagesize;

  /* The p_align value for this backend.  If it is set, p_align of
      PT_LOAD alignment will be to p_align by default.  */
  bfd_vma p_align;

  /* The BFD flags applied to sections created for dynamic linking.  */
  flagword dynamic_sec_flags;

  /* Architecture-specific data for this backend.
     This is actually a pointer to some type like struct elf_ARCH_data.  */
  const void *arch_data;

  /* A function to translate an ELF RELA relocation to a BFD arelent
     structure.  Returns TRUE upon success, FALSE otherwise.  */
  bool (*elf_info_to_howto)
    (bfd *, arelent *, Elf_Internal_Rela *);

  /* A function to translate an ELF REL relocation to a BFD arelent
     structure.  Returns TRUE upon success, FALSE otherwise.  */
  bool (*elf_info_to_howto_rel)
    (bfd *, arelent *, Elf_Internal_Rela *);

  /* A function to determine whether a symbol is global when
     partitioning the symbol table into local and global symbols.
     This should be NULL for most targets, in which case the correct
     thing will be done.  MIPS ELF, at least on the Irix 5, has
     special requirements.  */
  bool (*elf_backend_sym_is_global)
    (bfd *, asymbol *);

  /* The remaining functions are hooks which are called only if they
     are not NULL.  */

  /* A function to permit a backend specific check on whether a
     particular BFD format is relevant for an object file, and to
     permit the backend to set any global information it wishes.  When
     this is called elf_elfheader is set, but anything else should be
     used with caution.  If this returns FALSE, the check_format
     routine will return a bfd_error_wrong_format error.  */
  bool (*elf_backend_object_p)
    (bfd *);

  /* A function to do additional symbol processing when reading the
     ELF symbol table.  This is where any processor-specific special
     section indices are handled.  */
  void (*elf_backend_symbol_processing)
    (bfd *, asymbol *);

  /* A function to do additional symbol processing after reading the
     entire ELF symbol table.  */
  bool (*elf_backend_symbol_table_processing)
    (bfd *, elf_symbol_type *, unsigned int);

  /* A function to set the type of the info field.  Processor-specific
     types should be handled here.  */
  int (*elf_backend_get_symbol_type)
    (Elf_Internal_Sym *, int);

  /* A function to return the linker hash table entry of a symbol that
     might be satisfied by an archive symbol.  */
  struct bfd_link_hash_entry * (*elf_backend_archive_symbol_lookup)
    (bfd *, struct bfd_link_info *, const char *);

  /* Return true if local section symbols should have a non-null st_name.
     NULL implies false.  */
  bool (*elf_backend_name_local_section_symbols)
    (bfd *);

  /* A function to do additional processing on the ELF section header
     just before writing it out.  This is used to set the flags and
     type fields for some sections, or to actually write out data for
     unusual sections.  */
  bool (*elf_backend_section_processing)
    (bfd *, Elf_Internal_Shdr *);

  /* A function to handle unusual section types when creating BFD
     sections from ELF sections.  */
  bool (*elf_backend_section_from_shdr)
    (bfd *, Elf_Internal_Shdr *, const char *, int);

  /* A function to convert machine dependent ELF section header flags to
     BFD internal section header flags.  */
  bool (*elf_backend_section_flags)
    (const Elf_Internal_Shdr *);

  /* A function that returns a struct containing ELF section flags and
     type for the given BFD section.   */
  const struct bfd_elf_special_section * (*get_sec_type_attr)
    (bfd *, asection *);

  /* A function to handle unusual program segment types when creating BFD
     sections from ELF program segments.  */
  bool (*elf_backend_section_from_phdr)
    (bfd *, Elf_Internal_Phdr *, int, const char *);

  /* A function to set up the ELF section header for a BFD section in
     preparation for writing it out.  This is where the flags and type
     fields are set for unusual sections.  */
  bool (*elf_backend_fake_sections)
    (bfd *, Elf_Internal_Shdr *, asection *);

  /* A function to get the ELF section index for a BFD section.  If
     this returns TRUE, the section was found.  If it is a normal ELF
     section, *RETVAL should be left unchanged.  If it is not a normal
     ELF section *RETVAL should be set to the SHN_xxxx index.  */
  bool (*elf_backend_section_from_bfd_section)
    (bfd *, asection *, int *retval);

  /* If this field is not NULL, it is called by the add_symbols phase
     of a link just before adding a symbol to the global linker hash
     table.  It may modify any of the fields as it wishes.  If *NAME
     is set to NULL, the symbol will be skipped rather than being
     added to the hash table.  This function is responsible for
     handling all processor dependent symbol bindings and section
     indices, and must set at least *FLAGS and *SEC for each processor
     dependent case; failure to do so will cause a link error.  */
  bool (*elf_add_symbol_hook)
    (bfd *abfd, struct bfd_link_info *info, Elf_Internal_Sym *,
     const char **name, flagword *flags, asection **sec, bfd_vma *value);

  /* If this field is not NULL, it is called by the elf_link_output_sym
     phase of a link for each symbol which will appear in the object file.
     On error, this function returns 0.  1 is returned when the symbol
     should be output, 2 is returned when the symbol should be discarded.  */
  int (*elf_backend_link_output_symbol_hook)
    (struct bfd_link_info *info, const char *, Elf_Internal_Sym *,
     asection *, struct elf_link_hash_entry *);

  /* The CREATE_DYNAMIC_SECTIONS function is called by the ELF backend
     linker the first time it encounters a dynamic object in the link.
     This function must create any sections required for dynamic
     linking.  The ABFD argument is a dynamic object.  The .interp,
     .dynamic, .dynsym, .dynstr, and .hash functions have already been
     created, and this function may modify the section flags if
     desired.  This function will normally create the .got and .plt
     sections, but different backends have different requirements.  */
  bool (*elf_backend_create_dynamic_sections)
    (bfd *abfd, struct bfd_link_info *info);

  /* When creating a shared library, determine whether to omit the
     dynamic symbol for the section.  */
  bool (*elf_backend_omit_section_dynsym)
    (bfd *output_bfd, struct bfd_link_info *info, asection *osec);

  /* Return TRUE if relocations of targets are compatible to the extent
     that CHECK_RELOCS will properly process them.  PR 4424.  */
  bool (*relocs_compatible) (const bfd_target *, const bfd_target *);

  /* The CHECK_RELOCS function is called after all input files have been
     opened.  It is called once for each section with relocs of an object
     file.  The function must look through the relocs and do any special
     handling required.  This generally means allocating space in the
     global offset table, and perhaps allocating space for a reloc.  The
     relocs are always passed as Rela structures; if the section
     actually uses Rel structures, the r_addend field will always be
     zero.  */
  bool (*check_relocs)
    (bfd *abfd, struct bfd_link_info *info, asection *o,
     const Elf_Internal_Rela *relocs);

  /* The SIZE_RELATIVE_RELOCS function is called to size relative
     relocations when mappig sections to segments.  */
  bool (*size_relative_relocs)
    (struct bfd_link_info *info, bool *need_layout);

  /* The FINISH_RELATIVE_RELOCS function is called to finish relative
     relocations in bfd_elf_final_link.  */
  bool (*finish_relative_relocs)
    (struct bfd_link_info *info);

  /* The CHECK_DIRECTIVES function is called once per input file by
     the add_symbols phase of the ELF backend linker.  The function
     must inspect the bfd and create any additional symbols according
     to any custom directives in the bfd.  */
  bool (*check_directives)
    (bfd *abfd, struct bfd_link_info *info);

  /* The NOTICE_AS_NEEDED function is called as the linker is about to
     handle an as-needed lib (ACT = notice_as_needed), and after the
     linker has decided to keep the lib (ACT = notice_needed) or when
     the lib is not needed (ACT = notice_not_needed).  */
  bool (*notice_as_needed)
    (bfd *abfd, struct bfd_link_info *info, enum notice_asneeded_action act);

  /* The ADJUST_DYNAMIC_SYMBOL function is called by the ELF backend
     linker for every symbol which is defined by a dynamic object and
     referenced by a regular object.  This is called after all the
     input files have been seen, but before the SIZE_DYNAMIC_SECTIONS
     function has been called.  The hash table entry should be
     bfd_link_hash_defined ore bfd_link_hash_defweak, and it should be
     defined in a section from a dynamic object.  Dynamic object
     sections are not included in the final link, and this function is
     responsible for changing the value to something which the rest of
     the link can deal with.  This will normally involve adding an
     entry to the .plt or .got or some such section, and setting the
     symbol to point to that.  */
  bool (*elf_backend_adjust_dynamic_symbol)
    (struct bfd_link_info *info, struct elf_link_hash_entry *h);

  /* The ALWAYS_SIZE_SECTIONS function is called by the backend linker
     after all the linker input files have been seen but before the
     section sizes have been set.  This is called after
     ADJUST_DYNAMIC_SYMBOL, but before SIZE_DYNAMIC_SECTIONS.  */
  bool (*elf_backend_always_size_sections)
    (bfd *output_bfd, struct bfd_link_info *info);

  /* The SIZE_DYNAMIC_SECTIONS function is called by the ELF backend
     linker after all the linker input files have been seen but before
     the sections sizes have been set.  This is called after
     ADJUST_DYNAMIC_SYMBOL has been called on all appropriate symbols.
     It is only called when linking against a dynamic object.  It must
     set the sizes of the dynamic sections, and may fill in their
     contents as well.  The generic ELF linker can handle the .dynsym,
     .dynstr and .hash sections.  This function must handle the
     .interp section and any sections created by the
     CREATE_DYNAMIC_SECTIONS entry point.  */
  bool (*elf_backend_size_dynamic_sections)
    (bfd *output_bfd, struct bfd_link_info *info);

  /* The STRIP_ZERO_SIZED_DYNAMIC_SECTIONS function is called by the
     ELF backend linker to strip zero-sized dynamic sections after
     the section sizes have been set.  */
  bool (*elf_backend_strip_zero_sized_dynamic_sections)
    (struct bfd_link_info *info);

  /* Set TEXT_INDEX_SECTION and DATA_INDEX_SECTION, the output sections
     we keep to use as a base for relocs and symbols.  */
  void (*elf_backend_init_index_section)
    (bfd *output_bfd, struct bfd_link_info *info);

  /* The RELOCATE_SECTION function is called by the ELF backend linker
     to handle the relocations for a section.

     The relocs are always passed as Rela structures; if the section
     actually uses Rel structures, the r_addend field will always be
     zero.

     This function is responsible for adjust the section contents as
     necessary, and (if using Rela relocs and generating a
     relocatable output file) adjusting the reloc addend as
     necessary.

     This function does not have to worry about setting the reloc
     address or the reloc symbol index.

     LOCAL_SYMS is a pointer to the swapped in local symbols.

     LOCAL_SECTIONS is an array giving the section in the input file
     corresponding to the st_shndx field of each local symbol.

     The global hash table entry for the global symbols can be found
     via elf_sym_hashes (input_bfd).

     When generating relocatable output, this function must handle
     STB_LOCAL/STT_SECTION symbols specially.  The output symbol is
     going to be the section symbol corresponding to the output
     section, which means that the addend must be adjusted
     accordingly.

     Returns FALSE on error, TRUE on success, 2 if successful and
     relocations should be written for this section.  */
  int (*elf_backend_relocate_section)
    (bfd *output_bfd, struct bfd_link_info *info, bfd *input_bfd,
     asection *input_section, bfd_byte *contents, Elf_Internal_Rela *relocs,
     Elf_Internal_Sym *local_syms, asection **local_sections);

  /* The FINISH_DYNAMIC_SYMBOL function is called by the ELF backend
     linker just before it writes a symbol out to the .dynsym section.
     The processor backend may make any required adjustment to the
     symbol.  It may also take the opportunity to set contents of the
     dynamic sections.  Note that FINISH_DYNAMIC_SYMBOL is called on
     all .dynsym symbols, while ADJUST_DYNAMIC_SYMBOL is only called
     on those symbols which are defined by a dynamic object.  */
  bool (*elf_backend_finish_dynamic_symbol)
    (bfd *output_bfd, struct bfd_link_info *info,
     struct elf_link_hash_entry *h, Elf_Internal_Sym *sym);

  /* The FINISH_DYNAMIC_SECTIONS function is called by the ELF backend
     linker just before it writes all the dynamic sections out to the
     output file.  The FINISH_DYNAMIC_SYMBOL will have been called on
     all dynamic symbols.  */
  bool (*elf_backend_finish_dynamic_sections)
    (bfd *output_bfd, struct bfd_link_info *info);

  /* A function to do any beginning processing needed for the ELF file
     before building the ELF headers and computing file positions.  */
  void (*elf_backend_begin_write_processing)
    (bfd *, struct bfd_link_info *);

  /* A function to do any final processing needed for the ELF file
     before writing it out.  */
  bool (*elf_backend_final_write_processing)
    (bfd *);

  /* This function is called by get_program_header_size.  It should
     return the number of additional program segments which this BFD
     will need.  It should return -1 on error.  */
  int (*elf_backend_additional_program_headers)
    (bfd *, struct bfd_link_info *);

  /* This function is called to modify an existing segment map in a
     backend specific fashion.  */
  bool (*elf_backend_modify_segment_map)
    (bfd *, struct bfd_link_info *);

  /* This function is called to modify program headers just before
     they are written.  */
  bool (*elf_backend_modify_headers)
    (bfd *, struct bfd_link_info *);

  /* This function is called to see if the PHDR header should be
     checked for validity.  */
  bool (*elf_backend_allow_non_load_phdr)
    (bfd *,  const Elf_Internal_Phdr *, unsigned);

  /* This function is called before section garbage collection to
     mark entry symbol sections.  */
  void (*gc_keep)
    (struct bfd_link_info *);

  /* This function is called during section garbage collection to
     mark sections that define global symbols.  */
  bool (*gc_mark_dynamic_ref)
    (struct elf_link_hash_entry *, void *);

  /* This function is called during section gc to discover the section a
     particular relocation refers to.  */
  elf_gc_mark_hook_fn gc_mark_hook;

  /* This function, if defined, is called after the first gc marking pass
     to allow the backend to mark additional sections.  */
  bool (*gc_mark_extra_sections)
    (struct bfd_link_info *, elf_gc_mark_hook_fn);

  /* This function is called to initialise ELF file header info.
     Customised versions can modify things like the OS and ABI version.  */
  bool (*elf_backend_init_file_header)
    (bfd *, struct bfd_link_info *);

  /* This function, if defined, prints a symbol to file and returns the
     name of the symbol to be printed.  It should return NULL to fall
     back to default symbol printing.  */
  const char *(*elf_backend_print_symbol_all)
    (bfd *, void *, asymbol *);

  /* This function, if defined, is called after all local symbols and
     global symbols converted to locals are emitted into the symtab
     section.  It allows the backend to emit special local symbols
     not handled in the hash table.  */
  bool (*elf_backend_output_arch_local_syms)
    (bfd *, struct bfd_link_info *, void *,
     int (*) (void *, const char *, Elf_Internal_Sym *, asection *,
	      struct elf_link_hash_entry *));

  /* This function, if defined, is called after all symbols are emitted
     into the symtab section.  It allows the backend to emit special
     global symbols not handled in the hash table.  */
  bool (*elf_backend_output_arch_syms)
    (bfd *, struct bfd_link_info *, void *,
     int (*) (void *, const char *, Elf_Internal_Sym *, asection *,
	      struct elf_link_hash_entry *));

  /* Filter what symbols of the output file to include in the import
     library if one is created.  */
  unsigned int (*elf_backend_filter_implib_symbols)
    (bfd *, struct bfd_link_info *, asymbol **, long);

  /* Copy any information related to dynamic linking from a pre-existing
     symbol to a newly created symbol.  Also called to copy flags and
     other back-end info to a weakdef, in which case the symbol is not
     newly created and plt/got refcounts and dynamic indices should not
     be copied.  */
  void (*elf_backend_copy_indirect_symbol)
    (struct bfd_link_info *, struct elf_link_hash_entry *,
     struct elf_link_hash_entry *);

  /* Modify any information related to dynamic linking such that the
     symbol is not exported.  */
  void (*elf_backend_hide_symbol)
    (struct bfd_link_info *, struct elf_link_hash_entry *, bool);

  /* A function to do additional symbol fixup, called by
     _bfd_elf_fix_symbol_flags.  */
  bool (*elf_backend_fixup_symbol)
    (struct bfd_link_info *, struct elf_link_hash_entry *);

  /* Merge the backend specific symbol attribute.  */
  void (*elf_backend_merge_symbol_attribute)
    (struct elf_link_hash_entry *, unsigned int, bool, bool);

  /* This function, if defined, will return a string containing the
     name of a target-specific dynamic tag.  */
  char *(*elf_backend_get_target_dtag)
    (bfd_vma);

  /* Decide whether an undefined symbol is special and can be ignored.
     This is the case for OPTIONAL symbols on IRIX.  */
  bool (*elf_backend_ignore_undef_symbol)
    (struct elf_link_hash_entry *);

  /* Emit relocations.  Overrides default routine for emitting relocs,
     except during a relocatable link, or if all relocs are being emitted.  */
  bool (*elf_backend_emit_relocs)
    (bfd *, asection *, Elf_Internal_Shdr *, Elf_Internal_Rela *,
     struct elf_link_hash_entry **);

  /* Update relocations.  It is allowed to change the number and the order.
     In such a case hashes should be invalidated.  */
  void (*elf_backend_update_relocs)
    (asection *, struct bfd_elf_section_reloc_data *);

  /* Count relocations.  Not called for relocatable links
     or if all relocs are being preserved in the output.  */
  unsigned int (*elf_backend_count_relocs)
    (struct bfd_link_info *, asection *);

  /* Count additionals relocations.  Called for relocatable links if
     additional relocations needs to be created.  */
  unsigned int (*elf_backend_count_additional_relocs)
    (asection *);

  /* Say whether to sort relocs output by ld -r and ld --emit-relocs,
     by r_offset.  If NULL, default to true.  */
  bool (*sort_relocs_p)
    (asection *);

  /* This function, if defined, is called when an NT_PRSTATUS note is found
     in a core file.  */
  bool (*elf_backend_grok_prstatus)
    (bfd *, Elf_Internal_Note *);

  /* This function, if defined, is called when an NT_PSINFO or NT_PRPSINFO
     note is found in a core file.  */
  bool (*elf_backend_grok_psinfo)
    (bfd *, Elf_Internal_Note *);

  /* This function, if defined, is called when a "FreeBSD" NT_PRSTATUS
     note is found in a core file.  */
  bool (*elf_backend_grok_freebsd_prstatus)
    (bfd *, Elf_Internal_Note *);

  /* This function, if defined, is called to write a note to a corefile.  */
  char *(*elf_backend_write_core_note)
    (bfd *abfd, char *buf, int *bufsiz, int note_type, ...);

  /* This function, if defined, is called to convert target-specific
     section flag names into hex values.  */
  flagword (*elf_backend_lookup_section_flags_hook)
    (char *);

  /* This function returns class of a reloc type.  */
  enum elf_reloc_type_class (*elf_backend_reloc_type_class)
  (const struct bfd_link_info *, const asection *, const Elf_Internal_Rela *);

  /* This function, if defined, removes information about discarded functions
     from other sections which mention them.  */
  bool (*elf_backend_discard_info)
    (bfd *, struct elf_reloc_cookie *, struct bfd_link_info *);

  /* This function, if defined, signals that the function above has removed
     the discarded relocations for this section.  */
  bool (*elf_backend_ignore_discarded_relocs)
    (asection *);

  /* What to do when ld finds relocations against symbols defined in
     discarded sections.  */
  unsigned int (*action_discarded)
    (asection *);

  /* This function returns the width of FDE pointers in bytes, or 0 if
     that can't be determined for some reason.  The default definition
     goes by the bfd's EI_CLASS.  */
  unsigned int (*elf_backend_eh_frame_address_size)
    (bfd *, const asection *);

  /* These functions tell elf-eh-frame whether to attempt to turn
     absolute or lsda encodings into pc-relative ones.  The default
     definition enables these transformations.  */
  bool (*elf_backend_can_make_relative_eh_frame)
     (bfd *, struct bfd_link_info *, asection *);
  bool (*elf_backend_can_make_lsda_relative_eh_frame)
     (bfd *, struct bfd_link_info *, asection *);

  /* Tell linker to support multiple eh_frame sections.  */
  bool elf_backend_can_make_multiple_eh_frame;

  /* This function returns an encoding after computing the encoded
     value (and storing it in ENCODED) for the given OFFSET into OSEC,
     to be stored in at LOC_OFFSET into the LOC_SEC input section.
     The default definition chooses a 32-bit PC-relative encoding.  */
  bfd_byte (*elf_backend_encode_eh_address)
     (bfd *abfd, struct bfd_link_info *info,
      asection *osec, bfd_vma offset,
      asection *loc_sec, bfd_vma loc_offset,
      bfd_vma *encoded);

  /* This function, if defined, may write out the given section.
     Returns TRUE if it did so and FALSE if the caller should.  */
  bool (*elf_backend_write_section)
    (bfd *, struct bfd_link_info *, asection *, bfd_byte *);

  /* This function, if defined, returns TRUE if it is section symbols
     only that are considered local for the purpose of partitioning the
     symbol table into local and global symbols.  This should be NULL
     for most targets, in which case the correct thing will be done.
     MIPS ELF, at least on the Irix 5, has special requirements.  */
  bool (*elf_backend_elfsym_local_is_section)
    (bfd *);

  /* The level of IRIX compatibility we're striving for.
     MIPS ELF specific function.  */
  irix_compat_t (*elf_backend_mips_irix_compat)
    (bfd *);

  reloc_howto_type *(*elf_backend_mips_rtype_to_howto)
    (bfd *, unsigned int, bool);

  /* The swapping table to use when dealing with ECOFF information.
     Used for the MIPS ELF .mdebug section.  */
  const struct ecoff_debug_swap *elf_backend_ecoff_debug_swap;

  /* This function implements `bfd_elf_bfd_from_remote_memory';
     see elf.c, elfcode.h.  */
  bfd *(*elf_backend_bfd_from_remote_memory)
    (bfd *templ, bfd_vma ehdr_vma, bfd_size_type size, bfd_vma *loadbasep,
     int (*target_read_memory) (bfd_vma vma, bfd_byte *myaddr,
				bfd_size_type len));

  bool (*elf_backend_core_find_build_id) (bfd *, bfd_vma);

  /* This function is used by `_bfd_elf_get_synthetic_symtab';
     see elf.c.  */
  bfd_vma (*plt_sym_val) (bfd_vma, const asection *, const arelent *);

  /* Is symbol defined in common section?  */
  bool (*common_definition) (Elf_Internal_Sym *);

  /* Return a common section index for section.  */
  unsigned int (*common_section_index) (asection *);

  /* Return a common section for section.  */
  asection *(*common_section) (asection *);

  /* Return TRUE if we can merge 2 definitions.  */
  bool (*merge_symbol) (struct elf_link_hash_entry *,
			       const Elf_Internal_Sym *, asection **,
			       bool, bool,
			       bfd *, const asection *);

  /* Return TRUE if symbol should be hashed in the `.gnu.hash' section.  */
  bool (*elf_hash_symbol) (struct elf_link_hash_entry *);

  /* If non-NULL, called to register the location of XLAT_LOC within
     .MIPS.xhash at which real final dynindx for H will be written.
     If XLAT_LOC is zero, the symbol is not included in
     .MIPS.xhash and no dynindx will be written.  */
  void (*record_xhash_symbol)
    (struct elf_link_hash_entry *h, bfd_vma xlat_loc);

  /* Return TRUE if type is a function symbol type.  */
  bool (*is_function_type) (unsigned int type);

  /* If the ELF symbol SYM might be a function in SEC, return the
     function size and set *CODE_OFF to the function's entry point,
     otherwise return zero.  */
  bfd_size_type (*maybe_function_sym) (const asymbol *sym, asection *sec,
				       bfd_vma *code_off);

  /* Given NAME, the name of a relocation section stripped of its
     .rel/.rela prefix, return the section in ABFD to which the
     relocations apply.  */
  asection *(*get_reloc_section) (bfd *abfd, const char *name);

  /* Called to set the sh_flags, sh_link and sh_info fields of OSECTION which
     has a type >= SHT_LOOS.  Returns TRUE if the fields were initialised,
     FALSE otherwise.  Can be called multiple times for a given section,
     until it returns TRUE.  Most of the times it is called ISECTION will be
     set to an input section that might be associated with the output section.
     The last time that it is called, ISECTION will be set to NULL.  */
  bool (*elf_backend_copy_special_section_fields)
    (const bfd *ibfd, bfd *obfd, const Elf_Internal_Shdr *isection,
     Elf_Internal_Shdr *osection);

  /* Used to handle bad SHF_LINK_ORDER input.  */
  void (*link_order_error_handler) (const char *, ...);

  /* Name of the PLT relocation section.  */
  const char *relplt_name;

  /* Alternate EM_xxxx machine codes for this backend.  */
  int elf_machine_alt1;
  int elf_machine_alt2;

  const struct elf_size_info *s;

  /* An array of target specific special sections.  */
  const struct bfd_elf_special_section *special_sections;

  /* The size in bytes of the header for the GOT.  This includes the
     so-called reserved entries on some systems.  */
  bfd_vma got_header_size;

  /* The size of the GOT entry for the symbol pointed to by H if non-NULL,
     otherwise by the local symbol with index SYMNDX in IBFD.  */
  bfd_vma (*got_elt_size) (bfd *, struct bfd_link_info *,
			   struct elf_link_hash_entry *h,
			   bfd *ibfd, unsigned long symndx);

  /* The vendor name to use for a processor-standard attributes section.  */
  const char *obj_attrs_vendor;

  /* The section name to use for a processor-standard attributes section.  */
  const char *obj_attrs_section;

  /* Return 1, 2 or 3 to indicate what type of arguments a
     processor-specific tag takes.  */
  int (*obj_attrs_arg_type) (int);

  /* The section type to use for an attributes section.  */
  unsigned int obj_attrs_section_type;

  /* This function determines the order in which any attributes are
     written.  It must be defined for input in the range
     LEAST_KNOWN_OBJ_ATTRIBUTE..NUM_KNOWN_OBJ_ATTRIBUTES-1 (this range
     is used in order to make unity easy).  The returned value is the
     actual tag number to place in the input position.  */
  int (*obj_attrs_order) (int);

  /* Handle merging unknown attributes; either warn and return TRUE,
     or give an error and return FALSE.  */
  bool (*obj_attrs_handle_unknown) (bfd *, int);

  /* Parse GNU properties.  Return the property kind.  If the property
     is corrupt, issue an error message and return property_corrupt.  */
  enum elf_property_kind (*parse_gnu_properties) (bfd *, unsigned int,
						  bfd_byte *,
						  unsigned int);

  /* Merge GNU properties.  Return TRUE if property is updated.  */
  bool (*merge_gnu_properties) (struct bfd_link_info *, bfd *, bfd *,
				       elf_property *, elf_property *);

  /* Set up GNU properties.  */
  bfd *(*setup_gnu_properties) (struct bfd_link_info *);

  /* Fix up GNU properties.  */
  void (*fixup_gnu_properties) (struct bfd_link_info *,
				elf_property_list **);

  /* Encoding used for compact EH tables.  */
  int (*compact_eh_encoding) (struct bfd_link_info *);

  /* Opcode representing no unwind.  */
  int (*cant_unwind_opcode) (struct bfd_link_info *);

  /* Called when emitting an ELF symbol whoes input version had an
     ST_SHNDX field set to a value in the range SHN_LOPROC..SHN_HIOS.
     Returns the value to be installed in the ST_SHNDX field of the
     emitted symbol.  If not defined, the value is left unchanged.  */
  unsigned int (*symbol_section_index) (bfd *, elf_symbol_type *);
  
  /* Called when a section has extra reloc sections.  */
  bool (*init_secondary_reloc_section) (bfd *, Elf_Internal_Shdr *,
					const char *, unsigned int);

  /* Called when after loading the normal relocs for a section.  */
  bool (*slurp_secondary_relocs) (bfd *, asection *, asymbol **, bool);

  /* Called after writing the normal relocs for a section.  */
  bool (*write_secondary_relocs) (bfd *, asection *);

  /* This is non-zero if static TLS segments require a special alignment.  */
  unsigned static_tls_alignment;

  /* Alignment for the PT_GNU_STACK segment.  */
  unsigned stack_align;

  /* Flag bits to assign to a section of type SHT_STRTAB.  */
  unsigned long elf_strtab_flags;

  /* This is TRUE if the linker should act like collect and gather
     global constructors and destructors by name.  This is TRUE for
     MIPS ELF because the Irix 5 tools can not handle the .init
     section.  */
  unsigned collect : 1;

  /* This is TRUE if the linker should ignore changes to the type of a
     symbol.  This is TRUE for MIPS ELF because some Irix 5 objects
     record undefined functions as STT_OBJECT although the definitions
     are STT_FUNC.  */
  unsigned type_change_ok : 1;

  /* Whether the backend may use REL relocations.  (Some backends use
     both REL and RELA relocations, and this flag is set for those
     backends.)  */
  unsigned may_use_rel_p : 1;

  /* Whether the backend may use RELA relocations.  (Some backends use
     both REL and RELA relocations, and this flag is set for those
     backends.)  */
  unsigned may_use_rela_p : 1;

  /* Whether the default relocation type is RELA.  If a backend with
     this flag set wants REL relocations for a particular section,
     it must note that explicitly.  Similarly, if this flag is clear,
     and the backend wants RELA relocations for a particular
     section.  */
  unsigned default_use_rela_p : 1;

  /* True if PLT and copy relocations should be RELA by default.  */
  unsigned rela_plts_and_copies_p : 1;

  /* Set if RELA relocations for a relocatable link can be handled by
     generic code.  Backends that set this flag need do nothing in the
     backend relocate_section routine for relocatable linking.  */
  unsigned rela_normal : 1;

  /* Set if DT_REL/DT_RELA/DT_RELSZ/DT_RELASZ should not include PLT
     relocations.  */
  unsigned dtrel_excludes_plt : 1;

  /* TRUE if addresses "naturally" sign extend.  This is used when
     swapping in from Elf32 when BFD64.  */
  unsigned sign_extend_vma : 1;

  unsigned want_got_plt : 1;
  unsigned plt_readonly : 1;
  unsigned want_plt_sym : 1;
  unsigned plt_not_loaded : 1;
  unsigned plt_alignment : 4;
  unsigned can_gc_sections : 1;
  unsigned can_refcount : 1;
  unsigned want_got_sym : 1;
  unsigned want_dynbss : 1;
  unsigned want_dynrelro : 1;

  /* Targets which do not support physical addressing often require
     that the p_paddr field in the section header to be set to zero.
     This field indicates whether this behavior is required.  */
  unsigned want_p_paddr_set_to_zero : 1;

  /* Target has broken hardware and/or kernel that requires pages not
     to be mapped twice with different permissions.  */
  unsigned no_page_alias : 1;

  /* True if an object file lacking a .note.GNU-stack section
     should be assumed to be requesting exec stack.  At least one
     other file in the link needs to have a .note.GNU-stack section
     for a PT_GNU_STACK segment to be created.  */
  unsigned default_execstack : 1;

  /* True if elf_section_data(sec)->this_hdr.contents is sec->rawsize
     in length rather than sec->size in length, if sec->rawsize is
     non-zero and smaller than sec->size.  */
  unsigned caches_rawsize : 1;

  /* Address of protected data defined in the shared library may be
     external, i.e., due to copy relocation.   */
  unsigned extern_protected_data : 1;

  /* True if `_bfd_elf_link_renumber_dynsyms' must be called even for
     static binaries.  */
  unsigned always_renumber_dynsyms : 1;

  /* True if the 32-bit Linux PRPSINFO structure's `pr_uid' and `pr_gid'
     members use a 16-bit data type.  */
  unsigned linux_prpsinfo32_ugid16 : 1;

  /* True if the 64-bit Linux PRPSINFO structure's `pr_uid' and `pr_gid'
     members use a 16-bit data type.  */
  unsigned linux_prpsinfo64_ugid16 : 1;
};

/* Information about reloc sections associated with a bfd_elf_section_data
   structure.  */
struct bfd_elf_section_reloc_data
{
  /* The ELF header for the reloc section associated with this
     section, if any.  */
  Elf_Internal_Shdr *hdr;
  /* The number of relocations currently assigned to HDR.  */
  unsigned int count;
  /* The ELF section number of the reloc section.  Only used for an
     output file.  */
  int idx;
  /* Used by the backend linker to store the symbol hash table entries
     associated with relocs against global symbols.  */
  struct elf_link_hash_entry **hashes;
};

/* Information stored for each BFD section in an ELF file.  This
   structure is allocated by elf_new_section_hook.  */

struct bfd_elf_section_data
{
  /* The ELF header for this section.  */
  Elf_Internal_Shdr this_hdr;

  /* INPUT_SECTION_FLAGS if specified in the linker script.  */
  struct flag_info *section_flag_info;

  /* Information about the REL and RELA reloc sections associated
     with this section, if any.  */
  struct bfd_elf_section_reloc_data rel, rela;

  /* The ELF section number of this section.  */
  int this_idx;

  /* Used by the backend linker when generating a shared library to
     record the dynamic symbol index for a section symbol
     corresponding to this section.  A value of 0 means that there is
     no dynamic symbol for this section.  */
  int dynindx;

  /* A pointer to the linked-to section for SHF_LINK_ORDER.  */
  asection *linked_to;

  /* A pointer to the swapped relocs.  If the section uses REL relocs,
     rather than RELA, all the r_addend fields will be zero.  This
     pointer may be NULL.  It is used by the backend linker.  */
  Elf_Internal_Rela *relocs;

  /* A pointer to a linked list tracking dynamic relocs copied for
     local symbols.  */
  void *local_dynrel;

  /* A pointer to the bfd section used for dynamic relocs.  */
  asection *sreloc;

  union {
    /* Group name, if this section is a member of a group.  */
    const char *name;

    /* Group signature sym, if this is the SHT_GROUP section.  */
    struct bfd_symbol *id;
  } group;

  /* For a member of a group, points to the SHT_GROUP section.
     NULL for the SHT_GROUP section itself and non-group sections.  */
  asection *sec_group;

  /* A linked list of member sections in the group.  Circular when used by
     the linker.  For the SHT_GROUP section, points at first member.  */
  asection *next_in_group;

  /* The FDEs associated with this section.  The u.fde.next_in_section
     field acts as a chain pointer.  */
  struct eh_cie_fde *fde_list;

  /* Link from a text section to its .eh_frame_entry section.  */
  asection *eh_frame_entry;

  /* TRUE if the section has secondary reloc sections associated with it.
     FIXME: In the future it might be better to change this into a list
     of secondary reloc sections, making lookup easier and faster.  */
  bool has_secondary_relocs;

  /* A pointer used for various section optimizations.  */
  void *sec_info;
};

#define elf_section_data(sec) ((struct bfd_elf_section_data*)(sec)->used_by_bfd)
#define elf_linked_to_section(sec) (elf_section_data(sec)->linked_to)
#define elf_section_type(sec)	(elf_section_data(sec)->this_hdr.sh_type)
#define elf_section_flags(sec)	(elf_section_data(sec)->this_hdr.sh_flags)
#define elf_section_info(sec)	(elf_section_data(sec)->this_hdr.sh_info)
#define elf_group_name(sec)	(elf_section_data(sec)->group.name)
#define elf_group_id(sec)	(elf_section_data(sec)->group.id)
#define elf_next_in_group(sec)	(elf_section_data(sec)->next_in_group)
#define elf_fde_list(sec)	(elf_section_data(sec)->fde_list)
#define elf_sec_group(sec)	(elf_section_data(sec)->sec_group)
#define elf_section_eh_frame_entry(sec)	(elf_section_data(sec)->eh_frame_entry)

#define xvec_get_elf_backend_data(xvec) \
  ((const struct elf_backend_data *) (xvec)->backend_data)

#define get_elf_backend_data(abfd) \
   xvec_get_elf_backend_data ((abfd)->xvec)

/* The least object attributes (within an attributes subsection) known
   for any target.  Some code assumes that the value 0 is not used and
   the field for that attribute can instead be used as a marker to
   indicate that attributes have been initialized.  */
#define LEAST_KNOWN_OBJ_ATTRIBUTE 2

/* The maximum number of known object attributes for any target.  */
#define NUM_KNOWN_OBJ_ATTRIBUTES 77

/* The value of an object attribute.  The type indicates whether the attribute
   holds and integer, a string, or both.  It can also indicate that there can
   be no default (i.e. all values must be written to file, even zero), or
   that the value is in error and should not be written to file.  */

typedef struct obj_attribute
{
#define ATTR_TYPE_FLAG_INT_VAL    (1 << 0)
#define ATTR_TYPE_FLAG_STR_VAL    (1 << 1)
#define ATTR_TYPE_FLAG_NO_DEFAULT (1 << 2)
#define ATTR_TYPE_FLAG_ERROR	  (1 << 3)

#define ATTR_TYPE_HAS_INT_VAL(TYPE)	((TYPE) & ATTR_TYPE_FLAG_INT_VAL)
#define ATTR_TYPE_HAS_STR_VAL(TYPE)	((TYPE) & ATTR_TYPE_FLAG_STR_VAL)
#define ATTR_TYPE_HAS_NO_DEFAULT(TYPE)	((TYPE) & ATTR_TYPE_FLAG_NO_DEFAULT)
#define ATTR_TYPE_HAS_ERROR(TYPE)	((TYPE) & ATTR_TYPE_FLAG_ERROR)

  int type;
  unsigned int i;
  char *s;
} obj_attribute;

typedef struct obj_attribute_list
{
  struct obj_attribute_list *next;
  unsigned int tag;
  obj_attribute attr;
} obj_attribute_list;

/* Object attributes may either be defined by the processor ABI, index
   OBJ_ATTR_PROC in the *_obj_attributes arrays, or be GNU-specific
   (and possibly also processor-specific), index OBJ_ATTR_GNU.  */
#define OBJ_ATTR_PROC 0
#define OBJ_ATTR_GNU 1
#define OBJ_ATTR_FIRST OBJ_ATTR_PROC
#define OBJ_ATTR_LAST OBJ_ATTR_GNU

/* The following object attribute tags are taken as generic, for all
   targets and for "gnu" where there is no target standard.  */
enum
{
  Tag_NULL = 0,
  Tag_File = 1,
  Tag_Section = 2,
  Tag_Symbol = 3,
  Tag_compatibility = 32
};

/* The following struct stores information about every SystemTap section
   found in the object file.  */
struct sdt_note
{
  struct sdt_note *next;
  bfd_size_type size;
  bfd_byte data[1];
};

/* tdata information grabbed from an elf core file.  */
struct core_elf_obj_tdata
{
  int signal;
  int pid;
  int lwpid;
  char* program;
  char* command;
};

/* Extra tdata information held for output ELF BFDs.  */
struct output_elf_obj_tdata
{
  struct elf_segment_map *seg_map;
  struct elf_strtab_hash *strtab_ptr;

  /* STT_SECTION symbols for each section */
  asymbol **section_syms;

  /* NT_GNU_BUILD_ID note type info.  */
  struct
  {
    bool (*after_write_object_contents) (bfd *);
    const char *style;
    asection *sec;
  } build_id;

  /* FDO_PACKAGING_METADATA note type info.  */
  struct
  {
    bool (*after_write_object_contents) (bfd *);
    const char *json;
    asection *sec;
  } package_metadata;

  /* Records the result of `get_program_header_size'.  */
  bfd_size_type program_header_size;

  /* Used when laying out sections.  */
  file_ptr next_file_pos;

  /* Linker information.  */
  struct bfd_link_info *link_info;

  unsigned int num_section_syms;
  unsigned int shstrtab_section, strtab_section;

  /* Segment flags for the PT_GNU_STACK segment.  */
  unsigned int stack_flags;

  /* Used to determine if PT_GNU_SFRAME segment header should be
     created.  */
  asection *sframe;

  /* Used to determine if the e_flags field has been initialized */
  bool flags_init;
};

/* Indicate if the bfd contains SHF_GNU_MBIND/SHF_GNU_RETAIN sections or
   symbols that have the STT_GNU_IFUNC symbol type or STB_GNU_UNIQUE
   binding.  Used to set the osabi field in the ELF header structure.  */
enum elf_gnu_osabi
{
  elf_gnu_osabi_mbind = 1 << 0,
  elf_gnu_osabi_ifunc = 1 << 1,
  elf_gnu_osabi_unique = 1 << 2,
  elf_gnu_osabi_retain = 1 << 3,
};

typedef struct elf_section_list
{
  Elf_Internal_Shdr	     hdr;
  unsigned int		     ndx;
  struct elf_section_list *  next;
} elf_section_list;

enum dynamic_lib_link_class {
  DYN_NORMAL = 0,
  DYN_AS_NEEDED = 1,
  DYN_DT_NEEDED = 2,
  DYN_NO_ADD_NEEDED = 4,
  DYN_NO_NEEDED = 8
};

/* Some private data is stashed away for future use using the tdata pointer
   in the bfd structure.  */

struct elf_obj_tdata
{
  Elf_Internal_Ehdr elf_header[1];	/* Actual data, but ref like ptr */
  Elf_Internal_Shdr **elf_sect_ptr;
  Elf_Internal_Phdr *phdr;
  Elf_Internal_Shdr symtab_hdr;
  Elf_Internal_Shdr shstrtab_hdr;
  Elf_Internal_Shdr strtab_hdr;
  Elf_Internal_Shdr dynsymtab_hdr;
  Elf_Internal_Shdr dynstrtab_hdr;
  Elf_Internal_Shdr dynversym_hdr;
  Elf_Internal_Shdr dynverref_hdr;
  Elf_Internal_Shdr dynverdef_hdr;
  Elf_Internal_Sym *dt_symtab;
  bfd_byte *dt_versym;
  bfd_byte *dt_verdef;
  bfd_byte *dt_verneed;
  size_t dt_symtab_count;
  size_t dt_verdef_count;
  size_t dt_verneed_count;
  char *dt_strtab;
  elf_section_list * symtab_shndx_list;
  bfd_vma gp;				/* The gp value */
  unsigned int gp_size;			/* The gp size */
  unsigned int num_elf_sections;	/* elf_sect_ptr size */
  unsigned char *being_created;

  /* A mapping from external symbols to entries in the linker hash
     table, used when linking.  This is indexed by the symbol index
     minus the sh_info field of the symbol table header.  */
  struct elf_link_hash_entry **sym_hashes;

  /* Track usage and final offsets of GOT entries for local symbols.
     This array is indexed by symbol index.  Elements are used
     identically to "got" in struct elf_link_hash_entry.  */
  union
    {
      bfd_signed_vma *refcounts;
      bfd_vma *offsets;
      struct got_entry **ents;
    } local_got;

  /* The linker ELF emulation code needs to let the backend ELF linker
     know what filename should be used for a dynamic object if the
     dynamic object is found using a search.  The emulation code then
     sometimes needs to know what name was actually used.  Until the
     file has been added to the linker symbol table, this field holds
     the name the linker wants.  After it has been added, it holds the
     name actually used, which will be the DT_SONAME entry if there is
     one.  */
  const char *dt_name;

  /* The linker emulation needs to know what audit libs
     are used by a dynamic object.  */
  const char *dt_audit;

  /* Used by find_nearest_line entry point.  */
  void *line_info;

  /* A place to stash dwarf1 info for this bfd.  */
  void *dwarf1_find_line_info;

  /* A place to stash dwarf2 info for this bfd.  */
  void *dwarf2_find_line_info;

  /* Stash away info for yet another find line/function variant.  */
  void *elf_find_function_cache;

  /* Number of symbol version definitions we are about to emit.  */
  unsigned int cverdefs;

  /* Number of symbol version references we are about to emit.  */
  unsigned int cverrefs;

  /* Symbol version definitions in external objects.  */
  Elf_Internal_Verdef *verdef;

  /* Symbol version references to external objects.  */
  Elf_Internal_Verneed *verref;

  /* A pointer to the .eh_frame section.  */
  asection *eh_frame_section;

  /* Symbol buffer.  */
  void *symbuf;

  /* List of GNU properties.  Will be updated by setup_gnu_properties
     after all input GNU properties are merged for output.  */
  elf_property_list *properties;

  obj_attribute known_obj_attributes[2][NUM_KNOWN_OBJ_ATTRIBUTES];
  obj_attribute_list *other_obj_attributes[2];

  /* Linked-list containing information about every Systemtap section
     found in the object file.  Each section corresponds to one entry
     in the list.  */
  struct sdt_note *sdt_note_head;

  Elf_Internal_Shdr **group_sect_ptr;
  unsigned int num_group;

  /* Index into group_sect_ptr, updated by setup_group when finding a
     section's group.  Used to optimize subsequent group searches.  */
  unsigned int group_search_offset;

  unsigned int symtab_section, dynsymtab_section;
  unsigned int dynversym_section, dynverdef_section, dynverref_section;

  /* An identifier used to distinguish different target
     specific extensions to this structure.  */
  ENUM_BITFIELD (elf_target_id) object_id : 6;

  /* Whether a dyanmic object was specified normally on the linker
     command line, or was specified when --as-needed was in effect,
     or was found via a DT_NEEDED entry.  */
  ENUM_BITFIELD (dynamic_lib_link_class) dyn_lib_class : 4;

  /* Whether the bfd uses OS specific bits that require ELFOSABI_GNU.  */
  ENUM_BITFIELD (elf_gnu_osabi) has_gnu_osabi : 4;

  /* Whether if the bfd contains the GNU_PROPERTY_NO_COPY_ON_PROTECTED
     property.  */
  unsigned int has_no_copy_on_protected : 1;

  /* Whether if the bfd contains the
     GNU_PROPERTY_1_NEEDED_INDIRECT_EXTERN_ACCESS property.  */
  unsigned int has_indirect_extern_access : 1;

  /* Irix 5 often screws up the symbol table, sorting local symbols
     after global symbols.  This flag is set if the symbol table in
     this BFD appears to be screwed up.  If it is, we ignore the
     sh_info field in the symbol table header, and always read all the
     symbols.  */
  unsigned int bad_symtab : 1;

  /* Set if DT_FLAGS_1 has DF_1_PIE set.  */
  unsigned int is_pie : 1;

  /* Information grabbed from an elf core file.  */
  struct core_elf_obj_tdata *core;

  /* More information held for output ELF BFDs.  */
  struct output_elf_obj_tdata *o;
};

#define elf_tdata(bfd)		((bfd) -> tdata.elf_obj_data)

#define elf_object_id(bfd)	(elf_tdata(bfd) -> object_id)
#define elf_program_header_size(bfd) (elf_tdata(bfd) -> o->program_header_size)
#define elf_elfheader(bfd)	(elf_tdata(bfd) -> elf_header)
#define elf_elfsections(bfd)	(elf_tdata(bfd) -> elf_sect_ptr)
#define elf_numsections(bfd)	(elf_tdata(bfd) -> num_elf_sections)
#define elf_seg_map(bfd)	(elf_tdata(bfd) -> o->seg_map)
#define elf_link_info(bfd)	(elf_tdata(bfd) -> o->link_info)
#define elf_next_file_pos(bfd)	(elf_tdata(bfd) -> o->next_file_pos)
#define elf_stack_flags(bfd)	(elf_tdata(bfd) -> o->stack_flags)
#define elf_sframe(bfd)		(elf_tdata(bfd) -> o->sframe)
#define elf_shstrtab(bfd)	(elf_tdata(bfd) -> o->strtab_ptr)
#define elf_onesymtab(bfd)	(elf_tdata(bfd) -> symtab_section)
#define elf_symtab_shndx_list(bfd)	(elf_tdata(bfd) -> symtab_shndx_list)
#define elf_strtab_sec(bfd)	(elf_tdata(bfd) -> o->strtab_section)
#define elf_shstrtab_sec(bfd)	(elf_tdata(bfd) -> o->shstrtab_section)
#define elf_symtab_hdr(bfd)	(elf_tdata(bfd) -> symtab_hdr)
#define elf_dynsymtab(bfd)	(elf_tdata(bfd) -> dynsymtab_section)
#define elf_dynversym(bfd)	(elf_tdata(bfd) -> dynversym_section)
#define elf_dynverdef(bfd)	(elf_tdata(bfd) -> dynverdef_section)
#define elf_dynverref(bfd)	(elf_tdata(bfd) -> dynverref_section)
#define elf_eh_frame_section(bfd) \
				(elf_tdata(bfd) -> eh_frame_section)
#define elf_section_syms(bfd)	(elf_tdata(bfd) -> o->section_syms)
#define elf_num_section_syms(bfd) (elf_tdata(bfd) -> o->num_section_syms)
#define core_prpsinfo(bfd)	(elf_tdata(bfd) -> prpsinfo)
#define core_prstatus(bfd)	(elf_tdata(bfd) -> prstatus)
#define elf_gp(bfd)		(elf_tdata(bfd) -> gp)
#define elf_gp_size(bfd)	(elf_tdata(bfd) -> gp_size)
#define elf_sym_hashes(bfd)	(elf_tdata(bfd) -> sym_hashes)
#define elf_local_got_refcounts(bfd) (elf_tdata(bfd) -> local_got.refcounts)
#define elf_local_got_offsets(bfd) (elf_tdata(bfd) -> local_got.offsets)
#define elf_local_got_ents(bfd) (elf_tdata(bfd) -> local_got.ents)
#define elf_dt_name(bfd)	(elf_tdata(bfd) -> dt_name)
#define elf_dt_audit(bfd)	(elf_tdata(bfd) -> dt_audit)
#define elf_dyn_lib_class(bfd)	(elf_tdata(bfd) -> dyn_lib_class)
#define elf_bad_symtab(bfd)	(elf_tdata(bfd) -> bad_symtab)
#define elf_flags_init(bfd)	(elf_tdata(bfd) -> o->flags_init)
#define elf_use_dt_symtab_p(bfd) (elf_tdata(bfd) -> dt_symtab_count != 0)
#define elf_known_obj_attributes(bfd) (elf_tdata (bfd) -> known_obj_attributes)
#define elf_other_obj_attributes(bfd) (elf_tdata (bfd) -> other_obj_attributes)
#define elf_known_obj_attributes_proc(bfd) \
  (elf_known_obj_attributes (bfd) [OBJ_ATTR_PROC])
#define elf_other_obj_attributes_proc(bfd) \
  (elf_other_obj_attributes (bfd) [OBJ_ATTR_PROC])
#define elf_properties(bfd) (elf_tdata (bfd) -> properties)
#define elf_has_no_copy_on_protected(bfd) \
  (elf_tdata(bfd) -> has_no_copy_on_protected)
#define elf_has_indirect_extern_access(bfd) \
  (elf_tdata(bfd) -> has_indirect_extern_access)

extern void _bfd_elf_swap_verdef_in
  (bfd *, const Elf_External_Verdef *, Elf_Internal_Verdef *);
extern void _bfd_elf_swap_verdef_out
  (bfd *, const Elf_Internal_Verdef *, Elf_External_Verdef *);
extern void _bfd_elf_swap_verdaux_in
  (bfd *, const Elf_External_Verdaux *, Elf_Internal_Verdaux *);
extern void _bfd_elf_swap_verdaux_out
  (bfd *, const Elf_Internal_Verdaux *, Elf_External_Verdaux *);
extern void _bfd_elf_swap_verneed_in
  (bfd *, const Elf_External_Verneed *, Elf_Internal_Verneed *);
extern void _bfd_elf_swap_verneed_out
  (bfd *, const Elf_Internal_Verneed *, Elf_External_Verneed *);
extern void _bfd_elf_swap_vernaux_in
  (bfd *, const Elf_External_Vernaux *, Elf_Internal_Vernaux *);
extern void _bfd_elf_swap_vernaux_out
  (bfd *, const Elf_Internal_Vernaux *, Elf_External_Vernaux *);
extern void _bfd_elf_swap_versym_in
  (bfd *, const Elf_External_Versym *, Elf_Internal_Versym *);
extern void _bfd_elf_swap_versym_out
  (bfd *, const Elf_Internal_Versym *, Elf_External_Versym *);

extern unsigned int _bfd_elf_section_from_bfd_section
  (bfd *, asection *);
extern char *bfd_elf_string_from_elf_section
  (bfd *, unsigned, unsigned);
extern Elf_Internal_Sym *bfd_elf_get_elf_syms
  (bfd *, Elf_Internal_Shdr *, size_t, size_t, Elf_Internal_Sym *, void *,
   Elf_External_Sym_Shndx *);
extern char * bfd_elf_get_str_section (bfd *, unsigned int);
extern const char *bfd_elf_sym_name
  (bfd *, Elf_Internal_Shdr *, Elf_Internal_Sym *, asection *);

extern bool _bfd_elf_copy_private_bfd_data
  (bfd *, bfd *);
extern bool _bfd_elf_print_private_bfd_data
  (bfd *, void *);
const char * _bfd_elf_get_symbol_version_string
  (bfd *, asymbol *, bool, bool *);
extern void bfd_elf_print_symbol
  (bfd *, void *, asymbol *, bfd_print_symbol_type);

extern unsigned int _bfd_elf_eh_frame_address_size
  (bfd *, const asection *);
extern bfd_byte _bfd_elf_encode_eh_address
  (bfd *abfd, struct bfd_link_info *info, asection *osec, bfd_vma offset,
   asection *loc_sec, bfd_vma loc_offset, bfd_vma *encoded);
extern bool _bfd_elf_can_make_relative
  (bfd *input_bfd, struct bfd_link_info *info, asection *eh_frame_section);

extern enum elf_reloc_type_class _bfd_elf_reloc_type_class
  (const struct bfd_link_info *, const asection *,
   const Elf_Internal_Rela *);
extern bfd_vma _bfd_elf_rela_local_sym
  (bfd *, Elf_Internal_Sym *, asection **, Elf_Internal_Rela *);
extern bfd_vma _bfd_elf_rel_local_sym
  (bfd *, Elf_Internal_Sym *, asection **, bfd_vma);
extern bfd_vma _bfd_elf_section_offset
  (bfd *, struct bfd_link_info *, asection *, bfd_vma);

extern unsigned long bfd_elf_hash
  (const char *);
extern unsigned long bfd_elf_gnu_hash
  (const char *);

extern bfd_reloc_status_type bfd_elf_generic_reloc
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);
extern bool bfd_elf_allocate_object
  (bfd *, size_t, enum elf_target_id);
extern bool bfd_elf_make_object
  (bfd *);
extern bool bfd_elf_mkcorefile
  (bfd *);
extern bool _bfd_elf_make_section_from_shdr
  (bfd *, Elf_Internal_Shdr *, const char *, int);
extern bool _bfd_elf_make_section_from_phdr
  (bfd *, Elf_Internal_Phdr *, int, const char *);
extern struct bfd_hash_entry *_bfd_elf_link_hash_newfunc
  (struct bfd_hash_entry *, struct bfd_hash_table *, const char *);
extern struct bfd_link_hash_table *_bfd_elf_link_hash_table_create
  (bfd *);
extern void _bfd_elf_link_hash_table_free
  (bfd *);
extern void _bfd_elf_link_hash_copy_indirect
  (struct bfd_link_info *, struct elf_link_hash_entry *,
   struct elf_link_hash_entry *);
extern void _bfd_elf_link_hash_hide_symbol
  (struct bfd_link_info *, struct elf_link_hash_entry *, bool);
extern void _bfd_elf_link_hide_symbol
  (bfd *, struct bfd_link_info *, struct bfd_link_hash_entry *);
extern bool _bfd_elf_link_hash_fixup_symbol
  (struct bfd_link_info *, struct elf_link_hash_entry *);
extern bool _bfd_elf_link_hash_table_init
  (struct elf_link_hash_table *, bfd *,
   struct bfd_hash_entry *(*)
     (struct bfd_hash_entry *, struct bfd_hash_table *, const char *),
   unsigned int, enum elf_target_id);
extern bool _bfd_elf_slurp_version_tables
  (bfd *, bool);
extern bool _bfd_elf_merge_sections
  (bfd *, struct bfd_link_info *);
extern bool _bfd_elf_match_sections_by_type
  (bfd *, const asection *, bfd *, const asection *);
extern bool bfd_elf_is_group_section
  (bfd *, const struct bfd_section *);
extern const char *bfd_elf_group_name
  (bfd *, const struct bfd_section *);
extern bool _bfd_elf_section_already_linked
  (bfd *, asection *, struct bfd_link_info *);
extern void bfd_elf_set_group_contents
  (bfd *, asection *, void *);
extern unsigned int _bfd_elf_filter_global_symbols
  (bfd *, struct bfd_link_info *, asymbol **, long);
extern asection *_bfd_elf_check_kept_section
  (asection *, struct bfd_link_info *);
#define _bfd_elf_link_just_syms _bfd_generic_link_just_syms
extern void _bfd_elf_copy_link_hash_symbol_type
  (bfd *, struct bfd_link_hash_entry *, struct bfd_link_hash_entry *);
extern bool _bfd_elf_size_group_sections
  (struct bfd_link_info *);
extern bool _bfd_elf_fixup_group_sections
(bfd *, asection *);
extern bool _bfd_elf_copy_private_header_data
  (bfd *, bfd *);
extern bool _bfd_elf_copy_private_symbol_data
  (bfd *, asymbol *, bfd *, asymbol *);
#define _bfd_generic_init_private_section_data \
  _bfd_elf_init_private_section_data
extern bool _bfd_elf_init_private_section_data
  (bfd *, asection *, bfd *, asection *, struct bfd_link_info *);
extern bool _bfd_elf_copy_private_section_data
  (bfd *, asection *, bfd *, asection *);
extern bool _bfd_elf_write_object_contents
  (bfd *);
extern bool _bfd_elf_write_corefile_contents
  (bfd *);
extern bool _bfd_elf_set_section_contents
  (bfd *, sec_ptr, const void *, file_ptr, bfd_size_type);
extern long _bfd_elf_get_symtab_upper_bound
  (bfd *);
extern long _bfd_elf_canonicalize_symtab
  (bfd *, asymbol **);
extern long _bfd_elf_get_dynamic_symtab_upper_bound
  (bfd *);
extern long _bfd_elf_canonicalize_dynamic_symtab
  (bfd *, asymbol **);
extern long _bfd_elf_get_synthetic_symtab
  (bfd *, long, asymbol **, long, asymbol **, asymbol **);
extern long _bfd_elf_get_reloc_upper_bound
  (bfd *, sec_ptr);
extern long _bfd_elf_canonicalize_reloc
  (bfd *, sec_ptr, arelent **, asymbol **);
extern asection * _bfd_elf_get_dynamic_reloc_section
  (bfd *, asection *, bool);
extern asection * _bfd_elf_make_dynamic_reloc_section
  (asection *, bfd *, unsigned int, bfd *, bool);
extern long _bfd_elf_get_dynamic_reloc_upper_bound
  (bfd *);
extern long _bfd_elf_canonicalize_dynamic_reloc
  (bfd *, arelent **, asymbol **);
extern asymbol *_bfd_elf_make_empty_symbol
  (bfd *);
extern void _bfd_elf_get_symbol_info
  (bfd *, asymbol *, symbol_info *);
extern bool _bfd_elf_is_local_label_name
  (bfd *, const char *);
extern alent *_bfd_elf_get_lineno
  (bfd *, asymbol *);
extern bool _bfd_elf_set_arch_mach
  (bfd *, enum bfd_architecture, unsigned long);
extern bool _bfd_elf_find_nearest_line
  (bfd *, asymbol **, asection *, bfd_vma,
   const char **, const char **, unsigned int *, unsigned int *);
extern bool _bfd_elf_find_nearest_line_with_alt
  (bfd *, const char *, asymbol **, asection *, bfd_vma,
   const char **, const char **, unsigned int *, unsigned int *);
extern bool _bfd_elf_find_line
  (bfd *, asymbol **, asymbol *, const char **, unsigned int *);
extern bool _bfd_elf_find_inliner_info
  (bfd *, const char **, const char **, unsigned int *);
extern asymbol *_bfd_elf_find_function
  (bfd *, asymbol **, asection *, bfd_vma, const char **, const char **);
#define _bfd_elf_read_minisymbols _bfd_generic_read_minisymbols
#define _bfd_elf_minisymbol_to_symbol _bfd_generic_minisymbol_to_symbol
extern int _bfd_elf_sizeof_headers
  (bfd *, struct bfd_link_info *);
extern bool _bfd_elf_new_section_hook
  (bfd *, asection *);
extern const struct bfd_elf_special_section *_bfd_elf_get_special_section
  (const char *, const struct bfd_elf_special_section *, unsigned int);
extern const struct bfd_elf_special_section *_bfd_elf_get_sec_type_attr
  (bfd *, asection *);

extern bool _bfd_elf_link_hide_sym_by_version
  (struct bfd_link_info *, struct elf_link_hash_entry *);

/* If the target doesn't have reloc handling written yet:  */
extern bool _bfd_elf_no_info_to_howto
  (bfd *, arelent *, Elf_Internal_Rela *);

extern bool bfd_section_from_shdr
  (bfd *, unsigned int shindex);
extern bool bfd_section_from_phdr
  (bfd *, Elf_Internal_Phdr *, int);

extern int _bfd_elf_symbol_from_bfd_symbol
  (bfd *, asymbol **);

extern Elf_Internal_Sym *bfd_sym_from_r_symndx
  (struct sym_cache *, bfd *, unsigned long);
extern asection *bfd_section_from_elf_index
  (bfd *, unsigned int);

extern struct elf_strtab_hash * _bfd_elf_strtab_init
  (void);
extern void _bfd_elf_strtab_free
  (struct elf_strtab_hash *);
extern size_t _bfd_elf_strtab_add
  (struct elf_strtab_hash *, const char *, bool);
extern void _bfd_elf_strtab_addref
  (struct elf_strtab_hash *, size_t);
extern void _bfd_elf_strtab_delref
  (struct elf_strtab_hash *, size_t);
extern unsigned int _bfd_elf_strtab_refcount
  (struct elf_strtab_hash *, size_t);
extern void _bfd_elf_strtab_clear_all_refs
  (struct elf_strtab_hash *);
extern void *_bfd_elf_strtab_save
  (struct elf_strtab_hash *);
extern void _bfd_elf_strtab_restore
  (struct elf_strtab_hash *, void *);
extern bfd_size_type _bfd_elf_strtab_size
  (struct elf_strtab_hash *);
extern bfd_size_type _bfd_elf_strtab_len
  (struct elf_strtab_hash *);
extern bfd_size_type _bfd_elf_strtab_offset
  (struct elf_strtab_hash *, size_t);
extern const char * _bfd_elf_strtab_str
  (struct elf_strtab_hash *, size_t idx, bfd_size_type *offset);
extern bool _bfd_elf_strtab_emit
  (bfd *, struct elf_strtab_hash *);
extern void _bfd_elf_strtab_finalize
  (struct elf_strtab_hash *);

extern bool bfd_elf_parse_eh_frame_entries
  (bfd *, struct bfd_link_info *);
extern bool _bfd_elf_parse_eh_frame_entry
  (struct bfd_link_info *, asection *, struct elf_reloc_cookie *);
extern void _bfd_elf_parse_eh_frame
  (bfd *, struct bfd_link_info *, asection *, struct elf_reloc_cookie *);
extern bool _bfd_elf_end_eh_frame_parsing
  (struct bfd_link_info *info);

extern bool _bfd_elf_discard_section_eh_frame
  (bfd *, struct bfd_link_info *, asection *,
   bool (*) (bfd_vma, void *), struct elf_reloc_cookie *);
extern bool _bfd_elf_adjust_eh_frame_global_symbol
  (struct elf_link_hash_entry *, void *);
extern bool _bfd_elf_discard_section_eh_frame_hdr
  (struct bfd_link_info *);
extern bfd_vma _bfd_elf_eh_frame_section_offset
  (bfd *, struct bfd_link_info *, asection *, bfd_vma);
extern bool _bfd_elf_write_section_eh_frame
  (bfd *, struct bfd_link_info *, asection *, bfd_byte *);
bool _bfd_elf_write_section_eh_frame_entry
  (bfd *, struct bfd_link_info *, asection *, bfd_byte *);
extern bool _bfd_elf_fixup_eh_frame_hdr (struct bfd_link_info *);
extern bool _bfd_elf_write_section_eh_frame_hdr
  (bfd *, struct bfd_link_info *);
extern bool _bfd_elf_eh_frame_present
  (struct bfd_link_info *);
extern bool _bfd_elf_eh_frame_entry_present
  (struct bfd_link_info *);
extern bool _bfd_elf_maybe_strip_eh_frame_hdr
  (struct bfd_link_info *);

extern bool _bfd_elf_sframe_present
  (struct bfd_link_info *);
extern bool _bfd_elf_parse_sframe
  (bfd *, struct bfd_link_info *, asection *, struct elf_reloc_cookie *);
extern bool _bfd_elf_discard_section_sframe
  (asection *, bool (*) (bfd_vma, void *), struct elf_reloc_cookie *);
extern bool _bfd_elf_merge_section_sframe
  (bfd *, struct bfd_link_info *, asection *, bfd_byte *);
extern bool _bfd_elf_write_section_sframe
  (bfd *, struct bfd_link_info *);
extern bool _bfd_elf_set_section_sframe (bfd *, struct bfd_link_info *);

extern bool _bfd_elf_hash_symbol (struct elf_link_hash_entry *);

extern long _bfd_elf_link_lookup_local_dynindx
  (struct bfd_link_info *, bfd *, long);
extern bool _bfd_elf_compute_section_file_positions
  (bfd *, struct bfd_link_info *);
extern file_ptr _bfd_elf_assign_file_position_for_section
  (Elf_Internal_Shdr *, file_ptr, bool);
extern bool _bfd_elf_modify_headers
  (bfd *, struct bfd_link_info *);

extern bool _bfd_elf_validate_reloc
  (bfd *, arelent *);

extern bool bfd_elf_record_link_assignment
  (bfd *, struct bfd_link_info *, const char *, bool,
   bool);
extern bool bfd_elf_stack_segment_size (bfd *, struct bfd_link_info *,
					const char *, bfd_vma);
extern bool bfd_elf_size_dynamic_sections
  (bfd *, const char *, const char *, const char *, const char *, const char *,
   const char * const *, struct bfd_link_info *, struct bfd_section **);
extern bool bfd_elf_size_dynsym_hash_dynstr
  (bfd *, struct bfd_link_info *);
extern bool bfd_elf_get_bfd_needed_list
  (bfd *, struct bfd_link_needed_list **);
extern struct bfd_link_needed_list *bfd_elf_get_needed_list
  (bfd *, struct bfd_link_info *);
extern void bfd_elf_set_dt_needed_name
  (bfd *, const char *);
extern const char *bfd_elf_get_dt_soname
  (bfd *);
extern void bfd_elf_set_dyn_lib_class
  (bfd *, enum dynamic_lib_link_class);
extern int bfd_elf_get_dyn_lib_class
  (bfd *);
extern struct bfd_link_needed_list *bfd_elf_get_runpath_list
  (bfd *, struct bfd_link_info *);
extern int bfd_elf_discard_info
  (bfd *, struct bfd_link_info *);
extern unsigned int _bfd_elf_default_action_discarded
  (struct bfd_section *);
extern struct bfd_section *_bfd_elf_tls_setup
  (bfd *, struct bfd_link_info *);

extern bool _bfd_elf_link_create_dynamic_sections
  (bfd *, struct bfd_link_info *);
extern bool _bfd_elf_omit_section_dynsym_default
  (bfd *, struct bfd_link_info *, asection *);
extern bool _bfd_elf_omit_section_dynsym_all
  (bfd *, struct bfd_link_info *, asection *);
extern bool _bfd_elf_create_dynamic_sections
  (bfd *, struct bfd_link_info *);
extern bool _bfd_elf_create_got_section
  (bfd *, struct bfd_link_info *);
extern asection *_bfd_elf_section_for_symbol
  (struct elf_reloc_cookie *, unsigned long, bool);
extern struct elf_link_hash_entry *_bfd_elf_define_linkage_sym
  (bfd *, struct bfd_link_info *, asection *, const char *);
extern void _bfd_elf_init_1_index_section
  (bfd *, struct bfd_link_info *);
extern void _bfd_elf_init_2_index_sections
  (bfd *, struct bfd_link_info *);

extern bool _bfd_elfcore_make_pseudosection
  (bfd *, char *, size_t, ufile_ptr);
extern char *_bfd_elfcore_strndup
  (bfd *, char *, size_t);

extern Elf_Internal_Rela *_bfd_elf_link_read_relocs
  (bfd *, asection *, void *, Elf_Internal_Rela *, bool);
extern Elf_Internal_Rela *_bfd_elf_link_info_read_relocs
  (bfd *, struct bfd_link_info *, asection *, void *, Elf_Internal_Rela *,
   bool);

extern bool _bfd_elf_link_output_relocs
  (bfd *, asection *, Elf_Internal_Shdr *, Elf_Internal_Rela *,
   struct elf_link_hash_entry **);

extern bool _bfd_elf_adjust_dynamic_copy
  (struct bfd_link_info *, struct elf_link_hash_entry *, asection *);

extern bool _bfd_elf_dynamic_symbol_p
  (struct elf_link_hash_entry *, struct bfd_link_info *, bool);

extern bool _bfd_elf_symbol_refs_local_p
  (struct elf_link_hash_entry *, struct bfd_link_info *, bool);

extern bfd_reloc_status_type bfd_elf_perform_complex_relocation
  (bfd *, asection *, bfd_byte *, Elf_Internal_Rela *, bfd_vma);

extern bool _bfd_elf_setup_sections
  (bfd *);

extern bool _bfd_elf_get_dynamic_symbols
  (bfd *, Elf_Internal_Phdr *, Elf_Internal_Phdr *, size_t,
   bfd_size_type);
extern asection *_bfd_elf_get_section_from_dynamic_symbol
  (bfd *, Elf_Internal_Sym *);

extern struct bfd_link_hash_entry *bfd_elf_define_start_stop
  (struct bfd_link_info *, const char *, asection *);

extern bool _bfd_elf_init_file_header (bfd *, struct bfd_link_info *);

extern bool _bfd_elf_final_write_processing (bfd *);

extern bfd_cleanup bfd_elf32_object_p
  (bfd *);
extern bfd_cleanup bfd_elf32_core_file_p
  (bfd *);
extern char *bfd_elf32_core_file_failing_command
  (bfd *);
extern int bfd_elf32_core_file_failing_signal
  (bfd *);
extern bool bfd_elf32_core_file_matches_executable_p
  (bfd *, bfd *);
extern int bfd_elf32_core_file_pid
  (bfd *);
extern bool _bfd_elf32_core_find_build_id
  (bfd *, bfd_vma);

extern bool bfd_elf32_swap_symbol_in
  (bfd *, const void *, const void *, Elf_Internal_Sym *);
extern void bfd_elf32_swap_symbol_out
  (bfd *, const Elf_Internal_Sym *, void *, void *);
extern void bfd_elf32_swap_reloc_in
  (bfd *, const bfd_byte *, Elf_Internal_Rela *);
extern void bfd_elf32_swap_reloc_out
  (bfd *, const Elf_Internal_Rela *, bfd_byte *);
extern void bfd_elf32_swap_reloca_in
  (bfd *, const bfd_byte *, Elf_Internal_Rela *);
extern void bfd_elf32_swap_reloca_out
  (bfd *, const Elf_Internal_Rela *, bfd_byte *);
extern void bfd_elf32_swap_phdr_in
  (bfd *, const Elf32_External_Phdr *, Elf_Internal_Phdr *);
extern void bfd_elf32_swap_phdr_out
  (bfd *, const Elf_Internal_Phdr *, Elf32_External_Phdr *);
extern void bfd_elf32_swap_dyn_in
  (bfd *, const void *, Elf_Internal_Dyn *);
extern void bfd_elf32_swap_dyn_out
  (bfd *, const Elf_Internal_Dyn *, void *);
extern long bfd_elf32_slurp_symbol_table
  (bfd *, asymbol **, bool);
extern bool bfd_elf32_write_shdrs_and_ehdr
  (bfd *);
extern int bfd_elf32_write_out_phdrs
  (bfd *, const Elf_Internal_Phdr *, unsigned int);
extern bool bfd_elf32_checksum_contents
  (bfd * , void (*) (const void *, size_t, void *), void *);
extern void bfd_elf32_write_relocs
  (bfd *, asection *, void *);
extern bool bfd_elf32_slurp_reloc_table
  (bfd *, asection *, asymbol **, bool);

extern bfd_cleanup bfd_elf64_object_p
  (bfd *);
extern bfd_cleanup bfd_elf64_core_file_p
  (bfd *);
extern char *bfd_elf64_core_file_failing_command
  (bfd *);
extern int bfd_elf64_core_file_failing_signal
  (bfd *);
extern bool bfd_elf64_core_file_matches_executable_p
  (bfd *, bfd *);
extern int bfd_elf64_core_file_pid
  (bfd *);
extern bool _bfd_elf64_core_find_build_id
  (bfd *, bfd_vma);

extern bool bfd_elf64_swap_symbol_in
  (bfd *, const void *, const void *, Elf_Internal_Sym *);
extern void bfd_elf64_swap_symbol_out
  (bfd *, const Elf_Internal_Sym *, void *, void *);
extern void bfd_elf64_swap_reloc_in
  (bfd *, const bfd_byte *, Elf_Internal_Rela *);
extern void bfd_elf64_swap_reloc_out
  (bfd *, const Elf_Internal_Rela *, bfd_byte *);
extern void bfd_elf64_swap_reloca_in
  (bfd *, const bfd_byte *, Elf_Internal_Rela *);
extern void bfd_elf64_swap_reloca_out
  (bfd *, const Elf_Internal_Rela *, bfd_byte *);
extern void bfd_elf64_swap_phdr_in
  (bfd *, const Elf64_External_Phdr *, Elf_Internal_Phdr *);
extern void bfd_elf64_swap_phdr_out
  (bfd *, const Elf_Internal_Phdr *, Elf64_External_Phdr *);
extern void bfd_elf64_swap_dyn_in
  (bfd *, const void *, Elf_Internal_Dyn *);
extern void bfd_elf64_swap_dyn_out
  (bfd *, const Elf_Internal_Dyn *, void *);
extern long bfd_elf64_slurp_symbol_table
  (bfd *, asymbol **, bool);
extern bool bfd_elf64_write_shdrs_and_ehdr
  (bfd *);
extern int bfd_elf64_write_out_phdrs
  (bfd *, const Elf_Internal_Phdr *, unsigned int);
extern bool bfd_elf64_checksum_contents
  (bfd * , void (*) (const void *, size_t, void *), void *);
extern void bfd_elf64_write_relocs
  (bfd *, asection *, void *);
extern bool bfd_elf64_slurp_reloc_table
  (bfd *, asection *, asymbol **, bool);

extern bool _bfd_elf_default_relocs_compatible
  (const bfd_target *, const bfd_target *);

extern bool _bfd_elf_relocs_compatible
  (const bfd_target *, const bfd_target *);
extern bool _bfd_elf_notice_as_needed
  (bfd *, struct bfd_link_info *, enum notice_asneeded_action);

extern struct bfd_link_hash_entry *_bfd_elf_archive_symbol_lookup
  (bfd *, struct bfd_link_info *, const char *);
extern bool bfd_elf_link_add_symbols
  (bfd *, struct bfd_link_info *);
extern bool _bfd_elf_add_dynamic_entry
  (struct bfd_link_info *, bfd_vma, bfd_vma);
extern bool _bfd_elf_strip_zero_sized_dynamic_sections
  (struct bfd_link_info *);
extern int bfd_elf_add_dt_needed_tag
  (bfd *, struct bfd_link_info *);
extern bool _bfd_elf_link_check_relocs
  (bfd *, struct bfd_link_info *);
extern bool _bfd_elf_link_iterate_on_relocs
 (bfd *, struct bfd_link_info *,
  bool (*) (bfd *, struct bfd_link_info *, asection *,
	    const Elf_Internal_Rela *));

extern bool bfd_elf_link_record_dynamic_symbol
  (struct bfd_link_info *, struct elf_link_hash_entry *);

extern int bfd_elf_link_record_local_dynamic_symbol
  (struct bfd_link_info *, bfd *, long);

extern bool _bfd_elf_free_cached_info
  (bfd *);

extern bool _bfd_elf_common_definition
  (Elf_Internal_Sym *);

extern unsigned int _bfd_elf_common_section_index
  (asection *);

extern asection *_bfd_elf_common_section
  (asection *);

extern bfd_vma _bfd_elf_default_got_elt_size
(bfd *, struct bfd_link_info *, struct elf_link_hash_entry *, bfd *,
 unsigned long);

extern bfd_reloc_status_type _bfd_elf_rel_vtable_reloc_fn
  (bfd *, arelent *, struct bfd_symbol *, void *,
   asection *, bfd *, char **);

extern bool bfd_elf_final_link
  (bfd *, struct bfd_link_info *);

extern void _bfd_elf_gc_keep
  (struct bfd_link_info *info);

extern bool bfd_elf_gc_mark_dynamic_ref_symbol
  (struct elf_link_hash_entry *h, void *inf);

extern bool bfd_elf_gc_sections
  (bfd *, struct bfd_link_info *);

extern bool bfd_elf_gc_record_vtinherit
  (bfd *, asection *, struct elf_link_hash_entry *, bfd_vma);

extern bool bfd_elf_gc_record_vtentry
  (bfd *, asection *, struct elf_link_hash_entry *, bfd_vma);

extern asection *_bfd_elf_gc_mark_hook
  (asection *, struct bfd_link_info *, Elf_Internal_Rela *,
   struct elf_link_hash_entry *, Elf_Internal_Sym *);

extern asection *_bfd_elf_gc_mark_rsec
  (struct bfd_link_info *, asection *, elf_gc_mark_hook_fn,
   struct elf_reloc_cookie *, bool *);

extern bool _bfd_elf_gc_mark_reloc
  (struct bfd_link_info *, asection *, elf_gc_mark_hook_fn,
   struct elf_reloc_cookie *);

extern bool _bfd_elf_gc_mark_fdes
  (struct bfd_link_info *, asection *, asection *, elf_gc_mark_hook_fn,
   struct elf_reloc_cookie *);

extern bool _bfd_elf_gc_mark
  (struct bfd_link_info *, asection *, elf_gc_mark_hook_fn);

extern bool _bfd_elf_gc_mark_extra_sections
  (struct bfd_link_info *, elf_gc_mark_hook_fn);

extern bool bfd_elf_gc_common_finalize_got_offsets
  (bfd *, struct bfd_link_info *);

extern bool bfd_elf_gc_common_final_link
  (bfd *, struct bfd_link_info *);

extern bool bfd_elf_reloc_symbol_deleted_p
  (bfd_vma, void *);

extern struct elf_segment_map * _bfd_elf_make_dynamic_segment
  (bfd *, asection *);

extern bool _bfd_elf_map_sections_to_segments
  (bfd *, struct bfd_link_info *, bool *);

extern bool _bfd_elf_is_function_type (unsigned int);

extern bfd_size_type _bfd_elf_maybe_function_sym (const asymbol *, asection *,
						  bfd_vma *);

extern asection *_bfd_elf_plt_get_reloc_section (bfd *, const char *);

extern int bfd_elf_get_default_section_type (flagword);

extern bool bfd_elf_lookup_section_flags
  (struct bfd_link_info *, struct flag_info *, asection *);

extern Elf_Internal_Phdr * _bfd_elf_find_segment_containing_section
  (bfd * abfd, asection * section);

/* PowerPC @tls opcode transform/validate.  */
extern unsigned int _bfd_elf_ppc_at_tls_transform
  (unsigned int, unsigned int);
/* PowerPC @tprel opcode transform/validate.  */
extern unsigned int _bfd_elf_ppc_at_tprel_transform
  (unsigned int, unsigned int);
/* PowerPC elf_object_p tweak.  */
extern bool _bfd_elf_ppc_set_arch (bfd *);
/* PowerPC .gnu.attributes handling common to both 32-bit and 64-bit.  */
extern bool _bfd_elf_ppc_merge_fp_attributes
  (bfd *, struct bfd_link_info *);

/* Return an upper bound on the number of bytes required to store a
   copy of ABFD's program header table entries.  Return -1 if an error
   occurs; bfd_get_error will return an appropriate code.  */
extern long bfd_get_elf_phdr_upper_bound
  (bfd *abfd);

/* Copy ABFD's program header table entries to *PHDRS.  The entries
   will be stored as an array of Elf_Internal_Phdr structures, as
   defined in include/elf/internal.h.  To find out how large the
   buffer needs to be, call bfd_get_elf_phdr_upper_bound.

   Return the number of program header table entries read, or -1 if an
   error occurs; bfd_get_error will return an appropriate code.  */
extern int bfd_get_elf_phdrs
  (bfd *abfd, void *phdrs);

/* Exported interface for writing elf corefile notes.  */
extern char *elfcore_write_note
  (bfd *, char *, int *, const char *, int, const void *, int);
extern char *elfcore_write_prpsinfo
  (bfd *, char *, int *, const char *, const char *);
extern char *elfcore_write_prstatus
  (bfd *, char *, int *, long, int, const void *);
extern char * elfcore_write_pstatus
  (bfd *, char *, int *, long, int, const void *);
extern char *elfcore_write_prfpreg
  (bfd *, char *, int *, const void *, int);
extern char *elfcore_write_prxfpreg
  (bfd *, char *, int *, const void *, int);
extern char *elfcore_write_xstatereg
  (bfd *, char *, int *, const void *, int);
extern char *elfcore_write_x86_segbases
  (bfd *, char *, int *, const void *, int);
extern char *elfcore_write_ppc_vmx
  (bfd *, char *, int *, const void *, int);
extern char *elfcore_write_ppc_vsx
  (bfd *, char *, int *, const void *, int);
extern char *elfcore_write_ppc_tar
  (bfd *, char *, int *, const void *, int);
extern char *elfcore_write_ppc_ppr
  (bfd *, char *, int *, const void *, int);
extern char *elfcore_write_ppc_dscr
  (bfd *, char *, int *, const void *, int);
extern char *elfcore_write_ppc_ebb
  (bfd *, char *, int *, const void *, int);
extern char *elfcore_write_ppc_pmu
  (bfd *, char *, int *, const void *, int);
extern char *elfcore_write_ppc_tm_cgpr
  (bfd *, char *, int *, const void *, int);
extern char *elfcore_write_ppc_tm_cfpr
  (bfd *, char *, int *, const void *, int);
extern char *elfcore_write_ppc_tm_cvmx
  (bfd *, char *, int *, const void *, int);
extern char *elfcore_write_ppc_tm_cvsx
  (bfd *, char *, int *, const void *, int);
extern char *elfcore_write_ppc_tm_spr
  (bfd *, char *, int *, const void *, int);
extern char *elfcore_write_ppc_tm_ctar
  (bfd *, char *, int *, const void *, int);
extern char *elfcore_write_ppc_tm_cppr
  (bfd *, char *, int *, const void *, int);
extern char *elfcore_write_ppc_tm_cdscr
  (bfd *, char *, int *, const void *, int);
extern char *elfcore_write_s390_timer
  (bfd *, char *, int *, const void *, int);
extern char *elfcore_write_s390_todcmp
  (bfd *, char *, int *, const void *, int);
extern char *elfcore_write_s390_todpreg
  (bfd *, char *, int *, const void *, int);
extern char *elfcore_write_s390_ctrs
  (bfd *, char *, int *, const void *, int);
extern char *elfcore_write_s390_prefix
  (bfd *, char *, int *, const void *, int);
extern char *elfcore_write_s390_last_break
  (bfd *, char *, int *, const void *, int);
extern char *elfcore_write_s390_system_call
  (bfd *, char *, int *, const void *, int);
extern char *elfcore_write_s390_tdb
  (bfd *, char *, int *, const void *, int);
extern char *elfcore_write_s390_vxrs_low
  (bfd *, char *, int *, const void *, int);
extern char *elfcore_write_s390_vxrs_high
  (bfd *, char *, int *, const void *, int);
extern char *elfcore_write_s390_gs_cb
  (bfd *, char *, int *, const void *, int);
extern char *elfcore_write_s390_gs_bc
  (bfd *, char *, int *, const void *, int);
extern char *elfcore_write_arm_vfp
  (bfd *, char *, int *, const void *, int);
extern char *elfcore_write_aarch_tls
  (bfd *, char *, int *, const void *, int);
extern char *elfcore_write_aarch_hw_break
  (bfd *, char *, int *, const void *, int);
extern char *elfcore_write_aarch_hw_watch
  (bfd *, char *, int *, const void *, int);
extern char *elfcore_write_aarch_sve
  (bfd *, char *, int *, const void *, int);
extern char *elfcore_write_aarch_pauth
  (bfd *, char *, int *, const void *, int);
extern char *elfcore_write_aarch_mte
  (bfd *, char *, int *, const void *, int);
extern char *elfcore_write_arc_v2
  (bfd *, char *, int *, const void *, int);
extern char *elfcore_write_riscv_csr
  (bfd *, char *, int *, const void *, int);
extern char *elfcore_write_gdb_tdesc
  (bfd *, char *, int *, const void *, int);
extern char *elfcore_write_lwpstatus
  (bfd *, char *, int *, long, int, const void *);
extern char *elfcore_write_register_note
  (bfd *, char *, int *, const char *, const void *, int);
extern char *elfcore_write_file_note
  (bfd *, char *, int *, const void*, int);
extern char *elfcore_write_loongarch_cpucfg
  (bfd *, char *, int *, const void*, int);
extern char *elfcore_write_loongarch_lbt
  (bfd *, char *, int *, const void*, int);
extern char *elfcore_write_loongarch_lsx
  (bfd *, char *, int *, const void*, int);
extern char *elfcore_write_loongarch_lasx
  (bfd *, char *, int *, const void*, int);

/* Internal structure which holds information to be included in the
   PRPSINFO section of Linux core files.

   This is an "internal" structure in the sense that it should be used
   to pass information to BFD (via the `elfcore_write_linux_prpsinfo'
   function), so things like endianess shouldn't be an issue.  This
   structure will eventually be converted in one of the
   `elf_external_linux_*' structures and written out to an output bfd
   by one of the functions declared below.  */

struct elf_internal_linux_prpsinfo
  {
    char pr_state;			/* Numeric process state.  */
    char pr_sname;			/* Char for pr_state.  */
    char pr_zomb;			/* Zombie.  */
    char pr_nice;			/* Nice val.  */
    unsigned long pr_flag;		/* Flags.  */
    unsigned int pr_uid;
    unsigned int pr_gid;
    int pr_pid, pr_ppid, pr_pgrp, pr_sid;
    char pr_fname[16 + 1];		/* Filename of executable.  */
    char pr_psargs[80 + 1];		/* Initial part of arg list.  */
  };

/* Linux/most 32-bit archs.  */
extern char *elfcore_write_linux_prpsinfo32
  (bfd *, char *, int *, const struct elf_internal_linux_prpsinfo *);

/* Linux/most 64-bit archs.  */
extern char *elfcore_write_linux_prpsinfo64
  (bfd *, char *, int *, const struct elf_internal_linux_prpsinfo *);

extern bfd *_bfd_elf32_bfd_from_remote_memory
  (bfd *templ, bfd_vma ehdr_vma, bfd_size_type size, bfd_vma *loadbasep,
   int (*target_read_memory) (bfd_vma, bfd_byte *, bfd_size_type));
extern bfd *_bfd_elf64_bfd_from_remote_memory
  (bfd *templ, bfd_vma ehdr_vma, bfd_size_type size, bfd_vma *loadbasep,
   int (*target_read_memory) (bfd_vma, bfd_byte *, bfd_size_type));

extern bfd_vma bfd_elf_obj_attr_size (bfd *);
extern void bfd_elf_set_obj_attr_contents (bfd *, bfd_byte *, bfd_vma);
extern int bfd_elf_get_obj_attr_int (bfd *, int, unsigned int);
extern void bfd_elf_add_obj_attr_int (bfd *, int, unsigned int, unsigned int);
#define bfd_elf_add_proc_attr_int(BFD, TAG, VALUE) \
  bfd_elf_add_obj_attr_int ((BFD), OBJ_ATTR_PROC, (TAG), (VALUE))
extern void bfd_elf_add_obj_attr_string (bfd *, int, unsigned int, const char *);
#define bfd_elf_add_proc_attr_string(BFD, TAG, VALUE) \
  bfd_elf_add_obj_attr_string ((BFD), OBJ_ATTR_PROC, (TAG), (VALUE))
extern void bfd_elf_add_obj_attr_int_string (bfd *, int, unsigned int,
					     unsigned int, const char *);
#define bfd_elf_add_proc_attr_int_string(BFD, TAG, INTVAL, STRVAL) \
  bfd_elf_add_obj_attr_int_string ((BFD), OBJ_ATTR_PROC, (TAG), \
				   (INTVAL), (STRVAL))

extern char *_bfd_elf_attr_strdup (bfd *, const char *);
extern void _bfd_elf_copy_obj_attributes (bfd *, bfd *);
extern int _bfd_elf_obj_attrs_arg_type (bfd *, int, unsigned int);
extern void _bfd_elf_parse_attributes (bfd *, Elf_Internal_Shdr *);
extern bool _bfd_elf_merge_object_attributes
  (bfd *, struct bfd_link_info *);
extern bool _bfd_elf_merge_unknown_attribute_low (bfd *, bfd *, int);
extern bool _bfd_elf_merge_unknown_attribute_list (bfd *, bfd *);
extern Elf_Internal_Shdr *_bfd_elf_single_rel_hdr (asection *sec);
extern bool elf_read_notes (bfd *, file_ptr, bfd_size_type, size_t);

extern bool _bfd_elf_parse_gnu_properties
  (bfd *, Elf_Internal_Note *);
extern elf_property * _bfd_elf_get_property
  (bfd *, unsigned int, unsigned int);
extern bfd *_bfd_elf_link_setup_gnu_properties
  (struct bfd_link_info *);
extern bfd_size_type _bfd_elf_convert_gnu_property_size
  (bfd *, bfd *);
extern bool _bfd_elf_convert_gnu_properties
  (bfd *, asection *, bfd *, bfd_byte **, bfd_size_type *);

/* The linker may need to keep track of the number of relocs that it
   decides to copy as dynamic relocs in check_relocs for each symbol.
   This is so that it can later discard them if they are found to be
   unnecessary.  We can store the information in a field extending the
   regular ELF linker hash table.  */

struct elf_dyn_relocs
{
  struct elf_dyn_relocs *next;

  /* The input section of the reloc.  */
  asection *sec;

  /* Total number of relocs copied for the input section.  */
  bfd_size_type count;

  /* Number of pc-relative relocs copied for the input section.  */
  bfd_size_type pc_count;
};

extern bool _bfd_elf_create_ifunc_sections
  (bfd *, struct bfd_link_info *);
extern bool _bfd_elf_allocate_ifunc_dyn_relocs
  (struct bfd_link_info *, struct elf_link_hash_entry *,
   struct elf_dyn_relocs **, unsigned int, unsigned int,
   unsigned int, bool);

extern void elf_append_rela (bfd *, asection *, Elf_Internal_Rela *);
extern void elf_append_rel (bfd *, asection *, Elf_Internal_Rela *);

extern bfd_vma elf64_r_info (bfd_vma, bfd_vma);
extern bfd_vma elf64_r_sym (bfd_vma);
extern bfd_vma elf32_r_info (bfd_vma, bfd_vma);
extern bfd_vma elf32_r_sym (bfd_vma);

extern bool is_debuginfo_file (bfd *);


extern bool _bfd_elf_init_secondary_reloc_section
  (bfd *, Elf_Internal_Shdr *, const char *, unsigned int);
extern bool _bfd_elf_slurp_secondary_reloc_section
  (bfd *, asection *, asymbol **, bool);
extern bool _bfd_elf_copy_special_section_fields
  (const bfd *, bfd *, const Elf_Internal_Shdr *, Elf_Internal_Shdr *);
extern bool _bfd_elf_write_secondary_reloc_section
  (bfd *, asection *);
extern unsigned int _bfd_elf_symbol_section_index
  (bfd *, elf_symbol_type *);

extern asection *_bfd_elf_readonly_dynrelocs
  (struct elf_link_hash_entry *);
extern bool _bfd_elf_maybe_set_textrel
  (struct elf_link_hash_entry *, void *);

extern bool _bfd_elf_add_dynamic_tags
  (bfd *, struct bfd_link_info *, bool);

/* Large common section.  */
extern asection _bfd_elf_large_com_section;

/* Hash for local symbol with the first section id, ID, in the input
   file and the local symbol index, SYM.  */
#define ELF_LOCAL_SYMBOL_HASH(ID, SYM) \
  (((((ID) & 0xffU) << 24) | (((ID) & 0xff00) << 8)) \
   ^ (SYM) ^ (((ID) & 0xffff0000U) >> 16))

/* This is the condition under which finish_dynamic_symbol will be called.
   If our finish_dynamic_symbol isn't called, we'll need to do something
   about initializing any .plt and .got entries in relocate_section.  */
#define WILL_CALL_FINISH_DYNAMIC_SYMBOL(DYN, SHARED, H) \
  ((DYN)								\
   && ((SHARED) || !(H)->forced_local)					\
   && ((H)->dynindx != -1 || (H)->forced_local))

/* This macro is to avoid lots of duplicated code in the body
   of xxx_relocate_section() in the various elfxx-xxxx.c files.  */
#define RELOC_FOR_GLOBAL_SYMBOL(info, input_bfd, input_section, rel,	\
				r_symndx, symtab_hdr, sym_hashes,	\
				h, sec, relocation,			\
				unresolved_reloc, warned, ignored)	\
  do									\
    {									\
      /* It seems this can happen with erroneous or unsupported		\
	 input (mixing a.out and elf in an archive, for example.)  */	\
      if (sym_hashes == NULL)						\
	return false;							\
									\
      h = sym_hashes[r_symndx - symtab_hdr->sh_info];			\
									\
      if (info->wrap_hash != NULL					\
	  && (input_section->flags & SEC_DEBUGGING) != 0)		\
	h = ((struct elf_link_hash_entry *)				\
	     unwrap_hash_lookup (info, input_bfd, &h->root));		\
									\
      while (h->root.type == bfd_link_hash_indirect			\
	     || h->root.type == bfd_link_hash_warning)			\
	h = (struct elf_link_hash_entry *) h->root.u.i.link;		\
									\
      warned = false;							\
      ignored = false;							\
      unresolved_reloc = false;						\
      relocation = 0;							\
      if (h->root.type == bfd_link_hash_defined				\
	  || h->root.type == bfd_link_hash_defweak)			\
	{								\
	  sec = h->root.u.def.section;					\
	  if (sec == NULL						\
	      || sec->output_section == NULL)				\
	    /* Set a flag that will be cleared later if we find a	\
	       relocation value for this symbol.  output_section	\
	       is typically NULL for symbols satisfied by a shared	\
	       library.  */						\
	    unresolved_reloc = true;					\
	  else								\
	    relocation = (h->root.u.def.value				\
			  + sec->output_section->vma			\
			  + sec->output_offset);			\
	}								\
      else if (h->root.type == bfd_link_hash_undefweak)			\
	;								\
      else if (info->unresolved_syms_in_objects == RM_IGNORE		\
	       && ELF_ST_VISIBILITY (h->other) == STV_DEFAULT)		\
	ignored = true;							\
      else if (!bfd_link_relocatable (info))				\
	{								\
	  bool err = ((info->unresolved_syms_in_objects == RM_DIAGNOSE	\
		       && !info->warn_unresolved_syms)			\
		      || ELF_ST_VISIBILITY (h->other) != STV_DEFAULT);	\
	  (*info->callbacks->undefined_symbol) (info,			\
						h->root.root.string,	\
						input_bfd,		\
						input_section,		\
						rel->r_offset, err);	\
	  warned = true;						\
	}								\
      (void) unresolved_reloc;						\
      (void) warned;							\
      (void) ignored;							\
    }									\
  while (0)

/* This macro is to avoid lots of duplicated code in the body of the
   loop over relocations in xxx_relocate_section() in the various
   elfxx-xxxx.c files.

   Handle relocations against symbols from removed linkonce sections,
   or sections discarded by a linker script.  When doing a relocatable
   link, we remove such relocations.  Otherwise, we just want the
   section contents zeroed and avoid any special processing.  */
#define RELOC_AGAINST_DISCARDED_SECTION(info, input_bfd, input_section,	\
					rel, count, relend,		\
					howto, index, contents)		\
  {									\
    int i_;								\
    _bfd_clear_contents (howto, input_bfd, input_section,		\
			 contents, rel[index].r_offset);		\
									\
    if (bfd_link_relocatable (info)					\
	&& (input_section->flags & SEC_DEBUGGING))			\
      {									\
	/* Only remove relocations in debug sections since other	\
	   sections may require relocations.  */			\
	Elf_Internal_Shdr *rel_hdr;					\
									\
	rel_hdr = _bfd_elf_single_rel_hdr (input_section->output_section); \
									\
	/* Avoid empty output section.  */				\
	if (rel_hdr->sh_size > rel_hdr->sh_entsize)			\
	  {								\
	    rel_hdr->sh_size -= rel_hdr->sh_entsize;			\
	    rel_hdr = _bfd_elf_single_rel_hdr (input_section);		\
	    rel_hdr->sh_size -= rel_hdr->sh_entsize;			\
									\
	    memmove (rel, rel + count,					\
		     (relend - rel - count) * sizeof (*rel));		\
									\
	    input_section->reloc_count -= count;			\
	    relend -= count;						\
	    rel--;							\
	    continue;							\
	  }								\
      }									\
									\
    for (i_ = 0; i_ < count; i_++)					\
      {									\
	rel[i_].r_info = 0;						\
	rel[i_].r_addend = 0;						\
      }									\
    rel += count - 1;							\
    continue;								\
  }

/* Will a symbol be bound to the definition within the shared
   library, if any.  A unique symbol can never be bound locally.  */
#define SYMBOLIC_BIND(INFO, H) \
    (!(H)->unique_global \
     && ((INFO)->symbolic \
	 || (H)->start_stop \
	 || ((INFO)->dynamic && !(H)->dynamic)))

/* Determine if a section contains CTF data, using its name.  */
static inline bool
bfd_section_is_ctf (const asection *sec)
{
  const char *name = bfd_section_name (sec);
  return startswith (name, ".ctf") && (name[4] == 0 || name[4] == '.');
}

#ifdef __cplusplus
}
#endif
#endif /* _LIBELF_H_ */
