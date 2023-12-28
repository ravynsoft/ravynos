/* A program to test BFD.
   Copyright (C) 2012-2023 Free Software Foundation, Inc.

   This file is part of the GNU Binutils.

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

#include "sysdep.h"
#include "bfd.h"

static void
die (const char *s)
{
  printf ("oops: %s\n", s);
  exit (1);
}

int
main (int argc, char **argv)
{
  bfd *archive;
  bfd *last, *next;
  int count;

  if (argc != 2)
    die ("usage: bfdtest1 <archive>");

  archive = bfd_openr (argv[1], NULL);
  if (archive == NULL)
    die ("no such archive");

  if (!bfd_check_format (archive, bfd_archive))
    {
      bfd_close (archive);
      die ("bfd_check_format");
    }

  for (count = 0, last = bfd_openr_next_archived_file (archive, NULL);
       last;
       last = next)
    {
      next = bfd_openr_next_archived_file (archive, last);
      bfd_close (last);
      count++;
    }

  for (last = bfd_openr_next_archived_file (archive, NULL);
       last;
       last = next)
    {
      next = bfd_openr_next_archived_file (archive, last);
      bfd_close (last);
      count--;
    }

  if (count != 0)
    die ("element count differs in second scan");

  if (!bfd_close (archive))
    die ("bfd_close");

  return 0;
}
