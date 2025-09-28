/* BFD back-end for PDP-11 a.out binaries.
   Copyright (C) 2001-2023 Free Software Foundation, Inc.

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
   MA 02110-1301, USA. */


/* BFD backend for PDP-11, running 2.11BSD in particular.

   This file was hacked up by looking hard at the existing vaxnetbsd
   back end and the header files in 2.11BSD.  The symbol table format
   of 2.11BSD has been extended to accommodate .stab symbols.  See
   struct pdp11_external_nlist below for details.

   TODO
   * support for V7 file formats
   * support for overlay object files (see 2.11 a.out(5))
   * support for old and very old archives
   (see 2.11 ar(5), historical section)

   Search for TODO to find other areas needing more work.  */

#define	BYTES_IN_WORD	2
#define	BYTES_IN_LONG	4
#define ARCH_SIZE	16
#undef TARGET_IS_BIG_ENDIAN_P

#define	TARGET_PAGE_SIZE	8192
#define	SEGMENT__SIZE	TARGET_PAGE_SIZE

#define	DEFAULT_ARCH	bfd_arch_pdp11
#define	DEFAULT_MID	M_PDP11

/* Do not "beautify" the CONCAT* macro args.  Traditional C will not
   remove whitespace added here, and thus will fail to concatenate
   the tokens.  */
#define MY(OP) CONCAT2 (pdp11_aout_,OP)

/* This needs to start with a.out so GDB knows it is an a.out variant.  */
#define TARGETNAME "a.out-pdp11"

/* This is the normal load address for executables.  */
#define TEXT_START_ADDR		0

/* The header is not included in the text segment.  */
#define N_HEADER_IN_TEXT(x)	0

/* There is no flags field.  */
#define N_FLAGS(execp)		0

#define N_SET_FLAGS(execp, flags) do { } while (0)
#define N_BADMAG(x) (N_MAGIC(x) != OMAGIC	\
		     && N_MAGIC(x) != NMAGIC	\
		     && N_MAGIC(x) != IMAGIC	\
		     && N_MAGIC(x) != ZMAGIC)

#include "sysdep.h"
#include <limits.h>
#include "bfd.h"

#define external_exec pdp11_external_exec
struct pdp11_external_exec
{
  bfd_byte e_info[2];		/* Magic number.  */
  bfd_byte e_text[2];		/* Length of text section in bytes.  */
  bfd_byte e_data[2];		/* Length of data section in bytes.  */
  bfd_byte e_bss[2];		/* Length of bss area in bytes.  */
  bfd_byte e_syms[2];		/* Length of symbol table in bytes.  */
  bfd_byte e_entry[2];		/* Start address.  */
  bfd_byte e_unused[2];		/* Not used.  */
  bfd_byte e_flag[2];		/* Relocation info stripped.  */
  bfd_byte e_relocatable;	/* Ugly hack.  */
};

#define	EXEC_BYTES_SIZE	(8 * 2)

#define	A_MAGIC1	OMAGIC
#define OMAGIC		0407	/* ...object file or impure executable.  */
#define	A_MAGIC2	NMAGIC
#define NMAGIC		0410	/* Pure executable.  */
#define ZMAGIC		0413	/* Demand-paged executable.  */
#define	IMAGIC		0411	/* Separated I&D.  */
#define	A_MAGIC3	IMAGIC
#define	A_MAGIC4	0405	/* Overlay.  */
#define	A_MAGIC5	0430	/* Auto-overlay (nonseparate).  */
#define	A_MAGIC6	0431	/* Auto-overlay (separate).  */
#define QMAGIC		0
#define BMAGIC		0

#define A_FLAG_RELOC_STRIPPED	0x0001

/* The following struct defines the format of an entry in the object file
   symbol table.  In the original 2.11BSD struct the index into the string
   table is stored as a long, but the PDP11 C convention for storing a long in
   memory placed the most significant word first even though the bytes within a
   word are stored least significant first.  So here the string table index is
   considered to be just 16 bits and the first two bytes of the struct were
   previously named e_unused.  To extend the symbol table format to accommodate
   .stab symbols, the e_unused bytes are renamed e_desc to store the desc field
   of the .stab symbol.  The GDP Project's STABS document says that the "other"
   field is almost always unused and can be set to zero; the only nonzero cases
   identified were for stabs in their own sections, which does not apply for
   pdp11 a.out format, and for a special case of GNU Modula2 which is not
   supported for the PDP11.  */
#define external_nlist pdp11_external_nlist
struct pdp11_external_nlist
{
  bfd_byte e_desc[2];		/* The desc field for .stab symbols, else 0.  */
  bfd_byte e_strx[2];		/* Index into string table of name.  */
  bfd_byte e_type[1];		/* Type of symbol.  */
  bfd_byte e_ovly[1];		/* Overlay number.  */
  bfd_byte e_value[2];		/* Value of symbol.  */
};

#define	EXTERNAL_NLIST_SIZE	8

#define N_TXTOFF(x)	(EXEC_BYTES_SIZE)
#define N_DATOFF(x)	(N_TXTOFF(x) + (x)->a_text)
#define N_TRELOFF(x)	(N_DATOFF(x) + (x)->a_data)
#define N_DRELOFF(x)	(N_TRELOFF(x) + (x)->a_trsize)
#define N_SYMOFF(x)	(N_DRELOFF(x) + (x)->a_drsize)
#define N_STROFF(x)	(N_SYMOFF(x) + (x)->a_syms)

#define WRITE_HEADERS(abfd, execp) pdp11_aout_write_headers (abfd, execp)

#include "libbfd.h"
#include "libaout.h"

#define SWAP_MAGIC(ext) bfd_getl16 (ext)

#define MY_entry_is_text_address 1

#define MY_write_object_contents MY(write_object_contents)
static bool MY(write_object_contents) (bfd *);
#define MY_text_includes_header 1

#define MY_BFD_TARGET

#include "aout-target.h"

/* Start of modified aoutx.h.  */
#define KEEPIT udata.i

#include <string.h>		/* For strchr and friends.  */
#include "bfd.h"
#include "sysdep.h"
#include "safe-ctype.h"
#include "bfdlink.h"

#include "libaout.h"
#include "aout/aout64.h"
#include "aout/stab_gnu.h"
#include "aout/ar.h"

/* The symbol type numbers for the 16-bit a.out format from 2.11BSD differ from
   those defined in aout64.h so we must redefine them here.  N_EXT changes from
   0x01 to 0x20 which creates a conflict with some .stab values, in particular
   between undefined externals (N_UNDF+N_EXT) vs. global variables (N_GYSM) and
   between external bss symbols (N_BSS+N_EXT) vs. function names (N_FUN).  We
   disambiguate those conflicts with a hack in is_stab() to look for the ':' in
   the global variable or function name string.  */
#undef N_TYPE
#undef N_UNDF
#undef N_ABS
#undef N_TEXT
#undef N_DATA
#undef N_BSS
#undef N_REG
#undef N_FN
#undef N_EXT
#undef N_STAB
#define N_TYPE		0x1f	/* Type mask.  */
#define N_UNDF		0x00	/* Undefined.  */
#define N_ABS		0x01	/* Absolute.  */
#define N_TEXT		0x02	/* Text segment.  */
#define N_DATA		0x03	/* Data segment.  */
#define N_BSS		0x04	/* Bss segment.  */
#define N_REG		0x14	/* Register symbol.  */
#define N_FN		0x1f	/* File name.  */
#define N_EXT		0x20	/* External flag.  */
/* Type numbers from .stab entries that could conflict:
	N_GSYM		0x20	   Global variable [conflict with external undef]
	N_FNAME		0x22	   Function name (for BSD Fortran) [ignored]
	N_FUN		0x24	   Function name [conflict with external BSS]
	N_NOMAP		0x34	   No DST map for sym. [ext. reg. doesn't exist]
*/

#define RELOC_SIZE 2

#define RELFLG		0x0001	/* PC-relative flag.  */
#define RTYPE		0x000e	/* Type mask.  */
#define RIDXMASK	0xfff0	/* Index mask.  */

#define RABS		0x00	/* Absolute.  */
#define RTEXT		0x02	/* Text.  */
#define RDATA		0x04	/* Data.  */
#define RBSS		0x06	/* Bss.  */
#define REXT		0x08	/* External.  */

#define RINDEX(x)	(((x) & 0xfff0) >> 4)

#ifndef MY_final_link_relocate
#define MY_final_link_relocate _bfd_final_link_relocate
#endif

#ifndef MY_relocate_contents
#define MY_relocate_contents _bfd_relocate_contents
#endif

/* A hash table used for header files with N_BINCL entries.  */

struct aout_link_includes_table
{
  struct bfd_hash_table root;
};

/* A linked list of totals that we have found for a particular header
   file.  */

struct aout_link_includes_totals
{
  struct aout_link_includes_totals *next;
  bfd_vma total;
};

/* An entry in the header file hash table.  */

struct aout_link_includes_entry
{
  struct bfd_hash_entry root;
  /* List of totals we have found for this file.  */
  struct aout_link_includes_totals *totals;
};

/* During the final link step we need to pass around a bunch of
   information, so we do it in an instance of this structure.  */

struct aout_final_link_info
{
  /* General link information.  */
  struct bfd_link_info *info;
  /* Output bfd.  */
  bfd *output_bfd;
  /* Reloc file positions.  */
  file_ptr treloff, dreloff;
  /* File position of symbols.  */
  file_ptr symoff;
  /* String table.  */
  struct bfd_strtab_hash *strtab;
  /* Header file hash table.  */
  struct aout_link_includes_table includes;
  /* A buffer large enough to hold the contents of any section.  */
  bfd_byte *contents;
  /* A buffer large enough to hold the relocs of any section.  */
  void * relocs;
  /* A buffer large enough to hold the symbol map of any input BFD.  */
  int *symbol_map;
  /* A buffer large enough to hold output symbols of any input BFD.  */
  struct external_nlist *output_syms;
};

/* Copy of the link_info.separate_code boolean to select the output format with
   separate instruction and data spaces selected by --imagic */
static bool separate_i_d = false;

reloc_howto_type howto_table_pdp11[] =
{
  /* type	       rs size bsz  pcrel bitpos ovrf			  sf name     part_inpl readmask  setmask    pcdone */
HOWTO( 0,	       0,  2,  16,  false, 0, complain_overflow_dont,0,"16",	true, 0x0000ffff,0x0000ffff, false),
HOWTO( 1,	       0,  2,  16,  true,  0, complain_overflow_dont,0,"DISP16",	true, 0x0000ffff,0x0000ffff, false),
HOWTO( 2,	       0,  4,  32,  false, 0, complain_overflow_dont,0,"32",	true, 0x0000ffff,0x0000ffff, false),
};

#define TABLE_SIZE(TABLE)	(sizeof(TABLE)/sizeof(TABLE[0]))


static bool aout_link_check_archive_element (bfd *, struct bfd_link_info *,
					     struct bfd_link_hash_entry *,
					     const char *, bool *);
static bool aout_link_add_object_symbols (bfd *, struct bfd_link_info *);
static bool aout_link_add_symbols (bfd *, struct bfd_link_info *);
static bool aout_link_write_symbols (struct aout_final_link_info *, bfd *);


reloc_howto_type *
NAME (aout, reloc_type_lookup) (bfd * abfd ATTRIBUTE_UNUSED,
				bfd_reloc_code_real_type code)
{
  switch (code)
    {
    case BFD_RELOC_16:
      return &howto_table_pdp11[0];
    case BFD_RELOC_16_PCREL:
      return &howto_table_pdp11[1];
    case BFD_RELOC_32:
      return &howto_table_pdp11[2];
    default:
      return NULL;
    }
}

reloc_howto_type *
NAME (aout, reloc_name_lookup) (bfd *abfd ATTRIBUTE_UNUSED,
				      const char *r_name)
{
  unsigned int i;

  for (i = 0;
       i < sizeof (howto_table_pdp11) / sizeof (howto_table_pdp11[0]);
       i++)
    if (howto_table_pdp11[i].name != NULL
	&& strcasecmp (howto_table_pdp11[i].name, r_name) == 0)
      return &howto_table_pdp11[i];

  return NULL;
}

/* Disambiguate conflicts between normal symbol types and .stab symbol types
   (undefined externals N_UNDF+N_EXT vs. global variables N_GYSM and external
   bss symbols N_BSS+N_EXT vs. function names N_FUN) with a hack to look for
   the ':' in the global variable or function name string.  */

static int
is_stab (int type, const char *name)
{
  if (type == N_GSYM || type == N_FUN)
    return strchr (name, ':') != NULL;
  return type > N_FUN;
}

static int
pdp11_aout_write_headers (bfd *abfd, struct internal_exec *execp)
{
  struct external_exec exec_bytes;

  if (adata(abfd).magic == undecided_magic)
    NAME (aout, adjust_sizes_and_vmas) (abfd);

  execp->a_syms = bfd_get_symcount (abfd) * EXTERNAL_NLIST_SIZE;
  execp->a_entry = bfd_get_start_address (abfd);

  if (obj_textsec (abfd)->reloc_count > 0
      || obj_datasec (abfd)->reloc_count > 0)
    {
      execp->a_trsize = execp->a_text;
      execp->a_drsize = execp->a_data;
    }
  else
    {
      execp->a_trsize = 0;
      execp->a_drsize = 0;
    }

  NAME (aout, swap_exec_header_out) (abfd, execp, & exec_bytes);

  if (bfd_seek (abfd, (file_ptr) 0, SEEK_SET) != 0)
    return false;

  if (bfd_bwrite ((void *) &exec_bytes, (bfd_size_type) EXEC_BYTES_SIZE, abfd)
      != EXEC_BYTES_SIZE)
    return false;

  /* Now write out reloc info, followed by syms and strings.  */
  if (bfd_get_outsymbols (abfd) != NULL
      && bfd_get_symcount (abfd) != 0)
    {
      if (bfd_seek (abfd, (file_ptr) (N_SYMOFF (execp)), SEEK_SET) != 0)
	return false;

      if (! NAME (aout, write_syms) (abfd))
	return false;
    }

  if (obj_textsec (abfd)->reloc_count > 0
      || obj_datasec (abfd)->reloc_count > 0)
    {
      if (bfd_seek (abfd, (file_ptr) (N_TRELOFF (execp)), SEEK_SET) != 0
	  || !NAME (aout, squirt_out_relocs) (abfd, obj_textsec (abfd))
	  || bfd_seek (abfd, (file_ptr) (N_DRELOFF (execp)), SEEK_SET) != 0
	  || !NAME (aout, squirt_out_relocs) (abfd, obj_datasec (abfd)))
	return false;
    }

  return true;
}

/* Write an object file.
   Section contents have already been written.  We write the
   file header, symbols, and relocation.  */

static bool
MY(write_object_contents) (bfd *abfd)
{
  struct internal_exec *execp = exec_hdr (abfd);

  /* We must make certain that the magic number has been set.  This
     will normally have been done by set_section_contents, but only if
     there actually are some section contents.  */
  if (! abfd->output_has_begun)
    NAME (aout, adjust_sizes_and_vmas) (abfd);

  obj_reloc_entry_size (abfd) = RELOC_SIZE;

  return WRITE_HEADERS (abfd, execp);
}

/* Swap the information in an executable header @var{raw_bytes} taken
   from a raw byte stream memory image into the internal exec header
   structure "execp".  */

#ifndef NAME_swap_exec_header_in
void
NAME (aout, swap_exec_header_in) (bfd *abfd,
				  struct external_exec *bytes,
				  struct internal_exec *execp)
{
  /* The internal_exec structure has some fields that are unused in this
     configuration (IE for i960), so ensure that all such uninitialized
     fields are zero'd out.  There are places where two of these structs
     are memcmp'd, and thus the contents do matter.  */
  memset ((void *) execp, 0, sizeof (struct internal_exec));
  /* Now fill in fields in the execp, from the bytes in the raw data.  */
  execp->a_info   = GET_MAGIC (abfd, bytes->e_info);
  execp->a_text   = GET_WORD (abfd, bytes->e_text);
  execp->a_data   = GET_WORD (abfd, bytes->e_data);
  execp->a_bss    = GET_WORD (abfd, bytes->e_bss);
  execp->a_syms   = GET_WORD (abfd, bytes->e_syms);
  execp->a_entry  = GET_WORD (abfd, bytes->e_entry);

  if (GET_WORD (abfd, bytes->e_flag) & A_FLAG_RELOC_STRIPPED)
    {
      execp->a_trsize = 0;
      execp->a_drsize = 0;
    }
  else
    {
      execp->a_trsize = execp->a_text;
      execp->a_drsize = execp->a_data;
    }
}
#define NAME_swap_exec_header_in NAME (aout, swap_exec_header_in)
#endif

/*  Swap the information in an internal exec header structure
    "execp" into the buffer "bytes" ready for writing to disk.  */
void
NAME (aout, swap_exec_header_out) (bfd *abfd,
				   struct internal_exec *execp,
				   struct external_exec *bytes)
{
  /* Now fill in fields in the raw data, from the fields in the exec struct.  */
  PUT_MAGIC (abfd, execp->a_info,		bytes->e_info);
  PUT_WORD (abfd, execp->a_text,		bytes->e_text);
  PUT_WORD (abfd, execp->a_data,		bytes->e_data);
  PUT_WORD (abfd, execp->a_bss,			bytes->e_bss);
  PUT_WORD (abfd, execp->a_syms,		bytes->e_syms);
  PUT_WORD (abfd, execp->a_entry,		bytes->e_entry);
  PUT_WORD (abfd, 0,				bytes->e_unused);

  if ((execp->a_trsize == 0 || execp->a_text == 0)
      && (execp->a_drsize == 0 || execp->a_data == 0))
    PUT_WORD (abfd, A_FLAG_RELOC_STRIPPED, bytes->e_flag);
  else if (execp->a_trsize == execp->a_text
	   && execp->a_drsize == execp->a_data)
    PUT_WORD (abfd, 0, bytes->e_flag);
  else
    {
      /* TODO: print a proper warning message.  */
      fprintf (stderr, "BFD:%s:%d: internal error\n", __FILE__, __LINE__);
      PUT_WORD (abfd, 0,			bytes->e_flag);
    }
}

/* Make all the section for an a.out file.  */

bool
NAME (aout, make_sections) (bfd *abfd)
{
  if (obj_textsec (abfd) == NULL && bfd_make_section (abfd, ".text") == NULL)
    return false;
  if (obj_datasec (abfd) == NULL && bfd_make_section (abfd, ".data") == NULL)
    return false;
  if (obj_bsssec (abfd) == NULL  && bfd_make_section (abfd, ".bss") == NULL)
    return false;
  return true;
}

/* Some a.out variant thinks that the file open in ABFD
   checking is an a.out file.  Do some more checking, and set up
   for access if it really is.  Call back to the calling
   environment's "finish up" function just before returning, to
   handle any last-minute setup.  */

