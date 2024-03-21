// strong_ref_weak_def_1.c -- test a strong reference to a weak definition
// in a DSO.

// Copyright (C) 2010-2023 Free Software Foundation, Inc.
// Written by Doug Kwan <dougkwan@google.com>.

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

// We test that we correctly deal with a non-weak reference to
// a weak symbol in an DSO.  We need to make sure that reference
// is not turned into a weak one.

// This source is used to build an executable that references a weak
// symbol in a DSO.

// Strong reference to a weak symbol.
extern void weak_def (void);

int
main (void)
{
  weak_def ();
  return 0;
}
