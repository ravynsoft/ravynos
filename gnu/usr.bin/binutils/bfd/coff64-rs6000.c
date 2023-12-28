/* BFD back-end for IBM RS/6000 "XCOFF64" files.
   Copyright (C) 2000-2023 Free Software Foundation, Inc.
   Written Clinton Popetz.
   Contributed by Cygnus Support.

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

#include "sysdep.h"
#include "bfd.h"
#include "bfdlink.h"
#include "libbfd.h"
#include "coff/internal.h"
#include "coff/xcoff.h"
#include "coff/rs6k64.h"
#include "libcoff.h"
#include "libxcoff.h"

#define GET_FILEHDR_SYMPTR H_GET_64
#define PUT_FILEHDR_SYMPTR H_PUT_64
#define GET_AOUTHDR_DATA_START H_GET_64
#define PUT_AOUTHDR_DATA_START H_PUT_64
#define GET_AOUTHDR_TEXT_START H_GET_64
#define PUT_AOUTHDR_TEXT_START H_PUT_64
#define GET_AOUTHDR_TSIZE H_GET_64
#define PUT_AOUTHDR_TSIZE H_PUT_64
#define GET_AOUTHDR_DSIZE H_GET_64
#define PUT_AOUTHDR_DSIZE H_PUT_64
#define GET_AOUTHDR_BSIZE H_GET_64
#define PUT_AOUTHDR_BSIZE H_PUT_64
#define GET_AOUTHDR_ENTRY H_GET_64
#define PUT_AOUTHDR_ENTRY H_PUT_64
#define GET_SCNHDR_PADDR H_GET_64
#define PUT_SCNHDR_PADDR H_PUT_64
#define GET_SCNHDR_VADDR H_GET_64
#define PUT_SCNHDR_VADDR H_PUT_64
#define GET_SCNHDR_SIZE H_GET_64
#define PUT_SCNHDR_SIZE H_PUT_64
#define GET_SCNHDR_SCNPTR H_GET_64
#define PUT_SCNHDR_SCNPTR H_PUT_64
#define GET_SCNHDR_RELPTR H_GET_64
#define PUT_SCNHDR_RELPTR H_PUT_64
#define GET_SCNHDR_LNNOPTR H_GET_64
#define PUT_SCNHDR_LNNOPTR H_PUT_64
#define GET_SCNHDR_NRELOC H_GET_32
#define MAX_SCNHDR_NRELOC 0xffffffff
#define PUT_SCNHDR_NRELOC H_PUT_32
#define GET_SCNHDR_NLNNO H_GET_32
#define MAX_SCNHDR_NLNNO 0xffffffff
#define PUT_SCNHDR_NLNNO H_PUT_32
#define GET_RELOC_VADDR H_GET_64
#define PUT_RELOC_VADDR H_PUT_64

#define COFF_FORCE_SYMBOLS_IN_STRINGS
#define COFF_DEBUG_STRING_WIDE_PREFIX


#define COFF_ADJUST_SCNHDR_OUT_POST(ABFD, INT, EXT)			\
  do									\
    {									\
      memset (((SCNHDR *) EXT)->s_pad, 0,				\
	      sizeof (((SCNHDR *) EXT)->s_pad));			\
    }									\
  while (0)

#define NO_COFF_LINENOS

#define coff_SWAP_lineno_in _bfd_xcoff64_swap_lineno_in
#define coff_SWAP_lineno_out _bfd_xcoff64_swap_lineno_out

static void _bfd_xcoff64_swap_lineno_in
  (bfd *, void *, void *);
static unsigned int _bfd_xcoff64_swap_lineno_out
  (bfd *, void *, void *);
static bool _bfd_xcoff64_put_symbol_name
  (struct bfd_link_info *, struct bfd_strtab_hash *,
   struct internal_syment *, const char *);
static bool _bfd_xcoff64_put_ldsymbol_name
  (bfd *, struct xcoff_loader_info *, struct internal_ldsym *, const char *);
static void _bfd_xcoff64_swap_sym_in
  (bfd *, void *, void *);
static unsigned int _bfd_xcoff64_swap_sym_out
  (bfd *, void *, void *);
static void _bfd_xcoff64_swap_aux_in
  (bfd *, void *, int, int, int, int, void *);
static unsigned int _bfd_xcoff64_swap_aux_out
  (bfd *, void *, int, int, int, int, void *);
static void xcoff64_swap_reloc_in
  (bfd *, void *, void *);
static unsigned int xcoff64_swap_reloc_out
  (bfd *, void *, void *);
extern bool _bfd_xcoff_mkobject
  (bfd *);
extern bool _bfd_xcoff_copy_private_bfd_data
  (bfd *, bfd *);
extern bool _bfd_xcoff_is_local_label_name
  (bfd *, const char *);
extern void xcoff64_rtype2howto
  (arelent *, struct internal_reloc *);
extern reloc_howto_type * xcoff64_reloc_type_lookup
  (bfd *, bfd_reloc_code_real_type);
extern bool _bfd_xcoff_slurp_armap
  (bfd *);
extern void *_bfd_xcoff_read_ar_hdr
  (bfd *);
extern bfd *_bfd_xcoff_openr_next_archived_file
  (bfd *, bfd *);
extern int _bfd_xcoff_stat_arch_elt
  (bfd *, struct stat *);
extern bool _bfd_xcoff_write_armap
  (bfd *, unsigned int, struct orl *, unsigned int, int);
extern bool _bfd_xcoff_write_archive_contents
  (bfd *);
extern int _bfd_xcoff_sizeof_headers
  (bfd *, struct bfd_link_info *);
extern void _bfd_xcoff_swap_sym_in
  (bfd *, void *, void *);
extern unsigned int _bfd_xcoff_swap_sym_out
  (bfd *, void *, void *);
extern void _bfd_xcoff_swap_aux_in
  (bfd *, void *, int, int, int, int, void *);
extern unsigned int _bfd_xcoff_swap_aux_out
  (bfd *, void *, int, int, int, int, void *);
static void xcoff64_swap_ldhdr_in
  (bfd *, const void *, struct internal_ldhdr *);
static void xcoff64_swap_ldhdr_out
  (bfd *, const struct internal_ldhdr *, void *d);
static void xcoff64_swap_ldsym_in
  (bfd *, const void *, struct internal_ldsym *);
static void xcoff64_swap_ldsym_out
  (bfd *, const struct internal_ldsym *, void *d);
static void xcoff64_swap_ldrel_in
  (bfd *, const void *, struct internal_ldrel *);
static void xcoff64_swap_ldrel_out
  (bfd *, const struct internal_ldrel *, void *d);
static bool xcoff64_ppc_relocate_section
  (bfd *, struct bfd_link_info *, bfd *, asection *, bfd_byte *,
   struct internal_reloc *, struct internal_syment *,
   asection **);
static bool xcoff64_slurp_armap
  (bfd *);
static bfd_cleanup xcoff64_archive_p
  (bfd *);
static bfd *xcoff64_openr_next_archived_file
  (bfd *, bfd *);
static int xcoff64_sizeof_headers
  (bfd *, struct bfd_link_info *);
static asection *xcoff64_create_csect_from_smclas
  (bfd *, union internal_auxent *, const char *);
static bool xcoff64_is_lineno_count_overflow
  (bfd *, bfd_vma);
static bool xcoff64_is_reloc_count_overflow
  (bfd *, bfd_vma);
static bfd_vma xcoff64_loader_symbol_offset
  (bfd *, struct internal_ldhdr *);
static bfd_vma xcoff64_loader_reloc_offset
  (bfd *, struct internal_ldhdr *);
static bool xcoff64_generate_rtinit
  (bfd *, const char *, const char *, bool);
static bool xcoff64_bad_format_hook
  (bfd *, void *);

/* Relocation functions */
static xcoff_reloc_function xcoff64_reloc_type_br;

xcoff_reloc_function *const
xcoff64_calculate_relocation[XCOFF_MAX_CALCULATE_RELOCATION] =
{
  xcoff_reloc_type_pos,  /* R_POS     (0x00) */
  xcoff_reloc_type_neg,  /* R_NEG     (0x01) */
  xcoff_reloc_type_rel,  /* R_REL     (0x02) */
  xcoff_reloc_type_toc,  /* R_TOC     (0x03) */
  xcoff_reloc_type_toc,  /* R_TRL     (0x04) */
  xcoff_reloc_type_toc,  /* R_GL      (0x05) */
  xcoff_reloc_type_toc,  /* R_TCL     (0x06) */
  xcoff_reloc_type_fail, /*           (0x07) */
  xcoff_reloc_type_ba,   /* R_BA      (0x08) */
  xcoff_reloc_type_fail, /*           (0x09) */
  xcoff64_reloc_type_br, /* R_BR      (0x0a) */
  xcoff_reloc_type_fail, /*           (0x0b) */
  xcoff_reloc_type_pos,  /* R_RL      (0x0c) */
  xcoff_reloc_type_pos,  /* R_RLA     (0x0d) */
  xcoff_reloc_type_fail, /*           (0x0e) */
  xcoff_reloc_type_noop, /* R_REF     (0x0f) */
  xcoff_reloc_type_fail, /*           (0x10) */
  xcoff_reloc_type_fail, /*           (0x11) */
  xcoff_reloc_type_fail, /*           (0x12) */
  xcoff_reloc_type_toc,  /* R_TRLA    (0x13) */
  xcoff_reloc_type_fail, /* R_RRTBI   (0x14) */
  xcoff_reloc_type_fail, /* R_RRTBA   (0x15) */
  xcoff_reloc_type_ba,   /* R_CAI     (0x16) */
  xcoff_reloc_type_crel, /* R_CREL    (0x17) */
  xcoff_reloc_type_ba,   /* R_RBA     (0x18) */
  xcoff_reloc_type_ba,   /* R_RBAC    (0x19) */
  xcoff64_reloc_type_br, /* R_RBR     (0x1a) */
  xcoff_reloc_type_ba,   /* R_RBRC    (0x1b) */
  xcoff_reloc_type_fail, /*           (0x1c) */
  xcoff_reloc_type_fail, /*           (0x1d) */
  xcoff_reloc_type_fail, /*           (0x1e) */
  xcoff_reloc_type_fail, /*           (0x1f) */
  xcoff_reloc_type_tls,  /* R_TLS     (0x20) */
  xcoff_reloc_type_tls,  /* R_TLS_IE  (0x21) */
  xcoff_reloc_type_tls,  /* R_TLS_LD  (0x22) */
  xcoff_reloc_type_tls,  /* R_TLS_LE  (0x23) */
  xcoff_reloc_type_tls,  /* R_TLSM    (0x24) */
  xcoff_reloc_type_tls,  /* R_TLSML   (0x25) */
  xcoff_reloc_type_fail, /*           (0x26) */
  xcoff_reloc_type_fail, /*           (0x27) */
  xcoff_reloc_type_fail, /*           (0x28) */
  xcoff_reloc_type_fail, /*           (0x29) */
  xcoff_reloc_type_fail, /*           (0x2a) */
  xcoff_reloc_type_fail, /*           (0x2b) */
  xcoff_reloc_type_fail, /*           (0x2c) */
  xcoff_reloc_type_fail, /*           (0x2d) */
  xcoff_reloc_type_fail, /*           (0x2e) */
  xcoff_reloc_type_fail, /*           (0x2f) */
  xcoff_reloc_type_toc, /* R_TOCU    (0x30) */
  xcoff_reloc_type_toc, /* R_TOCL    (0x31) */
};

/* coffcode.h needs these to be defined.  */
/* Internalcoff.h and coffcode.h modify themselves based on these flags.  */
#define XCOFF64
#define RS6000COFF_C 1

#define SELECT_RELOC(internal, howto)					\
  {									\
    internal.r_type = howto->type;					\
    internal.r_size =							\
      ((howto->complain_on_overflow == complain_overflow_signed		\
	? 0x80								\
	: 0)								\
       | (howto->bitsize - 1));						\
  }

#define COFF_DEFAULT_SECTION_ALIGNMENT_POWER (3)
#define COFF_LONG_FILENAMES
#define NO_COFF_SYMBOLS
#define RTYPE2HOWTO(cache_ptr, dst) xcoff64_rtype2howto (cache_ptr, dst)
#define coff_mkobject _bfd_xcoff_mkobject
#define coff_bfd_copy_private_bfd_data _bfd_xcoff_copy_private_bfd_data
#define coff_bfd_is_local_label_name _bfd_xcoff_is_local_label_name
#define coff_bfd_reloc_type_lookup xcoff64_reloc_type_lookup
#define coff_bfd_reloc_name_lookup xcoff64_reloc_name_lookup
#ifdef AIX_CORE
extern bfd_cleanup rs6000coff_core_p
  (bfd *abfd);
extern bool rs6000coff_core_file_matches_executable_p
  (bfd *cbfd, bfd *ebfd);
extern char *rs6000coff_core_file_failing_command
  (bfd *abfd);
extern int rs6000coff_core_file_failing_signal
  (bfd *abfd);
#define CORE_FILE_P rs6000coff_core_p
#define coff_core_file_failing_command \
  rs6000coff_core_file_failing_command
