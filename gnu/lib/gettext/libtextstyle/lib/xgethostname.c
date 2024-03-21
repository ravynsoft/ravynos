/* xgethostname.c -- return current hostname with unlimited length

   Copyright (C) 1992, 1996, 2000-2001, 2003-2006, 2009-2023 Free Software
   Foundation, Inc.

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

/* written by Jim Meyering and Paul Eggert */

#include <config.h>

/* Specification.  */
#include "xgethostname.h"

#define GETANAME gethostname
#define XGETANAME xgethostname
#include "xgetaname-impl.h"
