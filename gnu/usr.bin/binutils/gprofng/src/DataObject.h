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

#ifndef _DATAOBJECT_H
#define _DATAOBJECT_H

// A DataObject object represents a distinct dataobject.

#include "dbe_structs.h"
#include "Histable.h"

extern char *DOBJ_UNSPECIFIED;
extern char *DOBJ_UNIDENTIFIED;
extern char *DOBJ_UNDETERMINED;
extern char *DOBJ_ANON;
extern char *DOBJ_UNASCERTAINABLE;
extern char *DOBJ_UNVERIFIABLE;
extern char *DOBJ_UNRESOLVABLE;

class DataObject : public Histable
{
public:
  DataObject ();
  ~DataObject ();

  static const unsigned UNSPECIFIED_ID = 0xFFFFFFFF;

  int64_t size;             // size of the dataobject in bytes
  int64_t offset;           // offset of dataobject from parent
  DataObject *parent;       // this dataobject's parent (if any)
  Histable *scope;          // scope of this dataobject
  DataObject *master;       // this dataobject's master (if any)

  Histable_type get_type ()         { return DOBJECT; }
  int64_t get_size ()               { return size; }
  int64_t get_offset ()             { return offset; }
  DataObject *get_parent ()         { return parent; }
  DataObject *get_master ()         { return master; }
  char *get_typename ()             { return _typename; }
  char *get_instname ()             { return _instname; }
  Histable *get_scope ()            { return scope; }

  char *get_unannotated_name ()
  { // name without a <Scalar> or <Unknown> prefix
    if (_unannotated_name)
      return _unannotated_name;
    return get_name ();
  }

  uint64_t get_addr ();
  char get_offset_mark ();
  char *get_offset_name ();
  void set_dobjname (char *type_name, char *inst_name); // dobj->parent must already be set
  void set_name (char *);
  Histable *convertto (Histable_type type, Histable *obj = NULL);
  DbeEA *find_dbeEA (Vaddr EA);

private:
  char *_unannotated_name;  // name without a <Scalar> or <Unknown> prefix
  char *_typename;          // name of this dataobject's type
  char *_instname;          // name of this dataobject instance
  Vector<DbeEA*> *EAs;
};

#endif  /* _DATAOBJECT_H */
