/* pr20976.c -- test forced common allocation

   Copyright (C) 2016-2023 Free Software Foundation, Inc.
   Written by Cary Coutant <ccoutant@gmail.com>

   This file is part of gold.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.

   This test checks that forced common allocation (-d) with -r
   produces the correct result when the .bss section contains
   other allocated data besides common symbols. */

int a = 0;
int b;

int main(void)
{
  a = 1;
  return b;
}
