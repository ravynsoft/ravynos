// icf_sht_rel_addend_test_2.cc -- a test case for gold

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
// by ICF when the relocation types are SHT_REL.  ICF inlines strings that
// can be merged.  To do this, it must get the addend of the relocation
// pointing to the string.  For SHT_REL relocations, the addend is encoded
// in the text section at the offset of the relocation.  If ICF fails to
// get the addend correctly, function name1 in icf_sht_rel_addend_test_1.cc
// will be incorrectly folded with name2.


const char* foo()
{
  return "AAAAAA";
}
const char* name2()
{
  return "Name2";
}
