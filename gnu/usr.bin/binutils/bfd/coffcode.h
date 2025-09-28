/* Support for the generic parts of most COFF variants, for BFD.
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

/* Most of this hacked by  Steve Chamberlain,
			sac@cygnus.com.  */
/*
SECTION
	coff backends

	BFD supports a number of different flavours of coff format.
	The major differences between formats are the sizes and
	alignments of fields in structures on disk, and the occasional
	extra field.

	Coff in all its varieties is implemented with a few common
	files and a number of implementation specific files. For
	example, the i386 coff format is implemented in the file
	@file{coff-i386.c}.  This file @code{#include}s
	@file{coff/i386.h} which defines the external structure of the
	coff format for the i386, and @file{coff/internal.h} which
	defines the internal structure. @file{coff-i386.c} also
	defines the relocations used by the i386 coff format
	@xref{Relocations}.

SUBSECTION
	Porting to a new version of coff

	The recommended method is to select from the existing
	implementations the version of coff which is most like the one
	you want to use.  For example, we'll say that i386 coff is
	the one you select, and that your coff flavour is called foo.
	Copy @file{i386coff.c} to @file{foocoff.c}, copy
	@file{../include/coff/i386.h} to @file{../include/coff/foo.h},
	and add the lines to @file{targets.c} and @file{Makefile.in}
	so that your new back end is used. Alter the shapes of the
	structures in @file{../include/coff/foo.h} so that they match
	what you need. You will probably also have to add
	@code{#ifdef}s to the code in @file{coff/internal.h} and
	@file{coffcode.h} if your version of coff is too wild.

	You can verify that your new BFD backend works quite simply by
	building @file{objdump} from the @file{binutils} directory,
	and making sure that its version of what's going on and your
	host system's idea (assuming it has the pretty standard coff
	dump utility, usually called @code{att-dump} or just
	@code{dump}) are the same.  Then clean up your code, and send
	what you've done to Cygnus. Then your stuff will be in the
	next release, and you won't have to keep integrating it.

SUBSECTION
	How the coff backend works

SUBSUBSECTION
	File layout

	The Coff backend is split into generic routines that are
	applicable to any Coff target and routines that are specific
	to a particular target.  The target-specific routines are
	further split into ones which are basically the same for all
	Coff targets except that they use the external symbol format
	or use different values for certain constants.

	The generic routines are in @file{coffgen.c}.  These routines
	work for any Coff target.  They use some hooks into the target
	specific code; the hooks are in a @code{bfd_coff_backend_data}
	structure, one of which exists for each target.

	The essentially similar target-specific routines are in
	@file{coffcode.h}.  This header file includes executable C code.
	The various Coff targets first include the appropriate Coff
	header file, make any special defines that are needed, and
	then include @file{coffcode.h}.

	Some of the Coff targets then also have additional routines in
	the target source file itself.

SUBSUBSECTION
	Coff long section names

	In the standard Coff object format, section names are limited to
	the eight bytes available in the @code{s_name} field of the
	@code{SCNHDR} section header structure.  The format requires the
	field to be NUL-padded, but not necessarily NUL-terminated, so
	the longest section names permitted are a full eight characters.

	The Microsoft PE variants of the Coff object file format add
	an extension to support the use of long section names.  This
	extension is defined in section 4 of the Microsoft PE/COFF
	specification (rev 8.1).  If a section name is too long to fit
	into the section header's @code{s_name} field, it is instead
	placed into the string table, and the @code{s_name} field is
	filled with a slash ("/") followed by the ASCII decimal
	representation of the offset of the full name relative to the
	string table base.

	Note that this implies that the extension can only be used in object
	files, as executables do not contain a string table.  The standard
	specifies that long section names from objects emitted into executable
	images are to be truncated.

	However, as a GNU extension, BFD can generate executable images
	that contain a string table and long section names.  This
	would appear to be technically valid, as the standard only says
	that Coff debugging information is deprecated, not forbidden,
	and in practice it works, although some tools that parse PE files
	expecting the MS standard format may become confused; @file{PEview} is
	one known example.

	The functionality is supported in BFD by code implemented under
	the control of the macro @code{COFF_LONG_SECTION_NAMES}.  If not
	defined, the format does not support long section names in any way.
	If defined, it is used to initialise a flag,
	@code{_bfd_coff_long_section_names}, and a hook function pointer,
	@code{_bfd_coff_set_long_section_names}, in the Coff backend data
	structure.  The flag controls the generation of long section names
	in output BFDs at runtime; if it is false, as it will be by default
	when generating an executable image, long section names are truncated;
	if true, the long section names extension is employed.  The hook
	points to a function that allows the value of a copy of the flag
	in coff object tdata to be altered at runtime, on formats that
	support long section names at all; on other formats it points
	to a stub that returns an error indication.

	With input BFDs, the flag is set according to whether any long section
	names are detected while reading the section headers.  For a completely
	new BFD, the flag is set to the default for the target format.  This
	information can be used by a client of the BFD library when deciding
	what output format to generate, and means that a BFD that is opened
	for read and subsequently converted to a writeable BFD and modified
	in-place will retain whatever format it had on input.

	If @code{COFF_LONG_SECTION_NAMES} is simply defined (blank), or is
	defined to the value "1", then long section names are enabled by
	default; if it is defined to the value zero, they are disabled by
	default (but still accepted in input BFDs).  The header @file{coffcode.h}
	defines a macro, @code{COFF_DEFAULT_LONG_SECTION_NAMES}, which is
	used in the backends to initialise the backend data structure fields
	appropriately; see the comments for further detail.

SUBSUBSECTION
	Bit twiddling

	Each flavour of coff supported in BFD has its own header file
	describing the external layout of the structures. There is also
	an internal description of the coff layout, in
	@file{coff/internal.h}. A major function of the
	coff backend is swapping the bytes and twiddling the bits to
	translate the external form of the structures into the normal
	internal form. This is all performed in the
	@code{bfd_swap}_@i{thing}_@i{direction} routines. Some
	elements are different sizes between different versions of
	coff; it is the duty of the coff version specific include file
	to override the definitions of various packing routines in
	@file{coffcode.h}. E.g., the size of line number entry in coff is
	sometimes 16 bits, and sometimes 32 bits. @code{#define}ing
	@code{PUT_LNSZ_LNNO} and @code{GET_LNSZ_LNNO} will select the
	correct one. No doubt, some day someone will find a version of
	coff which has a varying field size not catered to at the
	moment. To port BFD, that person will have to add more @code{#defines}.
	Three of the bit twiddling routines are exported to
	@code{gdb}; @code{coff_swap_aux_in}, @code{coff_swap_sym_in}
	and @code{coff_swap_lineno_in}. @code{GDB} reads the symbol
	table on its own, but uses BFD to fix things up.  More of the
	bit twiddlers are exported for @code{gas};
	@code{coff_swap_aux_out}, @code{coff_swap_sym_out},
	@code{coff_swap_lineno_out}, @code{coff_swap_reloc_out},
	@code{coff_swap_filehdr_out}, @code{coff_swap_aouthdr_out},
	@code{coff_swap_scnhdr_out}. @code{Gas} currently keeps track
	of all the symbol table and reloc drudgery itself, thereby
	saving the internal BFD overhead, but uses BFD to swap things
	on the way out, making cross ports much safer.  Doing so also
	allows BFD (and thus the linker) to use the same header files
	as @code{gas}, which makes one avenue to disaster disappear.

SUBSUBSECTION
	Symbol reading

	The simple canonical form for symbols used by BFD is not rich
	enough to keep all the information available in a coff symbol
	table. The back end gets around this problem by keeping the original
	symbol table around, "behind the scenes".

	When a symbol table is requested (through a call to
	@code{bfd_canonicalize_symtab}), a request gets through to
	@code{coff_get_normalized_symtab}. This reads the symbol table from
	the coff file and swaps all the structures inside into the
	internal form. It also fixes up all the pointers in the table
	(represented in the file by offsets from the first symbol in
	the table) into physical pointers to elements in the new
	internal table. This involves some work since the meanings of
	fields change depending upon context: a field that is a
	pointer to another structure in the symbol table at one moment
	may be the size in bytes of a structure at the next.  Another
	pass is made over the table. All symbols which mark file names
	(<<C_FILE>> symbols) are modified so that the internal
	string points to the value in the auxent (the real filename)
	rather than the normal text associated with the symbol
	(@code{".file"}).

	At this time the symbol names are moved around. Coff stores
	all symbols less than nine characters long physically
	within the symbol table; longer strings are kept at the end of
	the file in the string table. This pass moves all strings
	into memory and replaces them with pointers to the strings.

	The symbol table is massaged once again, this time to create
	the canonical table used by the BFD application. Each symbol
	is inspected in turn, and a decision made (using the
	@code{sclass} field) about the various flags to set in the
	@code{asymbol}.  @xref{Symbols}. The generated canonical table
	shares strings with the hidden internal symbol table.

	Any linenumbers are read from the coff file too, and attached
	to the symbols which own the functions the linenumbers belong to.

SUBSUBSECTION
	Symbol writing

	Writing a symbol to a coff file which didn't come from a coff
	file will lose any debugging information. The @code{asymbol}
	structure remembers the BFD from which the symbol was taken, and on
	output the back end makes sure that the same destination target as
	source target is present.

	When the symbols have come from a coff file then all the
	debugging information is preserved.

	Symbol tables are provided for writing to the back end in a
	vector of pointers to pointers. This allows applications like
	the linker to accumulate and output large symbol tables
	without having to do too much byte copying.

	This function runs through the provided symbol table and
	patches each symbol marked as a file place holder
	(@code{C_FILE}) to point to the next file place holder in the
	list. It also marks each @code{offset} field in the list with
	the offset from the first symbol of the current symbol.

	Another function of this procedure is to turn the canonical
	value form of BFD into the form used by coff. Internally, BFD
	expects symbol values to be offsets from a section base; so a
	symbol physically at 0x120, but in a section starting at
	0x100, would have the value 0x20. Coff expects symbols to
	contain their final value, so symbols have their values
	changed at this point to reflect their sum with their owning
	section.  This transformation uses the
	<<output_section>> field of the @code{asymbol}'s
	@code{asection} @xref{Sections}.

	o <<coff_mangle_symbols>>

	This routine runs though the provided symbol table and uses
	the offsets generated by the previous pass and the pointers
	generated when the symbol table was read in to create the
	structured hierarchy required by coff. It changes each pointer
	to a symbol into the index into the symbol table of the asymbol.

	o <<coff_write_symbols>>

	This routine runs through the symbol table and patches up the
	symbols from their internal form into the coff way, calls the
	bit twiddlers, and writes out the table to the file.

*/

/*
INTERNAL_DEFINITION
	coff_symbol_type

DESCRIPTION
	The hidden information for an <<asymbol>> is described in a
	<<combined_entry_type>>:

CODE_FRAGMENT
.typedef struct coff_ptr_struct
.{
.  {* Remembers the offset from the first symbol in the file for
.     this symbol.  Generated by coff_renumber_symbols.  *}
.  unsigned int offset;
.
.  {* Selects between the elements of the union below.  *}
.  unsigned int is_sym : 1;
.
.  {* Selects between the elements of the x_sym.x_tagndx union.  If set,
.     p is valid and the field will be renumbered.  *}
.  unsigned int fix_tag : 1;
.
.  {* Selects between the elements of the x_sym.x_fcnary.x_fcn.x_endndx
.     union.  If set, p is valid and the field will be renumbered.  *}
.  unsigned int fix_end : 1;
.
.  {* Selects between the elements of the x_csect.x_scnlen union.  If set,
.     p is valid and the field will be renumbered.  *}
.  unsigned int fix_scnlen : 1;
.
.  {* If set, u.syment.n_value contains a pointer to a symbol.  The final
.     value will be the offset field.  Used for XCOFF C_BSTAT symbols.  *}
.  unsigned int fix_value : 1;
.
.  {* If set, u.syment.n_value is an index into the line number entries.
.     Used for XCOFF C_BINCL/C_EINCL symbols.  *}
.  unsigned int fix_line : 1;
.
.  {* The container for the symbol structure as read and translated
.     from the file.  *}
.  union
.  {
.    union internal_auxent auxent;
.    struct internal_syment syment;
.  } u;
.
. {* An extra pointer which can used by format based on COFF (like XCOFF)
.    to provide extra information to their backend.  *}
. void *extrap;
.} combined_entry_type;
.
.{* Each canonical asymbol really looks like this: *}
.
.typedef struct coff_symbol_struct
.{
.  {* The actual symbol which the rest of BFD works with *}
.  asymbol symbol;
.
.  {* A pointer to the hidden information for this symbol *}
.  combined_entry_type *native;
.
.  {* A pointer to the linenumber information for this symbol *}
.  struct lineno_cache_entry *lineno;
.
.  {* Have the line numbers been relocated yet ? *}
.  bool done_lineno;
.} coff_symbol_type;
.
*/

#include "libiberty.h"

#ifdef COFF_WITH_PE
#include "peicode.h"
#else
#include "coffswap.h"
#endif

#define STRING_SIZE_SIZE 4

#define DOT_DEBUG	".debug"
#define DOT_ZDEBUG	".zdebug"
#define GNU_LINKONCE_WI ".gnu.linkonce.wi."
#define GNU_LINKONCE_WT ".gnu.linkonce.wt."
#define DOT_RELOC	".reloc"

#if defined(COFF_WITH_PE) || defined(COFF_GO32_EXE) || defined(COFF_GO32)
# define COFF_WITH_EXTENDED_RELOC_COUNTER
#endif

#if defined (COFF_LONG_SECTION_NAMES)
/* Needed to expand the inputs to BLANKOR1TOODD.  */
#define COFFLONGSECTIONCATHELPER(x,y)    x ## y
/* If the input macro Y is blank or '1', return an odd number; if it is
   '0', return an even number.  Result undefined in all other cases.  */
#define BLANKOR1TOODD(y)		 COFFLONGSECTIONCATHELPER(1,y)
/* Defined to numerical 0 or 1 according to whether generation of long
   section names is disabled or enabled by default.  */
#define COFF_ENABLE_LONG_SECTION_NAMES   (BLANKOR1TOODD(COFF_LONG_SECTION_NAMES) & 1)
/* Where long section names are supported, we allow them to be enabled
   and disabled at runtime, so select an appropriate hook function for
   _bfd_coff_set_long_section_names.  */
#define COFF_LONG_SECTION_NAMES_SETTER   bfd_coff_set_long_section_names_allowed
#else /* !defined (COFF_LONG_SECTION_NAMES) */
/* If long section names are not supported, this stub disallows any
   attempt to enable them at run-time.  */
#define COFF_LONG_SECTION_NAMES_SETTER   bfd_coff_set_long_section_names_disallowed
#endif /* defined (COFF_LONG_SECTION_NAMES) */

/* Define a macro that can be used to initialise both the fields relating
   to long section names in the backend data struct simultaneously.  */
#if COFF_ENABLE_LONG_SECTION_NAMES
#define COFF_DEFAULT_LONG_SECTION_NAMES  (true), COFF_LONG_SECTION_NAMES_SETTER
#else /* !COFF_ENABLE_LONG_SECTION_NAMES */
#define COFF_DEFAULT_LONG_SECTION_NAMES  (false), COFF_LONG_SECTION_NAMES_SETTER
#endif /* COFF_ENABLE_LONG_SECTION_NAMES */

static enum coff_symbol_classification coff_classify_symbol
  (bfd *, struct internal_syment *);

/* void warning(); */

#if defined (COFF_LONG_SECTION_NAMES)
static bool
bfd_coff_set_long_section_names_allowed (bfd *abfd, int enable)
{
  bfd_coff_long_section_names (abfd) = enable;
  return true;
}
#else /* !defined (COFF_LONG_SECTION_NAMES) */
static bool
bfd_coff_set_long_section_names_disallowed (bfd *abfd ATTRIBUTE_UNUSED,
					    int enable ATTRIBUTE_UNUSED)
{
  return false;
}
#endif /* defined (COFF_LONG_SECTION_NAMES) */

/* Return a word with STYP_* (scnhdr.s_flags) flags set to represent
   the incoming SEC_* flags.  The inverse of this function is
   styp_to_sec_flags().  NOTE: If you add to/change this routine, you
   should probably mirror the changes in styp_to_sec_flags().  */

#ifndef COFF_WITH_PE

/* Macros for setting debugging flags.  */

#ifdef STYP_DEBUG
#define STYP_XCOFF_DEBUG STYP_DEBUG
#else
#define STYP_XCOFF_DEBUG STYP_INFO
#endif

#ifdef COFF_ALIGN_IN_S_FLAGS
#define STYP_DEBUG_INFO STYP_DSECT
#else
#define STYP_DEBUG_INFO STYP_INFO
#endif

static long
sec_to_styp_flags (const char *sec_name, flagword sec_flags)
{
  long styp_flags = 0;

  if (!strcmp (sec_name, _TEXT))
    {
      styp_flags = STYP_TEXT;
    }
  else if (!strcmp (sec_name, _DATA))
    {
      styp_flags = STYP_DATA;
    }
  else if (!strcmp (sec_name, _BSS))
    {
      styp_flags = STYP_BSS;
#ifdef _COMMENT
    }
  else if (!strcmp (sec_name, _COMMENT))
    {
      styp_flags = STYP_INFO;
#endif /* _COMMENT */
#ifdef _LIB
    }
  else if (!strcmp (sec_name, _LIB))
    {
      styp_flags = STYP_LIB;
#endif /* _LIB */
#ifdef _LIT
    }
  else if (!strcmp (sec_name, _LIT))
    {
      styp_flags = STYP_LIT;
#endif /* _LIT */
    }
  else if (startswith (sec_name, DOT_DEBUG)
	   || startswith (sec_name, DOT_ZDEBUG))
    {
      /* Handle the XCOFF debug section and DWARF2 debug sections.  */
      if (!sec_name[6])
	styp_flags = STYP_XCOFF_DEBUG;
      else
	styp_flags = STYP_DEBUG_INFO;
    }
  else if (startswith (sec_name, ".stab"))
    {
      styp_flags = STYP_DEBUG_INFO;
    }
#ifdef COFF_LONG_SECTION_NAMES
  else if (startswith (sec_name, GNU_LINKONCE_WI)
	   || startswith (sec_name, GNU_LINKONCE_WT))
    {
      styp_flags = STYP_DEBUG_INFO;
    }
#endif
#ifdef RS6000COFF_C
  else if (!strcmp (sec_name, _TDATA))
    {
      styp_flags = STYP_TDATA;
    }
  else if (!strcmp (sec_name, _TBSS))
    {
      styp_flags = STYP_TBSS;
    }
  else if (!strcmp (sec_name, _PAD))
    {
      styp_flags = STYP_PAD;
    }
  else if (!strcmp (sec_name, _LOADER))
    {
      styp_flags = STYP_LOADER;
    }
  else if (!strcmp (sec_name, _EXCEPT))
    {
      styp_flags = STYP_EXCEPT;
    }
  else if (!strcmp (sec_name, _TYPCHK))
    {
      styp_flags = STYP_TYPCHK;
    }
  else if (sec_flags & SEC_DEBUGGING)
    {
      int i;

      for (i = 0; i < XCOFF_DWSECT_NBR_NAMES; i++)
	if (!strcmp (sec_name, xcoff_dwsect_names[i].xcoff_name))
	  {
	    styp_flags = STYP_DWARF | xcoff_dwsect_names[i].flag;
	    break;
	  }
    }
#endif
  /* Try and figure out what it should be */
  else if (sec_flags & SEC_CODE)
    {
      styp_flags = STYP_TEXT;
    }
  else if (sec_flags & SEC_DATA)
    {
      styp_flags = STYP_DATA;
    }
  else if (sec_flags & SEC_READONLY)
    {
#ifdef STYP_LIT			/* 29k readonly text/data section */
      styp_flags = STYP_LIT;
#else
      styp_flags = STYP_TEXT;
#endif /* STYP_LIT */
    }
  else if (sec_flags & SEC_LOAD)
    {
      styp_flags = STYP_TEXT;
    }
  else if (sec_flags & SEC_ALLOC)
    {
      styp_flags = STYP_BSS;
    }

#ifdef STYP_CLINK
  if (sec_flags & SEC_TIC54X_CLINK)
    styp_flags |= STYP_CLINK;
#endif

#ifdef STYP_BLOCK
  if (sec_flags & SEC_TIC54X_BLOCK)
    styp_flags |= STYP_BLOCK;
#endif

#ifdef STYP_NOLOAD
  if ((sec_flags & (SEC_NEVER_LOAD | SEC_COFF_SHARED_LIBRARY)) != 0)
    styp_flags |= STYP_NOLOAD;
#endif

  return styp_flags;
}

#else /* COFF_WITH_PE */

/* The PE version; see above for the general comments.  The non-PE
   case seems to be more guessing, and breaks PE format; specifically,
   .rdata is readonly, but it sure ain't text.  Really, all this
   should be set up properly in gas (or whatever assembler is in use),
   and honor whatever objcopy/strip, etc. sent us as input.  */

static long
sec_to_styp_flags (const char *sec_name, flagword sec_flags)
{
  long styp_flags = 0;
  bool is_dbg = false;

  if (startswith (sec_name, DOT_DEBUG)
      || startswith (sec_name, DOT_ZDEBUG)
#ifdef COFF_LONG_SECTION_NAMES
      || startswith (sec_name, GNU_LINKONCE_WI)
      || startswith (sec_name, GNU_LINKONCE_WT)
#endif
      || startswith (sec_name, ".stab"))
    is_dbg = true;

  /* caution: there are at least three groups of symbols that have
     very similar bits and meanings: IMAGE_SCN*, SEC_*, and STYP_*.
     SEC_* are the BFD internal flags, used for generic BFD
     information.  STYP_* are the COFF section flags which appear in
     COFF files.  IMAGE_SCN_* are the PE section flags which appear in
     PE files.  The STYP_* flags and the IMAGE_SCN_* flags overlap,
     but there are more IMAGE_SCN_* flags.  */

  /* FIXME: There is no gas syntax to specify the debug section flag.  */
  if (is_dbg)
    {
      sec_flags &= (SEC_LINK_ONCE | SEC_LINK_DUPLICATES_DISCARD
		    | SEC_LINK_DUPLICATES_SAME_CONTENTS
		    | SEC_LINK_DUPLICATES_SAME_SIZE);
      sec_flags |= SEC_DEBUGGING | SEC_READONLY;
    }

  /* skip LOAD */
  /* READONLY later */
  /* skip RELOC */
  if ((sec_flags & SEC_CODE) != 0)
    styp_flags |= IMAGE_SCN_CNT_CODE;
  if ((sec_flags & (SEC_DATA | SEC_DEBUGGING)) != 0)
    styp_flags |= IMAGE_SCN_CNT_INITIALIZED_DATA;
  if ((sec_flags & SEC_ALLOC) != 0 && (sec_flags & SEC_LOAD) == 0)
    styp_flags |= IMAGE_SCN_CNT_UNINITIALIZED_DATA;  /* ==STYP_BSS */
  /* skip ROM */
  /* skip constRUCTOR */
  /* skip CONTENTS */
#ifndef COFF_IMAGE_WITH_PE
  /* I don't think any of the IMAGE_SCN_LNK_* flags set below should be set
     when the output is PE. Only object files should have them, for the linker
     to consume.  */
  if ((sec_flags & SEC_IS_COMMON) != 0)
    styp_flags |= IMAGE_SCN_LNK_COMDAT;
#endif
  if ((sec_flags & SEC_DEBUGGING) != 0)
    styp_flags |= IMAGE_SCN_MEM_DISCARDABLE;
  if ((sec_flags & (SEC_EXCLUDE | SEC_NEVER_LOAD)) != 0 && !is_dbg)
#ifdef COFF_IMAGE_WITH_PE
    styp_flags |= IMAGE_SCN_MEM_DISCARDABLE;
#else
    styp_flags |= IMAGE_SCN_LNK_REMOVE;
#endif
  /* skip IN_MEMORY */
  /* skip SORT */
#ifndef COFF_IMAGE_WITH_PE
  if (sec_flags & SEC_LINK_ONCE)
    styp_flags |= IMAGE_SCN_LNK_COMDAT;
  if ((sec_flags
       & (SEC_LINK_DUPLICATES_DISCARD | SEC_LINK_DUPLICATES_SAME_CONTENTS
	  | SEC_LINK_DUPLICATES_SAME_SIZE)) != 0)
    styp_flags |= IMAGE_SCN_LNK_COMDAT;
#endif

  /* skip LINKER_CREATED */

  if ((sec_flags & SEC_COFF_NOREAD) == 0)
    styp_flags |= IMAGE_SCN_MEM_READ;     /* Invert NOREAD for read.  */
  if ((sec_flags & SEC_READONLY) == 0)
    styp_flags |= IMAGE_SCN_MEM_WRITE;    /* Invert READONLY for write.  */
  if (sec_flags & SEC_CODE)
    styp_flags |= IMAGE_SCN_MEM_EXECUTE;  /* CODE->EXECUTE.  */
  if (sec_flags & SEC_COFF_SHARED)
    styp_flags |= IMAGE_SCN_MEM_SHARED;   /* Shared remains meaningful.  */

  return styp_flags;
}

#endif /* COFF_WITH_PE */

/* Return a word with SEC_* flags set to represent the incoming STYP_*
   flags (from scnhdr.s_flags).  The inverse of this function is
   sec_to_styp_flags().  NOTE: If you add to/change this routine, you
   should probably mirror the changes in sec_to_styp_flags().  */

#ifndef COFF_WITH_PE

