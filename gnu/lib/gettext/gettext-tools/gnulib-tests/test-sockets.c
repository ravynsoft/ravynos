/*
 * Copyright (C) 2008-2023 Free Software Foundation, Inc.
 * Written by Simon Josefsson.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#include <config.h>

#include <stdio.h>

#include "sockets.h"

int
main (void)
{
  int err;

  err = gl_sockets_startup (SOCKETS_1_1);
  if (err != 0)
    {
      printf ("wsastartup failed %d\n", err);
      return 1;
    }

  err = gl_sockets_cleanup ();
  if (err != 0)
    {
      printf ("wsacleanup failed %d\n", err);
      return 1;
    }

  (void) gl_fd_to_handle (0);

  return 0;
}
