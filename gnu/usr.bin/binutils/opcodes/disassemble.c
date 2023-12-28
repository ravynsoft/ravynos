/* Select disassembly routine for specified architecture.
   Copyright (C) 1994-2023 Free Software Foundation, Inc.

   This file is part of the GNU opcodes library.

   This library is free software; you can redistribute it and/or modify
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
#include "disassemble.h"
#include "safe-ctype.h"
#include "opintl.h"

#ifdef ARCH_all
#ifdef BFD64
#define ARCH_aarch64
#define ARCH_alpha
#define ARCH_bpf
#define ARCH_ia64
#define ARCH_loongarch
#define ARCH_mips
#define ARCH_mmix
#define ARCH_nfp
#define ARCH_riscv
#define ARCH_score
#define ARCH_tilegx
#endif
#define ARCH_arc
#define ARCH_arm
#define ARCH_avr
#define ARCH_bfin
#define ARCH_cr16
#define ARCH_cris
#define ARCH_crx
#define ARCH_csky
#define ARCH_d10v
#define ARCH_d30v
#define ARCH_dlx
#define ARCH_epiphany
#define ARCH_fr30
#define ARCH_frv
#define ARCH_ft32
#define ARCH_h8300
#define ARCH_hppa
#define ARCH_i386
#define ARCH_ip2k
#define ARCH_iq2000
#define ARCH_lm32
#define ARCH_m32c
#define ARCH_m32r
#define ARCH_m68hc11
#define ARCH_m68hc12
#define ARCH_m68k
#define ARCH_mcore
#define ARCH_mep
#define ARCH_metag
#define ARCH_microblaze
#define ARCH_mn10200
#define ARCH_mn10300
#define ARCH_moxie
#define ARCH_mt
#define ARCH_msp430
#define ARCH_nds32
#define ARCH_nios2
#define ARCH_ns32k
#define ARCH_or1k
#define ARCH_pdp11
#define ARCH_pj
#define ARCH_powerpc
#define ARCH_pru
#define ARCH_rs6000
#define ARCH_rl78
#define ARCH_rx
#define ARCH_s12z
#define ARCH_s390
#define ARCH_sh
#define ARCH_sparc
#define ARCH_spu
#define ARCH_tic30
#define ARCH_tic4x
#define ARCH_tic54x
#define ARCH_tic6x
#define ARCH_tilepro
#define ARCH_v850
#define ARCH_vax
#define ARCH_visium
#define ARCH_wasm32
#define ARCH_xstormy16
#define ARCH_xgate
#define ARCH_xtensa
#define ARCH_z80
#define ARCH_z8k
#endif

#ifdef ARCH_m32c
#include "m32c-desc.h"
#endif

#ifdef ARCH_bpf
/* XXX this should be including bpf-desc.h instead of this hackery,
   but at the moment it is not possible to include several CGEN
   generated *-desc.h files simultaneously.  To be fixed in
   CGEN...  */

# ifdef ARCH_m32c
enum epbf_isa_attr
{
  ISA_EBPFLE, ISA_EBPFBE, ISA_XBPFLE, ISA_XBPFBE, ISA_EBPFMAX
};
# else
#  include "bpf-desc.h"
#  define ISA_EBPFMAX ISA_MAX
# endif
#endif /* ARCH_bpf */

