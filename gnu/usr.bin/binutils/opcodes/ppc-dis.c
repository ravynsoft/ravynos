/* ppc-dis.c -- Disassemble PowerPC instructions
   Copyright (C) 1994-2023 Free Software Foundation, Inc.
   Written by Ian Lance Taylor, Cygnus Support

   This file is part of the GNU opcodes library.

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   It is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with this file; see the file COPYING.  If not, write to the
   Free Software Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "sysdep.h"
#include <stdio.h>
#include "disassemble.h"
#include "elf-bfd.h"
#include "elf/ppc.h"
#include "opintl.h"
#include "opcode/ppc.h"
#include "libiberty.h"

/* This file provides several disassembler functions, all of which use
   the disassembler interface defined in dis-asm.h.  Several functions
   are provided because this file handles disassembly for the PowerPC
   in both big and little endian mode and also for the POWER (RS/6000)
   chip.  */
static int print_insn_powerpc (bfd_vma, struct disassemble_info *, int,
			       ppc_cpu_t);

struct dis_private
{
  /* Stash the result of parsing disassembler_options here.  */
  ppc_cpu_t dialect;

  /* .got and .plt sections.  NAME is set to NULL if not present.  */
  struct sec_buf {
    asection *sec;
    bfd_byte *buf;
    const char *name;
  } special[2];
};

static inline struct dis_private *
private_data (struct disassemble_info *info)
{
  return (struct dis_private *) info->private_data;
}

struct ppc_mopt {
  /* Option string, without -m or -M prefix.  */
  const char *opt;
  /* CPU option flags.  */
  ppc_cpu_t cpu;
  /* Flags that should stay on, even when combined with another cpu
     option.  This should only be used for generic options like
     "-many" or "-maltivec" where it is reasonable to add some
     capability to another cpu selection.  The added flags are sticky
     so that, for example, "-many -me500" and "-me500 -many" result in
     the same assembler or disassembler behaviour.  Do not use
     "sticky" for specific cpus, as this will prevent that cpu's flags
     from overriding the defaults set in powerpc_init_dialect or a
     prior -m option.  */
  ppc_cpu_t sticky;
};

