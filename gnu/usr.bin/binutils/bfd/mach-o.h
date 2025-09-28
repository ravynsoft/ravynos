/* Mach-O support for BFD.
   Copyright (C) 1999-2023 Free Software Foundation, Inc.

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

#ifndef _BFD_MACH_O_H_
#define _BFD_MACH_O_H_

#include "mach-o/loader.h"
#include "mach-o/external.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct bfd_mach_o_header
{
  unsigned long magic;
  unsigned long cputype;
  unsigned long cpusubtype;
  unsigned long filetype;
  unsigned long ncmds;
  unsigned long sizeofcmds;
  unsigned long flags;
  unsigned int reserved;
  /* Version 1: 32 bits, version 2: 64 bits.  */
  unsigned int version;
  enum bfd_endian byteorder;
}
bfd_mach_o_header;

typedef struct bfd_mach_o_asymbol
{
  /* The actual symbol which the rest of BFD works with.  */
  asymbol symbol;

  /* Mach-O symbol fields.  */
  unsigned char n_type;
  unsigned char n_sect;
  unsigned short n_desc;
}
bfd_mach_o_asymbol;

#define BFD_MACH_O_SEGNAME_SIZE 16
#define BFD_MACH_O_SECTNAME_SIZE 16

typedef struct bfd_mach_o_section
{
  /* Fields present in the file.  */
  char sectname[BFD_MACH_O_SECTNAME_SIZE + 1];	/* Always NUL padded.  */
  char segname[BFD_MACH_O_SEGNAME_SIZE + 1];
  bfd_vma addr;
  bfd_vma size;
  bfd_vma offset;
  unsigned long align;
  bfd_vma reloff;
  unsigned long nreloc;
  unsigned long flags;
  unsigned long reserved1;
  unsigned long reserved2;
  unsigned long reserved3;

  /* Corresponding bfd section.  */
  asection *bfdsection;

  /* An array holding the indirect symbols for this section.
     NULL values indicate local symbols.
     The number of symbols is determined from the section size and type.  */

  bfd_mach_o_asymbol **indirect_syms;

  /* Simply linked list.  */
  struct bfd_mach_o_section *next;
}
bfd_mach_o_section;

typedef struct bfd_mach_o_segment_command
{
  char segname[BFD_MACH_O_SEGNAME_SIZE + 1];
  bfd_vma vmaddr;
  bfd_vma vmsize;
  bfd_vma fileoff;
  unsigned long filesize;
  unsigned long maxprot;	/* Maximum permitted protection.  */
  unsigned long initprot;	/* Initial protection.  */
  unsigned long nsects;
  unsigned long flags;

  /* Linked list of sections.  */
  bfd_mach_o_section *sect_head;
  bfd_mach_o_section *sect_tail;
}
bfd_mach_o_segment_command;

/* Protection flags.  */
#define BFD_MACH_O_PROT_READ    0x01
#define BFD_MACH_O_PROT_WRITE   0x02
#define BFD_MACH_O_PROT_EXECUTE 0x04

/* Target platforms.  */
#define BFD_MACH_O_PLATFORM_MACOS    1
#define BFD_MACH_O_PLATFORM_IOS      2
#define BFD_MACH_O_PLATFORM_TVOS     3
#define BFD_MACH_O_PLATFORM_WATCHOS  4
#define BFD_MACH_O_PLATFORM_BRIDGEOS 5

/* Build tools.  */
#define BFD_MACH_O_TOOL_CLANG 1
#define BFD_MACH_O_TOOL_SWIFT 2
#define BFD_MACH_O_TOOL_LD    3

/* Expanded internal representation of a relocation entry.  */
typedef struct bfd_mach_o_reloc_info
{
  bfd_vma r_address;
  bfd_vma r_value;
  unsigned int r_scattered : 1;
  unsigned int r_type : 4;
  unsigned int r_pcrel : 1;
  unsigned int r_length : 2;
  unsigned int r_extern : 1;
}
bfd_mach_o_reloc_info;

/* The symbol table is sorted like this:
 (1) local.
	(otherwise in order of generation)
 (2) external defined
	(sorted by name)
 (3) external undefined / common
	(sorted by name)
*/