#define coff_core_file_failing_signal \
  rs6000coff_core_file_failing_signal
#define coff_core_file_matches_executable_p \
  rs6000coff_core_file_matches_executable_p
#define coff_core_file_pid \
  _bfd_nocore_core_file_pid
#else
#define CORE_FILE_P _bfd_dummy_target
#define coff_core_file_failing_command \
  _bfd_nocore_core_file_failing_command
#define coff_core_file_failing_signal \
  _bfd_nocore_core_file_failing_signal
#define coff_core_file_matches_executable_p \
  _bfd_nocore_core_file_matches_executable_p
#define coff_core_file_pid \
  _bfd_nocore_core_file_pid
#endif
#define coff_SWAP_sym_in _bfd_xcoff64_swap_sym_in
#define coff_SWAP_sym_out _bfd_xcoff64_swap_sym_out
#define coff_SWAP_aux_in _bfd_xcoff64_swap_aux_in
#define coff_SWAP_aux_out _bfd_xcoff64_swap_aux_out
#define coff_swap_reloc_in xcoff64_swap_reloc_in
#define coff_swap_reloc_out xcoff64_swap_reloc_out
#define NO_COFF_RELOCS

#ifndef bfd_pe_print_pdata
#define bfd_pe_print_pdata	NULL
#endif

#include "coffcode.h"

/* For XCOFF64, the effective width of symndx changes depending on
   whether we are the first entry.  Sigh.  */
static void
_bfd_xcoff64_swap_lineno_in (bfd *abfd, void *ext1, void *in1)
{
  LINENO *ext = (LINENO *) ext1;
  struct internal_lineno *in = (struct internal_lineno *) in1;

  in->l_lnno = H_GET_32 (abfd, (ext->l_lnno));
  if (in->l_lnno == 0)
    in->l_addr.l_symndx = H_GET_32 (abfd, ext->l_addr.l_symndx);
  else
    in->l_addr.l_paddr = H_GET_64 (abfd, ext->l_addr.l_paddr);
}

static unsigned int
_bfd_xcoff64_swap_lineno_out (bfd *abfd, void *inp, void *outp)
{
  struct internal_lineno *in = (struct internal_lineno *) inp;
  struct external_lineno *ext = (struct external_lineno *) outp;

  H_PUT_32 (abfd, in->l_addr.l_symndx, ext->l_addr.l_symndx);
  H_PUT_32 (abfd, in->l_lnno, (ext->l_lnno));

  if (in->l_lnno == 0)
    H_PUT_32 (abfd, in->l_addr.l_symndx, ext->l_addr.l_symndx);
  else
    H_PUT_64 (abfd, in->l_addr.l_paddr, ext->l_addr.l_paddr);

  return bfd_coff_linesz (abfd);
}

static void
_bfd_xcoff64_swap_sym_in (bfd *abfd, void *ext1, void *in1)
{
  struct external_syment *ext = (struct external_syment *) ext1;
  struct internal_syment *in = (struct internal_syment *) in1;

  in->_n._n_n._n_zeroes = 0;
  in->_n._n_n._n_offset = H_GET_32 (abfd, ext->e_offset);
  in->n_value = H_GET_64 (abfd, ext->e_value);
  in->n_scnum = (short) H_GET_16 (abfd, ext->e_scnum);
  in->n_type = H_GET_16 (abfd, ext->e_type);
  in->n_sclass = H_GET_8 (abfd, ext->e_sclass);
  in->n_numaux = H_GET_8 (abfd, ext->e_numaux);
}

static unsigned int
_bfd_xcoff64_swap_sym_out (bfd *abfd, void *inp, void *extp)
{
  struct internal_syment *in = (struct internal_syment *) inp;
  struct external_syment *ext = (struct external_syment *) extp;

  H_PUT_32 (abfd, in->_n._n_n._n_offset, ext->e_offset);
  H_PUT_64 (abfd, in->n_value, ext->e_value);
  H_PUT_16 (abfd, in->n_scnum, ext->e_scnum);
  H_PUT_16 (abfd, in->n_type, ext->e_type);
  H_PUT_8 (abfd, in->n_sclass, ext->e_sclass);
  H_PUT_8 (abfd, in->n_numaux, ext->e_numaux);
  return bfd_coff_symesz (abfd);
}

static void
_bfd_xcoff64_swap_aux_in (bfd *abfd, void *ext1, int type ATTRIBUTE_UNUSED,
			  int in_class, int indx, int numaux, void *in1)
{
  union external_auxent *ext = (union external_auxent *) ext1;
  union internal_auxent *in = (union internal_auxent *) in1;
  unsigned char auxtype;

  switch (in_class)
    {
    default:
      _bfd_error_handler
	/* xgettext: c-format */
	(_("%pB: unsupported swap_aux_in for storage class %#x"),
	 abfd, (unsigned int) in_class);
      bfd_set_error (bfd_error_bad_value);
      break;

    case C_FILE:
      auxtype = H_GET_8 (abfd, ext->x_file.x_auxtype);
      if (auxtype != _AUX_FILE)
	goto error;

      if (ext->x_file.x_n.x_n.x_zeroes[0] == 0)
	{
	  in->x_file.x_n.x_n.x_zeroes = 0;
	  in->x_file.x_n.x_n.x_offset =
	    H_GET_32 (abfd, ext->x_file.x_n.x_n.x_offset);
	}
      else
	memcpy (in->x_file.x_n.x_fname, ext->x_file.x_n.x_fname, FILNMLEN);
      in->x_file.x_ftype = H_GET_8 (abfd, ext->x_file.x_ftype);
      break;

      /* RS/6000 "csect" auxents.
         There is always a CSECT auxiliary entry. But functions can
         have FCN and EXCEPT ones too. In this case, CSECT is always the last
         one.
         For now, we only support FCN types.  */
    case C_EXT:
    case C_AIX_WEAKEXT:
    case C_HIDEXT:
      if (indx + 1 == numaux)
	{
	  /* C_EXT can have several aux enties. But the _AUX_CSECT is always
	     the last one.  */
	  auxtype = H_GET_8 (abfd, ext->x_csect.x_auxtype);
	  if (auxtype != _AUX_CSECT)
	    goto error;

	  bfd_vma h = H_GET_32 (abfd, ext->x_csect.x_scnlen_hi);
	  bfd_vma l = H_GET_32 (abfd, ext->x_csect.x_scnlen_lo);

	  in->x_csect.x_scnlen.u64 = h << 32 | (l & 0xffffffff);

	  in->x_csect.x_parmhash = H_GET_32 (abfd, ext->x_csect.x_parmhash);
	  in->x_csect.x_snhash = H_GET_16 (abfd, ext->x_csect.x_snhash);
	  /* We don't have to hack bitfields in x_smtyp because it's
	     defined by shifts-and-ands, which are equivalent on all
	     byte orders.  */
	  in->x_csect.x_smtyp = H_GET_8 (abfd, ext->x_csect.x_smtyp);
	  in->x_csect.x_smclas = H_GET_8 (abfd, ext->x_csect.x_smclas);
	}
      else
	{
	  /* It can also be a _AUX_EXCEPT entry. But it's not supported
	     for now. */
	  auxtype = H_GET_8 (abfd, ext->x_fcn.x_auxtype);
	  if (auxtype != _AUX_FCN)
	    goto error;

	  in->x_sym.x_fcnary.x_fcn.x_lnnoptr
	    = H_GET_64 (abfd, ext->x_fcn.x_lnnoptr);
	  in->x_sym.x_misc.x_fsize
	    = H_GET_32 (abfd, ext->x_fcn.x_fsize);
	  in->x_sym.x_fcnary.x_fcn.x_endndx.u32
	    = H_GET_32 (abfd, ext->x_fcn.x_endndx);
	}
      break;

    case C_STAT:
      _bfd_error_handler
	/* xgettext: c-format */
	(_("%pB: C_STAT isn't supported by XCOFF64"),
	 abfd);
      bfd_set_error (bfd_error_bad_value);
      break;

    case C_BLOCK:
    case C_FCN:
      auxtype = H_GET_8 (abfd, ext->x_sym.x_auxtype);
      if (auxtype != _AUX_SYM)
	goto error;

      in->x_sym.x_misc.x_lnsz.x_lnno
	= H_GET_32 (abfd, ext->x_sym.x_lnno);
      break;

    case C_DWARF:
      auxtype = H_GET_8 (abfd, ext->x_sect.x_auxtype);
      if (auxtype != _AUX_SECT)
	goto error;

      in->x_sect.x_scnlen = H_GET_64 (abfd, ext->x_sect.x_scnlen);
      in->x_sect.x_nreloc = H_GET_64 (abfd, ext->x_sect.x_nreloc);
      break;
    }

  return;

 error:
  _bfd_error_handler
    /* xgettext: c-format */
    (_("%pB: wrong auxtype %#x for storage class %#x"),
     abfd, auxtype, (unsigned int) in_class);
  bfd_set_error (bfd_error_bad_value);


}

static unsigned int
_bfd_xcoff64_swap_aux_out (bfd *abfd, void *inp, int type ATTRIBUTE_UNUSED,
			   int in_class, int indx, int numaux, void *extp)
{
  union internal_auxent *in = (union internal_auxent *) inp;
  union external_auxent *ext = (union external_auxent *) extp;

  memset (ext, 0, bfd_coff_auxesz (abfd));
  switch (in_class)
    {
    default:
      _bfd_error_handler
	/* xgettext: c-format */
	(_("%pB: unsupported swap_aux_out for storage class %#x"),
	 abfd, (unsigned int) in_class);
      bfd_set_error (bfd_error_bad_value);
      break;

    case C_FILE:
      if (in->x_file.x_n.x_n.x_zeroes == 0)
	{
	  H_PUT_32 (abfd, 0, ext->x_file.x_n.x_n.x_zeroes);
	  H_PUT_32 (abfd, in->x_file.x_n.x_n.x_offset,
		    ext->x_file.x_n.x_n.x_offset);
	}
      else
	memcpy (ext->x_file.x_n.x_fname, in->x_file.x_n.x_fname, FILNMLEN);
      H_PUT_8 (abfd, in->x_file.x_ftype, ext->x_file.x_ftype);
      H_PUT_8 (abfd, _AUX_FILE, ext->x_file.x_auxtype);
      break;

      /* RS/6000 "csect" auxents.
         There is always a CSECT auxiliary entry. But functions can
         have FCN and EXCEPT ones too. In this case, CSECT is always the last
         one.
         For now, we only support FCN types.  */
    case C_EXT:
    case C_AIX_WEAKEXT:
    case C_HIDEXT:
      if (indx + 1 == numaux)
	{
	  bfd_vma temp;

	  temp = in->x_csect.x_scnlen.u64 & 0xffffffff;
	  H_PUT_32 (abfd, temp, ext->x_csect.x_scnlen_lo);
	  temp = in->x_csect.x_scnlen.u64 >> 32;
	  H_PUT_32 (abfd, temp, ext->x_csect.x_scnlen_hi);
	  H_PUT_32 (abfd, in->x_csect.x_parmhash, ext->x_csect.x_parmhash);
	  H_PUT_16 (abfd, in->x_csect.x_snhash, ext->x_csect.x_snhash);
	  /* We don't have to hack bitfields in x_smtyp because it's
	     defined by shifts-and-ands, which are equivalent on all
	     byte orders.  */
	  H_PUT_8 (abfd, in->x_csect.x_smtyp, ext->x_csect.x_smtyp);
	  H_PUT_8 (abfd, in->x_csect.x_smclas, ext->x_csect.x_smclas);
	  H_PUT_8 (abfd, _AUX_CSECT, ext->x_csect.x_auxtype);
	}
      else
	{
	  H_PUT_64 (abfd, in->x_sym.x_fcnary.x_fcn.x_lnnoptr,
		    ext->x_fcn.x_lnnoptr);
	  H_PUT_32 (abfd, in->x_sym.x_misc.x_fsize, ext->x_fcn.x_fsize);
	  H_PUT_32 (abfd, in->x_sym.x_fcnary.x_fcn.x_endndx.u32,
		    ext->x_fcn.x_endndx);
	  H_PUT_8 (abfd, _AUX_FCN, ext->x_csect.x_auxtype);
	}
      break;

    case C_STAT:
      _bfd_error_handler
	/* xgettext: c-format */
	(_("%pB: C_STAT isn't supported by XCOFF64"),
	 abfd);
      bfd_set_error (bfd_error_bad_value);
      break;

    case C_BLOCK:
    case C_FCN:
      H_PUT_32 (abfd, in->x_sym.x_misc.x_lnsz.x_lnno, ext->x_sym.x_lnno);
      H_PUT_8 (abfd, _AUX_SYM, ext->x_sym.x_auxtype);
      break;

    case C_DWARF:
      H_PUT_64 (abfd, in->x_sect.x_scnlen, ext->x_sect.x_scnlen);
      H_PUT_64 (abfd, in->x_sect.x_nreloc, ext->x_sect.x_nreloc);
      H_PUT_8 (abfd, _AUX_SECT, ext->x_sect.x_auxtype);
      break;
    }

  return bfd_coff_auxesz (abfd);
}

