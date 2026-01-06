/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */

/*
 * Copyright 2008 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#include <memory.h>
#include "libelf.h"

#include <sys/link.h>

#include "decl.h"
#include "msg.h"
#include <string.h>

/*
 * fmsize:  Array used to determine what size the the structures
 *	    are (for memory image & file image).
 *
 * x32:  Translation routines - to file & to memory.
 *
 * What must be done when adding a new type for conversion:
 *
 * The first question is whether you need a new ELF_T_* type
 * to be created.  If you've introduced a new structure - then
 * it will need to be described - this is done by:
 *
 * o adding a new type ELF_T_* to usr/src/head/libelf.h
 * o Create a new macro to define the bytes contained in the structure. Take a
 *   look at the 'Syminfo_1' macro defined below.  The declarations describe
 *   the structure based off of the field size of each element of the structure.
 * o Add a entry to the fmsize table for the new ELF_T_* type.
 * o Create a <newtype>_11_tof macro.  Take a look at 'syminfo_11_tof'.
 * o Create a <newtype>_11_tom macro.  Take a look at 'syminfo_11_tom'.
 * o The <newtype>_11_tof & <newtype>_11_tom results in conversion routines
 *   <newtype>_2L11_tof, <newtype>_2L11_tom, <newtype>_2M11_tof,
 *   <newtype>_2M11_tom being created in xlate.c.  These routines
 *   need to be added to the 'x32[]' array.
 * o Add entries to getdata.c::align32[] and getdata.c::align64[].  These
 *   tables define what the alignment requirements for a data type are.
 *
 * In order to tie a section header type (SHT_*) to a data
 * structure you need to update elf32_mtype() so that it can
 * make the association.  If you are introducing a new section built
 * on a basic datatype (SHT_INIT_ARRAY) then this is all the updating
 * that needs to be done.
 *
 *
 * ELF translation routines
 *	These routines make a subtle implicit assumption.
 *	The file representations of all structures are "packed,"
 *	meaning no implicit padding bytes occur.  This might not
 *	be the case for the memory representations.  Consequently,
 *	the memory representations ALWAYS contain at least as many
 *	bytes as the file representations.  Otherwise, the memory
 *	structures would lose information, meaning they're not
 *	implemented properly.
 *
 *	The words above apply to structures with the same members.
 *	If a future version changes the number of members, the
 *	relative structure sizes for different version must be
 *	tested with the compiler.
 */

#define	HI32	0x80000000UL
#define	LO31	0x7fffffffUL

/*
 *	These macros create indexes for accessing the bytes of
 *	words and halfwords for ELFCLASS32 data representations
 *	(currently ELFDATA2LSB and ELFDATA2MSB).  In all cases,
 *
 *	w = (((((X_3 << 8) + X_2) << 8) + X_1) << 8) + X_0
 *	h = (X_1 << 8) + X_0
 *
 *	These assume the file representations for Addr, Off,
 *	Sword, and Word use 4 bytes, but the memory def's for
 *	the types may differ.
 *
 *	Naming convention:
 *		..._L	ELFDATA2LSB
 *		..._M	ELFDATA2MSB
 *
 *	enuma_*(n)	define enum names for addr n
 *	enumb_*(n)	define enum names for byte n
 *	enumh_*(n)	define enum names for half n
 *	enumo_*(n)	define enum names for off n
 *	enumw_*(n)	define enum names for word n
 *	enuml_*(n)	define enum names for Lword n
 *	tofa(d,s,n)	xlate addr n from mem s to file d
 *	tofb(d,s,n)	xlate byte n from mem s to file d
 *	tofh(d,s,n)	xlate half n from mem s to file d
 *	tofo(d,s,n)	xlate off n from mem s to file d
 *	tofw(d,s,n)	xlate word n from mem s to file d
 *	tofl(d,s,n)	xlate Lword n from mem s to file d
 *	toma(s,n)	xlate addr n from file s to expression value
 *	tomb(s,n)	xlate byte n from file s to expression value
 *	tomh(s,n)	xlate half n from file s to expression value
 *	tomo(s,n)	xlate off n from file s to expression value
 *	tomw(s,n)	xlate word n from file s to expression value
 *	toml(s,n)	xlate Lword n from file s to expression value
 *
 *	tof*() macros must move a multi-byte value into a temporary
 *	because `in place' conversions are allowed.  If a temp is not
 *	used for multi-byte objects, storing an initial destination byte
 *	may clobber a source byte not yet examined.
 *
 *	tom*() macros compute an expression value from the source
 *	without touching the destination; so they're safe.
 */

