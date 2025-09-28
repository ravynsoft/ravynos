/* ffsll.c -- version of ffsll for gold.  */

/* Copyright (C) 2009-2023 Free Software Foundation, Inc.
   Written by Ian Lance Taylor <iant@google.com>.

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

#include "config.h"

#include <string.h>

extern int ffsll (long long);

/* This file implements ffsll for systems which don't have it.  We use
   ffsll if possible because gcc supports it as a builtin which will
   use a machine instruction if there is one.  */

int
ffsll (long long arg)
{
  unsigned long long i;
  int ret;

  if (arg == 0)
    ret = 0;
  else
    {
      ret = 1;
      for (i = (unsigned long long) arg; (i & 1) == 0; i >>= 1)
	++ret;
    }
  return ret;
}
