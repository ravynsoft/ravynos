/* Test of <sys/socket.h> substitute.
   Copyright (C) 2007, 2009-2023 Free Software Foundation, Inc.

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

/* Written by Bruno Haible <bruno@clisp.org>, 2007.  */

#include <config.h>

#include <sys/socket.h>

#if HAVE_SHUTDOWN
/* Check some integer constant expressions.  */
int a[] = { SHUT_RD, SHUT_WR, SHUT_RDWR };
#endif

/* Check that the 'socklen_t' type is defined.  */
socklen_t t1;

/* Check that the 'size_t' and 'ssize_t' types are defined.  */
size_t t2;
ssize_t t3;

/* Check that 'struct iovec' is defined.  */
struct iovec io;

/* Check that a minimal set of 'struct msghdr' is defined.  */
struct msghdr msg;

#include <errno.h>

int
main (void)
{
  struct sockaddr_storage x;
  sa_family_t i;

  /* Check some errno values.  */
  switch (ENOTSOCK)
    {
    case ENOTSOCK:
    case EADDRINUSE:
    case ENETRESET:
    case ECONNABORTED:
    case ECONNRESET:
    case ENOTCONN:
    case ESHUTDOWN:
      break;
    }

  x.ss_family = 42;
  i = 42;
  msg.msg_iov = &io;

  return (x.ss_family - i + msg.msg_namelen + msg.msg_iov->iov_len
          + msg.msg_iovlen);
}
