/* BFD back-end for IBM RS/6000 "XCOFF" files.
   Copyright (C) 1990-2023 Free Software Foundation, Inc.
   Written by Metin G. Ozisik, Mimi Phuong-Thao Vo, and John Gilmore.
   Archive support from Damon A. Permezel.
   Contributed by IBM Corporation and Cygnus Support.

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
#include "libiberty.h"
#include "bfd.h"
#include "bfdlink.h"
#include "libbfd.h"
#include "coff/internal.h"
#include "coff/xcoff.h"
#include "coff/rs6000.h"
#include "libcoff.h"
#include "libxcoff.h"

extern bool _bfd_xcoff_mkobject (bfd *);
extern bool _bfd_xcoff_copy_private_bfd_data (bfd *, bfd *);
extern bool _bfd_xcoff_is_local_label_name (bfd *, const char *);
extern reloc_howto_type *_bfd_xcoff_reloc_type_lookup
  (bfd *, bfd_reloc_code_real_type);
extern bool _bfd_xcoff_slurp_armap (bfd *);
extern bfd_cleanup _bfd_xcoff_archive_p (bfd *);
extern void * _bfd_xcoff_read_ar_hdr (bfd *);
extern bfd *_bfd_xcoff_openr_next_archived_file (bfd *, bfd *);
extern int _bfd_xcoff_stat_arch_elt (bfd *, struct stat *);
extern bool _bfd_xcoff_write_armap
  (bfd *, unsigned int, struct orl *, unsigned int, int);
extern bool _bfd_xcoff_write_archive_contents (bfd *);
extern int _bfd_xcoff_sizeof_headers (bfd *, struct bfd_link_info *);
extern void _bfd_xcoff_swap_sym_in (bfd *, void *, void *);
extern unsigned int _bfd_xcoff_swap_sym_out (bfd *, void *, void *);
extern void _bfd_xcoff_swap_aux_in (bfd *, void *, int, int, int, int, void *);
extern unsigned int _bfd_xcoff_swap_aux_out
  (bfd *, void *, int, int, int, int, void *);
static void xcoff_swap_reloc_in (bfd *, void *, void *);
static unsigned int xcoff_swap_reloc_out (bfd *, void *, void *);

/* Forward declare xcoff_rtype2howto for coffcode.h macro.  */
void xcoff_rtype2howto (arelent *, struct internal_reloc *);

/* coffcode.h needs these to be defined.  */
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
#define RTYPE2HOWTO(cache_ptr, dst) xcoff_rtype2howto (cache_ptr, dst)
#define coff_mkobject _bfd_xcoff_mkobject
#define coff_bfd_is_local_label_name _bfd_xcoff_is_local_label_name
#ifdef AIX_CORE
extern bfd_cleanup rs6000coff_core_p (bfd *abfd);
extern bool rs6000coff_core_file_matches_executable_p
  (bfd *cbfd, bfd *ebfd);
extern char *rs6000coff_core_file_failing_command (bfd *abfd);
extern int rs6000coff_core_file_failing_signal (bfd *abfd);
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
#define coff_SWAP_sym_in _bfd_xcoff_swap_sym_in
#define coff_SWAP_sym_out _bfd_xcoff_swap_sym_out
#define coff_SWAP_aux_in _bfd_xcoff_swap_aux_in
#define coff_SWAP_aux_out _bfd_xcoff_swap_aux_out
#define coff_swap_reloc_in xcoff_swap_reloc_in
#define coff_swap_reloc_out xcoff_swap_reloc_out
#define NO_COFF_RELOCS

#ifndef bfd_pe_print_pdata
#define bfd_pe_print_pdata	NULL
#endif

#include "coffcode.h"

/* The main body of code is in coffcode.h.  */

static const char *normalize_filename (bfd *);
static bool xcoff_write_armap_old
  (bfd *, unsigned int, struct orl *, unsigned int, int);
static bool xcoff_write_armap_big
  (bfd *, unsigned int, struct orl *, unsigned int, int);
static bool xcoff_write_archive_contents_old (bfd *);
static bool xcoff_write_archive_contents_big (bfd *);
static void xcoff_swap_ldhdr_in (bfd *, const void *, struct internal_ldhdr *);
static void xcoff_swap_ldhdr_out (bfd *, const struct internal_ldhdr *, void *);
static void xcoff_swap_ldsym_in (bfd *, const void *, struct internal_ldsym *);
static void xcoff_swap_ldsym_out (bfd *, const struct internal_ldsym *, void *);
static void xcoff_swap_ldrel_in (bfd *, const void *, struct internal_ldrel *);
static void xcoff_swap_ldrel_out (bfd *, const struct internal_ldrel *, void *);
static bool xcoff_ppc_relocate_section
  (bfd *, struct bfd_link_info *, bfd *, asection *, bfd_byte *,
   struct internal_reloc *, struct internal_syment *, asection **);
static bool _bfd_xcoff_put_ldsymbol_name
  (bfd *, struct xcoff_loader_info *, struct internal_ldsym *, const char *);
static asection *xcoff_create_csect_from_smclas
  (bfd *, union internal_auxent *, const char *);
static bool xcoff_is_lineno_count_overflow (bfd *, bfd_vma);
static bool xcoff_is_reloc_count_overflow (bfd *, bfd_vma);
static bfd_vma xcoff_loader_symbol_offset (bfd *, struct internal_ldhdr *);
static bfd_vma xcoff_loader_reloc_offset (bfd *, struct internal_ldhdr *);
static bool xcoff_generate_rtinit
  (bfd *, const char *, const char *, bool);
static bool do_pad (bfd *, unsigned int);
static bool do_copy (bfd *, bfd *);

/* Relocation functions */
static xcoff_reloc_function xcoff_reloc_type_br;

static xcoff_complain_function xcoff_complain_overflow_dont_func;
static xcoff_complain_function xcoff_complain_overflow_bitfield_func;
static xcoff_complain_function xcoff_complain_overflow_signed_func;
static xcoff_complain_function xcoff_complain_overflow_unsigned_func;

xcoff_reloc_function *const
xcoff_calculate_relocation[XCOFF_MAX_CALCULATE_RELOCATION] =
{
  xcoff_reloc_type_pos,  /* R_POS   (0x00) */
  xcoff_reloc_type_neg,  /* R_NEG   (0x01) */
  xcoff_reloc_type_rel,  /* R_REL   (0x02) */
  xcoff_reloc_type_toc,  /* R_TOC   (0x03) */
  xcoff_reloc_type_toc,  /* R_TRL   (0x04) */
  xcoff_reloc_type_toc,  /* R_GL    (0x05) */
  xcoff_reloc_type_toc,  /* R_TCL   (0x06) */
  xcoff_reloc_type_fail, /*         (0x07) */
  xcoff_reloc_type_ba,   /* R_BA    (0x08) */
  xcoff_reloc_type_fail, /*         (0x09) */
  xcoff_reloc_type_br,   /* R_BR    (0x0a) */
  xcoff_reloc_type_fail, /*         (0x0b) */
  xcoff_reloc_type_pos,  /* R_RL    (0x0c) */
  xcoff_reloc_type_pos,  /* R_RLA   (0x0d) */
  xcoff_reloc_type_fail, /*         (0x0e) */
  xcoff_reloc_type_noop, /* R_REF   (0x0f) */
  xcoff_reloc_type_fail, /*         (0x10) */
  xcoff_reloc_type_fail, /*         (0x11) */
  xcoff_reloc_type_fail, /*         (0x12) */
  xcoff_reloc_type_toc,  /* R_TRLA  (0x13) */
  xcoff_reloc_type_fail, /* R_RRTBI (0x14) */
  xcoff_reloc_type_fail, /* R_RRTBA (0x15) */
  xcoff_reloc_type_ba,   /* R_CAI   (0x16) */
  xcoff_reloc_type_crel, /* R_CREL  (0x17) */
  xcoff_reloc_type_ba,   /* R_RBA   (0x18) */
  xcoff_reloc_type_ba,   /* R_RBAC  (0x19) */
  xcoff_reloc_type_br,   /* R_RBR   (0x1a) */
  xcoff_reloc_type_ba,   /* R_RBRC  (0x1b) */
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
  xcoff_reloc_type_toc,  /* R_TOCU    (0x30) */
  xcoff_reloc_type_toc,  /* R_TOCL    (0x31) */
};

xcoff_complain_function *const
xcoff_complain_overflow[XCOFF_MAX_COMPLAIN_OVERFLOW] =
{
  xcoff_complain_overflow_dont_func,
  xcoff_complain_overflow_bitfield_func,
  xcoff_complain_overflow_signed_func,
  xcoff_complain_overflow_unsigned_func,
};

/* Information about one member of an archive.  */
struct member_layout
{
  /* The archive member that this structure describes.  */
  bfd *member;

  /* The number of bytes of padding that must be inserted before the
     start of the member in order to ensure that the section contents
     are correctly aligned.  */
  unsigned int leading_padding;

  /* The offset of MEMBER from the start of the archive (i.e. the end
     of the leading padding).  */
  file_ptr offset;

  /* The normalized name of MEMBER.  */
  const char *name;

  /* The length of NAME, without padding.  */
  bfd_size_type namlen;

  /* The length of NAME, with padding.  */
  bfd_size_type padded_namlen;

  /* The size of MEMBER's header, including the name and magic sequence.  */
  bfd_size_type header_size;

  /* The size of the MEMBER's contents.  */
  bfd_size_type contents_size;

  /* The number of bytes of padding that must be inserted after MEMBER
     in order to preserve even alignment.  */
  bfd_size_type trailing_padding;
};

/* A structure used for iterating over the members of an archive.  */
struct archive_iterator
{
  /* The archive itself.  */
  bfd *archive;

  /* Information about the current archive member.  */
  struct member_layout current;

  /* Information about the next archive member.  MEMBER is null if there
     are no more archive members, in which case OFFSET is the offset of
     the first unused byte.  */
  struct member_layout next;
};

/* Initialize INFO so that it describes member MEMBER of archive ARCHIVE.
   OFFSET is the even-padded offset of MEMBER, not including any leading
   padding needed for section alignment.  */

static void
member_layout_init (struct member_layout *info, bfd *archive,
		    bfd *member, file_ptr offset)
{
  info->member = member;
  info->leading_padding = 0;
  if (member)
    {
      info->name = normalize_filename (member);
      info->namlen = strlen (info->name);
      info->padded_namlen = info->namlen + (info->namlen & 1);
      if (xcoff_big_format_p (archive))
	info->header_size = SIZEOF_AR_HDR_BIG;
      else
	info->header_size = SIZEOF_AR_HDR;
      info->header_size += info->padded_namlen + SXCOFFARFMAG;
      info->contents_size = arelt_size (member);
      info->trailing_padding = info->contents_size & 1;

      if (bfd_check_format (member, bfd_object)
	  && bfd_get_flavour (member) == bfd_target_xcoff_flavour
	  && (member->flags & DYNAMIC) != 0)
	info->leading_padding
	  = (-(offset + info->header_size)
	     & ((1 << bfd_xcoff_text_align_power (member)) - 1));
    }
  info->offset = offset + info->leading_padding;
}

/* Set up ITERATOR to iterate through archive ARCHIVE.  */

static void
archive_iterator_begin (struct archive_iterator *iterator,
			bfd *archive)
{
  iterator->archive = archive;
  member_layout_init (&iterator->next, archive, archive->archive_head,
		      xcoff_big_format_p (archive)
		      ? SIZEOF_AR_FILE_HDR_BIG
		      : SIZEOF_AR_FILE_HDR);
}

/* Make ITERATOR visit the first unvisited archive member.  Return true
   on success; return false if all members have been visited.  */

static bool
archive_iterator_next (struct archive_iterator *iterator)
{
  if (!iterator->next.member)
    return false;

  iterator->current = iterator->next;
  member_layout_init (&iterator->next, iterator->archive,
		      iterator->current.member->archive_next,
		      iterator->current.offset
		      + iterator->current.header_size
		      + iterator->current.contents_size
		      + iterator->current.trailing_padding);
  return true;
}

/* We use our own tdata type.  Its first field is the COFF tdata type,
   so the COFF routines are compatible.  */

bool
_bfd_xcoff_mkobject (bfd *abfd)
{
  coff_data_type *coff;
  size_t amt = sizeof (struct xcoff_tdata);

  abfd->tdata.xcoff_obj_data = (struct xcoff_tdata *) bfd_zalloc (abfd, amt);
  if (abfd->tdata.xcoff_obj_data == NULL)
    return false;
  coff = coff_data (abfd);
  coff->symbols = (coff_symbol_type *) NULL;
  coff->conversion_table = (unsigned int *) NULL;
  coff->raw_syments = (struct coff_ptr_struct *) NULL;
  coff->relocbase = 0;

  xcoff_data (abfd)->modtype = ('1' << 8) | 'L';

  /* We set cputype to -1 to indicate that it has not been
     initialized.  */
  xcoff_data (abfd)->cputype = -1;

  xcoff_data (abfd)->csects = NULL;
  xcoff_data (abfd)->debug_indices = NULL;

  /* text section alignment is different than the default */
  bfd_xcoff_text_align_power (abfd) = 2;

  return true;
}

/* Copy XCOFF data from one BFD to another.  */

bool
_bfd_xcoff_copy_private_bfd_data (bfd *ibfd, bfd *obfd)
{
  struct xcoff_tdata *ix, *ox;
  asection *sec;

  if (ibfd->xvec != obfd->xvec)
    return true;
  ix = xcoff_data (ibfd);
  ox = xcoff_data (obfd);
  ox->full_aouthdr = ix->full_aouthdr;
  ox->toc = ix->toc;
  if (ix->sntoc == 0)
    ox->sntoc = 0;
  else
    {
      sec = coff_section_from_bfd_index (ibfd, ix->sntoc);
      if (sec == NULL || sec->output_section == NULL)
	ox->sntoc = 0;
      else
	ox->sntoc = sec->output_section->target_index;
    }
  if (ix->snentry == 0)
    ox->snentry = 0;
  else
    {
      sec = coff_section_from_bfd_index (ibfd, ix->snentry);
      if (sec == NULL || sec->output_section == NULL)
	ox->snentry = 0;
      else
	ox->snentry = sec->output_section->target_index;
    }
  bfd_xcoff_text_align_power (obfd) = bfd_xcoff_text_align_power (ibfd);
  bfd_xcoff_data_align_power (obfd) = bfd_xcoff_data_align_power (ibfd);
  ox->modtype = ix->modtype;
  ox->cputype = ix->cputype;
  ox->maxdata = ix->maxdata;
  ox->maxstack = ix->maxstack;
  return true;
}

/* I don't think XCOFF really has a notion of local labels based on
   name.  This will mean that ld -X doesn't actually strip anything.
   The AIX native linker does not have a -X option, and it ignores the
   -x option.  */

bool
_bfd_xcoff_is_local_label_name (bfd *abfd ATTRIBUTE_UNUSED,
				const char *name ATTRIBUTE_UNUSED)
{
  return false;
}

void
_bfd_xcoff_swap_sym_in (bfd *abfd, void * ext1, void * in1)
{
  SYMENT *ext = (SYMENT *)ext1;
  struct internal_syment * in = (struct internal_syment *)in1;

  if (ext->e.e_name[0] != 0)
    {
      memcpy (in->_n._n_name, ext->e.e_name, SYMNMLEN);
    }
  else
    {
      in->_n._n_n._n_zeroes = 0;
      in->_n._n_n._n_offset = H_GET_32 (abfd, ext->e.e.e_offset);
    }

  in->n_value = H_GET_32 (abfd, ext->e_value);
  in->n_scnum = (short) H_GET_16 (abfd, ext->e_scnum);
  in->n_type = H_GET_16 (abfd, ext->e_type);
  in->n_sclass = H_GET_8 (abfd, ext->e_sclass);
  in->n_numaux = H_GET_8 (abfd, ext->e_numaux);
}

unsigned int
_bfd_xcoff_swap_sym_out (bfd *abfd, void * inp, void * extp)
{
  struct internal_syment *in = (struct internal_syment *)inp;
  SYMENT *ext =(SYMENT *)extp;

  if (in->_n._n_name[0] != 0)
    {
      memcpy (ext->e.e_name, in->_n._n_name, SYMNMLEN);
    }
  else
    {
      H_PUT_32 (abfd, 0, ext->e.e.e_zeroes);
      H_PUT_32 (abfd, in->_n._n_n._n_offset, ext->e.e.e_offset);
    }

  H_PUT_32 (abfd, in->n_value, ext->e_value);
  H_PUT_16 (abfd, in->n_scnum, ext->e_scnum);
  H_PUT_16 (abfd, in->n_type, ext->e_type);
  H_PUT_8 (abfd, in->n_sclass, ext->e_sclass);
  H_PUT_8 (abfd, in->n_numaux, ext->e_numaux);
  return bfd_coff_symesz (abfd);
}

