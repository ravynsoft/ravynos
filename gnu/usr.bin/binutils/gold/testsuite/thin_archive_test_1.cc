// thin_archive_test_1.cc -- part of a test case for thin archives

// Copyright (C) 2008-2023 Free Software Foundation, Inc.
// Written by Cary Coutant <ccoutant@google.com>.

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

// This tests references between files and archives.  This is file 1 of 4.
// Each of the first three files contains a reference to the next.
// We test the archives as follows:

// Files 1 and 2 in libthin1.a, files 3 and 4 in libthin2.a.
// Files 1 and 4 in libthin3.a, files 2 and 3 in libthin4.a, with
// libthin3.a and libthin4.a nested inside libthinall.a.

extern int t2();

int
t1()
{
  return (t2() << 4) | 1;
}
