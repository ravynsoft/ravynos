/* BFD backend for RISC-V
   Copyright (C) 2011-2023 Free Software Foundation, Inc.

   Contributed by Andrew Waterman (andrew@sifive.com).
   Based on MIPS target.

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
   along with this program; see the file COPYING3. If not,
   see <http://www.gnu.org/licenses/>.  */

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"
#include "cpu-riscv.h"

static const bfd_arch_info_type *
riscv_compatible (const bfd_arch_info_type *a, const bfd_arch_info_type *b)
{
  if (a->arch != b->arch)
    return NULL;

  /* Machine compatibility is checked in
     _bfd_riscv_elf_merge_private_bfd_data.  */

  return a;
}

/* Return TRUE if STRING matches the architecture described by INFO.  */

static bool
riscv_scan (const struct bfd_arch_info *info, const char *string)
{
  if (bfd_default_scan (info, string))
    return true;

  /* The incoming STRING might take the form of riscv:rvXXzzz, where XX is
     32 or 64, and zzz are one or more extension characters.  As we
     currently only have 3 architectures defined, 'riscv', 'riscv:rv32',
     and 'riscv:rv64', we would like to ignore the zzz for the purpose of
     matching here.

     However, we don't want the default 'riscv' to match over a more
     specific 'riscv:rv32' or 'riscv:rv64', so in the case of the default
     architecture (with the shorter 'riscv' name) we don't allow any
     special matching, but for the 'riscv:rvXX' cases, we allow a match
     with any additional trailing characters being ignored.  */
  if (!info->the_default
      && strncasecmp (string, info->printable_name,
                      strlen (info->printable_name)) == 0)
    return true;

  return false;
}

#define N(BITS, NUMBER, PRINT, DEFAULT, NEXT)			\
  {								\
    BITS,      /* Bits in a word.  */				\
    BITS,      /* Bits in an address.  */			\
    8,	       /* Bits in a byte.  */				\
    bfd_arch_riscv,						\
    NUMBER,							\
    "riscv",							\
    PRINT,							\
    3,								\
    DEFAULT,							\
    riscv_compatible,						\
    riscv_scan,							\
    bfd_arch_default_fill,					\
    NEXT,							\
    0 /* Maximum offset of a reloc from the start of an insn.  */\
  }

/* This enum must be kept in the same order as arch_info_struct.  */
enum
{
  I_riscv64,
  I_riscv32
};

#define NN(index) (&arch_info_struct[(index) + 1])

/* This array must be kept in the same order as the anonymous enum above,
   and each entry except the last should end with NN (my enum value).  */
static const bfd_arch_info_type arch_info_struct[] =
{
  N (64, bfd_mach_riscv64, "riscv:rv64", false, NN (I_riscv64)),
  N (32, bfd_mach_riscv32, "riscv:rv32", false, NULL)
};

/* The default architecture is riscv:rv64.  */
const bfd_arch_info_type bfd_riscv_arch =
  N (64, 0, "riscv", true, &arch_info_struct[0]);

/* List for all supported ISA spec versions.  */
const struct riscv_spec riscv_isa_specs[] =
{
  {"2.2",      ISA_SPEC_CLASS_2P2},
  {"20190608", ISA_SPEC_CLASS_20190608},
  {"20191213", ISA_SPEC_CLASS_20191213},
};

/* List for all supported privileged spec versions.  */
const struct riscv_spec riscv_priv_specs[] =
{
  {"1.9.1", PRIV_SPEC_CLASS_1P9P1},
  {"1.10",  PRIV_SPEC_CLASS_1P10},
  {"1.11",  PRIV_SPEC_CLASS_1P11},
  {"1.12",  PRIV_SPEC_CLASS_1P12},
};

/* Get the corresponding CSR version class by giving privilege
   version numbers.  It is usually used to convert the priv
   attribute numbers into the corresponding class.  */

void
riscv_get_priv_spec_class_from_numbers (unsigned int major,
					unsigned int minor,
					unsigned int revision,
					enum riscv_spec_class *class)
{
  enum riscv_spec_class class_t = *class;
  char buf[36];

  if (revision != 0)
    snprintf (buf, sizeof (buf), "%u.%u.%u", major, minor, revision);
  else
    snprintf (buf, sizeof (buf), "%u.%u", major, minor);

  RISCV_GET_PRIV_SPEC_CLASS (buf, class_t);
  *class = class_t;
}

/* Define mapping symbols for riscv.  */

bool
riscv_elf_is_mapping_symbols (const char *name)
{
  return (!strncmp (name, "$d", 2)
	  || !strncmp (name, "$x", 2));
}
