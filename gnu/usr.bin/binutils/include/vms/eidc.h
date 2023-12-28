/* Alpha VMS external format of Ident Consistency check.

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

#ifndef _VMS_EIDC_H
#define _VMS_EIDC_H

struct vms_eidc
{
  /* Record type.  */
  unsigned char rectyp[2];

  /* Record size.  */
  unsigned char recsiz[2];

  unsigned char flags[4];

  /* Entity name (ASCIC).  */
  /* Object name (ASCIC).  */
  /* Ident string (ASCIC or binary BINIDENT set).  */
  unsigned char name[1];
};

/* Fields of flags.  */
#define EIDC__V_BINIDENT	(1 << 0)	/* Ident is a longword.  */
#define EIDC__V_IDMATCH_SH	1		/* Ident match control.  */
#define EIDC__V_IDMATCH_MASK	3
#define EIDC__V_ERRSEV_SH	3		/* Error severity.  */
#define EIDC__V_ERRSEV_MASK	7

#endif /* _VMS_EIDC_H */