static bool
styp_to_sec_flags (bfd *abfd,
		   void * hdr,
		   const char *name,
		   asection *section ATTRIBUTE_UNUSED,
		   flagword *flags_ptr)
{
  struct internal_scnhdr *internal_s = (struct internal_scnhdr *) hdr;
  unsigned long styp_flags = internal_s->s_flags;
  flagword sec_flags = 0;

#ifdef STYP_BLOCK
  if (styp_flags & STYP_BLOCK)
    sec_flags |= SEC_TIC54X_BLOCK;
#endif

#ifdef STYP_CLINK
  if (styp_flags & STYP_CLINK)
    sec_flags |= SEC_TIC54X_CLINK;
#endif

#ifdef STYP_NOLOAD
  if (styp_flags & STYP_NOLOAD)
    sec_flags |= SEC_NEVER_LOAD;
#endif /* STYP_NOLOAD */

  /* For 386 COFF, at least, an unloadable text or data section is
     actually a shared library section.  */
  if (styp_flags & STYP_TEXT)
    {
      if (sec_flags & SEC_NEVER_LOAD)
	sec_flags |= SEC_CODE | SEC_COFF_SHARED_LIBRARY;
      else
	sec_flags |= SEC_CODE | SEC_LOAD | SEC_ALLOC;
    }
  else if (styp_flags & STYP_DATA)
    {
      if (sec_flags & SEC_NEVER_LOAD)
	sec_flags |= SEC_DATA | SEC_COFF_SHARED_LIBRARY;
      else
	sec_flags |= SEC_DATA | SEC_LOAD | SEC_ALLOC;
    }
  else if (styp_flags & STYP_BSS)
    {
#ifdef BSS_NOLOAD_IS_SHARED_LIBRARY
      if (sec_flags & SEC_NEVER_LOAD)
	sec_flags |= SEC_ALLOC | SEC_COFF_SHARED_LIBRARY;
      else
#endif
	sec_flags |= SEC_ALLOC;
    }
  else if (styp_flags & STYP_INFO)
    {
      /* We mark these as SEC_DEBUGGING, but only if COFF_PAGE_SIZE is
	 defined.  coff_compute_section_file_positions uses
	 COFF_PAGE_SIZE to ensure that the low order bits of the
	 section VMA and the file offset match.  If we don't know
	 COFF_PAGE_SIZE, we can't ensure the correct correspondence,
	 and demand page loading of the file will fail.  */
#if defined (COFF_PAGE_SIZE) && !defined (COFF_ALIGN_IN_S_FLAGS)
      sec_flags |= SEC_DEBUGGING;
#endif
    }
  else if (styp_flags & STYP_PAD)
    sec_flags = 0;
#ifdef RS6000COFF_C
  else if (styp_flags & STYP_TDATA)
    {
      if (sec_flags & SEC_NEVER_LOAD)
	sec_flags |= SEC_DATA | SEC_THREAD_LOCAL | SEC_COFF_SHARED_LIBRARY;
      else
	sec_flags |= SEC_DATA | SEC_THREAD_LOCAL | SEC_LOAD | SEC_ALLOC;
    }
  else if (styp_flags & STYP_TBSS)
    {
#ifdef BSS_NOLOAD_IS_SHARED_LIBRARY
      if (sec_flags & SEC_NEVER_LOAD)
	sec_flags |= SEC_ALLOC | SEC_THREAD_LOCAL | SEC_COFF_SHARED_LIBRARY;
      else
#endif
	sec_flags |= SEC_ALLOC | SEC_THREAD_LOCAL;
    }
  else if (styp_flags & STYP_EXCEPT)
    sec_flags |= SEC_LOAD;
  else if (styp_flags & STYP_LOADER)
    sec_flags |= SEC_LOAD;
  else if (styp_flags & STYP_TYPCHK)
    sec_flags |= SEC_LOAD;
  else if (styp_flags & STYP_DWARF)
    sec_flags |= SEC_DEBUGGING;
#endif
  else if (strcmp (name, _TEXT) == 0)
    {
      if (sec_flags & SEC_NEVER_LOAD)
	sec_flags |= SEC_CODE | SEC_COFF_SHARED_LIBRARY;
      else
	sec_flags |= SEC_CODE | SEC_LOAD | SEC_ALLOC;
    }
  else if (strcmp (name, _DATA) == 0)
    {
      if (sec_flags & SEC_NEVER_LOAD)
	sec_flags |= SEC_DATA | SEC_COFF_SHARED_LIBRARY;
      else
	sec_flags |= SEC_DATA | SEC_LOAD | SEC_ALLOC;
    }
  else if (strcmp (name, _BSS) == 0)
    {
#ifdef BSS_NOLOAD_IS_SHARED_LIBRARY
      if (sec_flags & SEC_NEVER_LOAD)
	sec_flags |= SEC_ALLOC | SEC_COFF_SHARED_LIBRARY;
      else
#endif
	sec_flags |= SEC_ALLOC;
    }
  else if (startswith (name, DOT_DEBUG)
	   || startswith (name, DOT_ZDEBUG)
#ifdef _COMMENT
	   || strcmp (name, _COMMENT) == 0
#endif
#ifdef COFF_LONG_SECTION_NAMES
	   || startswith (name, GNU_LINKONCE_WI)
	   || startswith (name, GNU_LINKONCE_WT)
#endif
	   || startswith (name, ".stab"))
    {
#ifdef COFF_PAGE_SIZE
      sec_flags |= SEC_DEBUGGING;
#endif
    }
#ifdef _LIB
  else if (strcmp (name, _LIB) == 0)
    ;
#endif
#ifdef _LIT
  else if (strcmp (name, _LIT) == 0)
    sec_flags = SEC_LOAD | SEC_ALLOC | SEC_READONLY;
#endif
  else
    sec_flags |= SEC_ALLOC | SEC_LOAD;

#ifdef STYP_LIT			/* A29k readonly text/data section type.  */
  if ((styp_flags & STYP_LIT) == STYP_LIT)
    sec_flags = (SEC_LOAD | SEC_ALLOC | SEC_READONLY);
#endif /* STYP_LIT */

#ifdef STYP_OTHER_LOAD		/* Other loaded sections.  */
  if (styp_flags & STYP_OTHER_LOAD)
    sec_flags = (SEC_LOAD | SEC_ALLOC);
#endif /* STYP_SDATA */

  if ((bfd_applicable_section_flags (abfd) & SEC_SMALL_DATA) != 0
      && (startswith (name, ".sbss")
	  || startswith (name, ".sdata")))
    sec_flags |= SEC_SMALL_DATA;

#if defined (COFF_LONG_SECTION_NAMES) && defined (COFF_SUPPORT_GNU_LINKONCE)
  /* As a GNU extension, if the name begins with .gnu.linkonce, we
     only link a single copy of the section.  This is used to support
     g++.  g++ will emit each template expansion in its own section.
     The symbols will be defined as weak, so that multiple definitions
     are permitted.  The GNU linker extension is to actually discard
     all but one of the sections.  */
  if (startswith (name, ".gnu.linkonce"))
    sec_flags |= SEC_LINK_ONCE | SEC_LINK_DUPLICATES_DISCARD;
#endif

  if (flags_ptr == NULL)
    return false;

  * flags_ptr = sec_flags;
  return true;
}

#else /* COFF_WITH_PE */

static bool
handle_COMDAT (bfd * abfd,
	       flagword *sec_flags,
	       void * hdr,
	       const char *name,
	       asection *section)
{
  struct internal_scnhdr *internal_s = (struct internal_scnhdr *) hdr;
  bfd_byte *esymstart, *esym, *esymend;
  int seen_state = 0;
  char *target_name = NULL;

  *sec_flags |= SEC_LINK_ONCE;

  /* Unfortunately, the PE format stores essential information in
     the symbol table, of all places.  We need to extract that
     information now, so that objdump and the linker will know how
     to handle the section without worrying about the symbols.  We
     can't call slurp_symtab, because the linker doesn't want the
     swapped symbols.  */

  /* COMDAT sections are special.  The first symbol is the section
     symbol, which tells what kind of COMDAT section it is.  The
     second symbol is the "comdat symbol" - the one with the
     unique name.  GNU uses the section symbol for the unique
     name; MS uses ".text" for every comdat section.  Sigh.  - DJ */

  /* This is not mirrored in sec_to_styp_flags(), but there
     doesn't seem to be a need to, either, and it would at best be
     rather messy.  */

  if (! _bfd_coff_get_external_symbols (abfd))
    return true;

  esymstart = esym = (bfd_byte *) obj_coff_external_syms (abfd);
  esymend = esym + obj_raw_syment_count (abfd) * bfd_coff_symesz (abfd);

  for (struct internal_syment isym;
       esym < esymend;
       esym += (isym.n_numaux + 1) * bfd_coff_symesz (abfd))
    {
      char buf[SYMNMLEN + 1];
      const char *symname;

      bfd_coff_swap_sym_in (abfd, esym, &isym);

      BFD_ASSERT (sizeof (internal_s->s_name) <= SYMNMLEN);

      if (isym.n_scnum == section->target_index)
	{
	  /* According to the MSVC documentation, the first
	     TWO entries with the section # are both of
	     interest to us.  The first one is the "section
	     symbol" (section name).  The second is the comdat
	     symbol name.  Here, we've found the first
	     qualifying entry; we distinguish it from the
	     second with a state flag.

	     In the case of gas-generated (at least until that
	     is fixed) .o files, it isn't necessarily the
	     second one.  It may be some other later symbol.

	     Since gas also doesn't follow MS conventions and
	     emits the section similar to .text$<name>, where
	     <something> is the name we're looking for, we
	     distinguish the two as follows:

	     If the section name is simply a section name (no
	     $) we presume it's MS-generated, and look at
	     precisely the second symbol for the comdat name.
	     If the section name has a $, we assume it's
	     gas-generated, and look for <something> (whatever
	     follows the $) as the comdat symbol.  */

	  /* All 3 branches use this.  */
	  symname = _bfd_coff_internal_syment_name (abfd, &isym, buf);

	  /* PR 17512 file: 078-11867-0.004  */
	  if (symname == NULL)
	    {
	      _bfd_error_handler (_("%pB: unable to load COMDAT section name"),
				  abfd);
	      return false;
	    }

	  switch (seen_state)
	    {
	    case 0:
	      {
		/* The first time we've seen the symbol.  */
		union internal_auxent aux;

		/* If it isn't the stuff we're expecting, die;
		   The MS documentation is vague, but it
		   appears that the second entry serves BOTH
		   as the comdat symbol and the defining
		   symbol record (either C_STAT or C_EXT,
		   possibly with an aux entry with debug
		   information if it's a function.)  It
		   appears the only way to find the second one
		   is to count.  (On Intel, they appear to be
		   adjacent, but on Alpha, they have been
		   found separated.)

		   Here, we think we've found the first one,
		   but there's some checking we can do to be
		   sure.  */

		if (! ((isym.n_sclass == C_STAT
			|| isym.n_sclass == C_EXT)
		       && BTYPE (isym.n_type) == T_NULL
		       && isym.n_value == 0))
		  {
		    /* Malformed input files can trigger this test.
		       cf PR 21781.  */
		    _bfd_error_handler (_("%pB: error: unexpected symbol '%s' in COMDAT section"),
					abfd, symname);
		    return false;
		  }

		/* FIXME LATER: MSVC generates section names
		   like .text for comdats.  Gas generates
		   names like .text$foo__Fv (in the case of a
		   function).  See comment above for more.  */

		if (isym.n_sclass == C_STAT && strcmp (name, symname) != 0)
		  /* xgettext:c-format */
		  _bfd_error_handler (_("%pB: warning: COMDAT symbol '%s'"
					" does not match section name '%s'"),
				      abfd, symname, name);

		/* This is the section symbol.  */
		seen_state = 1;
		target_name = strchr (name, '$');
		if (target_name != NULL)
		  {
		    /* Gas mode.  */
		    seen_state = 2;
		    /* Skip the `$'.  */
		    target_name += 1;
		  }

		if (isym.n_numaux == 0)
		  aux.x_scn.x_comdat = 0;
		else
		  {
		    /* PR 17512: file: e2cfe54f.  */
		    if (esym + bfd_coff_symesz (abfd) >= esymend)
		      {
			/* xgettext:c-format */
			_bfd_error_handler (_("%pB: warning: no symbol for"
					      " section '%s' found"),
					    abfd, symname);
			break;
		      }
		    bfd_coff_swap_aux_in (abfd, esym + bfd_coff_symesz (abfd),
					  isym.n_type, isym.n_sclass,
					  0, isym.n_numaux, &aux);
		  }

		/* FIXME: Microsoft uses NODUPLICATES and
		   ASSOCIATIVE, but gnu uses ANY and
		   SAME_SIZE.  Unfortunately, gnu doesn't do
		   the comdat symbols right.  So, until we can
		   fix it to do the right thing, we are
		   temporarily disabling comdats for the MS
		   types (they're used in DLLs and C++, but we
		   don't support *their* C++ libraries anyway
		   - DJ.  */

		/* Cygwin does not follow the MS style, and
		   uses ANY and SAME_SIZE where NODUPLICATES
		   and ASSOCIATIVE should be used.  For
		   Interix, we just do the right thing up
		   front.  */

		switch (aux.x_scn.x_comdat)
		  {
		  case IMAGE_COMDAT_SELECT_NODUPLICATES:
#ifdef STRICT_PE_FORMAT
		    *sec_flags |= SEC_LINK_DUPLICATES_ONE_ONLY;
#else
		    *sec_flags &= ~SEC_LINK_ONCE;
#endif
		    break;

		  case IMAGE_COMDAT_SELECT_ANY:
		    *sec_flags |= SEC_LINK_DUPLICATES_DISCARD;
		    break;

		  case IMAGE_COMDAT_SELECT_SAME_SIZE:
		    *sec_flags |= SEC_LINK_DUPLICATES_SAME_SIZE;
		    break;

		  case IMAGE_COMDAT_SELECT_EXACT_MATCH:
		    /* Not yet fully implemented ??? */
		    *sec_flags |= SEC_LINK_DUPLICATES_SAME_CONTENTS;
		    break;

		    /* debug$S gets this case; other
		       implications ??? */

		    /* There may be no symbol... we'll search
		       the whole table... Is this the right
		       place to play this game? Or should we do
		       it when reading it in.  */
		  case IMAGE_COMDAT_SELECT_ASSOCIATIVE:
#ifdef STRICT_PE_FORMAT
		    /* FIXME: This is not currently implemented.  */
		    *sec_flags |= SEC_LINK_DUPLICATES_DISCARD;
#else
		    *sec_flags &= ~SEC_LINK_ONCE;
#endif
		    break;

		  default:  /* 0 means "no symbol" */
		    /* debug$F gets this case; other
		       implications ??? */
		    *sec_flags |= SEC_LINK_DUPLICATES_DISCARD;
		    break;
		  }
	      }
	      break;

	    case 2:
	      /* Gas mode: the first matching on partial name.  */

#ifndef TARGET_UNDERSCORE
#define TARGET_UNDERSCORE 0
#endif
	      /* Is this the name we're looking for ?  */
	      if (strcmp (target_name,
			  symname + (TARGET_UNDERSCORE ? 1 : 0)) != 0)
		{
		  /* Not the name we're looking for */
		  continue;
		}
	      /* Fall through.  */
	    case 1:
	      /* MSVC mode: the lexically second symbol (or
		 drop through from the above).  */
	      {
		/* This must the second symbol with the
		   section #.  It is the actual symbol name.
		   Intel puts the two adjacent, but Alpha (at
		   least) spreads them out.  */

		struct coff_comdat_info *comdat;
		size_t len = strlen (symname) + 1;

		comdat = bfd_alloc (abfd, sizeof (*comdat) + len);
		if (comdat == NULL)
		  return false;

		coff_section_data (abfd, section)->comdat = comdat;
		comdat->symbol = (esym - esymstart) / bfd_coff_symesz (abfd);
		char *newname = (char *) (comdat + 1);
		comdat->name = newname;
		memcpy (newname, symname, len);
		return true;
	      }
	    }
	}
    }

  return true;
}


/* The PE version; see above for the general comments.

   Since to set the SEC_LINK_ONCE and associated flags, we have to
   look at the symbol table anyway, we return the symbol table index
   of the symbol being used as the COMDAT symbol.  This is admittedly
   ugly, but there's really nowhere else that we have access to the
   required information.  FIXME: Is the COMDAT symbol index used for
   any purpose other than objdump?  */

static bool
styp_to_sec_flags (bfd *abfd,
		   void * hdr,
		   const char *name,
		   asection *section,
		   flagword *flags_ptr)
{
  struct internal_scnhdr *internal_s = (struct internal_scnhdr *) hdr;
  unsigned long styp_flags = internal_s->s_flags;
  flagword sec_flags;
  bool result = true;
  bool is_dbg = false;

  if (startswith (name, DOT_DEBUG)
      || startswith (name, DOT_ZDEBUG)
#ifdef COFF_LONG_SECTION_NAMES
      || startswith (name, GNU_LINKONCE_WI)
      || startswith (name, GNU_LINKONCE_WT)
      /* FIXME: These definitions ought to be in a header file.  */
#define GNU_DEBUGLINK		".gnu_debuglink"
#define GNU_DEBUGALTLINK	".gnu_debugaltlink"
      || startswith (name, GNU_DEBUGLINK)
      || startswith (name, GNU_DEBUGALTLINK)
#endif
      || startswith (name, ".stab"))
    is_dbg = true;
  /* Assume read only unless IMAGE_SCN_MEM_WRITE is specified.  */
  sec_flags = SEC_READONLY;

  /* If section disallows read, then set the NOREAD flag. */
  if ((styp_flags & IMAGE_SCN_MEM_READ) == 0)
    sec_flags |= SEC_COFF_NOREAD;

  /* Process each flag bit in styp_flags in turn.  */
  while (styp_flags)
    {
      unsigned long flag = styp_flags & - styp_flags;
      char * unhandled = NULL;

      styp_flags &= ~ flag;

      /* We infer from the distinct read/write/execute bits the settings
	 of some of the bfd flags; the actual values, should we need them,
	 are also in pei_section_data (abfd, section)->pe_flags.  */

      switch (flag)
	{
	case STYP_DSECT:
	  unhandled = "STYP_DSECT";
	  break;
	case STYP_GROUP:
	  unhandled = "STYP_GROUP";
	  break;
	case STYP_COPY:
	  unhandled = "STYP_COPY";
	  break;
	case STYP_OVER:
	  unhandled = "STYP_OVER";
	  break;
#ifdef SEC_NEVER_LOAD
	case STYP_NOLOAD:
	  sec_flags |= SEC_NEVER_LOAD;
	  break;
#endif
	case IMAGE_SCN_MEM_READ:
	  sec_flags &= ~SEC_COFF_NOREAD;
	  break;
	case IMAGE_SCN_TYPE_NO_PAD:
	  /* Skip.  */
	  break;
	case IMAGE_SCN_LNK_OTHER:
	  unhandled = "IMAGE_SCN_LNK_OTHER";
	  break;
	case IMAGE_SCN_MEM_NOT_CACHED:
	  unhandled = "IMAGE_SCN_MEM_NOT_CACHED";
	  break;
	case IMAGE_SCN_MEM_NOT_PAGED:
	  /* Generate a warning message rather using the 'unhandled'
	     variable as this will allow some .sys files generate by
	     other toolchains to be processed.  See bugzilla issue 196.  */
	  /* xgettext:c-format */
	  _bfd_error_handler (_("%pB: warning: ignoring section flag"
				" %s in section %s"),
			      abfd, "IMAGE_SCN_MEM_NOT_PAGED", name);
	  break;
	case IMAGE_SCN_MEM_EXECUTE:
	  sec_flags |= SEC_CODE;
	  break;
	case IMAGE_SCN_MEM_WRITE:
	  sec_flags &= ~ SEC_READONLY;
	  break;
	case IMAGE_SCN_MEM_DISCARDABLE:
	  /* The MS PE spec says that debug sections are DISCARDABLE,
	     but the presence of a DISCARDABLE flag does not necessarily
	     mean that a given section contains debug information.  Thus
	     we only set the SEC_DEBUGGING flag on sections that we
	     recognise as containing debug information.  */
	     if (is_dbg
#ifdef _COMMENT
	      || strcmp (name, _COMMENT) == 0
#endif
	      )
	    {
	      sec_flags |= SEC_DEBUGGING | SEC_READONLY;
	    }
	  break;
	case IMAGE_SCN_MEM_SHARED:
	  sec_flags |= SEC_COFF_SHARED;
	  break;
	case IMAGE_SCN_LNK_REMOVE:
	  if (!is_dbg)
	    sec_flags |= SEC_EXCLUDE;
	  break;
	case IMAGE_SCN_CNT_CODE:
	  sec_flags |= SEC_CODE | SEC_ALLOC | SEC_LOAD;
	  break;
	case IMAGE_SCN_CNT_INITIALIZED_DATA:
	  if (is_dbg)
	    sec_flags |= SEC_DEBUGGING;
	  else
	    sec_flags |= SEC_DATA | SEC_ALLOC | SEC_LOAD;
	  break;
	case IMAGE_SCN_CNT_UNINITIALIZED_DATA:
	  sec_flags |= SEC_ALLOC;
	  break;
	case IMAGE_SCN_LNK_INFO:
	  /* We mark these as SEC_DEBUGGING, but only if COFF_PAGE_SIZE is
	     defined.  coff_compute_section_file_positions uses
	     COFF_PAGE_SIZE to ensure that the low order bits of the
	     section VMA and the file offset match.  If we don't know
	     COFF_PAGE_SIZE, we can't ensure the correct correspondence,
	     and demand page loading of the file will fail.  */
#ifdef COFF_PAGE_SIZE
	  sec_flags |= SEC_DEBUGGING;
#endif
	  break;
	case IMAGE_SCN_LNK_COMDAT:
	  /* COMDAT gets very special treatment.  */
	  if (!handle_COMDAT (abfd, &sec_flags, hdr, name, section))
	    result = false;
	  break;
	default:
	  /* Silently ignore for now.  */
	  break;
	}

      /* If the section flag was not handled, report it here.  */
      if (unhandled != NULL)
	{
	  _bfd_error_handler
	    /* xgettext:c-format */
	    (_("%pB (%s): section flag %s (%#lx) ignored"),
	     abfd, name, unhandled, flag);
	  result = false;
	}
    }

  if ((bfd_applicable_section_flags (abfd) & SEC_SMALL_DATA) != 0
      && (startswith (name, ".sbss")
	  || startswith (name, ".sdata")))
    sec_flags |= SEC_SMALL_DATA;

#if defined (COFF_LONG_SECTION_NAMES) && defined (COFF_SUPPORT_GNU_LINKONCE)
  /* As a GNU extension, if the name begins with .gnu.linkonce, we
     only link a single copy of the section.  This is used to support
     g++.  g++ will emit each template expansion in its own section.
     The symbols will be defined as weak, so that multiple definitions
     are permitted.  The GNU linker extension is to actually discard
     all but one of the sections.  */
  if (startswith (name, ".gnu.linkonce"))
    sec_flags |= SEC_LINK_ONCE | SEC_LINK_DUPLICATES_DISCARD;
#endif

  if (flags_ptr)
    * flags_ptr = sec_flags;

  return result;
}

#endif /* COFF_WITH_PE */

#define	get_index(symbol)	((symbol)->udata.i)

