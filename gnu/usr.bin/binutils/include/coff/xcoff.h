/* Internal format of XCOFF object file data structures for BFD.

   Copyright (C) 1995-2023 Free Software Foundation, Inc.
   Written by Ian Lance Taylor <ian@cygnus.com>, Cygnus Support.

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

#ifndef _INTERNAL_XCOFF_H
#define _INTERNAL_XCOFF_H

/* XCOFF specific f_flags.  */

/* File was profiled with fdpr.  */
#define F_FDPR_PROF 0x0010

/* File was reordered with fdpr.  */
#define F_FDPR_OPTI 0x0020

/* File use very large program support.  */
#define F_DSA       0x0040

/* One aux header specifying medium page sizes is non-zero.  */
#define F_VARPG     0x0100

/* Read/write sections are non-executable.  */
#define F_NONEXEC   0x8000

/* Linker */

/* Names of "special" sections.  */
#define _TEXT	".text"
#define _DATA	".data"
#define _BSS	".bss"
#define _TDATA	".tdata"
#define _TBSS	".tbss"
#define _PAD	".pad"
#define _LOADER	".loader"
#define _EXCEPT ".except"
#define _TYPCHK ".typchk"

/* XCOFF uses special .dwXXX sections with the type STYP_DWARF.  */
#define STYP_DWARF  0x0010

/* High-order 16-bits dwarf subtypes.  */
#define SSUBTYP_DWINFO  0x10000
#define SSUBTYP_DWLINE  0x20000
#define SSUBTYP_DWPBNMS 0x30000
#define SSUBTYP_DWPBTYP 0x40000
#define SSUBTYP_DWARNGE 0x50000
#define SSUBTYP_DWABREV 0x60000
#define SSUBTYP_DWSTR   0x70000
#define SSUBTYP_DWRNGES 0x80000
#define SSUBTYP_DWLOC   0x90000
#define SSUBTYP_DWFRAME 0xA0000
#define SSUBTYP_DWMAC   0xB0000

/* XCOFF uses a special .loader section with type STYP_LOADER.  */
#define STYP_LOADER 0x1000

/* Specifies an exception section.  A section of this type provides 
   information to identify the reason that a trap or ececptin occured within 
   and executable object program */
#define STYP_EXCEPT 0x0100

/* Specifies an initialized thread-local data section.  */
#define STYP_TDATA  0x0400

/* Specifies an uninitialized thread-local data section.  */
#define STYP_TBSS   0x0800

/* XCOFF uses a special .debug section with type STYP_DEBUG.  */
#define STYP_DEBUG  0x2000

/* Specifies a type check section.  A section of this type contains parameter 
   argument type check strings used by the AIX binder.  */
#define STYP_TYPCHK 0x4000

/* XCOFF handles line number or relocation overflow by creating
   another section header with STYP_OVRFLO set.  */
#define STYP_OVRFLO 0x8000

#define	RS6K_AOUTHDR_OMAGIC 0x0107 /* old: text & data writeable */
#define	RS6K_AOUTHDR_NMAGIC 0x0108 /* new: text r/o, data r/w */
#define	RS6K_AOUTHDR_ZMAGIC 0x010B /* paged: text r/o, both page-aligned */

/* Flags for aouthdr o_flags */
#define RS6K_AOUTHDR_TLS_LE    0x80  /* TLS local-exec code was generated */
#define RS6K_AOUTHDR_RAS       0x40  /* kernel module is key & recovery safe */
#define RS6K_AOUTHDR_ALGNTDATA 0xf   /* TLS alignment */

/* Flags for aouthdr o_x64flags */
#define RS6K_AOUTHDR_SHR_SYMTAB  0x8000
#define RS6K_AOUTHDR_FORK_POLICY 0x4000
#define RS6K_AOUTHDR_FORK_COR    0x2000


/* XCOFF relocation types.
   The relocations are described in the function
   xcoff[64]_ppc_relocate_section in coff64-rs6000.c and coff-rs6000.c  */