struct ppc_mopt ppc_opts[] = {
  { "403",     PPC_OPCODE_PPC | PPC_OPCODE_403,
    0 },
  { "405",     PPC_OPCODE_PPC | PPC_OPCODE_403 | PPC_OPCODE_405,
    0 },
  { "440",     (PPC_OPCODE_PPC | PPC_OPCODE_BOOKE | PPC_OPCODE_440
		| PPC_OPCODE_ISEL | PPC_OPCODE_RFMCI),
    0 },
  { "464",     (PPC_OPCODE_PPC | PPC_OPCODE_BOOKE | PPC_OPCODE_440
		| PPC_OPCODE_ISEL | PPC_OPCODE_RFMCI),
    0 },
  { "476",     (PPC_OPCODE_PPC | PPC_OPCODE_ISEL | PPC_OPCODE_476
		| PPC_OPCODE_POWER4 | PPC_OPCODE_POWER5),
    0 },
  { "601",     PPC_OPCODE_PPC | PPC_OPCODE_601,
    0 },
  { "603",     PPC_OPCODE_PPC,
    0 },
  { "604",     PPC_OPCODE_PPC,
    0 },
  { "620",     PPC_OPCODE_PPC | PPC_OPCODE_64,
    0 },
  { "7400",    PPC_OPCODE_PPC | PPC_OPCODE_ALTIVEC,
    0 },
  { "7410",    PPC_OPCODE_PPC | PPC_OPCODE_ALTIVEC,
    0 },
  { "7450",    PPC_OPCODE_PPC | PPC_OPCODE_7450 | PPC_OPCODE_ALTIVEC,
    0 },
  { "7455",    PPC_OPCODE_PPC | PPC_OPCODE_ALTIVEC,
    0 },
  { "750cl",   PPC_OPCODE_PPC | PPC_OPCODE_750 | PPC_OPCODE_PPCPS
    , 0 },
  { "gekko",   PPC_OPCODE_PPC | PPC_OPCODE_750 | PPC_OPCODE_PPCPS
    , 0 },
  { "broadway", PPC_OPCODE_PPC | PPC_OPCODE_750 | PPC_OPCODE_PPCPS
    , 0 },
  { "821",     PPC_OPCODE_PPC | PPC_OPCODE_860,
    0 },
  { "850",     PPC_OPCODE_PPC | PPC_OPCODE_860,
    0 },
  { "860",     PPC_OPCODE_PPC | PPC_OPCODE_860,
    0 },
  { "a2",      (PPC_OPCODE_PPC | PPC_OPCODE_ISEL | PPC_OPCODE_POWER4
		| PPC_OPCODE_POWER5 | PPC_OPCODE_CACHELCK | PPC_OPCODE_64
		| PPC_OPCODE_A2),
    0 },
  { "altivec", PPC_OPCODE_PPC,
    PPC_OPCODE_ALTIVEC },
  { "any",     PPC_OPCODE_PPC,
    PPC_OPCODE_ANY },
  { "booke",   PPC_OPCODE_PPC | PPC_OPCODE_BOOKE,
    0 },
  { "booke32", PPC_OPCODE_PPC | PPC_OPCODE_BOOKE,
    0 },
  { "cell",    (PPC_OPCODE_PPC | PPC_OPCODE_64 | PPC_OPCODE_POWER4
		| PPC_OPCODE_CELL | PPC_OPCODE_ALTIVEC),
    0 },
  { "com",     PPC_OPCODE_COMMON,
    0 },
  { "e200z2",  (PPC_OPCODE_PPC | PPC_OPCODE_BOOKE | PPC_OPCODE_LSP
		| PPC_OPCODE_ISEL | PPC_OPCODE_EFS | PPC_OPCODE_BRLOCK
		| PPC_OPCODE_PMR | PPC_OPCODE_CACHELCK | PPC_OPCODE_RFMCI
		| PPC_OPCODE_E500 | PPC_OPCODE_VLE | PPC_OPCODE_E200Z4
		| PPC_OPCODE_EFS2),
    0 },
  { "e200z4",  (PPC_OPCODE_PPC | PPC_OPCODE_BOOKE | PPC_OPCODE_SPE
		| PPC_OPCODE_ISEL | PPC_OPCODE_EFS | PPC_OPCODE_BRLOCK
		| PPC_OPCODE_PMR | PPC_OPCODE_CACHELCK | PPC_OPCODE_RFMCI
		| PPC_OPCODE_E500 | PPC_OPCODE_VLE | PPC_OPCODE_E200Z4
		| PPC_OPCODE_EFS2),
    0 },
  { "e300",    PPC_OPCODE_PPC | PPC_OPCODE_E300,
    0 },
  { "e500",    (PPC_OPCODE_PPC | PPC_OPCODE_BOOKE | PPC_OPCODE_SPE
		| PPC_OPCODE_ISEL | PPC_OPCODE_EFS | PPC_OPCODE_BRLOCK
		| PPC_OPCODE_PMR | PPC_OPCODE_CACHELCK | PPC_OPCODE_RFMCI
		| PPC_OPCODE_E500),
    0 },
  { "e500mc",  (PPC_OPCODE_PPC | PPC_OPCODE_BOOKE | PPC_OPCODE_ISEL
		| PPC_OPCODE_PMR | PPC_OPCODE_CACHELCK | PPC_OPCODE_RFMCI
		| PPC_OPCODE_E500MC),
    0 },
  { "e500mc64",  (PPC_OPCODE_PPC | PPC_OPCODE_BOOKE | PPC_OPCODE_ISEL
		| PPC_OPCODE_PMR | PPC_OPCODE_CACHELCK | PPC_OPCODE_RFMCI
		| PPC_OPCODE_E500MC | PPC_OPCODE_64 | PPC_OPCODE_POWER5
		| PPC_OPCODE_POWER6 | PPC_OPCODE_POWER7),
    0 },
  { "e5500",    (PPC_OPCODE_PPC | PPC_OPCODE_BOOKE | PPC_OPCODE_ISEL
		| PPC_OPCODE_PMR | PPC_OPCODE_CACHELCK | PPC_OPCODE_RFMCI
		| PPC_OPCODE_E500MC | PPC_OPCODE_64 | PPC_OPCODE_POWER4
		| PPC_OPCODE_POWER5 | PPC_OPCODE_POWER6 | PPC_OPCODE_POWER7),
    0 },
  { "e6500",   (PPC_OPCODE_PPC | PPC_OPCODE_BOOKE | PPC_OPCODE_ISEL
		| PPC_OPCODE_PMR | PPC_OPCODE_CACHELCK | PPC_OPCODE_RFMCI
		| PPC_OPCODE_E500MC | PPC_OPCODE_64 | PPC_OPCODE_ALTIVEC
		| PPC_OPCODE_E6500 | PPC_OPCODE_TMR | PPC_OPCODE_POWER4
		| PPC_OPCODE_POWER5 | PPC_OPCODE_POWER6 | PPC_OPCODE_POWER7),
    0 },
  { "e500x2",  (PPC_OPCODE_PPC | PPC_OPCODE_BOOKE | PPC_OPCODE_SPE
		| PPC_OPCODE_ISEL | PPC_OPCODE_EFS | PPC_OPCODE_BRLOCK
		| PPC_OPCODE_PMR | PPC_OPCODE_CACHELCK | PPC_OPCODE_RFMCI
		| PPC_OPCODE_E500),
    0 },
  { "efs",     PPC_OPCODE_PPC | PPC_OPCODE_EFS,
    0 },
  { "efs2",    PPC_OPCODE_PPC | PPC_OPCODE_EFS | PPC_OPCODE_EFS2,
    0 },
  { "lsp",     PPC_OPCODE_PPC,
    PPC_OPCODE_LSP },
  { "power4",  PPC_OPCODE_PPC | PPC_OPCODE_64 | PPC_OPCODE_POWER4,
    0 },
  { "power5",  (PPC_OPCODE_PPC | PPC_OPCODE_64 | PPC_OPCODE_POWER4
		| PPC_OPCODE_POWER5),
    0 },
  { "power6",  (PPC_OPCODE_PPC | PPC_OPCODE_64 | PPC_OPCODE_POWER4
		| PPC_OPCODE_POWER5 | PPC_OPCODE_POWER6 | PPC_OPCODE_ALTIVEC),
    0 },
  { "power7",  (PPC_OPCODE_PPC | PPC_OPCODE_ISEL | PPC_OPCODE_64
		| PPC_OPCODE_POWER4 | PPC_OPCODE_POWER5 | PPC_OPCODE_POWER6
		| PPC_OPCODE_POWER7 | PPC_OPCODE_ALTIVEC | PPC_OPCODE_VSX),
    0 },
  { "power8",  (PPC_OPCODE_PPC | PPC_OPCODE_ISEL | PPC_OPCODE_64
		| PPC_OPCODE_POWER4 | PPC_OPCODE_POWER5 | PPC_OPCODE_POWER6
		| PPC_OPCODE_POWER7 | PPC_OPCODE_POWER8
		| PPC_OPCODE_ALTIVEC | PPC_OPCODE_VSX),
    0 },
  { "power9",  (PPC_OPCODE_PPC | PPC_OPCODE_ISEL | PPC_OPCODE_64
		| PPC_OPCODE_POWER4 | PPC_OPCODE_POWER5 | PPC_OPCODE_POWER6
		| PPC_OPCODE_POWER7 | PPC_OPCODE_POWER8 | PPC_OPCODE_POWER9
		| PPC_OPCODE_ALTIVEC | PPC_OPCODE_VSX),
    0 },
  { "power10", (PPC_OPCODE_PPC | PPC_OPCODE_ISEL | PPC_OPCODE_64
		| PPC_OPCODE_POWER4 | PPC_OPCODE_POWER5 | PPC_OPCODE_POWER6
		| PPC_OPCODE_POWER7 | PPC_OPCODE_POWER8 | PPC_OPCODE_POWER9
		| PPC_OPCODE_POWER10 | PPC_OPCODE_ALTIVEC | PPC_OPCODE_VSX),
    0 },
  { "libresoc",(PPC_OPCODE_PPC | PPC_OPCODE_ISEL | PPC_OPCODE_64
		| PPC_OPCODE_POWER4 | PPC_OPCODE_POWER5 | PPC_OPCODE_POWER6
		| PPC_OPCODE_POWER7 | PPC_OPCODE_POWER8 | PPC_OPCODE_POWER9
		| PPC_OPCODE_ALTIVEC | PPC_OPCODE_VSX | PPC_OPCODE_SVP64),
    0 },
  { "future",  (PPC_OPCODE_PPC | PPC_OPCODE_ISEL | PPC_OPCODE_64
		| PPC_OPCODE_POWER4 | PPC_OPCODE_POWER5 | PPC_OPCODE_POWER6
		| PPC_OPCODE_POWER7 | PPC_OPCODE_POWER8 | PPC_OPCODE_POWER9
		| PPC_OPCODE_POWER10 | PPC_OPCODE_ALTIVEC | PPC_OPCODE_VSX
		| PPC_OPCODE_FUTURE),
    0 },
  { "ppc",     PPC_OPCODE_PPC,
    0 },
  { "ppc32",   PPC_OPCODE_PPC,
    0 },
  { "32",      PPC_OPCODE_PPC,
    0 },
  { "ppc64",   PPC_OPCODE_PPC | PPC_OPCODE_64,
    0 },
  { "64",      PPC_OPCODE_PPC | PPC_OPCODE_64,
    0 },
  { "ppc64bridge", PPC_OPCODE_PPC | PPC_OPCODE_64_BRIDGE,
    0 },
  { "ppcps",   PPC_OPCODE_PPC | PPC_OPCODE_PPCPS,
    0 },
  { "pwr",     PPC_OPCODE_POWER,
    0 },
  { "pwr2",    PPC_OPCODE_POWER | PPC_OPCODE_POWER2,
    0 },
  { "pwr4",    PPC_OPCODE_PPC | PPC_OPCODE_64 | PPC_OPCODE_POWER4,
    0 },
  { "pwr5",    (PPC_OPCODE_PPC | PPC_OPCODE_64 | PPC_OPCODE_POWER4
		| PPC_OPCODE_POWER5),
    0 },
  { "pwr5x",   (PPC_OPCODE_PPC | PPC_OPCODE_64 | PPC_OPCODE_POWER4
		| PPC_OPCODE_POWER5),
    0 },
  { "pwr6",    (PPC_OPCODE_PPC | PPC_OPCODE_64 | PPC_OPCODE_POWER4
		| PPC_OPCODE_POWER5 | PPC_OPCODE_POWER6 | PPC_OPCODE_ALTIVEC),
    0 },
  { "pwr7",    (PPC_OPCODE_PPC | PPC_OPCODE_ISEL | PPC_OPCODE_64
		| PPC_OPCODE_POWER4 | PPC_OPCODE_POWER5 | PPC_OPCODE_POWER6
		| PPC_OPCODE_POWER7 | PPC_OPCODE_ALTIVEC | PPC_OPCODE_VSX),
    0 },
  { "pwr8",    (PPC_OPCODE_PPC | PPC_OPCODE_ISEL | PPC_OPCODE_64
		| PPC_OPCODE_POWER4 | PPC_OPCODE_POWER5 | PPC_OPCODE_POWER6
		| PPC_OPCODE_POWER7 | PPC_OPCODE_POWER8
		| PPC_OPCODE_ALTIVEC | PPC_OPCODE_VSX),
    0 },
  { "pwr9",    (PPC_OPCODE_PPC | PPC_OPCODE_ISEL | PPC_OPCODE_64
		| PPC_OPCODE_POWER4 | PPC_OPCODE_POWER5 | PPC_OPCODE_POWER6
		| PPC_OPCODE_POWER7 | PPC_OPCODE_POWER8 | PPC_OPCODE_POWER9
		| PPC_OPCODE_ALTIVEC | PPC_OPCODE_VSX),
    0 },
  { "pwr10",   (PPC_OPCODE_PPC | PPC_OPCODE_ISEL | PPC_OPCODE_64
		| PPC_OPCODE_POWER4 | PPC_OPCODE_POWER5 | PPC_OPCODE_POWER6
		| PPC_OPCODE_POWER7 | PPC_OPCODE_POWER8 | PPC_OPCODE_POWER9
		| PPC_OPCODE_POWER10 | PPC_OPCODE_ALTIVEC | PPC_OPCODE_VSX),
    0 },
  { "pwrx",    PPC_OPCODE_POWER | PPC_OPCODE_POWER2,
    0 },
  { "raw",     PPC_OPCODE_PPC,
    PPC_OPCODE_RAW },
  { "spe",     PPC_OPCODE_PPC | PPC_OPCODE_EFS,
    PPC_OPCODE_SPE },
  { "spe2",     PPC_OPCODE_PPC | PPC_OPCODE_EFS | PPC_OPCODE_EFS2 | PPC_OPCODE_SPE,
    PPC_OPCODE_SPE2 },
  { "titan",   (PPC_OPCODE_PPC | PPC_OPCODE_BOOKE | PPC_OPCODE_PMR
		| PPC_OPCODE_RFMCI | PPC_OPCODE_TITAN),
    0 },
  { "vle",     (PPC_OPCODE_PPC | PPC_OPCODE_BOOKE | PPC_OPCODE_SPE
		| PPC_OPCODE_ISEL | PPC_OPCODE_EFS | PPC_OPCODE_BRLOCK
		| PPC_OPCODE_PMR | PPC_OPCODE_CACHELCK | PPC_OPCODE_RFMCI
		| PPC_OPCODE_EFS2 | PPC_OPCODE_SPE2),
    PPC_OPCODE_VLE },
  { "vsx",     PPC_OPCODE_PPC,
    PPC_OPCODE_VSX },
};

