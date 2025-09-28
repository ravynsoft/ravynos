/* Object file "section" support for the BFD library.
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
SECTION
	Sections

	The raw data contained within a BFD is maintained through the
	section abstraction.  A single BFD may have any number of
	sections.  It keeps hold of them by pointing to the first;
	each one points to the next in the list.

	Sections are supported in BFD in <<section.c>>.

@menu
@* Section Input::
@* Section Output::
@* typedef asection::
@* section prototypes::
@end menu

INODE
Section Input, Section Output, Sections, Sections
SUBSECTION
	Section input

	When a BFD is opened for reading, the section structures are
	created and attached to the BFD.

	Each section has a name which describes the section in the
	outside world---for example, <<a.out>> would contain at least
	three sections, called <<.text>>, <<.data>> and <<.bss>>.

	Names need not be unique; for example a COFF file may have several
	sections named <<.data>>.

	Sometimes a BFD will contain more than the ``natural'' number of
	sections. A back end may attach other sections containing
	constructor data, or an application may add a section (using
	<<bfd_make_section>>) to the sections attached to an already open
	BFD. For example, the linker creates an extra section
	<<COMMON>> for each input file's BFD to hold information about
	common storage.

	The raw data is not necessarily read in when
	the section descriptor is created. Some targets may leave the
	data in place until a <<bfd_get_section_contents>> call is
	made. Other back ends may read in all the data at once.  For
	example, an S-record file has to be read once to determine the
	size of the data.

INODE
Section Output, typedef asection, Section Input, Sections

SUBSECTION
	Section output

	To write a new object style BFD, the various sections to be
	written have to be created. They are attached to the BFD in
	the same way as input sections; data is written to the
	sections using <<bfd_set_section_contents>>.

	Any program that creates or combines sections (e.g., the assembler
	and linker) must use the <<asection>> fields <<output_section>> and
	<<output_offset>> to indicate the file sections to which each
	section must be written.  (If the section is being created from
	scratch, <<output_section>> should probably point to the section
	itself and <<output_offset>> should probably be zero.)

	The data to be written comes from input sections attached
	(via <<output_section>> pointers) to
	the output sections.  The output section structure can be
	considered a filter for the input section: the output section
	determines the vma of the output data and the name, but the
	input section determines the offset into the output section of
	the data to be written.

	E.g., to create a section "O", starting at 0x100, 0x123 long,
	containing two subsections, "A" at offset 0x0 (i.e., at vma
	0x100) and "B" at offset 0x20 (i.e., at vma 0x120) the <<asection>>
	structures would look like:

|   section name          "A"
|     output_offset   0x00
|     size            0x20
|     output_section ----------->  section name    "O"
|                             |    vma             0x100
|   section name          "B" |    size            0x123
|     output_offset   0x20    |
|     size            0x103   |
|     output_section  --------|

SUBSECTION
	Link orders

	The data within a section is stored in a @dfn{link_order}.
	These are much like the fixups in <<gas>>.  The link_order
	abstraction allows a section to grow and shrink within itself.

	A link_order knows how big it is, and which is the next
	link_order and where the raw data for it is; it also points to
	a list of relocations which apply to it.

	The link_order is used by the linker to perform relaxing on
	final code.  The compiler creates code which is as big as
	necessary to make it work without relaxing, and the user can
	select whether to relax.  Sometimes relaxing takes a lot of
	time.  The linker runs around the relocations to see if any
	are attached to data which can be shrunk, if so it does it on
	a link_order by link_order basis.

*/

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"
#include "bfdlink.h"

