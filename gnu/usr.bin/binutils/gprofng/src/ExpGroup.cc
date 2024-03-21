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
#include "util.h"
#include "ExpGroup.h"
#include "Experiment.h"
#include "LoadObject.h"
#include "DbeSession.h"

//////////////////////////////////////////////////////////
//  class ExpGroup

int ExpGroup::phaseCompareIdx = 0;

ExpGroup::ExpGroup (char *nm)
{
  name = dbe_strdup (nm);
  canonical_path (name);
  exps = new Vector<Experiment*>;
  founder = NULL;
  groupId = 0;
  phaseCompareIdx++;
  loadObjs = NULL;
  loadObjsMap = NULL;
}

ExpGroup::~ExpGroup ()
{
  phaseCompareIdx++;
  free (name);
  delete exps;
  delete loadObjs;
  delete loadObjsMap;
}

void
ExpGroup::append (Experiment *exp)
{
  for (int i = 0, sz = exps->size (); i < sz; i++)
    {
      Experiment *e = exps->fetch (i);
      if (exp == e)
	return;
    }
  exps->append (exp);
  if (exps->size () == 1)
    founder = exp;
}

void
ExpGroup::drop_experiment (Experiment *exp)
{
  for (int i = 0, sz = exps->size (); i < sz; i++)
    {
      Experiment *e = exps->fetch (i);
      if (exp == e)
	{
	  exps->remove (i);
	  break;
	}
    }
  if (founder == exp)
    founder = NULL;
}

Vector<Experiment*> *
ExpGroup::get_founders ()
{
  Vector<Experiment*> *expList = NULL;
  for (int i = 0, sz = exps ? exps->size () : 0; i < sz; i++)
    {
      Experiment *exp = exps->fetch (i);
      if (exp->founder_exp == NULL)
	{
	  if (expList == NULL)
	    expList = new Vector<Experiment*>;
	  expList->append (exp);
	}
    }
  return expList;
}

void
ExpGroup::create_list_of_loadObjects ()
{
  if (loadObjs == NULL)
    {
      loadObjs = new Vector<LoadObject*>();
      loadObjsMap = new DefaultMap<LoadObject*, int>();
      for (int i = 0, sz = exps ? exps->size () : 0; i < sz; i++)
	{
	  Experiment *exp = exps->fetch (i);
	  for (int i1 = 0, sz1 = VecSize(exp->loadObjs); i1 < sz1; i1++)
	    {
	      LoadObject *lo = exp->loadObjs->fetch (i1);
	      if (!loadObjsMap->get (lo))
		{
		  loadObjs->append (lo);
		  loadObjsMap->put (lo, loadObjs->size ());
		}
	    }
	}
    }
}

LoadObject *
ExpGroup::get_comparable_loadObject (LoadObject *lo)
{
  create_list_of_loadObjects ();
  if (loadObjsMap->get (lo))
    return lo;
  if ((lo->flags & SEG_FLAG_EXE) != 0)
    if (dbeSession->expGroups->size () == dbeSession->nexps ())
      for (int i = 0, sz = loadObjs ? loadObjs->size () : 0; i < sz; i++)
	{
	  LoadObject *lobj = loadObjs->fetch (i);
	  if ((lobj->flags & SEG_FLAG_EXE) != 0)
	    return lobj;
	}

  long first_ind = -1;
  char *bname = get_basename (lo->get_pathname ());
  for (long i = 0, sz = loadObjs ? loadObjs->size () : 0; i < sz; i++)
    {
      LoadObject *lobj = loadObjs->get (i);
      if (lobj->comparable_objs == NULL
	  && strcmp (bname, get_basename (lobj->get_pathname ())) == 0)
	{
	  if (lo->platform == lobj->platform)
	    {
	      if ((lo->flags & SEG_FLAG_DYNAMIC) != 0)
		{
		  if (dbe_strcmp (lo->firstExp->uarglist,
				  lobj->firstExp->uarglist) == 0)
		    return lobj;
		}
	      else
		return lobj;
	    }
	  if (first_ind == -1)
	    first_ind = i;
	}
    }
  return first_ind == -1 ? NULL : loadObjs->get (first_ind);
}
