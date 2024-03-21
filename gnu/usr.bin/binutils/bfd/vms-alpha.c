/* vms.c -- BFD back-end for EVAX (openVMS/Alpha) files.
   Copyright (C) 1996-2023 Free Software Foundation, Inc.

   Initial version written by Klaus Kaempf (kkaempf@rmi.de)
   Major rewrite by Adacore.

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

/* TODO:
   o  overlayed sections
   o  PIC
   o  Generation of shared image
   o  Relocation optimizations
   o  EISD for the stack
   o  Vectors isect
   o  64 bits sections
   o  Entry point
   o  LIB$INITIALIZE
   o  protected sections (for messages)
   ...
*/

#include "sysdep.h"
#include <limits.h>
#include "bfd.h"
#include "bfdlink.h"
#include "libbfd.h"
#include "bfdver.h"

#include "vms.h"
#include "vms/eihd.h"
#include "vms/eiha.h"
#include "vms/eihi.h"
#include "vms/eihs.h"
#include "vms/eisd.h"
#include "vms/dmt.h"
#include "vms/dst.h"
#include "vms/eihvn.h"
#include "vms/eobjrec.h"
#include "vms/egsd.h"
#include "vms/egps.h"
#include "vms/esgps.h"
#include "vms/eeom.h"
#include "vms/emh.h"
#include "vms/eiaf.h"
#include "vms/shl.h"
#include "vms/eicp.h"
#include "vms/etir.h"
#include "vms/egsy.h"
#include "vms/esdf.h"
#include "vms/esdfm.h"
#include "vms/esdfv.h"
#include "vms/esrf.h"
#include "vms/egst.h"
#include "vms/eidc.h"
#include "vms/dsc.h"
#include "vms/prt.h"
#include "vms/internal.h"


#define MIN(a,b) ((a) < (b) ? (a) : (b))
#ifndef CHAR_BIT
#define CHAR_BIT 8
#endif

/* The r_type field in a reloc is one of the following values.  */
#define ALPHA_R_IGNORE		0
#define ALPHA_R_REFQUAD		1
#define ALPHA_R_BRADDR		2
#define ALPHA_R_HINT		3
#define ALPHA_R_SREL16		4
#define ALPHA_R_SREL32		5
#define ALPHA_R_SREL64		6
#define ALPHA_R_OP_PUSH		7
#define ALPHA_R_OP_STORE	8
#define ALPHA_R_OP_PSUB		9
#define ALPHA_R_OP_PRSHIFT	10
#define ALPHA_R_LINKAGE		11
#define ALPHA_R_REFLONG		12
#define ALPHA_R_CODEADDR	13
#define ALPHA_R_NOP		14
#define ALPHA_R_BSR		15
#define ALPHA_R_LDA		16
#define ALPHA_R_BOH		17

/* These are used with DST_S_C_LINE_NUM.  */
#define DST_S_C_LINE_NUM_HEADER_SIZE 4

/* These are used with DST_S_C_SOURCE */

#define DST_S_B_PCLINE_UNSBYTE	 1
#define DST_S_W_PCLINE_UNSWORD	 1
#define DST_S_L_PCLINE_UNSLONG	 1

#define DST_S_B_MODBEG_NAME	14
#define DST_S_L_RTNBEG_ADDRESS	 5
#define DST_S_B_RTNBEG_NAME	13
#define DST_S_L_RTNEND_SIZE	 5

/* These are used with DST_S_C_SOURCE.  */
#define DST_S_C_SOURCE_HEADER_SIZE 4

#define DST_S_B_SRC_DF_LENGTH	  1
#define DST_S_W_SRC_DF_FILEID	  3
#define DST_S_B_SRC_DF_FILENAME	 20
#define DST_S_B_SRC_UNSBYTE	  1
#define DST_S_W_SRC_UNSWORD	  1
#define DST_S_L_SRC_UNSLONG	  1

/* Debugger symbol definitions.  */

#define DBG_S_L_DMT_MODBEG	 0
#define DBG_S_L_DST_SIZE	 4
#define DBG_S_W_DMT_PSECT_COUNT	 8
#define DBG_S_C_DMT_HEADER_SIZE 12

#define DBG_S_L_DMT_PSECT_START	 0
#define DBG_S_L_DMT_PSECT_LENGTH 4
#define DBG_S_C_DMT_PSECT_SIZE	 8

/* VMS module header.  */

struct hdr_struct
{
  char hdr_b_strlvl;
  int hdr_l_arch1;
  int hdr_l_arch2;
  int hdr_l_recsiz;
  char *hdr_t_name;
  char *hdr_t_version;
  char *hdr_t_date;
  char *hdr_c_lnm;
  char *hdr_c_src;
  char *hdr_c_ttl;
};

#define EMH_DATE_LENGTH  17

/* VMS End-Of-Module records (EOM/EEOM).  */

struct eom_struct
{
  unsigned int eom_l_total_lps;
  unsigned short eom_w_comcod;
  bool eom_has_transfer;
  unsigned char eom_b_tfrflg;
  unsigned int eom_l_psindx;
  unsigned int eom_l_tfradr;
};

struct vms_symbol_entry
{
  bfd *owner;

  /* Common fields.  */
  unsigned char typ;
  unsigned char data_type;
  unsigned short flags;

  /* Section and offset/value of the symbol.  */
  unsigned int value;
  asection *section;

  /* Section and offset/value for the entry point (only for subprg).  */
  asection *code_section;
  unsigned int code_value;

  /* Symbol vector offset.  */
  unsigned int symbol_vector;

  /* Length of the name.  */
  unsigned char namelen;

  char name[1];
};

/* Stack value for push/pop commands.  */

struct stack_struct
{
  bfd_vma value;
  unsigned int reloc;
};

#define STACKSIZE 128

/* A minimal decoding of DST compilation units.  We only decode
   what's needed to get to the line number information.  */

struct fileinfo
{
  char *name;
  unsigned int srec;
};

struct srecinfo
{
  struct srecinfo *next;
  unsigned int line;
  unsigned int sfile;
  unsigned int srec;
};

struct lineinfo
{
  struct lineinfo *next;
  bfd_vma address;
  unsigned int line;
};

struct funcinfo
{
  struct funcinfo *next;
  char *name;
  bfd_vma low;
  bfd_vma high;
};

struct module
{
  /* Chain the previously read compilation unit.  */
  struct module *next;

  /* The module name.  */
  char *name;

  /* The start offset and size of debug info in the DST section.  */
  unsigned int modbeg;
  unsigned int size;

  /* The lowest and highest addresses contained in this compilation
     unit as specified in the compilation unit header.  */
  bfd_vma low;
  bfd_vma high;

  /* The listing line table.  */
  struct lineinfo *line_table;

  /* The source record table.  */
  struct srecinfo *srec_table;

  /* A list of the functions found in this module.  */
  struct funcinfo *func_table;

  /* Current allocation of file_table.  */
  unsigned int file_table_count;

  /* An array of the files making up this module.  */
  struct fileinfo *file_table;
};

/* BFD private data for alpha-vms.  */

struct vms_private_data_struct
{
  /* If 1, relocs have been read successfully, if 0 they have yet to be
     read, if -1 reading relocs failed.  */
  int reloc_done;

  /* Record input buffer.  */
  struct vms_rec_rd recrd;
  struct vms_rec_wr recwr;

  struct hdr_struct hdr_data;		/* data from HDR/EMH record  */
  struct eom_struct eom_data;		/* data from EOM/EEOM record  */

  /* Transfer addresses (entry points).  */
  bfd_vma transfer_address[4];

  /* Array of GSD sections to get the correspond BFD one.  */
  unsigned int section_max;		/* Size of the sections array.  */
  unsigned int section_count;		/* Number of GSD sections.  */
  asection **sections;

  /* Array of raw symbols.  */
  struct vms_symbol_entry **syms;

  /* Canonicalized symbols.  */
  asymbol **csymbols;

  /* Number of symbols.  */
  unsigned int gsd_sym_count;
  /* Size of the syms array.  */
  unsigned int max_sym_count;
  /* Number of procedure symbols.  */
  unsigned int norm_sym_count;

  /* Stack used to evaluate TIR/ETIR commands.  */
  struct stack_struct *stack;
  int stackptr;

  /* Content reading.  */
  asection *image_section;		/* section for image_ptr  */
  file_ptr image_offset;		/* Offset for image_ptr.  */

  struct module *modules;		/* list of all compilation units */

  /* The DST section.  */
  asection *dst_section;

  unsigned int dst_ptr_offsets_count;	/* # of offsets in following array  */
  unsigned int *dst_ptr_offsets;	/* array of saved image_ptr offsets */

  /* Shared library support */
  bfd_vma symvva; /* relative virtual address of symbol vector */
  unsigned int ident;
  unsigned char matchctl;

  /* Shared library index.  This is used for input bfd while linking.  */
  unsigned int shr_index;

  /* Used to place structures in the file.  */
  file_ptr file_pos;

  /* Simply linked list of eisd.  */
  struct vms_internal_eisd_map *eisd_head;
  struct vms_internal_eisd_map *eisd_tail;

  /* Simply linked list of eisd for shared libraries.  */
  struct vms_internal_eisd_map *gbl_eisd_head;
  struct vms_internal_eisd_map *gbl_eisd_tail;

  /* linkage index counter used by conditional store commands */
  unsigned int vms_linkage_index;
};

#define PRIV2(abfd, name) \
  (((struct vms_private_data_struct *)(abfd)->tdata.any)->name)
#define PRIV(name) PRIV2(abfd,name)


/* Used to keep extra VMS specific information for a given section.

   reloc_size holds the size of the relocation stream, note this
   is very different from the number of relocations as VMS relocations
   are variable length.

   reloc_stream is the actual stream of relocation entries.  */

struct vms_section_data_struct
{
  /* Maximnum number of entries in sec->relocation.  */
  unsigned reloc_max;

  /* Corresponding eisd.  Used only while generating executables.  */
  struct vms_internal_eisd_map *eisd;

  /* PSC flags to be clear.  */
  flagword no_flags;

  /* PSC flags to be set.  */
  flagword flags;
};

#define vms_section_data(sec) \
  ((struct vms_section_data_struct *)sec->used_by_bfd)

/* To be called from the debugger.  */
struct vms_private_data_struct *bfd_vms_get_data (bfd *);

static int vms_get_remaining_object_record (bfd *, unsigned int);
static bool _bfd_vms_slurp_object_records (bfd * abfd);
static bool alpha_vms_add_fixup_lp (struct bfd_link_info *, bfd *, bfd *);
static bool alpha_vms_add_fixup_ca (struct bfd_link_info *, bfd *, bfd *);
static bool alpha_vms_add_fixup_qr (struct bfd_link_info *, bfd *, bfd *,
				    bfd_vma);
static bool alpha_vms_add_fixup_lr (struct bfd_link_info *, unsigned int,
				    bfd_vma);
static bool alpha_vms_add_lw_reloc (struct bfd_link_info *);
static bool alpha_vms_add_qw_reloc (struct bfd_link_info *);

struct vector_type
{
  unsigned int max_el;
  unsigned int nbr_el;
  void *els;
};

/* Number of elements in VEC.  */

#define VEC_COUNT(VEC) ((VEC).nbr_el)

/* Get the address of the Nth element.  */

#define VEC_EL(VEC, TYPE, N) (((TYPE *)((VEC).els))[N])

#define VEC_INIT(VEC)				\
  do {						\
    (VEC).max_el = 0;				\
    (VEC).nbr_el = 0;				\
    (VEC).els = NULL;				\
  } while (0)

/* Be sure there is room for a new element.  */

static void *vector_grow1 (struct vector_type *vec, size_t elsz);

/* Allocate room for a new element and return its address.  */

#define VEC_APPEND(VEC, TYPE)					\
  ((TYPE *) vector_grow1 (&VEC, sizeof (TYPE)))

struct alpha_vms_vma_ref
{
  bfd_vma vma;	/* Vma in the output.  */
  bfd_vma ref;	/* Reference in the input.  */
};

struct alpha_vms_shlib_el
{
  bfd *abfd;
  bool has_fixups;

  struct vector_type lp;	/* Vector of bfd_vma.  */
  struct vector_type ca;	/* Vector of bfd_vma.  */
  struct vector_type qr;	/* Vector of struct alpha_vms_vma_ref.  */
};

/* Alpha VMS linker hash table.  */

struct alpha_vms_link_hash_table
{
  struct bfd_link_hash_table root;

  /* Vector of shared libraries.  */
  struct vector_type shrlibs;

  /* Fixup section.  */
  asection *fixup;

  /* Base address.  Used by fixups.  */
  bfd_vma base_addr;
};

#define alpha_vms_link_hash(INFO) \
  ((struct alpha_vms_link_hash_table *)(INFO->hash))

/* Alpha VMS linker hash table entry.  */

struct alpha_vms_link_hash_entry
{
  struct bfd_link_hash_entry root;

  /* Pointer to the original vms symbol.  */
  struct vms_symbol_entry *sym;
};

/* Image reading.  */

/* Read & process EIHD record.
   Return TRUE on success, FALSE on error.  */

static bool
_bfd_vms_slurp_eihd (bfd *abfd, unsigned int *eisd_offset,
		     unsigned int *eihs_offset)
{
  unsigned int imgtype, size;
  bfd_vma symvva;
  struct vms_eihd *eihd = (struct vms_eihd *)PRIV (recrd.rec);

  vms_debug2 ((8, "_bfd_vms_slurp_eihd\n"));

  /* PR 21813: Check for an undersized record.  */
  if (PRIV (recrd.buf_size) < sizeof (* eihd))
    {
      _bfd_error_handler (_("corrupt EIHD record - size is too small"));
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  size = bfd_getl32 (eihd->size);
  imgtype = bfd_getl32 (eihd->imgtype);

  if (imgtype == EIHD__K_EXE || imgtype == EIHD__K_LIM)
    abfd->flags |= EXEC_P;

  symvva = bfd_getl64 (eihd->symvva);
  if (symvva != 0)
    {
      PRIV (symvva) = symvva;
      abfd->flags |= DYNAMIC;
    }

  PRIV (ident) = bfd_getl32 (eihd->ident);
  PRIV (matchctl) = eihd->matchctl;

  *eisd_offset = bfd_getl32 (eihd->isdoff);
  *eihs_offset = bfd_getl32 (eihd->symdbgoff);

  vms_debug2 ((4, "EIHD size %d imgtype %d symvva 0x%lx eisd %d eihs %d\n",
	       size, imgtype, (unsigned long)symvva,
	       *eisd_offset, *eihs_offset));
  (void) size;

  return true;
}

/* Read & process EISD record.
   Return TRUE on success, FALSE on error.  */

static bool
_bfd_vms_slurp_eisd (bfd *abfd, unsigned int offset)
{
  int section_count = 0;

  vms_debug2 ((8, "_bfd_vms_slurp_eisd\n"));

  while (1)
    {
      struct vms_eisd *eisd;
      unsigned int rec_size;
      unsigned int size;
      uint64_t vaddr;
      unsigned int flags;
      unsigned int vbn;
      char *name = NULL;
      asection *section;
      flagword bfd_flags;

      /* PR 17512: file: 3d9e9fe9.  */
      if (offset > PRIV (recrd.rec_size)
	  || (PRIV (recrd.rec_size) - offset
	      < offsetof (struct vms_eisd, eisdsize) + 4))
	return false;
      eisd = (struct vms_eisd *) (PRIV (recrd.rec) + offset);
      rec_size = bfd_getl32 (eisd->eisdsize);
      if (rec_size == 0)
	break;

      /* Skip to next block if pad.  */
      if (rec_size == 0xffffffff)
	{
	  offset = (offset + VMS_BLOCK_SIZE) & ~(VMS_BLOCK_SIZE - 1);
	  continue;
	}

      /* Make sure that there is enough data present in the record.  */
      if (rec_size < offsetof (struct vms_eisd, type) + 1)
	return false;
      /* Make sure that the record is not too big either.  */
      if (rec_size > PRIV (recrd.rec_size) - offset)
	return false;

      offset += rec_size;

      size = bfd_getl32 (eisd->secsize);
      vaddr = bfd_getl64 (eisd->virt_addr);
      flags = bfd_getl32 (eisd->flags);
      vbn = bfd_getl32 (eisd->vbn);

      vms_debug2 ((4, "EISD at 0x%x size 0x%x addr 0x%lx flags 0x%x blk %d\n",
		   offset, size, (unsigned long)vaddr, flags, vbn));

      /* VMS combines psects from .obj files into isects in the .exe.  This
	 process doesn't preserve enough information to reliably determine
	 what's in each section without examining the data.  This is
	 especially true of DWARF debug sections.  */
      bfd_flags = SEC_ALLOC;
      if (vbn != 0)
	bfd_flags |= SEC_HAS_CONTENTS | SEC_LOAD;

      if (flags & EISD__M_EXE)
	bfd_flags |= SEC_CODE;

      if (flags & EISD__M_NONSHRADR)
	bfd_flags |= SEC_DATA;

      if (!(flags & EISD__M_WRT))
	bfd_flags |= SEC_READONLY;

      if (flags & EISD__M_DZRO)
	bfd_flags |= SEC_DATA;

      if (flags & EISD__M_FIXUPVEC)
	bfd_flags |= SEC_DATA;

      if (flags & EISD__M_CRF)
	bfd_flags |= SEC_DATA;

      if (flags & EISD__M_GBL)
	{
	  if (rec_size <= offsetof (struct vms_eisd, gblnam))
	    return false;
	  else if (rec_size < sizeof (struct vms_eisd))
	    name = _bfd_vms_save_counted_string (abfd, eisd->gblnam,
						 rec_size - offsetof (struct vms_eisd, gblnam));
	  else
	    name = _bfd_vms_save_counted_string (abfd, eisd->gblnam,
						 EISD__K_GBLNAMLEN);
	  if (name == NULL || name[0] == 0)
	    return false;
	  bfd_flags |= SEC_COFF_SHARED_LIBRARY;
	  bfd_flags &= ~(SEC_ALLOC | SEC_LOAD);
	}
      else if (flags & EISD__M_FIXUPVEC)
	name = "$FIXUPVEC$";
      else if (eisd->type == EISD__K_USRSTACK)
	name = "$STACK$";
      else
	{
	  const char *pfx;

	  name = (char *) bfd_alloc (abfd, 32);
	  if (name == NULL)
	    return false;
	  if (flags & EISD__M_DZRO)
	    pfx = "BSS";
	  else if (flags & EISD__M_EXE)
	    pfx = "CODE";
	  else if (!(flags & EISD__M_WRT))
	    pfx = "RO";
	  else
	    pfx = "LOCAL";
	  BFD_ASSERT (section_count < 999);
	  sprintf (name, "$%s_%03d$", pfx, section_count++);
	}

      section = bfd_make_section (abfd, name);

      if (!section)
	return false;

      section->filepos = vbn ? VMS_BLOCK_SIZE * (vbn - 1) : 0;
      section->size = size;
      section->vma = vaddr;

      if (!bfd_set_section_flags (section, bfd_flags))
	return false;
    }

  return true;
}

/* Read & process EIHS record.
   Return TRUE on success, FALSE on error.  */

static bool
_bfd_vms_slurp_eihs (bfd *abfd, unsigned int offset)
{
  unsigned char *p = PRIV (recrd.rec) + offset;
  unsigned int gstvbn;
  unsigned int gstsize ATTRIBUTE_UNUSED;
  unsigned int dstvbn;
  unsigned int dstsize;
  unsigned int dmtvbn;
  unsigned int dmtbytes;
  asection *section;

  /* PR 21611: Check that offset is valid.  */
  if (offset > PRIV (recrd.rec_size) - (EIHS__L_DMTBYTES + 4))
    {
      _bfd_error_handler (_("unable to read EIHS record at offset %#x"),
			  offset);
      bfd_set_error (bfd_error_file_truncated);
      return false;
    }

  gstvbn   = bfd_getl32 (p + EIHS__L_GSTVBN);
  gstsize  = bfd_getl32 (p + EIHS__L_GSTSIZE);
  dstvbn   = bfd_getl32 (p + EIHS__L_DSTVBN);
  dstsize  = bfd_getl32 (p + EIHS__L_DSTSIZE);
  dmtvbn   = bfd_getl32 (p + EIHS__L_DMTVBN);
  dmtbytes = bfd_getl32 (p + EIHS__L_DMTBYTES);

#if VMS_DEBUG
  vms_debug (8, "_bfd_vms_slurp_ihs\n");
  vms_debug (4, "EIHS record gstvbn %d gstsize %d dstvbn %d dstsize %d dmtvbn %d dmtbytes %d\n",
	     gstvbn, gstsize, dstvbn, dstsize, dmtvbn, dmtbytes);
#endif

  if (dstvbn)
    {
      flagword bfd_flags = SEC_HAS_CONTENTS | SEC_DEBUGGING;

      section = bfd_make_section (abfd, "$DST$");
      if (!section)
	return false;

      section->size = dstsize;
      section->filepos = VMS_BLOCK_SIZE * (dstvbn - 1);

      if (!bfd_set_section_flags (section, bfd_flags))
	return false;

      PRIV (dst_section) = section;
      abfd->flags |= (HAS_DEBUG | HAS_LINENO);
    }

  if (dmtvbn)
    {
      flagword bfd_flags = SEC_HAS_CONTENTS | SEC_DEBUGGING;

      section = bfd_make_section (abfd, "$DMT$");
      if (!section)
	return false;

      section->size = dmtbytes;
      section->filepos = VMS_BLOCK_SIZE * (dmtvbn - 1);

      if (!bfd_set_section_flags (section, bfd_flags))
	return false;
    }

  if (gstvbn)
    {
      if (bfd_seek (abfd, VMS_BLOCK_SIZE * (gstvbn - 1), SEEK_SET))
	{
	  bfd_set_error (bfd_error_file_truncated);
	  return false;
	}

      if (!_bfd_vms_slurp_object_records (abfd))
	return false;

      abfd->flags |= HAS_SYMS;
    }

  return true;
}

/* Object file reading.  */

/* Object file input functions.  */

/* Get next record from object file to vms_buf.
   Set PRIV(buf_size) and return it

   This is a little tricky since it should be portable.

   The openVMS object file has 'variable length' which means that
   read() returns data in chunks of (hopefully) correct and expected
   size.  The linker (and other tools on VMS) depend on that. Unix
   doesn't know about 'formatted' files, so reading and writing such
   an object file in a Unix environment is not trivial.

   With the tool 'file' (available on all VMS FTP sites), one
   can view and change the attributes of a file.  Changing from
   'variable length' to 'fixed length, 512 bytes' reveals the
   record size at the first 2 bytes of every record.  The same
   may happen during the transfer of object files from VMS to Unix,
   at least with UCX, the DEC implementation of TCP/IP.

   The VMS format repeats the size at bytes 2 & 3 of every record.

   On the first call (file_format == FF_UNKNOWN) we check if
   the first and the third byte pair (!) of the record match.
   If they do it's an object file in an Unix environment or with
   wrong attributes (FF_FOREIGN), else we should be in a VMS
   environment where read() returns the record size (FF_NATIVE).

   Reading is always done in 2 steps:
    1. first just the record header is read and the size extracted,
    2. then the read buffer is adjusted and the remaining bytes are
       read in.

   All file I/O is done on even file positions.  */

#define VMS_OBJECT_ADJUSTMENT  2

static void
maybe_adjust_record_pointer_for_object (bfd *abfd)
{
  /* Set the file format once for all on the first invocation.  */
  if (PRIV (recrd.file_format) == FF_UNKNOWN)
    {
      if (PRIV (recrd.rec)[0] == PRIV (recrd.rec)[4]
	  && PRIV (recrd.rec)[1] == PRIV (recrd.rec)[5])
	PRIV (recrd.file_format) = FF_FOREIGN;
      else
	PRIV (recrd.file_format) = FF_NATIVE;
    }

  /* The adjustment is needed only in an Unix environment.  */
  if (PRIV (recrd.file_format) == FF_FOREIGN)
    PRIV (recrd.rec) += VMS_OBJECT_ADJUSTMENT;
}

/* Implement step #1 of the object record reading procedure.
   Return the record type or -1 on failure.  */

static int
_bfd_vms_get_object_record (bfd *abfd)
{
  unsigned int test_len = 6;
  int type;

  vms_debug2 ((8, "_bfd_vms_get_obj_record\n"));

  /* Skip alignment byte if the current position is odd.  */
  if (PRIV (recrd.file_format) == FF_FOREIGN && (bfd_tell (abfd) & 1))
    {
      if (bfd_bread (PRIV (recrd.buf), 1, abfd) != 1)
	{
	  bfd_set_error (bfd_error_file_truncated);
	  return -1;
	}
    }

  /* Read the record header  */
  if (bfd_bread (PRIV (recrd.buf), test_len, abfd) != test_len)
    {
      bfd_set_error (bfd_error_file_truncated);
      return -1;
    }

  /* Reset the record pointer.  */
  PRIV (recrd.rec) = PRIV (recrd.buf);
  maybe_adjust_record_pointer_for_object (abfd);

  if (vms_get_remaining_object_record (abfd, test_len) <= 0)
    return -1;

  type = bfd_getl16 (PRIV (recrd.rec));

  vms_debug2 ((8, "_bfd_vms_get_obj_record: rec %p, size %d, type %d\n",
	       PRIV (recrd.rec), PRIV (recrd.rec_size), type));

  return type;
}

/* Implement step #2 of the object record reading procedure.
   Return the size of the record or 0 on failure.  */

static int
vms_get_remaining_object_record (bfd *abfd, unsigned int read_so_far)
{
  unsigned int to_read;

  vms_debug2 ((8, "vms_get_remaining_obj_record\n"));

  /* Extract record size.  */
  PRIV (recrd.rec_size) = bfd_getl16 (PRIV (recrd.rec) + 2);

  if (PRIV (recrd.rec_size) == 0)
    {
      bfd_set_error (bfd_error_file_truncated);
      return 0;
    }

  /* That's what the linker manual says.  */
  if (PRIV (recrd.rec_size) > EOBJ__C_MAXRECSIZ)
    {
      bfd_set_error (bfd_error_file_truncated);
      return 0;
    }

  /* Take into account object adjustment.  */
  to_read = PRIV (recrd.rec_size);
  if (PRIV (recrd.file_format) == FF_FOREIGN)
    to_read += VMS_OBJECT_ADJUSTMENT;

  /* Adjust the buffer.  */
  if (to_read > PRIV (recrd.buf_size))
    {
      PRIV (recrd.buf)
	= (unsigned char *) bfd_realloc_or_free (PRIV (recrd.buf), to_read);
      if (PRIV (recrd.buf) == NULL)
	return 0;
      PRIV (recrd.buf_size) = to_read;
    }
  /* PR 17512: file: 025-1974-0.004.  */
  else if (to_read <= read_so_far)
    return 0;

  /* Read the remaining record.  */
  to_read -= read_so_far;

  vms_debug2 ((8, "vms_get_remaining_obj_record: to_read %d\n", to_read));

  if (bfd_bread (PRIV (recrd.buf) + read_so_far, to_read, abfd) != to_read)
    {
      bfd_set_error (bfd_error_file_truncated);
      return 0;
    }

  /* Reset the record pointer.  */
  PRIV (recrd.rec) = PRIV (recrd.buf);
  maybe_adjust_record_pointer_for_object (abfd);

  vms_debug2 ((8, "vms_get_remaining_obj_record: size %d\n",
	       PRIV (recrd.rec_size)));

  return PRIV (recrd.rec_size);
}

/* Read and process emh record.
   Return TRUE on success, FALSE on error.  */

static bool
_bfd_vms_slurp_ehdr (bfd *abfd)
{
  unsigned char *ptr;
  unsigned char *vms_rec;
  unsigned char *end;
  int subtype;

  vms_rec = PRIV (recrd.rec);
  /* PR 17512: file: 62736583.  */
  end = PRIV (recrd.buf) + PRIV (recrd.buf_size);

  vms_debug2 ((2, "HDR/EMH\n"));

  subtype = bfd_getl16 (vms_rec + 4);

  vms_debug2 ((3, "subtype %d\n", subtype));

  switch (subtype)
    {
    case EMH__C_MHD:
      /* Module header.  */
      if (vms_rec + 21 >= end)
	goto fail;
      PRIV (hdr_data).hdr_b_strlvl = vms_rec[6];
      PRIV (hdr_data).hdr_l_arch1  = bfd_getl32 (vms_rec + 8);
      PRIV (hdr_data).hdr_l_arch2  = bfd_getl32 (vms_rec + 12);
      PRIV (hdr_data).hdr_l_recsiz = bfd_getl32 (vms_rec + 16);
      if ((vms_rec + 20 + vms_rec[20] + 1) >= end)
	goto fail;
      PRIV (hdr_data).hdr_t_name
	= _bfd_vms_save_counted_string (abfd, vms_rec + 20, vms_rec[20]);
      ptr = vms_rec + 20 + vms_rec[20] + 1;
      if ((ptr + *ptr + 1) >= end)
	goto fail;
      PRIV (hdr_data).hdr_t_version
	= _bfd_vms_save_counted_string (abfd, ptr, *ptr);
      ptr += *ptr + 1;
      if (ptr + 17 >= end)
	goto fail;
      PRIV (hdr_data).hdr_t_date
	= _bfd_vms_save_sized_string (abfd, ptr, 17);
      break;

    case EMH__C_LNM:
      if (vms_rec + PRIV (recrd.rec_size - 6) > end)
	goto fail;
      PRIV (hdr_data).hdr_c_lnm
	= _bfd_vms_save_sized_string (abfd, vms_rec, PRIV (recrd.rec_size - 6));
      break;

    case EMH__C_SRC:
      if (vms_rec + PRIV (recrd.rec_size - 6) > end)
	goto fail;
      PRIV (hdr_data).hdr_c_src
	= _bfd_vms_save_sized_string (abfd, vms_rec, PRIV (recrd.rec_size - 6));
      break;

    case EMH__C_TTL:
      if (vms_rec + PRIV (recrd.rec_size - 6) > end)
	goto fail;
      PRIV (hdr_data).hdr_c_ttl
	= _bfd_vms_save_sized_string (abfd, vms_rec, PRIV (recrd.rec_size - 6));
      break;

    case EMH__C_CPR:
    case EMH__C_MTC:
    case EMH__C_GTX:
      break;

    default:
    fail:
      bfd_set_error (bfd_error_wrong_format);
      return false;
    }

  return true;
}

/* Typical sections for evax object files.  */

#define EVAX_ABS_NAME		"$ABS$"
#define EVAX_CODE_NAME		"$CODE$"
#define EVAX_LINK_NAME		"$LINK$"
#define EVAX_DATA_NAME		"$DATA$"
#define EVAX_BSS_NAME		"$BSS$"
#define EVAX_READONLYADDR_NAME	"$READONLY_ADDR$"
#define EVAX_READONLY_NAME	"$READONLY$"
#define EVAX_LITERAL_NAME	"$LITERAL$"
#define EVAX_LITERALS_NAME	"$LITERALS"
#define EVAX_COMMON_NAME	"$COMMON$"
#define EVAX_LOCAL_NAME		"$LOCAL$"

struct sec_flags_struct
{
  const char *name;		/* Name of section.  */
  int vflags_always;
  flagword flags_always;	/* Flags we set always.  */
  int vflags_hassize;
  flagword flags_hassize;	/* Flags we set if the section has a size > 0.  */
};

/* These flags are deccrtl/vaxcrtl (openVMS 6.2 Alpha) compatible.  */

static const struct sec_flags_struct evax_section_flags[] =
  {
    { EVAX_ABS_NAME,
      EGPS__V_SHR,
      0,
      EGPS__V_SHR,
      0 },
    { EVAX_CODE_NAME,
      EGPS__V_PIC | EGPS__V_REL | EGPS__V_SHR | EGPS__V_EXE,
      SEC_CODE | SEC_READONLY,
      EGPS__V_PIC | EGPS__V_REL | EGPS__V_SHR | EGPS__V_EXE,
      SEC_CODE | SEC_READONLY | SEC_HAS_CONTENTS | SEC_ALLOC | SEC_LOAD },
    { EVAX_LITERAL_NAME,
      EGPS__V_PIC | EGPS__V_REL | EGPS__V_SHR | EGPS__V_RD | EGPS__V_NOMOD,
      SEC_DATA | SEC_READONLY,
      EGPS__V_PIC | EGPS__V_REL | EGPS__V_SHR | EGPS__V_RD,
      SEC_DATA | SEC_READONLY | SEC_HAS_CONTENTS | SEC_ALLOC | SEC_LOAD },
    { EVAX_LINK_NAME,
      EGPS__V_REL | EGPS__V_RD,
      SEC_DATA | SEC_READONLY,
      EGPS__V_REL | EGPS__V_RD,
      SEC_DATA | SEC_READONLY | SEC_HAS_CONTENTS | SEC_ALLOC | SEC_LOAD },
    { EVAX_DATA_NAME,
      EGPS__V_REL | EGPS__V_RD | EGPS__V_WRT | EGPS__V_NOMOD,
      SEC_DATA,
      EGPS__V_REL | EGPS__V_RD | EGPS__V_WRT,
      SEC_DATA | SEC_HAS_CONTENTS | SEC_ALLOC | SEC_LOAD },
    { EVAX_BSS_NAME,
      EGPS__V_REL | EGPS__V_RD | EGPS__V_WRT | EGPS__V_NOMOD,
      SEC_NO_FLAGS,
      EGPS__V_REL | EGPS__V_RD | EGPS__V_WRT | EGPS__V_NOMOD,
      SEC_ALLOC },
    { EVAX_READONLYADDR_NAME,
      EGPS__V_PIC | EGPS__V_REL | EGPS__V_RD,
      SEC_DATA | SEC_READONLY,
      EGPS__V_PIC | EGPS__V_REL | EGPS__V_RD,
      SEC_DATA | SEC_READONLY | SEC_HAS_CONTENTS | SEC_ALLOC | SEC_LOAD },
    { EVAX_READONLY_NAME,
      EGPS__V_PIC | EGPS__V_REL | EGPS__V_SHR | EGPS__V_RD | EGPS__V_NOMOD,
      SEC_DATA | SEC_READONLY,
      EGPS__V_PIC | EGPS__V_REL | EGPS__V_SHR | EGPS__V_RD,
      SEC_DATA | SEC_READONLY | SEC_HAS_CONTENTS | SEC_ALLOC | SEC_LOAD },
    { EVAX_LOCAL_NAME,
      EGPS__V_REL | EGPS__V_RD | EGPS__V_WRT,
      SEC_DATA,
      EGPS__V_REL | EGPS__V_RD | EGPS__V_WRT,
      SEC_DATA | SEC_HAS_CONTENTS | SEC_ALLOC | SEC_LOAD },
    { EVAX_LITERALS_NAME,
      EGPS__V_PIC | EGPS__V_OVR,
      SEC_DATA | SEC_READONLY,
      EGPS__V_PIC | EGPS__V_OVR,
      SEC_DATA | SEC_READONLY | SEC_HAS_CONTENTS | SEC_ALLOC | SEC_LOAD },
    { NULL,
      EGPS__V_REL | EGPS__V_RD | EGPS__V_WRT,
      SEC_DATA,
      EGPS__V_REL | EGPS__V_RD | EGPS__V_WRT,
      SEC_DATA | SEC_HAS_CONTENTS | SEC_ALLOC | SEC_LOAD }
  };

/* Retrieve BFD section flags by name and size.  */

static flagword
vms_secflag_by_name (const struct sec_flags_struct *section_flags,
		     const char *name,
		     int hassize)
{
  int i = 0;

  while (section_flags[i].name != NULL)
    {
      if (strcmp (name, section_flags[i].name) == 0)
	{
	  if (hassize)
	    return section_flags[i].flags_hassize;
	  else
	    return section_flags[i].flags_always;
	}
      i++;
    }
  if (hassize)
    return section_flags[i].flags_hassize;
  return section_flags[i].flags_always;
}

/* Retrieve VMS section flags by name and size.  */

static flagword
vms_esecflag_by_name (const struct sec_flags_struct *section_flags,
		      const char *name,
		      int hassize)
{
  int i = 0;

  while (section_flags[i].name != NULL)
    {
      if (strcmp (name, section_flags[i].name) == 0)
	{
	  if (hassize)
	    return section_flags[i].vflags_hassize;
	  else
	    return section_flags[i].vflags_always;
	}
      i++;
    }
  if (hassize)
    return section_flags[i].vflags_hassize;
  return section_flags[i].vflags_always;
}

/* Add SYM to the symbol table of ABFD.
   Return FALSE in case of error.  */

static bool
add_symbol_entry (bfd *abfd, struct vms_symbol_entry *sym)
{
  if (PRIV (gsd_sym_count) >= PRIV (max_sym_count))
    {
      if (PRIV (max_sym_count) == 0)
	{
	  PRIV (max_sym_count) = 128;
	  PRIV (syms) = bfd_malloc
	    (PRIV (max_sym_count) * sizeof (struct vms_symbol_entry *));
	}
      else
	{
	  PRIV (max_sym_count) *= 2;
	  PRIV (syms) = bfd_realloc_or_free
	    (PRIV (syms),
	     (PRIV (max_sym_count) * sizeof (struct vms_symbol_entry *)));
	}
      if (PRIV (syms) == NULL)
	return false;
    }

  PRIV (syms)[PRIV (gsd_sym_count)++] = sym;
  return true;
}

/* Create a symbol whose name is ASCIC and add it to ABFD.
   Return NULL in case of error.  */

static struct vms_symbol_entry *
add_symbol (bfd *abfd, const unsigned char *ascic, unsigned int max)
{
  struct vms_symbol_entry *entry;
  unsigned int len;

  len = *ascic++;
  max -= 1;
  if (len > max)
    {
      _bfd_error_handler (_("record is too small for symbol name length"));
      bfd_set_error (bfd_error_bad_value);
      return NULL;
    }

  entry = (struct vms_symbol_entry *)bfd_zalloc (abfd, sizeof (*entry) + len);
  if (entry == NULL)
    return NULL;
  entry->namelen = len;
  memcpy (entry->name, ascic, len);
  entry->name[len] = 0;
  entry->owner = abfd;

  if (!add_symbol_entry (abfd, entry))
    return NULL;
  return entry;
}

/* Read and process EGSD.  Return FALSE on failure.  */

static bool
_bfd_vms_slurp_egsd (bfd *abfd)
{
  int gsd_type;
  unsigned int gsd_size;
  unsigned char *vms_rec;
  bfd_vma base_addr;
  long psindx;

  vms_debug2 ((2, "EGSD\n"));

  if (PRIV (recrd.rec_size) < 8)
    {
      _bfd_error_handler (_("corrupt EGSD record: its size (%#x) is too small"),
			  PRIV (recrd.rec_size));
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  PRIV (recrd.rec) += 8;	/* Skip type, size, align pad.  */
  PRIV (recrd.rec_size) -= 8;

  /* Calculate base address for each section.  */
  base_addr = 0;

  while (PRIV (recrd.rec_size) > 4)
    {
      vms_rec = PRIV (recrd.rec);

      gsd_type = bfd_getl16 (vms_rec);
      gsd_size = bfd_getl16 (vms_rec + 2);

      vms_debug2 ((3, "egsd_type %d\n", gsd_type));

      /* PR 21615: Check for size overflow.  */
      if (PRIV (recrd.rec_size) < gsd_size)
	{
	  _bfd_error_handler (_("corrupt EGSD record type %d: size (%#x) "
				"is larger than remaining space (%#x)"),
			      gsd_type, gsd_size, PRIV (recrd.rec_size));
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}

      if (gsd_size < 4)
	{
	too_small:
	  _bfd_error_handler (_("corrupt EGSD record type %d: size (%#x) "
				"is too small"),
			      gsd_type, gsd_size);
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}

      switch (gsd_type)
	{
	case EGSD__C_PSC:
	  /* Program section definition.  */
	  {
	    struct vms_egps *egps = (struct vms_egps *) vms_rec;
	    flagword new_flags, vms_flags;
	    asection *section;

	    if (offsetof (struct vms_egps, flags) + 2 > gsd_size)
	      goto too_small;
	    vms_flags = bfd_getl16 (egps->flags);

	    if ((vms_flags & EGPS__V_REL) == 0)
	      {
		/* Use the global absolute section for all
		   absolute sections.  */
		section = bfd_abs_section_ptr;
	      }
	    else
	      {
		char *name;
		bfd_vma align_addr;
		size_t left;

		if (offsetof (struct vms_egps, namlng) >= gsd_size)
		  goto too_small;
		left = gsd_size - offsetof (struct vms_egps, namlng);
		name = _bfd_vms_save_counted_string (abfd, &egps->namlng, left);
		if (name == NULL || name[0] == 0)
		  return false;

		section = bfd_make_section (abfd, name);
		if (!section)
		  return false;

		section->filepos = 0;
		section->size = bfd_getl32 (egps->alloc);
		section->alignment_power = egps->align & 31;

		vms_section_data (section)->flags = vms_flags;
		vms_section_data (section)->no_flags = 0;

		new_flags = vms_secflag_by_name (evax_section_flags,
						 section->name,
						 section->size > 0);
		if (section->size > 0)
		  new_flags |= SEC_LOAD;
		if (!(vms_flags & EGPS__V_NOMOD) && section->size > 0)
		  {
		    /* Set RELOC and HAS_CONTENTS if the section is not
		       demand-zero and not empty.  */
		    new_flags |= SEC_HAS_CONTENTS;
		    if (vms_flags & EGPS__V_REL)
		      new_flags |= SEC_RELOC;
		  }
		if (vms_flags & EGPS__V_EXE)
		  {
		    /* Set CODE if section is executable.  */
		    new_flags |= SEC_CODE;
		    new_flags &= ~SEC_DATA;
		  }
		if (!bfd_set_section_flags (section, new_flags))
		  return false;

		/* Give a non-overlapping vma to non absolute sections.  */
		align_addr = (bfd_vma) 1 << section->alignment_power;
		base_addr = (base_addr + align_addr - 1) & -align_addr;
		section->vma = base_addr;
		base_addr += section->size;
	      }

	    /* Append it to the section array.  */
	    if (PRIV (section_count) >= PRIV (section_max))
	      {
		if (PRIV (section_max) == 0)
		  PRIV (section_max) = 16;
		else
		  PRIV (section_max) *= 2;
		PRIV (sections) = bfd_realloc_or_free
		  (PRIV (sections), PRIV (section_max) * sizeof (asection *));
		if (PRIV (sections) == NULL)
		  return false;
	      }

	    PRIV (sections)[PRIV (section_count)] = section;
	    PRIV (section_count)++;
	  }
	  break;

	case EGSD__C_SYM:
	  {
	    unsigned int nameoff;
	    struct vms_symbol_entry *entry;
	    struct vms_egsy *egsy = (struct vms_egsy *) vms_rec;
	    flagword old_flags;

	    if (offsetof (struct vms_egsy, flags) + 2 > gsd_size)
	      goto too_small;
	    old_flags = bfd_getl16 (egsy->flags);
	    if (old_flags & EGSY__V_DEF)
	      nameoff = ESDF__B_NAMLNG;
	    else
	      nameoff = ESRF__B_NAMLNG;

	    if (nameoff >= gsd_size)
	      goto too_small;
	    entry = add_symbol (abfd, vms_rec + nameoff, gsd_size - nameoff);
	    if (entry == NULL)
	      return false;

	    /* Allow only duplicate reference.  */
	    if ((entry->flags & EGSY__V_DEF) && (old_flags & EGSY__V_DEF))
	      abort ();

	    if (entry->typ == 0)
	      {
		entry->typ = gsd_type;
		entry->data_type = egsy->datyp;
		entry->flags = old_flags;
	      }

	    if (old_flags & EGSY__V_DEF)
	      {
		struct vms_esdf *esdf = (struct vms_esdf *) vms_rec;

		entry->value = bfd_getl64 (esdf->value);
		if (PRIV (sections) == NULL)
		  return false;

		psindx = bfd_getl32 (esdf->psindx);
		/* PR 21813: Check for an out of range index.  */
		if (psindx < 0 || psindx >= (int) PRIV (section_count))
		  {
		  bad_psindx:
		    _bfd_error_handler (_("corrupt EGSD record: its psindx "
					  "field is too big (%#lx)"),
					psindx);
		    bfd_set_error (bfd_error_bad_value);
		    return false;
		  }
		entry->section = PRIV (sections)[psindx];

		if (old_flags & EGSY__V_NORM)
		  {
		    PRIV (norm_sym_count)++;

		    entry->code_value = bfd_getl64 (esdf->code_address);
		    psindx = bfd_getl32 (esdf->ca_psindx);
		    /* PR 21813: Check for an out of range index.  */
		    if (psindx < 0 || psindx >= (int) PRIV (section_count))
		      goto bad_psindx;
		    entry->code_section = PRIV (sections)[psindx];
		  }
	      }
	  }
	  break;

	case EGSD__C_SYMG:
	  {
	    struct vms_symbol_entry *entry;
	    struct vms_egst *egst = (struct vms_egst *)vms_rec;
	    flagword old_flags;
	    unsigned int nameoff = offsetof (struct vms_egst, namlng);

	    if (nameoff >= gsd_size)
	      goto too_small;
	    entry = add_symbol (abfd, &egst->namlng, gsd_size - nameoff);
	    if (entry == NULL)
	      return false;

	    old_flags = bfd_getl16 (egst->header.flags);
	    entry->typ = gsd_type;
	    entry->data_type = egst->header.datyp;
	    entry->flags = old_flags;

	    entry->symbol_vector = bfd_getl32 (egst->value);

	    if (old_flags & EGSY__V_REL)
	      {
		if (PRIV (sections) == NULL)
		  return false;
		psindx = bfd_getl32 (egst->psindx);
		/* PR 21813: Check for an out of range index.  */
		if (psindx < 0 || psindx >= (int) PRIV (section_count))
		  goto bad_psindx;
		entry->section = PRIV (sections)[psindx];
	      }
	    else
	      entry->section = bfd_abs_section_ptr;

	    entry->value = bfd_getl64 (egst->lp_2);

	    if (old_flags & EGSY__V_NORM)
	      {
		PRIV (norm_sym_count)++;

		entry->code_value = bfd_getl64 (egst->lp_1);
		entry->code_section = bfd_abs_section_ptr;
	      }
	  }
	  break;

	case EGSD__C_SPSC:
	case EGSD__C_IDC:
	  /* Currently ignored.  */
	  break;
	case EGSD__C_SYMM:
	case EGSD__C_SYMV:
	default:
	  _bfd_error_handler (_("unknown EGSD subtype %d"), gsd_type);
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}

      PRIV (recrd.rec_size) -= gsd_size;
      PRIV (recrd.rec) += gsd_size;
    }

  /* FIXME: Should we complain if PRIV (recrd.rec_size) is not zero ?  */

  if (PRIV (gsd_sym_count) > 0)
    abfd->flags |= HAS_SYMS;

  return true;
}

/* Stack routines for vms ETIR commands.  */

/* Push value and section index.  */

static bool
_bfd_vms_push (bfd *abfd, bfd_vma val, unsigned int reloc)
{
  vms_debug2 ((4, "<push %08lx (0x%08x) at %d>\n",
	       (unsigned long)val, reloc, PRIV (stackptr)));

  PRIV (stack[PRIV (stackptr)]).value = val;
  PRIV (stack[PRIV (stackptr)]).reloc = reloc;
  PRIV (stackptr)++;
  if (PRIV (stackptr) >= STACKSIZE)
    {
      bfd_set_error (bfd_error_bad_value);
      _bfd_error_handler (_("stack overflow (%d) in _bfd_vms_push"), PRIV (stackptr));
      return false;
    }
  return true;
}

/* Pop value and section index.  */

static bool
_bfd_vms_pop (bfd *abfd, bfd_vma *val, unsigned int *rel)
{
  if (PRIV (stackptr) == 0)
    {
      bfd_set_error (bfd_error_bad_value);
      _bfd_error_handler (_("stack underflow in _bfd_vms_pop"));
      return false;
    }
  PRIV (stackptr)--;
  *val = PRIV (stack[PRIV (stackptr)]).value;
  *rel = PRIV (stack[PRIV (stackptr)]).reloc;

  vms_debug2 ((4, "<pop %08lx (0x%08x)>\n", (unsigned long)*val, *rel));
  return true;
}

/* Routines to fill sections contents during tir/etir read.  */

/* Initialize image buffer pointer to be filled.  */

static void
image_set_ptr (bfd *abfd, bfd_vma vma, int sect, struct bfd_link_info *info)
{
  asection *sec;

  vms_debug2 ((4, "image_set_ptr (0x%08x, sect=%d)\n", (unsigned)vma, sect));

  if (PRIV (sections) == NULL)
    return;
  if (sect < 0 || sect >= (int) PRIV (section_count))
    return;

  sec = PRIV (sections)[sect];

  if (info)
    {
      /* Reading contents to an output bfd.  */

      if (sec->output_section == NULL)
	{
	  /* Section discarded.  */
	  vms_debug2 ((5, " section %s discarded\n", sec->name));

	  /* This is not used.  */
	  PRIV (image_section) = NULL;
	  PRIV (image_offset) = 0;
	  return;
	}
      PRIV (image_offset) = sec->output_offset + vma;
      PRIV (image_section) = sec->output_section;
    }
  else
    {
      PRIV (image_offset) = vma;
      PRIV (image_section) = sec;
    }
}

/* Increment image buffer pointer by offset.  */

static void
image_inc_ptr (bfd *abfd, bfd_vma offset)
{
  vms_debug2 ((4, "image_inc_ptr (%u)\n", (unsigned)offset));

  PRIV (image_offset) += offset;
}

/* Save current DST location counter under specified index.  */

static bool
dst_define_location (bfd *abfd, unsigned int loc)
{
  vms_debug2 ((4, "dst_define_location (%d)\n", (int)loc));

  if (loc > 1 << 24)
    {
      /* 16M entries ought to be plenty.  */
      bfd_set_error (bfd_error_bad_value);
      _bfd_error_handler (_("dst_define_location %u too large"), loc);
      return false;
    }

  /* Grow the ptr offset table if necessary.  */
  if (loc + 1 > PRIV (dst_ptr_offsets_count))
    {
      PRIV (dst_ptr_offsets)
	= bfd_realloc_or_free (PRIV (dst_ptr_offsets),
			       (loc + 1) * sizeof (unsigned int));
      if (PRIV (dst_ptr_offsets) == NULL)
	return false;
      memset (PRIV (dst_ptr_offsets) + PRIV (dst_ptr_offsets_count), 0,
	      (loc - PRIV (dst_ptr_offsets_count)) * sizeof (unsigned int));
      PRIV (dst_ptr_offsets_count) = loc + 1;
    }

  PRIV (dst_ptr_offsets)[loc] = PRIV (image_offset);
  return true;
}

/* Restore saved DST location counter from specified index.  */

static bool
dst_restore_location (bfd *abfd, unsigned int loc)
{
  vms_debug2 ((4, "dst_restore_location (%d)\n", (int)loc));

  if (loc < PRIV (dst_ptr_offsets_count))
    {
      PRIV (image_offset) = PRIV (dst_ptr_offsets)[loc];
      return true;
    }
  return false;
}

/* Retrieve saved DST location counter from specified index.  */

static bool
dst_retrieve_location (bfd *abfd, bfd_vma *loc)
{
  vms_debug2 ((4, "dst_retrieve_location (%d)\n", (int) *loc));

  if (*loc < PRIV (dst_ptr_offsets_count))
    {
      *loc = PRIV (dst_ptr_offsets)[*loc];
      return true;
    }
  return false;
}

/* Write multiple bytes to section image.  */

static bool
image_write (bfd *abfd, unsigned char *ptr, unsigned int size)
{
  asection *sec = PRIV (image_section);
  size_t off = PRIV (image_offset);

  /* Check bounds.  */
  if (off > sec->size
      || size > sec->size - off)
    {
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

#if VMS_DEBUG
  _bfd_vms_debug (8, "image_write from (%p, %d) to (%ld)\n", ptr, size,
		  (long) off));
#endif

  if (PRIV (image_section)->contents != NULL)
    memcpy (sec->contents + off, ptr, size);
  else
    {
      unsigned int i;
      for (i = 0; i < size; i++)
	if (ptr[i] != 0)
	  {
	    bfd_set_error (bfd_error_bad_value);
	    return false;
	  }
    }

#if VMS_DEBUG
  _bfd_hexdump (9, ptr, size, 0);
#endif

  PRIV (image_offset) += size;
  return true;
}

/* Write byte to section image.  */

static bool
image_write_b (bfd * abfd, unsigned int value)
{
  unsigned char data[1];

  vms_debug2 ((6, "image_write_b (%02x)\n", (int) value));

  *data = value;

  return image_write (abfd, data, sizeof (data));
}

/* Write 2-byte word to image.  */

static bool
image_write_w (bfd * abfd, unsigned int value)
{
  unsigned char data[2];

  vms_debug2 ((6, "image_write_w (%04x)\n", (int) value));

  bfd_putl16 (value, data);
  return image_write (abfd, data, sizeof (data));
}

/* Write 4-byte long to image.  */

static bool
image_write_l (bfd * abfd, unsigned long value)
{
  unsigned char data[4];

  vms_debug2 ((6, "image_write_l (%08lx)\n", value));

  bfd_putl32 (value, data);
  return image_write (abfd, data, sizeof (data));
}

/* Write 8-byte quad to image.  */

static bool
image_write_q (bfd * abfd, bfd_vma value)
{
  unsigned char data[8];

  vms_debug2 ((6, "image_write_q (%08lx)\n", (unsigned long)value));

  bfd_putl64 (value, data);
  return image_write (abfd, data, sizeof (data));
}

static const char *
_bfd_vms_etir_name (int cmd)
{
  switch (cmd)
    {
    case ETIR__C_STA_GBL: return "ETIR__C_STA_GBL";
    case ETIR__C_STA_LW: return "ETIR__C_STA_LW";
    case ETIR__C_STA_QW: return "ETIR__C_STA_QW";
    case ETIR__C_STA_PQ: return "ETIR__C_STA_PQ";
    case ETIR__C_STA_LI: return "ETIR__C_STA_LI";
    case ETIR__C_STA_MOD: return "ETIR__C_STA_MOD";
    case ETIR__C_STA_CKARG: return "ETIR__C_STA_CKARG";
    case ETIR__C_STO_B: return "ETIR__C_STO_B";
    case ETIR__C_STO_W: return "ETIR__C_STO_W";
    case ETIR__C_STO_GBL: return "ETIR__C_STO_GBL";
    case ETIR__C_STO_CA: return "ETIR__C_STO_CA";
    case ETIR__C_STO_RB: return "ETIR__C_STO_RB";
    case ETIR__C_STO_AB: return "ETIR__C_STO_AB";
    case ETIR__C_STO_OFF: return "ETIR__C_STO_OFF";
    case ETIR__C_STO_IMM: return "ETIR__C_STO_IMM";
    case ETIR__C_STO_IMMR: return "ETIR__C_STO_IMMR";
    case ETIR__C_STO_LW: return "ETIR__C_STO_LW";
    case ETIR__C_STO_QW: return "ETIR__C_STO_QW";
    case ETIR__C_STO_GBL_LW: return "ETIR__C_STO_GBL_LW";
    case ETIR__C_STO_LP_PSB: return "ETIR__C_STO_LP_PSB";
    case ETIR__C_STO_HINT_GBL: return "ETIR__C_STO_HINT_GBL";
    case ETIR__C_STO_HINT_PS: return "ETIR__C_STO_HINT_PS";
    case ETIR__C_OPR_ADD: return "ETIR__C_OPR_ADD";
    case ETIR__C_OPR_SUB: return "ETIR__C_OPR_SUB";
    case ETIR__C_OPR_INSV: return "ETIR__C_OPR_INSV";
    case ETIR__C_OPR_USH: return "ETIR__C_OPR_USH";
    case ETIR__C_OPR_ROT: return "ETIR__C_OPR_ROT";
    case ETIR__C_OPR_REDEF: return "ETIR__C_OPR_REDEF";
    case ETIR__C_OPR_DFLIT: return "ETIR__C_OPR_DFLIT";
    case ETIR__C_STC_LP: return "ETIR__C_STC_LP";
    case ETIR__C_STC_GBL: return "ETIR__C_STC_GBL";
    case ETIR__C_STC_GCA: return "ETIR__C_STC_GCA";
    case ETIR__C_STC_PS: return "ETIR__C_STC_PS";
    case ETIR__C_STC_NBH_PS: return "ETIR__C_STC_NBH_PS";
    case ETIR__C_STC_NOP_GBL: return "ETIR__C_STC_NOP_GBL";
    case ETIR__C_STC_NOP_PS: return "ETIR__C_STC_NOP_PS";
    case ETIR__C_STC_BSR_GBL: return "ETIR__C_STC_BSR_GBL";
    case ETIR__C_STC_BSR_PS: return "ETIR__C_STC_BSR_PS";
    case ETIR__C_STC_LDA_GBL: return "ETIR__C_STC_LDA_GBL";
    case ETIR__C_STC_LDA_PS: return "ETIR__C_STC_LDA_PS";
    case ETIR__C_STC_BOH_GBL: return "ETIR__C_STC_BOH_GBL";
    case ETIR__C_STC_BOH_PS: return "ETIR__C_STC_BOH_PS";
    case ETIR__C_STC_NBH_GBL: return "ETIR__C_STC_NBH_GBL";
    case ETIR__C_STC_LP_PSB: return "ETIR__C_STC_LP_PSB";
    case ETIR__C_CTL_SETRB: return "ETIR__C_CTL_SETRB";
    case ETIR__C_CTL_AUGRB: return "ETIR__C_CTL_AUGRB";
    case ETIR__C_CTL_DFLOC: return "ETIR__C_CTL_DFLOC";
    case ETIR__C_CTL_STLOC: return "ETIR__C_CTL_STLOC";
    case ETIR__C_CTL_STKDL: return "ETIR__C_CTL_STKDL";

    default:
      /* These names have not yet been added to this switch statement.  */
      _bfd_error_handler (_("unknown ETIR command %d"), cmd);
    }

  return NULL;
}
#define HIGHBIT(op) ((op & 0x80000000L) == 0x80000000L)

static void
_bfd_vms_get_value (bfd *abfd,
		    const unsigned char *ascic,
		    const unsigned char *max_ascic,
		    struct bfd_link_info *info,
		    bfd_vma *vma,
		    struct alpha_vms_link_hash_entry **hp)
{
  char name[257];
  unsigned int len;
  unsigned int i;
  struct alpha_vms_link_hash_entry *h;

  /* Not linking.  Do not try to resolve the symbol.  */
  if (info == NULL)
    {
      *vma = 0;
      *hp = NULL;
      return;
    }

  len = *ascic;
  if (ascic + len >= max_ascic)
    {
      _bfd_error_handler (_("corrupt vms value"));
      *vma = 0;
      *hp = NULL;
      return;
    }

  for (i = 0; i < len; i++)
    name[i] = ascic[i + 1];
  name[i] = 0;

  h = (struct alpha_vms_link_hash_entry *)
    bfd_link_hash_lookup (info->hash, name, false, false, true);

  *hp = h;

  if (h != NULL
      && (h->root.type == bfd_link_hash_defined
	  || h->root.type == bfd_link_hash_defweak))
    *vma = h->root.u.def.value
      + h->root.u.def.section->output_offset
      + h->root.u.def.section->output_section->vma;
  else if (h && h->root.type == bfd_link_hash_undefweak)
    *vma = 0;
  else
    {
      (*info->callbacks->undefined_symbol)
	(info, name, abfd, PRIV (image_section), PRIV (image_offset), true);
      *vma = 0;
    }
}

#define RELC_NONE 0
#define RELC_REL  1
#define RELC_SHR_BASE 0x10000
#define RELC_SEC_BASE 0x20000
#define RELC_MASK     0x0ffff

static unsigned int
alpha_vms_sym_to_ctxt (struct alpha_vms_link_hash_entry *h)
{
  /* Handle undefined symbols.  */
  if (h == NULL || h->sym == NULL)
    return RELC_NONE;

  if (h->sym->typ == EGSD__C_SYMG)
    {
      if (h->sym->flags & EGSY__V_REL)
	return RELC_SHR_BASE + PRIV2 (h->sym->owner, shr_index);
      else
	{
	  /* Can this happen (non-relocatable symg) ?  I'd like to see
	     an example.  */
	  abort ();
	}
    }
  if (h->sym->typ == EGSD__C_SYM)
    {
      if (h->sym->flags & EGSY__V_REL)
	return RELC_REL;
      else
	return RELC_NONE;
    }
  abort ();
}

static bfd_vma
alpha_vms_get_sym_value (asection *sect, bfd_vma addr)
{
  return sect->output_section->vma + sect->output_offset + addr;
}

static bfd_vma
alpha_vms_fix_sec_rel (bfd *abfd, struct bfd_link_info *info,
		       unsigned int rel, bfd_vma vma)
{
  asection *sec;

  if (PRIV (sections) == NULL)
    return 0;

  sec = PRIV (sections)[rel & RELC_MASK];

  if (info)
    {
      if (sec->output_section == NULL)
	abort ();
      return vma + sec->output_section->vma + sec->output_offset;
    }
  else
    return vma + sec->vma;
}

/* Read an ETIR record from ABFD.  If INFO is not null, put the content into
   the output section (used during linking).
   Return FALSE in case of error.  */

static bool
_bfd_vms_slurp_etir (bfd *abfd, struct bfd_link_info *info)
{
  unsigned char *ptr;
  unsigned int length;
  unsigned char *maxptr;
  bfd_vma op1 = 0;
  bfd_vma op2 = 0;
  unsigned int rel1 = RELC_NONE;
  unsigned int rel2 = RELC_NONE;
  struct alpha_vms_link_hash_entry *h;

  PRIV (recrd.rec) += ETIR__C_HEADER_SIZE;
  PRIV (recrd.rec_size) -= ETIR__C_HEADER_SIZE;

  ptr = PRIV (recrd.rec);
  length = PRIV (recrd.rec_size);
  maxptr = ptr + length;

  vms_debug2 ((2, "ETIR: %d bytes\n", length));

  while (ptr < maxptr)
    {
      int cmd, cmd_length;

      if (ptr + 4 > maxptr)
	goto corrupt_etir;

      cmd = bfd_getl16 (ptr);
      cmd_length = bfd_getl16 (ptr + 2);

      /* PR 21589 and 21579: Check for a corrupt ETIR record.  */
      if (cmd_length < 4 || ptr + cmd_length > maxptr)
	{
	corrupt_etir:
	  _bfd_error_handler (_("corrupt ETIR record encountered"));
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}
      ptr += 4;
      cmd_length -= 4;

#if VMS_DEBUG
      _bfd_vms_debug (4, "etir: %s(%d)\n",
		      _bfd_vms_etir_name (cmd), cmd);
      _bfd_hexdump (8, ptr, cmd_length, 0);
#endif

      switch (cmd)
	{
	  /* Stack global
	     arg: cs	symbol name

	     stack 32 bit value of symbol (high bits set to 0).  */
	case ETIR__C_STA_GBL:
	  _bfd_vms_get_value (abfd, ptr, ptr + cmd_length, info, &op1, &h);
	  if (!_bfd_vms_push (abfd, op1, alpha_vms_sym_to_ctxt (h)))
	    return false;
	  break;

	  /* Stack longword
	     arg: lw	value

	     stack 32 bit value, sign extend to 64 bit.  */
	case ETIR__C_STA_LW:
	  if (cmd_length < 4)
	    goto corrupt_etir;
	  if (!_bfd_vms_push (abfd, bfd_getl32 (ptr), RELC_NONE))
	    return false;
	  break;

	  /* Stack quadword
	     arg: qw	value

	     stack 64 bit value of symbol.  */
	case ETIR__C_STA_QW:
	  if (cmd_length < 8)
	    goto corrupt_etir;
	  if (!_bfd_vms_push (abfd, bfd_getl64 (ptr), RELC_NONE))
	    return false;
	  break;

	  /* Stack psect base plus quadword offset
	     arg: lw	section index
	     qw	signed quadword offset (low 32 bits)

	     Stack qw argument and section index
	     (see ETIR__C_STO_OFF, ETIR__C_CTL_SETRB).  */
	case ETIR__C_STA_PQ:
	  {
	    int psect;

	    if (cmd_length < 12)
	      goto corrupt_etir;
	    psect = bfd_getl32 (ptr);
	    if ((unsigned int) psect >= PRIV (section_count))
	      {
		_bfd_error_handler (_("bad section index in %s"),
				    _bfd_vms_etir_name (cmd));
		bfd_set_error (bfd_error_bad_value);
		return false;
	      }
	    op1 = bfd_getl64 (ptr + 4);
	    if (!_bfd_vms_push (abfd, op1, psect | RELC_SEC_BASE))
	      return false;
	  }
	  break;

	case ETIR__C_STA_LI:
	case ETIR__C_STA_MOD:
	case ETIR__C_STA_CKARG:
	  _bfd_error_handler (_("unsupported STA cmd %s"),
			      _bfd_vms_etir_name (cmd));
	  return false;
	  break;

	  /* Store byte: pop stack, write byte
	     arg: -.  */
	case ETIR__C_STO_B:
	  if (!_bfd_vms_pop (abfd, &op1, &rel1))
	    return false;
	  if (rel1 != RELC_NONE)
	    goto bad_context;
	  if (!image_write_b (abfd, (unsigned int) op1 & 0xff))
	    return false;
	  break;

	  /* Store word: pop stack, write word
	     arg: -.  */
	case ETIR__C_STO_W:
	  if (!_bfd_vms_pop (abfd, &op1, &rel1))
	    return false;
	  if (rel1 != RELC_NONE)
	    goto bad_context;
	  if (!image_write_w (abfd, (unsigned int) op1 & 0xffff))
	    return false;
	  break;

	  /* Store longword: pop stack, write longword
	     arg: -.  */
	case ETIR__C_STO_LW:
	  if (!_bfd_vms_pop (abfd, &op1, &rel1))
	    return false;
	  if (rel1 & RELC_SEC_BASE)
	    {
	      op1 = alpha_vms_fix_sec_rel (abfd, info, rel1, op1);
	      rel1 = RELC_REL;
	    }
	  else if (rel1 & RELC_SHR_BASE)
	    {
	      if (!alpha_vms_add_fixup_lr (info, rel1 & RELC_MASK, op1))
		return false;
	      rel1 = RELC_NONE;
	    }
	  if (rel1 != RELC_NONE)
	    {
	      if (rel1 != RELC_REL)
		abort ();
	      if (!alpha_vms_add_lw_reloc (info))
		return false;
	    }
	  if (!image_write_l (abfd, op1))
	    return false;
	  break;

	  /* Store quadword: pop stack, write quadword
	     arg: -.  */
	case ETIR__C_STO_QW:
	  if (!_bfd_vms_pop (abfd, &op1, &rel1))
	    return false;
	  if (rel1 & RELC_SEC_BASE)
	    {
	      op1 = alpha_vms_fix_sec_rel (abfd, info, rel1, op1);
	      rel1 = RELC_REL;
	    }
	  else if (rel1 & RELC_SHR_BASE)
	    abort ();
	  if (rel1 != RELC_NONE)
	    {
	      if (rel1 != RELC_REL)
		abort ();
	      if (!alpha_vms_add_qw_reloc (info))
		return false;
	    }
	  if (!image_write_q (abfd, op1))
	    return false;
	  break;

	  /* Store immediate repeated: pop stack for repeat count
	     arg: lw	byte count
	     da	data.  */
	case ETIR__C_STO_IMMR:
	  {
	    int size;

	    if (cmd_length < 4)
	      goto corrupt_etir;
	    size = bfd_getl32 (ptr);
	    if (size > cmd_length - 4)
	      goto corrupt_etir;
	    if (!_bfd_vms_pop (abfd, &op1, &rel1))
	      return false;
	    if (rel1 != RELC_NONE)
	      goto bad_context;
	    if (size == 0)
	      break;
	    op1 &= 0xffffffff;
	    while (op1-- > 0)
	      if (!image_write (abfd, ptr + 4, size))
		return false;
	  }
	  break;

	  /* Store global: write symbol value
	     arg: cs	global symbol name.  */
	case ETIR__C_STO_GBL:
	  _bfd_vms_get_value (abfd, ptr, ptr + cmd_length, info, &op1, &h);
	  if (h && h->sym)
	    {
	      if (h->sym->typ == EGSD__C_SYMG)
		{
		  if (!alpha_vms_add_fixup_qr (info, abfd, h->sym->owner,
					       h->sym->symbol_vector))
		    return false;
		  op1 = 0;
		}
	      else
		{
		  op1 = alpha_vms_get_sym_value (h->sym->section,
						 h->sym->value);
		  if (!alpha_vms_add_qw_reloc (info))
		    return false;
		}
	    }
	  if (!image_write_q (abfd, op1))
	    return false;
	  break;

	  /* Store code address: write address of entry point
	     arg: cs	global symbol name (procedure).  */
	case ETIR__C_STO_CA:
	  _bfd_vms_get_value (abfd, ptr, ptr + cmd_length, info, &op1, &h);
	  if (h && h->sym)
	    {
	      if (h->sym->flags & EGSY__V_NORM)
		{
		  /* That's really a procedure.  */
		  if (h->sym->typ == EGSD__C_SYMG)
		    {
		      if (!alpha_vms_add_fixup_ca (info, abfd, h->sym->owner))
			return false;
		      op1 = h->sym->symbol_vector;
		    }
		  else
		    {
		      op1 = alpha_vms_get_sym_value (h->sym->code_section,
						     h->sym->code_value);
		      if (!alpha_vms_add_qw_reloc (info))
			return false;
		    }
		}
	      else
		{
		  /* Symbol is not a procedure.  */
		  abort ();
		}
	    }
	  if (!image_write_q (abfd, op1))
	    return false;
	  break;

	  /* Store offset to psect: pop stack, add low 32 bits to base of psect
	     arg: none.  */
	case ETIR__C_STO_OFF:
	  if (!_bfd_vms_pop (abfd, &op1, &rel1))
	    return false;

	  if (!(rel1 & RELC_SEC_BASE))
	    abort ();

	  op1 = alpha_vms_fix_sec_rel (abfd, info, rel1, op1);
	  rel1 = RELC_REL;
	  if (!image_write_q (abfd, op1))
	    return false;
	  break;

	  /* Store immediate
	     arg: lw	count of bytes
	     da	data.  */
	case ETIR__C_STO_IMM:
	  {
	    unsigned int size;

	    if (cmd_length < 4)
	      goto corrupt_etir;
	    size = bfd_getl32 (ptr);
	    if (!image_write (abfd, ptr + 4, size))
	      return false;
	  }
	  break;

	  /* This code is 'reserved to digital' according to the openVMS
	     linker manual, however it is generated by the DEC C compiler
	     and defined in the include file.
	     FIXME, since the following is just a guess
	     store global longword: store 32bit value of symbol
	     arg: cs	symbol name.  */
	case ETIR__C_STO_GBL_LW:
	  _bfd_vms_get_value (abfd, ptr, ptr + cmd_length, info, &op1, &h);
#if 0
	  abort ();
#endif
	  if (!image_write_l (abfd, op1))
	    return false;
	  break;

	case ETIR__C_STO_RB:
	case ETIR__C_STO_AB:
	case ETIR__C_STO_LP_PSB:
	  _bfd_error_handler (_("%s: not supported"),
			      _bfd_vms_etir_name (cmd));
	  return false;
	  break;
	case ETIR__C_STO_HINT_GBL:
	case ETIR__C_STO_HINT_PS:
	  _bfd_error_handler (_("%s: not implemented"),
			      _bfd_vms_etir_name (cmd));
	  return false;
	  break;

	  /* 200 Store-conditional Linkage Pair
	     arg: none.  */
	case ETIR__C_STC_LP:

	  /* 202 Store-conditional Address at global address
	     lw	linkage index
	     cs	global name.  */

	case ETIR__C_STC_GBL:

	  /* 203 Store-conditional Code Address at global address
	     lw	linkage index
	     cs	procedure name.  */
	case ETIR__C_STC_GCA:

	  /* 204 Store-conditional Address at psect + offset
	     lw	linkage index
	     lw	psect index
	     qw	offset.  */
	case ETIR__C_STC_PS:
	  _bfd_error_handler (_("%s: not supported"),
			      _bfd_vms_etir_name (cmd));
	  return false;
	  break;

	  /* 201 Store-conditional Linkage Pair with Procedure Signature
	     lw	linkage index
	     cs	procedure name
	     by	signature length
	     da	signature.  */

	case ETIR__C_STC_LP_PSB:
	  if (cmd_length < 4)
	    goto corrupt_etir;
	  _bfd_vms_get_value (abfd, ptr + 4, ptr + cmd_length, info, &op1, &h);
	  if (h && h->sym)
	    {
	      if (h->sym->typ == EGSD__C_SYMG)
		{
		  if (!alpha_vms_add_fixup_lp (info, abfd, h->sym->owner))
		    return false;
		  op1 = h->sym->symbol_vector;
		  op2 = 0;
		}
	      else
		{
		  op1 = alpha_vms_get_sym_value (h->sym->code_section,
						 h->sym->code_value);
		  op2 = alpha_vms_get_sym_value (h->sym->section,
						h->sym->value);
		}
	    }
	  else
	    {
	      /* Undefined symbol.  */
	      op1 = 0;
	      op2 = 0;
	    }
	  if (!image_write_q (abfd, op1)
	      || !image_write_q (abfd, op2))
	    return false;
	  break;

	  /* 205 Store-conditional NOP at address of global
	     arg: none.  */
	case ETIR__C_STC_NOP_GBL:
	  /* ALPHA_R_NOP */

	  /* 207 Store-conditional BSR at global address
	     arg: none.  */

	case ETIR__C_STC_BSR_GBL:
	  /* ALPHA_R_BSR */

	  /* 209 Store-conditional LDA at global address
	     arg: none.  */

	case ETIR__C_STC_LDA_GBL:
	  /* ALPHA_R_LDA */

	  /* 211 Store-conditional BSR or Hint at global address
	     arg: none.  */

	case ETIR__C_STC_BOH_GBL:
	  /* Currentl ignored.  */
	  break;

	  /* 213 Store-conditional NOP,BSR or HINT at global address
	     arg: none.  */

	case ETIR__C_STC_NBH_GBL:

	  /* 206 Store-conditional NOP at pect + offset
	     arg: none.  */

	case ETIR__C_STC_NOP_PS:

	  /* 208 Store-conditional BSR at pect + offset
	     arg: none.  */

	case ETIR__C_STC_BSR_PS:

	  /* 210 Store-conditional LDA at psect + offset
	     arg: none.  */

	case ETIR__C_STC_LDA_PS:

	  /* 212 Store-conditional BSR or Hint at pect + offset
	     arg: none.  */

	case ETIR__C_STC_BOH_PS:

	  /* 214 Store-conditional NOP, BSR or HINT at psect + offset
	     arg: none.  */
	case ETIR__C_STC_NBH_PS:
	  _bfd_error_handler (_("%s: not supported"),
			      _bfd_vms_etir_name (cmd));
	  return false;
	  break;

	  /* Det relocation base: pop stack, set image location counter
	     arg: none.  */
	case ETIR__C_CTL_SETRB:
	  if (!_bfd_vms_pop (abfd, &op1, &rel1))
	    return false;
	  if (!(rel1 & RELC_SEC_BASE))
	    abort ();
	  image_set_ptr (abfd, op1, rel1 & RELC_MASK, info);
	  break;

	  /* Augment relocation base: increment image location counter by offset
	     arg: lw	offset value.  */
	case ETIR__C_CTL_AUGRB:
	  if (cmd_length < 4)
	    goto corrupt_etir;
	  op1 = bfd_getl32 (ptr);
	  image_inc_ptr (abfd, op1);
	  break;

	  /* Define location: pop index, save location counter under index
	     arg: none.  */
	case ETIR__C_CTL_DFLOC:
	  if (!_bfd_vms_pop (abfd, &op1, &rel1))
	    return false;
	  if (rel1 != RELC_NONE)
	    goto bad_context;
	  if (!dst_define_location (abfd, op1))
	    return false;
	  break;

	  /* Set location: pop index, restore location counter from index
	     arg: none.  */
	case ETIR__C_CTL_STLOC:
	  if (!_bfd_vms_pop (abfd, &op1, &rel1))
	    return false;
	  if (rel1 != RELC_NONE)
	    goto bad_context;
	  if (!dst_restore_location (abfd, op1))
	    {
	      bfd_set_error (bfd_error_bad_value);
	      _bfd_error_handler (_("invalid %s"), "ETIR__C_CTL_STLOC");
	      return false;
	    }
	  break;

	  /* Stack defined location: pop index, push location counter from index
	     arg: none.  */
	case ETIR__C_CTL_STKDL:
	  if (!_bfd_vms_pop (abfd, &op1, &rel1))
	    return false;
	  if (rel1 != RELC_NONE)
	    goto bad_context;
	  if (!dst_retrieve_location (abfd, &op1))
	    {
	      bfd_set_error (bfd_error_bad_value);
	      _bfd_error_handler (_("invalid %s"), "ETIR__C_CTL_STKDL");
	      return false;
	    }
	  if (!_bfd_vms_push (abfd, op1, RELC_NONE))
	    return false;
	  break;

	case ETIR__C_OPR_NOP:      /* No-op.  */
	  break;

	case ETIR__C_OPR_ADD:      /* Add.  */
	  if (!_bfd_vms_pop (abfd, &op1, &rel1)
	      || !_bfd_vms_pop (abfd, &op2, &rel2))
	    return false;
	  if (rel1 == RELC_NONE && rel2 != RELC_NONE)
	    rel1 = rel2;
	  else if (rel1 != RELC_NONE && rel2 != RELC_NONE)
	    goto bad_context;
	  if (!_bfd_vms_push (abfd, op1 + op2, rel1))
	    return false;
	  break;

	case ETIR__C_OPR_SUB:      /* Subtract.  */
	  if (!_bfd_vms_pop (abfd, &op1, &rel1)
	      || !_bfd_vms_pop (abfd, &op2, &rel2))
	    return false;
	  if (rel1 == RELC_NONE && rel2 != RELC_NONE)
	    rel1 = rel2;
	  else if ((rel1 & RELC_SEC_BASE) && (rel2 & RELC_SEC_BASE))
	    {
	      op1 = alpha_vms_fix_sec_rel (abfd, info, rel1, op1);
	      op2 = alpha_vms_fix_sec_rel (abfd, info, rel2, op2);
	      rel1 = RELC_NONE;
	    }
	  else if (rel1 != RELC_NONE && rel2 != RELC_NONE)
	    goto bad_context;
	  if (!_bfd_vms_push (abfd, op2 - op1, rel1))
	    return false;
	  break;

	case ETIR__C_OPR_MUL:      /* Multiply.  */
	  if (!_bfd_vms_pop (abfd, &op1, &rel1)
	      || !_bfd_vms_pop (abfd, &op2, &rel2))
	    return false;
	  if (rel1 != RELC_NONE || rel2 != RELC_NONE)
	    goto bad_context;
	  if (!_bfd_vms_push (abfd, op1 * op2, RELC_NONE))
	    return false;
	  break;

	case ETIR__C_OPR_DIV:      /* Divide.  */
	  if (!_bfd_vms_pop (abfd, &op1, &rel1)
	      || !_bfd_vms_pop (abfd, &op2, &rel2))
	    return false;
	  if (rel1 != RELC_NONE || rel2 != RELC_NONE)
	    goto bad_context;
	  if (op1 == 0)
	    {
	      /* Divide by zero is supposed to give a result of zero,
		 and a non-fatal warning message.  */
	      _bfd_error_handler (_("%s divide by zero"), "ETIR__C_OPR_DIV");
	      if (!_bfd_vms_push (abfd, 0, RELC_NONE))
		return false;
	    }
	  else
	    {
	      if (!_bfd_vms_push (abfd, op2 / op1, RELC_NONE))
		return false;
	    }
	  break;

	case ETIR__C_OPR_AND:      /* Logical AND.  */
	  if (!_bfd_vms_pop (abfd, &op1, &rel1)
	      || !_bfd_vms_pop (abfd, &op2, &rel2))
	    return false;
	  if (rel1 != RELC_NONE || rel2 != RELC_NONE)
	    goto bad_context;
	  if (!_bfd_vms_push (abfd, op1 & op2, RELC_NONE))
	    return false;
	  break;

	case ETIR__C_OPR_IOR:      /* Logical inclusive OR.  */
	  if (!_bfd_vms_pop (abfd, &op1, &rel1)
	      || !_bfd_vms_pop (abfd, &op2, &rel2))
	    return false;
	  if (rel1 != RELC_NONE || rel2 != RELC_NONE)
	    goto bad_context;
	  if (!_bfd_vms_push (abfd, op1 | op2, RELC_NONE))
	    return false;
	  break;

	case ETIR__C_OPR_EOR:      /* Logical exclusive OR.  */
	  if (!_bfd_vms_pop (abfd, &op1, &rel1)
	      || !_bfd_vms_pop (abfd, &op2, &rel2))
	    return false;
	  if (rel1 != RELC_NONE || rel2 != RELC_NONE)
	    goto bad_context;
	  if (!_bfd_vms_push (abfd, op1 ^ op2, RELC_NONE))
	    return false;
	  break;

	case ETIR__C_OPR_NEG:      /* Negate.  */
	  if (!_bfd_vms_pop (abfd, &op1, &rel1))
	    return false;
	  if (rel1 != RELC_NONE)
	    goto bad_context;
	  if (!_bfd_vms_push (abfd, -op1, RELC_NONE))
	    return false;
	  break;

	case ETIR__C_OPR_COM:      /* Complement.  */
	  if (!_bfd_vms_pop (abfd, &op1, &rel1))
	    return false;
	  if (rel1 != RELC_NONE)
	    goto bad_context;
	  if (!_bfd_vms_push (abfd, ~op1, RELC_NONE))
	    return false;
	  break;

	case ETIR__C_OPR_ASH:      /* Arithmetic shift.  */
	  if (!_bfd_vms_pop (abfd, &op1, &rel1)
	      || !_bfd_vms_pop (abfd, &op2, &rel2))
	    return false;
	  if (rel1 != RELC_NONE || rel2 != RELC_NONE)
	    {
	    bad_context:
	      _bfd_error_handler (_("invalid use of %s with contexts"),
				  _bfd_vms_etir_name (cmd));
	      return false;
	    }
	  if ((bfd_signed_vma) op2 < 0)
	    {
	      /* Shift right.  */
	      bfd_vma sign;
	      op2 = -op2;
	      if (op2 >= CHAR_BIT * sizeof (op1))
		op2 = CHAR_BIT * sizeof (op1) - 1;
	      /* op1 = (bfd_signed_vma) op1 >> op2; */
	      sign = op1 & ((bfd_vma) 1 << (CHAR_BIT * sizeof (op1) - 1));
	      op1 >>= op2;
	      sign >>= op2;
	      op1 = (op1 ^ sign) - sign;
	    }
	  else
	    {
	      /* Shift left.  */
	      if (op2 >= CHAR_BIT * sizeof (op1))
		op1 = 0;
	      else
		op1 <<= op2;
	    }
	  if (!_bfd_vms_push (abfd, op1, RELC_NONE)) /* FIXME: sym.  */
	    return false;
	  break;

	case ETIR__C_OPR_INSV:      /* Insert field.   */
	case ETIR__C_OPR_USH:       /* Unsigned shift.   */
	case ETIR__C_OPR_ROT:       /* Rotate.  */
	case ETIR__C_OPR_REDEF:     /* Redefine symbol to current location.  */
	case ETIR__C_OPR_DFLIT:     /* Define a literal.  */
	  _bfd_error_handler (_("%s: not supported"),
			      _bfd_vms_etir_name (cmd));
	  return false;
	  break;

	case ETIR__C_OPR_SEL:      /* Select.  */
	  if (!_bfd_vms_pop (abfd, &op1, &rel1))
	    return false;
	  if (op1 & 0x01L)
	    {
	      if (!_bfd_vms_pop (abfd, &op1, &rel1))
		return false;
	    }
	  else
	    {
	      if (!_bfd_vms_pop (abfd, &op1, &rel1)
		  || !_bfd_vms_pop (abfd, &op2, &rel2))
		return false;
	      if (!_bfd_vms_push (abfd, op1, rel1))
		return false;
	    }
	  break;

	default:
	  _bfd_error_handler (_("reserved cmd %d"), cmd);
	  return false;
	  break;
	}

      ptr += cmd_length;
    }

  return true;
}

/* Process EDBG/ETBT record.
   Return TRUE on success, FALSE on error  */

static bool
vms_slurp_debug (bfd *abfd)
{
  asection *section = PRIV (dst_section);

  if (section == NULL)
    {
      /* We have no way to find out beforehand how much debug info there
	 is in an object file, so pick an initial amount and grow it as
	 needed later.  */
      flagword flags = SEC_HAS_CONTENTS | SEC_DEBUGGING | SEC_RELOC
	| SEC_IN_MEMORY;

      section = bfd_make_section (abfd, "$DST$");
      if (!section)
	return false;
      if (!bfd_set_section_flags (section, flags))
	return false;
      PRIV (dst_section) = section;
    }

  PRIV (image_section) = section;
  PRIV (image_offset) = section->size;

  if (!_bfd_vms_slurp_etir (abfd, NULL))
    return false;

  section->size = PRIV (image_offset);
  return true;
}

/* Process EDBG record.
   Return TRUE on success, FALSE on error.  */

static bool
_bfd_vms_slurp_edbg (bfd *abfd)
{
  vms_debug2 ((2, "EDBG\n"));

  abfd->flags |= HAS_DEBUG | HAS_LINENO;

  return vms_slurp_debug (abfd);
}

/* Process ETBT record.
   Return TRUE on success, FALSE on error.  */

static bool
_bfd_vms_slurp_etbt (bfd *abfd)
{
  vms_debug2 ((2, "ETBT\n"));

  abfd->flags |= HAS_LINENO;

  return vms_slurp_debug (abfd);
}

/* Process EEOM record.
   Return TRUE on success, FALSE on error.  */

static bool
_bfd_vms_slurp_eeom (bfd *abfd)
{
  struct vms_eeom *eeom = (struct vms_eeom *) PRIV (recrd.rec);

  vms_debug2 ((2, "EEOM\n"));

  /* PR 21813: Check for an undersized record.  */
  if (PRIV (recrd.buf_size) < sizeof (* eeom))
    {
      _bfd_error_handler (_("corrupt EEOM record - size is too small"));
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  PRIV (eom_data).eom_l_total_lps = bfd_getl32 (eeom->total_lps);
  PRIV (eom_data).eom_w_comcod = bfd_getl16 (eeom->comcod);
  if (PRIV (eom_data).eom_w_comcod > 1)
    {
      _bfd_error_handler (_("object module not error-free !"));
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  PRIV (eom_data).eom_has_transfer = false;
  if (PRIV (recrd.rec_size) > 10)
    {
      PRIV (eom_data).eom_has_transfer = true;
      PRIV (eom_data).eom_b_tfrflg = eeom->tfrflg;
      PRIV (eom_data).eom_l_psindx = bfd_getl32 (eeom->psindx);
      PRIV (eom_data).eom_l_tfradr = bfd_getl32 (eeom->tfradr);

      abfd->start_address = PRIV (eom_data).eom_l_tfradr;
    }
  return true;
}

/* Slurp an ordered set of VMS object records.  Return FALSE on error.  */

static bool
_bfd_vms_slurp_object_records (bfd * abfd)
{
  bool ok;
  int type;

  do
    {
      vms_debug2 ((7, "reading at %08lx\n", (unsigned long)bfd_tell (abfd)));

      type = _bfd_vms_get_object_record (abfd);
      if (type < 0)
	{
	  vms_debug2 ((2, "next_record failed\n"));
	  return false;
	}

      switch (type)
	{
	case EOBJ__C_EMH:
	  ok = _bfd_vms_slurp_ehdr (abfd);
	  break;
	case EOBJ__C_EEOM:
	  ok = _bfd_vms_slurp_eeom (abfd);
	  break;
	case EOBJ__C_EGSD:
	  ok = _bfd_vms_slurp_egsd (abfd);
	  break;
	case EOBJ__C_ETIR:
	  ok = true; /* _bfd_vms_slurp_etir (abfd); */
	  break;
	case EOBJ__C_EDBG:
	  ok = _bfd_vms_slurp_edbg (abfd);
	  break;
	case EOBJ__C_ETBT:
	  ok = _bfd_vms_slurp_etbt (abfd);
	  break;
	default:
	  ok = false;
	}
      if (!ok)
	{
	  vms_debug2 ((2, "slurp type %d failed\n", type));
	  return false;
	}
    }
  while (type != EOBJ__C_EEOM);

  return true;
}

/* Initialize private data  */
static bool
vms_initialize (bfd * abfd)
{
  size_t amt;

  amt = sizeof (struct vms_private_data_struct);
  abfd->tdata.any = bfd_zalloc (abfd, amt);
  if (abfd->tdata.any == NULL)
    return false;

  PRIV (recrd.file_format) = FF_UNKNOWN;

  amt = sizeof (struct stack_struct) * STACKSIZE;
  PRIV (stack) = bfd_alloc (abfd, amt);
  if (PRIV (stack) == NULL)
    goto error_ret1;

  return true;

 error_ret1:
  bfd_release (abfd, abfd->tdata.any);
  abfd->tdata.any = NULL;
  return false;
}

/* Free malloc'd memory.  */

static void
alpha_vms_free_private (bfd *abfd)
{
  struct module *module;

  free (PRIV (recrd.buf));
  free (PRIV (sections));
  free (PRIV (syms));
  free (PRIV (dst_ptr_offsets));

  for (module = PRIV (modules); module; module = module->next)
    free (module->file_table);
}

/* Check the format for a file being read.
   Return a (bfd_target *) if it's an object file or zero if not.  */

static bfd_cleanup
alpha_vms_object_p (bfd *abfd)
{
  void *tdata_save = abfd->tdata.any;
  unsigned int test_len;
  unsigned char *buf;

  vms_debug2 ((1, "vms_object_p(%p)\n", abfd));

  /* Allocate alpha-vms specific data.  */
  if (!vms_initialize (abfd))
    {
      abfd->tdata.any = tdata_save;
      return NULL;
    }

  if (bfd_seek (abfd, (file_ptr) 0, SEEK_SET))
    goto error_ret;

  /* The first challenge with VMS is to discover the kind of the file.

     Image files (executable or shared images) are stored as a raw
     stream of bytes (like on UNIX), but there is no magic number.

     Object files are written with RMS (record management service), ie
     each records are preceeded by its length (on a word - 2 bytes), and
     padded for word-alignment.  That would be simple but when files
     are transfered to a UNIX filesystem (using ftp), records are lost.
     Only the raw content of the records are transfered.  Fortunately,
     the Alpha Object file format also store the length of the record
     in the records.  Is that clear ?  */

  /* Minimum is 6 bytes for objects (2 bytes size, 2 bytes record id,
     2 bytes size repeated) and 12 bytes for images (4 bytes major id,
     4 bytes minor id, 4 bytes length).  */
  test_len = 12;
  buf = _bfd_malloc_and_read (abfd, test_len, test_len);
  if (buf == NULL)
    goto error_ret;
  PRIV (recrd.buf) = buf;
  PRIV (recrd.buf_size) = test_len;
  PRIV (recrd.rec) = buf;

  /* Is it an image?  */
  if ((bfd_getl32 (buf) == EIHD__K_MAJORID)
      && (bfd_getl32 (buf + 4) == EIHD__K_MINORID))
    {
      unsigned int eisd_offset, eihs_offset;

      /* Extract the header size.  */
      PRIV (recrd.rec_size) = bfd_getl32 (buf + EIHD__L_SIZE);

      /* The header size is 0 for DSF files.  */
      if (PRIV (recrd.rec_size) == 0)
	PRIV (recrd.rec_size) = sizeof (struct vms_eihd);

      /* PR 21813: Check for a truncated record.  */
      /* PR 17512: file: 7d7c57c2.  */
      if (PRIV (recrd.rec_size) < sizeof (struct vms_eihd))
	goto err_wrong_format;

      if (bfd_seek (abfd, (file_ptr) 0, SEEK_SET))
	goto error_ret;

      free (PRIV (recrd.buf));
      PRIV (recrd.buf) = NULL;
      buf = _bfd_malloc_and_read (abfd, PRIV (recrd.rec_size),
				  PRIV (recrd.rec_size));
      if (buf == NULL)
	goto error_ret;

      PRIV (recrd.buf) = buf;
      PRIV (recrd.buf_size) = PRIV (recrd.rec_size);
      PRIV (recrd.rec) = buf;

      vms_debug2 ((2, "file type is image\n"));

      if (!_bfd_vms_slurp_eihd (abfd, &eisd_offset, &eihs_offset))
	goto err_wrong_format;

      if (!_bfd_vms_slurp_eisd (abfd, eisd_offset))
	goto err_wrong_format;

      /* EIHS is optional.  */
      if (eihs_offset != 0 && !_bfd_vms_slurp_eihs (abfd, eihs_offset))
	goto err_wrong_format;
    }
  else
    {
      int type;

      /* Assume it's a module and adjust record pointer if necessary.  */
      maybe_adjust_record_pointer_for_object (abfd);

      /* But is it really a module?  */
      if (bfd_getl16 (PRIV (recrd.rec)) <= EOBJ__C_MAXRECTYP
	  && bfd_getl16 (PRIV (recrd.rec) + 2) <= EOBJ__C_MAXRECSIZ)
	{
	  if (vms_get_remaining_object_record (abfd, test_len) <= 0)
	    goto err_wrong_format;

	  vms_debug2 ((2, "file type is module\n"));

	  type = bfd_getl16 (PRIV (recrd.rec));
	  if (type != EOBJ__C_EMH || !_bfd_vms_slurp_ehdr (abfd))
	    goto err_wrong_format;

	  if (!_bfd_vms_slurp_object_records (abfd))
	    goto err_wrong_format;
	}
      else
	goto err_wrong_format;
    }

  /* Set arch_info to alpha.   */

  if (! bfd_default_set_arch_mach (abfd, bfd_arch_alpha, 0))
    goto err_wrong_format;

  return alpha_vms_free_private;

 err_wrong_format:
  bfd_set_error (bfd_error_wrong_format);

 error_ret:
  alpha_vms_free_private (abfd);
  if (abfd->tdata.any != tdata_save && abfd->tdata.any != NULL)
    bfd_release (abfd, abfd->tdata.any);
  abfd->tdata.any = tdata_save;
  return NULL;
}

/* Image write.  */

/* Write an EMH/MHD record.  */

static void
_bfd_vms_write_emh (bfd *abfd)
{
  struct vms_rec_wr *recwr = &PRIV (recwr);
  unsigned char tbuf[18];

  _bfd_vms_output_alignment (recwr, 2);

  /* EMH.  */
  _bfd_vms_output_begin (recwr, EOBJ__C_EMH);
  _bfd_vms_output_short (recwr, EMH__C_MHD);
  _bfd_vms_output_short (recwr, EOBJ__C_STRLVL);
  _bfd_vms_output_long (recwr, 0);
  _bfd_vms_output_long (recwr, 0);
  _bfd_vms_output_long (recwr, MAX_OUTREC_SIZE);

  /* Create module name from filename.  */
  if (bfd_get_filename (abfd) != 0)
    {
      char *module = vms_get_module_name (bfd_get_filename (abfd), true);
      _bfd_vms_output_counted (recwr, module);
      free (module);
    }
  else
    _bfd_vms_output_counted (recwr, "NONAME");

  _bfd_vms_output_counted (recwr, BFD_VERSION_STRING);
  _bfd_vms_output_dump (recwr, get_vms_time_string (tbuf), EMH_DATE_LENGTH);
  _bfd_vms_output_fill (recwr, 0, EMH_DATE_LENGTH);
  _bfd_vms_output_end (abfd, recwr);
}

/* Write an EMH/LMN record.  */

static void
_bfd_vms_write_lmn (bfd *abfd, const char *name)
{
  char version [64];
  struct vms_rec_wr *recwr = &PRIV (recwr);
  unsigned int ver = BFD_VERSION / 10000;

  /* LMN.  */
  _bfd_vms_output_begin (recwr, EOBJ__C_EMH);
  _bfd_vms_output_short (recwr, EMH__C_LNM);
  snprintf (version, sizeof (version), "%s %d.%d.%d", name,
	    ver / 10000, (ver / 100) % 100, ver % 100);
  _bfd_vms_output_dump (recwr, (unsigned char *)version, strlen (version));
  _bfd_vms_output_end (abfd, recwr);
}


/* Write eom record for bfd abfd.  Return FALSE on error.  */

static bool
_bfd_vms_write_eeom (bfd *abfd)
{
  struct vms_rec_wr *recwr = &PRIV (recwr);

  vms_debug2 ((2, "vms_write_eeom\n"));

  _bfd_vms_output_alignment (recwr, 2);

  _bfd_vms_output_begin (recwr, EOBJ__C_EEOM);
  _bfd_vms_output_long (recwr, PRIV (vms_linkage_index + 1) >> 1);
  _bfd_vms_output_byte (recwr, 0);	/* Completion code.  */
  _bfd_vms_output_byte (recwr, 0);	/* Fill byte.  */

  if ((abfd->flags & EXEC_P) == 0
      && bfd_get_start_address (abfd) != (bfd_vma)-1)
    {
      asection *section;

      section = bfd_get_section_by_name (abfd, ".link");
      if (section == 0)
	{
	  bfd_set_error (bfd_error_nonrepresentable_section);
	  return false;
	}
      _bfd_vms_output_short (recwr, 0);
      _bfd_vms_output_long (recwr, (unsigned long) section->target_index);
      _bfd_vms_output_long (recwr,
			     (unsigned long) bfd_get_start_address (abfd));
      _bfd_vms_output_long (recwr, 0);
    }

  _bfd_vms_output_end (abfd, recwr);
  return true;
}

static void *
vector_grow1 (struct vector_type *vec, size_t elsz)
{
  if (vec->nbr_el >= vec->max_el)
    {
      if (vec->max_el == 0)
	{
	  vec->max_el = 16;
	  vec->els = bfd_malloc (vec->max_el * elsz);
	}
      else
	{
	  size_t amt;
	  if (vec->max_el > -1u / 2)
	    {
	      bfd_set_error (bfd_error_file_too_big);
	      return NULL;
	    }
	  vec->max_el *= 2;
	  if (_bfd_mul_overflow (vec->max_el, elsz, &amt))
	    {
	      bfd_set_error (bfd_error_file_too_big);
	      return NULL;
	    }
	  vec->els = bfd_realloc_or_free (vec->els, amt);
	}
    }
  if (vec->els == NULL)
    return NULL;
  return (char *) vec->els + elsz * vec->nbr_el++;
}

/* Bump ABFD file position to next block.  */

static void
alpha_vms_file_position_block (bfd *abfd)
{
  /* Next block.  */
  PRIV (file_pos) += VMS_BLOCK_SIZE - 1;
  PRIV (file_pos) -= (PRIV (file_pos) % VMS_BLOCK_SIZE);
}

/* Convert from internal structure SRC to external structure DST.  */

static void
alpha_vms_swap_eisd_out (struct vms_internal_eisd_map *src,
			 struct vms_eisd *dst)
{
  bfd_putl32 (src->u.eisd.majorid, dst->majorid);
  bfd_putl32 (src->u.eisd.minorid, dst->minorid);
  bfd_putl32 (src->u.eisd.eisdsize, dst->eisdsize);
  if (src->u.eisd.eisdsize <= EISD__K_LENEND)
    return;
  bfd_putl32 (src->u.eisd.secsize, dst->secsize);
  bfd_putl64 (src->u.eisd.virt_addr, dst->virt_addr);
  bfd_putl32 (src->u.eisd.flags, dst->flags);
  bfd_putl32 (src->u.eisd.vbn, dst->vbn);
  dst->pfc = src->u.eisd.pfc;
  dst->matchctl = src->u.eisd.matchctl;
  dst->type = src->u.eisd.type;
  dst->fill_1 = 0;
  if (src->u.eisd.flags & EISD__M_GBL)
    {
      bfd_putl32 (src->u.gbl_eisd.ident, dst->ident);
      memcpy (dst->gblnam, src->u.gbl_eisd.gblnam,
	      src->u.gbl_eisd.gblnam[0] + 1);
    }
}

/* Append EISD to the list of extra eisd for ABFD.  */

static void
alpha_vms_append_extra_eisd (bfd *abfd, struct vms_internal_eisd_map *eisd)
{
  eisd->next = NULL;
  if (PRIV (gbl_eisd_head) == NULL)
    PRIV (gbl_eisd_head) = eisd;
  else
    PRIV (gbl_eisd_tail)->next = eisd;
  PRIV (gbl_eisd_tail) = eisd;
}

/* Create an EISD for shared image SHRIMG.
   Return FALSE in case of error.  */

static bool
alpha_vms_create_eisd_for_shared (bfd *abfd, bfd *shrimg)
{
  struct vms_internal_eisd_map *eisd;
  int namlen;

  namlen = strlen (PRIV2 (shrimg, hdr_data.hdr_t_name));
  if (namlen + 5 > EISD__K_GBLNAMLEN)
    {
      /* Won't fit.  */
      return false;
    }

  eisd = bfd_alloc (abfd, sizeof (*eisd));
  if (eisd == NULL)
    return false;

  /* Fill the fields.  */
  eisd->u.gbl_eisd.common.majorid = EISD__K_MAJORID;
  eisd->u.gbl_eisd.common.minorid = EISD__K_MINORID;
  eisd->u.gbl_eisd.common.eisdsize = (EISD__K_LEN + 4 + namlen + 5 + 3) & ~3;
  eisd->u.gbl_eisd.common.secsize = VMS_BLOCK_SIZE;	/* Must not be 0.  */
  eisd->u.gbl_eisd.common.virt_addr = 0;
  eisd->u.gbl_eisd.common.flags = EISD__M_GBL;
  eisd->u.gbl_eisd.common.vbn = 0;
  eisd->u.gbl_eisd.common.pfc = 0;
  eisd->u.gbl_eisd.common.matchctl = PRIV2 (shrimg, matchctl);
  eisd->u.gbl_eisd.common.type = EISD__K_SHRPIC;

  eisd->u.gbl_eisd.ident = PRIV2 (shrimg, ident);
  eisd->u.gbl_eisd.gblnam[0] = namlen + 4;
  memcpy (eisd->u.gbl_eisd.gblnam + 1, PRIV2 (shrimg, hdr_data.hdr_t_name),
	  namlen);
  memcpy (eisd->u.gbl_eisd.gblnam + 1 + namlen, "_001", 4);

  /* Append it to the list.  */
  alpha_vms_append_extra_eisd (abfd, eisd);

  return true;
}

/* Create an EISD for section SEC.
   Return FALSE in case of failure.  */

static bool
alpha_vms_create_eisd_for_section (bfd *abfd, asection *sec)
{
  struct vms_internal_eisd_map *eisd;

  /* Only for allocating section.  */
  if (!(sec->flags & SEC_ALLOC))
    return true;

  BFD_ASSERT (vms_section_data (sec)->eisd == NULL);
  eisd = bfd_alloc (abfd, sizeof (*eisd));
  if (eisd == NULL)
    return false;
  vms_section_data (sec)->eisd = eisd;

  /* Fill the fields.  */
  eisd->u.eisd.majorid = EISD__K_MAJORID;
  eisd->u.eisd.minorid = EISD__K_MINORID;
  eisd->u.eisd.eisdsize = EISD__K_LEN;
  eisd->u.eisd.secsize =
    (sec->size + VMS_BLOCK_SIZE - 1) & ~(VMS_BLOCK_SIZE - 1);
  eisd->u.eisd.virt_addr = sec->vma;
  eisd->u.eisd.flags = 0;
  eisd->u.eisd.vbn = 0; /* To be later defined.  */
  eisd->u.eisd.pfc = 0; /* Default.  */
  eisd->u.eisd.matchctl = EISD__K_MATALL;
  eisd->u.eisd.type = EISD__K_NORMAL;

  if (sec->flags & SEC_CODE)
    eisd->u.eisd.flags |= EISD__M_EXE;
  if (!(sec->flags & SEC_READONLY))
    eisd->u.eisd.flags |= EISD__M_WRT | EISD__M_CRF;

  /* If relocations or fixup will be applied, make this isect writeable.  */
  if (sec->flags & SEC_RELOC)
    eisd->u.eisd.flags |= EISD__M_WRT | EISD__M_CRF;

  if (!(sec->flags & SEC_HAS_CONTENTS))
    {
      eisd->u.eisd.flags |= EISD__M_DZRO;
      eisd->u.eisd.flags &= ~EISD__M_CRF;
    }
  if (sec->flags & SEC_LINKER_CREATED)
    {
      if (strcmp (sec->name, "$FIXUP$") == 0)
	eisd->u.eisd.flags |= EISD__M_FIXUPVEC;
    }

  /* Append it to the list.  */
  eisd->next = NULL;
  if (PRIV (eisd_head) == NULL)
    PRIV (eisd_head) = eisd;
  else
    PRIV (eisd_tail)->next = eisd;
  PRIV (eisd_tail) = eisd;

  return true;
}

/* Layout executable ABFD and write it to the disk.
   Return FALSE in case of failure.  */

static bool
alpha_vms_write_exec (bfd *abfd)
{
  struct vms_eihd eihd;
  struct vms_eiha *eiha;
  struct vms_eihi *eihi;
  struct vms_eihs *eihs = NULL;
  asection *sec;
  struct vms_internal_eisd_map *first_eisd;
  struct vms_internal_eisd_map *eisd;
  asection *dst;
  asection *dmt;
  file_ptr gst_filepos = 0;
  unsigned int lnkflags = 0;

  /* Build the EIHD.  */
  PRIV (file_pos) = EIHD__C_LENGTH;

  memset (&eihd, 0, sizeof (eihd));
  memset (eihd.fill_2, 0xff, sizeof (eihd.fill_2));

  bfd_putl32 (EIHD__K_MAJORID, eihd.majorid);
  bfd_putl32 (EIHD__K_MINORID, eihd.minorid);

  bfd_putl32 (sizeof (eihd), eihd.size);
  bfd_putl32 (0, eihd.isdoff);
  bfd_putl32 (0, eihd.activoff);
  bfd_putl32 (0, eihd.symdbgoff);
  bfd_putl32 (0, eihd.imgidoff);
  bfd_putl32 (0, eihd.patchoff);
  bfd_putl64 (0, eihd.iafva);
  bfd_putl32 (0, eihd.version_array_off);

  bfd_putl32 (EIHD__K_EXE, eihd.imgtype);
  bfd_putl32 (0, eihd.subtype);

  bfd_putl32 (0, eihd.imgiocnt);
  bfd_putl32 (-1, eihd.privreqs);
  bfd_putl32 (-1, eihd.privreqs + 4);

  bfd_putl32 ((sizeof (eihd) + VMS_BLOCK_SIZE - 1) / VMS_BLOCK_SIZE,
	      eihd.hdrblkcnt);
  bfd_putl32 (0, eihd.ident);
  bfd_putl32 (0, eihd.sysver);

  eihd.matchctl = 0;
  bfd_putl32 (0, eihd.symvect_size);
  bfd_putl32 (16, eihd.virt_mem_block_size);
  bfd_putl32 (0, eihd.ext_fixup_off);
  bfd_putl32 (0, eihd.noopt_psect_off);
  bfd_putl16 (-1, eihd.alias);

  /* Alloc EIHA.  */
  eiha = (struct vms_eiha *)((char *) &eihd + PRIV (file_pos));
  bfd_putl32 (PRIV (file_pos), eihd.activoff);
  PRIV (file_pos) += sizeof (struct vms_eiha);

  bfd_putl32 (sizeof (struct vms_eiha), eiha->size);
  bfd_putl32 (0, eiha->spare);
  bfd_putl64 (PRIV (transfer_address[0]), eiha->tfradr1);
  bfd_putl64 (PRIV (transfer_address[1]), eiha->tfradr2);
  bfd_putl64 (PRIV (transfer_address[2]), eiha->tfradr3);
  bfd_putl64 (PRIV (transfer_address[3]), eiha->tfradr4);
  bfd_putl64 (0, eiha->inishr);

  /* Alloc EIHI.  */
  eihi = (struct vms_eihi *)((char *) &eihd + PRIV (file_pos));
  bfd_putl32 (PRIV (file_pos), eihd.imgidoff);
  PRIV (file_pos) += sizeof (struct vms_eihi);

  bfd_putl32 (EIHI__K_MAJORID, eihi->majorid);
  bfd_putl32 (EIHI__K_MINORID, eihi->minorid);
  {
    char *module;
    unsigned int len;

    /* Set module name.  */
    module = vms_get_module_name (bfd_get_filename (abfd), true);
    len = strlen (module);
    if (len > sizeof (eihi->imgnam) - 1)
      len = sizeof (eihi->imgnam) - 1;
    eihi->imgnam[0] = len;
    memcpy (eihi->imgnam + 1, module, len);
    free (module);
  }
  {
    unsigned int lo;
    unsigned int hi;

    /* Set time.  */
    vms_get_time (&hi, &lo);
    bfd_putl32 (lo, eihi->linktime + 0);
    bfd_putl32 (hi, eihi->linktime + 4);
  }
  eihi->imgid[0] = 0;
  eihi->linkid[0] = 0;
  eihi->imgbid[0] = 0;

  /* Alloc EIHS.  */
  dst = PRIV (dst_section);
  dmt = bfd_get_section_by_name (abfd, "$DMT$");
  if (dst != NULL && dst->size != 0)
    {
      eihs = (struct vms_eihs *)((char *) &eihd + PRIV (file_pos));
      bfd_putl32 (PRIV (file_pos), eihd.symdbgoff);
      PRIV (file_pos) += sizeof (struct vms_eihs);

      bfd_putl32 (EIHS__K_MAJORID, eihs->majorid);
      bfd_putl32 (EIHS__K_MINORID, eihs->minorid);
      bfd_putl32 (0, eihs->dstvbn);
      bfd_putl32 (0, eihs->dstsize);
      bfd_putl32 (0, eihs->gstvbn);
      bfd_putl32 (0, eihs->gstsize);
      bfd_putl32 (0, eihs->dmtvbn);
      bfd_putl32 (0, eihs->dmtsize);
    }

  /* One EISD per section.  */
  for (sec = abfd->sections; sec; sec = sec->next)
    {
      if (!alpha_vms_create_eisd_for_section (abfd, sec))
	return false;
    }

  /* Merge section EIDS which extra ones.  */
  if (PRIV (eisd_tail))
    PRIV (eisd_tail)->next = PRIV (gbl_eisd_head);
  else
    PRIV (eisd_head) = PRIV (gbl_eisd_head);
  if (PRIV (gbl_eisd_tail))
    PRIV (eisd_tail) = PRIV (gbl_eisd_tail);

  first_eisd = PRIV (eisd_head);

  /* Add end of eisd.  */
  if (first_eisd)
    {
      eisd = bfd_zalloc (abfd, sizeof (*eisd));
      if (eisd == NULL)
	return false;
      eisd->u.eisd.majorid = 0;
      eisd->u.eisd.minorid = 0;
      eisd->u.eisd.eisdsize = 0;
      alpha_vms_append_extra_eisd (abfd, eisd);
    }

  /* Place EISD in the file.  */
  for (eisd = first_eisd; eisd; eisd = eisd->next)
    {
      file_ptr room = VMS_BLOCK_SIZE - (PRIV (file_pos) % VMS_BLOCK_SIZE);

      /* First block is a little bit special: there is a word at the end.  */
      if (PRIV (file_pos) < VMS_BLOCK_SIZE && room > 2)
	room -= 2;
      if (room < eisd->u.eisd.eisdsize + EISD__K_LENEND)
	alpha_vms_file_position_block (abfd);

      eisd->file_pos = PRIV (file_pos);
      PRIV (file_pos) += eisd->u.eisd.eisdsize;

      if (eisd->u.eisd.flags & EISD__M_FIXUPVEC)
	bfd_putl64 (eisd->u.eisd.virt_addr, eihd.iafva);
    }

  if (first_eisd != NULL)
    {
      bfd_putl32 (first_eisd->file_pos, eihd.isdoff);
      /* Real size of end of eisd marker.  */
      PRIV (file_pos) += EISD__K_LENEND;
    }

  bfd_putl32 (PRIV (file_pos), eihd.size);
  bfd_putl32 ((PRIV (file_pos) + VMS_BLOCK_SIZE - 1) / VMS_BLOCK_SIZE,
	      eihd.hdrblkcnt);

  /* Place sections.  */
  for (sec = abfd->sections; sec; sec = sec->next)
    {
      if (!(sec->flags & SEC_HAS_CONTENTS)
	  || sec->contents == NULL)
	continue;

      eisd = vms_section_data (sec)->eisd;

      /* Align on a block.  */
      alpha_vms_file_position_block (abfd);
      sec->filepos = PRIV (file_pos);

      if (eisd != NULL)
	eisd->u.eisd.vbn = (sec->filepos / VMS_BLOCK_SIZE) + 1;

      PRIV (file_pos) += sec->size;
    }

  /* Update EIHS.  */
  if (eihs != NULL && dst != NULL)
    {
      bfd_putl32 ((dst->filepos / VMS_BLOCK_SIZE) + 1, eihs->dstvbn);
      bfd_putl32 (dst->size, eihs->dstsize);

      if (dmt != NULL)
	{
	  lnkflags |= EIHD__M_DBGDMT;
	  bfd_putl32 ((dmt->filepos / VMS_BLOCK_SIZE) + 1, eihs->dmtvbn);
	  bfd_putl32 (dmt->size, eihs->dmtsize);
	}
      if (PRIV (gsd_sym_count) != 0)
	{
	  alpha_vms_file_position_block (abfd);
	  gst_filepos = PRIV (file_pos);
	  bfd_putl32 ((gst_filepos / VMS_BLOCK_SIZE) + 1, eihs->gstvbn);
	  bfd_putl32 ((PRIV (gsd_sym_count) + 4) / 5 + 4, eihs->gstsize);
	}
    }

  /* Write EISD in hdr.  */
  for (eisd = first_eisd; eisd && eisd->file_pos < VMS_BLOCK_SIZE;
       eisd = eisd->next)
    alpha_vms_swap_eisd_out
      (eisd, (struct vms_eisd *)((char *)&eihd + eisd->file_pos));

  /* Write first block.  */
  bfd_putl32 (lnkflags, eihd.lnkflags);
  if (bfd_bwrite (&eihd, sizeof (eihd), abfd) != sizeof (eihd))
    return false;

  /* Write remaining eisd.  */
  if (eisd != NULL)
    {
      unsigned char blk[VMS_BLOCK_SIZE];
      struct vms_internal_eisd_map *next_eisd;

      memset (blk, 0xff, sizeof (blk));
      while (eisd != NULL)
	{
	  alpha_vms_swap_eisd_out
	    (eisd,
	     (struct vms_eisd *)(blk + (eisd->file_pos % VMS_BLOCK_SIZE)));

	  next_eisd = eisd->next;
	  if (next_eisd == NULL
	      || (next_eisd->file_pos / VMS_BLOCK_SIZE
		  != eisd->file_pos / VMS_BLOCK_SIZE))
	    {
	      if (bfd_bwrite (blk, sizeof (blk), abfd) != sizeof (blk))
		return false;

	      memset (blk, 0xff, sizeof (blk));
	    }
	  eisd = next_eisd;
	}
    }

  /* Write sections.  */
  for (sec = abfd->sections; sec; sec = sec->next)
    {
      unsigned char blk[VMS_BLOCK_SIZE];
      bfd_size_type len;

      if (sec->size == 0 || !(sec->flags & SEC_HAS_CONTENTS)
	  || sec->contents == NULL)
	continue;
      if (bfd_bwrite (sec->contents, sec->size, abfd) != sec->size)
	return false;

      /* Pad.  */
      len = VMS_BLOCK_SIZE - sec->size % VMS_BLOCK_SIZE;
      if (len != VMS_BLOCK_SIZE)
	{
	  memset (blk, 0, len);
	  if (bfd_bwrite (blk, len, abfd) != len)
	    return false;
	}
    }

  /* Write GST.  */
  if (gst_filepos != 0)
    {
      struct vms_rec_wr *recwr = &PRIV (recwr);
      unsigned int i;

      _bfd_vms_write_emh (abfd);
      _bfd_vms_write_lmn (abfd, "GNU LD");

      /* PSC for the absolute section.  */
      _bfd_vms_output_begin (recwr, EOBJ__C_EGSD);
      _bfd_vms_output_long (recwr, 0);
      _bfd_vms_output_begin_subrec (recwr, EGSD__C_PSC);
      _bfd_vms_output_short (recwr, 0);
      _bfd_vms_output_short (recwr, EGPS__V_PIC | EGPS__V_LIB | EGPS__V_RD);
      _bfd_vms_output_long (recwr, 0);
      _bfd_vms_output_counted (recwr, ".$$ABS$$.");
      _bfd_vms_output_end_subrec (recwr);
      _bfd_vms_output_end (abfd, recwr);

      for (i = 0; i < PRIV (gsd_sym_count); i++)
	{
	  struct vms_symbol_entry *sym = PRIV (syms)[i];
	  bfd_vma val;
	  bfd_vma ep;

	  if ((i % 5) == 0)
	    {
	      _bfd_vms_output_alignment (recwr, 8);
	      _bfd_vms_output_begin (recwr, EOBJ__C_EGSD);
	      _bfd_vms_output_long (recwr, 0);
	    }
	  _bfd_vms_output_begin_subrec (recwr, EGSD__C_SYMG);
	  _bfd_vms_output_short (recwr, 0); /* Data type, alignment.  */
	  _bfd_vms_output_short (recwr, sym->flags);

	  if (sym->code_section)
	    ep = alpha_vms_get_sym_value (sym->code_section, sym->code_value);
	  else
	    {
	      BFD_ASSERT (sym->code_value == 0);
	      ep = 0;
	    }
	  val = alpha_vms_get_sym_value (sym->section, sym->value);
	  _bfd_vms_output_quad
	    (recwr, sym->typ == EGSD__C_SYMG ? sym->symbol_vector : val);
	  _bfd_vms_output_quad (recwr, ep);
	  _bfd_vms_output_quad (recwr, val);
	  _bfd_vms_output_long (recwr, 0);
	  _bfd_vms_output_counted (recwr, sym->name);
	  _bfd_vms_output_end_subrec (recwr);
	  if ((i % 5) == 4)
	    _bfd_vms_output_end (abfd, recwr);
	}
      if ((i % 5) != 0)
	_bfd_vms_output_end (abfd, recwr);

      if (!_bfd_vms_write_eeom (abfd))
	return false;
    }
  return true;
}

/* Object write.  */

/* Write section and symbol directory of bfd abfd.  Return FALSE on error.  */

static bool
_bfd_vms_write_egsd (bfd *abfd)
{
  asection *section;
  asymbol *symbol;
  unsigned int symnum;
  const char *sname;
  flagword new_flags, old_flags;
  int abs_section_index = -1;
  unsigned int target_index = 0;
  struct vms_rec_wr *recwr = &PRIV (recwr);

  vms_debug2 ((2, "vms_write_egsd\n"));

  /* Egsd is quadword aligned.  */
  _bfd_vms_output_alignment (recwr, 8);

  _bfd_vms_output_begin (recwr, EOBJ__C_EGSD);
  _bfd_vms_output_long (recwr, 0);

  /* Number sections.  */
  for (section = abfd->sections; section != NULL; section = section->next)
    {
      if (section->flags & SEC_DEBUGGING)
	continue;
      if (!strcmp (section->name, ".vmsdebug"))
	{
	  section->flags |= SEC_DEBUGGING;
	  continue;
	}
      section->target_index = target_index++;
    }

  for (section = abfd->sections; section != NULL; section = section->next)
    {
      vms_debug2 ((3, "Section #%d %s, %d bytes\n",
		   section->target_index, section->name, (int)section->size));

      /* Don't write out the VMS debug info section since it is in the
	 ETBT and EDBG sections in etir. */
      if (section->flags & SEC_DEBUGGING)
	continue;

      /* 13 bytes egsd, max 31 chars name -> should be 44 bytes.  */
      if (_bfd_vms_output_check (recwr, 64) < 0)
	{
	  _bfd_vms_output_end (abfd, recwr);
	  _bfd_vms_output_begin (recwr, EOBJ__C_EGSD);
	  _bfd_vms_output_long (recwr, 0);
	}

      /* Don't know if this is necessary for the linker but for now it keeps
	 vms_slurp_gsd happy.  */
      sname = section->name;
      if (*sname == '.')
	{
	  /* Remove leading dot.  */
	  sname++;
	  if ((*sname == 't') && (strcmp (sname, "text") == 0))
	    sname = EVAX_CODE_NAME;
	  else if ((*sname == 'd') && (strcmp (sname, "data") == 0))
	    sname = EVAX_DATA_NAME;
	  else if ((*sname == 'b') && (strcmp (sname, "bss") == 0))
	    sname = EVAX_BSS_NAME;
	  else if ((*sname == 'l') && (strcmp (sname, "link") == 0))
	    sname = EVAX_LINK_NAME;
	  else if ((*sname == 'r') && (strcmp (sname, "rdata") == 0))
	    sname = EVAX_READONLY_NAME;
	  else if ((*sname == 'l') && (strcmp (sname, "literal") == 0))
	    sname = EVAX_LITERAL_NAME;
	  else if ((*sname == 'l') && (strcmp (sname, "literals") == 0))
	    sname = EVAX_LITERALS_NAME;
	  else if ((*sname == 'c') && (strcmp (sname, "comm") == 0))
	    sname = EVAX_COMMON_NAME;
	  else if ((*sname == 'l') && (strcmp (sname, "lcomm") == 0))
	    sname = EVAX_LOCAL_NAME;
	}

      if (bfd_is_com_section (section))
	new_flags = (EGPS__V_OVR | EGPS__V_REL | EGPS__V_GBL | EGPS__V_RD
		     | EGPS__V_WRT | EGPS__V_NOMOD | EGPS__V_COM);
      else
	new_flags = vms_esecflag_by_name (evax_section_flags, sname,
					  section->size > 0);

      /* Modify them as directed.  */
      if (section->flags & SEC_READONLY)
	new_flags &= ~EGPS__V_WRT;

      new_flags &= ~vms_section_data (section)->no_flags;
      new_flags |= vms_section_data (section)->flags;

      vms_debug2 ((3, "sec flags %x\n", section->flags));
      vms_debug2 ((3, "new_flags %x, _raw_size %lu\n",
		   new_flags, (unsigned long)section->size));

      _bfd_vms_output_begin_subrec (recwr, EGSD__C_PSC);
      _bfd_vms_output_short (recwr, section->alignment_power & 0xff);
      _bfd_vms_output_short (recwr, new_flags);
      _bfd_vms_output_long (recwr, (unsigned long) section->size);
      _bfd_vms_output_counted (recwr, sname);
      _bfd_vms_output_end_subrec (recwr);

      /* If the section is an obsolute one, remind its index as it will be
	 used later for absolute symbols.  */
      if ((new_flags & EGPS__V_REL) == 0 && abs_section_index < 0)
	abs_section_index = section->target_index;
    }

  /* Output symbols.  */
  vms_debug2 ((3, "%d symbols found\n", abfd->symcount));

  bfd_set_start_address (abfd, (bfd_vma) -1);

  for (symnum = 0; symnum < abfd->symcount; symnum++)
    {
      symbol = abfd->outsymbols[symnum];
      old_flags = symbol->flags;

      /* Work-around a missing feature:  consider __main as the main entry
	 point.  */
      if (symbol->name[0] == '_' && strcmp (symbol->name, "__main") == 0)
	bfd_set_start_address (abfd, (bfd_vma)symbol->value);

      /* Only put in the GSD the global and the undefined symbols.  */
      if (old_flags & BSF_FILE)
	continue;

      if ((old_flags & BSF_GLOBAL) == 0 && !bfd_is_und_section (symbol->section))
	{
	  /* If the LIB$INITIIALIZE section is present, add a reference to
	     LIB$INITIALIZE symbol.  FIXME: this should be done explicitely
	     in the assembly file.  */
	  if (!((old_flags & BSF_SECTION_SYM) != 0
		&& strcmp (symbol->section->name, "LIB$INITIALIZE") == 0))
	    continue;
	}

      /* 13 bytes egsd, max 64 chars name -> should be 77 bytes.  Add 16 more
	 bytes for a possible ABS section.  */
      if (_bfd_vms_output_check (recwr, 80 + 16) < 0)
	{
	  _bfd_vms_output_end (abfd, recwr);
	  _bfd_vms_output_begin (recwr, EOBJ__C_EGSD);
	  _bfd_vms_output_long (recwr, 0);
	}

      if ((old_flags & BSF_GLOBAL) != 0
	  && bfd_is_abs_section (symbol->section)
	  && abs_section_index <= 0)
	{
	  /* Create an absolute section if none was defined.  It is highly
	     unlikely that the name $ABS$ clashes with a user defined
	     non-absolute section name.  */
	  _bfd_vms_output_begin_subrec (recwr, EGSD__C_PSC);
	  _bfd_vms_output_short (recwr, 4);
	  _bfd_vms_output_short (recwr, EGPS__V_SHR);
	  _bfd_vms_output_long (recwr, 0);
	  _bfd_vms_output_counted (recwr, "$ABS$");
	  _bfd_vms_output_end_subrec (recwr);

	  abs_section_index = target_index++;
	}

      _bfd_vms_output_begin_subrec (recwr, EGSD__C_SYM);

      /* Data type, alignment.  */
      _bfd_vms_output_short (recwr, 0);

      new_flags = 0;

      if (old_flags & BSF_WEAK)
	new_flags |= EGSY__V_WEAK;
      if (bfd_is_com_section (symbol->section))		/* .comm  */
	new_flags |= (EGSY__V_WEAK | EGSY__V_COMM);

      if (old_flags & BSF_FUNCTION)
	{
	  new_flags |= EGSY__V_NORM;
	  new_flags |= EGSY__V_REL;
	}
      if (old_flags & BSF_GLOBAL)
	{
	  new_flags |= EGSY__V_DEF;
	  if (!bfd_is_abs_section (symbol->section))
	    new_flags |= EGSY__V_REL;
	}
      _bfd_vms_output_short (recwr, new_flags);

      if (old_flags & BSF_GLOBAL)
	{
	  /* Symbol definition.  */
	  bfd_vma code_address = 0;
	  unsigned long ca_psindx = 0;
	  unsigned long psindx;

	  if ((old_flags & BSF_FUNCTION) && symbol->udata.p != NULL)
	    {
	      asymbol *sym;

	      sym =
		((struct evax_private_udata_struct *)symbol->udata.p)->enbsym;
	      code_address = sym->value;
	      ca_psindx = sym->section->target_index;
	    }
	  if (bfd_is_abs_section (symbol->section))
	    psindx = abs_section_index;
	  else
	    psindx = symbol->section->target_index;

	  _bfd_vms_output_quad (recwr, symbol->value);
	  _bfd_vms_output_quad (recwr, code_address);
	  _bfd_vms_output_long (recwr, ca_psindx);
	  _bfd_vms_output_long (recwr, psindx);
	}
      _bfd_vms_output_counted (recwr, symbol->name);

      _bfd_vms_output_end_subrec (recwr);
    }

  _bfd_vms_output_alignment (recwr, 8);
  _bfd_vms_output_end (abfd, recwr);

  return true;
}

/* Write object header for bfd abfd.  Return FALSE on error.  */

static bool
_bfd_vms_write_ehdr (bfd *abfd)
{
  asymbol *symbol;
  unsigned int symnum;
  struct vms_rec_wr *recwr = &PRIV (recwr);

  vms_debug2 ((2, "vms_write_ehdr (%p)\n", abfd));

  _bfd_vms_output_alignment (recwr, 2);

  _bfd_vms_write_emh (abfd);
  _bfd_vms_write_lmn (abfd, "GNU AS");

  /* SRC.  */
  _bfd_vms_output_begin (recwr, EOBJ__C_EMH);
  _bfd_vms_output_short (recwr, EMH__C_SRC);

  for (symnum = 0; symnum < abfd->symcount; symnum++)
    {
      symbol = abfd->outsymbols[symnum];

      if (symbol->flags & BSF_FILE)
	{
	  _bfd_vms_output_dump (recwr, (unsigned char *) symbol->name,
				(int) strlen (symbol->name));
	  break;
	}
    }

  if (symnum == abfd->symcount)
    _bfd_vms_output_dump (recwr, (unsigned char *) STRING_COMMA_LEN ("noname"));

  _bfd_vms_output_end (abfd, recwr);

  /* TTL.  */
  _bfd_vms_output_begin (recwr, EOBJ__C_EMH);
  _bfd_vms_output_short (recwr, EMH__C_TTL);
  _bfd_vms_output_dump (recwr, (unsigned char *) STRING_COMMA_LEN ("TTL"));
  _bfd_vms_output_end (abfd, recwr);

  /* CPR.  */
  _bfd_vms_output_begin (recwr, EOBJ__C_EMH);
  _bfd_vms_output_short (recwr, EMH__C_CPR);
  _bfd_vms_output_dump (recwr,
			(unsigned char *)"GNU BFD ported by Klaus Kämpf 1994-1996",
			 39);
  _bfd_vms_output_end (abfd, recwr);

  return true;
}

/* Part 4.6, relocations.  */


/* WRITE ETIR SECTION

   This is still under construction and therefore not documented.  */

/* Close the etir/etbt record.  */

static void
end_etir_record (bfd * abfd)
{
  struct vms_rec_wr *recwr = &PRIV (recwr);

  _bfd_vms_output_end (abfd, recwr);
}

static void
start_etir_or_etbt_record (bfd *abfd, asection *section, bfd_vma offset)
{
  struct vms_rec_wr *recwr = &PRIV (recwr);

  if (section->flags & SEC_DEBUGGING)
    {
      _bfd_vms_output_begin (recwr, EOBJ__C_ETBT);

      if (offset == 0)
	{
	  /* Push start offset.  */
	  _bfd_vms_output_begin_subrec (recwr, ETIR__C_STA_LW);
	  _bfd_vms_output_long (recwr, (unsigned long) 0);
	  _bfd_vms_output_end_subrec (recwr);

	  /* Set location.  */
	  _bfd_vms_output_begin_subrec (recwr, ETIR__C_CTL_DFLOC);
	  _bfd_vms_output_end_subrec (recwr);
	}
    }
  else
    {
      _bfd_vms_output_begin (recwr, EOBJ__C_ETIR);

      if (offset == 0)
	{
	  /* Push start offset.  */
	  _bfd_vms_output_begin_subrec (recwr, ETIR__C_STA_PQ);
	  _bfd_vms_output_long (recwr, (unsigned long) section->target_index);
	  _bfd_vms_output_quad (recwr, offset);
	  _bfd_vms_output_end_subrec (recwr);

	  /* Start = pop ().  */
	  _bfd_vms_output_begin_subrec (recwr, ETIR__C_CTL_SETRB);
	  _bfd_vms_output_end_subrec (recwr);
	}
    }
}

/* Output a STO_IMM command for SSIZE bytes of data from CPR at virtual
   address VADDR in section specified by SEC_INDEX and NAME.  */

static void
sto_imm (bfd *abfd, asection *section,
	 bfd_size_type ssize, unsigned char *cptr, bfd_vma vaddr)
{
  bfd_size_type size;
  struct vms_rec_wr *recwr = &PRIV (recwr);

#if VMS_DEBUG
  _bfd_vms_debug (8, "sto_imm %d bytes\n", (int) ssize);
  _bfd_hexdump (9, cptr, (int) ssize, (int) vaddr);
#endif

  while (ssize > 0)
    {
      /* Try all the rest.  */
      size = ssize;

      if (_bfd_vms_output_check (recwr, size) < 0)
	{
	  /* Doesn't fit, split !  */
	  end_etir_record (abfd);

	  start_etir_or_etbt_record (abfd, section, vaddr);

	  size = _bfd_vms_output_check (recwr, 0);	/* get max size */
	  if (size > ssize)			/* more than what's left ? */
	    size = ssize;
	}

      _bfd_vms_output_begin_subrec (recwr, ETIR__C_STO_IMM);
      _bfd_vms_output_long (recwr, (unsigned long) (size));
      _bfd_vms_output_dump (recwr, cptr, size);
      _bfd_vms_output_end_subrec (recwr);

#if VMS_DEBUG
      _bfd_vms_debug (10, "dumped %d bytes\n", (int) size);
      _bfd_hexdump (10, cptr, (int) size, (int) vaddr);
#endif

      vaddr += size;
      cptr += size;
      ssize -= size;
    }
}

static void
etir_output_check (bfd *abfd, asection *section, bfd_vma vaddr, int checklen)
{
  if (_bfd_vms_output_check (&PRIV (recwr), checklen) < 0)
    {
      /* Not enough room in this record.  Close it and open a new one.  */
      end_etir_record (abfd);
      start_etir_or_etbt_record (abfd, section, vaddr);
    }
}

/* Return whether RELOC must be deferred till the end.  */

static bool
defer_reloc_p (arelent *reloc)
{
  switch (reloc->howto->type)
    {
    case ALPHA_R_NOP:
    case ALPHA_R_LDA:
    case ALPHA_R_BSR:
    case ALPHA_R_BOH:
      return true;

    default:
      return false;
    }
}

/* Write section contents for bfd abfd.  Return FALSE on error.  */

static bool
_bfd_vms_write_etir (bfd * abfd, int objtype ATTRIBUTE_UNUSED)
{
  asection *section;
  struct vms_rec_wr *recwr = &PRIV (recwr);

  vms_debug2 ((2, "vms_write_tir (%p, %d)\n", abfd, objtype));

  _bfd_vms_output_alignment (recwr, 4);

  PRIV (vms_linkage_index) = 0;

  for (section = abfd->sections; section; section = section->next)
    {
      vms_debug2 ((4, "writing %d. section '%s' (%d bytes)\n",
		   section->target_index, section->name, (int) (section->size)));

      if (!(section->flags & SEC_HAS_CONTENTS)
	  || bfd_is_com_section (section))
	continue;

      if (!section->contents)
	{
	  bfd_set_error (bfd_error_no_contents);
	  return false;
	}

      start_etir_or_etbt_record (abfd, section, 0);

      if (section->flags & SEC_RELOC)
	{
	  bfd_vma curr_addr = 0;
	  unsigned char *curr_data = section->contents;
	  bfd_size_type size;
	  int pass2_needed = 0;
	  int pass2_in_progress = 0;
	  unsigned int irel;

	  if (section->reloc_count == 0)
	    _bfd_error_handler
	      (_("SEC_RELOC with no relocs in section %pA"), section);

#if VMS_DEBUG
	  else
	    {
	      int i = section->reloc_count;
	      arelent **rptr = section->orelocation;
	      _bfd_vms_debug (4, "%d relocations:\n", i);
	      while (i-- > 0)
		{
		  _bfd_vms_debug (4, "sym %s in sec %s, value %08lx, "
				     "addr %08lx, off %08lx, len %d: %s\n",
				  (*(*rptr)->sym_ptr_ptr)->name,
				  (*(*rptr)->sym_ptr_ptr)->section->name,
				  (long) (*(*rptr)->sym_ptr_ptr)->value,
				  (unsigned long)(*rptr)->address,
				  (unsigned long)(*rptr)->addend,
				  bfd_get_reloc_size ((*rptr)->howto),
				  ( *rptr)->howto->name);
		  rptr++;
		}
	    }
#endif

	new_pass:
	  for (irel = 0; irel < section->reloc_count; irel++)
	    {
	      struct evax_private_udata_struct *udata;
	      arelent *rptr = section->orelocation [irel];
	      bfd_vma addr = rptr->address;
	      asymbol *sym = *rptr->sym_ptr_ptr;
	      asection *sec = sym->section;
	      bool defer = defer_reloc_p (rptr);
	      unsigned int slen;

	      if (pass2_in_progress)
		{
		  /* Non-deferred relocs have already been output.  */
		  if (!defer)
		    continue;
		}
	      else
		{
		  /* Deferred relocs must be output at the very end.  */
		  if (defer)
		    {
		      pass2_needed = 1;
		      continue;
		    }

		  /* Regular relocs are intertwined with binary data.  */
		  if (curr_addr > addr)
		    _bfd_error_handler (_("size error in section %pA"),
					section);
		  size = addr - curr_addr;
		  sto_imm (abfd, section, size, curr_data, curr_addr);
		  curr_data += size;
		  curr_addr += size;
		}

	      size = bfd_get_reloc_size (rptr->howto);

	      switch (rptr->howto->type)
		{
		case ALPHA_R_IGNORE:
		  break;

		case ALPHA_R_REFLONG:
		  if (bfd_is_und_section (sym->section))
		    {
		      bfd_vma addend = rptr->addend;
		      slen = strlen ((char *) sym->name);
		      etir_output_check (abfd, section, curr_addr, slen);
		      if (addend)
			{
			  _bfd_vms_output_begin_subrec (recwr, ETIR__C_STA_GBL);
			  _bfd_vms_output_counted (recwr, sym->name);
			  _bfd_vms_output_end_subrec (recwr);
			  _bfd_vms_output_begin_subrec (recwr, ETIR__C_STA_LW);
			  _bfd_vms_output_long (recwr, (unsigned long) addend);
			  _bfd_vms_output_end_subrec (recwr);
			  _bfd_vms_output_begin_subrec (recwr, ETIR__C_OPR_ADD);
			  _bfd_vms_output_end_subrec (recwr);
			  _bfd_vms_output_begin_subrec (recwr, ETIR__C_STO_LW);
			  _bfd_vms_output_end_subrec (recwr);
			}
		      else
			{
			  _bfd_vms_output_begin_subrec
			    (recwr, ETIR__C_STO_GBL_LW);
			  _bfd_vms_output_counted (recwr, sym->name);
			  _bfd_vms_output_end_subrec (recwr);
			}
		    }
		  else if (bfd_is_abs_section (sym->section))
		    {
		      etir_output_check (abfd, section, curr_addr, 16);
		      _bfd_vms_output_begin_subrec (recwr, ETIR__C_STA_LW);
		      _bfd_vms_output_long (recwr, (unsigned long) sym->value);
		      _bfd_vms_output_end_subrec (recwr);
		      _bfd_vms_output_begin_subrec (recwr, ETIR__C_STO_LW);
		      _bfd_vms_output_end_subrec (recwr);
		    }
		  else
		    {
		      etir_output_check (abfd, section, curr_addr, 32);
		      _bfd_vms_output_begin_subrec (recwr, ETIR__C_STA_PQ);
		      _bfd_vms_output_long (recwr,
					    (unsigned long) sec->target_index);
		      _bfd_vms_output_quad (recwr, rptr->addend + sym->value);
		      _bfd_vms_output_end_subrec (recwr);
		      /* ??? Table B-8 of the OpenVMS Linker Utilily Manual
			 says that we should have a ETIR__C_STO_OFF here.
			 But the relocation would not be BFD_RELOC_32 then.
			 This case is very likely unreachable.  */
		      _bfd_vms_output_begin_subrec (recwr, ETIR__C_STO_LW);
		      _bfd_vms_output_end_subrec (recwr);
		    }
		  break;

		case ALPHA_R_REFQUAD:
		  if (bfd_is_und_section (sym->section))
		    {
		      bfd_vma addend = rptr->addend;
		      slen = strlen ((char *) sym->name);
		      etir_output_check (abfd, section, curr_addr, slen);
		      if (addend)
			{
			  _bfd_vms_output_begin_subrec (recwr, ETIR__C_STA_GBL);
			  _bfd_vms_output_counted (recwr, sym->name);
			  _bfd_vms_output_end_subrec (recwr);
			  _bfd_vms_output_begin_subrec (recwr, ETIR__C_STA_QW);
			  _bfd_vms_output_quad (recwr, addend);
			  _bfd_vms_output_end_subrec (recwr);
			  _bfd_vms_output_begin_subrec (recwr, ETIR__C_OPR_ADD);
			  _bfd_vms_output_end_subrec (recwr);
			  _bfd_vms_output_begin_subrec (recwr, ETIR__C_STO_QW);
			  _bfd_vms_output_end_subrec (recwr);
			}
		      else
			{
			  _bfd_vms_output_begin_subrec (recwr, ETIR__C_STO_GBL);
			  _bfd_vms_output_counted (recwr, sym->name);
			  _bfd_vms_output_end_subrec (recwr);
			}
		    }
		  else if (bfd_is_abs_section (sym->section))
		    {
		      etir_output_check (abfd, section, curr_addr, 16);
		      _bfd_vms_output_begin_subrec (recwr, ETIR__C_STA_QW);
		      _bfd_vms_output_quad (recwr, sym->value);
		      _bfd_vms_output_end_subrec (recwr);
		      _bfd_vms_output_begin_subrec (recwr, ETIR__C_STO_QW);
		      _bfd_vms_output_end_subrec (recwr);
		    }
		  else
		    {
		      etir_output_check (abfd, section, curr_addr, 32);
		      _bfd_vms_output_begin_subrec (recwr, ETIR__C_STA_PQ);
		      _bfd_vms_output_long (recwr,
					    (unsigned long) sec->target_index);
		      _bfd_vms_output_quad (recwr, rptr->addend + sym->value);
		      _bfd_vms_output_end_subrec (recwr);
		      _bfd_vms_output_begin_subrec (recwr, ETIR__C_STO_OFF);
		      _bfd_vms_output_end_subrec (recwr);
		    }
		  break;

		case ALPHA_R_HINT:
		  sto_imm (abfd, section, size, curr_data, curr_addr);
		  break;

		case ALPHA_R_LINKAGE:
		  size = 16;
		  etir_output_check (abfd, section, curr_addr, 64);
		  _bfd_vms_output_begin_subrec (recwr, ETIR__C_STC_LP_PSB);
		  _bfd_vms_output_long
		    (recwr, (unsigned long) rptr->addend);
		  if (rptr->addend > PRIV (vms_linkage_index))
		    PRIV (vms_linkage_index) = rptr->addend;
		  _bfd_vms_output_counted (recwr, sym->name);
		  _bfd_vms_output_byte (recwr, 0);
		  _bfd_vms_output_end_subrec (recwr);
		  break;

		case ALPHA_R_CODEADDR:
		  slen = strlen ((char *) sym->name);
		  etir_output_check (abfd, section, curr_addr, slen);
		  _bfd_vms_output_begin_subrec (recwr, ETIR__C_STO_CA);
		  _bfd_vms_output_counted (recwr, sym->name);
		  _bfd_vms_output_end_subrec (recwr);
		  break;

		case ALPHA_R_NOP:
		  udata
		    = (struct evax_private_udata_struct *) rptr->sym_ptr_ptr;
		  etir_output_check (abfd, section, curr_addr,
				     32 + 1 + strlen (udata->origname));
		  _bfd_vms_output_begin_subrec (recwr, ETIR__C_STC_NOP_GBL);
		  _bfd_vms_output_long (recwr, (unsigned long) udata->lkindex);
		  _bfd_vms_output_long
		    (recwr, (unsigned long) section->target_index);
		  _bfd_vms_output_quad (recwr, rptr->address);
		  _bfd_vms_output_long (recwr, (unsigned long) 0x47ff041f);
		  _bfd_vms_output_long
		    (recwr, (unsigned long) section->target_index);
		  _bfd_vms_output_quad (recwr, rptr->addend);
		  _bfd_vms_output_counted (recwr, udata->origname);
		  _bfd_vms_output_end_subrec (recwr);
		  break;

		case ALPHA_R_BSR:
		  _bfd_error_handler (_("spurious ALPHA_R_BSR reloc"));
		  break;

		case ALPHA_R_LDA:
		  udata
		    = (struct evax_private_udata_struct *) rptr->sym_ptr_ptr;
		  etir_output_check (abfd, section, curr_addr,
				     32 + 1 + strlen (udata->origname));
		  _bfd_vms_output_begin_subrec (recwr, ETIR__C_STC_LDA_GBL);
		  _bfd_vms_output_long
		    (recwr, (unsigned long) udata->lkindex + 1);
		  _bfd_vms_output_long
		    (recwr, (unsigned long) section->target_index);
		  _bfd_vms_output_quad (recwr, rptr->address);
		  _bfd_vms_output_long (recwr, (unsigned long) 0x237B0000);
		  _bfd_vms_output_long
		    (recwr, (unsigned long) udata->bsym->section->target_index);
		  _bfd_vms_output_quad (recwr, rptr->addend);
		  _bfd_vms_output_counted (recwr, udata->origname);
		  _bfd_vms_output_end_subrec (recwr);
		  break;

		case ALPHA_R_BOH:
		  udata
		    = (struct evax_private_udata_struct *) rptr->sym_ptr_ptr;
		  etir_output_check (abfd, section, curr_addr,
				       32 + 1 + strlen (udata->origname));
		  _bfd_vms_output_begin_subrec (recwr, ETIR__C_STC_BOH_GBL);
		  _bfd_vms_output_long (recwr, (unsigned long) udata->lkindex);
		  _bfd_vms_output_long
		    (recwr, (unsigned long) section->target_index);
		  _bfd_vms_output_quad (recwr, rptr->address);
		  _bfd_vms_output_long (recwr, (unsigned long) 0xD3400000);
		  _bfd_vms_output_long
		    (recwr, (unsigned long) section->target_index);
		  _bfd_vms_output_quad (recwr, rptr->addend);
		  _bfd_vms_output_counted (recwr, udata->origname);
		  _bfd_vms_output_end_subrec (recwr);
		  break;

		default:
		  _bfd_error_handler (_("unhandled relocation %s"),
				      rptr->howto->name);
		  break;
		}

	      curr_data += size;
	      curr_addr += size;
	    } /* End of relocs loop.  */

	  if (!pass2_in_progress)
	    {
	      /* Output rest of section.  */
	      if (curr_addr > section->size)
		{
		  _bfd_error_handler (_("size error in section %pA"), section);
		  return false;
		}
	      size = section->size - curr_addr;
	      sto_imm (abfd, section, size, curr_data, curr_addr);
	      curr_data += size;
	      curr_addr += size;

	      if (pass2_needed)
		{
		  pass2_in_progress = 1;
		  goto new_pass;
		}
	    }
	}

      else /* (section->flags & SEC_RELOC) */
	sto_imm (abfd, section, section->size, section->contents, 0);

      end_etir_record (abfd);
    }

  _bfd_vms_output_alignment (recwr, 2);
  return true;
}

/* Write cached information into a file being written, at bfd_close.  */

static bool
alpha_vms_write_object_contents (bfd *abfd)
{
  vms_debug2 ((1, "vms_write_object_contents (%p)\n", abfd));

  if (abfd->flags & (EXEC_P | DYNAMIC))
    {
      return alpha_vms_write_exec (abfd);
    }
  else
    {
      if (abfd->section_count > 0)			/* we have sections */
	{
	  if (!_bfd_vms_write_ehdr (abfd))
	    return false;
	  if (!_bfd_vms_write_egsd (abfd))
	    return false;
	  if (!_bfd_vms_write_etir (abfd, EOBJ__C_ETIR))
	    return false;
	  if (!_bfd_vms_write_eeom (abfd))
	    return false;
	}
    }
  return true;
}

/* Debug stuff: nearest line.  */

#define SET_MODULE_PARSED(m) \
  do { if ((m)->name == NULL) (m)->name = ""; } while (0)
#define IS_MODULE_PARSED(m) ((m)->name != NULL)

/* Build a new module for the specified BFD.  */

static struct module *
new_module (bfd *abfd)
{
  struct module *module
    = (struct module *) bfd_zalloc (abfd, sizeof (struct module));
  module->file_table_count = 16; /* Arbitrary.  */
  module->file_table
    = bfd_zmalloc (module->file_table_count * sizeof (struct fileinfo));
  return module;
}

/* Parse debug info for a module and internalize it.  */

static bool
parse_module (bfd *abfd, struct module *module, unsigned char *ptr,
	      bfd_size_type length)
{
  unsigned char *maxptr = ptr + length;
  unsigned char *src_ptr, *pcl_ptr;
  unsigned int prev_linum = 0, curr_linenum = 0;
  bfd_vma prev_pc = 0, curr_pc = 0;
  struct srecinfo *curr_srec, *srec;
  struct lineinfo *curr_line, *line;
  struct funcinfo *funcinfo;

  /* Initialize tables with zero element.  */
  curr_srec = (struct srecinfo *) bfd_zalloc (abfd, sizeof (struct srecinfo));
  if (!curr_srec)
    return false;
  module->srec_table = curr_srec;

  curr_line = (struct lineinfo *) bfd_zalloc (abfd, sizeof (struct lineinfo));
  if (!curr_line)
    return false;
  module->line_table = curr_line;

  while (ptr + 3 < maxptr)
    {
      /* The first byte is not counted in the recorded length.  */
      int rec_length = bfd_getl16 (ptr) + 1;
      int rec_type = bfd_getl16 (ptr + 2);

      vms_debug2 ((2, "DST record: leng %d, type %d\n", rec_length, rec_type));

      if (rec_length > maxptr - ptr)
	break;
      if (rec_type == DST__K_MODEND)
	break;

      switch (rec_type)
	{
	case DST__K_MODBEG:
	  if (rec_length <= DST_S_B_MODBEG_NAME)
	    break;
	  module->name
	    = _bfd_vms_save_counted_string (abfd, ptr + DST_S_B_MODBEG_NAME,
					    rec_length - DST_S_B_MODBEG_NAME);

	  curr_pc = 0;
	  prev_pc = 0;
	  curr_linenum = 0;
	  prev_linum = 0;

	  vms_debug2 ((3, "module: %s\n", module->name));
	  break;

	case DST__K_MODEND:
	  break;

	case DST__K_RTNBEG:
	  if (rec_length <= DST_S_B_RTNBEG_NAME)
	    break;
	  funcinfo = (struct funcinfo *)
	    bfd_zalloc (abfd, sizeof (struct funcinfo));
	  if (!funcinfo)
	    return false;
	  funcinfo->name
	    = _bfd_vms_save_counted_string (abfd, ptr + DST_S_B_RTNBEG_NAME,
					    rec_length - DST_S_B_RTNBEG_NAME);
	  funcinfo->low = bfd_getl32 (ptr + DST_S_L_RTNBEG_ADDRESS);
	  funcinfo->next = module->func_table;
	  module->func_table = funcinfo;

	  vms_debug2 ((3, "routine: %s at 0x%lx\n",
		       funcinfo->name, (unsigned long) funcinfo->low));
	  break;

	case DST__K_RTNEND:
	  if (rec_length < DST_S_L_RTNEND_SIZE + 4)
	    break;
	  if (!module->func_table)
	    return false;
	  module->func_table->high = module->func_table->low
	    + bfd_getl32 (ptr + DST_S_L_RTNEND_SIZE) - 1;

	  if (module->func_table->high > module->high)
	    module->high = module->func_table->high;

	  vms_debug2 ((3, "end routine\n"));
	  break;

	case DST__K_PROLOG:
	  vms_debug2 ((3, "prologue\n"));
	  break;

	case DST__K_EPILOG:
	  vms_debug2 ((3, "epilog\n"));
	  break;

	case DST__K_BLKBEG:
	  vms_debug2 ((3, "block\n"));
	  break;

	case DST__K_BLKEND:
	  vms_debug2 ((3, "end block\n"));
	  break;

	case DST__K_SOURCE:
	  src_ptr = ptr + DST_S_C_SOURCE_HEADER_SIZE;

	  vms_debug2 ((3, "source info\n"));

	  while (src_ptr - ptr < rec_length)
	    {
	      int cmd = src_ptr[0], cmd_length, data;

	      switch (cmd)
		{
		case DST__K_SRC_DECLFILE:
		  if (src_ptr - ptr + DST_S_B_SRC_DF_LENGTH >= rec_length)
		    cmd_length = 0x10000;
		  else
		    cmd_length = src_ptr[DST_S_B_SRC_DF_LENGTH] + 2;
		  break;

		case DST__K_SRC_DEFLINES_B:
		  cmd_length = 2;
		  break;

		case DST__K_SRC_DEFLINES_W:
		  cmd_length = 3;
		  break;

		case DST__K_SRC_INCRLNUM_B:
		  cmd_length = 2;
		  break;

		case DST__K_SRC_SETFILE:
		  cmd_length = 3;
		  break;

		case DST__K_SRC_SETLNUM_L:
		  cmd_length = 5;
		  break;

		case DST__K_SRC_SETLNUM_W:
		  cmd_length = 3;
		  break;

		case DST__K_SRC_SETREC_L:
		  cmd_length = 5;
		  break;

		case DST__K_SRC_SETREC_W:
		  cmd_length = 3;
		  break;

		case DST__K_SRC_FORMFEED:
		  cmd_length = 1;
		  break;

		default:
		  cmd_length = 2;
		  break;
		}

	      if (src_ptr - ptr + cmd_length > rec_length)
		break;

	      switch (cmd)
		{
		case DST__K_SRC_DECLFILE:
		  {
		    unsigned int fileid
		      = bfd_getl16 (src_ptr + DST_S_W_SRC_DF_FILEID);
		    char *filename = _bfd_vms_save_counted_string
		      (abfd,
		       src_ptr + DST_S_B_SRC_DF_FILENAME,
		       ptr + rec_length - (src_ptr + DST_S_B_SRC_DF_FILENAME));

		    if (fileid >= module->file_table_count)
		      {
			unsigned int old_count = module->file_table_count;
			module->file_table_count += fileid;
			module->file_table
			  = bfd_realloc_or_free (module->file_table,
						 module->file_table_count
						 * sizeof (struct fileinfo));
			if (module->file_table == NULL)
			  return false;
			memset (module->file_table + old_count, 0,
				fileid * sizeof (struct fileinfo));
		      }

		    module->file_table [fileid].name = filename;
		    module->file_table [fileid].srec = 1;
		    vms_debug2 ((4, "DST_S_C_SRC_DECLFILE: %d, %s\n",
				 fileid, module->file_table [fileid].name));
		  }
		  break;

		case DST__K_SRC_DEFLINES_B:
		  /* Perform the association and set the next higher index
		     to the limit.  */
		  data = src_ptr[DST_S_B_SRC_UNSBYTE];
		  srec = (struct srecinfo *)
		    bfd_zalloc (abfd, sizeof (struct srecinfo));
		  srec->line = curr_srec->line + data;
		  srec->srec = curr_srec->srec + data;
		  srec->sfile = curr_srec->sfile;
		  curr_srec->next = srec;
		  curr_srec = srec;
		  vms_debug2 ((4, "DST_S_C_SRC_DEFLINES_B: %d\n", data));
		  break;

		case DST__K_SRC_DEFLINES_W:
		  /* Perform the association and set the next higher index
		     to the limit.  */
		  data = bfd_getl16 (src_ptr + DST_S_W_SRC_UNSWORD);
		  srec = (struct srecinfo *)
		    bfd_zalloc (abfd, sizeof (struct srecinfo));
		  srec->line = curr_srec->line + data;
		  srec->srec = curr_srec->srec + data,
		  srec->sfile = curr_srec->sfile;
		  curr_srec->next = srec;
		  curr_srec = srec;
		  vms_debug2 ((4, "DST_S_C_SRC_DEFLINES_W: %d\n", data));
		  break;

		case DST__K_SRC_INCRLNUM_B:
		  data = src_ptr[DST_S_B_SRC_UNSBYTE];
		  curr_srec->line += data;
		  vms_debug2 ((4, "DST_S_C_SRC_INCRLNUM_B: %d\n", data));
		  break;

		case DST__K_SRC_SETFILE:
		  data = bfd_getl16 (src_ptr + DST_S_W_SRC_UNSWORD);
		  if ((unsigned int) data < module->file_table_count)
		    {
		      curr_srec->sfile = data;
		      curr_srec->srec = module->file_table[data].srec;
		    }
		  vms_debug2 ((4, "DST_S_C_SRC_SETFILE: %d\n", data));
		  break;

		case DST__K_SRC_SETLNUM_L:
		  data = bfd_getl32 (src_ptr + DST_S_L_SRC_UNSLONG);
		  curr_srec->line = data;
		  vms_debug2 ((4, "DST_S_C_SRC_SETLNUM_L: %d\n", data));
		  break;

		case DST__K_SRC_SETLNUM_W:
		  data = bfd_getl16 (src_ptr + DST_S_W_SRC_UNSWORD);
		  curr_srec->line = data;
		  vms_debug2 ((4, "DST_S_C_SRC_SETLNUM_W: %d\n", data));
		  break;

		case DST__K_SRC_SETREC_L:
		  data = bfd_getl32 (src_ptr + DST_S_L_SRC_UNSLONG);
		  curr_srec->srec = data;
		  module->file_table[curr_srec->sfile].srec = data;
		  vms_debug2 ((4, "DST_S_C_SRC_SETREC_L: %d\n", data));
		  break;

		case DST__K_SRC_SETREC_W:
		  data = bfd_getl16 (src_ptr + DST_S_W_SRC_UNSWORD);
		  curr_srec->srec = data;
		  module->file_table[curr_srec->sfile].srec = data;
		  vms_debug2 ((4, "DST_S_C_SRC_SETREC_W: %d\n", data));
		  break;

		case DST__K_SRC_FORMFEED:
		  vms_debug2 ((4, "DST_S_C_SRC_FORMFEED\n"));
		  break;

		default:
		  _bfd_error_handler (_("unknown source command %d"),
				      cmd);
		  break;
		}

	      src_ptr += cmd_length;
	    }
	  break;

	case DST__K_LINE_NUM:
	  pcl_ptr = ptr + DST_S_C_LINE_NUM_HEADER_SIZE;

	  vms_debug2 ((3, "line info\n"));

	  while (pcl_ptr - ptr < rec_length)
	    {
	      /* The command byte is signed so we must sign-extend it.  */
	      int cmd = ((signed char *)pcl_ptr)[0], cmd_length, data;

	      switch (cmd)
		{
		case DST__K_DELTA_PC_W:
		  cmd_length = 3;
		  break;

		case DST__K_DELTA_PC_L:
		  cmd_length = 5;
		  break;

		case DST__K_INCR_LINUM:
		  cmd_length = 2;
		  break;

		case DST__K_INCR_LINUM_W:
		  cmd_length = 3;
		  break;

		case DST__K_INCR_LINUM_L:
		  cmd_length = 5;
		  break;

		case DST__K_SET_LINUM_INCR:
		  cmd_length = 2;
		  break;

		case DST__K_SET_LINUM_INCR_W:
		  cmd_length = 3;
		  break;

		case DST__K_RESET_LINUM_INCR:
		  cmd_length = 1;
		  break;

		case DST__K_BEG_STMT_MODE:
		  cmd_length = 1;
		  break;

		case DST__K_END_STMT_MODE:
		  cmd_length = 1;
		  break;

		case DST__K_SET_LINUM_B:
		  cmd_length = 2;
		  break;

		case DST__K_SET_LINUM:
		  cmd_length = 3;
		  break;

		case DST__K_SET_LINUM_L:
		  cmd_length = 5;
		  break;

		case DST__K_SET_PC:
		  cmd_length = 2;
		  break;

		case DST__K_SET_PC_W:
		  cmd_length = 3;
		  break;

		case DST__K_SET_PC_L:
		  cmd_length = 5;
		  break;

		case DST__K_SET_STMTNUM:
		  cmd_length = 2;
		  break;

		case DST__K_TERM:
		  cmd_length = 2;
		  break;

		case DST__K_TERM_W:
		  cmd_length = 3;
		  break;

		case DST__K_TERM_L:
		  cmd_length = 5;
		  break;

		case DST__K_SET_ABS_PC:
		  cmd_length = 5;
		  break;

		default:
		  if (cmd <= 0)
		    cmd_length = 1;
		  else
		    cmd_length = 2;
		  break;
		}

	      if (pcl_ptr - ptr + cmd_length > rec_length)
		break;

	      switch (cmd)
		{
		case DST__K_DELTA_PC_W:
		  data = bfd_getl16 (pcl_ptr + DST_S_W_PCLINE_UNSWORD);
		  curr_pc += data;
		  curr_linenum += 1;
		  vms_debug2 ((4, "DST__K_DELTA_PC_W: %d\n", data));
		  break;

		case DST__K_DELTA_PC_L:
		  data = bfd_getl32 (pcl_ptr + DST_S_L_PCLINE_UNSLONG);
		  curr_pc += data;
		  curr_linenum += 1;
		  vms_debug2 ((4, "DST__K_DELTA_PC_L: %d\n", data));
		  break;

		case DST__K_INCR_LINUM:
		  data = pcl_ptr[DST_S_B_PCLINE_UNSBYTE];
		  curr_linenum += data;
		  vms_debug2 ((4, "DST__K_INCR_LINUM: %d\n", data));
		  break;

		case DST__K_INCR_LINUM_W:
		  data = bfd_getl16 (pcl_ptr + DST_S_W_PCLINE_UNSWORD);
		  curr_linenum += data;
		  vms_debug2 ((4, "DST__K_INCR_LINUM_W: %d\n", data));
		  break;

		case DST__K_INCR_LINUM_L:
		  data = bfd_getl32 (pcl_ptr + DST_S_L_PCLINE_UNSLONG);
		  curr_linenum += data;
		  vms_debug2 ((4, "DST__K_INCR_LINUM_L: %d\n", data));
		  break;

		case DST__K_SET_LINUM_INCR:
		  _bfd_error_handler
		    (_("%s not implemented"), "DST__K_SET_LINUM_INCR");
		  break;

		case DST__K_SET_LINUM_INCR_W:
		  _bfd_error_handler
		    (_("%s not implemented"), "DST__K_SET_LINUM_INCR_W");
		  break;

		case DST__K_RESET_LINUM_INCR:
		  _bfd_error_handler
		    (_("%s not implemented"), "DST__K_RESET_LINUM_INCR");
		  break;

		case DST__K_BEG_STMT_MODE:
		  _bfd_error_handler
		    (_("%s not implemented"), "DST__K_BEG_STMT_MODE");
		  break;

		case DST__K_END_STMT_MODE:
		  _bfd_error_handler
		    (_("%s not implemented"), "DST__K_END_STMT_MODE");
		  break;

		case DST__K_SET_LINUM_B:
		  data = pcl_ptr[DST_S_B_PCLINE_UNSBYTE];
		  curr_linenum = data;
		  vms_debug2 ((4, "DST__K_SET_LINUM_B: %d\n", data));
		  break;

		case DST__K_SET_LINUM:
		  data = bfd_getl16 (pcl_ptr + DST_S_W_PCLINE_UNSWORD);
		  curr_linenum = data;
		  vms_debug2 ((4, "DST__K_SET_LINE_NUM: %d\n", data));
		  break;

		case DST__K_SET_LINUM_L:
		  data = bfd_getl32 (pcl_ptr + DST_S_L_PCLINE_UNSLONG);
		  curr_linenum = data;
		  vms_debug2 ((4, "DST__K_SET_LINUM_L: %d\n", data));
		  break;

		case DST__K_SET_PC:
		  _bfd_error_handler
		    (_("%s not implemented"), "DST__K_SET_PC");
		  break;

		case DST__K_SET_PC_W:
		  _bfd_error_handler
		    (_("%s not implemented"), "DST__K_SET_PC_W");
		  break;

		case DST__K_SET_PC_L:
		  _bfd_error_handler
		    (_("%s not implemented"), "DST__K_SET_PC_L");
		  break;

		case DST__K_SET_STMTNUM:
		  _bfd_error_handler
		    (_("%s not implemented"), "DST__K_SET_STMTNUM");
		  break;

		case DST__K_TERM:
		  data = pcl_ptr[DST_S_B_PCLINE_UNSBYTE];
		  curr_pc += data;
		  vms_debug2 ((4, "DST__K_TERM: %d\n", data));
		  break;

		case DST__K_TERM_W:
		  data = bfd_getl16 (pcl_ptr + DST_S_W_PCLINE_UNSWORD);
		  curr_pc += data;
		  vms_debug2 ((4, "DST__K_TERM_W: %d\n", data));
		  break;

		case DST__K_TERM_L:
		  data = bfd_getl32 (pcl_ptr + DST_S_L_PCLINE_UNSLONG);
		  curr_pc += data;
		  vms_debug2 ((4, "DST__K_TERM_L: %d\n", data));
		  break;

		case DST__K_SET_ABS_PC:
		  data = bfd_getl32 (pcl_ptr + DST_S_L_PCLINE_UNSLONG);
		  curr_pc = data;
		  vms_debug2 ((4, "DST__K_SET_ABS_PC: 0x%x\n", data));
		  break;

		default:
		  if (cmd <= 0)
		    {
		      curr_pc -= cmd;
		      curr_linenum += 1;
		      vms_debug2 ((4, "bump pc to 0x%lx and line to %d\n",
				   (unsigned long)curr_pc, curr_linenum));
		    }
		  else
		    _bfd_error_handler (_("unknown line command %d"), cmd);
		  break;
		}

	      if ((curr_linenum != prev_linum && curr_pc != prev_pc)
		  || cmd <= 0
		  || cmd == DST__K_DELTA_PC_L
		  || cmd == DST__K_DELTA_PC_W)
		{
		  line = (struct lineinfo *)
		    bfd_zalloc (abfd, sizeof (struct lineinfo));
		  line->address = curr_pc;
		  line->line = curr_linenum;

		  curr_line->next = line;
		  curr_line = line;

		  prev_linum = curr_linenum;
		  prev_pc = curr_pc;
		  vms_debug2 ((4, "-> correlate pc 0x%lx with line %d\n",
			       (unsigned long)curr_pc, curr_linenum));
		}

	      pcl_ptr += cmd_length;
	    }
	  break;

	case 0x17: /* Undocumented type used by DEC C to declare equates.  */
	  vms_debug2 ((3, "undocumented type 0x17\n"));
	  break;

	default:
	  vms_debug2 ((3, "ignoring record\n"));
	  break;

	}

      ptr += rec_length;
    }

  /* Finalize tables with EOL marker.  */
  srec = (struct srecinfo *) bfd_zalloc (abfd, sizeof (struct srecinfo));
  srec->line = (unsigned int) -1;
  srec->srec = (unsigned int) -1;
  curr_srec->next = srec;

  line = (struct lineinfo *) bfd_zalloc (abfd, sizeof (struct lineinfo));
  line->line = (unsigned int) -1;
  line->address = (bfd_vma) -1;
  curr_line->next = line;

  /* Advertise that this module has been parsed.  This is needed
     because parsing can be either performed at module creation
     or deferred until debug info is consumed.  */
  SET_MODULE_PARSED (module);
  return true;
}

/* Build the list of modules for the specified BFD.  */

static struct module *
build_module_list (bfd *abfd)
{
  struct module *module, *list = NULL;
  asection *dmt;

  if ((dmt = bfd_get_section_by_name (abfd, "$DMT$")))
    {
      /* We have a DMT section so this must be an image.  Parse the
	 section and build the list of modules.  This is sufficient
	 since we can compute the start address and the end address
	 of every module from the section contents.  */
      bfd_size_type size = bfd_section_size (dmt);
      unsigned char *buf, *ptr, *end;

      if (! bfd_malloc_and_get_section (abfd, dmt, &buf))
	return NULL;

      vms_debug2 ((2, "DMT\n"));

      ptr = buf;
      end = ptr + size;
      while (end - ptr >= DBG_S_C_DMT_HEADER_SIZE)
	{
	  /* Each header declares a module with its start offset and size
	     of debug info in the DST section, as well as the count of
	     program sections (i.e. address spans) it contains.  */
	  unsigned int modbeg = bfd_getl32 (ptr + DBG_S_L_DMT_MODBEG);
	  unsigned int msize = bfd_getl32 (ptr + DBG_S_L_DST_SIZE);
	  int count = bfd_getl16 (ptr + DBG_S_W_DMT_PSECT_COUNT);
	  ptr += DBG_S_C_DMT_HEADER_SIZE;

	  vms_debug2 ((3, "module: modbeg = %u, size = %u, count = %d\n",
		       modbeg, msize, count));

	  /* We create a 'module' structure for each program section since
	     we only support contiguous addresses in a 'module' structure.
	     As a consequence, the actual debug info in the DST section is
	     shared and can be parsed multiple times; that doesn't seem to
	     cause problems in practice.  */
	  while (count-- > 0 && end - ptr >= DBG_S_C_DMT_PSECT_SIZE)
	    {
	      unsigned int start = bfd_getl32 (ptr + DBG_S_L_DMT_PSECT_START);
	      unsigned int length = bfd_getl32 (ptr + DBG_S_L_DMT_PSECT_LENGTH);
	      module = new_module (abfd);
	      module->modbeg = modbeg;
	      module->size = msize;
	      module->low = start;
	      module->high = start + length;
	      module->next = list;
	      list = module;
	      ptr += DBG_S_C_DMT_PSECT_SIZE;

	      vms_debug2 ((4, "section: start = 0x%x, length = %u\n",
			   start, length));
	    }
	}
      free (buf);
    }
  else
    {
      /* We don't have a DMT section so this must be an object.  Parse
	 the module right now in order to compute its start address and
	 end address.  */
      void *dst = PRIV (dst_section)->contents;

      if (dst == NULL)
	return NULL;

      module = new_module (abfd);
      if (!parse_module (abfd, module, PRIV (dst_section)->contents,
			 PRIV (dst_section)->size))
	return NULL;
      list = module;
    }

  return list;
}

/* Calculate and return the name of the source file and the line nearest
   to the wanted location in the specified module.  */

static bool
module_find_nearest_line (bfd *abfd, struct module *module, bfd_vma addr,
			  const char **file, const char **func,
			  unsigned int *line)
{
  struct funcinfo *funcinfo;
  struct lineinfo *lineinfo;
  struct srecinfo *srecinfo;
  bool ret = false;

  /* Parse this module if that was not done at module creation.  */
  if (! IS_MODULE_PARSED (module))
    {
      unsigned int size = module->size;
      unsigned int modbeg = PRIV (dst_section)->filepos + module->modbeg;
      unsigned char *buffer;

      if (bfd_seek (abfd, modbeg, SEEK_SET) != 0
	  || (buffer = _bfd_malloc_and_read (abfd, size, size)) == NULL)
	{
	  bfd_set_error (bfd_error_no_debug_section);
	  return false;
	}

      ret = parse_module (abfd, module, buffer, size);
      free (buffer);
      if (!ret)
	return ret;
    }

  /* Find out the function (if any) that contains the address.  */
  for (funcinfo = module->func_table; funcinfo; funcinfo = funcinfo->next)
    if (addr >= funcinfo->low && addr <= funcinfo->high)
      {
	*func = funcinfo->name;
	ret = true;
	break;
      }

  /* Find out the source file and the line nearest to the address.  */
  for (lineinfo = module->line_table; lineinfo; lineinfo = lineinfo->next)
    if (lineinfo->next && addr < lineinfo->next->address)
      {
	for (srecinfo = module->srec_table; srecinfo; srecinfo = srecinfo->next)
	  if (srecinfo->next && lineinfo->line < srecinfo->next->line)
	    {
	      if (srecinfo->sfile > 0)
		{
		  *file = module->file_table[srecinfo->sfile].name;
		  *line = srecinfo->srec + lineinfo->line - srecinfo->line;
		}
	      else
		{
		  *file = module->name;
		  *line = lineinfo->line;
		}
	      return true;
	    }

	break;
      }

  return ret;
}

/* Provided a BFD, a section and an offset into the section, calculate and
   return the name of the source file and the line nearest to the wanted
   location.  */

static bool
_bfd_vms_find_nearest_line (bfd *abfd,
			    asymbol **symbols ATTRIBUTE_UNUSED,
			    asection *section,
			    bfd_vma offset,
			    const char **file,
			    const char **func,
			    unsigned int *line,
			    unsigned int *discriminator)
{
  struct module *module;

  /* What address are we looking for?  */
  bfd_vma addr = section->vma + offset;

  *file = NULL;
  *func = NULL;
  *line = 0;
  if (discriminator)
    *discriminator = 0;

  /* We can't do anything if there is no DST (debug symbol table).  */
  if (PRIV (dst_section) == NULL)
    return false;

  /* Create the module list - if not already done.  */
  if (PRIV (modules) == NULL)
    {
      PRIV (modules) = build_module_list (abfd);
      if (PRIV (modules) == NULL)
	return false;
    }

  for (module = PRIV (modules); module; module = module->next)
    if (addr >= module->low && addr <= module->high)
      return module_find_nearest_line (abfd, module, addr, file, func, line);

  return false;
}

/* Canonicalizations.  */
/* Set name, value, section and flags of SYM from E.  */

static bool
alpha_vms_convert_symbol (bfd *abfd, struct vms_symbol_entry *e, asymbol *sym)
{
  flagword flags;
  symvalue value;
  asection *sec;
  const char *name;

  name = e->name;
  value = 0;
  flags = BSF_NO_FLAGS;
  sec = NULL;

  switch (e->typ)
    {
    case EGSD__C_SYM:
      if (e->flags & EGSY__V_WEAK)
	flags |= BSF_WEAK;

      if (e->flags & EGSY__V_DEF)
	{
	  /* Symbol definition.  */
	  flags |= BSF_GLOBAL;
	  if (e->flags & EGSY__V_NORM)
	    flags |= BSF_FUNCTION;
	  value = e->value;
	  sec = e->section;
	}
      else
	{
	  /* Symbol reference.  */
	  sec = bfd_und_section_ptr;
	}
      break;

    case EGSD__C_SYMG:
      /* A universal symbol is by definition global...  */
      flags |= BSF_GLOBAL;

      /* ...and dynamic in shared libraries.  */
      if (abfd->flags & DYNAMIC)
	flags |= BSF_DYNAMIC;

      if (e->flags & EGSY__V_WEAK)
	flags |= BSF_WEAK;

      if (!(e->flags & EGSY__V_DEF))
	abort ();

      if (e->flags & EGSY__V_NORM)
	flags |= BSF_FUNCTION;

      value = e->value;
      /* sec = e->section; */
      sec = bfd_abs_section_ptr;
      break;

    default:
      return false;
    }

  sym->name = name;
  sym->section = sec;
  sym->flags = flags;
  sym->value = value;
  return true;
}


/* Return the number of bytes required to store a vector of pointers
   to asymbols for all the symbols in the BFD abfd, including a
   terminal NULL pointer. If there are no symbols in the BFD,
   then return 0.  If an error occurs, return -1.  */

static long
alpha_vms_get_symtab_upper_bound (bfd *abfd)
{
  vms_debug2 ((1, "alpha_vms_get_symtab_upper_bound (%p), %d symbols\n",
	       abfd, PRIV (gsd_sym_count)));

  return (PRIV (gsd_sym_count) + 1) * sizeof (asymbol *);
}

/* Read the symbols from the BFD abfd, and fills in the vector
   location with pointers to the symbols and a trailing NULL.

   Return number of symbols read.   */

static long
alpha_vms_canonicalize_symtab (bfd *abfd, asymbol **symbols)
{
  unsigned int i;

  vms_debug2 ((1, "alpha_vms_canonicalize_symtab (%p, <ret>)\n", abfd));

  if (PRIV (csymbols) == NULL)
    {
      PRIV (csymbols) = (asymbol **) bfd_alloc
	(abfd, PRIV (gsd_sym_count) * sizeof (asymbol *));

      /* Traverse table and fill symbols vector.  */
      for (i = 0; i < PRIV (gsd_sym_count); i++)
	{
	  struct vms_symbol_entry *e = PRIV (syms)[i];
	  asymbol *sym;

	  sym = bfd_make_empty_symbol (abfd);
	  if (sym == NULL || !alpha_vms_convert_symbol (abfd, e, sym))
	    {
	      bfd_release (abfd, PRIV (csymbols));
	      PRIV (csymbols) = NULL;
	      return -1;
	    }

	  PRIV (csymbols)[i] = sym;
	}
    }

  if (symbols != NULL)
    {
      for (i = 0; i < PRIV (gsd_sym_count); i++)
	symbols[i] = PRIV (csymbols)[i];
      symbols[i] = NULL;
    }

  return PRIV (gsd_sym_count);
}

/* Read and convert relocations from ETIR.  We do it once for all sections.  */

static bool
alpha_vms_slurp_relocs (bfd *abfd)
{
  int cur_psect = -1;

  vms_debug2 ((3, "alpha_vms_slurp_relocs\n"));

  /* We slurp relocs only once, for all sections.  */
  if (PRIV (reloc_done) != 0)
    return PRIV (reloc_done) == 1;

  if (alpha_vms_canonicalize_symtab (abfd, NULL) < 0)
    goto fail;

  if (bfd_seek (abfd, 0, SEEK_SET) != 0)
    goto fail;

  while (1)
    {
      unsigned char *begin;
      unsigned char *end;
      unsigned char *ptr;
      bfd_reloc_code_real_type reloc_code;
      int type;
      bfd_vma vaddr = 0;

      int length;

      bfd_vma cur_address;
      int cur_psidx = -1;
      unsigned char *cur_sym = NULL;
      int prev_cmd = -1;
      bfd_vma cur_addend = 0;

      /* Skip non-ETIR records.  */
      type = _bfd_vms_get_object_record (abfd);
      if (type < 0)
	goto fail;
      if (type == EOBJ__C_EEOM)
	break;
      if (type != EOBJ__C_ETIR)
	continue;

      begin = PRIV (recrd.rec) + 4;
      end = PRIV (recrd.rec) + PRIV (recrd.rec_size);

      for (ptr = begin; ptr + 4 <= end; ptr += length)
	{
	  int cmd;

	  cmd = bfd_getl16 (ptr);
	  length = bfd_getl16 (ptr + 2);
	  if (length < 4 || length > end - ptr)
	    {
	    bad_rec:
	      _bfd_error_handler (_("corrupt reloc record"));
	      goto fail;
	    }

	  cur_address = vaddr;

	  vms_debug2 ((4, "alpha_vms_slurp_relocs: etir %s\n",
		       _bfd_vms_etir_name (cmd)));

	  switch (cmd)
	    {
	    case ETIR__C_STA_GBL: /* ALPHA_R_REFLONG und_section, step 1 */
				  /* ALPHA_R_REFQUAD und_section, step 1 */
	      cur_sym = ptr + 4;
	      prev_cmd = cmd;
	      continue;

	    case ETIR__C_STA_PQ: /* ALPHA_R_REF{LONG|QUAD}, others part 1 */
	      if (length < 16)
		goto bad_rec;
	      cur_psidx = bfd_getl32 (ptr + 4);
	      cur_addend = bfd_getl64 (ptr + 8);
	      prev_cmd = cmd;
	      continue;

	    case ETIR__C_CTL_SETRB:
	      if (prev_cmd != ETIR__C_STA_PQ)
		{
		  _bfd_error_handler
		    /* xgettext:c-format */
		    (_("unknown reloc %s + %s"), _bfd_vms_etir_name (prev_cmd),
		     _bfd_vms_etir_name (cmd));
		  goto fail;
		}
	      cur_psect = cur_psidx;
	      vaddr = cur_addend;
	      cur_psidx = -1;
	      cur_addend = 0;
	      continue;

	    case ETIR__C_STA_LW: /* ALPHA_R_REFLONG abs_section, step 1 */
				 /* ALPHA_R_REFLONG und_section, step 2 */
	      if (prev_cmd != -1)
		{
		  if (prev_cmd != ETIR__C_STA_GBL)
		    {
		      _bfd_error_handler
			/* xgettext:c-format */
			(_("unknown reloc %s + %s"), _bfd_vms_etir_name (cmd),
			 _bfd_vms_etir_name (ETIR__C_STA_LW));
		      goto fail;
		    }
		}
	      if (length < 8)
		goto bad_rec;
	      cur_addend = bfd_getl32 (ptr + 4);
	      prev_cmd = cmd;
	      continue;

	    case ETIR__C_STA_QW: /* ALPHA_R_REFQUAD abs_section, step 1 */
				 /* ALPHA_R_REFQUAD und_section, step 2 */
	      if (prev_cmd != -1 && prev_cmd != ETIR__C_STA_GBL)
		{
		  _bfd_error_handler
		    /* xgettext:c-format */
		    (_("unknown reloc %s + %s"), _bfd_vms_etir_name (cmd),
		     _bfd_vms_etir_name (ETIR__C_STA_QW));
		  goto fail;
		}
	      if (length < 12)
		goto bad_rec;
	      cur_addend = bfd_getl64 (ptr + 4);
	      prev_cmd = cmd;
	      continue;

	    case ETIR__C_STO_LW: /* ALPHA_R_REFLONG und_section, step 4 */
				 /* ALPHA_R_REFLONG abs_section, step 2 */
				 /* ALPHA_R_REFLONG others, step 2 */
	      if (prev_cmd != ETIR__C_OPR_ADD
		  && prev_cmd != ETIR__C_STA_LW
		  && prev_cmd != ETIR__C_STA_PQ)
		{
		  /* xgettext:c-format */
		  _bfd_error_handler (_("unknown reloc %s + %s"),
				      _bfd_vms_etir_name (prev_cmd),
				      _bfd_vms_etir_name (ETIR__C_STO_LW));
		  goto fail;
		}
	      reloc_code = BFD_RELOC_32;
	      break;

	    case ETIR__C_STO_QW: /* ALPHA_R_REFQUAD und_section, step 4 */
				 /* ALPHA_R_REFQUAD abs_section, step 2 */
	      if (prev_cmd != ETIR__C_OPR_ADD && prev_cmd != ETIR__C_STA_QW)
		{
		  /* xgettext:c-format */
		  _bfd_error_handler (_("unknown reloc %s + %s"),
				      _bfd_vms_etir_name (prev_cmd),
				      _bfd_vms_etir_name (ETIR__C_STO_QW));
		  goto fail;
		}
	      reloc_code = BFD_RELOC_64;
	      break;

	    case ETIR__C_STO_OFF: /* ALPHA_R_REFQUAD others, step 2 */
	      if (prev_cmd != ETIR__C_STA_PQ)
		{
		  /* xgettext:c-format */
		  _bfd_error_handler (_("unknown reloc %s + %s"),
				      _bfd_vms_etir_name (prev_cmd),
				      _bfd_vms_etir_name (ETIR__C_STO_OFF));
		  goto fail;
		}
	      reloc_code = BFD_RELOC_64;
	      break;

	    case ETIR__C_OPR_ADD: /* ALPHA_R_REFLONG und_section, step 3 */
				  /* ALPHA_R_REFQUAD und_section, step 3 */
	      if (prev_cmd != ETIR__C_STA_LW && prev_cmd != ETIR__C_STA_QW)
		{
		  /* xgettext:c-format */
		  _bfd_error_handler (_("unknown reloc %s + %s"),
				      _bfd_vms_etir_name (prev_cmd),
				      _bfd_vms_etir_name (ETIR__C_OPR_ADD));
		  goto fail;
		}
	      prev_cmd = ETIR__C_OPR_ADD;
	      continue;

	    case ETIR__C_STO_CA: /* ALPHA_R_CODEADDR */
	      reloc_code = BFD_RELOC_ALPHA_CODEADDR;
	      cur_sym = ptr + 4;
	      break;

	    case ETIR__C_STO_GBL: /* ALPHA_R_REFQUAD und_section */
	      reloc_code = BFD_RELOC_64;
	      cur_sym = ptr + 4;
	      break;

	    case ETIR__C_STO_GBL_LW: /* ALPHA_R_REFLONG und_section */
	      reloc_code = BFD_RELOC_32;
	      cur_sym = ptr + 4;
	      break;

	    case ETIR__C_STC_LP_PSB: /* ALPHA_R_LINKAGE */
	      reloc_code = BFD_RELOC_ALPHA_LINKAGE;
	      cur_sym = ptr + 8;
	      break;

	    case ETIR__C_STC_NOP_GBL: /* ALPHA_R_NOP */
	      reloc_code = BFD_RELOC_ALPHA_NOP;
	      goto call_reloc;

	    case ETIR__C_STC_BSR_GBL: /* ALPHA_R_BSR */
	      reloc_code = BFD_RELOC_ALPHA_BSR;
	      goto call_reloc;

	    case ETIR__C_STC_LDA_GBL: /* ALPHA_R_LDA */
	      reloc_code = BFD_RELOC_ALPHA_LDA;
	      goto call_reloc;

	    case ETIR__C_STC_BOH_GBL: /* ALPHA_R_BOH */
	      reloc_code = BFD_RELOC_ALPHA_BOH;
	      goto call_reloc;

	    call_reloc:
	      if (length < 36)
		goto bad_rec;
	      cur_sym = ptr + 4 + 32;
	      cur_address = bfd_getl64 (ptr + 4 + 8);
	      cur_addend = bfd_getl64 (ptr + 4 + 24);
	      break;

	    case ETIR__C_STO_IMM:
	      if (length < 8)
		goto bad_rec;
	      vaddr += bfd_getl32 (ptr + 4);
	      continue;

	    default:
	      _bfd_error_handler (_("unknown reloc %s"),
				  _bfd_vms_etir_name (cmd));
	      goto fail;
	    }

	  {
	    asection *sec;
	    struct vms_section_data_struct *vms_sec;
	    arelent *reloc;
	    bfd_size_type size;

	    /* Get section to which the relocation applies.  */
	    if (cur_psect < 0 || cur_psect > (int)PRIV (section_count))
	      {
		_bfd_error_handler (_("invalid section index in ETIR"));
		goto fail;
	      }

	    if (PRIV (sections) == NULL)
	      goto fail;
	    sec = PRIV (sections)[cur_psect];
	    if (sec == bfd_abs_section_ptr)
	      {
		_bfd_error_handler (_("relocation for non-REL psect"));
		goto fail;
	      }

	    vms_sec = vms_section_data (sec);

	    /* Allocate a reloc entry.  */
	    if (sec->reloc_count >= vms_sec->reloc_max)
	      {
		if (vms_sec->reloc_max == 0)
		  {
		    vms_sec->reloc_max = 64;
		    sec->relocation = bfd_zmalloc
		      (vms_sec->reloc_max * sizeof (arelent));
		  }
		else
		  {
		    vms_sec->reloc_max *= 2;
		    sec->relocation = bfd_realloc_or_free
		      (sec->relocation, vms_sec->reloc_max * sizeof (arelent));
		    if (sec->relocation == NULL)
		      goto fail;
		  }
	      }
	    reloc = &sec->relocation[sec->reloc_count];
	    sec->reloc_count++;

	    reloc->howto = bfd_reloc_type_lookup (abfd, reloc_code);

	    if (cur_sym != NULL)
	      {
		unsigned int j;
		int symlen;
		asymbol **sym;

		/* Linear search.  */
		if (end - cur_sym < 1)
		  goto bad_rec;
		symlen = *cur_sym;
		cur_sym++;
		if (end - cur_sym < symlen)
		  goto bad_rec;
		sym = NULL;

		for (j = 0; j < PRIV (gsd_sym_count); j++)
		  if (PRIV (syms)[j]->namelen == symlen
		      && memcmp (PRIV (syms)[j]->name, cur_sym, symlen) == 0)
		    {
		      sym = &PRIV (csymbols)[j];
		      break;
		    }
		if (sym == NULL)
		  {
		    _bfd_error_handler (_("unknown symbol in command %s"),
					_bfd_vms_etir_name (cmd));
		    reloc->sym_ptr_ptr = NULL;
		  }
		else
		  reloc->sym_ptr_ptr = sym;
	      }
	    else if (cur_psidx >= 0)
	      {
		if (PRIV (sections) == NULL || cur_psidx >= (int) PRIV (section_count))
		  goto fail;
		reloc->sym_ptr_ptr =
		  PRIV (sections)[cur_psidx]->symbol_ptr_ptr;
	      }
	    else
	      reloc->sym_ptr_ptr = NULL;

	    reloc->address = cur_address;
	    reloc->addend = cur_addend;

	    if (reloc_code == ALPHA_R_LINKAGE)
	      size = 16;
	    else
	      size = bfd_get_reloc_size (reloc->howto);
	    vaddr += size;
	  }

	  cur_addend = 0;
	  prev_cmd = -1;
	  cur_sym = NULL;
	  cur_psidx = -1;
	}
    }
  vms_debug2 ((3, "alpha_vms_slurp_relocs: result = true\n"));
  PRIV (reloc_done) = 1;
  return true;

fail:
  PRIV (reloc_done) = -1;
  return false;
}

/* Return the number of bytes required to store the relocation
   information associated with the given section.  */

static long
alpha_vms_get_reloc_upper_bound (bfd *abfd ATTRIBUTE_UNUSED, asection *section)
{
  if (!alpha_vms_slurp_relocs (abfd))
    return -1;

  return (section->reloc_count + 1L) * sizeof (arelent *);
}

/* Convert relocations from VMS (external) form into BFD internal
   form.  Return the number of relocations.  */

static long
alpha_vms_canonicalize_reloc (bfd *abfd, asection *section, arelent **relptr,
			      asymbol **symbols ATTRIBUTE_UNUSED)
{
  arelent *tblptr;
  int count;

  if (!alpha_vms_slurp_relocs (abfd))
    return -1;

  count = section->reloc_count;
  tblptr = section->relocation;

  while (count--)
    *relptr++ = tblptr++;

  *relptr = (arelent *) NULL;
  return section->reloc_count;
}

/* Install a new set of internal relocs.  */

#define alpha_vms_set_reloc _bfd_generic_set_reloc


/* This is just copied from ecoff-alpha, needs to be fixed probably.  */

/* How to process the various reloc types.  */

static bfd_reloc_status_type
reloc_nil (bfd * abfd ATTRIBUTE_UNUSED,
	   arelent *reloc ATTRIBUTE_UNUSED,
	   asymbol *sym ATTRIBUTE_UNUSED,
	   void * data ATTRIBUTE_UNUSED,
	   asection *sec ATTRIBUTE_UNUSED,
	   bfd *output_bfd ATTRIBUTE_UNUSED,
	   char **error_message ATTRIBUTE_UNUSED)
{
#if VMS_DEBUG
  vms_debug (1, "reloc_nil (abfd %p, output_bfd %p)\n", abfd, output_bfd);
  vms_debug (2, "In section %s, symbol %s\n",
	sec->name, sym->name);
  vms_debug (2, "reloc sym %s, addr %08lx, addend %08lx, reloc is a %s\n",
		reloc->sym_ptr_ptr[0]->name,
		(unsigned long)reloc->address,
		(unsigned long)reloc->addend, reloc->howto->name);
  vms_debug (2, "data at %p\n", data);
  /*  _bfd_hexdump (2, data, bfd_get_reloc_size (reloc->howto), 0); */
#endif

  return bfd_reloc_ok;
}

/* In case we're on a 32-bit machine, construct a 64-bit "-1" value
   from smaller values.  Start with zero, widen, *then* decrement.  */
#define MINUS_ONE	(((bfd_vma)0) - 1)

static reloc_howto_type alpha_howto_table[] =
{
  HOWTO (ALPHA_R_IGNORE,	/* Type.  */
	 0,			/* Rightshift.  */
	 1,			/* Size.  */
	 8,			/* Bitsize.  */
	 true,			/* PC relative.  */
	 0,			/* Bitpos.  */
	 complain_overflow_dont,/* Complain_on_overflow.  */
	 reloc_nil,		/* Special_function.  */
	 "IGNORE",		/* Name.  */
	 true,			/* Partial_inplace.  */
	 0,			/* Source mask */
	 0,			/* Dest mask.  */
	 true),			/* PC rel offset.  */

  /* A 64 bit reference to a symbol.  */
  HOWTO (ALPHA_R_REFQUAD,	/* Type.  */
	 0,			/* Rightshift.  */
	 8,			/* Size.  */
	 64,			/* Bitsize.  */
	 false,			/* PC relative.  */
	 0,			/* Bitpos.  */
	 complain_overflow_bitfield, /* Complain_on_overflow.  */
	 reloc_nil,		/* Special_function.  */
	 "REFQUAD",		/* Name.  */
	 true,			/* Partial_inplace.  */
	 MINUS_ONE,		/* Source mask.  */
	 MINUS_ONE,		/* Dest mask.  */
	 false),		/* PC rel offset.  */

  /* A 21 bit branch.  The native assembler generates these for
     branches within the text segment, and also fills in the PC
     relative offset in the instruction.  */
  HOWTO (ALPHA_R_BRADDR,	/* Type.  */
	 2,			/* Rightshift.  */
	 4,			/* Size.  */
	 21,			/* Bitsize.  */
	 true,			/* PC relative.  */
	 0,			/* Bitpos.  */
	 complain_overflow_signed, /* Complain_on_overflow.  */
	 reloc_nil,		/* Special_function.  */
	 "BRADDR",		/* Name.  */
	 true,			/* Partial_inplace.  */
	 0x1fffff,		/* Source mask.  */
	 0x1fffff,		/* Dest mask.  */
	 false),		/* PC rel offset.  */

  /* A hint for a jump to a register.  */
  HOWTO (ALPHA_R_HINT,		/* Type.  */
	 2,			/* Rightshift.  */
	 2,			/* Size.  */
	 14,			/* Bitsize.  */
	 true,			/* PC relative.  */
	 0,			/* Bitpos.  */
	 complain_overflow_dont,/* Complain_on_overflow.  */
	 reloc_nil,		/* Special_function.  */
	 "HINT",		/* Name.  */
	 true,			/* Partial_inplace.  */
	 0x3fff,		/* Source mask.  */
	 0x3fff,		/* Dest mask.  */
	 false),		/* PC rel offset.  */

  /* 16 bit PC relative offset.  */
  HOWTO (ALPHA_R_SREL16,	/* Type.  */
	 0,			/* Rightshift.  */
	 2,			/* Size.  */
	 16,			/* Bitsize.  */
	 true,			/* PC relative.  */
	 0,			/* Bitpos.  */
	 complain_overflow_signed, /* Complain_on_overflow.  */
	 reloc_nil,		/* Special_function.  */
	 "SREL16",		/* Name.  */
	 true,			/* Partial_inplace.  */
	 0xffff,		/* Source mask.  */
	 0xffff,		/* Dest mask.  */
	 false),		/* PC rel offset.  */

  /* 32 bit PC relative offset.  */
  HOWTO (ALPHA_R_SREL32,	/* Type.  */
	 0,			/* Rightshift.  */
	 4,			/* Size.  */
	 32,			/* Bitsize.  */
	 true,			/* PC relative.  */
	 0,			/* Bitpos.  */
	 complain_overflow_signed, /* Complain_on_overflow.  */
	 reloc_nil,		/* Special_function.  */
	 "SREL32",		/* Name.  */
	 true,			/* Partial_inplace.  */
	 0xffffffff,		/* Source mask.  */
	 0xffffffff,		/* Dest mask.  */
	 false),		/* PC rel offset.  */

  /* A 64 bit PC relative offset.  */
  HOWTO (ALPHA_R_SREL64,	/* Type.  */
	 0,			/* Rightshift.  */
	 8,			/* Size.  */
	 64,			/* Bitsize.  */
	 true,			/* PC relative.  */
	 0,			/* Bitpos.  */
	 complain_overflow_signed, /* Complain_on_overflow.  */
	 reloc_nil,		/* Special_function.  */
	 "SREL64",		/* Name.  */
	 true,			/* Partial_inplace.  */
	 MINUS_ONE,		/* Source mask.  */
	 MINUS_ONE,		/* Dest mask.  */
	 false),		/* PC rel offset.  */

  /* Push a value on the reloc evaluation stack.  */
  HOWTO (ALPHA_R_OP_PUSH,	/* Type.  */
	 0,			/* Rightshift.  */
	 0,			/* Size.  */
	 0,			/* Bitsize.  */
	 false,			/* PC relative.  */
	 0,			/* Bitpos.  */
	 complain_overflow_dont,/* Complain_on_overflow.  */
	 reloc_nil,		/* Special_function.  */
	 "OP_PUSH",		/* Name.  */
	 false,			/* Partial_inplace.  */
	 0,			/* Source mask.  */
	 0,			/* Dest mask.  */
	 false),		/* PC rel offset.  */

  /* Store the value from the stack at the given address.  Store it in
     a bitfield of size r_size starting at bit position r_offset.  */
  HOWTO (ALPHA_R_OP_STORE,	/* Type.  */
	 0,			/* Rightshift.  */
	 8,			/* Size.  */
	 64,			/* Bitsize.  */
	 false,			/* PC relative.  */
	 0,			/* Bitpos.  */
	 complain_overflow_dont,/* Complain_on_overflow.  */
	 reloc_nil,		/* Special_function.  */
	 "OP_STORE",		/* Name.  */
	 false,			/* Partial_inplace.  */
	 0,			/* Source mask.  */
	 MINUS_ONE,		/* Dest mask.  */
	 false),		/* PC rel offset.  */

  /* Subtract the reloc address from the value on the top of the
     relocation stack.  */
  HOWTO (ALPHA_R_OP_PSUB,	/* Type.  */
	 0,			/* Rightshift.  */
	 0,			/* Size.  */
	 0,			/* Bitsize.  */
	 false,			/* PC relative.  */
	 0,			/* Bitpos.  */
	 complain_overflow_dont,/* Complain_on_overflow.  */
	 reloc_nil,		/* Special_function.  */
	 "OP_PSUB",		/* Name.  */
	 false,			/* Partial_inplace.  */
	 0,			/* Source mask.  */
	 0,			/* Dest mask.  */
	 false),		/* PC rel offset.  */

  /* Shift the value on the top of the relocation stack right by the
     given value.  */
  HOWTO (ALPHA_R_OP_PRSHIFT,	/* Type.  */
	 0,			/* Rightshift.  */
	 0,			/* Size.  */
	 0,			/* Bitsize.  */
	 false,			/* PC relative.  */
	 0,			/* Bitpos.  */
	 complain_overflow_dont,/* Complain_on_overflow.  */
	 reloc_nil,		/* Special_function.  */
	 "OP_PRSHIFT",		/* Name.  */
	 false,			/* Partial_inplace.  */
	 0,			/* Source mask.  */
	 0,			/* Dest mask.  */
	 false),		/* PC rel offset.  */

  /* Hack. Linkage is done by linker.  */
  HOWTO (ALPHA_R_LINKAGE,	/* Type.  */
	 0,			/* Rightshift.  */
	 0,			/* Size.  */
	 0,			/* Bitsize.  */
	 false,			/* PC relative.  */
	 0,			/* Bitpos.  */
	 complain_overflow_dont,/* Complain_on_overflow.  */
	 reloc_nil,		/* Special_function.  */
	 "LINKAGE",		/* Name.  */
	 false,			/* Partial_inplace.  */
	 0,			/* Source mask.  */
	 0,			/* Dest mask.  */
	 false),		/* PC rel offset.  */

  /* A 32 bit reference to a symbol.  */
  HOWTO (ALPHA_R_REFLONG,	/* Type.  */
	 0,			/* Rightshift.  */
	 4,			/* Size.  */
	 32,			/* Bitsize.  */
	 false,			/* PC relative.  */
	 0,			/* Bitpos.  */
	 complain_overflow_bitfield, /* Complain_on_overflow.  */
	 reloc_nil,		/* Special_function.  */
	 "REFLONG",		/* Name.  */
	 true,			/* Partial_inplace.  */
	 0xffffffff,		/* Source mask.  */
	 0xffffffff,		/* Dest mask.  */
	 false),		/* PC rel offset.  */

  /* A 64 bit reference to a procedure, written as 32 bit value.  */
  HOWTO (ALPHA_R_CODEADDR,	/* Type.  */
	 0,			/* Rightshift.  */
	 8,			/* Size.  */
	 64,			/* Bitsize.  */
	 false,			/* PC relative.  */
	 0,			/* Bitpos.  */
	 complain_overflow_signed,/* Complain_on_overflow.  */
	 reloc_nil,		/* Special_function.  */
	 "CODEADDR",		/* Name.  */
	 false,			/* Partial_inplace.  */
	 0xffffffff,		/* Source mask.  */
	 0xffffffff,		/* Dest mask.  */
	 false),		/* PC rel offset.  */

  HOWTO (ALPHA_R_NOP,		/* Type.  */
	 0,			/* Rightshift.  */
	 0,			/* Size.  */
	 0,			/* Bitsize.  */
	 /* The following value must match that of ALPHA_R_BSR/ALPHA_R_BOH
	    because the calculations for the 3 relocations are the same.
	    See B.4.5.2 of the OpenVMS Linker Utility Manual.  */
	 true,			/* PC relative.  */
	 0,			/* Bitpos.   */
	 complain_overflow_dont,/* Complain_on_overflow.  */
	 reloc_nil,		/* Special_function.  */
	 "NOP",			/* Name.  */
	 false,			/* Partial_inplace.  */
	 0xffffffff,		/* Source mask.  */
	 0xffffffff,		/* Dest mask.  */
	 false),		/* PC rel offset.  */

  HOWTO (ALPHA_R_BSR,		/* Type.  */
	 0,			/* Rightshift.  */
	 0,			/* Size.  */
	 0,			/* Bitsize.  */
	 true,			/* PC relative.  */
	 0,			/* Bitpos.  */
	 complain_overflow_dont,/* Complain_on_overflow.  */
	 reloc_nil,		/* Special_function.  */
	 "BSR",			/* Name.  */
	 false,			/* Partial_inplace.  */
	 0xffffffff,		/* Source mask.  */
	 0xffffffff,		/* Dest mask.  */
	 false),		/* PC rel offset.  */

  HOWTO (ALPHA_R_LDA,		/* Type.  */
	 0,			/* Rightshift.  */
	 0,			/* Size.  */
	 0,			/* Bitsize.  */
	 false,			/* PC relative.  */
	 0,			/* Bitpos.  */
	 complain_overflow_dont,/* Complain_on_overflow.  */
	 reloc_nil,		/* Special_function.  */
	 "LDA",			/* Name.  */
	 false,			/* Partial_inplace.  */
	 0xffffffff,		/* Source mask.  */
	 0xffffffff,		/* Dest mask.  */
	 false),		/* PC rel offset.  */

  HOWTO (ALPHA_R_BOH,		/* Type.  */
	 0,			/* Rightshift.  */
	 0,			/* Size.  */
	 0,			/* Bitsize.  */
	 true,			/* PC relative.  */
	 0,			/* Bitpos.  */
	 complain_overflow_dont,/* Complain_on_overflow.  */
	 reloc_nil,		/* Special_function.  */
	 "BOH",			/* Name.  */
	 false,			/* Partial_inplace.  */
	 0xffffffff,		/* Source mask.  */
	 0xffffffff,		/* Dest mask.  */
	 false),		/* PC rel offset.  */
};

/* Return a pointer to a howto structure which, when invoked, will perform
   the relocation code on data from the architecture noted.  */

static reloc_howto_type *
alpha_vms_bfd_reloc_type_lookup (bfd * abfd ATTRIBUTE_UNUSED,
				 bfd_reloc_code_real_type code)
{
  int alpha_type;

  vms_debug2 ((1, "vms_bfd_reloc_type_lookup (%p, %d)\t", abfd, code));

  switch (code)
    {
      case BFD_RELOC_16:		alpha_type = ALPHA_R_SREL16;	break;
      case BFD_RELOC_32:		alpha_type = ALPHA_R_REFLONG;	break;
      case BFD_RELOC_64:		alpha_type = ALPHA_R_REFQUAD;	break;
      case BFD_RELOC_CTOR:		alpha_type = ALPHA_R_REFQUAD;	break;
      case BFD_RELOC_23_PCREL_S2:	alpha_type = ALPHA_R_BRADDR;	break;
      case BFD_RELOC_ALPHA_HINT:	alpha_type = ALPHA_R_HINT;	break;
      case BFD_RELOC_16_PCREL:		alpha_type = ALPHA_R_SREL16;	break;
      case BFD_RELOC_32_PCREL:		alpha_type = ALPHA_R_SREL32;	break;
      case BFD_RELOC_64_PCREL:		alpha_type = ALPHA_R_SREL64;	break;
      case BFD_RELOC_ALPHA_LINKAGE:	alpha_type = ALPHA_R_LINKAGE;	break;
      case BFD_RELOC_ALPHA_CODEADDR:	alpha_type = ALPHA_R_CODEADDR;	break;
      case BFD_RELOC_ALPHA_NOP:		alpha_type = ALPHA_R_NOP;	break;
      case BFD_RELOC_ALPHA_BSR:		alpha_type = ALPHA_R_BSR;	break;
      case BFD_RELOC_ALPHA_LDA:		alpha_type = ALPHA_R_LDA;	break;
      case BFD_RELOC_ALPHA_BOH:		alpha_type = ALPHA_R_BOH;	break;
      default:
	_bfd_error_handler (_("reloc (%d) is *UNKNOWN*"), code);
	return NULL;
    }
  vms_debug2 ((2, "reloc is %s\n", alpha_howto_table[alpha_type].name));
  return & alpha_howto_table[alpha_type];
}

static reloc_howto_type *
alpha_vms_bfd_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED,
				 const char *r_name)
{
  unsigned int i;

  for (i = 0;
       i < sizeof (alpha_howto_table) / sizeof (alpha_howto_table[0]);
       i++)
    if (alpha_howto_table[i].name != NULL
	&& strcasecmp (alpha_howto_table[i].name, r_name) == 0)
      return &alpha_howto_table[i];

  return NULL;
}

static long
alpha_vms_get_synthetic_symtab (bfd *abfd,
				long symcount ATTRIBUTE_UNUSED,
				asymbol **usyms ATTRIBUTE_UNUSED,
				long dynsymcount ATTRIBUTE_UNUSED,
				asymbol **dynsyms ATTRIBUTE_UNUSED,
				asymbol **ret)
{
  asymbol *syms;
  unsigned int i;
  unsigned int n = 0;

  syms = (asymbol *) bfd_malloc (PRIV (norm_sym_count) * sizeof (asymbol));
  *ret = syms;
  if (syms == NULL)
    return -1;

  for (i = 0; i < PRIV (gsd_sym_count); i++)
    {
      struct vms_symbol_entry *e = PRIV (syms)[i];
      asymbol *sym;
      flagword flags;
      symvalue value;
      asection *sec;
      const char *name;
      char *sname;
      int l;

      name = e->name;
      value = 0;
      flags = BSF_LOCAL | BSF_SYNTHETIC;
      sec = NULL;

      switch (e->typ)
	{
	case EGSD__C_SYM:
	case EGSD__C_SYMG:
	  if ((e->flags & EGSY__V_DEF) && (e->flags & EGSY__V_NORM))
	    {
	      value = e->code_value;
	      sec = e->code_section;
	    }
	  else
	    continue;
	  break;

	default:
	  continue;
	}

      l = strlen (name);
      sname = bfd_alloc (abfd, l + 5);
      if (sname == NULL)
	return false;
      memcpy (sname, name, l);
      memcpy (sname + l, "..en", 5);

      sym = &syms[n++];
      sym->name = sname;
      sym->section = sec;
      sym->flags = flags;
      sym->value = value;
      sym->udata.p = NULL;
    }

  return n;
}

/* Private dump.  */

static const char *
vms_time_to_str (unsigned char *buf)
{
  time_t t = vms_rawtime_to_time_t (buf);
  char *res = ctime (&t);

  if (!res)
    res = "*invalid time*";
  else
    res[24] = 0;
  return res;
}

static void
evax_bfd_print_emh (FILE *file, unsigned char *rec, unsigned int rec_len)
{
  struct vms_emh_common *emh = (struct vms_emh_common *)rec;
  unsigned int subtype;
  int extra;

  subtype = (unsigned) bfd_getl16 (emh->subtyp);

  /* xgettext:c-format */
  fprintf (file, _("  EMH %u (len=%u): "), subtype, rec_len);

  /* PR 21618: Check for invalid lengths.  */
  if (rec_len < sizeof (* emh))
    {
      fprintf (file, _("   Error: The length is less than the length of an EMH record\n"));
      return;
    }
  extra = rec_len - sizeof (struct vms_emh_common);

  switch (subtype)
    {
    case EMH__C_MHD:
      {
	struct vms_emh_mhd *mhd = (struct vms_emh_mhd *) rec;
	unsigned char *name;
	unsigned char *nextname;
	unsigned char *maxname;

	/* PR 21840: Check for invalid lengths.  */
	if (rec_len < sizeof (* mhd))
	  {
	    fprintf (file, _("   Error: The record length is less than the size of an EMH_MHD record\n"));
	    return;
	  }
	fprintf (file, _("Module header\n"));
	fprintf (file, _("   structure level: %u\n"), mhd->strlvl);
	fprintf (file, _("   max record size: %u\n"),
		 (unsigned) bfd_getl32 (mhd->recsiz));
	name = (unsigned char *) (mhd + 1);
	maxname = (unsigned char *) rec + rec_len;
	if (name > maxname - 2)
	  {
	    fprintf (file, _("   Error: The module name is missing\n"));
	    return;
	  }
	nextname = name + name[0] + 1;
	if (nextname >= maxname)
	  {
	    fprintf (file, _("   Error: The module name is too long\n"));
	    return;
	  }
	fprintf (file, _("   module name    : %.*s\n"), name[0], name + 1);
	name = nextname;
	if (name > maxname - 2)
	  {
	    fprintf (file, _("   Error: The module version is missing\n"));
	    return;
	  }
	nextname = name + name[0] + 1;
	if (nextname >= maxname)
	  {
	    fprintf (file, _("   Error: The module version is too long\n"));
	    return;
	  }
	fprintf (file, _("   module version : %.*s\n"), name[0], name + 1);
	name = nextname;
	if ((maxname - name) < 17 && maxname[-1] != 0)
	  fprintf (file, _("   Error: The compile date is truncated\n"));
	else
	  fprintf (file, _("   compile date   : %.17s\n"), name);
      }
      break;

    case EMH__C_LNM:
      fprintf (file, _("Language Processor Name\n"));
      fprintf (file, _("   language name: %.*s\n"), extra, (char *)(emh + 1));
      break;

    case EMH__C_SRC:
      fprintf (file, _("Source Files Header\n"));
      fprintf (file, _("   file: %.*s\n"), extra, (char *)(emh + 1));
      break;

    case EMH__C_TTL:
      fprintf (file, _("Title Text Header\n"));
      fprintf (file, _("   title: %.*s\n"), extra, (char *)(emh + 1));
      break;

    case EMH__C_CPR:
      fprintf (file, _("Copyright Header\n"));
      fprintf (file, _("   copyright: %.*s\n"), extra, (char *)(emh + 1));
      break;

    default:
      fprintf (file, _("unhandled emh subtype %u\n"), subtype);
      break;
    }
}

static void
evax_bfd_print_eeom (FILE *file, unsigned char *rec, unsigned int rec_len)
{
  struct vms_eeom *eeom = (struct vms_eeom *)rec;

  fprintf (file, _("  EEOM (len=%u):\n"), rec_len);

  /* PR 21618: Check for invalid lengths.  */
  if (rec_len < sizeof (* eeom))
    {
      fprintf (file, _("   Error: The length is less than the length of an EEOM record\n"));
      return;
    }

  fprintf (file, _("   number of cond linkage pairs: %u\n"),
	   (unsigned)bfd_getl32 (eeom->total_lps));
  fprintf (file, _("   completion code: %u\n"),
	   (unsigned)bfd_getl16 (eeom->comcod));
  if (rec_len > 10)
    {
      fprintf (file, _("   transfer addr flags: 0x%02x\n"), eeom->tfrflg);
      fprintf (file, _("   transfer addr psect: %u\n"),
	       (unsigned)bfd_getl32 (eeom->psindx));
      fprintf (file, _("   transfer address   : 0x%08x\n"),
	       (unsigned)bfd_getl32 (eeom->tfradr));
    }
}

static void
exav_bfd_print_egsy_flags (unsigned int flags, FILE *file)
{
  if (flags & EGSY__V_WEAK)
    fputs (_(" WEAK"), file);
  if (flags & EGSY__V_DEF)
    fputs (_(" DEF"), file);
  if (flags & EGSY__V_UNI)
    fputs (_(" UNI"), file);
  if (flags & EGSY__V_REL)
    fputs (_(" REL"), file);
  if (flags & EGSY__V_COMM)
    fputs (_(" COMM"), file);
  if (flags & EGSY__V_VECEP)
    fputs (_(" VECEP"), file);
  if (flags & EGSY__V_NORM)
    fputs (_(" NORM"), file);
  if (flags & EGSY__V_QUAD_VAL)
    fputs (_(" QVAL"), file);
}

static void
evax_bfd_print_egsd_flags (FILE *file, unsigned int flags)
{
  if (flags & EGPS__V_PIC)
    fputs (_(" PIC"), file);
  if (flags & EGPS__V_LIB)
    fputs (_(" LIB"), file);
  if (flags & EGPS__V_OVR)
    fputs (_(" OVR"), file);
  if (flags & EGPS__V_REL)
    fputs (_(" REL"), file);
  if (flags & EGPS__V_GBL)
    fputs (_(" GBL"), file);
  if (flags & EGPS__V_SHR)
    fputs (_(" SHR"), file);
  if (flags & EGPS__V_EXE)
    fputs (_(" EXE"), file);
  if (flags & EGPS__V_RD)
    fputs (_(" RD"), file);
  if (flags & EGPS__V_WRT)
    fputs (_(" WRT"), file);
  if (flags & EGPS__V_VEC)
    fputs (_(" VEC"), file);
  if (flags & EGPS__V_NOMOD)
    fputs (_(" NOMOD"), file);
  if (flags & EGPS__V_COM)
    fputs (_(" COM"), file);
  if (flags & EGPS__V_ALLOC_64BIT)
    fputs (_(" 64B"), file);
}

static void
evax_bfd_print_egsd (FILE *file, unsigned char *rec, unsigned int rec_len)
{
  unsigned int off = sizeof (struct vms_egsd);
  unsigned int n = 0;

  fprintf (file, _("  EGSD (len=%u):\n"), rec_len);
  if (rec_len < sizeof (struct vms_egsd) + sizeof (struct vms_egsd_entry))
    return;

  while (off <= rec_len - sizeof (struct vms_egsd_entry))
    {
      struct vms_egsd_entry *e = (struct vms_egsd_entry *)(rec + off);
      unsigned int type;
      unsigned int len;
      unsigned int rest;

      type = (unsigned)bfd_getl16 (e->gsdtyp);
      len = (unsigned)bfd_getl16 (e->gsdsiz);

      /* xgettext:c-format */
      fprintf (file, _("  EGSD entry %2u (type: %u, len: %u): "),
	       n, type, len);
      n++;

      if (len < sizeof (struct vms_egsd_entry) || len > rec_len - off)
	{
	  fprintf (file, _("   Erroneous length\n"));
	  return;
	}

      switch (type)
	{
	case EGSD__C_PSC:
	  if (len >= offsetof (struct vms_egps, name))
	    {
	      struct vms_egps *egps = (struct vms_egps *) e;
	      unsigned int flags = bfd_getl16 (egps->flags);
	      unsigned int l;

	      fprintf (file, _("PSC - Program section definition\n"));
	      fprintf (file, _("   alignment  : 2**%u\n"), egps->align);
	      fprintf (file, _("   flags      : 0x%04x"), flags);
	      evax_bfd_print_egsd_flags (file, flags);
	      fputc ('\n', file);
	      l = bfd_getl32 (egps->alloc);
	      fprintf (file, _("   alloc (len): %u (0x%08x)\n"), l, l);
	      rest = len - offsetof (struct vms_egps, name);
	      fprintf (file, _("   name       : %.*s\n"),
		       egps->namlng > rest ? rest : egps->namlng,
		       egps->name);
	    }
	  break;
	case EGSD__C_SPSC:
	  if (len >= offsetof (struct vms_esgps, name))
	    {
	      struct vms_esgps *esgps = (struct vms_esgps *) e;
	      unsigned int flags = bfd_getl16 (esgps->flags);
	      unsigned int l;

	      fprintf (file, _("SPSC - Shared Image Program section def\n"));
	      fprintf (file, _("   alignment  : 2**%u\n"), esgps->align);
	      fprintf (file, _("   flags      : 0x%04x"), flags);
	      evax_bfd_print_egsd_flags (file, flags);
	      fputc ('\n', file);
	      l = bfd_getl32 (esgps->alloc);
	      fprintf (file, _("   alloc (len)   : %u (0x%08x)\n"), l, l);
	      fprintf (file, _("   image offset  : 0x%08x\n"),
		       (unsigned int) bfd_getl32 (esgps->base));
	      fprintf (file, _("   symvec offset : 0x%08x\n"),
		       (unsigned int) bfd_getl32 (esgps->value));
	      rest = len - offsetof (struct vms_esgps, name);
	      fprintf (file, _("   name          : %.*s\n"),
		       esgps->namlng > rest ? rest : esgps->namlng,
		       esgps->name);
	    }
	  break;
	case EGSD__C_SYM:
	  if (len >= sizeof (struct vms_egsy))
	    {
	      struct vms_egsy *egsy = (struct vms_egsy *) e;
	      unsigned int flags = bfd_getl16 (egsy->flags);

	      if ((flags & EGSY__V_DEF) != 0
		  && len >= offsetof (struct vms_esdf, name))
		{
		  struct vms_esdf *esdf = (struct vms_esdf *) e;

		  fprintf (file, _("SYM - Global symbol definition\n"));
		  fprintf (file, _("   flags: 0x%04x"), flags);
		  exav_bfd_print_egsy_flags (flags, file);
		  fputc ('\n', file);
		  fprintf (file, _("   psect offset: 0x%08x\n"),
			   (unsigned) bfd_getl32 (esdf->value));
		  if (flags & EGSY__V_NORM)
		    {
		      fprintf (file, _("   code address: 0x%08x\n"),
			       (unsigned) bfd_getl32 (esdf->code_address));
		      fprintf (file, _("   psect index for entry point : %u\n"),
			       (unsigned) bfd_getl32 (esdf->ca_psindx));
		    }
		  fprintf (file, _("   psect index : %u\n"),
			   (unsigned) bfd_getl32 (esdf->psindx));
		  rest = len - offsetof (struct vms_esdf, name);
		  fprintf (file, _("   name        : %.*s\n"),
			   esdf->namlng > rest ? rest : esdf->namlng,
			   esdf->name);
		}
	      else if (len >= offsetof (struct vms_esrf, name))
		{
		  struct vms_esrf *esrf = (struct vms_esrf *)e;

		  fprintf (file, _("SYM - Global symbol reference\n"));
		  rest = len - offsetof (struct vms_esrf, name);
		  fprintf (file, _("   name       : %.*s\n"),
			   esrf->namlng > rest ? rest : esrf->namlng,
			   esrf->name);
		}
	    }
	  break;
	case EGSD__C_IDC:
	  if (len >= sizeof (struct vms_eidc))
	    {
	      struct vms_eidc *eidc = (struct vms_eidc *) e;
	      unsigned int flags = bfd_getl32 (eidc->flags);
	      unsigned char *p;

	      fprintf (file, _("IDC - Ident Consistency check\n"));
	      fprintf (file, _("   flags         : 0x%08x"), flags);
	      if (flags & EIDC__V_BINIDENT)
		fputs (" BINDENT", file);
	      fputc ('\n', file);
	      fprintf (file, _("   id match      : %x\n"),
		       (flags >> EIDC__V_IDMATCH_SH) & EIDC__V_IDMATCH_MASK);
	      fprintf (file, _("   error severity: %x\n"),
		       (flags >> EIDC__V_ERRSEV_SH) & EIDC__V_ERRSEV_MASK);
	      p = eidc->name;
	      rest = len - (p - (unsigned char *) e);
	      fprintf (file, _("   entity name   : %.*s\n"),
		       p[0] > rest - 1 ? rest - 1 : p[0], p + 1);
	      if (rest > 1u + p[0])
		{
		  rest -= 1 + p[0];
		  p += 1 + p[0];
		  fprintf (file, _("   object name   : %.*s\n"),
			   p[0] > rest - 1 ? rest - 1 : p[0], p + 1);
		  if (rest > 1u + p[0])
		    {
		      rest -= 1 + p[0];
		      p += 1 + p[0];
		      if (flags & EIDC__V_BINIDENT)
			{
			  if (rest >= 4)
			    fprintf (file, _("   binary ident  : 0x%08x\n"),
				     (unsigned) bfd_getl32 (p));
			}
		      else
			fprintf (file, _("   ascii ident   : %.*s\n"),
				 p[0] > rest - 1 ? rest - 1 : p[0], p + 1);
		    }
		}
	    }
	  break;
	case EGSD__C_SYMG:
	  if (len >= offsetof (struct vms_egst, name))
	    {
	      struct vms_egst *egst = (struct vms_egst *) e;
	      unsigned int flags = bfd_getl16 (egst->header.flags);

	      fprintf (file, _("SYMG - Universal symbol definition\n"));
	      fprintf (file, _("   flags: 0x%04x"), flags);
	      exav_bfd_print_egsy_flags (flags, file);
	      fputc ('\n', file);
	      fprintf (file, _("   symbol vector offset: 0x%08x\n"),
		       (unsigned) bfd_getl32 (egst->value));
	      fprintf (file, _("   entry point: 0x%08x\n"),
		       (unsigned) bfd_getl32 (egst->lp_1));
	      fprintf (file, _("   proc descr : 0x%08x\n"),
		       (unsigned) bfd_getl32 (egst->lp_2));
	      fprintf (file, _("   psect index: %u\n"),
		       (unsigned) bfd_getl32 (egst->psindx));
	      rest = len - offsetof (struct vms_egst, name);
	      fprintf (file, _("   name       : %.*s\n"),
		       egst->namlng > rest ? rest : egst->namlng,
		       egst->name);
	    }
	  break;
	case EGSD__C_SYMV:
	  if (len >= offsetof (struct vms_esdfv, name))
	    {
	      struct vms_esdfv *esdfv = (struct vms_esdfv *) e;
	      unsigned int flags = bfd_getl16 (esdfv->flags);

	      fprintf (file, _("SYMV - Vectored symbol definition\n"));
	      fprintf (file, _("   flags: 0x%04x"), flags);
	      exav_bfd_print_egsy_flags (flags, file);
	      fputc ('\n', file);
	      fprintf (file, _("   vector      : 0x%08x\n"),
		       (unsigned) bfd_getl32 (esdfv->vector));
	      fprintf (file, _("   psect offset: %u\n"),
		       (unsigned) bfd_getl32 (esdfv->value));
	      fprintf (file, _("   psect index : %u\n"),
		       (unsigned) bfd_getl32 (esdfv->psindx));
	      rest = len - offsetof (struct vms_esdfv, name);
	      fprintf (file, _("   name        : %.*s\n"),
		       esdfv->namlng > rest ? rest : esdfv->namlng,
		       esdfv->name);
	    }
	  break;
	case EGSD__C_SYMM:
	  if (len >= offsetof (struct vms_esdfm, name))
	    {
	      struct vms_esdfm *esdfm = (struct vms_esdfm *) e;
	      unsigned int flags = bfd_getl16 (esdfm->flags);

	      fprintf (file,
		       _("SYMM - Global symbol definition with version\n"));
	      fprintf (file, _("   flags: 0x%04x"), flags);
	      exav_bfd_print_egsy_flags (flags, file);
	      fputc ('\n', file);
	      fprintf (file, _("   version mask: 0x%08x\n"),
		       (unsigned)bfd_getl32 (esdfm->version_mask));
	      fprintf (file, _("   psect offset: %u\n"),
		       (unsigned)bfd_getl32 (esdfm->value));
	      fprintf (file, _("   psect index : %u\n"),
		       (unsigned)bfd_getl32 (esdfm->psindx));
	      rest = len - offsetof (struct vms_esdfm, name);
	      fprintf (file, _("   name        : %.*s\n"),
		       esdfm->namlng > rest ? rest : esdfm->namlng,
		       esdfm->name);
	    }
	  break;
	default:
	  fprintf (file, _("unhandled egsd entry type %u\n"), type);
	  break;
	}
      off += len;
    }
}

static void
evax_bfd_print_hex (FILE *file, const char *pfx,
		    const unsigned char *buf, unsigned int len)
{
  unsigned int i;
  unsigned int n;

  n = 0;
  for (i = 0; i < len; i++)
    {
      if (n == 0)
	fputs (pfx, file);
      fprintf (file, " %02x", buf[i]);
      n++;
      if (n == 16)
	{
	  n = 0;
	  fputc ('\n', file);
	}
    }
  if (n != 0)
    fputc ('\n', file);
}

static void
evax_bfd_print_etir_stc_ir (FILE *file, const unsigned char *buf,
			    unsigned int len, int is_ps)
{
  if (is_ps ? len < 44 : len < 33)
    return;

  /* xgettext:c-format */
  fprintf (file, _("    linkage index: %u, replacement insn: 0x%08x\n"),
	   (unsigned)bfd_getl32 (buf),
	   (unsigned)bfd_getl32 (buf + 16));
  /* xgettext:c-format */
  fprintf (file, _("    psect idx 1: %u, offset 1: 0x%08x %08x\n"),
	   (unsigned)bfd_getl32 (buf + 4),
	   (unsigned)bfd_getl32 (buf + 12),
	   (unsigned)bfd_getl32 (buf + 8));
  /* xgettext:c-format */
  fprintf (file, _("    psect idx 2: %u, offset 2: 0x%08x %08x\n"),
	   (unsigned)bfd_getl32 (buf + 20),
	   (unsigned)bfd_getl32 (buf + 28),
	   (unsigned)bfd_getl32 (buf + 24));
  if (is_ps)
    /* xgettext:c-format */
    fprintf (file, _("    psect idx 3: %u, offset 3: 0x%08x %08x\n"),
	     (unsigned)bfd_getl32 (buf + 32),
	     (unsigned)bfd_getl32 (buf + 40),
	     (unsigned)bfd_getl32 (buf + 36));
  else
    fprintf (file, _("    global name: %.*s\n"),
	     buf[32] > len - 33 ? len - 33 : buf[32],
	     buf + 33);
}

static void
evax_bfd_print_etir (FILE *file, const char *name,
		     unsigned char *rec, unsigned int rec_len)
{
  unsigned int off = sizeof (struct vms_eobjrec);

  /* xgettext:c-format */
  fprintf (file, _("  %s (len=%u):\n"), name, (unsigned) rec_len);
  if (rec_len < sizeof (struct vms_eobjrec) + sizeof (struct vms_etir))
    return;

  while (off <= rec_len - sizeof (struct vms_etir))
    {
      struct vms_etir *etir = (struct vms_etir *)(rec + off);
      unsigned char *buf;
      unsigned int type;
      unsigned int size;
      unsigned int rest;

      type = bfd_getl16 (etir->rectyp);
      size = bfd_getl16 (etir->size);
      buf = rec + off + sizeof (struct vms_etir);

      if (size < sizeof (struct vms_etir) || size > rec_len - off)
	{
	  fprintf (file, _("   Erroneous length\n"));
	  return;
	}

      /* xgettext:c-format */
      fprintf (file, _("   (type: %3u, size: %3u): "), type, size);
      rest = size - sizeof (struct vms_etir);
      switch (type)
	{
	case ETIR__C_STA_GBL:
	  if (rest >= 1)
	    fprintf (file, _("STA_GBL (stack global) %.*s\n"),
		     buf[0] > rest - 1 ? rest - 1 : buf[0], buf + 1);
	  break;
	case ETIR__C_STA_LW:
	  fprintf (file, _("STA_LW (stack longword)"));
	  if (rest >= 4)
	    fprintf (file, " 0x%08x\n",
		     (unsigned) bfd_getl32 (buf));
	  break;
	case ETIR__C_STA_QW:
	  fprintf (file, _("STA_QW (stack quadword)"));
	  if (rest >= 8)
	    fprintf (file, " 0x%08x %08x\n",
		     (unsigned) bfd_getl32 (buf + 4),
		     (unsigned) bfd_getl32 (buf + 0));
	  break;
	case ETIR__C_STA_PQ:
	  fprintf (file, _("STA_PQ (stack psect base + offset)\n"));
	  if (rest >= 12)
	    /* xgettext:c-format */
	    fprintf (file, _("    psect: %u, offset: 0x%08x %08x\n"),
		     (unsigned) bfd_getl32 (buf + 0),
		     (unsigned) bfd_getl32 (buf + 8),
		     (unsigned) bfd_getl32 (buf + 4));
	  break;
	case ETIR__C_STA_LI:
	  fprintf (file, _("STA_LI (stack literal)\n"));
	  break;
	case ETIR__C_STA_MOD:
	  fprintf (file, _("STA_MOD (stack module)\n"));
	  break;
	case ETIR__C_STA_CKARG:
	  fprintf (file, _("STA_CKARG (compare procedure argument)\n"));
	  break;

	case ETIR__C_STO_B:
	  fprintf (file, _("STO_B (store byte)\n"));
	  break;
	case ETIR__C_STO_W:
	  fprintf (file, _("STO_W (store word)\n"));
	  break;
	case ETIR__C_STO_LW:
	  fprintf (file, _("STO_LW (store longword)\n"));
	  break;
	case ETIR__C_STO_QW:
	  fprintf (file, _("STO_QW (store quadword)\n"));
	  break;
	case ETIR__C_STO_IMMR:
	  if (rest >= 4)
	    {
	      unsigned int rpt = bfd_getl32 (buf);
	      fprintf (file,
		       _("STO_IMMR (store immediate repeat) %u bytes\n"),
		       rpt);
	      if (rpt > rest - 4)
		rpt = rest - 4;
	      evax_bfd_print_hex (file, "   ", buf + 4, rpt);
	    }
	  break;
	case ETIR__C_STO_GBL:
	  if (rest >= 1)
	    fprintf (file, _("STO_GBL (store global) %.*s\n"),
		     buf[0] > rest - 1 ? rest - 1 : buf[0], buf + 1);
	  break;
	case ETIR__C_STO_CA:
	  if (rest >= 1)
	    fprintf (file, _("STO_CA (store code address) %.*s\n"),
		     buf[0] > rest - 1 ? rest - 1 : buf[0], buf + 1);
	  break;
	case ETIR__C_STO_RB:
	  fprintf (file, _("STO_RB (store relative branch)\n"));
	  break;
	case ETIR__C_STO_AB:
	  fprintf (file, _("STO_AB (store absolute branch)\n"));
	  break;
	case ETIR__C_STO_OFF:
	  fprintf (file, _("STO_OFF (store offset to psect)\n"));
	  break;
	case ETIR__C_STO_IMM:
	  if (rest >= 4)
	    {
	      unsigned int rpt = bfd_getl32 (buf);
	      fprintf (file,
		       _("STO_IMM (store immediate) %u bytes\n"),
		       rpt);
	      if (rpt > rest - 4)
		rpt = rest - 4;
	      evax_bfd_print_hex (file, "   ", buf + 4, rpt);
	    }
	  break;
	case ETIR__C_STO_GBL_LW:
	  if (rest >= 1)
	    fprintf (file, _("STO_GBL_LW (store global longword) %.*s\n"),
		     buf[0] > rest - 1 ? rest - 1 : buf[0], buf + 1);
	  break;
	case ETIR__C_STO_LP_PSB:
	  fprintf (file, _("STO_OFF (store LP with procedure signature)\n"));
	  break;
	case ETIR__C_STO_HINT_GBL:
	  fprintf (file, _("STO_BR_GBL (store branch global) *todo*\n"));
	  break;
	case ETIR__C_STO_HINT_PS:
	  fprintf (file, _("STO_BR_PS (store branch psect + offset) *todo*\n"));
	  break;

	case ETIR__C_OPR_NOP:
	  fprintf (file, _("OPR_NOP (no-operation)\n"));
	  break;
	case ETIR__C_OPR_ADD:
	  fprintf (file, _("OPR_ADD (add)\n"));
	  break;
	case ETIR__C_OPR_SUB:
	  fprintf (file, _("OPR_SUB (subtract)\n"));
	  break;
	case ETIR__C_OPR_MUL:
	  fprintf (file, _("OPR_MUL (multiply)\n"));
	  break;
	case ETIR__C_OPR_DIV:
	  fprintf (file, _("OPR_DIV (divide)\n"));
	  break;
	case ETIR__C_OPR_AND:
	  fprintf (file, _("OPR_AND (logical and)\n"));
	  break;
	case ETIR__C_OPR_IOR:
	  fprintf (file, _("OPR_IOR (logical inclusive or)\n"));
	  break;
	case ETIR__C_OPR_EOR:
	  fprintf (file, _("OPR_EOR (logical exclusive or)\n"));
	  break;
	case ETIR__C_OPR_NEG:
	  fprintf (file, _("OPR_NEG (negate)\n"));
	  break;
	case ETIR__C_OPR_COM:
	  fprintf (file, _("OPR_COM (complement)\n"));
	  break;
	case ETIR__C_OPR_INSV:
	  fprintf (file, _("OPR_INSV (insert field)\n"));
	  break;
	case ETIR__C_OPR_ASH:
	  fprintf (file, _("OPR_ASH (arithmetic shift)\n"));
	  break;
	case ETIR__C_OPR_USH:
	  fprintf (file, _("OPR_USH (unsigned shift)\n"));
	  break;
	case ETIR__C_OPR_ROT:
	  fprintf (file, _("OPR_ROT (rotate)\n"));
	  break;
	case ETIR__C_OPR_SEL:
	  fprintf (file, _("OPR_SEL (select)\n"));
	  break;
	case ETIR__C_OPR_REDEF:
	  fprintf (file, _("OPR_REDEF (redefine symbol to curr location)\n"));
	  break;
	case ETIR__C_OPR_DFLIT:
	  fprintf (file, _("OPR_REDEF (define a literal)\n"));
	  break;

	case ETIR__C_STC_LP:
	  fprintf (file, _("STC_LP (store cond linkage pair)\n"));
	  break;
	case ETIR__C_STC_LP_PSB:
	  fprintf (file,
		   _("STC_LP_PSB (store cond linkage pair + signature)\n"));
	  if (rest >= 5)
	    {
	      /* xgettext:c-format */
	      fprintf (file, _("   linkage index: %u, procedure: %.*s\n"),
		       (unsigned) bfd_getl32 (buf),
		       buf[4] > rest - 5 ? rest - 5 : buf[4], buf + 5);
	      if (rest > 4 + 1u + buf[4])
		{
		  rest -= 4 + 1 + buf[4];
		  buf += 4 + 1 + buf[4];
		  fprintf (file, _("   signature: %.*s\n"),
			   buf[0] > rest - 1 ? rest - 1: buf[0], buf + 1);
		}
	    }
	  break;
	case ETIR__C_STC_GBL:
	  fprintf (file, _("STC_GBL (store cond global)\n"));
	  if (rest >= 5)
	    /* xgettext:c-format */
	    fprintf (file, _("   linkage index: %u, global: %.*s\n"),
		     (unsigned) bfd_getl32 (buf),
		     buf[4] > rest - 5 ? rest - 5 : buf[4], buf + 5);
	  break;
	case ETIR__C_STC_GCA:
	  fprintf (file, _("STC_GCA (store cond code address)\n"));
	  if (rest >= 5)
	    /* xgettext:c-format */
	    fprintf (file, _("   linkage index: %u, procedure name: %.*s\n"),
		     (unsigned) bfd_getl32 (buf),
		     buf[4] > rest - 5 ? rest - 5 : buf[4], buf + 5);
	  break;
	case ETIR__C_STC_PS:
	  fprintf (file, _("STC_PS (store cond psect + offset)\n"));
	  if (rest >= 16)
	    fprintf (file,
		     /* xgettext:c-format */
		     _("   linkage index: %u, psect: %u, offset: 0x%08x %08x\n"),
		     (unsigned)bfd_getl32 (buf),
		     (unsigned)bfd_getl32 (buf + 4),
		     (unsigned)bfd_getl32 (buf + 12),
		     (unsigned)bfd_getl32 (buf + 8));
	  break;
	case ETIR__C_STC_NOP_GBL:
	  fprintf (file, _("STC_NOP_GBL (store cond NOP at global addr)\n"));
	  evax_bfd_print_etir_stc_ir (file, buf, rest, 0);
	  break;
	case ETIR__C_STC_NOP_PS:
	  fprintf (file, _("STC_NOP_PS (store cond NOP at psect + offset)\n"));
	  evax_bfd_print_etir_stc_ir (file, buf, rest, 1);
	  break;
	case ETIR__C_STC_BSR_GBL:
	  fprintf (file, _("STC_BSR_GBL (store cond BSR at global addr)\n"));
	  evax_bfd_print_etir_stc_ir (file, buf, rest, 0);
	  break;
	case ETIR__C_STC_BSR_PS:
	  fprintf (file, _("STC_BSR_PS (store cond BSR at psect + offset)\n"));
	  evax_bfd_print_etir_stc_ir (file, buf, rest, 1);
	  break;
	case ETIR__C_STC_LDA_GBL:
	  fprintf (file, _("STC_LDA_GBL (store cond LDA at global addr)\n"));
	  evax_bfd_print_etir_stc_ir (file, buf, rest, 0);
	  break;
	case ETIR__C_STC_LDA_PS:
	  fprintf (file, _("STC_LDA_PS (store cond LDA at psect + offset)\n"));
	  evax_bfd_print_etir_stc_ir (file, buf, rest, 1);
	  break;
	case ETIR__C_STC_BOH_GBL:
	  fprintf (file, _("STC_BOH_GBL (store cond BOH at global addr)\n"));
	  evax_bfd_print_etir_stc_ir (file, buf, rest, 0);
	  break;
	case ETIR__C_STC_BOH_PS:
	  fprintf (file, _("STC_BOH_PS (store cond BOH at psect + offset)\n"));
	  evax_bfd_print_etir_stc_ir (file, buf, rest, 1);
	  break;
	case ETIR__C_STC_NBH_GBL:
	  fprintf (file,
		   _("STC_NBH_GBL (store cond or hint at global addr)\n"));
	  break;
	case ETIR__C_STC_NBH_PS:
	  fprintf (file,
		   _("STC_NBH_PS (store cond or hint at psect + offset)\n"));
	  break;

	case ETIR__C_CTL_SETRB:
	  fprintf (file, _("CTL_SETRB (set relocation base)\n"));
	  break;
	case ETIR__C_CTL_AUGRB:
	  if (rest >= 4)
	    {
	      unsigned int val = bfd_getl32 (buf);
	      fprintf (file, _("CTL_AUGRB (augment relocation base) %u\n"),
		       val);
	    }
	  break;
	case ETIR__C_CTL_DFLOC:
	  fprintf (file, _("CTL_DFLOC (define location)\n"));
	  break;
	case ETIR__C_CTL_STLOC:
	  fprintf (file, _("CTL_STLOC (set location)\n"));
	  break;
	case ETIR__C_CTL_STKDL:
	  fprintf (file, _("CTL_STKDL (stack defined location)\n"));
	  break;
	default:
	  fprintf (file, _("*unhandled*\n"));
	  break;
	}
      off += size;
    }
}

static void
evax_bfd_print_eobj (struct bfd *abfd, FILE *file)
{
  bool is_first = true;
  bool has_records = true;

  while (1)
    {
      unsigned int rec_len;
      unsigned int pad_len;
      unsigned char *rec;
      unsigned int hdr_size;
      unsigned int type;
      unsigned char buf[6];

      hdr_size = has_records ? 6 : 4;
      if (bfd_bread (buf, hdr_size, abfd) != hdr_size)
	{
	  fprintf (file, _("cannot read GST record header\n"));
	  return;
	}

      type = bfd_getl16 (buf);
      rec_len = bfd_getl16 (buf + 2);
      pad_len = rec_len;
      if (has_records)
	{
	  unsigned int rec_len2 = bfd_getl16 (buf + 4);

	  if (is_first)
	    {
	      is_first = false;
	      if (type == rec_len2 && rec_len == EOBJ__C_EMH)
		/* Matched a VMS record EMH.  */
		;
	      else
		{
		  has_records = false;
		  if (type != EOBJ__C_EMH)
		    {
		      /* Ill-formed.  */
		      fprintf (file, _("cannot find EMH in first GST record\n"));
		      return;
		    }
		}
	    }

	  if (has_records)
	    {
	      /* VMS record format is: record-size, type, record-size.
		 See maybe_adjust_record_pointer_for_object comment.  */
	      if (type == rec_len2)
		{
		  type = rec_len;
		  rec_len = rec_len2;
		}
	      else
		rec_len = 0;
	      pad_len = (rec_len + 1) & ~1U;
	      hdr_size = 4;
	    }
	}

      if (rec_len < hdr_size)
	{
	  fprintf (file, _("corrupted GST\n"));
	  return;
	}

      rec = bfd_malloc (pad_len);
      if (rec == NULL)
	return;

      memcpy (rec, buf + (has_records ? 2 : 0), hdr_size);

      if (bfd_bread (rec + hdr_size, pad_len - hdr_size, abfd)
	  != pad_len - hdr_size)
	{
	  fprintf (file, _("cannot read GST record\n"));
	  return;
	}

      switch (type)
	{
	case EOBJ__C_EMH:
	  evax_bfd_print_emh (file, rec, rec_len);
	  break;
	case EOBJ__C_EGSD:
	  evax_bfd_print_egsd (file, rec, rec_len);
	  break;
	case EOBJ__C_EEOM:
	  evax_bfd_print_eeom (file, rec, rec_len);
	  free (rec);
	  return;
	  break;
	case EOBJ__C_ETIR:
	  evax_bfd_print_etir (file, "ETIR", rec, rec_len);
	  break;
	case EOBJ__C_EDBG:
	  evax_bfd_print_etir (file, "EDBG", rec, rec_len);
	  break;
	case EOBJ__C_ETBT:
	  evax_bfd_print_etir (file, "ETBT", rec, rec_len);
	  break;
	default:
	  fprintf (file, _(" unhandled EOBJ record type %u\n"), type);
	  break;
	}
      free (rec);
    }
}

static void
evax_bfd_print_relocation_records (FILE *file, const unsigned char *buf,
				   size_t buf_size, size_t off,
				   unsigned int stride)
{
  while (off <= buf_size - 8)
    {
      unsigned int base;
      unsigned int count;
      unsigned int j;

      count = bfd_getl32 (buf + off + 0);

      if (count == 0)
	break;
      base = bfd_getl32 (buf + off + 4);

      /* xgettext:c-format */
      fprintf (file, _("  bitcount: %u, base addr: 0x%08x\n"),
	       count, base);

      off += 8;
      for (j = 0; count > 0 && off <= buf_size - 4; j += 4, count -= 32)
	{
	  unsigned int k;
	  unsigned int n = 0;
	  unsigned int val;

	  val = bfd_getl32 (buf + off);
	  off += 4;

	  /* xgettext:c-format */
	  fprintf (file, _("   bitmap: 0x%08x (count: %u):\n"), val, count);

	  for (k = 0; k < 32; k++)
	    if (val & (1u << k))
	      {
		if (n == 0)
		  fputs ("   ", file);
		fprintf (file, _(" %08x"), base + (j * 8 + k) * stride);
		n++;
		if (n == 8)
		  {
		    fputs ("\n", file);
		    n = 0;
		  }
	      }
	  if (n)
	    fputs ("\n", file);
	}
    }
}

static void
evax_bfd_print_address_fixups (FILE *file, const unsigned char *buf,
			       size_t buf_size, size_t off)
{
  while (off <= buf_size - 8)
    {
      unsigned int j;
      unsigned int count;

      count = bfd_getl32 (buf + off + 0);
      if (count == 0)
	return;
      /* xgettext:c-format */
      fprintf (file, _("  image %u (%u entries)\n"),
	       (unsigned) bfd_getl32 (buf + off + 4), count);
      off += 8;
      for (j = 0; j < count && off <= buf_size - 8; j++)
	{
	  /* xgettext:c-format */
	  fprintf (file, _("   offset: 0x%08x, val: 0x%08x\n"),
		   (unsigned) bfd_getl32 (buf + off + 0),
		   (unsigned) bfd_getl32 (buf + off + 4));
	  off += 8;
	}
    }
}

static void
evax_bfd_print_reference_fixups (FILE *file, const unsigned char *buf,
				 size_t buf_size, size_t off)
{
  unsigned int count;

  while (off <= buf_size - 8)
    {
      unsigned int j;
      unsigned int n = 0;

      count = bfd_getl32 (buf + off + 0);
      if (count == 0)
	break;
      /* xgettext:c-format */
      fprintf (file, _("  image %u (%u entries), offsets:\n"),
	       (unsigned) bfd_getl32 (buf + off + 4), count);
      off += 8;
      for (j = 0; j < count && off <= buf_size - 4; j++)
	{
	  if (n == 0)
	    fputs ("   ", file);
	  fprintf (file, _(" 0x%08x"), (unsigned) bfd_getl32 (buf + off));
	  n++;
	  if (n == 7)
	    {
	      fputs ("\n", file);
	      n = 0;
	    }
	  off += 4;
	}
      if (n)
	fputs ("\n", file);
    }
}

static void
evax_bfd_print_indent (int indent, FILE *file)
{
  for (; indent; indent--)
    fputc (' ', file);
}

static const char *
evax_bfd_get_dsc_name (unsigned int v)
{
  switch (v)
    {
    case DSC__K_DTYPE_Z:
      return "Z (Unspecified)";
    case DSC__K_DTYPE_V:
      return "V (Bit)";
    case DSC__K_DTYPE_BU:
      return "BU (Byte logical)";
    case DSC__K_DTYPE_WU:
      return "WU (Word logical)";
    case DSC__K_DTYPE_LU:
      return "LU (Longword logical)";
    case DSC__K_DTYPE_QU:
      return "QU (Quadword logical)";
    case DSC__K_DTYPE_B:
      return "B (Byte integer)";
    case DSC__K_DTYPE_W:
      return "W (Word integer)";
    case DSC__K_DTYPE_L:
      return "L (Longword integer)";
    case DSC__K_DTYPE_Q:
      return "Q (Quadword integer)";
    case DSC__K_DTYPE_F:
      return "F (Single-precision floating)";
    case DSC__K_DTYPE_D:
      return "D (Double-precision floating)";
    case DSC__K_DTYPE_FC:
      return "FC (Complex)";
    case DSC__K_DTYPE_DC:
      return "DC (Double-precision Complex)";
    case DSC__K_DTYPE_T:
      return "T (ASCII text string)";
    case DSC__K_DTYPE_NU:
      return "NU (Numeric string, unsigned)";
    case DSC__K_DTYPE_NL:
      return "NL (Numeric string, left separate sign)";
    case DSC__K_DTYPE_NLO:
      return "NLO (Numeric string, left overpunched sign)";
    case DSC__K_DTYPE_NR:
      return "NR (Numeric string, right separate sign)";
    case DSC__K_DTYPE_NRO:
      return "NRO (Numeric string, right overpunched sig)";
    case DSC__K_DTYPE_NZ:
      return "NZ (Numeric string, zoned sign)";
    case DSC__K_DTYPE_P:
      return "P (Packed decimal string)";
    case DSC__K_DTYPE_ZI:
      return "ZI (Sequence of instructions)";
    case DSC__K_DTYPE_ZEM:
      return "ZEM (Procedure entry mask)";
    case DSC__K_DTYPE_DSC:
      return "DSC (Descriptor, used for arrays of dyn strings)";
    case DSC__K_DTYPE_OU:
      return "OU (Octaword logical)";
    case DSC__K_DTYPE_O:
      return "O (Octaword integer)";
    case DSC__K_DTYPE_G:
      return "G (Double precision G floating, 64 bit)";
    case DSC__K_DTYPE_H:
      return "H (Quadruple precision floating, 128 bit)";
    case DSC__K_DTYPE_GC:
      return "GC (Double precision complex, G floating)";
    case DSC__K_DTYPE_HC:
      return "HC (Quadruple precision complex, H floating)";
    case DSC__K_DTYPE_CIT:
      return "CIT (COBOL intermediate temporary)";
    case DSC__K_DTYPE_BPV:
      return "BPV (Bound Procedure Value)";
    case DSC__K_DTYPE_BLV:
      return "BLV (Bound Label Value)";
    case DSC__K_DTYPE_VU:
      return "VU (Bit Unaligned)";
    case DSC__K_DTYPE_ADT:
      return "ADT (Absolute Date-Time)";
    case DSC__K_DTYPE_VT:
      return "VT (Varying Text)";
    case DSC__K_DTYPE_T2:
      return "T2 (16-bit char)";
    case DSC__K_DTYPE_VT2:
      return "VT2 (16-bit varying char)";
    default:
      return "?? (unknown)";
    }
}

static void
evax_bfd_print_desc (const unsigned char *buf, unsigned int bufsize,
		     int indent, FILE *file)
{
  if (bufsize < 8)
    return;

  unsigned char bclass = buf[3];
  unsigned char dtype = buf[2];
  unsigned int len = (unsigned)bfd_getl16 (buf);
  unsigned int pointer = (unsigned)bfd_getl32 (buf + 4);

  evax_bfd_print_indent (indent, file);

  if (len == 1 && pointer == 0xffffffffUL)
    {
      /* 64 bits.  */
      fprintf (file, _("64 bits *unhandled*\n"));
    }
  else
    {
      /* xgettext:c-format */
      fprintf (file, _("class: %u, dtype: %u, length: %u, pointer: 0x%08x\n"),
	       bclass, dtype, len, pointer);
      switch (bclass)
	{
	case DSC__K_CLASS_NCA:
	  {
	    const struct vms_dsc_nca *dsc = (const void *)buf;
	    unsigned int i;
	    const unsigned char *b;

	    evax_bfd_print_indent (indent, file);
	    fprintf (file, _("non-contiguous array of %s\n"),
		     evax_bfd_get_dsc_name (dsc->dtype));
	    if (bufsize >= sizeof (*dsc))
	      {
		evax_bfd_print_indent (indent + 1, file);
		fprintf (file,
			 /* xgettext:c-format */
			 _("dimct: %u, aflags: 0x%02x, digits: %u, scale: %u\n"),
			 dsc->dimct, dsc->aflags, dsc->digits, dsc->scale);
		evax_bfd_print_indent (indent + 1, file);
		fprintf (file,
			 /* xgettext:c-format */
			 _("arsize: %u, a0: 0x%08x\n"),
			 (unsigned) bfd_getl32 (dsc->arsize),
			 (unsigned) bfd_getl32 (dsc->a0));
		evax_bfd_print_indent (indent + 1, file);
		fprintf (file, _("Strides:\n"));
		b = buf + sizeof (*dsc);
		bufsize -= sizeof (*dsc);
		for (i = 0; i < dsc->dimct; i++)
		  {
		    if (bufsize < 4)
		      break;
		    evax_bfd_print_indent (indent + 2, file);
		    fprintf (file, "[%u]: %u\n", i + 1,
			     (unsigned) bfd_getl32 (b));
		    b += 4;
		    bufsize -= 4;
		  }
		evax_bfd_print_indent (indent + 1, file);
		fprintf (file, _("Bounds:\n"));
		for (i = 0; i < dsc->dimct; i++)
		  {
		    if (bufsize < 8)
		      break;
		    evax_bfd_print_indent (indent + 2, file);
		    /* xgettext:c-format */
		    fprintf (file, _("[%u]: Lower: %u, upper: %u\n"), i + 1,
			     (unsigned) bfd_getl32 (b + 0),
			     (unsigned) bfd_getl32 (b + 4));
		    b += 8;
		    bufsize -= 8;
		  }
	      }
	  }
	  break;
	case DSC__K_CLASS_UBS:
	  {
	    const struct vms_dsc_ubs *ubs = (const void *)buf;

	    evax_bfd_print_indent (indent, file);
	    fprintf (file, _("unaligned bit-string of %s\n"),
		     evax_bfd_get_dsc_name (ubs->dtype));
	    if (bufsize >= sizeof (*ubs))
	      {
		evax_bfd_print_indent (indent + 1, file);
		fprintf (file,
			 /* xgettext:c-format */
			 _("base: %u, pos: %u\n"),
			 (unsigned) bfd_getl32 (ubs->base),
			 (unsigned) bfd_getl32 (ubs->pos));
	      }
	  }
	  break;
	default:
	  fprintf (file, _("*unhandled*\n"));
	  break;
	}
    }
}

static unsigned int
evax_bfd_print_valspec (const unsigned char *buf, unsigned int bufsize,
			int indent, FILE *file)
{
  if (bufsize < 5)
    return bufsize;

  unsigned int vflags = buf[0];
  unsigned int value = (unsigned) bfd_getl32 (buf + 1);
  unsigned int len = 5;

  evax_bfd_print_indent (indent, file);
  /* xgettext:c-format */
  fprintf (file, _("vflags: 0x%02x, value: 0x%08x "), vflags, value);
  buf += 5;
  bufsize -= 5;

  switch (vflags)
    {
    case DST__K_VFLAGS_NOVAL:
      fprintf (file, _("(no value)\n"));
      break;
    case DST__K_VFLAGS_NOTACTIVE:
      fprintf (file, _("(not active)\n"));
      break;
    case DST__K_VFLAGS_UNALLOC:
      fprintf (file, _("(not allocated)\n"));
      break;
    case DST__K_VFLAGS_DSC:
      fprintf (file, _("(descriptor)\n"));
      if (value <= bufsize)
	evax_bfd_print_desc (buf + value, bufsize - value, indent + 1, file);
      break;
    case DST__K_VFLAGS_TVS:
      fprintf (file, _("(trailing value)\n"));
      break;
    case DST__K_VS_FOLLOWS:
      fprintf (file, _("(value spec follows)\n"));
      break;
    case DST__K_VFLAGS_BITOFFS:
      fprintf (file, _("(at bit offset %u)\n"), value);
      break;
    default:
      /* xgettext:c-format */
      fprintf (file, _("(reg: %u, disp: %u, indir: %u, kind: "),
	       (vflags & DST__K_REGNUM_MASK) >> DST__K_REGNUM_SHIFT,
	       vflags & DST__K_DISP ? 1 : 0,
	       vflags & DST__K_INDIR ? 1 : 0);
      switch (vflags & DST__K_VALKIND_MASK)
	{
	case DST__K_VALKIND_LITERAL:
	  fputs (_("literal"), file);
	  break;
	case DST__K_VALKIND_ADDR:
	  fputs (_("address"), file);
	  break;
	case DST__K_VALKIND_DESC:
	  fputs (_("desc"), file);
	  break;
	case DST__K_VALKIND_REG:
	  fputs (_("reg"), file);
	  break;
	}
      fputs (")\n", file);
      break;
    }
  return len;
}

static void
evax_bfd_print_typspec (const unsigned char *buf, unsigned int bufsize,
			int indent, FILE *file)
{
  if (bufsize < 3)
    return;

  unsigned char kind = buf[2];
  unsigned int len = (unsigned) bfd_getl16 (buf);

  evax_bfd_print_indent (indent, file);
  /* xgettext:c-format */
  fprintf (file, _("len: %2u, kind: %2u "), len, kind);
  buf += 3;
  bufsize -= 3;
  switch (kind)
    {
    case DST__K_TS_ATOM:
    /* xgettext:c-format */
      if (bufsize >= 1)
	fprintf (file, _("atomic, type=0x%02x %s\n"),
		 buf[0], evax_bfd_get_dsc_name (buf[0]));
      break;
    case DST__K_TS_IND:
      if (bufsize >= 4)
	fprintf (file, _("indirect, defined at 0x%08x\n"),
		 (unsigned) bfd_getl32 (buf));
      break;
    case DST__K_TS_TPTR:
      fprintf (file, _("typed pointer\n"));
      evax_bfd_print_typspec (buf, bufsize, indent + 1, file);
      break;
    case DST__K_TS_PTR:
      fprintf (file, _("pointer\n"));
      break;
    case DST__K_TS_ARRAY:
      {
	const unsigned char *vs;
	unsigned int vs_len;
	unsigned int vec_len;
	unsigned int i;

	if (bufsize == 0)
	  return;
	fprintf (file, _("array, dim: %u, bitmap: "), buf[0]);
	--bufsize;
	vec_len = (buf[0] + 1 + 7) / 8;
	for (i = 0; i < vec_len; i++)
	  {
	    if (bufsize == 0)
	      break;
	    fprintf (file, " %02x", buf[i + 1]);
	    --bufsize;
	  }
	fputc ('\n', file);
	if (bufsize == 0)
	  return;
	vs = buf + 1 + vec_len;
	evax_bfd_print_indent (indent, file);
	fprintf (file, _("array descriptor:\n"));
	vs_len = evax_bfd_print_valspec (vs, bufsize, indent + 1, file);
	vs += vs_len;
	if (bufsize > vs_len)
	  {
	    bufsize -= vs_len;
	    for (i = 0; i < buf[0] + 1U; i++)
	      if (buf[1 + i / 8] & (1 << (i % 8)))
		{
		  evax_bfd_print_indent (indent, file);
		  if (i == 0)
		    fprintf (file, _("type spec for element:\n"));
		  else
		    fprintf (file, _("type spec for subscript %u:\n"), i);
		  evax_bfd_print_typspec (vs, bufsize, indent + 1, file);
		  if (bufsize < 2)
		    break;
		  vs_len = bfd_getl16 (vs);
		  if (bufsize <= vs_len)
		    break;
		  vs += vs_len;
		  bufsize -= vs_len;
		}
	  }
      }
      break;
    default:
      fprintf (file, _("*unhandled*\n"));
    }
}

static void
evax_bfd_print_dst (struct bfd *abfd, unsigned int dst_size, FILE *file)
{
  unsigned int off = 0;
  unsigned int pc = 0;
  unsigned int line = 0;

  fprintf (file, _("Debug symbol table:\n"));

  while (dst_size > 0)
    {
      struct vms_dst_header dsth;
      unsigned int len;
      unsigned int type;
      unsigned char *buf;

      if (bfd_bread (&dsth, sizeof (dsth), abfd) != sizeof (dsth))
	{
	  fprintf (file, _("cannot read DST header\n"));
	  return;
	}
      len = bfd_getl16 (dsth.length);
      type = bfd_getl16 (dsth.type);
      /* xgettext:c-format */
      fprintf (file, _(" type: %3u, len: %3u (at 0x%08x): "),
	       type, len, off);
      if (len < sizeof (dsth))
	{
	  fputc ('\n', file);
	  break;
	}
      dst_size -= len;
      off += len;
      len -= sizeof (dsth);
      if (len == 0)
	buf = NULL;
      else
	{
	  buf = _bfd_malloc_and_read (abfd, len, len);
	  if (buf == NULL)
	    {
	      fprintf (file, _("cannot read DST symbol\n"));
	      return;
	    }
	}
      switch (type)
	{
	case DSC__K_DTYPE_V:
	case DSC__K_DTYPE_BU:
	case DSC__K_DTYPE_WU:
	case DSC__K_DTYPE_LU:
	case DSC__K_DTYPE_QU:
	case DSC__K_DTYPE_B:
	case DSC__K_DTYPE_W:
	case DSC__K_DTYPE_L:
	case DSC__K_DTYPE_Q:
	case DSC__K_DTYPE_F:
	case DSC__K_DTYPE_D:
	case DSC__K_DTYPE_FC:
	case DSC__K_DTYPE_DC:
	case DSC__K_DTYPE_T:
	case DSC__K_DTYPE_NU:
	case DSC__K_DTYPE_NL:
	case DSC__K_DTYPE_NLO:
	case DSC__K_DTYPE_NR:
	case DSC__K_DTYPE_NRO:
	case DSC__K_DTYPE_NZ:
	case DSC__K_DTYPE_P:
	case DSC__K_DTYPE_ZI:
	case DSC__K_DTYPE_ZEM:
	case DSC__K_DTYPE_DSC:
	case DSC__K_DTYPE_OU:
	case DSC__K_DTYPE_O:
	case DSC__K_DTYPE_G:
	case DSC__K_DTYPE_H:
	case DSC__K_DTYPE_GC:
	case DSC__K_DTYPE_HC:
	case DSC__K_DTYPE_CIT:
	case DSC__K_DTYPE_BPV:
	case DSC__K_DTYPE_BLV:
	case DSC__K_DTYPE_VU:
	case DSC__K_DTYPE_ADT:
	case DSC__K_DTYPE_VT:
	case DSC__K_DTYPE_T2:
	case DSC__K_DTYPE_VT2:
	  fprintf (file, _("standard data: %s\n"),
		   evax_bfd_get_dsc_name (type));
	  evax_bfd_print_valspec (buf, len, 4, file);
	  if (len > 6)
	    fprintf (file, _("    name: %.*s\n"),
		     buf[5] > len - 6 ? len - 6 : buf[5], buf + 6);
	  break;
	case DST__K_MODBEG:
	  {
	    struct vms_dst_modbeg *dst = (void *)buf;
	    unsigned char *name = buf + sizeof (*dst);

	    fprintf (file, _("modbeg\n"));
	    if (len < sizeof (*dst))
	      break;
	    /* xgettext:c-format */
	    fprintf (file, _("   flags: %d, language: %u, "
			     "major: %u, minor: %u\n"),
		     dst->flags,
		     (unsigned)bfd_getl32 (dst->language),
		     (unsigned)bfd_getl16 (dst->major),
		     (unsigned)bfd_getl16 (dst->minor));
	    len -= sizeof (*dst);
	    if (len > 0)
	      {
		int nlen = len - 1;
		fprintf (file, _("   module name: %.*s\n"),
			 name[0] > nlen ? nlen : name[0], name + 1);
		if (name[0] < nlen)
		  {
		    len -= name[0] + 1;
		    name += name[0] + 1;
		    nlen = len - 1;
		    fprintf (file, _("   compiler   : %.*s\n"),
			     name[0] > nlen ? nlen : name[0], name + 1);
		  }
	      }
	  }
	  break;
	case DST__K_MODEND:
	  fprintf (file, _("modend\n"));
	  break;
	case DST__K_RTNBEG:
	  {
	    struct vms_dst_rtnbeg *dst = (void *)buf;
	    unsigned char *name = buf + sizeof (*dst);

	    fputs (_("rtnbeg\n"), file);
	    if (len >= sizeof (*dst))
	      {
		/* xgettext:c-format */
		fprintf (file, _("    flags: %u, address: 0x%08x, "
				 "pd-address: 0x%08x\n"),
			 dst->flags,
			 (unsigned) bfd_getl32 (dst->address),
			 (unsigned) bfd_getl32 (dst->pd_address));
		len -= sizeof (*dst);
		if (len > 0)
		  {
		    int nlen = len - 1;
		    fprintf (file, _("    routine name: %.*s\n"),
			     name[0] > nlen ? nlen : name[0], name + 1);
		  }
	      }
	  }
	  break;
	case DST__K_RTNEND:
	  {
	    struct vms_dst_rtnend *dst = (void *)buf;

	    if (len >= sizeof (*dst))
	      fprintf (file, _("rtnend: size 0x%08x\n"),
		       (unsigned) bfd_getl32 (dst->size));
	  }
	  break;
	case DST__K_PROLOG:
	  {
	    struct vms_dst_prolog *dst = (void *)buf;

	    if (len >= sizeof (*dst))
	      /* xgettext:c-format */
	      fprintf (file, _("prolog: bkpt address 0x%08x\n"),
		       (unsigned) bfd_getl32 (dst->bkpt_addr));
	  }
	  break;
	case DST__K_EPILOG:
	  {
	    struct vms_dst_epilog *dst = (void *)buf;

	    if (len >= sizeof (*dst))
	      /* xgettext:c-format */
	      fprintf (file, _("epilog: flags: %u, count: %u\n"),
		       dst->flags, (unsigned) bfd_getl32 (dst->count));
	  }
	  break;
	case DST__K_BLKBEG:
	  {
	    struct vms_dst_blkbeg *dst = (void *)buf;
	    unsigned char *name = buf + sizeof (*dst);

	    if (len > sizeof (*dst))
	      {
		int nlen;
		len -= sizeof (*dst);
		nlen = len - 1;
		/* xgettext:c-format */
		fprintf (file, _("blkbeg: address: 0x%08x, name: %.*s\n"),
			 (unsigned) bfd_getl32 (dst->address),
			 name[0] > nlen ? nlen : name[0], name + 1);
	      }
	  }
	  break;
	case DST__K_BLKEND:
	  {
	    struct vms_dst_blkend *dst = (void *)buf;

	    if (len >= sizeof (*dst))
	      /* xgettext:c-format */
	      fprintf (file, _("blkend: size: 0x%08x\n"),
		       (unsigned) bfd_getl32 (dst->size));
	  }
	  break;
	case DST__K_TYPSPEC:
	  {
	    fprintf (file, _("typspec (len: %u)\n"), len);
	    if (len >= 1)
	      {
		int nlen = len - 1;
		fprintf (file, _("    name: %.*s\n"),
			 buf[0] > nlen ? nlen : buf[0], buf + 1);
		if (nlen > buf[0])
		  evax_bfd_print_typspec (buf + 1 + buf[0], len - (1 + buf[0]),
					  5, file);
	      }
	  }
	  break;
	case DST__K_SEPTYP:
	  {
	    if (len >= 6)
	      {
		fprintf (file, _("septyp, name: %.*s\n"),
			 buf[5] > len - 6 ? len - 6 : buf[5], buf + 6);
		evax_bfd_print_valspec (buf, len, 4, file);
	      }
	  }
	  break;
	case DST__K_RECBEG:
	  {
	    struct vms_dst_recbeg *recbeg = (void *)buf;
	    unsigned char *name = buf + sizeof (*recbeg);

	    if (len > sizeof (*recbeg))
	      {
		int nlen = len - sizeof (*recbeg) - 1;
		if (name[0] < nlen)
		  nlen = name[0];
		fprintf (file, _("recbeg: name: %.*s\n"), nlen, name + 1);
		evax_bfd_print_valspec (buf, len, 4, file);
		len -= 1 + nlen;
		if (len >= 4)
		  fprintf (file, _("    len: %u bits\n"),
			   (unsigned) bfd_getl32 (name + 1 + nlen));
	      }
	  }
	  break;
	case DST__K_RECEND:
	  fprintf (file, _("recend\n"));
	  break;
	case DST__K_ENUMBEG:
	  if (len >= 2)
	    /* xgettext:c-format */
	    fprintf (file, _("enumbeg, len: %u, name: %.*s\n"),
		     buf[0], buf[1] > len - 2 ? len - 2 : buf[1], buf + 2);
	  break;
	case DST__K_ENUMELT:
	  if (len >= 6)
	    {
	      fprintf (file, _("enumelt, name: %.*s\n"),
		       buf[5] > len - 6 ? len - 6 : buf[5], buf + 6);
	      evax_bfd_print_valspec (buf, len, 4, file);
	    }
	  break;
	case DST__K_ENUMEND:
	  fprintf (file, _("enumend\n"));
	  break;
	case DST__K_LABEL:
	  {
	    struct vms_dst_label *lab = (void *)buf;
	    if (len >= sizeof (*lab))
	      {
		fprintf (file, _("label, name: %.*s\n"),
			 lab->name[0] > len - 1 ? len - 1 : lab->name[0],
			 lab->name + 1);
		fprintf (file, _("    address: 0x%08x\n"),
			 (unsigned) bfd_getl32 (lab->value));
	      }
	  }
	  break;
	case DST__K_DIS_RANGE:
	  if (len >= 4)
	    {
	      unsigned int cnt = bfd_getl32 (buf);
	      unsigned char *rng = buf + 4;
	      unsigned int i;

	      fprintf (file, _("discontiguous range (nbr: %u)\n"), cnt);
	      len -= 4;
	      for (i = 0; i < cnt; i++, rng += 8)
		{
		  if (len < 8)
		    break;
		  /* xgettext:c-format */
		  fprintf (file, _("    address: 0x%08x, size: %u\n"),
			   (unsigned) bfd_getl32 (rng),
			   (unsigned) bfd_getl32 (rng + 4));
		  len -= 8;
		}
	    }
	  break;
	case DST__K_LINE_NUM:
	  {
	    unsigned char *buf_orig = buf;

	    fprintf (file, _("line num  (len: %u)\n"), len);

	    while (len > 0)
	      {
		int cmd;
		unsigned int val;
		int cmdlen = -1;

		cmd = *buf++;
		len--;

		fputs ("    ", file);

		switch (cmd)
		  {
		  case DST__K_DELTA_PC_W:
		    if (len < 2)
		      break;
		    val = bfd_getl16 (buf);
		    fprintf (file, _("delta_pc_w %u\n"), val);
		    pc += val;
		    line++;
		    cmdlen = 2;
		    break;
		  case DST__K_INCR_LINUM:
		    if (len < 1)
		      break;
		    val = *buf;
		    fprintf (file, _("incr_linum(b): +%u\n"), val);
		    line += val;
		    cmdlen = 1;
		    break;
		  case DST__K_INCR_LINUM_W:
		    if (len < 2)
		      break;
		    val = bfd_getl16 (buf);
		    fprintf (file, _("incr_linum_w: +%u\n"), val);
		    line += val;
		    cmdlen = 2;
		    break;
		  case DST__K_INCR_LINUM_L:
		    if (len < 4)
		      break;
		    val = bfd_getl32 (buf);
		    fprintf (file, _("incr_linum_l: +%u\n"), val);
		    line += val;
		    cmdlen = 4;
		    break;
		  case DST__K_SET_LINUM:
		    if (len < 2)
		      break;
		    line = bfd_getl16 (buf);
		    fprintf (file, _("set_line_num(w) %u\n"), line);
		    cmdlen = 2;
		    break;
		  case DST__K_SET_LINUM_B:
		    if (len < 1)
		      break;
		    line = *buf;
		    fprintf (file, _("set_line_num_b %u\n"), line);
		    cmdlen = 1;
		    break;
		  case DST__K_SET_LINUM_L:
		    if (len < 4)
		      break;
		    line = bfd_getl32 (buf);
		    fprintf (file, _("set_line_num_l %u\n"), line);
		    cmdlen = 4;
		    break;
		  case DST__K_SET_ABS_PC:
		    if (len < 4)
		      break;
		    pc = bfd_getl32 (buf);
		    fprintf (file, _("set_abs_pc: 0x%08x\n"), pc);
		    cmdlen = 4;
		    break;
		  case DST__K_DELTA_PC_L:
		    if (len < 4)
		      break;
		    fprintf (file, _("delta_pc_l: +0x%08x\n"),
			     (unsigned) bfd_getl32 (buf));
		    cmdlen = 4;
		    break;
		  case DST__K_TERM:
		    if (len < 1)
		      break;
		    fprintf (file, _("term(b): 0x%02x"), *buf);
		    pc += *buf;
		    fprintf (file, _("        pc: 0x%08x\n"), pc);
		    cmdlen = 1;
		    break;
		  case DST__K_TERM_W:
		    if (len < 2)
		      break;
		    val = bfd_getl16 (buf);
		    fprintf (file, _("term_w: 0x%04x"), val);
		    pc += val;
		    fprintf (file, _("    pc: 0x%08x\n"), pc);
		    cmdlen = 2;
		    break;
		  default:
		    if (cmd <= 0)
		      {
			fprintf (file, _("delta pc +%-4d"), -cmd);
			line++;  /* FIXME: curr increment.  */
			pc += -cmd;
			/* xgettext:c-format */
			fprintf (file, _("    pc: 0x%08x line: %5u\n"),
				 pc, line);
			cmdlen = 0;
		      }
		    else
		      fprintf (file, _("    *unhandled* cmd %u\n"), cmd);
		    break;
		  }
		if (cmdlen < 0)
		  break;
		len -= cmdlen;
		buf += cmdlen;
	      }
	    buf = buf_orig;
	  }
	  break;
	case DST__K_SOURCE:
	  {
	    unsigned char *buf_orig = buf;

	    fprintf (file, _("source (len: %u)\n"), len);

	    while (len > 0)
	      {
		int cmd = *buf++;
		int cmdlen = -1;

		len--;
		switch (cmd)
		  {
		  case DST__K_SRC_DECLFILE:
		    {
		      struct vms_dst_src_decl_src *src = (void *) buf;
		      unsigned char *name;
		      int nlen;

		      if (len < sizeof (*src))
			break;
		      /* xgettext:c-format */
		      fprintf (file, _("   declfile: len: %u, flags: %u, "
				       "fileid: %u\n"),
			       src->length, src->flags,
			       (unsigned)bfd_getl16 (src->fileid));
		      /* xgettext:c-format */
		      fprintf (file, _("   rms: cdt: 0x%08x %08x, "
				       "ebk: 0x%08x, ffb: 0x%04x, "
				       "rfo: %u\n"),
			       (unsigned)bfd_getl32 (src->rms_cdt + 4),
			       (unsigned)bfd_getl32 (src->rms_cdt + 0),
			       (unsigned)bfd_getl32 (src->rms_ebk),
			       (unsigned)bfd_getl16 (src->rms_ffb),
			       src->rms_rfo);
		      if (src->length > len || src->length <= sizeof (*src))
			break;
		      nlen = src->length - sizeof (*src) - 1;
		      name = buf + sizeof (*src);
		      fprintf (file, _("   filename   : %.*s\n"),
			       name[0] > nlen ? nlen : name[0], name + 1);
		      if (name[0] >= nlen)
			break;
		      nlen -= name[0] + 1;
		      name += name[0] + 1;
		      fprintf (file, _("   module name: %.*s\n"),
			       name[0] > nlen ? nlen : name[0], name + 1);
		      if (name[0] > nlen)
			break;
		      cmdlen = src->length;
		    }
		    break;
		  case DST__K_SRC_SETFILE:
		    if (len < 2)
		      break;
		    fprintf (file, _("   setfile %u\n"),
			     (unsigned) bfd_getl16 (buf));
		    cmdlen = 2;
		    break;
		  case DST__K_SRC_SETREC_W:
		    if (len < 2)
		      break;
		    fprintf (file, _("   setrec %u\n"),
			     (unsigned) bfd_getl16 (buf));
		    cmdlen = 2;
		    break;
		  case DST__K_SRC_SETREC_L:
		    if (len < 4)
		      break;
		    fprintf (file, _("   setrec %u\n"),
			     (unsigned) bfd_getl32 (buf));
		    cmdlen = 4;
		    break;
		  case DST__K_SRC_SETLNUM_W:
		    if (len < 2)
		      break;
		    fprintf (file, _("   setlnum %u\n"),
			     (unsigned) bfd_getl16 (buf));
		    cmdlen = 2;
		    break;
		  case DST__K_SRC_SETLNUM_L:
		    if (len < 4)
		      break;
		    fprintf (file, _("   setlnum %u\n"),
			     (unsigned) bfd_getl32 (buf));
		    cmdlen = 4;
		    break;
		  case DST__K_SRC_DEFLINES_W:
		    if (len < 2)
		      break;
		    fprintf (file, _("   deflines %u\n"),
			     (unsigned) bfd_getl16 (buf));
		    cmdlen = 2;
		    break;
		  case DST__K_SRC_DEFLINES_B:
		    if (len < 1)
		      break;
		    fprintf (file, _("   deflines %u\n"), *buf);
		    cmdlen = 1;
		    break;
		  case DST__K_SRC_FORMFEED:
		    fprintf (file, _("   formfeed\n"));
		    cmdlen = 0;
		    break;
		  default:
		    fprintf (file, _("   *unhandled* cmd %u\n"), cmd);
		    break;
		  }
		if (cmdlen < 0)
		  break;
		len -= cmdlen;
		buf += cmdlen;
	      }
	    buf = buf_orig;
	  }
	  break;
	default:
	  fprintf (file, _("*unhandled* dst type %u\n"), type);
	  break;
	}
      free (buf);
    }
}

static void
evax_bfd_print_image (bfd *abfd, FILE *file)
{
  struct vms_eihd eihd;
  const char *name;
  unsigned int val;
  unsigned int eiha_off;
  unsigned int eihi_off;
  unsigned int eihs_off;
  unsigned int eisd_off;
  unsigned int eihef_off = 0;
  unsigned int eihnp_off = 0;
  unsigned int dmt_vbn = 0;
  unsigned int dmt_size = 0;
  unsigned int dst_vbn = 0;
  unsigned int dst_size = 0;
  unsigned int gst_vbn = 0;
  unsigned int gst_size = 0;
  unsigned int eiaf_vbn = 0;
  unsigned int eiaf_size = 0;
  unsigned int eihvn_off;

  if (bfd_seek (abfd, (file_ptr) 0, SEEK_SET)
      || bfd_bread (&eihd, sizeof (eihd), abfd) != sizeof (eihd))
    {
      fprintf (file, _("cannot read EIHD\n"));
      return;
    }
  /* xgettext:c-format */
  fprintf (file, _("EIHD: (size: %u, nbr blocks: %u)\n"),
	   (unsigned)bfd_getl32 (eihd.size),
	   (unsigned)bfd_getl32 (eihd.hdrblkcnt));
  /* xgettext:c-format */
  fprintf (file, _(" majorid: %u, minorid: %u\n"),
	   (unsigned)bfd_getl32 (eihd.majorid),
	   (unsigned)bfd_getl32 (eihd.minorid));

  val = (unsigned)bfd_getl32 (eihd.imgtype);
  switch (val)
    {
    case EIHD__K_EXE:
      name = _("executable");
      break;
    case EIHD__K_LIM:
      name = _("linkable image");
      break;
    default:
      name = _("unknown");
      break;
    }
  /* xgettext:c-format */
  fprintf (file, _(" image type: %u (%s)"), val, name);

  val = (unsigned)bfd_getl32 (eihd.subtype);
  switch (val)
    {
    case EIHD__C_NATIVE:
      name = _("native");
      break;
    case EIHD__C_CLI:
      name = _("CLI");
      break;
    default:
      name = _("unknown");
      break;
    }
  /* xgettext:c-format */
  fprintf (file, _(", subtype: %u (%s)\n"), val, name);

  eisd_off = bfd_getl32 (eihd.isdoff);
  eiha_off = bfd_getl32 (eihd.activoff);
  eihi_off = bfd_getl32 (eihd.imgidoff);
  eihs_off = bfd_getl32 (eihd.symdbgoff);
  /* xgettext:c-format */
  fprintf (file, _(" offsets: isd: %u, activ: %u, symdbg: %u, "
		   "imgid: %u, patch: %u\n"),
	   eisd_off, eiha_off, eihs_off, eihi_off,
	   (unsigned)bfd_getl32 (eihd.patchoff));
  fprintf (file, _(" fixup info rva: "));
  bfd_fprintf_vma (abfd, file, bfd_getl64 (eihd.iafva));
  fprintf (file, _(", symbol vector rva: "));
  bfd_fprintf_vma (abfd, file, bfd_getl64 (eihd.symvva));
  eihvn_off = bfd_getl32 (eihd.version_array_off);
  fprintf (file, _("\n"
		   " version array off: %u\n"),
	   eihvn_off);
  fprintf (file,
	   /* xgettext:c-format */
	   _(" img I/O count: %u, nbr channels: %u, req pri: %08x%08x\n"),
	   (unsigned)bfd_getl32 (eihd.imgiocnt),
	   (unsigned)bfd_getl32 (eihd.iochancnt),
	   (unsigned)bfd_getl32 (eihd.privreqs + 4),
	   (unsigned)bfd_getl32 (eihd.privreqs + 0));
  val = (unsigned)bfd_getl32 (eihd.lnkflags);
  fprintf (file, _(" linker flags: %08x:"), val);
  if (val & EIHD__M_LNKDEBUG)
    fprintf (file, " LNKDEBUG");
  if (val & EIHD__M_LNKNOTFR)
    fprintf (file, " LNKNOTFR");
  if (val & EIHD__M_NOP0BUFS)
    fprintf (file, " NOP0BUFS");
  if (val & EIHD__M_PICIMG)
    fprintf (file, " PICIMG");
  if (val & EIHD__M_P0IMAGE)
    fprintf (file, " P0IMAGE");
  if (val & EIHD__M_DBGDMT)
    fprintf (file, " DBGDMT");
  if (val & EIHD__M_INISHR)
    fprintf (file, " INISHR");
  if (val & EIHD__M_XLATED)
    fprintf (file, " XLATED");
  if (val & EIHD__M_BIND_CODE_SEC)
    fprintf (file, " BIND_CODE_SEC");
  if (val & EIHD__M_BIND_DATA_SEC)
    fprintf (file, " BIND_DATA_SEC");
  if (val & EIHD__M_MKTHREADS)
    fprintf (file, " MKTHREADS");
  if (val & EIHD__M_UPCALLS)
    fprintf (file, " UPCALLS");
  if (val & EIHD__M_OMV_READY)
    fprintf (file, " OMV_READY");
  if (val & EIHD__M_EXT_BIND_SECT)
    fprintf (file, " EXT_BIND_SECT");
  fprintf (file, "\n");
  /* xgettext:c-format */
  fprintf (file, _(" ident: 0x%08x, sysver: 0x%08x, "
		   "match ctrl: %u, symvect_size: %u\n"),
	   (unsigned)bfd_getl32 (eihd.ident),
	   (unsigned)bfd_getl32 (eihd.sysver),
	   eihd.matchctl,
	   (unsigned)bfd_getl32 (eihd.symvect_size));
  fprintf (file, _(" BPAGE: %u"),
	   (unsigned)bfd_getl32 (eihd.virt_mem_block_size));
  if (val & (EIHD__M_OMV_READY | EIHD__M_EXT_BIND_SECT))
    {
      eihef_off = bfd_getl32 (eihd.ext_fixup_off);
      eihnp_off = bfd_getl32 (eihd.noopt_psect_off);
      /* xgettext:c-format */
      fprintf (file, _(", ext fixup offset: %u, no_opt psect off: %u"),
	       eihef_off, eihnp_off);
    }
  fprintf (file, _(", alias: %u\n"), (unsigned)bfd_getl16 (eihd.alias));

  if (eihvn_off != 0)
    {
      struct vms_eihvn eihvn;
      unsigned int mask;
      unsigned int j;

      fprintf (file, _("system version array information:\n"));
      if (bfd_seek (abfd, (file_ptr) eihvn_off, SEEK_SET)
	  || bfd_bread (&eihvn, sizeof (eihvn), abfd) != sizeof (eihvn))
	{
	  fprintf (file, _("cannot read EIHVN header\n"));
	  return;
	}
      mask = bfd_getl32 (eihvn.subsystem_mask);
      for (j = 0; j < 32; j++)
	if (mask & (1u << j))
	  {
	    struct vms_eihvn_subversion ver;
	    if (bfd_bread (&ver, sizeof (ver), abfd) != sizeof (ver))
	      {
		fprintf (file, _("cannot read EIHVN version\n"));
		return;
	      }
	    fprintf (file, _("   %02u "), j);
	    switch (j)
	      {
	      case EIHVN__BASE_IMAGE_BIT:
		fputs (_("BASE_IMAGE       "), file);
		break;
	      case EIHVN__MEMORY_MANAGEMENT_BIT:
		fputs (_("MEMORY_MANAGEMENT"), file);
		break;
	      case EIHVN__IO_BIT:
		fputs (_("IO               "), file);
		break;
	      case EIHVN__FILES_VOLUMES_BIT:
		fputs (_("FILES_VOLUMES    "), file);
		break;
	      case EIHVN__PROCESS_SCHED_BIT:
		fputs (_("PROCESS_SCHED    "), file);
		break;
	      case EIHVN__SYSGEN_BIT:
		fputs (_("SYSGEN           "), file);
		break;
	      case EIHVN__CLUSTERS_LOCKMGR_BIT:
		fputs (_("CLUSTERS_LOCKMGR "), file);
		break;
	      case EIHVN__LOGICAL_NAMES_BIT:
		fputs (_("LOGICAL_NAMES    "), file);
		break;
	      case EIHVN__SECURITY_BIT:
		fputs (_("SECURITY         "), file);
		break;
	      case EIHVN__IMAGE_ACTIVATOR_BIT:
		fputs (_("IMAGE_ACTIVATOR  "), file);
		break;
	      case EIHVN__NETWORKS_BIT:
		fputs (_("NETWORKS         "), file);
		break;
	      case EIHVN__COUNTERS_BIT:
		fputs (_("COUNTERS         "), file);
		break;
	      case EIHVN__STABLE_BIT:
		fputs (_("STABLE           "), file);
		break;
	      case EIHVN__MISC_BIT:
		fputs (_("MISC             "), file);
		break;
	      case EIHVN__CPU_BIT:
		fputs (_("CPU              "), file);
		break;
	      case EIHVN__VOLATILE_BIT:
		fputs (_("VOLATILE         "), file);
		break;
	      case EIHVN__SHELL_BIT:
		fputs (_("SHELL            "), file);
		break;
	      case EIHVN__POSIX_BIT:
		fputs (_("POSIX            "), file);
		break;
	      case EIHVN__MULTI_PROCESSING_BIT:
		fputs (_("MULTI_PROCESSING "), file);
		break;
	      case EIHVN__GALAXY_BIT:
		fputs (_("GALAXY           "), file);
		break;
	      default:
		fputs (_("*unknown*        "), file);
		break;
	      }
	    fprintf (file, ": %u.%u\n",
		     (unsigned)bfd_getl16 (ver.major),
		     (unsigned)bfd_getl16 (ver.minor));
	  }
    }

  if (eiha_off != 0)
    {
      struct vms_eiha eiha;

      if (bfd_seek (abfd, (file_ptr) eiha_off, SEEK_SET)
	  || bfd_bread (&eiha, sizeof (eiha), abfd) != sizeof (eiha))
	{
	  fprintf (file, _("cannot read EIHA\n"));
	  return;
	}
      fprintf (file, _("Image activation:  (size=%u)\n"),
	       (unsigned)bfd_getl32 (eiha.size));
      /* xgettext:c-format */
      fprintf (file, _(" First address : 0x%08x 0x%08x\n"),
	       (unsigned)bfd_getl32 (eiha.tfradr1_h),
	       (unsigned)bfd_getl32 (eiha.tfradr1));
      /* xgettext:c-format */
      fprintf (file, _(" Second address: 0x%08x 0x%08x\n"),
	       (unsigned)bfd_getl32 (eiha.tfradr2_h),
	       (unsigned)bfd_getl32 (eiha.tfradr2));
      /* xgettext:c-format */
      fprintf (file, _(" Third address : 0x%08x 0x%08x\n"),
	       (unsigned)bfd_getl32 (eiha.tfradr3_h),
	       (unsigned)bfd_getl32 (eiha.tfradr3));
      /* xgettext:c-format */
      fprintf (file, _(" Fourth address: 0x%08x 0x%08x\n"),
	       (unsigned)bfd_getl32 (eiha.tfradr4_h),
	       (unsigned)bfd_getl32 (eiha.tfradr4));
      /* xgettext:c-format */
      fprintf (file, _(" Shared image  : 0x%08x 0x%08x\n"),
	       (unsigned)bfd_getl32 (eiha.inishr_h),
	       (unsigned)bfd_getl32 (eiha.inishr));
    }
  if (eihi_off != 0)
    {
      struct vms_eihi eihi;

      if (bfd_seek (abfd, (file_ptr) eihi_off, SEEK_SET)
	  || bfd_bread (&eihi, sizeof (eihi), abfd) != sizeof (eihi))
	{
	  fprintf (file, _("cannot read EIHI\n"));
	  return;
	}
      /* xgettext:c-format */
      fprintf (file, _("Image identification: (major: %u, minor: %u)\n"),
	       (unsigned)bfd_getl32 (eihi.majorid),
	       (unsigned)bfd_getl32 (eihi.minorid));
      fprintf (file, _(" image name       : %.*s\n"),
	       eihi.imgnam[0], eihi.imgnam + 1);
      fprintf (file, _(" link time        : %s\n"),
	       vms_time_to_str (eihi.linktime));
      fprintf (file, _(" image ident      : %.*s\n"),
	       eihi.imgid[0], eihi.imgid + 1);
      fprintf (file, _(" linker ident     : %.*s\n"),
	       eihi.linkid[0], eihi.linkid + 1);
      fprintf (file, _(" image build ident: %.*s\n"),
	       eihi.imgbid[0], eihi.imgbid + 1);
    }
  if (eihs_off != 0)
    {
      struct vms_eihs eihs;

      if (bfd_seek (abfd, (file_ptr) eihs_off, SEEK_SET)
	  || bfd_bread (&eihs, sizeof (eihs), abfd) != sizeof (eihs))
	{
	  fprintf (file, _("cannot read EIHS\n"));
	  return;
	}
      /* xgettext:c-format */
      fprintf (file, _("Image symbol & debug table: (major: %u, minor: %u)\n"),
	       (unsigned)bfd_getl32 (eihs.majorid),
	       (unsigned)bfd_getl32 (eihs.minorid));
      dst_vbn = bfd_getl32 (eihs.dstvbn);
      dst_size = bfd_getl32 (eihs.dstsize);
      /* xgettext:c-format */
      fprintf (file, _(" debug symbol table : vbn: %u, size: %u (0x%x)\n"),
	       dst_vbn, dst_size, dst_size);
      gst_vbn = bfd_getl32 (eihs.gstvbn);
      gst_size = bfd_getl32 (eihs.gstsize);
      /* xgettext:c-format */
      fprintf (file, _(" global symbol table: vbn: %u, records: %u\n"),
	       gst_vbn, gst_size);
      dmt_vbn = bfd_getl32 (eihs.dmtvbn);
      dmt_size = bfd_getl32 (eihs.dmtsize);
      /* xgettext:c-format */
      fprintf (file, _(" debug module table : vbn: %u, size: %u\n"),
	       dmt_vbn, dmt_size);
    }
  while (eisd_off != 0)
    {
      struct vms_eisd eisd;
      unsigned int len;

      while (1)
	{
	  if (bfd_seek (abfd, (file_ptr) eisd_off, SEEK_SET)
	      || bfd_bread (&eisd, sizeof (eisd), abfd) != sizeof (eisd))
	    {
	      fprintf (file, _("cannot read EISD\n"));
	      return;
	    }
	  len = (unsigned)bfd_getl32 (eisd.eisdsize);
	  if (len != (unsigned)-1)
	    break;

	  /* Next block.  */
	  eisd_off = (eisd_off + VMS_BLOCK_SIZE) & ~(VMS_BLOCK_SIZE - 1);
	}
      /* xgettext:c-format */
      fprintf (file, _("Image section descriptor: (major: %u, minor: %u, "
		       "size: %u, offset: %u)\n"),
	       (unsigned)bfd_getl32 (eisd.majorid),
	       (unsigned)bfd_getl32 (eisd.minorid),
	       len, eisd_off);
      if (len == 0)
	break;
      /* xgettext:c-format */
      fprintf (file, _(" section: base: 0x%08x%08x size: 0x%08x\n"),
	       (unsigned)bfd_getl32 (eisd.virt_addr + 4),
	       (unsigned)bfd_getl32 (eisd.virt_addr + 0),
	       (unsigned)bfd_getl32 (eisd.secsize));
      val = (unsigned)bfd_getl32 (eisd.flags);
      fprintf (file, _(" flags: 0x%04x"), val);
      if (val & EISD__M_GBL)
	fprintf (file, " GBL");
      if (val & EISD__M_CRF)
	fprintf (file, " CRF");
      if (val & EISD__M_DZRO)
	fprintf (file, " DZRO");
      if (val & EISD__M_WRT)
	fprintf (file, " WRT");
      if (val & EISD__M_INITALCODE)
	fprintf (file, " INITALCODE");
      if (val & EISD__M_BASED)
	fprintf (file, " BASED");
      if (val & EISD__M_FIXUPVEC)
	fprintf (file, " FIXUPVEC");
      if (val & EISD__M_RESIDENT)
	fprintf (file, " RESIDENT");
      if (val & EISD__M_VECTOR)
	fprintf (file, " VECTOR");
      if (val & EISD__M_PROTECT)
	fprintf (file, " PROTECT");
      if (val & EISD__M_LASTCLU)
	fprintf (file, " LASTCLU");
      if (val & EISD__M_EXE)
	fprintf (file, " EXE");
      if (val & EISD__M_NONSHRADR)
	fprintf (file, " NONSHRADR");
      if (val & EISD__M_QUAD_LENGTH)
	fprintf (file, " QUAD_LENGTH");
      if (val & EISD__M_ALLOC_64BIT)
	fprintf (file, " ALLOC_64BIT");
      fprintf (file, "\n");
      if (val & EISD__M_FIXUPVEC)
	{
	  eiaf_vbn = bfd_getl32 (eisd.vbn);
	  eiaf_size = bfd_getl32 (eisd.secsize);
	}
      /* xgettext:c-format */
      fprintf (file, _(" vbn: %u, pfc: %u, matchctl: %u type: %u ("),
	       (unsigned)bfd_getl32 (eisd.vbn),
	       eisd.pfc, eisd.matchctl, eisd.type);
      switch (eisd.type)
	{
	case EISD__K_NORMAL:
	  fputs (_("NORMAL"), file);
	  break;
	case EISD__K_SHRFXD:
	  fputs (_("SHRFXD"), file);
	  break;
	case EISD__K_PRVFXD:
	  fputs (_("PRVFXD"), file);
	  break;
	case EISD__K_SHRPIC:
	  fputs (_("SHRPIC"), file);
	  break;
	case EISD__K_PRVPIC:
	  fputs (_("PRVPIC"), file);
	  break;
	case EISD__K_USRSTACK:
	  fputs (_("USRSTACK"), file);
	  break;
	default:
	  fputs (_("*unknown*"), file);
	  break;
	}
      fputs (_(")\n"), file);
      if (val & EISD__M_GBL)
	/* xgettext:c-format */
	fprintf (file, _(" ident: 0x%08x, name: %.*s\n"),
		 (unsigned)bfd_getl32 (eisd.ident),
		 eisd.gblnam[0], eisd.gblnam + 1);
      eisd_off += len;
    }

  if (dmt_vbn != 0)
    {
      if (bfd_seek (abfd, (file_ptr) (dmt_vbn - 1) * VMS_BLOCK_SIZE, SEEK_SET))
	{
	  fprintf (file, _("cannot read DMT\n"));
	  return;
	}

      fprintf (file, _("Debug module table:\n"));

      while (dmt_size > 0)
	{
	  struct vms_dmt_header dmth;
	  unsigned int count;

	  if (bfd_bread (&dmth, sizeof (dmth), abfd) != sizeof (dmth))
	    {
	      fprintf (file, _("cannot read DMT header\n"));
	      return;
	    }
	  count = bfd_getl16 (dmth.psect_count);
	  fprintf (file,
		   /* xgettext:c-format */
		   _(" module offset: 0x%08x, size: 0x%08x, (%u psects)\n"),
		   (unsigned)bfd_getl32 (dmth.modbeg),
		   (unsigned)bfd_getl32 (dmth.size), count);
	  dmt_size -= sizeof (dmth);
	  while (count > 0)
	    {
	      struct vms_dmt_psect dmtp;

	      if (bfd_bread (&dmtp, sizeof (dmtp), abfd) != sizeof (dmtp))
		{
		  fprintf (file, _("cannot read DMT psect\n"));
		  return;
		}
	      /* xgettext:c-format */
	      fprintf (file, _("  psect start: 0x%08x, length: %u\n"),
		       (unsigned)bfd_getl32 (dmtp.start),
		       (unsigned)bfd_getl32 (dmtp.length));
	      count--;
	      dmt_size -= sizeof (dmtp);
	    }
	}
    }

  if (dst_vbn != 0)
    {
      if (bfd_seek (abfd, (file_ptr) (dst_vbn - 1) * VMS_BLOCK_SIZE, SEEK_SET))
	{
	  fprintf (file, _("cannot read DST\n"));
	  return;
	}

      evax_bfd_print_dst (abfd, dst_size, file);
    }
  if (gst_vbn != 0)
    {
      if (bfd_seek (abfd, (file_ptr) (gst_vbn - 1) * VMS_BLOCK_SIZE, SEEK_SET))
	{
	  fprintf (file, _("cannot read GST\n"));
	  return;
	}

      fprintf (file, _("Global symbol table:\n"));
      evax_bfd_print_eobj (abfd, file);
    }
  if (eiaf_vbn != 0 && eiaf_size >= sizeof (struct vms_eiaf))
    {
      unsigned char *buf;
      struct vms_eiaf *eiaf;
      unsigned int qrelfixoff;
      unsigned int lrelfixoff;
      unsigned int qdotadroff;
      unsigned int ldotadroff;
      unsigned int shrimgcnt;
      unsigned int shlstoff;
      unsigned int codeadroff;
      unsigned int lpfixoff;
      unsigned int chgprtoff;
      file_ptr f_off = (file_ptr) (eiaf_vbn - 1) * VMS_BLOCK_SIZE;

      if (bfd_seek (abfd, f_off, SEEK_SET) != 0
	  || (buf = _bfd_malloc_and_read (abfd, eiaf_size, eiaf_size)) == NULL)
	{
	  fprintf (file, _("cannot read EIHA\n"));
	  return;
	}
      eiaf = (struct vms_eiaf *)buf;
      fprintf (file,
	       /* xgettext:c-format */
	       _("Image activator fixup: (major: %u, minor: %u)\n"),
	       (unsigned)bfd_getl32 (eiaf->majorid),
	       (unsigned)bfd_getl32 (eiaf->minorid));
      /* xgettext:c-format */
      fprintf (file, _("  iaflink : 0x%08x %08x\n"),
	       (unsigned)bfd_getl32 (eiaf->iaflink + 0),
	       (unsigned)bfd_getl32 (eiaf->iaflink + 4));
      /* xgettext:c-format */
      fprintf (file, _("  fixuplnk: 0x%08x %08x\n"),
	       (unsigned)bfd_getl32 (eiaf->fixuplnk + 0),
	       (unsigned)bfd_getl32 (eiaf->fixuplnk + 4));
      fprintf (file, _("  size : %u\n"),
	       (unsigned)bfd_getl32 (eiaf->size));
      fprintf (file, _("  flags: 0x%08x\n"),
	       (unsigned)bfd_getl32 (eiaf->flags));
      qrelfixoff = bfd_getl32 (eiaf->qrelfixoff);
      lrelfixoff = bfd_getl32 (eiaf->lrelfixoff);
      /* xgettext:c-format */
      fprintf (file, _("  qrelfixoff: %5u, lrelfixoff: %5u\n"),
	       qrelfixoff, lrelfixoff);
      qdotadroff = bfd_getl32 (eiaf->qdotadroff);
      ldotadroff = bfd_getl32 (eiaf->ldotadroff);
      /* xgettext:c-format */
      fprintf (file, _("  qdotadroff: %5u, ldotadroff: %5u\n"),
	       qdotadroff, ldotadroff);
      codeadroff = bfd_getl32 (eiaf->codeadroff);
      lpfixoff = bfd_getl32 (eiaf->lpfixoff);
      /* xgettext:c-format */
      fprintf (file, _("  codeadroff: %5u, lpfixoff  : %5u\n"),
	       codeadroff, lpfixoff);
      chgprtoff = bfd_getl32 (eiaf->chgprtoff);
      fprintf (file, _("  chgprtoff : %5u\n"), chgprtoff);
      shrimgcnt = bfd_getl32 (eiaf->shrimgcnt);
      shlstoff = bfd_getl32 (eiaf->shlstoff);
      /* xgettext:c-format */
      fprintf (file, _("  shlstoff  : %5u, shrimgcnt : %5u\n"),
	       shlstoff, shrimgcnt);
      /* xgettext:c-format */
      fprintf (file, _("  shlextra  : %5u, permctx   : %5u\n"),
	       (unsigned)bfd_getl32 (eiaf->shlextra),
	       (unsigned)bfd_getl32 (eiaf->permctx));
      fprintf (file, _("  base_va : 0x%08x\n"),
	       (unsigned)bfd_getl32 (eiaf->base_va));
      fprintf (file, _("  lppsbfixoff: %5u\n"),
	       (unsigned)bfd_getl32 (eiaf->lppsbfixoff));

      if (shlstoff)
	{
	  unsigned int j;

	  fprintf (file, _(" Shareable images:\n"));
	  for (j = 0;
	       j < shrimgcnt && shlstoff <= eiaf_size - sizeof (struct vms_shl);
	       j++, shlstoff += sizeof (struct vms_shl))
	    {
	      struct vms_shl *shl = (struct vms_shl *) (buf + shlstoff);
	      fprintf (file,
		       /* xgettext:c-format */
		       _("  %u: size: %u, flags: 0x%02x, name: %.*s\n"),
		       j, shl->size, shl->flags,
		       shl->imgnam[0], shl->imgnam + 1);
	    }
	}
      if (qrelfixoff != 0)
	{
	  fprintf (file, _(" quad-word relocation fixups:\n"));
	  evax_bfd_print_relocation_records (file, buf, eiaf_size,
					     qrelfixoff, 8);
	}
      if (lrelfixoff != 0)
	{
	  fprintf (file, _(" long-word relocation fixups:\n"));
	  evax_bfd_print_relocation_records (file, buf, eiaf_size,
					     lrelfixoff, 4);
	}
      if (qdotadroff != 0)
	{
	  fprintf (file, _(" quad-word .address reference fixups:\n"));
	  evax_bfd_print_address_fixups (file, buf, eiaf_size, qdotadroff);
	}
      if (ldotadroff != 0)
	{
	  fprintf (file, _(" long-word .address reference fixups:\n"));
	  evax_bfd_print_address_fixups (file, buf, eiaf_size, ldotadroff);
	}
      if (codeadroff != 0)
	{
	  fprintf (file, _(" Code Address Reference Fixups:\n"));
	  evax_bfd_print_reference_fixups (file, buf, eiaf_size, codeadroff);
	}
      if (lpfixoff != 0)
	{
	  fprintf (file, _(" Linkage Pairs Reference Fixups:\n"));
	  evax_bfd_print_reference_fixups (file, buf, eiaf_size, lpfixoff);
	}
      if (chgprtoff && chgprtoff <= eiaf_size - 4)
	{
	  unsigned int count = (unsigned) bfd_getl32 (buf + chgprtoff);
	  unsigned int j;

	  fprintf (file, _(" Change Protection (%u entries):\n"), count);
	  for (j = 0, chgprtoff += 4;
	       j < count && chgprtoff <= eiaf_size - sizeof (struct vms_eicp);
	       j++, chgprtoff += sizeof (struct vms_eicp))
	    {
	      struct vms_eicp *eicp = (struct vms_eicp *) (buf + chgprtoff);
	      unsigned int prot = bfd_getl32 (eicp->newprt);
	      fprintf (file,
		       /* xgettext:c-format */
		       _("  base: 0x%08x %08x, size: 0x%08x, prot: 0x%08x "),
		       (unsigned) bfd_getl32 (eicp->baseva + 4),
		       (unsigned) bfd_getl32 (eicp->baseva + 0),
		       (unsigned) bfd_getl32 (eicp->size),
		       (unsigned) bfd_getl32 (eicp->newprt));
	      switch (prot)
		{
		case PRT__C_NA:
		  fprintf (file, "NA");
		  break;
		case PRT__C_RESERVED:
		  fprintf (file, "RES");
		  break;
		case PRT__C_KW:
		  fprintf (file, "KW");
		  break;
		case PRT__C_KR:
		  fprintf (file, "KR");
		  break;
		case PRT__C_UW:
		  fprintf (file, "UW");
		  break;
		case PRT__C_EW:
		  fprintf (file, "EW");
		  break;
		case PRT__C_ERKW:
		  fprintf (file, "ERKW");
		  break;
		case PRT__C_ER:
		  fprintf (file, "ER");
		  break;
		case PRT__C_SW:
		  fprintf (file, "SW");
		  break;
		case PRT__C_SREW:
		  fprintf (file, "SREW");
		  break;
		case PRT__C_SRKW:
		  fprintf (file, "SRKW");
		  break;
		case PRT__C_SR:
		  fprintf (file, "SR");
		  break;
		case PRT__C_URSW:
		  fprintf (file, "URSW");
		  break;
		case PRT__C_UREW:
		  fprintf (file, "UREW");
		  break;
		case PRT__C_URKW:
		  fprintf (file, "URKW");
		  break;
		case PRT__C_UR:
		  fprintf (file, "UR");
		  break;
		default:
		  fputs ("??", file);
		  break;
		}
	      fputc ('\n', file);
	    }
	}
      free (buf);
    }
}

static bool
vms_bfd_print_private_bfd_data (bfd *abfd, void *ptr)
{
  FILE *file = (FILE *)ptr;

  if (bfd_get_file_flags (abfd) & (EXEC_P | DYNAMIC))
    evax_bfd_print_image (abfd, file);
  else
    {
      if (bfd_seek (abfd, 0, SEEK_SET))
	return false;
      evax_bfd_print_eobj (abfd, file);
    }
  return true;
}

/* Linking.  */

/* Slurp ETIR/EDBG/ETBT VMS object records.  */

static bool
alpha_vms_read_sections_content (bfd *abfd, struct bfd_link_info *info)
{
  asection *cur_section;
  file_ptr cur_offset;
  asection *dst_section;
  file_ptr dst_offset;

  if (bfd_seek (abfd, 0, SEEK_SET) != 0)
    return false;

  cur_section = NULL;
  cur_offset = 0;

  dst_section = PRIV (dst_section);
  dst_offset = 0;
  if (info)
    {
      if (info->strip == strip_all || info->strip == strip_debugger)
	{
	  /* Discard the DST section.  */
	  dst_offset = 0;
	  dst_section = NULL;
	}
      else if (dst_section)
	{
	  dst_offset = dst_section->output_offset;
	  dst_section = dst_section->output_section;
	}
    }

  while (1)
    {
      int type;
      bool res;

      type = _bfd_vms_get_object_record (abfd);
      if (type < 0)
	{
	  vms_debug2 ((2, "next_record failed\n"));
	  return false;
	}
      switch (type)
	{
	case EOBJ__C_ETIR:
	  PRIV (image_section) = cur_section;
	  PRIV (image_offset) = cur_offset;
	  res = _bfd_vms_slurp_etir (abfd, info);
	  cur_section = PRIV (image_section);
	  cur_offset = PRIV (image_offset);
	  break;
	case EOBJ__C_EDBG:
	case EOBJ__C_ETBT:
	  if (dst_section == NULL)
	    continue;
	  PRIV (image_section) = dst_section;
	  PRIV (image_offset) = dst_offset;
	  res = _bfd_vms_slurp_etir (abfd, info);
	  dst_offset = PRIV (image_offset);
	  break;
	case EOBJ__C_EEOM:
	  return true;
	default:
	  continue;
	}
      if (!res)
	{
	  vms_debug2 ((2, "slurp eobj type %d failed\n", type));
	  return false;
	}
    }
}

static int
alpha_vms_sizeof_headers (bfd *abfd ATTRIBUTE_UNUSED,
			  struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  return 0;
}

/* Add a linkage pair fixup at address SECT + OFFSET to SHLIB. */

static bool
alpha_vms_add_fixup_lp (struct bfd_link_info *info, bfd *src, bfd *shlib)
{
  struct alpha_vms_shlib_el *sl;
  asection *sect = PRIV2 (src, image_section);
  file_ptr offset = PRIV2 (src, image_offset);
  bfd_vma *p;

  sl = &VEC_EL (alpha_vms_link_hash (info)->shrlibs,
		struct alpha_vms_shlib_el, PRIV2 (shlib, shr_index));
  sl->has_fixups = true;
  p = VEC_APPEND (sl->lp, bfd_vma);
  if (p == NULL)
    return false;
  *p = sect->output_section->vma + sect->output_offset + offset;
  sect->output_section->flags |= SEC_RELOC;
  return true;
}

/* Add a code address fixup at address SECT + OFFSET to SHLIB. */

static bool
alpha_vms_add_fixup_ca (struct bfd_link_info *info, bfd *src, bfd *shlib)
{
  struct alpha_vms_shlib_el *sl;
  asection *sect = PRIV2 (src, image_section);
  file_ptr offset = PRIV2 (src, image_offset);
  bfd_vma *p;

  sl = &VEC_EL (alpha_vms_link_hash (info)->shrlibs,
		struct alpha_vms_shlib_el, PRIV2 (shlib, shr_index));
  sl->has_fixups = true;
  p = VEC_APPEND (sl->ca, bfd_vma);
  if (p == NULL)
    return false;
  *p = sect->output_section->vma + sect->output_offset + offset;
  sect->output_section->flags |= SEC_RELOC;
  return true;
}

/* Add a quad word relocation fixup at address SECT + OFFSET to SHLIB. */

static bool
alpha_vms_add_fixup_qr (struct bfd_link_info *info, bfd *src,
			bfd *shlib, bfd_vma vec)
{
  struct alpha_vms_shlib_el *sl;
  struct alpha_vms_vma_ref *r;
  asection *sect = PRIV2 (src, image_section);
  file_ptr offset = PRIV2 (src, image_offset);

  sl = &VEC_EL (alpha_vms_link_hash (info)->shrlibs,
		struct alpha_vms_shlib_el, PRIV2 (shlib, shr_index));
  sl->has_fixups = true;
  r = VEC_APPEND (sl->qr, struct alpha_vms_vma_ref);
  if (r == NULL)
    return false;
  r->vma = sect->output_section->vma + sect->output_offset + offset;
  r->ref = vec;
  sect->output_section->flags |= SEC_RELOC;
  return true;
}

static bool
alpha_vms_add_fixup_lr (struct bfd_link_info *info ATTRIBUTE_UNUSED,
			unsigned int shr ATTRIBUTE_UNUSED,
			bfd_vma vec ATTRIBUTE_UNUSED)
{
  /* Not yet supported.  */
  return false;
}

/* Add relocation.  FIXME: Not yet emitted.  */

static bool
alpha_vms_add_lw_reloc (struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  return false;
}

static bool
alpha_vms_add_qw_reloc (struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  return false;
}

static struct bfd_hash_entry *
alpha_vms_link_hash_newfunc (struct bfd_hash_entry *entry,
			     struct bfd_hash_table *table,
			     const char *string)
{
  struct alpha_vms_link_hash_entry *ret =
    (struct alpha_vms_link_hash_entry *) entry;

  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (ret == NULL)
    ret = ((struct alpha_vms_link_hash_entry *)
	   bfd_hash_allocate (table,
			      sizeof (struct alpha_vms_link_hash_entry)));
  if (ret == NULL)
    return NULL;

  /* Call the allocation method of the superclass.  */
  ret = ((struct alpha_vms_link_hash_entry *)
	 _bfd_link_hash_newfunc ((struct bfd_hash_entry *) ret,
				 table, string));

  ret->sym = NULL;

  return (struct bfd_hash_entry *) ret;
}

static void
alpha_vms_bfd_link_hash_table_free (bfd *abfd)
{
  struct alpha_vms_link_hash_table *t;
  unsigned i;

  t = (struct alpha_vms_link_hash_table *) abfd->link.hash;
  for (i = 0; i < VEC_COUNT (t->shrlibs); i++)
    {
      struct alpha_vms_shlib_el *shlib;

      shlib = &VEC_EL (t->shrlibs, struct alpha_vms_shlib_el, i);
      free (&VEC_EL (shlib->ca, bfd_vma, 0));
      free (&VEC_EL (shlib->lp, bfd_vma, 0));
      free (&VEC_EL (shlib->qr, struct alpha_vms_vma_ref, 0));
    }
  free (&VEC_EL (t->shrlibs, struct alpha_vms_shlib_el, 0));

  _bfd_generic_link_hash_table_free (abfd);
}

/* Create an Alpha/VMS link hash table.  */

static struct bfd_link_hash_table *
alpha_vms_bfd_link_hash_table_create (bfd *abfd)
{
  struct alpha_vms_link_hash_table *ret;
  size_t amt = sizeof (struct alpha_vms_link_hash_table);

  ret = (struct alpha_vms_link_hash_table *) bfd_malloc (amt);
  if (ret == NULL)
    return NULL;
  if (!_bfd_link_hash_table_init (&ret->root, abfd,
				  alpha_vms_link_hash_newfunc,
				  sizeof (struct alpha_vms_link_hash_entry)))
    {
      free (ret);
      return NULL;
    }

  VEC_INIT (ret->shrlibs);
  ret->fixup = NULL;
  ret->root.hash_table_free = alpha_vms_bfd_link_hash_table_free;

  return &ret->root;
}

static bool
alpha_vms_link_add_object_symbols (bfd *abfd, struct bfd_link_info *info)
{
  unsigned int i;

  for (i = 0; i < PRIV (gsd_sym_count); i++)
    {
      struct vms_symbol_entry *e = PRIV (syms)[i];
      struct alpha_vms_link_hash_entry *h;
      struct bfd_link_hash_entry *h_root;
      asymbol sym;

      if (!alpha_vms_convert_symbol (abfd, e, &sym))
	return false;

      if ((e->flags & EGSY__V_DEF) && abfd->selective_search)
	{
	  /* In selective_search mode, only add definition that are
	     required.  */
	  h = (struct alpha_vms_link_hash_entry *)bfd_link_hash_lookup
	    (info->hash, sym.name, false, false, false);
	  if (h == NULL || h->root.type != bfd_link_hash_undefined)
	    continue;
	}
      else
	h = NULL;

      h_root = (struct bfd_link_hash_entry *) h;
      if (!_bfd_generic_link_add_one_symbol (info, abfd, sym.name, sym.flags,
					     sym.section, sym.value, NULL,
					     false, false, &h_root))
	return false;
      h = (struct alpha_vms_link_hash_entry *) h_root;

      if ((e->flags & EGSY__V_DEF)
	  && h->sym == NULL
	  && abfd->xvec == info->output_bfd->xvec)
	h->sym = e;
    }

  if (abfd->flags & DYNAMIC)
    {
      struct alpha_vms_shlib_el *shlib;

      /* We do not want to include any of the sections in a dynamic
	 object in the output file.  See comment in elflink.c.  */
      bfd_section_list_clear (abfd);

      shlib = VEC_APPEND (alpha_vms_link_hash (info)->shrlibs,
			  struct alpha_vms_shlib_el);
      if (shlib == NULL)
	return false;
      shlib->abfd = abfd;
      VEC_INIT (shlib->ca);
      VEC_INIT (shlib->lp);
      VEC_INIT (shlib->qr);
      PRIV (shr_index) = VEC_COUNT (alpha_vms_link_hash (info)->shrlibs) - 1;
    }

  return true;
}

static bool
alpha_vms_link_add_archive_symbols (bfd *abfd, struct bfd_link_info *info)
{
  int pass;
  struct bfd_link_hash_entry **pundef;
  struct bfd_link_hash_entry **next_pundef;

  /* We only accept VMS libraries.  */
  if (info->output_bfd->xvec != abfd->xvec)
    {
      bfd_set_error (bfd_error_wrong_format);
      return false;
    }

  /* The archive_pass field in the archive itself is used to
     initialize PASS, since we may search the same archive multiple
     times.  */
  pass = ++abfd->archive_pass;

  /* Look through the list of undefined symbols.  */
  for (pundef = &info->hash->undefs; *pundef != NULL; pundef = next_pundef)
    {
      struct bfd_link_hash_entry *h;
      symindex symidx;
      bfd *element;
      bfd *orig_element;

      h = *pundef;
      next_pundef = &(*pundef)->u.undef.next;

      /* When a symbol is defined, it is not necessarily removed from
	 the list.  */
      if (h->type != bfd_link_hash_undefined
	  && h->type != bfd_link_hash_common)
	{
	  /* Remove this entry from the list, for general cleanliness
	     and because we are going to look through the list again
	     if we search any more libraries.  We can't remove the
	     entry if it is the tail, because that would lose any
	     entries we add to the list later on.  */
	  if (*pundef != info->hash->undefs_tail)
	    {
	      *pundef = *next_pundef;
	      next_pundef = pundef;
	    }
	  continue;
	}

      /* Look for this symbol in the archive hash table.  */
      symidx = _bfd_vms_lib_find_symbol (abfd, h->root.string);
      if (symidx == BFD_NO_MORE_SYMBOLS)
	{
	  /* Nothing in this slot.  */
	  continue;
	}

      element = bfd_get_elt_at_index (abfd, symidx);
      if (element == NULL)
	return false;

      if (element->archive_pass == -1 || element->archive_pass == pass)
	{
	  /* Next symbol if this archive is wrong or already handled.  */
	  continue;
	}

      if (! bfd_check_format (element, bfd_object))
	{
	  element->archive_pass = -1;
	  return false;
	}

      orig_element = element;
      if (bfd_is_thin_archive (abfd))
	{
	  element = _bfd_vms_lib_get_imagelib_file (element);
	  if (element == NULL || !bfd_check_format (element, bfd_object))
	    {
	      orig_element->archive_pass = -1;
	      return false;
	    }
	}

      /* Unlike the generic linker, we know that this element provides
	 a definition for an undefined symbol and we know that we want
	 to include it.  We don't need to check anything.  */
      if (!(*info->callbacks
	    ->add_archive_element) (info, element, h->root.string, &element))
	continue;
      if (!alpha_vms_link_add_object_symbols (element, info))
	return false;

      orig_element->archive_pass = pass;
    }

  return true;
}

static bool
alpha_vms_bfd_link_add_symbols (bfd *abfd, struct bfd_link_info *info)
{
  switch (bfd_get_format (abfd))
    {
    case bfd_object:
      vms_debug2 ((2, "vms_link_add_symbols for object %s\n",
		   abfd->filename));
      return alpha_vms_link_add_object_symbols (abfd, info);
      break;
    case bfd_archive:
      vms_debug2 ((2, "vms_link_add_symbols for archive %s\n",
		   abfd->filename));
      return alpha_vms_link_add_archive_symbols (abfd, info);
      break;
    default:
      bfd_set_error (bfd_error_wrong_format);
      return false;
    }
}

static bool
alpha_vms_build_fixups (struct bfd_link_info *info)
{
  struct alpha_vms_link_hash_table *t = alpha_vms_link_hash (info);
  unsigned char *content;
  unsigned int i;
  unsigned int sz = 0;
  unsigned int lp_sz = 0;
  unsigned int ca_sz = 0;
  unsigned int qr_sz = 0;
  unsigned int shrimg_cnt = 0;
  unsigned int chgprt_num = 0;
  unsigned int chgprt_sz = 0;
  struct vms_eiaf *eiaf;
  unsigned int off;
  asection *sec;

  /* Shared libraries.  */
  for (i = 0; i < VEC_COUNT (t->shrlibs); i++)
    {
      struct alpha_vms_shlib_el *shlib;

      shlib = &VEC_EL (t->shrlibs, struct alpha_vms_shlib_el, i);

      if (!shlib->has_fixups)
	continue;

      shrimg_cnt++;

      if (VEC_COUNT (shlib->ca) > 0)
	{
	  /* Header + entries.  */
	  ca_sz += 8;
	  ca_sz += VEC_COUNT (shlib->ca) * 4;
	}
      if (VEC_COUNT (shlib->lp) > 0)
	{
	  /* Header + entries.  */
	  lp_sz += 8;
	  lp_sz += VEC_COUNT (shlib->lp) * 4;
	}
      if (VEC_COUNT (shlib->qr) > 0)
	{
	  /* Header + entries.  */
	  qr_sz += 8;
	  qr_sz += VEC_COUNT (shlib->qr) * 8;
	}
    }
  /* Add markers.  */
  if (ca_sz > 0)
    ca_sz += 8;
  if (lp_sz > 0)
    lp_sz += 8;
  if (qr_sz > 0)
    qr_sz += 8;

  /* Finish now if there is no content.  */
  if (ca_sz + lp_sz + qr_sz == 0)
    return true;

  /* Add an eicp entry for the fixup itself.  */
  chgprt_num = 1;
  for (sec = info->output_bfd->sections; sec != NULL; sec = sec->next)
    {
      /* This isect could be made RO or EXE after relocations are applied.  */
      if ((sec->flags & SEC_RELOC) != 0
	  && (sec->flags & (SEC_CODE | SEC_READONLY)) != 0)
	chgprt_num++;
    }
  chgprt_sz = 4 + chgprt_num * sizeof (struct vms_eicp);

  /* Allocate section content (round-up size)  */
  sz = sizeof (struct vms_eiaf) + shrimg_cnt * sizeof (struct vms_shl)
    + ca_sz + lp_sz + qr_sz + chgprt_sz;
  sz = (sz + VMS_BLOCK_SIZE - 1) & ~(VMS_BLOCK_SIZE - 1);
  content = bfd_zalloc (info->output_bfd, sz);
  if (content == NULL)
    return false;

  sec = alpha_vms_link_hash (info)->fixup;
  sec->contents = content;
  sec->size = sz;

  eiaf = (struct vms_eiaf *)content;
  off = sizeof (struct vms_eiaf);
  bfd_putl32 (0, eiaf->majorid);
  bfd_putl32 (0, eiaf->minorid);
  bfd_putl32 (0, eiaf->iaflink);
  bfd_putl32 (0, eiaf->fixuplnk);
  bfd_putl32 (sizeof (struct vms_eiaf), eiaf->size);
  bfd_putl32 (0, eiaf->flags);
  bfd_putl32 (0, eiaf->qrelfixoff);
  bfd_putl32 (0, eiaf->lrelfixoff);
  bfd_putl32 (0, eiaf->qdotadroff);
  bfd_putl32 (0, eiaf->ldotadroff);
  bfd_putl32 (0, eiaf->codeadroff);
  bfd_putl32 (0, eiaf->lpfixoff);
  bfd_putl32 (0, eiaf->chgprtoff);
  bfd_putl32 (shrimg_cnt ? off : 0, eiaf->shlstoff);
  bfd_putl32 (shrimg_cnt, eiaf->shrimgcnt);
  bfd_putl32 (0, eiaf->shlextra);
  bfd_putl32 (0, eiaf->permctx);
  bfd_putl32 (0, eiaf->base_va);
  bfd_putl32 (0, eiaf->lppsbfixoff);

  if (shrimg_cnt)
    {
      shrimg_cnt = 0;

      /* Write shl.  */
      for (i = 0; i < VEC_COUNT (t->shrlibs); i++)
	{
	  struct alpha_vms_shlib_el *shlib;
	  struct vms_shl *shl;

	  shlib = &VEC_EL (t->shrlibs, struct alpha_vms_shlib_el, i);

	  if (!shlib->has_fixups)
	    continue;

	  /* Renumber shared images.  */
	  PRIV2 (shlib->abfd, shr_index) = shrimg_cnt++;

	  shl = (struct vms_shl *)(content + off);
	  bfd_putl32 (0, shl->baseva);
	  bfd_putl32 (0, shl->shlptr);
	  bfd_putl32 (0, shl->ident);
	  bfd_putl32 (0, shl->permctx);
	  shl->size = sizeof (struct vms_shl);
	  bfd_putl16 (0, shl->fill_1);
	  shl->flags = 0;
	  bfd_putl32 (0, shl->icb);
	  shl->imgnam[0] = strlen (PRIV2 (shlib->abfd, hdr_data.hdr_t_name));
	  memcpy (shl->imgnam + 1, PRIV2 (shlib->abfd, hdr_data.hdr_t_name),
		  shl->imgnam[0]);

	  off += sizeof (struct vms_shl);
	}

      /* CA fixups.  */
      if (ca_sz != 0)
	{
	  bfd_putl32 (off, eiaf->codeadroff);

	  for (i = 0; i < VEC_COUNT (t->shrlibs); i++)
	    {
	      struct alpha_vms_shlib_el *shlib;
	      unsigned int j;

	      shlib = &VEC_EL (t->shrlibs, struct alpha_vms_shlib_el, i);

	      if (VEC_COUNT (shlib->ca) == 0)
		continue;

	      bfd_putl32 (VEC_COUNT (shlib->ca), content + off);
	      bfd_putl32 (PRIV2 (shlib->abfd, shr_index), content + off + 4);
	      off += 8;

	      for (j = 0; j < VEC_COUNT (shlib->ca); j++)
		{
		  bfd_putl32 (VEC_EL (shlib->ca, bfd_vma, j) - t->base_addr,
			      content + off);
		  off += 4;
		}
	    }

	  bfd_putl32 (0, content + off);
	  bfd_putl32 (0, content + off + 4);
	  off += 8;
	}

      /* LP fixups.  */
      if (lp_sz != 0)
	{
	  bfd_putl32 (off, eiaf->lpfixoff);

	  for (i = 0; i < VEC_COUNT (t->shrlibs); i++)
	    {
	      struct alpha_vms_shlib_el *shlib;
	      unsigned int j;

	      shlib = &VEC_EL (t->shrlibs, struct alpha_vms_shlib_el, i);

	      if (VEC_COUNT (shlib->lp) == 0)
		continue;

	      bfd_putl32 (VEC_COUNT (shlib->lp), content + off);
	      bfd_putl32 (PRIV2 (shlib->abfd, shr_index), content + off + 4);
	      off += 8;

	      for (j = 0; j < VEC_COUNT (shlib->lp); j++)
		{
		  bfd_putl32 (VEC_EL (shlib->lp, bfd_vma, j) - t->base_addr,
			      content + off);
		  off += 4;
		}
	    }

	  bfd_putl32 (0, content + off);
	  bfd_putl32 (0, content + off + 4);
	  off += 8;
	}

      /* QR fixups.  */
      if (qr_sz != 0)
	{
	  bfd_putl32 (off, eiaf->qdotadroff);

	  for (i = 0; i < VEC_COUNT (t->shrlibs); i++)
	    {
	      struct alpha_vms_shlib_el *shlib;
	      unsigned int j;

	      shlib = &VEC_EL (t->shrlibs, struct alpha_vms_shlib_el, i);

	      if (VEC_COUNT (shlib->qr) == 0)
		continue;

	      bfd_putl32 (VEC_COUNT (shlib->qr), content + off);
	      bfd_putl32 (PRIV2 (shlib->abfd, shr_index), content + off + 4);
	      off += 8;

	      for (j = 0; j < VEC_COUNT (shlib->qr); j++)
		{
		  struct alpha_vms_vma_ref *r;
		  r = &VEC_EL (shlib->qr, struct alpha_vms_vma_ref, j);
		  bfd_putl32 (r->vma - t->base_addr, content + off);
		  bfd_putl32 (r->ref, content + off + 4);
		  off += 8;
		}
	    }

	  bfd_putl32 (0, content + off);
	  bfd_putl32 (0, content + off + 4);
	  off += 8;
	}
    }

  /* Write the change protection table.  */
  bfd_putl32 (off, eiaf->chgprtoff);
  bfd_putl32 (chgprt_num, content + off);
  off += 4;

  for (sec = info->output_bfd->sections; sec != NULL; sec = sec->next)
    {
      struct vms_eicp *eicp;
      unsigned int prot;

      if ((sec->flags & SEC_LINKER_CREATED) != 0 &&
	  strcmp (sec->name, "$FIXUP$") == 0)
	prot = PRT__C_UREW;
      else if ((sec->flags & SEC_RELOC) != 0
	       && (sec->flags & (SEC_CODE | SEC_READONLY)) != 0)
	prot = PRT__C_UR;
      else
	continue;

      eicp = (struct vms_eicp *)(content + off);
      bfd_putl64 (sec->vma - t->base_addr, eicp->baseva);
      bfd_putl32 ((sec->size + VMS_BLOCK_SIZE - 1) & ~(VMS_BLOCK_SIZE - 1),
		  eicp->size);
      bfd_putl32 (prot, eicp->newprt);
      off += sizeof (struct vms_eicp);
    }

  return true;
}

/* Called by bfd_hash_traverse to fill the symbol table.
   Return FALSE in case of failure.  */

static bool
alpha_vms_link_output_symbol (struct bfd_hash_entry *bh, void *infov)
{
  struct bfd_link_hash_entry *hc = (struct bfd_link_hash_entry *) bh;
  struct bfd_link_info *info = (struct bfd_link_info *)infov;
  struct alpha_vms_link_hash_entry *h;
  struct vms_symbol_entry *sym;

  if (hc->type == bfd_link_hash_warning)
    {
      hc = hc->u.i.link;
      if (hc->type == bfd_link_hash_new)
	return true;
    }
  h = (struct alpha_vms_link_hash_entry *) hc;

  switch (h->root.type)
    {
    case bfd_link_hash_undefined:
      return true;
    case bfd_link_hash_new:
    case bfd_link_hash_warning:
      abort ();
    case bfd_link_hash_undefweak:
      return true;
    case bfd_link_hash_defined:
    case bfd_link_hash_defweak:
      {
	asection *sec = h->root.u.def.section;

	/* FIXME: this is certainly a symbol from a dynamic library.  */
	if (bfd_is_abs_section (sec))
	  return true;

	if (sec->owner->flags & DYNAMIC)
	  return true;
      }
      break;
    case bfd_link_hash_common:
      break;
    case bfd_link_hash_indirect:
      return true;
    }

  /* Do not write not kept symbols.  */
  if (info->strip == strip_some
      && bfd_hash_lookup (info->keep_hash, h->root.root.string,
			  false, false) != NULL)
    return true;

  if (h->sym == NULL)
    {
      /* This symbol doesn't come from a VMS object.  So we suppose it is
	 a data.  */
      int len = strlen (h->root.root.string);

      sym = (struct vms_symbol_entry *)bfd_zalloc (info->output_bfd,
						   sizeof (*sym) + len);
      if (sym == NULL)
	abort ();
      sym->namelen = len;
      memcpy (sym->name, h->root.root.string, len);
      sym->name[len] = 0;
      sym->owner = info->output_bfd;

      sym->typ = EGSD__C_SYMG;
      sym->data_type = 0;
      sym->flags = EGSY__V_DEF | EGSY__V_REL;
      sym->symbol_vector = h->root.u.def.value;
      sym->section = h->root.u.def.section;
      sym->value = h->root.u.def.value;
    }
  else
    sym = h->sym;

  if (!add_symbol_entry (info->output_bfd, sym))
    return false;

  return true;
}

static bool
alpha_vms_bfd_final_link (bfd *abfd, struct bfd_link_info *info)
{
  asection *o;
  struct bfd_link_order *p;
  bfd *sub;
  asection *fixupsec;
  bfd_vma base_addr;
  bfd_vma last_addr;
  asection *dst;
  asection *dmt;

  if (bfd_link_relocatable (info))
    {
      /* FIXME: we do not yet support relocatable link.  It is not obvious
	 how to do it for debug infos.  */
      (*info->callbacks->einfo)(_("%P: relocatable link is not supported\n"));
      return false;
    }

  abfd->outsymbols = NULL;
  abfd->symcount = 0;

  /* Mark all sections which will be included in the output file.  */
  for (o = abfd->sections; o != NULL; o = o->next)
    for (p = o->map_head.link_order; p != NULL; p = p->next)
      if (p->type == bfd_indirect_link_order)
	p->u.indirect.section->linker_mark = true;

#if 0
  /* Handle all the link order information for the sections.  */
  for (o = abfd->sections; o != NULL; o = o->next)
    {
      printf ("For section %s (at 0x%08x, flags=0x%08x):\n",
	      o->name, (unsigned)o->vma, (unsigned)o->flags);

      for (p = o->map_head.link_order; p != NULL; p = p->next)
	{
	  printf (" at 0x%08x - 0x%08x: ",
		  (unsigned)p->offset, (unsigned)(p->offset + p->size - 1));
	  switch (p->type)
	    {
	    case bfd_section_reloc_link_order:
	    case bfd_symbol_reloc_link_order:
	      printf ("  section/symbol reloc\n");
	      break;
	    case bfd_indirect_link_order:
	      printf ("  section %s of %s\n",
		      p->u.indirect.section->name,
		      p->u.indirect.section->owner->filename);
	      break;
	    case bfd_data_link_order:
	      printf ("  explicit data\n");
	      break;
	    default:
	      printf ("  *unknown* type %u\n", p->type);
	      break;
	    }
	}
    }
#endif

  /* Generate the symbol table.  */
  BFD_ASSERT (PRIV (syms) == NULL);
  if (info->strip != strip_all)
    bfd_hash_traverse (&info->hash->table, alpha_vms_link_output_symbol, info);

  /* Find the entry point.  */
  if (bfd_get_start_address (abfd) == 0)
    {
      bfd *startbfd = NULL;

      for (sub = info->input_bfds; sub != NULL; sub = sub->link.next)
	{
	  /* Consider only VMS object files.  */
	  if (sub->xvec != abfd->xvec)
	    continue;

	  if (!PRIV2 (sub, eom_data).eom_has_transfer)
	    continue;
	  if ((PRIV2 (sub, eom_data).eom_b_tfrflg & EEOM__M_WKTFR) && startbfd)
	    continue;
	  if (startbfd != NULL
	      && !(PRIV2 (sub, eom_data).eom_b_tfrflg & EEOM__M_WKTFR))
	    {
	      (*info->callbacks->einfo)
		/* xgettext:c-format */
		(_("%P: multiple entry points: in modules %pB and %pB\n"),
		 startbfd, sub);
	      continue;
	    }
	  startbfd = sub;
	}

      if (startbfd)
	{
	  unsigned int ps_idx = PRIV2 (startbfd, eom_data).eom_l_psindx;
	  bfd_vma tfradr = PRIV2 (startbfd, eom_data).eom_l_tfradr;
	  asection *sec;

	  sec = PRIV2 (startbfd, sections)[ps_idx];

	  bfd_set_start_address
	    (abfd, sec->output_section->vma + sec->output_offset + tfradr);
	}
    }

  /* Set transfer addresses.  */
  {
    int i;
    struct bfd_link_hash_entry *h;

    i = 0;
    PRIV (transfer_address[i++]) = 0xffffffff00000340ULL;	/* SYS$IMGACT */
    h = bfd_link_hash_lookup (info->hash, "LIB$INITIALIZE", false, false, true);
    if (h != NULL && h->type == bfd_link_hash_defined)
      PRIV (transfer_address[i++]) =
	alpha_vms_get_sym_value (h->u.def.section, h->u.def.value);
    PRIV (transfer_address[i++]) = bfd_get_start_address (abfd);
    while (i < 4)
      PRIV (transfer_address[i++]) = 0;
  }

  /* Allocate contents.
     Also compute the virtual base address.  */
  base_addr = (bfd_vma)-1;
  last_addr = 0;
  for (o = abfd->sections; o != NULL; o = o->next)
    {
      if (o->flags & SEC_HAS_CONTENTS)
	{
	  o->contents = bfd_alloc (abfd, o->size);
	  if (o->contents == NULL)
	    return false;
	}
      if (o->flags & SEC_LOAD)
	{
	  if (o->vma < base_addr)
	    base_addr = o->vma;
	  if (o->vma + o->size > last_addr)
	    last_addr = o->vma + o->size;
	}
      /* Clear the RELOC flags.  Currently we don't support incremental
	 linking.  We use the RELOC flag for computing the eicp entries.  */
      o->flags &= ~SEC_RELOC;
    }

  /* Create the fixup section.  */
  fixupsec = bfd_make_section_anyway_with_flags
    (info->output_bfd, "$FIXUP$",
     SEC_ALLOC | SEC_LOAD | SEC_HAS_CONTENTS | SEC_LINKER_CREATED);
  if (fixupsec == NULL)
    return false;
  last_addr = (last_addr + 0xffff) & ~0xffff;
  fixupsec->vma = last_addr;

  alpha_vms_link_hash (info)->fixup = fixupsec;
  alpha_vms_link_hash (info)->base_addr = base_addr;

  /* Create the DMT section, if necessary.  */
  BFD_ASSERT (PRIV (dst_section) == NULL);
  dst = bfd_get_section_by_name (abfd, "$DST$");
  if (dst != NULL && dst->size == 0)
    dst = NULL;
  if (dst != NULL)
    {
      PRIV (dst_section) = dst;
      dmt = bfd_make_section_anyway_with_flags
	(info->output_bfd, "$DMT$",
	 SEC_DEBUGGING | SEC_HAS_CONTENTS | SEC_LINKER_CREATED);
      if (dmt == NULL)
	return false;
    }
  else
    dmt = NULL;

  /* Read all sections from the inputs.  */
  for (sub = info->input_bfds; sub != NULL; sub = sub->link.next)
    {
      if (sub->flags & DYNAMIC)
	{
	  alpha_vms_create_eisd_for_shared (abfd, sub);
	  continue;
	}

      if (!alpha_vms_read_sections_content (sub, info))
	return false;
    }

  /* Handle all the link order information for the sections.
     Note: past this point, it is not possible to create new sections.  */
  for (o = abfd->sections; o != NULL; o = o->next)
    {
      for (p = o->map_head.link_order; p != NULL; p = p->next)
	{
	  switch (p->type)
	    {
	    case bfd_section_reloc_link_order:
	    case bfd_symbol_reloc_link_order:
	      abort ();
	      return false;
	    case bfd_indirect_link_order:
	      /* Already done.  */
	      break;
	    default:
	      if (! _bfd_default_link_order (abfd, info, o, p))
		return false;
	      break;
	    }
	}
    }

  /* Compute fixups.  */
  if (!alpha_vms_build_fixups (info))
    return false;

  /* Compute the DMT.  */
  if (dmt != NULL)
    {
      int pass;
      unsigned char *contents = NULL;

      /* In pass 1, compute the size.  In pass 2, write the DMT contents.  */
      for (pass = 0; pass < 2; pass++)
	{
	  unsigned int off = 0;

	  /* For each object file (ie for each module).  */
	  for (sub = info->input_bfds; sub != NULL; sub = sub->link.next)
	    {
	      asection *sub_dst;
	      struct vms_dmt_header *dmth = NULL;
	      unsigned int psect_count;

	      /* Skip this module if it has no DST.  */
	      sub_dst = PRIV2 (sub, dst_section);
	      if (sub_dst == NULL || sub_dst->size == 0)
		continue;

	      if (pass == 1)
		{
		  /* Write the header.  */
		  dmth = (struct vms_dmt_header *)(contents + off);
		  bfd_putl32 (sub_dst->output_offset, dmth->modbeg);
		  bfd_putl32 (sub_dst->size, dmth->size);
		}

	      off += sizeof (struct vms_dmt_header);
	      psect_count = 0;

	      /* For each section (ie for each psect).  */
	      for (o = sub->sections; o != NULL; o = o->next)
		{
		  /* Only consider interesting sections.  */
		  if (!(o->flags & SEC_ALLOC))
		    continue;
		  if (o->flags & SEC_LINKER_CREATED)
		    continue;

		  if (pass == 1)
		    {
		      /* Write an entry.  */
		      struct vms_dmt_psect *dmtp;

		      dmtp = (struct vms_dmt_psect *)(contents + off);
		      bfd_putl32 (o->output_offset + o->output_section->vma,
				  dmtp->start);
		      bfd_putl32 (o->size, dmtp->length);
		      psect_count++;
		    }
		  off += sizeof (struct vms_dmt_psect);
		}
	      if (pass == 1)
		bfd_putl32 (psect_count, dmth->psect_count);
	    }

	  if (pass == 0)
	    {
	      contents = bfd_zalloc (info->output_bfd, off);
	      if (contents == NULL)
		return false;
	      dmt->contents = contents;
	      dmt->size = off;
	    }
	  else
	    {
	      BFD_ASSERT (off == dmt->size);
	    }
	}
    }

  return true;
}

/* Read the contents of a section.
   buf points to a buffer of buf_size bytes to be filled with
   section data (starting at offset into section)  */

static bool
alpha_vms_get_section_contents (bfd *abfd, asection *section,
				void *buf, file_ptr offset,
				bfd_size_type count)
{
  asection *sec;

  /* Image are easy.  */
  if (bfd_get_file_flags (abfd) & (EXEC_P | DYNAMIC))
    return _bfd_generic_get_section_contents (abfd, section,
					      buf, offset, count);

  /* Safety check.  */
  if (offset + count < count
      || offset + count > section->size)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return false;
    }

  /* If the section is already in memory, just copy it.  */
  if (section->flags & SEC_IN_MEMORY)
    {
      BFD_ASSERT (section->contents != NULL);
      memcpy (buf, section->contents + offset, count);
      return true;
    }
  if (section->size == 0)
    return true;

  /* Alloc in memory and read ETIRs.  */
  for (sec = abfd->sections; sec; sec = sec->next)
    {
      BFD_ASSERT (sec->contents == NULL);

      if (sec->size != 0 && (sec->flags & SEC_HAS_CONTENTS))
	{
	  sec->contents = bfd_alloc (abfd, sec->size);
	  if (sec->contents == NULL)
	    return false;
	}
    }
  if (!alpha_vms_read_sections_content (abfd, NULL))
    return false;
  for (sec = abfd->sections; sec; sec = sec->next)
    if (sec->contents)
      sec->flags |= SEC_IN_MEMORY;
  memcpy (buf, section->contents + offset, count);
  return true;
}


/* Set the format of a file being written.  */

static bool
alpha_vms_mkobject (bfd * abfd)
{
  const bfd_arch_info_type *arch;

  vms_debug2 ((1, "alpha_vms_mkobject (%p)\n", abfd));

  if (!vms_initialize (abfd))
    return false;

  PRIV (recwr.buf) = bfd_alloc (abfd, MAX_OUTREC_SIZE);
  if (PRIV (recwr.buf) == NULL)
    return false;

  arch = bfd_scan_arch ("alpha");

  if (arch == 0)
    {
      bfd_set_error (bfd_error_wrong_format);
      return false;
    }

  abfd->arch_info = arch;
  return true;
}


/* 4.1, generic.  */

/* Called when the BFD is being closed to do any necessary cleanup.  */

static bool
vms_close_and_cleanup (bfd * abfd)
{
  vms_debug2 ((1, "vms_close_and_cleanup (%p)\n", abfd));

  if (abfd == NULL || abfd->tdata.any == NULL)
    return true;

  if (abfd->format == bfd_object)
    {
      alpha_vms_free_private (abfd);

#ifdef VMS
      if (abfd->direction == write_direction)
	{
	  /* Last step on VMS is to convert the file to variable record length
	     format.  */
	  if (!bfd_cache_close (abfd))
	    return false;
	  if (!_bfd_vms_convert_to_var_unix_filename (abfd->filename))
	    return false;
	}
#endif
    }

  return _bfd_generic_close_and_cleanup (abfd);
}

/* Called when a new section is created.  */

static bool
vms_new_section_hook (bfd * abfd, asection *section)
{
  size_t amt;

  vms_debug2 ((1, "vms_new_section_hook (%p, [%u]%s)\n",
	       abfd, section->index, section->name));

  if (!bfd_set_section_alignment (section, 0))
    return false;

  vms_debug2 ((7, "%u: %s\n", section->index, section->name));

  amt = sizeof (struct vms_section_data_struct);
  section->used_by_bfd = bfd_zalloc (abfd, amt);
  if (section->used_by_bfd == NULL)
    return false;

  /* Create the section symbol.  */
  return _bfd_generic_new_section_hook (abfd, section);
}

/* Part 4.5, symbols.  */

/* Print symbol to file according to how. how is one of
   bfd_print_symbol_name	just print the name
   bfd_print_symbol_more	print more (???)
   bfd_print_symbol_all	print all we know, which is not much right now :-).  */

static void
vms_print_symbol (bfd * abfd,
		  void * file,
		  asymbol *symbol,
		  bfd_print_symbol_type how)
{
  vms_debug2 ((1, "vms_print_symbol (%p, %p, %p, %d)\n",
	       abfd, file, symbol, how));

  switch (how)
    {
      case bfd_print_symbol_name:
      case bfd_print_symbol_more:
	fprintf ((FILE *)file," %s", symbol->name);
      break;

      case bfd_print_symbol_all:
	{
	  const char *section_name = symbol->section->name;

	  bfd_print_symbol_vandf (abfd, file, symbol);

	  fprintf ((FILE *) file," %-8s %s", section_name, symbol->name);
	}
      break;
    }
}

/* Return information about symbol in ret.

   fill type, value and name
   type:
	A	absolute
	B	bss segment symbol
	C	common symbol
	D	data segment symbol
	f	filename
	t	a static function symbol
	T	text segment symbol
	U	undefined
	-	debug.  */

static void
vms_get_symbol_info (bfd * abfd ATTRIBUTE_UNUSED,
		     asymbol *symbol,
		     symbol_info *ret)
{
  asection *sec;

  vms_debug2 ((1, "vms_get_symbol_info (%p, %p, %p)\n", abfd, symbol, ret));

  sec = symbol->section;

  if (ret == NULL)
    return;

  if (sec == NULL)
    ret->type = 'U';
  else if (bfd_is_com_section (sec))
    ret->type = 'C';
  else if (bfd_is_abs_section (sec))
    ret->type = 'A';
  else if (bfd_is_und_section (sec))
    ret->type = 'U';
  else if (bfd_is_ind_section (sec))
    ret->type = 'I';
  else if ((symbol->flags & BSF_FUNCTION)
	   || (bfd_section_flags (sec) & SEC_CODE))
    ret->type = 'T';
  else if (bfd_section_flags (sec) & SEC_DATA)
    ret->type = 'D';
  else if (bfd_section_flags (sec) & SEC_ALLOC)
    ret->type = 'B';
  else
    ret->type = '?';

  if (ret->type != 'U')
    ret->value = symbol->value + symbol->section->vma;
  else
    ret->value = 0;
  ret->name = symbol->name;
}

/* Return TRUE if the given symbol sym in the BFD abfd is
   a compiler generated local label, else return FALSE.  */

static bool
vms_bfd_is_local_label_name (bfd * abfd ATTRIBUTE_UNUSED,
			     const char *name)
{
  return name[0] == '$';
}

/* Part 4.7, writing an object file.  */

/* Sets the contents of the section section in BFD abfd to the data starting
   in memory at LOCATION. The data is written to the output section starting
   at offset offset for count bytes.

   Normally TRUE is returned, else FALSE. Possible error returns are:
   o bfd_error_no_contents - The output section does not have the
	SEC_HAS_CONTENTS attribute, so nothing can be written to it.
   o and some more too  */

static bool
_bfd_vms_set_section_contents (bfd * abfd,
			       asection *section,
			       const void * location,
			       file_ptr offset,
			       bfd_size_type count)
{
  if (section->contents == NULL)
    {
      section->contents = bfd_alloc (abfd, section->size);
      if (section->contents == NULL)
	return false;

      memcpy (section->contents + offset, location, (size_t) count);
    }

  return true;
}

/* Set the architecture and machine type in BFD abfd to arch and mach.
   Find the correct pointer to a structure and insert it into the arch_info
   pointer.  */

static bool
alpha_vms_set_arch_mach (bfd *abfd,
			 enum bfd_architecture arch, unsigned long mach)
{
  if (arch != bfd_arch_alpha
      && arch != bfd_arch_unknown)
    return false;

  return bfd_default_set_arch_mach (abfd, arch, mach);
}

/* Set section VMS flags.  Clear NO_FLAGS and set FLAGS.  */

void
bfd_vms_set_section_flags (bfd *abfd ATTRIBUTE_UNUSED,
			   asection *sec, flagword no_flags, flagword flags)
{
  vms_section_data (sec)->no_flags = no_flags;
  vms_section_data (sec)->flags = flags;
}

struct vms_private_data_struct *
bfd_vms_get_data (bfd *abfd)
{
  return (struct vms_private_data_struct *)abfd->tdata.any;
}

#define vms_bfd_copy_private_bfd_data	  _bfd_generic_bfd_copy_private_bfd_data
#define vms_bfd_merge_private_bfd_data	  _bfd_generic_bfd_merge_private_bfd_data
#define vms_bfd_copy_private_section_data _bfd_generic_bfd_copy_private_section_data
#define vms_bfd_copy_private_symbol_data  _bfd_generic_bfd_copy_private_symbol_data
#define vms_bfd_copy_private_header_data  _bfd_generic_bfd_copy_private_header_data
#define vms_bfd_set_private_flags	  _bfd_generic_bfd_set_private_flags

/* Symbols table.  */
#define alpha_vms_make_empty_symbol	   _bfd_generic_make_empty_symbol
#define alpha_vms_bfd_is_target_special_symbol _bfd_bool_bfd_asymbol_false
#define alpha_vms_print_symbol		   vms_print_symbol
#define alpha_vms_get_symbol_info	   vms_get_symbol_info
#define alpha_vms_get_symbol_version_string \
  _bfd_nosymbols_get_symbol_version_string

#define alpha_vms_read_minisymbols	   _bfd_generic_read_minisymbols
#define alpha_vms_minisymbol_to_symbol	   _bfd_generic_minisymbol_to_symbol
#define alpha_vms_get_lineno		   _bfd_nosymbols_get_lineno
#define alpha_vms_find_inliner_info	   _bfd_nosymbols_find_inliner_info
#define alpha_vms_bfd_make_debug_symbol	   _bfd_nosymbols_bfd_make_debug_symbol
#define alpha_vms_find_nearest_line	   _bfd_vms_find_nearest_line
#define alpha_vms_find_nearest_line_with_alt \
   _bfd_nosymbols_find_nearest_line_with_alt
#define alpha_vms_find_line		   _bfd_nosymbols_find_line
#define alpha_vms_bfd_is_local_label_name  vms_bfd_is_local_label_name

/* Generic table.  */
#define alpha_vms_close_and_cleanup	   vms_close_and_cleanup
#define alpha_vms_bfd_free_cached_info	   _bfd_bool_bfd_true
#define alpha_vms_new_section_hook	   vms_new_section_hook
#define alpha_vms_set_section_contents	   _bfd_vms_set_section_contents
#define alpha_vms_get_section_contents_in_window _bfd_generic_get_section_contents_in_window

#define alpha_vms_bfd_get_relocated_section_contents \
  bfd_generic_get_relocated_section_contents

#define alpha_vms_bfd_relax_section bfd_generic_relax_section
#define alpha_vms_bfd_gc_sections bfd_generic_gc_sections
#define alpha_vms_bfd_lookup_section_flags bfd_generic_lookup_section_flags
#define alpha_vms_bfd_merge_sections bfd_generic_merge_sections
#define alpha_vms_bfd_is_group_section bfd_generic_is_group_section
#define alpha_vms_bfd_group_name bfd_generic_group_name
#define alpha_vms_bfd_discard_group bfd_generic_discard_group
#define alpha_vms_section_already_linked \
  _bfd_generic_section_already_linked

#define alpha_vms_bfd_define_common_symbol bfd_generic_define_common_symbol
#define alpha_vms_bfd_link_hide_symbol _bfd_generic_link_hide_symbol
#define alpha_vms_bfd_define_start_stop bfd_generic_define_start_stop
#define alpha_vms_bfd_link_just_syms _bfd_generic_link_just_syms
#define alpha_vms_bfd_copy_link_hash_symbol_type \
  _bfd_generic_copy_link_hash_symbol_type

#define alpha_vms_bfd_link_split_section  _bfd_generic_link_split_section

#define alpha_vms_get_dynamic_symtab_upper_bound \
  _bfd_nodynamic_get_dynamic_symtab_upper_bound
#define alpha_vms_canonicalize_dynamic_symtab \
  _bfd_nodynamic_canonicalize_dynamic_symtab
#define alpha_vms_get_dynamic_reloc_upper_bound \
  _bfd_nodynamic_get_dynamic_reloc_upper_bound
#define alpha_vms_canonicalize_dynamic_reloc \
  _bfd_nodynamic_canonicalize_dynamic_reloc
#define alpha_vms_bfd_link_check_relocs		     _bfd_generic_link_check_relocs

const bfd_target alpha_vms_vec =
{
  "vms-alpha",			/* Name.  */
  bfd_target_evax_flavour,
  BFD_ENDIAN_LITTLE,		/* Data byte order is little.  */
  BFD_ENDIAN_LITTLE,		/* Header byte order is little.  */

  (HAS_RELOC | EXEC_P | HAS_LINENO | HAS_DEBUG | HAS_SYMS | HAS_LOCALS
   | WP_TEXT | D_PAGED),	/* Object flags.  */
  (SEC_ALLOC | SEC_LOAD | SEC_RELOC
   | SEC_READONLY | SEC_CODE | SEC_DATA
   | SEC_HAS_CONTENTS | SEC_IN_MEMORY),		/* Sect flags.  */
  0,				/* symbol_leading_char.  */
  ' ',				/* ar_pad_char.  */
  15,				/* ar_max_namelen.  */
  0,				/* match priority.  */
  TARGET_KEEP_UNUSED_SECTION_SYMBOLS, /* keep unused section symbols.  */
  bfd_getl64, bfd_getl_signed_64, bfd_putl64,
  bfd_getl32, bfd_getl_signed_32, bfd_putl32,
  bfd_getl16, bfd_getl_signed_16, bfd_putl16,
  bfd_getl64, bfd_getl_signed_64, bfd_putl64,
  bfd_getl32, bfd_getl_signed_32, bfd_putl32,
  bfd_getl16, bfd_getl_signed_16, bfd_putl16,

  {				/* bfd_check_format.  */
    _bfd_dummy_target,
    alpha_vms_object_p,
    _bfd_vms_lib_alpha_archive_p,
    _bfd_dummy_target
  },
  {				/* bfd_set_format.  */
    _bfd_bool_bfd_false_error,
    alpha_vms_mkobject,
    _bfd_vms_lib_alpha_mkarchive,
    _bfd_bool_bfd_false_error
  },
  {				/* bfd_write_contents.  */
    _bfd_bool_bfd_false_error,
    alpha_vms_write_object_contents,
    _bfd_vms_lib_write_archive_contents,
    _bfd_bool_bfd_false_error
  },

  BFD_JUMP_TABLE_GENERIC (alpha_vms),
  BFD_JUMP_TABLE_COPY (vms),
  BFD_JUMP_TABLE_CORE (_bfd_nocore),
  BFD_JUMP_TABLE_ARCHIVE (_bfd_vms_lib),
  BFD_JUMP_TABLE_SYMBOLS (alpha_vms),
  BFD_JUMP_TABLE_RELOCS (alpha_vms),
  BFD_JUMP_TABLE_WRITE (alpha_vms),
  BFD_JUMP_TABLE_LINK (alpha_vms),
  BFD_JUMP_TABLE_DYNAMIC (alpha_vms),

  NULL,

  NULL
};
