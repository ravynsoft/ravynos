/* AMDGCN ELF support for BFD.

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
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* This file handles ELF files that are of the AMDGCN architecture.  The
   format is documented here:

     https://llvm.org/docs/AMDGPUUsage.html#elf-code-object */

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"
#include "elf-bfd.h"
#include "elf/amdgpu.h"

#include <string.h>

static bool
elf64_amdgcn_object_p (bfd *abfd)
{
  Elf_Internal_Ehdr *hdr = elf_elfheader (abfd);
  unsigned int mach;
  unsigned char osabi;
  unsigned char osabi_version;

  BFD_ASSERT (hdr->e_machine == EM_AMDGPU);

  osabi = hdr->e_ident[EI_OSABI];
  osabi_version = hdr->e_ident[EI_ABIVERSION];

  /* Objects with OS ABI HSA version 2 encoded the GPU model differently (in a
     note), but they are deprecated, so we don't need to support them.  Reject
     them specifically.

     At the time of writing, all AMDGCN objects encode the specific GPU
     model in the EF_AMDGPU_MACH field of e_flags.  */
  if (osabi == ELFOSABI_AMDGPU_HSA
      && osabi_version < ELFABIVERSION_AMDGPU_HSA_V3)
    return false;

  mach = elf_elfheader (abfd)->e_flags & EF_AMDGPU_MACH;

  /* Avoid matching non-AMDGCN AMDGPU objects (e.g. r600).  */
  if (mach < EF_AMDGPU_MACH_AMDGCN_MIN)
    return false;

  bfd_default_set_arch_mach (abfd, bfd_arch_amdgcn, mach);
  return true;
}


#define TARGET_LITTLE_SYM	amdgcn_elf64_le_vec
#define TARGET_LITTLE_NAME	"elf64-amdgcn"
#define ELF_ARCH		bfd_arch_amdgcn
#define ELF_TARGET_ID		AMDGCN_ELF_DATA
#define ELF_MACHINE_CODE	EM_AMDGPU
#define ELF_MAXPAGESIZE		0x10000 /* 64KB */
#define ELF_COMMONPAGESIZE	0x1000  /* 4KB */

#define bfd_elf64_bfd_reloc_type_lookup bfd_default_reloc_type_lookup
#define bfd_elf64_bfd_reloc_name_lookup _bfd_norelocs_bfd_reloc_name_lookup

#define elf_backend_object_p elf64_amdgcn_object_p

#include "elf64-target.h"