/*
INTERNAL_DEFINITION
	bfd_coff_backend_data

INTERNAL
.{* COFF symbol classifications.  *}
.
.enum coff_symbol_classification
.{
.  {* Global symbol.  *}
.  COFF_SYMBOL_GLOBAL,
.  {* Common symbol.  *}
.  COFF_SYMBOL_COMMON,
.  {* Undefined symbol.  *}
.  COFF_SYMBOL_UNDEFINED,
.  {* Local symbol.  *}
.  COFF_SYMBOL_LOCAL,
.  {* PE section symbol.  *}
.  COFF_SYMBOL_PE_SECTION
.};
.
.typedef asection * (*coff_gc_mark_hook_fn)
.  (asection *, struct bfd_link_info *, struct internal_reloc *,
.   struct coff_link_hash_entry *, struct internal_syment *);
.

Special entry points for gdb to swap in coff symbol table parts:

CODE_FRAGMENT
.typedef struct
.{
.  void (*_bfd_coff_swap_aux_in)
.    (bfd *, void *, int, int, int, int, void *);
.
.  void (*_bfd_coff_swap_sym_in)
.    (bfd *, void *, void *);
.
.  void (*_bfd_coff_swap_lineno_in)
.    (bfd *, void *, void *);
.
.  unsigned int (*_bfd_coff_swap_aux_out)
.    (bfd *, void *, int, int, int, int, void *);
.
.  unsigned int (*_bfd_coff_swap_sym_out)
.    (bfd *, void *, void *);
.
.  unsigned int (*_bfd_coff_swap_lineno_out)
.    (bfd *, void *, void *);
.
.  unsigned int (*_bfd_coff_swap_reloc_out)
.    (bfd *, void *, void *);
.
.  unsigned int (*_bfd_coff_swap_filehdr_out)
.    (bfd *, void *, void *);
.
.  unsigned int (*_bfd_coff_swap_aouthdr_out)
.    (bfd *, void *, void *);
.
.  unsigned int (*_bfd_coff_swap_scnhdr_out)
.    (bfd *, void *, void *);
.
.  unsigned int _bfd_filhsz;
.  unsigned int _bfd_aoutsz;
.  unsigned int _bfd_scnhsz;
.  unsigned int _bfd_symesz;
.  unsigned int _bfd_auxesz;
.  unsigned int _bfd_relsz;
.  unsigned int _bfd_linesz;
.  unsigned int _bfd_filnmlen;
.  bool _bfd_coff_long_filenames;
.
.  bool _bfd_coff_long_section_names;
.  bool (*_bfd_coff_set_long_section_names)
.    (bfd *, int);
.
.  unsigned int _bfd_coff_default_section_alignment_power;
.  bool _bfd_coff_force_symnames_in_strings;
.  unsigned int _bfd_coff_debug_string_prefix_length;
.  unsigned int _bfd_coff_max_nscns;
.
.  void (*_bfd_coff_swap_filehdr_in)
.    (bfd *, void *, void *);
.
.  void (*_bfd_coff_swap_aouthdr_in)
.    (bfd *, void *, void *);
.
.  void (*_bfd_coff_swap_scnhdr_in)
.    (bfd *, void *, void *);
.
.  void (*_bfd_coff_swap_reloc_in)
.    (bfd *abfd, void *, void *);
.
.  bool (*_bfd_coff_bad_format_hook)
.    (bfd *, void *);
.
.  bool (*_bfd_coff_set_arch_mach_hook)
.    (bfd *, void *);
.
.  void * (*_bfd_coff_mkobject_hook)
.    (bfd *, void *, void *);
.
.  bool (*_bfd_styp_to_sec_flags_hook)
.    (bfd *, void *, const char *, asection *, flagword *);
.
.  void (*_bfd_set_alignment_hook)
.    (bfd *, asection *, void *);
.
.  bool (*_bfd_coff_slurp_symbol_table)
.    (bfd *);
.
.  bool (*_bfd_coff_symname_in_debug)
.    (bfd *, struct internal_syment *);
.
.  bool (*_bfd_coff_pointerize_aux_hook)
.    (bfd *, combined_entry_type *, combined_entry_type *,
.     unsigned int, combined_entry_type *);
.
.  bool (*_bfd_coff_print_aux)
.    (bfd *, FILE *, combined_entry_type *, combined_entry_type *,
.     combined_entry_type *, unsigned int);
.
.  bool (*_bfd_coff_reloc16_extra_cases)
.    (bfd *, struct bfd_link_info *, struct bfd_link_order *, arelent *,
.     bfd_byte *, size_t *, size_t *);
.
.  int (*_bfd_coff_reloc16_estimate)
.    (bfd *, asection *, arelent *, unsigned int,
.     struct bfd_link_info *);
.
.  enum coff_symbol_classification (*_bfd_coff_classify_symbol)
.    (bfd *, struct internal_syment *);
.
.  bool (*_bfd_coff_compute_section_file_positions)
.    (bfd *);
.
.  bool (*_bfd_coff_start_final_link)
.    (bfd *, struct bfd_link_info *);
.
.  bool (*_bfd_coff_relocate_section)
.    (bfd *, struct bfd_link_info *, bfd *, asection *, bfd_byte *,
.     struct internal_reloc *, struct internal_syment *, asection **);
.
.  reloc_howto_type *(*_bfd_coff_rtype_to_howto)
.    (bfd *, asection *, struct internal_reloc *,
.     struct coff_link_hash_entry *, struct internal_syment *, bfd_vma *);
.
.  bool (*_bfd_coff_adjust_symndx)
.    (bfd *, struct bfd_link_info *, bfd *, asection *,
.     struct internal_reloc *, bool *);
.
.  bool (*_bfd_coff_link_add_one_symbol)
.    (struct bfd_link_info *, bfd *, const char *, flagword,
.     asection *, bfd_vma, const char *, bool, bool,
.     struct bfd_link_hash_entry **);
.
.  bool (*_bfd_coff_link_output_has_begun)
.    (bfd *, struct coff_final_link_info *);
.
.  bool (*_bfd_coff_final_link_postscript)
.    (bfd *, struct coff_final_link_info *);
.
.  bool (*_bfd_coff_print_pdata)
.    (bfd *, void *);
.
.} bfd_coff_backend_data;
.

INTERNAL
.#define coff_backend_info(abfd) \
.  ((const bfd_coff_backend_data *) (abfd)->xvec->backend_data)
.
.#define bfd_coff_swap_aux_in(a,e,t,c,ind,num,i) \
.  ((coff_backend_info (a)->_bfd_coff_swap_aux_in) (a,e,t,c,ind,num,i))
.
.#define bfd_coff_swap_sym_in(a,e,i) \
.  ((coff_backend_info (a)->_bfd_coff_swap_sym_in) (a,e,i))
.
.#define bfd_coff_swap_lineno_in(a,e,i) \
.  ((coff_backend_info ( a)->_bfd_coff_swap_lineno_in) (a,e,i))
.
.#define bfd_coff_swap_reloc_out(abfd, i, o) \
.  ((coff_backend_info (abfd)->_bfd_coff_swap_reloc_out) (abfd, i, o))
.
.#define bfd_coff_swap_lineno_out(abfd, i, o) \
.  ((coff_backend_info (abfd)->_bfd_coff_swap_lineno_out) (abfd, i, o))
.
.#define bfd_coff_swap_aux_out(a,i,t,c,ind,num,o) \
.  ((coff_backend_info (a)->_bfd_coff_swap_aux_out) (a,i,t,c,ind,num,o))
.
.#define bfd_coff_swap_sym_out(abfd, i,o) \
.  ((coff_backend_info (abfd)->_bfd_coff_swap_sym_out) (abfd, i, o))
.
.#define bfd_coff_swap_scnhdr_out(abfd, i,o) \
.  ((coff_backend_info (abfd)->_bfd_coff_swap_scnhdr_out) (abfd, i, o))
.
.#define bfd_coff_swap_filehdr_out(abfd, i,o) \
.  ((coff_backend_info (abfd)->_bfd_coff_swap_filehdr_out) (abfd, i, o))
.
.#define bfd_coff_swap_aouthdr_out(abfd, i,o) \
.  ((coff_backend_info (abfd)->_bfd_coff_swap_aouthdr_out) (abfd, i, o))
.
.#define bfd_coff_filhsz(abfd) (coff_backend_info (abfd)->_bfd_filhsz)
.#define bfd_coff_aoutsz(abfd) (coff_backend_info (abfd)->_bfd_aoutsz)
.#define bfd_coff_scnhsz(abfd) (coff_backend_info (abfd)->_bfd_scnhsz)
.#define bfd_coff_symesz(abfd) (coff_backend_info (abfd)->_bfd_symesz)
.#define bfd_coff_auxesz(abfd) (coff_backend_info (abfd)->_bfd_auxesz)
.#define bfd_coff_relsz(abfd)  (coff_backend_info (abfd)->_bfd_relsz)
.#define bfd_coff_linesz(abfd) (coff_backend_info (abfd)->_bfd_linesz)
.#define bfd_coff_filnmlen(abfd) (coff_backend_info (abfd)->_bfd_filnmlen)
.#define bfd_coff_long_filenames(abfd) \
.  (coff_backend_info (abfd)->_bfd_coff_long_filenames)
.#define bfd_coff_long_section_names(abfd) \
.  (coff_data (abfd)->long_section_names)
.#define bfd_coff_set_long_section_names(abfd, enable) \
.  ((coff_backend_info (abfd)->_bfd_coff_set_long_section_names) (abfd, enable))
.#define bfd_coff_default_section_alignment_power(abfd) \
.  (coff_backend_info (abfd)->_bfd_coff_default_section_alignment_power)
.#define bfd_coff_max_nscns(abfd) \
.  (coff_backend_info (abfd)->_bfd_coff_max_nscns)
.
.#define bfd_coff_swap_filehdr_in(abfd, i,o) \
.  ((coff_backend_info (abfd)->_bfd_coff_swap_filehdr_in) (abfd, i, o))
.
.#define bfd_coff_swap_aouthdr_in(abfd, i,o) \
.  ((coff_backend_info (abfd)->_bfd_coff_swap_aouthdr_in) (abfd, i, o))
.
.#define bfd_coff_swap_scnhdr_in(abfd, i,o) \
.  ((coff_backend_info (abfd)->_bfd_coff_swap_scnhdr_in) (abfd, i, o))
.
.#define bfd_coff_swap_reloc_in(abfd, i, o) \
.  ((coff_backend_info (abfd)->_bfd_coff_swap_reloc_in) (abfd, i, o))
.
.#define bfd_coff_bad_format_hook(abfd, filehdr) \
.  ((coff_backend_info (abfd)->_bfd_coff_bad_format_hook) (abfd, filehdr))
.
.#define bfd_coff_set_arch_mach_hook(abfd, filehdr)\
.  ((coff_backend_info (abfd)->_bfd_coff_set_arch_mach_hook) (abfd, filehdr))
.#define bfd_coff_mkobject_hook(abfd, filehdr, aouthdr)\
.  ((coff_backend_info (abfd)->_bfd_coff_mkobject_hook)\
.   (abfd, filehdr, aouthdr))
.
.#define bfd_coff_styp_to_sec_flags_hook(abfd, scnhdr, name, section, flags_ptr)\
.  ((coff_backend_info (abfd)->_bfd_styp_to_sec_flags_hook)\
.   (abfd, scnhdr, name, section, flags_ptr))
.
.#define bfd_coff_set_alignment_hook(abfd, sec, scnhdr)\
.  ((coff_backend_info (abfd)->_bfd_set_alignment_hook) (abfd, sec, scnhdr))
.
.#define bfd_coff_slurp_symbol_table(abfd)\
.  ((coff_backend_info (abfd)->_bfd_coff_slurp_symbol_table) (abfd))
.
.#define bfd_coff_symname_in_debug(abfd, sym)\
.  ((coff_backend_info (abfd)->_bfd_coff_symname_in_debug) (abfd, sym))
.
.#define bfd_coff_force_symnames_in_strings(abfd)\
.  (coff_backend_info (abfd)->_bfd_coff_force_symnames_in_strings)
.
.#define bfd_coff_debug_string_prefix_length(abfd)\
.  (coff_backend_info (abfd)->_bfd_coff_debug_string_prefix_length)
.
.#define bfd_coff_print_aux(abfd, file, base, symbol, aux, indaux)\
.  ((coff_backend_info (abfd)->_bfd_coff_print_aux)\
.   (abfd, file, base, symbol, aux, indaux))
.
.#define bfd_coff_reloc16_extra_cases(abfd, link_info, link_order,\
.				      reloc, data, src_ptr, dst_ptr)\
.  ((coff_backend_info (abfd)->_bfd_coff_reloc16_extra_cases)\
.   (abfd, link_info, link_order, reloc, data, src_ptr, dst_ptr))
.
.#define bfd_coff_reloc16_estimate(abfd, section, reloc, shrink, link_info)\
.  ((coff_backend_info (abfd)->_bfd_coff_reloc16_estimate)\
.   (abfd, section, reloc, shrink, link_info))
.
.#define bfd_coff_classify_symbol(abfd, sym)\
.  ((coff_backend_info (abfd)->_bfd_coff_classify_symbol)\
.   (abfd, sym))
.
.#define bfd_coff_compute_section_file_positions(abfd)\
.  ((coff_backend_info (abfd)->_bfd_coff_compute_section_file_positions)\
.   (abfd))
.
.#define bfd_coff_start_final_link(obfd, info)\
.  ((coff_backend_info (obfd)->_bfd_coff_start_final_link)\
.   (obfd, info))
.#define bfd_coff_relocate_section(obfd,info,ibfd,o,con,rel,isyms,secs)\
.  ((coff_backend_info (ibfd)->_bfd_coff_relocate_section)\
.   (obfd, info, ibfd, o, con, rel, isyms, secs))
.#define bfd_coff_rtype_to_howto(abfd, sec, rel, h, sym, addendp)\
.  ((coff_backend_info (abfd)->_bfd_coff_rtype_to_howto)\
.   (abfd, sec, rel, h, sym, addendp))
.#define bfd_coff_adjust_symndx(obfd, info, ibfd, sec, rel, adjustedp)\
.  ((coff_backend_info (abfd)->_bfd_coff_adjust_symndx)\
.   (obfd, info, ibfd, sec, rel, adjustedp))
.#define bfd_coff_link_add_one_symbol(info, abfd, name, flags, section,\
.				      value, string, cp, coll, hashp)\
.  ((coff_backend_info (abfd)->_bfd_coff_link_add_one_symbol)\
.   (info, abfd, name, flags, section, value, string, cp, coll, hashp))
.
.#define bfd_coff_link_output_has_begun(a,p) \
.  ((coff_backend_info (a)->_bfd_coff_link_output_has_begun) (a, p))
.#define bfd_coff_final_link_postscript(a,p) \
.  ((coff_backend_info (a)->_bfd_coff_final_link_postscript) (a, p))
.
.#define bfd_coff_have_print_pdata(a) \
.  (coff_backend_info (a)->_bfd_coff_print_pdata)
.#define bfd_coff_print_pdata(a,p) \
.  ((coff_backend_info (a)->_bfd_coff_print_pdata) (a, p))
.
.{* Macro: Returns true if the bfd is a PE executable as opposed to a
.   PE object file.  *}
.#define bfd_pei_p(abfd) \
.  (startswith ((abfd)->xvec->name, "pei-"))
*/

/* See whether the magic number matches.  */

static bool
coff_bad_format_hook (bfd * abfd ATTRIBUTE_UNUSED, void * filehdr)
{
  struct internal_filehdr *internal_f = (struct internal_filehdr *) filehdr;

  if (BADMAG (*internal_f))
    return false;

  return true;
}

#ifdef TICOFF
static bool
ticoff0_bad_format_hook (bfd *abfd ATTRIBUTE_UNUSED, void * filehdr)
{
  struct internal_filehdr *internal_f = (struct internal_filehdr *) filehdr;

  if (COFF0_BADMAG (*internal_f))
    return false;

  return true;
}
#endif

#ifdef TICOFF
static bool
ticoff1_bad_format_hook (bfd *abfd ATTRIBUTE_UNUSED, void * filehdr)
{
  struct internal_filehdr *internal_f = (struct internal_filehdr *) filehdr;

  if (COFF1_BADMAG (*internal_f))
    return false;

  return true;
}
#endif

/* Check whether this section uses an alignment other than the
   default.  */

static void
coff_set_custom_section_alignment (bfd *abfd ATTRIBUTE_UNUSED,
				   asection *section,
				   const struct coff_section_alignment_entry *alignment_table,
				   const unsigned int table_size)
{
  const unsigned int default_alignment = COFF_DEFAULT_SECTION_ALIGNMENT_POWER;
  unsigned int i;

  for (i = 0; i < table_size; ++i)
    {
      const char *secname = bfd_section_name (section);

      if (alignment_table[i].comparison_length == (unsigned int) -1
	  ? strcmp (alignment_table[i].name, secname) == 0
	  : strncmp (alignment_table[i].name, secname,
		     alignment_table[i].comparison_length) == 0)
	break;
    }
  if (i >= table_size)
    return;

  if (alignment_table[i].default_alignment_min != COFF_ALIGNMENT_FIELD_EMPTY
      && default_alignment < alignment_table[i].default_alignment_min)
    return;

  if (alignment_table[i].default_alignment_max != COFF_ALIGNMENT_FIELD_EMPTY
#if COFF_DEFAULT_SECTION_ALIGNMENT_POWER != 0
      && default_alignment > alignment_table[i].default_alignment_max
#endif
      )
    return;

  section->alignment_power = alignment_table[i].alignment_power;
}

/* Custom section alignment records.  */

static const struct coff_section_alignment_entry
coff_section_alignment_table[] =
{
#ifdef COFF_SECTION_ALIGNMENT_ENTRIES
  COFF_SECTION_ALIGNMENT_ENTRIES,
#endif
  /* There must not be any gaps between .stabstr sections.  */
  { COFF_SECTION_NAME_PARTIAL_MATCH (".stabstr"),
    1, COFF_ALIGNMENT_FIELD_EMPTY, 0 },
  /* The .stab section must be aligned to 2**2 at most, to avoid gaps.  */
  { COFF_SECTION_NAME_PARTIAL_MATCH (".stab"),
    3, COFF_ALIGNMENT_FIELD_EMPTY, 2 },
  /* Similarly for the .ctors and .dtors sections.  */
  { COFF_SECTION_NAME_EXACT_MATCH (".ctors"),
    3, COFF_ALIGNMENT_FIELD_EMPTY, 2 },
  { COFF_SECTION_NAME_EXACT_MATCH (".dtors"),
    3, COFF_ALIGNMENT_FIELD_EMPTY, 2 }
};

static const unsigned int coff_section_alignment_table_size =
  sizeof coff_section_alignment_table / sizeof coff_section_alignment_table[0];

/* Initialize a section structure with information peculiar to this
   particular implementation of COFF.  */

static bool
coff_new_section_hook (bfd * abfd, asection * section)
{
  combined_entry_type *native;
  size_t amt;
  unsigned char sclass = C_STAT;

  section->alignment_power = COFF_DEFAULT_SECTION_ALIGNMENT_POWER;

#ifdef RS6000COFF_C
  if (bfd_xcoff_text_align_power (abfd) != 0
      && strcmp (bfd_section_name (section), ".text") == 0)
    section->alignment_power = bfd_xcoff_text_align_power (abfd);
  else if (bfd_xcoff_data_align_power (abfd) != 0
      && strcmp (bfd_section_name (section), ".data") == 0)
    section->alignment_power = bfd_xcoff_data_align_power (abfd);
  else
    {
      int i;

      for (i = 0; i < XCOFF_DWSECT_NBR_NAMES; i++)
	if (strcmp (bfd_section_name (section),
		    xcoff_dwsect_names[i].xcoff_name) == 0)
	  {
	    section->alignment_power = 0;
	    sclass = C_DWARF;
	    break;
	  }
    }
#endif

  /* Set up the section symbol.  */
  if (!_bfd_generic_new_section_hook (abfd, section))
    return false;

  /* Allocate aux records for section symbols, to store size and
     related info.

     @@ The 10 is a guess at a plausible maximum number of aux entries
     (but shouldn't be a constant).  */
  amt = sizeof (combined_entry_type) * 10;
  native = (combined_entry_type *) bfd_zalloc (abfd, amt);
  if (native == NULL)
    return false;

  /* We don't need to set up n_name, n_value, or n_scnum in the native
     symbol information, since they'll be overridden by the BFD symbol
     anyhow.  However, we do need to set the type and storage class,
     in case this symbol winds up getting written out.  The value 0
     for n_numaux is already correct.  */

  native->is_sym = true;
  native->u.syment.n_type = T_NULL;
  native->u.syment.n_sclass = sclass;

  coffsymbol (section->symbol)->native = native;

  coff_set_custom_section_alignment (abfd, section,
				     coff_section_alignment_table,
				     coff_section_alignment_table_size);

  return true;
}

#ifdef COFF_ALIGN_IN_SECTION_HEADER

/* Set the alignment of a BFD section.  */

static void
coff_set_alignment_hook (bfd * abfd ATTRIBUTE_UNUSED,
			 asection * section,
			 void * scnhdr)
{
  struct internal_scnhdr *hdr = (struct internal_scnhdr *) scnhdr;
  unsigned int i;

#ifdef COFF_DECODE_ALIGNMENT
  i = COFF_DECODE_ALIGNMENT(hdr->s_flags);
#endif
  section->alignment_power = i;

#ifdef coff_set_section_load_page
  coff_set_section_load_page (section, hdr->s_page);
#endif
}

#else /* ! COFF_ALIGN_IN_SECTION_HEADER */
#ifdef COFF_WITH_PE

static void
coff_set_alignment_hook (bfd * abfd ATTRIBUTE_UNUSED,
			 asection * section,
			 void * scnhdr)
{
  struct internal_scnhdr *hdr = (struct internal_scnhdr *) scnhdr;
  size_t amt;
  unsigned int alignment_power_const
    = hdr->s_flags & IMAGE_SCN_ALIGN_POWER_BIT_MASK;

  switch (alignment_power_const)
    {
    case IMAGE_SCN_ALIGN_8192BYTES:
    case IMAGE_SCN_ALIGN_4096BYTES:
    case IMAGE_SCN_ALIGN_2048BYTES:
    case IMAGE_SCN_ALIGN_1024BYTES:
    case IMAGE_SCN_ALIGN_512BYTES:
    case IMAGE_SCN_ALIGN_256BYTES:
    case IMAGE_SCN_ALIGN_128BYTES:
    case IMAGE_SCN_ALIGN_64BYTES:
    case IMAGE_SCN_ALIGN_32BYTES:
    case IMAGE_SCN_ALIGN_16BYTES:
    case IMAGE_SCN_ALIGN_8BYTES:
    case IMAGE_SCN_ALIGN_4BYTES:
    case IMAGE_SCN_ALIGN_2BYTES:
    case IMAGE_SCN_ALIGN_1BYTES:
      section->alignment_power
	= IMAGE_SCN_ALIGN_POWER_NUM (alignment_power_const);
      break;
    default:
      break;
    }

  /* In a PE image file, the s_paddr field holds the virtual size of a
     section, while the s_size field holds the raw size.  We also keep
     the original section flag value, since not every bit can be
     mapped onto a generic BFD section bit.  */
  if (coff_section_data (abfd, section) == NULL)
    {
      amt = sizeof (struct coff_section_tdata);
      section->used_by_bfd = bfd_zalloc (abfd, amt);
      if (section->used_by_bfd == NULL)
	/* FIXME: Return error.  */
	abort ();
    }

  if (pei_section_data (abfd, section) == NULL)
    {
      amt = sizeof (struct pei_section_tdata);
      coff_section_data (abfd, section)->tdata = bfd_zalloc (abfd, amt);
      if (coff_section_data (abfd, section)->tdata == NULL)
	/* FIXME: Return error.  */
	abort ();
    }
  pei_section_data (abfd, section)->virt_size = hdr->s_paddr;
  pei_section_data (abfd, section)->pe_flags = hdr->s_flags;

  section->lma = hdr->s_vaddr;

  /* Check for extended relocs.  */
  if (hdr->s_flags & IMAGE_SCN_LNK_NRELOC_OVFL)
    {
      struct external_reloc dst;
      struct internal_reloc n;
      file_ptr oldpos = bfd_tell (abfd);
      bfd_size_type relsz = bfd_coff_relsz (abfd);

      if (bfd_seek (abfd, (file_ptr) hdr->s_relptr, 0) != 0)
	return;
      if (bfd_bread (& dst, relsz, abfd) != relsz)
	return;

      bfd_coff_swap_reloc_in (abfd, &dst, &n);
      if (bfd_seek (abfd, oldpos, 0) != 0)
	return;
      if (n.r_vaddr < 0x10000)
	{
	  _bfd_error_handler (_("%pB: overflow reloc count too small"), abfd);
	  bfd_set_error (bfd_error_bad_value);
	  return;
	}
      section->reloc_count = hdr->s_nreloc = n.r_vaddr - 1;
      section->rel_filepos += relsz;
    }
  else if (hdr->s_nreloc == 0xffff)
    _bfd_error_handler
      (_("%pB: warning: claims to have 0xffff relocs, without overflow"),
       abfd);
}
#undef ALIGN_SET
#undef ELIFALIGN_SET

#else /* ! COFF_WITH_PE */
#ifdef RS6000COFF_C

/* We grossly abuse this function to handle XCOFF overflow headers.
   When we see one, we correct the reloc and line number counts in the
   real header, and remove the section we just created.  */

static void
coff_set_alignment_hook (bfd *abfd, asection *section, void * scnhdr)
{
  struct internal_scnhdr *hdr = (struct internal_scnhdr *) scnhdr;
  asection *real_sec;

  if ((hdr->s_flags & STYP_OVRFLO) == 0)
    return;

  real_sec = coff_section_from_bfd_index (abfd, (int) hdr->s_nreloc);
  if (real_sec == NULL)
    return;

  real_sec->reloc_count = hdr->s_paddr;
  real_sec->lineno_count = hdr->s_vaddr;

  if (!bfd_section_removed_from_list (abfd, section))
    {
      bfd_section_list_remove (abfd, section);
      --abfd->section_count;
    }
}

#else /* ! RS6000COFF_C */
#if defined (COFF_GO32_EXE) || defined (COFF_GO32)

static void
coff_set_alignment_hook (bfd * abfd, asection * section, void * scnhdr)
{
  struct internal_scnhdr *hdr = (struct internal_scnhdr *) scnhdr;

  /* Check for extended relocs.  */
  if (hdr->s_flags & IMAGE_SCN_LNK_NRELOC_OVFL)
    {
      struct external_reloc dst;
      struct internal_reloc n;
      const file_ptr oldpos = bfd_tell (abfd);
      const bfd_size_type relsz = bfd_coff_relsz (abfd);

      if (bfd_seek (abfd, (file_ptr) hdr->s_relptr, 0) != 0)
	return;
      if (bfd_bread (& dst, relsz, abfd) != relsz)
	return;

      bfd_coff_swap_reloc_in (abfd, &dst, &n);
      if (bfd_seek (abfd, oldpos, 0) != 0)
	return;
      section->reloc_count = hdr->s_nreloc = n.r_vaddr - 1;
      section->rel_filepos += relsz;
    }
  else if (hdr->s_nreloc == 0xffff)
    _bfd_error_handler
      (_("%pB: warning: claims to have 0xffff relocs, without overflow"),
       abfd);
}

#else /* ! COFF_GO32_EXE && ! COFF_GO32 */

static void
coff_set_alignment_hook (bfd *abfd ATTRIBUTE_UNUSED,
			 asection *section ATTRIBUTE_UNUSED,
			 void *scnhdr ATTRIBUTE_UNUSED)
{
}

#endif /* ! COFF_GO32_EXE && ! COFF_GO32 */
#endif /* ! RS6000COFF_C */
#endif /* ! COFF_WITH_PE */
#endif /* ! COFF_ALIGN_IN_SECTION_HEADER */

#ifndef coff_mkobject

static bool
coff_mkobject (bfd * abfd)
{
  coff_data_type *coff;
  size_t amt = sizeof (coff_data_type);

  abfd->tdata.coff_obj_data = bfd_zalloc (abfd, amt);
  if (abfd->tdata.coff_obj_data == NULL)
    return false;

  coff = coff_data (abfd);
  coff->symbols = NULL;
  coff->conversion_table = NULL;
  coff->raw_syments = NULL;
  coff->relocbase = 0;
  coff->local_toc_sym_map = 0;

  bfd_coff_long_section_names (abfd)
    = coff_backend_info (abfd)->_bfd_coff_long_section_names;

/*  make_abs_section(abfd);*/

  return true;
}
#endif

/* Create the COFF backend specific information.  */

#ifndef coff_mkobject_hook
static void *
coff_mkobject_hook (bfd * abfd,
		    void * filehdr,
		    void * aouthdr ATTRIBUTE_UNUSED)
{
  struct internal_filehdr *internal_f = (struct internal_filehdr *) filehdr;
  coff_data_type *coff;

  if (! coff_mkobject (abfd))
    return NULL;

  coff = coff_data (abfd);

  coff->sym_filepos = internal_f->f_symptr;

  /* These members communicate important constants about the symbol
     table to GDB's symbol-reading code.  These `constants'
     unfortunately vary among coff implementations...  */
  coff->local_n_btmask = N_BTMASK;
  coff->local_n_btshft = N_BTSHFT;
  coff->local_n_tmask = N_TMASK;
  coff->local_n_tshift = N_TSHIFT;
  coff->local_symesz = bfd_coff_symesz (abfd);
  coff->local_auxesz = bfd_coff_auxesz (abfd);
  coff->local_linesz = bfd_coff_linesz (abfd);

  coff->timestamp = internal_f->f_timdat;

  obj_raw_syment_count (abfd) =
    obj_conv_table_size (abfd) =
      internal_f->f_nsyms;

#ifdef RS6000COFF_C
  if ((internal_f->f_flags & F_SHROBJ) != 0)
    abfd->flags |= DYNAMIC;
  if (aouthdr != NULL && internal_f->f_opthdr >= bfd_coff_aoutsz (abfd))
    {
      struct internal_aouthdr *internal_a =
	(struct internal_aouthdr *) aouthdr;
      struct xcoff_tdata *xcoff;

      xcoff = xcoff_data (abfd);
# ifdef U803XTOCMAGIC
      xcoff->xcoff64 = internal_f->f_magic == U803XTOCMAGIC;
# else
      xcoff->xcoff64 = 0;
# endif
      xcoff->full_aouthdr = true;
      xcoff->toc = internal_a->o_toc;
      xcoff->sntoc = internal_a->o_sntoc;
      xcoff->snentry = internal_a->o_snentry;
      bfd_xcoff_text_align_power (abfd) = internal_a->o_algntext;
      bfd_xcoff_data_align_power (abfd) = internal_a->o_algndata;
      xcoff->modtype = internal_a->o_modtype;
      xcoff->cputype = internal_a->o_cputype;
      xcoff->maxdata = internal_a->o_maxdata;
      xcoff->maxstack = internal_a->o_maxstack;
    }
#endif

#ifdef ARM
  /* Set the flags field from the COFF header read in.  */
  if (! _bfd_coff_arm_set_private_flags (abfd, internal_f->f_flags))
    coff->flags = 0;
#endif

#ifdef COFF_WITH_PE
  /* FIXME: I'm not sure this is ever executed, since peicode.h
     defines coff_mkobject_hook.  */
  if ((internal_f->f_flags & IMAGE_FILE_DEBUG_STRIPPED) == 0)
    abfd->flags |= HAS_DEBUG;
#endif

  return coff;
}
#endif

/* Determine the machine architecture and type.  FIXME: This is target
   dependent because the magic numbers are defined in the target
   dependent header files.  But there is no particular need for this.
   If the magic numbers were moved to a separate file, this function
   would be target independent and would also be much more successful
   at linking together COFF files for different architectures.  */

