/* setsockopt.c --- wrappers for Windows setsockopt function

   Copyright (C) 2008-2023 Free Software Foundation, Inc.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by Paolo Bonzini */

#include <config.h>

#define WIN32_LEAN_AND_MEAN
/* Get winsock2.h. */
#include <sys/socket.h>

/* Get struct timeval. */
#include <sys/time.h>

/* Get set_winsock_errno, FD_TO_SOCKET etc. */
#include "w32sock.h"

#undef setsockopt

int
rpl_setsockopt (int fd, int level, int optname, const void *optval, socklen_t optlen)
{
  SOCKET sock = FD_TO_SOCKET (fd);
  int r;

  if (sock == INVALID_SOCKET)
    {
      errno = EBADF;
      return -1;
    }
  else
    {
      if (level == SOL_SOCKET
          && (optname == SO_RCVTIMEO || optname == SO_SNDTIMEO))
        {
          const struct timeval *tv = optval;
          int milliseconds = tv->tv_sec * 1000 + tv->tv_usec / 1000;
          optval = &milliseconds;
          r = setsockopt (sock, level, optname, optval, sizeof (int));
        }
      else
        {
          r = setsockopt (sock, level, optname, optval, optlen);
        }

      if (r < 0)
        set_winsock_errno ();

      return r;
    }
}
