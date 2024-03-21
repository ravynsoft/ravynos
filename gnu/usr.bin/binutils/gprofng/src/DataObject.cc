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

#include "config.h"
#include <assert.h>
#include <string.h>
#include <ctype.h>

#include "util.h"
#include "DbeSession.h"
#include "Application.h"
#include "DataObject.h"
#include "Module.h"
#include "debug.h"

DataObject::DataObject ()
{
  name = NULL;
  parent = NULL;
  master = NULL;
  _unannotated_name = NULL;
  _typename = NULL;
  _instname = NULL;
  scope = NULL;
  EAs = new Vector<DbeEA*>;
  size = 0;
  offset = (uint64_t) (-1);
}

DataObject::~DataObject ()
{
  free (_unannotated_name);
  free (_typename);
  free (_instname);
  EAs->destroy ();
  delete EAs;
}

// get_addr() doesn't return an actual address for a DataObject
// but rather synthesises an address-like identifier tuple.
// XXXX since an aggregate and its first element have identical tuples
// may need to arrange for special-purpose sorting "by address"
uint64_t
DataObject::get_addr ()
{
  uint64_t addr;
  if (parent && parent->get_typename ())
    addr = MAKE_ADDRESS (parent->id, offset);   // element
  else if (parent)
    addr = MAKE_ADDRESS (parent->id, id) | 0x8000000000000000ULL; // Scalar, Unknown
  else if (id == dbeSession->get_Scalars_DataObject ()->id)
    addr = MAKE_ADDRESS (id, 0) | 0x8000000000000000ULL;    // Scalar aggregate
  else if (id == dbeSession->get_Unknown_DataObject ()->id)
    addr = MAKE_ADDRESS (id, 0) | 0x8000000000000000ULL;    // Unknown aggregate
  else
    addr = MAKE_ADDRESS (id, 0);     // aggregate
  return addr;
}

Histable *
DataObject::convertto (Histable_type type, Histable *)
{
  return type == DOBJECT ? this : NULL;
}

char
DataObject::get_offset_mark ()
{
  enum
  {
    blocksize = 32
  };

  if (size == 0 || offset == -1)
    return '?';     // undefined
  if (size > blocksize)
    return '#';     // requires multiple blocks
  if (size == blocksize && (offset % blocksize == 0))
    return '<';     // fits block entirely
  if (offset % blocksize == 0)
    return '/';     // starts block
  if ((offset + size) % blocksize == 0)
    return '\\';    // closes block
  if (offset / blocksize == ((offset + size) / blocksize))
    return '|';     // inside block
  return 'X';       // crosses blocks unnecessarily
}

char *
DataObject::get_offset_name ()
{
  char *offset_name;
  if (parent && parent->get_typename ()) // element
    offset_name = dbe_sprintf (GTXT ("%c%+6lld .{%s %s}"),
			       get_offset_mark (), (long long) offset,
			       _typename ? _typename : GTXT ("NO_TYPE"),
			       _instname ? _instname : GTXT ("-")); // "NO_NAME"
  else if ((offset != -1) && (offset > 0)) // filler
    offset_name = dbe_sprintf (GTXT ("%c%+6lld %s"), get_offset_mark (),
			       (long long) offset, get_name ());
  else if (parent) // Scalar/Unknown element
    offset_name = dbe_sprintf (GTXT ("        .%s"), get_unannotated_name ());
  else // aggregate
    offset_name = dbe_strdup (get_name ());
  return offset_name;
}

void
DataObject::set_dobjname (char *type_name, char *inst_name)
{
  _unannotated_name = _typename = _instname = NULL;
  if (inst_name)
    _instname = dbe_strdup (inst_name);

  char *buf;
  if (parent == dbeSession->get_Scalars_DataObject ())
    {
      if (type_name)
	_typename = dbe_strdup (type_name);
      _unannotated_name = dbe_sprintf (NTXT ("{%s %s}"), type_name,
				       inst_name ? inst_name : NTXT ("-"));
      buf = dbe_sprintf (NTXT ("%s.%s"), parent->get_name (), _unannotated_name);
    }
  else if (parent == dbeSession->get_Unknown_DataObject ())
    {
      _unannotated_name = dbe_strdup (type_name);
      buf = dbe_sprintf (NTXT ("%s.%s"), parent->get_name (), _unannotated_name);
    }
  else
    {
      if (type_name)
	_typename = dbe_strdup (type_name);
      if (parent && parent->get_typename ())
	buf = dbe_sprintf (NTXT ("%s.{%s %s}"),
			   parent->get_name () ? parent->get_name () : NTXT ("ORPHAN"),
			   type_name ? type_name : NTXT ("NO_TYPE"),
			   inst_name ? inst_name : NTXT ("-")); // "NO_NAME"
      else
	buf = dbe_sprintf (NTXT ("{%s %s}"),
			   type_name ? type_name : NTXT ("NO_TYPE"),
			   inst_name ? inst_name : NTXT ("-")); // "NO_NAME"
    }
  name = buf;
  dbeSession->dobj_updateHT (this);
}

void
DataObject::set_name (char *string)
{
  name = dbe_strdup (string);
  dbeSession->dobj_updateHT (this);
}

DbeEA *
DataObject::find_dbeEA (Vaddr EA)
{
  DbeEA *dbeEA;
  int left = 0;
  int right = EAs->size () - 1;
  while (left <= right)
    {
      int index = (left + right) / 2;
      dbeEA = EAs->fetch (index);
      if (EA < dbeEA->eaddr)
	right = index - 1;
      else if (EA > dbeEA->eaddr)
	left = index + 1;
      else
	return dbeEA;
    }

  // None found, create a new one
  dbeEA = new DbeEA (this, EA);
  EAs->insert (left, dbeEA);
  return dbeEA;
}