bfd_cleanup
NAME (aout, some_aout_object_p) (bfd *abfd,
				 struct internal_exec *execp,
				 bfd_cleanup (*callback_to_real_object_p) (bfd *))
{
  struct aout_data_struct *rawptr, *oldrawptr;
  bfd_cleanup cleanup;
  size_t amt = sizeof (struct aout_data_struct);

  rawptr = bfd_zalloc (abfd, amt);
  if (rawptr == NULL)
    return 0;

  oldrawptr = abfd->tdata.aout_data;
  abfd->tdata.aout_data = rawptr;

  /* Copy the contents of the old tdata struct.  */
  if (oldrawptr != NULL)
    *abfd->tdata.aout_data = *oldrawptr;

  abfd->tdata.aout_data->a.hdr = &rawptr->e;
  *(abfd->tdata.aout_data->a.hdr) = *execp;	/* Copy in the internal_exec struct.  */
  execp = abfd->tdata.aout_data->a.hdr;

  /* Set the file flags.  */
  abfd->flags = BFD_NO_FLAGS;
  if (execp->a_drsize || execp->a_trsize)
    abfd->flags |= HAS_RELOC;
  /* Setting of EXEC_P has been deferred to the bottom of this function.  */
  if (execp->a_syms)
    abfd->flags |= HAS_LINENO | HAS_DEBUG | HAS_SYMS | HAS_LOCALS;
  if (N_DYNAMIC (execp))
    abfd->flags |= DYNAMIC;

  if (N_MAGIC (execp) == ZMAGIC)
    {
      abfd->flags |= D_PAGED | WP_TEXT;
      adata (abfd).magic = z_magic;
    }
  else if (N_MAGIC (execp) == NMAGIC)
    {
      abfd->flags |= WP_TEXT;
      adata (abfd).magic = n_magic;
    }
  else if (N_MAGIC (execp) == OMAGIC)
    adata (abfd).magic = o_magic;
  else if (N_MAGIC (execp) == IMAGIC)
    adata (abfd).magic = i_magic;
  else
    {
      /* Should have been checked with N_BADMAG before this routine
	 was called.  */
      abort ();
    }

  abfd->start_address = execp->a_entry;

  obj_aout_symbols (abfd) = NULL;
  abfd->symcount = execp->a_syms / sizeof (struct external_nlist);

  /* The default relocation entry size is that of traditional V7 Unix.  */
  obj_reloc_entry_size (abfd) = RELOC_SIZE;

  /* The default symbol entry size is that of traditional Unix.  */
  obj_symbol_entry_size (abfd) = EXTERNAL_NLIST_SIZE;

#ifdef USE_MMAP
  bfd_init_window (&obj_aout_sym_window (abfd));
  bfd_init_window (&obj_aout_string_window (abfd));
#endif

  obj_aout_external_syms (abfd) = NULL;
  obj_aout_external_strings (abfd) = NULL;
  obj_aout_sym_hashes (abfd) = NULL;

  if (! NAME (aout, make_sections) (abfd))
    return NULL;

  obj_datasec (abfd)->size = execp->a_data;
  obj_bsssec (abfd)->size = execp->a_bss;

  obj_textsec (abfd)->flags =
    (execp->a_trsize != 0
     ? (SEC_ALLOC | SEC_LOAD | SEC_CODE | SEC_HAS_CONTENTS | SEC_RELOC)
     : (SEC_ALLOC | SEC_LOAD | SEC_CODE | SEC_HAS_CONTENTS));
  obj_datasec (abfd)->flags =
    (execp->a_drsize != 0
     ? (SEC_ALLOC | SEC_LOAD | SEC_DATA | SEC_HAS_CONTENTS | SEC_RELOC)
     : (SEC_ALLOC | SEC_LOAD | SEC_DATA | SEC_HAS_CONTENTS));
  obj_bsssec (abfd)->flags = SEC_ALLOC;

#ifdef THIS_IS_ONLY_DOCUMENTATION
  /* The common code can't fill in these things because they depend
     on either the start address of the text segment, the rounding
     up of virtual addresses between segments, or the starting file
     position of the text segment -- all of which varies among different
     versions of a.out.  */

  /* Call back to the format-dependent code to fill in the rest of the
     fields and do any further cleanup.  Things that should be filled
     in by the callback:  */
  struct exec *execp = exec_hdr (abfd);

  obj_textsec (abfd)->size = N_TXTSIZE (execp);
  /* Data and bss are already filled in since they're so standard.  */

  /* The virtual memory addresses of the sections.  */
  obj_textsec (abfd)->vma = N_TXTADDR (execp);
  obj_datasec (abfd)->vma = N_DATADDR (execp);
  obj_bsssec  (abfd)->vma = N_BSSADDR (execp);

  /* The file offsets of the sections.  */
  obj_textsec (abfd)->filepos = N_TXTOFF (execp);
  obj_datasec (abfd)->filepos = N_DATOFF (execp);

  /* The file offsets of the relocation info.  */
  obj_textsec (abfd)->rel_filepos = N_TRELOFF (execp);
  obj_datasec (abfd)->rel_filepos = N_DRELOFF (execp);

  /* The file offsets of the string table and symbol table.  */
  obj_str_filepos (abfd) = N_STROFF (execp);
  obj_sym_filepos (abfd) = N_SYMOFF (execp);

  /* Determine the architecture and machine type of the object file.  */
  abfd->obj_arch = bfd_arch_obscure;

  adata(abfd)->page_size = TARGET_PAGE_SIZE;
  adata(abfd)->segment_size = SEGMENT_SIZE;
  adata(abfd)->exec_bytes_size = EXEC_BYTES_SIZE;

  return _bfd_no_cleanup;

  /* The architecture is encoded in various ways in various a.out variants,
     or is not encoded at all in some of them.  The relocation size depends
     on the architecture and the a.out variant.  Finally, the return value
     is the bfd_target vector in use.  If an error occurs, return zero and
     set bfd_error to the appropriate error code.

     Formats such as b.out, which have additional fields in the a.out
     header, should cope with them in this callback as well.  */
#endif	/* DOCUMENTATION */

  cleanup = (*callback_to_real_object_p)(abfd);

  /* Now that the segment addresses have been worked out, take a better
     guess at whether the file is executable.  If the entry point
     is within the text segment, assume it is.  (This makes files
     executable even if their entry point address is 0, as long as
     their text starts at zero.).

     This test had to be changed to deal with systems where the text segment
     runs at a different location than the default.  The problem is that the
     entry address can appear to be outside the text segment, thus causing an
     erroneous conclusion that the file isn't executable.

     To fix this, we now accept any non-zero entry point as an indication of
     executability.  This will work most of the time, since only the linker
     sets the entry point, and that is likely to be non-zero for most systems. */

  if (execp->a_entry != 0
      || (execp->a_entry >= obj_textsec (abfd)->vma
	  && execp->a_entry < (obj_textsec (abfd)->vma
			       + obj_textsec (abfd)->size)
	  && execp->a_trsize == 0
	  && execp->a_drsize == 0))
    abfd->flags |= EXEC_P;
#ifdef STAT_FOR_EXEC
  else
    {
      struct stat stat_buf;

      /* The original heuristic doesn't work in some important cases.
	The a.out file has no information about the text start
	address.  For files (like kernels) linked to non-standard
	addresses (ld -Ttext nnn) the entry point may not be between
	the default text start (obj_textsec(abfd)->vma) and
	(obj_textsec(abfd)->vma) + text size.  This is not just a mach
	issue.  Many kernels are loaded at non standard addresses.  */
      if (abfd->iostream != NULL
	  && (abfd->flags & BFD_IN_MEMORY) == 0
	  && (fstat(fileno((FILE *) (abfd->iostream)), &stat_buf) == 0)
	  && ((stat_buf.st_mode & 0111) != 0))
	abfd->flags |= EXEC_P;
    }
#endif /* STAT_FOR_EXEC */

  if (!cleanup)
    {
      free (rawptr);
      abfd->tdata.aout_data = oldrawptr;
    }
  return cleanup;
}

/* Initialize ABFD for use with a.out files.  */

bool
NAME (aout, mkobject) (bfd *abfd)
{
  struct aout_data_struct  *rawptr;
  size_t amt = sizeof (struct aout_data_struct);

  bfd_set_error (bfd_error_system_call);

  /* Use an intermediate variable for clarity.  */
  rawptr = bfd_zalloc (abfd, amt);

  if (rawptr == NULL)
    return false;

  abfd->tdata.aout_data = rawptr;
  exec_hdr (abfd) = &(rawptr->e);

  obj_textsec (abfd) = NULL;
  obj_datasec (abfd) = NULL;
  obj_bsssec (abfd)  = NULL;

  return true;
}

/* Keep track of machine architecture and machine type for
   a.out's. Return the <<machine_type>> for a particular
   architecture and machine, or <<M_UNKNOWN>> if that exact architecture
   and machine can't be represented in a.out format.

   If the architecture is understood, machine type 0 (default)
   is always understood.  */

enum machine_type
NAME (aout, machine_type) (enum bfd_architecture arch,
			   unsigned long machine,
			   bool *unknown)
{
  enum machine_type arch_flags;

  arch_flags = M_UNKNOWN;
  *unknown = true;

  switch (arch)
    {
    case bfd_arch_sparc:
      if (machine == 0
	  || machine == bfd_mach_sparc
	  || machine == bfd_mach_sparc_sparclite
	  || machine == bfd_mach_sparc_v9)
	arch_flags = M_SPARC;
      else if (machine == bfd_mach_sparc_sparclet)
	arch_flags = M_SPARCLET;
      break;

    case bfd_arch_i386:
      if (machine == 0
	  || machine == bfd_mach_i386_i386
	  || machine == bfd_mach_i386_i386_intel_syntax)
	arch_flags = M_386;
      break;

    case bfd_arch_arm:
      if (machine == 0)	arch_flags = M_ARM;
      break;

    case bfd_arch_mips:
      switch (machine)
	{
	case 0:
	case 2000:
	case bfd_mach_mips3000:
	  arch_flags = M_MIPS1;
	  break;
	case bfd_mach_mips4000: /* MIPS3 */
	case bfd_mach_mips4400:
	case bfd_mach_mips8000: /* MIPS4 */
	case bfd_mach_mips6000: /* Real MIPS2: */
	  arch_flags = M_MIPS2;
	  break;
	default:
	  arch_flags = M_UNKNOWN;
	  break;
	}
      break;

    case bfd_arch_ns32k:
      switch (machine)
	{
	case 0:			arch_flags = M_NS32532; break;
	case 32032:		arch_flags = M_NS32032; break;
	case 32532:		arch_flags = M_NS32532; break;
	default:		arch_flags = M_UNKNOWN; break;
	}
      break;

    case bfd_arch_pdp11:
      /* TODO: arch_flags = M_PDP11; */
      *unknown = false;
      break;

    case bfd_arch_vax:
      *unknown = false;
      break;

    default:
      arch_flags = M_UNKNOWN;
    }

  if (arch_flags != M_UNKNOWN)
    *unknown = false;

  return arch_flags;
}

/* Set the architecture and the machine of the ABFD to the
   values ARCH and MACHINE.  Verify that @ABFD's format
   can support the architecture required.  */

bool
NAME (aout, set_arch_mach) (bfd *abfd,
			    enum bfd_architecture arch,
			    unsigned long machine)
{
  if (! bfd_default_set_arch_mach (abfd, arch, machine))
    return false;

  if (arch != bfd_arch_unknown)
    {
      bool unknown;

      NAME (aout, machine_type) (arch, machine, &unknown);
      if (unknown)
	return false;
    }

  obj_reloc_entry_size (abfd) = RELOC_SIZE;

  return (*aout_backend_info(abfd)->set_sizes) (abfd);
}

static void
adjust_o_magic (bfd *abfd, struct internal_exec *execp)
{
  file_ptr pos = adata (abfd).exec_bytes_size;
  bfd_vma vma = 0;
  int pad = 0;
  asection *text = obj_textsec (abfd);
  asection *data = obj_datasec (abfd);
  asection *bss = obj_bsssec (abfd);

  /* Text.  */
  text->filepos = pos;
  if (!text->user_set_vma)
    text->vma = vma;
  else
    vma = text->vma;

  pos += execp->a_text;
  vma += execp->a_text;

  /* Data.  */
  if (!data->user_set_vma)
    {
      pos += pad;
      vma += pad;
      data->vma = vma;
    }
  else
    vma = data->vma;
  execp->a_text += pad;

  data->filepos = pos;
  pos += data->size;
  vma += data->size;

  /* BSS.  */
  if (!bss->user_set_vma)
    {
      pos += pad;
      vma += pad;
      bss->vma = vma;
    }
  else if (data->size > 0 || bss->size > 0) /* PR25677: for objcopy --extract-symbol */
    {
      /* The VMA of the .bss section is set by the VMA of the
	 .data section plus the size of the .data section.  We may
	 need to add padding bytes to make this true.  */
      pad = bss->vma - vma;
      if (pad < 0)
	pad = 0;
      pos += pad;
    }
  execp->a_data = data->size + pad;
  bss->filepos = pos;
  execp->a_bss = bss->size;

  N_SET_MAGIC (execp, OMAGIC);
}

static void
adjust_z_magic (bfd *abfd, struct internal_exec *execp)
{
  bfd_size_type data_pad, text_pad;
  file_ptr text_end;
  const struct aout_backend_data *abdp;
  /* TRUE if text includes exec header.  */
  bool ztih;
  asection *text = obj_textsec (abfd);
  asection *data = obj_datasec (abfd);
  asection *bss = obj_bsssec (abfd);

  abdp = aout_backend_info (abfd);

  /* Text.  */
  ztih = (abdp != NULL
	  && (abdp->text_includes_header
	      || obj_aout_subformat (abfd) == q_magic_format));
  text->filepos = (ztih
		   ? adata (abfd).exec_bytes_size
		   : adata (abfd).zmagic_disk_block_size);
  if (!text->user_set_vma)
    {
      /* ?? Do we really need to check for relocs here?  */
      text->vma = ((abfd->flags & HAS_RELOC)
		   ? 0
		   : (ztih
		      ? abdp->default_text_vma + adata (abfd).exec_bytes_size
		      : abdp->default_text_vma));
      text_pad = 0;
    }
  else
    {
      /* The .text section is being loaded at an unusual address.  We
	 may need to pad it such that the .data section starts at a page
	 boundary.  */
      if (ztih)
	text_pad = ((text->filepos - text->vma)
		    & (adata (abfd).page_size - 1));
      else
	text_pad = (-text->vma
		    & (adata (abfd).page_size - 1));
    }

  /* Find start of data.  */
  if (ztih)
    {
      text_end = text->filepos + execp->a_text;
      text_pad += BFD_ALIGN (text_end, adata (abfd).page_size) - text_end;
    }
  else
    {
      /* Note that if page_size == zmagic_disk_block_size, then
	 filepos == page_size, and this case is the same as the ztih
	 case.  */
      text_end = execp->a_text;
      text_pad += BFD_ALIGN (text_end, adata (abfd).page_size) - text_end;
      text_end += text->filepos;
    }
  execp->a_text += text_pad;

  /* Data.  */
  if (!data->user_set_vma)
    {
      bfd_vma vma;
      vma = text->vma + execp->a_text;
      data->vma = BFD_ALIGN (vma, adata (abfd).segment_size);
    }
  if (abdp && abdp->zmagic_mapped_contiguous)
    {
      text_pad = data->vma - (text->vma + execp->a_text);
      /* Only pad the text section if the data
	 section is going to be placed after it.  */
      if (text_pad > 0)
	execp->a_text += text_pad;
    }
  data->filepos = text->filepos + execp->a_text;

  /* Fix up exec header while we're at it.  */
  if (ztih && (!abdp || (abdp && !abdp->exec_header_not_counted)))
    execp->a_text += adata (abfd).exec_bytes_size;
  N_SET_MAGIC (execp, ZMAGIC);

  /* Spec says data section should be rounded up to page boundary.  */
  execp->a_data = align_power (data->size, bss->alignment_power);
  execp->a_data = BFD_ALIGN (execp->a_data, adata (abfd).page_size);
  data_pad = execp->a_data - data->size;

  /* BSS.  */
  if (!bss->user_set_vma)
    bss->vma = data->vma + execp->a_data;
  /* If the BSS immediately follows the data section and extra space
     in the page is left after the data section, fudge data
     in the header so that the bss section looks smaller by that
     amount.  We'll start the bss section there, and lie to the OS.
     (Note that a linker script, as well as the above assignment,
     could have explicitly set the BSS vma to immediately follow
     the data section.)  */
  if (align_power (bss->vma, bss->alignment_power) == data->vma + execp->a_data)
    execp->a_bss = data_pad > bss->size ? 0 : bss->size - data_pad;
  else
    execp->a_bss = bss->size;
}

static void
adjust_n_magic (bfd *abfd, struct internal_exec *execp)
{
  file_ptr pos = adata (abfd).exec_bytes_size;
  bfd_vma vma = 0;
  int pad;
  asection *text = obj_textsec (abfd);
  asection *data = obj_datasec (abfd);
  asection *bss = obj_bsssec (abfd);

  /* Text.  */
  text->filepos = pos;
  if (!text->user_set_vma)
    text->vma = vma;
  else
    vma = text->vma;
  pos += execp->a_text;
  vma += execp->a_text;

  /* Data.  */
  data->filepos = pos;
  if (!data->user_set_vma)
    data->vma = BFD_ALIGN (vma, adata (abfd).segment_size);
  vma = data->vma;

  /* Since BSS follows data immediately, see if it needs alignment.  */
  vma += data->size;
  pad = align_power (vma, bss->alignment_power) - vma;
  execp->a_data = data->size + pad;
  pos += execp->a_data;

  /* BSS.  */
  if (!bss->user_set_vma)
    bss->vma = vma;
  else
    vma = bss->vma;

  /* Fix up exec header.  */
  execp->a_bss = bss->size;
  N_SET_MAGIC (execp, NMAGIC);
}

static void
adjust_i_magic (bfd *abfd, struct internal_exec *execp)
{
  file_ptr pos = adata (abfd).exec_bytes_size;
  bfd_vma vma = 0;
  int pad;
  asection *text = obj_textsec (abfd);
  asection *data = obj_datasec (abfd);
  asection *bss = obj_bsssec (abfd);

  /* Text.  */
  text->filepos = pos;
  if (!text->user_set_vma)
    text->vma = vma;
  else
    vma = text->vma;
  pos += execp->a_text;

  /* Data.  */
  data->filepos = pos;
  if (!data->user_set_vma)
    data->vma = 0;
  vma = data->vma;

  /* Since BSS follows data immediately, see if it needs alignment.  */
  vma += data->size;
  pad = align_power (vma, bss->alignment_power) - vma;
  execp->a_data = data->size + pad;
  pos += execp->a_data;

  /* BSS.  */
  if (!bss->user_set_vma)
    bss->vma = vma;
  else
    vma = bss->vma;

  /* Fix up exec header.  */
  execp->a_bss = bss->size;
  N_SET_MAGIC (execp, IMAGIC);
}