/*
DOCDD
INODE
	typedef asection, section prototypes, Section Output, Sections
SUBSECTION
	typedef asection

	Here is the section structure:

EXTERNAL
.{* Linenumber stuff.  *}
.typedef struct lineno_cache_entry
.{
.  unsigned int line_number;	{* Linenumber from start of function.  *}
.  union
.  {
.    struct bfd_symbol *sym;	{* Function name.  *}
.    bfd_vma offset;		{* Offset into section.  *}
.  } u;
.}
.alent;
.

CODE_FRAGMENT
.typedef struct bfd_section
.{
.  {* The name of the section; the name isn't a copy, the pointer is
.     the same as that passed to bfd_make_section.  *}
.  const char *name;
.
.  {* The next section in the list belonging to the BFD, or NULL.  *}
.  struct bfd_section *next;
.
.  {* The previous section in the list belonging to the BFD, or NULL.  *}
.  struct bfd_section *prev;
.
.  {* A unique sequence number.  *}
.  unsigned int id;
.
.  {* A unique section number which can be used by assembler to
.     distinguish different sections with the same section name.  *}
.  unsigned int section_id;
.
.  {* Which section in the bfd; 0..n-1 as sections are created in a bfd.  *}
.  unsigned int index;
.
.  {* The field flags contains attributes of the section. Some
.     flags are read in from the object file, and some are
.     synthesized from other information.  *}
.  flagword flags;
.
.#define SEC_NO_FLAGS                      0x0
.
.  {* Tells the OS to allocate space for this section when loading.
.     This is clear for a section containing debug information only.  *}
.#define SEC_ALLOC                         0x1
.
.  {* Tells the OS to load the section from the file when loading.
.     This is clear for a .bss section.  *}
.#define SEC_LOAD                          0x2
.
.  {* The section contains data still to be relocated, so there is
.     some relocation information too.  *}
.#define SEC_RELOC                         0x4
.
.  {* A signal to the OS that the section contains read only data.  *}
.#define SEC_READONLY                      0x8
.
.  {* The section contains code only.  *}
.#define SEC_CODE                         0x10
.
.  {* The section contains data only.  *}
.#define SEC_DATA                         0x20
.
.  {* The section will reside in ROM.  *}
.#define SEC_ROM                          0x40
.
.  {* The section contains constructor information. This section
.     type is used by the linker to create lists of constructors and
.     destructors used by <<g++>>. When a back end sees a symbol
.     which should be used in a constructor list, it creates a new
.     section for the type of name (e.g., <<__CTOR_LIST__>>), attaches
.     the symbol to it, and builds a relocation. To build the lists
.     of constructors, all the linker has to do is catenate all the
.     sections called <<__CTOR_LIST__>> and relocate the data
.     contained within - exactly the operations it would peform on
.     standard data.  *}
.#define SEC_CONSTRUCTOR                  0x80
.
.  {* The section has contents - a data section could be
.     <<SEC_ALLOC>> | <<SEC_HAS_CONTENTS>>; a debug section could be
.     <<SEC_HAS_CONTENTS>>  *}
.#define SEC_HAS_CONTENTS                0x100
.
.  {* An instruction to the linker to not output the section
.     even if it has information which would normally be written.  *}
.#define SEC_NEVER_LOAD                  0x200
.
.  {* The section contains thread local data.  *}
.#define SEC_THREAD_LOCAL                0x400
.
.  {* The section's size is fixed.  Generic linker code will not
.     recalculate it and it is up to whoever has set this flag to
.     get the size right.  *}
.#define SEC_FIXED_SIZE                  0x800
.
.  {* The section contains common symbols (symbols may be defined
.     multiple times, the value of a symbol is the amount of
.     space it requires, and the largest symbol value is the one
.     used).  Most targets have exactly one of these (which we
.     translate to bfd_com_section_ptr), but ECOFF has two.  *}
.#define SEC_IS_COMMON                  0x1000
.
.  {* The section contains only debugging information.  For
.     example, this is set for ELF .debug and .stab sections.
.     strip tests this flag to see if a section can be
.     discarded.  *}
.#define SEC_DEBUGGING                  0x2000
.
.  {* The contents of this section are held in memory pointed to
.     by the contents field.  This is checked by bfd_get_section_contents,
.     and the data is retrieved from memory if appropriate.  *}
.#define SEC_IN_MEMORY                  0x4000
.
.  {* The contents of this section are to be excluded by the
.     linker for executable and shared objects unless those
.     objects are to be further relocated.  *}
.#define SEC_EXCLUDE                    0x8000
.
.  {* The contents of this section are to be sorted based on the sum of
.     the symbol and addend values specified by the associated relocation
.     entries.  Entries without associated relocation entries will be
.     appended to the end of the section in an unspecified order.  *}
.#define SEC_SORT_ENTRIES              0x10000
.
.  {* When linking, duplicate sections of the same name should be
.     discarded, rather than being combined into a single section as
.     is usually done.  This is similar to how common symbols are
.     handled.  See SEC_LINK_DUPLICATES below.  *}
.#define SEC_LINK_ONCE                 0x20000
.
.  {* If SEC_LINK_ONCE is set, this bitfield describes how the linker
.     should handle duplicate sections.  *}
.#define SEC_LINK_DUPLICATES           0xc0000
.
.  {* This value for SEC_LINK_DUPLICATES means that duplicate
.     sections with the same name should simply be discarded.  *}
.#define SEC_LINK_DUPLICATES_DISCARD       0x0
.
.  {* This value for SEC_LINK_DUPLICATES means that the linker
.     should warn if there are any duplicate sections, although
.     it should still only link one copy.  *}
.#define SEC_LINK_DUPLICATES_ONE_ONLY  0x40000
.
.  {* This value for SEC_LINK_DUPLICATES means that the linker
.     should warn if any duplicate sections are a different size.  *}
.#define SEC_LINK_DUPLICATES_SAME_SIZE 0x80000
.
.  {* This value for SEC_LINK_DUPLICATES means that the linker
.     should warn if any duplicate sections contain different
.     contents.  *}
.#define SEC_LINK_DUPLICATES_SAME_CONTENTS \
.  (SEC_LINK_DUPLICATES_ONE_ONLY | SEC_LINK_DUPLICATES_SAME_SIZE)
.
.  {* This section was created by the linker as part of dynamic
.     relocation or other arcane processing.  It is skipped when
.     going through the first-pass output, trusting that someone
.     else up the line will take care of it later.  *}
.#define SEC_LINKER_CREATED           0x100000
.
.  {* This section contains a section ID to distinguish different
.     sections with the same section name.  *}
.#define SEC_ASSEMBLER_SECTION_ID     0x100000
.
.  {* This section should not be subject to garbage collection.
.     Also set to inform the linker that this section should not be
.     listed in the link map as discarded.  *}
.#define SEC_KEEP                     0x200000
.
.  {* This section contains "short" data, and should be placed
.     "near" the GP.  *}
.#define SEC_SMALL_DATA               0x400000
.
.  {* Attempt to merge identical entities in the section.
.     Entity size is given in the entsize field.  *}
.#define SEC_MERGE                    0x800000
.
.  {* If given with SEC_MERGE, entities to merge are zero terminated
.     strings where entsize specifies character size instead of fixed
.     size entries.  *}
.#define SEC_STRINGS                 0x1000000
.
.  {* This section contains data about section groups.  *}
.#define SEC_GROUP                   0x2000000
.
.  {* The section is a COFF shared library section.  This flag is
.     only for the linker.  If this type of section appears in
.     the input file, the linker must copy it to the output file
.     without changing the vma or size.  FIXME: Although this
.     was originally intended to be general, it really is COFF
.     specific (and the flag was renamed to indicate this).  It
.     might be cleaner to have some more general mechanism to
.     allow the back end to control what the linker does with
.     sections.  *}
.#define SEC_COFF_SHARED_LIBRARY     0x4000000
.
.  {* This input section should be copied to output in reverse order
.     as an array of pointers.  This is for ELF linker internal use
.     only.  *}
.#define SEC_ELF_REVERSE_COPY        0x4000000
.
.  {* This section contains data which may be shared with other
.     executables or shared objects. This is for COFF only.  *}
.#define SEC_COFF_SHARED             0x8000000
.
.  {* Indicate that section has the purecode flag set.  *}
.#define SEC_ELF_PURECODE            0x8000000
.
.  {* When a section with this flag is being linked, then if the size of
.     the input section is less than a page, it should not cross a page
.     boundary.  If the size of the input section is one page or more,
.     it should be aligned on a page boundary.  This is for TI
.     TMS320C54X only.  *}
.#define SEC_TIC54X_BLOCK           0x10000000
.
.  {* Conditionally link this section; do not link if there are no
.     references found to any symbol in the section.  This is for TI
.     TMS320C54X only.  *}
.#define SEC_TIC54X_CLINK           0x20000000
.
.  {* This section contains vliw code.  This is for Toshiba MeP only.  *}
.#define SEC_MEP_VLIW               0x20000000
.
.  {* All symbols, sizes and relocations in this section are octets
.     instead of bytes.  Required for DWARF debug sections as DWARF
.     information is organized in octets, not bytes.  *}
.#define SEC_ELF_OCTETS             0x40000000
.
.  {* Indicate that section has the no read flag set. This happens
.     when memory read flag isn't set. *}
.#define SEC_COFF_NOREAD            0x40000000
.
.  {*  End of section flags.  *}
.
.  {* Some internal packed boolean fields.  *}
.
.  {* See the vma field.  *}
.  unsigned int user_set_vma : 1;
.
.  {* A mark flag used by some of the linker backends.  *}
.  unsigned int linker_mark : 1;
.
.  {* Another mark flag used by some of the linker backends.  Set for
.     output sections that have an input section.  *}
.  unsigned int linker_has_input : 1;
.
.  {* Mark flag used by some linker backends for garbage collection.  *}
.  unsigned int gc_mark : 1;
.
.  {* Section compression status.  *}
.  unsigned int compress_status : 2;
.#define COMPRESS_SECTION_NONE    0
.#define COMPRESS_SECTION_DONE    1
.#define DECOMPRESS_SECTION_ZLIB  2
.#define DECOMPRESS_SECTION_ZSTD  3
.
.  {* The following flags are used by the ELF linker. *}
.
.  {* Mark sections which have been allocated to segments.  *}
.  unsigned int segment_mark : 1;
.
.  {* Type of sec_info information.  *}
.  unsigned int sec_info_type:3;
.#define SEC_INFO_TYPE_NONE      0
.#define SEC_INFO_TYPE_STABS     1
.#define SEC_INFO_TYPE_MERGE     2
.#define SEC_INFO_TYPE_EH_FRAME  3
.#define SEC_INFO_TYPE_JUST_SYMS 4
.#define SEC_INFO_TYPE_TARGET    5
.#define SEC_INFO_TYPE_EH_FRAME_ENTRY 6
.#define SEC_INFO_TYPE_SFRAME  7
.
.  {* Nonzero if this section uses RELA relocations, rather than REL.  *}
.  unsigned int use_rela_p:1;
.
.  {* Bits used by various backends.  The generic code doesn't touch
.     these fields.  *}
.
.  unsigned int sec_flg0:1;
.  unsigned int sec_flg1:1;
.  unsigned int sec_flg2:1;
.  unsigned int sec_flg3:1;
.  unsigned int sec_flg4:1;
.  unsigned int sec_flg5:1;
.
.  {* End of internal packed boolean fields.  *}
.
.  {*  The virtual memory address of the section - where it will be
.      at run time.  The symbols are relocated against this.  The
.      user_set_vma flag is maintained by bfd; if it's not set, the
.      backend can assign addresses (for example, in <<a.out>>, where
.      the default address for <<.data>> is dependent on the specific
.      target and various flags).  *}
.  bfd_vma vma;
.
.  {*  The load address of the section - where it would be in a
.      rom image; really only used for writing section header
.      information.  *}
.  bfd_vma lma;
.
.  {* The size of the section in *octets*, as it will be output.
.     Contains a value even if the section has no contents (e.g., the
.     size of <<.bss>>).  *}
.  bfd_size_type size;
.
.  {* For input sections, the original size on disk of the section, in
.     octets.  This field should be set for any section whose size is
.     changed by linker relaxation.  It is required for sections where
.     the linker relaxation scheme doesn't cache altered section and
.     reloc contents (stabs, eh_frame, SEC_MERGE, some coff relaxing
.     targets), and thus the original size needs to be kept to read the
.     section multiple times.  For output sections, rawsize holds the
.     section size calculated on a previous linker relaxation pass.  *}
.  bfd_size_type rawsize;
.
.  {* The compressed size of the section in octets.  *}
.  bfd_size_type compressed_size;
.
.  {* If this section is going to be output, then this value is the
.     offset in *bytes* into the output section of the first byte in the
.     input section (byte ==> smallest addressable unit on the
.     target).  In most cases, if this was going to start at the
.     100th octet (8-bit quantity) in the output section, this value
.     would be 100.  However, if the target byte size is 16 bits
.     (bfd_octets_per_byte is "2"), this value would be 50.  *}
.  bfd_vma output_offset;
.
.  {* The output section through which to map on output.  *}
.  struct bfd_section *output_section;
.
.  {* If an input section, a pointer to a vector of relocation
.     records for the data in this section.  *}
.  struct reloc_cache_entry *relocation;
.
.  {* If an output section, a pointer to a vector of pointers to
.     relocation records for the data in this section.  *}
.  struct reloc_cache_entry **orelocation;
.
.  {* The number of relocation records in one of the above.  *}
.  unsigned reloc_count;
.
.  {* The alignment requirement of the section, as an exponent of 2 -
.     e.g., 3 aligns to 2^3 (or 8).  *}
.  unsigned int alignment_power;
.
.  {* Information below is back end specific - and not always used
.     or updated.  *}
.
.  {* File position of section data.  *}
.  file_ptr filepos;
.
.  {* File position of relocation info.  *}
.  file_ptr rel_filepos;
.
.  {* File position of line data.  *}
.  file_ptr line_filepos;
.
.  {* Pointer to data for applications.  *}
.  void *userdata;
.
.  {* If the SEC_IN_MEMORY flag is set, this points to the actual
.     contents.  *}
.  bfd_byte *contents;
.
.  {* Attached line number information.  *}
.  alent *lineno;
.
.  {* Number of line number records.  *}
.  unsigned int lineno_count;
.
.  {* Entity size for merging purposes.  *}
.  unsigned int entsize;
.
.  {* Points to the kept section if this section is a link-once section,
.     and is discarded.  *}
.  struct bfd_section *kept_section;
.
.  {* When a section is being output, this value changes as more
.     linenumbers are written out.  *}
.  file_ptr moving_line_filepos;
.
.  {* What the section number is in the target world.  *}
.  int target_index;
.
.  void *used_by_bfd;
.
.  {* If this is a constructor section then here is a list of the
.     relocations created to relocate items within it.  *}
.  struct relent_chain *constructor_chain;
.
.  {* The BFD which owns the section.  *}
.  bfd *owner;
.
.  {* A symbol which points at this section only.  *}
.  struct bfd_symbol *symbol;
.  struct bfd_symbol **symbol_ptr_ptr;
.
.  {* Early in the link process, map_head and map_tail are used to build
.     a list of input sections attached to an output section.  Later,
.     output sections use these fields for a list of bfd_link_order
.     structs.  The linked_to_symbol_name field is for ELF assembler
.     internal use.  *}
.  union {
.    struct bfd_link_order *link_order;
.    struct bfd_section *s;
.    const char *linked_to_symbol_name;
.  } map_head, map_tail;
.
.  {* Points to the output section this section is already assigned to,
.     if any.  This is used when support for non-contiguous memory
.     regions is enabled.  *}
.  struct bfd_section *already_assigned;
.
.  {* Explicitly specified section type, if non-zero.  *}
.  unsigned int type;
.
.} asection;
.

EXTERNAL
.static inline const char *
.bfd_section_name (const asection *sec)
.{
.  return sec->name;
.}
.
.static inline bfd_size_type
.bfd_section_size (const asection *sec)
.{
.  return sec->size;
.}
.
.static inline bfd_vma
.bfd_section_vma (const asection *sec)
.{
.  return sec->vma;
.}
.
.static inline bfd_vma
.bfd_section_lma (const asection *sec)
.{
.  return sec->lma;
.}
.
.static inline unsigned int
.bfd_section_alignment (const asection *sec)
.{
.  return sec->alignment_power;
.}
.
.static inline flagword
.bfd_section_flags (const asection *sec)
.{
.  return sec->flags;
.}
.
.static inline void *
.bfd_section_userdata (const asection *sec)
.{
.  return sec->userdata;
.}
.static inline bool
.bfd_is_com_section (const asection *sec)
.{
.  return (sec->flags & SEC_IS_COMMON) != 0;
.}
.
.{* Note: the following are provided as inline functions rather than macros
.   because not all callers use the return value.  A macro implementation
.   would use a comma expression, eg: "((ptr)->foo = val, TRUE)" and some
.   compilers will complain about comma expressions that have no effect.  *}
.static inline bool
.bfd_set_section_userdata (asection *sec, void *val)
.{
.  sec->userdata = val;
.  return true;
.}
.
.static inline bool
.bfd_set_section_vma (asection *sec, bfd_vma val)
.{
.  sec->vma = sec->lma = val;
.  sec->user_set_vma = true;
.  return true;
.}
.
.static inline bool
.bfd_set_section_lma (asection *sec, bfd_vma val)
.{
.  sec->lma = val;
.  return true;
.}
.
.static inline bool
.bfd_set_section_alignment (asection *sec, unsigned int val)
.{
.  if (val >= sizeof (bfd_vma) * 8 - 1)
.    return false;
.  sec->alignment_power = val;
.  return true;
.}
.
.{* These sections are global, and are managed by BFD.  The application
.   and target back end are not permitted to change the values in
.   these sections.  *}
.extern asection _bfd_std_section[4];
.
.#define BFD_ABS_SECTION_NAME "*ABS*"
.#define BFD_UND_SECTION_NAME "*UND*"
.#define BFD_COM_SECTION_NAME "*COM*"
.#define BFD_IND_SECTION_NAME "*IND*"
.
.{* Pointer to the common section.  *}
.#define bfd_com_section_ptr (&_bfd_std_section[0])
.{* Pointer to the undefined section.  *}
.#define bfd_und_section_ptr (&_bfd_std_section[1])
.{* Pointer to the absolute section.  *}
.#define bfd_abs_section_ptr (&_bfd_std_section[2])
.{* Pointer to the indirect section.  *}
.#define bfd_ind_section_ptr (&_bfd_std_section[3])
.
.static inline bool
.bfd_is_und_section (const asection *sec)
.{
.  return sec == bfd_und_section_ptr;
.}
.
.static inline bool
.bfd_is_abs_section (const asection *sec)
.{
.  return sec == bfd_abs_section_ptr;
.}
.
.static inline bool
.bfd_is_ind_section (const asection *sec)
.{
.  return sec == bfd_ind_section_ptr;
.}
.
.static inline bool
.bfd_is_const_section (const asection *sec)
.{
.  return (sec >= _bfd_std_section
.          && sec < _bfd_std_section + (sizeof (_bfd_std_section)
.                                       / sizeof (_bfd_std_section[0])));
.}
.
.{* Return TRUE if input section SEC has been discarded.  *}
.static inline bool
.discarded_section (const asection *sec)
.{
.  return (!bfd_is_abs_section (sec)
.          && bfd_is_abs_section (sec->output_section)
.          && sec->sec_info_type != SEC_INFO_TYPE_MERGE
.          && sec->sec_info_type != SEC_INFO_TYPE_JUST_SYMS);
.}
.
.#define BFD_FAKE_SECTION(SEC, SYM, NAME, IDX, FLAGS)			\
.  {* name, next, prev, id,  section_id, index, flags, user_set_vma, *}	\
.  {  NAME, NULL, NULL, IDX, 0,          0,     FLAGS, 0,		\
.									\
.  {* linker_mark, linker_has_input, gc_mark, decompress_status,     *}	\
.     0,           0,                1,       0,			\
.									\
.  {* segment_mark, sec_info_type, use_rela_p,                       *}	\
.     0,            0,             0,					\
.									\
.  {* sec_flg0, sec_flg1, sec_flg2, sec_flg3, sec_flg4, sec_flg5,    *}	\
.     0,        0,        0,        0,        0,        0,		\
.									\
.  {* vma, lma, size, rawsize, compressed_size,                      *}	\
.     0,   0,   0,    0,       0,					\
.									\
.  {* output_offset, output_section, relocation, orelocation,        *}	\
.     0,             &SEC,           NULL,       NULL,			\
.									\
.  {* reloc_count, alignment_power, filepos, rel_filepos,            *}	\
.     0,           0,               0,       0,				\
.									\
.  {* line_filepos, userdata, contents, lineno, lineno_count,        *}	\
.     0,            NULL,     NULL,     NULL,   0,			\
.									\
.  {* entsize, kept_section, moving_line_filepos,                    *}	\
.     0,       NULL,         0,						\
.									\
.  {* target_index, used_by_bfd, constructor_chain, owner,           *}	\
.     0,            NULL,        NULL,              NULL,		\
.									\
.  {* symbol,                    symbol_ptr_ptr,                     *}	\
.     (struct bfd_symbol *) SYM, &SEC.symbol,				\
.									\
.  {* map_head, map_tail, already_assigned, type                     *}	\
.     { NULL }, { NULL }, NULL,             0				\
.									\
.    }
.
.{* We use a macro to initialize the static asymbol structures because
.   traditional C does not permit us to initialize a union member while
.   gcc warns if we don't initialize it.
.   the_bfd, name, value, attr, section [, udata]  *}
.#ifdef __STDC__
.#define GLOBAL_SYM_INIT(NAME, SECTION) \
.  { 0, NAME, 0, BSF_SECTION_SYM, SECTION, { 0 }}
.#else
.#define GLOBAL_SYM_INIT(NAME, SECTION) \
.  { 0, NAME, 0, BSF_SECTION_SYM, SECTION }
.#endif
.
*/

