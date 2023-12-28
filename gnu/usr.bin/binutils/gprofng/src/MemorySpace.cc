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
#include <ctype.h>

#include "util.h"
#include "DbeSession.h"
#include "Application.h"
#include "Experiment.h"
#include "Exp_Layout.h"
#include "MetricList.h"
#include "MemObject.h"
#include "PathTree.h"
#include "DbeView.h"
#include "Metric.h"
#include "MemorySpace.h"
#include "Table.h"
#include "IndexObject.h"

MemObjType_t::MemObjType_t ()
{
  type = -1;
  name = NULL;
  index_expr = NULL;
  machmodel = NULL;
  mnemonic = 0;
  short_description = NULL;
  long_description = NULL;
}

MemObjType_t::~MemObjType_t ()
{
  free (name);
  free (index_expr);
  free (machmodel);
  free (short_description);
  free (long_description);
}

MemorySpace::MemorySpace (DbeView *_dbev, int _mstype)
{
  char *mname;
  dbev = _dbev;
  phaseIdx = -1;

  // set up the MemoryObject information
  objs = new HashMap<uint64_t, MemObj*>;
  mstype = _mstype;
  msindex_exp = NULL;
  msname = NULL;
  msindex_exp_str = NULL;

  // find the memory space in the table
  MemObjType_t *mot = findMemSpaceByIndex (mstype);
  if (mot)
    {
      msname = dbe_strdup (mot->name);
      if (mot->index_expr != NULL)
	{
	  msindex_exp_str = dbe_strdup (mot->index_expr);
	  msindex_exp = dbeSession->ql_parse (msindex_exp_str);
	  if (msindex_exp == NULL)
	    // this was checked when the definition was created
	    abort ();
	}
    }

  // create the Total and Unknown objects
  mname = dbe_strdup (NTXT ("<Total>"));
  total_memobj = createMemObject ((uint64_t) - 2, mname);
  mname = dbe_strdup (GTXT ("<Unknown>"));
  unk_memobj = createMemObject ((uint64_t) - 1, mname);
  hist_data_all = NULL;
  selected_mo_index = (uint64_t) - 3;
  sel_ind = -1;
}

MemorySpace::~MemorySpace ()
{
  reset ();
  delete objs;
  free (msname);
  free (msindex_exp);
  free (msindex_exp_str);
}

void
MemorySpace::reset ()
{
  if (hist_data_all != NULL)
    {
      delete hist_data_all;
      hist_data_all = NULL;
    }
  // do not clear the selected object's index
  // selected_mo_index = (uint64_t)-3;

  // destroy any existing objects, but keep the vector
  // Now that we have a hashmap, which has its own vector,
  //     safe to delete and reallocate
  delete objs;
  objs = new HashMap<uint64_t, MemObj*>;
}

// find a memory object by its memory-object index
int
MemorySpace::findMemObject (uint64_t indx)
{
  int index;
  Hist_data::HistItem *hi;
  if (indx == (uint64_t) - 3)
    return -1;

  Vec_loop (Hist_data::HistItem *, hist_data_all->hist_items, index, hi)
  {
    if (((uint64_t) ((MemObj *) hi->obj)->id) == indx)
      return index;
  }
  // object does not exist; filter change eliminated it, for example
  return -1;
}

// find the object referenced in the packet
MemObj *
MemorySpace::lookupMemObject (Experiment *exp, DataView *packets, long i)
{
  uint64_t idx;
  uint64_t va = (uint64_t) packets->getLongValue (PROP_VADDR, i);
  if (va == ABS_UNSUPPORTED)
    // return NULL, to ignore the record
    return NULL;
  if (va < ABS_CODE_RANGE)
    // The va is not valid, rather, it's an error code
    // return the <Unknown> object
    return unk_memobj;

  Expression::Context ctx (dbev, exp, packets, i);
  idx = msindex_exp->eval (&ctx);
  if (idx == (uint64_t) - 1)
    return unk_memobj;

  // do a binary search for the memory object
  MemObj *res = objs->get (idx);
  if (res == NULL)
    {
      res = createMemObject (idx, NULL);
      objs->put (idx, res);
    }
  else
    return res;

  // recompute range
  if (idx < idx_min)
    idx_min = idx;
  if (idx > idx_max)
    idx_max = idx;
  return res;
}

