/* Tests for opening a file without destroying an old file with the same name.

   Copyright (C) 2020-2023 Free Software Foundation, Inc.

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

/* Written by Bruno Haible, 2020.  */

#include <config.h>

/* Specification.  */
#include "supersede.h"

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "filenamecat.h"
#include "read-file.h"
#include "macros.h"

/* The name of the "always silent" device.  */
#if defined _WIN32 && ! defined __CYGWIN__
/* Native Windows API.  */
# define DEV_NULL "NUL"
#else
/* Unix API.  */
# define DEV_NULL "/dev/null"
#endif

#include "test-supersede-open.h"
#include "test-supersede-fopen.h"

int
main (void)
{
  test_open_supersede (false, false);
  test_open_supersede (false, true);
  test_open_supersede (true, false);
  test_open_supersede (true, true);

  test_fopen_supersede (false, false);
  test_fopen_supersede (false, true);
  test_fopen_supersede (true, false);
  test_fopen_supersede (true, true);

  return 0;
}