/* These symbols are global, not specific to any BFD.  Therefore, anything
   that tries to change them is broken, and should be repaired.  */

static const asymbol global_syms[] =
{
  GLOBAL_SYM_INIT (BFD_COM_SECTION_NAME, bfd_com_section_ptr),
  GLOBAL_SYM_INIT (BFD_UND_SECTION_NAME, bfd_und_section_ptr),
  GLOBAL_SYM_INIT (BFD_ABS_SECTION_NAME, bfd_abs_section_ptr),
  GLOBAL_SYM_INIT (BFD_IND_SECTION_NAME, bfd_ind_section_ptr)
};

#define STD_SECTION(NAME, IDX, FLAGS) \
  BFD_FAKE_SECTION(_bfd_std_section[IDX], &global_syms[IDX], NAME, IDX, FLAGS)

asection _bfd_std_section[] = {
  STD_SECTION (BFD_COM_SECTION_NAME, 0, SEC_IS_COMMON),
  STD_SECTION (BFD_UND_SECTION_NAME, 1, 0),
  STD_SECTION (BFD_ABS_SECTION_NAME, 2, 0),
  STD_SECTION (BFD_IND_SECTION_NAME, 3, 0)
};
#undef STD_SECTION

/* Initialize an entry in the section hash table.  */

