/* tls_test_c.c -- test TLS common symbol

   Copyright (C) 2008-2023 Free Software Foundation, Inc.
   Written by Ian Lance Taylor <iant@google.com>

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

/* The only way I know to get gcc to generate a TLS common symbol is
   to use a C file and an OpenMP directive.  */

#include "config.h"
#include <stdio.h>

#define CHECK_EQ_OR_RETURN(var, expected)				\
  do									\
    {									\
      if ((var) != (expected))						\
	{								\
	  printf(#var ": expected %d, found %d\n", expected, var);	\
	  return 0;							\
	}								\
    }									\
  while (0)

#ifdef HAVE_OMP_SUPPORT
int v7;
#pragma omp threadprivate (v7)
#endif

int t11(void);
int t11_last(void);

int
t11(void)
{
#ifdef HAVE_OMP_SUPPORT
  CHECK_EQ_OR_RETURN(v7, 0);
  v7 = 70;
#endif
  return 1;
}

int
t11_last(void)
{
#ifdef HAVE_OMP_SUPPORT
  CHECK_EQ_OR_RETURN(v7, 70);
#endif
  return 1;
}