typedef struct bfd_mach_o_symtab_command
{
  unsigned int symoff;
  unsigned int nsyms;
  unsigned int stroff;
  unsigned int strsize;
  bfd_mach_o_asymbol *symbols;
  char *strtab;
}
bfd_mach_o_symtab_command;

/* This is the second set of the symbolic information which is used to support
   the data structures for the dynamically link editor.

   The original set of symbolic information in the symtab_command which contains
   the symbol and string tables must also be present when this load command is
   present.  When this load command is present the symbol table is organized
   into three groups of symbols:
       local symbols (static and debugging symbols) - grouped by module
       defined external symbols - grouped by module (sorted by name if not lib)
       undefined external symbols (sorted by name)
   In this load command there are offsets and counts to each of the three groups
   of symbols.

   This load command contains a the offsets and sizes of the following new
   symbolic information tables:
       table of contents
       module table
       reference symbol table
       indirect symbol table
   The first three tables above (the table of contents, module table and
   reference symbol table) are only present if the file is a dynamically linked
   shared library.  For executable and object modules, which are files
   containing only one module, the information that would be in these three
   tables is determined as follows:
       table of contents - the defined external symbols are sorted by name
       module table - the file contains only one module so everything in the
		      file is part of the module.
       reference symbol table - is the defined and undefined external symbols

   For dynamically linked shared library files this load command also contains
   offsets and sizes to the pool of relocation entries for all sections
   separated into two groups:
       external relocation entries
       local relocation entries
   For executable and object modules the relocation entries continue to hang
   off the section structures.  */

typedef struct bfd_mach_o_dylib_module
{
  /* Index into the string table indicating the name of the module.  */
  unsigned long module_name_idx;
  char *module_name;

  /* Index into the symbol table of the first defined external symbol provided
     by the module.  */
  unsigned long iextdefsym;

  /* Number of external symbols provided by this module.  */
  unsigned long nextdefsym;

  /* Index into the external reference table of the first entry
     provided by this module.  */
  unsigned long irefsym;

  /* Number of external reference entries provided by this module.  */
  unsigned long nrefsym;

  /* Index into the symbol table of the first local symbol provided by this
     module.  */
  unsigned long ilocalsym;

  /* Number of local symbols provided by this module.  */
  unsigned long nlocalsym;

  /* Index into the external relocation table of the first entry provided
     by this module.  */
  unsigned long iextrel;

  /* Number of external relocation entries provided by this module.  */
  unsigned long nextrel;

  /* Index in the module initialization section to the pointers for this
     module.  */
  unsigned short iinit;

  /* Index in the module termination section to the pointers for this
     module.  */
  unsigned short iterm;

  /* Number of pointers in the module initialization for this module.  */
  unsigned short ninit;

  /* Number of pointers in the module termination for this module.  */
  unsigned short nterm;

  /* Number of data byte for this module that are used in the __module_info
     section of the __OBJC segment.  */
  unsigned long objc_module_info_size;

  /* Statically linked address of the start of the data for this module
     in the __module_info section of the __OBJC_segment.  */
  bfd_vma objc_module_info_addr;
}
bfd_mach_o_dylib_module;

typedef struct bfd_mach_o_dylib_table_of_content
{
  /* Index into the symbol table to the defined external symbol.  */
  unsigned long symbol_index;

  /* Index into the module table to the module for this entry.  */
  unsigned long module_index;
}
bfd_mach_o_dylib_table_of_content;

typedef struct bfd_mach_o_dylib_reference
{
  /* Index into the symbol table for the symbol being referenced.  */
  unsigned long isym;

  /* Type of the reference being made (use REFERENCE_FLAGS constants).  */
  unsigned long flags;
}
bfd_mach_o_dylib_reference;
#define BFD_MACH_O_REFERENCE_SIZE 4