bool
NAME (aout, adjust_sizes_and_vmas) (bfd *abfd)
{
  struct internal_exec *execp = exec_hdr (abfd);

  if (! NAME (aout, make_sections) (abfd))
    return false;

  if (adata (abfd).magic != undecided_magic)
    return true;

  execp->a_text = align_power (obj_textsec (abfd)->size,
			       obj_textsec (abfd)->alignment_power);

  /* Rule (heuristic) for when to pad to a new page.  Note that there
     are (at least) two ways demand-paged (ZMAGIC) files have been
     handled.  Most Berkeley-based systems start the text segment at
     (TARGET_PAGE_SIZE).  However, newer versions of SUNOS start the text
     segment right after the exec header; the latter is counted in the
     text segment size, and is paged in by the kernel with the rest of
     the text.  */

  /* This perhaps isn't the right way to do this, but made it simpler for me
     to understand enough to implement it.  Better would probably be to go
     right from BFD flags to alignment/positioning characteristics.  But the
     old code was sloppy enough about handling the flags, and had enough
     other magic, that it was a little hard for me to understand.  I think
     I understand it better now, but I haven't time to do the cleanup this
     minute.  */

  if (separate_i_d)
    adata (abfd).magic = i_magic;
  else if (abfd->flags & WP_TEXT)
    adata (abfd).magic = n_magic;
  else
    adata (abfd).magic = o_magic;

#ifdef BFD_AOUT_DEBUG /* requires gcc2 */
#if __GNUC__ >= 2
  fprintf (stderr, "%s text=<%x,%x,%x> data=<%x,%x,%x> bss=<%x,%x,%x>\n",
	   ({ char *str;
	      switch (adata (abfd).magic)
		{
		case n_magic: str = "NMAGIC"; break;
		case o_magic: str = "OMAGIC"; break;
		case i_magic: str = "IMAGIC"; break;
		case z_magic: str = "ZMAGIC"; break;
		default: abort ();
		}
	      str;
	    }),
	   obj_textsec (abfd)->vma, obj_textsec (abfd)->size,
		obj_textsec (abfd)->alignment_power,
	   obj_datasec (abfd)->vma, obj_datasec (abfd)->size,
		obj_datasec (abfd)->alignment_power,
	   obj_bsssec (abfd)->vma, obj_bsssec (abfd)->size,
		obj_bsssec (abfd)->alignment_power);
#endif
#endif

  switch (adata (abfd).magic)
    {
    case o_magic:
      adjust_o_magic (abfd, execp);
      break;
    case z_magic:
      adjust_z_magic (abfd, execp);
      break;
    case n_magic:
      adjust_n_magic (abfd, execp);
      break;
    case i_magic:
      adjust_i_magic (abfd, execp);
      break;
    default:
      abort ();
    }

#ifdef BFD_AOUT_DEBUG
  fprintf (stderr, "       text=<%x,%x,%x> data=<%x,%x,%x> bss=<%x,%x>\n",
	   obj_textsec (abfd)->vma, execp->a_text,
		obj_textsec (abfd)->filepos,
	   obj_datasec (abfd)->vma, execp->a_data,
		obj_datasec (abfd)->filepos,
	   obj_bsssec (abfd)->vma, execp->a_bss);
#endif

  return true;
}

/* Called by the BFD in response to a bfd_make_section request.  */

bool
NAME (aout, new_section_hook) (bfd *abfd, asection *newsect)
{
  /* Align to double at least.  */
  newsect->alignment_power = bfd_get_arch_info(abfd)->section_align_power;

  if (bfd_get_format (abfd) == bfd_object)
    {
      if (obj_textsec (abfd) == NULL
	  && !strcmp (newsect->name, ".text"))
	{
	  obj_textsec(abfd)= newsect;
	  newsect->target_index = N_TEXT;
	}
      else if (obj_datasec (abfd) == NULL
	       && !strcmp (newsect->name, ".data"))
	{
	  obj_datasec (abfd) = newsect;
	  newsect->target_index = N_DATA;
	}
      else if (obj_bsssec (abfd) == NULL
	       && !strcmp (newsect->name, ".bss"))
	{
	  obj_bsssec (abfd) = newsect;
	  newsect->target_index = N_BSS;
	}
    }

  /* We allow more than three sections internally.  */
  return _bfd_generic_new_section_hook (abfd, newsect);
}

bool
NAME (aout, set_section_contents) (bfd *abfd,
				   sec_ptr section,
				   const void * location,
				   file_ptr offset,
				   bfd_size_type count)
{
  if (! abfd->output_has_begun)
    {
      if (! NAME (aout, adjust_sizes_and_vmas) (abfd))
	return false;
    }

  if (section == obj_bsssec (abfd))
    {
      bfd_set_error (bfd_error_no_contents);
      return false;
    }

  if (section != obj_textsec (abfd)
      && section != obj_datasec (abfd))
    {
      _bfd_error_handler
	/* xgettext:c-format */
	(_("%pB: can not represent section `%pA' in a.out object file format"),
	 abfd, section);
      bfd_set_error (bfd_error_nonrepresentable_section);
      return false;
    }

  if (count != 0)
    {
      if (bfd_seek (abfd, section->filepos + offset, SEEK_SET) != 0
	  || bfd_bwrite (location, count, abfd) != count)
	return false;
    }

  return true;
}

/* Read the external symbols from an a.out file.  */

static bool
aout_get_external_symbols (bfd *abfd)
{
  if (obj_aout_external_syms (abfd) == NULL)
    {
      bfd_size_type count;
      struct external_nlist *syms;

      count = exec_hdr (abfd)->a_syms / EXTERNAL_NLIST_SIZE;

      /* PR 17512: file: 011f5a08.  */
      if (count == 0)
	{
	  obj_aout_external_syms (abfd) = NULL;
	  obj_aout_external_sym_count (abfd) = count;
	  return true;
	}

#ifdef USE_MMAP
      if (! bfd_get_file_window (abfd, obj_sym_filepos (abfd),
				 exec_hdr (abfd)->a_syms,
				 &obj_aout_sym_window (abfd), true))
	return false;
      syms = (struct external_nlist *) obj_aout_sym_window (abfd).data;
#else
      /* We allocate using malloc to make the values easy to free
	 later on.  If we put them on the objalloc it might not be
	 possible to free them.  */
      if (bfd_seek (abfd, obj_sym_filepos (abfd), SEEK_SET) != 0)
	return false;
      syms = (struct external_nlist *)
	_bfd_malloc_and_read (abfd, count * EXTERNAL_NLIST_SIZE,
			      count * EXTERNAL_NLIST_SIZE);
      if (syms == NULL)
	return false;
#endif

      obj_aout_external_syms (abfd) = syms;
      obj_aout_external_sym_count (abfd) = count;
    }

  if (obj_aout_external_strings (abfd) == NULL
      && exec_hdr (abfd)->a_syms != 0)
    {
      unsigned char string_chars[BYTES_IN_LONG];
      bfd_size_type stringsize;
      char *strings;
      bfd_size_type amt = BYTES_IN_LONG;

      /* Get the size of the strings.  */
      if (bfd_seek (abfd, obj_str_filepos (abfd), SEEK_SET) != 0
	  || bfd_bread ((void *) string_chars, amt, abfd) != amt)
	return false;
      stringsize = H_GET_32 (abfd, string_chars);
      if (stringsize == 0)
	stringsize = 1;
      else if (stringsize < BYTES_IN_LONG
	       || (size_t) stringsize != stringsize)
	{
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}

#ifdef USE_MMAP
      if (stringsize >= BYTES_IN_LONG)
	{
	  if (! bfd_get_file_window (abfd, obj_str_filepos (abfd), stringsize + 1,
				     &obj_aout_string_window (abfd), true))
	    return false;
	  strings = (char *) obj_aout_string_window (abfd).data;
	}
      else
#endif
	{
	  strings = (char *) bfd_malloc (stringsize + 1);
	  if (strings == NULL)
	    return false;

	  if (stringsize >= BYTES_IN_LONG)
	    {
	      amt = stringsize - BYTES_IN_LONG;
	      if (bfd_bread (strings + BYTES_IN_LONG, amt, abfd) != amt)
		{
		  free (strings);
		  return false;
		}
	    }
	}
      /* Ensure that a zero index yields an empty string.  */
      if (stringsize >= BYTES_IN_WORD)
	memset (strings, 0, BYTES_IN_LONG);

      /* Ensure that the string buffer is NUL terminated.  */
      strings[stringsize] = 0;

      obj_aout_external_strings (abfd) = strings;
      obj_aout_external_string_size (abfd) = stringsize;
    }

  return true;
}

/* Translate an a.out symbol into a BFD symbol.  The desc, other, type
   and symbol->value fields of CACHE_PTR will be set from the a.out
   nlist structure.  This function is responsible for setting
   symbol->flags and symbol->section, and adjusting symbol->value.  */

static bool
translate_from_native_sym_flags (bfd *abfd,
				 aout_symbol_type *cache_ptr)
{
  flagword visible;

  if (is_stab (cache_ptr->type, cache_ptr->symbol.name))
    {
      asection *sec;

      /* This is a debugging symbol.  */
      cache_ptr->symbol.flags = BSF_DEBUGGING;

      /* Work out the symbol section.  */
      switch (cache_ptr->type)
	{
	case N_SO:
	case N_SOL:
	case N_FUN:
	case N_ENTRY:
	case N_SLINE:
	case N_FN:
	  sec = obj_textsec (abfd);
	  break;
	case N_STSYM:
	case N_DSLINE:
	  sec = obj_datasec (abfd);
	  break;
	case N_LCSYM:
	case N_BSLINE:
	  sec = obj_bsssec (abfd);
	  break;
	default:
	  sec = bfd_abs_section_ptr;
	  break;
	}

      cache_ptr->symbol.section = sec;
      cache_ptr->symbol.value -= sec->vma;

      return true;
    }

  /* Get the default visibility.  This does not apply to all types, so
     we just hold it in a local variable to use if wanted.  */
  if ((cache_ptr->type & N_EXT) == 0)
    visible = BSF_LOCAL;
  else
    visible = BSF_GLOBAL;

  switch (cache_ptr->type)
    {
    default:
    case N_ABS: case N_ABS | N_EXT:
      cache_ptr->symbol.section = bfd_abs_section_ptr;
      cache_ptr->symbol.flags = visible;
      break;

    case N_UNDF | N_EXT:
      if (cache_ptr->symbol.value != 0)
	{
	  /* This is a common symbol.  */
	  cache_ptr->symbol.flags = BSF_GLOBAL;
	  cache_ptr->symbol.section = bfd_com_section_ptr;
	}
      else
	{
	  cache_ptr->symbol.flags = 0;
	  cache_ptr->symbol.section = bfd_und_section_ptr;
	}
      break;

    case N_TEXT: case N_TEXT | N_EXT:
      cache_ptr->symbol.section = obj_textsec (abfd);
      cache_ptr->symbol.value -= cache_ptr->symbol.section->vma;
      cache_ptr->symbol.flags = visible;
      break;

    case N_DATA: case N_DATA | N_EXT:
      cache_ptr->symbol.section = obj_datasec (abfd);
      cache_ptr->symbol.value -= cache_ptr->symbol.section->vma;
      cache_ptr->symbol.flags = visible;
      break;

    case N_BSS: case N_BSS | N_EXT:
      cache_ptr->symbol.section = obj_bsssec (abfd);
      cache_ptr->symbol.value -= cache_ptr->symbol.section->vma;
      cache_ptr->symbol.flags = visible;
      break;
    }

  return true;
}

/* Set the fields of SYM_POINTER according to CACHE_PTR.  */

static bool
translate_to_native_sym_flags (bfd *abfd,
			       asymbol *cache_ptr,
			       struct external_nlist *sym_pointer)
{
  bfd_vma value = cache_ptr->value;
  asection *sec;
  bfd_vma off;
  const char *name = cache_ptr->name != NULL ? cache_ptr->name : "*unknown*";

  /* Mask out any existing type bits in case copying from one section
     to another.  */
  if (!is_stab (sym_pointer->e_type[0], name))
    sym_pointer->e_type[0] &= ~N_TYPE;

  sec = bfd_asymbol_section (cache_ptr);
  off = 0;

  if (sec == NULL)
    {
      /* This case occurs, e.g., for the *DEBUG* section of a COFF
	 file.  */
      _bfd_error_handler
	/* xgettext:c-format */
	(_("%pB: can not represent section for symbol `%s' in a.out object file format"),
	 abfd, name);
      bfd_set_error (bfd_error_nonrepresentable_section);
      return false;
    }

  if (sec->output_section != NULL)
    {
      off = sec->output_offset;
      sec = sec->output_section;
    }

  if (bfd_is_abs_section (sec))
    sym_pointer->e_type[0] |= N_ABS;
  else if (sec == obj_textsec (abfd))
    sym_pointer->e_type[0] |= N_TEXT;
  else if (sec == obj_datasec (abfd))
    sym_pointer->e_type[0] |= N_DATA;
  else if (sec == obj_bsssec (abfd))
    sym_pointer->e_type[0] |= N_BSS;
  else if (bfd_is_und_section (sec))
    sym_pointer->e_type[0] = N_UNDF | N_EXT;
  else if (bfd_is_com_section (sec))
    sym_pointer->e_type[0] = N_UNDF | N_EXT;
  else
    {
      _bfd_error_handler
	/* xgettext:c-format */
	(_("%pB: can not represent section `%pA' in a.out object file format"),
	 abfd, sec);
      bfd_set_error (bfd_error_nonrepresentable_section);
      return false;
    }

  /* Turn the symbol from section relative to absolute again */
  value += sec->vma + off;

  if ((cache_ptr->flags & BSF_DEBUGGING) != 0)
    sym_pointer->e_type[0] = ((aout_symbol_type *) cache_ptr)->type;
  else if ((cache_ptr->flags & BSF_GLOBAL) != 0)
    sym_pointer->e_type[0] |= N_EXT;

  PUT_WORD(abfd, value, sym_pointer->e_value);

  return true;
}

/* Native-level interface to symbols. */

asymbol *
NAME (aout, make_empty_symbol) (bfd *abfd)
{
  size_t amt = sizeof (aout_symbol_type);
  aout_symbol_type *new_symbol_type = bfd_zalloc (abfd, amt);

  if (!new_symbol_type)
    return NULL;
  new_symbol_type->symbol.the_bfd = abfd;

  return &new_symbol_type->symbol;
}

/* Translate a set of external symbols into internal symbols.  */

bool
NAME (aout, translate_symbol_table) (bfd *abfd,
				     aout_symbol_type *in,
				     struct external_nlist *ext,
				     bfd_size_type count,
				     char *str,
				     bfd_size_type strsize,
				     bool dynamic)
{
  struct external_nlist *ext_end;

  ext_end = ext + count;
  for (; ext < ext_end; ext++, in++)
    {
      bfd_vma x;
      int ovly;

      x = GET_WORD (abfd, ext->e_strx);
      in->symbol.the_bfd = abfd;

      /* For the normal symbols, the zero index points at the number
	 of bytes in the string table but is to be interpreted as the
	 null string.  For the dynamic symbols, the number of bytes in
	 the string table is stored in the __DYNAMIC structure and the
	 zero index points at an actual string.  */
      if (x == 0 && ! dynamic)
	in->symbol.name = "";
      else if (x < strsize)
	in->symbol.name = str + x;
      else
	{
	  _bfd_error_handler
	    (_("%pB: invalid string offset %" PRIu64 " >= %" PRIu64),
	     abfd, (uint64_t) x, (uint64_t) strsize);
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}

      ovly = H_GET_8 (abfd, ext->e_ovly);
      if (ovly != 0)
	{
	  _bfd_error_handler
	    (_("%pB: symbol indicates overlay (not supported)"), abfd);
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}

      in->symbol.value = GET_WORD (abfd,  ext->e_value);
      /* e_desc is zero for normal symbols but for .stab symbols it
	 carries the desc field in our extended 2.11BSD format. */
      in->desc = H_GET_16 (abfd, ext->e_desc);
      in->other = 0;
      in->type = H_GET_8 (abfd,  ext->e_type);
      in->symbol.udata.p = NULL;

      if (! translate_from_native_sym_flags (abfd, in))
	return false;

      if (dynamic)
	in->symbol.flags |= BSF_DYNAMIC;
    }

  return true;
}

/* We read the symbols into a buffer, which is discarded when this
   function exits.  We read the strings into a buffer large enough to
   hold them all plus all the cached symbol entries.  */

bool
NAME (aout, slurp_symbol_table) (bfd *abfd)
{
  struct external_nlist *old_external_syms;
  aout_symbol_type *cached;
  bfd_size_type cached_size;

  /* If there's no work to be done, don't do any.  */
  if (obj_aout_symbols (abfd) != NULL)
    return true;

  old_external_syms = obj_aout_external_syms (abfd);

  if (! aout_get_external_symbols (abfd))
    return false;

  cached_size = obj_aout_external_sym_count (abfd);
  cached_size *= sizeof (aout_symbol_type);
  cached = bfd_zmalloc (cached_size);
  if (cached == NULL && cached_size != 0)
    return false;

  /* Convert from external symbol information to internal.  */
  if (! (NAME (aout, translate_symbol_table)
	 (abfd, cached,
	  obj_aout_external_syms (abfd),
	  obj_aout_external_sym_count (abfd),
	  obj_aout_external_strings (abfd),
	  obj_aout_external_string_size (abfd),
	  false)))
    {
      free (cached);
      return false;
    }

  abfd->symcount = obj_aout_external_sym_count (abfd);

  obj_aout_symbols (abfd) = cached;

  /* It is very likely that anybody who calls this function will not
     want the external symbol information, so if it was allocated
     because of our call to aout_get_external_symbols, we free it up
     right away to save space.  */
  if (old_external_syms == NULL
      && obj_aout_external_syms (abfd) != NULL)
    {
#ifdef USE_MMAP
      bfd_free_window (&obj_aout_sym_window (abfd));
#else
      free (obj_aout_external_syms (abfd));
#endif
      obj_aout_external_syms (abfd) = NULL;
    }

  return true;
}

/* We use a hash table when writing out symbols so that we only write
   out a particular string once.  This helps particularly when the
   linker writes out stabs debugging entries, because each different
   contributing object file tends to have many duplicate stabs
   strings.

   This hash table code breaks dbx on SunOS 4.1.3, so we don't do it
   if BFD_TRADITIONAL_FORMAT is set.  */

/* Get the index of a string in a strtab, adding it if it is not
   already present.  */

static inline bfd_size_type
add_to_stringtab (bfd *abfd,
		  struct bfd_strtab_hash *tab,
		  const char *str,
		  bool copy)
{
  bool hash;
  bfd_size_type str_index;

  /* An index of 0 always means the empty string.  */
  if (str == 0 || *str == '\0')
    return 0;

  /* Don't hash if BFD_TRADITIONAL_FORMAT is set, because SunOS dbx
     doesn't understand a hashed string table.  */
  hash = true;
  if ((abfd->flags & BFD_TRADITIONAL_FORMAT) != 0)
    hash = false;

  str_index = _bfd_stringtab_add (tab, str, hash, copy);

  if (str_index != (bfd_size_type) -1)
    /* Add BYTES_IN_LONG to the return value to account for the
       space taken up by the string table size.  */
    str_index += BYTES_IN_LONG;

  return str_index;
}

/* Write out a strtab.  ABFD is already at the right location in the
   file.  */

