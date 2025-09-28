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

#ifndef _DBE_DEFAULTMAP2D_H
#define _DBE_DEFAULTMAP2D_H

#include <assert.h>
#include <vec.h>
#include <DefaultMap.h>
#include <IntervalMap.h>
#include <Map2D.h>

/*
 *	Default Map2D implementation.
 *
 *    Default Map2D is a cartesian product of two default Maps.
 */

template <typename Key1_t, typename Key2_t, typename Value_t>
class DefaultMap2D : public Map2D<Key1_t, Key2_t, Value_t>
{
public:
  DefaultMap2D ();
  DefaultMap2D (typename Map2D<Key1_t, Key2_t, Value_t>::MapType _type);
  ~DefaultMap2D ();
  void put (Key1_t key1, Key2_t key2, Value_t val);
  Value_t get (Key1_t key1, Key2_t key2);
  Value_t get (Key1_t key1, Key2_t key2,
	       typename Map2D<Key1_t, Key2_t, Value_t>::Relation rel);
  Value_t remove (Key1_t, Key2_t);

private:
  typename Map2D<Key1_t, Key2_t, Value_t>::MapType type;
  Map<Key1_t, Map<Key2_t, Value_t>*> *map1;
  Vector<Map<Key2_t, Value_t>*> *map2list;
};

template <typename Key1_t, typename Key2_t, typename Value_t>
DefaultMap2D<Key1_t, Key2_t, Value_t>::DefaultMap2D ()
{
  type = Map2D<Key1_t, Key2_t, Value_t>::Default;
  map1 = new DefaultMap<Key1_t, Map<Key2_t, Value_t>*>;
  map2list = new Vector<Map<Key2_t, Value_t>*>;
}

template <typename Key1_t, typename Key2_t, typename Value_t>
DefaultMap2D<Key1_t, Key2_t, Value_t>::DefaultMap2D (
			 typename Map2D<Key1_t, Key2_t, Value_t>::MapType _type)
{
  type = _type;
  map1 = new DefaultMap<Key1_t, Map<Key2_t, Value_t>*>;
  map2list = new Vector<Map<Key2_t, Value_t>*>;
}

template <typename Key1_t, typename Key2_t, typename Value_t>
DefaultMap2D<Key1_t, Key2_t, Value_t>::~DefaultMap2D ()
{
  map2list->destroy ();
  delete map2list;
  delete map1;
}

template <typename Key1_t, typename Key2_t, typename Value_t>
void
DefaultMap2D<Key1_t, Key2_t, Value_t>::put (Key1_t key1, Key2_t key2, Value_t val)
{
  Map<Key2_t, Value_t> *map2 = map1->get (key1);
  if (map2 == NULL)
    {
      if (type == Map2D<Key1_t, Key2_t, Value_t>::Interval)
	map2 = new IntervalMap<Key2_t, Value_t>;
      else
	map2 = new DefaultMap<Key2_t, Value_t>;
      map2list->append (map2);
      map1->put (key1, map2);
    }
  map2->put (key2, val);
}

template <typename Key1_t, typename Key2_t, typename Value_t>
Value_t
DefaultMap2D<Key1_t, Key2_t, Value_t>::get (Key1_t key1, Key2_t key2)
{
  Map<Key2_t, Value_t> *map2 = map1->get (key1);
  if (map2 == NULL)
    return (Value_t) 0;
  return map2->get (key2);
}

template <typename Key1_t, typename Key2_t, typename Value_t>
Value_t
DefaultMap2D<Key1_t, Key2_t, Value_t>::get (Key1_t key1, Key2_t key2,
			  typename Map2D<Key1_t, Key2_t, Value_t>::Relation rel)
{
  Map<Key2_t, Value_t> *map2 = map1->get (key1);
  if (map2 == NULL)
    return (Value_t) 0;
  typename Map<Key2_t, Value_t>::Relation rel2;
  switch (rel)
    {
    case Map2D<Key1_t, Key2_t, Value_t>::REL_EQLT:
      rel2 = map2->REL_LT;
      break;
    case Map2D<Key1_t, Key2_t, Value_t>::REL_EQLE:
      rel2 = map2->REL_LE;
      break;
    case Map2D<Key1_t, Key2_t, Value_t>::REL_EQGE:
      rel2 = map2->REL_GE;
      break;
    case Map2D<Key1_t, Key2_t, Value_t>::REL_EQGT:
      rel2 = map2->REL_GT;
      break;
    default:
      rel2 = map2->REL_EQ;
      break;
    }
  return map2->get (key2, rel2);
}

template <typename Key1_t, typename Key2_t, typename Value_t>
Value_t
DefaultMap2D<Key1_t, Key2_t, Value_t>::remove (Key1_t, Key2_t)
{
  // Not implemented
  if (1)
    assert (0);
  return (Value_t) 0;
}

#endif
