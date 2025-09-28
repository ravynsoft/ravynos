// text_section_grouping.cc -- a test case for gold

// Copyright (C) 2012-2023 Free Software Foundation, Inc.
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

// The goal of this program is to verify if .text sections are grouped
// according to prefix.  .text.unlikely, .text.startup and .text.hot should
// be grouped and placed together.

extern "C"
__attribute__ ((section(".text.hot.foo")))
int hot_foo()
{
  return 1;
}

extern "C"
__attribute__ ((section(".text.startup.foo")))
int startup_foo()
{
  return 1;
}

extern "C"
__attribute__ ((section(".text.unlikely.foo")))
int unlikely_foo()
{
  return 1;
}

extern "C"
__attribute__ ((section(".text.hot.bar")))
int hot_bar()
{
  return 1;
}

extern "C"
__attribute__ ((section(".text.startup.bar")))
int startup_bar()
{
  return 1;
}

extern "C"
__attribute__ ((section(".text.unlikely.bar")))
int unlikely_bar()
{
  return 1;
}

int main()
{
  return 1;
}
