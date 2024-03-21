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

#ifndef _DBE_MAP2D_H
#define _DBE_MAP2D_H

template <typename Key1_t, typename Key2_t, typename Value_t>
class Map2D
{
public:

  enum MapType
  {
    Default,
    Interval
  };

  // Relation for the first key is always EQUAL
  enum Relation
  {
    REL_EQLT,
    REL_EQLE,
    REL_EQEQ,
    REL_EQGE,
    REL_EQGT
  };

  virtual ~Map2D () { };
  virtual void put (Key1_t key1, Key2_t key2, Value_t val) = 0;
  virtual Value_t get (Key1_t key1, Key2_t key2) = 0;
  virtual Value_t get (Key1_t key1, Key2_t key2, Relation rel) = 0;
  virtual Value_t remove (Key1_t key1, Key2_t key2) = 0;

};

#endif
