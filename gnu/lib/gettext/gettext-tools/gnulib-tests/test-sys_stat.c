/* Test of <sys/stat.h> substitute.
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

#include <config.h>

#include <sys/stat.h>

/* Check the existence of some macros.  */
int a[] =
  {
    S_IFMT,
#ifdef S_IFBLK /* missing on MSVC */
    S_IFBLK,
#endif
    S_IFCHR, S_IFDIR, S_IFIFO, S_IFREG,
#ifdef S_IFLNK /* missing on native Windows and DJGPP */
    S_IFLNK,
#endif
#ifdef S_IFSOCK /* missing on native Windows and DJGPP */
    S_IFSOCK,
#endif
    S_IRWXU, S_IRUSR, S_IWUSR, S_IXUSR,
    S_IRWXG, S_IRGRP, S_IWGRP, S_IXGRP,
    S_IRWXO, S_IROTH, S_IWOTH, S_IXOTH,
    S_ISUID, S_ISGID, S_ISVTX,
    S_ISBLK (S_IFREG),
    S_ISCHR (S_IFREG),
    S_ISDIR (S_IFREG),
    S_ISFIFO (S_IFREG),
    S_ISREG (S_IFREG),
    S_ISLNK (S_IFREG),
    S_ISSOCK (S_IFREG),
    S_ISDOOR (S_IFREG),
    S_ISMPB (S_IFREG),
    S_ISMPX (S_IFREG),
    S_ISNAM (S_IFREG),
    S_ISNWK (S_IFREG),
    S_ISPORT (S_IFREG),
    S_ISCTG (S_IFREG),
    S_ISOFD (S_IFREG),
    S_ISOFL (S_IFREG),
    S_ISWHT (S_IFREG)
  };

/* Sanity checks.  */

static_assert (S_IRWXU == (S_IRUSR | S_IWUSR | S_IXUSR));
static_assert (S_IRWXG == (S_IRGRP | S_IWGRP | S_IXGRP));
static_assert (S_IRWXO == (S_IROTH | S_IWOTH | S_IXOTH));

#ifdef S_IFBLK
static_assert (S_ISBLK (S_IFBLK));
#endif
static_assert (!S_ISBLK (S_IFCHR));
static_assert (!S_ISBLK (S_IFDIR));
static_assert (!S_ISBLK (S_IFIFO));
static_assert (!S_ISBLK (S_IFREG));
#ifdef S_IFLNK
static_assert (!S_ISBLK (S_IFLNK));
#endif
#ifdef S_IFSOCK
static_assert (!S_ISBLK (S_IFSOCK));
#endif

#ifdef S_IFBLK
static_assert (!S_ISCHR (S_IFBLK));
#endif
static_assert (S_ISCHR (S_IFCHR));
static_assert (!S_ISCHR (S_IFDIR));
static_assert (!S_ISCHR (S_IFIFO));
static_assert (!S_ISCHR (S_IFREG));
#ifdef S_IFLNK
static_assert (!S_ISCHR (S_IFLNK));
#endif
#ifdef S_IFSOCK
static_assert (!S_ISCHR (S_IFSOCK));
#endif

#ifdef S_IFBLK
static_assert (!S_ISDIR (S_IFBLK));
#endif
static_assert (!S_ISDIR (S_IFCHR));
static_assert (S_ISDIR (S_IFDIR));
static_assert (!S_ISDIR (S_IFIFO));
static_assert (!S_ISDIR (S_IFREG));
#ifdef S_IFLNK
static_assert (!S_ISDIR (S_IFLNK));
#endif
#ifdef S_IFSOCK
static_assert (!S_ISDIR (S_IFSOCK));
#endif

#ifdef S_IFBLK
static_assert (!S_ISFIFO (S_IFBLK));
#endif
static_assert (!S_ISFIFO (S_IFCHR));
static_assert (!S_ISFIFO (S_IFDIR));
static_assert (S_ISFIFO (S_IFIFO));
static_assert (!S_ISFIFO (S_IFREG));
#ifdef S_IFLNK
static_assert (!S_ISFIFO (S_IFLNK));
#endif
#ifdef S_IFSOCK
static_assert (!S_ISFIFO (S_IFSOCK));
#endif