struct bfd_hash_entry *
bfd_section_hash_newfunc (struct bfd_hash_entry *entry,
			  struct bfd_hash_table *table,
			  const char *string)
{
  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (entry == NULL)
    {
      entry = (struct bfd_hash_entry *)
	bfd_hash_allocate (table, sizeof (struct section_hash_entry));
      if (entry == NULL)
	return entry;
    }

  /* Call the allocation method of the superclass.  */
  entry = bfd_hash_newfunc (entry, table, string);
  if (entry != NULL)
    memset (&((struct section_hash_entry *) entry)->section, 0,
	    sizeof (asection));

  return entry;
}

#define section_hash_lookup(table, string, create, copy) \
  ((struct section_hash_entry *) \
   bfd_hash_lookup ((table), (string), (create), (copy)))

/* Create a symbol whose only job is to point to this section.  This
   is useful for things like relocs which are relative to the base
   of a section.  */

bool
_bfd_generic_new_section_hook (bfd *abfd, asection *newsect)
{
  newsect->symbol = bfd_make_empty_symbol (abfd);
  if (newsect->symbol == NULL)
    return false;

  newsect->symbol->name = newsect->name;
  newsect->symbol->value = 0;
  newsect->symbol->section = newsect;
  newsect->symbol->flags = BSF_SECTION_SYM;

  newsect->symbol_ptr_ptr = &newsect->symbol;
  return true;
}

