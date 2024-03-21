// plugin_final_layout.cc -- a test case for gold

// Copyright (C) 2011-2023 Free Software Foundation, Inc.
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

// The goal of this program is to verify if section ordering
// via plugins happens correctly.  Also, test if plugin based ordering
// overrides default text section ordering where ".text.hot" sections
// are grouped.  The plugin does not want foo and baz next to each other.
// Plugin section order is foo() followed by bar() and then baz().

__attribute__ ((section(".text._Z3barv")))
void bar ()
{
}

__attribute__ ((section(".text.hot._Z3bazv")))
void baz ()
{
}

__attribute__ ((section(".text.hot._Z3foov")))
void foo ()
{
}

int main ()
{
  return 0;
}