disassembler_ftype
disassembler (enum bfd_architecture a,
	      bool big ATTRIBUTE_UNUSED,
	      unsigned long mach ATTRIBUTE_UNUSED,
	      bfd *abfd ATTRIBUTE_UNUSED)
{
  disassembler_ftype disassemble;

  switch (a)
    {
      /* If you add a case to this table, also add it to the
	 ARCH_all definition right above this function.  */
#ifdef ARCH_aarch64
    case bfd_arch_aarch64:
      disassemble = print_insn_aarch64;
      break;
#endif
#ifdef ARCH_alpha
    case bfd_arch_alpha:
      disassemble = print_insn_alpha;
      break;
#endif
#ifdef ARCH_arc
    case bfd_arch_arc:
      disassemble = arc_get_disassembler (abfd);
      break;
#endif
#ifdef ARCH_arm
    case bfd_arch_arm:
      if (big)
	disassemble = print_insn_big_arm;
      else
	disassemble = print_insn_little_arm;
      break;
#endif
#ifdef ARCH_avr
    case bfd_arch_avr:
      disassemble = print_insn_avr;
      break;
#endif
#ifdef ARCH_bfin
    case bfd_arch_bfin:
      disassemble = print_insn_bfin;
      break;
#endif
#ifdef ARCH_cr16
    case bfd_arch_cr16:
      disassemble = print_insn_cr16;
      break;
#endif
#ifdef ARCH_cris
    case bfd_arch_cris:
      disassemble = cris_get_disassembler (abfd);
      break;
#endif
#ifdef ARCH_crx
    case bfd_arch_crx:
      disassemble = print_insn_crx;
      break;
#endif
#ifdef ARCH_csky
    case bfd_arch_csky:
      disassemble = csky_get_disassembler (abfd);
      break;
#endif

#ifdef ARCH_d10v
    case bfd_arch_d10v:
      disassemble = print_insn_d10v;
      break;
#endif
#ifdef ARCH_d30v
    case bfd_arch_d30v:
      disassemble = print_insn_d30v;
      break;
#endif
#ifdef ARCH_dlx
    case bfd_arch_dlx:
      /* As far as I know we only handle big-endian DLX objects.  */
      disassemble = print_insn_dlx;
      break;
#endif
#ifdef ARCH_h8300
    case bfd_arch_h8300:
      if (mach == bfd_mach_h8300h || mach == bfd_mach_h8300hn)
	disassemble = print_insn_h8300h;
      else if (mach == bfd_mach_h8300s
	       || mach == bfd_mach_h8300sn
	       || mach == bfd_mach_h8300sx
	       || mach == bfd_mach_h8300sxn)
	disassemble = print_insn_h8300s;
      else
	disassemble = print_insn_h8300;
      break;
#endif
#ifdef ARCH_hppa
    case bfd_arch_hppa:
      disassemble = print_insn_hppa;
      break;
#endif
#ifdef ARCH_i386
    case bfd_arch_i386:
    case bfd_arch_iamcu:
      disassemble = print_insn_i386;
      break;
#endif
#ifdef ARCH_ia64
    case bfd_arch_ia64:
      disassemble = print_insn_ia64;
      break;
#endif
#ifdef ARCH_ip2k
    case bfd_arch_ip2k:
      disassemble = print_insn_ip2k;
      break;
#endif
#ifdef ARCH_bpf
    case bfd_arch_bpf:
      disassemble = print_insn_bpf;
      break;
#endif
#ifdef ARCH_epiphany
    case bfd_arch_epiphany:
      disassemble = print_insn_epiphany;
      break;
#endif
#ifdef ARCH_fr30
    case bfd_arch_fr30:
      disassemble = print_insn_fr30;
      break;
#endif
#ifdef ARCH_lm32
    case bfd_arch_lm32:
      disassemble = print_insn_lm32;
      break;
#endif
#ifdef ARCH_m32r
    case bfd_arch_m32r:
      disassemble = print_insn_m32r;
      break;
#endif
#if defined(ARCH_m68hc11) || defined(ARCH_m68hc12) \
    || defined(ARCH_9s12x) || defined(ARCH_m9s12xg)
    case bfd_arch_m68hc11:
      disassemble = print_insn_m68hc11;
      break;
    case bfd_arch_m68hc12:
      disassemble = print_insn_m68hc12;
      break;
    case bfd_arch_m9s12x:
      disassemble = print_insn_m9s12x;
      break;
    case bfd_arch_m9s12xg:
      disassemble = print_insn_m9s12xg;
      break;
#endif
#if defined(ARCH_s12z)
    case bfd_arch_s12z:
      disassemble = print_insn_s12z;
      break;
#endif
#ifdef ARCH_m68k
    case bfd_arch_m68k:
      disassemble = print_insn_m68k;
      break;
#endif
#ifdef ARCH_mt
    case bfd_arch_mt:
      disassemble = print_insn_mt;
      break;
#endif
#ifdef ARCH_microblaze
    case bfd_arch_microblaze:
      disassemble = print_insn_microblaze;
      break;
#endif
#ifdef ARCH_msp430
    case bfd_arch_msp430:
      disassemble = print_insn_msp430;
      break;
#endif
#ifdef ARCH_nds32
    case bfd_arch_nds32:
      disassemble = print_insn_nds32;
      break;
#endif
#ifdef ARCH_nfp
    case bfd_arch_nfp:
      disassemble = print_insn_nfp;
      break;
#endif
#ifdef ARCH_ns32k
    case bfd_arch_ns32k:
      disassemble = print_insn_ns32k;
      break;
#endif
#ifdef ARCH_mcore
    case bfd_arch_mcore:
      disassemble = print_insn_mcore;
      break;
#endif
#ifdef ARCH_mep
    case bfd_arch_mep:
      disassemble = print_insn_mep;
      break;
#endif
#ifdef ARCH_metag
    case bfd_arch_metag:
      disassemble = print_insn_metag;
      break;
#endif
#ifdef ARCH_mips
    case bfd_arch_mips:
      if (big)
	disassemble = print_insn_big_mips;
      else
	disassemble = print_insn_little_mips;
      break;
#endif
#ifdef ARCH_mmix
    case bfd_arch_mmix:
      disassemble = print_insn_mmix;
      break;
#endif
#ifdef ARCH_mn10200
    case bfd_arch_mn10200:
      disassemble = print_insn_mn10200;
      break;
#endif
#ifdef ARCH_mn10300
    case bfd_arch_mn10300:
      disassemble = print_insn_mn10300;
      break;
#endif
#ifdef ARCH_nios2
    case bfd_arch_nios2:
      if (big)
	disassemble = print_insn_big_nios2;
      else
	disassemble = print_insn_little_nios2;
      break;
#endif
#ifdef ARCH_or1k
    case bfd_arch_or1k:
      disassemble = print_insn_or1k;
      break;
#endif
#ifdef ARCH_pdp11
    case bfd_arch_pdp11:
      disassemble = print_insn_pdp11;
      break;
#endif
#ifdef ARCH_pj
    case bfd_arch_pj:
      disassemble = print_insn_pj;
      break;
#endif
#ifdef ARCH_powerpc
    case bfd_arch_powerpc:
#endif
#ifdef ARCH_rs6000
    case bfd_arch_rs6000:
#endif
#if defined ARCH_powerpc || defined ARCH_rs6000
      if (big)
	disassemble = print_insn_big_powerpc;
      else
	disassemble = print_insn_little_powerpc;
      break;
#endif
#ifdef ARCH_pru
    case bfd_arch_pru:
      disassemble = print_insn_pru;
      break;
#endif
#ifdef ARCH_riscv
    case bfd_arch_riscv:
      disassemble = riscv_get_disassembler (abfd);
      break;
#endif
#ifdef ARCH_rl78
    case bfd_arch_rl78:
      disassemble = rl78_get_disassembler (abfd);
      break;
#endif
#ifdef ARCH_rx
    case bfd_arch_rx:
      disassemble = print_insn_rx;
      break;
#endif
#ifdef ARCH_s390
    case bfd_arch_s390:
      disassemble = print_insn_s390;
      break;
#endif
#ifdef ARCH_score
    case bfd_arch_score:
      if (big)
	disassemble = print_insn_big_score;
      else
	disassemble = print_insn_little_score;
     break;
#endif
#ifdef ARCH_sh
    case bfd_arch_sh:
      disassemble = print_insn_sh;
      break;
#endif
#ifdef ARCH_sparc
    case bfd_arch_sparc:
      disassemble = print_insn_sparc;
      break;
#endif
#ifdef ARCH_spu
    case bfd_arch_spu:
      disassemble = print_insn_spu;
      break;
#endif
#ifdef ARCH_tic30
    case bfd_arch_tic30:
      disassemble = print_insn_tic30;
      break;
#endif
#ifdef ARCH_tic4x
    case bfd_arch_tic4x:
      disassemble = print_insn_tic4x;
      break;
#endif
#ifdef ARCH_tic54x
    case bfd_arch_tic54x:
      disassemble = print_insn_tic54x;
      break;
#endif
#ifdef ARCH_tic6x
    case bfd_arch_tic6x:
      disassemble = print_insn_tic6x;
      break;
#endif
#ifdef ARCH_ft32
    case bfd_arch_ft32:
      disassemble = print_insn_ft32;
      break;
#endif
#ifdef ARCH_v850
    case bfd_arch_v850:
    case bfd_arch_v850_rh850:
      disassemble = print_insn_v850;
      break;
#endif
#ifdef ARCH_wasm32
    case bfd_arch_wasm32:
      disassemble = print_insn_wasm32;
      break;
#endif
#ifdef ARCH_xgate
    case bfd_arch_xgate:
      disassemble = print_insn_xgate;
      break;
#endif
#ifdef ARCH_xstormy16
    case bfd_arch_xstormy16:
      disassemble = print_insn_xstormy16;
      break;
#endif
#ifdef ARCH_xtensa
    case bfd_arch_xtensa:
      disassemble = print_insn_xtensa;
      break;
#endif
#ifdef ARCH_z80
    case bfd_arch_z80:
      disassemble = print_insn_z80;
      break;
#endif
#ifdef ARCH_z8k
    case bfd_arch_z8k:
      if (mach == bfd_mach_z8001)
	disassemble = print_insn_z8001;
      else
	disassemble = print_insn_z8002;
      break;
#endif
#ifdef ARCH_vax
    case bfd_arch_vax:
      disassemble = print_insn_vax;
      break;
#endif
#ifdef ARCH_visium
     case bfd_arch_visium:
       disassemble = print_insn_visium;
       break;
#endif
#ifdef ARCH_frv
    case bfd_arch_frv:
      disassemble = print_insn_frv;
      break;
#endif
#ifdef ARCH_moxie
    case bfd_arch_moxie:
      disassemble = print_insn_moxie;
      break;
#endif
#ifdef ARCH_iq2000
    case bfd_arch_iq2000:
      disassemble = print_insn_iq2000;
      break;
#endif
#ifdef ARCH_m32c
    case bfd_arch_m32c:
      disassemble = print_insn_m32c;
      break;
#endif
#ifdef ARCH_tilegx
    case bfd_arch_tilegx:
      disassemble = print_insn_tilegx;
      break;
#endif
#ifdef ARCH_tilepro
    case bfd_arch_tilepro:
      disassemble = print_insn_tilepro;
      break;
#endif
#ifdef ARCH_loongarch
    case bfd_arch_loongarch:
      disassemble = print_insn_loongarch;
      break;
#endif
    default:
      return 0;
    }
  return disassemble;
}

