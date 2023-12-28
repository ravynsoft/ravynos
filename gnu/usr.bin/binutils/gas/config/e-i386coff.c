/* Copyright (C) 2007-2023 Free Software Foundation, Inc.

   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

#include "as.h"
#include "emul.h"

static const char *i386coff_bfd_name (void);

static const char *
i386coff_bfd_name (void)
{
  abort ();
  return NULL;
}

#define emul_bfd_name	i386coff_bfd_name
#define emul_format	&coff_format_ops

#define emul_name	"i386coff"
#define emul_struct_name i386coff
#define emul_default_endian 0
#include "emul-target.h"