MemObj *
MemorySpace::createMemObject (uint64_t index, char *moname)
{
  MemObj *res;
  char *name;
  if (moname != NULL)
    {
      res = new MemObj (index, moname);
      return res;
    }

  // Custom memory objects
  // The memory_page_size is defined in the machine model file such
  // as ./machinemodels/t4.ermm.
  // Most users prefer to look at the hexadecimal version of virtual
  // addresses. Display only the hexadecimal version of virtual addresses
  // for all machine model views with an exception of virtual page size.
  if (dbe_strcmp (msname, NTXT ("Memory_page_size")) == 0)
    name = dbe_sprintf (NTXT ("%s 0x%16.16llx (%llu)"), msname,
			(long long) index, (unsigned long long) index);
  else if (dbe_strcmp (msname, NTXT ("Memory_in_home_lgrp")) == 0)
    name = dbe_sprintf (NTXT ("%s: %s"), msname,
			index == 1 ? GTXT ("True") : index == 0 ? GTXT ("False")
			: GTXT ("<Unknown>"));
  else if (dbe_strcmp (msname, NTXT ("Memory_lgrp")) == 0)
    name = dbe_sprintf (NTXT ("%s %llu"), msname, (unsigned long long) index);
  else
    name = dbe_sprintf (NTXT ("%s 0x%16.16llx"), msname, (long long) index);

  res = new MemObj (index, name);
  return res;
}


static Vector<MemObjType_t*> dyn_memobj_vec;
static Vector<MemObjType_t*> *dyn_memobj = &dyn_memobj_vec;
static Vector<int> *ordlist;

// Static function to get a vector of custom memory object definitions

Vector<void*> *
MemorySpace::getMemObjects ()
{
  MemObjType_t *mot;
  int ii;
  int size = dyn_memobj->size ();
  Vector<int> *indx = new Vector<int>(size);
  Vector<char*> *name = new Vector<char*>(size);
  Vector<char> *mnemonic = new Vector<char>(size);
  Vector<char*> *formula = new Vector<char*>(size);
  Vector<char*> *machmodel = new Vector<char*>(size);
  Vector<int> *order = new Vector<int>(size);
  Vector<char*> *sdesc = new Vector<char*>(size);
  Vector<char*> *ldesc = new Vector<char*>(size);

  if (size > 0)
    {
      Vec_loop (MemObjType_t *, dyn_memobj, ii, mot)
      {
	indx->store (ii, mot->type);
	order->store (ii, ii);
	name->store (ii, dbe_strdup (mot->name));
	formula->store (ii, dbe_strdup (mot->index_expr));
	mnemonic->store (ii, mot->mnemonic);
	sdesc->store (ii, mot->short_description == NULL ? NULL
		      : dbe_strdup (mot->short_description));
	ldesc->store (ii, mot->long_description == NULL ? NULL
		      : dbe_strdup (mot->long_description));
	if (mot->machmodel == NULL)
	  machmodel->store (ii, NULL);
	else
	  machmodel->store (ii, dbe_strdup (mot->machmodel));
      }
    }
  Vector<void*> *res = new Vector<void*>(8);
  res->store (0, indx);
  res->store (1, name);
  res->store (2, mnemonic);
  res->store (3, formula);
  res->store (4, machmodel);
  res->store (5, order);
  res->store (6, sdesc);
  res->store (7, ldesc);
  return (res);
}

// Static function to set order of memory object tabs
void
MemorySpace::set_MemTabOrder (Vector<int> *orders)
{
  int size = orders->size ();
  ordlist = new Vector<int>(size);
  for (int i = 0; i < size; i++)
    ordlist->store (i, orders->fetch (i));
}

// Static function to define a new memory object type
char *
MemorySpace::mobj_define (char *mname, char *mindex_exp, char *_machmodel,
			  char *short_description, char *long_description)
{
  MemObjType_t *mot;

  if (mname == NULL)
    return dbe_strdup (GTXT ("No memory object name has been specified."));
  if (isalpha ((int) (mname[0])) == 0)
    return dbe_sprintf (GTXT ("Memory Object type name %s does not begin with an alphabetic character"),
			mname);
  char *p = mname;
  while (*p != 0)
    {
      if (isalnum ((int) (*p)) == 0 && *p != '_')
	return dbe_sprintf (GTXT ("Memory Object type name %s contains a non-alphanumeric character"),
			    mname);
      p++;
    }

  mot = findMemSpaceByName (mname);
  if (mot != NULL)
    {
      if (strcmp (mot->index_expr, mindex_exp) == 0)
	// It's a redefinition, but the new definition is the same
	return NULL;
      return dbe_sprintf (GTXT ("Memory/Index Object type name %s is already defined"),
			  mname);
    }

  // make sure the name is not in use
  if (dbeSession->findIndexSpaceByName (mname) >= 0)
    return dbe_sprintf (GTXT ("Memory/Index Object type name %s is already defined"),
			mname);

  if (mindex_exp == NULL || *mindex_exp == 0)
    return dbe_strdup (GTXT ("No index-expr has been specified."));

  // verify that the index expression parses correctly
  Expression *e = dbeSession->ql_parse (mindex_exp);
  if (e == NULL)
    return dbe_sprintf (GTXT ("Memory Object index expression is invalid: %s"),
			mindex_exp);
  delete e;

  // It's OK, create the new table entry
  char *s = dbeSession->indxobj_define (mname, NULL, mindex_exp,
					short_description, long_description);
  if (s)
    return s;
  IndexObjType_t *indObj = dbeSession->findIndexSpace (mname);

  mot = new MemObjType_t;
  mot->type = indObj->type;
  indObj->memObj = mot;
  mot->name = dbe_strdup (mname);
  mot->index_expr = dbe_strdup (mindex_exp);
  mot->mnemonic = MemorySpace::pickMnemonic (mname);
  mot->machmodel = dbe_strdup (_machmodel);
  mot->short_description = dbe_strdup (short_description);
  mot->long_description = dbe_strdup (long_description);

  // add it to the list
  dyn_memobj->append (mot);

  // tell the session
  if (dbeSession != NULL)
    dbeSession->mobj_define (mot);
  return NULL;
}

