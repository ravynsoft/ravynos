/* Alpha VMS external format of Extended Global Symbol.

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

#ifndef _VMS_EGSY_H
#define _VMS_EGSY_H

#define EGSY__W_FLAGS  6

#define EGSY__V_WEAK		0x0001	/* Weak symbol definition.  */
#define EGSY__V_DEF		0x0002	/* Symbol definition.  */
#define EGSY__V_UNI		0x0004	/* Reserved.  */
#define EGSY__V_REL		0x0008	/* Relocatable (vs absolute).  */
#define EGSY__V_COMM		0x0010	/* Conditional symbol def.  */
#define EGSY__V_VECEP		0x0020	/* Reserved.  */
#define EGSY__V_NORM		0x0040	/* Normal procedure definition.  */
#define EGSY__V_QUAD_VAL	0x0080	/* Value exceed 32 bits.  */

struct vms_egsy
{
  /* Entry type.  */
  unsigned char gsdtyp[2];

  /* Length of the entry.  */
  unsigned char gsdsiz[2];

  /* Data type.  */
  unsigned char datyp;

  /* Pad for alignment.  */
  unsigned char temp;

  unsigned char flags[2];
};

#endif /* _VMS_EGSY_H */
