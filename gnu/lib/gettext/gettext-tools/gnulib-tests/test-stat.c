/* Tests of stat.
   Copyright (C) 2009-2023 Free Software Foundation, Inc.

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

/* Written by Eric Blake <ebb9@byu.net>, 2009.  */

#include <config.h>

#include <sys/stat.h>

/* Caution: stat may be a function-like macro.  Although this
   signature check must pass, it may be the signature of the real (and
   broken) stat rather than rpl_stat.  Most code should not use the
   address of stat.  */
#include "signature.h"
SIGNATURE_CHECK (stat, int, (char const *, struct stat *));

#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "same-inode.h"
#include "macros.h"

#define BASE "test-stat.t"

#include "test-stat.h"

/* Wrapper around stat, which works even if stat is a function-like
   macro, where test_stat_func(stat) would do the wrong thing.  */
static int
do_stat (char const *name, struct stat *st)
{
  return stat (name, st);
}

int
main (void)
{
  return test_stat_func (do_stat, true);
}