unsigned int _bfd_section_id = 0x10;  /* id 0 to 3 used by STD_SECTION.  */

/* Initializes a new section.  NEWSECT->NAME is already set.  */

static asection *
bfd_section_init (bfd *abfd, asection *newsect)
{
  newsect->id = _bfd_section_id;
  newsect->index = abfd->section_count;
  newsect->owner = abfd;

  if (! BFD_SEND (abfd, _new_section_hook, (abfd, newsect)))
    return NULL;

  _bfd_section_id++;
  abfd->section_count++;
  bfd_section_list_append (abfd, newsect);
  return newsect;
}

/*
DOCDD
INODE
section prototypes,  , typedef asection, Sections
SUBSECTION
	Section prototypes

These are the functions exported by the section handling part of BFD.
*/

/*
FUNCTION
	bfd_section_list_clear

SYNOPSIS
	void bfd_section_list_clear (bfd *);

DESCRIPTION
	Clears the section list, and also resets the section count and
	hash table entries.
*/

void
bfd_section_list_clear (bfd *abfd)
{
  abfd->sections = NULL;
  abfd->section_last = NULL;
  abfd->section_count = 0;
  memset (abfd->section_htab.table, 0,
	  abfd->section_htab.size * sizeof (struct bfd_hash_entry *));
  abfd->section_htab.count = 0;
}

/*
FUNCTION
	bfd_get_section_by_name

SYNOPSIS
	asection *bfd_get_section_by_name (bfd *abfd, const char *name);

DESCRIPTION
	Return the most recently created section attached to @var{abfd}
	named @var{name}.  Return NULL if no such section exists.
*/

asection *
bfd_get_section_by_name (bfd *abfd, const char *name)
{
  struct section_hash_entry *sh;

  if (name == NULL)
    return NULL;

  sh = section_hash_lookup (&abfd->section_htab, name, false, false);
  if (sh != NULL)
    return &sh->section;

  return NULL;
}

/*
FUNCTION
       bfd_get_next_section_by_name

SYNOPSIS
       asection *bfd_get_next_section_by_name (bfd *ibfd, asection *sec);

DESCRIPTION
       Given @var{sec} is a section returned by @code{bfd_get_section_by_name},
       return the next most recently created section attached to the same
       BFD with the same name, or if no such section exists in the same BFD and
       IBFD is non-NULL, the next section with the same name in any input
       BFD following IBFD.  Return NULL on finding no section.
*/

asection *
bfd_get_next_section_by_name (bfd *ibfd, asection *sec)
{
  struct section_hash_entry *sh;
  const char *name;
  unsigned long hash;

  sh = ((struct section_hash_entry *)
	((char *) sec - offsetof (struct section_hash_entry, section)));

  hash = sh->root.hash;
  name = sec->name;
  for (sh = (struct section_hash_entry *) sh->root.next;
       sh != NULL;
       sh = (struct section_hash_entry *) sh->root.next)
    if (sh->root.hash == hash
       && strcmp (sh->root.string, name) == 0)
      return &sh->section;

  if (ibfd != NULL)
    {
      while ((ibfd = ibfd->link.next) != NULL)
	{
	  asection *s = bfd_get_section_by_name (ibfd, name);
	  if (s != NULL)
	    return s;
	}
    }

  return NULL;
}

/*
FUNCTION
	bfd_get_linker_section

SYNOPSIS
	asection *bfd_get_linker_section (bfd *abfd, const char *name);

DESCRIPTION
	Return the linker created section attached to @var{abfd}
	named @var{name}.  Return NULL if no such section exists.
*/

asection *
bfd_get_linker_section (bfd *abfd, const char *name)
{
  asection *sec = bfd_get_section_by_name (abfd, name);

  while (sec != NULL && (sec->flags & SEC_LINKER_CREATED) == 0)
    sec = bfd_get_next_section_by_name (NULL, sec);
  return sec;
}

/*
FUNCTION
	bfd_get_section_by_name_if

SYNOPSIS
	asection *bfd_get_section_by_name_if
	  (bfd *abfd,
	   const char *name,
	   bool (*func) (bfd *abfd, asection *sect, void *obj),
	   void *obj);

DESCRIPTION
	Call the provided function @var{func} for each section
	attached to the BFD @var{abfd} whose name matches @var{name},
	passing @var{obj} as an argument. The function will be called
	as if by

|	func (abfd, the_section, obj);

	It returns the first section for which @var{func} returns true,
	otherwise <<NULL>>.

*/

asection *
bfd_get_section_by_name_if (bfd *abfd, const char *name,
			    bool (*operation) (bfd *, asection *, void *),
			    void *user_storage)
{
  struct section_hash_entry *sh;
  unsigned long hash;

  if (name == NULL)
    return NULL;

  sh = section_hash_lookup (&abfd->section_htab, name, false, false);
  if (sh == NULL)
    return NULL;

  hash = sh->root.hash;
  for (; sh != NULL; sh = (struct section_hash_entry *) sh->root.next)
    if (sh->root.hash == hash
	&& strcmp (sh->root.string, name) == 0
	&& (*operation) (abfd, &sh->section, user_storage))
      return &sh->section;

  return NULL;
}

