/* Alpha VMS external format for DeCompression.

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

#ifndef _VMS_DCX_H
#define _VMS_DCX_H

struct vms_dcxmap
{
  unsigned char size[4];
  unsigned char version[2];

  unsigned char pad[2];
  unsigned char sanity[4];
  unsigned char flags[4];
  unsigned char nsubs[2];
  unsigned char sub0[2];
};

struct vms_dcxsbm
{
  unsigned char size[2];
  unsigned char min_char;
  unsigned char max_char;
  unsigned char escape;
  unsigned char flags_bits;
  unsigned char flags[2];
  unsigned char nodes[2];
  unsigned char next[2];
};

#endif /* _VMS_DCX_H */
