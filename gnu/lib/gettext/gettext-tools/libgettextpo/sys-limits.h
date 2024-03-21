/* System call limits

   Copyright 2018-2023 Free Software Foundation, Inc.

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

#ifndef _GL_SYS_LIMITS_H
#define _GL_SYS_LIMITS_H

#include <limits.h>

/* Maximum number of bytes to read or write in a single system call.
   This can be useful for system calls like sendfile on GNU/Linux,
   which do not handle more than MAX_RW_COUNT bytes correctly.
   The Linux kernel MAX_RW_COUNT is at least INT_MAX >> 20 << 20,
   where the 20 comes from the Hexagon port with 1 MiB pages; use that
   as an approximation, as the exact value may not be available to us.

   Using this also works around a serious Linux bug before 2.6.16; see
   <https://bugzilla.redhat.com/show_bug.cgi?id=612839>.

   Using this also works around a Tru64 5.1 bug, where attempting
   to read INT_MAX bytes fails with errno == EINVAL.  See
   <https://lists.gnu.org/r/bug-gnu-utils/2002-04/msg00010.html>.

   Using this is likely to work around similar bugs in other operating
   systems.  */

enum { SYS_BUFSIZE_MAX = INT_MAX >> 20 << 20 };

#endif
