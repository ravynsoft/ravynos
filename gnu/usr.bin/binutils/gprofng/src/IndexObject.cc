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
#include "DbeSession.h"
#include "DbeView.h"
#include "IndexObject.h"
#include "StringBuilder.h"

IndexObject::IndexObject (int _indextype, uint64_t _index)
{
  indextype = _indextype;
  obj = NULL;
  id = _index;
  name = NULL;
  nameIsFinal = false;
}

IndexObject::IndexObject (int _indextype, Histable *_obj)
{
  indextype = _indextype;
  obj = _obj;
  id = obj ? obj->id : (uint64_t) - 1;
  name = NULL;
  nameIsFinal = false;
}

void
IndexObject::set_name (char * other_name)
{
  if (name == NULL)
    {
      name = other_name;
      nameIsFinal = true;
    }
}

static uint64_t
extractExpgrid (uint64_t id)
{
  return (id >> IndexObject::INDXOBJ_EXPGRID_SHIFT)
	  & IndexObject::INDXOBJ_EXPGRID_MASK;
}

static uint64_t
extractExpid (uint64_t id)
{
  return (id >> IndexObject::INDXOBJ_EXPID_SHIFT)
	  & IndexObject::INDXOBJ_EXPID_MASK;
}

static uint64_t
extractPayload (uint64_t id)
{
  return (id >> IndexObject::INDXOBJ_PAYLOAD_SHIFT)
	  & IndexObject::INDXOBJ_PAYLOAD_MASK;
}

static void
printCompareLabel (StringBuilder *sb, uint64_t grpId);

static bool
printThread (StringBuilder *sbIn, Expression::Context * ctx, uint64_t id)
{
  uint64_t proc = extractExpid (id);
  uint64_t thrid = extractPayload (id);
  bool isFinal = true;
  bool hasJava = false;
  bool javaThread = false;
  if (ctx)
    {
      if (ctx->dview && ctx->dview->getProp (PROP_JTHREAD))
	{
	  hasJava = true;
	  uint64_t tstamp = ctx->dview->getLongValue (PROP_TSTAMP, ctx->eventId);
	  JThread *jthread = ctx->exp->map_pckt_to_Jthread (thrid, tstamp);
	  if (jthread != JTHREAD_NONE && jthread != JTHREAD_DEFAULT)
	    {
	      sbIn->appendf (GTXT ("Process %llu, Thread %llu, JThread %llu \'%s\', Group \'%s\', Parent \'%s\'"),
			     (unsigned long long) proc,
			     (unsigned long long) thrid,
			     (unsigned long long) jthread->jthr_id,
			     get_str(jthread->name, ""),
			     get_str(jthread->group_name, ""),
			     get_str(jthread->parent_name, ""));
	      javaThread = true;
	    }
	}
    }
  if (!javaThread)
    {
      sbIn->appendf (GTXT ("Process %llu, Thread %llu"),
		     (unsigned long long) proc, (unsigned long long) thrid);
      if (hasJava)
	// sometimes threads start as native and later become Java; keep checking
	isFinal = false;
    }
  if (ctx && ctx->dbev && ctx->dbev->comparingExperiments ())
    {
      Vector <Histable *> *v = ctx->exp->get_comparable_objs ();
      int st = 0;
      for (long i = 0, sz = VecSize (v); i < sz; i++)
	{
	  Experiment *exp = (Experiment *) v->get (i);
	  if (exp)
	    {
	      if (st == 0)
		{
		  st = 1;
		  continue;
		}
	      sbIn->appendf (GTXT (" [ Group %llu  Process %llu ]"),
			     (unsigned long long) exp->groupId - 1,
			     (unsigned long long) exp->getUserExpId ());
	    }
	}
    }
  return isFinal;
}