#define R_POS    (0x00)
#define R_NEG    (0x01)
#define R_REL    (0x02)
#define R_TOC    (0x03)
#define R_TRL    (0x04)
#define R_GL     (0x05)
#define R_TCL    (0x06)
#define R_BA     (0x08)
#define R_BR     (0x0a)
#define R_RL     (0x0c)
#define R_RLA    (0x0d)
#define R_REF    (0x0f)
#define R_TRLA   (0x13)
#define R_RRTBI  (0x14)
#define R_RRTBA  (0x15)
#define R_CAI    (0x16)
#define R_CREL   (0x17)
#define R_RBA    (0x18)
#define R_RBAC   (0x19)
#define R_RBR    (0x1a)
#define R_RBRC   (0x1b)
#define R_TLS    (0x20)
#define R_TLS_IE (0x21)
#define R_TLS_LD (0x22)
#define R_TLS_LE (0x23)
#define R_TLSM   (0x24)
#define R_TLSML  (0x25)
#define R_TOCU   (0x30)
#define R_TOCL   (0x31)

/* Storage class #defines, from /usr/include/storclass.h that are not already 
   defined in internal.h */

/* Comment string in .info section */
#define	C_INFO		110	

/* Dwarf symbol.  */
#define C_DWARF		112

/* Auxillary Symbol Entries  */

/* x_smtyp values:  */
#define	SMTYP_ALIGN(x)	((x) >> 3)	/* log2 of alignment */
#define	SMTYP_SMTYP(x)	((x) & 0x7)	/* symbol type */
/* Symbol type values:  */
#define	XTY_ER	0		/* External reference */
#define	XTY_SD	1		/* Csect definition */
#define	XTY_LD	2		/* Label definition */
#define XTY_CM	3		/* .BSS */
#define	XTY_EM	4		/* Error message */
#define	XTY_US	5		/* "Reserved for internal use" */

/* x_smclas values:  */
#define	XMC_PR	0		/* Read-only program code */
#define	XMC_RO	1		/* Read-only constant */
#define	XMC_DB	2		/* Read-only debug dictionary table */
#define	XMC_TC	3		/* Read-write general TOC entry */
#define	XMC_UA	4		/* Read-write unclassified */
#define	XMC_RW	5		/* Read-write data */
#define	XMC_GL	6		/* Read-only global linkage */
#define	XMC_XO	7		/* Read-only extended operation */
#define	XMC_SV	8		/* Read-only supervisor call */
#define	XMC_BS	9		/* Read-write BSS */
#define	XMC_DS	10		/* Read-write descriptor csect */
#define	XMC_UC	11		/* Read-write unnamed Fortran common */
#define	XMC_TI	12		/* Read-only traceback index csect */
#define	XMC_TB	13		/* Read-only traceback table csect */
/* 		14	??? */
#define	XMC_TC0	15		/* Read-write TOC anchor */
#define XMC_TD	16		/* Read-write data in TOC */
#define	XMC_SV64   17		/* Read-only 64 bit supervisor call */
#define	XMC_SV3264 18		/* Read-only 32 or 64 bit supervisor call */
/*                19   ??? */
#define XMC_TL     20          /* Read-write initialized TLS data */
#define XMC_UL     21          /* Read-write uninitialized TLS data */
#define XMC_TE     22          /* Same as XMC_TC but mapped after it */

/* x_ftype values:  */
#define XFT_FN 0    /* Specifies the source-file name */
#define XFT_CT 1    /* Specifies the compiler time stamp */
#define XFT_CV 2    /* Specifies the compiler version number */
#define XFT_CD 128  /*Specifies compiler-defined information */

/* The ldhdr structure.  This appears at the start of the .loader
   section.  */

struct internal_ldhdr
{
  /* The version number: 
     1 : 32 bit
     2 : 64 bit */
  unsigned long l_version;

  /* The number of symbol table entries.  */
  bfd_size_type l_nsyms;

  /* The number of relocation table entries.  */
  bfd_size_type l_nreloc;

  /* The length of the import file string table.  */
  bfd_size_type l_istlen;

