// gdb_index_test.c -- a test case for the --gdb-index option.

// Copyright (C) 2012-2023 Free Software Foundation, Inc.

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

// This source file is just a simple C source file that is mainly to
// test the CU DW_AT_high_pc FORM encoding is handled correctly by the
// DWARF scanner in gold.

int check_int (int);
int main (void);

int j = 0;

int
check_int (int i)
{ return i > 0; }

int
main()
{
  return check_int (0);
}