typedef struct {
	Elf32_Sword d_tag;		/* how to interpret value */
	union {
		Elf32_Word	d_val;
		Elf32_Addr	d_ptr;
		Elf32_Off	d_off;
	} d_un;
} Elf32_Dyn;

typedef struct {
	Elf32_Addr	r_offset;
	Elf32_Word	r_info;		/* sym, type: ELF32_R_... */
} Elf32_Rel;

typedef struct {
	Elf32_Addr	r_offset;
	Elf32_Word	r_info;		/* sym, type: ELF32_R_... */
	Elf32_Sword	r_addend;
} Elf32_Rela;

typedef struct {
	Elf32_Half	si_boundto;	/* direct bindings - symbol bound to */
	Elf32_Half	si_flags;	/* per symbol flags */
} Elf32_Syminfo;

/*
 * ELF data object indexes
 *	The enums are broken apart to get around deficiencies
 *	in some compilers.
 */



enum
{
	A_L0, A_L1, A_L2, A_L3
};

enum
{
	A_M3, A_M2, A_M1, A_M0,
	A_sizeof
};





enum
{
	H_L0, H_L1
};

enum
{
	H_M1, H_M0,
	H_sizeof
};




enum
{
	L_L0, L_L1, L_L2, L_L3, L_L4, L_L5, L_L6, L_L7
};

enum
{
	L_M7, L_M6, L_M5, L_M4, L_M3, L_M2, L_M1, L_M0,
	L_sizeof
};





enum
{
	M1_value_L0, M1_value_L1, M1_value_L2, M1_value_L3, M1_value_L4, M1_value_L5, M1_value_L6, M1_value_L7,
	M1_info_L0, M1_info_L1, M1_info_L2, M1_info_L3,
	M1_poffset_L0, M1_poffset_L1, M1_poffset_L2, M1_poffset_L3,
	M1_repeat_L0, M1_repeat_L1,
	M1_stride_L0, M1_stride_L1
};

enum
{
	M1_value_M7, M1_value_M6, M1_value_M5, M1_value_M4, M1_value_M3, M1_value_M2, M1_value_M1, M1_value_M0,
	M1_info_M3, M1_info_M2, M1_info_M1, M1_info_M0,
	M1_poffset_M3, M1_poffset_M2, M1_poffset_M1, M1_poffset_M0,
	M1_repeat_M1, M1_repeat_M0,
	M1_stride_M1, M1_stride_M0,
	M1_sizeof
};





enum
{
	MP1_value_L0, MP1_value_L1, MP1_value_L2, MP1_value_L3, MP1_value_L4, MP1_value_L5, MP1_value_L6, MP1_value_L7,
	MP1_info_L0, MP1_info_L1, MP1_info_L2, MP1_info_L3,
	MP1_poffset_L0, MP1_poffset_L1, MP1_poffset_L2, MP1_poffset_L3,
	MP1_repeat_L0, MP1_repeat_L1,
	MP1_stride_L0, MP1_stride_L1,
	MP1_padding_L0, MP1_padding_L1, MP1_padding_L2, MP1_padding_L3
};

