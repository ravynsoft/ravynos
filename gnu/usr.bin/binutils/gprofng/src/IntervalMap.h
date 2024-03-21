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
 *  Interval Map implementation.
 *
 *    Interval Map makes the following assumptions:
 *    - if duplicate keys, the last one will be stored
 *    - <TBC>
 */
#ifndef _DBE_INTERVALMAP_H
#define _DBE_INTERVALMAP_H

#include <assert.h>
#include <vec.h>
#include <Map.h>

template <typename Key_t, typename Value_t>
class IntervalMap : public Map<Key_t, Value_t>
{
public:

  IntervalMap ();
  ~IntervalMap ();
  void put (Key_t key, Value_t val);
  Value_t get (Key_t key);
  Value_t get (Key_t key, typename Map<Key_t, Value_t>::Relation rel);
  Value_t remove (Key_t key);

private:

  struct Entry
  {
    Key_t key;
    Value_t val;
  };

  static const int CHUNK_SIZE;

  int entries;
  int nchunks;
  Entry **chunks;
  Vector<Entry*> *index;
};

template <typename Key_t, typename Value_t>
const int IntervalMap<Key_t, Value_t>::CHUNK_SIZE = 16384;

template <typename Key_t, typename Value_t>
IntervalMap<Key_t, Value_t>::IntervalMap ()
{
  entries = 0;
  nchunks = 0;
  chunks = NULL;
  index = new Vector<Entry*>;
}

template <typename Key_t, typename Value_t>
IntervalMap<Key_t, Value_t>::~IntervalMap ()
{
  for (int i = 0; i < nchunks; i++)
    delete[] chunks[i];
  delete[] chunks;
  delete index;
}

template <typename Key_t, typename Value_t>
void
IntervalMap<Key_t, Value_t>::put (Key_t key, Value_t val)
{
  int lo = 0;
  int hi = entries - 1;
  while (lo <= hi)
    {
      int md = (lo + hi) / 2;
      Entry *entry = index->fetch (md);
      int cmp = entry->key < key ? -1 : entry->key > key ? 1 : 0;
      if (cmp < 0)
	lo = md + 1;
      else if (cmp > 0)
	hi = md - 1;
      else
	{
	  entry->val = val;
	  return;
	}
    }

  if (entries >= nchunks * CHUNK_SIZE)
    {
      nchunks++;
      // Reallocate Entry chunk array
      Entry **new_chunks = new Entry*[nchunks];
      for (int i = 0; i < nchunks - 1; i++)
	new_chunks[i] = chunks[i];
      delete chunks;
      chunks = new_chunks;

      // Allocate new chunk for entries.
      chunks[nchunks - 1] = new Entry[CHUNK_SIZE];
    }
  Entry *entry = &chunks[entries / CHUNK_SIZE][entries % CHUNK_SIZE];
  entry->key = key;
  entry->val = val;
  index->insert (lo, entry);
  entries++;
}

template <typename Key_t, typename Value_t>
Value_t
IntervalMap<Key_t, Value_t>::get (Key_t key)
{
  return get (key, Map<Key_t, Value_t>::REL_EQ);
}

template <typename Key_t, typename Value_t>
Value_t
IntervalMap<Key_t, Value_t>::get (Key_t key, typename Map<Key_t, Value_t>::Relation rel)
{
  int lo = 0;
  int hi = entries - 1;
  while (lo <= hi)
    {
      int md = (lo + hi) / 2;
      Entry *entry = index->fetch (md);
      int cmp = entry->key < key ? -1 : entry->key > key ? 1 : 0;
      switch (rel)
	{
	case Map<Key_t, Value_t>::REL_LT:
	  if (cmp < 0)
	    lo = md + 1;
	  else
	    hi = md - 1;
	  break;
	case Map<Key_t, Value_t>::REL_GT:
	  if (cmp <= 0)
	    lo = md + 1;
	  else
	    hi = md - 1;
	  break;
	case Map<Key_t, Value_t>::REL_LE:
	case Map<Key_t, Value_t>::REL_GE:
	case Map<Key_t, Value_t>::REL_EQ:
	  if (cmp < 0)
	    lo = md + 1;
	  else if (cmp > 0)
	    hi = md - 1;
	  else
	    return entry->val;
	  break;
	}
    }
  switch (rel)
    {
    case Map<Key_t, Value_t>::REL_LT:
    case Map<Key_t, Value_t>::REL_LE:
      return hi >= 0 ? index->fetch (hi)->val : (Value_t) 0;
    case Map<Key_t, Value_t>::REL_GT:
    case Map<Key_t, Value_t>::REL_GE:
      return lo < entries ? index->fetch (lo)->val : (Value_t) 0;
    case Map<Key_t, Value_t>::REL_EQ:
      break;
    }
  return (Value_t) 0;
}

template <typename Key_t, typename Value_t>
Value_t
IntervalMap<Key_t, Value_t>::remove (Key_t)
{
  // Not implemented
  if (1)
    assert (0);
  return (Value_t) 0;
}

#endif
