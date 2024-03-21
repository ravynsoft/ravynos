/* socket.c --- wrappers for Windows socket function

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

#include "sockets.h"

/* Don't assume that UNICODE is defined.  */
#undef WSASocket
#define WSASocket WSASocketW

int
rpl_socket (int domain, int type, int protocol)
{
  SOCKET fh;

  gl_sockets_startup (SOCKETS_1_1);

  /* We have to use WSASocket() to create non-overlapped IO sockets.
     Overlapped IO sockets cannot be used with read/write.  */
  fh = WSASocket (domain, type, protocol, NULL, 0, 0);

  if (fh == INVALID_SOCKET)
    {
      set_winsock_errno ();
      return -1;
    }
  else
    return SOCKET_TO_FD (fh);
}
