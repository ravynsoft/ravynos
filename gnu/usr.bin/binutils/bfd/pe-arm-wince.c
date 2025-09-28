/* BFD back-end for ARM WINCE PE files.
   Copyright (C) 2006-2023 Free Software Foundation, Inc.

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

#include "sysdep.h"
#include "bfd.h"

#define TARGET_UNDERSCORE    0
#define USER_LABEL_PREFIX    ""

#define TARGET_LITTLE_SYM    arm_pe_wince_le_vec
#define TARGET_LITTLE_NAME   "pe-arm-wince-little"
#define TARGET_BIG_SYM       arm_pe_wince_be_vec
#define TARGET_BIG_NAME      "pe-arm-wince-big"

#define LOCAL_LABEL_PREFIX "."

#undef  bfd_pe_print_pdata
#define	bfd_pe_print_pdata   _bfd_pe_print_ce_compressed_pdata

#define bfd_arm_allocate_interworking_sections \
  bfd_arm_wince_pe_allocate_interworking_sections
#define bfd_arm_get_bfd_for_interworking \
  bfd_arm_wince_pe_get_bfd_for_interworking
#define bfd_arm_process_before_allocation \
  bfd_arm_wince_pe_process_before_allocation

#include "pe-arm.c"