// Static function to delete a new memory object type

char *
MemorySpace::mobj_delete (char *mname)
{
  if (mname == NULL)
    return dbe_strdup (GTXT ("No memory object name has been specified.\n"));

  // search the dynamic types
  for (long idx = 0, sz = VecSize (dyn_memobj); idx < sz; idx++)
    {
      MemObjType_t *mt = dyn_memobj->get (idx);
      if (strcasecmp (mt->name, mname) == 0)
	{
	  // delete it from the vector
	  mt = dyn_memobj->remove (idx);
	  delete mt;
	  dbeSession->removeIndexSpaceByName (mname);
	  return NULL;
	}
    }
  return dbe_sprintf (GTXT ("Memory object `%s' is not defined.\n"), mname);
}

// Static function to get a list of memory object names from a machine model

Vector <char*> *
MemorySpace::getMachineModelMemObjs (char *mname)
{
  Vector <char *> *ret = new Vector <char *> ();
  if (mname == NULL)
    return ret;

  // search the memory objects
  int idx;
  MemObjType_t *mt;
  Vec_loop (MemObjType_t*, dyn_memobj, idx, mt)
  {
    if (mt->machmodel != NULL && strcmp (mt->machmodel, mname) == 0)
      {
	char *n = dbe_strdup (mt->name);
	ret->append (n);
      }
  }
  return ret;
}

char
MemorySpace::pickMnemonic (char *name)
{
  return name[0];
}

void
MemorySpace::get_filter_keywords (Vector <void*> *res)
{
  Vector <char*> *kwCategory = (Vector<char*>*) res->fetch (0);
  Vector <char*> *kwCategoryI18N = (Vector<char*>*) res->fetch (1);
  Vector <char*> *kwDataType = (Vector<char*>*) res->fetch (2);
  Vector <char*> *kwKeyword = (Vector<char*>*) res->fetch (3);
  Vector <char*> *kwFormula = (Vector<char*>*) res->fetch (4);
  Vector <char*> *kwDescription = (Vector<char*>*) res->fetch (5);
  Vector <void*> *kwEnumDescs = (Vector<void*>*) res->fetch (6);

  char *vtypeNames[] = VTYPE_TYPE_NAMES;
  for (int i = 0, sz = dyn_memobj ? dyn_memobj->size () : 0; i < sz; i++)
    {
      MemObjType_t *obj = dyn_memobj->fetch (i);
      kwCategory->append (dbe_strdup (NTXT ("FK_MEMOBJ")));
      kwCategoryI18N->append (dbe_strdup (GTXT ("Memory Object Definitions")));
      kwDataType->append (dbe_strdup (vtypeNames[TYPE_INT64]));
      kwKeyword->append (dbe_strdup (obj->name));
      kwFormula->append (dbe_strdup (obj->index_expr));
      kwDescription->append (NULL);
      kwEnumDescs->append (NULL);
    }
}

MemObjType_t *
MemorySpace::findMemSpaceByName (const char *mname)
{
  int idx;
  MemObjType_t *mt;

  // search the dynamic types
  Vec_loop (MemObjType_t*, dyn_memobj, idx, mt)
  {
    if (strcasecmp (mt->name, mname) == 0)
      return mt;
  }
  return NULL;
}

MemObjType_t *
MemorySpace::findMemSpaceByIndex (int index)
{
  int idx;
  MemObjType_t *mt;

  // search the dynamic types
  Vec_loop (MemObjType_t*, dyn_memobj, idx, mt)
  {
    if (mt->type == index)
      return mt;
  }
  return NULL;
}
