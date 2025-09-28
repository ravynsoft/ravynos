/* Alpha VMS external format of Extended Object Records.

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

#ifndef _VMS_EOBJREC_H
#define _VMS_EOBJREC_H

#define EOBJ__C_EMH 8		/* EVAX mdule header record.  */
#define EOBJ__C_EEOM 9		/* EVAX ed of module record.  */
#define EOBJ__C_EGSD 10		/* EVAX gobal symbol definition record.  */
#define EOBJ__C_ETIR 11		/* EVAX txt information record.  */
#define EOBJ__C_EDBG 12		/* EVAX Dbugger information record.  */
#define EOBJ__C_ETBT 13		/* EVAX Taceback information record.  */
#define EOBJ__C_MAXRECTYP 13	/* EVAX Lst assigned record type.  */

struct vms_eobjrec
{
  /* Record type.  */
  unsigned char rectyp[2];

  /* Record size.  */
  unsigned char size[2];

#if 0
  /* Record subtype.  */
  unsigned char subtyp[2];
#endif
};

#endif /* _VMS_EOBJREC_H */