typedef struct bfd_mach_o_dysymtab_command
{
  /* The symbols indicated by symoff and nsyms of the LC_SYMTAB load command
     are grouped into the following three groups:
       local symbols (further grouped by the module they are from)
       defined external symbols (further grouped by the module they are from)
       undefined symbols

     The local symbols are used only for debugging.  The dynamic binding
     process may have to use them to indicate to the debugger the local
     symbols for a module that is being bound.

     The last two groups are used by the dynamic binding process to do the
     binding (indirectly through the module table and the reference symbol
     table when this is a dynamically linked shared library file).  */

  unsigned long ilocalsym;    /* Index to local symbols.  */
  unsigned long nlocalsym;    /* Number of local symbols.  */
  unsigned long iextdefsym;   /* Index to externally defined symbols.  */
  unsigned long nextdefsym;   /* Number of externally defined symbols.  */
  unsigned long iundefsym;    /* Index to undefined symbols.  */
  unsigned long nundefsym;    /* Number of undefined symbols.  */

  /* For the for the dynamic binding process to find which module a symbol
     is defined in the table of contents is used (analogous to the ranlib
     structure in an archive) which maps defined external symbols to modules
     they are defined in.  This exists only in a dynamically linked shared
     library file.  For executable and object modules the defined external
     symbols are sorted by name and is use as the table of contents.  */

  unsigned long tocoff;       /* File offset to table of contents.  */
  unsigned long ntoc;	      /* Number of entries in table of contents.  */

  /* To support dynamic binding of "modules" (whole object files) the symbol
     table must reflect the modules that the file was created from.  This is
     done by having a module table that has indexes and counts into the merged
     tables for each module.  The module structure that these two entries
     refer to is described below.  This exists only in a dynamically linked
     shared library file.  For executable and object modules the file only
     contains one module so everything in the file belongs to the module.  */

  unsigned long modtaboff;    /* File offset to module table.  */
  unsigned long nmodtab;      /* Number of module table entries.  */

  /* To support dynamic module binding the module structure for each module
     indicates the external references (defined and undefined) each module
     makes.  For each module there is an offset and a count into the
     reference symbol table for the symbols that the module references.
     This exists only in a dynamically linked shared library file.  For
     executable and object modules the defined external symbols and the
     undefined external symbols indicates the external references.  */

  unsigned long extrefsymoff;  /* Offset to referenced symbol table.  */
  unsigned long nextrefsyms;   /* Number of referenced symbol table entries.  */

  /* The sections that contain "symbol pointers" and "routine stubs" have
     indexes and (implied counts based on the size of the section and fixed
     size of the entry) into the "indirect symbol" table for each pointer
     and stub.  For every section of these two types the index into the
     indirect symbol table is stored in the section header in the field
     reserved1.  An indirect symbol table entry is simply a 32bit index into
     the symbol table to the symbol that the pointer or stub is referring to.
     The indirect symbol table is ordered to match the entries in the section.  */

  unsigned long indirectsymoff; /* File offset to the indirect symbol table.  */
  unsigned long nindirectsyms;  /* Number of indirect symbol table entries.  */

  /* To support relocating an individual module in a library file quickly the
     external relocation entries for each module in the library need to be
     accessed efficiently.  Since the relocation entries can't be accessed
     through the section headers for a library file they are separated into
     groups of local and external entries further grouped by module.  In this
     case the presents of this load command who's extreloff, nextrel,
     locreloff and nlocrel fields are non-zero indicates that the relocation
     entries of non-merged sections are not referenced through the section
     structures (and the reloff and nreloc fields in the section headers are
     set to zero).

     Since the relocation entries are not accessed through the section headers
     this requires the r_address field to be something other than a section
     offset to identify the item to be relocated.  In this case r_address is
     set to the offset from the vmaddr of the first LC_SEGMENT command.

     The relocation entries are grouped by module and the module table
     entries have indexes and counts into them for the group of external
     relocation entries for that the module.

     For sections that are merged across modules there must not be any
     remaining external relocation entries for them (for merged sections
     remaining relocation entries must be local).  */

  unsigned long extreloff;    /* Offset to external relocation entries.  */
  unsigned long nextrel;      /* Number of external relocation entries.  */

  /* All the local relocation entries are grouped together (they are not
     grouped by their module since they are only used if the object is moved
     from it statically link edited address).  */

  unsigned long locreloff;    /* Offset to local relocation entries.  */
  unsigned long nlocrel;      /* Number of local relocation entries.  */

  bfd_mach_o_dylib_module *dylib_module;
  bfd_mach_o_dylib_table_of_content *dylib_toc;
  unsigned int *indirect_syms;
  bfd_mach_o_dylib_reference *ext_refs;
}
bfd_mach_o_dysymtab_command;