void
_bfd_xcoff_swap_aux_in (bfd *abfd, void * ext1, int type ATTRIBUTE_UNUSED,
			int in_class, int indx, int numaux, void * in1)
{
  AUXENT * ext = (AUXENT *)ext1;
  union internal_auxent *in = (union internal_auxent *)in1;

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
      if (ext->x_file.x_n.x_fname[0] == 0)
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
         have FCN ones too. In this case, CSECT is always the last
         one. */
    case C_EXT:
    case C_AIX_WEAKEXT:
    case C_HIDEXT:
      if (indx + 1 == numaux)
	{
	  in->x_csect.x_scnlen.u64 = H_GET_32 (abfd, ext->x_csect.x_scnlen);
	  in->x_csect.x_parmhash = H_GET_32 (abfd, ext->x_csect.x_parmhash);
	  in->x_csect.x_snhash   = H_GET_16 (abfd, ext->x_csect.x_snhash);
	  /* We don't have to hack bitfields in x_smtyp because it's
	     defined by shifts-and-ands, which are equivalent on all
	     byte orders.  */
	  in->x_csect.x_smtyp    = H_GET_8 (abfd, ext->x_csect.x_smtyp);
	  in->x_csect.x_smclas   = H_GET_8 (abfd, ext->x_csect.x_smclas);
	  in->x_csect.x_stab     = H_GET_32 (abfd, ext->x_csect.x_stab);
	  in->x_csect.x_snstab   = H_GET_16 (abfd, ext->x_csect.x_snstab);
	}
      else
	{
	  /* x_exptr isn't supported.  */
	  in->x_sym.x_misc.x_fsize
	    = H_GET_32 (abfd, ext->x_fcn.x_fsize);
	  in->x_sym.x_fcnary.x_fcn.x_lnnoptr
	    = H_GET_32 (abfd, ext->x_fcn.x_lnnoptr);
	  in->x_sym.x_fcnary.x_fcn.x_endndx.u32
	    = H_GET_32 (abfd, ext->x_fcn.x_endndx);
	}
      break;

    case C_STAT:
      in->x_scn.x_scnlen = H_GET_32 (abfd, ext->x_scn.x_scnlen);
      in->x_scn.x_nreloc = H_GET_16 (abfd, ext->x_scn.x_nreloc);
      in->x_scn.x_nlinno = H_GET_16 (abfd, ext->x_scn.x_nlinno);
      /* PE defines some extra fields; we zero them out for
	 safety.  */
      in->x_scn.x_checksum = 0;
      in->x_scn.x_associated = 0;
      in->x_scn.x_comdat = 0;
      break;

    case C_BLOCK:
    case C_FCN:
      in->x_sym.x_misc.x_lnsz.x_lnno
	= H_GET_32 (abfd, ext->x_sym.x_lnno);
      break;

    case C_DWARF:
      in->x_sect.x_scnlen = H_GET_32 (abfd, ext->x_sect.x_scnlen);
      in->x_sect.x_nreloc = H_GET_32 (abfd, ext->x_sect.x_nreloc);
      break;

    }
}

unsigned int
_bfd_xcoff_swap_aux_out (bfd *abfd, void * inp, int type ATTRIBUTE_UNUSED,
			 int in_class, int indx, int numaux, void * extp)
{
  union internal_auxent *in = (union internal_auxent *)inp;
  AUXENT *ext = (AUXENT *)extp;

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
      if (in->x_file.x_n.x_fname[0] == 0)
	{
	  H_PUT_32 (abfd, 0, ext->x_file.x_n.x_n.x_zeroes);
	  H_PUT_32 (abfd, in->x_file.x_n.x_n.x_offset,
		    ext->x_file.x_n.x_n.x_offset);
	}
      else
	memcpy (ext->x_file.x_n.x_fname, in->x_file.x_n.x_fname, FILNMLEN);
      H_PUT_8 (abfd, in->x_file.x_ftype, ext->x_file.x_ftype);
      break;

      /* RS/6000 "csect" auxents */
    case C_EXT:
    case C_AIX_WEAKEXT:
    case C_HIDEXT:
      if (indx + 1 == numaux)
	{
	  H_PUT_32 (abfd, in->x_csect.x_scnlen.u64, ext->x_csect.x_scnlen);
	  H_PUT_32 (abfd, in->x_csect.x_parmhash, ext->x_csect.x_parmhash);
	  H_PUT_16 (abfd, in->x_csect.x_snhash, ext->x_csect.x_snhash);
	  /* We don't have to hack bitfields in x_smtyp because it's
	     defined by shifts-and-ands, which are equivalent on all
	     byte orders.  */
	  H_PUT_8 (abfd, in->x_csect.x_smtyp, ext->x_csect.x_smtyp);
	  H_PUT_8 (abfd, in->x_csect.x_smclas, ext->x_csect.x_smclas);
	  H_PUT_32 (abfd, in->x_csect.x_stab, ext->x_csect.x_stab);
	  H_PUT_16 (abfd, in->x_csect.x_snstab, ext->x_csect.x_snstab);
	}
      else
	{
	  H_PUT_32 (abfd, in->x_sym.x_misc.x_fsize, ext->x_fcn.x_fsize);
	  H_PUT_32 (abfd, in->x_sym.x_fcnary.x_fcn.x_lnnoptr,
		    ext->x_fcn.x_lnnoptr);
	  H_PUT_32 (abfd, in->x_sym.x_fcnary.x_fcn.x_endndx.u32,
		    ext->x_fcn.x_endndx);
	}
      break;

    case C_STAT:
      H_PUT_32 (abfd, in->x_scn.x_scnlen, ext->x_scn.x_scnlen);
      H_PUT_16 (abfd, in->x_scn.x_nreloc, ext->x_scn.x_nreloc);
      H_PUT_16 (abfd, in->x_scn.x_nlinno, ext->x_scn.x_nlinno);
      break;

    case C_BLOCK:
    case C_FCN:
      H_PUT_32 (abfd, in->x_sym.x_misc.x_lnsz.x_lnno, ext->x_sym.x_lnno);
      break;

    case C_DWARF:
      H_PUT_32 (abfd, in->x_sect.x_scnlen, ext->x_sect.x_scnlen);
      H_PUT_32 (abfd, in->x_sect.x_nreloc, ext->x_sect.x_nreloc);
      break;
    }

  return bfd_coff_auxesz (abfd);
}

/* The XCOFF reloc table.
   XCOFF relocations aren't defined only by the type field r_type.
   The bitsize and whether they are signed or not, are defined by
   r_size field.  Thus, it's complicated to create a constant
   table reference every possible relocation.
   This table contains the "default" relocation and few modified
   relocations what were already there.  It's enough when
   xcoff_rtype2howto is called.
   For relocations from an input bfd to an output bfd, the default
   relocation is retrieved and when manually adapted.

   For now, it seems to be enought.  */

