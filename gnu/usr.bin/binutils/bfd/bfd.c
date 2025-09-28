/* Generic BFD library interface and support routines.
   Copyright (C) 1990-2023 Free Software Foundation, Inc.
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

/*
INODE
typedef bfd, Error reporting, BFD front end, BFD front end

SECTION
	<<typedef bfd>>

	A BFD has type <<bfd>>; objects of this type are the
	cornerstone of any application using BFD. Using BFD
	consists of making references though the BFD and to data in the BFD.

	Here is the structure that defines the type <<bfd>>.  It
	contains the major data about the file and pointers
	to the rest of the data.

EXTERNAL
.typedef enum bfd_format
.  {
.    bfd_unknown = 0,	{* File format is unknown.  *}
.    bfd_object,	{* Linker/assembler/compiler output.  *}
.    bfd_archive,	{* Object archive file.  *}
.    bfd_core,		{* Core dump.  *}
.    bfd_type_end	{* Marks the end; don't use it!  *}
.  }
.bfd_format;
.
.enum bfd_direction
.  {
.    no_direction = 0,
.    read_direction = 1,
.    write_direction = 2,
.    both_direction = 3
.  };
.
.enum bfd_plugin_format
.  {
.    bfd_plugin_unknown = 0,
.    bfd_plugin_yes = 1,
.    bfd_plugin_no = 2
.  };
.
.struct bfd_build_id
.  {
.    bfd_size_type size;
.    bfd_byte data[1];
.  };
.

CODE_FRAGMENT
.struct bfd
.{
.  {* The filename the application opened the BFD with.  *}
.  const char *filename;
.
.  {* A pointer to the target jump table.  *}
.  const struct bfd_target *xvec;
.
.  {* The IOSTREAM, and corresponding IO vector that provide access
.     to the file backing the BFD.  *}
.  void *iostream;
.  const struct bfd_iovec *iovec;
.
.  {* The caching routines use these to maintain a
.     least-recently-used list of BFDs.  *}
.  struct bfd *lru_prev, *lru_next;
.
.  {* Track current file position (or current buffer offset for
.     in-memory BFDs).  When a file is closed by the caching routines,
.     BFD retains state information on the file here.  *}
.  ufile_ptr where;
.
.  {* File modified time, if mtime_set is TRUE.  *}
.  long mtime;
.
.  {* A unique identifier of the BFD  *}
.  unsigned int id;
.
.  {* Format_specific flags.  *}
.  flagword flags;
.
.  {* Values that may appear in the flags field of a BFD.  These also
.     appear in the object_flags field of the bfd_target structure, where
.     they indicate the set of flags used by that backend (not all flags
.     are meaningful for all object file formats) (FIXME: at the moment,
.     the object_flags values have mostly just been copied from backend
.     to another, and are not necessarily correct).  *}
.
.#define BFD_NO_FLAGS                0x0
.
.  {* BFD contains relocation entries.  *}
.#define HAS_RELOC                   0x1
.
.  {* BFD is directly executable.  *}
.#define EXEC_P                      0x2
.
.  {* BFD has line number information (basically used for F_LNNO in a
.     COFF header).  *}
.#define HAS_LINENO                  0x4
.
.  {* BFD has debugging information.  *}
.#define HAS_DEBUG                  0x08
.
.  {* BFD has symbols.  *}
.#define HAS_SYMS                   0x10
.
.  {* BFD has local symbols (basically used for F_LSYMS in a COFF
.     header).  *}
.#define HAS_LOCALS                 0x20
.
.  {* BFD is a dynamic object.  *}
.#define DYNAMIC                    0x40
.
.  {* Text section is write protected (if D_PAGED is not set, this is
.     like an a.out NMAGIC file) (the linker sets this by default, but
.     clears it for -r or -N).  *}
.#define WP_TEXT                    0x80
.
.  {* BFD is dynamically paged (this is like an a.out ZMAGIC file) (the
.     linker sets this by default, but clears it for -r or -n or -N).  *}
.#define D_PAGED                   0x100
.
.  {* BFD is relaxable (this means that bfd_relax_section may be able to
.     do something) (sometimes bfd_relax_section can do something even if
.     this is not set).  *}
.#define BFD_IS_RELAXABLE          0x200
.
.  {* This may be set before writing out a BFD to request using a
.     traditional format.  For example, this is used to request that when
.     writing out an a.out object the symbols not be hashed to eliminate
.     duplicates.  *}
.#define BFD_TRADITIONAL_FORMAT    0x400
.
.  {* This flag indicates that the BFD contents are actually cached
.     in memory.  If this is set, iostream points to a bfd_in_memory
.     struct.  *}
.#define BFD_IN_MEMORY             0x800
.
.  {* This BFD has been created by the linker and doesn't correspond
.     to any input file.  *}
.#define BFD_LINKER_CREATED       0x1000
.
.  {* This may be set before writing out a BFD to request that it
.     be written using values for UIDs, GIDs, timestamps, etc. that
.     will be consistent from run to run.  *}
.#define BFD_DETERMINISTIC_OUTPUT 0x2000
.
.  {* Compress sections in this BFD.  *}
.#define BFD_COMPRESS             0x4000
.
.  {* Decompress sections in this BFD.  *}
.#define BFD_DECOMPRESS           0x8000
.
.  {* BFD is a dummy, for plugins.  *}
.#define BFD_PLUGIN              0x10000
.
.  {* Compress sections in this BFD with SHF_COMPRESSED from gABI.  *}
.#define BFD_COMPRESS_GABI       0x20000
.
.  {* Convert ELF common symbol type to STT_COMMON or STT_OBJECT in this
.     BFD.  *}
.#define BFD_CONVERT_ELF_COMMON  0x40000
.
.  {* Use the ELF STT_COMMON type in this BFD.  *}
.#define BFD_USE_ELF_STT_COMMON  0x80000
.
.  {* Put pathnames into archives (non-POSIX).  *}
.#define BFD_ARCHIVE_FULL_PATH  0x100000
.
.#define BFD_CLOSED_BY_CACHE    0x200000

.  {* Compress sections in this BFD with SHF_COMPRESSED zstd.  *}
.#define BFD_COMPRESS_ZSTD      0x400000
.
.  {* Don't generate ELF section header.  *}
.#define BFD_NO_SECTION_HEADER	0x800000
.
.  {* Flags bits which are for BFD use only.  *}
.#define BFD_FLAGS_FOR_BFD_USE_MASK \
.  (BFD_IN_MEMORY | BFD_COMPRESS | BFD_DECOMPRESS | BFD_LINKER_CREATED \
.   | BFD_PLUGIN | BFD_TRADITIONAL_FORMAT | BFD_DETERMINISTIC_OUTPUT \
.   | BFD_COMPRESS_GABI | BFD_CONVERT_ELF_COMMON | BFD_USE_ELF_STT_COMMON \
.   | BFD_NO_SECTION_HEADER)
.
.  {* The format which belongs to the BFD. (object, core, etc.)  *}
.  ENUM_BITFIELD (bfd_format) format : 3;
.
.  {* The direction with which the BFD was opened.  *}
.  ENUM_BITFIELD (bfd_direction) direction : 2;
.
.  {* Is the file descriptor being cached?  That is, can it be closed as
.     needed, and re-opened when accessed later?  *}
.  unsigned int cacheable : 1;
.
.  {* Marks whether there was a default target specified when the
.     BFD was opened. This is used to select which matching algorithm
.     to use to choose the back end.  *}
.  unsigned int target_defaulted : 1;
.
.  {* ... and here: (``once'' means at least once).  *}
.  unsigned int opened_once : 1;
.
.  {* Set if we have a locally maintained mtime value, rather than
.     getting it from the file each time.  *}
.  unsigned int mtime_set : 1;
.
.  {* Flag set if symbols from this BFD should not be exported.  *}
.  unsigned int no_export : 1;
.
.  {* Remember when output has begun, to stop strange things
.     from happening.  *}
.  unsigned int output_has_begun : 1;
.
.  {* Have archive map.  *}
.  unsigned int has_armap : 1;
.
.  {* Set if this is a thin archive.  *}
.  unsigned int is_thin_archive : 1;
.
.  {* Set if this archive should not cache element positions.  *}
.  unsigned int no_element_cache : 1;
.
.  {* Set if only required symbols should be added in the link hash table for
.     this object.  Used by VMS linkers.  *}
.  unsigned int selective_search : 1;
.
.  {* Set if this is the linker output BFD.  *}
.  unsigned int is_linker_output : 1;
.
.  {* Set if this is the linker input BFD.  *}
.  unsigned int is_linker_input : 1;
.
.  {* If this is an input for a compiler plug-in library.  *}
.  ENUM_BITFIELD (bfd_plugin_format) plugin_format : 2;
.
.  {* Set if this is a plugin output file.  *}
.  unsigned int lto_output : 1;
.
.  {* Set if this is a slim LTO object not loaded with a compiler plugin.  *}
.  unsigned int lto_slim_object : 1;
.
.  {* Do not attempt to modify this file.  Set when detecting errors
.     that BFD is not prepared to handle for objcopy/strip.  *}
.  unsigned int read_only : 1;
.
.  {* Set to dummy BFD created when claimed by a compiler plug-in
.     library.  *}
.  bfd *plugin_dummy_bfd;
.
.  {* The offset of this bfd in the file, typically 0 if it is not
.     contained in an archive.  *}
.  ufile_ptr origin;
.
.  {* The origin in the archive of the proxy entry.  This will
.     normally be the same as origin, except for thin archives,
.     when it will contain the current offset of the proxy in the
.     thin archive rather than the offset of the bfd in its actual
.     container.  *}
.  ufile_ptr proxy_origin;
.
.  {* A hash table for section names.  *}
.  struct bfd_hash_table section_htab;
.
.  {* Pointer to linked list of sections.  *}
.  struct bfd_section *sections;
.
.  {* The last section on the section list.  *}
.  struct bfd_section *section_last;
.
.  {* The number of sections.  *}
.  unsigned int section_count;
.
.  {* The archive plugin file descriptor.  *}
.  int archive_plugin_fd;
.
.  {* The number of opens on the archive plugin file descriptor.  *}
.  unsigned int archive_plugin_fd_open_count;
.
.  {* A field used by _bfd_generic_link_add_archive_symbols.  This will
.     be used only for archive elements.  *}
.  int archive_pass;
.
.  {* The total size of memory from bfd_alloc.  *}
.  bfd_size_type alloc_size;
.
.  {* Stuff only useful for object files:
.     The start address.  *}
.  bfd_vma start_address;
.
.  {* Symbol table for output BFD (with symcount entries).
.     Also used by the linker to cache input BFD symbols.  *}
.  struct bfd_symbol **outsymbols;
.
.  {* Used for input and output.  *}
.  unsigned int symcount;
.
.  {* Used for slurped dynamic symbol tables.  *}
.  unsigned int dynsymcount;
.
.  {* Pointer to structure which contains architecture information.  *}
.  const struct bfd_arch_info *arch_info;
.
.  {* Cached length of file for bfd_get_size.  0 until bfd_get_size is
.     called, 1 if stat returns an error or the file size is too large to
.     return in ufile_ptr.  Both 0 and 1 should be treated as "unknown".  *}
.  ufile_ptr size;
.
.  {* Stuff only useful for archives.  *}
.  void *arelt_data;
.  struct bfd *my_archive;      {* The containing archive BFD.  *}
.  struct bfd *archive_next;    {* The next BFD in the archive.  *}
.  struct bfd *archive_head;    {* The first BFD in the archive.  *}
.  struct bfd *nested_archives; {* List of nested archive in a flattened
.				   thin archive.  *}
.
.  union {
.    {* For input BFDs, a chain of BFDs involved in a link.  *}
.    struct bfd *next;
.    {* For output BFD, the linker hash table.  *}
.    struct bfd_link_hash_table *hash;
.  } link;
.
.  {* Used by the back end to hold private data.  *}
.  union
.    {
.      struct aout_data_struct *aout_data;
.      struct artdata *aout_ar_data;
.      struct coff_tdata *coff_obj_data;
.      struct pe_tdata *pe_obj_data;
.      struct xcoff_tdata *xcoff_obj_data;
.      struct ecoff_tdata *ecoff_obj_data;
.      struct srec_data_struct *srec_data;
.      struct verilog_data_struct *verilog_data;
.      struct ihex_data_struct *ihex_data;
.      struct tekhex_data_struct *tekhex_data;
.      struct elf_obj_tdata *elf_obj_data;
.      struct mmo_data_struct *mmo_data;
.      struct trad_core_struct *trad_core_data;
.      struct som_data_struct *som_data;
.      struct hpux_core_struct *hpux_core_data;
.      struct hppabsd_core_struct *hppabsd_core_data;
.      struct sgi_core_struct *sgi_core_data;
.      struct lynx_core_struct *lynx_core_data;
.      struct osf_core_struct *osf_core_data;
.      struct cisco_core_struct *cisco_core_data;
.      struct netbsd_core_struct *netbsd_core_data;
.      struct mach_o_data_struct *mach_o_data;
.      struct mach_o_fat_data_struct *mach_o_fat_data;
.      struct plugin_data_struct *plugin_data;
.      struct bfd_pef_data_struct *pef_data;
.      struct bfd_pef_xlib_data_struct *pef_xlib_data;
.      struct bfd_sym_data_struct *sym_data;
.      void *any;
.    }
.  tdata;
.
.  {* Used by the application to hold private data.  *}
.  void *usrdata;
.
.  {* Where all the allocated stuff under this BFD goes.  This is a
.     struct objalloc *, but we use void * to avoid requiring the inclusion
.     of objalloc.h.  *}
.  void *memory;
.
.  {* For input BFDs, the build ID, if the object has one. *}
.  const struct bfd_build_id *build_id;
.};
.

EXTERNAL
.static inline const char *
.bfd_get_filename (const bfd *abfd)
.{
.  return abfd->filename;
.}
.
.static inline bool
.bfd_get_cacheable (const bfd *abfd)
.{
.  return abfd->cacheable;
.}
.
.static inline enum bfd_format
.bfd_get_format (const bfd *abfd)
.{
.  return abfd->format;
.}
.
.static inline flagword
.bfd_get_file_flags (const bfd *abfd)
.{
.  return abfd->flags;
.}
.
.static inline bfd_vma
.bfd_get_start_address (const bfd *abfd)
.{
.  return abfd->start_address;
.}
.
.static inline unsigned int
.bfd_get_symcount (const bfd *abfd)
.{
.  return abfd->symcount;
.}
.
.static inline unsigned int
.bfd_get_dynamic_symcount (const bfd *abfd)
.{
.  return abfd->dynsymcount;
.}
.
.static inline struct bfd_symbol **
.bfd_get_outsymbols (const bfd *abfd)
.{
.  return abfd->outsymbols;
.}
.
.static inline unsigned int
.bfd_count_sections (const bfd *abfd)
.{
.  return abfd->section_count;
.}
.
.static inline bool
.bfd_has_map (const bfd *abfd)
.{
.  return abfd->has_armap;
.}
.
.static inline bool
.bfd_is_thin_archive (const bfd *abfd)
.{
.  return abfd->is_thin_archive;
.}
.
.static inline void *
.bfd_usrdata (const bfd *abfd)
.{
.  return abfd->usrdata;
.}
.
.{* See note beside bfd_set_section_userdata.  *}
.static inline bool
.bfd_set_cacheable (bfd * abfd, bool val)
.{
.  abfd->cacheable = val;
.  return true;
.}
.
.static inline void
.bfd_set_thin_archive (bfd *abfd, bool val)
.{
.  abfd->is_thin_archive = val;
.}
.
.static inline void
.bfd_set_usrdata (bfd *abfd, void *val)
.{
.  abfd->usrdata = val;
.}
.
.static inline asection *
.bfd_asymbol_section (const asymbol *sy)
.{
.  return sy->section;
.}
.
.static inline bfd_vma
.bfd_asymbol_value (const asymbol *sy)
.{
.  return sy->section->vma + sy->value;
.}
.
.static inline const char *
.bfd_asymbol_name (const asymbol *sy)
.{
.  return sy->name;
.}
.
.static inline struct bfd *
.bfd_asymbol_bfd (const asymbol *sy)
.{
.  return sy->the_bfd;
.}
.
.static inline void
.bfd_set_asymbol_name (asymbol *sy, const char *name)
.{
.  sy->name = name;
.}
.
.{* For input sections return the original size on disk of the
.   section.  For output sections return the current size.  *}
.static inline bfd_size_type
.bfd_get_section_limit_octets (const bfd *abfd, const asection *sec)
.{
.  if (abfd->direction != write_direction && sec->rawsize != 0)
.    return sec->rawsize;
.  return sec->size;
.}
.
.{* Find the address one past the end of SEC.  *}
.static inline bfd_size_type
.bfd_get_section_limit (const bfd *abfd, const asection *sec)
.{
.  return (bfd_get_section_limit_octets (abfd, sec)
.	   / bfd_octets_per_byte (abfd, sec));
.}
.
.{* For input sections return the larger of the current size and the
.   original size on disk of the section.  For output sections return
.   the current size.  *}
.static inline bfd_size_type
.bfd_get_section_alloc_size (const bfd *abfd, const asection *sec)
.{
.  if (abfd->direction != write_direction && sec->rawsize > sec->size)
.    return sec->rawsize;
.  return sec->size;
.}
.
.{* Functions to handle insertion and deletion of a bfd's sections.  These
.   only handle the list pointers, ie. do not adjust section_count,
.   target_index etc.  *}
.static inline void
.bfd_section_list_remove (bfd *abfd, asection *s)
.{
.  asection *next = s->next;
.  asection *prev = s->prev;
.  if (prev)
.    prev->next = next;
.  else
.    abfd->sections = next;
.  if (next)
.    next->prev = prev;
.  else
.    abfd->section_last = prev;
.}
.
.static inline void
.bfd_section_list_append (bfd *abfd, asection *s)
.{
.  s->next = 0;
.  if (abfd->section_last)
.    {
.      s->prev = abfd->section_last;
.      abfd->section_last->next = s;
.    }
.  else
.    {
.      s->prev = 0;
.      abfd->sections = s;
.    }
.  abfd->section_last = s;
.}
.
.static inline void
.bfd_section_list_prepend (bfd *abfd, asection *s)
.{
.  s->prev = 0;
.  if (abfd->sections)
.    {
.      s->next = abfd->sections;
.      abfd->sections->prev = s;
.    }
.  else
.    {
.      s->next = 0;
.      abfd->section_last = s;
.    }
.  abfd->sections = s;
.}
.
.static inline void
.bfd_section_list_insert_after (bfd *abfd, asection *a, asection *s)
.{
.  asection *next = a->next;
.  s->next = next;
.  s->prev = a;
.  a->next = s;
.  if (next)
.    next->prev = s;
.  else
.    abfd->section_last = s;
.}
.
.static inline void
.bfd_section_list_insert_before (bfd *abfd, asection *b, asection *s)
.{
.  asection *prev = b->prev;
.  s->prev = prev;
.  s->next = b;
.  b->prev = s;
.  if (prev)
.    prev->next = s;
.  else
.    abfd->sections = s;
.}
.
.static inline bool
.bfd_section_removed_from_list (const bfd *abfd, const asection *s)
.{
.  return s->next ? s->next->prev != s : abfd->section_last != s;
.}
.
*/

