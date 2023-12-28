// weak_alias_test_2.cc -- test weak aliases for gold

// Copyright (C) 2008-2023 Free Software Foundation, Inc.
// Written by Ian Lance Taylor <iant@google.com>.

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

// Define a strong symbol.

int strong_symbol = 2;

// Define a weak symbol.  This will be overridden by the weak symbol
// in weak_alias_test_1.cc.

extern int weak_symbol __attribute__ ((weak));
int weak_symbol = 3;

// These are overridden by weak_alias_test_1.cc
int strong_aliased = 100;
extern int weak_aliased __attribute__ ((weak, alias ("strong_aliased")));
int strong_aliased_2 = 102;
extern int weak_aliased_2 __attribute__ ((weak, alias ("strong_aliased_2")));
int strong_aliased_3 = 104;
extern int weak_aliased_3 __attribute__ ((weak, alias ("strong_aliased_3")));
int strong_aliased_4 = 106;
extern int weak_aliased_4 __attribute__ ((weak, alias ("strong_aliased_4")));