static bool
emit_stringtab (bfd *abfd, struct bfd_strtab_hash *tab)
{
  bfd_byte buffer[BYTES_IN_LONG];

  /* The string table starts with the size.  */
  H_PUT_32 (abfd, _bfd_stringtab_size (tab) + BYTES_IN_LONG, buffer);
  if (bfd_bwrite ((void *) buffer, (bfd_size_type) BYTES_IN_LONG, abfd)
      != BYTES_IN_LONG)
    return false;

  return _bfd_stringtab_emit (abfd, tab);
}

bool
NAME (aout, write_syms) (bfd *abfd)
{
  unsigned int count ;
  asymbol **generic = bfd_get_outsymbols (abfd);
  struct bfd_strtab_hash *strtab;

  strtab = _bfd_stringtab_init ();
  if (strtab == NULL)
    return false;

  for (count = 0; count < bfd_get_symcount (abfd); count++)
    {
      asymbol *g = generic[count];
      bfd_size_type indx;
      struct external_nlist nsp;

      indx = add_to_stringtab (abfd, strtab, g->name, false);
      if (indx == (bfd_size_type) -1)
	goto error_return;
      PUT_WORD (abfd, indx, nsp.e_strx);

      if (bfd_asymbol_flavour(g) == abfd->xvec->flavour)
	{
	  H_PUT_16 (abfd, aout_symbol (g)->desc,  nsp.e_desc);
	  H_PUT_8 (abfd, 0, nsp.e_ovly);
	  H_PUT_8 (abfd, aout_symbol (g)->type,  nsp.e_type);
	}
      else
	{
	  H_PUT_16 (abfd, 0, nsp.e_desc);
	  H_PUT_8 (abfd, 0, nsp.e_ovly);
	  H_PUT_8 (abfd, 0, nsp.e_type);
	}

      if (! translate_to_native_sym_flags (abfd, g, &nsp))
	goto error_return;

      if (bfd_bwrite ((void *)&nsp, (bfd_size_type) EXTERNAL_NLIST_SIZE, abfd)
	  != EXTERNAL_NLIST_SIZE)
	goto error_return;

      /* NB: `KEEPIT' currently overlays `udata.p', so set this only
	 here, at the end.  */
      g->KEEPIT = count;
    }

  if (! emit_stringtab (abfd, strtab))
    goto error_return;

  _bfd_stringtab_free (strtab);

  return true;

 error_return:
  _bfd_stringtab_free (strtab);
  return false;
}


long
NAME (aout, canonicalize_symtab) (bfd *abfd, asymbol **location)
{
  unsigned int counter = 0;
  aout_symbol_type *symbase;

  if (!NAME (aout, slurp_symbol_table) (abfd))
    return -1;

  for (symbase = obj_aout_symbols (abfd); counter++ < bfd_get_symcount (abfd);)
    *(location++) = (asymbol *)(symbase++);
  *location++ =0;
  return bfd_get_symcount (abfd);
}


/* Output extended relocation information to a file in target byte order.  */

static void
pdp11_aout_swap_reloc_out (bfd *abfd, arelent *g, bfd_byte *natptr)
{
  int r_index;
  int r_pcrel;
  int reloc_entry;
  int r_type;
  asymbol *sym = *(g->sym_ptr_ptr);
  asection *output_section = sym->section->output_section;

  if (g->addend != 0)
    fprintf (stderr, "BFD: can't do this reloc addend stuff\n");

  r_pcrel = g->howto->pc_relative;

  if (bfd_is_abs_section (output_section))
    r_type = RABS;
  else if (output_section == obj_textsec (abfd))
    r_type = RTEXT;
  else if (output_section == obj_datasec (abfd))
    r_type = RDATA;
  else if (output_section == obj_bsssec (abfd))
    r_type = RBSS;
  else if (bfd_is_und_section (output_section))
    r_type = REXT;
  else if (bfd_is_com_section (output_section))
    r_type = REXT;
  else
    r_type = -1;

  BFD_ASSERT (r_type != -1);

  if (r_type == RABS)
    r_index = 0;
  else
    r_index = (*(g->sym_ptr_ptr))->KEEPIT;

  reloc_entry = r_index << 4 | r_type | r_pcrel;

  PUT_WORD (abfd, reloc_entry, natptr);
}

/* BFD deals internally with all things based from the section they're
   in. so, something in 10 bytes into a text section  with a base of
   50 would have a symbol (.text+10) and know .text vma was 50.

   Aout keeps all it's symbols based from zero, so the symbol would
   contain 60. This macro subs the base of each section from the value
   to give the true offset from the section */


#define MOVE_ADDRESS(ad)						\
  if (r_extern)								\
    {									\
      /* Undefined symbol.  */						\
      if (symbols != NULL && r_index < bfd_get_symcount (abfd))		\
	cache_ptr->sym_ptr_ptr = symbols + r_index;			\
      else								\
	cache_ptr->sym_ptr_ptr = bfd_abs_section_ptr->symbol_ptr_ptr;	\
      cache_ptr->addend = ad;						\
    }									\
  else									\
    {									\
      /* Defined, section relative. replace symbol with pointer to	\
	 symbol which points to section.  */				\
      switch (r_index)							\
	{								\
	case N_TEXT:							\
	case N_TEXT | N_EXT:						\
	  cache_ptr->sym_ptr_ptr  = obj_textsec (abfd)->symbol_ptr_ptr;	\
	  cache_ptr->addend = ad  - su->textsec->vma;			\
	  break;							\
	case N_DATA:							\
	case N_DATA | N_EXT:						\
	  cache_ptr->sym_ptr_ptr  = obj_datasec (abfd)->symbol_ptr_ptr;	\
	  cache_ptr->addend = ad - su->datasec->vma;			\
	  break;							\
	case N_BSS:							\
	case N_BSS | N_EXT:						\
	  cache_ptr->sym_ptr_ptr  = obj_bsssec (abfd)->symbol_ptr_ptr;	\
	  cache_ptr->addend = ad - su->bsssec->vma;			\
	  break;							\
	default:							\
	case N_ABS:							\
	case N_ABS | N_EXT:						\
	  cache_ptr->sym_ptr_ptr = bfd_abs_section_ptr->symbol_ptr_ptr;	\
	  cache_ptr->addend = ad;					\
	  break;							\
	}								\
    }

static void
pdp11_aout_swap_reloc_in (bfd *		 abfd,
			  bfd_byte *	 bytes,
			  arelent *	 cache_ptr,
			  bfd_size_type	 offset,
			  asymbol **	 symbols,
			  bfd_size_type	 symcount)
{
  struct aoutdata *su = &(abfd->tdata.aout_data->a);
  unsigned int r_index;
  int reloc_entry;
  int r_extern;
  int r_pcrel;

  reloc_entry = GET_WORD (abfd, (void *) bytes);

  r_pcrel = reloc_entry & RELFLG;

  cache_ptr->address = offset;
  cache_ptr->howto = howto_table_pdp11 + (r_pcrel ? 1 : 0);

  if ((reloc_entry & RTYPE) == RABS)
    r_index = N_ABS;
  else
    r_index = RINDEX (reloc_entry);

  /* r_extern reflects whether the symbol the reloc is against is
     local or global.  */
  r_extern = (reloc_entry & RTYPE) == REXT;

  if (r_extern && r_index >= symcount)
    {
      /* We could arrange to return an error, but it might be useful
	 to see the file even if it is bad.  FIXME: Of course this
	 means that objdump -r *doesn't* see the actual reloc, and
	 objcopy silently writes a different reloc.  */
      r_extern = 0;
      r_index = N_ABS;
    }

  MOVE_ADDRESS(0);
}

/* Read and swap the relocs for a section.  */

bool
NAME (aout, slurp_reloc_table) (bfd *abfd, sec_ptr asect, asymbol **symbols)
{
  bfd_byte *rptr;
  bfd_size_type count;
  bfd_size_type reloc_size;
  void * relocs;
  arelent *reloc_cache;
  size_t each_size;
  unsigned int counter = 0;
  arelent *cache_ptr;

  if (asect->relocation)
    return true;

  if (asect->flags & SEC_CONSTRUCTOR)
    return true;

  if (asect == obj_datasec (abfd))
    reloc_size = exec_hdr(abfd)->a_drsize;
  else if (asect == obj_textsec (abfd))
    reloc_size = exec_hdr(abfd)->a_trsize;
  else if (asect == obj_bsssec (abfd))
    reloc_size = 0;
  else
    {
      bfd_set_error (bfd_error_invalid_operation);
      return false;
    }

  if (bfd_seek (abfd, asect->rel_filepos, SEEK_SET) != 0)
    return false;
  relocs = _bfd_malloc_and_read (abfd, reloc_size, reloc_size);
  if (relocs == NULL && reloc_size != 0)
    return false;

  each_size = obj_reloc_entry_size (abfd);
  count = reloc_size / each_size;

  /* Count the number of NON-ZERO relocs, this is the count we want.  */
  {
    unsigned int real_count = 0;

    for (counter = 0; counter < count; counter++)
      {
	int x;

	x = GET_WORD (abfd, (char *) relocs + each_size * counter);
	if (x != 0)
	  real_count++;
      }

    count = real_count;
  }

  reloc_cache = bfd_zmalloc (count * sizeof (arelent));
  if (reloc_cache == NULL && count != 0)
    return false;

  cache_ptr = reloc_cache;

  rptr = relocs;
  for (counter = 0;
       counter < count;
       counter++, rptr += RELOC_SIZE, cache_ptr++)
    {
      while (GET_WORD (abfd, (void *) rptr) == 0)
	{
	  rptr += RELOC_SIZE;
	  if ((char *) rptr >= (char *) relocs + reloc_size)
	    goto done;
	}

      pdp11_aout_swap_reloc_in (abfd, rptr, cache_ptr,
				(bfd_size_type) ((char *) rptr - (char *) relocs),
				symbols,
				(bfd_size_type) bfd_get_symcount (abfd));
    }
 done:
  /* Just in case, if rptr >= relocs + reloc_size should happen
     too early.  */
  BFD_ASSERT (counter == count);

  free (relocs);

  asect->relocation = reloc_cache;
  asect->reloc_count = cache_ptr - reloc_cache;

  return true;
}

/* Write out a relocation section into an object file.  */

bool
NAME (aout, squirt_out_relocs) (bfd *abfd, asection *section)
{
  arelent **generic;
  unsigned char *native;
  unsigned int count = section->reloc_count;
  bfd_size_type natsize;

  natsize = section->size;
  native = bfd_zalloc (abfd, natsize);
  if (!native)
    return false;

  generic = section->orelocation;
  if (generic != NULL)
    {
      while (count > 0)
	{
	  bfd_byte *r;

	  if ((*generic)->howto == NULL
	      || (*generic)->sym_ptr_ptr == NULL)
	    {
	      bfd_set_error (bfd_error_invalid_operation);
	      _bfd_error_handler (_("%pB: attempt to write out "
				    "unknown reloc type"), abfd);
	      bfd_release (abfd, native);
	      return false;
	    }
	  r = native + (*generic)->address;
	  pdp11_aout_swap_reloc_out (abfd, *generic, r);
	  count--;
	  generic++;
	}
    }

  if (bfd_bwrite ((void *) native, natsize, abfd) != natsize)
    {
      bfd_release (abfd, native);
      return false;
    }

  bfd_release (abfd, native);
  return true;
}

/* This is stupid.  This function should be a boolean predicate.  */

long
NAME (aout, canonicalize_reloc) (bfd *abfd,
				 sec_ptr section,
				 arelent **relptr,
				 asymbol **symbols)
{
  arelent *tblptr = section->relocation;
  unsigned int count;

  if (section == obj_bsssec (abfd))
    {
      *relptr = NULL;
      return 0;
    }

  if (!(tblptr || NAME (aout, slurp_reloc_table)(abfd, section, symbols)))
    return -1;

  if (section->flags & SEC_CONSTRUCTOR)
    {
      arelent_chain *chain = section->constructor_chain;

      for (count = 0; count < section->reloc_count; count ++)
	{
	  *relptr ++ = &chain->relent;
	  chain = chain->next;
	}
    }
  else
    {
      tblptr = section->relocation;

      for (count = 0; count++ < section->reloc_count;)
	*relptr++ = tblptr++;
    }

  *relptr = 0;

  return section->reloc_count;
}

long
NAME (aout, get_reloc_upper_bound) (bfd *abfd, sec_ptr asect)
{
  size_t count, raw;

  if (asect->flags & SEC_CONSTRUCTOR)
    count = asect->reloc_count;
  else if (asect == obj_datasec (abfd))
    count = exec_hdr (abfd)->a_drsize / obj_reloc_entry_size (abfd);
  else if (asect == obj_textsec (abfd))
    count = exec_hdr (abfd)->a_trsize / obj_reloc_entry_size (abfd);
  else if (asect == obj_bsssec (abfd))
    count = 0;
  else
    {
      bfd_set_error (bfd_error_invalid_operation);
      return -1;
    }

  if (count >= LONG_MAX / sizeof (arelent *)
      || _bfd_mul_overflow (count, obj_reloc_entry_size (abfd), &raw))
    {
      bfd_set_error (bfd_error_file_too_big);
      return -1;
    }
  if (!bfd_write_p (abfd))
    {
      ufile_ptr filesize = bfd_get_file_size (abfd);
      if (filesize != 0 && raw > filesize)
	{
	  bfd_set_error (bfd_error_file_truncated);
	  return -1;
	}
    }
  return (count + 1) * sizeof (arelent *);
}


long
NAME (aout, get_symtab_upper_bound) (bfd *abfd)
{
  if (!NAME (aout, slurp_symbol_table) (abfd))
    return -1;

  return (bfd_get_symcount (abfd) + 1) * (sizeof (aout_symbol_type *));
}

alent *
NAME (aout, get_lineno) (bfd * abfd ATTRIBUTE_UNUSED,
			 asymbol * symbol ATTRIBUTE_UNUSED)
{
  return NULL;
}

void
NAME (aout, get_symbol_info) (bfd * abfd ATTRIBUTE_UNUSED,
			      asymbol *symbol,
			      symbol_info *ret)
{
  bfd_symbol_info (symbol, ret);

  if (ret->type == '?')
    {
      int type_code = aout_symbol(symbol)->type & 0xff;
      const char *stab_name = bfd_get_stab_name (type_code);
      static char buf[10];

      if (stab_name == NULL)
	{
	  sprintf(buf, "(%d)", type_code);
	  stab_name = buf;
	}
      ret->type = '-';
      ret->stab_type  = type_code;
      ret->stab_other = (unsigned) (aout_symbol(symbol)->other & 0xff);
      ret->stab_desc  = (unsigned) (aout_symbol(symbol)->desc & 0xffff);
      ret->stab_name  = stab_name;
    }
}

void
NAME (aout, print_symbol) (bfd * abfd,
			   void * afile,
			   asymbol *symbol,
			   bfd_print_symbol_type how)
{
  FILE *file = (FILE *) afile;

  switch (how)
    {
    case bfd_print_symbol_name:
      if (symbol->name)
	fprintf(file,"%s", symbol->name);
      break;
    case bfd_print_symbol_more:
      fprintf(file,"%4x %2x %2x",
	      (unsigned) (aout_symbol (symbol)->desc & 0xffff),
	      (unsigned) (aout_symbol (symbol)->other & 0xff),
	      (unsigned) (aout_symbol (symbol)->type));
      break;
    case bfd_print_symbol_all:
      {
	const char *section_name = symbol->section->name;

	bfd_print_symbol_vandf (abfd, (void *) file, symbol);

	fprintf (file," %-5s %04x %02x %02x",
		 section_name,
		 (unsigned) (aout_symbol (symbol)->desc & 0xffff),
		 (unsigned) (aout_symbol (symbol)->other & 0xff),
		 (unsigned) (aout_symbol (symbol)->type  & 0xff));
	if (symbol->name)
	  fprintf(file," %s", symbol->name);
      }
      break;
    }
}

/* If we don't have to allocate more than 1MB to hold the generic
   symbols, we use the generic minisymbol method: it's faster, since
   it only translates the symbols once, not multiple times.  */
#define MINISYM_THRESHOLD (1000000 / sizeof (asymbol))

/* Read minisymbols.  For minisymbols, we use the unmodified a.out
   symbols.  The minisymbol_to_symbol function translates these into
   BFD asymbol structures.  */

long
NAME (aout, read_minisymbols) (bfd *abfd,
			       bool dynamic,
			       void * *minisymsp,
			       unsigned int *sizep)
{
  if (dynamic)
    /* We could handle the dynamic symbols here as well, but it's
       easier to hand them off.  */
    return _bfd_generic_read_minisymbols (abfd, dynamic, minisymsp, sizep);

  if (! aout_get_external_symbols (abfd))
    return -1;

  if (obj_aout_external_sym_count (abfd) < MINISYM_THRESHOLD)
    return _bfd_generic_read_minisymbols (abfd, dynamic, minisymsp, sizep);

  *minisymsp = (void *) obj_aout_external_syms (abfd);

  /* By passing the external symbols back from this routine, we are
     giving up control over the memory block.  Clear
     obj_aout_external_syms, so that we do not try to free it
     ourselves.  */
  obj_aout_external_syms (abfd) = NULL;

  *sizep = EXTERNAL_NLIST_SIZE;
  return obj_aout_external_sym_count (abfd);
}

/* Convert a minisymbol to a BFD asymbol.  A minisymbol is just an
   unmodified a.out symbol.  The SYM argument is a structure returned
   by bfd_make_empty_symbol, which we fill in here.  */

asymbol *
NAME (aout, minisymbol_to_symbol) (bfd *abfd,
				   bool dynamic,
				   const void * minisym,
				   asymbol *sym)
{
  if (dynamic
      || obj_aout_external_sym_count (abfd) < MINISYM_THRESHOLD)
    return _bfd_generic_minisymbol_to_symbol (abfd, dynamic, minisym, sym);

  memset (sym, 0, sizeof (aout_symbol_type));

  /* We call translate_symbol_table to translate a single symbol.  */
  if (! (NAME (aout, translate_symbol_table)
	 (abfd,
	  (aout_symbol_type *) sym,
	  (struct external_nlist *) minisym,
	  (bfd_size_type) 1,
	  obj_aout_external_strings (abfd),
	  obj_aout_external_string_size (abfd),
	  false)))
    return NULL;

  return sym;
}

/* Provided a BFD, a section and an offset into the section, calculate
   and return the name of the source file and the line nearest to the
   wanted location.  */

