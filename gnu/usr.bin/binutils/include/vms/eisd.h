/* Alpha VMS external format of Extended Image Section Descriptor.

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

#ifndef _VMS_EISD_H
#define _VMS_EISD_H

/* Flags.  */
#define EISD__M_GBL		0x0001	/* Global.  */
#define EISD__M_CRF		0x0002	/* Copy on reference.  */
#define EISD__M_DZRO		0x0004	/* Demand zero page.  */
#define EISD__M_WRT		0x0008	/* Writable.  */
#define EISD__M_INITALCODE	0x0010	/* Part of initialization code.  */
#define EISD__M_BASED		0x0020	/* Isect is based.  */
#define EISD__M_FIXUPVEC	0x0040	/* Isect is fixup section.  */
#define EISD__M_RESIDENT	0x0080	/* Isect is memory resident.  */
#define EISD__M_VECTOR		0x0100	/* Vector contained in isect.  */
#define EISD__M_PROTECT		0x0200	/* Isect is protected.  */
#define EISD__M_LASTCLU		0x0400	/* Last cluster.  */
#define EISD__M_EXE		0x0800	/* Code isect.  */
#define EISD__M_NONSHRADR	0x1000	/* Contains non-shareable data.  */
#define EISD__M_QUAD_LENGTH	0x2000	/* Quad length field valid.  */
#define EISD__M_ALLOC_64BIT	0x4000	/* Allocate 64-bit space.  */

#define EISD__K_LEN 36
#define EISD__K_LENEND 12	/* For end marker or next block.  */
#define EISD__K_MAXLENGLBL 84

#define EISD__K_GBLNAMLEN 44	/* Size of the field.  */

struct vms_eisd
{
  unsigned char majorid[4];
  unsigned char minorid[4];

  /* Size (in bytes) of this eisd.  */
  unsigned char eisdsize[4];

  /* Size (in bytes) of the section.  */
  unsigned char secsize[4];

  /* Virtual address of the section.  */
  unsigned char virt_addr[8];

  /* Flags.  */
  unsigned char flags[4];

  /* Base virtual block number.  */
  unsigned char vbn[4];

  /* Page fault cluster.  */
  unsigned char pfc;

  /* Linker match control.  */
  unsigned char matchctl;

  /* Section type.  */
  unsigned char type;

  unsigned char fill_1;

  /* End of structure for normal records.  */

  /* Ident for global section.  */
  unsigned char ident[4];
      
  /* Global name ascic.  First 8 bytes are quad length field.  */
  unsigned char gblnam[EISD__K_GBLNAMLEN];
};

/* Versions.  */
#define EISD__K_MAJORID 1
#define EISD__K_MINORID 1

/* Match control.  */
#define EISD__K_MATALL 0	/* Match always.  */
#define EISD__K_MATEQU 1	/* Match if equal.  */
#define EISD__K_MATLEQ 2	/* Match if less or equal.  */
#define EISD__K_MATNEV 3	/* Match never.  */

/* Section type.  */
#define EISD__K_NORMAL 0	/* Normal program image section.  */
#define EISD__K_SHRFXD 1	/* Shareable fixed section.  */
#define EISD__K_PRVFXD 2	/* Private fixed section.  */
#define EISD__K_SHRPIC 3	/* Shareable pic section.  */
#define EISD__K_PRVPIC 4	/* Private PIC section.  */
#define EISD__K_USRSTACK 253	/* User stack section.  */

/* EISD offsets.  */

#define EISD__L_EISDSIZE	 8
#define EISD__L_SECSIZE		12
#define EISD__Q_VIR_ADDR	16
#define EISD__L_FLAGS		24
#define EISD__L_VBN		28
#define EISD__R_CONTROL		32
#define EISD__L_IDENT		36
#define EISD__T_GBLNAM		40

#endif /* _VMS_EISD_H */
