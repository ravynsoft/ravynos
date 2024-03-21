/* Alpha VMS external format of Extended Module Header.

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

#ifndef _VMS_EMH_H
#define _VMS_EMH_H

#define EMH__C_MHD 0		/* Main header record.		*/
#define EMH__C_LNM 1		/* Language name and version.	*/
#define EMH__C_SRC 2		/* Source file specification.	*/
#define EMH__C_TTL 3		/* Title text of module.	*/
#define EMH__C_CPR 4		/* Copyright notice.		*/
#define EMH__C_MTC 5		/* Maintenance status.		*/
#define EMH__C_GTX 6		/* General text.		*/
#define EMH__C_MAXHDRTYP 6	/* Maximum allowable type.	*/

struct vms_emh_common
{
  /* Record type.  */
  unsigned char rectyp[2];

  /* Record size.  */
  unsigned char size[2];

  /* Subtype.  */
  unsigned char subtyp[2];
};

struct vms_emh_mhd
{
  struct vms_emh_common common;

  unsigned char strlvl;

  unsigned char temp;

  unsigned char arch1[4];
  unsigned char arch2[4];

  unsigned char recsiz[4];

  /* Module name: ASCIC.  */
  /* Module version: ASCIC.  */
  /* Compile data: ASCIC.  */
};

#define EOBJ__C_MAXRECSIZ 8192  /* Maximum legal record size.  */
#define EOBJ__C_STRLVL 2	/* Structure level.  */
#define EOBJ__C_SYMSIZ 64	/* Maximum symbol length.  */
#define EOBJ__C_SECSIZ 31	/* Maximum section name length.  */
#define EOBJ__C_STOREPLIM -1	/* Maximum repeat count on store commands.  */
#define EOBJ__C_PSCALILIM 16	/* Maximum p-sect alignment.  */

struct vms_emh_lnm
{
  struct vms_emh_common common;

  /* Language processor name: ASCII.  */
};

#endif /* _VMS_EMH_H */
