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

#ifndef _DBE_DEFAULTMAP_H
#define _DBE_DEFAULTMAP_H

#include <assert.h>
#include <vec.h>
#include <Map.h>

template <typename Key_t, typename Value_t>
class DefaultMap : public Map<Key_t, Value_t>
{
public:

  DefaultMap ();
  ~DefaultMap ();
  void clear ();
  void put (Key_t key, Value_t val);
  Value_t get (Key_t key);
  Value_t get (Key_t key, typename Map<Key_t, Value_t>::Relation rel);
  Value_t remove (Key_t);
  Vector<Key_t> *keySet ();
  Vector<Value_t> *values ();

private:

  struct Entry
  {
    Key_t key;
    Value_t val;
  };

  static const int CHUNK_SIZE;
  static const int HTABLE_SIZE;

  int entries;
  int nchunks;
  Entry **chunks;
  Vector<Entry*> *index;
  Entry **hashTable;
};


template <typename Key_t, typename Value_t>
const int DefaultMap<Key_t, Value_t>::CHUNK_SIZE = 16384;
template <typename Key_t, typename Value_t>
const int DefaultMap<Key_t, Value_t>::HTABLE_SIZE = 1024;

template <typename Key_t, typename Value_t>
DefaultMap<Key_t, Value_t>::DefaultMap ()
{
  entries = 0;
  nchunks = 0;
  chunks = NULL;
  index = new Vector<Entry*>;
  hashTable = new Entry*[HTABLE_SIZE];
  for (int i = 0; i < HTABLE_SIZE; i++)
    hashTable[i] = NULL;
}

template <typename Key_t, typename Value_t>
DefaultMap<Key_t, Value_t>::~DefaultMap ()
{
  for (int i = 0; i < nchunks; i++)
    delete[] chunks[i];
  delete[] chunks;
  delete index;
  delete[] hashTable;
}

template <typename Key_t, typename Value_t>
void
DefaultMap<Key_t, Value_t>::clear ()
{
  entries = 0;
  index->reset ();
  for (int i = 0; i < HTABLE_SIZE; i++)
    hashTable[i] = NULL;
}

template <typename Key_t>
inline unsigned
hash (Key_t key)
{
  unsigned h = (unsigned) ((unsigned long) key);
  h ^= (h >> 20) ^ (h >> 12);
  return (h ^ (h >> 7) ^ (h >> 4));
}

template <typename Key_t, typename Value_t>
void
DefaultMap<Key_t, Value_t>::put (Key_t key, Value_t val)
{
  unsigned idx = hash (key) % HTABLE_SIZE;
  Entry *entry = hashTable[idx];
  if (entry && entry->key == key)
    {
      entry->val = val;
      return;
    }
  int lo = 0;
  int hi = entries - 1;
  while (lo <= hi)
    {
      int md = (lo + hi) / 2;
      entry = index->fetch (md);
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
      delete[] chunks;
      chunks = new_chunks;

      // Allocate new chunk for entries.
      chunks[nchunks - 1] = new Entry[CHUNK_SIZE];
    }
  entry = &chunks[entries / CHUNK_SIZE][entries % CHUNK_SIZE];
  entry->key = key;
  entry->val = val;
  index->insert (lo, entry);
  hashTable[idx] = entry;
  entries++;
}

template <typename Key_t, typename Value_t>
Value_t
DefaultMap<Key_t, Value_t>::get (Key_t key)
{
  unsigned idx = hash (key) % HTABLE_SIZE;
  Entry *entry = hashTable[idx];
  if (entry && entry->key == key)
    return entry->val;

  int lo = 0;
  int hi = entries - 1;
  while (lo <= hi)
    {
      int md = (lo + hi) / 2;
      entry = index->fetch (md);
      int cmp = entry->key < key ? -1 : entry->key > key ? 1 : 0;
      if (cmp < 0)
	lo = md + 1;
      else if (cmp > 0)
	hi = md - 1;
      else
	{
	  hashTable[idx] = entry;
	  return entry->val;
	}
    }
  return (Value_t) 0;
}

template <typename Key_t, typename Value_t>
Value_t
DefaultMap<Key_t, Value_t>::get (Key_t key,
				 typename Map<Key_t, Value_t>::Relation rel)
{
  if (rel != Map<Key_t, Value_t>::REL_EQ)
    return (Value_t) 0;
  return get (key);
}

template <typename Key_t, typename Value_t>
Value_t
DefaultMap<Key_t, Value_t>::remove (Key_t)
{
  // Not implemented
  if (1)
    assert (0);
  return (Value_t) 0;
}

template <typename Key_t, typename Value_t>
Vector<Value_t> *
DefaultMap<Key_t, Value_t>::values ()
{
  Vector<Value_t> *vals = new Vector<Value_t>(entries);
  for (int i = 0; i < entries; ++i)
    {
      Entry *entry = index->fetch (i);
      vals->append (entry->val);
    }
  return vals;
}

template <typename Key_t, typename Value_t>
Vector<Key_t> *
DefaultMap<Key_t, Value_t>::keySet ()
{
  Vector<Key_t> *keys = new Vector<Key_t>(entries);
  for (int i = 0; i < entries; ++i)
    {
      Entry *entry = index->fetch (i);
      keys->append (entry->key);
    }
  return keys;
}

#endif
