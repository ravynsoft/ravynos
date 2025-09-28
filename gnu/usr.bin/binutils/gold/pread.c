/* pread.c -- version of pread for gold.  */

/* Copyright (C) 2006-2023 Free Software Foundation, Inc.
   Written by Ian Lance Taylor <iant@google.com>.

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

/* This file implements pread for systems which don't have it.  This
   file is only compiled if pread is not present on the system.  This
   is not an exact version of pread, as it does not preserve the
   current file offset.  */

#include "config.h"

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

extern ssize_t pread (int, void *, size_t, off_t);

ssize_t
pread (int fd, void *buf, size_t count, off_t offset)
{
  if (lseek(fd, offset, SEEK_SET) != offset)
    return -1;
  return read(fd, buf, count);
}
