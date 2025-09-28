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
 *	String Map implementation.
 */

#ifndef _DBE_STRINGMAP_H
#define _DBE_STRINGMAP_H

#include <assert.h>
#include <vec.h>
#include <Map.h>
#include <util.h>

template <typename Value_t>
class StringMap : public Map<const char*, Value_t>
{
public:

  StringMap (int htable_size = 1024, int chunk_size = 16384);
  ~StringMap ();
  void clear ();
  void put (const char *key, Value_t val);
  Value_t get (const char *key);
  Value_t get (const char *key, typename Map<const char*, Value_t>::Relation rel);
  Value_t remove (const char*);
  Vector<const char*> *keySet ();
  Vector<Value_t> *values ();

private:

  static unsigned
  hash (const char *key)
  {
    return (unsigned) crc64 (key, strlen (key));
  }

  struct Entry
  {
    char *key;
    Value_t val;
  };

  int CHUNK_SIZE, HTABLE_SIZE;
  int entries;
  int nchunks;
  Entry **chunks;
  Vector<Entry*> *index;
  Entry **hashTable;
};

template <typename Value_t>
StringMap<Value_t>::StringMap (int htable_size, int chunk_size)
{
  HTABLE_SIZE = htable_size;
  CHUNK_SIZE = chunk_size;
  entries = 0;
  nchunks = 0;
  chunks = NULL;
  index = new Vector<Entry*>;
  hashTable = new Entry*[HTABLE_SIZE];
  for (int i = 0; i < HTABLE_SIZE; i++)
    hashTable[i] = NULL;
}

template <typename Value_t>
StringMap<Value_t>::~StringMap ()
{
  for (int i = 0; i < entries; ++i)
    {
      Entry *entry = index->fetch (i);
      free (entry->key);
    }
  for (int i = 0; i < nchunks; i++)
    delete[] chunks[i];
  delete[] chunks;
  delete index;
  delete[] hashTable;
}

template <typename Value_t>
void
StringMap<Value_t>::clear ()
{
  for (int i = 0; i < entries; ++i)
    {
      Entry *entry = index->fetch (i);
      free (entry->key);
    }
  entries = 0;
  index->reset ();
  for (int i = 0; i < HTABLE_SIZE; i++)
    hashTable[i] = NULL;
}

template <typename Value_t>
void
StringMap<Value_t>::put (const char *key, Value_t val)
{
  unsigned idx = hash (key) % HTABLE_SIZE;
  Entry *entry = hashTable[idx];
  if (entry && strcmp (entry->key, key) == 0)
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
      int cmp = strcmp (entry->key, key);
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
  entry->key = strdup (key);
  entry->val = val;
  index->insert (lo, entry);
  hashTable[idx] = entry;
  entries++;
}

template <typename Value_t>
Value_t
StringMap<Value_t>::get (const char *key)
{
  unsigned idx = hash (key) % HTABLE_SIZE;
  Entry *entry = hashTable[idx];
  if (entry && strcmp (entry->key, key) == 0)
    return entry->val;
  int lo = 0;
  int hi = entries - 1;
  while (lo <= hi)
    {
      int md = (lo + hi) / 2;
      entry = index->fetch (md);
      int cmp = strcmp (entry->key, key);
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

template <typename Value_t>
Value_t
StringMap<Value_t>::get (const char *key, typename Map<const char*,
			 Value_t>::Relation rel)
{
  if (rel != Map<const char*, Value_t>::REL_EQ)
    return (Value_t) 0;
  return get (key);
}

template <typename Value_t>
Value_t
StringMap<Value_t>::remove (const char*)
{
  // Not implemented
  if (1)
    assert (0);
  return (Value_t) 0;
}

template <typename Value_t>
Vector<Value_t> *
StringMap<Value_t>::values ()
{
  Vector<Value_t> *vals = new Vector<Value_t>(entries);
  for (int i = 0; i < entries; ++i)
    {
      Entry *entry = index->fetch (i);
      vals->append (entry->val);
    }
  return vals;
}

template <typename Value_t>
Vector<const char*> *
StringMap<Value_t>::keySet ()
{
  Vector<const char*> *keys = new Vector<const char*>(entries);
  for (int i = 0; i < entries; ++i)
    {
      Entry *entry = index->fetch (i);
      keys->append (entry->key);
    }
  return keys;
}

#endif
