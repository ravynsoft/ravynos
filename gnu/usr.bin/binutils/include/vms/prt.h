/* Alpha VMS external format of Protection values.

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

#ifndef _VMS_PRT_H
#define _VMS_PRT_H

#define PRT__C_NA	0	/* No access.  */
#define PRT__C_RESERVED 1
#define PRT__C_KW	2	/* Kernel RW.  */
#define PRT__C_KR	3	/* Kernel RO. */
#define PRT__C_UW	4	/* User RW.  */
#define PRT__C_EW	5	/* Executive RW.  */
#define PRT__C_ERKW	6	/* Executive RO, Kernel RW.  */
#define PRT__C_ER	7	/* Executive RO.  */
#define PRT__C_SW	8	/* Supervisor RW.  */
#define PRT__C_SREW	9	/* Supervisor RO, Executive RW.  */
#define PRT__C_SRKW	10	/* Supervisor RO, Kernel RW.  */
#define PRT__C_SR	11	/* Supervisor RO.  */
#define PRT__C_URSW	12	/* User RO, Supervisor RW.  */
#define PRT__C_UREW	13	/* User RO, Executive RW.  */
#define PRT__C_URKW	14	/* User RO, Kernel RW.  */
#define PRT__C_UR	15	/* User RO.  */

#endif /* _VMS_PRT_H */
