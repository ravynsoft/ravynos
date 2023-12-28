/* 32-bit ELF support for ARM
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


/* ARM VFP11 erratum workaround support.  */
typedef enum
{
  BFD_ARM_VFP11_FIX_DEFAULT,
  BFD_ARM_VFP11_FIX_NONE,
  BFD_ARM_VFP11_FIX_SCALAR,
  BFD_ARM_VFP11_FIX_VECTOR
} bfd_arm_vfp11_fix;

extern void bfd_elf32_arm_init_maps
  (bfd *);

extern void bfd_elf32_arm_set_vfp11_fix
  (bfd *, struct bfd_link_info *);

extern void bfd_elf32_arm_set_cortex_a8_fix
  (bfd *, struct bfd_link_info *);

extern bool bfd_elf32_arm_vfp11_erratum_scan
  (bfd *, struct bfd_link_info *);

extern void bfd_elf32_arm_vfp11_fix_veneer_locations
  (bfd *, struct bfd_link_info *);

/* ARM STM STM32L4XX erratum workaround support.  */
typedef enum
{
  BFD_ARM_STM32L4XX_FIX_NONE,
  BFD_ARM_STM32L4XX_FIX_DEFAULT,
  BFD_ARM_STM32L4XX_FIX_ALL
} bfd_arm_stm32l4xx_fix;

extern void bfd_elf32_arm_set_stm32l4xx_fix
  (bfd *, struct bfd_link_info *);

extern bool bfd_elf32_arm_stm32l4xx_erratum_scan
  (bfd *, struct bfd_link_info *);

extern void bfd_elf32_arm_stm32l4xx_fix_veneer_locations
  (bfd *, struct bfd_link_info *);

/* ELF ARM Interworking support.  Called from linker.  */
extern bool bfd_elf32_arm_allocate_interworking_sections
  (struct bfd_link_info *);

extern bool bfd_elf32_arm_process_before_allocation
  (bfd *, struct bfd_link_info *);

struct elf32_arm_params {
  char *thumb_entry_symbol;
  int byteswap_code;
  int target1_is_rel;
  char * target2_type;
  int fix_v4bx;
  int use_blx;
  bfd_arm_vfp11_fix vfp11_denorm_fix;
  bfd_arm_stm32l4xx_fix stm32l4xx_fix;
  int no_enum_size_warning;
  int no_wchar_size_warning;
  int pic_veneer;
  int fix_cortex_a8;
  int fix_arm1176;
  int merge_exidx_entries;
  int cmse_implib;
  bfd *in_implib_bfd;
};

void bfd_elf32_arm_set_target_params
  (bfd *, struct bfd_link_info *, struct elf32_arm_params *);

extern bool bfd_elf32_arm_get_bfd_for_interworking
  (bfd *, struct bfd_link_info *);

extern bool bfd_elf32_arm_add_glue_sections_to_bfd
  (bfd *, struct bfd_link_info *);

extern void bfd_elf32_arm_keep_private_stub_output_sections
  (struct bfd_link_info *);

extern void bfd_elf32_arm_set_byteswap_code
  (struct bfd_link_info *, int);

extern void bfd_elf32_arm_use_long_plt (void);

/* ARM stub generation support.  Called from the linker.  */
extern int elf32_arm_setup_section_lists
  (bfd *, struct bfd_link_info *);
extern void elf32_arm_next_input_section
  (struct bfd_link_info *, struct bfd_section *);
extern bool elf32_arm_size_stubs
  (bfd *, bfd *, struct bfd_link_info *, bfd_signed_vma,
   struct bfd_section * (*) (const char *, struct bfd_section *,
			     struct bfd_section *, unsigned int),
   void (*) (void));
extern bool elf32_arm_build_stubs
  (struct bfd_link_info *);

/* ARM unwind section editing support.  */
extern bool elf32_arm_fix_exidx_coverage
(struct bfd_section **, unsigned int, struct bfd_link_info *, bool);
