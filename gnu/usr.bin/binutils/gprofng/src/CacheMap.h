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

/*
 *  Cache Map implementation.
 *
 *  Cache Map makes the following assumptions:
 *    - Cache Map is used for very fast but not guaranteed mapping;
 *    - only REL_EQ Relation can be used;
 *    - all objects used as keys or values has to be managed
 *      outside CacheMap (f.e. deletion);
 *    - (Key_t)0 is invalid key;
 *    - (Value_t)0 is invalid value;
 *    - <TBC>
 */

#ifndef _DBE_CACHEMAP_H
#define _DBE_CACHEMAP_H

#include <assert.h>
#include <vec.h>
#include <Map.h>

template <typename Key_t, typename Value_t>
class CacheMap : public Map<Key_t, Value_t>
{
public:

  CacheMap ();
  ~CacheMap ();
  void put (Key_t key, Value_t val);
  Value_t get (Key_t key);
  Value_t get (Key_t key, typename Map<Key_t, Value_t>::Relation rel);
  Value_t
  remove (Key_t key);

private:

  struct Entry
  {
    Key_t key;
    Value_t val;

    Entry ()
    {
      key = (Key_t) 0;
    }
  };

  static const int INIT_SIZE;
  static const int MAX_SIZE;

  static unsigned hash (Key_t key);
  Entry *getEntry (Key_t key);

  int cursize;
  int nputs;
  int nchunks;
  Entry **chunks;
};

template <typename Key_t, typename Value_t>
const int CacheMap<Key_t, Value_t>::INIT_SIZE = 1 << 14;
template <typename Key_t, typename Value_t>
const int CacheMap<Key_t, Value_t>::MAX_SIZE = 1 << 20;

template <typename Key_t, typename Value_t>CacheMap<Key_t, Value_t>
::CacheMap ()
{
  cursize = INIT_SIZE;
  chunks = new Entry*[32];
  nchunks = 0;
  chunks[nchunks++] = new Entry[cursize];
  nputs = 0;
}

template <typename Key_t, typename Value_t>
CacheMap<Key_t, Value_t>::~CacheMap ()
{
  for (int i = 0; i < nchunks; i++)
    delete[] chunks[i];
  delete[] chunks;
}

template <typename Key_t, typename Value_t>
unsigned
CacheMap<Key_t, Value_t>::hash (Key_t key)
{
  unsigned h = (unsigned) key ^ (unsigned) (key >> 32);
  h ^= (h >> 20) ^ (h >> 12);
  return h ^ (h >> 7) ^ (h >> 4);
}

template <typename Key_t, typename Value_t>
void
CacheMap<Key_t, Value_t>::put (Key_t key, Value_t val)
{
  if (nputs >= cursize && cursize < MAX_SIZE)
    {
      // Allocate new chunk for entries.
      chunks[nchunks++] = new Entry[cursize];
      cursize *= 2;

      // Copy all old entries to the newly allocated chunk
      Entry *newchunk = chunks[nchunks - 1];
      int prevsz = 0;
      int nextsz = INIT_SIZE;
      for (int i = 0; i < nchunks - 1; i++)
	{
	  Entry *oldchunk = chunks[i];
	  for (int j = prevsz; j < nextsz; j++)
	    newchunk[j] = oldchunk[j - prevsz];
	  prevsz = nextsz;
	  nextsz *= 2;
	}
    }
  Entry *entry = getEntry (key);
  entry->key = key;
  entry->val = val;
  nputs++;
}

template <typename Key_t, typename Value_t>
typename CacheMap<Key_t, Value_t>::Entry *
CacheMap<Key_t, Value_t>::getEntry (Key_t key)
{
  unsigned idx = hash (key);
  int i = nchunks - 1;
  int j = cursize / 2;
  for (; i > 0; i -= 1, j /= 2)
    if (idx & j)
      break;
  if (i == 0)
    j *= 2;
  return &chunks[i][idx & (j - 1)];
}

template <typename Key_t, typename Value_t>
Value_t
CacheMap<Key_t, Value_t>::get (Key_t key)
{
  Entry *entry = getEntry (key);
  return entry->key == key ? entry->val : (Value_t) 0;
}

template <typename Key_t, typename Value_t>
Value_t
CacheMap<Key_t, Value_t>::get (Key_t key, typename Map<Key_t, Value_t>::Relation rel)
{
  if (rel != Map<Key_t, Value_t>::REL_EQ)
    return (Value_t) 0;
  return get (key);
}

template <typename Key_t, typename Value_t>
Value_t
CacheMap<Key_t, Value_t>::remove (Key_t key)
{
  Entry *entry = getEntry (key);
  Value_t res = (Value_t) 0;
  if (entry->key == key)
    {
      res = entry->val;
      entry->val = (Value_t) 0;
    }
  return res;
}

#endif
