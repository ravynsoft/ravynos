/* Copyright (C) 2007-2023 Free Software Foundation, Inc.

   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 3,
   or (at your option) any later version.

   GAS is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
   the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

/* This file is te-netbsd532.h
   Written by Ian Dall <idall@eleceng.adelaide.edu.au>
   19-Jun-94.  */

#define TARGET_FORMAT		"a.out-ns32k-netbsd"

#include "obj-format.h"

/* Maybe these should be more like TC_NS32532 and TC_NS32381 in case
   of conflicts. NS32381 is used in opcode/ns32k.h and that is also
   used by GDB. Need to check.  */
#define NS32532
#define NS32381
