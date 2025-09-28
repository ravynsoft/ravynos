/* Copyright (C) 2021-2023 Free Software Foundation, Inc.
   Contributed by Oracle.

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#ifndef _DBE_TYPES_H
#define _DBE_TYPES_H

#include <stdint.h>
#include "gp-time.h"

typedef unsigned long long Size;    /* object sizes in 64 bit apps */
typedef unsigned long long Vaddr;   /* process address for 64 bit apps */

typedef unsigned long long ull_t;
typedef long long ll_t;
typedef unsigned long ul_t;

// Note: these values are stored in archive files; changing them
// may cause old archives to become incompatible.
enum Platform_t
{
  Unknown = 0,
  Sparc,
  Sparcv9,
  Intel,
  Sparcv8plus,
  Java,
  Amd64,
  Aarch64
};

enum WSize_t
{
  Wnone,
  W32,
  W64
};

enum VMode
{
  VMODE_MACHINE = 0,
  VMODE_USER,
  VMODE_EXPERT
};

#endif /* _DBE_TYPES_H */