enum
{
	MP1_value_M7, MP1_value_M6, MP1_value_M5, MP1_value_M4, MP1_value_M3, MP1_value_M2, MP1_value_M1, MP1_value_M0,
	MP1_info_M3, MP1_info_M2, MP1_info_M1, MP1_info_M0,
	MP1_poffset_M3, MP1_poffset_M2, MP1_poffset_M1, MP1_poffset_M0,
	MP1_repeat_M1, MP1_repeat_M0,
	MP1_stride_M1, MP1_stride_M0,
	MP1_padding_M3, MP1_padding_M2, MP1_padding_M1, MP1_padding_M0,
	MP1_sizeof
};





enum
{
	O_L0, O_L1, O_L2, O_L3
};

enum
{
	O_M3, O_M2, O_M1, O_M0,
	O_sizeof
};





enum
{
	W_L0, W_L1, W_L2, W_L3
};

enum
{
	W_M3, W_M2, W_M1, W_M0,
	W_sizeof
};





enum
{
	D1_tag_L0, D1_tag_L1, D1_tag_L2, D1_tag_L3,
	D1_val_L0, D1_val_L1, D1_val_L2, D1_val_L3
};

enum
{
	D1_tag_M3, D1_tag_M2, D1_tag_M1, D1_tag_M0,
	D1_val_M3, D1_val_M2, D1_val_M1, D1_val_M0,
	D1_sizeof
};


#define	E1_Nident	16




enum
{
	E1_ident, E1_ident_L_Z = E1_Nident - 1,
	E1_type_L0, E1_type_L1,
	E1_machine_L0, E1_machine_L1,
	E1_version_L0, E1_version_L1, E1_version_L2, E1_version_L3,
	E1_entry_L0, E1_entry_L1, E1_entry_L2, E1_entry_L3,
	E1_phoff_L0, E1_phoff_L1, E1_phoff_L2, E1_phoff_L3,
	E1_shoff_L0, E1_shoff_L1, E1_shoff_L2, E1_shoff_L3,
	E1_flags_L0, E1_flags_L1, E1_flags_L2, E1_flags_L3,
	E1_ehsize_L0, E1_ehsize_L1,
	E1_phentsize_L0, E1_phentsize_L1,
	E1_phnum_L0, E1_phnum_L1,
	E1_shentsize_L0, E1_shentsize_L1,
	E1_shnum_L0, E1_shnum_L1,
	E1_shstrndx_L0, E1_shstrndx_L1
};

enum
{
	E1_ident_M_Z = E1_Nident - 1,
	E1_type_M1, E1_type_M0,
	E1_machine_M1, E1_machine_M0,
	E1_version_M3, E1_version_M2, E1_version_M1, E1_version_M0,
	E1_entry_M3, E1_entry_M2, E1_entry_M1, E1_entry_M0,
	E1_phoff_M3, E1_phoff_M2, E1_phoff_M1, E1_phoff_M0,
	E1_shoff_M3, E1_shoff_M2, E1_shoff_M1, E1_shoff_M0,
	E1_flags_M3, E1_flags_M2, E1_flags_M1, E1_flags_M0,
	E1_ehsize_M1, E1_ehsize_M0,
	E1_phentsize_M1, E1_phentsize_M0,
	E1_phnum_M1, E1_phnum_M0,
	E1_shentsize_M1, E1_shentsize_M0,
	E1_shnum_M1, E1_shnum_M0,
	E1_shstrndx_M1, E1_shstrndx_M0,
	E1_sizeof
};




enum
{
	N1_namesz_L0, N1_namesz_L1, N1_namesz_L2, N1_namesz_L3,
	N1_descsz_L0, N1_descsz_L1, N1_descsz_L2, N1_descsz_L3,
	N1_type_L0, N1_type_L1, N1_type_L2, N1_type_L3
};

enum
{
	N1_namesz_M3, N1_namesz_M2, N1_namesz_M1, N1_namesz_M0,
	N1_descsz_M3, N1_descsz_M2, N1_descsz_M1, N1_descsz_M0,
	N1_type_M3, N1_type_M2, N1_type_M1, N1_type_M0,
	N1_sizeof
};