static bool
coff_set_arch_mach_hook (bfd *abfd, void * filehdr)
{
  unsigned long machine;
  enum bfd_architecture arch;
  struct internal_filehdr *internal_f = (struct internal_filehdr *) filehdr;

  /* Zero selects the default machine for an arch.  */
  machine = 0;
  switch (internal_f->f_magic)
    {
#ifdef I386MAGIC
    case I386MAGIC:
    case I386PTXMAGIC:
    case I386AIXMAGIC:		/* Danbury PS/2 AIX C Compiler.  */
    case LYNXCOFFMAGIC:
    case I386_APPLE_MAGIC:
    case I386_FREEBSD_MAGIC:
    case I386_LINUX_MAGIC:
    case I386_NETBSD_MAGIC:
      arch = bfd_arch_i386;
      break;
#endif
#ifdef AMD64MAGIC
    case AMD64MAGIC:
    case AMD64_APPLE_MAGIC:
    case AMD64_FREEBSD_MAGIC:
    case AMD64_LINUX_MAGIC:
    case AMD64_NETBSD_MAGIC:
      arch = bfd_arch_i386;
      machine = bfd_mach_x86_64;
      break;
#endif
#ifdef IA64MAGIC
    case IA64MAGIC:
      arch = bfd_arch_ia64;
      break;
#endif
#ifdef ARMMAGIC
    case ARMMAGIC:
    case ARMPEMAGIC:
    case THUMBPEMAGIC:
      arch = bfd_arch_arm;
      machine = bfd_arm_get_mach_from_notes (abfd, ARM_NOTE_SECTION);
      if (machine == bfd_mach_arm_unknown)
	{
	  switch (internal_f->f_flags & F_ARM_ARCHITECTURE_MASK)
	    {
	    case F_ARM_2:  machine = bfd_mach_arm_2;  break;
	    case F_ARM_2a: machine = bfd_mach_arm_2a; break;
	    case F_ARM_3:  machine = bfd_mach_arm_3;  break;
	    default:
	    case F_ARM_3M: machine = bfd_mach_arm_3M; break;
	    case F_ARM_4:  machine = bfd_mach_arm_4;  break;
	    case F_ARM_4T: machine = bfd_mach_arm_4T; break;
	      /* The COFF header does not have enough bits available
		 to cover all the different ARM architectures.  So
		 we interpret F_ARM_5, the highest flag value to mean
		 "the highest ARM architecture known to BFD" which is
		 currently the XScale.  */
	    case F_ARM_5:  machine = bfd_mach_arm_XScale;  break;
	    }
	}
      break;
#endif
#ifdef AARCH64MAGIC
    case AARCH64MAGIC:
      arch = bfd_arch_aarch64;
      machine = internal_f->f_flags & F_AARCH64_ARCHITECTURE_MASK;
      break;
#endif
#ifdef LOONGARCH64MAGIC
    case LOONGARCH64MAGIC:
      arch = bfd_arch_loongarch;
      machine = internal_f->f_flags & F_LOONGARCH64_ARCHITECTURE_MASK;
      break;
#endif
#ifdef Z80MAGIC
    case Z80MAGIC:
      arch = bfd_arch_z80;
      switch (internal_f->f_flags & F_MACHMASK)
	{
	case bfd_mach_z80strict << 12:
	case bfd_mach_z80 << 12:
	case bfd_mach_z80n << 12:
	case bfd_mach_z80full << 12:
	case bfd_mach_r800 << 12:
	case bfd_mach_gbz80 << 12:
	case bfd_mach_z180 << 12:
	case bfd_mach_ez80_z80 << 12:
	case bfd_mach_ez80_adl << 12:
	  machine = ((unsigned)internal_f->f_flags & F_MACHMASK) >> 12;
	  break;
	default:
	  return false;
	}
      break;
#endif
#ifdef Z8KMAGIC
    case Z8KMAGIC:
      arch = bfd_arch_z8k;
      switch (internal_f->f_flags & F_MACHMASK)
	{
	case F_Z8001:
	  machine = bfd_mach_z8001;
	  break;
	case F_Z8002:
	  machine = bfd_mach_z8002;
	  break;
	default:
	  return false;
	}
      break;
#endif

#ifdef RS6000COFF_C
#ifdef XCOFF64
    case U64_TOCMAGIC:
    case U803XTOCMAGIC:
#else
    case U802ROMAGIC:
    case U802WRMAGIC:
    case U802TOCMAGIC:
#endif
      {
	int cputype;

	if (xcoff_data (abfd)->cputype != -1)
	  cputype = xcoff_data (abfd)->cputype & 0xff;
	else
	  {
	    /* We did not get a value from the a.out header.  If the
	       file has not been stripped, we may be able to get the
	       architecture information from the first symbol, if it
	       is a .file symbol.  */
	    if (obj_raw_syment_count (abfd) == 0)
	      cputype = 0;
	    else
	      {
		bfd_byte *buf;
		struct internal_syment sym;
		bfd_size_type amt = bfd_coff_symesz (abfd);

		if (bfd_seek (abfd, obj_sym_filepos (abfd), SEEK_SET) != 0)
		  return false;
		buf = _bfd_malloc_and_read (abfd, amt, amt);
		if (buf == NULL)
		  return false;
		bfd_coff_swap_sym_in (abfd, buf, & sym);
		if (sym.n_sclass == C_FILE)
		  cputype = sym.n_type & 0xff;
		else
		  cputype = 0;
		free (buf);
	      }
	  }

	/* FIXME: We don't handle all cases here.  */
	switch (cputype)
	  {
	  default:
	  case 0:
	    arch = bfd_xcoff_architecture (abfd);
	    machine = bfd_xcoff_machine (abfd);
	    break;

	  case 1:
	    arch = bfd_arch_powerpc;
	    machine = bfd_mach_ppc_601;
	    break;
	  case 2: /* 64 bit PowerPC */
	    arch = bfd_arch_powerpc;
	    machine = bfd_mach_ppc_620;
	    break;
	  case 3:
	    arch = bfd_arch_powerpc;
	    machine = bfd_mach_ppc;
	    break;
	  case 4:
	    arch = bfd_arch_rs6000;
	    machine = bfd_mach_rs6k;
	    break;
	  }
      }
      break;
#endif

#ifdef SH_ARCH_MAGIC_BIG
    case SH_ARCH_MAGIC_BIG:
    case SH_ARCH_MAGIC_LITTLE:
#ifdef COFF_WITH_PE
    case SH_ARCH_MAGIC_WINCE:
#endif
      arch = bfd_arch_sh;
      break;
#endif

#ifdef MIPS_ARCH_MAGIC_WINCE
    case MIPS_ARCH_MAGIC_WINCE:
      arch = bfd_arch_mips;
      break;
#endif

#ifdef SPARCMAGIC
    case SPARCMAGIC:
#ifdef LYNXCOFFMAGIC
    case LYNXCOFFMAGIC:
#endif
      arch = bfd_arch_sparc;
      break;
#endif

#ifdef TIC30MAGIC
    case TIC30MAGIC:
      arch = bfd_arch_tic30;
      break;
#endif

#ifdef TICOFF0MAGIC
#ifdef TICOFF_TARGET_ARCH
      /* This TI COFF section should be used by all new TI COFF v0 targets.  */
    case TICOFF0MAGIC:
      arch = TICOFF_TARGET_ARCH;
      machine = TICOFF_TARGET_MACHINE_GET (internal_f->f_flags);
      break;
#endif
#endif

#ifdef TICOFF1MAGIC
      /* This TI COFF section should be used by all new TI COFF v1/2 targets.  */
      /* TI COFF1 and COFF2 use the target_id field to specify which arch.  */
    case TICOFF1MAGIC:
    case TICOFF2MAGIC:
      switch (internal_f->f_target_id)
	{
#ifdef TI_TARGET_ID
	case TI_TARGET_ID:
	  arch = TICOFF_TARGET_ARCH;
	  machine = TICOFF_TARGET_MACHINE_GET (internal_f->f_flags);
	  break;
#endif
	default:
	  arch = bfd_arch_obscure;
	  _bfd_error_handler
	    (_("unrecognized TI COFF target id '0x%x'"),
	     internal_f->f_target_id);
	  break;
	}
      break;
#endif

#ifdef MCOREMAGIC
    case MCOREMAGIC:
      arch = bfd_arch_mcore;
      break;
#endif

    default:			/* Unreadable input file type.  */
      arch = bfd_arch_obscure;
      break;
    }

  bfd_default_set_arch_mach (abfd, arch, machine);
  return true;
}

static bool
symname_in_debug_hook (bfd *abfd ATTRIBUTE_UNUSED,
		       struct internal_syment *sym ATTRIBUTE_UNUSED)
{
#ifdef SYMNAME_IN_DEBUG
  return SYMNAME_IN_DEBUG (sym) != 0;
#else
  return false;
#endif
}

#ifdef RS6000COFF_C

#ifdef XCOFF64
#define FORCE_SYMNAMES_IN_STRINGS
#endif

/* Handle the csect auxent of a C_EXT, C_AIX_WEAKEXT or C_HIDEXT symbol.  */

static bool
coff_pointerize_aux_hook (bfd *abfd ATTRIBUTE_UNUSED,
			  combined_entry_type *table_base,
			  combined_entry_type *symbol,
			  unsigned int indaux,
			  combined_entry_type *aux)
{
  BFD_ASSERT (symbol->is_sym);
  int n_sclass = symbol->u.syment.n_sclass;

  if (CSECT_SYM_P (n_sclass)
      && indaux + 1 == symbol->u.syment.n_numaux)
    {
      BFD_ASSERT (! aux->is_sym);
      if (SMTYP_SMTYP (aux->u.auxent.x_csect.x_smtyp) == XTY_LD
	  && aux->u.auxent.x_csect.x_scnlen.u64 < obj_raw_syment_count (abfd))
	{
	  aux->u.auxent.x_csect.x_scnlen.p =
	    table_base + aux->u.auxent.x_csect.x_scnlen.u64;
	  aux->fix_scnlen = 1;
	}

      /* Return TRUE to indicate that the caller should not do any
	 further work on this auxent.  */
      return true;
    }

  /* Return FALSE to indicate that this auxent should be handled by
     the caller.  */
  return false;
}

#else
#define coff_pointerize_aux_hook 0
#endif /* ! RS6000COFF_C */

/* Print an aux entry.  This returns TRUE if it has printed it.  */

static bool
coff_print_aux (bfd *abfd ATTRIBUTE_UNUSED,
		FILE *file ATTRIBUTE_UNUSED,
		combined_entry_type *table_base ATTRIBUTE_UNUSED,
		combined_entry_type *symbol ATTRIBUTE_UNUSED,
		combined_entry_type *aux ATTRIBUTE_UNUSED,
		unsigned int indaux ATTRIBUTE_UNUSED)
{
  BFD_ASSERT (symbol->is_sym);
  BFD_ASSERT (! aux->is_sym);
#ifdef RS6000COFF_C
  if (CSECT_SYM_P (symbol->u.syment.n_sclass)
      && indaux + 1 == symbol->u.syment.n_numaux)
    {
      /* This is a csect entry.  */
      fprintf (file, "AUX ");
      if (SMTYP_SMTYP (aux->u.auxent.x_csect.x_smtyp) != XTY_LD)
	{
	  BFD_ASSERT (! aux->fix_scnlen);
	  fprintf (file, "val %5" PRIu64,
		   aux->u.auxent.x_csect.x_scnlen.u64);
	}
      else
	{
	  fprintf (file, "indx ");
	  if (! aux->fix_scnlen)
	    fprintf (file, "%4" PRIu64,
		     aux->u.auxent.x_csect.x_scnlen.u64);
	  else
	    fprintf (file, "%4ld",
		     (long) (aux->u.auxent.x_csect.x_scnlen.p - table_base));
	}
      fprintf (file,
	       " prmhsh %u snhsh %u typ %d algn %d clss %u stb %u snstb %u",
	       aux->u.auxent.x_csect.x_parmhash,
	       (unsigned int) aux->u.auxent.x_csect.x_snhash,
	       SMTYP_SMTYP (aux->u.auxent.x_csect.x_smtyp),
	       SMTYP_ALIGN (aux->u.auxent.x_csect.x_smtyp),
	       (unsigned int) aux->u.auxent.x_csect.x_smclas,
	       aux->u.auxent.x_csect.x_stab,
	       (unsigned int) aux->u.auxent.x_csect.x_snstab);
      return true;
    }
#endif

  /* Return FALSE to indicate that no special action was taken.  */
  return false;
}

/*
SUBSUBSECTION
	Writing relocations

	To write relocations, the back end steps though the
	canonical relocation table and create an
	@code{internal_reloc}. The symbol index to use is removed from
	the @code{offset} field in the symbol table supplied.  The
	address comes directly from the sum of the section base
	address and the relocation offset; the type is dug directly
	from the howto field.  Then the @code{internal_reloc} is
	swapped into the shape of an @code{external_reloc} and written
	out to disk.

*/

#ifdef TARG_AUX


/* AUX's ld wants relocations to be sorted.  */
static int
compare_arelent_ptr (const void * x, const void * y)
{
  const arelent **a = (const arelent **) x;
  const arelent **b = (const arelent **) y;
  bfd_size_type aadr = (*a)->address;
  bfd_size_type badr = (*b)->address;

  return (aadr < badr ? -1 : badr < aadr ? 1 : 0);
}

#endif /* TARG_AUX */

static bool
coff_write_relocs (bfd * abfd, int first_undef)
{
  asection *s;

  for (s = abfd->sections; s != NULL; s = s->next)
    {
      unsigned int i;
      struct external_reloc dst;
      arelent **p;

#ifndef TARG_AUX
      p = s->orelocation;
#else
      {
	/* Sort relocations before we write them out.  */
	bfd_size_type amt;

	amt = s->reloc_count;
	amt *= sizeof (arelent *);
	p = bfd_malloc (amt);
	if (p == NULL)
	  {
	    if (s->reloc_count > 0)
	      return false;
	  }
	else
	  {
	    memcpy (p, s->orelocation, (size_t) amt);
	    qsort (p, s->reloc_count, sizeof (arelent *), compare_arelent_ptr);
	  }
      }
#endif

      if (bfd_seek (abfd, s->rel_filepos, SEEK_SET) != 0)
	return false;

#ifdef COFF_WITH_EXTENDED_RELOC_COUNTER
      if ((obj_pe (abfd) || obj_go32 (abfd)) && s->reloc_count >= 0xffff)
	{
	  /* Encode real count here as first reloc.  */
	  struct internal_reloc n;

	  memset (& n, 0, sizeof (n));
	  /* Add one to count *this* reloc (grr).  */
	  n.r_vaddr = s->reloc_count + 1;
	  coff_swap_reloc_out (abfd, &n, &dst);
	  if (bfd_bwrite (& dst, (bfd_size_type) bfd_coff_relsz (abfd),
			  abfd) != bfd_coff_relsz (abfd))
	    return false;
	}
#endif

      for (i = 0; i < s->reloc_count; i++)
	{
	  struct internal_reloc n;
	  arelent *q = p[i];

	  memset (& n, 0, sizeof (n));

	  /* Now we've renumbered the symbols we know where the
	     undefined symbols live in the table.  Check the reloc
	     entries for symbols who's output bfd isn't the right one.
	     This is because the symbol was undefined (which means
	     that all the pointers are never made to point to the same
	     place). This is a bad thing,'cause the symbols attached
	     to the output bfd are indexed, so that the relocation
	     entries know which symbol index they point to.  So we
	     have to look up the output symbol here.  */

	  if (q->sym_ptr_ptr[0] != NULL && q->sym_ptr_ptr[0]->the_bfd != abfd)
	    {
	      int j;
	      const char *sname = q->sym_ptr_ptr[0]->name;
	      asymbol **outsyms = abfd->outsymbols;

	      for (j = first_undef; outsyms[j]; j++)
		{
		  const char *intable = outsyms[j]->name;

		  if (strcmp (intable, sname) == 0)
		    {
		      /* Got a hit, so repoint the reloc.  */
		      q->sym_ptr_ptr = outsyms + j;
		      break;
		    }
		}
	    }

	  n.r_vaddr = q->address + s->vma;

#ifdef R_IHCONST
	  /* The 29k const/consth reloc pair is a real kludge.  The consth
	     part doesn't have a symbol; it has an offset.  So rebuilt
	     that here.  */
	  if (q->howto->type == R_IHCONST)
	    n.r_symndx = q->addend;
	  else
#endif
	    if (q->sym_ptr_ptr && q->sym_ptr_ptr[0] != NULL)
	      {
#ifdef SECTION_RELATIVE_ABSOLUTE_SYMBOL_P
		if (SECTION_RELATIVE_ABSOLUTE_SYMBOL_P (q, s))
#else
		if ((*q->sym_ptr_ptr)->section == bfd_abs_section_ptr
		    && ((*q->sym_ptr_ptr)->flags & BSF_SECTION_SYM) != 0)
#endif
		  /* This is a relocation relative to the absolute symbol.  */
		  n.r_symndx = -1;
		else
		  {
		    n.r_symndx = get_index ((*(q->sym_ptr_ptr)));
		    /* Check to see if the symbol reloc points to a symbol
		       we don't have in our symbol table.  */
		    if (n.r_symndx > obj_conv_table_size (abfd))
		      {
			bfd_set_error (bfd_error_bad_value);
			/* xgettext:c-format */
			_bfd_error_handler (_("%pB: reloc against a non-existent"
					      " symbol index: %ld"),
					    abfd, n.r_symndx);
			return false;
		      }
		  }
	      }

#ifdef SWAP_OUT_RELOC_OFFSET
	  n.r_offset = q->addend;
#endif

#ifdef SELECT_RELOC
	  /* Work out reloc type from what is required.  */
	  if (q->howto)
	    SELECT_RELOC (n, q->howto);
#else
	  if (q->howto)
	    n.r_type = q->howto->type;
#endif
	  coff_swap_reloc_out (abfd, &n, &dst);

	  if (bfd_bwrite (& dst, (bfd_size_type) bfd_coff_relsz (abfd),
			 abfd) != bfd_coff_relsz (abfd))
	    return false;
	}

#ifdef TARG_AUX
      free (p);
#endif
    }

  return true;
}

/* Set flags and magic number of a coff file from architecture and machine
   type.  Result is TRUE if we can represent the arch&type, FALSE if not.  */

static bool
coff_set_flags (bfd * abfd,
		unsigned int *magicp ATTRIBUTE_UNUSED,
		unsigned short *flagsp ATTRIBUTE_UNUSED)
{
  switch (bfd_get_arch (abfd))
    {
#ifdef Z80MAGIC
    case bfd_arch_z80:
      *magicp = Z80MAGIC;
      switch (bfd_get_mach (abfd))
	{
	case bfd_mach_z80strict:
	case bfd_mach_z80:
	case bfd_mach_z80n:
	case bfd_mach_z80full:
	case bfd_mach_r800:
	case bfd_mach_gbz80:
	case bfd_mach_z180:
	case bfd_mach_ez80_z80:
	case bfd_mach_ez80_adl:
	  *flagsp = bfd_get_mach (abfd) << 12;
	  break;
	default:
	  return false;
	}
      return true;
#endif

#ifdef Z8KMAGIC
    case bfd_arch_z8k:
      *magicp = Z8KMAGIC;

      switch (bfd_get_mach (abfd))
	{
	case bfd_mach_z8001: *flagsp = F_Z8001;	break;
	case bfd_mach_z8002: *flagsp = F_Z8002;	break;
	default:	     return false;
	}
      return true;
#endif

#ifdef TIC30MAGIC
    case bfd_arch_tic30:
      *magicp = TIC30MAGIC;
      return true;
#endif

#ifdef TICOFF_DEFAULT_MAGIC
    case TICOFF_TARGET_ARCH:
      /* If there's no indication of which version we want, use the default.  */
      if (!abfd->xvec )
	*magicp = TICOFF_DEFAULT_MAGIC;
      else
	{
	  /* We may want to output in a different COFF version.  */
	  switch (abfd->xvec->name[4])
	    {
	    case '0':
	      *magicp = TICOFF0MAGIC;
	      break;
	    case '1':
	      *magicp = TICOFF1MAGIC;
	      break;
	    case '2':
	      *magicp = TICOFF2MAGIC;
	      break;
	    default:
	      return false;
	    }
	}
      TICOFF_TARGET_MACHINE_SET (flagsp, bfd_get_mach (abfd));
      return true;
#endif

#ifdef AARCH64MAGIC
    case bfd_arch_aarch64:
      * magicp = AARCH64MAGIC;
      return true;
#endif

#ifdef LOONGARCH64MAGIC
    case bfd_arch_loongarch:
      * magicp = LOONGARCH64MAGIC;
      return true;
#endif

#ifdef ARMMAGIC
    case bfd_arch_arm:
#ifdef ARM_WINCE
      * magicp = ARMPEMAGIC;
#else
      * magicp = ARMMAGIC;
#endif
      * flagsp = 0;
      if (APCS_SET (abfd))
	{
	  if (APCS_26_FLAG (abfd))
	    * flagsp |= F_APCS26;

	  if (APCS_FLOAT_FLAG (abfd))
	    * flagsp |= F_APCS_FLOAT;

	  if (PIC_FLAG (abfd))
	    * flagsp |= F_PIC;
	}
      if (INTERWORK_SET (abfd) && INTERWORK_FLAG (abfd))
	* flagsp |= F_INTERWORK;
      switch (bfd_get_mach (abfd))
	{
	case bfd_mach_arm_2:  * flagsp |= F_ARM_2;  break;
	case bfd_mach_arm_2a: * flagsp |= F_ARM_2a; break;
	case bfd_mach_arm_3:  * flagsp |= F_ARM_3;  break;
	case bfd_mach_arm_3M: * flagsp |= F_ARM_3M; break;
	case bfd_mach_arm_4:  * flagsp |= F_ARM_4;  break;
	case bfd_mach_arm_4T: * flagsp |= F_ARM_4T; break;
	case bfd_mach_arm_5:  * flagsp |= F_ARM_5;  break;
	  /* FIXME: we do not have F_ARM vaues greater than F_ARM_5.
	     See also the comment in coff_set_arch_mach_hook().  */
	case bfd_mach_arm_5T: * flagsp |= F_ARM_5;  break;
	case bfd_mach_arm_5TE: * flagsp |= F_ARM_5; break;
	case bfd_mach_arm_XScale: * flagsp |= F_ARM_5; break;
	}
      return true;
#endif

#if defined(I386MAGIC) || defined(AMD64MAGIC)
    case bfd_arch_i386:
#if defined(I386MAGIC)
      *magicp = I386MAGIC;
#endif
#if defined LYNXOS
      /* Just overwrite the usual value if we're doing Lynx.  */
      *magicp = LYNXCOFFMAGIC;
#endif
#if defined AMD64MAGIC
      *magicp = AMD64MAGIC;
#endif
      return true;
#endif

#ifdef IA64MAGIC
    case bfd_arch_ia64:
      *magicp = IA64MAGIC;
      return true;
#endif

#ifdef SH_ARCH_MAGIC_BIG
    case bfd_arch_sh:
#ifdef COFF_IMAGE_WITH_PE
      *magicp = SH_ARCH_MAGIC_WINCE;
#else
      if (bfd_big_endian (abfd))
	*magicp = SH_ARCH_MAGIC_BIG;
      else
	*magicp = SH_ARCH_MAGIC_LITTLE;
#endif
      return true;
#endif

#ifdef MIPS_ARCH_MAGIC_WINCE
    case bfd_arch_mips:
      *magicp = MIPS_ARCH_MAGIC_WINCE;
      return true;
#endif

#ifdef SPARCMAGIC
    case bfd_arch_sparc:
      *magicp = SPARCMAGIC;
#ifdef LYNXOS
      /* Just overwrite the usual value if we're doing Lynx.  */
      *magicp = LYNXCOFFMAGIC;
#endif
      return true;
#endif

#ifdef RS6000COFF_C
    case bfd_arch_rs6000:
    case bfd_arch_powerpc:
      BFD_ASSERT (bfd_get_flavour (abfd) == bfd_target_xcoff_flavour);
      *magicp = bfd_xcoff_magic_number (abfd);
      return true;
#endif

#ifdef MCOREMAGIC
    case bfd_arch_mcore:
      * magicp = MCOREMAGIC;
      return true;
#endif

    default:			/* Unknown architecture.  */
      break;
    }

  return false;
}

static bool
coff_set_arch_mach (bfd * abfd,
		    enum bfd_architecture arch,
		    unsigned long machine)
{
  unsigned dummy1;
  unsigned short dummy2;

  if (! bfd_default_set_arch_mach (abfd, arch, machine))
    return false;

  if (arch != bfd_arch_unknown
      && ! coff_set_flags (abfd, &dummy1, &dummy2))
    return false;		/* We can't represent this type.  */

  return true;			/* We're easy...  */
}

#ifdef COFF_IMAGE_WITH_PE

/* This is used to sort sections by VMA, as required by PE image
   files.  */

static int
sort_by_secaddr (const void * arg1, const void * arg2)
{
  const asection *a = *(const asection **) arg1;
  const asection *b = *(const asection **) arg2;

  if (a->vma < b->vma)
    return -1;
  else if (a->vma > b->vma)
    return 1;

  return 0;
}

#endif /* COFF_IMAGE_WITH_PE */

/* Calculate the file position for each section.  */

#define ALIGN_SECTIONS_IN_FILE
#ifdef TICOFF
#undef ALIGN_SECTIONS_IN_FILE
#endif

