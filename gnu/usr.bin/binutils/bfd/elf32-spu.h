/* SPU specific support for 32-bit ELF.

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

struct spu_elf_params
{
  /* Stash various callbacks for --auto-overlay.  */
  void (*place_spu_section) (asection *, asection *, const char *);
  bfd_size_type (*spu_elf_load_ovl_mgr) (void);
  FILE *(*spu_elf_open_overlay_script) (void);
  void (*spu_elf_relink) (void);

  /* Bit 0 set if --auto-overlay.
     Bit 1 set if --auto-relink.
     Bit 2 set if --overlay-rodata.  */
  unsigned int auto_overlay : 3;
#define AUTO_OVERLAY 1
#define AUTO_RELINK 2
#define OVERLAY_RODATA 4

  /* Type of overlays, enum _ovly_flavour.  */
  unsigned int ovly_flavour : 1;
  unsigned int compact_stub : 1;

  /* Set if we should emit symbols for stubs.  */
  unsigned int emit_stub_syms : 1;

  /* Set if we want stubs on calls out of overlay regions to
     non-overlay regions.  */
  unsigned int non_overlay_stubs : 1;

  /* Set if lr liveness analysis should be done.  */
  unsigned int lrlive_analysis : 1;

  /* Set if stack size analysis should be done.  */
  unsigned int stack_analysis : 1;

  /* Set if __stack_* syms will be emitted.  */
  unsigned int emit_stack_syms : 1;

  /* Set if non-icache code should be allowed in icache lines.  */
  unsigned int non_ia_text : 1;

  /* Set when the .fixup section should be generated. */
  unsigned int emit_fixups : 1;

  /* Range of valid addresses for loadable sections.  */
  bfd_vma local_store_lo;
  bfd_vma local_store_hi;

  /* Control --auto-overlay feature.  */
  unsigned int num_lines;
  unsigned int line_size;
  unsigned int max_branch;
  unsigned int auto_overlay_fixed;
  unsigned int auto_overlay_reserved;
  int extra_stack_space;
};

/* Extra info kept for SPU sections.  */

struct spu_elf_stack_info;

struct _spu_elf_section_data
{
  struct bfd_elf_section_data elf;

  union {
    /* Info kept for input sections.  */
    struct {
      /* Stack analysis info kept for this section.  */
      struct spu_elf_stack_info *stack_info;
    } i;

    /* Info kept for output sections.  */
    struct {
      /* Non-zero for overlay output sections.  */
      unsigned int ovl_index;
      unsigned int ovl_buf;
    } o;
  } u;
};

#define spu_elf_section_data(sec) \
  ((struct _spu_elf_section_data *) elf_section_data (sec))

enum _ovly_flavour
{
  ovly_normal,
  ovly_soft_icache
};

struct _ovl_stream
{
  const void *start;
  const void *end;
};

extern void spu_elf_setup (struct bfd_link_info *, struct spu_elf_params *);
extern void spu_elf_plugin (int);
extern bool spu_elf_open_builtin_lib (bfd **, const struct _ovl_stream *);
extern bool spu_elf_create_sections (struct bfd_link_info *);
extern bool spu_elf_size_sections (bfd *, struct bfd_link_info *);
extern int spu_elf_find_overlays (struct bfd_link_info *);
extern int spu_elf_size_stubs (struct bfd_link_info *);
extern void spu_elf_place_overlay_data (struct bfd_link_info *);
extern asection *spu_elf_check_vma (struct bfd_link_info *);