enum
{
	P1_type_L0, P1_type_L1, P1_type_L2, P1_type_L3,
	P1_offset_L0, P1_offset_L1, P1_offset_L2, P1_offset_L3,
	P1_vaddr_L0, P1_vaddr_L1, P1_vaddr_L2, P1_vaddr_L3,
	P1_paddr_L0, P1_paddr_L1, P1_paddr_L2, P1_paddr_L3,
	P1_filesz_L0, P1_filesz_L1, P1_filesz_L2, P1_filesz_L3,
	P1_memsz_L0, P1_memsz_L1, P1_memsz_L2, P1_memsz_L3,
	P1_flags_L0, P1_flags_L1, P1_flags_L2, P1_flags_L3,
	P1_align_L0, P1_align_L1, P1_align_L2, P1_align_L3
};

enum
{
	P1_type_M3, P1_type_M2, P1_type_M1, P1_type_M0,
	P1_offset_M3, P1_offset_M2, P1_offset_M1, P1_offset_M0,
	P1_vaddr_M3, P1_vaddr_M2, P1_vaddr_M1, P1_vaddr_M0,
	P1_paddr_M3, P1_paddr_M2, P1_paddr_M1, P1_paddr_M0,
	P1_filesz_M3, P1_filesz_M2, P1_filesz_M1, P1_filesz_M0,
	P1_memsz_M3, P1_memsz_M2, P1_memsz_M1, P1_memsz_M0,
	P1_flags_M3, P1_flags_M2, P1_flags_M1, P1_flags_M0,
	P1_align_M3, P1_align_M2, P1_align_M1, P1_align_M0,
	P1_sizeof
};





enum
{
	R1_offset_L0, R1_offset_L1, R1_offset_L2, R1_offset_L3,
	R1_info_L0, R1_info_L1, R1_info_L2, R1_info_L3
};

enum
{
	R1_offset_M3, R1_offset_M2, R1_offset_M1, R1_offset_M0,
	R1_info_M3, R1_info_M2, R1_info_M1, R1_info_M0,
	R1_sizeof
};





enum
{
	RA1_offset_L0, RA1_offset_L1, RA1_offset_L2, RA1_offset_L3,
	RA1_info_L0, RA1_info_L1, RA1_info_L2, RA1_info_L3,
	RA1_addend_L0, RA1_addend_L1, RA1_addend_L2, RA1_addend_L3
};

enum
{
	RA1_offset_M3, RA1_offset_M2, RA1_offset_M1, RA1_offset_M0,
	RA1_info_M3, RA1_info_M2, RA1_info_M1, RA1_info_M0,
	RA1_addend_M3, RA1_addend_M2, RA1_addend_M1, RA1_addend_M0,
	RA1_sizeof
};





enum
{
	SH1_name_L0, SH1_name_L1, SH1_name_L2, SH1_name_L3,
	SH1_type_L0, SH1_type_L1, SH1_type_L2, SH1_type_L3,
	SH1_flags_L0, SH1_flags_L1, SH1_flags_L2, SH1_flags_L3,
	SH1_addr_L0, SH1_addr_L1, SH1_addr_L2, SH1_addr_L3,
	SH1_offset_L0, SH1_offset_L1, SH1_offset_L2, SH1_offset_L3,
	SH1_size_L0, SH1_size_L1, SH1_size_L2, SH1_size_L3,
	SH1_link_L0, SH1_link_L1, SH1_link_L2, SH1_link_L3,
	SH1_info_L0, SH1_info_L1, SH1_info_L2, SH1_info_L3,
	SH1_addralign_L0, SH1_addralign_L1, SH1_addralign_L2, SH1_addralign_L3,
	SH1_entsize_L0, SH1_entsize_L1, SH1_entsize_L2, SH1_entsize_L3
};