/*
FUNCTION
	bfd_get_unique_section_name

SYNOPSIS
	char *bfd_get_unique_section_name
	  (bfd *abfd, const char *templat, int *count);

DESCRIPTION
	Invent a section name that is unique in @var{abfd} by tacking
	a dot and a digit suffix onto the original @var{templat}.  If
	@var{count} is non-NULL, then it specifies the first number
	tried as a suffix to generate a unique name.  The value
	pointed to by @var{count} will be incremented in this case.
*/

char *
bfd_get_unique_section_name (bfd *abfd, const char *templat, int *count)
{
  int num;
  unsigned int len;
  char *sname;

  len = strlen (templat);
  sname = (char *) bfd_malloc (len + 8);
  if (sname == NULL)
    return NULL;
  memcpy (sname, templat, len);
  num = 1;
  if (count != NULL)
    num = *count;

  do
    {
      /* If we have a million sections, something is badly wrong.  */
      if (num > 999999)
	abort ();
      sprintf (sname + len, ".%d", num++);
    }
  while (section_hash_lookup (&abfd->section_htab, sname, false, false));

  if (count != NULL)
    *count = num;
  return sname;
}

/*
FUNCTION
	bfd_make_section_old_way

SYNOPSIS
	asection *bfd_make_section_old_way (bfd *abfd, const char *name);

DESCRIPTION
	Create a new empty section called @var{name}
	and attach it to the end of the chain of sections for the
	BFD @var{abfd}. An attempt to create a section with a name which
	is already in use returns its pointer without changing the
	section chain.

	It has the funny name since this is the way it used to be
	before it was rewritten....

	Possible errors are:
	o <<bfd_error_invalid_operation>> -
	If output has already started for this BFD.
	o <<bfd_error_no_memory>> -
	If memory allocation fails.

*/

asection *
bfd_make_section_old_way (bfd *abfd, const char *name)
{
  asection *newsect;

  if (abfd->output_has_begun)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return NULL;
    }

  if (strcmp (name, BFD_ABS_SECTION_NAME) == 0)
    newsect = bfd_abs_section_ptr;
  else if (strcmp (name, BFD_COM_SECTION_NAME) == 0)
    newsect = bfd_com_section_ptr;
  else if (strcmp (name, BFD_UND_SECTION_NAME) == 0)
    newsect = bfd_und_section_ptr;
  else if (strcmp (name, BFD_IND_SECTION_NAME) == 0)
    newsect = bfd_ind_section_ptr;
  else
    {
      struct section_hash_entry *sh;

      sh = section_hash_lookup (&abfd->section_htab, name, true, false);
      if (sh == NULL)
	return NULL;

      newsect = &sh->section;
      if (newsect->name != NULL)
	{
	  /* Section already exists.  */
	  return newsect;
	}

      newsect->name = name;
      return bfd_section_init (abfd, newsect);
    }

  /* Call new_section_hook when "creating" the standard abs, com, und
     and ind sections to tack on format specific section data.
     Also, create a proper section symbol.  */
  if (! BFD_SEND (abfd, _new_section_hook, (abfd, newsect)))
    return NULL;
  return newsect;
}

/*
FUNCTION
	bfd_make_section_anyway_with_flags

SYNOPSIS
	asection *bfd_make_section_anyway_with_flags
	  (bfd *abfd, const char *name, flagword flags);

DESCRIPTION
   Create a new empty section called @var{name} and attach it to the end of
   the chain of sections for @var{abfd}.  Create a new section even if there
   is already a section with that name.  Also set the attributes of the
   new section to the value @var{flags}.

   Return <<NULL>> and set <<bfd_error>> on error; possible errors are:
   o <<bfd_error_invalid_operation>> - If output has already started for @var{abfd}.
   o <<bfd_error_no_memory>> - If memory allocation fails.
*/

sec_ptr
bfd_make_section_anyway_with_flags (bfd *abfd, const char *name,
				    flagword flags)
{
  struct section_hash_entry *sh;
  asection *newsect;

  if (abfd->output_has_begun)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return NULL;
    }

  sh = section_hash_lookup (&abfd->section_htab, name, true, false);
  if (sh == NULL)
    return NULL;

  newsect = &sh->section;
  if (newsect->name != NULL)
    {
      /* We are making a section of the same name.  Put it in the
	 section hash table.  Even though we can't find it directly by a
	 hash lookup, we'll be able to find the section by traversing
	 sh->root.next quicker than looking at all the bfd sections.  */
      struct section_hash_entry *new_sh;
      new_sh = (struct section_hash_entry *)
	bfd_section_hash_newfunc (NULL, &abfd->section_htab, name);
      if (new_sh == NULL)
	return NULL;

      new_sh->root = sh->root;
      sh->root.next = &new_sh->root;
      newsect = &new_sh->section;
    }

  newsect->flags = flags;
  newsect->name = name;
  return bfd_section_init (abfd, newsect);
}

/*
FUNCTION
	bfd_make_section_anyway

SYNOPSIS
	asection *bfd_make_section_anyway (bfd *abfd, const char *name);

DESCRIPTION
   Create a new empty section called @var{name} and attach it to the end of
   the chain of sections for @var{abfd}.  Create a new section even if there
   is already a section with that name.

   Return <<NULL>> and set <<bfd_error>> on error; possible errors are:
   o <<bfd_error_invalid_operation>> - If output has already started for @var{abfd}.
   o <<bfd_error_no_memory>> - If memory allocation fails.
*/

sec_ptr
bfd_make_section_anyway (bfd *abfd, const char *name)
{
  return bfd_make_section_anyway_with_flags (abfd, name, 0);
}

/*
FUNCTION
	bfd_make_section_with_flags

SYNOPSIS
	asection *bfd_make_section_with_flags
	  (bfd *, const char *name, flagword flags);

DESCRIPTION
   Like <<bfd_make_section_anyway>>, but return <<NULL>> (without calling
   bfd_set_error ()) without changing the section chain if there is already a
   section named @var{name}.  Also set the attributes of the new section to
   the value @var{flags}.  If there is an error, return <<NULL>> and set
   <<bfd_error>>.
*/

asection *
bfd_make_section_with_flags (bfd *abfd, const char *name,
			     flagword flags)
{
  struct section_hash_entry *sh;
  asection *newsect;

  if (abfd == NULL || name == NULL || abfd->output_has_begun)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return NULL;
    }

  if (strcmp (name, BFD_ABS_SECTION_NAME) == 0
      || strcmp (name, BFD_COM_SECTION_NAME) == 0
      || strcmp (name, BFD_UND_SECTION_NAME) == 0
      || strcmp (name, BFD_IND_SECTION_NAME) == 0)
    return NULL;

  sh = section_hash_lookup (&abfd->section_htab, name, true, false);
  if (sh == NULL)
    return NULL;

  newsect = &sh->section;
  if (newsect->name != NULL)
    {
      /* Section already exists.  */
      return NULL;
    }

  newsect->name = name;
  newsect->flags = flags;
  return bfd_section_init (abfd, newsect);
}

