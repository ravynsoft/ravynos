// section_sorting_name.cc -- a test case for gold

// Copyright (C) 2013-2023 Free Software Foundation, Inc.
// Written by Alexander Ivchenko <alexander.ivchenko@intel.com>.

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

// The goal of this program is to verify that when using --sort-section=name
// option all .text, .data and .bss sections are sorted by name

extern "C"
__attribute__ ((section(".text.hot0001")))
int hot_foo_0001()
{
  return 1;
}

int vdata_0003  __attribute__((section(".data.0003"))) = 3;
int vbss_0003  __attribute__((section(".bss.0003"))) = 0;

extern "C"
__attribute__ ((section(".text.hot0003")))
int hot_foo_0003()
{
  return 1;
}

int vdata_0001  __attribute__((section(".data.0001"))) = 1;
int vbss_0001  __attribute__((section(".bss.0001"))) = 0;

extern "C"
__attribute__ ((section(".text.hot0002")))
int hot_foo_0002()
{
  return 1;
}

extern "C"
__attribute__ ((section(".text.sorted.0002")))
int sorted_foo_0002()
{
  return 1;
}

extern "C"
__attribute__ ((section(".text.sorted.0001.abc")))
int sorted_foo_0001_abc()
{
  return 1;
}


extern "C"
__attribute__ ((section(".text.sorted.0001")))
int sorted_foo_0001()
{
  return 1;
}

extern "C"
__attribute__ ((section(".text.sorted.0003")))
int sorted_foo_0003()
{
  return 1;
}

extern "C"
__attribute__ ((section(".text.sorted.z")))
int sorted_foo_z()
{
  return 1;
}

extern "C"
__attribute__ ((section(".text.sorted.y")))
int sorted_foo_y()
{
  return 1;
}

int vdata_0002  __attribute__((section(".data.0002"))) = 2;
int vbss_0002 __attribute__((section(".bss.0002"))) = 0;

int main()
{
  return 1;
}
