/* DO NOT EDIT! GENERATED AUTOMATICALLY! */
/* Test the Unicode character type functions.
   Copyright (C) 2007-2022 Free Software Foundation, Inc.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published
   by the Free Software Foundation, either version 3 of the License,
   or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#include "test-predicate-part1.h"

    { 0x0009, 0x000D },
    { 0x0020, 0x0020 },
    { 0x1680, 0x1680 },
    { 0x2000, 0x2006 },
    { 0x2008, 0x200A },
    { 0x2028, 0x2029 },
    { 0x205F, 0x205F },
    { 0x3000, 0x3000 }

#define PREDICATE(c) uc_is_space (c)
#include "test-predicate-part2.h"
