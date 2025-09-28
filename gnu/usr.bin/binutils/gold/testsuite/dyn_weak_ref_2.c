// dyn_weak_ref_1.c -- test that a weak ref remains weak in output when
// there is a DSO with the same weak ref.

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

// We test that we correctly deal with a weak reference to from both
// a DSO and a weak reference to the same symbol in an executable.  The
// symbol should remains weak.

// This source is used to build a DSO that contains a weak reference.

extern void weak_ref (void) __attribute__((weak));

void* ptr2 = weak_ref;
