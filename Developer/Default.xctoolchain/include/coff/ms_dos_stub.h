#ifndef _COFF_MS_DOS_STUB_H
#define _COFF_MS_DOS_STUB_H
/*
 * These data structures are discussed be not discribed in detail in
 * pecoff_v8.doc in section "3.1. MS DOS Stub (Image Only)"
 */
#include <stdint.h>

/*
 * The MS DOS stub is a valid application that runs under MS DOS. It is placed
 * at the front of a pecoff image. The default stub prints out the message "This
 * program cannot be run in DOS mode" when the image is run in MS DOS.
 *
 * At location 0x3c (the e_lfanew field), the stub has the file offset to the
 * PE signature. This information enables Windows to properly execute the image 
 * file, even though it has an MS DOS stub. This file offset is placed at
 * location 0x3c during linking.
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

/* Magic values that are true for all dos/nt implementations.  */
#define DOSMAGIC       0x5a4d  

struct ms_dos_stub
{
  /* DOS header fields - always at offset zero in the EXE file.  */
  uint16_t e_magic;	/* Magic number, 0x5a4d. (DOSMAGIC above) */
  uint16_t e_cblp;	/* Bytes on last page of file, 0x90.  */
  uint16_t e_cp;	/* Pages in file, 0x3.  */
  uint16_t e_crlc;	/* Relocations, 0x0.  */
  uint16_t e_cparhdr;	/* Size of header in paragraphs, 0x4.  */
  uint16_t e_minalloc;	/* Minimum extra paragraphs needed, 0x0.  */
  uint16_t e_maxalloc;	/* Maximum extra paragraphs needed, 0xFFFF.  */
  uint16_t e_ss;	/* Initial (relative) SS value, 0x0.  */
  uint16_t e_sp;	/* Initial SP value, 0xb8.  */
  uint16_t e_csum;	/* Checksum, 0x0.  */
  uint16_t e_ip;	/* Initial IP value, 0x0.  */
  uint16_t e_cs;	/* Initial (relative) CS value, 0x0.  */
  uint16_t e_lfarlc;	/* File address of relocation table, 0x40.  */
  uint16_t e_ovno;	/* Overlay number, 0x0.  */
  uint16_t e_res[4];	/* Reserved words, all 0x0.  */
  uint16_t e_oemid;	/* OEM identifier (for e_oeminfo), 0x0.  */
  uint16_t e_oeminfo;	/* OEM information; e_oemid specific, 0x0.  */
  uint16_t e_res2[10];	/* Reserved words, all 0x0.  */
  uint32_t e_lfanew;	/* File address of new exe header, usually 0x80.  */
  char dos_program[64];	/* MS-DOS ,stufa always follow DOS header.  */
};

#endif /* _COFF_MS_DOS_STUB_H */