#include "sysdep.h"
#include <stdarg.h>
#include "bfd.h"
#include "bfdver.h"
#include "libiberty.h"
#include "demangle.h"
#include "safe-ctype.h"
#include "bfdlink.h"
#include "libbfd.h"
#include "coff/internal.h"
#include "coff/sym.h"
#include "libcoff.h"
#include "libecoff.h"
#undef obj_symbols
#include "elf-bfd.h"

#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif


/* provide storage for subsystem, stack and heap data which may have been
   passed in on the command line.  Ld puts this data into a bfd_link_info
   struct which ultimately gets passed in to the bfd.  When it arrives, copy
   it to the following struct so that the data will be available in coffcode.h
   where it is needed.  The typedef's used are defined in bfd.h */

/*
INODE
Error reporting, Initialization, typedef bfd, BFD front end

SECTION
	Error reporting

	Most BFD functions return nonzero on success (check their
	individual documentation for precise semantics).  On an error,
	they call <<bfd_set_error>> to set an error condition that callers
	can check by calling <<bfd_get_error>>.
	If that returns <<bfd_error_system_call>>, then check
	<<errno>>.

	The easiest way to report a BFD error to the user is to
	use <<bfd_perror>>.

SUBSECTION
	Type <<bfd_error_type>>

	The values returned by <<bfd_get_error>> are defined by the
	enumerated type <<bfd_error_type>>.

CODE_FRAGMENT
.typedef enum bfd_error
.{
.  bfd_error_no_error = 0,
.  bfd_error_system_call,
.  bfd_error_invalid_target,
.  bfd_error_wrong_format,
.  bfd_error_wrong_object_format,
.  bfd_error_invalid_operation,
.  bfd_error_no_memory,
.  bfd_error_no_symbols,
.  bfd_error_no_armap,
.  bfd_error_no_more_archived_files,
.  bfd_error_malformed_archive,
.  bfd_error_missing_dso,
.  bfd_error_file_not_recognized,
.  bfd_error_file_ambiguously_recognized,
.  bfd_error_no_contents,
.  bfd_error_nonrepresentable_section,
.  bfd_error_no_debug_section,
.  bfd_error_bad_value,
.  bfd_error_file_truncated,
.  bfd_error_file_too_big,
.  bfd_error_sorry,
.  bfd_error_on_input,
.  bfd_error_invalid_error_code
.}
.bfd_error_type;
.
INTERNAL
.{* A buffer that is freed on bfd_close.  *}
.extern char *_bfd_error_buf;
.
*/