static bool
coff_compute_section_file_positions (bfd * abfd)
{
  asection *current;
  file_ptr sofar = bfd_coff_filhsz (abfd);
  bool align_adjust;
  unsigned int target_index;
#ifdef ALIGN_SECTIONS_IN_FILE
  asection *previous = NULL;
  file_ptr old_sofar;
#endif

#ifdef COFF_IMAGE_WITH_PE
  unsigned int page_size;

  if (coff_data (abfd)->link_info
      || (pe_data (abfd) && pe_data (abfd)->pe_opthdr.FileAlignment))
    {
      page_size = pe_data (abfd)->pe_opthdr.FileAlignment;

      /* If no file alignment has been set, default to one.
	 This repairs 'ld -r' for arm-wince-pe target.  */
      if (page_size == 0)
	page_size = 1;
    }
  else
    page_size = PE_DEF_FILE_ALIGNMENT;
#else
#ifdef COFF_PAGE_SIZE
  unsigned int page_size = COFF_PAGE_SIZE;
#endif
#endif

#ifdef RS6000COFF_C
  /* On XCOFF, if we have symbols, set up the .debug section.  */
  if (bfd_get_symcount (abfd) > 0)
    {
      bfd_size_type sz;
      bfd_size_type i, symcount;
      asymbol **symp;

      sz = 0;
      symcount = bfd_get_symcount (abfd);
      for (symp = abfd->outsymbols, i = 0; i < symcount; symp++, i++)
	{
	  coff_symbol_type *cf;

	  cf = coff_symbol_from (*symp);
	  if (cf != NULL
	      && cf->native != NULL
	      && cf->native->is_sym
	      && SYMNAME_IN_DEBUG (&cf->native->u.syment))
	    {
	      size_t len;

	      len = strlen (bfd_asymbol_name (*symp));
	      if (len > SYMNMLEN || bfd_coff_force_symnames_in_strings (abfd))
		sz += len + 1 + bfd_coff_debug_string_prefix_length (abfd);
	    }
	}
      if (sz > 0)
	{
	  asection *dsec;

	  dsec = bfd_make_section_old_way (abfd, DOT_DEBUG);
	  if (dsec == NULL)
	    abort ();
	  dsec->size = sz;
	  dsec->flags |= SEC_HAS_CONTENTS;
	}
    }
#endif

  if (bfd_get_start_address (abfd))
    /*  A start address may have been added to the original file. In this
	case it will need an optional header to record it.  */
    abfd->flags |= EXEC_P;

  if (abfd->flags & EXEC_P)
    sofar += bfd_coff_aoutsz (abfd);
#ifdef RS6000COFF_C
  else if (xcoff_data (abfd)->full_aouthdr)
    sofar += bfd_coff_aoutsz (abfd);
  else
    sofar += SMALL_AOUTSZ;
#endif

  sofar += abfd->section_count * bfd_coff_scnhsz (abfd);

#ifdef RS6000COFF_C
  /* XCOFF handles overflows in the reloc and line number count fields
     by allocating a new section header to hold the correct counts.  */
  for (current = abfd->sections; current != NULL; current = current->next)
    if (current->reloc_count >= 0xffff || current->lineno_count >= 0xffff)
      sofar += bfd_coff_scnhsz (abfd);
#endif

  if (coff_data (abfd)->section_by_target_index)
    htab_empty (coff_data (abfd)->section_by_target_index);

#ifdef COFF_IMAGE_WITH_PE
  {
    /* PE requires the sections to be in memory order when listed in
       the section headers.  It also does not like empty loadable
       sections.  The sections apparently do not have to be in the
       right order in the image file itself, but we do need to get the
       target_index values right.  */

    unsigned int count;
    asection **section_list;
    unsigned int i;
    bfd_size_type amt;

#ifdef COFF_PAGE_SIZE
    /* Clear D_PAGED if section / file alignment aren't suitable for
       paging at COFF_PAGE_SIZE granularity.  */
   if (pe_data (abfd)->pe_opthdr.SectionAlignment < COFF_PAGE_SIZE
       || page_size < COFF_PAGE_SIZE)
     abfd->flags &= ~D_PAGED;
#endif

    count = 0;
    for (current = abfd->sections; current != NULL; current = current->next)
      ++count;

    /* We allocate an extra cell to simplify the final loop.  */
    amt = sizeof (struct asection *) * (count + 1);
    section_list = (asection **) bfd_malloc (amt);
    if (section_list == NULL)
      return false;

    i = 0;
    for (current = abfd->sections; current != NULL; current = current->next)
      {
	section_list[i] = current;
	++i;
      }
    section_list[i] = NULL;

    qsort (section_list, count, sizeof (asection *), sort_by_secaddr);

    /* Rethread the linked list into sorted order; at the same time,
       assign target_index values.  */
    target_index = 1;
    abfd->sections = NULL;
    abfd->section_last = NULL;
    for (i = 0; i < count; i++)
      {
	current = section_list[i];
	bfd_section_list_append (abfd, current);

	/* Later, if the section has zero size, we'll be throwing it
	   away, so we don't want to number it now.  Note that having
	   a zero size and having real contents are different
	   concepts: .bss has no contents, but (usually) non-zero
	   size.  */
	if (current->size == 0)
	  {
	    /* Discard.  However, it still might have (valid) symbols
	       in it, so arbitrarily set it to section 1 (indexing is
	       1-based here; usually .text).  __end__ and other
	       contents of .endsection really have this happen.
	       FIXME: This seems somewhat dubious.  */
	    current->target_index = 1;
	  }
	else
	  current->target_index = target_index++;
      }

    free (section_list);
  }
#else /* ! COFF_IMAGE_WITH_PE */
  {
    /* Set the target_index field.  */
    target_index = 1;
    for (current = abfd->sections; current != NULL; current = current->next)
      current->target_index = target_index++;
  }
#endif /* ! COFF_IMAGE_WITH_PE */

  if (target_index >= bfd_coff_max_nscns (abfd))
    {
      bfd_set_error (bfd_error_file_too_big);
      _bfd_error_handler
	/* xgettext:c-format */
	(_("%pB: too many sections (%d)"), abfd, target_index);
      return false;
    }

  align_adjust = false;
  for (current = abfd->sections;
       current != NULL;
       current = current->next)
    {
#ifdef COFF_IMAGE_WITH_PE
      /* With PE we have to pad each section to be a multiple of its
	 page size too, and remember both sizes.  */
      if (coff_section_data (abfd, current) == NULL)
	{
	  size_t amt = sizeof (struct coff_section_tdata);

	  current->used_by_bfd = bfd_zalloc (abfd, amt);
	  if (current->used_by_bfd == NULL)
	    return false;
	}
      if (pei_section_data (abfd, current) == NULL)
	{
	  size_t amt = sizeof (struct pei_section_tdata);

	  coff_section_data (abfd, current)->tdata = bfd_zalloc (abfd, amt);
	  if (coff_section_data (abfd, current)->tdata == NULL)
	    return false;
	}
      if (pei_section_data (abfd, current)->virt_size == 0)
	pei_section_data (abfd, current)->virt_size = current->size;
#endif

      /* Only deal with sections which have contents.  */
      if (!(current->flags & SEC_HAS_CONTENTS))
	continue;

      current->rawsize = current->size;

#ifdef COFF_IMAGE_WITH_PE
      /* Make sure we skip empty sections in a PE image.  */
      if (current->size == 0)
	continue;
#endif

      /* Align the sections in the file to the same boundary on
	 which they are aligned in virtual memory.  */
#ifdef ALIGN_SECTIONS_IN_FILE
      if ((abfd->flags & EXEC_P) != 0)
	{
	  /* Make sure this section is aligned on the right boundary - by
	     padding the previous section up if necessary.  */
	  old_sofar = sofar;

#ifdef COFF_IMAGE_WITH_PE
	  sofar = BFD_ALIGN (sofar, page_size);
#else
	  sofar = BFD_ALIGN (sofar, (bfd_vma) 1 << current->alignment_power);
#endif

#ifdef RS6000COFF_C
	  /* Make sure the file offset and the vma of .text/.data are at the
	     same page offset, so that the file can be mmap'ed without being
	     relocated.  Failing that, AIX is able to load and execute the
	     program, but it will be silently relocated (possible as
	     executables are PIE).  But the relocation is slightly costly and
	     complexify the use of addr2line or gdb.  So better to avoid it,
	     like does the native linker.  Usually gnu ld makes sure that
	     the vma of .text is the file offset so this issue shouldn't
	     appear unless you are stripping such an executable.

	     AIX loader checks the text section alignment of (vma - filepos),
	     and the native linker doesn't try to align the text sections.
	     For example:

	     0 .text	     000054cc  10000128	 10000128  00000128  2**5
			     CONTENTS, ALLOC, LOAD, CODE

	     Don't perform the above tweak if the previous one is .tdata,
	     as it will increase the memory allocated for every threads
	     created and not just improve performances with gdb.
	  */

	  if ((!strcmp (current->name, _TEXT)
	       || !strcmp (current->name, _DATA))
	      && (previous == NULL || strcmp(previous->name, _TDATA)))
	    {
	      bfd_vma align = 4096;
	      bfd_vma sofar_off = sofar % align;
	      bfd_vma vma_off = current->vma % align;

	      if (vma_off > sofar_off)
		sofar += vma_off - sofar_off;
	      else if (vma_off < sofar_off)
		sofar += align + vma_off - sofar_off;
	    }
#endif
	  if (previous != NULL)
	    previous->size += sofar - old_sofar;
	}

#endif

      /* In demand paged files the low order bits of the file offset
	 must match the low order bits of the virtual address.  */
#ifdef COFF_PAGE_SIZE
      if ((abfd->flags & D_PAGED) != 0
	  && (current->flags & SEC_ALLOC) != 0)
	sofar += (current->vma - (bfd_vma) sofar) % page_size;
#endif
      current->filepos = sofar;

#ifdef COFF_IMAGE_WITH_PE
      /* Set the padded size.  */
      current->size = (current->size + page_size - 1) & -page_size;
#endif

      sofar += current->size;

#ifdef ALIGN_SECTIONS_IN_FILE
      /* Make sure that this section is of the right size too.  */
      if ((abfd->flags & EXEC_P) == 0)
	{
	  bfd_size_type old_size;

	  old_size = current->size;
	  current->size = BFD_ALIGN (current->size,
				     (bfd_vma) 1 << current->alignment_power);
	  align_adjust = current->size != old_size;
	  sofar += current->size - old_size;
	}
      else
	{
	  old_sofar = sofar;
#ifdef COFF_IMAGE_WITH_PE
	  sofar = BFD_ALIGN (sofar, page_size);
#else
	  sofar = BFD_ALIGN (sofar, (bfd_vma) 1 << current->alignment_power);
#endif
	  align_adjust = sofar != old_sofar;
	  current->size += sofar - old_sofar;
	}
#endif

#ifdef COFF_IMAGE_WITH_PE
      /* For PE we need to make sure we pad out to the aligned
	 size, in case the caller only writes out data to the
	 unaligned size.  */
      if (pei_section_data (abfd, current)->virt_size < current->size)
	align_adjust = true;
#endif

#ifdef _LIB
      /* Force .lib sections to start at zero.  The vma is then
	 incremented in coff_set_section_contents.  This is right for
	 SVR3.2.  */
      if (strcmp (current->name, _LIB) == 0)
	bfd_set_section_vma (current, 0);
#endif

#ifdef ALIGN_SECTIONS_IN_FILE
      previous = current;
#endif
    }

  /* It is now safe to write to the output file.  If we needed an
     alignment adjustment for the last section, then make sure that
     there is a byte at offset sofar.  If there are no symbols and no
     relocs, then nothing follows the last section.  If we don't force
     the last byte out, then the file may appear to be truncated.  */
  if (align_adjust)
    {
      bfd_byte b;

      b = 0;
      if (bfd_seek (abfd, sofar - 1, SEEK_SET) != 0
	  || bfd_bwrite (&b, (bfd_size_type) 1, abfd) != 1)
	return false;
    }

  /* Make sure the relocations are aligned.  We don't need to make
     sure that this byte exists, because it will only matter if there
     really are relocs.  */
  sofar = BFD_ALIGN (sofar,
		     (bfd_vma) 1 << COFF_DEFAULT_SECTION_ALIGNMENT_POWER);

  obj_relocbase (abfd) = sofar;
  abfd->output_has_begun = true;

  return true;
}

#ifdef COFF_IMAGE_WITH_PE

static bool
coff_read_word (bfd *abfd, unsigned int *value, unsigned int *pelength)
{
  unsigned char b[2];
  int status;

  status = bfd_bread (b, (bfd_size_type) 2, abfd);
  if (status < 1)
    {
      *value = 0;
      return false;
    }

  if (status == 1)
    *value = (unsigned int) b[0];
  else
    *value = (unsigned int) (b[0] + (b[1] << 8));

  *pelength += status;

  return true;
}

/* Read a two byte number from buffer B returning the result in VALUE.
   No more than BUF_SIZE bytes will be read.
   Returns true upobn success, false otherwise.
   If successful, increases the value stored in PELENGTH by the number
   of bytes read.  */

static bool
coff_read_word_from_buffer (unsigned char *  b,
			    int              buf_size,
                            unsigned int *   value,
			    unsigned int *   pelength)
{
  if (buf_size < 1)
    {
      *value = 0;
      return false;
    }

  if (buf_size == 1)
    {
      *value = (unsigned int)b[0];
      *pelength += 1;
    }
  else
    {
      *value = (unsigned int)(b[0] + (b[1] << 8));
      *pelength += 2;
    }

  return true;
}

#define COFF_CHECKSUM_BUFFER_SIZE 0x800000

static unsigned int
coff_compute_checksum (bfd *abfd, unsigned int *pelength)
{
  file_ptr filepos;
  unsigned int value;
  unsigned int total;
  unsigned char *buf;
  int buf_size;

  total = 0;
  *pelength = 0;
  filepos = (file_ptr) 0;
  buf = (unsigned char *) bfd_malloc (COFF_CHECKSUM_BUFFER_SIZE);
  if (buf == NULL)
    return 0;
  buf_size = 0;

  do
    {
      unsigned char *cur_buf;
      int cur_buf_size;

      if (bfd_seek (abfd, filepos, SEEK_SET) != 0)
	return 0;

      buf_size = bfd_bread (buf, COFF_CHECKSUM_BUFFER_SIZE, abfd);
      cur_buf_size = buf_size;
      cur_buf = buf;

      while (cur_buf_size > 0)
        {
          coff_read_word_from_buffer (cur_buf, cur_buf_size, &value, pelength);
          cur_buf += 2;
          cur_buf_size -= 2;
          total += value;
          total = 0xffff & (total + (total >> 0x10));
        }

      filepos += buf_size;
    }
  while (buf_size > 0);

  free (buf);

  return (0xffff & (total + (total >> 0x10)));
}

static bool
coff_apply_checksum (bfd *abfd)
{
  unsigned int computed;
  unsigned int checksum = 0;
  unsigned int peheader;
  unsigned int pelength;

  if (bfd_seek (abfd, 0x3c, SEEK_SET) != 0)
    return false;

  if (!coff_read_word (abfd, &peheader, &pelength))
    return false;

  if (bfd_seek (abfd, peheader + 0x58, SEEK_SET) != 0)
    return false;

  checksum = 0;
  bfd_bwrite (&checksum, (bfd_size_type) 4, abfd);

  if (bfd_seek (abfd, peheader, SEEK_SET) != 0)
    return false;

  computed = coff_compute_checksum (abfd, &pelength);

  checksum = computed + pelength;

  if (bfd_seek (abfd, peheader + 0x58, SEEK_SET) != 0)
    return false;

  bfd_bwrite (&checksum, (bfd_size_type) 4, abfd);

  return true;
}

#endif /* COFF_IMAGE_WITH_PE */

static bool
coff_write_object_contents (bfd * abfd)
{
  asection *current;
  bool hasrelocs = false;
  bool haslinno = false;
#ifdef COFF_IMAGE_WITH_PE
  bool hasdebug = false;
#endif
  file_ptr scn_base;
  file_ptr reloc_base;
  file_ptr lineno_base;
  file_ptr sym_base;
  unsigned long reloc_size = 0, reloc_count = 0;
  unsigned long lnno_size = 0;
  bool long_section_names;
  asection *text_sec = NULL;
  asection *data_sec = NULL;
  asection *bss_sec = NULL;
#ifdef RS6000COFF_C
  asection *tdata_sec = NULL;
  asection *tbss_sec = NULL;
#endif
  struct internal_filehdr internal_f;
  struct internal_aouthdr internal_a;
#ifdef COFF_LONG_SECTION_NAMES
  size_t string_size = STRING_SIZE_SIZE;
#endif

  bfd_set_error (bfd_error_system_call);

  /* Make a pass through the symbol table to count line number entries and
     put them into the correct asections.  */
  lnno_size = coff_count_linenumbers (abfd) * bfd_coff_linesz (abfd);

  if (! abfd->output_has_begun)
    {
      if (! coff_compute_section_file_positions (abfd))
	return false;
    }

  reloc_base = obj_relocbase (abfd);

  /* Work out the size of the reloc and linno areas.  */

  for (current = abfd->sections; current != NULL; current =
       current->next)
    {
#ifdef COFF_WITH_EXTENDED_RELOC_COUNTER
      /* We store the actual reloc count in the first reloc's addr.  */
      if ((obj_pe (abfd) || obj_go32 (abfd)) && current->reloc_count >= 0xffff)
	reloc_count ++;
#endif
      reloc_count += current->reloc_count;
    }

  reloc_size = reloc_count * bfd_coff_relsz (abfd);

  lineno_base = reloc_base + reloc_size;
  sym_base = lineno_base + lnno_size;

  /* Indicate in each section->line_filepos its actual file address.  */
  for (current = abfd->sections; current != NULL; current =
       current->next)
    {
      if (current->lineno_count)
	{
	  current->line_filepos = lineno_base;
	  current->moving_line_filepos = lineno_base;
	  lineno_base += current->lineno_count * bfd_coff_linesz (abfd);
	}
      else
	current->line_filepos = 0;

      if (current->reloc_count)
	{
	  current->rel_filepos = reloc_base;
	  reloc_base += current->reloc_count * bfd_coff_relsz (abfd);
#ifdef COFF_WITH_EXTENDED_RELOC_COUNTER
	  /* Extra reloc to hold real count.  */
	  if ((obj_pe (abfd) || obj_go32 (abfd)) && current->reloc_count >= 0xffff)
	    reloc_base += bfd_coff_relsz (abfd);
#endif
	}
      else
	current->rel_filepos = 0;
    }

  /* Write section headers to the file.  */
  internal_f.f_nscns = 0;

  if ((abfd->flags & EXEC_P) != 0)
    scn_base = bfd_coff_filhsz (abfd) + bfd_coff_aoutsz (abfd);
  else
    {
      scn_base = bfd_coff_filhsz (abfd);
#ifdef RS6000COFF_C
#ifndef XCOFF64
      if (xcoff_data (abfd)->full_aouthdr)
	scn_base += bfd_coff_aoutsz (abfd);
      else
	scn_base += SMALL_AOUTSZ;
#endif
#endif
    }

  if (bfd_seek (abfd, scn_base, SEEK_SET) != 0)
    return false;

  long_section_names = false;
  for (current = abfd->sections;
       current != NULL;
       current = current->next)
    {
      struct internal_scnhdr section;
#ifdef COFF_IMAGE_WITH_PE
      bool is_reloc_section = false;

      if (strcmp (current->name, DOT_RELOC) == 0)
	{
	  is_reloc_section = true;
	  hasrelocs = true;
	  pe_data (abfd)->has_reloc_section = 1;
	}
#endif

      internal_f.f_nscns++;

      strncpy (section.s_name, current->name, SCNNMLEN);

#ifdef COFF_LONG_SECTION_NAMES
      /* Handle long section names as in PE.  This must be compatible
	 with the code in coff_write_symbols and _bfd_coff_final_link.  */
      if (bfd_coff_long_section_names (abfd))
	{
	  size_t len;

	  len = strlen (current->name);
	  if (len > SCNNMLEN)
	    {

	      /* An inherent limitation of the /nnnnnnn notation used to indicate
		 the offset of the long name in the string table is that we
		 cannot address entries beyone the ten million byte boundary.  */
	      if (string_size < 10000000)
		{
		  /* The s_name field is defined to be NUL-padded but need not
		     be NUL-terminated.  We use a temporary buffer so that we
		     can still sprintf all eight chars without splatting a
		     terminating NUL over the first byte of the following
		     member (s_paddr).  */
		  /* PR 21096: The +20 is to stop a bogus warning from gcc7
		     about a possible buffer overflow.  */
		  char s_name_buf[SCNNMLEN + 1 + 20];

		  /* We do not need to use snprintf here as we have already
		     verified that string_size is not too big, plus we have
		     an overlarge buffer, just in case.  */
		  sprintf (s_name_buf, "/%lu", (unsigned long) string_size);
		  /* Then strncpy takes care of any padding for us.  */
		  strncpy (section.s_name, s_name_buf, SCNNMLEN);
		}
	      else
#ifdef COFF_WITH_PE
		{
		  /* PE use a base 64 encoding for long section names whose
		     index is very large.  But contrary to RFC 4648, there is
		     no padding: 6 characters must be generated.  */
		  static const char base64[] =
		    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		    "abcdefghijklmnopqrstuvwxyz"
		    "0123456789+/";
		  unsigned long off = string_size;
		  unsigned i;

		  section.s_name[0] = '/';
		  section.s_name[1] = '/';
		  for (i = SCNNMLEN - 1; i >= 2; i--)
		    {
		      section.s_name[i] = base64[off & 0x3f];
		      off >>= 6;
		    }
		}
#endif
	      if (string_size > 0xffffffffUL - (len + 1)
#ifndef COFF_WITH_PE
		  || string_size >= 10000000
#endif
		  )
		{
		  bfd_set_error (bfd_error_file_too_big);
		  _bfd_error_handler
		    /* xgettext:c-format */
		    (_("%pB: section %pA: string table overflow at offset %ld"),
		    abfd, current, (unsigned long) string_size);
		  return false;
		}

	      string_size += len + 1;
	      long_section_names = true;
	    }
	}
#endif

#ifdef _LIB
      /* Always set s_vaddr of .lib to 0.  This is right for SVR3.2
	 Ian Taylor <ian@cygnus.com>.  */
      if (strcmp (current->name, _LIB) == 0)
	section.s_vaddr = 0;
      else
#endif
      section.s_vaddr = current->vma;
      section.s_paddr = current->lma;
      section.s_size =  current->size;
#ifdef coff_get_section_load_page
      section.s_page = coff_get_section_load_page (current);
#else
      section.s_page = 0;
#endif

#ifdef COFF_WITH_PE
      section.s_paddr = 0;
#endif
#ifdef COFF_IMAGE_WITH_PE
      /* Reminder: s_paddr holds the virtual size of the section.  */
      if (coff_section_data (abfd, current) != NULL
	  && pei_section_data (abfd, current) != NULL)
	section.s_paddr = pei_section_data (abfd, current)->virt_size;
      else
	section.s_paddr = 0;
#endif

      /* If this section has no size or is unloadable then the scnptr
	 will be 0 too.  */
      if (current->size == 0
	  || (current->flags & (SEC_LOAD | SEC_HAS_CONTENTS)) == 0)
	section.s_scnptr = 0;
      else
	section.s_scnptr = current->filepos;

      section.s_relptr = current->rel_filepos;
      section.s_lnnoptr = current->line_filepos;
      section.s_nreloc = current->reloc_count;
      section.s_nlnno = current->lineno_count;
#ifndef COFF_IMAGE_WITH_PE
      /* In PEI, relocs come in the .reloc section.  */
      if (current->reloc_count != 0)
	hasrelocs = true;
#endif
      if (current->lineno_count != 0)
	haslinno = true;
#ifdef COFF_IMAGE_WITH_PE
      if ((current->flags & SEC_DEBUGGING) != 0
	  && ! is_reloc_section)
	hasdebug = true;
#endif

#ifdef RS6000COFF_C
#ifndef XCOFF64
      /* Indicate the use of an XCOFF overflow section header.  */
      if (current->reloc_count >= 0xffff || current->lineno_count >= 0xffff)
	{
	  section.s_nreloc = 0xffff;
	  section.s_nlnno = 0xffff;
	}
#endif
#endif

      section.s_flags = sec_to_styp_flags (current->name, current->flags);

      if (!strcmp (current->name, _TEXT))
	text_sec = current;
      else if (!strcmp (current->name, _DATA))
	data_sec = current;
      else if (!strcmp (current->name, _BSS))
	bss_sec = current;
#ifdef RS6000COFF_C
      else if (!strcmp (current->name, _TDATA))
	tdata_sec = current;
      else if (!strcmp (current->name, _TBSS))
	tbss_sec = current;
#endif


#ifdef COFF_ENCODE_ALIGNMENT
      if (COFF_ENCODE_ALIGNMENT (abfd, section, current->alignment_power)
	  && (COFF_DECODE_ALIGNMENT (section.s_flags)
	      != current->alignment_power))
	{
	  bool warn = (coff_data (abfd)->link_info
		       && !bfd_link_relocatable (coff_data (abfd)->link_info));

	  _bfd_error_handler
	    /* xgettext:c-format */
	    (_("%pB:%s section %s: alignment 2**%u not representable"),
	     abfd, warn ? " warning:" : "", current->name,
	     current->alignment_power);
	  if (!warn)
	    {
	      bfd_set_error (bfd_error_nonrepresentable_section);
	      return false;
	    }
	}
#endif

#ifdef COFF_IMAGE_WITH_PE
      /* Suppress output of the sections if they are null.  ld
	 includes the bss and data sections even if there is no size
	 assigned to them.  NT loader doesn't like it if these section
	 headers are included if the sections themselves are not
	 needed.  See also coff_compute_section_file_positions.  */
      if (section.s_size == 0)
	internal_f.f_nscns--;
      else
#endif
	{
	  SCNHDR buff;
	  bfd_size_type amt = bfd_coff_scnhsz (abfd);

	  if (bfd_coff_swap_scnhdr_out (abfd, &section, &buff) == 0
	      || bfd_bwrite (& buff, amt, abfd) != amt)
	    return false;
	}

#ifdef COFF_WITH_PE
      /* PE stores COMDAT section information in the symbol table.  If
	 this section is supposed to have some COMDAT info, track down
	 the symbol in the symbol table and modify it.  */
      if ((current->flags & SEC_LINK_ONCE) != 0)
	{
	  unsigned int i, count;
	  asymbol **psym;
	  coff_symbol_type *csym = NULL;
	  asymbol **psymsec;

	  psymsec = NULL;
	  count = bfd_get_symcount (abfd);
	  for (i = 0, psym = abfd->outsymbols; i < count; i++, psym++)
	    {
	      if ((*psym)->section != current)
		continue;

	      /* Remember the location of the first symbol in this
		 section.  */
	      if (psymsec == NULL)
		psymsec = psym;

	      /* See if this is the section symbol.  */
	      if (strcmp ((*psym)->name, current->name) == 0)
		{
		  csym = coff_symbol_from (*psym);
		  if (csym == NULL
		      || csym->native == NULL
		      || ! csym->native->is_sym
		      || csym->native->u.syment.n_numaux < 1
		      || csym->native->u.syment.n_sclass != C_STAT
		      || csym->native->u.syment.n_type != T_NULL)
		    continue;

		  /* Here *PSYM is the section symbol for CURRENT.  */

		  break;
		}
	    }

	  /* Did we find it?
	     Note that we might not if we're converting the file from
	     some other object file format.  */
	  if (i < count)
	    {
	      combined_entry_type *aux;

	      /* We don't touch the x_checksum field.  The
		 x_associated field is not currently supported.  */

	      aux = csym->native + 1;
	      BFD_ASSERT (! aux->is_sym);
	      switch (current->flags & SEC_LINK_DUPLICATES)
		{
		case SEC_LINK_DUPLICATES_DISCARD:
		  aux->u.auxent.x_scn.x_comdat = IMAGE_COMDAT_SELECT_ANY;
		  break;

		case SEC_LINK_DUPLICATES_ONE_ONLY:
		  aux->u.auxent.x_scn.x_comdat =
		    IMAGE_COMDAT_SELECT_NODUPLICATES;
		  break;

		case SEC_LINK_DUPLICATES_SAME_SIZE:
		  aux->u.auxent.x_scn.x_comdat =
		    IMAGE_COMDAT_SELECT_SAME_SIZE;
		  break;

		case SEC_LINK_DUPLICATES_SAME_CONTENTS:
		  aux->u.auxent.x_scn.x_comdat =
		    IMAGE_COMDAT_SELECT_EXACT_MATCH;
		  break;
		}

	      /* The COMDAT symbol must be the first symbol from this
		 section in the symbol table.  In order to make this
		 work, we move the COMDAT symbol before the first
		 symbol we found in the search above.  It's OK to
		 rearrange the symbol table at this point, because
		 coff_renumber_symbols is going to rearrange it
		 further and fix up all the aux entries.  */
	      if (psym != psymsec)
		{
		  asymbol *hold;
		  asymbol **pcopy;

		  hold = *psym;
		  for (pcopy = psym; pcopy > psymsec; pcopy--)
		    pcopy[0] = pcopy[-1];
		  *psymsec = hold;
		}
	    }
	}
#endif /* COFF_WITH_PE */
    }

#ifdef RS6000COFF_C
#ifndef XCOFF64
  /* XCOFF handles overflows in the reloc and line number count fields
     by creating a new section header to hold the correct values.  */
  for (current = abfd->sections; current != NULL; current = current->next)
    {
      if (current->reloc_count >= 0xffff || current->lineno_count >= 0xffff)
	{
	  struct internal_scnhdr scnhdr;
	  SCNHDR buff;
	  bfd_size_type amt;

	  internal_f.f_nscns++;
	  memcpy (scnhdr.s_name, ".ovrflo", 8);
	  scnhdr.s_paddr = current->reloc_count;
	  scnhdr.s_vaddr = current->lineno_count;
	  scnhdr.s_size = 0;
	  scnhdr.s_scnptr = 0;
	  scnhdr.s_relptr = current->rel_filepos;
	  scnhdr.s_lnnoptr = current->line_filepos;
	  scnhdr.s_nreloc = current->target_index;
	  scnhdr.s_nlnno = current->target_index;
	  scnhdr.s_flags = STYP_OVRFLO;
	  amt = bfd_coff_scnhsz (abfd);
	  if (bfd_coff_swap_scnhdr_out (abfd, &scnhdr, &buff) == 0
	      || bfd_bwrite (& buff, amt, abfd) != amt)
	    return false;
	}
    }
#endif
#endif

#if defined (COFF_GO32_EXE) || defined (COFF_GO32)
  /* Pad section headers.  */
  if ((abfd->flags & EXEC_P) && abfd->sections != NULL)
    {
      file_ptr cur_ptr = scn_base
			 + abfd->section_count * bfd_coff_scnhsz (abfd);
      long fill_size = (abfd->sections->filepos - cur_ptr);
      bfd_byte *b = bfd_zmalloc (fill_size);
      if (b)
	{
	  bfd_bwrite (b, fill_size, abfd);
	  free (b);
	}
    }
#endif

  /* OK, now set up the filehdr...  */

  /* Don't include the internal abs section in the section count */

  /* We will NOT put a fucking timestamp in the header here. Every time you
     put it back, I will come in and take it out again.  I'm sorry.  This
     field does not belong here.  We fill it with a 0 so it compares the
     same but is not a reasonable time. -- gnu@cygnus.com  */
  internal_f.f_timdat = 0;
  internal_f.f_flags = 0;

  if (abfd->flags & EXEC_P)
    internal_f.f_opthdr = bfd_coff_aoutsz (abfd);
  else
    {
      internal_f.f_opthdr = 0;
#ifdef RS6000COFF_C
#ifndef XCOFF64
      if (xcoff_data (abfd)->full_aouthdr)
	internal_f.f_opthdr = bfd_coff_aoutsz (abfd);
      else
	internal_f.f_opthdr = SMALL_AOUTSZ;
#endif
#endif
    }

  if (!hasrelocs)
    internal_f.f_flags |= F_RELFLG;
  if (!haslinno)
    internal_f.f_flags |= F_LNNO;
  if (abfd->flags & EXEC_P)
    internal_f.f_flags |= F_EXEC;
#ifdef COFF_IMAGE_WITH_PE
  if (! hasdebug)
    internal_f.f_flags |= IMAGE_FILE_DEBUG_STRIPPED;
  if (pe_data (abfd)->real_flags & IMAGE_FILE_LARGE_ADDRESS_AWARE)
    internal_f.f_flags |= IMAGE_FILE_LARGE_ADDRESS_AWARE;
#endif

#if !defined(COFF_WITH_pex64) && !defined(COFF_WITH_peAArch64) && !defined(COFF_WITH_peLoongArch64)
#ifdef COFF_WITH_PE
  internal_f.f_flags |= IMAGE_FILE_32BIT_MACHINE;
#else
  if (bfd_little_endian (abfd))
    internal_f.f_flags |= F_AR32WR;
  else
    internal_f.f_flags |= F_AR32W;
#endif
#endif

#ifdef TI_TARGET_ID
  /* Target id is used in TI COFF v1 and later; COFF0 won't use this field,
     but it doesn't hurt to set it internally.  */
  internal_f.f_target_id = TI_TARGET_ID;
#endif

  /* FIXME, should do something about the other byte orders and
     architectures.  */

#ifdef RS6000COFF_C
  if ((abfd->flags & DYNAMIC) != 0)
    internal_f.f_flags |= F_SHROBJ;
  if (bfd_get_section_by_name (abfd, _LOADER) != NULL)
    internal_f.f_flags |= F_DYNLOAD;
#endif

  memset (&internal_a, 0, sizeof internal_a);

  /* Set up architecture-dependent stuff.  */
  {
    unsigned int magic = 0;
    unsigned short flags = 0;

    coff_set_flags (abfd, &magic, &flags);
    internal_f.f_magic = magic;
    internal_f.f_flags |= flags;
    /* ...and the "opt"hdr...  */

#ifdef TICOFF_AOUT_MAGIC
    internal_a.magic = TICOFF_AOUT_MAGIC;
#define __A_MAGIC_SET__
#endif

#if defined(ARM)
#define __A_MAGIC_SET__
    internal_a.magic = ZMAGIC;
#endif

#if defined(AARCH64)
#define __A_MAGIC_SET__
    internal_a.magic = ZMAGIC;
#endif

#if defined(LOONGARCH64)
#define __A_MAGIC_SET__
    internal_a.magic = ZMAGIC;
#endif

#if defined MCORE_PE
#define __A_MAGIC_SET__
    internal_a.magic = IMAGE_NT_OPTIONAL_HDR_MAGIC;
#endif

#if defined(I386)
#define __A_MAGIC_SET__
#if defined LYNXOS
    internal_a.magic = LYNXCOFFMAGIC;
#elif defined AMD64
    internal_a.magic = IMAGE_NT_OPTIONAL_HDR64_MAGIC;
#else
    internal_a.magic = ZMAGIC;
#endif
#endif /* I386 */

#if defined(IA64)
#define __A_MAGIC_SET__
    internal_a.magic = PE32PMAGIC;
#endif /* IA64 */

#if defined(SPARC)
#define __A_MAGIC_SET__
#if defined(LYNXOS)
    internal_a.magic = LYNXCOFFMAGIC;
#endif /* LYNXOS */
#endif /* SPARC */

#ifdef RS6000COFF_C
#define __A_MAGIC_SET__
    internal_a.magic = (abfd->flags & D_PAGED) ? RS6K_AOUTHDR_ZMAGIC :
    (abfd->flags & WP_TEXT) ? RS6K_AOUTHDR_NMAGIC :
    RS6K_AOUTHDR_OMAGIC;
#endif

#if defined(SH) && defined(COFF_WITH_PE)
#define __A_MAGIC_SET__
    internal_a.magic = SH_PE_MAGIC;
#endif

#if defined(MIPS) && defined(COFF_WITH_PE)
#define __A_MAGIC_SET__
    internal_a.magic = MIPS_PE_MAGIC;
#endif

#ifndef __A_MAGIC_SET__
#include "Your aouthdr magic number is not being set!"
#else
#undef __A_MAGIC_SET__
#endif
  }

#ifdef RS6000COFF_C
  /* XCOFF 32bit needs this to have new behaviour for n_type field.  */
  internal_a.vstamp = 2;
#else
  /* FIXME: Does anybody ever set this to another value?  */
  internal_a.vstamp = 0;
#endif

  /* Now should write relocs, strings, syms.  */
  obj_sym_filepos (abfd) = sym_base;

  if (bfd_get_symcount (abfd) != 0)
    {
      int firstundef;

      if (!coff_renumber_symbols (abfd, &firstundef))
	return false;
      coff_mangle_symbols (abfd);
      if (! coff_write_symbols (abfd))
	return false;
      if (! coff_write_linenumbers (abfd))
	return false;
      if (! coff_write_relocs (abfd, firstundef))
	return false;
    }
#ifdef COFF_LONG_SECTION_NAMES
  else if (long_section_names && ! obj_coff_strings_written (abfd))
    {
      /* If we have long section names we have to write out the string
	 table even if there are no symbols.  */
      if (! coff_write_symbols (abfd))
	return false;
    }
#endif
  /* If bfd_get_symcount (abfd) != 0, then we are not using the COFF
     backend linker, and obj_raw_syment_count is not valid until after
     coff_write_symbols is called.  */
  if (obj_raw_syment_count (abfd) != 0)
    {
      internal_f.f_symptr = sym_base;
#ifdef RS6000COFF_C
      /* AIX appears to require that F_RELFLG not be set if there are
	 local symbols but no relocations.  */
      internal_f.f_flags &=~ F_RELFLG;
#endif
    }
  else
    {
      if (long_section_names)
	internal_f.f_symptr = sym_base;
      else
	internal_f.f_symptr = 0;
      internal_f.f_flags |= F_LSYMS;
    }

  if (text_sec)
    {
      internal_a.tsize = text_sec->size;
      internal_a.text_start = internal_a.tsize ? text_sec->vma : 0;
    }
  if (data_sec)
    {
      internal_a.dsize = data_sec->size;
      internal_a.data_start = internal_a.dsize ? data_sec->vma : 0;
    }
  if (bss_sec)
    {
      internal_a.bsize = bss_sec->size;
      if (internal_a.bsize && bss_sec->vma < internal_a.data_start)
	internal_a.data_start = bss_sec->vma;
    }

  internal_a.entry = bfd_get_start_address (abfd);
  internal_f.f_nsyms = obj_raw_syment_count (abfd);

#ifdef RS6000COFF_C
  if (xcoff_data (abfd)->full_aouthdr)
    {
      bfd_vma toc;
      asection *loader_sec;

      internal_a.vstamp = 2;

      internal_a.o_snentry = xcoff_data (abfd)->snentry;
      if (internal_a.o_snentry == 0)
	internal_a.entry = (bfd_vma) -1;

      if (text_sec != NULL)
	{
	  internal_a.o_sntext = text_sec->target_index;
	  internal_a.o_algntext = bfd_section_alignment (text_sec);
	}
      else
	{
	  internal_a.o_sntext = 0;
	  internal_a.o_algntext = 0;
	}
      if (data_sec != NULL)
	{
	  internal_a.o_sndata = data_sec->target_index;
	  internal_a.o_algndata = bfd_section_alignment (data_sec);
	}
      else
	{
	  internal_a.o_sndata = 0;
	  internal_a.o_algndata = 0;
	}
      loader_sec = bfd_get_section_by_name (abfd, ".loader");
      if (loader_sec != NULL)
	internal_a.o_snloader = loader_sec->target_index;
      else
	internal_a.o_snloader = 0;
      if (bss_sec != NULL)
	internal_a.o_snbss = bss_sec->target_index;
      else
	internal_a.o_snbss = 0;

      if (tdata_sec != NULL)
	{
	  internal_a.o_sntdata = tdata_sec->target_index;
	  /* TODO: o_flags should be set to RS6K_AOUTHDR_TLS_LE
	     if there is at least one R_TLS_LE relocations.  */
	  internal_a.o_flags = 0;
#ifdef XCOFF64
	  internal_a.o_x64flags = 0;
#endif
	}
      else
	{
	  internal_a.o_sntdata = 0;
	  internal_a.o_flags = 0;
#ifdef XCOFF64
	  internal_a.o_x64flags = 0;
#endif
	}
      if (tbss_sec != NULL)
	  internal_a.o_sntbss = tbss_sec->target_index;
      else
	  internal_a.o_sntbss = 0;

      toc = xcoff_data (abfd)->toc;
      internal_a.o_toc = toc;
      internal_a.o_sntoc = xcoff_data (abfd)->sntoc;

      internal_a.o_modtype = xcoff_data (abfd)->modtype;
      if (xcoff_data (abfd)->cputype != -1)
	internal_a.o_cputype = xcoff_data (abfd)->cputype;
      else
	{
	  switch (bfd_get_arch (abfd))
	    {
	    case bfd_arch_rs6000:
	      internal_a.o_cputype = 4;
	      break;
	    case bfd_arch_powerpc:
	      if (bfd_get_mach (abfd) == bfd_mach_ppc)
		internal_a.o_cputype = 3;
	      else if (bfd_get_mach (abfd) == bfd_mach_ppc_620)
		internal_a.o_cputype = 2;
	      else
		internal_a.o_cputype = 1;
	      break;
	    default:
	      abort ();
	    }
	}
      internal_a.o_maxstack = xcoff_data (abfd)->maxstack;
      internal_a.o_maxdata = xcoff_data (abfd)->maxdata;
    }
#endif

#ifdef COFF_WITH_PE
  {
    /* After object contents are finalized so we can compute a reasonable hash,
       but before header is written so we can update it to point to debug directory.  */
    struct pe_tdata *pe = pe_data (abfd);

    if (pe->build_id.after_write_object_contents != NULL)
      (*pe->build_id.after_write_object_contents) (abfd);
  }
#endif

  /* Now write header.  */
  if (bfd_seek (abfd, (file_ptr) 0, SEEK_SET) != 0)
    return false;

  {
    char * buff;
    bfd_size_type amount = bfd_coff_filhsz (abfd);

    buff = (char *) bfd_malloc (amount);
    if (buff == NULL)
      return false;

    bfd_coff_swap_filehdr_out (abfd, & internal_f, buff);
    amount = bfd_bwrite (buff, amount, abfd);

    free (buff);

    if (amount != bfd_coff_filhsz (abfd))
      return false;
  }

  if (abfd->flags & EXEC_P)
    {
      /* Note that peicode.h fills in a PEAOUTHDR, not an AOUTHDR.
	 include/coff/pe.h sets AOUTSZ == sizeof (PEAOUTHDR)).  */
      char * buff;
      bfd_size_type amount = bfd_coff_aoutsz (abfd);

      buff = (char *) bfd_malloc (amount);
      if (buff == NULL)
	return false;

      coff_swap_aouthdr_out (abfd, & internal_a, buff);
      amount = bfd_bwrite (buff, amount, abfd);

      free (buff);

      if (amount != bfd_coff_aoutsz (abfd))
	return false;

#ifdef COFF_IMAGE_WITH_PE
      if (! coff_apply_checksum (abfd))
	return false;
#endif
    }
#ifdef RS6000COFF_C
#ifndef XCOFF64
  else
    {
      AOUTHDR buff;
      size_t size;

      /* XCOFF32 seems to always write at least a small a.out header.  */
      coff_swap_aouthdr_out (abfd, & internal_a, & buff);
      if (xcoff_data (abfd)->full_aouthdr)
	size = bfd_coff_aoutsz (abfd);
      else
	size = SMALL_AOUTSZ;
      if (bfd_bwrite (& buff, (bfd_size_type) size, abfd) != size)
	return false;
    }
#endif
#endif

  return true;
}

