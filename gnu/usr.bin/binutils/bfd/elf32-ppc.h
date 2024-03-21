/* PowerPC-specific support for 64-bit ELF.
   Copyright (C) 2003-2023 Free Software Foundation, Inc.

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

enum ppc_elf_plt_type
{
  PLT_UNSET,
  PLT_OLD,
  PLT_NEW,
  PLT_VXWORKS
};

/* Various options passed from the linker to bfd backend.  */
struct ppc_elf_params
{
  /* Chooses the type of .plt.  */
  enum ppc_elf_plt_type plt_style;

  /* Set if individual PLT call stubs should be aligned.  */
  int plt_stub_align;

  /* Whether to emit symbols for stubs.  */
  int emit_stub_syms;

  /* Whether to emit special stub for __tls_get_addr calls.  */
  int no_tls_get_addr_opt;

  /* Insert trampolines for branches that won't reach their destination.  */
  int branch_trampolines;

  /* Avoid execution falling into new page.  */
  int ppc476_workaround;
  unsigned int pagesize_p2;

  /* The bfd backend detected a non-PIC reference to a protected symbol
     defined in a shared library.  */
  int pic_fixup;

  /* Relocate 16A relocs as 16D and vice versa.  */
  int vle_reloc_fixup;

  bfd_vma pagesize;
};

void ppc_elf_link_params (struct bfd_link_info *, struct ppc_elf_params *);
int ppc_elf_select_plt_layout (bfd *, struct bfd_link_info *);
bool ppc_elf_inline_plt (struct bfd_link_info *);
asection *ppc_elf_tls_setup (bfd *, struct bfd_link_info *);
bool ppc_elf_tls_optimize (bfd *, struct bfd_link_info *);
void ppc_elf_maybe_strip_sdata_syms (struct bfd_link_info *);
extern bool ppc_elf_modify_segment_map (bfd *, struct bfd_link_info *);
extern bool ppc_elf_section_processing (bfd *, Elf_Internal_Shdr *);
extern bool ppc_finish_symbols (struct bfd_link_info *);
