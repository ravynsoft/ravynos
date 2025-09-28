/* plugin_pr22868_a.c -- a test case for the plugin API with GC.

   Copyright (C) 2018-2023 Free Software Foundation, Inc.
   Written by Cary Coutant <ccoutant@gmail.com>.

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
   MA 02110-1301, USA.  */

int foo(int i) __attribute__ (( weak ));

int foo(int i)
{
  return i + 1;
}
