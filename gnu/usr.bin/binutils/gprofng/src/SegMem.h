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

#ifndef _SEGMEM_H
#define _SEGMEM_H

#include "dbe_types.h"
class Histable;

class SegMem
{
public:

  // The various segments types.
  enum Seg_mode
  {
    READ,
    WRITE,
    EXEC,
    UNKNOWN
  };

  void
  set_file_offset (uint64_t fo)
  {
    file_offset = fo;
  }

  uint64_t
  get_file_offset ()
  {
    return file_offset;
  }

  void
  set_mode (Seg_mode sm)
  {
    mode = sm;
  }

  Seg_mode
  get_mode ()
  {
    return mode;
  }

  Size size;            // Size of this instance
  Histable *obj;        // Pointer to Segment/Function object
  Vaddr base;           // Base address
  hrtime_t load_time;
  hrtime_t unload_time;
  Size page_size;

private:
  uint64_t file_offset;
  Seg_mode mode;
};

#endif /* _SEGMEM_H */