#ifdef S_IFBLK
static_assert (!S_ISREG (S_IFBLK));
#endif
static_assert (!S_ISREG (S_IFCHR));
static_assert (!S_ISREG (S_IFDIR));
static_assert (!S_ISREG (S_IFIFO));
static_assert (S_ISREG (S_IFREG));
#ifdef S_IFLNK
static_assert (!S_ISREG (S_IFLNK));
#endif
#ifdef S_IFSOCK
static_assert (!S_ISREG (S_IFSOCK));
#endif

#ifdef S_IFBLK
static_assert (!S_ISLNK (S_IFBLK));
#endif
static_assert (!S_ISLNK (S_IFCHR));
static_assert (!S_ISLNK (S_IFDIR));
static_assert (!S_ISLNK (S_IFIFO));
static_assert (!S_ISLNK (S_IFREG));
#ifdef S_IFLNK
static_assert (S_ISLNK (S_IFLNK));
#endif
#ifdef S_IFSOCK
static_assert (!S_ISLNK (S_IFSOCK));
#endif

#ifdef S_IFBLK
static_assert (!S_ISSOCK (S_IFBLK));
#endif
static_assert (!S_ISSOCK (S_IFCHR));
static_assert (!S_ISSOCK (S_IFDIR));
static_assert (!S_ISSOCK (S_IFIFO));
static_assert (!S_ISSOCK (S_IFREG));
#ifdef S_IFLNK
static_assert (!S_ISSOCK (S_IFLNK));
#endif
#ifdef S_IFSOCK
static_assert (S_ISSOCK (S_IFSOCK));
#endif

#ifdef S_IFBLK
static_assert (!S_ISDOOR (S_IFBLK));
#endif
static_assert (!S_ISDOOR (S_IFCHR));
static_assert (!S_ISDOOR (S_IFDIR));
static_assert (!S_ISDOOR (S_IFIFO));
static_assert (!S_ISDOOR (S_IFREG));
#ifdef S_IFLNK
static_assert (!S_ISDOOR (S_IFLNK));
#endif
#ifdef S_IFSOCK
static_assert (!S_ISDOOR (S_IFSOCK));
#endif

#ifdef S_IFBLK
static_assert (!S_ISMPB (S_IFBLK));
#endif
static_assert (!S_ISMPB (S_IFCHR));
static_assert (!S_ISMPB (S_IFDIR));
static_assert (!S_ISMPB (S_IFIFO));
static_assert (!S_ISMPB (S_IFREG));
#ifdef S_IFLNK
static_assert (!S_ISMPB (S_IFLNK));
#endif
#ifdef S_IFSOCK
static_assert (!S_ISMPB (S_IFSOCK));
#endif

#ifdef S_IFBLK
static_assert (!S_ISMPX (S_IFBLK));
#endif
static_assert (!S_ISMPX (S_IFCHR));
static_assert (!S_ISMPX (S_IFDIR));
static_assert (!S_ISMPX (S_IFIFO));
static_assert (!S_ISMPX (S_IFREG));
#ifdef S_IFLNK
static_assert (!S_ISMPX (S_IFLNK));
#endif
#ifdef S_IFSOCK
static_assert (!S_ISMPX (S_IFSOCK));
#endif

#ifdef S_IFBLK
static_assert (!S_ISNAM (S_IFBLK));
#endif
static_assert (!S_ISNAM (S_IFCHR));
static_assert (!S_ISNAM (S_IFDIR));
static_assert (!S_ISNAM (S_IFIFO));
static_assert (!S_ISNAM (S_IFREG));
#ifdef S_IFLNK
static_assert (!S_ISNAM (S_IFLNK));
#endif
#ifdef S_IFSOCK
static_assert (!S_ISNAM (S_IFSOCK));
#endif

