/* Test of <sys/uio.h> substitute.
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

/* Written by Eric Blake <eblake@redhat.com>, 2011.  */

#include <config.h>

#include <sys/uio.h>

/* Check that necessary types are defined.  */
size_t a;
ssize_t b;
struct iovec c;

int
main (void)
{
  return a + b + !!c.iov_base + c.iov_len;
}