enum
{
	SH1_name_M3, SH1_name_M2, SH1_name_M1, SH1_name_M0,
	SH1_type_M3, SH1_type_M2, SH1_type_M1, SH1_type_M0,
	SH1_flags_M3, SH1_flags_M2, SH1_flags_M1, SH1_flags_M0,
	SH1_addr_M3, SH1_addr_M2, SH1_addr_M1, SH1_addr_M0,
	SH1_offset_M3, SH1_offset_M2, SH1_offset_M1, SH1_offset_M0,
	SH1_size_M3, SH1_size_M2, SH1_size_M1, SH1_size_M0,
	SH1_link_M3, SH1_link_M2, SH1_link_M1, SH1_link_M0,
	SH1_info_M3, SH1_info_M2, SH1_info_M1, SH1_info_M0,
	SH1_addralign_M3, SH1_addralign_M2, SH1_addralign_M1, SH1_addralign_M0,
	SH1_entsize_M3, SH1_entsize_M2, SH1_entsize_M1, SH1_entsize_M0,
	SH1_sizeof
};





enum
{
	ST1_name_L0, ST1_name_L1, ST1_name_L2, ST1_name_L3,
	ST1_value_L0, ST1_value_L1, ST1_value_L2, ST1_value_L3,
	ST1_size_L0, ST1_size_L1, ST1_size_L2, ST1_size_L3,
	ST1_info_L,
	ST1_other_L,
	ST1_shndx_L0, ST1_shndx_L1
};

enum
{
	ST1_name_M3, ST1_name_M2, ST1_name_M1, ST1_name_M0,
	ST1_value_M3, ST1_value_M2, ST1_value_M1, ST1_value_M0,
	ST1_size_M3, ST1_size_M2, ST1_size_M1, ST1_size_M0,
	ST1_info_M,
	ST1_other_M,
	ST1_shndx_M1, ST1_shndx_M0,
	ST1_sizeof
};

enum
{
	SI1_boundto_M1, SI1_boundto_M0,
	SI1_flags_M1, SI1_flags_M0,
	SI1_sizeof
};

enum
{
	C1_tag_M3, C1_tag_M2, C1_tag_M1, C1_tag_M0,
	C1_val_M3, C1_val_M2, C1_val_M1, C1_val_M0,
	C1_sizeof
};

enum
{
	VD1_version_M1, VD1_version_M0,
	VD1_flags_M1, VD1_flags_M0,
	VD1_ndx_M1, VD1_ndx_M0,
	VD1_cnt_M1, VD1_cnt_M0,
	VD1_hash_M3, VD1_hash_M2, VD1_hash_M1, VD1_hash_M0,
	VD1_aux_M3, VD1_aux_M2, VD1_aux_M1, VD1_aux_M0,
	VD1_next_M3, VD1_next_M2, VD1_next_M1, VD1_next_M0,
	VD1_sizeof
};

enum
{
	VDA1_name_M3, VDA1_name_M2, VDA1_name_M1, VDA1_name_M0,
	VDA1_next_M3, VDA1_next_M2, VDA1_next_M1, VDA1_next_M0,
	VDA1_sizeof
};

enum
{
	VN1_version_M1, VN1_version_M0,
	VN1_cnt_M1, VN1_cnt_M0,
	VN1_file_M3, VN1_file_M2, VN1_file_M1, VN1_file_M0,
	VN1_aux_M3, VN1_aux_M2, VN1_aux_M1, VN1_aux_M0,
	VN1_next_M3, VN1_next_M2, VN1_next_M1, VN1_next_M0,
	VN1_sizeof
};

enum
{
	VNA1_hash_M3, VNA1_hash_M2, VNA1_hash_M1, VNA1_hash_M0,
	VNA1_flags_M1, VNA1_flags_M0,
	VNA1_other_M1, VNA1_other_M0,
	VNA1_name_M3, VNA1_name_M2, VNA1_name_M1, VNA1_name_M0,
	VNA1_next_M3, VNA1_next_M2, VNA1_next_M1, VNA1_next_M0,
	VNA1_sizeof
};

/*	x32 [dst_version - 1] [src_version - 1] [encode - 1] [type]
 */

/*
 *	size [version - 1] [type]
 */

