// file_in_many_sections.c -- test STT_FILE when more than 64k sections

// Copyright (C) 2016-2023 Free Software Foundation, Inc.
// Written by Tristan Gingold <gingold@adacore.com>

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

// This program tests having many sections.  It uses a generated .h
// files to define 70,000 variables, each in a different section.  It
// uses another generated .h file to verify that they all have the
// right value.

#include "many_sections_define.h"

int
main (void)
{
  return 0;
}
