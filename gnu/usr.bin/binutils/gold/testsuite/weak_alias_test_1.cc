// weak_alias_test_1.cc -- test weak aliases for gold

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

// Define a weak symbol.

extern int weak_symbol __attribute__ ((weak));
int weak_symbol = 1;

// Define a strong symbol with a weak alias.

int strong_aliased = 2;
extern int weak_aliased __attribute__ ((weak, alias ("strong_aliased")));

// And another one.

int strong_aliased_2 = 5;
extern int weak_aliased_2 __attribute__ ((weak, alias ("strong_aliased_2")));

// And a third.

int strong_aliased_3 = 7;
extern int weak_aliased_3 __attribute__ ((weak, alias ("strong_aliased_3")));

// And a fourth.

int strong_aliased_4 = 8;
extern int weak_aliased_4 __attribute__ ((weak, alias ("strong_aliased_4")));

// We want a symbol whose name is the same length as "strong_symbol",
// so that weak_symbol here lines up with weak_symbol in
// weak_alias_test_2.so.

int Strong_Symbol = 101;