/* Switch between Booke and VLE dialects for interlinked dumps.  */
static ppc_cpu_t
get_powerpc_dialect (struct disassemble_info *info)
{
  ppc_cpu_t dialect = 0;

  if (info->private_data)
    dialect = private_data (info)->dialect;

  /* Disassemble according to the section headers flags for VLE-mode.  */
  if (dialect & PPC_OPCODE_VLE
      && info->section != NULL && info->section->owner != NULL
      && bfd_get_flavour (info->section->owner) == bfd_target_elf_flavour
      && elf_object_id (info->section->owner) == PPC32_ELF_DATA
      && (elf_section_flags (info->section) & SHF_PPC_VLE) != 0)
    return dialect;
  else
    return dialect & ~ PPC_OPCODE_VLE;
}

/* Handle -m and -M options that set cpu type, and .machine arg.  */

ppc_cpu_t
ppc_parse_cpu (ppc_cpu_t ppc_cpu, ppc_cpu_t *sticky, const char *arg)
{
  unsigned int i;

  for (i = 0; i < ARRAY_SIZE (ppc_opts); i++)
    if (disassembler_options_cmp (ppc_opts[i].opt, arg) == 0)
      {
	if (ppc_opts[i].sticky)
	  {
	    *sticky |= ppc_opts[i].sticky;
	    if ((ppc_cpu & ~*sticky) != 0)
	      break;
	  }
	ppc_cpu = ppc_opts[i].cpu;
	break;
      }
  if (i >= ARRAY_SIZE (ppc_opts))
    return 0;

  /* SPE and LSP are mutually exclusive, don't allow them both in
     sticky options.  However do allow them both in ppc_cpu, so that
     for example, -mvle -mlsp enables both SPE and LSP for assembly.  */
  if ((ppc_opts[i].sticky & PPC_OPCODE_LSP) != 0)
    *sticky &= ~(PPC_OPCODE_SPE | PPC_OPCODE_SPE2);
  else if ((ppc_opts[i].sticky & (PPC_OPCODE_SPE | PPC_OPCODE_SPE2)) != 0)
    *sticky &= ~PPC_OPCODE_LSP;
  ppc_cpu |= *sticky;

  return ppc_cpu;
}

