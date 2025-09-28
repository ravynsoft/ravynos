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

#ifndef _INDEXOBJECT_H
#define _INDEXOBJECT_H

#include "Histable.h"
#include "Expression.h"

class IndexObject : public Histable
{
public:
  IndexObject (int _indextype, uint64_t _index);
  IndexObject (int _indextype, Histable *_obj);
  bool requires_string_sort (); // name column should be sorted using name text

  virtual Histable_type
  get_type ()
  {
    return INDEXOBJ;
  }

  virtual char *get_name (NameFormat = NA);
  virtual void set_name (char*);
  virtual void set_name_from_context (Expression::Context *);
  virtual Histable *convertto (Histable_type, Histable* = NULL);

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

  Histable *
  get_obj ()
  {
    return obj;
  }

  // for use in index object definitions
  static const uint64_t INDXOBJ_EXPGRID_SHIFT   = 60;
  static const uint64_t INDXOBJ_EXPID_SHIFT     = 32;
  static const uint64_t INDXOBJ_PAYLOAD_SHIFT   = 0;
  static const uint64_t INDXOBJ_EXPGRID_MASK    =
	((1LLU << (64 - INDXOBJ_EXPGRID_SHIFT)) - 1);
  static const uint64_t INDXOBJ_EXPID_MASK      =
	((1LLU << (INDXOBJ_EXPGRID_SHIFT - INDXOBJ_EXPID_SHIFT)) - 1);
  static const uint64_t INDXOBJ_PAYLOAD_MASK    =
	((1LLU << (INDXOBJ_EXPID_SHIFT - INDXOBJ_PAYLOAD_SHIFT)) - 1);

private:

  int indextype;
  Histable *obj;
  bool nameIsFinal;
};

typedef enum IndexObjTypes
{
  INDEX_THREADS = 0,
  INDEX_CPUS,
  INDEX_SAMPLES,
  INDEX_GCEVENTS,
  INDEX_SECONDS,
  INDEX_PROCESSES,
  INDEX_EXPERIMENTS,
  INDEX_BYTES,
  INDEX_DURATION,
  INDEX_LAST    // never used; marks the count of precompiled items
} IndexObjTypes_t;

class IndexObjType_t
{
public:
  IndexObjType_t ();
  ~IndexObjType_t ();
  int type;
  char *name;           // used as input
  char *i18n_name;      // used for output
  char *index_expr_str;
  Expression *index_expr;
  char mnemonic;
  char *short_description;
  char *long_description;
  MemObjType_t *memObj;
};

#endif  /* _INDEXOBJECT_H */
