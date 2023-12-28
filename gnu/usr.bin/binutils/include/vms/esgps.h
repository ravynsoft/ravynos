/* Alpha VMS external format of Extended Shared Program Section Definition.

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

#ifndef _VMS_ESGPS_H
#define _VMS_ESGPS_H

struct vms_esgps
{
  /* Entry type.  */
  unsigned char gsdtyp[2];

  /* Length of the entry.  */
  unsigned char gsdsiz[2];

  /* Psect alignment.  */
  unsigned char align;

  /* Pad for alignment.  */
  unsigned char temp;

  unsigned char flags[2];

  /* Length of this contribution.  */
  unsigned char alloc[4];

  /* Image offset of the psect.  */
  unsigned char base[4];

  /* Symbol vector offset.  */
  unsigned char value[8];

  /* Name.  */
  unsigned char namlng;
  unsigned char name[31];
};

/* These are the same as EGPS flags.  */

#define ESGPS__V_PIC (1 << 0)	/* Not meaningful.  */
#define ESGPS__V_LIB (1 << 1)	/* Defined in a shareable image.  */
#define ESGPS__V_OVR (1 << 2)	/* Overlaid contribution.  */
#define ESGPS__V_REL (1 << 3)	/* Relocatable.  */
#define ESGPS__V_GBL (1 << 4)	/* Global.  */
#define ESGPS__V_SHR (1 << 5)	/* Shareable.  */
#define ESGPS__V_EXE (1 << 6)	/* Executable.  */
#define ESGPS__V_RD  (1 << 7)	/* Readable.  */
#define ESGPS__V_WRT (1 << 8)	/* Writable.  */
#define ESGPS__V_VEC (1 << 9)	/* Change mode dispatch or message vectors.  */
#define ESGPS__V_NOMOD (1 << 10)	/* Demand-zero.  */
#define ESGPS__V_COM (1 << 11)	/* Conditional storage.  */
#define ESGPS__V_ALLOC_64BIT (1 << 12)	/* Allocated in 64-bit space.  */

#endif /* _VMS_ESGPS_H */