/*
FUNCTION
	bfd_make_section

SYNOPSIS
	asection *bfd_make_section (bfd *, const char *name);

DESCRIPTION
   Like <<bfd_make_section_anyway>>, but return <<NULL>> (without calling
   bfd_set_error ()) without changing the section chain if there is already a
   section named @var{name}.  If there is an error, return <<NULL>> and set
   <<bfd_error>>.
*/

asection *
bfd_make_section (bfd *abfd, const char *name)
{
  return bfd_make_section_with_flags (abfd, name, 0);
}

/*
FUNCTION
	bfd_set_section_flags

SYNOPSIS
	bool bfd_set_section_flags (asection *sec, flagword flags);

DESCRIPTION
	Set the attributes of the section @var{sec} to the value @var{flags}.
	Return <<TRUE>> on success, <<FALSE>> on error.  Possible error
	returns are:

	o <<bfd_error_invalid_operation>> -
	The section cannot have one or more of the attributes
	requested. For example, a .bss section in <<a.out>> may not
	have the <<SEC_HAS_CONTENTS>> field set.

*/

bool
bfd_set_section_flags (asection *section, flagword flags)
{
  section->flags = flags;
  return true;
}

/*
FUNCTION
	bfd_rename_section

SYNOPSIS
	void bfd_rename_section
	  (asection *sec, const char *newname);

DESCRIPTION
	Rename section @var{sec} to @var{newname}.
*/

void
bfd_rename_section (asection *sec, const char *newname)
{
  struct section_hash_entry *sh;

  sh = (struct section_hash_entry *)
    ((char *) sec - offsetof (struct section_hash_entry, section));
  sh->section.name = newname;
  bfd_hash_rename (&sec->owner->section_htab, newname, &sh->root);
}

/*
FUNCTION
	bfd_map_over_sections

SYNOPSIS
	void bfd_map_over_sections
	  (bfd *abfd,
	   void (*func) (bfd *abfd, asection *sect, void *obj),
	   void *obj);

DESCRIPTION
	Call the provided function @var{func} for each section
	attached to the BFD @var{abfd}, passing @var{obj} as an
	argument. The function will be called as if by

|	func (abfd, the_section, obj);

	This is the preferred method for iterating over sections; an
	alternative would be to use a loop:

|	   asection *p;
|	   for (p = abfd->sections; p != NULL; p = p->next)
|	      func (abfd, p, ...)

*/

void
bfd_map_over_sections (bfd *abfd,
		       void (*operation) (bfd *, asection *, void *),
		       void *user_storage)
{
  asection *sect;
  unsigned int i = 0;

  for (sect = abfd->sections; sect != NULL; i++, sect = sect->next)
    (*operation) (abfd, sect, user_storage);

  if (i != abfd->section_count)	/* Debugging */
    abort ();
}

/*
FUNCTION
	bfd_sections_find_if

SYNOPSIS
	asection *bfd_sections_find_if
	  (bfd *abfd,
	   bool (*operation) (bfd *abfd, asection *sect, void *obj),
	   void *obj);

DESCRIPTION
	Call the provided function @var{operation} for each section
	attached to the BFD @var{abfd}, passing @var{obj} as an
	argument. The function will be called as if by

|	operation (abfd, the_section, obj);

	It returns the first section for which @var{operation} returns true.

*/

asection *
bfd_sections_find_if (bfd *abfd,
		      bool (*operation) (bfd *, asection *, void *),
		      void *user_storage)
{
  asection *sect;

  for (sect = abfd->sections; sect != NULL; sect = sect->next)
    if ((*operation) (abfd, sect, user_storage))
      break;

  return sect;
}

/*
FUNCTION
	bfd_set_section_size

SYNOPSIS
	bool bfd_set_section_size (asection *sec, bfd_size_type val);

DESCRIPTION
	Set @var{sec} to the size @var{val}. If the operation is
	ok, then <<TRUE>> is returned, else <<FALSE>>.

	Possible error returns:
	o <<bfd_error_invalid_operation>> -
	Writing has started to the BFD, so setting the size is invalid.

*/

bool
bfd_set_section_size (asection *sec, bfd_size_type val)
{
  /* Once you've started writing to any section you cannot create or change
     the size of any others.  */

  if (sec->owner == NULL || sec->owner->output_has_begun)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return false;
    }

  sec->size = val;
  return true;
}

/*
FUNCTION
	bfd_set_section_contents

SYNOPSIS
	bool bfd_set_section_contents
	  (bfd *abfd, asection *section, const void *data,
	   file_ptr offset, bfd_size_type count);

DESCRIPTION
	Sets the contents of the section @var{section} in BFD
	@var{abfd} to the data starting in memory at @var{location}.
	The data is written to the output section starting at offset
	@var{offset} for @var{count} octets.

	Normally <<TRUE>> is returned, but <<FALSE>> is returned if
	there was an error.  Possible error returns are:
	o <<bfd_error_no_contents>> -
	The output section does not have the <<SEC_HAS_CONTENTS>>
	attribute, so nothing can be written to it.
	o <<bfd_error_bad_value>> -
	The section is unable to contain all of the data.
	o <<bfd_error_invalid_operation>> -
	The BFD is not writeable.
	o and some more too.

	This routine is front end to the back end function
	<<_bfd_set_section_contents>>.

*/