reloc_howto_type xcoff_howto_table[] =
{
  /* 0x00: Standard 32 bit relocation.  */
  HOWTO (R_POS,			/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 0,			/* special_function */
	 "R_POS",		/* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 0x01: 32 bit relocation, but store negative value.  */
  HOWTO (R_NEG,			/* type */
	 0,			/* rightshift */
	 -4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 0,			/* special_function */
	 "R_NEG",		/* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 0x02: 32 bit PC relative relocation.  */
  HOWTO (R_REL,			/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 true,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 0,			/* special_function */
	 "R_REL",		/* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
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

  /* 0x04: Same as R_TOC  */
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
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 0,			/* special_function */
	 "R_RL",		/* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 0x0d: Same as R_POS.  */
  HOWTO (R_RLA,			/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 0,			/* special_function */
	 "R_RLA",		/* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
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

  /* 0x13: Same as R_TOC.  */
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
	 0,			/* src_mask */
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

  /* 0x1c: 16 bit Non modifiable absolute branch.  */
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

  /* 0x1d: Modifiable branch relative.  */
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

  /* 0x1e: Modifiable branch relative.  */
  HOWTO (R_RBA,			/* type */
	 0,			/* rightshift */
	 2,			/* size */
	 16,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 0,			/* special_function */
	 "R_RBA_16",		/* name */
	 true,			/* partial_inplace */
	 0xffff,		/* src_mask */
	 0xffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  EMPTY_HOWTO (0x1f),

  /* 0x20: General-dynamic TLS relocation.  */
  HOWTO (R_TLS,			/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 0,			/* special_function */
	 "R_TLS",		/* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 0x21: Initial-exec TLS relocation.  */
  HOWTO (R_TLS_IE,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 0,			/* special_function */
	 "R_TLS_IE",		/* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 0x22: Local-dynamic TLS relocation.  */
  HOWTO (R_TLS_LD,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 0,			/* special_function */
	 "R_TLS_LD",		/* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 0x23: Local-exec TLS relocation.  */
  HOWTO (R_TLS_LE,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 0,			/* special_function */
	 "R_TLS_LE",		/* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  /* 0x24: TLS relocation.  */
  HOWTO (R_TLSM,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 0,			/* special_function */
	 "R_TLSM",		/* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */


  /* 0x25: TLS module relocation.  */
  HOWTO (R_TLSML,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 0,			/* special_function */
	 "R_TLSML",		/* name */
	 true,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */

  EMPTY_HOWTO(0x26),
  EMPTY_HOWTO(0x27),
  EMPTY_HOWTO(0x28),
  EMPTY_HOWTO(0x29),
  EMPTY_HOWTO(0x2a),
  EMPTY_HOWTO(0x2b),
  EMPTY_HOWTO(0x2c),
  EMPTY_HOWTO(0x2d),
  EMPTY_HOWTO(0x2e),
  EMPTY_HOWTO(0x2f),

  /* 0x30: High-order 16 bit TOC relative relocation.  */
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
xcoff_rtype2howto (arelent *relent, struct internal_reloc *internal)
{
  if (internal->r_type > R_TOCL)
    abort ();

  /* Default howto layout works most of the time */
  relent->howto = &xcoff_howto_table[internal->r_type];

  /* Special case some 16 bit reloc */
  if (15 == (internal->r_size & 0x1f))
    {
      if (R_BA == internal->r_type)
	relent->howto = &xcoff_howto_table[0x1c];
      else if (R_RBR == internal->r_type)
	relent->howto = &xcoff_howto_table[0x1d];
      else if (R_RBA == internal->r_type)
	relent->howto = &xcoff_howto_table[0x1e];
    }

  /* The r_size field of an XCOFF reloc encodes the bitsize of the
     relocation, as well as indicating whether it is signed or not.
     Doublecheck that the relocation information gathered from the
     type matches this information.  The bitsize is not significant
     for R_REF relocs.  */
  if (relent->howto->dst_mask != 0
      && (relent->howto->bitsize
	  != ((unsigned int) internal->r_size & 0x1f) + 1))
    abort ();
}

reloc_howto_type *
_bfd_xcoff_reloc_type_lookup (bfd *abfd ATTRIBUTE_UNUSED,
			      bfd_reloc_code_real_type code)
{
  switch (code)
    {
    case BFD_RELOC_PPC_B26:
      return &xcoff_howto_table[0xa];
    case BFD_RELOC_PPC_BA16:
      return &xcoff_howto_table[0x1c];
    case BFD_RELOC_PPC_BA26:
      return &xcoff_howto_table[8];
    case BFD_RELOC_PPC_TOC16:
      return &xcoff_howto_table[3];
    case BFD_RELOC_PPC_TOC16_HI:
      return &xcoff_howto_table[0x30];
    case BFD_RELOC_PPC_TOC16_LO:
      return &xcoff_howto_table[0x31];
    case BFD_RELOC_PPC_B16:
      return &xcoff_howto_table[0x1d];
    case BFD_RELOC_32:
    case BFD_RELOC_CTOR:
      return &xcoff_howto_table[0];
    case BFD_RELOC_NONE:
      return &xcoff_howto_table[0xf];
    case BFD_RELOC_PPC_NEG:
      return &xcoff_howto_table[0x1];
    case BFD_RELOC_PPC_TLSGD:
      return &xcoff_howto_table[0x20];
    case BFD_RELOC_PPC_TLSIE:
      return &xcoff_howto_table[0x21];
    case BFD_RELOC_PPC_TLSLD:
      return &xcoff_howto_table[0x22];
    case BFD_RELOC_PPC_TLSLE:
      return &xcoff_howto_table[0x23];
    case BFD_RELOC_PPC_TLSM:
      return &xcoff_howto_table[0x24];
    case BFD_RELOC_PPC_TLSML:
      return &xcoff_howto_table[0x25];
    default:
      return NULL;
    }
}

static reloc_howto_type *
_bfd_xcoff_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED,
			      const char *r_name)
{
  unsigned int i;

  for (i = 0;
       i < sizeof (xcoff_howto_table) / sizeof (xcoff_howto_table[0]);
       i++)
    if (xcoff_howto_table[i].name != NULL
	&& strcasecmp (xcoff_howto_table[i].name, r_name) == 0)
      return &xcoff_howto_table[i];

  return NULL;
}

/* XCOFF archive support.  The original version of this code was by
   Damon A. Permezel.  It was enhanced to permit cross support, and
   writing archive files, by Ian Lance Taylor, Cygnus Support.

   XCOFF uses its own archive format.  Everything is hooked together
   with file offset links, so it is possible to rapidly update an
   archive in place.  Of course, we don't do that.  An XCOFF archive
   has a real file header, not just an ARMAG string.  The structure of
   the file header and of each archive header appear below.

   An XCOFF archive also has a member table, which is a list of
   elements in the archive (you can get that by looking through the
   linked list, but you have to read a lot more of the file).  The
   member table has a normal archive header with an empty name.  It is
   normally (and perhaps must be) the second to last entry in the
   archive.  The member table data is almost printable ASCII.  It
   starts with a 12 character decimal string which is the number of
   entries in the table.  For each entry it has a 12 character decimal
   string which is the offset in the archive of that member.  These
   entries are followed by a series of null terminated strings which
   are the member names for each entry.

   Finally, an XCOFF archive has a global symbol table, which is what
   we call the armap.  The global symbol table has a normal archive
   header with an empty name.  It is normally (and perhaps must be)
   the last entry in the archive.  The contents start with a four byte
   binary number which is the number of entries.  This is followed by
   a that many four byte binary numbers; each is the file offset of an
   entry in the archive.  These numbers are followed by a series of
   null terminated strings, which are symbol names.

   AIX 4.3 introduced a new archive format which can handle larger
   files and also 32- and 64-bit objects in the same archive.  The
   things said above remain true except that there is now more than
   one global symbol table.  The one is used to index 32-bit objects,
   the other for 64-bit objects.

   The new archives (recognizable by the new ARMAG string) has larger
   field lengths so that we cannot really share any code.  Also we have
   to take care that we are not generating the new form of archives
   on AIX 4.2 or earlier systems.  */

/* PR 21786:  The PE/COFF standard does not require NUL termination for any of
   the ASCII fields in the archive headers.  So in order to be able to extract
   numerical values we provide our own versions of strtol and strtoll which
   take a maximum length as an additional parameter.  Also - just to save space,
   we omit the endptr return parameter, since we know that it is never used.  */

static unsigned long
_bfd_strntol (const char * nptr, int base, unsigned int maxlen)
{
  char buf[24]; /* Should be enough.  */

  BFD_ASSERT (maxlen < (sizeof (buf) - 1));

  memcpy (buf, nptr, maxlen);
  buf[maxlen] = 0;
  return strtol (buf, NULL, base);
}

static unsigned long long
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

#define EQ_VALUE_IN_FIELD(VAR, FIELD, BASE)			\
  (sizeof (VAR) > sizeof (long)					\
   ? (VAR) == _bfd_strntoll (FIELD, BASE, sizeof FIELD)		\
   : (VAR) == _bfd_strntol (FIELD, BASE, sizeof FIELD))

/* Read in the armap of an XCOFF archive.  */

bool
_bfd_xcoff_slurp_armap (bfd *abfd)
{
  ufile_ptr off;
  size_t namlen;
  bfd_size_type sz;
  bfd_byte *contents, *cend;
  bfd_vma c, i;
  carsym *arsym;
  bfd_byte *p;

  if (x_artdata (abfd) == NULL)
    {
      abfd->has_armap = false;
      return true;
    }

  if (! xcoff_big_format_p (abfd))
    {
      /* This is for the old format.  */
      struct xcoff_ar_hdr hdr;

      GET_VALUE_IN_FIELD (off, x_artdata (abfd)->u.hdr.symoff, 10);
      if (off == 0)
	{
	  abfd->has_armap = false;
	  return true;
	}

      if (bfd_seek (abfd, off, SEEK_SET) != 0)
	return false;

      /* The symbol table starts with a normal archive header.  */
      if (bfd_bread (&hdr, (bfd_size_type) SIZEOF_AR_HDR, abfd)
	  != SIZEOF_AR_HDR)
	return false;

      /* Skip the name (normally empty).  */
      GET_VALUE_IN_FIELD (namlen, hdr.namlen, 10);
      off = ((namlen + 1) & ~ (size_t) 1) + SXCOFFARFMAG;
      if (bfd_seek (abfd, off, SEEK_CUR) != 0)
	return false;

      GET_VALUE_IN_FIELD (sz, hdr.size, 10);
      if (sz + 1 < 5)
	{
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}

      /* Read in the entire symbol table.  */
      contents = (bfd_byte *) _bfd_alloc_and_read (abfd, sz + 1, sz);
      if (contents == NULL)
	return false;

      /* Ensure strings are NULL terminated so we don't wander off the
	 end of the buffer.  */
      contents[sz] = 0;

      /* The symbol table starts with a four byte count.  */
      c = H_GET_32 (abfd, contents);

      if (c >= sz / 4)
	{
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}

      bfd_ardata (abfd)->symdefs =
	((carsym *) bfd_alloc (abfd, c * sizeof (carsym)));
      if (bfd_ardata (abfd)->symdefs == NULL)
	return false;

      /* After the count comes a list of four byte file offsets.  */
      for (i = 0, arsym = bfd_ardata (abfd)->symdefs, p = contents + 4;
	   i < c;
	   ++i, ++arsym, p += 4)
	arsym->file_offset = H_GET_32 (abfd, p);
    }
  else
    {
      /* This is for the new format.  */
      struct xcoff_ar_hdr_big hdr;

      GET_VALUE_IN_FIELD (off, x_artdata (abfd)->u.bhdr.symoff, 10);
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
      off = ((namlen + 1) & ~ (size_t) 1) + SXCOFFARFMAG;
      if (bfd_seek (abfd, off, SEEK_CUR) != 0)
	return false;

      GET_VALUE_IN_FIELD (sz, hdr.size, 10);
      if (sz + 1 < 9)
	{
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}

      /* Read in the entire symbol table.  */
      contents = (bfd_byte *) _bfd_alloc_and_read (abfd, sz + 1, sz);
      if (contents == NULL)
	return false;

      /* Ensure strings are NULL terminated so we don't wander off the
	 end of the buffer.  */
      contents[sz] = 0;

      /* The symbol table starts with an eight byte count.  */
      c = H_GET_64 (abfd, contents);

      if (c >= sz / 8)
	{
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}

      bfd_ardata (abfd)->symdefs =
	((carsym *) bfd_alloc (abfd, c * sizeof (carsym)));
      if (bfd_ardata (abfd)->symdefs == NULL)
	return false;

      /* After the count comes a list of eight byte file offsets.  */
      for (i = 0, arsym = bfd_ardata (abfd)->symdefs, p = contents + 8;
	   i < c;
	   ++i, ++arsym, p += 8)
	arsym->file_offset = H_GET_64 (abfd, p);
    }

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

/* See if this is an XCOFF archive.  */

bfd_cleanup
_bfd_xcoff_archive_p (bfd *abfd)
{
  struct artdata *tdata_hold;
  char magic[SXCOFFARMAG];
  size_t amt = SXCOFFARMAG;

  if (bfd_bread (magic, amt, abfd) != amt)
    {
      if (bfd_get_error () != bfd_error_system_call)
	bfd_set_error (bfd_error_wrong_format);
      return NULL;
    }

  if (strncmp (magic, XCOFFARMAG, SXCOFFARMAG) != 0
      && strncmp (magic, XCOFFARMAGBIG, SXCOFFARMAG) != 0)
    {
      bfd_set_error (bfd_error_wrong_format);
      return NULL;
    }

  tdata_hold = bfd_ardata (abfd);

  amt = sizeof (struct artdata);
  bfd_ardata (abfd) = (struct artdata *) bfd_zalloc (abfd, amt);
  if (bfd_ardata (abfd) == (struct artdata *) NULL)
    goto error_ret_restore;

  /* Now handle the two formats.  */
  if (magic[1] != 'b')
    {
      /* This is the old format.  */
      struct xcoff_ar_file_hdr hdr;

      /* Copy over the magic string.  */
      memcpy (hdr.magic, magic, SXCOFFARMAG);

      /* Now read the rest of the file header.  */
      amt = SIZEOF_AR_FILE_HDR - SXCOFFARMAG;
      if (bfd_bread (&hdr.memoff, amt, abfd) != amt)
	{
	  if (bfd_get_error () != bfd_error_system_call)
	    bfd_set_error (bfd_error_wrong_format);
	  goto error_ret;
	}

      GET_VALUE_IN_FIELD (bfd_ardata (abfd)->first_file_filepos,
			  hdr.firstmemoff, 10);

      amt = sizeof (struct xcoff_artdata);
      bfd_ardata (abfd)->tdata = bfd_zalloc (abfd, amt);
      if (bfd_ardata (abfd)->tdata == NULL)
	goto error_ret;

      memcpy (&x_artdata (abfd)->u.hdr, &hdr, SIZEOF_AR_FILE_HDR);
    }
  else
    {
      /* This is the new format.  */
      struct xcoff_ar_file_hdr_big hdr;

      /* Copy over the magic string.  */
      memcpy (hdr.magic, magic, SXCOFFARMAG);

      /* Now read the rest of the file header.  */
      amt = SIZEOF_AR_FILE_HDR_BIG - SXCOFFARMAG;
      if (bfd_bread (&hdr.memoff, amt, abfd) != amt)
	{
	  if (bfd_get_error () != bfd_error_system_call)
	    bfd_set_error (bfd_error_wrong_format);
	  goto error_ret;
	}

      bfd_ardata (abfd)->first_file_filepos = bfd_scan_vma (hdr.firstmemoff,
							    (const char **) 0,
							    10);

      amt = sizeof (struct xcoff_artdata);
      bfd_ardata (abfd)->tdata = bfd_zalloc (abfd, amt);
      if (bfd_ardata (abfd)->tdata == NULL)
	goto error_ret;

      memcpy (&x_artdata (abfd)->u.bhdr, &hdr, SIZEOF_AR_FILE_HDR_BIG);
    }

  if (! _bfd_xcoff_slurp_armap (abfd))
    {
    error_ret:
      bfd_release (abfd, bfd_ardata (abfd));
    error_ret_restore:
      bfd_ardata (abfd) = tdata_hold;
      return NULL;
    }

  return _bfd_no_cleanup;
}

/* Track file ranges occupied by elements.  Add [START,END) to the
   list of ranges and return TRUE if there is no overlap between the
   new and any other element or the archive file header.  This is
   aimed at preventing infinite looping on malformed archives, for
   "ar" and similar which typically use code like:
   .  for (last = bfd_openr_next_archived_file (archive, NULL);
   .       last;
   .       last = next)
   .    {
   .      do_something_with (last);
   .      next = bfd_openr_next_archived_file (archive, last);
   .      bfd_close (last);
   .    }
   The check implemented here is only possible due to the fact that
   for XCOFF archives bfd_openr_next_archived_file is the only code
   path leading to _bfd_read_ar_hdr.  _bfd_read_ar_hdr is not called
   when reading the armap, nor do XCOFF archives use the extended name
   scheme implemented in archive.c.

   Note that the check relies on the previous element being closed,
   and there is one case where add_range might fail but I think it is
   sufficently unusual that it doesn't warrant fixing:
   If the loop body above called bfd_openr_next_archived_file twice
   with the same arguments and the element returned is bfd_close'd
   between those calls then we'll return false here for the second
   call.  (For why this is so see _bfd_look_for_bfd_in_cache in
   _bfd_get_elt_at_filepos, and know that bfd_close removes elements
   from the cache.)  */

static bool
add_range (bfd *abfd, ufile_ptr start, ufile_ptr end)
{
  if (end <= start)
    {
    err:
      bfd_set_error (bfd_error_malformed_archive);
      return false;
    }

  /* This list is kept sorted by address.  Find the highest address
     range on the list that ends before the new range starts.  Exit
     the loop with that range in LO, and the mext higher range in HI.  */
  struct ar_ranges *hi = &x_artdata (abfd)->ranges;
  struct ar_ranges *lo = NULL;
  while (hi && hi->end <= start)
    {
      lo = hi;
      hi = hi->next;
    }

  if (lo == NULL)
    /* Start overlaps the file header or elements adjacent to it.  */
    goto err;

  if (hi && hi->start < end)
    /* Overlap with another element.  */
    goto err;

  /* A zero size element with a one char name is this big.  */
  unsigned min_elt = x_artdata (abfd)->ar_hdr_size + 2 + SXCOFFARFMAG;
  if (start - lo->end < min_elt)
    {
      /* Merge into an existing range.  */
      lo->end = end;
      if (hi && hi->start - end < min_elt)
	{
	  /* In fact, we can merge two ranges.  */
	  lo->end = hi->end;
	  lo->next = hi->next;
	  /* The list uses bfd_alloc so don't free HI.  */
	}
      return true;
    }

  if (hi && hi->start - end < min_elt)
    {
      /* Merge into an existing range.  */
      hi->start = start;
      return true;
    }

  struct ar_ranges *newr = bfd_alloc (abfd, sizeof (*newr));
  if (newr == NULL)
    return false;
  newr->start = start;
  newr->end = end;
  newr->next = hi;
  lo->next = newr;
  return true;
}

/* Read the archive header in an XCOFF archive.  */

void *
_bfd_xcoff_read_ar_hdr (bfd *abfd)
{
  bfd_size_type namlen;
  struct areltdata *ret;
  bfd_size_type amt;
  ufile_ptr start = abfd->where;

  if (! xcoff_big_format_p (abfd))
    {
      struct xcoff_ar_hdr hdr;
      struct xcoff_ar_hdr *hdrp;

      if (bfd_bread (&hdr, SIZEOF_AR_HDR, abfd) != SIZEOF_AR_HDR)
	return NULL;

      GET_VALUE_IN_FIELD (namlen, hdr.namlen, 10);
      if (namlen > bfd_get_file_size (abfd))
	return NULL;
      amt = sizeof (struct areltdata) + SIZEOF_AR_HDR + namlen + 1;
      ret = (struct areltdata *) bfd_malloc (amt);
      if (ret == NULL)
	return ret;

      hdrp = (struct xcoff_ar_hdr *) (ret + 1);
      memcpy (hdrp, &hdr, SIZEOF_AR_HDR);
      if (bfd_bread ((char *) hdrp + SIZEOF_AR_HDR, namlen, abfd) != namlen)
	{
	  free (ret);
	  return NULL;
	}
      ((char *) hdrp)[SIZEOF_AR_HDR + namlen] = '\0';

      ret->arch_header = (char *) hdrp;
      GET_VALUE_IN_FIELD (ret->parsed_size, hdr.size, 10);
      ret->filename = (char *) hdrp + SIZEOF_AR_HDR;
    }
  else
    {
      struct xcoff_ar_hdr_big hdr;
      struct xcoff_ar_hdr_big *hdrp;

      if (bfd_bread (&hdr, SIZEOF_AR_HDR_BIG, abfd) != SIZEOF_AR_HDR_BIG)
	return NULL;

      GET_VALUE_IN_FIELD (namlen, hdr.namlen, 10);
      if (namlen > bfd_get_file_size (abfd))
	return NULL;
      amt = sizeof (struct areltdata) + SIZEOF_AR_HDR_BIG + namlen + 1;
      ret = (struct areltdata *) bfd_malloc (amt);
      if (ret == NULL)
	return ret;

      hdrp = (struct xcoff_ar_hdr_big *) (ret + 1);
      memcpy (hdrp, &hdr, SIZEOF_AR_HDR_BIG);
      if (bfd_bread ((char *) hdrp + SIZEOF_AR_HDR_BIG, namlen, abfd) != namlen)
	{
	  free (ret);
	  return NULL;
	}
      ((char *) hdrp)[SIZEOF_AR_HDR_BIG + namlen] = '\0';

      ret->arch_header = (char *) hdrp;
      GET_VALUE_IN_FIELD (ret->parsed_size, hdr.size, 10);
      ret->filename = (char *) hdrp + SIZEOF_AR_HDR_BIG;
    }

  /* Size occupied by the header above that covered in the fixed
     SIZEOF_AR_HDR or SIZEOF_AR_HDR_BIG.  */
  ret->extra_size = namlen + (namlen & 1) + SXCOFFARFMAG;

  /* Skip over the XCOFFARFMAG at the end of the file name.  */
  if (bfd_seek (abfd, (namlen & 1) + SXCOFFARFMAG, SEEK_CUR) != 0
      || !add_range (abfd, start, abfd->where + ret->parsed_size))
    {
      free (ret);
      return NULL;
    }

  return ret;
}

/* Open the next element in an XCOFF archive.  */

bfd *
_bfd_xcoff_openr_next_archived_file (bfd *archive, bfd *last_file)
{
  ufile_ptr filestart;

  if (x_artdata (archive) == NULL)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return NULL;
    }

  if (! xcoff_big_format_p (archive))
    {
      if (last_file == NULL)
	{
	  /* If we are scanning over elements twice in an open archive,
	     which can happen in gdb after a fork, ensure we start the
	     second scan with clean ranges.  */
	  x_artdata (archive)->ranges.start = 0;
	  x_artdata (archive)->ranges.end = SIZEOF_AR_FILE_HDR;
	  x_artdata (archive)->ranges.next = NULL;
	  x_artdata (archive)->ar_hdr_size = SIZEOF_AR_HDR;
	  filestart = bfd_ardata (archive)->first_file_filepos;
	}
      else
	GET_VALUE_IN_FIELD (filestart, arch_xhdr (last_file)->nextoff, 10);

      if (filestart == 0
	  || EQ_VALUE_IN_FIELD (filestart,
				x_artdata (archive)->u.hdr.memoff, 10)
	  || EQ_VALUE_IN_FIELD (filestart,
				x_artdata (archive)->u.hdr.symoff, 10))
	{
	  bfd_set_error (bfd_error_no_more_archived_files);
	  return NULL;
	}
    }
  else
    {
      if (last_file == NULL)
	{
	  x_artdata (archive)->ranges.start = 0;
	  x_artdata (archive)->ranges.end = SIZEOF_AR_FILE_HDR_BIG;
	  x_artdata (archive)->ranges.next = NULL;
	  x_artdata (archive)->ar_hdr_size = SIZEOF_AR_HDR_BIG;
	  filestart = bfd_ardata (archive)->first_file_filepos;
	}
      else
	GET_VALUE_IN_FIELD (filestart, arch_xhdr_big (last_file)->nextoff, 10);

      if (filestart == 0
	  || EQ_VALUE_IN_FIELD (filestart,
				x_artdata (archive)->u.bhdr.memoff, 10)
	  || EQ_VALUE_IN_FIELD (filestart,
				x_artdata (archive)->u.bhdr.symoff, 10))
	{
	  bfd_set_error (bfd_error_no_more_archived_files);
	  return NULL;
	}
    }

  /* Check that we aren't pointing back at the last element.  This is
     necessary depite the add_range checking in _bfd_xcoff_read_ar_hdr
     because archive.c leaves the last element open and thus in the
     archive element cache until the next element is opened.  */
  if (last_file != NULL)
    {
      ufile_ptr laststart = last_file->proxy_origin;
      laststart -= x_artdata (archive)->ar_hdr_size;
      laststart -= arch_eltdata (last_file)->extra_size;
      if (filestart == laststart)
	{
	  bfd_set_error (bfd_error_malformed_archive);
	  return NULL;
	}
    }

  return _bfd_get_elt_at_filepos (archive, filestart, NULL);
}

/* Stat an element in an XCOFF archive.  */

int
_bfd_xcoff_stat_arch_elt (bfd *abfd, struct stat *s)
{
  if (abfd->arelt_data == NULL)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return -1;
    }

  if (! xcoff_big_format_p (abfd->my_archive))
    {
      struct xcoff_ar_hdr *hdrp = arch_xhdr (abfd);

      GET_VALUE_IN_FIELD (s->st_mtime, hdrp->date, 10);
      GET_VALUE_IN_FIELD (s->st_uid, hdrp->uid, 10);
      GET_VALUE_IN_FIELD (s->st_gid, hdrp->gid, 10);
      GET_VALUE_IN_FIELD (s->st_mode, hdrp->mode, 8);
      s->st_size = arch_eltdata (abfd)->parsed_size;
    }
  else
    {
      struct xcoff_ar_hdr_big *hdrp = arch_xhdr_big (abfd);

      GET_VALUE_IN_FIELD (s->st_mtime, hdrp->date, 10);
      GET_VALUE_IN_FIELD (s->st_uid, hdrp->uid, 10);
      GET_VALUE_IN_FIELD (s->st_gid, hdrp->gid, 10);
      GET_VALUE_IN_FIELD (s->st_mode, hdrp->mode, 8);
      s->st_size = arch_eltdata (abfd)->parsed_size;
    }

  return 0;
}

/* Normalize a file name for inclusion in an archive.  */

static const char *
normalize_filename (bfd *abfd)
{
  const char *file;
  const char *filename;

  file = bfd_get_filename (abfd);
  filename = strrchr (file, '/');
  if (filename != NULL)
    filename++;
  else
    filename = file;
  return filename;
}

/* Write out an XCOFF armap.  */

static bool
xcoff_write_armap_old (bfd *abfd, unsigned int elength ATTRIBUTE_UNUSED,
		       struct orl *map, unsigned int orl_count, int stridx)
{
  struct archive_iterator iterator;
  struct xcoff_ar_hdr hdr;
  char *p;
  unsigned char buf[4];
  unsigned int i;

  memset (&hdr, 0, sizeof hdr);
  sprintf (hdr.size, "%ld", (long) (4 + orl_count * 4 + stridx));
  sprintf (hdr.nextoff, "%d", 0);
  memcpy (hdr.prevoff, x_artdata (abfd)->u.hdr.memoff,
	  XCOFFARMAG_ELEMENT_SIZE);
  sprintf (hdr.date, "%d", 0);
  sprintf (hdr.uid, "%d", 0);
  sprintf (hdr.gid, "%d", 0);
  sprintf (hdr.mode, "%d", 0);
  sprintf (hdr.namlen, "%d", 0);

  /* We need spaces, not null bytes, in the header.  */
  for (p = (char *) &hdr; p < (char *) &hdr + SIZEOF_AR_HDR; p++)
    if (*p == '\0')
      *p = ' ';

  if (bfd_bwrite (&hdr, (bfd_size_type) SIZEOF_AR_HDR, abfd)
      != SIZEOF_AR_HDR
      || (bfd_bwrite (XCOFFARFMAG, (bfd_size_type) SXCOFFARFMAG, abfd)
	  != SXCOFFARFMAG))
    return false;

  H_PUT_32 (abfd, orl_count, buf);
  if (bfd_bwrite (buf, (bfd_size_type) 4, abfd) != 4)
    return false;

  i = 0;
  archive_iterator_begin (&iterator, abfd);
  while (i < orl_count && archive_iterator_next (&iterator))
    while (map[i].u.abfd == iterator.current.member)
      {
	H_PUT_32 (abfd, iterator.current.offset, buf);
	if (bfd_bwrite (buf, (bfd_size_type) 4, abfd) != 4)
	  return false;
	++i;
      }

  for (i = 0; i < orl_count; i++)
    {
      const char *name;
      size_t namlen;

      name = *map[i].name;
      namlen = strlen (name);
      if (bfd_bwrite (name, (bfd_size_type) (namlen + 1), abfd) != namlen + 1)
	return false;
    }

  if ((stridx & 1) != 0)
    {
      char b;

      b = '\0';
      if (bfd_bwrite (&b, (bfd_size_type) 1, abfd) != 1)
	return false;
    }

  return true;
}

static char buff20[XCOFFARMAGBIG_ELEMENT_SIZE + 1];
#define FMT20  "%-20" PRId64
#define FMT12  "%-12d"
#define FMT12_OCTAL  "%-12o"
#define FMT4  "%-4d"
#define PRINT20(d, v) \
  sprintf (buff20, FMT20, (uint64_t) (v)), \
  memcpy ((void *) (d), buff20, 20)

#define PRINT12(d, v) \
  sprintf (buff20, FMT12, (int)(v)), \
  memcpy ((void *) (d), buff20, 12)

#define PRINT12_OCTAL(d, v) \
  sprintf (buff20, FMT12_OCTAL, (unsigned int)(v)), \
  memcpy ((void *) (d), buff20, 12)

#define PRINT4(d, v) \
  sprintf (buff20, FMT4, (int)(v)), \
  memcpy ((void *) (d), buff20, 4)

#define READ20(d, v) \
  buff20[20] = 0, \
  memcpy (buff20, (d), 20), \
  (v) = bfd_scan_vma (buff20, (const char **) NULL, 10)

static bool
do_pad (bfd *abfd, unsigned int number)
{
  bfd_byte b = 0;

  /* Limit pad to <= 4096.  */
  if (number > 4096)
    return false;

  while (number--)
    if (bfd_bwrite (&b, (bfd_size_type) 1, abfd) != 1)
      return false;

  return true;
}

static bool
do_copy (bfd *out_bfd, bfd *in_bfd)
{
  bfd_size_type remaining;
  bfd_byte buffer[DEFAULT_BUFFERSIZE];

  if (bfd_seek (in_bfd, (file_ptr) 0, SEEK_SET) != 0)
    return false;

  remaining = arelt_size (in_bfd);

  while (remaining >= DEFAULT_BUFFERSIZE)
    {
      if (bfd_bread (buffer, DEFAULT_BUFFERSIZE, in_bfd) != DEFAULT_BUFFERSIZE
	  || bfd_bwrite (buffer, DEFAULT_BUFFERSIZE, out_bfd) != DEFAULT_BUFFERSIZE)
	return false;

      remaining -= DEFAULT_BUFFERSIZE;
    }

  if (remaining)
    {
      if (bfd_bread (buffer, remaining, in_bfd) != remaining
	  || bfd_bwrite (buffer, remaining, out_bfd) != remaining)
	return false;
    }

  return true;
}

static bool
xcoff_write_armap_big (bfd *abfd, unsigned int elength ATTRIBUTE_UNUSED,
		       struct orl *map, unsigned int orl_count, int stridx)
{
  struct archive_iterator iterator;
  struct xcoff_ar_file_hdr_big *fhdr;
  bfd_vma i, sym_32, sym_64, str_32, str_64;
  const bfd_arch_info_type *arch_info;
  bfd *current_bfd;
  size_t string_length;
  file_ptr nextoff, prevoff;

  /* First, we look through the symbols and work out which are
     from 32-bit objects and which from 64-bit ones.  */
  sym_32 = sym_64 = str_32 = str_64 = 0;

  i = 0;
  for (current_bfd = abfd->archive_head;
       current_bfd != NULL && i < orl_count;
       current_bfd = current_bfd->archive_next)
    {
      arch_info = bfd_get_arch_info (current_bfd);
      while (map[i].u.abfd == current_bfd)
	{
	  string_length = strlen (*map[i].name) + 1;
	  if (arch_info->bits_per_address == 64)
	    {
	      sym_64++;
	      str_64 += string_length;
	    }
	  else
	    {
	      sym_32++;
	      str_32 += string_length;
	    }
	  i++;
	}
    }

  /* A quick sanity check... */
  BFD_ASSERT (sym_64 + sym_32 == orl_count);
  /* Explicit cast to int for compiler.  */
  BFD_ASSERT ((int)(str_64 + str_32) == stridx);

  fhdr = &x_artdata (abfd)->u.bhdr;

  /* xcoff_write_archive_contents_big passes nextoff in symoff. */
  READ20 (fhdr->memoff, prevoff);
  READ20 (fhdr->symoff, nextoff);

  BFD_ASSERT (nextoff == bfd_tell (abfd));

  /* Write out the symbol table.
     Layout :

     standard big archive header
     0x0000		      ar_size	[0x14]
     0x0014		      ar_nxtmem [0x14]
     0x0028		      ar_prvmem [0x14]
     0x003C		      ar_date	[0x0C]
     0x0048		      ar_uid	[0x0C]
     0x0054		      ar_gid	[0x0C]
     0x0060		      ar_mod	[0x0C]
     0x006C		      ar_namelen[0x04]
     0x0070		      ar_fmag	[SXCOFFARFMAG]

     Symbol table
     0x0072		      num_syms	[0x08], binary
     0x0078		      offsets	[0x08 * num_syms], binary
     0x0086 + 0x08 * num_syms names	[??]
     ??			      pad to even bytes.
  */

  if (sym_32)
    {
      struct xcoff_ar_hdr_big *hdr;
      char *symbol_table;
      char *st;

      bfd_vma symbol_table_size =
	SIZEOF_AR_HDR_BIG
	+ SXCOFFARFMAG
	+ 8
	+ 8 * sym_32
	+ str_32 + (str_32 & 1);

      symbol_table = bfd_zmalloc (symbol_table_size);
      if (symbol_table == NULL)
	return false;

      hdr = (struct xcoff_ar_hdr_big *) symbol_table;

      PRINT20 (hdr->size, 8 + 8 * sym_32 + str_32 + (str_32 & 1));

      if (sym_64)
	PRINT20 (hdr->nextoff, nextoff + symbol_table_size);
      else
	PRINT20 (hdr->nextoff, 0);

      PRINT20 (hdr->prevoff, prevoff);
      PRINT12 (hdr->date, 0);
      PRINT12 (hdr->uid, 0);
      PRINT12 (hdr->gid, 0);
      PRINT12 (hdr->mode, 0);
      PRINT4 (hdr->namlen, 0) ;

      st = symbol_table + SIZEOF_AR_HDR_BIG;
      memcpy (st, XCOFFARFMAG, SXCOFFARFMAG);
      st += SXCOFFARFMAG;

      bfd_h_put_64 (abfd, sym_32, st);
      st += 8;

      /* loop over the 32 bit offsets */
      i = 0;
      archive_iterator_begin (&iterator, abfd);
      while (i < orl_count && archive_iterator_next (&iterator))
	{
	  arch_info = bfd_get_arch_info (iterator.current.member);
	  while (map[i].u.abfd == iterator.current.member)
	    {
	      if (arch_info->bits_per_address == 32)
		{
		  bfd_h_put_64 (abfd, iterator.current.offset, st);
		  st += 8;
		}
	      i++;
	    }
	}

      /* loop over the 32 bit symbol names */
      i = 0;
      for (current_bfd = abfd->archive_head;
	   current_bfd != NULL && i < orl_count;
	   current_bfd = current_bfd->archive_next)
	{
	  arch_info = bfd_get_arch_info (current_bfd);
	  while (map[i].u.abfd == current_bfd)
	    {
	      if (arch_info->bits_per_address == 32)
		{
		  string_length = sprintf (st, "%s", *map[i].name);
		  st += string_length + 1;
		}
	      i++;
	    }
	}

      bfd_bwrite (symbol_table, symbol_table_size, abfd);

      free (symbol_table);

      prevoff = nextoff;
      nextoff = nextoff + symbol_table_size;
    }
  else
    PRINT20 (fhdr->symoff, 0);

  if (sym_64)
    {
      struct xcoff_ar_hdr_big *hdr;
      char *symbol_table;
      char *st;

      bfd_vma symbol_table_size =
	SIZEOF_AR_HDR_BIG
	+ SXCOFFARFMAG
	+ 8
	+ 8 * sym_64
	+ str_64 + (str_64 & 1);

      symbol_table = bfd_zmalloc (symbol_table_size);
      if (symbol_table == NULL)
	return false;

      hdr = (struct xcoff_ar_hdr_big *) symbol_table;

      PRINT20 (hdr->size, 8 + 8 * sym_64 + str_64 + (str_64 & 1));
      PRINT20 (hdr->nextoff, 0);
      PRINT20 (hdr->prevoff, prevoff);
      PRINT12 (hdr->date, 0);
      PRINT12 (hdr->uid, 0);
      PRINT12 (hdr->gid, 0);
      PRINT12 (hdr->mode, 0);
      PRINT4 (hdr->namlen, 0);

      st = symbol_table + SIZEOF_AR_HDR_BIG;
      memcpy (st, XCOFFARFMAG, SXCOFFARFMAG);
      st += SXCOFFARFMAG;

      bfd_h_put_64 (abfd, sym_64, st);
      st += 8;

      /* loop over the 64 bit offsets */
      i = 0;
      archive_iterator_begin (&iterator, abfd);
      while (i < orl_count && archive_iterator_next (&iterator))
	{
	  arch_info = bfd_get_arch_info (iterator.current.member);
	  while (map[i].u.abfd == iterator.current.member)
	    {
	      if (arch_info->bits_per_address == 64)
		{
		  bfd_h_put_64 (abfd, iterator.current.offset, st);
		  st += 8;
		}
	      i++;
	    }
	}

      /* loop over the 64 bit symbol names */
      i = 0;
      for (current_bfd = abfd->archive_head;
	   current_bfd != NULL && i < orl_count;
	   current_bfd = current_bfd->archive_next)
	{
	  arch_info = bfd_get_arch_info (current_bfd);
	  while (map[i].u.abfd == current_bfd)
	    {
	      if (arch_info->bits_per_address == 64)
		{
		  string_length = sprintf (st, "%s", *map[i].name);
		  st += string_length + 1;
		}
	      i++;
	    }
	}

      bfd_bwrite (symbol_table, symbol_table_size, abfd);

      free (symbol_table);

      PRINT20 (fhdr->symoff64, nextoff);
    }
  else
    PRINT20 (fhdr->symoff64, 0);

  return true;
}

bool
_bfd_xcoff_write_armap (bfd *abfd, unsigned int elength ATTRIBUTE_UNUSED,
			struct orl *map, unsigned int orl_count, int stridx)
{
  if (! xcoff_big_format_p (abfd))
    return xcoff_write_armap_old (abfd, elength, map, orl_count, stridx);
  else
    return xcoff_write_armap_big (abfd, elength, map, orl_count, stridx);
}

/* Write out an XCOFF archive.  We always write an entire archive,
   rather than fussing with the freelist and so forth.  */

static bool
xcoff_write_archive_contents_old (bfd *abfd)
{
  struct archive_iterator iterator;
  struct xcoff_artdata xtdata;
  struct xcoff_ar_file_hdr *fhdr = &xtdata.u.hdr;
  bfd_size_type count;
  bfd_size_type total_namlen;
  file_ptr *offsets;
  bool makemap;
  bool hasobjects;
  file_ptr prevoff, nextoff;
  bfd *sub;
  size_t i;
  struct xcoff_ar_hdr ahdr;
  bfd_size_type size;
  char *p;
  char decbuf[XCOFFARMAG_ELEMENT_SIZE + 1];

  memset (&xtdata, 0, sizeof (xtdata));
  memcpy (fhdr->magic, XCOFFARMAG, SXCOFFARMAG);
  sprintf (fhdr->firstmemoff, "%zu", SIZEOF_AR_FILE_HDR);
  sprintf (fhdr->freeoff, "%d", 0);

  count = 0;
  total_namlen = 0;
  for (sub = abfd->archive_head; sub != NULL; sub = sub->archive_next)
    {
      ++count;
      total_namlen += strlen (normalize_filename (sub)) + 1;
      if (sub->arelt_data == NULL)
	{
	  sub->arelt_data = bfd_zmalloc (sizeof (struct areltdata));
	  if (sub->arelt_data == NULL)
	    return false;
	}
      if (arch_xhdr (sub) == NULL)
	{
	  struct xcoff_ar_hdr *ahdrp;
	  struct stat s;

	  if ((sub->flags & BFD_IN_MEMORY) != 0)
	    {
	      /* Assume we just "made" the member, and fake it.  */
	      struct bfd_in_memory *bim
		= (struct bfd_in_memory *) sub->iostream;
	      time (&s.st_mtime);
	      s.st_uid = getuid ();
	      s.st_gid = getgid ();
	      s.st_mode = 0644;
	      s.st_size = bim->size;
	    }
	  else if (stat (bfd_get_filename (sub), &s) != 0)
	    {
	      bfd_set_input_error (sub, bfd_error_system_call);
	      return false;
	    }

	  if ((abfd->flags & BFD_DETERMINISTIC_OUTPUT) != 0)
	    {
	      s.st_mtime = 0;
	      s.st_uid = 0;
	      s.st_gid = 0;
	      s.st_mode = 0644;
	    }

	  ahdrp = bfd_zalloc (sub, sizeof (*ahdrp));
	  if (ahdrp == NULL)
	    return false;

	  sprintf (ahdrp->size, "%ld", (long) s.st_size);
	  sprintf (ahdrp->date, "%ld", (long) s.st_mtime);
	  sprintf (ahdrp->uid, "%ld", (long) s.st_uid);
	  sprintf (ahdrp->gid, "%ld", (long) s.st_gid);
	  sprintf (ahdrp->mode, "%o", (unsigned int) s.st_mode);

	  arch_eltdata (sub)->arch_header = (char *) ahdrp;
	  arch_eltdata (sub)->parsed_size = s.st_size;
	}
    }
  offsets = (file_ptr *) bfd_alloc (abfd, count * sizeof (file_ptr));
  if (offsets == NULL)
    return false;

  if (bfd_seek (abfd, (file_ptr) SIZEOF_AR_FILE_HDR, SEEK_SET) != 0)
    return false;

  makemap = bfd_has_map (abfd);
  hasobjects = false;
  prevoff = 0;
  for (archive_iterator_begin (&iterator, abfd), i = 0;
       archive_iterator_next (&iterator);
       i++)
    {
      bfd_size_type namlen;
      struct xcoff_ar_hdr *ahdrp;

      if (makemap && ! hasobjects)
	{
	  if (bfd_check_format (iterator.current.member, bfd_object))
	    hasobjects = true;
	}

      ahdrp = arch_xhdr (iterator.current.member);
      sprintf (ahdrp->prevoff, "%ld", (long) prevoff);
      sprintf (ahdrp->namlen, "%ld", (long) iterator.current.namlen);
      sprintf (ahdrp->nextoff, "%ld", (long) iterator.next.offset);

      /* We need spaces, not null bytes, in the header.  */
      for (p = (char *) ahdrp; p < (char *) ahdrp + SIZEOF_AR_HDR; p++)
	if (*p == '\0')
	  *p = ' ';

      if (!do_pad (abfd, iterator.current.leading_padding))
	return false;

      BFD_ASSERT (iterator.current.offset == bfd_tell (abfd));
      namlen = iterator.current.padded_namlen;
      if (bfd_bwrite (ahdrp, SIZEOF_AR_HDR, abfd) != SIZEOF_AR_HDR
	  || bfd_bwrite (iterator.current.name, namlen, abfd) != namlen
	  || bfd_bwrite (XCOFFARFMAG, SXCOFFARFMAG, abfd) != SXCOFFARFMAG
	  || bfd_seek (iterator.current.member, 0, SEEK_SET) != 0
	  || !do_copy (abfd, iterator.current.member)
	  || !do_pad (abfd, iterator.current.trailing_padding))
	return false;

      offsets[i] = iterator.current.offset;
      prevoff = iterator.current.offset;
    }

  sprintf (fhdr->lastmemoff, "%ld", (long) prevoff);

  /* Write out the member table.  */

  nextoff = iterator.next.offset;
  BFD_ASSERT (nextoff == bfd_tell (abfd));
  sprintf (fhdr->memoff, "%ld", (long) nextoff);

  memset (&ahdr, 0, sizeof ahdr);
  sprintf (ahdr.size, "%ld", (long) (XCOFFARMAG_ELEMENT_SIZE
				     + count * XCOFFARMAG_ELEMENT_SIZE
				     + total_namlen));
  sprintf (ahdr.prevoff, "%ld", (long) prevoff);
  sprintf (ahdr.date, "%d", 0);
  sprintf (ahdr.uid, "%d", 0);
  sprintf (ahdr.gid, "%d", 0);
  sprintf (ahdr.mode, "%d", 0);
  sprintf (ahdr.namlen, "%d", 0);

  size = (SIZEOF_AR_HDR
	  + XCOFFARMAG_ELEMENT_SIZE
	  + count * XCOFFARMAG_ELEMENT_SIZE
	  + total_namlen
	  + SXCOFFARFMAG);

  prevoff = nextoff;
  nextoff += size + (size & 1);

  if (makemap && hasobjects)
    sprintf (ahdr.nextoff, "%ld", (long) nextoff);
  else
    sprintf (ahdr.nextoff, "%d", 0);

  /* We need spaces, not null bytes, in the header.  */
  for (p = (char *) &ahdr; p < (char *) &ahdr + SIZEOF_AR_HDR; p++)
    if (*p == '\0')
      *p = ' ';

  if ((bfd_bwrite (&ahdr, (bfd_size_type) SIZEOF_AR_HDR, abfd)
       != SIZEOF_AR_HDR)
      || (bfd_bwrite (XCOFFARFMAG, (bfd_size_type) SXCOFFARFMAG, abfd)
	  != SXCOFFARFMAG))
    return false;

  sprintf (decbuf, "%-12ld", (long) count);
  if (bfd_bwrite (decbuf, (bfd_size_type) XCOFFARMAG_ELEMENT_SIZE, abfd)
      != XCOFFARMAG_ELEMENT_SIZE)
    return false;
  for (i = 0; i < (size_t) count; i++)
    {
      sprintf (decbuf, "%-12ld", (long) offsets[i]);
      if (bfd_bwrite (decbuf, (bfd_size_type) XCOFFARMAG_ELEMENT_SIZE,
		      abfd) != XCOFFARMAG_ELEMENT_SIZE)
	return false;
    }
  for (sub = abfd->archive_head; sub != NULL; sub = sub->archive_next)
    {
      const char *name;
      bfd_size_type namlen;

      name = normalize_filename (sub);
      namlen = strlen (name);
      if (bfd_bwrite (name, namlen + 1, abfd) != namlen + 1)
	return false;
    }

  if (! do_pad (abfd, size & 1))
    return false;

  /* Write out the armap, if appropriate.  */
  if (! makemap || ! hasobjects)
    sprintf (fhdr->symoff, "%d", 0);
  else
    {
      BFD_ASSERT (nextoff == bfd_tell (abfd));
      sprintf (fhdr->symoff, "%ld", (long) nextoff);
      bfd_ardata (abfd)->tdata = &xtdata;
      bool ret = _bfd_compute_and_write_armap (abfd, 0);
      bfd_ardata (abfd)->tdata = NULL;
      if (!ret)
	return false;
    }

  /* Write out the archive file header.  */

  /* We need spaces, not null bytes, in the header.  */
  for (p = (char *) fhdr; p < (char *) fhdr + SIZEOF_AR_FILE_HDR; p++)
    if (*p == '\0')
      *p = ' ';

  if (bfd_seek (abfd, 0, SEEK_SET) != 0
      || (bfd_bwrite (fhdr, SIZEOF_AR_FILE_HDR, abfd) != SIZEOF_AR_FILE_HDR))
    return false;

  return true;
}

static bool
xcoff_write_archive_contents_big (bfd *abfd)
{
  struct xcoff_artdata xtdata;
  struct xcoff_ar_file_hdr_big *fhdr = &xtdata.u.bhdr;
  bfd_size_type count;
  bfd_size_type total_namlen;
  file_ptr *offsets;
  bool makemap;
  bool hasobjects;
  file_ptr prevoff, nextoff;
  bfd *current_bfd;
  size_t i;
  struct xcoff_ar_hdr_big *hdr;
  bfd_size_type size;
  char *member_table, *mt;
  bfd_vma member_table_size;
  struct archive_iterator iterator;

  memset (&xtdata, 0, sizeof (xtdata));
  memcpy (fhdr->magic, XCOFFARMAGBIG, SXCOFFARMAG);

  if (bfd_seek (abfd, (file_ptr) SIZEOF_AR_FILE_HDR_BIG, SEEK_SET) != 0)
    return false;

  /* Calculate count and total_namlen.  */
  makemap = bfd_has_map (abfd);
  hasobjects = false;
  for (current_bfd = abfd->archive_head, count = 0, total_namlen = 0;
       current_bfd != NULL;
       current_bfd = current_bfd->archive_next, count++)
    {
      total_namlen += strlen (normalize_filename (current_bfd)) + 1;

      if (makemap
	  && ! hasobjects
	  && bfd_check_format (current_bfd, bfd_object))
	hasobjects = true;

      if (current_bfd->arelt_data == NULL)
	{
	  size = sizeof (struct areltdata);
	  current_bfd->arelt_data = bfd_zmalloc (size);
	  if (current_bfd->arelt_data == NULL)
	    return false;
	}

      if (arch_xhdr_big (current_bfd) == NULL)
	{
	  struct xcoff_ar_hdr_big *ahdrp;
	  struct stat s;

	  if ((current_bfd->flags & BFD_IN_MEMORY) != 0)
	    {
	      /* Assume we just "made" the member, and fake it.  */
	      struct bfd_in_memory *bim
		= (struct bfd_in_memory *) current_bfd->iostream;
	      time (&s.st_mtime);
	      s.st_uid = getuid ();
	      s.st_gid = getgid ();
	      s.st_mode = 0644;
	      s.st_size = bim->size;
	    }
	  else if (stat (bfd_get_filename (current_bfd), &s) != 0)
	    {
	      bfd_set_input_error (current_bfd, bfd_error_system_call);
	      return false;
	    }

	  if ((abfd->flags & BFD_DETERMINISTIC_OUTPUT) != 0)
	    {
	      s.st_mtime = 0;
	      s.st_uid = 0;
	      s.st_gid = 0;
	      s.st_mode = 0644;
	    }

	  ahdrp = bfd_zalloc (current_bfd, sizeof (*ahdrp));
	  if (ahdrp == NULL)
	    return false;

	  PRINT20 (ahdrp->size, s.st_size);
	  PRINT12 (ahdrp->date, s.st_mtime);
	  PRINT12 (ahdrp->uid,  s.st_uid);
	  PRINT12 (ahdrp->gid,  s.st_gid);
	  PRINT12_OCTAL (ahdrp->mode, s.st_mode);

	  arch_eltdata (current_bfd)->arch_header = (char *) ahdrp;
	  arch_eltdata (current_bfd)->parsed_size = s.st_size;
	}
    }

  offsets = NULL;
  if (count)
    {
      offsets = (file_ptr *) bfd_malloc (count * sizeof (file_ptr));
      if (offsets == NULL)
	return false;
    }

  prevoff = 0;
  for (archive_iterator_begin (&iterator, abfd), i = 0;
       archive_iterator_next (&iterator);
       i++)
    {
      bfd_size_type namlen;
      struct xcoff_ar_hdr_big *ahdrp;

      ahdrp = arch_xhdr_big (iterator.current.member);
      PRINT20 (ahdrp->prevoff, prevoff);
      PRINT4 (ahdrp->namlen, iterator.current.namlen);
      PRINT20 (ahdrp->nextoff, iterator.next.offset);

      if (!do_pad (abfd, iterator.current.leading_padding))
	{
	  free (offsets);
	  return false;
	}

      BFD_ASSERT (iterator.current.offset == bfd_tell (abfd));
      namlen = iterator.current.padded_namlen;
      if (bfd_bwrite (ahdrp, SIZEOF_AR_HDR_BIG, abfd) != SIZEOF_AR_HDR_BIG
	  || bfd_bwrite (iterator.current.name, namlen, abfd) != namlen
	  || bfd_bwrite (XCOFFARFMAG, SXCOFFARFMAG, abfd) != SXCOFFARFMAG
	  || bfd_seek (iterator.current.member, 0, SEEK_SET) != 0
	  || !do_copy (abfd, iterator.current.member)
	  || !do_pad (abfd, iterator.current.trailing_padding))
	{
	  free (offsets);
	  return false;
	}

      offsets[i] = iterator.current.offset;
      prevoff = iterator.current.offset;
    }

  if (count)
    {
      PRINT20 (fhdr->firstmemoff, offsets[0]);
      PRINT20 (fhdr->lastmemoff, prevoff);
    }

  /* Write out the member table.
     Layout :

     standard big archive header
     0x0000		      ar_size	[0x14]
     0x0014		      ar_nxtmem [0x14]
     0x0028		      ar_prvmem [0x14]
     0x003C		      ar_date	[0x0C]
     0x0048		      ar_uid	[0x0C]
     0x0054		      ar_gid	[0x0C]
     0x0060		      ar_mod	[0x0C]
     0x006C		      ar_namelen[0x04]
     0x0070		      ar_fmag	[0x02]

     Member table
     0x0072		      count	[0x14]
     0x0086		      offsets	[0x14 * counts]
     0x0086 + 0x14 * counts   names	[??]
     ??			      pad to even bytes.
   */

  nextoff = iterator.next.offset;
  BFD_ASSERT (nextoff == bfd_tell (abfd));

  member_table_size = (SIZEOF_AR_HDR_BIG
		       + SXCOFFARFMAG
		       + XCOFFARMAGBIG_ELEMENT_SIZE
		       + count * XCOFFARMAGBIG_ELEMENT_SIZE
		       + total_namlen);

  member_table_size += member_table_size & 1;
  member_table = bfd_zmalloc (member_table_size);
  if (member_table == NULL)
    {
      free (offsets);
      return false;
    }

  hdr = (struct xcoff_ar_hdr_big *) member_table;

  PRINT20 (hdr->size, (XCOFFARMAGBIG_ELEMENT_SIZE
		       + count * XCOFFARMAGBIG_ELEMENT_SIZE
		       + total_namlen + (total_namlen & 1)));
  if (makemap && hasobjects)
    PRINT20 (hdr->nextoff, nextoff + member_table_size);
  else
    PRINT20 (hdr->nextoff, 0);
  PRINT20 (hdr->prevoff, prevoff);
  PRINT12 (hdr->date, 0);
  PRINT12 (hdr->uid, 0);
  PRINT12 (hdr->gid, 0);
  PRINT12 (hdr->mode, 0);
  PRINT4 (hdr->namlen, 0);

  mt = member_table + SIZEOF_AR_HDR_BIG;
  memcpy (mt, XCOFFARFMAG, SXCOFFARFMAG);
  mt += SXCOFFARFMAG;

  PRINT20 (mt, count);
  mt += XCOFFARMAGBIG_ELEMENT_SIZE;
  for (i = 0; i < (size_t) count; i++)
    {
      PRINT20 (mt, offsets[i]);
      mt += XCOFFARMAGBIG_ELEMENT_SIZE;
    }

  if (count)
    {
      free (offsets);
      offsets = NULL;
    }

  for (current_bfd = abfd->archive_head;
       current_bfd != NULL;
       current_bfd = current_bfd->archive_next)
    {
      const char *name;
      size_t namlen;

      name = normalize_filename (current_bfd);
      namlen = sprintf (mt, "%s", name);
      mt += namlen + 1;
    }

  if (bfd_bwrite (member_table, member_table_size, abfd) != member_table_size)
    return false;

  free (member_table);

  PRINT20 (fhdr->memoff, nextoff);

  prevoff = nextoff;
  nextoff += member_table_size;

  /* Write out the armap, if appropriate.  */

  if (! makemap || ! hasobjects)
    PRINT20 (fhdr->symoff, 0);
  else
    {
      BFD_ASSERT (nextoff == bfd_tell (abfd));

      /* Save nextoff in fhdr->symoff so the armap routine can use it.  */
      PRINT20 (fhdr->symoff, nextoff);

      bfd_ardata (abfd)->tdata = &xtdata;
      bool ret = _bfd_compute_and_write_armap (abfd, 0);
      bfd_ardata (abfd)->tdata = NULL;
      if (!ret)
	return false;
    }

  /* Write out the archive file header.  */

  if (bfd_seek (abfd, 0, SEEK_SET) != 0
      || bfd_bwrite (fhdr,
		     SIZEOF_AR_FILE_HDR_BIG, abfd) != SIZEOF_AR_FILE_HDR_BIG)
    return false;

  return true;
}

bool
_bfd_xcoff_write_archive_contents (bfd *abfd)
{
  if (! xcoff_big_format_p (abfd))
    return xcoff_write_archive_contents_old (abfd);
  else
    return xcoff_write_archive_contents_big (abfd);
}

/* We can't use the usual coff_sizeof_headers routine, because AIX
   always uses an a.out header.  */

int
_bfd_xcoff_sizeof_headers (bfd *abfd,
			   struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  int size;

  size = FILHSZ;
  if (xcoff_data (abfd)->full_aouthdr)
    size += AOUTSZ;
  else
    size += SMALL_AOUTSZ;
  size += abfd->section_count * SCNHSZ;

  if (info->strip != strip_all)
    {
      /* There can be additional sections just for dealing with overflow in
	 reloc and lineno counts. But the numbers of relocs and lineno aren't
	 known when bfd_sizeof_headers is called, so we compute them by
	 summing the numbers from input sections.  */
      struct nbr_reloc_lineno
      {
	unsigned int reloc_count;
	unsigned int lineno_count;
      };
      struct nbr_reloc_lineno *n_rl;
      bfd *sub;
      unsigned int max_index;
      asection *s;

      /* Although the number of sections is known, the maximum value of
	 section->index isn't (because some sections may have been removed).
	 Don't try to renumber sections, just compute the upper bound.  */
      max_index = 0;
      for (s = abfd->sections; s != NULL; s = s->next)
	if (s->index > max_index)
	  max_index = s->index;

      /* Allocate the per section counters. It could be possible to use a
	 preallocated array as the number of sections is limited on XCOFF,
	 but this creates a maintainance issue.  */
      n_rl = bfd_zmalloc ((max_index + 1) * sizeof (*n_rl));
      if (n_rl == NULL)
	return -1;

      /* Sum.  */
      for (sub = info->input_bfds; sub != NULL; sub = sub->link.next)
	for (s = sub->sections; s != NULL; s = s->next)
	  if (s->output_section->owner == abfd
	      && !bfd_section_removed_from_list (abfd, s->output_section))
	    {
	      struct nbr_reloc_lineno *e = &n_rl[s->output_section->index];
	      e->reloc_count += s->reloc_count;
	      e->lineno_count += s->lineno_count;
	    }

      /* Add the size of a section for each section with an overflow.  */
      for (s = abfd->sections; s != NULL; s = s->next)
	{
	  struct nbr_reloc_lineno *e = &n_rl[s->index];

	  if (e->reloc_count >= 0xffff
	      || (e->lineno_count >= 0xffff && info->strip != strip_debugger))
	    size += SCNHSZ;
	}

      free (n_rl);
    }

  return size;
}

/* Routines to swap information in the XCOFF .loader section.  If we
   ever need to write an XCOFF loader, this stuff will need to be
   moved to another file shared by the linker (which XCOFF calls the
   ``binder'') and the loader.  */

/* Swap in the ldhdr structure.  */

static void
xcoff_swap_ldhdr_in (bfd *abfd, const void * s, struct internal_ldhdr *dst)
{
  const struct external_ldhdr *src = (const struct external_ldhdr *) s;

  dst->l_version = bfd_get_32 (abfd, src->l_version);
  dst->l_nsyms = bfd_get_32 (abfd, src->l_nsyms);
  dst->l_nreloc = bfd_get_32 (abfd, src->l_nreloc);
  dst->l_istlen = bfd_get_32 (abfd, src->l_istlen);
  dst->l_nimpid = bfd_get_32 (abfd, src->l_nimpid);
  dst->l_impoff = bfd_get_32 (abfd, src->l_impoff);
  dst->l_stlen = bfd_get_32 (abfd, src->l_stlen);
  dst->l_stoff = bfd_get_32 (abfd, src->l_stoff);
}

/* Swap out the ldhdr structure.  */

static void
xcoff_swap_ldhdr_out (bfd *abfd, const struct internal_ldhdr *src, void * d)
{
  struct external_ldhdr *dst = (struct external_ldhdr *) d;

  bfd_put_32 (abfd, (bfd_vma) src->l_version, dst->l_version);
  bfd_put_32 (abfd, src->l_nsyms, dst->l_nsyms);
  bfd_put_32 (abfd, src->l_nreloc, dst->l_nreloc);
  bfd_put_32 (abfd, src->l_istlen, dst->l_istlen);
  bfd_put_32 (abfd, src->l_nimpid, dst->l_nimpid);
  bfd_put_32 (abfd, src->l_impoff, dst->l_impoff);
  bfd_put_32 (abfd, src->l_stlen, dst->l_stlen);
  bfd_put_32 (abfd, src->l_stoff, dst->l_stoff);
}

/* Swap in the ldsym structure.  */

static void
xcoff_swap_ldsym_in (bfd *abfd, const void * s, struct internal_ldsym *dst)
{
  const struct external_ldsym *src = (const struct external_ldsym *) s;

  if (bfd_get_32 (abfd, src->_l._l_l._l_zeroes) != 0) {
    memcpy (dst->_l._l_name, src->_l._l_name, SYMNMLEN);
  } else {
    dst->_l._l_l._l_zeroes = 0;
    dst->_l._l_l._l_offset = bfd_get_32 (abfd, src->_l._l_l._l_offset);
  }
  dst->l_value = bfd_get_32 (abfd, src->l_value);
  dst->l_scnum = bfd_get_16 (abfd, src->l_scnum);
  dst->l_smtype = bfd_get_8 (abfd, src->l_smtype);
  dst->l_smclas = bfd_get_8 (abfd, src->l_smclas);
  dst->l_ifile = bfd_get_32 (abfd, src->l_ifile);
  dst->l_parm = bfd_get_32 (abfd, src->l_parm);
}

/* Swap out the ldsym structure.  */

static void
xcoff_swap_ldsym_out (bfd *abfd, const struct internal_ldsym *src, void * d)
{
  struct external_ldsym *dst = (struct external_ldsym *) d;

  if (src->_l._l_l._l_zeroes != 0)
    memcpy (dst->_l._l_name, src->_l._l_name, SYMNMLEN);
  else
    {
      bfd_put_32 (abfd, (bfd_vma) 0, dst->_l._l_l._l_zeroes);
      bfd_put_32 (abfd, (bfd_vma) src->_l._l_l._l_offset,
		  dst->_l._l_l._l_offset);
    }
  bfd_put_32 (abfd, src->l_value, dst->l_value);
  bfd_put_16 (abfd, (bfd_vma) src->l_scnum, dst->l_scnum);
  bfd_put_8 (abfd, src->l_smtype, dst->l_smtype);
  bfd_put_8 (abfd, src->l_smclas, dst->l_smclas);
  bfd_put_32 (abfd, src->l_ifile, dst->l_ifile);
  bfd_put_32 (abfd, src->l_parm, dst->l_parm);
}

static void
xcoff_swap_reloc_in (bfd *abfd, void * s, void * d)
{
  struct external_reloc *src = (struct external_reloc *) s;
  struct internal_reloc *dst = (struct internal_reloc *) d;

  memset (dst, 0, sizeof (struct internal_reloc));

  dst->r_vaddr = bfd_get_32 (abfd, src->r_vaddr);
  dst->r_symndx = bfd_get_32 (abfd, src->r_symndx);
  dst->r_size = bfd_get_8 (abfd, src->r_size);
  dst->r_type = bfd_get_8 (abfd, src->r_type);
}

static unsigned int
xcoff_swap_reloc_out (bfd *abfd, void * s, void * d)
{
  struct internal_reloc *src = (struct internal_reloc *) s;
  struct external_reloc *dst = (struct external_reloc *) d;

  bfd_put_32 (abfd, src->r_vaddr, dst->r_vaddr);
  bfd_put_32 (abfd, src->r_symndx, dst->r_symndx);
  bfd_put_8 (abfd, src->r_type, dst->r_type);
  bfd_put_8 (abfd, src->r_size, dst->r_size);

  return bfd_coff_relsz (abfd);
}

/* Swap in the ldrel structure.  */

static void
xcoff_swap_ldrel_in (bfd *abfd, const void * s, struct internal_ldrel *dst)
{
  const struct external_ldrel *src = (const struct external_ldrel *) s;

  dst->l_vaddr = bfd_get_32 (abfd, src->l_vaddr);
  dst->l_symndx = bfd_get_32 (abfd, src->l_symndx);
  dst->l_rtype = bfd_get_16 (abfd, src->l_rtype);
  dst->l_rsecnm = bfd_get_16 (abfd, src->l_rsecnm);
}

/* Swap out the ldrel structure.  */

static void
xcoff_swap_ldrel_out (bfd *abfd, const struct internal_ldrel *src, void * d)
{
  struct external_ldrel *dst = (struct external_ldrel *) d;

  bfd_put_32 (abfd, src->l_vaddr, dst->l_vaddr);
  bfd_put_32 (abfd, src->l_symndx, dst->l_symndx);
  bfd_put_16 (abfd, (bfd_vma) src->l_rtype, dst->l_rtype);
  bfd_put_16 (abfd, (bfd_vma) src->l_rsecnm, dst->l_rsecnm);
}


bool
xcoff_reloc_type_noop (bfd *input_bfd ATTRIBUTE_UNUSED,
		       asection *input_section ATTRIBUTE_UNUSED,
		       bfd *output_bfd ATTRIBUTE_UNUSED,
		       struct internal_reloc *rel ATTRIBUTE_UNUSED,
		       struct internal_syment *sym ATTRIBUTE_UNUSED,
		       struct reloc_howto_struct *howto ATTRIBUTE_UNUSED,
		       bfd_vma val ATTRIBUTE_UNUSED,
		       bfd_vma addend ATTRIBUTE_UNUSED,
		       bfd_vma *relocation ATTRIBUTE_UNUSED,
		       bfd_byte *contents ATTRIBUTE_UNUSED,
		       struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  return true;
}

bool
xcoff_reloc_type_fail (bfd *input_bfd,
		       asection *input_section ATTRIBUTE_UNUSED,
		       bfd *output_bfd ATTRIBUTE_UNUSED,
		       struct internal_reloc *rel,
		       struct internal_syment *sym ATTRIBUTE_UNUSED,
		       struct reloc_howto_struct *howto ATTRIBUTE_UNUSED,
		       bfd_vma val ATTRIBUTE_UNUSED,
		       bfd_vma addend ATTRIBUTE_UNUSED,
		       bfd_vma *relocation ATTRIBUTE_UNUSED,
		       bfd_byte *contents ATTRIBUTE_UNUSED,
		       struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  _bfd_error_handler
    /* xgettext: c-format */
    (_("%pB: unsupported relocation type %#x"),
     input_bfd, (unsigned int) rel->r_type);
  bfd_set_error (bfd_error_bad_value);
  return false;
}

bool
xcoff_reloc_type_pos (bfd *input_bfd ATTRIBUTE_UNUSED,
		      asection *input_section ATTRIBUTE_UNUSED,
		      bfd *output_bfd ATTRIBUTE_UNUSED,
		      struct internal_reloc *rel ATTRIBUTE_UNUSED,
		      struct internal_syment *sym ATTRIBUTE_UNUSED,
		      struct reloc_howto_struct *howto ATTRIBUTE_UNUSED,
		      bfd_vma val,
		      bfd_vma addend,
		      bfd_vma *relocation,
		      bfd_byte *contents ATTRIBUTE_UNUSED,
		      struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  *relocation = val + addend;
  return true;
}

bool
xcoff_reloc_type_neg (bfd *input_bfd ATTRIBUTE_UNUSED,
		      asection *input_section ATTRIBUTE_UNUSED,
		      bfd *output_bfd ATTRIBUTE_UNUSED,
		      struct internal_reloc *rel ATTRIBUTE_UNUSED,
		      struct internal_syment *sym ATTRIBUTE_UNUSED,
		      struct reloc_howto_struct *howto ATTRIBUTE_UNUSED,
		      bfd_vma val,
		      bfd_vma addend,
		      bfd_vma *relocation,
		      bfd_byte *contents ATTRIBUTE_UNUSED,
		      struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  *relocation = - val - addend;
  return true;
}

bool
xcoff_reloc_type_rel (bfd *input_bfd ATTRIBUTE_UNUSED,
		      asection *input_section,
		      bfd *output_bfd ATTRIBUTE_UNUSED,
		      struct internal_reloc *rel ATTRIBUTE_UNUSED,
		      struct internal_syment *sym ATTRIBUTE_UNUSED,
		      struct reloc_howto_struct *howto,
		      bfd_vma val,
		      bfd_vma addend,
		      bfd_vma *relocation,
		      bfd_byte *contents ATTRIBUTE_UNUSED,
		      struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  howto->pc_relative = true;

  /* A PC relative reloc includes the section address.  */
  addend += input_section->vma;

  *relocation = val + addend;
  *relocation -= (input_section->output_section->vma
		  + input_section->output_offset);
  return true;
}

bool
xcoff_reloc_type_toc (bfd *input_bfd,
		      asection *input_section ATTRIBUTE_UNUSED,
		      bfd *output_bfd,
		      struct internal_reloc *rel,
		      struct internal_syment *sym ATTRIBUTE_UNUSED,
		      struct reloc_howto_struct *howto ATTRIBUTE_UNUSED,
		      bfd_vma val,
		      bfd_vma addend ATTRIBUTE_UNUSED,
		      bfd_vma *relocation,
		      bfd_byte *contents ATTRIBUTE_UNUSED,
		      struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  struct xcoff_link_hash_entry *h;

  if (0 > rel->r_symndx)
    return false;

  h = obj_xcoff_sym_hashes (input_bfd)[rel->r_symndx];

  if (h != NULL && h->smclas != XMC_TD)
    {
      if (h->toc_section == NULL)
	{
	  _bfd_error_handler
	    /* xgettext: c-format */
	    (_("%pB: TOC reloc at %#" PRIx64 " to symbol `%s' with no TOC entry"),
	     input_bfd, (uint64_t) rel->r_vaddr, h->root.root.string);
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}

      BFD_ASSERT ((h->flags & XCOFF_SET_TOC) == 0);
      val = (h->toc_section->output_section->vma
	      + h->toc_section->output_offset);
    }

  /* We can't use the preexisting value written down by the
     assembly, as R_TOCU needs to be adjusted when the final
     R_TOCL value is signed.  */
  *relocation = val - xcoff_data (output_bfd)->toc;

  if (rel->r_type == R_TOCU)
    *relocation = ((*relocation + 0x8000) >> 16) & 0xffff;
  if (rel->r_type == R_TOCL)
    *relocation = *relocation & 0x0000ffff;

  return true;
}

bool
xcoff_reloc_type_ba (bfd *input_bfd ATTRIBUTE_UNUSED,
		     asection *input_section ATTRIBUTE_UNUSED,
		     bfd *output_bfd ATTRIBUTE_UNUSED,
		     struct internal_reloc *rel ATTRIBUTE_UNUSED,
		     struct internal_syment *sym ATTRIBUTE_UNUSED,
		     struct reloc_howto_struct *howto,
		     bfd_vma val,
		     bfd_vma addend,
		     bfd_vma *relocation,
		     bfd_byte *contents ATTRIBUTE_UNUSED,
		     struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  howto->src_mask &= ~3;
  howto->dst_mask = howto->src_mask;

  *relocation = val + addend;

  return true;
}

static bool
xcoff_reloc_type_br (bfd *input_bfd,
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
     instruction, we replace the cror with lwz r2,20(r1).  This
     restores the TOC after the glink code.  Contrariwise, if the
     call is followed by a lwz r2,20(r1), but the call is not
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

      /* The _ptrgl function is magic.  It is used by the AIX
	 compiler to call a function through a pointer.  */
      if (h->smclas == XMC_GL || strcmp (h->root.root.string, "._ptrgl") == 0)
	{
	  if (next == 0x4def7b82			/* cror 15,15,15 */
	      || next == 0x4ffffb82			/* cror 31,31,31 */
	      || next == 0x60000000)			/* ori r0,r0,0 */
	    bfd_put_32 (input_bfd, 0x80410014, pnext);	/* lwz r2,20(r1) */

	}
      else
	{
	  if (next == 0x80410014)			/* lwz r2,20(r1) */
	    bfd_put_32 (input_bfd, 0x60000000, pnext);	/* ori r0,r0,0 */
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

bool
xcoff_reloc_type_crel (bfd *input_bfd ATTRIBUTE_UNUSED,
		       asection *input_section,
		       bfd *output_bfd ATTRIBUTE_UNUSED,
		       struct internal_reloc *rel ATTRIBUTE_UNUSED,
		       struct internal_syment *sym ATTRIBUTE_UNUSED,
		       struct reloc_howto_struct *howto,
		       bfd_vma val ATTRIBUTE_UNUSED,
		       bfd_vma addend,
		       bfd_vma *relocation,
		       bfd_byte *contents ATTRIBUTE_UNUSED,
		       struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  howto->pc_relative = true;
  howto->src_mask &= ~3;
  howto->dst_mask = howto->src_mask;

  /* A PC relative reloc includes the section address.  */
  addend += input_section->vma;

  *relocation = val + addend;
  *relocation -= (input_section->output_section->vma
		  + input_section->output_offset);
  return true;
}

bool
xcoff_reloc_type_tls (bfd *input_bfd ATTRIBUTE_UNUSED,
		      asection *input_section ATTRIBUTE_UNUSED,
		      bfd *output_bfd ATTRIBUTE_UNUSED,
		      struct internal_reloc *rel ATTRIBUTE_UNUSED,
		      struct internal_syment *sym ATTRIBUTE_UNUSED,
		      struct reloc_howto_struct *howto,
		      bfd_vma val,
		      bfd_vma addend,
		      bfd_vma *relocation,
		      bfd_byte *contents ATTRIBUTE_UNUSED,
		      struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  struct xcoff_link_hash_entry *h;

  if (0 > rel->r_symndx)
    return false;

  h = obj_xcoff_sym_hashes (input_bfd)[rel->r_symndx];

  /* R_TLSML is handled by the loader but must be from a
     TOC entry targeting itslef.  This is already verified in
     xcoff_link_add_symbols.
     The value must be 0.  */
  if (howto->type == R_TLSML)
    {
      *relocation = 0;
      return true;
    }

  /* The target symbol should always be available even if it's not
     exported.  */
  BFD_ASSERT (h != NULL);

  /* TLS relocations must target a TLS symbol.  */
  if (h->smclas != XMC_TL && h->smclas != XMC_UL)
    {
      _bfd_error_handler
	(_("%pB: TLS relocation at 0x%" PRIx64 " over non-TLS symbol %s (0x%x)\n"),
	 input_bfd, (uint64_t) rel->r_vaddr, h->root.root.string, h->smclas);
      return false;
    }

  /* Local TLS relocations must target a local symbol, ie
     non-imported. */
  if ((rel->r_type == R_TLS_LD || rel->r_type == R_TLS_LE)
      && (((h->flags & XCOFF_DEF_REGULAR) == 0
       && (h->flags & XCOFF_DEF_DYNAMIC) != 0)
	  || (h->flags & XCOFF_IMPORT) != 0))
    {
      _bfd_error_handler
	(_("%pB: TLS local relocation at 0x%" PRIx64 " over imported symbol %s\n"),
	 input_bfd, (uint64_t) rel->r_vaddr, h->root.root.string);
      return false;
    }

  /* R_TLSM are relocations used by the loader.
     The value must be 0.  */
  if (howto->type == R_TLSM)
    {
      *relocation = 0;
      return true;
    }

  /* Other TLS relocations aims to put offsets from TLS pointers
     starting at -0x7c00 (or -0x7800 in XCOFF64).  It becomes a
     simple R_POS relocation as long as .tdata and .tbss addresses
     start at the same value. This is done in aix ld scripts.
     TODO: implement optimization when tls size is < 62K.  */
  *relocation = val + addend;

  return true;
}

static bool
xcoff_complain_overflow_dont_func (bfd *input_bfd ATTRIBUTE_UNUSED,
				   bfd_vma val ATTRIBUTE_UNUSED,
				   bfd_vma relocation ATTRIBUTE_UNUSED,
				   struct reloc_howto_struct *
				      howto ATTRIBUTE_UNUSED)
{
  return false;
}

static bool
xcoff_complain_overflow_bitfield_func (bfd *input_bfd,
				       bfd_vma val,
				       bfd_vma relocation,
				       struct reloc_howto_struct *howto)
{
  bfd_vma fieldmask, signmask, ss;
  bfd_vma a, b, sum;

  /* Get the values to be added together.  For signed and unsigned
     relocations, we assume that all values should be truncated to
     the size of an address.  For bitfields, all the bits matter.
     See also bfd_check_overflow.  */
  fieldmask = N_ONES (howto->bitsize);
  a = relocation;
  b = val & howto->src_mask;

  /* Much like unsigned, except no trimming with addrmask.  In
     addition, the sum overflows if there is a carry out of
     the bfd_vma, i.e., the sum is less than either input
     operand.  */
  a >>= howto->rightshift;
  b >>= howto->bitpos;

  /* Bitfields are sometimes used for signed numbers; for
     example, a 13-bit field sometimes represents values in
     0..8191 and sometimes represents values in -4096..4095.
     If the field is signed and a is -4095 (0x1001) and b is
     -1 (0x1fff), the sum is -4096 (0x1000), but (0x1001 +
     0x1fff is 0x3000).  It's not clear how to handle this
     everywhere, since there is not way to know how many bits
     are significant in the relocation, but the original code
     assumed that it was fully sign extended, and we will keep
     that assumption.  */
  signmask = (fieldmask >> 1) + 1;

  if ((a & ~ fieldmask) != 0)
    {
      /* Some bits out of the field are set.  This might not
	 be a problem: if this is a signed bitfield, it is OK
	 iff all the high bits are set, including the sign
	 bit.  We'll try setting all but the most significant
	 bit in the original relocation value: if this is all
	 ones, we are OK, assuming a signed bitfield.  */
      ss = (signmask << howto->rightshift) - 1;
      if ((ss | relocation) != ~ (bfd_vma) 0)
	return true;
      a &= fieldmask;
    }

  /* We just assume (b & ~ fieldmask) == 0.  */

  /* We explicitly permit wrap around if this relocation
     covers the high bit of an address.  The Linux kernel
     relies on it, and it is the only way to write assembler
     code which can run when loaded at a location 0x80000000
     away from the location at which it is linked.  */
  if ((unsigned) howto->bitsize + howto->rightshift
      == bfd_arch_bits_per_address (input_bfd))
    return false;

  sum = a + b;
  if (sum < a || (sum & ~ fieldmask) != 0)
    {
      /* There was a carry out, or the field overflow.  Test
	 for signed operands again.  Here is the overflow test
	 is as for complain_overflow_signed.  */
      if (((~ (a ^ b)) & (a ^ sum)) & signmask)
	return true;
    }

  return false;
}

static bool
xcoff_complain_overflow_signed_func (bfd *input_bfd,
				     bfd_vma val,
				     bfd_vma relocation,
				     struct reloc_howto_struct *howto)
{
  bfd_vma addrmask, fieldmask, signmask, ss;
  bfd_vma a, b, sum;

  /* Get the values to be added together.  For signed and unsigned
     relocations, we assume that all values should be truncated to
     the size of an address.  For bitfields, all the bits matter.
     See also bfd_check_overflow.  */
  fieldmask = N_ONES (howto->bitsize);
  addrmask = N_ONES (bfd_arch_bits_per_address (input_bfd)) | fieldmask;
  a = relocation;
  b = val & howto->src_mask;

  a = (a & addrmask) >> howto->rightshift;

  /* If any sign bits are set, all sign bits must be set.
     That is, A must be a valid negative address after
     shifting.  */
  signmask = ~ (fieldmask >> 1);
  ss = a & signmask;
  if (ss != 0 && ss != ((addrmask >> howto->rightshift) & signmask))
    return true;

  /* We only need this next bit of code if the sign bit of B
     is below the sign bit of A.  This would only happen if
     SRC_MASK had fewer bits than BITSIZE.  Note that if
     SRC_MASK has more bits than BITSIZE, we can get into
     trouble; we would need to verify that B is in range, as
     we do for A above.  */
  signmask = ((~ howto->src_mask) >> 1) & howto->src_mask;
  if ((b & signmask) != 0)
    {
      /* Set all the bits above the sign bit.  */
      b -= signmask <<= 1;
    }

  b = (b & addrmask) >> howto->bitpos;

  /* Now we can do the addition.  */
  sum = a + b;

  /* See if the result has the correct sign.  Bits above the
     sign bit are junk now; ignore them.  If the sum is
     positive, make sure we did not have all negative inputs;
     if the sum is negative, make sure we did not have all
     positive inputs.  The test below looks only at the sign
     bits, and it really just
     SIGN (A) == SIGN (B) && SIGN (A) != SIGN (SUM)
  */
  signmask = (fieldmask >> 1) + 1;
  if (((~ (a ^ b)) & (a ^ sum)) & signmask)
    return true;

  return false;
}

static bool
xcoff_complain_overflow_unsigned_func (bfd *input_bfd,
				       bfd_vma val,
				       bfd_vma relocation,
				       struct reloc_howto_struct *howto)
{
  bfd_vma addrmask, fieldmask;
  bfd_vma a, b, sum;

  /* Get the values to be added together.  For signed and unsigned
     relocations, we assume that all values should be truncated to
     the size of an address.  For bitfields, all the bits matter.
     See also bfd_check_overflow.  */
  fieldmask = N_ONES (howto->bitsize);
  addrmask = N_ONES (bfd_arch_bits_per_address (input_bfd)) | fieldmask;
  a = relocation;
  b = val & howto->src_mask;

  /* Checking for an unsigned overflow is relatively easy:
     trim the addresses and add, and trim the result as well.
     Overflow is normally indicated when the result does not
     fit in the field.  However, we also need to consider the
     case when, e.g., fieldmask is 0x7fffffff or smaller, an
     input is 0x80000000, and bfd_vma is only 32 bits; then we
     will get sum == 0, but there is an overflow, since the
     inputs did not fit in the field.  Instead of doing a
     separate test, we can check for this by or-ing in the
     operands when testing for the sum overflowing its final
     field.  */
  a = (a & addrmask) >> howto->rightshift;
  b = (b & addrmask) >> howto->bitpos;
  sum = (a + b) & addrmask;
  if ((a | b | sum) & ~ fieldmask)
    return true;

  return false;
}

/* This is the relocation function for the RS/6000/POWER/PowerPC.
   This is currently the only processor which uses XCOFF; I hope that
   will never change.

   The original version was based on two documents:
   the PowerPC AIX Version 4 Application Binary Interface, First
   Edition (April 1992), and the PowerOpen ABI, Big-Endian
   32-Bit Hardware Implementation (June 30, 1994).  Differences
   between the documents are noted below.
   Now, IBM has released an official documentation about XCOFF
   format:
   https://www.ibm.com/support/knowledgecenter/ssw_aix_72/filesreference/XCOFF.html

   Unsupported r_type's

   R_RTB:
   R_RRTBI:
   R_RRTBA:

   These relocs are defined by the PowerPC ABI to be
   relative branches which use half of the difference
   between the symbol and the program counter.  I can't
   quite figure out when this is useful.  These relocs are
   not defined by the PowerOpen ABI.

   Supported r_type's

   R_POS:
   Simple positive relocation.

   R_NEG:
   Simple negative relocation.

   R_REL:
   Simple PC relative relocation.

   R_TOC:
   TOC relative relocation.  The value in the instruction in
   the input file is the offset from the input file TOC to
   the desired location.  We want the offset from the final
   TOC to the desired location.  We have:
   isym = iTOC + in
   iinsn = in + o
   osym = oTOC + on
   oinsn = on + o
   so we must change insn by on - in.
   This relocation allows the linker to perform optimizations
   by transforming a load instruction into a add-immediate
   when possible. The relocation is, then, changed to R_TRLA
   in the output file.
   TODO: Currently, the optimisation isn't implemented.

   R_TRL:
   TOC relative relocation.  Same as R_TOC, except that
   the optimization isn't allowed

   R_TRLA:
   TOC relative relocation.  This is a TOC relative load
   address instruction which have been changed to an add-
   immediate instruction.

   R_GL:
   GL linkage relocation.  The value of this relocation
   is the address of the external symbol in the TOC
   section.

   R_TCL:
   Local object TOC address.  I can't figure out the
   difference between this and case R_GL.

   R_RL:
   The PowerPC AIX ABI describes this as a load which may be
   changed to a load address.  The PowerOpen ABI says this
   is the same as case R_POS.

   R_RLA:
   The PowerPC AIX ABI describes this as a load address
   which may be changed to a load.  The PowerOpen ABI says
   this is the same as R_POS.

   R_REF:
   Not a relocation but a way to prevent the garbage
   collector of AIX linker to remove symbols.
   This is not needed in our case.

   R_BA:
   The PowerOpen ABI says this is the same as R_RBA.

   R_RBA:
   Absolute branch which may be modified to become a
   relative branch.

   R_BR:
   The PowerOpen ABI says this is the same as R_RBR.

   R_RBR:
   A relative branch which may be modified to become an
   absolute branch.

   R_CAI:
   The PowerPC ABI defines this as an absolute call which
   may be modified to become a relative call.  The PowerOpen
   ABI does not define this relocation type.

   R_CREL:
   The PowerPC ABI defines this as a relative call which may
   be modified to become an absolute call.  The PowerOpen
   ABI does not define this relocation type.

   R_RBAC:
   The PowerPC ABI defines this as an absolute branch to a
   fixed address which may be modified to an absolute branch
   to a symbol.  The PowerOpen ABI does not define this
   relocation type.

   R_RBRC:
   The PowerPC ABI defines this as an absolute branch to a
   fixed address which may be modified to a relative branch.
   The PowerOpen ABI does not define this relocation type.

   R_TLS:
   Thread-local storage relocation using general-dynamic
   model.

   R_TLS_IE:
   Thread-local storage relocation using initial-exec model.

   R_TLS_LD:
   Thread-local storage relocation using local-dynamic model.

   R_TLS_LE:
   Thread-local storage relocation using local-exec model.

   R_TLSM:
   Tread-local storage relocation used by the loader.

   R_TLSML:
   Tread-local storage relocation used by the loader.

   R_TOCU:
   Upper TOC relative relocation. The value is the
   high-order 16 bit of a TOC relative relocation.

   R_TOCL:
   Lower TOC relative relocation. The value is the
   low-order 16 bit of a TOC relative relocation.
*/

bool
xcoff_ppc_relocate_section (bfd *output_bfd,
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
      memcpy (&howto, &xcoff_howto_table[rel->r_type], sizeof (howto));
      if (howto.bitsize != (rel->r_size & 0x1f) + 1)
	{
	  switch (rel->r_type)
	    {
	    case R_POS:
	    case R_NEG:
	      howto.bitsize = (rel->r_size & 0x1f) + 1;
	      howto.size = HOWTO_RSIZE (howto.bitsize > 16 ? 4 : 2);
	      howto.src_mask = howto.dst_mask = N_ONES (howto.bitsize);
	      break;

	    default:
	      _bfd_error_handler
		(_("%pB: relocation (%d) at 0x%" PRIx64 " has wrong r_rsize (0x%x)\n"),
		 input_bfd, rel->r_type, (uint64_t) rel->r_vaddr, rel->r_size);
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
		(*info->callbacks->undefined_symbol)
		  (info, h->root.root.string,
		   input_bfd, input_section,
		   rel->r_vaddr - input_section->vma,
		   info->unresolved_syms_in_objects == RM_DIAGNOSE &&
		       !info->warn_unresolved_syms);

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
			      || (info->static_link
				  && (h->flags & XCOFF_WAS_UNDEFINED) != 0)
			      || (h->flags & XCOFF_DEF_DYNAMIC) != 0
			      || (h->flags & XCOFF_IMPORT) != 0);
		}
	    }
	}

      if (rel->r_type >= XCOFF_MAX_CALCULATE_RELOCATION
	  || !((*xcoff_calculate_relocation[rel->r_type])
	       (input_bfd, input_section, output_bfd, rel, sym, &howto, val,
		addend, &relocation, contents, info)))
	return false;

      /* address */
      address = rel->r_vaddr - input_section->vma;
      location = contents + address;

      if (address > input_section->size)
	abort ();

      /* Get the value we are going to relocate.  */
      if (2 == bfd_get_reloc_size (&howto))
	value_to_relocate = bfd_get_16 (input_bfd, location);
      else
	value_to_relocate = bfd_get_32 (input_bfd, location);

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
      if (2 == bfd_get_reloc_size (&howto))
	bfd_put_16 (input_bfd, value_to_relocate, location);
      else
	bfd_put_32 (input_bfd, value_to_relocate, location);
    }

  return true;
}

/* gcc-8 warns (*) on all the strncpy calls in this function about
   possible string truncation.  The "truncation" is not a bug.  We
   have an external representation of structs with fields that are not
   necessarily NULL terminated and corresponding internal
   representation fields that are one larger so that they can always
   be NULL terminated.
   gcc versions between 4.2 and 4.6 do not allow pragma control of
   diagnostics inside functions, giving a hard error if you try to use
   the finer control available with later versions.
   gcc prior to 4.2 warns about diagnostic push and pop.
   gcc-5, gcc-6 and gcc-7 warn that -Wstringop-truncation is unknown,
   unless you also add #pragma GCC diagnostic ignored "-Wpragma".
   (*) Depending on your system header files!  */
#if GCC_VERSION >= 8000
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wstringop-truncation"
#endif
static bool
_bfd_xcoff_put_ldsymbol_name (bfd *abfd ATTRIBUTE_UNUSED,
			      struct xcoff_loader_info *ldinfo,
			      struct internal_ldsym *ldsym,
			      const char *name)
{
  size_t len;
  len = strlen (name);

  if (len <= SYMNMLEN)
    strncpy (ldsym->_l._l_name, name, SYMNMLEN);
  else
    {
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
    }

  return true;
}

static bool
_bfd_xcoff_put_symbol_name (struct bfd_link_info *info,
			    struct bfd_strtab_hash *strtab,
			    struct internal_syment *sym,
			    const char *name)
{
  if (strlen (name) <= SYMNMLEN)
    {
      strncpy (sym->_n._n_name, name, SYMNMLEN);
    }
  else
    {
      bool hash;
      bfd_size_type indx;

      hash = !info->traditional_format;
      indx = _bfd_stringtab_add (strtab, name, hash, false);
      if (indx == (bfd_size_type) -1)
	return false;
      sym->_n._n_n._n_zeroes = 0;
      sym->_n._n_n._n_offset = STRING_SIZE_SIZE + indx;
    }
  return true;
}
#if GCC_VERSION >= 8000
# pragma GCC diagnostic pop
#endif

static asection *
xcoff_create_csect_from_smclas (bfd *abfd,
				union internal_auxent *aux,
				const char *symbol_name)
{
  asection *return_value = NULL;

  /* .sv64 = x_smclas == 17
     This is an invalid csect for 32 bit apps.  */
  static const char * const names[] =
    {
      ".pr", ".ro", ".db", ".tc", ".ua", ".rw", ".gl", ".xo", /* 0 - 7 */
      ".sv", ".bs", ".ds", ".uc", ".ti", ".tb", NULL, ".tc0", /* 8 - 15 */
      ".td", NULL, ".sv3264", NULL, ".tl", ".ul", ".te"
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
xcoff_is_lineno_count_overflow (bfd *abfd ATTRIBUTE_UNUSED, bfd_vma value)
{
  if (0xffff <= value)
    return true;

  return false;
}

static bool
xcoff_is_reloc_count_overflow (bfd *abfd ATTRIBUTE_UNUSED, bfd_vma value)
{
  if (0xffff <= value)
    return true;

  return false;
}

static bfd_vma
xcoff_loader_symbol_offset (bfd *abfd,
			    struct internal_ldhdr *ldhdr ATTRIBUTE_UNUSED)
{
  return bfd_xcoff_ldhdrsz (abfd);
}

static bfd_vma
xcoff_loader_reloc_offset (bfd *abfd, struct internal_ldhdr *ldhdr)
{
  return bfd_xcoff_ldhdrsz (abfd) + ldhdr->l_nsyms * bfd_xcoff_ldsymsz (abfd);
}

static bool
xcoff_generate_rtinit  (bfd *abfd, const char *init, const char *fini,
			bool rtld)
{
  bfd_byte filehdr_ext[FILHSZ];
  bfd_byte scnhdr_ext[SCNHSZ];
  bfd_byte syment_ext[SYMESZ * 10];
  bfd_byte reloc_ext[RELSZ * 3];
  bfd_byte *data_buffer;
  bfd_size_type data_buffer_size;
  bfd_byte *string_table = NULL, *st_tmp = NULL;
  bfd_size_type string_table_size;
  bfd_vma val;
  size_t initsz, finisz;
  struct internal_filehdr filehdr;
  struct internal_scnhdr scnhdr;
  struct internal_syment syment;
  union internal_auxent auxent;
  struct internal_reloc reloc;

  char *data_name = ".data";
  char *rtinit_name = "__rtinit";
  char *rtld_name = "__rtld";

  if (! bfd_xcoff_rtinit_size (abfd))
    return false;

  initsz = (init == NULL ? 0 : 1 + strlen (init));
  finisz = (fini == NULL ? 0 : 1 + strlen (fini));

  /* file header */
  memset (filehdr_ext, 0, FILHSZ);
  memset (&filehdr, 0, sizeof (struct internal_filehdr));
  filehdr.f_magic = bfd_xcoff_magic_number (abfd);
  filehdr.f_nscns = 1;
  filehdr.f_timdat = 0;
  filehdr.f_nsyms = 0;  /* at least 6, no more than 10 */
  filehdr.f_symptr = 0; /* set below */
  filehdr.f_opthdr = 0;
  filehdr.f_flags = 0;

  /* section header */
  memset (scnhdr_ext, 0, SCNHSZ);
  memset (&scnhdr, 0, sizeof (struct internal_scnhdr));
  memcpy (scnhdr.s_name, data_name, strlen (data_name));
  scnhdr.s_paddr = 0;
  scnhdr.s_vaddr = 0;
  scnhdr.s_size = 0;    /* set below */
  scnhdr.s_scnptr = FILHSZ + SCNHSZ;
  scnhdr.s_relptr = 0;  /* set below */
  scnhdr.s_lnnoptr = 0;
  scnhdr.s_nreloc = 0;  /* either 1 or 2 */
  scnhdr.s_nlnno = 0;
  scnhdr.s_flags = STYP_DATA;

  /* .data
     0x0000	      0x00000000 : rtl
     0x0004	      0x00000010 : offset to init, or 0
     0x0008	      0x00000028 : offset to fini, or 0
     0x000C	      0x0000000C : size of descriptor
     0x0010	      0x00000000 : init, needs a reloc
     0x0014	      0x00000040 : offset to init name
     0x0018	      0x00000000 : flags, padded to a word
     0x001C	      0x00000000 : empty init
     0x0020	      0x00000000 :
     0x0024	      0x00000000 :
     0x0028	      0x00000000 : fini, needs a reloc
     0x002C	      0x00000??? : offset to fini name
     0x0030	      0x00000000 : flags, padded to a word
     0x0034	      0x00000000 : empty fini
     0x0038	      0x00000000 :
     0x003C	      0x00000000 :
     0x0040	      init name
     0x0040 + initsz  fini name */

  data_buffer_size = 0x0040 + initsz + finisz;
  data_buffer_size = (data_buffer_size + 7) &~ (bfd_size_type) 7;
  data_buffer = NULL;
  data_buffer = (bfd_byte *) bfd_zmalloc (data_buffer_size);
  if (data_buffer == NULL)
    return false;

  if (initsz)
    {
      val = 0x10;
      bfd_h_put_32 (abfd, val, &data_buffer[0x04]);
      val = 0x40;
      bfd_h_put_32 (abfd, val, &data_buffer[0x14]);
      memcpy (&data_buffer[val], init, initsz);
    }

  if (finisz)
    {
      val = 0x28;
      bfd_h_put_32 (abfd, val, &data_buffer[0x08]);
      val = 0x40 + initsz;
      bfd_h_put_32 (abfd, val, &data_buffer[0x2C]);
      memcpy (&data_buffer[val], fini, finisz);
    }

  val = 0x0C;
  bfd_h_put_32 (abfd, val, &data_buffer[0x0C]);

  scnhdr.s_size = data_buffer_size;

  /* string table */
  string_table_size = 0;
  if (initsz > 9)
    string_table_size += initsz;
  if (finisz > 9)
    string_table_size += finisz;
  if (string_table_size)
    {
      string_table_size += 4;
      string_table = (bfd_byte *) bfd_zmalloc (string_table_size);
      if (string_table == NULL)
	return false;

      val = string_table_size;
      bfd_h_put_32 (abfd, val, &string_table[0]);
      st_tmp = string_table + 4;
    }

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
  memcpy (syment._n._n_name, data_name, strlen (data_name));
  syment.n_scnum = 1;
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
  memcpy (syment._n._n_name, rtinit_name, strlen (rtinit_name));
  syment.n_scnum = 1;
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

  /* init */
  if (initsz)
    {
      memset (&syment, 0, sizeof (struct internal_syment));
      memset (&auxent, 0, sizeof (union internal_auxent));

      if (initsz > 9)
	{
	  syment._n._n_n._n_offset = st_tmp - string_table;
	  memcpy (st_tmp, init, initsz);
	  st_tmp += initsz;
	}
      else
	memcpy (syment._n._n_name, init, initsz - 1);

      syment.n_sclass = C_EXT;
      syment.n_numaux = 1;
      bfd_coff_swap_sym_out (abfd, &syment,
			     &syment_ext[filehdr.f_nsyms * SYMESZ]);
      bfd_coff_swap_aux_out (abfd, &auxent, syment.n_type, syment.n_sclass, 0,
			     syment.n_numaux,
			     &syment_ext[(filehdr.f_nsyms + 1) * SYMESZ]);

      /* reloc */
      memset (&reloc, 0, sizeof (struct internal_reloc));
      reloc.r_vaddr = 0x0010;
      reloc.r_symndx = filehdr.f_nsyms;
      reloc.r_type = R_POS;
      reloc.r_size = 31;
      bfd_coff_swap_reloc_out (abfd, &reloc, &reloc_ext[0]);

      filehdr.f_nsyms += 2;
      scnhdr.s_nreloc += 1;
    }

  /* fini */
  if (finisz)
    {
      memset (&syment, 0, sizeof (struct internal_syment));
      memset (&auxent, 0, sizeof (union internal_auxent));

      if (finisz > 9)
	{
	  syment._n._n_n._n_offset = st_tmp - string_table;
	  memcpy (st_tmp, fini, finisz);
	  st_tmp += finisz;
	}
      else
	memcpy (syment._n._n_name, fini, finisz - 1);

      syment.n_sclass = C_EXT;
      syment.n_numaux = 1;
      bfd_coff_swap_sym_out (abfd, &syment,
			     &syment_ext[filehdr.f_nsyms * SYMESZ]);
      bfd_coff_swap_aux_out (abfd, &auxent, syment.n_type, syment.n_sclass, 0,
			     syment.n_numaux,
			     &syment_ext[(filehdr.f_nsyms + 1) * SYMESZ]);

      /* reloc */
      memset (&reloc, 0, sizeof (struct internal_reloc));
      reloc.r_vaddr = 0x0028;
      reloc.r_symndx = filehdr.f_nsyms;
      reloc.r_type = R_POS;
      reloc.r_size = 31;
      bfd_coff_swap_reloc_out (abfd, &reloc,
			       &reloc_ext[scnhdr.s_nreloc * RELSZ]);

      filehdr.f_nsyms += 2;
      scnhdr.s_nreloc += 1;
    }

  if (rtld)
    {
      memset (&syment, 0, sizeof (struct internal_syment));
      memset (&auxent, 0, sizeof (union internal_auxent));
      memcpy (syment._n._n_name, rtld_name, strlen (rtld_name));
      syment.n_sclass = C_EXT;
      syment.n_numaux = 1;
      bfd_coff_swap_sym_out (abfd, &syment,
			     &syment_ext[filehdr.f_nsyms * SYMESZ]);
      bfd_coff_swap_aux_out (abfd, &auxent, syment.n_type, syment.n_sclass, 0,
			     syment.n_numaux,
			     &syment_ext[(filehdr.f_nsyms + 1) * SYMESZ]);

      /* reloc */
      memset (&reloc, 0, sizeof (struct internal_reloc));
      reloc.r_vaddr = 0x0000;
      reloc.r_symndx = filehdr.f_nsyms;
      reloc.r_type = R_POS;
      reloc.r_size = 31;
      bfd_coff_swap_reloc_out (abfd, &reloc,
			       &reloc_ext[scnhdr.s_nreloc * RELSZ]);

      filehdr.f_nsyms += 2;
      scnhdr.s_nreloc += 1;
    }

  scnhdr.s_relptr = scnhdr.s_scnptr + data_buffer_size;
  filehdr.f_symptr = scnhdr.s_relptr + scnhdr.s_nreloc * RELSZ;

  bfd_coff_swap_filehdr_out (abfd, &filehdr, filehdr_ext);
  bfd_bwrite (filehdr_ext, FILHSZ, abfd);
  bfd_coff_swap_scnhdr_out (abfd, &scnhdr, scnhdr_ext);
  bfd_bwrite (scnhdr_ext, SCNHSZ, abfd);
  bfd_bwrite (data_buffer, data_buffer_size, abfd);
  bfd_bwrite (reloc_ext, scnhdr.s_nreloc * RELSZ, abfd);
  bfd_bwrite (syment_ext, filehdr.f_nsyms * SYMESZ, abfd);
  bfd_bwrite (string_table, string_table_size, abfd);

  free (data_buffer);
  data_buffer = NULL;

  return true;
}


static reloc_howto_type xcoff_dynamic_reloc =
HOWTO (0,			/* type */
       0,			/* rightshift */
       4,			/* size */
       32,			/* bitsize */
       false,			/* pc_relative */
       0,			/* bitpos */
       complain_overflow_bitfield, /* complain_on_overflow */
       0,			/* special_function */
       "R_POS",			/* name */
       true,			/* partial_inplace */
       0xffffffff,		/* src_mask */
       0xffffffff,		/* dst_mask */
       false);			/* pcrel_offset */

/* Indirect call stub
   The first word of the code must be modified by filling in
   the correct TOC offset.  */

static const unsigned long xcoff_stub_indirect_call_code[4] =
  {
    0x81820000,	/* lwz r12,0(r2) */
    0x800c0000,	/* lwz r0,0(r12) */
    0x7c0903a6,	/* mtctr r0 */
    0x4e800420,	/* bctr */
  };

/*  Shared call stub
    The first word of the code must be modified by filling in
    the correct TOC offset.
    This is exactly as the glink code but without the traceback,
    as it won't be an independent function.  */

static const unsigned long xcoff_stub_shared_call_code[6] =
  {
    0x81820000,	/* lwz r12,0(r2) */
    0x90410014,	/* stw r2,20(r1) */
    0x800c0000,	/* lwz r0,0(r12) */
    0x804c0004,	/* lwz r2,4(r12) */
    0x7c0903a6,	/* mtctr r0 */
    0x4e800420,	/* bctr */
  };

/*  glink

   The first word of global linkage code must be modified by filling in
   the correct TOC offset.  */

static const unsigned long xcoff_glink_code[9] =
  {
    0x81820000,	/* lwz r12,0(r2) */
    0x90410014,	/* stw r2,20(r1) */
    0x800c0000,	/* lwz r0,0(r12) */
    0x804c0004,	/* lwz r2,4(r12) */
    0x7c0903a6,	/* mtctr r0 */
    0x4e800420,	/* bctr */
    0x00000000,	/* start of traceback table */
    0x000c8000,	/* traceback table */
    0x00000000,	/* traceback table */
  };

/* Table to convert DWARF flags to section names.
   Remember to update binutils/dwarf.c:debug_displays
   if new DWARF sections are supported by XCOFF.  */

const struct xcoff_dwsect_name xcoff_dwsect_names[] = {
  { SSUBTYP_DWINFO,  ".dwinfo",  ".debug_info",     true },
  { SSUBTYP_DWLINE,  ".dwline",  ".debug_line",     true },
  { SSUBTYP_DWPBNMS, ".dwpbnms", ".debug_pubnames", true },
  { SSUBTYP_DWPBTYP, ".dwpbtyp", ".debug_pubtypes", true },
  { SSUBTYP_DWARNGE, ".dwarnge", ".debug_aranges",  true },
  { SSUBTYP_DWABREV, ".dwabrev", ".debug_abbrev",   false },
  { SSUBTYP_DWSTR,   ".dwstr",   ".debug_str",      true },
  { SSUBTYP_DWRNGES, ".dwrnges", ".debug_ranges",   true },
  { SSUBTYP_DWLOC,   ".dwloc",   ".debug_loc",      true },
  { SSUBTYP_DWFRAME, ".dwframe", ".debug_frame",    true },
  { SSUBTYP_DWMAC,   ".dwmac",   ".debug_macro",    true }
};

/* For generic entry points.  */
#define _bfd_xcoff_close_and_cleanup coff_close_and_cleanup
#define _bfd_xcoff_bfd_free_cached_info coff_bfd_free_cached_info
#define _bfd_xcoff_new_section_hook coff_new_section_hook
#define _bfd_xcoff_get_section_contents _bfd_generic_get_section_contents
#define _bfd_xcoff_get_section_contents_in_window \
  _bfd_generic_get_section_contents_in_window

/* For copy private data entry points.  */
#define _bfd_xcoff_bfd_copy_private_bfd_data \
  _bfd_xcoff_copy_private_bfd_data
#define _bfd_xcoff_bfd_merge_private_bfd_data \
  _bfd_generic_bfd_merge_private_bfd_data
#define _bfd_xcoff_bfd_copy_private_section_data \
  _bfd_generic_bfd_copy_private_section_data
#define _bfd_xcoff_bfd_copy_private_symbol_data \
   _bfd_generic_bfd_copy_private_symbol_data
#define _bfd_xcoff_bfd_copy_private_header_data \
   _bfd_generic_bfd_copy_private_header_data
#define _bfd_xcoff_bfd_set_private_flags \
   _bfd_generic_bfd_set_private_flags
#define _bfd_xcoff_bfd_print_private_bfd_data \
   _bfd_generic_bfd_print_private_bfd_data

/* For archive entry points.  */
#define _bfd_xcoff_slurp_extended_name_table \
   _bfd_noarchive_slurp_extended_name_table
#define _bfd_xcoff_construct_extended_name_table \
   _bfd_noarchive_construct_extended_name_table
#define _bfd_xcoff_truncate_arname bfd_dont_truncate_arname
#define _bfd_xcoff_write_ar_hdr _bfd_generic_write_ar_hdr
#define _bfd_xcoff_get_elt_at_index _bfd_generic_get_elt_at_index
#define _bfd_xcoff_generic_stat_arch_elt _bfd_xcoff_stat_arch_elt
#define _bfd_xcoff_update_armap_timestamp _bfd_bool_bfd_true

/* For symbols entry points.  */
#define _bfd_xcoff_get_symtab_upper_bound coff_get_symtab_upper_bound
#define _bfd_xcoff_canonicalize_symtab coff_canonicalize_symtab
#define _bfd_xcoff_make_empty_symbol coff_make_empty_symbol
#define _bfd_xcoff_print_symbol coff_print_symbol
#define _bfd_xcoff_get_symbol_info coff_get_symbol_info
#define _bfd_xcoff_get_symbol_version_string \
  _bfd_nosymbols_get_symbol_version_string
#define _bfd_xcoff_bfd_is_local_label_name _bfd_xcoff_is_local_label_name
#define _bfd_xcoff_bfd_is_target_special_symbol \
  coff_bfd_is_target_special_symbol
#define _bfd_xcoff_get_lineno coff_get_lineno
#define _bfd_xcoff_find_nearest_line coff_find_nearest_line
#define _bfd_xcoff_find_nearest_line_with_alt \
coff_find_nearest_line_with_alt
#define _bfd_xcoff_find_line coff_find_line
#define _bfd_xcoff_find_inliner_info coff_find_inliner_info
#define _bfd_xcoff_bfd_make_debug_symbol coff_bfd_make_debug_symbol
#define _bfd_xcoff_read_minisymbols _bfd_generic_read_minisymbols
#define _bfd_xcoff_minisymbol_to_symbol _bfd_generic_minisymbol_to_symbol

/* For reloc entry points.  */
#define _bfd_xcoff_get_reloc_upper_bound coff_get_reloc_upper_bound
#define _bfd_xcoff_canonicalize_reloc coff_canonicalize_reloc
#define _bfd_xcoff_set_reloc _bfd_generic_set_reloc
#define _bfd_xcoff_bfd_reloc_type_lookup _bfd_xcoff_reloc_type_lookup
#define _bfd_xcoff_bfd_reloc_name_lookup _bfd_xcoff_reloc_name_lookup

/* For link entry points.  */
#define _bfd_xcoff_bfd_get_relocated_section_contents \
  bfd_generic_get_relocated_section_contents
#define _bfd_xcoff_bfd_relax_section bfd_generic_relax_section
#define _bfd_xcoff_bfd_link_hash_table_free _bfd_generic_link_hash_table_free
#define _bfd_xcoff_bfd_link_just_syms _bfd_generic_link_just_syms
#define _bfd_xcoff_bfd_copy_link_hash_symbol_type \
  _bfd_generic_copy_link_hash_symbol_type
#define _bfd_xcoff_bfd_link_split_section _bfd_generic_link_split_section
#define _bfd_xcoff_bfd_gc_sections bfd_generic_gc_sections
#define _bfd_xcoff_bfd_lookup_section_flags bfd_generic_lookup_section_flags
#define _bfd_xcoff_bfd_merge_sections bfd_generic_merge_sections
#define _bfd_xcoff_bfd_is_group_section bfd_generic_is_group_section
#define _bfd_xcoff_bfd_group_name bfd_generic_group_name
#define _bfd_xcoff_bfd_discard_group bfd_generic_discard_group
#define _bfd_xcoff_section_already_linked _bfd_generic_section_already_linked
#define _bfd_xcoff_bfd_define_common_symbol _bfd_xcoff_define_common_symbol
#define _bfd_xcoff_bfd_link_hide_symbol _bfd_generic_link_hide_symbol
#define _bfd_xcoff_bfd_define_start_stop    bfd_generic_define_start_stop
#define _bfd_xcoff_bfd_link_check_relocs    _bfd_generic_link_check_relocs

/* For dynamic symbols and relocs entry points.  */
#define _bfd_xcoff_get_synthetic_symtab _bfd_nodynamic_get_synthetic_symtab

static const struct xcoff_backend_data_rec bfd_xcoff_backend_data =
  {
    { /* COFF backend, defined in libcoff.h.  */
      _bfd_xcoff_swap_aux_in,
      _bfd_xcoff_swap_sym_in,
      coff_swap_lineno_in,
      _bfd_xcoff_swap_aux_out,
      _bfd_xcoff_swap_sym_out,
      coff_swap_lineno_out,
      xcoff_swap_reloc_out,
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
      false,			/* _bfd_coff_force_symnames_in_strings */
      2,			/* _bfd_coff_debug_string_prefix_length */
      32768,			/* _bfd_coff_max_nscns */
      coff_swap_filehdr_in,
      coff_swap_aouthdr_in,
      coff_swap_scnhdr_in,
      xcoff_swap_reloc_in,
      coff_bad_format_hook,
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
      xcoff_ppc_relocate_section,
      coff_rtype_to_howto,
      NULL,			/* _bfd_coff_adjust_symndx */
      _bfd_generic_link_add_one_symbol,
      coff_link_output_has_begun,
      coff_final_link_postscript,
      NULL			/* print_pdata.  */
    },

    0x01DF,			/* magic number */
    bfd_arch_rs6000,
    bfd_mach_rs6k,

    /* Function pointers to xcoff specific swap routines.  */
    xcoff_swap_ldhdr_in,
    xcoff_swap_ldhdr_out,
    xcoff_swap_ldsym_in,
    xcoff_swap_ldsym_out,
    xcoff_swap_ldrel_in,
    xcoff_swap_ldrel_out,

    /* Sizes.  */
    LDHDRSZ,
    LDSYMSZ,
    LDRELSZ,
    12,				/* _xcoff_function_descriptor_size */
    SMALL_AOUTSZ,

    /* Versions.  */
    1,				/* _xcoff_ldhdr_version */

    _bfd_xcoff_put_symbol_name,
    _bfd_xcoff_put_ldsymbol_name,
    &xcoff_dynamic_reloc,
    xcoff_create_csect_from_smclas,

    /* Lineno and reloc count overflow.  */
    xcoff_is_lineno_count_overflow,
    xcoff_is_reloc_count_overflow,

    xcoff_loader_symbol_offset,
    xcoff_loader_reloc_offset,

    /* glink.  */
    &xcoff_glink_code[0],
    36,				/* _xcoff_glink_size */

    /* rtinit */
    64,				/* _xcoff_rtinit_size */
    xcoff_generate_rtinit,

    /* Stub indirect call.  */
    &xcoff_stub_indirect_call_code[0],
    16,				/* _xcoff_stub_indirect_call_size */

    /* Stub shared call.  */
    &xcoff_stub_shared_call_code[0],
    24,				/* _xcoff_stub_shared_call_size */
  };

/* The transfer vector that leads the outside world to all of the above.  */
const bfd_target rs6000_xcoff_vec =
  {
    "aixcoff-rs6000",
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
      _bfd_xcoff_archive_p,
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

    BFD_JUMP_TABLE_GENERIC (_bfd_xcoff),
    BFD_JUMP_TABLE_COPY (_bfd_xcoff),
    BFD_JUMP_TABLE_CORE (coff),
    BFD_JUMP_TABLE_ARCHIVE (_bfd_xcoff),
    BFD_JUMP_TABLE_SYMBOLS (_bfd_xcoff),
    BFD_JUMP_TABLE_RELOCS (_bfd_xcoff),
    BFD_JUMP_TABLE_WRITE (coff),
    BFD_JUMP_TABLE_LINK (_bfd_xcoff),
    BFD_JUMP_TABLE_DYNAMIC (_bfd_xcoff),

    /* Opposite endian version, none exists */
    NULL,

    & bfd_xcoff_backend_data,
  };

/* xcoff-powermac target
   Old target.
   Only difference between this target and the rs6000 target is the
   the default architecture and machine type used in coffcode.h

   PowerPC Macs use the same magic numbers as RS/6000
   (because that's how they were bootstrapped originally),
   but they are always PowerPC architecture.  */
static const struct xcoff_backend_data_rec bfd_pmac_xcoff_backend_data =
  {
    { /* COFF backend, defined in libcoff.h.  */
      _bfd_xcoff_swap_aux_in,
      _bfd_xcoff_swap_sym_in,
      coff_swap_lineno_in,
      _bfd_xcoff_swap_aux_out,
      _bfd_xcoff_swap_sym_out,
      coff_swap_lineno_out,
      xcoff_swap_reloc_out,
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
      false,			/* _bfd_coff_force_symnames_in_strings */
      2,			/* _bfd_coff_debug_string_prefix_length */
      32768,			/* _bfd_coff_max_nscns */
      coff_swap_filehdr_in,
      coff_swap_aouthdr_in,
      coff_swap_scnhdr_in,
      xcoff_swap_reloc_in,
      coff_bad_format_hook,
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
      xcoff_ppc_relocate_section,
      coff_rtype_to_howto,
      NULL,			/* _bfd_coff_adjust_symndx */
      _bfd_generic_link_add_one_symbol,
      coff_link_output_has_begun,
      coff_final_link_postscript,
      NULL			/* print_pdata.  */
    },

    0x01DF,			/* magic number */
    bfd_arch_powerpc,
    bfd_mach_ppc,

    /* Function pointers to xcoff specific swap routines.  */
    xcoff_swap_ldhdr_in,
    xcoff_swap_ldhdr_out,
    xcoff_swap_ldsym_in,
    xcoff_swap_ldsym_out,
    xcoff_swap_ldrel_in,
    xcoff_swap_ldrel_out,

    /* Sizes.  */
    LDHDRSZ,
    LDSYMSZ,
    LDRELSZ,
    12,				/* _xcoff_function_descriptor_size */
    SMALL_AOUTSZ,

    /* Versions.  */
    1,				/* _xcoff_ldhdr_version */

    _bfd_xcoff_put_symbol_name,
    _bfd_xcoff_put_ldsymbol_name,
    &xcoff_dynamic_reloc,
    xcoff_create_csect_from_smclas,

    /* Lineno and reloc count overflow.  */
    xcoff_is_lineno_count_overflow,
    xcoff_is_reloc_count_overflow,

    xcoff_loader_symbol_offset,
    xcoff_loader_reloc_offset,

    /* glink.  */
    &xcoff_glink_code[0],
    36,				/* _xcoff_glink_size */

    /* rtinit */
    0,				/* _xcoff_rtinit_size */
    xcoff_generate_rtinit,

    /* Stub indirect call.  */
    &xcoff_stub_indirect_call_code[0],
    16,				/* _xcoff_stub_indirect_call_size */

    /* Stub shared call.  */
    &xcoff_stub_shared_call_code[0],
    24,				/* _xcoff_stub_shared_call_size */
  };

/* The transfer vector that leads the outside world to all of the above.  */
const bfd_target powerpc_xcoff_vec =
  {
    "xcoff-powermac",
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
      _bfd_xcoff_archive_p,
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

    BFD_JUMP_TABLE_GENERIC (_bfd_xcoff),
    BFD_JUMP_TABLE_COPY (_bfd_xcoff),
    BFD_JUMP_TABLE_CORE (coff),
    BFD_JUMP_TABLE_ARCHIVE (_bfd_xcoff),
    BFD_JUMP_TABLE_SYMBOLS (_bfd_xcoff),
    BFD_JUMP_TABLE_RELOCS (_bfd_xcoff),
    BFD_JUMP_TABLE_WRITE (coff),
    BFD_JUMP_TABLE_LINK (_bfd_xcoff),
    BFD_JUMP_TABLE_DYNAMIC (_bfd_xcoff),

    /* Opposite endian version, none exists */
    NULL,

    & bfd_pmac_xcoff_backend_data,
  };
