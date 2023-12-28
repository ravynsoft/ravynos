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

#ifndef _DATA_WINDOW_H
#define _DATA_WINDOW_H

// The Data_window class hiearchy is used to access raw data in the
// experiment record.
//
// The Data_window base class implements a set of windows into a raw data file.
// It is responsible for mapping and unmapping regions of the file as
// requested by other levels inside of the DBE.

#include "util.h"

class Data_window
{
public:

  // Span in data file
  typedef struct
  {
    int64_t offset; // file offset
    int64_t length; // span length
  } Span;

  Data_window (char *filename);
  ~Data_window ();

  // Return address of "offset" byte of window for "length" bytes.
  // Return 0 on error or locked.
  void *bind (Span *span, int64_t minSize);
  void *bind (int64_t file_offset, int64_t minSize);
  void *get_data (int64_t offset, int64_t size, void *datap);
  int64_t get_buf_size ();
  int64_t copy_to_file (int f, int64_t offset, int64_t size);

  bool not_opened ()            { return !opened; }
  off64_t get_fsize ()          { return fsize; }

  template <typename Key_t> inline Key_t
  get_align_val (Key_t *vp)
  {
    if (sizeof (Key_t) <= sizeof (int))
      return *vp;
    // 64-bit value can have a wrong alignment
    Key_t val = (Key_t) 0;
    uint32_t *p1 = (uint32_t *) vp;
    uint32_t *p2 = (uint32_t*) (&val);
    p2[0] = p1[0];
    p2[1] = p1[1];
    return val;
  }

  template <typename Key_t> inline Key_t
  decode (Key_t &v)
  {
    Key_t val = get_align_val (&v);
    if (need_swap_endian)
      swapByteOrder (&val, sizeof (val));
    return val;
  }

  bool need_swap_endian;
  char *fname;          // file name

protected:
  int fd;               // file descriptor
  bool mmap_on_file;

private:
  long page_size;       // used in mmap()
  bool use_mmap;
  bool opened;
  int64_t fsize;        // file size
  void *base;           // current window
  int64_t woffset;      // offset of current window
  int64_t wsize;        // size of current window
  int64_t basesize;     // size of allocated window
};

#endif /* _DATA_WINDOW_H */