  /* The number of import files.  */
  bfd_size_type l_nimpid;

  /* The offset from the start of the .loader section to the first
     entry in the import file table.  */
  bfd_size_type l_impoff;

  /* The length of the string table.  */
  bfd_size_type l_stlen;

  /* The offset from the start of the .loader section to the first
     entry in the string table.  */
  bfd_size_type l_stoff;

  /* The offset to start of the symbol table, only in XCOFF64 */
  bfd_vma l_symoff;

  /* The offset to the start of the relocation table, only in XCOFF64 */
  bfd_vma l_rldoff;
};

/* The ldsym structure.  This is used to represent a symbol in the
   .loader section.  */

struct internal_ldsym
{
  union
  {
    /* The symbol name if <= SYMNMLEN characters.  */
    char _l_name[SYMNMLEN];
    struct
    {
      /* Zero if the symbol name is more than SYMNMLEN characters.  */
	long _l_zeroes;
      
      /* The offset in the string table if the symbol name is more
	 than SYMNMLEN characters.  */
      long _l_offset;
    } 
    _l_l;
  }
  _l;

  /* The symbol value.  */
  bfd_vma l_value;

  /* The symbol section number.  */
  short l_scnum;

  /* The symbol type and flags.  */
  char l_smtype;

  /* The symbol storage class.  */
  char l_smclas;

  /* The import file ID.  */
  bfd_size_type l_ifile;

  /* Offset to the parameter type check string.  */
  bfd_size_type l_parm;
};

/* These flags are for the l_smtype field (the lower three bits are an
   XTY_* value).  */

/* Imported symbol.  */
#define L_IMPORT (0x40)
/* Entry point.  */
#define L_ENTRY (0x20)
/* Exported symbol.  */
#define L_EXPORT (0x10)
/* Weak symbol.  */
#define L_WEAK (0x08)

/* The ldrel structure.  This is used to represent a reloc in the
   .loader section.  */

struct internal_ldrel
{
  /* The reloc address.  */
  bfd_vma l_vaddr;

  /* The symbol table index in the .loader section symbol table.  */
  bfd_size_type l_symndx;

  /* The relocation type and size.  */
  short l_rtype;

  /* The section number this relocation applies to.  */
  short l_rsecnm;
};

/* An entry in the XCOFF linker hash table.  */
struct xcoff_link_hash_entry
{
  struct bfd_link_hash_entry root;

  /* Symbol index in output file.  Set to -1 initially.  Set to -2 if
     there is a reloc against this symbol.  */
  long indx;

  /* If we have created a TOC entry for this symbol, this is the .tc
     section which holds it.  */
  asection *toc_section;

  union
  {
    /* If we have created a TOC entry (the XCOFF_SET_TOC flag is
       set), this is the offset in toc_section.  */
    bfd_vma toc_offset;
    
    /* If the TOC entry comes from an input file, this is set to the
       symbol index of the C_HIDEXT XMC_TC or XMC_TD symbol.  */
    long toc_indx;
  } 
  u;

  /* If this symbol is a function entry point which is called, this
     field holds a pointer to the function descriptor.  If this symbol
     is a function descriptor, this field holds a pointer to the
     function entry point.  */
  struct xcoff_link_hash_entry *descriptor;

  /* The .loader symbol table entry, if there is one.  */
  struct internal_ldsym *ldsym;

  /* If XCOFF_BUILT_LDSYM is set, this is the .loader symbol table
     index.  If XCOFF_BUILD_LDSYM is clear, and XCOFF_IMPORT is set,
     this is the l_ifile value.  */
  long ldindx;

  /* Some linker flags.  */
  unsigned long flags;

  /* Symbol visibility, using the same define than n_type.  */
  unsigned short visibility;

  /* The storage mapping class.  */
  unsigned char smclas;
};

/*  Flags for xcoff_link_hash_entry.  */

