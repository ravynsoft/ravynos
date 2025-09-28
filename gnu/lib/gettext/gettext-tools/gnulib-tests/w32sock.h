/* w32sock.h --- internal auxiliary functions for Windows socket functions

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

#include <errno.h>

/* Get O_RDWR and O_BINARY.  */
#include <fcntl.h>

/* Get _open_osfhandle().  */
#include <io.h>

/* Get _get_osfhandle().  */
#if GNULIB_MSVC_NOTHROW
# include "msvc-nothrow.h"
#else
# include <io.h>
#endif

#define FD_TO_SOCKET(fd)   ((SOCKET) _get_osfhandle ((fd)))
#define SOCKET_TO_FD(fh)   (_open_osfhandle ((intptr_t) (fh), O_RDWR | O_BINARY))

static inline void
set_winsock_errno (void)
{
  int err = WSAGetLastError ();

  /* Map some WSAE* errors to the runtime library's error codes.  */
  switch (err)
    {
    case WSA_INVALID_HANDLE:
      errno = EBADF;
      break;
    case WSA_NOT_ENOUGH_MEMORY:
      errno = ENOMEM;
      break;
    case WSA_INVALID_PARAMETER:
      errno = EINVAL;
      break;
    case WSAENAMETOOLONG:
      errno = ENAMETOOLONG;
      break;
    case WSAENOTEMPTY:
      errno = ENOTEMPTY;
      break;
    case WSAEWOULDBLOCK:
      errno = EWOULDBLOCK;
      break;
    case WSAEINPROGRESS:
      errno = EINPROGRESS;
      break;
    case WSAEALREADY:
      errno = EALREADY;
      break;
    case WSAENOTSOCK:
      errno = ENOTSOCK;
      break;
    case WSAEDESTADDRREQ:
      errno = EDESTADDRREQ;
      break;
    case WSAEMSGSIZE:
      errno = EMSGSIZE;
      break;
    case WSAEPROTOTYPE:
      errno = EPROTOTYPE;
      break;
    case WSAENOPROTOOPT:
      errno = ENOPROTOOPT;
      break;
    case WSAEPROTONOSUPPORT:
      errno = EPROTONOSUPPORT;
      break;
    case WSAEOPNOTSUPP:
      errno = EOPNOTSUPP;
      break;
    case WSAEAFNOSUPPORT:
      errno = EAFNOSUPPORT;
      break;
    case WSAEADDRINUSE:
      errno = EADDRINUSE;
      break;
    case WSAEADDRNOTAVAIL:
      errno = EADDRNOTAVAIL;
      break;
    case WSAENETDOWN:
      errno = ENETDOWN;
      break;
    case WSAENETUNREACH:
      errno = ENETUNREACH;
      break;
    case WSAENETRESET:
      errno = ENETRESET;
      break;
    case WSAECONNABORTED:
      errno = ECONNABORTED;
      break;
    case WSAECONNRESET:
      errno = ECONNRESET;
      break;
    case WSAENOBUFS:
      errno = ENOBUFS;
      break;
    case WSAEISCONN:
      errno = EISCONN;
      break;
    case WSAENOTCONN:
      errno = ENOTCONN;
      break;
    case WSAETIMEDOUT:
      errno = ETIMEDOUT;
      break;
    case WSAECONNREFUSED:
      errno = ECONNREFUSED;
      break;
    case WSAELOOP:
      errno = ELOOP;
      break;
    case WSAEHOSTUNREACH:
      errno = EHOSTUNREACH;
      break;
    default:
      errno = (err > 10000 && err < 10025) ? err - 10000 : err;
      break;
    }
}
