#ifndef _COFF_FILEHDR_H
#define _COFF_FILEHDR_H
/*
 * These data structures are discribed in the pecoff_v8.doc in section
 * "3.3. COFF File Header (Object and Image)"
 */
#include <stdint.h>

/*
 * At the beginning of an object file, or immediately after the signature of an 
 * image file, is a standard COFF file header in the following format. Note
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

struct filehdr
{
	uint16_t f_magic;		/* Magic number.		*/
	uint16_t f_nscns;		/* Number of sections.		*/
	uint32_t f_timdat;		/* Time & date stamp.		*/
	uint32_t f_symptr;		/* File pointer to symtab.	*/
	uint32_t f_nsyms;		/* Number of symtab entries.	*/
	uint16_t f_opthdr;		/* Sizeof(optional hdr).	*/
	uint16_t f_flags;		/* Flags.			*/
};

/* Machine numbers (for the f_magic field).  */
#define IMAGE_FILE_MACHINE_ARM               0x01c0
#define IMAGE_FILE_MACHINE_ARM64             0x01c6
#define IMAGE_FILE_MACHINE_I386              0x014c
#define IMAGE_FILE_MACHINE_AMD64             0x8664

/* NT specific file attributes (for the f_flags field).  */
#define IMAGE_FILE_EXECUTABLE_IMAGE          0x0002
#define IMAGE_FILE_LINE_NUMS_STRIPPED        0x0004
#define IMAGE_FILE_LOCAL_SYMS_STRIPPED       0x0008
#define IMAGE_FILE_32BIT_MACHINE             0x0100
#define IMAGE_FILE_DEBUG_STRIPPED            0x0200

#endif /* _COFF_FILEHDR_H */