/* Determine which set of machines to disassemble for.  */

static void
powerpc_init_dialect (struct disassemble_info *info)
{
  ppc_cpu_t dialect = 0;
  ppc_cpu_t sticky = 0;
  struct dis_private *priv = calloc (sizeof (*priv), 1);

  if (priv == NULL)
    return;

  switch (info->mach)
    {
    case bfd_mach_ppc_403:
    case bfd_mach_ppc_403gc:
      dialect = ppc_parse_cpu (dialect, &sticky, "403");
      break;
    case bfd_mach_ppc_405:
      dialect = ppc_parse_cpu (dialect, &sticky, "405");
      break;
    case bfd_mach_ppc_601:
      dialect = ppc_parse_cpu (dialect, &sticky, "601");
      break;
    case bfd_mach_ppc_750:
      dialect = ppc_parse_cpu (dialect, &sticky, "750cl");
      break;
    case bfd_mach_ppc_a35:
    case bfd_mach_ppc_rs64ii:
    case bfd_mach_ppc_rs64iii:
      dialect = ppc_parse_cpu (dialect, &sticky, "pwr2") | PPC_OPCODE_64;
      break;
    case bfd_mach_ppc_e500:
      dialect = ppc_parse_cpu (dialect, &sticky, "e500");
      break;
    case bfd_mach_ppc_e500mc:
      dialect = ppc_parse_cpu (dialect, &sticky, "e500mc");
      break;
    case bfd_mach_ppc_e500mc64:
      dialect = ppc_parse_cpu (dialect, &sticky, "e500mc64");
      break;
    case bfd_mach_ppc_e5500:
      dialect = ppc_parse_cpu (dialect, &sticky, "e5500");
      break;
    case bfd_mach_ppc_e6500:
      dialect = ppc_parse_cpu (dialect, &sticky, "e6500");
      break;
    case bfd_mach_ppc_titan:
      dialect = ppc_parse_cpu (dialect, &sticky, "titan");
      break;
    case bfd_mach_ppc_vle:
      dialect = ppc_parse_cpu (dialect, &sticky, "vle");
      break;
    default:
      if (info->arch == bfd_arch_powerpc)
	dialect = ppc_parse_cpu (dialect, &sticky, "power10") | PPC_OPCODE_ANY;
      else
	dialect = ppc_parse_cpu (dialect, &sticky, "pwr");
      break;
    }

  const char *opt;
  FOR_EACH_DISASSEMBLER_OPTION (opt, info->disassembler_options)
    {
      ppc_cpu_t new_cpu = 0;

      if (disassembler_options_cmp (opt, "32") == 0)
	dialect &= ~(ppc_cpu_t) PPC_OPCODE_64;
      else if (disassembler_options_cmp (opt, "64") == 0)
	dialect |= PPC_OPCODE_64;
      else if ((new_cpu = ppc_parse_cpu (dialect, &sticky, opt)) != 0)
	dialect = new_cpu;
      else
	/* xgettext: c-format */
	opcodes_error_handler (_("warning: ignoring unknown -M%s option"), opt);
    }

  info->private_data = priv;
  private_data (info)->dialect = dialect;
}

#define PPC_OPCD_SEGS (1 + PPC_OP (-1))
static unsigned short powerpc_opcd_indices[PPC_OPCD_SEGS + 1];
#define PREFIX_OPCD_SEGS (1 + PPC_PREFIX_SEG (-1))
static unsigned short prefix_opcd_indices[PREFIX_OPCD_SEGS + 1];
#define VLE_OPCD_SEGS (1 + VLE_OP_TO_SEG (VLE_OP (-1, 0xffff)))
static unsigned short vle_opcd_indices[VLE_OPCD_SEGS + 1];
#define LSP_OPCD_SEGS (1 + LSP_OP_TO_SEG (-1))
static unsigned short lsp_opcd_indices[LSP_OPCD_SEGS + 1];
#define SPE2_OPCD_SEGS (1 + SPE2_XOP_TO_SEG (SPE2_XOP (-1)))
static unsigned short spe2_opcd_indices[SPE2_OPCD_SEGS + 1];

static bool
ppc_symbol_is_valid (asymbol *sym,
		     struct disassemble_info *info ATTRIBUTE_UNUSED)
{
  elf_symbol_type * est;

  if (sym == NULL)
    return false;

  est = elf_symbol_from (sym);

  /* Ignore ELF hidden, local, no-type symbols.
     These are generated by annobin.  */
  if (est != NULL
      && ELF_ST_VISIBILITY (est->internal_elf_sym.st_other) == STV_HIDDEN
      && ELF_ST_BIND (est->internal_elf_sym.st_info) == STB_LOCAL
      && ELF_ST_TYPE (est->internal_elf_sym.st_info) == STT_NOTYPE)
    return false;

  return true;
}

/* Calculate opcode table indices to speed up disassembly,
   and init dialect.  */

