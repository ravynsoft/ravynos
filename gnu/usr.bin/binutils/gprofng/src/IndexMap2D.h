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
 *	Index Map2D implementation.
 *
 *    Index Map2D is dynamic two dimensional array
 */

#ifndef _DBE_INDEXMAP2D_H
#define _DBE_INDEXMAP2D_H

#include <assert.h>
#include <vec.h>
#include <Map2D.h>

template <typename Key1_t, typename Key2_t, typename Value_t>
class IndexMap2D : public Map2D<Key1_t, Key2_t, Value_t>
{
public:

  IndexMap2D ();
  ~IndexMap2D ();

  void put (Key1_t key1, Key2_t key2, Value_t val);
  Value_t get (Key1_t key1, Key2_t key2);
  Value_t get (Key1_t key1, Key2_t key2,
	       typename Map2D<Key1_t, Key2_t, Value_t>::Relation rel);
  Value_t remove (Key1_t key1, Key2_t key2);

private:

  Vector<Vector<Value_t>*> *map1;
};

template <typename Key1_t, typename Key2_t, typename Value_t>
IndexMap2D<Key1_t, Key2_t, Value_t>::IndexMap2D ()
{
  map1 = new Vector<Vector<Value_t>*>;
}

template <typename Key1_t, typename Key2_t, typename Value_t>
IndexMap2D<Key1_t, Key2_t, Value_t>::~IndexMap2D ()
{
  map1->destroy ();
  delete map1;
}

template <typename Key1_t, typename Key2_t, typename Value_t>
void
IndexMap2D<Key1_t, Key2_t, Value_t>::put (Key1_t key1, Key2_t key2, Value_t val)
{
  if (key1 < 0 || key2 < 0)
    return;
  Vector<Value_t> *map2 = NULL;
  if (key1 < map1->size ())
    map2 = map1->fetch ((int) key1);
  if (map2 == NULL)
    {
      map2 = new Vector<Value_t>;
      map1->store ((int) key1, map2);
    }
  map2->store ((int) key2, val);
}

template <typename Key1_t, typename Key2_t, typename Value_t>
Value_t
IndexMap2D<Key1_t, Key2_t, Value_t>::get (Key1_t key1, Key2_t key2)
{
  if (key1 < 0 || key1 >= map1->size () || key2 < 0)
    return (Value_t) 0;
  Vector<Value_t> *map2 = map1->fetch ((int) key1);
  if (map2 == NULL || key2 >= map2->size ())
    return (Value_t) 0;
  return map2->fetch ((int) key2);
}

template <typename Key1_t, typename Key2_t, typename Value_t>
Value_t
IndexMap2D<Key1_t, Key2_t, Value_t>::get (Key1_t key1, Key2_t key2,
			  typename Map2D<Key1_t, Key2_t, Value_t>::Relation rel)
{
  if (rel != Map2D<Key1_t, Key2_t, Value_t>::REL_EQEQ)
    return (Value_t) 0;
  return get (key1, key2);
}

template <typename Key1_t, typename Key2_t, typename Value_t>
Value_t
IndexMap2D<Key1_t, Key2_t, Value_t>::remove (Key1_t key1, Key2_t key2)
{
  if (key1 < 0 || key1 >= map1->size () || key2 < 0)
    return (Value_t) 0;
  Vector<Value_t> *map2 = map1->fetch ((int) key1);
  if (map2 == NULL || key2 >= map2->size ())
    return (Value_t) 0;
  Value_t res = map2->fetch ((int) key2);
  map2->store ((int) key2, (Value_t) 0);
  return res;
}

#endif