static bool
_bfd_xcoff64_put_symbol_name (struct bfd_link_info *info,
			      struct bfd_strtab_hash *strtab,
			      struct internal_syment *sym,
			      const char *name)
{
  bool hash;
  bfd_size_type indx;

  hash = !info->traditional_format;
  indx = _bfd_stringtab_add (strtab, name, hash, false);

  if (indx == (bfd_size_type) -1)
    return false;

  sym->_n._n_n._n_zeroes = 0;
  sym->_n._n_n._n_offset = STRING_SIZE_SIZE + indx;

  return true;
}

static bool
_bfd_xcoff64_put_ldsymbol_name (bfd *abfd ATTRIBUTE_UNUSED,
				struct xcoff_loader_info *ldinfo,
				struct internal_ldsym *ldsym,
				const char *name)
{
  size_t len;
  len = strlen (name);

  if (ldinfo->string_size + len + 3 > ldinfo->string_alc)
    {
      bfd_size_type newalc;
      char *newstrings;

      newalc = ldinfo->string_alc * 2;
      if (newalc == 0)
	newalc = 32;
      while (ldinfo->string_size + len + 3 > newalc)
	newalc *= 2;

      newstrings = bfd_realloc (ldinfo->strings, newalc);
      if (newstrings == NULL)
	{
	  ldinfo->failed = true;
	  return false;
	}
      ldinfo->string_alc = newalc;
      ldinfo->strings = newstrings;
    }

  ldinfo->strings[ldinfo->string_size] = ((len + 1) >> 8) & 0xff;
  ldinfo->strings[ldinfo->string_size + 1] = ((len + 1)) & 0xff;
  strcpy (ldinfo->strings + ldinfo->string_size + 2, name);
  ldsym->_l._l_l._l_zeroes = 0;
  ldsym->_l._l_l._l_offset = ldinfo->string_size + 2;
  ldinfo->string_size += len + 3;

  return true;
}

/* Routines to swap information in the XCOFF .loader section.  If we
   ever need to write an XCOFF loader, this stuff will need to be
   moved to another file shared by the linker (which XCOFF calls the
   ``binder'') and the loader.  */

/* Swap in the ldhdr structure.  */

static void
xcoff64_swap_ldhdr_in (bfd *abfd,
		       const void *s,
		       struct internal_ldhdr *dst)
{
  const struct external_ldhdr *src = (const struct external_ldhdr *) s;

  dst->l_version = bfd_get_32 (abfd, src->l_version);
  dst->l_nsyms = bfd_get_32 (abfd, src->l_nsyms);
  dst->l_nreloc = bfd_get_32 (abfd, src->l_nreloc);
  dst->l_istlen = bfd_get_32 (abfd, src->l_istlen);
  dst->l_nimpid = bfd_get_32 (abfd, src->l_nimpid);
  dst->l_stlen = bfd_get_32 (abfd, src->l_stlen);
  dst->l_impoff = bfd_get_64 (abfd, src->l_impoff);
  dst->l_stoff = bfd_get_64 (abfd, src->l_stoff);
  dst->l_symoff = bfd_get_64 (abfd, src->l_symoff);
  dst->l_rldoff = bfd_get_64 (abfd, src->l_rldoff);
}

/* Swap out the ldhdr structure.  */

static void
xcoff64_swap_ldhdr_out (bfd *abfd, const struct internal_ldhdr *src, void *d)
{
  struct external_ldhdr *dst = (struct external_ldhdr *) d;

  bfd_put_32 (abfd, (bfd_vma) src->l_version, dst->l_version);
  bfd_put_32 (abfd, src->l_nsyms, dst->l_nsyms);
  bfd_put_32 (abfd, src->l_nreloc, dst->l_nreloc);
  bfd_put_32 (abfd, src->l_istlen, dst->l_istlen);
  bfd_put_32 (abfd, src->l_nimpid, dst->l_nimpid);
  bfd_put_32 (abfd, src->l_stlen, dst->l_stlen);
  bfd_put_64 (abfd, src->l_impoff, dst->l_impoff);
  bfd_put_64 (abfd, src->l_stoff, dst->l_stoff);
  bfd_put_64 (abfd, src->l_symoff, dst->l_symoff);
  bfd_put_64 (abfd, src->l_rldoff, dst->l_rldoff);
}

/* Swap in the ldsym structure.  */

static void
xcoff64_swap_ldsym_in (bfd *abfd, const void *s, struct internal_ldsym *dst)
{
  const struct external_ldsym *src = (const struct external_ldsym *) s;
  /* XCOFF64 does not use l_zeroes like XCOFF32
     Set the internal l_zeroes to 0 so the common 32/64 code uses l_value
     as an offset into the loader symbol table.  */
  dst->_l._l_l._l_zeroes = 0;
  dst->_l._l_l._l_offset = bfd_get_32 (abfd, src->l_offset);
  dst->l_value = bfd_get_64 (abfd, src->l_value);
  dst->l_scnum = bfd_get_16 (abfd, src->l_scnum);
  dst->l_smtype = bfd_get_8 (abfd, src->l_smtype);
  dst->l_smclas = bfd_get_8 (abfd, src->l_smclas);
  dst->l_ifile = bfd_get_32 (abfd, src->l_ifile);
  dst->l_parm = bfd_get_32 (abfd, src->l_parm);
}

/* Swap out the ldsym structure.  */

static void
xcoff64_swap_ldsym_out (bfd *abfd, const struct internal_ldsym *src, void *d)
{
  struct external_ldsym *dst = (struct external_ldsym *) d;

  bfd_put_64 (abfd, src->l_value, dst->l_value);
  bfd_put_32 (abfd, (bfd_vma) src->_l._l_l._l_offset, dst->l_offset);
  bfd_put_16 (abfd, (bfd_vma) src->l_scnum, dst->l_scnum);
  bfd_put_8 (abfd, src->l_smtype, dst->l_smtype);
  bfd_put_8 (abfd, src->l_smclas, dst->l_smclas);
  bfd_put_32 (abfd, src->l_ifile, dst->l_ifile);
  bfd_put_32 (abfd, src->l_parm, dst->l_parm);
}

static void
xcoff64_swap_reloc_in (bfd *abfd, void *s, void *d)
{
  struct external_reloc *src = (struct external_reloc *) s;
  struct internal_reloc *dst = (struct internal_reloc *) d;

  memset (dst, 0, sizeof (struct internal_reloc));

  dst->r_vaddr = bfd_get_64 (abfd, src->r_vaddr);
  dst->r_symndx = bfd_get_32 (abfd, src->r_symndx);
  dst->r_size = bfd_get_8 (abfd, src->r_size);
  dst->r_type = bfd_get_8 (abfd, src->r_type);
}

static unsigned int
xcoff64_swap_reloc_out (bfd *abfd, void *s, void *d)
{
  struct internal_reloc *src = (struct internal_reloc *) s;
  struct external_reloc *dst = (struct external_reloc *) d;

  bfd_put_64 (abfd, src->r_vaddr, dst->r_vaddr);
  bfd_put_32 (abfd, src->r_symndx, dst->r_symndx);
  bfd_put_8 (abfd, src->r_type, dst->r_type);
  bfd_put_8 (abfd, src->r_size, dst->r_size);

  return bfd_coff_relsz (abfd);
}

/* Swap in the ldrel structure.  */

static void
xcoff64_swap_ldrel_in (bfd *abfd, const void *s, struct internal_ldrel *dst)
{
  const struct external_ldrel *src = (const struct external_ldrel *) s;

  dst->l_vaddr = bfd_get_64 (abfd, src->l_vaddr);
  dst->l_symndx = bfd_get_32 (abfd, src->l_symndx);
  dst->l_rtype = bfd_get_16 (abfd, src->l_rtype);
  dst->l_rsecnm = bfd_get_16 (abfd, src->l_rsecnm);
}

/* Swap out the ldrel structure.  */

static void
xcoff64_swap_ldrel_out (bfd *abfd, const struct internal_ldrel *src, void *d)
{
  struct external_ldrel *dst = (struct external_ldrel *) d;

  bfd_put_64 (abfd, src->l_vaddr, dst->l_vaddr);
  bfd_put_16 (abfd, (bfd_vma) src->l_rtype, dst->l_rtype);
  bfd_put_16 (abfd, (bfd_vma) src->l_rsecnm, dst->l_rsecnm);
  bfd_put_32 (abfd, src->l_symndx, dst->l_symndx);
}


static bool
xcoff64_reloc_type_br (bfd *input_bfd,
		       asection *input_section,
		       bfd *output_bfd ATTRIBUTE_UNUSED,
		       struct internal_reloc *rel,
		       struct internal_syment *sym ATTRIBUTE_UNUSED,
		       struct reloc_howto_struct *howto,
		       bfd_vma val,
		       bfd_vma addend,
		       bfd_vma *relocation,
		       bfd_byte *contents,
		       struct bfd_link_info *info)
{
  struct xcoff_link_hash_entry *h;
  bfd_vma section_offset;
  struct xcoff_stub_hash_entry *stub_entry = NULL;
  enum xcoff_stub_type stub_type;

  if (0 > rel->r_symndx)
    return false;

  h = obj_xcoff_sym_hashes (input_bfd)[rel->r_symndx];
  section_offset = rel->r_vaddr - input_section->vma;

  /* If we see an R_BR or R_RBR reloc which is jumping to global
     linkage code, and it is followed by an appropriate cror nop
     instruction, we replace the cror with ld r2,40(r1).  This
     restores the TOC after the glink code.  Contrariwise, if the
     call is followed by a ld r2,40(r1), but the call is not
     going to global linkage code, we can replace the load with a
     cror.  */
  if (NULL != h
      && (bfd_link_hash_defined == h->root.type
	  || bfd_link_hash_defweak == h->root.type)
      && section_offset + 8 <= input_section->size)
    {
      bfd_byte *pnext;
      unsigned long next;

      pnext = contents + section_offset + 4;
      next = bfd_get_32 (input_bfd, pnext);

      /* The _ptrgl function is magic.  It is used by the AIX compiler to call
	 a function through a pointer.  */
      if (h->smclas == XMC_GL || strcmp (h->root.root.string, "._ptrgl") == 0)
	{
	  if (next == 0x4def7b82			/* cror 15,15,15  */
	      || next == 0x4ffffb82			/* cror 31,31,31  */
	      || next == 0x60000000)			/* ori	r0,r0,0	  */
	    bfd_put_32 (input_bfd, 0xe8410028, pnext);	/* ld	r2,40(r1) */
	}
      else
	{
	  if (next == 0xe8410028)			/* ld r2,40(r1)	  */
	    bfd_put_32 (input_bfd, 0x60000000, pnext);	/* ori r0,r0,0	  */
	}
    }
  else if (NULL != h && bfd_link_hash_undefined == h->root.type)
    {
      /* Normally, this relocation is against a defined symbol.  In the
	 case where this is a partial link and the output section offset
	 is greater than 2^25, the linker will return an invalid error
	 message that the relocation has been truncated.  Yes it has been
	 truncated but no it not important.  For this case, disable the
	 overflow checking. */
      howto->complain_on_overflow = complain_overflow_dont;
    }

  /* Check if a stub is needed.  */
  stub_type = bfd_xcoff_type_of_stub (input_section, rel, val, h);
  if (stub_type != xcoff_stub_none)
    {
      asection *stub_csect;

      stub_entry = bfd_xcoff_get_stub_entry (input_section, h, info);
      if (stub_entry == NULL)
	{
	  _bfd_error_handler (_("Unable to find the stub entry targeting %s"),
			      h->root.root.string);
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}

      stub_csect = stub_entry->hcsect->root.u.def.section;
      val = (stub_entry->stub_offset
	     + stub_csect->output_section->vma
	     + stub_csect->output_offset);
    }

  /* The original PC-relative relocation is biased by -r_vaddr, so adding
     the value below will give the absolute target address.  */
  *relocation = val + addend + rel->r_vaddr;

  howto->src_mask &= ~3;
  howto->dst_mask = howto->src_mask;

  if (h != NULL
      && (h->root.type == bfd_link_hash_defined
	  || h->root.type == bfd_link_hash_defweak)
      && bfd_is_abs_section (h->root.u.def.section)
      && section_offset + 4 <= input_section->size)
    {
      bfd_byte *ptr;
      bfd_vma insn;

      /* Turn the relative branch into an absolute one by setting the
	 AA bit.  */
      ptr = contents + section_offset;
      insn = bfd_get_32 (input_bfd, ptr);
      insn |= 2;
      bfd_put_32 (input_bfd, insn, ptr);

      /* Make the howto absolute too.  */
      howto->pc_relative = false;
      howto->complain_on_overflow = complain_overflow_bitfield;
    }
  else
    {
      /* Use a PC-relative howto and subtract the instruction's address
	 from the target address we calculated above.  */
      howto->pc_relative = true;
      *relocation -= (input_section->output_section->vma
		      + input_section->output_offset
		      + section_offset);
    }
  return true;
}



/* The XCOFF reloc table.
   Cf xcoff_howto_table comments.  */