/* Symbol is referenced by a regular object. */
#define XCOFF_REF_REGULAR      0x00000001
/* Symbol is defined by a regular object. */
#define XCOFF_DEF_REGULAR      0x00000002
/* Symbol is defined by a dynamic object. */
#define XCOFF_DEF_DYNAMIC      0x00000004
/* Symbol is used in a reloc being copied into the .loader section.  */
#define XCOFF_LDREL            0x00000008
/* Symbol is the entry point.  */
#define XCOFF_ENTRY            0x00000010
/* Symbol is for a function and is the target of a relocation.
   The relocation may or may not be a branch-type relocation.  */
#define XCOFF_CALLED           0x00000020
/* Symbol needs the TOC entry filled in.  */
#define XCOFF_SET_TOC          0x00000040
/* Symbol is implicitly or explicitly imported.  */
#define XCOFF_IMPORT           0x00000080
/* Symbol is explicitly exported.  */
#define XCOFF_EXPORT           0x00000100
/* Symbol has been processed by xcoff_build_ldsyms.  */
#define XCOFF_BUILT_LDSYM      0x00000200
/* Symbol is mentioned by a section which was not garbage collected. */
#define XCOFF_MARK             0x00000400
/* Symbol size is recorded in size_list list from hash table.  */
#define XCOFF_HAS_SIZE         0x00000800
/* Symbol is a function descriptor.  */
#define XCOFF_DESCRIPTOR       0x00001000
/* Multiple definitions have been for the symbol. */
#define XCOFF_MULTIPLY_DEFINED 0x00002000
/* Symbol is the __rtinit symbol.  */
#define XCOFF_RTINIT           0x00004000
/* Symbol is an imported 32 bit syscall.  */
#define XCOFF_SYSCALL32        0x00008000
/* Symbol is an imported 64 bit syscall.  */
#define XCOFF_SYSCALL64        0x00010000 
/* Symbol was not explicitly defined by the time it was marked.  */
#define XCOFF_WAS_UNDEFINED    0x00020000
/* We have assigned an output XCOFF entry to this symbol.  */
#define XCOFF_ALLOCATED	       0x00040000

/* The XCOFF linker hash table.  */

#define XCOFF_NUMBER_OF_SPECIAL_SECTIONS 6
#define XCOFF_SPECIAL_SECTION_TEXT       0
#define XCOFF_SPECIAL_SECTION_ETEXT      1 
#define XCOFF_SPECIAL_SECTION_DATA       2
#define XCOFF_SPECIAL_SECTION_EDATA      3
#define XCOFF_SPECIAL_SECTION_END        4
#define XCOFF_SPECIAL_SECTION_END2       5

/* These flags indicate which of -bexpall and -bexpfull are in effect.  */
#define XCOFF_EXPALL 1
#define XCOFF_EXPFULL 2

/* This structure is used to pass information through
   xcoff_link_hash_traverse.  */

struct xcoff_loader_info
{
  /* Set if a problem occurred.  */
  bool failed;

  /* Output BFD.  */
  bfd *output_bfd;

  /* Link information structure.  */
  struct bfd_link_info *info;

  /* A mask of XCOFF_EXPALL and XCOFF_EXPFULL flags.  */
  unsigned int auto_export_flags;

  /* Number of ldsym structures.  */
  size_t ldsym_count;

  /* A count of non TOC relative relocs which will need to be
     allocated in the .loader section.  */
  size_t ldrel_count;

  /* Size of string table.  */
  size_t string_size;

  /* String table.  */
  char *strings;

  /* Allocated size of string table.  */
  size_t string_alc;

  /* The libpath being used.  */
  const char *libpath;
};

/* In case we're on a 32-bit machine, construct a 64-bit "-1" value
   from smaller values.  Start with zero, widen, *then* decrement.  */
#define MINUS_ONE       (((bfd_vma) 0) - 1)

/* __rtinit, from /usr/include/rtinit.h.  */
struct __rtinit 
{
  /* Pointer to runtime linker.     
     XXX: Is the parameter really void?  */
  int	(*rtl) (void);	

  /* Offset to array of init functions, 0 if none. */
  int	init_offset;