void
disassembler_usage (FILE *stream ATTRIBUTE_UNUSED)
{
#ifdef ARCH_aarch64
  print_aarch64_disassembler_options (stream);
#endif
#ifdef ARCH_arc
  print_arc_disassembler_options (stream);
#endif
#ifdef ARCH_arm
  print_arm_disassembler_options (stream);
#endif
#ifdef ARCH_mips
  print_mips_disassembler_options (stream);
#endif
#ifdef ARCH_nfp
  print_nfp_disassembler_options (stream);
#endif
#ifdef ARCH_powerpc
  print_ppc_disassembler_options (stream);
#endif
#ifdef ARCH_riscv
  print_riscv_disassembler_options (stream);
#endif
#ifdef ARCH_i386
  print_i386_disassembler_options (stream);
#endif
#ifdef ARCH_s390
  print_s390_disassembler_options (stream);
#endif
#ifdef ARCH_wasm32
  print_wasm32_disassembler_options (stream);
#endif
#ifdef ARCH_loongarch
  print_loongarch_disassembler_options (stream);
#endif

  return;
}

void
disassemble_init_for_target (struct disassemble_info * info)
{
  if (info == NULL)
    return;

  switch (info->arch)
    {
#ifdef ARCH_aarch64
    case bfd_arch_aarch64:
      info->symbol_is_valid = aarch64_symbol_is_valid;
      info->disassembler_needs_relocs = true;
      info->created_styled_output = true;
      break;
#endif
#ifdef ARCH_arc
    case bfd_arch_arc:
      info->created_styled_output = true;
      break;
#endif
#ifdef ARCH_arm
    case bfd_arch_arm:
      info->symbol_is_valid = arm_symbol_is_valid;
      info->disassembler_needs_relocs = true;
      info->created_styled_output = true;
      break;
#endif
#ifdef ARCH_avr
    case bfd_arch_avr:
      info->created_styled_output = true;
      break;
#endif
#ifdef ARCH_csky
    case bfd_arch_csky:
      info->symbol_is_valid = csky_symbol_is_valid;
      info->disassembler_needs_relocs = true;
      break;
#endif
#ifdef ARCH_i386
    case bfd_arch_i386:
    case bfd_arch_iamcu:
      info->created_styled_output = true;
      break;
#endif
#ifdef ARCH_ia64
    case bfd_arch_ia64:
      info->skip_zeroes = 16;
      break;
#endif
#ifdef ARCH_loongarch
    case bfd_arch_loongarch:
      info->created_styled_output = true;
      break;
#endif
#ifdef ARCH_tic4x
    case bfd_arch_tic4x:
      info->skip_zeroes = 32;
      break;
#endif
#ifdef ARCH_m68k
    case bfd_arch_m68k:
      info->created_styled_output = true;
      break;
#endif
#ifdef ARCH_mep
    case bfd_arch_mep:
      info->skip_zeroes = 256;
      info->skip_zeroes_at_end = 0;
      break;
#endif
#ifdef ARCH_metag
    case bfd_arch_metag:
      info->disassembler_needs_relocs = true;
      break;
#endif
#ifdef ARCH_mips
    case bfd_arch_mips:
      info->created_styled_output = true;
      break;
#endif
#ifdef ARCH_m32c
    case bfd_arch_m32c:
      /* This processor in fact is little endian.  The value set here
	 reflects the way opcodes are written in the cgen description.  */
      info->endian = BFD_ENDIAN_BIG;
      if (!info->private_data)
	{
	  info->private_data = cgen_bitset_create (ISA_MAX);
	  if (info->mach == bfd_mach_m16c)
	    cgen_bitset_set (info->private_data, ISA_M16C);
	  else
	    cgen_bitset_set (info->private_data, ISA_M32C);
	}
      break;
#endif
#ifdef ARCH_bpf
    case bfd_arch_bpf:
      info->endian_code = BFD_ENDIAN_LITTLE;
      if (!info->private_data)
	{
	  info->private_data = cgen_bitset_create (ISA_MAX);
	  if (info->endian == BFD_ENDIAN_BIG)
	    {
	      cgen_bitset_set (info->private_data, ISA_EBPFBE);
	      if (info->mach == bfd_mach_xbpf)
		cgen_bitset_set (info->private_data, ISA_XBPFBE);
	    }
	  else
	    {
	      cgen_bitset_set (info->private_data, ISA_EBPFLE);
	      if (info->mach == bfd_mach_xbpf)
		cgen_bitset_set (info->private_data, ISA_XBPFLE);
	    }
	}
      break;
#endif
#ifdef ARCH_pru
    case bfd_arch_pru:
      info->disassembler_needs_relocs = true;
      break;
#endif
#ifdef ARCH_powerpc
    case bfd_arch_powerpc:
#endif
#ifdef ARCH_rs6000
    case bfd_arch_rs6000:
#endif
#if defined (ARCH_powerpc) || defined (ARCH_rs6000)
      disassemble_init_powerpc (info);
      info->created_styled_output = true;
      break;
#endif
#ifdef ARCH_riscv
    case bfd_arch_riscv:
      info->symbol_is_valid = riscv_symbol_is_valid;
      info->created_styled_output = true;
      break;
#endif
#ifdef ARCH_wasm32
    case bfd_arch_wasm32:
      disassemble_init_wasm32 (info);
      break;
#endif
#ifdef ARCH_s390
    case bfd_arch_s390:
      disassemble_init_s390 (info);
      info->created_styled_output = true;
      break;
#endif
#ifdef ARCH_nds32
    case bfd_arch_nds32:
      disassemble_init_nds32 (info);
      break;
 #endif
    default:
      break;
    }
}