static bfd_error_type bfd_error;
static bfd_error_type input_error;
static bfd *input_bfd;
char *_bfd_error_buf;

const char *const bfd_errmsgs[] =
{
  N_("no error"),
  N_("system call error"),
  N_("invalid bfd target"),
  N_("file in wrong format"),
  N_("archive object file in wrong format"),
  N_("invalid operation"),
  N_("memory exhausted"),
  N_("no symbols"),
  N_("archive has no index; run ranlib to add one"),
  N_("no more archived files"),
  N_("malformed archive"),
  N_("DSO missing from command line"),
  N_("file format not recognized"),
  N_("file format is ambiguous"),
  N_("section has no contents"),
  N_("nonrepresentable section on output"),
  N_("symbol needs debug section which does not exist"),
  N_("bad value"),
  N_("file truncated"),
  N_("file too big"),
  N_("sorry, cannot handle this file"),
  N_("error reading %s: %s"),
  N_("#<invalid error code>")
};

/*
FUNCTION
	bfd_get_error

SYNOPSIS
	bfd_error_type bfd_get_error (void);

DESCRIPTION
	Return the current BFD error condition.
*/

bfd_error_type
bfd_get_error (void)
{
  return bfd_error;
}

/*
FUNCTION
	bfd_set_error

SYNOPSIS
	void bfd_set_error (bfd_error_type error_tag);

DESCRIPTION
	Set the BFD error condition to be @var{error_tag}.

	@var{error_tag} must not be bfd_error_on_input.  Use
	bfd_set_input_error for input errors instead.
*/

void
bfd_set_error (bfd_error_type error_tag)
{
  bfd_error = error_tag;
  if (bfd_error >= bfd_error_on_input)
    abort ();
}

/*
FUNCTION
	bfd_set_input_error

SYNOPSIS
	void bfd_set_input_error (bfd *input, bfd_error_type error_tag);

DESCRIPTION

	Set the BFD error condition to be bfd_error_on_input.
	@var{input} is the input bfd where the error occurred, and
	@var{error_tag} the bfd_error_type error.
*/

void
bfd_set_input_error (bfd *input, bfd_error_type error_tag)
{
  /* This is an error that occurred during bfd_close when writing an
     archive, but on one of the input files.  */
  bfd_error = bfd_error_on_input;
  free (_bfd_error_buf);
  _bfd_error_buf = NULL;
  input_bfd = input;
  input_error = error_tag;
  if (input_error >= bfd_error_on_input)
    abort ();
}

/*
FUNCTION
	bfd_errmsg

SYNOPSIS
	const char *bfd_errmsg (bfd_error_type error_tag);

DESCRIPTION
	Return a string describing the error @var{error_tag}, or
	the system error if @var{error_tag} is <<bfd_error_system_call>>.
*/

const char *
bfd_errmsg (bfd_error_type error_tag)
{
#ifndef errno
  extern int errno;
#endif
  if (error_tag == bfd_error_on_input)
    {
      const char *msg = bfd_errmsg (input_error);
      char *ret = bfd_asprintf (_(bfd_errmsgs[error_tag]),
				bfd_get_filename (input_bfd), msg);
      if (ret)
	return ret;

      /* Ick, what to do on out of memory?  */
      return msg;
    }

  if (error_tag == bfd_error_system_call)
    return xstrerror (errno);

  if (error_tag > bfd_error_invalid_error_code)
    error_tag = bfd_error_invalid_error_code;	/* sanity check */

  return _(bfd_errmsgs[error_tag]);
}

/*
FUNCTION
	bfd_perror

SYNOPSIS
	void bfd_perror (const char *message);

DESCRIPTION
	Print to the standard error stream a string describing the
	last BFD error that occurred, or the last system error if
	the last BFD error was a system call failure.  If @var{message}
	is non-NULL and non-empty, the error string printed is preceded
	by @var{message}, a colon, and a space.  It is followed by a newline.
*/

void
bfd_perror (const char *message)
{
  fflush (stdout);
  if (message == NULL || *message == '\0')
    fprintf (stderr, "%s\n", bfd_errmsg (bfd_get_error ()));
  else
    fprintf (stderr, "%s: %s\n", message, bfd_errmsg (bfd_get_error ()));
  fflush (stderr);
}

/*
INTERNAL_FUNCTION
	bfd_asprintf

SYNOPSIS
	char *bfd_asprintf (const char *fmt, ...);

DESCRIPTION
	Primarily for error reporting, this function is like
	libiberty's xasprintf except that it can return NULL on no
	memory and the returned string should not be freed.  Uses a
	single malloc'd buffer managed by libbfd, _bfd_error_buf.
	Be aware that a call to this function frees the result of any
	previous call.  bfd_errmsg (bfd_error_on_input) also calls
	this function.
*/

char *
bfd_asprintf (const char *fmt, ...)
{
  free (_bfd_error_buf);
  _bfd_error_buf = NULL;
  va_list ap;
  va_start (ap, fmt);
  int count = vasprintf (&_bfd_error_buf, fmt, ap);
  va_end (ap);
  if (count == -1)
    {
      bfd_set_error (bfd_error_no_memory);
      _bfd_error_buf = NULL;
    }
  return _bfd_error_buf;
}

/*
SUBSECTION
	BFD error handler

	Some BFD functions want to print messages describing the
	problem.  They call a BFD error handler function.  This
	function may be overridden by the program.

	The BFD error handler acts like vprintf.

CODE_FRAGMENT
.typedef void (*bfd_error_handler_type) (const char *, va_list);
.
*/

/* The program name used when printing BFD error messages.  */

static const char *_bfd_error_program_name;

/* Support for positional parameters.  */

union _bfd_doprnt_args
{
  int i;
  long l;
  long long ll;
  double d;
  long double ld;
  void *p;
  enum
  {
    Bad,
    Int,
    Long,
    LongLong,
    Double,
    LongDouble,
    Ptr
  } type;
};

/* Maximum number of _bfd_error_handler args.  Don't increase this
   without changing the code handling positional parameters.  */
#define MAX_ARGS 9

/* This macro and _bfd_doprnt taken from libiberty _doprnt.c, tidied a
   little and extended to handle '%pA', '%pB' and positional parameters.  */

#define PRINT_TYPE(TYPE, FIELD) \
  do								\
    {								\
      TYPE value = (TYPE) args[arg_no].FIELD;			\
      result = print (stream, specifier, value);		\
    } while (0)

typedef int (*print_func) (void *, const char *, ...);

static int
_bfd_doprnt (print_func print, void *stream, const char *format,
	     union _bfd_doprnt_args *args)
{
  const char *ptr = format;
  char specifier[128];
  int total_printed = 0;
  unsigned int arg_count = 0;