  /* Offset to array of fini functions, 0 if none. */		   
  int	fini_offset;		

  /* Size of __RTINIT_DESCRIPTOR. This value should be used instead of 
     sizeof(__RTINIT_DESCRIPTOR). */
  int	__rtinit_descriptor_size; 
};

#define RTINIT_DESCRIPTOR_SIZE (12)

struct __rtinit_descriptor 
{
  /* Init/fini function. */
  int	f;

  /* Offset, relative to the start of the __rtinit symbol, to name of the 
     function. */

  int	name_offset;	

  /* Flags */			   
  unsigned char	flags;	
};

/* Archive */

#define XCOFFARMAG    "<aiaff>\012"
#define XCOFFARMAGBIG "<bigaf>\012"
#define SXCOFFARMAG   8

/* The size of the ascii archive elements */
#define XCOFFARMAG_ELEMENT_SIZE 12
#define XCOFFARMAGBIG_ELEMENT_SIZE 20

/* This terminates an XCOFF archive member name.  */

#define XCOFFARFMAG "`\012"
#define SXCOFFARFMAG 2

/* XCOFF archives start with this (printable) structure.  */

struct xcoff_ar_file_hdr
{
  /* Magic string.  */
  char magic[SXCOFFARMAG];

  /* Offset of the member table (decimal ASCII string).  */
  char memoff[XCOFFARMAG_ELEMENT_SIZE];

  /* Offset of the global symbol table (decimal ASCII string).  */
  char symoff[XCOFFARMAG_ELEMENT_SIZE];

  /* Offset of the first member in the archive (decimal ASCII string).  */
  char firstmemoff[XCOFFARMAG_ELEMENT_SIZE];

  /* Offset of the last member in the archive (decimal ASCII string).  */
  char lastmemoff[XCOFFARMAG_ELEMENT_SIZE];

  /* Offset of the first member on the free list (decimal ASCII
     string).  */
  char freeoff[XCOFFARMAG_ELEMENT_SIZE];
};

#define SIZEOF_AR_FILE_HDR (sizeof (struct xcoff_ar_file_hdr))

/* This is the equivalent data structure for the big archive format.  */

struct xcoff_ar_file_hdr_big
{
  /* Magic string.  */
  char magic[SXCOFFARMAG];

  /* Offset of the member table (decimal ASCII string).  */
  char memoff[XCOFFARMAGBIG_ELEMENT_SIZE];

  /* Offset of the global symbol table for 32-bit objects (decimal ASCII
     string).  */
  char symoff[XCOFFARMAGBIG_ELEMENT_SIZE];

  /* Offset of the global symbol table for 64-bit objects (decimal ASCII
     string).  */
  char symoff64[XCOFFARMAGBIG_ELEMENT_SIZE];

  /* Offset of the first member in the archive (decimal ASCII string).  */
  char firstmemoff[XCOFFARMAGBIG_ELEMENT_SIZE];

  /* Offset of the last member in the archive (decimal ASCII string).  */
  char lastmemoff[XCOFFARMAGBIG_ELEMENT_SIZE];

  /* Offset of the first member on the free list (decimal ASCII
     string).  */
  char freeoff[XCOFFARMAGBIG_ELEMENT_SIZE];
};

#define SIZEOF_AR_FILE_HDR_BIG (sizeof (struct xcoff_ar_file_hdr_big))

/* Each XCOFF archive member starts with this (printable) structure.  */

struct xcoff_ar_hdr
{
  /* File size not including the header (decimal ASCII string).  */
  char size[XCOFFARMAG_ELEMENT_SIZE];

  /* File offset of next archive member (decimal ASCII string).  */
  char nextoff[XCOFFARMAG_ELEMENT_SIZE];

  /* File offset of previous archive member (decimal ASCII string).  */
  char prevoff[XCOFFARMAG_ELEMENT_SIZE];

  /* File mtime (decimal ASCII string).  */
  char date[12];

  /* File UID (decimal ASCII string).  */
  char uid[12];