void
disassemble_free_target (struct disassemble_info *info)
{
  if (info == NULL)
    return;

  switch (info->arch)
    {
    default:
      return;

#ifdef ARCH_bpf
    case bfd_arch_bpf:
#endif
#ifdef ARCH_m32c
    case bfd_arch_m32c:
#endif
#if defined ARCH_bpf || defined ARCH_m32c
      if (info->private_data)
	{
	  CGEN_BITSET *mask = info->private_data;
	  free (mask->bits);
	}
      break;
#endif

#ifdef ARCH_arc
    case bfd_arch_arc:
      break;
#endif
#ifdef ARCH_cris
    case bfd_arch_cris:
      break;
#endif
#ifdef ARCH_mmix
    case bfd_arch_mmix:
      break;
#endif
#ifdef ARCH_nfp
    case bfd_arch_nfp:
      break;
#endif
#ifdef ARCH_powerpc
    case bfd_arch_powerpc:
      break;
#endif
#ifdef ARCH_riscv
    case bfd_arch_riscv:
      disassemble_free_riscv (info);
      break;
#endif
#ifdef ARCH_rs6000
    case bfd_arch_rs6000:
      break;
#endif
    }

  free (info->private_data);
}

/* Remove whitespace and consecutive commas from OPTIONS.  */