void
disassemble_init_powerpc (struct disassemble_info *info)
{
  info->symbol_is_valid = ppc_symbol_is_valid;

  if (powerpc_opcd_indices[PPC_OPCD_SEGS] == 0)
    {
      unsigned seg, idx, op;

      /* PPC opcodes */
      for (seg = 0, idx = 0; seg <= PPC_OPCD_SEGS; seg++)
	{
	  powerpc_opcd_indices[seg] = idx;
	  for (; idx < powerpc_num_opcodes; idx++)
	    if (seg < PPC_OP (powerpc_opcodes[idx].opcode))
	      break;
	}

      /* 64-bit prefix opcodes */
      for (seg = 0, idx = 0; seg <= PREFIX_OPCD_SEGS; seg++)
	{
	  prefix_opcd_indices[seg] = idx;
	  for (; idx < prefix_num_opcodes; idx++)
	    if (seg < PPC_PREFIX_SEG (prefix_opcodes[idx].opcode))
	      break;
	}

      /* VLE opcodes */
      for (seg = 0, idx = 0; seg <= VLE_OPCD_SEGS; seg++)
	{
	  vle_opcd_indices[seg] = idx;
	  for (; idx < vle_num_opcodes; idx++)
	    {
	      op = VLE_OP (vle_opcodes[idx].opcode, vle_opcodes[idx].mask);
	      if (seg < VLE_OP_TO_SEG (op))
		break;
	    }
	}

      /* LSP opcodes */
      for (seg = 0, idx = 0; seg <= LSP_OPCD_SEGS; seg++)
	{
	  lsp_opcd_indices[seg] = idx;
	  for (; idx < lsp_num_opcodes; idx++)
	    if (seg < LSP_OP_TO_SEG (lsp_opcodes[idx].opcode))
	      break;
	}

      /* SPE2 opcodes */
      for (seg = 0, idx = 0; seg <= SPE2_OPCD_SEGS; seg++)
	{
	  spe2_opcd_indices[seg] = idx;
	  for (; idx < spe2_num_opcodes; idx++)
	    {
	      op = SPE2_XOP (spe2_opcodes[idx].opcode);
	      if (seg < SPE2_XOP_TO_SEG (op))
		break;
	    }
	}
    }

  powerpc_init_dialect (info);
  if (info->private_data != NULL)
    {
      private_data (info)->special[0].name = ".got";
      private_data (info)->special[1].name = ".plt";
    }
}

/* Print a big endian PowerPC instruction.  */

int
print_insn_big_powerpc (bfd_vma memaddr, struct disassemble_info *info)
{
  return print_insn_powerpc (memaddr, info, 1, get_powerpc_dialect (info));
}

/* Print a little endian PowerPC instruction.  */

int
print_insn_little_powerpc (bfd_vma memaddr, struct disassemble_info *info)
{
  return print_insn_powerpc (memaddr, info, 0, get_powerpc_dialect (info));
}

/* Extract the operand value from the PowerPC or POWER instruction.  */

static int64_t
operand_value_powerpc (const struct powerpc_operand *operand,
		       uint64_t insn, ppc_cpu_t dialect)
{
  int64_t value;
  int invalid = 0;
  /* Extract the value from the instruction.  */
  if (operand->extract)
    value = (*operand->extract) (insn, dialect, &invalid);
  else
    {
      if (operand->shift >= 0)
	value = (insn >> operand->shift) & operand->bitm;
      else
	value = (insn << -operand->shift) & operand->bitm;
      if ((operand->flags & PPC_OPERAND_SIGNED) != 0)
	{
	  /* BITM is always some number of zeros followed by some
	     number of ones, followed by some number of zeros.  */
	  uint64_t top = operand->bitm;
	  /* top & -top gives the rightmost 1 bit, so this
	     fills in any trailing zeros.  */
	  top |= (top & -top) - 1;
	  top &= ~(top >> 1);
	  value = (value ^ top) - top;
	}
    }

  if ((operand->flags & PPC_OPERAND_NONZERO) != 0)
    ++value;

  return value;
}

/* Determine whether the optional operand(s) should be printed.  */

static bool
skip_optional_operands (const ppc_opindex_t *opindex,
			uint64_t insn, ppc_cpu_t dialect, bool *is_pcrel)
{
  const struct powerpc_operand *operand;
  int num_optional;

  for (num_optional = 0; *opindex != 0; opindex++)
    {
      operand = &powerpc_operands[*opindex];
      if ((operand->flags & PPC_OPERAND_NEXT) != 0)
	return false;
      if ((operand->flags & PPC_OPERAND_OPTIONAL) != 0)
	{
	  int64_t value = operand_value_powerpc (operand, insn, dialect);

	  if (operand->shift == 52)
	    *is_pcrel = value != 0;

	  /* Negative count is used as a flag to extract function.  */
	  --num_optional;
	  if (value != ppc_optional_operand_value (operand, insn, dialect,
						   num_optional))
	    return false;
	}
    }

  return true;
}

/* Find a match for INSN in the opcode table, given machine DIALECT.  */

static const struct powerpc_opcode *
lookup_powerpc (uint64_t insn, ppc_cpu_t dialect)
{
  const struct powerpc_opcode *opcode, *opcode_end;
  unsigned long op;

  /* Get the major opcode of the instruction.  */
  op = PPC_OP (insn);

  /* Find the first match in the opcode table for this major opcode.  */
  opcode_end = powerpc_opcodes + powerpc_opcd_indices[op + 1];
  for (opcode = powerpc_opcodes + powerpc_opcd_indices[op];
       opcode < opcode_end;
       ++opcode)
    {
      const ppc_opindex_t *opindex;
      const struct powerpc_operand *operand;
      int invalid;

      if ((insn & opcode->mask) != opcode->opcode
	  || ((dialect & PPC_OPCODE_ANY) == 0
	      && ((opcode->flags & dialect) == 0
		  || (opcode->deprecated & dialect) != 0))
	  || (opcode->deprecated & dialect & PPC_OPCODE_RAW) != 0)
	continue;

      /* Check validity of operands.  */
      invalid = 0;
      for (opindex = opcode->operands; *opindex != 0; opindex++)
	{
	  operand = powerpc_operands + *opindex;
	  if (operand->extract)
	    (*operand->extract) (insn, dialect, &invalid);
	}
      if (invalid)
	continue;

      return opcode;
    }

  return NULL;
}

/* Find a match for INSN in the PREFIX opcode table.  */

static const struct powerpc_opcode *
lookup_prefix (uint64_t insn, ppc_cpu_t dialect)
{
  const struct powerpc_opcode *opcode, *opcode_end;
  unsigned long seg;

  /* Get the opcode segment of the instruction.  */
  seg = PPC_PREFIX_SEG (insn);

  /* Find the first match in the opcode table for this major opcode.  */
  opcode_end = prefix_opcodes + prefix_opcd_indices[seg + 1];
  for (opcode = prefix_opcodes + prefix_opcd_indices[seg];
       opcode < opcode_end;
       ++opcode)
    {
      const ppc_opindex_t *opindex;
      const struct powerpc_operand *operand;
      int invalid;

      if ((insn & opcode->mask) != opcode->opcode
	  || ((dialect & PPC_OPCODE_ANY) == 0
	      && (opcode->flags & dialect) == 0)
	  || (opcode->deprecated & dialect) != 0)
	continue;

      /* Check validity of operands.  */
      invalid = 0;
      for (opindex = opcode->operands; *opindex != 0; opindex++)
	{
	  operand = powerpc_operands + *opindex;
	  if (operand->extract)
	    (*operand->extract) (insn, dialect, &invalid);
	}
      if (invalid)
	continue;

      return opcode;
    }

  return NULL;
}

/* Find a match for INSN in the VLE opcode table.  */