  /* File GID (decimal ASCII string).  */
  char gid[12];

  /* File mode (octal ASCII string).  */
  char mode[12];

  /* Length of file name (decimal ASCII string).  */
  char namlen[4];

  /* This structure is followed by the file name.  The length of the
     name is given in the namlen field.  If the length of the name is
     odd, the name is followed by a null byte.  The name and optional
     null byte are followed by XCOFFARFMAG, which is not included in
     namlen.  The contents of the archive member follow; the number of
     bytes is given in the size field.  */
};

#define SIZEOF_AR_HDR (sizeof (struct xcoff_ar_hdr))

/* The equivalent for the big archive format.  */

struct xcoff_ar_hdr_big
{
  /* File size not including the header (decimal ASCII string).  */
  char size[XCOFFARMAGBIG_ELEMENT_SIZE];

  /* File offset of next archive member (decimal ASCII string).  */
  char nextoff[XCOFFARMAGBIG_ELEMENT_SIZE];

  /* File offset of previous archive member (decimal ASCII string).  */
  char prevoff[XCOFFARMAGBIG_ELEMENT_SIZE];

  /* File mtime (decimal ASCII string).  */
  char date[12];

  /* File UID (decimal ASCII string).  */
  char uid[12];

  /* File GID (decimal ASCII string).  */
  char gid[12];

  /* File mode (octal ASCII string).  */
  char mode[12];

  /* Length of file name (decimal ASCII string).  */
  char namlen[4];

  /* This structure is followed by the file name.  The length of the
     name is given in the namlen field.  If the length of the name is
     odd, the name is followed by a null byte.  The name and optional
     null byte are followed by XCOFFARFMAG, which is not included in
     namlen.  The contents of the archive member follow; the number of
     bytes is given in the size field.  */
};

#define SIZEOF_AR_HDR_BIG (sizeof (struct xcoff_ar_hdr_big))

/* Track archive file offsets used by elements and the header.  */
struct ar_ranges
{
  ufile_ptr start, end;
  struct ar_ranges *next;
};

/* An archive bfd has tdata pointing to a struct artdata.  The xcoff
   backend has artdata.tdata pointing to the following.  */
struct xcoff_artdata
{
  union
  {
    struct xcoff_ar_file_hdr hdr;
    struct xcoff_ar_file_hdr_big bhdr;
  } u;
  struct ar_ranges ranges;
  /* Anything less than this size can't hold an archive element.  */
  unsigned int ar_hdr_size;
};

#define x_artdata(abfd) ((struct xcoff_artdata *) bfd_ardata (abfd)->tdata)

/* We often have to distinguish between the old and big file format.
   u.hdr.magic and u.bhdr.magic have the same size and position.  */
#ifndef SMALL_ARCHIVE
/* Creates big archives by default */
#define xcoff_big_format_p(abfd) \
  (bfd_ardata (abfd) == NULL			\
   || x_artdata (abfd) == NULL			\
   || x_artdata (abfd)->u.hdr.magic[1] != 'a')
#else
/* Creates small archives by default. */
#define xcoff_big_format_p(abfd) \
  (bfd_ardata (abfd) != NULL			\
   && x_artdata (abfd) != NULL			\
   && x_artdata (abfd)->u.hdr.magic[1] == 'b')
#endif

/* We store a copy of the xcoff_ar_hdr in the arelt_data field of an
   archive element.  Similar for the big archive.  */
#define arch_eltdata(bfd) ((struct areltdata *) ((bfd)->arelt_data))
#define arch_xhdr(bfd) \
  ((struct xcoff_ar_hdr *) arch_eltdata (bfd)->arch_header)
#define arch_xhdr_big(bfd) \
  ((struct xcoff_ar_hdr_big *) arch_eltdata (bfd)->arch_header)

/* True if symbols of class CLASS are external.  */
#define EXTERN_SYM_P(CLASS) \
  ((CLASS) == C_EXT || (CLASS) == C_AIX_WEAKEXT)

#endif /* _INTERNAL_XCOFF_H */