  while (*ptr != '\0')
    {
      int result;

      if (*ptr != '%')
	{
	  /* While we have regular characters, print them.  */
	  char *end = strchr (ptr, '%');
	  if (end != NULL)
	    result = print (stream, "%.*s", (int) (end - ptr), ptr);
	  else
	    result = print (stream, "%s", ptr);
	  ptr += result;
	}
      else if (ptr[1] == '%')
	{
	  fputc ('%', stream);
	  result = 1;
	  ptr += 2;
	}
      else
	{
	  /* We have a format specifier!  */
	  char *sptr = specifier;
	  int wide_width = 0, short_width = 0;
	  unsigned int arg_no;

	  /* Copy the % and move forward.  */
	  *sptr++ = *ptr++;

	  /* Check for a positional parameter.  */
	  arg_no = -1u;
	  if (*ptr != '0' && ISDIGIT (*ptr) && ptr[1] == '$')
	    {
	      arg_no = *ptr - '1';
	      ptr += 2;
	    }

	  /* Move past flags.  */
	  while (strchr ("-+ #0'I", *ptr))
	    *sptr++ = *ptr++;

	  if (*ptr == '*')
	    {
	      int value;
	      unsigned int arg_index;

	      ptr++;
	      arg_index = arg_count;
	      if (*ptr != '0' && ISDIGIT (*ptr) && ptr[1] == '$')
		{
		  arg_index = *ptr - '1';
		  ptr += 2;
		}
	      value = abs (args[arg_index].i);
	      arg_count++;
	      sptr += sprintf (sptr, "%d", value);
	    }
	  else
	    /* Handle explicit numeric value.  */
	    while (ISDIGIT (*ptr))
	      *sptr++ = *ptr++;

	  /* Precision.  */
	  if (*ptr == '.')
	    {
	      /* Copy and go past the period.  */
	      *sptr++ = *ptr++;
	      if (*ptr == '*')
		{
		  int value;
		  unsigned int arg_index;

		  ptr++;
		  arg_index = arg_count;
		  if (*ptr != '0' && ISDIGIT (*ptr) && ptr[1] == '$')
		    {
		      arg_index = *ptr - '1';
		      ptr += 2;
		    }
		  value = abs (args[arg_index].i);
		  arg_count++;
		  sptr += sprintf (sptr, "%d", value);
		}
	      else
		/* Handle explicit numeric value.  */
		while (ISDIGIT (*ptr))
		  *sptr++ = *ptr++;
	    }
	  while (strchr ("hlL", *ptr))
	    {
	      switch (*ptr)
		{
		case 'h':
		  short_width = 1;
		  break;
		case 'l':
		  wide_width++;
		  break;
		case 'L':
		  wide_width = 2;
		  break;
		default:
		  abort();
		}
	      *sptr++ = *ptr++;
	    }

	  /* Copy the type specifier, and NULL terminate.  */
	  *sptr++ = *ptr++;
	  *sptr = '\0';
	  if ((int) arg_no < 0)
	    arg_no = arg_count;

	  switch (ptr[-1])
	    {
	    case 'd':
	    case 'i':
	    case 'o':
	    case 'u':
	    case 'x':
	    case 'X':
	    case 'c':
	      {
		/* Short values are promoted to int, so just copy it
		   as an int and trust the C library printf to cast it
		   to the right width.  */
		if (short_width)
		  PRINT_TYPE (int, i);
		else
		  {
		    switch (wide_width)
		      {
		      case 0:
			PRINT_TYPE (int, i);
			break;
		      case 1:
			PRINT_TYPE (long, l);
			break;
		      case 2:
		      default:
#if defined (__MSVCRT__)
			sptr[-3] = 'I';
			sptr[-2] = '6';
			sptr[-1] = '4';
			*sptr++ = ptr[-1];
			*sptr = '\0';
#endif
			PRINT_TYPE (long long, ll);
			break;
		      }
		  }
	      }
	      break;
	    case 'f':
	    case 'e':
	    case 'E':
	    case 'g':
	    case 'G':
	      {
		if (wide_width == 0)
		  PRINT_TYPE (double, d);
		else
		  PRINT_TYPE (long double, ld);
	      }
	      break;
	    case 's':
	      PRINT_TYPE (char *, p);
	      break;
	    case 'p':
	      if (*ptr == 'A')
		{
		  asection *sec;
		  bfd *abfd;
		  const char *group = NULL;
		  struct coff_comdat_info *ci;

		  ptr++;
		  sec = (asection *) args[arg_no].p;
		  if (sec == NULL)
		    /* Invoking %pA with a null section pointer is an
		       internal error.  */
		    abort ();
		  abfd = sec->owner;
		  if (abfd != NULL
		      && bfd_get_flavour (abfd) == bfd_target_elf_flavour
		      && elf_next_in_group (sec) != NULL
		      && (sec->flags & SEC_GROUP) == 0)
		    group = elf_group_name (sec);
		  else if (abfd != NULL
			   && bfd_get_flavour (abfd) == bfd_target_coff_flavour
			   && (ci = bfd_coff_get_comdat_section (sec->owner,
								 sec)) != NULL)
		    group = ci->name;
		  if (group != NULL)
		    result = print (stream, "%s[%s]", sec->name, group);
		  else
		    result = print (stream, "%s", sec->name);
		}
	      else if (*ptr == 'B')
		{
		  bfd *abfd;

		  ptr++;
		  abfd = (bfd *) args[arg_no].p;
		  if (abfd == NULL)
		    /* Invoking %pB with a null bfd pointer is an
		       internal error.  */
		    abort ();
		  else if (abfd->my_archive
			   && !bfd_is_thin_archive (abfd->my_archive))
		    result = print (stream, "%s(%s)",
				    bfd_get_filename (abfd->my_archive),
				    bfd_get_filename (abfd));
		  else
		    result = print (stream, "%s", bfd_get_filename (abfd));
		}
	      else
		PRINT_TYPE (void *, p);
	      break;
	    default:
	      abort();
	    }
	  arg_count++;
	}
      if (result == -1)
	return -1;
      total_printed += result;
    }

  return total_printed;
}

/* First pass over FORMAT to gather ARGS.  Returns number of args.  */

static unsigned int
_bfd_doprnt_scan (const char *format, va_list ap, union _bfd_doprnt_args *args)
{
  const char *ptr = format;
  unsigned int arg_count = 0;

  for (unsigned int i = 0; i < MAX_ARGS; i++)
    args[i].type = Bad;

  while (*ptr != '\0')
    {
      if (*ptr != '%')
	{
	  ptr = strchr (ptr, '%');
	  if (ptr == NULL)
	    break;
	}
      else if (ptr[1] == '%')
	ptr += 2;
      else
	{
	  int wide_width = 0, short_width = 0;
	  unsigned int arg_no;
	  int arg_type;

	  ptr++;

	  /* Check for a positional parameter.  */
	  arg_no = -1u;
	  if (*ptr != '0' && ISDIGIT (*ptr) && ptr[1] == '$')
	    {
	      arg_no = *ptr - '1';
	      ptr += 2;
	    }

	  /* Move past flags.  */
	  while (strchr ("-+ #0'I", *ptr))
	    ptr++;

	  if (*ptr == '*')
	    {
	      unsigned int arg_index;

	      ptr++;
	      arg_index = arg_count;
	      if (*ptr != '0' && ISDIGIT (*ptr) && ptr[1] == '$')
		{
		  arg_index = *ptr - '1';
		  ptr += 2;
		}
	      if (arg_index >= MAX_ARGS)
		abort ();
	      args[arg_index].type = Int;
	      arg_count++;
	    }
	  else
	    /* Handle explicit numeric value.  */
	    while (ISDIGIT (*ptr))
	      ptr++;

	  /* Precision.  */
	  if (*ptr == '.')
	    {
	      ptr++;
	      if (*ptr == '*')
		{
		  unsigned int arg_index;

		  ptr++;
		  arg_index = arg_count;
		  if (*ptr != '0' && ISDIGIT (*ptr) && ptr[1] == '$')
		    {
		      arg_index = *ptr - '1';
		      ptr += 2;
		    }
		  if (arg_index >= MAX_ARGS)
		    abort ();
		  args[arg_index].type = Int;
		  arg_count++;
		}
	      else
		/* Handle explicit numeric value.  */
		while (ISDIGIT (*ptr))
		  ptr++;
	    }
	  while (strchr ("hlL", *ptr))
	    {
	      switch (*ptr)
		{
		case 'h':
		  short_width = 1;
		  break;
		case 'l':
		  wide_width++;
		  break;
		case 'L':
		  wide_width = 2;
		  break;
		default:
		  abort();
		}
	      ptr++;
	    }

	  ptr++;
	  if ((int) arg_no < 0)
	    arg_no = arg_count;

	  arg_type = Bad;
	  switch (ptr[-1])
	    {
	    case 'd':
	    case 'i':
	    case 'o':
	    case 'u':
	    case 'x':
	    case 'X':
	    case 'c':
	      {
		if (short_width)
		  arg_type = Int;
		else
		  {
		    switch (wide_width)
		      {
		      case 0:
			arg_type = Int;
			break;
		      case 1:
			arg_type = Long;
			break;
		      case 2:
		      default:
			arg_type = LongLong;
			break;
		      }
		  }
	      }
	      break;
	    case 'f':
	    case 'e':
	    case 'E':
	    case 'g':
	    case 'G':
	      {
		if (wide_width == 0)
		  arg_type = Double;
		else
		  arg_type = LongDouble;
	      }
	      break;
	    case 's':
	      arg_type = Ptr;
	      break;
	    case 'p':
	      if (*ptr == 'A' || *ptr == 'B')
		ptr++;
	      arg_type = Ptr;
	      break;
	    default:
	      abort();
	    }

	  if (arg_no >= MAX_ARGS)
	    abort ();
	  args[arg_no].type = arg_type;
	  arg_count++;
	}
    }

  for (unsigned int i = 0; i < arg_count; i++)
    {
      switch (args[i].type)
	{
	case Int:
	  args[i].i = va_arg (ap, int);
	  break;
	case Long:
	  args[i].l = va_arg (ap, long);
	  break;
	case LongLong:
	  args[i].ll = va_arg (ap, long long);
	  break;
	case Double:
	  args[i].d = va_arg (ap, double);
	  break;
	case LongDouble:
	  args[i].ld = va_arg (ap, long double);
	  break;
	case Ptr:
	  args[i].p = va_arg (ap, void *);
	  break;
	default:
	  abort ();
	}
    }

  return arg_count;
}

