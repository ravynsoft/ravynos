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

#ifndef _HEAPMAP_H
#define _HEAPMAP_H

#include "dbe_types.h"
#include "vec.h"

struct HeapObj;
struct HeapChunk;

typedef struct UnmapChunk
{
  long val;
  int64_t size;
  UnmapChunk *next;
} UnmapChunk;

class HeapMap
{
public:
  HeapMap ();
  ~HeapMap ();
  void allocate (uint64_t addr, long val);
  long deallocate (uint64_t addr);
  UnmapChunk *mmap (uint64_t addr, int64_t size, long val);
  UnmapChunk *munmap (uint64_t addr, int64_t size);

private:
  void allocateChunk ();
  HeapObj *getHeapObj ();
  void releaseHeapObj (HeapObj*);
  UnmapChunk *process (HeapObj *obj, uint64_t addr, int64_t size);

  HeapChunk *chunks;
  HeapObj *empty;
  HeapObj **chain;
  HeapObj *mmaps;
};

#endif /* _HEAPMAP_H */
