/* Test of lseek() function.
   Copyright (C) 2007-2023 Free Software Foundation, Inc.

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

/* Written by Eric Blake, 2007.  */

#include <config.h>

#include <unistd.h>

#include "signature.h"
SIGNATURE_CHECK (lseek, off_t, (int, off_t, int));

#include <errno.h>

#include "macros.h"

/* ARGC must be 2; *ARGV[1] is '0' if stdin and stdout are files, '1'
   if they are pipes, and '2' if they are closed.  Check for proper
   semantics of lseek.  */
int
main (int argc, char **argv)
{
  if (argc != 2)
    return 2;
  switch (*argv[1])
    {
    case '0': /* regular files */
      ASSERT (lseek (0, (off_t)2, SEEK_SET) == 2);
      ASSERT (lseek (0, (off_t)-4, SEEK_CUR) == -1);
      ASSERT (errno == EINVAL);
      errno = 0;
#if ! defined __BEOS__
      /* POSIX says that the last lseek call, when failing, does not change
         the current offset.  But BeOS sets it to 0.  */
      ASSERT (lseek (0, (off_t)0, SEEK_CUR) == 2);
#endif
#if 0 /* leads to SIGSYS on IRIX 6.5 */
      ASSERT (lseek (0, (off_t)0, (SEEK_SET | SEEK_CUR | SEEK_END) + 1) == -1);
      ASSERT (errno == EINVAL);
#endif
      ASSERT (lseek (1, (off_t)2, SEEK_SET) == 2);
      errno = 0;
      ASSERT (lseek (1, (off_t)-4, SEEK_CUR) == -1);
      ASSERT (errno == EINVAL);
      errno = 0;
#if ! defined __BEOS__
      /* POSIX says that the last lseek call, when failing, does not change
         the current offset.  But BeOS sets it to 0.  */
      ASSERT (lseek (1, (off_t)0, SEEK_CUR) == 2);
#endif
#if 0 /* leads to SIGSYS on IRIX 6.5 */
      ASSERT (lseek (1, (off_t)0, (SEEK_SET | SEEK_CUR | SEEK_END) + 1) == -1);
      ASSERT (errno == EINVAL);
#endif
      break;

    case '1': /* pipes */
      errno = 0;
      ASSERT (lseek (0, (off_t)0, SEEK_CUR) == -1);
      ASSERT (errno == ESPIPE);
      errno = 0;
      ASSERT (lseek (1, (off_t)0, SEEK_CUR) == -1);
      ASSERT (errno == ESPIPE);
      break;

    case '2': /* closed */
      /* Explicitly close file descriptors 0 and 1.  The <&- and >&- in the
         invoking shell are not enough on HP-UX.  */
      close (0);
      close (1);

      errno = 0;
      ASSERT (lseek (0, (off_t)0, SEEK_CUR) == -1);
      ASSERT (errno == EBADF);

      errno = 0;
      ASSERT (lseek (1, (off_t)0, SEEK_CUR) == -1);
      ASSERT (errno == EBADF);

      /* Test behaviour for invalid file descriptors.  */
      errno = 0;
      ASSERT (lseek (-1, (off_t)0, SEEK_CUR) == -1);
      ASSERT (errno == EBADF);

      close (99);
      errno = 0;
      ASSERT (lseek (99, (off_t)0, SEEK_CUR) == -1);
      ASSERT (errno == EBADF);

      break;

    default:
      return 1;
    }
  return 0;
}
