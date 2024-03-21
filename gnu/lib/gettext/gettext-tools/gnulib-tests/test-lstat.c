/* Test of lstat() function.
   Copyright (C) 2008-2023 Free Software Foundation, Inc.

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

/* Written by Simon Josefsson, 2008; and Eric Blake, 2009.  */

#include <config.h>

#include <sys/stat.h>

/* Caution: lstat may be a function-like macro.  Although this
   signature check must pass, it may be the signature of the real (and
   broken) lstat rather than rpl_lstat.  Most code should not use the
   address of lstat.  */
#include "signature.h"
SIGNATURE_CHECK (lstat, int, (char const *, struct stat *));

#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "same-inode.h"
#include "ignore-value.h"
#include "macros.h"

#define BASE "test-lstat.t"

#include "test-lstat.h"

/* Wrapper around lstat, which works even if lstat is a function-like
   macro, where test_lstat_func(lstat) would do the wrong thing.  */
static int
do_lstat (char const *name, struct stat *st)
{
  return lstat (name, st);
}

int
main (void)
{
  /* Remove any leftovers from a previous partial run.  */
  ignore_value (system ("rm -rf " BASE "*"));

  return test_lstat_func (do_lstat, true);
}