static bool
coff_set_section_contents (bfd * abfd,
			   sec_ptr section,
			   const void * location,
			   file_ptr offset,
			   bfd_size_type count)
{
  if (! abfd->output_has_begun)	/* Set by bfd.c handler.  */
    {
      if (! coff_compute_section_file_positions (abfd))
	return false;
    }

#if defined(_LIB) && !defined(TARG_AUX)
   /* The physical address field of a .lib section is used to hold the
      number of shared libraries in the section.  This code counts the
      number of sections being written, and increments the lma field
      with the number.

      I have found no documentation on the contents of this section.
      Experimentation indicates that the section contains zero or more
      records, each of which has the following structure:

      - a (four byte) word holding the length of this record, in words,
      - a word that always seems to be set to "2",
      - the path to a shared library, null-terminated and then padded
	to a whole word boundary.

      bfd_assert calls have been added to alert if an attempt is made
      to write a section which doesn't follow these assumptions.  The
      code has been tested on ISC 4.1 by me, and on SCO by Robert Lipe
      <robertl@arnet.com> (Thanks!).

      Gvran Uddeborg <gvran@uddeborg.pp.se>.  */
    if (strcmp (section->name, _LIB) == 0)
      {
	bfd_byte *rec, *recend;

	rec = (bfd_byte *) location;
	recend = rec + count;
	while (recend - rec >= 4)
	  {
	    size_t len = bfd_get_32 (abfd, rec);
	    if (len == 0 || len > (size_t) (recend - rec) / 4)
	      break;
	    rec += len * 4;
	    ++section->lma;
	  }

	BFD_ASSERT (rec == recend);
      }
#endif

  /* Don't write out bss sections - one way to do this is to
       see if the filepos has not been set.  */
  if (section->filepos == 0)
    return true;

  if (bfd_seek (abfd, section->filepos + offset, SEEK_SET) != 0)
    return false;

  if (count == 0)
    return true;

  return bfd_bwrite (location, count, abfd) == count;
}

static void *
buy_and_read (bfd *abfd, file_ptr where,
	      bfd_size_type nmemb, bfd_size_type size)
{
  size_t amt;

  if (_bfd_mul_overflow (nmemb, size, &amt))
    {
      bfd_set_error (bfd_error_file_too_big);
      return NULL;
    }
  if (bfd_seek (abfd, where, SEEK_SET) != 0)
    return NULL;
  return _bfd_malloc_and_read (abfd, amt, amt);
}

/*
SUBSUBSECTION
	Reading linenumbers

	Creating the linenumber table is done by reading in the entire
	coff linenumber table, and creating another table for internal use.

	A coff linenumber table is structured so that each function
	is marked as having a line number of 0. Each line within the
	function is an offset from the first line in the function. The
	base of the line number information for the table is stored in
	the symbol associated with the function.

	Note: The PE format uses line number 0 for a flag indicating a
	new source file.

	The information is copied from the external to the internal
	table, and each symbol which marks a function is marked by
	pointing its...

	How does this work ?
*/

static int
coff_sort_func_alent (const void * arg1, const void * arg2)
{
  const alent *al1 = *(const alent **) arg1;
  const alent *al2 = *(const alent **) arg2;
  const coff_symbol_type *s1 = (const coff_symbol_type *) (al1->u.sym);
  const coff_symbol_type *s2 = (const coff_symbol_type *) (al2->u.sym);

  if (s1 == NULL || s2 == NULL)
    return 0;
  if (s1->symbol.value < s2->symbol.value)
    return -1;
  else if (s1->symbol.value > s2->symbol.value)
    return 1;

  return 0;
}

static bool
coff_slurp_line_table (bfd *abfd, asection *asect)
{
  LINENO *native_lineno;
  alent *lineno_cache;
  unsigned int counter;
  alent *cache_ptr;
  bfd_vma prev_offset = 0;
  bool ordered = true;
  unsigned int nbr_func;
  LINENO *src;
  bool have_func;
  bool ret = true;
  size_t amt;

  if (asect->lineno_count == 0)
    return true;

  BFD_ASSERT (asect->lineno == NULL);

  native_lineno = (LINENO *) buy_and_read (abfd, asect->line_filepos,
					   asect->lineno_count,
					   bfd_coff_linesz (abfd));
  if (native_lineno == NULL)
    {
      _bfd_error_handler
	(_("%pB: warning: line number table read failed"), abfd);
      return false;
    }

  if (_bfd_mul_overflow (asect->lineno_count + 1, sizeof (alent), &amt))
    {
      bfd_set_error (bfd_error_file_too_big);
      free (native_lineno);
      return false;
    }
  lineno_cache = (alent *) bfd_alloc (abfd, amt);
  if (lineno_cache == NULL)
    {
      free (native_lineno);
      return false;
    }

  cache_ptr = lineno_cache;
  asect->lineno = lineno_cache;
  src = native_lineno;
  nbr_func = 0;
  have_func = false;

  for (counter = 0; counter < asect->lineno_count; counter++, src++)
    {
      struct internal_lineno dst;

      bfd_coff_swap_lineno_in (abfd, src, &dst);
      cache_ptr->line_number = dst.l_lnno;
      /* Appease memory checkers that get all excited about
	 uninitialised memory when copying alents if u.offset is
	 larger than u.sym.  (64-bit BFD on 32-bit host.)  */
      memset (&cache_ptr->u, 0, sizeof (cache_ptr->u));

      if (cache_ptr->line_number == 0)
	{
	  combined_entry_type * ent;
	  unsigned long symndx;
	  coff_symbol_type *sym;

	  have_func = false;
	  symndx = dst.l_addr.l_symndx;
	  if (symndx >= obj_raw_syment_count (abfd))
	    {
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB: warning: illegal symbol index 0x%lx in line number entry %d"),
		 abfd, symndx, counter);
	      cache_ptr->line_number = -1;
	      ret = false;
	      continue;
	    }

	  ent = obj_raw_syments (abfd) + symndx;
	  /* FIXME: We should not be casting between ints and
	     pointers like this.  */
	  if (! ent->is_sym)
	    {
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB: warning: illegal symbol index 0x%lx in line number entry %d"),
		 abfd, symndx, counter);
	      cache_ptr->line_number = -1;
	      ret = false;
	      continue;
	    }
	  sym = (coff_symbol_type *) (ent->u.syment._n._n_n._n_zeroes);

	  /* PR 17512 file: 078-10659-0.004  */
	  if (sym < obj_symbols (abfd)
	      || sym >= obj_symbols (abfd) + bfd_get_symcount (abfd))
	    {
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB: warning: illegal symbol in line number entry %d"),
		 abfd, counter);
	      cache_ptr->line_number = -1;
	      ret = false;
	      continue;
	    }

	  have_func = true;
	  nbr_func++;
	  cache_ptr->u.sym = (asymbol *) sym;
	  if (sym->lineno != NULL)
	    _bfd_error_handler
	      /* xgettext:c-format */
	      (_("%pB: warning: duplicate line number information for `%s'"),
	       abfd, bfd_asymbol_name (&sym->symbol));

	  sym->lineno = cache_ptr;
	  if (sym->symbol.value < prev_offset)
	    ordered = false;
	  prev_offset = sym->symbol.value;
	}
      else if (!have_func)
	/* Drop line information that has no associated function.
	   PR 17521: file: 078-10659-0.004.  */
	continue;
      else
	cache_ptr->u.offset = dst.l_addr.l_paddr - bfd_section_vma (asect);
      cache_ptr++;
    }

  asect->lineno_count = cache_ptr - lineno_cache;
  memset (cache_ptr, 0, sizeof (*cache_ptr));
  free (native_lineno);

  /* On some systems (eg AIX5.3) the lineno table may not be sorted.  */
  if (!ordered)
    {
      /* Sort the table.  */
      alent **func_table;
      alent *n_lineno_cache;

      /* Create a table of functions.  */
      if (_bfd_mul_overflow (nbr_func, sizeof (alent *), &amt))
	{
	  bfd_set_error (bfd_error_file_too_big);
	  ret = false;
	}
      else if ((func_table = (alent **) bfd_alloc (abfd, amt)) != NULL)
	{
	  alent **p = func_table;
	  unsigned int i;

	  for (i = 0; i < asect->lineno_count; i++)
	    if (lineno_cache[i].line_number == 0)
	      *p++ = &lineno_cache[i];

	  BFD_ASSERT ((unsigned int) (p - func_table) == nbr_func);

	  /* Sort by functions.  */
	  qsort (func_table, nbr_func, sizeof (alent *), coff_sort_func_alent);

	  /* Create the new sorted table.  */
	  if (_bfd_mul_overflow (asect->lineno_count, sizeof (alent), &amt))
	    {
	      bfd_set_error (bfd_error_file_too_big);
	      ret = false;
	    }
	  else if ((n_lineno_cache = (alent *) bfd_alloc (abfd, amt)) != NULL)
	    {
	      alent *n_cache_ptr = n_lineno_cache;

	      for (i = 0; i < nbr_func; i++)
		{
		  coff_symbol_type *sym;
		  alent *old_ptr = func_table[i];

		  /* Update the function entry.  */
		  sym = (coff_symbol_type *) old_ptr->u.sym;
		  /* PR binutils/17512: Point the lineno to where
		     this entry will be after the memcpy below.  */
		  sym->lineno = lineno_cache + (n_cache_ptr - n_lineno_cache);
		  /* Copy the function and line number entries.  */
		  do
		    *n_cache_ptr++ = *old_ptr++;
		  while (old_ptr->line_number != 0);
		}

	      memcpy (lineno_cache, n_lineno_cache,
		      asect->lineno_count * sizeof (alent));
	    }
	  else
	    ret = false;
	  bfd_release (abfd, func_table);
	}
      else
	ret = false;
    }

  return ret;
}

/* Slurp in the symbol table, converting it to generic form.  Note
   that if coff_relocate_section is defined, the linker will read
   symbols via coff_link_add_symbols, rather than via this routine.  */