static const struct {
	size_t	s_filesz,
		s_memsz;
} fmsize [EV_CURRENT] [ELF_T_NUM] =
{
	{					/* [1-1][.] */
/* BYTE */	{ 1, 1 },
/* ADDR */	{ A_sizeof, sizeof (Elf32_Addr) },
/* DYN */	{ D1_sizeof, sizeof (Elf32_Dyn) },
/* EHDR */	{ E1_sizeof, sizeof (Elf32_Ehdr) },
/* HALF */	{ H_sizeof, sizeof (Elf32_Half) },
/* OFF */	{ O_sizeof, sizeof (Elf32_Off) },
/* PHDR */	{ P1_sizeof, sizeof (Elf32_Phdr) },
/* RELA */	{ RA1_sizeof, sizeof (Elf32_Rela) },
/* REL */	{ R1_sizeof, sizeof (Elf32_Rel) },
/* SHDR */	{ SH1_sizeof, sizeof (Elf32_Shdr) },
/* SWORD */	{ W_sizeof, sizeof (Elf32_Sword) },
/* SYM */	{ ST1_sizeof, sizeof (Elf32_Sym) },
/* WORD */	{ W_sizeof, sizeof (Elf32_Word) },
/* VERDEF */	{ 1, 1},	/* because bot VERDEF & VERNEED have varying */
/* VERNEED */	{ 1, 1},	/* sized structures we set their sizes */
				/* to 1 byte */
/* SXWORD */			{ 0, 0 },	/* illegal 32-bit op */
/* XWORD */			{ 0, 0 },	/* illegal 32-bit op */
/* SYMINFO */	{ SI1_sizeof, sizeof (Elf32_Syminfo) },
/* NOTE */	{ 1, 1},	/* NOTE has varying sized data we can't */
				/*  use the usual table magic. */
	},
};


/*
 *	memory type [version - 1] [section type]
 */

static const Elf_Type	mtype[EV_CURRENT][SHT_NUM] =
{
	{			/* [1-1][.] */
/* NULL */		ELF_T_BYTE,
/* PROGBITS */		ELF_T_BYTE,
/* SYMTAB */		ELF_T_SYM,
/* STRTAB */		ELF_T_BYTE,
/* RELA */		ELF_T_RELA,
/* HASH */		ELF_T_WORD,
/* DYNAMIC */		ELF_T_DYN,
/* NOTE */		ELF_T_NOTE,
/* NOBITS */		ELF_T_BYTE,
/* REL */		ELF_T_REL,
/* SHLIB */		ELF_T_BYTE,
/* DYNSYM */		ELF_T_SYM,
/* UNKNOWN12 */		ELF_T_BYTE,
/* UNKNOWN13 */		ELF_T_BYTE,
/* INIT_ARRAY */	ELF_T_ADDR,
/* FINI_ARRAY */	ELF_T_ADDR,
/* PREINIT_ARRAY */	ELF_T_ADDR,
/* GROUP */		ELF_T_WORD,
/* SYMTAB_SHNDX */	ELF_T_WORD
	},
};


size_t
elf32_fsize(Elf_Type type, size_t count, unsigned ver)
{
	if (--ver >= EV_CURRENT) {
		_elf_seterr(EREQ_VER, 0);
		return (0);
	}
	if ((unsigned)type >= ELF_T_NUM) {
		_elf_seterr(EREQ_TYPE, 0);
		return (0);
	}
	return (fmsize[ver][type].s_filesz * count);
}


size_t
_elf32_msize(Elf_Type type, unsigned ver)
{
	return (fmsize[ver - 1][type].s_memsz);
}


Elf_Type
_elf32_mtype(Elf * elf, Elf32_Word shtype, unsigned ver)
{
#pragma unused(elf)

	if (shtype < SHT_NUM)
		return (mtype[ver - 1][shtype]);
	/*
	 * And the default is ELF_T_BYTE - but we should
	 * certainly have caught any sections we know about
	 * above.  This is for unknown sections to libelf.
	 */
	return (ELF_T_BYTE);
}

/*
 * Determine the data encoding used by the current system.
 */
