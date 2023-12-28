/* LoongArch-specific backend routines.
   Copyright (C) 2021-2023 Free Software Foundation, Inc.
   Contributed by Loongson Ltd.

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
   along with this program; see the file COPYING3.  If not,
   see <http://www.gnu.org/licenses/>.  */

#include "elf/common.h"
#include "elf/internal.h"

extern reloc_howto_type *
loongarch_elf_rtype_to_howto (bfd *abfd, unsigned int r_type);

extern reloc_howto_type *
loongarch_reloc_type_lookup (bfd *abfd, bfd_reloc_code_real_type code);

extern reloc_howto_type *
loongarch_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED, const char *r_name);

extern bfd_reloc_code_real_type
loongarch_larch_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED,
				   const char *l_r_name);

bool
loongarch_adjust_reloc_bitsfield (bfd *, reloc_howto_type *, bfd_vma *);
void
bfd_elf32_loongarch_set_data_segment_info (struct bfd_link_info *, int *);
void
bfd_elf64_loongarch_set_data_segment_info (struct bfd_link_info *, int *);

bfd_byte *
loongarch_write_unsigned_leb128 (bfd_byte *p, unsigned int len, bfd_vma value);

int loongarch_get_uleb128_length (bfd_byte *buf);

/* TRUE if this is a PLT reference to a local IFUNC.  */
#define PLT_LOCAL_IFUNC_P(INFO, H) \
  ((H)->dynindx == -1 \
   || ((bfd_link_executable (INFO) \
	|| ELF_ST_VISIBILITY ((H)->other) != STV_DEFAULT) \
	&& (H)->def_regular \
	&& (H)->type == STT_GNU_IFUNC))
