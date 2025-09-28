/* Alpha VMS external format of Extended Image Header.

   Copyright (C) 2010-2023 Free Software Foundation, Inc.
   Written by Tristan Gingold <gingold@adacore.com>, AdaCore.

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

#ifndef _VMS_EIHD_H
#define _VMS_EIHD_H

/* Extended Image Header (eihd) structure.  */
struct vms_eihd
{
  /* Version of this EIHD.  */
  unsigned char majorid[4];
  unsigned char minorid[4];

  /* Size in bytes of the header.  */
  unsigned char size[4];

  /* Byte offset to ISD (Image Section Descriptors) list.  */
  unsigned char isdoff[4];

  /* Byte offset to activation data (off=16).  */
  unsigned char activoff[4];

  /* Byte offset to symbol table and debugging data.  */ 
  unsigned char symdbgoff[4];

  /* Byte offset to image ident.  */
  unsigned char imgidoff[4];

  /* Byte offset to patch data.  */
  unsigned char patchoff[4];

  /* RVA of fixup info (off=32).  */
  unsigned char iafva[8];

  /* RVA of symbol vector.  */
  unsigned char symvva[8];

  /* Byte offset to version number array (off=48).  */
  unsigned char version_array_off[4];

  /* Image type.  */
  unsigned char imgtype[4];

  /* Image subtype.  */
  unsigned char subtype[4];

  /* Size in bytes of image I/O section requested.  */
  unsigned char imgiocnt[4];

  /* Nbr of channels requested (off=64).  */
  unsigned char iochancnt[4];

  /* Requested privilege mask.  */
  unsigned char privreqs[8];

  /* Number of header diskblocks.  */
  unsigned char hdrblkcnt[4];

  /* Linker produced image flags.  */
  unsigned char lnkflags[4];

  /* GBL SEC ident value for linkable image.  */
  unsigned char ident[4];

  /* SYS$K_VERSION or 0 if not linked with exec.  */
  unsigned char sysver[4];

  /* Linker match control.  */
  unsigned char matchctl;
  unsigned char fill_1[3];

  /* Size of the symbol vector in bytes.  */
  unsigned char symvect_size[4];

  /* Value of /BPAGE.  */
  unsigned char virt_mem_block_size[4];

  /* Byte offset to extended fixup data.  */
  unsigned char ext_fixup_off[4];

  /* Byte offset to no_optimize psect table.  */
  unsigned char noopt_psect_off[4];

  unsigned char fill_2[398];

  /* CODE identifies image type to MOM.  */
  unsigned char alias[2];
};

#define EIHD__K_MAJORID	3	/* Major id constant	*/
#define EIHD__K_MINORID	0	/* Minor id constant	*/

/* Image type.  */
#define EIHD__K_EXE		1	/* Executable image	*/
#define EIHD__K_LIM		2	/* Linkable image.  */

/* Image subtype.  */
#define EIHD__C_NATIVE		0	/* Alpha native image.  */
#define EIHD__C_CLI		1	/* Image is a CLI, run LOGINOUT.  */

/* Linker image flags.  */
#define EIHD__M_LNKDEBUG	0x0001	/* Full debugging requested.  */
#define EIHD__M_LNKNOTFR	0x0002	/* No first transfer address.  */
#define EIHD__M_NOP0BUFS	0x0004	/* No RMS use of P0 for image I/O.  */
#define EIHD__M_PICIMG		0x0008	/* PIC image.  */
#define EIHD__M_P0IMAGE		0x0010	/* P0 only image.  */
#define EIHD__M_DBGDMT		0x0020	/* Image header has dmt fields.  */
#define EIHD__M_INISHR		0x0040	/* Transfer array contains LNISHR.  */
#define EIHD__M_XLATED		0x0080	/* Translated image.  */
#define EIHD__M_BIND_CODE_SEC	0x0100	/* EXE sect can be put into S0.  */
#define EIHD__M_BIND_DATA_SEC	0x0200	/* DATA sect can be put into S0.  */
#define EIHD__M_MKTHREADS	0x0400	/* Multiple kernel threads.  */
#define EIHD__M_UPCALLS		0x0800	/* Upcalls enabled.  */
#define EIHD__M_OMV_READY	0x1000	/* Can be processed by OMV.  */
#define EIHD__M_EXT_BIND_SECT	0x2000	/* May be moved, using ext fixups.  */

/* Offsets of some fields.  */
#define EIHD__L_SIZE		8
#define EIHD__L_ISDOFF		12
#define EIHD__L_SYMDBGOFF	20
#define EIHD__Q_SYMVVA		40
#define EIHD__L_IMGTYPE		52

#define EIHD__C_LENGTH 104

#endif /* _VMS_EIHD_H */
