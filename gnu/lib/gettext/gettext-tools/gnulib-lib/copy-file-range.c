/* Stub for copy_file_range
   Copyright 2019-2023 Free Software Foundation, Inc.

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

#include <config.h>

#include <unistd.h>

#include <errno.h>

#if defined __linux__ && HAVE_COPY_FILE_RANGE
# include <sys/utsname.h>
#endif

ssize_t
copy_file_range (int infd, off_t *pinoff,
                 int outfd, off_t *poutoff,
                 size_t length, unsigned int flags)
{
#undef copy_file_range

#if defined __linux__ && HAVE_COPY_FILE_RANGE
  /* The implementation of copy_file_range (which first appeared in
     Linux kernel release 4.5) had many issues before release 5.3
     <https://lwn.net/Articles/789527/>, so fail with ENOSYS for Linux
     kernels 5.2 and earlier.

     This workaround, and the configure-time check for Linux, can be
     removed when such kernels (released March 2016 through September
     2019) are no longer a consideration.  As of January 2021, the
     furthest-future planned kernel EOL is December 2024 for kernel
     release 4.19.  */

    static signed char ok;

    if (! ok)
      {
        struct utsname name;
        uname (&name);
        char *p = name.release;
        ok = ((p[1] != '.' || '5' < p[0]
               || (p[0] == '5' && (p[3] != '.' || '2' < p[2])))
              ? 1 : -1);
      }

    if (0 < ok)
      return copy_file_range (infd, pinoff, outfd, poutoff, length, flags);
#endif

  /* There is little need to emulate copy_file_range with read+write,
     since programs that use copy_file_range must fall back on
     read+write anyway.  */
  errno = ENOSYS;
  return -1;
}
