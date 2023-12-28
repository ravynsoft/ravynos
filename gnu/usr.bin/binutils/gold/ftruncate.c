/* ftruncate emulations that work on some System V's.
   This file is in the public domain.  */

/* Copyright (C) 2012-2023 Free Software Foundation, Inc.

   This file is part of gold.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include <config.h>

/* Specification.  */
#include <unistd.h>

#include <sys/types.h>
#include <fcntl.h>

extern int ftruncate (int, off_t);

#ifdef F_CHSIZE

int
ftruncate (int fd, off_t length)
{
  return fcntl (fd, F_CHSIZE, length);
}

#else /* not F_CHSIZE */
# ifdef F_FREESP

/* By William Kucharski <kucharsk@netcom.com>.  */

#  include <sys/stat.h>
#  include <errno.h>

int
ftruncate (int fd, off_t length)
{
  struct flock fl;
  struct stat filebuf;

  if (fstat (fd, &filebuf) < 0)
    return -1;

  if (filebuf.st_size < length)
    {
      /* Extend file length. */
      if (lseek (fd, (length - 1), SEEK_SET) < 0)
	return -1;

      /* Write a "0" byte. */
      if (write (fd, "", 1) != 1)
	return -1;
    }
  else
    {

      /* Truncate length. */

      fl.l_whence = 0;
      fl.l_len = 0;
      fl.l_start = length;
      fl.l_type = F_WRLCK;	/* write lock on file space */

      /* This relies on the *undocumented* F_FREESP argument to fcntl,
	 which truncates the file so that it ends at the position
	 indicated by fl.l_start.  Will minor miracles never cease?  */

      if (fcntl (fd, F_FREESP, &fl) < 0)
	return -1;
    }

  return 0;
}

# else /* not F_CHSIZE nor F_FREESP */
#  if HAVE_CHSIZE                      /* native Windows, e.g. mingw */

int
ftruncate (int fd, off_t length)
{
  return chsize (fd, length);
}

#  else /* not F_CHSIZE nor F_FREESP nor HAVE_CHSIZE */

#   include <errno.h>

int
ftruncate (int fd, off_t length)
{
  errno = EIO;
  return -1;
}

#  endif /* not HAVE_CHSIZE */
# endif /* not F_FREESP */
#endif /* not F_CHSIZE */
