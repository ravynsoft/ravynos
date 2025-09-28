// justsyms_exec.c -- test --just-symbols for gold

// Copyright (C) 2011-2023 Free Software Foundation, Inc.
// Written by Cary Coutant <ccoutant@google.com>.

// This file is part of gold.

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
// MA 02110-1301, USA.

// The Linux kernel builds an executable file using a linker script, and
// then links against that object file using the -R option.  This is a
// test for that usage.

#include <stdio.h>

extern int exported_func(void);

extern int exported_data;

static int errs = 0;

void check(void *sym, long v, const char *name);

void
check(void *sym, long v, const char *name)
{
  if (sym != (void *)v)
    {
      fprintf(stderr, "&%s is %8p, expected %08lx\n", name, sym, v);
      errs++;
    }
}

int
main(void)
{
#if !defined (__powerpc64__) || (defined (_CALL_ELF) && _CALL_ELF == 2)
  /* PowerPC64 ELFv1 uses function descriptors.  */
  check(exported_func, 0x1000200, "exported_func");
#endif
  check(&exported_data, 0x2000000, "exported_data");
  return errs;
}
