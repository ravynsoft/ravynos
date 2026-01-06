#ifndef _COFF_SCNHDR_H
#define _COFF_SCNHDR_H
/*
 * These data structures are discribed in the pecoff_v8.doc in section
 * "4. Section Table (Section Headers)"
 */
#include <stdint.h>

/*
 * Each row of the section table is, in effect, a section header. This table
 * immediately follows the optional header, if any. This positioning is
 * required because the file header does not contain a direct pointer to the
 * section table. Instead, the location of the section table is determined by
 * calculating the location of the first byte after the headers.
 *
 * The number of entries in the section table is given by the NumberOfSections
 * field in the file header. Entries in the section table are numbered starting
 * from one (1). The code and data memory section entries are in the order
 * chosen by the linker.
 *
 * In an image file, the VAs for sections must be assigned by the linker so
 * that they are in ascending order and adjacent, and they must be a multiple
 * of the SectionAlignment value in the optional header.
 *
 * Since definitions for this header were based from the GNU binutils
 * coff/pe.h header file that copyright info is below.
 */

/* pe.h  -  PE COFF header information 

   Copyright 2000, 2001, 2003, 2004 Free Software Foundation, Inc.

   This file is part of BFD, the Binary File Descriptor library.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

struct scnhdr
{
	char s_name[8];		/* section name				*/
	uint32_t s_vsize;	/* virtual size				*/
	uint32_t s_vaddr;	/* virtual address			*/
	uint32_t s_size;	/* section size				*/
	uint32_t s_scnptr;	/* file ptr to raw data for section 	*/
	uint32_t s_relptr;	/* file ptr to relocation		*/
	uint32_t s_lnnoptr;	/* file ptr to line numbers		*/
	uint16_t s_nreloc;	/* number of relocation entries		*/
	uint16_t s_nlnno;	/* number of line number entries	*/
	uint32_t s_flags;	/* flags				*/
};

/* values or'ed into the s_flags field */
#define IMAGE_SCN_MEM_DISCARDABLE 0x02000000
#define IMAGE_SCN_MEM_EXECUTE     0x20000000
#define IMAGE_SCN_MEM_READ        0x40000000
#define IMAGE_SCN_MEM_WRITE       0x80000000

#define IMAGE_SCN_CNT_CODE                   0x00000020  /* Section contains code. */
#define IMAGE_SCN_CNT_INITIALIZED_DATA       0x00000040  /* Section contains initialized data. */
#define IMAGE_SCN_CNT_UNINITIALIZED_DATA     0x00000080  /* Section contains uninitialized data. */

#endif /* _COFF_SCNHDR_H */