/* The standard error handler that prints to stderr.  */

static void
error_handler_fprintf (const char *fmt, va_list ap)
{
  union _bfd_doprnt_args args[MAX_ARGS];

  _bfd_doprnt_scan (fmt, ap, args);

  /* PR 4992: Don't interrupt output being sent to stdout.  */
  fflush (stdout);

  fprintf (stderr, "%s: ", _bfd_get_error_program_name ());

  _bfd_doprnt ((print_func) fprintf, stderr, fmt, args);

  /* On AIX, putc is implemented as a macro that triggers a -Wunused-value
     warning, so use the fputc function to avoid it.  */
  fputc ('\n', stderr);
  fflush (stderr);
}

/* Control printing to a string buffer.  */
struct buf_stream
{
  char *ptr;
  int left;
};

/* An fprintf like function that instead prints to a string buffer.  */

static int
err_sprintf (void *stream, const char *fmt, ...)
{
  struct buf_stream *s = stream;
  va_list ap;

  va_start (ap, fmt);
  int total = vsnprintf (s->ptr, s->left, fmt, ap);
  va_end (ap);
  if (total < 0)
    ;
  else if (total > s->left)
    {
      s->ptr += s->left;
      s->left = 0;
    }
  else
    {
      s->ptr += total;
      s->left -= total;
    }
  return total;
}

/* Communicate the bfd processed by bfd_check_format_matches to the
   error handling function error_handler_sprintf.  */

static bfd *error_handler_bfd;

/* An error handler that prints to a string, then dups that string to
   a per-xvec cache.  */

static void
error_handler_sprintf (const char *fmt, va_list ap)
{
  union _bfd_doprnt_args args[MAX_ARGS];
  char error_buf[1024];
  struct buf_stream error_stream;

  _bfd_doprnt_scan (fmt, ap, args);

  error_stream.ptr = error_buf;
  error_stream.left = sizeof (error_buf);
  _bfd_doprnt (err_sprintf, &error_stream, fmt, args);

  size_t len = error_stream.ptr - error_buf;
  struct per_xvec_message **warn
    = _bfd_per_xvec_warn (error_handler_bfd->xvec, len + 1);
  if (*warn)
    {
      memcpy ((*warn)->message, error_buf, len);
      (*warn)->message[len] = 0;
    }
}

/* This is a function pointer to the routine which should handle BFD
   error messages.  It is called when a BFD routine encounters an
   error for which it wants to print a message.  Going through a
   function pointer permits a program linked against BFD to intercept
   the messages and deal with them itself.  */

static bfd_error_handler_type _bfd_error_internal = error_handler_fprintf;

/*
FUNCTION
	_bfd_error_handler

SYNOPSIS
	void _bfd_error_handler (const char *fmt, ...) ATTRIBUTE_PRINTF_1;

DESCRIPTION
	This is the default routine to handle BFD error messages.
	Like fprintf (stderr, ...), but also handles some extra format
	specifiers.

	%pA section name from section.  For group components, prints
	group name too.
	%pB file name from bfd.  For archive components, prints
	archive too.

	Beware: Only supports a maximum of 9 format arguments.
*/

void
_bfd_error_handler (const char *fmt, ...)
{
  va_list ap;

  va_start (ap, fmt);
  _bfd_error_internal (fmt, ap);
  va_end (ap);
}

/*
FUNCTION
	bfd_set_error_handler

SYNOPSIS
	bfd_error_handler_type bfd_set_error_handler (bfd_error_handler_type);

DESCRIPTION
	Set the BFD error handler function.  Returns the previous
	function.
*/

bfd_error_handler_type
bfd_set_error_handler (bfd_error_handler_type pnew)
{
  bfd_error_handler_type pold;

  pold = _bfd_error_internal;
  _bfd_error_internal = pnew;
  return pold;
}

/*
INTERNAL_FUNCTION
	_bfd_set_error_handler_caching

SYNOPSIS
	bfd_error_handler_type _bfd_set_error_handler_caching (bfd *);

DESCRIPTION
	Set the BFD error handler function to one that stores messages
	to the per_xvec_warn array.  Returns the previous function.
*/

bfd_error_handler_type
_bfd_set_error_handler_caching (bfd *abfd)
{
  error_handler_bfd = abfd;
  return bfd_set_error_handler (error_handler_sprintf);
}

/*
FUNCTION
	bfd_set_error_program_name

SYNOPSIS
	void bfd_set_error_program_name (const char *);

DESCRIPTION
	Set the program name to use when printing a BFD error.  This
	is printed before the error message followed by a colon and
	space.  The string must not be changed after it is passed to
	this function.
*/

void
bfd_set_error_program_name (const char *name)
{
  _bfd_error_program_name = name;
}

/*
INTERNAL_FUNCTION
	_bfd_get_error_program_name

SYNOPSIS
	const char *_bfd_get_error_program_name (void);

DESCRIPTION
	Get the program name used when printing a BFD error.
*/

const char *
_bfd_get_error_program_name (void)
{
  if (_bfd_error_program_name != NULL)
    return _bfd_error_program_name;
  return "BFD";
}

/*
SUBSECTION
	BFD assert handler

	If BFD finds an internal inconsistency, the bfd assert
	handler is called with information on the BFD version, BFD
	source file and line.  If this happens, most programs linked
	against BFD are expected to want to exit with an error, or mark
	the current BFD operation as failed, so it is recommended to
	override the default handler, which just calls
	_bfd_error_handler and continues.

CODE_FRAGMENT
.typedef void (*bfd_assert_handler_type) (const char *bfd_formatmsg,
.					  const char *bfd_version,
.					  const char *bfd_file,
.					  int bfd_line);
.
*/

/* Note the use of bfd_ prefix on the parameter names above: we want to
   show which one is the message and which is the version by naming the
   parameters, but avoid polluting the program-using-bfd namespace as
   the typedef is visible in the exported headers that the program
   includes.  Below, it's just for consistency.  */

static void
_bfd_default_assert_handler (const char *bfd_formatmsg,
			     const char *bfd_version,
			     const char *bfd_file,
			     int bfd_line)

{
  _bfd_error_handler (bfd_formatmsg, bfd_version, bfd_file, bfd_line);
}

/* Similar to _bfd_error_handler, a program can decide to exit on an
   internal BFD error.  We use a non-variadic type to simplify passing
   on parameters to other functions, e.g. _bfd_error_handler.  */

static bfd_assert_handler_type _bfd_assert_handler = _bfd_default_assert_handler;

/*
FUNCTION
	bfd_set_assert_handler

SYNOPSIS
	bfd_assert_handler_type bfd_set_assert_handler (bfd_assert_handler_type);

DESCRIPTION
	Set the BFD assert handler function.  Returns the previous
	function.
*/

bfd_assert_handler_type
bfd_set_assert_handler (bfd_assert_handler_type pnew)
{
  bfd_assert_handler_type pold;

  pold = _bfd_assert_handler;
  _bfd_assert_handler = pnew;
  return pold;
}

/*
INODE
Initialization, Miscellaneous, Error reporting, BFD front end

FUNCTION
	bfd_init

SYNOPSIS
	unsigned int bfd_init (void);

DESCRIPTION
	This routine must be called before any other BFD function to
	initialize magical internal data structures.
	Returns a magic number, which may be used to check
	that the bfd library is configured as expected by users.

.{* Value returned by bfd_init.  *}
.#define BFD_INIT_MAGIC (sizeof (struct bfd_section))
.
*/

unsigned int
bfd_init (void)
{
  bfd_error = bfd_error_no_error;
  input_bfd = NULL;
  free (_bfd_error_buf);
  _bfd_error_buf = NULL;
  input_error = bfd_error_no_error;
  _bfd_error_program_name = NULL;
  _bfd_error_internal = error_handler_fprintf;
  _bfd_assert_handler = _bfd_default_assert_handler;

  return BFD_INIT_MAGIC;
}

/*
INODE
Miscellaneous, Memory Usage, Initialization, BFD front end

SECTION
	Miscellaneous

SUBSECTION
	Miscellaneous functions
*/

/*
FUNCTION
	bfd_get_reloc_upper_bound

SYNOPSIS
	long bfd_get_reloc_upper_bound (bfd *abfd, asection *sect);

DESCRIPTION
	Return the number of bytes required to store the
	relocation information associated with section @var{sect}
	attached to bfd @var{abfd}.  If an error occurs, return -1.

*/

long
bfd_get_reloc_upper_bound (bfd *abfd, sec_ptr asect)
{
  if (abfd->format != bfd_object)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return -1;
    }

  return BFD_SEND (abfd, _get_reloc_upper_bound, (abfd, asect));
}

