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

#ifndef _DATASTREAM_H
#define _DATASTREAM_H

#include "Data_window.h"

// sequential access to the file
class DataStream : public Data_window
{
public:
  // Create an empty data window.
  DataStream (char *file_name);
  ~DataStream ();
  void set_span (int64_t f_offset, int64_t sz);
  int64_t read (void *buf, int64_t len);

  template <typename Key_t> inline int64_t
  read (Key_t &val)
  {
    int64_t sz = read (&val, sizeof (val));
    if (need_swap_endian && sz == sizeof (val))
      swapByteOrder (&val, sizeof (val));
    return sz;
  }

private:
  int64_t span_offset;
  int64_t span_size;        // the window size
  int64_t span_fileoffset;  // the window begin on the file
};

#endif /* _DATASTREAM_H */
