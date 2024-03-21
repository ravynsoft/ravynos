/* Test the "ignore-value" module.

   Copyright (C) 2011-2023 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by Eric Blake.  */

#include <config.h>

#include "ignore-value.h"

#include <stdio.h>

#include "attribute.h"

struct s { int i; };
NODISCARD static char doChar (void);
NODISCARD static int doInt (void);
NODISCARD static off_t doOff (void);
NODISCARD static void *doPtr (void);
NODISCARD static struct s doStruct (void);

static char
doChar (void)
{
  return 0;
}

static int
doInt (void)
{
  return 0;
}

static off_t
doOff (void)
{
  return 0;
}

static void *
doPtr (void)
{
  return NULL;
}

static struct s
doStruct (void)
{
  static struct s s1;
  return s1;
}

int
main (void)
{
  /* If this test can compile with -Werror and the same warnings as
     the rest of the project, then we are properly silencing warnings
     about ignored return values.  */
  ignore_value (doChar ());
  ignore_value (doInt ());
  ignore_value (doOff ());
  ignore_value (doPtr ());
  ignore_value (doStruct ());
  return 0;
}
