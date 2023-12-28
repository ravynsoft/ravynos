// icf_string_merge_test.cc -- a test case for gold

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

// The goal of this program is to verify is strings are handled correctly
// by ICF.  ICF inlines strings that can be merged.  In some cases, the
// addend of the relocation pointing to a string merge section must be
// ignored.  This program has no pair of identical functions that can be
// folded.  However, if the addend is not ignored then get2 and get3 will
// become identical.

const char* const str1 = "aaaaaaaaaastr1";
const char* const str2 = "bbbbaaaaaastr1";
const char* const str3 = "cccccaaaaastr1";

const char* get1()
{
  return str1;
}
const char* get2()
{
  return str2;
}

const char* get3()
{
  return str3;
}
int main()
{
  return 0;
}
