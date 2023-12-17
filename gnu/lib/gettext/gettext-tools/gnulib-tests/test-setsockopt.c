/* Test setsockopt() function.
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

#include <config.h>

#include <sys/socket.h>

#include "signature.h"
SIGNATURE_CHECK (setsockopt, int, (int, int, int, const void *, socklen_t));

#include <errno.h>
#include <unistd.h>

#include "sockets.h"
#include "macros.h"

int
main (void)
{
  (void) gl_sockets_startup (SOCKETS_1_1);

  /* Test behaviour for invalid file descriptors.  */
  {
    int value = 1;

    errno = 0;
    ASSERT (setsockopt (-1, SOL_SOCKET, SO_REUSEADDR, &value, sizeof (value))
            == -1);
    ASSERT (errno == EBADF);
  }
  {
    int value = 1;

    close (99);
    errno = 0;
    ASSERT (setsockopt (99, SOL_SOCKET, SO_REUSEADDR, &value, sizeof (value))
            == -1);
    ASSERT (errno == EBADF);
  }

  return 0;
}
