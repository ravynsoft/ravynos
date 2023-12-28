// gc_orphan_section_test.cc -- a test case for gold

// Copyright (C) 2010-2023 Free Software Foundation, Inc.
// Written by Sriraman Tallam <tmsriram@google.com>.

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

// The goal of this program is to verify if garbage collection does not
// discard orphan sections when references to them through __start_XXX
// and __stop_XXX are present.  Here section _foo must not be gc'ed but
// _boo should be gc'ed.

extern const int *__start__foo;
int foo __attribute__((__section__("_foo"))) = 1;
int boo __attribute__((__section__("_boo"))) = 1;

int main()
{
  return *__start__foo;
}

