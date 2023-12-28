/* BFD support for AArch64.
   Copyright (C) 2009-2023 Free Software Foundation, Inc.
   Contributed by ARM Ltd.

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
#include "libiberty.h"
#include "cpu-aarch64.h"

/* This routine is provided two arch_infos and works out which Aarch64
   machine which would be compatible with both and returns a pointer
   to its info structure.  */

static const bfd_arch_info_type *
compatible (const bfd_arch_info_type * a, const bfd_arch_info_type * b)
{
  /* If a & b are for different architecture we can do nothing.  */
  if (a->arch != b->arch)
    return NULL;

  /* If a & b are for the same machine then all is well.  */
  if (a->mach == b->mach)
    return a;

  /* Don't allow mixing data models.  */
  if ((a->mach ^ b->mach) & (bfd_mach_aarch64_ilp32 | bfd_mach_aarch64_llp64))
    return NULL;

  /* Otherwise if either a or b is the 'default' machine
     then it can be polymorphed into the other.  */
  if (a->the_default)
    return b;

  if (b->the_default)
    return a;

  /* So far all newer cores are
     supersets of previous cores.  */
  if (a->mach < b->mach)
    return b;
  else if (a->mach > b->mach)
    return a;

  /* Never reached!  */
  return NULL;
}

static struct
{
  unsigned int mach;
  char *name;
}
processors[] =
{
  { bfd_mach_aarch64,	  "cortex-a34"	    },
  { bfd_mach_aarch64,	  "cortex-a65"	    },
  { bfd_mach_aarch64,	  "cortex-a65ae"    },
  { bfd_mach_aarch64,	  "cortex-a76ae"    },
  { bfd_mach_aarch64,	  "cortex-a77"	    }
};

static bool
scan (const struct bfd_arch_info *info, const char *string)
{
  int i;

  /* First test for an exact match.  */
  if (strcasecmp (string, info->printable_name) == 0)
    return true;

  /* If there is a prefix of "aarch64:" then skip it.  */
  const char * colon;
  if ((colon = strchr (string, ':')) != NULL)
    {
      if (strncasecmp (string, "aarch64", colon - string) != 0)
	return false;
      string = colon + 1;
    }

  /* Next check for a processor name instead of an Architecture name.  */
  for (i = sizeof (processors) / sizeof (processors[0]); i--;)
    {
      if (strcasecmp (string, processors[i].name) == 0)
	break;
    }

  if (i != -1 && info->mach == processors[i].mach)
    return true;

  /* Finally check for the default architecture.  */
  if (strcasecmp (string, "aarch64") == 0)
    return info->the_default;

  return false;
}

/* Figure out if llp64 is default */
#if DEFAULT_VECTOR == aarch64_pe_le_vec
#define LLP64_DEFAULT true
#define AARCH64_DEFAULT false
#else
#define LLP64_DEFAULT false
#define AARCH64_DEFAULT true
#endif

#define N(NUMBER, PRINT, WORDSIZE, ADDRSIZE, DEFAULT, NEXT)		\
  { WORDSIZE, ADDRSIZE, 8, bfd_arch_aarch64, NUMBER,		\
     "aarch64", PRINT, 4, DEFAULT, compatible, scan,		\
       bfd_arch_default_fill, NEXT, 0 }

 static const bfd_arch_info_type bfd_aarch64_arch_v8_r =
  N (bfd_mach_aarch64_8R, "aarch64:armv8-r", 64, 64, false, NULL);

static const bfd_arch_info_type bfd_aarch64_arch_ilp32 =
  N (bfd_mach_aarch64_ilp32, "aarch64:ilp32", 32, 32, false,
     &bfd_aarch64_arch_v8_r);

static const bfd_arch_info_type bfd_aarch64_arch_llp64 =
  N (bfd_mach_aarch64_llp64, "aarch64:llp64", 32, 64, LLP64_DEFAULT,
     &bfd_aarch64_arch_ilp32);

 const bfd_arch_info_type bfd_aarch64_arch =
  N (0, "aarch64", 64, 64, AARCH64_DEFAULT, &bfd_aarch64_arch_llp64);

bool
bfd_is_aarch64_special_symbol_name (const char *name, int type)
{
  if (!name || name[0] != '$')
    return false;
  if (name[1] == 'x' || name[1] == 'd')
    type &= BFD_AARCH64_SPECIAL_SYM_TYPE_MAP;
  else if (name[1] == 'm' || name[1] == 'f' || name[1] == 'p')
    type &= BFD_AARCH64_SPECIAL_SYM_TYPE_TAG;
  else
    return false;

  return (type != 0 && (name[2] == 0 || name[2] == '.'));
}