static const struct powerpc_opcode *
lookup_vle (uint64_t insn, ppc_cpu_t dialect)
{
  const struct powerpc_opcode *opcode;
  const struct powerpc_opcode *opcode_end;
  unsigned op, seg;

  op = PPC_OP (insn);
  if (op >= 0x20 && op <= 0x37)
    {
      /* This insn has a 4-bit opcode.  */
      op &= 0x3c;
    }
  seg = VLE_OP_TO_SEG (op);

  /* Find the first match in the opcode table for this major opcode.  */
  opcode_end = vle_opcodes + vle_opcd_indices[seg + 1];
  for (opcode = vle_opcodes + vle_opcd_indices[seg];
       opcode < opcode_end;
       ++opcode)
    {
      uint64_t table_opcd = opcode->opcode;
      uint64_t table_mask = opcode->mask;
      bool table_op_is_short = PPC_OP_SE_VLE(table_mask);
      uint64_t insn2;
      const ppc_opindex_t *opindex;
      const struct powerpc_operand *operand;
      int invalid;

      insn2 = insn;
      if (table_op_is_short)
	insn2 >>= 16;
      if ((insn2 & table_mask) != table_opcd
	  || (opcode->deprecated & dialect) != 0)
	continue;

      /* Check validity of operands.  */
      invalid = 0;
      for (opindex = opcode->operands; *opindex != 0; ++opindex)
	{
	  operand = powerpc_operands + *opindex;
	  if (operand->extract)
	    (*operand->extract) (insn, (ppc_cpu_t)0, &invalid);
	}
      if (invalid)
	continue;

      return opcode;
    }

  return NULL;
}

/* Find a match for INSN in the LSP opcode table.  */

static const struct powerpc_opcode *
lookup_lsp (uint64_t insn, ppc_cpu_t dialect)
{
  const struct powerpc_opcode *opcode, *opcode_end;
  unsigned op, seg;

  op = PPC_OP (insn);
  if (op != 0x4)
    return NULL;

  seg = LSP_OP_TO_SEG (insn);

  /* Find the first match in the opcode table for this opcode.  */
  opcode_end = lsp_opcodes + lsp_opcd_indices[seg + 1];
  for (opcode = lsp_opcodes + lsp_opcd_indices[seg];
       opcode < opcode_end;
       ++opcode)
    {
      const ppc_opindex_t *opindex;
      const struct powerpc_operand *operand;
      int invalid;

      if ((insn & opcode->mask) != opcode->opcode
	  || (opcode->deprecated & dialect) != 0)
	continue;

      /* Check validity of operands.  */
      invalid = 0;
      for (opindex = opcode->operands; *opindex != 0; ++opindex)
	{
	  operand = powerpc_operands + *opindex;
	  if (operand->extract)
	    (*operand->extract) (insn, (ppc_cpu_t) 0, &invalid);
	}
      if (invalid)
	continue;

      return opcode;
    }

  return NULL;
}

/* Find a match for INSN in the SPE2 opcode table.  */

static const struct powerpc_opcode *
lookup_spe2 (uint64_t insn, ppc_cpu_t dialect)
{
  const struct powerpc_opcode *opcode, *opcode_end;
  unsigned op, xop, seg;

  op = PPC_OP (insn);
  if (op != 0x4)
    {
      /* This is not SPE2 insn.
       * All SPE2 instructions have OP=4 and differs by XOP  */
      return NULL;
    }
  xop = SPE2_XOP (insn);
  seg = SPE2_XOP_TO_SEG (xop);

  /* Find the first match in the opcode table for this opcode.  */
  opcode_end = spe2_opcodes + spe2_opcd_indices[seg + 1];
  for (opcode = spe2_opcodes + spe2_opcd_indices[seg];
       opcode < opcode_end;
       ++opcode)
    {
      uint64_t table_opcd = opcode->opcode;
      uint64_t table_mask = opcode->mask;
      uint64_t insn2;
      const ppc_opindex_t *opindex;
      const struct powerpc_operand *operand;
      int invalid;

      insn2 = insn;
      if ((insn2 & table_mask) != table_opcd
	  || (opcode->deprecated & dialect) != 0)
	continue;

      /* Check validity of operands.  */
      invalid = 0;
      for (opindex = opcode->operands; *opindex != 0; ++opindex)
	{
	  operand = powerpc_operands + *opindex;
	  if (operand->extract)
	    (*operand->extract) (insn, (ppc_cpu_t)0, &invalid);
	}
      if (invalid)
	continue;

      return opcode;
    }

  return NULL;
}

static arelent *
bsearch_reloc (arelent **lo, arelent **hi, bfd_vma vma)
{
  while (lo < hi)
    {
      arelent **mid = lo + (hi - lo) / 2;
      arelent *rel = *mid;

      if (vma < rel->address)
	hi = mid;
      else if (vma > rel->address)
	lo = mid + 1;
      else
	return rel;
    }
  return NULL;
}

static bool
print_got_plt (struct sec_buf *sb, uint64_t vma, struct disassemble_info *info)
{
  if (sb->name != NULL)
    {
      asection *s = sb->sec;
      if (s == NULL)
	{
	  s = bfd_get_section_by_name (info->section->owner, sb->name);
	  sb->sec = s;
	  if (s == NULL)
	    sb->name = NULL;
	}
      if (s != NULL
	  && vma >= s->vma
	  && vma < s->vma + s->size)
	{
	  asymbol *sym = NULL;
	  uint64_t ent = 0;
	  if (info->dynrelcount > 0)
	    {
	      arelent **lo = info->dynrelbuf;
	      arelent **hi = lo + info->dynrelcount;
	      arelent *rel = bsearch_reloc (lo, hi, vma);
	      if (rel != NULL && rel->sym_ptr_ptr != NULL)
		sym = *rel->sym_ptr_ptr;
	    }
	  if (sym == NULL && (s->flags & SEC_HAS_CONTENTS) != 0)
	    {
	      if (sb->buf == NULL
		  && !bfd_malloc_and_get_section (s->owner, s, &sb->buf))
		sb->name = NULL;
	      if (sb->buf != NULL)
		{
		  ent = bfd_get_64 (s->owner, sb->buf + (vma - s->vma));
		  if (ent != 0)
		    sym = (*info->symbol_at_address_func) (ent, info);
		}
	    }
	  (*info->fprintf_styled_func) (info->stream, dis_style_text, " [");
	  if (sym != NULL)
	    {
	      (*info->fprintf_styled_func) (info->stream, dis_style_symbol,
					    "%s", bfd_asymbol_name (sym));
	      (*info->fprintf_styled_func) (info->stream, dis_style_text, "@");
	      (*info->fprintf_styled_func) (info->stream, dis_style_symbol,
					    "%s", sb->name + 1);
	    }
	  else
	    {
	      (*info->fprintf_styled_func) (info->stream, dis_style_address,
					    "%" PRIx64, ent);
	      (*info->fprintf_styled_func) (info->stream, dis_style_text, "@");
	      (*info->fprintf_styled_func) (info->stream, dis_style_symbol,
					    "%s", sb->name + 1);
	    }
	  (*info->fprintf_styled_func) (info->stream, dis_style_text, "]");
	  return true;
	}
    }
  return false;
}