static bool
coff_slurp_symbol_table (bfd * abfd)
{
  combined_entry_type *native_symbols;
  coff_symbol_type *cached_area;
  unsigned int *table_ptr;
  unsigned int number_of_symbols = 0;
  bool ret = true;
  size_t amt;

  if (obj_symbols (abfd))
    return true;

  /* Read in the symbol table.  */
  if ((native_symbols = coff_get_normalized_symtab (abfd)) == NULL)
    return false;

  /* Allocate enough room for all the symbols in cached form.  */
  if (_bfd_mul_overflow (obj_raw_syment_count (abfd),
			 sizeof (*cached_area), &amt))
    {
      bfd_set_error (bfd_error_file_too_big);
      return false;
    }
  cached_area = (coff_symbol_type *) bfd_alloc (abfd, amt);
  if (cached_area == NULL)
    return false;

  if (_bfd_mul_overflow (obj_raw_syment_count (abfd),
			 sizeof (*table_ptr), &amt))
    {
      bfd_set_error (bfd_error_file_too_big);
      return false;
    }
  table_ptr = (unsigned int *) bfd_zalloc (abfd, amt);
  if (table_ptr == NULL)
    return false;
  else
    {
      coff_symbol_type *dst = cached_area;
      unsigned int last_native_index = obj_raw_syment_count (abfd);
      unsigned int this_index = 0;

      while (this_index < last_native_index)
	{
	  combined_entry_type *src = native_symbols + this_index;
	  table_ptr[this_index] = number_of_symbols;

	  dst->symbol.the_bfd = abfd;
	  BFD_ASSERT (src->is_sym);
	  dst->symbol.name = (char *) (src->u.syment._n._n_n._n_offset);
	  /* We use the native name field to point to the cached field.  */
	  src->u.syment._n._n_n._n_zeroes = (uintptr_t) dst;
	  dst->symbol.section = coff_section_from_bfd_index (abfd,
						     src->u.syment.n_scnum);
	  dst->symbol.flags = 0;
	  /* PR 17512: file: 079-7098-0.001:0.1.  */
	  dst->symbol.value = 0;
	  dst->done_lineno = false;

	  switch (src->u.syment.n_sclass)
	    {
	    case C_EXT:
	    case C_WEAKEXT:
#if defined ARM
	    case C_THUMBEXT:
	    case C_THUMBEXTFUNC:
#endif
#ifdef RS6000COFF_C
	    case C_HIDEXT:
#if ! defined _AIX52 && ! defined AIX_WEAK_SUPPORT
	    case C_AIX_WEAKEXT:
#endif
#endif
#ifdef C_SYSTEM
	    case C_SYSTEM:	/* System Wide variable.  */
#endif
#ifdef COFF_WITH_PE
	    /* In PE, 0x68 (104) denotes a section symbol.  */
	    case C_SECTION:
	    /* In PE, 0x69 (105) denotes a weak external symbol.  */
	    case C_NT_WEAK:
#endif
	      switch (coff_classify_symbol (abfd, &src->u.syment))
		{
		case COFF_SYMBOL_GLOBAL:
		  dst->symbol.flags = BSF_EXPORT | BSF_GLOBAL;
#if defined COFF_WITH_PE
		  /* PE sets the symbol to a value relative to the
		     start of the section.  */
		  dst->symbol.value = src->u.syment.n_value;
#else
		  dst->symbol.value = (src->u.syment.n_value
				       - dst->symbol.section->vma);
#endif
		  if (ISFCN ((src->u.syment.n_type)))
		    /* A function ext does not go at the end of a
		       file.  */
		    dst->symbol.flags |= BSF_NOT_AT_END | BSF_FUNCTION;
		  break;

		case COFF_SYMBOL_COMMON:
		  dst->symbol.section = bfd_com_section_ptr;
		  dst->symbol.value = src->u.syment.n_value;
		  break;

		case COFF_SYMBOL_UNDEFINED:
		  dst->symbol.section = bfd_und_section_ptr;
		  dst->symbol.value = 0;
		  break;

		case COFF_SYMBOL_PE_SECTION:
		  dst->symbol.flags |= BSF_EXPORT | BSF_SECTION_SYM;
		  dst->symbol.value = 0;
		  break;

		case COFF_SYMBOL_LOCAL:
		  dst->symbol.flags = BSF_LOCAL;
#if defined COFF_WITH_PE
		  /* PE sets the symbol to a value relative to the
		     start of the section.  */
		  dst->symbol.value = src->u.syment.n_value;
#else
		  dst->symbol.value = (src->u.syment.n_value
				       - dst->symbol.section->vma);
#endif
		  if (ISFCN ((src->u.syment.n_type)))
		    dst->symbol.flags |= BSF_NOT_AT_END | BSF_FUNCTION;
		  break;
		}

#ifdef RS6000COFF_C
	      /* A symbol with a csect entry should not go at the end.  */
	      if (src->u.syment.n_numaux > 0)
		dst->symbol.flags |= BSF_NOT_AT_END;
#endif

#ifdef COFF_WITH_PE
	      if (src->u.syment.n_sclass == C_NT_WEAK)
		dst->symbol.flags |= BSF_WEAK;

	      if (src->u.syment.n_sclass == C_SECTION
		  && src->u.syment.n_scnum > 0)
		dst->symbol.flags = BSF_LOCAL;
#endif
	      if (src->u.syment.n_sclass == C_WEAKEXT
#ifdef RS6000COFF_C
		  || src->u.syment.n_sclass == C_AIX_WEAKEXT
#endif
		  )
		dst->symbol.flags |= BSF_WEAK;

	      break;

	    case C_STAT:	 /* Static.  */
#if defined ARM
	    case C_THUMBSTAT:    /* Thumb static.  */
	    case C_THUMBLABEL:   /* Thumb label.  */
	    case C_THUMBSTATFUNC:/* Thumb static function.  */
#endif
#ifdef RS6000COFF_C
	    case C_DWARF:	 /* A label in a dwarf section.  */
	    case C_INFO:	 /* A label in a comment section.  */
#endif
	    case C_LABEL:	 /* Label.  */
	      if (src->u.syment.n_scnum == N_DEBUG)
		dst->symbol.flags = BSF_DEBUGGING;
	      else
		dst->symbol.flags = BSF_LOCAL;

	      /* Base the value as an index from the base of the
		 section, if there is one.  */
	      if (dst->symbol.section)
		{
#if defined COFF_WITH_PE
		  /* PE sets the symbol to a value relative to the
		     start of the section.  */
		  dst->symbol.value = src->u.syment.n_value;
#else
		  dst->symbol.value = (src->u.syment.n_value
				       - dst->symbol.section->vma);
#endif
		}
	      else
		dst->symbol.value = src->u.syment.n_value;
	      break;

	    case C_FILE:	/* File name.  */
	      dst->symbol.flags = BSF_FILE;
	      /* Fall through.  */
	    case C_MOS:		/* Member of structure.  */
	    case C_EOS:		/* End of structure.  */
	    case C_REGPARM:	/* Register parameter.  */
	    case C_REG:		/* register variable.  */
	      /* C_AUTOARG conflicts with TI COFF C_UEXT.  */
	    case C_TPDEF:	/* Type definition.  */
	    case C_ARG:
	    case C_AUTO:	/* Automatic variable.  */
	    case C_FIELD:	/* Bit field.  */
	    case C_ENTAG:	/* Enumeration tag.  */
	    case C_MOE:		/* Member of enumeration.  */
	    case C_MOU:		/* Member of union.  */
	    case C_UNTAG:	/* Union tag.  */
	    case C_STRTAG:	/* Structure tag.  */
#ifdef RS6000COFF_C
	    case C_GSYM:
	    case C_LSYM:
	    case C_PSYM:
	    case C_RSYM:
	    case C_RPSYM:
	    case C_STSYM:
	    case C_TCSYM:
	    case C_BCOMM:
	    case C_ECOML:
	    case C_ECOMM:
	    case C_DECL:
	    case C_ENTRY:
	    case C_FUN:
	    case C_ESTAT:
#endif
	      dst->symbol.flags |= BSF_DEBUGGING;
	      dst->symbol.value = (src->u.syment.n_value);
	      break;

#ifdef RS6000COFF_C
	    case C_BINCL:	/* Beginning of include file.  */
	    case C_EINCL:	/* Ending of include file.  */
	      /* The value is actually a pointer into the line numbers
		 of the file.  We locate the line number entry, and
		 set the section to the section which contains it, and
		 the value to the index in that section.  */
	      {
		asection *sec;

		dst->symbol.flags = BSF_DEBUGGING;
		for (sec = abfd->sections; sec != NULL; sec = sec->next)
		  if (sec->line_filepos <= (file_ptr) src->u.syment.n_value
		      && ((file_ptr) (sec->line_filepos
				      + sec->lineno_count * bfd_coff_linesz (abfd))
			  > (file_ptr) src->u.syment.n_value))
		    break;
		if (sec == NULL)
		  dst->symbol.value = 0;
		else
		  {
		    dst->symbol.section = sec;
		    dst->symbol.value = ((src->u.syment.n_value
					  - sec->line_filepos)
					 / bfd_coff_linesz (abfd));
		    src->fix_line = 1;
		  }
	      }
	      break;

	    case C_BSTAT:
	      dst->symbol.flags = BSF_DEBUGGING;

	      if (src->u.syment.n_value >= obj_raw_syment_count (abfd))
		dst->symbol.value = 0;
	      else
		{
		  /* The value is actually a symbol index.  Save a pointer
		     to the symbol instead of the index.  FIXME: This
		     should use a union.  */
		  src->u.syment.n_value
		    = (uintptr_t) (native_symbols + src->u.syment.n_value);
		  dst->symbol.value = src->u.syment.n_value;
		  src->fix_value = 1;
		}
	      break;
#endif

	    case C_BLOCK:	/* ".bb" or ".eb".  */
	    case C_FCN:		/* ".bf" or ".ef" (or PE ".lf").  */
	    case C_EFCN:	/* Physical end of function.  */
#if defined COFF_WITH_PE
	      /* PE sets the symbol to a value relative to the start
		 of the section.  */
	      dst->symbol.value = src->u.syment.n_value;
	      if (strcmp (dst->symbol.name, ".bf") != 0)
		{
		  /* PE uses funny values for .ef and .lf; don't
		     relocate them.  */
		  dst->symbol.flags = BSF_DEBUGGING;
		}
	      else
		dst->symbol.flags = BSF_DEBUGGING | BSF_DEBUGGING_RELOC;
#else
	      /* Base the value as an index from the base of the
		 section.  */
	      dst->symbol.flags = BSF_LOCAL;
	      dst->symbol.value = (src->u.syment.n_value
				   - dst->symbol.section->vma);
#endif
	      break;

	    case C_STATLAB:	/* Static load time label.  */
	      dst->symbol.value = src->u.syment.n_value;
	      dst->symbol.flags = BSF_GLOBAL;
	      break;

	    case C_NULL:
	      /* PE DLLs sometimes have zeroed out symbols for some
		 reason.  Just ignore them without a warning.  */
	      if (src->u.syment.n_type == 0
		  && src->u.syment.n_value == 0
		  && src->u.syment.n_scnum == 0)
		break;
#ifdef RS6000COFF_C
	      /* XCOFF specific: deleted entry.  */
	      if (src->u.syment.n_value == C_NULL_VALUE)
		break;
#endif
	      /* Fall through.  */
	    case C_EXTDEF:	/* External definition.  */
	    case C_ULABEL:	/* Undefined label.  */
	    case C_USTATIC:	/* Undefined static.  */
#ifndef COFF_WITH_PE
	    /* C_LINE in regular coff is 0x68.  NT has taken over this storage
	       class to represent a section symbol.  */
	    case C_LINE:	/* line # reformatted as symbol table entry.  */
	      /* NT uses 0x67 for a weak symbol, not C_ALIAS.  */
	    case C_ALIAS:	/* Duplicate tag.  */
#endif
	      /* New storage classes for TI COFF.  */
#ifdef TICOFF
	    case C_UEXT:	/* Tentative external definition.  */
#endif
	    case C_EXTLAB:	/* External load time label.  */
	    default:
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB: unrecognized storage class %d for %s symbol `%s'"),
		 abfd, src->u.syment.n_sclass,
		 dst->symbol.section->name, dst->symbol.name);
	      ret = false;
	      /* Fall through.  */
	    case C_HIDDEN:	/* Ext symbol in dmert public lib.  */
	      /* PR 20722: These symbols can also be generated by
		 building DLLs with --gc-sections enabled.  */
	      dst->symbol.flags = BSF_DEBUGGING;
	      dst->symbol.value = (src->u.syment.n_value);
	      break;
	    }

	  dst->native = src;
	  dst->symbol.udata.i = 0;
	  dst->lineno = NULL;

	  this_index += (src->u.syment.n_numaux) + 1;
	  dst++;
	  number_of_symbols++;
	}
    }

  obj_symbols (abfd) = cached_area;
  obj_raw_syments (abfd) = native_symbols;

  abfd->symcount = number_of_symbols;
  obj_convert (abfd) = table_ptr;
  /* Slurp the line tables for each section too.  */
  {
    asection *p;

    p = abfd->sections;
    while (p)
      {
	if (! coff_slurp_line_table (abfd, p))
	  return false;
	p = p->next;
      }
  }

  return ret;
}

/* Classify a COFF symbol.  A couple of targets have globally visible
   symbols which are not class C_EXT, and this handles those.  It also
   recognizes some special PE cases.  */

static enum coff_symbol_classification
coff_classify_symbol (bfd *abfd,
		      struct internal_syment *syment)
{
  /* FIXME: This partially duplicates the switch in
     coff_slurp_symbol_table.  */
  switch (syment->n_sclass)
    {
    case C_EXT:
    case C_WEAKEXT:
#ifdef ARM
    case C_THUMBEXT:
    case C_THUMBEXTFUNC:
#endif
#ifdef RS6000COFF_C
    case C_HIDEXT:
#if ! defined _AIX52 && ! defined AIX_WEAK_SUPPORT
    case C_AIX_WEAKEXT:
#endif
#endif
#ifdef C_SYSTEM
    case C_SYSTEM:
#endif
#ifdef COFF_WITH_PE
    case C_NT_WEAK:
#endif
      if (syment->n_scnum == 0)
	{
	  if (syment->n_value == 0)
	    return COFF_SYMBOL_UNDEFINED;
	  else
	    return COFF_SYMBOL_COMMON;
	}
#ifdef RS6000COFF_C
      if (syment->n_sclass == C_HIDEXT)
	return COFF_SYMBOL_LOCAL;
#endif
      return COFF_SYMBOL_GLOBAL;

    default:
      break;
    }

#ifdef COFF_WITH_PE
  if (syment->n_sclass == C_STAT)
    {
      if (syment->n_scnum == 0)
	/* The Microsoft compiler sometimes generates these if a
	   small static function is inlined every time it is used.
	   The function is discarded, but the symbol table entry
	   remains.  */
	return COFF_SYMBOL_LOCAL;

#ifdef STRICT_PE_FORMAT
      /* This is correct for Microsoft generated objects, but it
	 breaks gas generated objects.  */
      if (syment->n_value == 0)
	{
	  const asection *sec;
	  const char *name;
	  char buf[SYMNMLEN + 1];

	  name = _bfd_coff_internal_syment_name (abfd, syment, buf);
	  sec = coff_section_from_bfd_index (abfd, syment->n_scnum);
	  if (sec != NULL && name != NULL
	      && (strcmp (bfd_section_name (sec), name) == 0))
	    return COFF_SYMBOL_PE_SECTION;
	}
#endif

      return COFF_SYMBOL_LOCAL;
    }

  if (syment->n_sclass == C_SECTION)
    {
      /* In some cases in a DLL generated by the Microsoft linker, the
	 n_value field will contain garbage.  FIXME: This should
	 probably be handled by the swapping function instead.  */
      syment->n_value = 0;
      if (syment->n_scnum == 0)
	return COFF_SYMBOL_UNDEFINED;
      return COFF_SYMBOL_PE_SECTION;
    }
#endif /* COFF_WITH_PE */

  /* If it is not a global symbol, we presume it is a local symbol.  */
  if (syment->n_scnum == 0)
    {
      char buf[SYMNMLEN + 1];

      _bfd_error_handler
	/* xgettext:c-format */
	(_("warning: %pB: local symbol `%s' has no section"),
	 abfd, _bfd_coff_internal_syment_name (abfd, syment, buf));
    }

  return COFF_SYMBOL_LOCAL;
}

/*
SUBSUBSECTION
	Reading relocations

	Coff relocations are easily transformed into the internal BFD form
	(@code{arelent}).

	Reading a coff relocation table is done in the following stages:

	o Read the entire coff relocation table into memory.

	o Process each relocation in turn; first swap it from the
	external to the internal form.

	o Turn the symbol referenced in the relocation's symbol index
	into a pointer into the canonical symbol table.
	This table is the same as the one returned by a call to
	@code{bfd_canonicalize_symtab}. The back end will call that
	routine and save the result if a canonicalization hasn't been done.

	o The reloc index is turned into a pointer to a howto
	structure, in a back end specific way. For instance, the 386
	uses the @code{r_type} to directly produce an index
	into a howto table vector.

	o Note that @code{arelent.addend} for COFF is often not what
	most people understand as a relocation addend, but rather an
	adjustment to the relocation addend stored in section contents
	of relocatable object files.  The value found in section
	contents may also be confusing, depending on both symbol value
	and addend somewhat similar to the field value for a
	final-linked object.  See @code{CALC_ADDEND}.
*/

#ifndef CALC_ADDEND
#define CALC_ADDEND(abfd, ptr, reloc, cache_ptr)		\
  {								\
    coff_symbol_type *coffsym = NULL;				\
								\
    if (ptr && bfd_asymbol_bfd (ptr) != abfd)			\
      coffsym = (obj_symbols (abfd)				\
		 + (cache_ptr->sym_ptr_ptr - symbols));		\
    else if (ptr)						\
      coffsym = coff_symbol_from (ptr);				\
    if (coffsym != NULL						\
	&& coffsym->native->is_sym				\
	&& coffsym->native->u.syment.n_scnum == 0)		\
      cache_ptr->addend = 0;					\
    else if (ptr && bfd_asymbol_bfd (ptr) == abfd		\
	     && ptr->section != NULL)				\
      cache_ptr->addend = - (ptr->section->vma + ptr->value);	\
    else							\
      cache_ptr->addend = 0;					\
  }
#endif

static bool
coff_slurp_reloc_table (bfd * abfd, sec_ptr asect, asymbol ** symbols)
{
  bfd_byte *native_relocs;
  arelent *reloc_cache;
  arelent *cache_ptr;
  unsigned int idx;
  size_t amt;

  if (asect->relocation)
    return true;
  if (asect->reloc_count == 0)
    return true;
  if (asect->flags & SEC_CONSTRUCTOR)
    return true;
  if (!coff_slurp_symbol_table (abfd))
    return false;

  native_relocs = (bfd_byte *) buy_and_read (abfd, asect->rel_filepos,
					     asect->reloc_count,
					     bfd_coff_relsz (abfd));
  if (native_relocs == NULL)
    return false;

  if (_bfd_mul_overflow (asect->reloc_count, sizeof (arelent), &amt))
    {
      bfd_set_error (bfd_error_file_too_big);
      return false;
    }
  reloc_cache = (arelent *) bfd_alloc (abfd, amt);
  if (reloc_cache == NULL)
    {
      free (native_relocs);
      return false;
    }

  for (idx = 0; idx < asect->reloc_count; idx++)
    {
      struct internal_reloc dst;
      void *src;
#ifndef RELOC_PROCESSING
      asymbol *ptr;
#endif

      cache_ptr = reloc_cache + idx;
      src = native_relocs + idx * (size_t) bfd_coff_relsz (abfd);

      dst.r_offset = 0;
      bfd_coff_swap_reloc_in (abfd, src, &dst);

#ifdef RELOC_PROCESSING
      RELOC_PROCESSING (cache_ptr, &dst, symbols, abfd, asect);
#else
      cache_ptr->address = dst.r_vaddr;

      if (dst.r_symndx != -1 && symbols != NULL)
	{
	  if (dst.r_symndx < 0 || dst.r_symndx >= obj_conv_table_size (abfd))
	    {
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB: warning: illegal symbol index %ld in relocs"),
		 abfd, dst.r_symndx);
	      cache_ptr->sym_ptr_ptr = bfd_abs_section_ptr->symbol_ptr_ptr;
	      ptr = NULL;
	    }
	  else
	    {
	      cache_ptr->sym_ptr_ptr = (symbols
					+ obj_convert (abfd)[dst.r_symndx]);
	      ptr = *(cache_ptr->sym_ptr_ptr);
	    }
	}
      else
	{
	  cache_ptr->sym_ptr_ptr = bfd_abs_section_ptr->symbol_ptr_ptr;
	  ptr = NULL;
	}

      /* The symbols definitions that we have read in have been
	 relocated as if their sections started at 0. But the offsets
	 refering to the symbols in the raw data have not been
	 modified, so we have to have a negative addend to compensate.

	 Note that symbols which used to be common must be left alone.  */

      /* Calculate any reloc addend by looking at the symbol.  */
      CALC_ADDEND (abfd, ptr, dst, cache_ptr);
      (void) ptr;

      cache_ptr->address -= asect->vma;
      /* !! cache_ptr->section = NULL;*/

      /* Fill in the cache_ptr->howto field from dst.r_type.  */
      RTYPE2HOWTO (cache_ptr, &dst);
#endif	/* RELOC_PROCESSING */

      if (cache_ptr->howto == NULL)
	{
	  _bfd_error_handler
	    /* xgettext:c-format */
	    (_("%pB: illegal relocation type %d at address %#" PRIx64),
	     abfd, dst.r_type, (uint64_t) dst.r_vaddr);
	  bfd_set_error (bfd_error_bad_value);
	  free (native_relocs);
	  return false;
	}
    }

  free (native_relocs);
  asect->relocation = reloc_cache;
  return true;
}

#ifndef coff_rtype_to_howto
#ifdef RTYPE2HOWTO

/* Get the howto structure for a reloc.  This is only used if the file
   including this one defines coff_relocate_section to be
   _bfd_coff_generic_relocate_section, so it is OK if it does not
   always work.  It is the responsibility of the including file to
   make sure it is reasonable if it is needed.  */

static reloc_howto_type *
coff_rtype_to_howto (bfd *abfd ATTRIBUTE_UNUSED,
		     asection *sec ATTRIBUTE_UNUSED,
		     struct internal_reloc *rel ATTRIBUTE_UNUSED,
		     struct coff_link_hash_entry *h ATTRIBUTE_UNUSED,
		     struct internal_syment *sym ATTRIBUTE_UNUSED,
		     bfd_vma *addendp ATTRIBUTE_UNUSED)
{
  arelent genrel;

  genrel.howto = NULL;
  RTYPE2HOWTO (&genrel, rel);
  return genrel.howto;
}

#else /* ! defined (RTYPE2HOWTO) */

#define coff_rtype_to_howto NULL

#endif /* ! defined (RTYPE2HOWTO) */
#endif /* ! defined (coff_rtype_to_howto) */

/* This is stupid.  This function should be a boolean predicate.  */

static long
coff_canonicalize_reloc (bfd * abfd,
			 sec_ptr section,
			 arelent ** relptr,
			 asymbol ** symbols)
{
  arelent *tblptr = section->relocation;
  unsigned int count = 0;

  if (section->flags & SEC_CONSTRUCTOR)
    {
      /* This section has relocs made up by us, they are not in the
	 file, so take them out of their chain and place them into
	 the data area provided.  */
      arelent_chain *chain = section->constructor_chain;

      for (count = 0; count < section->reloc_count; count++)
	{
	  *relptr++ = &chain->relent;
	  chain = chain->next;
	}
    }
  else
    {
      if (! coff_slurp_reloc_table (abfd, section, symbols))
	return -1;

      tblptr = section->relocation;

      for (; count++ < section->reloc_count;)
	*relptr++ = tblptr++;
    }
  *relptr = 0;
  return section->reloc_count;
}

#ifndef coff_set_reloc
#define coff_set_reloc _bfd_generic_set_reloc
#endif

#ifndef coff_reloc16_estimate
#define coff_reloc16_estimate dummy_reloc16_estimate

static int
dummy_reloc16_estimate (bfd *abfd ATTRIBUTE_UNUSED,
			asection *input_section ATTRIBUTE_UNUSED,
			arelent *reloc ATTRIBUTE_UNUSED,
			unsigned int shrink ATTRIBUTE_UNUSED,
			struct bfd_link_info *link_info ATTRIBUTE_UNUSED)
{
  abort ();
  return 0;
}

#endif

#ifndef coff_reloc16_extra_cases

#define coff_reloc16_extra_cases dummy_reloc16_extra_cases

static bool
dummy_reloc16_extra_cases (bfd *abfd ATTRIBUTE_UNUSED,
			   struct bfd_link_info *link_info ATTRIBUTE_UNUSED,
			   struct bfd_link_order *link_order ATTRIBUTE_UNUSED,
			   arelent *reloc ATTRIBUTE_UNUSED,
			   bfd_byte *data ATTRIBUTE_UNUSED,
			   size_t *src_ptr ATTRIBUTE_UNUSED,
			   size_t *dst_ptr ATTRIBUTE_UNUSED)
{
  return false;
}
#endif

/* If coff_relocate_section is defined, we can use the optimized COFF
   backend linker.  Otherwise we must continue to use the old linker.  */

#ifdef coff_relocate_section

#ifndef coff_bfd_link_hash_table_create
#define coff_bfd_link_hash_table_create _bfd_coff_link_hash_table_create
#endif
#ifndef coff_bfd_link_add_symbols
#define coff_bfd_link_add_symbols _bfd_coff_link_add_symbols
#endif
#ifndef coff_bfd_final_link
#define coff_bfd_final_link _bfd_coff_final_link
#endif

#else /* ! defined (coff_relocate_section) */

#define coff_relocate_section NULL
#ifndef coff_bfd_link_hash_table_create
#define coff_bfd_link_hash_table_create _bfd_generic_link_hash_table_create
#endif
#ifndef coff_bfd_link_add_symbols
#define coff_bfd_link_add_symbols _bfd_generic_link_add_symbols
#endif
#define coff_bfd_final_link _bfd_generic_final_link

#endif /* ! defined (coff_relocate_section) */

#define coff_bfd_link_just_syms      _bfd_generic_link_just_syms
#define coff_bfd_copy_link_hash_symbol_type \
  _bfd_generic_copy_link_hash_symbol_type
#define coff_bfd_link_split_section  _bfd_generic_link_split_section

#define coff_bfd_link_check_relocs   _bfd_generic_link_check_relocs

#ifndef coff_start_final_link
#define coff_start_final_link NULL
#endif

#ifndef coff_adjust_symndx
#define coff_adjust_symndx NULL
#endif

#ifndef coff_link_add_one_symbol
#define coff_link_add_one_symbol _bfd_generic_link_add_one_symbol
#endif

#ifndef coff_link_output_has_begun

static bool
coff_link_output_has_begun (bfd * abfd,
			    struct coff_final_link_info * info ATTRIBUTE_UNUSED)
{
  return abfd->output_has_begun;
}
#endif

#ifndef coff_final_link_postscript

static bool
coff_final_link_postscript (bfd * abfd ATTRIBUTE_UNUSED,
			    struct coff_final_link_info * pfinfo ATTRIBUTE_UNUSED)
{
  return true;
}
#endif

#ifndef coff_SWAP_aux_in
#define coff_SWAP_aux_in coff_swap_aux_in
#endif
#ifndef coff_SWAP_sym_in
#define coff_SWAP_sym_in coff_swap_sym_in
#endif
#ifndef coff_SWAP_lineno_in
#define coff_SWAP_lineno_in coff_swap_lineno_in
#endif
#ifndef coff_SWAP_aux_out
#define coff_SWAP_aux_out coff_swap_aux_out
#endif
#ifndef coff_SWAP_sym_out
#define coff_SWAP_sym_out coff_swap_sym_out
#endif
#ifndef coff_SWAP_lineno_out
#define coff_SWAP_lineno_out coff_swap_lineno_out
#endif
#ifndef coff_SWAP_reloc_out
#define coff_SWAP_reloc_out coff_swap_reloc_out
#endif
#ifndef coff_SWAP_filehdr_out
#define coff_SWAP_filehdr_out coff_swap_filehdr_out
#endif
#ifndef coff_SWAP_aouthdr_out
#define coff_SWAP_aouthdr_out coff_swap_aouthdr_out
#endif
#ifndef coff_SWAP_scnhdr_out
#define coff_SWAP_scnhdr_out coff_swap_scnhdr_out
#endif
#ifndef coff_SWAP_reloc_in
#define coff_SWAP_reloc_in coff_swap_reloc_in
#endif
#ifndef coff_SWAP_filehdr_in
#define coff_SWAP_filehdr_in coff_swap_filehdr_in
#endif
#ifndef coff_SWAP_aouthdr_in
#define coff_SWAP_aouthdr_in coff_swap_aouthdr_in
#endif
#ifndef coff_SWAP_scnhdr_in
#define coff_SWAP_scnhdr_in coff_swap_scnhdr_in
#endif

#define COFF_SWAP_TABLE (void *) &bfd_coff_std_swap_table

static const bfd_coff_backend_data bfd_coff_std_swap_table ATTRIBUTE_UNUSED =
{
  coff_SWAP_aux_in, coff_SWAP_sym_in, coff_SWAP_lineno_in,
  coff_SWAP_aux_out, coff_SWAP_sym_out,
  coff_SWAP_lineno_out, coff_SWAP_reloc_out,
  coff_SWAP_filehdr_out, coff_SWAP_aouthdr_out,
  coff_SWAP_scnhdr_out,
  FILHSZ, AOUTSZ, SCNHSZ, SYMESZ, AUXESZ, RELSZ, LINESZ, FILNMLEN,
#ifdef COFF_LONG_FILENAMES
  true,
#else
  false,
#endif
  COFF_DEFAULT_LONG_SECTION_NAMES,
  COFF_DEFAULT_SECTION_ALIGNMENT_POWER,
#ifdef COFF_FORCE_SYMBOLS_IN_STRINGS
  true,
#else
  false,
#endif
#ifdef COFF_DEBUG_STRING_WIDE_PREFIX
  4,
#else
  2,
#endif
  32768,
  coff_SWAP_filehdr_in, coff_SWAP_aouthdr_in, coff_SWAP_scnhdr_in,
  coff_SWAP_reloc_in, coff_bad_format_hook, coff_set_arch_mach_hook,
  coff_mkobject_hook, styp_to_sec_flags, coff_set_alignment_hook,
  coff_slurp_symbol_table, symname_in_debug_hook, coff_pointerize_aux_hook,
  coff_print_aux, coff_reloc16_extra_cases, coff_reloc16_estimate,
  coff_classify_symbol, coff_compute_section_file_positions,
  coff_start_final_link, coff_relocate_section, coff_rtype_to_howto,
  coff_adjust_symndx, coff_link_add_one_symbol,
  coff_link_output_has_begun, coff_final_link_postscript,
  bfd_pe_print_pdata
};

#ifdef TICOFF
/* COFF0 differs in file/section header size and relocation entry size.  */