bool
bfd_set_section_contents (bfd *abfd,
			  sec_ptr section,
			  const void *location,
			  file_ptr offset,
			  bfd_size_type count)
{
  bfd_size_type sz;

  if (!(bfd_section_flags (section) & SEC_HAS_CONTENTS))
    {
      bfd_set_error (bfd_error_no_contents);
      return false;
    }

  sz = section->size;
  if ((bfd_size_type) offset > sz
      || count > sz - offset
      || count != (size_t) count)
    {
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  if (!bfd_write_p (abfd))
    {
      bfd_set_error (bfd_error_invalid_operation);
      return false;
    }

  /* Record a copy of the data in memory if desired.  */
  if (section->contents
      && location != section->contents + offset)
    memcpy (section->contents + offset, location, (size_t) count);

  if (BFD_SEND (abfd, _bfd_set_section_contents,
		(abfd, section, location, offset, count)))
    {
      abfd->output_has_begun = true;
      return true;
    }

  return false;
}

/*
FUNCTION
	bfd_get_section_contents

SYNOPSIS
	bool bfd_get_section_contents
	  (bfd *abfd, asection *section, void *location, file_ptr offset,
	   bfd_size_type count);

DESCRIPTION
	Read data from @var{section} in BFD @var{abfd}
	into memory starting at @var{location}. The data is read at an
	offset of @var{offset} from the start of the input section,
	and is read for @var{count} bytes.

	If the contents of a constructor with the <<SEC_CONSTRUCTOR>>
	flag set are requested or if the section does not have the
	<<SEC_HAS_CONTENTS>> flag set, then the @var{location} is filled
	with zeroes. If no errors occur, <<TRUE>> is returned, else
	<<FALSE>>.

*/
bool
bfd_get_section_contents (bfd *abfd,
			  sec_ptr section,
			  void *location,
			  file_ptr offset,
			  bfd_size_type count)
{
  bfd_size_type sz;

  if (section->flags & SEC_CONSTRUCTOR)
    {
      memset (location, 0, (size_t) count);
      return true;
    }

  sz = bfd_get_section_limit_octets (abfd, section);
  if ((bfd_size_type) offset > sz
      || count > sz - offset
      || count != (size_t) count)
    {
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  if (count == 0)
    /* Don't bother.  */
    return true;

  if ((section->flags & SEC_HAS_CONTENTS) == 0)
    {
      memset (location, 0, (size_t) count);
      return true;
    }

  if ((section->flags & SEC_IN_MEMORY) != 0)
    {
      if (section->contents == NULL)
	{
	  /* This can happen because of errors earlier on in the linking process.
	     We do not want to seg-fault here, so clear the flag and return an
	     error code.  */
	  section->flags &= ~ SEC_IN_MEMORY;
	  bfd_set_error (bfd_error_invalid_operation);
	  return false;
	}

      memmove (location, section->contents + offset, (size_t) count);
      return true;
    }

  return BFD_SEND (abfd, _bfd_get_section_contents,
		   (abfd, section, location, offset, count));
}

/*
FUNCTION
	bfd_malloc_and_get_section

SYNOPSIS
	bool bfd_malloc_and_get_section
	  (bfd *abfd, asection *section, bfd_byte **buf);

DESCRIPTION
	Read all data from @var{section} in BFD @var{abfd}
	into a buffer, *@var{buf}, malloc'd by this function.
	Return @code{true} on success, @code{false} on failure in which
	case *@var{buf} will be NULL.
*/

bool
bfd_malloc_and_get_section (bfd *abfd, sec_ptr sec, bfd_byte **buf)
{
  *buf = NULL;
  return bfd_get_full_section_contents (abfd, sec, buf);
}
/*
FUNCTION
	bfd_copy_private_section_data

SYNOPSIS
	bool bfd_copy_private_section_data
	  (bfd *ibfd, asection *isec, bfd *obfd, asection *osec);

DESCRIPTION
	Copy private section information from @var{isec} in the BFD
	@var{ibfd} to the section @var{osec} in the BFD @var{obfd}.
	Return <<TRUE>> on success, <<FALSE>> on error.  Possible error
	returns are:

	o <<bfd_error_no_memory>> -
	Not enough memory exists to create private data for @var{osec}.

.#define bfd_copy_private_section_data(ibfd, isection, obfd, osection) \
.	BFD_SEND (obfd, _bfd_copy_private_section_data, \
.		  (ibfd, isection, obfd, osection))
*/

/*
FUNCTION
	bfd_generic_is_group_section

SYNOPSIS
	bool bfd_generic_is_group_section (bfd *, const asection *sec);

DESCRIPTION
	Returns TRUE if @var{sec} is a member of a group.
*/

bool
bfd_generic_is_group_section (bfd *abfd ATTRIBUTE_UNUSED,
			      const asection *sec ATTRIBUTE_UNUSED)
{
  return false;
}

/*
FUNCTION
	bfd_generic_group_name

SYNOPSIS
	const char *bfd_generic_group_name (bfd *, const asection *sec);

DESCRIPTION
	Returns group name if @var{sec} is a member of a group.
*/

const char *
bfd_generic_group_name (bfd *abfd ATTRIBUTE_UNUSED,
			const asection *sec ATTRIBUTE_UNUSED)
{
  return NULL;
}

/*
FUNCTION
	bfd_generic_discard_group

SYNOPSIS
	bool bfd_generic_discard_group (bfd *abfd, asection *group);

DESCRIPTION
	Remove all members of @var{group} from the output.
*/

bool
bfd_generic_discard_group (bfd *abfd ATTRIBUTE_UNUSED,
			   asection *group ATTRIBUTE_UNUSED)
{
  return true;
}

bool
_bfd_nowrite_set_section_contents (bfd *abfd,
				   sec_ptr section ATTRIBUTE_UNUSED,
				   const void *location ATTRIBUTE_UNUSED,
				   file_ptr offset ATTRIBUTE_UNUSED,
				   bfd_size_type count ATTRIBUTE_UNUSED)
{
  return _bfd_bool_bfd_false_error (abfd);
}

/*
INTERNAL_FUNCTION
	_bfd_section_size_insane

SYNOPSIS
	bool _bfd_section_size_insane (bfd *abfd, asection *sec);

DESCRIPTION
	Returns true if the given section has a size that indicates
	it cannot be read from file.  Return false if the size is OK
	*or* this function can't say one way or the other.

*/

bool
_bfd_section_size_insane (bfd *abfd, asection *sec)
{
  bfd_size_type size = bfd_get_section_limit_octets (abfd, sec);
  if (size == 0)
    return false;

  if ((bfd_section_flags (sec) & SEC_IN_MEMORY) != 0
      /* PR 24753: Linker created sections can be larger than
	 the file size, eg if they are being used to hold stubs.  */
      || (bfd_section_flags (sec) & SEC_LINKER_CREATED) != 0
      /* PR 24753: Sections which have no content should also be
	 excluded as they contain no size on disk.  */
      || (bfd_section_flags (sec) & SEC_HAS_CONTENTS) == 0
      /* The MMO file format supports its own special compression
	 technique, but it uses COMPRESS_SECTION_NONE when loading
	 a section's contents.  */
      || bfd_get_flavour (abfd) == bfd_target_mmo_flavour)
    return false;

  ufile_ptr filesize = bfd_get_file_size (abfd);
  if (filesize == 0)
    return false;

  if (sec->compress_status == DECOMPRESS_SECTION_ZSTD
      || sec->compress_status == DECOMPRESS_SECTION_ZLIB)
    {
      /* PR26946, PR28834: Sanity check compress header uncompressed
	 size against the original file size, and check that the
	 compressed section can be read from file.  We choose an
	 arbitrary uncompressed size of 10x the file size, rather than
	 a compress ratio.  The reason being that compiling
	 "int aaa..a;" with "a" repeated enough times can result in
	 compression ratios without limit for .debug_str, whereas such
	 a file will usually also have the enormous symbol
	 uncompressed in .symtab.  */
     if (size / 10 > filesize)
       {
	 bfd_set_error (bfd_error_bad_value);
	 return true;
       }
     size = sec->compressed_size;
    }

  if ((ufile_ptr) sec->filepos > filesize || size > filesize - sec->filepos)
    {
      bfd_set_error (bfd_error_file_truncated);
      return true;
    }
  return false;
}