/* Print a PowerPC or POWER instruction.  */

static int
print_insn_powerpc (bfd_vma memaddr,
		    struct disassemble_info *info,
		    int bigendian,
		    ppc_cpu_t dialect)
{
  bfd_byte buffer[4];
  int status;
  uint64_t insn;
  const struct powerpc_opcode *opcode;
  int insn_length = 4;  /* Assume we have a normal 4-byte instruction.  */

  status = (*info->read_memory_func) (memaddr, buffer, 4, info);

  /* The final instruction may be a 2-byte VLE insn.  */
  if (status != 0 && (dialect & PPC_OPCODE_VLE) != 0)
    {
      /* Clear buffer so unused bytes will not have garbage in them.  */
      buffer[2] = buffer[3] = 0;
      status = (*info->read_memory_func) (memaddr, buffer, 2, info);
      insn_length = 2;
    }

  if (status != 0)
    {
      (*info->memory_error_func) (status, memaddr, info);
      return -1;
    }

  if (bigendian)
    insn = bfd_getb32 (buffer);
  else
    insn = bfd_getl32 (buffer);

  /* Get the major opcode of the insn.  */
  opcode = NULL;
  if ((dialect & PPC_OPCODE_POWER10) != 0
      && PPC_OP (insn) == 0x1)
    {
      uint64_t temp_insn, suffix;
      status = (*info->read_memory_func) (memaddr + 4, buffer, 4, info);
      if (status == 0)
	{
	  if (bigendian)
	    suffix = bfd_getb32 (buffer);
	  else
	    suffix = bfd_getl32 (buffer);
	  temp_insn = (insn << 32) | suffix;
	  opcode = lookup_prefix (temp_insn, dialect & ~PPC_OPCODE_ANY);
	  if (opcode == NULL && (dialect & PPC_OPCODE_ANY) != 0)
	    opcode = lookup_prefix (temp_insn, dialect);
	  if (opcode != NULL)
	    {
	      insn = temp_insn;
	      insn_length = 8;
	      if ((info->flags & WIDE_OUTPUT) != 0)
		info->bytes_per_line = 8;
	    }
	}
    }
  if (opcode == NULL && (dialect & PPC_OPCODE_VLE) != 0)
    {
      opcode = lookup_vle (insn, dialect);
      if (opcode != NULL && PPC_OP_SE_VLE (opcode->mask))
	{
	  /* The operands will be fetched out of the 16-bit instruction.  */
	  insn >>= 16;
	  insn_length = 2;
	}
    }
  if (opcode == NULL && insn_length == 4)
    {
      if ((dialect & PPC_OPCODE_LSP) != 0)
	opcode = lookup_lsp (insn, dialect);
      if ((dialect & PPC_OPCODE_SPE2) != 0)
	opcode = lookup_spe2 (insn, dialect);
      if (opcode == NULL)
	opcode = lookup_powerpc (insn, dialect & ~PPC_OPCODE_ANY);
      if (opcode == NULL && (dialect & PPC_OPCODE_ANY) != 0)
	opcode = lookup_powerpc (insn, dialect);
      if (opcode == NULL && (dialect & PPC_OPCODE_ANY) != 0)
	opcode = lookup_spe2 (insn, dialect);
      if (opcode == NULL && (dialect & PPC_OPCODE_ANY) != 0)
	opcode = lookup_lsp (insn, dialect);
    }

  if (opcode != NULL)
    {
      const ppc_opindex_t *opindex;
      const struct powerpc_operand *operand;
      enum {
	need_comma = 0,
	need_1space = 1,
	need_2spaces = 2,
	need_3spaces = 3,
	need_4spaces = 4,
	need_5spaces = 5,
	need_6spaces = 6,
	need_7spaces = 7,
	need_paren
      } op_separator;
      bool skip_optional;
      bool is_pcrel;
      uint64_t d34;
      int blanks;

      (*info->fprintf_styled_func) (info->stream, dis_style_mnemonic,
				    "%s", opcode->name);
      /* gdb fprintf_styled_func doesn't return count printed.  */
      blanks = 8 - strlen (opcode->name);
      if (blanks <= 0)
	blanks = 1;

      /* Now extract and print the operands.  */
      op_separator = blanks;
      skip_optional = false;
      is_pcrel = false;
      d34 = 0;
      for (opindex = opcode->operands; *opindex != 0; opindex++)
	{
	  int64_t value;

	  operand = powerpc_operands + *opindex;

	  /* If all of the optional operands past this one have their
	     default value, then don't print any of them.  Except in
	     raw mode, print them all.  */
	  if ((operand->flags & PPC_OPERAND_OPTIONAL) != 0
	      && (dialect & PPC_OPCODE_RAW) == 0)
	    {
	      if (!skip_optional)
		skip_optional = skip_optional_operands (opindex, insn,
							dialect, &is_pcrel);
	      if (skip_optional)
		continue;
	    }

	  value = operand_value_powerpc (operand, insn, dialect);

	  if (op_separator == need_comma)
	    (*info->fprintf_styled_func) (info->stream, dis_style_text, ",");
	  else if (op_separator == need_paren)
	    (*info->fprintf_styled_func) (info->stream, dis_style_text, "(");
	  else
	    (*info->fprintf_styled_func) (info->stream, dis_style_text, "%*s",
					  op_separator, " ");

	  /* Print the operand as directed by the flags.  */
	  if ((operand->flags & PPC_OPERAND_GPR) != 0
	      || ((operand->flags & PPC_OPERAND_GPR_0) != 0 && value != 0))
	    (*info->fprintf_styled_func) (info->stream, dis_style_register,
					  "r%" PRId64, value);
	  else if ((operand->flags & PPC_OPERAND_FPR) != 0)
	    (*info->fprintf_styled_func) (info->stream, dis_style_register,
					  "f%" PRId64, value);
	  else if ((operand->flags & PPC_OPERAND_VR) != 0)
	    (*info->fprintf_styled_func) (info->stream, dis_style_register,
					  "v%" PRId64, value);
	  else if ((operand->flags & PPC_OPERAND_VSR) != 0)
	    (*info->fprintf_styled_func) (info->stream, dis_style_register,
					  "vs%" PRId64, value);
	  else if ((operand->flags & PPC_OPERAND_DMR) != 0)
	    (*info->fprintf_styled_func) (info->stream, dis_style_register,
					  "dm%" PRId64, value);
	  else if ((operand->flags & PPC_OPERAND_ACC) != 0)
	    (*info->fprintf_styled_func) (info->stream, dis_style_register,
					  "a%" PRId64, value);
	  else if ((operand->flags & PPC_OPERAND_RELATIVE) != 0)
	    (*info->print_address_func) (memaddr + value, info);
	  else if ((operand->flags & PPC_OPERAND_ABSOLUTE) != 0)
	    (*info->print_address_func) ((bfd_vma) value & 0xffffffff, info);
	  else if ((operand->flags & PPC_OPERAND_FSL) != 0)
	    (*info->fprintf_styled_func) (info->stream, dis_style_register,
					  "fsl%" PRId64, value);
	  else if ((operand->flags & PPC_OPERAND_FCR) != 0)
	    (*info->fprintf_styled_func) (info->stream, dis_style_register,
					  "fcr%" PRId64, value);
	  else if ((operand->flags & PPC_OPERAND_UDI) != 0)
	    (*info->fprintf_styled_func) (info->stream, dis_style_register,
					  "%" PRId64, value);
	  else if ((operand->flags & PPC_OPERAND_CR_REG) != 0
		   && (operand->flags & PPC_OPERAND_CR_BIT) == 0
		   && (((dialect & PPC_OPCODE_PPC) != 0)
		       || ((dialect & PPC_OPCODE_VLE) != 0)))
	    (*info->fprintf_styled_func) (info->stream, dis_style_register,
					  "cr%" PRId64, value);
	  else if ((operand->flags & PPC_OPERAND_CR_BIT) != 0
		   && (operand->flags & PPC_OPERAND_CR_REG) == 0
		   && (((dialect & PPC_OPCODE_PPC) != 0)
		       || ((dialect & PPC_OPCODE_VLE) != 0)))
	    {
	      static const char *cbnames[4] = { "lt", "gt", "eq", "so" };
	      int cr;
	      int cc;

	      cr = value >> 2;
	      cc = value & 3;
	      if (cr != 0)
		{
		  (*info->fprintf_styled_func) (info->stream, dis_style_text,
						"4*");
		  (*info->fprintf_styled_func) (info->stream,
						dis_style_register,
						"cr%d", cr);
		  (*info->fprintf_styled_func) (info->stream, dis_style_text,
						"+");
		}

	      (*info->fprintf_styled_func) (info->stream,
					    dis_style_sub_mnemonic,
					    "%s", cbnames[cc]);
	    }
	  else
	    {
	      /* An immediate, but what style?  */
	      enum disassembler_style style;

	      if ((operand->flags & PPC_OPERAND_PARENS) != 0)
		style = dis_style_address_offset;
	      else
		style = dis_style_immediate;

	      (*info->fprintf_styled_func) (info->stream, style,
					    "%" PRId64, value);
	    }

	  if (operand->shift == 52)
	    is_pcrel = value != 0;
	  else if (operand->bitm == UINT64_C (0x3ffffffff))
	    d34 = value;

	  if (op_separator == need_paren)
	    (*info->fprintf_styled_func) (info->stream, dis_style_text, ")");

	  op_separator = need_comma;
	  if ((operand->flags & PPC_OPERAND_PARENS) != 0)
	    op_separator = need_paren;
	}

      if (is_pcrel)
	{
	  d34 += memaddr;
	  (*info->fprintf_styled_func) (info->stream,
					dis_style_comment_start,
					"\t# %" PRIx64, d34);
	  asymbol *sym = (*info->symbol_at_address_func) (d34, info);
	  if (sym)
	    (*info->fprintf_styled_func) (info->stream, dis_style_text,
					  " <%s>", bfd_asymbol_name (sym));

	  if (info->private_data != NULL
	      && info->section != NULL
	      && info->section->owner != NULL
	      && (bfd_get_file_flags (info->section->owner)
		  & (EXEC_P | DYNAMIC)) != 0
	      && ((insn & ((-1ULL << 50) | (0x3fULL << 26)))
		  == ((1ULL << 58) | (1ULL << 52) | (57ULL << 26)) /* pld */))
	    {
	      for (int i = 0; i < 2; i++)
		if (print_got_plt (private_data (info)->special + i, d34, info))
		  break;
	    }
	}

      /* We have found and printed an instruction.  */
      return insn_length;
    }

  /* We could not find a match.  */
  if (insn_length == 4)
    (*info->fprintf_styled_func) (info->stream,
				  dis_style_assembler_directive, ".long");
  else
    {
      (*info->fprintf_styled_func) (info->stream,
				    dis_style_assembler_directive, ".word");
      insn >>= 16;
    }
  (*info->fprintf_styled_func) (info->stream, dis_style_text, " ");
  (*info->fprintf_styled_func) (info->stream, dis_style_immediate, "0x%x",
				(unsigned int) insn);


  return insn_length;
}

