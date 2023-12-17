/* Test of execution of file name canonicalization.
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

/* Written by Bruno Haible <bruno@clisp.org>, 2007.  */

/* Don't use __attribute__ __nonnull__ in this compilation unit.  Otherwise gcc
   may "optimize" the null_ptr function, when its result gets passed to a
   function that has an argument declared as _GL_ARG_NONNULL.  */
#define _GL_ARG_NONNULL(params)

#include <config.h>

#include "canonicalize.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "same-inode.h"
#include "ignore-value.h"

#if GNULIB_defined_canonicalize_file_name
# include "null-ptr.h"
#endif

#include "macros.h"

#define BASE "t-can.tmp"

int
main (void)
{
  /* Setup some hierarchy to be used by this test.  Start by removing
     any leftovers from a previous partial run.  */
  {
    int fd;
    ignore_value (system ("rm -rf " BASE " ise"));
    ASSERT (mkdir (BASE, 0700) == 0);
    fd = creat (BASE "/tra", 0600);
    ASSERT (0 <= fd);
    ASSERT (close (fd) == 0);
  }

  /* Check // handling (the easy cases, without symlinks).
     This // handling is not mandated by POSIX.  However, many applications
     expect that canonicalize_filename_mode "canonicalizes" the file name,
     that is, that different results of canonicalize_filename_mode correspond
     to different files (except for hard links).  */
  {
    char *result0 = canonicalize_file_name ("/etc/passwd");
    if (result0 != NULL) /* This file does not exist on native Windows.  */
      {
        char *result;

        result = canonicalize_filename_mode ("/etc/passwd", CAN_MISSING);
        ASSERT (result != NULL && strcmp (result, result0) == 0);

        result = canonicalize_filename_mode ("/etc//passwd", CAN_MISSING);
        ASSERT (result != NULL && strcmp (result, result0) == 0);

        result = canonicalize_filename_mode ("/etc///passwd", CAN_MISSING);
        ASSERT (result != NULL && strcmp (result, result0) == 0);

        /* On Windows, the syntax //host/share/filename denotes a file
           in a directory named 'share', exported from host 'host'.
           See also m4/double-slash-root.m4.  */
#if !(defined _WIN32 || defined __CYGWIN__)
        result = canonicalize_filename_mode ("//etc/passwd", CAN_MISSING);
        ASSERT (result != NULL && strcmp (result, result0) == 0);

        result = canonicalize_filename_mode ("//etc//passwd", CAN_MISSING);
        ASSERT (result != NULL && strcmp (result, result0) == 0);

        result = canonicalize_filename_mode ("//etc///passwd", CAN_MISSING);
        ASSERT (result != NULL && strcmp (result, result0) == 0);
#endif

        result = canonicalize_filename_mode ("///etc/passwd", CAN_MISSING);
        ASSERT (result != NULL && strcmp (result, result0) == 0);

        result = canonicalize_filename_mode ("///etc//passwd", CAN_MISSING);
        ASSERT (result != NULL && strcmp (result, result0) == 0);

        result = canonicalize_filename_mode ("///etc///passwd", CAN_MISSING);
        ASSERT (result != NULL && strcmp (result, result0) == 0);
      }
  }

  /* Check for ., .., intermediate // handling, and for error cases.  */
  {
    char *result1 = canonicalize_file_name (BASE "//./..//" BASE "/tra");
    char *result2 = canonicalize_filename_mode (BASE "//./..//" BASE "/tra",
                                                CAN_EXISTING);
    ASSERT (result1 != NULL);
    ASSERT (result2 != NULL);
    ASSERT (strcmp (result1, result2) == 0);
    ASSERT (strstr (result1, "/" BASE "/tra")
            == result1 + strlen (result1) - strlen ("/" BASE "/tra"));
    free (result1);
    free (result2);

    errno = 0;
    result1 = canonicalize_file_name ("");
    ASSERT (result1 == NULL);
    ASSERT (errno == ENOENT);

    errno = 0;
    result2 = canonicalize_filename_mode ("", CAN_EXISTING);
    ASSERT (result2 == NULL);
    ASSERT (errno == ENOENT);

    /* This test works only if the canonicalize_file_name implementation
       comes from gnulib.  If it comes from libc, we have no way to prevent
       gcc from "optimizing" the null_ptr function in invalid ways.  See
       <https://gcc.gnu.org/bugzilla/show_bug.cgi?id=93156>.  */
#if GNULIB_defined_canonicalize_file_name
    errno = 0;
    result1 = canonicalize_file_name (null_ptr ());
    ASSERT (result1 == NULL);
    ASSERT (errno == EINVAL);
#endif

    errno = 0;
    result2 = canonicalize_filename_mode (NULL, CAN_EXISTING);
    ASSERT (result2 == NULL);
    ASSERT (errno == EINVAL);

    errno = 0;
    result2 = canonicalize_filename_mode (".", CAN_MISSING | CAN_ALL_BUT_LAST);
    ASSERT (result2 == NULL);
    ASSERT (errno == EINVAL);
  }

  /* Check that a non-directory with trailing slash yields NULL.  */
  {
    char *result1;
    char *result2;
    errno = 0;
    result1 = canonicalize_file_name (BASE "/tra/");
    ASSERT (result1 == NULL);
    ASSERT (errno == ENOTDIR);
    errno = 0;
    result2 = canonicalize_filename_mode (BASE "/tra/", CAN_EXISTING);
    ASSERT (result2 == NULL);
    ASSERT (errno == ENOTDIR);
  }

  /* Check that a missing directory yields NULL.  */
  {
    char *result1;
    char *result2;
    errno = 0;
    result1 = canonicalize_file_name (BASE "/zzz/..");
    ASSERT (result1 == NULL);
    ASSERT (errno == ENOENT);
    errno = 0;
    result2 = canonicalize_filename_mode (BASE "/zzz/..", CAN_EXISTING);
    ASSERT (result2 == NULL);
    ASSERT (errno == ENOENT);
  }

  /* From here on out, tests involve symlinks.  */
  if (symlink (BASE "/ket", "ise") != 0)
    {
      ASSERT (remove (BASE "/tra") == 0);
      ASSERT (rmdir (BASE) == 0);
      fputs ("skipping test: symlinks not supported on this file system\n",
             stderr);
      return 77;
    }
  ASSERT (symlink ("bef", BASE "/plo") == 0);
  ASSERT (symlink ("tra", BASE "/huk") == 0);
  ASSERT (symlink ("lum", BASE "/bef") == 0);
  ASSERT (symlink ("wum", BASE "/ouk") == 0);
  ASSERT (symlink ("../ise", BASE "/ket") == 0);
  ASSERT (mkdir (BASE "/lum", 0700) == 0);
  ASSERT (symlink ("s", BASE "/p") == 0);
  ASSERT (symlink ("d", BASE "/s") == 0);
  ASSERT (mkdir (BASE "/d", 0700) == 0);
  ASSERT (close (creat (BASE "/d/2", 0600)) == 0);
  ASSERT (symlink ("../s/2", BASE "/d/1") == 0);
  ASSERT (symlink ("//.//../..", BASE "/droot") == 0);

  /* Check that symbolic links are not resolved, with CAN_NOLINKS.  */
  {
    char *result1 = canonicalize_filename_mode (BASE "/huk", CAN_NOLINKS);
    ASSERT (result1 != NULL);
    ASSERT (strcmp (result1 + strlen (result1) - strlen ("/" BASE "/huk"),
                    "/" BASE "/huk") == 0);
    free (result1);
  }

  /* Check that the symbolic link to a file can be resolved.  */
  {
    char *result1 = canonicalize_file_name (BASE "/huk");
    char *result2 = canonicalize_file_name (BASE "/tra");
    char *result3 = canonicalize_filename_mode (BASE "/huk", CAN_EXISTING);
    ASSERT (result1 != NULL);
    ASSERT (result2 != NULL);
    ASSERT (result3 != NULL);
    ASSERT (strcmp (result1, result2) == 0);
    ASSERT (strcmp (result2, result3) == 0);
    ASSERT (strcmp (result1 + strlen (result1) - strlen ("/" BASE "/tra"),
                    "/" BASE "/tra") == 0);
    free (result1);
    free (result2);
    free (result3);
  }

  /* Check that the symbolic link to a directory can be resolved.  */
  {
    char *result1 = canonicalize_file_name (BASE "/plo");
    char *result2 = canonicalize_file_name (BASE "/bef");
    char *result3 = canonicalize_file_name (BASE "/lum");
    char *result4 = canonicalize_filename_mode (BASE "/plo", CAN_EXISTING);
    ASSERT (result1 != NULL);
    ASSERT (result2 != NULL);
    ASSERT (result3 != NULL);
    ASSERT (result4 != NULL);
    ASSERT (strcmp (result1, result2) == 0);
    ASSERT (strcmp (result2, result3) == 0);
    ASSERT (strcmp (result3, result4) == 0);
    ASSERT (strcmp (result1 + strlen (result1) - strlen ("/" BASE "/lum"),
                    "/" BASE "/lum") == 0);
    free (result1);
    free (result2);
    free (result3);
    free (result4);
  }

  /* Check that a symbolic link to a nonexistent file yields NULL.  */
  {
    char *result1;
    char *result2;
    errno = 0;
    result1 = canonicalize_file_name (BASE "/ouk");
    ASSERT (result1 == NULL);
    ASSERT (errno == ENOENT);
    errno = 0;
    result2 = canonicalize_filename_mode (BASE "/ouk", CAN_EXISTING);
    ASSERT (result2 == NULL);
    ASSERT (errno == ENOENT);
  }

  /* Check that a non-directory symlink with trailing slash yields NULL,
     and likewise for other troublesome suffixes.  */
  {
    char const *const file_name[]
      = {
         BASE "/huk/",
         BASE "/huk/.",
         BASE "/huk/./",
         BASE "/huk/./.",
         BASE "/huk/x",
         BASE "/huk/..",
         BASE "/huk/../",
         BASE "/huk/../.",
         BASE "/huk/../x",
         BASE "/huk/./..",
         BASE "/huk/././../x",
        };
    for (int i = 0; i < sizeof file_name / sizeof *file_name; i++)
      {
        errno = 0;
        ASSERT (!canonicalize_file_name (file_name[i]));
        ASSERT (errno == ENOTDIR);
        errno = 0;
        ASSERT (!canonicalize_filename_mode (file_name[i], CAN_EXISTING));
        ASSERT (errno == ENOTDIR);
      }
  }

  /* Check that a missing directory via symlink yields NULL.  */
  {
    char *result1;
    char *result2;
    errno = 0;
    result1 = canonicalize_file_name (BASE "/ouk/..");
    ASSERT (result1 == NULL);
    ASSERT (errno == ENOENT);
    errno = 0;
    result2 = canonicalize_filename_mode (BASE "/ouk/..", CAN_EXISTING);
    ASSERT (result2 == NULL);
    ASSERT (errno == ENOENT);
  }

  /* Check that a loop of symbolic links is detected.  */
  {
    char *result1;
    char *result2;
    errno = 0;
    result1 = canonicalize_file_name ("ise");
    ASSERT (result1 == NULL);
    ASSERT (errno == ELOOP);
    errno = 0;
    result2 = canonicalize_filename_mode ("ise", CAN_EXISTING);
    ASSERT (result2 == NULL);
    ASSERT (errno == ELOOP);
  }

  /* Check that alternate modes can resolve missing basenames.  */
  {
    char *result1 = canonicalize_filename_mode (BASE "/zzz", CAN_ALL_BUT_LAST);
    char *result2 = canonicalize_filename_mode (BASE "/zzz", CAN_MISSING);
    char *result3 = canonicalize_filename_mode (BASE "/zzz/", CAN_ALL_BUT_LAST);
    char *result4 = canonicalize_filename_mode (BASE "/zzz/", CAN_MISSING);
    ASSERT (result1 != NULL);
    ASSERT (result2 != NULL);
    ASSERT (result3 != NULL);
    ASSERT (result4 != NULL);
    ASSERT (strcmp (result1, result2) == 0);
    ASSERT (strcmp (result2, result3) == 0);
    ASSERT (strcmp (result3, result4) == 0);
    ASSERT (strcmp (result1 + strlen (result1) - strlen ("/" BASE "/zzz"),
                    "/" BASE "/zzz") == 0);
    free (result1);
    free (result2);
    free (result3);
    free (result4);
  }

  /* Check that alternate modes can resolve broken symlink basenames.  */
  {
    char *result1 = canonicalize_filename_mode (BASE "/ouk", CAN_ALL_BUT_LAST);
    char *result2 = canonicalize_filename_mode (BASE "/ouk", CAN_MISSING);
    char *result3 = canonicalize_filename_mode (BASE "/ouk/", CAN_ALL_BUT_LAST);
    char *result4 = canonicalize_filename_mode (BASE "/ouk/", CAN_MISSING);
    ASSERT (result1 != NULL);
    ASSERT (result2 != NULL);
    ASSERT (result3 != NULL);
    ASSERT (result4 != NULL);
    ASSERT (strcmp (result1, result2) == 0);
    ASSERT (strcmp (result2, result3) == 0);
    ASSERT (strcmp (result3, result4) == 0);
    ASSERT (strcmp (result1 + strlen (result1) - strlen ("/" BASE "/wum"),
                    "/" BASE "/wum") == 0);
    free (result1);
    free (result2);
    free (result3);
    free (result4);
  }

  /* Check that alternate modes can handle missing dirnames.  */
  {
    char *result1 = canonicalize_filename_mode ("t-can.zzz/zzz", CAN_ALL_BUT_LAST);
    char *result2 = canonicalize_filename_mode ("t-can.zzz/zzz", CAN_MISSING);
    ASSERT (result1 == NULL);
    ASSERT (result2 != NULL);
    ASSERT (strcmp (result2 + strlen (result2) - 14, "/t-can.zzz/zzz") == 0);
    free (result2);
  }

  /* Ensure that the following is resolved properly.
     Before 2007-09-27, it would mistakenly report a loop.  */
  {
    char *result1 = canonicalize_filename_mode (BASE, CAN_EXISTING);
    char *result2 = canonicalize_filename_mode (BASE "/p/1", CAN_EXISTING);
    ASSERT (result1 != NULL);
    ASSERT (result2 != NULL);
    ASSERT (strcmp (result2 + strlen (result1), "/d/2") == 0);
    free (result1);
    free (result2);
  }

  /* Check that leading // within symlinks is honored correctly.  */
  {
    struct stat st1;
    struct stat st2;
    char *result1 = canonicalize_file_name ("//.");
    char *result2 = canonicalize_filename_mode ("//.", CAN_EXISTING);
    char *result3 = canonicalize_file_name (BASE "/droot");
    char *result4 = canonicalize_filename_mode (BASE "/droot", CAN_EXISTING);
    ASSERT (result1);
    ASSERT (result2);
    ASSERT (result3);
    ASSERT (result4);
    ASSERT (stat ("/", &st1) == 0);
    ASSERT (stat ("//", &st2) == 0);
    bool same = psame_inode (&st1, &st2);
#if defined __MVS__ || defined MUSL_LIBC
    /* On IBM z/OS and musl libc, "/" and "//" both canonicalize to
       themselves, yet they both have st_dev == st_ino == 1.  */
    same = false;
#endif
    if (same)
      {
        ASSERT (strcmp (result1, "/") == 0);
        ASSERT (strcmp (result2, "/") == 0);
        ASSERT (strcmp (result3, "/") == 0);
        ASSERT (strcmp (result4, "/") == 0);
      }
    else
      {
        ASSERT (strcmp (result1, "//") == 0);
        ASSERT (strcmp (result2, "//") == 0);
        ASSERT (strcmp (result3, "//") == 0);
        ASSERT (strcmp (result4, "//") == 0);
      }
    free (result1);
    free (result2);
    free (result3);
    free (result4);
  }

  /* Cleanup.  */
  ASSERT (remove (BASE "/droot") == 0);
  ASSERT (remove (BASE "/d/1") == 0);
  ASSERT (remove (BASE "/d/2") == 0);
  ASSERT (remove (BASE "/d") == 0);
  ASSERT (remove (BASE "/s") == 0);
  ASSERT (remove (BASE "/p") == 0);
  ASSERT (remove (BASE "/plo") == 0);
  ASSERT (remove (BASE "/huk") == 0);
  ASSERT (remove (BASE "/bef") == 0);
  ASSERT (remove (BASE "/ouk") == 0);
  ASSERT (remove (BASE "/ket") == 0);
  ASSERT (remove (BASE "/lum") == 0);
  ASSERT (remove (BASE "/tra") == 0);
  ASSERT (remove (BASE) == 0);
  ASSERT (remove ("ise") == 0);

  return 0;
}
