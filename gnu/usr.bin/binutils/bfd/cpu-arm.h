/* BFD support for the ARM processor
   Copyright (C) 2019-2023 Free Software Foundation, Inc.

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

/* ARM Note section processing.  */
extern bool bfd_arm_merge_machines
  (bfd *, bfd *);

extern bool bfd_arm_update_notes
  (bfd *, const char *);

extern unsigned int bfd_arm_get_mach_from_notes
  (bfd *, const char *);

/* ELF ARM mapping symbol support.  */
#define BFD_ARM_SPECIAL_SYM_TYPE_MAP	(1 << 0)
#define BFD_ARM_SPECIAL_SYM_TYPE_TAG	(1 << 1)
#define BFD_ARM_SPECIAL_SYM_TYPE_OTHER	(1 << 2)
#define BFD_ARM_SPECIAL_SYM_TYPE_ANY	(~0)

extern bool bfd_is_arm_special_symbol_name
  (const char *, int);

