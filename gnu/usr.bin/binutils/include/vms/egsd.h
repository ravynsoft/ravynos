/* Alpha VMS external format of Extended Global Symbol Directory.

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

#ifndef _VMS_EGSD_H
#define _VMS_EGSD_H

#define EGSD__K_ENTRIES 2	/* Offset to first entry in record.	*/
#define EGSD__C_ENTRIES 2	/* Offset to first entry in record.	*/
#define EGSD__C_PSC 0		/* Psect definition.			*/
#define EGSD__C_SYM 1		/* Symbol specification.		*/
#define EGSD__C_IDC 2		/* Random entity check.			*/
#define EGSD__C_SPSC 5		/* Shareable image psect definition.	*/
#define EGSD__C_SYMV 6		/* Vectored (dual-valued) versions of SYM.  */
#define EGSD__C_SYMM 7		/* Masked versions of SYM.		*/
#define EGSD__C_SYMG 8		/* EGST - gst version of SYM.		*/
#define EGSD__C_MAXRECTYP 8	/* Maximum entry type defined.		*/

struct vms_egsd
{
  /* Record type.  */
  unsigned char rectyp[2];

  /* Record size.  */
  unsigned char recsiz[2];

  /* Padding for alignment.  */
  unsigned char alignlw[4];

  /* Followed by egsd entries.  */
};

struct vms_egsd_entry
{
  /* Entry type.  */
  unsigned char gsdtyp[2];

  /* Length of the entry.  */
  unsigned char gsdsiz[2];
};

#endif /* _VMS_EGSD_H */
