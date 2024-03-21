/* This file defines standard ELF types, structures, and macros.
   Copyright (C) 1995-2023 Free Software Foundation, Inc.

   This file is part of libctf.

   libctf is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 3, or (at your option) any later
   version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not see
   <http://www.gnu.org/licenses/>.  */

#ifndef _CTF_ELF_H
#define _CTF_ELF_H

#include "config.h"
#include "ansidecl.h"
#include <stdint.h>
#include "elf/common.h"
#include "elf/external.h"

typedef uint32_t Elf32_Word;
typedef uint32_t Elf64_Word;
typedef uint32_t Elf32_Addr;
typedef uint64_t Elf64_Addr;
typedef uint64_t Elf64_Xword;
typedef uint16_t Elf32_Section;
typedef uint16_t Elf64_Section;

#define SHN_EXTABS	0xFFF1		/* Associated symbol is absolute */

/* Symbol table entry.  */

typedef struct
{
  Elf32_Word	st_name;		/* Symbol name (string tbl index) */
  Elf32_Addr	st_value;		/* Symbol value */
  Elf32_Word	st_size;		/* Symbol size */
  unsigned char	st_info;		/* Symbol type and binding */
  unsigned char	st_other;		/* Symbol visibility */
  Elf32_Section	st_shndx;		/* Section index */
} Elf32_Sym;

typedef struct
{
  Elf64_Word	st_name;		/* Symbol name (string tbl index) */
  unsigned char	st_info;		/* Symbol type and binding */
  unsigned char st_other;		/* Symbol visibility */
  Elf64_Section	st_shndx;		/* Section index */
  Elf64_Addr	st_value;		/* Symbol value */
  Elf64_Xword	st_size;		/* Symbol size */
} Elf64_Sym;

#endif	/* _CTF_ELF_H */
