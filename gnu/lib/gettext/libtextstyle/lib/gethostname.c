/* gethostname emulation for SysV and POSIX.1.

   Copyright (C) 1992, 2003, 2006, 2008-2023 Free Software Foundation, Inc.

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

/* David MacKenzie <djm@gnu.ai.mit.edu>
   Windows port by Simon Josefsson <simon@josefsson.org> */

#include <config.h>

#if !(defined _WIN32 && !defined __CYGWIN__)
/* Unix API.  */

/* Specification.  */
#include <unistd.h>

#ifdef HAVE_UNAME
# include <sys/utsname.h>
#endif

#include <string.h>

/* Put up to LEN chars of the host name into NAME.
   Null terminate it if the name is shorter than LEN.
   Return 0 if ok, -1 if error.  */

#include <stddef.h>

int
gethostname (char *name, size_t len)
{
#ifdef HAVE_UNAME
  struct utsname uts;

  if (uname (&uts) == -1)
    return -1;
  if (len > sizeof (uts.nodename))
    {
      /* More space than we need is available.  */
      name[sizeof (uts.nodename)] = '\0';
      len = sizeof (uts.nodename);
    }
  strncpy (name, uts.nodename, len);
#else
  strcpy (name, "");            /* Hardcode your system name if you want.  */
#endif
  return 0;
}

#else
/* Native Windows API.  Which primitive to choose?
   - gethostname() requires linking with -lws2_32.
   - GetComputerName() does not return the right kind of hostname.
   - GetComputerNameEx(ComputerNameDnsHostname,...) returns the right hostname,
     but it is hard to use portably:
       - It requires defining _WIN32_WINNT to at least 0x0500.
       - With mingw, it also requires
         "#define GetComputerNameEx GetComputerNameExA".
       - With older versions of mingw, none of the declarations are present at
         all, not even of the enum value ComputerNameDnsHostname.
   So we use gethostname().  Linking with -lws2_32 is the least evil.  */

#define WIN32_LEAN_AND_MEAN
/* Get winsock2.h. */
#include <unistd.h>

/* Get INT_MAX.  */
#include <limits.h>

/* Get set_winsock_errno. */
#include "w32sock.h"

#include "sockets.h"

#undef gethostname

int
rpl_gethostname (char *name, size_t len)
{
  int r;

  if (len > INT_MAX)
    len = INT_MAX;
  gl_sockets_startup (SOCKETS_1_1);
  r = gethostname (name, (int) len);
  if (r < 0)
    set_winsock_errno ();

  return r;
}

#endif
