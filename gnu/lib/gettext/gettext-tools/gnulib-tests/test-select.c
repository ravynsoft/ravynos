/* Test of select() substitute.
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

/* Written by Paolo Bonzini, 2008.  */

#include <config.h>

#include <sys/select.h>

#include "signature.h"

SIGNATURE_CHECK (select, int, (int, fd_set *, fd_set *, fd_set *,
                               struct timeval *));

#define TEST_PORT 12346
#include "test-select.h"

int
main (void)
{
  return test_function (select);
}