reloc_howto_type xcoff64_howto_table[] =
{
  /* 0x00: Standard 64 bit relocation.  */
  HOWTO (R_POS,			/* type */
	 0,			/* rightshift */
	 8,			/* size */
	 64,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 0,			/* special_function */
	 "R_POS_64",		/* name */
	 true,			/* partial_inplace */
	 MINUS_ONE,		/* src_mask */
	 MINUS_ONE,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 0x01: 64 bit relocation, but store negative value.  */
  HOWTO (R_NEG,			/* type */
	 0,			/* rightshift */
	 -8,			/* size */
	 64,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 0,			/* special_function */
	 "R_NEG",		/* name */
	 true,			/* partial_inplace */
	 MINUS_ONE,		/* src_mask */
	 MINUS_ONE,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 0x02: 64 bit PC relative relocation.  */
  HOWTO (R_REL,			/* type */
	 0,			/* rightshift */
	 8,			/* size */
	 64,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 0,			/* special_function */
	 "R_REL",		/* name */
	 true,			/* partial_inplace */
	 MINUS_ONE,		/* src_mask */
	 MINUS_ONE,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 0x03: 16 bit TOC relative relocation.  */
  HOWTO (R_TOC,			/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 0,			/* special_function */
	 "R_TOC",		/* name */
	 true,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 0x04: Same as R_TOC.  */
  HOWTO (R_TRL,			/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 0,			/* special_function */
	 "R_TRL",		/* name */
	 true,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 0x05: External TOC relative symbol.  */
  HOWTO (R_GL,			/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 0,			/* special_function */
	 "R_GL",		/* name */
	 true,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 0x06: Local TOC relative symbol.	 */
  HOWTO (R_TCL,			/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 0,			/* special_function */
	 "R_TCL",		/* name */
	 true,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  EMPTY_HOWTO (7),

  /* 0x08: Same as R_RBA.  */
  HOWTO (R_BA,			/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 26,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 0,			/* special_function */
	 "R_BA_26",		/* name */
	 true,			/* partial_inplace */
	 0x03fffffc,		/* src_mask */
	 0x03fffffc,		/* dst_mask */
	 false),		/* pcrel_offset */

  EMPTY_HOWTO (9),

  /* 0x0a: Same as R_RBR.  */
  HOWTO (R_BR,			/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 26,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 0,			/* special_function */
	 "R_BR",		/* name */
	 true,			/* partial_inplace */
	 0x03fffffc,		/* src_mask */
	 0x03fffffc,		/* dst_mask */
	 false),		/* pcrel_offset */

  EMPTY_HOWTO (0xb),

  /* 0x0c: Same as R_POS.  */
  HOWTO (R_RL,			/* type */
	 0,			/* rightshift */
	 8,			/* size */
	 64,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 0,			/* special_function */
	 "R_RL",		/* name */
	 true,			/* partial_inplace */
	 MINUS_ONE,		/* src_mask */
	 MINUS_ONE,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 0x0d: Same as R_POS.  */
  HOWTO (R_RLA,			/* type */
	 0,			/* rightshift */
	 8,			/* size */
	 64,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 0,			/* special_function */
	 "R_RLA",		/* name */
	 true,			/* partial_inplace */
	 MINUS_ONE,		/* src_mask */
	 MINUS_ONE,		/* dst_mask */
	 false),		/* pcrel_offset */

  EMPTY_HOWTO (0xe),

  /* 0x0f: Non-relocating reference.  Bitsize is 1 so that r_rsize is 0.  */
  HOWTO (R_REF,			/* type */
	 0,			/* rightshift */
	 1,			/* size */
	 1,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 0,			/* special_function */
	 "R_REF",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),		/* pcrel_offset */

  EMPTY_HOWTO (0x10),
  EMPTY_HOWTO (0x11),
  EMPTY_HOWTO (0x12),

  /* 0x13: Same as R_TOC  */
  HOWTO (R_TRLA,		/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 0,			/* special_function */
	 "R_TRLA",		/* name */
	 true,			/* partial_inplace */
	 0xffff,		/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 0x14: Modifiable relative branch.  */
  HOWTO (R_RRTBI,		/* type */
	 1,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 0,			/* special_function */
	 "R_RRTBI",		/* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 0x15: Modifiable absolute branch.  */
  HOWTO (R_RRTBA,		/* type */
	 1,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 0,			/* special_function */
	 "R_RRTBA",		/* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 0x16: Modifiable call absolute indirect.  */
  HOWTO (R_CAI,			/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 0,			/* special_function */
	 "R_CAI",		/* name */
	 true,			/* partial_inplace */
	 0xffff,		/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 0x17: Modifiable call relative.  */
  HOWTO (R_CREL,		/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 0,			/* special_function */
	 "R_CREL",		/* name */
	 true,			/* partial_inplace */
	 0xffff,		/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 0x18: Modifiable branch absolute.  */
  HOWTO (R_RBA,			/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 26,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 0,			/* special_function */
	 "R_RBA",		/* name */
	 true,			/* partial_inplace */
	 0x03fffffc,		/* src_mask */
	 0x03fffffc,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 0x19: Modifiable branch absolute.  */
  HOWTO (R_RBAC,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 0,			/* special_function */
	 "R_RBAC",		/* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 0x1a: Modifiable branch relative.  */
  HOWTO (R_RBR,			/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 26,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 0,			/* special_function */
	 "R_RBR_26",		/* name */
	 true,			/* partial_inplace */
	 0x03fffffc,		/* src_mask */
	 0x03fffffc,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 0x1b: Modifiable branch absolute.  */
  HOWTO (R_RBRC,		/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 0,			/* special_function */
	 "R_RBRC",		/* name */
	 true,			/* partial_inplace */
	 0xffff,		/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 0x1c: Standard 32 bit relocation.  */
  HOWTO (R_POS,			/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 0,			/* special_function */
	 "R_POS_32",		/* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 0x1d: 16 bit Non modifiable absolute branch.  */
  HOWTO (R_BA,			/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 0,			/* special_function */
	 "R_BA_16",		/* name */
	 true,			/* partial_inplace */
	 0xfffc,		/* src_mask */
	 0xfffc,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 0x1e: Modifiable branch relative.  */
  HOWTO (R_RBR,			/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 0,			/* special_function */
	 "R_RBR_16",		/* name */
	 true,			/* partial_inplace */
	 0xfffc,		/* src_mask */
	 0xfffc,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 0x1f: Modifiable branch absolute.  */
  HOWTO (R_RBA,			/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 0,			/* special_function */
	 "R_RBA_16",		/* name */
	 true,			/* partial_inplace */
	 0xffff,		/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 0x20: General-dynamic TLS relocation.  */
  HOWTO (R_TLS,			/* type */
	 0,			/* rightshift */
	 8,			/* size */
	 64,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 0,			/* special_function */
	 "R_TLS",		/* name */
	 true,			/* partial_inplace */
	 MINUS_ONE,		/* src_mask */
	 MINUS_ONE,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 0x21: Initial-exec TLS relocation.  */
  HOWTO (R_TLS_IE,		/* type */
	 0,			/* rightshift */
	 8,			/* size */
	 64,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 0,			/* special_function */
	 "R_TLS_IE",		/* name */
	 true,			/* partial_inplace */
	 MINUS_ONE,		/* src_mask */
	 MINUS_ONE,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 0x22: Local-dynamic TLS relocation.  */
  HOWTO (R_TLS_LD,		/* type */
	 0,			/* rightshift */
	 8,			/* size */
	 64,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 0,			/* special_function */
	 "R_TLS_LD",		/* name */
	 true,			/* partial_inplace */
	 MINUS_ONE,		/* src_mask */
	 MINUS_ONE,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 0x23: Local-exec TLS relocation.  */
  HOWTO (R_TLS_LE,		/* type */
	 0,			/* rightshift */
	 8,			/* size */
	 64,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 0,			/* special_function */
	 "R_TLS_LE",		/* name */
	 true,			/* partial_inplace */
	 MINUS_ONE,		/* src_mask */
	 MINUS_ONE,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 0x24: TLS relocation.  */
  HOWTO (R_TLSM,		/* type */
	 0,			/* rightshift */
	 8,			/* size */
	 64,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 0,			/* special_function */
	 "R_TLSM",		/* name */
	 true,			/* partial_inplace */
	 MINUS_ONE,		/* src_mask */
	 MINUS_ONE,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 0x25: TLS module relocation.  */
  HOWTO (R_TLSML,		/* type */
	 0,			/* rightshift */
	 8,			/* size */
	 64,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 0,			/* special_function */
	 "R_TLSML",		/* name */
	 true,			/* partial_inplace */
	 MINUS_ONE,		/* src_mask */
	 MINUS_ONE,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 0x26: 32 bit relocation, but store negative value.  */
  HOWTO (R_NEG,			/* type */
	 0,			/* rightshift */
	 -4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 0,			/* special_function */
	 "R_NEG_32",		/* name */
	 true,			/* partial_inplace */
	 MINUS_ONE,		/* src_mask */
	 MINUS_ONE,		/* dst_mask */
	 false),		/* pcrel_offset */

  EMPTY_HOWTO(0x27),
  EMPTY_HOWTO(0x28),
  EMPTY_HOWTO(0x29),
  EMPTY_HOWTO(0x2a),
  EMPTY_HOWTO(0x2b),
  EMPTY_HOWTO(0x2c),
  EMPTY_HOWTO(0x2d),
  EMPTY_HOWTO(0x2e),
  EMPTY_HOWTO(0x2f),

  HOWTO (R_TOCU,		/* type */
	 16,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 0,			/* special_function */
	 "R_TOCU",		/* name */
	 true,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 0x31: Low-order 16 bit TOC relative relocation.  */
  HOWTO (R_TOCL,		/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 0,			/* special_function */
	 "R_TOCL",		/* name */
	 true,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

};

void
xcoff64_rtype2howto (arelent *relent, struct internal_reloc *internal)
{
  if (internal->r_type > R_TOCL)
    abort ();

  /* Default howto layout works most of the time */
  relent->howto = &xcoff64_howto_table[internal->r_type];

  /* Special case some 16 bit reloc */
  if (15 == (internal->r_size & 0x3f))
    {
      if (R_BA == internal->r_type)
	relent->howto = &xcoff64_howto_table[0x1d];
      else if (R_RBR == internal->r_type)
	relent->howto = &xcoff64_howto_table[0x1e];
      else if (R_RBA == internal->r_type)
	relent->howto = &xcoff64_howto_table[0x1f];
    }
  /* Special case 32 bit */
  else if (31 == (internal->r_size & 0x3f))
    {
      if (R_POS == internal->r_type)
	relent->howto = &xcoff64_howto_table[0x1c];

      if (R_NEG == internal->r_type)
	relent->howto = &xcoff64_howto_table[0x26];
    }

  /* The r_size field of an XCOFF reloc encodes the bitsize of the
     relocation, as well as indicating whether it is signed or not.
     Doublecheck that the relocation information gathered from the
     type matches this information.  The bitsize is not significant
     for R_REF relocs.  */
  if (relent->howto->dst_mask != 0
      && (relent->howto->bitsize
	  != ((unsigned int) internal->r_size & 0x3f) + 1))
    abort ();
}

reloc_howto_type *
xcoff64_reloc_type_lookup (bfd *abfd ATTRIBUTE_UNUSED,
			   bfd_reloc_code_real_type code)
{
  switch (code)
    {
    case BFD_RELOC_PPC_B26:
      return &xcoff64_howto_table[0xa];
    case BFD_RELOC_PPC_BA16:
      return &xcoff64_howto_table[0x1d];
    case BFD_RELOC_PPC_BA26:
      return &xcoff64_howto_table[8];
    case BFD_RELOC_PPC_TOC16:
      return &xcoff64_howto_table[3];
    case BFD_RELOC_PPC_TOC16_HI:
      return &xcoff64_howto_table[0x30];
    case BFD_RELOC_PPC_TOC16_LO:
      return &xcoff64_howto_table[0x31];
    case BFD_RELOC_PPC_B16:
      return &xcoff64_howto_table[0x1e];
    case BFD_RELOC_32:
    case BFD_RELOC_CTOR:
      return &xcoff64_howto_table[0x1c];
    case BFD_RELOC_64:
      return &xcoff64_howto_table[0];
    case BFD_RELOC_NONE:
      return &xcoff64_howto_table[0xf];
    case BFD_RELOC_PPC_NEG:
      return &xcoff64_howto_table[0x1];
    case BFD_RELOC_PPC64_TLSGD:
      return &xcoff64_howto_table[0x20];
    case BFD_RELOC_PPC64_TLSIE:
      return &xcoff64_howto_table[0x21];
    case BFD_RELOC_PPC64_TLSLD:
      return &xcoff64_howto_table[0x22];
    case BFD_RELOC_PPC64_TLSLE:
      return &xcoff64_howto_table[0x23];
    case BFD_RELOC_PPC64_TLSM:
      return &xcoff64_howto_table[0x24];
    case BFD_RELOC_PPC64_TLSML:
      return &xcoff64_howto_table[0x25];
    default:
      return NULL;
    }
}

static reloc_howto_type *
xcoff64_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED,
			   const char *r_name)
{
  unsigned int i;

  for (i = 0;
       i < sizeof (xcoff64_howto_table) / sizeof (xcoff64_howto_table[0]);
       i++)
    if (xcoff64_howto_table[i].name != NULL
	&& strcasecmp (xcoff64_howto_table[i].name, r_name) == 0)
      return &xcoff64_howto_table[i];

  return NULL;
}

/* This is the relocation function for the PowerPC64.
   See xcoff_ppc_relocation_section for more information. */

bool
xcoff64_ppc_relocate_section (bfd *output_bfd,
			      struct bfd_link_info *info,
			      bfd *input_bfd,
			      asection *input_section,
			      bfd_byte *contents,
			      struct internal_reloc *relocs,
			      struct internal_syment *syms,
			      asection **sections)
{
  struct internal_reloc *rel;
  struct internal_reloc *relend;

  rel = relocs;
  relend = rel + input_section->reloc_count;
  for (; rel < relend; rel++)
    {
      long symndx;
      struct xcoff_link_hash_entry *h;
      struct internal_syment *sym;
      bfd_vma addend;
      bfd_vma val;
      struct reloc_howto_struct howto;
      bfd_vma relocation;
      bfd_vma value_to_relocate;
      bfd_vma address;
      bfd_byte *location;

      /* Relocation type R_REF is a special relocation type which is
	 merely used to prevent garbage collection from occurring for
	 the csect including the symbol which it references.  */
      if (rel->r_type == R_REF)
	continue;

      /* Retrieve default value in HOWTO table and fix up according
	 to r_size field, if it can be different.
	 This should be made during relocation reading but the algorithms
	 are expecting constant howtos.  */
      memcpy (&howto, &xcoff64_howto_table[rel->r_type], sizeof (howto));
      if (howto.bitsize != (rel->r_size & 0x3f) + 1)
	{
	  switch (rel->r_type)
	    {
	    case R_POS:
	    case R_NEG:
	      howto.bitsize = (rel->r_size & 0x3f) + 1;
	      howto.size = HOWTO_RSIZE (howto.bitsize <= 16
					? 2 : howto.bitsize <= 32
					? 4 : 8);
	      howto.src_mask = howto.dst_mask = N_ONES (howto.bitsize);
	      break;

	    default:
	      _bfd_error_handler
		(_("%pB: relocation (%d) at (0x%" PRIx64 ") has wrong"
		   " r_rsize (0x%x)\n"),
		 input_bfd, rel->r_type, rel->r_vaddr, rel->r_size);
	      return false;
	    }
	}

      howto.complain_on_overflow = (rel->r_size & 0x80
				    ? complain_overflow_signed
				    : complain_overflow_bitfield);

      /* symbol */
      val = 0;
      addend = 0;
      h = NULL;
      sym = NULL;
      symndx = rel->r_symndx;

      if (-1 != symndx)
	{
	  asection *sec;

	  h = obj_xcoff_sym_hashes (input_bfd)[symndx];
	  sym = syms + symndx;
	  addend = - sym->n_value;

	  if (NULL == h)
	    {
	      sec = sections[symndx];
	      /* Hack to make sure we use the right TOC anchor value
		 if this reloc is against the TOC anchor.  */
	      if (sec->name[3] == '0'
		  && strcmp (sec->name, ".tc0") == 0)
		val = xcoff_data (output_bfd)->toc;
	      else
		val = (sec->output_section->vma
		       + sec->output_offset
		       + sym->n_value
		       - sec->vma);
	    }
	  else
	    {
	      if (info->unresolved_syms_in_objects != RM_IGNORE
		  && (h->flags & XCOFF_WAS_UNDEFINED) != 0)
		info->callbacks->undefined_symbol
		  (info, h->root.root.string, input_bfd, input_section,
		   rel->r_vaddr - input_section->vma,
		   info->unresolved_syms_in_objects == RM_DIAGNOSE
		   && !info->warn_unresolved_syms);

	      if (h->root.type == bfd_link_hash_defined
		  || h->root.type == bfd_link_hash_defweak)
		{
		  sec = h->root.u.def.section;
		  val = (h->root.u.def.value
			 + sec->output_section->vma
			 + sec->output_offset);
		}
	      else if (h->root.type == bfd_link_hash_common)
		{
		  sec = h->root.u.c.p->section;
		  val = (sec->output_section->vma
			 + sec->output_offset);
		}
	      else
		{
		  BFD_ASSERT (bfd_link_relocatable (info)
			      || (h->flags & XCOFF_DEF_DYNAMIC) != 0
			      || (h->flags & XCOFF_IMPORT) != 0);
		}
	    }
	}

      if (rel->r_type >= XCOFF_MAX_CALCULATE_RELOCATION
	  || !((*xcoff64_calculate_relocation[rel->r_type])
	      (input_bfd, input_section, output_bfd, rel, sym, &howto, val,
	       addend, &relocation, contents, info)))
	return false;

      /* address */
      address = rel->r_vaddr - input_section->vma;
      location = contents + address;

      if (address > input_section->size)
	abort ();

      /* Get the value we are going to relocate.  */
      switch (bfd_get_reloc_size (&howto))
	{
	case 2:
	  value_to_relocate = bfd_get_16 (input_bfd, location);
	  break;
	case 4:
	  value_to_relocate = bfd_get_32 (input_bfd, location);
	  break;
	default:
	  value_to_relocate = bfd_get_64 (input_bfd, location);
	  break;
	}

      /* overflow.

	 FIXME: We may drop bits during the addition
	 which we don't check for.  We must either check at every single
	 operation, which would be tedious, or we must do the computations
	 in a type larger than bfd_vma, which would be inefficient.  */

      if (((*xcoff_complain_overflow[howto.complain_on_overflow])
	   (input_bfd, value_to_relocate, relocation, &howto)))
	{
	  const char *name;
	  char buf[SYMNMLEN + 1];
	  char reloc_type_name[10];

	  if (symndx == -1)
	    {
	      name = "*ABS*";
	    }
	  else if (h != NULL)
	    {
	      name = NULL;
	    }
	  else
	    {
	      name = _bfd_coff_internal_syment_name (input_bfd, sym, buf);
	      if (name == NULL)
		name = "UNKNOWN";
	    }
	  sprintf (reloc_type_name, "0x%02x", rel->r_type);

	  (*info->callbacks->reloc_overflow)
	    (info, (h ? &h->root : NULL), name, reloc_type_name,
	     (bfd_vma) 0, input_bfd, input_section,
	     rel->r_vaddr - input_section->vma);
	}

      /* Add RELOCATION to the right bits of VALUE_TO_RELOCATE.  */
      value_to_relocate = ((value_to_relocate & ~howto.dst_mask)
			   | (((value_to_relocate & howto.src_mask)
			       + relocation) & howto.dst_mask));

      /* Put the value back in the object file.  */
      switch (bfd_get_reloc_size (&howto))
	{
	case 2:
	  bfd_put_16 (input_bfd, value_to_relocate, location);
	  break;
	case 4:
	  bfd_put_32 (input_bfd, value_to_relocate, location);
	  break;
	default:
	  bfd_put_64 (input_bfd, value_to_relocate, location);
	  break;
	}
    }
  return true;
}


/* PR 21786:  The PE/COFF standard does not require NUL termination for any of
   the ASCII fields in the archive headers.  So in order to be able to extract
   numerical values we provide our own versions of strtol and strtoll which
   take a maximum length as an additional parameter.  Also - just to save space,
   we omit the endptr return parameter, since we know that it is never used.  */

static long
_bfd_strntol (const char * nptr, int base, unsigned int maxlen)
{
  char buf[24]; /* Should be enough.  */

  BFD_ASSERT (maxlen < (sizeof (buf) - 1));

  memcpy (buf, nptr, maxlen);
  buf[maxlen] = 0;
  return strtol (buf, NULL, base);
}

static long long
_bfd_strntoll (const char * nptr, int base, unsigned int maxlen)
{
  char buf[32]; /* Should be enough.  */

  BFD_ASSERT (maxlen < (sizeof (buf) - 1));

  memcpy (buf, nptr, maxlen);
  buf[maxlen] = 0;
  return strtoll (buf, NULL, base);
}

/* Macro to read an ASCII value stored in an archive header field.  */
#define GET_VALUE_IN_FIELD(VAR, FIELD, BASE)			\
  do								\
    {								\
      (VAR) = (sizeof (VAR) > sizeof (long)			\
	       ? _bfd_strntoll (FIELD, BASE, sizeof FIELD)	\
	       : _bfd_strntol (FIELD, BASE, sizeof FIELD));	\
    }								\
  while (0)

/* Read in the armap of an XCOFF archive.  */

static bool
xcoff64_slurp_armap (bfd *abfd)
{
  file_ptr off;
  size_t namlen;
  bfd_size_type sz, amt;
  bfd_byte *contents, *cend;
  bfd_vma c, i;
  carsym *arsym;
  bfd_byte *p;
  file_ptr pos;

  /* This is for the new format.  */
  struct xcoff_ar_hdr_big hdr;

  if (x_artdata (abfd) == NULL)
    {
      abfd->has_armap = false;
      return true;
    }

  off = bfd_scan_vma (x_artdata (abfd)->u.bhdr.symoff64,
		      (const char **) NULL, 10);
  if (off == 0)
    {
      abfd->has_armap = false;
      return true;
    }

  if (bfd_seek (abfd, off, SEEK_SET) != 0)
    return false;

  /* The symbol table starts with a normal archive header.  */
  if (bfd_bread (&hdr, (bfd_size_type) SIZEOF_AR_HDR_BIG, abfd)
      != SIZEOF_AR_HDR_BIG)
    return false;

  /* Skip the name (normally empty).  */
  GET_VALUE_IN_FIELD (namlen, hdr.namlen, 10);
  pos = ((namlen + 1) & ~(size_t) 1) + SXCOFFARFMAG;
  if (bfd_seek (abfd, pos, SEEK_CUR) != 0)
    return false;

  sz = bfd_scan_vma (hdr.size, (const char **) NULL, 10);
  if (sz + 1 < 9)
    {
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  /* Read in the entire symbol table.  */
  contents = (bfd_byte *) _bfd_alloc_and_read (abfd, sz + 1, sz);
  if (contents == NULL)
    return false;

  /* Ensure strings are NULL terminated so we don't wander off the end
     of the buffer.  */
  contents[sz] = 0;

  /* The symbol table starts with an eight byte count.  */
  c = H_GET_64 (abfd, contents);

  if (c >= sz / 8)
    {
      bfd_set_error (bfd_error_bad_value);
      return false;
    }
  amt = c;
  amt *= sizeof (carsym);
  bfd_ardata (abfd)->symdefs = (carsym *) bfd_alloc (abfd, amt);
  if (bfd_ardata (abfd)->symdefs == NULL)
    return false;

  /* After the count comes a list of eight byte file offsets.  */
  for (i = 0, arsym = bfd_ardata (abfd)->symdefs, p = contents + 8;
       i < c;
       ++i, ++arsym, p += 8)
    arsym->file_offset = H_GET_64 (abfd, p);

  /* After the file offsets come null terminated symbol names.  */
  cend = contents + sz;
  for (i = 0, arsym = bfd_ardata (abfd)->symdefs;
       i < c;
       ++i, ++arsym, p += strlen ((char *) p) + 1)
    {
      if (p >= cend)
	{
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}
      arsym->name = (char *) p;
    }

  bfd_ardata (abfd)->symdef_count = c;
  abfd->has_armap = true;

  return true;
}


/* See if this is an NEW XCOFF archive.  */

static bfd_cleanup
xcoff64_archive_p (bfd *abfd)
{
  struct artdata *tdata_hold;
  char magic[SXCOFFARMAG];
  /* This is the new format.  */
  struct xcoff_ar_file_hdr_big hdr;
  size_t amt = SXCOFFARMAG;

  if (bfd_bread (magic, amt, abfd) != amt)
    {
      if (bfd_get_error () != bfd_error_system_call)
	bfd_set_error (bfd_error_wrong_format);
      return NULL;
    }

  if (strncmp (magic, XCOFFARMAGBIG, SXCOFFARMAG) != 0)
    {
      bfd_set_error (bfd_error_wrong_format);
      return NULL;
    }

  /* Copy over the magic string.  */
  memcpy (hdr.magic, magic, SXCOFFARMAG);

  /* Now read the rest of the file header.  */
  amt = SIZEOF_AR_FILE_HDR_BIG - SXCOFFARMAG;
  if (bfd_bread (&hdr.memoff, amt, abfd) != amt)
    {
      if (bfd_get_error () != bfd_error_system_call)
	bfd_set_error (bfd_error_wrong_format);
      return NULL;
    }

  tdata_hold = bfd_ardata (abfd);

  amt = sizeof (struct artdata);
  bfd_ardata (abfd) = (struct artdata *) bfd_zalloc (abfd, amt);
  if (bfd_ardata (abfd) == (struct artdata *) NULL)
    goto error_ret_restore;

  bfd_ardata (abfd)->first_file_filepos = bfd_scan_vma (hdr.firstmemoff,
							(const char **) NULL,
							10);

  amt = sizeof (struct xcoff_artdata);
  bfd_ardata (abfd)->tdata = bfd_zalloc (abfd, amt);
  if (bfd_ardata (abfd)->tdata == NULL)
    goto error_ret;

  memcpy (&x_artdata (abfd)->u.bhdr, &hdr, SIZEOF_AR_FILE_HDR_BIG);

  if (! xcoff64_slurp_armap (abfd))
    {
    error_ret:
      bfd_release (abfd, bfd_ardata (abfd));
    error_ret_restore:
      bfd_ardata (abfd) = tdata_hold;
      return NULL;
    }

  return _bfd_no_cleanup;
}


/* Open the next element in an XCOFF archive.  */

static bfd *
xcoff64_openr_next_archived_file (bfd *archive, bfd *last_file)
{
  if (x_artdata (archive) == NULL
      || ! xcoff_big_format_p (archive))
    {
      bfd_set_error (bfd_error_invalid_operation);
      return NULL;
    }

  return _bfd_xcoff_openr_next_archived_file (archive, last_file);
}

/* We can't use the usual coff_sizeof_headers routine, because AIX
   always uses an a.out header.  */

static int
xcoff64_sizeof_headers (bfd *abfd,
			struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  int size;

  size = bfd_coff_filhsz (abfd);

  /* Don't think the small aout header can be used since some of the
     old elements have been reordered past the end of the old coff
     small aout size.  */

  if (xcoff_data (abfd)->full_aouthdr)
    size += bfd_coff_aoutsz (abfd);

  size += abfd->section_count * bfd_coff_scnhsz (abfd);
  return size;
}

static asection *
xcoff64_create_csect_from_smclas (bfd *abfd, union internal_auxent *aux,
				  const char *symbol_name)
{
  asection *return_value = NULL;

  /* Changes from 32 :
     .sv == 8, is only for 32 bit programs
     .ti == 12 and .tb == 13 are now reserved.  */
  static const char * const names[] =
  {
    ".pr", ".ro", ".db", ".tc", ".ua", ".rw", ".gl", ".xo",
    NULL, ".bs", ".ds", ".uc", NULL,  NULL,  NULL,  ".tc0",
    ".td", ".sv64", ".sv3264", NULL, ".tl", ".ul", ".te"
  };

  if ((aux->x_csect.x_smclas < ARRAY_SIZE (names))
      && (NULL != names[aux->x_csect.x_smclas]))
    {

      return_value = bfd_make_section_anyway
	(abfd, names[aux->x_csect.x_smclas]);

    }
  else
    {
      _bfd_error_handler
	/* xgettext: c-format */
	(_("%pB: symbol `%s' has unrecognized smclas %d"),
	 abfd, symbol_name, aux->x_csect.x_smclas);
      bfd_set_error (bfd_error_bad_value);
    }

  return return_value;
}

static bool
xcoff64_is_lineno_count_overflow (bfd *abfd ATTRIBUTE_UNUSED,
				  bfd_vma value ATTRIBUTE_UNUSED)
{
  return false;
}

static bool
xcoff64_is_reloc_count_overflow (bfd *abfd ATTRIBUTE_UNUSED,
				 bfd_vma value ATTRIBUTE_UNUSED)
{
  return false;
}

static bfd_vma
xcoff64_loader_symbol_offset (bfd *abfd ATTRIBUTE_UNUSED,
			      struct internal_ldhdr *ldhdr)
{
  return (ldhdr->l_symoff);
}

static bfd_vma
xcoff64_loader_reloc_offset (bfd *abfd ATTRIBUTE_UNUSED,
			     struct internal_ldhdr *ldhdr)
{
  return (ldhdr->l_rldoff);
}

static bool
xcoff64_bad_format_hook (bfd * abfd, void *filehdr)
{
  struct internal_filehdr *internal_f = (struct internal_filehdr *) filehdr;

  /* Check flavor first.  */
  if (bfd_get_flavour (abfd) != bfd_target_xcoff_flavour)
    return false;

  if (bfd_xcoff_magic_number (abfd) != internal_f->f_magic)
    return false;

  return true;
}

static bool
xcoff64_generate_rtinit (bfd *abfd, const char *init, const char *fini,
			 bool rtld)
{
  bfd_byte filehdr_ext[FILHSZ];
  bfd_byte scnhdr_ext[SCNHSZ * 3];
  bfd_byte syment_ext[SYMESZ * 10];
  bfd_byte reloc_ext[RELSZ * 3];
  bfd_byte *data_buffer;
  bfd_size_type data_buffer_size;
  bfd_byte *string_table, *st_tmp;
  bfd_size_type string_table_size;
  bfd_vma val;
  size_t initsz, finisz;
  struct internal_filehdr filehdr;
  struct internal_scnhdr text_scnhdr;
  struct internal_scnhdr data_scnhdr;
  struct internal_scnhdr bss_scnhdr;
  struct internal_syment syment;
  union internal_auxent auxent;
  struct internal_reloc reloc;

  char *text_name = ".text";
  char *data_name = ".data";
  char *bss_name = ".bss";
  char *rtinit_name = "__rtinit";
  char *rtld_name = "__rtld";

  if (! bfd_xcoff_rtinit_size (abfd))
    return false;

  initsz = (init == NULL ? 0 : 1 + strlen (init));
  finisz = (fini == NULL ? 0 : 1 + strlen (fini));

  /* File header.  */
  memset (filehdr_ext, 0, FILHSZ);
  memset (&filehdr, 0, sizeof (struct internal_filehdr));
  filehdr.f_magic = bfd_xcoff_magic_number (abfd);
  filehdr.f_nscns = 3;
  filehdr.f_timdat = 0;
  filehdr.f_nsyms = 0;  /* at least 6, no more than 8 */
  filehdr.f_symptr = 0; /* set below */
  filehdr.f_opthdr = 0;
  filehdr.f_flags = 0;

  /* Section headers.  */
  memset (scnhdr_ext, 0, 3 * SCNHSZ);

  /* Text.  */
  memset (&text_scnhdr, 0, sizeof (struct internal_scnhdr));
  memcpy (text_scnhdr.s_name, text_name, strlen (text_name));
  text_scnhdr.s_paddr = 0;
  text_scnhdr.s_vaddr = 0;
  text_scnhdr.s_size = 0;
  text_scnhdr.s_scnptr = 0;
  text_scnhdr.s_relptr = 0;
  text_scnhdr.s_lnnoptr = 0;
  text_scnhdr.s_nreloc = 0;
  text_scnhdr.s_nlnno = 0;
  text_scnhdr.s_flags = STYP_TEXT;

  /* Data.  */
  memset (&data_scnhdr, 0, sizeof (struct internal_scnhdr));
  memcpy (data_scnhdr.s_name, data_name, strlen (data_name));
  data_scnhdr.s_paddr = 0;
  data_scnhdr.s_vaddr = 0;
  data_scnhdr.s_size = 0;    /* set below */
  data_scnhdr.s_scnptr = FILHSZ + 3 * SCNHSZ;
  data_scnhdr.s_relptr = 0;  /* set below */
  data_scnhdr.s_lnnoptr = 0;
  data_scnhdr.s_nreloc = 0;  /* either 1 or 2 */
  data_scnhdr.s_nlnno = 0;
  data_scnhdr.s_flags = STYP_DATA;

  /* Bss.  */
  memset (&bss_scnhdr, 0, sizeof (struct internal_scnhdr));
  memcpy (bss_scnhdr.s_name, bss_name, strlen (bss_name));
  bss_scnhdr.s_paddr = 0; /* set below */
  bss_scnhdr.s_vaddr = 0; /* set below */
  bss_scnhdr.s_size = 0;  /* set below */
  bss_scnhdr.s_scnptr = 0;
  bss_scnhdr.s_relptr = 0;
  bss_scnhdr.s_lnnoptr = 0;
  bss_scnhdr.s_nreloc = 0;
  bss_scnhdr.s_nlnno = 0;
  bss_scnhdr.s_flags = STYP_BSS;

  /* .data
     0x0000	      0x00000000 : rtl
     0x0004	      0x00000000 :
     0x0008	      0x00000018 : offset to init, or 0
     0x000C	      0x00000038 : offset to fini, or 0
     0x0010	      0x00000010 : size of descriptor
     0x0014	      0x00000000 : pad
     0x0018	      0x00000000 : init, needs a reloc
     0x001C	      0x00000000 :
     0x0020	      0x00000058 : offset to init name
     0x0024	      0x00000000 : flags, padded to a word
     0x0028	      0x00000000 : empty init
     0x002C	      0x00000000 :
     0x0030	      0x00000000 :
     0x0034	      0x00000000 :
     0x0038	      0x00000000 : fini, needs a reloc
     0x003C	      0x00000000 :
     0x0040	      0x00000??? : offset to fini name
     0x0044	      0x00000000 : flags, padded to a word
     0x0048	      0x00000000 : empty fini
     0x004C	      0x00000000 :
     0x0050	      0x00000000 :
     0x0054	      0x00000000 :
     0x0058	      init name
     0x0058 + initsz  fini name */

  data_buffer_size = 0x0058 + initsz + finisz;
  data_buffer_size = (data_buffer_size + 7) &~ (bfd_size_type) 7;
  data_buffer = NULL;
  data_buffer = (bfd_byte *) bfd_zmalloc (data_buffer_size);
  if (data_buffer == NULL)
    return false;

  if (initsz)
    {
      val = 0x18;
      bfd_put_32 (abfd, val, &data_buffer[0x08]);
      val = 0x58;
      bfd_put_32 (abfd, val, &data_buffer[0x20]);
      memcpy (&data_buffer[val], init, initsz);
    }

  if (finisz)
    {
      val = 0x38;
      bfd_put_32 (abfd, val, &data_buffer[0x0C]);
      val = 0x58 + initsz;
      bfd_put_32 (abfd, val, &data_buffer[0x40]);
      memcpy (&data_buffer[val], fini, finisz);
    }

  val = 0x10;
  bfd_put_32 (abfd, val, &data_buffer[0x10]);
  data_scnhdr.s_size = data_buffer_size;
  bss_scnhdr.s_paddr = bss_scnhdr.s_vaddr = data_scnhdr.s_size;

  /* String table.  */
  string_table_size = 4;
  string_table_size += strlen (data_name) + 1;
  string_table_size += strlen (rtinit_name) + 1;
  string_table_size += initsz;
  string_table_size += finisz;
  if (rtld)
    string_table_size += strlen (rtld_name) + 1;

  string_table = (bfd_byte *) bfd_zmalloc (string_table_size);
  if (string_table == NULL)
    return false;

  val = string_table_size;
  bfd_put_32 (abfd, val, &string_table[0]);
  st_tmp = string_table + 4;

  /* symbols
     0. .data csect
     2. __rtinit
     4. init function
     6. fini function
     8. __rtld  */
  memset (syment_ext, 0, 10 * SYMESZ);
  memset (reloc_ext, 0, 3 * RELSZ);

  /* .data csect */
  memset (&syment, 0, sizeof (struct internal_syment));
  memset (&auxent, 0, sizeof (union internal_auxent));

  syment._n._n_n._n_offset = st_tmp - string_table;
  memcpy (st_tmp, data_name, strlen (data_name));
  st_tmp += strlen (data_name) + 1;

  syment.n_scnum = 2;
  syment.n_sclass = C_HIDEXT;
  syment.n_numaux = 1;
  auxent.x_csect.x_scnlen.u64 = data_buffer_size;
  auxent.x_csect.x_smtyp = 3 << 3 | XTY_SD;
  auxent.x_csect.x_smclas = XMC_RW;
  bfd_coff_swap_sym_out (abfd, &syment,
			 &syment_ext[filehdr.f_nsyms * SYMESZ]);
  bfd_coff_swap_aux_out (abfd, &auxent, syment.n_type, syment.n_sclass, 0,
			 syment.n_numaux,
			 &syment_ext[(filehdr.f_nsyms + 1) * SYMESZ]);
  filehdr.f_nsyms += 2;

  /* __rtinit */
  memset (&syment, 0, sizeof (struct internal_syment));
  memset (&auxent, 0, sizeof (union internal_auxent));
  syment._n._n_n._n_offset = st_tmp - string_table;
  memcpy (st_tmp, rtinit_name, strlen (rtinit_name));
  st_tmp += strlen (rtinit_name) + 1;

  syment.n_scnum = 2;
  syment.n_sclass = C_EXT;
  syment.n_numaux = 1;
  auxent.x_csect.x_smtyp = XTY_LD;
  auxent.x_csect.x_smclas = XMC_RW;
  bfd_coff_swap_sym_out (abfd, &syment,
			 &syment_ext[filehdr.f_nsyms * SYMESZ]);
  bfd_coff_swap_aux_out (abfd, &auxent, syment.n_type, syment.n_sclass, 0,
			 syment.n_numaux,
			 &syment_ext[(filehdr.f_nsyms + 1) * SYMESZ]);
  filehdr.f_nsyms += 2;

  /* Init.  */
  if (initsz)
    {
      memset (&syment, 0, sizeof (struct internal_syment));
      memset (&auxent, 0, sizeof (union internal_auxent));

      syment._n._n_n._n_offset = st_tmp - string_table;
      memcpy (st_tmp, init, initsz);
      st_tmp += initsz;

      syment.n_sclass = C_EXT;
      syment.n_numaux = 1;
      bfd_coff_swap_sym_out (abfd, &syment,
			     &syment_ext[filehdr.f_nsyms * SYMESZ]);
      bfd_coff_swap_aux_out (abfd, &auxent, syment.n_type, syment.n_sclass, 0,
			     syment.n_numaux,
			     &syment_ext[(filehdr.f_nsyms + 1) * SYMESZ]);
      /* Reloc.  */
      memset (&reloc, 0, sizeof (struct internal_reloc));
      reloc.r_vaddr = 0x0018;
      reloc.r_symndx = filehdr.f_nsyms;
      reloc.r_type = R_POS;
      reloc.r_size = 63;
      bfd_coff_swap_reloc_out (abfd, &reloc, &reloc_ext[0]);

      filehdr.f_nsyms += 2;
      data_scnhdr.s_nreloc += 1;
    }

  /* Finit.  */
  if (finisz)
    {
      memset (&syment, 0, sizeof (struct internal_syment));
      memset (&auxent, 0, sizeof (union internal_auxent));

      syment._n._n_n._n_offset = st_tmp - string_table;
      memcpy (st_tmp, fini, finisz);
      st_tmp += finisz;

      syment.n_sclass = C_EXT;
      syment.n_numaux = 1;
      bfd_coff_swap_sym_out (abfd, &syment,
			     &syment_ext[filehdr.f_nsyms * SYMESZ]);
      bfd_coff_swap_aux_out (abfd, &auxent, syment.n_type, syment.n_sclass, 0,
			     syment.n_numaux,
			     &syment_ext[(filehdr.f_nsyms + 1) * SYMESZ]);

      /* Reloc.  */
      memset (&reloc, 0, sizeof (struct internal_reloc));
      reloc.r_vaddr = 0x0038;
      reloc.r_symndx = filehdr.f_nsyms;
      reloc.r_type = R_POS;
      reloc.r_size = 63;
      bfd_coff_swap_reloc_out (abfd, &reloc,
			       &reloc_ext[data_scnhdr.s_nreloc * RELSZ]);

      filehdr.f_nsyms += 2;
      data_scnhdr.s_nreloc += 1;
    }

  if (rtld)
    {
      memset (&syment, 0, sizeof (struct internal_syment));
      memset (&auxent, 0, sizeof (union internal_auxent));

      syment._n._n_n._n_offset = st_tmp - string_table;
      memcpy (st_tmp, rtld_name, strlen (rtld_name));
      st_tmp += strlen (rtld_name) + 1;

      syment.n_sclass = C_EXT;
      syment.n_numaux = 1;
      bfd_coff_swap_sym_out (abfd, &syment,
			     &syment_ext[filehdr.f_nsyms * SYMESZ]);
      bfd_coff_swap_aux_out (abfd, &auxent, syment.n_type, syment.n_sclass, 0,
			     syment.n_numaux,
			     &syment_ext[(filehdr.f_nsyms + 1) * SYMESZ]);

      /* Reloc.  */
      memset (&reloc, 0, sizeof (struct internal_reloc));
      reloc.r_vaddr = 0x0000;
      reloc.r_symndx = filehdr.f_nsyms;
      reloc.r_type = R_POS;
      reloc.r_size = 63;
      bfd_coff_swap_reloc_out (abfd, &reloc,
			       &reloc_ext[data_scnhdr.s_nreloc * RELSZ]);

      filehdr.f_nsyms += 2;
      data_scnhdr.s_nreloc += 1;

      bss_scnhdr.s_size = 0;
    }

  data_scnhdr.s_relptr = data_scnhdr.s_scnptr + data_buffer_size;
  filehdr.f_symptr = data_scnhdr.s_relptr + data_scnhdr.s_nreloc * RELSZ;

  bfd_coff_swap_filehdr_out (abfd, &filehdr, filehdr_ext);
  bfd_bwrite (filehdr_ext, FILHSZ, abfd);
  bfd_coff_swap_scnhdr_out (abfd, &text_scnhdr, &scnhdr_ext[SCNHSZ * 0]);
  bfd_coff_swap_scnhdr_out (abfd, &data_scnhdr, &scnhdr_ext[SCNHSZ * 1]);
  bfd_coff_swap_scnhdr_out (abfd, &bss_scnhdr, &scnhdr_ext[SCNHSZ * 2]);
  bfd_bwrite (scnhdr_ext, 3 * SCNHSZ, abfd);
  bfd_bwrite (data_buffer, data_buffer_size, abfd);
  bfd_bwrite (reloc_ext, data_scnhdr.s_nreloc * RELSZ, abfd);
  bfd_bwrite (syment_ext, filehdr.f_nsyms * SYMESZ, abfd);
  bfd_bwrite (string_table, string_table_size, abfd);

  free (data_buffer);
  data_buffer = NULL;

  return true;
}

/* The typical dynamic reloc.  */

static reloc_howto_type xcoff64_dynamic_reloc =
HOWTO (0,			/* type */
       0,			/* rightshift */
       8,			/* size */
       64,			/* bitsize */
       false,			/* pc_relative */
       0,			/* bitpos */
       complain_overflow_bitfield, /* complain_on_overflow */
       0,			/* special_function */
       "R_POS",			/* name */
       true,			/* partial_inplace */
       MINUS_ONE,		/* src_mask */
       MINUS_ONE,		/* dst_mask */
       false);			/* pcrel_offset */

/* Indirect call stub */
static const unsigned long xcoff64_stub_indirect_call_code[4] =
  {
    0xe9820000,	/* ld r12,0(r2) */
    0xe80c0000,	/* ld r0,0(r12) */
    0x7c0903a6,	/* mtctr r0 */
    0x4e800420,	/* bctr */
  };

/* Shared call stub */
static const unsigned long xcoff64_stub_shared_call_code[6] =
  {
    0xe9820000,	/* ld r12,0(r2) */
    0xf8410028,	/* std r2,40(r1) */
    0xe80c0000,	/* ld r0,0(r12) */
    0xe84c0008,	/* ld r2,8(r12) */
    0x7c0903a6,	/* mtctr r0 */
    0x4e800420,	/* bctr */
  };

static const unsigned long xcoff64_glink_code[10] =
{
  0xe9820000,	/* ld r12,0(r2) */
  0xf8410028,	/* std r2,40(r1) */
  0xe80c0000,	/* ld r0,0(r12) */
  0xe84c0008,	/* ld r2,8(r12) */
  0x7c0903a6,	/* mtctr r0 */
  0x4e800420,	/* bctr */
  0x00000000,	/* start of traceback table */
  0x000ca000,	/* traceback table */
  0x00000000,	/* traceback table */
  0x00000018,	/* ??? */
};

static const struct xcoff_backend_data_rec bfd_xcoff_backend_data =
  {
    { /* COFF backend, defined in libcoff.h.  */
      _bfd_xcoff64_swap_aux_in,
      _bfd_xcoff64_swap_sym_in,
      _bfd_xcoff64_swap_lineno_in,
      _bfd_xcoff64_swap_aux_out,
      _bfd_xcoff64_swap_sym_out,
      _bfd_xcoff64_swap_lineno_out,
      xcoff64_swap_reloc_out,
      coff_swap_filehdr_out,
      coff_swap_aouthdr_out,
      coff_swap_scnhdr_out,
      FILHSZ,
      AOUTSZ,
      SCNHSZ,
      SYMESZ,
      AUXESZ,
      RELSZ,
      LINESZ,
      FILNMLEN,
      true,			/* _bfd_coff_long_filenames */
      XCOFF_NO_LONG_SECTION_NAMES,  /* _bfd_coff_long_section_names */
      3,			/* _bfd_coff_default_section_alignment_power */
      true,			/* _bfd_coff_force_symnames_in_strings */
      4,			/* _bfd_coff_debug_string_prefix_length */
      32768,			/* _bfd_coff_max_nscns */
      coff_swap_filehdr_in,
      coff_swap_aouthdr_in,
      coff_swap_scnhdr_in,
      xcoff64_swap_reloc_in,
      xcoff64_bad_format_hook,
      coff_set_arch_mach_hook,
      coff_mkobject_hook,
      styp_to_sec_flags,
      coff_set_alignment_hook,
      coff_slurp_symbol_table,
      symname_in_debug_hook,
      coff_pointerize_aux_hook,
      coff_print_aux,
      dummy_reloc16_extra_cases,
      dummy_reloc16_estimate,
      NULL,			/* bfd_coff_symbol_classification */
      coff_compute_section_file_positions,
      NULL,			/* _bfd_coff_start_final_link */
      xcoff64_ppc_relocate_section,
      coff_rtype_to_howto,
      NULL,			/* _bfd_coff_adjust_symndx */
      _bfd_generic_link_add_one_symbol,
      coff_link_output_has_begun,
      coff_final_link_postscript,
      NULL			/* print_pdata.  */
    },

    0x01EF,			/* magic number */
    bfd_arch_powerpc,
    bfd_mach_ppc_620,

    /* Function pointers to xcoff specific swap routines.  */
    xcoff64_swap_ldhdr_in,
    xcoff64_swap_ldhdr_out,
    xcoff64_swap_ldsym_in,
    xcoff64_swap_ldsym_out,
    xcoff64_swap_ldrel_in,
    xcoff64_swap_ldrel_out,

    /* Sizes.  */
    LDHDRSZ,
    LDSYMSZ,
    LDRELSZ,
    24,				/* _xcoff_function_descriptor_size */
    0,				/* _xcoff_small_aout_header_size */

    /* Versions.  */
    2,				/* _xcoff_ldhdr_version */

    _bfd_xcoff64_put_symbol_name,
    _bfd_xcoff64_put_ldsymbol_name,
    &xcoff64_dynamic_reloc,
    xcoff64_create_csect_from_smclas,

    /* Lineno and reloc count overflow.  */
    xcoff64_is_lineno_count_overflow,
    xcoff64_is_reloc_count_overflow,

    xcoff64_loader_symbol_offset,
    xcoff64_loader_reloc_offset,

    /* glink.  */
    &xcoff64_glink_code[0],
    40,				/* _xcoff_glink_size */

    /* rtinit.  */
    88,				/* _xcoff_rtinit_size */
    xcoff64_generate_rtinit,

    /* Stub indirect call.  */
    &xcoff64_stub_indirect_call_code[0],
    16,				/* _xcoff_stub_indirect_call_size */

    /* Stub shared call.  */
    &xcoff64_stub_shared_call_code[0],
    24,				/* _xcoff_stub_shared_call_size */
  };

/* The transfer vector that leads the outside world to all of the above.  */
const bfd_target rs6000_xcoff64_vec =
  {
    "aixcoff64-rs6000",
    bfd_target_xcoff_flavour,
    BFD_ENDIAN_BIG,		/* data byte order is big */
    BFD_ENDIAN_BIG,		/* header byte order is big */

    (HAS_RELOC | EXEC_P | HAS_LINENO | HAS_DEBUG | DYNAMIC
     | HAS_SYMS | HAS_LOCALS | WP_TEXT),

    SEC_HAS_CONTENTS | SEC_ALLOC | SEC_LOAD | SEC_RELOC | SEC_CODE | SEC_DATA,
    0,				/* leading char */
    '/',			/* ar_pad_char */
    15,				/* ar_max_namelen */
    0,				/* match priority.  */
    TARGET_KEEP_UNUSED_SECTION_SYMBOLS, /* keep unused section symbols.  */

    /* data */
    bfd_getb64,
    bfd_getb_signed_64,
    bfd_putb64,
    bfd_getb32,
    bfd_getb_signed_32,
    bfd_putb32,
    bfd_getb16,
    bfd_getb_signed_16,
    bfd_putb16,

    /* hdrs */
    bfd_getb64,
    bfd_getb_signed_64,
    bfd_putb64,
    bfd_getb32,
    bfd_getb_signed_32,
    bfd_putb32,
    bfd_getb16,
    bfd_getb_signed_16,
    bfd_putb16,

    { /* bfd_check_format */
      _bfd_dummy_target,
      coff_object_p,
      xcoff64_archive_p,
      CORE_FILE_P
    },

    { /* bfd_set_format */
      _bfd_bool_bfd_false_error,
      coff_mkobject,
      _bfd_generic_mkarchive,
      _bfd_bool_bfd_false_error
    },

    {/* bfd_write_contents */
      _bfd_bool_bfd_false_error,
      coff_write_object_contents,
      _bfd_xcoff_write_archive_contents,
      _bfd_bool_bfd_false_error
    },

    /* Generic */
    coff_close_and_cleanup,
    coff_bfd_free_cached_info,
    coff_new_section_hook,
    _bfd_generic_get_section_contents,
    _bfd_generic_get_section_contents_in_window,

    /* Copy */
    _bfd_xcoff_copy_private_bfd_data,
    _bfd_generic_bfd_merge_private_bfd_data,
    _bfd_generic_init_private_section_data,
    _bfd_generic_bfd_copy_private_section_data,
    _bfd_generic_bfd_copy_private_symbol_data,
    _bfd_generic_bfd_copy_private_header_data,
    _bfd_generic_bfd_set_private_flags,
    _bfd_generic_bfd_print_private_bfd_data,

    /* Core */
    BFD_JUMP_TABLE_CORE (coff),

    /* Archive */
    xcoff64_slurp_armap,
    _bfd_noarchive_slurp_extended_name_table,
    _bfd_noarchive_construct_extended_name_table,
    bfd_dont_truncate_arname,
    _bfd_xcoff_write_armap,
    _bfd_xcoff_read_ar_hdr,
    _bfd_generic_write_ar_hdr,
    xcoff64_openr_next_archived_file,
    _bfd_generic_get_elt_at_index,
    _bfd_xcoff_stat_arch_elt,
    _bfd_bool_bfd_true,

    /* Symbols */
    coff_get_symtab_upper_bound,
    coff_canonicalize_symtab,
    coff_make_empty_symbol,
    coff_print_symbol,
    coff_get_symbol_info,
    coff_get_symbol_version_string,
    _bfd_xcoff_is_local_label_name,
    coff_bfd_is_target_special_symbol,
    coff_get_lineno,
    coff_find_nearest_line,
    coff_find_nearest_line_with_alt,
    coff_find_line,
    coff_find_inliner_info,
    coff_bfd_make_debug_symbol,
    _bfd_generic_read_minisymbols,
    _bfd_generic_minisymbol_to_symbol,

    /* Reloc */
    coff_get_reloc_upper_bound,
    coff_canonicalize_reloc,
    _bfd_generic_set_reloc,
    xcoff64_reloc_type_lookup,
    xcoff64_reloc_name_lookup,

    /* Write */
    coff_set_arch_mach,
    coff_set_section_contents,

    /* Link */
    xcoff64_sizeof_headers,
    bfd_generic_get_relocated_section_contents,
    bfd_generic_relax_section,
    _bfd_xcoff_bfd_link_hash_table_create,
    _bfd_xcoff_bfd_link_add_symbols,
    _bfd_generic_link_just_syms,
    _bfd_generic_copy_link_hash_symbol_type,
    _bfd_xcoff_bfd_final_link,
    _bfd_generic_link_split_section,
    _bfd_generic_link_check_relocs,
    bfd_generic_gc_sections,
    bfd_generic_lookup_section_flags,
    bfd_generic_merge_sections,
    bfd_generic_is_group_section,
    bfd_generic_group_name,
    bfd_generic_discard_group,
    _bfd_generic_section_already_linked,
    _bfd_xcoff_define_common_symbol,
    _bfd_generic_link_hide_symbol,
    bfd_generic_define_start_stop,

    /* Dynamic */
    _bfd_xcoff_get_dynamic_symtab_upper_bound,
    _bfd_xcoff_canonicalize_dynamic_symtab,
    _bfd_nodynamic_get_synthetic_symtab,
    _bfd_xcoff_get_dynamic_reloc_upper_bound,
    _bfd_xcoff_canonicalize_dynamic_reloc,

    /* Opposite endian version, none exists */
    NULL,

    &bfd_xcoff_backend_data,
  };

extern bfd_cleanup xcoff64_core_p
  (bfd *);
extern bool xcoff64_core_file_matches_executable_p
  (bfd *, bfd *);
extern char *xcoff64_core_file_failing_command
  (bfd *);
extern int xcoff64_core_file_failing_signal
  (bfd *);
#define xcoff64_core_file_pid _bfd_nocore_core_file_pid

/* AIX 5 */
static const struct xcoff_backend_data_rec bfd_xcoff_aix5_backend_data =
  {
    { /* COFF backend, defined in libcoff.h.  */
      _bfd_xcoff64_swap_aux_in,
      _bfd_xcoff64_swap_sym_in,
      _bfd_xcoff64_swap_lineno_in,
      _bfd_xcoff64_swap_aux_out,
      _bfd_xcoff64_swap_sym_out,
      _bfd_xcoff64_swap_lineno_out,
      xcoff64_swap_reloc_out,
      coff_swap_filehdr_out,
      coff_swap_aouthdr_out,
      coff_swap_scnhdr_out,
      FILHSZ,
      AOUTSZ,
      SCNHSZ,
      SYMESZ,
      AUXESZ,
      RELSZ,
      LINESZ,
      FILNMLEN,
      true,			/* _bfd_coff_long_filenames */
      XCOFF_NO_LONG_SECTION_NAMES,  /* _bfd_coff_long_section_names */
      3,			/* _bfd_coff_default_section_alignment_power */
      true,			/* _bfd_coff_force_symnames_in_strings */
      4,			/* _bfd_coff_debug_string_prefix_length */
      32768,			/* _bfd_coff_max_nscns */
      coff_swap_filehdr_in,
      coff_swap_aouthdr_in,
      coff_swap_scnhdr_in,
      xcoff64_swap_reloc_in,
      xcoff64_bad_format_hook,
      coff_set_arch_mach_hook,
      coff_mkobject_hook,
      styp_to_sec_flags,
      coff_set_alignment_hook,
      coff_slurp_symbol_table,
      symname_in_debug_hook,
      coff_pointerize_aux_hook,
      coff_print_aux,
      dummy_reloc16_extra_cases,
      dummy_reloc16_estimate,
      NULL,			/* bfd_coff_sym_is_global */
      coff_compute_section_file_positions,
      NULL,			/* _bfd_coff_start_final_link */
      xcoff64_ppc_relocate_section,
      coff_rtype_to_howto,
      NULL,			/* _bfd_coff_adjust_symndx */
      _bfd_generic_link_add_one_symbol,
      coff_link_output_has_begun,
      coff_final_link_postscript,
      NULL			/* print_pdata.  */
    },

    U64_TOCMAGIC,		/* magic number */
    bfd_arch_powerpc,
    bfd_mach_ppc_620,

    /* Function pointers to xcoff specific swap routines.  */
    xcoff64_swap_ldhdr_in,
    xcoff64_swap_ldhdr_out,
    xcoff64_swap_ldsym_in,
    xcoff64_swap_ldsym_out,
    xcoff64_swap_ldrel_in,
    xcoff64_swap_ldrel_out,

    /* Sizes.  */
    LDHDRSZ,
    LDSYMSZ,
    LDRELSZ,
    24,				/* _xcoff_function_descriptor_size */
    0,				/* _xcoff_small_aout_header_size */
    /* Versions.  */
    2,				/* _xcoff_ldhdr_version */

    _bfd_xcoff64_put_symbol_name,
    _bfd_xcoff64_put_ldsymbol_name,
    &xcoff64_dynamic_reloc,
    xcoff64_create_csect_from_smclas,

    /* Lineno and reloc count overflow.  */
    xcoff64_is_lineno_count_overflow,
    xcoff64_is_reloc_count_overflow,

    xcoff64_loader_symbol_offset,
    xcoff64_loader_reloc_offset,

    /* glink.  */
    &xcoff64_glink_code[0],
    40,				/* _xcoff_glink_size */

    /* rtinit.  */
    88,				/* _xcoff_rtinit_size */
    xcoff64_generate_rtinit,

    /* Stub indirect call.  */
    &xcoff64_stub_indirect_call_code[0],
    16,				/* _xcoff_stub_indirect_call_size */

    /* Stub shared call.  */
    &xcoff64_stub_shared_call_code[0],
    24,				/* _xcoff_stub_shared_call_size */
  };

/* The transfer vector that leads the outside world to all of the above.  */
const bfd_target rs6000_xcoff64_aix_vec =
  {
    "aix5coff64-rs6000",
    bfd_target_xcoff_flavour,
    BFD_ENDIAN_BIG,		/* data byte order is big */
    BFD_ENDIAN_BIG,		/* header byte order is big */

    (HAS_RELOC | EXEC_P | HAS_LINENO | HAS_DEBUG | DYNAMIC
     | HAS_SYMS | HAS_LOCALS | WP_TEXT),

    SEC_HAS_CONTENTS | SEC_ALLOC | SEC_LOAD | SEC_RELOC | SEC_CODE | SEC_DATA,
    0,				/* leading char */
    '/',			/* ar_pad_char */
    15,				/* ar_max_namelen */
    0,				/* match priority.  */
    TARGET_KEEP_UNUSED_SECTION_SYMBOLS, /* keep unused section symbols.  */

    /* data */
    bfd_getb64,
    bfd_getb_signed_64,
    bfd_putb64,
    bfd_getb32,
    bfd_getb_signed_32,
    bfd_putb32,
    bfd_getb16,
    bfd_getb_signed_16,
    bfd_putb16,

    /* hdrs */
    bfd_getb64,
    bfd_getb_signed_64,
    bfd_putb64,
    bfd_getb32,
    bfd_getb_signed_32,
    bfd_putb32,
    bfd_getb16,
    bfd_getb_signed_16,
    bfd_putb16,

    { /* bfd_check_format */
      _bfd_dummy_target,
      coff_object_p,
      xcoff64_archive_p,
      xcoff64_core_p
    },

    { /* bfd_set_format */
      _bfd_bool_bfd_false_error,
      coff_mkobject,
      _bfd_generic_mkarchive,
      _bfd_bool_bfd_false_error
    },

    {/* bfd_write_contents */
      _bfd_bool_bfd_false_error,
      coff_write_object_contents,
      _bfd_xcoff_write_archive_contents,
      _bfd_bool_bfd_false_error
    },

    /* Generic */
    coff_close_and_cleanup,
    coff_bfd_free_cached_info,
    coff_new_section_hook,
    _bfd_generic_get_section_contents,
    _bfd_generic_get_section_contents_in_window,

    /* Copy */
    _bfd_xcoff_copy_private_bfd_data,
    _bfd_generic_bfd_merge_private_bfd_data,
    _bfd_generic_init_private_section_data,
    _bfd_generic_bfd_copy_private_section_data,
    _bfd_generic_bfd_copy_private_symbol_data,
    _bfd_generic_bfd_copy_private_header_data,
    _bfd_generic_bfd_set_private_flags,
    _bfd_generic_bfd_print_private_bfd_data,

    /* Core */
    BFD_JUMP_TABLE_CORE (xcoff64),

    /* Archive */
    xcoff64_slurp_armap,
    _bfd_noarchive_slurp_extended_name_table,
    _bfd_noarchive_construct_extended_name_table,
    bfd_dont_truncate_arname,
    _bfd_xcoff_write_armap,
    _bfd_xcoff_read_ar_hdr,
    _bfd_generic_write_ar_hdr,
    xcoff64_openr_next_archived_file,
    _bfd_generic_get_elt_at_index,
    _bfd_xcoff_stat_arch_elt,
    _bfd_bool_bfd_true,

    /* Symbols */
    coff_get_symtab_upper_bound,
    coff_canonicalize_symtab,
    coff_make_empty_symbol,
    coff_print_symbol,
    coff_get_symbol_info,
    coff_get_symbol_version_string,
    _bfd_xcoff_is_local_label_name,
    coff_bfd_is_target_special_symbol,
    coff_get_lineno,
    coff_find_nearest_line,
    coff_find_nearest_line_with_alt,
    coff_find_line,
    coff_find_inliner_info,
    coff_bfd_make_debug_symbol,
    _bfd_generic_read_minisymbols,
    _bfd_generic_minisymbol_to_symbol,

    /* Reloc */
    coff_get_reloc_upper_bound,
    coff_canonicalize_reloc,
    _bfd_generic_set_reloc,
    xcoff64_reloc_type_lookup,
    xcoff64_reloc_name_lookup,

    /* Write */
    coff_set_arch_mach,
    coff_set_section_contents,

    /* Link */
    xcoff64_sizeof_headers,
    bfd_generic_get_relocated_section_contents,
    bfd_generic_relax_section,
    _bfd_xcoff_bfd_link_hash_table_create,
    _bfd_xcoff_bfd_link_add_symbols,
    _bfd_generic_link_just_syms,
    _bfd_generic_copy_link_hash_symbol_type,
    _bfd_xcoff_bfd_final_link,
    _bfd_generic_link_split_section,
    _bfd_generic_link_check_relocs,
    bfd_generic_gc_sections,
    bfd_generic_lookup_section_flags,
    bfd_generic_merge_sections,
    bfd_generic_is_group_section,
    bfd_generic_group_name,
    bfd_generic_discard_group,
    _bfd_generic_section_already_linked,
    _bfd_xcoff_define_common_symbol,
    _bfd_generic_link_hide_symbol,
    bfd_generic_define_start_stop,

    /* Dynamic */
    _bfd_xcoff_get_dynamic_symtab_upper_bound,
    _bfd_xcoff_canonicalize_dynamic_symtab,
    _bfd_nodynamic_get_synthetic_symtab,
    _bfd_xcoff_get_dynamic_reloc_upper_bound,
    _bfd_xcoff_canonicalize_dynamic_reloc,

    /* Opposite endian version, none exists.  */
    NULL,

    & bfd_xcoff_aix5_backend_data,
  };