static const bfd_coff_backend_data ticoff0_swap_table =
{
  coff_SWAP_aux_in, coff_SWAP_sym_in, coff_SWAP_lineno_in,
  coff_SWAP_aux_out, coff_SWAP_sym_out,
  coff_SWAP_lineno_out, coff_swap_reloc_v0_out,
  coff_SWAP_filehdr_out, coff_SWAP_aouthdr_out,
  coff_SWAP_scnhdr_out,
  FILHSZ_V0, AOUTSZ, SCNHSZ_V01, SYMESZ, AUXESZ, RELSZ_V0, LINESZ, FILNMLEN,
#ifdef COFF_LONG_FILENAMES
  true,
#else
  false,
#endif
  COFF_DEFAULT_LONG_SECTION_NAMES,
  COFF_DEFAULT_SECTION_ALIGNMENT_POWER,
#ifdef COFF_FORCE_SYMBOLS_IN_STRINGS
  true,
#else
  false,
#endif
#ifdef COFF_DEBUG_STRING_WIDE_PREFIX
  4,
#else
  2,
#endif
  32768,
  coff_SWAP_filehdr_in, coff_SWAP_aouthdr_in, coff_SWAP_scnhdr_in,
  coff_swap_reloc_v0_in, ticoff0_bad_format_hook, coff_set_arch_mach_hook,
  coff_mkobject_hook, styp_to_sec_flags, coff_set_alignment_hook,
  coff_slurp_symbol_table, symname_in_debug_hook, coff_pointerize_aux_hook,
  coff_print_aux, coff_reloc16_extra_cases, coff_reloc16_estimate,
  coff_classify_symbol, coff_compute_section_file_positions,
  coff_start_final_link, coff_relocate_section, coff_rtype_to_howto,
  coff_adjust_symndx, coff_link_add_one_symbol,
  coff_link_output_has_begun, coff_final_link_postscript,
  bfd_pe_print_pdata
};
#endif

#ifdef TICOFF
/* COFF1 differs in section header size.  */

static const bfd_coff_backend_data ticoff1_swap_table =
{
  coff_SWAP_aux_in, coff_SWAP_sym_in, coff_SWAP_lineno_in,
  coff_SWAP_aux_out, coff_SWAP_sym_out,
  coff_SWAP_lineno_out, coff_SWAP_reloc_out,
  coff_SWAP_filehdr_out, coff_SWAP_aouthdr_out,
  coff_SWAP_scnhdr_out,
  FILHSZ, AOUTSZ, SCNHSZ_V01, SYMESZ, AUXESZ, RELSZ, LINESZ, FILNMLEN,
#ifdef COFF_LONG_FILENAMES
  true,
#else
  false,
#endif
  COFF_DEFAULT_LONG_SECTION_NAMES,
  COFF_DEFAULT_SECTION_ALIGNMENT_POWER,
#ifdef COFF_FORCE_SYMBOLS_IN_STRINGS
  true,
#else
  false,
#endif
#ifdef COFF_DEBUG_STRING_WIDE_PREFIX
  4,
#else
  2,
#endif
  32768,
  coff_SWAP_filehdr_in, coff_SWAP_aouthdr_in, coff_SWAP_scnhdr_in,
  coff_SWAP_reloc_in, ticoff1_bad_format_hook, coff_set_arch_mach_hook,
  coff_mkobject_hook, styp_to_sec_flags, coff_set_alignment_hook,
  coff_slurp_symbol_table, symname_in_debug_hook, coff_pointerize_aux_hook,
  coff_print_aux, coff_reloc16_extra_cases, coff_reloc16_estimate,
  coff_classify_symbol, coff_compute_section_file_positions,
  coff_start_final_link, coff_relocate_section, coff_rtype_to_howto,
  coff_adjust_symndx, coff_link_add_one_symbol,
  coff_link_output_has_begun, coff_final_link_postscript,
  bfd_pe_print_pdata	/* huh */
};
#endif

#ifdef COFF_WITH_PE_BIGOBJ
/* The UID for bigobj files.  */

static const char header_bigobj_classid[16] =
{
  0xC7, 0xA1, 0xBA, 0xD1,
  0xEE, 0xBA,
  0xa9, 0x4b,
  0xAF, 0x20,
  0xFA, 0xF6, 0x6A, 0xA4, 0xDC, 0xB8
};

/* Swap routines.  */

static void
coff_bigobj_swap_filehdr_in (bfd * abfd, void * src, void * dst)
{
  struct external_ANON_OBJECT_HEADER_BIGOBJ *filehdr_src =
    (struct external_ANON_OBJECT_HEADER_BIGOBJ *) src;
  struct internal_filehdr *filehdr_dst = (struct internal_filehdr *) dst;

  filehdr_dst->f_magic  = H_GET_16 (abfd, filehdr_src->Machine);
  filehdr_dst->f_nscns  = H_GET_32 (abfd, filehdr_src->NumberOfSections);
  filehdr_dst->f_timdat = H_GET_32 (abfd, filehdr_src->TimeDateStamp);
  filehdr_dst->f_symptr =
    GET_FILEHDR_SYMPTR (abfd, filehdr_src->PointerToSymbolTable);
  filehdr_dst->f_nsyms  = H_GET_32 (abfd, filehdr_src->NumberOfSymbols);
  filehdr_dst->f_opthdr = 0;
  filehdr_dst->f_flags  = 0;

  /* Check other magic numbers.  */
  if (H_GET_16 (abfd, filehdr_src->Sig1) != IMAGE_FILE_MACHINE_UNKNOWN
      || H_GET_16 (abfd, filehdr_src->Sig2) != 0xffff
      || H_GET_16 (abfd, filehdr_src->Version) != 2
      || memcmp (filehdr_src->ClassID, header_bigobj_classid, 16) != 0)
    filehdr_dst->f_opthdr = 0xffff;

  /* Note that CLR metadata are ignored.  */
}

static unsigned int
coff_bigobj_swap_filehdr_out (bfd *abfd, void * in, void * out)
{
  struct internal_filehdr *filehdr_in = (struct internal_filehdr *) in;
  struct external_ANON_OBJECT_HEADER_BIGOBJ *filehdr_out =
    (struct external_ANON_OBJECT_HEADER_BIGOBJ *) out;

  memset (filehdr_out, 0, sizeof (*filehdr_out));

  H_PUT_16 (abfd, IMAGE_FILE_MACHINE_UNKNOWN, filehdr_out->Sig1);
  H_PUT_16 (abfd, 0xffff, filehdr_out->Sig2);
  H_PUT_16 (abfd, 2, filehdr_out->Version);
  memcpy (filehdr_out->ClassID, header_bigobj_classid, 16);
  H_PUT_16 (abfd, filehdr_in->f_magic, filehdr_out->Machine);
  H_PUT_32 (abfd, filehdr_in->f_nscns, filehdr_out->NumberOfSections);
  H_PUT_32 (abfd, filehdr_in->f_timdat, filehdr_out->TimeDateStamp);
  PUT_FILEHDR_SYMPTR (abfd, filehdr_in->f_symptr,
		      filehdr_out->PointerToSymbolTable);
  H_PUT_32 (abfd, filehdr_in->f_nsyms, filehdr_out->NumberOfSymbols);

  return bfd_coff_filhsz (abfd);
}

static void
coff_bigobj_swap_sym_in (bfd * abfd, void * ext1, void * in1)
{
  SYMENT_BIGOBJ *ext = (SYMENT_BIGOBJ *) ext1;
  struct internal_syment *in = (struct internal_syment *) in1;

  if (ext->e.e_name[0] == 0)
    {
      in->_n._n_n._n_zeroes = 0;
      in->_n._n_n._n_offset = H_GET_32 (abfd, ext->e.e.e_offset);
    }
  else
    {
#if SYMNMLEN != E_SYMNMLEN
#error we need to cope with truncating or extending SYMNMLEN
#else
      memcpy (in->_n._n_name, ext->e.e_name, SYMNMLEN);
#endif
    }

  in->n_value = H_GET_32 (abfd, ext->e_value);
  BFD_ASSERT (sizeof (in->n_scnum) >= 4);
  in->n_scnum = H_GET_32 (abfd, ext->e_scnum);
  in->n_type = H_GET_16 (abfd, ext->e_type);
  in->n_sclass = H_GET_8 (abfd, ext->e_sclass);
  in->n_numaux = H_GET_8 (abfd, ext->e_numaux);
}

static unsigned int
coff_bigobj_swap_sym_out (bfd * abfd, void * inp, void * extp)
{
  struct internal_syment *in = (struct internal_syment *) inp;
  SYMENT_BIGOBJ *ext = (SYMENT_BIGOBJ *) extp;

  if (in->_n._n_name[0] == 0)
    {
      H_PUT_32 (abfd, 0, ext->e.e.e_zeroes);
      H_PUT_32 (abfd, in->_n._n_n._n_offset, ext->e.e.e_offset);
    }
  else
    {
#if SYMNMLEN != E_SYMNMLEN
#error we need to cope with truncating or extending SYMNMLEN
#else
      memcpy (ext->e.e_name, in->_n._n_name, SYMNMLEN);
#endif
    }

  H_PUT_32 (abfd, in->n_value, ext->e_value);
  H_PUT_32 (abfd, in->n_scnum, ext->e_scnum);

  H_PUT_16 (abfd, in->n_type, ext->e_type);
  H_PUT_8 (abfd, in->n_sclass, ext->e_sclass);
  H_PUT_8 (abfd, in->n_numaux, ext->e_numaux);

  return SYMESZ_BIGOBJ;
}

static void
coff_bigobj_swap_aux_in (bfd *abfd,
			 void * ext1,
			 int type,
			 int in_class,
			 int indx,
			 int numaux,
			 void * in1)
{
  AUXENT_BIGOBJ *ext = (AUXENT_BIGOBJ *) ext1;
  union internal_auxent *in = (union internal_auxent *) in1;

  /* Make sure that all fields in the aux structure are
     initialised.  */
  memset (in, 0, sizeof * in);
  switch (in_class)
    {
    case C_FILE:
      if (numaux > 1)
	{
	  if (indx == 0)
	    memcpy (in->x_file.x_n.x_fname, ext->File.Name,
		    numaux * sizeof (AUXENT_BIGOBJ));
	}
      else
	memcpy (in->x_file.x_n.x_fname, ext->File.Name, sizeof (ext->File.Name));
      break;

    case C_STAT:
    case C_LEAFSTAT:
    case C_HIDDEN:
      if (type == T_NULL)
	{
	  in->x_scn.x_scnlen = H_GET_32 (abfd, ext->Section.Length);
	  in->x_scn.x_nreloc =
	    H_GET_16 (abfd, ext->Section.NumberOfRelocations);
	  in->x_scn.x_nlinno =
	    H_GET_16 (abfd, ext->Section.NumberOfLinenumbers);
	  in->x_scn.x_checksum = H_GET_32 (abfd, ext->Section.Checksum);
	  in->x_scn.x_associated = H_GET_16 (abfd, ext->Section.Number)
	    | (H_GET_16 (abfd, ext->Section.HighNumber) << 16);
	  in->x_scn.x_comdat = H_GET_8 (abfd, ext->Section.Selection);
	  return;
	}
      break;

    default:
      in->x_sym.x_tagndx.u32 = H_GET_32 (abfd, ext->Sym.WeakDefaultSymIndex);
      /* Characteristics is ignored.  */
      break;
    }
}

static unsigned int
coff_bigobj_swap_aux_out (bfd * abfd,
			  void * inp,
			  int type,
			  int in_class,
			  int indx ATTRIBUTE_UNUSED,
			  int numaux ATTRIBUTE_UNUSED,
			  void * extp)
{
  union internal_auxent * in = (union internal_auxent *) inp;
  AUXENT_BIGOBJ *ext = (AUXENT_BIGOBJ *) extp;

  memset (ext, 0, AUXESZ);

  switch (in_class)
    {
    case C_FILE:
      memcpy (ext->File.Name, in->x_file.x_n.x_fname, sizeof (ext->File.Name));

      return AUXESZ;

    case C_STAT:
    case C_LEAFSTAT:
    case C_HIDDEN:
      if (type == T_NULL)
	{
	  H_PUT_32 (abfd, in->x_scn.x_scnlen, ext->Section.Length);
	  H_PUT_16 (abfd, in->x_scn.x_nreloc,
		    ext->Section.NumberOfRelocations);
	  H_PUT_16 (abfd, in->x_scn.x_nlinno,
		    ext->Section.NumberOfLinenumbers);
	  H_PUT_32 (abfd, in->x_scn.x_checksum, ext->Section.Checksum);
	  H_PUT_16 (abfd, in->x_scn.x_associated & 0xffff,
		    ext->Section.Number);
	  H_PUT_16 (abfd, (in->x_scn.x_associated >> 16),
		    ext->Section.HighNumber);
	  H_PUT_8 (abfd, in->x_scn.x_comdat, ext->Section.Selection);
	  return AUXESZ;
	}
      break;
    }

  H_PUT_32 (abfd, in->x_sym.x_tagndx.u32, ext->Sym.WeakDefaultSymIndex);
  H_PUT_32 (abfd, 1, ext->Sym.WeakSearchType);

  return AUXESZ;
}

static const bfd_coff_backend_data bigobj_swap_table =
{
  coff_bigobj_swap_aux_in, coff_bigobj_swap_sym_in, coff_SWAP_lineno_in,
  coff_bigobj_swap_aux_out, coff_bigobj_swap_sym_out,
  coff_SWAP_lineno_out, coff_SWAP_reloc_out,
  coff_bigobj_swap_filehdr_out, coff_SWAP_aouthdr_out,
  coff_SWAP_scnhdr_out,
  FILHSZ_BIGOBJ, AOUTSZ, SCNHSZ, SYMESZ_BIGOBJ, AUXESZ_BIGOBJ,
   RELSZ, LINESZ, FILNMLEN_BIGOBJ,
  true,
  COFF_DEFAULT_LONG_SECTION_NAMES,
  COFF_DEFAULT_SECTION_ALIGNMENT_POWER,
  false,
  2,
  1U << 31,
  coff_bigobj_swap_filehdr_in, coff_SWAP_aouthdr_in, coff_SWAP_scnhdr_in,
  coff_SWAP_reloc_in, coff_bad_format_hook, coff_set_arch_mach_hook,
  coff_mkobject_hook, styp_to_sec_flags, coff_set_alignment_hook,
  coff_slurp_symbol_table, symname_in_debug_hook, coff_pointerize_aux_hook,
  coff_print_aux, coff_reloc16_extra_cases, coff_reloc16_estimate,
  coff_classify_symbol, coff_compute_section_file_positions,
  coff_start_final_link, coff_relocate_section, coff_rtype_to_howto,
  coff_adjust_symndx, coff_link_add_one_symbol,
  coff_link_output_has_begun, coff_final_link_postscript,
  bfd_pe_print_pdata	/* huh */
};

#endif /* COFF_WITH_PE_BIGOBJ */

#ifndef coff_close_and_cleanup
#define coff_close_and_cleanup		    _bfd_generic_close_and_cleanup
#endif

#ifndef coff_bfd_free_cached_info
#define coff_bfd_free_cached_info	    _bfd_coff_free_cached_info
#endif

#ifndef coff_get_section_contents
#define coff_get_section_contents	    _bfd_generic_get_section_contents
#endif

#ifndef coff_bfd_copy_private_symbol_data
#define coff_bfd_copy_private_symbol_data   _bfd_generic_bfd_copy_private_symbol_data
#endif

#ifndef coff_bfd_copy_private_header_data
#define coff_bfd_copy_private_header_data   _bfd_generic_bfd_copy_private_header_data
#endif

#ifndef coff_bfd_copy_private_section_data
#define coff_bfd_copy_private_section_data  _bfd_generic_bfd_copy_private_section_data
#endif

#ifndef coff_bfd_copy_private_bfd_data
#define coff_bfd_copy_private_bfd_data      _bfd_generic_bfd_copy_private_bfd_data
#endif

#ifndef coff_bfd_merge_private_bfd_data
#define coff_bfd_merge_private_bfd_data     _bfd_generic_bfd_merge_private_bfd_data
#endif

#ifndef coff_bfd_set_private_flags
#define coff_bfd_set_private_flags	    _bfd_generic_bfd_set_private_flags
#endif

#ifndef coff_bfd_print_private_bfd_data
#define coff_bfd_print_private_bfd_data     _bfd_generic_bfd_print_private_bfd_data
#endif

#ifndef coff_bfd_is_local_label_name
#define coff_bfd_is_local_label_name	    _bfd_coff_is_local_label_name
#endif

#ifndef coff_bfd_is_target_special_symbol
#define coff_bfd_is_target_special_symbol   _bfd_bool_bfd_asymbol_false
#endif

#ifndef coff_read_minisymbols
#define coff_read_minisymbols		    _bfd_generic_read_minisymbols
#endif

#ifndef coff_minisymbol_to_symbol
#define coff_minisymbol_to_symbol	    _bfd_generic_minisymbol_to_symbol
#endif

/* The reloc lookup routine must be supplied by each individual COFF
   backend.  */
#ifndef coff_bfd_reloc_type_lookup
#define coff_bfd_reloc_type_lookup	    _bfd_norelocs_bfd_reloc_type_lookup
#endif
#ifndef coff_bfd_reloc_name_lookup
#define coff_bfd_reloc_name_lookup    _bfd_norelocs_bfd_reloc_name_lookup
#endif

#ifndef coff_bfd_get_relocated_section_contents
#define coff_bfd_get_relocated_section_contents \
  bfd_generic_get_relocated_section_contents
#endif

#ifndef coff_bfd_relax_section
#define coff_bfd_relax_section		    bfd_generic_relax_section
#endif

#ifndef coff_bfd_gc_sections
#define coff_bfd_gc_sections		    bfd_coff_gc_sections
#endif

#ifndef coff_bfd_lookup_section_flags
#define coff_bfd_lookup_section_flags	    bfd_generic_lookup_section_flags
#endif

#ifndef coff_bfd_merge_sections
#define coff_bfd_merge_sections		    bfd_generic_merge_sections
#endif

#ifndef coff_bfd_is_group_section
#define coff_bfd_is_group_section	    bfd_generic_is_group_section
#endif

#ifndef coff_bfd_group_name
#define coff_bfd_group_name		    bfd_coff_group_name
#endif

#ifndef coff_bfd_discard_group
#define coff_bfd_discard_group		    bfd_generic_discard_group
#endif

#ifndef coff_section_already_linked
#define coff_section_already_linked \
  _bfd_coff_section_already_linked
#endif

#ifndef coff_bfd_define_common_symbol
#define coff_bfd_define_common_symbol	    bfd_generic_define_common_symbol
#endif

#ifndef coff_bfd_link_hide_symbol
#define coff_bfd_link_hide_symbol	    _bfd_generic_link_hide_symbol
#endif

#ifndef coff_bfd_define_start_stop
#define coff_bfd_define_start_stop	    bfd_generic_define_start_stop
#endif

#define CREATE_BIG_COFF_TARGET_VEC(VAR, NAME, EXTRA_O_FLAGS, EXTRA_S_FLAGS, UNDER, ALTERNATIVE, SWAP_TABLE)	\
const bfd_target VAR =							\
{									\
  NAME ,								\
  bfd_target_coff_flavour,						\
  BFD_ENDIAN_BIG,		/* Data byte order is big.  */		\
  BFD_ENDIAN_BIG,		/* Header byte order is big.  */	\
  /* object flags */							\
  (HAS_RELOC | EXEC_P | HAS_LINENO | HAS_DEBUG |			\
   HAS_SYMS | HAS_LOCALS | WP_TEXT | EXTRA_O_FLAGS),			\
  /* section flags */							\
  (SEC_HAS_CONTENTS | SEC_ALLOC | SEC_LOAD | SEC_RELOC | EXTRA_S_FLAGS),\
  UNDER,			/* Leading symbol underscore.  */	\
  '/',				/* AR_pad_char.  */			\
  15,				/* AR_max_namelen.  */			\
  0,				/* match priority.  */			\
  TARGET_KEEP_UNUSED_SECTION_SYMBOLS, /* keep unused section symbols.  */ \
									\
  /* Data conversion functions.  */					\
  bfd_getb64, bfd_getb_signed_64, bfd_putb64,				\
  bfd_getb32, bfd_getb_signed_32, bfd_putb32,				\
  bfd_getb16, bfd_getb_signed_16, bfd_putb16,				\
									\
  /* Header conversion functions.  */					\
  bfd_getb64, bfd_getb_signed_64, bfd_putb64,				\
  bfd_getb32, bfd_getb_signed_32, bfd_putb32,				\
  bfd_getb16, bfd_getb_signed_16, bfd_putb16,				\
									\
  {				/* bfd_check_format.  */		\
    _bfd_dummy_target,							\
    coff_object_p,							\
    bfd_generic_archive_p,						\
    _bfd_dummy_target							\
  },									\
  {				/* bfd_set_format.  */			\
    _bfd_bool_bfd_false_error,						\
    coff_mkobject,							\
    _bfd_generic_mkarchive,						\
    _bfd_bool_bfd_false_error						\
  },									\
  {				/* bfd_write_contents.  */		\
    _bfd_bool_bfd_false_error,						\
    coff_write_object_contents,						\
    _bfd_write_archive_contents,					\
    _bfd_bool_bfd_false_error						\
  },									\
									\
  BFD_JUMP_TABLE_GENERIC (coff),					\
  BFD_JUMP_TABLE_COPY (coff),						\
  BFD_JUMP_TABLE_CORE (_bfd_nocore),					\
  BFD_JUMP_TABLE_ARCHIVE (_bfd_archive_coff),				\
  BFD_JUMP_TABLE_SYMBOLS (coff),					\
  BFD_JUMP_TABLE_RELOCS (coff),						\
  BFD_JUMP_TABLE_WRITE (coff),						\
  BFD_JUMP_TABLE_LINK (coff),						\
  BFD_JUMP_TABLE_DYNAMIC (_bfd_nodynamic),				\
									\
  ALTERNATIVE,								\
									\
  SWAP_TABLE								\
};

#define CREATE_BIGHDR_COFF_TARGET_VEC(VAR, NAME, EXTRA_O_FLAGS, EXTRA_S_FLAGS, UNDER, ALTERNATIVE, SWAP_TABLE)	\
const bfd_target VAR =							\
{									\
  NAME ,								\
  bfd_target_coff_flavour,						\
  BFD_ENDIAN_LITTLE,		/* Data byte order is little.  */	\
  BFD_ENDIAN_BIG,		/* Header byte order is big.  */	\
  /* object flags */							\
  (HAS_RELOC | EXEC_P | HAS_LINENO | HAS_DEBUG |			\
   HAS_SYMS | HAS_LOCALS | WP_TEXT | EXTRA_O_FLAGS),			\
  /* section flags */							\
  (SEC_HAS_CONTENTS | SEC_ALLOC | SEC_LOAD | SEC_RELOC | EXTRA_S_FLAGS),\
  UNDER,			/* Leading symbol underscore.  */	\
  '/',				/* AR_pad_char.  */			\
  15,				/* AR_max_namelen.  */			\
  0,				/* match priority.  */			\
  TARGET_KEEP_UNUSED_SECTION_SYMBOLS, /* keep unused section symbols.  */ \
									\
  /* Data conversion functions.  */					\
  bfd_getb64, bfd_getb_signed_64, bfd_putb64,				\
  bfd_getb32, bfd_getb_signed_32, bfd_putb32,				\
  bfd_getb16, bfd_getb_signed_16, bfd_putb16,				\
									\
  /* Header conversion functions.  */					\
  bfd_getb64, bfd_getb_signed_64, bfd_putb64,				\
  bfd_getb32, bfd_getb_signed_32, bfd_putb32,				\
  bfd_getb16, bfd_getb_signed_16, bfd_putb16,				\
									\
  {				/* bfd_check_format.  */		\
    _bfd_dummy_target,							\
    coff_object_p,							\
    bfd_generic_archive_p,						\
    _bfd_dummy_target							\
  },									\
  {				/* bfd_set_format.  */			\
    _bfd_bool_bfd_false_error,						\
    coff_mkobject,							\
    _bfd_generic_mkarchive,						\
    _bfd_bool_bfd_false_error						\
  },									\
  {				/* bfd_write_contents.  */		\
    _bfd_bool_bfd_false_error,						\
    coff_write_object_contents,						\
    _bfd_write_archive_contents,					\
    _bfd_bool_bfd_false_error						\
  },									\
									\
  BFD_JUMP_TABLE_GENERIC (coff),					\
  BFD_JUMP_TABLE_COPY (coff),						\
  BFD_JUMP_TABLE_CORE (_bfd_nocore),					\
  BFD_JUMP_TABLE_ARCHIVE (_bfd_archive_coff),				\
  BFD_JUMP_TABLE_SYMBOLS (coff),					\
  BFD_JUMP_TABLE_RELOCS (coff),						\
  BFD_JUMP_TABLE_WRITE (coff),						\
  BFD_JUMP_TABLE_LINK (coff),						\
  BFD_JUMP_TABLE_DYNAMIC (_bfd_nodynamic),				\
									\
  ALTERNATIVE,								\
									\
  SWAP_TABLE								\
};

#define CREATE_LITTLE_COFF_TARGET_VEC(VAR, NAME, EXTRA_O_FLAGS, EXTRA_S_FLAGS, UNDER, ALTERNATIVE, SWAP_TABLE)	\
const bfd_target VAR =							\
{									\
  NAME ,								\
  bfd_target_coff_flavour,						\
  BFD_ENDIAN_LITTLE,		/* Data byte order is little.  */	\
  BFD_ENDIAN_LITTLE,		/* Header byte order is little.  */	\
	/* object flags */						\
  (HAS_RELOC | EXEC_P | HAS_LINENO | HAS_DEBUG |			\
   HAS_SYMS | HAS_LOCALS | WP_TEXT | EXTRA_O_FLAGS),			\
	/* section flags */						\
  (SEC_HAS_CONTENTS | SEC_ALLOC | SEC_LOAD | SEC_RELOC | EXTRA_S_FLAGS),\
  UNDER,			/* Leading symbol underscore.  */	\
  '/',				/* AR_pad_char.  */			\
  15,				/* AR_max_namelen.  */			\
  0,				/* match priority.  */			\
  TARGET_KEEP_UNUSED_SECTION_SYMBOLS, /* keep unused section symbols.  */ \
									\
  /* Data conversion functions.  */					\
  bfd_getl64, bfd_getl_signed_64, bfd_putl64,				\
  bfd_getl32, bfd_getl_signed_32, bfd_putl32,				\
  bfd_getl16, bfd_getl_signed_16, bfd_putl16,				\
  /* Header conversion functions.  */					\
  bfd_getl64, bfd_getl_signed_64, bfd_putl64,				\
  bfd_getl32, bfd_getl_signed_32, bfd_putl32,				\
  bfd_getl16, bfd_getl_signed_16, bfd_putl16,				\
									\
  {				/* bfd_check_format.  */		\
    _bfd_dummy_target,							\
    coff_object_p,							\
    bfd_generic_archive_p,						\
    _bfd_dummy_target							\
  },									\
  {				/* bfd_set_format.  */			\
    _bfd_bool_bfd_false_error,						\
    coff_mkobject,							\
    _bfd_generic_mkarchive,						\
    _bfd_bool_bfd_false_error						\
  },									\
  {				/* bfd_write_contents.  */		\
    _bfd_bool_bfd_false_error,						\
    coff_write_object_contents,						\
    _bfd_write_archive_contents,					\
    _bfd_bool_bfd_false_error						\
  },									\
									\
  BFD_JUMP_TABLE_GENERIC (coff),					\
  BFD_JUMP_TABLE_COPY (coff),						\
  BFD_JUMP_TABLE_CORE (_bfd_nocore),					\
  BFD_JUMP_TABLE_ARCHIVE (_bfd_archive_coff),				\
  BFD_JUMP_TABLE_SYMBOLS (coff),					\
  BFD_JUMP_TABLE_RELOCS (coff),						\
  BFD_JUMP_TABLE_WRITE (coff),						\
  BFD_JUMP_TABLE_LINK (coff),						\
  BFD_JUMP_TABLE_DYNAMIC (_bfd_nodynamic),				\
									\
  ALTERNATIVE,								\
									\
  SWAP_TABLE								\
};