char *
remove_whitespace_and_extra_commas (char *options)
{
  char *str;
  size_t i, len;

  if (options == NULL)
    return NULL;

  /* Strip off all trailing whitespace and commas.  */
  for (len = strlen (options); len > 0; len--)
    {
      if (!ISSPACE (options[len - 1]) && options[len - 1] != ',')
	break;
      options[len - 1] = '\0';
    }

  /* Convert all remaining whitespace to commas.  */
  for (i = 0; options[i] != '\0'; i++)
    if (ISSPACE (options[i]))
      options[i] = ',';

  /* Remove consecutive commas.  */
  for (str = options; *str != '\0'; str++)
    if (*str == ',' && (*(str + 1) == ',' || str == options))
      {
	char *next = str + 1;
	while (*next == ',')
	  next++;
	len = strlen (next);
	if (str != options)
	  str++;
	memmove (str, next, len);
	next[len - (size_t)(next - str)] = '\0';
      }
  return (strlen (options) != 0) ? options : NULL;
}

/* Like STRCMP, but treat ',' the same as '\0' so that we match
   strings like "foobar" against "foobar,xxyyzz,...".  */

int
disassembler_options_cmp (const char *s1, const char *s2)
{
  unsigned char c1, c2;

  do
    {
      c1 = (unsigned char) *s1++;
      if (c1 == ',')
	c1 = '\0';
      c2 = (unsigned char) *s2++;
      if (c2 == ',')
	c2 = '\0';
      if (c1 == '\0')
	return c1 - c2;
    }
  while (c1 == c2);

  return c1 - c2;
}

void
opcodes_assert (const char *file, int line)
{
  opcodes_error_handler (_("assertion fail %s:%d"), file, line);
  opcodes_error_handler (_("Please report this bug"));
  abort ();
}

/* Set the stream, and the styled and unstyled printf functions within
   INFO.  */

void
disassemble_set_printf (struct disassemble_info *info, void *stream,
			fprintf_ftype unstyled_printf,
			fprintf_styled_ftype styled_printf)
{
  info->stream = stream;
  info->fprintf_func = unstyled_printf;
  info->fprintf_styled_func = styled_printf;
}