/* An indirect symbol table entry is simply a 32bit index into the symbol table
   to the symbol that the pointer or stub is refering to.  Unless it is for a
   non-lazy symbol pointer section for a defined symbol which strip(1) has
   removed.  In which case it has the value INDIRECT_SYMBOL_LOCAL.  If the
   symbol was also absolute INDIRECT_SYMBOL_ABS is or'ed with that.  */

#define BFD_MACH_O_INDIRECT_SYMBOL_LOCAL 0x80000000
#define BFD_MACH_O_INDIRECT_SYMBOL_ABS   0x40000000
#define BFD_MACH_O_INDIRECT_SYMBOL_SIZE  4

/* For LC_TWOLEVEL_HINTS.  */

typedef struct bfd_mach_o_twolevel_hints_command
{
  /* Offset to the hint table.  */
  unsigned int offset;

  /* Number of entries in the table.  */
  unsigned int nhints;
}
bfd_mach_o_twolevel_hints_command;

/* For LC_PREBIND_CKSUM.  */

typedef struct bfd_mach_o_prebind_cksum_command
{
  /* Checksum or zero.  */
  unsigned int cksum;
}
bfd_mach_o_prebind_cksum_command;

/* For LC_THREAD or LC_UNIXTHREAD.  */

typedef struct bfd_mach_o_thread_flavour
{
  unsigned long flavour;
  unsigned long offset;
  unsigned long size;
}
bfd_mach_o_thread_flavour;

typedef struct bfd_mach_o_thread_command
{
  unsigned long nflavours;
  bfd_mach_o_thread_flavour *flavours;
  asection *section;
}
bfd_mach_o_thread_command;

/* For LC_LOAD_DYLINKER and LC_ID_DYLINKER.  */

typedef struct bfd_mach_o_dylinker_command
{
  unsigned int name_offset;	    /* Offset to library's path name.  */
  char *name_str;
}
bfd_mach_o_dylinker_command;

/* For LC_LOAD_DYLIB, LC_LOAD_WEAK_DYLIB, LC_ID_DYLIB
   or LC_REEXPORT_DYLIB.  */

typedef struct bfd_mach_o_dylib_command
{
  unsigned int name_offset;	       /* Offset to library's path name.  */
  unsigned long timestamp;	       /* Library's build time stamp.  */
  unsigned long current_version;       /* Library's current version number.  */
  unsigned long compatibility_version; /* Library's compatibility vers number.  */
  char *name_str;
}
bfd_mach_o_dylib_command;

/* For LC_PREBOUND_DYLIB.  */

typedef struct bfd_mach_o_prebound_dylib_command
{
  unsigned int name_offset;	      /* Library's path name.  */
  unsigned int nmodules;	      /* Number of modules in library.  */
  unsigned int linked_modules_offset; /* Bit vector of linked modules.  */

  char *name_str;
  unsigned char *linked_modules;
}
bfd_mach_o_prebound_dylib_command;

/* For LC_UUID.  */

typedef struct bfd_mach_o_uuid_command
{
  unsigned char uuid[16];
}
bfd_mach_o_uuid_command;

/* For LC_CODE_SIGNATURE or LC_SEGMENT_SPLIT_INFO.  */

typedef struct bfd_mach_o_linkedit_command
{
  unsigned long dataoff;
  unsigned long datasize;
}
bfd_mach_o_linkedit_command;

typedef struct bfd_mach_o_str_command
{
  unsigned long stroff;
  unsigned long str_len;
  char *str;
}
bfd_mach_o_str_command;

typedef struct bfd_mach_o_fvmlib_command
{
  unsigned int name_offset;
  char *name_str;
  unsigned int minor_version;
  unsigned int header_addr;
}
bfd_mach_o_fvmlib_command;