static bool
printProcess (StringBuilder *sbIn, Expression::Context * ctx, uint64_t id)
{
  uint64_t proc = id;
  if (ctx && ctx->exp)
    {
      int st = 0;
      if (ctx->dbev && ctx->dbev->comparingExperiments ())
	{
	  Vector <Histable *> *v = ctx->exp->get_comparable_objs ();
	  for (long i = 0, sz = VecSize (v); i < sz; i++)
	    {
	      Experiment *exp = (Experiment *) v->get (i);
	      if (exp)
		{
		  if (st == 0)
		    {
		      st = 1;
		      sbIn->appendf (GTXT ("%s, Process %3llu, PID %llu"),
				 get_str (exp->utargname, GTXT ("(unknown)")),
				     (unsigned long long) proc,
				     (unsigned long long) exp->getPID ());
		      continue;
		    }
		  sbIn->appendf (GTXT (" [ Group %llu,  Process %llu, PID %llu ]"),
				 (unsigned long long) exp->groupId - 1,
				 (unsigned long long) exp->getUserExpId (),
				 (unsigned long long) exp->getPID ());
		}
	    }
	}
      if (st == 0)
	sbIn->appendf (GTXT ("%s, Process %3llu, PID %llu"),
		       get_str (ctx->exp->utargname, GTXT ("(unknown)")),
		       (unsigned long long) proc,
		       (unsigned long long) ctx->exp->getPID ());
    }
  else
    sbIn->appendf (GTXT ("Process %3llu"), (unsigned long long) proc);
  return true; //name is final
}

static bool
printExperiment (StringBuilder *sbIn, Expression::Context * ctx, uint64_t id)
{
  uint64_t grpId = extractExpgrid (id);
  uint64_t expid = extractExpid (id);
  if (ctx && ctx->dbev->comparingExperiments ())
    printCompareLabel (sbIn, grpId);
  if (ctx)
    {
      Experiment *hasFounder = ctx->exp->founder_exp;
      int pid = ctx->exp->getPID ();
      uint64_t founderExpid;
      if (hasFounder)
	founderExpid = hasFounder->getUserExpId ();
      else
	founderExpid = expid;
      sbIn->appendf (GTXT ("Base Experiment %llu, Process %llu, PID %llu, %s"),
		     (unsigned long long) founderExpid,
		     (unsigned long long) expid,
		     (unsigned long long) pid,
		     get_str (ctx->exp->utargname, GTXT ("(unknown)")));
    }
  else
    sbIn->appendf (GTXT ("Process %llu"), (unsigned long long) expid);
  return true; // name is final
}

void
IndexObject::set_name_from_context (Expression::Context * ctx)
{
  if (name != NULL)
    if (nameIsFinal && strstr (name, GTXT ("(unknown)")) == NULL)
      return;
  if (ctx == NULL || ctx->dview == NULL || ctx->dbev == NULL)
    return;
  StringBuilder sb;
  switch (indextype)
    {
    case INDEX_THREADS:
      nameIsFinal = printThread (&sb, ctx, id);
      break;
    case INDEX_PROCESSES:
      nameIsFinal = printProcess (&sb, ctx, id);
      break;
    case INDEX_EXPERIMENTS:
      nameIsFinal = printExperiment (&sb, ctx, id);
      break;
    default:
      name = NULL;
      return;
    }
  if (sb.length ())
    name = sb.toString ();
}

static void
printCompareLabel (StringBuilder *sbIn, uint64_t grpId)
{
  static const char *labels[] = {"", GTXT ("Baseline"), GTXT ("Comparison")};
  static int length;
  if (!length)
    {
      length = strlen (labels[1]);
      int length2 = strlen (labels[2]);
      if (length < length2)
	length = length2;
      length += 5; // for open/close brace and grpId number and spaces
    }
  char *s = NULL;
  if (grpId != 0)
    {
      if (grpId <= 2)
	s = dbe_sprintf ("[%s]", labels[grpId]);
      else
	s = dbe_sprintf ("[%s-%llu]", labels[2],
			 (unsigned long long) (grpId - 1));
    }
  sbIn->appendf ("%-*s", length, get_str (s, ""));
  free (s);
}