static uint_t
_elf_sys_encoding(void)
{
	union {
		Elf32_Word	w;
		unsigned char	c[W_sizeof];
	} u;

	u.w = 0x10203;
	/*CONSTANTCONDITION*/
	if (~(Elf32_Word)0 == -(Elf32_Sword)1 && (((((((Elf32_Word)(u.c)[W_L3]<<8)
		+(u.c)[W_L2])<<8)
		+(u.c)[W_L1])<<8)
		+(u.c)[W_L0]) == 0x10203)
		return (ELFDATA2LSB);

	/*CONSTANTCONDITION*/
	if (~(Elf32_Word)0 == -(Elf32_Sword)1 && (((((((Elf32_Word)(u.c)[W_M3]<<8)
		+(u.c)[W_M2])<<8)
		+(u.c)[W_M1])<<8)
		+(u.c)[W_M0]) == 0x10203)
		return (ELFDATA2MSB);

	/* Not expected to occur */
	return (ELFDATANONE);
}


/*
 * XX64	This routine is also used to 'version' interactions with Elf64
 *	applications, but there's no way to figure out if the caller is
 *	asking Elf32 or Elf64 questions, even though it has Elf32
 *	dependencies.  Ick.
 */
unsigned
elf_version(unsigned ver)
{
	register unsigned	j;

	if (ver == EV_NONE)
		return EV_CURRENT;
	if (ver > EV_CURRENT)
	{
		_elf_seterr(EREQ_VER, 0);
		return EV_NONE;
	}
	(void) mutex_lock(&_elf_globals_mutex);
	if (_elf_work != EV_NONE)
	{
		j = _elf_work;
		_elf_work = ver;
		(void) mutex_unlock(&_elf_globals_mutex);
		return j;
	}
	_elf_work = ver;

	_elf_encode = _elf_sys_encoding();

	(void) mutex_unlock(&_elf_globals_mutex);

	return ver;
}


static Elf_Data *
xlate(Elf_Data *dst, const Elf_Data *src, unsigned encode)
{
	size_t		cnt, dsz, ssz;
	unsigned	type;
	unsigned	dver, sver;
	unsigned	_encode;

	if (dst == 0 || src == 0)
		return (0);
	if (--encode >= (ELFDATANUM - 1)) {
		_elf_seterr(EREQ_ENCODE, 0);
		return (0);
	}
	if ((dver = dst->d_version - 1) >= EV_CURRENT ||
	    (sver = src->d_version - 1) >= EV_CURRENT) {
		_elf_seterr(EREQ_VER, 0);
		return (0);
	}
	if ((type = src->d_type) >= ELF_T_NUM) {
		_elf_seterr(EREQ_TYPE, 0);
		return (0);
	}

	dsz = fmsize[dver][type].s_memsz;
	ssz = fmsize[sver][type].s_filesz;

	cnt = src->d_size / ssz;
	if (dst->d_size < dsz * cnt) {
		_elf_seterr(EREQ_DSZ, 0);
		return (0);
	}

	ELFACCESSDATA(_encode, _elf_encode)
	if ((_encode == (encode + 1)) && (dsz == ssz)) {
		/*
		 *	ld(1) frequently produces empty sections (eg. .dynsym,
		 *	.dynstr, .symtab, .strtab, etc) so that the initial
		 *	output image can be created of the correct size.  Later
		 *	these sections are filled in with the associated data.
		 *	So that we don't have to pre-allocate buffers for
		 *	these segments, allow for the src destination to be 0.
		 */
		if (src->d_buf && src->d_buf != dst->d_buf)
			(void) memcpy(dst->d_buf, src->d_buf, src->d_size);
		dst->d_type = src->d_type;
		dst->d_size = src->d_size;
		return (dst);
	}
	else {
		_elf_seterr(EREQ_NOTSUP, 0);
		return (0);
	}
}

Elf_Data *
elf32_xlatetom(Elf_Data *dst, const Elf_Data *src, unsigned encode)
{
	return (xlate(dst, src, encode));
}