typedef struct bfd_mach_o_dyld_info_command
{
  /* File offset and size to rebase info.  */
  unsigned int rebase_off;
  unsigned int rebase_size;
  unsigned char *rebase_content;

  /* File offset and size of binding info.  */
  unsigned int bind_off;
  unsigned int bind_size;
  unsigned char *bind_content;

  /* File offset and size of weak binding info.  */
  unsigned int weak_bind_off;
  unsigned int weak_bind_size;
  unsigned char *weak_bind_content;

  /* File offset and size of lazy binding info.  */
  unsigned int lazy_bind_off;
  unsigned int lazy_bind_size;
  unsigned char *lazy_bind_content;

  /* File offset and size of export info.  */
  unsigned int export_off;
  unsigned int export_size;
  unsigned char *export_content;
}
bfd_mach_o_dyld_info_command;

typedef struct bfd_mach_o_version_min_command
{
  uint32_t version;
  uint32_t sdk;
}
bfd_mach_o_version_min_command;

typedef struct bfd_mach_o_encryption_info_command
{
  unsigned int cryptoff;
  unsigned int cryptsize;
  unsigned int cryptid;
}
bfd_mach_o_encryption_info_command;

typedef struct bfd_mach_o_main_command
{
  uint64_t entryoff;
  uint64_t stacksize;
}
bfd_mach_o_main_command;

typedef struct bfd_mach_o_source_version_command
{
  unsigned int a;
  unsigned short b;
  unsigned short c;
  unsigned short d;
  unsigned short e;
}
bfd_mach_o_source_version_command;

typedef struct bfd_mach_o_note_command
{
  char data_owner[16];
  uint64_t offset;
  uint64_t size;
}
bfd_mach_o_note_command;

typedef struct bfd_mach_o_build_version_tool
{
  uint32_t tool;
  uint32_t version;
}
bfd_mach_o_build_version_tool;

typedef struct bfd_mach_o_build_version_command
{
  uint32_t platform;
  uint32_t minos;
  uint32_t sdk;
  uint32_t ntools;
}
bfd_mach_o_build_version_command;

typedef struct bfd_mach_o_load_command
{
  /* Next command in the single linked list.  */
  struct bfd_mach_o_load_command *next;

  /* Type and required flag.  */
  bfd_mach_o_load_command_type type;
  bool type_required;

  /* Offset and length in the file.  */
  unsigned int offset;
  unsigned int len;

  union
  {
    bfd_mach_o_segment_command segment;
    bfd_mach_o_symtab_command symtab;
    bfd_mach_o_dysymtab_command dysymtab;
    bfd_mach_o_thread_command thread;
    bfd_mach_o_dylib_command dylib;
    bfd_mach_o_dylinker_command dylinker;
    bfd_mach_o_prebound_dylib_command prebound_dylib;
    bfd_mach_o_prebind_cksum_command prebind_cksum;
    bfd_mach_o_twolevel_hints_command twolevel_hints;
    bfd_mach_o_uuid_command uuid;
    bfd_mach_o_linkedit_command linkedit;
    bfd_mach_o_str_command str;
    bfd_mach_o_dyld_info_command dyld_info;
    bfd_mach_o_version_min_command version_min;
    bfd_mach_o_encryption_info_command encryption_info;
    bfd_mach_o_fvmlib_command fvmlib;
    bfd_mach_o_main_command main;
    bfd_mach_o_source_version_command source_version;
    bfd_mach_o_note_command note;
    bfd_mach_o_build_version_command build_version;
  } command;
}
bfd_mach_o_load_command;

typedef struct mach_o_data_struct
{
  /* Mach-O header.  */
  bfd_mach_o_header header;

  /* File offset of the header.  Usually this is 0.  */
  file_ptr hdr_offset;

  /* Array of load commands (length is given by header.ncmds).  */
  bfd_mach_o_load_command *first_command;
  bfd_mach_o_load_command *last_command;

  /* Flatten array of sections.  The array is 0-based.  */
  unsigned long nsects;
  bfd_mach_o_section **sections;

  /* Used while writing: current length of the output file.  This is used
     to allocate space in the file.  */
  ufile_ptr filelen;

  /* As symtab is referenced by other load command, it is handy to have
     a direct access to it.  Although it is not clearly stated, only one symtab
     is expected.  */
  bfd_mach_o_symtab_command *symtab;
  bfd_mach_o_dysymtab_command *dysymtab;

  /* A place to stash dwarf2 info for this bfd.  */
  void *dwarf2_find_line_info;

  /* BFD of .dSYM file.  */
  bfd *dsym_bfd;

  /* Cache of dynamic relocs.  */
  arelent *dyn_reloc_cache;
}
bfd_mach_o_data_struct;