/*
FUNCTION
	bfd_canonicalize_reloc

SYNOPSIS
	long bfd_canonicalize_reloc
	  (bfd *abfd, asection *sec, arelent **loc, asymbol **syms);

DESCRIPTION
	Call the back end associated with the open BFD
	@var{abfd} and translate the external form of the relocation
	information attached to @var{sec} into the internal canonical
	form.  Place the table into memory at @var{loc}, which has
	been preallocated, usually by a call to
	<<bfd_get_reloc_upper_bound>>.  Returns the number of relocs, or
	-1 on error.

	The @var{syms} table is also needed for horrible internal magic
	reasons.

*/
long
bfd_canonicalize_reloc (bfd *abfd,
			sec_ptr asect,
			arelent **location,
			asymbol **symbols)
{
  if (abfd->format != bfd_object)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return -1;
    }

  return BFD_SEND (abfd, _bfd_canonicalize_reloc,
		   (abfd, asect, location, symbols));
}

/*
FUNCTION
	bfd_set_reloc

SYNOPSIS
	void bfd_set_reloc
	  (bfd *abfd, asection *sec, arelent **rel, unsigned int count);

DESCRIPTION
	Set the relocation pointer and count within
	section @var{sec} to the values @var{rel} and @var{count}.
	The argument @var{abfd} is ignored.

.#define bfd_set_reloc(abfd, asect, location, count) \
.	BFD_SEND (abfd, _bfd_set_reloc, (abfd, asect, location, count))
*/

/*
FUNCTION
	bfd_set_file_flags

SYNOPSIS
	bool bfd_set_file_flags (bfd *abfd, flagword flags);

DESCRIPTION
	Set the flag word in the BFD @var{abfd} to the value @var{flags}.

	Possible errors are:
	o <<bfd_error_wrong_format>> - The target bfd was not of object format.
	o <<bfd_error_invalid_operation>> - The target bfd was open for reading.
	o <<bfd_error_invalid_operation>> -
	The flag word contained a bit which was not applicable to the
	type of file.  E.g., an attempt was made to set the <<D_PAGED>> bit
	on a BFD format which does not support demand paging.

*/

bool
bfd_set_file_flags (bfd *abfd, flagword flags)
{
  if (abfd->format != bfd_object)
    {
      bfd_set_error (bfd_error_wrong_format);
      return false;
    }

  if (bfd_read_p (abfd))
    {
      bfd_set_error (bfd_error_invalid_operation);
      return false;
    }

  abfd->flags = flags;
  if ((flags & bfd_applicable_file_flags (abfd)) != flags)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return false;
    }

  return true;
}

void
bfd_assert (const char *file, int line)
{
  /* xgettext:c-format */
  (*_bfd_assert_handler) (_("BFD %s assertion fail %s:%d"),
			  BFD_VERSION_STRING, file, line);
}

/* A more or less friendly abort message.  In libbfd.h abort is
   defined to call this function.  */

void
_bfd_abort (const char *file, int line, const char *fn)
{
  if (fn != NULL)
    _bfd_error_handler
      /* xgettext:c-format */
      (_("BFD %s internal error, aborting at %s:%d in %s\n"),
       BFD_VERSION_STRING, file, line, fn);
  else
    _bfd_error_handler
      /* xgettext:c-format */
      (_("BFD %s internal error, aborting at %s:%d\n"),
       BFD_VERSION_STRING, file, line);
  _bfd_error_handler (_("Please report this bug.\n"));
  _exit (EXIT_FAILURE);
}

/*
FUNCTION
	bfd_get_arch_size

SYNOPSIS
	int bfd_get_arch_size (bfd *abfd);

DESCRIPTION
	Returns the normalized architecture address size, in bits, as
	determined by the object file's format.  By normalized, we mean
	either 32 or 64.  For ELF, this information is included in the
	header.  Use bfd_arch_bits_per_address for number of bits in
	the architecture address.

	Returns the arch size in bits if known, <<-1>> otherwise.
*/

int
bfd_get_arch_size (bfd *abfd)
{
  if (abfd->xvec->flavour == bfd_target_elf_flavour)
    return get_elf_backend_data (abfd)->s->arch_size;

  return bfd_arch_bits_per_address (abfd) > 32 ? 64 : 32;
}

/*
FUNCTION
	bfd_get_sign_extend_vma

SYNOPSIS
	int bfd_get_sign_extend_vma (bfd *abfd);

DESCRIPTION
	Indicates if the target architecture "naturally" sign extends
	an address.  Some architectures implicitly sign extend address
	values when they are converted to types larger than the size
	of an address.  For instance, bfd_get_start_address() will
	return an address sign extended to fill a bfd_vma when this is
	the case.

	Returns <<1>> if the target architecture is known to sign
	extend addresses, <<0>> if the target architecture is known to
	not sign extend addresses, and <<-1>> otherwise.
*/

int
bfd_get_sign_extend_vma (bfd *abfd)
{
  const char *name;

  if (bfd_get_flavour (abfd) == bfd_target_elf_flavour)
    return get_elf_backend_data (abfd)->sign_extend_vma;

  name = bfd_get_target (abfd);

  /* Return a proper value for DJGPP & PE COFF.
     This function is required for DWARF2 support, but there is
     no place to store this information in the COFF back end.
     Should enough other COFF targets add support for DWARF2,
     a place will have to be found.  Until then, this hack will do.  */
  if (startswith (name, "coff-go32")
      || strcmp (name, "pe-i386") == 0
      || strcmp (name, "pei-i386") == 0
      || strcmp (name, "pe-x86-64") == 0
      || strcmp (name, "pei-x86-64") == 0
      || strcmp (name, "pe-aarch64-little") == 0
      || strcmp (name, "pei-aarch64-little") == 0
      || strcmp (name, "pe-arm-wince-little") == 0
      || strcmp (name, "pei-arm-wince-little") == 0
      || strcmp (name, "pei-loongarch64") == 0
      || strcmp (name, "aixcoff-rs6000") == 0
      || strcmp (name, "aix5coff64-rs6000") == 0)
    return 1;

  if (startswith (name, "mach-o"))
    return 0;

  bfd_set_error (bfd_error_wrong_format);
  return -1;
}

/*
FUNCTION
	bfd_set_start_address

SYNOPSIS
	bool bfd_set_start_address (bfd *abfd, bfd_vma vma);

DESCRIPTION
	Make @var{vma} the entry point of output BFD @var{abfd}.

	Returns <<TRUE>> on success, <<FALSE>> otherwise.
*/

bool
bfd_set_start_address (bfd *abfd, bfd_vma vma)
{
  abfd->start_address = vma;
  return true;
}

/*
FUNCTION
	bfd_get_gp_size

SYNOPSIS
	unsigned int bfd_get_gp_size (bfd *abfd);

DESCRIPTION
	Return the maximum size of objects to be optimized using the GP
	register under MIPS ECOFF.  This is typically set by the <<-G>>
	argument to the compiler, assembler or linker.
*/

unsigned int
bfd_get_gp_size (bfd *abfd)
{
  if (abfd->format == bfd_object)
    {
      if (abfd->xvec->flavour == bfd_target_ecoff_flavour)
	return ecoff_data (abfd)->gp_size;
      else if (abfd->xvec->flavour == bfd_target_elf_flavour)
	return elf_gp_size (abfd);
    }
  return 0;
}

/*
FUNCTION
	bfd_set_gp_size

SYNOPSIS
	void bfd_set_gp_size (bfd *abfd, unsigned int i);

DESCRIPTION
	Set the maximum size of objects to be optimized using the GP
	register under ECOFF or MIPS ELF.  This is typically set by
	the <<-G>> argument to the compiler, assembler or linker.
*/

void
bfd_set_gp_size (bfd *abfd, unsigned int i)
{
  /* Don't try to set GP size on an archive or core file!  */
  if (abfd->format != bfd_object)
    return;

  if (abfd->xvec->flavour == bfd_target_ecoff_flavour)
    ecoff_data (abfd)->gp_size = i;
  else if (abfd->xvec->flavour == bfd_target_elf_flavour)
    elf_gp_size (abfd) = i;
}

/* Get the GP value.  This is an internal function used by some of the
   relocation special_function routines on targets which support a GP
   register.  */

bfd_vma
_bfd_get_gp_value (bfd *abfd)
{
  if (! abfd)
    return 0;
  if (abfd->format != bfd_object)
    return 0;

  if (abfd->xvec->flavour == bfd_target_ecoff_flavour)
    return ecoff_data (abfd)->gp;
  else if (abfd->xvec->flavour == bfd_target_elf_flavour)
    return elf_gp (abfd);

  return 0;
}

/* Set the GP value.  */

void
_bfd_set_gp_value (bfd *abfd, bfd_vma v)
{
  if (! abfd)
    abort ();
  if (abfd->format != bfd_object)
    return;

  if (abfd->xvec->flavour == bfd_target_ecoff_flavour)
    ecoff_data (abfd)->gp = v;
  else if (abfd->xvec->flavour == bfd_target_elf_flavour)
    elf_gp (abfd) = v;
}

/*
FUNCTION
	bfd_set_gp_value

SYNOPSIS
	void bfd_set_gp_value (bfd *abfd, bfd_vma v);

DESCRIPTION
	Allow external access to the fucntion to set the GP value.
	This is specifically added for gdb-compile support.
*/

void
bfd_set_gp_value (bfd *abfd, bfd_vma v)
{
  _bfd_set_gp_value (abfd, v);
}

