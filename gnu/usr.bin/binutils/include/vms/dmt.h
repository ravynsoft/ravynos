/* Alpha VMS external format of Debug Module Table.

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

#ifndef _VMS_DMT_H
#define _VMS_DMT_H

struct vms_dmt_header
{
  /* Offset in the DST of the module.  */
  unsigned char modbeg[4];

  /* Size of the DST chunk for this module.  */
  unsigned char size[4];

  /* Number of psect for this module.  */
  unsigned char psect_count[2];

  unsigned char mbz[2];
};

struct vms_dmt_psect
{
  /* Address of the psect.  */
  unsigned char start[4];

  /* Length of the psect.  */
  unsigned char length[4];
};
#endif /* _VMS_DMT_H */