typedef struct bfd_mach_o_xlat_name
{
  const char *name;
  unsigned long val;
}
bfd_mach_o_xlat_name;

/* Target specific routines.  */

#define bfd_mach_o_get_data(abfd) ((abfd)->tdata.mach_o_data)
#define bfd_mach_o_get_backend_data(abfd) \
  ((bfd_mach_o_backend_data*)(abfd)->xvec->backend_data)

/* Get the Mach-O header for section SEC.  */
#define bfd_mach_o_get_mach_o_section(sec) \
  ((bfd_mach_o_section *)(sec)->used_by_bfd)

bool bfd_mach_o_valid (bfd *);
bool bfd_mach_o_mkobject_init (bfd *);
bfd_cleanup bfd_mach_o_object_p (bfd *);
bfd_cleanup bfd_mach_o_core_p (bfd *);
bfd_cleanup bfd_mach_o_fat_archive_p (bfd *);
bfd *bfd_mach_o_fat_openr_next_archived_file (bfd *, bfd *);
bool bfd_mach_o_set_arch_mach (bfd *, enum bfd_architecture, unsigned long);
int bfd_mach_o_lookup_command (bfd *, bfd_mach_o_load_command_type,
			       bfd_mach_o_load_command **);
bool bfd_mach_o_new_section_hook (bfd *, asection *);
bool bfd_mach_o_write_contents (bfd *);
bool bfd_mach_o_bfd_copy_private_symbol_data (bfd *, asymbol *,
					      bfd *, asymbol *);
bool bfd_mach_o_bfd_copy_private_section_data (bfd *, asection *,
					       bfd *, asection *);
bool bfd_mach_o_bfd_copy_private_header_data (bfd *, bfd *);
bool bfd_mach_o_bfd_set_private_flags (bfd *, flagword);
bool bfd_mach_o_bfd_print_private_bfd_data (bfd *, void *);
long bfd_mach_o_get_symtab_upper_bound (bfd *);
long bfd_mach_o_canonicalize_symtab (bfd *, asymbol **);
long bfd_mach_o_get_synthetic_symtab (bfd *, long, asymbol **, long,
				      asymbol **, asymbol **ret);
long bfd_mach_o_get_reloc_upper_bound (bfd *, asection *);
long bfd_mach_o_canonicalize_reloc (bfd *, asection *, arelent **, asymbol **);
long bfd_mach_o_get_dynamic_reloc_upper_bound (bfd *);
long bfd_mach_o_canonicalize_dynamic_reloc (bfd *, arelent **, asymbol **);
asymbol *bfd_mach_o_make_empty_symbol (bfd *);
void bfd_mach_o_get_symbol_info (bfd *, asymbol *, symbol_info *);
void bfd_mach_o_print_symbol (bfd *, void *, asymbol *, bfd_print_symbol_type);
int bfd_mach_o_sizeof_headers (bfd *, struct bfd_link_info *);
unsigned long bfd_mach_o_stack_addr (enum bfd_mach_o_cpu_type);
int bfd_mach_o_core_fetch_environment (bfd *, unsigned char **, unsigned int *);
char *bfd_mach_o_core_file_failing_command (bfd *);
int bfd_mach_o_core_file_failing_signal (bfd *);
bool bfd_mach_o_core_file_matches_executable_p (bfd *, bfd *);
bfd *bfd_mach_o_fat_extract (bfd *, bfd_format , const bfd_arch_info_type *);
bfd_cleanup bfd_mach_o_header_p (bfd *, file_ptr, bfd_mach_o_filetype,
				 bfd_mach_o_cpu_type);
bool bfd_mach_o_build_commands (bfd *);
bool bfd_mach_o_set_section_contents (bfd *, asection *, const void *,
				      file_ptr, bfd_size_type);
