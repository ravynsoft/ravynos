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

#ifndef _DBE_MAP_H
#define _DBE_MAP_H

#include "vec.h"

template <typename Key_t, typename Value_t>
class Map
{
public:

  enum Relation
  {
    REL_LT,
    REL_LE,
    REL_EQ,
    REL_GE,
    REL_GT
  };

  virtual ~Map () { };
  virtual void put (Key_t key, Value_t val) = 0;
  virtual Value_t get (Key_t key) = 0;
  virtual Value_t get (Key_t key, Relation rel) = 0;
  virtual Value_t remove (Key_t key) = 0;

  virtual Vector<Key_t> *
  keySet ()
  {
    return NULL;
  }

  virtual Vector<Value_t> *
  values ()
  {
    return NULL;
  }
};

#define destroy_map(t, p) if (p) { Vector<t> *v = p->values (); Destroy (v); delete p; }

#endif
