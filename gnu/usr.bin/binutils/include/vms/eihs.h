/* Alpha VMS external format of Extended Image Symbols and debug table.

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

#ifndef _VMS_EIHS_H
#define _VMS_EIHS_H

#define EIHS__K_MAJORID 1
#define EIHS__K_MINORID 1

struct vms_eihs
{
  unsigned char majorid[4];
  unsigned char minorid[4];

  /* Debug symbol table virtual block number (vbn).  */
  unsigned char dstvbn[4];

  /* Debug symbol table size.  */
  unsigned char dstsize[4];

  /* Global symbol table vbn.  */
  unsigned char gstvbn[4];

  /* Global symtol table size.  */
  unsigned char gstsize[4];

  /* Debug module table vbn.  */
  unsigned char dmtvbn[4];

  /* Debug module table size.  */
  unsigned char dmtsize[4];
};

/* Various offsets.  */

#define EIHS__L_DSTVBN		 8
#define EIHS__L_DSTSIZE		12
#define EIHS__L_GSTVBN		16
#define EIHS__L_GSTSIZE		20
#define EIHS__L_DMTVBN		24
#define EIHS__L_DMTBYTES	28

#endif /* _VMS_EIHS_H */