/*
FUNCTION
	bfd_scan_vma

SYNOPSIS
	bfd_vma bfd_scan_vma (const char *string, const char **end, int base);

DESCRIPTION
	Convert, like <<strtoul>>, a numerical expression
	@var{string} into a <<bfd_vma>> integer, and return that integer.
	(Though without as many bells and whistles as <<strtoul>>.)
	The expression is assumed to be unsigned (i.e., positive).
	If given a @var{base}, it is used as the base for conversion.
	A base of 0 causes the function to interpret the string
	in hex if a leading "0x" or "0X" is found, otherwise
	in octal if a leading zero is found, otherwise in decimal.

	If the value would overflow, the maximum <<bfd_vma>> value is
	returned.
*/

bfd_vma
bfd_scan_vma (const char *string, const char **end, int base)
{
  bfd_vma value;
  bfd_vma cutoff;
  unsigned int cutlim;
  int overflow;

  /* Let the host do it if possible.  */
  if (sizeof (bfd_vma) <= sizeof (unsigned long))
    return strtoul (string, (char **) end, base);

  if (sizeof (bfd_vma) <= sizeof (unsigned long long))
    return strtoull (string, (char **) end, base);

  if (base == 0)
    {
      if (string[0] == '0')
	{
	  if ((string[1] == 'x') || (string[1] == 'X'))
	    base = 16;
	  else
	    base = 8;
	}
    }

  if ((base < 2) || (base > 36))
    base = 10;

  if (base == 16
      && string[0] == '0'
      && (string[1] == 'x' || string[1] == 'X')
      && ISXDIGIT (string[2]))
    {
      string += 2;
    }

  cutoff = (~ (bfd_vma) 0) / (bfd_vma) base;
  cutlim = (~ (bfd_vma) 0) % (bfd_vma) base;
  value = 0;
  overflow = 0;
  while (1)
    {
      unsigned int digit;

      digit = *string;
      if (ISDIGIT (digit))
	digit = digit - '0';
      else if (ISALPHA (digit))
	digit = TOUPPER (digit) - 'A' + 10;
      else
	break;
      if (digit >= (unsigned int) base)
	break;
      if (value > cutoff || (value == cutoff && digit > cutlim))
	overflow = 1;
      value = value * base + digit;
      ++string;
    }

  if (overflow)
    value = ~ (bfd_vma) 0;

  if (end != NULL)
    *end = string;

  return value;
}

/*
FUNCTION
	bfd_copy_private_header_data

SYNOPSIS
	bool bfd_copy_private_header_data (bfd *ibfd, bfd *obfd);

DESCRIPTION
	Copy private BFD header information from the BFD @var{ibfd} to the
	the BFD @var{obfd}.  This copies information that may require
	sections to exist, but does not require symbol tables.  Return
	<<true>> on success, <<false>> on error.
	Possible error returns are:

	o <<bfd_error_no_memory>> -
	Not enough memory exists to create private data for @var{obfd}.

.#define bfd_copy_private_header_data(ibfd, obfd) \
.	BFD_SEND (obfd, _bfd_copy_private_header_data, \
.		  (ibfd, obfd))

*/

/*
FUNCTION
	bfd_copy_private_bfd_data

SYNOPSIS
	bool bfd_copy_private_bfd_data (bfd *ibfd, bfd *obfd);

DESCRIPTION
	Copy private BFD information from the BFD @var{ibfd} to the
	the BFD @var{obfd}.  Return <<TRUE>> on success, <<FALSE>> on error.
	Possible error returns are:

	o <<bfd_error_no_memory>> -
	Not enough memory exists to create private data for @var{obfd}.

.#define bfd_copy_private_bfd_data(ibfd, obfd) \
.	BFD_SEND (obfd, _bfd_copy_private_bfd_data, \
.		  (ibfd, obfd))

*/

/*
FUNCTION
	bfd_set_private_flags

SYNOPSIS
	bool bfd_set_private_flags (bfd *abfd, flagword flags);

DESCRIPTION
	Set private BFD flag information in the BFD @var{abfd}.
	Return <<TRUE>> on success, <<FALSE>> on error.  Possible error
	returns are:

	o <<bfd_error_no_memory>> -
	Not enough memory exists to create private data for @var{obfd}.

.#define bfd_set_private_flags(abfd, flags) \
.	BFD_SEND (abfd, _bfd_set_private_flags, (abfd, flags))

*/

/*
FUNCTION
	Other functions

DESCRIPTION
	The following functions exist but have not yet been documented.

.#define bfd_sizeof_headers(abfd, info) \
.	BFD_SEND (abfd, _bfd_sizeof_headers, (abfd, info))
.
.#define bfd_find_nearest_line(abfd, sec, syms, off, file, func, line) \
.	BFD_SEND (abfd, _bfd_find_nearest_line, \
.		  (abfd, syms, sec, off, file, func, line, NULL))
.
.#define bfd_find_nearest_line_with_alt(abfd, alt_filename, sec, syms, off, \
.					file, func, line, disc) \
.	BFD_SEND (abfd, _bfd_find_nearest_line_with_alt, \
.		  (abfd, alt_filename, syms, sec, off, file, func, line, disc))
.
.#define bfd_find_nearest_line_discriminator(abfd, sec, syms, off, file, func, \
.					    line, disc) \
.	BFD_SEND (abfd, _bfd_find_nearest_line, \
.		  (abfd, syms, sec, off, file, func, line, disc))
.
.#define bfd_find_line(abfd, syms, sym, file, line) \
.	BFD_SEND (abfd, _bfd_find_line, \
.		  (abfd, syms, sym, file, line))
.
.#define bfd_find_inliner_info(abfd, file, func, line) \
.	BFD_SEND (abfd, _bfd_find_inliner_info, \
.		  (abfd, file, func, line))
.
.#define bfd_debug_info_start(abfd) \
.	BFD_SEND (abfd, _bfd_debug_info_start, (abfd))
.
.#define bfd_debug_info_end(abfd) \
.	BFD_SEND (abfd, _bfd_debug_info_end, (abfd))
.
.#define bfd_debug_info_accumulate(abfd, section) \
.	BFD_SEND (abfd, _bfd_debug_info_accumulate, (abfd, section))
.
.#define bfd_stat_arch_elt(abfd, stat) \
.	BFD_SEND (abfd->my_archive ? abfd->my_archive : abfd, \
.		  _bfd_stat_arch_elt, (abfd, stat))
.
.#define bfd_update_armap_timestamp(abfd) \
.	BFD_SEND (abfd, _bfd_update_armap_timestamp, (abfd))
.
.#define bfd_set_arch_mach(abfd, arch, mach)\
.	BFD_SEND ( abfd, _bfd_set_arch_mach, (abfd, arch, mach))
.
.#define bfd_relax_section(abfd, section, link_info, again) \
.	BFD_SEND (abfd, _bfd_relax_section, (abfd, section, link_info, again))
.
.#define bfd_gc_sections(abfd, link_info) \
.	BFD_SEND (abfd, _bfd_gc_sections, (abfd, link_info))
.
.#define bfd_lookup_section_flags(link_info, flag_info, section) \
.	BFD_SEND (abfd, _bfd_lookup_section_flags, (link_info, flag_info, section))
.
.#define bfd_merge_sections(abfd, link_info) \
.	BFD_SEND (abfd, _bfd_merge_sections, (abfd, link_info))
.
.#define bfd_is_group_section(abfd, sec) \
.	BFD_SEND (abfd, _bfd_is_group_section, (abfd, sec))
.
.#define bfd_group_name(abfd, sec) \
.	BFD_SEND (abfd, _bfd_group_name, (abfd, sec))
.
.#define bfd_discard_group(abfd, sec) \
.	BFD_SEND (abfd, _bfd_discard_group, (abfd, sec))
.
.#define bfd_link_hash_table_create(abfd) \
.	BFD_SEND (abfd, _bfd_link_hash_table_create, (abfd))
.
.#define bfd_link_add_symbols(abfd, info) \
.	BFD_SEND (abfd, _bfd_link_add_symbols, (abfd, info))
.
.#define bfd_link_just_syms(abfd, sec, info) \
.	BFD_SEND (abfd, _bfd_link_just_syms, (sec, info))
.
.#define bfd_final_link(abfd, info) \
.	BFD_SEND (abfd, _bfd_final_link, (abfd, info))
.
.#define bfd_free_cached_info(abfd) \
.	BFD_SEND (abfd, _bfd_free_cached_info, (abfd))
.
.#define bfd_get_dynamic_symtab_upper_bound(abfd) \
.	BFD_SEND (abfd, _bfd_get_dynamic_symtab_upper_bound, (abfd))
.
.#define bfd_print_private_bfd_data(abfd, file)\
.	BFD_SEND (abfd, _bfd_print_private_bfd_data, (abfd, file))
.
.#define bfd_canonicalize_dynamic_symtab(abfd, asymbols) \
.	BFD_SEND (abfd, _bfd_canonicalize_dynamic_symtab, (abfd, asymbols))
.
.#define bfd_get_synthetic_symtab(abfd, count, syms, dyncount, dynsyms, ret) \
.	BFD_SEND (abfd, _bfd_get_synthetic_symtab, (abfd, count, syms, \
.						    dyncount, dynsyms, ret))
.
.#define bfd_get_dynamic_reloc_upper_bound(abfd) \
.	BFD_SEND (abfd, _bfd_get_dynamic_reloc_upper_bound, (abfd))
.
.#define bfd_canonicalize_dynamic_reloc(abfd, arels, asyms) \
.	BFD_SEND (abfd, _bfd_canonicalize_dynamic_reloc, (abfd, arels, asyms))
.
*/