bool
NAME (aout, find_nearest_line) (bfd *abfd,
				asymbol **symbols,
				asection *section,
				bfd_vma offset,
				const char **filename_ptr,
				const char **functionname_ptr,
				unsigned int *line_ptr,
				unsigned int *discriminator_ptr)
{
  /* Run down the file looking for the filename, function and linenumber.  */
  asymbol **p;
  const char *directory_name = NULL;
  const char *main_file_name = NULL;
  const char *current_file_name = NULL;
  const char *line_file_name = NULL; /* Value of current_file_name at line number.  */
  bfd_vma low_line_vma = 0;
  bfd_vma low_func_vma = 0;
  asymbol *func = 0;
  size_t filelen, funclen;
  char *buf;

  *filename_ptr = bfd_get_filename (abfd);
  *functionname_ptr = NULL;
  *line_ptr = 0;
  if (discriminator_ptr)
    *discriminator_ptr = 0;

  if (symbols != NULL)
    {
      for (p = symbols; *p; p++)
	{
	  aout_symbol_type  *q = (aout_symbol_type *)(*p);
	next:
	  switch (q->type)
	    {
	    case N_TEXT:
	      /* If this looks like a file name symbol, and it comes after
		 the line number we have found so far, but before the
		 offset, then we have probably not found the right line
		 number.  */
	      if (q->symbol.value <= offset
		  && ((q->symbol.value > low_line_vma
		       && (line_file_name != NULL
			   || *line_ptr != 0))
		      || (q->symbol.value > low_func_vma
			  && func != NULL)))
		{
		  const char * symname;

		  symname = q->symbol.name;

		  if (symname != NULL
		      && strlen (symname) > 2
		      && strcmp (symname + strlen (symname) - 2, ".o") == 0)
		    {
		      if (q->symbol.value > low_line_vma)
			{
			  *line_ptr = 0;
			  line_file_name = NULL;
			}
		      if (q->symbol.value > low_func_vma)
			func = NULL;
		    }
		}
	      break;

	    case N_SO:
	      /* If this symbol is less than the offset, but greater than
		 the line number we have found so far, then we have not
		 found the right line number.  */
	      if (q->symbol.value <= offset)
		{
		  if (q->symbol.value > low_line_vma)
		    {
		      *line_ptr = 0;
		      line_file_name = NULL;
		    }
		  if (q->symbol.value > low_func_vma)
		    func = NULL;
		}

	      main_file_name = current_file_name = q->symbol.name;
	      /* Look ahead to next symbol to check if that too is an N_SO.  */
	      p++;
	      if (*p == NULL)
		goto done;
	      q = (aout_symbol_type *)(*p);
	      if (q->type != (int) N_SO)
		goto next;

	      /* Found a second N_SO  First is directory; second is filename.  */
	      directory_name = current_file_name;
	      main_file_name = current_file_name = q->symbol.name;
	      if (obj_textsec(abfd) != section)
		goto done;
	      break;
	    case N_SOL:
	      current_file_name = q->symbol.name;
	      break;

	    case N_SLINE:
	    case N_DSLINE:
	    case N_BSLINE:
	      /* We'll keep this if it resolves nearer than the one we have
		 already.  */
	      if (q->symbol.value >= low_line_vma
		  && q->symbol.value <= offset)
		{
		  *line_ptr = q->desc;
		  low_line_vma = q->symbol.value;
		  line_file_name = current_file_name;
		}
	      break;

	    case N_FUN:
	      {
		/* We'll keep this if it is nearer than the one we have already.  */
		if (q->symbol.value >= low_func_vma &&
		    q->symbol.value <= offset)
		  {
		    low_func_vma = q->symbol.value;
		    func = (asymbol *) q;
		  }
		else if (q->symbol.value > offset)
		  goto done;
	      }
	      break;
	    }
	}
    }

 done:
  if (*line_ptr != 0)
    main_file_name = line_file_name;

  if (main_file_name == NULL
      || main_file_name[0] == '/'
      || directory_name == NULL)
    filelen = 0;
  else
    filelen = strlen (directory_name) + strlen (main_file_name);
  if (func == NULL)
    funclen = 0;
  else
    funclen = strlen (bfd_asymbol_name (func));

  free (adata (abfd).line_buf);
  if (filelen + funclen == 0)
    adata (abfd).line_buf = buf = NULL;
  else
    {
      buf = bfd_malloc ((bfd_size_type) filelen + funclen + 3);
      adata (abfd).line_buf = buf;
      if (buf == NULL)
	return false;
    }

  if (main_file_name != NULL)
    {
      if (main_file_name[0] == '/' || directory_name == NULL)
	*filename_ptr = main_file_name;
      else
	{
	  if (buf == NULL)
	    /* PR binutils/20891: In a corrupt input file both
	       main_file_name and directory_name can be empty...  */
	    * filename_ptr = NULL;
	  else
	    {
	      snprintf (buf, filelen + 1, "%s%s", directory_name,
			main_file_name);
	      *filename_ptr = buf;
	      buf += filelen + 1;
	    }
	}
    }

  if (func)
    {
      const char *function = func->name;
      char *colon;

      if (buf == NULL)
	{
	  /* PR binutils/20892: In a corrupt input file func can be empty.  */
	  * functionname_ptr = NULL;
	  return true;
	}
      /* The caller expects a symbol name.  We actually have a
	 function name, without the leading underscore.  Put the
	 underscore back in, so that the caller gets a symbol name.  */
      if (bfd_get_symbol_leading_char (abfd) == '\0')
	strcpy (buf, function);
      else
	{
	  buf[0] = bfd_get_symbol_leading_char (abfd);
	  strcpy (buf + 1, function);
	}

      /* Have to remove : stuff.  */
      colon = strchr (buf, ':');
      if (colon != NULL)
	*colon = '\0';
      *functionname_ptr = buf;
    }

  return true;
}

int
NAME (aout, sizeof_headers) (bfd *abfd,
			     struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  return adata (abfd).exec_bytes_size;
}

/* Throw away most malloc'd and alloc'd information for this BFD.  */

bool
NAME (aout, bfd_free_cached_info) (bfd *abfd)
{
  if ((bfd_get_format (abfd) == bfd_object
       || bfd_get_format (abfd) == bfd_core)
      && abfd->tdata.aout_data != NULL)
    {
#define BFCI_FREE(x) do { free (x); x = NULL; } while (0)
      BFCI_FREE (adata (abfd).line_buf);
      BFCI_FREE (obj_aout_symbols (abfd));
#ifdef USE_MMAP
      obj_aout_external_syms (abfd) = 0;
      bfd_free_window (&obj_aout_sym_window (abfd));
      bfd_free_window (&obj_aout_string_window (abfd));
      obj_aout_external_strings (abfd) = 0;
#else
      BFCI_FREE (obj_aout_external_syms (abfd));
      BFCI_FREE (obj_aout_external_strings (abfd));
#endif
      for (asection *o = abfd->sections; o != NULL; o = o->next)
	BFCI_FREE (o->relocation);
#undef BFCI_FREE
    }

  return _bfd_generic_bfd_free_cached_info (abfd);
}

/* Routine to create an entry in an a.out link hash table.  */

struct bfd_hash_entry *
NAME (aout, link_hash_newfunc) (struct bfd_hash_entry *entry,
				struct bfd_hash_table *table,
				const char *string)
{
  struct aout_link_hash_entry *ret = (struct aout_link_hash_entry *) entry;

  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (ret == NULL)
    ret = bfd_hash_allocate (table, sizeof (* ret));
  if (ret == NULL)
    return NULL;

  /* Call the allocation method of the superclass.  */
  ret = (struct aout_link_hash_entry *)
	 _bfd_link_hash_newfunc ((struct bfd_hash_entry *) ret, table, string);
  if (ret)
    {
      /* Set local fields.  */
      ret->written = false;
      ret->indx = -1;
    }

  return (struct bfd_hash_entry *) ret;
}

/* Initialize an a.out link hash table.  */

bool
NAME (aout, link_hash_table_init) (struct aout_link_hash_table *table,
				   bfd *abfd,
				   struct bfd_hash_entry *(*newfunc) (struct bfd_hash_entry *,
								     struct bfd_hash_table *,
								     const char *),
				   unsigned int entsize)
{
  return _bfd_link_hash_table_init (&table->root, abfd, newfunc, entsize);
}

/* Create an a.out link hash table.  */

struct bfd_link_hash_table *
NAME (aout, link_hash_table_create) (bfd *abfd)
{
  struct aout_link_hash_table *ret;
  size_t amt = sizeof (struct aout_link_hash_table);

  ret = bfd_malloc (amt);
  if (ret == NULL)
    return NULL;
  if (! NAME (aout, link_hash_table_init) (ret, abfd,
					   NAME (aout, link_hash_newfunc),
					   sizeof (struct aout_link_hash_entry)))
    {
      free (ret);
      return NULL;
    }
  return &ret->root;
}

/* Free up the internal symbols read from an a.out file.  */

static bool
aout_link_free_symbols (bfd *abfd)
{
  if (obj_aout_external_syms (abfd) != NULL)
    {
#ifdef USE_MMAP
      bfd_free_window (&obj_aout_sym_window (abfd));
#else
      free ((void *) obj_aout_external_syms (abfd));
#endif
      obj_aout_external_syms (abfd) = NULL;
    }

  if (obj_aout_external_strings (abfd) != NULL)
    {
#ifdef USE_MMAP
      bfd_free_window (&obj_aout_string_window (abfd));
#else
      free ((void *) obj_aout_external_strings (abfd));
#endif
      obj_aout_external_strings (abfd) = NULL;
    }
  return true;
}

/* Given an a.out BFD, add symbols to the global hash table as
   appropriate.  */

bool
NAME (aout, link_add_symbols) (bfd *abfd, struct bfd_link_info *info)
{
  switch (bfd_get_format (abfd))
    {
    case bfd_object:
      return aout_link_add_object_symbols (abfd, info);
    case bfd_archive:
      return _bfd_generic_link_add_archive_symbols
	(abfd, info, aout_link_check_archive_element);
    default:
      bfd_set_error (bfd_error_wrong_format);
      return false;
    }
}

/* Add symbols from an a.out object file.  */

static bool
aout_link_add_object_symbols (bfd *abfd, struct bfd_link_info *info)
{
  if (! aout_get_external_symbols (abfd))
    return false;
  if (! aout_link_add_symbols (abfd, info))
    return false;
  if (! info->keep_memory)
    {
      if (! aout_link_free_symbols (abfd))
	return false;
    }
  return true;
}

/* Look through the internal symbols to see if this object file should
   be included in the link.  We should include this object file if it
   defines any symbols which are currently undefined.  If this object
   file defines a common symbol, then we may adjust the size of the
   known symbol but we do not include the object file in the link
   (unless there is some other reason to include it).  */

static bool
aout_link_check_ar_symbols (bfd *abfd,
			    struct bfd_link_info *info,
			    bool *pneeded,
			    bfd **subsbfd)
{
  struct external_nlist *p;
  struct external_nlist *pend;
  char *strings;

  *pneeded = false;

  /* Look through all the symbols.  */
  p = obj_aout_external_syms (abfd);
  pend = p + obj_aout_external_sym_count (abfd);
  strings = obj_aout_external_strings (abfd);
  for (; p < pend; p++)
    {
      int type = H_GET_8 (abfd, p->e_type);
      const char *name = strings + GET_WORD (abfd, p->e_strx);
      struct bfd_link_hash_entry *h;

      /* Ignore symbols that are not externally visible.  This is an
	 optimization only, as we check the type more thoroughly
	 below.  */
      if ((type & N_EXT) == 0
	  || is_stab(type, name)
	  || type == N_FN)
	continue;

      h = bfd_link_hash_lookup (info->hash, name, false, false, true);

      /* We are only interested in symbols that are currently
	 undefined or common.  */
      if (h == NULL
	  || (h->type != bfd_link_hash_undefined
	      && h->type != bfd_link_hash_common))
	continue;

      if (type == (N_TEXT | N_EXT)
	  || type == (N_DATA | N_EXT)
	  || type == (N_BSS | N_EXT)
	  || type == (N_ABS | N_EXT))
	{
	  /* This object file defines this symbol.  We must link it
	     in.  This is true regardless of whether the current
	     definition of the symbol is undefined or common.  If the
	     current definition is common, we have a case in which we
	     have already seen an object file including
		 int a;
	     and this object file from the archive includes
		 int a = 5;
	     In such a case we must include this object file.

	     FIXME: The SunOS 4.1.3 linker will pull in the archive
	     element if the symbol is defined in the .data section,
	     but not if it is defined in the .text section.  That
	     seems a bit crazy to me, and I haven't implemented it.
	     However, it might be correct.  */
	  if (!(*info->callbacks
		->add_archive_element) (info, abfd, name, subsbfd))
	    continue;
	  *pneeded = true;
	  return true;
	}

      if (type == (N_UNDF | N_EXT))
	{
	  bfd_vma value;

	  value = GET_WORD (abfd, p->e_value);
	  if (value != 0)
	    {
	      /* This symbol is common in the object from the archive
		 file.  */
	      if (h->type == bfd_link_hash_undefined)
		{
		  bfd *symbfd;
		  unsigned int power;

		  symbfd = h->u.undef.abfd;
		  if (symbfd == NULL)
		    {
		      /* This symbol was created as undefined from
			 outside BFD.  We assume that we should link
			 in the object file.  This is done for the -u
			 option in the linker.  */
		      if (!(*info->callbacks
			    ->add_archive_element) (info, abfd, name, subsbfd))
			return false;
		      *pneeded = true;
		      return true;
		    }
		  /* Turn the current link symbol into a common
		     symbol.  It is already on the undefs list.  */
		  h->type = bfd_link_hash_common;
		  h->u.c.p = bfd_hash_allocate (&info->hash->table,
						sizeof (struct bfd_link_hash_common_entry));
		  if (h->u.c.p == NULL)
		    return false;

		  h->u.c.size = value;

		  /* FIXME: This isn't quite right.  The maximum
		     alignment of a common symbol should be set by the
		     architecture of the output file, not of the input
		     file.  */
		  power = bfd_log2 (value);
		  if (power > bfd_get_arch_info (abfd)->section_align_power)
		    power = bfd_get_arch_info (abfd)->section_align_power;
		  h->u.c.p->alignment_power = power;

		  h->u.c.p->section = bfd_make_section_old_way (symbfd,
								"COMMON");
		}
	      else
		{
		  /* Adjust the size of the common symbol if
		     necessary.  */
		  if (value > h->u.c.size)
		    h->u.c.size = value;
		}
	    }
	}
    }

  /* We do not need this object file.  */
  return true;
}

/* Check a single archive element to see if we need to include it in
   the link.  *PNEEDED is set according to whether this element is
   needed in the link or not.  This is called from
   _bfd_generic_link_add_archive_symbols.  */

static bool
aout_link_check_archive_element (bfd *abfd,
				 struct bfd_link_info *info,
				 struct bfd_link_hash_entry *h ATTRIBUTE_UNUSED,
				 const char *name ATTRIBUTE_UNUSED,
				 bool *pneeded)
{
  bfd *oldbfd;
  bool needed;

  if (!aout_get_external_symbols (abfd))
    return false;

  oldbfd = abfd;
  if (!aout_link_check_ar_symbols (abfd, info, pneeded, &abfd))
    return false;

  needed = *pneeded;
  if (needed)
    {
      /* Potentially, the add_archive_element hook may have set a
	 substitute BFD for us.  */
      if (abfd != oldbfd)
	{
	  if (!info->keep_memory
	      && !aout_link_free_symbols (oldbfd))
	    return false;
	  if (!aout_get_external_symbols (abfd))
	    return false;
	}
      if (!aout_link_add_symbols (abfd, info))
	return false;
    }

  if (!info->keep_memory || !needed)
    {
      if (!aout_link_free_symbols (abfd))
	return false;
    }

  return true;
}

/* Add all symbols from an object file to the hash table.  */

static bool
aout_link_add_symbols (bfd *abfd, struct bfd_link_info *info)
{
  bool (*add_one_symbol)
    (struct bfd_link_info *, bfd *, const char *, flagword, asection *,
     bfd_vma, const char *, bool, bool, struct bfd_link_hash_entry **);
  struct external_nlist *syms;
  bfd_size_type sym_count;
  char *strings;
  bool copy;
  struct aout_link_hash_entry **sym_hash;
  struct external_nlist *p;
  struct external_nlist *pend;

  syms = obj_aout_external_syms (abfd);
  sym_count = obj_aout_external_sym_count (abfd);
  strings = obj_aout_external_strings (abfd);
  if (info->keep_memory)
    copy = false;
  else
    copy = true;

  if (aout_backend_info (abfd)->add_dynamic_symbols != NULL)
    {
      if (! ((*aout_backend_info (abfd)->add_dynamic_symbols)
	     (abfd, info, &syms, &sym_count, &strings)))
	return false;
    }

  /* We keep a list of the linker hash table entries that correspond
     to particular symbols.  We could just look them up in the hash
     table, but keeping the list is more efficient.  Perhaps this
     should be conditional on info->keep_memory.  */
  sym_hash = bfd_alloc (abfd,
			sym_count * sizeof (struct aout_link_hash_entry *));
  if (sym_hash == NULL && sym_count != 0)
    return false;
  obj_aout_sym_hashes (abfd) = sym_hash;

  add_one_symbol = aout_backend_info (abfd)->add_one_symbol;
  if (add_one_symbol == NULL)
    add_one_symbol = _bfd_generic_link_add_one_symbol;

  p = syms;
  pend = p + sym_count;
  for (; p < pend; p++, sym_hash++)
    {
      int type;
      const char *name;
      bfd_vma value;
      asection *section;
      flagword flags;
      const char *string;

      *sym_hash = NULL;

      type = H_GET_8 (abfd, p->e_type);

      /* PR 19629: Corrupt binaries can contain illegal string offsets.  */
      if (GET_WORD (abfd, p->e_strx) >= obj_aout_external_string_size (abfd))
	return false;
      name = strings + GET_WORD (abfd, p->e_strx);

      /* Ignore debugging symbols.  */
      if (is_stab (type, name))
	continue;

      value = GET_WORD (abfd, p->e_value);
      flags = BSF_GLOBAL;
      string = NULL;
      switch (type)
	{
	default:
	  /* Shouldn't be any types not covered.  */
	  BFD_ASSERT (0);
	  continue;

	case N_UNDF:
	case N_ABS:
	case N_TEXT:
	case N_DATA:
	case N_BSS:
	case N_REG:
	case N_FN:
	  /* Ignore symbols that are not externally visible.  */
	  continue;

	case N_UNDF | N_EXT:
	  if (value == 0)
	    {
	      section = bfd_und_section_ptr;
	      flags = 0;
	    }
	  else
	    section = bfd_com_section_ptr;
	  break;
	case N_ABS | N_EXT:
	  section = bfd_abs_section_ptr;
	  break;
	case N_TEXT | N_EXT:
	  section = obj_textsec (abfd);
	  value -= bfd_section_vma (section);
	  break;
	case N_DATA | N_EXT:
	  /* Treat N_SETV symbols as N_DATA symbol; see comment in
	     translate_from_native_sym_flags.  */
	  section = obj_datasec (abfd);
	  value -= bfd_section_vma (section);
	  break;
	case N_BSS | N_EXT:
	  section = obj_bsssec (abfd);
	  value -= bfd_section_vma (section);
	  break;
	}

      if (! ((*add_one_symbol)
	     (info, abfd, name, flags, section, value, string, copy, false,
	      (struct bfd_link_hash_entry **) sym_hash)))
	return false;

      /* Restrict the maximum alignment of a common symbol based on
	 the architecture, since a.out has no way to represent
	 alignment requirements of a section in a .o file.  FIXME:
	 This isn't quite right: it should use the architecture of the
	 output file, not the input files.  */
      if ((*sym_hash)->root.type == bfd_link_hash_common
	  && ((*sym_hash)->root.u.c.p->alignment_power >
	      bfd_get_arch_info (abfd)->section_align_power))
	(*sym_hash)->root.u.c.p->alignment_power =
	  bfd_get_arch_info (abfd)->section_align_power;

      /* If this is a set symbol, and we are not building sets, then
	 it is possible for the hash entry to not have been set.  In
	 such a case, treat the symbol as not globally defined.  */
      if ((*sym_hash)->root.type == bfd_link_hash_new)
	{
	  BFD_ASSERT ((flags & BSF_CONSTRUCTOR) != 0);
	  *sym_hash = NULL;
	}
    }

  return true;
}

