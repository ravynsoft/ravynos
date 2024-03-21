/* Test of <errno.h> substitute.
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

/* Written by Bruno Haible <bruno@clisp.org>, 2008.  */

#include <config.h>

#include <errno.h>

/* Verify that the POSIX mandated errno values exist and can be used as
   initializers outside of a function.
   The variable names happen to match the Linux/x86 error numbers.  */
int e1 = EPERM;
int e2 = ENOENT;
int e3 = ESRCH;
int e4 = EINTR;
int e5 = EIO;
int e6 = ENXIO;
int e7 = E2BIG;
int e8 = ENOEXEC;
int e9 = EBADF;
int e10 = ECHILD;
int e11 = EAGAIN;
int e11a = EWOULDBLOCK;
int e12 = ENOMEM;
int e13 = EACCES;
int e14 = EFAULT;
int e16 = EBUSY;
int e17 = EEXIST;
int e18 = EXDEV;
int e19 = ENODEV;
int e20 = ENOTDIR;
int e21 = EISDIR;
int e22 = EINVAL;
int e23 = ENFILE;
int e24 = EMFILE;
int e25 = ENOTTY;
int e26 = ETXTBSY;
int e27 = EFBIG;
int e28 = ENOSPC;
int e29 = ESPIPE;
int e30 = EROFS;
int e31 = EMLINK;
int e32 = EPIPE;
int e33 = EDOM;
int e34 = ERANGE;
int e35 = EDEADLK;
int e36 = ENAMETOOLONG;
int e37 = ENOLCK;
int e38 = ENOSYS;
int e39 = ENOTEMPTY;
int e40 = ELOOP;
int e42 = ENOMSG;
int e43 = EIDRM;
int e67 = ENOLINK;
int e71 = EPROTO;
int e72 = EMULTIHOP;
int e74 = EBADMSG;
int e75 = EOVERFLOW;
int e84 = EILSEQ;
int e88 = ENOTSOCK;
int e89 = EDESTADDRREQ;
int e90 = EMSGSIZE;
int e91 = EPROTOTYPE;
int e92 = ENOPROTOOPT;
int e93 = EPROTONOSUPPORT;
int e95 = EOPNOTSUPP;
int e95a = ENOTSUP;
int e97 = EAFNOSUPPORT;
int e98 = EADDRINUSE;
int e99 = EADDRNOTAVAIL;
int e100 = ENETDOWN;
int e101 = ENETUNREACH;
int e102 = ENETRESET;
int e103 = ECONNABORTED;
int e104 = ECONNRESET;
int e105 = ENOBUFS;
int e106 = EISCONN;
int e107 = ENOTCONN;
int e110 = ETIMEDOUT;
int e111 = ECONNREFUSED;
int e113 = EHOSTUNREACH;
int e114 = EALREADY;
int e115 = EINPROGRESS;
int e116 = ESTALE;
int e122 = EDQUOT;
int e125 = ECANCELED;
int e130 = EOWNERDEAD;
int e131 = ENOTRECOVERABLE;

/* Don't verify that these errno values are all different, except for possibly
   EWOULDBLOCK == EAGAIN.  Even Linux/x86 does not pass this check: it has
   ENOTSUP == EOPNOTSUPP.  */

int
main ()
{
  /* Verify that errno can be assigned.  */
  errno = EOVERFLOW;

  /* snprintf() callers want to distinguish EINVAL and EOVERFLOW.  */
  if (errno == EINVAL)
    return 1;

  return 0;
}