/*
FUNCTION
	bfd_get_relocated_section_contents

SYNOPSIS
	bfd_byte *bfd_get_relocated_section_contents
	  (bfd *, struct bfd_link_info *, struct bfd_link_order *, bfd_byte *,
	   bool, asymbol **);

DESCRIPTION
	Read and relocate the indirect link_order section, into DATA
	(if non-NULL) or to a malloc'd buffer.  Return the buffer, or
	NULL on errors.
*/

bfd_byte *
bfd_get_relocated_section_contents (bfd *abfd,
				    struct bfd_link_info *link_info,
				    struct bfd_link_order *link_order,
				    bfd_byte *data,
				    bool relocatable,
				    asymbol **symbols)
{
  bfd *abfd2;
  bfd_byte *(*fn) (bfd *, struct bfd_link_info *, struct bfd_link_order *,
		   bfd_byte *, bool, asymbol **);

  if (link_order->type == bfd_indirect_link_order)
    {
      abfd2 = link_order->u.indirect.section->owner;
      if (abfd2 == NULL)
	abfd2 = abfd;
    }
  else
    abfd2 = abfd;

  fn = abfd2->xvec->_bfd_get_relocated_section_contents;

  return (*fn) (abfd, link_info, link_order, data, relocatable, symbols);
}

/*
FUNCTION
	bfd_record_phdr

SYNOPSIS
	bool bfd_record_phdr
	  (bfd *, unsigned long, bool, flagword, bool, bfd_vma,
	   bool, bool, unsigned int, struct bfd_section **);

DESCRIPTION
	Record information about an ELF program header.
*/

bool
bfd_record_phdr (bfd *abfd,
		 unsigned long type,
		 bool flags_valid,
		 flagword flags,
		 bool at_valid,
		 bfd_vma at,  /* Bytes.  */
		 bool includes_filehdr,
		 bool includes_phdrs,
		 unsigned int count,
		 asection **secs)
{
  struct elf_segment_map *m, **pm;
  size_t amt;
  unsigned int opb = bfd_octets_per_byte (abfd, NULL);

  if (bfd_get_flavour (abfd) != bfd_target_elf_flavour)
    return true;

  amt = sizeof (struct elf_segment_map);
  amt += ((bfd_size_type) count - 1) * sizeof (asection *);
  m = (struct elf_segment_map *) bfd_zalloc (abfd, amt);
  if (m == NULL)
    return false;

  m->p_type = type;
  m->p_flags = flags;
  m->p_paddr = at * opb;
  m->p_flags_valid = flags_valid;
  m->p_paddr_valid = at_valid;
  m->includes_filehdr = includes_filehdr;
  m->includes_phdrs = includes_phdrs;
  m->count = count;
  if (count > 0)
    memcpy (m->sections, secs, count * sizeof (asection *));

  for (pm = &elf_seg_map (abfd); *pm != NULL; pm = &(*pm)->next)
    ;
  *pm = m;

  return true;
}

#ifdef BFD64
/* Return true iff this target is 32-bit.  */

static bool
is32bit (bfd *abfd)
{
  if (bfd_get_flavour (abfd) == bfd_target_elf_flavour)
    {
      const struct elf_backend_data *bed = get_elf_backend_data (abfd);
      return bed->s->elfclass == ELFCLASS32;
    }

  /* For non-ELF targets, use architecture information.  */
  return bfd_arch_bits_per_address (abfd) <= 32;
}
#endif

/*
FUNCTION
	bfd_sprintf_vma
	bfd_fprintf_vma

SYNOPSIS
	void bfd_sprintf_vma (bfd *, char *, bfd_vma);
	void bfd_fprintf_vma (bfd *, void *, bfd_vma);

DESCRIPTION
	bfd_sprintf_vma and bfd_fprintf_vma display an address in the
	target's address size.

EXTERNAL
.#define bfd_printf_vma(abfd,x) bfd_fprintf_vma (abfd, stdout, x)
.
*/

void
bfd_sprintf_vma (bfd *abfd ATTRIBUTE_UNUSED, char *buf, bfd_vma value)
{
#ifdef BFD64
  if (!is32bit (abfd))
    {
      sprintf (buf, "%016" PRIx64, (uint64_t) value);
      return;
    }
#endif
  sprintf (buf, "%08lx", (unsigned long) value & 0xffffffff);
}

void
bfd_fprintf_vma (bfd *abfd ATTRIBUTE_UNUSED, void *stream, bfd_vma value)
{
#ifdef BFD64
  if (!is32bit (abfd))
    {
      fprintf ((FILE *) stream, "%016" PRIx64, (uint64_t) value);
      return;
    }
#endif
  fprintf ((FILE *) stream, "%08lx", (unsigned long) value & 0xffffffff);
}

/*
FUNCTION
	bfd_alt_mach_code

SYNOPSIS
	bool bfd_alt_mach_code (bfd *abfd, int alternative);

DESCRIPTION

	When more than one machine code number is available for the
	same machine type, this function can be used to switch between
	the preferred one (alternative == 0) and any others.  Currently,
	only ELF supports this feature, with up to two alternate
	machine codes.
*/

bool
bfd_alt_mach_code (bfd *abfd, int alternative)
{
  if (bfd_get_flavour (abfd) == bfd_target_elf_flavour)
    {
      int code;

      switch (alternative)
	{
	case 0:
	  code = get_elf_backend_data (abfd)->elf_machine_code;
	  break;

	case 1:
	  code = get_elf_backend_data (abfd)->elf_machine_alt1;
	  if (code == 0)
	    return false;
	  break;

	case 2:
	  code = get_elf_backend_data (abfd)->elf_machine_alt2;
	  if (code == 0)
	    return false;
	  break;

	default:
	  return false;
	}

      elf_elfheader (abfd)->e_machine = code;

      return true;
    }

  return false;
}

/*
FUNCTION
	bfd_emul_get_maxpagesize

SYNOPSIS
	bfd_vma bfd_emul_get_maxpagesize (const char *);

DESCRIPTION
	Returns the maximum page size, in bytes, as determined by
	emulation.
*/

bfd_vma
bfd_emul_get_maxpagesize (const char *emul)
{
  const bfd_target *target;

  target = bfd_find_target (emul, NULL);
  if (target != NULL
      && target->flavour == bfd_target_elf_flavour)
    return xvec_get_elf_backend_data (target)->maxpagesize;

  return 0;
}

/*
FUNCTION
	bfd_emul_get_commonpagesize

SYNOPSIS
	bfd_vma bfd_emul_get_commonpagesize (const char *);

DESCRIPTION
	Returns the common page size, in bytes, as determined by
	emulation.
*/

bfd_vma
bfd_emul_get_commonpagesize (const char *emul)
{
  const bfd_target *target;

  target = bfd_find_target (emul, NULL);
  if (target != NULL
      && target->flavour == bfd_target_elf_flavour)
    {
      const struct elf_backend_data *bed;

      bed = xvec_get_elf_backend_data (target);
      return bed->commonpagesize;
    }
  return 0;
}

/*
FUNCTION
	bfd_demangle

SYNOPSIS
	char *bfd_demangle (bfd *, const char *, int);

DESCRIPTION
	Wrapper around cplus_demangle.  Strips leading underscores and
	other such chars that would otherwise confuse the demangler.
	If passed a g++ v3 ABI mangled name, returns a buffer allocated
	with malloc holding the demangled name.  Returns NULL otherwise
	and on memory alloc failure.
*/

char *
bfd_demangle (bfd *abfd, const char *name, int options)
{
  char *res, *alloc;
  const char *pre, *suf;
  size_t pre_len;
  bool skip_lead;

  skip_lead = (abfd != NULL
	       && *name != '\0'
	       && bfd_get_symbol_leading_char (abfd) == *name);
  if (skip_lead)
    ++name;

  /* This is a hack for better error reporting on XCOFF, PowerPC64-ELF
     or the MS PE format.  These formats have a number of leading '.'s
     on at least some symbols, so we remove all dots to avoid
     confusing the demangler.  */
  pre = name;
  while (*name == '.' || *name == '$')
    ++name;
  pre_len = name - pre;

  /* Strip off @plt and suchlike too.  */
  alloc = NULL;
  suf = strchr (name, '@');
  if (suf != NULL)
    {
      alloc = (char *) bfd_malloc (suf - name + 1);
      if (alloc == NULL)
	return NULL;
      memcpy (alloc, name, suf - name);
      alloc[suf - name] = '\0';
      name = alloc;
    }

  res = cplus_demangle (name, options);

  free (alloc);

  if (res == NULL)
    {
      if (skip_lead)
	{
	  size_t len = strlen (pre) + 1;
	  alloc = (char *) bfd_malloc (len);
	  if (alloc == NULL)
	    return NULL;
	  memcpy (alloc, pre, len);
	  return alloc;
	}
      return NULL;
    }

  /* Put back any prefix or suffix.  */
  if (pre_len != 0 || suf != NULL)
    {
      size_t len;
      size_t suf_len;
      char *final;

      len = strlen (res);
      if (suf == NULL)
	suf = res + len;
      suf_len = strlen (suf) + 1;
      final = (char *) bfd_malloc (pre_len + len + suf_len);
      if (final != NULL)
	{
	  memcpy (final, pre, pre_len);
	  memcpy (final + pre_len, res, len);
	  memcpy (final + pre_len + len, suf, suf_len);
	}
      free (res);
      res = final;
    }

  return res;
}

/* Get the linker information.  */

struct bfd_link_info *
_bfd_get_link_info (bfd *abfd)
{
  if (bfd_get_flavour (abfd) != bfd_target_elf_flavour)
    return NULL;

  return elf_link_info (abfd);
}