/* Look up an entry in an the header file hash table.  */

#define aout_link_includes_lookup(table, string, create, copy) \
  ((struct aout_link_includes_entry *) \
   bfd_hash_lookup (&(table)->root, (string), (create), (copy)))

/* The function to create a new entry in the header file hash table.  */

static struct bfd_hash_entry *
aout_link_includes_newfunc (struct bfd_hash_entry *entry,
			    struct bfd_hash_table *table,
			    const char *string)
{
  struct aout_link_includes_entry * ret =
    (struct aout_link_includes_entry *) entry;

  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (ret == NULL)
    ret = bfd_hash_allocate (table,
			     sizeof (struct aout_link_includes_entry));
  if (ret == NULL)
    return NULL;

  /* Call the allocation method of the superclass.  */
  ret = ((struct aout_link_includes_entry *)
	 bfd_hash_newfunc ((struct bfd_hash_entry *) ret, table, string));
  if (ret)
    /* Set local fields.  */
    ret->totals = NULL;

  return (struct bfd_hash_entry *) ret;
}

/* Write out a symbol that was not associated with an a.out input
   object.  */

static bool
aout_link_write_other_symbol (struct bfd_hash_entry *bh, void *data)
{
  struct aout_link_hash_entry *h = (struct aout_link_hash_entry *) bh;
  struct aout_final_link_info *flaginfo = (struct aout_final_link_info *) data;
  bfd *output_bfd;
  int type;
  bfd_vma val;
  struct external_nlist outsym;
  bfd_size_type indx;
  size_t amt;

  if (h->root.type == bfd_link_hash_warning)
    {
      h = (struct aout_link_hash_entry *) h->root.u.i.link;
      if (h->root.type == bfd_link_hash_new)
	return true;
    }

  output_bfd = flaginfo->output_bfd;

  if (aout_backend_info (output_bfd)->write_dynamic_symbol != NULL)
    {
      if (! ((*aout_backend_info (output_bfd)->write_dynamic_symbol)
	     (output_bfd, flaginfo->info, h)))
	{
	  /* FIXME: No way to handle errors.  */
	  abort ();
	}
    }

  if (h->written)
    return true;

  h->written = true;

  /* An indx of -2 means the symbol must be written.  */
  if (h->indx != -2
      && (flaginfo->info->strip == strip_all
	  || (flaginfo->info->strip == strip_some
	      && bfd_hash_lookup (flaginfo->info->keep_hash, h->root.root.string,
				  false, false) == NULL)))
    return true;

  switch (h->root.type)
    {
    default:
      abort ();
      /* Avoid variable not initialized warnings.  */
      return true;
    case bfd_link_hash_new:
      /* This can happen for set symbols when sets are not being
	 built.  */
      return true;
    case bfd_link_hash_undefined:
      type = N_UNDF | N_EXT;
      val = 0;
      break;
    case bfd_link_hash_defined:
    case bfd_link_hash_defweak:
      {
	asection *sec;

	sec = h->root.u.def.section->output_section;
	BFD_ASSERT (bfd_is_abs_section (sec)
		    || sec->owner == output_bfd);
	if (sec == obj_textsec (output_bfd))
	  type = h->root.type == bfd_link_hash_defined ? N_TEXT : N_WEAKT;
	else if (sec == obj_datasec (output_bfd))
	  type = h->root.type == bfd_link_hash_defined ? N_DATA : N_WEAKD;
	else if (sec == obj_bsssec (output_bfd))
	  type = h->root.type == bfd_link_hash_defined ? N_BSS : N_WEAKB;
	else
	  type = h->root.type == bfd_link_hash_defined ? N_ABS : N_WEAKA;
	type |= N_EXT;
	val = (h->root.u.def.value
	       + sec->vma
	       + h->root.u.def.section->output_offset);
      }
      break;
    case bfd_link_hash_common:
      type = N_UNDF | N_EXT;
      val = h->root.u.c.size;
      break;
    case bfd_link_hash_undefweak:
      type = N_WEAKU;
      val = 0;
      /* Fall through.  */
    case bfd_link_hash_indirect:
    case bfd_link_hash_warning:
      /* FIXME: Ignore these for now.  The circumstances under which
	 they should be written out are not clear to me.  */
      return true;
    }

  H_PUT_8 (output_bfd, type, outsym.e_type);
  H_PUT_8 (output_bfd, 0, outsym.e_ovly);
  indx = add_to_stringtab (output_bfd, flaginfo->strtab, h->root.root.string,
			   false);
  if (indx == (bfd_size_type) -1)
    /* FIXME: No way to handle errors.  */
    abort ();

  PUT_WORD (output_bfd, 0, outsym.e_desc);
  PUT_WORD (output_bfd, indx, outsym.e_strx);
  PUT_WORD (output_bfd, val, outsym.e_value);

  amt = EXTERNAL_NLIST_SIZE;
  if (bfd_seek (output_bfd, flaginfo->symoff, SEEK_SET) != 0
      || bfd_bwrite ((void *) &outsym, amt, output_bfd) != amt)
    /* FIXME: No way to handle errors.  */
    abort ();

  flaginfo->symoff += amt;
  h->indx = obj_aout_external_sym_count (output_bfd);
  ++obj_aout_external_sym_count (output_bfd);

  return true;
}

/* Handle a link order which is supposed to generate a reloc.  */

static bool
aout_link_reloc_link_order (struct aout_final_link_info *flaginfo,
			    asection *o,
			    struct bfd_link_order *p)
{
  struct bfd_link_order_reloc *pr;
  int r_index;
  int r_extern;
  reloc_howto_type *howto;
  file_ptr *reloff_ptr;
  struct reloc_std_external srel;
  void * rel_ptr;
  bfd_size_type rel_size;

  pr = p->u.reloc.p;

  if (p->type == bfd_section_reloc_link_order)
    {
      r_extern = 0;
      if (bfd_is_abs_section (pr->u.section))
	r_index = N_ABS | N_EXT;
      else
	{
	  BFD_ASSERT (pr->u.section->owner == flaginfo->output_bfd);
	  r_index = pr->u.section->target_index;
	}
    }
  else
    {
      struct aout_link_hash_entry *h;

      BFD_ASSERT (p->type == bfd_symbol_reloc_link_order);
      r_extern = 1;
      h = ((struct aout_link_hash_entry *)
	   bfd_wrapped_link_hash_lookup (flaginfo->output_bfd, flaginfo->info,
					 pr->u.name, false, false, true));
      if (h != NULL
	  && h->indx >= 0)
	r_index = h->indx;
      else if (h != NULL)
	{
	  /* We decided to strip this symbol, but it turns out that we
	     can't.  Note that we lose the other and desc information
	     here.  I don't think that will ever matter for a global
	     symbol.  */
	  h->indx = -2;
	  h->written = false;
	  if (!aout_link_write_other_symbol (&h->root.root, flaginfo))
	    return false;
	  r_index = h->indx;
	}
      else
	{
	  (*flaginfo->info->callbacks->unattached_reloc)
	    (flaginfo->info, pr->u.name, NULL, NULL, (bfd_vma) 0);
	  r_index = 0;
	}
    }

  howto = bfd_reloc_type_lookup (flaginfo->output_bfd, pr->reloc);
  if (howto == 0)
    {
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  if (o == obj_textsec (flaginfo->output_bfd))
    reloff_ptr = &flaginfo->treloff;
  else if (o == obj_datasec (flaginfo->output_bfd))
    reloff_ptr = &flaginfo->dreloff;
  else
    abort ();

#ifdef MY_put_reloc
  MY_put_reloc(flaginfo->output_bfd, r_extern, r_index, p->offset, howto,
	       &srel);
#else
  {
    int r_pcrel;
    int r_baserel;
    int r_jmptable;
    int r_relative;
    int r_length;

    fprintf (stderr, "TODO: line %d in bfd/pdp11.c\n", __LINE__);

    r_pcrel = howto->pc_relative;
    r_baserel = (howto->type & 8) != 0;
    r_jmptable = (howto->type & 16) != 0;
    r_relative = (howto->type & 32) != 0;
    r_length = bfd_log2 (bfd_get_reloc_size (howto));

    PUT_WORD (flaginfo->output_bfd, p->offset, srel.r_address);
    if (bfd_header_big_endian (flaginfo->output_bfd))
      {
	srel.r_index[0] = r_index >> 16;
	srel.r_index[1] = r_index >> 8;
	srel.r_index[2] = r_index;
	srel.r_type[0] =
	  ((r_extern ?     RELOC_STD_BITS_EXTERN_BIG : 0)
	   | (r_pcrel ?    RELOC_STD_BITS_PCREL_BIG : 0)
	   | (r_baserel ?  RELOC_STD_BITS_BASEREL_BIG : 0)
	   | (r_jmptable ? RELOC_STD_BITS_JMPTABLE_BIG : 0)
	   | (r_relative ? RELOC_STD_BITS_RELATIVE_BIG : 0)
	   | (r_length <<  RELOC_STD_BITS_LENGTH_SH_BIG));
      }
    else
      {
	srel.r_index[2] = r_index >> 16;
	srel.r_index[1] = r_index >> 8;
	srel.r_index[0] = r_index;
	srel.r_type[0] =
	  ((r_extern ?     RELOC_STD_BITS_EXTERN_LITTLE : 0)
	   | (r_pcrel ?    RELOC_STD_BITS_PCREL_LITTLE : 0)
	   | (r_baserel ?  RELOC_STD_BITS_BASEREL_LITTLE : 0)
	   | (r_jmptable ? RELOC_STD_BITS_JMPTABLE_LITTLE : 0)
	   | (r_relative ? RELOC_STD_BITS_RELATIVE_LITTLE : 0)
	   | (r_length <<  RELOC_STD_BITS_LENGTH_SH_LITTLE));
      }
  }
#endif
  rel_ptr = (void *) &srel;

  /* We have to write the addend into the object file, since
     standard a.out relocs are in place.  It would be more
     reliable if we had the current contents of the file here,
     rather than assuming zeroes, but we can't read the file since
     it was opened using bfd_openw.  */
  if (pr->addend != 0)
    {
      bfd_size_type size;
      bfd_reloc_status_type r;
      bfd_byte *buf;
      bool ok;

      size = bfd_get_reloc_size (howto);
      buf = bfd_zmalloc (size);
      if (buf == NULL && size != 0)
	return false;
      r = MY_relocate_contents (howto, flaginfo->output_bfd,
				pr->addend, buf);
      switch (r)
	{
	case bfd_reloc_ok:
	  break;
	default:
	case bfd_reloc_outofrange:
	  abort ();
	case bfd_reloc_overflow:
	  (*flaginfo->info->callbacks->reloc_overflow)
	    (flaginfo->info, NULL,
	     (p->type == bfd_section_reloc_link_order
	      ? bfd_section_name (pr->u.section)
	      : pr->u.name),
	     howto->name, pr->addend, NULL,
	     (asection *) NULL, (bfd_vma) 0);
	  break;
	}
      ok = bfd_set_section_contents (flaginfo->output_bfd, o,
				     (void *) buf,
				     (file_ptr) p->offset,
				     size);
      free (buf);
      if (! ok)
	return false;
    }

  rel_size = obj_reloc_entry_size (flaginfo->output_bfd);
  if (bfd_seek (flaginfo->output_bfd, *reloff_ptr, SEEK_SET) != 0
      || bfd_bwrite (rel_ptr, rel_size, flaginfo->output_bfd) != rel_size)
    return false;

  *reloff_ptr += rel_size;

  /* Assert that the relocs have not run into the symbols, and that n
     the text relocs have not run into the data relocs.  */
  BFD_ASSERT (*reloff_ptr <= obj_sym_filepos (flaginfo->output_bfd)
	      && (reloff_ptr != &flaginfo->treloff
		  || (*reloff_ptr
		      <= obj_datasec (flaginfo->output_bfd)->rel_filepos)));

  return true;
}

/* Get the section corresponding to a reloc index.  */

static inline asection *
aout_reloc_type_to_section (bfd *abfd, int type)
{
  switch (type)
    {
    case RTEXT:	return obj_textsec (abfd);
    case RDATA: return obj_datasec (abfd);
    case RBSS:  return obj_bsssec (abfd);
    case RABS:  return bfd_abs_section_ptr;
    case REXT:  return bfd_und_section_ptr;
    default:    abort ();
    }
}

static bool
pdp11_aout_link_input_section (struct aout_final_link_info *flaginfo,
			       bfd *input_bfd,
			       asection *input_section,
			       bfd_byte *relocs,
			       bfd_size_type rel_size,
			       bfd_byte *contents)
{
  bool (*check_dynamic_reloc)
    (struct bfd_link_info *, bfd *, asection *,
     struct aout_link_hash_entry *, void *, bfd_byte *, bool *, bfd_vma *);
  bfd *output_bfd;
  bool relocatable;
  struct external_nlist *syms;
  char *strings;
  struct aout_link_hash_entry **sym_hashes;
  int *symbol_map;
  bfd_byte *rel;
  bfd_byte *rel_end;

  output_bfd = flaginfo->output_bfd;
  check_dynamic_reloc = aout_backend_info (output_bfd)->check_dynamic_reloc;

  BFD_ASSERT (obj_reloc_entry_size (input_bfd) == RELOC_SIZE);
  BFD_ASSERT (input_bfd->xvec->header_byteorder
	      == output_bfd->xvec->header_byteorder);

  relocatable = bfd_link_relocatable (flaginfo->info);
  syms = obj_aout_external_syms (input_bfd);
  strings = obj_aout_external_strings (input_bfd);
  sym_hashes = obj_aout_sym_hashes (input_bfd);
  symbol_map = flaginfo->symbol_map;

  rel = relocs;
  rel_end = rel + rel_size;
  for (; rel < rel_end; rel += RELOC_SIZE)
    {
      bfd_vma r_addr;
      int r_index;
      int r_type;
      int r_pcrel;
      int r_extern;
      reloc_howto_type *howto;
      struct aout_link_hash_entry *h = NULL;
      bfd_vma relocation;
      bfd_reloc_status_type r;
      int reloc_entry;

      reloc_entry = GET_WORD (input_bfd, (void *) rel);
      if (reloc_entry == 0)
	continue;

      {
	unsigned int howto_idx;

	r_index = (reloc_entry & RIDXMASK) >> 4;
	r_type = reloc_entry & RTYPE;
	r_pcrel = reloc_entry & RELFLG;
	r_addr = (char *) rel - (char *) relocs;

	r_extern = (r_type == REXT);

	howto_idx = r_pcrel;
	if (howto_idx < TABLE_SIZE (howto_table_pdp11))
	  howto = howto_table_pdp11 + howto_idx;
	else
	  {
	    _bfd_error_handler (_("%pB: unsupported relocation type"),
				input_bfd);
	    bfd_set_error (bfd_error_bad_value);
	    return false;
	  }
      }

      if (relocatable)
	{
	  /* We are generating a relocatable output file, and must
	     modify the reloc accordingly.  */
	  if (r_extern)
	    {
	      /* If we know the symbol this relocation is against,
		 convert it into a relocation against a section.  This
		 is what the native linker does.  */
	      h = sym_hashes[r_index];
	      if (h != NULL
		  && (h->root.type == bfd_link_hash_defined
		      || h->root.type == bfd_link_hash_defweak))
		{
		  asection *output_section;

		  /* Compute a new r_index.  */
		  output_section = h->root.u.def.section->output_section;
		  if (output_section == obj_textsec (output_bfd))
		    r_type = N_TEXT;
		  else if (output_section == obj_datasec (output_bfd))
		    r_type = N_DATA;
		  else if (output_section == obj_bsssec (output_bfd))
		    r_type = N_BSS;
		  else
		    r_type = N_ABS;

		  /* Add the symbol value and the section VMA to the
		     addend stored in the contents.  */
		  relocation = (h->root.u.def.value
				+ output_section->vma
				+ h->root.u.def.section->output_offset);
		}
	      else
		{
		  /* We must change r_index according to the symbol
		     map.  */
		  r_index = symbol_map[r_index];

		  if (r_index == -1)
		    {
		      if (h != NULL)
			{
			  /* We decided to strip this symbol, but it
			     turns out that we can't.  Note that we
			     lose the other and desc information here.
			     I don't think that will ever matter for a
			     global symbol.  */
			  if (h->indx < 0)
			    {
			      h->indx = -2;
			      h->written = false;
			      if (!aout_link_write_other_symbol (&h->root.root,
								 flaginfo))
				return false;
			    }
			  r_index = h->indx;
			}
		      else
			{
			  const char *name;

			  name = strings + GET_WORD (input_bfd,
						     syms[r_index].e_strx);
			  (*flaginfo->info->callbacks->unattached_reloc)
			    (flaginfo->info, name, input_bfd, input_section,
			     r_addr);
			  r_index = 0;
			}
		    }

		  relocation = 0;
		}

	      /* Write out the new r_index value.  */
	      reloc_entry = GET_WORD (input_bfd, rel);
	      reloc_entry &= RIDXMASK;
	      reloc_entry |= r_index << 4;
	      PUT_WORD (input_bfd, reloc_entry, rel);
	    }
	  else
	    {
	      asection *section;

	      /* This is a relocation against a section.  We must
		 adjust by the amount that the section moved.  */
	      section = aout_reloc_type_to_section (input_bfd, r_type);
	      relocation = (section->output_section->vma
			    + section->output_offset
			    - section->vma);
	    }

	  /* Change the address of the relocation.  */
	  fprintf (stderr, "TODO: change the address of the relocation\n");

	  /* Adjust a PC relative relocation by removing the reference
	     to the original address in the section and including the
	     reference to the new address.  */
	  if (r_pcrel)
	    relocation -= (input_section->output_section->vma
			   + input_section->output_offset
			   - input_section->vma);

#ifdef MY_relocatable_reloc
	  MY_relocatable_reloc (howto, output_bfd, rel, relocation, r_addr);
#endif

	  if (relocation == 0)
	    r = bfd_reloc_ok;
	  else
	    r = MY_relocate_contents (howto,
				      input_bfd, relocation,
				      contents + r_addr);
	}
      else
	{
	  bool hundef;

	  /* We are generating an executable, and must do a full
	     relocation.  */
	  hundef = false;
	  if (r_extern)
	    {
	      h = sym_hashes[r_index];

	      if (h != NULL
		  && (h->root.type == bfd_link_hash_defined
		      || h->root.type == bfd_link_hash_defweak))
		{
		  relocation = (h->root.u.def.value
				+ h->root.u.def.section->output_section->vma
				+ h->root.u.def.section->output_offset);
		}
	      else if (h != NULL
		       && h->root.type == bfd_link_hash_undefweak)
		relocation = 0;
	      else
		{
		  hundef = true;
		  relocation = 0;
		}
	    }
	  else
	    {
	      asection *section;

	      section = aout_reloc_type_to_section (input_bfd, r_type);
	      relocation = (section->output_section->vma
			    + section->output_offset
			    - section->vma);
	      if (r_pcrel)
		relocation += input_section->vma;
	    }

	  if (check_dynamic_reloc != NULL)
	    {
	      bool skip;

	      if (! ((*check_dynamic_reloc)
		     (flaginfo->info, input_bfd, input_section, h,
		      (void *) rel, contents, &skip, &relocation)))
		return false;
	      if (skip)
		continue;
	    }

	  /* Now warn if a global symbol is undefined.  We could not
	     do this earlier, because check_dynamic_reloc might want
	     to skip this reloc.  */
	  if (hundef && ! bfd_link_pic (flaginfo->info))
	    {
	      const char *name;

	      if (h != NULL)
		name = h->root.root.string;
	      else
		name = strings + GET_WORD (input_bfd, syms[r_index].e_strx);
	      (*flaginfo->info->callbacks->undefined_symbol)
		(flaginfo->info, name, input_bfd, input_section,
		 r_addr, true);
	    }

	  r = MY_final_link_relocate (howto,
				      input_bfd, input_section,
				      contents, r_addr, relocation,
				      (bfd_vma) 0);
	}

      if (r != bfd_reloc_ok)
	{
	  switch (r)
	    {
	    default:
	    case bfd_reloc_outofrange:
	      abort ();
	    case bfd_reloc_overflow:
	      {
		const char *name;

		if (h != NULL)
		  name = NULL;
		else if (r_extern)
		  name = strings + GET_WORD (input_bfd,
					     syms[r_index].e_strx);
		else
		  {
		    asection *s;

		    s = aout_reloc_type_to_section (input_bfd, r_type);
		    name = bfd_section_name (s);
		  }
		(*flaginfo->info->callbacks->reloc_overflow)
		  (flaginfo->info, (h ? &h->root : NULL), name, howto->name,
		   (bfd_vma) 0, input_bfd, input_section, r_addr);
	      }
	      break;
	    }
	}
    }

  return true;
}

/* Link an a.out section into the output file.  */

static bool
aout_link_input_section (struct aout_final_link_info *flaginfo,
			 bfd *input_bfd,
			 asection *input_section,
			 file_ptr *reloff_ptr,
			 bfd_size_type rel_size)
{
  bfd_size_type input_size;
  void * relocs;

  /* Get the section contents.  */
  input_size = input_section->size;
  if (! bfd_get_section_contents (input_bfd, input_section,
				  (void *) flaginfo->contents,
				  (file_ptr) 0, input_size))
    return false;

  /* Read in the relocs if we haven't already done it.  */
  if (aout_section_data (input_section) != NULL
      && aout_section_data (input_section)->relocs != NULL)
    relocs = aout_section_data (input_section)->relocs;
  else
    {
      relocs = flaginfo->relocs;
      if (rel_size > 0)
	{
	  if (bfd_seek (input_bfd, input_section->rel_filepos, SEEK_SET) != 0
	      || bfd_bread (relocs, rel_size, input_bfd) != rel_size)
	    return false;
	}
    }

  /* Relocate the section contents.  */
  if (! pdp11_aout_link_input_section (flaginfo, input_bfd, input_section,
				       (bfd_byte *) relocs,
				       rel_size, flaginfo->contents))
    return false;

  /* Write out the section contents.  */
  if (! bfd_set_section_contents (flaginfo->output_bfd,
				  input_section->output_section,
				  (void *) flaginfo->contents,
				  (file_ptr) input_section->output_offset,
				  input_size))
    return false;

  /* If we are producing relocatable output, the relocs were
     modified, and we now write them out.  */
  if (bfd_link_relocatable (flaginfo->info) && rel_size > 0)
    {
      if (bfd_seek (flaginfo->output_bfd, *reloff_ptr, SEEK_SET) != 0)
	return false;
      if (bfd_bwrite (relocs, rel_size, flaginfo->output_bfd) != rel_size)
	return false;
      *reloff_ptr += rel_size;

      /* Assert that the relocs have not run into the symbols, and
	 that if these are the text relocs they have not run into the
	 data relocs.  */
      BFD_ASSERT (*reloff_ptr <= obj_sym_filepos (flaginfo->output_bfd)
		  && (reloff_ptr != &flaginfo->treloff
		      || (*reloff_ptr
			  <= obj_datasec (flaginfo->output_bfd)->rel_filepos)));
    }

  return true;
}

/* Link an a.out input BFD into the output file.  */

static bool
aout_link_input_bfd (struct aout_final_link_info *flaginfo, bfd *input_bfd)
{
  BFD_ASSERT (bfd_get_format (input_bfd) == bfd_object);

  /* If this is a dynamic object, it may need special handling.  */
  if ((input_bfd->flags & DYNAMIC) != 0
      && aout_backend_info (input_bfd)->link_dynamic_object != NULL)
    return ((*aout_backend_info (input_bfd)->link_dynamic_object)
	    (flaginfo->info, input_bfd));

  /* Get the symbols.  We probably have them already, unless
     flaginfo->info->keep_memory is FALSE.  */
  if (! aout_get_external_symbols (input_bfd))
    return false;

  /* Write out the symbols and get a map of the new indices.  The map
     is placed into flaginfo->symbol_map.  */
  if (! aout_link_write_symbols (flaginfo, input_bfd))
    return false;

  /* Relocate and write out the sections.  These functions use the
     symbol map created by aout_link_write_symbols.  The linker_mark
     field will be set if these sections are to be included in the
     link, which will normally be the case.  */
  if (obj_textsec (input_bfd)->linker_mark)
    {
      if (! aout_link_input_section (flaginfo, input_bfd,
				     obj_textsec (input_bfd),
				     &flaginfo->treloff,
				     exec_hdr (input_bfd)->a_trsize))
	return false;
    }
  if (obj_datasec (input_bfd)->linker_mark)
    {
      if (! aout_link_input_section (flaginfo, input_bfd,
				     obj_datasec (input_bfd),
				     &flaginfo->dreloff,
				     exec_hdr (input_bfd)->a_drsize))
	return false;
    }

  /* If we are not keeping memory, we don't need the symbols any
     longer.  We still need them if we are keeping memory, because the
     strings in the hash table point into them.  */
  if (! flaginfo->info->keep_memory)
    {
      if (! aout_link_free_symbols (input_bfd))
	return false;
    }

  return true;
}

/* Do the final link step.  This is called on the output BFD.  The
   INFO structure should point to a list of BFDs linked through the
   link.next field which can be used to find each BFD which takes part
   in the output.  Also, each section in ABFD should point to a list
   of bfd_link_order structures which list all the input sections for
   the output section.  */

bool
NAME (aout, final_link) (bfd *abfd,
			 struct bfd_link_info *info,
			 void (*callback) (bfd *, file_ptr *, file_ptr *, file_ptr *))
{
  struct aout_final_link_info aout_info;
  bool includes_hash_initialized = false;
  bfd *sub;
  bfd_size_type trsize, drsize;
  bfd_size_type max_contents_size;
  bfd_size_type max_relocs_size;
  bfd_size_type max_sym_count;
  struct bfd_link_order *p;
  asection *o;
  bool have_link_order_relocs;

  if (bfd_link_pic (info))
    abfd->flags |= DYNAMIC;

  separate_i_d = info->separate_code;
  aout_info.info = info;
  aout_info.output_bfd = abfd;
  aout_info.contents = NULL;
  aout_info.relocs = NULL;
  aout_info.symbol_map = NULL;
  aout_info.output_syms = NULL;

  if (!bfd_hash_table_init_n (&aout_info.includes.root,
			      aout_link_includes_newfunc,
			      sizeof (struct aout_link_includes_entry),
			      251))
    goto error_return;
  includes_hash_initialized = true;

  /* Figure out the largest section size.  Also, if generating
     relocatable output, count the relocs.  */
  trsize = 0;
  drsize = 0;
  max_contents_size = 0;
  max_relocs_size = 0;
  max_sym_count = 0;
  for (sub = info->input_bfds; sub != NULL; sub = sub->link.next)
    {
      size_t sz;

      if (bfd_link_relocatable (info))
	{
	  if (bfd_get_flavour (sub) == bfd_target_aout_flavour)
	    {
	      trsize += exec_hdr (sub)->a_trsize;
	      drsize += exec_hdr (sub)->a_drsize;
	    }
	  else
	    {
	      /* FIXME: We need to identify the .text and .data sections
		 and call get_reloc_upper_bound and canonicalize_reloc to
		 work out the number of relocs needed, and then multiply
		 by the reloc size.  */
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB: relocatable link from %s to %s not supported"),
		 abfd, sub->xvec->name, abfd->xvec->name);
	      bfd_set_error (bfd_error_invalid_operation);
	      goto error_return;
	    }
	}

      if (bfd_get_flavour (sub) == bfd_target_aout_flavour)
	{
	  sz = obj_textsec (sub)->size;
	  if (sz > max_contents_size)
	    max_contents_size = sz;
	  sz = obj_datasec (sub)->size;
	  if (sz > max_contents_size)
	    max_contents_size = sz;

	  sz = exec_hdr (sub)->a_trsize;
	  if (sz > max_relocs_size)
	    max_relocs_size = sz;
	  sz = exec_hdr (sub)->a_drsize;
	  if (sz > max_relocs_size)
	    max_relocs_size = sz;

	  sz = obj_aout_external_sym_count (sub);
	  if (sz > max_sym_count)
	    max_sym_count = sz;
	}
    }

