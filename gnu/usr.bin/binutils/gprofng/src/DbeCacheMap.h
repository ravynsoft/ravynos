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
 *	Dbe Cache Map implementation.
 *
 *    Simple Cache Map makes the following assumptions:
 *    - Cache Map is used for very fast but not guaranteed mapping;
 *    - No Relations can be used;
 *    - all objects used as keys or values has to be managed
 *      outside CacheMap (f.e. deletion);
 */

#ifndef _DbeCacheMap_h
#define _DbeCacheMap_h

#include <Map.h>

template <typename Key_t, class ITEM>
class DbeCacheMap : public Map<Key_t, ITEM *>
{
public:

  DbeCacheMap (int _size = DefaultSize)
  { // _size should be 2 ** N
    size = _size;
    table = new DbeCache_T[size];
    memset (table, 0, size * sizeof (DbeCache_T));
  };

  ~DbeCacheMap ()
  {
    delete[] table;
  };

  void
  put (Key_t key, ITEM *val)
  {
    int ind = get_hash (key);
    table[ind].key = key;
    table[ind].value = val;
  };

  ITEM *
  get (Key_t key)
  {
    int ind = get_hash (key);
    if (table[ind].key == key)
      return table[ind].value;
    return (ITEM *) NULL;
  };

  ITEM *
  remove (Key_t key)
  {
    int ind = get_hash (key);
    ITEM *v = table[ind].value;
    table[ind].value = (ITEM *) NULL;
    return v;
  };

  ITEM *
  get (Key_t /* key */, typename Map<Key_t, ITEM *>::Relation /* rel */)
  {
    return (ITEM *) NULL;
  };

private:

  enum
  {
    DefaultSize     = (1 << 13)
  };

  typedef struct DbeCache_S
  {
    Key_t key;
    ITEM *value;
  } DbeCache_T;
  DbeCache_T *table;
  int size;

  int
  get_hash (Key_t key)
  {
    unsigned long long h = (unsigned long long) key;
    h ^= (h >> 20) ^ (h >> 12);
    return (h ^ (h >> 7) ^ (h >> 4)) & (size - 1);
  }
};

#endif
