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

#ifndef _MEMOBJECT_H
#define _MEMOBJECT_H

#include "Histable.h"
#include "util.h"

class MemObj : public Histable
{
public:
  friend class MemorySpace;

  MemObj (uint64_t _index, char *_name);
  ~MemObj ();

  virtual Histable *convertto (Histable_type, Histable* = NULL);

  virtual Histable_type
  get_type ()
  {
    return MEMOBJ;
  }

  virtual char *
  get_name (NameFormat = NA)
  {
    return dbe_strdup (name);
  }

  virtual uint64_t
  get_addr ()
  {
    return id;
  }

  uint64_t
  get_index ()
  {
    return id;
  }
};

#endif  /* _MEMOBJECT_H */