  if (bfd_link_relocatable (info))
    {
      if (obj_textsec (abfd) != NULL)
	trsize += (_bfd_count_link_order_relocs (obj_textsec (abfd)
						 ->map_head.link_order)
		   * obj_reloc_entry_size (abfd));
      if (obj_datasec (abfd) != NULL)
	drsize += (_bfd_count_link_order_relocs (obj_datasec (abfd)
						 ->map_head.link_order)
		   * obj_reloc_entry_size (abfd));
    }

  exec_hdr (abfd)->a_trsize = trsize;
  exec_hdr (abfd)->a_drsize = drsize;
  exec_hdr (abfd)->a_entry = bfd_get_start_address (abfd);

  /* Adjust the section sizes and vmas according to the magic number.
     This sets a_text, a_data and a_bss in the exec_hdr and sets the
     filepos for each section.  */
  if (! NAME (aout, adjust_sizes_and_vmas) (abfd))
    goto error_return;

  /* The relocation and symbol file positions differ among a.out
     targets.  We are passed a callback routine from the backend
     specific code to handle this.
     FIXME: At this point we do not know how much space the symbol
     table will require.  This will not work for any (nonstandard)
     a.out target that needs to know the symbol table size before it
     can compute the relocation file positions.  */
  (*callback) (abfd, &aout_info.treloff, &aout_info.dreloff,
	       &aout_info.symoff);
  obj_textsec (abfd)->rel_filepos = aout_info.treloff;
  obj_datasec (abfd)->rel_filepos = aout_info.dreloff;
  obj_sym_filepos (abfd) = aout_info.symoff;

  /* We keep a count of the symbols as we output them.  */
  obj_aout_external_sym_count (abfd) = 0;

  /* We accumulate the string table as we write out the symbols.  */
  aout_info.strtab = _bfd_stringtab_init ();
  if (aout_info.strtab == NULL)
    goto error_return;

  /* Allocate buffers to hold section contents and relocs.  */
  aout_info.contents = bfd_malloc (max_contents_size);
  aout_info.relocs = bfd_malloc (max_relocs_size);
  aout_info.symbol_map = bfd_malloc (max_sym_count * sizeof (int *));
  aout_info.output_syms = bfd_malloc ((max_sym_count + 1)
				      * sizeof (struct external_nlist));
  if ((aout_info.contents == NULL && max_contents_size != 0)
      || (aout_info.relocs == NULL && max_relocs_size != 0)
      || (aout_info.symbol_map == NULL && max_sym_count != 0)
      || aout_info.output_syms == NULL)
    goto error_return;

  /* If we have a symbol named __DYNAMIC, force it out now.  This is
     required by SunOS.  Doing this here rather than in sunos.c is a
     hack, but it's easier than exporting everything which would be
     needed.  */
  {
    struct aout_link_hash_entry *h;

    h = aout_link_hash_lookup (aout_hash_table (info), "__DYNAMIC",
			       false, false, false);
    if (h != NULL)
      aout_link_write_other_symbol (&h->root.root, &aout_info);
  }

  /* The most time efficient way to do the link would be to read all
     the input object files into memory and then sort out the
     information into the output file.  Unfortunately, that will
     probably use too much memory.  Another method would be to step
     through everything that composes the text section and write it
     out, and then everything that composes the data section and write
     it out, and then write out the relocs, and then write out the
     symbols.  Unfortunately, that requires reading stuff from each
     input file several times, and we will not be able to keep all the
     input files open simultaneously, and reopening them will be slow.

     What we do is basically process one input file at a time.  We do
     everything we need to do with an input file once--copy over the
     section contents, handle the relocation information, and write
     out the symbols--and then we throw away the information we read
     from it.  This approach requires a lot of lseeks of the output
     file, which is unfortunate but still faster than reopening a lot
     of files.

     We use the output_has_begun field of the input BFDs to see
     whether we have already handled it.  */
  for (sub = info->input_bfds; sub != NULL; sub = sub->link.next)
    sub->output_has_begun = false;

  /* Mark all sections which are to be included in the link.  This
     will normally be every section.  We need to do this so that we
     can identify any sections which the linker has decided to not
     include.  */
  for (o = abfd->sections; o != NULL; o = o->next)
    {
      for (p = o->map_head.link_order; p != NULL; p = p->next)
	if (p->type == bfd_indirect_link_order)
	  p->u.indirect.section->linker_mark = true;
    }

  have_link_order_relocs = false;
  for (o = abfd->sections; o != NULL; o = o->next)
    {
      for (p = o->map_head.link_order;
	   p != NULL;
	   p = p->next)
	{
	  if (p->type == bfd_indirect_link_order
	      && (bfd_get_flavour (p->u.indirect.section->owner)
		  == bfd_target_aout_flavour))
	    {
	      bfd *input_bfd;

	      input_bfd = p->u.indirect.section->owner;
	      if (! input_bfd->output_has_begun)
		{
		  if (! aout_link_input_bfd (&aout_info, input_bfd))
		    goto error_return;
		  input_bfd->output_has_begun = true;
		}
	    }
	  else if (p->type == bfd_section_reloc_link_order
		   || p->type == bfd_symbol_reloc_link_order)
	    /* These are handled below.  */
	    have_link_order_relocs = true;
	  else
	    {
	      if (! _bfd_default_link_order (abfd, info, o, p))
		goto error_return;
	    }
	}
    }

  /* Write out any symbols that we have not already written out.  */
  bfd_hash_traverse (&info->hash->table,
		     aout_link_write_other_symbol,
		     &aout_info);

  /* Now handle any relocs we were asked to create by the linker.
     These did not come from any input file.  We must do these after
     we have written out all the symbols, so that we know the symbol
     indices to use.  */
  if (have_link_order_relocs)
    {
      for (o = abfd->sections; o != NULL; o = o->next)
	{
	  for (p = o->map_head.link_order;
	       p != NULL;
	       p = p->next)
	    {
	      if (p->type == bfd_section_reloc_link_order
		  || p->type == bfd_symbol_reloc_link_order)
		{
		  if (! aout_link_reloc_link_order (&aout_info, o, p))
		    goto error_return;
		}
	    }
	}
    }

  free (aout_info.contents);
  aout_info.contents = NULL;
  free (aout_info.relocs);
  aout_info.relocs = NULL;
  free (aout_info.symbol_map);
  aout_info.symbol_map = NULL;
  free (aout_info.output_syms);
  aout_info.output_syms = NULL;
  if (includes_hash_initialized)
    {
      bfd_hash_table_free (&aout_info.includes.root);
      includes_hash_initialized = false;
    }

  /* Finish up any dynamic linking we may be doing.  */
  if (aout_backend_info (abfd)->finish_dynamic_link != NULL)
    {
      if (! (*aout_backend_info (abfd)->finish_dynamic_link) (abfd, info))
	goto error_return;
    }

  /* Update the header information.  */
  abfd->symcount = obj_aout_external_sym_count (abfd);
  exec_hdr (abfd)->a_syms = abfd->symcount * EXTERNAL_NLIST_SIZE;
  obj_str_filepos (abfd) = obj_sym_filepos (abfd) + exec_hdr (abfd)->a_syms;
  obj_textsec (abfd)->reloc_count =
    exec_hdr (abfd)->a_trsize / obj_reloc_entry_size (abfd);
  obj_datasec (abfd)->reloc_count =
    exec_hdr (abfd)->a_drsize / obj_reloc_entry_size (abfd);

  /* Write out the string table, unless there are no symbols.  */
  if (abfd->symcount > 0)
    {
      if (bfd_seek (abfd, obj_str_filepos (abfd), SEEK_SET) != 0
	  || ! emit_stringtab (abfd, aout_info.strtab))
	goto error_return;
    }
  else if (obj_textsec (abfd)->reloc_count == 0
	   && obj_datasec (abfd)->reloc_count == 0)
    {
      /* The layout of a typical a.out file is header, text, data,
	 relocs, symbols, string table.  When there are no relocs,
	 symbols or string table, the last thing in the file is data
	 and a_data may be rounded up.  However we may have a smaller
	 sized .data section and thus not written final padding.  The
	 same thing can happen with text if there is no data.  Write
	 final padding here to extend the file.  */
      file_ptr pos = 0;

      if (exec_hdr (abfd)->a_data > obj_datasec (abfd)->size)
	pos = obj_datasec (abfd)->filepos + exec_hdr (abfd)->a_data;
      else if (obj_datasec (abfd)->size == 0
	       && exec_hdr (abfd)->a_text > obj_textsec (abfd)->size)
	pos = obj_textsec (abfd)->filepos + exec_hdr (abfd)->a_text;
      if (pos != 0)
	{
	  bfd_byte b = 0;

	  if (bfd_seek (abfd, pos - 1, SEEK_SET) != 0
	      || bfd_bwrite (&b, (bfd_size_type) 1, abfd) != 1)
	    goto error_return;
	}
    }

  return true;

 error_return:
  free (aout_info.contents);
  free (aout_info.relocs);
  free (aout_info.symbol_map);
  free (aout_info.output_syms);
  if (includes_hash_initialized)
    bfd_hash_table_free (&aout_info.includes.root);
  return false;
}

/* Adjust and write out the symbols for an a.out file.  Set the new
   symbol indices into a symbol_map.  */

