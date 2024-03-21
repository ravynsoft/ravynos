/* Copyright (C) 2021-2023 Free Software Foundation, Inc.
   Contributed by Oracle.

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "config.h"
#include "util.h"
#include "DataStream.h"
#include "debug.h"

DataStream::DataStream (char *filename) : Data_window (filename)
{
  set_span (0, -1);
}

DataStream::~DataStream () { }

void
DataStream::set_span (int64_t f_offset, int64_t sz)
{
  span_offset = 0;
  span_fileoffset = f_offset;
  int64_t fsz = get_fsize ();
  span_size = sz == -1 ? fsz : sz;
  if (span_fileoffset >= fsz)
    span_fileoffset = fsz;
  if (span_size > fsz - span_fileoffset)
    span_size = fsz - span_fileoffset;
}

int64_t
DataStream::read (void *buf, int64_t len)
{
  if (len > span_size - span_offset)
    len = span_size - span_offset;
  int64_t off = span_offset + span_fileoffset;
  span_offset += len;
  get_data (off, len, buf);
  return len;
}