#ifdef S_IFBLK
static_assert (!S_ISNWK (S_IFBLK));
#endif
static_assert (!S_ISNWK (S_IFCHR));
static_assert (!S_ISNWK (S_IFDIR));
static_assert (!S_ISNWK (S_IFIFO));
static_assert (!S_ISNWK (S_IFREG));
#ifdef S_IFLNK
static_assert (!S_ISNWK (S_IFLNK));
#endif
#ifdef S_IFSOCK
static_assert (!S_ISNWK (S_IFSOCK));
#endif

#ifdef S_IFBLK
static_assert (!S_ISPORT (S_IFBLK));
#endif
static_assert (!S_ISPORT (S_IFCHR));
static_assert (!S_ISPORT (S_IFDIR));
static_assert (!S_ISPORT (S_IFIFO));
static_assert (!S_ISPORT (S_IFREG));
#ifdef S_IFLNK
static_assert (!S_ISPORT (S_IFLNK));
#endif
#ifdef S_IFSOCK
static_assert (!S_ISPORT (S_IFSOCK));
#endif

#ifdef S_IFBLK
static_assert (!S_ISCTG (S_IFBLK));
#endif
static_assert (!S_ISCTG (S_IFCHR));
static_assert (!S_ISCTG (S_IFDIR));
static_assert (!S_ISCTG (S_IFIFO));
static_assert (!S_ISCTG (S_IFREG));
#ifdef S_IFLNK
static_assert (!S_ISCTG (S_IFLNK));
#endif
#ifdef S_IFSOCK
static_assert (!S_ISCTG (S_IFSOCK));
#endif

#ifdef S_IFBLK
static_assert (!S_ISOFD (S_IFBLK));
#endif
static_assert (!S_ISOFD (S_IFCHR));
static_assert (!S_ISOFD (S_IFDIR));
static_assert (!S_ISOFD (S_IFIFO));
static_assert (!S_ISOFD (S_IFREG));
#ifdef S_IFLNK
static_assert (!S_ISOFD (S_IFLNK));
#endif
#ifdef S_IFSOCK
static_assert (!S_ISOFD (S_IFSOCK));
#endif

#ifdef S_IFBLK
static_assert (!S_ISOFL (S_IFBLK));
#endif
static_assert (!S_ISOFL (S_IFCHR));
static_assert (!S_ISOFL (S_IFDIR));
static_assert (!S_ISOFL (S_IFIFO));
static_assert (!S_ISOFL (S_IFREG));
#ifdef S_IFLNK
static_assert (!S_ISOFL (S_IFLNK));
#endif
#ifdef S_IFSOCK
static_assert (!S_ISOFL (S_IFSOCK));
#endif

#ifdef S_IFBLK
static_assert (!S_ISWHT (S_IFBLK));
#endif
static_assert (!S_ISWHT (S_IFCHR));
static_assert (!S_ISWHT (S_IFDIR));
static_assert (!S_ISWHT (S_IFIFO));
static_assert (!S_ISWHT (S_IFREG));
#ifdef S_IFLNK
static_assert (!S_ISWHT (S_IFLNK));
#endif
#ifdef S_IFSOCK
static_assert (!S_ISWHT (S_IFSOCK));
#endif

/* POSIX 2008 requires traditional encoding of permission constants.  */
static_assert (S_IRWXU == 00700);
static_assert (S_IRUSR == 00400);
static_assert (S_IWUSR == 00200);
static_assert (S_IXUSR == 00100);
static_assert (S_IRWXG == 00070);
static_assert (S_IRGRP == 00040);
static_assert (S_IWGRP == 00020);
static_assert (S_IXGRP == 00010);
static_assert (S_IRWXO == 00007);
static_assert (S_IROTH == 00004);
static_assert (S_IWOTH == 00002);
static_assert (S_IXOTH == 00001);
static_assert (S_ISUID == 04000);
static_assert (S_ISGID == 02000);
static_assert (S_ISVTX == 01000);

#if ((0 <= UTIME_NOW && UTIME_NOW < 1000000000)           \
     || (0 <= UTIME_OMIT && UTIME_OMIT < 1000000000)      \
     || UTIME_NOW == UTIME_OMIT)
invalid UTIME macros
#endif

/* Check the existence of some types.  */
nlink_t t1;
off_t t2;
mode_t t3;

struct timespec st;

int
main (void)
{
  return 0;
}
