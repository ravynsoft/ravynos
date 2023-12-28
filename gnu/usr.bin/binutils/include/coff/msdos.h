/* msdos.h  -  MS-DOS and derived executable header information

   Copyright (C) 1999-2023 Free Software Foundation, Inc.

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
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.  */
#ifndef _MSDOS_H
#define _MSDOS_H

#define IMAGE_DOS_SIGNATURE	0x5a4d		/* MZ */
#define IMAGE_OS2_SIGNATURE	0x454e		/* NE */
#define IMAGE_OS2_SIGNATURE_LE	0x454c		/* LE */
#define IMAGE_OS2_SIGNATURE_LX	0x584c		/* LX */
#define IMAGE_NT_SIGNATURE	0x00004550	/* PE\0\0 */

struct external_DOS_hdr
{
  /* DOS header fields - always at offset zero in the EXE file.  */
  char e_magic[2];		/* Magic number.  */
  char e_cblp[2];		/* Bytes on last page of file.  */
  char e_cp[2];			/* Pages in file.  */
  char e_crlc[2];		/* Relocations.  */
  char e_cparhdr[2];		/* Size of header in paragraphs.  */
  char e_minalloc[2];		/* Minimum extra paragraphs needed.  */
  char e_maxalloc[2];		/* Maximum extra paragraphs needed.  */
  char e_ss[2];			/* Initial (relative) SS value.  */
  char e_sp[2];			/* Initial SP value.  */
  char e_csum[2];		/* Checksum.  */
  char e_ip[2];			/* Initial IP value.  */
  char e_cs[2];			/* Initial (relative) CS value.  */
  char e_lfarlc[2];		/* File address of relocation table.  */
  char e_ovno[2];		/* Overlay number.  */
  char e_res[4][2];		/* Reserved words, all 0x0.  */
  char e_oemid[2];		/* OEM identifier.  */
  char e_oeminfo[2];		/* OEM information.  */
  char e_res2[10][2];		/* Reserved words, all 0x0.  */
  char e_lfanew[4];		/* File address of new exe header, usually 0x80.  */
  char dos_message[16][4];	/* Other stuff, always follow DOS header.  */
};

/* The actual DOS header only includes up to the e_ovno field.  */
#define DOS_HDR_SIZE		(offsetof (struct external_DOS_hdr, e_res))

#endif /* _MSDOS_H */
