// plugin_layout_with_alignment.cc -- a test case for gold

// Copyright (C) 2016-2023 Free Software Foundation, Inc.
// Written by Than McIntosh <thanm@google.com>.

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

// Verify that plugin interfaces for section size and alignment work
// correctly, and that section ordering via plugins is working
// for .bss/.rodata/.data sections.

// --- Initialized .rodata items

__attribute__ ((section(".rodata.v1_a2"), aligned(2)))
const short rodata_item1 = 101;

__attribute__ ((section(".rodata.v2_a1"), aligned(1)))
const char rodata_item2 = 'a';

__attribute__ ((section(".rodata.v3_a8"), aligned(8)))
const double rodata_item3 = 777.777;

__attribute__ ((section(".rodata.v4_a1"), aligned(1)))
const char rodata_item4[7] = {'1', '2', '3', '4', '5', '6', '7'};

// --- Initialized .data items

__attribute__ ((section(".data.v1_a2"), aligned(2)))
short rwdata_item1 = 101;

__attribute__ ((section(".data.v2_a1"), aligned(1)))
char rwdata_item2 = 'a';

__attribute__ ((section(".data.v3_a8"), aligned(8)))
double rwdata_item3 = 'b';

__attribute__ ((section(".data.v4_a1"), aligned(1)))
char rwdata_item4[3] = {'a', 'b', 'c'};

// --- Uninitialized .data items

__attribute__ ((section(".bss.v1_a2"), aligned(2)))
short bss_item1;

__attribute__ ((section(".bss.v2_a1"), aligned(1)))
char bss_item2;

__attribute__ ((section(".bss.v3_a8"), aligned(8)))
struct blah { union { double d; char c; } u; } bss_item3;

__attribute__ ((section(".bss.v4_a1"), aligned(1)))
char bss_item4[3];

int main (void)
{
  return 0;
}