unsigned int bfd_mach_o_version (bfd *);

unsigned int bfd_mach_o_get_section_type_from_name (bfd *, const char *);
unsigned int bfd_mach_o_get_section_attribute_from_name (const char *);

void bfd_mach_o_convert_section_name_to_bfd (bfd *, const char *, const char *,
					     const char **, flagword *);
bool bfd_mach_o_find_nearest_line (bfd *, asymbol **,
					  asection *, bfd_vma,
					  const char **, const char **,
					  unsigned int *, unsigned int *);
#define bfd_mach_o_find_nearest_line_with_alt \
  _bfd_nosymbols_find_nearest_line_with_alt
#define bfd_mach_o_find_line _bfd_nosymbols_find_line
bool bfd_mach_o_close_and_cleanup (bfd *);
bool bfd_mach_o_bfd_free_cached_info (bfd *);

unsigned int bfd_mach_o_section_get_nbr_indirect (bfd *, bfd_mach_o_section *);
unsigned int bfd_mach_o_section_get_entry_size (bfd *, bfd_mach_o_section *);
bool bfd_mach_o_read_symtab_symbols (bfd *);
bool bfd_mach_o_read_symtab_strtab (bfd *abfd);

bfd_vma bfd_mach_o_get_base_address (bfd *);

void bfd_mach_o_swap_in_non_scattered_reloc (bfd *, bfd_mach_o_reloc_info *,
					     unsigned char *);
bool bfd_mach_o_canonicalize_non_scattered_reloc (bfd *, bfd_mach_o_reloc_info *, arelent *, asymbol **);
bool bfd_mach_o_pre_canonicalize_one_reloc (bfd *, struct mach_o_reloc_info_external *, bfd_mach_o_reloc_info *, arelent *, asymbol **);

/* A placeholder in case we need to suppress emitting the dysymtab for some
   reason (e.g. compatibility with older system versions).  */
#define bfd_mach_o_should_emit_dysymtab(x) true

extern const bfd_mach_o_xlat_name bfd_mach_o_section_attribute_name[];
extern const bfd_mach_o_xlat_name bfd_mach_o_section_type_name[];

extern const bfd_target mach_o_fat_vec;

/* Interfaces between BFD names and Mach-O names.  */

typedef struct mach_o_section_name_xlat
{
  const char *bfd_name;
  const char *mach_o_name;
  flagword bfd_flags;
  unsigned int macho_sectype;
  unsigned int macho_secattr;
  unsigned int sectalign;
} mach_o_section_name_xlat;

typedef struct mach_o_segment_name_xlat
{
  const char *segname;
  const mach_o_section_name_xlat *sections;
} mach_o_segment_name_xlat;

const mach_o_section_name_xlat *
bfd_mach_o_section_data_for_mach_sect (bfd *, const char *, const char *);
const mach_o_section_name_xlat *
bfd_mach_o_section_data_for_bfd_name (bfd *, const char *, const char **);

typedef struct bfd_mach_o_backend_data
{
  enum bfd_architecture arch;
  bfd_vma page_size;
  bool (*_bfd_mach_o_canonicalize_one_reloc)
  (bfd *, struct mach_o_reloc_info_external *, arelent *, asymbol **, arelent *);
  bool (*_bfd_mach_o_swap_reloc_out)(arelent *, bfd_mach_o_reloc_info *);
  bool (*_bfd_mach_o_print_thread)(bfd *, bfd_mach_o_thread_flavour *,
					  void *, char *);
  const mach_o_segment_name_xlat *segsec_names_xlat;
  bool (*bfd_mach_o_section_type_valid_for_target) (unsigned long);
}
bfd_mach_o_backend_data;

/* Values used in symbol.udata.i, to signal that the mach-o-specific data in the
   symbol are not yet set, or need validation (where this is possible).  */

#define SYM_MACHO_FIELDS_UNSET ((bfd_vma) -1)
#define SYM_MACHO_FIELDS_NOT_VALIDATED ((bfd_vma) -2)

#ifdef __cplusplus
}
#endif

#endif /* _BFD_MACH_O_H_ */