static bool
aout_link_write_symbols (struct aout_final_link_info *flaginfo, bfd *input_bfd)
{
  bfd *output_bfd;
  bfd_size_type sym_count;
  char *strings;
  enum bfd_link_strip strip;
  enum bfd_link_discard discard;
  struct external_nlist *outsym;
  bfd_size_type strtab_index;
  struct external_nlist *sym;
  struct external_nlist *sym_end;
  struct aout_link_hash_entry **sym_hash;
  int *symbol_map;
  bool pass;
  bool skip_next;

  output_bfd = flaginfo->output_bfd;
  sym_count = obj_aout_external_sym_count (input_bfd);
  strings = obj_aout_external_strings (input_bfd);
  strip = flaginfo->info->strip;
  discard = flaginfo->info->discard;
  outsym = flaginfo->output_syms;

  /* First write out a symbol for this object file, unless we are
     discarding such symbols.  */
  if (strip != strip_all
      && (strip != strip_some
	  || bfd_hash_lookup (flaginfo->info->keep_hash,
			      bfd_get_filename (input_bfd),
			      false, false) != NULL)
      && discard != discard_all)
    {
      H_PUT_8 (output_bfd, N_TEXT, outsym->e_type);
      H_PUT_8 (output_bfd, 0, outsym->e_ovly);
      H_PUT_16 (output_bfd, 0, outsym->e_desc);
      strtab_index = add_to_stringtab (output_bfd, flaginfo->strtab,
				       bfd_get_filename (input_bfd), false);
      if (strtab_index == (bfd_size_type) -1)
	return false;
      PUT_WORD (output_bfd, strtab_index, outsym->e_strx);
      PUT_WORD (output_bfd,
		(bfd_section_vma (obj_textsec (input_bfd)->output_section)
		 + obj_textsec (input_bfd)->output_offset),
		outsym->e_value);
      ++obj_aout_external_sym_count (output_bfd);
      ++outsym;
    }

  pass = false;
  skip_next = false;
  sym = obj_aout_external_syms (input_bfd);
  sym_end = sym + sym_count;
  sym_hash = obj_aout_sym_hashes (input_bfd);
  symbol_map = flaginfo->symbol_map;
  memset (symbol_map, 0, (size_t) sym_count * sizeof *symbol_map);
  for (; sym < sym_end; sym++, sym_hash++, symbol_map++)
    {
      const char *name;
      int type;
      struct aout_link_hash_entry *h;
      bool skip;
      asection *symsec;
      bfd_vma val = 0;
      bool copy;

      /* We set *symbol_map to 0 above for all symbols.  If it has
	 already been set to -1 for this symbol, it means that we are
	 discarding it because it appears in a duplicate header file.
	 See the N_BINCL code below.  */
      if (*symbol_map == -1)
	continue;

      /* Initialize *symbol_map to -1, which means that the symbol was
	 not copied into the output file.  We will change it later if
	 we do copy the symbol over.  */
      *symbol_map = -1;

      type = H_GET_8 (input_bfd, sym->e_type);
      name = strings + GET_WORD (input_bfd, sym->e_strx);

      h = NULL;

      if (pass)
	{
	  /* Pass this symbol through.  It is the target of an
	     indirect or warning symbol.  */
	  val = GET_WORD (input_bfd, sym->e_value);
	  pass = false;
	}
      else if (skip_next)
	{
	  /* Skip this symbol, which is the target of an indirect
	     symbol that we have changed to no longer be an indirect
	     symbol.  */
	  skip_next = false;
	  continue;
	}
      else
	{
	  struct aout_link_hash_entry *hresolve;

	  /* We have saved the hash table entry for this symbol, if
	     there is one.  Note that we could just look it up again
	     in the hash table, provided we first check that it is an
	     external symbol. */
	  h = *sym_hash;

	  /* Use the name from the hash table, in case the symbol was
	     wrapped.  */
	  if (h != NULL)
	    name = h->root.root.string;

	  /* If this is an indirect or warning symbol, then change
	     hresolve to the base symbol.  We also change *sym_hash so
	     that the relocation routines relocate against the real
	     symbol.  */
	  hresolve = h;
	  if (h != NULL
	      && (h->root.type == bfd_link_hash_indirect
		  || h->root.type == bfd_link_hash_warning))
	    {
	      hresolve = (struct aout_link_hash_entry *) h->root.u.i.link;
	      while (hresolve->root.type == bfd_link_hash_indirect
		     || hresolve->root.type == bfd_link_hash_warning)
		hresolve = ((struct aout_link_hash_entry *)
			    hresolve->root.u.i.link);
	      *sym_hash = hresolve;
	    }

	  /* If the symbol has already been written out, skip it.  */
	  if (h != NULL
	      && h->root.type != bfd_link_hash_warning
	      && h->written)
	    {
	      if ((type & N_TYPE) == N_INDR
		  || type == N_WARNING)
		skip_next = true;
	      *symbol_map = h->indx;
	      continue;
	    }

	  /* See if we are stripping this symbol.  */
	  skip = false;
	  switch (strip)
	    {
	    case strip_none:
	      break;
	    case strip_debugger:
	      if (is_stab (type, name))
		skip = true;
	      break;
	    case strip_some:
	      if (bfd_hash_lookup (flaginfo->info->keep_hash, name,
				   false, false) == NULL)
		skip = true;
	      break;
	    case strip_all:
	      skip = true;
	      break;
	    }
	  if (skip)
	    {
	      if (h != NULL)
		h->written = true;
	      continue;
	    }

	  /* Get the value of the symbol.  */
	  if (is_stab (type, name))
	    {
	      switch (type)
		{
		default:
		  symsec = bfd_abs_section_ptr;
		  break;
		case N_SO:
		case N_SOL:
		case N_FUN:
		case N_ENTRY:
		case N_SLINE:
		case N_FN:
		  symsec = obj_textsec (input_bfd);
		  break;
		case N_STSYM:
		case N_DSLINE:
		  symsec = obj_datasec (input_bfd);
		  break;
		case N_LCSYM:
		case N_BSLINE:
		  symsec = obj_bsssec (input_bfd);
		  break;
		}
	      val = GET_WORD (input_bfd, sym->e_value);
	    }
	  else if ((type & N_TYPE) == N_TEXT
	      || type == N_WEAKT)
	    symsec = obj_textsec (input_bfd);
	  else if ((type & N_TYPE) == N_DATA
		   || type == N_WEAKD)
	    symsec = obj_datasec (input_bfd);
	  else if ((type & N_TYPE) == N_BSS
		   || type == N_WEAKB)
	    symsec = obj_bsssec (input_bfd);
	  else if ((type & N_TYPE) == N_ABS
		   || type == N_WEAKA)
	    symsec = bfd_abs_section_ptr;
	  else if (((type & N_TYPE) == N_INDR
		    && (hresolve == NULL
			|| (hresolve->root.type != bfd_link_hash_defined
			    && hresolve->root.type != bfd_link_hash_defweak
			    && hresolve->root.type != bfd_link_hash_common)))
		   || type == N_WARNING)
	    {
	      /* Pass the next symbol through unchanged.  The
		 condition above for indirect symbols is so that if
		 the indirect symbol was defined, we output it with
		 the correct definition so the debugger will
		 understand it.  */
	      pass = true;
	      val = GET_WORD (input_bfd, sym->e_value);
	      symsec = NULL;
	    }
	  else
	    {
	      /* If we get here with an indirect symbol, it means that
		 we are outputting it with a real definition.  In such
		 a case we do not want to output the next symbol,
		 which is the target of the indirection.  */
	      if ((type & N_TYPE) == N_INDR)
		skip_next = true;

	      symsec = NULL;

	      /* We need to get the value from the hash table.  We use
		 hresolve so that if we have defined an indirect
		 symbol we output the final definition.  */
	      if (h == NULL)
		{
		  switch (type & N_TYPE)
		    {
		    case N_SETT:
		      symsec = obj_textsec (input_bfd);
		      break;
		    case N_SETD:
		      symsec = obj_datasec (input_bfd);
		      break;
		    case N_SETB:
		      symsec = obj_bsssec (input_bfd);
		      break;
		    case N_SETA:
		      symsec = bfd_abs_section_ptr;
		      break;
		    default:
		      val = 0;
		      break;
		    }
		}
	      else if (hresolve->root.type == bfd_link_hash_defined
		       || hresolve->root.type == bfd_link_hash_defweak)
		{
		  asection *input_section;
		  asection *output_section;

		  /* This case usually means a common symbol which was
		     turned into a defined symbol.  */
		  input_section = hresolve->root.u.def.section;
		  output_section = input_section->output_section;
		  BFD_ASSERT (bfd_is_abs_section (output_section)
			      || output_section->owner == output_bfd);
		  val = (hresolve->root.u.def.value
			 + bfd_section_vma (output_section)
			 + input_section->output_offset);

		  /* Get the correct type based on the section.  If
		     this is a constructed set, force it to be
		     globally visible.  */
		  if (type == N_SETT
		      || type == N_SETD
		      || type == N_SETB
		      || type == N_SETA)
		    type |= N_EXT;

		  type &=~ N_TYPE;

		  if (output_section == obj_textsec (output_bfd))
		    type |= (hresolve->root.type == bfd_link_hash_defined
			     ? N_TEXT
			     : N_WEAKT);
		  else if (output_section == obj_datasec (output_bfd))
		    type |= (hresolve->root.type == bfd_link_hash_defined
			     ? N_DATA
			     : N_WEAKD);
		  else if (output_section == obj_bsssec (output_bfd))
		    type |= (hresolve->root.type == bfd_link_hash_defined
			     ? N_BSS
			     : N_WEAKB);
		  else
		    type |= (hresolve->root.type == bfd_link_hash_defined
			     ? N_ABS
			     : N_WEAKA);
		}
	      else if (hresolve->root.type == bfd_link_hash_common)
		val = hresolve->root.u.c.size;
	      else if (hresolve->root.type == bfd_link_hash_undefweak)
		{
		  val = 0;
		  type = N_WEAKU;
		}
	      else
		val = 0;
	    }
	  if (symsec != NULL)
	    val = (symsec->output_section->vma
		   + symsec->output_offset
		   + (GET_WORD (input_bfd, sym->e_value)
		      - symsec->vma));

	  /* If this is a global symbol set the written flag, and if
	     it is a local symbol see if we should discard it.  */
	  if (h != NULL)
	    {
	      h->written = true;
	      h->indx = obj_aout_external_sym_count (output_bfd);
	    }
	  else if ((type & N_TYPE) != N_SETT
		   && (type & N_TYPE) != N_SETD
		   && (type & N_TYPE) != N_SETB
		   && (type & N_TYPE) != N_SETA)
	    {
	      switch (discard)
		{
		case discard_none:
		case discard_sec_merge:
		  break;
		case discard_l:
		  if (!is_stab (type, name)
		      && bfd_is_local_label_name (input_bfd, name))
		    skip = true;
		  break;
		case discard_all:
		  skip = true;
		  break;
		}
	      if (skip)
		{
		  pass = false;
		  continue;
		}
	    }

	  /* An N_BINCL symbol indicates the start of the stabs
	     entries for a header file.  We need to scan ahead to the
	     next N_EINCL symbol, ignoring nesting, adding up all the
	     characters in the symbol names, not including the file
	     numbers in types (the first number after an open
	     parenthesis).  */
	  if (type == N_BINCL)
	    {
	      struct external_nlist *incl_sym;
	      int nest;
	      struct aout_link_includes_entry *incl_entry;
	      struct aout_link_includes_totals *t;

	      val = 0;
	      nest = 0;
	      for (incl_sym = sym + 1; incl_sym < sym_end; incl_sym++)
		{
		  int incl_type;

		  incl_type = H_GET_8 (input_bfd, incl_sym->e_type);
		  if (incl_type == N_EINCL)
		    {
		      if (nest == 0)
			break;
		      --nest;
		    }
		  else if (incl_type == N_BINCL)
		    ++nest;
		  else if (nest == 0)
		    {
		      const char *s;

		      s = strings + GET_WORD (input_bfd, incl_sym->e_strx);
		      for (; *s != '\0'; s++)
			{
			  val += *s;
			  if (*s == '(')
			    {
			      /* Skip the file number.  */
			      ++s;
			      while (ISDIGIT (*s))
				++s;
			      --s;
			    }
			}
		    }
		}

	      /* If we have already included a header file with the
		 same value, then replace this one with an N_EXCL
		 symbol.  */
	      copy = ! flaginfo->info->keep_memory;
	      incl_entry = aout_link_includes_lookup (&flaginfo->includes,
						      name, true, copy);
	      if (incl_entry == NULL)
		return false;
	      for (t = incl_entry->totals; t != NULL; t = t->next)
		if (t->total == val)
		  break;
	      if (t == NULL)
		{
		  /* This is the first time we have seen this header
		     file with this set of stabs strings.  */
		  t = bfd_hash_allocate (&flaginfo->includes.root,
					 sizeof *t);
		  if (t == NULL)
		    return false;
		  t->total = val;
		  t->next = incl_entry->totals;
		  incl_entry->totals = t;
		}
	      else
		{
		  int *incl_map;

		  /* This is a duplicate header file.  We must change
		     it to be an N_EXCL entry, and mark all the
		     included symbols to prevent outputting them.  */
		  type = N_EXCL;

		  nest = 0;
		  for (incl_sym = sym + 1, incl_map = symbol_map + 1;
		       incl_sym < sym_end;
		       incl_sym++, incl_map++)
		    {
		      int incl_type;

		      incl_type = H_GET_8 (input_bfd, incl_sym->e_type);
		      if (incl_type == N_EINCL)
			{
			  if (nest == 0)
			    {
			      *incl_map = -1;
			      break;
			    }
			  --nest;
			}
		      else if (incl_type == N_BINCL)
			++nest;
		      else if (nest == 0)
			*incl_map = -1;
		    }
		}
	    }
	}

      /* Copy this symbol into the list of symbols we are going to
	 write out.  */
      H_PUT_8 (output_bfd, type, outsym->e_type);
      H_PUT_8 (output_bfd, H_GET_8 (input_bfd, sym->e_ovly), outsym->e_ovly);
      H_PUT_16 (output_bfd, H_GET_16 (input_bfd, sym->e_desc), outsym->e_desc);
      copy = false;
      if (! flaginfo->info->keep_memory)
	{
	  /* name points into a string table which we are going to
	     free.  If there is a hash table entry, use that string.
	     Otherwise, copy name into memory.  */
	  if (h != NULL)
	    name = h->root.root.string;
	  else
	    copy = true;
	}
      strtab_index = add_to_stringtab (output_bfd, flaginfo->strtab,
				       name, copy);
      if (strtab_index == (bfd_size_type) -1)
	return false;
      PUT_WORD (output_bfd, strtab_index, outsym->e_strx);
      PUT_WORD (output_bfd, val, outsym->e_value);
      *symbol_map = obj_aout_external_sym_count (output_bfd);
      ++obj_aout_external_sym_count (output_bfd);
      ++outsym;
    }

  /* Write out the output symbols we have just constructed.  */
  if (outsym > flaginfo->output_syms)
    {
      bfd_size_type size;

      if (bfd_seek (output_bfd, flaginfo->symoff, SEEK_SET) != 0)
	return false;
      size = outsym - flaginfo->output_syms;
      size *= EXTERNAL_NLIST_SIZE;
      if (bfd_bwrite ((void *) flaginfo->output_syms, size, output_bfd) != size)
	return false;
      flaginfo->symoff += size;
    }

  return true;
}

/* Write out a symbol that was not associated with an a.out input
   object.  */

static bfd_vma
bfd_getp32 (const void *p)
{
  const bfd_byte *addr = p;
  unsigned long v;

  v = (unsigned long) addr[1] << 24;
  v |= (unsigned long) addr[0] << 16;
  v |= (unsigned long) addr[3] << 8;
  v |= (unsigned long) addr[2];
  return v;
}

#define COERCE32(x) (((bfd_signed_vma) (x) ^ 0x80000000) - 0x80000000)

static bfd_signed_vma
bfd_getp_signed_32 (const void *p)
{
  const bfd_byte *addr = p;
  unsigned long v;

  v = (unsigned long) addr[1] << 24;
  v |= (unsigned long) addr[0] << 16;
  v |= (unsigned long) addr[3] << 8;
  v |= (unsigned long) addr[2];
  return COERCE32 (v);
}

static void
bfd_putp32 (bfd_vma data, void *p)
{
  bfd_byte *addr = p;

  addr[0] = (data >> 16) & 0xff;
  addr[1] = (data >> 24) & 0xff;
  addr[2] = (data >> 0) & 0xff;
  addr[3] = (data >> 8) & 0xff;
}

const bfd_target MY (vec) =
{
  TARGETNAME,			/* Name.  */
  bfd_target_aout_flavour,
  BFD_ENDIAN_LITTLE,		/* Target byte order (little).  */
  BFD_ENDIAN_LITTLE,		/* Target headers byte order (little).  */
  (HAS_RELOC | EXEC_P |		/* Object flags.  */
   HAS_LINENO | HAS_DEBUG |
   HAS_SYMS | HAS_LOCALS | WP_TEXT),
  (SEC_HAS_CONTENTS | SEC_ALLOC | SEC_LOAD | SEC_RELOC | SEC_CODE | SEC_DATA),
  MY_symbol_leading_char,
  AR_PAD_CHAR,			/* AR_pad_char.  */
  15,				/* AR_max_namelen.  */
  0,				/* match priority.  */
  TARGET_KEEP_UNUSED_SECTION_SYMBOLS, /* keep unused section symbols.  */
  bfd_getl64, bfd_getl_signed_64, bfd_putl64,
     bfd_getp32, bfd_getp_signed_32, bfd_putp32,
     bfd_getl16, bfd_getl_signed_16, bfd_putl16, /* Data.  */
  bfd_getl64, bfd_getl_signed_64, bfd_putl64,
     bfd_getp32, bfd_getp_signed_32, bfd_putp32,
     bfd_getl16, bfd_getl_signed_16, bfd_putl16, /* Headers.  */
  {				/* bfd_check_format.  */
    _bfd_dummy_target,
    MY_object_p,
    bfd_generic_archive_p,
    MY_core_file_p
  },
  {				/* bfd_set_format.  */
    _bfd_bool_bfd_false_error,
    MY_mkobject,
    _bfd_generic_mkarchive,
    _bfd_bool_bfd_false_error
  },
  {			/* bfd_write_contents.  */
    _bfd_bool_bfd_false_error,
    MY_write_object_contents,
    _bfd_write_archive_contents,
    _bfd_bool_bfd_false_error
  },

  BFD_JUMP_TABLE_GENERIC (MY),
  BFD_JUMP_TABLE_COPY (MY),
  BFD_JUMP_TABLE_CORE (MY),
  BFD_JUMP_TABLE_ARCHIVE (MY),
  BFD_JUMP_TABLE_SYMBOLS (MY),
  BFD_JUMP_TABLE_RELOCS (MY),
  BFD_JUMP_TABLE_WRITE (MY),
  BFD_JUMP_TABLE_LINK (MY),
  BFD_JUMP_TABLE_DYNAMIC (MY),

  /* Alternative_target.  */
  NULL,

  (void *) MY_backend_data
};
