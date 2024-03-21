/* common_test_3.c -- test common symbol name conflicts

   Copyright (C) 2009-2023 Free Software Foundation, Inc.
   Written by Ian Lance Taylor <iant@google.com>

   This file is part of gold.

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

/* Define a function with a default version whose name is the same as
   a common symbol.  This file will wind up in a shared library.  */

void c1_v1 (void);

void
c1_v1 (void)
{
}
__asm__ (".symver c1_v1,c1@@VER1");