const disasm_options_and_args_t *
disassembler_options_powerpc (void)
{
  static disasm_options_and_args_t *opts_and_args;

  if (opts_and_args == NULL)
    {
      size_t i, num_options = ARRAY_SIZE (ppc_opts);
      disasm_options_t *opts;

      opts_and_args = XNEW (disasm_options_and_args_t);
      opts_and_args->args = NULL;

      opts = &opts_and_args->options;
      opts->name = XNEWVEC (const char *, num_options + 1);
      opts->description = NULL;
      opts->arg = NULL;
      for (i = 0; i < num_options; i++)
	opts->name[i] = ppc_opts[i].opt;
      /* The array we return must be NULL terminated.  */
      opts->name[i] = NULL;
    }

  return opts_and_args;
}

void
print_ppc_disassembler_options (FILE *stream)
{
  unsigned int i, col;

  fprintf (stream, _("\n\
The following PPC specific disassembler options are supported for use with\n\
the -M switch:\n"));

  for (col = 0, i = 0; i < ARRAY_SIZE (ppc_opts); i++)
    {
      col += fprintf (stream, " %s,", ppc_opts[i].opt);
      if (col > 66)
	{
	  fprintf (stream, "\n");
	  col = 0;
	}
    }
  fprintf (stream, "\n");
}
