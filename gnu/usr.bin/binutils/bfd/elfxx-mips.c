/* MIPS-specific support for ELF
   Copyright (C) 1993-2023 Free Software Foundation, Inc.

   Most of the information added by Ian Lance Taylor, Cygnus Support,
   <ian@cygnus.com>.
   N32/64 ABI support added by Mark Mitchell, CodeSourcery, LLC.
   <mark@codesourcery.com>
   Traditional MIPS targets support added by Koundinya.K, Dansk Data
   Elektronik & Operations Research Group. <kk@ddeorg.soft.net>

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


/* This file handles functionality common to the different MIPS ABI's.  */

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"
#include "libiberty.h"
#include "elf-bfd.h"
#include "ecoff-bfd.h"
#include "elfxx-mips.h"
#include "elf/mips.h"
#include "elf-vxworks.h"
#include "dwarf2.h"

/* Get the ECOFF swapping routines.  */
#include "coff/sym.h"
#include "coff/symconst.h"
#include "coff/ecoff.h"
#include "coff/mips.h"

#include "hashtab.h"

/* Types of TLS GOT entry.  */
enum mips_got_tls_type {
  GOT_TLS_NONE,
  GOT_TLS_GD,
  GOT_TLS_LDM,
  GOT_TLS_IE
};

/* This structure is used to hold information about one GOT entry.
   There are four types of entry:

      (1) an absolute address
	    requires: abfd == NULL
	    fields: d.address

      (2) a SYMBOL + OFFSET address, where SYMBOL is local to an input bfd
	    requires: abfd != NULL, symndx >= 0, tls_type != GOT_TLS_LDM
	    fields: abfd, symndx, d.addend, tls_type

      (3) a SYMBOL address, where SYMBOL is not local to an input bfd
	    requires: abfd != NULL, symndx == -1
	    fields: d.h, tls_type

      (4) a TLS LDM slot
	    requires: abfd != NULL, symndx == 0, tls_type == GOT_TLS_LDM
	    fields: none; there's only one of these per GOT.  */
struct mips_got_entry
{
  /* One input bfd that needs the GOT entry.  */
  bfd *abfd;
  /* The index of the symbol, as stored in the relocation r_info, if
     we have a local symbol; -1 otherwise.  */
  long symndx;
  union
  {
    /* If abfd == NULL, an address that must be stored in the got.  */
    bfd_vma address;
    /* If abfd != NULL && symndx != -1, the addend of the relocation
       that should be added to the symbol value.  */
    bfd_vma addend;
    /* If abfd != NULL && symndx == -1, the hash table entry
       corresponding to a symbol in the GOT.  The symbol's entry
       is in the local area if h->global_got_area is GGA_NONE,
       otherwise it is in the global area.  */
    struct mips_elf_link_hash_entry *h;
  } d;

  /* The TLS type of this GOT entry.  An LDM GOT entry will be a local
     symbol entry with r_symndx == 0.  */
  unsigned char tls_type;

  /* True if we have filled in the GOT contents for a TLS entry,
     and created the associated relocations.  */
  unsigned char tls_initialized;

  /* The offset from the beginning of the .got section to the entry
     corresponding to this symbol+addend.  If it's a global symbol
     whose offset is yet to be decided, it's going to be -1.  */
  long gotidx;
};

/* This structure represents a GOT page reference from an input bfd.
   Each instance represents a symbol + ADDEND, where the representation
   of the symbol depends on whether it is local to the input bfd.
   If it is, then SYMNDX >= 0, and the symbol has index SYMNDX in U.ABFD.
   Otherwise, SYMNDX < 0 and U.H points to the symbol's hash table entry.

   Page references with SYMNDX >= 0 always become page references
   in the output.  Page references with SYMNDX < 0 only become page
   references if the symbol binds locally; in other cases, the page
   reference decays to a global GOT reference.  */
struct mips_got_page_ref
{
  long symndx;
  union
  {
    struct mips_elf_link_hash_entry *h;
    bfd *abfd;
  } u;
  bfd_vma addend;
};

/* This structure describes a range of addends: [MIN_ADDEND, MAX_ADDEND].
   The structures form a non-overlapping list that is sorted by increasing
   MIN_ADDEND.  */
struct mips_got_page_range
{
  struct mips_got_page_range *next;
  bfd_signed_vma min_addend;
  bfd_signed_vma max_addend;
};

/* This structure describes the range of addends that are applied to page
   relocations against a given section.  */
struct mips_got_page_entry
{
  /* The section that these entries are based on.  */
  asection *sec;
  /* The ranges for this page entry.  */
  struct mips_got_page_range *ranges;
  /* The maximum number of page entries needed for RANGES.  */
  bfd_vma num_pages;
};

/* This structure is used to hold .got information when linking.  */

struct mips_got_info
{
  /* The number of global .got entries.  */
  unsigned int global_gotno;
  /* The number of global .got entries that are in the GGA_RELOC_ONLY area.  */
  unsigned int reloc_only_gotno;
  /* The number of .got slots used for TLS.  */
  unsigned int tls_gotno;
  /* The first unused TLS .got entry.  Used only during
     mips_elf_initialize_tls_index.  */
  unsigned int tls_assigned_gotno;
  /* The number of local .got entries, eventually including page entries.  */
  unsigned int local_gotno;
  /* The maximum number of page entries needed.  */
  unsigned int page_gotno;
  /* The number of relocations needed for the GOT entries.  */
  unsigned int relocs;
  /* The first unused local .got entry.  */
  unsigned int assigned_low_gotno;
  /* The last unused local .got entry.  */
  unsigned int assigned_high_gotno;
  /* A hash table holding members of the got.  */
  struct htab *got_entries;
  /* A hash table holding mips_got_page_ref structures.  */
  struct htab *got_page_refs;
  /* A hash table of mips_got_page_entry structures.  */
  struct htab *got_page_entries;
  /* In multi-got links, a pointer to the next got (err, rather, most
     of the time, it points to the previous got).  */
  struct mips_got_info *next;
};

/* Structure passed when merging bfds' gots.  */

struct mips_elf_got_per_bfd_arg
{
  /* The output bfd.  */
  bfd *obfd;
  /* The link information.  */
  struct bfd_link_info *info;
  /* A pointer to the primary got, i.e., the one that's going to get
     the implicit relocations from DT_MIPS_LOCAL_GOTNO and
     DT_MIPS_GOTSYM.  */
  struct mips_got_info *primary;
  /* A non-primary got we're trying to merge with other input bfd's
     gots.  */
  struct mips_got_info *current;
  /* The maximum number of got entries that can be addressed with a
     16-bit offset.  */
  unsigned int max_count;
  /* The maximum number of page entries needed by each got.  */
  unsigned int max_pages;
  /* The total number of global entries which will live in the
     primary got and be automatically relocated.  This includes
     those not referenced by the primary GOT but included in
     the "master" GOT.  */
  unsigned int global_count;
};

/* A structure used to pass information to htab_traverse callbacks
   when laying out the GOT.  */

struct mips_elf_traverse_got_arg
{
  struct bfd_link_info *info;
  struct mips_got_info *g;
  int value;
};

struct _mips_elf_section_data
{
  struct bfd_elf_section_data elf;
  union
  {
    bfd_byte *tdata;
  } u;
};

#define mips_elf_section_data(sec) \
  ((struct _mips_elf_section_data *) elf_section_data (sec))

#define is_mips_elf(bfd)				\
  (bfd_get_flavour (bfd) == bfd_target_elf_flavour	\
   && elf_tdata (bfd) != NULL				\
   && elf_object_id (bfd) == MIPS_ELF_DATA)

/* The ABI says that every symbol used by dynamic relocations must have
   a global GOT entry.  Among other things, this provides the dynamic
   linker with a free, directly-indexed cache.  The GOT can therefore
   contain symbols that are not referenced by GOT relocations themselves
   (in other words, it may have symbols that are not referenced by things
   like R_MIPS_GOT16 and R_MIPS_GOT_PAGE).

   GOT relocations are less likely to overflow if we put the associated
   GOT entries towards the beginning.  We therefore divide the global
   GOT entries into two areas: "normal" and "reloc-only".  Entries in
   the first area can be used for both dynamic relocations and GP-relative
   accesses, while those in the "reloc-only" area are for dynamic
   relocations only.

   These GGA_* ("Global GOT Area") values are organised so that lower
   values are more general than higher values.  Also, non-GGA_NONE
   values are ordered by the position of the area in the GOT.  */
#define GGA_NORMAL 0
#define GGA_RELOC_ONLY 1
#define GGA_NONE 2

/* Information about a non-PIC interface to a PIC function.  There are
   two ways of creating these interfaces.  The first is to add:

	lui	$25,%hi(func)
	addiu	$25,$25,%lo(func)

   immediately before a PIC function "func".  The second is to add:

	lui	$25,%hi(func)
	j	func
	addiu	$25,$25,%lo(func)

   to a separate trampoline section.

   Stubs of the first kind go in a new section immediately before the
   target function.  Stubs of the second kind go in a single section
   pointed to by the hash table's "strampoline" field.  */
struct mips_elf_la25_stub {
  /* The generated section that contains this stub.  */
  asection *stub_section;

  /* The offset of the stub from the start of STUB_SECTION.  */
  bfd_vma offset;

  /* One symbol for the original function.  Its location is available
     in H->root.root.u.def.  */
  struct mips_elf_link_hash_entry *h;
};

/* Macros for populating a mips_elf_la25_stub.  */

#define LA25_LUI(VAL) (0x3c190000 | (VAL))	/* lui t9,VAL */
#define LA25_J(VAL) (0x08000000 | (((VAL) >> 2) & 0x3ffffff)) /* j VAL */
#define LA25_BC(VAL) (0xc8000000 | (((VAL) >> 2) & 0x3ffffff)) /* bc VAL */
#define LA25_ADDIU(VAL) (0x27390000 | (VAL))	/* addiu t9,t9,VAL */
#define LA25_LUI_MICROMIPS(VAL)						\
  (0x41b90000 | (VAL))				/* lui t9,VAL */
#define LA25_J_MICROMIPS(VAL)						\
  (0xd4000000 | (((VAL) >> 1) & 0x3ffffff))	/* j VAL */
#define LA25_ADDIU_MICROMIPS(VAL)					\
  (0x33390000 | (VAL))				/* addiu t9,t9,VAL */

/* This structure is passed to mips_elf_sort_hash_table_f when sorting
   the dynamic symbols.  */

struct mips_elf_hash_sort_data
{
  /* The symbol in the global GOT with the lowest dynamic symbol table
     index.  */
  struct elf_link_hash_entry *low;
  /* The least dynamic symbol table index corresponding to a non-TLS
     symbol with a GOT entry.  */
  bfd_size_type min_got_dynindx;
  /* The greatest dynamic symbol table index corresponding to a symbol
     with a GOT entry that is not referenced (e.g., a dynamic symbol
     with dynamic relocations pointing to it from non-primary GOTs).  */
  bfd_size_type max_unref_got_dynindx;
  /* The greatest dynamic symbol table index corresponding to a local
     symbol.  */
  bfd_size_type max_local_dynindx;
  /* The greatest dynamic symbol table index corresponding to an external
     symbol without a GOT entry.  */
  bfd_size_type max_non_got_dynindx;
  /* If non-NULL, output BFD for .MIPS.xhash finalization.  */
  bfd *output_bfd;
  /* If non-NULL, pointer to contents of .MIPS.xhash for filling in
     real final dynindx.  */
  bfd_byte *mipsxhash;
};

/* We make up to two PLT entries if needed, one for standard MIPS code
   and one for compressed code, either a MIPS16 or microMIPS one.  We
   keep a separate record of traditional lazy-binding stubs, for easier
   processing.  */

struct plt_entry
{
  /* Traditional SVR4 stub offset, or -1 if none.  */
  bfd_vma stub_offset;

  /* Standard PLT entry offset, or -1 if none.  */
  bfd_vma mips_offset;

  /* Compressed PLT entry offset, or -1 if none.  */
  bfd_vma comp_offset;

  /* The corresponding .got.plt index, or -1 if none.  */
  bfd_vma gotplt_index;

  /* Whether we need a standard PLT entry.  */
  unsigned int need_mips : 1;

  /* Whether we need a compressed PLT entry.  */
  unsigned int need_comp : 1;
};

/* The MIPS ELF linker needs additional information for each symbol in
   the global hash table.  */

struct mips_elf_link_hash_entry
{
  struct elf_link_hash_entry root;

  /* External symbol information.  */
  EXTR esym;

  /* The la25 stub we have created for ths symbol, if any.  */
  struct mips_elf_la25_stub *la25_stub;

  /* Number of R_MIPS_32, R_MIPS_REL32, or R_MIPS_64 relocs against
     this symbol.  */
  unsigned int possibly_dynamic_relocs;

  /* If there is a stub that 32 bit functions should use to call this
     16 bit function, this points to the section containing the stub.  */
  asection *fn_stub;

  /* If there is a stub that 16 bit functions should use to call this
     32 bit function, this points to the section containing the stub.  */
  asection *call_stub;

  /* This is like the call_stub field, but it is used if the function
     being called returns a floating point value.  */
  asection *call_fp_stub;

  /* If non-zero, location in .MIPS.xhash to write real final dynindx.  */
  bfd_vma mipsxhash_loc;

  /* The highest GGA_* value that satisfies all references to this symbol.  */
  unsigned int global_got_area : 2;

  /* True if all GOT relocations against this symbol are for calls.  This is
     a looser condition than no_fn_stub below, because there may be other
     non-call non-GOT relocations against the symbol.  */
  unsigned int got_only_for_calls : 1;

  /* True if one of the relocations described by possibly_dynamic_relocs
     is against a readonly section.  */
  unsigned int readonly_reloc : 1;

  /* True if there is a relocation against this symbol that must be
     resolved by the static linker (in other words, if the relocation
     cannot possibly be made dynamic).  */
  unsigned int has_static_relocs : 1;

  /* True if we must not create a .MIPS.stubs entry for this symbol.
     This is set, for example, if there are relocations related to
     taking the function's address, i.e. any but R_MIPS_CALL*16 ones.
     See "MIPS ABI Supplement, 3rd Edition", p. 4-20.  */
  unsigned int no_fn_stub : 1;

  /* Whether we need the fn_stub; this is true if this symbol appears
     in any relocs other than a 16 bit call.  */
  unsigned int need_fn_stub : 1;

  /* True if this symbol is referenced by branch relocations from
     any non-PIC input file.  This is used to determine whether an
     la25 stub is required.  */
  unsigned int has_nonpic_branches : 1;

  /* Does this symbol need a traditional MIPS lazy-binding stub
     (as opposed to a PLT entry)?  */
  unsigned int needs_lazy_stub : 1;

  /* Does this symbol resolve to a PLT entry?  */
  unsigned int use_plt_entry : 1;
};

/* MIPS ELF linker hash table.  */

struct mips_elf_link_hash_table
{
  struct elf_link_hash_table root;

  /* The number of .rtproc entries.  */
  bfd_size_type procedure_count;

  /* The size of the .compact_rel section (if SGI_COMPAT).  */
  bfd_size_type compact_rel_size;

  /* This flag indicates that the value of DT_MIPS_RLD_MAP dynamic entry
     is set to the address of __rld_obj_head as in IRIX5 and IRIX6.  */
  bool use_rld_obj_head;

  /* The  __rld_map or __rld_obj_head symbol. */
  struct elf_link_hash_entry *rld_symbol;

  /* This is set if we see any mips16 stub sections.  */
  bool mips16_stubs_seen;

  /* True if we can generate copy relocs and PLTs.  */
  bool use_plts_and_copy_relocs;

  /* True if we can only use 32-bit microMIPS instructions.  */
  bool insn32;

  /* True if we suppress checks for invalid branches between ISA modes.  */
  bool ignore_branch_isa;

  /* True if we are targetting R6 compact branches.  */
  bool compact_branches;

  /* True if we already reported the small-data section overflow.  */
  bool small_data_overflow_reported;

  /* True if we use the special `__gnu_absolute_zero' symbol.  */
  bool use_absolute_zero;

  /* True if we have been configured for a GNU target.  */
  bool gnu_target;

  /* Shortcuts to some dynamic sections, or NULL if they are not
     being used.  */
  asection *srelplt2;
  asection *sstubs;

  /* The master GOT information.  */
  struct mips_got_info *got_info;

  /* The global symbol in the GOT with the lowest index in the dynamic
     symbol table.  */
  struct elf_link_hash_entry *global_gotsym;

  /* The size of the PLT header in bytes.  */
  bfd_vma plt_header_size;

  /* The size of a standard PLT entry in bytes.  */
  bfd_vma plt_mips_entry_size;

  /* The size of a compressed PLT entry in bytes.  */
  bfd_vma plt_comp_entry_size;

  /* The offset of the next standard PLT entry to create.  */
  bfd_vma plt_mips_offset;

  /* The offset of the next compressed PLT entry to create.  */
  bfd_vma plt_comp_offset;

  /* The index of the next .got.plt entry to create.  */
  bfd_vma plt_got_index;

  /* The number of functions that need a lazy-binding stub.  */
  bfd_vma lazy_stub_count;

  /* The size of a function stub entry in bytes.  */
  bfd_vma function_stub_size;

  /* The number of reserved entries at the beginning of the GOT.  */
  unsigned int reserved_gotno;

  /* The section used for mips_elf_la25_stub trampolines.
     See the comment above that structure for details.  */
  asection *strampoline;

  /* A table of mips_elf_la25_stubs, indexed by (input_section, offset)
     pairs.  */
  htab_t la25_stubs;

  /* A function FN (NAME, IS, OS) that creates a new input section
     called NAME and links it to output section OS.  If IS is nonnull,
     the new section should go immediately before it, otherwise it
     should go at the (current) beginning of OS.

     The function returns the new section on success, otherwise it
     returns null.  */
  asection *(*add_stub_section) (const char *, asection *, asection *);

  /* Is the PLT header compressed?  */
  unsigned int plt_header_is_comp : 1;
};

/* Get the MIPS ELF linker hash table from a link_info structure.  */

#define mips_elf_hash_table(p) \
  ((is_elf_hash_table ((p)->hash)					\
    && elf_hash_table_id (elf_hash_table (p)) == MIPS_ELF_DATA)		\
   ? (struct mips_elf_link_hash_table *) (p)->hash : NULL)

/* A structure used to communicate with htab_traverse callbacks.  */
struct mips_htab_traverse_info
{
  /* The usual link-wide information.  */
  struct bfd_link_info *info;
  bfd *output_bfd;

  /* Starts off FALSE and is set to TRUE if the link should be aborted.  */
  bool error;
};

/* Used to store a REL high-part relocation such as R_MIPS_HI16 or
   R_MIPS_GOT16.  REL is the relocation, INPUT_SECTION is the section
   that contains the relocation field and DATA points to the start of
   INPUT_SECTION.  */

struct mips_hi16
{
  struct mips_hi16 *next;
  bfd_byte *data;
  asection *input_section;
  arelent rel;
};

/* MIPS ELF private object data.  */

struct mips_elf_obj_tdata
{
  /* Generic ELF private object data.  */
  struct elf_obj_tdata root;

  /* Input BFD providing Tag_GNU_MIPS_ABI_FP attribute for output.  */
  bfd *abi_fp_bfd;

  /* Input BFD providing Tag_GNU_MIPS_ABI_MSA attribute for output.  */
  bfd *abi_msa_bfd;

  /* The abiflags for this object.  */
  Elf_Internal_ABIFlags_v0 abiflags;
  bool abiflags_valid;

  /* The GOT requirements of input bfds.  */
  struct mips_got_info *got;

  /* Used by _bfd_mips_elf_find_nearest_line.  The structure could be
     included directly in this one, but there's no point to wasting
     the memory just for the infrequently called find_nearest_line.  */
  struct mips_elf_find_line *find_line_info;

  /* An array of stub sections indexed by symbol number.  */
  asection **local_stubs;
  asection **local_call_stubs;

  /* The Irix 5 support uses two virtual sections, which represent
     text/data symbols defined in dynamic objects.  */
  asymbol *elf_data_symbol;
  asymbol *elf_text_symbol;
  asection *elf_data_section;
  asection *elf_text_section;

  struct mips_hi16 *mips_hi16_list;
};

/* Get MIPS ELF private object data from BFD's tdata.  */

#define mips_elf_tdata(bfd) \
  ((struct mips_elf_obj_tdata *) (bfd)->tdata.any)

#define TLS_RELOC_P(r_type) \
  (r_type == R_MIPS_TLS_DTPMOD32		\
   || r_type == R_MIPS_TLS_DTPMOD64		\
   || r_type == R_MIPS_TLS_DTPREL32		\
   || r_type == R_MIPS_TLS_DTPREL64		\
   || r_type == R_MIPS_TLS_GD			\
   || r_type == R_MIPS_TLS_LDM			\
   || r_type == R_MIPS_TLS_DTPREL_HI16		\
   || r_type == R_MIPS_TLS_DTPREL_LO16		\
   || r_type == R_MIPS_TLS_GOTTPREL		\
   || r_type == R_MIPS_TLS_TPREL32		\
   || r_type == R_MIPS_TLS_TPREL64		\
   || r_type == R_MIPS_TLS_TPREL_HI16		\
   || r_type == R_MIPS_TLS_TPREL_LO16		\
   || r_type == R_MIPS16_TLS_GD			\
   || r_type == R_MIPS16_TLS_LDM		\
   || r_type == R_MIPS16_TLS_DTPREL_HI16	\
   || r_type == R_MIPS16_TLS_DTPREL_LO16	\
   || r_type == R_MIPS16_TLS_GOTTPREL		\
   || r_type == R_MIPS16_TLS_TPREL_HI16		\
   || r_type == R_MIPS16_TLS_TPREL_LO16		\
   || r_type == R_MICROMIPS_TLS_GD		\
   || r_type == R_MICROMIPS_TLS_LDM		\
   || r_type == R_MICROMIPS_TLS_DTPREL_HI16	\
   || r_type == R_MICROMIPS_TLS_DTPREL_LO16	\
   || r_type == R_MICROMIPS_TLS_GOTTPREL	\
   || r_type == R_MICROMIPS_TLS_TPREL_HI16	\
   || r_type == R_MICROMIPS_TLS_TPREL_LO16)

/* Structure used to pass information to mips_elf_output_extsym.  */

struct extsym_info
{
  bfd *abfd;
  struct bfd_link_info *info;
  struct ecoff_debug_info *debug;
  const struct ecoff_debug_swap *swap;
  bool failed;
};

/* The names of the runtime procedure table symbols used on IRIX5.  */

static const char * const mips_elf_dynsym_rtproc_names[] =
{
  "_procedure_table",
  "_procedure_string_table",
  "_procedure_table_size",
  NULL
};

/* These structures are used to generate the .compact_rel section on
   IRIX5.  */

typedef struct
{
  unsigned long id1;		/* Always one?  */
  unsigned long num;		/* Number of compact relocation entries.  */
  unsigned long id2;		/* Always two?  */
  unsigned long offset;		/* The file offset of the first relocation.  */
  unsigned long reserved0;	/* Zero?  */
  unsigned long reserved1;	/* Zero?  */
} Elf32_compact_rel;

typedef struct
{
  bfd_byte id1[4];
  bfd_byte num[4];
  bfd_byte id2[4];
  bfd_byte offset[4];
  bfd_byte reserved0[4];
  bfd_byte reserved1[4];
} Elf32_External_compact_rel;

typedef struct
{
  unsigned int ctype : 1;	/* 1: long 0: short format. See below.  */
  unsigned int rtype : 4;	/* Relocation types. See below.  */
  unsigned int dist2to : 8;
  unsigned int relvaddr : 19;	/* (VADDR - vaddr of the previous entry)/ 4 */
  unsigned long konst;		/* KONST field. See below.  */
  unsigned long vaddr;		/* VADDR to be relocated.  */
} Elf32_crinfo;

typedef struct
{
  unsigned int ctype : 1;	/* 1: long 0: short format. See below.  */
  unsigned int rtype : 4;	/* Relocation types. See below.  */
  unsigned int dist2to : 8;
  unsigned int relvaddr : 19;	/* (VADDR - vaddr of the previous entry)/ 4 */
  unsigned long konst;		/* KONST field. See below.  */
} Elf32_crinfo2;

typedef struct
{
  bfd_byte info[4];
  bfd_byte konst[4];
  bfd_byte vaddr[4];
} Elf32_External_crinfo;

typedef struct
{
  bfd_byte info[4];
  bfd_byte konst[4];
} Elf32_External_crinfo2;

/* These are the constants used to swap the bitfields in a crinfo.  */

#define CRINFO_CTYPE (0x1U)
#define CRINFO_CTYPE_SH (31)
#define CRINFO_RTYPE (0xfU)
#define CRINFO_RTYPE_SH (27)
#define CRINFO_DIST2TO (0xffU)
#define CRINFO_DIST2TO_SH (19)
#define CRINFO_RELVADDR (0x7ffffU)
#define CRINFO_RELVADDR_SH (0)

/* A compact relocation info has long (3 words) or short (2 words)
   formats.  A short format doesn't have VADDR field and relvaddr
   fields contains ((VADDR - vaddr of the previous entry) >> 2).  */
#define CRF_MIPS_LONG			1
#define CRF_MIPS_SHORT			0

/* There are 4 types of compact relocation at least. The value KONST
   has different meaning for each type:

   (type)		(konst)
   CT_MIPS_REL32	Address in data
   CT_MIPS_WORD		Address in word (XXX)
   CT_MIPS_GPHI_LO	GP - vaddr
   CT_MIPS_JMPAD	Address to jump
   */

#define CRT_MIPS_REL32			0xa
#define CRT_MIPS_WORD			0xb
#define CRT_MIPS_GPHI_LO		0xc
#define CRT_MIPS_JMPAD			0xd

#define mips_elf_set_cr_format(x,format)	((x).ctype = (format))
#define mips_elf_set_cr_type(x,type)		((x).rtype = (type))
#define mips_elf_set_cr_dist2to(x,v)		((x).dist2to = (v))
#define mips_elf_set_cr_relvaddr(x,d)		((x).relvaddr = (d)<<2)

/* The structure of the runtime procedure descriptor created by the
   loader for use by the static exception system.  */

typedef struct runtime_pdr {
	bfd_vma	adr;		/* Memory address of start of procedure.  */
	long	regmask;	/* Save register mask.  */
	long	regoffset;	/* Save register offset.  */
	long	fregmask;	/* Save floating point register mask.  */
	long	fregoffset;	/* Save floating point register offset.  */
	long	frameoffset;	/* Frame size.  */
	short	framereg;	/* Frame pointer register.  */
	short	pcreg;		/* Offset or reg of return pc.  */
	long	irpss;		/* Index into the runtime string table.  */
	long	reserved;
	struct exception_info *exception_info;/* Pointer to exception array.  */
} RPDR, *pRPDR;
#define cbRPDR sizeof (RPDR)
#define rpdNil ((pRPDR) 0)

static struct mips_got_entry *mips_elf_create_local_got_entry
  (bfd *, struct bfd_link_info *, bfd *, bfd_vma, unsigned long,
   struct mips_elf_link_hash_entry *, int);
static bool mips_elf_sort_hash_table_f
  (struct mips_elf_link_hash_entry *, void *);
static bfd_vma mips_elf_high
  (bfd_vma);
static bool mips_elf_create_dynamic_relocation
  (bfd *, struct bfd_link_info *, const Elf_Internal_Rela *,
   struct mips_elf_link_hash_entry *, asection *, bfd_vma,
   bfd_vma *, asection *);
static bfd_vma mips_elf_adjust_gp
  (bfd *, struct mips_got_info *, bfd *);

/* This will be used when we sort the dynamic relocation records.  */
static bfd *reldyn_sorting_bfd;

/* True if ABFD is for CPUs with load interlocking that include
   non-MIPS1 CPUs and R3900.  */
#define LOAD_INTERLOCKS_P(abfd) \
  (   ((elf_elfheader (abfd)->e_flags & EF_MIPS_ARCH) != E_MIPS_ARCH_1) \
   || ((elf_elfheader (abfd)->e_flags & EF_MIPS_MACH) == E_MIPS_MACH_3900))

/* True if ABFD is for CPUs that are faster if JAL is converted to BAL.
   This should be safe for all architectures.  We enable this predicate
   for RM9000 for now.  */
#define JAL_TO_BAL_P(abfd) \
  ((elf_elfheader (abfd)->e_flags & EF_MIPS_MACH) == E_MIPS_MACH_9000)

/* True if ABFD is for CPUs that are faster if JALR is converted to BAL.
   This should be safe for all architectures.  We enable this predicate for
   all CPUs.  */
#define JALR_TO_BAL_P(abfd) 1

/* True if ABFD is for CPUs that are faster if JR is converted to B.
   This should be safe for all architectures.  We enable this predicate for
   all CPUs.  */
#define JR_TO_B_P(abfd) 1

/* True if ABFD is a PIC object.  */
#define PIC_OBJECT_P(abfd) \
  ((elf_elfheader (abfd)->e_flags & EF_MIPS_PIC) != 0)

/* Nonzero if ABFD is using the O32 ABI.  */
#define ABI_O32_P(abfd) \
  ((elf_elfheader (abfd)->e_flags & EF_MIPS_ABI) == E_MIPS_ABI_O32)

/* Nonzero if ABFD is using the N32 ABI.  */
#define ABI_N32_P(abfd) \
  ((elf_elfheader (abfd)->e_flags & EF_MIPS_ABI2) != 0)

/* Nonzero if ABFD is using the N64 ABI.  */
#define ABI_64_P(abfd) \
  (get_elf_backend_data (abfd)->s->elfclass == ELFCLASS64)

/* Nonzero if ABFD is using NewABI conventions.  */
#define NEWABI_P(abfd) (ABI_N32_P (abfd) || ABI_64_P (abfd))

/* Nonzero if ABFD has microMIPS code.  */
#define MICROMIPS_P(abfd) \
  ((elf_elfheader (abfd)->e_flags & EF_MIPS_ARCH_ASE_MICROMIPS) != 0)

/* Nonzero if ABFD is MIPS R6.  */
#define MIPSR6_P(abfd) \
  ((elf_elfheader (abfd)->e_flags & EF_MIPS_ARCH) == E_MIPS_ARCH_32R6 \
    || (elf_elfheader (abfd)->e_flags & EF_MIPS_ARCH) == E_MIPS_ARCH_64R6)

/* The IRIX compatibility level we are striving for.  */
#define IRIX_COMPAT(abfd) \
  (get_elf_backend_data (abfd)->elf_backend_mips_irix_compat (abfd))

/* Whether we are trying to be compatible with IRIX at all.  */
#define SGI_COMPAT(abfd) \
  (IRIX_COMPAT (abfd) != ict_none)

/* The name of the options section.  */
#define MIPS_ELF_OPTIONS_SECTION_NAME(abfd) \
  (NEWABI_P (abfd) ? ".MIPS.options" : ".options")

/* True if NAME is the recognized name of any SHT_MIPS_OPTIONS section.
   Some IRIX system files do not use MIPS_ELF_OPTIONS_SECTION_NAME.  */
#define MIPS_ELF_OPTIONS_SECTION_NAME_P(NAME) \
  (strcmp (NAME, ".MIPS.options") == 0 || strcmp (NAME, ".options") == 0)

/* True if NAME is the recognized name of any SHT_MIPS_ABIFLAGS section.  */
#define MIPS_ELF_ABIFLAGS_SECTION_NAME_P(NAME) \
  (strcmp (NAME, ".MIPS.abiflags") == 0)

/* Whether the section is readonly.  */
#define MIPS_ELF_READONLY_SECTION(sec) \
  ((sec->flags & (SEC_ALLOC | SEC_LOAD | SEC_READONLY))		\
   == (SEC_ALLOC | SEC_LOAD | SEC_READONLY))

/* The name of the stub section.  */
#define MIPS_ELF_STUB_SECTION_NAME(abfd) ".MIPS.stubs"

/* The size of an external REL relocation.  */
#define MIPS_ELF_REL_SIZE(abfd) \
  (get_elf_backend_data (abfd)->s->sizeof_rel)

/* The size of an external RELA relocation.  */
#define MIPS_ELF_RELA_SIZE(abfd) \
  (get_elf_backend_data (abfd)->s->sizeof_rela)

/* The size of an external dynamic table entry.  */
#define MIPS_ELF_DYN_SIZE(abfd) \
  (get_elf_backend_data (abfd)->s->sizeof_dyn)

/* The size of a GOT entry.  */
#define MIPS_ELF_GOT_SIZE(abfd) \
  (get_elf_backend_data (abfd)->s->arch_size / 8)

/* The size of the .rld_map section. */
#define MIPS_ELF_RLD_MAP_SIZE(abfd) \
  (get_elf_backend_data (abfd)->s->arch_size / 8)

/* The size of a symbol-table entry.  */
#define MIPS_ELF_SYM_SIZE(abfd) \
  (get_elf_backend_data (abfd)->s->sizeof_sym)

/* The default alignment for sections, as a power of two.  */
#define MIPS_ELF_LOG_FILE_ALIGN(abfd)				\
  (get_elf_backend_data (abfd)->s->log_file_align)

/* Get word-sized data.  */
#define MIPS_ELF_GET_WORD(abfd, ptr) \
  (ABI_64_P (abfd) ? bfd_get_64 (abfd, ptr) : bfd_get_32 (abfd, ptr))

/* Put out word-sized data.  */
#define MIPS_ELF_PUT_WORD(abfd, val, ptr)	\
  (ABI_64_P (abfd)				\
   ? bfd_put_64 (abfd, val, ptr)		\
   : bfd_put_32 (abfd, val, ptr))

/* The opcode for word-sized loads (LW or LD).  */
#define MIPS_ELF_LOAD_WORD(abfd) \
  (ABI_64_P (abfd) ? 0xdc000000 : 0x8c000000)

/* Add a dynamic symbol table-entry.  */
#define MIPS_ELF_ADD_DYNAMIC_ENTRY(info, tag, val)	\
  _bfd_elf_add_dynamic_entry (info, tag, val)

#define MIPS_ELF_RTYPE_TO_HOWTO(abfd, rtype, rela)			\
  (get_elf_backend_data (abfd)->elf_backend_mips_rtype_to_howto (abfd, rtype, rela))

/* The name of the dynamic relocation section.  */
#define MIPS_ELF_REL_DYN_NAME(INFO) \
  (mips_elf_hash_table (INFO)->root.target_os == is_vxworks \
   ? ".rela.dyn" : ".rel.dyn")

/* In case we're on a 32-bit machine, construct a 64-bit "-1" value
   from smaller values.  Start with zero, widen, *then* decrement.  */
#define MINUS_ONE	(((bfd_vma)0) - 1)
#define MINUS_TWO	(((bfd_vma)0) - 2)

/* The value to write into got[1] for SVR4 targets, to identify it is
   a GNU object.  The dynamic linker can then use got[1] to store the
   module pointer.  */
#define MIPS_ELF_GNU_GOT1_MASK(abfd) \
  ((bfd_vma) 1 << (ABI_64_P (abfd) ? 63 : 31))

/* The offset of $gp from the beginning of the .got section.  */
#define ELF_MIPS_GP_OFFSET(INFO) \
  (mips_elf_hash_table (INFO)->root.target_os == is_vxworks \
   ? 0x0 : 0x7ff0)

/* The maximum size of the GOT for it to be addressable using 16-bit
   offsets from $gp.  */
#define MIPS_ELF_GOT_MAX_SIZE(INFO) (ELF_MIPS_GP_OFFSET (INFO) + 0x7fff)

/* Instructions which appear in a stub.  */
#define STUB_LW(abfd)							\
  ((ABI_64_P (abfd)							\
    ? 0xdf998010				/* ld t9,0x8010(gp) */	\
    : 0x8f998010))				/* lw t9,0x8010(gp) */
#define STUB_MOVE 0x03e07825			/* or t7,ra,zero */
#define STUB_LUI(VAL) (0x3c180000 + (VAL))	/* lui t8,VAL */
#define STUB_JALR 0x0320f809			/* jalr ra,t9 */
#define STUB_JALRC 0xf8190000			/* jalrc ra,t9 */
#define STUB_ORI(VAL) (0x37180000 + (VAL))	/* ori t8,t8,VAL */
#define STUB_LI16U(VAL) (0x34180000 + (VAL))	/* ori t8,zero,VAL unsigned */
#define STUB_LI16S(abfd, VAL)						\
   ((ABI_64_P (abfd)							\
    ? (0x64180000 + (VAL))	/* daddiu t8,zero,VAL sign extended */	\
    : (0x24180000 + (VAL))))	/* addiu t8,zero,VAL sign extended */

/* Likewise for the microMIPS ASE.  */
#define STUB_LW_MICROMIPS(abfd)						\
  (ABI_64_P (abfd)							\
   ? 0xdf3c8010					/* ld t9,0x8010(gp) */	\
   : 0xff3c8010)				/* lw t9,0x8010(gp) */
#define STUB_MOVE_MICROMIPS 0x0dff		/* move t7,ra */
#define STUB_MOVE32_MICROMIPS 0x001f7a90	/* or t7,ra,zero */
#define STUB_LUI_MICROMIPS(VAL)						\
   (0x41b80000 + (VAL))				/* lui t8,VAL */
#define STUB_JALR_MICROMIPS 0x45d9		/* jalr t9 */
#define STUB_JALR32_MICROMIPS 0x03f90f3c	/* jalr ra,t9 */
#define STUB_ORI_MICROMIPS(VAL)						\
  (0x53180000 + (VAL))				/* ori t8,t8,VAL */
#define STUB_LI16U_MICROMIPS(VAL)					\
  (0x53000000 + (VAL))				/* ori t8,zero,VAL unsigned */
#define STUB_LI16S_MICROMIPS(abfd, VAL)					\
   (ABI_64_P (abfd)							\
    ? 0x5f000000 + (VAL)	/* daddiu t8,zero,VAL sign extended */	\
    : 0x33000000 + (VAL))	/* addiu t8,zero,VAL sign extended */

#define MIPS_FUNCTION_STUB_NORMAL_SIZE 16
#define MIPS_FUNCTION_STUB_BIG_SIZE 20
#define MICROMIPS_FUNCTION_STUB_NORMAL_SIZE 12
#define MICROMIPS_FUNCTION_STUB_BIG_SIZE 16
#define MICROMIPS_INSN32_FUNCTION_STUB_NORMAL_SIZE 16
#define MICROMIPS_INSN32_FUNCTION_STUB_BIG_SIZE 20

/* The name of the dynamic interpreter.  This is put in the .interp
   section.  */

#define ELF_DYNAMIC_INTERPRETER(abfd)		\
   (ABI_N32_P (abfd) ? "/usr/lib32/libc.so.1"	\
    : ABI_64_P (abfd) ? "/usr/lib64/libc.so.1"	\
    : "/usr/lib/libc.so.1")

#ifdef BFD64
#define MNAME(bfd,pre,pos) \
  (ABI_64_P (bfd) ? CONCAT4 (pre,64,_,pos) : CONCAT4 (pre,32,_,pos))
#define ELF_R_SYM(bfd, i)					\
  (ABI_64_P (bfd) ? ELF64_R_SYM (i) : ELF32_R_SYM (i))
#define ELF_R_TYPE(bfd, i)					\
  (ABI_64_P (bfd) ? ELF64_MIPS_R_TYPE (i) : ELF32_R_TYPE (i))
#define ELF_R_INFO(bfd, s, t)					\
  (ABI_64_P (bfd) ? ELF64_R_INFO (s, t) : ELF32_R_INFO (s, t))
#else
#define MNAME(bfd,pre,pos) CONCAT4 (pre,32,_,pos)
#define ELF_R_SYM(bfd, i)					\
  (ELF32_R_SYM (i))
#define ELF_R_TYPE(bfd, i)					\
  (ELF32_R_TYPE (i))
#define ELF_R_INFO(bfd, s, t)					\
  (ELF32_R_INFO (s, t))
#endif

  /* The mips16 compiler uses a couple of special sections to handle
     floating point arguments.

     Section names that look like .mips16.fn.FNNAME contain stubs that
     copy floating point arguments from the fp regs to the gp regs and
     then jump to FNNAME.  If any 32 bit function calls FNNAME, the
     call should be redirected to the stub instead.  If no 32 bit
     function calls FNNAME, the stub should be discarded.  We need to
     consider any reference to the function, not just a call, because
     if the address of the function is taken we will need the stub,
     since the address might be passed to a 32 bit function.

     Section names that look like .mips16.call.FNNAME contain stubs
     that copy floating point arguments from the gp regs to the fp
     regs and then jump to FNNAME.  If FNNAME is a 32 bit function,
     then any 16 bit function that calls FNNAME should be redirected
     to the stub instead.  If FNNAME is not a 32 bit function, the
     stub should be discarded.

     .mips16.call.fp.FNNAME sections are similar, but contain stubs
     which call FNNAME and then copy the return value from the fp regs
     to the gp regs.  These stubs store the return value in $18 while
     calling FNNAME; any function which might call one of these stubs
     must arrange to save $18 around the call.  (This case is not
     needed for 32 bit functions that call 16 bit functions, because
     16 bit functions always return floating point values in both
     $f0/$f1 and $2/$3.)

     Note that in all cases FNNAME might be defined statically.
     Therefore, FNNAME is not used literally.  Instead, the relocation
     information will indicate which symbol the section is for.

     We record any stubs that we find in the symbol table.  */

#define FN_STUB ".mips16.fn."
#define CALL_STUB ".mips16.call."
#define CALL_FP_STUB ".mips16.call.fp."

#define FN_STUB_P(name) startswith (name, FN_STUB)
#define CALL_STUB_P(name) startswith (name, CALL_STUB)
#define CALL_FP_STUB_P(name) startswith (name, CALL_FP_STUB)

/* The format of the first PLT entry in an O32 executable.  */
static const bfd_vma mips_o32_exec_plt0_entry[] =
{
  0x3c1c0000,	/* lui $28, %hi(&GOTPLT[0])				*/
  0x8f990000,	/* lw $25, %lo(&GOTPLT[0])($28)				*/
  0x279c0000,	/* addiu $28, $28, %lo(&GOTPLT[0])			*/
  0x031cc023,	/* subu $24, $24, $28					*/
  0x03e07825,	/* or t7, ra, zero					*/
  0x0018c082,	/* srl $24, $24, 2					*/
  0x0320f809,	/* jalr $25						*/
  0x2718fffe	/* subu $24, $24, 2					*/
};

/* The format of the first PLT entry in an O32 executable using compact
   jumps.  */
static const bfd_vma mipsr6_o32_exec_plt0_entry_compact[] =
{
  0x3c1c0000,	/* lui $28, %hi(&GOTPLT[0])				*/
  0x8f990000,	/* lw $25, %lo(&GOTPLT[0])($28)				*/
  0x279c0000,	/* addiu $28, $28, %lo(&GOTPLT[0])			*/
  0x031cc023,	/* subu $24, $24, $28					*/
  0x03e07821,	/* move $15, $31	# 32-bit move (addu)		*/
  0x0018c082,	/* srl $24, $24, 2					*/
  0x2718fffe,	/* subu $24, $24, 2					*/
  0xf8190000	/* jalrc $25						*/
};

/* The format of the first PLT entry in an N32 executable.  Different
   because gp ($28) is not available; we use t2 ($14) instead.  */
static const bfd_vma mips_n32_exec_plt0_entry[] =
{
  0x3c0e0000,	/* lui $14, %hi(&GOTPLT[0])				*/
  0x8dd90000,	/* lw $25, %lo(&GOTPLT[0])($14)				*/
  0x25ce0000,	/* addiu $14, $14, %lo(&GOTPLT[0])			*/
  0x030ec023,	/* subu $24, $24, $14					*/
  0x03e07825,	/* or t7, ra, zero					*/
  0x0018c082,	/* srl $24, $24, 2					*/
  0x0320f809,	/* jalr $25						*/
  0x2718fffe	/* subu $24, $24, 2					*/
};

/* The format of the first PLT entry in an N32 executable using compact
   jumps.  Different because gp ($28) is not available; we use t2 ($14)
   instead.  */
static const bfd_vma mipsr6_n32_exec_plt0_entry_compact[] =
{
  0x3c0e0000,	/* lui $14, %hi(&GOTPLT[0])				*/
  0x8dd90000,	/* lw $25, %lo(&GOTPLT[0])($14)				*/
  0x25ce0000,	/* addiu $14, $14, %lo(&GOTPLT[0])			*/
  0x030ec023,	/* subu $24, $24, $14					*/
  0x03e07821,	/* move $15, $31	# 32-bit move (addu)		*/
  0x0018c082,	/* srl $24, $24, 2					*/
  0x2718fffe,	/* subu $24, $24, 2					*/
  0xf8190000	/* jalrc $25						*/
};

/* The format of the first PLT entry in an N64 executable.  Different
   from N32 because of the increased size of GOT entries.  */
static const bfd_vma mips_n64_exec_plt0_entry[] =
{
  0x3c0e0000,	/* lui $14, %hi(&GOTPLT[0])				*/
  0xddd90000,	/* ld $25, %lo(&GOTPLT[0])($14)				*/
  0x25ce0000,	/* addiu $14, $14, %lo(&GOTPLT[0])			*/
  0x030ec023,	/* subu $24, $24, $14					*/
  0x03e07825,	/* or t7, ra, zero					*/
  0x0018c0c2,	/* srl $24, $24, 3					*/
  0x0320f809,	/* jalr $25						*/
  0x2718fffe	/* subu $24, $24, 2					*/
};

/* The format of the first PLT entry in an N64 executable using compact
   jumps.  Different from N32 because of the increased size of GOT
   entries.  */
static const bfd_vma mipsr6_n64_exec_plt0_entry_compact[] =
{
  0x3c0e0000,	/* lui $14, %hi(&GOTPLT[0])				*/
  0xddd90000,	/* ld $25, %lo(&GOTPLT[0])($14)				*/
  0x25ce0000,	/* addiu $14, $14, %lo(&GOTPLT[0])			*/
  0x030ec023,	/* subu $24, $24, $14					*/
  0x03e0782d,	/* move $15, $31	# 64-bit move (daddu)		*/
  0x0018c0c2,	/* srl $24, $24, 3					*/
  0x2718fffe,	/* subu $24, $24, 2					*/
  0xf8190000	/* jalrc $25						*/
};


/* The format of the microMIPS first PLT entry in an O32 executable.
   We rely on v0 ($2) rather than t8 ($24) to contain the address
   of the GOTPLT entry handled, so this stub may only be used when
   all the subsequent PLT entries are microMIPS code too.

   The trailing NOP is for alignment and correct disassembly only.  */
static const bfd_vma micromips_o32_exec_plt0_entry[] =
{
  0x7980, 0x0000,	/* addiupc $3, (&GOTPLT[0]) - .			*/
  0xff23, 0x0000,	/* lw $25, 0($3)				*/
  0x0535,		/* subu $2, $2, $3				*/
  0x2525,		/* srl $2, $2, 2				*/
  0x3302, 0xfffe,	/* subu $24, $2, 2				*/
  0x0dff,		/* move $15, $31				*/
  0x45f9,		/* jalrs $25					*/
  0x0f83,		/* move $28, $3					*/
  0x0c00		/* nop						*/
};

/* The format of the microMIPS first PLT entry in an O32 executable
   in the insn32 mode.  */
static const bfd_vma micromips_insn32_o32_exec_plt0_entry[] =
{
  0x41bc, 0x0000,	/* lui $28, %hi(&GOTPLT[0])			*/
  0xff3c, 0x0000,	/* lw $25, %lo(&GOTPLT[0])($28)			*/
  0x339c, 0x0000,	/* addiu $28, $28, %lo(&GOTPLT[0])		*/
  0x0398, 0xc1d0,	/* subu $24, $24, $28				*/
  0x001f, 0x7a90,	/* or $15, $31, zero				*/
  0x0318, 0x1040,	/* srl $24, $24, 2				*/
  0x03f9, 0x0f3c,	/* jalr $25					*/
  0x3318, 0xfffe	/* subu $24, $24, 2				*/
};

/* The format of subsequent standard PLT entries.  */
static const bfd_vma mips_exec_plt_entry[] =
{
  0x3c0f0000,	/* lui $15, %hi(.got.plt entry)			*/
  0x01f90000,	/* l[wd] $25, %lo(.got.plt entry)($15)		*/
  0x25f80000,	/* addiu $24, $15, %lo(.got.plt entry)		*/
  0x03200008	/* jr $25					*/
};

static const bfd_vma mipsr6_exec_plt_entry[] =
{
  0x3c0f0000,	/* lui $15, %hi(.got.plt entry)			*/
  0x01f90000,	/* l[wd] $25, %lo(.got.plt entry)($15)		*/
  0x25f80000,	/* addiu $24, $15, %lo(.got.plt entry)		*/
  0x03200009	/* jr $25					*/
};

static const bfd_vma mipsr6_exec_plt_entry_compact[] =
{
  0x3c0f0000,	/* lui $15, %hi(.got.plt entry)			*/
  0x01f90000,	/* l[wd] $25, %lo(.got.plt entry)($15)		*/
  0x25f80000,	/* addiu $24, $15, %lo(.got.plt entry)		*/
  0xd8190000	/* jic $25, 0					*/
};

/* The format of subsequent MIPS16 o32 PLT entries.  We use v0 ($2)
   and v1 ($3) as temporaries because t8 ($24) and t9 ($25) are not
   directly addressable.  */
static const bfd_vma mips16_o32_exec_plt_entry[] =
{
  0xb203,		/* lw $2, 12($pc)			*/
  0x9a60,		/* lw $3, 0($2)				*/
  0x651a,		/* move $24, $2				*/
  0xeb00,		/* jr $3				*/
  0x653b,		/* move $25, $3				*/
  0x6500,		/* nop					*/
  0x0000, 0x0000	/* .word (.got.plt entry)		*/
};

/* The format of subsequent microMIPS o32 PLT entries.  We use v0 ($2)
   as a temporary because t8 ($24) is not addressable with ADDIUPC.  */
static const bfd_vma micromips_o32_exec_plt_entry[] =
{
  0x7900, 0x0000,	/* addiupc $2, (.got.plt entry) - .	*/
  0xff22, 0x0000,	/* lw $25, 0($2)			*/
  0x4599,		/* jr $25				*/
  0x0f02		/* move $24, $2				*/
};

/* The format of subsequent microMIPS o32 PLT entries in the insn32 mode.  */
static const bfd_vma micromips_insn32_o32_exec_plt_entry[] =
{
  0x41af, 0x0000,	/* lui $15, %hi(.got.plt entry)		*/
  0xff2f, 0x0000,	/* lw $25, %lo(.got.plt entry)($15)	*/
  0x0019, 0x0f3c,	/* jr $25				*/
  0x330f, 0x0000	/* addiu $24, $15, %lo(.got.plt entry)	*/
};

/* The format of the first PLT entry in a VxWorks executable.  */
static const bfd_vma mips_vxworks_exec_plt0_entry[] =
{
  0x3c190000,	/* lui t9, %hi(_GLOBAL_OFFSET_TABLE_)		*/
  0x27390000,	/* addiu t9, t9, %lo(_GLOBAL_OFFSET_TABLE_)	*/
  0x8f390008,	/* lw t9, 8(t9)					*/
  0x00000000,	/* nop						*/
  0x03200008,	/* jr t9					*/
  0x00000000	/* nop						*/
};

/* The format of subsequent PLT entries.  */
static const bfd_vma mips_vxworks_exec_plt_entry[] =
{
  0x10000000,	/* b .PLT_resolver			*/
  0x24180000,	/* li t8, <pltindex>			*/
  0x3c190000,	/* lui t9, %hi(<.got.plt slot>)		*/
  0x27390000,	/* addiu t9, t9, %lo(<.got.plt slot>)	*/
  0x8f390000,	/* lw t9, 0(t9)				*/
  0x00000000,	/* nop					*/
  0x03200008,	/* jr t9				*/
  0x00000000	/* nop					*/
};

/* The format of the first PLT entry in a VxWorks shared object.  */
static const bfd_vma mips_vxworks_shared_plt0_entry[] =
{
  0x8f990008,	/* lw t9, 8(gp)		*/
  0x00000000,	/* nop			*/
  0x03200008,	/* jr t9		*/
  0x00000000,	/* nop			*/
  0x00000000,	/* nop			*/
  0x00000000	/* nop			*/
};

/* The format of subsequent PLT entries.  */
static const bfd_vma mips_vxworks_shared_plt_entry[] =
{
  0x10000000,	/* b .PLT_resolver	*/
  0x24180000	/* li t8, <pltindex>	*/
};

/* microMIPS 32-bit opcode helper installer.  */

static void
bfd_put_micromips_32 (const bfd *abfd, bfd_vma opcode, bfd_byte *ptr)
{
  bfd_put_16 (abfd, (opcode >> 16) & 0xffff, ptr);
  bfd_put_16 (abfd,  opcode	   & 0xffff, ptr + 2);
}

/* microMIPS 32-bit opcode helper retriever.  */

static bfd_vma
bfd_get_micromips_32 (const bfd *abfd, const bfd_byte *ptr)
{
  return (bfd_get_16 (abfd, ptr) << 16) | bfd_get_16 (abfd, ptr + 2);
}

/* Look up an entry in a MIPS ELF linker hash table.  */

#define mips_elf_link_hash_lookup(table, string, create, copy, follow)	\
  ((struct mips_elf_link_hash_entry *)					\
   elf_link_hash_lookup (&(table)->root, (string), (create),		\
			 (copy), (follow)))

/* Traverse a MIPS ELF linker hash table.  */

#define mips_elf_link_hash_traverse(table, func, info)			\
  (elf_link_hash_traverse						\
   (&(table)->root,							\
    (bool (*) (struct elf_link_hash_entry *, void *)) (func),		\
    (info)))

/* Find the base offsets for thread-local storage in this object,
   for GD/LD and IE/LE respectively.  */

#define TP_OFFSET 0x7000
#define DTP_OFFSET 0x8000

static bfd_vma
dtprel_base (struct bfd_link_info *info)
{
  /* If tls_sec is NULL, we should have signalled an error already.  */
  if (elf_hash_table (info)->tls_sec == NULL)
    return 0;
  return elf_hash_table (info)->tls_sec->vma + DTP_OFFSET;
}

static bfd_vma
tprel_base (struct bfd_link_info *info)
{
  /* If tls_sec is NULL, we should have signalled an error already.  */
  if (elf_hash_table (info)->tls_sec == NULL)
    return 0;
  return elf_hash_table (info)->tls_sec->vma + TP_OFFSET;
}

/* Create an entry in a MIPS ELF linker hash table.  */

static struct bfd_hash_entry *
mips_elf_link_hash_newfunc (struct bfd_hash_entry *entry,
			    struct bfd_hash_table *table, const char *string)
{
  struct mips_elf_link_hash_entry *ret =
    (struct mips_elf_link_hash_entry *) entry;

  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (ret == NULL)
    ret = bfd_hash_allocate (table, sizeof (struct mips_elf_link_hash_entry));
  if (ret == NULL)
    return (struct bfd_hash_entry *) ret;

  /* Call the allocation method of the superclass.  */
  ret = ((struct mips_elf_link_hash_entry *)
	 _bfd_elf_link_hash_newfunc ((struct bfd_hash_entry *) ret,
				     table, string));
  if (ret != NULL)
    {
      /* Set local fields.  */
      memset (&ret->esym, 0, sizeof (EXTR));
      /* We use -2 as a marker to indicate that the information has
	 not been set.  -1 means there is no associated ifd.  */
      ret->esym.ifd = -2;
      ret->la25_stub = 0;
      ret->possibly_dynamic_relocs = 0;
      ret->fn_stub = NULL;
      ret->call_stub = NULL;
      ret->call_fp_stub = NULL;
      ret->mipsxhash_loc = 0;
      ret->global_got_area = GGA_NONE;
      ret->got_only_for_calls = true;
      ret->readonly_reloc = false;
      ret->has_static_relocs = false;
      ret->no_fn_stub = false;
      ret->need_fn_stub = false;
      ret->has_nonpic_branches = false;
      ret->needs_lazy_stub = false;
      ret->use_plt_entry = false;
    }

  return (struct bfd_hash_entry *) ret;
}

/* Allocate MIPS ELF private object data.  */

bool
_bfd_mips_elf_mkobject (bfd *abfd)
{
  return bfd_elf_allocate_object (abfd, sizeof (struct mips_elf_obj_tdata),
				  MIPS_ELF_DATA);
}

/* MIPS ELF uses a special find_nearest_line routine in order the
   handle the ECOFF debugging information.  */

struct mips_elf_find_line
{
  struct ecoff_debug_info d;
  struct ecoff_find_line i;
};

bool
_bfd_mips_elf_free_cached_info (bfd *abfd)
{
  struct mips_elf_obj_tdata *tdata;

  if ((bfd_get_format (abfd) == bfd_object
       || bfd_get_format (abfd) == bfd_core)
      && (tdata = mips_elf_tdata (abfd)) != NULL)
    {
      BFD_ASSERT (tdata->root.object_id == MIPS_ELF_DATA);
      while (tdata->mips_hi16_list != NULL)
	{
	  struct mips_hi16 *hi = tdata->mips_hi16_list;
	  tdata->mips_hi16_list = hi->next;
	  free (hi);
	}
      if (tdata->find_line_info != NULL)
	_bfd_ecoff_free_ecoff_debug_info (&tdata->find_line_info->d);
    }
  return _bfd_elf_free_cached_info (abfd);
}

bool
_bfd_mips_elf_new_section_hook (bfd *abfd, asection *sec)
{
  if (!sec->used_by_bfd)
    {
      struct _mips_elf_section_data *sdata;
      size_t amt = sizeof (*sdata);

      sdata = bfd_zalloc (abfd, amt);
      if (sdata == NULL)
	return false;
      sec->used_by_bfd = sdata;
    }

  return _bfd_elf_new_section_hook (abfd, sec);
}

/* Read ECOFF debugging information from a .mdebug section into a
   ecoff_debug_info structure.  */

bool
_bfd_mips_elf_read_ecoff_info (bfd *abfd, asection *section,
			       struct ecoff_debug_info *debug)
{
  HDRR *symhdr;
  const struct ecoff_debug_swap *swap;
  char *ext_hdr;

  swap = get_elf_backend_data (abfd)->elf_backend_ecoff_debug_swap;
  memset (debug, 0, sizeof (*debug));

  ext_hdr = bfd_malloc (swap->external_hdr_size);
  if (ext_hdr == NULL && swap->external_hdr_size != 0)
    goto error_return;

  if (! bfd_get_section_contents (abfd, section, ext_hdr, 0,
				  swap->external_hdr_size))
    goto error_return;

  symhdr = &debug->symbolic_header;
  (*swap->swap_hdr_in) (abfd, ext_hdr, symhdr);
  free (ext_hdr);
  ext_hdr = NULL;

  /* The symbolic header contains absolute file offsets and sizes to
     read.  */
#define READ(ptr, offset, count, size)					\
  do									\
    {									\
      size_t amt;							\
      debug->ptr = NULL;						\
      if (symhdr->count == 0)						\
	break;								\
      if (_bfd_mul_overflow (size, symhdr->count, &amt))		\
	{								\
	  bfd_set_error (bfd_error_file_too_big);			\
	  goto error_return;						\
	}								\
      if (bfd_seek (abfd, symhdr->offset, SEEK_SET) != 0)		\
	goto error_return;						\
      debug->ptr = _bfd_malloc_and_read (abfd, amt + 1, amt);		\
      if (debug->ptr == NULL)						\
	goto error_return;						\
      ((char *) debug->ptr)[amt] = 0;					\
    } while (0)

  READ (line, cbLineOffset, cbLine, sizeof (unsigned char));
  READ (external_dnr, cbDnOffset, idnMax, swap->external_dnr_size);
  READ (external_pdr, cbPdOffset, ipdMax, swap->external_pdr_size);
  READ (external_sym, cbSymOffset, isymMax, swap->external_sym_size);
  READ (external_opt, cbOptOffset, ioptMax, swap->external_opt_size);
  READ (external_aux, cbAuxOffset, iauxMax, sizeof (union aux_ext));
  READ (ss, cbSsOffset, issMax, sizeof (char));
  READ (ssext, cbSsExtOffset, issExtMax, sizeof (char));
  READ (external_fdr, cbFdOffset, ifdMax, swap->external_fdr_size);
  READ (external_rfd, cbRfdOffset, crfd, swap->external_rfd_size);
  READ (external_ext, cbExtOffset, iextMax, swap->external_ext_size);
#undef READ

  return true;

 error_return:
  free (ext_hdr);
  _bfd_ecoff_free_ecoff_debug_info (debug);
  return false;
}

/* Swap RPDR (runtime procedure table entry) for output.  */

static void
ecoff_swap_rpdr_out (bfd *abfd, const RPDR *in, struct rpdr_ext *ex)
{
  H_PUT_S32 (abfd, in->adr, ex->p_adr);
  H_PUT_32 (abfd, in->regmask, ex->p_regmask);
  H_PUT_32 (abfd, in->regoffset, ex->p_regoffset);
  H_PUT_32 (abfd, in->fregmask, ex->p_fregmask);
  H_PUT_32 (abfd, in->fregoffset, ex->p_fregoffset);
  H_PUT_32 (abfd, in->frameoffset, ex->p_frameoffset);

  H_PUT_16 (abfd, in->framereg, ex->p_framereg);
  H_PUT_16 (abfd, in->pcreg, ex->p_pcreg);

  H_PUT_32 (abfd, in->irpss, ex->p_irpss);
}

/* Create a runtime procedure table from the .mdebug section.  */

static bool
mips_elf_create_procedure_table (void *handle, bfd *abfd,
				 struct bfd_link_info *info, asection *s,
				 struct ecoff_debug_info *debug)
{
  const struct ecoff_debug_swap *swap;
  HDRR *hdr = &debug->symbolic_header;
  RPDR *rpdr, *rp;
  struct rpdr_ext *erp;
  void *rtproc;
  struct pdr_ext *epdr;
  struct sym_ext *esym;
  char *ss, **sv;
  char *str;
  bfd_size_type size;
  bfd_size_type count;
  unsigned long sindex;
  unsigned long i;
  PDR pdr;
  SYMR sym;
  const char *no_name_func = _("static procedure (no name)");

  epdr = NULL;
  rpdr = NULL;
  esym = NULL;
  ss = NULL;
  sv = NULL;

  swap = get_elf_backend_data (abfd)->elf_backend_ecoff_debug_swap;

  sindex = strlen (no_name_func) + 1;
  count = hdr->ipdMax;
  if (count > 0)
    {
      size = swap->external_pdr_size;

      epdr = bfd_malloc (size * count);
      if (epdr == NULL)
	goto error_return;

      if (! _bfd_ecoff_get_accumulated_pdr (handle, (bfd_byte *) epdr))
	goto error_return;

      size = sizeof (RPDR);
      rp = rpdr = bfd_malloc (size * count);
      if (rpdr == NULL)
	goto error_return;

      size = sizeof (char *);
      sv = bfd_malloc (size * count);
      if (sv == NULL)
	goto error_return;

      count = hdr->isymMax;
      size = swap->external_sym_size;
      esym = bfd_malloc (size * count);
      if (esym == NULL)
	goto error_return;

      if (! _bfd_ecoff_get_accumulated_sym (handle, (bfd_byte *) esym))
	goto error_return;

      count = hdr->issMax;
      ss = bfd_malloc (count);
      if (ss == NULL)
	goto error_return;
      if (! _bfd_ecoff_get_accumulated_ss (handle, (bfd_byte *) ss))
	goto error_return;

      count = hdr->ipdMax;
      for (i = 0; i < (unsigned long) count; i++, rp++)
	{
	  (*swap->swap_pdr_in) (abfd, epdr + i, &pdr);
	  (*swap->swap_sym_in) (abfd, &esym[pdr.isym], &sym);
	  rp->adr = sym.value;
	  rp->regmask = pdr.regmask;
	  rp->regoffset = pdr.regoffset;
	  rp->fregmask = pdr.fregmask;
	  rp->fregoffset = pdr.fregoffset;
	  rp->frameoffset = pdr.frameoffset;
	  rp->framereg = pdr.framereg;
	  rp->pcreg = pdr.pcreg;
	  rp->irpss = sindex;
	  sv[i] = ss + sym.iss;
	  sindex += strlen (sv[i]) + 1;
	}
    }

  size = sizeof (struct rpdr_ext) * (count + 2) + sindex;
  size = BFD_ALIGN (size, 16);
  rtproc = bfd_alloc (abfd, size);
  if (rtproc == NULL)
    {
      mips_elf_hash_table (info)->procedure_count = 0;
      goto error_return;
    }

  mips_elf_hash_table (info)->procedure_count = count + 2;

  erp = rtproc;
  memset (erp, 0, sizeof (struct rpdr_ext));
  erp++;
  str = (char *) rtproc + sizeof (struct rpdr_ext) * (count + 2);
  strcpy (str, no_name_func);
  str += strlen (no_name_func) + 1;
  for (i = 0; i < count; i++)
    {
      ecoff_swap_rpdr_out (abfd, rpdr + i, erp + i);
      strcpy (str, sv[i]);
      str += strlen (sv[i]) + 1;
    }
  H_PUT_S32 (abfd, -1, (erp + count)->p_adr);

  /* Set the size and contents of .rtproc section.  */
  s->size = size;
  s->contents = rtproc;

  /* Skip this section later on (I don't think this currently
     matters, but someday it might).  */
  s->map_head.link_order = NULL;

  free (epdr);
  free (rpdr);
  free (esym);
  free (ss);
  free (sv);
  return true;

 error_return:
  free (epdr);
  free (rpdr);
  free (esym);
  free (ss);
  free (sv);
  return false;
}

/* We're going to create a stub for H.  Create a symbol for the stub's
   value and size, to help make the disassembly easier to read.  */

static bool
mips_elf_create_stub_symbol (struct bfd_link_info *info,
			     struct mips_elf_link_hash_entry *h,
			     const char *prefix, asection *s, bfd_vma value,
			     bfd_vma size)
{
  bool micromips_p = ELF_ST_IS_MICROMIPS (h->root.other);
  struct bfd_link_hash_entry *bh;
  struct elf_link_hash_entry *elfh;
  char *name;
  bool res;

  if (micromips_p)
    value |= 1;

  /* Create a new symbol.  */
  name = concat (prefix, h->root.root.root.string, NULL);
  bh = NULL;
  res = _bfd_generic_link_add_one_symbol (info, s->owner, name,
					  BSF_LOCAL, s, value, NULL,
					  true, false, &bh);
  free (name);
  if (! res)
    return false;

  /* Make it a local function.  */
  elfh = (struct elf_link_hash_entry *) bh;
  elfh->type = ELF_ST_INFO (STB_LOCAL, STT_FUNC);
  elfh->size = size;
  elfh->forced_local = 1;
  if (micromips_p)
    elfh->other = ELF_ST_SET_MICROMIPS (elfh->other);
  return true;
}

/* We're about to redefine H.  Create a symbol to represent H's
   current value and size, to help make the disassembly easier
   to read.  */

static bool
mips_elf_create_shadow_symbol (struct bfd_link_info *info,
			       struct mips_elf_link_hash_entry *h,
			       const char *prefix)
{
  struct bfd_link_hash_entry *bh;
  struct elf_link_hash_entry *elfh;
  char *name;
  asection *s;
  bfd_vma value;
  bool res;

  /* Read the symbol's value.  */
  BFD_ASSERT (h->root.root.type == bfd_link_hash_defined
	      || h->root.root.type == bfd_link_hash_defweak);
  s = h->root.root.u.def.section;
  value = h->root.root.u.def.value;

  /* Create a new symbol.  */
  name = concat (prefix, h->root.root.root.string, NULL);
  bh = NULL;
  res = _bfd_generic_link_add_one_symbol (info, s->owner, name,
					  BSF_LOCAL, s, value, NULL,
					  true, false, &bh);
  free (name);
  if (! res)
    return false;

  /* Make it local and copy the other attributes from H.  */
  elfh = (struct elf_link_hash_entry *) bh;
  elfh->type = ELF_ST_INFO (STB_LOCAL, ELF_ST_TYPE (h->root.type));
  elfh->other = h->root.other;
  elfh->size = h->root.size;
  elfh->forced_local = 1;
  return true;
}

/* Return TRUE if relocations in SECTION can refer directly to a MIPS16
   function rather than to a hard-float stub.  */

static bool
section_allows_mips16_refs_p (asection *section)
{
  const char *name;

  name = bfd_section_name (section);
  return (FN_STUB_P (name)
	  || CALL_STUB_P (name)
	  || CALL_FP_STUB_P (name)
	  || strcmp (name, ".pdr") == 0);
}

/* [RELOCS, RELEND) are the relocations against SEC, which is a MIPS16
   stub section of some kind.  Return the R_SYMNDX of the target
   function, or 0 if we can't decide which function that is.  */

static unsigned long
mips16_stub_symndx (const struct elf_backend_data *bed,
		    asection *sec ATTRIBUTE_UNUSED,
		    const Elf_Internal_Rela *relocs,
		    const Elf_Internal_Rela *relend)
{
  int int_rels_per_ext_rel = bed->s->int_rels_per_ext_rel;
  const Elf_Internal_Rela *rel;

  /* Trust the first R_MIPS_NONE relocation, if any, but not a subsequent
     one in a compound relocation.  */
  for (rel = relocs; rel < relend; rel += int_rels_per_ext_rel)
    if (ELF_R_TYPE (sec->owner, rel->r_info) == R_MIPS_NONE)
      return ELF_R_SYM (sec->owner, rel->r_info);

  /* Otherwise trust the first relocation, whatever its kind.  This is
     the traditional behavior.  */
  if (relocs < relend)
    return ELF_R_SYM (sec->owner, relocs->r_info);

  return 0;
}

/* Check the mips16 stubs for a particular symbol, and see if we can
   discard them.  */

static void
mips_elf_check_mips16_stubs (struct bfd_link_info *info,
			     struct mips_elf_link_hash_entry *h)
{
  /* Dynamic symbols must use the standard call interface, in case other
     objects try to call them.  */
  if (h->fn_stub != NULL
      && h->root.dynindx != -1)
    {
      mips_elf_create_shadow_symbol (info, h, ".mips16.");
      h->need_fn_stub = true;
    }

  if (h->fn_stub != NULL
      && ! h->need_fn_stub)
    {
      /* We don't need the fn_stub; the only references to this symbol
	 are 16 bit calls.  Clobber the size to 0 to prevent it from
	 being included in the link.  */
      h->fn_stub->size = 0;
      h->fn_stub->flags &= ~SEC_RELOC;
      h->fn_stub->reloc_count = 0;
      h->fn_stub->flags |= SEC_EXCLUDE;
      h->fn_stub->output_section = bfd_abs_section_ptr;
    }

  if (h->call_stub != NULL
      && ELF_ST_IS_MIPS16 (h->root.other))
    {
      /* We don't need the call_stub; this is a 16 bit function, so
	 calls from other 16 bit functions are OK.  Clobber the size
	 to 0 to prevent it from being included in the link.  */
      h->call_stub->size = 0;
      h->call_stub->flags &= ~SEC_RELOC;
      h->call_stub->reloc_count = 0;
      h->call_stub->flags |= SEC_EXCLUDE;
      h->call_stub->output_section = bfd_abs_section_ptr;
    }

  if (h->call_fp_stub != NULL
      && ELF_ST_IS_MIPS16 (h->root.other))
    {
      /* We don't need the call_stub; this is a 16 bit function, so
	 calls from other 16 bit functions are OK.  Clobber the size
	 to 0 to prevent it from being included in the link.  */
      h->call_fp_stub->size = 0;
      h->call_fp_stub->flags &= ~SEC_RELOC;
      h->call_fp_stub->reloc_count = 0;
      h->call_fp_stub->flags |= SEC_EXCLUDE;
      h->call_fp_stub->output_section = bfd_abs_section_ptr;
    }
}

/* Hashtable callbacks for mips_elf_la25_stubs.  */

static hashval_t
mips_elf_la25_stub_hash (const void *entry_)
{
  const struct mips_elf_la25_stub *entry;

  entry = (struct mips_elf_la25_stub *) entry_;
  return entry->h->root.root.u.def.section->id
    + entry->h->root.root.u.def.value;
}

static int
mips_elf_la25_stub_eq (const void *entry1_, const void *entry2_)
{
  const struct mips_elf_la25_stub *entry1, *entry2;

  entry1 = (struct mips_elf_la25_stub *) entry1_;
  entry2 = (struct mips_elf_la25_stub *) entry2_;
  return ((entry1->h->root.root.u.def.section
	   == entry2->h->root.root.u.def.section)
	  && (entry1->h->root.root.u.def.value
	      == entry2->h->root.root.u.def.value));
}

/* Called by the linker to set up the la25 stub-creation code.  FN is
   the linker's implementation of add_stub_function.  Return true on
   success.  */

bool
_bfd_mips_elf_init_stubs (struct bfd_link_info *info,
			  asection *(*fn) (const char *, asection *,
					   asection *))
{
  struct mips_elf_link_hash_table *htab;

  htab = mips_elf_hash_table (info);
  if (htab == NULL)
    return false;

  htab->add_stub_section = fn;
  htab->la25_stubs = htab_try_create (1, mips_elf_la25_stub_hash,
				      mips_elf_la25_stub_eq, NULL);
  if (htab->la25_stubs == NULL)
    return false;

  return true;
}

/* Return true if H is a locally-defined PIC function, in the sense
   that it or its fn_stub might need $25 to be valid on entry.
   Note that MIPS16 functions set up $gp using PC-relative instructions,
   so they themselves never need $25 to be valid.  Only non-MIPS16
   entry points are of interest here.  */

static bool
mips_elf_local_pic_function_p (struct mips_elf_link_hash_entry *h)
{
  return ((h->root.root.type == bfd_link_hash_defined
	   || h->root.root.type == bfd_link_hash_defweak)
	  && h->root.def_regular
	  && !bfd_is_abs_section (h->root.root.u.def.section)
	  && !bfd_is_und_section (h->root.root.u.def.section)
	  && (!ELF_ST_IS_MIPS16 (h->root.other)
	      || (h->fn_stub && h->need_fn_stub))
	  && (PIC_OBJECT_P (h->root.root.u.def.section->owner)
	      || ELF_ST_IS_MIPS_PIC (h->root.other)));
}

/* Set *SEC to the input section that contains the target of STUB.
   Return the offset of the target from the start of that section.  */

static bfd_vma
mips_elf_get_la25_target (struct mips_elf_la25_stub *stub,
			  asection **sec)
{
  if (ELF_ST_IS_MIPS16 (stub->h->root.other))
    {
      BFD_ASSERT (stub->h->need_fn_stub);
      *sec = stub->h->fn_stub;
      return 0;
    }
  else
    {
      *sec = stub->h->root.root.u.def.section;
      return stub->h->root.root.u.def.value;
    }
}

/* STUB describes an la25 stub that we have decided to implement
   by inserting an LUI/ADDIU pair before the target function.
   Create the section and redirect the function symbol to it.  */

static bool
mips_elf_add_la25_intro (struct mips_elf_la25_stub *stub,
			 struct bfd_link_info *info)
{
  struct mips_elf_link_hash_table *htab;
  char *name;
  asection *s, *input_section;
  unsigned int align;

  htab = mips_elf_hash_table (info);
  if (htab == NULL)
    return false;

  /* Create a unique name for the new section.  */
  name = bfd_malloc (11 + sizeof (".text.stub."));
  if (name == NULL)
    return false;
  sprintf (name, ".text.stub.%d", (int) htab_elements (htab->la25_stubs));

  /* Create the section.  */
  mips_elf_get_la25_target (stub, &input_section);
  s = htab->add_stub_section (name, input_section,
			      input_section->output_section);
  if (s == NULL)
    return false;

  /* Make sure that any padding goes before the stub.  */
  align = input_section->alignment_power;
  if (!bfd_set_section_alignment (s, align))
    return false;
  if (align > 3)
    s->size = (1 << align) - 8;

  /* Create a symbol for the stub.  */
  mips_elf_create_stub_symbol (info, stub->h, ".pic.", s, s->size, 8);
  stub->stub_section = s;
  stub->offset = s->size;

  /* Allocate room for it.  */
  s->size += 8;
  return true;
}

/* STUB describes an la25 stub that we have decided to implement
   with a separate trampoline.  Allocate room for it and redirect
   the function symbol to it.  */

static bool
mips_elf_add_la25_trampoline (struct mips_elf_la25_stub *stub,
			      struct bfd_link_info *info)
{
  struct mips_elf_link_hash_table *htab;
  asection *s;

  htab = mips_elf_hash_table (info);
  if (htab == NULL)
    return false;

  /* Create a trampoline section, if we haven't already.  */
  s = htab->strampoline;
  if (s == NULL)
    {
      asection *input_section = stub->h->root.root.u.def.section;
      s = htab->add_stub_section (".text", NULL,
				  input_section->output_section);
      if (s == NULL || !bfd_set_section_alignment (s, 4))
	return false;
      htab->strampoline = s;
    }

  /* Create a symbol for the stub.  */
  mips_elf_create_stub_symbol (info, stub->h, ".pic.", s, s->size, 16);
  stub->stub_section = s;
  stub->offset = s->size;

  /* Allocate room for it.  */
  s->size += 16;
  return true;
}

/* H describes a symbol that needs an la25 stub.  Make sure that an
   appropriate stub exists and point H at it.  */

static bool
mips_elf_add_la25_stub (struct bfd_link_info *info,
			struct mips_elf_link_hash_entry *h)
{
  struct mips_elf_link_hash_table *htab;
  struct mips_elf_la25_stub search, *stub;
  bool use_trampoline_p;
  asection *s;
  bfd_vma value;
  void **slot;

  /* Describe the stub we want.  */
  search.stub_section = NULL;
  search.offset = 0;
  search.h = h;

  /* See if we've already created an equivalent stub.  */
  htab = mips_elf_hash_table (info);
  if (htab == NULL)
    return false;

  slot = htab_find_slot (htab->la25_stubs, &search, INSERT);
  if (slot == NULL)
    return false;

  stub = (struct mips_elf_la25_stub *) *slot;
  if (stub != NULL)
    {
      /* We can reuse the existing stub.  */
      h->la25_stub = stub;
      return true;
    }

  /* Create a permanent copy of ENTRY and add it to the hash table.  */
  stub = bfd_malloc (sizeof (search));
  if (stub == NULL)
    return false;
  *stub = search;
  *slot = stub;

  /* Prefer to use LUI/ADDIU stubs if the function is at the beginning
     of the section and if we would need no more than 2 nops.  */
  value = mips_elf_get_la25_target (stub, &s);
  if (ELF_ST_IS_MICROMIPS (stub->h->root.other))
    value &= ~1;
  use_trampoline_p = (value != 0 || s->alignment_power > 4);

  h->la25_stub = stub;
  return (use_trampoline_p
	  ? mips_elf_add_la25_trampoline (stub, info)
	  : mips_elf_add_la25_intro (stub, info));
}

/* A mips_elf_link_hash_traverse callback that is called before sizing
   sections.  DATA points to a mips_htab_traverse_info structure.  */

static bool
mips_elf_check_symbols (struct mips_elf_link_hash_entry *h, void *data)
{
  struct mips_htab_traverse_info *hti;

  hti = (struct mips_htab_traverse_info *) data;
  if (!bfd_link_relocatable (hti->info))
    mips_elf_check_mips16_stubs (hti->info, h);

  if (mips_elf_local_pic_function_p (h))
    {
      /* PR 12845: If H is in a section that has been garbage
	 collected it will have its output section set to *ABS*.  */
      if (bfd_is_abs_section (h->root.root.u.def.section->output_section))
	return true;

      /* H is a function that might need $25 to be valid on entry.
	 If we're creating a non-PIC relocatable object, mark H as
	 being PIC.  If we're creating a non-relocatable object with
	 non-PIC branches and jumps to H, make sure that H has an la25
	 stub.  */
      if (bfd_link_relocatable (hti->info))
	{
	  if (!PIC_OBJECT_P (hti->output_bfd))
	    h->root.other = ELF_ST_SET_MIPS_PIC (h->root.other);
	}
      else if (h->has_nonpic_branches && !mips_elf_add_la25_stub (hti->info, h))
	{
	  hti->error = true;
	  return false;
	}
    }
  return true;
}

/* R_MIPS16_26 is used for the mips16 jal and jalx instructions.
   Most mips16 instructions are 16 bits, but these instructions
   are 32 bits.

   The format of these instructions is:

   +--------------+--------------------------------+
   |     JALX     | X|   Imm 20:16  |   Imm 25:21  |
   +--------------+--------------------------------+
   |		    Immediate  15:0		   |
   +-----------------------------------------------+

   JALX is the 5-bit value 00011.  X is 0 for jal, 1 for jalx.
   Note that the immediate value in the first word is swapped.

   When producing a relocatable object file, R_MIPS16_26 is
   handled mostly like R_MIPS_26.  In particular, the addend is
   stored as a straight 26-bit value in a 32-bit instruction.
   (gas makes life simpler for itself by never adjusting a
   R_MIPS16_26 reloc to be against a section, so the addend is
   always zero).  However, the 32 bit instruction is stored as 2
   16-bit values, rather than a single 32-bit value.  In a
   big-endian file, the result is the same; in a little-endian
   file, the two 16-bit halves of the 32 bit value are swapped.
   This is so that a disassembler can recognize the jal
   instruction.

   When doing a final link, R_MIPS16_26 is treated as a 32 bit
   instruction stored as two 16-bit values.  The addend A is the
   contents of the targ26 field.  The calculation is the same as
   R_MIPS_26.  When storing the calculated value, reorder the
   immediate value as shown above, and don't forget to store the
   value as two 16-bit values.

   To put it in MIPS ABI terms, the relocation field is T-targ26-16,
   defined as

   big-endian:
   +--------+----------------------+
   |	    |			   |
   |	    |	 targ26-16	   |
   |31	  26|25			  0|
   +--------+----------------------+

   little-endian:
   +----------+------+-------------+
   |	      |	     |		   |
   |  sub1    |	     |	   sub2	   |
   |0	     9|10  15|16	 31|
   +----------+--------------------+
   where targ26-16 is sub1 followed by sub2 (i.e., the addend field A is
   ((sub1 << 16) | sub2)).

   When producing a relocatable object file, the calculation is
   (((A < 2) | ((P + 4) & 0xf0000000) + S) >> 2)
   When producing a fully linked file, the calculation is
   let R = (((A < 2) | ((P + 4) & 0xf0000000) + S) >> 2)
   ((R & 0x1f0000) << 5) | ((R & 0x3e00000) >> 5) | (R & 0xffff)

   The table below lists the other MIPS16 instruction relocations.
   Each one is calculated in the same way as the non-MIPS16 relocation
   given on the right, but using the extended MIPS16 layout of 16-bit
   immediate fields:

	R_MIPS16_GPREL		R_MIPS_GPREL16
	R_MIPS16_GOT16		R_MIPS_GOT16
	R_MIPS16_CALL16		R_MIPS_CALL16
	R_MIPS16_HI16		R_MIPS_HI16
	R_MIPS16_LO16		R_MIPS_LO16

   A typical instruction will have a format like this:

   +--------------+--------------------------------+
   |    EXTEND    |     Imm 10:5    |   Imm 15:11  |
   +--------------+--------------------------------+
   |    Major     |   rx   |   ry   |   Imm  4:0   |
   +--------------+--------------------------------+

   EXTEND is the five bit value 11110.  Major is the instruction
   opcode.

   All we need to do here is shuffle the bits appropriately.
   As above, the two 16-bit halves must be swapped on a
   little-endian system.

   Finally R_MIPS16_PC16_S1 corresponds to R_MIPS_PC16, however the
   relocatable field is shifted by 1 rather than 2 and the same bit
   shuffling is done as with the relocations above.  */

static inline bool
mips16_reloc_p (int r_type)
{
  switch (r_type)
    {
    case R_MIPS16_26:
    case R_MIPS16_GPREL:
    case R_MIPS16_GOT16:
    case R_MIPS16_CALL16:
    case R_MIPS16_HI16:
    case R_MIPS16_LO16:
    case R_MIPS16_TLS_GD:
    case R_MIPS16_TLS_LDM:
    case R_MIPS16_TLS_DTPREL_HI16:
    case R_MIPS16_TLS_DTPREL_LO16:
    case R_MIPS16_TLS_GOTTPREL:
    case R_MIPS16_TLS_TPREL_HI16:
    case R_MIPS16_TLS_TPREL_LO16:
    case R_MIPS16_PC16_S1:
      return true;

    default:
      return false;
    }
}

/* Check if a microMIPS reloc.  */

static inline bool
micromips_reloc_p (unsigned int r_type)
{
  return r_type >= R_MICROMIPS_min && r_type < R_MICROMIPS_max;
}

/* Similar to MIPS16, the two 16-bit halves in microMIPS must be swapped
   on a little-endian system.  This does not apply to R_MICROMIPS_PC7_S1
   and R_MICROMIPS_PC10_S1 relocs that apply to 16-bit instructions.  */

static inline bool
micromips_reloc_shuffle_p (unsigned int r_type)
{
  return (micromips_reloc_p (r_type)
	  && r_type != R_MICROMIPS_PC7_S1
	  && r_type != R_MICROMIPS_PC10_S1);
}

static inline bool
got16_reloc_p (int r_type)
{
  return (r_type == R_MIPS_GOT16
	  || r_type == R_MIPS16_GOT16
	  || r_type == R_MICROMIPS_GOT16);
}

static inline bool
call16_reloc_p (int r_type)
{
  return (r_type == R_MIPS_CALL16
	  || r_type == R_MIPS16_CALL16
	  || r_type == R_MICROMIPS_CALL16);
}

static inline bool
got_disp_reloc_p (unsigned int r_type)
{
  return r_type == R_MIPS_GOT_DISP || r_type == R_MICROMIPS_GOT_DISP;
}

static inline bool
got_page_reloc_p (unsigned int r_type)
{
  return r_type == R_MIPS_GOT_PAGE || r_type == R_MICROMIPS_GOT_PAGE;
}

static inline bool
got_lo16_reloc_p (unsigned int r_type)
{
  return r_type == R_MIPS_GOT_LO16 || r_type == R_MICROMIPS_GOT_LO16;
}

static inline bool
call_hi16_reloc_p (unsigned int r_type)
{
  return r_type == R_MIPS_CALL_HI16 || r_type == R_MICROMIPS_CALL_HI16;
}

static inline bool
call_lo16_reloc_p (unsigned int r_type)
{
  return r_type == R_MIPS_CALL_LO16 || r_type == R_MICROMIPS_CALL_LO16;
}

static inline bool
hi16_reloc_p (int r_type)
{
  return (r_type == R_MIPS_HI16
	  || r_type == R_MIPS16_HI16
	  || r_type == R_MICROMIPS_HI16
	  || r_type == R_MIPS_PCHI16);
}

static inline bool
lo16_reloc_p (int r_type)
{
  return (r_type == R_MIPS_LO16
	  || r_type == R_MIPS16_LO16
	  || r_type == R_MICROMIPS_LO16
	  || r_type == R_MIPS_PCLO16);
}

static inline bool
mips16_call_reloc_p (int r_type)
{
  return r_type == R_MIPS16_26 || r_type == R_MIPS16_CALL16;
}

static inline bool
jal_reloc_p (int r_type)
{
  return (r_type == R_MIPS_26
	  || r_type == R_MIPS16_26
	  || r_type == R_MICROMIPS_26_S1);
}

static inline bool
b_reloc_p (int r_type)
{
  return (r_type == R_MIPS_PC26_S2
	  || r_type == R_MIPS_PC21_S2
	  || r_type == R_MIPS_PC16
	  || r_type == R_MIPS_GNU_REL16_S2
	  || r_type == R_MIPS16_PC16_S1
	  || r_type == R_MICROMIPS_PC16_S1
	  || r_type == R_MICROMIPS_PC10_S1
	  || r_type == R_MICROMIPS_PC7_S1);
}

static inline bool
aligned_pcrel_reloc_p (int r_type)
{
  return (r_type == R_MIPS_PC18_S3
	  || r_type == R_MIPS_PC19_S2);
}

static inline bool
branch_reloc_p (int r_type)
{
  return (r_type == R_MIPS_26
	  || r_type == R_MIPS_PC26_S2
	  || r_type == R_MIPS_PC21_S2
	  || r_type == R_MIPS_PC16
	  || r_type == R_MIPS_GNU_REL16_S2);
}

static inline bool
mips16_branch_reloc_p (int r_type)
{
  return (r_type == R_MIPS16_26
	  || r_type == R_MIPS16_PC16_S1);
}

static inline bool
micromips_branch_reloc_p (int r_type)
{
  return (r_type == R_MICROMIPS_26_S1
	  || r_type == R_MICROMIPS_PC16_S1
	  || r_type == R_MICROMIPS_PC10_S1
	  || r_type == R_MICROMIPS_PC7_S1);
}

static inline bool
tls_gd_reloc_p (unsigned int r_type)
{
  return (r_type == R_MIPS_TLS_GD
	  || r_type == R_MIPS16_TLS_GD
	  || r_type == R_MICROMIPS_TLS_GD);
}

static inline bool
tls_ldm_reloc_p (unsigned int r_type)
{
  return (r_type == R_MIPS_TLS_LDM
	  || r_type == R_MIPS16_TLS_LDM
	  || r_type == R_MICROMIPS_TLS_LDM);
}

static inline bool
tls_gottprel_reloc_p (unsigned int r_type)
{
  return (r_type == R_MIPS_TLS_GOTTPREL
	  || r_type == R_MIPS16_TLS_GOTTPREL
	  || r_type == R_MICROMIPS_TLS_GOTTPREL);
}

static inline bool
needs_shuffle (int r_type)
{
  return mips16_reloc_p (r_type) || micromips_reloc_shuffle_p (r_type);
}

void
_bfd_mips_elf_reloc_unshuffle (bfd *abfd, int r_type,
			       bool jal_shuffle, bfd_byte *data)
{
  bfd_vma first, second, val;

  if (!needs_shuffle (r_type))
    return;

  /* Pick up the first and second halfwords of the instruction.  */
  first = bfd_get_16 (abfd, data);
  second = bfd_get_16 (abfd, data + 2);
  if (micromips_reloc_p (r_type) || (r_type == R_MIPS16_26 && !jal_shuffle))
    val = first << 16 | second;
  else if (r_type != R_MIPS16_26)
    val = (((first & 0xf800) << 16) | ((second & 0xffe0) << 11)
	   | ((first & 0x1f) << 11) | (first & 0x7e0) | (second & 0x1f));
  else
    val = (((first & 0xfc00) << 16) | ((first & 0x3e0) << 11)
	   | ((first & 0x1f) << 21) | second);
  bfd_put_32 (abfd, val, data);
}

void
_bfd_mips_elf_reloc_shuffle (bfd *abfd, int r_type,
			     bool jal_shuffle, bfd_byte *data)
{
  bfd_vma first, second, val;

  if (!needs_shuffle (r_type))
    return;

  val = bfd_get_32 (abfd, data);
  if (micromips_reloc_p (r_type) || (r_type == R_MIPS16_26 && !jal_shuffle))
    {
      second = val & 0xffff;
      first = val >> 16;
    }
  else if (r_type != R_MIPS16_26)
    {
      second = ((val >> 11) & 0xffe0) | (val & 0x1f);
      first = ((val >> 16) & 0xf800) | ((val >> 11) & 0x1f) | (val & 0x7e0);
    }
  else
    {
      second = val & 0xffff;
      first = ((val >> 16) & 0xfc00) | ((val >> 11) & 0x3e0)
	       | ((val >> 21) & 0x1f);
    }
  bfd_put_16 (abfd, second, data + 2);
  bfd_put_16 (abfd, first, data);
}

/* Perform reloc offset checking.
   We can only use bfd_reloc_offset_in_range, which takes into account
   the size of the field being relocated, when section contents will
   be accessed because mips object files may use relocations that seem
   to access beyond section limits.
   gas/testsuite/gas/mips/dla-reloc.s is an example that puts
   R_MIPS_SUB, a 64-bit relocation, on the last instruction in the
   section.  The R_MIPS_SUB applies to the addend for the next reloc
   rather than the section contents.

   CHECK is CHECK_STD for the standard bfd_reloc_offset_in_range check,
   CHECK_INPLACE to only check partial_inplace relocs, and
   CHECK_SHUFFLE to only check relocs that shuffle/unshuffle.  */

bool
_bfd_mips_reloc_offset_in_range (bfd *abfd, asection *input_section,
				 arelent *reloc_entry, enum reloc_check check)
{
  if (check == check_inplace && !reloc_entry->howto->partial_inplace)
    return true;
  if (check == check_shuffle && !needs_shuffle (reloc_entry->howto->type))
    return true;
  return bfd_reloc_offset_in_range (reloc_entry->howto, abfd,
				    input_section, reloc_entry->address);
}

bfd_reloc_status_type
_bfd_mips_elf_gprel16_with_gp (bfd *abfd, asymbol *symbol,
			       arelent *reloc_entry, asection *input_section,
			       bool relocatable, void *data, bfd_vma gp)
{
  bfd_vma relocation;
  bfd_signed_vma val;
  bfd_reloc_status_type status;

  if (bfd_is_com_section (symbol->section))
    relocation = 0;
  else
    relocation = symbol->value;

  if (symbol->section->output_section != NULL)
    {
      relocation += symbol->section->output_section->vma;
      relocation += symbol->section->output_offset;
    }

  /* Set val to the offset into the section or symbol.  */
  val = reloc_entry->addend;

  _bfd_mips_elf_sign_extend (val, 16);

  /* Adjust val for the final section location and GP value.  If we
     are producing relocatable output, we don't want to do this for
     an external symbol.  */
  if (! relocatable
      || (symbol->flags & BSF_SECTION_SYM) != 0)
    val += relocation - gp;

  if (reloc_entry->howto->partial_inplace)
    {
      if (!bfd_reloc_offset_in_range (reloc_entry->howto, abfd, input_section,
				      reloc_entry->address))
	return bfd_reloc_outofrange;

      status = _bfd_relocate_contents (reloc_entry->howto, abfd, val,
				       (bfd_byte *) data
				       + reloc_entry->address);
      if (status != bfd_reloc_ok)
	return status;
    }
  else
    reloc_entry->addend = val;

  if (relocatable)
    reloc_entry->address += input_section->output_offset;

  return bfd_reloc_ok;
}

/* A howto special_function for REL *HI16 relocations.  We can only
   calculate the correct value once we've seen the partnering
   *LO16 relocation, so just save the information for later.

   The ABI requires that the *LO16 immediately follow the *HI16.
   However, as a GNU extension, we permit an arbitrary number of
   *HI16s to be associated with a single *LO16.  This significantly
   simplies the relocation handling in gcc.  */

bfd_reloc_status_type
_bfd_mips_elf_hi16_reloc (bfd *abfd, arelent *reloc_entry,
			  asymbol *symbol ATTRIBUTE_UNUSED, void *data,
			  asection *input_section, bfd *output_bfd,
			  char **error_message ATTRIBUTE_UNUSED)
{
  struct mips_hi16 *n;
  struct mips_elf_obj_tdata *tdata;

  if (reloc_entry->address > bfd_get_section_limit (abfd, input_section))
    return bfd_reloc_outofrange;

  n = bfd_malloc (sizeof *n);
  if (n == NULL)
    return bfd_reloc_outofrange;

  tdata = mips_elf_tdata (abfd);
  n->next = tdata->mips_hi16_list;
  n->data = data;
  n->input_section = input_section;
  n->rel = *reloc_entry;
  tdata->mips_hi16_list = n;

  if (output_bfd != NULL)
    reloc_entry->address += input_section->output_offset;

  return bfd_reloc_ok;
}

/* A howto special_function for REL R_MIPS*_GOT16 relocations.  This is just
   like any other 16-bit relocation when applied to global symbols, but is
   treated in the same as R_MIPS_HI16 when applied to local symbols.  */

bfd_reloc_status_type
_bfd_mips_elf_got16_reloc (bfd *abfd, arelent *reloc_entry, asymbol *symbol,
			   void *data, asection *input_section,
			   bfd *output_bfd, char **error_message)
{
  if ((symbol->flags & (BSF_GLOBAL | BSF_WEAK)) != 0
      || bfd_is_und_section (bfd_asymbol_section (symbol))
      || bfd_is_com_section (bfd_asymbol_section (symbol)))
    /* The relocation is against a global symbol.  */
    return _bfd_mips_elf_generic_reloc (abfd, reloc_entry, symbol, data,
					input_section, output_bfd,
					error_message);

  return _bfd_mips_elf_hi16_reloc (abfd, reloc_entry, symbol, data,
				   input_section, output_bfd, error_message);
}

/* A howto special_function for REL *LO16 relocations.  The *LO16 itself
   is a straightforward 16 bit inplace relocation, but we must deal with
   any partnering high-part relocations as well.  */

bfd_reloc_status_type
_bfd_mips_elf_lo16_reloc (bfd *abfd, arelent *reloc_entry, asymbol *symbol,
			  void *data, asection *input_section,
			  bfd *output_bfd, char **error_message)
{
  bfd_vma vallo;
  bfd_byte *location = (bfd_byte *) data + reloc_entry->address;
  struct mips_elf_obj_tdata *tdata;

  if (!bfd_reloc_offset_in_range (reloc_entry->howto, abfd, input_section,
				  reloc_entry->address))
    return bfd_reloc_outofrange;

  _bfd_mips_elf_reloc_unshuffle (abfd, reloc_entry->howto->type, false,
				 location);
  /* The high 16 bits of the addend are stored in the high insn, the
     low 16 bits in the low insn, but there is a catch:  You can't
     just concatenate the high and low parts.  The high part of the
     addend is adjusted for the fact that the low part is sign
     extended.  For example, an addend of 0x38000 would have 0x0004 in
     the high part and 0x8000 (=0xff..f8000) in the low part.
     To extract the actual addend, calculate (a)
     ((hi & 0xffff) << 16) + ((lo & 0xffff) ^ 0x8000) - 0x8000.
     We will be applying (symbol + addend) & 0xffff to the low insn,
     and we want to apply (b) (symbol + addend + 0x8000) >> 16 to the
     high insn (the +0x8000 adjusting for when the applied low part is
     negative).  Substituting (a) into (b) and recognising that
     (hi & 0xffff) is already in the high insn gives a high part
     addend adjustment of (lo & 0xffff) ^ 0x8000.  */
  vallo = (bfd_get_32 (abfd, location) & 0xffff) ^ 0x8000;
  _bfd_mips_elf_reloc_shuffle (abfd, reloc_entry->howto->type, false,
			       location);

  tdata = mips_elf_tdata (abfd);
  while (tdata->mips_hi16_list != NULL)
    {
      bfd_reloc_status_type ret;
      struct mips_hi16 *hi;

      hi = tdata->mips_hi16_list;

      /* R_MIPS*_GOT16 relocations are something of a special case.  We
	 want to install the addend in the same way as for a R_MIPS*_HI16
	 relocation (with a rightshift of 16).  However, since GOT16
	 relocations can also be used with global symbols, their howto
	 has a rightshift of 0.  */
      if (hi->rel.howto->type == R_MIPS_GOT16)
	hi->rel.howto = MIPS_ELF_RTYPE_TO_HOWTO (abfd, R_MIPS_HI16, false);
      else if (hi->rel.howto->type == R_MIPS16_GOT16)
	hi->rel.howto = MIPS_ELF_RTYPE_TO_HOWTO (abfd, R_MIPS16_HI16, false);
      else if (hi->rel.howto->type == R_MICROMIPS_GOT16)
	hi->rel.howto = MIPS_ELF_RTYPE_TO_HOWTO (abfd, R_MICROMIPS_HI16, false);

      hi->rel.addend += vallo;

      ret = _bfd_mips_elf_generic_reloc (abfd, &hi->rel, symbol, hi->data,
					 hi->input_section, output_bfd,
					 error_message);
      if (ret != bfd_reloc_ok)
	return ret;

      tdata->mips_hi16_list = hi->next;
      free (hi);
    }

  return _bfd_mips_elf_generic_reloc (abfd, reloc_entry, symbol, data,
				      input_section, output_bfd,
				      error_message);
}

/* A generic howto special_function.  This calculates and installs the
   relocation itself, thus avoiding the oft-discussed problems in
   bfd_perform_relocation and bfd_install_relocation.  */

bfd_reloc_status_type
_bfd_mips_elf_generic_reloc (bfd *abfd ATTRIBUTE_UNUSED, arelent *reloc_entry,
			     asymbol *symbol, void *data ATTRIBUTE_UNUSED,
			     asection *input_section, bfd *output_bfd,
			     char **error_message ATTRIBUTE_UNUSED)
{
  bfd_signed_vma val;
  bfd_reloc_status_type status;
  bool relocatable;

  relocatable = (output_bfd != NULL);

  if (!_bfd_mips_reloc_offset_in_range (abfd, input_section, reloc_entry,
					(relocatable
					 ? check_inplace : check_std)))
    return bfd_reloc_outofrange;

  /* Build up the field adjustment in VAL.  */
  val = 0;
  if ((!relocatable || (symbol->flags & BSF_SECTION_SYM) != 0)
      && symbol->section->output_section != NULL)
    {
      /* Either we're calculating the final field value or we have a
	 relocation against a section symbol.  Add in the section's
	 offset or address.  */
      val += symbol->section->output_section->vma;
      val += symbol->section->output_offset;
    }

  if (!relocatable)
    {
      /* We're calculating the final field value.  Add in the symbol's value
	 and, if pc-relative, subtract the address of the field itself.  */
      val += symbol->value;
      if (reloc_entry->howto->pc_relative)
	{
	  val -= input_section->output_section->vma;
	  val -= input_section->output_offset;
	  val -= reloc_entry->address;
	}
    }

  /* VAL is now the final adjustment.  If we're keeping this relocation
     in the output file, and if the relocation uses a separate addend,
     we just need to add VAL to that addend.  Otherwise we need to add
     VAL to the relocation field itself.  */
  if (relocatable && !reloc_entry->howto->partial_inplace)
    reloc_entry->addend += val;
  else
    {
      bfd_byte *location = (bfd_byte *) data + reloc_entry->address;

      /* Add in the separate addend, if any.  */
      val += reloc_entry->addend;

      /* Add VAL to the relocation field.  */
      _bfd_mips_elf_reloc_unshuffle (abfd, reloc_entry->howto->type, false,
				     location);
      status = _bfd_relocate_contents (reloc_entry->howto, abfd, val,
				       location);
      _bfd_mips_elf_reloc_shuffle (abfd, reloc_entry->howto->type, false,
				   location);

      if (status != bfd_reloc_ok)
	return status;
    }

  if (relocatable)
    reloc_entry->address += input_section->output_offset;

  return bfd_reloc_ok;
}

/* Swap an entry in a .gptab section.  Note that these routines rely
   on the equivalence of the two elements of the union.  */

static void
bfd_mips_elf32_swap_gptab_in (bfd *abfd, const Elf32_External_gptab *ex,
			      Elf32_gptab *in)
{
  in->gt_entry.gt_g_value = H_GET_32 (abfd, ex->gt_entry.gt_g_value);
  in->gt_entry.gt_bytes = H_GET_32 (abfd, ex->gt_entry.gt_bytes);
}

static void
bfd_mips_elf32_swap_gptab_out (bfd *abfd, const Elf32_gptab *in,
			       Elf32_External_gptab *ex)
{
  H_PUT_32 (abfd, in->gt_entry.gt_g_value, ex->gt_entry.gt_g_value);
  H_PUT_32 (abfd, in->gt_entry.gt_bytes, ex->gt_entry.gt_bytes);
}

static void
bfd_elf32_swap_compact_rel_out (bfd *abfd, const Elf32_compact_rel *in,
				Elf32_External_compact_rel *ex)
{
  H_PUT_32 (abfd, in->id1, ex->id1);
  H_PUT_32 (abfd, in->num, ex->num);
  H_PUT_32 (abfd, in->id2, ex->id2);
  H_PUT_32 (abfd, in->offset, ex->offset);
  H_PUT_32 (abfd, in->reserved0, ex->reserved0);
  H_PUT_32 (abfd, in->reserved1, ex->reserved1);
}

static void
bfd_elf32_swap_crinfo_out (bfd *abfd, const Elf32_crinfo *in,
			   Elf32_External_crinfo *ex)
{
  unsigned long l;

  l = (((in->ctype & CRINFO_CTYPE) << CRINFO_CTYPE_SH)
       | ((in->rtype & CRINFO_RTYPE) << CRINFO_RTYPE_SH)
       | ((in->dist2to & CRINFO_DIST2TO) << CRINFO_DIST2TO_SH)
       | ((in->relvaddr & CRINFO_RELVADDR) << CRINFO_RELVADDR_SH));
  H_PUT_32 (abfd, l, ex->info);
  H_PUT_32 (abfd, in->konst, ex->konst);
  H_PUT_32 (abfd, in->vaddr, ex->vaddr);
}

/* A .reginfo section holds a single Elf32_RegInfo structure.  These
   routines swap this structure in and out.  They are used outside of
   BFD, so they are globally visible.  */

void
bfd_mips_elf32_swap_reginfo_in (bfd *abfd, const Elf32_External_RegInfo *ex,
				Elf32_RegInfo *in)
{
  in->ri_gprmask = H_GET_32 (abfd, ex->ri_gprmask);
  in->ri_cprmask[0] = H_GET_32 (abfd, ex->ri_cprmask[0]);
  in->ri_cprmask[1] = H_GET_32 (abfd, ex->ri_cprmask[1]);
  in->ri_cprmask[2] = H_GET_32 (abfd, ex->ri_cprmask[2]);
  in->ri_cprmask[3] = H_GET_32 (abfd, ex->ri_cprmask[3]);
  in->ri_gp_value = H_GET_32 (abfd, ex->ri_gp_value);
}

void
bfd_mips_elf32_swap_reginfo_out (bfd *abfd, const Elf32_RegInfo *in,
				 Elf32_External_RegInfo *ex)
{
  H_PUT_32 (abfd, in->ri_gprmask, ex->ri_gprmask);
  H_PUT_32 (abfd, in->ri_cprmask[0], ex->ri_cprmask[0]);
  H_PUT_32 (abfd, in->ri_cprmask[1], ex->ri_cprmask[1]);
  H_PUT_32 (abfd, in->ri_cprmask[2], ex->ri_cprmask[2]);
  H_PUT_32 (abfd, in->ri_cprmask[3], ex->ri_cprmask[3]);
  H_PUT_32 (abfd, in->ri_gp_value, ex->ri_gp_value);
}

/* In the 64 bit ABI, the .MIPS.options section holds register
   information in an Elf64_Reginfo structure.  These routines swap
   them in and out.  They are globally visible because they are used
   outside of BFD.  These routines are here so that gas can call them
   without worrying about whether the 64 bit ABI has been included.  */

void
bfd_mips_elf64_swap_reginfo_in (bfd *abfd, const Elf64_External_RegInfo *ex,
				Elf64_Internal_RegInfo *in)
{
  in->ri_gprmask = H_GET_32 (abfd, ex->ri_gprmask);
  in->ri_pad = H_GET_32 (abfd, ex->ri_pad);
  in->ri_cprmask[0] = H_GET_32 (abfd, ex->ri_cprmask[0]);
  in->ri_cprmask[1] = H_GET_32 (abfd, ex->ri_cprmask[1]);
  in->ri_cprmask[2] = H_GET_32 (abfd, ex->ri_cprmask[2]);
  in->ri_cprmask[3] = H_GET_32 (abfd, ex->ri_cprmask[3]);
  in->ri_gp_value = H_GET_64 (abfd, ex->ri_gp_value);
}

void
bfd_mips_elf64_swap_reginfo_out (bfd *abfd, const Elf64_Internal_RegInfo *in,
				 Elf64_External_RegInfo *ex)
{
  H_PUT_32 (abfd, in->ri_gprmask, ex->ri_gprmask);
  H_PUT_32 (abfd, in->ri_pad, ex->ri_pad);
  H_PUT_32 (abfd, in->ri_cprmask[0], ex->ri_cprmask[0]);
  H_PUT_32 (abfd, in->ri_cprmask[1], ex->ri_cprmask[1]);
  H_PUT_32 (abfd, in->ri_cprmask[2], ex->ri_cprmask[2]);
  H_PUT_32 (abfd, in->ri_cprmask[3], ex->ri_cprmask[3]);
  H_PUT_64 (abfd, in->ri_gp_value, ex->ri_gp_value);
}

/* Swap in an options header.  */

void
bfd_mips_elf_swap_options_in (bfd *abfd, const Elf_External_Options *ex,
			      Elf_Internal_Options *in)
{
  in->kind = H_GET_8 (abfd, ex->kind);
  in->size = H_GET_8 (abfd, ex->size);
  in->section = H_GET_16 (abfd, ex->section);
  in->info = H_GET_32 (abfd, ex->info);
}

/* Swap out an options header.  */

void
bfd_mips_elf_swap_options_out (bfd *abfd, const Elf_Internal_Options *in,
			       Elf_External_Options *ex)
{
  H_PUT_8 (abfd, in->kind, ex->kind);
  H_PUT_8 (abfd, in->size, ex->size);
  H_PUT_16 (abfd, in->section, ex->section);
  H_PUT_32 (abfd, in->info, ex->info);
}

/* Swap in an abiflags structure.  */

void
bfd_mips_elf_swap_abiflags_v0_in (bfd *abfd,
				  const Elf_External_ABIFlags_v0 *ex,
				  Elf_Internal_ABIFlags_v0 *in)
{
  in->version = H_GET_16 (abfd, ex->version);
  in->isa_level = H_GET_8 (abfd, ex->isa_level);
  in->isa_rev = H_GET_8 (abfd, ex->isa_rev);
  in->gpr_size = H_GET_8 (abfd, ex->gpr_size);
  in->cpr1_size = H_GET_8 (abfd, ex->cpr1_size);
  in->cpr2_size = H_GET_8 (abfd, ex->cpr2_size);
  in->fp_abi = H_GET_8 (abfd, ex->fp_abi);
  in->isa_ext = H_GET_32 (abfd, ex->isa_ext);
  in->ases = H_GET_32 (abfd, ex->ases);
  in->flags1 = H_GET_32 (abfd, ex->flags1);
  in->flags2 = H_GET_32 (abfd, ex->flags2);
}

/* Swap out an abiflags structure.  */

void
bfd_mips_elf_swap_abiflags_v0_out (bfd *abfd,
				   const Elf_Internal_ABIFlags_v0 *in,
				   Elf_External_ABIFlags_v0 *ex)
{
  H_PUT_16 (abfd, in->version, ex->version);
  H_PUT_8 (abfd, in->isa_level, ex->isa_level);
  H_PUT_8 (abfd, in->isa_rev, ex->isa_rev);
  H_PUT_8 (abfd, in->gpr_size, ex->gpr_size);
  H_PUT_8 (abfd, in->cpr1_size, ex->cpr1_size);
  H_PUT_8 (abfd, in->cpr2_size, ex->cpr2_size);
  H_PUT_8 (abfd, in->fp_abi, ex->fp_abi);
  H_PUT_32 (abfd, in->isa_ext, ex->isa_ext);
  H_PUT_32 (abfd, in->ases, ex->ases);
  H_PUT_32 (abfd, in->flags1, ex->flags1);
  H_PUT_32 (abfd, in->flags2, ex->flags2);
}

/* This function is called via qsort() to sort the dynamic relocation
   entries by increasing r_symndx value.  */

static int
sort_dynamic_relocs (const void *arg1, const void *arg2)
{
  Elf_Internal_Rela int_reloc1;
  Elf_Internal_Rela int_reloc2;
  int diff;

  bfd_elf32_swap_reloc_in (reldyn_sorting_bfd, arg1, &int_reloc1);
  bfd_elf32_swap_reloc_in (reldyn_sorting_bfd, arg2, &int_reloc2);

  diff = ELF32_R_SYM (int_reloc1.r_info) - ELF32_R_SYM (int_reloc2.r_info);
  if (diff != 0)
    return diff;

  if (int_reloc1.r_offset < int_reloc2.r_offset)
    return -1;
  if (int_reloc1.r_offset > int_reloc2.r_offset)
    return 1;
  return 0;
}

/* Like sort_dynamic_relocs, but used for elf64 relocations.  */

static int
sort_dynamic_relocs_64 (const void *arg1 ATTRIBUTE_UNUSED,
			const void *arg2 ATTRIBUTE_UNUSED)
{
#ifdef BFD64
  Elf_Internal_Rela int_reloc1[3];
  Elf_Internal_Rela int_reloc2[3];

  (*get_elf_backend_data (reldyn_sorting_bfd)->s->swap_reloc_in)
    (reldyn_sorting_bfd, arg1, int_reloc1);
  (*get_elf_backend_data (reldyn_sorting_bfd)->s->swap_reloc_in)
    (reldyn_sorting_bfd, arg2, int_reloc2);

  if (ELF64_R_SYM (int_reloc1[0].r_info) < ELF64_R_SYM (int_reloc2[0].r_info))
    return -1;
  if (ELF64_R_SYM (int_reloc1[0].r_info) > ELF64_R_SYM (int_reloc2[0].r_info))
    return 1;

  if (int_reloc1[0].r_offset < int_reloc2[0].r_offset)
    return -1;
  if (int_reloc1[0].r_offset > int_reloc2[0].r_offset)
    return 1;
  return 0;
#else
  abort ();
#endif
}


/* This routine is used to write out ECOFF debugging external symbol
   information.  It is called via mips_elf_link_hash_traverse.  The
   ECOFF external symbol information must match the ELF external
   symbol information.  Unfortunately, at this point we don't know
   whether a symbol is required by reloc information, so the two
   tables may wind up being different.  We must sort out the external
   symbol information before we can set the final size of the .mdebug
   section, and we must set the size of the .mdebug section before we
   can relocate any sections, and we can't know which symbols are
   required by relocation until we relocate the sections.
   Fortunately, it is relatively unlikely that any symbol will be
   stripped but required by a reloc.  In particular, it can not happen
   when generating a final executable.  */

static bool
mips_elf_output_extsym (struct mips_elf_link_hash_entry *h, void *data)
{
  struct extsym_info *einfo = data;
  bool strip;
  asection *sec, *output_section;

  if (h->root.indx == -2)
    strip = false;
  else if ((h->root.def_dynamic
	    || h->root.ref_dynamic
	    || h->root.type == bfd_link_hash_new)
	   && !h->root.def_regular
	   && !h->root.ref_regular)
    strip = true;
  else if (einfo->info->strip == strip_all
	   || (einfo->info->strip == strip_some
	       && bfd_hash_lookup (einfo->info->keep_hash,
				   h->root.root.root.string,
				   false, false) == NULL))
    strip = true;
  else
    strip = false;

  if (strip)
    return true;

  if (h->esym.ifd == -2)
    {
      h->esym.jmptbl = 0;
      h->esym.cobol_main = 0;
      h->esym.weakext = 0;
      h->esym.reserved = 0;
      h->esym.ifd = ifdNil;
      h->esym.asym.value = 0;
      h->esym.asym.st = stGlobal;

      if (h->root.root.type == bfd_link_hash_undefined
	  || h->root.root.type == bfd_link_hash_undefweak)
	{
	  const char *name;

	  /* Use undefined class.  Also, set class and type for some
	     special symbols.  */
	  name = h->root.root.root.string;
	  if (strcmp (name, mips_elf_dynsym_rtproc_names[0]) == 0
	      || strcmp (name, mips_elf_dynsym_rtproc_names[1]) == 0)
	    {
	      h->esym.asym.sc = scData;
	      h->esym.asym.st = stLabel;
	      h->esym.asym.value = 0;
	    }
	  else if (strcmp (name, mips_elf_dynsym_rtproc_names[2]) == 0)
	    {
	      h->esym.asym.sc = scAbs;
	      h->esym.asym.st = stLabel;
	      h->esym.asym.value =
		mips_elf_hash_table (einfo->info)->procedure_count;
	    }
	  else
	    h->esym.asym.sc = scUndefined;
	}
      else if (h->root.root.type != bfd_link_hash_defined
	  && h->root.root.type != bfd_link_hash_defweak)
	h->esym.asym.sc = scAbs;
      else
	{
	  const char *name;

	  sec = h->root.root.u.def.section;
	  output_section = sec->output_section;

	  /* When making a shared library and symbol h is the one from
	     the another shared library, OUTPUT_SECTION may be null.  */
	  if (output_section == NULL)
	    h->esym.asym.sc = scUndefined;
	  else
	    {
	      name = bfd_section_name (output_section);

	      if (strcmp (name, ".text") == 0)
		h->esym.asym.sc = scText;
	      else if (strcmp (name, ".data") == 0)
		h->esym.asym.sc = scData;
	      else if (strcmp (name, ".sdata") == 0)
		h->esym.asym.sc = scSData;
	      else if (strcmp (name, ".rodata") == 0
		       || strcmp (name, ".rdata") == 0)
		h->esym.asym.sc = scRData;
	      else if (strcmp (name, ".bss") == 0)
		h->esym.asym.sc = scBss;
	      else if (strcmp (name, ".sbss") == 0)
		h->esym.asym.sc = scSBss;
	      else if (strcmp (name, ".init") == 0)
		h->esym.asym.sc = scInit;
	      else if (strcmp (name, ".fini") == 0)
		h->esym.asym.sc = scFini;
	      else
		h->esym.asym.sc = scAbs;
	    }
	}

      h->esym.asym.reserved = 0;
      h->esym.asym.index = indexNil;
    }

  if (h->root.root.type == bfd_link_hash_common)
    h->esym.asym.value = h->root.root.u.c.size;
  else if (h->root.root.type == bfd_link_hash_defined
	   || h->root.root.type == bfd_link_hash_defweak)
    {
      if (h->esym.asym.sc == scCommon)
	h->esym.asym.sc = scBss;
      else if (h->esym.asym.sc == scSCommon)
	h->esym.asym.sc = scSBss;

      sec = h->root.root.u.def.section;
      output_section = sec->output_section;
      if (output_section != NULL)
	h->esym.asym.value = (h->root.root.u.def.value
			      + sec->output_offset
			      + output_section->vma);
      else
	h->esym.asym.value = 0;
    }
  else
    {
      struct mips_elf_link_hash_entry *hd = h;

      while (hd->root.root.type == bfd_link_hash_indirect)
	hd = (struct mips_elf_link_hash_entry *)h->root.root.u.i.link;

      if (hd->needs_lazy_stub)
	{
	  BFD_ASSERT (hd->root.plt.plist != NULL);
	  BFD_ASSERT (hd->root.plt.plist->stub_offset != MINUS_ONE);
	  /* Set type and value for a symbol with a function stub.  */
	  h->esym.asym.st = stProc;
	  sec = hd->root.root.u.def.section;
	  if (sec == NULL)
	    h->esym.asym.value = 0;
	  else
	    {
	      output_section = sec->output_section;
	      if (output_section != NULL)
		h->esym.asym.value = (hd->root.plt.plist->stub_offset
				      + sec->output_offset
				      + output_section->vma);
	      else
		h->esym.asym.value = 0;
	    }
	}
    }

  if (! bfd_ecoff_debug_one_external (einfo->abfd, einfo->debug, einfo->swap,
				      h->root.root.root.string,
				      &h->esym))
    {
      einfo->failed = true;
      return false;
    }

  return true;
}

/* A comparison routine used to sort .gptab entries.  */

static int
gptab_compare (const void *p1, const void *p2)
{
  const Elf32_gptab *a1 = p1;
  const Elf32_gptab *a2 = p2;

  return a1->gt_entry.gt_g_value - a2->gt_entry.gt_g_value;
}

/* Functions to manage the got entry hash table.  */

/* Use all 64 bits of a bfd_vma for the computation of a 32-bit
   hash number.  */

static inline hashval_t
mips_elf_hash_bfd_vma (bfd_vma addr)
{
#ifdef BFD64
  return addr + (addr >> 32);
#else
  return addr;
#endif
}

static hashval_t
mips_elf_got_entry_hash (const void *entry_)
{
  const struct mips_got_entry *entry = (struct mips_got_entry *)entry_;

  return (entry->symndx
	  + ((entry->tls_type == GOT_TLS_LDM) << 18)
	  + (entry->tls_type == GOT_TLS_LDM ? 0
	     : !entry->abfd ? mips_elf_hash_bfd_vma (entry->d.address)
	     : entry->symndx >= 0 ? (entry->abfd->id
				     + mips_elf_hash_bfd_vma (entry->d.addend))
	     : entry->d.h->root.root.root.hash));
}

static int
mips_elf_got_entry_eq (const void *entry1, const void *entry2)
{
  const struct mips_got_entry *e1 = (struct mips_got_entry *)entry1;
  const struct mips_got_entry *e2 = (struct mips_got_entry *)entry2;

  return (e1->symndx == e2->symndx
	  && e1->tls_type == e2->tls_type
	  && (e1->tls_type == GOT_TLS_LDM ? true
	      : !e1->abfd ? !e2->abfd && e1->d.address == e2->d.address
	      : e1->symndx >= 0 ? (e1->abfd == e2->abfd
				   && e1->d.addend == e2->d.addend)
	      : e2->abfd && e1->d.h == e2->d.h));
}

static hashval_t
mips_got_page_ref_hash (const void *ref_)
{
  const struct mips_got_page_ref *ref;

  ref = (const struct mips_got_page_ref *) ref_;
  return ((ref->symndx >= 0
	   ? (hashval_t) (ref->u.abfd->id + ref->symndx)
	   : ref->u.h->root.root.root.hash)
	  + mips_elf_hash_bfd_vma (ref->addend));
}

static int
mips_got_page_ref_eq (const void *ref1_, const void *ref2_)
{
  const struct mips_got_page_ref *ref1, *ref2;

  ref1 = (const struct mips_got_page_ref *) ref1_;
  ref2 = (const struct mips_got_page_ref *) ref2_;
  return (ref1->symndx == ref2->symndx
	  && (ref1->symndx < 0
	      ? ref1->u.h == ref2->u.h
	      : ref1->u.abfd == ref2->u.abfd)
	  && ref1->addend == ref2->addend);
}

static hashval_t
mips_got_page_entry_hash (const void *entry_)
{
  const struct mips_got_page_entry *entry;

  entry = (const struct mips_got_page_entry *) entry_;
  return entry->sec->id;
}

static int
mips_got_page_entry_eq (const void *entry1_, const void *entry2_)
{
  const struct mips_got_page_entry *entry1, *entry2;

  entry1 = (const struct mips_got_page_entry *) entry1_;
  entry2 = (const struct mips_got_page_entry *) entry2_;
  return entry1->sec == entry2->sec;
}

/* Create and return a new mips_got_info structure.  */

static struct mips_got_info *
mips_elf_create_got_info (bfd *abfd)
{
  struct mips_got_info *g;

  g = bfd_zalloc (abfd, sizeof (struct mips_got_info));
  if (g == NULL)
    return NULL;

  g->got_entries = htab_try_create (1, mips_elf_got_entry_hash,
				    mips_elf_got_entry_eq, NULL);
  if (g->got_entries == NULL)
    return NULL;

  g->got_page_refs = htab_try_create (1, mips_got_page_ref_hash,
				      mips_got_page_ref_eq, NULL);
  if (g->got_page_refs == NULL)
    return NULL;

  return g;
}

/* Return the GOT info for input bfd ABFD, trying to create a new one if
   CREATE_P and if ABFD doesn't already have a GOT.  */

static struct mips_got_info *
mips_elf_bfd_got (bfd *abfd, bool create_p)
{
  struct mips_elf_obj_tdata *tdata;

  if (!is_mips_elf (abfd))
    return NULL;

  tdata = mips_elf_tdata (abfd);
  if (!tdata->got && create_p)
    tdata->got = mips_elf_create_got_info (abfd);
  return tdata->got;
}

/* Record that ABFD should use output GOT G.  */

static void
mips_elf_replace_bfd_got (bfd *abfd, struct mips_got_info *g)
{
  struct mips_elf_obj_tdata *tdata;

  BFD_ASSERT (is_mips_elf (abfd));
  tdata = mips_elf_tdata (abfd);
  if (tdata->got)
    {
      /* The GOT structure itself and the hash table entries are
	 allocated to a bfd, but the hash tables aren't.  */
      htab_delete (tdata->got->got_entries);
      htab_delete (tdata->got->got_page_refs);
      if (tdata->got->got_page_entries)
	htab_delete (tdata->got->got_page_entries);
    }
  tdata->got = g;
}

/* Return the dynamic relocation section.  If it doesn't exist, try to
   create a new it if CREATE_P, otherwise return NULL.  Also return NULL
   if creation fails.  */

static asection *
mips_elf_rel_dyn_section (struct bfd_link_info *info, bool create_p)
{
  const char *dname;
  asection *sreloc;
  bfd *dynobj;

  dname = MIPS_ELF_REL_DYN_NAME (info);
  dynobj = elf_hash_table (info)->dynobj;
  sreloc = bfd_get_linker_section (dynobj, dname);
  if (sreloc == NULL && create_p)
    {
      sreloc = bfd_make_section_anyway_with_flags (dynobj, dname,
						   (SEC_ALLOC
						    | SEC_LOAD
						    | SEC_HAS_CONTENTS
						    | SEC_IN_MEMORY
						    | SEC_LINKER_CREATED
						    | SEC_READONLY));
      if (sreloc == NULL
	  || !bfd_set_section_alignment (sreloc,
					 MIPS_ELF_LOG_FILE_ALIGN (dynobj)))
	return NULL;
    }
  return sreloc;
}

/* Return the GOT_TLS_* type required by relocation type R_TYPE.  */

static int
mips_elf_reloc_tls_type (unsigned int r_type)
{
  if (tls_gd_reloc_p (r_type))
    return GOT_TLS_GD;

  if (tls_ldm_reloc_p (r_type))
    return GOT_TLS_LDM;

  if (tls_gottprel_reloc_p (r_type))
    return GOT_TLS_IE;

  return GOT_TLS_NONE;
}

/* Return the number of GOT slots needed for GOT TLS type TYPE.  */

static int
mips_tls_got_entries (unsigned int type)
{
  switch (type)
    {
    case GOT_TLS_GD:
    case GOT_TLS_LDM:
      return 2;

    case GOT_TLS_IE:
      return 1;

    case GOT_TLS_NONE:
      return 0;
    }
  abort ();
}

/* Count the number of relocations needed for a TLS GOT entry, with
   access types from TLS_TYPE, and symbol H (or a local symbol if H
   is NULL).  */

static int
mips_tls_got_relocs (struct bfd_link_info *info, unsigned char tls_type,
		     struct elf_link_hash_entry *h)
{
  int indx = 0;
  bool need_relocs = false;
  bool dyn = elf_hash_table (info)->dynamic_sections_created;

  if (h != NULL
      && h->dynindx != -1
      && WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn, bfd_link_pic (info), h)
      && (bfd_link_dll (info) || !SYMBOL_REFERENCES_LOCAL (info, h)))
    indx = h->dynindx;

  if ((bfd_link_dll (info) || indx != 0)
      && (h == NULL
	  || ELF_ST_VISIBILITY (h->other) == STV_DEFAULT
	  || h->root.type != bfd_link_hash_undefweak))
    need_relocs = true;

  if (!need_relocs)
    return 0;

  switch (tls_type)
    {
    case GOT_TLS_GD:
      return indx != 0 ? 2 : 1;

    case GOT_TLS_IE:
      return 1;

    case GOT_TLS_LDM:
      return bfd_link_dll (info) ? 1 : 0;

    default:
      return 0;
    }
}

/* Add the number of GOT entries and TLS relocations required by ENTRY
   to G.  */

static void
mips_elf_count_got_entry (struct bfd_link_info *info,
			  struct mips_got_info *g,
			  struct mips_got_entry *entry)
{
  if (entry->tls_type)
    {
      g->tls_gotno += mips_tls_got_entries (entry->tls_type);
      g->relocs += mips_tls_got_relocs (info, entry->tls_type,
					entry->symndx < 0
					? &entry->d.h->root : NULL);
    }
  else if (entry->symndx >= 0 || entry->d.h->global_got_area == GGA_NONE)
    g->local_gotno += 1;
  else
    g->global_gotno += 1;
}

/* Output a simple dynamic relocation into SRELOC.  */

static void
mips_elf_output_dynamic_relocation (bfd *output_bfd,
				    asection *sreloc,
				    unsigned long reloc_index,
				    unsigned long indx,
				    int r_type,
				    bfd_vma offset)
{
  Elf_Internal_Rela rel[3];

  memset (rel, 0, sizeof (rel));

  rel[0].r_info = ELF_R_INFO (output_bfd, indx, r_type);
  rel[0].r_offset = rel[1].r_offset = rel[2].r_offset = offset;

  if (ABI_64_P (output_bfd))
    {
      (*get_elf_backend_data (output_bfd)->s->swap_reloc_out)
	(output_bfd, &rel[0],
	 (sreloc->contents
	  + reloc_index * sizeof (Elf64_Mips_External_Rel)));
    }
  else
    bfd_elf32_swap_reloc_out
      (output_bfd, &rel[0],
       (sreloc->contents
	+ reloc_index * sizeof (Elf32_External_Rel)));
}

/* Initialize a set of TLS GOT entries for one symbol.  */

static void
mips_elf_initialize_tls_slots (bfd *abfd, struct bfd_link_info *info,
			       struct mips_got_entry *entry,
			       struct mips_elf_link_hash_entry *h,
			       bfd_vma value)
{
  bool dyn = elf_hash_table (info)->dynamic_sections_created;
  struct mips_elf_link_hash_table *htab;
  int indx;
  asection *sreloc, *sgot;
  bfd_vma got_offset, got_offset2;
  bool need_relocs = false;

  htab = mips_elf_hash_table (info);
  if (htab == NULL)
    return;

  sgot = htab->root.sgot;

  indx = 0;
  if (h != NULL
      && h->root.dynindx != -1
      && WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn, bfd_link_pic (info), &h->root)
      && (bfd_link_dll (info) || !SYMBOL_REFERENCES_LOCAL (info, &h->root)))
    indx = h->root.dynindx;

  if (entry->tls_initialized)
    return;

  if ((bfd_link_dll (info) || indx != 0)
      && (h == NULL
	  || ELF_ST_VISIBILITY (h->root.other) == STV_DEFAULT
	  || h->root.type != bfd_link_hash_undefweak))
    need_relocs = true;

  /* MINUS_ONE means the symbol is not defined in this object.  It may not
     be defined at all; assume that the value doesn't matter in that
     case.  Otherwise complain if we would use the value.  */
  BFD_ASSERT (value != MINUS_ONE || (indx != 0 && need_relocs)
	      || h->root.root.type == bfd_link_hash_undefweak);

  /* Emit necessary relocations.  */
  sreloc = mips_elf_rel_dyn_section (info, false);
  got_offset = entry->gotidx;

  switch (entry->tls_type)
    {
    case GOT_TLS_GD:
      /* General Dynamic.  */
      got_offset2 = got_offset + MIPS_ELF_GOT_SIZE (abfd);

      if (need_relocs)
	{
	  mips_elf_output_dynamic_relocation
	    (abfd, sreloc, sreloc->reloc_count++, indx,
	     ABI_64_P (abfd) ? R_MIPS_TLS_DTPMOD64 : R_MIPS_TLS_DTPMOD32,
	     sgot->output_offset + sgot->output_section->vma + got_offset);

	  if (indx)
	    mips_elf_output_dynamic_relocation
	      (abfd, sreloc, sreloc->reloc_count++, indx,
	       ABI_64_P (abfd) ? R_MIPS_TLS_DTPREL64 : R_MIPS_TLS_DTPREL32,
	       sgot->output_offset + sgot->output_section->vma + got_offset2);
	  else
	    MIPS_ELF_PUT_WORD (abfd, value - dtprel_base (info),
			       sgot->contents + got_offset2);
	}
      else
	{
	  MIPS_ELF_PUT_WORD (abfd, 1,
			     sgot->contents + got_offset);
	  MIPS_ELF_PUT_WORD (abfd, value - dtprel_base (info),
			     sgot->contents + got_offset2);
	}
      break;

    case GOT_TLS_IE:
      /* Initial Exec model.  */
      if (need_relocs)
	{
	  if (indx == 0)
	    MIPS_ELF_PUT_WORD (abfd, value - elf_hash_table (info)->tls_sec->vma,
			       sgot->contents + got_offset);
	  else
	    MIPS_ELF_PUT_WORD (abfd, 0,
			       sgot->contents + got_offset);

	  mips_elf_output_dynamic_relocation
	    (abfd, sreloc, sreloc->reloc_count++, indx,
	     ABI_64_P (abfd) ? R_MIPS_TLS_TPREL64 : R_MIPS_TLS_TPREL32,
	     sgot->output_offset + sgot->output_section->vma + got_offset);
	}
      else
	MIPS_ELF_PUT_WORD (abfd, value - tprel_base (info),
			   sgot->contents + got_offset);
      break;

    case GOT_TLS_LDM:
      /* The initial offset is zero, and the LD offsets will include the
	 bias by DTP_OFFSET.  */
      MIPS_ELF_PUT_WORD (abfd, 0,
			 sgot->contents + got_offset
			 + MIPS_ELF_GOT_SIZE (abfd));

      if (!bfd_link_dll (info))
	MIPS_ELF_PUT_WORD (abfd, 1,
			   sgot->contents + got_offset);
      else
	mips_elf_output_dynamic_relocation
	  (abfd, sreloc, sreloc->reloc_count++, indx,
	   ABI_64_P (abfd) ? R_MIPS_TLS_DTPMOD64 : R_MIPS_TLS_DTPMOD32,
	   sgot->output_offset + sgot->output_section->vma + got_offset);
      break;

    default:
      abort ();
    }

  entry->tls_initialized = true;
}

/* Return the offset from _GLOBAL_OFFSET_TABLE_ of the .got.plt entry
   for global symbol H.  .got.plt comes before the GOT, so the offset
   will be negative.  */

static bfd_vma
mips_elf_gotplt_index (struct bfd_link_info *info,
		       struct elf_link_hash_entry *h)
{
  bfd_vma got_address, got_value;
  struct mips_elf_link_hash_table *htab;

  htab = mips_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);

  BFD_ASSERT (h->plt.plist != NULL);
  BFD_ASSERT (h->plt.plist->gotplt_index != MINUS_ONE);

  /* Calculate the address of the associated .got.plt entry.  */
  got_address = (htab->root.sgotplt->output_section->vma
		 + htab->root.sgotplt->output_offset
		 + (h->plt.plist->gotplt_index
		    * MIPS_ELF_GOT_SIZE (info->output_bfd)));

  /* Calculate the value of _GLOBAL_OFFSET_TABLE_.  */
  got_value = (htab->root.hgot->root.u.def.section->output_section->vma
	       + htab->root.hgot->root.u.def.section->output_offset
	       + htab->root.hgot->root.u.def.value);

  return got_address - got_value;
}

/* Return the GOT offset for address VALUE.   If there is not yet a GOT
   entry for this value, create one.  If R_SYMNDX refers to a TLS symbol,
   create a TLS GOT entry instead.  Return -1 if no satisfactory GOT
   offset can be found.  */

static bfd_vma
mips_elf_local_got_index (bfd *abfd, bfd *ibfd, struct bfd_link_info *info,
			  bfd_vma value, unsigned long r_symndx,
			  struct mips_elf_link_hash_entry *h, int r_type)
{
  struct mips_elf_link_hash_table *htab;
  struct mips_got_entry *entry;

  htab = mips_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);

  entry = mips_elf_create_local_got_entry (abfd, info, ibfd, value,
					   r_symndx, h, r_type);
  if (!entry)
    return MINUS_ONE;

  if (entry->tls_type)
    mips_elf_initialize_tls_slots (abfd, info, entry, h, value);
  return entry->gotidx;
}

/* Return the GOT index of global symbol H in the primary GOT.  */

static bfd_vma
mips_elf_primary_global_got_index (bfd *obfd, struct bfd_link_info *info,
				   struct elf_link_hash_entry *h)
{
  struct mips_elf_link_hash_table *htab;
  long global_got_dynindx;
  struct mips_got_info *g;
  bfd_vma got_index;

  htab = mips_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);

  global_got_dynindx = 0;
  if (htab->global_gotsym != NULL)
    global_got_dynindx = htab->global_gotsym->dynindx;

  /* Once we determine the global GOT entry with the lowest dynamic
     symbol table index, we must put all dynamic symbols with greater
     indices into the primary GOT.  That makes it easy to calculate the
     GOT offset.  */
  BFD_ASSERT (h->dynindx >= global_got_dynindx);
  g = mips_elf_bfd_got (obfd, false);
  got_index = ((h->dynindx - global_got_dynindx + g->local_gotno)
	       * MIPS_ELF_GOT_SIZE (obfd));
  BFD_ASSERT (got_index < htab->root.sgot->size);

  return got_index;
}

/* Return the GOT index for the global symbol indicated by H, which is
   referenced by a relocation of type R_TYPE in IBFD.  */

static bfd_vma
mips_elf_global_got_index (bfd *obfd, struct bfd_link_info *info, bfd *ibfd,
			   struct elf_link_hash_entry *h, int r_type)
{
  struct mips_elf_link_hash_table *htab;
  struct mips_got_info *g;
  struct mips_got_entry lookup, *entry;
  bfd_vma gotidx;

  htab = mips_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);

  g = mips_elf_bfd_got (ibfd, false);
  BFD_ASSERT (g);

  lookup.tls_type = mips_elf_reloc_tls_type (r_type);
  if (!lookup.tls_type && g == mips_elf_bfd_got (obfd, false))
    return mips_elf_primary_global_got_index (obfd, info, h);

  lookup.abfd = ibfd;
  lookup.symndx = -1;
  lookup.d.h = (struct mips_elf_link_hash_entry *) h;
  entry = htab_find (g->got_entries, &lookup);
  BFD_ASSERT (entry);

  gotidx = entry->gotidx;
  BFD_ASSERT (gotidx > 0 && gotidx < htab->root.sgot->size);

  if (lookup.tls_type)
    {
      bfd_vma value = MINUS_ONE;

      if ((h->root.type == bfd_link_hash_defined
	   || h->root.type == bfd_link_hash_defweak)
	  && h->root.u.def.section->output_section)
	value = (h->root.u.def.value
		 + h->root.u.def.section->output_offset
		 + h->root.u.def.section->output_section->vma);

      mips_elf_initialize_tls_slots (obfd, info, entry, lookup.d.h, value);
    }
  return gotidx;
}

/* Find a GOT page entry that points to within 32KB of VALUE.  These
   entries are supposed to be placed at small offsets in the GOT, i.e.,
   within 32KB of GP.  Return the index of the GOT entry, or -1 if no
   entry could be created.  If OFFSETP is nonnull, use it to return the
   offset of the GOT entry from VALUE.  */

static bfd_vma
mips_elf_got_page (bfd *abfd, bfd *ibfd, struct bfd_link_info *info,
		   bfd_vma value, bfd_vma *offsetp)
{
  bfd_vma page, got_index;
  struct mips_got_entry *entry;

  page = (value + 0x8000) & ~(bfd_vma) 0xffff;
  entry = mips_elf_create_local_got_entry (abfd, info, ibfd, page, 0,
					   NULL, R_MIPS_GOT_PAGE);

  if (!entry)
    return MINUS_ONE;

  got_index = entry->gotidx;

  if (offsetp)
    *offsetp = value - entry->d.address;

  return got_index;
}

/* Find a local GOT entry for an R_MIPS*_GOT16 relocation against VALUE.
   EXTERNAL is true if the relocation was originally against a global
   symbol that binds locally.  */

static bfd_vma
mips_elf_got16_entry (bfd *abfd, bfd *ibfd, struct bfd_link_info *info,
		      bfd_vma value, bool external)
{
  struct mips_got_entry *entry;

  /* GOT16 relocations against local symbols are followed by a LO16
     relocation; those against global symbols are not.  Thus if the
     symbol was originally local, the GOT16 relocation should load the
     equivalent of %hi(VALUE), otherwise it should load VALUE itself.  */
  if (! external)
    value = mips_elf_high (value) << 16;

  /* It doesn't matter whether the original relocation was R_MIPS_GOT16,
     R_MIPS16_GOT16, R_MIPS_CALL16, etc.  The format of the entry is the
     same in all cases.  */
  entry = mips_elf_create_local_got_entry (abfd, info, ibfd, value, 0,
					   NULL, R_MIPS_GOT16);
  if (entry)
    return entry->gotidx;
  else
    return MINUS_ONE;
}

/* Returns the offset for the entry at the INDEXth position
   in the GOT.  */

static bfd_vma
mips_elf_got_offset_from_index (struct bfd_link_info *info, bfd *output_bfd,
				bfd *input_bfd, bfd_vma got_index)
{
  struct mips_elf_link_hash_table *htab;
  asection *sgot;
  bfd_vma gp;

  htab = mips_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);

  sgot = htab->root.sgot;
  gp = _bfd_get_gp_value (output_bfd)
    + mips_elf_adjust_gp (output_bfd, htab->got_info, input_bfd);

  return sgot->output_section->vma + sgot->output_offset + got_index - gp;
}

/* Create and return a local GOT entry for VALUE, which was calculated
   from a symbol belonging to INPUT_SECTON.  Return NULL if it could not
   be created.  If R_SYMNDX refers to a TLS symbol, create a TLS entry
   instead.  */

static struct mips_got_entry *
mips_elf_create_local_got_entry (bfd *abfd, struct bfd_link_info *info,
				 bfd *ibfd, bfd_vma value,
				 unsigned long r_symndx,
				 struct mips_elf_link_hash_entry *h,
				 int r_type)
{
  struct mips_got_entry lookup, *entry;
  void **loc;
  struct mips_got_info *g;
  struct mips_elf_link_hash_table *htab;
  bfd_vma gotidx;

  htab = mips_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);

  g = mips_elf_bfd_got (ibfd, false);
  if (g == NULL)
    {
      g = mips_elf_bfd_got (abfd, false);
      BFD_ASSERT (g != NULL);
    }

  /* This function shouldn't be called for symbols that live in the global
     area of the GOT.  */
  BFD_ASSERT (h == NULL || h->global_got_area == GGA_NONE);

  lookup.tls_type = mips_elf_reloc_tls_type (r_type);
  if (lookup.tls_type)
    {
      lookup.abfd = ibfd;
      if (tls_ldm_reloc_p (r_type))
	{
	  lookup.symndx = 0;
	  lookup.d.addend = 0;
	}
      else if (h == NULL)
	{
	  lookup.symndx = r_symndx;
	  lookup.d.addend = 0;
	}
      else
	{
	  lookup.symndx = -1;
	  lookup.d.h = h;
	}

      entry = (struct mips_got_entry *) htab_find (g->got_entries, &lookup);
      BFD_ASSERT (entry);

      gotidx = entry->gotidx;
      BFD_ASSERT (gotidx > 0 && gotidx < htab->root.sgot->size);

      return entry;
    }

  lookup.abfd = NULL;
  lookup.symndx = -1;
  lookup.d.address = value;
  loc = htab_find_slot (g->got_entries, &lookup, INSERT);
  if (!loc)
    return NULL;

  entry = (struct mips_got_entry *) *loc;
  if (entry)
    return entry;

  if (g->assigned_low_gotno > g->assigned_high_gotno)
    {
      /* We didn't allocate enough space in the GOT.  */
      _bfd_error_handler
	(_("not enough GOT space for local GOT entries"));
      bfd_set_error (bfd_error_bad_value);
      return NULL;
    }

  entry = (struct mips_got_entry *) bfd_alloc (abfd, sizeof (*entry));
  if (!entry)
    return NULL;

  if (got16_reloc_p (r_type)
      || call16_reloc_p (r_type)
      || got_page_reloc_p (r_type)
      || got_disp_reloc_p (r_type))
    lookup.gotidx = MIPS_ELF_GOT_SIZE (abfd) * g->assigned_low_gotno++;
  else
    lookup.gotidx = MIPS_ELF_GOT_SIZE (abfd) * g->assigned_high_gotno--;

  *entry = lookup;
  *loc = entry;

  MIPS_ELF_PUT_WORD (abfd, value, htab->root.sgot->contents + entry->gotidx);

  /* These GOT entries need a dynamic relocation on VxWorks.  */
  if (htab->root.target_os == is_vxworks)
    {
      Elf_Internal_Rela outrel;
      asection *s;
      bfd_byte *rloc;
      bfd_vma got_address;

      s = mips_elf_rel_dyn_section (info, false);
      got_address = (htab->root.sgot->output_section->vma
		     + htab->root.sgot->output_offset
		     + entry->gotidx);

      rloc = s->contents + (s->reloc_count++ * sizeof (Elf32_External_Rela));
      outrel.r_offset = got_address;
      outrel.r_info = ELF32_R_INFO (STN_UNDEF, R_MIPS_32);
      outrel.r_addend = value;
      bfd_elf32_swap_reloca_out (abfd, &outrel, rloc);
    }

  return entry;
}

/* Return the number of dynamic section symbols required by OUTPUT_BFD.
   The number might be exact or a worst-case estimate, depending on how
   much information is available to elf_backend_omit_section_dynsym at
   the current linking stage.  */

static bfd_size_type
count_section_dynsyms (bfd *output_bfd, struct bfd_link_info *info)
{
  bfd_size_type count;

  count = 0;
  if (bfd_link_pic (info)
      || elf_hash_table (info)->is_relocatable_executable)
    {
      asection *p;
      const struct elf_backend_data *bed;

      bed = get_elf_backend_data (output_bfd);
      for (p = output_bfd->sections; p ; p = p->next)
	if ((p->flags & SEC_EXCLUDE) == 0
	    && (p->flags & SEC_ALLOC) != 0
	    && elf_hash_table (info)->dynamic_relocs
	    && !(*bed->elf_backend_omit_section_dynsym) (output_bfd, info, p))
	  ++count;
    }
  return count;
}

/* Sort the dynamic symbol table so that symbols that need GOT entries
   appear towards the end.  */

static bool
mips_elf_sort_hash_table (bfd *abfd, struct bfd_link_info *info)
{
  struct mips_elf_link_hash_table *htab;
  struct mips_elf_hash_sort_data hsd;
  struct mips_got_info *g;

  htab = mips_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);

  if (htab->root.dynsymcount == 0)
    return true;

  g = htab->got_info;
  if (g == NULL)
    return true;

  hsd.low = NULL;
  hsd.max_unref_got_dynindx
    = hsd.min_got_dynindx
    = (htab->root.dynsymcount - g->reloc_only_gotno);
  /* Add 1 to local symbol indices to account for the mandatory NULL entry
     at the head of the table; see `_bfd_elf_link_renumber_dynsyms'.  */
  hsd.max_local_dynindx = count_section_dynsyms (abfd, info) + 1;
  hsd.max_non_got_dynindx = htab->root.local_dynsymcount + 1;
  hsd.output_bfd = abfd;
  if (htab->root.dynobj != NULL
      && htab->root.dynamic_sections_created
      && info->emit_gnu_hash)
    {
      asection *s = bfd_get_linker_section (htab->root.dynobj, ".MIPS.xhash");
      BFD_ASSERT (s != NULL);
      hsd.mipsxhash = s->contents;
      BFD_ASSERT (hsd.mipsxhash != NULL);
    }
  else
    hsd.mipsxhash = NULL;
  mips_elf_link_hash_traverse (htab, mips_elf_sort_hash_table_f, &hsd);

  /* There should have been enough room in the symbol table to
     accommodate both the GOT and non-GOT symbols.  */
  BFD_ASSERT (hsd.max_local_dynindx <= htab->root.local_dynsymcount + 1);
  BFD_ASSERT (hsd.max_non_got_dynindx <= hsd.min_got_dynindx);
  BFD_ASSERT (hsd.max_unref_got_dynindx == htab->root.dynsymcount);
  BFD_ASSERT (htab->root.dynsymcount - hsd.min_got_dynindx == g->global_gotno);

  /* Now we know which dynamic symbol has the lowest dynamic symbol
     table index in the GOT.  */
  htab->global_gotsym = hsd.low;

  return true;
}

/* If H needs a GOT entry, assign it the highest available dynamic
   index.  Otherwise, assign it the lowest available dynamic
   index.  */

static bool
mips_elf_sort_hash_table_f (struct mips_elf_link_hash_entry *h, void *data)
{
  struct mips_elf_hash_sort_data *hsd = data;

  /* Symbols without dynamic symbol table entries aren't interesting
     at all.  */
  if (h->root.dynindx == -1)
    return true;

  switch (h->global_got_area)
    {
    case GGA_NONE:
      if (h->root.forced_local)
	h->root.dynindx = hsd->max_local_dynindx++;
      else
	h->root.dynindx = hsd->max_non_got_dynindx++;
      break;

    case GGA_NORMAL:
      h->root.dynindx = --hsd->min_got_dynindx;
      hsd->low = (struct elf_link_hash_entry *) h;
      break;

    case GGA_RELOC_ONLY:
      if (hsd->max_unref_got_dynindx == hsd->min_got_dynindx)
	hsd->low = (struct elf_link_hash_entry *) h;
      h->root.dynindx = hsd->max_unref_got_dynindx++;
      break;
    }

  /* Populate the .MIPS.xhash translation table entry with
     the symbol dynindx.  */
  if (h->mipsxhash_loc != 0 && hsd->mipsxhash != NULL)
    bfd_put_32 (hsd->output_bfd, h->root.dynindx,
		hsd->mipsxhash + h->mipsxhash_loc);

  return true;
}

/* Record that input bfd ABFD requires a GOT entry like *LOOKUP
   (which is owned by the caller and shouldn't be added to the
   hash table directly).  */

static bool
mips_elf_record_got_entry (struct bfd_link_info *info, bfd *abfd,
			   struct mips_got_entry *lookup)
{
  struct mips_elf_link_hash_table *htab;
  struct mips_got_entry *entry;
  struct mips_got_info *g;
  void **loc, **bfd_loc;

  /* Make sure there's a slot for this entry in the master GOT.  */
  htab = mips_elf_hash_table (info);
  g = htab->got_info;
  loc = htab_find_slot (g->got_entries, lookup, INSERT);
  if (!loc)
    return false;

  /* Populate the entry if it isn't already.  */
  entry = (struct mips_got_entry *) *loc;
  if (!entry)
    {
      entry = (struct mips_got_entry *) bfd_alloc (abfd, sizeof (*entry));
      if (!entry)
	return false;

      lookup->tls_initialized = false;
      lookup->gotidx = -1;
      *entry = *lookup;
      *loc = entry;
    }

  /* Reuse the same GOT entry for the BFD's GOT.  */
  g = mips_elf_bfd_got (abfd, true);
  if (!g)
    return false;

  bfd_loc = htab_find_slot (g->got_entries, lookup, INSERT);
  if (!bfd_loc)
    return false;

  if (!*bfd_loc)
    *bfd_loc = entry;
  return true;
}

/* ABFD has a GOT relocation of type R_TYPE against H.  Reserve a GOT
   entry for it.  FOR_CALL is true if the caller is only interested in
   using the GOT entry for calls.  */

static bool
mips_elf_record_global_got_symbol (struct elf_link_hash_entry *h,
				   bfd *abfd, struct bfd_link_info *info,
				   bool for_call, int r_type)
{
  struct mips_elf_link_hash_table *htab;
  struct mips_elf_link_hash_entry *hmips;
  struct mips_got_entry entry;
  unsigned char tls_type;

  htab = mips_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);

  hmips = (struct mips_elf_link_hash_entry *) h;
  if (!for_call)
    hmips->got_only_for_calls = false;

  /* A global symbol in the GOT must also be in the dynamic symbol
     table.  */
  if (h->dynindx == -1)
    {
      switch (ELF_ST_VISIBILITY (h->other))
	{
	case STV_INTERNAL:
	case STV_HIDDEN:
	  _bfd_mips_elf_hide_symbol (info, h, true);
	  break;
	}
      if (!bfd_elf_link_record_dynamic_symbol (info, h))
	return false;
    }

  tls_type = mips_elf_reloc_tls_type (r_type);
  if (tls_type == GOT_TLS_NONE && hmips->global_got_area > GGA_NORMAL)
    hmips->global_got_area = GGA_NORMAL;

  entry.abfd = abfd;
  entry.symndx = -1;
  entry.d.h = (struct mips_elf_link_hash_entry *) h;
  entry.tls_type = tls_type;
  return mips_elf_record_got_entry (info, abfd, &entry);
}

/* ABFD has a GOT relocation of type R_TYPE against symbol SYMNDX + ADDEND,
   where SYMNDX is a local symbol.  Reserve a GOT entry for it.  */

static bool
mips_elf_record_local_got_symbol (bfd *abfd, long symndx, bfd_vma addend,
				  struct bfd_link_info *info, int r_type)
{
  struct mips_elf_link_hash_table *htab;
  struct mips_got_info *g;
  struct mips_got_entry entry;

  htab = mips_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);

  g = htab->got_info;
  BFD_ASSERT (g != NULL);

  entry.abfd = abfd;
  entry.symndx = symndx;
  entry.d.addend = addend;
  entry.tls_type = mips_elf_reloc_tls_type (r_type);
  return mips_elf_record_got_entry (info, abfd, &entry);
}

/* Record that ABFD has a page relocation against SYMNDX + ADDEND.
   H is the symbol's hash table entry, or null if SYMNDX is local
   to ABFD.  */

static bool
mips_elf_record_got_page_ref (struct bfd_link_info *info, bfd *abfd,
			      long symndx, struct elf_link_hash_entry *h,
			      bfd_signed_vma addend)
{
  struct mips_elf_link_hash_table *htab;
  struct mips_got_info *g1, *g2;
  struct mips_got_page_ref lookup, *entry;
  void **loc, **bfd_loc;

  htab = mips_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);

  g1 = htab->got_info;
  BFD_ASSERT (g1 != NULL);

  if (h)
    {
      lookup.symndx = -1;
      lookup.u.h = (struct mips_elf_link_hash_entry *) h;
    }
  else
    {
      lookup.symndx = symndx;
      lookup.u.abfd = abfd;
    }
  lookup.addend = addend;
  loc = htab_find_slot (g1->got_page_refs, &lookup, INSERT);
  if (loc == NULL)
    return false;

  entry = (struct mips_got_page_ref *) *loc;
  if (!entry)
    {
      entry = bfd_alloc (abfd, sizeof (*entry));
      if (!entry)
	return false;

      *entry = lookup;
      *loc = entry;
    }

  /* Add the same entry to the BFD's GOT.  */
  g2 = mips_elf_bfd_got (abfd, true);
  if (!g2)
    return false;

  bfd_loc = htab_find_slot (g2->got_page_refs, &lookup, INSERT);
  if (!bfd_loc)
    return false;

  if (!*bfd_loc)
    *bfd_loc = entry;

  return true;
}

/* Add room for N relocations to the .rel(a).dyn section in ABFD.  */

static void
mips_elf_allocate_dynamic_relocations (bfd *abfd, struct bfd_link_info *info,
				       unsigned int n)
{
  asection *s;
  struct mips_elf_link_hash_table *htab;

  htab = mips_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);

  s = mips_elf_rel_dyn_section (info, false);
  BFD_ASSERT (s != NULL);

  if (htab->root.target_os == is_vxworks)
    s->size += n * MIPS_ELF_RELA_SIZE (abfd);
  else
    {
      if (s->size == 0)
	{
	  /* Make room for a null element.  */
	  s->size += MIPS_ELF_REL_SIZE (abfd);
	  ++s->reloc_count;
	}
      s->size += n * MIPS_ELF_REL_SIZE (abfd);
    }
}

/* A htab_traverse callback for GOT entries, with DATA pointing to a
   mips_elf_traverse_got_arg structure.  Count the number of GOT
   entries and TLS relocs.  Set DATA->value to true if we need
   to resolve indirect or warning symbols and then recreate the GOT.  */

static int
mips_elf_check_recreate_got (void **entryp, void *data)
{
  struct mips_got_entry *entry;
  struct mips_elf_traverse_got_arg *arg;

  entry = (struct mips_got_entry *) *entryp;
  arg = (struct mips_elf_traverse_got_arg *) data;
  if (entry->abfd != NULL && entry->symndx == -1)
    {
      struct mips_elf_link_hash_entry *h;

      h = entry->d.h;
      if (h->root.root.type == bfd_link_hash_indirect
	  || h->root.root.type == bfd_link_hash_warning)
	{
	  arg->value = true;
	  return 0;
	}
    }
  mips_elf_count_got_entry (arg->info, arg->g, entry);
  return 1;
}

/* A htab_traverse callback for GOT entries, with DATA pointing to a
   mips_elf_traverse_got_arg structure.  Add all entries to DATA->g,
   converting entries for indirect and warning symbols into entries
   for the target symbol.  Set DATA->g to null on error.  */

static int
mips_elf_recreate_got (void **entryp, void *data)
{
  struct mips_got_entry new_entry, *entry;
  struct mips_elf_traverse_got_arg *arg;
  void **slot;

  entry = (struct mips_got_entry *) *entryp;
  arg = (struct mips_elf_traverse_got_arg *) data;
  if (entry->abfd != NULL
      && entry->symndx == -1
      && (entry->d.h->root.root.type == bfd_link_hash_indirect
	  || entry->d.h->root.root.type == bfd_link_hash_warning))
    {
      struct mips_elf_link_hash_entry *h;

      new_entry = *entry;
      entry = &new_entry;
      h = entry->d.h;
      do
	{
	  BFD_ASSERT (h->global_got_area == GGA_NONE);
	  h = (struct mips_elf_link_hash_entry *) h->root.root.u.i.link;
	}
      while (h->root.root.type == bfd_link_hash_indirect
	     || h->root.root.type == bfd_link_hash_warning);
      entry->d.h = h;
    }
  slot = htab_find_slot (arg->g->got_entries, entry, INSERT);
  if (slot == NULL)
    {
      arg->g = NULL;
      return 0;
    }
  if (*slot == NULL)
    {
      if (entry == &new_entry)
	{
	  entry = bfd_alloc (entry->abfd, sizeof (*entry));
	  if (!entry)
	    {
	      arg->g = NULL;
	      return 0;
	    }
	  *entry = new_entry;
	}
      *slot = entry;
      mips_elf_count_got_entry (arg->info, arg->g, entry);
    }
  return 1;
}

/* Return the maximum number of GOT page entries required for RANGE.  */

static bfd_vma
mips_elf_pages_for_range (const struct mips_got_page_range *range)
{
  return (range->max_addend - range->min_addend + 0x1ffff) >> 16;
}

/* Record that G requires a page entry that can reach SEC + ADDEND.  */

static bool
mips_elf_record_got_page_entry (struct mips_elf_traverse_got_arg *arg,
				asection *sec, bfd_signed_vma addend)
{
  struct mips_got_info *g = arg->g;
  struct mips_got_page_entry lookup, *entry;
  struct mips_got_page_range **range_ptr, *range;
  bfd_vma old_pages, new_pages;
  void **loc;

  /* Find the mips_got_page_entry hash table entry for this section.  */
  lookup.sec = sec;
  loc = htab_find_slot (g->got_page_entries, &lookup, INSERT);
  if (loc == NULL)
    return false;

  /* Create a mips_got_page_entry if this is the first time we've
     seen the section.  */
  entry = (struct mips_got_page_entry *) *loc;
  if (!entry)
    {
      entry = bfd_zalloc (arg->info->output_bfd, sizeof (*entry));
      if (!entry)
	return false;

      entry->sec = sec;
      *loc = entry;
    }

  /* Skip over ranges whose maximum extent cannot share a page entry
     with ADDEND.  */
  range_ptr = &entry->ranges;
  while (*range_ptr && addend > (*range_ptr)->max_addend + 0xffff)
    range_ptr = &(*range_ptr)->next;

  /* If we scanned to the end of the list, or found a range whose
     minimum extent cannot share a page entry with ADDEND, create
     a new singleton range.  */
  range = *range_ptr;
  if (!range || addend < range->min_addend - 0xffff)
    {
      range = bfd_zalloc (arg->info->output_bfd, sizeof (*range));
      if (!range)
	return false;

      range->next = *range_ptr;
      range->min_addend = addend;
      range->max_addend = addend;

      *range_ptr = range;
      entry->num_pages++;
      g->page_gotno++;
      return true;
    }

  /* Remember how many pages the old range contributed.  */
  old_pages = mips_elf_pages_for_range (range);

  /* Update the ranges.  */
  if (addend < range->min_addend)
    range->min_addend = addend;
  else if (addend > range->max_addend)
    {
      if (range->next && addend >= range->next->min_addend - 0xffff)
	{
	  old_pages += mips_elf_pages_for_range (range->next);
	  range->max_addend = range->next->max_addend;
	  range->next = range->next->next;
	}
      else
	range->max_addend = addend;
    }

  /* Record any change in the total estimate.  */
  new_pages = mips_elf_pages_for_range (range);
  if (old_pages != new_pages)
    {
      entry->num_pages += new_pages - old_pages;
      g->page_gotno += new_pages - old_pages;
    }

  return true;
}

/* A htab_traverse callback for which *REFP points to a mips_got_page_ref
   and for which DATA points to a mips_elf_traverse_got_arg.  Work out
   whether the page reference described by *REFP needs a GOT page entry,
   and record that entry in DATA->g if so.  Set DATA->g to null on failure.  */

static int
mips_elf_resolve_got_page_ref (void **refp, void *data)
{
  struct mips_got_page_ref *ref;
  struct mips_elf_traverse_got_arg *arg;
  struct mips_elf_link_hash_table *htab;
  asection *sec;
  bfd_vma addend;

  ref = (struct mips_got_page_ref *) *refp;
  arg = (struct mips_elf_traverse_got_arg *) data;
  htab = mips_elf_hash_table (arg->info);

  if (ref->symndx < 0)
    {
      struct mips_elf_link_hash_entry *h;

      /* Global GOT_PAGEs decay to GOT_DISP and so don't need page entries.  */
      h = ref->u.h;
      if (!SYMBOL_REFERENCES_LOCAL (arg->info, &h->root))
	return 1;

      /* Ignore undefined symbols; we'll issue an error later if
	 appropriate.  */
      if (!((h->root.root.type == bfd_link_hash_defined
	     || h->root.root.type == bfd_link_hash_defweak)
	    && h->root.root.u.def.section))
	return 1;

      sec = h->root.root.u.def.section;
      addend = h->root.root.u.def.value + ref->addend;
    }
  else
    {
      Elf_Internal_Sym *isym;

      /* Read in the symbol.  */
      isym = bfd_sym_from_r_symndx (&htab->root.sym_cache, ref->u.abfd,
				    ref->symndx);
      if (isym == NULL)
	{
	  arg->g = NULL;
	  return 0;
	}

      /* Get the associated input section.  */
      sec = bfd_section_from_elf_index (ref->u.abfd, isym->st_shndx);
      if (sec == NULL)
	{
	  arg->g = NULL;
	  return 0;
	}

      /* If this is a mergable section, work out the section and offset
	 of the merged data.  For section symbols, the addend specifies
	 of the offset _of_ the first byte in the data, otherwise it
	 specifies the offset _from_ the first byte.  */
      if (sec->flags & SEC_MERGE)
	{
	  void *secinfo;

	  secinfo = elf_section_data (sec)->sec_info;
	  if (ELF_ST_TYPE (isym->st_info) == STT_SECTION)
	    addend = _bfd_merged_section_offset (ref->u.abfd, &sec, secinfo,
						 isym->st_value + ref->addend);
	  else
	    addend = _bfd_merged_section_offset (ref->u.abfd, &sec, secinfo,
						 isym->st_value) + ref->addend;
	}
      else
	addend = isym->st_value + ref->addend;
    }
  if (!mips_elf_record_got_page_entry (arg, sec, addend))
    {
      arg->g = NULL;
      return 0;
    }
  return 1;
}

/* If any entries in G->got_entries are for indirect or warning symbols,
   replace them with entries for the target symbol.  Convert g->got_page_refs
   into got_page_entry structures and estimate the number of page entries
   that they require.  */

static bool
mips_elf_resolve_final_got_entries (struct bfd_link_info *info,
				    struct mips_got_info *g)
{
  struct mips_elf_traverse_got_arg tga;
  struct mips_got_info oldg;

  oldg = *g;

  tga.info = info;
  tga.g = g;
  tga.value = false;
  htab_traverse (g->got_entries, mips_elf_check_recreate_got, &tga);
  if (tga.value)
    {
      *g = oldg;
      g->got_entries = htab_create (htab_size (oldg.got_entries),
				    mips_elf_got_entry_hash,
				    mips_elf_got_entry_eq, NULL);
      if (!g->got_entries)
	return false;

      htab_traverse (oldg.got_entries, mips_elf_recreate_got, &tga);
      if (!tga.g)
	return false;

      htab_delete (oldg.got_entries);
    }

  g->got_page_entries = htab_try_create (1, mips_got_page_entry_hash,
					 mips_got_page_entry_eq, NULL);
  if (g->got_page_entries == NULL)
    return false;

  tga.info = info;
  tga.g = g;
  htab_traverse (g->got_page_refs, mips_elf_resolve_got_page_ref, &tga);

  return true;
}

/* Return true if a GOT entry for H should live in the local rather than
   global GOT area.  */

static bool
mips_use_local_got_p (struct bfd_link_info *info,
		      struct mips_elf_link_hash_entry *h)
{
  /* Symbols that aren't in the dynamic symbol table must live in the
     local GOT.  This includes symbols that are completely undefined
     and which therefore don't bind locally.  We'll report undefined
     symbols later if appropriate.  */
  if (h->root.dynindx == -1)
    return true;

  /* Absolute symbols, if ever they need a GOT entry, cannot ever go
     to the local GOT, as they would be implicitly relocated by the
     base address by the dynamic loader.  */
  if (bfd_is_abs_symbol (&h->root.root))
    return false;

  /* Symbols that bind locally can (and in the case of forced-local
     symbols, must) live in the local GOT.  */
  if (h->got_only_for_calls
      ? SYMBOL_CALLS_LOCAL (info, &h->root)
      : SYMBOL_REFERENCES_LOCAL (info, &h->root))
    return true;

  /* If this is an executable that must provide a definition of the symbol,
     either though PLTs or copy relocations, then that address should go in
     the local rather than global GOT.  */
  if (bfd_link_executable (info) && h->has_static_relocs)
    return true;

  return false;
}

/* A mips_elf_link_hash_traverse callback for which DATA points to the
   link_info structure.  Decide whether the hash entry needs an entry in
   the global part of the primary GOT, setting global_got_area accordingly.
   Count the number of global symbols that are in the primary GOT only
   because they have relocations against them (reloc_only_gotno).  */

static bool
mips_elf_count_got_symbols (struct mips_elf_link_hash_entry *h, void *data)
{
  struct bfd_link_info *info;
  struct mips_elf_link_hash_table *htab;
  struct mips_got_info *g;

  info = (struct bfd_link_info *) data;
  htab = mips_elf_hash_table (info);
  g = htab->got_info;
  if (h->global_got_area != GGA_NONE)
    {
      /* Make a final decision about whether the symbol belongs in the
	 local or global GOT.  */
      if (mips_use_local_got_p (info, h))
	/* The symbol belongs in the local GOT.  We no longer need this
	   entry if it was only used for relocations; those relocations
	   will be against the null or section symbol instead of H.  */
	h->global_got_area = GGA_NONE;
      else if (htab->root.target_os == is_vxworks
	       && h->got_only_for_calls
	       && h->root.plt.plist->mips_offset != MINUS_ONE)
	/* On VxWorks, calls can refer directly to the .got.plt entry;
	   they don't need entries in the regular GOT.  .got.plt entries
	   will be allocated by _bfd_mips_elf_adjust_dynamic_symbol.  */
	h->global_got_area = GGA_NONE;
      else if (h->global_got_area == GGA_RELOC_ONLY)
	{
	  g->reloc_only_gotno++;
	  g->global_gotno++;
	}
    }
  return 1;
}

/* A htab_traverse callback for GOT entries.  Add each one to the GOT
   given in mips_elf_traverse_got_arg DATA.  Clear DATA->G on error.  */

static int
mips_elf_add_got_entry (void **entryp, void *data)
{
  struct mips_got_entry *entry;
  struct mips_elf_traverse_got_arg *arg;
  void **slot;

  entry = (struct mips_got_entry *) *entryp;
  arg = (struct mips_elf_traverse_got_arg *) data;
  slot = htab_find_slot (arg->g->got_entries, entry, INSERT);
  if (!slot)
    {
      arg->g = NULL;
      return 0;
    }
  if (!*slot)
    {
      *slot = entry;
      mips_elf_count_got_entry (arg->info, arg->g, entry);
    }
  return 1;
}

/* A htab_traverse callback for GOT page entries.  Add each one to the GOT
   given in mips_elf_traverse_got_arg DATA.  Clear DATA->G on error.  */

static int
mips_elf_add_got_page_entry (void **entryp, void *data)
{
  struct mips_got_page_entry *entry;
  struct mips_elf_traverse_got_arg *arg;
  void **slot;

  entry = (struct mips_got_page_entry *) *entryp;
  arg = (struct mips_elf_traverse_got_arg *) data;
  slot = htab_find_slot (arg->g->got_page_entries, entry, INSERT);
  if (!slot)
    {
      arg->g = NULL;
      return 0;
    }
  if (!*slot)
    {
      *slot = entry;
      arg->g->page_gotno += entry->num_pages;
    }
  return 1;
}

/* Consider merging FROM, which is ABFD's GOT, into TO.  Return -1 if
   this would lead to overflow, 1 if they were merged successfully,
   and 0 if a merge failed due to lack of memory.  (These values are chosen
   so that nonnegative return values can be returned by a htab_traverse
   callback.)  */

static int
mips_elf_merge_got_with (bfd *abfd, struct mips_got_info *from,
			 struct mips_got_info *to,
			 struct mips_elf_got_per_bfd_arg *arg)
{
  struct mips_elf_traverse_got_arg tga;
  unsigned int estimate;

  /* Work out how many page entries we would need for the combined GOT.  */
  estimate = arg->max_pages;
  if (estimate >= from->page_gotno + to->page_gotno)
    estimate = from->page_gotno + to->page_gotno;

  /* And conservatively estimate how many local and TLS entries
     would be needed.  */
  estimate += from->local_gotno + to->local_gotno;
  estimate += from->tls_gotno + to->tls_gotno;

  /* If we're merging with the primary got, any TLS relocations will
     come after the full set of global entries.  Otherwise estimate those
     conservatively as well.  */
  if (to == arg->primary && from->tls_gotno + to->tls_gotno)
    estimate += arg->global_count;
  else
    estimate += from->global_gotno + to->global_gotno;

  /* Bail out if the combined GOT might be too big.  */
  if (estimate > arg->max_count)
    return -1;

  /* Transfer the bfd's got information from FROM to TO.  */
  tga.info = arg->info;
  tga.g = to;
  htab_traverse (from->got_entries, mips_elf_add_got_entry, &tga);
  if (!tga.g)
    return 0;

  htab_traverse (from->got_page_entries, mips_elf_add_got_page_entry, &tga);
  if (!tga.g)
    return 0;

  mips_elf_replace_bfd_got (abfd, to);
  return 1;
}

/* Attempt to merge GOT G, which belongs to ABFD.  Try to use as much
   as possible of the primary got, since it doesn't require explicit
   dynamic relocations, but don't use bfds that would reference global
   symbols out of the addressable range.  Failing the primary got,
   attempt to merge with the current got, or finish the current got
   and then make make the new got current.  */

static bool
mips_elf_merge_got (bfd *abfd, struct mips_got_info *g,
		    struct mips_elf_got_per_bfd_arg *arg)
{
  unsigned int estimate;
  int result;

  if (!mips_elf_resolve_final_got_entries (arg->info, g))
    return false;

  /* Work out the number of page, local and TLS entries.  */
  estimate = arg->max_pages;
  if (estimate > g->page_gotno)
    estimate = g->page_gotno;
  estimate += g->local_gotno + g->tls_gotno;

  /* We place TLS GOT entries after both locals and globals.  The globals
     for the primary GOT may overflow the normal GOT size limit, so be
     sure not to merge a GOT which requires TLS with the primary GOT in that
     case.  This doesn't affect non-primary GOTs.  */
  estimate += (g->tls_gotno > 0 ? arg->global_count : g->global_gotno);

  if (estimate <= arg->max_count)
    {
      /* If we don't have a primary GOT, use it as
	 a starting point for the primary GOT.  */
      if (!arg->primary)
	{
	  arg->primary = g;
	  return true;
	}

      /* Try merging with the primary GOT.  */
      result = mips_elf_merge_got_with (abfd, g, arg->primary, arg);
      if (result >= 0)
	return result;
    }

  /* If we can merge with the last-created got, do it.  */
  if (arg->current)
    {
      result = mips_elf_merge_got_with (abfd, g, arg->current, arg);
      if (result >= 0)
	return result;
    }

  /* Well, we couldn't merge, so create a new GOT.  Don't check if it
     fits; if it turns out that it doesn't, we'll get relocation
     overflows anyway.  */
  g->next = arg->current;
  arg->current = g;

  return true;
}

/* ENTRYP is a hash table entry for a mips_got_entry.  Set its gotidx
   to GOTIDX, duplicating the entry if it has already been assigned
   an index in a different GOT.  */

static bool
mips_elf_set_gotidx (void **entryp, long gotidx)
{
  struct mips_got_entry *entry;

  entry = (struct mips_got_entry *) *entryp;
  if (entry->gotidx > 0)
    {
      struct mips_got_entry *new_entry;

      new_entry = bfd_alloc (entry->abfd, sizeof (*entry));
      if (!new_entry)
	return false;

      *new_entry = *entry;
      *entryp = new_entry;
      entry = new_entry;
    }
  entry->gotidx = gotidx;
  return true;
}

/* Set the TLS GOT index for the GOT entry in ENTRYP.  DATA points to a
   mips_elf_traverse_got_arg in which DATA->value is the size of one
   GOT entry.  Set DATA->g to null on failure.  */

static int
mips_elf_initialize_tls_index (void **entryp, void *data)
{
  struct mips_got_entry *entry;
  struct mips_elf_traverse_got_arg *arg;

  /* We're only interested in TLS symbols.  */
  entry = (struct mips_got_entry *) *entryp;
  if (entry->tls_type == GOT_TLS_NONE)
    return 1;

  arg = (struct mips_elf_traverse_got_arg *) data;
  if (!mips_elf_set_gotidx (entryp, arg->value * arg->g->tls_assigned_gotno))
    {
      arg->g = NULL;
      return 0;
    }

  /* Account for the entries we've just allocated.  */
  arg->g->tls_assigned_gotno += mips_tls_got_entries (entry->tls_type);
  return 1;
}

/* A htab_traverse callback for GOT entries, where DATA points to a
   mips_elf_traverse_got_arg.  Set the global_got_area of each global
   symbol to DATA->value.  */

static int
mips_elf_set_global_got_area (void **entryp, void *data)
{
  struct mips_got_entry *entry;
  struct mips_elf_traverse_got_arg *arg;

  entry = (struct mips_got_entry *) *entryp;
  arg = (struct mips_elf_traverse_got_arg *) data;
  if (entry->abfd != NULL
      && entry->symndx == -1
      && entry->d.h->global_got_area != GGA_NONE)
    entry->d.h->global_got_area = arg->value;
  return 1;
}

/* A htab_traverse callback for secondary GOT entries, where DATA points
   to a mips_elf_traverse_got_arg.  Assign GOT indices to global entries
   and record the number of relocations they require.  DATA->value is
   the size of one GOT entry.  Set DATA->g to null on failure.  */

static int
mips_elf_set_global_gotidx (void **entryp, void *data)
{
  struct mips_got_entry *entry;
  struct mips_elf_traverse_got_arg *arg;

  entry = (struct mips_got_entry *) *entryp;
  arg = (struct mips_elf_traverse_got_arg *) data;
  if (entry->abfd != NULL
      && entry->symndx == -1
      && entry->d.h->global_got_area != GGA_NONE)
    {
      if (!mips_elf_set_gotidx (entryp, arg->value * arg->g->assigned_low_gotno))
	{
	  arg->g = NULL;
	  return 0;
	}
      arg->g->assigned_low_gotno += 1;

      if (bfd_link_pic (arg->info)
	  || (elf_hash_table (arg->info)->dynamic_sections_created
	      && entry->d.h->root.def_dynamic
	      && !entry->d.h->root.def_regular))
	arg->g->relocs += 1;
    }

  return 1;
}

/* A htab_traverse callback for GOT entries for which DATA is the
   bfd_link_info.  Forbid any global symbols from having traditional
   lazy-binding stubs.  */

static int
mips_elf_forbid_lazy_stubs (void **entryp, void *data)
{
  struct bfd_link_info *info;
  struct mips_elf_link_hash_table *htab;
  struct mips_got_entry *entry;

  entry = (struct mips_got_entry *) *entryp;
  info = (struct bfd_link_info *) data;
  htab = mips_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);

  if (entry->abfd != NULL
      && entry->symndx == -1
      && entry->d.h->needs_lazy_stub)
    {
      entry->d.h->needs_lazy_stub = false;
      htab->lazy_stub_count--;
    }

  return 1;
}

/* Return the offset of an input bfd IBFD's GOT from the beginning of
   the primary GOT.  */
static bfd_vma
mips_elf_adjust_gp (bfd *abfd, struct mips_got_info *g, bfd *ibfd)
{
  if (!g->next)
    return 0;

  g = mips_elf_bfd_got (ibfd, false);
  if (! g)
    return 0;

  BFD_ASSERT (g->next);

  g = g->next;

  return (g->local_gotno + g->global_gotno + g->tls_gotno)
    * MIPS_ELF_GOT_SIZE (abfd);
}

/* Turn a single GOT that is too big for 16-bit addressing into
   a sequence of GOTs, each one 16-bit addressable.  */

static bool
mips_elf_multi_got (bfd *abfd, struct bfd_link_info *info,
		    asection *got, bfd_size_type pages)
{
  struct mips_elf_link_hash_table *htab;
  struct mips_elf_got_per_bfd_arg got_per_bfd_arg;
  struct mips_elf_traverse_got_arg tga;
  struct mips_got_info *g, *gg;
  unsigned int assign, needed_relocs;
  bfd *dynobj, *ibfd;

  dynobj = elf_hash_table (info)->dynobj;
  htab = mips_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);

  g = htab->got_info;

  got_per_bfd_arg.obfd = abfd;
  got_per_bfd_arg.info = info;
  got_per_bfd_arg.current = NULL;
  got_per_bfd_arg.primary = NULL;
  got_per_bfd_arg.max_count = ((MIPS_ELF_GOT_MAX_SIZE (info)
				/ MIPS_ELF_GOT_SIZE (abfd))
			       - htab->reserved_gotno);
  got_per_bfd_arg.max_pages = pages;
  /* The number of globals that will be included in the primary GOT.
     See the calls to mips_elf_set_global_got_area below for more
     information.  */
  got_per_bfd_arg.global_count = g->global_gotno;

  /* Try to merge the GOTs of input bfds together, as long as they
     don't seem to exceed the maximum GOT size, choosing one of them
     to be the primary GOT.  */
  for (ibfd = info->input_bfds; ibfd; ibfd = ibfd->link.next)
    {
      gg = mips_elf_bfd_got (ibfd, false);
      if (gg && !mips_elf_merge_got (ibfd, gg, &got_per_bfd_arg))
	return false;
    }

  /* If we do not find any suitable primary GOT, create an empty one.  */
  if (got_per_bfd_arg.primary == NULL)
    g->next = mips_elf_create_got_info (abfd);
  else
    g->next = got_per_bfd_arg.primary;
  g->next->next = got_per_bfd_arg.current;

  /* GG is now the master GOT, and G is the primary GOT.  */
  gg = g;
  g = g->next;

  /* Map the output bfd to the primary got.  That's what we're going
     to use for bfds that use GOT16 or GOT_PAGE relocations that we
     didn't mark in check_relocs, and we want a quick way to find it.
     We can't just use gg->next because we're going to reverse the
     list.  */
  mips_elf_replace_bfd_got (abfd, g);

  /* Every symbol that is referenced in a dynamic relocation must be
     present in the primary GOT, so arrange for them to appear after
     those that are actually referenced.  */
  gg->reloc_only_gotno = gg->global_gotno - g->global_gotno;
  g->global_gotno = gg->global_gotno;

  tga.info = info;
  tga.value = GGA_RELOC_ONLY;
  htab_traverse (gg->got_entries, mips_elf_set_global_got_area, &tga);
  tga.value = GGA_NORMAL;
  htab_traverse (g->got_entries, mips_elf_set_global_got_area, &tga);

  /* Now go through the GOTs assigning them offset ranges.
     [assigned_low_gotno, local_gotno[ will be set to the range of local
     entries in each GOT.  We can then compute the end of a GOT by
     adding local_gotno to global_gotno.  We reverse the list and make
     it circular since then we'll be able to quickly compute the
     beginning of a GOT, by computing the end of its predecessor.  To
     avoid special cases for the primary GOT, while still preserving
     assertions that are valid for both single- and multi-got links,
     we arrange for the main got struct to have the right number of
     global entries, but set its local_gotno such that the initial
     offset of the primary GOT is zero.  Remember that the primary GOT
     will become the last item in the circular linked list, so it
     points back to the master GOT.  */
  gg->local_gotno = -g->global_gotno;
  gg->global_gotno = g->global_gotno;
  gg->tls_gotno = 0;
  assign = 0;
  gg->next = gg;

  do
    {
      struct mips_got_info *gn;

      assign += htab->reserved_gotno;
      g->assigned_low_gotno = assign;
      g->local_gotno += assign;
      g->local_gotno += (pages < g->page_gotno ? pages : g->page_gotno);
      g->assigned_high_gotno = g->local_gotno - 1;
      assign = g->local_gotno + g->global_gotno + g->tls_gotno;

      /* Take g out of the direct list, and push it onto the reversed
	 list that gg points to.  g->next is guaranteed to be nonnull after
	 this operation, as required by mips_elf_initialize_tls_index. */
      gn = g->next;
      g->next = gg->next;
      gg->next = g;

      /* Set up any TLS entries.  We always place the TLS entries after
	 all non-TLS entries.  */
      g->tls_assigned_gotno = g->local_gotno + g->global_gotno;
      tga.g = g;
      tga.value = MIPS_ELF_GOT_SIZE (abfd);
      htab_traverse (g->got_entries, mips_elf_initialize_tls_index, &tga);
      if (!tga.g)
	return false;
      BFD_ASSERT (g->tls_assigned_gotno == assign);

      /* Move onto the next GOT.  It will be a secondary GOT if nonull.  */
      g = gn;

      /* Forbid global symbols in every non-primary GOT from having
	 lazy-binding stubs.  */
      if (g)
	htab_traverse (g->got_entries, mips_elf_forbid_lazy_stubs, info);
    }
  while (g);

  got->size = assign * MIPS_ELF_GOT_SIZE (abfd);

  needed_relocs = 0;
  for (g = gg->next; g && g->next != gg; g = g->next)
    {
      unsigned int save_assign;

      /* Assign offsets to global GOT entries and count how many
	 relocations they need.  */
      save_assign = g->assigned_low_gotno;
      g->assigned_low_gotno = g->local_gotno;
      tga.info = info;
      tga.value = MIPS_ELF_GOT_SIZE (abfd);
      tga.g = g;
      htab_traverse (g->got_entries, mips_elf_set_global_gotidx, &tga);
      if (!tga.g)
	return false;
      BFD_ASSERT (g->assigned_low_gotno == g->local_gotno + g->global_gotno);
      g->assigned_low_gotno = save_assign;

      if (bfd_link_pic (info))
	{
	  g->relocs += g->local_gotno - g->assigned_low_gotno;
	  BFD_ASSERT (g->assigned_low_gotno == g->next->local_gotno
		      + g->next->global_gotno
		      + g->next->tls_gotno
		      + htab->reserved_gotno);
	}
      needed_relocs += g->relocs;
    }
  needed_relocs += g->relocs;

  if (needed_relocs)
    mips_elf_allocate_dynamic_relocations (dynobj, info,
					   needed_relocs);

  return true;
}


/* Returns the first relocation of type r_type found, beginning with
   RELOCATION.  RELEND is one-past-the-end of the relocation table.  */

static const Elf_Internal_Rela *
mips_elf_next_relocation (bfd *abfd ATTRIBUTE_UNUSED, unsigned int r_type,
			  const Elf_Internal_Rela *relocation,
			  const Elf_Internal_Rela *relend)
{
  unsigned long r_symndx = ELF_R_SYM (abfd, relocation->r_info);

  while (relocation < relend)
    {
      if (ELF_R_TYPE (abfd, relocation->r_info) == r_type
	  && ELF_R_SYM (abfd, relocation->r_info) == r_symndx)
	return relocation;

      ++relocation;
    }

  /* We didn't find it.  */
  return NULL;
}

/* Return whether an input relocation is against a local symbol.  */

static bool
mips_elf_local_relocation_p (bfd *input_bfd,
			     const Elf_Internal_Rela *relocation,
			     asection **local_sections)
{
  unsigned long r_symndx;
  Elf_Internal_Shdr *symtab_hdr;
  size_t extsymoff;

  r_symndx = ELF_R_SYM (input_bfd, relocation->r_info);
  symtab_hdr = &elf_tdata (input_bfd)->symtab_hdr;
  extsymoff = (elf_bad_symtab (input_bfd)) ? 0 : symtab_hdr->sh_info;

  if (r_symndx < extsymoff)
    return true;
  if (elf_bad_symtab (input_bfd) && local_sections[r_symndx] != NULL)
    return true;

  return false;
}

/* Sign-extend VALUE, which has the indicated number of BITS.  */

bfd_vma
_bfd_mips_elf_sign_extend (bfd_vma value, int bits)
{
  if (value & ((bfd_vma) 1 << (bits - 1)))
    /* VALUE is negative.  */
    value |= ((bfd_vma) - 1) << bits;

  return value;
}

/* Return non-zero if the indicated VALUE has overflowed the maximum
   range expressible by a signed number with the indicated number of
   BITS.  */

static bool
mips_elf_overflow_p (bfd_vma value, int bits)
{
  bfd_signed_vma svalue = (bfd_signed_vma) value;

  if (svalue > (1 << (bits - 1)) - 1)
    /* The value is too big.  */
    return true;
  else if (svalue < -(1 << (bits - 1)))
    /* The value is too small.  */
    return true;

  /* All is well.  */
  return false;
}

/* Calculate the %high function.  */

static bfd_vma
mips_elf_high (bfd_vma value)
{
  return ((value + (bfd_vma) 0x8000) >> 16) & 0xffff;
}

/* Calculate the %higher function.  */

static bfd_vma
mips_elf_higher (bfd_vma value ATTRIBUTE_UNUSED)
{
#ifdef BFD64
  return ((value + (bfd_vma) 0x80008000) >> 32) & 0xffff;
#else
  abort ();
  return MINUS_ONE;
#endif
}

/* Calculate the %highest function.  */

static bfd_vma
mips_elf_highest (bfd_vma value ATTRIBUTE_UNUSED)
{
#ifdef BFD64
  return ((value + (((bfd_vma) 0x8000 << 32) | 0x80008000)) >> 48) & 0xffff;
#else
  abort ();
  return MINUS_ONE;
#endif
}

/* Create the .compact_rel section.  */

static bool
mips_elf_create_compact_rel_section
  (bfd *abfd, struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  flagword flags;
  register asection *s;

  if (bfd_get_linker_section (abfd, ".compact_rel") == NULL)
    {
      flags = (SEC_HAS_CONTENTS | SEC_IN_MEMORY | SEC_LINKER_CREATED
	       | SEC_READONLY);

      s = bfd_make_section_anyway_with_flags (abfd, ".compact_rel", flags);
      if (s == NULL
	  || !bfd_set_section_alignment (s, MIPS_ELF_LOG_FILE_ALIGN (abfd)))
	return false;

      s->size = sizeof (Elf32_External_compact_rel);
    }

  return true;
}

/* Create the .got section to hold the global offset table.  */

static bool
mips_elf_create_got_section (bfd *abfd, struct bfd_link_info *info)
{
  flagword flags;
  register asection *s;
  struct elf_link_hash_entry *h;
  struct bfd_link_hash_entry *bh;
  struct mips_elf_link_hash_table *htab;

  htab = mips_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);

  /* This function may be called more than once.  */
  if (htab->root.sgot)
    return true;

  flags = (SEC_ALLOC | SEC_LOAD | SEC_HAS_CONTENTS | SEC_IN_MEMORY
	   | SEC_LINKER_CREATED);

  /* We have to use an alignment of 2**4 here because this is hardcoded
     in the function stub generation and in the linker script.  */
  s = bfd_make_section_anyway_with_flags (abfd, ".got", flags);
  if (s == NULL
      || !bfd_set_section_alignment (s, 4))
    return false;
  htab->root.sgot = s;

  /* Define the symbol _GLOBAL_OFFSET_TABLE_.  We don't do this in the
     linker script because we don't want to define the symbol if we
     are not creating a global offset table.  */
  bh = NULL;
  if (! (_bfd_generic_link_add_one_symbol
	 (info, abfd, "_GLOBAL_OFFSET_TABLE_", BSF_GLOBAL, s,
	  0, NULL, false, get_elf_backend_data (abfd)->collect, &bh)))
    return false;

  h = (struct elf_link_hash_entry *) bh;
  h->non_elf = 0;
  h->def_regular = 1;
  h->type = STT_OBJECT;
  h->other = (h->other & ~ELF_ST_VISIBILITY (-1)) | STV_HIDDEN;
  elf_hash_table (info)->hgot = h;

  if (bfd_link_pic (info)
      && ! bfd_elf_link_record_dynamic_symbol (info, h))
    return false;

  htab->got_info = mips_elf_create_got_info (abfd);
  mips_elf_section_data (s)->elf.this_hdr.sh_flags
    |= SHF_ALLOC | SHF_WRITE | SHF_MIPS_GPREL;

  /* We also need a .got.plt section when generating PLTs.  */
  s = bfd_make_section_anyway_with_flags (abfd, ".got.plt",
					  SEC_ALLOC | SEC_LOAD
					  | SEC_HAS_CONTENTS
					  | SEC_IN_MEMORY
					  | SEC_LINKER_CREATED);
  if (s == NULL)
    return false;
  htab->root.sgotplt = s;

  return true;
}

/* Return true if H refers to the special VxWorks __GOTT_BASE__ or
   __GOTT_INDEX__ symbols.  These symbols are only special for
   shared objects; they are not used in executables.  */

static bool
is_gott_symbol (struct bfd_link_info *info, struct elf_link_hash_entry *h)
{
  return (mips_elf_hash_table (info)->root.target_os == is_vxworks
	  && bfd_link_pic (info)
	  && (strcmp (h->root.root.string, "__GOTT_BASE__") == 0
	      || strcmp (h->root.root.string, "__GOTT_INDEX__") == 0));
}

/* Return TRUE if a relocation of type R_TYPE from INPUT_BFD might
   require an la25 stub.  See also mips_elf_local_pic_function_p,
   which determines whether the destination function ever requires a
   stub.  */

static bool
mips_elf_relocation_needs_la25_stub (bfd *input_bfd, int r_type,
				     bool target_is_16_bit_code_p)
{
  /* We specifically ignore branches and jumps from EF_PIC objects,
     where the onus is on the compiler or programmer to perform any
     necessary initialization of $25.  Sometimes such initialization
     is unnecessary; for example, -mno-shared functions do not use
     the incoming value of $25, and may therefore be called directly.  */
  if (PIC_OBJECT_P (input_bfd))
    return false;

  switch (r_type)
    {
    case R_MIPS_26:
    case R_MIPS_PC16:
    case R_MIPS_PC21_S2:
    case R_MIPS_PC26_S2:
    case R_MICROMIPS_26_S1:
    case R_MICROMIPS_PC7_S1:
    case R_MICROMIPS_PC10_S1:
    case R_MICROMIPS_PC16_S1:
    case R_MICROMIPS_PC23_S2:
      return true;

    case R_MIPS16_26:
      return !target_is_16_bit_code_p;

    default:
      return false;
    }
}

/* Obtain the field relocated by RELOCATION.  */

static bfd_vma
mips_elf_obtain_contents (reloc_howto_type *howto,
			  const Elf_Internal_Rela *relocation,
			  bfd *input_bfd, bfd_byte *contents)
{
  bfd_vma x = 0;
  bfd_byte *location = contents + relocation->r_offset;
  unsigned int size = bfd_get_reloc_size (howto);

  /* Obtain the bytes.  */
  if (size != 0)
    x = bfd_get (8 * size, input_bfd, location);

  return x;
}

/* Store the field relocated by RELOCATION.  */

static void
mips_elf_store_contents (reloc_howto_type *howto,
			 const Elf_Internal_Rela *relocation,
			 bfd *input_bfd, bfd_byte *contents, bfd_vma x)
{
  bfd_byte *location = contents + relocation->r_offset;
  unsigned int size = bfd_get_reloc_size (howto);

  /* Put the value into the output.  */
  if (size != 0)
    bfd_put (8 * size, input_bfd, x, location);
}

/* Try to patch a load from GOT instruction in CONTENTS pointed to by
   RELOCATION described by HOWTO, with a move of 0 to the load target
   register, returning TRUE if that is successful and FALSE otherwise.
   If DOIT is FALSE, then only determine it patching is possible and
   return status without actually changing CONTENTS.
*/

static bool
mips_elf_nullify_got_load (bfd *input_bfd, bfd_byte *contents,
			   const Elf_Internal_Rela *relocation,
			   reloc_howto_type *howto, bool doit)
{
  int r_type = ELF_R_TYPE (input_bfd, relocation->r_info);
  bfd_byte *location = contents + relocation->r_offset;
  bool nullified = true;
  bfd_vma x;

  _bfd_mips_elf_reloc_unshuffle (input_bfd, r_type, false, location);

  /* Obtain the current value.  */
  x = mips_elf_obtain_contents (howto, relocation, input_bfd, contents);

  /* Note that in the unshuffled MIPS16 encoding RX is at bits [21:19]
     while RY is at bits [18:16] of the combined 32-bit instruction word.  */
  if (mips16_reloc_p (r_type)
      && (((x >> 22) & 0x3ff) == 0x3d3				/* LW */
	  || ((x >> 22) & 0x3ff) == 0x3c7))			/* LD */
    x = (0x3cdU << 22) | (x & (7 << 16)) << 3;			/* LI */
  else if (micromips_reloc_p (r_type)
	   && ((x >> 26) & 0x37) == 0x37)			/* LW/LD */
    x = (0xc << 26) | (x & (0x1f << 21));			/* ADDIU */
  else if (((x >> 26) & 0x3f) == 0x23				/* LW */
	   || ((x >> 26) & 0x3f) == 0x37)			/* LD */
    x = (0x9 << 26) | (x & (0x1f << 16));			/* ADDIU */
  else
    nullified = false;

  /* Put the value into the output.  */
  if (doit && nullified)
    mips_elf_store_contents (howto, relocation, input_bfd, contents, x);

  _bfd_mips_elf_reloc_shuffle (input_bfd, r_type, false, location);

  return nullified;
}

/* Calculate the value produced by the RELOCATION (which comes from
   the INPUT_BFD).  The ADDEND is the addend to use for this
   RELOCATION; RELOCATION->R_ADDEND is ignored.

   The result of the relocation calculation is stored in VALUEP.
   On exit, set *CROSS_MODE_JUMP_P to true if the relocation field
   is a MIPS16 or microMIPS jump to standard MIPS code, or vice versa.

   This function returns bfd_reloc_continue if the caller need take no
   further action regarding this relocation, bfd_reloc_notsupported if
   something goes dramatically wrong, bfd_reloc_overflow if an
   overflow occurs, and bfd_reloc_ok to indicate success.  */

static bfd_reloc_status_type
mips_elf_calculate_relocation (bfd *abfd, bfd *input_bfd,
			       asection *input_section, bfd_byte *contents,
			       struct bfd_link_info *info,
			       const Elf_Internal_Rela *relocation,
			       bfd_vma addend, reloc_howto_type *howto,
			       Elf_Internal_Sym *local_syms,
			       asection **local_sections, bfd_vma *valuep,
			       const char **namep,
			       bool *cross_mode_jump_p,
			       bool save_addend)
{
  /* The eventual value we will return.  */
  bfd_vma value;
  /* The address of the symbol against which the relocation is
     occurring.  */
  bfd_vma symbol = 0;
  /* The final GP value to be used for the relocatable, executable, or
     shared object file being produced.  */
  bfd_vma gp;
  /* The place (section offset or address) of the storage unit being
     relocated.  */
  bfd_vma p;
  /* The value of GP used to create the relocatable object.  */
  bfd_vma gp0;
  /* The offset into the global offset table at which the address of
     the relocation entry symbol, adjusted by the addend, resides
     during execution.  */
  bfd_vma g = MINUS_ONE;
  /* The section in which the symbol referenced by the relocation is
     located.  */
  asection *sec = NULL;
  struct mips_elf_link_hash_entry *h = NULL;
  /* TRUE if the symbol referred to by this relocation is a local
     symbol.  */
  bool local_p, was_local_p;
  /* TRUE if the symbol referred to by this relocation is a section
     symbol.  */
  bool section_p = false;
  /* TRUE if the symbol referred to by this relocation is "_gp_disp".  */
  bool gp_disp_p = false;
  /* TRUE if the symbol referred to by this relocation is
     "__gnu_local_gp".  */
  bool gnu_local_gp_p = false;
  Elf_Internal_Shdr *symtab_hdr;
  size_t extsymoff;
  unsigned long r_symndx;
  int r_type;
  /* TRUE if overflow occurred during the calculation of the
     relocation value.  */
  bool overflowed_p;
  /* TRUE if this relocation refers to a MIPS16 function.  */
  bool target_is_16_bit_code_p = false;
  bool target_is_micromips_code_p = false;
  struct mips_elf_link_hash_table *htab;
  bfd *dynobj;
  bool resolved_to_zero;

  dynobj = elf_hash_table (info)->dynobj;
  htab = mips_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);

  /* Parse the relocation.  */
  r_symndx = ELF_R_SYM (input_bfd, relocation->r_info);
  r_type = ELF_R_TYPE (input_bfd, relocation->r_info);
  p = (input_section->output_section->vma
       + input_section->output_offset
       + relocation->r_offset);

  /* Assume that there will be no overflow.  */
  overflowed_p = false;

  /* Figure out whether or not the symbol is local, and get the offset
     used in the array of hash table entries.  */
  symtab_hdr = &elf_tdata (input_bfd)->symtab_hdr;
  local_p = mips_elf_local_relocation_p (input_bfd, relocation,
					 local_sections);
  was_local_p = local_p;
  if (! elf_bad_symtab (input_bfd))
    extsymoff = symtab_hdr->sh_info;
  else
    {
      /* The symbol table does not follow the rule that local symbols
	 must come before globals.  */
      extsymoff = 0;
    }

  /* Figure out the value of the symbol.  */
  if (local_p)
    {
      bool micromips_p = MICROMIPS_P (abfd);
      Elf_Internal_Sym *sym;

      sym = local_syms + r_symndx;
      sec = local_sections[r_symndx];

      section_p = ELF_ST_TYPE (sym->st_info) == STT_SECTION;

      symbol = sec->output_section->vma + sec->output_offset;
      if (!section_p || (sec->flags & SEC_MERGE))
	symbol += sym->st_value;
      if ((sec->flags & SEC_MERGE) && section_p)
	{
	  addend = _bfd_elf_rel_local_sym (abfd, sym, &sec, addend);
	  addend -= symbol;
	  addend += sec->output_section->vma + sec->output_offset;
	}

      /* MIPS16/microMIPS text labels should be treated as odd.  */
      if (ELF_ST_IS_COMPRESSED (sym->st_other))
	++symbol;

      /* Record the name of this symbol, for our caller.  */
      *namep = bfd_elf_string_from_elf_section (input_bfd,
						symtab_hdr->sh_link,
						sym->st_name);
      if (*namep == NULL || **namep == '\0')
	*namep = bfd_section_name (sec);

      /* For relocations against a section symbol and ones against no
	 symbol (absolute relocations) infer the ISA mode from the addend.  */
      if (section_p || r_symndx == STN_UNDEF)
	{
	  target_is_16_bit_code_p = (addend & 1) && !micromips_p;
	  target_is_micromips_code_p = (addend & 1) && micromips_p;
	}
      /* For relocations against an absolute symbol infer the ISA mode
	 from the value of the symbol plus addend.  */
      else if (bfd_is_abs_section (sec))
	{
	  target_is_16_bit_code_p = ((symbol + addend) & 1) && !micromips_p;
	  target_is_micromips_code_p = ((symbol + addend) & 1) && micromips_p;
	}
      /* Otherwise just use the regular symbol annotation available.  */
      else
	{
	  target_is_16_bit_code_p = ELF_ST_IS_MIPS16 (sym->st_other);
	  target_is_micromips_code_p = ELF_ST_IS_MICROMIPS (sym->st_other);
	}
    }
  else
    {
      /* ??? Could we use RELOC_FOR_GLOBAL_SYMBOL here ?  */

      /* For global symbols we look up the symbol in the hash-table.  */
      h = ((struct mips_elf_link_hash_entry *)
	   elf_sym_hashes (input_bfd) [r_symndx - extsymoff]);
      /* Find the real hash-table entry for this symbol.  */
      while (h->root.root.type == bfd_link_hash_indirect
	     || h->root.root.type == bfd_link_hash_warning)
	h = (struct mips_elf_link_hash_entry *) h->root.root.u.i.link;

      /* Record the name of this symbol, for our caller.  */
      *namep = h->root.root.root.string;

      /* See if this is the special _gp_disp symbol.  Note that such a
	 symbol must always be a global symbol.  */
      if (strcmp (*namep, "_gp_disp") == 0
	  && ! NEWABI_P (input_bfd))
	{
	  /* Relocations against _gp_disp are permitted only with
	     R_MIPS_HI16 and R_MIPS_LO16 relocations.  */
	  if (!hi16_reloc_p (r_type) && !lo16_reloc_p (r_type))
	    return bfd_reloc_notsupported;

	  gp_disp_p = true;
	}
      /* See if this is the special _gp symbol.  Note that such a
	 symbol must always be a global symbol.  */
      else if (strcmp (*namep, "__gnu_local_gp") == 0)
	gnu_local_gp_p = true;


      /* If this symbol is defined, calculate its address.  Note that
	 _gp_disp is a magic symbol, always implicitly defined by the
	 linker, so it's inappropriate to check to see whether or not
	 its defined.  */
      else if ((h->root.root.type == bfd_link_hash_defined
		|| h->root.root.type == bfd_link_hash_defweak)
	       && h->root.root.u.def.section)
	{
	  sec = h->root.root.u.def.section;
	  if (sec->output_section)
	    symbol = (h->root.root.u.def.value
		      + sec->output_section->vma
		      + sec->output_offset);
	  else
	    symbol = h->root.root.u.def.value;
	}
      else if (h->root.root.type == bfd_link_hash_undefweak)
	/* We allow relocations against undefined weak symbols, giving
	   it the value zero, so that you can undefined weak functions
	   and check to see if they exist by looking at their
	   addresses.  */
	symbol = 0;
      else if (info->unresolved_syms_in_objects == RM_IGNORE
	       && ELF_ST_VISIBILITY (h->root.other) == STV_DEFAULT)
	symbol = 0;
      else if (strcmp (*namep, SGI_COMPAT (input_bfd)
		       ? "_DYNAMIC_LINK" : "_DYNAMIC_LINKING") == 0)
	{
	  /* If this is a dynamic link, we should have created a
	     _DYNAMIC_LINK symbol or _DYNAMIC_LINKING(for normal mips) symbol
	     in _bfd_mips_elf_create_dynamic_sections.
	     Otherwise, we should define the symbol with a value of 0.
	     FIXME: It should probably get into the symbol table
	     somehow as well.  */
	  BFD_ASSERT (! bfd_link_pic (info));
	  BFD_ASSERT (bfd_get_section_by_name (abfd, ".dynamic") == NULL);
	  symbol = 0;
	}
      else if (ELF_MIPS_IS_OPTIONAL (h->root.other))
	{
	  /* This is an optional symbol - an Irix specific extension to the
	     ELF spec.  Ignore it for now.
	     XXX - FIXME - there is more to the spec for OPTIONAL symbols
	     than simply ignoring them, but we do not handle this for now.
	     For information see the "64-bit ELF Object File Specification"
	     which is available from here:
	     http://techpubs.sgi.com/library/manuals/4000/007-4658-001/pdf/007-4658-001.pdf  */
	  symbol = 0;
	}
      else
	{
          bool reject_undefined
	    = ((info->unresolved_syms_in_objects == RM_DIAGNOSE
		&& !info->warn_unresolved_syms)
	       || ELF_ST_VISIBILITY (h->root.other) != STV_DEFAULT);

	  info->callbacks->undefined_symbol
	    (info, h->root.root.root.string, input_bfd,
	     input_section, relocation->r_offset, reject_undefined);

	  if (reject_undefined)
	    return bfd_reloc_undefined;

	  symbol = 0;
	}

      target_is_16_bit_code_p = ELF_ST_IS_MIPS16 (h->root.other);
      target_is_micromips_code_p = ELF_ST_IS_MICROMIPS (h->root.other);
    }

  /* If this is a reference to a 16-bit function with a stub, we need
     to redirect the relocation to the stub unless:

     (a) the relocation is for a MIPS16 JAL;

     (b) the relocation is for a MIPS16 PIC call, and there are no
	 non-MIPS16 uses of the GOT slot; or

     (c) the section allows direct references to MIPS16 functions.  */
  if (r_type != R_MIPS16_26
      && !bfd_link_relocatable (info)
      && ((h != NULL
	   && h->fn_stub != NULL
	   && (r_type != R_MIPS16_CALL16 || h->need_fn_stub))
	  || (local_p
	      && mips_elf_tdata (input_bfd)->local_stubs != NULL
	      && mips_elf_tdata (input_bfd)->local_stubs[r_symndx] != NULL))
      && !section_allows_mips16_refs_p (input_section))
    {
      /* This is a 32- or 64-bit call to a 16-bit function.  We should
	 have already noticed that we were going to need the
	 stub.  */
      if (local_p)
	{
	  sec = mips_elf_tdata (input_bfd)->local_stubs[r_symndx];
	  value = 0;
	}
      else
	{
	  BFD_ASSERT (h->need_fn_stub);
	  if (h->la25_stub)
	    {
	      /* If a LA25 header for the stub itself exists, point to the
		 prepended LUI/ADDIU sequence.  */
	      sec = h->la25_stub->stub_section;
	      value = h->la25_stub->offset;
	    }
	  else
	    {
	      sec = h->fn_stub;
	      value = 0;
	    }
	}

      symbol = sec->output_section->vma + sec->output_offset + value;
      /* The target is 16-bit, but the stub isn't.  */
      target_is_16_bit_code_p = false;
    }
  /* If this is a MIPS16 call with a stub, that is made through the PLT or
     to a standard MIPS function, we need to redirect the call to the stub.
     Note that we specifically exclude R_MIPS16_CALL16 from this behavior;
     indirect calls should use an indirect stub instead.  */
  else if (r_type == R_MIPS16_26 && !bfd_link_relocatable (info)
	   && ((h != NULL && (h->call_stub != NULL || h->call_fp_stub != NULL))
	       || (local_p
		   && mips_elf_tdata (input_bfd)->local_call_stubs != NULL
		   && mips_elf_tdata (input_bfd)->local_call_stubs[r_symndx] != NULL))
	   && ((h != NULL && h->use_plt_entry) || !target_is_16_bit_code_p))
    {
      if (local_p)
	sec = mips_elf_tdata (input_bfd)->local_call_stubs[r_symndx];
      else
	{
	  /* If both call_stub and call_fp_stub are defined, we can figure
	     out which one to use by checking which one appears in the input
	     file.  */
	  if (h->call_stub != NULL && h->call_fp_stub != NULL)
	    {
	      asection *o;

	      sec = NULL;
	      for (o = input_bfd->sections; o != NULL; o = o->next)
		{
		  if (CALL_FP_STUB_P (bfd_section_name (o)))
		    {
		      sec = h->call_fp_stub;
		      break;
		    }
		}
	      if (sec == NULL)
		sec = h->call_stub;
	    }
	  else if (h->call_stub != NULL)
	    sec = h->call_stub;
	  else
	    sec = h->call_fp_stub;
	}

      BFD_ASSERT (sec->size > 0);
      symbol = sec->output_section->vma + sec->output_offset;
    }
  /* If this is a direct call to a PIC function, redirect to the
     non-PIC stub.  */
  else if (h != NULL && h->la25_stub
	   && mips_elf_relocation_needs_la25_stub (input_bfd, r_type,
						   target_is_16_bit_code_p))
    {
	symbol = (h->la25_stub->stub_section->output_section->vma
		  + h->la25_stub->stub_section->output_offset
		  + h->la25_stub->offset);
	if (ELF_ST_IS_MICROMIPS (h->root.other))
	  symbol |= 1;
    }
  /* For direct MIPS16 and microMIPS calls make sure the compressed PLT
     entry is used if a standard PLT entry has also been made.  In this
     case the symbol will have been set by mips_elf_set_plt_sym_value
     to point to the standard PLT entry, so redirect to the compressed
     one.  */
  else if ((mips16_branch_reloc_p (r_type)
	    || micromips_branch_reloc_p (r_type))
	   && !bfd_link_relocatable (info)
	   && h != NULL
	   && h->use_plt_entry
	   && h->root.plt.plist->comp_offset != MINUS_ONE
	   && h->root.plt.plist->mips_offset != MINUS_ONE)
    {
      bool micromips_p = MICROMIPS_P (abfd);

      sec = htab->root.splt;
      symbol = (sec->output_section->vma
		+ sec->output_offset
		+ htab->plt_header_size
		+ htab->plt_mips_offset
		+ h->root.plt.plist->comp_offset
		+ 1);

      target_is_16_bit_code_p = !micromips_p;
      target_is_micromips_code_p = micromips_p;
    }

  /* Make sure MIPS16 and microMIPS are not used together.  */
  if ((mips16_branch_reloc_p (r_type) && target_is_micromips_code_p)
      || (micromips_branch_reloc_p (r_type) && target_is_16_bit_code_p))
   {
      _bfd_error_handler
	(_("MIPS16 and microMIPS functions cannot call each other"));
      return bfd_reloc_notsupported;
   }

  /* Calls from 16-bit code to 32-bit code and vice versa require the
     mode change.  However, we can ignore calls to undefined weak symbols,
     which should never be executed at runtime.  This exception is important
     because the assembly writer may have "known" that any definition of the
     symbol would be 16-bit code, and that direct jumps were therefore
     acceptable.  */
  *cross_mode_jump_p = (!bfd_link_relocatable (info)
			&& !(h && h->root.root.type == bfd_link_hash_undefweak)
			&& ((mips16_branch_reloc_p (r_type)
			     && !target_is_16_bit_code_p)
			    || (micromips_branch_reloc_p (r_type)
				&& !target_is_micromips_code_p)
			    || ((branch_reloc_p (r_type)
				 || r_type == R_MIPS_JALR)
				&& (target_is_16_bit_code_p
				    || target_is_micromips_code_p))));

  resolved_to_zero = (h != NULL
		      && UNDEFWEAK_NO_DYNAMIC_RELOC (info, &h->root));

  switch (r_type)
    {
    case R_MIPS16_CALL16:
    case R_MIPS16_GOT16:
    case R_MIPS_CALL16:
    case R_MIPS_GOT16:
    case R_MIPS_GOT_PAGE:
    case R_MIPS_GOT_DISP:
    case R_MIPS_GOT_LO16:
    case R_MIPS_CALL_LO16:
    case R_MICROMIPS_CALL16:
    case R_MICROMIPS_GOT16:
    case R_MICROMIPS_GOT_PAGE:
    case R_MICROMIPS_GOT_DISP:
    case R_MICROMIPS_GOT_LO16:
    case R_MICROMIPS_CALL_LO16:
      if (resolved_to_zero
	  && !bfd_link_relocatable (info)
	  && bfd_reloc_offset_in_range (howto, input_bfd, input_section,
					relocation->r_offset)
	  && mips_elf_nullify_got_load (input_bfd, contents,
					relocation, howto, true))
	return bfd_reloc_continue;

      /* Fall through.  */
    case R_MIPS_GOT_HI16:
    case R_MIPS_CALL_HI16:
    case R_MICROMIPS_GOT_HI16:
    case R_MICROMIPS_CALL_HI16:
      if (resolved_to_zero
	  && htab->use_absolute_zero
	  && bfd_link_pic (info))
	{
	  /* Redirect to the special `__gnu_absolute_zero' symbol.  */
	  h = mips_elf_link_hash_lookup (htab, "__gnu_absolute_zero",
					 false, false, false);
	  BFD_ASSERT (h != NULL);
	}
      break;
    }

  local_p = (h == NULL || mips_use_local_got_p (info, h));

  gp0 = _bfd_get_gp_value (input_bfd);
  gp = _bfd_get_gp_value (abfd);
  if (htab->got_info)
    gp += mips_elf_adjust_gp (abfd, htab->got_info, input_bfd);

  if (gnu_local_gp_p)
    symbol = gp;

  /* Global R_MIPS_GOT_PAGE/R_MICROMIPS_GOT_PAGE relocations are equivalent
     to R_MIPS_GOT_DISP/R_MICROMIPS_GOT_DISP.  The addend is applied by the
     corresponding R_MIPS_GOT_OFST/R_MICROMIPS_GOT_OFST.  */
  if (got_page_reloc_p (r_type) && !local_p)
    {
      r_type = (micromips_reloc_p (r_type)
		? R_MICROMIPS_GOT_DISP : R_MIPS_GOT_DISP);
      addend = 0;
    }

  /* If we haven't already determined the GOT offset, and we're going
     to need it, get it now.  */
  switch (r_type)
    {
    case R_MIPS16_CALL16:
    case R_MIPS16_GOT16:
    case R_MIPS_CALL16:
    case R_MIPS_GOT16:
    case R_MIPS_GOT_DISP:
    case R_MIPS_GOT_HI16:
    case R_MIPS_CALL_HI16:
    case R_MIPS_GOT_LO16:
    case R_MIPS_CALL_LO16:
    case R_MICROMIPS_CALL16:
    case R_MICROMIPS_GOT16:
    case R_MICROMIPS_GOT_DISP:
    case R_MICROMIPS_GOT_HI16:
    case R_MICROMIPS_CALL_HI16:
    case R_MICROMIPS_GOT_LO16:
    case R_MICROMIPS_CALL_LO16:
    case R_MIPS_TLS_GD:
    case R_MIPS_TLS_GOTTPREL:
    case R_MIPS_TLS_LDM:
    case R_MIPS16_TLS_GD:
    case R_MIPS16_TLS_GOTTPREL:
    case R_MIPS16_TLS_LDM:
    case R_MICROMIPS_TLS_GD:
    case R_MICROMIPS_TLS_GOTTPREL:
    case R_MICROMIPS_TLS_LDM:
      /* Find the index into the GOT where this value is located.  */
      if (tls_ldm_reloc_p (r_type))
	{
	  g = mips_elf_local_got_index (abfd, input_bfd, info,
					0, 0, NULL, r_type);
	  if (g == MINUS_ONE)
	    return bfd_reloc_outofrange;
	}
      else if (!local_p)
	{
	  /* On VxWorks, CALL relocations should refer to the .got.plt
	     entry, which is initialized to point at the PLT stub.  */
	  if (htab->root.target_os == is_vxworks
	      && (call_hi16_reloc_p (r_type)
		  || call_lo16_reloc_p (r_type)
		  || call16_reloc_p (r_type)))
	    {
	      BFD_ASSERT (addend == 0);
	      BFD_ASSERT (h->root.needs_plt);
	      g = mips_elf_gotplt_index (info, &h->root);
	    }
	  else
	    {
	      BFD_ASSERT (addend == 0);
	      g = mips_elf_global_got_index (abfd, info, input_bfd,
					     &h->root, r_type);
	      if (!TLS_RELOC_P (r_type)
		  && !elf_hash_table (info)->dynamic_sections_created)
		/* This is a static link.  We must initialize the GOT entry.  */
		MIPS_ELF_PUT_WORD (dynobj, symbol, htab->root.sgot->contents + g);
	    }
	}
      else if (htab->root.target_os != is_vxworks
	       && (call16_reloc_p (r_type) || got16_reloc_p (r_type)))
	/* The calculation below does not involve "g".  */
	break;
      else
	{
	  g = mips_elf_local_got_index (abfd, input_bfd, info,
					symbol + addend, r_symndx, h, r_type);
	  if (g == MINUS_ONE)
	    return bfd_reloc_outofrange;
	}

      /* Convert GOT indices to actual offsets.  */
      g = mips_elf_got_offset_from_index (info, abfd, input_bfd, g);
      break;
    }

  /* Relocations against the VxWorks __GOTT_BASE__ and __GOTT_INDEX__
     symbols are resolved by the loader.  Add them to .rela.dyn.  */
  if (h != NULL && is_gott_symbol (info, &h->root))
    {
      Elf_Internal_Rela outrel;
      bfd_byte *loc;
      asection *s;

      s = mips_elf_rel_dyn_section (info, false);
      loc = s->contents + s->reloc_count++ * sizeof (Elf32_External_Rela);

      outrel.r_offset = (input_section->output_section->vma
			 + input_section->output_offset
			 + relocation->r_offset);
      outrel.r_info = ELF32_R_INFO (h->root.dynindx, r_type);
      outrel.r_addend = addend;
      bfd_elf32_swap_reloca_out (abfd, &outrel, loc);

      /* If we've written this relocation for a readonly section,
	 we need to set DF_TEXTREL again, so that we do not delete the
	 DT_TEXTREL tag.  */
      if (MIPS_ELF_READONLY_SECTION (input_section))
	info->flags |= DF_TEXTREL;

      *valuep = 0;
      return bfd_reloc_ok;
    }

  /* Figure out what kind of relocation is being performed.  */
  switch (r_type)
    {
    case R_MIPS_NONE:
      return bfd_reloc_continue;

    case R_MIPS_16:
      if (howto->partial_inplace)
	addend = _bfd_mips_elf_sign_extend (addend, 16);
      value = symbol + addend;
      overflowed_p = mips_elf_overflow_p (value, 16);
      break;

    case R_MIPS_32:
    case R_MIPS_REL32:
    case R_MIPS_64:
      if ((bfd_link_pic (info)
	   || (htab->root.dynamic_sections_created
	       && h != NULL
	       && h->root.def_dynamic
	       && !h->root.def_regular
	       && !h->has_static_relocs))
	  && r_symndx != STN_UNDEF
	  && (h == NULL
	      || h->root.root.type != bfd_link_hash_undefweak
	      || (ELF_ST_VISIBILITY (h->root.other) == STV_DEFAULT
		  && !resolved_to_zero))
	  && (input_section->flags & SEC_ALLOC) != 0)
	{
	  /* If we're creating a shared library, then we can't know
	     where the symbol will end up.  So, we create a relocation
	     record in the output, and leave the job up to the dynamic
	     linker.  We must do the same for executable references to
	     shared library symbols, unless we've decided to use copy
	     relocs or PLTs instead.  */
	  value = addend;
	  if (!mips_elf_create_dynamic_relocation (abfd,
						   info,
						   relocation,
						   h,
						   sec,
						   symbol,
						   &value,
						   input_section))
	    return bfd_reloc_undefined;
	}
      else
	{
	  if (r_type != R_MIPS_REL32)
	    value = symbol + addend;
	  else
	    value = addend;
	}
      value &= howto->dst_mask;
      break;

    case R_MIPS_PC32:
      value = symbol + addend - p;
      value &= howto->dst_mask;
      break;

    case R_MIPS16_26:
      /* The calculation for R_MIPS16_26 is just the same as for an
	 R_MIPS_26.  It's only the storage of the relocated field into
	 the output file that's different.  That's handled in
	 mips_elf_perform_relocation.  So, we just fall through to the
	 R_MIPS_26 case here.  */
    case R_MIPS_26:
    case R_MICROMIPS_26_S1:
      {
	unsigned int shift;

	/* Shift is 2, unusually, for microMIPS JALX.  */
	shift = (!*cross_mode_jump_p && r_type == R_MICROMIPS_26_S1) ? 1 : 2;

	if (howto->partial_inplace && !section_p)
	  value = _bfd_mips_elf_sign_extend (addend, 26 + shift);
	else
	  value = addend;
	value += symbol;

	/* Make sure the target of a jump is suitably aligned.  Bit 0 must
	   be the correct ISA mode selector except for weak undefined
	   symbols.  */
	if ((was_local_p || h->root.root.type != bfd_link_hash_undefweak)
	    && (*cross_mode_jump_p
		? (value & 3) != (r_type == R_MIPS_26)
		: (value & ((1 << shift) - 1)) != (r_type != R_MIPS_26)))
	  return bfd_reloc_outofrange;

	value >>= shift;
	if (was_local_p || h->root.root.type != bfd_link_hash_undefweak)
	  overflowed_p = (value >> 26) != ((p + 4) >> (26 + shift));
	value &= howto->dst_mask;
      }
      break;

    case R_MIPS_TLS_DTPREL_HI16:
    case R_MIPS16_TLS_DTPREL_HI16:
    case R_MICROMIPS_TLS_DTPREL_HI16:
      value = (mips_elf_high (addend + symbol - dtprel_base (info))
	       & howto->dst_mask);
      break;

    case R_MIPS_TLS_DTPREL_LO16:
    case R_MIPS_TLS_DTPREL32:
    case R_MIPS_TLS_DTPREL64:
    case R_MIPS16_TLS_DTPREL_LO16:
    case R_MICROMIPS_TLS_DTPREL_LO16:
      value = (symbol + addend - dtprel_base (info)) & howto->dst_mask;
      break;

    case R_MIPS_TLS_TPREL_HI16:
    case R_MIPS16_TLS_TPREL_HI16:
    case R_MICROMIPS_TLS_TPREL_HI16:
      value = (mips_elf_high (addend + symbol - tprel_base (info))
	       & howto->dst_mask);
      break;

    case R_MIPS_TLS_TPREL_LO16:
    case R_MIPS_TLS_TPREL32:
    case R_MIPS_TLS_TPREL64:
    case R_MIPS16_TLS_TPREL_LO16:
    case R_MICROMIPS_TLS_TPREL_LO16:
      value = (symbol + addend - tprel_base (info)) & howto->dst_mask;
      break;

    case R_MIPS_HI16:
    case R_MIPS16_HI16:
    case R_MICROMIPS_HI16:
      if (!gp_disp_p)
	{
	  value = mips_elf_high (addend + symbol);
	  value &= howto->dst_mask;
	}
      else
	{
	  /* For MIPS16 ABI code we generate this sequence
		0: li      $v0,%hi(_gp_disp)
		4: addiupc $v1,%lo(_gp_disp)
		8: sll     $v0,16
	       12: addu    $v0,$v1
	       14: move    $gp,$v0
	     So the offsets of hi and lo relocs are the same, but the
	     base $pc is that used by the ADDIUPC instruction at $t9 + 4.
	     ADDIUPC clears the low two bits of the instruction address,
	     so the base is ($t9 + 4) & ~3.  */
	  if (r_type == R_MIPS16_HI16)
	    value = mips_elf_high (addend + gp - ((p + 4) & ~(bfd_vma) 0x3));
	  /* The microMIPS .cpload sequence uses the same assembly
	     instructions as the traditional psABI version, but the
	     incoming $t9 has the low bit set.  */
	  else if (r_type == R_MICROMIPS_HI16)
	    value = mips_elf_high (addend + gp - p - 1);
	  else
	    value = mips_elf_high (addend + gp - p);
	}
      break;

    case R_MIPS_LO16:
    case R_MIPS16_LO16:
    case R_MICROMIPS_LO16:
    case R_MICROMIPS_HI0_LO16:
      if (!gp_disp_p)
	value = (symbol + addend) & howto->dst_mask;
      else
	{
	  /* See the comment for R_MIPS16_HI16 above for the reason
	     for this conditional.  */
	  if (r_type == R_MIPS16_LO16)
	    value = addend + gp - (p & ~(bfd_vma) 0x3);
	  else if (r_type == R_MICROMIPS_LO16
		   || r_type == R_MICROMIPS_HI0_LO16)
	    value = addend + gp - p + 3;
	  else
	    value = addend + gp - p + 4;
	  /* The MIPS ABI requires checking the R_MIPS_LO16 relocation
	     for overflow.  But, on, say, IRIX5, relocations against
	     _gp_disp are normally generated from the .cpload
	     pseudo-op.  It generates code that normally looks like
	     this:

	       lui    $gp,%hi(_gp_disp)
	       addiu  $gp,$gp,%lo(_gp_disp)
	       addu   $gp,$gp,$t9

	     Here $t9 holds the address of the function being called,
	     as required by the MIPS ELF ABI.  The R_MIPS_LO16
	     relocation can easily overflow in this situation, but the
	     R_MIPS_HI16 relocation will handle the overflow.
	     Therefore, we consider this a bug in the MIPS ABI, and do
	     not check for overflow here.  */
	}
      break;

    case R_MIPS_LITERAL:
    case R_MICROMIPS_LITERAL:
      /* Because we don't merge literal sections, we can handle this
	 just like R_MIPS_GPREL16.  In the long run, we should merge
	 shared literals, and then we will need to additional work
	 here.  */

      /* Fall through.  */

    case R_MIPS16_GPREL:
      /* The R_MIPS16_GPREL performs the same calculation as
	 R_MIPS_GPREL16, but stores the relocated bits in a different
	 order.  We don't need to do anything special here; the
	 differences are handled in mips_elf_perform_relocation.  */
    case R_MIPS_GPREL16:
    case R_MICROMIPS_GPREL7_S2:
    case R_MICROMIPS_GPREL16:
      /* Only sign-extend the addend if it was extracted from the
	 instruction.  If the addend was separate, leave it alone,
	 otherwise we may lose significant bits.  */
      if (howto->partial_inplace)
	addend = _bfd_mips_elf_sign_extend (addend, 16);
      value = symbol + addend - gp;
      /* If the symbol was local, any earlier relocatable links will
	 have adjusted its addend with the gp offset, so compensate
	 for that now.  Don't do it for symbols forced local in this
	 link, though, since they won't have had the gp offset applied
	 to them before.  */
      if (was_local_p)
	value += gp0;
      if (was_local_p || h->root.root.type != bfd_link_hash_undefweak)
	overflowed_p = mips_elf_overflow_p (value, 16);
      break;

    case R_MIPS16_GOT16:
    case R_MIPS16_CALL16:
    case R_MIPS_GOT16:
    case R_MIPS_CALL16:
    case R_MICROMIPS_GOT16:
    case R_MICROMIPS_CALL16:
      /* VxWorks does not have separate local and global semantics for
	 R_MIPS*_GOT16; every relocation evaluates to "G".  */
      if (htab->root.target_os != is_vxworks && local_p)
	{
	  value = mips_elf_got16_entry (abfd, input_bfd, info,
					symbol + addend, !was_local_p);
	  if (value == MINUS_ONE)
	    return bfd_reloc_outofrange;
	  value
	    = mips_elf_got_offset_from_index (info, abfd, input_bfd, value);
	  overflowed_p = mips_elf_overflow_p (value, 16);
	  break;
	}

      /* Fall through.  */

    case R_MIPS_TLS_GD:
    case R_MIPS_TLS_GOTTPREL:
    case R_MIPS_TLS_LDM:
    case R_MIPS_GOT_DISP:
    case R_MIPS16_TLS_GD:
    case R_MIPS16_TLS_GOTTPREL:
    case R_MIPS16_TLS_LDM:
    case R_MICROMIPS_TLS_GD:
    case R_MICROMIPS_TLS_GOTTPREL:
    case R_MICROMIPS_TLS_LDM:
    case R_MICROMIPS_GOT_DISP:
      value = g;
      overflowed_p = mips_elf_overflow_p (value, 16);
      break;

    case R_MIPS_GPREL32:
      value = (addend + symbol + gp0 - gp);
      if (!save_addend)
	value &= howto->dst_mask;
      break;

    case R_MIPS_PC16:
    case R_MIPS_GNU_REL16_S2:
      if (howto->partial_inplace)
	addend = _bfd_mips_elf_sign_extend (addend, 18);

      /* No need to exclude weak undefined symbols here as they resolve
	 to 0 and never set `*cross_mode_jump_p', so this alignment check
	 will never trigger for them.  */
      if (*cross_mode_jump_p
	  ? ((symbol + addend) & 3) != 1
	  : ((symbol + addend) & 3) != 0)
	return bfd_reloc_outofrange;

      value = symbol + addend - p;
      if (was_local_p || h->root.root.type != bfd_link_hash_undefweak)
	overflowed_p = mips_elf_overflow_p (value, 18);
      value >>= howto->rightshift;
      value &= howto->dst_mask;
      break;

    case R_MIPS16_PC16_S1:
      if (howto->partial_inplace)
	addend = _bfd_mips_elf_sign_extend (addend, 17);

      if ((was_local_p || h->root.root.type != bfd_link_hash_undefweak)
	  && (*cross_mode_jump_p
	      ? ((symbol + addend) & 3) != 0
	      : ((symbol + addend) & 1) == 0))
	return bfd_reloc_outofrange;

      value = symbol + addend - p;
      if (was_local_p || h->root.root.type != bfd_link_hash_undefweak)
	overflowed_p = mips_elf_overflow_p (value, 17);
      value >>= howto->rightshift;
      value &= howto->dst_mask;
      break;

    case R_MIPS_PC21_S2:
      if (howto->partial_inplace)
	addend = _bfd_mips_elf_sign_extend (addend, 23);

      if ((symbol + addend) & 3)
	return bfd_reloc_outofrange;

      value = symbol + addend - p;
      if (was_local_p || h->root.root.type != bfd_link_hash_undefweak)
	overflowed_p = mips_elf_overflow_p (value, 23);
      value >>= howto->rightshift;
      value &= howto->dst_mask;
      break;

    case R_MIPS_PC26_S2:
      if (howto->partial_inplace)
	addend = _bfd_mips_elf_sign_extend (addend, 28);

      if ((symbol + addend) & 3)
	return bfd_reloc_outofrange;

      value = symbol + addend - p;
      if (was_local_p || h->root.root.type != bfd_link_hash_undefweak)
	overflowed_p = mips_elf_overflow_p (value, 28);
      value >>= howto->rightshift;
      value &= howto->dst_mask;
      break;

    case R_MIPS_PC18_S3:
      if (howto->partial_inplace)
	addend = _bfd_mips_elf_sign_extend (addend, 21);

      if ((symbol + addend) & 7)
	return bfd_reloc_outofrange;

      value = symbol + addend - ((p | 7) ^ 7);
      if (was_local_p || h->root.root.type != bfd_link_hash_undefweak)
	overflowed_p = mips_elf_overflow_p (value, 21);
      value >>= howto->rightshift;
      value &= howto->dst_mask;
      break;

    case R_MIPS_PC19_S2:
      if (howto->partial_inplace)
	addend = _bfd_mips_elf_sign_extend (addend, 21);

      if ((symbol + addend) & 3)
	return bfd_reloc_outofrange;

      value = symbol + addend - p;
      if (was_local_p || h->root.root.type != bfd_link_hash_undefweak)
	overflowed_p = mips_elf_overflow_p (value, 21);
      value >>= howto->rightshift;
      value &= howto->dst_mask;
      break;

    case R_MIPS_PCHI16:
      value = mips_elf_high (symbol + addend - p);
      value &= howto->dst_mask;
      break;

    case R_MIPS_PCLO16:
      if (howto->partial_inplace)
	addend = _bfd_mips_elf_sign_extend (addend, 16);
      value = symbol + addend - p;
      value &= howto->dst_mask;
      break;

    case R_MICROMIPS_PC7_S1:
      if (howto->partial_inplace)
	addend = _bfd_mips_elf_sign_extend (addend, 8);

      if ((was_local_p || h->root.root.type != bfd_link_hash_undefweak)
	  && (*cross_mode_jump_p
	      ? ((symbol + addend + 2) & 3) != 0
	      : ((symbol + addend + 2) & 1) == 0))
	return bfd_reloc_outofrange;

      value = symbol + addend - p;
      if (was_local_p || h->root.root.type != bfd_link_hash_undefweak)
	overflowed_p = mips_elf_overflow_p (value, 8);
      value >>= howto->rightshift;
      value &= howto->dst_mask;
      break;

    case R_MICROMIPS_PC10_S1:
      if (howto->partial_inplace)
	addend = _bfd_mips_elf_sign_extend (addend, 11);

      if ((was_local_p || h->root.root.type != bfd_link_hash_undefweak)
	  && (*cross_mode_jump_p
	      ? ((symbol + addend + 2) & 3) != 0
	      : ((symbol + addend + 2) & 1) == 0))
	return bfd_reloc_outofrange;

      value = symbol + addend - p;
      if (was_local_p || h->root.root.type != bfd_link_hash_undefweak)
	overflowed_p = mips_elf_overflow_p (value, 11);
      value >>= howto->rightshift;
      value &= howto->dst_mask;
      break;

    case R_MICROMIPS_PC16_S1:
      if (howto->partial_inplace)
	addend = _bfd_mips_elf_sign_extend (addend, 17);

      if ((was_local_p || h->root.root.type != bfd_link_hash_undefweak)
	  && (*cross_mode_jump_p
	      ? ((symbol + addend) & 3) != 0
	      : ((symbol + addend) & 1) == 0))
	return bfd_reloc_outofrange;

      value = symbol + addend - p;
      if (was_local_p || h->root.root.type != bfd_link_hash_undefweak)
	overflowed_p = mips_elf_overflow_p (value, 17);
      value >>= howto->rightshift;
      value &= howto->dst_mask;
      break;

    case R_MICROMIPS_PC23_S2:
      if (howto->partial_inplace)
	addend = _bfd_mips_elf_sign_extend (addend, 25);
      value = symbol + addend - ((p | 3) ^ 3);
      if (was_local_p || h->root.root.type != bfd_link_hash_undefweak)
	overflowed_p = mips_elf_overflow_p (value, 25);
      value >>= howto->rightshift;
      value &= howto->dst_mask;
      break;

    case R_MIPS_GOT_HI16:
    case R_MIPS_CALL_HI16:
    case R_MICROMIPS_GOT_HI16:
    case R_MICROMIPS_CALL_HI16:
      /* We're allowed to handle these two relocations identically.
	 The dynamic linker is allowed to handle the CALL relocations
	 differently by creating a lazy evaluation stub.  */
      value = g;
      value = mips_elf_high (value);
      value &= howto->dst_mask;
      break;

    case R_MIPS_GOT_LO16:
    case R_MIPS_CALL_LO16:
    case R_MICROMIPS_GOT_LO16:
    case R_MICROMIPS_CALL_LO16:
      value = g & howto->dst_mask;
      break;

    case R_MIPS_GOT_PAGE:
    case R_MICROMIPS_GOT_PAGE:
      value = mips_elf_got_page (abfd, input_bfd, info, symbol + addend, NULL);
      if (value == MINUS_ONE)
	return bfd_reloc_outofrange;
      value = mips_elf_got_offset_from_index (info, abfd, input_bfd, value);
      overflowed_p = mips_elf_overflow_p (value, 16);
      break;

    case R_MIPS_GOT_OFST:
    case R_MICROMIPS_GOT_OFST:
      if (local_p)
	mips_elf_got_page (abfd, input_bfd, info, symbol + addend, &value);
      else
	value = addend;
      overflowed_p = mips_elf_overflow_p (value, 16);
      break;

    case R_MIPS_SUB:
    case R_MICROMIPS_SUB:
      value = symbol - addend;
      value &= howto->dst_mask;
      break;

    case R_MIPS_HIGHER:
    case R_MICROMIPS_HIGHER:
      value = mips_elf_higher (addend + symbol);
      value &= howto->dst_mask;
      break;

    case R_MIPS_HIGHEST:
    case R_MICROMIPS_HIGHEST:
      value = mips_elf_highest (addend + symbol);
      value &= howto->dst_mask;
      break;

    case R_MIPS_SCN_DISP:
    case R_MICROMIPS_SCN_DISP:
      value = symbol + addend - sec->output_offset;
      value &= howto->dst_mask;
      break;

    case R_MIPS_JALR:
    case R_MICROMIPS_JALR:
      /* This relocation is only a hint.  In some cases, we optimize
	 it into a bal instruction.  But we don't try to optimize
	 when the symbol does not resolve locally.  */
      if (h != NULL && !SYMBOL_CALLS_LOCAL (info, &h->root))
	return bfd_reloc_continue;
      /* We can't optimize cross-mode jumps either.  */
      if (*cross_mode_jump_p)
	return bfd_reloc_continue;
      value = symbol + addend;
      /* Neither we can non-instruction-aligned targets.  */
      if (r_type == R_MIPS_JALR ? (value & 3) != 0 : (value & 1) == 0)
	return bfd_reloc_continue;
      break;

    case R_MIPS_PJUMP:
    case R_MIPS_GNU_VTINHERIT:
    case R_MIPS_GNU_VTENTRY:
      /* We don't do anything with these at present.  */
      return bfd_reloc_continue;

    default:
      /* An unrecognized relocation type.  */
      return bfd_reloc_notsupported;
    }

  /* Store the VALUE for our caller.  */
  *valuep = value;
  return overflowed_p ? bfd_reloc_overflow : bfd_reloc_ok;
}

/* It has been determined that the result of the RELOCATION is the
   VALUE.  Use HOWTO to place VALUE into the output file at the
   appropriate position.  The SECTION is the section to which the
   relocation applies.
   CROSS_MODE_JUMP_P is true if the relocation field
   is a MIPS16 or microMIPS jump to standard MIPS code, or vice versa.

   Returns FALSE if anything goes wrong.  */

static bool
mips_elf_perform_relocation (struct bfd_link_info *info,
			     reloc_howto_type *howto,
			     const Elf_Internal_Rela *relocation,
			     bfd_vma value, bfd *input_bfd,
			     asection *input_section, bfd_byte *contents,
			     bool cross_mode_jump_p)
{
  bfd_vma x;
  bfd_byte *location;
  int r_type = ELF_R_TYPE (input_bfd, relocation->r_info);

  /* Figure out where the relocation is occurring.  */
  location = contents + relocation->r_offset;

  _bfd_mips_elf_reloc_unshuffle (input_bfd, r_type, false, location);

  /* Obtain the current value.  */
  x = mips_elf_obtain_contents (howto, relocation, input_bfd, contents);

  /* Clear the field we are setting.  */
  x &= ~howto->dst_mask;

  /* Set the field.  */
  x |= (value & howto->dst_mask);

  /* Detect incorrect JALX usage.  If required, turn JAL or BAL into JALX.  */
  if (!cross_mode_jump_p && jal_reloc_p (r_type))
    {
      bfd_vma opcode = x >> 26;

      if (r_type == R_MIPS16_26 ? opcode == 0x7
	  : r_type == R_MICROMIPS_26_S1 ? opcode == 0x3c
	  : opcode == 0x1d)
	{
	  info->callbacks->einfo
	    (_("%X%H: unsupported JALX to the same ISA mode\n"),
	     input_bfd, input_section, relocation->r_offset);
	  return true;
	}
    }
  if (cross_mode_jump_p && jal_reloc_p (r_type))
    {
      bool ok;
      bfd_vma opcode = x >> 26;
      bfd_vma jalx_opcode;

      /* Check to see if the opcode is already JAL or JALX.  */
      if (r_type == R_MIPS16_26)
	{
	  ok = ((opcode == 0x6) || (opcode == 0x7));
	  jalx_opcode = 0x7;
	}
      else if (r_type == R_MICROMIPS_26_S1)
	{
	  ok = ((opcode == 0x3d) || (opcode == 0x3c));
	  jalx_opcode = 0x3c;
	}
      else
	{
	  ok = ((opcode == 0x3) || (opcode == 0x1d));
	  jalx_opcode = 0x1d;
	}

      /* If the opcode is not JAL or JALX, there's a problem.  We cannot
	 convert J or JALS to JALX.  */
      if (!ok)
	{
	  info->callbacks->einfo
	    (_("%X%H: unsupported jump between ISA modes; "
	       "consider recompiling with interlinking enabled\n"),
	     input_bfd, input_section, relocation->r_offset);
	  return true;
	}

      /* Make this the JALX opcode.  */
      x = (x & ~(0x3fu << 26)) | (jalx_opcode << 26);
    }
  else if (cross_mode_jump_p && b_reloc_p (r_type))
    {
      bool ok = false;
      bfd_vma opcode = x >> 16;
      bfd_vma jalx_opcode = 0;
      bfd_vma sign_bit = 0;
      bfd_vma addr;
      bfd_vma dest;

      if (r_type == R_MICROMIPS_PC16_S1)
	{
	  ok = opcode == 0x4060;
	  jalx_opcode = 0x3c;
	  sign_bit = 0x10000;
	  value <<= 1;
	}
      else if (r_type == R_MIPS_PC16 || r_type == R_MIPS_GNU_REL16_S2)
	{
	  ok = opcode == 0x411;
	  jalx_opcode = 0x1d;
	  sign_bit = 0x20000;
	  value <<= 2;
	}

      if (ok && !bfd_link_pic (info))
	{
	  addr = (input_section->output_section->vma
		  + input_section->output_offset
		  + relocation->r_offset
		  + 4);
	  dest = (addr
		  + (((value & ((sign_bit << 1) - 1)) ^ sign_bit) - sign_bit));

	  if ((addr >> 28) << 28 != (dest >> 28) << 28)
	    {
	      info->callbacks->einfo
		(_("%X%H: cannot convert branch between ISA modes "
		   "to JALX: relocation out of range\n"),
		 input_bfd, input_section, relocation->r_offset);
	      return true;
	    }

	  /* Make this the JALX opcode.  */
	  x = ((dest >> 2) & 0x3ffffff) | jalx_opcode << 26;
	}
      else if (!mips_elf_hash_table (info)->ignore_branch_isa)
	{
	  info->callbacks->einfo
	    (_("%X%H: unsupported branch between ISA modes\n"),
	     input_bfd, input_section, relocation->r_offset);
	  return true;
	}
    }

  /* Try converting JAL to BAL and J(AL)R to B(AL), if the target is in
     range.  */
  if (!bfd_link_relocatable (info)
      && !cross_mode_jump_p
      && ((JAL_TO_BAL_P (input_bfd)
	   && r_type == R_MIPS_26
	   && (x >> 26) == 0x3)			/* jal addr */
	  || (JALR_TO_BAL_P (input_bfd)
	      && r_type == R_MIPS_JALR
	      && x == 0x0320f809)		/* jalr t9 */
	  || (JR_TO_B_P (input_bfd)
	      && r_type == R_MIPS_JALR
	      && (x & ~1) == 0x03200008)))	/* jr t9 / jalr zero, t9 */
    {
      bfd_vma addr;
      bfd_vma dest;
      bfd_signed_vma off;

      addr = (input_section->output_section->vma
	      + input_section->output_offset
	      + relocation->r_offset
	      + 4);
      if (r_type == R_MIPS_26)
	dest = (value << 2) | ((addr >> 28) << 28);
      else
	dest = value;
      off = dest - addr;
      if (off <= 0x1ffff && off >= -0x20000)
	{
	  if ((x & ~1) == 0x03200008)		/* jr t9 / jalr zero, t9 */
	    x = 0x10000000 | (((bfd_vma) off >> 2) & 0xffff);   /* b addr */
	  else
	    x = 0x04110000 | (((bfd_vma) off >> 2) & 0xffff);   /* bal addr */
	}
    }

  /* Put the value into the output.  */
  mips_elf_store_contents (howto, relocation, input_bfd, contents, x);

  _bfd_mips_elf_reloc_shuffle (input_bfd, r_type, !bfd_link_relocatable (info),
			       location);

  return true;
}

/* Create a rel.dyn relocation for the dynamic linker to resolve.  REL
   is the original relocation, which is now being transformed into a
   dynamic relocation.  The ADDENDP is adjusted if necessary; the
   caller should store the result in place of the original addend.  */

static bool
mips_elf_create_dynamic_relocation (bfd *output_bfd,
				    struct bfd_link_info *info,
				    const Elf_Internal_Rela *rel,
				    struct mips_elf_link_hash_entry *h,
				    asection *sec, bfd_vma symbol,
				    bfd_vma *addendp, asection *input_section)
{
  Elf_Internal_Rela outrel[3];
  asection *sreloc;
  bfd *dynobj;
  int r_type;
  long indx;
  bool defined_p;
  struct mips_elf_link_hash_table *htab;

  htab = mips_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);

  r_type = ELF_R_TYPE (output_bfd, rel->r_info);
  dynobj = elf_hash_table (info)->dynobj;
  sreloc = mips_elf_rel_dyn_section (info, false);
  BFD_ASSERT (sreloc != NULL);
  BFD_ASSERT (sreloc->contents != NULL);
  BFD_ASSERT (sreloc->reloc_count * MIPS_ELF_REL_SIZE (output_bfd)
	      < sreloc->size);

  outrel[0].r_offset =
    _bfd_elf_section_offset (output_bfd, info, input_section, rel[0].r_offset);
  if (ABI_64_P (output_bfd))
    {
      outrel[1].r_offset =
	_bfd_elf_section_offset (output_bfd, info, input_section, rel[1].r_offset);
      outrel[2].r_offset =
	_bfd_elf_section_offset (output_bfd, info, input_section, rel[2].r_offset);
    }

  if (outrel[0].r_offset == MINUS_ONE)
    /* The relocation field has been deleted.  */
    return true;

  if (outrel[0].r_offset == MINUS_TWO)
    {
      /* The relocation field has been converted into a relative value of
	 some sort.  Functions like _bfd_elf_write_section_eh_frame expect
	 the field to be fully relocated, so add in the symbol's value.  */
      *addendp += symbol;
      return true;
    }

  /* We must now calculate the dynamic symbol table index to use
     in the relocation.  */
  if (h != NULL && ! SYMBOL_REFERENCES_LOCAL (info, &h->root))
    {
      BFD_ASSERT (htab->root.target_os == is_vxworks
		  || h->global_got_area != GGA_NONE);
      indx = h->root.dynindx;
      if (SGI_COMPAT (output_bfd))
	defined_p = h->root.def_regular;
      else
	/* ??? glibc's ld.so just adds the final GOT entry to the
	   relocation field.  It therefore treats relocs against
	   defined symbols in the same way as relocs against
	   undefined symbols.  */
	defined_p = false;
    }
  else
    {
      if (sec != NULL && bfd_is_abs_section (sec))
	indx = 0;
      else if (sec == NULL || sec->owner == NULL)
	{
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}
      else
	{
	  indx = elf_section_data (sec->output_section)->dynindx;
	  if (indx == 0)
	    {
	      asection *osec = htab->root.text_index_section;
	      indx = elf_section_data (osec)->dynindx;
	    }
	  if (indx == 0)
	    abort ();
	}

      /* Instead of generating a relocation using the section
	 symbol, we may as well make it a fully relative
	 relocation.  We want to avoid generating relocations to
	 local symbols because we used to generate them
	 incorrectly, without adding the original symbol value,
	 which is mandated by the ABI for section symbols.  In
	 order to give dynamic loaders and applications time to
	 phase out the incorrect use, we refrain from emitting
	 section-relative relocations.  It's not like they're
	 useful, after all.  This should be a bit more efficient
	 as well.  */
      /* ??? Although this behavior is compatible with glibc's ld.so,
	 the ABI says that relocations against STN_UNDEF should have
	 a symbol value of 0.  Irix rld honors this, so relocations
	 against STN_UNDEF have no effect.  */
      if (!SGI_COMPAT (output_bfd))
	indx = 0;
      defined_p = true;
    }

  /* If the relocation was previously an absolute relocation and
     this symbol will not be referred to by the relocation, we must
     adjust it by the value we give it in the dynamic symbol table.
     Otherwise leave the job up to the dynamic linker.  */
  if (defined_p && r_type != R_MIPS_REL32)
    *addendp += symbol;

  if (htab->root.target_os == is_vxworks)
    /* VxWorks uses non-relative relocations for this.  */
    outrel[0].r_info = ELF32_R_INFO (indx, R_MIPS_32);
  else
    /* The relocation is always an REL32 relocation because we don't
       know where the shared library will wind up at load-time.  */
    outrel[0].r_info = ELF_R_INFO (output_bfd, (unsigned long) indx,
				   R_MIPS_REL32);

  /* For strict adherence to the ABI specification, we should
     generate a R_MIPS_64 relocation record by itself before the
     _REL32/_64 record as well, such that the addend is read in as
     a 64-bit value (REL32 is a 32-bit relocation, after all).
     However, since none of the existing ELF64 MIPS dynamic
     loaders seems to care, we don't waste space with these
     artificial relocations.  If this turns out to not be true,
     mips_elf_allocate_dynamic_relocation() should be tweaked so
     as to make room for a pair of dynamic relocations per
     invocation if ABI_64_P, and here we should generate an
     additional relocation record with R_MIPS_64 by itself for a
     NULL symbol before this relocation record.  */
  outrel[1].r_info = ELF_R_INFO (output_bfd, 0,
				 ABI_64_P (output_bfd)
				 ? R_MIPS_64
				 : R_MIPS_NONE);
  outrel[2].r_info = ELF_R_INFO (output_bfd, 0, R_MIPS_NONE);

  /* Adjust the output offset of the relocation to reference the
     correct location in the output file.  */
  outrel[0].r_offset += (input_section->output_section->vma
			 + input_section->output_offset);
  outrel[1].r_offset += (input_section->output_section->vma
			 + input_section->output_offset);
  outrel[2].r_offset += (input_section->output_section->vma
			 + input_section->output_offset);

  /* Put the relocation back out.  We have to use the special
     relocation outputter in the 64-bit case since the 64-bit
     relocation format is non-standard.  */
  if (ABI_64_P (output_bfd))
    {
      (*get_elf_backend_data (output_bfd)->s->swap_reloc_out)
	(output_bfd, &outrel[0],
	 (sreloc->contents
	  + sreloc->reloc_count * sizeof (Elf64_Mips_External_Rel)));
    }
  else if (htab->root.target_os == is_vxworks)
    {
      /* VxWorks uses RELA rather than REL dynamic relocations.  */
      outrel[0].r_addend = *addendp;
      bfd_elf32_swap_reloca_out
	(output_bfd, &outrel[0],
	 (sreloc->contents
	  + sreloc->reloc_count * sizeof (Elf32_External_Rela)));
    }
  else
    bfd_elf32_swap_reloc_out
      (output_bfd, &outrel[0],
       (sreloc->contents + sreloc->reloc_count * sizeof (Elf32_External_Rel)));

  /* We've now added another relocation.  */
  ++sreloc->reloc_count;

  /* Make sure the output section is writable.  The dynamic linker
     will be writing to it.  */
  elf_section_data (input_section->output_section)->this_hdr.sh_flags
    |= SHF_WRITE;

  /* On IRIX5, make an entry of compact relocation info.  */
  if (IRIX_COMPAT (output_bfd) == ict_irix5)
    {
      asection *scpt = bfd_get_linker_section (dynobj, ".compact_rel");
      bfd_byte *cr;

      if (scpt)
	{
	  Elf32_crinfo cptrel;

	  mips_elf_set_cr_format (cptrel, CRF_MIPS_LONG);
	  cptrel.vaddr = (rel->r_offset
			  + input_section->output_section->vma
			  + input_section->output_offset);
	  if (r_type == R_MIPS_REL32)
	    mips_elf_set_cr_type (cptrel, CRT_MIPS_REL32);
	  else
	    mips_elf_set_cr_type (cptrel, CRT_MIPS_WORD);
	  mips_elf_set_cr_dist2to (cptrel, 0);
	  cptrel.konst = *addendp;

	  cr = (scpt->contents
		+ sizeof (Elf32_External_compact_rel));
	  mips_elf_set_cr_relvaddr (cptrel, 0);
	  bfd_elf32_swap_crinfo_out (output_bfd, &cptrel,
				     ((Elf32_External_crinfo *) cr
				      + scpt->reloc_count));
	  ++scpt->reloc_count;
	}
    }

  /* If we've written this relocation for a readonly section,
     we need to set DF_TEXTREL again, so that we do not delete the
     DT_TEXTREL tag.  */
  if (MIPS_ELF_READONLY_SECTION (input_section))
    info->flags |= DF_TEXTREL;

  return true;
}

/* Return the MACH for a MIPS e_flags value.  */

unsigned long
_bfd_elf_mips_mach (flagword flags)
{
  switch (flags & EF_MIPS_MACH)
    {
    case E_MIPS_MACH_3900:
      return bfd_mach_mips3900;

    case E_MIPS_MACH_4010:
      return bfd_mach_mips4010;

    case E_MIPS_MACH_ALLEGREX:
      return bfd_mach_mips_allegrex;

    case E_MIPS_MACH_4100:
      return bfd_mach_mips4100;

    case E_MIPS_MACH_4111:
      return bfd_mach_mips4111;

    case E_MIPS_MACH_4120:
      return bfd_mach_mips4120;

    case E_MIPS_MACH_4650:
      return bfd_mach_mips4650;

    case E_MIPS_MACH_5400:
      return bfd_mach_mips5400;

    case E_MIPS_MACH_5500:
      return bfd_mach_mips5500;

    case E_MIPS_MACH_5900:
      return bfd_mach_mips5900;

    case E_MIPS_MACH_9000:
      return bfd_mach_mips9000;

    case E_MIPS_MACH_SB1:
      return bfd_mach_mips_sb1;

    case E_MIPS_MACH_LS2E:
      return bfd_mach_mips_loongson_2e;

    case E_MIPS_MACH_LS2F:
      return bfd_mach_mips_loongson_2f;

    case E_MIPS_MACH_GS464:
      return bfd_mach_mips_gs464;

    case E_MIPS_MACH_GS464E:
      return bfd_mach_mips_gs464e;

    case E_MIPS_MACH_GS264E:
      return bfd_mach_mips_gs264e;

    case E_MIPS_MACH_OCTEON3:
      return bfd_mach_mips_octeon3;

    case E_MIPS_MACH_OCTEON2:
      return bfd_mach_mips_octeon2;

    case E_MIPS_MACH_OCTEON:
      return bfd_mach_mips_octeon;

    case E_MIPS_MACH_XLR:
      return bfd_mach_mips_xlr;

    case E_MIPS_MACH_IAMR2:
      return bfd_mach_mips_interaptiv_mr2;

    default:
      switch (flags & EF_MIPS_ARCH)
	{
	default:
	case E_MIPS_ARCH_1:
	  return bfd_mach_mips3000;

	case E_MIPS_ARCH_2:
	  return bfd_mach_mips6000;

	case E_MIPS_ARCH_3:
	  return bfd_mach_mips4000;

	case E_MIPS_ARCH_4:
	  return bfd_mach_mips8000;

	case E_MIPS_ARCH_5:
	  return bfd_mach_mips5;

	case E_MIPS_ARCH_32:
	  return bfd_mach_mipsisa32;

	case E_MIPS_ARCH_64:
	  return bfd_mach_mipsisa64;

	case E_MIPS_ARCH_32R2:
	  return bfd_mach_mipsisa32r2;

	case E_MIPS_ARCH_64R2:
	  return bfd_mach_mipsisa64r2;

	case E_MIPS_ARCH_32R6:
	  return bfd_mach_mipsisa32r6;

	case E_MIPS_ARCH_64R6:
	  return bfd_mach_mipsisa64r6;
	}
    }

  return 0;
}

/* Return printable name for ABI.  */

static inline char *
elf_mips_abi_name (bfd *abfd)
{
  flagword flags;

  flags = elf_elfheader (abfd)->e_flags;
  switch (flags & EF_MIPS_ABI)
    {
    case 0:
      if (ABI_N32_P (abfd))
	return "N32";
      else if (ABI_64_P (abfd))
	return "64";
      else
	return "none";
    case E_MIPS_ABI_O32:
      return "O32";
    case E_MIPS_ABI_O64:
      return "O64";
    case E_MIPS_ABI_EABI32:
      return "EABI32";
    case E_MIPS_ABI_EABI64:
      return "EABI64";
    default:
      return "unknown abi";
    }
}

/* MIPS ELF uses two common sections.  One is the usual one, and the
   other is for small objects.  All the small objects are kept
   together, and then referenced via the gp pointer, which yields
   faster assembler code.  This is what we use for the small common
   section.  This approach is copied from ecoff.c.  */
static asection mips_elf_scom_section;
static const asymbol mips_elf_scom_symbol =
  GLOBAL_SYM_INIT (".scommon", &mips_elf_scom_section);
static asection mips_elf_scom_section =
  BFD_FAKE_SECTION (mips_elf_scom_section, &mips_elf_scom_symbol,
		    ".scommon", 0, SEC_IS_COMMON | SEC_SMALL_DATA);

/* MIPS ELF also uses an acommon section, which represents an
   allocated common symbol which may be overridden by a
   definition in a shared library.  */
static asection mips_elf_acom_section;
static const asymbol mips_elf_acom_symbol =
  GLOBAL_SYM_INIT (".acommon", &mips_elf_acom_section);
static asection mips_elf_acom_section =
  BFD_FAKE_SECTION (mips_elf_acom_section, &mips_elf_acom_symbol,
		    ".acommon", 0, SEC_ALLOC);

/* This is used for both the 32-bit and the 64-bit ABI.  */

void
_bfd_mips_elf_symbol_processing (bfd *abfd, asymbol *asym)
{
  elf_symbol_type *elfsym;

  /* Handle the special MIPS section numbers that a symbol may use.  */
  elfsym = (elf_symbol_type *) asym;
  switch (elfsym->internal_elf_sym.st_shndx)
    {
    case SHN_MIPS_ACOMMON:
      /* This section is used in a dynamically linked executable file.
	 It is an allocated common section.  The dynamic linker can
	 either resolve these symbols to something in a shared
	 library, or it can just leave them here.  For our purposes,
	 we can consider these symbols to be in a new section.  */
      asym->section = &mips_elf_acom_section;
      break;

    case SHN_COMMON:
      /* Common symbols less than the GP size are automatically
	 treated as SHN_MIPS_SCOMMON symbols, with some exceptions.  */
      if (asym->value > elf_gp_size (abfd)
	  || ELF_ST_TYPE (elfsym->internal_elf_sym.st_info) == STT_TLS
	  || IRIX_COMPAT (abfd) == ict_irix6
	  || strcmp (asym->name, "__gnu_lto_slim") == 0)
	break;
      /* Fall through.  */
    case SHN_MIPS_SCOMMON:
      asym->section = &mips_elf_scom_section;
      asym->value = elfsym->internal_elf_sym.st_size;
      break;

    case SHN_MIPS_SUNDEFINED:
      asym->section = bfd_und_section_ptr;
      break;

    case SHN_MIPS_TEXT:
      {
	asection *section = bfd_get_section_by_name (abfd, ".text");

	if (section != NULL)
	  {
	    asym->section = section;
	    /* MIPS_TEXT is a bit special, the address is not an offset
	       to the base of the .text section.  So subtract the section
	       base address to make it an offset.  */
	    asym->value -= section->vma;
	  }
      }
      break;

    case SHN_MIPS_DATA:
      {
	asection *section = bfd_get_section_by_name (abfd, ".data");

	if (section != NULL)
	  {
	    asym->section = section;
	    /* MIPS_DATA is a bit special, the address is not an offset
	       to the base of the .data section.  So subtract the section
	       base address to make it an offset.  */
	    asym->value -= section->vma;
	  }
      }
      break;
    }

  /* If this is an odd-valued function symbol, assume it's a MIPS16
     or microMIPS one.  */
  if (ELF_ST_TYPE (elfsym->internal_elf_sym.st_info) == STT_FUNC
      && (asym->value & 1) != 0)
    {
      asym->value--;
      if (MICROMIPS_P (abfd))
	elfsym->internal_elf_sym.st_other
	  = ELF_ST_SET_MICROMIPS (elfsym->internal_elf_sym.st_other);
      else
	elfsym->internal_elf_sym.st_other
	  = ELF_ST_SET_MIPS16 (elfsym->internal_elf_sym.st_other);
    }
}

/* Implement elf_backend_eh_frame_address_size.  This differs from
   the default in the way it handles EABI64.

   EABI64 was originally specified as an LP64 ABI, and that is what
   -mabi=eabi normally gives on a 64-bit target.  However, gcc has
   historically accepted the combination of -mabi=eabi and -mlong32,
   and this ILP32 variation has become semi-official over time.
   Both forms use elf32 and have pointer-sized FDE addresses.

   If an EABI object was generated by GCC 4.0 or above, it will have
   an empty .gcc_compiled_longXX section, where XX is the size of longs
   in bits.  Unfortunately, ILP32 objects generated by earlier compilers
   have no special marking to distinguish them from LP64 objects.

   We don't want users of the official LP64 ABI to be punished for the
   existence of the ILP32 variant, but at the same time, we don't want
   to mistakenly interpret pre-4.0 ILP32 objects as being LP64 objects.
   We therefore take the following approach:

      - If ABFD contains a .gcc_compiled_longXX section, use it to
	determine the pointer size.

      - Otherwise check the type of the first relocation.  Assume that
	the LP64 ABI is being used if the relocation is of type R_MIPS_64.

      - Otherwise punt.

   The second check is enough to detect LP64 objects generated by pre-4.0
   compilers because, in the kind of output generated by those compilers,
   the first relocation will be associated with either a CIE personality
   routine or an FDE start address.  Furthermore, the compilers never
   used a special (non-pointer) encoding for this ABI.

   Checking the relocation type should also be safe because there is no
   reason to use R_MIPS_64 in an ILP32 object.  Pre-4.0 compilers never
   did so.  */

unsigned int
_bfd_mips_elf_eh_frame_address_size (bfd *abfd, const asection *sec)
{
  if (elf_elfheader (abfd)->e_ident[EI_CLASS] == ELFCLASS64)
    return 8;
  if ((elf_elfheader (abfd)->e_flags & EF_MIPS_ABI) == E_MIPS_ABI_EABI64)
    {
      bool long32_p, long64_p;

      long32_p = bfd_get_section_by_name (abfd, ".gcc_compiled_long32") != 0;
      long64_p = bfd_get_section_by_name (abfd, ".gcc_compiled_long64") != 0;
      if (long32_p && long64_p)
	return 0;
      if (long32_p)
	return 4;
      if (long64_p)
	return 8;

      if (sec->reloc_count > 0
	  && elf_section_data (sec)->relocs != NULL
	  && (ELF32_R_TYPE (elf_section_data (sec)->relocs[0].r_info)
	      == R_MIPS_64))
	return 8;

      return 0;
    }
  return 4;
}

/* There appears to be a bug in the MIPSpro linker that causes GOT_DISP
   relocations against two unnamed section symbols to resolve to the
   same address.  For example, if we have code like:

	lw	$4,%got_disp(.data)($gp)
	lw	$25,%got_disp(.text)($gp)
	jalr	$25

   then the linker will resolve both relocations to .data and the program
   will jump there rather than to .text.

   We can work around this problem by giving names to local section symbols.
   This is also what the MIPSpro tools do.  */

bool
_bfd_mips_elf_name_local_section_symbols (bfd *abfd)
{
  return elf_elfheader (abfd)->e_type == ET_REL && SGI_COMPAT (abfd);
}

/* Work over a section just before writing it out.  This routine is
   used by both the 32-bit and the 64-bit ABI.  FIXME: We recognize
   sections that need the SHF_MIPS_GPREL flag by name; there has to be
   a better way.  */

bool
_bfd_mips_elf_section_processing (bfd *abfd, Elf_Internal_Shdr *hdr)
{
  if (hdr->sh_type == SHT_MIPS_REGINFO
      && hdr->sh_size > 0)
    {
      bfd_byte buf[4];

      BFD_ASSERT (hdr->contents == NULL);

      if (hdr->sh_size != sizeof (Elf32_External_RegInfo))
	{
	  _bfd_error_handler
	    (_("%pB: incorrect `.reginfo' section size; "
	       "expected %" PRIu64 ", got %" PRIu64),
	     abfd, (uint64_t) sizeof (Elf32_External_RegInfo),
	     (uint64_t) hdr->sh_size);
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}

      if (bfd_seek (abfd,
		    hdr->sh_offset + sizeof (Elf32_External_RegInfo) - 4,
		    SEEK_SET) != 0)
	return false;
      H_PUT_32 (abfd, elf_gp (abfd), buf);
      if (bfd_bwrite (buf, 4, abfd) != 4)
	return false;
    }

  if (hdr->sh_type == SHT_MIPS_OPTIONS
      && hdr->bfd_section != NULL
      && mips_elf_section_data (hdr->bfd_section) != NULL
      && mips_elf_section_data (hdr->bfd_section)->u.tdata != NULL)
    {
      bfd_byte *contents, *l, *lend;

      /* We stored the section contents in the tdata field in the
	 set_section_contents routine.  We save the section contents
	 so that we don't have to read them again.
	 At this point we know that elf_gp is set, so we can look
	 through the section contents to see if there is an
	 ODK_REGINFO structure.  */

      contents = mips_elf_section_data (hdr->bfd_section)->u.tdata;
      l = contents;
      lend = contents + hdr->sh_size;
      while (l + sizeof (Elf_External_Options) <= lend)
	{
	  Elf_Internal_Options intopt;

	  bfd_mips_elf_swap_options_in (abfd, (Elf_External_Options *) l,
					&intopt);
	  if (intopt.size < sizeof (Elf_External_Options))
	    {
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB: warning: bad `%s' option size %u smaller than"
		   " its header"),
		abfd, MIPS_ELF_OPTIONS_SECTION_NAME (abfd), intopt.size);
	      break;
	    }
	  if (ABI_64_P (abfd) && intopt.kind == ODK_REGINFO)
	    {
	      bfd_byte buf[8];

	      if (bfd_seek (abfd,
			    (hdr->sh_offset
			     + (l - contents)
			     + sizeof (Elf_External_Options)
			     + (sizeof (Elf64_External_RegInfo) - 8)),
			     SEEK_SET) != 0)
		return false;
	      H_PUT_64 (abfd, elf_gp (abfd), buf);
	      if (bfd_bwrite (buf, 8, abfd) != 8)
		return false;
	    }
	  else if (intopt.kind == ODK_REGINFO)
	    {
	      bfd_byte buf[4];

	      if (bfd_seek (abfd,
			    (hdr->sh_offset
			     + (l - contents)
			     + sizeof (Elf_External_Options)
			     + (sizeof (Elf32_External_RegInfo) - 4)),
			    SEEK_SET) != 0)
		return false;
	      H_PUT_32 (abfd, elf_gp (abfd), buf);
	      if (bfd_bwrite (buf, 4, abfd) != 4)
		return false;
	    }
	  l += intopt.size;
	}
    }

  if (hdr->bfd_section != NULL)
    {
      const char *name = bfd_section_name (hdr->bfd_section);

      /* .sbss is not handled specially here because the GNU/Linux
	 prelinker can convert .sbss from NOBITS to PROGBITS and
	 changing it back to NOBITS breaks the binary.  The entry in
	 _bfd_mips_elf_special_sections will ensure the correct flags
	 are set on .sbss if BFD creates it without reading it from an
	 input file, and without special handling here the flags set
	 on it in an input file will be followed.  */
      if (strcmp (name, ".sdata") == 0
	  || strcmp (name, ".lit8") == 0
	  || strcmp (name, ".lit4") == 0)
	hdr->sh_flags |= SHF_ALLOC | SHF_WRITE | SHF_MIPS_GPREL;
      else if (strcmp (name, ".srdata") == 0)
	hdr->sh_flags |= SHF_ALLOC | SHF_MIPS_GPREL;
      else if (strcmp (name, ".compact_rel") == 0)
	hdr->sh_flags = 0;
      else if (strcmp (name, ".rtproc") == 0)
	{
	  if (hdr->sh_addralign != 0 && hdr->sh_entsize == 0)
	    {
	      unsigned int adjust;

	      adjust = hdr->sh_size % hdr->sh_addralign;
	      if (adjust != 0)
		hdr->sh_size += hdr->sh_addralign - adjust;
	    }
	}
    }

  return true;
}

/* Handle a MIPS specific section when reading an object file.  This
   is called when elfcode.h finds a section with an unknown type.
   This routine supports both the 32-bit and 64-bit ELF ABI.  */

bool
_bfd_mips_elf_section_from_shdr (bfd *abfd,
				 Elf_Internal_Shdr *hdr,
				 const char *name,
				 int shindex)
{
  flagword flags = 0;

  /* There ought to be a place to keep ELF backend specific flags, but
     at the moment there isn't one.  We just keep track of the
     sections by their name, instead.  Fortunately, the ABI gives
     suggested names for all the MIPS specific sections, so we will
     probably get away with this.  */
  switch (hdr->sh_type)
    {
    case SHT_MIPS_LIBLIST:
      if (strcmp (name, ".liblist") != 0)
	return false;
      break;
    case SHT_MIPS_MSYM:
      if (strcmp (name, ".msym") != 0)
	return false;
      break;
    case SHT_MIPS_CONFLICT:
      if (strcmp (name, ".conflict") != 0)
	return false;
      break;
    case SHT_MIPS_GPTAB:
      if (! startswith (name, ".gptab."))
	return false;
      break;
    case SHT_MIPS_UCODE:
      if (strcmp (name, ".ucode") != 0)
	return false;
      break;
    case SHT_MIPS_DEBUG:
      if (strcmp (name, ".mdebug") != 0)
	return false;
      flags = SEC_DEBUGGING;
      break;
    case SHT_MIPS_REGINFO:
      if (strcmp (name, ".reginfo") != 0
	  || hdr->sh_size != sizeof (Elf32_External_RegInfo))
	return false;
      flags = (SEC_LINK_ONCE | SEC_LINK_DUPLICATES_SAME_SIZE);
      break;
    case SHT_MIPS_IFACE:
      if (strcmp (name, ".MIPS.interfaces") != 0)
	return false;
      break;
    case SHT_MIPS_CONTENT:
      if (! startswith (name, ".MIPS.content"))
	return false;
      break;
    case SHT_MIPS_OPTIONS:
      if (!MIPS_ELF_OPTIONS_SECTION_NAME_P (name))
	return false;
      break;
    case SHT_MIPS_ABIFLAGS:
      if (!MIPS_ELF_ABIFLAGS_SECTION_NAME_P (name))
	return false;
      flags = (SEC_LINK_ONCE | SEC_LINK_DUPLICATES_SAME_SIZE);
      break;
    case SHT_MIPS_DWARF:
      if (! startswith (name, ".debug_")
         && ! startswith (name, ".gnu.debuglto_.debug_")
         && ! startswith (name, ".zdebug_")
         && ! startswith (name, ".gnu.debuglto_.zdebug_"))
	return false;
      break;
    case SHT_MIPS_SYMBOL_LIB:
      if (strcmp (name, ".MIPS.symlib") != 0)
	return false;
      break;
    case SHT_MIPS_EVENTS:
      if (! startswith (name, ".MIPS.events")
	  && ! startswith (name, ".MIPS.post_rel"))
	return false;
      break;
    case SHT_MIPS_XHASH:
      if (strcmp (name, ".MIPS.xhash") != 0)
	return false;
    default:
      break;
    }

  if (! _bfd_elf_make_section_from_shdr (abfd, hdr, name, shindex))
    return false;

  if (hdr->sh_flags & SHF_MIPS_GPREL)
    flags |= SEC_SMALL_DATA;

  if (flags)
    {
      if (!bfd_set_section_flags (hdr->bfd_section,
				  (bfd_section_flags (hdr->bfd_section)
				   | flags)))
	return false;
    }

  if (hdr->sh_type == SHT_MIPS_ABIFLAGS)
    {
      Elf_External_ABIFlags_v0 ext;

      if (! bfd_get_section_contents (abfd, hdr->bfd_section,
				      &ext, 0, sizeof ext))
	return false;
      bfd_mips_elf_swap_abiflags_v0_in (abfd, &ext,
					&mips_elf_tdata (abfd)->abiflags);
      if (mips_elf_tdata (abfd)->abiflags.version != 0)
	return false;
      mips_elf_tdata (abfd)->abiflags_valid = true;
    }

  /* FIXME: We should record sh_info for a .gptab section.  */

  /* For a .reginfo section, set the gp value in the tdata information
     from the contents of this section.  We need the gp value while
     processing relocs, so we just get it now.  The .reginfo section
     is not used in the 64-bit MIPS ELF ABI.  */
  if (hdr->sh_type == SHT_MIPS_REGINFO)
    {
      Elf32_External_RegInfo ext;
      Elf32_RegInfo s;

      if (! bfd_get_section_contents (abfd, hdr->bfd_section,
				      &ext, 0, sizeof ext))
	return false;
      bfd_mips_elf32_swap_reginfo_in (abfd, &ext, &s);
      elf_gp (abfd) = s.ri_gp_value;
    }

  /* For a SHT_MIPS_OPTIONS section, look for a ODK_REGINFO entry, and
     set the gp value based on what we find.  We may see both
     SHT_MIPS_REGINFO and SHT_MIPS_OPTIONS/ODK_REGINFO; in that case,
     they should agree.  */
  if (hdr->sh_type == SHT_MIPS_OPTIONS)
    {
      bfd_byte *contents, *l, *lend;

      if (!bfd_malloc_and_get_section (abfd, hdr->bfd_section, &contents))
	{
	  free (contents);
	  return false;
	}
      l = contents;
      lend = contents + hdr->sh_size;
      while (l + sizeof (Elf_External_Options) <= lend)
	{
	  Elf_Internal_Options intopt;

	  bfd_mips_elf_swap_options_in (abfd, (Elf_External_Options *) l,
					&intopt);
	  if (intopt.size < sizeof (Elf_External_Options))
	    {
	    bad_opt:
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB: warning: truncated `%s' option"),
		 abfd, MIPS_ELF_OPTIONS_SECTION_NAME (abfd));
	      break;
	    }
	  if (intopt.kind == ODK_REGINFO)
	    {
	      if (ABI_64_P (abfd))
		{
		  Elf64_Internal_RegInfo intreg;
		  size_t needed = (sizeof (Elf_External_Options)
				   + sizeof (Elf64_External_RegInfo));
		  if (intopt.size < needed || (size_t) (lend - l) < needed)
		    goto bad_opt;
		  bfd_mips_elf64_swap_reginfo_in
		    (abfd,
		     ((Elf64_External_RegInfo *)
		      (l + sizeof (Elf_External_Options))),
		     &intreg);
		  elf_gp (abfd) = intreg.ri_gp_value;
		}
	      else
		{
		  Elf32_RegInfo intreg;
		  size_t needed = (sizeof (Elf_External_Options)
				   + sizeof (Elf32_External_RegInfo));
		  if (intopt.size < needed || (size_t) (lend - l) < needed)
		    goto bad_opt;
		  bfd_mips_elf32_swap_reginfo_in
		    (abfd,
		     ((Elf32_External_RegInfo *)
		      (l + sizeof (Elf_External_Options))),
		     &intreg);
		  elf_gp (abfd) = intreg.ri_gp_value;
		}
	    }
	  l += intopt.size;
	}
      free (contents);
    }

  return true;
}

/* Set the correct type for a MIPS ELF section.  We do this by the
   section name, which is a hack, but ought to work.  This routine is
   used by both the 32-bit and the 64-bit ABI.  */

bool
_bfd_mips_elf_fake_sections (bfd *abfd, Elf_Internal_Shdr *hdr, asection *sec)
{
  const char *name = bfd_section_name (sec);

  if (strcmp (name, ".liblist") == 0)
    {
      hdr->sh_type = SHT_MIPS_LIBLIST;
      hdr->sh_info = sec->size / sizeof (Elf32_Lib);
      /* The sh_link field is set in final_write_processing.  */
    }
  else if (strcmp (name, ".conflict") == 0)
    hdr->sh_type = SHT_MIPS_CONFLICT;
  else if (startswith (name, ".gptab."))
    {
      hdr->sh_type = SHT_MIPS_GPTAB;
      hdr->sh_entsize = sizeof (Elf32_External_gptab);
      /* The sh_info field is set in final_write_processing.  */
    }
  else if (strcmp (name, ".ucode") == 0)
    hdr->sh_type = SHT_MIPS_UCODE;
  else if (strcmp (name, ".mdebug") == 0)
    {
      hdr->sh_type = SHT_MIPS_DEBUG;
      /* In a shared object on IRIX 5.3, the .mdebug section has an
	 entsize of 0.  FIXME: Does this matter?  */
      if (SGI_COMPAT (abfd) && (abfd->flags & DYNAMIC) != 0)
	hdr->sh_entsize = 0;
      else
	hdr->sh_entsize = 1;
    }
  else if (strcmp (name, ".reginfo") == 0)
    {
      hdr->sh_type = SHT_MIPS_REGINFO;
      /* In a shared object on IRIX 5.3, the .reginfo section has an
	 entsize of 0x18.  FIXME: Does this matter?  */
      if (SGI_COMPAT (abfd))
	{
	  if ((abfd->flags & DYNAMIC) != 0)
	    hdr->sh_entsize = sizeof (Elf32_External_RegInfo);
	  else
	    hdr->sh_entsize = 1;
	}
      else
	hdr->sh_entsize = sizeof (Elf32_External_RegInfo);
    }
  else if (SGI_COMPAT (abfd)
	   && (strcmp (name, ".hash") == 0
	       || strcmp (name, ".dynamic") == 0
	       || strcmp (name, ".dynstr") == 0))
    {
      if (SGI_COMPAT (abfd))
	hdr->sh_entsize = 0;
#if 0
      /* This isn't how the IRIX6 linker behaves.  */
      hdr->sh_info = SIZEOF_MIPS_DYNSYM_SECNAMES;
#endif
    }
  else if (strcmp (name, ".got") == 0
	   || strcmp (name, ".srdata") == 0
	   || strcmp (name, ".sdata") == 0
	   || strcmp (name, ".sbss") == 0
	   || strcmp (name, ".lit4") == 0
	   || strcmp (name, ".lit8") == 0)
    hdr->sh_flags |= SHF_MIPS_GPREL;
  else if (strcmp (name, ".MIPS.interfaces") == 0)
    {
      hdr->sh_type = SHT_MIPS_IFACE;
      hdr->sh_flags |= SHF_MIPS_NOSTRIP;
    }
  else if (startswith (name, ".MIPS.content"))
    {
      hdr->sh_type = SHT_MIPS_CONTENT;
      hdr->sh_flags |= SHF_MIPS_NOSTRIP;
      /* The sh_info field is set in final_write_processing.  */
    }
  else if (MIPS_ELF_OPTIONS_SECTION_NAME_P (name))
    {
      hdr->sh_type = SHT_MIPS_OPTIONS;
      hdr->sh_entsize = 1;
      hdr->sh_flags |= SHF_MIPS_NOSTRIP;
    }
  else if (startswith (name, ".MIPS.abiflags"))
    {
      hdr->sh_type = SHT_MIPS_ABIFLAGS;
      hdr->sh_entsize = sizeof (Elf_External_ABIFlags_v0);
    }
  else if (startswith (name, ".debug_")
	   || startswith (name, ".gnu.debuglto_.debug_")
	   || startswith (name, ".zdebug_")
	   || startswith (name, ".gnu.debuglto_.zdebug_"))
    {
      hdr->sh_type = SHT_MIPS_DWARF;

      /* Irix facilities such as libexc expect a single .debug_frame
	 per executable, the system ones have NOSTRIP set and the linker
	 doesn't merge sections with different flags so ...  */
      if (SGI_COMPAT (abfd) && startswith (name, ".debug_frame"))
	hdr->sh_flags |= SHF_MIPS_NOSTRIP;
    }
  else if (strcmp (name, ".MIPS.symlib") == 0)
    {
      hdr->sh_type = SHT_MIPS_SYMBOL_LIB;
      /* The sh_link and sh_info fields are set in
	 final_write_processing.  */
    }
  else if (startswith (name, ".MIPS.events")
	   || startswith (name, ".MIPS.post_rel"))
    {
      hdr->sh_type = SHT_MIPS_EVENTS;
      hdr->sh_flags |= SHF_MIPS_NOSTRIP;
      /* The sh_link field is set in final_write_processing.  */
    }
  else if (strcmp (name, ".msym") == 0)
    {
      hdr->sh_type = SHT_MIPS_MSYM;
      hdr->sh_flags |= SHF_ALLOC;
      hdr->sh_entsize = 8;
    }
  else if (strcmp (name, ".MIPS.xhash") == 0)
    {
      hdr->sh_type = SHT_MIPS_XHASH;
      hdr->sh_flags |= SHF_ALLOC;
      hdr->sh_entsize = get_elf_backend_data(abfd)->s->arch_size == 64 ? 0 : 4;
    }

  /* The generic elf_fake_sections will set up REL_HDR using the default
   kind of relocations.  We used to set up a second header for the
   non-default kind of relocations here, but only NewABI would use
   these, and the IRIX ld doesn't like resulting empty RELA sections.
   Thus we create those header only on demand now.  */

  return true;
}

/* Given a BFD section, try to locate the corresponding ELF section
   index.  This is used by both the 32-bit and the 64-bit ABI.
   Actually, it's not clear to me that the 64-bit ABI supports these,
   but for non-PIC objects we will certainly want support for at least
   the .scommon section.  */

bool
_bfd_mips_elf_section_from_bfd_section (bfd *abfd ATTRIBUTE_UNUSED,
					asection *sec, int *retval)
{
  if (strcmp (bfd_section_name (sec), ".scommon") == 0)
    {
      *retval = SHN_MIPS_SCOMMON;
      return true;
    }
  if (strcmp (bfd_section_name (sec), ".acommon") == 0)
    {
      *retval = SHN_MIPS_ACOMMON;
      return true;
    }
  return false;
}

/* Hook called by the linker routine which adds symbols from an object
   file.  We must handle the special MIPS section numbers here.  */

bool
_bfd_mips_elf_add_symbol_hook (bfd *abfd, struct bfd_link_info *info,
			       Elf_Internal_Sym *sym, const char **namep,
			       flagword *flagsp ATTRIBUTE_UNUSED,
			       asection **secp, bfd_vma *valp)
{
  if (SGI_COMPAT (abfd)
      && (abfd->flags & DYNAMIC) != 0
      && strcmp (*namep, "_rld_new_interface") == 0)
    {
      /* Skip IRIX5 rld entry name.  */
      *namep = NULL;
      return true;
    }

  /* Shared objects may have a dynamic symbol '_gp_disp' defined as
     a SECTION *ABS*.  This causes ld to think it can resolve _gp_disp
     by setting a DT_NEEDED for the shared object.  Since _gp_disp is
     a magic symbol resolved by the linker, we ignore this bogus definition
     of _gp_disp.  New ABI objects do not suffer from this problem so this
     is not done for them. */
  if (!NEWABI_P(abfd)
      && (sym->st_shndx == SHN_ABS)
      && (strcmp (*namep, "_gp_disp") == 0))
    {
      *namep = NULL;
      return true;
    }

  switch (sym->st_shndx)
    {
    case SHN_COMMON:
      /* Common symbols less than the GP size are automatically
	 treated as SHN_MIPS_SCOMMON symbols, with some exceptions.  */
      if (sym->st_size > elf_gp_size (abfd)
	  || ELF_ST_TYPE (sym->st_info) == STT_TLS
	  || IRIX_COMPAT (abfd) == ict_irix6
	  || strcmp (*namep, "__gnu_lto_slim") == 0)
	break;
      /* Fall through.  */
    case SHN_MIPS_SCOMMON:
      *secp = bfd_make_section_old_way (abfd, ".scommon");
      (*secp)->flags |= SEC_IS_COMMON | SEC_SMALL_DATA;
      *valp = sym->st_size;
      break;

    case SHN_MIPS_TEXT:
      /* This section is used in a shared object.  */
      if (mips_elf_tdata (abfd)->elf_text_section == NULL)
	{
	  asymbol *elf_text_symbol;
	  asection *elf_text_section;
	  size_t amt = sizeof (asection);

	  elf_text_section = bfd_zalloc (abfd, amt);
	  if (elf_text_section == NULL)
	    return false;

	  amt = sizeof (asymbol);
	  elf_text_symbol = bfd_zalloc (abfd, amt);
	  if (elf_text_symbol == NULL)
	    return false;

	  /* Initialize the section.  */

	  mips_elf_tdata (abfd)->elf_text_section = elf_text_section;
	  mips_elf_tdata (abfd)->elf_text_symbol = elf_text_symbol;

	  elf_text_section->symbol = elf_text_symbol;
	  elf_text_section->symbol_ptr_ptr = &mips_elf_tdata (abfd)->elf_text_symbol;

	  elf_text_section->name = ".text";
	  elf_text_section->flags = SEC_NO_FLAGS;
	  elf_text_section->output_section = NULL;
	  elf_text_section->owner = abfd;
	  elf_text_symbol->name = ".text";
	  elf_text_symbol->flags = BSF_SECTION_SYM | BSF_DYNAMIC;
	  elf_text_symbol->section = elf_text_section;
	}
      /* This code used to do *secp = bfd_und_section_ptr if
	 bfd_link_pic (info).  I don't know why, and that doesn't make sense,
	 so I took it out.  */
      *secp = mips_elf_tdata (abfd)->elf_text_section;
      break;

    case SHN_MIPS_ACOMMON:
      /* Fall through. XXX Can we treat this as allocated data?  */
    case SHN_MIPS_DATA:
      /* This section is used in a shared object.  */
      if (mips_elf_tdata (abfd)->elf_data_section == NULL)
	{
	  asymbol *elf_data_symbol;
	  asection *elf_data_section;
	  size_t amt = sizeof (asection);

	  elf_data_section = bfd_zalloc (abfd, amt);
	  if (elf_data_section == NULL)
	    return false;

	  amt = sizeof (asymbol);
	  elf_data_symbol = bfd_zalloc (abfd, amt);
	  if (elf_data_symbol == NULL)
	    return false;

	  /* Initialize the section.  */

	  mips_elf_tdata (abfd)->elf_data_section = elf_data_section;
	  mips_elf_tdata (abfd)->elf_data_symbol = elf_data_symbol;

	  elf_data_section->symbol = elf_data_symbol;
	  elf_data_section->symbol_ptr_ptr = &mips_elf_tdata (abfd)->elf_data_symbol;

	  elf_data_section->name = ".data";
	  elf_data_section->flags = SEC_NO_FLAGS;
	  elf_data_section->output_section = NULL;
	  elf_data_section->owner = abfd;
	  elf_data_symbol->name = ".data";
	  elf_data_symbol->flags = BSF_SECTION_SYM | BSF_DYNAMIC;
	  elf_data_symbol->section = elf_data_section;
	}
      /* This code used to do *secp = bfd_und_section_ptr if
	 bfd_link_pic (info).  I don't know why, and that doesn't make sense,
	 so I took it out.  */
      *secp = mips_elf_tdata (abfd)->elf_data_section;
      break;

    case SHN_MIPS_SUNDEFINED:
      *secp = bfd_und_section_ptr;
      break;
    }

  if (SGI_COMPAT (abfd)
      && ! bfd_link_pic (info)
      && info->output_bfd->xvec == abfd->xvec
      && strcmp (*namep, "__rld_obj_head") == 0)
    {
      struct elf_link_hash_entry *h;
      struct bfd_link_hash_entry *bh;

      /* Mark __rld_obj_head as dynamic.  */
      bh = NULL;
      if (! (_bfd_generic_link_add_one_symbol
	     (info, abfd, *namep, BSF_GLOBAL, *secp, *valp, NULL, false,
	      get_elf_backend_data (abfd)->collect, &bh)))
	return false;

      h = (struct elf_link_hash_entry *) bh;
      h->non_elf = 0;
      h->def_regular = 1;
      h->type = STT_OBJECT;

      if (! bfd_elf_link_record_dynamic_symbol (info, h))
	return false;

      mips_elf_hash_table (info)->use_rld_obj_head = true;
      mips_elf_hash_table (info)->rld_symbol = h;
    }

  /* If this is a mips16 text symbol, add 1 to the value to make it
     odd.  This will cause something like .word SYM to come up with
     the right value when it is loaded into the PC.  */
  if (ELF_ST_IS_COMPRESSED (sym->st_other))
    ++*valp;

  return true;
}

/* This hook function is called before the linker writes out a global
   symbol.  We mark symbols as small common if appropriate.  This is
   also where we undo the increment of the value for a mips16 symbol.  */

int
_bfd_mips_elf_link_output_symbol_hook
  (struct bfd_link_info *info ATTRIBUTE_UNUSED,
   const char *name ATTRIBUTE_UNUSED, Elf_Internal_Sym *sym,
   asection *input_sec, struct elf_link_hash_entry *h ATTRIBUTE_UNUSED)
{
  /* If we see a common symbol, which implies a relocatable link, then
     if a symbol was small common in an input file, mark it as small
     common in the output file.  */
  if (sym->st_shndx == SHN_COMMON
      && strcmp (input_sec->name, ".scommon") == 0)
    sym->st_shndx = SHN_MIPS_SCOMMON;

  if (ELF_ST_IS_COMPRESSED (sym->st_other))
    sym->st_value &= ~1;

  return 1;
}

/* Functions for the dynamic linker.  */

/* Create dynamic sections when linking against a dynamic object.  */

bool
_bfd_mips_elf_create_dynamic_sections (bfd *abfd, struct bfd_link_info *info)
{
  struct elf_link_hash_entry *h;
  struct bfd_link_hash_entry *bh;
  flagword flags;
  register asection *s;
  const char * const *namep;
  struct mips_elf_link_hash_table *htab;

  htab = mips_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);

  flags = (SEC_ALLOC | SEC_LOAD | SEC_HAS_CONTENTS | SEC_IN_MEMORY
	   | SEC_LINKER_CREATED | SEC_READONLY);

  /* The psABI requires a read-only .dynamic section, but the VxWorks
     EABI doesn't.  */
  if (htab->root.target_os != is_vxworks)
    {
      s = bfd_get_linker_section (abfd, ".dynamic");
      if (s != NULL)
	{
	  if (!bfd_set_section_flags (s, flags))
	    return false;
	}
    }

  /* We need to create .got section.  */
  if (!mips_elf_create_got_section (abfd, info))
    return false;

  if (! mips_elf_rel_dyn_section (info, true))
    return false;

  /* Create .stub section.  */
  s = bfd_make_section_anyway_with_flags (abfd,
					  MIPS_ELF_STUB_SECTION_NAME (abfd),
					  flags | SEC_CODE);
  if (s == NULL
      || !bfd_set_section_alignment (s, MIPS_ELF_LOG_FILE_ALIGN (abfd)))
    return false;
  htab->sstubs = s;

  if (!mips_elf_hash_table (info)->use_rld_obj_head
      && bfd_link_executable (info)
      && bfd_get_linker_section (abfd, ".rld_map") == NULL)
    {
      s = bfd_make_section_anyway_with_flags (abfd, ".rld_map",
					      flags &~ (flagword) SEC_READONLY);
      if (s == NULL
	  || !bfd_set_section_alignment (s, MIPS_ELF_LOG_FILE_ALIGN (abfd)))
	return false;
    }

  /* Create .MIPS.xhash section.  */
  if (info->emit_gnu_hash)
    s = bfd_make_section_anyway_with_flags (abfd, ".MIPS.xhash",
					    flags | SEC_READONLY);

  /* On IRIX5, we adjust add some additional symbols and change the
     alignments of several sections.  There is no ABI documentation
     indicating that this is necessary on IRIX6, nor any evidence that
     the linker takes such action.  */
  if (IRIX_COMPAT (abfd) == ict_irix5)
    {
      for (namep = mips_elf_dynsym_rtproc_names; *namep != NULL; namep++)
	{
	  bh = NULL;
	  if (! (_bfd_generic_link_add_one_symbol
		 (info, abfd, *namep, BSF_GLOBAL, bfd_und_section_ptr, 0,
		  NULL, false, get_elf_backend_data (abfd)->collect, &bh)))
	    return false;

	  h = (struct elf_link_hash_entry *) bh;
	  h->mark = 1;
	  h->non_elf = 0;
	  h->def_regular = 1;
	  h->type = STT_SECTION;

	  if (! bfd_elf_link_record_dynamic_symbol (info, h))
	    return false;
	}

      /* We need to create a .compact_rel section.  */
      if (SGI_COMPAT (abfd))
	{
	  if (!mips_elf_create_compact_rel_section (abfd, info))
	    return false;
	}

      /* Change alignments of some sections.  */
      s = bfd_get_linker_section (abfd, ".hash");
      if (s != NULL)
	bfd_set_section_alignment (s, MIPS_ELF_LOG_FILE_ALIGN (abfd));

      s = bfd_get_linker_section (abfd, ".dynsym");
      if (s != NULL)
	bfd_set_section_alignment (s, MIPS_ELF_LOG_FILE_ALIGN (abfd));

      s = bfd_get_linker_section (abfd, ".dynstr");
      if (s != NULL)
	bfd_set_section_alignment (s, MIPS_ELF_LOG_FILE_ALIGN (abfd));

      /* ??? */
      s = bfd_get_section_by_name (abfd, ".reginfo");
      if (s != NULL)
	bfd_set_section_alignment (s, MIPS_ELF_LOG_FILE_ALIGN (abfd));

      s = bfd_get_linker_section (abfd, ".dynamic");
      if (s != NULL)
	bfd_set_section_alignment (s, MIPS_ELF_LOG_FILE_ALIGN (abfd));
    }

  if (bfd_link_executable (info))
    {
      const char *name;

      name = SGI_COMPAT (abfd) ? "_DYNAMIC_LINK" : "_DYNAMIC_LINKING";
      bh = NULL;
      if (!(_bfd_generic_link_add_one_symbol
	    (info, abfd, name, BSF_GLOBAL, bfd_abs_section_ptr, 0,
	     NULL, false, get_elf_backend_data (abfd)->collect, &bh)))
	return false;

      h = (struct elf_link_hash_entry *) bh;
      h->non_elf = 0;
      h->def_regular = 1;
      h->type = STT_SECTION;

      if (! bfd_elf_link_record_dynamic_symbol (info, h))
	return false;

      if (! mips_elf_hash_table (info)->use_rld_obj_head)
	{
	  /* __rld_map is a four byte word located in the .data section
	     and is filled in by the rtld to contain a pointer to
	     the _r_debug structure. Its symbol value will be set in
	     _bfd_mips_elf_finish_dynamic_symbol.  */
	  s = bfd_get_linker_section (abfd, ".rld_map");
	  BFD_ASSERT (s != NULL);

	  name = SGI_COMPAT (abfd) ? "__rld_map" : "__RLD_MAP";
	  bh = NULL;
	  if (!(_bfd_generic_link_add_one_symbol
		(info, abfd, name, BSF_GLOBAL, s, 0, NULL, false,
		 get_elf_backend_data (abfd)->collect, &bh)))
	    return false;

	  h = (struct elf_link_hash_entry *) bh;
	  h->non_elf = 0;
	  h->def_regular = 1;
	  h->type = STT_OBJECT;

	  if (! bfd_elf_link_record_dynamic_symbol (info, h))
	    return false;
	  mips_elf_hash_table (info)->rld_symbol = h;
	}
    }

  /* Create the .plt, .rel(a).plt, .dynbss and .rel(a).bss sections.
     Also, on VxWorks, create the _PROCEDURE_LINKAGE_TABLE_ symbol.  */
  if (!_bfd_elf_create_dynamic_sections (abfd, info))
    return false;

  /* Do the usual VxWorks handling.  */
  if (htab->root.target_os == is_vxworks
      && !elf_vxworks_create_dynamic_sections (abfd, info, &htab->srelplt2))
    return false;

  return true;
}

/* Return true if relocation REL against section SEC is a REL rather than
   RELA relocation.  RELOCS is the first relocation in the section and
   ABFD is the bfd that contains SEC.  */

static bool
mips_elf_rel_relocation_p (bfd *abfd, asection *sec,
			   const Elf_Internal_Rela *relocs,
			   const Elf_Internal_Rela *rel)
{
  Elf_Internal_Shdr *rel_hdr;
  const struct elf_backend_data *bed;

  /* To determine which flavor of relocation this is, we depend on the
     fact that the INPUT_SECTION's REL_HDR is read before RELA_HDR.  */
  rel_hdr = elf_section_data (sec)->rel.hdr;
  if (rel_hdr == NULL)
    return false;
  bed = get_elf_backend_data (abfd);
  return ((size_t) (rel - relocs)
	  < NUM_SHDR_ENTRIES (rel_hdr) * bed->s->int_rels_per_ext_rel);
}

/* Read the addend for REL relocation REL, which belongs to bfd ABFD.
   HOWTO is the relocation's howto and CONTENTS points to the contents
   of the section that REL is against.  */

static bfd_vma
mips_elf_read_rel_addend (bfd *abfd, asection *sec,
			  const Elf_Internal_Rela *rel,
			  reloc_howto_type *howto, bfd_byte *contents)
{
  bfd_byte *location;
  unsigned int r_type;
  bfd_vma addend;
  bfd_vma bytes;

  if (!bfd_reloc_offset_in_range (howto, abfd, sec, rel->r_offset))
    return 0;

  r_type = ELF_R_TYPE (abfd, rel->r_info);
  location = contents + rel->r_offset;

  /* Get the addend, which is stored in the input file.  */
  _bfd_mips_elf_reloc_unshuffle (abfd, r_type, false, location);
  bytes = mips_elf_obtain_contents (howto, rel, abfd, contents);
  _bfd_mips_elf_reloc_shuffle (abfd, r_type, false, location);

  addend = bytes & howto->src_mask;

  /* Shift is 2, unusually, for microMIPS JALX.  Adjust the addend
     accordingly.  */
  if (r_type == R_MICROMIPS_26_S1 && (bytes >> 26) == 0x3c)
    addend <<= 1;

  return addend;
}

/* REL is a relocation in ABFD that needs a partnering LO16 relocation
   and *ADDEND is the addend for REL itself.  Look for the LO16 relocation
   and update *ADDEND with the final addend.  Return true on success
   or false if the LO16 could not be found.  RELEND is the exclusive
   upper bound on the relocations for REL's section.  */

static bool
mips_elf_add_lo16_rel_addend (bfd *abfd,
			      asection *sec,
			      const Elf_Internal_Rela *rel,
			      const Elf_Internal_Rela *relend,
			      bfd_byte *contents, bfd_vma *addend)
{
  unsigned int r_type, lo16_type;
  const Elf_Internal_Rela *lo16_relocation;
  reloc_howto_type *lo16_howto;
  bfd_vma l;

  r_type = ELF_R_TYPE (abfd, rel->r_info);
  if (mips16_reloc_p (r_type))
    lo16_type = R_MIPS16_LO16;
  else if (micromips_reloc_p (r_type))
    lo16_type = R_MICROMIPS_LO16;
  else if (r_type == R_MIPS_PCHI16)
    lo16_type = R_MIPS_PCLO16;
  else
    lo16_type = R_MIPS_LO16;

  /* The combined value is the sum of the HI16 addend, left-shifted by
     sixteen bits, and the LO16 addend, sign extended.  (Usually, the
     code does a `lui' of the HI16 value, and then an `addiu' of the
     LO16 value.)

     Scan ahead to find a matching LO16 relocation.

     According to the MIPS ELF ABI, the R_MIPS_LO16 relocation must
     be immediately following.  However, for the IRIX6 ABI, the next
     relocation may be a composed relocation consisting of several
     relocations for the same address.  In that case, the R_MIPS_LO16
     relocation may occur as one of these.  We permit a similar
     extension in general, as that is useful for GCC.

     In some cases GCC dead code elimination removes the LO16 but keeps
     the corresponding HI16.  This is strictly speaking a violation of
     the ABI but not immediately harmful.  */
  lo16_relocation = mips_elf_next_relocation (abfd, lo16_type, rel, relend);
  if (lo16_relocation == NULL)
    return false;

  /* Obtain the addend kept there.  */
  lo16_howto = MIPS_ELF_RTYPE_TO_HOWTO (abfd, lo16_type, false);
  l = mips_elf_read_rel_addend (abfd, sec, lo16_relocation, lo16_howto,
				contents);

  l <<= lo16_howto->rightshift;
  l = _bfd_mips_elf_sign_extend (l, 16);

  *addend <<= 16;
  *addend += l;
  return true;
}

/* Try to read the contents of section SEC in bfd ABFD.  Return true and
   store the contents in *CONTENTS on success.  Assume that *CONTENTS
   already holds the contents if it is nonull on entry.  */

static bool
mips_elf_get_section_contents (bfd *abfd, asection *sec, bfd_byte **contents)
{
  if (*contents)
    return true;

  /* Get cached copy if it exists.  */
  if (elf_section_data (sec)->this_hdr.contents != NULL)
    {
      *contents = elf_section_data (sec)->this_hdr.contents;
      return true;
    }

  return bfd_malloc_and_get_section (abfd, sec, contents);
}

/* Make a new PLT record to keep internal data.  */

static struct plt_entry *
mips_elf_make_plt_record (bfd *abfd)
{
  struct plt_entry *entry;

  entry = bfd_zalloc (abfd, sizeof (*entry));
  if (entry == NULL)
    return NULL;

  entry->stub_offset = MINUS_ONE;
  entry->mips_offset = MINUS_ONE;
  entry->comp_offset = MINUS_ONE;
  entry->gotplt_index = MINUS_ONE;
  return entry;
}

/* Define the special `__gnu_absolute_zero' symbol.  We only need this
   for PIC code, as otherwise there is no load-time relocation involved
   and local GOT entries whose value is zero at static link time will
   retain their value at load time.  */

static bool
mips_elf_define_absolute_zero (bfd *abfd, struct bfd_link_info *info,
			       struct mips_elf_link_hash_table *htab,
			       unsigned int r_type)
{
  union
    {
      struct elf_link_hash_entry *eh;
      struct bfd_link_hash_entry *bh;
    }
  hzero;

  BFD_ASSERT (!htab->use_absolute_zero);
  BFD_ASSERT (bfd_link_pic (info));

  hzero.bh = NULL;
  if (!_bfd_generic_link_add_one_symbol (info, abfd, "__gnu_absolute_zero",
					 BSF_GLOBAL, bfd_abs_section_ptr, 0,
					 NULL, false, false, &hzero.bh))
    return false;

  BFD_ASSERT (hzero.bh != NULL);
  hzero.eh->size = 0;
  hzero.eh->type = STT_NOTYPE;
  hzero.eh->other = STV_PROTECTED;
  hzero.eh->def_regular = 1;
  hzero.eh->non_elf = 0;

  if (!mips_elf_record_global_got_symbol (hzero.eh, abfd, info, true, r_type))
    return false;

  htab->use_absolute_zero = true;

  return true;
}

/* Look through the relocs for a section during the first phase, and
   allocate space in the global offset table and record the need for
   standard MIPS and compressed procedure linkage table entries.  */

bool
_bfd_mips_elf_check_relocs (bfd *abfd, struct bfd_link_info *info,
			    asection *sec, const Elf_Internal_Rela *relocs)
{
  const char *name;
  bfd *dynobj;
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  size_t extsymoff;
  const Elf_Internal_Rela *rel;
  const Elf_Internal_Rela *rel_end;
  asection *sreloc;
  const struct elf_backend_data *bed;
  struct mips_elf_link_hash_table *htab;
  bfd_byte *contents;
  bfd_vma addend;
  reloc_howto_type *howto;

  if (bfd_link_relocatable (info))
    return true;

  htab = mips_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);

  dynobj = elf_hash_table (info)->dynobj;
  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (abfd);
  extsymoff = (elf_bad_symtab (abfd)) ? 0 : symtab_hdr->sh_info;

  bed = get_elf_backend_data (abfd);
  rel_end = relocs + sec->reloc_count;

  /* Check for the mips16 stub sections.  */

  name = bfd_section_name (sec);
  if (FN_STUB_P (name))
    {
      unsigned long r_symndx;

      /* Look at the relocation information to figure out which symbol
	 this is for.  */

      r_symndx = mips16_stub_symndx (bed, sec, relocs, rel_end);
      if (r_symndx == 0)
	{
	  _bfd_error_handler
	    /* xgettext:c-format */
	    (_("%pB: warning: cannot determine the target function for"
	       " stub section `%s'"),
	     abfd, name);
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}

      if (r_symndx < extsymoff
	  || sym_hashes[r_symndx - extsymoff] == NULL)
	{
	  asection *o;

	  /* This stub is for a local symbol.  This stub will only be
	     needed if there is some relocation in this BFD, other
	     than a 16 bit function call, which refers to this symbol.  */
	  for (o = abfd->sections; o != NULL; o = o->next)
	    {
	      Elf_Internal_Rela *sec_relocs;
	      const Elf_Internal_Rela *r, *rend;

	      /* We can ignore stub sections when looking for relocs.  */
	      if ((o->flags & SEC_RELOC) == 0
		  || o->reloc_count == 0
		  || section_allows_mips16_refs_p (o))
		continue;

	      sec_relocs
		= _bfd_elf_link_read_relocs (abfd, o, NULL, NULL,
					     info->keep_memory);
	      if (sec_relocs == NULL)
		return false;

	      rend = sec_relocs + o->reloc_count;
	      for (r = sec_relocs; r < rend; r++)
		if (ELF_R_SYM (abfd, r->r_info) == r_symndx
		    && !mips16_call_reloc_p (ELF_R_TYPE (abfd, r->r_info)))
		  break;

	      if (elf_section_data (o)->relocs != sec_relocs)
		free (sec_relocs);

	      if (r < rend)
		break;
	    }

	  if (o == NULL)
	    {
	      /* There is no non-call reloc for this stub, so we do
		 not need it.  Since this function is called before
		 the linker maps input sections to output sections, we
		 can easily discard it by setting the SEC_EXCLUDE
		 flag.  */
	      sec->flags |= SEC_EXCLUDE;
	      return true;
	    }

	  /* Record this stub in an array of local symbol stubs for
	     this BFD.  */
	  if (mips_elf_tdata (abfd)->local_stubs == NULL)
	    {
	      unsigned long symcount;
	      asection **n;
	      bfd_size_type amt;

	      if (elf_bad_symtab (abfd))
		symcount = NUM_SHDR_ENTRIES (symtab_hdr);
	      else
		symcount = symtab_hdr->sh_info;
	      amt = symcount * sizeof (asection *);
	      n = bfd_zalloc (abfd, amt);
	      if (n == NULL)
		return false;
	      mips_elf_tdata (abfd)->local_stubs = n;
	    }

	  sec->flags |= SEC_KEEP;
	  mips_elf_tdata (abfd)->local_stubs[r_symndx] = sec;

	  /* We don't need to set mips16_stubs_seen in this case.
	     That flag is used to see whether we need to look through
	     the global symbol table for stubs.  We don't need to set
	     it here, because we just have a local stub.  */
	}
      else
	{
	  struct mips_elf_link_hash_entry *h;

	  h = ((struct mips_elf_link_hash_entry *)
	       sym_hashes[r_symndx - extsymoff]);

	  while (h->root.root.type == bfd_link_hash_indirect
		 || h->root.root.type == bfd_link_hash_warning)
	    h = (struct mips_elf_link_hash_entry *) h->root.root.u.i.link;

	  /* H is the symbol this stub is for.  */

	  /* If we already have an appropriate stub for this function, we
	     don't need another one, so we can discard this one.  Since
	     this function is called before the linker maps input sections
	     to output sections, we can easily discard it by setting the
	     SEC_EXCLUDE flag.  */
	  if (h->fn_stub != NULL)
	    {
	      sec->flags |= SEC_EXCLUDE;
	      return true;
	    }

	  sec->flags |= SEC_KEEP;
	  h->fn_stub = sec;
	  mips_elf_hash_table (info)->mips16_stubs_seen = true;
	}
    }
  else if (CALL_STUB_P (name) || CALL_FP_STUB_P (name))
    {
      unsigned long r_symndx;
      struct mips_elf_link_hash_entry *h;
      asection **loc;

      /* Look at the relocation information to figure out which symbol
	 this is for.  */

      r_symndx = mips16_stub_symndx (bed, sec, relocs, rel_end);
      if (r_symndx == 0)
	{
	  _bfd_error_handler
	    /* xgettext:c-format */
	    (_("%pB: warning: cannot determine the target function for"
	       " stub section `%s'"),
	     abfd, name);
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}

      if (r_symndx < extsymoff
	  || sym_hashes[r_symndx - extsymoff] == NULL)
	{
	  asection *o;

	  /* This stub is for a local symbol.  This stub will only be
	     needed if there is some relocation (R_MIPS16_26) in this BFD
	     that refers to this symbol.  */
	  for (o = abfd->sections; o != NULL; o = o->next)
	    {
	      Elf_Internal_Rela *sec_relocs;
	      const Elf_Internal_Rela *r, *rend;

	      /* We can ignore stub sections when looking for relocs.  */
	      if ((o->flags & SEC_RELOC) == 0
		  || o->reloc_count == 0
		  || section_allows_mips16_refs_p (o))
		continue;

	      sec_relocs
		= _bfd_elf_link_read_relocs (abfd, o, NULL, NULL,
					     info->keep_memory);
	      if (sec_relocs == NULL)
		return false;

	      rend = sec_relocs + o->reloc_count;
	      for (r = sec_relocs; r < rend; r++)
		if (ELF_R_SYM (abfd, r->r_info) == r_symndx
		    && ELF_R_TYPE (abfd, r->r_info) == R_MIPS16_26)
		    break;

	      if (elf_section_data (o)->relocs != sec_relocs)
		free (sec_relocs);

	      if (r < rend)
		break;
	    }

	  if (o == NULL)
	    {
	      /* There is no non-call reloc for this stub, so we do
		 not need it.  Since this function is called before
		 the linker maps input sections to output sections, we
		 can easily discard it by setting the SEC_EXCLUDE
		 flag.  */
	      sec->flags |= SEC_EXCLUDE;
	      return true;
	    }

	  /* Record this stub in an array of local symbol call_stubs for
	     this BFD.  */
	  if (mips_elf_tdata (abfd)->local_call_stubs == NULL)
	    {
	      unsigned long symcount;
	      asection **n;
	      bfd_size_type amt;

	      if (elf_bad_symtab (abfd))
		symcount = NUM_SHDR_ENTRIES (symtab_hdr);
	      else
		symcount = symtab_hdr->sh_info;
	      amt = symcount * sizeof (asection *);
	      n = bfd_zalloc (abfd, amt);
	      if (n == NULL)
		return false;
	      mips_elf_tdata (abfd)->local_call_stubs = n;
	    }

	  sec->flags |= SEC_KEEP;
	  mips_elf_tdata (abfd)->local_call_stubs[r_symndx] = sec;

	  /* We don't need to set mips16_stubs_seen in this case.
	     That flag is used to see whether we need to look through
	     the global symbol table for stubs.  We don't need to set
	     it here, because we just have a local stub.  */
	}
      else
	{
	  h = ((struct mips_elf_link_hash_entry *)
	       sym_hashes[r_symndx - extsymoff]);

	  /* H is the symbol this stub is for.  */

	  if (CALL_FP_STUB_P (name))
	    loc = &h->call_fp_stub;
	  else
	    loc = &h->call_stub;

	  /* If we already have an appropriate stub for this function, we
	     don't need another one, so we can discard this one.  Since
	     this function is called before the linker maps input sections
	     to output sections, we can easily discard it by setting the
	     SEC_EXCLUDE flag.  */
	  if (*loc != NULL)
	    {
	      sec->flags |= SEC_EXCLUDE;
	      return true;
	    }

	  sec->flags |= SEC_KEEP;
	  *loc = sec;
	  mips_elf_hash_table (info)->mips16_stubs_seen = true;
	}
    }

  sreloc = NULL;
  contents = NULL;
  for (rel = relocs; rel < rel_end; ++rel)
    {
      unsigned long r_symndx;
      unsigned int r_type;
      struct elf_link_hash_entry *h;
      bool can_make_dynamic_p;
      bool call_reloc_p;
      bool constrain_symbol_p;

      r_symndx = ELF_R_SYM (abfd, rel->r_info);
      r_type = ELF_R_TYPE (abfd, rel->r_info);

      if (r_symndx < extsymoff)
	h = NULL;
      else if (r_symndx >= extsymoff + NUM_SHDR_ENTRIES (symtab_hdr))
	{
	  _bfd_error_handler
	    /* xgettext:c-format */
	    (_("%pB: malformed reloc detected for section %s"),
	     abfd, name);
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}
      else
	{
	  h = sym_hashes[r_symndx - extsymoff];
	  if (h != NULL)
	    {
	      while (h->root.type == bfd_link_hash_indirect
		     || h->root.type == bfd_link_hash_warning)
		h = (struct elf_link_hash_entry *) h->root.u.i.link;
	    }
	}

      /* Set CAN_MAKE_DYNAMIC_P to true if we can convert this
	 relocation into a dynamic one.  */
      can_make_dynamic_p = false;

      /* Set CALL_RELOC_P to true if the relocation is for a call,
	 and if pointer equality therefore doesn't matter.  */
      call_reloc_p = false;

      /* Set CONSTRAIN_SYMBOL_P if we need to take the relocation
	 into account when deciding how to define the symbol.  */
      constrain_symbol_p = true;

      switch (r_type)
	{
	case R_MIPS_CALL16:
	case R_MIPS_CALL_HI16:
	case R_MIPS_CALL_LO16:
	case R_MIPS16_CALL16:
	case R_MICROMIPS_CALL16:
	case R_MICROMIPS_CALL_HI16:
	case R_MICROMIPS_CALL_LO16:
	  call_reloc_p = true;
	  /* Fall through.  */

	case R_MIPS_GOT16:
	case R_MIPS_GOT_LO16:
	case R_MIPS_GOT_PAGE:
	case R_MIPS_GOT_DISP:
	case R_MIPS16_GOT16:
	case R_MICROMIPS_GOT16:
	case R_MICROMIPS_GOT_LO16:
	case R_MICROMIPS_GOT_PAGE:
	case R_MICROMIPS_GOT_DISP:
	  /* If we have a symbol that will resolve to zero at static link
	     time and it is used by a GOT relocation applied to code we
	     cannot relax to an immediate zero load, then we will be using
	     the special `__gnu_absolute_zero' symbol whose value is zero
	     at dynamic load time.  We ignore HI16-type GOT relocations at
	     this stage, because their handling will depend entirely on
	     the corresponding LO16-type GOT relocation.  */
	  if (!call_hi16_reloc_p (r_type)
	      && h != NULL
	      && bfd_link_pic (info)
	      && !htab->use_absolute_zero
	      && UNDEFWEAK_NO_DYNAMIC_RELOC (info, h))
	    {
	      bool rel_reloc;

	      if (!mips_elf_get_section_contents (abfd, sec, &contents))
		return false;

	      rel_reloc = mips_elf_rel_relocation_p (abfd, sec, relocs, rel);
	      howto = MIPS_ELF_RTYPE_TO_HOWTO (abfd, r_type, !rel_reloc);
	      if (bfd_reloc_offset_in_range (howto, abfd, sec, rel->r_offset))
		if (!mips_elf_nullify_got_load (abfd, contents, rel, howto,
						false))
		  if (!mips_elf_define_absolute_zero (abfd, info, htab,
						      r_type))
		    return false;
	    }

	  /* Fall through.  */
	case R_MIPS_GOT_HI16:
	case R_MIPS_GOT_OFST:
	case R_MIPS_TLS_GOTTPREL:
	case R_MIPS_TLS_GD:
	case R_MIPS_TLS_LDM:
	case R_MIPS16_TLS_GOTTPREL:
	case R_MIPS16_TLS_GD:
	case R_MIPS16_TLS_LDM:
	case R_MICROMIPS_GOT_HI16:
	case R_MICROMIPS_GOT_OFST:
	case R_MICROMIPS_TLS_GOTTPREL:
	case R_MICROMIPS_TLS_GD:
	case R_MICROMIPS_TLS_LDM:
	  if (dynobj == NULL)
	    elf_hash_table (info)->dynobj = dynobj = abfd;
	  if (!mips_elf_create_got_section (dynobj, info))
	    return false;
	  if (htab->root.target_os == is_vxworks
	      && !bfd_link_pic (info))
	    {
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB: GOT reloc at %#" PRIx64 " not expected in executables"),
		 abfd, (uint64_t) rel->r_offset);
	      bfd_set_error (bfd_error_bad_value);
	      return false;
	    }
	  can_make_dynamic_p = true;
	  break;

	case R_MIPS_NONE:
	case R_MIPS_JALR:
	case R_MICROMIPS_JALR:
	  /* These relocations have empty fields and are purely there to
	     provide link information.  The symbol value doesn't matter.  */
	  constrain_symbol_p = false;
	  break;

	case R_MIPS_GPREL16:
	case R_MIPS_GPREL32:
	case R_MIPS16_GPREL:
	case R_MICROMIPS_GPREL16:
	  /* GP-relative relocations always resolve to a definition in a
	     regular input file, ignoring the one-definition rule.  This is
	     important for the GP setup sequence in NewABI code, which
	     always resolves to a local function even if other relocations
	     against the symbol wouldn't.  */
	  constrain_symbol_p = false;
	  break;

	case R_MIPS_32:
	case R_MIPS_REL32:
	case R_MIPS_64:
	  /* In VxWorks executables, references to external symbols
	     must be handled using copy relocs or PLT entries; it is not
	     possible to convert this relocation into a dynamic one.

	     For executables that use PLTs and copy-relocs, we have a
	     choice between converting the relocation into a dynamic
	     one or using copy relocations or PLT entries.  It is
	     usually better to do the former, unless the relocation is
	     against a read-only section.  */
	  if ((bfd_link_pic (info)
	       || (h != NULL
		   && htab->root.target_os != is_vxworks
		   && strcmp (h->root.root.string, "__gnu_local_gp") != 0
		   && !(!info->nocopyreloc
			&& !PIC_OBJECT_P (abfd)
			&& MIPS_ELF_READONLY_SECTION (sec))))
	      && (sec->flags & SEC_ALLOC) != 0)
	    {
	      can_make_dynamic_p = true;
	      if (dynobj == NULL)
		elf_hash_table (info)->dynobj = dynobj = abfd;
	    }
	  break;

	case R_MIPS_26:
	case R_MIPS_PC16:
	case R_MIPS_PC21_S2:
	case R_MIPS_PC26_S2:
	case R_MIPS16_26:
	case R_MIPS16_PC16_S1:
	case R_MICROMIPS_26_S1:
	case R_MICROMIPS_PC7_S1:
	case R_MICROMIPS_PC10_S1:
	case R_MICROMIPS_PC16_S1:
	case R_MICROMIPS_PC23_S2:
	  call_reloc_p = true;
	  break;
	}

      if (h)
	{
	  if (constrain_symbol_p)
	    {
	      if (!can_make_dynamic_p)
		((struct mips_elf_link_hash_entry *) h)->has_static_relocs = 1;

	      if (!call_reloc_p)
		h->pointer_equality_needed = 1;

	      /* We must not create a stub for a symbol that has
		 relocations related to taking the function's address.
		 This doesn't apply to VxWorks, where CALL relocs refer
		 to a .got.plt entry instead of a normal .got entry.  */
	      if (htab->root.target_os != is_vxworks
		  && (!can_make_dynamic_p || !call_reloc_p))
		((struct mips_elf_link_hash_entry *) h)->no_fn_stub = true;
	    }

	  /* Relocations against the special VxWorks __GOTT_BASE__ and
	     __GOTT_INDEX__ symbols must be left to the loader.  Allocate
	     room for them in .rela.dyn.  */
	  if (is_gott_symbol (info, h))
	    {
	      if (sreloc == NULL)
		{
		  sreloc = mips_elf_rel_dyn_section (info, true);
		  if (sreloc == NULL)
		    return false;
		}
	      mips_elf_allocate_dynamic_relocations (dynobj, info, 1);
	      if (MIPS_ELF_READONLY_SECTION (sec))
		/* We tell the dynamic linker that there are
		   relocations against the text segment.  */
		info->flags |= DF_TEXTREL;
	    }
	}
      else if (call_lo16_reloc_p (r_type)
	       || got_lo16_reloc_p (r_type)
	       || got_disp_reloc_p (r_type)
	       || (got16_reloc_p (r_type)
		   && htab->root.target_os == is_vxworks))
	{
	  /* We may need a local GOT entry for this relocation.  We
	     don't count R_MIPS_GOT_PAGE because we can estimate the
	     maximum number of pages needed by looking at the size of
	     the segment.  Similar comments apply to R_MIPS*_GOT16 and
	     R_MIPS*_CALL16, except on VxWorks, where GOT relocations
	     always evaluate to "G".  We don't count R_MIPS_GOT_HI16, or
	     R_MIPS_CALL_HI16 because these are always followed by an
	     R_MIPS_GOT_LO16 or R_MIPS_CALL_LO16.  */
	  if (!mips_elf_record_local_got_symbol (abfd, r_symndx,
						 rel->r_addend, info, r_type))
	    return false;
	}

      if (h != NULL
	  && mips_elf_relocation_needs_la25_stub (abfd, r_type,
						  ELF_ST_IS_MIPS16 (h->other)))
	((struct mips_elf_link_hash_entry *) h)->has_nonpic_branches = true;

      switch (r_type)
	{
	case R_MIPS_CALL16:
	case R_MIPS16_CALL16:
	case R_MICROMIPS_CALL16:
	  if (h == NULL)
	    {
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB: CALL16 reloc at %#" PRIx64 " not against global symbol"),
		 abfd, (uint64_t) rel->r_offset);
	      bfd_set_error (bfd_error_bad_value);
	      return false;
	    }
	  /* Fall through.  */

	case R_MIPS_CALL_HI16:
	case R_MIPS_CALL_LO16:
	case R_MICROMIPS_CALL_HI16:
	case R_MICROMIPS_CALL_LO16:
	  if (h != NULL)
	    {
	      /* Make sure there is room in the regular GOT to hold the
		 function's address.  We may eliminate it in favour of
		 a .got.plt entry later; see mips_elf_count_got_symbols.  */
	      if (!mips_elf_record_global_got_symbol (h, abfd, info, true,
						      r_type))
		return false;

	      /* We need a stub, not a plt entry for the undefined
		 function.  But we record it as if it needs plt.  See
		 _bfd_elf_adjust_dynamic_symbol.  */
	      h->needs_plt = 1;
	      h->type = STT_FUNC;
	    }
	  break;

	case R_MIPS_GOT_PAGE:
	case R_MICROMIPS_GOT_PAGE:
	case R_MIPS16_GOT16:
	case R_MIPS_GOT16:
	case R_MIPS_GOT_HI16:
	case R_MIPS_GOT_LO16:
	case R_MICROMIPS_GOT16:
	case R_MICROMIPS_GOT_HI16:
	case R_MICROMIPS_GOT_LO16:
	  if (!h || got_page_reloc_p (r_type))
	    {
	      /* This relocation needs (or may need, if h != NULL) a
		 page entry in the GOT.  For R_MIPS_GOT_PAGE we do not
		 know for sure until we know whether the symbol is
		 preemptible.  */
	      if (mips_elf_rel_relocation_p (abfd, sec, relocs, rel))
		{
		  if (!mips_elf_get_section_contents (abfd, sec, &contents))
		    return false;
		  howto = MIPS_ELF_RTYPE_TO_HOWTO (abfd, r_type, false);
		  addend = mips_elf_read_rel_addend (abfd, sec, rel,
						     howto, contents);
		  if (got16_reloc_p (r_type))
		    mips_elf_add_lo16_rel_addend (abfd, sec, rel, rel_end,
						  contents, &addend);
		  else
		    addend <<= howto->rightshift;
		}
	      else
		addend = rel->r_addend;
	      if (!mips_elf_record_got_page_ref (info, abfd, r_symndx,
						 h, addend))
		return false;

	      if (h)
		{
		  struct mips_elf_link_hash_entry *hmips =
		    (struct mips_elf_link_hash_entry *) h;

		  /* This symbol is definitely not overridable.  */
		  if (hmips->root.def_regular
		      && ! (bfd_link_pic (info) && ! info->symbolic
			    && ! hmips->root.forced_local))
		    h = NULL;
		}
	    }
	  /* If this is a global, overridable symbol, GOT_PAGE will
	     decay to GOT_DISP, so we'll need a GOT entry for it.  */
	  /* Fall through.  */

	case R_MIPS_GOT_DISP:
	case R_MICROMIPS_GOT_DISP:
	  if (h && !mips_elf_record_global_got_symbol (h, abfd, info,
						       false, r_type))
	    return false;
	  break;

	case R_MIPS_TLS_GOTTPREL:
	case R_MIPS16_TLS_GOTTPREL:
	case R_MICROMIPS_TLS_GOTTPREL:
	  if (bfd_link_pic (info))
	    info->flags |= DF_STATIC_TLS;
	  /* Fall through */

	case R_MIPS_TLS_LDM:
	case R_MIPS16_TLS_LDM:
	case R_MICROMIPS_TLS_LDM:
	  if (tls_ldm_reloc_p (r_type))
	    {
	      r_symndx = STN_UNDEF;
	      h = NULL;
	    }
	  /* Fall through */

	case R_MIPS_TLS_GD:
	case R_MIPS16_TLS_GD:
	case R_MICROMIPS_TLS_GD:
	  /* This symbol requires a global offset table entry, or two
	     for TLS GD relocations.  */
	  if (h != NULL)
	    {
	      if (!mips_elf_record_global_got_symbol (h, abfd, info,
						      false, r_type))
		return false;
	    }
	  else
	    {
	      if (!mips_elf_record_local_got_symbol (abfd, r_symndx,
						     rel->r_addend,
						     info, r_type))
		return false;
	    }
	  break;

	case R_MIPS_32:
	case R_MIPS_REL32:
	case R_MIPS_64:
	  /* In VxWorks executables, references to external symbols
	     are handled using copy relocs or PLT stubs, so there's
	     no need to add a .rela.dyn entry for this relocation.  */
	  if (can_make_dynamic_p)
	    {
	      if (sreloc == NULL)
		{
		  sreloc = mips_elf_rel_dyn_section (info, true);
		  if (sreloc == NULL)
		    return false;
		}
	      if (bfd_link_pic (info) && h == NULL)
		{
		  /* When creating a shared object, we must copy these
		     reloc types into the output file as R_MIPS_REL32
		     relocs.  Make room for this reloc in .rel(a).dyn.  */
		  mips_elf_allocate_dynamic_relocations (dynobj, info, 1);
		  if (MIPS_ELF_READONLY_SECTION (sec))
		    /* We tell the dynamic linker that there are
		       relocations against the text segment.  */
		    info->flags |= DF_TEXTREL;
		}
	      else
		{
		  struct mips_elf_link_hash_entry *hmips;

		  /* For a shared object, we must copy this relocation
		     unless the symbol turns out to be undefined and
		     weak with non-default visibility, in which case
		     it will be left as zero.

		     We could elide R_MIPS_REL32 for locally binding symbols
		     in shared libraries, but do not yet do so.

		     For an executable, we only need to copy this
		     reloc if the symbol is defined in a dynamic
		     object.  */
		  hmips = (struct mips_elf_link_hash_entry *) h;
		  ++hmips->possibly_dynamic_relocs;
		  if (MIPS_ELF_READONLY_SECTION (sec))
		    /* We need it to tell the dynamic linker if there
		       are relocations against the text segment.  */
		    hmips->readonly_reloc = true;
		}
	    }

	  if (SGI_COMPAT (abfd))
	    mips_elf_hash_table (info)->compact_rel_size +=
	      sizeof (Elf32_External_crinfo);
	  break;

	case R_MIPS_26:
	case R_MIPS_GPREL16:
	case R_MIPS_LITERAL:
	case R_MIPS_GPREL32:
	case R_MICROMIPS_26_S1:
	case R_MICROMIPS_GPREL16:
	case R_MICROMIPS_LITERAL:
	case R_MICROMIPS_GPREL7_S2:
	  if (SGI_COMPAT (abfd))
	    mips_elf_hash_table (info)->compact_rel_size +=
	      sizeof (Elf32_External_crinfo);
	  break;

	  /* This relocation describes the C++ object vtable hierarchy.
	     Reconstruct it for later use during GC.  */
	case R_MIPS_GNU_VTINHERIT:
	  if (!bfd_elf_gc_record_vtinherit (abfd, sec, h, rel->r_offset))
	    return false;
	  break;

	  /* This relocation describes which C++ vtable entries are actually
	     used.  Record for later use during GC.  */
	case R_MIPS_GNU_VTENTRY:
	  if (!bfd_elf_gc_record_vtentry (abfd, sec, h, rel->r_offset))
	    return false;
	  break;

	default:
	  break;
	}

      /* Record the need for a PLT entry.  At this point we don't know
	 yet if we are going to create a PLT in the first place, but
	 we only record whether the relocation requires a standard MIPS
	 or a compressed code entry anyway.  If we don't make a PLT after
	 all, then we'll just ignore these arrangements.  Likewise if
	 a PLT entry is not created because the symbol is satisfied
	 locally.  */
      if (h != NULL
	  && (branch_reloc_p (r_type)
	      || mips16_branch_reloc_p (r_type)
	      || micromips_branch_reloc_p (r_type))
	  && !SYMBOL_CALLS_LOCAL (info, h))
	{
	  if (h->plt.plist == NULL)
	    h->plt.plist = mips_elf_make_plt_record (abfd);
	  if (h->plt.plist == NULL)
	    return false;

	  if (branch_reloc_p (r_type))
	    h->plt.plist->need_mips = true;
	  else
	    h->plt.plist->need_comp = true;
	}

      /* See if this reloc would need to refer to a MIPS16 hard-float stub,
	 if there is one.  We only need to handle global symbols here;
	 we decide whether to keep or delete stubs for local symbols
	 when processing the stub's relocations.  */
      if (h != NULL
	  && !mips16_call_reloc_p (r_type)
	  && !section_allows_mips16_refs_p (sec))
	{
	  struct mips_elf_link_hash_entry *mh;

	  mh = (struct mips_elf_link_hash_entry *) h;
	  mh->need_fn_stub = true;
	}

      /* Refuse some position-dependent relocations when creating a
	 shared library.  Do not refuse R_MIPS_32 / R_MIPS_64; they're
	 not PIC, but we can create dynamic relocations and the result
	 will be fine.  Also do not refuse R_MIPS_LO16, which can be
	 combined with R_MIPS_GOT16.  */
      if (bfd_link_pic (info))
	{
	  switch (r_type)
	    {
	    case R_MIPS_TLS_TPREL_HI16:
	    case R_MIPS16_TLS_TPREL_HI16:
	    case R_MICROMIPS_TLS_TPREL_HI16:
	    case R_MIPS_TLS_TPREL_LO16:
	    case R_MIPS16_TLS_TPREL_LO16:
	    case R_MICROMIPS_TLS_TPREL_LO16:
	      /* These are okay in PIE, but not in a shared library.  */
	      if (bfd_link_executable (info))
		break;

	      /* FALLTHROUGH */

	    case R_MIPS16_HI16:
	    case R_MIPS_HI16:
	    case R_MIPS_HIGHER:
	    case R_MIPS_HIGHEST:
	    case R_MICROMIPS_HI16:
	    case R_MICROMIPS_HIGHER:
	    case R_MICROMIPS_HIGHEST:
	      /* Don't refuse a high part relocation if it's against
		 no symbol (e.g. part of a compound relocation).  */
	      if (r_symndx == STN_UNDEF)
		break;

	      /* Likewise an absolute symbol.  */
	      if (h != NULL && bfd_is_abs_symbol (&h->root))
		break;

	      /* R_MIPS_HI16 against _gp_disp is used for $gp setup,
		 and has a special meaning.  */
	      if (!NEWABI_P (abfd) && h != NULL
		  && strcmp (h->root.root.string, "_gp_disp") == 0)
		break;

	      /* Likewise __GOTT_BASE__ and __GOTT_INDEX__ on VxWorks.  */
	      if (is_gott_symbol (info, h))
		break;

	      /* FALLTHROUGH */

	    case R_MIPS16_26:
	    case R_MIPS_26:
	    case R_MICROMIPS_26_S1:
	      howto = MIPS_ELF_RTYPE_TO_HOWTO (abfd, r_type, NEWABI_P (abfd));
	      /* An error for unsupported relocations is raised as part
		 of the above search, so we can skip the following.  */
	      if (howto != NULL)
		info->callbacks->einfo
		  /* xgettext:c-format */
		  (_("%X%H: relocation %s against `%s' cannot be used"
		     " when making a shared object; recompile with -fPIC\n"),
		   abfd, sec, rel->r_offset, howto->name,
		   (h) ? h->root.root.string : "a local symbol");
	      break;
	    default:
	      break;
	    }
	}
    }

  return true;
}

/* Allocate space for global sym dynamic relocs.  */

static bool
allocate_dynrelocs (struct elf_link_hash_entry *h, void *inf)
{
  struct bfd_link_info *info = inf;
  bfd *dynobj;
  struct mips_elf_link_hash_entry *hmips;
  struct mips_elf_link_hash_table *htab;

  htab = mips_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);

  dynobj = elf_hash_table (info)->dynobj;
  hmips = (struct mips_elf_link_hash_entry *) h;

  /* VxWorks executables are handled elsewhere; we only need to
     allocate relocations in shared objects.  */
  if (htab->root.target_os == is_vxworks && !bfd_link_pic (info))
    return true;

  /* Ignore indirect symbols.  All relocations against such symbols
     will be redirected to the target symbol.  */
  if (h->root.type == bfd_link_hash_indirect)
    return true;

  /* If this symbol is defined in a dynamic object, or we are creating
     a shared library, we will need to copy any R_MIPS_32 or
     R_MIPS_REL32 relocs against it into the output file.  */
  if (! bfd_link_relocatable (info)
      && hmips->possibly_dynamic_relocs != 0
      && (h->root.type == bfd_link_hash_defweak
	  || (!h->def_regular && !ELF_COMMON_DEF_P (h))
	  || bfd_link_pic (info)))
    {
      bool do_copy = true;

      if (h->root.type == bfd_link_hash_undefweak)
	{
	  /* Do not copy relocations for undefined weak symbols that
	     we are not going to export.  */
	  if (UNDEFWEAK_NO_DYNAMIC_RELOC (info, h))
	    do_copy = false;

	  /* Make sure undefined weak symbols are output as a dynamic
	     symbol in PIEs.  */
	  else if (h->dynindx == -1 && !h->forced_local)
	    {
	      if (! bfd_elf_link_record_dynamic_symbol (info, h))
		return false;
	    }
	}

      if (do_copy)
	{
	  /* Even though we don't directly need a GOT entry for this symbol,
	     the SVR4 psABI requires it to have a dynamic symbol table
	     index greater that DT_MIPS_GOTSYM if there are dynamic
	     relocations against it.

	     VxWorks does not enforce the same mapping between the GOT
	     and the symbol table, so the same requirement does not
	     apply there.  */
	  if (htab->root.target_os != is_vxworks)
	    {
	      if (hmips->global_got_area > GGA_RELOC_ONLY)
		hmips->global_got_area = GGA_RELOC_ONLY;
	      hmips->got_only_for_calls = false;
	    }

	  mips_elf_allocate_dynamic_relocations
	    (dynobj, info, hmips->possibly_dynamic_relocs);
	  if (hmips->readonly_reloc)
	    /* We tell the dynamic linker that there are relocations
	       against the text segment.  */
	    info->flags |= DF_TEXTREL;
	}
    }

  return true;
}

/* Adjust a symbol defined by a dynamic object and referenced by a
   regular object.  The current definition is in some section of the
   dynamic object, but we're not including those sections.  We have to
   change the definition to something the rest of the link can
   understand.  */

bool
_bfd_mips_elf_adjust_dynamic_symbol (struct bfd_link_info *info,
				     struct elf_link_hash_entry *h)
{
  bfd *dynobj;
  struct mips_elf_link_hash_entry *hmips;
  struct mips_elf_link_hash_table *htab;
  asection *s, *srel;

  htab = mips_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);

  dynobj = elf_hash_table (info)->dynobj;
  hmips = (struct mips_elf_link_hash_entry *) h;

  /* Make sure we know what is going on here.  */
  if (dynobj == NULL
      || (! h->needs_plt
	  && ! h->is_weakalias
	  && (! h->def_dynamic
	      || ! h->ref_regular
	      || h->def_regular)))
    {
      if (h->type == STT_GNU_IFUNC)
	_bfd_error_handler (_("IFUNC symbol %s in dynamic symbol table - IFUNCS are not supported"),
			    h->root.root.string);
      else
	_bfd_error_handler (_("non-dynamic symbol %s in dynamic symbol table"),
			    h->root.root.string);
      return true;
    }

  hmips = (struct mips_elf_link_hash_entry *) h;

  /* If there are call relocations against an externally-defined symbol,
     see whether we can create a MIPS lazy-binding stub for it.  We can
     only do this if all references to the function are through call
     relocations, and in that case, the traditional lazy-binding stubs
     are much more efficient than PLT entries.

     Traditional stubs are only available on SVR4 psABI-based systems;
     VxWorks always uses PLTs instead.  */
  if (htab->root.target_os != is_vxworks
      && h->needs_plt
      && !hmips->no_fn_stub)
    {
      if (! elf_hash_table (info)->dynamic_sections_created)
	return true;

      /* If this symbol is not defined in a regular file, then set
	 the symbol to the stub location.  This is required to make
	 function pointers compare as equal between the normal
	 executable and the shared library.  */
      if (!h->def_regular
	  && !bfd_is_abs_section (htab->sstubs->output_section))
	{
	  hmips->needs_lazy_stub = true;
	  htab->lazy_stub_count++;
	  return true;
	}
    }
  /* As above, VxWorks requires PLT entries for externally-defined
     functions that are only accessed through call relocations.

     Both VxWorks and non-VxWorks targets also need PLT entries if there
     are static-only relocations against an externally-defined function.
     This can technically occur for shared libraries if there are
     branches to the symbol, although it is unlikely that this will be
     used in practice due to the short ranges involved.  It can occur
     for any relative or absolute relocation in executables; in that
     case, the PLT entry becomes the function's canonical address.  */
  else if (((h->needs_plt && !hmips->no_fn_stub)
	    || (h->type == STT_FUNC && hmips->has_static_relocs))
	   && htab->use_plts_and_copy_relocs
	   && !SYMBOL_CALLS_LOCAL (info, h)
	   && !(ELF_ST_VISIBILITY (h->other) != STV_DEFAULT
		&& h->root.type == bfd_link_hash_undefweak))
    {
      bool micromips_p = MICROMIPS_P (info->output_bfd);
      bool newabi_p = NEWABI_P (info->output_bfd);

      /* If this is the first symbol to need a PLT entry, then make some
	 basic setup.  Also work out PLT entry sizes.  We'll need them
	 for PLT offset calculations.  */
      if (htab->plt_mips_offset + htab->plt_comp_offset == 0)
	{
	  BFD_ASSERT (htab->root.sgotplt->size == 0);
	  BFD_ASSERT (htab->plt_got_index == 0);

	  /* If we're using the PLT additions to the psABI, each PLT
	     entry is 16 bytes and the PLT0 entry is 32 bytes.
	     Encourage better cache usage by aligning.  We do this
	     lazily to avoid pessimizing traditional objects.  */
	  if (htab->root.target_os != is_vxworks
	      && !bfd_set_section_alignment (htab->root.splt, 5))
	    return false;

	  /* Make sure that .got.plt is word-aligned.  We do this lazily
	     for the same reason as above.  */
	  if (!bfd_set_section_alignment (htab->root.sgotplt,
					  MIPS_ELF_LOG_FILE_ALIGN (dynobj)))
	    return false;

	  /* On non-VxWorks targets, the first two entries in .got.plt
	     are reserved.  */
	  if (htab->root.target_os != is_vxworks)
	    htab->plt_got_index
	      += (get_elf_backend_data (dynobj)->got_header_size
		  / MIPS_ELF_GOT_SIZE (dynobj));

	  /* On VxWorks, also allocate room for the header's
	     .rela.plt.unloaded entries.  */
	  if (htab->root.target_os == is_vxworks
	      && !bfd_link_pic (info))
	    htab->srelplt2->size += 2 * sizeof (Elf32_External_Rela);

	  /* Now work out the sizes of individual PLT entries.  */
	  if (htab->root.target_os == is_vxworks
	      && bfd_link_pic (info))
	    htab->plt_mips_entry_size
	      = 4 * ARRAY_SIZE (mips_vxworks_shared_plt_entry);
	  else if (htab->root.target_os == is_vxworks)
	    htab->plt_mips_entry_size
	      = 4 * ARRAY_SIZE (mips_vxworks_exec_plt_entry);
	  else if (newabi_p)
	    htab->plt_mips_entry_size
	      = 4 * ARRAY_SIZE (mips_exec_plt_entry);
	  else if (!micromips_p)
	    {
	      htab->plt_mips_entry_size
		= 4 * ARRAY_SIZE (mips_exec_plt_entry);
	      htab->plt_comp_entry_size
		= 2 * ARRAY_SIZE (mips16_o32_exec_plt_entry);
	    }
	  else if (htab->insn32)
	    {
	      htab->plt_mips_entry_size
		= 4 * ARRAY_SIZE (mips_exec_plt_entry);
	      htab->plt_comp_entry_size
		= 2 * ARRAY_SIZE (micromips_insn32_o32_exec_plt_entry);
	    }
	  else
	    {
	      htab->plt_mips_entry_size
		= 4 * ARRAY_SIZE (mips_exec_plt_entry);
	      htab->plt_comp_entry_size
		= 2 * ARRAY_SIZE (micromips_o32_exec_plt_entry);
	    }
	}

      if (h->plt.plist == NULL)
	h->plt.plist = mips_elf_make_plt_record (dynobj);
      if (h->plt.plist == NULL)
	return false;

      /* There are no defined MIPS16 or microMIPS PLT entries for VxWorks,
	 n32 or n64, so always use a standard entry there.

	 If the symbol has a MIPS16 call stub and gets a PLT entry, then
	 all MIPS16 calls will go via that stub, and there is no benefit
	 to having a MIPS16 entry.  And in the case of call_stub a
	 standard entry actually has to be used as the stub ends with a J
	 instruction.  */
      if (newabi_p
	  || htab->root.target_os == is_vxworks
	  || hmips->call_stub
	  || hmips->call_fp_stub)
	{
	  h->plt.plist->need_mips = true;
	  h->plt.plist->need_comp = false;
	}

      /* Otherwise, if there are no direct calls to the function, we
	 have a free choice of whether to use standard or compressed
	 entries.  Prefer microMIPS entries if the object is known to
	 contain microMIPS code, so that it becomes possible to create
	 pure microMIPS binaries.  Prefer standard entries otherwise,
	 because MIPS16 ones are no smaller and are usually slower.  */
      if (!h->plt.plist->need_mips && !h->plt.plist->need_comp)
	{
	  if (micromips_p)
	    h->plt.plist->need_comp = true;
	  else
	    h->plt.plist->need_mips = true;
	}

      if (h->plt.plist->need_mips)
	{
	  h->plt.plist->mips_offset = htab->plt_mips_offset;
	  htab->plt_mips_offset += htab->plt_mips_entry_size;
	}
      if (h->plt.plist->need_comp)
	{
	  h->plt.plist->comp_offset = htab->plt_comp_offset;
	  htab->plt_comp_offset += htab->plt_comp_entry_size;
	}

      /* Reserve the corresponding .got.plt entry now too.  */
      h->plt.plist->gotplt_index = htab->plt_got_index++;

      /* If the output file has no definition of the symbol, set the
	 symbol's value to the address of the stub.  */
      if (!bfd_link_pic (info) && !h->def_regular)
	hmips->use_plt_entry = true;

      /* Make room for the R_MIPS_JUMP_SLOT relocation.  */
      htab->root.srelplt->size += (htab->root.target_os == is_vxworks
				   ? MIPS_ELF_RELA_SIZE (dynobj)
				   : MIPS_ELF_REL_SIZE (dynobj));

      /* Make room for the .rela.plt.unloaded relocations.  */
      if (htab->root.target_os == is_vxworks && !bfd_link_pic (info))
	htab->srelplt2->size += 3 * sizeof (Elf32_External_Rela);

      /* All relocations against this symbol that could have been made
	 dynamic will now refer to the PLT entry instead.  */
      hmips->possibly_dynamic_relocs = 0;

      return true;
    }

  /* If this is a weak symbol, and there is a real definition, the
     processor independent code will have arranged for us to see the
     real definition first, and we can just use the same value.  */
  if (h->is_weakalias)
    {
      struct elf_link_hash_entry *def = weakdef (h);
      BFD_ASSERT (def->root.type == bfd_link_hash_defined);
      h->root.u.def.section = def->root.u.def.section;
      h->root.u.def.value = def->root.u.def.value;
      return true;
    }

  /* Otherwise, there is nothing further to do for symbols defined
     in regular objects.  */
  if (h->def_regular)
    return true;

  /* There's also nothing more to do if we'll convert all relocations
     against this symbol into dynamic relocations.  */
  if (!hmips->has_static_relocs)
    return true;

  /* We're now relying on copy relocations.  Complain if we have
     some that we can't convert.  */
  if (!htab->use_plts_and_copy_relocs || bfd_link_pic (info))
    {
      _bfd_error_handler (_("non-dynamic relocations refer to "
			    "dynamic symbol %s"),
			  h->root.root.string);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  /* We must allocate the symbol in our .dynbss section, which will
     become part of the .bss section of the executable.  There will be
     an entry for this symbol in the .dynsym section.  The dynamic
     object will contain position independent code, so all references
     from the dynamic object to this symbol will go through the global
     offset table.  The dynamic linker will use the .dynsym entry to
     determine the address it must put in the global offset table, so
     both the dynamic object and the regular object will refer to the
     same memory location for the variable.  */

  if ((h->root.u.def.section->flags & SEC_READONLY) != 0)
    {
      s = htab->root.sdynrelro;
      srel = htab->root.sreldynrelro;
    }
  else
    {
      s = htab->root.sdynbss;
      srel = htab->root.srelbss;
    }
  if ((h->root.u.def.section->flags & SEC_ALLOC) != 0)
    {
      if (htab->root.target_os == is_vxworks)
	srel->size += sizeof (Elf32_External_Rela);
      else
	mips_elf_allocate_dynamic_relocations (dynobj, info, 1);
      h->needs_copy = 1;
    }

  /* All relocations against this symbol that could have been made
     dynamic will now refer to the local copy instead.  */
  hmips->possibly_dynamic_relocs = 0;

  return _bfd_elf_adjust_dynamic_copy (info, h, s);
}

/* This function is called after all the input files have been read,
   and the input sections have been assigned to output sections.  We
   check for any mips16 stub sections that we can discard.  */

bool
_bfd_mips_elf_always_size_sections (bfd *output_bfd,
				    struct bfd_link_info *info)
{
  asection *sect;
  struct mips_elf_link_hash_table *htab;
  struct mips_htab_traverse_info hti;

  htab = mips_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);

  /* The .reginfo section has a fixed size.  */
  sect = bfd_get_section_by_name (output_bfd, ".reginfo");
  if (sect != NULL)
    {
      bfd_set_section_size (sect, sizeof (Elf32_External_RegInfo));
      sect->flags |= SEC_FIXED_SIZE | SEC_HAS_CONTENTS;
    }

  /* The .MIPS.abiflags section has a fixed size.  */
  sect = bfd_get_section_by_name (output_bfd, ".MIPS.abiflags");
  if (sect != NULL)
    {
      bfd_set_section_size (sect, sizeof (Elf_External_ABIFlags_v0));
      sect->flags |= SEC_FIXED_SIZE | SEC_HAS_CONTENTS;
    }

  hti.info = info;
  hti.output_bfd = output_bfd;
  hti.error = false;
  mips_elf_link_hash_traverse (mips_elf_hash_table (info),
			       mips_elf_check_symbols, &hti);
  if (hti.error)
    return false;

  return true;
}

/* If the link uses a GOT, lay it out and work out its size.  */

static bool
mips_elf_lay_out_got (bfd *output_bfd, struct bfd_link_info *info)
{
  bfd *dynobj;
  asection *s;
  struct mips_got_info *g;
  bfd_size_type loadable_size = 0;
  bfd_size_type page_gotno;
  bfd *ibfd;
  struct mips_elf_traverse_got_arg tga;
  struct mips_elf_link_hash_table *htab;

  htab = mips_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);

  s = htab->root.sgot;
  if (s == NULL)
    return true;

  dynobj = elf_hash_table (info)->dynobj;
  g = htab->got_info;

  /* Allocate room for the reserved entries.  VxWorks always reserves
     3 entries; other objects only reserve 2 entries.  */
  BFD_ASSERT (g->assigned_low_gotno == 0);
  if (htab->root.target_os == is_vxworks)
    htab->reserved_gotno = 3;
  else
    htab->reserved_gotno = 2;
  g->local_gotno += htab->reserved_gotno;
  g->assigned_low_gotno = htab->reserved_gotno;

  /* Decide which symbols need to go in the global part of the GOT and
     count the number of reloc-only GOT symbols.  */
  mips_elf_link_hash_traverse (htab, mips_elf_count_got_symbols, info);

  if (!mips_elf_resolve_final_got_entries (info, g))
    return false;

  /* Calculate the total loadable size of the output.  That
     will give us the maximum number of GOT_PAGE entries
     required.  */
  for (ibfd = info->input_bfds; ibfd; ibfd = ibfd->link.next)
    {
      asection *subsection;

      for (subsection = ibfd->sections;
	   subsection;
	   subsection = subsection->next)
	{
	  if ((subsection->flags & SEC_ALLOC) == 0)
	    continue;
	  loadable_size += ((subsection->size + 0xf)
			    &~ (bfd_size_type) 0xf);
	}
    }

  if (htab->root.target_os == is_vxworks)
    /* There's no need to allocate page entries for VxWorks; R_MIPS*_GOT16
       relocations against local symbols evaluate to "G", and the EABI does
       not include R_MIPS_GOT_PAGE.  */
    page_gotno = 0;
  else
    /* Assume there are two loadable segments consisting of contiguous
       sections.  Is 5 enough?  */
    page_gotno = (loadable_size >> 16) + 5;

  /* Choose the smaller of the two page estimates; both are intended to be
     conservative.  */
  if (page_gotno > g->page_gotno)
    page_gotno = g->page_gotno;

  g->local_gotno += page_gotno;
  g->assigned_high_gotno = g->local_gotno - 1;

  s->size += g->local_gotno * MIPS_ELF_GOT_SIZE (output_bfd);
  s->size += g->global_gotno * MIPS_ELF_GOT_SIZE (output_bfd);
  s->size += g->tls_gotno * MIPS_ELF_GOT_SIZE (output_bfd);

  /* VxWorks does not support multiple GOTs.  It initializes $gp to
     __GOTT_BASE__[__GOTT_INDEX__], the value of which is set by the
     dynamic loader.  */
  if (htab->root.target_os != is_vxworks
      && s->size > MIPS_ELF_GOT_MAX_SIZE (info))
    {
      if (!mips_elf_multi_got (output_bfd, info, s, page_gotno))
	return false;
    }
  else
    {
      /* Record that all bfds use G.  This also has the effect of freeing
	 the per-bfd GOTs, which we no longer need.  */
      for (ibfd = info->input_bfds; ibfd; ibfd = ibfd->link.next)
	if (mips_elf_bfd_got (ibfd, false))
	  mips_elf_replace_bfd_got (ibfd, g);
      mips_elf_replace_bfd_got (output_bfd, g);

      /* Set up TLS entries.  */
      g->tls_assigned_gotno = g->global_gotno + g->local_gotno;
      tga.info = info;
      tga.g = g;
      tga.value = MIPS_ELF_GOT_SIZE (output_bfd);
      htab_traverse (g->got_entries, mips_elf_initialize_tls_index, &tga);
      if (!tga.g)
	return false;
      BFD_ASSERT (g->tls_assigned_gotno
		  == g->global_gotno + g->local_gotno + g->tls_gotno);

      /* Each VxWorks GOT entry needs an explicit relocation.  */
      if (htab->root.target_os == is_vxworks && bfd_link_pic (info))
	g->relocs += g->global_gotno + g->local_gotno - htab->reserved_gotno;

      /* Allocate room for the TLS relocations.  */
      if (g->relocs)
	mips_elf_allocate_dynamic_relocations (dynobj, info, g->relocs);
    }

  return true;
}

/* Estimate the size of the .MIPS.stubs section.  */

static void
mips_elf_estimate_stub_size (bfd *output_bfd, struct bfd_link_info *info)
{
  struct mips_elf_link_hash_table *htab;
  bfd_size_type dynsymcount;

  htab = mips_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);

  if (htab->lazy_stub_count == 0)
    return;

  /* IRIX rld assumes that a function stub isn't at the end of the .text
     section, so add a dummy entry to the end.  */
  htab->lazy_stub_count++;

  /* Get a worst-case estimate of the number of dynamic symbols needed.
     At this point, dynsymcount does not account for section symbols
     and count_section_dynsyms may overestimate the number that will
     be needed.  */
  dynsymcount = (elf_hash_table (info)->dynsymcount
		 + count_section_dynsyms (output_bfd, info));

  /* Determine the size of one stub entry.  There's no disadvantage
     from using microMIPS code here, so for the sake of pure-microMIPS
     binaries we prefer it whenever there's any microMIPS code in
     output produced at all.  This has a benefit of stubs being
     shorter by 4 bytes each too, unless in the insn32 mode.  */
  if (!MICROMIPS_P (output_bfd))
    htab->function_stub_size = (dynsymcount > 0x10000
				? MIPS_FUNCTION_STUB_BIG_SIZE
				: MIPS_FUNCTION_STUB_NORMAL_SIZE);
  else if (htab->insn32)
    htab->function_stub_size = (dynsymcount > 0x10000
				? MICROMIPS_INSN32_FUNCTION_STUB_BIG_SIZE
				: MICROMIPS_INSN32_FUNCTION_STUB_NORMAL_SIZE);
  else
    htab->function_stub_size = (dynsymcount > 0x10000
				? MICROMIPS_FUNCTION_STUB_BIG_SIZE
				: MICROMIPS_FUNCTION_STUB_NORMAL_SIZE);

  htab->sstubs->size = htab->lazy_stub_count * htab->function_stub_size;
}

/* A mips_elf_link_hash_traverse callback for which DATA points to a
   mips_htab_traverse_info.  If H needs a traditional MIPS lazy-binding
   stub, allocate an entry in the stubs section.  */

static bool
mips_elf_allocate_lazy_stub (struct mips_elf_link_hash_entry *h, void *data)
{
  struct mips_htab_traverse_info *hti = data;
  struct mips_elf_link_hash_table *htab;
  struct bfd_link_info *info;
  bfd *output_bfd;

  info = hti->info;
  output_bfd = hti->output_bfd;
  htab = mips_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);

  if (h->needs_lazy_stub)
    {
      bool micromips_p = MICROMIPS_P (output_bfd);
      unsigned int other = micromips_p ? STO_MICROMIPS : 0;
      bfd_vma isa_bit = micromips_p;

      BFD_ASSERT (htab->root.dynobj != NULL);
      if (h->root.plt.plist == NULL)
	h->root.plt.plist = mips_elf_make_plt_record (htab->sstubs->owner);
      if (h->root.plt.plist == NULL)
	{
	  hti->error = true;
	  return false;
	}
      h->root.root.u.def.section = htab->sstubs;
      h->root.root.u.def.value = htab->sstubs->size + isa_bit;
      h->root.plt.plist->stub_offset = htab->sstubs->size;
      h->root.other = other;
      htab->sstubs->size += htab->function_stub_size;
    }
  return true;
}

/* Allocate offsets in the stubs section to each symbol that needs one.
   Set the final size of the .MIPS.stub section.  */

static bool
mips_elf_lay_out_lazy_stubs (struct bfd_link_info *info)
{
  bfd *output_bfd = info->output_bfd;
  bool micromips_p = MICROMIPS_P (output_bfd);
  unsigned int other = micromips_p ? STO_MICROMIPS : 0;
  bfd_vma isa_bit = micromips_p;
  struct mips_elf_link_hash_table *htab;
  struct mips_htab_traverse_info hti;
  struct elf_link_hash_entry *h;
  bfd *dynobj;

  htab = mips_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);

  if (htab->lazy_stub_count == 0)
    return true;

  htab->sstubs->size = 0;
  hti.info = info;
  hti.output_bfd = output_bfd;
  hti.error = false;
  mips_elf_link_hash_traverse (htab, mips_elf_allocate_lazy_stub, &hti);
  if (hti.error)
    return false;
  htab->sstubs->size += htab->function_stub_size;
  BFD_ASSERT (htab->sstubs->size
	      == htab->lazy_stub_count * htab->function_stub_size);

  dynobj = elf_hash_table (info)->dynobj;
  BFD_ASSERT (dynobj != NULL);
  h = _bfd_elf_define_linkage_sym (dynobj, info, htab->sstubs, "_MIPS_STUBS_");
  if (h == NULL)
    return false;
  h->root.u.def.value = isa_bit;
  h->other = other;
  h->type = STT_FUNC;

  return true;
}

/* A mips_elf_link_hash_traverse callback for which DATA points to a
   bfd_link_info.  If H uses the address of a PLT entry as the value
   of the symbol, then set the entry in the symbol table now.  Prefer
   a standard MIPS PLT entry.  */

static bool
mips_elf_set_plt_sym_value (struct mips_elf_link_hash_entry *h, void *data)
{
  struct bfd_link_info *info = data;
  bool micromips_p = MICROMIPS_P (info->output_bfd);
  struct mips_elf_link_hash_table *htab;
  unsigned int other;
  bfd_vma isa_bit;
  bfd_vma val;

  htab = mips_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);

  if (h->use_plt_entry)
    {
      BFD_ASSERT (h->root.plt.plist != NULL);
      BFD_ASSERT (h->root.plt.plist->mips_offset != MINUS_ONE
		  || h->root.plt.plist->comp_offset != MINUS_ONE);

      val = htab->plt_header_size;
      if (h->root.plt.plist->mips_offset != MINUS_ONE)
	{
	  isa_bit = 0;
	  val += h->root.plt.plist->mips_offset;
	  other = 0;
	}
      else
	{
	  isa_bit = 1;
	  val += htab->plt_mips_offset + h->root.plt.plist->comp_offset;
	  other = micromips_p ? STO_MICROMIPS : STO_MIPS16;
	}
      val += isa_bit;
      /* For VxWorks, point at the PLT load stub rather than the lazy
	 resolution stub; this stub will become the canonical function
	 address.  */
      if (htab->root.target_os == is_vxworks)
	val += 8;

      h->root.root.u.def.section = htab->root.splt;
      h->root.root.u.def.value = val;
      h->root.other = other;
    }

  return true;
}

/* Set the sizes of the dynamic sections.  */

bool
_bfd_mips_elf_size_dynamic_sections (bfd *output_bfd,
				     struct bfd_link_info *info)
{
  bfd *dynobj;
  asection *s, *sreldyn;
  bool reltext;
  struct mips_elf_link_hash_table *htab;

  htab = mips_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);
  dynobj = elf_hash_table (info)->dynobj;
  BFD_ASSERT (dynobj != NULL);

  if (elf_hash_table (info)->dynamic_sections_created)
    {
      /* Set the contents of the .interp section to the interpreter.  */
      if (bfd_link_executable (info) && !info->nointerp)
	{
	  s = bfd_get_linker_section (dynobj, ".interp");
	  BFD_ASSERT (s != NULL);
	  s->size
	    = strlen (ELF_DYNAMIC_INTERPRETER (output_bfd)) + 1;
	  s->contents
	    = (bfd_byte *) ELF_DYNAMIC_INTERPRETER (output_bfd);
	}

      /* Figure out the size of the PLT header if we know that we
	 are using it.  For the sake of cache alignment always use
	 a standard header whenever any standard entries are present
	 even if microMIPS entries are present as well.  This also
	 lets the microMIPS header rely on the value of $v0 only set
	 by microMIPS entries, for a small size reduction.

	 Set symbol table entry values for symbols that use the
	 address of their PLT entry now that we can calculate it.

	 Also create the _PROCEDURE_LINKAGE_TABLE_ symbol if we
	 haven't already in _bfd_elf_create_dynamic_sections.  */
      if (htab->root.splt && htab->plt_mips_offset + htab->plt_comp_offset != 0)
	{
	  bool micromips_p = (MICROMIPS_P (output_bfd)
				     && !htab->plt_mips_offset);
	  unsigned int other = micromips_p ? STO_MICROMIPS : 0;
	  bfd_vma isa_bit = micromips_p;
	  struct elf_link_hash_entry *h;
	  bfd_vma size;

	  BFD_ASSERT (htab->use_plts_and_copy_relocs);
	  BFD_ASSERT (htab->root.sgotplt->size == 0);
	  BFD_ASSERT (htab->root.splt->size == 0);

	  if (htab->root.target_os == is_vxworks && bfd_link_pic (info))
	    size = 4 * ARRAY_SIZE (mips_vxworks_shared_plt0_entry);
	  else if (htab->root.target_os == is_vxworks)
	    size = 4 * ARRAY_SIZE (mips_vxworks_exec_plt0_entry);
	  else if (ABI_64_P (output_bfd))
	    size = 4 * ARRAY_SIZE (mips_n64_exec_plt0_entry);
	  else if (ABI_N32_P (output_bfd))
	    size = 4 * ARRAY_SIZE (mips_n32_exec_plt0_entry);
	  else if (!micromips_p)
	    size = 4 * ARRAY_SIZE (mips_o32_exec_plt0_entry);
	  else if (htab->insn32)
	    size = 2 * ARRAY_SIZE (micromips_insn32_o32_exec_plt0_entry);
	  else
	    size = 2 * ARRAY_SIZE (micromips_o32_exec_plt0_entry);

	  htab->plt_header_is_comp = micromips_p;
	  htab->plt_header_size = size;
	  htab->root.splt->size = (size
				   + htab->plt_mips_offset
				   + htab->plt_comp_offset);
	  htab->root.sgotplt->size = (htab->plt_got_index
				      * MIPS_ELF_GOT_SIZE (dynobj));

	  mips_elf_link_hash_traverse (htab, mips_elf_set_plt_sym_value, info);

	  if (htab->root.hplt == NULL)
	    {
	      h = _bfd_elf_define_linkage_sym (dynobj, info, htab->root.splt,
					       "_PROCEDURE_LINKAGE_TABLE_");
	      htab->root.hplt = h;
	      if (h == NULL)
		return false;
	    }

	  h = htab->root.hplt;
	  h->root.u.def.value = isa_bit;
	  h->other = other;
	  h->type = STT_FUNC;
	}
    }

  /* Allocate space for global sym dynamic relocs.  */
  elf_link_hash_traverse (&htab->root, allocate_dynrelocs, info);

  mips_elf_estimate_stub_size (output_bfd, info);

  if (!mips_elf_lay_out_got (output_bfd, info))
    return false;

  mips_elf_lay_out_lazy_stubs (info);

  /* The check_relocs and adjust_dynamic_symbol entry points have
     determined the sizes of the various dynamic sections.  Allocate
     memory for them.  */
  reltext = false;
  for (s = dynobj->sections; s != NULL; s = s->next)
    {
      const char *name;

      /* It's OK to base decisions on the section name, because none
	 of the dynobj section names depend upon the input files.  */
      name = bfd_section_name (s);

      if ((s->flags & SEC_LINKER_CREATED) == 0)
	continue;

      if (startswith (name, ".rel"))
	{
	  if (s->size != 0)
	    {
	      const char *outname;
	      asection *target;

	      /* If this relocation section applies to a read only
		 section, then we probably need a DT_TEXTREL entry.
		 If the relocation section is .rel(a).dyn, we always
		 assert a DT_TEXTREL entry rather than testing whether
		 there exists a relocation to a read only section or
		 not.  */
	      outname = bfd_section_name (s->output_section);
	      target = bfd_get_section_by_name (output_bfd, outname + 4);
	      if ((target != NULL
		   && (target->flags & SEC_READONLY) != 0
		   && (target->flags & SEC_ALLOC) != 0)
		  || strcmp (outname, MIPS_ELF_REL_DYN_NAME (info)) == 0)
		reltext = true;

	      /* We use the reloc_count field as a counter if we need
		 to copy relocs into the output file.  */
	      if (strcmp (name, MIPS_ELF_REL_DYN_NAME (info)) != 0)
		s->reloc_count = 0;

	      /* If combreloc is enabled, elf_link_sort_relocs() will
		 sort relocations, but in a different way than we do,
		 and before we're done creating relocations.  Also, it
		 will move them around between input sections'
		 relocation's contents, so our sorting would be
		 broken, so don't let it run.  */
	      info->combreloc = 0;
	    }
	}
      else if (bfd_link_executable (info)
	       && ! mips_elf_hash_table (info)->use_rld_obj_head
	       && startswith (name, ".rld_map"))
	{
	  /* We add a room for __rld_map.  It will be filled in by the
	     rtld to contain a pointer to the _r_debug structure.  */
	  s->size += MIPS_ELF_RLD_MAP_SIZE (output_bfd);
	}
      else if (SGI_COMPAT (output_bfd)
	       && startswith (name, ".compact_rel"))
	s->size += mips_elf_hash_table (info)->compact_rel_size;
      else if (s == htab->root.splt)
	{
	  /* If the last PLT entry has a branch delay slot, allocate
	     room for an extra nop to fill the delay slot.  This is
	     for CPUs without load interlocking.  */
	  if (! LOAD_INTERLOCKS_P (output_bfd)
	      && htab->root.target_os != is_vxworks
	      && s->size > 0)
	    s->size += 4;
	}
      else if (! startswith (name, ".init")
	       && s != htab->root.sgot
	       && s != htab->root.sgotplt
	       && s != htab->sstubs
	       && s != htab->root.sdynbss
	       && s != htab->root.sdynrelro)
	{
	  /* It's not one of our sections, so don't allocate space.  */
	  continue;
	}

      if (s->size == 0)
	{
	  s->flags |= SEC_EXCLUDE;
	  continue;
	}

      if ((s->flags & SEC_HAS_CONTENTS) == 0)
	continue;

      /* Allocate memory for the section contents.  */
      s->contents = bfd_zalloc (dynobj, s->size);
      if (s->contents == NULL)
	{
	  bfd_set_error (bfd_error_no_memory);
	  return false;
	}
    }

  if (elf_hash_table (info)->dynamic_sections_created)
    {
      /* Add some entries to the .dynamic section.  We fill in the
	 values later, in _bfd_mips_elf_finish_dynamic_sections, but we
	 must add the entries now so that we get the correct size for
	 the .dynamic section.  */

      /* SGI object has the equivalence of DT_DEBUG in the
	 DT_MIPS_RLD_MAP entry.  This must come first because glibc
	 only fills in DT_MIPS_RLD_MAP (not DT_DEBUG) and some tools
	 may only look at the first one they see.  */
      if (!bfd_link_pic (info)
	  && !MIPS_ELF_ADD_DYNAMIC_ENTRY (info, DT_MIPS_RLD_MAP, 0))
	return false;

      if (bfd_link_executable (info)
	  && !MIPS_ELF_ADD_DYNAMIC_ENTRY (info, DT_MIPS_RLD_MAP_REL, 0))
	return false;

      /* The DT_DEBUG entry may be filled in by the dynamic linker and
	 used by the debugger.  */
      if (bfd_link_executable (info)
	  && !SGI_COMPAT (output_bfd)
	  && !MIPS_ELF_ADD_DYNAMIC_ENTRY (info, DT_DEBUG, 0))
	return false;

      if (reltext
	  && (SGI_COMPAT (output_bfd)
	      || htab->root.target_os == is_vxworks))
	info->flags |= DF_TEXTREL;

      if ((info->flags & DF_TEXTREL) != 0)
	{
	  if (! MIPS_ELF_ADD_DYNAMIC_ENTRY (info, DT_TEXTREL, 0))
	    return false;

	  /* Clear the DF_TEXTREL flag.  It will be set again if we
	     write out an actual text relocation; we may not, because
	     at this point we do not know whether e.g. any .eh_frame
	     absolute relocations have been converted to PC-relative.  */
	  info->flags &= ~DF_TEXTREL;
	}

      if (! MIPS_ELF_ADD_DYNAMIC_ENTRY (info, DT_PLTGOT, 0))
	return false;

      sreldyn = mips_elf_rel_dyn_section (info, false);
      if (htab->root.target_os == is_vxworks)
	{
	  /* VxWorks uses .rela.dyn instead of .rel.dyn.  It does not
	     use any of the DT_MIPS_* tags.  */
	  if (sreldyn && sreldyn->size > 0)
	    {
	      if (! MIPS_ELF_ADD_DYNAMIC_ENTRY (info, DT_RELA, 0))
		return false;

	      if (! MIPS_ELF_ADD_DYNAMIC_ENTRY (info, DT_RELASZ, 0))
		return false;

	      if (! MIPS_ELF_ADD_DYNAMIC_ENTRY (info, DT_RELAENT, 0))
		return false;
	    }
	}
      else
	{
	  if (sreldyn && sreldyn->size > 0
	      && !bfd_is_abs_section (sreldyn->output_section))
	    {
	      if (! MIPS_ELF_ADD_DYNAMIC_ENTRY (info, DT_REL, 0))
		return false;

	      if (! MIPS_ELF_ADD_DYNAMIC_ENTRY (info, DT_RELSZ, 0))
		return false;

	      if (! MIPS_ELF_ADD_DYNAMIC_ENTRY (info, DT_RELENT, 0))
		return false;
	    }

	  if (! MIPS_ELF_ADD_DYNAMIC_ENTRY (info, DT_MIPS_RLD_VERSION, 0))
	    return false;

	  if (! MIPS_ELF_ADD_DYNAMIC_ENTRY (info, DT_MIPS_FLAGS, 0))
	    return false;

	  if (! MIPS_ELF_ADD_DYNAMIC_ENTRY (info, DT_MIPS_BASE_ADDRESS, 0))
	    return false;

	  if (! MIPS_ELF_ADD_DYNAMIC_ENTRY (info, DT_MIPS_LOCAL_GOTNO, 0))
	    return false;

	  if (! MIPS_ELF_ADD_DYNAMIC_ENTRY (info, DT_MIPS_SYMTABNO, 0))
	    return false;

	  if (! MIPS_ELF_ADD_DYNAMIC_ENTRY (info, DT_MIPS_UNREFEXTNO, 0))
	    return false;

	  if (! MIPS_ELF_ADD_DYNAMIC_ENTRY (info, DT_MIPS_GOTSYM, 0))
	    return false;

	  if (info->emit_gnu_hash
	      && ! MIPS_ELF_ADD_DYNAMIC_ENTRY (info, DT_MIPS_XHASH, 0))
	    return false;

	  if (IRIX_COMPAT (dynobj) == ict_irix5
	      && ! MIPS_ELF_ADD_DYNAMIC_ENTRY (info, DT_MIPS_HIPAGENO, 0))
	    return false;

	  if (IRIX_COMPAT (dynobj) == ict_irix6
	      && (bfd_get_section_by_name
		  (output_bfd, MIPS_ELF_OPTIONS_SECTION_NAME (dynobj)))
	      && !MIPS_ELF_ADD_DYNAMIC_ENTRY (info, DT_MIPS_OPTIONS, 0))
	    return false;
	}
      if (htab->root.splt->size > 0)
	{
	  if (! MIPS_ELF_ADD_DYNAMIC_ENTRY (info, DT_PLTREL, 0))
	    return false;

	  if (! MIPS_ELF_ADD_DYNAMIC_ENTRY (info, DT_JMPREL, 0))
	    return false;

	  if (! MIPS_ELF_ADD_DYNAMIC_ENTRY (info, DT_PLTRELSZ, 0))
	    return false;

	  if (! MIPS_ELF_ADD_DYNAMIC_ENTRY (info, DT_MIPS_PLTGOT, 0))
	    return false;
	}
      if (htab->root.target_os == is_vxworks
	  && !elf_vxworks_add_dynamic_entries (output_bfd, info))
	return false;
    }

  return true;
}

/* REL is a relocation in INPUT_BFD that is being copied to OUTPUT_BFD.
   Adjust its R_ADDEND field so that it is correct for the output file.
   LOCAL_SYMS and LOCAL_SECTIONS are arrays of INPUT_BFD's local symbols
   and sections respectively; both use symbol indexes.  */

static void
mips_elf_adjust_addend (bfd *output_bfd, struct bfd_link_info *info,
			bfd *input_bfd, Elf_Internal_Sym *local_syms,
			asection **local_sections, Elf_Internal_Rela *rel)
{
  unsigned int r_type, r_symndx;
  Elf_Internal_Sym *sym;
  asection *sec;

  if (mips_elf_local_relocation_p (input_bfd, rel, local_sections))
    {
      r_type = ELF_R_TYPE (output_bfd, rel->r_info);
      if (gprel16_reloc_p (r_type)
	  || r_type == R_MIPS_GPREL32
	  || literal_reloc_p (r_type))
	{
	  rel->r_addend += _bfd_get_gp_value (input_bfd);
	  rel->r_addend -= _bfd_get_gp_value (output_bfd);
	}

      r_symndx = ELF_R_SYM (output_bfd, rel->r_info);
      sym = local_syms + r_symndx;

      /* Adjust REL's addend to account for section merging.  */
      if (!bfd_link_relocatable (info))
	{
	  sec = local_sections[r_symndx];
	  _bfd_elf_rela_local_sym (output_bfd, sym, &sec, rel);
	}

      /* This would normally be done by the rela_normal code in elflink.c.  */
      if (ELF_ST_TYPE (sym->st_info) == STT_SECTION)
	rel->r_addend += local_sections[r_symndx]->output_offset;
    }
}

/* Handle relocations against symbols from removed linkonce sections,
   or sections discarded by a linker script.  We use this wrapper around
   RELOC_AGAINST_DISCARDED_SECTION to handle triplets of compound relocs
   on 64-bit ELF targets.  In this case for any relocation handled, which
   always be the first in a triplet, the remaining two have to be processed
   together with the first, even if they are R_MIPS_NONE.  It is the symbol
   index referred by the first reloc that applies to all the three and the
   remaining two never refer to an object symbol.  And it is the final
   relocation (the last non-null one) that determines the output field of
   the whole relocation so retrieve the corresponding howto structure for
   the relocatable field to be cleared by RELOC_AGAINST_DISCARDED_SECTION.

   Note that RELOC_AGAINST_DISCARDED_SECTION is a macro that uses "continue"
   and therefore requires to be pasted in a loop.  It also defines a block
   and does not protect any of its arguments, hence the extra brackets.  */

static void
mips_reloc_against_discarded_section (bfd *output_bfd,
				      struct bfd_link_info *info,
				      bfd *input_bfd, asection *input_section,
				      Elf_Internal_Rela **rel,
				      const Elf_Internal_Rela **relend,
				      bool rel_reloc,
				      reloc_howto_type *howto,
				      bfd_byte *contents)
{
  const struct elf_backend_data *bed = get_elf_backend_data (output_bfd);
  int count = bed->s->int_rels_per_ext_rel;
  unsigned int r_type;
  int i;

  for (i = count - 1; i > 0; i--)
    {
      r_type = ELF_R_TYPE (output_bfd, (*rel)[i].r_info);
      if (r_type != R_MIPS_NONE)
	{
	  howto = MIPS_ELF_RTYPE_TO_HOWTO (input_bfd, r_type, !rel_reloc);
	  break;
	}
    }
  do
    {
       RELOC_AGAINST_DISCARDED_SECTION (info, input_bfd, input_section,
					(*rel), count, (*relend),
					howto, i, contents);
    }
  while (0);
}

/* Relocate a MIPS ELF section.  */

int
_bfd_mips_elf_relocate_section (bfd *output_bfd, struct bfd_link_info *info,
				bfd *input_bfd, asection *input_section,
				bfd_byte *contents, Elf_Internal_Rela *relocs,
				Elf_Internal_Sym *local_syms,
				asection **local_sections)
{
  Elf_Internal_Rela *rel;
  const Elf_Internal_Rela *relend;
  bfd_vma addend = 0;
  bool use_saved_addend_p = false;

  relend = relocs + input_section->reloc_count;
  for (rel = relocs; rel < relend; ++rel)
    {
      const char *name;
      bfd_vma value = 0;
      reloc_howto_type *howto;
      bool cross_mode_jump_p = false;
      /* TRUE if the relocation is a RELA relocation, rather than a
	 REL relocation.  */
      bool rela_relocation_p = true;
      unsigned int r_type = ELF_R_TYPE (output_bfd, rel->r_info);
      const char *msg;
      unsigned long r_symndx;
      asection *sec;
      Elf_Internal_Shdr *symtab_hdr;
      struct elf_link_hash_entry *h;
      bool rel_reloc;

      rel_reloc = (NEWABI_P (input_bfd)
		   && mips_elf_rel_relocation_p (input_bfd, input_section,
						 relocs, rel));
      /* Find the relocation howto for this relocation.  */
      howto = MIPS_ELF_RTYPE_TO_HOWTO (input_bfd, r_type, !rel_reloc);

      r_symndx = ELF_R_SYM (input_bfd, rel->r_info);
      symtab_hdr = &elf_tdata (input_bfd)->symtab_hdr;
      if (mips_elf_local_relocation_p (input_bfd, rel, local_sections))
	{
	  sec = local_sections[r_symndx];
	  h = NULL;
	}
      else
	{
	  unsigned long extsymoff;

	  extsymoff = 0;
	  if (!elf_bad_symtab (input_bfd))
	    extsymoff = symtab_hdr->sh_info;
	  h = elf_sym_hashes (input_bfd) [r_symndx - extsymoff];
	  while (h->root.type == bfd_link_hash_indirect
		 || h->root.type == bfd_link_hash_warning)
	    h = (struct elf_link_hash_entry *) h->root.u.i.link;

	  sec = NULL;
	  if (h->root.type == bfd_link_hash_defined
	      || h->root.type == bfd_link_hash_defweak)
	    sec = h->root.u.def.section;
	}

      if (sec != NULL && discarded_section (sec))
	{
	  mips_reloc_against_discarded_section (output_bfd, info, input_bfd,
						input_section, &rel, &relend,
						rel_reloc, howto, contents);
	  continue;
	}

      if (r_type == R_MIPS_64 && ! NEWABI_P (input_bfd))
	{
	  /* Some 32-bit code uses R_MIPS_64.  In particular, people use
	     64-bit code, but make sure all their addresses are in the
	     lowermost or uppermost 32-bit section of the 64-bit address
	     space.  Thus, when they use an R_MIPS_64 they mean what is
	     usually meant by R_MIPS_32, with the exception that the
	     stored value is sign-extended to 64 bits.  */
	  howto = MIPS_ELF_RTYPE_TO_HOWTO (input_bfd, R_MIPS_32, false);

	  /* On big-endian systems, we need to lie about the position
	     of the reloc.  */
	  if (bfd_big_endian (input_bfd))
	    rel->r_offset += 4;
	}

      if (!use_saved_addend_p)
	{
	  /* If these relocations were originally of the REL variety,
	     we must pull the addend out of the field that will be
	     relocated.  Otherwise, we simply use the contents of the
	     RELA relocation.  */
	  if (mips_elf_rel_relocation_p (input_bfd, input_section,
					 relocs, rel))
	    {
	      rela_relocation_p = false;
	      addend = mips_elf_read_rel_addend (input_bfd, input_section,
						 rel, howto, contents);
	      if (hi16_reloc_p (r_type)
		  || (got16_reloc_p (r_type)
		      && mips_elf_local_relocation_p (input_bfd, rel,
						      local_sections)))
		{
		  if (!mips_elf_add_lo16_rel_addend (input_bfd, input_section,
						     rel, relend,
						     contents, &addend))
		    {
		      if (h)
			name = h->root.root.string;
		      else
			name = bfd_elf_sym_name (input_bfd, symtab_hdr,
						 local_syms + r_symndx,
						 sec);
		      _bfd_error_handler
			/* xgettext:c-format */
			(_("%pB: can't find matching LO16 reloc against `%s'"
			   " for %s at %#" PRIx64 " in section `%pA'"),
			 input_bfd, name,
			 howto->name, (uint64_t) rel->r_offset, input_section);
		    }
		}
	      else
		addend <<= howto->rightshift;
	    }
	  else
	    addend = rel->r_addend;
	  mips_elf_adjust_addend (output_bfd, info, input_bfd,
				  local_syms, local_sections, rel);
	}

      if (bfd_link_relocatable (info))
	{
	  if (r_type == R_MIPS_64 && ! NEWABI_P (output_bfd)
	      && bfd_big_endian (input_bfd))
	    rel->r_offset -= 4;

	  if (!rela_relocation_p && rel->r_addend)
	    {
	      addend += rel->r_addend;
	      if (hi16_reloc_p (r_type) || got16_reloc_p (r_type))
		addend = mips_elf_high (addend);
	      else if (r_type == R_MIPS_HIGHER)
		addend = mips_elf_higher (addend);
	      else if (r_type == R_MIPS_HIGHEST)
		addend = mips_elf_highest (addend);
	      else
		addend >>= howto->rightshift;

	      /* We use the source mask, rather than the destination
		 mask because the place to which we are writing will be
		 source of the addend in the final link.  */
	      addend &= howto->src_mask;

	      if (r_type == R_MIPS_64 && ! NEWABI_P (output_bfd))
		/* See the comment above about using R_MIPS_64 in the 32-bit
		   ABI.  Here, we need to update the addend.  It would be
		   possible to get away with just using the R_MIPS_32 reloc
		   but for endianness.  */
		{
		  bfd_vma sign_bits;
		  bfd_vma low_bits;
		  bfd_vma high_bits;

		  if (addend & ((bfd_vma) 1 << 31))
#ifdef BFD64
		    sign_bits = ((bfd_vma) 1 << 32) - 1;
#else
		    sign_bits = -1;
#endif
		  else
		    sign_bits = 0;

		  /* If we don't know that we have a 64-bit type,
		     do two separate stores.  */
		  if (bfd_big_endian (input_bfd))
		    {
		      /* Store the sign-bits (which are most significant)
			 first.  */
		      low_bits = sign_bits;
		      high_bits = addend;
		    }
		  else
		    {
		      low_bits = addend;
		      high_bits = sign_bits;
		    }
		  bfd_put_32 (input_bfd, low_bits,
			      contents + rel->r_offset);
		  bfd_put_32 (input_bfd, high_bits,
			      contents + rel->r_offset + 4);
		  continue;
		}

	      if (! mips_elf_perform_relocation (info, howto, rel, addend,
						 input_bfd, input_section,
						 contents, false))
		return false;
	    }

	  /* Go on to the next relocation.  */
	  continue;
	}

      /* In the N32 and 64-bit ABIs there may be multiple consecutive
	 relocations for the same offset.  In that case we are
	 supposed to treat the output of each relocation as the addend
	 for the next.  */
      if (rel + 1 < relend
	  && rel->r_offset == rel[1].r_offset
	  && ELF_R_TYPE (input_bfd, rel[1].r_info) != R_MIPS_NONE)
	use_saved_addend_p = true;
      else
	use_saved_addend_p = false;

      /* Figure out what value we are supposed to relocate.  */
      switch (mips_elf_calculate_relocation (output_bfd, input_bfd,
					     input_section, contents,
					     info, rel, addend, howto,
					     local_syms, local_sections,
					     &value, &name, &cross_mode_jump_p,
					     use_saved_addend_p))
	{
	case bfd_reloc_continue:
	  /* There's nothing to do.  */
	  continue;

	case bfd_reloc_undefined:
	  /* mips_elf_calculate_relocation already called the
	     undefined_symbol callback.  There's no real point in
	     trying to perform the relocation at this point, so we
	     just skip ahead to the next relocation.  */
	  continue;

	case bfd_reloc_notsupported:
	  msg = _("internal error: unsupported relocation error");
	  info->callbacks->warning
	    (info, msg, name, input_bfd, input_section, rel->r_offset);
	  return false;

	case bfd_reloc_overflow:
	  if (use_saved_addend_p)
	    /* Ignore overflow until we reach the last relocation for
	       a given location.  */
	    ;
	  else
	    {
	      struct mips_elf_link_hash_table *htab;

	      htab = mips_elf_hash_table (info);
	      BFD_ASSERT (htab != NULL);
	      BFD_ASSERT (name != NULL);
	      if (!htab->small_data_overflow_reported
		  && (gprel16_reloc_p (howto->type)
		      || literal_reloc_p (howto->type)))
		{
		  msg = _("small-data section exceeds 64KB;"
			  " lower small-data size limit (see option -G)");

		  htab->small_data_overflow_reported = true;
		  (*info->callbacks->einfo) ("%P: %s\n", msg);
		}
	      (*info->callbacks->reloc_overflow)
		(info, NULL, name, howto->name, (bfd_vma) 0,
		 input_bfd, input_section, rel->r_offset);
	    }
	  break;

	case bfd_reloc_ok:
	  break;

	case bfd_reloc_outofrange:
	  msg = NULL;
	  if (jal_reloc_p (howto->type))
	    msg = (cross_mode_jump_p
		   ? _("cannot convert a jump to JALX "
		       "for a non-word-aligned address")
		   : (howto->type == R_MIPS16_26
		      ? _("jump to a non-word-aligned address")
		      : _("jump to a non-instruction-aligned address")));
	  else if (b_reloc_p (howto->type))
	    msg = (cross_mode_jump_p
		   ? _("cannot convert a branch to JALX "
		       "for a non-word-aligned address")
		   : _("branch to a non-instruction-aligned address"));
	  else if (aligned_pcrel_reloc_p (howto->type))
	    msg = _("PC-relative load from unaligned address");
	  if (msg)
	    {
	      info->callbacks->einfo
		("%X%H: %s\n", input_bfd, input_section, rel->r_offset, msg);
	      break;
	    }
	  /* Fall through.  */

	default:
	  abort ();
	  break;
	}

      /* If we've got another relocation for the address, keep going
	 until we reach the last one.  */
      if (use_saved_addend_p)
	{
	  addend = value;
	  continue;
	}

      if (r_type == R_MIPS_64 && ! NEWABI_P (output_bfd))
	/* See the comment above about using R_MIPS_64 in the 32-bit
	   ABI.  Until now, we've been using the HOWTO for R_MIPS_32;
	   that calculated the right value.  Now, however, we
	   sign-extend the 32-bit result to 64-bits, and store it as a
	   64-bit value.  We are especially generous here in that we
	   go to extreme lengths to support this usage on systems with
	   only a 32-bit VMA.  */
	{
	  bfd_vma sign_bits;
	  bfd_vma low_bits;
	  bfd_vma high_bits;

	  if (value & ((bfd_vma) 1 << 31))
#ifdef BFD64
	    sign_bits = ((bfd_vma) 1 << 32) - 1;
#else
	    sign_bits = -1;
#endif
	  else
	    sign_bits = 0;

	  /* If we don't know that we have a 64-bit type,
	     do two separate stores.  */
	  if (bfd_big_endian (input_bfd))
	    {
	      /* Undo what we did above.  */
	      rel->r_offset -= 4;
	      /* Store the sign-bits (which are most significant)
		 first.  */
	      low_bits = sign_bits;
	      high_bits = value;
	    }
	  else
	    {
	      low_bits = value;
	      high_bits = sign_bits;
	    }
	  bfd_put_32 (input_bfd, low_bits,
		      contents + rel->r_offset);
	  bfd_put_32 (input_bfd, high_bits,
		      contents + rel->r_offset + 4);
	  continue;
	}

      /* Actually perform the relocation.  */
      if (! mips_elf_perform_relocation (info, howto, rel, value,
					 input_bfd, input_section,
					 contents, cross_mode_jump_p))
	return false;
    }

  return true;
}

/* A function that iterates over each entry in la25_stubs and fills
   in the code for each one.  DATA points to a mips_htab_traverse_info.  */

static int
mips_elf_create_la25_stub (void **slot, void *data)
{
  struct mips_htab_traverse_info *hti;
  struct mips_elf_link_hash_table *htab;
  struct mips_elf_la25_stub *stub;
  asection *s;
  bfd_byte *loc;
  bfd_vma offset, target, target_high, target_low;
  bfd_vma branch_pc;
  bfd_signed_vma pcrel_offset = 0;

  stub = (struct mips_elf_la25_stub *) *slot;
  hti = (struct mips_htab_traverse_info *) data;
  htab = mips_elf_hash_table (hti->info);
  BFD_ASSERT (htab != NULL);

  /* Create the section contents, if we haven't already.  */
  s = stub->stub_section;
  loc = s->contents;
  if (loc == NULL)
    {
      loc = bfd_malloc (s->size);
      if (loc == NULL)
	{
	  hti->error = true;
	  return false;
	}
      s->contents = loc;
    }

  /* Work out where in the section this stub should go.  */
  offset = stub->offset;

  /* We add 8 here to account for the LUI/ADDIU instructions
     before the branch instruction.  This cannot be moved down to
     where pcrel_offset is calculated as 's' is updated in
     mips_elf_get_la25_target.  */
  branch_pc = s->output_section->vma + s->output_offset + offset + 8;

  /* Work out the target address.  */
  target = mips_elf_get_la25_target (stub, &s);
  target += s->output_section->vma + s->output_offset;

  target_high = ((target + 0x8000) >> 16) & 0xffff;
  target_low = (target & 0xffff);

  /* Calculate the PC of the compact branch instruction (for the case where
     compact branches are used for either microMIPSR6 or MIPSR6 with
     compact branches.  Add 4-bytes to account for BC using the PC of the
     next instruction as the base.  */
  pcrel_offset = target - (branch_pc + 4);

  if (stub->stub_section != htab->strampoline)
    {
      /* This is a simple LUI/ADDIU stub.  Zero out the beginning
	 of the section and write the two instructions at the end.  */
      memset (loc, 0, offset);
      loc += offset;
      if (ELF_ST_IS_MICROMIPS (stub->h->root.other))
	{
	  bfd_put_micromips_32 (hti->output_bfd,
				LA25_LUI_MICROMIPS (target_high),
				loc);
	  bfd_put_micromips_32 (hti->output_bfd,
				LA25_ADDIU_MICROMIPS (target_low),
				loc + 4);
	}
      else
	{
	  bfd_put_32 (hti->output_bfd, LA25_LUI (target_high), loc);
	  bfd_put_32 (hti->output_bfd, LA25_ADDIU (target_low), loc + 4);
	}
    }
  else
    {
      /* This is trampoline.  */
      loc += offset;
      if (ELF_ST_IS_MICROMIPS (stub->h->root.other))
	{
	  bfd_put_micromips_32 (hti->output_bfd,
				LA25_LUI_MICROMIPS (target_high), loc);
	  bfd_put_micromips_32 (hti->output_bfd,
				LA25_J_MICROMIPS (target), loc + 4);
	  bfd_put_micromips_32 (hti->output_bfd,
				LA25_ADDIU_MICROMIPS (target_low), loc + 8);
	  bfd_put_32 (hti->output_bfd, 0, loc + 12);
	}
      else
	{
	  bfd_put_32 (hti->output_bfd, LA25_LUI (target_high), loc);
	  if (MIPSR6_P (hti->output_bfd) && htab->compact_branches)
	    {
	      bfd_put_32 (hti->output_bfd, LA25_ADDIU (target_low), loc + 4);
	      bfd_put_32 (hti->output_bfd, LA25_BC (pcrel_offset), loc + 8);
	    }
	  else
	    {
	      bfd_put_32 (hti->output_bfd, LA25_J (target), loc + 4);
	      bfd_put_32 (hti->output_bfd, LA25_ADDIU (target_low), loc + 8);
	    }
	  bfd_put_32 (hti->output_bfd, 0, loc + 12);
	}
    }
  return true;
}

/* If NAME is one of the special IRIX6 symbols defined by the linker,
   adjust it appropriately now.  */

static void
mips_elf_irix6_finish_dynamic_symbol (bfd *abfd ATTRIBUTE_UNUSED,
				      const char *name, Elf_Internal_Sym *sym)
{
  /* The linker script takes care of providing names and values for
     these, but we must place them into the right sections.  */
  static const char* const text_section_symbols[] = {
    "_ftext",
    "_etext",
    "__dso_displacement",
    "__elf_header",
    "__program_header_table",
    NULL
  };

  static const char* const data_section_symbols[] = {
    "_fdata",
    "_edata",
    "_end",
    "_fbss",
    NULL
  };

  const char* const *p;
  int i;

  for (i = 0; i < 2; ++i)
    for (p = (i == 0) ? text_section_symbols : data_section_symbols;
	 *p;
	 ++p)
      if (strcmp (*p, name) == 0)
	{
	  /* All of these symbols are given type STT_SECTION by the
	     IRIX6 linker.  */
	  sym->st_info = ELF_ST_INFO (STB_GLOBAL, STT_SECTION);
	  sym->st_other = STO_PROTECTED;

	  /* The IRIX linker puts these symbols in special sections.  */
	  if (i == 0)
	    sym->st_shndx = SHN_MIPS_TEXT;
	  else
	    sym->st_shndx = SHN_MIPS_DATA;

	  break;
	}
}

/* Finish up dynamic symbol handling.  We set the contents of various
   dynamic sections here.  */

bool
_bfd_mips_elf_finish_dynamic_symbol (bfd *output_bfd,
				     struct bfd_link_info *info,
				     struct elf_link_hash_entry *h,
				     Elf_Internal_Sym *sym)
{
  bfd *dynobj;
  asection *sgot;
  struct mips_got_info *g, *gg;
  const char *name;
  int idx;
  struct mips_elf_link_hash_table *htab;
  struct mips_elf_link_hash_entry *hmips;

  htab = mips_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);
  dynobj = elf_hash_table (info)->dynobj;
  hmips = (struct mips_elf_link_hash_entry *) h;

  BFD_ASSERT (htab->root.target_os != is_vxworks);

  if (h->plt.plist != NULL
      && (h->plt.plist->mips_offset != MINUS_ONE
	  || h->plt.plist->comp_offset != MINUS_ONE))
    {
      /* We've decided to create a PLT entry for this symbol.  */
      bfd_byte *loc;
      bfd_vma header_address, got_address;
      bfd_vma got_address_high, got_address_low, load;
      bfd_vma got_index;
      bfd_vma isa_bit;

      got_index = h->plt.plist->gotplt_index;

      BFD_ASSERT (htab->use_plts_and_copy_relocs);
      BFD_ASSERT (h->dynindx != -1);
      BFD_ASSERT (htab->root.splt != NULL);
      BFD_ASSERT (got_index != MINUS_ONE);
      BFD_ASSERT (!h->def_regular);

      /* Calculate the address of the PLT header.  */
      isa_bit = htab->plt_header_is_comp;
      header_address = (htab->root.splt->output_section->vma
			+ htab->root.splt->output_offset + isa_bit);

      /* Calculate the address of the .got.plt entry.  */
      got_address = (htab->root.sgotplt->output_section->vma
		     + htab->root.sgotplt->output_offset
		     + got_index * MIPS_ELF_GOT_SIZE (dynobj));

      got_address_high = ((got_address + 0x8000) >> 16) & 0xffff;
      got_address_low = got_address & 0xffff;

      /* The PLT sequence is not safe for N64 if .got.plt entry's address
	 cannot be loaded in two instructions.  */
      if (ABI_64_P (output_bfd)
	  && ((got_address + 0x80008000) & ~(bfd_vma) 0xffffffff) != 0)
	{
	  _bfd_error_handler
	    /* xgettext:c-format */
	    (_("%pB: `%pA' entry VMA of %#" PRIx64 " outside the 32-bit range "
	       "supported; consider using `-Ttext-segment=...'"),
	     output_bfd,
	     htab->root.sgotplt->output_section,
	     (int64_t) got_address);
	  bfd_set_error (bfd_error_no_error);
	  return false;
	}

      /* Initially point the .got.plt entry at the PLT header.  */
      loc = (htab->root.sgotplt->contents
	     + got_index * MIPS_ELF_GOT_SIZE (dynobj));
      if (ABI_64_P (output_bfd))
	bfd_put_64 (output_bfd, header_address, loc);
      else
	bfd_put_32 (output_bfd, header_address, loc);

      /* Now handle the PLT itself.  First the standard entry (the order
	 does not matter, we just have to pick one).  */
      if (h->plt.plist->mips_offset != MINUS_ONE)
	{
	  const bfd_vma *plt_entry;
	  bfd_vma plt_offset;

	  plt_offset = htab->plt_header_size + h->plt.plist->mips_offset;

	  BFD_ASSERT (plt_offset <= htab->root.splt->size);

	  /* Find out where the .plt entry should go.  */
	  loc = htab->root.splt->contents + plt_offset;

	  /* Pick the load opcode.  */
	  load = MIPS_ELF_LOAD_WORD (output_bfd);

	  /* Fill in the PLT entry itself.  */

	  if (MIPSR6_P (output_bfd))
	    plt_entry = htab->compact_branches ? mipsr6_exec_plt_entry_compact
					       : mipsr6_exec_plt_entry;
	  else
	    plt_entry = mips_exec_plt_entry;
	  bfd_put_32 (output_bfd, plt_entry[0] | got_address_high, loc);
	  bfd_put_32 (output_bfd, plt_entry[1] | got_address_low | load,
		      loc + 4);

	  if (! LOAD_INTERLOCKS_P (output_bfd)
	      || (MIPSR6_P (output_bfd) && htab->compact_branches))
	    {
	      bfd_put_32 (output_bfd, plt_entry[2] | got_address_low, loc + 8);
	      bfd_put_32 (output_bfd, plt_entry[3], loc + 12);
	    }
	  else
	    {
	      bfd_put_32 (output_bfd, plt_entry[3], loc + 8);
	      bfd_put_32 (output_bfd, plt_entry[2] | got_address_low,
			  loc + 12);
	    }
	}

      /* Now the compressed entry.  They come after any standard ones.  */
      if (h->plt.plist->comp_offset != MINUS_ONE)
	{
	  bfd_vma plt_offset;

	  plt_offset = (htab->plt_header_size + htab->plt_mips_offset
			+ h->plt.plist->comp_offset);

	  BFD_ASSERT (plt_offset <= htab->root.splt->size);

	  /* Find out where the .plt entry should go.  */
	  loc = htab->root.splt->contents + plt_offset;

	  /* Fill in the PLT entry itself.  */
	  if (!MICROMIPS_P (output_bfd))
	    {
	      const bfd_vma *plt_entry = mips16_o32_exec_plt_entry;

	      bfd_put_16 (output_bfd, plt_entry[0], loc);
	      bfd_put_16 (output_bfd, plt_entry[1], loc + 2);
	      bfd_put_16 (output_bfd, plt_entry[2], loc + 4);
	      bfd_put_16 (output_bfd, plt_entry[3], loc + 6);
	      bfd_put_16 (output_bfd, plt_entry[4], loc + 8);
	      bfd_put_16 (output_bfd, plt_entry[5], loc + 10);
	      bfd_put_32 (output_bfd, got_address, loc + 12);
	    }
	  else if (htab->insn32)
	    {
	      const bfd_vma *plt_entry = micromips_insn32_o32_exec_plt_entry;

	      bfd_put_16 (output_bfd, plt_entry[0], loc);
	      bfd_put_16 (output_bfd, got_address_high, loc + 2);
	      bfd_put_16 (output_bfd, plt_entry[2], loc + 4);
	      bfd_put_16 (output_bfd, got_address_low, loc + 6);
	      bfd_put_16 (output_bfd, plt_entry[4], loc + 8);
	      bfd_put_16 (output_bfd, plt_entry[5], loc + 10);
	      bfd_put_16 (output_bfd, plt_entry[6], loc + 12);
	      bfd_put_16 (output_bfd, got_address_low, loc + 14);
	    }
	  else
	    {
	      const bfd_vma *plt_entry = micromips_o32_exec_plt_entry;
	      bfd_signed_vma gotpc_offset;
	      bfd_vma loc_address;

	      BFD_ASSERT (got_address % 4 == 0);

	      loc_address = (htab->root.splt->output_section->vma
			     + htab->root.splt->output_offset + plt_offset);
	      gotpc_offset = got_address - ((loc_address | 3) ^ 3);

	      /* ADDIUPC has a span of +/-16MB, check we're in range.  */
	      if (gotpc_offset + 0x1000000 >= 0x2000000)
		{
		  _bfd_error_handler
		    /* xgettext:c-format */
		    (_("%pB: `%pA' offset of %" PRId64 " from `%pA' "
		       "beyond the range of ADDIUPC"),
		     output_bfd,
		     htab->root.sgotplt->output_section,
		     (int64_t) gotpc_offset,
		     htab->root.splt->output_section);
		  bfd_set_error (bfd_error_no_error);
		  return false;
		}
	      bfd_put_16 (output_bfd,
			  plt_entry[0] | ((gotpc_offset >> 18) & 0x7f), loc);
	      bfd_put_16 (output_bfd, (gotpc_offset >> 2) & 0xffff, loc + 2);
	      bfd_put_16 (output_bfd, plt_entry[2], loc + 4);
	      bfd_put_16 (output_bfd, plt_entry[3], loc + 6);
	      bfd_put_16 (output_bfd, plt_entry[4], loc + 8);
	      bfd_put_16 (output_bfd, plt_entry[5], loc + 10);
	    }
	}

      /* Emit an R_MIPS_JUMP_SLOT relocation against the .got.plt entry.  */
      mips_elf_output_dynamic_relocation (output_bfd, htab->root.srelplt,
					  got_index - 2, h->dynindx,
					  R_MIPS_JUMP_SLOT, got_address);

      /* We distinguish between PLT entries and lazy-binding stubs by
	 giving the former an st_other value of STO_MIPS_PLT.  Set the
	 flag and leave the value if there are any relocations in the
	 binary where pointer equality matters.  */
      sym->st_shndx = SHN_UNDEF;
      if (h->pointer_equality_needed)
	sym->st_other = ELF_ST_SET_MIPS_PLT (sym->st_other);
      else
	{
	  sym->st_value = 0;
	  sym->st_other = 0;
	}
    }

  if (h->plt.plist != NULL && h->plt.plist->stub_offset != MINUS_ONE)
    {
      /* We've decided to create a lazy-binding stub.  */
      bool micromips_p = MICROMIPS_P (output_bfd);
      unsigned int other = micromips_p ? STO_MICROMIPS : 0;
      bfd_vma stub_size = htab->function_stub_size;
      bfd_byte stub[MIPS_FUNCTION_STUB_BIG_SIZE];
      bfd_vma isa_bit = micromips_p;
      bfd_vma stub_big_size;

      if (!micromips_p)
	stub_big_size = MIPS_FUNCTION_STUB_BIG_SIZE;
      else if (htab->insn32)
	stub_big_size = MICROMIPS_INSN32_FUNCTION_STUB_BIG_SIZE;
      else
	stub_big_size = MICROMIPS_FUNCTION_STUB_BIG_SIZE;

      /* This symbol has a stub.  Set it up.  */

      BFD_ASSERT (h->dynindx != -1);

      BFD_ASSERT (stub_size == stub_big_size || h->dynindx <= 0xffff);

      /* Values up to 2^31 - 1 are allowed.  Larger values would cause
	 sign extension at runtime in the stub, resulting in a negative
	 index value.  */
      if (h->dynindx & ~0x7fffffff)
	return false;

      /* Fill the stub.  */
      if (micromips_p)
	{
	  idx = 0;
	  bfd_put_micromips_32 (output_bfd, STUB_LW_MICROMIPS (output_bfd),
				stub + idx);
	  idx += 4;
	  if (htab->insn32)
	    {
	      bfd_put_micromips_32 (output_bfd,
				    STUB_MOVE32_MICROMIPS, stub + idx);
	      idx += 4;
	    }
	  else
	    {
	      bfd_put_16 (output_bfd, STUB_MOVE_MICROMIPS, stub + idx);
	      idx += 2;
	    }
	  if (stub_size == stub_big_size)
	    {
	      long dynindx_hi = (h->dynindx >> 16) & 0x7fff;

	      bfd_put_micromips_32 (output_bfd,
				    STUB_LUI_MICROMIPS (dynindx_hi),
				    stub + idx);
	      idx += 4;
	    }
	  if (htab->insn32)
	    {
	      bfd_put_micromips_32 (output_bfd, STUB_JALR32_MICROMIPS,
				    stub + idx);
	      idx += 4;
	    }
	  else
	    {
	      bfd_put_16 (output_bfd, STUB_JALR_MICROMIPS, stub + idx);
	      idx += 2;
	    }

	  /* If a large stub is not required and sign extension is not a
	     problem, then use legacy code in the stub.  */
	  if (stub_size == stub_big_size)
	    bfd_put_micromips_32 (output_bfd,
				  STUB_ORI_MICROMIPS (h->dynindx & 0xffff),
				  stub + idx);
	  else if (h->dynindx & ~0x7fff)
	    bfd_put_micromips_32 (output_bfd,
				  STUB_LI16U_MICROMIPS (h->dynindx & 0xffff),
				  stub + idx);
	  else
	    bfd_put_micromips_32 (output_bfd,
				  STUB_LI16S_MICROMIPS (output_bfd,
							h->dynindx),
				  stub + idx);
	}
      else
	{
	  idx = 0;
	  bfd_put_32 (output_bfd, STUB_LW (output_bfd), stub + idx);
	  idx += 4;
	  bfd_put_32 (output_bfd, STUB_MOVE, stub + idx);
	  idx += 4;
	  if (stub_size == stub_big_size)
	    {
	      bfd_put_32 (output_bfd, STUB_LUI ((h->dynindx >> 16) & 0x7fff),
			  stub + idx);
	      idx += 4;
	    }

	  if (!(MIPSR6_P (output_bfd) && htab->compact_branches))
	    {
	      bfd_put_32 (output_bfd, STUB_JALR, stub + idx);
	      idx += 4;
	    }

	  /* If a large stub is not required and sign extension is not a
	     problem, then use legacy code in the stub.  */
	  if (stub_size == stub_big_size)
	    bfd_put_32 (output_bfd, STUB_ORI (h->dynindx & 0xffff),
			stub + idx);
	  else if (h->dynindx & ~0x7fff)
	    bfd_put_32 (output_bfd, STUB_LI16U (h->dynindx & 0xffff),
			stub + idx);
	  else
	    bfd_put_32 (output_bfd, STUB_LI16S (output_bfd, h->dynindx),
			stub + idx);
	  idx += 4;

	  if (MIPSR6_P (output_bfd) && htab->compact_branches)
	    bfd_put_32 (output_bfd, STUB_JALRC, stub + idx);
	}

      BFD_ASSERT (h->plt.plist->stub_offset <= htab->sstubs->size);
      memcpy (htab->sstubs->contents + h->plt.plist->stub_offset,
	      stub, stub_size);

      /* Mark the symbol as undefined.  stub_offset != -1 occurs
	 only for the referenced symbol.  */
      sym->st_shndx = SHN_UNDEF;

      /* The run-time linker uses the st_value field of the symbol
	 to reset the global offset table entry for this external
	 to its stub address when unlinking a shared object.  */
      sym->st_value = (htab->sstubs->output_section->vma
		       + htab->sstubs->output_offset
		       + h->plt.plist->stub_offset
		       + isa_bit);
      sym->st_other = other;
    }

  /* If we have a MIPS16 function with a stub, the dynamic symbol must
     refer to the stub, since only the stub uses the standard calling
     conventions.  */
  if (h->dynindx != -1 && hmips->fn_stub != NULL)
    {
      BFD_ASSERT (hmips->need_fn_stub);
      sym->st_value = (hmips->fn_stub->output_section->vma
		       + hmips->fn_stub->output_offset);
      sym->st_size = hmips->fn_stub->size;
      sym->st_other = ELF_ST_VISIBILITY (sym->st_other);
    }

  BFD_ASSERT (h->dynindx != -1
	      || h->forced_local);

  sgot = htab->root.sgot;
  g = htab->got_info;
  BFD_ASSERT (g != NULL);

  /* Run through the global symbol table, creating GOT entries for all
     the symbols that need them.  */
  if (hmips->global_got_area != GGA_NONE)
    {
      bfd_vma offset;
      bfd_vma value;

      value = sym->st_value;
      offset = mips_elf_primary_global_got_index (output_bfd, info, h);
      MIPS_ELF_PUT_WORD (output_bfd, value, sgot->contents + offset);
    }

  if (hmips->global_got_area != GGA_NONE && g->next)
    {
      struct mips_got_entry e, *p;
      bfd_vma entry;
      bfd_vma offset;

      gg = g;

      e.abfd = output_bfd;
      e.symndx = -1;
      e.d.h = hmips;
      e.tls_type = GOT_TLS_NONE;

      for (g = g->next; g->next != gg; g = g->next)
	{
	  if (g->got_entries
	      && (p = (struct mips_got_entry *) htab_find (g->got_entries,
							   &e)))
	    {
	      offset = p->gotidx;
	      BFD_ASSERT (offset > 0 && offset < htab->root.sgot->size);
	      if (bfd_link_pic (info)
		  || (elf_hash_table (info)->dynamic_sections_created
		      && p->d.h != NULL
		      && p->d.h->root.def_dynamic
		      && !p->d.h->root.def_regular))
		{
		  /* Create an R_MIPS_REL32 relocation for this entry.  Due to
		     the various compatibility problems, it's easier to mock
		     up an R_MIPS_32 or R_MIPS_64 relocation and leave
		     mips_elf_create_dynamic_relocation to calculate the
		     appropriate addend.  */
		  Elf_Internal_Rela rel[3];

		  memset (rel, 0, sizeof (rel));
		  if (ABI_64_P (output_bfd))
		    rel[0].r_info = ELF_R_INFO (output_bfd, 0, R_MIPS_64);
		  else
		    rel[0].r_info = ELF_R_INFO (output_bfd, 0, R_MIPS_32);
		  rel[0].r_offset = rel[1].r_offset = rel[2].r_offset = offset;

		  entry = 0;
		  if (! (mips_elf_create_dynamic_relocation
			 (output_bfd, info, rel,
			  e.d.h, NULL, sym->st_value, &entry, sgot)))
		    return false;
		}
	      else
		entry = sym->st_value;
	      MIPS_ELF_PUT_WORD (output_bfd, entry, sgot->contents + offset);
	    }
	}
    }

  /* Mark _DYNAMIC and _GLOBAL_OFFSET_TABLE_ as absolute.  */
  name = h->root.root.string;
  if (h == elf_hash_table (info)->hdynamic
      || h == elf_hash_table (info)->hgot)
    sym->st_shndx = SHN_ABS;
  else if (strcmp (name, "_DYNAMIC_LINK") == 0
	   || strcmp (name, "_DYNAMIC_LINKING") == 0)
    {
      sym->st_shndx = SHN_ABS;
      sym->st_info = ELF_ST_INFO (STB_GLOBAL, STT_SECTION);
      sym->st_value = 1;
    }
  else if (SGI_COMPAT (output_bfd))
    {
      if (strcmp (name, mips_elf_dynsym_rtproc_names[0]) == 0
	  || strcmp (name, mips_elf_dynsym_rtproc_names[1]) == 0)
	{
	  sym->st_info = ELF_ST_INFO (STB_GLOBAL, STT_SECTION);
	  sym->st_other = STO_PROTECTED;
	  sym->st_value = 0;
	  sym->st_shndx = SHN_MIPS_DATA;
	}
      else if (strcmp (name, mips_elf_dynsym_rtproc_names[2]) == 0)
	{
	  sym->st_info = ELF_ST_INFO (STB_GLOBAL, STT_SECTION);
	  sym->st_other = STO_PROTECTED;
	  sym->st_value = mips_elf_hash_table (info)->procedure_count;
	  sym->st_shndx = SHN_ABS;
	}
      else if (sym->st_shndx != SHN_UNDEF && sym->st_shndx != SHN_ABS)
	{
	  if (h->type == STT_FUNC)
	    sym->st_shndx = SHN_MIPS_TEXT;
	  else if (h->type == STT_OBJECT)
	    sym->st_shndx = SHN_MIPS_DATA;
	}
    }

  /* Emit a copy reloc, if needed.  */
  if (h->needs_copy)
    {
      asection *s;
      bfd_vma symval;

      BFD_ASSERT (h->dynindx != -1);
      BFD_ASSERT (htab->use_plts_and_copy_relocs);

      s = mips_elf_rel_dyn_section (info, false);
      symval = (h->root.u.def.section->output_section->vma
		+ h->root.u.def.section->output_offset
		+ h->root.u.def.value);
      mips_elf_output_dynamic_relocation (output_bfd, s, s->reloc_count++,
					  h->dynindx, R_MIPS_COPY, symval);
    }

  /* Handle the IRIX6-specific symbols.  */
  if (IRIX_COMPAT (output_bfd) == ict_irix6)
    mips_elf_irix6_finish_dynamic_symbol (output_bfd, name, sym);

  /* Keep dynamic compressed symbols odd.  This allows the dynamic linker
     to treat compressed symbols like any other.  */
  if (ELF_ST_IS_MIPS16 (sym->st_other))
    {
      BFD_ASSERT (sym->st_value & 1);
      sym->st_other -= STO_MIPS16;
    }
  else if (ELF_ST_IS_MICROMIPS (sym->st_other))
    {
      BFD_ASSERT (sym->st_value & 1);
      sym->st_other -= STO_MICROMIPS;
    }

  return true;
}

/* Likewise, for VxWorks.  */

bool
_bfd_mips_vxworks_finish_dynamic_symbol (bfd *output_bfd,
					 struct bfd_link_info *info,
					 struct elf_link_hash_entry *h,
					 Elf_Internal_Sym *sym)
{
  bfd *dynobj;
  asection *sgot;
  struct mips_got_info *g;
  struct mips_elf_link_hash_table *htab;
  struct mips_elf_link_hash_entry *hmips;

  htab = mips_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);
  dynobj = elf_hash_table (info)->dynobj;
  hmips = (struct mips_elf_link_hash_entry *) h;

  if (h->plt.plist != NULL && h->plt.plist->mips_offset != MINUS_ONE)
    {
      bfd_byte *loc;
      bfd_vma plt_address, got_address, got_offset, branch_offset;
      Elf_Internal_Rela rel;
      static const bfd_vma *plt_entry;
      bfd_vma gotplt_index;
      bfd_vma plt_offset;

      plt_offset = htab->plt_header_size + h->plt.plist->mips_offset;
      gotplt_index = h->plt.plist->gotplt_index;

      BFD_ASSERT (h->dynindx != -1);
      BFD_ASSERT (htab->root.splt != NULL);
      BFD_ASSERT (gotplt_index != MINUS_ONE);
      BFD_ASSERT (plt_offset <= htab->root.splt->size);

      /* Calculate the address of the .plt entry.  */
      plt_address = (htab->root.splt->output_section->vma
		     + htab->root.splt->output_offset
		     + plt_offset);

      /* Calculate the address of the .got.plt entry.  */
      got_address = (htab->root.sgotplt->output_section->vma
		     + htab->root.sgotplt->output_offset
		     + gotplt_index * MIPS_ELF_GOT_SIZE (output_bfd));

      /* Calculate the offset of the .got.plt entry from
	 _GLOBAL_OFFSET_TABLE_.  */
      got_offset = mips_elf_gotplt_index (info, h);

      /* Calculate the offset for the branch at the start of the PLT
	 entry.  The branch jumps to the beginning of .plt.  */
      branch_offset = -(plt_offset / 4 + 1) & 0xffff;

      /* Fill in the initial value of the .got.plt entry.  */
      bfd_put_32 (output_bfd, plt_address,
		  (htab->root.sgotplt->contents
		   + gotplt_index * MIPS_ELF_GOT_SIZE (output_bfd)));

      /* Find out where the .plt entry should go.  */
      loc = htab->root.splt->contents + plt_offset;

      if (bfd_link_pic (info))
	{
	  plt_entry = mips_vxworks_shared_plt_entry;
	  bfd_put_32 (output_bfd, plt_entry[0] | branch_offset, loc);
	  bfd_put_32 (output_bfd, plt_entry[1] | gotplt_index, loc + 4);
	}
      else
	{
	  bfd_vma got_address_high, got_address_low;

	  plt_entry = mips_vxworks_exec_plt_entry;
	  got_address_high = ((got_address + 0x8000) >> 16) & 0xffff;
	  got_address_low = got_address & 0xffff;

	  bfd_put_32 (output_bfd, plt_entry[0] | branch_offset, loc);
	  bfd_put_32 (output_bfd, plt_entry[1] | gotplt_index, loc + 4);
	  bfd_put_32 (output_bfd, plt_entry[2] | got_address_high, loc + 8);
	  bfd_put_32 (output_bfd, plt_entry[3] | got_address_low, loc + 12);
	  bfd_put_32 (output_bfd, plt_entry[4], loc + 16);
	  bfd_put_32 (output_bfd, plt_entry[5], loc + 20);
	  bfd_put_32 (output_bfd, plt_entry[6], loc + 24);
	  bfd_put_32 (output_bfd, plt_entry[7], loc + 28);

	  loc = (htab->srelplt2->contents
		 + (gotplt_index * 3 + 2) * sizeof (Elf32_External_Rela));

	  /* Emit a relocation for the .got.plt entry.  */
	  rel.r_offset = got_address;
	  rel.r_info = ELF32_R_INFO (htab->root.hplt->indx, R_MIPS_32);
	  rel.r_addend = plt_offset;
	  bfd_elf32_swap_reloca_out (output_bfd, &rel, loc);

	  /* Emit a relocation for the lui of %hi(<.got.plt slot>).  */
	  loc += sizeof (Elf32_External_Rela);
	  rel.r_offset = plt_address + 8;
	  rel.r_info = ELF32_R_INFO (htab->root.hgot->indx, R_MIPS_HI16);
	  rel.r_addend = got_offset;
	  bfd_elf32_swap_reloca_out (output_bfd, &rel, loc);

	  /* Emit a relocation for the addiu of %lo(<.got.plt slot>).  */
	  loc += sizeof (Elf32_External_Rela);
	  rel.r_offset += 4;
	  rel.r_info = ELF32_R_INFO (htab->root.hgot->indx, R_MIPS_LO16);
	  bfd_elf32_swap_reloca_out (output_bfd, &rel, loc);
	}

      /* Emit an R_MIPS_JUMP_SLOT relocation against the .got.plt entry.  */
      loc = (htab->root.srelplt->contents
	     + gotplt_index * sizeof (Elf32_External_Rela));
      rel.r_offset = got_address;
      rel.r_info = ELF32_R_INFO (h->dynindx, R_MIPS_JUMP_SLOT);
      rel.r_addend = 0;
      bfd_elf32_swap_reloca_out (output_bfd, &rel, loc);

      if (!h->def_regular)
	sym->st_shndx = SHN_UNDEF;
    }

  BFD_ASSERT (h->dynindx != -1 || h->forced_local);

  sgot = htab->root.sgot;
  g = htab->got_info;
  BFD_ASSERT (g != NULL);

  /* See if this symbol has an entry in the GOT.  */
  if (hmips->global_got_area != GGA_NONE)
    {
      bfd_vma offset;
      Elf_Internal_Rela outrel;
      bfd_byte *loc;
      asection *s;

      /* Install the symbol value in the GOT.   */
      offset = mips_elf_primary_global_got_index (output_bfd, info, h);
      MIPS_ELF_PUT_WORD (output_bfd, sym->st_value, sgot->contents + offset);

      /* Add a dynamic relocation for it.  */
      s = mips_elf_rel_dyn_section (info, false);
      loc = s->contents + (s->reloc_count++ * sizeof (Elf32_External_Rela));
      outrel.r_offset = (sgot->output_section->vma
			 + sgot->output_offset
			 + offset);
      outrel.r_info = ELF32_R_INFO (h->dynindx, R_MIPS_32);
      outrel.r_addend = 0;
      bfd_elf32_swap_reloca_out (dynobj, &outrel, loc);
    }

  /* Emit a copy reloc, if needed.  */
  if (h->needs_copy)
    {
      Elf_Internal_Rela rel;
      asection *srel;
      bfd_byte *loc;

      BFD_ASSERT (h->dynindx != -1);

      rel.r_offset = (h->root.u.def.section->output_section->vma
		      + h->root.u.def.section->output_offset
		      + h->root.u.def.value);
      rel.r_info = ELF32_R_INFO (h->dynindx, R_MIPS_COPY);
      rel.r_addend = 0;
      if (h->root.u.def.section == htab->root.sdynrelro)
	srel = htab->root.sreldynrelro;
      else
	srel = htab->root.srelbss;
      loc = srel->contents + srel->reloc_count * sizeof (Elf32_External_Rela);
      bfd_elf32_swap_reloca_out (output_bfd, &rel, loc);
      ++srel->reloc_count;
    }

  /* If this is a mips16/microMIPS symbol, force the value to be even.  */
  if (ELF_ST_IS_COMPRESSED (sym->st_other))
    sym->st_value &= ~1;

  return true;
}

/* Write out a plt0 entry to the beginning of .plt.  */

static bool
mips_finish_exec_plt (bfd *output_bfd, struct bfd_link_info *info)
{
  bfd_byte *loc;
  bfd_vma gotplt_value, gotplt_value_high, gotplt_value_low;
  static const bfd_vma *plt_entry;
  struct mips_elf_link_hash_table *htab;

  htab = mips_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);

  if (ABI_64_P (output_bfd))
    plt_entry = (htab->compact_branches
		 ? mipsr6_n64_exec_plt0_entry_compact
		 : mips_n64_exec_plt0_entry);
  else if (ABI_N32_P (output_bfd))
    plt_entry = (htab->compact_branches
		 ? mipsr6_n32_exec_plt0_entry_compact
		 : mips_n32_exec_plt0_entry);
  else if (!htab->plt_header_is_comp)
    plt_entry = (htab->compact_branches
		 ? mipsr6_o32_exec_plt0_entry_compact
		 : mips_o32_exec_plt0_entry);
  else if (htab->insn32)
    plt_entry = micromips_insn32_o32_exec_plt0_entry;
  else
    plt_entry = micromips_o32_exec_plt0_entry;

  /* Calculate the value of .got.plt.  */
  gotplt_value = (htab->root.sgotplt->output_section->vma
		  + htab->root.sgotplt->output_offset);
  gotplt_value_high = ((gotplt_value + 0x8000) >> 16) & 0xffff;
  gotplt_value_low = gotplt_value & 0xffff;

  /* The PLT sequence is not safe for N64 if .got.plt's address can
     not be loaded in two instructions.  */
  if (ABI_64_P (output_bfd)
      && ((gotplt_value + 0x80008000) & ~(bfd_vma) 0xffffffff) != 0)
    {
      _bfd_error_handler
	/* xgettext:c-format */
	(_("%pB: `%pA' start VMA of %#" PRIx64 " outside the 32-bit range "
	   "supported; consider using `-Ttext-segment=...'"),
	 output_bfd,
	 htab->root.sgotplt->output_section,
	 (int64_t) gotplt_value);
      bfd_set_error (bfd_error_no_error);
      return false;
    }

  /* Install the PLT header.  */
  loc = htab->root.splt->contents;
  if (plt_entry == micromips_o32_exec_plt0_entry)
    {
      bfd_vma gotpc_offset;
      bfd_vma loc_address;
      size_t i;

      BFD_ASSERT (gotplt_value % 4 == 0);

      loc_address = (htab->root.splt->output_section->vma
		     + htab->root.splt->output_offset);
      gotpc_offset = gotplt_value - ((loc_address | 3) ^ 3);

      /* ADDIUPC has a span of +/-16MB, check we're in range.  */
      if (gotpc_offset + 0x1000000 >= 0x2000000)
	{
	  _bfd_error_handler
	    /* xgettext:c-format */
	    (_("%pB: `%pA' offset of %" PRId64 " from `%pA' "
	       "beyond the range of ADDIUPC"),
	     output_bfd,
	     htab->root.sgotplt->output_section,
	     (int64_t) gotpc_offset,
	     htab->root.splt->output_section);
	  bfd_set_error (bfd_error_no_error);
	  return false;
	}
      bfd_put_16 (output_bfd,
		  plt_entry[0] | ((gotpc_offset >> 18) & 0x7f), loc);
      bfd_put_16 (output_bfd, (gotpc_offset >> 2) & 0xffff, loc + 2);
      for (i = 2; i < ARRAY_SIZE (micromips_o32_exec_plt0_entry); i++)
	bfd_put_16 (output_bfd, plt_entry[i], loc + (i * 2));
    }
  else if (plt_entry == micromips_insn32_o32_exec_plt0_entry)
    {
      size_t i;

      bfd_put_16 (output_bfd, plt_entry[0], loc);
      bfd_put_16 (output_bfd, gotplt_value_high, loc + 2);
      bfd_put_16 (output_bfd, plt_entry[2], loc + 4);
      bfd_put_16 (output_bfd, gotplt_value_low, loc + 6);
      bfd_put_16 (output_bfd, plt_entry[4], loc + 8);
      bfd_put_16 (output_bfd, gotplt_value_low, loc + 10);
      for (i = 6; i < ARRAY_SIZE (micromips_insn32_o32_exec_plt0_entry); i++)
	bfd_put_16 (output_bfd, plt_entry[i], loc + (i * 2));
    }
  else
    {
      bfd_put_32 (output_bfd, plt_entry[0] | gotplt_value_high, loc);
      bfd_put_32 (output_bfd, plt_entry[1] | gotplt_value_low, loc + 4);
      bfd_put_32 (output_bfd, plt_entry[2] | gotplt_value_low, loc + 8);
      bfd_put_32 (output_bfd, plt_entry[3], loc + 12);
      bfd_put_32 (output_bfd, plt_entry[4], loc + 16);
      bfd_put_32 (output_bfd, plt_entry[5], loc + 20);
      bfd_put_32 (output_bfd, plt_entry[6], loc + 24);
      bfd_put_32 (output_bfd, plt_entry[7], loc + 28);
    }

  return true;
}

/* Install the PLT header for a VxWorks executable and finalize the
   contents of .rela.plt.unloaded.  */

static void
mips_vxworks_finish_exec_plt (bfd *output_bfd, struct bfd_link_info *info)
{
  Elf_Internal_Rela rela;
  bfd_byte *loc;
  bfd_vma got_value, got_value_high, got_value_low, plt_address;
  static const bfd_vma *plt_entry;
  struct mips_elf_link_hash_table *htab;

  htab = mips_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);

  plt_entry = mips_vxworks_exec_plt0_entry;

  /* Calculate the value of _GLOBAL_OFFSET_TABLE_.  */
  got_value = (htab->root.hgot->root.u.def.section->output_section->vma
	       + htab->root.hgot->root.u.def.section->output_offset
	       + htab->root.hgot->root.u.def.value);

  got_value_high = ((got_value + 0x8000) >> 16) & 0xffff;
  got_value_low = got_value & 0xffff;

  /* Calculate the address of the PLT header.  */
  plt_address = (htab->root.splt->output_section->vma
		 + htab->root.splt->output_offset);

  /* Install the PLT header.  */
  loc = htab->root.splt->contents;
  bfd_put_32 (output_bfd, plt_entry[0] | got_value_high, loc);
  bfd_put_32 (output_bfd, plt_entry[1] | got_value_low, loc + 4);
  bfd_put_32 (output_bfd, plt_entry[2], loc + 8);
  bfd_put_32 (output_bfd, plt_entry[3], loc + 12);
  bfd_put_32 (output_bfd, plt_entry[4], loc + 16);
  bfd_put_32 (output_bfd, plt_entry[5], loc + 20);

  /* Output the relocation for the lui of %hi(_GLOBAL_OFFSET_TABLE_).  */
  loc = htab->srelplt2->contents;
  rela.r_offset = plt_address;
  rela.r_info = ELF32_R_INFO (htab->root.hgot->indx, R_MIPS_HI16);
  rela.r_addend = 0;
  bfd_elf32_swap_reloca_out (output_bfd, &rela, loc);
  loc += sizeof (Elf32_External_Rela);

  /* Output the relocation for the following addiu of
     %lo(_GLOBAL_OFFSET_TABLE_).  */
  rela.r_offset += 4;
  rela.r_info = ELF32_R_INFO (htab->root.hgot->indx, R_MIPS_LO16);
  bfd_elf32_swap_reloca_out (output_bfd, &rela, loc);
  loc += sizeof (Elf32_External_Rela);

  /* Fix up the remaining relocations.  They may have the wrong
     symbol index for _G_O_T_ or _P_L_T_ depending on the order
     in which symbols were output.  */
  while (loc < htab->srelplt2->contents + htab->srelplt2->size)
    {
      Elf_Internal_Rela rel;

      bfd_elf32_swap_reloca_in (output_bfd, loc, &rel);
      rel.r_info = ELF32_R_INFO (htab->root.hplt->indx, R_MIPS_32);
      bfd_elf32_swap_reloca_out (output_bfd, &rel, loc);
      loc += sizeof (Elf32_External_Rela);

      bfd_elf32_swap_reloca_in (output_bfd, loc, &rel);
      rel.r_info = ELF32_R_INFO (htab->root.hgot->indx, R_MIPS_HI16);
      bfd_elf32_swap_reloca_out (output_bfd, &rel, loc);
      loc += sizeof (Elf32_External_Rela);

      bfd_elf32_swap_reloca_in (output_bfd, loc, &rel);
      rel.r_info = ELF32_R_INFO (htab->root.hgot->indx, R_MIPS_LO16);
      bfd_elf32_swap_reloca_out (output_bfd, &rel, loc);
      loc += sizeof (Elf32_External_Rela);
    }
}

/* Install the PLT header for a VxWorks shared library.  */

static void
mips_vxworks_finish_shared_plt (bfd *output_bfd, struct bfd_link_info *info)
{
  unsigned int i;
  struct mips_elf_link_hash_table *htab;

  htab = mips_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);

  /* We just need to copy the entry byte-by-byte.  */
  for (i = 0; i < ARRAY_SIZE (mips_vxworks_shared_plt0_entry); i++)
    bfd_put_32 (output_bfd, mips_vxworks_shared_plt0_entry[i],
		htab->root.splt->contents + i * 4);
}

/* Finish up the dynamic sections.  */

bool
_bfd_mips_elf_finish_dynamic_sections (bfd *output_bfd,
				       struct bfd_link_info *info)
{
  bfd *dynobj;
  asection *sdyn;
  asection *sgot;
  struct mips_got_info *gg, *g;
  struct mips_elf_link_hash_table *htab;

  htab = mips_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);

  dynobj = elf_hash_table (info)->dynobj;

  sdyn = bfd_get_linker_section (dynobj, ".dynamic");

  sgot = htab->root.sgot;
  gg = htab->got_info;

  if (elf_hash_table (info)->dynamic_sections_created)
    {
      bfd_byte *b;
      int dyn_to_skip = 0, dyn_skipped = 0;

      BFD_ASSERT (sdyn != NULL);
      BFD_ASSERT (gg != NULL);

      g = mips_elf_bfd_got (output_bfd, false);
      BFD_ASSERT (g != NULL);

      for (b = sdyn->contents;
	   b < sdyn->contents + sdyn->size;
	   b += MIPS_ELF_DYN_SIZE (dynobj))
	{
	  Elf_Internal_Dyn dyn;
	  const char *name;
	  size_t elemsize;
	  asection *s;
	  bool swap_out_p;

	  /* Read in the current dynamic entry.  */
	  (*get_elf_backend_data (dynobj)->s->swap_dyn_in) (dynobj, b, &dyn);

	  /* Assume that we're going to modify it and write it out.  */
	  swap_out_p = true;

	  switch (dyn.d_tag)
	    {
	    case DT_RELENT:
	      dyn.d_un.d_val = MIPS_ELF_REL_SIZE (dynobj);
	      break;

	    case DT_RELAENT:
	      BFD_ASSERT (htab->root.target_os == is_vxworks);
	      dyn.d_un.d_val = MIPS_ELF_RELA_SIZE (dynobj);
	      break;

	    case DT_STRSZ:
	      /* Rewrite DT_STRSZ.  */
	      dyn.d_un.d_val =
		_bfd_elf_strtab_size (elf_hash_table (info)->dynstr);
	      break;

	    case DT_PLTGOT:
	      s = htab->root.sgot;
	      dyn.d_un.d_ptr = s->output_section->vma + s->output_offset;
	      break;

	    case DT_MIPS_PLTGOT:
	      s = htab->root.sgotplt;
	      dyn.d_un.d_ptr = s->output_section->vma + s->output_offset;
	      break;

	    case DT_MIPS_RLD_VERSION:
	      dyn.d_un.d_val = 1; /* XXX */
	      break;

	    case DT_MIPS_FLAGS:
	      dyn.d_un.d_val = RHF_NOTPOT; /* XXX */
	      break;

	    case DT_MIPS_TIME_STAMP:
	      {
		time_t t;
		time (&t);
		dyn.d_un.d_val = t;
	      }
	      break;

	    case DT_MIPS_ICHECKSUM:
	      /* XXX FIXME: */
	      swap_out_p = false;
	      break;

	    case DT_MIPS_IVERSION:
	      /* XXX FIXME: */
	      swap_out_p = false;
	      break;

	    case DT_MIPS_BASE_ADDRESS:
	      s = output_bfd->sections;
	      BFD_ASSERT (s != NULL);
	      dyn.d_un.d_ptr = s->vma & ~(bfd_vma) 0xffff;
	      break;

	    case DT_MIPS_LOCAL_GOTNO:
	      dyn.d_un.d_val = g->local_gotno;
	      break;

	    case DT_MIPS_UNREFEXTNO:
	      /* The index into the dynamic symbol table which is the
		 entry of the first external symbol that is not
		 referenced within the same object.  */
	      dyn.d_un.d_val = bfd_count_sections (output_bfd) + 1;
	      break;

	    case DT_MIPS_GOTSYM:
	      if (htab->global_gotsym)
		{
		  dyn.d_un.d_val = htab->global_gotsym->dynindx;
		  break;
		}
	      /* In case if we don't have global got symbols we default
		 to setting DT_MIPS_GOTSYM to the same value as
		 DT_MIPS_SYMTABNO.  */
	      /* Fall through.  */

	    case DT_MIPS_SYMTABNO:
	      name = ".dynsym";
	      elemsize = MIPS_ELF_SYM_SIZE (output_bfd);
	      s = bfd_get_linker_section (dynobj, name);

	      if (s != NULL)
		dyn.d_un.d_val = s->size / elemsize;
	      else
		dyn.d_un.d_val = 0;
	      break;

	    case DT_MIPS_HIPAGENO:
	      dyn.d_un.d_val = g->local_gotno - htab->reserved_gotno;
	      break;

	    case DT_MIPS_RLD_MAP:
	      {
		struct elf_link_hash_entry *h;
		h = mips_elf_hash_table (info)->rld_symbol;
		if (!h)
		  {
		    dyn_to_skip = MIPS_ELF_DYN_SIZE (dynobj);
		    swap_out_p = false;
		    break;
		  }
		s = h->root.u.def.section;

		/* The MIPS_RLD_MAP tag stores the absolute address of the
		   debug pointer.  */
		dyn.d_un.d_ptr = (s->output_section->vma + s->output_offset
				  + h->root.u.def.value);
	      }
	      break;

	    case DT_MIPS_RLD_MAP_REL:
	      {
		struct elf_link_hash_entry *h;
		bfd_vma dt_addr, rld_addr;
		h = mips_elf_hash_table (info)->rld_symbol;
		if (!h)
		  {
		    dyn_to_skip = MIPS_ELF_DYN_SIZE (dynobj);
		    swap_out_p = false;
		    break;
		  }
		s = h->root.u.def.section;

		/* The MIPS_RLD_MAP_REL tag stores the offset to the debug
		   pointer, relative to the address of the tag.  */
		dt_addr = (sdyn->output_section->vma + sdyn->output_offset
			   + (b - sdyn->contents));
		rld_addr = (s->output_section->vma + s->output_offset
			    + h->root.u.def.value);
		dyn.d_un.d_ptr = rld_addr - dt_addr;
	      }
	      break;

	    case DT_MIPS_OPTIONS:
	      s = (bfd_get_section_by_name
		   (output_bfd, MIPS_ELF_OPTIONS_SECTION_NAME (output_bfd)));
	      dyn.d_un.d_ptr = s->vma;
	      break;

	    case DT_PLTREL:
	      BFD_ASSERT (htab->use_plts_and_copy_relocs);
	      if (htab->root.target_os == is_vxworks)
		dyn.d_un.d_val = DT_RELA;
	      else
		dyn.d_un.d_val = DT_REL;
	      break;

	    case DT_PLTRELSZ:
	      BFD_ASSERT (htab->use_plts_and_copy_relocs);
	      dyn.d_un.d_val = htab->root.srelplt->size;
	      break;

	    case DT_JMPREL:
	      BFD_ASSERT (htab->use_plts_and_copy_relocs);
	      dyn.d_un.d_ptr = (htab->root.srelplt->output_section->vma
				+ htab->root.srelplt->output_offset);
	      break;

	    case DT_TEXTREL:
	      /* If we didn't need any text relocations after all, delete
		 the dynamic tag.  */
	      if (!(info->flags & DF_TEXTREL))
		{
		  dyn_to_skip = MIPS_ELF_DYN_SIZE (dynobj);
		  swap_out_p = false;
		}
	      break;

	    case DT_FLAGS:
	      /* If we didn't need any text relocations after all, clear
		 DF_TEXTREL from DT_FLAGS.  */
	      if (!(info->flags & DF_TEXTREL))
		dyn.d_un.d_val &= ~DF_TEXTREL;
	      else
		swap_out_p = false;
	      break;

	    case DT_MIPS_XHASH:
	      name = ".MIPS.xhash";
	      s = bfd_get_linker_section (dynobj, name);
	      dyn.d_un.d_ptr = s->output_section->vma + s->output_offset;
	      break;

	    default:
	      swap_out_p = false;
	      if (htab->root.target_os == is_vxworks
		  && elf_vxworks_finish_dynamic_entry (output_bfd, &dyn))
		swap_out_p = true;
	      break;
	    }

	  if (swap_out_p || dyn_skipped)
	    (*get_elf_backend_data (dynobj)->s->swap_dyn_out)
	      (dynobj, &dyn, b - dyn_skipped);

	  if (dyn_to_skip)
	    {
	      dyn_skipped += dyn_to_skip;
	      dyn_to_skip = 0;
	    }
	}

      /* Wipe out any trailing entries if we shifted down a dynamic tag.  */
      if (dyn_skipped > 0)
	memset (b - dyn_skipped, 0, dyn_skipped);
    }

  if (sgot != NULL && sgot->size > 0
      && !bfd_is_abs_section (sgot->output_section))
    {
      if (htab->root.target_os == is_vxworks)
	{
	  /* The first entry of the global offset table points to the
	     ".dynamic" section.  The second is initialized by the
	     loader and contains the shared library identifier.
	     The third is also initialized by the loader and points
	     to the lazy resolution stub.  */
	  MIPS_ELF_PUT_WORD (output_bfd,
			     sdyn->output_offset + sdyn->output_section->vma,
			     sgot->contents);
	  MIPS_ELF_PUT_WORD (output_bfd, 0,
			     sgot->contents + MIPS_ELF_GOT_SIZE (output_bfd));
	  MIPS_ELF_PUT_WORD (output_bfd, 0,
			     sgot->contents
			     + 2 * MIPS_ELF_GOT_SIZE (output_bfd));
	}
      else
	{
	  /* The first entry of the global offset table will be filled at
	     runtime. The second entry will be used by some runtime loaders.
	     This isn't the case of IRIX rld.  */
	  MIPS_ELF_PUT_WORD (output_bfd, (bfd_vma) 0, sgot->contents);
	  MIPS_ELF_PUT_WORD (output_bfd, MIPS_ELF_GNU_GOT1_MASK (output_bfd),
			     sgot->contents + MIPS_ELF_GOT_SIZE (output_bfd));
	}

      elf_section_data (sgot->output_section)->this_hdr.sh_entsize
	 = MIPS_ELF_GOT_SIZE (output_bfd);
    }

  /* Generate dynamic relocations for the non-primary gots.  */
  if (gg != NULL && gg->next)
    {
      Elf_Internal_Rela rel[3];
      bfd_vma addend = 0;

      memset (rel, 0, sizeof (rel));
      rel[0].r_info = ELF_R_INFO (output_bfd, 0, R_MIPS_REL32);

      for (g = gg->next; g->next != gg; g = g->next)
	{
	  bfd_vma got_index = g->next->local_gotno + g->next->global_gotno
	    + g->next->tls_gotno;

	  MIPS_ELF_PUT_WORD (output_bfd, 0, sgot->contents
			     + got_index++ * MIPS_ELF_GOT_SIZE (output_bfd));
	  MIPS_ELF_PUT_WORD (output_bfd, MIPS_ELF_GNU_GOT1_MASK (output_bfd),
			     sgot->contents
			     + got_index++ * MIPS_ELF_GOT_SIZE (output_bfd));

	  if (! bfd_link_pic (info))
	    continue;

	  for (; got_index < g->local_gotno; got_index++)
	    {
	      if (got_index >= g->assigned_low_gotno
		  && got_index <= g->assigned_high_gotno)
		continue;

	      rel[0].r_offset = rel[1].r_offset = rel[2].r_offset
		= got_index * MIPS_ELF_GOT_SIZE (output_bfd);
	      if (!(mips_elf_create_dynamic_relocation
		    (output_bfd, info, rel, NULL,
		     bfd_abs_section_ptr,
		     0, &addend, sgot)))
		return false;
	      BFD_ASSERT (addend == 0);
	    }
	}
    }

  /* The generation of dynamic relocations for the non-primary gots
     adds more dynamic relocations.  We cannot count them until
     here.  */

  if (elf_hash_table (info)->dynamic_sections_created)
    {
      bfd_byte *b;
      bool swap_out_p;

      BFD_ASSERT (sdyn != NULL);

      for (b = sdyn->contents;
	   b < sdyn->contents + sdyn->size;
	   b += MIPS_ELF_DYN_SIZE (dynobj))
	{
	  Elf_Internal_Dyn dyn;
	  asection *s;

	  /* Read in the current dynamic entry.  */
	  (*get_elf_backend_data (dynobj)->s->swap_dyn_in) (dynobj, b, &dyn);

	  /* Assume that we're going to modify it and write it out.  */
	  swap_out_p = true;

	  switch (dyn.d_tag)
	    {
	    case DT_RELSZ:
	      /* Reduce DT_RELSZ to account for any relocations we
		 decided not to make.  This is for the n64 irix rld,
		 which doesn't seem to apply any relocations if there
		 are trailing null entries.  */
	      s = mips_elf_rel_dyn_section (info, false);
	      dyn.d_un.d_val = (s->reloc_count
				* (ABI_64_P (output_bfd)
				   ? sizeof (Elf64_Mips_External_Rel)
				   : sizeof (Elf32_External_Rel)));
	      /* Adjust the section size too.  Tools like the prelinker
		 can reasonably expect the values to the same.  */
	      BFD_ASSERT (!bfd_is_abs_section (s->output_section));
	      elf_section_data (s->output_section)->this_hdr.sh_size
		= dyn.d_un.d_val;
	      break;

	    default:
	      swap_out_p = false;
	      break;
	    }

	  if (swap_out_p)
	    (*get_elf_backend_data (dynobj)->s->swap_dyn_out)
	      (dynobj, &dyn, b);
	}
    }

  {
    asection *s;
    Elf32_compact_rel cpt;

    if (SGI_COMPAT (output_bfd))
      {
	/* Write .compact_rel section out.  */
	s = bfd_get_linker_section (dynobj, ".compact_rel");
	if (s != NULL)
	  {
	    cpt.id1 = 1;
	    cpt.num = s->reloc_count;
	    cpt.id2 = 2;
	    cpt.offset = (s->output_section->filepos
			  + sizeof (Elf32_External_compact_rel));
	    cpt.reserved0 = 0;
	    cpt.reserved1 = 0;
	    bfd_elf32_swap_compact_rel_out (output_bfd, &cpt,
					    ((Elf32_External_compact_rel *)
					     s->contents));

	    /* Clean up a dummy stub function entry in .text.  */
	    if (htab->sstubs != NULL
		&& htab->sstubs->contents != NULL)
	      {
		file_ptr dummy_offset;

		BFD_ASSERT (htab->sstubs->size >= htab->function_stub_size);
		dummy_offset = htab->sstubs->size - htab->function_stub_size;
		memset (htab->sstubs->contents + dummy_offset, 0,
			htab->function_stub_size);
	      }
	  }
      }

    /* The psABI says that the dynamic relocations must be sorted in
       increasing order of r_symndx.  The VxWorks EABI doesn't require
       this, and because the code below handles REL rather than RELA
       relocations, using it for VxWorks would be outright harmful.  */
    if (htab->root.target_os != is_vxworks)
      {
	s = mips_elf_rel_dyn_section (info, false);
	if (s != NULL
	    && s->size > (bfd_vma)2 * MIPS_ELF_REL_SIZE (output_bfd))
	  {
	    reldyn_sorting_bfd = output_bfd;

	    if (ABI_64_P (output_bfd))
	      qsort ((Elf64_External_Rel *) s->contents + 1,
		     s->reloc_count - 1, sizeof (Elf64_Mips_External_Rel),
		     sort_dynamic_relocs_64);
	    else
	      qsort ((Elf32_External_Rel *) s->contents + 1,
		     s->reloc_count - 1, sizeof (Elf32_External_Rel),
		     sort_dynamic_relocs);
	  }
      }
  }

  if (htab->root.splt && htab->root.splt->size > 0)
    {
      if (htab->root.target_os == is_vxworks)
	{
	  if (bfd_link_pic (info))
	    mips_vxworks_finish_shared_plt (output_bfd, info);
	  else
	    mips_vxworks_finish_exec_plt (output_bfd, info);
	}
      else
	{
	  BFD_ASSERT (!bfd_link_pic (info));
	  if (!mips_finish_exec_plt (output_bfd, info))
	    return false;
	}
    }
  return true;
}


/* Set ABFD's EF_MIPS_ARCH and EF_MIPS_MACH flags.  */

static void
mips_set_isa_flags (bfd *abfd)
{
  flagword val;

  switch (bfd_get_mach (abfd))
    {
    default:
      if (ABI_N32_P (abfd) || ABI_64_P (abfd))
        val = MIPS_DEFAULT_R6 ? E_MIPS_ARCH_64R6 : E_MIPS_ARCH_3;
      else
        val = MIPS_DEFAULT_R6 ? E_MIPS_ARCH_32R6 : E_MIPS_ARCH_1;
      break;

    case bfd_mach_mips3000:
      val = E_MIPS_ARCH_1;
      break;

    case bfd_mach_mips3900:
      val = E_MIPS_ARCH_1 | E_MIPS_MACH_3900;
      break;

    case bfd_mach_mips6000:
      val = E_MIPS_ARCH_2;
      break;

    case bfd_mach_mips4010:
      val = E_MIPS_ARCH_2 | E_MIPS_MACH_4010;
      break;

    case bfd_mach_mips_allegrex:
      val = E_MIPS_ARCH_2 | E_MIPS_MACH_ALLEGREX;
      break;

    case bfd_mach_mips4000:
    case bfd_mach_mips4300:
    case bfd_mach_mips4400:
    case bfd_mach_mips4600:
      val = E_MIPS_ARCH_3;
      break;

    case bfd_mach_mips4100:
      val = E_MIPS_ARCH_3 | E_MIPS_MACH_4100;
      break;

    case bfd_mach_mips4111:
      val = E_MIPS_ARCH_3 | E_MIPS_MACH_4111;
      break;

    case bfd_mach_mips4120:
      val = E_MIPS_ARCH_3 | E_MIPS_MACH_4120;
      break;

    case bfd_mach_mips4650:
      val = E_MIPS_ARCH_3 | E_MIPS_MACH_4650;
      break;

    case bfd_mach_mips5400:
      val = E_MIPS_ARCH_4 | E_MIPS_MACH_5400;
      break;

    case bfd_mach_mips5500:
      val = E_MIPS_ARCH_4 | E_MIPS_MACH_5500;
      break;

    case bfd_mach_mips5900:
      val = E_MIPS_ARCH_3 | E_MIPS_MACH_5900;
      break;

    case bfd_mach_mips9000:
      val = E_MIPS_ARCH_4 | E_MIPS_MACH_9000;
      break;

    case bfd_mach_mips5000:
    case bfd_mach_mips7000:
    case bfd_mach_mips8000:
    case bfd_mach_mips10000:
    case bfd_mach_mips12000:
    case bfd_mach_mips14000:
    case bfd_mach_mips16000:
      val = E_MIPS_ARCH_4;
      break;

    case bfd_mach_mips5:
      val = E_MIPS_ARCH_5;
      break;

    case bfd_mach_mips_loongson_2e:
      val = E_MIPS_ARCH_3 | E_MIPS_MACH_LS2E;
      break;

    case bfd_mach_mips_loongson_2f:
      val = E_MIPS_ARCH_3 | E_MIPS_MACH_LS2F;
      break;

    case bfd_mach_mips_sb1:
      val = E_MIPS_ARCH_64 | E_MIPS_MACH_SB1;
      break;

    case bfd_mach_mips_gs464:
      val = E_MIPS_ARCH_64R2 | E_MIPS_MACH_GS464;
      break;

    case bfd_mach_mips_gs464e:
      val = E_MIPS_ARCH_64R2 | E_MIPS_MACH_GS464E;
      break;

    case bfd_mach_mips_gs264e:
      val = E_MIPS_ARCH_64R2 | E_MIPS_MACH_GS264E;
      break;

    case bfd_mach_mips_octeon:
    case bfd_mach_mips_octeonp:
      val = E_MIPS_ARCH_64R2 | E_MIPS_MACH_OCTEON;
      break;

    case bfd_mach_mips_octeon3:
      val = E_MIPS_ARCH_64R2 | E_MIPS_MACH_OCTEON3;
      break;

    case bfd_mach_mips_xlr:
      val = E_MIPS_ARCH_64 | E_MIPS_MACH_XLR;
      break;

    case bfd_mach_mips_octeon2:
      val = E_MIPS_ARCH_64R2 | E_MIPS_MACH_OCTEON2;
      break;

    case bfd_mach_mipsisa32:
      val = E_MIPS_ARCH_32;
      break;

    case bfd_mach_mipsisa64:
      val = E_MIPS_ARCH_64;
      break;

    case bfd_mach_mipsisa32r2:
    case bfd_mach_mipsisa32r3:
    case bfd_mach_mipsisa32r5:
      val = E_MIPS_ARCH_32R2;
      break;

    case bfd_mach_mips_interaptiv_mr2:
      val = E_MIPS_ARCH_32R2 | E_MIPS_MACH_IAMR2;
      break;

    case bfd_mach_mipsisa64r2:
    case bfd_mach_mipsisa64r3:
    case bfd_mach_mipsisa64r5:
      val = E_MIPS_ARCH_64R2;
      break;

    case bfd_mach_mipsisa32r6:
      val = E_MIPS_ARCH_32R6;
      break;

    case bfd_mach_mipsisa64r6:
      val = E_MIPS_ARCH_64R6;
      break;
    }
  elf_elfheader (abfd)->e_flags &= ~(EF_MIPS_ARCH | EF_MIPS_MACH);
  elf_elfheader (abfd)->e_flags |= val;

}


/* Whether to sort relocs output by ld -r or ld --emit-relocs, by r_offset.
   Don't do so for code sections.  We want to keep ordering of HI16/LO16
   as is.  On the other hand, elf-eh-frame.c processing requires .eh_frame
   relocs to be sorted.  */

bool
_bfd_mips_elf_sort_relocs_p (asection *sec)
{
  return (sec->flags & SEC_CODE) == 0;
}


/* The final processing done just before writing out a MIPS ELF object
   file.  This gets the MIPS architecture right based on the machine
   number.  This is used by both the 32-bit and the 64-bit ABI.  */

void
_bfd_mips_final_write_processing (bfd *abfd)
{
  unsigned int i;
  Elf_Internal_Shdr **hdrpp;
  const char *name;
  asection *sec;

  /* Keep the existing EF_MIPS_MACH and EF_MIPS_ARCH flags if the former
     is nonzero.  This is for compatibility with old objects, which used
     a combination of a 32-bit EF_MIPS_ARCH and a 64-bit EF_MIPS_MACH.  */
  if ((elf_elfheader (abfd)->e_flags & EF_MIPS_MACH) == 0)
    mips_set_isa_flags (abfd);

  /* Set the sh_info field for .gptab sections and other appropriate
     info for each special section.  */
  for (i = 1, hdrpp = elf_elfsections (abfd) + 1;
       i < elf_numsections (abfd);
       i++, hdrpp++)
    {
      switch ((*hdrpp)->sh_type)
	{
	case SHT_MIPS_MSYM:
	case SHT_MIPS_LIBLIST:
	  sec = bfd_get_section_by_name (abfd, ".dynstr");
	  if (sec != NULL)
	    (*hdrpp)->sh_link = elf_section_data (sec)->this_idx;
	  break;

	case SHT_MIPS_GPTAB:
	  BFD_ASSERT ((*hdrpp)->bfd_section != NULL);
	  name = bfd_section_name ((*hdrpp)->bfd_section);
	  BFD_ASSERT (name != NULL
		      && startswith (name, ".gptab."));
	  sec = bfd_get_section_by_name (abfd, name + sizeof ".gptab" - 1);
	  BFD_ASSERT (sec != NULL);
	  (*hdrpp)->sh_info = elf_section_data (sec)->this_idx;
	  break;

	case SHT_MIPS_CONTENT:
	  BFD_ASSERT ((*hdrpp)->bfd_section != NULL);
	  name = bfd_section_name ((*hdrpp)->bfd_section);
	  BFD_ASSERT (name != NULL
		      && startswith (name, ".MIPS.content"));
	  sec = bfd_get_section_by_name (abfd,
					 name + sizeof ".MIPS.content" - 1);
	  BFD_ASSERT (sec != NULL);
	  (*hdrpp)->sh_link = elf_section_data (sec)->this_idx;
	  break;

	case SHT_MIPS_SYMBOL_LIB:
	  sec = bfd_get_section_by_name (abfd, ".dynsym");
	  if (sec != NULL)
	    (*hdrpp)->sh_link = elf_section_data (sec)->this_idx;
	  sec = bfd_get_section_by_name (abfd, ".liblist");
	  if (sec != NULL)
	    (*hdrpp)->sh_info = elf_section_data (sec)->this_idx;
	  break;

	case SHT_MIPS_EVENTS:
	  BFD_ASSERT ((*hdrpp)->bfd_section != NULL);
	  name = bfd_section_name ((*hdrpp)->bfd_section);
	  BFD_ASSERT (name != NULL);
	  if (startswith (name, ".MIPS.events"))
	    sec = bfd_get_section_by_name (abfd,
					   name + sizeof ".MIPS.events" - 1);
	  else
	    {
	      BFD_ASSERT (startswith (name, ".MIPS.post_rel"));
	      sec = bfd_get_section_by_name (abfd,
					     (name
					      + sizeof ".MIPS.post_rel" - 1));
	    }
	  BFD_ASSERT (sec != NULL);
	  (*hdrpp)->sh_link = elf_section_data (sec)->this_idx;
	  break;

	case SHT_MIPS_XHASH:
	  sec = bfd_get_section_by_name (abfd, ".dynsym");
	  if (sec != NULL)
	    (*hdrpp)->sh_link = elf_section_data (sec)->this_idx;
	}
    }
}

bool
_bfd_mips_elf_final_write_processing (bfd *abfd)
{
  _bfd_mips_final_write_processing (abfd);
  return _bfd_elf_final_write_processing (abfd);
}

/* When creating an IRIX5 executable, we need REGINFO and RTPROC
   segments.  */

int
_bfd_mips_elf_additional_program_headers (bfd *abfd,
					  struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  asection *s;
  int ret = 0;

  /* See if we need a PT_MIPS_REGINFO segment.  */
  s = bfd_get_section_by_name (abfd, ".reginfo");
  if (s && (s->flags & SEC_LOAD))
    ++ret;

  /* See if we need a PT_MIPS_ABIFLAGS segment.  */
  if (bfd_get_section_by_name (abfd, ".MIPS.abiflags"))
    ++ret;

  /* See if we need a PT_MIPS_OPTIONS segment.  */
  if (IRIX_COMPAT (abfd) == ict_irix6
      && bfd_get_section_by_name (abfd,
				  MIPS_ELF_OPTIONS_SECTION_NAME (abfd)))
    ++ret;

  /* See if we need a PT_MIPS_RTPROC segment.  */
  if (IRIX_COMPAT (abfd) == ict_irix5
      && bfd_get_section_by_name (abfd, ".dynamic")
      && bfd_get_section_by_name (abfd, ".mdebug"))
    ++ret;

  /* Allocate a PT_NULL header in dynamic objects.  See
     _bfd_mips_elf_modify_segment_map for details.  */
  if (!SGI_COMPAT (abfd)
      && bfd_get_section_by_name (abfd, ".dynamic"))
    ++ret;

  return ret;
}

/* Modify the segment map for an IRIX5 executable.  */

bool
_bfd_mips_elf_modify_segment_map (bfd *abfd,
				  struct bfd_link_info *info)
{
  asection *s;
  struct elf_segment_map *m, **pm;
  size_t amt;

  /* If there is a .reginfo section, we need a PT_MIPS_REGINFO
     segment.  */
  s = bfd_get_section_by_name (abfd, ".reginfo");
  if (s != NULL && (s->flags & SEC_LOAD) != 0)
    {
      for (m = elf_seg_map (abfd); m != NULL; m = m->next)
	if (m->p_type == PT_MIPS_REGINFO)
	  break;
      if (m == NULL)
	{
	  amt = sizeof *m;
	  m = bfd_zalloc (abfd, amt);
	  if (m == NULL)
	    return false;

	  m->p_type = PT_MIPS_REGINFO;
	  m->count = 1;
	  m->sections[0] = s;

	  /* We want to put it after the PHDR and INTERP segments.  */
	  pm = &elf_seg_map (abfd);
	  while (*pm != NULL
		 && ((*pm)->p_type == PT_PHDR
		     || (*pm)->p_type == PT_INTERP))
	    pm = &(*pm)->next;

	  m->next = *pm;
	  *pm = m;
	}
    }

  /* If there is a .MIPS.abiflags section, we need a PT_MIPS_ABIFLAGS
     segment.  */
  s = bfd_get_section_by_name (abfd, ".MIPS.abiflags");
  if (s != NULL && (s->flags & SEC_LOAD) != 0)
    {
      for (m = elf_seg_map (abfd); m != NULL; m = m->next)
	if (m->p_type == PT_MIPS_ABIFLAGS)
	  break;
      if (m == NULL)
	{
	  amt = sizeof *m;
	  m = bfd_zalloc (abfd, amt);
	  if (m == NULL)
	    return false;

	  m->p_type = PT_MIPS_ABIFLAGS;
	  m->count = 1;
	  m->sections[0] = s;

	  /* We want to put it after the PHDR and INTERP segments.  */
	  pm = &elf_seg_map (abfd);
	  while (*pm != NULL
		 && ((*pm)->p_type == PT_PHDR
		     || (*pm)->p_type == PT_INTERP))
	    pm = &(*pm)->next;

	  m->next = *pm;
	  *pm = m;
	}
    }

  /* For IRIX 6, we don't have .mdebug sections, nor does anything but
     .dynamic end up in PT_DYNAMIC.  However, we do have to insert a
     PT_MIPS_OPTIONS segment immediately following the program header
     table.  */
  if (NEWABI_P (abfd)
      /* On non-IRIX6 new abi, we'll have already created a segment
	 for this section, so don't create another.  I'm not sure this
	 is not also the case for IRIX 6, but I can't test it right
	 now.  */
      && IRIX_COMPAT (abfd) == ict_irix6)
    {
      for (s = abfd->sections; s; s = s->next)
	if (elf_section_data (s)->this_hdr.sh_type == SHT_MIPS_OPTIONS)
	  break;

      if (s)
	{
	  struct elf_segment_map *options_segment;

	  pm = &elf_seg_map (abfd);
	  while (*pm != NULL
		 && ((*pm)->p_type == PT_PHDR
		     || (*pm)->p_type == PT_INTERP))
	    pm = &(*pm)->next;

	  if (*pm == NULL || (*pm)->p_type != PT_MIPS_OPTIONS)
	    {
	      amt = sizeof (struct elf_segment_map);
	      options_segment = bfd_zalloc (abfd, amt);
	      options_segment->next = *pm;
	      options_segment->p_type = PT_MIPS_OPTIONS;
	      options_segment->p_flags = PF_R;
	      options_segment->p_flags_valid = true;
	      options_segment->count = 1;
	      options_segment->sections[0] = s;
	      *pm = options_segment;
	    }
	}
    }
  else
    {
      if (IRIX_COMPAT (abfd) == ict_irix5)
	{
	  /* If there are .dynamic and .mdebug sections, we make a room
	     for the RTPROC header.  FIXME: Rewrite without section names.  */
	  if (bfd_get_section_by_name (abfd, ".interp") == NULL
	      && bfd_get_section_by_name (abfd, ".dynamic") != NULL
	      && bfd_get_section_by_name (abfd, ".mdebug") != NULL)
	    {
	      for (m = elf_seg_map (abfd); m != NULL; m = m->next)
		if (m->p_type == PT_MIPS_RTPROC)
		  break;
	      if (m == NULL)
		{
		  amt = sizeof *m;
		  m = bfd_zalloc (abfd, amt);
		  if (m == NULL)
		    return false;

		  m->p_type = PT_MIPS_RTPROC;

		  s = bfd_get_section_by_name (abfd, ".rtproc");
		  if (s == NULL)
		    {
		      m->count = 0;
		      m->p_flags = 0;
		      m->p_flags_valid = 1;
		    }
		  else
		    {
		      m->count = 1;
		      m->sections[0] = s;
		    }

		  /* We want to put it after the DYNAMIC segment.  */
		  pm = &elf_seg_map (abfd);
		  while (*pm != NULL && (*pm)->p_type != PT_DYNAMIC)
		    pm = &(*pm)->next;
		  if (*pm != NULL)
		    pm = &(*pm)->next;

		  m->next = *pm;
		  *pm = m;
		}
	    }
	}
      /* On IRIX5, the PT_DYNAMIC segment includes the .dynamic,
	 .dynstr, .dynsym, and .hash sections, and everything in
	 between.  */
      for (pm = &elf_seg_map (abfd); *pm != NULL;
	   pm = &(*pm)->next)
	if ((*pm)->p_type == PT_DYNAMIC)
	  break;
      m = *pm;
      /* GNU/Linux binaries do not need the extended PT_DYNAMIC section.
	 glibc's dynamic linker has traditionally derived the number of
	 tags from the p_filesz field, and sometimes allocates stack
	 arrays of that size.  An overly-big PT_DYNAMIC segment can
	 be actively harmful in such cases.  Making PT_DYNAMIC contain
	 other sections can also make life hard for the prelinker,
	 which might move one of the other sections to a different
	 PT_LOAD segment.  */
      if (SGI_COMPAT (abfd)
	  && m != NULL
	  && m->count == 1
	  && strcmp (m->sections[0]->name, ".dynamic") == 0)
	{
	  static const char *sec_names[] =
	  {
	    ".dynamic", ".dynstr", ".dynsym", ".hash"
	  };
	  bfd_vma low, high;
	  unsigned int i, c;
	  struct elf_segment_map *n;

	  low = ~(bfd_vma) 0;
	  high = 0;
	  for (i = 0; i < sizeof sec_names / sizeof sec_names[0]; i++)
	    {
	      s = bfd_get_section_by_name (abfd, sec_names[i]);
	      if (s != NULL && (s->flags & SEC_LOAD) != 0)
		{
		  bfd_size_type sz;

		  if (low > s->vma)
		    low = s->vma;
		  sz = s->size;
		  if (high < s->vma + sz)
		    high = s->vma + sz;
		}
	    }

	  c = 0;
	  for (s = abfd->sections; s != NULL; s = s->next)
	    if ((s->flags & SEC_LOAD) != 0
		&& s->vma >= low
		&& s->vma + s->size <= high)
	      ++c;

	  amt = sizeof *n - sizeof (asection *) + c * sizeof (asection *);
	  n = bfd_zalloc (abfd, amt);
	  if (n == NULL)
	    return false;
	  *n = *m;
	  n->count = c;

	  i = 0;
	  for (s = abfd->sections; s != NULL; s = s->next)
	    {
	      if ((s->flags & SEC_LOAD) != 0
		  && s->vma >= low
		  && s->vma + s->size <= high)
		{
		  n->sections[i] = s;
		  ++i;
		}
	    }

	  *pm = n;
	}
    }

  /* Allocate a spare program header in dynamic objects so that tools
     like the prelinker can add an extra PT_LOAD entry.

     If the prelinker needs to make room for a new PT_LOAD entry, its
     standard procedure is to move the first (read-only) sections into
     the new (writable) segment.  However, the MIPS ABI requires
     .dynamic to be in a read-only segment, and the section will often
     start within sizeof (ElfNN_Phdr) bytes of the last program header.

     Although the prelinker could in principle move .dynamic to a
     writable segment, it seems better to allocate a spare program
     header instead, and avoid the need to move any sections.
     There is a long tradition of allocating spare dynamic tags,
     so allocating a spare program header seems like a natural
     extension.

     If INFO is NULL, we may be copying an already prelinked binary
     with objcopy or strip, so do not add this header.  */
  if (info != NULL
      && !SGI_COMPAT (abfd)
      && bfd_get_section_by_name (abfd, ".dynamic"))
    {
      for (pm = &elf_seg_map (abfd); *pm != NULL; pm = &(*pm)->next)
	if ((*pm)->p_type == PT_NULL)
	  break;
      if (*pm == NULL)
	{
	  m = bfd_zalloc (abfd, sizeof (*m));
	  if (m == NULL)
	    return false;

	  m->p_type = PT_NULL;
	  *pm = m;
	}
    }

  return true;
}

/* Return the section that should be marked against GC for a given
   relocation.  */

asection *
_bfd_mips_elf_gc_mark_hook (asection *sec,
			    struct bfd_link_info *info,
			    Elf_Internal_Rela *rel,
			    struct elf_link_hash_entry *h,
			    Elf_Internal_Sym *sym)
{
  /* ??? Do mips16 stub sections need to be handled special?  */

  if (h != NULL)
    switch (ELF_R_TYPE (sec->owner, rel->r_info))
      {
      case R_MIPS_GNU_VTINHERIT:
      case R_MIPS_GNU_VTENTRY:
	return NULL;
      }

  return _bfd_elf_gc_mark_hook (sec, info, rel, h, sym);
}

/* Prevent .MIPS.abiflags from being discarded with --gc-sections.  */

bool
_bfd_mips_elf_gc_mark_extra_sections (struct bfd_link_info *info,
				      elf_gc_mark_hook_fn gc_mark_hook)
{
  bfd *sub;

  _bfd_elf_gc_mark_extra_sections (info, gc_mark_hook);

  for (sub = info->input_bfds; sub != NULL; sub = sub->link.next)
    {
      asection *o;

      if (! is_mips_elf (sub))
	continue;

      for (o = sub->sections; o != NULL; o = o->next)
	if (!o->gc_mark
	    && MIPS_ELF_ABIFLAGS_SECTION_NAME_P (bfd_section_name (o)))
	  {
	    if (!_bfd_elf_gc_mark (info, o, gc_mark_hook))
	      return false;
	  }
    }

  return true;
}

/* Copy data from a MIPS ELF indirect symbol to its direct symbol,
   hiding the old indirect symbol.  Process additional relocation
   information.  Also called for weakdefs, in which case we just let
   _bfd_elf_link_hash_copy_indirect copy the flags for us.  */

void
_bfd_mips_elf_copy_indirect_symbol (struct bfd_link_info *info,
				    struct elf_link_hash_entry *dir,
				    struct elf_link_hash_entry *ind)
{
  struct mips_elf_link_hash_entry *dirmips, *indmips;

  _bfd_elf_link_hash_copy_indirect (info, dir, ind);

  dirmips = (struct mips_elf_link_hash_entry *) dir;
  indmips = (struct mips_elf_link_hash_entry *) ind;
  /* Any absolute non-dynamic relocations against an indirect or weak
     definition will be against the target symbol.  */
  if (indmips->has_static_relocs)
    dirmips->has_static_relocs = true;

  if (ind->root.type != bfd_link_hash_indirect)
    return;

  dirmips->possibly_dynamic_relocs += indmips->possibly_dynamic_relocs;
  if (indmips->readonly_reloc)
    dirmips->readonly_reloc = true;
  if (indmips->no_fn_stub)
    dirmips->no_fn_stub = true;
  if (indmips->fn_stub)
    {
      dirmips->fn_stub = indmips->fn_stub;
      indmips->fn_stub = NULL;
    }
  if (indmips->need_fn_stub)
    {
      dirmips->need_fn_stub = true;
      indmips->need_fn_stub = false;
    }
  if (indmips->call_stub)
    {
      dirmips->call_stub = indmips->call_stub;
      indmips->call_stub = NULL;
    }
  if (indmips->call_fp_stub)
    {
      dirmips->call_fp_stub = indmips->call_fp_stub;
      indmips->call_fp_stub = NULL;
    }
  if (indmips->global_got_area < dirmips->global_got_area)
    dirmips->global_got_area = indmips->global_got_area;
  if (indmips->global_got_area < GGA_NONE)
    indmips->global_got_area = GGA_NONE;
  if (indmips->has_nonpic_branches)
    dirmips->has_nonpic_branches = true;
}

/* Take care of the special `__gnu_absolute_zero' symbol and ignore attempts
   to hide it.  It has to remain global (it will also be protected) so as to
   be assigned a global GOT entry, which will then remain unchanged at load
   time.  */

void
_bfd_mips_elf_hide_symbol (struct bfd_link_info *info,
			   struct elf_link_hash_entry *entry,
			   bool force_local)
{
  struct mips_elf_link_hash_table *htab;

  htab = mips_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);
  if (htab->use_absolute_zero
      && strcmp (entry->root.root.string, "__gnu_absolute_zero") == 0)
    return;

  _bfd_elf_link_hash_hide_symbol (info, entry, force_local);
}

#define PDR_SIZE 32

bool
_bfd_mips_elf_discard_info (bfd *abfd, struct elf_reloc_cookie *cookie,
			    struct bfd_link_info *info)
{
  asection *o;
  bool ret = false;
  unsigned char *tdata;
  size_t i, skip;

  o = bfd_get_section_by_name (abfd, ".pdr");
  if (! o)
    return false;
  if (o->size == 0)
    return false;
  if (o->size % PDR_SIZE != 0)
    return false;
  if (o->output_section != NULL
      && bfd_is_abs_section (o->output_section))
    return false;

  tdata = bfd_zmalloc (o->size / PDR_SIZE);
  if (! tdata)
    return false;

  cookie->rels = _bfd_elf_link_read_relocs (abfd, o, NULL, NULL,
					    info->keep_memory);
  if (!cookie->rels)
    {
      free (tdata);
      return false;
    }

  cookie->rel = cookie->rels;
  cookie->relend = cookie->rels + o->reloc_count;

  for (i = 0, skip = 0; i < o->size / PDR_SIZE; i ++)
    {
      if (bfd_elf_reloc_symbol_deleted_p (i * PDR_SIZE, cookie))
	{
	  tdata[i] = 1;
	  skip ++;
	}
    }

  if (skip != 0)
    {
      mips_elf_section_data (o)->u.tdata = tdata;
      if (o->rawsize == 0)
	o->rawsize = o->size;
      o->size -= skip * PDR_SIZE;
      ret = true;
    }
  else
    free (tdata);

  if (! info->keep_memory)
    free (cookie->rels);

  return ret;
}

bool
_bfd_mips_elf_ignore_discarded_relocs (asection *sec)
{
  if (strcmp (sec->name, ".pdr") == 0)
    return true;
  return false;
}

bool
_bfd_mips_elf_write_section (bfd *output_bfd,
			     struct bfd_link_info *link_info ATTRIBUTE_UNUSED,
			     asection *sec, bfd_byte *contents)
{
  bfd_byte *to, *from, *end;
  int i;

  if (strcmp (sec->name, ".pdr") != 0)
    return false;

  if (mips_elf_section_data (sec)->u.tdata == NULL)
    return false;

  to = contents;
  end = contents + sec->size;
  for (from = contents, i = 0;
       from < end;
       from += PDR_SIZE, i++)
    {
      if ((mips_elf_section_data (sec)->u.tdata)[i] == 1)
	continue;
      if (to != from)
	memcpy (to, from, PDR_SIZE);
      to += PDR_SIZE;
    }
  bfd_set_section_contents (output_bfd, sec->output_section, contents,
			    sec->output_offset, sec->size);
  return true;
}

/* microMIPS code retains local labels for linker relaxation.  Omit them
   from output by default for clarity.  */

bool
_bfd_mips_elf_is_target_special_symbol (bfd *abfd, asymbol *sym)
{
  return _bfd_elf_is_local_label_name (abfd, sym->name);
}

bool
_bfd_mips_elf_find_nearest_line (bfd *abfd, asymbol **symbols,
				 asection *section, bfd_vma offset,
				 const char **filename_ptr,
				 const char **functionname_ptr,
				 unsigned int *line_ptr,
				 unsigned int *discriminator_ptr)
{
  asection *msec;

  if (_bfd_dwarf2_find_nearest_line (abfd, symbols, NULL, section, offset,
				     filename_ptr, functionname_ptr,
				     line_ptr, discriminator_ptr,
				     dwarf_debug_sections,
				     &elf_tdata (abfd)->dwarf2_find_line_info)
      == 1)
    return true;

  if (_bfd_dwarf1_find_nearest_line (abfd, symbols, section, offset,
				     filename_ptr, functionname_ptr,
				     line_ptr))
    {
      if (!*functionname_ptr)
	_bfd_elf_find_function (abfd, symbols, section, offset,
				*filename_ptr ? NULL : filename_ptr,
				functionname_ptr);
      return true;
    }

  msec = bfd_get_section_by_name (abfd, ".mdebug");
  if (msec != NULL)
    {
      flagword origflags;
      struct mips_elf_find_line *fi;
      const struct ecoff_debug_swap * const swap =
	get_elf_backend_data (abfd)->elf_backend_ecoff_debug_swap;

      /* If we are called during a link, mips_elf_final_link may have
	 cleared the SEC_HAS_CONTENTS field.  We force it back on here
	 if appropriate (which it normally will be).  */
      origflags = msec->flags;
      if (elf_section_data (msec)->this_hdr.sh_type != SHT_NOBITS)
	msec->flags |= SEC_HAS_CONTENTS;

      fi = mips_elf_tdata (abfd)->find_line_info;
      if (fi == NULL)
	{
	  bfd_size_type external_fdr_size;
	  char *fraw_src;
	  char *fraw_end;
	  struct fdr *fdr_ptr;
	  bfd_size_type amt = sizeof (struct mips_elf_find_line);

	  fi = bfd_zalloc (abfd, amt);
	  if (fi == NULL)
	    {
	      msec->flags = origflags;
	      return false;
	    }

	  if (! _bfd_mips_elf_read_ecoff_info (abfd, msec, &fi->d))
	    {
	      msec->flags = origflags;
	      return false;
	    }

	  /* Swap in the FDR information.  */
	  amt = fi->d.symbolic_header.ifdMax * sizeof (struct fdr);
	  fi->d.fdr = bfd_alloc (abfd, amt);
	  if (fi->d.fdr == NULL)
	    {
	      _bfd_ecoff_free_ecoff_debug_info (&fi->d);
	      msec->flags = origflags;
	      return false;
	    }
	  external_fdr_size = swap->external_fdr_size;
	  fdr_ptr = fi->d.fdr;
	  fraw_src = (char *) fi->d.external_fdr;
	  fraw_end = (fraw_src
		      + fi->d.symbolic_header.ifdMax * external_fdr_size);
	  for (; fraw_src < fraw_end; fraw_src += external_fdr_size, fdr_ptr++)
	    (*swap->swap_fdr_in) (abfd, fraw_src, fdr_ptr);

	  mips_elf_tdata (abfd)->find_line_info = fi;
	}

      if (_bfd_ecoff_locate_line (abfd, section, offset, &fi->d, swap,
				  &fi->i, filename_ptr, functionname_ptr,
				  line_ptr))
	{
	  msec->flags = origflags;
	  return true;
	}

      msec->flags = origflags;
    }

  /* Fall back on the generic ELF find_nearest_line routine.  */

  return _bfd_elf_find_nearest_line (abfd, symbols, section, offset,
				     filename_ptr, functionname_ptr,
				     line_ptr, discriminator_ptr);
}

bool
_bfd_mips_elf_find_inliner_info (bfd *abfd,
				 const char **filename_ptr,
				 const char **functionname_ptr,
				 unsigned int *line_ptr)
{
  bool found;
  found = _bfd_dwarf2_find_inliner_info (abfd, filename_ptr,
					 functionname_ptr, line_ptr,
					 & elf_tdata (abfd)->dwarf2_find_line_info);
  return found;
}


/* When are writing out the .options or .MIPS.options section,
   remember the bytes we are writing out, so that we can install the
   GP value in the section_processing routine.  */

bool
_bfd_mips_elf_set_section_contents (bfd *abfd, sec_ptr section,
				    const void *location,
				    file_ptr offset, bfd_size_type count)
{
  if (MIPS_ELF_OPTIONS_SECTION_NAME_P (section->name))
    {
      bfd_byte *c;

      if (elf_section_data (section) == NULL)
	{
	  size_t amt = sizeof (struct bfd_elf_section_data);
	  section->used_by_bfd = bfd_zalloc (abfd, amt);
	  if (elf_section_data (section) == NULL)
	    return false;
	}
      c = mips_elf_section_data (section)->u.tdata;
      if (c == NULL)
	{
	  c = bfd_zalloc (abfd, section->size);
	  if (c == NULL)
	    return false;
	  mips_elf_section_data (section)->u.tdata = c;
	}

      memcpy (c + offset, location, count);
    }

  return _bfd_elf_set_section_contents (abfd, section, location, offset,
					count);
}

/* This is almost identical to bfd_generic_get_... except that some
   MIPS relocations need to be handled specially.  Sigh.  */

bfd_byte *
_bfd_elf_mips_get_relocated_section_contents
  (bfd *abfd,
   struct bfd_link_info *link_info,
   struct bfd_link_order *link_order,
   bfd_byte *data,
   bool relocatable,
   asymbol **symbols)
{
  bfd *input_bfd = link_order->u.indirect.section->owner;
  asection *input_section = link_order->u.indirect.section;
  long reloc_size;
  arelent **reloc_vector;
  long reloc_count;

  reloc_size = bfd_get_reloc_upper_bound (input_bfd, input_section);
  if (reloc_size < 0)
    return NULL;

  /* Read in the section.  */
  bfd_byte *orig_data = data;
  if (!bfd_get_full_section_contents (input_bfd, input_section, &data))
    return NULL;

  if (data == NULL)
    return NULL;

  if (reloc_size == 0)
    return data;

  reloc_vector = (arelent **) bfd_malloc (reloc_size);
  if (reloc_vector == NULL)
    {
      struct mips_elf_obj_tdata *tdata;
      struct mips_hi16 **hip, *hi;
    error_return:
      /* If we are going to return an error, remove entries on
	 mips_hi16_list that point into this section's data.  Data
	 will typically be freed on return from this function.  */
      tdata = mips_elf_tdata (abfd);
      hip = &tdata->mips_hi16_list;
      while ((hi = *hip) != NULL)
	{
	  if (hi->input_section == input_section)
	    {
	      *hip = hi->next;
	      free (hi);
	    }
	  else
	    hip = &hi->next;
	}
      if (orig_data == NULL)
	free (data);
      data = NULL;
      goto out;
    }

  reloc_count = bfd_canonicalize_reloc (input_bfd,
					input_section,
					reloc_vector,
					symbols);
  if (reloc_count < 0)
    goto error_return;

  if (reloc_count > 0)
    {
      arelent **parent;
      /* for mips */
      int gp_found;
      bfd_vma gp = 0x12345678;	/* initialize just to shut gcc up */

      {
	struct bfd_hash_entry *h;
	struct bfd_link_hash_entry *lh;
	/* Skip all this stuff if we aren't mixing formats.  */
	if (abfd && input_bfd
	    && abfd->xvec == input_bfd->xvec)
	  lh = 0;
	else
	  {
	    h = bfd_hash_lookup (&link_info->hash->table, "_gp", false, false);
	    lh = (struct bfd_link_hash_entry *) h;
	  }
      lookup:
	if (lh)
	  {
	    switch (lh->type)
	      {
	      case bfd_link_hash_undefined:
	      case bfd_link_hash_undefweak:
	      case bfd_link_hash_common:
		gp_found = 0;
		break;
	      case bfd_link_hash_defined:
	      case bfd_link_hash_defweak:
		gp_found = 1;
		gp = lh->u.def.value;
		break;
	      case bfd_link_hash_indirect:
	      case bfd_link_hash_warning:
		lh = lh->u.i.link;
		/* @@FIXME  ignoring warning for now */
		goto lookup;
	      case bfd_link_hash_new:
	      default:
		abort ();
	      }
	  }
	else
	  gp_found = 0;
      }
      /* end mips */

      for (parent = reloc_vector; *parent != NULL; parent++)
	{
	  char *error_message = NULL;
	  asymbol *symbol;
	  bfd_reloc_status_type r;

	  symbol = *(*parent)->sym_ptr_ptr;
	  /* PR ld/19628: A specially crafted input file
	     can result in a NULL symbol pointer here.  */
	  if (symbol == NULL)
	    {
	      link_info->callbacks->einfo
		/* xgettext:c-format */
		(_("%X%P: %pB(%pA): error: relocation for offset %V has no value\n"),
		 abfd, input_section, (* parent)->address);
	      goto error_return;
	    }

	  /* Zap reloc field when the symbol is from a discarded
	     section, ignoring any addend.  Do the same when called
	     from bfd_simple_get_relocated_section_contents for
	     undefined symbols in debug sections.  This is to keep
	     debug info reasonably sane, in particular so that
	     DW_FORM_ref_addr to another file's .debug_info isn't
	     confused with an offset into the current file's
	     .debug_info.  */
	  if ((symbol->section != NULL && discarded_section (symbol->section))
	      || (symbol->section == bfd_und_section_ptr
		  && (input_section->flags & SEC_DEBUGGING) != 0
		  && link_info->input_bfds == link_info->output_bfd))
	    {
	      bfd_vma off;
	      static reloc_howto_type none_howto
		= HOWTO (0, 0, 0, 0, false, 0, complain_overflow_dont, NULL,
			 "unused", false, 0, 0, false);

	      off = ((*parent)->address
		     * bfd_octets_per_byte (input_bfd, input_section));
	      _bfd_clear_contents ((*parent)->howto, input_bfd,
				   input_section, data, off);
	      (*parent)->sym_ptr_ptr = bfd_abs_section_ptr->symbol_ptr_ptr;
	      (*parent)->addend = 0;
	      (*parent)->howto = &none_howto;
	      r = bfd_reloc_ok;
	    }

	  /* Specific to MIPS: Deal with relocation types that require
	     knowing the gp of the output bfd.  */

	  /* If we've managed to find the gp and have a special
	     function for the relocation then go ahead, else default
	     to the generic handling.  */
	  else if (gp_found
		   && ((*parent)->howto->special_function
		       == _bfd_mips_elf32_gprel16_reloc))
	    r = _bfd_mips_elf_gprel16_with_gp (input_bfd, symbol, *parent,
					       input_section, relocatable,
					       data, gp);
	  else
	    r = bfd_perform_relocation (input_bfd,
					*parent,
					data,
					input_section,
					relocatable ? abfd : NULL,
					&error_message);

	  if (relocatable)
	    {
	      asection *os = input_section->output_section;

	      /* A partial link, so keep the relocs.  */
	      os->orelocation[os->reloc_count] = *parent;
	      os->reloc_count++;
	    }

	  if (r != bfd_reloc_ok)
	    {
	      switch (r)
		{
		case bfd_reloc_undefined:
		  (*link_info->callbacks->undefined_symbol)
		    (link_info, bfd_asymbol_name (*(*parent)->sym_ptr_ptr),
		     input_bfd, input_section, (*parent)->address, true);
		  break;
		case bfd_reloc_dangerous:
		  BFD_ASSERT (error_message != NULL);
		  (*link_info->callbacks->reloc_dangerous)
		    (link_info, error_message,
		     input_bfd, input_section, (*parent)->address);
		  break;
		case bfd_reloc_overflow:
		  (*link_info->callbacks->reloc_overflow)
		    (link_info, NULL,
		     bfd_asymbol_name (*(*parent)->sym_ptr_ptr),
		     (*parent)->howto->name, (*parent)->addend,
		     input_bfd, input_section, (*parent)->address);
		  break;
		case bfd_reloc_outofrange:
		  /* PR ld/13730:
		     This error can result when processing some partially
		     complete binaries.  Do not abort, but issue an error
		     message instead.  */
		  link_info->callbacks->einfo
		    /* xgettext:c-format */
		    (_("%X%P: %pB(%pA): relocation \"%pR\" goes out of range\n"),
		     abfd, input_section, * parent);
		  goto error_return;

		case bfd_reloc_notsupported:
		  /* PR ld/17512
		     This error can result when processing a corrupt binary.
		     Do not abort.  Issue an error message instead.  */
		  link_info->callbacks->einfo
		    /* xgettext:c-format */
		    (_("%X%P: %pB(%pA): relocation \"%pR\" is not supported\n"),
		     abfd, input_section, * parent);
		  goto error_return;

		default:
		  /* PR 17512; file: 90c2a92e.
		     Report unexpected results, without aborting.  */
		  link_info->callbacks->einfo
		    /* xgettext:c-format */
		    (_("%X%P: %pB(%pA): relocation \"%pR\" returns an unrecognized value %x\n"),
		     abfd, input_section, * parent, r);
		  break;
		}

	    }
	}
    }

 out:
  free (reloc_vector);
  return data;
}

static bool
mips_elf_relax_delete_bytes (bfd *abfd,
			     asection *sec, bfd_vma addr, int count)
{
  Elf_Internal_Shdr *symtab_hdr;
  unsigned int sec_shndx;
  bfd_byte *contents;
  Elf_Internal_Rela *irel, *irelend;
  Elf_Internal_Sym *isym;
  Elf_Internal_Sym *isymend;
  struct elf_link_hash_entry **sym_hashes;
  struct elf_link_hash_entry **end_hashes;
  struct elf_link_hash_entry **start_hashes;
  unsigned int symcount;

  sec_shndx = _bfd_elf_section_from_bfd_section (abfd, sec);
  contents = elf_section_data (sec)->this_hdr.contents;

  irel = elf_section_data (sec)->relocs;
  irelend = irel + sec->reloc_count;

  /* Actually delete the bytes.  */
  memmove (contents + addr, contents + addr + count,
	   (size_t) (sec->size - addr - count));
  sec->size -= count;

  /* Adjust all the relocs.  */
  for (irel = elf_section_data (sec)->relocs; irel < irelend; irel++)
    {
      /* Get the new reloc address.  */
      if (irel->r_offset > addr)
	irel->r_offset -= count;
    }

  BFD_ASSERT (addr % 2 == 0);
  BFD_ASSERT (count % 2 == 0);

  /* Adjust the local symbols defined in this section.  */
  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  isym = (Elf_Internal_Sym *) symtab_hdr->contents;
  for (isymend = isym + symtab_hdr->sh_info; isym < isymend; isym++)
    if (isym->st_shndx == sec_shndx && isym->st_value > addr)
      isym->st_value -= count;

  /* Now adjust the global symbols defined in this section.  */
  symcount = (symtab_hdr->sh_size / sizeof (Elf32_External_Sym)
	      - symtab_hdr->sh_info);
  sym_hashes = start_hashes = elf_sym_hashes (abfd);
  end_hashes = sym_hashes + symcount;

  for (; sym_hashes < end_hashes; sym_hashes++)
    {
      struct elf_link_hash_entry *sym_hash = *sym_hashes;

      if ((sym_hash->root.type == bfd_link_hash_defined
	   || sym_hash->root.type == bfd_link_hash_defweak)
	  && sym_hash->root.u.def.section == sec)
	{
	  bfd_vma value = sym_hash->root.u.def.value;

	  if (ELF_ST_IS_MICROMIPS (sym_hash->other))
	    value &= MINUS_TWO;
	  if (value > addr)
	    sym_hash->root.u.def.value -= count;
	}
    }

  return true;
}


/* Opcodes needed for microMIPS relaxation as found in
   opcodes/micromips-opc.c.  */

struct opcode_descriptor {
  unsigned long match;
  unsigned long mask;
};

/* The $ra register aka $31.  */

#define RA 31

/* 32-bit instruction format register fields.  */

#define OP32_SREG(opcode) (((opcode) >> 16) & 0x1f)
#define OP32_TREG(opcode) (((opcode) >> 21) & 0x1f)

/* Check if a 5-bit register index can be abbreviated to 3 bits.  */

#define OP16_VALID_REG(r) \
  ((2 <= (r) && (r) <= 7) || (16 <= (r) && (r) <= 17))


/* 32-bit and 16-bit branches.  */

static const struct opcode_descriptor b_insns_32[] = {
  { /* "b",	"p",		*/ 0x40400000, 0xffff0000 }, /* bgez 0 */
  { /* "b",	"p",		*/ 0x94000000, 0xffff0000 }, /* beq 0, 0 */
  { 0, 0 }  /* End marker for find_match().  */
};

static const struct opcode_descriptor bc_insn_32 =
  { /* "bc(1|2)(ft)", "N,p",	*/ 0x42800000, 0xfec30000 };

static const struct opcode_descriptor bz_insn_32 =
  { /* "b(g|l)(e|t)z", "s,p",	*/ 0x40000000, 0xff200000 };

static const struct opcode_descriptor bzal_insn_32 =
  { /* "b(ge|lt)zal", "s,p",	*/ 0x40200000, 0xffa00000 };

static const struct opcode_descriptor beq_insn_32 =
  { /* "b(eq|ne)", "s,t,p",	*/ 0x94000000, 0xdc000000 };

static const struct opcode_descriptor b_insn_16 =
  { /* "b",	"mD",		*/ 0xcc00,     0xfc00 };

static const struct opcode_descriptor bz_insn_16 =
  { /* "b(eq|ne)z", "md,mE",	*/ 0x8c00,     0xdc00 };


/* 32-bit and 16-bit branch EQ and NE zero.  */

/* NOTE: All opcode tables have BEQ/BNE in the same order: first the
   eq and second the ne.  This convention is used when replacing a
   32-bit BEQ/BNE with the 16-bit version.  */

#define BZC32_REG_FIELD(r) (((r) & 0x1f) << 16)

static const struct opcode_descriptor bz_rs_insns_32[] = {
  { /* "beqz",	"s,p",		*/ 0x94000000, 0xffe00000 },
  { /* "bnez",	"s,p",		*/ 0xb4000000, 0xffe00000 },
  { 0, 0 }  /* End marker for find_match().  */
};

static const struct opcode_descriptor bz_rt_insns_32[] = {
  { /* "beqz",	"t,p",		*/ 0x94000000, 0xfc01f000 },
  { /* "bnez",	"t,p",		*/ 0xb4000000, 0xfc01f000 },
  { 0, 0 }  /* End marker for find_match().  */
};

static const struct opcode_descriptor bzc_insns_32[] = {
  { /* "beqzc",	"s,p",		*/ 0x40e00000, 0xffe00000 },
  { /* "bnezc",	"s,p",		*/ 0x40a00000, 0xffe00000 },
  { 0, 0 }  /* End marker for find_match().  */
};

static const struct opcode_descriptor bz_insns_16[] = {
  { /* "beqz",	"md,mE",	*/ 0x8c00,     0xfc00 },
  { /* "bnez",	"md,mE",	*/ 0xac00,     0xfc00 },
  { 0, 0 }  /* End marker for find_match().  */
};

/* Switch between a 5-bit register index and its 3-bit shorthand.  */

#define BZ16_REG(opcode) ((((((opcode) >> 7) & 7) + 0x1e) & 0xf) + 2)
#define BZ16_REG_FIELD(r) (((r) & 7) << 7)


/* 32-bit instructions with a delay slot.  */

static const struct opcode_descriptor jal_insn_32_bd16 =
  { /* "jals",	"a",		*/ 0x74000000, 0xfc000000 };

static const struct opcode_descriptor jal_insn_32_bd32 =
  { /* "jal",	"a",		*/ 0xf4000000, 0xfc000000 };

static const struct opcode_descriptor jal_x_insn_32_bd32 =
  { /* "jal[x]", "a",		*/ 0xf0000000, 0xf8000000 };

static const struct opcode_descriptor j_insn_32 =
  { /* "j",	"a",		*/ 0xd4000000, 0xfc000000 };

static const struct opcode_descriptor jalr_insn_32 =
  { /* "jalr[.hb]", "t,s",	*/ 0x00000f3c, 0xfc00efff };

/* This table can be compacted, because no opcode replacement is made.  */

static const struct opcode_descriptor ds_insns_32_bd16[] = {
  { /* "jals",	"a",		*/ 0x74000000, 0xfc000000 },

  { /* "jalrs[.hb]", "t,s",	*/ 0x00004f3c, 0xfc00efff },
  { /* "b(ge|lt)zals", "s,p",	*/ 0x42200000, 0xffa00000 },

  { /* "b(g|l)(e|t)z", "s,p",	*/ 0x40000000, 0xff200000 },
  { /* "b(eq|ne)", "s,t,p",	*/ 0x94000000, 0xdc000000 },
  { /* "j",	"a",		*/ 0xd4000000, 0xfc000000 },
  { 0, 0 }  /* End marker for find_match().  */
};

/* This table can be compacted, because no opcode replacement is made.  */

static const struct opcode_descriptor ds_insns_32_bd32[] = {
  { /* "jal[x]", "a",		*/ 0xf0000000, 0xf8000000 },

  { /* "jalr[.hb]", "t,s",	*/ 0x00000f3c, 0xfc00efff },
  { /* "b(ge|lt)zal", "s,p",	*/ 0x40200000, 0xffa00000 },
  { 0, 0 }  /* End marker for find_match().  */
};


/* 16-bit instructions with a delay slot.  */

static const struct opcode_descriptor jalr_insn_16_bd16 =
  { /* "jalrs",	"my,mj",	*/ 0x45e0,     0xffe0 };

static const struct opcode_descriptor jalr_insn_16_bd32 =
  { /* "jalr",	"my,mj",	*/ 0x45c0,     0xffe0 };

static const struct opcode_descriptor jr_insn_16 =
  { /* "jr",	"mj",		*/ 0x4580,     0xffe0 };

#define JR16_REG(opcode) ((opcode) & 0x1f)

/* This table can be compacted, because no opcode replacement is made.  */

static const struct opcode_descriptor ds_insns_16_bd16[] = {
  { /* "jalrs",	"my,mj",	*/ 0x45e0,     0xffe0 },

  { /* "b",	"mD",		*/ 0xcc00,     0xfc00 },
  { /* "b(eq|ne)z", "md,mE",	*/ 0x8c00,     0xdc00 },
  { /* "jr",	"mj",		*/ 0x4580,     0xffe0 },
  { 0, 0 }  /* End marker for find_match().  */
};


/* LUI instruction.  */

static const struct opcode_descriptor lui_insn =
 { /* "lui",	"s,u",		*/ 0x41a00000, 0xffe00000 };


/* ADDIU instruction.  */

static const struct opcode_descriptor addiu_insn =
  { /* "addiu",	"t,r,j",	*/ 0x30000000, 0xfc000000 };

static const struct opcode_descriptor addiupc_insn =
  { /* "addiu",	"mb,$pc,mQ",	*/ 0x78000000, 0xfc000000 };

#define ADDIUPC_REG_FIELD(r) \
  (((2 <= (r) && (r) <= 7) ? (r) : ((r) - 16)) << 23)


/* Relaxable instructions in a JAL delay slot: MOVE.  */

/* The 16-bit move has rd in 9:5 and rs in 4:0.  The 32-bit moves
   (ADDU, OR) have rd in 15:11 and rs in 10:16.  */
#define MOVE32_RD(opcode) (((opcode) >> 11) & 0x1f)
#define MOVE32_RS(opcode) (((opcode) >> 16) & 0x1f)

#define MOVE16_RD_FIELD(r) (((r) & 0x1f) << 5)
#define MOVE16_RS_FIELD(r) (((r) & 0x1f)     )

static const struct opcode_descriptor move_insns_32[] = {
  { /* "move",	"d,s",		*/ 0x00000290, 0xffe007ff }, /* or   d,s,$0 */
  { /* "move",	"d,s",		*/ 0x00000150, 0xffe007ff }, /* addu d,s,$0 */
  { 0, 0 }  /* End marker for find_match().  */
};

static const struct opcode_descriptor move_insn_16 =
  { /* "move",	"mp,mj",	*/ 0x0c00,     0xfc00 };


/* NOP instructions.  */

static const struct opcode_descriptor nop_insn_32 =
  { /* "nop",	"",		*/ 0x00000000, 0xffffffff };

static const struct opcode_descriptor nop_insn_16 =
  { /* "nop",	"",		*/ 0x0c00,     0xffff };


/* Instruction match support.  */

#define MATCH(opcode, insn) ((opcode & insn.mask) == insn.match)

static int
find_match (unsigned long opcode, const struct opcode_descriptor insn[])
{
  unsigned long indx;

  for (indx = 0; insn[indx].mask != 0; indx++)
    if (MATCH (opcode, insn[indx]))
      return indx;

  return -1;
}


/* Branch and delay slot decoding support.  */

/* If PTR points to what *might* be a 16-bit branch or jump, then
   return the minimum length of its delay slot, otherwise return 0.
   Non-zero results are not definitive as we might be checking against
   the second half of another instruction.  */

static int
check_br16_dslot (bfd *abfd, bfd_byte *ptr)
{
  unsigned long opcode;
  int bdsize;

  opcode = bfd_get_16 (abfd, ptr);
  if (MATCH (opcode, jalr_insn_16_bd32) != 0)
    /* 16-bit branch/jump with a 32-bit delay slot.  */
    bdsize = 4;
  else if (MATCH (opcode, jalr_insn_16_bd16) != 0
	   || find_match (opcode, ds_insns_16_bd16) >= 0)
    /* 16-bit branch/jump with a 16-bit delay slot.  */
    bdsize = 2;
  else
    /* No delay slot.  */
    bdsize = 0;

  return bdsize;
}

/* If PTR points to what *might* be a 32-bit branch or jump, then
   return the minimum length of its delay slot, otherwise return 0.
   Non-zero results are not definitive as we might be checking against
   the second half of another instruction.  */

static int
check_br32_dslot (bfd *abfd, bfd_byte *ptr)
{
  unsigned long opcode;
  int bdsize;

  opcode = bfd_get_micromips_32 (abfd, ptr);
  if (find_match (opcode, ds_insns_32_bd32) >= 0)
    /* 32-bit branch/jump with a 32-bit delay slot.  */
    bdsize = 4;
  else if (find_match (opcode, ds_insns_32_bd16) >= 0)
    /* 32-bit branch/jump with a 16-bit delay slot.  */
    bdsize = 2;
  else
    /* No delay slot.  */
    bdsize = 0;

  return bdsize;
}

/* If PTR points to a 16-bit branch or jump with a 32-bit delay slot
   that doesn't fiddle with REG, then return TRUE, otherwise FALSE.  */

static bool
check_br16 (bfd *abfd, bfd_byte *ptr, unsigned long reg)
{
  unsigned long opcode;

  opcode = bfd_get_16 (abfd, ptr);
  if (MATCH (opcode, b_insn_16)
						/* B16  */
      || (MATCH (opcode, jr_insn_16) && reg != JR16_REG (opcode))
						/* JR16  */
      || (MATCH (opcode, bz_insn_16) && reg != BZ16_REG (opcode))
						/* BEQZ16, BNEZ16  */
      || (MATCH (opcode, jalr_insn_16_bd32)
						/* JALR16  */
	  && reg != JR16_REG (opcode) && reg != RA))
    return true;

  return false;
}

/* If PTR points to a 32-bit branch or jump that doesn't fiddle with REG,
   then return TRUE, otherwise FALSE.  */

static bool
check_br32 (bfd *abfd, bfd_byte *ptr, unsigned long reg)
{
  unsigned long opcode;

  opcode = bfd_get_micromips_32 (abfd, ptr);
  if (MATCH (opcode, j_insn_32)
						/* J  */
      || MATCH (opcode, bc_insn_32)
						/* BC1F, BC1T, BC2F, BC2T  */
      || (MATCH (opcode, jal_x_insn_32_bd32) && reg != RA)
						/* JAL, JALX  */
      || (MATCH (opcode, bz_insn_32) && reg != OP32_SREG (opcode))
						/* BGEZ, BGTZ, BLEZ, BLTZ  */
      || (MATCH (opcode, bzal_insn_32)
						/* BGEZAL, BLTZAL  */
	  && reg != OP32_SREG (opcode) && reg != RA)
      || ((MATCH (opcode, jalr_insn_32) || MATCH (opcode, beq_insn_32))
						/* JALR, JALR.HB, BEQ, BNE  */
	  && reg != OP32_SREG (opcode) && reg != OP32_TREG (opcode)))
    return true;

  return false;
}

/* If the instruction encoding at PTR and relocations [INTERNAL_RELOCS,
   IRELEND) at OFFSET indicate that there must be a compact branch there,
   then return TRUE, otherwise FALSE.  */

static bool
check_relocated_bzc (bfd *abfd, const bfd_byte *ptr, bfd_vma offset,
		     const Elf_Internal_Rela *internal_relocs,
		     const Elf_Internal_Rela *irelend)
{
  const Elf_Internal_Rela *irel;
  unsigned long opcode;

  opcode = bfd_get_micromips_32 (abfd, ptr);
  if (find_match (opcode, bzc_insns_32) < 0)
    return false;

  for (irel = internal_relocs; irel < irelend; irel++)
    if (irel->r_offset == offset
	&& ELF32_R_TYPE (irel->r_info) == R_MICROMIPS_PC16_S1)
      return true;

  return false;
}

/* Bitsize checking.  */
#define IS_BITSIZE(val, N)						\
  (((((val) & ((1ULL << (N)) - 1)) ^ (1ULL << ((N) - 1)))		\
    - (1ULL << ((N) - 1))) == (val))


bool
_bfd_mips_elf_relax_section (bfd *abfd, asection *sec,
			     struct bfd_link_info *link_info,
			     bool *again)
{
  bool insn32 = mips_elf_hash_table (link_info)->insn32;
  Elf_Internal_Shdr *symtab_hdr;
  Elf_Internal_Rela *internal_relocs;
  Elf_Internal_Rela *irel, *irelend;
  bfd_byte *contents = NULL;
  Elf_Internal_Sym *isymbuf = NULL;

  /* Assume nothing changes.  */
  *again = false;

  /* We don't have to do anything for a relocatable link, if
     this section does not have relocs, or if this is not a
     code section.  */

  if (bfd_link_relocatable (link_info)
      || sec->reloc_count == 0
      || (sec->flags & SEC_RELOC) == 0
      || (sec->flags & SEC_HAS_CONTENTS) == 0
      || (sec->flags & SEC_CODE) == 0)
    return true;

  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;

  /* Get a copy of the native relocations.  */
  internal_relocs = (_bfd_elf_link_read_relocs
		     (abfd, sec, NULL, (Elf_Internal_Rela *) NULL,
		      link_info->keep_memory));
  if (internal_relocs == NULL)
    goto error_return;

  /* Walk through them looking for relaxing opportunities.  */
  irelend = internal_relocs + sec->reloc_count;
  for (irel = internal_relocs; irel < irelend; irel++)
    {
      unsigned long r_symndx = ELF32_R_SYM (irel->r_info);
      unsigned int r_type = ELF32_R_TYPE (irel->r_info);
      bool target_is_micromips_code_p;
      unsigned long opcode;
      bfd_vma symval;
      bfd_vma pcrval;
      bfd_byte *ptr;
      int fndopc;

      /* The number of bytes to delete for relaxation and from where
	 to delete these bytes starting at irel->r_offset.  */
      int delcnt = 0;
      int deloff = 0;

      /* If this isn't something that can be relaxed, then ignore
	 this reloc.  */
      if (r_type != R_MICROMIPS_HI16
	  && r_type != R_MICROMIPS_PC16_S1
	  && r_type != R_MICROMIPS_26_S1)
	continue;

      /* Get the section contents if we haven't done so already.  */
      if (contents == NULL)
	{
	  /* Get cached copy if it exists.  */
	  if (elf_section_data (sec)->this_hdr.contents != NULL)
	    contents = elf_section_data (sec)->this_hdr.contents;
	  /* Go get them off disk.  */
	  else if (!bfd_malloc_and_get_section (abfd, sec, &contents))
	    goto error_return;
	}
      ptr = contents + irel->r_offset;

      /* Read this BFD's local symbols if we haven't done so already.  */
      if (isymbuf == NULL && symtab_hdr->sh_info != 0)
	{
	  isymbuf = (Elf_Internal_Sym *) symtab_hdr->contents;
	  if (isymbuf == NULL)
	    isymbuf = bfd_elf_get_elf_syms (abfd, symtab_hdr,
					    symtab_hdr->sh_info, 0,
					    NULL, NULL, NULL);
	  if (isymbuf == NULL)
	    goto error_return;
	}

      /* Get the value of the symbol referred to by the reloc.  */
      if (r_symndx < symtab_hdr->sh_info)
	{
	  /* A local symbol.  */
	  Elf_Internal_Sym *isym;
	  asection *sym_sec;

	  isym = isymbuf + r_symndx;
	  if (isym->st_shndx == SHN_UNDEF)
	    sym_sec = bfd_und_section_ptr;
	  else if (isym->st_shndx == SHN_ABS)
	    sym_sec = bfd_abs_section_ptr;
	  else if (isym->st_shndx == SHN_COMMON)
	    sym_sec = bfd_com_section_ptr;
	  else
	    sym_sec = bfd_section_from_elf_index (abfd, isym->st_shndx);
	  symval = (isym->st_value
		    + sym_sec->output_section->vma
		    + sym_sec->output_offset);
	  target_is_micromips_code_p = ELF_ST_IS_MICROMIPS (isym->st_other);
	}
      else
	{
	  unsigned long indx;
	  struct elf_link_hash_entry *h;

	  /* An external symbol.  */
	  indx = r_symndx - symtab_hdr->sh_info;
	  h = elf_sym_hashes (abfd)[indx];
	  BFD_ASSERT (h != NULL);

	  if (h->root.type != bfd_link_hash_defined
	      && h->root.type != bfd_link_hash_defweak)
	    /* This appears to be a reference to an undefined
	       symbol.  Just ignore it -- it will be caught by the
	       regular reloc processing.  */
	    continue;

	  symval = (h->root.u.def.value
		    + h->root.u.def.section->output_section->vma
		    + h->root.u.def.section->output_offset);
	  target_is_micromips_code_p = (!h->needs_plt
					&& ELF_ST_IS_MICROMIPS (h->other));
	}


      /* For simplicity of coding, we are going to modify the
	 section contents, the section relocs, and the BFD symbol
	 table.  We must tell the rest of the code not to free up this
	 information.  It would be possible to instead create a table
	 of changes which have to be made, as is done in coff-mips.c;
	 that would be more work, but would require less memory when
	 the linker is run.  */

      /* Only 32-bit instructions relaxed.  */
      if (irel->r_offset + 4 > sec->size)
	continue;

      opcode = bfd_get_micromips_32 (abfd, ptr);

      /* This is the pc-relative distance from the instruction the
	 relocation is applied to, to the symbol referred.  */
      pcrval = (symval
		- (sec->output_section->vma + sec->output_offset)
		- irel->r_offset);

      /* R_MICROMIPS_HI16 / LUI relaxation to nil, performing relaxation
	 of corresponding R_MICROMIPS_LO16 to R_MICROMIPS_HI0_LO16 or
	 R_MICROMIPS_PC23_S2.  The R_MICROMIPS_PC23_S2 condition is

	   (symval % 4 == 0 && IS_BITSIZE (pcrval, 25))

	 where pcrval has first to be adjusted to apply against the LO16
	 location (we make the adjustment later on, when we have figured
	 out the offset).  */
      if (r_type == R_MICROMIPS_HI16 && MATCH (opcode, lui_insn))
	{
	  bool bzc = false;
	  unsigned long nextopc;
	  unsigned long reg;
	  bfd_vma offset;

	  /* Give up if the previous reloc was a HI16 against this symbol
	     too.  */
	  if (irel > internal_relocs
	      && ELF32_R_TYPE (irel[-1].r_info) == R_MICROMIPS_HI16
	      && ELF32_R_SYM (irel[-1].r_info) == r_symndx)
	    continue;

	  /* Or if the next reloc is not a LO16 against this symbol.  */
	  if (irel + 1 >= irelend
	      || ELF32_R_TYPE (irel[1].r_info) != R_MICROMIPS_LO16
	      || ELF32_R_SYM (irel[1].r_info) != r_symndx)
	    continue;

	  /* Or if the second next reloc is a LO16 against this symbol too.  */
	  if (irel + 2 >= irelend
	      && ELF32_R_TYPE (irel[2].r_info) == R_MICROMIPS_LO16
	      && ELF32_R_SYM (irel[2].r_info) == r_symndx)
	    continue;

	  /* See if the LUI instruction *might* be in a branch delay slot.
	     We check whether what looks like a 16-bit branch or jump is
	     actually an immediate argument to a compact branch, and let
	     it through if so.  */
	  if (irel->r_offset >= 2
	      && check_br16_dslot (abfd, ptr - 2)
	      && !(irel->r_offset >= 4
		   && (bzc = check_relocated_bzc (abfd,
						  ptr - 4, irel->r_offset - 4,
						  internal_relocs, irelend))))
	    continue;
	  if (irel->r_offset >= 4
	      && !bzc
	      && check_br32_dslot (abfd, ptr - 4))
	    continue;

	  reg = OP32_SREG (opcode);

	  /* We only relax adjacent instructions or ones separated with
	     a branch or jump that has a delay slot.  The branch or jump
	     must not fiddle with the register used to hold the address.
	     Subtract 4 for the LUI itself.  */
	  offset = irel[1].r_offset - irel[0].r_offset;
	  switch (offset - 4)
	    {
	    case 0:
	      break;
	    case 2:
	      if (check_br16 (abfd, ptr + 4, reg))
		break;
	      continue;
	    case 4:
	      if (check_br32 (abfd, ptr + 4, reg))
		break;
	      continue;
	    default:
	      continue;
	    }

	  nextopc = bfd_get_micromips_32 (abfd, contents + irel[1].r_offset);

	  /* Give up unless the same register is used with both
	     relocations.  */
	  if (OP32_SREG (nextopc) != reg)
	    continue;

	  /* Now adjust pcrval, subtracting the offset to the LO16 reloc
	     and rounding up to take masking of the two LSBs into account.  */
	  pcrval = ((pcrval - offset + 3) | 3) ^ 3;

	  /* R_MICROMIPS_LO16 relaxation to R_MICROMIPS_HI0_LO16.  */
	  if (IS_BITSIZE (symval, 16))
	    {
	      /* Fix the relocation's type.  */
	      irel[1].r_info = ELF32_R_INFO (r_symndx, R_MICROMIPS_HI0_LO16);

	      /* Instructions using R_MICROMIPS_LO16 have the base or
		 source register in bits 20:16.  This register becomes $0
		 (zero) as the result of the R_MICROMIPS_HI16 being 0.  */
	      nextopc &= ~0x001f0000;
	      bfd_put_16 (abfd, (nextopc >> 16) & 0xffff,
			  contents + irel[1].r_offset);
	    }

	  /* R_MICROMIPS_LO16 / ADDIU relaxation to R_MICROMIPS_PC23_S2.
	     We add 4 to take LUI deletion into account while checking
	     the PC-relative distance.  */
	  else if (symval % 4 == 0
		   && IS_BITSIZE (pcrval + 4, 25)
		   && MATCH (nextopc, addiu_insn)
		   && OP32_TREG (nextopc) == OP32_SREG (nextopc)
		   && OP16_VALID_REG (OP32_TREG (nextopc)))
	    {
	      /* Fix the relocation's type.  */
	      irel[1].r_info = ELF32_R_INFO (r_symndx, R_MICROMIPS_PC23_S2);

	      /* Replace ADDIU with the ADDIUPC version.  */
	      nextopc = (addiupc_insn.match
			 | ADDIUPC_REG_FIELD (OP32_TREG (nextopc)));

	      bfd_put_micromips_32 (abfd, nextopc,
				    contents + irel[1].r_offset);
	    }

	  /* Can't do anything, give up, sigh...  */
	  else
	    continue;

	  /* Fix the relocation's type.  */
	  irel->r_info = ELF32_R_INFO (r_symndx, R_MIPS_NONE);

	  /* Delete the LUI instruction: 4 bytes at irel->r_offset.  */
	  delcnt = 4;
	  deloff = 0;
	}

      /* Compact branch relaxation -- due to the multitude of macros
	 employed by the compiler/assembler, compact branches are not
	 always generated.  Obviously, this can/will be fixed elsewhere,
	 but there is no drawback in double checking it here.  */
      else if (r_type == R_MICROMIPS_PC16_S1
	       && irel->r_offset + 5 < sec->size
	       && ((fndopc = find_match (opcode, bz_rs_insns_32)) >= 0
		   || (fndopc = find_match (opcode, bz_rt_insns_32)) >= 0)
	       && ((!insn32
		    && (delcnt = MATCH (bfd_get_16 (abfd, ptr + 4),
					nop_insn_16) ? 2 : 0))
		   || (irel->r_offset + 7 < sec->size
		       && (delcnt = MATCH (bfd_get_micromips_32 (abfd,
								 ptr + 4),
					   nop_insn_32) ? 4 : 0))))
	{
	  unsigned long reg;

	  reg = OP32_SREG (opcode) ? OP32_SREG (opcode) : OP32_TREG (opcode);

	  /* Replace BEQZ/BNEZ with the compact version.  */
	  opcode = (bzc_insns_32[fndopc].match
		    | BZC32_REG_FIELD (reg)
		    | (opcode & 0xffff));		/* Addend value.  */

	  bfd_put_micromips_32 (abfd, opcode, ptr);

	  /* Delete the delay slot NOP: two or four bytes from
	     irel->offset + 4; delcnt has already been set above.  */
	  deloff = 4;
	}

      /* R_MICROMIPS_PC16_S1 relaxation to R_MICROMIPS_PC10_S1.  We need
	 to check the distance from the next instruction, so subtract 2.  */
      else if (!insn32
	       && r_type == R_MICROMIPS_PC16_S1
	       && IS_BITSIZE (pcrval - 2, 11)
	       && find_match (opcode, b_insns_32) >= 0)
	{
	  /* Fix the relocation's type.  */
	  irel->r_info = ELF32_R_INFO (r_symndx, R_MICROMIPS_PC10_S1);

	  /* Replace the 32-bit opcode with a 16-bit opcode.  */
	  bfd_put_16 (abfd,
		      (b_insn_16.match
		       | (opcode & 0x3ff)),		/* Addend value.  */
		      ptr);

	  /* Delete 2 bytes from irel->r_offset + 2.  */
	  delcnt = 2;
	  deloff = 2;
	}

      /* R_MICROMIPS_PC16_S1 relaxation to R_MICROMIPS_PC7_S1.  We need
	 to check the distance from the next instruction, so subtract 2.  */
      else if (!insn32
	       && r_type == R_MICROMIPS_PC16_S1
	       && IS_BITSIZE (pcrval - 2, 8)
	       && (((fndopc = find_match (opcode, bz_rs_insns_32)) >= 0
		    && OP16_VALID_REG (OP32_SREG (opcode)))
		   || ((fndopc = find_match (opcode, bz_rt_insns_32)) >= 0
		       && OP16_VALID_REG (OP32_TREG (opcode)))))
	{
	  unsigned long reg;

	  reg = OP32_SREG (opcode) ? OP32_SREG (opcode) : OP32_TREG (opcode);

	  /* Fix the relocation's type.  */
	  irel->r_info = ELF32_R_INFO (r_symndx, R_MICROMIPS_PC7_S1);

	  /* Replace the 32-bit opcode with a 16-bit opcode.  */
	  bfd_put_16 (abfd,
		      (bz_insns_16[fndopc].match
		       | BZ16_REG_FIELD (reg)
		       | (opcode & 0x7f)),		/* Addend value.  */
		      ptr);

	  /* Delete 2 bytes from irel->r_offset + 2.  */
	  delcnt = 2;
	  deloff = 2;
	}

      /* R_MICROMIPS_26_S1 -- JAL to JALS relaxation for microMIPS targets.  */
      else if (!insn32
	       && r_type == R_MICROMIPS_26_S1
	       && target_is_micromips_code_p
	       && irel->r_offset + 7 < sec->size
	       && MATCH (opcode, jal_insn_32_bd32))
	{
	  unsigned long n32opc;
	  bool relaxed = false;

	  n32opc = bfd_get_micromips_32 (abfd, ptr + 4);

	  if (MATCH (n32opc, nop_insn_32))
	    {
	      /* Replace delay slot 32-bit NOP with a 16-bit NOP.  */
	      bfd_put_16 (abfd, nop_insn_16.match, ptr + 4);

	      relaxed = true;
	    }
	  else if (find_match (n32opc, move_insns_32) >= 0)
	    {
	      /* Replace delay slot 32-bit MOVE with 16-bit MOVE.  */
	      bfd_put_16 (abfd,
			  (move_insn_16.match
			   | MOVE16_RD_FIELD (MOVE32_RD (n32opc))
			   | MOVE16_RS_FIELD (MOVE32_RS (n32opc))),
			  ptr + 4);

	      relaxed = true;
	    }
	  /* Other 32-bit instructions relaxable to 16-bit
	     instructions will be handled here later.  */

	  if (relaxed)
	    {
	      /* JAL with 32-bit delay slot that is changed to a JALS
		 with 16-bit delay slot.  */
	      bfd_put_micromips_32 (abfd, jal_insn_32_bd16.match, ptr);

	      /* Delete 2 bytes from irel->r_offset + 6.  */
	      delcnt = 2;
	      deloff = 6;
	    }
	}

      if (delcnt != 0)
	{
	  /* Note that we've changed the relocs, section contents, etc.  */
	  elf_section_data (sec)->relocs = internal_relocs;
	  elf_section_data (sec)->this_hdr.contents = contents;
	  symtab_hdr->contents = (unsigned char *) isymbuf;

	  /* Delete bytes depending on the delcnt and deloff.  */
	  if (!mips_elf_relax_delete_bytes (abfd, sec,
					    irel->r_offset + deloff, delcnt))
	    goto error_return;

	  /* That will change things, so we should relax again.
	     Note that this is not required, and it may be slow.  */
	  *again = true;
	}
    }

  if (isymbuf != NULL
      && symtab_hdr->contents != (unsigned char *) isymbuf)
    {
      if (! link_info->keep_memory)
	free (isymbuf);
      else
	{
	  /* Cache the symbols for elf_link_input_bfd.  */
	  symtab_hdr->contents = (unsigned char *) isymbuf;
	}
    }

  if (contents != NULL
      && elf_section_data (sec)->this_hdr.contents != contents)
    {
      if (! link_info->keep_memory)
	free (contents);
      else
	{
	  /* Cache the section contents for elf_link_input_bfd.  */
	  elf_section_data (sec)->this_hdr.contents = contents;
	}
    }

  if (elf_section_data (sec)->relocs != internal_relocs)
    free (internal_relocs);

  return true;

 error_return:
  if (symtab_hdr->contents != (unsigned char *) isymbuf)
    free (isymbuf);
  if (elf_section_data (sec)->this_hdr.contents != contents)
    free (contents);
  if (elf_section_data (sec)->relocs != internal_relocs)
    free (internal_relocs);

  return false;
}

/* Create a MIPS ELF linker hash table.  */

struct bfd_link_hash_table *
_bfd_mips_elf_link_hash_table_create (bfd *abfd)
{
  struct mips_elf_link_hash_table *ret;
  size_t amt = sizeof (struct mips_elf_link_hash_table);

  ret = bfd_zmalloc (amt);
  if (ret == NULL)
    return NULL;

  if (!_bfd_elf_link_hash_table_init (&ret->root, abfd,
				      mips_elf_link_hash_newfunc,
				      sizeof (struct mips_elf_link_hash_entry),
				      MIPS_ELF_DATA))
    {
      free (ret);
      return NULL;
    }
  ret->root.init_plt_refcount.plist = NULL;
  ret->root.init_plt_offset.plist = NULL;

  return &ret->root.root;
}

/* Likewise, but indicate that the target is VxWorks.  */

struct bfd_link_hash_table *
_bfd_mips_vxworks_link_hash_table_create (bfd *abfd)
{
  struct bfd_link_hash_table *ret;

  ret = _bfd_mips_elf_link_hash_table_create (abfd);
  if (ret)
    {
      struct mips_elf_link_hash_table *htab;

      htab = (struct mips_elf_link_hash_table *) ret;
      htab->use_plts_and_copy_relocs = true;
    }
  return ret;
}

/* A function that the linker calls if we are allowed to use PLTs
   and copy relocs.  */

void
_bfd_mips_elf_use_plts_and_copy_relocs (struct bfd_link_info *info)
{
  mips_elf_hash_table (info)->use_plts_and_copy_relocs = true;
}

/* A function that the linker calls to select between all or only
   32-bit microMIPS instructions, and between making or ignoring
   branch relocation checks for invalid transitions between ISA modes.
   Also record whether we have been configured for a GNU target.  */

void
_bfd_mips_elf_linker_flags (struct bfd_link_info *info, bool insn32,
			    bool ignore_branch_isa,
			    bool gnu_target)
{
  mips_elf_hash_table (info)->insn32 = insn32;
  mips_elf_hash_table (info)->ignore_branch_isa = ignore_branch_isa;
  mips_elf_hash_table (info)->gnu_target = gnu_target;
}

/* A function that the linker calls to enable use of compact branches in
   linker generated code for MIPSR6.  */

void
_bfd_mips_elf_compact_branches (struct bfd_link_info *info, bool on)
{
  mips_elf_hash_table (info)->compact_branches = on;
}


/* Structure for saying that BFD machine EXTENSION extends BASE.  */

struct mips_mach_extension
{
  unsigned long extension, base;
};

/* An array that maps 64-bit architectures to the corresponding 32-bit
   architectures.  */
static const struct mips_mach_extension mips_mach_32_64[] =
{
  { bfd_mach_mipsisa64r6, bfd_mach_mipsisa32r6 },
  { bfd_mach_mipsisa64r5, bfd_mach_mipsisa32r5 },
  { bfd_mach_mipsisa64r3, bfd_mach_mipsisa32r3 },
  { bfd_mach_mipsisa64r2, bfd_mach_mipsisa32r2 },
  { bfd_mach_mipsisa64,   bfd_mach_mipsisa32 }
};

/* An array describing how BFD machines relate to one another.  The entries
   are ordered topologically with MIPS I extensions listed last.  */

static const struct mips_mach_extension mips_mach_extensions[] =
{
  /* MIPS64r2 extensions.  */
  { bfd_mach_mips_octeon3, bfd_mach_mips_octeon2 },
  { bfd_mach_mips_octeon2, bfd_mach_mips_octeonp },
  { bfd_mach_mips_octeonp, bfd_mach_mips_octeon },
  { bfd_mach_mips_octeon, bfd_mach_mipsisa64r2 },
  { bfd_mach_mips_gs264e, bfd_mach_mips_gs464e },
  { bfd_mach_mips_gs464e, bfd_mach_mips_gs464 },
  { bfd_mach_mips_gs464, bfd_mach_mipsisa64r2 },

  /* MIPS64 extensions.  */
  { bfd_mach_mipsisa64r2, bfd_mach_mipsisa64 },
  { bfd_mach_mips_sb1, bfd_mach_mipsisa64 },
  { bfd_mach_mips_xlr, bfd_mach_mipsisa64 },

  /* MIPS V extensions.  */
  { bfd_mach_mipsisa64, bfd_mach_mips5 },

  /* R10000 extensions.  */
  { bfd_mach_mips12000, bfd_mach_mips10000 },
  { bfd_mach_mips14000, bfd_mach_mips10000 },
  { bfd_mach_mips16000, bfd_mach_mips10000 },

  /* R5000 extensions.  Note: the vr5500 ISA is an extension of the core
     vr5400 ISA, but doesn't include the multimedia stuff.  It seems
     better to allow vr5400 and vr5500 code to be merged anyway, since
     many libraries will just use the core ISA.  Perhaps we could add
     some sort of ASE flag if this ever proves a problem.  */
  { bfd_mach_mips5500, bfd_mach_mips5400 },
  { bfd_mach_mips5400, bfd_mach_mips5000 },

  /* MIPS IV extensions.  */
  { bfd_mach_mips5, bfd_mach_mips8000 },
  { bfd_mach_mips10000, bfd_mach_mips8000 },
  { bfd_mach_mips5000, bfd_mach_mips8000 },
  { bfd_mach_mips7000, bfd_mach_mips8000 },
  { bfd_mach_mips9000, bfd_mach_mips8000 },

  /* VR4100 extensions.  */
  { bfd_mach_mips4120, bfd_mach_mips4100 },
  { bfd_mach_mips4111, bfd_mach_mips4100 },

  /* MIPS III extensions.  */
  { bfd_mach_mips_loongson_2e, bfd_mach_mips4000 },
  { bfd_mach_mips_loongson_2f, bfd_mach_mips4000 },
  { bfd_mach_mips8000, bfd_mach_mips4000 },
  { bfd_mach_mips4650, bfd_mach_mips4000 },
  { bfd_mach_mips4600, bfd_mach_mips4000 },
  { bfd_mach_mips4400, bfd_mach_mips4000 },
  { bfd_mach_mips4300, bfd_mach_mips4000 },
  { bfd_mach_mips4100, bfd_mach_mips4000 },
  { bfd_mach_mips5900, bfd_mach_mips4000 },

  /* MIPS32r3 extensions.  */
  { bfd_mach_mips_interaptiv_mr2, bfd_mach_mipsisa32r3 },

  /* MIPS32r2 extensions.  */
  { bfd_mach_mipsisa32r3, bfd_mach_mipsisa32r2 },

  /* MIPS32 extensions.  */
  { bfd_mach_mipsisa32r2, bfd_mach_mipsisa32 },

  /* MIPS II extensions.  */
  { bfd_mach_mips4000, bfd_mach_mips6000 },
  { bfd_mach_mipsisa32, bfd_mach_mips6000 },
  { bfd_mach_mips4010, bfd_mach_mips6000 },
  { bfd_mach_mips_allegrex, bfd_mach_mips6000 },

  /* MIPS I extensions.  */
  { bfd_mach_mips6000, bfd_mach_mips3000 },
  { bfd_mach_mips3900, bfd_mach_mips3000 }
};

/* Return true if bfd machine EXTENSION is the same as BASE, or if
   EXTENSION is the 64-bit equivalent of a 32-bit BASE.  */

static bool
mips_mach_extends_32_64 (unsigned long base, unsigned long extension)
{
  size_t i;

  if (extension == base)
    return true;

  for (i = 0; i < ARRAY_SIZE (mips_mach_32_64); i++)
    if (extension == mips_mach_32_64[i].extension)
      return base == mips_mach_32_64[i].base;

  return false;
}

static bool
mips_mach_extends_p (unsigned long base, unsigned long extension)
{
  size_t i;

  if (mips_mach_extends_32_64 (base, extension))
    return true;

  for (i = 0; i < ARRAY_SIZE (mips_mach_extensions); i++)
    if (extension == mips_mach_extensions[i].extension)
      {
	extension = mips_mach_extensions[i].base;
	if (mips_mach_extends_32_64 (base, extension))
	  return true;
      }

  return false;
}

/* Return the BFD mach for each .MIPS.abiflags ISA Extension.  */

static unsigned long
bfd_mips_isa_ext_mach (unsigned int isa_ext)
{
  switch (isa_ext)
    {
    case AFL_EXT_3900:	      return bfd_mach_mips3900;
    case AFL_EXT_4010:	      return bfd_mach_mips4010;
    case AFL_EXT_4100:	      return bfd_mach_mips4100;
    case AFL_EXT_4111:	      return bfd_mach_mips4111;
    case AFL_EXT_4120:	      return bfd_mach_mips4120;
    case AFL_EXT_4650:	      return bfd_mach_mips4650;
    case AFL_EXT_5400:	      return bfd_mach_mips5400;
    case AFL_EXT_5500:	      return bfd_mach_mips5500;
    case AFL_EXT_5900:	      return bfd_mach_mips5900;
    case AFL_EXT_10000:	      return bfd_mach_mips10000;
    case AFL_EXT_LOONGSON_2E: return bfd_mach_mips_loongson_2e;
    case AFL_EXT_LOONGSON_2F: return bfd_mach_mips_loongson_2f;
    case AFL_EXT_SB1:	      return bfd_mach_mips_sb1;
    case AFL_EXT_OCTEON:      return bfd_mach_mips_octeon;
    case AFL_EXT_OCTEONP:     return bfd_mach_mips_octeonp;
    case AFL_EXT_OCTEON2:     return bfd_mach_mips_octeon2;
    case AFL_EXT_XLR:	      return bfd_mach_mips_xlr;
    default:		      return bfd_mach_mips3000;
    }
}

/* Return the .MIPS.abiflags value representing each ISA Extension.  */

unsigned int
bfd_mips_isa_ext (bfd *abfd)
{
  switch (bfd_get_mach (abfd))
    {
    case bfd_mach_mips3900:	    return AFL_EXT_3900;
    case bfd_mach_mips4010:	    return AFL_EXT_4010;
    case bfd_mach_mips4100:	    return AFL_EXT_4100;
    case bfd_mach_mips4111:	    return AFL_EXT_4111;
    case bfd_mach_mips4120:	    return AFL_EXT_4120;
    case bfd_mach_mips4650:	    return AFL_EXT_4650;
    case bfd_mach_mips5400:	    return AFL_EXT_5400;
    case bfd_mach_mips5500:	    return AFL_EXT_5500;
    case bfd_mach_mips5900:	    return AFL_EXT_5900;
    case bfd_mach_mips10000:	    return AFL_EXT_10000;
    case bfd_mach_mips_loongson_2e: return AFL_EXT_LOONGSON_2E;
    case bfd_mach_mips_loongson_2f: return AFL_EXT_LOONGSON_2F;
    case bfd_mach_mips_sb1:	    return AFL_EXT_SB1;
    case bfd_mach_mips_octeon:	    return AFL_EXT_OCTEON;
    case bfd_mach_mips_octeonp:	    return AFL_EXT_OCTEONP;
    case bfd_mach_mips_octeon3:	    return AFL_EXT_OCTEON3;
    case bfd_mach_mips_octeon2:	    return AFL_EXT_OCTEON2;
    case bfd_mach_mips_xlr:	    return AFL_EXT_XLR;
    case bfd_mach_mips_interaptiv_mr2:
      return AFL_EXT_INTERAPTIV_MR2;
    default:			    return 0;
    }
}

/* Encode ISA level and revision as a single value.  */
#define LEVEL_REV(LEV,REV) ((LEV) << 3 | (REV))

/* Decode a single value into level and revision.  */
#define ISA_LEVEL(LEVREV)  ((LEVREV) >> 3)
#define ISA_REV(LEVREV)    ((LEVREV) & 0x7)

/* Update the isa_level, isa_rev, isa_ext fields of abiflags.  */

static void
update_mips_abiflags_isa (bfd *abfd, Elf_Internal_ABIFlags_v0 *abiflags)
{
  int new_isa = 0;
  switch (elf_elfheader (abfd)->e_flags & EF_MIPS_ARCH)
    {
    case E_MIPS_ARCH_1:    new_isa = LEVEL_REV (1, 0); break;
    case E_MIPS_ARCH_2:    new_isa = LEVEL_REV (2, 0); break;
    case E_MIPS_ARCH_3:    new_isa = LEVEL_REV (3, 0); break;
    case E_MIPS_ARCH_4:    new_isa = LEVEL_REV (4, 0); break;
    case E_MIPS_ARCH_5:    new_isa = LEVEL_REV (5, 0); break;
    case E_MIPS_ARCH_32:   new_isa = LEVEL_REV (32, 1); break;
    case E_MIPS_ARCH_32R2: new_isa = LEVEL_REV (32, 2); break;
    case E_MIPS_ARCH_32R6: new_isa = LEVEL_REV (32, 6); break;
    case E_MIPS_ARCH_64:   new_isa = LEVEL_REV (64, 1); break;
    case E_MIPS_ARCH_64R2: new_isa = LEVEL_REV (64, 2); break;
    case E_MIPS_ARCH_64R6: new_isa = LEVEL_REV (64, 6); break;
    default:
      _bfd_error_handler
	/* xgettext:c-format */
	(_("%pB: unknown architecture %s"),
	 abfd, bfd_printable_name (abfd));
    }

  if (new_isa > LEVEL_REV (abiflags->isa_level, abiflags->isa_rev))
    {
      abiflags->isa_level = ISA_LEVEL (new_isa);
      abiflags->isa_rev = ISA_REV (new_isa);
    }

  /* Update the isa_ext if ABFD describes a further extension.  */
  if (mips_mach_extends_p (bfd_mips_isa_ext_mach (abiflags->isa_ext),
			   bfd_get_mach (abfd)))
    abiflags->isa_ext = bfd_mips_isa_ext (abfd);
}

/* Return true if the given ELF header flags describe a 32-bit binary.  */

static bool
mips_32bit_flags_p (flagword flags)
{
  return ((flags & EF_MIPS_32BITMODE) != 0
	  || (flags & EF_MIPS_ABI) == E_MIPS_ABI_O32
	  || (flags & EF_MIPS_ABI) == E_MIPS_ABI_EABI32
	  || (flags & EF_MIPS_ARCH) == E_MIPS_ARCH_1
	  || (flags & EF_MIPS_ARCH) == E_MIPS_ARCH_2
	  || (flags & EF_MIPS_ARCH) == E_MIPS_ARCH_32
	  || (flags & EF_MIPS_ARCH) == E_MIPS_ARCH_32R2
	  || (flags & EF_MIPS_ARCH) == E_MIPS_ARCH_32R6);
}

/* Infer the content of the ABI flags based on the elf header.  */

static void
infer_mips_abiflags (bfd *abfd, Elf_Internal_ABIFlags_v0* abiflags)
{
  obj_attribute *in_attr;

  memset (abiflags, 0, sizeof (Elf_Internal_ABIFlags_v0));
  update_mips_abiflags_isa (abfd, abiflags);

  if (mips_32bit_flags_p (elf_elfheader (abfd)->e_flags))
    abiflags->gpr_size = AFL_REG_32;
  else
    abiflags->gpr_size = AFL_REG_64;

  abiflags->cpr1_size = AFL_REG_NONE;

  in_attr = elf_known_obj_attributes (abfd)[OBJ_ATTR_GNU];
  abiflags->fp_abi = in_attr[Tag_GNU_MIPS_ABI_FP].i;

  if (abiflags->fp_abi == Val_GNU_MIPS_ABI_FP_SINGLE
      || abiflags->fp_abi == Val_GNU_MIPS_ABI_FP_XX
      || (abiflags->fp_abi == Val_GNU_MIPS_ABI_FP_DOUBLE
	  && abiflags->gpr_size == AFL_REG_32))
    abiflags->cpr1_size = AFL_REG_32;
  else if (abiflags->fp_abi == Val_GNU_MIPS_ABI_FP_DOUBLE
	   || abiflags->fp_abi == Val_GNU_MIPS_ABI_FP_64
	   || abiflags->fp_abi == Val_GNU_MIPS_ABI_FP_64A)
    abiflags->cpr1_size = AFL_REG_64;

  abiflags->cpr2_size = AFL_REG_NONE;

  if (elf_elfheader (abfd)->e_flags & EF_MIPS_ARCH_ASE_MDMX)
    abiflags->ases |= AFL_ASE_MDMX;
  if (elf_elfheader (abfd)->e_flags & EF_MIPS_ARCH_ASE_M16)
    abiflags->ases |= AFL_ASE_MIPS16;
  if (elf_elfheader (abfd)->e_flags & EF_MIPS_ARCH_ASE_MICROMIPS)
    abiflags->ases |= AFL_ASE_MICROMIPS;

  if (abiflags->fp_abi != Val_GNU_MIPS_ABI_FP_ANY
      && abiflags->fp_abi != Val_GNU_MIPS_ABI_FP_SOFT
      && abiflags->fp_abi != Val_GNU_MIPS_ABI_FP_64A
      && abiflags->isa_level >= 32
      && abiflags->ases != AFL_ASE_LOONGSON_EXT)
    abiflags->flags1 |= AFL_FLAGS1_ODDSPREG;
}

/* We need to use a special link routine to handle the .reginfo and
   the .mdebug sections.  We need to merge all instances of these
   sections together, not write them all out sequentially.  */

bool
_bfd_mips_elf_final_link (bfd *abfd, struct bfd_link_info *info)
{
  asection *o;
  struct bfd_link_order *p;
  asection *reginfo_sec, *mdebug_sec, *gptab_data_sec, *gptab_bss_sec;
  asection *rtproc_sec, *abiflags_sec;
  Elf32_RegInfo reginfo;
  struct ecoff_debug_info debug;
  struct mips_htab_traverse_info hti;
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  const struct ecoff_debug_swap *swap = bed->elf_backend_ecoff_debug_swap;
  HDRR *symhdr = &debug.symbolic_header;
  void *mdebug_handle = NULL;
  asection *s;
  EXTR esym;
  unsigned int i;
  bfd_size_type amt;
  struct mips_elf_link_hash_table *htab;

  static const char * const secname[] =
  {
    ".text", ".init", ".fini", ".data",
    ".rodata", ".sdata", ".sbss", ".bss"
  };
  static const int sc[] =
  {
    scText, scInit, scFini, scData,
    scRData, scSData, scSBss, scBss
  };

  htab = mips_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);

  /* Sort the dynamic symbols so that those with GOT entries come after
     those without.  */
  if (!mips_elf_sort_hash_table (abfd, info))
    return false;

  /* Create any scheduled LA25 stubs.  */
  hti.info = info;
  hti.output_bfd = abfd;
  hti.error = false;
  htab_traverse (htab->la25_stubs, mips_elf_create_la25_stub, &hti);
  if (hti.error)
    return false;

  /* Get a value for the GP register.  */
  if (elf_gp (abfd) == 0)
    {
      struct bfd_link_hash_entry *h;

      h = bfd_link_hash_lookup (info->hash, "_gp", false, false, true);
      if (h != NULL && h->type == bfd_link_hash_defined)
	elf_gp (abfd) = (h->u.def.value
			 + h->u.def.section->output_section->vma
			 + h->u.def.section->output_offset);
      else if (htab->root.target_os == is_vxworks
	       && (h = bfd_link_hash_lookup (info->hash,
					     "_GLOBAL_OFFSET_TABLE_",
					     false, false, true))
	       && h->type == bfd_link_hash_defined)
	elf_gp (abfd) = (h->u.def.section->output_section->vma
			 + h->u.def.section->output_offset
			 + h->u.def.value);
      else if (bfd_link_relocatable (info))
	{
	  bfd_vma lo = MINUS_ONE;

	  /* Find the GP-relative section with the lowest offset.  */
	  for (o = abfd->sections; o != NULL; o = o->next)
	    if (o->vma < lo
		&& (elf_section_data (o)->this_hdr.sh_flags & SHF_MIPS_GPREL))
	      lo = o->vma;

	  /* And calculate GP relative to that.  */
	  elf_gp (abfd) = lo + ELF_MIPS_GP_OFFSET (info);
	}
      else
	{
	  /* If the relocate_section function needs to do a reloc
	     involving the GP value, it should make a reloc_dangerous
	     callback to warn that GP is not defined.  */
	}
    }

  /* Go through the sections and collect the .reginfo and .mdebug
     information.  */
  abiflags_sec = NULL;
  reginfo_sec = NULL;
  mdebug_sec = NULL;
  gptab_data_sec = NULL;
  gptab_bss_sec = NULL;
  for (o = abfd->sections; o != NULL; o = o->next)
    {
      if (strcmp (o->name, ".MIPS.abiflags") == 0)
	{
	  /* We have found the .MIPS.abiflags section in the output file.
	     Look through all the link_orders comprising it and remove them.
	     The data is merged in _bfd_mips_elf_merge_private_bfd_data.  */
	  for (p = o->map_head.link_order; p != NULL; p = p->next)
	    {
	      asection *input_section;

	      if (p->type != bfd_indirect_link_order)
		{
		  if (p->type == bfd_data_link_order)
		    continue;
		  abort ();
		}

	      input_section = p->u.indirect.section;

	      /* Hack: reset the SEC_HAS_CONTENTS flag so that
		 elf_link_input_bfd ignores this section.  */
	      input_section->flags &= ~SEC_HAS_CONTENTS;
	    }

	  /* Size has been set in _bfd_mips_elf_always_size_sections.  */
	  BFD_ASSERT(o->size == sizeof (Elf_External_ABIFlags_v0));

	  /* Skip this section later on (I don't think this currently
	     matters, but someday it might).  */
	  o->map_head.link_order = NULL;

	  abiflags_sec = o;
	}

      if (strcmp (o->name, ".reginfo") == 0)
	{
	  memset (&reginfo, 0, sizeof reginfo);

	  /* We have found the .reginfo section in the output file.
	     Look through all the link_orders comprising it and merge
	     the information together.  */
	  for (p = o->map_head.link_order; p != NULL; p = p->next)
	    {
	      asection *input_section;
	      bfd *input_bfd;
	      Elf32_External_RegInfo ext;
	      Elf32_RegInfo sub;
	      bfd_size_type sz;

	      if (p->type != bfd_indirect_link_order)
		{
		  if (p->type == bfd_data_link_order)
		    continue;
		  abort ();
		}

	      input_section = p->u.indirect.section;
	      input_bfd = input_section->owner;

	      sz = (input_section->size < sizeof (ext)
		    ? input_section->size : sizeof (ext));
	      memset (&ext, 0, sizeof (ext));
	      if (! bfd_get_section_contents (input_bfd, input_section,
					      &ext, 0, sz))
		return false;

	      bfd_mips_elf32_swap_reginfo_in (input_bfd, &ext, &sub);

	      reginfo.ri_gprmask |= sub.ri_gprmask;
	      reginfo.ri_cprmask[0] |= sub.ri_cprmask[0];
	      reginfo.ri_cprmask[1] |= sub.ri_cprmask[1];
	      reginfo.ri_cprmask[2] |= sub.ri_cprmask[2];
	      reginfo.ri_cprmask[3] |= sub.ri_cprmask[3];

	      /* ri_gp_value is set by the function
		 `_bfd_mips_elf_section_processing' when the section is
		 finally written out.  */

	      /* Hack: reset the SEC_HAS_CONTENTS flag so that
		 elf_link_input_bfd ignores this section.  */
	      input_section->flags &= ~SEC_HAS_CONTENTS;
	    }

	  /* Size has been set in _bfd_mips_elf_always_size_sections.  */
	  BFD_ASSERT(o->size == sizeof (Elf32_External_RegInfo));

	  /* Skip this section later on (I don't think this currently
	     matters, but someday it might).  */
	  o->map_head.link_order = NULL;

	  reginfo_sec = o;
	}

      if (strcmp (o->name, ".mdebug") == 0)
	{
	  struct extsym_info einfo;
	  bfd_vma last;

	  /* We have found the .mdebug section in the output file.
	     Look through all the link_orders comprising it and merge
	     the information together.  */
	  symhdr->magic = swap->sym_magic;
	  /* FIXME: What should the version stamp be?  */
	  symhdr->vstamp = 0;
	  symhdr->ilineMax = 0;
	  symhdr->cbLine = 0;
	  symhdr->idnMax = 0;
	  symhdr->ipdMax = 0;
	  symhdr->isymMax = 0;
	  symhdr->ioptMax = 0;
	  symhdr->iauxMax = 0;
	  symhdr->issMax = 0;
	  symhdr->issExtMax = 0;
	  symhdr->ifdMax = 0;
	  symhdr->crfd = 0;
	  symhdr->iextMax = 0;

	  /* We accumulate the debugging information itself in the
	     debug_info structure.  */
	  debug.alloc_syments = false;
	  debug.line = NULL;
	  debug.external_dnr = NULL;
	  debug.external_pdr = NULL;
	  debug.external_sym = NULL;
	  debug.external_opt = NULL;
	  debug.external_aux = NULL;
	  debug.ss = NULL;
	  debug.ssext = debug.ssext_end = NULL;
	  debug.external_fdr = NULL;
	  debug.external_rfd = NULL;
	  debug.external_ext = debug.external_ext_end = NULL;

	  mdebug_handle = bfd_ecoff_debug_init (abfd, &debug, swap, info);
	  if (mdebug_handle == NULL)
	    return false;

	  esym.jmptbl = 0;
	  esym.cobol_main = 0;
	  esym.weakext = 0;
	  esym.reserved = 0;
	  esym.ifd = ifdNil;
	  esym.asym.iss = issNil;
	  esym.asym.st = stLocal;
	  esym.asym.reserved = 0;
	  esym.asym.index = indexNil;
	  last = 0;
	  for (i = 0; i < sizeof (secname) / sizeof (secname[0]); i++)
	    {
	      esym.asym.sc = sc[i];
	      s = bfd_get_section_by_name (abfd, secname[i]);
	      if (s != NULL)
		{
		  esym.asym.value = s->vma;
		  last = s->vma + s->size;
		}
	      else
		esym.asym.value = last;
	      if (!bfd_ecoff_debug_one_external (abfd, &debug, swap,
						 secname[i], &esym))
		return false;
	    }

	  for (p = o->map_head.link_order; p != NULL; p = p->next)
	    {
	      asection *input_section;
	      bfd *input_bfd;
	      const struct ecoff_debug_swap *input_swap;
	      struct ecoff_debug_info input_debug;
	      char *eraw_src;
	      char *eraw_end;

	      if (p->type != bfd_indirect_link_order)
		{
		  if (p->type == bfd_data_link_order)
		    continue;
		  abort ();
		}

	      input_section = p->u.indirect.section;
	      input_bfd = input_section->owner;

	      if (!is_mips_elf (input_bfd))
		{
		  /* I don't know what a non MIPS ELF bfd would be
		     doing with a .mdebug section, but I don't really
		     want to deal with it.  */
		  continue;
		}

	      input_swap = (get_elf_backend_data (input_bfd)
			    ->elf_backend_ecoff_debug_swap);

	      BFD_ASSERT (p->size == input_section->size);

	      /* The ECOFF linking code expects that we have already
		 read in the debugging information and set up an
		 ecoff_debug_info structure, so we do that now.  */
	      if (! _bfd_mips_elf_read_ecoff_info (input_bfd, input_section,
						   &input_debug))
		return false;

	      if (! (bfd_ecoff_debug_accumulate
		     (mdebug_handle, abfd, &debug, swap, input_bfd,
		      &input_debug, input_swap, info)))
		{
		  _bfd_ecoff_free_ecoff_debug_info (&input_debug);
		  return false;
		}

	      /* Loop through the external symbols.  For each one with
		 interesting information, try to find the symbol in
		 the linker global hash table and save the information
		 for the output external symbols.  */
	      eraw_src = input_debug.external_ext;
	      eraw_end = (eraw_src
			  + (input_debug.symbolic_header.iextMax
			     * input_swap->external_ext_size));
	      for (;
		   eraw_src < eraw_end;
		   eraw_src += input_swap->external_ext_size)
		{
		  EXTR ext;
		  const char *name;
		  struct mips_elf_link_hash_entry *h;

		  (*input_swap->swap_ext_in) (input_bfd, eraw_src, &ext);
		  if (ext.asym.sc == scNil
		      || ext.asym.sc == scUndefined
		      || ext.asym.sc == scSUndefined)
		    continue;

		  name = input_debug.ssext + ext.asym.iss;
		  h = mips_elf_link_hash_lookup (mips_elf_hash_table (info),
						 name, false, false, true);
		  if (h == NULL || h->esym.ifd != -2)
		    continue;

		  if (ext.ifd != -1)
		    {
		      BFD_ASSERT (ext.ifd
				  < input_debug.symbolic_header.ifdMax);
		      ext.ifd = input_debug.ifdmap[ext.ifd];
		    }

		  h->esym = ext;
		}

	      /* Free up the information we just read.  */
	      _bfd_ecoff_free_ecoff_debug_info (&input_debug);

	      /* Hack: reset the SEC_HAS_CONTENTS flag so that
		 elf_link_input_bfd ignores this section.  */
	      input_section->flags &= ~SEC_HAS_CONTENTS;
	    }

	  if (SGI_COMPAT (abfd) && bfd_link_pic (info))
	    {
	      /* Create .rtproc section.  */
	      rtproc_sec = bfd_get_linker_section (abfd, ".rtproc");
	      if (rtproc_sec == NULL)
		{
		  flagword flags = (SEC_HAS_CONTENTS | SEC_IN_MEMORY
				    | SEC_LINKER_CREATED | SEC_READONLY);

		  rtproc_sec = bfd_make_section_anyway_with_flags (abfd,
								   ".rtproc",
								   flags);
		  if (rtproc_sec == NULL
		      || !bfd_set_section_alignment (rtproc_sec, 4))
		    return false;
		}

	      if (! mips_elf_create_procedure_table (mdebug_handle, abfd,
						     info, rtproc_sec,
						     &debug))
		return false;
	    }

	  /* Build the external symbol information.  */
	  einfo.abfd = abfd;
	  einfo.info = info;
	  einfo.debug = &debug;
	  einfo.swap = swap;
	  einfo.failed = false;
	  mips_elf_link_hash_traverse (mips_elf_hash_table (info),
				       mips_elf_output_extsym, &einfo);
	  if (einfo.failed)
	    return false;

	  /* Set the size of the .mdebug section.  */
	  o->size = bfd_ecoff_debug_size (abfd, &debug, swap);

	  /* Skip this section later on (I don't think this currently
	     matters, but someday it might).  */
	  o->map_head.link_order = NULL;

	  mdebug_sec = o;
	}

      if (startswith (o->name, ".gptab."))
	{
	  const char *subname;
	  unsigned int c;
	  Elf32_gptab *tab;
	  Elf32_External_gptab *ext_tab;
	  unsigned int j;

	  /* The .gptab.sdata and .gptab.sbss sections hold
	     information describing how the small data area would
	     change depending upon the -G switch.  These sections
	     not used in executables files.  */
	  if (! bfd_link_relocatable (info))
	    {
	      for (p = o->map_head.link_order; p != NULL; p = p->next)
		{
		  asection *input_section;

		  if (p->type != bfd_indirect_link_order)
		    {
		      if (p->type == bfd_data_link_order)
			continue;
		      abort ();
		    }

		  input_section = p->u.indirect.section;

		  /* Hack: reset the SEC_HAS_CONTENTS flag so that
		     elf_link_input_bfd ignores this section.  */
		  input_section->flags &= ~SEC_HAS_CONTENTS;
		}

	      /* Skip this section later on (I don't think this
		 currently matters, but someday it might).  */
	      o->map_head.link_order = NULL;

	      /* Really remove the section.  */
	      bfd_section_list_remove (abfd, o);
	      --abfd->section_count;

	      continue;
	    }

	  /* There is one gptab for initialized data, and one for
	     uninitialized data.  */
	  if (strcmp (o->name, ".gptab.sdata") == 0)
	    gptab_data_sec = o;
	  else if (strcmp (o->name, ".gptab.sbss") == 0)
	    gptab_bss_sec = o;
	  else
	    {
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB: illegal section name `%pA'"), abfd, o);
	      bfd_set_error (bfd_error_nonrepresentable_section);
	      return false;
	    }

	  /* The linker script always combines .gptab.data and
	     .gptab.sdata into .gptab.sdata, and likewise for
	     .gptab.bss and .gptab.sbss.  It is possible that there is
	     no .sdata or .sbss section in the output file, in which
	     case we must change the name of the output section.  */
	  subname = o->name + sizeof ".gptab" - 1;
	  if (bfd_get_section_by_name (abfd, subname) == NULL)
	    {
	      if (o == gptab_data_sec)
		o->name = ".gptab.data";
	      else
		o->name = ".gptab.bss";
	      subname = o->name + sizeof ".gptab" - 1;
	      BFD_ASSERT (bfd_get_section_by_name (abfd, subname) != NULL);
	    }

	  /* Set up the first entry.  */
	  c = 1;
	  amt = c * sizeof (Elf32_gptab);
	  tab = bfd_malloc (amt);
	  if (tab == NULL)
	    return false;
	  tab[0].gt_header.gt_current_g_value = elf_gp_size (abfd);
	  tab[0].gt_header.gt_unused = 0;

	  /* Combine the input sections.  */
	  for (p = o->map_head.link_order; p != NULL; p = p->next)
	    {
	      asection *input_section;
	      bfd *input_bfd;
	      bfd_size_type size;
	      unsigned long last;
	      bfd_size_type gpentry;

	      if (p->type != bfd_indirect_link_order)
		{
		  if (p->type == bfd_data_link_order)
		    continue;
		  abort ();
		}

	      input_section = p->u.indirect.section;
	      input_bfd = input_section->owner;

	      /* Combine the gptab entries for this input section one
		 by one.  We know that the input gptab entries are
		 sorted by ascending -G value.  */
	      size = input_section->size;
	      last = 0;
	      for (gpentry = sizeof (Elf32_External_gptab);
		   gpentry < size;
		   gpentry += sizeof (Elf32_External_gptab))
		{
		  Elf32_External_gptab ext_gptab;
		  Elf32_gptab int_gptab;
		  unsigned long val;
		  unsigned long add;
		  bool exact;
		  unsigned int look;

		  if (! (bfd_get_section_contents
			 (input_bfd, input_section, &ext_gptab, gpentry,
			  sizeof (Elf32_External_gptab))))
		    {
		      free (tab);
		      return false;
		    }

		  bfd_mips_elf32_swap_gptab_in (input_bfd, &ext_gptab,
						&int_gptab);
		  val = int_gptab.gt_entry.gt_g_value;
		  add = int_gptab.gt_entry.gt_bytes - last;

		  exact = false;
		  for (look = 1; look < c; look++)
		    {
		      if (tab[look].gt_entry.gt_g_value >= val)
			tab[look].gt_entry.gt_bytes += add;

		      if (tab[look].gt_entry.gt_g_value == val)
			exact = true;
		    }

		  if (! exact)
		    {
		      Elf32_gptab *new_tab;
		      unsigned int max;

		      /* We need a new table entry.  */
		      amt = (bfd_size_type) (c + 1) * sizeof (Elf32_gptab);
		      new_tab = bfd_realloc (tab, amt);
		      if (new_tab == NULL)
			{
			  free (tab);
			  return false;
			}
		      tab = new_tab;
		      tab[c].gt_entry.gt_g_value = val;
		      tab[c].gt_entry.gt_bytes = add;

		      /* Merge in the size for the next smallest -G
			 value, since that will be implied by this new
			 value.  */
		      max = 0;
		      for (look = 1; look < c; look++)
			{
			  if (tab[look].gt_entry.gt_g_value < val
			      && (max == 0
				  || (tab[look].gt_entry.gt_g_value
				      > tab[max].gt_entry.gt_g_value)))
			    max = look;
			}
		      if (max != 0)
			tab[c].gt_entry.gt_bytes +=
			  tab[max].gt_entry.gt_bytes;

		      ++c;
		    }

		  last = int_gptab.gt_entry.gt_bytes;
		}

	      /* Hack: reset the SEC_HAS_CONTENTS flag so that
		 elf_link_input_bfd ignores this section.  */
	      input_section->flags &= ~SEC_HAS_CONTENTS;
	    }

	  /* The table must be sorted by -G value.  */
	  if (c > 2)
	    qsort (tab + 1, c - 1, sizeof (tab[0]), gptab_compare);

	  /* Swap out the table.  */
	  amt = (bfd_size_type) c * sizeof (Elf32_External_gptab);
	  ext_tab = bfd_alloc (abfd, amt);
	  if (ext_tab == NULL)
	    {
	      free (tab);
	      return false;
	    }

	  for (j = 0; j < c; j++)
	    bfd_mips_elf32_swap_gptab_out (abfd, tab + j, ext_tab + j);
	  free (tab);

	  o->size = c * sizeof (Elf32_External_gptab);
	  o->contents = (bfd_byte *) ext_tab;

	  /* Skip this section later on (I don't think this currently
	     matters, but someday it might).  */
	  o->map_head.link_order = NULL;
	}
    }

  /* Invoke the regular ELF backend linker to do all the work.  */
  if (!bfd_elf_final_link (abfd, info))
    return false;

  /* Now write out the computed sections.  */

  if (abiflags_sec != NULL)
    {
      Elf_External_ABIFlags_v0 ext;
      Elf_Internal_ABIFlags_v0 *abiflags;

      abiflags = &mips_elf_tdata (abfd)->abiflags;

      /* Set up the abiflags if no valid input sections were found.  */
      if (!mips_elf_tdata (abfd)->abiflags_valid)
	{
	  infer_mips_abiflags (abfd, abiflags);
	  mips_elf_tdata (abfd)->abiflags_valid = true;
	}
      bfd_mips_elf_swap_abiflags_v0_out (abfd, abiflags, &ext);
      if (! bfd_set_section_contents (abfd, abiflags_sec, &ext, 0, sizeof ext))
	return false;
    }

  if (reginfo_sec != NULL)
    {
      Elf32_External_RegInfo ext;

      bfd_mips_elf32_swap_reginfo_out (abfd, &reginfo, &ext);
      if (! bfd_set_section_contents (abfd, reginfo_sec, &ext, 0, sizeof ext))
	return false;
    }

  if (mdebug_sec != NULL)
    {
      BFD_ASSERT (abfd->output_has_begun);
      if (! bfd_ecoff_write_accumulated_debug (mdebug_handle, abfd, &debug,
					       swap, info,
					       mdebug_sec->filepos))
	return false;

      bfd_ecoff_debug_free (mdebug_handle, abfd, &debug, swap, info);
    }

  if (gptab_data_sec != NULL)
    {
      if (! bfd_set_section_contents (abfd, gptab_data_sec,
				      gptab_data_sec->contents,
				      0, gptab_data_sec->size))
	return false;
    }

  if (gptab_bss_sec != NULL)
    {
      if (! bfd_set_section_contents (abfd, gptab_bss_sec,
				      gptab_bss_sec->contents,
				      0, gptab_bss_sec->size))
	return false;
    }

  if (SGI_COMPAT (abfd))
    {
      rtproc_sec = bfd_get_section_by_name (abfd, ".rtproc");
      if (rtproc_sec != NULL)
	{
	  if (! bfd_set_section_contents (abfd, rtproc_sec,
					  rtproc_sec->contents,
					  0, rtproc_sec->size))
	    return false;
	}
    }

  return true;
}

/* Merge object file header flags from IBFD into OBFD.  Raise an error
   if there are conflicting settings.  */

static bool
mips_elf_merge_obj_e_flags (bfd *ibfd, struct bfd_link_info *info)
{
  bfd *obfd = info->output_bfd;
  struct mips_elf_obj_tdata *out_tdata = mips_elf_tdata (obfd);
  flagword old_flags;
  flagword new_flags;
  bool ok;

  new_flags = elf_elfheader (ibfd)->e_flags;
  elf_elfheader (obfd)->e_flags |= new_flags & EF_MIPS_NOREORDER;
  old_flags = elf_elfheader (obfd)->e_flags;

  /* Check flag compatibility.  */

  new_flags &= ~EF_MIPS_NOREORDER;
  old_flags &= ~EF_MIPS_NOREORDER;

  /* Some IRIX 6 BSD-compatibility objects have this bit set.  It
     doesn't seem to matter.  */
  new_flags &= ~EF_MIPS_XGOT;
  old_flags &= ~EF_MIPS_XGOT;

  /* MIPSpro generates ucode info in n64 objects.  Again, we should
     just be able to ignore this.  */
  new_flags &= ~EF_MIPS_UCODE;
  old_flags &= ~EF_MIPS_UCODE;

  /* DSOs should only be linked with CPIC code.  */
  if ((ibfd->flags & DYNAMIC) != 0)
    new_flags |= EF_MIPS_PIC | EF_MIPS_CPIC;

  if (new_flags == old_flags)
    return true;

  ok = true;

  if (((new_flags & (EF_MIPS_PIC | EF_MIPS_CPIC)) != 0)
      != ((old_flags & (EF_MIPS_PIC | EF_MIPS_CPIC)) != 0))
    {
      _bfd_error_handler
	(_("%pB: warning: linking abicalls files with non-abicalls files"),
	 ibfd);
      ok = true;
    }

  if (new_flags & (EF_MIPS_PIC | EF_MIPS_CPIC))
    elf_elfheader (obfd)->e_flags |= EF_MIPS_CPIC;
  if (! (new_flags & EF_MIPS_PIC))
    elf_elfheader (obfd)->e_flags &= ~EF_MIPS_PIC;

  new_flags &= ~ (EF_MIPS_PIC | EF_MIPS_CPIC);
  old_flags &= ~ (EF_MIPS_PIC | EF_MIPS_CPIC);

  /* Compare the ISAs.  */
  if (mips_32bit_flags_p (old_flags) != mips_32bit_flags_p (new_flags))
    {
      _bfd_error_handler
	(_("%pB: linking 32-bit code with 64-bit code"),
	 ibfd);
      ok = false;
    }
  else if (!mips_mach_extends_p (bfd_get_mach (ibfd), bfd_get_mach (obfd)))
    {
      /* OBFD's ISA isn't the same as, or an extension of, IBFD's.  */
      if (mips_mach_extends_p (bfd_get_mach (obfd), bfd_get_mach (ibfd)))
	{
	  /* Copy the architecture info from IBFD to OBFD.  Also copy
	     the 32-bit flag (if set) so that we continue to recognise
	     OBFD as a 32-bit binary.  */
	  bfd_set_arch_info (obfd, bfd_get_arch_info (ibfd));
	  elf_elfheader (obfd)->e_flags &= ~(EF_MIPS_ARCH | EF_MIPS_MACH);
	  elf_elfheader (obfd)->e_flags
	    |= new_flags & (EF_MIPS_ARCH | EF_MIPS_MACH | EF_MIPS_32BITMODE);

	  /* Update the ABI flags isa_level, isa_rev, isa_ext fields.  */
	  update_mips_abiflags_isa (obfd, &out_tdata->abiflags);

	  /* Copy across the ABI flags if OBFD doesn't use them
	     and if that was what caused us to treat IBFD as 32-bit.  */
	  if ((old_flags & EF_MIPS_ABI) == 0
	      && mips_32bit_flags_p (new_flags)
	      && !mips_32bit_flags_p (new_flags & ~EF_MIPS_ABI))
	    elf_elfheader (obfd)->e_flags |= new_flags & EF_MIPS_ABI;
	}
      else
	{
	  /* The ISAs aren't compatible.  */
	  _bfd_error_handler
	    /* xgettext:c-format */
	    (_("%pB: linking %s module with previous %s modules"),
	     ibfd,
	     bfd_printable_name (ibfd),
	     bfd_printable_name (obfd));
	  ok = false;
	}
    }

  new_flags &= ~(EF_MIPS_ARCH | EF_MIPS_MACH | EF_MIPS_32BITMODE);
  old_flags &= ~(EF_MIPS_ARCH | EF_MIPS_MACH | EF_MIPS_32BITMODE);

  /* Compare ABIs.  The 64-bit ABI does not use EF_MIPS_ABI.  But, it
     does set EI_CLASS differently from any 32-bit ABI.  */
  if ((new_flags & EF_MIPS_ABI) != (old_flags & EF_MIPS_ABI)
      || (elf_elfheader (ibfd)->e_ident[EI_CLASS]
	  != elf_elfheader (obfd)->e_ident[EI_CLASS]))
    {
      /* Only error if both are set (to different values).  */
      if (((new_flags & EF_MIPS_ABI) && (old_flags & EF_MIPS_ABI))
	  || (elf_elfheader (ibfd)->e_ident[EI_CLASS]
	      != elf_elfheader (obfd)->e_ident[EI_CLASS]))
	{
	  _bfd_error_handler
	    /* xgettext:c-format */
	    (_("%pB: ABI mismatch: linking %s module with previous %s modules"),
	     ibfd,
	     elf_mips_abi_name (ibfd),
	     elf_mips_abi_name (obfd));
	  ok = false;
	}
      new_flags &= ~EF_MIPS_ABI;
      old_flags &= ~EF_MIPS_ABI;
    }

  /* Compare ASEs.  Forbid linking MIPS16 and microMIPS ASE modules together
     and allow arbitrary mixing of the remaining ASEs (retain the union).  */
  if ((new_flags & EF_MIPS_ARCH_ASE) != (old_flags & EF_MIPS_ARCH_ASE))
    {
      int old_micro = old_flags & EF_MIPS_ARCH_ASE_MICROMIPS;
      int new_micro = new_flags & EF_MIPS_ARCH_ASE_MICROMIPS;
      int old_m16 = old_flags & EF_MIPS_ARCH_ASE_M16;
      int new_m16 = new_flags & EF_MIPS_ARCH_ASE_M16;
      int micro_mis = old_m16 && new_micro;
      int m16_mis = old_micro && new_m16;

      if (m16_mis || micro_mis)
	{
	  _bfd_error_handler
	    /* xgettext:c-format */
	    (_("%pB: ASE mismatch: linking %s module with previous %s modules"),
	     ibfd,
	     m16_mis ? "MIPS16" : "microMIPS",
	     m16_mis ? "microMIPS" : "MIPS16");
	  ok = false;
	}

      elf_elfheader (obfd)->e_flags |= new_flags & EF_MIPS_ARCH_ASE;

      new_flags &= ~ EF_MIPS_ARCH_ASE;
      old_flags &= ~ EF_MIPS_ARCH_ASE;
    }

  /* Compare NaN encodings.  */
  if ((new_flags & EF_MIPS_NAN2008) != (old_flags & EF_MIPS_NAN2008))
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: linking %s module with previous %s modules"),
			  ibfd,
			  (new_flags & EF_MIPS_NAN2008
			   ? "-mnan=2008" : "-mnan=legacy"),
			  (old_flags & EF_MIPS_NAN2008
			   ? "-mnan=2008" : "-mnan=legacy"));
      ok = false;
      new_flags &= ~EF_MIPS_NAN2008;
      old_flags &= ~EF_MIPS_NAN2008;
    }

  /* Compare FP64 state.  */
  if ((new_flags & EF_MIPS_FP64) != (old_flags & EF_MIPS_FP64))
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: linking %s module with previous %s modules"),
			  ibfd,
			  (new_flags & EF_MIPS_FP64
			   ? "-mfp64" : "-mfp32"),
			  (old_flags & EF_MIPS_FP64
			   ? "-mfp64" : "-mfp32"));
      ok = false;
      new_flags &= ~EF_MIPS_FP64;
      old_flags &= ~EF_MIPS_FP64;
    }

  /* Warn about any other mismatches */
  if (new_flags != old_flags)
    {
      /* xgettext:c-format */
      _bfd_error_handler
	(_("%pB: uses different e_flags (%#x) fields than previous modules "
	   "(%#x)"),
	 ibfd, new_flags, old_flags);
      ok = false;
    }

  return ok;
}

/* Merge object attributes from IBFD into OBFD.  Raise an error if
   there are conflicting attributes.  */
static bool
mips_elf_merge_obj_attributes (bfd *ibfd, struct bfd_link_info *info)
{
  bfd *obfd = info->output_bfd;
  obj_attribute *in_attr;
  obj_attribute *out_attr;
  bfd *abi_fp_bfd;
  bfd *abi_msa_bfd;

  abi_fp_bfd = mips_elf_tdata (obfd)->abi_fp_bfd;
  in_attr = elf_known_obj_attributes (ibfd)[OBJ_ATTR_GNU];
  if (!abi_fp_bfd && in_attr[Tag_GNU_MIPS_ABI_FP].i != Val_GNU_MIPS_ABI_FP_ANY)
    mips_elf_tdata (obfd)->abi_fp_bfd = ibfd;

  abi_msa_bfd = mips_elf_tdata (obfd)->abi_msa_bfd;
  if (!abi_msa_bfd
      && in_attr[Tag_GNU_MIPS_ABI_MSA].i != Val_GNU_MIPS_ABI_MSA_ANY)
    mips_elf_tdata (obfd)->abi_msa_bfd = ibfd;

  if (!elf_known_obj_attributes_proc (obfd)[0].i)
    {
      /* This is the first object.  Copy the attributes.  */
      _bfd_elf_copy_obj_attributes (ibfd, obfd);

      /* Use the Tag_null value to indicate the attributes have been
	 initialized.  */
      elf_known_obj_attributes_proc (obfd)[0].i = 1;

      return true;
    }

  /* Check for conflicting Tag_GNU_MIPS_ABI_FP attributes and merge
     non-conflicting ones.  */
  out_attr = elf_known_obj_attributes (obfd)[OBJ_ATTR_GNU];
  if (in_attr[Tag_GNU_MIPS_ABI_FP].i != out_attr[Tag_GNU_MIPS_ABI_FP].i)
    {
      int out_fp, in_fp;

      out_fp = out_attr[Tag_GNU_MIPS_ABI_FP].i;
      in_fp = in_attr[Tag_GNU_MIPS_ABI_FP].i;
      out_attr[Tag_GNU_MIPS_ABI_FP].type = 1;
      if (out_fp == Val_GNU_MIPS_ABI_FP_ANY)
	out_attr[Tag_GNU_MIPS_ABI_FP].i = in_fp;
      else if (out_fp == Val_GNU_MIPS_ABI_FP_XX
	       && (in_fp == Val_GNU_MIPS_ABI_FP_DOUBLE
		   || in_fp == Val_GNU_MIPS_ABI_FP_64
		   || in_fp == Val_GNU_MIPS_ABI_FP_64A))
	{
	  mips_elf_tdata (obfd)->abi_fp_bfd = ibfd;
	  out_attr[Tag_GNU_MIPS_ABI_FP].i = in_attr[Tag_GNU_MIPS_ABI_FP].i;
	}
      else if (in_fp == Val_GNU_MIPS_ABI_FP_XX
	       && (out_fp == Val_GNU_MIPS_ABI_FP_DOUBLE
		   || out_fp == Val_GNU_MIPS_ABI_FP_64
		   || out_fp == Val_GNU_MIPS_ABI_FP_64A))
	/* Keep the current setting.  */;
      else if (out_fp == Val_GNU_MIPS_ABI_FP_64A
	       && in_fp == Val_GNU_MIPS_ABI_FP_64)
	{
	  mips_elf_tdata (obfd)->abi_fp_bfd = ibfd;
	  out_attr[Tag_GNU_MIPS_ABI_FP].i = in_attr[Tag_GNU_MIPS_ABI_FP].i;
	}
      else if (in_fp == Val_GNU_MIPS_ABI_FP_64A
	       && out_fp == Val_GNU_MIPS_ABI_FP_64)
	/* Keep the current setting.  */;
      else if (in_fp != Val_GNU_MIPS_ABI_FP_ANY)
	{
	  const char *out_string, *in_string;

	  out_string = _bfd_mips_fp_abi_string (out_fp);
	  in_string = _bfd_mips_fp_abi_string (in_fp);
	  /* First warn about cases involving unrecognised ABIs.  */
	  if (!out_string && !in_string)
	    /* xgettext:c-format */
	    _bfd_error_handler
	      (_("warning: %pB uses unknown floating point ABI %d "
		 "(set by %pB), %pB uses unknown floating point ABI %d"),
	       obfd, out_fp, abi_fp_bfd, ibfd, in_fp);
	  else if (!out_string)
	    _bfd_error_handler
	      /* xgettext:c-format */
	      (_("warning: %pB uses unknown floating point ABI %d "
		 "(set by %pB), %pB uses %s"),
	       obfd, out_fp, abi_fp_bfd, ibfd, in_string);
	  else if (!in_string)
	    _bfd_error_handler
	      /* xgettext:c-format */
	      (_("warning: %pB uses %s (set by %pB), "
		 "%pB uses unknown floating point ABI %d"),
	       obfd, out_string, abi_fp_bfd, ibfd, in_fp);
	  else
	    {
	      /* If one of the bfds is soft-float, the other must be
		 hard-float.  The exact choice of hard-float ABI isn't
		 really relevant to the error message.  */
	      if (in_fp == Val_GNU_MIPS_ABI_FP_SOFT)
		out_string = "-mhard-float";
	      else if (out_fp == Val_GNU_MIPS_ABI_FP_SOFT)
		in_string = "-mhard-float";
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("warning: %pB uses %s (set by %pB), %pB uses %s"),
		 obfd, out_string, abi_fp_bfd, ibfd, in_string);
	    }
	}
    }

  /* Check for conflicting Tag_GNU_MIPS_ABI_MSA attributes and merge
     non-conflicting ones.  */
  if (in_attr[Tag_GNU_MIPS_ABI_MSA].i != out_attr[Tag_GNU_MIPS_ABI_MSA].i)
    {
      out_attr[Tag_GNU_MIPS_ABI_MSA].type = 1;
      if (out_attr[Tag_GNU_MIPS_ABI_MSA].i == Val_GNU_MIPS_ABI_MSA_ANY)
	out_attr[Tag_GNU_MIPS_ABI_MSA].i = in_attr[Tag_GNU_MIPS_ABI_MSA].i;
      else if (in_attr[Tag_GNU_MIPS_ABI_MSA].i != Val_GNU_MIPS_ABI_MSA_ANY)
	switch (out_attr[Tag_GNU_MIPS_ABI_MSA].i)
	  {
	  case Val_GNU_MIPS_ABI_MSA_128:
	    _bfd_error_handler
	      /* xgettext:c-format */
	      (_("warning: %pB uses %s (set by %pB), "
		 "%pB uses unknown MSA ABI %d"),
	       obfd, "-mmsa", abi_msa_bfd,
	       ibfd, in_attr[Tag_GNU_MIPS_ABI_MSA].i);
	    break;

	  default:
	    switch (in_attr[Tag_GNU_MIPS_ABI_MSA].i)
	      {
	      case Val_GNU_MIPS_ABI_MSA_128:
		_bfd_error_handler
		  /* xgettext:c-format */
		  (_("warning: %pB uses unknown MSA ABI %d "
		     "(set by %pB), %pB uses %s"),
		     obfd, out_attr[Tag_GNU_MIPS_ABI_MSA].i,
		   abi_msa_bfd, ibfd, "-mmsa");
		  break;

	      default:
		_bfd_error_handler
		  /* xgettext:c-format */
		  (_("warning: %pB uses unknown MSA ABI %d "
		     "(set by %pB), %pB uses unknown MSA ABI %d"),
		   obfd, out_attr[Tag_GNU_MIPS_ABI_MSA].i,
		   abi_msa_bfd, ibfd, in_attr[Tag_GNU_MIPS_ABI_MSA].i);
		break;
	      }
	  }
    }

  /* Merge Tag_compatibility attributes and any common GNU ones.  */
  return _bfd_elf_merge_object_attributes (ibfd, info);
}

/* Merge object ABI flags from IBFD into OBFD.  Raise an error if
   there are conflicting settings.  */

static bool
mips_elf_merge_obj_abiflags (bfd *ibfd, bfd *obfd)
{
  obj_attribute *out_attr = elf_known_obj_attributes (obfd)[OBJ_ATTR_GNU];
  struct mips_elf_obj_tdata *out_tdata = mips_elf_tdata (obfd);
  struct mips_elf_obj_tdata *in_tdata = mips_elf_tdata (ibfd);

  /* Update the output abiflags fp_abi using the computed fp_abi.  */
  out_tdata->abiflags.fp_abi = out_attr[Tag_GNU_MIPS_ABI_FP].i;

#define max(a, b) ((a) > (b) ? (a) : (b))
  /* Merge abiflags.  */
  out_tdata->abiflags.isa_level = max (out_tdata->abiflags.isa_level,
				       in_tdata->abiflags.isa_level);
  out_tdata->abiflags.isa_rev = max (out_tdata->abiflags.isa_rev,
				     in_tdata->abiflags.isa_rev);
  out_tdata->abiflags.gpr_size = max (out_tdata->abiflags.gpr_size,
				      in_tdata->abiflags.gpr_size);
  out_tdata->abiflags.cpr1_size = max (out_tdata->abiflags.cpr1_size,
				       in_tdata->abiflags.cpr1_size);
  out_tdata->abiflags.cpr2_size = max (out_tdata->abiflags.cpr2_size,
				       in_tdata->abiflags.cpr2_size);
#undef max
  out_tdata->abiflags.ases |= in_tdata->abiflags.ases;
  out_tdata->abiflags.flags1 |= in_tdata->abiflags.flags1;

  return true;
}

/* Merge backend specific data from an object file to the output
   object file when linking.  */

bool
_bfd_mips_elf_merge_private_bfd_data (bfd *ibfd, struct bfd_link_info *info)
{
  bfd *obfd = info->output_bfd;
  struct mips_elf_obj_tdata *out_tdata;
  struct mips_elf_obj_tdata *in_tdata;
  bool null_input_bfd = true;
  asection *sec;
  bool ok;

  /* Check if we have the same endianness.  */
  if (! _bfd_generic_verify_endian_match (ibfd, info))
    {
      _bfd_error_handler
	(_("%pB: endianness incompatible with that of the selected emulation"),
	 ibfd);
      return false;
    }

  if (!is_mips_elf (ibfd) || !is_mips_elf (obfd))
    return true;

  in_tdata = mips_elf_tdata (ibfd);
  out_tdata = mips_elf_tdata (obfd);

  if (strcmp (bfd_get_target (ibfd), bfd_get_target (obfd)) != 0)
    {
      _bfd_error_handler
	(_("%pB: ABI is incompatible with that of the selected emulation"),
	 ibfd);
      return false;
    }

  /* Check to see if the input BFD actually contains any sections.  If not,
     then it has no attributes, and its flags may not have been initialized
     either, but it cannot actually cause any incompatibility.  */
  /* FIXME: This excludes any input shared library from consideration.  */
  for (sec = ibfd->sections; sec != NULL; sec = sec->next)
    {
      /* Ignore synthetic sections and empty .text, .data and .bss sections
	 which are automatically generated by gas.  Also ignore fake
	 (s)common sections, since merely defining a common symbol does
	 not affect compatibility.  */
      if ((sec->flags & SEC_IS_COMMON) == 0
	  && strcmp (sec->name, ".reginfo")
	  && strcmp (sec->name, ".mdebug")
	  && (sec->size != 0
	      || (strcmp (sec->name, ".text")
		  && strcmp (sec->name, ".data")
		  && strcmp (sec->name, ".bss"))))
	{
	  null_input_bfd = false;
	  break;
	}
    }
  if (null_input_bfd)
    return true;

  /* Populate abiflags using existing information.  */
  if (in_tdata->abiflags_valid)
    {
      obj_attribute *in_attr = elf_known_obj_attributes (ibfd)[OBJ_ATTR_GNU];
      Elf_Internal_ABIFlags_v0 in_abiflags;
      Elf_Internal_ABIFlags_v0 abiflags;

      /* Set up the FP ABI attribute from the abiflags if it is not already
	 set.  */
      if (in_attr[Tag_GNU_MIPS_ABI_FP].i == Val_GNU_MIPS_ABI_FP_ANY)
	in_attr[Tag_GNU_MIPS_ABI_FP].i = in_tdata->abiflags.fp_abi;

      infer_mips_abiflags (ibfd, &abiflags);
      in_abiflags = in_tdata->abiflags;

      /* It is not possible to infer the correct ISA revision
	 for R3 or R5 so drop down to R2 for the checks.  */
      if (in_abiflags.isa_rev == 3 || in_abiflags.isa_rev == 5)
	in_abiflags.isa_rev = 2;

      if (LEVEL_REV (in_abiflags.isa_level, in_abiflags.isa_rev)
	  < LEVEL_REV (abiflags.isa_level, abiflags.isa_rev))
	_bfd_error_handler
	  (_("%pB: warning: inconsistent ISA between e_flags and "
	     ".MIPS.abiflags"), ibfd);
      if (abiflags.fp_abi != Val_GNU_MIPS_ABI_FP_ANY
	  && in_abiflags.fp_abi != abiflags.fp_abi)
	_bfd_error_handler
	  (_("%pB: warning: inconsistent FP ABI between .gnu.attributes and "
	     ".MIPS.abiflags"), ibfd);
      if ((in_abiflags.ases & abiflags.ases) != abiflags.ases)
	_bfd_error_handler
	  (_("%pB: warning: inconsistent ASEs between e_flags and "
	     ".MIPS.abiflags"), ibfd);
      /* The isa_ext is allowed to be an extension of what can be inferred
	 from e_flags.  */
      if (!mips_mach_extends_p (bfd_mips_isa_ext_mach (abiflags.isa_ext),
				bfd_mips_isa_ext_mach (in_abiflags.isa_ext)))
	_bfd_error_handler
	  (_("%pB: warning: inconsistent ISA extensions between e_flags and "
	     ".MIPS.abiflags"), ibfd);
      if (in_abiflags.flags2 != 0)
	_bfd_error_handler
	  (_("%pB: warning: unexpected flag in the flags2 field of "
	     ".MIPS.abiflags (0x%lx)"), ibfd,
	   in_abiflags.flags2);
    }
  else
    {
      infer_mips_abiflags (ibfd, &in_tdata->abiflags);
      in_tdata->abiflags_valid = true;
    }

  if (!out_tdata->abiflags_valid)
    {
      /* Copy input abiflags if output abiflags are not already valid.  */
      out_tdata->abiflags = in_tdata->abiflags;
      out_tdata->abiflags_valid = true;
    }

  if (! elf_flags_init (obfd))
    {
      elf_flags_init (obfd) = true;
      elf_elfheader (obfd)->e_flags = elf_elfheader (ibfd)->e_flags;
      elf_elfheader (obfd)->e_ident[EI_CLASS]
	= elf_elfheader (ibfd)->e_ident[EI_CLASS];

      if (bfd_get_arch (obfd) == bfd_get_arch (ibfd)
	  && (bfd_get_arch_info (obfd)->the_default
	      || mips_mach_extends_p (bfd_get_mach (obfd),
				      bfd_get_mach (ibfd))))
	{
	  if (! bfd_set_arch_mach (obfd, bfd_get_arch (ibfd),
				   bfd_get_mach (ibfd)))
	    return false;

	  /* Update the ABI flags isa_level, isa_rev and isa_ext fields.  */
	  update_mips_abiflags_isa (obfd, &out_tdata->abiflags);
	}

      ok = true;
    }
  else
    ok = mips_elf_merge_obj_e_flags (ibfd, info);

  ok = mips_elf_merge_obj_attributes (ibfd, info) && ok;

  ok = mips_elf_merge_obj_abiflags (ibfd, obfd) && ok;

  if (!ok)
    {
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  return true;
}

/* Function to keep MIPS specific file flags like as EF_MIPS_PIC.  */

bool
_bfd_mips_elf_set_private_flags (bfd *abfd, flagword flags)
{
  BFD_ASSERT (!elf_flags_init (abfd)
	      || elf_elfheader (abfd)->e_flags == flags);

  elf_elfheader (abfd)->e_flags = flags;
  elf_flags_init (abfd) = true;
  return true;
}

char *
_bfd_mips_elf_get_target_dtag (bfd_vma dtag)
{
  switch (dtag)
    {
    default: return "";
    case DT_MIPS_RLD_VERSION:
      return "MIPS_RLD_VERSION";
    case DT_MIPS_TIME_STAMP:
      return "MIPS_TIME_STAMP";
    case DT_MIPS_ICHECKSUM:
      return "MIPS_ICHECKSUM";
    case DT_MIPS_IVERSION:
      return "MIPS_IVERSION";
    case DT_MIPS_FLAGS:
      return "MIPS_FLAGS";
    case DT_MIPS_BASE_ADDRESS:
      return "MIPS_BASE_ADDRESS";
    case DT_MIPS_MSYM:
      return "MIPS_MSYM";
    case DT_MIPS_CONFLICT:
      return "MIPS_CONFLICT";
    case DT_MIPS_LIBLIST:
      return "MIPS_LIBLIST";
    case DT_MIPS_LOCAL_GOTNO:
      return "MIPS_LOCAL_GOTNO";
    case DT_MIPS_CONFLICTNO:
      return "MIPS_CONFLICTNO";
    case DT_MIPS_LIBLISTNO:
      return "MIPS_LIBLISTNO";
    case DT_MIPS_SYMTABNO:
      return "MIPS_SYMTABNO";
    case DT_MIPS_UNREFEXTNO:
      return "MIPS_UNREFEXTNO";
    case DT_MIPS_GOTSYM:
      return "MIPS_GOTSYM";
    case DT_MIPS_HIPAGENO:
      return "MIPS_HIPAGENO";
    case DT_MIPS_RLD_MAP:
      return "MIPS_RLD_MAP";
    case DT_MIPS_RLD_MAP_REL:
      return "MIPS_RLD_MAP_REL";
    case DT_MIPS_DELTA_CLASS:
      return "MIPS_DELTA_CLASS";
    case DT_MIPS_DELTA_CLASS_NO:
      return "MIPS_DELTA_CLASS_NO";
    case DT_MIPS_DELTA_INSTANCE:
      return "MIPS_DELTA_INSTANCE";
    case DT_MIPS_DELTA_INSTANCE_NO:
      return "MIPS_DELTA_INSTANCE_NO";
    case DT_MIPS_DELTA_RELOC:
      return "MIPS_DELTA_RELOC";
    case DT_MIPS_DELTA_RELOC_NO:
      return "MIPS_DELTA_RELOC_NO";
    case DT_MIPS_DELTA_SYM:
      return "MIPS_DELTA_SYM";
    case DT_MIPS_DELTA_SYM_NO:
      return "MIPS_DELTA_SYM_NO";
    case DT_MIPS_DELTA_CLASSSYM:
      return "MIPS_DELTA_CLASSSYM";
    case DT_MIPS_DELTA_CLASSSYM_NO:
      return "MIPS_DELTA_CLASSSYM_NO";
    case DT_MIPS_CXX_FLAGS:
      return "MIPS_CXX_FLAGS";
    case DT_MIPS_PIXIE_INIT:
      return "MIPS_PIXIE_INIT";
    case DT_MIPS_SYMBOL_LIB:
      return "MIPS_SYMBOL_LIB";
    case DT_MIPS_LOCALPAGE_GOTIDX:
      return "MIPS_LOCALPAGE_GOTIDX";
    case DT_MIPS_LOCAL_GOTIDX:
      return "MIPS_LOCAL_GOTIDX";
    case DT_MIPS_HIDDEN_GOTIDX:
      return "MIPS_HIDDEN_GOTIDX";
    case DT_MIPS_PROTECTED_GOTIDX:
      return "MIPS_PROTECTED_GOT_IDX";
    case DT_MIPS_OPTIONS:
      return "MIPS_OPTIONS";
    case DT_MIPS_INTERFACE:
      return "MIPS_INTERFACE";
    case DT_MIPS_DYNSTR_ALIGN:
      return "DT_MIPS_DYNSTR_ALIGN";
    case DT_MIPS_INTERFACE_SIZE:
      return "DT_MIPS_INTERFACE_SIZE";
    case DT_MIPS_RLD_TEXT_RESOLVE_ADDR:
      return "DT_MIPS_RLD_TEXT_RESOLVE_ADDR";
    case DT_MIPS_PERF_SUFFIX:
      return "DT_MIPS_PERF_SUFFIX";
    case DT_MIPS_COMPACT_SIZE:
      return "DT_MIPS_COMPACT_SIZE";
    case DT_MIPS_GP_VALUE:
      return "DT_MIPS_GP_VALUE";
    case DT_MIPS_AUX_DYNAMIC:
      return "DT_MIPS_AUX_DYNAMIC";
    case DT_MIPS_PLTGOT:
      return "DT_MIPS_PLTGOT";
    case DT_MIPS_RWPLT:
      return "DT_MIPS_RWPLT";
    case DT_MIPS_XHASH:
      return "DT_MIPS_XHASH";
    }
}

/* Return the meaning of Tag_GNU_MIPS_ABI_FP value FP, or null if
   not known.  */

const char *
_bfd_mips_fp_abi_string (int fp)
{
  switch (fp)
    {
      /* These strings aren't translated because they're simply
	 option lists.  */
    case Val_GNU_MIPS_ABI_FP_DOUBLE:
      return "-mdouble-float";

    case Val_GNU_MIPS_ABI_FP_SINGLE:
      return "-msingle-float";

    case Val_GNU_MIPS_ABI_FP_SOFT:
      return "-msoft-float";

    case Val_GNU_MIPS_ABI_FP_OLD_64:
      return _("-mips32r2 -mfp64 (12 callee-saved)");

    case Val_GNU_MIPS_ABI_FP_XX:
      return "-mfpxx";

    case Val_GNU_MIPS_ABI_FP_64:
      return "-mgp32 -mfp64";

    case Val_GNU_MIPS_ABI_FP_64A:
      return "-mgp32 -mfp64 -mno-odd-spreg";

    default:
      return 0;
    }
}

static void
print_mips_ases (FILE *file, unsigned int mask)
{
  if (mask & AFL_ASE_DSP)
    fputs ("\n\tDSP ASE", file);
  if (mask & AFL_ASE_DSPR2)
    fputs ("\n\tDSP R2 ASE", file);
  if (mask & AFL_ASE_DSPR3)
    fputs ("\n\tDSP R3 ASE", file);
  if (mask & AFL_ASE_EVA)
    fputs ("\n\tEnhanced VA Scheme", file);
  if (mask & AFL_ASE_MCU)
    fputs ("\n\tMCU (MicroController) ASE", file);
  if (mask & AFL_ASE_MDMX)
    fputs ("\n\tMDMX ASE", file);
  if (mask & AFL_ASE_MIPS3D)
    fputs ("\n\tMIPS-3D ASE", file);
  if (mask & AFL_ASE_MT)
    fputs ("\n\tMT ASE", file);
  if (mask & AFL_ASE_SMARTMIPS)
    fputs ("\n\tSmartMIPS ASE", file);
  if (mask & AFL_ASE_VIRT)
    fputs ("\n\tVZ ASE", file);
  if (mask & AFL_ASE_MSA)
    fputs ("\n\tMSA ASE", file);
  if (mask & AFL_ASE_MIPS16)
    fputs ("\n\tMIPS16 ASE", file);
  if (mask & AFL_ASE_MICROMIPS)
    fputs ("\n\tMICROMIPS ASE", file);
  if (mask & AFL_ASE_XPA)
    fputs ("\n\tXPA ASE", file);
  if (mask & AFL_ASE_MIPS16E2)
    fputs ("\n\tMIPS16e2 ASE", file);
  if (mask & AFL_ASE_CRC)
    fputs ("\n\tCRC ASE", file);
  if (mask & AFL_ASE_GINV)
    fputs ("\n\tGINV ASE", file);
  if (mask & AFL_ASE_LOONGSON_MMI)
    fputs ("\n\tLoongson MMI ASE", file);
  if (mask & AFL_ASE_LOONGSON_CAM)
    fputs ("\n\tLoongson CAM ASE", file);
  if (mask & AFL_ASE_LOONGSON_EXT)
    fputs ("\n\tLoongson EXT ASE", file);
  if (mask & AFL_ASE_LOONGSON_EXT2)
    fputs ("\n\tLoongson EXT2 ASE", file);
  if (mask == 0)
    fprintf (file, "\n\t%s", _("None"));
  else if ((mask & ~AFL_ASE_MASK) != 0)
    fprintf (stdout, "\n\t%s (%x)", _("Unknown"), mask & ~AFL_ASE_MASK);
}

static void
print_mips_isa_ext (FILE *file, unsigned int isa_ext)
{
  switch (isa_ext)
    {
    case 0:
      fputs (_("None"), file);
      break;
    case AFL_EXT_XLR:
      fputs ("RMI XLR", file);
      break;
    case AFL_EXT_OCTEON3:
      fputs ("Cavium Networks Octeon3", file);
      break;
    case AFL_EXT_OCTEON2:
      fputs ("Cavium Networks Octeon2", file);
      break;
    case AFL_EXT_OCTEONP:
      fputs ("Cavium Networks OcteonP", file);
      break;
    case AFL_EXT_OCTEON:
      fputs ("Cavium Networks Octeon", file);
      break;
    case AFL_EXT_5900:
      fputs ("Toshiba R5900", file);
      break;
    case AFL_EXT_4650:
      fputs ("MIPS R4650", file);
      break;
    case AFL_EXT_4010:
      fputs ("LSI R4010", file);
      break;
    case AFL_EXT_4100:
      fputs ("NEC VR4100", file);
      break;
    case AFL_EXT_3900:
      fputs ("Toshiba R3900", file);
      break;
    case AFL_EXT_10000:
      fputs ("MIPS R10000", file);
      break;
    case AFL_EXT_SB1:
      fputs ("Broadcom SB-1", file);
      break;
    case AFL_EXT_4111:
      fputs ("NEC VR4111/VR4181", file);
      break;
    case AFL_EXT_4120:
      fputs ("NEC VR4120", file);
      break;
    case AFL_EXT_5400:
      fputs ("NEC VR5400", file);
      break;
    case AFL_EXT_5500:
      fputs ("NEC VR5500", file);
      break;
    case AFL_EXT_LOONGSON_2E:
      fputs ("ST Microelectronics Loongson 2E", file);
      break;
    case AFL_EXT_LOONGSON_2F:
      fputs ("ST Microelectronics Loongson 2F", file);
      break;
    case AFL_EXT_INTERAPTIV_MR2:
      fputs ("Imagination interAptiv MR2", file);
      break;
    default:
      fprintf (file, "%s (%d)", _("Unknown"), isa_ext);
      break;
    }
}

static void
print_mips_fp_abi_value (FILE *file, int val)
{
  switch (val)
    {
    case Val_GNU_MIPS_ABI_FP_ANY:
      fprintf (file, _("Hard or soft float\n"));
      break;
    case Val_GNU_MIPS_ABI_FP_DOUBLE:
      fprintf (file, _("Hard float (double precision)\n"));
      break;
    case Val_GNU_MIPS_ABI_FP_SINGLE:
      fprintf (file, _("Hard float (single precision)\n"));
      break;
    case Val_GNU_MIPS_ABI_FP_SOFT:
      fprintf (file, _("Soft float\n"));
      break;
    case Val_GNU_MIPS_ABI_FP_OLD_64:
      fprintf (file, _("Hard float (MIPS32r2 64-bit FPU 12 callee-saved)\n"));
      break;
    case Val_GNU_MIPS_ABI_FP_XX:
      fprintf (file, _("Hard float (32-bit CPU, Any FPU)\n"));
      break;
    case Val_GNU_MIPS_ABI_FP_64:
      fprintf (file, _("Hard float (32-bit CPU, 64-bit FPU)\n"));
      break;
    case Val_GNU_MIPS_ABI_FP_64A:
      fprintf (file, _("Hard float compat (32-bit CPU, 64-bit FPU)\n"));
      break;
    default:
      fprintf (file, "??? (%d)\n", val);
      break;
    }
}

static int
get_mips_reg_size (int reg_size)
{
  return (reg_size == AFL_REG_NONE) ? 0
	 : (reg_size == AFL_REG_32) ? 32
	 : (reg_size == AFL_REG_64) ? 64
	 : (reg_size == AFL_REG_128) ? 128
	 : -1;
}

bool
_bfd_mips_elf_print_private_bfd_data (bfd *abfd, void *ptr)
{
  FILE *file = ptr;

  BFD_ASSERT (abfd != NULL && ptr != NULL);

  /* Print normal ELF private data.  */
  _bfd_elf_print_private_bfd_data (abfd, ptr);

  /* xgettext:c-format */
  fprintf (file, _("private flags = %lx:"), elf_elfheader (abfd)->e_flags);

  if ((elf_elfheader (abfd)->e_flags & EF_MIPS_ABI) == E_MIPS_ABI_O32)
    fprintf (file, _(" [abi=O32]"));
  else if ((elf_elfheader (abfd)->e_flags & EF_MIPS_ABI) == E_MIPS_ABI_O64)
    fprintf (file, _(" [abi=O64]"));
  else if ((elf_elfheader (abfd)->e_flags & EF_MIPS_ABI) == E_MIPS_ABI_EABI32)
    fprintf (file, _(" [abi=EABI32]"));
  else if ((elf_elfheader (abfd)->e_flags & EF_MIPS_ABI) == E_MIPS_ABI_EABI64)
    fprintf (file, _(" [abi=EABI64]"));
  else if ((elf_elfheader (abfd)->e_flags & EF_MIPS_ABI))
    fprintf (file, _(" [abi unknown]"));
  else if (ABI_N32_P (abfd))
    fprintf (file, _(" [abi=N32]"));
  else if (ABI_64_P (abfd))
    fprintf (file, _(" [abi=64]"));
  else
    fprintf (file, _(" [no abi set]"));

  if ((elf_elfheader (abfd)->e_flags & EF_MIPS_ARCH) == E_MIPS_ARCH_1)
    fprintf (file, " [mips1]");
  else if ((elf_elfheader (abfd)->e_flags & EF_MIPS_ARCH) == E_MIPS_ARCH_2)
    fprintf (file, " [mips2]");
  else if ((elf_elfheader (abfd)->e_flags & EF_MIPS_ARCH) == E_MIPS_ARCH_3)
    fprintf (file, " [mips3]");
  else if ((elf_elfheader (abfd)->e_flags & EF_MIPS_ARCH) == E_MIPS_ARCH_4)
    fprintf (file, " [mips4]");
  else if ((elf_elfheader (abfd)->e_flags & EF_MIPS_ARCH) == E_MIPS_ARCH_5)
    fprintf (file, " [mips5]");
  else if ((elf_elfheader (abfd)->e_flags & EF_MIPS_ARCH) == E_MIPS_ARCH_32)
    fprintf (file, " [mips32]");
  else if ((elf_elfheader (abfd)->e_flags & EF_MIPS_ARCH) == E_MIPS_ARCH_64)
    fprintf (file, " [mips64]");
  else if ((elf_elfheader (abfd)->e_flags & EF_MIPS_ARCH) == E_MIPS_ARCH_32R2)
    fprintf (file, " [mips32r2]");
  else if ((elf_elfheader (abfd)->e_flags & EF_MIPS_ARCH) == E_MIPS_ARCH_64R2)
    fprintf (file, " [mips64r2]");
  else if ((elf_elfheader (abfd)->e_flags & EF_MIPS_ARCH) == E_MIPS_ARCH_32R6)
    fprintf (file, " [mips32r6]");
  else if ((elf_elfheader (abfd)->e_flags & EF_MIPS_ARCH) == E_MIPS_ARCH_64R6)
    fprintf (file, " [mips64r6]");
  else
    fprintf (file, _(" [unknown ISA]"));

  if (elf_elfheader (abfd)->e_flags & EF_MIPS_ARCH_ASE_MDMX)
    fprintf (file, " [mdmx]");

  if (elf_elfheader (abfd)->e_flags & EF_MIPS_ARCH_ASE_M16)
    fprintf (file, " [mips16]");

  if (elf_elfheader (abfd)->e_flags & EF_MIPS_ARCH_ASE_MICROMIPS)
    fprintf (file, " [micromips]");

  if (elf_elfheader (abfd)->e_flags & EF_MIPS_NAN2008)
    fprintf (file, " [nan2008]");

  if (elf_elfheader (abfd)->e_flags & EF_MIPS_FP64)
    fprintf (file, " [old fp64]");

  if (elf_elfheader (abfd)->e_flags & EF_MIPS_32BITMODE)
    fprintf (file, " [32bitmode]");
  else
    fprintf (file, _(" [not 32bitmode]"));

  if (elf_elfheader (abfd)->e_flags & EF_MIPS_NOREORDER)
    fprintf (file, " [noreorder]");

  if (elf_elfheader (abfd)->e_flags & EF_MIPS_PIC)
    fprintf (file, " [PIC]");

  if (elf_elfheader (abfd)->e_flags & EF_MIPS_CPIC)
    fprintf (file, " [CPIC]");

  if (elf_elfheader (abfd)->e_flags & EF_MIPS_XGOT)
    fprintf (file, " [XGOT]");

  if (elf_elfheader (abfd)->e_flags & EF_MIPS_UCODE)
    fprintf (file, " [UCODE]");

  fputc ('\n', file);

  if (mips_elf_tdata (abfd)->abiflags_valid)
    {
      Elf_Internal_ABIFlags_v0 *abiflags = &mips_elf_tdata (abfd)->abiflags;
      fprintf (file, "\nMIPS ABI Flags Version: %d\n", abiflags->version);
      fprintf (file, "\nISA: MIPS%d", abiflags->isa_level);
      if (abiflags->isa_rev > 1)
	fprintf (file, "r%d", abiflags->isa_rev);
      fprintf (file, "\nGPR size: %d",
	       get_mips_reg_size (abiflags->gpr_size));
      fprintf (file, "\nCPR1 size: %d",
	       get_mips_reg_size (abiflags->cpr1_size));
      fprintf (file, "\nCPR2 size: %d",
	       get_mips_reg_size (abiflags->cpr2_size));
      fputs ("\nFP ABI: ", file);
      print_mips_fp_abi_value (file, abiflags->fp_abi);
      fputs ("ISA Extension: ", file);
      print_mips_isa_ext (file, abiflags->isa_ext);
      fputs ("\nASEs:", file);
      print_mips_ases (file, abiflags->ases);
      fprintf (file, "\nFLAGS 1: %8.8lx", abiflags->flags1);
      fprintf (file, "\nFLAGS 2: %8.8lx", abiflags->flags2);
      fputc ('\n', file);
    }

  return true;
}

const struct bfd_elf_special_section _bfd_mips_elf_special_sections[] =
{
  { STRING_COMMA_LEN (".lit4"),	  0, SHT_PROGBITS,   SHF_ALLOC + SHF_WRITE + SHF_MIPS_GPREL },
  { STRING_COMMA_LEN (".lit8"),	  0, SHT_PROGBITS,   SHF_ALLOC + SHF_WRITE + SHF_MIPS_GPREL },
  { STRING_COMMA_LEN (".mdebug"), 0, SHT_MIPS_DEBUG, 0 },
  { STRING_COMMA_LEN (".sbss"),	 -2, SHT_NOBITS,     SHF_ALLOC + SHF_WRITE + SHF_MIPS_GPREL },
  { STRING_COMMA_LEN (".sdata"), -2, SHT_PROGBITS,   SHF_ALLOC + SHF_WRITE + SHF_MIPS_GPREL },
  { STRING_COMMA_LEN (".ucode"),  0, SHT_MIPS_UCODE, 0 },
  { STRING_COMMA_LEN (".MIPS.xhash"),  0, SHT_MIPS_XHASH,   SHF_ALLOC },
  { NULL,		      0,  0, 0,		     0 }
};

/* Merge non visibility st_other attributes.  Ensure that the
   STO_OPTIONAL flag is copied into h->other, even if this is not a
   definiton of the symbol.  */
void
_bfd_mips_elf_merge_symbol_attribute (struct elf_link_hash_entry *h,
				      unsigned int st_other,
				      bool definition,
				      bool dynamic ATTRIBUTE_UNUSED)
{
  if ((st_other & ~ELF_ST_VISIBILITY (-1)) != 0)
    {
      unsigned char other;

      other = (definition ? st_other : h->other);
      other &= ~ELF_ST_VISIBILITY (-1);
      h->other = other | ELF_ST_VISIBILITY (h->other);
    }

  if (!definition
      && ELF_MIPS_IS_OPTIONAL (st_other))
    h->other |= STO_OPTIONAL;
}

/* Decide whether an undefined symbol is special and can be ignored.
   This is the case for OPTIONAL symbols on IRIX.  */
bool
_bfd_mips_elf_ignore_undef_symbol (struct elf_link_hash_entry *h)
{
  return ELF_MIPS_IS_OPTIONAL (h->other) != 0;
}

bool
_bfd_mips_elf_common_definition (Elf_Internal_Sym *sym)
{
  return (sym->st_shndx == SHN_COMMON
	  || sym->st_shndx == SHN_MIPS_ACOMMON
	  || sym->st_shndx == SHN_MIPS_SCOMMON);
}

/* Return address for Ith PLT stub in section PLT, for relocation REL
   or (bfd_vma) -1 if it should not be included.  */

bfd_vma
_bfd_mips_elf_plt_sym_val (bfd_vma i, const asection *plt,
			   const arelent *rel ATTRIBUTE_UNUSED)
{
  return (plt->vma
	  + 4 * ARRAY_SIZE (mips_o32_exec_plt0_entry)
	  + i * 4 * ARRAY_SIZE (mips_exec_plt_entry));
}

/* Build a table of synthetic symbols to represent the PLT.  As with MIPS16
   and microMIPS PLT slots we may have a many-to-one mapping between .plt
   and .got.plt and also the slots may be of a different size each we walk
   the PLT manually fetching instructions and matching them against known
   patterns.  To make things easier standard MIPS slots, if any, always come
   first.  As we don't create proper ELF symbols we use the UDATA.I member
   of ASYMBOL to carry ISA annotation.  The encoding used is the same as
   with the ST_OTHER member of the ELF symbol.  */

long
_bfd_mips_elf_get_synthetic_symtab (bfd *abfd,
				    long symcount ATTRIBUTE_UNUSED,
				    asymbol **syms ATTRIBUTE_UNUSED,
				    long dynsymcount, asymbol **dynsyms,
				    asymbol **ret)
{
  static const char pltname[] = "_PROCEDURE_LINKAGE_TABLE_";
  static const char microsuffix[] = "@micromipsplt";
  static const char m16suffix[] = "@mips16plt";
  static const char mipssuffix[] = "@plt";

  bool (*slurp_relocs) (bfd *, asection *, asymbol **, bool);
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  bool micromips_p = MICROMIPS_P (abfd);
  Elf_Internal_Shdr *hdr;
  bfd_byte *plt_data;
  bfd_vma plt_offset;
  unsigned int other;
  bfd_vma entry_size;
  bfd_vma plt0_size;
  asection *relplt;
  bfd_vma opcode;
  asection *plt;
  asymbol *send;
  size_t size;
  char *names;
  long counti;
  arelent *p;
  asymbol *s;
  char *nend;
  long count;
  long pi;
  long i;
  long n;

  *ret = NULL;

  if ((abfd->flags & (DYNAMIC | EXEC_P)) == 0 || dynsymcount <= 0)
    return 0;

  relplt = bfd_get_section_by_name (abfd, ".rel.plt");
  if (relplt == NULL)
    return 0;

  hdr = &elf_section_data (relplt)->this_hdr;
  if (hdr->sh_link != elf_dynsymtab (abfd) || hdr->sh_type != SHT_REL)
    return 0;

  plt = bfd_get_section_by_name (abfd, ".plt");
  if (plt == NULL || (plt->flags & SEC_HAS_CONTENTS) == 0)
    return 0;

  slurp_relocs = get_elf_backend_data (abfd)->s->slurp_reloc_table;
  if (!(*slurp_relocs) (abfd, relplt, dynsyms, true))
    return -1;
  p = relplt->relocation;

  /* Calculating the exact amount of space required for symbols would
     require two passes over the PLT, so just pessimise assuming two
     PLT slots per relocation.  */
  count = NUM_SHDR_ENTRIES (hdr);
  counti = count * bed->s->int_rels_per_ext_rel;
  size = 2 * count * sizeof (asymbol);
  size += count * (sizeof (mipssuffix) +
		   (micromips_p ? sizeof (microsuffix) : sizeof (m16suffix)));
  for (pi = 0; pi < counti; pi += bed->s->int_rels_per_ext_rel)
    size += 2 * strlen ((*p[pi].sym_ptr_ptr)->name);

  /* Add the size of "_PROCEDURE_LINKAGE_TABLE_" too.  */
  size += sizeof (asymbol) + sizeof (pltname);

  if (!bfd_malloc_and_get_section (abfd, plt, &plt_data))
    return -1;

  if (plt->size < 16)
    return -1;

  s = *ret = bfd_malloc (size);
  if (s == NULL)
    return -1;
  send = s + 2 * count + 1;

  names = (char *) send;
  nend = (char *) s + size;
  n = 0;

  opcode = bfd_get_micromips_32 (abfd, plt_data + 12);
  if (opcode == 0x3302fffe)
    {
      if (!micromips_p)
	return -1;
      plt0_size = 2 * ARRAY_SIZE (micromips_o32_exec_plt0_entry);
      other = STO_MICROMIPS;
    }
  else if (opcode == 0x0398c1d0)
    {
      if (!micromips_p)
	return -1;
      plt0_size = 2 * ARRAY_SIZE (micromips_insn32_o32_exec_plt0_entry);
      other = STO_MICROMIPS;
    }
  else
    {
      plt0_size = 4 * ARRAY_SIZE (mips_o32_exec_plt0_entry);
      other = 0;
    }

  s->the_bfd = abfd;
  s->flags = BSF_SYNTHETIC | BSF_FUNCTION | BSF_LOCAL;
  s->section = plt;
  s->value = 0;
  s->name = names;
  s->udata.i = other;
  memcpy (names, pltname, sizeof (pltname));
  names += sizeof (pltname);
  ++s, ++n;

  pi = 0;
  for (plt_offset = plt0_size;
       plt_offset + 8 <= plt->size && s < send;
       plt_offset += entry_size)
    {
      bfd_vma gotplt_addr;
      const char *suffix;
      bfd_vma gotplt_hi;
      bfd_vma gotplt_lo;
      size_t suffixlen;

      opcode = bfd_get_micromips_32 (abfd, plt_data + plt_offset + 4);

      /* Check if the second word matches the expected MIPS16 instruction.  */
      if (opcode == 0x651aeb00)
	{
	  if (micromips_p)
	    return -1;
	  /* Truncated table???  */
	  if (plt_offset + 16 > plt->size)
	    break;
	  gotplt_addr = bfd_get_32 (abfd, plt_data + plt_offset + 12);
	  entry_size = 2 * ARRAY_SIZE (mips16_o32_exec_plt_entry);
	  suffixlen = sizeof (m16suffix);
	  suffix = m16suffix;
	  other = STO_MIPS16;
	}
      /* Likewise the expected microMIPS instruction (no insn32 mode).  */
      else if (opcode == 0xff220000)
	{
	  if (!micromips_p)
	    return -1;
	  gotplt_hi = bfd_get_16 (abfd, plt_data + plt_offset) & 0x7f;
	  gotplt_lo = bfd_get_16 (abfd, plt_data + plt_offset + 2) & 0xffff;
	  gotplt_hi = ((gotplt_hi ^ 0x40) - 0x40) << 18;
	  gotplt_lo <<= 2;
	  gotplt_addr = gotplt_hi + gotplt_lo;
	  gotplt_addr += ((plt->vma + plt_offset) | 3) ^ 3;
	  entry_size = 2 * ARRAY_SIZE (micromips_o32_exec_plt_entry);
	  suffixlen = sizeof (microsuffix);
	  suffix = microsuffix;
	  other = STO_MICROMIPS;
	}
      /* Likewise the expected microMIPS instruction (insn32 mode).  */
      else if ((opcode & 0xffff0000) == 0xff2f0000)
	{
	  gotplt_hi = bfd_get_16 (abfd, plt_data + plt_offset + 2) & 0xffff;
	  gotplt_lo = bfd_get_16 (abfd, plt_data + plt_offset + 6) & 0xffff;
	  gotplt_hi = ((gotplt_hi ^ 0x8000) - 0x8000) << 16;
	  gotplt_lo = (gotplt_lo ^ 0x8000) - 0x8000;
	  gotplt_addr = gotplt_hi + gotplt_lo;
	  entry_size = 2 * ARRAY_SIZE (micromips_insn32_o32_exec_plt_entry);
	  suffixlen = sizeof (microsuffix);
	  suffix = microsuffix;
	  other = STO_MICROMIPS;
	}
      /* Otherwise assume standard MIPS code.  */
      else
	{
	  gotplt_hi = bfd_get_32 (abfd, plt_data + plt_offset) & 0xffff;
	  gotplt_lo = bfd_get_32 (abfd, plt_data + plt_offset + 4) & 0xffff;
	  gotplt_hi = ((gotplt_hi ^ 0x8000) - 0x8000) << 16;
	  gotplt_lo = (gotplt_lo ^ 0x8000) - 0x8000;
	  gotplt_addr = gotplt_hi + gotplt_lo;
	  entry_size = 4 * ARRAY_SIZE (mips_exec_plt_entry);
	  suffixlen = sizeof (mipssuffix);
	  suffix = mipssuffix;
	  other = 0;
	}
      /* Truncated table???  */
      if (plt_offset + entry_size > plt->size)
	break;

      for (i = 0;
	   i < count && p[pi].address != gotplt_addr;
	   i++, pi = (pi + bed->s->int_rels_per_ext_rel) % counti);

      if (i < count)
	{
	  size_t namelen;
	  size_t len;

	  *s = **p[pi].sym_ptr_ptr;
	  /* Undefined syms won't have BSF_LOCAL or BSF_GLOBAL set.  Since
	     we are defining a symbol, ensure one of them is set.  */
	  if ((s->flags & BSF_LOCAL) == 0)
	    s->flags |= BSF_GLOBAL;
	  s->flags |= BSF_SYNTHETIC;
	  s->section = plt;
	  s->value = plt_offset;
	  s->name = names;
	  s->udata.i = other;

	  len = strlen ((*p[pi].sym_ptr_ptr)->name);
	  namelen = len + suffixlen;
	  if (names + namelen > nend)
	    break;

	  memcpy (names, (*p[pi].sym_ptr_ptr)->name, len);
	  names += len;
	  memcpy (names, suffix, suffixlen);
	  names += suffixlen;

	  ++s, ++n;
	  pi = (pi + bed->s->int_rels_per_ext_rel) % counti;
	}
    }

  free (plt_data);

  return n;
}

/* Return the ABI flags associated with ABFD if available.  */

Elf_Internal_ABIFlags_v0 *
bfd_mips_elf_get_abiflags (bfd *abfd)
{
  struct mips_elf_obj_tdata *tdata = mips_elf_tdata (abfd);

  return tdata->abiflags_valid ? &tdata->abiflags : NULL;
}

/* MIPS libc ABI versions, used with the EI_ABIVERSION ELF file header
   field.  Taken from `libc-abis.h' generated at GNU libc build time.
   Using a MIPS_ prefix as other libc targets use different values.  */
enum
{
  MIPS_LIBC_ABI_DEFAULT = 0,
  MIPS_LIBC_ABI_MIPS_PLT,
  MIPS_LIBC_ABI_UNIQUE,
  MIPS_LIBC_ABI_MIPS_O32_FP64,
  MIPS_LIBC_ABI_ABSOLUTE,
  MIPS_LIBC_ABI_XHASH,
  MIPS_LIBC_ABI_MAX
};

bool
_bfd_mips_init_file_header (bfd *abfd, struct bfd_link_info *link_info)
{
  struct mips_elf_link_hash_table *htab = NULL;
  Elf_Internal_Ehdr *i_ehdrp;

  if (!_bfd_elf_init_file_header (abfd, link_info))
    return false;

  i_ehdrp = elf_elfheader (abfd);
  if (link_info)
    {
      htab = mips_elf_hash_table (link_info);
      BFD_ASSERT (htab != NULL);
    }

  if (htab != NULL
      && htab->use_plts_and_copy_relocs
      && htab->root.target_os != is_vxworks)
    i_ehdrp->e_ident[EI_ABIVERSION] = MIPS_LIBC_ABI_MIPS_PLT;

  if (mips_elf_tdata (abfd)->abiflags.fp_abi == Val_GNU_MIPS_ABI_FP_64
      || mips_elf_tdata (abfd)->abiflags.fp_abi == Val_GNU_MIPS_ABI_FP_64A)
    i_ehdrp->e_ident[EI_ABIVERSION] = MIPS_LIBC_ABI_MIPS_O32_FP64;

  /* Mark that we need support for absolute symbols in the dynamic loader.  */
  if (htab != NULL && htab->use_absolute_zero && htab->gnu_target)
    i_ehdrp->e_ident[EI_ABIVERSION] = MIPS_LIBC_ABI_ABSOLUTE;

  /* Mark that we need support for .MIPS.xhash in the dynamic linker,
     if it is the only hash section that will be created.  */
  if (link_info && link_info->emit_gnu_hash && !link_info->emit_hash)
    i_ehdrp->e_ident[EI_ABIVERSION] = MIPS_LIBC_ABI_XHASH;
  return true;
}

int
_bfd_mips_elf_compact_eh_encoding
  (struct bfd_link_info *link_info ATTRIBUTE_UNUSED)
{
  return DW_EH_PE_pcrel | DW_EH_PE_sdata4;
}

/* Return the opcode for can't unwind.  */

int
_bfd_mips_elf_cant_unwind_opcode
  (struct bfd_link_info *link_info ATTRIBUTE_UNUSED)
{
  return COMPACT_EH_CANT_UNWIND_OPCODE;
}

/* Record a position XLAT_LOC in the xlat translation table, associated with
   the hash entry H.  The entry in the translation table will later be
   populated with the real symbol dynindx.  */

void
_bfd_mips_elf_record_xhash_symbol (struct elf_link_hash_entry *h,
				   bfd_vma xlat_loc)
{
  struct mips_elf_link_hash_entry *hmips;

  hmips = (struct mips_elf_link_hash_entry *) h;
  hmips->mipsxhash_loc = xlat_loc;
}
