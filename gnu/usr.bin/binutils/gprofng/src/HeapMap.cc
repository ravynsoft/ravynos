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
#include "HeapMap.h"

#define HEAPCHUNKSZ     1024 // number of HeapObj's in a chunk
#define HEAPCHAINS      9192 // number of address-based chains
#define hash(x)         (((x) >> 6) % HEAPCHAINS)

typedef struct HeapObj
{
  uint64_t addr;
  uint64_t size;
  long val;
  HeapObj *next;
} HeapObj;

typedef struct HeapChunk
{
  void *addr;
  HeapChunk *next;
} HeapChunk;

HeapMap::HeapMap ()
{
  chunks = NULL;
  empty = NULL;
  chain = new HeapObj*[HEAPCHAINS];
  for (int i = 0; i < HEAPCHAINS; i++)
    chain[i] = NULL;

  mmaps = new HeapObj;
  mmaps->addr = (uint64_t) 0;
  mmaps->size = (uint64_t) 0;
  mmaps->val = -1;
  mmaps->next = NULL;
}

HeapMap::~HeapMap ()
{
  // free up all the chunks
  HeapChunk *c = chunks;
  while (c != NULL)
    {
      HeapChunk *next = c->next;
      delete c;
      c = next;
    }
  delete[] chain;
  delete mmaps;
}

void
HeapMap::allocate (uint64_t addr, long val)
{
  // get a HeapObj, and set it up for the allocated block
  HeapObj *incoming = getHeapObj ();
  incoming->addr = addr;
  incoming->val = val;
  incoming->next = NULL;

  // determine which chain the block belongs on
  int ichain = (int) hash (addr);

  // if this is the first, just set it up and return
  if (chain[ichain] == NULL)
    {
      chain[ichain] = incoming;
      return;
    }
  // chain is non-empty -- find the slot for this one
  //  chain is maintained in reverse address order
  HeapObj *prev = NULL;
  HeapObj *next = chain[ichain];

  for (;;)
    {
      if ((next == NULL) || (next->addr < incoming->addr))
	{
	  // we've found the spot
	  incoming->next = next;
	  if (prev == NULL)
	    chain[ichain] = incoming;
	  else
	    prev->next = incoming;
	  return;
	}
      if (next->addr == incoming->addr)
	{
	  // error -- two blocks with same address active
	  //   ignore the new one
	  releaseHeapObj (incoming);
	  return;
	}
      // not yet, keep looking
      prev = next;
      next = next->next;
    }
}

long
HeapMap::deallocate (uint64_t addr)
{
  int ichain = (int) hash (addr);
  HeapObj *cur = chain[ichain];
  HeapObj *prev = NULL;
  while (cur != NULL)
    {
      if (cur->addr == addr)
	{
	  // we've found the block
	  long val = cur->val;

	  // delete the block from the chain
	  if (prev == NULL)
	    chain[ichain] = cur->next;
	  else
	    prev->next = cur->next;
	  releaseHeapObj (cur);
	  return val;
	}
      prev = cur;
      cur = cur->next;
    }

  // block not found
  return 0;
}

void
HeapMap::allocateChunk ()
{
  // allocate the memory
  HeapChunk *c = new HeapChunk;
  HeapObj *newc = new HeapObj[HEAPCHUNKSZ];

  // set up the chunk, and queue it for destructor's use
  c->addr = (void *) newc;
  c->next = chunks;
  chunks = c;

  // Initialize the HeapObj's in the chunk to one chain
  // last entry is left NULL, terminating the chain
  for (int i = 0; i < (HEAPCHUNKSZ - 1); i++)
    newc[i].next = newc + i + 1;
  newc[HEAPCHUNKSZ - 1].next = NULL;

  // put that chain on the empty queue
  empty = newc;
}

HeapObj *
HeapMap::getHeapObj ()
{
  if (empty == NULL)
    allocateChunk ();
  HeapObj *ret = empty;
  empty = ret->next;
  return ret;
}

void
HeapMap::releaseHeapObj (HeapObj *obj)
{
  obj->next = empty;
  empty = obj;
}

UnmapChunk*
HeapMap::mmap (uint64_t addr, int64_t size, long val)
{
  HeapObj *incoming = getHeapObj ();
  incoming->addr = addr;
  incoming->size = size;
  incoming->val = val;
  incoming->next = NULL;
  UnmapChunk* list = process (incoming, addr, size);
  return list;
}

UnmapChunk*
HeapMap::munmap (uint64_t addr, int64_t size)
{
  UnmapChunk* list = process (NULL, addr, size);
  return list;
}

UnmapChunk*
HeapMap::process (HeapObj *obj, uint64_t addr, int64_t size)
{
  // Some graphics are used to clarify the alignment of mmap regions
  // obj, shown as consecutive pages: "NNNNNN"
  // cur, shown as consecutive pages: "CCCCCC"

  // Find the first overlap, start of N is before end of C.  Examples:
  //   CCCCC
  //     NNNNN
  // or
  //   CCCCC
  //    NNN
  // or
  //   CCCCC
  //  NNNNN
  // or
  //   CCCCC
  //  NNNNNNN
  HeapObj *prev = mmaps;
  HeapObj *cur = prev->next;
  while (cur)
    {
      if (addr < cur->addr + cur->size)
	break;
      prev = cur;
      cur = prev->next;
    }

  // None found
  if (cur == NULL)
    {
      prev->next = obj;
      return NULL;
    }

  UnmapChunk* list = NULL;
  if (addr > cur->addr)
    {
      if (cur->addr + cur->size <= addr + size)
	{
	  // Process overlap on the left (low side) of new allocation
	  // New range: N-start to C-end (gets freed below)
	  prev = cur;
	  cur = getHeapObj ();
	  cur->addr = addr;
	  cur->size = prev->addr + prev->size - addr;
	  cur->val = prev->val;
	  cur->next = prev->next;

	  // Truncate range: C-start to N-start
	  prev->size = addr - prev->addr;
	}
      else
	{
	  // Process new allocation that fits completely within old allocation
	  // New range: N-start to N-end (freed below)
	  int64_t c_end = cur->addr + cur->size;
	  prev = cur;
	  cur = getHeapObj ();
	  cur->addr = addr;
	  cur->size = size;
	  cur->val = prev->val;
	  cur->next = prev->next;

	  // Truncate range: C-start to N-start
	  prev->size = addr - prev->addr;

	  // New range: N-end to C-end.
	  HeapObj *next = getHeapObj ();
	  next->addr = addr + size;
	  next->size = c_end - next->addr;
	  next->val = cur->val;
	  next->next = cur->next;
	  cur->next = next;
	}
    }

  // Process all full overlaps.
  // Copy details of cur to UnmapChunk list, remove cur from mmaps
  while (cur && cur->addr + cur->size <= addr + size)
    {

      UnmapChunk* uc = new UnmapChunk;
      uc->val = cur->val;
      uc->size = cur->size;
      uc->next = list;
      list = uc;

      HeapObj *t = cur;
      cur = cur->next;
      releaseHeapObj (t);
    }

  if (cur && cur->addr < addr + size)
    {
      // Process the last overlap (right side of new allocation)
      // Copy details of cur to UnmapChunk list
      UnmapChunk* uc = new UnmapChunk;
      uc->val = cur->val;
      uc->size = addr + size - cur->addr;
      uc->next = list;
      list = uc;

      // Truncate left side of cur
      cur->size -= uc->size;
      cur->addr = addr + size;
    }

  // Insert new allocation
  if (obj)
    {
      prev->next = obj;
      obj->next = cur;
    }
  else
    prev->next = cur;
  return list;
}
