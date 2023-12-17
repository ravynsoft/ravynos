/* connect.c --- wrappers for Windows connect function

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

/* Get set_winsock_errno, FD_TO_SOCKET etc. */
#include "w32sock.h"

#undef connect

int
rpl_connect (int fd, const struct sockaddr *sockaddr, socklen_t len)
{
  SOCKET sock = FD_TO_SOCKET (fd);

  if (sock == INVALID_SOCKET)
    {
      errno = EBADF;
      return -1;
    }
  else
    {
      int r = connect (sock, sockaddr, len);
      if (r < 0)
        {
          /* EINPROGRESS is not returned by WinSock 2.0; for backwards
             compatibility, connect(2) uses EWOULDBLOCK.  */
          if (WSAGetLastError () == WSAEWOULDBLOCK)
            WSASetLastError (WSAEINPROGRESS);

          set_winsock_errno ();
        }

      return r;
    }
}
