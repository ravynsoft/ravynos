/* Alpha VMS internal format.

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

#ifndef _VMS_INTERNAL_H
#define _VMS_INTERNAL_H

struct vms_internal_eisd
{
  unsigned int majorid;		/* Version.  */
  unsigned int minorid;
  unsigned int eisdsize;	/* Size (in bytes) of this eisd.  */
  unsigned int secsize;		/* Size (in bytes) of the section.  */
  bfd_vma virt_addr;		/* Virtual address of the section.  */
  unsigned int flags;		/* Flags.  */
  unsigned int vbn;		/* Base virtual block number.  */
  unsigned char pfc;		/* Page fault cluster.  */
  unsigned char matchctl;	/* Linker match control.  */
  unsigned char type;		/* Section type.  */
};

struct vms_internal_gbl_eisd
{
  struct vms_internal_eisd common;

  unsigned int ident;		/* Ident for global section.  */
  unsigned char gblnam[44];	/* Global name ascic.  */
};

struct vms_internal_eisd_map
{
  /* Next eisd in the list.  */
  struct vms_internal_eisd_map *next;

  /* Offset in output file.  */
  file_ptr file_pos;

  union
  {
    struct vms_internal_eisd eisd;
    struct vms_internal_gbl_eisd gbl_eisd;
  } u;
};

#endif /* _VMS_INTERNAL_H */