char *
IndexObject::get_name (NameFormat fmt)
{
  if (name == NULL)
    {
      StringBuilder sb;
      int64_t upper;
      int64_t num1;
      int64_t num2;
      switch (indextype)
	{
	case INDEX_THREADS:
	  printThread (&sb, NULL, id);
	  break;

	case INDEX_CPUS:
	  sb.sprintf (GTXT ("CPU %llu"), (unsigned long long) id);
	  break;

	case INDEX_SAMPLES:
	  sb.sprintf (GTXT ("Sample %llu"), (unsigned long long) id);
	  break;

	case INDEX_GCEVENTS:
	  if (id == 0)
	    {
	      sb.sprintf (GTXT ("Not in any GCEvent"));
	    }
	  else
	    {
	      sb.sprintf (GTXT ("GCEvent %llu"), (unsigned long long) id);
	    }
	  break;

	case INDEX_SECONDS:
	  sb.sprintf (GTXT ("Second of execution %llu"), (unsigned long long) id);
	  break;

	case INDEX_PROCESSES:
	  printProcess (&sb, NULL, id);
	  break;

	case INDEX_EXPERIMENTS:
	  printExperiment (&sb, NULL, id);
	  break;
	case INDEX_BYTES:
	  upper = id;
	  if (id == -1)
	    {
	      break;
	    }
	  if (id % 2 == 1 && id > 1)
	    {
	      upper = id - 1;
	      if (upper >= 1099511627776)
		{
		  num1 = upper / 1099511627776;
		  sb.sprintf (GTXT (">= %3llu TB"), (unsigned long long) num1);
		}
	      else
		{
		  // XXXX do nothing, this should not happen
		}
	    }
	  else
	    {
	      if (upper >= 1099511627776)
		{
		  num1 = upper / 1099511627776;
		  num2 = num1 / 4;
		  if (num2)
		    {
		      sb.sprintf (GTXT ("%3lluTB < n <= %3lluTB"), (unsigned long long) num2, (unsigned long long) num1);
		    }
		  else
		    {
		      sb.sprintf (GTXT ("256GB < n <= %3lluTB"), (unsigned long long) num1);
		    }
		}
	      else if (upper >= 1073741824)
		{
		  num1 = upper / 1073741824;
		  num2 = num1 / 4;
		  if (num2)
		    {
		      sb.sprintf (GTXT ("%3lluGB < n <= %3lluGB"), (unsigned long long) num2, (unsigned long long) num1);
		    }
		  else
		    {
		      sb.sprintf (GTXT ("256MB < n <= %3lluGB"), (unsigned long long) num1);
		    }
		}
	      else if (upper >= 1048576)
		{
		  num1 = upper / 1048576;
		  num2 = num1 / 4;
		  if (num2)
		    {
		      sb.sprintf (GTXT ("%3lluMB < n <= %3lluMB"), (unsigned long long) num2, (unsigned long long) num1);
		    }
		  else
		    {
		      sb.sprintf (GTXT ("256KB < n <= %3lluMB"), (unsigned long long) num1);
		    }
		}
	      else if (upper >= 1024)
		{
		  num1 = upper / 1024;
		  num2 = num1 / 4;
		  if (num2)
		    {
		      sb.sprintf (GTXT ("%3lluKB < n <= %3lluKB"), (unsigned long long) num2, (unsigned long long) num1);
		    }
		  else
		    {
		      sb.sprintf (GTXT ("  256 < n <= %3lluKB"), (unsigned long long) num1);
		    }
		}
	      else if (upper > 0)
		{
		  num1 = upper;
		  num2 = num1 / 4;
		  if (num1 == 1)
		    {
		      sb.sprintf (GTXT ("    1 Byte"));
		    }
		  else
		    {
		      sb.sprintf (GTXT ("%5llu < n <= %5llu Bytes"), (unsigned long long) num2, (unsigned long long) num1);
		    }
		}
	      else if (upper == 0)
		{
		  sb.sprintf (GTXT ("    0 Bytes"));
		}
	      else
		{
		  sb.sprintf (GTXT ("<No Data>"));
		}
	    }
	  break;
	case INDEX_DURATION:
	  if (id == -1)
	    {
	      break;
	    }

	  if (id > 10000000000000)
	    {
	      sb.sprintf (GTXT ("n > 10000s"));
	    }
	  else if (id > 1000000000000)
	    {
	      sb.sprintf (GTXT ("1000s < n <= 10000s"));
	    }
	  else if (id > 100000000000)
	    {
	      sb.sprintf (GTXT (" 100s < n <= 1000s"));
	    }
	  else if (id > 10000000000)
	    {
	      sb.sprintf (GTXT ("  10s < n <=  100s"));
	    }
	  else if (id > 1000000000)
	    {
	      sb.sprintf (GTXT ("   1s < n <=   10s"));
	    }
	  else if (id > 100000000)
	    {
	      sb.sprintf (GTXT ("100ms < n <=    1s"));
	    }
	  else if (id > 10000000)
	    {
	      sb.sprintf (GTXT (" 10ms < n <= 100ms"));
	    }
	  else if (id > 1000000)
	    {
	      sb.sprintf (GTXT ("  1ms < n <=  10ms"));
	    }
	  else if (id > 100000)
	    {
	      sb.sprintf (GTXT ("100us < n <=   1ms"));
	    }
	  else if (id > 10000)
	    {
	      sb.sprintf (GTXT (" 10us < n <= 100us"));
	    }
	  else if (id > 1000)
	    {
	      sb.sprintf (GTXT ("  1us < n <=  10us"));
	    }
	  else if (id > 0)
	    {
	      sb.sprintf (GTXT ("   0s < n <=   1us"));
	    }
	  else if (id == 0)
	    {
	      sb.sprintf (GTXT ("   0s"));
	    }
	  else
	    {
	      sb.sprintf (GTXT ("<No Data>"));
	    }
	  break;

	  // Custom index objects
	default:
	  if (obj)
	      sb.sprintf (GTXT ("%s from %s"),
			  dbeSession->getIndexSpaceDescr (indextype), obj->get_name (fmt));
	  else
	    {
	      IndexObjType_t *indexObj = dbeSession->getIndexSpace (indextype);
	      if (indexObj->memObj)
		{
		  if (strcasecmp (indexObj->name, NTXT ("Memory_page_size")) == 0)
		    {
		      if (id == 0)
			  sb.append (GTXT ("<Unknown>"));
		      else
			  sb.sprintf (NTXT ("%s 0x%16.16llx (%llu)"), indexObj->name,
				      (unsigned long long) id, (unsigned long long) id);
		    }
		  else if (strcasecmp (indexObj->name, NTXT ("Memory_in_home_lgrp")) == 0)
		    {
		      if (id == 0 || id == 1)
			  sb.sprintf (NTXT ("%s: %s"), indexObj->name,
				      id == 1 ? GTXT ("True") : GTXT ("False"));
		      else
			  sb.sprintf (NTXT ("%s %s (0x%llx"), indexObj->name,
				      GTXT ("<Unknown>"), (unsigned long long) id);
		    }
		  else if (strcasecmp (indexObj->name, NTXT ("Memory_lgrp")) == 0)
		    {
		      if (id == 0)
			  sb.append (GTXT ("<Unknown>"));
		      else
			  sb.sprintf (NTXT ("%s %llu"), indexObj->name, (unsigned long long) id);
		    }
		  else
		      sb.sprintf (NTXT ("%s 0x%16.16llx"), indexObj->name, (unsigned long long) id);
		}
	      else
		  sb.sprintf ("%s 0x%16.16llx (%llu)", indexObj->name,
			      (unsigned long long) id, (unsigned long long) id);
	    }
	}
      name = sb.toString ();
      nameIsFinal = true;
    }
  return name;
}

bool
IndexObject::requires_string_sort ()
{
  if (indextype == INDEX_PROCESSES || indextype >= INDEX_LAST)
    return true;
  return false;
}

Histable *
IndexObject::convertto (Histable_type type, Histable *ext)
{
  if (type == INDEXOBJ)
    return this;
  if (obj)
    return obj->convertto (type, ext);
  return NULL;
}

IndexObjType_t::IndexObjType_t ()
{
  type = 0;
  name = NULL;
  i18n_name = NULL;
  index_expr_str = NULL;
  index_expr = NULL;
  mnemonic = 0;
  short_description = NULL;
  long_description = NULL;
  memObj = NULL;
}

IndexObjType_t::~IndexObjType_t ()
{
  free (name);
  free (i18n_name);
  free (index_expr_str);
  delete index_expr;
  free (short_description);
  free (long_description);
}
